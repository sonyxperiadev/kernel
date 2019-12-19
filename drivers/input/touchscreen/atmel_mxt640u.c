/*
 * Atmel maXTouch Touchscreen driver
 *
 * Copyright (C) 2010 Samsung Electronics Co.Ltd
 * Copyright (C) 2011-2012 Atmel Corporation
 * Copyright (C) 2012 Google, Inc.
 * Copyright (C) 2013 LG Electronics, Inc.
 * Copyright (C) 2017, 2018 Sony Mobile Communications Inc.
 *
 * Author: <WX-BSP-TS@lge.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/completion.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/regulator/consumer.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/time.h>
#include <linux/timer.h>
#include <linux/mutex.h>
#include <linux/uaccess.h>
#include <linux/file.h>
#include <linux/async.h>
#include <linux/irq.h>
#include <linux/string.h>
#ifdef CONFIG_LGE_PM
#include <linux/power_supply.h>
#endif
#include <linux/pinctrl/consumer.h>

#define CONFIG_TOUCHSCREEN_ATMEL_STYLUS_PEN

#include "atmel_mxt640u.h"
#include "atmel_mxt640u_patch.h"
#define LGE_TOUCH_NAME      "lge_touch"

#define TCI_FAIL_NUM 		8

#define MXT_DEFAULT_LOG_LEVEL LEVEL_DEFAULT
#define MXT_DEFAULT_SIDEKEY_STATUS SIDE_KEY_DISABLE

int check_current_temp_mode(struct mxt_data *data, int current_temp);
void select_temp_event_knock_on(struct mxt_data *data, int temp_mode, int lpwg_mode);
static int drm_notifier_callback(struct notifier_block *self, unsigned long event, void *data);

DECLARE_COMPLETION(touch_charge_out_comp);

static struct bus_type touch_subsys = {
	 .name = LGE_TOUCH_NAME,
	 .dev_name = "lge_touch",
};

static struct device device_touch = {
	.id = 0,
	.bus = &touch_subsys,
};

struct lge_touch_attribute {
	struct attribute	attr;
	ssize_t (*show)(struct mxt_data *ts, char *buf);
	ssize_t (*store)(struct mxt_data *ts, const char *buf, size_t count);
};

#define LGE_TOUCH_ATTR(_name, _mode, _show, _store)	\
struct lge_touch_attribute lge_touch_attr_##_name = __ATTR(_name, _mode, _show, _store)

#define jitter_abs(x)	(x > 0 ? x : -x)
#define jitter_sub(x, y)	(x > y ? x - y : y - x)
#define get_time_interval(a,b) a>=b ? a-b : 1000000+a-b

static int mxt_soft_reset(struct mxt_data *data);
static int mxt_hw_reset(struct mxt_data *data);
static int mxt_t6_command(struct mxt_data *data, u16 cmd_offset, u8 value, bool wait);
static int mxt_t109_command(struct mxt_data *data, u16 cmd_offset, u8 value);
static void mxt_reset_slots(struct mxt_data *data);
static void mxt_start(struct mxt_data *data);
static int mxt_set_t7_power_cfg(struct mxt_data *data, u8 sleep);
static void mxt_stop(struct mxt_data *data);
static int mxt_read_config_crc(struct mxt_data *data, u32 *crc);
static int mxt_command_backup(struct mxt_data *data, u8 value);
static int mxt_command_reset(struct mxt_data *data, u8 value);
static void mxt_regulator_disable(struct mxt_data *data);
static void mxt_regulator_enable(struct mxt_data *data);
static int mxt_drm_suspend(struct mxt_data *data);
static int mxt_drm_resume(struct mxt_data *data);
static int mxt_pw_lock(incell_pw_lock status);
static int mxt_get_pw_status(void);
static void mxt_set_side_key(struct mxt_data *data);
static void mxt_set_stamina_mode(struct mxt_data *data);
static void mxt_set_cover_mode(struct mxt_data *data);
static void mxt_set_glove_mode(struct mxt_data *data);

char *knockon_event[2] = { "TOUCH_GESTURE_WAKEUP=WAKEUP", NULL };
char *lpwg_event[2] = { "TOUCH_GESTURE_WAKEUP=PASSWORD", NULL };
static enum log_level_list *log_level = NULL;
static int portrait_buffer[MXT_GRIP_REJECTION_BORDER_NUM] = {40, 180, 120, 250};
static int landscape_buffer[MXT_GRIP_REJECTION_BORDER_NUM] = {200, 30, 300, 80};

inline static void send_uevent(char* string[2])
{
	int ret = 0;
	ret = kobject_uevent_env(&device_touch.kobj, KOBJ_CHANGE, string);
	LOGN("uevent[%s], ret:%d\n", string[0], ret);
}

static int mxt_init_log_level(struct mxt_data *data)
{
	if (log_level == NULL && data != NULL) {
		log_level = &(data->log_level);
		return 0;
	}

	return -EINVAL;
}

static enum hrtimer_restart tci_timer_func(struct hrtimer *multi_tap_timer)
{
	struct mxt_data *ts = container_of(multi_tap_timer, struct mxt_data, multi_tap_timer);

	queue_work(ts->mxt_drv_data->touch_multi_tap_wq, &ts->multi_tap_work);

	LOGD("TCI TIMER in\n");
	return HRTIMER_NORESTART;
}
static void mxt_active_mode_start(struct mxt_data *data);
static void mxt_gesture_mode_start(struct mxt_data *data);

#ifdef CONFIG_TOUCHSCREEN_ATMEL_LPWG_DEBUG_REASON
static void save_lpwg_debug_reason(struct mxt_data *data, u8 *value);
static void save_lpwg_debug_reason_print(struct mxt_data *data);
#endif
static void touch_multi_tap_work(struct work_struct *multi_tap_work)
{
	struct mxt_data *ts = container_of(multi_tap_work, struct mxt_data, multi_tap_work);

	LOGD("TCI WORK in\n");
	pm_relax(&ts->client->dev);
	pm_wakeup_event(&ts->client->dev, msecs_to_jiffies(TAP_WAIT_TIME));

	LOGD("T93 ENABLE LPWG\n");
	send_uevent(lpwg_event);
}

static void waited_udf(struct mxt_data *data, u8 *message)
{
	u8 status = message[1];
	if (!(status & MXT_T9_PRESS))
		return ;

	hrtimer_try_to_cancel(&data->multi_tap_timer);

	//multi tap enable
	LOGD("TCI over tap in\n");

	data->mxt_drv_data->tci_press[0].x = -1;	//for Error state
	data->mxt_drv_data->tci_press[0].y = -1;

	pm_relax(&data->client->dev);
	pm_stay_awake(&data->client->dev);

	if (!hrtimer_callback_running(&data->multi_tap_timer))
		hrtimer_start(&data->multi_tap_timer, ktime_set(0, MS_TO_NS(MXT_WAITED_UDF_TIME)), HRTIMER_MODE_REL);
}

static int touch_enable_irq_wake(struct mxt_data *data)
{
	int ret = 0;
	unsigned int irq = data->irq;

	mutex_lock(&data->mxt_drv_data->irq_lock);

	if (!data->mxt_drv_data->touch_irq_wake_mask) {
		data->mxt_drv_data->touch_irq_wake_mask = 1;
		ret = enable_irq_wake(irq);
		if (ret != 0)
			LOGE("%s : %d\n", __func__, ret);
		else
			LOGD("%s : %d\n", __func__, ret);
	}

	mutex_unlock(&data->mxt_drv_data->irq_lock);
	return ret;
}

static int touch_disable_irq_wake(struct mxt_data *data)
{
	int ret = 0;
	unsigned int irq = data->irq;

	mutex_lock(&data->mxt_drv_data->irq_lock);

	if (data->mxt_drv_data->touch_irq_wake_mask) {
		data->mxt_drv_data->touch_irq_wake_mask = 0;
		ret = disable_irq_wake(irq);
		if (ret != 0)
			LOGE("%s : %d\n", __func__, ret);
		else
			LOGD("%s : %d\n", __func__, ret);
	}

	mutex_unlock(&data->mxt_drv_data->irq_lock);

	return ret;
}

static bool mxt_get_touch_irq(struct mxt_data *data)
{
	return data->mxt_drv_data->touch_irq_mask;
}

static void mxt_set_touch_irq(struct mxt_data *data, bool status)
{
	data->mxt_drv_data->touch_irq_mask = status;
}

static void touch_enable_irq(struct mxt_data *data)
{
	unsigned int irq = data->irq;

	mutex_lock(&data->mxt_drv_data->irq_lock);

	if (!mxt_get_touch_irq(data)) {
		mxt_set_touch_irq(data, STATUS_ENABLE);
		enable_irq(irq);
		LOGD("irq enabled\n");
	} else {
		LOGD("irq is already enabled\n");
	}

	mutex_unlock(&data->mxt_drv_data->irq_lock);
}

static void touch_disable_irq(struct mxt_data *data)
{
	unsigned int irq = data->irq;

	mutex_lock(&data->mxt_drv_data->irq_lock);

	if (mxt_get_touch_irq(data)) {
		mxt_set_touch_irq(data, STATUS_DISABLE);
		disable_irq_nosync(irq);
		LOGD("irq disabled\n");
	} else {
		LOGD("irq is already disabled\n");
	}

	mutex_unlock(&data->mxt_drv_data->irq_lock);
}

static char mxt_power_block_get(struct mxt_data *data)
{
	return data->mxt_drv_data->power_block_mask;
}

static void mxt_power_block(struct mxt_data *data, char value)
{
	data->mxt_drv_data->power_block_mask |= value;
}

static void mxt_power_unblock(struct mxt_data *data, char value)
{
	data->mxt_drv_data->power_block_mask &= ~(value);
}

static char mxt_patchevent_get(struct mxt_data *data, char value)
{
	return data->mxt_drv_data->patchevent_mask & value;
}

static void mxt_patchevent_set(struct mxt_data *data, char value)
{
	data->mxt_drv_data->patchevent_mask |= value;
}

static void mxt_patchevent_unset(struct mxt_data *data, char value)
{
	data->mxt_drv_data->patchevent_mask &= ~(value);
}

static inline u16 mxt_obj_size(const struct mxt_object *obj)
{
	return obj->size_minus_one + 1;
}

static inline unsigned int mxt_obj_instances(const struct mxt_object *obj)
{
	return obj->instances_minus_one + 1;
}

static bool mxt_object_readable(unsigned int type)
{
	switch (type) {
		case MXT_GEN_COMMAND_T6:
		case MXT_GEN_POWER_T7:
		case MXT_GEN_ACQUIRE_T8:
		case MXT_TOUCH_KEYARRAY_T15:
		case MXT_SPT_COMMSCONFIG_T18:
		case MXT_TOUCH_PROXIMITY_T23:
		case MXT_PROCI_ONETOUCH_T24:
		case MXT_SPT_SELFTEST_T25:
		case MXT_SPT_USERDATA_T38:
		case MXT_PROCI_GRIPSUPPRESSION_T40:
		case MXT_PROCI_TOUCHSUPPRESSION_T42:
		case MXT_SPT_CTECONFIG_T46:
		case MXT_PROCI_ADAPTIVETHRESHOLD_T55:
		case MXT_PROCI_SHIELDLESS_T56:
		case MXT_SPT_DYNAMICCONFIGURATIONCONTAINER_T71:
		return true;
	default:
		return false;
	}
}

static void mxt_dump_message(struct mxt_data *data, u8 *message)
{
	print_hex_dump(KERN_ERR, "[Solomon] MXT MSG:", DUMP_PREFIX_NONE, 16, 1,
		       message, data->T5_msg_size, false);
}

static int mxt_wait_for_completion(struct mxt_data *data,
			struct completion *comp, unsigned int timeout_ms)
{
	unsigned long timeout = msecs_to_jiffies(timeout_ms);
	long ret = 0;

	ret = wait_for_completion_interruptible_timeout(comp, timeout);
	if (ret < 0) {
		LOGE("Wait for completion interrupted.\n");
		return -EINTR;
	} else if (ret == 0) {
		LOGE("Wait for completion timed out.\n");
		return -ETIMEDOUT;
	}
	return 0;
}

static int mxt_bootloader_read(struct mxt_data *data, u8 *val, unsigned int count)
{
	int ret = 0;
	struct i2c_msg msg = {0};

	msg.addr = data->bootloader_addr;
	msg.flags = data->client->flags & I2C_M_TEN;
	msg.flags |= I2C_M_RD;
	msg.len = count;
	msg.buf = val;

	ret = i2c_transfer(data->client->adapter, &msg, 1);
	if (ret == 1) {
		ret = 0;
	} else {
		ret = (ret < 0) ? ret : -EIO;
		LOGE("i2c recv failed (%d)\n", ret);
	}

	return ret;
}

static int mxt_bootloader_write(struct mxt_data *data, const u8 * const val, unsigned int count)
{
	int ret = 0;
	struct i2c_msg msg = {0};
	u8 buf_t[1000] = {0};
	int i = 0;

	if (count > 1000) {
		ret = -EIO;
		LOGE("Send over size message\n");
		goto Error;
	}

	for (; i < count; i++) {
		buf_t[i] = val[i];
	}

	msg.addr = data->bootloader_addr;
	msg.flags = data->client->flags & I2C_M_TEN;
	msg.len = count;
	msg.buf = buf_t;

	ret = i2c_transfer(data->client->adapter, &msg, 1);
	if (ret == 1) {
		ret = 0;
	} else {
		ret = (ret < 0) ? ret : -EIO;
		LOGE("i2c send failed (%d)\n", ret);
	}
Error:
	return ret;
}

static int mxt_lookup_bootloader_address(struct mxt_data *data, u8 retry)
{
	u8 appmode = data->client->addr;
	u8 bootloader = 0;
	u8 family_id = 0;

	if (data->info)
		family_id = data->info->family_id;

	LOGN("%s appmode=0x%x\n", __func__, appmode);

	switch (appmode) {
	case 0x4a:
	case 0x4b:
		/* Chips after 1664S use different scheme */
		if ((retry % 2) || family_id >= 0xa2) {
			bootloader = appmode - 0x24;
			break;
		}
		/* Fall through for normal case */
	case 0x4c:
	case 0x4d:
	case 0x5a:
	case 0x5b:
		bootloader = appmode - 0x26;
		break;
	default:
		LOGE(
			"Appmode i2c address 0x%02x not found\n",
			appmode);
		return -EINVAL;
	}

	data->bootloader_addr = bootloader;
	LOGN("%s bootloader_addr=0x%x\n", __func__, bootloader);
	return 0;
}

static int mxt_probe_bootloader(struct mxt_data *data, u8 retry)
{
	int ret = 0;
	u8 val = 0;
	bool crc_failure = false;

	LOGD("%s\n", __func__);

	ret = mxt_lookup_bootloader_address(data, retry);
	if (ret)
		return ret;

	ret = mxt_bootloader_read(data, &val, 1);
	if (ret)
		return ret;

	/* Check app crc fail mode */
	crc_failure = (val & ~MXT_BOOT_STATUS_MASK) == MXT_APP_CRC_FAIL;

	LOGD("Detected bootloader, status:%02X%s\n", val, crc_failure ? ", APP_CRC_FAIL" : "");

	return 0;
}

static u8 mxt_get_bootloader_version(struct mxt_data *data, u8 val)
{
	u8 buf[3] = {0};

	if (val & MXT_BOOT_EXTENDED_ID) {
		if (mxt_bootloader_read(data, &buf[0], 3) != 0) {
			LOGE("%s: i2c failure\n", __func__);
			return -EIO;
		}

		LOGD("Bootloader ID:%d Version:%d\n", buf[1], buf[2]);

		return buf[0];
	} else {
		LOGD("Bootloader ID:%d\n", val & MXT_BOOT_ID_MASK);

		return val;
	}
}

static int mxt_check_bootloader(struct mxt_data *data, unsigned int state)
{
	u8 val = 0;
	int ret = 0;

recheck:
	ret = mxt_bootloader_read(data, &val, 1);
	if (ret)
		return ret;

	if (state == MXT_WAITING_BOOTLOAD_CMD)
		val = mxt_get_bootloader_version(data, val);

	switch (state) {
	case MXT_WAITING_BOOTLOAD_CMD:
	case MXT_WAITING_FRAME_DATA:
	case MXT_APP_CRC_FAIL:
		val &= ~MXT_BOOT_STATUS_MASK;
		break;
	case MXT_FRAME_CRC_PASS:
		if (val == MXT_FRAME_CRC_CHECK) {
			goto recheck;
		} else if (val == MXT_FRAME_CRC_FAIL) {
			LOGE("Bootloader CRC fail\n");
			return -EINVAL;
		}
		break;
	default:
		return -EINVAL;
	}

	if (val != state) {
		LOGE("Invalid bootloader state %02X != %02X\n",
			val, state);
		return -EINVAL;
	}

	return 0;
}

static int mxt_send_bootloader_cmd(struct mxt_data *data, bool unlock)
{
	int ret = 0;
	u8 buf[2] = {0};

	LOGD("%s : %d\n", __func__, unlock);

	if (unlock) {
		buf[0] = MXT_UNLOCK_CMD_LSB;
		buf[1] = MXT_UNLOCK_CMD_MSB;
	} else {
		buf[0] = 0x01;
		buf[1] = 0x01;
	}

	ret = mxt_bootloader_write(data, buf, 2);
	if (ret)
		return ret;

	return 0;
}

static int __mxt_read_reg(struct i2c_client *client, u16 reg, u16 len, void *val)
{
	struct i2c_msg xfer[2];
	u8 buf[2] = {0};
	int i = 0;

	buf[0] = reg & 0xff;
	buf[1] = (reg >> 8) & 0xff;

	/* Write register */
	xfer[0].addr = client->addr;
	xfer[0].flags = 0;
	xfer[0].len = 2;
	xfer[0].buf = buf;

	/* Read data */
	xfer[1].addr = client->addr;
	xfer[1].flags = I2C_M_RD;
	xfer[1].len = len;
	xfer[1].buf = val;
	do {
		if (i2c_transfer(client->adapter, xfer, ARRAY_SIZE(xfer)) == 2)
			return 0;
		LOGE("%s: i2c retry %d for reg:0x%04X\n", __func__, i+1, reg);
		msleep(MXT_WAKEUP_TIME);
	} while (++i < 10);

	LOGE("%s: i2c transfer failed reg:0x%04X\n", __func__, reg);
	return -EIO;
}

static int __mxt_write_reg(struct i2c_client *client, u16 reg, u16 len, const void *val)
{
	u8 data[288] = {0};
	u8 *buf = NULL;
	int count = 0;
	int i = 0;
	bool alloced = false;
	int ret = 0;

	count = len + 2;

	if (unlikely(count > 288)) {
		buf = kzalloc(count, GFP_KERNEL);
		if (!buf)
			return -ENOMEM;
		alloced = true;
	} else {
		buf = data;
	}

	buf[0] = reg & 0xff;
	buf[1] = (reg >> 8) & 0xff;
	memcpy(&buf[2], val, len);
	do {
		if (i2c_master_send(client, buf, count) == count) {
			ret = 0;
			goto out;
		}
		LOGE("%s: i2c retry %d for reg:0x%04X\n", __func__, i+1, reg);
		msleep(MXT_WAKEUP_TIME);
	} while (++i < 10);
	LOGE("%s: i2c transfer failed reg:0x%04X\n", __func__, reg);
	ret = -EIO;

out :
	if (unlikely(alloced)) {
		kfree(buf);
	}

	return ret;
}

static int mxt_write_reg(struct i2c_client *client, u16 reg, u8 val)
{
	return __mxt_write_reg(client, reg, 1, &val);
}

struct mxt_object *mxt_get_object(struct mxt_data *data, u8 type)
{
	struct mxt_object *object = NULL;
	int i = 0;

	for (i = 0; i < data->info->object_num; i++) {
		object = data->object_table + i;
		if (object->type == type)
			return object;
	}

	LOGE("Invalid object type T%u\n", type);
	return NULL;
}

int mxt_read_object(struct mxt_data *data, u8 type, u8 offset, u8 *val)
{
	struct mxt_object *object = NULL;
	int error = 0;

	object = mxt_get_object(data, type);
	if (!object)
		return -EINVAL;

	error = __mxt_read_reg(data->client, object->start_address + offset, 1, val);
	if (error)
		LOGE("Error to read T[%d] offset[%d] val[%d]\n",
				type, offset, *val);

	return error;
}

int mxt_write_object(struct mxt_data *data, u8 type, u8 offset, u8 val)
{
	struct mxt_object *object = NULL;
	int error = 0;
	u16 reg = 0;
	object = mxt_get_object(data, type);

	if (!object)
		return -EINVAL;

	reg = object->start_address;
	error = __mxt_write_reg(data->client, reg + offset, 1, &val);
	if (error) {
		LOGE("Error to write T[%d] offset[%d] val[%d]\n", type, offset, val);
		mxt_hw_reset(data);
	}
	return error;
}

static int mxt_set_diagnostic_mode(struct mxt_data *data, u8 dbg_mode)
{
	u8 cur_mode = 0;
	int ret = 0;
	int retry_cnt = 0;

	ret = mxt_write_object(data, MXT_GEN_COMMAND_T6/*MXT_GEN_COMMANDPROCESSOR_T6*/,
			MXT_COMMAND_DIAGNOSTIC, dbg_mode);

	if (ret) {
		LOGE("Failed change diagnositc mode to %d\n",
			 dbg_mode);
		goto out;
	}

	while (true) {
		u8 diagnostic = 0;

		ret = mxt_read_object(data, MXT_GEN_COMMAND_T6, MXT_COMMAND_DIAGNOSTIC, &diagnostic);
		if (ret) {
			LOGE("Failed getting diagnostic command rc:%d\n", ret);
			goto out;
		}

		if (!diagnostic)
			break;

		if (retry_cnt++ >= 4) {
			LOGE("Diagnostic (mode:%u) timed out\n", dbg_mode);
			ret = -ETIMEDOUT;
			goto out;
		}
		msleep(20);
	};

	if (dbg_mode & MXT_DIAG_MODE_MASK) {
		do {
			ret = mxt_read_object(data, MXT_DEBUG_DIAGNOSTIC_T37,
				MXT_DIAGNOSTIC_MODE, &cur_mode);
			if (ret || retry_cnt++ >= 4) {
				LOGE("Failed getting diagnositc mode(%d)\n", retry_cnt);
				goto out;
			}
			msleep(20);
		} while (cur_mode != dbg_mode);
		LOGE("current dianostic chip mode is %d\n", cur_mode);
	}

out:
	return ret;
}

int mxt_get_self_reference_chk(struct mxt_data *data)
{
	u8 cur_page = 0;
	u8 read_page = 0;
	struct mxt_object *dbg_object = NULL;
	struct mxt_object *object = NULL;

	int ret = 0;
	int i = 0;
	u8 ret_buf[NODE_PER_PAGE * DATA_PER_NODE] = {0};
	u8 comms_chk[2] = {0};
	u8 loop_chk = 0;
	u8 self_chk_thr = 0;

	s16 curr_ref;

	u8 err_cnt = 0;

	struct mxt_data *mxt_data = data->mxt_drv_data->mxt_data;

	ret = mxt_t6_command(data, MXT_COMMAND_DIAGNOSTIC, MXT_COMMAND_SELF_REFERENCE, false);
	if (ret) {
		return 0;
	}

	object = mxt_get_object(mxt_data, MXT_SPT_USERDATA_T38);
	if (!object) {
		LOGE("Failed to get object\n");
		return 1;
	}

	ret = __mxt_read_reg(data->client,
				object->start_address + 6, 1, &self_chk_thr);

	dbg_object = mxt_get_object(data, MXT_DEBUG_DIAGNOSTIC_T37);
	if (!dbg_object) {
		LOGE("Failed to get object_info\n");
		return 1;
	}

	for (i = 0; i < 7; i++) {
		msleep(20);
		__mxt_read_reg(data->client, dbg_object->start_address, 2, comms_chk);

		if (comms_chk[0] == MXT_COMMAND_SELF_REFERENCE && comms_chk[1] == 0) {
			LOGD("%s Enter success in Self Reference mode\n", __func__);
			break;
		} else if (i == 6) {
			LOGD("%s Enter fail in Self Reference mode\n", __func__);
			return 0; // Don't check self Reference no more!!
		}
	}

	for (read_page = 0 ; read_page < 7; read_page++) {
		__mxt_read_reg(data->client, dbg_object->start_address + 2, NODE_PER_PAGE * DATA_PER_NODE, ret_buf);

		LOGD("CURR SELF REFERENCE read page %u :", read_page);
		if (read_page == 0 || read_page == 1 || read_page == 6) {
			for (i = 0; i < 6; i++) {
			LOGD("%6hd %6hd %6hd %6hd %6hd %6hd %6hd %6hd %6hd %6hd",
				( ((u16)ret_buf[(i*10)*2+1] << 8) + (u16)ret_buf[(i*10)*2] ),
				( ((u16)ret_buf[(i*10+1)*2+1] << 8) + (u16)ret_buf[(i*10+1)*2] ),
				( ((u16)ret_buf[(i*10+2)*2+1] << 8) + (u16)ret_buf[(i*10+2)*2] ),
				( ((u16)ret_buf[(i*10+3)*2+1] << 8) + (u16)ret_buf[(i*10+3)*2] ),
				( ((u16)ret_buf[(i*10+4)*2+1] << 8) + (u16)ret_buf[(i*10+4)*2] ),
				( ((u16)ret_buf[(i*10+5)*2+1] << 8) + (u16)ret_buf[(i*10+5)*2] ),
				( ((u16)ret_buf[(i*10+6)*2+1] << 8) + (u16)ret_buf[(i*10+6)*2] ),
				( ((u16)ret_buf[(i*10+7)*2+1] << 8) + (u16)ret_buf[(i*10+7)*2] ),
				( ((u16)ret_buf[(i*10+8)*2+1] << 8) + (u16)ret_buf[(i*10+8)*2] ),
				( ((u16)ret_buf[(i*10+9)*2+1] << 8) + (u16)ret_buf[(i*10+9)*2] )
				);
			}
		}

		for (i = 0; i < NODE_PER_PAGE; i++) {
			curr_ref = ( ((u16)ret_buf[i * 2 + 1] << 8) + (u16)ret_buf[i * 2] );

			if (read_page == 0 && i > 40) {
				// Self Hover
				break;
			} else if (read_page == 1 && i < 8) {
				continue;
			} else if (read_page == 1 && i > 33) {
			    break;
			} else if (read_page > 1 && read_page < 6) {
			    break;
			} else if (read_page == 6 && i > 35) {
			    break;
			} else {
				// Self Touch & Proc reference check
				if ( curr_ref > self_chk_thr * 500) {
					break;
				} else {
					// Need to Self-Tune.
					LOGD("CURR SELF REFERENCE Error page %u, numger %d :", read_page, i);
					err_cnt++;
				}
			}
		}

		ret = mxt_set_diagnostic_mode(data, MXT_DIAG_PAGE_UP);
		if (ret) {
			LOGE("Failed to set self reference mode!\n");
			return 0; // Don't check self reference no more!!
		}

		loop_chk = 0;

		do {
			msleep(20);
			ret = __mxt_read_reg(data->client,
			dbg_object->start_address + MXT_DIAGNOSTIC_PAGE, 1, &cur_page);
			if (ret  || loop_chk++ >= 4) {
				LOGE("%s Read fail page(%d)\n", __func__, loop_chk);
				return 0; // Don't check self reference no more!!
			}
		} while (cur_page != read_page + 1);
	}

	LOGE("Reference Error Count: %d\n", err_cnt);

	if ( err_cnt > 0) {
		LOGE("Need to Self Cap Re tune!!!!!!!!!!!!\n");

		if (object) {
			 mxt_write_reg(mxt_data->client, object->start_address + 2, err_cnt);
		}

		return 1;
	} else {
		mxt_write_object(data, MXT_SPT_USERDATA_T38, 1, 1);

		/* Backup to memory */
		ret = mxt_command_backup(data, MXT_BACKUP_VALUE);
		if (ret) {
			LOGE("Failed backup NV data\n");
		}
	}

	LOGD("Self Reference Check Success!!!!!!!!!!!!\n");
	return 0;
}

static bool mxt_check_xy_range(struct mxt_data *data, u16 node)
{
	u8 x_line = node / data->info->matrix_ysize;
	u8 y_line = node % data->info->matrix_ysize;

	return (y_line < data->rawdata->num_ynode) ?
		(x_line < data->rawdata->num_xnode) : false;
}

static int mxt_treat_dbg_data(struct mxt_data *data,
	struct mxt_object *dbg_object, u8 dbg_mode, u8 read_point, u16 num)
{
	struct mxt_raw_data *rawdata = data->rawdata;
	u8 data_buffer[DATA_PER_NODE] = { 0 };
	int ret = 0;

	if (dbg_mode == MXT_DIAG_DELTA_MODE) {
		/* read delta data */
		__mxt_read_reg(data->client, dbg_object->start_address + read_point,
			DATA_PER_NODE, data_buffer);

		rawdata->delta[num] =
			((u16)data_buffer[1]<<8) + (u16)data_buffer[0];
		ret = rawdata->delta[num];
	} else if (dbg_mode == MXT_DIAG_REFERENCE_MODE) {
		/* read reference data */
		__mxt_read_reg(data->client, dbg_object->start_address + read_point,
			DATA_PER_NODE, data_buffer);

		rawdata->reference[num] =
			((u16)data_buffer[1] << 8) + (u16)data_buffer[0]
			- REF_OFFSET_VALUE;

		ret = rawdata->reference[num];
	}
	return ret;
}
static void mxt_prepare_debug_data(struct mxt_data *data)
{
	struct mxt_raw_data *rawdata = NULL;
	int error = 0;

	if (data->rawdata) {
		if (data->rawdata->reference) {
			kfree(data->rawdata->reference);
			data->rawdata->reference = NULL;
		}
		if (data->rawdata->delta) {
			kfree(data->rawdata->delta);
			data->rawdata->delta = NULL;
		}
		kfree(data->rawdata);
		data->rawdata = NULL;
	}

	rawdata = kzalloc(sizeof(struct mxt_raw_data), GFP_KERNEL);
	if (rawdata == NULL) {
		LOGE("Fail to allocate sysfs data.\n");
		error = -ENOMEM;
		return ;
	}
	rawdata->num_xnode = data->info->matrix_xsize;
	rawdata->num_ynode = data->info->matrix_ysize;
	rawdata->num_nodes = rawdata->num_xnode * rawdata->num_ynode;

	LOGD("%s: x=%d, y=%d, total=%d\n",
		__func__, rawdata->num_xnode, rawdata->num_ynode, rawdata->num_nodes);

	rawdata->reference = kzalloc(rawdata->num_nodes * sizeof(u16), GFP_KERNEL);
	if (!rawdata->reference) {
		LOGD("Fail to alloc reference of rawdata\n");
		error = -ENOMEM;
		goto err_alloc_reference;
	}

	rawdata->delta = kzalloc(rawdata->num_nodes * sizeof(s16), GFP_KERNEL);
	if (!rawdata->delta) {
		LOGD("Fail to alloc delta of fdata\n");
		error = -ENOMEM;
		goto err_alloc_delta;
	}

	data->rawdata = rawdata;
	return ;

err_alloc_delta:
err_alloc_reference:
	LOGD("kfree in %s\n", __func__);
	if (rawdata->delta)
		kfree(rawdata->delta);
	if (rawdata->reference)
		kfree(rawdata->reference);
	if (rawdata)
		kfree(rawdata);
	return ;

}

static int mxt_read_all_diagnostic_data(struct mxt_data *data, u8 dbg_mode, char *buf, int* len)
{
	struct mxt_object *dbg_object = NULL;
	u8 read_page = 0, cur_page = 0, end_page = 0, read_point = 0;
	u16 node = 0, num = 0,  cnt = 0;
	int ret = 0;
	int value = 0;
	int write_page = 16 * 1024;

	u8 t46_ctrl = 0x00;
	u8 t56_ctrl = 0x01;

	if (dbg_mode == MXT_DIAG_REFERENCE_MODE) {
		ret = __mxt_read_reg(data->client, data->T46_address + 0,
				     sizeof(t46_ctrl), &t46_ctrl);
		if (ret)
			LOGE("Failed to read from T46:0 rc:%d\n", ret);

		ret = __mxt_read_reg(data->client, data->T56_address + 0,
				     sizeof(t56_ctrl), &t56_ctrl);
		if (ret)
			LOGE("Failed to read from T56:0 rc:%d\n", ret);

		ret = mxt_write_reg(data->client, data->T46_address + 0, 8);
		if (ret)
			LOGE("Failed to write into T46:0 rc:%d\n", ret);

		ret = mxt_write_reg(data->client, data->T56_address + 0, 0);
		if (ret)
			LOGE("Failed to write into T56:0 rc:%d\n", ret);
	}

	touch_disable_irq(data);
	pm_wakeup_event(&data->client->dev, msecs_to_jiffies(2000));
	mutex_lock(&data->mxt_drv_data->dev_lock);

	/* to make the Page Num to 0 */
	ret = mxt_set_diagnostic_mode(data, MXT_DIAG_CTE_MODE);
	if (ret)
		goto out;

	/* change the debug mode */
	ret = mxt_set_diagnostic_mode(data, dbg_mode);
	if (ret)
		goto out;

	/* get object info for diagnostic */
	dbg_object = mxt_get_object(data, MXT_DEBUG_DIAGNOSTIC_T37);
	if (!dbg_object) {
		LOGE("fail to get object_info\n");
		ret = -EINVAL;
		goto out;
	}

	//LGE
	mxt_prepare_debug_data(data);

	switch (dbg_mode) {
		case MXT_DIAG_REFERENCE_MODE:
			*len += snprintf(buf + *len, write_page - *len,
					"======== rawdata ========");
			break;
		case MXT_DIAG_DELTA_MODE:
			*len += snprintf(buf + *len, write_page - *len,
					"======== deltadata ========");
			break;
	}


	end_page = (data->info->matrix_xsize * data->info->matrix_ysize) / NODE_PER_PAGE;
	/* read the dbg data */
	for (read_page = 0 ; read_page < end_page; read_page++) {
		for (node = 0; node < NODE_PER_PAGE; node++) {
			if (cnt / data->info->matrix_ysize >= data->channel_size.size_x) {
				break;
			}
			if (cnt%data->info->matrix_ysize == 0) {
				*len += snprintf(buf + *len , write_page - *len, "\n[%2d] ", cnt/data->info->matrix_ysize);
				if (cnt/data->info->matrix_ysize == data->info->matrix_xsize) {
					*len += snprintf(buf + *len , write_page - *len, "\n");
					break;
				}
			}
			read_point = (node * DATA_PER_NODE) + 2;

			if (!mxt_check_xy_range(data, cnt++))
				break;

			value = mxt_treat_dbg_data(data, dbg_object, MXT_DIAG_DELTA_MODE, read_point, num);

			if (cnt % data->info->matrix_ysize <= data->channel_size.size_y && cnt % data->info->matrix_ysize != 0) {
				*len += snprintf(buf + *len, write_page - *len, "%5d ", value);
			}
			num++;
		}
		ret = mxt_set_diagnostic_mode(data, MXT_DIAG_PAGE_UP);
		if (ret)
			goto out;
		do {
			msleep(20);
			ret = __mxt_read_reg(data->client,
				dbg_object->start_address + MXT_DIAGNOSTIC_PAGE, 1, &cur_page);
			if (ret) {
				LOGE("%s Read fail page\n", __func__);
				goto out;
			}
		} while (cur_page != read_page + 1);
	}

	*len += snprintf(buf + *len , write_page - *len, "\n");

	cnt = 0;
	while (buf[cnt]) {
		char *lf_pos = strchr(&buf[cnt], '\n');

		if (!lf_pos)
			break;

		*lf_pos = 0;
		LOGN("%s\n", &buf[cnt]);
		*lf_pos = '\n';
		cnt = lf_pos - buf + 1;
	}

out:
	if (dbg_mode == MXT_DIAG_REFERENCE_MODE) {
		ret = mxt_write_reg(data->client, data->T46_address + 0, t46_ctrl);
		if (ret)
			LOGE("Failed to write into T46:0 rc:%d\n", ret);

		ret = mxt_write_reg(data->client, data->T56_address + 0, t56_ctrl);
		if (ret)
			LOGE("Failed to write into T56:0 rc:%d\n", ret);
	}

	mutex_unlock(&data->mxt_drv_data->dev_lock);

	touch_enable_irq(data);

	return ret;
}
static int run_reference_read(void *device_data, char *buf, int *len)
{
	struct mxt_data *data = (struct mxt_data *)device_data;
	int ret = 0;

	ret = mxt_read_all_diagnostic_data(data, MXT_DIAG_REFERENCE_MODE, buf, len);

	return ret;
}

static int run_delta_read(void *device_data, char *buf, int *len)
{
	struct mxt_data *data = (struct mxt_data *)device_data;
	int ret = 0;

	ret = mxt_read_all_diagnostic_data(data, MXT_DIAG_DELTA_MODE, buf, len);
	return ret;
}

static void mxt_proc_t6_messages(struct mxt_data *data, u8 *msg)
{
	u8 status = msg[1];
	u32 crc = msg[2] | (msg[3] << 8) | (msg[4] << 16);
	struct mxt_object *object = NULL;
	u8 stylus_in_a_row_limit[6] = {0};
	int ret = 0;
	struct mxt_data *mxt_data = data->mxt_drv_data->mxt_data;

	if (crc != data->config_crc) {
		data->config_crc = crc;
		LOGN("T6 Config Checksum: 0x%06X\n", crc);
		complete(&data->crc_completion);
	}

	/* Detect transition out of reset */
	if ((data->t6_status & MXT_T6_STATUS_RESET) && !(status & MXT_T6_STATUS_RESET))
		complete(&data->reset_completion);

	/* Output debug if status has changed */
	if (status != data->t6_status) {
		LOGN("T6 Status 0x%02X%s%s%s%s%s%s%s\n",
			status,
			(status == 0) ? " OK" : "",
			(status & MXT_T6_STATUS_RESET) ? " RESET" : "",
			(status & MXT_T6_STATUS_OFL) ? " OFL" : "",
			(status & MXT_T6_STATUS_SIGERR) ? " SIGERR" : "",
			(status & MXT_T6_STATUS_CAL) ? " CAL" : "",
			(status & MXT_T6_STATUS_CFGERR) ? " CFGERR" : "",
			(status & MXT_T6_STATUS_COMSERR) ? " COMSERR" : "");

		if (status & MXT_T6_STATUS_SIGERR) {
			LOGE("signal error occur, do hw reset\n");
			mxt_hw_reset(data);
		}
	}

	/* Save current status */
	data->t6_status = status;
	if (!data->t6_status_reset_irq) {
		data->t6_status_reset_irq = status;
	}

	if (status & MXT_T6_STATUS_CAL || status & MXT_T6_STATUS_RESET) {
		//mxt_regulator_enable(data);
		mxt_reset_slots(data);
		if (data->delayed_cal) {
			LOGD("delayed calibration call\n");
			queue_delayed_work(data->mxt_drv_data->touch_wq, &data->work_delay_cal, msecs_to_jiffies(200));
			data->delayed_cal = false;
		}
		data->patch.src_item[MXT_PATCH_ITEM_USER4] = 0;
		object = mxt_get_object(data, MXT_SPT_USERDATA_T38);

		if (!object) {
			LOGN("fail to get object\n");
			return;
		}

		ret = mxt_read_mem(data, object->start_address+2, 6, stylus_in_a_row_limit);

		if (ret) {
			LOGN("T38 stylus_in_a_row_limit read fail\n");
		}

		data->stylus_in_a_row_cnt_thr = stylus_in_a_row_limit[0] << 8 | stylus_in_a_row_limit[1];
		data->x_zitter = stylus_in_a_row_limit[2] << 8 | stylus_in_a_row_limit[3];
		data->y_zitter = stylus_in_a_row_limit[4] << 8 | stylus_in_a_row_limit[5];
		LOGN("Set Stylus limit thr %d x_zitter %d y_zitter %d\n", data->stylus_in_a_row_cnt_thr, data->x_zitter, data->y_zitter);
	}

	/* Set KnockCode Delay after RESET */
	if (!data->mfts_enable) {
		if (status & MXT_T6_STATUS_RESET && data->is_knockCodeDelay) {
			mxt_write_object(data, MXT_PROCI_TOUCH_SEQUENCE_LOGGER_T93, 19, 43);
			LOGD("Set Knock Code delay after RESET (700ms)\n");
		} else if (status & MXT_T6_STATUS_RESET && !data->is_knockCodeDelay) {
			mxt_write_object(data, MXT_PROCI_TOUCH_SEQUENCE_LOGGER_T93, 19, 0);
			LOGD("Set Knock Code delay after RESET (0ms)\n");
		}
	}

	if (status & MXT_T6_STATUS_RESET && data->suspended) {
		LOGD("RESET Detected. Start Recover\n");

		if (mxt_patchevent_get(data, PATCH_EVENT_TA)) {
			LOGD("   Stage 1 : USB/TA \n");
			if (data->mxt_drv_data->factorymode)
				mxt_data->knock_on_mode = CHARGER_PLUGGED_AAT;
			else
				mxt_data->knock_on_mode = CHARGER_PLUGGED;

			mxt_patch_event(mxt_data, mxt_data->knock_on_mode);
		}

		if (mxt_patchevent_get(data, PATCH_EVENT_KNOCKON)) {
			if (data->lpwg_mode == LPWG_DOUBLE_TAP) {
				LOGD("   Stage 2 : Double Tap \n");
				if (mxt_patchevent_get(data, PATCH_EVENT_TA)) {
					data->knock_on_mode = CHARGER_KNOCKON_SLEEP;
				} else {
					data->knock_on_mode = NOCHARGER_KNOCKON_SLEEP;
				}
			} else if(data->lpwg_mode == LPWG_MULTI_TAP) {
				LOGD("   Stage 2 : Multi Tap \n");
				if (mxt_patchevent_get(data, PATCH_EVENT_TA)) {
					data->knock_on_mode = CHARGER_KNOCKON_SLEEP + PATCH_EVENT_PAIR_NUM;
				} else {
					data->knock_on_mode = NOCHARGER_KNOCKON_SLEEP + PATCH_EVENT_PAIR_NUM;
				}
			}
		}
		LOGD("Recover Complete\n");
	}

	if (status & MXT_T6_STATUS_RESET) {
		data->mxt_drv_data->is_resuming = true;
		if (!data->wireless_charge)
			mxt_set_side_key(data);
		mxt_set_stamina_mode(data);
		mxt_set_cover_mode(data);
		mxt_set_glove_mode(data);
		data->mxt_drv_data->is_resuming = false;
	}
}

static void mxt_input_sync(struct input_dev *input_dev)
{
	input_mt_report_pointer_emulation(input_dev, false);
	input_sync(input_dev);
}

static char *get_touch_button_string(u16 key_code)
{
	static char str[16] = {0};

	switch (key_code) {
		case KEY_BACK : /*158 0x9E*/
			sprintf(str, "BACK");
			break;
		case KEY_HOMEPAGE : /*172 0xAC*/
			sprintf(str, "HOME");
			break;
		case KEY_MENU : /* 139 0x8B*/
			sprintf(str, "MENU");
			break;
		default :
			sprintf(str, "Unknown");
			break;
	}

	return str;
}

static void mxt_button_lock_func(struct work_struct *work_button_lock)
{
	struct mxt_data *data = container_of(to_delayed_work(work_button_lock), struct mxt_data, work_button_lock);

	mutex_lock(&data->mxt_drv_data->dev_lock);
	data->button_lock = false;
	mutex_unlock(&data->mxt_drv_data->dev_lock);
}

static void mxt_palm_unlock_func(struct work_struct *work_palm_unlock)
{
	struct mxt_data *data = container_of(to_delayed_work(work_palm_unlock), struct mxt_data, work_palm_unlock);

	mutex_lock(&data->mxt_drv_data->dev_lock);
	data->palm = false;
	mutex_unlock(&data->mxt_drv_data->dev_lock);
}

static void mxt_delay_cal_func(struct work_struct *work_delay_cal)
{
	struct mxt_data *data = container_of(to_delayed_work(work_delay_cal), struct mxt_data, work_delay_cal);

	LOGN("Delayed work Calibration\n");
	/* Recalibrate since chip has been in deep sleep */
	mxt_t6_command(data, MXT_COMMAND_CALIBRATE, 1, false);
	data->enable_reporting = true;
}

static void mxt_watchdog_func(struct work_struct *work)
{
	struct mxt_watchdog *watchdog =
		container_of(to_delayed_work(work),
			struct mxt_watchdog, work);
	struct mxt_data *data = container_of(watchdog, struct mxt_data, watchdog);
	u8 family_id = 0;
	int ret = 0;

	mutex_lock(&data->mxt_drv_data->dev_lock);

	if (data->suspended || data->in_bootloader) {
		schedule_delayed_work(&data->watchdog.work,
			msecs_to_jiffies(data->watchdog.delay));
		mutex_unlock(&data->mxt_drv_data->dev_lock);
		return;
	}

	ret = mxt_pw_lock(INCELL_DISPLAY_POWER_LOCK);
	if (ret) {
		LOGE("Failed to lock power\n");
		mutex_unlock(&data->mxt_drv_data->dev_lock);
		return;
	}

	if (!data->suspended && !data->in_bootloader) {
		ret = mxt_read_mem(data, FAMILY_ID_START_LOCATION, FAMILY_ID_SIZE, &family_id);
		if (ret) {
			LOGE("Read fail. IC information\n");
			goto unlock;
		}

		if (family_id != BASIC_FAMILY_ID) {
			LOGE("Unexpected family_id %02X\n", family_id);
			mxt_hw_reset(data);
		}

		schedule_delayed_work(&data->watchdog.work,
			msecs_to_jiffies(data->watchdog.delay));
	}

unlock:
	ret = mxt_pw_lock(INCELL_DISPLAY_POWER_UNLOCK);
	if (ret)
		LOGE("Failed to unlock power\n");
	mutex_unlock(&data->mxt_drv_data->dev_lock);
}

int mxt_get_reference_chk(struct mxt_data *data)
{
	u16 buf[336] = {0};
	u16 num = 0;
	u16 end_page = 0;
	u8 cur_page = 0;
	u8 read_page = 0;
	struct mxt_object *dbg_object = NULL;
	int diff_v = 0;
	int diff_b_v = 0;
	u16 pre_v = 0;
	int err_cnt = 0;
	int ret = 0;
	int i = 0;
	u8 err_tot_x[24] = {0}, err_tot_y[14] = {0};
	u16 curx = 0, cury = 0, ref_min = 16384;
	u16 ref_max = 0, diff_max = 0;
	u8 ret_buf[NODE_PER_PAGE * DATA_PER_NODE] = {0};
	u8 line_err_chk = 0;
	u8 err_line_num = 0;
	u8 half_err_cnt = 0;
	u8 loop_chk = 0;
	static u8 err_chk = 0;
	static int last_call_statue = 0;

	data->patch.src_item[MXT_PATCH_ITEM_USER6] = 0;
	data->ref_chk = 0;

	/* to make the Page Num to 0 */
	ret = mxt_set_diagnostic_mode(data, MXT_DIAG_CTE_MODE);

	if (ret)
		return 1;

	/* change the debug mode */
	ret = mxt_set_diagnostic_mode(data, MXT_DIAG_REFERENCE_MODE);
	if (ret)
		return 1;

	dbg_object = mxt_get_object(data, MXT_DEBUG_DIAGNOSTIC_T37);
	if (!dbg_object) {
		LOGE("Failed to get object_info\n");
		return 1;
	}

	if (data->incoming_call == INCOMING_CALL_RINGING) {
		half_err_cnt = 1;
		data->mxt_drv_data->resume_flag = 0;
	} else if (data->incoming_call == INCOMING_CALL_OFFHOOK) {
		half_err_cnt = 3;
		data->mxt_drv_data->resume_flag = 0;
	} else if (last_call_statue == INCOMING_CALL_OFFHOOK && data->incoming_call == INCOMING_CALL_IDLE) {
		do_gettimeofday(&data->mxt_drv_data->t_ex_debug[TIME_EX_INIT_TIME]);
		data->mxt_drv_data->resume_flag = 1;
	}

	last_call_statue = data->incoming_call;

	if (data->mxt_drv_data->resume_flag) {
		do_gettimeofday(&data->mxt_drv_data->t_ex_debug[TIME_EX_FIRST_INT_TIME]);

		if ( data->mxt_drv_data->t_ex_debug[TIME_EX_FIRST_INT_TIME].tv_sec - data->mxt_drv_data->t_ex_debug[TIME_EX_INIT_TIME].tv_sec <= 3 ) {
			half_err_cnt = 3;
		} else if ( data->mxt_drv_data->t_ex_debug[TIME_EX_FIRST_INT_TIME].tv_sec - data->mxt_drv_data->t_ex_debug[TIME_EX_INIT_TIME].tv_sec <= 7 ) {
			half_err_cnt = data->ref_limit.ref_err_cnt/2;
		} else {
			half_err_cnt = data->ref_limit.ref_err_cnt;
		}

	}

	if (!half_err_cnt)
		half_err_cnt = 10;

	LOGD("half_err_cnt(%d) ref_err_cnt(%d)\n", half_err_cnt, data->ref_limit.ref_err_cnt);

	end_page = (data->info->matrix_xsize * data->info->matrix_ysize) / NODE_PER_PAGE;
	for (read_page = 0 ; read_page < end_page; read_page++) {
		__mxt_read_reg(data->client, dbg_object->start_address + 2, NODE_PER_PAGE * DATA_PER_NODE, ret_buf);
		for (i = 0; i < NODE_PER_PAGE; i++) {
			buf[num] = ( ((u16)ret_buf[i * 2 + 1] << 8) + (u16)ret_buf[i*2] ) - REF_OFFSET_VALUE;

			if ((curx >= data->channel_size.start_x + data->channel_size.size_x) || (curx < data->channel_size.start_x) ||
					(cury >= data->channel_size.start_y + data->channel_size.size_y) || (cury < data->channel_size.start_y)) {
				num++;
				cury++;
				if (cury >= data->info->matrix_ysize) {
					curx++;
					cury = 0;
				}
				continue;
			} else {
				if (ref_min > buf[num]) {
					ref_min = buf[num];
				}

				if (ref_max < buf[num]) {
					ref_max = buf[num];
				}

				if (cury <= data->channel_size.start_y) {
					pre_v = buf[num];
				} else {
					diff_v = buf[num] - pre_v;
					if ((diff_v > ((data->ref_limit.y_line_dif[cury] + data->ref_limit.err_weight) * 8)) ||
						(diff_v < ((data->ref_limit.y_line_dif[cury] - data->ref_limit.err_weight) * 8))) {
						LOGD("Err Node(%d, %d) buf<%d> pre_v<%d> diff_v<%d> (ref_max - ref_min) <%d>",
							curx, cury, buf[num], pre_v, diff_v, (ref_max - ref_min));
						LOGD("Weight Node(%d, %d, %d)\n",
							(int)data->ref_limit.y_line_dif[cury],
							(int)((data->ref_limit.y_line_dif[cury] + data->ref_limit.err_weight) * 8),
							(int)((data->ref_limit.y_line_dif[cury] - data->ref_limit.err_weight) * 8));
						err_cnt++;
						++err_tot_x[curx];
						++err_tot_y[cury];
					}
					pre_v = buf[num];

					if (diff_max < diff_v) {
						diff_max = diff_v;
						if (diff_max > (data->ref_limit.ref_diff_max * data->pdata->ref_reg_weight_val)) {
							data->mxt_drv_data->retry_chk++;
							LOGD("Diff max exceed (set : %d) (diff_max : %d)  retry_chk(%d)",
							(data->ref_limit.ref_diff_max * data->pdata->ref_reg_weight_val), diff_max, data->mxt_drv_data->retry_chk);
							if (data->mxt_drv_data->retry_chk >= 10) {
								err_chk = 1;
							} else {
								mxt_t6_command(data, MXT_COMMAND_CALIBRATE, 1, false);
								data->ref_chk = 1;
								return 0;
							}
						}
					}
				}
				num++;
				cury++;
				if (cury >= data->info->matrix_ysize) {
					curx++;
					cury = 0;
				}
			}

			if (num >= 335) break;

			// reference max-min check
			if ((ref_max - ref_min) > (data->ref_limit.ref_rng_limit * data->pdata->ref_reg_weight_val)) {
				data->mxt_drv_data->retry_chk++;
				LOGD("Reference Max - Min exceed (set : %d) (Max - Min : %d) retry_chk(%d)"
					, data->ref_limit.ref_rng_limit * data->pdata->ref_reg_weight_val, (ref_max - ref_min), data->mxt_drv_data->retry_chk);
				if (data->mxt_drv_data->retry_chk >= 10) {
					err_chk = 1;
				} else {
					mxt_t6_command(data, MXT_COMMAND_CALIBRATE, 1, false);
					data->ref_chk = 1;
					return 0;
				}
			}
		}

		ret = mxt_set_diagnostic_mode(data, MXT_DIAG_PAGE_UP);
		if (ret) {
			LOGE("Failed to set diagnostic mode!\n");
			data->mxt_drv_data->retry_chk = 0;
			return 1;
		}

		loop_chk = 0;
		do {
			msleep(20);
			ret = __mxt_read_reg(data->client,
				dbg_object->start_address + MXT_DIAGNOSTIC_PAGE, 1, &cur_page);
			if (ret  || loop_chk++ >= 4) {
				LOGE("%s Read fail page(%d)\n", __func__, loop_chk);
				data->mxt_drv_data->retry_chk = 0;
				return 1;
			}
		} while (cur_page != read_page + 1);
	}


	data->ref_chk = 0;
	LOGE("Err Cnt is %d, limit Cnt is %d\n", err_cnt, half_err_cnt);
	data->patch.src_item[MXT_PATCH_ITEM_USER6] = err_cnt;

	if (err_chk == 1) {
		err_chk = 0;
		data->mxt_drv_data->retry_chk = 0;
		return 1;
	}

	//processing for bad sample
	if (data->ref_limit.ref_x_all_err_line == 1) {
		for (i = 0; i < data->info->matrix_xsize; i++) {
			if (err_tot_x[i] != 0) {
				line_err_chk++;
				err_line_num = i;
			}

			if (err_tot_x[i] >= data->channel_size.size_y - 1) {
				LOGE("X%d Line all Fail, calibration skip error count(%d)\n", i, err_tot_x[i]);
				data->mxt_drv_data->retry_chk = 0;
				return 1;
			}
		}
	}

	if (line_err_chk ==1 ) {
		if (err_tot_x[err_line_num] > data->ref_limit.xline_max_err_cnt) {
			LOGE("X%d Line only Fail, calibration skip error count(%d), limit(%d)\n", i, err_tot_x[i], data->ref_limit.xline_max_err_cnt);
			data->mxt_drv_data->retry_chk = 0;
			return 1;
		}
	}

	line_err_chk = 0;
	err_line_num = 0;

	if (data->ref_limit.ref_y_all_err_line == 1) {
		for (i = 0; i < data->info->matrix_ysize; i++) {
			if (err_tot_y[i] != 0) {
				line_err_chk++;
				err_line_num = i;
			}
			if (err_tot_y[i] >= data->channel_size.size_x - 1) {
				LOGE("Y%d Line all Fail, calibration skip error count(%d)\n", i, err_tot_y[i]);
				data->mxt_drv_data->retry_chk = 0;
				return 1;
			}
		}
	}

	if (line_err_chk == 1 ) {
		if (err_tot_y[err_line_num] > data->ref_limit.yline_max_err_cnt) {
			LOGE("Y%d Line only Fail, calibration skip error count(%d), limit(%d)\n", i, err_tot_y[i], data->ref_limit.yline_max_err_cnt);
			    data->mxt_drv_data->retry_chk = 0;
			    return 1;
		}
	}

	if (err_cnt && (err_cnt >= half_err_cnt)) {
		LOGE("Reference Err Calibration: %d\n",err_cnt);
		mxt_t6_command(data, MXT_COMMAND_CALIBRATE, 1, false);
		data->ref_chk = 1;
	}

	/* butten Hole check */
	if (data->pdata->butt_check_enable) {
		if (err_cnt < half_err_cnt) {
			LOGD("Check the Button Calibration\n");
			for (i = 0; i < data->pdata->t15_num_keys - 1; i++) {
				diff_b_v = buf[data->pdata->t15_key_array_x[i+1]*data->info->matrix_ysize + data->pdata->t15_key_array_y[i+1]] - \
						buf[data->pdata->t15_key_array_x[i]*data->info->matrix_ysize + data->pdata->t15_key_array_y[i]];

				/* button channel check */
				if (diff_b_v > 4000 || diff_b_v < -4000) {
					LOGE("damaged key channel no calibration\n");
					err_chk = 0;
					data->mxt_drv_data->retry_chk = 0;
					return 0;
				}

				if (diff_b_v > (data->ref_limit.butt_dif[i] + data->ref_limit.err_weight) * 8 ||
					diff_b_v < (data->ref_limit.butt_dif[i] - data->ref_limit.err_weight) * 8) {
					LOGD("Prev[%d] Curr[%d] Diff[%d] Range : %d ~ %d\n",
						buf[data->pdata->t15_key_array_x[i+1]*data->info->matrix_ysize + data->pdata->t15_key_array_y[i+1]],
						buf[data->pdata->t15_key_array_x[i]*data->info->matrix_ysize + data->pdata->t15_key_array_y[i]],
						diff_b_v,
						(data->ref_limit.butt_dif[i] + data->ref_limit.err_weight) * 8,
						(data->ref_limit.butt_dif[i] - data->ref_limit.err_weight) * 8);

					LOGE("%s Button Hole Occur!", get_touch_button_string(data->pdata->t15_keymap[i]));
					mxt_t6_command(data, MXT_COMMAND_CALIBRATE, 1, false);
					return 1;
				}
			}
		}
	}

	err_chk = 0;
	data->mxt_drv_data->retry_chk = 0;
	return 0;
}

static void mxt_proc_t9_message(struct mxt_data *data, u8 *message)
{
	struct input_dev *input_dev = data->input_dev;
	int id = 0;
	u8 status = 0;
	int x = 0;
	int y = 0;
	int area = 0;
	int amplitude = 0;
	u8 vector = 0;
	int i = 0;

	/* do not report events if input device not yet registered */
	if (!data->enable_reporting)
		return;

	id = message[0] - data->T9_reportid_min;
	status = message[1];
	x = (message[2] << 4) | ((message[4] >> 4) & 0xf);
	y = (message[3] << 4) | ((message[4] & 0xf));

	/* Handle 10/12 bit switching */
	if (data->max_x < 1024)
		x >>= 2;
	if (data->max_y < 1024)
		y >>= 2;

	area = message[5];
	amplitude = message[6];
	vector = message[7];

	if (unlikely( id < 0 || id >= MXT_MAX_FINGER)) {
		LOGN("%s wrong id:%d\n", __func__, id);
		return;
	}

	if (status & MXT_T9_SUPPRESS) {
		if (data->mxt_drv_data->touched_finger_count) {
			LOGN(" MXT_T9_SUPPRESS\n");
			mxt_reset_slots(data);
		}
		if (data->palm) {
			cancel_delayed_work_sync(&data->work_palm_unlock);
		}
		data->palm = true;
		return;
	}

	if (status & MXT_T9_DETECT) {
		if ((status & MXT_T9_PRESS || (status & MXT_T9_MOVE)) && data->ts_data.prev_data[id].status < MXT_STATE_PRESS) {
			if (data->reported_keycode) {
				LOGN("KEY[%s:%d] is canceled\n", get_touch_button_string(data->reported_keycode), data->reported_keycode);
				input_report_key(input_dev, data->reported_keycode, 0xFF);
				data->reported_keycode = 0;
				return ;
			}

			LOGN("%d finger pressed <%d> : x[%3d] y[%3d] z[%3d] area[%3d]\n", ++data->mxt_drv_data->touched_finger_count, id, x, y, amplitude, area);
		}

		data->fingers[id].state = MXT_STATE_PRESS;
		data->ts_data.curr_data[id].status = MXT_STATE_PRESS;

		input_mt_slot(input_dev, id);
		input_mt_report_slot_state(input_dev, MT_TOOL_FINGER, FINGER_PRESS);
		input_report_abs(input_dev, ABS_MT_POSITION_X, x);
		input_report_abs(input_dev, ABS_MT_POSITION_Y, y);
		input_report_abs(input_dev, ABS_MT_PRESSURE, amplitude);
		input_report_abs(input_dev, ABS_MT_TOUCH_MAJOR, area);
		input_report_abs(input_dev, ABS_MT_ORIENTATION, vector);

		data->button_lock = true;
	}

	if (status & MXT_T9_RELEASE) {
		if (data->mxt_drv_data->touched_finger_count && data->ts_data.prev_data[id].status < MXT_STATE_PRESS) {
			for (i = MXT_MAX_FINGER - 1; i >= 0; i--) {
				if (data->ts_data.prev_data[i].status>= MXT_STATE_PRESS) {
					LOGN("finger id changed <%d> -> <%d>\n", id, i);
					id = i;
					data->ts_data.prev_data[id].status = MXT_STATE_PRESS;
					break;
				}
			}
		}

		input_mt_slot(input_dev, id);
		input_mt_report_slot_state(input_dev, MT_TOOL_FINGER, FINGER_RELEASE);
		LOGN("touch_release    <%d> : x[%3d] y[%3d]\n", id, x, y);
		data->fingers[id].state = MXT_STATE_RELEASE;
		data->ts_data.curr_data[id].status = MXT_STATE_RELEASE;
		data->ts_data.curr_data[id].pressure = 0;
		if (data->mxt_drv_data->touched_finger_count)
			data->ts_data.total_num = --data->mxt_drv_data->touched_finger_count;

		if (!data->mxt_drv_data->touched_finger_count) {
			queue_delayed_work(data->mxt_drv_data->touch_wq, &data->work_button_lock, msecs_to_jiffies(200));
		}
	}

	input_sync(input_dev);

	data->ts_data.curr_data[id].x_position = x;
	data->ts_data.curr_data[id].y_position = y;
	data->ts_data.curr_data[id].pressure = amplitude;
	data->ts_data.curr_data[id].width_major = area;
	data->ts_data.curr_data[id].width_orientation = vector;
	data->ts_data.total_num = data->mxt_drv_data->touched_finger_count;
	data->ts_data.prev_data[id] = data->ts_data.curr_data[id];
	data->ts_data.prev_total_num = data->ts_data.total_num;
	data->update_input = true;

	return ;
}

static void mxt_proc_t15_messages(struct mxt_data *data, u8 *msg)
{
	struct input_dev *input_dev = data->input_dev;
	unsigned long keystates = le32_to_cpu(msg[2]);
	u16 keycode = 0;
	int i = 0;

	/* do not report events if input device not yet registered */
	if (!data->enable_reporting)
		return;

	if ((data->mxt_drv_data->touched_finger_count || data->button_lock || data->palm)) {
		return;
	}

	if (data->reported_keycode == 0 && keystates == 0)
		return;

	for (i = 0; i < data->pdata->t15_num_keys; i++) {
		if (keystates == data->pdata->t15_keystate[i]) {
			keycode = data->pdata->t15_keymap[i];
			break;
		}
	}

	if (data->reported_keycode && keystates == 0) {
		LOGD("KEY[%s:%d] is released\n", get_touch_button_string(data->reported_keycode), data->reported_keycode);
		input_report_key(input_dev, data->reported_keycode, 0);
		data->reported_keycode = 0;
	} else if (data->reported_keycode == 0 && keystates) {
		LOGD("KEY[%s:%d] is pressed\n", get_touch_button_string(keycode), keycode);
		input_report_key(input_dev, keycode, 1);
		data->reported_keycode = keycode;
	}

	input_sync(input_dev);

}
/* T-series of Atmel Touch IC
 * The Touch Suppression T42 does not report its own messages.
 * Screen suppression messages are reported through the linked
 * Multiple Touch Touchscreen T100 object. */
static void mxt_proc_t42_messages(struct mxt_data *data, u8 *msg)
{
	u8 status = msg[1];

	if (status & MXT_T42_MSG_TCHSUP) {
		LOGD("Palm detected %d\n", data->mxt_drv_data->touched_finger_count);
		data->button_lock = true;
	} else {
		LOGD("Palm released \n");
		queue_delayed_work(data->mxt_drv_data->touch_wq, &data->work_button_lock, msecs_to_jiffies(200));
	}
	mxt_reset_slots(data);
}

static int mxt_proc_t48_messages(struct mxt_data *data, u8 *msg)
{
	u8 status = 0, state = 0;

	status = msg[1];
	state = msg[4];

	LOGD("T48 state %d status %02X %s%s%s%s%s\n",
			state,
			status,
			(status & 0x01) ? "FREQCHG " : "",
			(status & 0x02) ? "APXCHG " : "",
			(status & 0x04) ? "ALGOERR " : "",
			(status & 0x10) ? "STATCHG " : "",
			(status & 0x20) ? "NLVLCHG " : "");

	return 0;
}

static void mxt_proc_t57_messages(struct mxt_data *data, u8 *message)
{
	u16 area = 0;
	u16 touch_area = 0;
	u16 anti_touch_area = 0;

	area = (message[2] << 8 | message[1]);
	touch_area = (message[4] << 8 | message[3]);
	anti_touch_area = (message[6] << 8 | message[5]);

	if (data->t57_debug_enabled || data->ref_chk) {
		LOGD("T57 :%3d %3d %3d\n", area, touch_area, anti_touch_area);
	}
}

static int mxt_proc_t25_message(struct mxt_data *data, u8 *message)
{
	u8 status = message[1];

	if (!data->mxt_drv_data->selftest_enable)
		return 0;

	LOGD("T25 Self Test completed %u\n",status);

	memset(data->self_test_status, 0, sizeof(data->self_test_status));

	if (data->mxt_drv_data->selftest_show)
		data->self_test_status[0] = status;

	if ( status == 0xFE ) {
		LOGN("[SUCCESS] All tests passed\n");
		data->self_test_result = true;
	} else {
		if (status == 0xFD) {
			LOGE("[FAIL] Invalid test code\n");
		} else if (status == 0xFC)  {
			LOGE("[FAIL] Unrelated fault\n");
		} else if (status == 0x01) {
			LOGE("[FAIL] AVdd or XVdd is not present\n");
		} else if (status == 0x12) {
			LOGE("[FAIL] Pin fault (SEQ_NUM %u, X_PIN %u, Y_PIN %u)\n", message[2], message[3], message[4]);
			if (data->mxt_drv_data->selftest_show) {
				data->self_test_status[1] = message[2];
				data->self_test_status[2] = message[3];
				data->self_test_status[3] = message[4];
			}
		} else if (status == 0x17) {
			LOGE("[FAIL] Signal limit fault (TYPE_NUM %u, TYPE_INSTANCE %u)\n", message[2], message[3]);
			if (data->mxt_drv_data->selftest_show) {
				data->self_test_status[1] = message[2];
				data->self_test_status[2] = message[3];
			}
		} else;
		data->self_test_result = false;
	}

	data->mxt_drv_data->selftest_enable = false;
	complete(&data->t25_completion);
	return 0;
}

static int mxt_proc_t37_message(struct mxt_data *data, u8 *msg_buf)
{
	struct mxt_object *object = NULL;
	u8 *buf = NULL;
	u8 result = 0;
	int i = 0;
	int cnt = 0;
	int tap_num = 0;
	int msg_size = 0;
	int x = 0;
	int y = 0;
	int ret = 0;

	object = mxt_get_object(data, MXT_DEBUG_DIAGNOSTIC_T37);

	if (!object) {
		LOGE("error Cannot get object_type T%d\n", MXT_DEBUG_DIAGNOSTIC_T37);
		goto error;
	}

	if ( (mxt_obj_size(object) == 0) || (object->start_address == 0)) {
		LOGE("error object_type T%d\n", object->type);
		goto error;
	}

retry:
	msleep(50);		// to need time to write new data

	ret = __mxt_read_reg(data->client, object->start_address, 1, &result);
	if (ret != 0)
		goto error;

	if (result != UDF_MESSAGE_COMMAND) {
		if (cnt == 5) {
			LOGN("cnt = 5, result= %d\n", result);
			goto error;
		}

		msleep(20);
		cnt++;
		goto retry;
	}

	ret = __mxt_read_reg(data->client, object->start_address + 2, 1, &result);
	if (ret != 0)
		goto error;

	tap_num = result;

	if (data->g_tap_cnt != tap_num && data->mxt_multi_tap_enable) {
	    LOGN("Tab count dismatch\n");
	    goto error;
	} else {
	    LOGN("TAP Mode\n");
	}

	msg_size = tap_num * MAX_T37_MSG_SIZE ;
	buf = kmalloc(msg_size, GFP_KERNEL);
	if (!buf)
		goto error;

	ret = __mxt_read_reg(data->client, object->start_address + 3, msg_size, buf);

	if (ret != 0)
		goto error;

	for (i = 0; i < tap_num ; i++) {
		cnt = i * MAX_T37_MSG_SIZE;
		x = (buf[cnt + 1] << 8) | buf[cnt];
		y = (buf[cnt + 3] << 8) | buf[cnt + 2];
		data->mxt_drv_data->tci_press[i].x = x;
		data->mxt_drv_data->tci_press[i].y = y;

		LOGD("%d tap press x: %5u, y: %5u cnt: %d\n", i, x, y, cnt);

		x = (buf[cnt + 5] << 8) | buf[cnt + 4];
		y = (buf[cnt + 7] << 8) | buf[cnt + 6];
		data->mxt_drv_data->tci_report[i].x = x;
		data->mxt_drv_data->tci_report[i].y = y;

		LOGD("%d tap release x: %5u, y: %5u cnt: %d\n", i, x, y, cnt);
	}

	if (buf)
		kfree(buf);

	return 0;

error:
	LOGE("T37 error\n");
	if (buf)
		kfree(buf);

	return 1;
}

#ifdef CONFIG_TOUCHSCREEN_ATMEL_LPWG_DEBUG_REASON
static void mxt_proc_t93_messages(struct mxt_data *data, u8 *message)
{
	u8 lpwg_mode_msg = 0;
	u8 lpwg_fail_msg = 0;
	if (data->in_bootloader)
		return;

	lpwg_mode_msg = message[1];
	lpwg_fail_msg = message[2];

	LOGN("T93 lpwg_mode_msg:0x%x, lpwg_fail_msg:0x%x\n",lpwg_mode_msg, lpwg_fail_msg);

	if (lpwg_mode_msg & MXT_T93_FIRST_TOUCH) {
		mxt_t6_command(data, MXT_COMMAND_DIAGNOSTIC, UDF_MESSAGE_COMMAND, false);
		mxt_proc_t37_message(data, message);
		pm_relax(&data->client->dev);
		pm_wakeup_event(&data->client->dev, msecs_to_jiffies(TAP_WAIT_TIME));
		hrtimer_try_to_cancel(&data->multi_tap_timer);
		if (!hrtimer_callback_running(&data->multi_tap_timer))
			hrtimer_start(&data->multi_tap_timer, ktime_set(0, MS_TO_NS(MXT_WAITED_UDF_TIME)), HRTIMER_MODE_REL);
	} else if (lpwg_mode_msg & MXT_T93_SECOND_TOUCH) {
		LOGN("T93 Knock ON!!\n");
		pm_relax(&data->client->dev);
		pm_wakeup_event(&data->client->dev, msecs_to_jiffies(TAP_WAIT_TIME));
		send_uevent(knockon_event);
	}

}
#else
static void mxt_proc_t93_messages(struct mxt_data *data, u8 *message)
{
	u8 msg = 0;
	int x;
	int y;
	int i;
	int id;
	bool display_on;

	if (data->in_bootloader)
		return;

	msg = message[MXT_T93_TAP_COUNT];

	LOGN("T93 %u\n", msg);

	id = message[0] - data->T93_reportid;
	x = (message[MXT_T93_HIGH_X] << U8_BIT_SHIFT) | message[MXT_T93_LOW_X];
	y = (message[MXT_T93_HIGH_Y] << U8_BIT_SHIFT) | message[MXT_T93_LOW_Y];
	LOGD("x[%4d] y[%4d]\n", x, y);

#if SIDE_KEY_DEVICE
	data->input_dev = data->input_dev_touch;
#endif

	if (msg & MXT_T93_FIRST_TOUCH) {
		mxt_t6_command(data, MXT_COMMAND_DIAGNOSTIC, UDF_MESSAGE_COMMAND, false);
		mxt_proc_t37_message(data, message);
		pm_relax(&data->client->dev);
		pm_wakeup_event(&data->client->dev, msecs_to_jiffies(TAP_WAIT_TIME));
		hrtimer_try_to_cancel(&data->multi_tap_timer);
		if (!hrtimer_callback_running(&data->multi_tap_timer))
			hrtimer_start(&data->multi_tap_timer, ktime_set(0, MS_TO_NS(MXT_WAITED_UDF_TIME)), HRTIMER_MODE_REL);
	} else if (msg & MXT_T93_SECOND_TOUCH) {
		LOGN("T93 Knock ON!!\n");
		pm_relax(&data->client->dev);
		pm_wakeup_event(&data->client->dev, msecs_to_jiffies(TAP_WAIT_TIME));
		send_uevent(knockon_event);
	}
	if (message[MXT_T93_FAIL_REASON] != MXT_T93_DOUBLE_TAP_OK) {
		LOGE("Error code in detecting double tap in AoD mode is %d\n", message[MXT_T93_FAIL_REASON]);
		return;
	}

	display_on = incell_get_system_status();
	for (i = 0; i < AOD_MODE_DOUBLE_TAP; i++) {
		if (!display_on)
			i++;
		input_mt_slot(data->input_dev, id);
		input_mt_report_slot_state(data->input_dev, MT_TOOL_FINGER, FINGER_PRESS);
		input_report_abs(data->input_dev, ABS_MT_POSITION_X, x + i);
		input_report_abs(data->input_dev, ABS_MT_POSITION_Y, y + i);
		input_report_abs(data->input_dev, ABS_MT_PRESSURE, MAX_PRESSURE - i);
		mxt_input_sync(data->input_dev);
		input_mt_slot(data->input_dev, id);
		input_mt_report_slot_state(data->input_dev, MT_TOOL_FINGER, FINGER_RELEASE);
		mxt_input_sync(data->input_dev);
	}
}
#endif /* CONFIG_TOUCHSCREEN_ATMEL_LPWG_DEBUG_REASON */

static void mxt_proc_t24_messages(struct mxt_data *data, u8 *message)
{
	u8 msg = 0;
	int x = 0;
	int y = 0;

	if (data->in_bootloader)
		return;

	msg = message[1];

	x = (message[2] << 4) | ((message[4] >> 4) & 0xf);
	y = (message[3] << 4) | ((message[4] & 0xf));

	/* Handle 10/12 bit switching */
	if (data->max_x < 1024)
		x >>= 2;
	if (data->max_y < 1024)
		y >>= 2;

	if (msg == 0x04) {
		pm_wakeup_event(&data->client->dev, msecs_to_jiffies(2000));
		LOGD("Knock On detected x[%3d] y[%3d]\n", x, y);
		kobject_uevent_env(&device_touch.kobj, KOBJ_CHANGE, knockon_event);
	} else {
		LOGD("%s msg = %d\n", __func__, msg);
	}
}

static void mxt_proc_t80_messages(struct mxt_data *data, u8 *message)
{
	if (data->debug_enabled)
		print_hex_dump(KERN_ERR, "[Solomon] MXT MSG:", DUMP_PREFIX_NONE, 16, 1,
			message, data->T5_msg_size, false);
}

static void mxt_proc_t100_message(struct mxt_data *data, u8 *message)
{
	int id;
	u8 status;
	int x;
	int y;
	int area;
	int amplitude;
	u8 vector;
	u8 height;
	u8 width;
	static int pre_id = 0;
	static int pre_x = 0;
	static int pre_y = 0;
	static int  return_cnt = 0;

	/* do not report events if input device not yet registered */
	if (!data->enable_reporting) {
		LOGD( "return event\n");
		if (return_cnt++ > 30) {
			LOGE( "recalibration\n");
			touch_disable_irq(data);
			queue_delayed_work(data->mxt_drv_data->touch_wq, &data->work_delay_cal, msecs_to_jiffies(10));
			data->delayed_cal = false;
			msleep(50);
			touch_enable_irq(data);
			return_cnt = 0;
		}
		return;
	}

	return_cnt = 0;
	id = message[0] - data->T100_reportid_min - 2;

	status = message[1];
	x = (message[3] << U8_BIT_SHIFT) | message[2];
	y = (message[5] << U8_BIT_SHIFT) | message[4];

	vector =  message[data->t100_aux_vect];
	amplitude = message[data->t100_aux_ampl];	/* message[6] */
	area = message[data->t100_aux_area];

	height = message[data->t100_aux_resv];
	width = message[data->t100_aux_resv+1];

	if (status & MXT_T100_DETECT) {
		/* Multiple bits may be set if the host is slow to read the
		* status messages, indicating all the events that have
		* happened */

		if ((status & MXT_T100_TYPE_MASK) == MXT_T100_TYPE_STYLUS) {
			if (data->stylus_in_a_row_cnt == 0) {
				pre_id = id;
			}

			if (data->patch.src_item[MXT_PATCH_ITEM_USER4] == 0) {
				if (pre_id == id) {
					if (data->stylus_in_a_row_cnt_thr < data->stylus_in_a_row_cnt++) {
						data->patch.src_item[MXT_PATCH_ITEM_USER4] = 1;
						LOGN("Stylus Continually Mode Enter\n");
						data->stylus_in_a_row_cnt = 0;
					}
				} else {
					data->stylus_in_a_row_cnt = 0;
				}
			} else if (data->patch.src_item[MXT_PATCH_ITEM_USER4] == 1) {
				if (data->stylus_in_a_row_cnt == 0) {
					pre_x = x;
					pre_y = y;
				}

				if (pre_id == id                  &&
				    pre_x <= (x + data->x_zitter) &&
				    pre_x >= (x - data->x_zitter) &&
				    pre_y <= (y + data->y_zitter) &&
				    pre_y >= (y - data->y_zitter)) {
					if (data->stylus_in_a_row_cnt_thr < data->stylus_in_a_row_cnt++) {
						data->patch.src_item[MXT_PATCH_ITEM_USER4] = 2;
						LOGN("Stylus Continually Mode Exit\n");
						data->stylus_in_a_row_cnt = 0;
					}
				} else {
					data->stylus_in_a_row_cnt = 0;
				}
			} else if (data->patch.src_item[MXT_PATCH_ITEM_USER4] == 2) {
				if (data->stylus_in_a_row_cnt++ > 100) {
					LOGN("Stylus Continually Mode Re-Chk\n");
					data->patch.src_item[MXT_PATCH_ITEM_USER4] = 0;
					data->stylus_in_a_row_cnt = 0;
				}
			}
		} else {
			data->stylus_in_a_row_cnt = 0;
		}

#if SIDE_KEY_DEVICE
		if ((status & MXT_T100_STATUS_MASK) == MXT_T100_PRESS && (status & MXT_T100_TYPE_MASK) == MXT_T100_TYPE_SIDEKEY)
			data->side_key_storage[id] = y;
		else if ((status & MXT_T100_STATUS_MASK) == MXT_T100_MOVE && (status & MXT_T100_TYPE_MASK) == MXT_T100_TYPE_SIDEKEY) {
			if (abs(data->side_key_storage[id] - y) < SIDE_KEY_MINOR_MOVE)
				return;
		}
#endif

		if (data->landscape) {
			if ((status & MXT_T100_STATUS_MASK) == MXT_T100_PRESS) {
				if ((x < landscape_buffer[0] || x > data->max_x - landscape_buffer[0]) && (y < landscape_buffer[1] || y > data->max_y - landscape_buffer[1])) {
					if (!data->report_flag[id])
						return;
				} else
					data->report_flag[id] = true;
			} else if ((status & MXT_T100_STATUS_MASK) == MXT_T100_MOVE) {
				if ((x < landscape_buffer[0] || x > data->max_x - landscape_buffer[0]) && (y < landscape_buffer[1] || y > data->max_y - landscape_buffer[1])) {
					if (!data->report_flag[id])
						return;
				} else if ((x > landscape_buffer[2] && x < data->max_x - landscape_buffer[2]) || (y > landscape_buffer[3] && y < data->max_y - landscape_buffer[3])) {
					if (!data->report_flag[id])
						data->report_flag[id] = true;
				} else if (!data->report_flag[id])
					return;
			}
		} else {
			if ((status & MXT_T100_STATUS_MASK) == MXT_T100_PRESS) {
				if ((x < portrait_buffer[0] || x > data->max_x - portrait_buffer[0]) && (y > data->max_y - portrait_buffer[1])) {
					if (!data->report_flag[id])
						return;
				} else
					data->report_flag[id] = true;
			} else if ((status & MXT_T100_STATUS_MASK) == MXT_T100_MOVE) {
				if ((x < portrait_buffer[0] || x > data->max_x - portrait_buffer[0]) && (y > data->max_y - portrait_buffer[1])) {
					if (!data->report_flag[id])
						return;
				} else if ((x > portrait_buffer[2] && x < data->max_x - portrait_buffer[2]) || (y < data->max_y - portrait_buffer[3])) {
					if (!data->report_flag[id])
						data->report_flag[id] = true;
				} else if (!data->report_flag[id])
					return;
			}
		}

		if ((status & MXT_T100_STATUS_MASK) == MXT_T100_RELEASE || (status & MXT_T100_STATUS_MASK) == MXT_T100_SUPPRESSION) {
			data->ts_data.curr_data[id].id = id;
			data->ts_data.curr_data[id].status = FINGER_RELEASED;
			if ((status & MXT_T100_STATUS_MASK) == MXT_T100_SUPPRESSION)
				LOGE(  "T100_message[%u] ###DETECT && SUPPRESSION (%02X)\n", id, status);
		}

		data->ts_data.curr_data[id].id = id;
		data->ts_data.curr_data[id].x_position = x;
		data->ts_data.curr_data[id].y_position = y;
		data->ts_data.curr_data[id].area = area;
#ifdef CONFIG_TOUCHSCREEN_ATMEL_STYLUS_PEN
		if ((status & MXT_T100_TYPE_MASK) == MXT_T100_TYPE_STYLUS) {
			data->ts_data.curr_data[id].tool = MT_TOOL_PEN;
			data->ts_data.curr_data[id].is_pen = true;
		} else {
			data->ts_data.curr_data[id].tool = MT_TOOL_FINGER;
			data->ts_data.curr_data[id].is_pen = false;
		}
#else
		data->ts_data.curr_data[id].tool = MT_TOOL_FINGER;
#endif

#if SIDE_KEY_DEVICE
		if ((status & MXT_T100_TYPE_MASK) == MXT_T100_TYPE_SIDEKEY) {
			data->ts_data.curr_data[id].is_sidekey = true;
		} else {
			data->ts_data.curr_data[id].is_sidekey = false;
		}
#endif

		if (amplitude == 255 &&
				((status & MXT_T100_TYPE_MASK) == MXT_T100_TYPE_FINGER
				 || (status & MXT_T100_TYPE_MASK) == MXT_T100_TYPE_STYLUS)) {
			data->ts_data.curr_data[id].pressure = 240;
		} else if ((status & MXT_T100_TYPE_MASK) == MXT_T100_TYPE_PALM) {
			data->ts_data.curr_data[id].pressure = MAX_PRESSURE;
#ifdef CONFIG_TOUCHSCREEN_ATMEL_STYLUS_PEN
			data->ts_data.curr_data[id].is_palm = true;
#endif
			LOGE("Palm Detected\n");
		} else {
			data->ts_data.curr_data[id].pressure = amplitude;
		}

		if (height >= width) {
			data->ts_data.curr_data[id].touch_major = height;
			data->ts_data.curr_data[id].touch_minor = width;
		} else {
			data->ts_data.curr_data[id].touch_major = width;
			data->ts_data.curr_data[id].touch_minor = height;
		}

		if ((status & MXT_T100_STATUS_MASK) == MXT_T100_PRESS) {
			data->ts_data.curr_data[id].status = FINGER_PRESSED;
		} else if ((status & MXT_T100_STATUS_MASK) == MXT_T100_MOVE) {
			data->ts_data.curr_data[id].status = FINGER_MOVED;
		}

		if ((status & MXT_T100_STATUS_MASK) == MXT_T100_PRESS) {
#if SIDE_KEY_DEVICE
			if ((status & MXT_T100_TYPE_MASK) == MXT_T100_TYPE_SIDEKEY)
				LOGD("sidekey_pressed  <%d> : x[%4d] y[%4d] A[%3d] P[%3d] WM[%3d] Wm[%3d]\n",
					id, x, y, area, amplitude, data->ts_data.curr_data[id].touch_major, data->ts_data.curr_data[id].touch_minor);
			else
				LOGD("touch_pressed    <%d> : x[%4d] y[%4d] A[%3d] P[%3d] WM[%3d] Wm[%3d]\n",
					id, x, y, area, amplitude, data->ts_data.curr_data[id].touch_major, data->ts_data.curr_data[id].touch_minor);
#else
			LOGD("touch_pressed    <%d> : x[%4d] y[%4d] A[%3d] P[%3d] WM[%3d] Wm[%3d]\n",
				id, x, y, area, amplitude, data->ts_data.curr_data[id].touch_major, data->ts_data.curr_data[id].touch_minor);
#endif
		}
	} else {
		/* Touch Release */
		data->ts_data.curr_data[id].id = id;
		data->ts_data.curr_data[id].status = FINGER_RELEASED;
		data->report_flag[id] = false;
#if SIDE_KEY_DEVICE
		if ((status & MXT_T100_TYPE_MASK) == MXT_T100_TYPE_SIDEKEY)
			LOGD("sidekey_release    <%d> : x[%4d] y[%4d]\n", id, x, y);
		else
			LOGD("touch_release    <%d> : x[%4d] y[%4d]\n", id, x, y);
#else
		LOGD("touch_release    <%d> : x[%4d] y[%4d]\n", id, x, y);
#endif

#if SIDE_KEY_DEVICE
		if ((status & MXT_T100_TYPE_MASK) == MXT_T100_TYPE_SIDEKEY) {
			data->ts_data.curr_data[id].is_sidekey = true;
		} else {
			data->ts_data.curr_data[id].is_sidekey = false;
		}
#endif

	}

	if (data->debug_enabled) {
		LOGN( "T100_message[%u] %s%s%s%s%s%s%s%s%s %s%s%s%s%s (0x%02X) x:%u y:%u z:%u area:%u amp:%u vec:%u h:%u w:%u\n",
			id,
			((status & MXT_T100_STATUS_MASK) == MXT_T100_MOVE) ? "MOVE" : "",
			((status & MXT_T100_STATUS_MASK) == 2) ? "UNSUP" : "",
			((status & MXT_T100_STATUS_MASK) == 3) ? "SUP" : "",
			((status & MXT_T100_STATUS_MASK) == MXT_T100_PRESS) ? "PRESS" : "",
			((status & MXT_T100_STATUS_MASK) == MXT_T100_RELEASE) ? "RELEASE" : "",
			((status & MXT_T100_STATUS_MASK) == 6) ? "UNSUPSUP" : "",
			((status & MXT_T100_STATUS_MASK) == 7) ? "UNSUPUP" : "",
			((status & MXT_T100_STATUS_MASK) == 8) ? "DOWNSUP" : "",
			((status & MXT_T100_STATUS_MASK) == 9) ? "DOWNUP" : "",
			((status & MXT_T100_TYPE_MASK) == MXT_T100_TYPE_FINGER) ? "FIN" : ".",
			((status & MXT_T100_TYPE_MASK) == MXT_T100_TYPE_STYLUS) ? "PEN" : ".",
			((status & MXT_T100_TYPE_MASK) == MXT_T100_TYPE_PALM) ? "PALM" : ".",
			((status & MXT_T100_TYPE_MASK) == 0x40) ? "HOVER" : ".",
			((status & MXT_T100_TYPE_MASK) == 0x30) ? "ACTSTY" : ".",
			status, x, y, data->ts_data.curr_data[id].pressure, area, amplitude, vector,
			height, width);
	}

	data->update_input = true;

	return ;
}
//641t porting E

static void mxt_proc_t109_messages(struct mxt_data *data, u8 *msg)
{
	u8 command = msg[1];
	int ret = 0;

	LOGD("%s CMD: %u\n", __func__, command);

	if (command == MXT_T109_CMD_TUNE) {
		LOGD("%s Store to Configuration RAM.\n", __func__);
		mxt_t109_command(data, MXT_T109_CMD, MXT_T109_CMD_STORE_RAM);
	} else if (command == MXT_T109_CMD_STORE_RAM) {
		/* Self Tune Completed.. */
		mxt_write_object(data, MXT_SPT_USERDATA_T38, 1, 1);

		/* Backup to memory */
		ret = mxt_command_backup(data, MXT_BACKUP_VALUE);
		if (ret) {
			LOGE("Failed backup NV data\n");
			return;
		}

		/* Soft reset */
		ret = mxt_command_reset(data, MXT_RESET_VALUE);
		if (ret) {
			LOGE("Failed Reset IC\n");
			return;
		}

		LOGD("%s Self Tune Complete.\n", __func__);
	}
}

static int mxt_update_file_name(struct device *dev, char **file_name, const char *buf, size_t count)
{
	char *file_name_tmp = NULL;

	/* Simple sanity check */
	if (count >= PAGE_SIZE) {
		LOGE("File name too long %zu\n", count);
		return -EINVAL;
	}

	file_name_tmp = krealloc(*file_name, count + 1, GFP_KERNEL);
	if (!file_name_tmp) {
		LOGE("no memory\n");
		return -ENOMEM;
	}

	*file_name = file_name_tmp;
	memcpy(*file_name, buf, count);

	/* Echo into the sysfs entry may append newline at the end of buf */
	if (buf[count - 1] == '\n')
		(*file_name)[count - 1] = '\0';
	else
		(*file_name)[count] = '\0';

	return 0;
}

static ssize_t mxt_update_patch_store(struct mxt_data *data, const char *buf, size_t count)
{
	u8 *patch_data = NULL;
	const struct firmware *fw = NULL;
	char *name = NULL;
	int ret = 0;

	ret = mxt_update_file_name(&data->client->dev, &name, buf, count);
	if (ret) {
		LOGE("%s error patch name [%s]\n", __func__, name);
		goto out;
	}

	if (request_firmware(&fw, name, &data->client->dev) >= 0) {
		patch_data = kzalloc(fw->size, GFP_KERNEL);
		if (!patch_data) {
			LOGE("Failed to alloc buffer for fw\n");
			ret = -ENOMEM;
			goto out;
		}
		memcpy(patch_data, fw->data, fw->size);
		if (data->patch.patch) {
			kfree(data->patch.patch);
			data->patch.patch = NULL;
		}
		data->patch.patch = patch_data;
	} else {
		LOGE("Fail to request firmware(%s)\n", name);
		goto out;
	}
	LOGD("%s patch file size:%zu\n", __func__, fw->size);
	LOGD("%s ppatch:%p %p\n", __func__, patch_data, data->patch.patch);

	ret = mxt_patch_init(data, data->patch.patch);
	if (ret == 0) {
		data->mxt_drv_data->mxt_data = data;
	} else {
		data->mxt_drv_data->mxt_data = NULL;
		LOGE("%s data->mxt_drv_data->mxt_data is NULL \n", __func__);
	}

	if (fw)
		release_firmware(fw);

	return 1;

out:
	if (patch_data) {
		kfree(patch_data);
	}
	data->patch.patch = NULL;

	return 1;
}

void trigger_usb_state_from_otg(struct mxt_data *data, int usb_type)
{
	struct mxt_data *mxt_data = data->mxt_drv_data->mxt_data;
	LOGN("USB trigger USB_type: %d \n", usb_type);
	data->mxt_drv_data->usb_type = usb_type;
	if (mxt_data && mxt_data->patch.event_cnt) {

		mxt_data->global_object = mxt_get_object(mxt_data,
									MXT_PROCI_TOUCH_SEQUENCE_LOGGER_T93);
		if (!mxt_data->global_object)
			return;

		if (mxt_data->regulator_status == 0) {
			LOGN("IC Regulator Disabled. Do nothing\n");
			if (usb_type == 0) {
				mxt_patchevent_unset(data, PATCH_EVENT_TA);
				mxt_data->charging_mode = 0;
			} else {
				mxt_patchevent_set(data, PATCH_EVENT_TA);
				mxt_data->charging_mode = 1;
			}
			return;
		}

		if (mxt_data->mxt_mode_changed) {
			LOGN("Do not change when mxt mode changed\n");
			if (usb_type == 0) {
				mxt_patchevent_unset(data, PATCH_EVENT_TA);
				mxt_data->charging_mode = 0;
			} else {
				mxt_patchevent_set(data, PATCH_EVENT_TA);
				mxt_data->charging_mode = 1;
			}
			return;
		}

		if (mxt_data->mfts_enable && mxt_data->pdata->use_mfts) {
			LOGN("MFTS : Not support USB trigger \n");
			return;
		}

		pm_wakeup_event(&mxt_data->client->dev, msecs_to_jiffies(2000));

		if (mutex_is_locked(&data->mxt_drv_data->i2c_suspend_lock)) {
			LOGD("%s mutex_is_locked \n", __func__);
		}

		mutex_lock(&data->mxt_drv_data->i2c_suspend_lock);
		if (usb_type == 0) {
			if (mxt_patchevent_get(data, PATCH_EVENT_TA)) {
				if (mxt_patchevent_get(data, PATCH_EVENT_KNOCKON)) {
					mxt_patchevent_unset(data, PATCH_EVENT_KNOCKON);

					if (mxt_data->lpwg_mode == LPWG_DOUBLE_TAP) {
						mxt_data->knock_on_mode = CHARGER_KNOCKON_WAKEUP;
					} else if (mxt_data->lpwg_mode == LPWG_MULTI_TAP) {
						mxt_data->knock_on_mode = CHARGER_KNOCKON_WAKEUP + PATCH_EVENT_PAIR_NUM;
						/* Write TCHCNTTHR(Touch Count Threshold) for LPWG_MULTI_TAP */
						mxt_data->error = mxt_write_reg(mxt_data->client,
							mxt_data->global_object->start_address+17,
							mxt_data->g_tap_cnt);
						if (mxt_data->error)
							LOGE("Object Write Fail\n");
					}
				}

				mxt_data->charging_mode = 0;
				mxt_patch_event(mxt_data, CHARGER_UNplugged);
				mxt_patchevent_unset(data, PATCH_EVENT_TA);
			}
		} else {
			if (mxt_patchevent_get(data, PATCH_EVENT_KNOCKON)) {
				if (mxt_data->lpwg_mode == LPWG_DOUBLE_TAP){
					mxt_data->knock_on_mode = NOCHARGER_KNOCKON_WAKEUP;
				} else if (mxt_data->lpwg_mode == LPWG_MULTI_TAP) {
					mxt_data->knock_on_mode = NOCHARGER_KNOCKON_WAKEUP + PATCH_EVENT_PAIR_NUM;
					/* Write TCHCNTTHR(Touch Count Threshold) for LPWG_MULTI_TAP */
					mxt_data->error = mxt_write_reg(mxt_data->client,
						mxt_data->global_object->start_address+17,
						mxt_data->g_tap_cnt);
					if (mxt_data->error)
						LOGE("Object Write Fail\n");
				}
				mxt_patchevent_unset(data, PATCH_EVENT_KNOCKON);
			}
			mxt_data->charging_mode = 1;

			if (data->mxt_drv_data->factorymode)
				mxt_patch_event(mxt_data, CHARGER_PLUGGED_AAT);
			else
				mxt_patch_event(mxt_data, CHARGER_PLUGGED);

			mxt_patchevent_set(data, PATCH_EVENT_TA);
		}

		mutex_unlock(&data->mxt_drv_data->i2c_suspend_lock);
	} else {
		LOGE("mxt_data is null\n");
	}
}


static void mxt_proc_message_log(struct mxt_data *data, u8 type)
{
	if (mxt_patchevent_get(data, PATCH_EVENT_KNOCKON)) {
		if (type != 6 && type != 46 && type != 56 && type != 61 && type != 72)
			LOGD("mxt_interrupt T%d\n", type);
	}
}

static int mxt_proc_message(struct mxt_data *data, u8 *message)
{
	u8 report_id = message[0];
	u8 type = 0;
	bool dump = data->debug_enabled;

	if (report_id == MXT_RPTID_NOMSG)
		return 0;

	type = data->reportids[report_id].type;

	mxt_proc_message_log(data, type);

	if (type == MXT_GEN_COMMAND_T6) {
		mxt_proc_t6_messages(data, message);
	} else if (type == MXT_TOUCH_MULTI_T9) {
		if (data->mxt_multi_tap_enable && data->suspended) {
			waited_udf(data, message);
			return 1;
		} else {
			mxt_proc_t9_message(data, message);
		}
	} else if (type == MXT_TOUCH_KEYARRAY_T15) {
		mxt_proc_t15_messages(data, message);
	} else if (type == MXT_PROCI_ONETOUCH_T24 && data->mxt_knock_on_enable) {
		mxt_proc_t24_messages(data, message);
	} else if (type == MXT_SPT_SELFTEST_T25) {
		mxt_proc_t25_message(data, message);
	} else if (type == MXT_PROCI_TOUCHSUPPRESSION_T42) {
		mxt_proc_t42_messages(data, message);
	} else if (type == MXT_SPT_CTECONFIG_T46) {
		LOGD("MXT_SPT_CTECONFIG_T46\n");
	} else if (type == MXT_PROCG_NOISESUPPRESSION_T48) {
		mxt_proc_t48_messages(data, message);
	} else if (type == MXT_PROCI_SHIELDLESS_T56) {
		LOGD("MXT_PROCI_SHIELDLESS_T56\n");
	} else if (type == MXT_PROCI_EXTRATOUCHSCREENDATA_T57) {
		mxt_proc_t57_messages(data, message);
	} else if (type == MXT_PROCG_NOISESUPPRESSION_T72) {
		LOGD("MXT_PROCG_NOISESUPPRESSION_T72 Noise Status:%d\n", message[2] & 0x07);
	} else if (type == MXT_RETRANSMISSIONCOMPENSATION_T80) {
		mxt_proc_t80_messages(data, message);
	} else if (type == MXT_PROCI_TOUCH_SEQUENCE_LOGGER_T93 && data->lpwg_mode) {
		mxt_proc_t93_messages(data, message);
	} else if (type == MXT_TOUCH_MULTITOUCHSCREEN_T100) {
		if (report_id == data->T100_reportid_min || report_id == data->T100_reportid_min + 1) {
			/* do nothing */
		} else {
			if (data->mxt_multi_tap_enable && data->suspended) {
				waited_udf(data, message);
				return 1;
			} else {
				mxt_proc_t100_message(data, message);
			}
		}
	} else if (type == MXT_SPT_SELFCAPGLOBALCONFIG_T109) {
		mxt_proc_t109_messages(data, message);
	} else {
		if (type != MXT_SPT_TIMER_T61) {
			LOGE("%s : Unknown T%d\n", __func__, type);
			mxt_dump_message(data, message);
		}
	}

	if (dump)
		mxt_dump_message(data, message);

	if (mxt_power_block_get(data) == 0)
		mxt_patch_message(data, (struct mxt_message*)message);

	return 1;
}

static int mxt_read_and_process_messages(struct mxt_data *data, u8 count)
{
	int ret = 0;
	int i = 0;
	u8 num_valid = 0;

	/* Safety check for msg_buf */
	if (count > data->max_reportid)
		return -EINVAL;

	if (data->msg_buf == NULL) {
		LOGE("%s data->msg_buf = NULL\n", __func__);
	}

	/* Process remaining messages if necessary */
	ret = __mxt_read_reg(data->client, data->T5_address, data->T5_msg_size * count, data->msg_buf);
	if (ret) {
		LOGE("Failed to read %u messages (%d)\n", count, ret);
		return ret;
	}

	for (i = 0;  i < count; i++) {
		ret = mxt_proc_message(data, data->msg_buf + data->T5_msg_size * i);

		if (ret == 1)
			num_valid++;
	}

	/* return number of messages read */
	return num_valid;
}

static irqreturn_t mxt_process_messages_t44(struct mxt_data *data)
{
	int ret;
	int report_num = 0;
	int i = 0;
	u8 count, num_left;

#if SIDE_KEY_DEVICE
	bool update_touch = false;
	bool update_sidekey = false;
#endif

	/* Read T44 and T5 together */
	ret = __mxt_read_reg(data->client, data->T44_address,
		data->T5_msg_size + 1, data->msg_buf);

	if (ret) {
		LOGE( "Failed to read T44 and T5 (%d)\n", ret);
		return IRQ_NONE;
	}

	count = data->msg_buf[0];

	if (count == 0) {
		/* Zero message is too much occured.
		 * Remove this log until firmware fixed */
		return IRQ_HANDLED;
	} else if (count > data->max_reportid) {
		LOGE("T44 count %d exceeded max report id\n", count);
		count = data->max_reportid;
	}

	data->ts_data.total_num = 0;

	/* Process first message */
	ret = mxt_proc_message(data, data->msg_buf + 1);
	if (ret < 0) {
		LOGE( "Unexpected invalid message\n");
		return IRQ_NONE;
	}

	num_left = count - 1;

	/* Process remaining messages if necessary */
	if (num_left) {
		ret = mxt_read_and_process_messages(data, num_left);
		if (ret < 0)
			goto end;
		else if (ret != num_left)
			LOGE( "Unexpected invalid message\n");
	}

	for (i = 0; i < data->pdata->numtouch; i++) {
		if (data->ts_data.curr_data[i].status == FINGER_INACTIVE &&
			data->ts_data.prev_data[i].status != FINGER_INACTIVE &&
			data->ts_data.prev_data[i].status != FINGER_RELEASED) {
			memcpy(&data->ts_data.curr_data[i], &data->ts_data.prev_data[i], sizeof(data->ts_data.prev_data[i]));
			data->ts_data.curr_data[i].skip_report = true;
		} else if (data->ts_data.curr_data[i].status == FINGER_INACTIVE) {
			continue;
		}

		if (data->ts_data.curr_data[i].status == FINGER_PRESSED ||
			data->ts_data.curr_data[i].status == FINGER_MOVED) {
			data->ts_data.total_num++;
		}
		report_num++;
	}

	if (!data->enable_reporting || !report_num)
		goto out;

	for (i = 0; i < data->pdata->numtouch; i++) {
		if (data->ts_data.curr_data[i].status == FINGER_INACTIVE || data->ts_data.curr_data[i].skip_report) {
			continue;
		}
		if (data->ts_data.curr_data[i].status == FINGER_RELEASED && data->ts_data.prev_data[i].status != FINGER_RELEASED) {

#if SIDE_KEY_DEVICE
			if (data->ts_data.curr_data[i].is_sidekey) {
				data->input_dev = data->input_dev_sidekey;
				update_sidekey = true;
			} else {
				data->input_dev = data->input_dev_touch;
				update_touch = true;
			}
#endif
			input_mt_slot(data->input_dev, data->ts_data.curr_data[i].id);
			input_mt_report_slot_state(data->input_dev, MT_TOOL_FINGER, FINGER_RELEASE);
		} else {

#if SIDE_KEY_DEVICE
			if (data->ts_data.curr_data[i].is_sidekey) {
				data->input_dev = data->input_dev_sidekey;
				update_sidekey = true;
			} else {
				data->input_dev = data->input_dev_touch;
				update_touch = true;
			}
#endif
			if (data->ts_data.curr_data[i].status == FINGER_RELEASED) {
				input_mt_slot(data->input_dev, data->ts_data.curr_data[i].id);
				input_mt_report_slot_state(data->input_dev, MT_TOOL_FINGER, FINGER_RELEASE);
			} else {
				if (data->touch_type == MXT_T100_TYPE_GLOVE_TOUCH)
					if (data->ts_data.curr_data[i].id >= GLOVE_MODE_FINGER_LIMIT)
						goto skip;
				input_mt_slot(data->input_dev, data->ts_data.curr_data[i].id);
				input_mt_report_slot_state(data->input_dev, MT_TOOL_FINGER, FINGER_PRESS);
				input_report_abs(data->input_dev, ABS_MT_TRACKING_ID, data->ts_data.curr_data[i].id);
				input_report_abs(data->input_dev, ABS_MT_POSITION_X, data->ts_data.curr_data[i].x_position);
				input_report_abs(data->input_dev, ABS_MT_POSITION_Y, data->ts_data.curr_data[i].y_position);
				input_report_abs(data->input_dev, ABS_MT_PRESSURE, data->ts_data.curr_data[i].pressure);
				input_report_abs(data->input_dev, ABS_MT_WIDTH_MAJOR, data->ts_data.curr_data[i].touch_major);
				input_report_abs(data->input_dev, ABS_MT_WIDTH_MINOR, data->ts_data.curr_data[i].touch_minor);
			}
		}
	}

skip:
	if (data->ts_data.total_num < data->ts_data.prev_total_num)
		LOGD( "Total_num(move+press)= %d\n",data->ts_data.total_num);

	if (data->ts_data.total_num) {
		data->ts_data.prev_total_num = data->ts_data.total_num;
		memcpy(data->ts_data.prev_data, data->ts_data.curr_data, sizeof(data->ts_data.curr_data));
	} else {
		data->ts_data.prev_total_num = 0;
		memset(data->ts_data.prev_data, 0, sizeof(data->ts_data.prev_data));
	}
	memset(data->ts_data.curr_data, 0, sizeof(data->ts_data.curr_data));

end:
	if (data->update_input) {
#if SIDE_KEY_DEVICE
		if (update_touch) {
			data->input_dev = data->input_dev_touch;
			mxt_input_sync(data->input_dev);
		}
		if (update_sidekey) {
			data->input_dev = data->input_dev_sidekey;
			mxt_input_sync(data->input_dev);
		}
#else
		mxt_input_sync(data->input_dev);
#endif
		data->update_input = false;
	}

out:
	return IRQ_HANDLED;
}

static irqreturn_t mxt_process_messages(struct mxt_data *data)
{
	int total_handled = 0, num_handled = 0;
	u8 count = data->last_message_count;

	if (count < 1 || count > data->max_reportid)
		count = 1;

	/* include final invalid message */
	total_handled = mxt_read_and_process_messages(data, count);
	if (total_handled < 0)
		return IRQ_HANDLED;

	/* if there were invalid messages, then we are done */
	else if (total_handled <= count)
		goto update_count;

	/* read two at a time until an invalid message or else we reach
	 * reportid limit */
	do {
		num_handled = mxt_read_and_process_messages(data, 2);
		if (num_handled < 0)
			return IRQ_HANDLED;

		total_handled += num_handled;

		if (num_handled < 2)
			break;
	} while (total_handled < data->num_touchids);

update_count:
	data->last_message_count = total_handled;

	if (data->enable_reporting && data->update_input) {
		mxt_input_sync(data->input_dev);
		data->update_input = false;
	}

	return IRQ_HANDLED;
}
#ifdef CONFIG_TOUCHSCREEN_ATMEL_LPWG_DEBUG_REASON
static void save_lpwg_debug_reason_print(struct mxt_data *data)
{
	int i = 0;
	struct mxt_object *object = NULL;
	u8 value[10];

	mxt_t6_command(data, MXT_COMMAND_DIAGNOSTIC, UDF_MESSAGE_COMMAND, true);
	mxt_t6_command(data, MXT_COMMAND_DIAGNOSTIC, MXT_DIAG_PAGE_DOWN, true);
	object = mxt_get_object(data, MXT_DEBUG_DIAGNOSTIC_T37);
	if (object) {
		__mxt_read_reg(data->client, object->start_address + 2, 10, value);
		for (i = 0;i < 10;i++) {
			if (data->lpwg_mode == LPWG_DOUBLE_TAP) {
				LOGN("LPWG DEBUG Reason[%d]:%s\n", i, knock_on_debug_str[value[i] & 0x0F]);
			} else {
				LOGN("LPWG DEBUG Reason[%d] on:%s code:%s\n", i, knock_on_debug_str[value[i] & 0x0F], knock_code_debug_str[(value[i] & 0xF0) >> 4]);
			}
		}
	} else {
		LOGE("Failed get T37 Object\n");
	}
}
static void save_lpwg_debug_reason(struct mxt_data *data, u8 *value)
{
	int i = 0;
	struct mxt_object *object = NULL;
	int len = 0;
	char buf[100] = {0,};

	mxt_t6_command(data, MXT_COMMAND_DIAGNOSTIC, UDF_MESSAGE_COMMAND, true);
	mxt_t6_command(data, MXT_COMMAND_DIAGNOSTIC, MXT_DIAG_PAGE_DOWN, true);
	object = mxt_get_object(data, MXT_DEBUG_DIAGNOSTIC_T37);
	if (object) {
		__mxt_read_reg(data->client, object->start_address + 2, 10, value);
		for (i = 0;i < 10;i++) {
			len += snprintf(buf + len, PAGE_SIZE - len, "0x%x, ", value[i]);
		}
		len += snprintf(buf + len, PAGE_SIZE - len, "\n");
		LOGN("LPWG DEBUG Reason : %s len:%d\n", buf, len);
	} else {
		LOGE("Failed get T37 Object\n");
	}
}
static int mxt_lpwg_debug_interrupt_control(struct mxt_data* data, int mode)
{
	int ret = -1;
	u8 value[2] = {0,};
	u8 read_value[2] = {0,};
	struct mxt_object *object = NULL;

	LOGN("%s\n", __func__);

	if (!mode)
		return 0;

	object = mxt_get_object(data, MXT_PROCI_TOUCH_SEQUENCE_LOGGER_T93);
	if (object) {
		ret = __mxt_read_reg(data->client, object->start_address + 28, 2, read_value);
		if (ret) {
			LOGE("%s Read Failed T93\n",__func__);
			return ret;
		} else {
			LOGD("%s debug reason ctrl status:0x%x, 0x%x\n", __func__, read_value[0], read_value[1]);
		}
	} else {
		return ret;
	}

	if (data->lpwg_mode == LPWG_DOUBLE_TAP) {
		value[0] = 0x1E;
		value[1] = 0x00;
	} else if (data->lpwg_mode == LPWG_MULTI_TAP) {
		value[0] = 0x3C;
		value[1] = 0x00;
	}

	if ((read_value[0] == value[0]) && (read_value[1] == value[1])) {
		LOGD("T93 FAILRPTEN Field write skip\n");
		return ret;
	}

	if (object) {
		ret = __mxt_write_reg(data->client, object->start_address + 28, 2, value);
		if (ret) {
			LOGE("%s Write Failed T93\n",__func__);
			return ret;
		}

		ret = __mxt_read_reg(data->client, object->start_address + 28, 2, value);
		if (ret) {
			LOGE("%s Read Failed T93\n",__func__);
			return ret;
		} else {
			LOGD("%s new debug reason ctrl status:0x%x, 0x%x\n", __func__, value[0], value[1]);
		}
	}
	return ret;
}
#endif
/* touch_irq_handler
 *
 * When Interrupt occurs, it will be called before touch_thread_irq_handler.
 *
 * return
 * IRQ_HANDLED: touch_thread_irq_handler will not be called.
 * IRQ_WAKE_THREAD: touch_thread_irq_handler will be called.
 */
static irqreturn_t touch_irq_handler(int irq, void *dev_id)
{
	struct mxt_data *data = (struct mxt_data *)dev_id;
	if (data->pm_state >= PM_SUSPEND) {
		LOGN("interrupt in suspend[%d]\n",data->pm_state);
		data->pm_state = PM_SUSPEND_IRQ;
		pm_wakeup_event(&data->client->dev, msecs_to_jiffies(1000));
		return IRQ_HANDLED;
	}
	return IRQ_WAKE_THREAD;
}
static irqreturn_t mxt_interrupt(int irq, void *dev_id)
{
	struct mxt_data *data = (struct mxt_data*)dev_id;
	irqreturn_t ret = IRQ_NONE;

	if (data->in_bootloader) {
		/* bootloader state transition completion */
		complete(&data->bl_completion);
		return IRQ_HANDLED;
	}

	if (!data->object_table) {
		return IRQ_HANDLED;
	}

	if (data->mxt_drv_data->is_probing){
		return IRQ_HANDLED;
	}

	mutex_lock(&data->mxt_drv_data->dev_lock);
	if (!data->suspended) {
		if (data->T44_address)
			ret = mxt_process_messages_t44(data);
		else
			ret = mxt_process_messages(data);
	}
	mutex_unlock(&data->mxt_drv_data->dev_lock);
	return ret;
}

static int mxt_t6_command(struct mxt_data *data, u16 cmd_offset, u8 value, bool wait)
{
	u16 reg = 0;
	u8 command_register = 0;
	int timeout_counter = 0;
	int ret = 0;

	reg = data->T6_address + cmd_offset;

	ret = mxt_write_reg(data->client, reg, value);
	if (ret)
		return ret;

	if (!wait)
		return 0;

	do {
		msleep(20);
		ret = __mxt_read_reg(data->client, reg, 1, &command_register);
		if (ret)
			return ret;
	} while ((command_register != 0) && (timeout_counter++ <= 100));

	if (timeout_counter > 100) {
		LOGE("%s Command failed!\n", __func__);
		return -EIO;
	}

	return 0;
}

static int mxt_t109_command(struct mxt_data *data, u16 cmd_offset, u8 value)
{
	u16 reg = 0;
	int ret = 0;

	reg = data->T109_address + cmd_offset;

	ret = mxt_write_reg(data->client, reg, value);
	if (ret)
		return ret;

	return 0;
}

static int mxt_t25_command(struct mxt_data *data, u8 value, bool wait)
{
	u16 reg = 0;
	u8 command_register = 0;
	int timeout_counter = 0;
	int ret = 0;

	if (!data->mxt_drv_data->selftest_enable)
		return 0;

	reg = data->T25_address + 1 ;

	ret = mxt_write_reg(data->client, reg, value);
	if (ret) {
		LOGE("Write Self Test Command fail!\n");
		return ret;
	}

	if (!wait)
		return 0;

	do {
		msleep(20);
		ret = __mxt_read_reg(data->client, reg, 1, &command_register);
		if (ret)
			return ret;
	} while ((command_register != 0) && (timeout_counter++ <= 100));

	if (timeout_counter > 100) {
		LOGE("%s Command failed!\n", __func__);
		return -EIO;
	}

	return 0;
}

static int mxt_soft_reset(struct mxt_data *data)
{
	int ret = 0;

	LOGN("%s\n", __func__);

	init_completion(&data->reset_completion);

	ret = mxt_t6_command(data, MXT_COMMAND_RESET, MXT_RESET_VALUE, false);
	if (ret)
		return ret;

	msleep(MXT_RESET_TIME);

	return 0;
}

static int mxt_hw_reset(struct mxt_data *data)
{
	LOGN("%s\n", __func__);

	gpio_set_value(data->pdata->gpio_reset, 0);
	mdelay(5);

	data->t6_status_reset_irq = 0;

	gpio_set_value(data->pdata->gpio_reset, 1);
	msleep(MXT_RESET_TIME);

	return 0;
}
static void mxt_update_crc(struct mxt_data *data, u8 cmd, u8 value)
{
	/* on failure, CRC is set to 0 and config will always be downloaded */
	data->config_crc = 0;

	mxt_t6_command(data, cmd, value, true);
	/* Wait for crc message. On failure, CRC is set to 0 and config will
	 * always be downloaded */

	msleep(MXT_CRC_TIMEOUT);
}

static void mxt_calc_crc24(u32 *crc, u8 firstbyte, u8 secondbyte)
{
	static const unsigned int crcpoly = 0x80001B;
	u32 result = 0;
	u32 data_word = 0;

	data_word = (secondbyte << 8) | firstbyte;
	result = ((*crc << 1) ^ data_word);

	if (result & 0x1000000)
		result ^= crcpoly;

	*crc = result;
}

static u32 mxt_calculate_crc(u8 *base, off_t start_off, off_t end_off)
{
	u32 crc = 0;
	u8 *ptr = base + start_off;
	u8 *last_val = base + end_off - 1;

	if (end_off < start_off)
		return -EINVAL;

	while (ptr < last_val) {
		mxt_calc_crc24(&crc, *ptr, *(ptr + 1));
		ptr += 2;
	}

	/* if len is odd, fill the last byte with 0 */
	if (ptr == last_val)
		mxt_calc_crc24(&crc, *ptr, 0);

	/* Mask to 24-bit */
	crc &= 0x00FFFFFF;

	return crc;
}

static int mxt_set_t7_power_cfg(struct mxt_data *data, u8 sleep)
{
	int error = 0;
	struct t7_config *new_config = NULL;
	struct t7_config deepsleep = { .active = 0, .idle = 0 };
	u16 tmp;

	tmp = (u16)sizeof(data->t7_cfg);

	if (sleep == MXT_POWER_CFG_DEEPSLEEP)
		new_config = &deepsleep;
	else
		new_config = &data->t7_cfg;

	error = __mxt_write_reg(data->client, data->T7_address, tmp, new_config);
	if (error)
		return error;

	LOGD("Set T7 ACTV:%d IDLE:%d\n", new_config->active, new_config->idle);

	return 0;
}

static int mxt_init_t7_power_cfg(struct mxt_data *data)
{
	int error = 0;
	bool retry = false;
	u16 tmp;

	tmp = (u16)sizeof(data->t7_cfg);
recheck:
	error = __mxt_read_reg(data->client, data->T7_address, tmp, &data->t7_cfg);
	if (error)
		return error;

	if (data->t7_cfg.active == 0 || data->t7_cfg.idle == 0) {
		if (!retry) {
			LOGD("T7 cfg zero, resetting\n");
			mxt_soft_reset(data);
			retry = true;
			goto recheck;
		} else {
			LOGD("T7 cfg zero after reset, overriding\n");
			data->t7_cfg.active = 20;
			data->t7_cfg.idle = 100;
			return mxt_set_t7_power_cfg(data, MXT_POWER_CFG_RUN);
		}
	} else {
		LOGE("Initialised power cfg: ACTV %d, IDLE %d\n", data->t7_cfg.active, data->t7_cfg.idle);
		return 0;
	}
}

static int mxt_init_t93_tab_count(struct mxt_data *data)
{
	struct mxt_object *object = NULL;

	LOGN("%s\n", __func__);

	object = mxt_get_object(data, MXT_PROCI_TOUCH_SEQUENCE_LOGGER_T93);
	if (object) {
		__mxt_read_reg(data->client, object->start_address + 17, 1, &(data->g_tap_cnt));
		LOGN("%s data->g_tap_cnt : %d\n", __func__, data->g_tap_cnt);
		return 0;
	} else {
		data->g_tap_cnt = 0;
		return 1;
	}
}

static int mxt_check_reg_init(struct mxt_data *data, const char *name)
{
	struct device *dev = &data->client->dev;
	struct mxt_info cfg_info = {0};
	struct mxt_object *object = NULL;
	const struct firmware *cfg = NULL;
	int ret = 0;
	int offset = 0;
	int data_pos = 0;
	int byte_offset = 0;
	int i = 0;
	int cfg_start_ofs = 0;
	u32 info_crc = 0, config_crc = 0, calculated_crc = 0;
	u8 *config_mem = 0;
	u32 config_mem_size = 0;
	unsigned int type = 0, instance, size = 0;
	u8 val = 0;
	u16 reg = 0;

	LOGD("%s\n", __func__);

	if (!name) {
		LOGE("Skipping cfg download\n");
		return 0;
	}

	ret = request_firmware(&cfg, name, dev);
	if (ret < 0) {
		LOGE("Failure to request config file [%s]\n", name);
		return -EINVAL;
	}

	LOGD("Open [%s] configuration file\n", name);

	mxt_update_crc(data, MXT_COMMAND_REPORTALL, 1);

	if (strncmp(cfg->data, MXT_CFG_MAGIC, strlen(MXT_CFG_MAGIC))) {
		LOGE("Unrecognised config file\n");
		ret = -EINVAL;
		goto release;
	}

	data_pos = strlen(MXT_CFG_MAGIC);

	/* Load information block and check */
	for (i = 0; i < sizeof(struct mxt_info); i++) {
		ret = sscanf(cfg->data + data_pos, "%hhx%n", (unsigned char *)&cfg_info + i, &offset);
		if (ret != 1) {
			LOGE("Bad format\n");
			ret = -EINVAL;
			goto release;
		}

		data_pos += offset;
	}

	/* Read CRCs */
	ret = sscanf(cfg->data + data_pos, "%x%n", &info_crc, &offset);
	if (ret != 1) {
		LOGE("Bad format: failed to parse Info CRC\n");
		ret = -EINVAL;
		goto release;
	}
	data_pos += offset;

	ret = sscanf(cfg->data + data_pos, "%x%n", &config_crc, &offset);
	if (ret != 1) {
		LOGE("Bad format: failed to parse Config CRC\n");
		ret = -EINVAL;
		goto release;
	}
	data_pos += offset;

	LOGD("RAW Config CRC is 0x%06X\n", config_crc);

	if (data->config_crc == config_crc) {
		LOGN("Config already applied\n");
		ret = 0;
		goto release_mem;
	}

	if (memcmp((char *)data->info, (char *)&cfg_info, sizeof(struct mxt_info)) != 0) {
		LOGD("Compatibility Error. Could not apply\n");
			LOGD("Info Block [IC]   %02X %02X %02X %02X %02X %02X %02X\n",
			data->info->family_id, data->info->variant_id, data->info->version, data->info->build,
			data->info->matrix_xsize, data->info->matrix_ysize, data->info->object_num);

		LOGD("Info Block [File] %02X %02X %02X %02X %02X %02X %02X\n",
			cfg_info.family_id, cfg_info.variant_id, cfg_info.version, cfg_info.build,
			cfg_info.matrix_xsize, cfg_info.matrix_ysize, cfg_info.object_num);

		ret = -EINVAL;
		goto release_mem;
	}

	/* Malloc memory to store configuration */
	cfg_start_ofs = MXT_OBJECT_START + data->info->object_num * sizeof(struct mxt_object) + MXT_INFO_CHECKSUM_SIZE;
	config_mem_size = data->mem_size - cfg_start_ofs;
	config_mem = kzalloc(config_mem_size, GFP_KERNEL);
	if (!config_mem) {
		LOGE("Failed to allocate memory\n");
		ret = -ENOMEM;
		goto release;
	}

	while (data_pos < cfg->size) {
		/* Read type, instance, length */
		ret = sscanf(cfg->data + data_pos, "%x %x %x%n", &type, &instance, &size, &offset);
		if (ret == 0) {
			/* EOF */
			break;
		} else if (ret != 3) {
			LOGE("Bad format: failed to parse object\n");
			ret = -EINVAL;
			goto release_mem;
		}
		data_pos += offset;

		object = mxt_get_object(data, type);
		if (!object) {
			/* Skip object */
			for (i = 0; i < size; i++) {
				ret = sscanf(cfg->data + data_pos, "%hhx%n", &val, &offset);
				data_pos += offset;
			}
			continue;
		}

		if (instance >= mxt_obj_instances(object)) {
			LOGE("Object instances exceeded!\n");
			ret = -EINVAL;
			goto release_mem;
		}

		reg = object->start_address + mxt_obj_size(object) * instance;

		LOGN("\t %04X %04X %04X start addr %04X\n", type, instance, size, object->start_address);

		if (size != mxt_obj_size(object)) {
			LOGE("Size mismatched\n");
			ret = -EINVAL;
			goto release_mem;
		}

		for (i = 0; i < size; i++) {
			ret = sscanf(cfg->data + data_pos, "%hhx%n", &val, &offset);
			if (ret != 1) {
				LOGE("Bad format in T%d\n", type);
				ret = -EINVAL;
				goto release_mem;
			}

			if (i > mxt_obj_size(object))
				continue;

			byte_offset = reg + i - cfg_start_ofs;

			if ((byte_offset >= 0) && (byte_offset <= config_mem_size)) {
				*(config_mem + byte_offset) = val;
			} else {
				LOGE("Bad object: reg:%d, T%d, ofs=%d\n", reg, object->type, byte_offset);
				ret = -EINVAL;
				goto release_mem;
			}

			data_pos += offset;
		}
	}

	/* calculate crc of the received configs (not the raw config file) */
	if (data->T7_address < cfg_start_ofs) {
		LOGE("Bad T7 address, T7addr = %x, config offset %x\n", data->T7_address, cfg_start_ofs);
		ret = -EINVAL;
		goto release_mem;
	}

	calculated_crc = mxt_calculate_crc(config_mem, data->T7_address - cfg_start_ofs, config_mem_size);

	/* Check the crc, calculated should match what is in file */
	if (config_crc > 0 && (config_crc != calculated_crc)) {
		LOGE("CRC mismatch in config file, calculated=0x%06X, file=0x%06X\n", calculated_crc, config_crc);
		LOGE("Config not apply\n");
		ret = -EINVAL;
		goto release_mem;
	}

	/* Write configuration as blocks */
	byte_offset = 0;
	while (byte_offset < config_mem_size) {
		size = config_mem_size - byte_offset;

		if (size > MXT_MAX_BLOCK_WRITE)
			size = MXT_MAX_BLOCK_WRITE;

		ret = __mxt_write_reg(data->client, cfg_start_ofs + byte_offset, size, config_mem + byte_offset);
		if (ret != 0) {
			LOGE("Config write error, ret=%d\n", ret);
			ret = -EINVAL;
			goto release_mem;
		}

		byte_offset += size;
	}

	mxt_update_crc(data, MXT_COMMAND_BACKUPNV, MXT_BACKUP_VALUE);

	if ((config_crc > 0) && (config_crc != data->config_crc)) {
		LOGE("Config CRC is mismatched 0x%06X\n", data->config_crc);
	}

	ret = mxt_soft_reset(data);
	if (ret)
		goto release_mem;

	LOGN("Config written\n");

	/* T7 config may have changed */
	mxt_init_t7_power_cfg(data);

release_mem:
	LOGN("kfree in %s\n", __func__);
	kfree(config_mem);
release:
	if (cfg)
		release_firmware(cfg);

	return ret;
}

static int mxt_acquire_irq(struct mxt_data *data)
{
	touch_enable_irq(data);

	return 0;
}

static void mxt_free_input_device(struct mxt_data *data)
{
	if (data->input_dev && data->mxt_drv_data->is_probing) {
		LOGN("mxt_free_input_device\n");
		input_unregister_device(data->input_dev);
		data->input_dev = NULL;
	}
}

static void mxt_free_object_table(struct mxt_data *data)
{
	LOGN("%s\n", __func__);

	if (data->raw_info_block)
		kfree(data->raw_info_block);

	data->info = NULL;
	data->raw_info_block = NULL;

	mxt_free_input_device(data);
	data->enable_reporting = false;

	data->T5_address = 0;
	data->T5_msg_size = 0;
	data->T6_reportid = 0;
	data->T7_address = 0;
	data->T8_address = 0;
	data->T9_address = 0;
	data->T9_reportid_min = 0;
	data->T9_reportid_max = 0;
	data->T15_reportid_min = 0;
	data->T15_reportid_max = 0;
	data->T18_address = 0;
	data->T24_reportid = 0;
	data->T35_reportid = 0;
	data->T25_reportid = 0;
	data->T42_reportid_min = 0;
	data->T42_reportid_max = 0;
	data->T42_address = 0;
	data->T44_address = 0;
	data->T46_address = 0;
	data->T48_reportid = 0;
	data->T56_address = 0;
	data->T65_address = 0;
	data->T72_address = 0;
	data->T80_address = 0;
	data->T80_reportid = 0;
	data->T93_address = 0;
	data->T93_reportid = 0;
	data->T100_address = 0;
	data->T100_reportid_min = 0;
	data->T100_reportid_max = 0;
	data->T109_address = 0;
	data->T109_reportid = 0;
	data->max_reportid = 0;
}

static int mxt_parse_object_table(struct mxt_data *data)
{
	int i = 0;
	u8 reportid = 0;
	u16 end_address = 0;
	struct mxt_object *object = NULL;
	u8 min_id = 0, max_id = 0;

	/* Valid Report IDs start counting from 1 */
	reportid = 1;
	data->mem_size = 0;

	LOGN("%s\n", __func__);

	for (i = 0; i < data->info->object_num; i++) {
		object = data->object_table + i;

		le16_to_cpus(&object->start_address);

		if (object->num_report_ids) {
			min_id = reportid;
			reportid += object->num_report_ids * mxt_obj_instances(object);
			max_id = reportid - 1;
		} else {
			min_id = 0;
			max_id = 0;
		}

		LOGD("\t T%02u Start:%u Size:%03u Instances:%u Report IDs:%u-%u\n",
			object->type, object->start_address,
			mxt_obj_size(object), mxt_obj_instances(object),
			min_id, max_id);

		switch (object->type) {
		case MXT_GEN_MESSAGE_T5:
			if (data->info->family_id == 0x80) {
				/* On mXT224 read and discard unused CRC byte
				 * otherwise DMA reads are misaligned */
				data->T5_msg_size = mxt_obj_size(object);
			} else {
				/* CRC not enabled, so skip last byte */
				data->T5_msg_size = mxt_obj_size(object) - 1;
			}
			data->T5_address = object->start_address;
			break;
		case MXT_GEN_COMMAND_T6:
			data->T6_reportid = min_id;
			data->T6_address = object->start_address;
			break;
		case MXT_GEN_ACQUIRE_T8:
			data->T8_address = object->start_address;
			break;
		case MXT_GEN_POWER_T7:
			data->T7_address = object->start_address;
			break;
		case MXT_TOUCH_MULTI_T9:
			/* Only handle messages from first T9 instance */
			data->T9_address = object->start_address;
			data->T9_reportid_min = min_id;
			data->T9_reportid_max = min_id + object->num_report_ids - 1;
			data->num_touchids = object->num_report_ids;
			break;
		case MXT_TOUCH_KEYARRAY_T15:
			data->T15_reportid_min = min_id;
			data->T15_reportid_max = max_id;
			break;
		case MXT_SPT_COMMSCONFIG_T18:
			data->T18_address = object->start_address;
			break;
		case MXT_PROCI_ONETOUCH_T24:
			data->T24_reportid = min_id;
			break;
		case MXT_SPT_PROTOTYPE_T35:
			data->T35_reportid = min_id;
			break;
		case MXT_SPT_SELFTEST_T25:
			data->T25_reportid = min_id;
			data->T25_address = object->start_address;
			break;
		case MXT_PROCI_TOUCHSUPPRESSION_T42:
			data->T42_address = object->start_address;
			data->T42_reportid_min = min_id;
			data->T42_reportid_max = max_id;
			break;
		case MXT_SPT_MESSAGECOUNT_T44:
			data->T44_address = object->start_address;
			break;
		case MXT_SPT_CTECONFIG_T46:
			data->T46_address = object->start_address;
			break;
		case MXT_PROCI_STYLUS_T47:
			data->T47_address = object->start_address;
			break;
		case MXT_SPT_NOISESUPPRESSION_T48:
			data->T48_reportid = min_id;
			break;
		case MXT_PROCI_SHIELDLESS_T56:
			data->T56_address = object->start_address;
			break;
		case MXT_PROCI_LENSBENDING_T65:
			data->T65_address = object->start_address;
			break;
		case MXT_SPT_DYNAMICCONFIGURATIONCONTAINER_T71:
			data->T71_address = object->start_address;
			break;
		case MXT_PROCG_NOISESUPPRESSION_T72:
			data->T72_address = object->start_address;
			break;
		case MXT_RETRANSMISSIONCOMPENSATION_T80:
			data->T80_address = object->start_address;
			data->T80_reportid = min_id;
			break;
		case MXT_PROCI_TOUCH_SEQUENCE_LOGGER_T93:
			data->T93_reportid = min_id;
			data->T93_address = object->start_address;
			break;
		case MXT_TOUCH_MULTITOUCHSCREEN_T100:
			/* Only handle messages from first T100 instance */
			data->T100_address = object->start_address;
			data->T100_reportid_min = min_id;
			data->T100_reportid_max = min_id + object->num_report_ids - 1;
			data->num_touchids = object->num_report_ids - 2;
			LOGD("T100_reportid_min:%d T100_reportid_max:%d\n", data->T100_reportid_min, data->T100_reportid_max);
			break;
		case MXT_SPT_SELFCAPGLOBALCONFIG_T109:
			data->T109_address = object->start_address;
			data->T109_reportid = min_id;
			break;
		}

		end_address = object->start_address + mxt_obj_size(object) * mxt_obj_instances(object) - 1;

		if (end_address >= data->mem_size)
			data->mem_size = end_address + 1;
	}

	/* Store maximum reportid */
	data->max_reportid = reportid;

	if (data->msg_buf) {
		kfree(data->msg_buf);
	}

	data->msg_buf = kzalloc((data->max_reportid * data->T5_msg_size), GFP_KERNEL);
	if (!data->msg_buf) {
		LOGE("%s d Failed to allocate message buffer\n", __func__);
		return -ENOMEM;
	}

	return 0;
}

static int mxt_read_info_block(struct mxt_data *data)
{
	struct i2c_client *client = data->client;
	int error = 0;
	size_t size = 0;
	void *buf = NULL;
	struct mxt_info *info = NULL;
	u16 tmp = 0;

	LOGN("%s\n", __func__);

	/* Read 7-byte ID information block starting at address 0 */
	size = sizeof(struct mxt_info);
	buf = kzalloc(size, GFP_KERNEL);
	if (!buf) {
		LOGE("%s Failed to allocate memory 1\n", __func__);
		return -ENOMEM;
	}

	tmp = (u16)size;
	error = __mxt_read_reg(client, 0, tmp, buf);
	if (error) {
		LOGE("%s __mxt_read_reg error\n", __func__);
		goto err_free_mem;
	}

	/* Resize buffer to give space for rest of info block */
	info = (struct mxt_info *)buf;
	size += (MXT_OBJECT_NUM_MAX * sizeof(struct mxt_object)) + MXT_INFO_CHECKSUM_SIZE;
	buf = krealloc(buf, size, GFP_KERNEL);
	if (!buf) {
		LOGE("%s Failed to allocate memory 2\n", __func__);
		error = -ENOMEM;
		goto err_free_mem;
	}

	/* Read rest of info block */
	error = __mxt_read_reg(client, MXT_OBJECT_START,
			       tmp - MXT_OBJECT_START, buf + MXT_OBJECT_START);
	if (error) {
		LOGE("%s __mxt_read_reg error\n", __func__);
		goto err_free_mem;
	}

	/* Save pointers in device data structure */
	data->raw_info_block = buf;
	data->info = (struct mxt_info *)buf;

	if (data->object_table == NULL) {
		data->object_table = (struct mxt_object *)(buf + MXT_OBJECT_START);
	}

	LOGN("Family:%02X Variant:%02X Binary:%u.%u.%02X TX:%d RX:%d Objects:%d\n",
		 data->info->family_id, data->info->variant_id, data->info->version >> 4, data->info->version & 0xF,
		 data->info->build, data->info->matrix_xsize, data->info->matrix_ysize, data->info->object_num);

	/* Parse object table information */
	error = mxt_parse_object_table(data);
	if (error) {
		LOGE("%s Error %d reading object table\n", __func__, error);
		mxt_free_object_table(data);
		return error;
	}

	return 0;

err_free_mem:
	kfree(buf);
	data->raw_info_block = NULL;
	data->info = NULL;
	if (data->object_table)
		kfree(data->object_table);
	data->object_table = NULL;
	return error;
}

static int mxt_read_t9_resolution(struct mxt_data *data)
{
	struct i2c_client *client = data->client;
	int error = 0;
	struct t9_range range = {0};
	unsigned char orient = 0;
	struct mxt_object *object = NULL;
	u16 tmp = 0;

	memset(&range, 0, sizeof(range));

	object = mxt_get_object(data, MXT_TOUCH_MULTI_T9);
	if (!object)
		return -EINVAL;

	tmp = (u16)sizeof(range);
	error = __mxt_read_reg(client, object->start_address + MXT_T9_RANGE, tmp, &range);
	if (error)
		return error;

	le16_to_cpus(range.x);
	le16_to_cpus(range.y);

	error =  __mxt_read_reg(client, object->start_address + MXT_T9_ORIENT, 1, &orient);
	if (error)
		return error;

	/* Handle default values */
	if (range.x == 0)
		range.x = 2159;

	if (range.y == 0)
		range.y = 3839;

	if (orient & MXT_T9_ORIENT_SWITCH) {
		data->max_x = range.y;
		data->max_y = range.x;
	} else {
		data->max_x = range.x;
		data->max_y = range.y;
	}

	LOGN("Touchscreen size X:%u Y:%u\n", data->max_x, data->max_y);

	return 0;
}

static void mxt_regulator_enable(struct mxt_data *data)
{
#if 0

	if (data->regulator_status == 1)
		return;

	LOGN("%s\n", __func__);

	/* Touch Reset low */
	if (gpio_is_valid(data->pdata->gpio_reset)) {
		gpio_set_value(data->pdata->gpio_reset, 0);
	}
	msleep(5);

	/* DISP_VPNL/Touch AVDD on */
	if (! IS_ERR_OR_NULL(data->pdata->avdd)) {
		int ret = regulator_enable(data->pdata->avdd);
		LOGN("ERROR CODE iS %d\n", ret);
	}
	msleep(5);

	/* avdd enable */
/*	if (gpio_is_valid(data->pdata->gpio_avdd)) {
		gpio_set_value(data->pdata->gpio_avdd, 1);
	}*/
	//msleep(5);

	/* dvdd enable
	if (gpio_is_valid(data->pdata->gpio_dvdd)) {
		gpio_set_value(data->pdata->gpio_dvdd, 1);
	}
	msleep(5);
	*/
	/* Touch TP_VDDL on */
	if (gpio_is_valid(data->pdata->gpio_vddio)) {
		gpio_set_value(data->pdata->gpio_vddio, 0);
	}
	data->regulator_status = 1;

	init_completion(&data->bl_completion);

	msleep(5);
	/* Touch Reset high */
	if (gpio_is_valid(data->pdata->gpio_reset)) {
		gpio_set_value(data->pdata->gpio_reset, 1);
	}

	msleep(MXT_RESET_TIME);
	//LOGD("%s() reset:%d, avdd:%d, vio:%d, int:%d\n",__func__, gpio_get_value(data->pdata->gpio_reset), gpio_get_value(data->pdata->gpio_avdd), gpio_get_value(data->pdata->gpio_vio), gpio_get_value(data->pdata->gpio_int));
#else
	if (data->regulator_status == 1)
		return;

	LOGN("%s\n", __func__);

	data->regulator_status = 1;

	init_completion(&data->bl_completion);
#endif
}

static void mxt_regulator_disable(struct mxt_data *data)
{
#if 0
	//int error = 0;

	if (data->regulator_status == 0)
		return;

	/* Touch Reset low */
	if (gpio_is_valid(data->pdata->gpio_reset)) {
		gpio_set_value(data->pdata->gpio_reset, 0);
	}

	/* avdd disable */
/*	if (gpio_is_valid(data->pdata->gpio_avdd)) {
		gpio_set_value(data->pdata->gpio_avdd, 0);
	}*/

	/* dvdd disable
	if (gpio_is_valid(data->pdata->gpio_dvdd)) {
		gpio_set_value(data->pdata->gpio_dvdd, 0);
	}
	*/
	/* Touch VDDIO off */
	if (gpio_is_valid(data->pdata->gpio_vddio))
		gpio_set_value(data->pdata->gpio_vddio, 1);



	LOGN("%s\n", __func__);

	data->regulator_status = 0;
#else
	if (data->regulator_status == 0)
		return;

	LOGN("%s\n", __func__);

	data->regulator_status = 0;
#endif
}

#if 0
static void mxt_probe_regulators(struct mxt_data *data)
{
	struct device *dev = &data->client->dev;
	int error = 0;

	/* According to maXTouch power sequencing specification, RESET line
	 * must be kept low until some time after regulators come up to
	 * voltage */
	LOGN("%s start\n", __func__);

	data->regulator_status = 0;
	data->vcc_dig = NULL;

	if (!data->pdata->gpio_reset) {
		LOGN("Must have reset GPIO to use regulator support\n");
		goto fail;
	} else {
		LOGD("%s() gpio_reset:%ld\n", __func__, data->pdata->gpio_reset);
	}
	data->vcc_dig = regulator_get(dev, "vcc_dig");
	if (IS_ERR(data->vcc_dig)) {
		error = PTR_ERR(data->vcc_dig);
		LOGE("Error %d getting ana regulator\n", error);
		goto fail;
	}

	error = regulator_set_voltage(data->vcc_dig, 3300000, 3300000);
	if (error < 0) {
		LOGE("Error %d cannot control DVDD regulator\n", error);
		goto fail;
	}

	data->use_regulator = true;
	LOGN("%s end\n", __func__);
	return;

fail:
	LOGE("%s fail\n", __func__);
	data->vcc_dig = NULL;
	data->use_regulator = false;
}
#endif

static int mxt_configure_objects(struct mxt_data *data)
{
	int error = 0;

	LOGN("%s\n", __func__);

	error = mxt_init_t7_power_cfg(data);
	if (error) {
		LOGN("Failed to initialize power cfg\n");
		return error;
	}

	/* Check register init values */
	error = mxt_check_reg_init(data, NULL);
	if (error) {
		LOGE("Error %d initialising configuration\n", error);
		return error;
	}

	if (data->T9_reportid_min) {
		error = mxt_initialize_t9_input_device(data);
		if (error)
			return error;
	} else {
		LOGE("No touch object detected\n");
	}
	return 0;
}

static int mxt_rest_init(struct mxt_data *data)
{
	int error = 0;

	error = mxt_acquire_irq(data);
	if (error)
		return error;

	error = mxt_configure_objects(data);
	if (error)
		return error;

	return 0;
}

static void mxt_read_fw_version(struct mxt_data *data)
{
	LOGN("==================================\n");
	LOGN("Firmware Version = %d.%02d.%02d\n", data->pdata->fw_ver[0], data->pdata->fw_ver[1], data->pdata->fw_ver[2]);
	LOGN("FW Product       = %s\n", data->pdata->product);
	LOGN("Binary Version   = %u.%u.%02X\n", data->info->version >> 4, data->info->version & 0xF, data->info->build);
	LOGN("Config CRC       = 0x%X\n", data->config_crc);
	LOGN("Family Id        = 0x%02X\n", data->info->family_id);
	LOGN("Variant          = 0x%02X\n", data->info->variant_id);
	LOGN("Panel Type       = 0x%02X\n", data->panel_type);
	LOGN("==================================\n");
}

/* Firmware Version is returned as Major.Minor.Build */
static ssize_t mxt_fw_version_show(struct mxt_data *data, char *buf)
{
	return (ssize_t)scnprintf(buf, PAGE_SIZE, "%u.%u.%02X\n",
			 data->info->version >> 4, data->info->version & 0xf, data->info->build);
}

/* Hardware Version is returned as FamilyID.VariantID */
static ssize_t mxt_hw_version_show(struct mxt_data *data, char *buf)
{
	return (ssize_t)scnprintf(buf, PAGE_SIZE, "%02X.%02X\n",
			data->info->family_id, data->info->variant_id);
}

static ssize_t mxt_testmode_ver_show(struct mxt_data *data, char *buf)
{
	ssize_t ret = 0;

	ret += sprintf(buf+ret, "%d.%02d.%02d (panel_type:0x%02X)", data->pdata->fw_ver[0], data->pdata->fw_ver[1], data->pdata->fw_ver[2], data->panel_type);

	return ret;
}

static ssize_t mxt_info_show(struct mxt_data *data, char *buf)
{
	ssize_t ret = 0;

	mxt_read_fw_version(data);

	ret += sprintf(buf+ret, "FW Product       = %s\n", data->pdata->product);
	ret += sprintf(buf+ret, "Binary Version   = %u.%u.%02X\n", data->info->version >> 4, data->info->version & 0xF, data->info->build);
	ret += sprintf(buf+ret, "Config CRC       = 0x%X\n", data->config_crc);
	ret += sprintf(buf+ret, "Family Id        = 0x%02X\n", data->info->family_id);
	ret += sprintf(buf+ret, "Variant          = 0x%02X\n", data->info->variant_id);
	ret += sprintf(buf+ret, "Patch Date       = %d\n", data->patch.date);
	ret += sprintf(buf+ret, "Panel type       = 0x%02X\n", data->panel_type);
	ret += sprintf(buf+ret, "[Firmware Version = %d.%02d.%02d]\n", data->pdata->fw_ver[0], data->pdata->fw_ver[1], data->pdata->fw_ver[2]);
	return ret;
}

static ssize_t mxt_selftest(struct mxt_data *data, char *buf, int len, int mode)
{
	int ret = (int)len;
	int test_cmd;

	if (mode)
		test_cmd = 0xFE;
	else
		test_cmd = 0x17;

	data->mxt_drv_data->selftest_enable = true;
	data->mxt_drv_data->selftest_show = true;

	mxt_t25_command(data, test_cmd, false);
	msleep(MXT_SELFTEST_TIME);

	if (mode) {
		if (data->self_test_status[0] == 0) {
			ret += snprintf(buf + ret, PAGE_SIZE - ret,  "Need more time. Try Again.\n");
			return ret;
		}

		if (data->self_test_status[0] == 0xFD) {
			ret += snprintf(buf + ret, PAGE_SIZE - ret, "Invalid Test Code. Try Again.");
		} else if (data->self_test_status[0] == 0xFC) {
			ret += snprintf(buf + ret, PAGE_SIZE - ret, "The test could not be completed due to an unrelated fault. Try again.");
		} else {
			ret += snprintf(buf + ret, PAGE_SIZE - ret, "All Test Result: %s", (data->self_test_status[0] == 0xFE) ? "Pass\n" : "Fail\n");
			ret += snprintf(buf + ret, PAGE_SIZE - ret, "AVdd power Test Result: %s", (data->self_test_status[0] != 0x01) ? "Pass\n" : "Fail\n");

			ret += snprintf(buf + ret, PAGE_SIZE - ret, "Pin Falut Test Result: %s", (data->self_test_status[0] != 0x12) ? "Pass\n" : "Fail\n");
			if (data->self_test_status[0] == 0x12)
				ret += snprintf(buf+ret, PAGE_SIZE - ret, "# Fail # seq_num(%u) x_pin(%u) y_pin(%u)\n",
						data->self_test_status[1], data->self_test_status[2], data->self_test_status[3]);

			ret += snprintf(buf + ret, PAGE_SIZE - ret, "Signal Limit Test: %s", (data->self_test_status[0] != 0x17) ? "Pass\n" : "Fail\n");
			if (data->self_test_status[0] == 0x17)
				ret += snprintf(buf+ret, PAGE_SIZE - ret, "# Fail # type_num(%u) type_instance(%u)\n", data->self_test_status[1], data->self_test_status[2]);
		}
	} else {
		ret += snprintf(buf + ret, PAGE_SIZE - ret, "LPWG Signal Limit Test: %s", (data->self_test_status[0] != 0x17) ? "Pass\n" : "Fail\n");
		if (data->self_test_status[0] == 0x17)
			ret += snprintf(buf+ret, PAGE_SIZE - ret, "# Fail # type_num(%u) type_instance(%u)\n", data->self_test_status[1], data->self_test_status[2]);
	}

	data->mxt_drv_data->selftest_show = false;
	return ret;
}

static int mxt_show_instance(char *buf, int count, struct mxt_object *object, int instance, const u8 *val)
{
	int i = 0;

	if (mxt_obj_instances(object) > 1)
		count += scnprintf(buf + count, PAGE_SIZE - count, "Instance %u\n", instance);

	for (i = 0; i < mxt_obj_size(object); i++)
		count += scnprintf(buf + count, PAGE_SIZE - count, "\t[%2u]: %02x (%d)\n", i, val[i], val[i]);

	count += scnprintf(buf + count, PAGE_SIZE - count, "\n");

	return count;
}

static ssize_t mxt_object_show(struct mxt_data *data, char *buf)
{
	struct mxt_object *object = NULL;
	size_t count = 0;
	int i = 0, j = 0;
	int error = 0;
	u8 *obuf = NULL;
	u16 size = 0;
	u16 addr = 0;

	/* Pre-allocate buffer large enough to hold max sized object. */
	obuf = kzalloc(256, GFP_KERNEL);
	if (!obuf)
		return -ENOMEM;

	error = 0;
	for (i = 0; i < data->info->object_num; i++) {
		object = data->object_table + i;

		if (!mxt_object_readable(object->type))
			continue;

		count += scnprintf(buf + count, PAGE_SIZE - count, "T%u:\n", object->type);

		for (j = 0; j < mxt_obj_instances(object); j++) {
			size = mxt_obj_size(object);
			addr = object->start_address + j * size;

			error = __mxt_read_reg(data->client, addr, size, obuf);
			if (error)
				goto done;

			count = mxt_show_instance(buf, count, object, j, obuf);
		}
	}

done:
	kfree(obuf);

	return error ?: count;
}

static ssize_t mxt_object_control(struct mxt_data *data, const char *buf, size_t count)
{
	struct mxt_object *object = NULL;
	unsigned char command[6] = {0};
	int type = 0;
	int addr_offset = 0;
	int value = 0;
	int error = 0;
	int i = 0, j = 0;
	u8 *obuf = NULL;
	u16 size = 0;
	u16 addr = 0;

	sscanf(buf, "%s %d %d %d", command, &type, &addr_offset, &value);

	if (!strncmp(command, "mode", 4)) { /*mode*/
		LOGN("Mode changed MODE: %d\n", type);
		data->mxt_mode_changed = type;
		if (data->mxt_mode_changed)
			mxt_write_object(data, MXT_PROCG_NOISESUPPRESSION_T72, 0, 1);
		else
			mxt_write_object(data, MXT_PROCG_NOISESUPPRESSION_T72, 0, 11);
		return count;
	}

	obuf = kzalloc(256, GFP_KERNEL);
	if (!obuf)
		return -ENOMEM;

	if (type == 25)
		data->mxt_drv_data->selftest_enable = true;

	object = mxt_get_object(data, type);
	if (!object) {
		LOGE("error Cannot get object_type T%d\n", type);
		if (obuf)
			kfree(obuf);
		return -EINVAL;
	}

	if ((mxt_obj_size(object) == 0) || (object->start_address == 0)) {
		LOGE("error object_type T%d\n", type);
		if (obuf)
			kfree(obuf);
		return -ENODEV;
	}

	if (!strncmp(command, "read", 4)) {	/*read*/
		LOGD("Object Read T%d: start_addr=%d, size=%d * instance=%d\n",
		type, object->start_address, mxt_obj_size(object), mxt_obj_instances(object));

		for (j = 0; j < mxt_obj_instances(object); j++) {
			size = mxt_obj_size(object);
			addr = object->start_address + j * size;

			error = __mxt_read_reg(data->client, addr, size, obuf);
			if (error)
				LOGN("Object Read Fail\n");
		}

		for (i = 0; i < mxt_obj_size(object)*mxt_obj_instances(object); i++)
			LOGD("T%d [%d] %d[0x%x]\n", type, i, obuf[i], obuf[i]);

	} else if (!strncmp(command, "write", 5)) {	/*write*/
		LOGD("Object Write T%d: start_addr=%d, size=%d * instance=%d\n",
			type, object->start_address, mxt_obj_size(object), mxt_obj_instances(object));

		error = mxt_write_reg(data->client, object->start_address+addr_offset, value);
		if (error)
			LOGN("Object Write Fail\n");

		LOGN("Object Write Success. Execute Read Object and Check Value.\n");
	} else {
		LOGE("Command Fail. Usage: echo [read | write] object cmd_field value > object_ctrl\n");
	}

	if (obuf)
		kfree(obuf);

	return count;
}

static int mxt_check_firmware_format(struct device *dev, const struct firmware *fw)
{
	unsigned int pos = 0;
	char c = 0;

	while (pos < fw->size) {
		c = *(fw->data + pos);

		if (c < '0' || (c > '9' && c < 'A') || c > 'F')
			return 0;

		pos++;
	}

	/* To convert file try
	 * xxd -r -p mXTXXX__APP_VX-X-XX.enc > maxtouch.fw */
	LOGN("Aborting: firmware file must be in binary format\n");

	return -1;
}

static int mxt_load_bin(struct device *dev, const char *name)
{
	struct mxt_data *data = dev_get_drvdata(dev);
	const struct firmware *fw = NULL;
	unsigned int frame_size = 0;
	unsigned int pos = 0;
	unsigned int retry = 0;
	unsigned int frame = 0;
	int ret = 0;

	if (!name) {
		LOGN("Skipping bin download\n");
		return 0;
	}

	ret = request_firmware(&fw, name, dev);
	if (ret) {
		LOGE("Unable to open bin [%s]  ret %d\n",name, ret);
		return 1;
	} else {
		LOGD("Open bin [%s]\n", name);
	}

	/* Check for incorrect enc file */
	ret = mxt_check_firmware_format(dev, fw);
	if (ret)
		goto release_firmware;

	if (data->suspended) {
		if (data->use_regulator)
			mxt_regulator_enable(data);

		touch_enable_irq(data);
		data->suspended = false;
	}

	if (!data->in_bootloader) {
		/* Change to the bootloader mode */
		data->in_bootloader = true;

		ret = mxt_t6_command(data, MXT_COMMAND_RESET, MXT_BOOT_VALUE, false);
		if (ret)
			goto release_firmware;

		msleep(MXT_RESET_TIME);

		/* At this stage, do not need to scan since we know
		 * family ID */
		ret = mxt_lookup_bootloader_address(data, 0);
		if (ret)
			goto release_firmware;
	}

	mxt_free_object_table(data);
	init_completion(&data->bl_completion);

	ret = mxt_check_bootloader(data, MXT_WAITING_BOOTLOAD_CMD);
	if (ret) {
		/* Bootloader may still be unlocked from previous update
		 * attempt */
		ret = mxt_check_bootloader(data, MXT_WAITING_FRAME_DATA);
		if (ret)
			goto disable_irq;
	} else {
		LOGN("Unlocking bootloader\n");

		/* Unlock bootloader */
		ret = mxt_send_bootloader_cmd(data, true);
		if (ret)
			goto disable_irq;
	}

	while (pos < fw->size) {
		ret = mxt_check_bootloader(data, MXT_WAITING_FRAME_DATA);
		if (ret)
			goto disable_irq;

		frame_size = ((*(fw->data + pos) << 8) | *(fw->data + pos + 1));

		/* Take account of CRC bytes */
		frame_size += 2;

		/* Write one frame to device */
		ret = mxt_bootloader_write(data, fw->data + pos, frame_size);
		if (ret)
			goto disable_irq;

		ret = mxt_check_bootloader(data, MXT_FRAME_CRC_PASS);
		if (ret) {
			retry++;

			/* Back off by 20ms per retry */
			msleep(retry * 20);

			if (retry > 20) {
				LOGE("Retry count exceeded\n");
				goto disable_irq;
			}
		} else {
			retry = 0;
			pos += frame_size;
			frame++;
		}

		if (frame % 50 == 0)
			LOGD("Sent %d frames, %d/%zd bytes\n",
				 frame, pos, fw->size);
	}

	/* Wait for flash */
	ret = mxt_wait_for_completion(data, &data->bl_completion,
				      MXT_FW_RESET_TIME);
	if (ret)
		goto disable_irq;
	LOGD("Sent %d frames, %d bytes\n", frame, pos);

	data->in_bootloader = false;

disable_irq:
	touch_disable_irq(data);
release_firmware:
	if (fw)
		release_firmware(fw);
	return ret;
}

static ssize_t mxt_update_bin_store(struct mxt_data *data, const char *buf, size_t count)
{
	int error = 0;
	char *name = NULL;

	error = mxt_update_file_name(&data->client->dev, &name, buf, count);
	if (error)
		return error;

	error = mxt_load_bin(&data->client->dev, name);
	if (error) {
		LOGE("The bin update failed(%d)\n", error);
		count = error;
	} else {
		LOGD("The bin update succeeded\n");

		data->suspended = false;

		mxt_hw_reset(data);

		error = mxt_read_info_block(data);
		if (error)
			return error;

		error = mxt_rest_init(data);
		if (error)
			return error;

		LOGD("Need to update proper Configuration(RAW)\n");
	}
	return count;
}

static ssize_t mxt_update_raw_store(struct mxt_data *data, const char *buf, size_t count)
{
	int ret = 0;
	int value = 0;
	char *name = NULL;

	sscanf(buf, "%d", &value);
	LOGD("Update mxt Configuration.\n");

	if (data->in_bootloader) {
		LOGE("Not in appmode\n");
		return -EINVAL;
	}

	ret = mxt_update_file_name(&data->client->dev, &name, buf, count);
	if (ret)
		return ret;

	data->enable_reporting = false;

	ret = mxt_check_reg_init(data, name);
	if (ret < 0) {
		LOGE("Error mxt_check_reg_init ret=%d\n", ret);
		goto out;
	}

	LOGD("Update mxt Configuration Success.\n");

out:
	data->enable_reporting = true;

	return count;
}

static ssize_t mxt_update_fw_store(struct mxt_data *data, const char *buf, size_t count)
{
	char *package_name = NULL;
	int error = 0;
	int wait_cnt = 0;
	struct mxt_data *mxt_data = data->mxt_drv_data->mxt_data;
	const struct firmware *fw = NULL;

	LOGD("%s\n", __func__);

	pm_wakeup_event(&data->client->dev, msecs_to_jiffies(2000));

	if (data->suspended) {
		LOGN("LCD On\n");
		kobject_uevent_env(&device_touch.kobj, KOBJ_CHANGE, knockon_event);
		while (1) {
			if (data->suspended) {
				mdelay(100);
				wait_cnt++;
			}

			if (!data->suspended || wait_cnt > 50)
				break;
		}
	}

	LOGN("wait_cnt = %d\n", wait_cnt);

	touch_disable_irq(data);

	error = mxt_update_file_name(&data->client->dev, &package_name, buf, count);
	if (error) {
		LOGE("%s error package_name [%s]\n", __func__, package_name);
		goto exit;
	}

	LOGN("%s [%s]\n", __func__, package_name);
	if (package_name) {
		error = request_firmware(&fw, package_name, &data->client->dev);
		if (error) {
			LOGE("%s error request_firmware %d\n", __func__, error);
			error = 1;
			goto exit;
		}
	}

	error = mxt_update_firmware(data, fw);
	if (error) {
		LOGE("%s error\n", __func__);
		goto exit;
	}

	if (mxt_data) {
		error = __mxt_read_reg(mxt_data->client,
			mxt_data->T71_address + 51, 26,
			&mxt_data->ref_limit);
		if (!error) {
			LOGD("Succeed to read reference limit u:%d x_all_err_chk:%d y_all_err_chk:%d only_x_err_cnt : %d only_y_err_cnt : %d max-min rng:%d diff_rng:%d err_cnt:%d err_weight:%d\n",
				mxt_data->ref_limit.ref_chk_using, mxt_data->ref_limit.ref_x_all_err_line,
				mxt_data->ref_limit.ref_y_all_err_line, mxt_data->ref_limit.xline_max_err_cnt,
				mxt_data->ref_limit.yline_max_err_cnt, mxt_data->ref_limit.ref_rng_limit,
				mxt_data->ref_limit.ref_diff_max, mxt_data->ref_limit.ref_err_cnt,
				mxt_data->ref_limit.err_weight);
			LOGD("Err Range y: %d %d %d %d %d %d %d %d %d %d %d %d %d %d, Button range:%d %d %d\n",
				mxt_data->ref_limit.y_line_dif[0], mxt_data->ref_limit.y_line_dif[1],
				mxt_data->ref_limit.y_line_dif[2], mxt_data->ref_limit.y_line_dif[3],
				mxt_data->ref_limit.y_line_dif[4], mxt_data->ref_limit.y_line_dif[5],
				mxt_data->ref_limit.y_line_dif[6], mxt_data->ref_limit.y_line_dif[7],
				mxt_data->ref_limit.y_line_dif[8], mxt_data->ref_limit.y_line_dif[9],
				mxt_data->ref_limit.y_line_dif[10], mxt_data->ref_limit.y_line_dif[11],
				mxt_data->ref_limit.y_line_dif[12], mxt_data->ref_limit.y_line_dif[13],
				mxt_data->ref_limit.butt_dif[0], mxt_data->ref_limit.butt_dif[1],
				mxt_data->ref_limit.butt_dif[2]);
		}
	}

exit:
	if (package_name) {
		kfree(package_name);
	}

	if (mxt_patchevent_get(data, PATCH_EVENT_TA)) {
		trigger_usb_state_from_otg(data, 1);
	}

	touch_enable_irq(data);

	mxt_read_fw_version(data);

	return count;
}

static ssize_t mxt_debug_enable_show(struct mxt_data *data, char *buf)
{
	int count = 0;
	char c = 0;

	c = data->debug_enabled ? '1' : '0';
	count = sprintf(buf, "%c\n", c);

	return count;
}

static ssize_t mxt_debug_enable_store(struct mxt_data *data, const char *buf, size_t count)
{
	int i = 0;

	if (sscanf(buf, "%u", &i) == 1 && i < 2) {
		data->debug_enabled = (i == 1);

		LOGN("%s\n", i ? "debug enabled" : "debug disabled");
		return count;
	} else {
		LOGE("debug_enabled write error\n");
		return -EINVAL;
	}
}

static ssize_t mxt_t57_debug_enable_store(struct mxt_data *data, const char *buf, size_t count)
{
	int i = 0;

	if (sscanf(buf, "%u", &i) == 1 && i < 2) {
		data->t57_debug_enabled = (i == 1);

		LOGD("%s\n", i ? "t57 debug enabled" : "t57 debug disabled");
		return count;
	} else {
		LOGE("t57_debug_enabled write error\n");
		return -EINVAL;
	}
}

static ssize_t mxt_patch_debug_enable_show(struct mxt_data *data, char *buf)
{
	int count = 0;
	char c = 0;

	if (data->patch.patch == NULL) {
		LOGE("patch not support\n");
		return count;
	}

	c = data->patch.debug ? '1' : '0';
	count = sprintf(buf, "%c\n", c);

	return count;
}

static ssize_t mxt_patch_debug_enable_store(struct mxt_data *data, const char *buf, size_t count)
{
	int i = 0;

	if (data->patch.patch == NULL) {
		LOGE("patch not support\n");
		return count;
	}

	if (sscanf(buf, "%u", &i) == 1 && i < 2) {
		data->patch.debug = (i == 1);

		LOGN("%s\n", i ? "patch debug enabled" : "patch debug disabled");
		return count;
	} else {
		LOGE("patch_debug_enabled write error\n");
		return -EINVAL;
	}
}

static ssize_t mxt_power_control_show(struct mxt_data *data, char *buf)
{
	size_t ret = 0;

	ret += sprintf(buf+ret, "usage: echo [0|1|2|3] > power_control\n");
	ret += sprintf(buf+ret, "  0 : power off\n");
	ret += sprintf(buf+ret, "  1 : power on\n");
	ret += sprintf(buf+ret, "  2 : reset by I2C\n");
	ret += sprintf(buf+ret, "  3 : reset by reset_gpio\n");

	return ret;
}

static ssize_t mxt_power_control_store(struct mxt_data *data, const char *buf, size_t count)
{
	int cmd = 0;

	if (sscanf(buf, "%d", &cmd) != 1)
		return -EINVAL;
	switch (cmd) {
		case 0:
			mxt_regulator_disable(data);
			break;
		case 1:
			mxt_regulator_enable(data);
			mxt_t6_command(data, MXT_COMMAND_CALIBRATE, 1, false);
			break;
		case 2:
			mxt_soft_reset(data);
			break;
		case 3:
			mxt_hw_reset(data);
			break;
		default:
			LOGN("usage: echo [0|1|2|3] > power_control\n");
			LOGN("  0 : power off\n");
			LOGN("  1 : power on\n");
			LOGN("  2 : reset by I2C\n");
			LOGN("  3 : reset by reset_gpio\n");
			break;
	}
	return count;

}

static int mxt_check_mem_access_params(struct mxt_data *data, loff_t off, size_t *count)
{
	data->mem_size = 32768;

	if (off >= data->mem_size)
		return -EIO;

	if (off + *count > data->mem_size)
		*count = data->mem_size - off;

	if (*count > MXT_MAX_BLOCK_WRITE)
		*count = MXT_MAX_BLOCK_WRITE;

	return 0;
}

static ssize_t mxt_mem_access_read(struct file *filp, struct kobject *kobj,
	struct bin_attribute *bin_attr, char *buf, loff_t off, size_t count)
{
	struct device *dev = container_of(kobj, struct device, kobj);
	struct mxt_data *data = dev_get_drvdata(dev);
	int ret = 0;

	ret = mxt_check_mem_access_params(data, off, &count);
	if (ret < 0)
		return ret;

	if (count > 0)
		ret = __mxt_read_reg(data->client, off, (u16)count, buf);

	return ret == 0 ? count : (int)ret;
}

static ssize_t mxt_mem_access_write(struct file *filp, struct kobject *kobj,
	struct bin_attribute *bin_attr, char *buf, loff_t off, size_t count)
{
	struct device *dev = container_of(kobj, struct device, kobj);
	struct mxt_data *data = dev_get_drvdata(dev);
	int ret = 0;

	ret = mxt_check_mem_access_params(data, off, &count);
	if (ret < 0)
		return ret;

	if (count > 0)
		ret = __mxt_write_reg(data->client, off, (u16)count, buf);

	return ret == 0 ? count : 0;
}

static ssize_t mxt_get_knockon_type(struct mxt_data *data, char *buf)
{
	int ret = 0;

	ret += sprintf(buf+ret, "%d", data->pdata->knock_on_type);

	return ret;
}

static void mxt_set_side_key(struct mxt_data *data)
{
	if (!data->side_key.supported) {
		LOGN("side key is not supported\n");
		return;
	}

	if (data->glove_mode.status || data->wireless_charge) {
		LOGN("cannot change side key\n");
		return;
	}

	if (data->cover_mode.status) {
		switch (data->side_key.status) {
		case SIDE_KEY_ENABLE:
		case SIDE_KEY_ONLY_RIGHT:
			mxt_patch_event(data, PATCH_EVENT_CODE_SIDETOUCH_ONLY_RIGHT);
			LOGN("sidetouch_rightonly");
			break;
		case SIDE_KEY_ONLY_LEFT:
		case SIDE_KEY_DISABLE:
			mxt_patch_event(data, PATCH_EVENT_CODE_SIDETOUCH_DISABLE);
			LOGN("sidetouch_disable");
			break;
		default:
			LOGE("invalid status %d\n", data->side_key.status);
			break;
		}
	} else {
		switch (data->side_key.status) {
		case SIDE_KEY_ENABLE:
			mxt_patch_event(data, PATCH_EVENT_CODE_SIDETOUCH_ENABLE);
			LOGN("sidetouch_enable");
			break;
		case SIDE_KEY_ONLY_RIGHT:
			mxt_patch_event(data, PATCH_EVENT_CODE_SIDETOUCH_ONLY_RIGHT);
			LOGN("sidetouch_rightonly");
			break;
		case SIDE_KEY_ONLY_LEFT:
			mxt_patch_event(data, PATCH_EVENT_CODE_SIDETOUCH_ONLY_LEFT);
			LOGN("sidetouch_leftonly");
			break;
		case SIDE_KEY_DISABLE:
			mxt_patch_event(data, PATCH_EVENT_CODE_SIDETOUCH_DISABLE);
			LOGN("sidetouch_disable");
			break;
		default:
			LOGE("invalid status %d\n", data->side_key.status);
			break;
		}
	}
}

static void mxt_set_stamina_mode(struct mxt_data *data)
{
	if (!data->stamina_mode.supported) {
		LOGN("stamina mode is not supported\n");
		return;
	}

	if (data->stamina_mode.status) {
		mxt_patch_event(data, PATCH_EVENT_CODE_STAMINA_ENABLE);
		LOGN("stamina_enable");
	} else {
		if (data->mxt_drv_data->is_resuming){
			LOGN("the device resume\n");
			return;
		}

		mxt_patch_event(data, PATCH_EVENT_CODE_STAMINA_DISABLE);
		LOGN("stamina_disable");
	}
}

static void mxt_set_glove_mode(struct mxt_data *data)
{
	if (!data->glove_mode.supported) {
		LOGN("glove mode is not supported\n");
		return;
	}

	if (data->cover_mode.status) {
		LOGN("cannot change glove mode\n");
		return;
	}

	if (data->glove_mode.status) {
		if (data->side_key.status != SIDE_KEY_DISABLE) {
			mxt_patch_event(data, PATCH_EVENT_CODE_SIDETOUCH_DISABLE);
			LOGN("disable side touch in glove mode\n");
		}

		mxt_patch_event(data, PATCH_EVENT_CODE_GLOVE_ENABLE);
		LOGN("glove_enable");
	} else {
		if (data->mxt_drv_data->is_resuming){
			LOGN("the device resume\n");
			return;
		}

		mxt_patch_event(data, PATCH_EVENT_CODE_GLOVE_DISABLE);
		LOGN("glove_disable");
		mxt_set_side_key(data);
	}
}

static bool mxt_get_or_result(int para, ...)
{
	va_list paras;
	int result = 0;
	int i = 0;

	va_start(paras, para);

	for (; i < para; i++) {
		result += va_arg(paras, int);
		if (result > 0)
			break;
	}

	va_end(paras);
	return result ? true : false;
}

static void mxt_set_cover_mode(struct mxt_data *data)
{
	int sod_mode = SOD_MODE_ON;

	if (data->suspended) {
		LOGN("in suspend will be update after resume\n");
		return;
	}

	if (!data->cover_mode.supported) {
		LOGN("cover mode is not supported\n");
		return;
	}
	if (!incell_get_display_sod() && incell_get_system_status())
		sod_mode = SOD_MODE_OFF;
	if (data->cover_mode.status) {
		if (data->glove_mode.status)
			mxt_patch_event(data, PATCH_EVENT_CODE_GLOVE_DISABLE);
		if (!(mxt_get_or_result(3, data->aod_mode.status, sod_mode, (int)data->wireless_charge))) {
			switch (data->side_key.status) {
			case SIDE_KEY_ENABLE:
			case SIDE_KEY_ONLY_RIGHT:
				mxt_patch_event(data, PATCH_EVENT_CODE_SIDETOUCH_ONLY_RIGHT);
				break;
			case SIDE_KEY_ONLY_LEFT:
				mxt_patch_event(data, PATCH_EVENT_CODE_SIDETOUCH_DISABLE);
				break;
			case SIDE_KEY_DISABLE:
				LOGN("no need to update side key status here\n");
				break;
			default:
				LOGE("invalid status %d\n", data->side_key.status);
				break;
			}
		}

		mxt_patch_event(data, PATCH_EVENT_CODE_COVER_ENABLE);
	} else {
		if (data->mxt_drv_data->is_resuming){
			LOGN("the device resume\n");
			return;
		}

		mxt_patch_event(data, PATCH_EVENT_CODE_COVER_DISABLE);
		if (!(data->aod_mode.status || sod_mode))
			mxt_set_side_key(data);

		if (data->glove_mode.status)
			mxt_set_glove_mode(data);

		if (data->landscape)
			mxt_patch_event(data, PATCH_EVENT_CODE_GRIP_SUPPRESSSION_LANDSCAPE);
		else
			mxt_patch_event(data, PATCH_EVENT_CODE_GRIP_SUPPRESSSION_PORTRAIT);
	}
}

static void mxt_set_aod_mode(struct mxt_data *data)
{
	if (!data->aod_mode.supported) {
		LOGN("AOD mode is not supported\n");
		return;
	}

	if (data->aod_mode.status) {
		if (data->stamina_mode.status)
			mxt_patch_event(data, PATCH_EVENT_CODE_STAMINA_DISABLE);

		mxt_patch_event(data, PATCH_EVENT_CODE_AOD_ENABLE);
		LOGN("AOD_enable");
	} else {
		if (data->mxt_drv_data->is_resuming){
			LOGN("the device resume\n");
			return;
		}

		mxt_patch_event(data, PATCH_EVENT_CODE_AOD_DISABLE);
		LOGN("AOD_disable");
		if (data->stamina_mode.status)
			mxt_set_stamina_mode(data);

		if (data->glove_mode.status)
			mxt_set_glove_mode(data);

		if (data->cover_mode.status)
			mxt_set_cover_mode(data);

		mxt_set_side_key(data);
	}
}

static void mxt_set_feature_status(struct mxt_data *data)
{
	LOGN("%s\n", __func__);

	mxt_set_side_key(data);
	mxt_set_stamina_mode(data);

	data->aod_mode.status = incell_get_display_aod();
	mxt_set_aod_mode(data);

	mxt_set_cover_mode(data);
	mxt_set_glove_mode(data);
	mxt_patch_event(data, PATCH_EVENT_CODE_GRIP_SUPPRESSSION_PORTRAIT);
}

static ssize_t mxt_run_delta_show(struct file *file, struct kobject *kobj,
		struct bin_attribute *bin_attr, char *buf, loff_t off, size_t count)
{
	struct mxt_data *data = container_of(kobj, struct mxt_data, lge_touch_kobj);
	ssize_t retval = -EFAULT;
	int len = 0;

	LOGN("%s : off[%d] count[%d]\n", __func__, (int)off, (int)count);

	if (off == 0) {
		if (data->prd_delta_data == NULL) {
			LOGN("%s : memory NULL\n", __func__);
			data->prd_delta_data = kzalloc(MAX_ATTR_SIZE, GFP_KERNEL);
			if (data->prd_delta_data) {
				LOGN("%s font_buffer(%d KB) malloc success\n", __func__,
						MAX_ATTR_SIZE/1024);
			} else {
				LOGN("%s : memory alloc failed\n", __func__);
				goto error;
			}
		}
		mxt_power_block(data, POWERLOCK_SYSFS);
		run_delta_read(data, data->prd_delta_data, &len);
		mxt_power_unblock(data, POWERLOCK_SYSFS);
	}

	if (off + count > MAX_ATTR_SIZE) {
		LOGN("%s size error offset[%d] size[%d]\n", __func__,
				(int)off, (int)count);
	} else {
		memcpy(buf, &data->prd_delta_data[off], count);
		retval = count;
	}

error :
	return retval;
}

static ssize_t mxt_run_chstatus_show(struct mxt_data *data, char *buf)
{
	int len = 0;
	int mode = 1;

	mxt_power_block(data, POWERLOCK_SYSFS);

	if (data->pdata->panel_on == POWER_OFF) {
		pm_wakeup_event(&data->client->dev, msecs_to_jiffies(2000));
		len += snprintf(buf + len, PAGE_SIZE - len, "************************\n");
		len += snprintf(buf + len, PAGE_SIZE - len, "*** LCD STATUS : OFF ***\n");
		len += snprintf(buf + len, PAGE_SIZE - len, "************************\n");
	} else if (data->pdata->panel_on == POWER_ON) {
		len += snprintf(buf + len, PAGE_SIZE - len, "************************\n");
		len += snprintf(buf + len, PAGE_SIZE - len, "*** LCD STATUS : O N ***\n");
		len += snprintf(buf + len, PAGE_SIZE - len, "************************\n");
	}

	len += snprintf(buf + len, PAGE_SIZE - len, "====== MXT Self Test Info ======\n");
	len = mxt_selftest(data, buf, len, mode);

	run_reference_read(data, buf, &len);
	msleep(30);

	mxt_power_unblock(data, POWERLOCK_SYSFS);

	return len;
}

static ssize_t mxt_run_rawdata_show(struct file *file, struct kobject *kobj,
		struct bin_attribute *bin_attr, char *buf, loff_t off, size_t count)
{
	struct mxt_data *data = container_of(kobj, struct mxt_data, lge_touch_kobj);
	ssize_t retval = -EFAULT;
	int ret = 0;

	LOGN("%s : off[%d] count[%d]\n", __func__, (int)off, (int)count);
	LOGN("%s Start\n", __func__);

	if (off == 0) {
		if (data->prd_rawdata_data == NULL) {
			LOGN("%s : memory NULL\n", __func__);
			data->prd_rawdata_data = kzalloc(MAX_ATTR_SIZE, GFP_KERNEL);
			if (data->prd_rawdata_data) {
				LOGN("%s font_buffer(%d KB) malloc success\n", __func__,
						MAX_ATTR_SIZE/1024);
			} else {
				LOGN("%s : memory alloc failed\n", __func__);
				goto error;
			}
		}
		mxt_power_block(data, POWERLOCK_SYSFS);
		run_reference_read(data, data->prd_rawdata_data, &ret);
		mxt_power_unblock(data, POWERLOCK_SYSFS);
	}

	if (off + count > MAX_ATTR_SIZE) {
		LOGN("%s size error offset[%d] size[%d]\n", __func__,
				(int)off, (int)count);
	} else {
		memcpy(buf, &data->prd_rawdata_data[off], count);
		retval = count;
	}
	LOGN("%s End\n", __func__);

error :
	return retval;
}

static ssize_t mxt_run_self_diagnostic_show(struct mxt_data *data, char *buf)
{
	int len = 0;
	ssize_t info_len = 0;
	int ref_len = 0;
	char *ref_buf = NULL;
	bool chstatus_result = 1;
	bool rawdata_result = 1;
	int write_page = 1 << 14;
	char *fname = NULL;
	int mode = 1;

	mxt_power_block(data, POWERLOCK_SYSFS);

	data->self_test_result_status = SELF_DIAGNOSTIC_STATUS_RUNNING;

	if (data->pdata->panel_on == POWER_OFF) {
		len += snprintf(buf + len, PAGE_SIZE - len, "************************\n");
		len += snprintf(buf + len, PAGE_SIZE - len, "*** LCD STATUS : OFF ***\n");
		len += snprintf(buf + len, PAGE_SIZE - len, "************************\n");
		mxt_power_unblock(data, POWERLOCK_SYSFS);
		data->self_test_result_status = SELF_DIAGNOSTIC_STATUS_COMPLETE;
		return (ssize_t)len;
	}

	LOGN("MXT_COMMAND_CALIBRATE\n");
	mxt_t6_command(data, MXT_COMMAND_CALIBRATE, 1, false);

	ref_buf = kzalloc(write_page, GFP_KERNEL);
	if (!ref_buf) {
		LOGE("%s Failed to allocate memory\n", __func__);
		mxt_power_unblock(data, POWERLOCK_SYSFS);
		data->self_test_result_status = SELF_DIAGNOSTIC_STATUS_COMPLETE;
		return 0;
	}

	if (data->mxt_drv_data->factory_boot)
		fname = SELF_DIAGNOSTIC_FILE_PATH_FACTORY;
	else
		fname = SELF_DIAGNOSTIC_FILE_PATH_NORMAL;
	msleep(30);
	len += mxt_info_show(data, buf);
	len += snprintf(buf + len, PAGE_SIZE - len, "=======RESULT========\n");
	info_len = (ssize_t)len;
	len = mxt_selftest(data, buf, len, mode);
	msleep(30);
	run_reference_read(data, ref_buf, &ref_len);
	msleep(30);

	kfree(ref_buf);

	if ((data->self_test_status[0] == 0x01) || (data->self_test_status[0] == 0x02))
		chstatus_result = 0;

	if (data->self_test_status[0] == 0x17)
		rawdata_result = 0;
	if (data->self_test_status[0] == 0) {
		info_len += snprintf(buf + info_len, PAGE_SIZE - info_len, "Need more time. Try Again.\n");
	} else if (data->self_test_status[0] == 0xFD) {
		info_len += snprintf(buf + info_len, PAGE_SIZE - info_len, "Invalid Test Code. Try Again.\n");
	} else if (data->self_test_status[0] == 0xFC) {
		info_len += snprintf(buf + info_len, PAGE_SIZE - info_len, "The test could not be completed. Try Again.\n");
	} else {
		info_len += snprintf(buf + info_len, PAGE_SIZE - info_len, "Channel Status : %s\n", chstatus_result == 1 ? "PASS" : "FAIL");
		info_len += snprintf(buf + info_len, PAGE_SIZE - info_len, "Raw Data : %s\n", rawdata_result == 1 ? "PASS" : "FAIL");
	}
	mxt_power_unblock(data, POWERLOCK_SYSFS);

	data->self_test_result_status = SELF_DIAGNOSTIC_STATUS_COMPLETE;

	return info_len;
}

static ssize_t mxt_run_lpwg_self_diagnostic_show(struct mxt_data *data, char *buf)
{
	int len = 0;
	ssize_t info_len = 0;
	int ref_len = 0;
	char *ref_buf = NULL;
	bool rawdata_result = 1;
	int write_page = 1 << 14;
	char *fname = NULL;
	int mode = 0;

	mxt_power_block(data, POWERLOCK_SYSFS);

	data->self_test_result_status = SELF_DIAGNOSTIC_STATUS_RUNNING;

	if (data->pdata->panel_on == POWER_ON) {
		len += snprintf(buf + len, PAGE_SIZE - len, "************************\n");
		len += snprintf(buf + len, PAGE_SIZE - len, "*** LCD STATUS : ON ***\n");
		len += snprintf(buf + len, PAGE_SIZE - len, "************************\n");
		mxt_power_unblock(data, POWERLOCK_SYSFS);
		data->self_test_result_status = SELF_DIAGNOSTIC_STATUS_COMPLETE;
		return (ssize_t)len;
	}

	LOGN("MXT_COMMAND_CALIBRATE\n");
	mxt_t6_command(data, MXT_COMMAND_CALIBRATE, 1, false);

	ref_buf = kzalloc(write_page, GFP_KERNEL);
	if (!ref_buf) {
		LOGE("%s Failed to allocate memory\n", __func__);
		mxt_power_unblock(data, POWERLOCK_SYSFS);
		data->self_test_result_status = SELF_DIAGNOSTIC_STATUS_COMPLETE;
		return 0;
	}

	if (data->mxt_drv_data->factory_boot)
		fname = SELF_DIAGNOSTIC_FILE_PATH_FACTORY;
	else
		fname = SELF_DIAGNOSTIC_FILE_PATH_NORMAL;
	msleep(30);
	len += mxt_info_show(data, buf);
	len += snprintf(buf + len, PAGE_SIZE - len, "=======RESULT========\n");
	info_len = (ssize_t)len;
	len = mxt_selftest(data, buf, len, mode);
	msleep(30);
	run_reference_read(data, ref_buf, &ref_len);
	msleep(30);

	kfree(ref_buf);

	if (data->self_test_status[0] == 0x17)
		rawdata_result = 0;

	if (data->self_test_status[0] == 0) {
		info_len += snprintf(buf + info_len, PAGE_SIZE - info_len, "Need more time. Try Again.\n");
	} else if (data->self_test_status[0] == 0xFD) {
		info_len += snprintf(buf + info_len, PAGE_SIZE - info_len, "Invalid Test Code. Try Again.\n");
	} else if (data->self_test_status[0] == 0xFC) {
		info_len += snprintf(buf + info_len, PAGE_SIZE - info_len, "The test could not be completed. Try Again.\n");
	} else {
		info_len += snprintf(buf + info_len, PAGE_SIZE - info_len, "LPWG RawData : %s\n", rawdata_result == 1 ? "PASS" : "FAIL");
	}
	mxt_power_unblock(data, POWERLOCK_SYSFS);

	data->self_test_result_status = SELF_DIAGNOSTIC_STATUS_COMPLETE;

	return info_len;
}

static ssize_t mxt_run_self_diagnostic_status_show(struct mxt_data *data, char *buf)
{
	ssize_t len = 0;

	len += snprintf(buf + len, PAGE_SIZE - len, "%d\n", data->self_test_result_status);

	return len;
}

static ssize_t mxt_run_pin_fault_show(struct mxt_data *data, char *buf)
{
	const uint32_t PIN_FAULT_TEST = 0x12;
	const uint32_t TIMEOUT = 3000; /* [ms] */

	int rc = 0;
	ssize_t len = 0;
	uint32_t t;
	u8 t46_ctrl = 0x00;

	data->self_test_status[0] = 0;
	data->mxt_drv_data->selftest_enable = true;
	data->mxt_drv_data->selftest_show = true;

	LOGN("======== Pin fault ========\n");

	rc = __mxt_read_reg(data->client, data->T46_address + 0,
		sizeof(t46_ctrl), &t46_ctrl);
	if (rc)
		LOGE("Failed to read from T46:0 rc:%d\n", rc);

	rc = mxt_write_reg(data->client, data->T46_address + 0, 0);
	if (rc)
		LOGE("Failed to write into T46:0 rc:%d\n", rc);

	rc = mxt_t25_command(data, PIN_FAULT_TEST, true);
	if (rc) {
		LOGE("mxt_t25_command(0x12) rc:%d\n", rc);
		goto end;
	}

	for (t = 0; t < TIMEOUT && !data->self_test_status[0]; t++)
		msleep(1);

	if (t >= TIMEOUT) {
		LOGE("Pin fault test timed out\n");
		rc = -ETIMEDOUT;
		goto end;
	}

	len = scnprintf(buf, PAGE_SIZE, "%s\n",
			(data->self_test_status[0] != PIN_FAULT_TEST) ? "Pass" : "Fail");
end:
	rc = mxt_write_reg(data->client, data->T46_address + 0, t46_ctrl);
	if (rc)
		LOGE("Failed to write into T46:0 rc:%d\n", rc);

	data->mxt_drv_data->selftest_show = false;
	data->mxt_drv_data->selftest_enable = false;

	return rc ? rc : len;
}

static ssize_t mxt_run_microcrack_show(struct mxt_data *data, char *buf)
{
	const u32 DWELL_TIME_STEP = 41670; //[ps]
	const u8 XSIZE = data->info->matrix_xsize;
	const u8 YSIZE = data->info->matrix_ysize;

	struct mxt_object *dbg_object = NULL;
	u8 read_page, cur_page = 0, end_page = 0;
	u8 read_point;
	u8 ct_t8 = 0, half_of_ct_t8 = 0;
	u16 node = 0, num = 0,  cnt = 0;
	int rc = 0;
	int i = 0, len = 0;
	int value = 0;
	u8 t46_ctrl = 0x00;
	u8 t56_ctrl = 0x01;

	len += scnprintf(buf + len, PAGE_SIZE - len,
			 "======== Micro crack ========");

	/* Evironment Setup for Microcrack testing T46, T8, T56 */
	rc = __mxt_read_reg(data->client, data->T46_address + 0,
		sizeof(t46_ctrl), &t46_ctrl);
	if (rc)
		LOGE("Failed to read from T46:0 rc:%d\n", rc);

	rc = __mxt_read_reg(data->client, data->T56_address + 0,
		sizeof(t56_ctrl), &t56_ctrl);
	if (rc)
		LOGE("Failed to read from T56:0 rc:%d\n", rc);

	rc = mxt_write_reg(data->client, data->T46_address + 0, 8);
	if (rc)
		LOGE("Failed to write into T46:0 rc:%d\n", rc);

	rc = mxt_write_reg(data->client, data->T56_address + 0, 0);
	if (rc)
		LOGE("Failed to write into T56:0 rc:%d\n", rc);

	rc = __mxt_read_reg(data->client, data->T8_address + 0, sizeof(ct_t8),
			    &ct_t8);
	if (rc)
		LOGE("Failed to read from T8:0 rc:%d\n", rc);

	half_of_ct_t8 = ct_t8 / 2;
	rc = mxt_write_reg(data->client, data->T8_address + 0, half_of_ct_t8);
	if (rc)
		LOGE("Failed to write into T8:0 rc:%d\n", rc);

	LOGN("Change dwell time until scanning from %uns to %uns\n",
		ct_t8 * DWELL_TIME_STEP / 1000,
		half_of_ct_t8 * DWELL_TIME_STEP / 1000);

	msleep(20);

	touch_disable_irq(data);
	pm_wakeup_event(&data->client->dev, msecs_to_jiffies(2000));
	mutex_lock(&data->mxt_drv_data->dev_lock);

	/* to make the Page Num to 0 */
	rc = mxt_set_diagnostic_mode(data, MXT_DIAG_CTE_MODE);
	if (rc) {
		LOGE("Failed to make the page number to 0 rc:%d\n", rc);
		goto out;
	}

	/* change the debug mode */
	rc = mxt_set_diagnostic_mode(data, MXT_DIAG_REFERENCE_MODE);
	if (rc) {
		LOGE("Failed to change the debug mode rc:%d\n", rc);
		goto out;
	}

	/* get object info for diagnostic */
	dbg_object = mxt_get_object(data, MXT_DEBUG_DIAGNOSTIC_T37);
	if (!dbg_object) {
		LOGE("Failed to get object_info\n");
		rc = -EINVAL;
		goto out;
	}

	mxt_prepare_debug_data(data);

	end_page = (XSIZE * YSIZE) / NODE_PER_PAGE;
	for (read_page = 0; read_page < end_page; read_page++) {
		for (node = 0; node < NODE_PER_PAGE; node++) {
			if (cnt / YSIZE >= data->channel_size.size_x)
				break;
			if (cnt % YSIZE == 0) {
				len += scnprintf(buf + len, PAGE_SIZE - len,
						 "\n[%2d] ", cnt / YSIZE);
			}
			read_point = (node * DATA_PER_NODE) + 2;

			if (!mxt_check_xy_range(data, cnt++))
				break;

			value = mxt_treat_dbg_data(data, dbg_object,
				MXT_DIAG_DELTA_MODE,
				read_point, num);
			if (cnt % YSIZE <= data->channel_size.size_y &&
			    cnt % YSIZE != 0) {
				len += scnprintf(buf + len, PAGE_SIZE - len,
						 "%5d", value);
			}
			num++;
		}

		rc = mxt_set_diagnostic_mode(data, MXT_DIAG_PAGE_UP);
		if (rc) {
			LOGE("Failed to make the page up\n");
			goto out;
		}
		do {
			msleep(20);
			rc = __mxt_read_reg(data->client,
				dbg_object->start_address + MXT_DIAGNOSTIC_PAGE, 1, &cur_page);
			if (rc) {
				LOGE("Failed to read page\n");
				goto out;
			}
		} while (cur_page != read_page + 1);
	}

	len += snprintf(buf + len, PAGE_SIZE - len, "\n");

	for (i = 0; buf[i];) {
		char *lf_pos = strchr(&buf[i], '\n');

		if (!lf_pos)
			break;

		*lf_pos = 0;
		LOGN("%s\n", &buf[i]);
		*lf_pos = '\n';
		i = lf_pos - buf + 1;
	}

out:
	/* Recover to Original Config */
	rc = mxt_write_reg(data->client, data->T46_address + 0, t46_ctrl);
	if (rc)
		LOGE("Failed to write into T46:0\n");

	rc = mxt_write_reg(data->client, data->T56_address + 0, t56_ctrl);
	if (rc)
		LOGE("Failed to write into T56:0\n");

	rc = mxt_write_reg(data->client, data->T8_address + 0, ct_t8);
	if (rc)
		LOGE("Failed to write into T8:0\n");

	mutex_unlock(&data->mxt_drv_data->dev_lock);

	touch_enable_irq(data);

	return rc ? rc : len;
}

static ssize_t mxt_run_reset_irq_show(struct mxt_data *data, char *buf)
{
	ssize_t len = 0;

	LOGN("======== Reset & Interrupt pin check ========\n");

	(void)mxt_hw_reset(data);

	len = scnprintf(buf, PAGE_SIZE, "%s\n",
			(data->t6_status_reset_irq == MXT_T6_STATUS_RESET)
			? "Pass" : "Fail");

	return len;
}

static ssize_t mxt_run_diag_data_show(struct mxt_data *data, char *buf)
{
	int rc = 0;
	u8 row;

	if (!data->rawdata || !data->rawdata->delta) {
		rc = -EFAULT;
		goto out;
	}

	buf[0] = data->channel_size.size_x;
	buf[1] = data->channel_size.size_y;
	for (row = 0; row < data->channel_size.size_x; row++) {
		memcpy(&buf[row * data->channel_size.size_y * sizeof(s16) + 2],
			&data->rawdata->delta[row * data->rawdata->num_ynode],
			data->channel_size.size_y * sizeof(s16));
	}

out:
	return rc ?
		rc :
		data->channel_size.size_x *
			data->channel_size.size_y *
			sizeof(s16) + 2;
}

static ssize_t mxt_global_access_pixel_show(struct mxt_data *data, char *buf)
{
	ssize_t len = 0;

	len += snprintf(buf + len, PAGE_SIZE - len, "%d\n", data->pdata->global_access_pixel);

	return len;
}

static ssize_t mxt_global_access_pixel_store(struct mxt_data *data, const char *buf, size_t count)
{
	int value = 0;

	sscanf(buf, "%d", &value);

	LOGD("%s = %d\n", __func__, value);

	data->pdata->global_access_pixel = value;

	return count;
}

static ssize_t mxt_force_rebase_show(struct mxt_data *data, char *buf)
{
	ssize_t len = 0;

	if (data->pdata->panel_on == POWER_OFF) {
		pm_wakeup_event(&data->client->dev, msecs_to_jiffies(2000));
	}

	LOGD("MXT_COMMAND_CALIBRATE\n");
	mxt_t6_command(data, MXT_COMMAND_CALIBRATE, 1, false);

	return len;
}

static ssize_t mxt_mfts_enable_show(struct mxt_data *data, char *buf)
{
	ssize_t len = 0;

	len += snprintf(buf + len, PAGE_SIZE - len, "%d\n", data->mfts_enable);

	return len;
}

static ssize_t mxt_mfts_enable_store(struct mxt_data *data, const char *buf, size_t count)
{
	int value = 0;

	sscanf(buf, "%d", &value);

	LOGN("%s = %d\n", __func__, value);

	data->mfts_enable = value;

	/* Touch IC Reset for Initial configration. */
	mxt_soft_reset(data);

	/* Calibrate for Active touch IC */
	mxt_t6_command(data, MXT_COMMAND_CALIBRATE, 1, false);

	return count;
}

static ssize_t mxt_glove_mode_show(struct mxt_data *data, char *buf)
{
	ssize_t len = 0;

	len += snprintf(buf + len, PAGE_SIZE - len, "%d\n", data->glove_mode.status);

	return len;
}

static ssize_t mxt_glove_mode_store(struct mxt_data *data, const char *buf, size_t count)
{
	bool value;

	if (kstrtobool(buf, &value)) {
		LOGE("failed to read sysfs\n");
		return -EINVAL;
	}

	data->glove_mode.status = value;
	LOGN("request glove mode:%s\n",
		data->glove_mode.status ? "Enable" : "Disable");

	if (!(data->after_work && data->charge_out)) {
		LOGE("initializing incomplete, will update feature on resume\n");
		LOGE("after_work:%s, charge_out:%s\n",
			data->after_work ? "Passed" : "Failure",
			data->charge_out ? "Passed" : "Failure");
		goto update_skip;
	}

	if (mxt_get_pw_status()) {
		LOGE("params update, will update feature on resume\n");
		goto update_skip;
	}

	mxt_pw_lock(INCELL_DISPLAY_POWER_LOCK);
	mxt_set_glove_mode(data);
	mxt_pw_lock(INCELL_DISPLAY_POWER_UNLOCK);

update_skip:
	return count;
}

static ssize_t mxt_sidekey_status_show(struct mxt_data *data, char *buf)
{
	ssize_t len = 0;

	len += snprintf(buf + len, PAGE_SIZE - len, "%d\n", data->side_key.status);

	return len;
}

static ssize_t mxt_sidekey_status_store(struct mxt_data *data, const char *buf, size_t count)
{
	int value;
	u16 base = 10; /* decimal */

	if (kstrtoint(buf, base, &value)) {
		LOGE("failed to read sysfs\n");
		return -EINVAL;
	}

	if (value < SIDE_KEY_ENABLE_CHARGER)
		data->side_key.status = value;
	else if (value < SIDE_KEY_DISABLE_CHARGER)
		data->wireless_charge = true;
	else
		data->wireless_charge = false;

	if (mxt_get_pw_status()) {
		LOGE("params update, will update feature on resume\n");
		goto update_skip;
	}

	mxt_pw_lock(INCELL_DISPLAY_POWER_LOCK);
	if (value == SIDE_KEY_ENABLE_CHARGER)
		mxt_patch_event(data, PATCH_EVENT_CODE_SIDETOUCH_DISABLE);
	else
		mxt_set_side_key(data);

	mxt_pw_lock(INCELL_DISPLAY_POWER_UNLOCK);

update_skip:
	return count;
}

static ssize_t mxt_cover_mode_show(struct mxt_data *data, char *buf)
{
	ssize_t len = 0;

	len += snprintf(buf + len, PAGE_SIZE - len, "%d\n", data->cover_mode.status);

	return len;
}

static ssize_t mxt_cover_mode_store(struct mxt_data *data, const char *buf, size_t count)
{
	bool value;

	if (kstrtobool(buf, &value)) {
		LOGE("failed to read sysfs\n");
		return -EINVAL;
	}

	data->cover_mode.mode = value;
	LOGN("request cover mode:%s\n",
		data->cover_mode.status ? "Enable" : "Disable");

	return count;
}

static ssize_t mxt_cover_status_show(struct mxt_data *data, char *buf)
{
	ssize_t len = 0;

	len += snprintf(buf + len, PAGE_SIZE - len, "%d\n", data->cover_mode.status);

	return len;
}

static ssize_t mxt_cover_status_store(struct mxt_data *data, const char *buf, size_t count)
{
	bool value;

	if (kstrtobool(buf, &value)) {
		LOGE("failed to read sysfs\n");
		return -EINVAL;
	}

	if (!data->cover_mode.mode) {
		LOGN("cover mode is disable\n");
		goto update_skip;
	}

	data->cover_mode.status = value;
	LOGN("request cover mode:%s\n",
		data->cover_mode.status ? "Enable" : "Disable");

	if (!(data->after_work && data->charge_out)) {
		LOGE("initializing incomplete, will update feature on resume\n");
		LOGE("after_work:%s, charge_out:%s\n",
			data->after_work ? "Passed" : "Failure",
			data->charge_out ? "Passed" : "Failure");
		goto update_skip;
	}

	if (mxt_get_pw_status()) {
		LOGE("params update, will update feature on resume\n");
		goto update_skip;
	}

	mxt_pw_lock(INCELL_DISPLAY_POWER_LOCK);
	mxt_set_cover_mode(data);
	mxt_pw_lock(INCELL_DISPLAY_POWER_UNLOCK);

update_skip:
	return count;
}

static ssize_t mxt_stamina_mode_show(struct mxt_data *data, char *buf)
{
	ssize_t len = 0;

	len += snprintf(buf + len, PAGE_SIZE - len, "%d\n", data->stamina_mode.status);

	return len;
}

static ssize_t mxt_stamina_mode_store(struct mxt_data *data, const char *buf, size_t count)
{
	bool value;

	if (kstrtobool(buf, &value)) {
		LOGE("failed to read sysfs\n");
		return -EINVAL;
	}

	data->stamina_mode.status = value;
	LOGN("request stamina mode:%s\n",
		data->stamina_mode.status ? "Enable" : "Disable");

	if (!(data->after_work && data->charge_out)) {
		LOGE("initializing incomplete, will update feature on resume\n");
		LOGE("after_work:%s, charge_out:%s\n",
			data->after_work ? "Passed" : "Failure",
			data->charge_out ? "Passed" : "Failure");
		goto update_skip;
	}

	if (mxt_get_pw_status()) {
		LOGE("params update, will update feature on resume\n");
		goto update_skip;
	}



	mxt_pw_lock(INCELL_DISPLAY_POWER_LOCK);
	mxt_set_stamina_mode(data);
	mxt_pw_lock(INCELL_DISPLAY_POWER_UNLOCK);

update_skip:
	return count;
}

static ssize_t mxt_aod_mode_show(struct mxt_data *data, char *buf)
{
	ssize_t len = 0;

	len += snprintf(buf + len, PAGE_SIZE - len, "%d\n", data->aod_mode.status);

	return len;
}

static ssize_t mxt_aod_mode_store(struct mxt_data *data, const char *buf, size_t count)
{
	bool value;

	if (kstrtobool(buf, &value)) {
		LOGE("failed to read sysfs\n");
		return -EINVAL;
	}

	data->aod_mode.status = value;
	LOGN("request aod mode:%s\n",
		data->aod_mode.status ? "Enable" : "Disable");

	if (!(data->after_work && data->charge_out)) {
		LOGE("initializing incomplete, will update feature on resume\n");
		LOGE("after_work:%s, charge_out:%s\n",
			data->after_work ? "Passed" : "Failure",
			data->charge_out ? "Passed" : "Failure");
		goto update_skip;
	}

	if (mxt_get_pw_status()) {
		LOGE("params update, will update feature on resume\n");
		goto update_skip;
	}

	mxt_pw_lock(INCELL_DISPLAY_POWER_LOCK);
	mxt_set_aod_mode(data);
	mxt_pw_lock(INCELL_DISPLAY_POWER_UNLOCK);

update_skip:
	return count;
}

static ssize_t mxt_sod_mode_store(struct mxt_data *data, const char *buf, size_t count)
{
	unsigned int value = 0;

	if (kstrtouint(buf, 0, &value)) {
		LOGE("failed to read sysfs\n");
		return -EINVAL;
	}

	if (value > SOD_POWER_OFF) {
		LOGE("invalid value :%d\n", value);
		return -EINVAL;
	}

	if (data->sod_mode.status != value) {
		data->sod_mode.status = value;
		LOGN("request sod mode: %d\n", data->sod_mode.status);
	}

	return count;
}

static ssize_t mxt_pre_sod_mode_store(struct mxt_data *data, const char *buf, size_t count)
{
	bool value;

	if (kstrtobool(buf, &value)) {
		LOGE("failed to read sysfs\n");
		return -EINVAL;
	}

	data->sod_mode.pre_status = value;
	LOGN("request sod mode: %s\n", data->sod_mode.pre_status ? "Enable" : "Disable");

	return count;
}

static ssize_t mxt_grip_suppression_landscape_store(struct mxt_data *data, const char *buf, size_t count)
{
	bool value;

	if (kstrtobool(buf, &value)) {
		LOGE("failed to read sysfs\n");
		return -EINVAL;
	}

	data->landscape = value;
	LOGN("Change to %s mode\n", data->landscape ? "Landscape" : "Portrait");

	if (data->aod_mode.status || data->cover_mode.status) {
		LOGN("Can not change grip suppression now\n");
		return count;
	}

	if (mxt_get_pw_status()) {
		LOGE("No power supply\n");
		return count;
	}

	if (data->landscape)
		mxt_patch_event(data, PATCH_EVENT_CODE_GRIP_SUPPRESSSION_LANDSCAPE);
	else
		mxt_patch_event(data, PATCH_EVENT_CODE_GRIP_SUPPRESSSION_PORTRAIT);

	return count;
}

static ssize_t mxt_range_changer_store(struct mxt_data *data, const char *buf, size_t count)
{
	int status;
	int buffer[MXT_GRIP_REJECTION_BORDER_NUM] = {0};
	int i = 0;
	int ret = 0;

	ret = sscanf(buf, "%d %d %d %d %d", &status, &buffer[0], &buffer[1], &buffer[2], &buffer[3]);
	if (ret != 1) {
		LOGN("Wrong fromat\n");
		return count;
	}

	if (status == 0) {
		for (; i < MXT_GRIP_REJECTION_BORDER_NUM; i++)
			portrait_buffer[i] = buffer[i];
	} else {
		for (; i < MXT_GRIP_REJECTION_BORDER_NUM; i++)
			landscape_buffer[i] = buffer[i];
	}

	return count;
}

static ssize_t mxt_register_r_show(struct mxt_data *data, char *buf)
{
	ssize_t len = 0;

	len += snprintf(buf + len, PAGE_SIZE - len, "%2X\n", data->register_value);

	return len;
}

static ssize_t mxt_register_r_store(struct mxt_data *data, const char *buf, size_t count)
{
	int type = 0;
	int offset = 0;
	int error = 0;

	sscanf(buf, "%d %d\n", &type, &offset);

	error = mxt_read_object(data, (u8)type, (u8)offset, &data->register_value);
	if (error)
		LOGE("Failed to read object Type %d, byte %d\n", type, offset);

	return count;
}

static ssize_t mxt_register_w_store(struct mxt_data *data, const char *buf, size_t count)
{
	int type = 0;
	int offset = 0;
	int value = 0;
	int error = 0;

	sscanf(buf, "%d %d %d\n", &type, &offset, &value);

	error = mxt_write_object(data, (u8)type, (u8)offset, (u8)value);
	if (error)
		LOGE("Failed to write object Type %d, byte %d, with value %d\n", type, offset, value);

	error = mxt_read_object(data, (u8)type, (u8)offset, &data->register_value);
	if (error)
		LOGE("Failed to read object Type %d, byte %d\n", type, offset);

	return count;
}

static void mxt_lpwg_enable(struct mxt_data *data, u32 value)
{
	struct mxt_object *object;
	int error = 0;
	struct mxt_data *mxt_data = data->mxt_drv_data->mxt_data;

	object = mxt_get_object(data, MXT_PROCI_TOUCH_SEQUENCE_LOGGER_T93);
	if (!object)
		return;

	if (data->suspended) {
		LOGD("%s : Wake Up from Quick Cover.\n", __func__);
		return;
	}

	if (data->suspended) {
		mxt_reset_slots(data);
		mxt_regulator_enable(data);

		if (mxt_patchevent_get(data, PATCH_EVENT_KNOCKON)) {
			LOGN("Suspend : Knock On already enabled \n");
		} else {
			mxt_patchevent_set(data, PATCH_EVENT_KNOCKON);
			if (mxt_data->lpwg_mode == LPWG_DOUBLE_TAP) {
				if (mxt_patchevent_get(data, PATCH_EVENT_TA)) {
					LOGN("Suspend : Knock On Enabled(TA) \n");
					data->knock_on_mode = CHARGER_KNOCKON_SLEEP;
				} else {
					LOGN("Suspend : Knock On Enabled\n");
					data->knock_on_mode = NOCHARGER_KNOCKON_SLEEP;
				}
			} else if (mxt_data->lpwg_mode == LPWG_MULTI_TAP) {
				if (mxt_patchevent_get(data, PATCH_EVENT_TA)) {
					LOGN("Suspend : Knock On Enabled(TA) \n");
					data->knock_on_mode = CHARGER_KNOCKON_SLEEP + PATCH_EVENT_PAIR_NUM;
				} else {
					LOGN("Suspend : Knock On Enabled\n");
					data->knock_on_mode = NOCHARGER_KNOCKON_SLEEP + PATCH_EVENT_PAIR_NUM;
				}
				/* Write TCHCNTTHR(Touch Count Threshold) for LPWG_MULTI_TAP */
				error = mxt_write_reg(data->client, object->start_address+17, data->g_tap_cnt);
				if (error)
					LOGE("Object Write Fail\n");
			}
		}
		touch_enable_irq(data);
		touch_enable_irq_wake(data);
		data->enable_reporting = true;
	}

	if (value == LPWG_DOUBLE_TAP) {
		data->is_knockONonly = true;
		error = mxt_write_object(data, MXT_PROCI_TOUCH_SEQUENCE_LOGGER_T93, 22, 110);
		LOGN("Set Knock ON range (10mm)\n");
	} else if (value == LPWG_MULTI_TAP) {
		data->is_knockONonly = false;
		error = mxt_write_object(data, MXT_PROCI_TOUCH_SEQUENCE_LOGGER_T93, 22, 70);
		LOGN("Set Knock ON range (7mm)\n");
	}
}

static void mxt_lpwg_disable(struct mxt_data *data, u32 value)
{
	if (data->suspended) {
		mxt_reset_slots(data);
		if (mxt_patchevent_get(data, PATCH_EVENT_KNOCKON)) {
			LOGN("Suspend : Knock On Disabled \n");
			mxt_regulator_disable(data);
			mxt_patchevent_unset(data, PATCH_EVENT_KNOCKON);
		} else {
			LOGN("Suspend : Knock On already disabled\n");
		}
		touch_disable_irq_wake(data);
		touch_disable_irq(data);
		data->enable_reporting = false;
	} else {
		if (value == LPWG_NONE) {
			data->mxt_knock_on_enable = false;
			data->mxt_multi_tap_enable = false;
			LOGN("KnockOn/Multitap Gesture Disable\n");
		} else {
			LOGN("Unknown Value. Not Setting\n");
			return;
		}
	}
}

static void mxt_lpwg_control(struct mxt_data *data, u32 value, bool onoff)
{

	LOGN("%s [%s]\n", __func__, data->suspended ? "SLEEP" : "WAKEUP");

	if (data->in_bootloader) {
		LOGN("%s : Fw upgrade mode.\n", __func__);
		return;
	}

	mutex_lock(&data->mxt_drv_data->dev_lock);

	if (onoff == 1) {
		mxt_lpwg_enable(data, value);
	} else {
		mxt_lpwg_disable(data, value);
	}

	mutex_unlock(&data->mxt_drv_data->dev_lock);
}

static void lpwg_early_suspend(struct mxt_data *data)
{
	LOGN("%s Start\n", __func__);

	switch (data->lpwg_mode) {
		case LPWG_DOUBLE_TAP:
			data->mxt_knock_on_enable = true;
			data->mxt_multi_tap_enable = false;
			break;
		case LPWG_MULTI_TAP:
			data->mxt_knock_on_enable = false;
			data->mxt_multi_tap_enable = true;
			break;
		default:
			break;
	}
	LOGN("%s End\n", __func__);
}

static void lpwg_late_resume(struct mxt_data *data)
{
	LOGN("%s Start\n", __func__);

	data->mxt_knock_on_enable = false;
	data->mxt_multi_tap_enable = false;

	memset(data->mxt_drv_data->tci_press, 0, sizeof(data->mxt_drv_data->tci_press));
	memset(data->mxt_drv_data->tci_report, 0, sizeof(data->mxt_drv_data->tci_report));
	LOGN("%s End\n", __func__);
}

static void lpwg_double_tap_check(struct mxt_data *data, u32 value)
{
	int error = 0;
	int knockOn_delay = 0; // 16ms/unit

	LOGN("%s Double Tap check value:%d\n", __func__, value);

	if (value == 1) {
		data->is_knockCodeDelay = true;
		knockOn_delay = 43;
	} else {
		data->is_knockCodeDelay = false;
		knockOn_delay = 0;
	}

	error = mxt_write_object(data, MXT_PROCI_TOUCH_SEQUENCE_LOGGER_T93, 19, knockOn_delay);
	if (error) {
		LOGE("%s T93 waited(%d) knock On write fail\n", __func__, knockOn_delay);
		return;
	}
}

static void atmel_ts_lpwg_update_all(struct i2c_client* client, u32 code, u32* value)
{

	struct mxt_data *data = i2c_get_clientdata(client);

	data->lpwg_mode = value[0];
	data->screen = value[1];
	data->sensor = value[2];
	data->qcover = value[3];

	LOGN("%s mode:%d, Scree:%d Proximity:%d(FAR:1, NEAR:0) QCover:%d(OPEN:0, CLOSE:1)\n",
			__func__, data->lpwg_mode, data->screen, data->sensor, data->qcover);

	if(data->screen) { // Screen On
		if(data->suspended) {
			touch_disable_irq_wake(data);
			touch_disable_irq(data);
			data->enable_reporting = false;
		}

		data->mxt_knock_on_enable = false;
		data->mxt_multi_tap_enable = false;
	} else {
		switch (data->lpwg_mode) {
			case LPWG_DOUBLE_TAP:
				data->mxt_knock_on_enable = true;
				data->mxt_multi_tap_enable = false;
			break;
			case LPWG_MULTI_TAP:
				data->mxt_knock_on_enable = false;
				data->mxt_multi_tap_enable = true;
			break;
			default:
				data->mxt_knock_on_enable = false;
				data->mxt_multi_tap_enable = false;
			break;
		}
	}
	if (!data->lpwg_mode) {
		mxt_lpwg_control(data, value[0], false);
	} else if (!data->screen && // Screen Off
			data->qcover) {   // QCover Close
		mxt_lpwg_control(data, value[0], false);
	} else if (!data->screen && // Screen Off
			data->sensor &&  // Proximity FAR
			!data->qcover) {  // QCover Open
		mxt_lpwg_control(data, value[0], true);
		if(data->mxt_knock_on_enable || data->mxt_multi_tap_enable) {
			touch_disable_irq(data);
			touch_disable_irq_wake(data);

			mxt_gesture_mode_start(data);

			touch_enable_irq(data);
			touch_enable_irq_wake(data);
		}
	} else if (!data->screen && // Screen Off
			!data->sensor &&  // Proximity NEAR
			!data->qcover) {  // QCover Open
		mxt_lpwg_control(data, value[0], false);
	}
	else if(data->screen) { // Screen On
		touch_disable_irq(data);
		touch_disable_irq_wake(data);

		if (!data->qcover)
			mxt_regulator_enable(data);

		mxt_active_mode_start(data);

		touch_enable_irq(data);
	}
	else {
		LOGD(" ELSE case\n");
	}
}

err_t atmel_ts_lpwg(struct i2c_client* client, u32 code, u32 value, struct point *tci_point)
{
	struct mxt_data *data = i2c_get_clientdata(client);
	int i;
	LOGN("%s Code: %d Value: %d\n", __func__, code, value);

	switch (code) {
	case LPWG_READ:
		if (data->mxt_multi_tap_enable) {
			if ((data->mxt_drv_data->tci_press[0].x == -1) && (data->mxt_drv_data->tci_press[0].y == -1)) {
				LOGE("Tap count error \n");
				tci_point[0].x = 1;
				tci_point[0].y = 1;

				tci_point[1].x = -1;
				tci_point[1].y = -1;
			} else {
				for (i = 0; i < data->g_tap_cnt ; i++) {
					tci_point[i].x = data->mxt_drv_data->tci_report[i].x;
					tci_point[i].y = data->mxt_drv_data->tci_report[i].y;
				}

				// '-1' should be assinged to the last data.
				tci_point[data->g_tap_cnt].x = -1;
				tci_point[data->g_tap_cnt].y = -1;

				// Each data should be converted to LCD-resolution.
				// TODO
			}
		}
		break;
	case LPWG_ENABLE:
		data->lpwg_mode = value;

		if (value)
			mxt_lpwg_control(data, value, true);
		else
			mxt_lpwg_control(data, value, false);

		break;
	case LPWG_LCD_X:
	case LPWG_LCD_Y:
		// If touch-resolution is not same with LCD-resolution,
		// position-data should be converted to LCD-resolution.
		break;
	case LPWG_ACTIVE_AREA_X1:
		data->qwindow_size->x_min = value;
		break;
	case LPWG_ACTIVE_AREA_X2:
		data->qwindow_size->x_max = value;
		break;
	case LPWG_ACTIVE_AREA_Y1:
		data->qwindow_size->y_min = value;
		break;
	case LPWG_ACTIVE_AREA_Y2:
		data->qwindow_size->y_max = value;
		break;
		// Quick Cover Area
	case LPWG_TAP_COUNT:
		// Tap Count Control . get from framework write to IC
		data->g_tap_cnt = value;
		break;
	case LPWG_REPLY:
		// Do something, if you need.
		if (value == 0 && data->mxt_multi_tap_enable) {	/* password fail */
			LOGE("Screen on fail\n");
			if (data->pdata->panel_on == POWER_OFF)
				mxt_gesture_mode_start(data);
		}
		break;
	case LPWG_LENGTH_BETWEEN_TAP:
		break;
	case LPWG_EARLY_MODE:
		if (value == 0)
			lpwg_early_suspend(data);
		else if (value == 1)
			lpwg_late_resume(data);
		break;
	case LPWG_DOUBLE_TAP_CHECK:
		lpwg_double_tap_check(data, value);
		break;
	default:
		break;
	}

	return NO_ERROR;
}

/* Sysfs - lpwg_data (Low Power Wake-up Gesture)
 *
 * read : "x1 y1\n x2 y2\n ..."
 * write
 * 1 : ENABLE/DISABLE
 * 2 : LCD SIZE
 * 3 : ACTIVE AREA
 * 4 : TAP COUNT
 */
static struct point lpwg_data[MAX_POINT_SIZE_FOR_LPWG];
static ssize_t show_lpwg_data(struct mxt_data *data, char *buf)
{
	int i = 0;
	ssize_t ret = 0;

	LOGN("%s\n", __func__);

	memset(lpwg_data, 0, sizeof(struct point)*MAX_POINT_SIZE_FOR_LPWG);
	atmel_ts_lpwg(data->client, LPWG_READ, 0, lpwg_data);

	for (i = 0; i < MAX_POINT_SIZE_FOR_LPWG; i++) {
		if (lpwg_data[i].x == -1 && lpwg_data[i].y == -1)
			break;
		ret += sprintf(buf+ret, "%d %d\n", lpwg_data[i].x, lpwg_data[i].y);
	}
	return ret;
}

static ssize_t store_lpwg_data(struct mxt_data *data, const char *buf, size_t count)
{
	int reply = 0;

	sscanf(buf, "%d", &reply);
	LOGN("%s reply : %d\n", __func__, reply);
	atmel_ts_lpwg(data->client, LPWG_REPLY, reply, NULL);

	pm_relax(&data->client->dev);

	return count;
}

/* Sysfs - lpwg_notify (Low Power Wake-up Gesture)
 *
 */
static ssize_t store_lpwg_notify(struct mxt_data *data, const char *buf, size_t count)
{
	int type = 0;
	int value[4] = {0};

	if (!(data->after_work && data->charge_out)) {
		LOGE("initializing incomplete, will update feature on resume\n");
		LOGE("after_work:%s, charge_out:%s\n",
			data->after_work ? "Passed" : "Failure",
			data->charge_out ? "Passed" : "Failure");
		goto update_skip;
	}

	if (mxt_get_pw_status()) {
		LOGE("params update, will update feature on resume\n");
		goto update_skip;
	}

	mutex_lock(&data->mxt_drv_data->lpwg_lock);

	sscanf(buf, "%d %d %d %d %d", &type, &value[0], &value[1], &value[2], &value[3]);

	if (type == 1 || type == 2 || type == 5 || type == 6 || type == 7) {
		mutex_unlock(&data->mxt_drv_data->lpwg_lock);
		return count;
	}

	LOGN("%s : %d %d %d %d %d\n", __func__, type, value[0], value[1], value[2], value[3]);

	switch (type) {
	case 3 :
		atmel_ts_lpwg(data->client, LPWG_ACTIVE_AREA_X1, value[0], NULL);
		atmel_ts_lpwg(data->client, LPWG_ACTIVE_AREA_X2, value[1], NULL);
		atmel_ts_lpwg(data->client, LPWG_ACTIVE_AREA_Y1, value[2], NULL);
		atmel_ts_lpwg(data->client, LPWG_ACTIVE_AREA_Y2, value[3], NULL);
		break;
	case 4 :
		atmel_ts_lpwg(data->client, LPWG_TAP_COUNT, value[0], NULL);
		break;
	case 8 :
		atmel_ts_lpwg(data->client, LPWG_DOUBLE_TAP_CHECK, value[0], NULL);
		break;
	case 9 :
		atmel_ts_lpwg_update_all(data->client, LPWG_UPDATE_ALL, value);
		break;
	default:
		break;
		}
	mutex_unlock(&data->mxt_drv_data->lpwg_lock);

update_skip:
	return count;
}

static ssize_t store_incoming_call(struct mxt_data *data, const char *buf, size_t count)
{
	static char incoming_call_str[3][8] = {"IDLE", "RINGING", "OFFHOOK"};

	sscanf(buf, "%d", &data->incoming_call);

	if (data->incoming_call <= INCOMING_CALL_OFFHOOK)
		LOGN("%s : %s(%d)\n", __func__, incoming_call_str[data->incoming_call], data->incoming_call);
	else
		LOGN("%s : %d\n", __func__, data->incoming_call);

	return count;
}

static ssize_t store_keyguard_info(struct mxt_data *data, const char *buf, size_t count)
{
	int value = 0;

	sscanf(buf, "%d", &value);

	LOGD("%s : %d\n", __func__, value);

	return count;
}

static ssize_t store_ime_status(struct mxt_data *data, const char *buf, size_t count)
{
	int value = 0;

	sscanf(buf, "%d", &value);

	if(data->mxt_drv_data->ime_status_value == value)
		return count;

	LOGD("%s : %d\n", __func__, value);

	if (value == 1) {
		mxt_write_object(data, MXT_PROCG_NOISESUPPRESSION_T72, 1, 2);
		LOGD("Very Noise ON\n");

	} else if (value == 0) {
		mxt_write_object(data, MXT_PROCG_NOISESUPPRESSION_T72, 1, 0);
		LOGD("Very Noise Off\n");
	}

	data->mxt_drv_data->ime_status_value = value;

	return count;
}

static ssize_t mxt_show_self_ref(struct mxt_data *data, char *buf)
{
	mxt_get_self_reference_chk(data);
	return 0;
}

#ifdef CONFIG_TOUCHSCREEN_ATMEL_STYLUS_PEN
static ssize_t mxt_show_pen_support(struct mxt_data *data, char *buf)
{
	ssize_t ret = 0;

	LOGD("%s() val : %d\n", __func__, data->pen_support);
	ret += sprintf(buf + ret, "%d\n", data->pen_support);
	return ret;
}

static int mxt_read_pen_support(struct mxt_data *data)
{
	int error = 0;
	u8 val = 0;

	error = mxt_read_object(data, MXT_PROCI_STYLUS_T47, 0, &val);
	if (error) {
		LOGE("Read Fail MXT_PROCI_STYLUS_T47 1st byte\n");
		data->pen_support = 0;
		return error;
	}
	data->pen_support = val & 0x01;
	LOGD("%s() val : %d\n",__func__, data->pen_support);

	return error;
}
#endif

static ssize_t mxt_self_cap_show(struct mxt_data *data, char *buf)
{
	int len = 0;
	len += snprintf(buf + len, PAGE_SIZE - len, "%d\n", data->self_cap);
	return len;
}

static ssize_t mxt_self_cap_store(struct mxt_data *data, const char *buf, size_t count)
{
	int value = 0;
	sscanf(buf, "%d", &value);
	LOGD("%s : %d\n", __func__, value);
	if (value == 1) {
	    data->self_cap = value;
		LOGD(" Noise suppression \n");
	} else if (value == 0){
	    data->self_cap = value;
		LOGD(" Noise recover \n");
	}else{
		LOGD(" Do nothing\n");
	}
	return count;
}

static ssize_t mxt_mfts_lpwg_show(struct mxt_data *data, char *buf)
{
	int len = 0;

	len += snprintf(buf + len, PAGE_SIZE - len, "%d\n", data->pdata->lpwg_test);

	return len;
}

static ssize_t mxt_mfts_lpwg_store(struct mxt_data *data, const char *buf, size_t count)
{
	int value = 0;

	if (sscanf(buf, "%d", &value) <= 0)
		return count;

	data->mfts_lpwg = value;
	LOGD("mfts_lpwg : %d\n", data->mfts_lpwg);

	return count;
}

static ssize_t mxt_noise_suppression_show(struct mxt_data *data, char *buf)
{
	int len = 0;
	data->self_cap = 1;
	len += snprintf(buf + len, PAGE_SIZE - len, "%d\n", data->self_cap);
	LOGD("%s : %d \n", __func__, data->self_cap);
	LOGD(" Noise suppression \n");
	return len;
}
static ssize_t mxt_noise_suppression_store(struct mxt_data *data, const char *buf, size_t count)
{
	int value = 0;
	sscanf(buf, "%d", &value);
	LOGD("%s : %d \n", __func__, value);
	if(value == 1){
	    data->self_cap = value;
		LOGD(" Noise suppression \n");
	} else if (value == 0){
	    data->self_cap = value;
		LOGD(" Noise recover \n");
	}else{
		LOGD(" Do nothing\n");
	}
	return count;
}

static ssize_t mxt_noise_recover_show(struct mxt_data *data, char *buf)
{
	int len = 0;
	data->self_cap = 0;
	len += snprintf(buf + len, PAGE_SIZE - len, "%d\n", data->self_cap);
	LOGD("%s : %d \n", __func__, data->self_cap);
	LOGD(" Noise suppression recovered \n");
	return len;
}

static ssize_t mxt_noise_recover_store(struct mxt_data *data, const char *buf, size_t count)
{
	int value = 0;
	sscanf(buf, "%d", &value);
	data->self_cap = value;
	LOGD("%s : %d \n", __func__, value);
	if(value == 1){
	    data->self_cap = value;
		LOGD(" Noise suppression \n");
	} else if (value == 0){
	    data->self_cap = value;
		LOGD(" Noise recover \n");
	}else{
		LOGD(" Do nothing\n");
	}
	return count;
}
#ifdef CONFIG_TOUCHSCREEN_ATMEL_LPWG_DEBUG_REASON
static ssize_t show_lpwg_debug_reason(struct mxt_data *data, char *buf)
{
	int len = 0;
	u8 value[2] = {0,};
	struct mxt_object *object = NULL;

	object = mxt_get_object(data, MXT_PROCI_TOUCH_SEQUENCE_LOGGER_T93);
	if (object) {
		__mxt_read_reg(data->client, object->start_address + 28, 2, value);
		len += snprintf(buf + len, PAGE_SIZE - len, "0x%x, 0x%x\n", value[0], value[1]);
		data->pdata->use_debug_reason = (value[0] | value[1]) ? 1 : 0;
		LOGD("%s debug reason ctrl 28th:0x%x, 29:0x%x, use_debug_reason:%d\n",
				__func__, value[0], value[1], data->pdata->use_debug_reason);
	} else {
		LOGE("Failed get T93 Object\n");
	}
	return len;
}

static ssize_t show_save_lpwg_debug_reason(struct mxt_data *data, char *buf)
{
	int len = 0;
	int i = 0;
	u8 value[10] = {0,};

	save_lpwg_debug_reason(data, value);

	for (i = 0;i < 10;i++) {
		len += snprintf(buf + len, PAGE_SIZE - len, "0x%x, ", value[i]);
	}

	len += snprintf(buf + len, PAGE_SIZE - len, "\n");

	return len;
}
#endif

static ssize_t mxt_irq_enable_show(struct mxt_data *data, char *buf)
{
	int len = 0;

	len += snprintf(buf + len, PAGE_SIZE - len, "%d\n", mxt_get_touch_irq(data));

	return len;
}

static ssize_t mxt_irq_enable_store(struct mxt_data *data, const char *buf, size_t count)
{
	bool value;

	if (!(data->after_work && data->charge_out)) {
		LOGE("please, set it after probe completion\n");
		goto exit;
	}

	if (kstrtobool(buf, &value)) {
		LOGE("failed to read sysfs\n");
		count = -EINVAL;
		goto exit;
	}
	if (value) {
		if (!mxt_get_pw_status())
			touch_enable_irq(data);
		else
			LOGN("irq cannot enable, because panel power status is off\n");
	} else {
		touch_disable_irq(data);
	}

exit:
	return count;
}

static ssize_t mxt_charge_out_store(struct mxt_data *data, const char *buf, size_t count)
{
	int ret = 0;

	if (!data->charge_out) {
		if (!mxt_get_pw_status()) {
			wait_for_completion_timeout(&touch_charge_out_comp,
				MXT_CHARGE_OUT_TIMEOUT * HZ);
			if (!data->after_work)
				goto exit;

			ret = mxt_pw_lock(INCELL_DISPLAY_POWER_LOCK);
			ret = mxt_drm_resume(data);
			ret = mxt_pw_lock(INCELL_DISPLAY_POWER_UNLOCK);

			if (ret) {
				LOGE("stop session\n");
				goto exit;
			}
		} else {
			LOGN("display is not boot yet."
				" resuming by display notification callbacks\n");
		}

		data->charge_out = true;
	} else {
		LOGN("already charge outs\n");
	}

exit:
	return count;
}

static ssize_t mxt_watchdog_store(struct mxt_data *data, const char *buf, size_t count)
{
	int value = 0;

	if (kstrtoint(buf, 0, &value)) {
		LOGE("failed to read sysfs\n");
		goto exit;
	}

	if (!(data->after_work && data->charge_out)) {
		LOGE("please, set it after probe completion\n");
		goto exit;
	}

	if (mxt_get_pw_status()) {
		LOGE("please, set it with power on\n");
		goto exit;
	}

	mutex_lock(&data->mxt_drv_data->dev_lock);
	mxt_pw_lock(INCELL_DISPLAY_POWER_LOCK);

	cancel_delayed_work(&data->watchdog.work);

	if (0 < value) {
		data->watchdog.delay = value;
		LOGN("set delay watchdog:%d ms\n", value);
		schedule_delayed_work(&data->watchdog.work,
			msecs_to_jiffies(data->watchdog.delay));
		data->watchdog.supported = true;
	} else {
		LOGN("disable watchdog:%d\n", value);
		data->watchdog.supported = false;
	}

	mxt_pw_lock(INCELL_DISPLAY_POWER_UNLOCK);
	mutex_unlock(&data->mxt_drv_data->dev_lock);

exit:
	return count;
}

static ssize_t mxt_watchdog_show(struct mxt_data *data, char *buf)
{
	int len = 0;

	len += snprintf(buf + len, PAGE_SIZE - len, "support:%d delay:%d\n",
		data->watchdog.supported ? 1 : 0,
		data->watchdog.delay);

	return len;
}

static ssize_t mxt_log_level_store(struct mxt_data *data, const char *buf, size_t count)
{
	int value = 0;

	if (kstrtoint(buf, 0, &value)) {
		LOGE("failed to read sysfs\n");
		goto exit;
	}

	switch (value) {
	case LEVEL_DEFAULT:
	case LEVEL_DEBUG:
	case LEVEL_ALL:
		LOGN("register value :%d\n", value);
		data->log_level = value;
		break;
	default:
		LOGN("invalid value :%d\n", value);
		break;
	}

	LOGN("===now displaying logs===\n");
	LOGE("error\n");
	LOGW("warning\n");
	LOGN("notice\n");
	LOGI("info\n");
	LOGD("debug\n");

exit:
	return count;
}

static ssize_t mxt_log_level_show(struct mxt_data *data, char *buf)
{
	int len = 0;

	len += snprintf(buf + len, PAGE_SIZE - len, "log_level:%d\n", data->log_level);

	return len;
}

static LGE_TOUCH_ATTR(fw_version, S_IRUGO, mxt_fw_version_show, NULL);
static LGE_TOUCH_ATTR(hw_version, S_IRUGO, mxt_hw_version_show, NULL);
static LGE_TOUCH_ATTR(testmode_ver, S_IRUGO | S_IWUSR, mxt_testmode_ver_show, NULL);
static LGE_TOUCH_ATTR(version, S_IRUGO, mxt_info_show, NULL);
static LGE_TOUCH_ATTR(mxt_info, S_IRUGO, mxt_info_show, NULL);
static LGE_TOUCH_ATTR(object, S_IRUGO, mxt_object_show, NULL);
static LGE_TOUCH_ATTR(object_ctrl, S_IWUSR, NULL, mxt_object_control);
static LGE_TOUCH_ATTR(update_bin, S_IWUSR, NULL, mxt_update_bin_store);
static LGE_TOUCH_ATTR(update_raw, S_IWUSR, NULL, mxt_update_raw_store);
static LGE_TOUCH_ATTR(debug_enable, S_IWUSR | S_IRUSR, mxt_debug_enable_show, mxt_debug_enable_store);
static LGE_TOUCH_ATTR(t57_debug_enable, S_IWUSR | S_IRUSR, NULL, mxt_t57_debug_enable_store);
static LGE_TOUCH_ATTR(patch_debug_enable, S_IWUSR | S_IRUSR, mxt_patch_debug_enable_show, mxt_patch_debug_enable_store);
static LGE_TOUCH_ATTR(knock_on_type, S_IRUGO, mxt_get_knockon_type, NULL);
static LGE_TOUCH_ATTR(lpwg_data, S_IRUGO | S_IWUSR, show_lpwg_data, store_lpwg_data);
static LGE_TOUCH_ATTR(lpwg_notify, S_IRUGO | S_IWUSR, NULL, store_lpwg_notify);
static LGE_TOUCH_ATTR(chstatus, S_IRUGO, mxt_run_chstatus_show, NULL);
static LGE_TOUCH_ATTR(sd, S_IRUGO, mxt_run_self_diagnostic_show, NULL);
static LGE_TOUCH_ATTR(sd_status, S_IRUGO, mxt_run_self_diagnostic_status_show, NULL);
static LGE_TOUCH_ATTR(lpwg_sd, S_IRUGO, mxt_run_lpwg_self_diagnostic_show, NULL);
static LGE_TOUCH_ATTR(update_patch, S_IWUSR, NULL, mxt_update_patch_store);
static LGE_TOUCH_ATTR(update_fw, S_IWUSR, NULL, mxt_update_fw_store);
static LGE_TOUCH_ATTR(power_control, S_IRUGO | S_IWUSR, mxt_power_control_show, mxt_power_control_store);
static LGE_TOUCH_ATTR(global_access_pixel, S_IWUSR | S_IRUSR, mxt_global_access_pixel_show, mxt_global_access_pixel_store);
static LGE_TOUCH_ATTR(rebase, S_IWUSR | S_IRUGO, mxt_force_rebase_show, NULL);
static LGE_TOUCH_ATTR(mfts, S_IWUSR | S_IRUSR, mxt_mfts_enable_show, mxt_mfts_enable_store);
static LGE_TOUCH_ATTR(incoming_call, S_IRUGO | S_IWUSR, NULL, store_incoming_call);
static LGE_TOUCH_ATTR(keyguard, S_IRUGO | S_IWUSR, NULL, store_keyguard_info);
static LGE_TOUCH_ATTR(ime_status, S_IRUGO | S_IWUSR, NULL, store_ime_status);
static LGE_TOUCH_ATTR(self_ref_check, S_IRUGO | S_IWUSR, mxt_show_self_ref, NULL);
#ifdef CONFIG_TOUCHSCREEN_ATMEL_LPWG_DEBUG_REASON
static LGE_TOUCH_ATTR(lpwg_debug_reason, S_IRUGO | S_IWUSR, show_lpwg_debug_reason, NULL);
static LGE_TOUCH_ATTR(save_lpwg_debug_reason, S_IRUGO | S_IWUSR, show_save_lpwg_debug_reason, NULL);
#endif
#ifdef CONFIG_TOUCHSCREEN_ATMEL_STYLUS_PEN
static LGE_TOUCH_ATTR(pen_support, S_IRUGO | S_IWUSR, mxt_show_pen_support, NULL);
#endif
static LGE_TOUCH_ATTR(self_cap, S_IWUSR | S_IRUGO, mxt_self_cap_show, mxt_self_cap_store);
static LGE_TOUCH_ATTR(noise_suppression, S_IWUSR | S_IRUGO, mxt_noise_suppression_show, mxt_noise_suppression_store);
static LGE_TOUCH_ATTR(noise_recover, S_IWUSR | S_IRUGO, mxt_noise_recover_show, mxt_noise_recover_store);
static LGE_TOUCH_ATTR(mfts_lpwg, S_IWUSR | S_IRUGO, mxt_mfts_lpwg_show, mxt_mfts_lpwg_store);
static LGE_TOUCH_ATTR(firmware, S_IRUGO, mxt_info_show, NULL);
static LGE_TOUCH_ATTR(ts_noise, S_IRUGO, mxt_run_self_diagnostic_show, NULL);
static LGE_TOUCH_ATTR(glove_mode, S_IWUSR | S_IRUGO, mxt_glove_mode_show, mxt_glove_mode_store);
static LGE_TOUCH_ATTR(sidekey_status, S_IWUSR | S_IRUGO, mxt_sidekey_status_show, mxt_sidekey_status_store);
static LGE_TOUCH_ATTR(cover_mode, S_IWUSR | S_IRUGO, mxt_cover_mode_show, mxt_cover_mode_store);
static LGE_TOUCH_ATTR(cover_status, S_IWUSR | S_IRUGO, mxt_cover_status_show, mxt_cover_status_store);
static LGE_TOUCH_ATTR(stamina_mode, S_IWUSR | S_IRUGO, mxt_stamina_mode_show, mxt_stamina_mode_store);
static LGE_TOUCH_ATTR(aod_mode, S_IWUSR | S_IRUGO, mxt_aod_mode_show, mxt_aod_mode_store);
static LGE_TOUCH_ATTR(irq_enable, S_IWUSR | S_IRUSR, mxt_irq_enable_show, mxt_irq_enable_store);
static LGE_TOUCH_ATTR(pin_fault, S_IRUGO, mxt_run_pin_fault_show, NULL);
static LGE_TOUCH_ATTR(microcrack, S_IRUGO, mxt_run_microcrack_show, NULL);
static LGE_TOUCH_ATTR(reset_irq, S_IRUGO, mxt_run_reset_irq_show, NULL);
static LGE_TOUCH_ATTR(diag_data, S_IRUGO, mxt_run_diag_data_show, NULL);
static LGE_TOUCH_ATTR(charge_out, S_IWUSR, NULL, mxt_charge_out_store);
static LGE_TOUCH_ATTR(watchdog, S_IWUSR | S_IRUGO, mxt_watchdog_show, mxt_watchdog_store);
static LGE_TOUCH_ATTR(register_r, S_IWUSR | S_IRUGO, mxt_register_r_show, mxt_register_r_store);
static LGE_TOUCH_ATTR(register_w, S_IWUSR | S_IRUGO, NULL, mxt_register_w_store);
static LGE_TOUCH_ATTR(log_level, S_IWUSR | S_IRUGO, mxt_log_level_show, mxt_log_level_store);
static LGE_TOUCH_ATTR(sod_mode, S_IWUSR | S_IRUGO, NULL, mxt_sod_mode_store);
static LGE_TOUCH_ATTR(pre_sod_mode, S_IWUSR | S_IRUGO, NULL, mxt_pre_sod_mode_store);
static LGE_TOUCH_ATTR(grip_suppression, S_IWUSR | S_IRUGO, NULL, mxt_grip_suppression_landscape_store);
static LGE_TOUCH_ATTR(range_changer, S_IWUSR | S_IRUGO, NULL, mxt_range_changer_store);

static struct attribute *lge_touch_attribute_list[] = {
	&lge_touch_attr_fw_version.attr,
	&lge_touch_attr_hw_version.attr,
	&lge_touch_attr_testmode_ver.attr,
	&lge_touch_attr_version.attr,
	&lge_touch_attr_mxt_info.attr,
	&lge_touch_attr_object.attr,
	&lge_touch_attr_object_ctrl.attr,
	&lge_touch_attr_update_bin.attr,
	&lge_touch_attr_update_raw.attr,
	&lge_touch_attr_debug_enable.attr,
	&lge_touch_attr_t57_debug_enable.attr,
	&lge_touch_attr_patch_debug_enable.attr,
	&lge_touch_attr_knock_on_type.attr,
	&lge_touch_attr_lpwg_data.attr,
	&lge_touch_attr_lpwg_notify.attr,
	&lge_touch_attr_chstatus.attr,
	&lge_touch_attr_sd.attr,
	&lge_touch_attr_sd_status.attr,
	&lge_touch_attr_lpwg_sd.attr,
	&lge_touch_attr_update_patch.attr,
	&lge_touch_attr_update_fw.attr,
	&lge_touch_attr_power_control.attr,
	&lge_touch_attr_global_access_pixel.attr,
	&lge_touch_attr_rebase.attr,
	&lge_touch_attr_mfts.attr,
	&lge_touch_attr_incoming_call.attr,
	&lge_touch_attr_keyguard.attr,
	&lge_touch_attr_ime_status.attr,
	&lge_touch_attr_self_ref_check.attr,
#ifdef CONFIG_TOUCHSCREEN_ATMEL_STYLUS_PEN
	&lge_touch_attr_pen_support.attr,
#endif
#ifdef CONFIG_TOUCHSCREEN_ATMEL_LPWG_DEBUG_REASON
	&lge_touch_attr_lpwg_debug_reason.attr,
	&lge_touch_attr_save_lpwg_debug_reason.attr,
#endif
	&lge_touch_attr_self_cap.attr,
	&lge_touch_attr_noise_suppression.attr,
	&lge_touch_attr_noise_recover.attr,
	&lge_touch_attr_mfts_lpwg.attr,
	&lge_touch_attr_firmware.attr,
	&lge_touch_attr_ts_noise.attr,
	&lge_touch_attr_glove_mode.attr,
	&lge_touch_attr_sidekey_status.attr,
	&lge_touch_attr_cover_mode.attr,
	&lge_touch_attr_cover_status.attr,
	&lge_touch_attr_stamina_mode.attr,
	&lge_touch_attr_aod_mode.attr,
	&lge_touch_attr_irq_enable.attr,
	&lge_touch_attr_pin_fault.attr,
	&lge_touch_attr_microcrack.attr,
	&lge_touch_attr_reset_irq.attr,
	&lge_touch_attr_diag_data.attr,
	&lge_touch_attr_charge_out.attr,
	&lge_touch_attr_watchdog.attr,
	&lge_touch_attr_register_r.attr,
	&lge_touch_attr_register_w.attr,
	&lge_touch_attr_log_level.attr,
	&lge_touch_attr_sod_mode.attr,
	&lge_touch_attr_pre_sod_mode.attr,
	&lge_touch_attr_grip_suppression.attr,
	&lge_touch_attr_range_changer.attr,
	NULL
};

static ssize_t lge_touch_attr_show(struct kobject *lge_touch_kobj, struct attribute *attr, char *buf)
{
	struct mxt_data *ts = container_of(lge_touch_kobj, struct mxt_data, lge_touch_kobj);
	struct lge_touch_attribute *lge_touch_priv = container_of(attr, struct lge_touch_attribute, attr);
	ssize_t ret = 0;

	if (lge_touch_priv->show)
		ret = lge_touch_priv->show(ts, buf);

	return ret;
}

static ssize_t lge_touch_attr_store(struct kobject *lge_touch_kobj, struct attribute *attr, const char *buf, size_t count)
{
	struct mxt_data *ts = container_of(lge_touch_kobj, struct mxt_data, lge_touch_kobj);
	struct lge_touch_attribute *lge_touch_priv = container_of(attr, struct lge_touch_attribute, attr);
	ssize_t ret = 0;

	if (lge_touch_priv->store)
		ret = lge_touch_priv->store(ts, buf, count);

	return ret;
}

static const struct sysfs_ops lge_touch_sysfs_ops = {
	.show = lge_touch_attr_show,
	.store = lge_touch_attr_store,
};

static struct kobj_type lge_touch_kobj_type = {
	.sysfs_ops = &lge_touch_sysfs_ops,
	.default_attrs = lge_touch_attribute_list,
};

static void mxt_reset_slots(struct mxt_data *data)
{
	struct input_dev *input_dev = data->input_dev;
	int id = 0;

#if SIDE_KEY_DEVICE
	input_dev = data->input_dev_touch;
#endif

	for (id = 0; id < data->pdata->numtouch; id++) {
		input_mt_slot(input_dev, id);
		input_mt_report_slot_state(input_dev, MT_TOOL_FINGER, FINGER_RELEASE);
	}

	mxt_input_sync(input_dev);
	LOGN("Release all event\n");

#if SIDE_KEY_DEVICE
	if (data->input_dev_sidekey) {
		input_dev = data->input_dev_sidekey;

		for (id = 0; id < data->pdata->numtouch; id++) {
			input_mt_slot(input_dev, id);
			input_mt_report_slot_state(input_dev, MT_TOOL_FINGER, FINGER_RELEASE);
		}

		mxt_input_sync(input_dev);
		LOGN("Release all side key\n");
	}
#endif

	memset(data->ts_data.prev_data, 0x0, sizeof(data->ts_data.prev_data));
	memset(data->ts_data.curr_data, 0x0, sizeof(data->ts_data.curr_data));
	data->mxt_drv_data->touched_finger_count = 0;
	data->button_lock = false;
	data->palm = false;
}

static void mxt_gesture_mode_start(struct mxt_data *data)
{
	struct mxt_object *object;
	int error = 0;

	object = mxt_get_object(data, MXT_PROCI_TOUCH_SEQUENCE_LOGGER_T93);
	if (!object || error)
		return;

	mxt_patchevent_set(data, PATCH_EVENT_KNOCKON);

	if (data->lpwg_mode == LPWG_DOUBLE_TAP) {
		if (mxt_patchevent_get(data, PATCH_EVENT_TA)) {
			data->knock_on_mode = CHARGER_KNOCKON_SLEEP;
		} else {
			data->knock_on_mode = NOCHARGER_KNOCKON_SLEEP;
		}
	} else if (data->lpwg_mode == LPWG_MULTI_TAP) {
		if (mxt_patchevent_get(data, PATCH_EVENT_TA)) {
			data->knock_on_mode = CHARGER_KNOCKON_SLEEP + PATCH_EVENT_PAIR_NUM;
		} else {
			data->knock_on_mode = NOCHARGER_KNOCKON_SLEEP + PATCH_EVENT_PAIR_NUM;
		}
		/* Write TCHCNTTHR(Touch Count Threshold) for LPWG_MULTI_TAP */
		error = mxt_write_reg(data->client, object->start_address+17, data->g_tap_cnt);
		if (error)
			LOGE("Object Write Fail\n");

	}

	if(data->mfts_lpwg) {
		if (mxt_patchevent_get(data, PATCH_EVENT_TA)) {
			data->knock_on_mode = CHARGER_KNOCKON_SLEEP;
		} else {
			data->knock_on_mode = NOCHARGER_KNOCKON_SLEEP;
		}
	}
}

static void mxt_active_mode_start(struct mxt_data *data)
{

	struct mxt_object *object;
	int error = 0;

	object = mxt_get_object(data, MXT_PROCI_TOUCH_SEQUENCE_LOGGER_T93);
	if (!object || error)
		return;

	if (data->mxt_knock_on_enable || data->lpwg_mode == LPWG_DOUBLE_TAP){
		if (mxt_patchevent_get(data, PATCH_EVENT_TA)) {
			data->knock_on_mode = CHARGER_KNOCKON_WAKEUP;
		} else {
			data->knock_on_mode = NOCHARGER_KNOCKON_WAKEUP;
		}
	} else if (data->mxt_multi_tap_enable || data->lpwg_mode == LPWG_MULTI_TAP) {
		if (mxt_patchevent_get(data, PATCH_EVENT_TA)) {
			data->knock_on_mode = CHARGER_KNOCKON_WAKEUP + PATCH_EVENT_PAIR_NUM;
		} else {
			data->knock_on_mode = NOCHARGER_KNOCKON_WAKEUP + PATCH_EVENT_PAIR_NUM;
		}
		/* Write TCHCNTTHR(Touch Count Threshold) for LPWG_MULTI_TAP */
		error = mxt_write_reg(data->client, object->start_address+17, data->g_tap_cnt);
		if (error)
			LOGE("Object Write Fail\n");
	}

	if(data->mfts_lpwg) {
		if (mxt_patchevent_get(data, PATCH_EVENT_TA)) {
			data->knock_on_mode = CHARGER_KNOCKON_WAKEUP;
		} else {
			data->knock_on_mode = NOCHARGER_KNOCKON_WAKEUP;
		}
	}

	mxt_patchevent_unset(data, PATCH_EVENT_KNOCKON);
}

static void mxt_start(struct mxt_data *data)
{
	LOGN("start %s\n", __func__);
	if (!data->suspended || data->in_bootloader) {
		if (data->regulator_status == 1 && !data->in_bootloader) {
			LOGD("%s suspended flag is false. Calibration after System Shutdown.\n", __func__);
			mxt_t6_command(data, MXT_COMMAND_CALIBRATE, 1, false);
		}
		return;
	}

	LOGN("%s\n", __func__);

	touch_disable_irq(data);

	if (!data->lpwg_mode && !data->mfts_enable) {
		mxt_regulator_enable(data);
	} else {
		LOGD("%s : After Quick Cover Opened.\n", __func__);
		mxt_regulator_enable(data);
	}

#ifdef CONFIG_TOUCHSCREEN_ATMEL_LPWG_DEBUG_REASON
	if (data->lpwg_mode) {
		save_lpwg_debug_reason_print(data);
	}
#endif

	mxt_active_mode_start(data);
	data->delayed_cal = true;

#ifdef MXT_FACTORY
	if (data->mxt_drv_data->factorymode) {
		mxt_patch_event(data, PATCH_EVENT_AAT);
	}
#endif
	if (data->lpwg_mode) {
		int temp_mode = 0;
		temp_mode = check_current_temp_mode(data, data->mxt_drv_data->touch_current_temp);
		select_temp_event_knock_on(data, temp_mode, 0);
	}

	mxt_reset_slots(data);
	data->suspended = false;
	data->button_lock = false;
	touch_enable_irq(data);
}

static void mxt_stop(struct mxt_data *data)
{
	if (data->suspended || data->in_bootloader)
		return;

	LOGN("%s\n", __func__);

	touch_disable_irq(data);
	touch_disable_irq_wake(data);

	if (data->lpwg_mode || data->mfts_lpwg) {
		mxt_gesture_mode_start(data);
		mxt_t6_command(data, MXT_COMMAND_CALIBRATE, 1, false);
	} else {
		LOGN("%s MXT_POWER_CFG_DEEPSLEEP\n", __func__);
		mxt_regulator_disable(data);
	}

	mxt_reset_slots(data);
	data->suspended = true;
	data->button_lock = false;

	if (data->mfts_lpwg) {
		int temp_mode = 0;
		temp_mode = check_current_temp_mode(data, data->mxt_drv_data->touch_current_temp);
		select_temp_event_knock_on(data, temp_mode, 1);

		touch_enable_irq(data);
		touch_enable_irq_wake(data);
	}
}

static int mxt_input_open(struct input_dev *dev)
{
	struct mxt_data *data = input_get_drvdata(dev);
	if (!data->mfts_enable) {
		mxt_start(data);
		data->enable_reporting = true;
	}

	return 0;
}

void change_temp_patch_event(struct mxt_data *data, int patch_event)
{
	struct mxt_data *mxt_data = data->mxt_drv_data->mxt_data;
	if (mxt_data != NULL) {
		LOGN("%s patch event = %d\n", __func__, patch_event);
		switch (patch_event) {
			case HIGH_TEMP_SET: /*13 or 19*/
				if (mxt_data->suspended)
					mxt_patch_event(mxt_data, HIGH_TEMP_KNOCKON_SET);
				else
					mxt_patch_event(mxt_data, HIGH_TEMP_KNOCKON_UNSET);
				break;
			case HIGH_TEMP_UNSET: /*14 or 20*/
				break;
			case LOW_TEMP_SET: /*17*/
				mxt_patch_event(mxt_data, LOW_TEMP_SET);
				if (mxt_data->suspended)
					mxt_patch_event(mxt_data, LOW_TEMP_KNOCKON_SET);
				else
					mxt_patch_event(mxt_data, LOW_TEMP_KNOCKON_UNSET);
				break;
			case LOW_TEMP_UNSET: /*18*/
				mxt_patch_event(mxt_data, LOW_TEMP_UNSET);
				break;
			case LOW_TEMP_KNOCKON_SET: /*19*/
				mxt_patch_event(mxt_data, LOW_TEMP_KNOCKON_SET);
				break;
			case LOW_TEMP_KNOCKON_UNSET: /*20*/
				mxt_patch_event(mxt_data, LOW_TEMP_KNOCKON_UNSET);
				break;
			case HIGH_TEMP_KNOCKON_SET: /*21*/
				mxt_patch_event(mxt_data, HIGH_TEMP_KNOCKON_SET);
				break;
			case HIGH_TEMP_KNOCKON_UNSET: /*22*/
				mxt_patch_event(mxt_data, HIGH_TEMP_KNOCKON_UNSET);
				break;
			default:
				break;
		}
	}
}

int check_current_temp_mode(struct mxt_data *data, int current_temp)
{
	if (current_temp >= HIGH_TEMP_SET_LEVEL) {
		data->mxt_drv_data->temp_mode = HIGH_TEMP_MODE;
	} else if (current_temp <= LOW_TEMP_SET_LEVEL) {
		data->mxt_drv_data->temp_mode = LOW_TEMP_MODE;
	} else {
		if (data->mxt_drv_data->temp_mode == HIGH_TEMP_MODE) {
			if (current_temp > (HIGH_TEMP_SET_LEVEL-5))
				data->mxt_drv_data->temp_mode = HIGH_TEMP_MODE;
			else
				data->mxt_drv_data->temp_mode = NORMAL_TEMP_MODE;
		}
		else if (data->mxt_drv_data->temp_mode == LOW_TEMP_MODE) {
			if (current_temp < (LOW_TEMP_SET_LEVEL+5))
				data->mxt_drv_data->temp_mode = LOW_TEMP_MODE;
			else
				data->mxt_drv_data->temp_mode = NORMAL_TEMP_MODE;
		}
		else
			data->mxt_drv_data->temp_mode = NORMAL_TEMP_MODE;
	}

	return data->mxt_drv_data->temp_mode;
}

void select_temp_event(struct mxt_data *data, int temp_mode)
{
	int patch_event = 0;

	if (data->mxt_drv_data->touch_previous_temp_mode == NORMAL_TEMP_MODE
			&& temp_mode == HIGH_TEMP_MODE) {
		patch_event = HIGH_TEMP_SET;
	} else if (data->mxt_drv_data->touch_previous_temp_mode == NORMAL_TEMP_MODE
			&& temp_mode == LOW_TEMP_MODE) {
		patch_event = LOW_TEMP_SET;
	} else if (data->mxt_drv_data->touch_previous_temp_mode == HIGH_TEMP_MODE
			&& temp_mode == NORMAL_TEMP_MODE) {
		patch_event = HIGH_TEMP_UNSET;
	} else if (data->mxt_drv_data->touch_previous_temp_mode == LOW_TEMP_MODE
			&& temp_mode == NORMAL_TEMP_MODE) {
		patch_event = LOW_TEMP_UNSET;
	}
	if (patch_event)
		change_temp_patch_event(data, patch_event);

	data->mxt_drv_data->touch_previous_temp_mode = temp_mode;
	LOGN("%s temp_mode = %d, patch_event = %d\n", __func__, temp_mode, patch_event);
}

void select_temp_event_knock_on(struct mxt_data *data, int temp_mode, int lpwg_mode)
{
	int patch_event = 0;

	switch (temp_mode) {
		case LOW_TEMP_MODE:
			if (lpwg_mode) {
				patch_event = LOW_TEMP_KNOCKON_SET;
			} else {
				patch_event = LOW_TEMP_KNOCKON_UNSET;
			}
			break;
		case HIGH_TEMP_MODE:
			if (lpwg_mode) {
				patch_event = HIGH_TEMP_KNOCKON_SET;
			} else {
				patch_event = HIGH_TEMP_KNOCKON_UNSET;
			}
			break;
		default:
			break;
	}
	if (patch_event)
		change_temp_patch_event(data, patch_event);

	LOGN("%s temp_mode = %d, lpwg_mode = %d,  patch_event = %d\n", __func__, temp_mode, lpwg_mode, patch_event);
}

void trigger_touch_temp(struct mxt_data *data, int temp)
{
	bool is_changed = false;

	LOGN("%s temp = %d\n", __func__, temp);

	if (data->mxt_drv_data->touch_current_temp != temp)
		is_changed = true;

	if (is_changed) {
		data->mxt_drv_data->temp_mode = check_current_temp_mode(data, temp);
		select_temp_event(data, data->mxt_drv_data->temp_mode);
		data->mxt_drv_data->touch_current_temp = temp;
	}
}

static void mxt_input_close(struct input_dev *dev)
{
	struct mxt_data *data = input_get_drvdata(dev);

	mxt_stop(data);
}

static int mxt_parse_dt_feature(struct device *dev, struct mxt_data *data)
{
	struct device_node *node = dev->of_node;
	int rc = 0;
	u32 temp_val = 0;

	/* reset, irq gpio info */
	if (node == NULL)
		return -ENODEV;

	/* watchdog */
	rc = of_property_read_u32(node,
		"atmel,watchdog_supported", &temp_val);
	if (rc) {
		LOGE("DT : Unable to read watchdog_supported\n");
		data->watchdog.supported = false;
	} else {
		data->watchdog.supported = temp_val ? true : false;
		LOGD("DT : watchdog_supported: %s\n",
			data->watchdog.supported ? "true" : "false");

		/* delay */
		rc = of_property_read_u32(node,
			"atmel,watchdog_delay_ms", &temp_val);
		if (rc) {
			LOGE(
				"DT : Unable to read watchdog_delay_ms\n");
			data->watchdog.delay = 0;
			data->watchdog.supported = false;
		} else {
			data->watchdog.delay = temp_val;
			LOGD("DT : watchdog_delay_ms: %d\n",
				data->watchdog.delay);
		}
	}

	rc = of_property_read_u32(node, "atmel,side_key_supported", &temp_val);
	if (rc) {
		LOGE("DT : Unable to read side_key_supported\n");
		data->side_key.supported = false;
		data->side_key.status = SIDE_KEY_DISABLE;
	} else {
		data->side_key.supported = temp_val ? true : false;
		if (data->side_key.supported)
			data->side_key.status = MXT_DEFAULT_SIDEKEY_STATUS;
		else
			data->side_key.status = SIDE_KEY_DISABLE;
		LOGD("DT : side_key_supported: %s\n",
			data->side_key.supported ? "true" : "false");
	}

	rc = of_property_read_u32(node, "atmel,glove_mode_supported", &temp_val);
	if (rc) {
		LOGE("DT : Unable to read glove_mode_supported\n");
		data->glove_mode.supported = false;
	} else {
		data->glove_mode.supported = temp_val ? true : false;
		LOGD("DT : glove_mode_supported: %s\n",
			data->glove_mode.supported ? "true" : "false");
	}

	rc = of_property_read_u32(node, "atmel,cover_mode_supported", &temp_val);
	if (rc) {
		LOGE("DT : Unable to read cover_mode_supported\n");
		data->cover_mode.supported = false;
	} else {
		data->cover_mode.supported = temp_val ? true : false;
		LOGD("DT : cover_mode_supported: %s\n",
			data->cover_mode.supported ? "true" : "false");
	}

	rc = of_property_read_u32(node, "atmel,aod_mode_supported", &temp_val);
	if (rc) {
		LOGE("DT : Unable to read aod_mode_supported\n");
		data->aod_mode.supported = false;
	} else {
		data->aod_mode.supported = temp_val ? true : false;
		LOGD("DT : aod_mode_supported: %s\n",
			data->aod_mode.supported ? "true" : "false");
	}

	rc = of_property_read_u32(node, "atmel,stamina_mode_supported", &temp_val);
	if (rc) {
		LOGE("DT : Unable to read stamina_mode_supported\n");
		data->stamina_mode.supported = false;
	} else {
		data->stamina_mode.supported = temp_val ? true : false;
		LOGD("DT : stamina_mode_supported: %s\n",
			data->stamina_mode.supported ? "true" : "false");
	}

	rc = of_property_read_u32(node, "atmel,sod_mode_supported", &temp_val);
	if (rc) {
		LOGE("DT : Unable to read sod_mode_supported\n");
		data->sod_mode.supported = false;
	} else {
		data->sod_mode.supported = temp_val ? true : false;
		LOGD("DT : sod_mode_supported: %s\n",
			data->sod_mode.supported ? "true" : "false");
	}

	return 0;
}

static int mxt_parse_dt(struct device *dev, struct mxt_platform_data *pdata)
{
	struct device_node *node = dev->of_node;
	int rc = 0;
	u32 temp_val = 0;
#if 0
	pdata->avdd = regulator_get(dev, "avdd");
#endif

	/* reset, irq gpio info */
	if (node == NULL)
		return -ENODEV;
#if 0
	pdata->gpio_vddio = of_get_named_gpio_flags(node, "atmel,vddio-gpio", 0, NULL);

	if (pdata->gpio_vddio)
		LOGD("DT : gpio_vddio = %lu\n", pdata->gpio_vddio);
	else
		LOGE("DT get gpio_vddio error\n");
#endif

	pdata->gpio_reset = of_get_named_gpio_flags(node, "atmel,reset-gpio", 0, NULL);
	if (pdata->gpio_reset)
		LOGD("DT : gpio_reset = %lu\n", pdata->gpio_reset);
	else
		LOGE("DT get gpio_reset error\n");

	pdata->gpio_int = of_get_named_gpio_flags(node, "atmel,irq-gpio", 0, NULL);
	if (pdata->gpio_int)
		LOGD("DT : gpio_int = %lu\n", pdata->gpio_int);
	else
		LOGE("DT get gpio_int error\n");

	rc = of_property_read_u32(node, "atmel,numtouch", &temp_val);
	if (rc) {
		LOGE("DT : Unable to read numtouch\n");
	} else {
		pdata->numtouch = temp_val;
		LOGD("DT : numtouch = %d\n", pdata->numtouch);
	}

	rc = of_property_read_string(node, "atmel,fw_name",  &pdata->fw_name);
	if (rc) {
		LOGE("DT : atmel,fw_name error\n");
		pdata->fw_name = NULL;
	} else {
		LOGD("DT : fw_name : %s\n", pdata->fw_name);
	}

	rc = of_property_read_u32(node, "atmel,knock_on_type",  &temp_val);
	if (rc) {
		LOGE("DT : Unable to read knock_on_type - set as 0\n" );
		pdata->knock_on_type = 0;
	} else {
		pdata->knock_on_type = temp_val;
		LOGD("DT : knock_on_type = %d\n",pdata->knock_on_type);
	}

	rc = of_property_read_u32(node, "atmel,global_access_pixel",  &temp_val);
	if (rc) {
		LOGE("DT : Unable to read global_access_pixel - set as 0\n" );
		pdata->global_access_pixel = 0;
	} else {
		pdata->global_access_pixel = temp_val;
		LOGD("DT : global_access_pixel = %d\n",pdata->global_access_pixel);
	}

	rc = of_property_read_u32(node, "atmel,use_mfts",  &temp_val);
	if (rc) {
		LOGE("DT : Unable to read use_mfts - set as false\n" );
		pdata->use_mfts = 0;
	} else {
		pdata->use_mfts = temp_val;
		LOGD("DT : use_mfts = %d\n",pdata->use_mfts);
	}

	rc = of_property_read_u32(node, "atmel,lcd_x", &temp_val);
	if (rc) {
		LOGE( "DT : Unable to read lcd_x\n");
		pdata->lcd_x = 540;
	} else {
		pdata->lcd_x = temp_val;
		LOGD("DT : lcd_x: %d\n",pdata->lcd_x);
	}

	rc = of_property_read_u32(node, "atmel,lcd_y", &temp_val);
	if (rc) {
		LOGE( "DT : Unable to read lcd_y\n");
		pdata->lcd_y = 960;
	} else {
		pdata->lcd_y = temp_val;
		LOGD("DT : lcd_y: %d\n",pdata->lcd_y);
	}

	rc = of_property_read_u32(node, "atmel,butt_check_enable", &temp_val);
	if (rc) {
		LOGE( "DT : Unable to read butt_check_enable\n");
		pdata->butt_check_enable = 0;
	} else {
		pdata->butt_check_enable = temp_val;
		LOGD("DT : butt_check_enable: %d\n",pdata->butt_check_enable);
	}

#ifdef CONFIG_TOUCHSCREEN_ATMEL_LPWG_DEBUG_REASON
	rc = of_property_read_u32(node, "atmel,use_debug_reason", &temp_val);
	if (rc) {
		LOGE( "DT : Unable to read use_debug_reason\n");
		pdata->use_debug_reason = 0;
	} else {
		pdata->use_debug_reason = temp_val;
		LOGD("DT : use_debug_reason: %d\n",pdata->use_debug_reason);
	}
#endif
	rc = of_property_read_u32(node, "atmel,use_lpwg_test", &temp_val);
	if (rc) {
		LOGE( "DT : Unable to read use_lpwg_test\n");
		pdata->lpwg_test = 0;
	} else {
		pdata->lpwg_test = temp_val;
		LOGD("DT : use_lpwg_test: %d\n",pdata->lpwg_test);
	}

	return 0;
}

static int mxt_parse_from_dt(struct device *dev, struct mxt_data *data)
{
	int ret = 0;

	ret = mxt_parse_dt(dev, data->pdata);
	if (ret)
		goto exit;

	ret = mxt_parse_dt_feature(dev, data);
	if (ret)
		goto exit;

exit:
	return ret;
}

static int mxt_read_t100_config(struct mxt_data *data)
{
	struct i2c_client *client = data->client;
	int error;
	struct mxt_object *object;
	u16 range_x = 0, range_y = 0;
	u8 cfg = 0, tchaux = 0;
	u8 aux = 0;

	object = mxt_get_object(data, MXT_TOUCH_MULTITOUCHSCREEN_T100);
	if (!object)
		return -EINVAL;

	error = __mxt_read_reg(client,
		object->start_address + MXT_T100_XRANGE,
		(u16)sizeof(range_x), &range_x);
	if (error)
		return error;

	le16_to_cpus(range_x);

	error = __mxt_read_reg(client,
		object->start_address + MXT_T100_YRANGE,
		(u16)sizeof(range_y), &range_y);
	if (error)
		return error;

	le16_to_cpus(range_y);

	error =  __mxt_read_reg(client,
				object->start_address + MXT_T100_CFG1,
				1, &cfg);
	if (error)
		return error;
	error =  __mxt_read_reg(client,
				object->start_address + MXT_T100_TCHAUX,
				1, &tchaux);
	if (error)
		return error;

	/* Handle default values */
	if (range_x == 0)
		range_x = 1023;

	/* Handle default values */
	if (range_x == 0)
		range_x = 1023;

	if (range_y == 0)
		range_y = 1023;

	if (cfg & MXT_T100_CFG_SWITCHXY) {
		data->max_x = range_y;
		data->max_y = range_x;
	} else {
		data->max_x = range_x;
		data->max_y = range_y;
	}

	/* allocate aux bytes */
	aux = 6;

	if (tchaux & MXT_T100_TCHAUX_VECT)
		data->t100_aux_vect = aux++;

	if (tchaux & MXT_T100_TCHAUX_AMPL)
		data->t100_aux_ampl = aux++;
	else
		data->t100_aux_ampl = aux;

	if (tchaux & MXT_T100_TCHAUX_AREA)
		data->t100_aux_area = aux++;

	if (tchaux & MXT_T100_TCHAUX_RESV)
		data->t100_aux_resv = aux++;

	LOGD("T100 Touchscreen size X%uY%u\n", data->max_x, data->max_y);

	return 0;
}

int mxt_initialize_t100_input_device(struct mxt_data *data)
{
	struct input_dev *input_dev;
	int error = 0;

	if (data->T100_reportid_min)
		error = mxt_read_t100_config(data);
	if (error)
		LOGE("failed to initialize T100 resolution\n");

	if (!data->input_dev) {
		LOGN("alloc input device\n");
		input_dev = input_allocate_device();

	} else {
		input_dev = data->input_dev;
	}

	if (!data) {
		LOGE("mxt_data is NULL\n");
		if (input_dev) {
			input_free_device(input_dev);
		}
		return -ENOMEM;
	}

	if (!input_dev) {
		LOGE("Input_dev is failed to allocate memory\n");
		return -ENOMEM;
	}

	input_dev->name = MXT_DEVICE_NAME;

	input_dev->phys = data->phys;
	input_dev->id.bustype = BUS_I2C;

	__set_bit(EV_ABS, input_dev->evbit);
	__set_bit(INPUT_PROP_DIRECT, input_dev->propbit);

	if (data->input_dev) {
		/* For multi touch */
		error = input_mt_init_slots(input_dev, data->num_touchids, 0);
		if (error) {
			LOGE("Error %d initialising slots\n", error);
			goto err_free_mem;
		}
	}
	input_set_abs_params(input_dev, ABS_MT_TRACKING_ID, 0, data->pdata->numtouch, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_WIDTH_MAJOR, 0, MXT_MAX_AREA, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_WIDTH_MINOR, 0, MXT_MAX_AREA, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_POSITION_X, 0, data->max_x, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_POSITION_Y, 0, data->max_y, 0, 0);

	if (data->t100_aux_area)
		input_set_abs_params(input_dev, ABS_MT_TOUCH_MAJOR, 0, MXT_MAX_AREA, 0, 0);

	if (data->t100_aux_ampl)
		input_set_abs_params(input_dev, ABS_MT_PRESSURE, 0, MAX_PRESSURE, 0, 0);

	if (data->t100_aux_vect)
		input_set_abs_params(input_dev, ABS_MT_ORIENTATION, 0, 255, 0, 0);

	if (!data->input_dev) {
		LOGN("registoring input device\n");
		input_set_drvdata(input_dev, data);

		error = input_register_device(input_dev);
		if (error) {
			LOGE("Error %d registering input device\n", error);
			goto err_free_mem;
		}
	}
#if SIDE_KEY_DEVICE
	if (!data->input_dev) {
		LOGN("skip sidekey register\n");
		goto skip_sidekey;
	}

	data->input_dev_touch = data->input_dev;
	if (!data->input_dev_sidekey) {
		data->input_dev_sidekey = input_allocate_device();
		if (!data->input_dev_sidekey) {
			LOGE("Error input_allocate_device - input_dev_sidekey\n");
			goto err_free_sidekey;
		}
		data->input_dev_sidekey->name = SIDE_KEY_DEVICE_NAME;
		__set_bit(EV_ABS, data->input_dev_sidekey->evbit);
		__set_bit(INPUT_PROP_DIRECT, data->input_dev_sidekey->propbit);

		error = input_mt_init_slots(data->input_dev_sidekey, data->num_touchids, 0);
		if (error) {
			LOGE("Error %d initialising slots - input_dev_sidekey\n", error);
			goto err_free_sidekey;
		}

		input_set_abs_params(data->input_dev_sidekey, ABS_MT_WIDTH_MAJOR, 0, MXT_MAX_AREA, 0, 0);
		input_set_abs_params(data->input_dev_sidekey, ABS_MT_WIDTH_MINOR, 0, MXT_MAX_AREA, 0, 0);
		input_set_abs_params(data->input_dev_sidekey, ABS_MT_POSITION_X, 0, data->max_x, 0, 0);
		input_set_abs_params(data->input_dev_sidekey, ABS_MT_POSITION_Y, 0, data->max_y, 0, 0);

		input_set_drvdata(data->input_dev_sidekey, data);

		error = input_register_device(data->input_dev_sidekey);
		if (error) {
			LOGE("Error %d registering input device - input_dev_sidekey\n", error);
			goto err_free_sidekey;
		}
	}

#endif
skip_sidekey:
	data->input_dev = input_dev;
	return 0;

err_free_mem:
	input_free_device(input_dev);
err_free_sidekey:
	if (data->input_dev_sidekey)
		input_free_device(data->input_dev_sidekey);

	return error;
}

int mxt_initialize_t9_input_device(struct mxt_data *data)
{
	struct input_dev *input_dev = NULL;
	int error = 0;
	unsigned int num_mt_slots = 0;
	int i = 0;

	if (data->input_dev && !data->mxt_drv_data->is_probing) {
		LOGE("ignore %s \n", __func__);
		return 0;
	}

	error = mxt_read_t9_resolution(data);
	if (error)
		LOGE("Failed to initialize T9 resolution\n");

	input_dev = input_allocate_device();
	if (!input_dev) {
		LOGE("Failed to allocate memory\n");
		return -ENOMEM;
	}

	input_dev->name = MXT_DEVICE_NAME;
	input_dev->phys = data->phys;
	input_dev->id.bustype = BUS_I2C;
	input_dev->open = mxt_input_open;
	input_dev->close = mxt_input_close;

	__set_bit(EV_ABS, input_dev->evbit);
	input_set_capability(input_dev, EV_KEY, BTN_TOUCH);

	num_mt_slots = data->num_touchids;
	error = input_mt_init_slots(input_dev, num_mt_slots, 0);
	if (error) {
		LOGE("Error %d initialising slots\n", error);
		goto err_free_mem;
	}

	input_set_abs_params(input_dev, ABS_MT_TOUCH_MAJOR, 0, MXT_MAX_AREA, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_POSITION_X, 0, data->max_x, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_POSITION_Y, 0, data->max_y, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_PRESSURE, 0, MAX_PRESSURE, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_ORIENTATION, 0, 255, 0, 0);

	/* For T15 key array */
	if (data->T15_reportid_min) {
		data->t15_keystatus = 0;

		input_dev->evbit[0] = BIT_MASK(EV_ABS) | BIT_MASK(EV_KEY);

		for (i = 0; i < data->pdata->t15_num_keys; i++)
			input_set_capability(input_dev, EV_KEY, data->pdata->t15_keymap[i]);
			input_dev->keybit[BIT_WORD(data->pdata->t15_keymap[i])] |= BIT_MASK(data->pdata->t15_keymap[i]);
	}

	input_set_drvdata(input_dev, data);

	error = input_register_device(input_dev);
	if (error) {
		LOGE("Error %d registering input device\n", error);
		goto err_free_mem;
	} else {
		LOGN("input_register_device done\n");
	}

	data->input_dev = input_dev;

	return 0;

err_free_mem:
	input_free_device(input_dev);
	return error;
}

static int mxt_command_reset(struct mxt_data *data, u8 value)
{
	int error = 0;

	error = mxt_write_object(data, MXT_GEN_COMMAND_T6, MXT_COMMAND_RESET, value);
	msleep(MXT_RESET_TIME);

	if (error)
		LOGE("Not respond after reset command[%d]\n", value);

	return error;
}

static int mxt_command_backup(struct mxt_data *data, u8 value)
{
	mxt_write_object(data, MXT_GEN_COMMAND_T6,MXT_COMMAND_BACKUPNV, value);
	msleep(MXT_BACKUP_TIME);

	return 0;
}

int mxt_read_mem(struct mxt_data *data, u16 reg, u16 len, void *buf)
{
	int ret = 0;

	ret = __mxt_read_reg(data->client, reg, len, buf);

	return ret;
}

int mxt_write_mem(struct mxt_data *data, u16 reg, u16 len, const u8 *buf)
{
	int ret = 0;

	ret = __mxt_write_reg(data->client, reg, len, buf);

	return ret;
}

int mxt_verify_fw(struct mxt_fw_info *fw_info, const struct firmware *fw)
{
	struct mxt_data *data = fw_info->data;
	struct mxt_fw_image *fw_img = NULL;
	char *extra_info = NULL;
	struct patch_header* ppheader = NULL;
	u8* patch = NULL;
	u32 ppos = 0;

	if (!fw) {
		LOGE("could not find firmware file\n");
		return -ENOENT;
	}

	fw_img = (struct mxt_fw_image *)fw->data;

	if (le32_to_cpu(fw_img->magic_code) != MXT_FW_MAGIC) {
		/* In case, firmware file only consist of firmware */
		LOGE("Firmware file only consist of raw firmware\n");
		fw_info->fw_len = fw->size;
		fw_info->fw_raw_data = fw->data;
	} else {
		/*
		 * In case, firmware file consist of header,
		 * configuration, firmware.
		 */
		LOGN("Firmware file consist of header, configuration, firmware\n");
		fw_info->bin_ver = fw_img->bin_ver;
		fw_info->build_ver = fw_img->build_ver;
		fw_info->hdr_len = le32_to_cpu(fw_img->hdr_len);
		fw_info->cfg_len = le32_to_cpu(fw_img->cfg_len);
		fw_info->fw_len = le32_to_cpu(fw_img->fw_len);
		fw_info->cfg_crc = le32_to_cpu(fw_img->cfg_crc);

		extra_info = fw_img->extra_info;
		fw_info->data->pdata->fw_ver[0] = extra_info[0];
		fw_info->data->pdata->fw_ver[1] = extra_info[1];
		fw_info->data->pdata->fw_ver[2] = extra_info[2];
		memcpy(fw_info->data->pdata->product, &extra_info[4], 10);

		/* Check the firmware file with header */
		if (fw_info->hdr_len != sizeof(struct mxt_fw_image)
			|| fw_info->hdr_len + fw_info->cfg_len + fw_info->fw_len != fw->size) {

			ppos = fw_info->hdr_len + fw_info->cfg_len + fw_info->fw_len;
			ppheader = (struct patch_header*)(fw->data + ppos);
			if (ppheader->magic == MXT_PATCH_MAGIC) {
				LOGN("Firmware file has patch size: %d\n", ppheader->size);
				if (ppheader->size) {
					patch = NULL;
					if (data->patch.patch) {
						kfree(data->patch.patch);
						data->patch.patch = NULL;
					}
					patch = kzalloc(ppheader->size, GFP_KERNEL);
					if (patch) {
						memcpy(patch, (u8*)ppheader, ppheader->size);
						data->patch.patch = patch;
						LOGN("%s Patch Updated\n", __func__);
					} else {
						data->patch.patch = NULL;
						LOGE("%s Patch Update Failed\n", __func__);
					}
				}
			} else {
				LOGE("Firmware file is invaild !!hdr size[%d] cfg,fw size[%d,%d] filesize[%zu]\n",
					fw_info->hdr_len, fw_info->cfg_len, fw_info->fw_len, fw->size);
				return -EINVAL;
			}
		}

		if (!fw_info->cfg_len) {
			LOGE("Firmware file dose not include configuration data\n");
			return -EINVAL;
		}

		if (!fw_info->fw_len) {
			LOGE("Firmware file dose not include raw firmware data\n");
			return -EINVAL;
		}

		/* Get the address of configuration data */
		fw_info->cfg_raw_data = fw_img->data;

		/* Get the address of firmware data */
		fw_info->fw_raw_data = fw_img->data + fw_info->cfg_len;
	}

	return 0;
}

static int mxt_read_id_info(struct mxt_data *data)
{
	int ret = 0;
	u8 id[MXT_INFOMATION_BLOCK_SIZE] = {0};

	/* Read IC information */
	ret = mxt_read_mem(data, 0, MXT_INFOMATION_BLOCK_SIZE, id);
	if (ret) {
		LOGE("Read fail. IC information\n");
		goto out;
	} else {
		if (data->info) {
			kfree(data->info);
		}
		data->info = kzalloc(sizeof(struct mxt_info), GFP_KERNEL);
		if (data->info != NULL) {
			data->info->family_id = id[0];
			data->info->variant_id = id[1];
			data->info->version = id[2];
			data->info->build = id[3];
			data->info->matrix_xsize = id[4];
			data->info->matrix_ysize = id[5];
			data->info->object_num = id[6];
			LOGN("Family:%02X Variant:%02X Binary:%u.%u.%02X TX:%d RX:%d Objects:%d\n",
					data->info->family_id, data->info->variant_id, data->info->version >> 4, data->info->version & 0xF, data->info->build, data->info->matrix_xsize, data->info->matrix_ysize, data->info->object_num);
		}
	}

out:
	return ret;
}

static int mxt_get_object_table(struct mxt_data *data)
{
	int error = 0;
	int i = 0;
	u16 reg = 0;
	u8 reportid = 0;
	u8 buf[MXT_OBJECT_TABLE_ELEMENT_SIZE] = {0};
	struct mxt_object *object = NULL;

	if (data->info && data->object_table) {
		for (i = 0; i < data->info->object_num; i++) {
			object = data->object_table + i;

			reg = MXT_OBJECT_TABLE_START_ADDRESS + (MXT_OBJECT_TABLE_ELEMENT_SIZE * i);
			error = mxt_read_mem(data, reg, MXT_OBJECT_TABLE_ELEMENT_SIZE, buf);
			if (error) {
				LOGE("%s mxt_read_mem error\n", __func__);
				return error;
			}

			object->type = buf[0];
			object->start_address = (buf[2] << 8) | buf[1];
			/* the real size of object is buf[3]+1 */
			object->size_minus_one = buf[3];
			/* the real instances of object is buf[4]+1 */
			object->instances_minus_one = buf[4];
			object->num_report_ids = buf[5];

			if (object->num_report_ids) {
				reportid += object->num_report_ids * (object->instances_minus_one+1);
				data->max_reportid = reportid;
			}
		}
	} else {
		LOGE("%s() info, object_table is NULL\n",__func__);
		return 1;
	}

	/* Store maximum reportid */
	data->max_reportid = reportid;

	return 0;
}

static int mxt_enter_bootloader(struct mxt_data *data)
{
	int error = 0;

	if (data->object_table) {
		memset(data->object_table, 0x0, (MXT_OBJECT_NUM_MAX * sizeof(struct mxt_object)));
	} else {
		data->object_table = kzalloc((MXT_OBJECT_NUM_MAX * sizeof(struct mxt_object)), GFP_KERNEL);
		if (!data->object_table) {
			LOGE("%s Failed to allocate memory\n", __func__);
			error = 1;
			goto err_free_mem;
		} else {
			memset(data->object_table, 0x0, (MXT_OBJECT_NUM_MAX * sizeof(struct mxt_object)));
		}
	}
	/* Get object table information*/
	error = mxt_get_object_table(data);
	if (error)
		goto err_free_mem;

	/* Change to the bootloader mode */
	error = mxt_command_reset(data, MXT_BOOT_VALUE);
	if (error)
		goto err_free_mem;

err_free_mem:
	return error;
}

static int mxt_flash_fw(struct mxt_fw_info *fw_info)
{
	struct mxt_data *data = fw_info->data;
	const u8 *fw_data = fw_info->fw_raw_data;
	u32 fw_size = fw_info->fw_len;
	unsigned int frame_size = 0;
	unsigned int frame = 0;
	unsigned int pos = 0;
	int ret = 0;

	if (!fw_data) {
		LOGE("%s firmware data is Null\n", __func__);
		return -ENOMEM;
	}

	/* T1664 use 0x26 bootloader addr */
	ret = mxt_lookup_bootloader_address(data, 1);
	if (ret) {
		LOGE("Failed to lookup bootloader address\n");
		return ret;
	}

	init_completion(&data->bl_completion);

	ret = mxt_check_bootloader(data, MXT_WAITING_BOOTLOAD_CMD);
	if (ret) {
		/*may still be unlocked from previous update attempt */
		ret = mxt_check_bootloader(data, MXT_WAITING_FRAME_DATA);
		if (ret) {
			LOGE("Failed to check bootloader\n");
			goto out;
		}
	} else {
		LOGN("Unlocking bootloader\n");
		mxt_send_bootloader_cmd(data, true);
	}
	while (pos < fw_size) {
		ret = mxt_check_bootloader(data, MXT_WAITING_FRAME_DATA);
		if (ret) {
			LOGE("Fail updating firmware. wating_frame_data err\n");
			goto out;
		}

		frame_size = ((*(fw_data + pos) << 8) | *(fw_data + pos + 1));

		/*
		* We should add 2 at frame size as the the firmware data is not
		* included the CRC bytes.
		*/

		frame_size += 2;


		/* Write one frame to device */
		mxt_bootloader_write(data, fw_data + pos, frame_size);

		ret = mxt_check_bootloader(data, MXT_FRAME_CRC_PASS);
		if (ret) {
			LOGN("Fail updating firmware. frame_crc err\n");
			goto out;
		}

		pos += frame_size;
		frame++;

		if (frame % 50 == 0)
			LOGD("\t Updated %5d / %5d bytes\n", pos, fw_size);

		msleep(20);
	}

	msleep(MXT_FW_RESET_TIME);

out:
	return ret;
}

static int mxt_flash_fw_on_probe(struct mxt_fw_info *fw_info)
{
	struct mxt_data *data = fw_info->data;
	int error = 0;

	error = mxt_read_id_info(data);

	if (error) {
		/* need to check IC is in boot mode */
		/* T1664 use 0x26 bootloader Addr */
		error = mxt_probe_bootloader(data, 1);
		if (error) {
			LOGE("Failed to verify bootloader's status\n");
			goto out;
		}

		LOGN("Updating firmware from boot-mode\n");
		goto load_fw;
	}

	/* compare the version to verify necessity of firmware updating */
	LOGN("Binary Version [IC:%u.%u.%02X] [FW:%u.%u.%02X]\n",
		data->info->version >> 4, data->info->version & 0xF, data->info->build, fw_info->bin_ver >> 4,
		fw_info->bin_ver & 0xF, fw_info->build_ver);

	if (data->info->version == fw_info->bin_ver && data->info->build == fw_info->build_ver) {
		LOGN("Binary Version is same\n");
		goto out;
	}

	error = mxt_enter_bootloader(data);
	if (error) {
		LOGE("Failed enter bootloader mode\n");
		goto out;
	}

load_fw:
	error = mxt_flash_fw(fw_info);
	if (error)
		LOGE("Failed updating firmware\n");
	else
		LOGN("succeeded updating firmware\n");
out:
	return error;
}

static void mxt_make_reportid_table(struct mxt_data *data)
{
	struct mxt_object *objects = data->object_table;
	struct mxt_reportid *reportids = data->reportids;
	int i = 0, j = 0;
	int id = 0;

	for (i = 0; i < data->info->object_num; i++) {
		for (j = 0; j < objects[i].num_report_ids * (objects[i].instances_minus_one+1); j++) {
			id++;

			reportids[id].type = objects[i].type;
			reportids[id].index = j;
		}
	}
}

static int mxt_read_info_crc(struct mxt_data *data, u32 *crc_pointer)
{
	u16 crc_address = 0;
	u8 msg[3] = {0};
	int ret = 0;

	/* Read Info block CRC address */
	crc_address = MXT_OBJECT_TABLE_START_ADDRESS +
			data->info->object_num * MXT_OBJECT_TABLE_ELEMENT_SIZE;

	ret = mxt_read_mem(data, crc_address, 3, msg);
	if (ret)
		return ret;

	*crc_pointer = msg[0] | (msg[1] << 8) | (msg[2] << 16);

	return 0;
}

static int mxt_table_initialize(struct mxt_data *data)
{
	u32 read_info_crc = 0;
	int ret = 0;

	ret = mxt_read_id_info(data);
	if (ret)
		return ret;

	if (data->object_table) {
		memset(data->object_table, 0x0, (MXT_OBJECT_NUM_MAX * sizeof(struct mxt_object)));
	} else {
		data->object_table = kzalloc((MXT_OBJECT_NUM_MAX * sizeof(struct mxt_object)), GFP_KERNEL);
		if (!data->object_table) {
			LOGE("%s Failed to allocate memory\n", __func__);
			ret = 1;
			goto out;
		} else {
			memset(data->object_table, 0x0, (MXT_OBJECT_NUM_MAX * sizeof(struct mxt_object)));
		}
	}

	/* Get object table infomation */
	ret = mxt_get_object_table(data);
	if (ret)
		goto out;

	if (data->reportids) {
		kfree(data->reportids);
	}

	data->reportids = kzalloc(((data->max_reportid + 1) * sizeof(struct mxt_reportid)), GFP_KERNEL);
	if (!data->reportids) {
		LOGE("%s Failed to allocate memory 2\n", __func__);
		ret = -ENOMEM;
		goto out;
	}

	/* Make report id table */
	mxt_make_reportid_table(data);

	/* Verify the info CRC */
	ret = mxt_read_info_crc(data, &read_info_crc);
	if (ret)
		goto out;

	return 0;
out:
	return ret;
}

static int mxt_read_message(struct mxt_data *data, struct mxt_message *message)
{
	struct mxt_object *object = NULL;

	object = mxt_get_object(data, MXT_GEN_MESSAGE_T5);
	if (!object) {
		LOGE("mxt_read_message-mxt_get_object error\n");
		return -EINVAL;
	}

	return mxt_read_mem(data, object->start_address, sizeof(struct mxt_message), message);
}

static int mxt_read_message_reportid(struct mxt_data *data, struct mxt_message *message, u8 reportid)
{
	int try = 0;
	int error = 0;
	int fail_count = 0;

	fail_count = data->max_reportid * 2;

	while (++try < fail_count) {
		error = mxt_read_message(data, message);
		if (error) {
			LOGE("mxt_read_message error\n");
			print_hex_dump(KERN_DEBUG, "[Solomon] CRC : ",
				DUMP_PREFIX_NONE, 16, 1,
				message, sizeof(struct mxt_message), false);
			return error;
		}

		if (message->reportid == 0xff)
			continue;

		if (message->reportid == reportid)
			return 0;
	}

	return -EINVAL;
}

static int mxt_read_config_crc(struct mxt_data *data, u32 *crc)
{
	struct mxt_message message = {0};
	struct mxt_object *object = NULL;
	int error = 0;

	object = mxt_get_object(data, MXT_GEN_COMMAND_T6);
	if (!object)
		return -EIO;

	/* Try to read the config checksum of the existing cfg */
	mxt_write_object(data, MXT_GEN_COMMAND_T6, MXT_COMMAND_REPORTALL, 1);

	/* Read message from command processor, which only has one report ID */
	error = mxt_read_message_reportid(data, &message, 1);//data->max_reportid);
	if (error) {
		LOGE("Failed to retrieve CRC\n");
		return error;
	}

	/* Bytes 1-3 are the checksum. */
	*crc = message.message[1] | (message.message[2] << 8) | (message.message[3] << 16);

	return 0;
}

static int mxt_write_config(struct mxt_fw_info *fw_info)
{
	struct mxt_data *data = fw_info->data;
	struct mxt_object *object = NULL;
	struct mxt_cfg_data *cfg_data = NULL;
	u32 current_crc = 0;
	u8 i = 0, val = 0;
	u16 reg = 0, index = 0;
	int ret = 0;

	LOGN("%s\n", __func__);

	if (!fw_info->cfg_raw_data) {
		LOGN("No cfg data in file\n");
		return ret;
	}

	/* Get config CRC from device */
	ret = mxt_read_config_crc(data, &current_crc);
	if (ret) {
		LOGN("fail to read config crc\n");
		return ret;
	}

	/* Check Version information */
	if (fw_info->bin_ver != data->info->version) {
		LOGE("Warning: version mismatch! %s\n", __func__);
		return 0;
	}
	if (fw_info->build_ver != data->info->build) {
		LOGE("Warning: build num mismatch! %s\n", __func__);
		return 0;
	}


	if (current_crc == fw_info->cfg_crc) {
		LOGN("Same Config[%06X] Skip Writing\n", current_crc);
		return 0;
	}

	/* Restore memory and stop event handing */
	ret = mxt_command_backup(data, MXT_DISALEEVT_VALUE);
	if (ret) {
		LOGE("Failed Restore NV and stop event\n");
		return -EINVAL;
	}

	LOGN("Writing Config:[FW:%06X] [IC:%06X]\n", fw_info->cfg_crc, current_crc);
	/* Write config info */
	for (index = 0; index < fw_info->cfg_len;) {
		if (index + (u16)sizeof(struct mxt_cfg_data) >= fw_info->cfg_len) {
			LOGD("index(%d) of cfg_data exceeded total size(%d)!!\n",
				index + (u16)sizeof(struct mxt_cfg_data), fw_info->cfg_len);
			return -EINVAL;
		}

		/* Get the info about each object */
		cfg_data = (struct mxt_cfg_data *)(&fw_info->cfg_raw_data[index]);

		index += (u16)sizeof(struct mxt_cfg_data) + cfg_data->size;
		if (index > fw_info->cfg_len) {
			LOGD("index(%d) of cfg_data exceeded total size(%d) in T%d object!!\n",
				index, fw_info->cfg_len, cfg_data->type);
			return -EINVAL;
		}

		object = mxt_get_object(data, cfg_data->type);
		if (!object) {
			LOGE("T%d is Invalid object type\n", cfg_data->type);
			return -EINVAL;
		}

		/* Check and compare the size, instance of each object */
		if (cfg_data->size > (object->size_minus_one+1)) {
			LOGE("T%d Object length exceeded!\n", cfg_data->type);
			return -EINVAL;
		}
		if (cfg_data->instance >= (object->instances_minus_one+1)) {
			LOGE("T%d Object instances exceeded!\n", cfg_data->type);
			return -EINVAL;
		}

		LOGD("\t Writing config for T%02d len %3d instance %d (%3d/%3d)\n",
			cfg_data->type, object->size_minus_one, cfg_data->instance, index, fw_info->cfg_len);

		reg = object->start_address + (object->size_minus_one+1) * cfg_data->instance;

		/* Write register values of each object */
		ret = mxt_write_mem(data, reg, cfg_data->size, cfg_data->register_val);
		if (ret) {
			LOGE("Write T%d Object failed\n", object->type);
			return ret;
		}

		/*
		 * If firmware is upgraded, new bytes may be added to end of
		 * objects. It is generally forward compatible to zero these
		 * bytes - previous behaviour will be retained. However
		 * this does invalidate the CRC and will force a config
		 * download every time until the configuration is updated.
		 */
		if (cfg_data->size < (object->size_minus_one+1)) {
			LOGE("Warning: zeroing %d byte(s) in T%d\n",
				 (object->size_minus_one+1) - cfg_data->size, cfg_data->type);

			for (i = cfg_data->size + 1; i < (object->size_minus_one+1); i++) {
				ret = mxt_write_mem(data, reg + i, 1, &val);
				if (ret)
					return ret;
			}
		}
	}

	LOGN("Configuration Updated\n");

	/* Backup to memory */
	ret = mxt_command_backup(data, MXT_BACKUP_VALUE);
	if (ret) {
		LOGE("Failed backup NV data\n");
		return -EINVAL;
	}

	/* Soft reset */
	ret = mxt_command_reset(data, MXT_RESET_VALUE);
	if (ret) {
		LOGE("Failed Reset IC\n");
		return -EINVAL;
	}

	return ret;
}

static int  mxt_config_initialize(struct mxt_fw_info *fw_info)
{
	struct mxt_data *data = fw_info->data;
	int ret = 0;

	LOGN("%s\n", __func__);

	ret = mxt_write_config(fw_info);
	if (ret) {
		LOGE("Failed to write config from file\n");
		goto out;
	}

	if (data->patch.patch) {
		ret = mxt_patch_init(data, data->patch.patch);
	}

	if (ret == 0) {
		data->mxt_drv_data->mxt_data = data;
	} else {
		data->mxt_drv_data->mxt_data = NULL;
		LOGE("Failed to get data->mxt_drv_data->mxt_data(NULL) \n");
	}

out:
	return ret;
}

static int mxt_no_fw_initialize(struct mxt_data *data)
{
	struct mxt_patch *patch_info = NULL;
	int error = 0;

	if (!data) {
		LOGE("mxt_data is NULL\n");
		return -EINVAL;
	}

	LOGN("start\n");

	error = mxt_table_initialize(data);
	if (error) {
		LOGE("Failed to initialize\n");
		return -EINVAL;
	}

	data->mxt_drv_data->mxt_data = data;
	if (!data->mxt_drv_data->mxt_data)
		LOGE("%s Failed to pointer [%p]\n", __func__, data->mxt_drv_data->mxt_data);

	patch_info = &data->patch;
	patch_info->src_item = kzalloc(MXT_PATCH_ITEM_END * sizeof(u16), GFP_KERNEL);
	if (!patch_info->src_item) {
		LOGE("src_item alloc error\n");
		error = -EINVAL;
		goto out;
	}

	patch_info->event_addr = kzalloc(patch_info->event_cnt * sizeof(u16), GFP_KERNEL);
	if (!patch_info->event_addr) {
		TOUCH_PATCH_INFO_MSG("event_addr alloc error\n");
		error = -EINVAL;
		goto out;
	}

	error = mxt_read_info_block(data);
	if (error) {
		LOGE("%s error mxt_read_info_block\n", __func__);
		goto out;
	}

	error = mxt_init_t7_power_cfg(data);
	if (error)
		LOGE("%s error mxt_init_t7_power_cfg\n", __func__);

out:
	if (error) {
		LOGE("Free allocated memory because failed\n");
		if (patch_info->event_addr) {
			kfree(patch_info->event_addr);
			patch_info->event_addr = NULL;
		}
		if (patch_info->src_item) {
			kfree(patch_info->src_item);
			patch_info->src_item = NULL;
		}
	}
	return error;
}

int mxt_request_firmware_work(const struct firmware *fw, void *context)
{
	struct mxt_data *data = context;
	int error = 0;

	mxt_power_block(data, POWERLOCK_FW_UP);

	data->fw_info.data = data;
	if (fw) {
		error = mxt_verify_fw(&data->fw_info, fw);
		if (error)
			goto ts_rest_init;
	} else {
		goto out;
	}
	/* Skip update on boot up if firmware file does not have a header */
	if (!data->fw_info.hdr_len)
		goto ts_rest_init;

	error = mxt_flash_fw_on_probe(&data->fw_info);
	if (error)
		goto out;

ts_rest_init:
	error = mxt_table_initialize(data);
	if (error) {
		LOGE("Failed to initialize\n");
		goto out;
	}

	error = mxt_init_t93_tab_count(data);
	if (error) {
		LOGE("Failed to get T93 tab count\n");
	}

	error = mxt_config_initialize(&data->fw_info);
	if (error) {
		LOGE("Failed to rest initialize\n");
		goto out;
	}

out:
	mxt_power_unblock(data, POWERLOCK_FW_UP);
	return error;
}

int mxt_update_firmware(struct mxt_data *data, const struct firmware *fw)
{
	int error = 0;

	LOGN("%s %lu\n", __func__, sizeof(*(fw->data)));
	error = mxt_request_firmware_work(fw, data);
	if (error) {
		LOGE("%s error mxt_request_firmware_work\n", __func__);
		return 1;
	}

	error = mxt_read_info_block(data);
	if (error) {
		LOGE("%s error mxt_read_info_block\n", __func__);
		return 1;
	}

	error = mxt_init_t7_power_cfg(data);
	if (error) {
		LOGE("%s error mxt_init_t7_power_cfg\n", __func__);
		return 1;
	}

	return 0;

}

static int mxt_get_pw_status(void)
{
	int ret = 0;
	incell_pw_status status = { false, false };

	ret = incell_get_power_status(&status);
	if (ret) {
		LOGE("failed get power status\n");
		goto exit;
	}

	if (status.display_power && status.touch_power) {
		ret = INCELL_OK;
	} else {
		LOGN("power status: disable\n");
		ret = INCELL_ERROR;
	}

exit:
	return ret;
}

static int mxt_pw_lock(incell_pw_lock status)
{
	int ret = 0;
	incell_pw_status pwr_status = { false, false };

	ret = incell_power_lock_ctrl(status,
		&pwr_status);

	switch (ret) {
	case INCELL_OK:
		LOGN("power status: %s\n",
			status ? "LOCK" : "UNLOCK");
		break;
	case INCELL_ERROR:
	case INCELL_ALREADY_LOCKED:
	case INCELL_ALREADY_UNLOCKED:
	default:
		LOGE("power lock failed ret:%d\n", ret);
		break;
	}

	return ret;
}

static int mxt_init_sysfs(struct i2c_client *client, struct mxt_data *data)
{
	int ret = 0;

	ret = kobject_init_and_add(&data->lge_touch_kobj, &lge_touch_kobj_type,
			data->input_dev->dev.kobj.parent, "%s", LGE_TOUCH_NAME);
	if (ret < 0) {
		LOGE("kobject_init_and_add is failed\n");
		ret = -EINVAL;
		goto exit;
	}

	sysfs_bin_attr_init(&data->mem_access_attr);
	data->mem_access_attr.attr.name = "mem_access";
	data->mem_access_attr.attr.mode = S_IWUSR | S_IRUSR;
	data->mem_access_attr.read = mxt_mem_access_read;
	data->mem_access_attr.write = mxt_mem_access_write;
	data->mem_access_attr.size = 32768;
	if (sysfs_create_bin_file(&client->dev.kobj, &data->mem_access_attr) < 0) {
		LOGE("Failed to create %s\n", data->mem_access_attr.attr.name);
		ret = -EINVAL;
		goto exit;
	}

	data->prd_rawdata_data = kzalloc(MAX_ATTR_SIZE, GFP_KERNEL);
	if (data->prd_rawdata_data) {
		LOGN("%s rawdata_buffer(%d KB) malloc success\n", __func__,
				MAX_ATTR_SIZE/1024);
	} else {
		LOGE("%s rawdata_buffer(%d KB) malloc failed\n", __func__,
				MAX_ATTR_SIZE/1024);
		ret = -EINVAL;
		goto exit;

	}
	sysfs_bin_attr_init(&data->rawdata_attr);
	data->rawdata_attr.attr.name = "rawdata";
	data->rawdata_attr.attr.mode = S_IWUSR | S_IRUGO;
	data->rawdata_attr.read = mxt_run_rawdata_show;
	data->rawdata_attr.size = MAX_ATTR_SIZE;
	if (sysfs_create_bin_file(&data->lge_touch_kobj, &data->rawdata_attr) < 0) {
		LOGE("Failed to create %s\n", data->rawdata_attr.attr.name);
		ret = -EINVAL;
		goto exit;
	}

	data->prd_delta_data = kzalloc(MAX_ATTR_SIZE, GFP_KERNEL);
	if (data->prd_delta_data) {
		LOGN("%s delta_buffer(%d KB) malloc success\n", __func__,
				MAX_ATTR_SIZE/1024);
	} else {
		LOGE("%s delta_buffer(%d KB) malloc failed\n", __func__,
				MAX_ATTR_SIZE/1024);
		ret = -EINVAL;
		goto exit;
	}
	sysfs_bin_attr_init(&data->delta_attr);
	data->delta_attr.attr.name = "delta";
	data->delta_attr.attr.mode = S_IWUSR | S_IRUGO;
	data->delta_attr.read = mxt_run_delta_show;
	data->delta_attr.size = MAX_ATTR_SIZE;
	if (sysfs_create_bin_file(&data->lge_touch_kobj, &data->delta_attr) < 0) {
		LOGE("Failed to create %s\n", data->delta_attr.attr.name);
		ret = -EINVAL;
		goto exit;
	}

exit:
	return ret;
}

static int mxt_init_works(struct mxt_data *data)
{
	int ret = 0;

	INIT_WORK(&data->multi_tap_work, touch_multi_tap_work);

	INIT_DELAYED_WORK(&data->work_button_lock, mxt_button_lock_func);

	INIT_DELAYED_WORK(&data->work_palm_unlock, mxt_palm_unlock_func);

	INIT_DELAYED_WORK(&data->work_delay_cal, mxt_delay_cal_func);

	if (data->watchdog.supported)
		INIT_DELAYED_WORK(&data->watchdog.work, mxt_watchdog_func);

	return ret;
}

static int mxt_init_gpios(struct mxt_data *data)
{
	int ret = 0;

	/* request interrupt pin */
	if (data->pdata->gpio_int > 0) {
		ret = gpio_request(data->pdata->gpio_int, "touch_int");
		if (ret < 0) {
			LOGE("FAIL: touch_int gpio_request\n");
			goto exit;
		}
		gpio_direction_input(data->pdata->gpio_int);
	}

exit:
	return ret;
}

static int mxt_after_init_work(struct i2c_client *client,
		struct mxt_data *data, const struct firmware *fw)
{
	int ret = 0;
	int cnt = 0;
	struct mxt_data *mxt_data = NULL;

	/* power lock */
	ret = mxt_pw_lock(INCELL_DISPLAY_POWER_LOCK);
	if (ret) {
		LOGE("Failed to lock power\n");
		return ret;
	}

	if (!fw) {
		ret = -ENOENT;
		goto skip_fw;
	}

	for (cnt = 0; cnt < MXT_FW_RETRY_NUM; cnt++) {
		ret = mxt_update_firmware(data, fw);
		if (!ret)
			break;
		mxt_hw_reset(data);
	}

skip_fw:
	if (ret) {
		LOGE("Firmware load failed. Initializing anyway.\n");

		ret = mxt_no_fw_initialize(data);
		if (ret) {
			LOGE("Failed mxt_no_fw_initialize\n");
			goto unlock;
		}
	}

	if (data->mxt_drv_data->mxt_data)
		mxt_data =  data->mxt_drv_data->mxt_data;

	mxt_write_object(data, MXT_GEN_COMMAND_T6, MXT_COMMAND_REPORTALL, 1);

	mxt_read_fw_version(data);

	if (data->T100_reportid_min) {
		ret = mxt_initialize_t100_input_device(data);
		if (ret) {
			LOGE("Failed to init t100\n");
			goto err_free_irq;
		}
	} else {
		LOGE("Failed to read touch object\n");
		goto err_free_irq;
	}

	/* init work queues */
	ret = mxt_init_works(data);
	if (ret)
		LOGE("Failed to init works: %d\n", ret);

	/*channal size init for reference check*/
	ret = __mxt_read_reg(data->client, data->T100_address + 8, 1, &data->channel_size.start_x);
	ret = __mxt_read_reg(data->client, data->T100_address + 9, 1, &data->channel_size.size_x);
	ret = __mxt_read_reg(data->client, data->T100_address + 19, 1, &data->channel_size.start_y);
	ret = __mxt_read_reg(data->client, data->T100_address + 20, 1, &data->channel_size.size_y);

	if (!ret)
		LOGD("Succeed to read channel_size %d %d %d %d\n", data->channel_size.start_x, data->channel_size.start_y, data->channel_size.size_x, data->channel_size.size_y);

	if (mxt_data) {
		ret = __mxt_read_reg(mxt_data->client,
			mxt_data->T71_address + 51, 26,
			&mxt_data->ref_limit);
		if (!ret) {
			LOGD("Succeed to read reference limit u:%d x_all_err_chk:%d y_all_err_chk:%d only_x_err_cnt : %d only_y_err_cnt : %d max-min rng:%d diff_rng:%d err_cnt:%d err_weight:%d\n",
				mxt_data->ref_limit.ref_chk_using, mxt_data->ref_limit.ref_x_all_err_line,
				mxt_data->ref_limit.ref_y_all_err_line, mxt_data->ref_limit.xline_max_err_cnt,
				mxt_data->ref_limit.yline_max_err_cnt, mxt_data->ref_limit.ref_rng_limit,
				mxt_data->ref_limit.ref_diff_max, mxt_data->ref_limit.ref_err_cnt,
				mxt_data->ref_limit.err_weight);
			LOGD("Err Range y: %d %d %d %d %d %d %d %d %d %d %d %d %d %d, Button range:%d %d %d\n",
				mxt_data->ref_limit.y_line_dif[0], mxt_data->ref_limit.y_line_dif[1],
				mxt_data->ref_limit.y_line_dif[2], mxt_data->ref_limit.y_line_dif[3],
				mxt_data->ref_limit.y_line_dif[4], mxt_data->ref_limit.y_line_dif[5],
				mxt_data->ref_limit.y_line_dif[6], mxt_data->ref_limit.y_line_dif[7],
				mxt_data->ref_limit.y_line_dif[8], mxt_data->ref_limit.y_line_dif[9],
				mxt_data->ref_limit.y_line_dif[10], mxt_data->ref_limit.y_line_dif[11],
				mxt_data->ref_limit.y_line_dif[12], mxt_data->ref_limit.y_line_dif[13],
				mxt_data->ref_limit.butt_dif[0], mxt_data->ref_limit.butt_dif[1],
				mxt_data->ref_limit.butt_dif[2]);
		}
	}

	data->ref_chk = 0;

	data->mxt_drv_data->factorymode = false;

	hrtimer_init(&data->multi_tap_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	data->multi_tap_timer.function = &tci_timer_func;

	if (mxt_data) {
		LOGD("%s mxt_data exist \n", __func__);
	} else {
		LOGE("%s mxt_data is NULL \n", __func__);
	}

#ifdef CONFIG_TOUCHSCREEN_ATMEL_STYLUS_PEN
	if (mxt_read_pen_support(data)) {
		LOGE("Read Pen_support Fail\n");
	}
#endif
	trigger_usb_state_from_otg(data, data->mxt_drv_data->usb_type);

	msleep(5);
	LOGN("%s success \n", __func__);
	data->mxt_drv_data->is_probing = false;

	mxt_drm_suspend(data);

	data->suspended = true;
	data->enable_reporting = false;

	complete(&touch_charge_out_comp);

	/* power unlock */
	ret = mxt_pw_lock(INCELL_DISPLAY_POWER_UNLOCK);
	data->after_work = true;

	if (fw)
		release_firmware(fw);

	return ret;

err_free_irq:
	free_irq(data->irq, data);
	drm_unregister_client(&data->drm_notif);
	if (data)
		kfree(data);
unlock:
	ret = mxt_pw_lock(INCELL_DISPLAY_POWER_UNLOCK);
	if (ret)
		LOGE("Failed to unlock power\n");
	return -EINVAL;
}

static void mxt_after_init_load_fw(const struct firmware *fw, void *ctx)
{
	struct mxt_data *data = ctx;
	struct i2c_client *client = data->client;

	mxt_after_init_work(client, data, fw);
}

static int mxt_init_recover(struct i2c_client *client, struct mxt_data *data)
{
	const struct firmware *fw = NULL;
	int ret;

	if (data->pdata->fw_name) {
		ret = request_firmware(&fw, data->pdata->fw_name, &data->client->dev);
		if (ret) {
			LOGE("%s error request_firmware %d\n", __func__, ret);
			return 1;
		}
	}

	return mxt_after_init_work(client, data, fw);
}

static int mxt_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct mxt_data *data = NULL;
	int error = 0;
	char *path_name = "common_touch";

	pr_notice("%s \n", __func__);

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		pr_err("i2c functionality check error\n");
		return -ENOMEM;
	}
	data = kzalloc(sizeof(struct mxt_data), GFP_KERNEL);
	if (!data) {
		pr_err("[Solomon] Failed to allocate memory\n");
		return -ENOMEM;
	}
	data->log_level = MXT_DEFAULT_LOG_LEVEL;
	if (mxt_init_log_level(data)) {
		pr_err("[Solomon] init failed log_level\n");
		goto err_free_mem;
	}

	LOGN("SIZE OF MXT DATA IS %lu in %s\n", sizeof(struct mxt_data), __func__);
	LOGN("SIZE OF mxt_fw_info IS %lu in %s\n", sizeof(struct mxt_fw_info), __func__);

	data->mxt_drv_data = kzalloc(sizeof(struct mxt_driver_data), GFP_KERNEL);
	if (!data->mxt_drv_data) {
		LOGE("Failed to allocate memory\n");
		return -ENOMEM;
	}
	LOGN("SIZE OF MXT DRV DATA IS %lu in %s\n", sizeof(struct mxt_driver_data), __func__);

	data->mxt_drv_data->touch_wq = create_singlethread_workqueue("touch_wq");
	if (!data->mxt_drv_data->touch_wq) {
		LOGN("touch_wq error\n");
		return -ENOMEM;
	}

	data->mxt_drv_data->touch_multi_tap_wq = create_singlethread_workqueue("touch_multi_tap_wq");
	if (!data->mxt_drv_data->touch_multi_tap_wq) {
		LOGN("touch_multi_tap_wq error\n");
		return -ENOMEM;
	}

	data->mxt_drv_data->is_probing = true;
	mutex_init(&data->mxt_drv_data->i2c_suspend_lock);
	mutex_init(&data->mxt_drv_data->irq_lock);
	mutex_init(&data->mxt_drv_data->lpwg_lock);
	mutex_init(&data->mxt_drv_data->dev_lock);

	data->ref_chk = 1;
	snprintf(data->phys, sizeof(data->phys), "i2c-%u-%04x/input0", client->adapter->nr, client->addr);
	LOGN("i2c-%u-%04x/input0\n", client->adapter->nr, client->addr);
	data->client = client;
	data->irq = client->irq;
	i2c_set_clientdata(client, data);

	/*read dtsi data*/
	if (client->dev.of_node) {
		data->pdata = devm_kzalloc(&client->dev, sizeof(struct mxt_platform_data), GFP_KERNEL);
		if (!data->pdata) {
			LOGE("Failed to allocate pdata memory\n");
			error = -ENOMEM;
			goto err_free_mem;
		}

		error = mxt_parse_from_dt(&client->dev, data);
		if (error)
			goto err_free_mem;
	}

	LOGN("Use LPWG feature\n");
	data->qwindow_size = devm_kzalloc(&client->dev, sizeof(struct quickcover_size), GFP_KERNEL);
	if (!data->qwindow_size) {
		LOGE("Failed to allocate qwindow_size memory\n");
		error = -ENOMEM;
		goto err_free_mem;
	}

	device_init_wakeup(&client->dev, 1);

	init_completion(&data->bl_completion);
	init_completion(&data->reset_completion);
	init_completion(&data->crc_completion);
	init_completion(&data->t25_completion); /* Self Test */

	error = mxt_init_gpios(data);
	if (error)
		goto err_interrupt_failed;

	LOGN("request_threaded_irq %s\n", __func__);
	error = request_threaded_irq(data->irq, touch_irq_handler, mxt_interrupt,
			IRQF_TRIGGER_LOW | IRQF_ONESHOT, client->name, data);
	if (error) {
		LOGE("Failed to register interrupt\n");
		goto err_free_pdata;
	}
	disable_irq_nosync(data->irq);

	/* init notification callbacks */
	data->drm_notif.notifier_call = drm_notifier_callback;

	error = drm_register_client(&data->drm_notif);
	if (error) {
		LOGE("Unable to register drm_notifier: %d\n", error);
		goto err_free_pdata;
	}
//	mxt_probe_regulators(data);
	mxt_regulator_enable(data);

	touch_disable_irq(data);

	data->object_table = kzalloc((MXT_OBJECT_NUM_MAX * sizeof(struct mxt_object)), GFP_KERNEL);
	if (!data->object_table) {
		LOGE("%s Failed to allocate memory\n", __func__);
		goto err_free_pdata;
	}

	error = mxt_initialize_t100_input_device(data);
	if (error) {
		LOGE("Failed to init t100\n");
		goto err_free_irq;
	}

	error = subsys_system_register(&touch_subsys, NULL);
	if (error < 0)
		LOGE("%s, bus is not registered, ret : %d\n", __func__, error);

	error = device_register(&device_touch);
	if (error < 0)
		LOGE("%s, device is not registered, ret : %d\n", __func__, error);

	error = mxt_init_sysfs(client, data);
	if (error)
		goto err_lge_touch_sysfs_init_and_add;
	dev_set_name(&data->virtdev, "%s", path_name);
	error = device_register(&data->virtdev);
	if (error)
		LOGE("%s, device virtdev register error\n", __func__);
	dev_set_drvdata(&data->virtdev, data);
	error = sysfs_create_link(&data->virtdev.kobj, &data->lge_touch_kobj, "touch");
	if (error) {
		LOGE("%s, sysfs link error\n", __func__);
		goto err_create_sysfs_link;
	}
	if (!mxt_get_pw_status()) {
		LOGN("Requesting firmware [%s]\n", data->pdata->fw_name);
		error = request_firmware_nowait(THIS_MODULE, true,
						data->pdata->fw_name,
						&client->dev, GFP_KERNEL,
						data, mxt_after_init_load_fw);
		if (error) {
			LOGE("failed after init on probe\n");
			drm_unregister_client(&data->drm_notif);
			complete(&touch_charge_out_comp);
		}
	}

	data->first_unblank = true;

	return 0;

err_create_sysfs_link:
	if (drm_unregister_client(&data->drm_notif))
		LOGE("Error occurred while unregistering drm_notifier\n");
	sysfs_remove_bin_file(&client->dev.kobj, &data->mem_access_attr);
	sysfs_remove_bin_file(&data->lge_touch_kobj, &data->rawdata_attr);
	sysfs_remove_bin_file(&data->lge_touch_kobj, &data->delta_attr);

err_lge_touch_sysfs_init_and_add:
	kobject_del(&data->lge_touch_kobj);
	mxt_free_object_table(data);
err_free_irq:
	free_irq(data->irq, data);
err_interrupt_failed:
err_free_pdata:
err_free_mem:
	if (data)
		kfree(data);
	LOGE("%s Failed\n", __func__);
	return error;
}

static int mxt_remove(struct i2c_client *client)
{
	struct mxt_data *data = i2c_get_clientdata(client);

	pr_notice("%s\n", __func__);

	if (data) {
		if (data->mxt_drv_data->touch_wq)
			destroy_workqueue(data->mxt_drv_data->touch_wq);

		if (data->mxt_drv_data->touch_multi_tap_wq)
			destroy_workqueue(data->mxt_drv_data->touch_multi_tap_wq);

		if (drm_unregister_client(&data->drm_notif))
			LOGE("Error occurred while unregistering drm_notifier\n");

		mutex_destroy(&data->mxt_drv_data->i2c_suspend_lock);
		mutex_destroy(&data->mxt_drv_data->irq_lock);
		mutex_destroy(&data->mxt_drv_data->lpwg_lock);
		mutex_destroy(&data->mxt_drv_data->dev_lock);

		device_init_wakeup(&client->dev, 0);
		kobject_del(&data->lge_touch_kobj);

		if (data->pdata->gpio_int > 0)
			gpio_free(data->pdata->gpio_int);
		if (data->irq)
			free_irq(data->irq, data);

		mxt_free_object_table(data);

		kfree(data);
	}

	return 0;
}

static int mxt_suspend(struct device *dev)
{
	struct mxt_data *data = dev_get_drvdata(dev);

	if(!data->mxt_drv_data->is_probing) {
		LOGN("%s int status:%d\n", __func__, gpio_get_value(data->pdata->gpio_int));
	}
	return 0;
}

static int mxt_resume(struct device *dev)
{
	struct mxt_data *data = dev_get_drvdata(dev);

	if(!data->mxt_drv_data->is_probing) {
		LOGN("%s int status:%d\n", __func__, gpio_get_value(data->pdata->gpio_int));

		if (data->pm_state == PM_SUSPEND_IRQ) {
			struct irq_desc *desc = irq_to_desc(data->irq);

			if (desc == NULL) {
				LOGE("Null Pointer from irq_to_desc\n");
				return -ENOMEM;
			}

			data->pm_state = PM_RESUME;

			LOGD("resend interrupt\n");

			return 0;
		}

		data->pm_state = PM_RESUME;
	}
	return 0;
}

static void mxt_prepare_sod_mode(struct mxt_data *data)
{
	int value[4] = {1, 0, 1, 0};

	if (!data->sod_mode.supported) {
		LOGN("SOD mode is not supported\n");
		return;
	}

	if (mxt_get_pw_status()) {
		LOGE("Power supply is dropped, cannot change to sod mode\n");
		return;
	}

	touch_disable_irq_wake(data);
	touch_disable_irq(data);

	touch_enable_irq(data);
	touch_enable_irq_wake(data);

	mutex_lock(&data->mxt_drv_data->lpwg_lock);

	if (data->stamina_mode.status)
		mxt_patch_event(data, PATCH_EVENT_CODE_STAMINA_DISABLE);

	mxt_patch_event(data, PATCH_EVENT_CODE_AOD_ENABLE);

	atmel_ts_lpwg_update_all(data->client, LPWG_UPDATE_ALL, value);

	mutex_unlock(&data->mxt_drv_data->lpwg_lock);
}

static int mxt_drm_suspend(struct mxt_data *data)
{
	struct input_dev *input_dev = data->input_dev;
	int ret = 0;

	LOGN("%s\n", __func__);

	data->sod_mode.status = incell_get_display_sod();
	data->sod_mode.pre_status = incell_get_display_pre_sod();
	if (data->sod_mode.pre_status && data->sod_mode.status == SOD_POWER_OFF) {
		LOGN("%s　power off\n", __func__);
	} else if (data->sod_mode.pre_status) {
		LOGN("%s power off skip\n", __func__);
		mxt_prepare_sod_mode(data);
		return ret;
	} else if (data->pdata->panel_on == POWER_OFF) {
		LOGN("%s already power off\n", __func__);
		return ret;
	}

	if (data->watchdog.supported)
		cancel_delayed_work(&data->watchdog.work);

	cancel_delayed_work_sync(&data->work_delay_cal);

	if (data->after_work && data->charge_out) {
		ret = mxt_pw_lock(INCELL_DISPLAY_POWER_LOCK);
		if (ret)
			LOGN("failed lock power\n");
	}

	if (mxt_power_block_get(data)) {
		LOGN("still in use. do nothing\n");
		return ret;
	}

	mutex_lock(&data->mxt_drv_data->dev_lock);

	data->enable_reporting = false;
	data->pdata->panel_on = POWER_OFF;
	data->mxt_drv_data->resume_flag = 0;

	if (input_dev->users)
		mxt_stop(data);

#ifdef CONFIG_TOUCHSCREEN_ATMEL_LPWG_DEBUG_REASON
	if (data->lpwg_mode) {
		ret = mxt_lpwg_debug_interrupt_control(data, data->pdata->use_debug_reason);
		if (ret)
			LOGE("%s error mxt_lpwg_debug_interrupt_control\n", __func__);
	}
#endif

	if (data->after_work && data->charge_out) {
		ret = mxt_pw_lock(INCELL_DISPLAY_POWER_UNLOCK);
		if (ret)
			LOGN("failed unlock power\n");
	}

	mutex_unlock(&data->mxt_drv_data->dev_lock);
	return ret;
}

static void mxt_cancel_sod_mode(struct mxt_data *data)
{
	int value[4] = {0, 0, 0, 0};

	if (!data->sod_mode.supported) {
		LOGN("SOD mode is not supported\n");
		return;
	}

	if (mxt_get_pw_status()) {
		LOGE("Power supply is dropped, cannot cancel sod mode\n");
		return;
	}

	touch_disable_irq_wake(data);
	touch_disable_irq(data);

	touch_enable_irq(data);

	mutex_lock(&data->mxt_drv_data->lpwg_lock);

	mxt_patch_event(data, PATCH_EVENT_CODE_AOD_DISABLE);

	atmel_ts_lpwg_update_all(data->client, LPWG_UPDATE_ALL, value);

	mutex_unlock(&data->mxt_drv_data->lpwg_lock);
}

static int mxt_drm_resume(struct mxt_data *data)
{
	struct input_dev *input_dev = data->input_dev;
	int ret = 0;

	LOGN("%s\n", __func__);

	if (data->sod_mode.status || data->aod_mode.status)
		mxt_cancel_sod_mode(data);

	if (data->after_work && data->charge_out) {
		ret = mxt_pw_lock(INCELL_DISPLAY_POWER_LOCK);
		if (ret)
			LOGN("failed lock power\n");
	}

	if (mxt_power_block_get(data)) {
		LOGN("still in use. Do nothing\n");
		return ret;
	}

	if (data->pdata->panel_on == POWER_ON) {
		LOGN("already power on\n");
		return ret;
	}

	mutex_lock(&data->mxt_drv_data->dev_lock);

	if (data->work_deepsleep_enabled) {
		data->work_deepsleep_enabled = false;
		cancel_delayed_work_sync(&data->work_deepsleep);
	}

	if (data->watchdog.supported)
		queue_delayed_work(data->mxt_drv_data->touch_wq, &data->watchdog.work,
			msecs_to_jiffies(data->watchdog.delay));

	data->pdata->panel_on = POWER_ON;
	data->palm = false;

	data->mxt_drv_data->resume_flag = 1;

	if (data->lpwg_mode || data->mfts_lpwg) {
		touch_disable_irq_wake(data);
	}

	if (data->pdata->use_mfts && data->mfts_enable) {
		LOGN("MFTS : IC Init start\n");

		touch_disable_irq(data);

		mxt_regulator_enable(data);

		ret = mxt_table_initialize(data);
		if (ret) {
			LOGE("Failed to initialize\n");
			goto exit;
		}

		ret = mxt_read_info_block(data);
		if (ret) {
			LOGE("%s error mxt_read_info_block\n", __func__);
			goto exit;
		}

		if (data->T100_reportid_min) {
			ret = mxt_initialize_t100_input_device(data);
			if (ret) {
				LOGE("Failed to init t100\n");
				goto exit;
			}
		} else {
			LOGE("Failed to read touch object\n");
			goto exit;
		}

		touch_enable_irq(data);

exit:
		LOGN("MFTS : IC Init complete\n");

		mxt_read_fw_version(data);
	}

	if (input_dev->users) {
		data->mxt_drv_data->is_resuming = true;
		mxt_start(data);
		mxt_set_feature_status(data);
		data->mxt_drv_data->is_resuming = false;
	}

	if (data->after_work && data->charge_out) {
		ret = mxt_pw_lock(INCELL_DISPLAY_POWER_UNLOCK);
		if (ret)
			LOGN("failed unlock power\n");
	}

	mutex_unlock(&data->mxt_drv_data->dev_lock);
	return ret;
}

static int drm_notifier_callback(struct notifier_block *self, unsigned long event, void *data)
{
	struct drm_ext_event *evdata = (struct drm_ext_event *)data;
	struct mxt_data *ts = container_of(self, struct mxt_data, drm_notif);
	int blank;

	if (event != DRM_EXT_EVENT_BEFORE_BLANK &&
	    event != DRM_EXT_EVENT_AFTER_BLANK)
		return NOTIFY_DONE;

	if (unlikely(!evdata || !evdata->data)) {
		LOGN("%s: Bad screen state change notifier call.\n");
		return NOTIFY_DONE;
	}
	blank = *(int *)evdata->data;

	switch (blank) {
		case DRM_BLANK_POWERDOWN:
			if (event == DRM_EXT_EVENT_BEFORE_BLANK) {
				if (!ts->after_work || !ts->charge_out) {
					LOGN("not already sleep out\n");
					return 0;
				}

				if (mxt_drm_suspend(ts))
					LOGE("Failed mxt_drm_suspend\n");
				break;
			}
		case DRM_BLANK_UNBLANK:
			if (event == DRM_EXT_EVENT_AFTER_BLANK) {
				if (!ts->after_work) {
					if (mxt_init_recover(ts->client, ts))
						return 0;
				}
				if (ts->first_unblank) {
					ts->first_unblank = false;
					ts->charge_out = true;
				} else if (!ts->charge_out) {
					LOGN("not already sleep out\n");
					return 0;
				}

				if (mxt_drm_resume(ts))
					LOGE("Failed mxt_drm_resume\n");

			}
			break;
		default:
			break;
	}
	return 0;
}

static void mxt_shutdown(struct i2c_client *client)
{
	struct mxt_data *data = i2c_get_clientdata(client);

	LOGN("%s\n", __func__);

	if (data && data->irq) {
		touch_disable_irq(data);
	}
}

static struct of_device_id mxt_match_table[] = {
	{ .compatible = "atmel,mxt640u",},
	{ },
};

static const struct i2c_device_id mxt_id[] = {
	{ MXT_DEVICE_NAME, 0},
	{ }
};

MODULE_DEVICE_TABLE(i2c, mxt_id);

static struct dev_pm_ops touch_pm_ops = {
	.suspend = mxt_suspend,
	.resume = mxt_resume,
};

static struct i2c_driver mxt_driver = {
	.driver = {
		.name = "touch_atmel",
		.of_match_table = mxt_match_table,
		.owner = THIS_MODULE,
		.pm = &touch_pm_ops,
	},
	.probe	 = mxt_probe,
	.remove	 = mxt_remove,
	.shutdown = mxt_shutdown,
	.id_table = mxt_id,
};

static void async_mxt_init(void *data, async_cookie_t cookie)
{
	i2c_add_driver(&mxt_driver);
	return;
}


static int __init mxt_init(void)
{
	async_schedule(async_mxt_init, NULL);

	return 0;
}

static void __exit mxt_exit(void)
{
	i2c_del_driver(&mxt_driver);
}

static u8 t255_user[MXT_PATCH_USER_DATA_MAX];

struct touch_pos tpos_data;
struct touch_supp tsupp_data;

static void mxt_patch_init_userdata(void)
{
	memset(t255_user, 0, MXT_PATCH_USER_DATA_MAX);
}

static void mxt_patch_calibration(struct mxt_data *data)
{
	mxt_write_object(data, MXT_GEN_COMMAND_T6, MXT_COMMAND_CALIBRATE, 1);
}

static int mxt_patch_start_timer(struct mxt_data *data, u16 period)
{
	struct mxt_object* object = NULL;
	int ret = 0;
	u8 t61_reg[5] = {3, 1, 0, 0, 0};

	object = mxt_get_object(data, MXT_SPT_TIMER_T61);

	if (object != NULL) {
		t61_reg[3] = period & 0xFF;
		t61_reg[4] = (period >> 8) & 0xFF;

		ret = mxt_write_mem(data, object->start_address + (5 * data->patch.timer_id), 5, t61_reg);
		if (!ret) {
			__mxt_patch_debug(data, "START STAGE: %d TIMER[%d] %dms\n",
				data->patch.cur_stage, data->patch.timer_id, period);
		}
	} else {
		__mxt_patch_debug(data, "TIMER NOT SUPPORTED\n");
	}

	return ret;
}

static int mxt_patch_stop_timer(struct mxt_data *data)
{
	struct mxt_object* object = NULL;
	int ret = 0;
	u8 t61_reg[5] = {3, 2, 0, 0, 0};

	object = mxt_get_object(data, MXT_SPT_TIMER_T61);

	if (object!=NULL) {
		ret = mxt_write_mem(data, object->start_address + (5 * data->patch.timer_id), 5, t61_reg);
		if (!ret) {
			__mxt_patch_debug(data, "STOP TIMER[%d]\n", data->patch.timer_id);
		}
	} else {
		__mxt_patch_debug(data, "TIMER NOT SUPPORTED\n");
	}

	return ret;
}

static int mxt_patch_write_stage_cfg(struct mxt_data *data, struct stage_cfg* pscfg, bool do_action)
{
	if (do_action) {
		__mxt_patch_debug(data, "|- SCFG_WRITE: OBJECT_TYPE:%d OFFSET:%d VAL:%d OPT:%d\n",
			pscfg->obj_type, pscfg->offset, pscfg->val, pscfg->option);

		if (pscfg->obj_type == 255) {
			t255_user[pscfg->offset] = pscfg->val;
		} else {
			mxt_write_object(data, pscfg->obj_type, pscfg->offset, pscfg->val);
		}
	}

	return 0;
}

static int mxt_patch_write_action_cfg(struct mxt_data *data, struct action_cfg* pacfg, bool do_action)
{
	if (do_action) {
		__mxt_patch_debug(data, "|-- ACFG_WRITE: OBJECT_TYPE:%d OFFSET:%d VAL:%d OPT:%d\n",
			pacfg->obj_type, pacfg->offset, pacfg->val, pacfg->option);

		if (pacfg->obj_type == 255) {
			t255_user[pacfg->offset] = pacfg->val;
		} else {
			mxt_write_object(data, pacfg->obj_type, pacfg->offset, pacfg->val);
		}
	}

	return 0;
}

static int mxt_patch_write_trigger_cfg(struct mxt_data *data, struct trigger_cfg* ptcfg, bool do_action)
{
	if (do_action) {
		__mxt_patch_debug(data, "|-- TCFG_WRITE: OBJECT_TYPE:%d OFFSET:%d VAL:%d\n",
			ptcfg->obj_type, ptcfg->offset, ptcfg->val);

		if (ptcfg->obj_type == 255) {
			t255_user[ptcfg->offset] = ptcfg->val;
		} else {
			mxt_write_object(data, ptcfg->obj_type, ptcfg->offset, ptcfg->val);
		}
	}

	return 0;
}

static int mxt_patch_write_event_cfg(struct mxt_data *data, struct event_cfg* pecfg, bool do_action)
{
	if (do_action) {
		__mxt_patch_debug(data, "|-- ECFG_WRITE: OBJECT_TYPE:%d OFFSET:%d VAL:%d\n",
			pecfg->obj_type, pecfg->offset, pecfg->val);

		if (pecfg->obj_type == 99) {
			data->patch.start = false;
			data->patch.skip_test = 1;
			data->patch.run_stage = false;
			data->patch.start_stage = pecfg->val;
			__mxt_patch_debug(data, "Start stage change:%d\n", pecfg->val);
		} else if (pecfg->obj_type == 255) {
			t255_user[pecfg->offset] = pecfg->val;
		} else {
			return mxt_write_object(data, pecfg->obj_type, pecfg->offset, pecfg->val);
		}
	}
	return 0;
}

static int mxt_patch_predefined_action(struct mxt_data *data, u8 action_id, u16 action_val, bool do_action)
{
	if (do_action) {
		switch (action_id) {
			case MXT_PATCH_ACTION_NONE:
				__mxt_patch_debug(data, "|-- ACTION NONE\n");
				break;
			case MXT_PATCH_ACTION_CAL:
				__mxt_patch_debug(data, "|-- ACTION CALIBRATE: %d\n", action_val);
				mxt_patch_calibration(data);
				data->patch.start = false; // Wait Restart
				data->patch.start_stage = action_val;
				break;
			case MXT_PATCH_ACTION_EXTEND_TIMER:
				__mxt_patch_debug(data, "|-- ACTION EXTEND TIMER: %d\n", action_val);
				mxt_patch_start_timer(data, action_val);
				break;
			case MXT_PATCH_ACTION_GOTO_STAGE:
				__mxt_patch_debug(data, "|-- ACTION GOTO STAGE: %d\n", action_val);
				data->patch.skip_test = 1;
				data->patch.cur_stage = action_val;
				data->patch.run_stage = false;
				break;
			case MXT_PATCH_ACTION_CHANGE_START:
				__mxt_patch_debug(data, "|-- ACTION CHANGE START STAGE: %d\n", action_val);
				data->patch.start_stage = action_val;
				break;
			default:
				__mxt_patch_debug(data, "@@ INVALID ACTION ID=%d !!\n", action_id);
				return -1;
		}
	}
	return 0;
}

static void mxt_patch_init_tpos(struct mxt_data *data, struct touch_pos* tpos)
{
	int i = 0;

	for (i = 0; i < MXT_MAX_FINGER; i++) {
		tpos->tcount[i] = 0;
		tpos->initx[i] = 0;
		tpos->inity[i] = 0;
		tpos->oldx[i] = 0;
		tpos->oldy[i] = 0;
	}
	tpos->locked_id = 0xff;
	tpos->moved_cnt = 0;
}

static bool mxt_patch_check_locked(struct mxt_data *data, struct touch_pos* tpos, u8 tid, u16 x, u16 y)
{
	s16 diffx = 0, diffy = 0;
	u32 distance = 0;

	//OLD DIFF
	diffx = x - tpos->oldx[tid];
	diffy = y - tpos->oldy[tid];
	distance = abs(diffx) + abs(diffy);

	// INIT DIFF
	if ((tpos->initx[tid] != 0) && (tpos->inity[tid] != 0)) {
		diffx = x - tpos->initx[tid];
		diffy = y - tpos->inity[tid];
		__mxt_patch_ddebug(data, "[TPOS] INITDIFF[%d] ABS X=%d, ABS Y=%d\n", tid, (int)abs(diffx), (int)abs(diffy));
	}

	if ((tpos->initx[tid] == 0) && (tpos->inity[tid] == 0)) {
		__mxt_patch_ddebug(data, "[TPOS] INITSET[%d] X=%d, Y=%d\n", tid, x, y);
		tpos->initx[tid] = x;
		tpos->inity[tid] = y;
		tpos->moved_cnt = 0;
	} else {
		// OLD DIFF vs INIT DIFF
		if ((distance < tpos->jitter) && ((abs(diffx) > tpos->maxdiff) || (abs(diffy) > tpos->maxdiff))) {
			tpos->moved_cnt++;
		}
	}

	if (tpos->moved_cnt > tpos->reset_cnt) {
		__mxt_patch_ddebug(data, "[TPOS] RESET[%d] X=%d, Y=%d\n", tid, x, y);
		tpos->initx[tid] = x;
		tpos->inity[tid] = y;
		tpos->moved_cnt = 0;
	}

	if ((distance < tpos->distance) && (abs(diffx) < tpos->maxdiff) && (abs(diffy) < tpos->maxdiff)) {
		return true;
	} else {
		return false;
	}

	return false;
}

static void mxt_patch_check_pattern(struct mxt_data *data, struct touch_pos* tpos, u8 tid, u16 x, u16 y, u8 finger_cnt)
{
	bool cal_condition = false;
	int error = 0;

	if (!finger_cnt) {
		return;
	}

	if (mxt_patch_check_locked(data, tpos, tid, x, y)) {
		tpos->tcount[tid] = tpos->tcount[tid]+1;
	} else {
		tpos->tcount[tid] = 0;
	}

	tpos->oldx[tid] = x;
	tpos->oldy[tid] = y;

	if (finger_cnt == 1) {
		if (tpos->tcount[tid] > tpos->locked_cnt) {
			__mxt_patch_debug(data, "[TPOS] ONE TOUCH LOCKED\n");
			mxt_patch_init_tpos(data, tpos);
			cal_condition = true;
		}
	} else {
		if ((tpos->tcount[tid] > tpos->locked_cnt) && tpos->locked_id != tid && tpos->locked_id != 0xff) {
			__mxt_patch_debug(data, "[TPOS] TWO TOUCH LOCKED [%d, %d]\n", tid, tpos->locked_id);
			mxt_patch_init_tpos(data, tpos);
			cal_condition = true;
		}

		if (tpos->tcount[tid] > tpos->locked_cnt) {
			tpos->locked_id = tid;
			if (tpos->tcount[tid] >= 0xFF) {
				__mxt_patch_debug(data, "[TPOS] OVER LOCKED\n");
				mxt_patch_init_tpos(data, tpos);
				cal_condition = true;
			}
		}
	}

	if (cal_condition) {
		error = mxt_read_object(data, MXT_SPT_DYNAMICCONFIGURATIONCONTAINER_T71,
			MXT_PATCH_T71_PTN_CAL, &tpos->cal_enable);
		if (error) {
			TOUCH_PATCH_INFO_MSG( "%s: Error read T71 [%d]\n", __func__, error);
		} else {
			if (tpos->cal_enable) {
				__mxt_patch_debug(data, "[TPOS] CAL\n");
				mxt_patch_calibration(data);

				error = mxt_read_object(data, MXT_SPT_DYNAMICCONFIGURATIONCONTAINER_T71,
					MXT_PATCH_T71_PTN_OPT, &tpos->option);

				if (!error) {
					if (tpos->option & 0x01) { // Onetime
						mxt_write_object(data, MXT_SPT_DYNAMICCONFIGURATIONCONTAINER_T71,
							MXT_PATCH_T71_PTN_CAL, 0);
						__mxt_patch_debug(data, "[TPOS] DISABLE T71[2]\n");
					}
				}
			} else {
				__mxt_patch_debug(data, "[TPOS] SKIP CAL T71[2]=0\n");
			}
		}
	}
}

static void mxt_patch_init_supp(struct mxt_data *data, struct touch_supp* tsup)
{
	tsup->old_time = jiffies_to_msecs(jiffies);
	tsup->repeat_cnt = 0;
}

static void mxt_patch_check_supp(struct mxt_data *data, struct touch_supp* tsup)
{
	u32 curr_time = jiffies_to_msecs(jiffies);
	u32 time_diff = 0;

	time_diff = TIME_WRAP_AROUND(tsup->old_time, curr_time);

	if (time_diff < tsup->time_gap*100) {
		__mxt_patch_debug(data, "[TSUP] Abnormal suppress %d\n", tsup->repeat_cnt);

		if (tsup->repeat_cnt++ > tsup->repeat_max) {
			__mxt_patch_debug(data, "[TSUP] Abnormal suppress detected\n");
			mxt_patch_calibration(data);
		}
	} else {
		tsup->repeat_cnt = 0;
		__mxt_patch_debug(data, "[TSUP] Normal suppress\n");
	}

	tsup->old_time = curr_time;
	mxt_patch_dump_source(data, true);
}

static void mxt_patch_load_t71data(struct mxt_data *data)
{
	struct mxt_object *obj = NULL;
	u8 buf[MXT_PATCH_T71_DATA_MAX] = {0};
	struct touch_pos* tpos = &tpos_data;
	int error = 0;

	obj = mxt_get_object(data, MXT_SPT_DYNAMICCONFIGURATIONCONTAINER_T71);
	if (obj) {
		error = mxt_read_mem(data, obj->start_address,MXT_PATCH_T71_DATA_MAX, buf);

		if (!error) {
			tpos->option = buf[MXT_PATCH_T71_PTN_OPT];
			tpos->cal_enable = buf[MXT_PATCH_T71_PTN_CAL];
			tpos->reset_cnt = buf[3];
			tpos->distance = buf[4];
			tpos->maxdiff = buf[5];
			tpos->locked_cnt = buf[6];
			tpos->jitter = buf[7];
			tpos->amp = buf[8];
			tpos->area = buf[9];
			tpos->sum_size_t57 = buf[10];
			tpos->tch_count_t57 = buf[11];
			tpos->atch_count_t57 = buf[12];
			tpos->amp_2finger_min = buf[13];
			tpos->area_2finger_min = buf[14];
			tpos->sum_size_t57_2finger_min = buf[15];
			tpos->tch_count_t57_2finger_min = buf[16];
			tpos->atch_count_t57_2finger_min = buf[17];
			tpos->amp_2finger_max = buf[18];
			tpos->area_2finger_max = buf[19];
			tpos->sum_size_t57_2finger_max = buf[20];
			tpos->tch_count_t57_2finger_max = buf[21];
			tpos->atch_count_t57_2finger_max = buf[22];

			tpos->amp_3finger_min = buf[23];
			tpos->area_3finger_min = buf[24];
			tpos->sum_size_t57_3finger_min = buf[25];
			tpos->tch_count_t57_3finger_min = buf[26];
			tpos->atch_count_t57_3finger_min = buf[27];
			tpos->amp_3finger_max = buf[28];
			tpos->area_3finger_max = buf[29];
			tpos->sum_size_t57_3finger_max = buf[30];
			tpos->tch_count_t57_3finger_max = buf[31];
			tpos->atch_count_t57_3finger_max = buf[32];

			tpos->amp_mfinger_min = buf[33];
			tpos->area_mfinger_min = buf[34];
			tpos->sum_size_t57_mfinger_min = buf[35];
			tpos->tch_count_t57_mfinger_min = buf[36];
			tpos->atch_count_t57_mfinger_min = buf[37];
			tpos->amp_mfinger_max = buf[38];
			tpos->area_mfinger_max = buf[39];
			tpos->sum_size_t57_mfinger_max = buf[40];
			tpos->tch_count_t57_mfinger_max = buf[41];
			tpos->atch_count_t57_mfinger_max = buf[42];

			tpos->xlo_limit = buf[43];
			tpos->xhi_limit = (buf[44]<<8) | buf[45];
			tpos->ylo_limit = buf[46];
			tpos->yhi_limit = (buf[47]<<8) | buf[48];
			__mxt_patch_debug(data, "PTN CAL %d RST %d DST %d DIF %d CNT %d JIT %d\n",
				tpos->cal_enable, tpos->reset_cnt, tpos->distance, tpos->maxdiff, tpos->locked_cnt, tpos->jitter);

			TOUCH_PATCH_INFO_MSG("PTN CAL %d RST %d DST %d DIF %d CNT %d JIT %d\n",
				tpos->cal_enable, tpos->reset_cnt, tpos->distance, tpos->maxdiff, tpos->locked_cnt, tpos->jitter);

			tsupp_data.time_gap = buf[49];
			tsupp_data.repeat_max = buf[50];

			__mxt_patch_debug(data, "SUPP GAP %d*100ms CNT %d\n",
				tsupp_data.time_gap, tsupp_data.repeat_max);
		}
	}
}

const char* mxt_patch_src_item_name(u8 src_id)
{
	const char* src_item_name[MXT_PATCH_MAX_TYPE] = {
		MXT_XML_SRC_NONE,	//MXT_PATCH_ITEM_NONE		0
		MXT_XML_SRC_CHRG,	//MXT_PATCH_ITEM_CHARGER	1
		MXT_XML_SRC_FCNT,	//MXT_PATCH_ITEM_FINGER_CNT	2
		MXT_XML_SRC_AREA,	//MXT_PATCH_ITEM_T9_AREA	3
		MXT_XML_SRC_AMP,	//MXT_PATCH_ITEM_T9_AMP		4
		MXT_XML_SRC_SUM,	//MXT_PATCH_ITEM_T57_SUM	5
		MXT_XML_SRC_TCH,	//MXT_PATCH_ITEM_T57_TCH	6
		MXT_XML_SRC_ATCH,	//MXT_PATCH_ITEM_T57_ATCH	7
		MXT_XML_SRC_KCNT,	//MXT_PATCH_ITEM_KCNT		8
		MXT_XML_SRC_KVAL,	//MXT_PATCH_ITEM_KVAL		9
		MXT_XML_SRC_T9STATUS,	//MXT_PATCH_ITEM_T9STATUS	10
		MXT_XML_SRC_USER1,
		MXT_XML_SRC_USER2,
		MXT_XML_SRC_USER3,
		MXT_XML_SRC_USER4,
		MXT_XML_SRC_USER5,
	};

	if (MXT_PATCH_ITEM_NONE <= src_id && src_id < MXT_PATCH_ITEM_END) {
		return src_item_name[src_id];
	}

	return "ERR";
}

const char* mxt_patch_cond_name(u8 con_id)
{
	const char* cond_name[MXT_PATCH_MAX_CON] = {
		MXT_XML_CON_NONE,	//MXT_PATCH_CON_NONE	0
		MXT_XML_CON_EQUAL,	//MXT_PATCH_CON_EQUAL	1
		MXT_XML_CON_BELOW,	//MXT_PATCH_CON_BELOW	2
		MXT_XML_CON_ABOVE,	//MXT_PATCH_CON_ABOVE	3
		MXT_XML_CON_PLUS,	//MXT_PATCH_CON_PLUS	4
		MXT_XML_CON_MINUS,	//MXT_PATCH_CON_MINUS	5
		MXT_XML_CON_MUL,	//MXT_PATCH_CON_MUL	6
		MXT_XML_CON_DIV,	//MXT_PATCH_CON_DIV	7
		MXT_XML_CON_MASK,	//MXT_PATCH_CON_MASK	8
	};

	if (MXT_PATCH_CON_NONE <= con_id && con_id < MXT_PATCH_CON_END) {
		return cond_name[con_id];
	}

	return "ERR";
}

static int mxt_patch_item_lval(struct mxt_data *data, u16* psrc_item, u8 src_id)
{
	if (psrc_item != NULL) {
		if (MXT_PATCH_ITEM_NONE <= src_id && src_id < MXT_PATCH_ITEM_END) {
			return psrc_item[src_id];
		} else {
			__mxt_patch_debug(data, "@@ INVALID ITEM ID=%d !!\n", src_id);
		}
	}

	return 0;
}

static int mxt_patch_item_rval(struct mxt_data *data, u16* psrc_item, struct item_val ival)
{
	int lval = mxt_patch_item_lval(data, psrc_item, ival.val_id);
	int rval = ival.val;

	switch (ival.val_eq) {
		case MXT_PATCH_CON_NONE:
			return lval ? lval : rval;
		case MXT_PATCH_CON_PLUS:
			lval += rval;
			break;
		case MXT_PATCH_CON_MINUS:
			lval -= rval;
			break;
		case MXT_PATCH_CON_MUL:
			lval *= rval;
			break;
		case MXT_PATCH_CON_DIV:
			lval /= rval;
			break;
		default:
			if (psrc_item) {
				__mxt_patch_debug(data, "@@ INVALID VAL_EQ=%d (LVAL=%d) => RVAL=%d !!\n",
					ival.val_eq, lval, rval);
			}
			return rval;
	}
	return lval;
}

static int mxt_patch_item_check(struct mxt_data *data, u16* psrc_item, struct test_item* ptitem, bool do_action)
{
	int lval = mxt_patch_item_lval(data, psrc_item, ptitem->src_id);
	int rval = mxt_patch_item_rval(data, psrc_item, ptitem->ival);

	if (!do_action) {
		__mxt_patch_debug(data, "|-- ITEM SRC_ID:%s COND:%s VAL_ID:%s EQ:%s VAL:%d\n",
			mxt_patch_src_item_name(ptitem->src_id), mxt_patch_cond_name(ptitem->cond),
			mxt_patch_src_item_name(ptitem->ival.val_id), mxt_patch_cond_name(ptitem->ival.val_eq), ptitem->ival.val);
	}

	if (psrc_item) {
		switch (ptitem->cond) {
			case MXT_PATCH_CON_EQUAL:
				__mxt_patch_ddebug(data, "|--- IF %s: %d == %d = %d\n",
					mxt_patch_src_item_name(ptitem->src_id), lval, rval, lval == rval ? 1 : 0);
				return lval == rval ? 1 : 0;
			case MXT_PATCH_CON_BELOW:
				__mxt_patch_ddebug(data, "|--- IF %s: %d < %d = %d\n",
					mxt_patch_src_item_name(ptitem->src_id), lval, rval, lval < rval ? 1 : 0);
				return lval < rval ? 1 : 0;
			case MXT_PATCH_CON_ABOVE:
				__mxt_patch_ddebug(data, "|--- IF %s: %d > %d = %d\n",
					mxt_patch_src_item_name(ptitem->src_id), lval, rval, lval > rval ? 1 : 0);
				return lval > rval ? 1 : 0;
			case MXT_PATCH_CON_MASK:
				__mxt_patch_ddebug(data, "|--- IF %s: %d & %d = %d\n",
					mxt_patch_src_item_name(ptitem->src_id), lval, rval, lval & rval ? 1 : 0);
				return lval & rval ? 1 : 0;
			default:
				__mxt_patch_debug(data, "@@ INVALID TEST COND=%d !!\n", ptitem->cond);
				return -1;
		}
	}
	return -1;
}

static int mxt_patch_stage_timer(struct mxt_data *data, u16 period, bool do_action)
{
	int ret = 0;
	u32 time = period * 10;

	if (do_action) {
		ret = mxt_patch_start_timer(data, time);
		if (!ret) {
			data->patch.period = period;
		}
	}
	return 0;
}

void mxt_patch_dump_source(struct mxt_data *data, bool do_action)
{
	if (do_action) {
		__mxt_patch_debug(data, "TA:%d FCNT:%d AREA:%d AMP:%d"
			" SUM:%d TCH:%d ATCH:%d KCNT:%d KVAL:%d S:%d U1:%d U2:%d U3:%d U4:%d U5:%d U6:%d Charger : %d\n",
			data->patch.src_item[1], data->patch.src_item[2],
			data->patch.src_item[3], data->patch.src_item[4],
			data->patch.src_item[5], data->patch.src_item[6],
			data->patch.src_item[7], data->patch.src_item[8],
			data->patch.src_item[9], data->patch.src_item[10],
			data->patch.src_item[11], data->patch.src_item[12],
			data->patch.src_item[13], data->patch.src_item[14],
			data->patch.src_item[15], data->patch.src_item[16], data->charging_mode);
	}
}

static int mxt_patch_parse_test_line(struct mxt_data *data, u8* ppatch, u16* psrc_item, u16* check_cnt, bool do_action)
{
	struct test_line* ptline = NULL;
	struct test_item* ptitem = NULL;
	struct action_cfg* pacfg = NULL;
	u32 i = 0, ulpos = 0;
	u8 test_result = 0;
	bool test_action = false;

	ptline = (struct test_line*)ppatch;

	if (!do_action) {
		__mxt_patch_debug(data, "|- TEST_LINE:%X OPT:%d CHK_CNT:%d ITEM_CNT:%d CFG_CNT:%d ACTION:%d VAL:%d\n",
			ptline->test_id, ptline->option, ptline->check_cnt, ptline->item_cnt, ptline->cfg_cnt, ptline->act_id, ptline->act_val);
	}

	ulpos += sizeof(struct test_line);

	test_result = 0;
	test_action = false;

	for (i = 0; i < ptline->item_cnt; i++) { /* Test Item Parsing */
		ptitem = (struct test_item*)(ppatch+ulpos);

		if (mxt_patch_item_check(data, psrc_item,ptitem, do_action) > 0) {
			test_result++;

			if (test_result == ptline->item_cnt) {
				if (check_cnt != NULL) {
					*check_cnt = *check_cnt + 1;

					if (*check_cnt == ptline->check_cnt) {
						test_action = true;

						TOUCH_PATCH_INFO_MSG("STAGE:%d TEST %d MATCHED\n", data->patch.cur_stage, ptline->test_id);
						TOUCH_PATCH_INFO_MSG("TA:%d FCNT:%d AREA:%d AMP:%d"
									" SUM:%d TCH:%d ATCH:%d KCNT:%d KVAL:%d S:%d U1:%d U2:%d U3:%d U4:%d U5:%d U6:%d Charger : %d\n",
									data->patch.src_item[1], data->patch.src_item[2],
									data->patch.src_item[3], data->patch.src_item[4],
									data->patch.src_item[5], data->patch.src_item[6],
									data->patch.src_item[7], data->patch.src_item[8],
									data->patch.src_item[9], data->patch.src_item[10],
									data->patch.src_item[11], data->patch.src_item[12],
									data->patch.src_item[13], data->patch.src_item[14],
									data->patch.src_item[15], data->patch.src_item[16], data->charging_mode);

						mxt_patch_dump_source(data, test_action);

						if (ptline->option & 0x01) {
							*check_cnt = 0;
							__mxt_patch_ddebug(data, "CHEK CNT CLEAR\n");
						}
					}
				}
			}
		} else {
			if (data->patch.option & 0x04) {
				if (do_action && psrc_item) {// Skip if any item was failed
					__mxt_patch_ddebug(data, "SKIP REMAINED ITEMS %d\n", i);
					return 0;
				}
			}
		}
		ulpos += sizeof(struct test_item);
	}

	for (i = 0; i <ptline->cfg_cnt; i++) { /* Test Line Action config */
		pacfg = (struct action_cfg*)(ppatch+ulpos);
		if (!do_action) {
			__mxt_patch_debug(data, "|-- ACTION_CFG: OBJ:%d OFFSET:%d VAL:%d OPT:%d\n",
				pacfg->obj_type, pacfg->offset, pacfg->val, pacfg->option);
		}
		mxt_patch_write_action_cfg(data, pacfg, test_action);
		ulpos += sizeof(struct action_cfg);
	}
	mxt_patch_predefined_action(data, ptline->act_id, ptline->act_val, test_action);

	return ulpos;
}

static int mxt_patch_parse_stage(struct mxt_data *data, u8* ppatch, u16* ptline_addr, u8* ptline_cnt, bool do_action)
{
	struct stage_def* psdef = NULL;
	struct stage_cfg* pscfg = NULL;
	u32 i = 0, ulpos = 0;

	psdef = (struct stage_def*)ppatch;

	if (!do_action) {
		__mxt_patch_debug(data,
			"STAGE_ID:%d OPT:%d PERIOD:%d CFG_CNT:%d TST_CNT:%d RESET:%d\n",
			psdef->stage_id, psdef->option, psdef->stage_period,
			psdef->cfg_cnt, psdef->test_cnt, psdef->reset_period);
	}

	mxt_patch_stage_timer(data, psdef->stage_period, do_action);
	ulpos += sizeof(struct stage_def);

	for (i = 0; i < psdef->cfg_cnt; i++) { /* Stage Config Parsing */
		pscfg = (struct stage_cfg*)(ppatch+ulpos);

		if (!do_action) {
			__mxt_patch_debug(data, "|- STAGE_CFG: OBJ:%d OFFSET:%d VAL:%d OPT:%d\n",
				pscfg->obj_type, pscfg->offset, pscfg->val, pscfg->option);
		}
		mxt_patch_write_stage_cfg(data, pscfg, do_action);
		ulpos += sizeof(struct stage_cfg);
	}

	for (i = 0; i < psdef->test_cnt; i++) { /* Test Line Parsing */
		if (ptline_addr != NULL) {
			ptline_addr[i] = (u16)ulpos;
		}
		ulpos += mxt_patch_parse_test_line(data, ppatch+ulpos, NULL, NULL, do_action);
	}

	if (ptline_cnt != NULL)
		*ptline_cnt = psdef->test_cnt;

	return ulpos;
}

static u16 mxt_patch_match_lval(struct mxt_data *data, u8* pmsg, u8 offset, u16 mask)
{
	u16 lval = 0;
	u8 msg[MXT_PATCH_MAX_MSG_SIZE+1] = {0};

	if (pmsg) {
		if (offset >= 200 && offset <= 255) {
			return t255_user[offset-200];
		}
		memcpy(msg, pmsg, MXT_PATCH_MAX_MSG_SIZE);
		if (0 <= offset && offset < MXT_PATCH_MAX_MSG_SIZE) {
			lval = msg[offset] | (msg[offset+1] << 8);
			return mask ? lval & mask : lval;
		} else {
			__mxt_patch_debug(data, "@@ INVALID OFFSET=%d !!\n", offset);
		}
	}

	return 0;
}

static int mxt_patch_match_check(struct mxt_data *data, u8* pmsg, struct match* pmatch, bool do_action)
{
	u16 lval = mxt_patch_match_lval(data, pmsg, pmatch->offset, pmatch->mask);
	u16 rval = pmatch->val;

	if (pmsg) {
		switch (pmatch->cond) {
			case MXT_PATCH_CON_EQUAL:
				__mxt_patch_ddebug(data, "|--- IF %d == %d = %d\n", lval, rval, lval == rval ? 1 : 0);
				return lval == rval ? 1 : 0;
			case MXT_PATCH_CON_BELOW:
				__mxt_patch_ddebug(data, "|--- IF %d < %d = %d\n", lval, rval, lval < rval ? 1 : 0);
				return lval < rval ? 1 : 0;
			case MXT_PATCH_CON_ABOVE:
				__mxt_patch_ddebug(data, "|--- IF %d > %d = %d\n", lval, rval, lval > rval ? 1 : 0);
				return lval > rval ? 1 : 0;
			default:
				__mxt_patch_debug(data, "@@ INVALID MATCH COND=%d !!\n", pmatch->cond);
				return -1;
		}
	}

	return -1;
}

static int mxt_patch_trigger_check(struct mxt_data *data, u8 object, u8 index, u8* pmsg)
{
	u8 reportid = pmsg[0];
	u8 type = 0, id = 0;

	type = data->reportids[reportid].type;
	id = data->reportids[reportid].index;

	if ((type == object) && (id == index))
		return 0;

	return 1;
}

static int mxt_patch_parse_trigger(struct mxt_data *data, u8* ppatch, u8* pmsg, bool do_action, u8 option)
{
	struct trigger* ptrgg = NULL;
	struct match* pmatch = NULL;
	struct trigger_cfg* ptcfg = NULL;
	u32 i = 0, ulpos = 0;
	u8 match_result = 0;
	u8 trigger_action = 0;

	ptrgg = (struct trigger*)ppatch;

	if (!do_action) {
		__mxt_patch_debug(data, "TRIGGER ID:%d OPT:%d OBJ:%d IDX:%d MATCH:%d CFG:%d ACT:%d VAL:%d\n",
			ptrgg->tid, ptrgg->option, ptrgg->object, ptrgg->index, ptrgg->match_cnt, ptrgg->cfg_cnt, ptrgg->act_id, ptrgg->act_val);
	}

	ulpos += sizeof(struct trigger);

	// Message Filter
	if (do_action) {
		if (mxt_patch_trigger_check(data, ptrgg->object, ptrgg->index, pmsg))
			return 1;
	}

	// Match Parsing
	match_result = 0;
	trigger_action = false;
	for (i = 0; i < ptrgg->match_cnt; i++) {
		pmatch = (struct match*)(ppatch+ulpos);
		if (!do_action) {
			__mxt_patch_debug(data, "|- MATCH:%d OFFSET:%d MASK:%d COND:%s VAL:%d\n",
				i, pmatch->offset, pmatch->mask, mxt_patch_cond_name(pmatch->cond), pmatch->val);
		}

		if (mxt_patch_match_check(data, pmsg, pmatch, do_action) > 0) {
			match_result++;
			if (match_result == ptrgg->match_cnt) {
				if (option == ptrgg->option)
					trigger_action = true;
			}
		}
		ulpos += sizeof(struct match);
	}

	// Trigger Config Parsing
	for (i = 0; i < ptrgg->cfg_cnt; i++) {
		ptcfg = (struct trigger_cfg*)(ppatch+ulpos);

		if (!do_action) {
			__mxt_patch_debug(data, "|- TRIGGER_CFG: OBJECT_TYPE:%d OFFSET:%d VAL:%d\n",
				ptcfg->obj_type, ptcfg->offset, ptcfg->val);
		}
		mxt_patch_write_trigger_cfg(data, ptcfg, trigger_action);
		ulpos += sizeof(struct trigger_cfg);
	}
	// Predefined Action
	mxt_patch_predefined_action(data, ptrgg->act_id, ptrgg->act_val, trigger_action);

	return ulpos;
}

int mxt_patch_parse_event(struct mxt_data *data, u8* ppatch, bool do_action)
{
	struct user_event* pevent = NULL;
	struct event_cfg* pecfg = NULL;
	u32 i = 0, ulpos = 0;
	int error = 0;

	pevent = (struct user_event*)ppatch;

	if (!do_action) {
		__mxt_patch_debug(data, "EVENT ID:%d OPT:%d CFG:%d\n",
			pevent->eid, pevent->option, pevent->cfg_cnt);
	}
	ulpos += sizeof(struct user_event);

	// Event Config Parsing
	for (i = 0; i < pevent->cfg_cnt; i++) {
		pecfg = (struct event_cfg*)(ppatch+ulpos);
		if (!do_action) {
			__mxt_patch_debug(data, "|- EVENT_CFG: OBJECT_TYPE:%d OFFSET:%d VAL:%d\n",
				pecfg->obj_type, pecfg->offset, pecfg->val);
		}
		error = mxt_patch_write_event_cfg(data, pecfg, do_action);
		if (error)
			i = pevent->cfg_cnt+1;

		ulpos += sizeof(struct event_cfg);
	}

	return ulpos;
}

static int mxt_patch_parse_header(struct mxt_data *data, u8* ppatch, u16* pstage_addr, u16* ptrigger_addr, u16* pevent_addr)
{
	struct patch_header* ppheader = NULL;
	u32 i = 0, ulpos = 0;

	ppheader = (struct patch_header*)ppatch;

	TOUCH_PATCH_INFO_MSG("%s\n", __func__);

	TOUCH_PATCH_INFO_MSG( "PATCH MAGIC:%X SIZE:%d DATE:%d VER:%d OPT:%d DBG:%d TMR:%d STG:%d TRG:%d EVT:%d\n",
		ppheader->magic, ppheader->size, ppheader->date, ppheader->version, ppheader->option, ppheader->debug,
		ppheader->timer_id, ppheader->stage_cnt, ppheader->trigger_cnt, ppheader->event_cnt);

	if (ppheader->version != MXT_PATCH_VERSION) {
		TOUCH_PATCH_INFO_MSG( "MXT_PATCH_VERSION ERR\n");
	}

	ulpos = sizeof(struct patch_header);

	for (i = 0; i < ppheader->stage_cnt; i++) { /* Stage Def Parsing */
		if (pstage_addr != NULL) {
			pstage_addr[i] = (u16)ulpos;
		}
		ulpos += mxt_patch_parse_stage(data, ppatch+ulpos, NULL, NULL, false);
	}

	for (i = 0; i < ppheader->trigger_cnt; i++) { /* Trigger Parsing */
		if (ptrigger_addr != NULL) {
			ptrigger_addr[i] = (u16)ulpos;
		}
		ulpos += mxt_patch_parse_trigger(data, ppatch+ulpos, NULL, false, 0);
	}

	for (i = 0; i < ppheader->event_cnt; i++) { /* Event */
		if (pevent_addr != NULL) {
			pevent_addr[i] = (u16)ulpos;
		}
		ulpos += mxt_patch_parse_event(data, ppatch+ulpos, false);
	}

	if (ppheader->size != ulpos) { /* Patch Size Check */
		TOUCH_PATCH_INFO_MSG("Size Error %d != %d\n", ppheader->size, ulpos);
		return 0;
	} else {
		TOUCH_PATCH_INFO_MSG("Size OK= %d\n", ulpos);
	}

	return ulpos;
}

int mxt_patch_run_stage(struct mxt_data *data)
{
	struct stage_def* psdef = NULL;
	u8* ppatch = data->patch.patch;
	u16* pstage_addr = data->patch.stage_addr;
	u16 tline_addr[MXT_PATCH_MAX_TLINE] = {0};
	u8 tline_cnt = 0;
	u8 cur_stage = data->patch.cur_stage;

	__mxt_patch_debug(data, "RUN STAGE:%d\n", cur_stage);

	if (unlikely(!ppatch || !pstage_addr)) {
		TOUCH_PATCH_INFO_MSG( "%s pstage_addr is null\n", __func__);
		return 1;
	}

	psdef = (struct stage_def*)(ppatch+pstage_addr[cur_stage]);
	data->patch.cur_stage_opt = psdef->option;

	mxt_patch_parse_stage(data, (u8*)psdef, tline_addr, &tline_cnt, true);

	if (unlikely(!data->patch.tline_addr)) {
		data->patch.tline_addr = kzalloc(MXT_PATCH_MAX_TLINE, GFP_KERNEL);
	}

	if (unlikely(!data->patch.check_cnt)) {
		data->patch.check_cnt = kzalloc(MXT_PATCH_MAX_TLINE, GFP_KERNEL);
	}

	if (unlikely(!data->patch.tline_addr || !data->patch.check_cnt)) {
		TOUCH_PATCH_INFO_MSG( "tline_addr alloc error\n");
		return 1;
	}

	memcpy(data->patch.tline_addr, tline_addr, tline_cnt*sizeof(u16));
	memset(data->patch.check_cnt, 0, tline_cnt*sizeof(u16));
	data->patch.tline_cnt = tline_cnt;
	data->patch.run_stage = 1;
	data->patch.skip_test = 0;
	data->patch.stage_timestamp = jiffies_to_msecs(jiffies);
	__mxt_patch_ddebug(data, "Stage[%d] %d\n", cur_stage, data->patch.stage_timestamp);

	return 0;
}

static int mxt_patch_test_source(struct mxt_data *data, u16* psrc_item)
{
	int i = 0;
	u8* ppatch = data->patch.patch;
	u16* pstage_addr = data->patch.stage_addr;
	u8	cur_stage = data->patch.cur_stage;
	u32 curr_time = jiffies_to_msecs(jiffies);
	u32 time_diff = TIME_WRAP_AROUND(data->patch.stage_timestamp, curr_time);
	struct stage_def* psdef = NULL;
	u16* ptline_addr = NULL;
	u16* pcheck_cnt = NULL;

	if (!ppatch || !pstage_addr) {
		TOUCH_PATCH_INFO_MSG( "%s pstage_addr is null\n", __func__);
		return 1;
	}

	if (!data->patch.run_stage) {
		mxt_patch_run_stage(data);
	}

	if (data->patch.run_stage) {
		for (i = 0; i< data->patch.tline_cnt; i++) {
			ptline_addr = data->patch.tline_addr;
			pcheck_cnt = data->patch.check_cnt;

			if (!ptline_addr || !pcheck_cnt) {
				TOUCH_PATCH_INFO_MSG( "ptline_addr is null\n");
				return 1;
			}

			__mxt_patch_ddebug(data, "STAGE:%d, TEST:%d\n", cur_stage, i);

			mxt_patch_parse_test_line(data, ppatch+pstage_addr[cur_stage]+ptline_addr[i],
				psrc_item, &pcheck_cnt[i], true);

			psdef = (struct stage_def*)(ppatch+pstage_addr[cur_stage]);
			if (psdef->reset_period) {
				if (time_diff > psdef->reset_period*10) {
					pcheck_cnt[i] = 0;
					__mxt_patch_ddebug(data, "RESET CNT STAGE:%d, TEST:%d RESET:%d DIF:%d\n",
						cur_stage, i, psdef->reset_period, time_diff);
					data->patch.stage_timestamp = jiffies_to_msecs(jiffies);
				}
			}

			if (data->patch.skip_test) {
				__mxt_patch_debug(data, "REMAINED TEST SKIP\n");
				return 0;
			}
		}
	}

	return 0;
}

static void mxt_patch_init_tsrc(struct test_src* tsrc)
{
	tsrc->charger = -1;
	tsrc->finger_cnt = -1;
	tsrc->area = -1;
	tsrc->amp = -1;
	tsrc->sum_size = -1;
	tsrc->tch_ch = -1;
	tsrc->atch_ch = -1;
	tsrc->key_cnt = -1;
	tsrc->key_val = -1;
	tsrc->status = -1;
	tsrc->user1 = t255_user[0];
	tsrc->user2 = t255_user[1];
	tsrc->user3 = t255_user[2];
	tsrc->user4 = t255_user[3];
	tsrc->user5 = t255_user[4];
	tsrc->user6 = t255_user[5];
}

static int mxt_patch_make_source(struct mxt_data *data, struct test_src* tsrc)
{
	if (tsrc->charger >= 0)
		data->patch.src_item[MXT_PATCH_ITEM_CHARG] = tsrc->charger;
	if (tsrc->finger_cnt >= 0)
		data->patch.src_item[MXT_PATCH_ITEM_FCNT] = tsrc->finger_cnt;
	if (tsrc->area >= 0)
		data->patch.src_item[MXT_PATCH_ITEM_AREA] = tsrc->area;
	if (tsrc->amp >= 0)
		data->patch.src_item[MXT_PATCH_ITEM_AMP] = tsrc->amp;
	if (tsrc->sum_size >= 0)
		data->patch.src_item[MXT_PATCH_ITEM_SUM] = tsrc->sum_size;
	if (tsrc->tch_ch >= 0)
		data->patch.src_item[MXT_PATCH_ITEM_TCH] = tsrc->tch_ch;
	if (tsrc->atch_ch >= 0)
		data->patch.src_item[MXT_PATCH_ITEM_ATCH] = tsrc->atch_ch;
	if (tsrc->key_cnt >= 0)
		data->patch.src_item[MXT_PATCH_ITEM_KCNT] = tsrc->key_cnt;
	if (tsrc->key_val >= 0)
		data->patch.src_item[MXT_PATCH_ITEM_KVAL] = tsrc->key_val;
	if (tsrc->status >= 0)
		data->patch.src_item[MXT_PATCH_ITEM_T9STATUS] = tsrc->status;
	if (tsrc->user1 >= 0)
		data->patch.src_item[MXT_PATCH_ITEM_USER1] = tsrc->user1;
	if (tsrc->user2 >= 0)
		data->patch.src_item[MXT_PATCH_ITEM_USER2] = tsrc->user2;
	if (tsrc->user3 >= 0)
		data->patch.src_item[MXT_PATCH_ITEM_USER3] = tsrc->user3;
	if (tsrc->user5 >= 0)
		data->patch.src_item[MXT_PATCH_ITEM_USER5] = tsrc->user5;

	return 0;
}

static int mxt_patch_start_stage(struct mxt_data *data)
{
	if (data->patch.patch) {
		mxt_patch_stop_timer(data);
		data->patch.start = true;
		data->patch.cur_stage = 0;
		data->patch.run_stage = false;

		if (data->patch.start_stage) {
			data->patch.cur_stage = data->patch.start_stage;
		}
		__mxt_patch_debug(data, "PATCH: START STAGE %d\n", data->patch.cur_stage);

		mxt_patch_init_tpos(data, &tpos_data);

		mxt_patch_init_supp(data, &tsupp_data);

		return 0;
	}

	return 1;
}

static int mxt_patch_test_trigger(struct mxt_data *data, struct mxt_message *message, u8 option)
{
	int i = 0;
	u8* ppatch = data->patch.patch;
	u16* ptrigger_addr = data->patch.trigger_addr;
	u8	trigger_cnt = data->patch.trigger_cnt;
	u8	tmsg[MXT_PATCH_MAX_MSG_SIZE] = {0};

	if (!ppatch || !ptrigger_addr) {
		TOUCH_PATCH_INFO_MSG( "%s ptrigger_addr is null\n", __func__);
		return 1;
	}

	memset(tmsg, 0, MXT_PATCH_MAX_MSG_SIZE);
	tmsg[0] =  message->reportid;
	memcpy(&tmsg[1], message->message, 8);

	for (i = 0; i< trigger_cnt; i++) {
		mxt_patch_parse_trigger(data, ppatch+ptrigger_addr[i], tmsg, true, option);
	}

	return 0;
}

int mxt_patch_event(struct mxt_data *data, u8 event_id)
{
	u8* ppatch = NULL;
	u16* pevent_addr = NULL;

	LOGN("Patch event %d\n", event_id);

	if (!data) {
		TOUCH_PATCH_INFO_MSG("%s addr is null\n", __func__);
		return 1;
	}

	ppatch = data->patch.patch;
	pevent_addr = data->patch.event_addr;

	if (!ppatch || !pevent_addr) {
		TOUCH_PATCH_INFO_MSG("%s addr is null\n", __func__);
		return 1;
	}

	if (event_id < data->patch.event_cnt) {
		mxt_patch_parse_event(data, ppatch+pevent_addr[event_id], true);
	}

	return 0;
}

static void mxt_patch_T6_object(struct mxt_data *data, struct mxt_message *message)
{
	/* Normal mode */
	if (message->message[0] == 0x00) {
		__mxt_patch_debug(data, "PATCH: NORMAL\n");
		if (data->patch.cal_flag == 1) {
			mxt_patch_start_stage(data);
			data->patch.cal_flag = 0;
		}
	}

	/* Calibration */
	if (message->message[0] & 0x10) {
		__mxt_patch_debug(data, "PATCH: CAL\n");
		data->patch.cal_flag = 1;
	}

	/* Reset */
	if (message->message[0] & 0x80) {
		__mxt_patch_debug(data, "PATCH: RESET\n");
		data->patch.start_stage = 0;
		data->patch.cal_flag = 1;
	}
}

static void mxt_patch_T9_object(struct mxt_data *data, struct mxt_message *message)
{
	int id = 0;
	u8 *msg = message->message;
	struct test_src tsrc = {0};

	id = data->reportids[message->reportid].index;

	mxt_patch_init_tsrc(&tsrc);

	data->fingers[id].x = ((msg[1] << 4) | ((msg[3] >> 4) & 0xf));
	data->fingers[id].y = ((msg[2] << 4) | (msg[3] & 0xf));
	data->fingers[id].x >>= 2;
	data->fingers[id].y >>= 2;

	tsrc.area = msg[4];
	tsrc.amp = msg[5];
	tsrc.status = msg[0];

	if (data->patch.start) {
		mxt_patch_make_source(data, &tsrc);

		if (data->patch.cur_stage_opt&0x02) {
			if ((msg[0] & MXT_DETECT_MSG_MASK) != MXT_DETECT_MSG_MASK) {
				if (msg[0] & MXT_SUPPRESS_MSG_MASK) {
					mxt_patch_check_supp(data, &tsupp_data);
				}
			}
		}
	}
}

static void mxt_patch_T15_object(struct mxt_data *data, struct mxt_message *message)
{
	struct test_src tsrc = {0};
	unsigned long keystates = message->message[MXT_MSG_T15_KEYSTATE];
	u8 key_cnt = 0;
	int i = 0;

	for (i = 0; i < 8; i++) {
		if (test_bit(i, &keystates)) {
			key_cnt++;
		}
	}
	mxt_patch_init_tsrc(&tsrc);
	tsrc.key_cnt = key_cnt;
	tsrc.key_val = keystates;

	if (data->patch.start) {
		mxt_patch_make_source(data, &tsrc);
		if (data->patch.option & 0x02) //0905#2
			mxt_patch_test_source(data, data->patch.src_item);
	}
}

static u8 check_pattern_tracking_condition(struct mxt_data *data, struct test_src* tsrc, struct touch_pos* tpos)
{
	u8 rtn = 0;
	int i = 0;

	// 1. checking edge area for ghost touches
	if (tsrc->finger_cnt) {
		for (i = 0; i < MXT_MAX_FINGER; i++) {
			if ((data->fingers[i].state != MXT_STATE_INACTIVE) &&	(data->fingers[i].state != MXT_STATE_RELEASE)) {
				if ((data->fingers[i].x < tpos_data.xlo_limit) ||(data->fingers[i].x > tpos_data.xhi_limit) ||
					(data->fingers[i].y < tpos_data.ylo_limit) ||(data->fingers[i].y > tpos_data.yhi_limit)) {
					rtn = 1;
				}
			}
		}
	}

	// 2. checking amp, size, t57 messages for ghost touches
	switch (tsrc->finger_cnt) {
		case 0:
			rtn = 0;
			break;
		case 1:
			if ((tsrc->amp < tpos_data.amp) && (tsrc->area < tpos_data.area) && (tsrc->sum_size < tpos_data.sum_size_t57) &&
				(tsrc->tch_ch < tpos_data.tch_count_t57) && (tsrc->atch_ch < tpos_data.atch_count_t57)) {
				rtn = 1;
			}
			break;
		case 2:
			if ((tsrc->amp > tpos_data.amp_2finger_min) && (tsrc->amp < tpos_data.amp_2finger_max) &&
				(tsrc->area > tpos_data.area_2finger_min) && (tsrc->area < tpos_data.area_2finger_max) &&
				(tsrc->sum_size > tpos_data.sum_size_t57_2finger_min) && (tsrc->sum_size < tpos_data.sum_size_t57_2finger_max) &&
				(tsrc->tch_ch > tpos_data.tch_count_t57_2finger_min) &&  (tsrc->tch_ch < tpos_data.tch_count_t57_2finger_max) &&
				(tsrc->atch_ch > tpos_data.atch_count_t57_2finger_min) && (tsrc->atch_ch < tpos_data.atch_count_t57_2finger_max)) {
				rtn = 1;
			}
			break;
		case 3:
			if ((tsrc->amp > tpos_data.amp_3finger_min) && (tsrc->amp < tpos_data.amp_3finger_max) &&
				(tsrc->area > tpos_data.area_3finger_min) && (tsrc->area < tpos_data.area_3finger_max) &&
				(tsrc->sum_size > tpos_data.sum_size_t57_3finger_min) && (tsrc->sum_size < tpos_data.sum_size_t57_3finger_max) &&
				(tsrc->tch_ch > tpos_data.tch_count_t57_3finger_min) &&  (tsrc->tch_ch < tpos_data.tch_count_t57_3finger_max) &&
				(tsrc->atch_ch > tpos_data.atch_count_t57_3finger_min) && (tsrc->atch_ch < tpos_data.atch_count_t57_3finger_max)) {
				rtn = 1;
			}
			break;
		default: // over 4 touches
			if ((tsrc->amp > tpos_data.amp_mfinger_min) && (tsrc->amp < tpos_data.amp_mfinger_max) &&
				(tsrc->area > tpos_data.area_mfinger_min) && (tsrc->area < tpos_data.area_mfinger_max) &&
				(tsrc->sum_size > tpos_data.sum_size_t57_mfinger_min) && (tsrc->sum_size < tpos_data.sum_size_t57_mfinger_max) &&
				(tsrc->tch_ch > tpos_data.tch_count_t57_mfinger_min) &&  (tsrc->tch_ch < tpos_data.tch_count_t57_mfinger_max) &&
				(tsrc->atch_ch > tpos_data.atch_count_t57_mfinger_min) && (tsrc->atch_ch < tpos_data.atch_count_t57_mfinger_max)) {
				rtn = 1;
			}
			break;
	}

	return rtn;
}

static void mxt_patch_T57_object(struct mxt_data *data, struct mxt_message *message)
{
	struct test_src tsrc = {0};
	u8 *msg = message->message;
	u8 finger_cnt = 0;
	int i = 0;

	mxt_patch_init_tsrc(&tsrc);

	for (i = 0; i < MXT_MAX_FINGER; i++) {
		if ((data->fingers[i].state != MXT_STATE_INACTIVE) && (data->fingers[i].state != MXT_STATE_RELEASE))
			finger_cnt++;
	}

	tsrc.finger_cnt = finger_cnt;

	tsrc.sum_size = msg[0] | (msg[1] << 8);
	tsrc.tch_ch = msg[2] | (msg[3] << 8);
	tsrc.atch_ch = msg[4] | (msg[5] << 8);

	tsrc.area = data->patch.src_item[MXT_PATCH_ITEM_AREA];
	tsrc.amp = data->patch.src_item[MXT_PATCH_ITEM_AMP];

	if (data->patch.start) {
		if ((data->patch.option & 0x01) == 0x01 && !finger_cnt)
			return;

		mxt_patch_make_source(data, &tsrc);
		mxt_patch_test_source(data, data->patch.src_item);
	}

	if ((data->patch.cur_stage_opt & 0x01) && check_pattern_tracking_condition(data, &tsrc, &tpos_data)) {
		if (finger_cnt) {
			for (i = 0; i < MXT_MAX_FINGER; i++) {
				if ((data->fingers[i].state != MXT_STATE_INACTIVE) && (data->fingers[i].state != MXT_STATE_RELEASE)) {
					mxt_patch_check_pattern(data, &tpos_data, i, data->fingers[i].x, data->fingers[i].y, finger_cnt);
				}
			}
		}
	}

	if (finger_cnt == 0) {
		mxt_patch_init_tpos(data, &tpos_data);
	}

}

static void mxt_patch_T61_object(struct mxt_data *data, struct mxt_message *message)
{
	int id = 0;
	u8 *msg = message->message;
	id = data->reportids[message->reportid].index;

	if ((id != data->patch.timer_id) || ((msg[0] & 0xa0) != 0xa0))
		return;

	__mxt_patch_debug(data, "END STAGE %d TIMER\n", data->patch.cur_stage);

	if ((data->patch.cur_stage+1) == data->patch.stage_cnt) {
		if (data->patch.period == 0) {
			__mxt_patch_debug(data, "EX-STAGE\n");
		} else {
			data->patch.start = false;
			__mxt_patch_debug(data, "END ALL STAGE\n");
		}
	} else {
		data->patch.cur_stage++;
		data->patch.run_stage = false;
	}

	if (!data->patch.run_stage) {
		mxt_patch_run_stage(data);
	}
}

static void mxt_patch_T100_object(struct mxt_data *data,
		struct mxt_message *message)
{
	u8 id, index;
	u8 *msg = message->message;
	u8 touch_type = 0, touch_event = 0, touch_detect = 0;
	u16 x, y;
	struct test_src tsrc;

	index = data->reportids[message->reportid].index;

	mxt_patch_init_tsrc(&tsrc);

	/* Treate screen messages */
	if (index < MXT_T100_SCREEN_MESSAGE_NUM_RPT_ID) {
		if (index == MXT_T100_SCREEN_MSG_FIRST_RPT_ID) {
			data->patch.finger_cnt = msg[1];
			tsrc.finger_cnt = data->patch.finger_cnt;
			tsrc.tch_ch = (msg[3] << 8) | msg[2];
			tsrc.atch_ch = (msg[5] << 8) | msg[4];
			tsrc.sum_size = (msg[7] << 8) | msg[6];

			if (data->patch.start) {
				mxt_patch_make_source(data, &tsrc);
				if ((data->patch.option & 0x08) == 0x08) {
					mxt_patch_test_source(data, data->patch.src_item);
				}
			}

			return;
		}
	}

	if (index >= MXT_T100_SCREEN_MESSAGE_NUM_RPT_ID) {
		/* Treate touch status messages */
		id = index - MXT_T100_SCREEN_MESSAGE_NUM_RPT_ID;
		touch_detect = msg[0] >> MXT_T100_DETECT_MSG_MASK;
		touch_type = (msg[0] & 0x70) >> 4;
		data->touch_type = touch_type;
		touch_event = msg[0] & 0x0F;

		switch (touch_type) {
		case MXT_T100_TYPE_PATCH_FINGER:
		case MXT_T100_TYPE_PASSIVE_STYLUS:
			x = msg[1] | (msg[2] << 8);
			y = msg[3] | (msg[4] << 8);

			tsrc.amp = msg[5];
			tsrc.area2 = msg[6]&0x3f;
			tsrc.area = msg[7]&0x1f;

			if (data->patch.start) {
				if ((data->patch.option & 0x01) == 0x01 && !touch_detect)
					return;

				mxt_patch_make_source(data, &tsrc);
				mxt_patch_test_source(data, data->patch.src_item);
			}

			break;
		}
	}
}

void mxt_patch_message(struct mxt_data *data, struct mxt_message *message)
{
	u8 reportid = 0, type = 0;
	reportid = message->reportid;

	if (reportid > data->max_reportid)
		return;

	type = data->reportids[reportid].type;
	switch (type) {
		case MXT_GEN_COMMAND_T6:
			mxt_patch_T6_object(data, message);
			break;
		case MXT_TOUCH_MULTI_T9:
			mxt_patch_T9_object(data, message);
			break;
		case MXT_TOUCH_KEYARRAY_T15:
			mxt_patch_T15_object(data, message);
			break;
		case MXT_PROCI_EXTRATOUCHSCREENDATA_T57:
			mxt_patch_T57_object(data, message);
			break;
		case MXT_SPT_TIMER_T61:
			mxt_patch_T61_object(data, message);
			break;
		case MXT_TOUCH_MULTITOUCHSCREEN_T100:
			mxt_patch_T100_object(data, message);
			break;
	}

	if (data->patch.trigger_cnt && type) {
		mxt_patch_test_trigger(data, message, data->charging_mode);
	}
}

int mxt_patch_init(struct mxt_data *data, u8* ppatch)
{
	struct mxt_patch *patch_info = &data->patch;
	struct patch_header *ppheader = NULL;
	u16 stage_addr[64] = {0};
	u16 trigger_addr[64] = {0};
	u16 event_addr[64] = {0};
	u32 patch_size = 0;

	TOUCH_PATCH_INFO_MSG("%s\n", __func__);

	if (!ppatch) {
		TOUCH_PATCH_INFO_MSG("%s patch file error\n", __func__);
		return 1;
	}

	patch_size = mxt_patch_parse_header(data, ppatch, stage_addr, trigger_addr, event_addr);
	if (!patch_size) {
		TOUCH_PATCH_INFO_MSG("%s patch_size error\n", __func__);
		return 1;
	}
	ppheader = (struct patch_header*)ppatch;
	patch_info->timer_id = ppheader->timer_id;
	patch_info->option = ppheader->option;
	patch_info->debug = 0; // ppheader->debug;
	patch_info->stage_cnt = ppheader->stage_cnt;
	patch_info->trigger_cnt = ppheader->trigger_cnt;
	patch_info->event_cnt = ppheader->event_cnt;
	patch_info->date = ppheader->date;

	if (data->patch.src_item) {
		kfree(data->patch.src_item);
	}

	patch_info->src_item = kzalloc(MXT_PATCH_ITEM_END*sizeof(u16), GFP_KERNEL);

	if (patch_info->stage_cnt) {
		if (patch_info->stage_addr) {
			kfree(patch_info->stage_addr);
		}

		patch_info->stage_addr = kzalloc(patch_info->stage_cnt*sizeof(u16), GFP_KERNEL);
		if (!patch_info->stage_addr) {
			TOUCH_PATCH_INFO_MSG("stage_addr alloc error\n");
			return 1;
		}
		memcpy(patch_info->stage_addr, stage_addr, patch_info->stage_cnt*sizeof(u16));
	}

	if (patch_info->trigger_cnt) {
		if (patch_info->trigger_addr) {
			kfree(patch_info->trigger_addr);
		}

		patch_info->trigger_addr = kzalloc(patch_info->trigger_cnt*sizeof(u16), GFP_KERNEL);
		if (!patch_info->trigger_addr) {
			TOUCH_PATCH_INFO_MSG("trigger_addr alloc error\n");
			return 1;
		}
		memcpy(patch_info->trigger_addr, trigger_addr, patch_info->trigger_cnt*sizeof(u16));
	}

	if (patch_info->event_cnt) {
		if (patch_info->event_addr) {
			kfree(patch_info->event_addr);
		}

		patch_info->event_addr = kzalloc(patch_info->event_cnt*sizeof(u16), GFP_KERNEL);
		if (!patch_info->event_addr) {
			TOUCH_PATCH_INFO_MSG("event_addr alloc error\n");
			return 1;
		}
		memcpy(patch_info->event_addr, event_addr, patch_info->event_cnt*sizeof(u16));
	}

	mxt_patch_load_t71data(data);
	mxt_patch_init_userdata();

	return 0;
}

late_initcall(mxt_init);
module_exit(mxt_exit);

/* Module information */
MODULE_AUTHOR("<WX-BSP-TS@lge.com>");
MODULE_DESCRIPTION("Atmel Touchscreen driver");
MODULE_LICENSE("GPL");
