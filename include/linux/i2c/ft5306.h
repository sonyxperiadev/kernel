/*****************************************************************************
* Copyright 2009-2010 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL"). 
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/

#ifndef _FT5306_H_
#define _FT5306_H_
#include <linux/i2c-kona.h>
#include <linux/semaphore.h>
#include <linux/mutex.h>
#include <linux/i2c.h>
#include <linux/notifier.h>

#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif

#undef	I2C_TS_DRIVER_NAME
#define I2C_TS_DRIVER_NAME			"FocalTech-Ft5306"

#define TP_CNTRL_PIN_WAKEUP	1
#define TP_CNTRL_PIN_RESET	0
#define TP_CNTRL_PIN_TYPE 	TP_CNTRL_PIN_WAKEUP

#define ENABLE_TP_DIAG 0

typedef enum {
	TS_OFF,
	TS_ON,
} ts_power_status;

struct synaptics_rmi4 {
	struct i2c_client *client;
	struct input_dev *input_dev;
	int use_irq;
	struct hrtimer timer;
	struct work_struct  work;
	struct work_struct firmware_update_work;
#if ENABLE_TP_DIAG
	struct hrtimer diag_timer;
	struct work_struct diag_work;
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend early_suspend;
#endif
	__u8 data_reg;
	__u8 data_length;
	__u8 *data;
	struct i2c_msg data_i2c_msg[2];
	bool hasEgrPinch;
	bool hasEgrPress;
	bool hasEgrFlick;
	bool hasEgrEarlyTap;
	bool hasEgrDoubleTap;
	bool hasEgrTapAndHold;
	bool hasEgrSingleTap;
	bool hasEgrPalmDetect;
	int enable;
	int(*power)(ts_power_status vreg_en);
};

struct Synaptics_ts_platform_data {
    int gpio_irq_pin;
	int gpio_reset_pin;
	int gpio_wakeup_pin;
	int x_max_value;
	int y_max_value;
	int(*power)(ts_power_status vreg_en);
};

int register_touch_key_notifier(struct notifier_block *n);
int unregister_touch_key_notifier(struct notifier_block *n);

#endif /* _FT5306_H_ */
