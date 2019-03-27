/*
 * Copyright (C) 2010 - 2017 Novatek, Inc.
 *
 * $Revision: 23175 $
 * $Date: 2018-02-12 16:26:21 +0800 (Mon, 12 Feb 2018) $
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 */

#define pr_fmt(fmt) "nt36xxx: " fmt

#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/input/mt.h>
#include <linux/of_gpio.h>
#include <linux/of_irq.h>

#if defined(CONFIG_FB)
#include <linux/notifier.h>
#include <linux/fb.h>
#endif

#ifdef CONFIG_DRM_MSM_DSI_SOMC_PANEL
#include <linux/drm_notify.h>
#endif

#include "nt36xxx.h"

#if NVT_TOUCH_EXT_PROC
extern int32_t nvt_extra_proc_init(void);
#endif

#if NVT_TOUCH_MP
extern int32_t nvt_mp_proc_init(void);
#endif

struct nvt_ts_data *ts;

static struct workqueue_struct *nvt_wq;

#if BOOT_UPDATE_FIRMWARE
static struct workqueue_struct *nvt_fwu_wq;
extern void Boot_Update_Firmware(struct work_struct *work);
#endif

#if defined(CONFIG_FB) && !defined(CONFIG_DRM_MSM_DSI_SOMC_PANEL)
static int fb_notifier_callback(struct notifier_block *self, unsigned long event, void *data);
#elif defined(CONFIG_FB) && defined(CONFIG_DRM_MSM_DSI_SOMC_PANEL)
static int drm_notifier_callback(struct notifier_block *self, unsigned long event, void *data);
#endif

static uint8_t bTouchIsAwake = 0;

/*---For SMx3 use start---*/
uint32_t last_input_x = -1;
uint32_t last_input_y = -1;

/*******************************************************
 *Description:
 *	SMx3 touchscreen translate tp->source code to text
 *	function.
 *
 *return:
 *	Executive outcomes: tp source code.
 *******************************************************/
char *smx3_tp_source_to_text(void)
{
	if (ts->tp_source == TP_SOURCE_TR_M08)
		return "TR M 0.8mm";
	else if (ts->tp_source == TP_SOURCE_TR_M10)
		return "TR M 1.0mm";
	else if (ts->tp_source == TP_SOURCE_TR_S08)
		return "TR S 0.8mm";
	else if (ts->tp_source == TP_SOURCE_TM)
		return "TM M 0.8mm";

	return "Unknown   ";
}

#if NVT_PROJECT_INFO_READ
/*******************************************************
 *Description:
 *	SMx3 touchscreen check tp source function.
 *
 *return:
 *	Executive outcomes: tp source code.
 *******************************************************/
uint8_t smx3_check_project_info(void)
{
	uint8_t ret = 0xFF;

	if (ts->project_info[0] == TP_MODULE_PID_SM13) {
		if (ts->project_info[1] == TP_MODULE_VE_TR) {
			if (ts->project_info[2] == TP_MODULE_CG_KM) {
				if (ts->project_info[3] == TP_MODULE_CG_08)
					ret = TP_SOURCE_TR_M08;
				else if (ts->project_info[3] == TP_MODULE_CG_10)
					ret = TP_SOURCE_TR_M10;
			} else if (ts->project_info[2] == TP_MODULE_CG_TR) {
				if (ts->project_info[3] == TP_MODULE_CG_08)
					ret = TP_SOURCE_TR_S08;
			}
		}
	} else if (ts->project_info[0] == TP_MODULE_PID_SM23) {
		if (ts->project_info[1] == TP_MODULE_VE_TM) {
			if (ts->project_info[2] == TP_MODULE_CG_TM) {
				if (ts->project_info[3] == TP_MODULE_CG_08)
					ret = TP_SOURCE_TM;
			}
		}
	}

	return ret;
}

/*******************************************************
 *Description:
 *	Novatek touchscreen project info read function.
 *
 *return:
 *	Executive outcomes. 0---succeed. negative---failed.
 *******************************************************/
int32_t nvt_read_project_info(void)
{
	uint8_t buf[3 + PROJECT_INFO_LEN] = {0};
	int32_t ret = 0;
	unsigned char i;

	/*---Stop CRC check to prevent IC auto reboot--- */
	nvt_stop_crc_reboot();

	/* Step 1 : initial bootloader */
	ret = Init_BootLoader();
	if (ret)
		return ret;

	/* Step 2 : Resume PD */
	ret = Resume_PD();
	if (ret)
		return ret;

	/* Step 3 : unlock */
	buf[0] = 0x00;
	buf[1] = 0x35;
	ret = CTP_I2C_WRITE(ts->client, I2C_HW_Address, buf, 2);
	if (ret < 0) {
		pr_err("write unlock error!!(%d)\n", ret);
		return ret;
	}

	msleep(20);

	/* Step 4 : Flash Read Command */
	buf[0] = 0x00;
	buf[1] = 0x03;
	buf[2] = (PROJECT_INFO_ADDR >> 16) & 0xFF; /* Addr_H */
	buf[3] = (PROJECT_INFO_ADDR >> 8) & 0xFF; /* Addr_M */
	buf[4] = PROJECT_INFO_ADDR & 0xFF; /* Addr_L */
	buf[5] = (PROJECT_INFO_LEN >> 8) & 0xFF; /* Len_H */
	buf[6] = PROJECT_INFO_LEN & 0xFF; /* Len_L */
	ret = CTP_I2C_WRITE(ts->client, I2C_HW_Address, buf, 7);
	if (ret < 0) {
		pr_err("write Read Command error!!(%d)\n", ret);
		return ret;
	}

	msleep(20);

	/* Check 0xAA (Read Command) */
	buf[0] = 0x00;
	buf[1] = 0x00;
	ret = CTP_I2C_READ(ts->client, I2C_HW_Address, buf, 2);
	if (ret < 0) {
		pr_err("Check 0xAA (Read Command) error!!(%d)\n", ret);
		return ret;
	}
	if (buf[1] != 0xAA) {
		pr_err("Check 0xAA (Read Command) error!! status=0x%02X\n",
			buf[1]);
		return -EPERM;
	}

	msleep(20);

	/* Step 5 : Read Flash Data */
	buf[0] = 0xFF;
	buf[1] = (ts->mmap->READ_FLASH_CHECKSUM_ADDR >> 16) & 0xFF;
	buf[2] = (ts->mmap->READ_FLASH_CHECKSUM_ADDR >> 8) & 0xFF;
	ret = CTP_I2C_WRITE(ts->client, I2C_BLDR_Address, buf, 3);
	if (ret < 0) {
		pr_err("change index error!! (%d)\n", ret);
		return ret;
	}

	msleep(20);

	/* Read Back */
	buf[0] = ts->mmap->READ_FLASH_CHECKSUM_ADDR & 0xFF;
	ret = CTP_I2C_READ(ts->client, I2C_BLDR_Address, buf,
		3 + PROJECT_INFO_LEN);
	if (ret < 0) {
		pr_err("Read Back error!! (%d)\n", ret);
		return ret;
	}

	/* pr_debug("Project info = 0x%02X%02X%02X%02X%02X%02X%02X%02X\n",
	 * buf[3], buf[4], buf[5], buf[6], buf[7], buf[8], buf[9], buf[10]);
	 */

	for (i = 0; i < PROJECT_INFO_LEN_VALID; i++)
		ts->project_info[i] = buf[i + 3];

	/* Step 6 : Bootloader Reset */
	nvt_bootloader_reset();
	nvt_check_fw_reset_state(RESET_STATE_INIT);

	return 0;
}
#endif /* NVT_PROJECT_INFO_READ */

static int smx3_get_irq_depth(void)
{
	struct irq_desc *desc = irq_to_desc(ts->client->irq);

	if (!desc) {
		pr_debug("irq depth is null\n");
		return 0;
	}

	return desc->depth;
}

#if NVT_DOZE_MODE_FINC
int32_t nvt_enter_doze_mode(void)
{
	uint8_t buf[3] = {0};

	mutex_lock(&ts->lock);

	disable_irq(ts->client->irq);

	/*--- write i2c command to enter "deep sleep mode" ---*/
	buf[0] = EVENT_MAP_HOST_CMD;
	buf[1] = 0x11;
	CTP_I2C_WRITE(ts->client, I2C_FW_Address, buf, 2);

	mutex_unlock(&ts->lock);

	return 0;
}

int32_t nvt_enter_normal_mode(void)
{
	mutex_lock(&ts->lock);

	nvt_bootloader_reset();

	enable_irq(ts->client->irq);

	mutex_unlock(&ts->lock);

	return 0;
}
#endif

#if NVT_IRQ_CONTROL
#define ENABLE_IRQ_DELAY_MS 20
static int smx3_enable_irq(bool enable, bool no_sync)
{
	int ret;

	mutex_lock(&ts->irq_lock);

	if (ts->irq_gpio < 0) {
		pr_err("Invalid IRQ GPIO\n");
		ret = -EINVAL;
		goto exit;
	}

	if (enable) {
		if (smx3_get_irq_depth() == 0) {
			pr_debug("Interrupt already enabled\n");
			ret = 0;
			goto exit;
		}

		enable_irq(ts->client->irq);
		pr_debug("Interrupt enabled\n");

		ret = 0;
		msleep(ENABLE_IRQ_DELAY_MS);
	} else {
		if (smx3_get_irq_depth() > 0) {
			pr_debug("Interrupt already disabled\n");
			ret = 0;
			goto exit;
		}

		if (no_sync) {
			disable_irq_nosync(ts->client->irq);
			pr_debug("Interrupt disabled and no_sync\n");
		} else {
			disable_irq(ts->client->irq);
			pr_debug("Interrupt disabled\n");
		}
		ret = 0;
	}

exit:
	mutex_unlock(&ts->irq_lock);

	return ret;
}
#endif

/* sysfs device node start */
char tp_probe_log[1024];
int tp_probe_log_index = -1;

static ssize_t smx3_tp_probe_log_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{

	return snprintf(buf, 37 + tp_probe_log_index,
		"******start******\n%s*******end*******\n", tp_probe_log);
}

#if NVT_PROJECT_INFO_READ
static ssize_t smx3_tp_project_info_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	mutex_lock(&ts->lock);

	/* Get external flash id (project info) */
	nvt_read_project_info();
	mutex_unlock(&ts->lock);

	return snprintf(buf, 15, "0x%02X%02X%02X%02X%02X\n",
		ts->project_info[0], ts->project_info[1], ts->project_info[2],
		ts->project_info[3], ts->project_info[4]);
}
#endif /*def NVT_PROJECT_INFO_READ */

static ssize_t smx3_tp_source_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return snprintf(buf, 25, "%s (0x%02x)\n", smx3_tp_source_to_text(),
		ts->tp_source);
}

#if NVT_DOZE_MODE_FINC
static ssize_t smx3_Doze_mode_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return snprintf(buf, 64, "NVT Doze mode\n");
}

static ssize_t smx3_Doze_mode_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	u8 i;
	int ret, ret_func;

	if (kstrtou8(buf, 0, &i) == 0 && i == 0) {
		ret_func = nvt_enter_doze_mode();

		if (ret_func < 0)
			pr_err("Fail, enter Doze mode\n");
		else
			pr_debug("Success, enter Doze mode\n");
	} else if (i == 1) {
		ret_func = nvt_enter_normal_mode();

		if (ret_func < 0)
			pr_err("Fail, enter Normal mode\n");
		else
			pr_debug("Success, enter Normal mode\n");
	} else {
		pr_debug("input wrong command\n");
	}

	ret = count;

	return ret;
}
#endif

#if NVT_IRQ_CONTROL
static ssize_t smx3_irq_depth_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return snprintf(buf, 30, "depth = %d\n", smx3_get_irq_depth());
}

static ssize_t smx3_enable_irq_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	u8 i;
	int ret;

	mutex_lock(&ts->lock);

	if (kstrtou8(buf, 0, &i) == 0 && i == 0) {
		ret = smx3_enable_irq(false, false);

		if (ret < 0)
			pr_err("Fail disable irq\n");
		else
			pr_debug("Success disable irq\n");
	} else if (i == 1) {
		ret = smx3_enable_irq(true, false);

		if (ret < 0)
			pr_err("Fail enable irq\n");
		else
			pr_debug("Success enable irq\n");
	} else {
		pr_debug("input wrong command\n");
	}

	mutex_unlock(&ts->lock);

	ret = count;

	return ret;
}
#endif

#if NVT_LIBSYSINFO
static ssize_t smx3_fw_version_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int buf_offset = 0;
	int ret = 0;

	if (mutex_lock_interruptible(&ts->lock))
		goto exit;

	if (nvt_get_fw_info())
		goto exit;


	ret = snprintf(buf + buf_offset, 16, "0x%02x",
		ts->fw_ver);
	if (ret < 0)
		goto exit;

	buf_offset += ret;
	ret = buf_offset;

exit:
	mutex_unlock(&ts->lock);

	return ret;
}

static ssize_t smx3_source_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return snprintf(buf, 32, "%s (0x%02x)", smx3_tp_source_to_text(),
		ts->tp_source);
}

static ssize_t smx3_module_pid_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int buf_offset = 0;
	int ret = 0;

	ret = snprintf(buf + buf_offset, 16, "0x%04x",
		ts->nvt_pid);
	if (ret < 0)
		goto exit;

	buf_offset += ret;
	ret = buf_offset;

exit:
	return ret;
}
#endif

static inline ssize_t smx3_tpnode_store_error(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	dev_warn(dev, "%s Attempted to write to read-only attribute %s\n",
			__func__, attr->attr.name);
	return -EPERM;
}

static struct device_attribute attrs[] = {
	__ATTR(smx3_probe_log, S_IRUGO,
			smx3_tp_probe_log_show,
			smx3_tpnode_store_error),
#if NVT_PROJECT_INFO_READ
	__ATTR(smx3_project_info, S_IRUGO,
			smx3_tp_project_info_show,
			smx3_tpnode_store_error),
#endif /*def NVT_PROJECT_INFO_READ */
	__ATTR(smx3_source, S_IRUGO,
			smx3_tp_source_show,
			smx3_tpnode_store_error),
#if NVT_DOZE_MODE_FINC
	__ATTR(Doze_mode, (S_IRUGO | S_IWUSR | S_IWGRP),
			smx3_Doze_mode_show,
			smx3_Doze_mode_store),
#endif
#if NVT_IRQ_CONTROL
	__ATTR(smx3_enable_irq, (S_IRUGO | S_IWUSR | S_IWGRP),
			smx3_irq_depth_show,
			smx3_enable_irq_store),
#endif
#if NVT_LIBSYSINFO
	__ATTR(fw_version, S_IRUGO,
			smx3_fw_version_show,
			smx3_tpnode_store_error),
	__ATTR(tp_source, S_IRUGO,
			smx3_source_show,
			smx3_tpnode_store_error),
	__ATTR(module_pid, S_IRUGO,
			smx3_module_pid_show,
			smx3_tpnode_store_error),
#endif
};
/* sysfs device node end */
/*---For SMx3 use end---*/

/*******************************************************
Description:
	Novatek touchscreen i2c read function.

return:
	Executive outcomes. 2---succeed. -5---I/O error
*******************************************************/
int32_t CTP_I2C_READ(struct i2c_client *client, uint16_t address, uint8_t *buf, uint16_t len)
{
	struct i2c_msg msgs[2];
	int32_t ret = -1;
	int32_t retries = 0;

	msgs[0].flags = !I2C_M_RD;
	msgs[0].addr  = address;
	msgs[0].len   = 1;
	msgs[0].buf   = &buf[0];

	msgs[1].flags = I2C_M_RD;
	msgs[1].addr  = address;
	msgs[1].len   = len - 1;
	msgs[1].buf   = &buf[1];

	while (retries < 5) {
		ret = i2c_transfer(client->adapter, msgs, 2);
		if (ret == 2)	break;
		retries++;
	}

	if (unlikely(retries == 5)) {
		pr_err("error at retry %d times, ret=%d\n", retries - 1, ret);
		ret = -EIO;
	}

	return ret;
}

/*******************************************************
Description:
	Novatek touchscreen i2c write function.

return:
	Executive outcomes. 1---succeed. -5---I/O error
*******************************************************/
int32_t CTP_I2C_WRITE(struct i2c_client *client, uint16_t address, uint8_t *buf, uint16_t len)
{
	struct i2c_msg msg;
	int32_t ret = -1;
	int32_t retries = 0;

	msg.flags = !I2C_M_RD;
	msg.addr  = address;
	msg.len   = len;
	msg.buf   = buf;

	while (retries < 5) {
		ret = i2c_transfer(client->adapter, &msg, 1);
		if (ret == 1)	break;
		retries++;
	}

	if (unlikely(retries == 5)) {
		pr_err("error at retry %d times, ret=%d\n", retries - 1, ret);
		ret = -EIO;
	}

	return ret;
}


/*******************************************************
Description:
	Novatek touchscreen reset MCU then into idle mode
    function.

return:
	n.a.
*******************************************************/
void nvt_sw_reset_idle(void)
{
	uint8_t buf[4]={0};

	//---write i2c cmds to reset idle---
	buf[0]=0x00;
	buf[1]=0xA5;
	CTP_I2C_WRITE(ts->client, I2C_HW_Address, buf, 2);

	msleep(15);
}

/*******************************************************
Description:
	Novatek touchscreen reset MCU (boot) function.

return:
	n.a.
*******************************************************/
void nvt_bootloader_reset(void)
{
	uint8_t buf[8] = {0};

	//---write i2c cmds to reset---
	buf[0] = 0x00;
	buf[1] = 0x69;
	CTP_I2C_WRITE(ts->client, I2C_HW_Address, buf, 2);

	// need 35ms delay after bootloader reset
	msleep(35);
}

/*******************************************************
Description:
	Novatek touchscreen clear FW status function.

return:
	Executive outcomes. 0---succeed. -1---fail.
*******************************************************/
int32_t nvt_clear_fw_status(void)
{
	uint8_t buf[8] = {0};
	int32_t i = 0;
	const int32_t retry = 20;

	for (i = 0; i < retry; i++) {
		//---set xdata index to EVENT BUF ADDR---
		buf[0] = 0xFF;
		buf[1] = (ts->mmap->EVENT_BUF_ADDR >> 16) & 0xFF;
		buf[2] = (ts->mmap->EVENT_BUF_ADDR >> 8) & 0xFF;
		CTP_I2C_WRITE(ts->client, I2C_FW_Address, buf, 3);

		//---clear fw status---
		buf[0] = EVENT_MAP_HANDSHAKING_or_SUB_CMD_BYTE;
		buf[1] = 0x00;
		CTP_I2C_WRITE(ts->client, I2C_FW_Address, buf, 2);

		//---read fw status---
		buf[0] = EVENT_MAP_HANDSHAKING_or_SUB_CMD_BYTE;
		buf[1] = 0xFF;
		CTP_I2C_READ(ts->client, I2C_FW_Address, buf, 2);

		if (buf[1] == 0x00)
			break;

		msleep(10);
	}

	if (i >= retry) {
		pr_err("failed, i=%d, buf[1]=0x%02X\n", i, buf[1]);
		return -1;
	} else {
		return 0;
	}

}

/*******************************************************
Description:
	Novatek touchscreen check FW status function.

return:
	Executive outcomes. 0---succeed. -1---failed.
*******************************************************/
int32_t nvt_check_fw_status(void)
{
	uint8_t buf[8] = {0};
	int32_t i = 0;
	const int32_t retry = 50;

	for (i = 0; i < retry; i++) {
		//---set xdata index to EVENT BUF ADDR---
		buf[0] = 0xFF;
		buf[1] = (ts->mmap->EVENT_BUF_ADDR >> 16) & 0xFF;
		buf[2] = (ts->mmap->EVENT_BUF_ADDR >> 8) & 0xFF;
		CTP_I2C_WRITE(ts->client, I2C_FW_Address, buf, 3);

		//---read fw status---
		buf[0] = EVENT_MAP_HANDSHAKING_or_SUB_CMD_BYTE;
		buf[1] = 0x00;
		CTP_I2C_READ(ts->client, I2C_FW_Address, buf, 2);

		if ((buf[1] & 0xF0) == 0xA0)
			break;

		msleep(10);
	}

	if (i >= retry) {
		pr_err("failed, i=%d, buf[1]=0x%02X\n", i, buf[1]);
			return -1;
	} else {
			return 0;
	}
}

/*******************************************************
Description:
	Novatek touchscreen check FW reset state function.

return:
	Executive outcomes. 0---succeed. -1---failed.
*******************************************************/
int32_t nvt_check_fw_reset_state(RST_COMPLETE_STATE check_reset_state)
{
	uint8_t buf[8] = {0};
	int32_t ret = 0;
	int32_t retry = 0;

	while (1) {
		msleep(10);

		//---read reset state---
		buf[0] = EVENT_MAP_RESET_COMPLETE;
		buf[1] = 0x00;
		CTP_I2C_READ(ts->client, I2C_FW_Address, buf, 6);

		if ((buf[1] >= check_reset_state) && (buf[1] <= RESET_STATE_MAX)) {
			ret = 0;
			break;
		}

		retry++;
		if(unlikely(retry > 100)) {
			pr_err("error, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X\n",
				buf[1], buf[2], buf[3], buf[4], buf[5]);
			ret = -1;
			break;
		}
	}

	return ret;
}

/*******************************************************
Description:
	Novatek touchscreen get novatek project id information
	function.

return:
	Executive outcomes. 0---success. -1---fail.
*******************************************************/
int32_t nvt_read_pid(void)
{
	uint8_t buf[3] = {0};
	int32_t ret = 0;

	//---set xdata index to EVENT BUF ADDR---
	buf[0] = 0xFF;
	buf[1] = (ts->mmap->EVENT_BUF_ADDR >> 16) & 0xFF;
	buf[2] = (ts->mmap->EVENT_BUF_ADDR >> 8) & 0xFF;
	CTP_I2C_WRITE(ts->client, I2C_FW_Address, buf, 3);

	//---read project id---
	buf[0] = EVENT_MAP_PROJECTID;
	buf[1] = 0x00;
	buf[2] = 0x00;
	CTP_I2C_READ(ts->client, I2C_FW_Address, buf, 3);

	ts->nvt_pid = (buf[2] << 8) + buf[1];

	pr_debug("PID=%04X\n", ts->nvt_pid);

	return ret;
}

/*******************************************************
Description:
	Novatek touchscreen get firmware related information
	function.

return:
	Executive outcomes. 0---success. -1---fail.
*******************************************************/
int32_t nvt_get_fw_info(void)
{
	uint8_t buf[64] = {0};
	uint32_t retry_count = 0;
	int32_t ret = 0;

info_retry:
	//---set xdata index to EVENT BUF ADDR---
	buf[0] = 0xFF;
	buf[1] = (ts->mmap->EVENT_BUF_ADDR >> 16) & 0xFF;
	buf[2] = (ts->mmap->EVENT_BUF_ADDR >> 8) & 0xFF;
	CTP_I2C_WRITE(ts->client, I2C_FW_Address, buf, 3);

	//---read fw info---
	buf[0] = EVENT_MAP_FWINFO;
	CTP_I2C_READ(ts->client, I2C_FW_Address, buf, 17);
	ts->fw_ver = buf[1];
	ts->x_num = buf[3];
	ts->y_num = buf[4];
	ts->abs_x_max = (uint16_t)((buf[5] << 8) | buf[6]);
	ts->abs_y_max = (uint16_t)((buf[7] << 8) | buf[8]);
	ts->max_button_num = buf[11];

	//---clear x_num, y_num if fw info is broken---
	if ((buf[1] + buf[2]) != 0xFF) {
		pr_err("FW info is broken! fw_ver=0x%02X, ~fw_ver=0x%02X\n",
			buf[1], buf[2]);
		ts->fw_ver = 0;
		ts->x_num = 16;
		ts->y_num = 36;
		ts->abs_x_max = TOUCH_DEFAULT_MAX_WIDTH;
		ts->abs_y_max = TOUCH_DEFAULT_MAX_HEIGHT;
		ts->max_button_num = 0;

		if(retry_count < 3) {
			retry_count++;
			pr_err("retry_count=%d\n", retry_count);
			goto info_retry;
		} else {
			pr_err("fw_ver=%d, x=%d, y=%d, x_m=%d, y_m=%d, btn=%d\n",
				ts->fw_ver, ts->x_num, ts->y_num, ts->abs_x_max,
				ts->abs_y_max, ts->max_button_num);
			ret = -1;
		}
	} else {
		ret = 0;
	}

	//---Get Novatek PID---
	nvt_read_pid();

	return ret;
}

/*******************************************************
  Create Device Node (Proc Entry)
*******************************************************/
#if NVT_TOUCH_PROC
static struct proc_dir_entry *NVT_proc_entry;
#define DEVICE_NAME	"NVTflash"

/*******************************************************
Description:
	Novatek touchscreen /proc/NVTflash read function.

return:
	Executive outcomes. 2---succeed. -5,-14---failed.
*******************************************************/
static ssize_t nvt_flash_read(struct file *file, char __user *buff, size_t count, loff_t *offp)
{
	uint8_t str[68] = {0};
	int32_t ret = -1;
	int32_t retries = 0;
	int8_t i2c_wr = 0;

	if (count > sizeof(str)) {
		pr_err("error count=%zu\n", count);
		return -EFAULT;
	}

	if (copy_from_user(str, buff, count)) {
		pr_err("copy from user error\n");
		return -EFAULT;
	}

	i2c_wr = str[0] >> 7;

	if (i2c_wr == 0) {	//I2C write
		while (retries < 20) {
			ret = CTP_I2C_WRITE(ts->client, (str[0] & 0x7F), &str[2], str[1]);
			if (ret == 1)
				break;
			else
				pr_err("error, retries=%d, ret=%d\n",
					retries, ret);

			retries++;
		}

		if (unlikely(retries == 20)) {
			pr_err("error, ret = %d\n", ret);
			return -EIO;
		}

		return ret;
	} else if (i2c_wr == 1) {	//I2C read
		while (retries < 20) {
			ret = CTP_I2C_READ(ts->client, (str[0] & 0x7F), &str[2], str[1]);
			if (ret == 2)
				break;
			else
				pr_err("error, retries=%d, ret=%d\n",
					retries, ret);

			retries++;
		}

		// copy buff to user if i2c transfer
		if (retries < 20) {
			if (copy_to_user(buff, str, count))
				return -EFAULT;
		}

		if (unlikely(retries == 20)) {
			pr_err("error, ret = %d\n", ret);
			return -EIO;
		}

			return ret;
	} else {
		pr_err("Call error, str[0]=%d\n", str[0]);
			return -EFAULT;
	}
}

/*******************************************************
Description:
	Novatek touchscreen /proc/NVTflash open function.

return:
	Executive outcomes. 0---succeed. -12---failed.
*******************************************************/
static int32_t nvt_flash_open(struct inode *inode, struct file *file)
{
	struct nvt_flash_data *dev;

	dev = kmalloc(sizeof(struct nvt_flash_data), GFP_KERNEL);
	if (dev == NULL) {
		pr_err("Failed to allocate memory for nvt flash data\n");
		return -ENOMEM;
	}

	rwlock_init(&dev->lock);
	file->private_data = dev;

	return 0;
}

/*******************************************************
Description:
	Novatek touchscreen /proc/NVTflash close function.

return:
	Executive outcomes. 0---succeed.
*******************************************************/
static int32_t nvt_flash_close(struct inode *inode, struct file *file)
{
	struct nvt_flash_data *dev = file->private_data;

	if (dev)
		kfree(dev);

	return 0;
}

static const struct file_operations nvt_flash_fops = {
	.owner = THIS_MODULE,
	.open = nvt_flash_open,
	.release = nvt_flash_close,
	.read = nvt_flash_read,
};

/*******************************************************
Description:
	Novatek touchscreen /proc/NVTflash initial function.

return:
	Executive outcomes. 0---succeed. -12---failed.
*******************************************************/
static int32_t nvt_flash_proc_init(void)
{
	NVT_proc_entry = proc_create(DEVICE_NAME, 0444, NULL,&nvt_flash_fops);
	if (NVT_proc_entry == NULL) {
		pr_err("Failed!\n");
		return -ENOMEM;
	} else {
		pr_debug("Succeeded Create /proc/NVTflash\n");
	}

	return 0;
}
#endif

/*******************************************************
Description:
	Novatek touchscreen parse device tree function.

return:
	n.a.
*******************************************************/
#ifdef CONFIG_OF
static void nvt_parse_dt(struct device *dev)
{
	struct device_node *np = dev->of_node;

	ts->irq_gpio = of_get_named_gpio_flags(np, "novatek,irq-gpio", 0, &ts->irq_flags);
	pr_debug("novatek,irq-gpio=%d\n", ts->irq_gpio);

}
#else
static void nvt_parse_dt(struct device *dev)
{
	ts->irq_gpio = NVTTOUCH_INT_PIN;
}
#endif

/*******************************************************
Description:
	Novatek touchscreen config and request gpio

return:
	Executive outcomes. 0---succeed. not 0---failed.
*******************************************************/
static int nvt_gpio_config(struct nvt_ts_data *ts)
{
	int32_t ret = 0;

	/* request INT-pin (Input) */
	if (gpio_is_valid(ts->irq_gpio)) {
		ret = gpio_request_one(ts->irq_gpio, GPIOF_IN, "NVT-int");
		if (ret) {
			pr_err("Failed to request NVT-int GPIO\n");
			goto err_request_irq_gpio;
		}
	}

	return ret;

err_request_irq_gpio:
	return ret;
}

#define POINT_DATA_LEN 65
/*******************************************************
Description:
	Novatek touchscreen work function.

return:
	n.a.
*******************************************************/
static void nvt_ts_work_func(struct work_struct *work)
{
	int32_t ret = -1;
	uint8_t point_data[POINT_DATA_LEN + 1] = {0};
	uint32_t position = 0;
	uint32_t input_x = 0;
	uint32_t input_y = 0;
	uint32_t input_w = 0;
	uint32_t input_p = 0;
	uint8_t input_id = 0;
#if MT_PROTOCOL_B
	uint8_t press_id[TOUCH_MAX_FINGER_NUM] = {0};
#endif /* MT_PROTOCOL_B */
	int32_t i = 0;
	int32_t finger_cnt = 0;

	mutex_lock(&ts->lock);

	ret = CTP_I2C_READ(ts->client, I2C_FW_Address, point_data, POINT_DATA_LEN + 1);
	if (ret < 0) {
		pr_err("CTP_I2C_READ failed.(%d)\n", ret);
		goto XFER_ERROR;
	}

	finger_cnt = 0;

	for (i = 0; i < ts->max_touch_num; i++) {
		position = 1 + 6 * i;
		input_id = (uint8_t)(point_data[position + 0] >> 3);
		if ((input_id == 0) || (input_id > ts->max_touch_num))
			continue;

		if (((point_data[position] & 0x07) == 0x01) || ((point_data[position] & 0x07) == 0x02)) {	//finger down (enter & moving)
			input_x = (uint32_t)(point_data[position + 1] << 4) + (uint32_t) (point_data[position + 3] >> 4);
			input_y = (uint32_t)(point_data[position + 2] << 4) + (uint32_t) (point_data[position + 3] & 0x0F);
			if ((input_x < 0) || (input_y < 0))
				continue;
			if ((input_x > ts->abs_x_max) || (input_y > ts->abs_y_max))
				continue;
			input_w = (uint32_t)(point_data[position + 4]);
			if (input_w == 0)
				input_w = 1;
			if (i < 2) {
				input_p = (uint32_t)(point_data[position + 5]) + (uint32_t)(point_data[i + 63] << 8);
				if (input_p > TOUCH_FORCE_NUM)
					input_p = TOUCH_FORCE_NUM;
			} else {
				input_p = (uint32_t)(point_data[position + 5]);
			}
			if (input_p == 0)
				input_p = 1;

#if MT_PROTOCOL_B
			press_id[input_id - 1] = 1;
			input_mt_slot(ts->input_dev, input_id - 1);
			input_mt_report_slot_state(ts->input_dev, MT_TOOL_FINGER, true);
#else /* MT_PROTOCOL_B */
			input_report_abs(ts->input_dev, ABS_MT_TRACKING_ID, input_id - 1);
#if !TP_DISABLE_BTN
			input_report_key(ts->input_dev, BTN_TOUCH, 1);
#endif
#endif /* MT_PROTOCOL_B */

			input_report_abs(ts->input_dev, ABS_MT_POSITION_X, input_x);
			input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, input_y);
			input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, input_w);
			input_report_abs(ts->input_dev, ABS_MT_PRESSURE, input_p);

#if MT_PROTOCOL_B
#else /* MT_PROTOCOL_B */
			input_mt_sync(ts->input_dev);
#endif /* MT_PROTOCOL_B */
			pr_debug("Finger %d: x = %d, y = %d, wx = %d, p = %d\n",
					i, input_x, input_y, input_w, input_p);

			last_input_x = input_x;
			last_input_y = input_y;

			finger_cnt++;
		}
	}

#if MT_PROTOCOL_B
	for (i = 0; i < ts->max_touch_num; i++) {
		if (press_id[i] != 1) {
			input_mt_slot(ts->input_dev, i);
			input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, 0);
			input_report_abs(ts->input_dev, ABS_MT_PRESSURE, 0);
			input_mt_report_slot_state(ts->input_dev, MT_TOOL_FINGER, false);
		}
	}

#if !TP_DISABLE_BTN
	input_report_key(ts->input_dev, BTN_TOUCH, (finger_cnt > 0));
#endif

	if (finger_cnt == 0) {
		pr_debug("Number of fingers to process = %d (%d, %d)\n",
			finger_cnt, last_input_x, last_input_y);
	} else {
		pr_debug("Number of fingers to process = %d\n", finger_cnt);
	}
#else /* MT_PROTOCOL_B */
	if (finger_cnt == 0) {
#if !TP_DISABLE_BTN
		input_report_key(ts->input_dev, BTN_TOUCH, 0);
#endif
		input_mt_sync(ts->input_dev);
	}
#endif /* MT_PROTOCOL_B */

	input_sync(ts->input_dev);

XFER_ERROR:
	enable_irq(ts->client->irq);

	mutex_unlock(&ts->lock);
}

/*******************************************************
Description:
	External interrupt service routine.

return:
	irq execute status.
*******************************************************/
static irqreturn_t nvt_ts_irq_handler(int32_t irq, void *dev_id)
{
	disable_irq_nosync(ts->client->irq);

	queue_work(nvt_wq, &ts->nvt_work);

	return IRQ_HANDLED;
}

/*******************************************************
Description:
	Novatek touchscreen check and stop crc reboot loop.

return:
	n.a.
*******************************************************/
void nvt_stop_crc_reboot(void)
{
	uint8_t buf[8] = {0};
	int32_t retry = 0;

	//read dummy buffer to check CRC fail reboot is happening or not

	//---change I2C index to prevent geting 0xFF, but not 0xFC---
	buf[0] = 0xFF;
	buf[1] = 0x01;
	buf[2] = 0xF6;
	CTP_I2C_WRITE(ts->client, I2C_BLDR_Address, buf, 3);

	//---read to check if buf is 0xFC which means IC is in CRC reboot ---
	buf[0] = 0x4E;
	CTP_I2C_READ(ts->client, I2C_BLDR_Address, buf, 4);

	if ((buf[1] == 0xFC) ||
		((buf[1] == 0xFF) && (buf[2] == 0xFF) && (buf[3] == 0xFF))) {

		//IC is in CRC fail reboot loop, needs to be stopped!
		for (retry = 5; retry > 0; retry--) {

			//---write i2c cmds to reset idle : 1st---
			buf[0]=0x00;
			buf[1]=0xA5;
			CTP_I2C_WRITE(ts->client, I2C_HW_Address, buf, 2);

			//---write i2c cmds to reset idle : 2rd---
			buf[0]=0x00;
			buf[1]=0xA5;
			CTP_I2C_WRITE(ts->client, I2C_HW_Address, buf, 2);
			msleep(1);

			//---clear CRC_ERR_FLAG---
			buf[0] = 0xFF;
			buf[1] = 0x03;
			buf[2] = 0xF1;
			CTP_I2C_WRITE(ts->client, I2C_BLDR_Address, buf, 3);

			buf[0] = 0x35;
			buf[1] = 0xA5;
			CTP_I2C_WRITE(ts->client, I2C_BLDR_Address, buf, 2);

			//---check CRC_ERR_FLAG---
			buf[0] = 0xFF;
			buf[1] = 0x03;
			buf[2] = 0xF1;
			CTP_I2C_WRITE(ts->client, I2C_BLDR_Address, buf, 3);

			buf[0] = 0x35;
			buf[1] = 0x00;
			CTP_I2C_READ(ts->client, I2C_BLDR_Address, buf, 2);

			if (buf[1] == 0xA5)
				break;
		}
		if (retry == 0)
			pr_err("CRC auto reboot can't stopped! buf[1]=0x%02X\n",
				buf[1]);
	}

	return;
}

/*******************************************************
Description:
	Novatek touchscreen check chip version trim function.

return:
	Executive outcomes. 0---NVT IC. -1---not NVT IC.
*******************************************************/
static int8_t nvt_ts_check_chip_ver_trim(void)
{
	uint8_t buf[8] = {0};
	int32_t retry = 0;
	int32_t list = 0;
	int32_t i = 0;
	int32_t found_nvt_chip = 0;
	int32_t ret = -1;

	nvt_bootloader_reset(); // NOT in retry loop

	//---Check for 5 times---
	for (retry = 5; retry > 0; retry--) {
		nvt_sw_reset_idle();

		buf[0] = 0x00;
		buf[1] = 0x35;
		CTP_I2C_WRITE(ts->client, I2C_HW_Address, buf, 2);
		msleep(10);

		buf[0] = 0xFF;
		buf[1] = 0x01;
		buf[2] = 0xF6;
		CTP_I2C_WRITE(ts->client, I2C_BLDR_Address, buf, 3);

		buf[0] = 0x4E;
		buf[1] = 0x00;
		buf[2] = 0x00;
		buf[3] = 0x00;
		buf[4] = 0x00;
		buf[5] = 0x00;
		buf[6] = 0x00;
		CTP_I2C_READ(ts->client, I2C_BLDR_Address, buf, 7);
		pr_debug("buf = (0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X\n",
			buf[1], buf[2], buf[3], buf[4], buf[5], buf[6]);

		/* ---Stop CRC check to prevent IC auto reboot--- */
		if ((buf[1] == 0xFC) || ((buf[1] == 0xFF) && (buf[2] == 0xFF)
			&& (buf[3] == 0xFF))) {
			nvt_stop_crc_reboot();
			continue;
		}

		// compare read chip id on supported list
		for (list = 0; list < (sizeof(trim_id_table) / sizeof(struct nvt_ts_trim_id_table)); list++) {
			found_nvt_chip = 0;

			// compare each byte
			for (i = 0; i < NVT_ID_BYTE_MAX; i++) {
				if (trim_id_table[list].mask[i]) {
					if (buf[i + 1] != trim_id_table[list].id[i])
						break;
				}
			}

			if (i == NVT_ID_BYTE_MAX) {
				found_nvt_chip = 1;
			}

			if (found_nvt_chip) {
				pr_debug("This is NVT touch IC\n");
				ts->mmap = trim_id_table[list].mmap;
				ts->carrier_system = trim_id_table[list].carrier_system;
				ret = 0;
				goto out;
			} else {
				ts->mmap = NULL;
				ret = -1;
			}
		}

		msleep(10);
	}

out:
	return ret;
}

/*******************************************************
Description:
	Novatek touchscreen driver probe function.

return:
	Executive outcomes. 0---succeed. negative---failed
*******************************************************/
static int32_t nvt_ts_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int32_t ret = 0;
	unsigned int attr_count, attr_num;

	tp_probe_log_index = 0;

	pr_debug("probe start\n");

	ts = kmalloc(sizeof(struct nvt_ts_data), GFP_KERNEL);
	if (ts == NULL) {
		pr_err("failed to allocated memory for nvt ts data\n");
		return -ENOMEM;
	} else {
		pr_debug("allocated nvt_ts_data ok\n");
	}

	ts->client = client;
	i2c_set_clientdata(client, ts);

	//---parse dts---
	nvt_parse_dt(&client->dev);

	//---request and config GPIOs---
	ret = nvt_gpio_config(ts);
	if (ret) {
		pr_err("gpio config error!\n");
		goto err_gpio_config_failed;
	} else {
		pr_debug("gpio config ok\n");
	}

	//---check i2c func.---
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		pr_err("i2c_check_functionality failed. (no I2C_FUNC_I2C)\n");
		ret = -ENODEV;
		goto err_check_functionality_failed;
	} else {
		pr_debug("i2c check func ok\n");
	}

	// need 10ms delay after POR(power on reset)
	msleep(10);

	//---check chip version trim---
	ret = nvt_ts_check_chip_ver_trim();
	if (ret) {
		pr_err("chip couldn't identified\n");
		ret = -EINVAL;
		goto err_chipvertrim_failed;
	} else {
		pr_debug("This is NVT chip\n");
	}

	mutex_init(&ts->lock);
#if NVT_IRQ_CONTROL
	mutex_init(&ts->irq_lock);
#endif
	mutex_lock(&ts->lock);
	nvt_bootloader_reset();
	nvt_check_fw_reset_state(RESET_STATE_INIT);
	nvt_get_fw_info();
	mutex_unlock(&ts->lock);

	/*---For SMx3 use start---*/
	pr_debug("ts->nvt_pid = 0x%04X\n", ts->nvt_pid);

	/* Get GPIO LCD ID */
	ts->lcd_id = gpio_get_value(TP_READ_LCDID_PIN);

	ts->tp_source = TP_SOURCE_UNKNOWN;

#if NVT_PROJECT_INFO_READ
	mutex_lock(&ts->lock);

	/* Get external flash id (project info) */
	nvt_read_project_info();
	mutex_unlock(&ts->lock);

	/* get tp_source */
	ts->tp_source = smx3_check_project_info();
#endif /* NVT_PROJECT_INFO_READ */

	pr_debug("lcd_id = %d, source = %s (0x%02X)\n",
		ts->lcd_id, smx3_tp_source_to_text(), ts->tp_source);
	/*---For SMx3 use end---*/

	//---create workqueue---
	nvt_wq = create_workqueue("nvt_wq");
	if (!nvt_wq) {
		pr_err("nvt_wq create workqueue failed\n");
		ret = -ENOMEM;
		goto err_create_nvt_wq_failed;
	} else {
		pr_debug("create nvt touch wq ok\n");
	}
	INIT_WORK(&ts->nvt_work, nvt_ts_work_func);

	//---allocate input device---
	ts->input_dev = input_allocate_device();
	if (ts->input_dev == NULL) {
		pr_err("allocate input device failed\n");
		ret = -ENOMEM;
		goto err_input_dev_alloc_failed;
	} else {
		pr_debug("allocate input_dev ok\n");
	}

	ts->max_touch_num = TOUCH_MAX_FINGER_NUM;

	ts->int_trigger_type = INT_TRIGGER_TYPE;

	//---set input device info.---
	ts->input_dev->evbit[0] = BIT_MASK(EV_SYN) | BIT_MASK(EV_KEY) | BIT_MASK(EV_ABS) ;
#if !TP_DISABLE_BTN
	ts->input_dev->keybit[BIT_WORD(BTN_TOUCH)] = BIT_MASK(BTN_TOUCH);
#endif
	ts->input_dev->propbit[0] = BIT(INPUT_PROP_DIRECT);

#if MT_PROTOCOL_B
	input_mt_init_slots(ts->input_dev, ts->max_touch_num, 0);
#endif

	input_set_abs_params(ts->input_dev, ABS_MT_PRESSURE, 0, TOUCH_FORCE_NUM, 0, 0);    //pressure = TOUCH_FORCE_NUM

#if TOUCH_MAX_FINGER_NUM > 1
	input_set_abs_params(ts->input_dev, ABS_MT_TOUCH_MAJOR, 0, 255, 0, 0);    //area = 255

	input_set_abs_params(ts->input_dev, ABS_MT_POSITION_X, 0, ts->abs_x_max, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_POSITION_Y, 0, ts->abs_y_max, 0, 0);
#if MT_PROTOCOL_B
	// no need to set ABS_MT_TRACKING_ID, input_mt_init_slots() already set it
#else
	input_set_abs_params(ts->input_dev, ABS_MT_TRACKING_ID, 0, ts->max_touch_num, 0, 0);
#endif //MT_PROTOCOL_B
#endif //TOUCH_MAX_FINGER_NUM > 1

	sprintf(ts->phys, "input/ts");
	ts->input_dev->name = NVT_TS_NAME;
	ts->input_dev->phys = ts->phys;
	ts->input_dev->id.bustype = BUS_I2C;

	//---register input device---
	ret = input_register_device(ts->input_dev);
	if (ret) {
		pr_err("register input device (%s) failed. ret=%d\n",
			ts->input_dev->name, ret);
		goto err_input_register_device_failed;
	} else {
		pr_debug("register input_dev ok\n");
	}

	//---set int-pin & request irq---
	client->irq = gpio_to_irq(ts->irq_gpio);
	if (client->irq) {
		pr_debug("int_trigger_type=%d\n", ts->int_trigger_type);

		ret = request_irq(client->irq, nvt_ts_irq_handler, ts->int_trigger_type, client->name, ts);
		if (ret != 0) {
			pr_err("request irq failed. ret=%d\n", ret);
			goto err_int_request_failed;
		} else {
			disable_irq(client->irq);
			pr_debug("request irq %d succeed\n", client->irq);
		}
	}

#if BOOT_UPDATE_FIRMWARE
	ts->sw_fw_ver = 0xFF;
	nvt_fwu_wq = create_singlethread_workqueue("nvt_fwu_wq");
	if (!nvt_fwu_wq) {
		pr_err("nvt_fwu_wq create workqueue failed\n");
		ret = -ENOMEM;
		goto err_create_nvt_fwu_wq_failed;
	} else {
		pr_debug("create nvt FW upgrade upgrade wq ok\n");
	}
	INIT_WORK(&ts->nvt_fwu_work, Boot_Update_Firmware);
	// please make sure boot update start after display reset(RESX) sequence
	queue_work(nvt_fwu_wq, &ts->nvt_fwu_work);
#endif

	//---set device node---
#if NVT_TOUCH_PROC
	ret = nvt_flash_proc_init();
	if (ret != 0) {
		pr_err("nvt flash proc init failed. ret=%d\n", ret);
		goto err_init_NVT_ts;
	} else {
		pr_debug("create nvt flash proc node ok\n");
	}
#endif

#if NVT_TOUCH_EXT_PROC
	ret = nvt_extra_proc_init();
	if (ret != 0) {
		pr_err("nvt extra proc init failed. ret=%d\n", ret);
		goto err_init_NVT_ts;
	} else {
		pr_debug("create nvt extra proc node ok\n");
	}
#endif

#if NVT_TOUCH_MP
	ret = nvt_mp_proc_init();
	if (ret != 0) {
		pr_err("nvt mp proc init failed. ret=%d\n", ret);
		goto err_init_NVT_ts;
	} else {
		pr_debug("create nvt mp proc node ok\n");
	}
#endif

#if defined(CONFIG_FB) && !defined(CONFIG_DRM_MSM_DSI_SOMC_PANEL)
	ts->fb_notif.notifier_call = fb_notifier_callback;
	ret = fb_register_client(&ts->fb_notif);
	if(ret) {
		pr_err("register fb_notifier failed. ret=%d\n", ret);
		goto err_register_fb_notif_failed;
	} else {
		pr_debug("register fb_notifier ok\n");
	}
#elif defined(CONFIG_FB) && defined(CONFIG_DRM_MSM_DSI_SOMC_PANEL)
	ts->fb_notif.notifier_call = drm_notifier_callback;
	ret = drm_register_client(&ts->fb_notif);
	if(ret) {
		pr_err("register drm_notifier failed. ret=%d\n", ret);
		goto err_register_fb_notif_failed;
	} else {
		pr_debug("register drm_notifier ok\n");
	}
#endif

	/*---For SMx3 use start---*/
	/* create sysfs node for touch driver */
	attr_num = ARRAY_SIZE(attrs);
	for (attr_count = 0; attr_count < attr_num; attr_count++) {
		ret = sysfs_create_file(&ts->input_dev->dev.kobj,
				&attrs[attr_count].attr);
		if (ret < 0) {
			pr_err("Failed to create sysfs attributes\n");
			attr_num = attr_count;
			goto err_sysfs;
		}
	}

	pr_debug("create sysfs attributes ok\n");

	/* create sysfs link to input dev */
	ts->touchscreen_link = kobject_create_and_add("touchscreen", NULL);
	if (ts->touchscreen_link != NULL) {
		ret = sysfs_create_link(ts->touchscreen_link,
				&ts->input_dev->dev.kobj,
				"link_input_dev");
		if (ret < 0) {
			pr_err("Failed to create sysfs link for input_dev\n");
		} else {
			pr_debug("create sysfs link for input_dev ok\n");
		}
	}
	/*---For SMx3 use end---*/

	bTouchIsAwake = 1;
	pr_debug("probe end : success\n");

	enable_irq(client->irq);

	return 0;

err_sysfs:
	for (attr_count = 0; attr_count < attr_num; attr_count++) {
		sysfs_remove_file(&ts->input_dev->dev.kobj,
				&attrs[attr_count].attr);
	}
#if defined(CONFIG_FB)
err_register_fb_notif_failed:
#endif
#if (NVT_TOUCH_PROC || NVT_TOUCH_EXT_PROC || NVT_TOUCH_MP)
err_init_NVT_ts:
#endif
	free_irq(client->irq, ts);
#if BOOT_UPDATE_FIRMWARE
err_create_nvt_fwu_wq_failed:
#endif
err_int_request_failed:
err_input_register_device_failed:
	input_free_device(ts->input_dev);
err_input_dev_alloc_failed:
err_create_nvt_wq_failed:
	mutex_destroy(&ts->lock);
err_chipvertrim_failed:
err_check_functionality_failed:
	gpio_free(ts->irq_gpio);
err_gpio_config_failed:
	i2c_set_clientdata(client, NULL);
	kfree(ts);

	pr_debug("probe end : failed\n");

	smx3_probe_log_proc_init();

	return ret;
}

/*******************************************************
Description:
	Novatek touchscreen driver release function.

return:
	Executive outcomes. 0---succeed.
*******************************************************/
static int32_t nvt_ts_remove(struct i2c_client *client)
{
	//struct nvt_ts_data *ts = i2c_get_clientdata(client);

#if defined(CONFIG_FB) && !defined(CONFIG_DRM_MSM_DSI_SOMC_PANEL)
	if (fb_unregister_client(&ts->fb_notif))
		pr_err("Error occurred while unregistering fb_notifier.\n");
#elif defined(CONFIG_FB) && defined(CONFIG_DRM_MSM_DSI_SOMC_PANEL)
	if (drm_unregister_client(&ts->fb_notif))
		pr_err("Error occurred while unregistering DRM notifier.\n");
#endif

	mutex_destroy(&ts->lock);

	pr_debug("Removing driver...\n");

	free_irq(client->irq, ts);
	input_unregister_device(ts->input_dev);
	i2c_set_clientdata(client, NULL);
	kfree(ts);

	return 0;
}

/*******************************************************
Description:
	Novatek touchscreen driver suspend function.

return:
	Executive outcomes. 0---succeed.
*******************************************************/
static int32_t nvt_ts_suspend(struct device *dev)
{
	uint8_t buf[4] = {0};
#if MT_PROTOCOL_B
	uint32_t i = 0;
#endif

	if (!bTouchIsAwake) {
		pr_info("Touch is already suspend\n");
		return 0;
	}

	mutex_lock(&ts->lock);

	bTouchIsAwake = 0;

	disable_irq(ts->client->irq);

	//---write i2c command to enter "deep sleep mode"---
	buf[0] = EVENT_MAP_HOST_CMD;
	buf[1] = 0x11;
	CTP_I2C_WRITE(ts->client, I2C_FW_Address, buf, 2);

	/* release all touches */
#if MT_PROTOCOL_B
	for (i = 0; i < ts->max_touch_num; i++) {
		input_mt_slot(ts->input_dev, i);
		input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, 0);
		input_report_abs(ts->input_dev, ABS_MT_PRESSURE, 0);
		input_mt_report_slot_state(ts->input_dev, MT_TOOL_FINGER, 0);
	}
#endif
#if !TP_DISABLE_BTN
	input_report_key(ts->input_dev, BTN_TOUCH, 0);
#endif
#if !MT_PROTOCOL_B
	input_mt_sync(ts->input_dev);
#endif
	input_sync(ts->input_dev);

	msleep(50);

	mutex_unlock(&ts->lock);

	return 0;
}

/*******************************************************
Description:
	Novatek touchscreen driver resume function.

return:
	Executive outcomes. 0---succeed.
*******************************************************/
static int32_t nvt_ts_resume(struct device *dev)
{
	if (bTouchIsAwake) {
		pr_info("Touch is already resume\n");
		return 0;
	}

	mutex_lock(&ts->lock);

	// please make sure display reset(RESX) sequence and mipi dsi cmds sent before this
	nvt_bootloader_reset();
	nvt_check_fw_reset_state(RESET_STATE_REK);

	enable_irq(ts->client->irq);

	bTouchIsAwake = 1;

	mutex_unlock(&ts->lock);

	pr_debug("irq depth : %d\n", smx3_get_irq_depth());

	return 0;
}

#if defined(CONFIG_FB) && !defined(CONFIG_DRM_MSM_DSI_SOMC_PANEL)
static int fb_notifier_callback(struct notifier_block *self, unsigned long event, void *data)
{
	struct fb_event *evdata = data;
	int *blank;
	struct nvt_ts_data *ts =
		container_of(self, struct nvt_ts_data, fb_notif);

	if (evdata && evdata->data && event == FB_EARLY_EVENT_BLANK) {
		blank = evdata->data;
		if (*blank == FB_BLANK_POWERDOWN) {
			nvt_ts_suspend(&ts->client->dev);
		}
	} else if (evdata && evdata->data && event == FB_EVENT_BLANK) {
		blank = evdata->data;
		if (*blank == FB_BLANK_UNBLANK) {
			nvt_ts_resume(&ts->client->dev);
		}
	}

	return 0;
}
#elif defined(CONFIG_FB) && defined(CONFIG_DRM_MSM_DSI_SOMC_PANEL)
static int drm_notifier_callback(struct notifier_block *self, unsigned long event, void *data)
{
	struct drm_ext_event *evdata = data;
	int transition;
	struct nvt_ts_data *ts =
		container_of(self, struct nvt_ts_data, fb_notif);

	if (unlikely(!evdata))
		return -EINVAL;

	if (unlikely(!evdata->data))
		return -EINVAL;

	transition = *(int *)evdata->data;

	if (event == DRM_EXT_EVENT_AFTER_BLANK &&
	    transition == DRM_BLANK_UNBLANK) {
		nvt_ts_resume(&ts->client->dev);
	} else if (event == DRM_EXT_EVENT_BEFORE_BLANK &&
		   transition == DRM_BLANK_POWERDOWN) {
		nvt_ts_suspend(&ts->client->dev);
	}

	return 0;
}
#endif

static const struct i2c_device_id nvt_ts_id[] = {
	{ NVT_I2C_NAME, 0 },
	{ }
};

#ifdef CONFIG_OF
static struct of_device_id nvt_match_table[] = {
	{ .compatible = "novatek,NVT-ts",},
	{ },
};
#endif

static struct i2c_driver nvt_i2c_driver = {
	.probe		= nvt_ts_probe,
	.remove		= nvt_ts_remove,
//	.suspend	= nvt_ts_suspend,
//	.resume		= nvt_ts_resume,
	.id_table	= nvt_ts_id,
	.driver = {
		.name	= NVT_I2C_NAME,
		.owner	= THIS_MODULE,
#ifdef CONFIG_OF
		.of_match_table = nvt_match_table,
#endif
	},
};

/*******************************************************
Description:
	Driver Install function.

return:
	Executive Outcomes. 0---succeed. not 0---failed.
********************************************************/
static int32_t __init nvt_driver_init(void)
{
	int32_t ret = 0;

	//---add i2c driver---
	ret = i2c_add_driver(&nvt_i2c_driver);
	if (ret) {
		pr_err("failed to add i2c driver\n");
		goto err_driver;
	}

err_driver:
	return ret;
}

/*******************************************************
Description:
	Driver uninstall function.

return:
	n.a.
********************************************************/
static void __exit nvt_driver_exit(void)
{
	i2c_del_driver(&nvt_i2c_driver);

	if (nvt_wq)
		destroy_workqueue(nvt_wq);

#if BOOT_UPDATE_FIRMWARE
	if (nvt_fwu_wq)
		destroy_workqueue(nvt_fwu_wq);
#endif
}

late_initcall(nvt_driver_init);
//module_init(nvt_driver_init);
module_exit(nvt_driver_exit);

MODULE_DESCRIPTION("Novatek Touchscreen Driver");
MODULE_LICENSE("GPL");
