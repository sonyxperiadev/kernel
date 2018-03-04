/* drivers/video/msm/msm_dba/sil9612.c
 *
 * Author: Yasufumi Hironaka <Yasufumi.X.Hironaka@sonymobile.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */
/*
 * Copyright (C) 2016 Sony Mobile Communications Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation.
 */

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/of_gpio.h>
#include <linux/of_irq.h>
#include <linux/pm.h>
#include <linux/pm_runtime.h>
#include <linux/mdss_io_util.h>

#include <linux/stat.h>
#include <linux/fcntl.h>
#include <linux/unistd.h>

#include <linux/fb.h>
#include <linux/vt_kern.h>
#include <linux/unistd.h>
#include <linux/switch.h>
#include <linux/syscalls.h>

#include "sil9612.h"

static u8 reg_r_addr[2];
static int reg_r_size;
static int reg_w_addr;
static wait_queue_head_t irq_wait_q;
static bool irq_flag;
static int irq_timeout;

static int sil9612_i2c_read(struct i2c_client *client,
			u8 addr, u8 *reg, u8 *read_buf, u8 size)
{
	int rc = 0;
	struct i2c_msg msg[2];

	if (!client) {
		pr_err("%s: Invalid params\n", __func__);
		return -EINVAL;
	}

	client->addr = addr;

	msg[0].addr  = addr;
	msg[0].flags = 0;
	msg[0].len   = 2;
	msg[0].buf   = reg;

	msg[1].addr  = addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len   = size;
	msg[1].buf   = read_buf;

	rc = i2c_transfer(client->adapter, msg, 2);
	if (rc != 2) {
		pr_err("%s: i2c read failed\n", __func__);
		goto exit;
	}

	switch (size) {
	case 1:
		pr_debug("%s: [%s:0x%02x] : R[0x%02x%02x, 0x%02x]\n",
			__func__, client->name, addr,
			reg[0], reg[1], read_buf[0]);
		break;
	case 2:
		pr_debug("%s: [%s:0x%02x] : R[0x%02x%02x, 0x%02x%02x]\n",
			__func__, client->name, addr, reg[0],
			reg[1], read_buf[1], read_buf[0]);
		break;
	case 3:
		pr_debug("%s: [%s:0x%02x] : R[0x%02x%02x, 0x%02x%02x%02x]\n",
			__func__, client->name, addr, reg[0],
			reg[1], read_buf[2], read_buf[1], read_buf[0]);
		break;
	case 4:
	default:
		pr_debug("%s: [%s:0x%02x] : R[0x%02x%02x, 0x%02x%02x%02x%02x]\n",
			__func__, client->name, addr, reg[0], reg[1],
			read_buf[3], read_buf[2], read_buf[1], read_buf[0]);
		break;
	}

exit:
	return rc;
}

static int sil9612_i2c_write(struct i2c_client *client,
			u8 addr, u16 reg, u8 size, u64 val)
{
	int rc = 0;
	struct i2c_msg msg;
	u8 wbuf[8+2];

	if (!client) {
		pr_err("%s: Invalid params\n", __func__);
		return -EINVAL;
	}
	pr_debug("%s: [%s:0x%02x]\n", __func__,  client->name, addr);

	client->addr = addr;

	wbuf[0] = reg >> 8;
	wbuf[1] = reg & 0xFF;
	pr_debug("%s: size[%d], val[0x%08x%08x]\n",
		__func__, size,
		(unsigned int)((val >> 32) & 0xFFFFFFFF),
		(unsigned int)(val & 0xFFFFFFFF));
	switch (size) {
	case 1:
		wbuf[2] = val & 0xFF;
		pr_debug("%s: reg[0x%04x], wbuf[0x%02x]\n",
				__func__, reg, wbuf[2]);
		break;
	case 2:
		wbuf[2] = val & 0xFF;
		wbuf[3] = (val >> 8) & 0xFF;
		pr_debug("%s: reg[0x%04x], val[0x%02x%02x]->swap->wbuf[0x%02x%02x]\n",
			__func__, reg, wbuf[3], wbuf[2], wbuf[2], wbuf[3]);
		break;
	case 3:
		wbuf[2] = val & 0xFF;
		wbuf[3] = (val >>  8) & 0xFF;
		wbuf[4] = (val >> 16) & 0xFF;
		pr_debug("%s: reg[0x%04x], val[0x%02x%02x%02x]->swap->wbuf[0x%02x%02x%02x]\n",
			__func__, reg, wbuf[4], wbuf[3], wbuf[2],
			wbuf[2], wbuf[3], wbuf[4]);
		break;
	case 4:
		wbuf[2] = val & 0xFF;
		wbuf[3] = (val >>  8) & 0xFF;
		wbuf[4] = (val >> 16) & 0xFF;
		wbuf[5] = (val >> 24) & 0xFF;
		pr_debug("%s: reg[0x%04x], val[0x%02x%02x%02x%02x]->swap->wbuf[0x%02x%02x%02x%02x]\n",
			__func__, reg, wbuf[5], wbuf[4], wbuf[3],
			wbuf[2], wbuf[2], wbuf[3], wbuf[4], wbuf[5]);
		break;
	case 5:
		wbuf[2] = val & 0xFF;
		wbuf[3] = (val >>  8) & 0xFF;
		wbuf[4] = (val >> 16) & 0xFF;
		wbuf[5] = (val >> 24) & 0xFF;
		wbuf[6] = (val >> 32) & 0xFF;
		pr_debug("%s: reg[0x%04x], val[0x%02x%02x%02x%02x%02x]->swap->wbuf[0x%02x%02x%02x%02x%02x]\n",
			__func__, reg,
			wbuf[6], wbuf[5], wbuf[4], wbuf[3], wbuf[2],
			wbuf[2], wbuf[3], wbuf[4], wbuf[5], wbuf[6]);
		break;
	case 6:
		wbuf[2] = val & 0xFF;
		wbuf[3] = (val >>  8) & 0xFF;
		wbuf[4] = (val >> 16) & 0xFF;
		wbuf[5] = (val >> 24) & 0xFF;
		wbuf[6] = (val >> 32) & 0xFF;
		wbuf[7] = (val >> 40) & 0xFF;
		pr_debug("%s: reg[0x%04x], val[0x%02x%02x%02x%02x%02x%02x]->swap->wbuf[0x%02x%02x%02x%02x%02x%02x]\n",
			__func__, reg,
			wbuf[7], wbuf[6], wbuf[5], wbuf[4], wbuf[3], wbuf[2],
			wbuf[2], wbuf[3], wbuf[4], wbuf[5], wbuf[6], wbuf[7]);
		break;
	case 7:
		wbuf[2] = val & 0xFF;
		wbuf[3] = (val >>  8) & 0xFF;
		wbuf[4] = (val >> 16) & 0xFF;
		wbuf[5] = (val >> 24) & 0xFF;
		wbuf[6] = (val >> 32) & 0xFF;
		wbuf[7] = (val >> 40) & 0xFF;
		wbuf[8] = (val >> 48) & 0xFF;
		pr_debug("%s: reg[0x%04x], val[0x%02x%02x%02x%02x%02x%02x%02x]->swap->wbuf[0x%02x%02x%02x%02x%02x%02x%02x]\n",
			__func__, reg,
			wbuf[8], wbuf[7], wbuf[6], wbuf[5],
			wbuf[4], wbuf[3], wbuf[2],
			wbuf[2], wbuf[3], wbuf[4], wbuf[5],
			wbuf[6], wbuf[7], wbuf[8]);
		break;
	case 8:
		wbuf[2] = val & 0xFF;
		wbuf[3] = (val >>  8) & 0xFF;
		wbuf[4] = (val >> 16) & 0xFF;
		wbuf[5] = (val >> 24) & 0xFF;
		wbuf[6] = (val >> 32) & 0xFF;
		wbuf[7] = (val >> 40) & 0xFF;
		wbuf[8] = (val >> 48) & 0xFF;
		wbuf[9] = (val >> 56) & 0xFF;
		pr_debug("%s: reg[0x%04x], val[0x%02x%02x%02x%02x%02x%02x%02x%02x]->swap->wbuf[0x%02x%02x%02x%02x%02x%02x%02x%02x]\n",
			__func__, reg,
			wbuf[9], wbuf[8], wbuf[7], wbuf[6],
			wbuf[5], wbuf[4], wbuf[3], wbuf[2],
			wbuf[2], wbuf[3], wbuf[4], wbuf[5],
			wbuf[6], wbuf[7], wbuf[8], wbuf[9]);
		break;
	}

	msg.addr  = addr;
	msg.flags = 0;
	msg.len   = size+2;
	msg.buf   = wbuf;

	if (i2c_transfer(client->adapter, &msg, 1) != 1) {
		pr_err("%s: i2c write failed\n", __func__);
		rc = -EIO;
	}

	return rc;
}

static int sil9612_i2c_write_array(struct i2c_client *client,
			struct sil9612_reg_cfg *cfg)
{
	int rc = 0;
	int i = 0;

	if (!client) {
		pr_err("%s: Invalid params\n", __func__);
		return -EINVAL;
	}
	pr_debug("%s: [%s:0x%02x]\n", __func__, client->name, cfg->i2c_addr);

	client->addr = cfg->i2c_addr;

	while (cfg[i].i2c_addr != I2C_ADDR_MAX) {
		rc = sil9612_i2c_write(client, cfg[i].i2c_addr,
				cfg[i].reg, cfg[i].val_size, cfg[i].val);
		if (rc != 0)
			break;
		i++;
	}

	return rc;
}

static ssize_t sil9612_set_addr_store(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t len)
{
	int reg;

	if (kstrtoint(buf, 10, &reg)) {
		reg_r_addr[0] = 0xFF;
		reg_r_addr[1] = 0xFF;
		dev_err(dev, "%s: invalid argument\n", __func__);
		return -EINVAL;
	}
	dev_dbg(dev, "%s: reg_addr=0x%04x\n", __func__, reg);
	reg_r_addr[0] = reg >> 8;
	reg_r_addr[1] = reg & 0xFF;
	reg_w_addr = reg;
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t sil9612_reg_size_store(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t len)
{
	if (kstrtoint(buf, 10, &reg_r_size)) {
		reg_r_size = 0;
		dev_err(dev, "%s: invalid argument\n", __func__);
		return -EINVAL;
	}
	dev_dbg(dev, "%s: read size(%d)\n", __func__, reg_r_size);
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t sil9612_write_store(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t len)
{
	struct i2c_client *client = to_i2c_client(dev);
	int ret = -EINVAL;
	u64 value;

	if (!client)
		return -ENODEV;

	if (kstrtoull(buf, 10, &value)) {
		reg_r_size = 0;
		dev_err(dev, "%s: invalid argument\n", __func__);
		return -EINVAL;
	}

	if (reg_w_addr == 0xFFFF) {
		dev_err(dev, "%s: reg address is not set\n", __func__);
		return -EINVAL;
	}

	dev_dbg(dev, "%s: i2c write!!! addr : 0x%04x value : 0x%08x%08x size : %d\n",
			__func__, reg_w_addr,
			(unsigned int)((value >> 32) & 0xFFFFFFFF),
			(unsigned int)(value & 0xFFFFFFFF), reg_r_size);

	ret = sil9612_i2c_write(client, I2C_ADDR_MAIN,
				reg_w_addr, reg_r_size, value);
	if (ret)
		dev_err(dev, "%s: write error(%d)\n", __func__, ret);
	return strnlen(buf, PAGE_SIZE);
}

static ssize_t sil9612_read_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	int size;

	dev_dbg(dev, "%s: start ---- read addr[0x%04x]\n",
			__func__, reg_w_addr);

	if (reg_r_addr[0] == 0xFF && reg_r_addr[1] == 0xFF) {
		dev_err(dev, "%s: reg address is not set\n", __func__);
		return -EINVAL;
	}

	size = sil9612_i2c_read(client, I2C_ADDR_MAIN,
			reg_r_addr, buf, reg_r_size);

	return reg_r_size;
}

static ssize_t sil9612_irq_detect_read(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	int ret;

	pr_debug("%s: called\n", __func__);
	if (!irq_timeout)
		ret = wait_event_interruptible(irq_wait_q, (irq_flag == true));
	else
		ret = wait_event_interruptible_timeout(irq_wait_q,
			(irq_flag == true), msecs_to_jiffies(irq_timeout));
	if (ret < 0)
		return snprintf(buf, PAGE_SIZE, "%d", ret);
	else
		return snprintf(buf, PAGE_SIZE, "%d", irq_flag);
}

static ssize_t sil9612_irq_detect_write(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	int value;

	pr_debug("%s: called\n", __func__);
	if (kstrtoint(buf, 10, &value)) {
		dev_err(dev, "%s: invalid argument\n", __func__);
		value = 0;
	}
	if (value < 0)
		value = 0;
	irq_timeout = value;
	irq_flag = false;
	return count;
}

static ssize_t sil9612_get_enabled(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct sil9612 *pdata = i2c_get_clientdata(client);

	pr_debug("%s: called\n", __func__);
	return snprintf(buf, 1, "%d\n", pdata->hdmi_enabled);
}

static void sil9612_set_pinctrl(struct sil9612 *pdata, int enabled)
{
	int ret = 0;

	if (enabled && pdata->hdmi_det) {
		/* Scaler enable */
		if (!IS_ERR_OR_NULL(pdata->ts_pinctrl)) {
			pr_debug("%s:ts_pinctrl is enabled\n", __func__);
			ret = pinctrl_select_state(pdata->ts_pinctrl,
					pdata->pinctrl_state_active);

			if (ret < 0) {
				pr_err("%s: Failed to select %s pinstate %d\n",
					__func__, PINCTRL_STATE_ACTIVE, ret);
			}
		}
	} else if (!enabled) {
		/* Scaler disable */
		if (!IS_ERR_OR_NULL(pdata->ts_pinctrl)) {
			pr_debug("%s:ts_pinctrl is disabled\n", __func__);
			ret = pinctrl_select_state(pdata->ts_pinctrl,
					pdata->pinctrl_state_suspend);
			if (ret < 0)
				pr_err("%s: Failed to select %s pinstate %d\n",
					__func__, PINCTRL_STATE_SUSPEND, ret);
		}
	}
	switch_set_state(&pdata->hdmi_audio, enabled);
}

static ssize_t sil9612_set_enabled(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct sil9612 *pdata = i2c_get_clientdata(client);
	int value;

	pr_debug("%s: called\n", __func__);
	if (kstrtoint(buf, 10, &value)) {
		dev_err(dev, "%s: invalid argument\n", __func__);
		return -EINVAL;
	}
	if (pdata->hdmi_enabled != !!value) {
		sil9612_set_pinctrl(pdata, !!value);
		pdata->hdmi_enabled = !!value;
	}
	return count;
}

static struct device_attribute dev_attr_set_addr =
	__ATTR(set_addr, S_IWUSR, NULL, sil9612_set_addr_store);
static struct device_attribute dev_attr_set_r_size =
	__ATTR(set_reg_size, S_IWUSR, NULL, sil9612_reg_size_store);
static struct device_attribute dev_attr_reg_read =
	__ATTR(i2c_read, S_IRUSR, sil9612_read_show, NULL);
static struct device_attribute dev_attr_reg_write =
	__ATTR(i2c_write, S_IWUSR, NULL, sil9612_write_store);
static struct device_attribute dev_attr_sil9612_isr =
	__ATTR(sil9612_irq, S_IWUSR | S_IRUSR,
			sil9612_irq_detect_read, sil9612_irq_detect_write);
static struct device_attribute dev_attr_sil9612_enabled =
	__ATTR(sil9612_enabled, S_IWUSR | S_IRUSR,
			sil9612_get_enabled, sil9612_set_enabled);

static int sil9612_parse_dt(struct device *dev,
		struct sil9612 *pdata)
{
	struct device_node *np = dev->of_node;
	u32 temp_val;
	int ret = 0;
	int debug_cnt = 0;

	pr_debug("%s:%d\n", __func__, debug_cnt++);
	ret = of_property_read_u32(np, "sil,main-addr", &temp_val);
	pr_debug("%s: DT property %s is %X\n", __func__,
			"sil,main-addr", temp_val);

	if (ret)
		goto end;

	pdata->main_i2c_addr = (u8)temp_val;

	pr_debug("%s:%d\n", __func__, debug_cnt++);
	ret = 0;

	pdata->hdmi_det_irq_gpio = of_get_named_gpio_flags(np,
			"somc,hdmi-det-irq-gpio", 0,
			&pdata->hdmi_det_irq_flags);
	pr_debug("%s:pdata->hdmi_det_irq_gpio = %d\n",
			__func__, pdata->hdmi_det_irq_gpio);

	pdata->sil_int_irq_gpio = of_get_named_gpio_flags(np,
			"somc,sil-int-irq", 0, &pdata->sil_int_irq_flags);
	pr_debug("%s:pdata->sil_int_irq_gpio = %d\n",
			__func__, pdata->sil_int_irq_gpio);

	/* Get pinctrl if target uses pinctrl */
	pdata->ts_pinctrl = devm_pinctrl_get(dev);
	if (IS_ERR_OR_NULL(pdata->ts_pinctrl)) {
		pr_err("%s:ts_pinctrl is null or err\n", __func__);
		ret = PTR_ERR(pdata->ts_pinctrl);
		pr_err("%s: Pincontrol DT property returned %X\n",
				__func__, ret);
	}

	if (!ret) {
		pr_debug("%s:%d\n", __func__, debug_cnt++);
		pdata->pinctrl_state_active =
			pinctrl_lookup_state(pdata->ts_pinctrl, "sil9612_act");
		pr_debug("%s:%d\n", __func__, debug_cnt++);
		if (IS_ERR_OR_NULL(pdata->pinctrl_state_active)) {
			pr_err("%s:%d\n", __func__, debug_cnt++);
			ret = PTR_ERR(pdata->pinctrl_state_active);
			pr_err("Can not lookup %s pinstate %d\n",
					PINCTRL_STATE_ACTIVE, ret);
		}
	}

	if (!ret) {
		pr_debug("%s:%d\n", __func__, debug_cnt++);
		pdata->pinctrl_state_suspend =
			pinctrl_lookup_state(pdata->ts_pinctrl, "sil9612_sus");
		if (IS_ERR_OR_NULL(pdata->pinctrl_state_suspend)) {
			pr_err("%s:%d\n", __func__, debug_cnt++);
			ret = PTR_ERR(pdata->pinctrl_state_suspend);
			pr_err("Can not lookup %s pinstate %d\n",
					PINCTRL_STATE_SUSPEND, ret);
		}
	}

end:
	pr_debug("%s:end\n", __func__);
	return ret;
}

static int sil9612_gpio_configure(struct sil9612 *pdata, bool on)
{
	int ret = 0;

	pr_debug("%s\n", __func__);

	if (on) {
		/* HDMICN_DET IRQ config */
		if (gpio_is_valid(pdata->hdmi_det_irq_gpio)) {
			ret = gpio_request(pdata->hdmi_det_irq_gpio,
					"hdmi_det_irq_gpio");
			if (ret) {
				pr_err("unable to request gpio [%d]\n",
						pdata->hdmi_det_irq_gpio);
				goto end;
			}
			ret = gpio_direction_input(pdata->hdmi_det_irq_gpio);
			if (ret) {
				pr_err("unable to set dir for gpio[%d]\n",
						pdata->hdmi_det_irq_gpio);
				goto err_hdmi_det_irq_gpio;
			}
		} else {
			pr_err("hpd irq gpio not provided\n");
		}

		/* sil9612 IRQ config */
		if (gpio_is_valid(pdata->sil_int_irq_gpio)) {
			ret = gpio_request(pdata->sil_int_irq_gpio,
					"sil_int_irq_gpio");
			if (ret)
				pr_warn(
				    "%s: sil_int_irq_gpio already requested\n",
				    __func__);
			ret = gpio_direction_input(pdata->sil_int_irq_gpio);
			if (ret) {
				pr_err("%s: unable to setdir for sil_int_irq\n",
					__func__);
				goto err_hdmi_det_irq_gpio;
			}
		} else {
			pr_err("sil irq gpio not provided\n");
		}

		ret = 0;
		goto end;
	} else {
		if (gpio_is_valid(pdata->hdmi_det_irq_gpio))
			gpio_free(pdata->hdmi_det_irq_gpio);
		return 0;
	}

err_hdmi_det_irq_gpio:
	if (gpio_is_valid(pdata->hdmi_det_irq_gpio))
		gpio_free(pdata->hdmi_det_irq_gpio);
end:
	return ret;
}

static int sil9612_set_switch_device(struct sil9612 *pdata)
{
	int error = 0;

	pdata->hdmi.name = SIL9612_SW_HDMI_NAME;
	pdata->hdmi.state = 0;
	error = switch_dev_register(&pdata->hdmi);
	if (error) {
		pr_err("%s cannot regist HDMI(%d)\n", __func__, error);
		return error;
	}

	pdata->hdmi_audio.name = SIL9612_SW_HDMI_AUDIO_NAME;
	pdata->hdmi_audio.state = 0;
	error = switch_dev_register(&pdata->hdmi_audio);
	if (error)
		pr_err("%s cannot regist HDMI(%d)\n", __func__, error);

	return error;
}

static void hdmicn_det_timer(unsigned long func_data)
{
	struct sil9612 *pdata = (struct sil9612 *)func_data;

	schedule_work(&pdata->det_work);
}

static void hdmicn_det_work(struct work_struct *work)
{
	struct sil9612 *pdata = container_of(work, struct sil9612, det_work);
	int hdmi_det = 0;

	if (!pdata) {
		pr_err("%s: invalid input\n", __func__);
		goto end;
	}

	hdmi_det = gpio_get_value(pdata->hdmi_det_irq_gpio);
	pr_debug("%s: get_value(%d) = %d\n", __func__,
			pdata->hdmi_det_irq_gpio, hdmi_det);

	if (pdata->hdmi_det != hdmi_det) {
		pdata->hdmi_det = hdmi_det;
		switch_set_state(&pdata->hdmi, hdmi_det);
		if (!pdata->hdmi_det && pdata->hdmi_enabled) {
			sil9612_set_pinctrl(pdata, 0);
			pdata->hdmi_enabled = 0;
		}
	}

end:
	return;
}

static irqreturn_t hdmicn_det_irq(int irq, void *data)
{
	struct sil9612 *pdata = (struct sil9612 *)data;
	int timer_debounce = 0;

	if (timer_debounce)
		mod_timer(&pdata->det_timer,
			jiffies + msecs_to_jiffies(timer_debounce));
	else
		schedule_work(&pdata->det_work);

	return IRQ_HANDLED;
}

static irqreturn_t sil9612_int3_isr(int irq, void *dev_id)
{
	pr_debug("%s: called\n", __func__);
	if (irq_flag == false) {
		irq_flag = true;
		pr_debug("%s: wake_up_interruptible called\n", __func__);
		wake_up_interruptible(&irq_wait_q);
	}
	return IRQ_HANDLED;
}

static struct i2c_device_id sil9612_id[] = {
	{ "sil9612", 0},
	{}
};
static ssize_t sil9612_read_ksv_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	int i = 0;
	u8 w_buf[2];
	int rc = 0;
	u8 ksv[5] = {1, 2, 3, 4, 5};
	u8 ret = 0;
	bool is_err = false;

	struct i2c_client *client = to_i2c_client(dev);

	/* read BKSV */
	ret += snprintf(buf + ret, PAGE_SIZE - ret, "Scaler Sink(Rx)  :");
	i = 0;
	while (sil9612_hdcp_bksv[i].i2c_addr != I2C_ADDR_MAX) {
		w_buf[0] = sil9612_hdcp_bksv[i].reg >> 8;
		w_buf[1] = sil9612_hdcp_bksv[i].reg & 0xFF;
		rc = sil9612_i2c_read(client,
			sil9612_hdcp_bksv[i].i2c_addr, w_buf, &ksv[i], 1);
		if (rc < 0) {
			pr_err("%s: Failed to read i2c bksv rc=%d i=%d\n",
				__func__, rc, i);
			ret += snprintf(buf + ret,
				PAGE_SIZE - ret, "[%d]err ", i);
			is_err = true;
		} else {
			ret += snprintf(buf + ret, PAGE_SIZE - ret,
				"[%d]0x%02x ", i, ksv[i]);
		}
		i++;
	}
	pr_debug("%s: bksv ret=%d buf=%s\n", __func__, ret, buf);

	/* read AKSV */
	ret += snprintf(buf + ret, PAGE_SIZE - ret, "\nScaler Source(Tx):");
	i = 0;
	while (sil9612_hdcp_aksv[i].i2c_addr != I2C_ADDR_MAX) {
		w_buf[0] = sil9612_hdcp_aksv[i].reg >> 8;
		w_buf[1] = sil9612_hdcp_aksv[i].reg & 0xFF;
		rc = sil9612_i2c_read(client,
			sil9612_hdcp_aksv[i].i2c_addr, w_buf, &ksv[i], 1);
		if (rc < 0) {
			pr_err("%s: Failed to read i2c aksv rc=%d i=%d\n",
					__func__, rc, i);
			ret += snprintf(buf + ret,
				PAGE_SIZE - ret, "[%d]err ", i);
			is_err = true;
		} else {
			ret += snprintf(buf + ret, PAGE_SIZE - ret,
				"[%d]0x%02x ", i, ksv[i]);
		}
		i++;
	}
	pr_debug("%s: aksv ret=%d buf=%s\n", __func__, ret, buf);

	/* read KSV of MIPItoHDMI */
	ret += snprintf(buf + ret, PAGE_SIZE - ret, "\nMIPItoHDMI(Tx)   :");
	sil9612_i2c_write_array(client, sil9612_hdcp_mipi2hdmi_ksv_enable);
	i = 0;
	while (sil9612_hdcp_mipi2hdmi_ksv[i].i2c_addr != I2C_ADDR_MAX) {
		w_buf[0] = sil9612_hdcp_mipi2hdmi_ksv[i].reg >> 8;
		w_buf[1] = sil9612_hdcp_mipi2hdmi_ksv[i].reg & 0xFF;
		rc = sil9612_i2c_read(client,
			sil9612_hdcp_mipi2hdmi_ksv[i].i2c_addr,
			w_buf, &ksv[i], 1);
		if (rc < 0) {
			pr_err("%s: Failed to read i2c MIPItoHDMI ksv rc=%d i=%d\n",
				__func__, rc, i);
			ret += snprintf(buf + ret, PAGE_SIZE - ret,
				"[%d]err ", i);
			is_err = true;
		} else {
			ret += snprintf(buf + ret, PAGE_SIZE - ret,
				"[%d]0x%02x ", i, ksv[i]);
		}
		i++;
	}
	ret += snprintf(buf + ret, PAGE_SIZE - ret, "\n");

	if (is_err) {
		ret += snprintf(buf + ret, PAGE_SIZE - ret,
			"\n!!I2C ERROR!! Please connect any HDMI devices once.\n");
		pr_err("%s:I2C ERROR\n", __func__);
	}

	return ret;

}
static struct device_attribute dev_attr_read_ksv =
	__ATTR(sil9612_read_ksv, S_IRUGO,
		sil9612_read_ksv_show, NULL);

static struct attribute *sil9612_attrs[] = {
	&dev_attr_read_ksv.attr,
	&dev_attr_set_addr.attr,
	&dev_attr_set_r_size.attr,
	&dev_attr_reg_read.attr,
	&dev_attr_reg_write.attr,
	&dev_attr_sil9612_isr.attr,
	&dev_attr_sil9612_enabled.attr,
	NULL,
};
static struct attribute_group sil9612_attr_group = {
	.name = NULL,
	.attrs = sil9612_attrs,
};
static int sil9612_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	static struct sil9612 *pdata;
	int ret = 0;
	int debug_cnt = 0;

	pr_debug("%s\n", __func__);

	if (!client || !client->dev.of_node) {
		pr_err("%s: invalid input\n", __func__);
		return -EINVAL;
	}

	pr_debug("%s:%d\n", __func__, debug_cnt++);

	pdata = devm_kzalloc(&client->dev, sizeof(struct sil9612), GFP_KERNEL);
	if (pdata == NULL) {
		pr_err("%s: Failed to allocate memory\n", __func__);
		return -ENOMEM;
	}

	pr_debug("%s:%d\n", __func__, debug_cnt++);

	ret = sil9612_parse_dt(&client->dev, pdata);
	if (ret) {
		pr_err("%s: Failed to parse DT\n", __func__);
		goto err_dt_parse;
	}

	pr_debug("%s:%d\n", __func__, debug_cnt++);
	pdata->i2c_client = client;
	i2c_set_clientdata(client, pdata);
	pdata->hdmi_enabled = false;
	irq_timeout = 0;
	irq_flag = false;

	pr_debug("%s:%d\n", __func__, debug_cnt++);
	ret = sil9612_gpio_configure(pdata, true);
	if (ret) {
		pr_err("%s: ret = %d\n", __func__, ret);
		pr_err("%s: Failed to configure GPIOs\n", __func__);
		goto err_gpio_cfg;
	}

	INIT_WORK(&pdata->det_work, hdmicn_det_work);
	setup_timer(&pdata->det_timer, hdmicn_det_timer, (unsigned long)pdata);

	pr_debug("%s:%d\n", __func__, debug_cnt++);
	pdata->hdmi_det_irq = gpio_to_irq(pdata->hdmi_det_irq_gpio);

	pr_debug("%s:%d\n", __func__, debug_cnt++);
	ret = request_threaded_irq(pdata->hdmi_det_irq, NULL, hdmicn_det_irq,
		IRQF_ONESHOT | IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,
		"hdmi_det", pdata);
	if (ret) {
		pr_err("%s: Failed to enable sil9612 interrupt\n",
				__func__);
		goto err_hdmi_det_irq;
	}

	pdata->sil_int_irq = gpio_to_irq(pdata->sil_int_irq_gpio);
	pr_debug("%s: pdata->sil_int_irq =(%d)\n",
			__func__, pdata->sil_int_irq);
	ret = request_threaded_irq(pdata->sil_int_irq, NULL, sil9612_int3_isr,
			IRQF_TRIGGER_FALLING | IRQF_ONESHOT | IRQF_SHARED,
			"sil_int_det", pdata);
	if (ret) {
		pr_err("%s: Failed to enable sil9612 interrupt(ret=%d)\n",
				__func__, ret);
		goto err_sil_int_irq;
	}

	pr_debug("%s:%d\n", __func__, debug_cnt++);
	pm_runtime_enable(&client->dev);
	pr_debug("%s:%d\n", __func__, debug_cnt++);
	pm_runtime_set_active(&client->dev);

	ret = sysfs_create_group(&client->dev.kobj, &sil9612_attr_group);
	if (ret) {
		pr_err("%s:sysfs_create_group ret = %d\n", __func__, ret);
		goto err_hdmi_det_irq;
	}

	ret = sil9612_set_switch_device(pdata);
	if (ret) {
		pr_err("%s:cannot set switch dev ret = %d\n", __func__, ret);
		goto err_hdmi_det_irq;
	}

	init_waitqueue_head(&irq_wait_q);
	schedule_work(&pdata->det_work);

	pr_info("%s:success\n", __func__);

	return 0;

err_sil_int_irq:
	disable_irq(pdata->sil_int_irq);
	free_irq(pdata->sil_int_irq, pdata);
err_hdmi_det_irq:
	switch_dev_unregister(&pdata->hdmi);
	switch_dev_unregister(&pdata->hdmi_audio);
	sil9612_gpio_configure(pdata, false);
err_gpio_cfg:
err_dt_parse:
	devm_kfree(&client->dev, pdata);
	return ret;
}

static int sil9612_remove(struct i2c_client *client)
{
	int ret = -EINVAL;
	struct msm_dba_device_info *dev;
	struct sil9612 *pdata;

	pr_debug("%s\n", __func__);
	if (!client)
		goto end;

	dev = dev_get_drvdata(&client->dev);
	if (!dev)
		goto end;

	pm_runtime_disable(&client->dev);
	disable_irq(pdata->hdmi_det_irq);
	free_irq(pdata->hdmi_det_irq, pdata);
	disable_irq(pdata->sil_int_irq);
	free_irq(pdata->sil_int_irq, pdata);

	ret = sil9612_gpio_configure(pdata, false);

	sysfs_remove_group(&client->dev.kobj, &sil9612_attr_group);
	switch_dev_unregister(&pdata->hdmi);
	switch_dev_unregister(&pdata->hdmi_audio);

	devm_kfree(&client->dev, pdata);

end:
	return ret;
}

static struct i2c_driver sil9612_driver = {
	.driver = {
		.name = "sil,sil9612",
		.owner = THIS_MODULE,
	},
	.probe = sil9612_probe,
	.remove = sil9612_remove,
	.id_table = sil9612_id,
};

static int __init sil9612_init(void)
{
	pr_debug("%s\n", __func__);
	return i2c_add_driver(&sil9612_driver);
}

static void __exit sil9612_exit(void)
{
	i2c_del_driver(&sil9612_driver);
}

module_init(sil9612_init);
module_exit(sil9612_exit);
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("sil9612 driver");
