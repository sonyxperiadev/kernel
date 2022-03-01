/* drivers/input/touchscreen/sec_ts.c
 *
 * Copyright (C) 2011 Samsung Electronics Co., Ltd.
 * http://www.samsungsemi.com/
 * Copyright (C) 2018 Sony Mobile Communications Inc.
 *
 * Core file for Samsung TSC driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/moduleparam.h>

struct sec_ts_data *tsp_info;

#include "include/sec_ts.h"

static int report_rejected_event = 0;
module_param(report_rejected_event, int, 0660);

struct class *sec_class;
struct sec_ts_data *ts_dup;
struct drm_panel *sec_ts_active_panel;

u32 portrait_buffer[SEC_TS_GRIP_REJECTION_BORDER_NUM] = { 0 };
u32 landscape_buffer[SEC_TS_GRIP_REJECTION_BORDER_NUM] = { 0 };
u32 radius_portrait[SEC_TS_GRIP_REJECTION_BORDER_NUM_PORTRAIT] = { 0 };
u32 radius_landscape[SEC_TS_GRIP_REJECTION_BORDER_NUM_LANDSCAPE] = { 0 };

static bool is_first_ts_kickstart = true;

#ifdef USE_POWER_RESET_WORK
static void sec_ts_reset_work(struct work_struct *work);
#endif
#ifdef USE_SELF_TEST_WORK
static void sec_ts_read_info_work(struct work_struct *work);
#endif
static void sec_ts_after_init_work(struct work_struct *work);
#ifdef USE_OPEN_CLOSE
static int sec_ts_input_open(struct input_dev *dev);
static void sec_ts_input_close(struct input_dev *dev);
#endif

int sec_ts_read_information(struct sec_ts_data *ts);
static int drm_notifier_callback(struct notifier_block *self, unsigned long event, void *data);

void sec_ts_set_irq(struct sec_ts_data *ts, bool enable)
{
	if (!mutex_trylock(&ts->irq_mutex)) {
		input_err(true, &ts->client->dev, "%s: sec_ts_set_irq() is called by two or more threaded at the same time.\n", __func__);
		mutex_lock(&ts->irq_mutex);
	}

	if (ts->client->irq) {
		if (enable && !ts->irq_status) {
			enable_irq(ts->client->irq);
			ts->irq_status = true;
			input_info(true, &ts->client->dev, "Change irq enabled\n");
		} else if (!enable && ts->irq_status) {
			disable_irq_nosync(ts->client->irq);
			ts->irq_status = false;
			input_info(true, &ts->client->dev, "Change irq was disable\n");
		} else {
			input_info(true, &ts->client->dev, "no irq change [%s]\n",
				ts->irq_status ? "enable" : "disable");
		}

		if (ts->irq_status &&
			irqd_irq_disabled(irq_get_irq_data(ts->client->irq))) {
			input_err(true, &ts->client->dev, "%s: unexpected irq %d disable! Fixing..\n", __func__, ts->client->irq);
			enable_irq(ts->client->irq);
		} else if (!ts->irq_status &&
			!irqd_irq_disabled(irq_get_irq_data(ts->client->irq))) {
			input_err(true, &ts->client->dev, "%s: unexpected irq %d enable! Fixing..\n", __func__, ts->client->irq);
			disable_irq(ts->client->irq);
		}
	}
	mutex_unlock(&ts->irq_mutex);
}

void sec_ts_free_irq(struct sec_ts_data *ts)
{
	if (ts->client->irq) {
		free_irq(ts->client->irq, ts);
		ts->client->irq = 0;
		input_info(true, &ts->client->dev, "irq release done [%s]\n", ts->client->irq);
	}
	return;
}

void sec_ts_irq_wake(struct sec_ts_data *ts, bool enable)
{
	mutex_lock(&ts->irq_mutex);
	input_dbg(ts->debug_flag, &ts->client->dev, "irq_wake_mask [%d]\n", ts->irq_wake_mask);
	if (enable && !ts->irq_wake_mask) {
		enable_irq_wake(ts->client->irq);
		ts->irq_wake_mask = true;
	} else if (!enable && ts->irq_wake_mask) {
		disable_irq_wake(ts->client->irq);
		ts->irq_wake_mask = false;
	} else {
		input_info(true, &ts->client->dev, "no irq wake change [%s]\n",
		     ts->irq_wake_mask ? "enable" : "disable");
	}
	mutex_unlock(&ts->irq_mutex);
}

int sec_ts_i2c_write(struct sec_ts_data *ts, u8 reg, u8 *data, int len)
{
	u8 buf[I2C_WRITE_BUFFER_SIZE + 1];
	int ret;
	unsigned char retry;
	struct i2c_msg msg;
	int i;

	if (len > I2C_WRITE_BUFFER_SIZE) {
		input_err(true, &ts->client->dev, "%s: len is larger than buffer size\n", __func__);
		return -EINVAL;
	}

	if (ts->power_status == SEC_TS_STATE_POWER_OFF) {
		input_err(true, &ts->client->dev, "%s: POWER_STATUS : OFF\n", __func__);
		goto err;
	}

	buf[0] = reg;
	memcpy(buf + 1, data, len);

	msg.addr = ts->client->addr;
	msg.flags = 0;
	msg.len = len + 1;
	msg.buf = buf;

	mutex_lock(&ts->i2c_mutex);
	for (retry = 0; retry < SEC_TS_I2C_RETRY_CNT; retry++) {
		ret = i2c_transfer(ts->client->adapter, &msg, 1);
		if (ret == 1)
			break;

		if (ts->power_status == SEC_TS_STATE_POWER_OFF) {
			input_err(true, &ts->client->dev, "%s: POWER_STATUS : OFF, retry:%d\n", __func__, retry);
			mutex_unlock(&ts->i2c_mutex);
			goto err;
		}

		usleep_range(1 * 1000, 1 * 1000);

		if (retry > 1) {
			input_err(true, &ts->client->dev, "%s: I2C retry %d, ret:%d\n", __func__, retry + 1, ret);
			ts->comm_err_count++;
		}
	}

	mutex_unlock(&ts->i2c_mutex);

	if (retry == SEC_TS_I2C_RETRY_CNT) {
		input_err(true, &ts->client->dev, "%s: I2C write over retry limit\n", __func__);
		ret = -EIO;
#ifdef USE_POR_AFTER_I2C_RETRY
		if (ts->after_work.done && !ts->reset_is_on_going)
			schedule_delayed_work(&ts->reset_work, msecs_to_jiffies(TOUCH_RESET_DWORK_TIME));
#endif
	}

	if (ts->debug_flag & SEC_TS_DEBUG_PRINT_I2C_WRITE_CMD) {
		pr_info("sec_input:i2c_cmd: W: %02X | ", reg);
		for (i = 0; i < len; i++)
			pr_cont("%02X ", data[i]);
		pr_cont("\n");
	}

	if (ret == 1)
		return 0;
err:
	return -EIO;
}

int sec_ts_i2c_read(struct sec_ts_data *ts, u8 reg, u8 *data, int len)
{
	u8 buf[4];
	int ret;
	unsigned char retry;
	struct i2c_msg msg[2];
	int remain = len;
	int i;

	if (ts->power_status == SEC_TS_STATE_POWER_OFF) {
		input_err(true, &ts->client->dev, "%s: POWER_STATUS : OFF\n", __func__);
		goto err;
	}

	buf[0] = reg;

	msg[0].addr = ts->client->addr;
	msg[0].flags = 0;
	msg[0].len = 1;
	msg[0].buf = buf;

	msg[1].addr = ts->client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = len;
	msg[1].buf = data;

	mutex_lock(&ts->i2c_mutex);

	if (len <= ts->i2c_burstmax) {

		for (retry = 0; retry < SEC_TS_I2C_RETRY_CNT; retry++) {
			ret = i2c_transfer(ts->client->adapter, msg, 2);
			if (ret == 2)
				break;
			usleep_range(1 * 1000, 1 * 1000);
			if (ts->power_status == SEC_TS_STATE_POWER_OFF) {
				input_err(true, &ts->client->dev, "%s: POWER_STATUS : OFF, retry:%d\n", __func__, retry);
				mutex_unlock(&ts->i2c_mutex);
				goto err;
			}

			if (retry > 1) {
				input_err(true, &ts->client->dev, "%s: I2C retry %d, ret:%d\n",
					__func__, retry + 1, ret);
				ts->comm_err_count++;
			}
		}

	} else {
		/*
		 * I2C read buffer is 256 byte. do not support long buffer over than 256.
		 * So, try to seperate reading data about 256 bytes.
		 */

		for (retry = 0; retry < SEC_TS_I2C_RETRY_CNT; retry++) {
			ret = i2c_transfer(ts->client->adapter, msg, 1);
			if (ret == 1)
				break;
			usleep_range(1 * 1000, 1 * 1000);
			if (ts->power_status == SEC_TS_STATE_POWER_OFF) {
				input_err(true, &ts->client->dev, "%s: POWER_STATUS : OFF, retry:%d\n", __func__, retry);
				mutex_unlock(&ts->i2c_mutex);
				goto err;
			}

			if (retry > 1) {
				input_err(true, &ts->client->dev, "%s: I2C retry %d, ret:%d\n",
					__func__, retry + 1, ret);
				ts->comm_err_count++;
			}
		}

		do {
			if (remain > ts->i2c_burstmax)
				msg[1].len = ts->i2c_burstmax;
			else
				msg[1].len = remain;

			remain -= ts->i2c_burstmax;

			for (retry = 0; retry < SEC_TS_I2C_RETRY_CNT; retry++) {
				ret = i2c_transfer(ts->client->adapter, &msg[1], 1);
				if (ret == 1)
					break;
				usleep_range(1 * 1000, 1 * 1000);
				if (ts->power_status == SEC_TS_STATE_POWER_OFF) {
					input_err(true, &ts->client->dev, "%s: POWER_STATUS : OFF, retry:%d\n", __func__, retry);
					mutex_unlock(&ts->i2c_mutex);
					goto err;
				}

				if (retry > 1) {
					input_err(true, &ts->client->dev, "%s: I2C retry %d, ret:%d\n",
						__func__, retry + 1, ret);
					ts->comm_err_count++;
				}
			}

			msg[1].buf += msg[1].len;

		} while (remain > 0);

	}

	mutex_unlock(&ts->i2c_mutex);

	if (retry == SEC_TS_I2C_RETRY_CNT) {
		input_err(true, &ts->client->dev, "%s: I2C read over retry limit\n", __func__);
		ret = -EIO;
#ifdef USE_POR_AFTER_I2C_RETRY
		if (ts->after_work.done && !ts->reset_is_on_going)
			schedule_delayed_work(&ts->reset_work, msecs_to_jiffies(TOUCH_RESET_DWORK_TIME));
#endif

	}

	if (ts->debug_flag & SEC_TS_DEBUG_PRINT_I2C_READ_CMD) {
		pr_info("sec_input:i2c_cmd: R: %02X | ", reg);
		for (i = 0; i < len; i++)
			pr_cont("%02X ", data[i]);
		pr_cont("\n");
	}

	return ret;

err:
	return -EIO;
}

static int sec_ts_i2c_write_burst(struct sec_ts_data *ts, u8 *data, int len)
{
	int ret;
	int retry;

	mutex_lock(&ts->i2c_mutex);
	for (retry = 0; retry < SEC_TS_I2C_RETRY_CNT; retry++) {
		ret = i2c_master_send(ts->client, data, len);
		if (ret == len)
			break;

		usleep_range(1 * 1000, 1 * 1000);

		if (retry > 1) {
			input_err(true, &ts->client->dev, "%s: I2C retry %d, ret:%d\n", __func__, retry + 1, ret);
			ts->comm_err_count++;
		}
	}

	mutex_unlock(&ts->i2c_mutex);
	if (retry == SEC_TS_I2C_RETRY_CNT) {
		input_err(true, &ts->client->dev, "%s: I2C write over retry limit\n", __func__);
		ret = -EIO;
	}

	return ret;
}

static int sec_ts_i2c_read_bulk(struct sec_ts_data *ts, u8 *data, int len)
{
	int ret;
	unsigned char retry;
	int remain = len;
	struct i2c_msg msg;

	msg.addr = ts->client->addr;
	msg.flags = I2C_M_RD;
	msg.len = len;
	msg.buf = data;

	mutex_lock(&ts->i2c_mutex);

	do {
		if (remain > ts->i2c_burstmax)
			msg.len = ts->i2c_burstmax;
		else
			msg.len = remain;

		remain -= ts->i2c_burstmax;

		for (retry = 0; retry < SEC_TS_I2C_RETRY_CNT; retry++) {
			ret = i2c_transfer(ts->client->adapter, &msg, 1);
			if (ret == 1)
				break;
			usleep_range(1 * 1000, 1 * 1000);

			if (retry > 1) {
				input_err(true, &ts->client->dev, "%s: I2C retry %d, ret:%d\n",
					__func__, retry + 1, ret);
				ts->comm_err_count++;
			}
		}

		if (retry == SEC_TS_I2C_RETRY_CNT) {
			input_err(true, &ts->client->dev, "%s: I2C read over retry limit\n", __func__);
			ret = -EIO;

			break;
		}
		msg.buf += msg.len;

	} while (remain > 0);

	mutex_unlock(&ts->i2c_mutex);

	if (ret == 1)
		return 0;

	return -EIO;
}

void sec_ts_delay(unsigned int ms)
{
	if (ms < 20)
		usleep_range(ms * 1000, ms * 1000);
	else
		msleep(ms);
}

int sec_ts_wait_for_ready(struct sec_ts_data *ts, unsigned int ack)
{
	int rc = -1;
	int retry = 0;
	u8 tBuff[SEC_TS_EVENT_BUFF_SIZE] = {0,};

	while (sec_ts_i2c_read(ts, SEC_TS_READ_ONE_EVENT, tBuff, SEC_TS_EVENT_BUFF_SIZE) > 0) {
		if (((tBuff[0] >> 2) & 0xF) == TYPE_STATUS_EVENT_INFO) {
			if (tBuff[1] == ack) {
				rc = 0;
				break;
			}
		} else if (((tBuff[0] >> 2) & 0xF) == TYPE_STATUS_EVENT_VENDOR_INFO) {
			if (tBuff[1] == ack) {
				rc = 0;
				break;
			}
		}

		if (retry++ > SEC_TS_WAIT_RETRY_CNT) {
			input_err(true, &ts->client->dev, "%s: Time Over\n", __func__);
			break;
		}
		sec_ts_delay(20);
	}

	input_info(true, &ts->client->dev,
			"%s: %02X, %02X, %02X, %02X, %02X, %02X, %02X, %02X [%d]\n",
			__func__, tBuff[0], tBuff[1], tBuff[2], tBuff[3],
			tBuff[4], tBuff[5], tBuff[6], tBuff[7], retry);

	return rc;
}

int sec_ts_read_calibration_report(struct sec_ts_data *ts)
{
	int ret;
	u8 buf[5] = { 0 };

	buf[0] = SEC_TS_READ_CALIBRATION_REPORT;

	ret = sec_ts_i2c_read(ts, buf[0], &buf[1], 4);
	if (ret < 0) {
		input_err(true, &ts->client->dev, "%s: failed to read, %d\n", __func__, ret);
		return ret;
	}

	input_info(true, &ts->client->dev, "%s: count:%d, pass count:%d, fail count:%d, status:0x%X\n",
			__func__, buf[1], buf[2], buf[3], buf[4]);

	return buf[4];
}

void sec_ts_reinit(struct sec_ts_data *ts)
{
	u8 w_data[2] = {0x00, 0x00};
	int ret = 0;

	input_info(true, &ts->client->dev,
			"%s : charger=0x%x, touch_functions=0x%x, Power mode=0x%x, noise_mode=%d\n",
			__func__, ts->charger_mode, ts->touch_functions,
			ts->power_status, ts->touch_noise_status);

	ts->touch_noise_status = 0;

	/* charger mode */
	if (ts->charger_mode != SEC_TS_BIT_CHARGER_MODE_NO) {
		w_data[0] = ts->charger_mode;
		ret = ts->sec_ts_i2c_write(ts, SET_TS_CMD_SET_CHARGER_MODE, (u8 *)&w_data[0], 1);
		if (ret < 0)
			input_err(true, &ts->client->dev, "%s: Failed to send command(0x%x)",
					__func__, SET_TS_CMD_SET_CHARGER_MODE);
	}

	/* Cover mode */
	if (ts->touch_functions & SEC_TS_BIT_SETFUNC_COVER) {
		w_data[0] = ts->cover_type;
		ret = sec_ts_i2c_write(ts, SEC_TS_CMD_SET_COVERTYPE, (u8 *)&w_data[0], 1);
		if (ret < 0)
			input_err(true, &ts->client->dev, "%s: Failed to send command(0x%x)",
					__func__, SEC_TS_CMD_SET_COVERTYPE);
	}

	ret = sec_ts_i2c_write(ts, SEC_TS_CMD_SET_TOUCHFUNCTION, (u8 *)&(ts->touch_functions), 2);
	if (ret < 0)
		input_err(true, &ts->client->dev, "%s: Failed to send command(0x%x)",
				__func__, SEC_TS_CMD_SET_TOUCHFUNCTION);

	w_data[0] = CONVERT_SIDE_ENABLE_MODE_FOR_WRITE(ts->side_enable);

	ret = sec_ts_i2c_write(ts, SEC_TS_CMD_ENABLE_SIDETOUCH, (u8 *)&w_data[0], 1);
	if (ret < 0)
		input_err(true, &ts->client->dev, "%s: Failed to send command(0x%x)",
				__func__, SEC_TS_CMD_ENABLE_SIDETOUCH);

	ret = sec_ts_i2c_write(ts, SEC_TS_CMD_ENABLE_STAMINAMODE, (u8 *)&(ts->stamina_enable), 1);
	if (ret < 0)
		input_err(true, &ts->client->dev, "%s: Failed to send command(0x%x)",
				__func__, SEC_TS_CMD_ENABLE_STAMINAMODE);

	/* Power mode */
	if (ts->power_status == SEC_TS_STATE_LPM) {
		w_data[0] = TO_LOWPOWER_MODE;
		ret = sec_ts_i2c_write(ts, SEC_TS_CMD_SET_POWER_MODE, (u8 *)&w_data[0], 1);
		if (ret < 0)
			input_err(true, &ts->client->dev, "%s: Failed to send command(0x%x)",
					__func__, SEC_TS_CMD_SET_POWER_MODE);
		sec_ts_delay(50);
	} else {

		sec_ts_set_grip_type(ts, ONLY_EDGE_HANDLER);

		if (ts->dex_mode) {
			input_info(true, &ts->client->dev, "%s: set dex mode\n", __func__);
			ret = ts->sec_ts_i2c_write(ts, SEC_TS_CMD_SET_DEX_MODE, &ts->dex_mode, 1);
			if (ret < 0)
				input_err(true, &ts->client->dev,
						"%s: failed to set dex mode %x\n", __func__, ts->dex_mode);
		}

		if (ts->brush_mode) {
			input_info(true, &ts->client->dev, "%s: set brush mode\n", __func__);
			ret = ts->sec_ts_i2c_write(ts, SEC_TS_CMD_SET_BRUSH_MODE, &ts->brush_mode, 1);
			if (ret < 0)
				input_err(true, &ts->client->dev,
						"%s: failed to set brush mode\n", __func__);
		}

		if (ts->touchable_area) {
			input_info(true, &ts->client->dev, "%s: set 16:9 mode\n", __func__);
			ret = ts->sec_ts_i2c_write(ts, SEC_TS_CMD_SET_TOUCHABLE_AREA, &ts->touchable_area, 1);
			if (ret < 0)
				input_err(true, &ts->client->dev,
						"%s: failed to set 16:9 mode\n", __func__);
		}

	}
	return;
}

void report_touch(struct sec_ts_data *ts, u8 t_id, bool stored)
{
	u16 x;
	u16 y;

	if (!(report_rejected_event && ts->report_rejected_event_flag) && stored)
		return;

	if (stored) {
		x = ts->saved_data_x[t_id];
		y = ts->saved_data_y[t_id];
	} else {
		x = ts->coord[t_id].x;
		y = ts->coord[t_id].y;
	}

	input_mt_slot(ts->input_dev, t_id);
	input_mt_report_slot_state(ts->input_dev, MT_TOOL_FINGER, 1);
	input_report_key(ts->input_dev, BTN_TOUCH, 1);
	input_report_key(ts->input_dev, BTN_TOOL_FINGER, 1);

	input_report_abs(ts->input_dev, ABS_MT_POSITION_X, x);
	input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, y);
	input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, ts->coord[t_id].major);
	input_report_abs(ts->input_dev, ABS_MT_TOUCH_MINOR, ts->coord[t_id].minor);
	if (ts->plat_data->support_mt_pressure)
		input_report_abs(ts->input_dev, ABS_MT_PRESSURE, ts->coord[t_id].z);

	if (stored)
		input_sync(ts->input_dev);
	return;
}

static int get_location(struct sec_ts_data *ts, u32 x, u32 y)
{
    if (x > ts->plat_data->max_x / 2) {
		if (y > ts->plat_data->max_y / 2)
			return CORNER_3;
		return CORNER_2;
	} else {
		if (y > ts->plat_data->max_y / 2)
			return CORNER_1;
		return CORNER_0;
    }
}

static u32 compute_sum_of_squares(u32 x, u32 y)
{
	return x * x + y * y;
}

static bool check_area(struct sec_ts_data *ts, int location, u32 x, u32 y)
{
	if (ts->landscape) {
		switch (location) {
		case CORNER_0:
			break;
		case CORNER_1:
			y = ts->plat_data->max_y - y;
			break;
		case CORNER_2:
			x = ts->plat_data->max_x - x;
			break;
		case CORNER_3:
			x = ts->plat_data->max_x - x;
			y = ts->plat_data->max_y - y;
			break;
		default:
			input_err(true, &ts->client->dev,
						"%s: failed to find the area \n", __func__);
			return false;
		}

		return compute_sum_of_squares(x, y) < ts->circle_range_l[location];
	} else {
		if (location == CORNER_1) {
			y = ts->plat_data->max_y - y;
			location = CORNER_0;
		} else if (location == CORNER_3) {
			x = ts->plat_data->max_x - x;
			y = ts->plat_data->max_y - y;
			location = CORNER_1;
		} else {
			return false;
		}

		return compute_sum_of_squares(x, y) < ts->circle_range_p[location];
	}
}

#define MAX_EVENT_COUNT 32
static void sec_ts_read_event(struct sec_ts_data *ts)
{
	int ret;
	int i;
	u8 t_id;
	u8 event_id;
	u8 left_event_count;
	u8 read_event_buff[MAX_EVENT_COUNT][SEC_TS_EVENT_BUFF_SIZE] = { { 0 } };
	u8 *event_buff;
	u8 tdata[3] = { 0 };
	struct sec_ts_event_coordinate *p_event_coord;
	struct sec_ts_gesture_status *p_gesture_status;
	struct sec_ts_event_status *p_event_status;
	int curr_pos;
	int remain_event_count = 0;
	int pre_ttype = 0;
	bool update_touch = false;
	bool update_side_touch = false;
	incell_pw_status status = { false, false };
	int location;

	if (ts->power_status == SEC_TS_STATE_LPM) {
		pm_wakeup_event(&ts->client->dev, msecs_to_jiffies(500));

		/* waiting for blsp block resuming, if not occurs i2c error */
		ret = wait_for_completion_interruptible_timeout(&ts->resume_done, msecs_to_jiffies(500));
		if (ret == 0) {
			input_err(true, &ts->client->dev, "%s: LPM: pm resume is not handled\n", __func__);
			return;
		}

		if (ret < 0) {
			input_err(true, &ts->client->dev, "%s: LPM: -ERESTARTSYS if interrupted, %d\n", __func__, ret);
			return;
		}

		input_dbg(ts->debug_flag, &ts->client->dev, "%s: run LPM interrupt handler, %d\n", __func__, ret);
		/* run lpm interrupt handler */
	}

	ret = t_id = event_id = curr_pos = remain_event_count = 0;
	/* repeat READ_ONE_EVENT until buffer is empty(No event) */
	ret = sec_ts_i2c_read(ts, SEC_TS_READ_ONE_EVENT, (u8 *)read_event_buff[0], SEC_TS_EVENT_BUFF_SIZE);
	if (ret < 0) {
		input_err(true, &ts->client->dev, "%s: i2c read one event failed\n", __func__);
		return;
	}

	if (ts->debug_flag & SEC_TS_DEBUG_PRINT_ONEEVENT)
		input_dbg(ts->debug_flag, &ts->client->dev, "ONE: %02X %02X %02X %02X %02X %02X %02X %02X\n",
				read_event_buff[0][0], read_event_buff[0][1],
				read_event_buff[0][2], read_event_buff[0][3],
				read_event_buff[0][4], read_event_buff[0][5],
				read_event_buff[0][6], read_event_buff[0][7]);

	if (read_event_buff[0][0] == 0) {
		input_info(true, &ts->client->dev, "%s: event buffer is empty\n", __func__);
		return;
	}

	left_event_count = read_event_buff[0][7] & 0x3F;
	remain_event_count = left_event_count;

	if (left_event_count > MAX_EVENT_COUNT - 1) {
		input_err(true, &ts->client->dev, "%s: event buffer overflow\n", __func__);

		/* write clear event stack command when read_event_count > MAX_EVENT_COUNT */
		ret = sec_ts_i2c_write(ts, SEC_TS_CMD_CLEAR_EVENT_STACK, NULL, 0);
		if (ret < 0)
			input_err(true, &ts->client->dev, "%s: i2c write clear event failed\n", __func__);

		sec_ts_unlocked_release_all_finger(ts);

		return;
	}

	if (left_event_count > 0) {
		ret = sec_ts_i2c_read(ts, SEC_TS_READ_ALL_EVENT, (u8 *)read_event_buff[1],
				sizeof(u8) * (SEC_TS_EVENT_BUFF_SIZE) * (left_event_count));
		if (ret < 0) {
			input_err(true, &ts->client->dev, "%s: i2c read one event failed\n", __func__);
			return;
		}
	}

	do {
		event_buff = read_event_buff[curr_pos];
		event_id = event_buff[0] & 0x3;

		if (ts->debug_flag & SEC_TS_DEBUG_PRINT_ALLEVENT)
			input_dbg(ts->debug_flag, &ts->client->dev, "ALL: %02X %02X %02X %02X %02X %02X %02X %02X\n",
					event_buff[0], event_buff[1], event_buff[2], event_buff[3],
					event_buff[4], event_buff[5], event_buff[6], event_buff[7]);

		switch (event_id) {
		case SEC_TS_STATUS_EVENT:
			p_event_status = (struct sec_ts_event_status *)event_buff;

			/* tchsta == 0 && ttype == 0 && eid == 0 : buffer empty */
			if (p_event_status->stype > 0)
				input_dbg(ts->debug_flag, &ts->client->dev, "%s: STATUS %x %x %x %x %x %x %x %x\n", __func__,
						event_buff[0], event_buff[1], event_buff[2],
						event_buff[3], event_buff[4], event_buff[5],
						event_buff[6], event_buff[7]);

			/* watchdog reset -> send SENSEON command */
			if ((p_event_status->stype == TYPE_STATUS_EVENT_INFO) &&
				(p_event_status->status_id == SEC_TS_ACK_BOOT_COMPLETE) &&
				(p_event_status->status_data_1 == 0x20)) {

				sec_ts_unlocked_release_all_finger(ts);

				ret = sec_ts_i2c_write(ts, SEC_TS_CMD_SENSE_ON, NULL, 0);
				if (ret < 0)
					input_err(true, &ts->client->dev, "%s: fail to write Sense_on\n", __func__);

				sec_ts_reinit(ts);
			}

			/* event queue full-> all finger release */
			if ((p_event_status->stype == TYPE_STATUS_EVENT_ERR) &&
				(p_event_status->status_id == SEC_TS_ERR_EVENT_QUEUE_FULL)) {
				input_err(true, &ts->client->dev, "%s: IC Event Queue is full\n", __func__);
				sec_ts_unlocked_release_all_finger(ts);
			}

			if ((p_event_status->stype == TYPE_STATUS_EVENT_ERR) &&
				(p_event_status->status_id == SEC_TS_ERR_EVENT_ESD)) {
				input_err(true, &ts->client->dev, "%s: ESD detected. run reset\n", __func__);
#ifdef USE_RESET_DURING_POWER_ON
				schedule_work(&ts->reset_work.work);
#endif
			}

			if ((p_event_status->stype == TYPE_STATUS_EVENT_INFO) &&
				(p_event_status->status_id == SEC_TS_ACK_WET_MODE)) {
				ts->wet_mode = p_event_status->status_data_1;
				input_dbg(ts->debug_flag, &ts->client->dev, "%s: water wet mode %d\n",
						__func__, ts->wet_mode);
				if (ts->wet_mode)
					ts->wet_count++;
			}

			if ((p_event_status->stype == TYPE_STATUS_EVENT_VENDOR_INFO) &&
					(p_event_status->status_id == SEC_TS_VENDOR_ACK_NOISE_STATUS_NOTI)) {

				ts->touch_noise_status = !!p_event_status->status_data_1;
				input_dbg(ts->debug_flag, &ts->client->dev, "%s: TSP NOISE MODE %s[%d]\n",
						__func__, ts->touch_noise_status == 0 ? "OFF" : "ON",
						p_event_status->status_data_1);

				if (ts->touch_noise_status)
					ts->noise_count++;
			}

			break;

		case SEC_TS_COORDINATE_EVENT:
			if (ts->power_status != SEC_TS_STATE_POWER_ON) {
				input_err(true, &ts->client->dev, "%s: device is closed\n", __func__);
				break;
			}
			p_event_coord = (struct sec_ts_event_coordinate *)event_buff;

			t_id = (p_event_coord->tid - 1);

			if (t_id < MAX_SUPPORT_TOUCH_COUNT + MAX_SUPPORT_HOVER_COUNT) {
				pre_ttype = ts->coord[t_id].ttype;
				ts->coord[t_id].id = t_id;
				ts->coord[t_id].action = p_event_coord->tchsta;
				ts->coord[t_id].x = (p_event_coord->x_11_4 << 4) | (p_event_coord->x_3_0);
				ts->coord[t_id].y = (p_event_coord->y_11_4 << 4) | (p_event_coord->y_3_0);
				ts->coord[t_id].z = p_event_coord->z & 0x3F;
				ts->coord[t_id].ttype = p_event_coord->ttype_3_2 << 2 | p_event_coord->ttype_1_0 << 0;
				ts->coord[t_id].major = p_event_coord->major;
				ts->coord[t_id].minor = p_event_coord->minor;

				if (!ts->coord[t_id].palm && (ts->coord[t_id].ttype == SEC_TS_TOUCHTYPE_PALM))
					ts->coord[t_id].palm_count++;

				ts->coord[t_id].palm = (ts->coord[t_id].ttype == SEC_TS_TOUCHTYPE_PALM);
				ts->coord[t_id].left_event = p_event_coord->left_event;

				if (ts->coord[t_id].z <= 0)
					ts->coord[t_id].z = 1;

				if ((ts->coord[t_id].ttype == SEC_TS_TOUCHTYPE_NORMAL)
						|| (ts->coord[t_id].ttype == SEC_TS_TOUCHTYPE_PALM)
						|| (ts->coord[t_id].ttype == SEC_TS_TOUCHTYPE_WET)
						|| (ts->coord[t_id].ttype == SEC_TS_TOUCHTYPE_GLOVE)
						|| (ts->coord[t_id].ttype == SEC_TS_TOUCHTYPE_SIDE)) {
					if (ts->coord[t_id].ttype == SEC_TS_TOUCHTYPE_SIDE) {
						ts->input_dev = ts->input_dev_side;
						update_side_touch = true;
					} else {
						ts->input_dev = ts->input_dev_touch;
						update_touch = true;
					}

					location = get_location(ts, ts->coord[t_id].x, ts->coord[t_id].y);
					if (ts->coord[t_id].action == SEC_TS_COORDINATE_ACTION_RELEASE) {

						do_gettimeofday(&ts->time_released[t_id]);
						if (ts->coord[t_id].ttype != SEC_TS_TOUCHTYPE_SIDE)
							ts->report_flag[t_id] = false;

						ts->saved_data_x[t_id] = 0;
						ts->saved_data_y[t_id] = 0;

						if (ts->time_longest < (ts->time_released[t_id].tv_sec - ts->time_pressed[t_id].tv_sec))
							ts->time_longest = (ts->time_released[t_id].tv_sec - ts->time_pressed[t_id].tv_sec);

						input_mt_slot(ts->input_dev, t_id);
						if (ts->plat_data->support_mt_pressure)
							input_report_abs(ts->input_dev, ABS_MT_PRESSURE, 0);
						input_mt_report_slot_state(ts->input_dev, MT_TOOL_FINGER, 0);

						if (ts->touch_count > 0)
							ts->touch_count--;
						if (ts->touch_count == 0) {
							input_report_key(ts->input_dev, BTN_TOUCH, 0);
							input_report_key(ts->input_dev, BTN_TOOL_FINGER, 0);
							ts->check_multi = 0;
						}
						if (ts->coord[t_id].ttype == SEC_TS_TOUCHTYPE_SIDE) {
							input_dbg(ts->debug_flag, &ts->client->dev, "[SIDE_TOUCH] RELEASE tID:%d lx:%d ly:%d\n",
								   t_id, ts->coord[t_id].x, ts->coord[t_id].y);
						} else {
#if !defined(CONFIG_SAMSUNG_PRODUCT_SHIP)
						input_dbg(ts->debug_flag, &ts->client->dev,
								"%s[R] tID:%d mc:%d tc:%d lx:%d ly:%d v:%02X%02X cal:%02X id(%d,%d) p:%d noise:%x lp:(%x)\n",
								ts->dex_name,
								t_id, ts->coord[t_id].mcount, ts->touch_count,
								ts->coord[t_id].x, ts->coord[t_id].y,
								ts->plat_data->img_version_of_ic[2],
								ts->plat_data->img_version_of_ic[3],
								ts->cal_status, ts->tspid_val,
								ts->tspicid_val, ts->coord[t_id].palm_count,
								ts->touch_noise_status, ts->lowpower_mode);
#else
						input_dbg(ts->debug_flag, &ts->client->dev,
								"%s[R] tID:%d mc:%d tc:%d v:%02X%02X cal:%02X id(%d,%d) p:%d noise:%x lp:(%x)\n",
								ts->dex_name,
								t_id, ts->coord[t_id].mcount, ts->touch_count,
								ts->plat_data->img_version_of_ic[2],
								ts->plat_data->img_version_of_ic[3],
								ts->cal_status, ts->tspid_val,
								ts->tspicid_val, ts->coord[t_id].palm_count,
								ts->touch_noise_status, ts->lowpower_mode);
#endif
						}
						ts->coord[t_id].action = SEC_TS_COORDINATE_ACTION_NONE;
						ts->coord[t_id].mcount = 0;
						ts->coord[t_id].palm_count = 0;


					} else if (ts->coord[t_id].action == SEC_TS_COORDINATE_ACTION_PRESS) {

						do_gettimeofday(&ts->time_pressed[t_id]);
						ts->touch_count++;
						ts->all_finger_count++;

						if (ts->coord[t_id].ttype != SEC_TS_TOUCHTYPE_SIDE) {
							if (ts->rejection_mode > 0) {
								if (check_area(ts, location, ts->coord[t_id].x, ts->coord[t_id].y)) {
									ts->saved_data_x[t_id] = ts->coord[t_id].x;
									ts->saved_data_y[t_id] = ts->coord[t_id].y;
									goto skip_process;
								}
								ts->report_flag[t_id] = true;
							} else {
								if (ts->landscape) {
									if ((ts->coord[t_id].x < landscape_buffer[0] || ts->coord[t_id].x > ts->plat_data->max_x - landscape_buffer[0]) && (ts->coord[t_id].y < landscape_buffer[1] || ts->coord[t_id].y > ts->plat_data->max_y - landscape_buffer[1])) {
										if (!ts->report_flag[t_id]) {
											ts->saved_data_x[t_id] = ts->coord[t_id].x;
											ts->saved_data_y[t_id] = ts->coord[t_id].y;
											goto skip_process;
										}
									} else
										ts->report_flag[t_id] = true;
								} else {
									if ((ts->coord[t_id].x < portrait_buffer[0] || ts->coord[t_id].x > ts->plat_data->max_x - portrait_buffer[0]) && (ts->coord[t_id].y > ts->plat_data->max_y - portrait_buffer[1])) {
										if (!ts->report_flag[t_id]) {
											ts->saved_data_x[t_id] = ts->coord[t_id].x;
											ts->saved_data_y[t_id] = ts->coord[t_id].y;
											goto skip_process;
										}
									} else
										ts->report_flag[t_id] = true;
								}
							}
						}

						ts->max_z_value = max((unsigned int)ts->coord[t_id].z, ts->max_z_value);
						ts->min_z_value = min((unsigned int)ts->coord[t_id].z, ts->min_z_value);
						ts->sum_z_value += (unsigned int)ts->coord[t_id].z;

						report_touch(ts, t_id, false);
						if (ts->brush_mode)
							input_report_abs(ts->input_dev, ABS_MT_CUSTOM, (ts->coord[t_id].z << 1) | ts->coord[t_id].palm);
						else
							input_report_abs(ts->input_dev, ABS_MT_CUSTOM, (BRUSH_Z_DATA << 1) | ts->coord[t_id].palm);

						if ((ts->touch_count > 4) && (ts->check_multi == 0)) {
							ts->check_multi = 1;
							ts->multi_count++;
						}

						if (ts->coord[t_id].ttype == SEC_TS_TOUCHTYPE_SIDE) {
							input_dbg(ts->debug_flag, &ts->client->dev, "[SIDE_TOUCH] PRESS tID:%d lx:%d ly:%d\n",
								   t_id, ts->coord[t_id].x, ts->coord[t_id].y);
						} else {
#if !defined(CONFIG_SAMSUNG_PRODUCT_SHIP)
						input_dbg(ts->debug_flag, &ts->client->dev,
								"%s[P] tID:%d x:%d y:%d z:%d major:%d minor:%d tc:%d type:%X noise:%x\n",																																																																																
								ts->dex_name, t_id, ts->coord[t_id].x,
								ts->coord[t_id].y, ts->coord[t_id].z,
								ts->coord[t_id].major, ts->coord[t_id].minor,
								ts->touch_count,
								ts->coord[t_id].ttype, ts->touch_noise_status);
#else
						input_dbg(ts->debug_flag, &ts->client->dev,
								"%s[P] tID:%d z:%d major:%d minor:%d tc:%d type:%X noise:%x\n",
								ts->dex_name,
								t_id, ts->coord[t_id].z, ts->coord[t_id].major,
								ts->coord[t_id].minor, ts->touch_count,
								ts->coord[t_id].ttype, ts->touch_noise_status);
#endif
						}
					} else if (ts->coord[t_id].action == SEC_TS_COORDINATE_ACTION_MOVE) {
						if (ts->coord[t_id].ttype != SEC_TS_TOUCHTYPE_SIDE) {
							if (ts->rejection_mode > 0) {
								if (check_area(ts, location, ts->coord[t_id].x, ts->coord[t_id].y))
									goto skip_process;
								if (!ts->report_flag[t_id]) {
									report_touch(ts, t_id, true);
									ts->report_flag[t_id] = true;
								}
							} else {
								if (ts->landscape) {
									if ((ts->coord[t_id].x < landscape_buffer[0] || ts->coord[t_id].x > ts->plat_data->max_x - landscape_buffer[0]) && (ts->coord[t_id].y < landscape_buffer[1] || ts->coord[t_id].y > ts->plat_data->max_y - landscape_buffer[1])) {
										if (!ts->report_flag[t_id])
											goto skip_process;
									} else if ((ts->coord[t_id].x > landscape_buffer[2] && ts->coord[t_id].x < ts->plat_data->max_x - landscape_buffer[2]) || (ts->coord[t_id].y > landscape_buffer[3] && ts->coord[t_id].y < ts->plat_data->max_y - landscape_buffer[3])) {
										if (!ts->report_flag[t_id]) {
											report_touch(ts, t_id, true);
											ts->report_flag[t_id] = true;
										}
									} else if (!ts->report_flag[t_id])
									goto skip_process;
								} else {
									if ((ts->coord[t_id].x < portrait_buffer[0] || ts->coord[t_id].x > ts->plat_data->max_x - portrait_buffer[0]) && (ts->coord[t_id].y > ts->plat_data->max_y - portrait_buffer[1])) {
										if (!ts->report_flag[t_id])
											goto skip_process;
									} else if ((ts->coord[t_id].x > portrait_buffer[2] && ts->coord[t_id].x < ts->plat_data->max_x - portrait_buffer[2]) || (ts->coord[t_id].y < ts->plat_data->max_y - portrait_buffer[3])) {
										if (!ts->report_flag[t_id]) {
											report_touch(ts, t_id, true);
											ts->report_flag[t_id] = true;
										}
									} else if (!ts->report_flag[t_id])
									goto skip_process;
								}
							}
						}

						if ((ts->coord[t_id].ttype == SEC_TS_TOUCHTYPE_GLOVE) && !ts->touchkey_glove_mode_status) {
							ts->touchkey_glove_mode_status = true;
							input_report_switch(ts->input_dev, SW_GLOVE, 1);
						} else if ((ts->coord[t_id].ttype != SEC_TS_TOUCHTYPE_GLOVE) && ts->touchkey_glove_mode_status) {
							ts->touchkey_glove_mode_status = false;
							input_report_switch(ts->input_dev, SW_GLOVE, 0);
						}

						report_touch(ts, t_id, false);

						if (ts->brush_mode)
							input_report_abs(ts->input_dev, ABS_MT_CUSTOM, (ts->coord[t_id].z << 1) | ts->coord[t_id].palm);
						else
							input_report_abs(ts->input_dev, ABS_MT_CUSTOM, (BRUSH_Z_DATA << 1) | ts->coord[t_id].palm);

						ts->coord[t_id].mcount++;

						if (ts->coord[t_id].ttype == SEC_TS_TOUCHTYPE_SIDE)
							input_dbg(ts->debug_flag, &ts->client->dev, "[SIDE_TOUCH] MOVE tID:%d lx:%d ly:%d\n",
								   t_id, ts->coord[t_id].x, ts->coord[t_id].y);
					} else {
						input_dbg(ts->debug_flag, &ts->client->dev,
								"%s: do not support coordinate action(%d)\n", __func__, ts->coord[t_id].action);
					}

					if ((ts->coord[t_id].action == SEC_TS_COORDINATE_ACTION_PRESS)
							|| (ts->coord[t_id].action == SEC_TS_COORDINATE_ACTION_MOVE)) {

						if (ts->coord[t_id].ttype != pre_ttype) {
							input_dbg(ts->debug_flag, &ts->client->dev, "%s : tID:%d ttype(%x->%x)\n",
									__func__, ts->coord[t_id].id,
									pre_ttype, ts->coord[t_id].ttype);
						}
					}

				} else {
					input_dbg(ts->debug_flag, &ts->client->dev,
							"%s: do not support coordinate type(%d)\n", __func__, ts->coord[t_id].ttype);
				}

			} else {
				input_err(true, &ts->client->dev, "%s: tid(%d) is out of range\n", __func__, t_id);
			}
			break;

		case SEC_TS_GESTURE_EVENT:
			p_gesture_status = (struct sec_ts_gesture_status *)event_buff;
			ts->input_dev = ts->input_dev_touch;
			switch (p_gesture_status->gesture_id) {
			case SEC_TS_GESTURE_ID_DOUBLETAP:
				ret = sec_ts_i2c_read(ts, SEC_TS_READ_GESTURE_COORD, tdata, p_gesture_status->gesture_data);
				ts->scrub_x = (tdata[0] << 4) | (tdata[2] >> 4 & 0xF);
				ts->scrub_y = (tdata[1] << 4) | (tdata[2] >> 0 & 0xF);

				input_info(true, &ts->client->dev, "%s: double tap x:%d, y:%d\n",
						__func__, ts->scrub_x, ts->scrub_y);
				for (i = 0; i < AOD_MODE_DOUBLE_TAP; i++) {
					/* Touch the panel */
					ret = incell_get_power_status(&status);
					if (ret)
						input_err(true, &ts->client->dev, "%s: failed get power status\n", __func__);

					if (!status.display_power) {
						input_report_key(ts->input_dev, KEY_POWER, 1);
						input_sync(ts->input_dev);
						input_report_key(ts->input_dev, KEY_POWER, 0);
						input_sync(ts->input_dev);
						break;
					}

					input_mt_slot(ts->input_dev, 0);
					input_mt_report_slot_state(ts->input_dev, MT_TOOL_FINGER, 1);
					input_report_key(ts->input_dev, BTN_TOUCH, 1);
					input_report_key(ts->input_dev, BTN_TOOL_FINGER, 1);
					input_report_abs(ts->input_dev, ABS_MT_POSITION_X, ts->scrub_x + i);
					input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, ts->scrub_y + i);
					input_report_abs(ts->input_dev, ABS_MT_PRESSURE, MAX_PRESSURE - i);
					input_sync(ts->input_dev);
					/* Release finger from the panel */
					input_mt_slot(ts->input_dev, 0);
					input_mt_report_slot_state(ts->input_dev, MT_TOOL_FINGER, 0);
					input_sync(ts->input_dev);
				}
			}
			break;

		default:
			input_err(true, &ts->client->dev, "%s: unknown event %x %x %x %x %x %x\n", __func__,
					event_buff[0], event_buff[1], event_buff[2],
					event_buff[3], event_buff[4], event_buff[5]);
			break;
		}
skip_process:
		curr_pos++;
		remain_event_count--;
	} while (remain_event_count >= 0);
	if (update_touch) {
		ts->input_dev = ts->input_dev_touch;
		input_sync(ts->input_dev);
	} else if (update_side_touch) {
		ts->input_dev = ts->input_dev_side;
		input_sync(ts->input_dev);
	}
}

static irqreturn_t sec_ts_irq_thread(int irq, void *ptr)
{
	struct sec_ts_data *ts = (struct sec_ts_data *)ptr;

	if (ts->power_status == SEC_TS_STATE_POWER_OFF) {
		return IRQ_HANDLED;
	}

	mutex_lock(&ts->eventlock);

	sec_ts_read_event(ts);

	mutex_unlock(&ts->eventlock);

	return IRQ_HANDLED;
}

int get_tsp_status(void)
{
	return 0;
}
EXPORT_SYMBOL(get_tsp_status);

void sec_ts_set_charger(bool enable)
{
	return;
}
EXPORT_SYMBOL(sec_ts_set_charger);

int sec_ts_glove_mode_enables(struct sec_ts_data *ts, int mode)
{
	int ret;

	if (mode)
		ts->touch_functions = (ts->touch_functions | SEC_TS_BIT_SETFUNC_GLOVE | SEC_TS_DEFAULT_ENABLE_BIT_SETFUNC);
	else
		ts->touch_functions = ((ts->touch_functions & (~SEC_TS_BIT_SETFUNC_GLOVE)) | SEC_TS_DEFAULT_ENABLE_BIT_SETFUNC);

	if (ts->power_status == SEC_TS_STATE_POWER_OFF) {
		input_err(true, &ts->client->dev, "%s: pwr off, glove:%d, status:%x\n", __func__,
				mode, ts->touch_functions);
		goto glove_enable_err;
	}

	ret = sec_ts_i2c_write(ts, SEC_TS_CMD_SET_TOUCHFUNCTION, (u8 *)&ts->touch_functions, 2);
	if (ret < 0) {
		input_err(true, &ts->client->dev, "%s: Failed to send command", __func__);
		goto glove_enable_err;
	}

	input_info(true, &ts->client->dev, "%s: glove:%d, status:%x\n", __func__,
			mode, ts->touch_functions);

	return 0;

glove_enable_err:
	return -EIO;
}
EXPORT_SYMBOL(sec_ts_glove_mode_enables);

int sec_ts_set_cover_type(struct sec_ts_data *ts, bool enable)
{
	int ret;

	input_info(true, &ts->client->dev, "%s: %d\n", __func__, ts->flip_enable);

	if (enable)
		ts->touch_functions = (ts->touch_functions | SEC_TS_BIT_SETFUNC_COVER | SEC_TS_DEFAULT_ENABLE_BIT_SETFUNC);
	else
		ts->touch_functions = ((ts->touch_functions & (~SEC_TS_BIT_SETFUNC_COVER)) | SEC_TS_DEFAULT_ENABLE_BIT_SETFUNC);

	if (ts->power_status == SEC_TS_STATE_POWER_OFF) {
		input_err(true, &ts->client->dev, "%s: pwr off, close:%d, status:%x\n", __func__,
				enable, ts->touch_functions);
		goto cover_enable_err;
	}

	if (enable) {
		ret = sec_ts_i2c_write(ts, SEC_TS_CMD_SET_COVERTYPE, &ts->cover_type, 1);
		if (ret < 0) {
			input_err(true, &ts->client->dev, "%s: Failed to send covertype command: %d", __func__, ts->cover_type);
			goto cover_enable_err;
		}
	}

	ret = sec_ts_i2c_write(ts, SEC_TS_CMD_SET_TOUCHFUNCTION, (u8 *)&(ts->touch_functions), 2);
	if (ret < 0) {
		input_err(true, &ts->client->dev, "%s: Failed to send command", __func__);
		goto cover_enable_err;
	}

	input_info(true, &ts->client->dev, "%s: close:%d, status:%x\n", __func__,
			enable, ts->touch_functions);

	return 0;

cover_enable_err:
	return -EIO;


}
EXPORT_SYMBOL(sec_ts_set_cover_type);

void sec_ts_set_grip_type(struct sec_ts_data *ts, u8 set_type)
{
	u8 mode = G_NONE;

	input_info(true, &ts->client->dev, "%s: re-init grip(%d), edh:%d, edg:%d, lan:%d\n", __func__,
			set_type, ts->grip_edgehandler_direction, ts->grip_edge_range, ts->grip_landscape_mode);

	/* edge handler */
	if (ts->grip_edgehandler_direction != 0)
		mode |= G_SET_EDGE_HANDLER;

	if (set_type == GRIP_ALL_DATA) {
		/* edge */
		if (ts->grip_edge_range != 60)
			mode |= G_SET_EDGE_ZONE;

		/* dead zone */
		if (ts->grip_landscape_mode == 1)	/* default 0 mode, 32 */
			mode |= G_SET_LANDSCAPE_MODE;
		else
			mode |= G_SET_NORMAL_MODE;
	}

	if (mode)
		set_grip_data_to_ic(ts, mode);

}

static int sec_ts_get_active_panel(struct device_node *np)
{
	int i;
	int count;
	struct device_node *node;
	struct drm_panel *panel;

	count = of_count_phandle_with_args(np, "panel", NULL);

	for (i = 0; i < count; i++) {
		node = of_parse_phandle(np, "panel", i);
		panel = of_drm_find_panel(node);
		of_node_put(node);
		if (!IS_ERR(panel)) {
			printk("%s Get active panel\n", __func__);
			sec_ts_active_panel = panel;
		} else {
			printk("%s Failed to Get active panel\n", __func__);
		}
	}
	return 0;
}

static int sec_ts_parse_dt(struct i2c_client *client)
{
	struct device *dev = &client->dev;
	struct sec_ts_plat_data *pdata = dev->platform_data;
	struct device_node *np = dev->of_node;
	u32 coords[2];
	int ret = 0;
	int count = 0;
	int lcdtype = 0;

	pdata->irq_gpio = of_get_named_gpio(np, "sec,irq_gpio", 0);
	if (gpio_is_valid(pdata->irq_gpio)) {
		ret = gpio_request_one(pdata->irq_gpio, GPIOF_DIR_IN, "sec,tsp_int");
		if (ret) {
			input_err(true, &client->dev, "%s: Unable to request tsp_int [%d]\n", __func__, pdata->irq_gpio);
			return -EINVAL;
		}
	} else {
		input_err(true, &client->dev, "%s: Failed to get irq gpio\n", __func__);
		return -EINVAL;
	}

	client->irq = gpio_to_irq(pdata->irq_gpio);

	if (of_property_read_u32(np, "sec,irq_type", &pdata->irq_type)) {
		input_err(true, dev, "%s: Failed to get irq_type property\n", __func__);
		pdata->irq_type = IRQF_TRIGGER_LOW | IRQF_ONESHOT;
	} else {
		input_info(true, dev, "%s: irq_type property:%X, %d\n", __func__,
				pdata->irq_type, pdata->irq_type);
	}

	if (of_property_read_u32(np, "sec,i2c-burstmax", &pdata->i2c_burstmax)) {
		input_dbg(true, &client->dev, "%s: Failed to get i2c_burstmax property\n", __func__);
		pdata->i2c_burstmax = 256;
	}

	if (of_property_read_u32_array(np, "sec,max_coords", coords, 2)) {
		input_err(true, &client->dev, "%s: Failed to get max_coords property\n", __func__);
		return -EINVAL;
	}
	pdata->max_x = coords[0] - 1;
	pdata->max_y = coords[1] - 1;

	count = of_property_count_strings(np, "sec,firmware_name");
	if (count <= 0) {
		pdata->firmware_name = NULL;
	} else {
		if (gpio_is_valid(pdata->tsp_id))
			of_property_read_string_index(np, "sec,firmware_name", gpio_get_value(pdata->tsp_id), &pdata->firmware_name);
		else
			of_property_read_string_index(np, "sec,firmware_name", 0, &pdata->firmware_name);
	}

	if (of_property_read_string_index(np, "sec,project_name", 0, &pdata->project_name))
		input_err(true, &client->dev, "%s: skipped to get project_name property\n", __func__);
	if (of_property_read_string_index(np, "sec,project_name", 1, &pdata->model_name))
		input_err(true, &client->dev, "%s: skipped to get model_name property\n", __func__);

	if (of_property_read_u32(np, "sec,always_lpmode", &pdata->always_lpmode) < 0)
		pdata->always_lpmode = 0;

	if (of_property_read_u32(np, "sec,bringup", &pdata->bringup) < 0)
		pdata->bringup = 0;

	if (of_property_read_u32(np, "sec,mis_cal_check", &pdata->mis_cal_check) < 0)
		pdata->mis_cal_check = 0;

	/* delay */
	if (of_property_read_u32(np, "sec,ack_wait_time", &pdata->ack_wait_time)) {
		input_info(false, &client->dev, "%s: Failed to get ack_wait_time\n", __func__);
		pdata->ack_wait_time = 0;
	} else {
		input_info(false, &client->dev, "ack_wait_time : %d\n", pdata->ack_wait_time);
	}

	pdata->regulator_boot_on = of_property_read_bool(np, "sec,regulator_boot_on");
	pdata->support_dex = of_property_read_bool(np, "support_dex_mode");

	pdata->support_mt_pressure = true;

	sec_ts_get_active_panel(np);

	if (of_property_read_u32(np, "somc,touch-rst-gpio",
				 &pdata->touch_rst_gpio)) {
		if (of_property_read_u32(np, "sec,platform-touch-reset-gpio",
					 &pdata->touch_rst_gpio) == 0)
			goto ts_rst_found;

		input_err(true, &client->dev, "%s: Reset GPIO not specified. "
			  "Please specify somc,touch-rst-gpio in your sec_ts "
			  "DT node (hint: qcom,platform-touch-reset-gpio in "
			  " display DT!)\n", __func__);

		/*
		 * It won't work: stop the probe to avoid all sorts of issues.
		 * Beware! This is done because *at least* Sony Seine likes to
		 * trigger short circuit protection on the display if anything
		 * in this driver goes wrong.
		 *
		 * If you return 0 here, you may DAMAGE YOUR HARDWARE!!!
		 */
		return -EINVAL;
	}

ts_rst_found:
	ret = of_property_read_u8_array(np, "sec,device_id", pdata->expected_device_id, 3);
	if (ret) {
		input_err(true, &client->dev, "%s: No expected device id "
				"specified in DT. Using Kumano defaults. Err: %d\n",
				__func__, ret);
		pdata->expected_device_id[0] = 0xAC;
		pdata->expected_device_id[1] = 0x37;
		pdata->expected_device_id[2] = 0x71;
	}

	input_info(true, &client->dev, "%s: i2c buffer limit: %d, lcd_id:%06X, bringup:%d, FW:%s(%d), id:%d,%d, mis_cal:%d dex:%d, gesture:%d\n",
		__func__, pdata->i2c_burstmax, lcdtype, pdata->bringup, pdata->firmware_name,
			count, pdata->tsp_id, pdata->tsp_icid, pdata->mis_cal_check,
			pdata->support_dex);
	return 0;
}

static int sec_ts_parse_dt_feature(struct i2c_client *client)
{
	struct device *dev = &client->dev;
	struct sec_ts_plat_data *pdata = dev->platform_data;
	struct device_node *np = dev->of_node;
	u32 tmp_value = 0;
	u32 rejection_buff[SEC_TS_GRIP_REJECTION_BORDER_NUM];
	u32 portrait_rejection_buff[SEC_TS_GRIP_REJECTION_BORDER_NUM_PORTRAIT];

	if (np == NULL)
		return -ENODEV;

	/* watchdog */
	if (of_property_read_u32(np, "sec,watchdog_supported", &tmp_value)) {
		input_err(true, &client->dev, "Unable to read watchdog_supported\n");
		pdata->watchdog.supported = false;
	} else {
		pdata->watchdog.supported = tmp_value ? true : false;
		input_info(true, &client->dev, "watchdog_supported: %s\n",
			pdata->watchdog.supported ? "true" : "false");

		/* delay */
		if (of_property_read_u32(np, "sec,watchdog_delay_ms", &tmp_value)) {
			input_err(true, &client->dev, "Unable to read watchdog_delay_ms\n");
			pdata->watchdog.delay_ms = 0;
			pdata->watchdog.supported = false;
		} else {
			pdata->watchdog.delay_ms = tmp_value;
			input_info(true, &client->dev, "watchdog_delay_ms: %d\n",
				pdata->watchdog.delay_ms);
		}
	}

	/* feature */
	if (of_property_read_u32(np, "sec,side_touch_supported", &tmp_value)) {
		input_err(true, &client->dev, "Unable to read side_touch_supported\n");
		pdata->side_touch.supported = false;
	} else {
		pdata->side_touch.supported = tmp_value ? true : false;
		input_info(true, &client->dev, "side_touch_supported: %s\n",
			pdata->side_touch.supported ? "true" : "false");
	}

	if (of_property_read_u32(np, "sec,glove_mode_supported", &tmp_value)) {
		input_err(true, &client->dev, "Unable to read glove_mode_supported\n");
		pdata->glove_mode.supported = false;
	} else {
		pdata->glove_mode.supported = tmp_value ? true : false;
		input_info(true, &client->dev, "glove_mode_supported: %s\n",
			pdata->glove_mode.supported ? "true" : "false");
	}

	if (of_property_read_u32(np, "sec,cover_mode_supported", &tmp_value)) {
		input_err(true, &client->dev, "Unable to read cover_mode_supported\n");
		pdata->cover_mode.supported = false;
	} else {
		pdata->cover_mode.supported = tmp_value ? true : false;
		input_info(true, &client->dev, "cover_mode_supported: %s\n",
			pdata->cover_mode.supported ? "true" : "false");
	}

	if (of_property_read_u32(np, "sec,aod_mode_supported", &tmp_value)) {
		input_err(true, &client->dev, "Unable to read aod_mode_supported\n");
		pdata->aod_mode.supported = false;
	} else {
		pdata->aod_mode.supported = tmp_value ? true : false;
		input_info(true, &client->dev, "aod_mode_supported: %s\n",
			pdata->aod_mode.supported ? "true" : "false");
	}

	if (of_property_read_u32(np, "sec,sod_mode_supported", &tmp_value)) {
		input_err(true, &client->dev, "Unable to read sod_mode_supported\n");
		pdata->sod_mode.supported = false;
	} else {
		pdata->sod_mode.supported = tmp_value ? true : false;
		input_info(true, &client->dev, "sod_mode_supported: %s\n",
			pdata->sod_mode.supported ? "true" : "false");
	}

	if (of_property_read_u32(np, "sec,stamina_mode_supported", &tmp_value)) {
		input_err(true, &client->dev, "Unable to read stamina_mode_supported\n");
		pdata->stamina_mode.supported = false;
	} else {
		pdata->stamina_mode.supported = tmp_value ? true : false;
		input_info(true, &client->dev, "stamina_mode_supported: %s\n",
			pdata->stamina_mode.supported ? "true" : "false");
	}

	if (of_property_read_u32_array(np, "sec,rejection_area_portrait", rejection_buff, SEC_TS_GRIP_REJECTION_BORDER_NUM))
		input_err(true, &client->dev, "%s: grip rejection not supported\n", __func__);
	else
		memcpy(portrait_buffer, rejection_buff, sizeof(portrait_buffer));

	if (of_property_read_u32_array(np, "sec,rejection_area_landscape", rejection_buff, SEC_TS_GRIP_REJECTION_BORDER_NUM))
		input_err(true, &client->dev, "%s: grip rejection not supported\n", __func__);
	else
		memcpy(landscape_buffer, rejection_buff, sizeof(landscape_buffer));

	if (of_property_read_u32_array(np, "sec,rejection_area_portrait_ge", portrait_rejection_buff, SEC_TS_GRIP_REJECTION_BORDER_NUM_PORTRAIT))
		input_err(true, &client->dev, "%s: game enhencer grip rejection not supported\n", __func__);
	else
		memcpy(radius_portrait, portrait_rejection_buff, sizeof(radius_portrait));

	if (of_property_read_u32_array(np, "sec,rejection_area_landscape_ge", rejection_buff, SEC_TS_GRIP_REJECTION_BORDER_NUM))
		input_err(true, &client->dev, "%s: game enhencer grip rejection not supported\n", __func__);
	else
		memcpy(radius_landscape, rejection_buff, sizeof(radius_landscape));

	if (of_property_read_u32(np, "sec,wireless_charging_supported", &tmp_value)) {
		input_err(true, &client->dev, "Unable to read wireless_charging_supported\n");
		pdata->wireless_charging.supported = false;
	} else {
		pdata->wireless_charging.supported = tmp_value ? true : false;
		input_info(true, &client->dev, "wireless_charging_supported: %s\n",
			pdata->wireless_charging.supported ? "true" : "false");
	}

	return 0;
}

int sec_ts_read_information(struct sec_ts_data *ts)
{
	unsigned char data[13] = { 0 };
	int ret;

	memset(data, 0x0, 3);
	ret = sec_ts_i2c_read(ts, SEC_TS_READ_ID, data, 3);
	if (ret < 0) {
		input_err(true, &ts->client->dev,
				"%s: failed to read device id(%d)\n",
				__func__, ret);
		return ret;
	}

	input_info(true, &ts->client->dev,
			"%s: %X, %X, %X\n",
			__func__, data[0], data[1], data[2]);
	memset(data, 0x0, 11);
	ret = sec_ts_i2c_read(ts,  SEC_TS_READ_PANEL_INFO, data, 11);
	if (ret < 0) {
		input_err(true, &ts->client->dev,
				"%s: failed to read sub id(%d)\n",
				__func__, ret);
		return ret;
	}

	input_info(true, &ts->client->dev,
			"%s: nTX:%X, nRX:%X, rY:%d, rX:%d\n",
			__func__, data[8], data[9],
			(data[2] << 8) | data[3], (data[0] << 8) | data[1]);

	/* Set X,Y Resolution from IC information. */
	if (((data[0] << 8) | data[1]) > 0)
		ts->plat_data->max_x = ((data[0] << 8) | data[1]) - 1;

	if (((data[2] << 8) | data[3]) > 0)
		ts->plat_data->max_y = ((data[2] << 8) | data[3]) - 1;

	ts->tx_count = data[8];
	ts->rx_count = data[9];

	data[0] = 0;
	ret = sec_ts_i2c_read(ts, SEC_TS_READ_BOOT_STATUS, data, 1);
	if (ret < 0) {
		input_err(true, &ts->client->dev,
				"%s: failed to read sub id(%d)\n",
				__func__, ret);
		return ret;
	}

	input_info(true, &ts->client->dev,
			"%s: STATUS : %X\n",
			__func__, data[0]);

	memset(data, 0x0, 4);
	ret = sec_ts_i2c_read(ts, SEC_TS_READ_TS_STATUS, data, 4);
	if (ret < 0) {
		input_err(true, &ts->client->dev,
				"%s: failed to read sub id(%d)\n",
				__func__, ret);
		return ret;
	}

	input_info(true, &ts->client->dev,
			"%s: TOUCH STATUS : %02X, %02X, %02X, %02X\n",
			__func__, data[0], data[1], data[2], data[3]);
	ret = sec_ts_i2c_read(ts, SEC_TS_CMD_SET_TOUCHFUNCTION,  (u8 *)&(ts->touch_functions), 2);
	if (ret < 0) {
		input_err(true, &ts->client->dev,
				"%s: failed to read touch functions(%d)\n",
				__func__, ret);
		return ret;
	}

	input_info(true, &ts->client->dev,
			"%s: Functions : %02X\n",
			__func__, ts->touch_functions);

	ret = sec_ts_i2c_read(ts, SEC_TS_CMD_ENABLE_STAMINAMODE, (u8 *)&(ts->stamina_enable), 1);
	if (ret < 0) {
		input_err(true, &ts->client->dev,
				"%s: failed to read stamina mode enable(%d)\n",
				__func__, ret);
		return ret;
	}

	input_info(true, &ts->client->dev,
			"%s: Stamina mode : %d\n",
			__func__, ts->stamina_enable);

	ret = sec_ts_i2c_read(ts, SEC_TS_CMD_ENABLE_SIDETOUCH, (u8 *)&data[0], 1);
	if (ret < 0) {
		input_err(true, &ts->client->dev,
				"%s: failed to read side touch enable(%d)\n",
				__func__, ret);
		return ret;
	}
	ts->side_enable = CONVERT_SIDE_ENABLE_MODE_FOR_READ(data[0]);

	input_info(true, &ts->client->dev,
			"%s: Side touch : %d\n",
			__func__, ts->side_enable);

	ret = sec_ts_i2c_read(ts, SEC_TS_CMD_SET_DOZE_TIMEOUT, data, 4);
	if (ret < 0) {
		input_err(true, &ts->client->dev,
				"%s: failed to read idle timeout setting(%d)\n",
				__func__, ret);
		return ret;
	}
	ts->doze_timeout = ((data[0] << 8) & 0xFF00) + (data[1] & 0x00FF);
	ts->doze_timeout = (ts->doze_timeout * 833) / 100;

	input_info(true, &ts->client->dev,
			"%s: Doze timeout : %d\n",
			__func__, ts->doze_timeout);

	return ret;
}

static void sec_ts_set_input_prop(struct sec_ts_data *ts, struct input_dev *dev, u8 propbit)
{
	static char sec_ts_phys[64] = { 0 };

	snprintf(sec_ts_phys, sizeof(sec_ts_phys), "%s/input1",
			dev->name);
	dev->phys = sec_ts_phys;
	dev->id.bustype = BUS_I2C;
	dev->dev.parent = &ts->client->dev;

	set_bit(EV_SYN, dev->evbit);
	set_bit(EV_KEY, dev->evbit);
	set_bit(EV_ABS, dev->evbit);
	set_bit(EV_SW, dev->evbit);
	set_bit(BTN_TOUCH, dev->keybit);
	set_bit(BTN_TOOL_FINGER, dev->keybit);
	// set_bit(KEY_BLACK_UI_GESTURE, dev->keybit);
#ifdef SEC_TS_SUPPORT_TOUCH_KEY
	if (ts->plat_data->support_mskey) {
		int i;

		for (i = 0 ; i < ts->plat_data->num_touchkey ; i++)
			set_bit(ts->plat_data->touchkey[i].keycode, dev->keybit);

		set_bit(EV_LED, dev->evbit);
		set_bit(LED_MISC, dev->ledbit);
	}
#endif

	set_bit(propbit, dev->propbit);
	set_bit(KEY_HOMEPAGE, dev->keybit);

	input_set_capability(dev, EV_SW, SW_GLOVE);
	input_set_capability(dev, EV_KEY, KEY_POWER);

	input_set_abs_params(dev, ABS_MT_POSITION_X, 0, ts->plat_data->max_x, 0, 0);
	input_set_abs_params(dev, ABS_MT_POSITION_Y, 0, ts->plat_data->max_y, 0, 0);
	input_set_abs_params(dev, ABS_MT_TOUCH_MAJOR, 0, 255, 0, 0);
	input_set_abs_params(dev, ABS_MT_TOUCH_MINOR, 0, 255, 0, 0);
	input_set_abs_params(dev, ABS_MT_CUSTOM, 0, 0xFFFFFFFF, 0, 0);
	if (ts->plat_data->support_mt_pressure)
		input_set_abs_params(dev, ABS_MT_PRESSURE, 0, 255, 0, 0);

	if (propbit == INPUT_PROP_POINTER)
		input_mt_init_slots(dev, MAX_SUPPORT_TOUCH_COUNT, INPUT_MT_POINTER);
	else
		input_mt_init_slots(dev, MAX_SUPPORT_TOUCH_COUNT, INPUT_MT_DIRECT);

	input_set_drvdata(dev, ts);
}

int sec_ts_get_pw_status(void)
{
	int ret = 0;
	incell_pw_status status = { false, false };

	ret = incell_get_power_status(&status);
	if (ret) {
		pr_err("%s: failed get power status\n", __func__);
		goto exit;
	}

	if (status.display_power && status.touch_power) {
		ret = INCELL_OK;
	} else {
		pr_err("%s: power status: disable\n", __func__);
		ret = INCELL_ERROR;
	}

exit:
	return ret;
}

int sec_ts_pw_lock(struct sec_ts_data *ts, incell_pw_lock status)
{
	int ret = 0;
	incell_pw_status pwr_status = { false, false };

	if (status == INCELL_DISPLAY_POWER_LOCK) {
		atomic_inc(&ts->lock_cnt);
		input_dbg(ts->debug_flag, &ts->client->dev, "POWER_LOCK_COUNT [%d]\n", ts->lock_cnt);
		if (atomic_read(&ts->lock_cnt) != 1)
			goto out;
	} else {
		atomic_dec(&ts->lock_cnt);
		input_dbg(ts->debug_flag, &ts->client->dev, "POWER_UNLOCK_COUNT [%d]\n", ts->lock_cnt);
		if (atomic_read(&ts->lock_cnt) != 0)
			goto out;
	}

	ret = incell_power_lock_ctrl(status, &pwr_status);

	switch (ret) {
	case INCELL_OK:
		input_dbg(ts->debug_flag, &ts->client->dev, "power status: %s\n", status ? "LOCK" : "UNLOCK");
		break;
	case INCELL_ERROR:
	case INCELL_ALREADY_LOCKED:
	case INCELL_ALREADY_UNLOCKED:
	default:
		input_info(true, &ts->client->dev, "power lock failed ret:%d\n", ret);
		break;
	}

out:
	return ret;
}

static void sec_ts_watchdog_func(struct work_struct *work_watchdog)
{
	struct sec_ts_data *ts = container_of(to_delayed_work(work_watchdog), struct sec_ts_data, work_watchdog);
	u8 mode = 0;
	u8 deviceId[3] = { 0 };
	int ret = 0;

	if (ts->power_status == SEC_TS_STATE_POWER_OFF || (ts->power_status == SEC_TS_STATE_LPM && !ts->lowpower_mode)) {
		input_err(true, &ts->client->dev, "%s: POWER_STATUS : %s\n", __func__, ts->power_status ? "LPM" : "OFF");
		return;
	}

	mutex_lock(&ts->lock);
	ret = sec_ts_pw_lock(ts, INCELL_DISPLAY_POWER_LOCK);
	if (ret) {
		input_err(true, &ts->client->dev, "failed to lock power\n");
		goto unlock;
	}

	ret = sec_ts_i2c_read(ts, SEC_TS_READ_BOOT_STATUS, &mode, 1);
	if (ret < 0) {
		input_err(true, &ts->client->dev, "%s: failed to read boot status(%d)\n", __func__, ret);
		goto unlock;
	}

	if (mode == SEC_TS_STATUS_BOOT_MODE)
		goto unlock;

	ret = sec_ts_i2c_read(ts, SEC_TS_READ_ID, deviceId, 3);
	if (ret < 0) {
		input_err(true, &ts->client->dev, "%s: failed to read device id(%d)\n", __func__, ret);
		goto unlock;
	}

	if (deviceId[0] != ts->plat_data->expected_device_id[0] ||
	    deviceId[1] != ts->plat_data->expected_device_id[1] ||
	    deviceId[2] != ts->plat_data->expected_device_id[2]) {
		input_err(true, &ts->client->dev, "improper device id\n");
		pr_err("Expected ID: %x %x %x", ts->plat_data->expected_device_id[0], ts->plat_data->expected_device_id[1], ts->plat_data->expected_device_id[2]);
		pr_err("Instead got: %x %x %x", deviceId[0], deviceId[1], deviceId[2]);
		gpio_set_value(ts->plat_data->touch_rst_gpio, 0);
		mdelay(5);
		gpio_set_value(ts->plat_data->touch_rst_gpio, 1);
		mdelay(200);
	}

unlock:
	schedule_delayed_work(&ts->work_watchdog, msecs_to_jiffies(ts->plat_data->watchdog.delay_ms));
	ret = sec_ts_pw_lock(ts, INCELL_DISPLAY_POWER_UNLOCK);
	if (ret) {
		input_err(true, &ts->client->dev, "failed to unlock power\n");
	}
	mutex_unlock(&ts->lock);
}

static int sec_ts_after_init(struct sec_ts_data *ts)
{
	int ret = 0;
	bool force_update = false;
	bool valid_firmware_integrity = false;
	unsigned char data[5] = { 0 };
	unsigned char deviceID[5] = { 0 };
	unsigned char result = 0;

	input_info(true, &ts->client->dev, "%s: start\n", __func__);

	ts->power_status = SEC_TS_STATE_POWER_ON;
	input_info(true, &ts->client->dev, "%s: request_irq = %d\n", __func__, ts->client->irq);
	if (!ts->irq_req) {
		ret = request_threaded_irq(ts->client->irq, NULL, sec_ts_irq_thread,
				ts->plat_data->irq_type, SEC_TS_I2C_NAME, ts);
		input_dbg(true, &ts->client->dev, "%s: ret %d\n", __func__, ret);
		if (ret < 0) {
			input_err(true, &ts->client->dev, "%s: Unable to request threaded irq\n", __func__);
			goto err;
		}
		ts->irq_req = true;
	}

	ts->irq_wake_mask = false;
	ts->irq_status = true;
	sec_ts_set_irq(ts, false);

	if (ts->plat_data->ack_wait_time) {
		input_info(true, &ts->client->dev, "%s: wait [%d]\n", __func__, ts->plat_data->ack_wait_time);
		sec_ts_delay(ts->plat_data->ack_wait_time);
	}

	ret = sec_ts_i2c_read(ts, SEC_TS_READ_DEVICE_ID, deviceID, 5);
	if (ret < 0)
		input_err(true, &ts->client->dev, "%s: failed to read device ID(%d)\n", __func__, ret);
	else
		input_info(true, &ts->client->dev,
				"%s: TOUCH DEVICE ID : %02X, %02X, %02X, %02X, %02X\n", __func__,
				deviceID[0], deviceID[1], deviceID[2], deviceID[3], deviceID[4]);

	ret = sec_ts_i2c_read(ts, SEC_TS_READ_FIRMWARE_INTEGRITY, &result, 1);
	if (ret < 0) {
		input_err(true, &ts->client->dev, "%s: failed to integrity check (%d)\n", __func__, ret);
	} else {
		if (result & 0x80) {
			valid_firmware_integrity = true;
		} else if (result & 0x40) {
			valid_firmware_integrity = false;
			input_err(true, &ts->client->dev, "%s: invalid firmware (0x%x)\n", __func__, result);
		} else {
			valid_firmware_integrity = false;
			input_err(true, &ts->client->dev, "%s: invalid integrity result (0x%x)\n", __func__, result);
		}
	}

	ret = sec_ts_i2c_read(ts, SEC_TS_READ_BOOT_STATUS, &data[0], 1);
	if (ret < 0) {
		input_err(true, &ts->client->dev,
				"%s: failed to read sub id(%d)\n",
				__func__, ret);
	} else {
		ret = sec_ts_i2c_read(ts, SEC_TS_READ_TS_STATUS, &data[1], 4);
		if (ret < 0) {
			input_err(true, &ts->client->dev,
					"%s: failed to touch status(%d)\n",
					__func__, ret);
		}
	}
	input_info(true, &ts->client->dev,
			"%s: TOUCH STATUS : %02X || %02X, %02X, %02X, %02X\n",
			__func__, data[0], data[1], data[2], data[3], data[4]);

	if (data[0] == SEC_TS_STATUS_BOOT_MODE)
		ts->checksum_result = 1;

	if ((data[0] == SEC_TS_STATUS_APP_MODE && data[2] == TOUCH_SYSTEM_MODE_FLASH) ||
			(valid_firmware_integrity == false))
		force_update = true;
	else
		force_update = false;

#ifdef SEC_TS_FW_UPDATE_ON_PROBE
	ret = sec_ts_firmware_update_on_probe(ts, force_update);
	if (ret < 0) {
		input_err(true, &ts->client->dev, "%s: fail firmware update on probe\n", __func__);
	}
#else
	input_info(true, &ts->client->dev, "%s: fw update on probe disabled!\n", __func__);
#endif

	ret = sec_ts_read_information(ts);
	if (ret < 0) {
		input_err(true, &ts->client->dev, "%s: fail to read information 0x%x\n", __func__, ret);
		goto err;
	}

	ts->touch_functions |= SEC_TS_DEFAULT_ENABLE_BIT_SETFUNC;
	ret = sec_ts_i2c_write(ts, SEC_TS_CMD_SET_TOUCHFUNCTION, (u8 *)&ts->touch_functions, 2);
	if (ret < 0)
		input_err(true, &ts->client->dev, "%s: Failed to send touch func_mode command", __func__);

	/* Sense_on */
	ret = sec_ts_i2c_write(ts, SEC_TS_CMD_SENSE_ON, NULL, 0);
	if (ret < 0) {
		input_err(true, &ts->client->dev, "%s: fail to write Sense_on\n", __func__);
		return ret;
	}

	ts->pFrame = kzalloc(ts->tx_count * ts->rx_count * 2, GFP_KERNEL);
	if (!ts->pFrame) {
		ret = -ENOMEM;
		goto err;
	}

	if (ts->plat_data->watchdog.supported)
		schedule_delayed_work(&ts->work_watchdog, msecs_to_jiffies(ts->plat_data->watchdog.delay_ms));

#ifdef USE_SELF_TEST_WORK
	schedule_delayed_work(&ts->work_read_info, msecs_to_jiffies(50));
#endif
	if (ts->after_work.err) {
		ts->after_work.done = true;
		ts->after_work.err = false;
	}
	sec_ts_set_irq(ts, true);

	ts->report_rejected_event_flag = report_rejected_event;

	input_info(true, &ts->client->dev, "%s: success\n", __func__);
	return ret;
err:
	ts->power_status = SEC_TS_STATE_POWER_OFF;
	input_err(true, &ts->client->dev, "%s: failed\n", __func__);
	return -EINVAL;
}

static int sec_ts_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct sec_ts_data *ts;
	struct sec_ts_plat_data *pdata;
	int ret = 0;
	int i = 0;

	input_info(true, &client->dev, "%s: start\n", __func__);

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		input_err(true, &client->dev, "%s: EIO err!\n", __func__);
		return -EIO;
	}

	/* parse dt */
	if (client->dev.of_node) {
		pdata = devm_kzalloc(&client->dev,
				sizeof(struct sec_ts_plat_data), GFP_KERNEL);

		if (!pdata) {
			input_err(true, &client->dev, "%s: Failed to allocate platform data\n", __func__);
			goto error_allocate_pdata;
		}

		client->dev.platform_data = pdata;

		ret = sec_ts_parse_dt(client);
		if (ret) {
			input_err(true, &client->dev, "%s: Failed to parse dt\n", __func__);
			goto error_allocate_mem;
		}

		ret = sec_ts_parse_dt_feature(client);
		if (ret) {
			input_err(true, &client->dev, "%s: Failed to parse dt feature\n", __func__);
			goto error_allocate_mem;
		}
	} else {
		pdata = client->dev.platform_data;
		if (!pdata) {
			input_err(true, &client->dev, "%s: No platform data found\n", __func__);
			goto error_allocate_pdata;
		}
	}

	ts = kzalloc(sizeof(struct sec_ts_data), GFP_KERNEL);
	if (!ts)
		goto error_allocate_mem;

	ts->client = client;
	ts->plat_data = pdata;
	ts->crc_addr = 0x0001FE00;
	ts->fw_addr = 0x00002000;
	ts->para_addr = 0x18000;
	ts->flash_page_size = SEC_TS_FW_BLK_SIZE_DEFAULT;
	ts->sec_ts_i2c_read = sec_ts_i2c_read;
	ts->sec_ts_i2c_write = sec_ts_i2c_write;
	ts->sec_ts_i2c_write_burst = sec_ts_i2c_write_burst;
	ts->sec_ts_i2c_read_bulk = sec_ts_i2c_read_bulk;
	ts->i2c_burstmax = pdata->i2c_burstmax;

	ts->touch_count = 0;

	ts->max_z_value = 0;
	ts->min_z_value = 0xFFFFFFFF;
	ts->sum_z_value = 0;
#ifdef USE_POWER_RESET_WORK
	INIT_DELAYED_WORK(&ts->reset_work, sec_ts_reset_work);
#endif
#ifdef USE_SELF_TEST_WORK
	INIT_DELAYED_WORK(&ts->work_read_info, sec_ts_read_info_work);
#endif
	INIT_DELAYED_WORK(&ts->after_work.start, sec_ts_after_init_work);
	if (ts->plat_data->watchdog.supported)
		INIT_DELAYED_WORK(&ts->work_watchdog, sec_ts_watchdog_func);

	i2c_set_clientdata(client, ts);

	if (gpio_is_valid(ts->plat_data->tsp_id))
		ts->tspid_val = gpio_get_value(ts->plat_data->tsp_id);

	if (gpio_is_valid(ts->plat_data->tsp_icid))
		ts->tspicid_val = gpio_get_value(ts->plat_data->tsp_icid);

	ts->input_dev = input_allocate_device();
	if (!ts->input_dev) {
		input_err(true, &ts->client->dev, "%s: allocate device err!\n", __func__);
		ret = -ENOMEM;
		goto err_allocate_input_dev;
	}

	if (ts->plat_data->support_dex) {
		ts->input_dev_pad = input_allocate_device();
		if (!ts->input_dev_pad) {
			input_err(true, &ts->client->dev, "%s: allocate device err!\n", __func__);
			ret = -ENOMEM;
			goto err_allocate_input_dev_pad;
		}
	}

	ts->input_dev_side = input_allocate_device();
	if (!ts->input_dev_side) {
		input_err(true, &ts->client->dev, "%s: allocate device err!\n", __func__);
		ret = -ENOMEM;
		goto err_allocate_input_dev_side;
	}

	ts->doze_timeout = 250;
	ts->side_enable = OFF;
	ts->stamina_enable = 0;

	for (i = 0; i < SEC_TS_GRIP_REJECTION_BORDER_NUM; i++) {
		ts->circle_range_l[i] = radius_landscape[i] * radius_landscape[i];
		if (i < SEC_TS_GRIP_REJECTION_BORDER_NUM_PORTRAIT)
			ts->circle_range_p[i] = radius_portrait[i] * radius_portrait[i];
	}

	mutex_init(&ts->lock);
	mutex_init(&ts->device_mutex);
	mutex_init(&ts->i2c_mutex);
	mutex_init(&ts->eventlock);
	mutex_init(&ts->modechange);
	mutex_init(&ts->irq_mutex);
	mutex_init(&ts->aod_mutex);

	init_completion(&ts->resume_done);
	complete_all(&ts->resume_done);

	atomic_set(&ts->lock_cnt, 0);

	input_info(true, &client->dev, "%s: init resource\n", __func__);

	if (!pdata->regulator_boot_on)
		sec_ts_delay(70);
	ts->external_factory = false;

	input_info(true, &client->dev, "%s: power enable\n", __func__);

	/* init notification callbacks */
	ts->drm_notif.notifier_call = drm_notifier_callback;
	if (sec_ts_active_panel) {
		ret = drm_panel_notifier_register(sec_ts_active_panel, &ts->drm_notif);
		if (ret < 0) {
			input_err(true, &ts->client->dev, "Unable to register drm_notifier: %d\n", ret);
			goto err_notifier_register_client;
		}
	}

	if (ts->plat_data->support_dex) {
		ts->input_dev_pad->name = "sec_touchpad";
		sec_ts_set_input_prop(ts, ts->input_dev_pad, INPUT_PROP_POINTER);
	}
	ts->dex_name = "";

	ts->input_dev->name = "sec_touchscreen";
	sec_ts_set_input_prop(ts, ts->input_dev, INPUT_PROP_DIRECT);

	ts->input_dev_side->name = "sec_touchscreen_side";
	sec_ts_set_input_prop(ts, ts->input_dev_side, INPUT_PROP_DIRECT);

	ts->input_dev_touch = ts->input_dev;

#ifdef USE_OPEN_CLOSE
	ts->input_dev->open = sec_ts_input_open;
	ts->input_dev->close = sec_ts_input_close;
#endif

	ret = input_register_device(ts->input_dev);
	if (ret) {
		input_err(true, &ts->client->dev, "%s: Unable to register %s input device\n", __func__, ts->input_dev->name);
		goto err_input_register_device;
	}
	if (ts->plat_data->support_dex) {
		ret = input_register_device(ts->input_dev_pad);
		if (ret) {
			input_err(true, &ts->client->dev, "%s: Unable to register %s input device\n", __func__, ts->input_dev_pad->name);
			goto err_input_pad_register_device;
		}
	}

	ret = input_register_device(ts->input_dev_side);
	if (ret) {
		input_err(true, &ts->client->dev, "%s: Unable to register %s input device\n", __func__, ts->input_dev_side->name);
		goto err_input_side_register_device;
	}

	sec_ts_raw_device_init(ts);
	sec_ts_fn_init(ts);

	device_init_wakeup(&client->dev, true);

	ts_dup = ts;
	ts->probe_done = true;

	input_info(true, &ts->client->dev, "%s: done\n", __func__);
	input_log_fix();

	ts->after_work.err = true;
	schedule_delayed_work(&ts->after_work.start, msecs_to_jiffies(12000));

	return 0;

err_input_side_register_device:
	input_unregister_device(ts->input_dev_pad);
	ts->input_dev_pad = NULL;
err_input_pad_register_device:
	input_unregister_device(ts->input_dev);
	ts->input_dev = NULL;
	ts->input_dev_touch = NULL;
err_input_register_device:
	if (sec_ts_active_panel)
		drm_panel_notifier_unregister(sec_ts_active_panel, &ts->drm_notif);
	kfree(ts->pFrame);
	device_init_wakeup(&client->dev, false);
	if (ts->plat_data->support_dex) {
		if (ts->input_dev_pad)
			input_free_device(ts->input_dev_pad);
	}
err_notifier_register_client:
	if (ts->input_dev_side)
		input_free_device(ts->input_dev_side);
err_allocate_input_dev_side:
	if (ts->input_dev_pad)
		input_free_device(ts->input_dev_pad);
err_allocate_input_dev_pad:
	if (ts->input_dev)
		input_free_device(ts->input_dev);
err_allocate_input_dev:
	kfree(ts);

error_allocate_mem:
	if (gpio_is_valid(pdata->irq_gpio))
		gpio_free(pdata->irq_gpio);
	if (gpio_is_valid(pdata->tsp_id))
		gpio_free(pdata->tsp_id);
	if (gpio_is_valid(pdata->tsp_icid))
		gpio_free(pdata->tsp_icid);

error_allocate_pdata:
	if (ret == -ECONNREFUSED)
		sec_ts_delay(100);
	ret = -ENODEV;
#ifdef CONFIG_TOUCHSCREEN_DUMP_MODE
	p_ghost_check = NULL;
#endif
	ts_dup = NULL;
	input_err(true, &client->dev, "%s: failed(%d)\n", __func__, ret);
	input_log_fix();
	return ret;
}

void sec_ts_unlocked_release_all_finger(struct sec_ts_data *ts)
{
	int i;

	ts->input_dev = ts->input_dev_touch;

	for (i = 0; i < MAX_SUPPORT_TOUCH_COUNT; i++) {
		input_mt_slot(ts->input_dev, i);
		input_mt_report_slot_state(ts->input_dev, MT_TOOL_FINGER, false);

		if ((ts->coord[i].action == SEC_TS_COORDINATE_ACTION_PRESS) ||
			(ts->coord[i].action == SEC_TS_COORDINATE_ACTION_MOVE)) {

			ts->coord[i].action = SEC_TS_COORDINATE_ACTION_RELEASE;
			input_info(true, &ts->client->dev,
					"%s: [RA] tID:%d mc:%d tc:%d v:%02X%02X cal:%02X id(%d,%d) p:%d\n",
					__func__, i, ts->coord[i].mcount, ts->touch_count,
					ts->plat_data->img_version_of_ic[2],
					ts->plat_data->img_version_of_ic[3],
					ts->cal_status, ts->tspid_val,
					ts->tspicid_val, ts->coord[i].palm_count);

			do_gettimeofday(&ts->time_released[i]);
			
			if (ts->time_longest < (ts->time_released[i].tv_sec - ts->time_pressed[i].tv_sec))
				ts->time_longest = (ts->time_released[i].tv_sec - ts->time_pressed[i].tv_sec);
		}

		ts->coord[i].mcount = 0;
		ts->coord[i].palm_count = 0;

	}

	input_mt_slot(ts->input_dev, 0);

	input_report_key(ts->input_dev, BTN_TOUCH, false);
	input_report_key(ts->input_dev, BTN_TOOL_FINGER, false);
	input_report_switch(ts->input_dev, SW_GLOVE, false);
	ts->touchkey_glove_mode_status = false;
	ts->touch_count = 0;
	ts->check_multi = 0;

	input_report_key(ts->input_dev, KEY_HOMEPAGE, 0);
	input_sync(ts->input_dev);

	if (ts->input_dev_side) {
		for (i = 0; i < MAX_SUPPORT_TOUCH_COUNT; i++) {
			input_mt_slot(ts->input_dev_side, i);
			input_mt_report_slot_state(ts->input_dev_side, MT_TOOL_FINGER, 0);
		}

		input_mt_report_pointer_emulation(ts->input_dev_side, false);
		input_sync(ts->input_dev_side);
	}
}

static int sec_ts_feature_settings(struct sec_ts_data *ts)
{
	int ret = 0;
	u8 tBuff[3] = { 0 };

	input_info(true, &ts->client->dev, "%s: start\n", __func__);

	/* cover */
	if (ts->flip_enable) {
		ret = sec_ts_i2c_write(ts, SEC_TS_CMD_SET_COVERTYPE, &ts->cover_type, 1);
		if (ret < 0)
			goto err;

		ts->touch_functions = ts->touch_functions | SEC_TS_BIT_SETFUNC_COVER;
		input_info(true, &ts->client->dev,
				"%s: cover cmd write type:%d, mode:%x, ret:%d\n",
				__func__, ts->touch_functions, ts->cover_type, ret);
	} else {
		ts->touch_functions = (ts->touch_functions & (~SEC_TS_BIT_SETFUNC_COVER));
		input_info(true, &ts->client->dev,
				"%s: cover open, not send cmd\n", __func__);
	}

	/* glove */
	if (ts->plat_data->glove_mode.status)
		ts->touch_functions = ts->touch_functions | SEC_TS_BIT_SETFUNC_GLOVE;
	else
		ts->touch_functions = ts->touch_functions & (~SEC_TS_BIT_SETFUNC_GLOVE);

	ts->touch_functions = ts->touch_functions | SEC_TS_DEFAULT_ENABLE_BIT_SETFUNC;
	ret = sec_ts_i2c_write(ts, SEC_TS_CMD_SET_TOUCHFUNCTION, (u8 *)&ts->touch_functions, 2);
	if (ret < 0) {
		input_err(true, &ts->client->dev,
				"%s: Failed to send touch function command\n", __func__);
		goto err;
	}

	/* side touch */
	if (ts->plat_data->wireless_charging.status)
		tBuff[0] = 0;
	else
		tBuff[0] = CONVERT_SIDE_ENABLE_MODE_FOR_WRITE(ts->side_enable);
	ret = sec_ts_i2c_write(ts, SEC_TS_CMD_ENABLE_SIDETOUCH, tBuff, 1);
	if (ret < 0) {
		input_err(true, &ts->client->dev,
				"%s: Failed to send sidetouch enable command\n", __func__);
		goto err;
	}

	/* stamina mode */
	ret = sec_ts_i2c_write(ts, SEC_TS_CMD_ENABLE_STAMINAMODE, (u8 *)&(ts->stamina_enable), 1);
	if (ret < 0) {
		input_err(true, &ts->client->dev,
				"%s: Failed to send stamina enable command\n", __func__);
		goto err;
	}

	/* doze */
	tBuff[0] = 0x01;
	tBuff[1] = (ts->doze_timeout & 0xFF00) >> 8;
	tBuff[2] = (ts->doze_timeout & 0x00FF) >> 0;

	ret = sec_ts_i2c_write(ts, SEC_TS_CMD_SET_DOZE_TIMEOUT, tBuff, 3);
	if (ret < 0) {
		input_err(true, &ts->client->dev,
				"%s: Failed to set doze timeout command\n", __func__);
		goto err;
	}

	input_info(true, &ts->client->dev, "%s: cover:%d, glove:%d, side:%d, stamina:%d, doze timeout:%d\n",
			__func__, ts->cover_type, ts->touch_functions, ts->side_enable, ts->stamina_enable, ts->doze_timeout);

	input_info(true, &ts->client->dev, "%s: success\n", __func__);

err:
	return ret;
}

void sec_ts_locked_release_all_finger(struct sec_ts_data *ts)
{
	int i;

	ts->input_dev = ts->input_dev_touch;

	mutex_lock(&ts->eventlock);

	for (i = 0; i < MAX_SUPPORT_TOUCH_COUNT; i++) {
		input_mt_slot(ts->input_dev, i);
		input_mt_report_slot_state(ts->input_dev, MT_TOOL_FINGER, false);

		if ((ts->coord[i].action == SEC_TS_COORDINATE_ACTION_PRESS) ||
			(ts->coord[i].action == SEC_TS_COORDINATE_ACTION_MOVE)) {

			ts->coord[i].action = SEC_TS_COORDINATE_ACTION_RELEASE;
			input_info(true, &ts->client->dev,
					"%s: [RA] tID:%d mc: %d tc:%d, v:%02X%02X, cal:%X id(%d,%d), p:%d\n",
					__func__, i, ts->coord[i].mcount, ts->touch_count,
					ts->plat_data->img_version_of_ic[2],
					ts->plat_data->img_version_of_ic[3],
					ts->cal_status, ts->tspid_val, ts->tspicid_val, ts->coord[i].palm_count);

			do_gettimeofday(&ts->time_released[i]);
			
			if (ts->time_longest < (ts->time_released[i].tv_sec - ts->time_pressed[i].tv_sec))
				ts->time_longest = (ts->time_released[i].tv_sec - ts->time_pressed[i].tv_sec);
		}

		ts->coord[i].mcount = 0;
		ts->coord[i].palm_count = 0;

	}

	input_mt_slot(ts->input_dev, 0);

	input_report_key(ts->input_dev, BTN_TOUCH, false);
	input_report_key(ts->input_dev, BTN_TOOL_FINGER, false);
	input_report_switch(ts->input_dev, SW_GLOVE, false);
	ts->touchkey_glove_mode_status = false;
	ts->touch_count = 0;
	ts->check_multi = 0;

	input_report_key(ts->input_dev, KEY_HOMEPAGE, 0);
	input_sync(ts->input_dev);

	if (ts->input_dev_side) {
		for (i = 0; i < MAX_SUPPORT_TOUCH_COUNT; i++) {
			input_mt_slot(ts->input_dev_side, i);
			input_mt_report_slot_state(ts->input_dev_side, MT_TOOL_FINGER, 0);
		}

		input_mt_report_pointer_emulation(ts->input_dev_side, false);
		input_sync(ts->input_dev_side);
	}

	mutex_unlock(&ts->eventlock);

}

#ifdef USE_POWER_RESET_WORK
static void sec_ts_reset_work(struct work_struct *work)
{
	struct sec_ts_data *ts = container_of(work, struct sec_ts_data,
			reset_work.work);
	int ret;

	if (ts->reset_is_on_going) {
		input_err(true, &ts->client->dev, "%s: reset is ongoing\n", __func__);
		return;
	}

	mutex_lock(&ts->modechange);
	pm_relax(&ts->client->dev);

	ts->reset_is_on_going = true;
	input_info(true, &ts->client->dev, "%s\n", __func__);

	sec_ts_stop_device(ts);

	sec_ts_delay(30);

	ret = sec_ts_start_device(ts);
	if (ret < 0) {
		input_err(true, &ts->client->dev, "%s: failed to reset, ret:%d\n", __func__, ret);
		ts->reset_is_on_going = false;
		cancel_delayed_work(&ts->reset_work);
		schedule_delayed_work(&ts->reset_work, msecs_to_jiffies(TOUCH_RESET_DWORK_TIME));
		mutex_unlock(&ts->modechange);

		pm_relax(&ts->client->dev);

		return;
	}

	ts->reset_is_on_going = false;
	mutex_unlock(&ts->modechange);
	pm_relax(&ts->client->dev);
}
#endif

#ifdef USE_SELF_TEST_WORK
static void sec_ts_read_info_work(struct work_struct *work)
{
	struct sec_ts_data *ts = container_of(work, struct sec_ts_data,
			work_read_info.work);
	char para = TO_TOUCH_MODE;
	int ret;

	input_info(true, &ts->client->dev, "%s\n", __func__);

	/* run self-test */
	sec_ts_set_irq(ts, false);
	execute_selftest(ts, false);
	sec_ts_set_irq(ts, true);

	input_info(true, &ts->client->dev, "%s: %02X %02X %02X %02X\n",
		__func__, ts->ito_test[0], ts->ito_test[1]
		, ts->ito_test[2], ts->ito_test[3]);

	ret = ts->sec_ts_i2c_write(ts, SEC_TS_CMD_SET_POWER_MODE, &para, 1);
	if (ret < 0)
		 input_err(true, &ts->client->dev, "%s: Failed to set\n", __func__);

	sec_ts_delay(350);

	input_log_fix();

	sec_ts_run_rawdata_all(ts, false);
	ts->info_work_done = true;
}
#endif

#define SEC_TS_RETRY_SESSION_COUNT 30
static void sec_ts_after_init_work(struct work_struct *work)
{
	struct sec_ts_data *ts = container_of(work, struct sec_ts_data,
					after_work.start.work);
#ifdef SEC_TS_FW_UPDATE_ON_PROBE
	const struct firmware *fw_entry;
#endif
	int ret = 0;


	if (ts->after_work.done) {
		input_info(true, &ts->client->dev, "already after_init_work\n");
			goto out;
	}

	input_info(true, &ts->client->dev, "%s: session_work start\n", __func__);

	if (sec_ts_get_pw_status()) {
		input_info(true, &ts->client->dev, "params update, will update feature on resume\n");
		goto after_work_fail;
	}

#ifdef SEC_TS_FW_UPDATE_ON_PROBE
	if (is_first_ts_kickstart) {
		if (request_firmware(&fw_entry,
				     ts->plat_data->firmware_name,
				     &ts->client->dev) !=  0) {
			input_info(true, &ts->client->dev,
			   "Userspace is not ready. Delaying TS init...\n");
			goto userspace_not_ready;
		} else {
			release_firmware(fw_entry);
		}
	}
#endif

	sec_ts_pw_lock(ts, INCELL_DISPLAY_POWER_LOCK);
	mutex_lock(&ts->lock);
	ret = sec_ts_after_init(ts);
	mutex_unlock(&ts->lock);
	sec_ts_pw_lock(ts, INCELL_DISPLAY_POWER_UNLOCK);

	if (ret) {
		input_info(true, &ts->client->dev, "stop session_work\n");
			goto after_work_fail;
	}

	input_info(true, &ts->client->dev, "after_init_work done\n");
	ts->after_work.done = true;

	if (is_first_ts_kickstart) {
		is_first_ts_kickstart = false;
		sec_ts_start_device(ts);
	}

	goto out;

after_work_fail:
	ts->after_work.retry++;
	if (ts->after_work.retry > SEC_TS_RETRY_SESSION_COUNT) {
		input_err(true, &ts->client->dev, "stop after_work\n");
		ts->after_work.err = true;
		goto out;
	}
userspace_not_ready:
	input_info(true, &ts->client->dev, "retry after_work [%d]\n", ts->after_work.retry);
	schedule_delayed_work(&ts->after_work.start, 3 * HZ);
out:
	return;
}

int sec_ts_suspend(struct sec_ts_data *ts)
{
	int ret = 0;

	input_info(true, &ts->client->dev, "%s: start\n", __func__);

	if (ts->power_status == SEC_TS_STATE_POWER_OFF) {
		input_info(true, &ts->client->dev, "%s: already power off\n", __func__);
		goto out;
	}
#ifdef USE_POWER_RESET_WORK
	cancel_delayed_work_sync(&ts->reset_work);
#endif

	if (ts->plat_data->watchdog.supported)
		cancel_delayed_work(&ts->work_watchdog);

	ret = sec_ts_stop_device(ts);
	if (ret)
		input_err(true, &ts->client->dev, "%s: failed sec_ts_stop_device\n", __func__);
out:
	input_info(true, &ts->client->dev, "%s: end\n", __func__);
	return 0;
}

int sec_ts_resume(struct sec_ts_data *ts)
{
	int ret = 0;

	input_info(true, &ts->client->dev, "%s: start\n", __func__);

	if (ts->power_status == SEC_TS_STATE_POWER_ON) {
		input_info(true, &ts->client->dev, "%s: already power on\n", __func__);
		goto out;
	}

	if (ts->plat_data->watchdog.supported)
		schedule_delayed_work(&ts->work_watchdog, msecs_to_jiffies(ts->plat_data->watchdog.delay_ms));

	if (!is_first_ts_kickstart)
		ret = sec_ts_start_device(ts);
	if (ret < 0)
		input_err(true, &ts->client->dev, "%s: failed sec_ts_start_device\n", __func__);
out:
	input_info(true, &ts->client->dev, "%s: end\n", __func__);
	return 0;
}

int sec_ts_set_lowpowermode(struct sec_ts_data *ts, u8 mode)
{
	int ret;
	int retrycnt = 0;
	char para = 0;

	input_info(true, &ts->client->dev, "%s: %s(%X)\n", __func__,
			mode == TO_LOWPOWER_MODE ? "ENTER" : "EXIT", ts->lowpower_mode);

retry_pmode:
	ret = sec_ts_i2c_write(ts, SEC_TS_CMD_SET_POWER_MODE, &mode, 1);
	if (ret < 0) {
		input_err(true, &ts->client->dev, "%s: failed\n", __func__);
		goto i2c_error;
	}

	sec_ts_delay(50);

	ret = sec_ts_i2c_read(ts, SEC_TS_CMD_SET_POWER_MODE, &para, 1);
	if (ret < 0) {
		input_err(true, &ts->client->dev, "%s: read power mode failed!\n", __func__);
		goto i2c_error;
	} else {
		input_info(true, &ts->client->dev, "%s: power mode - write(%d) read(%d)\n", __func__, mode, para);
	}

	if (mode != para) {
		retrycnt++;
		if (retrycnt < 5)
			goto retry_pmode;
	}

	if (mode) {
		ret = sec_ts_i2c_write(ts, SEC_TS_CMD_CLEAR_EVENT_STACK, NULL, 0);
		if (ret < 0) {
			input_err(true, &ts->client->dev, "%s: i2c write clear event failed\n", __func__);
			goto i2c_error;
		}
	}

	sec_ts_locked_release_all_finger(ts);

	if (device_may_wakeup(&ts->client->dev)) {
		if (mode)
			sec_ts_irq_wake(ts, true);
		else
			sec_ts_irq_wake(ts, false);
	}

	if (mode == TO_LOWPOWER_MODE)
		ts->power_status = SEC_TS_STATE_LPM;
	else
		ts->power_status = SEC_TS_STATE_POWER_ON;

i2c_error:
	input_info(true, &ts->client->dev, "%s: end %d\n", __func__, ret);

	return ret;
}

#ifdef USE_OPEN_CLOSE
static int sec_ts_input_open(struct input_dev *dev)
{
	struct sec_ts_data *ts = input_get_drvdata(dev);

#ifdef USE_SELF_TEST_WORK
	if (!ts->info_work_done) {
		input_err(true, &ts->client->dev, "%s not finished info work\n", __func__);
		return 0;
	}
#endif

	mutex_lock(&ts->modechange);

	ts->input_closed = false;

	input_info(true, &ts->client->dev, "%s\n", __func__);

	mutex_unlock(&ts->modechange);
	
	return 0;
}

static void sec_ts_input_close(struct input_dev *dev)
{
	struct sec_ts_data *ts = input_get_drvdata(dev);

#ifdef USE_SELF_TEST_WORK
	if (!ts->info_work_done) {
		input_err(true, &ts->client->dev, "%s not finished info work\n", __func__);
		return;
	}
#endif

	mutex_lock(&ts->modechange);

	ts->input_closed = true;

	input_info(true, &ts->client->dev, "%s\n", __func__);

#ifdef USE_POWER_RESET_WORK
	cancel_delayed_work(&ts->reset_work);
#endif
	sec_ts_stop_device(ts);

	mutex_unlock(&ts->modechange);
}
#endif

static int sec_ts_remove(struct i2c_client *client)
{
	struct sec_ts_data *ts = i2c_get_clientdata(client);

	input_info(true, &ts->client->dev, "%s\n", __func__);

#ifdef USE_SELF_TEST_WORK
	cancel_delayed_work_sync(&ts->work_read_info);
	flush_delayed_work(&ts->work_read_info);
#endif

	sec_ts_set_irq(ts, false);
	sec_ts_free_irq(ts);
	input_info(true, &ts->client->dev, "%s: irq disabled\n", __func__);

#ifdef USE_POWER_RESET_WORK
	cancel_delayed_work_sync(&ts->reset_work);
	flush_delayed_work(&ts->reset_work);

	input_info(true, &ts->client->dev, "%s: flush queue\n", __func__);

#endif

	sec_ts_fn_remove(ts);

#ifdef CONFIG_TOUCHSCREEN_DUMP_MODE
	p_ghost_check = NULL;
#endif
	device_init_wakeup(&client->dev, false);

	ts->lowpower_mode = false;
	ts->probe_done = false;
	ts->after_work.done = false;

	if (ts->plat_data->support_dex) {
		input_mt_destroy_slots(ts->input_dev_pad);
		input_unregister_device(ts->input_dev_pad);
	}

	ts->input_dev = ts->input_dev_touch;
	input_mt_destroy_slots(ts->input_dev);
	input_unregister_device(ts->input_dev);

	input_mt_destroy_slots(ts->input_dev_side);
	input_unregister_device(ts->input_dev_side);
	ts->input_dev_side = NULL;

	mutex_destroy(&ts->lock);
	mutex_destroy(&ts->device_mutex);
	mutex_destroy(&ts->i2c_mutex);
	mutex_destroy(&ts->eventlock);
	mutex_destroy(&ts->modechange);
	mutex_destroy(&ts->irq_mutex);
	mutex_destroy(&ts->aod_mutex);

	ts->input_dev_pad = NULL;
	ts->input_dev = NULL;
	ts->input_dev_touch = NULL;
	if (sec_ts_active_panel)
		drm_panel_notifier_unregister(sec_ts_active_panel, &ts->drm_notif);
	ts_dup = NULL;

	kfree(ts);
	return 0;
}

static void sec_ts_shutdown(struct i2c_client *client)
{
	struct sec_ts_data *ts = i2c_get_clientdata(client);

	input_info(true, &ts->client->dev, "%s\n", __func__);

	sec_ts_remove(client);
}

int sec_ts_stop_device(struct sec_ts_data *ts)
{
#ifndef TOUCH_DRIVER_NOT_SOD_PROXIMITY
	int display_sod_mode;
#endif

	input_info(true, &ts->client->dev, "%s: start\n", __func__);

#ifndef TOUCH_DRIVER_NOT_SOD_PROXIMITY
	if (ts->plat_data->sod_mode.status) {
		display_sod_mode = incell_get_display_sod();
		if ((!ts->cover_set && ts->flip_enable) || display_sod_mode) {
			input_dbg(true, &ts->client->dev, "%s: sod skip - SEC_TS_STATE_POWER_OFF\n", __func__);
			ts->power_status = SEC_TS_STATE_POWER_OFF;
			somc_panel_external_control_touch_power(false);
		} else {
			sec_ts_set_irq(ts, true);
			sec_ts_set_lowpowermode(ts, TO_LOWPOWER_MODE);
			return 0;
		}
	}
#else
	if (ts->plat_data->sod_mode.status) {
		sec_ts_set_irq(ts, true);
		sec_ts_set_lowpowermode(ts, TO_LOWPOWER_MODE);
		return 0;
	}
#endif

	mutex_lock(&ts->device_mutex);

	sec_ts_set_irq(ts, false);

	ts->power_status = SEC_TS_STATE_POWER_OFF;

	sec_ts_locked_release_all_finger(ts);

	if (ts->plat_data->enable_sync)
		ts->plat_data->enable_sync(false);

	input_info(true, &ts->client->dev, "%s: end\n", __func__);
	mutex_unlock(&ts->device_mutex);
	return 0;
}

int sec_ts_start_device(struct sec_ts_data *ts)
{
	int ret, lock_ret = 0;
	u8 cmd_ena = 0x01;
	u8 cmd_dis = 0;

	input_info(true, &ts->client->dev, "%s: start\n", __func__);

	if (ts->plat_data->sod_mode.status && ts->power_status == SEC_TS_STATE_LPM) {
		sec_ts_set_lowpowermode(ts, TO_TOUCH_MODE);
		return 0;
	}

	lock_ret = sec_ts_pw_lock(ts, INCELL_DISPLAY_POWER_LOCK);
	if (lock_ret) {
		input_err(true, &ts->client->dev, "failed lock power\n");
		return lock_ret;
	}

	mutex_lock(&ts->device_mutex);

	if (unlikely(ts->after_work.err)) {
		input_err(true, &ts->client->dev,
			  "%s: failed after_init_work. Call after_init\n",
			  __func__);

		ret = sec_ts_after_init(ts);
		if (ret) {
			input_err(true, &ts->client->dev, "%s: failed after_init\n");
			goto init_err;
		}

		sec_ts_set_lowpowermode(ts, TO_TOUCH_MODE);

		ts->sec_ts_i2c_write(ts, SEC_TS_CMD_GRIP_REJECTION,
				     &cmd_ena, 1);
		ts->sec_ts_i2c_write(ts, SEC_TS_CMD_ENABLE_SIDETOUCH,
				     &cmd_dis, 1);
	}

	sec_ts_locked_release_all_finger(ts);

	sec_ts_delay(70);
	ts->power_status = SEC_TS_STATE_POWER_ON;
	ts->touch_noise_status = 0;

#ifdef USE_POR_AFTER_I2C_RETRY
	if (!ts->reset_is_on_going) {
		ret = sec_ts_wait_for_ready(ts, SEC_TS_ACK_BOOT_COMPLETE);
		if (ret < 0) {
			input_err(true, &ts->client->dev,
					"%s: Failed to wait_for_ready\n", __func__);
			goto err;
		}
	}
#endif

	if (ts->plat_data->enable_sync)
		ts->plat_data->enable_sync(true);

	ret = sec_ts_feature_settings(ts);
	if (ret) {
		input_err(true, &ts->client->dev,
				"%s: Failed to feature settigs\n", __func__);
		goto err;
	}

	sec_ts_set_grip_type(ts, ONLY_EDGE_HANDLER);

	if (ts->dex_mode) {
		input_info(true, &ts->client->dev, "%s: set dex mode\n", __func__);
		ret = ts->sec_ts_i2c_write(ts, SEC_TS_CMD_SET_DEX_MODE, &ts->dex_mode, 1);
		if (ret < 0) {
			input_err(true, &ts->client->dev,
					"%s: failed to set dex mode %x\n", __func__, ts->dex_mode);
			goto err;
		}
	}

	if (ts->brush_mode) {
		input_info(true, &ts->client->dev, "%s: set brush mode\n", __func__);
		ret = ts->sec_ts_i2c_write(ts, SEC_TS_CMD_SET_BRUSH_MODE, &ts->brush_mode, 1);
		if (ret < 0) {
			input_err(true, &ts->client->dev,
					"%s: failed to set brush mode\n", __func__);
			goto err;
		}
	}

	if (ts->touchable_area) {
		input_info(true, &ts->client->dev, "%s: set 16:9 mode\n", __func__);
		ret = ts->sec_ts_i2c_write(ts, SEC_TS_CMD_SET_TOUCHABLE_AREA, &ts->touchable_area, 1);
		if (ret < 0) {
			input_err(true, &ts->client->dev,
					"%s: failed to set 16:9 mode\n", __func__);
			goto err;
		}
	}

	/* Sense_on */
	ret = sec_ts_i2c_write(ts, SEC_TS_CMD_SENSE_ON, NULL, 0);
	if (ret < 0) {
		input_err(true, &ts->client->dev, "%s: fail to write Sense_on\n", __func__);
		goto err;
	}

err:
	sec_ts_set_irq(ts, true);

init_err:
	lock_ret = sec_ts_pw_lock(ts, INCELL_DISPLAY_POWER_UNLOCK);
	if (lock_ret) {
		input_err(true, &ts->client->dev, "failed unlock power\n");
		mutex_unlock(&ts->device_mutex);
		return lock_ret;
	}
	input_info(true, &ts->client->dev, "%s: end\n", __func__);
	mutex_unlock(&ts->device_mutex);
	return ret;
}

#ifdef CONFIG_PM
static int sec_ts_pm_suspend(struct device *dev)
{
	struct sec_ts_data *ts = dev_get_drvdata(dev);

	if (ts->lowpower_mode || ts->plat_data->sod_mode.status)
		reinit_completion(&ts->resume_done);

	return 0;
}

static int sec_ts_pm_resume(struct device *dev)
{
	struct sec_ts_data *ts = dev_get_drvdata(dev);

	if (ts->lowpower_mode || ts->plat_data->sod_mode.status)
		complete_all(&ts->resume_done);

	return 0;
}
#endif

#ifdef CONFIG_TRUSTONIC_TRUSTED_UI
void trustedui_mode_on(void)
{
	if (!tsp_info)
		return;

	sec_ts_unlocked_release_all_finger(tsp_info);
}
EXPORT_SYMBOL(trustedui_mode_on);


void trustedui_mode_off(void)
{
	if (!tsp_info)
		return;
}
EXPORT_SYMBOL(trustedui_mode_off);
#endif

int drm_notifier_callback(struct notifier_block *self, unsigned long event, void *data)
{
	struct drm_panel_notifier *evdata = data;
	struct sec_ts_data *ts = container_of(self, struct sec_ts_data, drm_notif);
	struct timespec time;
	int blank;

	mutex_lock(&ts->aod_mutex);
	if (evdata && evdata->data && ts) {
		if (event == DRM_PANEL_EARLY_EVENT_BLANK) {
			blank = *(int *)evdata->data;
			input_info(true, &ts->client->dev, "Before: %s\n",
				(blank == DRM_PANEL_BLANK_POWERDOWN) ? "Powerdown" :
				(blank == DRM_PANEL_BLANK_UNBLANK) ? "Unblank" :
				 "???");
			switch (blank) {
			case DRM_PANEL_BLANK_POWERDOWN:
				if (!ts->after_work.done) {
					input_info(true, &ts->client->dev, "not already sleep out\n");
					mutex_unlock(&ts->aod_mutex);
					return 0;
				}

				get_monotonic_boottime(&time);
				input_info(true, &ts->client->dev, "start@%ld.%06ld\n",
					time.tv_sec, time.tv_nsec);
				sec_ts_suspend(ts);
				get_monotonic_boottime(&time);
				input_info(true, &ts->client->dev, "end@%ld.%06ld\n",
					time.tv_sec, time.tv_nsec);
				break;
			case DRM_PANEL_BLANK_UNBLANK:
				break;
			default:
				break;
			}
		} else if (event == DRM_PANEL_EVENT_BLANK) {
			blank = *(int *)evdata->data;
			input_info(true, &ts->client->dev, "After: %s\n",
				(blank == DRM_PANEL_BLANK_POWERDOWN) ? "Powerdown" :
				(blank == DRM_PANEL_BLANK_UNBLANK) ? "Unblank" :
				 "???");
			switch (blank) {
			case DRM_PANEL_BLANK_POWERDOWN:
				break;
			case DRM_PANEL_BLANK_UNBLANK:
				if (!ts->after_work.done && !ts->after_work.err) {
					input_info(true, &ts->client->dev, "not already sleep out\n");
					mutex_unlock(&ts->aod_mutex);
					return 0;
				}
				if (is_first_ts_kickstart) {
					input_info(true, &ts->client->dev,
						   "First TS kickstart, wait "
						   "for worker sleep-out\n");
					mutex_unlock(&ts->aod_mutex);
					return 0;
				}

				if (ts->after_work.done || ts->after_work.err) {
					get_monotonic_boottime(&time);
					input_info(true, &ts->client->dev, "start@%ld.%06ld\n",
						time.tv_sec, time.tv_nsec);

					sec_ts_resume(ts);
					get_monotonic_boottime(&time);
					input_info(true, &ts->client->dev, "end@%ld.%06ld\n",
						time.tv_sec, time.tv_nsec);
				}
				break;
			default:
				break;
			}

			if (ts->aod_pending) {
				input_info(true, &ts->client->dev,
					"Applying aod_pending_lowpower_mode: %d\n",
					ts->aod_pending_lowpower_mode);
				ts->aod_pending = false;
				ts->lowpower_mode = ts->aod_pending_lowpower_mode;
				sec_ts_set_lowpowermode(ts, ts->lowpower_mode);
			}
		}
	}
	mutex_unlock(&ts->aod_mutex);
	return 0;
}

static const struct i2c_device_id sec_ts_id[] = {
	{ SEC_TS_I2C_NAME, 0 },
	{ },
};

#ifdef CONFIG_PM
static const struct dev_pm_ops sec_ts_dev_pm_ops = {
	.suspend = sec_ts_pm_suspend,
	.resume = sec_ts_pm_resume,
};
#endif

#ifdef CONFIG_OF
static const struct of_device_id sec_ts_match_table[] = {
	{ .compatible = "sec,sec_ts",},
	{ },
};
#else
#define sec_ts_match_table NULL
#endif

static struct i2c_driver sec_ts_driver = {
	.probe		= sec_ts_probe,
	.remove		= sec_ts_remove,
	.shutdown	= sec_ts_shutdown,
	.id_table	= sec_ts_id,
	.driver = {
		.owner	= THIS_MODULE,
		.name	= SEC_TS_I2C_NAME,
#ifdef CONFIG_OF
		.of_match_table = sec_ts_match_table,
#endif
#ifdef CONFIG_PM
		.pm = &sec_ts_dev_pm_ops,
#endif
	},
};

static int __init sec_ts_init(void)
{
#ifdef CONFIG_BATTERY_SAMSUNG
	if (lpcharge == 1) {
		pr_err("%s %s: Do not load driver due to : lpm %d\n",
				SECLOG, __func__, lpcharge);
		return -ENODEV;
	}
#endif
	pr_info("%s %s\n", SECLOG, __func__);

	return i2c_add_driver(&sec_ts_driver);
}

static void __exit sec_ts_exit(void)
{
	i2c_del_driver(&sec_ts_driver);
}

MODULE_AUTHOR("Hyobae, Ahn<hyobae.ahn@samsung.com>");
MODULE_DESCRIPTION("Samsung Electronics TouchScreen driver");
MODULE_LICENSE("GPL");

late_initcall(sec_ts_init);
module_exit(sec_ts_exit);
