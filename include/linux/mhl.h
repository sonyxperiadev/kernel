/*
 * Copyright (C) 2011 Sony Ericsson Mobile Communications AB.
 * Copyright (C) 2012-2013 Sony Mobile Communications AB.
 * Copyright (C) 2011 Silicon Image Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef __LINUX_MHL_H__
#define __LINUX_MHL_H__

#ifdef __KERNEL__

#include <linux/device.h>
#include <linux/input.h>
#include <linux/mutex.h>
#include <linux/mhl_defs.h>
#include <linux/earlysuspend.h>

#define TRUE 1
#define FALSE 0

enum discovery_result_enum {
	MHL_DISCOVERY_RESULT_USB = 0,
	MHL_DISCOVERY_RESULT_MHL,
};

struct msc_command_struct {
	u8 command;
	u8 offset;
	u8 length;
	union {
		u8 data[16];
		u8 *burst_data;
	} payload;
	u8 retval;
};

struct mhl_ops {
	int (*discovery_result_get)(int *result);
	int (*send_msc_command)(struct msc_command_struct *req);
	int (*charging_control)(int enable, int max_curr);
	int (*hpd_control)(int enable);
	int (*tmds_control)(int enable);
#ifdef CONFIG_MHL_OSD_NAME
	int (*scratchpad_data_get)(void);
#endif /* CONFIG_MHL_OSD_NAME */
};

struct mhl_state {
	u8 device_status[2];
	u8 peer_status[2];
	u8 peer_devcap[16];
};

#define MHL_OFFLINE			(0)
#define MHL_PLUGGED			BIT(0)
#define MHL_LINK_ESTABLISHED		BIT(1)
#define MHL_PEER_DCAP_READ		BIT(2)
#define MHL_ONLINE			(BIT(0) | BIT(1) | BIT(2))

#define MHL_DEVCAP_READ_DONE(___st)	(((___st) & 0xffff) == 0xffff)

#define MSC_COMMAND_QUEUE_SIZE 64
#define MSC_NORMAL_SEND	0
#define MSC_PRIOR_SEND	1

struct mhl_event {
	struct list_head msc_queue;
	struct msc_command_struct msc_command_queue;
};

#ifdef CONFIG_MHL_RAP
struct mhl_rap_event {
	struct list_head rap_queue;
	struct msc_command_struct rap_command_queue;
};
#endif

#ifdef CONFIG_MHL_OSD_NAME
#define MHL_ADOPTER_ID_SOMC	0x03A7
#define MHL_SCPD_GIVE_OSD_NAME	0xA1
#define MHL_SCPD_SET_OSD_NAME	0xA2
#define MHL_OSD_NAME_SIZE	12
struct mhl_osd_msg {
	u16 adopter_id;
	u8 command_id;
	u8 name[MHL_OSD_NAME_SIZE];
	u8 decrement_cnt;
} __packed;
#endif /* CONFIG_MHL_OSD_NAME */

struct mhl_device {
	struct device dev;

	struct mutex ops_mutex;
	const struct mhl_ops *ops;

	int full_operation;
	int suspended;

	struct mhl_state state;
	unsigned int mhl_online;
	unsigned int hpd_state;
	unsigned int tmds_state;
	u16 devcap_state;

	struct work_struct msc_command_work;
	int msc_command_counter;
	struct list_head msc_queue;

#ifdef CONFIG_MHL_RAP
	/* RAP send control */
	int rap_sending;
	int rap_action_code_retry;
	int rap_send_retry_num;
	struct timer_list rap_send_timer;
	struct work_struct rap_command_work;
	struct timer_list rap_retry_timer;
	struct work_struct rap_retry_work;
	int rap_command_counter;
	struct list_head rap_queue;
#endif

#ifdef CONFIG_MHL_OSD_NAME
	/* Scratchpad send control */
	struct mhl_osd_msg scratchpad_send_data;
	struct work_struct scratchpad_work;
#endif

	/* MSC WRITE BURST */
	struct completion req_write_done;
	int write_burst_requested;

	/* USB interface */
	struct work_struct usb_online_work;

	int key_release_supported;
	int mouse_enabled;
	int mouse_speed;
	struct input_dev *input;

	struct early_suspend early_suspend;
};

/********************************
 * MHL event
 ********************************/

extern int mhl_notify_plugged(struct mhl_device *dev);
extern int mhl_notify_unplugged(struct mhl_device *dev);

extern int mhl_notify_online(struct mhl_device *dev);
extern int mhl_notify_offline(struct mhl_device *dev);

extern int mhl_notify_hpd(struct mhl_device *dev, int state);

#ifdef CONFIG_MHL_OSD_NAME
extern int mhl_notify_scpd_recv(struct mhl_device *mhl_dev, const char *buf);
#endif /* CONFIG_MHL_OSD_NAME */

/********************************
 * MSC command
 ********************************/

/*
 * chip driver must call this function with retval
 * after send_msc_command successfully done.
 */
extern int mhl_msc_command_done(struct mhl_device *dev,
	struct msc_command_struct *req);

/*
 * MSC: 0x60 WRITE_STAT
 */
extern int mhl_msc_send_write_stat(struct mhl_device *dev,
	u8 offset, u8 value);
extern int mhl_msc_recv_write_stat(struct mhl_device *dev,
	u8 offset, u8 value);

/*
 * MSC: 0x60 SET_INT
 */
extern int mhl_msc_send_set_int(struct mhl_device *dev, u8 offset, u8 mask);
extern int mhl_msc_recv_set_int(struct mhl_device *dev, u8 offset, u8 mask);

/*
 * MSC: 0x61 READ_DEVCAP
 */
extern int mhl_msc_read_devcap(struct mhl_device *dev, u8 offset);
extern int mhl_msc_read_devcap_all(struct mhl_device *mhl_dev);

/*
 * MSC: 0x68 MSC_MSG
 */
extern int mhl_msc_recv_msc_msg(struct mhl_device *mhl_dev,
	u8 sub_cmd, u8 cmd_data);
extern int mhl_msc_send_msc_msg(struct mhl_device *mhl_dev,
	u8 sub_cmd, u8 cmd_data);

/*
 * MSC: 0x6C WRITE_BURST
 */
extern int mhl_msc_request_write_burst(struct mhl_device *mhl_dev,
	u8 offset, u8 *data, u8 length);

/********************************
 * USB driver interface
 ********************************/

/*
 * mhl_device_discovery
 * - get whether MHL or USB connected.
 *   caller may be blocked during device discovery.
 */
extern int mhl_device_discovery(const char *name, int *result);

/*
 * mhl_register_callback
 * - register|unregister MHL cable plug callback.
 */
extern int mhl_register_callback
	(const char *name, void (*callback)(int online));
extern int mhl_unregister_callback(const char *name);

/********************************
 * HDMI driver interface
 ********************************/

extern int mhl_full_operation(const char *name, int enable);

/********************************
 * MHL class driver
 ********************************/

#define MHL_DEVICE_NAME_MAX 256
#define to_mhl_device(obj) container_of(obj, struct mhl_device, dev)

extern struct mhl_device *mhl_device_register(const char *name,
	struct device *parent, void *devdata, const struct mhl_ops *ops);
extern void mhl_device_unregister(struct mhl_device *mhl_dev);
extern void mhl_device_shutdown(struct mhl_device *mhl_dev);


#endif /* __KERNEL__ */

#endif /* __LINUX_MHL_H__ */
