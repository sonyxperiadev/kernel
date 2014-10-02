/*
 * TI LP855x Backlight Driver
 *
 * Copyright (C) 2011 Texas Instruments
 * Copyright (C) 2012-2013 Sony Mobile Communications AB.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/module.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/backlight.h>
#include <linux/err.h>
#include <linux/debugfs.h>
#include <linux/uaccess.h>
#include <linux/lp855x.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/regulator/consumer.h>

#define BRIGHTNESS_CTRL 0x00
#define DEVICE_CTRL     0x01
#define FULL_BRIGHTNESS 255

#define BL_CTL_SHFT     0
#define BRT_MODE_SHFT   1
#define BRT_MODE_MASK   0x06

#define CFG98_CTRL      0x98
#define IBOOST_LIM_2X_MASK   0x80

#define CFG3_CTRL       0xA3
#define SLOPE_FILTER_MASK    0x7C

/* Enable backlight. Only valid when BRT_MODE=10(I2C only) */
#define ENABLE_BL       1
#define DISABLE_BL      0

#define I2C_CONFIG(id)  id ## _I2C_CONFIG
#define PWM_CONFIG(id)  id ## _PWM_CONFIG

/* DEVICE CONTROL register - LP8550 */
#define LP8550_PWM_CONFIG  (LP8550_PWM_ONLY << BRT_MODE_SHFT)
#define LP8550_I2C_CONFIG  ((ENABLE_BL << BL_CTL_SHFT) | \
	(LP8550_I2C_ONLY << BRT_MODE_SHFT))

/* DEVICE CONTROL register - LP8551 */
#define LP8551_PWM_CONFIG  LP8550_PWM_CONFIG
#define LP8551_I2C_CONFIG  LP8550_I2C_CONFIG

/* DEVICE CONTROL register - LP8552 */
#define LP8552_PWM_CONFIG  LP8550_PWM_CONFIG
#define LP8552_I2C_CONFIG  LP8550_I2C_CONFIG

/* DEVICE CONTROL register - LP8553 */
#define LP8553_PWM_CONFIG  LP8550_PWM_CONFIG
#define LP8553_I2C_CONFIG  LP8550_I2C_CONFIG

/* DEVICE CONTROL register - LP8556 */
#define LP8556_PWM_CONFIG   (LP8556_PWM_ONLY << BRT_MODE_SHFT)
#define LP8556_COMB1_CONFIG (LP8556_COMBINED1 << BRT_MODE_SHFT)
#define LP8556_I2C_CONFIG   ((ENABLE_BL << BL_CTL_SHFT) | \
	(LP8556_I2C_ONLY << BRT_MODE_SHFT))
#define LP8556_COMB2_CONFIG (LP8556_COMBINED2 << BRT_MODE_SHFT)

/* ROM area boundary */
#define EEPROM_START 0xA0
#define EEPROM_END   0xA7
#define EPROM_START  0x98
#define EPROM_END    0xAF

#define LP855x_PWR_DELAY_US 12000

enum lp855x_chip_id {
	LP8550,
	LP8551,
	LP8552,
	LP8553,
	LP8556,
};

enum lp8550_brighntess_source {
	LP8550_PWM_ONLY,
	LP8550_I2C_ONLY = 2,
};

enum lp8551_brighntess_source {
	LP8551_PWM_ONLY = LP8550_PWM_ONLY,
	LP8551_I2C_ONLY = LP8550_I2C_ONLY,
};

enum lp8552_brighntess_source {
	LP8552_PWM_ONLY = LP8550_PWM_ONLY,
	LP8552_I2C_ONLY = LP8550_I2C_ONLY,
};

enum lp8553_brighntess_source {
	LP8553_PWM_ONLY = LP8550_PWM_ONLY,
	LP8553_I2C_ONLY = LP8550_I2C_ONLY,
};

enum lp8556_brightness_source {
	LP8556_PWM_ONLY,
	LP8556_COMBINED1, /* pwm + i2c before the shaper block */
	LP8556_I2C_ONLY,
	LP8556_COMBINED2, /* pwm + i2c after the shaper block */
};

#ifdef CONFIG_DEBUG_FS
struct debug_dentry {
	struct dentry *dir;
	struct dentry *reg;
	struct dentry *chip;
	struct dentry *blmode;
};
#endif

struct lp855x {
	const struct i2c_device_id *chipid;
	struct i2c_client *client;
	struct backlight_device *bl;
	struct device *dev;
	struct mutex xfer_lock;
	struct lp855x_platform_data *pdata;
	int stored_br;
#ifdef CONFIG_DEBUG_FS
	struct debug_dentry dd;
#endif
};

static int lp855x_i2c_read(struct lp855x *lp, u8 reg, u8 *data, u8 len)
{
	s32 ret;

	ret = i2c_smbus_read_i2c_block_data(lp->client, reg, len, data);
	return (ret != len) ? -EIO : 0;
}

static int lp855x_i2c_write(struct lp855x *lp, u8 reg, u8 *data, u8 len)
{
	s32 ret;

	ret = i2c_smbus_write_i2c_block_data(lp->client, reg, len, data);
	return ret;
}

static inline int lp855x_read_byte(struct lp855x *lp, u8 reg, u8 *data)
{
	return lp855x_i2c_read(lp, reg, data, 1);
}

static inline int lp855x_write_byte(struct lp855x *lp, u8 reg, u8 data)
{
	u8 written = data;
	return lp855x_i2c_write(lp, reg, &written, 1);
}

#ifdef CONFIG_DEBUG_FS
static int lp855x_dbg_open(struct inode *inode, struct file *file)
{
	file->private_data = inode->i_private;
	return 0;
}

static ssize_t lp855x_help_register(struct file *file, char __user *userbuf,
				    size_t count, loff_t *ppos)
{
	const char *help = "\nHow to read/write LP855x registers\n\n"
	"(example) To read 0x00 register,\n"
	"echo 0x00 r > /sys/kernel/debug/lp855x/registers\n"
	"To write 0xff into 0x1 address,\n"
	"echo 0x00 0xff w > /sys/kernel/debug/lp855x/registers\n"
	"To dump values from 0x00 to 0x06 address,\n"
	"echo 0x00 0x06 d > /sys/kernel/debug/lp855x/registers\n";

	return simple_read_from_buffer(userbuf, count, ppos, help,
			sizeof(help) - 1);
}

static char *lp855x_parse_register_cmd(const char *cmd, size_t count, u8 *byte)
{
	char tmp[10];
	char *blank;
	unsigned long arg;

	blank = strnchr(cmd, count, ' ');
	memset(tmp, 0x0, sizeof(tmp));
	memcpy(tmp, cmd, blank - cmd);

	if (kstrtol(tmp, 16, &arg) < 0)
		return NULL;

	*byte = arg;
	return blank;
}

static ssize_t lp855x_ctrl_register(struct file *file,
				    const char __user *userbuf, size_t count,
				    loff_t *ppos)
{
	char mode, buf[20];
	char *pos, *pos2;
	u8 i, arg1, arg2, val;
	struct lp855x *lp = file->private_data;
	int rc;
	size_t offs;

	if (copy_from_user(buf, userbuf, min(count, sizeof(buf))))
		return -EFAULT;

	mode = buf[count - 2];
	switch (mode) {
	case 'r':
		if (!lp855x_parse_register_cmd(buf, count, &arg1))
			return -EINVAL;

		rc = lp855x_read_byte(lp, arg1, &val);
		dev_info(lp->dev, "Read [0x%.2x] = 0x%.2x\n", arg1, val);
		break;
	case 'w':
		pos = lp855x_parse_register_cmd(buf, count, &arg1);
		if (!pos)
			return -EINVAL;
		offs = pos - buf;
		pos2 = lp855x_parse_register_cmd(pos + 1, count - offs, &arg2);
		if (!pos2)
			return -EINVAL;

		rc = lp855x_write_byte(lp, arg1, arg2);
		dev_info(lp->dev, "Written [0x%.2x] = 0x%.2x\n", arg1, arg2);
		break;
	case 'd':
		pos = lp855x_parse_register_cmd(buf, count, &arg1);
		if (!pos)
			return -EINVAL;
		offs = pos - buf;
		pos2 = lp855x_parse_register_cmd(pos + 1, count - offs, &arg2);
		if (!pos2)
			return -EINVAL;

		for (rc = 0, i = arg1; i <= arg2 && !rc; i++) {
			rc = lp855x_read_byte(lp, i, &val);
			dev_info(lp->dev, "Read [0x%.2x] = 0x%.2x\n", i, val);
		}
		break;
	default:
		rc = -EINVAL;
		break;
	}

	return rc ? rc : count;
}

static ssize_t lp855x_get_chipid(struct file *file, char __user *userbuf,
				 size_t count, loff_t *ppos)
{
	struct lp855x *lp = file->private_data;
	char buf[10];
	unsigned int len;

	len = scnprintf(buf, sizeof(buf), "%s\n", lp->chipid->name);
	return simple_read_from_buffer(userbuf, count, ppos, buf, len);
}

static ssize_t lp855x_get_bl_mode(struct file *file, char __user *userbuf,
				  size_t count, loff_t *ppos)
{
	char buf[20];
	unsigned int len;
	char *strmode = NULL;
	struct lp855x *lp = file->private_data;
	enum lp855x_brightness_ctrl_mode mode = lp->pdata->mode;

	if (mode == PWM_BASED)
		strmode = "pwm based";
	else if (mode == REGISTER_BASED)
		strmode = "register based";

	len = scnprintf(buf, sizeof(buf), "%s\n", strmode);
	return simple_read_from_buffer(userbuf, count, ppos, buf, len);
}

#define LP855X_DBG_ENTRY(name, pread, pwrite) \
static const struct file_operations dbg_##name##_fops = { \
	.open = lp855x_dbg_open, \
	.read = pread, \
	.write = pwrite, \
	.owner = THIS_MODULE, \
	.llseek = default_llseek, \
}

LP855X_DBG_ENTRY(registers, lp855x_help_register, lp855x_ctrl_register);
LP855X_DBG_ENTRY(chip, lp855x_get_chipid, NULL);
LP855X_DBG_ENTRY(blmode, lp855x_get_bl_mode, NULL);

static void lp855x_create_debugfs(struct lp855x *lp)
{
	struct debug_dentry *dd = &lp->dd;

	dd->dir = debugfs_create_dir("lp855x", NULL);

	dd->reg = debugfs_create_file("registers", S_IWUSR | S_IRUGO,
				      dd->dir, lp, &dbg_registers_fops);

	dd->chip = debugfs_create_file("chip_id", S_IRUGO,
				       dd->dir, lp, &dbg_chip_fops);

	dd->blmode = debugfs_create_file("bl_ctl_mode", S_IRUGO,
					 dd->dir, lp, &dbg_blmode_fops);
}

static void lp855x_remove_debugfs(struct lp855x *lp)
{
	struct debug_dentry *dd = &lp->dd;

	debugfs_remove(dd->blmode);
	debugfs_remove(dd->chip);
	debugfs_remove(dd->reg);
	debugfs_remove(dd->dir);
}
#else
static inline void lp855x_create_debugfs(struct lp855x *lp)
{
}

static inline void lp855x_remove_debugfs(struct lp855x *lp)
{
}
#endif

static int lp855x_is_valid_rom_area(struct lp855x *lp, u8 addr)
{
	bool rc;

	switch (lp->chipid->driver_data) {
	case LP8550:
	case LP8551:
	case LP8552:
	case LP8553:
		rc = addr >= EEPROM_START && addr <= EEPROM_END;
		break;
	case LP8556:
		rc = addr >= EPROM_START && addr <= EPROM_END;
		break;
	default:
		rc = false;
		break;
	}
	return rc;
}

static void lp855x_init_device(struct lp855x *lp)
{
	u8 val, addr;
	int i, ret;
	struct lp855x_platform_data *pd = lp->pdata;

	val = pd->device_control;
	ret = lp855x_write_byte(lp, DEVICE_CTRL, val);

	if (pd->load_new_rom_data && pd->size_program) {
		for (i = 0; i < pd->size_program; i++) {
			addr = pd->rom_data[i].addr;
			val = pd->rom_data[i].val;
			if (!lp855x_is_valid_rom_area(lp, addr))
				continue;

			if (addr == CFG98_CTRL) {
				u8 cfg98;
				lp855x_read_byte(lp, CFG98_CTRL, &cfg98);
				cfg98 &= ~IBOOST_LIM_2X_MASK;
				val = cfg98 | (val & IBOOST_LIM_2X_MASK);
			}

			ret |= lp855x_write_byte(lp, addr, val);
		}
	}
	ret |= lp855x_write_byte(lp, CFG3_CTRL, pd->cfg3);

	if (ret)
		dev_err(lp->dev, "i2c write err\n");
}

static int lp855x_set_slope_filter(struct lp855x *lp, bool enable)
{
	u8 val = lp->pdata->cfg3;

	if (!enable)
		val &= ~SLOPE_FILTER_MASK;
	return lp855x_write_byte(lp, CFG3_CTRL, val);
}

static int lp855x_bl_update_status(struct backlight_device *bl)
{
	struct lp855x *lp = bl_get_data(bl);
	enum lp855x_brightness_ctrl_mode mode = lp->pdata->mode;
	int br;

	mutex_lock(&lp->xfer_lock);
	if (bl->props.state & BL_CORE_SUSPENDED)
		bl->props.brightness = 0;

	br = bl->props.brightness;

	if (bl->props.state & BL_CORE_FBBLANK)
		br = 0;

	if (!br && lp->stored_br)
		(void)lp855x_set_slope_filter(lp, false);
	else if (br && !lp->stored_br)
		(void)lp855x_set_slope_filter(lp, true);

	lp->stored_br = br;

	if (mode == PWM_BASED) {
		struct lp855x_pwm_data *pd = &lp->pdata->pwm_data;
		int max_br = bl->props.max_brightness;

		if (pd->pwm_set_intensity)
			pd->pwm_set_intensity(br, max_br);

	} else if (mode == REGISTER_BASED)
		lp855x_write_byte(lp, BRIGHTNESS_CTRL, br);

	mutex_unlock(&lp->xfer_lock);

	return br;
}

static int lp855x_bl_get_brightness(struct backlight_device *bl)
{
	struct lp855x *lp = bl_get_data(bl);
	enum lp855x_brightness_ctrl_mode mode = lp->pdata->mode;
	int br;

	mutex_lock(&lp->xfer_lock);
	if (mode == PWM_BASED) {
		struct lp855x_pwm_data *pd = &lp->pdata->pwm_data;
		int max_br = bl->props.max_brightness;

		if (pd->pwm_get_intensity)
			bl->props.brightness = pd->pwm_get_intensity(max_br);

	} else if (mode == REGISTER_BASED) {
		u8 val;

		lp855x_read_byte(lp, BRIGHTNESS_CTRL, &val);
		bl->props.brightness = val;
	}
	br = bl->props.brightness;
	mutex_unlock(&lp->xfer_lock);
	return br;
}

static const struct backlight_ops lp855x_bl_ops = {
	.options = BL_CORE_SUSPENDRESUME,
	.update_status = lp855x_bl_update_status,
	.get_brightness = lp855x_bl_get_brightness,
};

static int lp855x_backlight_register(struct lp855x *lp)
{
	struct backlight_device *bl;
	struct backlight_properties props;
	const char *name = lp->pdata->name;

	if (!name)
		return -ENODEV;

	memset(&props, 0, sizeof(struct backlight_properties));
	props.type = BACKLIGHT_RAW;
	props.brightness = lp->pdata->initial_brightness;
	props.max_brightness =
		lp->pdata->max_brightness > FULL_BRIGHTNESS ?
		FULL_BRIGHTNESS : lp->pdata->max_brightness;

	bl = backlight_device_register(name, lp->dev, lp,
				       &lp855x_bl_ops, &props);
	if (IS_ERR(bl))
		return -EIO;

	lp->bl = bl;

	return 0;
}

static void lp855x_backlight_unregister(struct lp855x *lp)
{
	if (lp->bl)
		backlight_device_unregister(lp->bl);
}

#define INITIAL_BRT	0x3F
#define MAX_BRT	0xFF
static const struct i2c_device_id *lp855x_get_i2c_device_id(const char *chip);

static int lp855x_parse_dt(struct device *dev,
		struct lp855x_platform_data *pdata,
		const struct i2c_device_id **id)
{
	int i, rc;
	const char *str;
	struct device_node *node, *temp;
	unsigned tmp_data;

	rc = of_property_read_string(dev->of_node, "linux,name", &pdata->name);
	if (rc) {
		dev_err(dev, "Failure reading name, rc = %d\n", rc);
		pdata->name = "lcd-backlight";
	}
	rc = of_property_read_string(dev->of_node, "mode", &str);
	if (!rc) {
		pdata->mode = (strncmp(str, "pwm based", strlen(str)) == 0) ?
			PWM_BASED : REGISTER_BASED;
	} else {
		dev_err(dev, "Failure reading mode, rc = %d\n", rc);
		pdata->mode = REGISTER_BASED;
	}
	rc = of_property_read_string(dev->of_node, "chip_name", &str);
	if (!rc) {
		*id = lp855x_get_i2c_device_id(str);
	} else {
		dev_err(dev, "Failure reading chip_name, rc = %d\n", rc);
		*id = lp855x_get_i2c_device_id("lp8556");
	}
	switch ((*id)->driver_data) {
	default:
	case LP8550:
		pdata->device_control = (pdata->mode == PWM_BASED) ?
			PWM_CONFIG(LP8550) : I2C_CONFIG(LP8550);
		break;
	case LP8551:
		pdata->device_control = (pdata->mode == PWM_BASED) ?
			PWM_CONFIG(LP8551) : I2C_CONFIG(LP8551);
		break;
	case LP8552:
		pdata->device_control = (pdata->mode == PWM_BASED) ?
			PWM_CONFIG(LP8552) : I2C_CONFIG(LP8552);
		break;
	case LP8553:
		pdata->device_control = (pdata->mode == PWM_BASED) ?
			PWM_CONFIG(LP8553) : I2C_CONFIG(LP8553);
		break;
	case LP8556:
		rc = of_property_read_string(dev->of_node, "config", &str);
		if (!rc) {
			if (strncmp(str, "combined1", strlen(str)) == 0)
				pdata->device_control = LP8556_COMB1_CONFIG;
			else if (strncmp(str, "combined2", strlen(str)) == 0)
				pdata->device_control = LP8556_COMB2_CONFIG;
			else
				pdata->device_control = I2C_CONFIG(LP8556);
		} else {
			pdata->device_control = (pdata->mode == PWM_BASED) ?
				PWM_CONFIG(LP8556) : I2C_CONFIG(LP8556);
		}
		break;
	}
	rc = of_property_read_u32(dev->of_node,
		"max_br", &pdata->max_brightness);
	if (rc) {
		dev_err(dev, "Failure reading max_br, rc = %d\n", rc);
		pdata->max_brightness = MAX_BRT;
	}
	if (pdata->max_brightness > MAX_BRT)
		pdata->max_brightness = MAX_BRT;
	rc = of_property_read_u32(dev->of_node,
		"init_br", &pdata->initial_brightness);
	if (rc) {
		dev_err(dev, "Failure reading init_br, rc = %d\n", rc);
		pdata->initial_brightness = INITIAL_BRT;
	}
	if (pdata->initial_brightness > pdata->max_brightness)
		pdata->initial_brightness = pdata->max_brightness;
	rc = of_property_read_u32(dev->of_node, "cfg3_reg", &tmp_data);
	if (!rc) {
		pdata->cfg3 = tmp_data;
	} else {
		dev_err(dev, "Failure reading cfg3_reg, rc = %d\n", rc);
		pdata->cfg3 = 0x5e;
	}
	pdata->gpio_bl_enable = of_get_named_gpio(dev->of_node, "bl_enable", 0);
	if (!gpio_is_valid(pdata->gpio_bl_enable)) {
		dev_err(dev, "%s can't get gpio\n", __func__);
		return -ENODEV;
	}
	node = dev->of_node;
	if (node == NULL)
		return -ENODEV;

	pdata->load_new_rom_data = 0;
	pdata->size_program = 0;
	temp = NULL;
	while ((temp = of_get_next_child(node, temp)))
		++pdata->size_program;

	if (!pdata->size_program)
		return 0;
	pdata->rom_data = kzalloc(
		sizeof(struct lp855x_rom_data) * pdata->size_program,
		GFP_KERNEL);
	if (!pdata->rom_data) {
		dev_err(dev, "Unable to allocate memory\n");
		return 0;
	}
	i = 0;
	for_each_child_of_node(node, temp) {
		rc = of_property_read_u32(temp, "addr", &tmp_data);
		if (rc) {
			dev_err(dev, "Failure reading addr, rc = %d\n", rc);
			continue;
		}
		pdata->rom_data[i].addr = tmp_data;
		rc = of_property_read_u32(temp, "data", &tmp_data);
		if (rc) {
			dev_err(dev, "Failure reading data, rc = %d\n", rc);
			continue;
		}
		pdata->rom_data[i].val = tmp_data;
		++i;
	}
	if (i)
		pdata->load_new_rom_data = 1;
	return 0;
}

static int lp855x_setup(struct device *dev, struct lp855x_platform_data *pdata)
{
	int rc = gpio_request(pdata->gpio_bl_enable, "bl_enable");
	if (rc)
		dev_err(dev, "%s gpio_request failed : %d\n", __func__, rc);
	return rc;
}
static int lp855x_power_on(struct device *dev,
		struct lp855x_platform_data *pdata)
{
	gpio_set_value_cansleep(pdata->gpio_bl_enable, 1);
	return 0;
}
static int lp855x_power_off(struct device *dev,
		struct lp855x_platform_data *pdata)
{
	gpio_set_value_cansleep(pdata->gpio_bl_enable, 0);
	return 0;
}

static int lp855x_probe(struct i2c_client *cl, const struct i2c_device_id *id)
{
	struct lp855x *lp = NULL;
	struct lp855x_platform_data *pdata = NULL;
	const struct i2c_device_id *chip_id;
	int ret;

	lp = kzalloc(sizeof(struct lp855x), GFP_KERNEL);
	pdata = kzalloc(sizeof(struct lp855x_platform_data), GFP_KERNEL);
	if (!lp || !pdata) {
		ret = -ENOMEM;
		dev_err(&cl->dev, "failed to allocate memory\n");
		goto err_mem;
	}
	ret = lp855x_parse_dt(&cl->dev, pdata, &chip_id);
	if (ret)
		goto err_setup;

	ret = lp855x_setup(&cl->dev, pdata);
	if (ret) {
		goto err_setup;
	} else {
		ret = lp855x_power_on(&cl->dev, pdata);
		if (ret)
			goto err_setup;
		else
			usleep_range(LP855x_PWR_DELAY_US,
				LP855x_PWR_DELAY_US + 1000);
	}

	if (!i2c_check_functionality(cl->adapter, I2C_FUNC_SMBUS_I2C_BLOCK)) {
		ret = -EIO;
		dev_err(&cl->dev, "failed to check i2c functionality\n");
		goto err_io;
	}

	lp->client = cl;
	lp->dev = &cl->dev;
	lp->pdata = pdata;
	lp->chipid = chip_id;
	i2c_set_clientdata(cl, lp);
	dev_set_drvdata(&cl->dev, lp);

	mutex_init(&lp->xfer_lock);

	lp855x_init_device(lp);
	ret = lp855x_write_byte(lp, BRIGHTNESS_CTRL, pdata->initial_brightness);
	if (ret) {
		dev_err(lp->dev, "can't set initial brightness (%d)\n" , ret);
		goto err_dev;
	}
	ret = lp855x_backlight_register(lp);
	if (ret) {
		dev_err(lp->dev,
			"can't register backlight device. errcode = %d\n", ret);
		goto err_dev;
	}
	backlight_update_status(lp->bl);
	lp855x_create_debugfs(lp);

	return ret;

err_dev:
err_io:
err_setup:
err_mem:
	if (pdata)
		kfree(pdata->rom_data);
	kfree(pdata);
	kfree(lp);
	return ret;
}

static int __devexit lp855x_remove(struct i2c_client *cl)
{
	struct lp855x *lp = i2c_get_clientdata(cl);

	lp->bl->props.brightness = 0;
	backlight_update_status(lp->bl);
	lp855x_remove_debugfs(lp);
	lp855x_backlight_unregister(lp);
	kfree(lp->pdata->rom_data);
	kfree(lp->pdata);
	kfree(lp);

	return 0;
}

#ifdef CONFIG_PM
static int lp855x_suspend(struct device *dev)
{
	struct lp855x *lp = dev_get_drvdata(dev);
	struct lp855x_platform_data *pdata = lp->pdata;
	int ret = 0;

	dev_dbg(lp->dev, "%s\n", __func__);
	ret = lp855x_power_off(dev, pdata);

	return ret;
}

static int lp855x_resume(struct device *dev)
{
	struct lp855x *lp = dev_get_drvdata(dev);
	struct lp855x_platform_data *pdata = lp->pdata;
	int ret = 0;

	dev_dbg(lp->dev, "%s\n", __func__);
	ret = lp855x_power_on(dev, pdata);
	if (ret)
		goto err;
	else
		usleep_range(LP855x_PWR_DELAY_US,
			LP855x_PWR_DELAY_US + 1000);
	lp855x_init_device(lp);
	backlight_update_status(lp->bl);
err:
	return ret;
}

static const struct dev_pm_ops lp855x_dev_pm_ops = {
	.suspend = lp855x_suspend,
	.resume  = lp855x_resume,
};
#endif

static const struct i2c_device_id lp855x_ids[] = {
	{"lp8550", LP8550},
	{"lp8551", LP8551},
	{"lp8552", LP8552},
	{"lp8553", LP8553},
	{"lp8556", LP8556},
};

static const struct i2c_device_id *lp855x_get_i2c_device_id(const char *chip)
{
	int i;
	for (i = 0; i < sizeof(lp855x_ids)/sizeof(lp855x_ids[0]); ++i) {
		if (strncmp(chip, lp855x_ids[i].name, strlen(chip)) == 0)
			return &lp855x_ids[i];
	}
	return &lp855x_ids[0];
}

static struct of_device_id lp855x_match_table[] = {
	{ .compatible = "Texas Instruments,lp855x_bl", },
	{ },
};

static struct i2c_driver lp855x_driver = {
	.driver = {
		.owner	= THIS_MODULE,
		.name = "lp855x",
		.of_match_table = lp855x_match_table,
#ifdef CONFIG_PM
		.pm	= &lp855x_dev_pm_ops,
#endif
	},
	.probe = lp855x_probe,
	.remove = __devexit_p(lp855x_remove),
	.id_table = lp855x_ids,
};

static int __init lp855x_init(void)
{
	return i2c_add_driver(&lp855x_driver);
}

static void __exit lp855x_exit(void)
{
	i2c_del_driver(&lp855x_driver);
}

module_init(lp855x_init);
module_exit(lp855x_exit);

MODULE_DESCRIPTION("Texas Instruments LP855x Backlight driver");
MODULE_AUTHOR("Milo Kim <milo.kim@ti.com>, Dainel Jeong <daniel.jeong@ti.com>,"\
" Aleksej Makarov <aleksej.makarov@sonymobile.com>");
MODULE_LICENSE("GPL");
