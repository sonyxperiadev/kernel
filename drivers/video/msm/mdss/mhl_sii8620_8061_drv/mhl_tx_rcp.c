/* kernel/drivers/video/msm/mdss/mhl_sii8620_8061_drv/mhl_tx_rcp.c
 *
 * Copyright (C) 2013 Sony Mobile Communications AB.
 * Copyright (C) 2013 Silicon Image Inc.
 *
 * Author: [Karino, Masaharu XA <masaharu.xa.karino@sonymobile.com>]
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/vmalloc.h>
#include <linux/input.h>
#include <linux/usb/msm_hsusb.h>

#include "mhl_common.h"
#include "mhl_tx.h"

#define RCP_KEY_RELEASE_TIME1	450
#define RCP_KEY_RELEASE_TIME2	200
#define RCP_KEY_INVALID			-1

static int mouse_mode;
static int mouse_move_distance_dx;
static int mouse_move_distance_dy;

/* supported RCP key code */
u16 support_key_code_tbl[] = {
	KEY_ENTER,		/* 0x00 Select */
	KEY_UP,			/* 0x01 Up */
	KEY_DOWN,		/* 0x02 Down */
	KEY_LEFT,		/* 0x03 Left */
	KEY_RIGHT,		/* 0x04 Right */
	KEY_UNKNOWN,		/* 0x05 Right-up */
	KEY_UNKNOWN,		/* 0x06 Right-down */
	KEY_UNKNOWN,		/* 0x07 Left-up */
	KEY_UNKNOWN,		/* 0x08 Left-down */
	KEY_MENU,		/* 0x09 Root Menu */
	KEY_OPTION,		/* 0x0A Setup Menu */
	KEY_UNKNOWN,		/* 0x0B Contents Menu */
	KEY_UNKNOWN,		/* 0x0C Favorite Menu */
	KEY_EXIT,		/* 0x0D Exit */
	KEY_RESERVED,		/* 0x0E */
	KEY_RESERVED,
	KEY_RESERVED,
	KEY_RESERVED,
	KEY_RESERVED,
	KEY_RESERVED,
	KEY_RESERVED,
	KEY_RESERVED,
	KEY_RESERVED,
	KEY_RESERVED,
	KEY_RESERVED,
	KEY_RESERVED,
	KEY_RESERVED,
	KEY_RESERVED,
	KEY_RESERVED,
	KEY_RESERVED,
	KEY_RESERVED,
	KEY_RESERVED,		/* 0x1F */
	KEY_NUMERIC_0,		/* 0x20 NUMERIC_0 */
	KEY_NUMERIC_1,		/* 0x21 NUMERIC_1 */
	KEY_NUMERIC_2,		/* 0x22 NUMERIC_2 */
	KEY_NUMERIC_3,		/* 0x23 NUMERIC_3 */
	KEY_NUMERIC_4,		/* 0x24 NUMERIC_4 */
	KEY_NUMERIC_5,		/* 0x25 NUMERIC_5 */
	KEY_NUMERIC_6,		/* 0x26 NUMERIC_6 */
	KEY_NUMERIC_7,		/* 0x27 NUMERIC_7 */
	KEY_NUMERIC_8,		/* 0x28 NUMERIC_8 */
	KEY_NUMERIC_9,		/* 0x29 NUMERIC_9 */
	KEY_DOT,		/* 0x2A Dot */
	KEY_ENTER,		/* 0x2B Enter */
	KEY_ESC,		/* 0x2C Clear */
	KEY_RESERVED,		/* 0x2D */
	KEY_RESERVED,		/* 0x2E */
	KEY_RESERVED,		/* 0x2F */
	KEY_UNKNOWN,		/* 0x30 Channel Up */
	KEY_UNKNOWN,		/* 0x31 Channel Down */
	KEY_UNKNOWN,		/* 0x32 Previous Channel */
	KEY_UNKNOWN,		/* 0x33 Sound Select */
	KEY_UNKNOWN,		/* 0x34 Input Select */
	KEY_UNKNOWN,		/* 0x35 Show Information */
	KEY_UNKNOWN,		/* 0x36 Help */
	KEY_UNKNOWN,		/* 0x37 Page Up */
	KEY_UNKNOWN,		/* 0x38 Page Down */
	KEY_RESERVED,		/* 0x39 */
	KEY_RESERVED,
	KEY_RESERVED,
	KEY_RESERVED,
	KEY_RESERVED,
	KEY_RESERVED,
	KEY_RESERVED,		/* 0x3F */
	KEY_RESERVED,		/* 0x40 */
	KEY_VOLUMEUP,		/* 0x41 Volume Up */
	KEY_VOLUMEDOWN,		/* 0x42 Volume Down */
	KEY_MUTE,		/* 0x43 Mute */
	KEY_PLAY,		/* 0x44 Play */
	KEY_STOP,		/* 0x45 Stop */
	KEY_PAUSE,		/* 0x46 Pause */
	KEY_UNKNOWN,		/* 0x47 Record */
	KEY_REWIND,		/* 0x48 Rewind */
	KEY_FASTFORWARD,	/* 0x49 Fast Forward */
	KEY_UNKNOWN,		/* 0x4A Eject */
	KEY_FORWARD,		/* 0x4B Forward */
	KEY_BACK,		/* 0x4C Backward */
	KEY_RESERVED,		/* 0x4D */
	KEY_RESERVED,
	KEY_RESERVED,		/* 0x4F */
	KEY_UNKNOWN,		/* 0x50 Angle */
	KEY_UNKNOWN,		/* 0x51 Subtitle */
	KEY_RESERVED,		/* 0x52 */
	KEY_RESERVED,
	KEY_RESERVED,
	KEY_RESERVED,
	KEY_RESERVED,
	KEY_RESERVED,
	KEY_RESERVED,
	KEY_RESERVED,
	KEY_RESERVED,
	KEY_RESERVED,
	KEY_RESERVED,
	KEY_RESERVED,
	KEY_RESERVED,
	KEY_RESERVED,		/* 0x5F */
	KEY_PLAYPAUSE,		/* 0x60 Play Function */
	KEY_PLAYPAUSE,		/* 0x61 Pause_Play Function */
	KEY_UNKNOWN,		/* 0x62 Record Function */
	KEY_PAUSE,		/* 0x63 Pause Record Function */
	KEY_STOP,		/* 0x64 Stop Function  */
	KEY_MUTE,		/* 0x65 Mute Function */
	KEY_UNKNOWN,		/* 0x66 Restore Volume Function */
	KEY_UNKNOWN,		/* 0x67 Tune Function */
	KEY_UNKNOWN,		/* 0x68 Select Media Function */
	KEY_RESERVED,		/* 0x69 */
	KEY_RESERVED,
	KEY_RESERVED,
	KEY_RESERVED,
	KEY_RESERVED,
	KEY_RESERVED,
	KEY_RESERVED,
	KEY_RESERVED,		/* 0x70 */
	KEY_BLUE,			/* 0x71 F1 */
	KEY_RED,			/* 0x72 F2 */
	KEY_GREEN,			/* 0x73 F3 */
	KEY_YELLOW,			/* 0x74 F4 */
	KEY_UNKNOWN,		/* 0x75 F5 */
	KEY_RESERVED,		/* 0x76 */
	KEY_RESERVED,
	KEY_RESERVED,
	KEY_RESERVED,
	KEY_RESERVED,
	KEY_RESERVED,
	KEY_RESERVED,
	KEY_RESERVED,		/* 0x7D */
	KEY_VENDOR,		/* Vendor Specific */
	KEY_RESERVED,		/* 0x7F */
};

static DEFINE_MUTEX(rcp_key_release_mutex);
static struct workqueue_struct *rcp_key_release_workqueue;
static struct timer_list key_release_timer;

struct rcp_info {
	bool bReady;
	int key_release;
	int pre_input_key;
	u16 *key_code_tbl;
	size_t key_code_tbl_len;

	struct input_dev *input;
	struct work_struct key_release_work;
	struct device dev;
};

static struct rcp_info s_rcp_info = {0};

static ssize_t mhl_tx_rcp_mouse_mode(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	sscanf(buf, "%d", &mouse_mode);
	pr_debug("%s: mouse_mode = %d\n", __func__, mouse_mode);
	return count;
}

static ssize_t mhl_tx_rcp_mouse_move_distance_dx(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	sscanf(buf, "%d", &mouse_move_distance_dx);
	pr_debug("%s: mouse_move_distance_dx = %d\n",
		 __func__, mouse_move_distance_dx);
	return count;
}

static ssize_t mhl_tx_rcp_mouse_move_distance_dy(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	sscanf(buf, "%d", &mouse_move_distance_dy);
	pr_debug("%s: mouse_move_distance_dy = %d\n",
		 __func__, mouse_move_distance_dy);
	return count;
}

static void mhl_tx_rcp_key_release_timer(unsigned long data)
{
	struct rcp_info *prcpinfo = (struct rcp_info *)data;
	if (!prcpinfo)
		return;

	if (prcpinfo->bReady)
		queue_work(rcp_key_release_workqueue,
				&prcpinfo->key_release_work);
}

static void mhl_tx_rcp_key_release_work(struct work_struct *work)
{
	struct rcp_info *prcpinfo;
	prcpinfo = container_of(work, struct rcp_info, key_release_work);

	if (!prcpinfo)
		return;

	mutex_lock(&rcp_key_release_mutex);
	if (prcpinfo->key_release != true) {
		mutex_unlock(&rcp_key_release_mutex);
		return;
	}
	if (prcpinfo->pre_input_key == RCP_KEY_INVALID) {
		mutex_unlock(&rcp_key_release_mutex);
		return;
	}

	input_report_key(prcpinfo->input, prcpinfo->pre_input_key, 0);
	input_sync(prcpinfo->input);
	prcpinfo->key_release = false;
	prcpinfo->pre_input_key = RCP_KEY_INVALID;
	mutex_unlock(&rcp_key_release_mutex);
}

static void mhl_tx_rcp_key_input(u8 key_code, u16 input_key_code)
{
	int axis = REL_X, distance = -1;
	bool mouse_event = false;
	int key_press = (key_code & 0x80) == 0;

	pr_debug("%s: send key events[%x][%x][%d]\n",
			__func__, key_code, input_key_code, key_press);

	if (mouse_mode) {
		switch (input_key_code) {
		case KEY_UP:
			axis = REL_Y;
			distance = -mouse_move_distance_dy;
			mouse_event = true;
			break;
		case KEY_DOWN:
			axis = REL_Y;
			distance = mouse_move_distance_dy;
			mouse_event = true;
			break;
		case KEY_LEFT:
			axis = REL_X;
			distance = -mouse_move_distance_dx;
			mouse_event = true;
			break;
		case KEY_RIGHT:
			axis = REL_X;
			distance = mouse_move_distance_dx;
			mouse_event = true;
			break;
		case KEY_ENTER:
			axis = -1;
			distance = 0;
			mouse_event = true;
			break;
		case KEY_BLUE:
			input_key_code = KEY_EXIT;
			break;
		default:
			break;
		}
	}

	if (mouse_event) {
		if (axis >= 0 && key_press) {
			input_report_rel(s_rcp_info.input, axis, distance);
			input_sync(s_rcp_info.input);
			return;
		} else {
			input_key_code = BTN_LEFT;
		}
	}

	if (key_press) {
		mutex_lock(&rcp_key_release_mutex);
		if (s_rcp_info.pre_input_key != input_key_code &&
				s_rcp_info.key_release) {
			/* Release previous press key code if current press key
			 * differs from previous press key.
			 */
			input_report_key(s_rcp_info.input,
					s_rcp_info.pre_input_key, 0);
			input_sync(s_rcp_info.input);
		}
		s_rcp_info.key_release = true;
		s_rcp_info.pre_input_key = input_key_code;
		mutex_unlock(&rcp_key_release_mutex);
		/* rcp key release timer start */
		switch (input_key_code) {
		case KEY_UP:
		case KEY_DOWN:
		case KEY_LEFT:
		case KEY_RIGHT:
		case BTN_LEFT:
			mod_timer(&key_release_timer,
				jiffies +
				msecs_to_jiffies(RCP_KEY_RELEASE_TIME2));
			break;
		default:
			mod_timer(&key_release_timer,
				jiffies +
				msecs_to_jiffies(RCP_KEY_RELEASE_TIME1));
			break;
		}
	} else {
		mutex_lock(&rcp_key_release_mutex);
		s_rcp_info.key_release = false;
		s_rcp_info.pre_input_key = RCP_KEY_INVALID;
		mutex_unlock(&rcp_key_release_mutex);
	}

	pr_debug("%s: send key events[%x][%x][%d]\n",
		 __func__, key_code, input_key_code, key_press);
	mutex_lock(&rcp_key_release_mutex);
	input_report_key(s_rcp_info.input, input_key_code, key_press);
	input_sync(s_rcp_info.input);
	mutex_unlock(&rcp_key_release_mutex);
}

static void mhl_tx_rcp_common_resource_free(void)
{
	s_rcp_info.bReady = false;
	del_timer_sync(&key_release_timer);
	destroy_workqueue(rcp_key_release_workqueue);
}
/* ----------------------- global functions -----------------------*/
static DEVICE_ATTR(mouse_move_distance_dx, 0660,
				NULL, mhl_tx_rcp_mouse_move_distance_dx);
static DEVICE_ATTR(mouse_move_distance_dy, 0660,
				NULL, mhl_tx_rcp_mouse_move_distance_dy);
static DEVICE_ATTR(mouse_mode, 0660,
				NULL, mhl_tx_rcp_mouse_mode);

int mhl_tx_rcp_init(struct device *parent)
{
	int rc;
	struct class *cls = parent->class;

	if (IS_ERR(cls)) {
		pr_debug("%s: failed to create class", __func__);
		return -ENOMEM;
	}

	s_rcp_info.dev.class = cls;
	s_rcp_info.dev.parent = parent;
	dev_set_name(&s_rcp_info.dev, "rcp");

	rc = device_register(&s_rcp_info.dev);
	if (rc) {
		pr_err("%s: failed to register device", __func__);
		return rc;
	}

	rc = device_create_file(&s_rcp_info.dev,
				 &dev_attr_mouse_move_distance_dx);
	if (rc) {
		pr_err("%s: failed to create file for mouse dx\n", __func__);
		return rc;
	}

	rc = device_create_file(&s_rcp_info.dev,
				 &dev_attr_mouse_move_distance_dy);
	if (rc) {
		pr_err("%s: failed to create file for mouse dy\n", __func__);
		return rc;
	}

	rc = device_create_file(&s_rcp_info.dev,
				 &dev_attr_mouse_mode);
	if (rc) {
		pr_err("%s: failed to create file for mouse mode\n", __func__);
		return rc;
	}
	return 0;
}

void mhl_tx_rcp_release(void)
{
	mhl_tx_rcp_stop();

	device_remove_file(&s_rcp_info.dev, &dev_attr_mouse_move_distance_dx);
	device_remove_file(&s_rcp_info.dev, &dev_attr_mouse_move_distance_dy);
	device_remove_file(&s_rcp_info.dev, &dev_attr_mouse_mode);
	device_unregister(&s_rcp_info.dev);
}

int mhl_tx_rcp_start(void)
{
	if (!s_rcp_info.input) {
		pr_debug("%s: initial input device for RCP\n", __func__);
		/* key status */
		s_rcp_info.key_release = false;
		s_rcp_info.pre_input_key = RCP_KEY_INVALID;

		/* work queue */
		rcp_key_release_workqueue =
			create_singlethread_workqueue("mhl_rcp_key_release");
		INIT_WORK(&s_rcp_info.key_release_work,
				mhl_tx_rcp_key_release_work);

		/*--- timer initialize ---*/
		init_timer(&key_release_timer);
		key_release_timer.function = mhl_tx_rcp_key_release_timer;
		key_release_timer.data = (unsigned long)&s_rcp_info;
		key_release_timer.expires = jiffies - HZ;
		add_timer(&key_release_timer);
		s_rcp_info.bReady = true;
		s_rcp_info.input = input_allocate_device();

		/*--- input structure ---*/
		if (s_rcp_info.input) {
			int i;
			int err = -EINVAL;

			s_rcp_info.key_code_tbl = vmalloc(
				sizeof(support_key_code_tbl));
			if (!s_rcp_info.key_code_tbl) {
				pr_err("%s: no alloc mem for rcp keycode tbl\n",
					   __func__);
				mhl_tx_rcp_common_resource_free();
				input_free_device(s_rcp_info.input);
				s_rcp_info.input = NULL;
				return -ENOMEM;
			}

			s_rcp_info.key_code_tbl_len =
					sizeof(support_key_code_tbl);
			memcpy(s_rcp_info.key_code_tbl,
				   &support_key_code_tbl[0],
				   s_rcp_info.key_code_tbl_len);

			s_rcp_info.input->phys = "cbus/input0";
			s_rcp_info.input->id.bustype = BUS_VIRTUAL;
			s_rcp_info.input->id.vendor  = 0x1095;
			s_rcp_info.input->id.product = 0x8620;
			s_rcp_info.input->id.version = 0xA;

			s_rcp_info.input->name = "mhl-rcp";

			s_rcp_info.input->keycode = support_key_code_tbl;
			s_rcp_info.input->keycodesize = sizeof(u16);
			s_rcp_info.input->keycodemax =
					ARRAY_SIZE(support_key_code_tbl);

			s_rcp_info.input->evbit[0] = EV_KEY | EV_REP | EV_REL;
			input_set_capability(s_rcp_info.input, EV_REL, REL_X);
			input_set_capability(s_rcp_info.input, EV_REL, REL_Y);
			input_set_capability(s_rcp_info.input, EV_KEY,
					BTN_LEFT);

			for (i = 0; i < ARRAY_SIZE(support_key_code_tbl); i++) {
				if (support_key_code_tbl[i] > 1)
					input_set_capability(s_rcp_info.input,
						EV_KEY,
						support_key_code_tbl[i]);
			}

			err = input_register_device(s_rcp_info.input);
			if (err < 0) {
				pr_err("%s: failed to register input device\n"
						, __func__);
				vfree(s_rcp_info.key_code_tbl);
				mhl_tx_rcp_common_resource_free();
				input_free_device(s_rcp_info.input);
				s_rcp_info.input = NULL;
				return err;
			}
		} else {
			mhl_tx_rcp_common_resource_free();
			return -ENOMEM;
		}
	}
	return 0;
}

void mhl_tx_rcp_stop(void)
{
	mutex_lock(&rcp_key_release_mutex);

	if (s_rcp_info.input) {
		vfree(s_rcp_info.key_code_tbl);
		mhl_tx_rcp_common_resource_free();
		input_unregister_device(s_rcp_info.input);
		input_free_device(s_rcp_info.input);
		s_rcp_info.input = NULL;
	}

	mutex_unlock(&rcp_key_release_mutex);
}

u16 mhl_tx_rcp_IsSupported(u8 key_code)
{
	u8 index = key_code & 0x7f;
	u16 input_key_code;

	if (!s_rcp_info.key_code_tbl) {
		pr_err("%s: RCP Key Code Table not initialized\n", __func__);
		return 0;
	}

	input_key_code = s_rcp_info.key_code_tbl[index];

	if ((index < s_rcp_info.key_code_tbl_len) &&
	    (input_key_code > 0)) {
		return input_key_code;
	} else {
		return 0;
	}
}

int mhl_tx_rcp_input(u8 key_code)
{
	u16 input_key_code;

	input_key_code = mhl_tx_rcp_IsSupported(key_code);
	if (input_key_code) {
		mhl_tx_rcp_key_input(key_code, input_key_code);
		return 0;
	} else {
		return -EINVAL;
	}
}

