/*
 * TI LP855x Backlight Driver
 *
 * Copyright (C) 2011 Texas Instruments
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */
/*
 * Copyright (C) 2014 Sony Mobile Communications Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation.
 */
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/leds.h>
#include <linux/err.h>
#include <linux/debugfs.h>
#include <linux/uaccess.h>
#include <linux/lp855x.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/of.h>
#include <linux/regulator/consumer.h>
#include <linux/pwm.h>


/* LP8550/1/2/3/6 Registers */
#define LP855X_BRIGHTNESS_CTRL	0x00
#define LP855X_DEVICE_CTRL		0x01
#define LP855X_EEPROM_START		0xA0
#define LP855X_EEPROM_END		0xA7
#define LP8556_EPROM_START		0xA0
#define LP8556_EPROM_END		0xAF

/* LP8557 Registers */
#define LP8557_BL_CMD			0x00
#define LP8557_BL_MASK			0x01
#define LP8557_BL_ON			0x01
#define LP8557_BL_OFF			0x00
#define LP8557_BRTLO			0x03
#define LP8557_BRTHI			0x04
#define LP8557_CONFIG			0x10

#define LP8557_EPROM_START		0x10
#define LP8557_EPROM_END		0x1E

#define LP8557_BRTLO_SHFT	(4)
#define LP8557_BRTLO_MASK	(0xF0)
#define LP8557_BRTHI_MASK	(0xFF)

#define BL_CTL_SHFT		(0)
#define BL_CTL_MASK		(0x01)
#define BRT_MODE_SHFT	(1)
#define BRT_MODE_MASK	(0x06)

#define CFG98_CTRL			0x98
#define IBOOST_LIM_2X_MASK	0x80

#define CFG3_CTRL			0xA3
#define SLOPE_FILTER_MASK	0x7C
#define STEP_CTRL			0x15
#define STEP_SLOPE_FILTER_MASK	0xC3

/* Enable backlight. Only valid when BRT_MODE=10(I2C only) */
#define ENABLE_BL	(1)
#define DISABLE_BL	(0)

#define I2C_CONFIG(id)	id ## _I2C_CONFIG
#define PWM_CONFIG(id)	id ## _PWM_CONFIG

/* DEVICE CONTROL register - LP8550 */
#define LP8550_PWM_CONFIG	(LP8550_PWM_ONLY << BRT_MODE_SHFT)
#define LP8550_I2C_CONFIG	((ENABLE_BL << BL_CTL_SHFT) | \
				(LP8550_I2C_ONLY << BRT_MODE_SHFT))

/* DEVICE CONTROL register - LP8551 */
#define LP8551_PWM_CONFIG	LP8550_PWM_CONFIG
#define LP8551_I2C_CONFIG	LP8550_I2C_CONFIG

/* DEVICE CONTROL register - LP8552 */
#define LP8552_PWM_CONFIG	LP8550_PWM_CONFIG
#define LP8552_I2C_CONFIG	LP8550_I2C_CONFIG

/* DEVICE CONTROL register - LP8553 */
#define LP8553_PWM_CONFIG	LP8550_PWM_CONFIG
#define LP8553_I2C_CONFIG	LP8550_I2C_CONFIG

/* DEVICE CONTROL register - LP8556 */
#define LP8556_PWM_CONFIG	(LP8556_PWM_ONLY << BRT_MODE_SHFT)
#define LP8556_COMB1_CONFIG	(LP8556_COMBINED1 << BRT_MODE_SHFT)
#define LP8556_I2C_CONFIG	((ENABLE_BL << BL_CTL_SHFT) | \
				(LP8556_I2C_ONLY << BRT_MODE_SHFT))
#define LP8556_COMB2_CONFIG	(LP8556_COMBINED2 << BRT_MODE_SHFT)
#define LP8556_FAST_CONFIG	BIT(7) /* use it if EPROMs should be maintained
					  when exiting the low power mode */

/* DEVICE CONTROL- FAST register - LP8556 */
#define FAST_SHFT		7
#define LP8556_FAST		1
#define LP8556_FAST_CTRL	(LP8556_FAST << FAST_SHFT)
#define LP8556_FAST_MASK	0x80

/* CONFIG register - LP8557 */
#define LP8557_PWM_STANDBY	BIT(7)
#define LP8557_PWM_FILTER	BIT(6)
#define LP8557_RELOAD_EPROM	BIT(3)	/* use it if EPROMs should be reset
					   when the backlight turns on */
#define LP8557_OFF_OPENLEDS	BIT(2)

#define LP8557_PWM_CONFIG	LP8557_PWM_ONLY
#define LP8557_I2C_CONFIG	LP8557_I2C_ONLY
#define LP8557_COMB1_CONFIG	LP8557_COMBINED1
#define LP8557_COMB2_CONFIG	LP8557_COMBINED2

#define LP855x_PWR_DELAY_US	12000
#define LP855x_LED_BUFF_SIZE 50

enum lp855x_chip_id {
	LP8550,
	LP8551,
	LP8552,
	LP8553,
	LP8556,
	LP8557,
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
	LP8556_COMBINED1,	/* pwm + i2c before the shaper block */
	LP8556_I2C_ONLY,
	LP8556_COMBINED2,	/* pwm + i2c after the shaper block */
};

enum lp8557_brightness_source {
	LP8557_PWM_ONLY,
	LP8557_I2C_ONLY,
	LP8557_COMBINED1,	/* pwm + i2c after the shaper block */
	LP8557_COMBINED2,	/* pwm + i2c before the shaper block */
};

#ifdef CONFIG_DEBUG_FS
struct debug_dentry {
	struct dentry *dir;
	struct dentry *reg;
	struct dentry *chip;
	struct dentry *blmode;
};
#endif

enum lp855x_brightness_ctrl_mode {
	PWM_BASED = 1,
	REGISTER_BASED,
};

struct lp855x;

/*
 * struct lp855x_device_config
 * @is_8bit_brightness: flag for check 8 bit or 12 bit brightness
 * @reg_slope: register address for slope
 * @reg_slope_mask: register mask value for slope
 * @reg_devicectrl: register address for devicectrl
 * @reg_devicectrl_mask: register mask value for devicectrl
 * @pre_init_device: init device function call before updating the brightness
 * @post_init_device: late init device function call
 * @resume_init: init the register when resume status
 */
struct lp855x_device_config {
	bool is_8bit_brightness;
	u8 reg_slope;
	u8 reg_slope_mask;
	u8 reg_devicectrl;
	u8 reg_devicectrl_mask;
	u8 reg_brt;
	int (*pre_init_device)(struct lp855x *);
	int (*post_init_device)(struct lp855x *);
	int (*resume_init)(struct lp855x *);
};

struct lp855x {
	const struct i2c_device_id *chipid;
	enum lp855x_brightness_ctrl_mode mode;
	struct lp855x_device_config *cfg;
	struct i2c_client *client;
	struct led_classdev cdev;
	struct device *dev;
	struct work_struct work;
	spinlock_t lock;
	struct lp855x_platform_data *pdata;
	struct pwm_device *pwm;
	int bl_scale;
	int stored_br;
#ifdef CONFIG_DEBUG_FS
	struct debug_dentry dd;
#endif
};


static bool bl_on_in_boot;
static int __init continous_splash_setup(char *str)
{
	if (!str)
		return 0;
	if (!strncmp(str, "on", 2))
		bl_on_in_boot = true;
	return 0;
}
__setup("display_status=", continous_splash_setup);

static int lp855x_write_byte(struct lp855x *lp, u8 reg, u8 data)
{
	return i2c_smbus_write_byte_data(lp->client, reg, data);
}

static int lp855x_write_brightness(struct lp855x *lp,
				bool is_8bit, int brightness)
{
	int ret;
	u8 val;

	if ((lp->bl_scale > 0) && (lp->bl_scale < 100))
		brightness = (brightness * lp->bl_scale) / 100;
	if (is_8bit) {
		ret = lp855x_write_byte(lp,
			lp->cfg->reg_brt, (u8)brightness);
	} else {
		val = (u8)((brightness << LP8557_BRTLO_SHFT)
			& LP8557_BRTLO_MASK);
		ret = lp855x_write_byte(lp, LP8557_BRTLO, val);
		val = (u8)((brightness >> LP8557_BRTLO_SHFT)
			& LP8557_BRTHI_MASK);
		ret |= lp855x_write_byte(lp, LP8557_BRTHI, val);
	}
	if (ret)
		dev_err(lp->dev, "%s:write brightness is failed!\n", __func__);

	return ret;
}

static int lp855x_update_bit(struct lp855x *lp, u8 reg, u8 mask, u8 data)
{
	int ret;
	u8 tmp;

	ret = i2c_smbus_read_byte_data(lp->client, reg);
	if (ret < 0) {
		dev_err(lp->dev, "failed to read 0x%.2x\n", reg);
		return ret;
	}

	tmp = (u8)ret;
	tmp &= ~mask;
	tmp |= data & mask;

	return lp855x_write_byte(lp, reg, tmp);
}

static inline int lp855x_read_byte(struct lp855x *lp, u8 reg, u8 *data)
{
	int ret;

	ret = i2c_smbus_read_byte_data(lp->client, reg);
	if (ret < 0) {
		dev_err(lp->dev, "failed to read 0x%.2x\n", reg);
		return ret;
	}

	*data = (u8)ret;
	return 0;
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
	pos = lp855x_parse_register_cmd(buf, count, &arg1);
	if (!pos)
		return -EINVAL;
	switch (mode) {
	case 'r':
		rc = lp855x_read_byte(lp, arg1, &val);
		dev_info(lp->dev, "Read [0x%.2x] = 0x%.2x\n", arg1, val);
		break;
	case 'w':
		offs = pos - buf;
		pos2 = lp855x_parse_register_cmd(pos + 1, count - offs, &arg2);
		if (!pos2)
			return -EINVAL;

		rc = lp855x_write_byte(lp, arg1, arg2);
		dev_info(lp->dev, "Written [0x%.2x] = 0x%.2x\n", arg1, arg2);
		break;
	case 'd':
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
	enum lp855x_brightness_ctrl_mode mode = lp->mode;

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
		rc = addr >= LP855X_EEPROM_START && addr <= LP855X_EEPROM_END;
		break;
	case LP8556:
		rc = addr >= LP8556_EPROM_START && addr <= LP8556_EPROM_END;
		break;
	case LP8557:
		rc = addr >= LP8557_EPROM_START && addr <= LP8557_EPROM_END;
		break;
	default:
		rc = false;
		break;
	}
	return rc;
}

static int lp8557_bl_off(struct lp855x *lp)
{
	int ret = 0;
	u8 val;
	/* BL_ON = 0 before updating EPROM settings */
	ret = lp855x_update_bit(lp, LP8557_BL_CMD, LP8557_BL_MASK,
			LP8557_BL_OFF);
	if (ret) {
		dev_err(lp->dev, "set BL_ON = 0 err: %d\n", ret);
		goto exit;
	}
	val = lp->pdata->device_control;
	ret = lp855x_write_byte(lp, LP8557_CONFIG, val);
	if (ret) {
		dev_err(lp->dev, "set LP8557_CONFIG err: %d\n", ret);
	}
exit:
	return ret;
}

static int lp8557_bl_on(struct lp855x *lp)
{
	/* BL_ON = 1 after updating EPROM settings */
	return lp855x_update_bit(lp, LP8557_BL_CMD, LP8557_BL_MASK,
				LP8557_BL_ON);
}

static int lp855x_bl_off(struct lp855x *lp)
{
	u8 val = lp->pdata->device_control;
	if (lp->pdata->device_control & LP8556_FAST_MASK)
		val &= ~(ENABLE_BL << BL_CTL_SHFT);
	return lp855x_write_byte(lp, LP855X_DEVICE_CTRL, val);
}

static int lp855x_bl_on(struct lp855x *lp)
{
	u8 val;
	if (lp->pdata->device_control & LP8556_FAST_MASK) {
		val = lp->pdata->device_control;
		val |= (ENABLE_BL << BL_CTL_SHFT);
		return lp855x_write_byte(lp, LP855X_DEVICE_CTRL, val);
	}
	return 0;
}

static int lp855x_resume_init(struct lp855x *lp)
{
	u8 val, addr;
	int i, ret = 0;

	if (lp->cfg->pre_init_device) {
		ret = lp->cfg->pre_init_device(lp);
		if (ret) {
			dev_err(lp->dev, "pre init device err: %d\n", ret);
			goto exit;
		}
	}

	if (lp->pdata->size_program > 0) {
		for (i = 0; i < lp->pdata->size_program; i++) {
			addr = lp->pdata->rom_data[i].addr;
			val = lp->pdata->rom_data[i].val;
			if (!lp855x_is_valid_rom_area(lp, addr))
				continue;

			if (addr == CFG98_CTRL) {
				u8 cfg98;
				lp855x_read_byte(lp,
					CFG98_CTRL, &cfg98);
				cfg98 &= ~IBOOST_LIM_2X_MASK;
				val = cfg98 |
					(val & IBOOST_LIM_2X_MASK);
			}
			ret = lp855x_write_byte(lp, addr, val);
			if (ret) {
				dev_err(lp->dev, "set register err: %d\n", ret);
			goto exit;
			}
		}
	}
	ret = lp855x_write_byte(lp, lp->cfg->reg_slope, lp->pdata->slope_reg);
	if (ret) {
		dev_err(lp->dev, "i2c write err\n");
		goto exit;
	}

	if (lp->cfg->post_init_device) {
		ret = lp->cfg->post_init_device(lp);
		if (ret) {
			dev_err(lp->dev, "post init device err: %d\n", ret);
			goto exit;
		}
	}
exit:
	return ret;
}

static int lp855x_check_need_config(struct lp855x *lp, bool *need_config_flg)
{
	int ret = 0;
	u8 val;

	if (!bl_on_in_boot) {
		*need_config_flg = true;
		goto exit;
	}
	ret = lp855x_read_byte(lp, lp->cfg->reg_devicectrl, &val);
	if (ret) {
		dev_err(lp->dev,
			"Read ctrl register[0x%.04x] err: %d\n", lp->cfg->reg_devicectrl, ret);
		goto exit;
	}
	if (val & lp->cfg->reg_devicectrl_mask)
		*need_config_flg = false;
	else
		*need_config_flg = true;
exit:
	return ret;
}

static struct lp855x_device_config lp855x_dev_cfg = {
	.is_8bit_brightness = true,
	.reg_slope = CFG3_CTRL,
	.reg_slope_mask = SLOPE_FILTER_MASK,
	.reg_devicectrl = LP855X_DEVICE_CTRL,
	.reg_devicectrl_mask = BL_CTL_MASK,
	.reg_brt = LP855X_BRIGHTNESS_CTRL,
	.pre_init_device = lp855x_bl_off,
	.post_init_device = lp855x_bl_on,
	.resume_init = lp855x_resume_init,
};

static struct lp855x_device_config lp8557_dev_cfg = {
	.is_8bit_brightness = true,
	.reg_slope = STEP_CTRL,
	.reg_slope_mask = STEP_SLOPE_FILTER_MASK,
	.reg_devicectrl = LP8557_BL_CMD,
	.reg_devicectrl_mask = LP8557_BL_MASK,
	.reg_brt = LP8557_BRTHI,
	.pre_init_device = lp8557_bl_off,
	.post_init_device = lp8557_bl_on,
	.resume_init = NULL,
};

/*
 * Device specific configuration flow
 *
 *    a) pre_init_device(optional)
 *    b) update the brightness register
 *    c) update ROM area(optional)
 *    d) post_init_device(optional)
 *
 */
static int lp855x_configure(struct lp855x *lp)
{
	u8 val, addr;
	bool need_config;
	int i, ret, brightness;
	struct lp855x_platform_data *pd = lp->pdata;

	switch (lp->chipid->driver_data) {
	case LP8550 ... LP8556:
		lp->cfg = &lp855x_dev_cfg;
		break;
	case LP8557:
		lp->cfg = &lp8557_dev_cfg;
		break;
	default:
		ret = -EINVAL;
		dev_err(lp->dev, "device support err: %d\n", ret);
		goto exit;
	}

	ret = lp855x_check_need_config(lp, &need_config);
	if (ret || !need_config)
		goto exit;

	if (lp->cfg->pre_init_device) {
		ret = lp->cfg->pre_init_device(lp);
		if (ret) {
			dev_err(lp->dev, "pre init device err: %d\n", ret);
			goto exit;
		}
	}

	brightness = pd->initial_brightness;
	ret = lp855x_write_brightness(lp,
		lp->cfg->is_8bit_brightness, brightness);
	if (ret) {
		dev_err(lp->dev, "set initial brightness err: %d\n", ret);
		goto exit;
	}

	if (pd->size_program > 0) {
		for (i = 0; i < pd->size_program; i++) {
			addr = pd->rom_data[i].addr;
			val = pd->rom_data[i].val;
			if (!lp855x_is_valid_rom_area(lp, addr))
				continue;

			if (addr == CFG98_CTRL) {
				u8 cfg98;
				lp855x_read_byte(lp,
					CFG98_CTRL, &cfg98);
				cfg98 &= ~IBOOST_LIM_2X_MASK;
				val = cfg98 |
					(val & IBOOST_LIM_2X_MASK);
			}
			ret = lp855x_write_byte(lp, addr, val);
			if (ret) {
				dev_err(lp->dev, "set register err: %d\n", ret);
				goto exit;
			}
		}
	}
	ret = lp855x_write_byte(lp, lp->cfg->reg_slope, pd->slope_reg);
	if (ret) {
		dev_err(lp->dev, "i2c write err\n");
		goto exit;
	}

	if (lp->cfg->post_init_device) {
		ret = lp->cfg->post_init_device(lp);
		if (ret) {
			dev_err(lp->dev, "post init device err: %d\n", ret);
			goto exit;
		}
	}
exit:
	return ret;
}

static void lp855x_pwm_ctrl(struct lp855x *lp, int br, int max_br)
{
	unsigned int period = lp->pdata->period_ns;
	unsigned int duty = br * period / max_br;
	struct pwm_device *pwm;

	/* request pwm device with the consumer name */
	if (!lp->pwm) {
		pwm = devm_pwm_get(lp->dev, lp->chipid->name);
		if (IS_ERR(pwm))
			return;

		lp->pwm = pwm;
	}

	pwm_config(lp->pwm, duty, period);
	if (duty)
		pwm_enable(lp->pwm);
	else
		pwm_disable(lp->pwm);
}

static int lp855x_set_slope_filter(struct lp855x *lp, bool enable)
{
	u8 val = lp->pdata->slope_reg;

	if (!enable)
		val &= ~lp->cfg->reg_slope_mask;

	return lp855x_write_byte(lp, lp->cfg->reg_slope, val);
}

static void lp855x_led_work(struct work_struct *work)
{
	struct lp855x *lp = container_of(work, struct lp855x, work);
	enum lp855x_brightness_ctrl_mode mode;
	int br;

	br = lp->cdev.brightness;
	mode = lp->mode;

	if (!br && lp->stored_br)
		(void)lp855x_set_slope_filter(lp, false);
	else if (br && !lp->stored_br)
		(void)lp855x_set_slope_filter(lp, true);
	lp->stored_br = br;

	if (mode == PWM_BASED) {
		int max_br = lp->cdev.max_brightness;

		lp855x_pwm_ctrl(lp, br, max_br);

	} else if (mode == REGISTER_BASED) {
		lp855x_write_brightness(lp, lp->cfg->is_8bit_brightness, br);
	}
}

static void lp855x_led_set(struct led_classdev *led_cdev,
				enum led_brightness value)
{
	unsigned long flags;
	struct lp855x *lp = container_of(led_cdev, struct lp855x, cdev);

	if (!lp)
		return;
	spin_lock_irqsave(&lp->lock, flags);
	lp->cdev.brightness = value;
	schedule_work(&lp->work);
	spin_unlock_irqrestore(&lp->lock, flags);
}

static ssize_t lp855x_led_bl_scale_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct lp855x *lp = container_of(led_cdev, struct lp855x, cdev);
	dev_dbg(lp->dev, "%s: bl_scale = %d\n", __func__, lp->bl_scale);

	return snprintf(buf, LP855x_LED_BUFF_SIZE, "%u\n", lp->bl_scale);
}

static ssize_t lp855x_led_bl_scale_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct lp855x *lp = container_of(led_cdev, struct lp855x, cdev);
	unsigned long scale = 0;
	ssize_t ret = -EINVAL;

	ret = kstrtoul(buf, 10, &scale);
	if (!ret) {
		ret = size;
		if (scale > 100)
			scale = 100;
		lp->bl_scale = scale;
		dev_dbg(lp->dev, "%s: bl_scale = %d\n", __func__, lp->bl_scale);
	} else {
		dev_err(lp->dev, "Failure to set sysfs bl_scale.\n");
		ret = -EINVAL;
	}

	return ret;
}

static struct device_attribute lp855x_led_attrs[] = {
	__ATTR(bl_scale, (S_IRUGO | S_IWUSR | S_IWGRP),
			lp855x_led_bl_scale_show,
			lp855x_led_bl_scale_store),
};

static int lp855x_led_class_register(struct lp855x *lp)
{
	int ret;
	lp->cdev.name = lp->pdata->name;
	lp->cdev.default_trigger = lp->pdata->default_trigger;
	lp->cdev.brightness = lp->pdata->initial_brightness;
	lp->cdev.max_brightness = lp->pdata->max_brightness;
	lp->cdev.brightness_set = lp855x_led_set;
	ret = led_classdev_register(lp->dev, &lp->cdev);
	if (ret)
		dev_err(lp->dev, "unable to register rgb %d\n", ret);
	return ret;
}

static void lp855x_led_class_unregister(struct lp855x *lp)
{
	led_classdev_unregister(&lp->cdev);
}

#define INITIAL_BRT	0x7FF
#define MAX_BRT	0xFFF
static const struct i2c_device_id *lp855x_get_i2c_device_id(const char *chip);

static int lp855x_parse_dt(struct device *dev,
		struct lp855x *lp,
		const struct i2c_device_id **id)
{
	int i, rc;
	const char *str;
	struct device_node *node, *temp;
	u32 tmp_data;
	lp->pdata->bl_enable_valid = true;

	rc = of_property_read_string(dev->of_node,
		"linux,name", &lp->pdata->name);
	if (rc) {
		dev_err(dev, "Failure reading name, rc = %d\n", rc);
		lp->pdata->name = "lcd-backlight";
	}
	rc = of_property_read_string(dev->of_node, "linux,default-trigger",
		&str);
	if (!rc) {
		lp->pdata->default_trigger = str;
	} else {
		dev_err(dev, "Failure reading default-trigger, rc = %d\n", rc);
		lp->pdata->default_trigger = "none";
	}
	rc = of_property_read_string(dev->of_node, "mode", &str);
	if (!rc) {
		lp->mode = (strncmp(str, "pwm based", strlen(str)) == 0) ?
			PWM_BASED : REGISTER_BASED;
	} else {
		dev_err(dev, "Failure reading mode, rc = %d\n", rc);
		lp->mode = REGISTER_BASED;
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
		lp->pdata->device_control = (lp->mode == PWM_BASED) ?
			PWM_CONFIG(LP8550) : I2C_CONFIG(LP8550);
		break;
	case LP8551:
		lp->pdata->device_control = (lp->mode == PWM_BASED) ?
			PWM_CONFIG(LP8551) : I2C_CONFIG(LP8551);
		break;
	case LP8552:
		lp->pdata->device_control = (lp->mode == PWM_BASED) ?
			PWM_CONFIG(LP8552) : I2C_CONFIG(LP8552);
		break;
	case LP8553:
		lp->pdata->device_control = (lp->mode == PWM_BASED) ?
			PWM_CONFIG(LP8553) : I2C_CONFIG(LP8553);
		break;
	case LP8556:
		rc = of_property_read_string(dev->of_node, "config", &str);
		if (!rc) {
			if (strncmp(str, "combined1", strlen(str)) == 0)
				lp->pdata->device_control = LP8556_COMB1_CONFIG;
			else if (strncmp(str, "combined2", strlen(str)) == 0)
				lp->pdata->device_control = LP8556_COMB2_CONFIG;
			else
				lp->pdata->device_control = I2C_CONFIG(LP8556);
		} else {
			lp->pdata->device_control = (lp->mode == PWM_BASED) ?
				PWM_CONFIG(LP8556) : I2C_CONFIG(LP8556);
			if (of_property_read_bool(dev->of_node, "fast_ctrl"))
				lp->pdata->device_control |= (LP8556_FAST_CTRL);
		}
		break;
	case LP8557:
		rc = of_property_read_u32(dev->of_node,
			"dev-ctrl", &tmp_data);
		if (rc) {
			dev_err(dev, "Failure reading dev-ctrl, rc = %d\n", rc);
			lp->pdata->device_control = (lp->mode == PWM_BASED) ?
				PWM_CONFIG(LP8557) : I2C_CONFIG(LP8557);
		} else {
			lp->pdata->device_control = (u8)tmp_data;
		}
		rc = of_property_read_u32(dev->of_node,
			"pwm-period", &lp->pdata->period_ns);
		if (rc) {
			dev_err(dev,
				"Failure reading pwm-period, rc = %d\n",
				rc);
			lp->pdata->period_ns = (lp->mode == PWM_BASED) ? 80 : 0;
		}
		lp->pdata->bl_enable_valid = false;
		break;
	}
	rc = of_property_read_u32(dev->of_node,
		"max_br", &lp->pdata->max_brightness);
	if (rc) {
		dev_err(dev, "Failure reading max_br, rc = %d\n", rc);
		lp->pdata->max_brightness = MAX_BRT;
	}
	if (lp->pdata->max_brightness > MAX_BRT)
		lp->pdata->max_brightness = MAX_BRT;

	rc = of_property_read_u32(dev->of_node,
			"init_br", &lp->pdata->initial_brightness);
	if (rc) {
		dev_err(dev, "Failure reading init_br, rc = %d\n", rc);
		lp->pdata->initial_brightness = INITIAL_BRT;
	}

	if (lp->pdata->initial_brightness > lp->pdata->max_brightness)
		lp->pdata->initial_brightness = lp->pdata->max_brightness;

	rc = of_property_read_u32(dev->of_node, "slope_reg", &tmp_data);
	if (!rc) {
		lp->pdata->slope_reg = tmp_data;
	} else {
		dev_err(dev,
			"Failure reading cfg3_reg, rc = %d\n",
			rc);
		lp->pdata->slope_reg = 0x5e;
	}
	if (lp->pdata->bl_enable_valid) {
		lp->pdata->gpio_bl_enable = of_get_named_gpio(dev->of_node,
			"bl_enable", 0);
		if (!gpio_is_valid(lp->pdata->gpio_bl_enable)) {
			dev_err(dev, "%s can't get gpio\n", __func__);
			return -ENODEV;
		}
	}

	node = dev->of_node;
	if (node == NULL)
		return -ENODEV;

	lp->pdata->load_new_rom_data = 0;
	temp = NULL;
	lp->pdata->size_program = of_get_child_count(node);

	if (!lp->pdata->size_program)
		return 0;
	lp->pdata->rom_data = kzalloc(
		sizeof(struct lp855x_rom_data) * lp->pdata->size_program,
		GFP_KERNEL);
	if (!lp->pdata->rom_data) {
		dev_err(dev, "Unable to allocate memory\n");
		return 0;
	}
	i = 0;
	for_each_child_of_node(node, temp) {
		rc = of_property_read_u32(temp, "addr", &tmp_data);
		if (rc) {
			dev_err(dev,
				"Failure reading addr, rc = %d\n",
				rc);
			continue;
		}
		lp->pdata->rom_data[i].addr = tmp_data;
		rc = of_property_read_u32(temp, "data", &tmp_data);
		if (rc) {
			dev_err(dev,
				"Failure reading data, rc = %d\n",
				rc);
			continue;
		}
		lp->pdata->rom_data[i].val = tmp_data;
		++i;
	}
	if (i)
		lp->pdata->load_new_rom_data = 1;
	return 0;
}

static int lp855x_setup(struct device *dev, struct lp855x_platform_data *pdata)
{
	int rc;
	if (!pdata->bl_enable_valid)
		return 0;
	rc = gpio_request(pdata->gpio_bl_enable, "bl_enable");
	if (rc)
		dev_err(dev, "%s gpio_request failed : %d\n", __func__, rc);
	return rc;
}

static int lp855x_power_on(struct lp855x *lp,
		struct lp855x_platform_data *pdata)
{
	int ret;
	if (!lp) {
		dev_err(lp->dev, "Failure power on\n");
		return -EINVAL;
	}
	if (!pdata->bl_enable_valid) {
		ret = lp8557_bl_on(lp);
		if (ret)
			dev_err(lp->dev, "Failure power on, ret = %d\n", ret);
	} else {
		gpio_set_value_cansleep(pdata->gpio_bl_enable, 1);
	}
	return 0;
}
static int lp855x_power_off(struct lp855x *lp,
		struct lp855x_platform_data *pdata)
{
	int ret;
	if (!lp) {
		dev_err(lp->dev, "Failure power off\n");
		return -EINVAL;
	}
	if (!pdata->bl_enable_valid) {
		ret = lp8557_bl_off(lp);
		if (ret)
			dev_err(lp->dev, "Failure power off, ret = %d\n", ret);
	} else {
		gpio_set_value_cansleep(pdata->gpio_bl_enable, 0);
	}
	return 0;
}

static int lp855x_probe(struct i2c_client *cl, const struct i2c_device_id *id)
{
	struct lp855x *lp = NULL;
	struct lp855x_platform_data *pdata = NULL;
	const struct i2c_device_id *chip_id;
	int ret, i;

	lp = kzalloc(sizeof(struct lp855x), GFP_KERNEL);
	pdata = kzalloc(sizeof(struct lp855x_platform_data), GFP_KERNEL);
	if (!lp || !pdata) {
		ret = -ENOMEM;
		dev_err(&cl->dev, "failed to allocate memory\n");
		goto err_mem;
	}
	lp->pdata = pdata;
	ret = lp855x_parse_dt(&cl->dev, lp, &chip_id);
	if (ret)
		goto err_setup;

	lp->client = cl;
	ret = lp855x_setup(&cl->dev, pdata);
	if (ret) {
		goto err_setup;
	} else {
		ret = lp855x_power_on(lp, pdata);
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

	lp->dev = &cl->dev;
	lp->chipid = chip_id;
	i2c_set_clientdata(cl, lp);
	dev_set_drvdata(&cl->dev, lp);

	spin_lock_init(&lp->lock);
	INIT_WORK(&lp->work, lp855x_led_work);

	ret = lp855x_configure(lp);
	if (ret) {
		dev_err(lp->dev, "Failure to configure. errcode = %d\n", ret);
		goto err_dev;
	}
	ret = lp855x_led_class_register(lp);
	if (ret) {
		dev_err(lp->dev, "can't register. errcode = %d\n", ret);
		goto err_dev;
	}

	for (i = 0; i < ARRAY_SIZE(lp855x_led_attrs); i++) {
		ret = sysfs_create_file(&lp->cdev.dev->kobj,
					&lp855x_led_attrs[i].attr);
		if (ret < 0) {
			dev_err(lp->dev, "sysfs creation failed\n");
			goto sysfs_fail;
		}
	}

	lp855x_create_debugfs(lp);
	return ret;
sysfs_fail:
	for (i--; i >= 0; i--)
		sysfs_remove_file(&lp->cdev.dev->kobj,
			&lp855x_led_attrs[i].attr);
	led_classdev_unregister(&lp->cdev);
err_dev:
err_io:
err_setup:
err_mem:
	dev_err(lp->dev, "Finish lp855x_probe by error. errcode = %d\n", ret);
	if (pdata)
		kfree(pdata->rom_data);
	kfree(pdata);
	kfree(lp);
	return ret;
}

static int lp855x_remove(struct i2c_client *cl)
{
	struct lp855x *lp = i2c_get_clientdata(cl);
	int i;

	lp855x_remove_debugfs(lp);
	for (i = 0; i < ARRAY_SIZE(lp855x_led_attrs); i++)
		sysfs_remove_file(&lp->cdev.dev->kobj,
				&lp855x_led_attrs[i].attr);
	lp855x_led_class_unregister(lp);
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
	ret = lp855x_power_off(lp, pdata);

	return ret;
}

static int lp855x_resume(struct device *dev)
{
	struct lp855x *lp = dev_get_drvdata(dev);
	struct lp855x_platform_data *pdata = lp->pdata;
	int ret = 0;

	dev_dbg(lp->dev, "%s\n", __func__);
	ret = lp855x_power_on(lp, pdata);
	if (ret)
		goto err;
	else
		usleep_range(LP855x_PWR_DELAY_US,
			LP855x_PWR_DELAY_US + 1000);
	if (lp->cfg->resume_init) {
		ret = lp->cfg->resume_init(lp);
		if (ret) {
			dev_err(lp->dev,
				"Failure resume init, ret = %d\n", ret);
			goto err;
		}
	}
err:
	return ret;
}

static const struct dev_pm_ops lp855x_dev_pm_ops = {
	.suspend = lp855x_suspend,
	.resume  = lp855x_resume,
};
#endif

static const struct of_device_id lp855x_dt_ids[] = {
	{ .compatible = "ti,lp8550", },
	{ .compatible = "ti,lp8551", },
	{ .compatible = "ti,lp8552", },
	{ .compatible = "ti,lp8553", },
	{ .compatible = "ti,lp8556", },
	{ .compatible = "ti,lp8557", },
	{ }
};

static const struct i2c_device_id lp855x_ids[] = {
	{"lp8550", LP8550},
	{"lp8551", LP8551},
	{"lp8552", LP8552},
	{"lp8553", LP8553},
	{"lp8556", LP8556},
	{"lp8557", LP8557},
	{ }
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

static struct i2c_driver lp855x_driver = {
	.driver = {
		.owner	= THIS_MODULE,
		.name = "lp855x",
		.of_match_table = of_match_ptr(lp855x_dt_ids),
#ifdef CONFIG_PM
		.pm	= &lp855x_dev_pm_ops,
#endif
	},
	.probe = lp855x_probe,
	.remove = lp855x_remove,
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
MODULE_AUTHOR("Milo Kim <milo.kim@ti.com>, Aleksej Makarov <aleksej.makarov@sonymobile.com>");
MODULE_LICENSE("GPL");
