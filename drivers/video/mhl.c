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

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/vmalloc.h>

#include <linux/mhl.h>
#include <linux/mhl_defs.h>

#ifdef CONFIG_MHL_OSD_NAME
#include "msm/mdp.h"
#include "msm/external_common.h"
#endif /* CONFIG_MHL_OSD_NAME */

#define PRINT_DEVCAP

#ifdef CONFIG_MHL_RAP
/* RAPK_WAIT(TRAP_WAIT) 1000ms */
#define RAPK_WAIT_TIME		(jiffies + HZ)
#define RAPK_RETRY_TIME		(jiffies + HZ/2)
#define RAP_SEND_RETRY_MAX	2
#endif /* CONFIG_MHL_RAP */

#define MHL_KEYCODE_OFFSET	0x40
#define MOUSE_MOVE_DISTANCE	25
/*
 * mhl.c - MHL control abustruction provides following feature
 * - Userspace interface
 * - USB interface
 * - Common control for MSC commands
 * - Common control for RCP/RAP
 */

static struct class *mhl_class;

static DEFINE_MUTEX(msc_command_queue_mutex);
struct workqueue_struct *msc_command_workqueue;

#ifdef CONFIG_MHL_RAP
static DEFINE_MUTEX(rap_command_queue_mutex);
struct workqueue_struct *rap_command_workqueue;
static int mhl_rap_send_msc_msg(struct mhl_device *mhl_dev, u8 sub_cmd,
	u8 cmd_data);
#endif /* CONFIG_MHL_RAP */

static DEFINE_MUTEX(usb_online_mutex);
struct workqueue_struct *usb_online_workqueue;

#ifdef CONFIG_MHL_OSD_NAME
static struct workqueue_struct *scratchpad_workqueue;
#endif /* CONFIG_MHL_OSD_NAME */

static void (*notify_usb_online)(int online);

static int mhl_update_peer_devcap(struct mhl_device *mhl_dev,
	int offset, u8 devcap);

static int __mhl_match(struct device *dev, void *data)
{
	char *name = (char *) data;
	if (strncmp(dev_name(dev), name, MHL_DEVICE_NAME_MAX) == 0)
		return 1;
	return 0;
}

static int atoi(const char *name)
{
	int val = 0;

	for (;; name++) {
		switch (*name) {
		case '0' ... '9':
			val = 10*val+(*name-'0');
			break;
		default:
			return val;
		}
	}
}

static struct mhl_device *mhl_get_dev(const char *name)
{
	struct device *dev;

	if (!name)
		return NULL;

	/* lookup mhl device by name */
	dev = class_find_device(mhl_class, NULL, (void *) name, __mhl_match);
	if (!dev) {
		pr_err("%s: mhl device (%s) not registered!\n",
			__func__, name);
		return NULL;
	}
	return to_mhl_device(dev);
}

/********************************
 * MHL event
 ********************************/

int mhl_notify_plugged(struct mhl_device *mhl_dev)
{
	if (!mhl_dev)
		return -EFAULT;

	mhl_dev->mhl_online |= MHL_PLUGGED;

	/* callback usb driver if callback registered */
	queue_work(usb_online_workqueue, &mhl_dev->usb_online_work);

	return 0;
}
EXPORT_SYMBOL(mhl_notify_plugged);

int mhl_notify_unplugged(struct mhl_device *mhl_dev)
{
	if (!mhl_dev)
		return -EFAULT;

	mhl_dev->mhl_online = 0;
	mhl_dev->hpd_state = 0;
	mhl_dev->tmds_state = FALSE;
	mhl_dev->devcap_state = 0;
	mhl_dev->key_release_supported = 0;
	memset(&mhl_dev->state, 0, sizeof(struct mhl_state));

	/* callback usb driver if callback registered */
	queue_work(usb_online_workqueue, &mhl_dev->usb_online_work);

	return 0;
}
EXPORT_SYMBOL(mhl_notify_unplugged);

static int mhl_check_sink_version(struct mhl_device *mhl_dev)
{
	int ret_val = 0;

	/* The points to distinguish whether it is Samsung MHL 1.0 Sink
	   Devices are as follows.
	   1.MHL_VERSION = 0x10
	   2.INT_STAT_SIZE = 0x44
	*/
	if (mhl_dev->state.peer_devcap[DEVCAP_OFFSET_MHL_VERSION] == 0x10 &&
	    mhl_dev->state.peer_devcap[DEVCAP_OFFSET_INT_STAT_SIZE] == 0x44)
		ret_val = 1;

	return ret_val;
}

int mhl_notify_online(struct mhl_device *mhl_dev)
{
	if (!mhl_dev)
		return -EFAULT;

	/* WRITE_STAT: DCAP_RDY */
	mhl_msc_send_write_stat(
		mhl_dev,
		MHL_STATUS_REG_CONNECTED_RDY,
		MHL_STATUS_DCAP_RDY);

	/* SET_INT: DCAP_CHG */
	mhl_msc_send_set_int(
		mhl_dev,
		MHL_RCHANGE_INT,
		MHL_INT_DCAP_CHG);

	mhl_dev->mhl_online |= MHL_LINK_ESTABLISHED;

	if (mhl_dev->mhl_online == MHL_ONLINE && mhl_dev->hpd_state)
		kobject_uevent(&mhl_dev->dev.kobj, KOBJ_ONLINE);

	return 0;
}
EXPORT_SYMBOL(mhl_notify_online);

static int mhl_set_mhl_state(struct mhl_device *mhl_dev, unsigned int state)
{
	if (!mhl_dev)
		return -EFAULT;

	mhl_dev->mhl_online |= state;

	if (mhl_dev->mhl_online == MHL_ONLINE && mhl_dev->hpd_state)
		kobject_uevent(&mhl_dev->dev.kobj, KOBJ_ONLINE);

	return 0;
}

static int mhl_clear_mhl_state(struct mhl_device *mhl_dev, unsigned int state)
{
	if (!mhl_dev)
		return -EFAULT;

	mhl_dev->mhl_online &= ~state;

	return 0;
}

int mhl_notify_offline(struct mhl_device *mhl_dev)
{
	if (!mhl_dev)
		return -EFAULT;

	mhl_dev->mhl_online = 0;
	mhl_dev->hpd_state = 0;
	mhl_dev->devcap_state = 0;
	memset(&mhl_dev->state, 0, sizeof(struct mhl_state));

	kobject_uevent(&mhl_dev->dev.kobj, KOBJ_OFFLINE);
	mhl_dev->mhl_online = MHL_OFFLINE;

	return 0;
}
EXPORT_SYMBOL(mhl_notify_offline);

int mhl_notify_hpd(struct mhl_device *mhl_dev, int state)
{
	if (!mhl_dev)
		return -EFAULT;

	if (mhl_dev->hpd_state == state)
		return 0;

	mhl_dev->hpd_state = state;
	if (mhl_dev->mhl_online == MHL_ONLINE && mhl_dev->hpd_state)
		kobject_uevent(&mhl_dev->dev.kobj, KOBJ_ONLINE);
	else
		kobject_uevent(&mhl_dev->dev.kobj, KOBJ_OFFLINE);

	return 0;
}
EXPORT_SYMBOL(mhl_notify_hpd);

static int mhl_qualify_path_enable(struct mhl_device *mhl_dev)
{
	int ret_val = 0;

	if (mhl_dev->tmds_state) {
		ret_val = 1;
	} else {
		if (mhl_check_sink_version(mhl_dev)) {
			ret_val = 1;
			mhl_dev->tmds_state = TRUE;
		}
	}

	return ret_val;
}

/********************************
 * MSC command
 ********************************/

/* this function called by chip driver's send_msc_command implementation.
 * so ops_mutex must be already acquired.
 */
int mhl_msc_command_done(struct mhl_device *mhl_dev,
	struct msc_command_struct *req)
{
	char *envp[2];

	switch (req->command) {
	case MHL_WRITE_STAT:
		if (req->offset == MHL_STATUS_REG_LINK_MODE) {
			if (req->payload.data[0]
				& MHL_STATUS_PATH_ENABLED) {
				/* Enable TMDS output */
				if (mhl_dev->full_operation)
					mhl_dev->ops->tmds_control(TRUE);
				mhl_dev->tmds_state = TRUE;
			} else {
				/* Disable TMDS output */
				mhl_dev->ops->tmds_control(FALSE);
				mhl_dev->tmds_state = FALSE;
			}
		}
		break;
	case MHL_READ_DEVCAP:
		mhl_update_peer_devcap(mhl_dev,
			req->offset, req->retval);
		mhl_dev->devcap_state |= BIT(req->offset);
		if (MHL_DEVCAP_READ_DONE(mhl_dev->devcap_state)) {
			mhl_dev->devcap_state = 0;
			envp[0] = "DEVCAP_CHANGED";
			envp[1] = NULL;
			kobject_uevent_env(&mhl_dev->dev.kobj,
					   KOBJ_CHANGE, envp);
		}
		switch (req->offset) {
		case MHL_DEV_CATEGORY_OFFSET:
			if (req->retval & MHL_DEV_CATEGORY_POW_BIT) {
				if (mhl_dev->ops->charging_control)
					mhl_dev->ops->charging_control
						(TRUE, 700);
			} else {
				if (mhl_dev->ops->charging_control)
					mhl_dev->ops->charging_control
						(FALSE, 0);
			}
			mhl_set_mhl_state(mhl_dev, MHL_PEER_DCAP_READ);
			break;
#ifdef CONFIG_MHL_RAP
		case DEVCAP_OFFSET_FEATURE_FLAG:
			mhl_rap_send_msc_msg(mhl_dev, MHL_MSC_MSG_RAP,
				MHL_RAP_CONTENT_ON);
			break;
#endif /* CONFIG_MHL_RAP */
		case DEVCAP_OFFSET_MHL_VERSION:
		case DEVCAP_OFFSET_INT_STAT_SIZE:
			if (!mhl_dev->tmds_state) {
				if (mhl_qualify_path_enable(mhl_dev))
					mhl_dev->ops->tmds_control(TRUE);
			}
			break;
		}

		break;
	}
	return 0;
}
EXPORT_SYMBOL(mhl_msc_command_done);

static void mhl_msc_command_work(struct work_struct *work)
{
	struct mhl_device *mhl_dev =
		container_of(work, struct mhl_device, msc_command_work);
	struct mhl_event *event;
	int retry;
	int ret;

	mutex_lock(&msc_command_queue_mutex);
	while (!list_empty(&mhl_dev->msc_queue)) {
		event = list_first_entry(&mhl_dev->msc_queue, struct mhl_event,
			msc_queue);
		list_del(&event->msc_queue);
		mutex_unlock(&msc_command_queue_mutex);

		mutex_lock(&mhl_dev->ops_mutex);
		ret = mhl_dev->ops->send_msc_command
			(&event->msc_command_queue);
		mutex_unlock(&mhl_dev->ops_mutex);

		if (ret == -EAGAIN) {
			retry = 2;
			while (retry--) {
				mutex_lock(&mhl_dev->ops_mutex);
				ret = mhl_dev->ops->send_msc_command
					(&event->msc_command_queue);
				mutex_unlock(&mhl_dev->ops_mutex);
				if (ret != -EAGAIN)
					break;
			}
		}
		if (ret == -EAGAIN)
			pr_err("%s: send_msc_command retry out!\n", __func__);

#ifdef CONFIG_MHL_RAP
		if (event->msc_command_queue.payload.data[0] ==
			MHL_MSC_MSG_RAP)
			mod_timer(&mhl_dev->rap_send_timer, RAPK_WAIT_TIME);
#endif

		vfree(event);

		mutex_lock(&msc_command_queue_mutex);
		if (mhl_dev->msc_command_counter)
			mhl_dev->msc_command_counter--;
		else
			pr_err("%s: msc_command_counter fail!\n", __func__);
	}
	mutex_unlock(&msc_command_queue_mutex);
}

static int mhl_queue_msc_command(struct mhl_device *mhl_dev,
	struct msc_command_struct *comm, int queue_kind)
{
	struct mhl_event *new_event;

	mutex_lock(&msc_command_queue_mutex);
	if (mhl_dev->msc_command_counter >= MSC_COMMAND_QUEUE_SIZE) {
		pr_err("%s: queue full!\n", __func__);
		mutex_unlock(&msc_command_queue_mutex);
		return -EBUSY;
	}

	new_event = vmalloc(sizeof(struct mhl_event));
	if (!new_event) {
		pr_err("%s: out of memory!\n", __func__);
		mutex_unlock(&msc_command_queue_mutex);
		return -ENOMEM;
	}
	memcpy(&new_event->msc_command_queue, comm,
		sizeof(struct msc_command_struct));

	mhl_dev->msc_command_counter++;
	if (queue_kind)
		list_add(&new_event->msc_queue, &mhl_dev->msc_queue);
	else
		list_add_tail(&new_event->msc_queue, &mhl_dev->msc_queue);

	mutex_unlock(&msc_command_queue_mutex);
	queue_work(msc_command_workqueue, &mhl_dev->msc_command_work);
	return 0;
}

#ifdef CONFIG_MHL_RAP
/* rap command */
static void mhl_rap_command_work(struct work_struct *work)
{
	struct mhl_device *mhl_dev =
		container_of(work, struct mhl_device, rap_command_work);
	struct mhl_rap_event *event;
	int ret;

	/* Check the RAP running */
	if (mhl_dev->rap_sending)
		return;

	mutex_lock(&rap_command_queue_mutex);
	if (list_empty(&mhl_dev->rap_queue)) {
		mutex_unlock(&rap_command_queue_mutex);
		return;
	}
	event = list_first_entry(&mhl_dev->rap_queue, struct mhl_rap_event,
			rap_queue);
	list_del(&event->rap_queue);
	mutex_unlock(&rap_command_queue_mutex);

	mhl_dev->rap_sending = TRUE;
	mhl_dev->rap_send_retry_num = RAP_SEND_RETRY_MAX;
	mhl_dev->rap_action_code_retry =
		event->rap_command_queue.payload.data[1];

	ret = mhl_queue_msc_command(mhl_dev, &event->rap_command_queue,
		MSC_NORMAL_SEND);
	vfree(event);
	if (ret) {
		pr_err("%s: queue full!\n", __func__);
		return;
	}

	mutex_lock(&rap_command_queue_mutex);
	if (mhl_dev->rap_command_counter)
		mhl_dev->rap_command_counter--;
	else
		pr_err("%s: rap_command_counter fail!\n", __func__);

	mutex_unlock(&rap_command_queue_mutex);
}

static int mhl_queue_rap_command(struct mhl_device *mhl_dev,
	struct msc_command_struct *comm, int queue_kind)
{
	struct mhl_rap_event *new_event;

	mutex_lock(&rap_command_queue_mutex);
	if (mhl_dev->rap_command_counter >= MSC_COMMAND_QUEUE_SIZE) {
		pr_err("%s: queue full!\n", __func__);
		mutex_unlock(&rap_command_queue_mutex);
		return -EBUSY;
	}

	new_event = vmalloc(sizeof(struct mhl_rap_event));
	if (!new_event) {
		pr_err("%s: out of memory!\n", __func__);
		mutex_unlock(&rap_command_queue_mutex);
		return -ENOMEM;
	}
	memcpy(&new_event->rap_command_queue, comm,
		sizeof(struct msc_command_struct));

	mhl_dev->rap_command_counter++;
	if (queue_kind)
		list_add(&new_event->rap_queue, &mhl_dev->rap_queue);
	else
		list_add_tail(&new_event->rap_queue, &mhl_dev->rap_queue);

	mutex_unlock(&rap_command_queue_mutex);
	queue_work(rap_command_workqueue, &mhl_dev->rap_command_work);
	return 0;
}

static void mhl_rap_retry_work(struct work_struct *work)
{
	struct mhl_device *mhl_dev =
		container_of(work, struct mhl_device, rap_retry_work);

	mod_timer(&mhl_dev->rap_send_timer, RAPK_WAIT_TIME);

	mhl_msc_send_msc_msg(mhl_dev, MHL_MSC_MSG_RAP,
		mhl_dev->rap_action_code_retry);
}

/********************************
 * MHL rap timers
 ********************************/
static void mhl_rap_send_timer(unsigned long data)
{
	struct mhl_device *mhl_dev = (struct mhl_device *)data;
	if (!mhl_dev)
		return;

	mhl_dev->rap_sending = FALSE;
	queue_work(rap_command_workqueue, &mhl_dev->rap_command_work);
}

static void mhl_rap_send_retry_timer(unsigned long data)
{
	struct mhl_device *mhl_dev = (struct mhl_device *)data;
	if (!mhl_dev)
		return;

	queue_work(rap_command_workqueue, &mhl_dev->rap_retry_work);
}

static void mhl_init_rap_timers(struct mhl_device *mhl_dev)
{
	init_timer(&mhl_dev->rap_send_timer);
	mhl_dev->rap_send_timer.function = mhl_rap_send_timer;
	mhl_dev->rap_send_timer.data = (unsigned long)mhl_dev;
	mhl_dev->rap_send_timer.expires = 0xffffffffL;
	add_timer(&mhl_dev->rap_send_timer);

	init_timer(&mhl_dev->rap_retry_timer);
	mhl_dev->rap_retry_timer.function = mhl_rap_send_retry_timer;
	mhl_dev->rap_retry_timer.data = (unsigned long)mhl_dev;
	mhl_dev->rap_retry_timer.expires = 0xffffffffL;
	add_timer(&mhl_dev->rap_retry_timer);
}
#endif /* CONFIG_MHL_RAP */
/*
 * MSC: 0x60 WRITE_STAT
 */
int mhl_msc_send_write_stat(struct mhl_device *mhl_dev, u8 offset, u8 value)
{
	struct msc_command_struct req;
	if (!mhl_dev)
		return -EFAULT;
	req.command = MHL_WRITE_STAT;
	req.offset = offset;
	req.payload.data[0] = value;
	return mhl_queue_msc_command(mhl_dev, &req, MSC_NORMAL_SEND);
}
EXPORT_SYMBOL(mhl_msc_send_write_stat);

#ifdef CONFIG_MHL_OSD_NAME
static void mhl_scratchpad_send_work(struct work_struct *work)
{
	struct mhl_device *mhl_dev =
		container_of(work, struct mhl_device, scratchpad_work);

	mhl_msc_request_write_burst(
		mhl_dev, 0x40,
		(u8 *)&mhl_dev->scratchpad_send_data,
		MHL_SCRATCHPAD_SIZE);

#ifdef DEBUG
	{
		char tmpbuf[128];
		int ret, i;
		ret = snprintf(tmpbuf, sizeof(tmpbuf), "SCPD=");
		for (i = 0; i < sizeof(mhl_dev->scratchpad_send_data); i++)
			ret += snprintf(
				tmpbuf + ret, sizeof(tmpbuf)-ret, "%02x",
				*((u8 *)(&mhl_dev->scratchpad_send_data) + i));
		pr_info("RAW SCPD response=%s\n", tmpbuf);
	}
#endif
}

static void mhl_set_osd_name_send(struct mhl_device *mhl_dev, const char *name)
{
	struct mhl_osd_msg *response = &mhl_dev->scratchpad_send_data;
	const u16 adopter_id = htons(
		(mhl_dev->state.peer_devcap[MHL_DEV_ADOPTER_ID_H_OFFSET]<<8) +
		mhl_dev->state.peer_devcap[MHL_DEV_ADOPTER_ID_L_OFFSET]);

	memset(response, 0, sizeof(struct mhl_osd_msg));
	response->adopter_id = adopter_id;
	response->command_id = MHL_SCPD_SET_OSD_NAME;
	strlcpy(response->name, name, sizeof(response->name));
	queue_work(scratchpad_workqueue, &mhl_dev->scratchpad_work);
}

int mhl_notify_scpd_recv(struct mhl_device *mhl_dev, const char *buf)
{
	struct mhl_osd_msg *data = (struct mhl_osd_msg *)buf;
	pr_debug("SCPD received: adopter_id=0x%x, command_id=0x%x\n",
		 ntohs(data->adopter_id), data->command_id);

	if (!external_common_state) {
		pr_err("%s: external_common_state is null!\n", __func__);
		return -EFAULT;
	}
	if (ntohs(data->adopter_id) == MHL_ADOPTER_ID_SOMC &&
		data->command_id == MHL_SCPD_GIVE_OSD_NAME) {
		/* Sony TV asks for phone name */
		mhl_set_osd_name_send(mhl_dev,
			external_common_state->spd_product_description);
	} else {
		pr_debug("%s: Skipped adopter_id=0x%x, command_id=0x%x\n",
			 __func__, ntohs(data->adopter_id), data->command_id);
	}
	return 0;
}
EXPORT_SYMBOL(mhl_notify_scpd_recv);
#endif /* CONFIG_MHL_OSD_NAME */

int mhl_msc_recv_write_stat(struct mhl_device *mhl_dev, u8 offset, u8 value)
{
	if (!mhl_dev)
		return -EFAULT;
	if (offset >= 2)
		return -EFAULT;

	switch (offset) {
	case 0:
		/* DCAP_RDY */
		if (((value ^ mhl_dev->state.device_status[offset]) &
			MHL_STATUS_DCAP_RDY)) {
			if (value & MHL_STATUS_DCAP_RDY) {
				mhl_dev->devcap_state = 0;
				mhl_msc_read_devcap_all(mhl_dev);
			} else {
				/* peer dcap turned not ready */
				mhl_clear_mhl_state(mhl_dev,
					MHL_PEER_DCAP_READ);
			}
		}
		break;
	case 1:
		/* PATH_EN */
		if ((value ^ mhl_dev->state.device_status[offset])
			& MHL_STATUS_PATH_ENABLED) {
			if (value & MHL_STATUS_PATH_ENABLED) {
				mhl_dev->state.peer_status[1]
					|= (MHL_STATUS_PATH_ENABLED |
					MHL_STATUS_CLK_MODE_NORMAL);
				mhl_msc_send_write_stat(
					mhl_dev,
					MHL_STATUS_REG_LINK_MODE,
					mhl_dev->state.peer_status[1]);
#ifdef CONFIG_MHL_RAP
				mhl_rap_send_msc_msg(mhl_dev,
					MHL_MSC_MSG_RAP,
					MHL_RAP_CONTENT_ON);
#endif /* CONFIG_MHL_RAP */
			} else {
				mhl_dev->state.peer_status[1]
					&= ~(MHL_STATUS_PATH_ENABLED |
					MHL_STATUS_CLK_MODE_NORMAL);
				mhl_msc_send_write_stat(
					mhl_dev,
					MHL_STATUS_REG_LINK_MODE,
					mhl_dev->state.peer_status[1]);
			}
		}
		break;
	}
	mhl_dev->state.device_status[offset] = value;
	return 0;
}
EXPORT_SYMBOL(mhl_msc_recv_write_stat);

/*
 * MSC: 0x60 SET_INT
 */
int mhl_msc_send_set_int(struct mhl_device *mhl_dev, u8 offset, u8 mask)
{
	struct msc_command_struct req;
	if (!mhl_dev)
		return -EFAULT;
	req.command = MHL_SET_INT;
	req.offset = offset;
	req.payload.data[0] = mask;
	return mhl_queue_msc_command(mhl_dev, &req, MSC_NORMAL_SEND);
}
EXPORT_SYMBOL(mhl_msc_send_set_int);

int mhl_msc_recv_set_int(struct mhl_device *mhl_dev, u8 offset, u8 mask)
{
	if (!mhl_dev)
		return -EFAULT;
	if (offset >= 2)
		return -EFAULT;

	switch (offset) {
	case 0:
		/* DCAP_CHG */
		if (mask & MHL_INT_DCAP_CHG) {
			/* peer dcap has changed */
			mhl_clear_mhl_state(mhl_dev,
				MHL_PEER_DCAP_READ);
			mhl_dev->devcap_state = 0;
			mhl_msc_read_devcap_all(mhl_dev);
		}
		/* DSCR_CHG */
#ifdef CONFIG_MHL_OSD_NAME
		if (mask & MHL_INT_DSCR_CHG) {
			mhl_dev->ops->scratchpad_data_get();
			mhl_dev->write_burst_requested = FALSE;
		}
#else
		if (mask & MHL_INT_DSCR_CHG)
			mhl_dev->write_burst_requested = FALSE;
#endif /* CONFIG_MHL_OSD_NAME */
		/* REQ_WRT */
		if (mask & MHL_INT_REQ_WRT) {
			mhl_dev->write_burst_requested = TRUE;
			/* SET_INT: GRT_WRT */
			mhl_msc_send_set_int(
				mhl_dev,
				MHL_RCHANGE_INT,
				MHL_INT_GRT_WRT);
		}
		/* GRT_WRT */
		if (mask & MHL_INT_GRT_WRT)
			complete_all(&mhl_dev->req_write_done);
		break;
	case 1:
		/* EDID_CHG */
		if (mask & MHL_INT_EDID_CHG) {
			/* peer EDID has changed.
			   toggle HPD to read EDID again */
			if (mhl_dev->ops->hpd_control) {
				mhl_dev->ops->hpd_control(FALSE);
				msleep(110);
				mhl_dev->ops->hpd_control(TRUE);
			} else {
				/* chip driver doesn't have HPD control
				   send offline/online to userspace */
				kobject_uevent(&mhl_dev->dev.kobj,
					KOBJ_ONLINE);
				msleep(110);
				kobject_uevent(&mhl_dev->dev.kobj,
					KOBJ_OFFLINE);
			}
		}
		break;
	}
	return 0;
}
EXPORT_SYMBOL(mhl_msc_recv_set_int);

/*
 * MSC: 0x61 READ_DEVCAP
 */
#ifdef PRINT_DEVCAP
const char *devcap_reg_name[] = {
	"DEV_STATE       ",
	"MHL_VERSION     ",
	"DEV_CAT         ",
	"ADOPTER_ID_H    ",
	"ADOPTER_ID_L    ",
	"VID_LINK_MODE   ",
	"AUD_LINK_MODE   ",
	"VIDEO_TYPE      ",
	"LOG_DEV_MAP     ",
	"BANDWIDTH       ",
	"FEATURE_FLAG    ",
	"DEVICE_ID_H     ",
	"DEVICE_ID_L     ",
	"SCRATCHPAD_SIZE ",
	"INT_STAT_SIZE   ",
	"Reserved        ",
};

static void mhl_print_devcap(struct mhl_device *mhl_dev, int offset)
{
	u8 reg;
	switch (offset) {
	case DEVCAP_OFFSET_DEV_CAT:
		reg = mhl_dev->state.peer_devcap[offset];
		pr_info("DCAP: %02X %s: %02X DEV_TYPE=%X POW=%s\n",
			offset, devcap_reg_name[offset], reg,
			reg & 0x0F, (reg & 0x10) ? "y" : "n");
		break;
	case DEVCAP_OFFSET_FEATURE_FLAG:
		reg = mhl_dev->state.peer_devcap[offset];
		pr_info("DCAP: %02X %s: %02X RCP=%s RAP=%s SP=%s\n",
			offset, devcap_reg_name[offset], reg,
			(reg & 0x01) ? "y" : "n",
			(reg & 0x02) ? "y" : "n",
			(reg & 0x04) ? "y" : "n");
		break;
	default:
		reg = mhl_dev->state.peer_devcap[offset];
		pr_info("DCAP: %02X %s: %02X\n",
			offset, devcap_reg_name[offset], reg);
		break;
	}
}
#else
static inline void mhl_print_devcap(struct mhl_device *mhl_dev) {}
#endif

int mhl_msc_read_devcap(struct mhl_device *mhl_dev, u8 offset)
{
	struct msc_command_struct req;
	if (!mhl_dev)
		return -EFAULT;
	if (offset > 15)
		return -EFAULT;
	req.command = MHL_READ_DEVCAP;
	req.offset = offset;
	req.payload.data[0] = 0;
	return mhl_queue_msc_command(mhl_dev, &req, MSC_NORMAL_SEND);
}
EXPORT_SYMBOL(mhl_msc_read_devcap);

int mhl_msc_read_devcap_all(struct mhl_device *mhl_dev)
{
	int offset;
	int ret;

	/* check if peer dcap already read */
	if (mhl_dev->mhl_online & MHL_PEER_DCAP_READ)
		return 0;

	for (offset = 0; offset < DEVCAP_SIZE; offset++) {
		ret = mhl_msc_read_devcap(mhl_dev, offset);
		if (ret == -EBUSY)
			pr_err("%s: queue busy!\n", __func__);
	}

	return 0;
}
EXPORT_SYMBOL(mhl_msc_read_devcap_all);

static int mhl_update_peer_devcap(struct mhl_device *mhl_dev,
	int offset, u8 devcap)
{
	if (!mhl_dev)
		return -EFAULT;
	if (offset < 0 || offset > 15)
		return -EFAULT;
	mhl_dev->state.peer_devcap[offset] = devcap;

#ifdef PRINT_DEVCAP
	mhl_print_devcap(mhl_dev, offset);
#endif

	return 0;
}

u8 mhl_peer_devcap(struct mhl_device *mhl_dev, int offset)
{
	return mhl_dev->state.peer_devcap[offset];
}
EXPORT_SYMBOL(mhl_peer_devcap);

/*
 * MSC: 0x68 MSC_MSG
 */
int mhl_msc_send_msc_msg(
	struct mhl_device *mhl_dev, u8 sub_cmd, u8 cmd_data)
{
	struct msc_command_struct req;
	if (!mhl_dev)
		return -EFAULT;
	req.command = MHL_MSC_MSG;
	req.payload.data[0] = sub_cmd;
	req.payload.data[1] = cmd_data;
	return mhl_queue_msc_command(mhl_dev, &req, MSC_NORMAL_SEND);
}
EXPORT_SYMBOL(mhl_msc_send_msc_msg);

static int mhl_prior_send_msc_command_msc_msg(
	struct mhl_device *mhl_dev, u8 sub_cmd, u8 cmd_data)
{
	struct msc_command_struct req;
	if (!mhl_dev)
		return -EFAULT;
	req.command = MHL_MSC_MSG;
	req.payload.data[0] = sub_cmd;
	req.payload.data[1] = cmd_data;
	return mhl_queue_msc_command(mhl_dev, &req, MSC_PRIOR_SEND);
}

#ifdef CONFIG_MHL_RAP
static int mhl_rap_send_msc_msg(
	struct mhl_device *mhl_dev, u8 sub_cmd, u8 cmd_data)
{
	struct msc_command_struct req;
	u8 peer_features;

	if (!mhl_dev)
		return -EFAULT;

	peer_features =
		mhl_dev->state.peer_devcap[MHL_DEV_FEATURE_FLAG_OFFSET];
	if ((peer_features & MHL_FEATURE_RAP_SUPPORT) == 0 ||
		!mhl_dev->full_operation || mhl_dev->suspended ||
		mhl_dev->mhl_online != MHL_ONLINE)
		return -ENXIO;

	req.command = MHL_MSC_MSG;
	req.payload.data[0] = sub_cmd;
	req.payload.data[1] = cmd_data;
	return mhl_queue_rap_command(mhl_dev, &req, MSC_NORMAL_SEND);
}
#endif /* CONFIG_MHL_RAP */

static void mhl_handle_input(struct mhl_device *mhl_dev, u8 key_code)
{
	int input_key_code = (key_code & 0x7F) + MHL_KEYCODE_OFFSET;
	int key_pressed = (key_code & 0x80) == 0 ? 1 : 0;
	pr_debug("%s: input_key_code = 0x%x, key_pressed=%d, " \
		 "key_code = 0x%x, mouse_enabled=%d\n",
		 __func__, input_key_code, key_pressed,
		 key_code, mhl_dev->mouse_enabled);

	if (mhl_dev->mouse_enabled) {
		switch (key_code & 0x7F) {
		case 0x00: /* CENTER */
			input_report_key(mhl_dev->input, BTN_LEFT, key_pressed);
			break;
		case 0x01: /* UP */
			input_report_rel(mhl_dev->input, REL_Y,
					 -mhl_dev->mouse_speed * key_pressed);
			break;
		case 0x02: /* DOWN */
			input_report_rel(mhl_dev->input, REL_Y,
					 mhl_dev->mouse_speed * key_pressed);
			break;
		case 0x03: /* LEFT */
			input_report_rel(mhl_dev->input, REL_X,
					 -mhl_dev->mouse_speed * key_pressed);
			break;
		case 0x04: /* RIGHT */
			input_report_rel(mhl_dev->input, REL_X,
					 mhl_dev->mouse_speed * key_pressed);
			break;
		default:
			input_report_key(mhl_dev->input, input_key_code,
					 key_pressed);
			break;
		}
	} else {
		input_report_key(mhl_dev->input, input_key_code, key_pressed);
	}
	input_sync(mhl_dev->input);
}

static int mhl_rcp_recv(struct mhl_device *mhl_dev, u8 key_code)
{
	int rc;

	rc = mhl_prior_send_msc_command_msc_msg(
		mhl_dev,
		MHL_MSC_MSG_RCPK,
		key_code);

	if (mhl_dev->full_operation && !mhl_dev->suspended) {
		if (key_code & 0x80)
			mhl_dev->key_release_supported = 1;
		if (mhl_dev->input) {
			mhl_handle_input(mhl_dev, key_code);
			/* Fake key release */
			if (!mhl_dev->key_release_supported)
				mhl_handle_input(mhl_dev, key_code | 0x80);
		}
	}

	return rc;
}

#ifdef CONFIG_MHL_RAP
static int mhl_notify_rap_recv(struct mhl_device *mhl_dev, u8 action_code)
{
	char *envp[2];
	envp[0] = kmalloc(128, GFP_KERNEL);
	if (!envp[0])
		return -ENOMEM;
	snprintf(envp[0], 128, "RAP_ACTIONCODE=%x", action_code);
	envp[1] = NULL;
	kobject_uevent_env(&mhl_dev->dev.kobj, KOBJ_CHANGE, envp);
	kfree(envp[0]);
	return 0;
}
#endif /* CONFIG_MHL_RAP */

static int mhl_rap_action(struct mhl_device *mhl_dev, u8 action_code)
{
#ifdef CONFIG_MHL_RAP
	switch (action_code) {
	case MHL_RAP_CONTENT_ON:
		if (!mhl_dev->tmds_state) {
			mhl_dev->ops->tmds_control(TRUE);
			/* notify userspace */
			mhl_notify_rap_recv(mhl_dev, action_code);
		}
		break;
	case MHL_RAP_CONTENT_OFF:
		if (mhl_dev->tmds_state && !mhl_dev->suspended) {
			mhl_dev->ops->hpd_control(FALSE);
			mhl_dev->ops->tmds_control(FALSE);
			mhl_dev->tmds_state = FALSE;
			/* notify userspace */
			mhl_notify_rap_recv(mhl_dev, action_code);
			/* NACK any RAP call until we resumed */
			mhl_dev->suspended = 1;
			/* fake Vendor_Specific key event to suspend phone */
			mhl_handle_input(mhl_dev, 0x7E);
			mhl_handle_input(mhl_dev, 0x7E | 0x80);
		}
		break;
	default:
		break;
	}
	return 0;
#else
	switch (action_code) {
	case MHL_RAP_CONTENT_ON:
		mutex_lock(&mhl_dev->ops_mutex);
		mhl_dev->ops->tmds_control(TRUE);
		mutex_unlock(&mhl_dev->ops_mutex);
		break;
	case MHL_RAP_CONTENT_OFF:
		mutex_lock(&mhl_dev->ops_mutex);
		mhl_dev->ops->tmds_control(FALSE);
		mutex_unlock(&mhl_dev->ops_mutex);
		break;
	default:
		break;
	}
	return 0;
#endif /* CONFIG_MHL_RAP */
}

#ifdef CONFIG_MHL_RAP
#else
static int mhl_notify_rap_recv(struct mhl_device *mhl_dev, u8 action_code)
{
	char *envp[2];
	envp[0] = kmalloc(128, GFP_KERNEL);
	if (!envp[0])
		return -ENOMEM;
	snprintf(envp[0], 128, "RAP_ACTIONCODE=%x", action_code);
	envp[1] = NULL;
	kobject_uevent_env(&mhl_dev->dev.kobj, KOBJ_CHANGE, envp);
	kfree(envp[0]);
	return 0;
}
#endif /* CONFIG_MHL_RAP */

static int mhl_rap_recv(struct mhl_device *mhl_dev, u8 action_code)
{
	u8 error_code;

	switch (action_code) {
	case MHL_RAP_POLL:
	case MHL_RAP_CONTENT_ON:
	case MHL_RAP_CONTENT_OFF:
		if (mhl_dev->full_operation && !mhl_dev->suspended) {
			mhl_rap_action(mhl_dev, action_code);
			error_code = MHL_RAPK_NO_ERROR;
#ifdef CONFIG_MHL_RAP
#else
			/* notify userspace */
			mhl_notify_rap_recv(mhl_dev, action_code);
#endif /* CONFIG_MHL_RAP */
		} else
			error_code = MHL_RAPK_UNSUPPORTED_ACTION_CODE;
		break;
	default:
		error_code = MHL_RAPK_UNRECOGNIZED_ACTION_CODE;
		break;
	}
	/* prior send rapk */
	return mhl_prior_send_msc_command_msc_msg(
		mhl_dev,
		MHL_MSC_MSG_RAPK,
		error_code);
}

int mhl_msc_recv_msc_msg(
	struct mhl_device *mhl_dev, u8 sub_cmd, u8 cmd_data)
{
	int rc = 0;
	if (!mhl_dev)
		return -EFAULT;
	switch (sub_cmd) {
	case MHL_MSC_MSG_RCP:
		pr_info("MHL: receive RCP(0x%02x)\n", cmd_data);
		rc = mhl_rcp_recv(mhl_dev, cmd_data);
		break;
	case MHL_MSC_MSG_RCPK:
		pr_info("MHL: receive RCPK(0x%02x)\n", cmd_data);
		break;
	case MHL_MSC_MSG_RCPE:
		pr_info("MHL: receive RCPE(0x%02x)\n", cmd_data);
		break;
	case MHL_MSC_MSG_RAP:
		pr_info("MHL: receive RAP(0x%02x)\n", cmd_data);
		rc = mhl_rap_recv(mhl_dev, cmd_data);
		break;
	case MHL_MSC_MSG_RAPK:
		pr_info("MHL: receive RAPK(0x%02x)\n", cmd_data);
#ifdef CONFIG_MHL_RAP
		del_timer(&mhl_dev->rap_send_timer);
		if (cmd_data == MHL_RAPK_BUSY) {
			if (!mhl_dev->rap_send_retry_num) {
				mhl_dev->rap_sending = FALSE;
				del_timer(&mhl_dev->rap_retry_timer);
				queue_work(rap_command_workqueue,
					&mhl_dev->rap_command_work);
			} else {
				/* retry */
				mhl_dev->rap_send_retry_num--;
				mod_timer(&mhl_dev->rap_retry_timer,
					RAPK_RETRY_TIME);
			}
		} else {
			mhl_dev->rap_sending = FALSE;
			del_timer(&mhl_dev->rap_retry_timer);
			queue_work(rap_command_workqueue,
				&mhl_dev->rap_command_work);
		}
#endif /* CONFIG_MHL_RAP */
		break;
	default:
		break;
	}
	return rc;
}
EXPORT_SYMBOL(mhl_msc_recv_msc_msg);

static ssize_t mhl_store_rcp(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct mhl_device *mhl_dev = to_mhl_device(dev);
	u8 peer_features =
		mhl_dev->state.peer_devcap[MHL_DEV_FEATURE_FLAG_OFFSET];
	u8 key_code;

	key_code = (u8) atoi(buf);
	key_code &= 0x7f;

	if (peer_features & MHL_FEATURE_RCP_SUPPORT)
		mhl_msc_send_msc_msg(
			mhl_dev,
			MHL_MSC_MSG_RCP,
			key_code);
	else
		return -EFAULT;

	return count;
}

static ssize_t mhl_store_rap(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct mhl_device *mhl_dev = to_mhl_device(dev);
	u8 peer_features =
		mhl_dev->state.peer_devcap[MHL_DEV_FEATURE_FLAG_OFFSET];
	u8 action_code;

	action_code = (u8) atoi(buf);

	if (peer_features & MHL_FEATURE_RAP_SUPPORT) {
		switch (action_code) {
		case MHL_RAP_CONTENT_ON:
		case MHL_RAP_CONTENT_OFF:
#ifdef CONFIG_MHL_RAP
			mhl_rap_send_msc_msg(
				mhl_dev,
				MHL_MSC_MSG_RAP,
				action_code);
#else
			mhl_msc_send_msc_msg(
				mhl_dev,
				MHL_MSC_MSG_RAP,
				action_code);
#endif /* CONFIG_MHL_RAP */
			break;
		default:
			return -EFAULT;
		}
	} else
		return -EFAULT;

	return count;
}

/*
 * MSC: 0x6C WRITE_BURST
 */
static int mhl_msc_write_burst(struct mhl_device *mhl_dev,
	u8 offset, u8 *data, u8 length)
{
	struct msc_command_struct req;
	if (!mhl_dev)
		return -EFAULT;
	req.command = MHL_WRITE_BURST;
	req.offset = offset;
	req.length = length;
	req.payload.burst_data = data;
	return mhl_queue_msc_command(mhl_dev, &req, MSC_NORMAL_SEND);
}

int mhl_msc_request_write_burst(struct mhl_device *mhl_dev,
	u8 offset, u8 *data, u8 length)
{
	int retry = 20;
	int timeout;

	while (mhl_dev->write_burst_requested && retry--)
		msleep(50);
	if (!retry)
		return -EAGAIN;

	if (mhl_dev->state.peer_devcap[MHL_DEV_FEATURE_FLAG_OFFSET] &
		MHL_FEATURE_SP_SUPPORT) {

		retry = 5;
		do {
			init_completion(&mhl_dev->req_write_done);
			/* SET_INT: REQ_WRT */
			mhl_msc_send_set_int(
				mhl_dev,
				MHL_RCHANGE_INT,
				MHL_INT_REQ_WRT);
			timeout = wait_for_completion_interruptible_timeout
				(&mhl_dev->req_write_done, HZ);
		} while (retry-- > 0 && timeout == 0);

		if (!timeout) {
			pr_err("%s: write_burst_send timed out!\n", __func__);
			return -EAGAIN;
		} else
			msleep(20);

		mhl_msc_write_burst(
			mhl_dev,
			offset,
			data,
			length);

		/* SET_INT: DSCR_CHG */
		mhl_msc_send_set_int(
			mhl_dev,
			MHL_RCHANGE_INT,
			MHL_INT_DSCR_CHG);
	}
	return 0;
}
EXPORT_SYMBOL(mhl_msc_request_write_burst);

/********************************
 * USB driver interface
 ********************************/

int mhl_device_discovery(const char *name, int *result)
{
	struct device *dev;
	struct mhl_device *mhl_dev;
	int rc;

	if (!name)
		return -EFAULT;

	/* lookup mhl device by name */
	dev = class_find_device(mhl_class, NULL, (void *) name, __mhl_match);
	if (!dev) {
		pr_err("%s: mhl device (%s) not registered!\n",
			__func__, name);
		return -EFAULT;
	}
	mhl_dev = to_mhl_device(dev);

	/* ok, call chip driver */
	mutex_lock(&mhl_dev->ops_mutex);
	rc = mhl_dev->ops->discovery_result_get(result);
	mutex_unlock(&mhl_dev->ops_mutex);
	if (rc)
		return rc;

	return 0;
}
EXPORT_SYMBOL(mhl_device_discovery);

static ssize_t mhl_show_discovery(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int result;
	mhl_device_discovery(dev_name(dev), &result);
	return snprintf(buf, 4, "%s\n", result ? "mhl" : "usb");
}

static void mhl_usb_online_work(struct work_struct *work)
{
	struct mhl_device *mhl_dev =
		container_of(work, struct mhl_device, usb_online_work);

	if (notify_usb_online) {
		pr_info("%s: mhl usb online(%d)\n",
			__func__, !!(mhl_dev->mhl_online & MHL_PLUGGED));
		notify_usb_online
			(!!(mhl_dev->mhl_online & MHL_PLUGGED));
	}
}

int mhl_register_callback(const char *name, void (*callback)(int on))
{
	int ret = 0;

	if (!notify_usb_online)
		notify_usb_online = callback;
	else {
		pr_err("%s: callback is already registered!\n", __func__);
		ret = -EFAULT;
	}

	return ret;
}
EXPORT_SYMBOL(mhl_register_callback);

int mhl_unregister_callback(const char *name)
{
	int ret = 0;

	if (notify_usb_online)
		notify_usb_online = NULL;
	else {
		pr_err("%s: callback is already unregistered!\n", __func__);
		ret = -EFAULT;
	}

	return ret;
}
EXPORT_SYMBOL(mhl_unregister_callback);

/********************************
 * HDMI driver interface
 ********************************/

int mhl_full_operation(const char *name, int enable)
{
	struct mhl_device *mhl_dev;

	mhl_dev = mhl_get_dev(name);
	if (!mhl_dev)
		return -EFAULT;

	pr_info("%s: %s\n", __func__, enable ? "enable" : "disable");

	if (enable) {
		if (mhl_qualify_path_enable(mhl_dev)) {
			pr_info("%s: enabling TMDS output..\n", __func__);
			/* enable TMDS output */
			mhl_dev->ops->tmds_control(TRUE);
			/* enable HPD routing */
			mhl_dev->ops->hpd_control(TRUE);
		}
	} else {
		if (mhl_dev->tmds_state) {
			pr_info("%s: disabling TMDS output..\n", __func__);
			/* disable HPD routing (force HPD=LOW) */
			mhl_dev->ops->hpd_control(FALSE);
			/* disable TMDS output */
			mhl_dev->ops->tmds_control(FALSE);
		}
	}
	mhl_dev->full_operation = enable;

#ifdef CONFIG_MHL_RAP
	/* If MHL cable is connected before full_operation is called
	 * by HDMI driver during the phone start up, CONTENT_ON won't
	 * be sent in PATH_EN processing. Do it here to avoid Bravia
	 * Sync feature issue.
	 */
	mhl_rap_send_msc_msg(mhl_dev,
		MHL_MSC_MSG_RAP, MHL_RAP_CONTENT_ON);
#endif /* CONFIG_MHL_RAP */

	return 0;
}
EXPORT_SYMBOL(mhl_full_operation);

/********************************
 * interfaces for user space
 ********************************/

static ssize_t mhl_show_adopter_id(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct mhl_device *mhl_dev = to_mhl_device(dev);
	return snprintf(buf, PAGE_SIZE, "%02x%02x\n",
			 mhl_dev->state.peer_devcap
				[MHL_DEV_ADOPTER_ID_H_OFFSET],
			 mhl_dev->state.peer_devcap
				[MHL_DEV_ADOPTER_ID_L_OFFSET]);
}

static ssize_t mhl_show_device_id(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct mhl_device *mhl_dev = to_mhl_device(dev);
	return snprintf(buf, PAGE_SIZE, "%02x%02x\n",
			 mhl_dev->state.peer_devcap
				[MHL_DEV_DEVICE_ID_H_OFFSET],
			 mhl_dev->state.peer_devcap
				[MHL_DEV_DEVICE_ID_L_OFFSET]);
}

static ssize_t mhl_show_mouse_mode(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct mhl_device *mhl_dev = to_mhl_device(dev);
	return snprintf(buf, PAGE_SIZE, "%d,%d\n",
			mhl_dev->mouse_enabled, mhl_dev->mouse_speed);
}

static ssize_t mhl_store_mouse_mode(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct mhl_device *mhl_dev = to_mhl_device(dev);
	sscanf(buf, "%d,%d", &mhl_dev->mouse_enabled, &mhl_dev->mouse_speed);
	return count;
}

/********************************
 * MHL class driver
 ********************************/

static void mhl_device_release(struct device *dev)
{
	struct mhl_device *mhl_dev = to_mhl_device(dev);
	unregister_early_suspend(&mhl_dev->early_suspend);
	kfree(mhl_dev);
}

static void mhl_content_off_and_suspend(struct mhl_device *mhl_dev)
{
#ifdef CONFIG_MHL_RAP
	/* send CONTENT OFF for capable devices */
	mhl_rap_send_msc_msg(mhl_dev, MHL_MSC_MSG_RAP, MHL_RAP_CONTENT_OFF);

	/* NACK any RAP call until we resumed */
	mhl_dev->suspended = 1;
#endif
}

void mhl_device_shutdown(struct mhl_device *mhl_dev)
{
	mhl_content_off_and_suspend(mhl_dev);
}
EXPORT_SYMBOL(mhl_device_shutdown);

#if defined(CONFIG_HAS_EARLYSUSPEND) && defined(CONFIG_MHL_RAP)
static void mhl_early_suspend(struct early_suspend *handler)
{
	struct mhl_device *mhl_dev =
		container_of(handler, struct mhl_device, early_suspend);

	dev_info(&mhl_dev->dev, "early suspend\n");

	if (mhl_dev->mhl_online != MHL_ONLINE || !mhl_dev->tmds_state
			|| !mhl_dev->full_operation) {
		mhl_dev->suspended = 1;
		return;
	}

	mhl_dev->ops->hpd_control(FALSE);
	mhl_dev->ops->tmds_control(FALSE);
	mhl_dev->tmds_state = FALSE;

	mhl_content_off_and_suspend(mhl_dev);
}

static void mhl_early_resume(struct early_suspend *handler)
{
	struct mhl_device *mhl_dev =
		container_of(handler, struct mhl_device, early_suspend);

	dev_info(&mhl_dev->dev, "early resume\n");

	mhl_dev->suspended = 0;

	if (mhl_dev->mhl_online != MHL_ONLINE || !mhl_dev->full_operation)
		return;

	mhl_dev->ops->tmds_control(TRUE);
	mhl_dev->tmds_state = TRUE;
	mhl_dev->ops->hpd_control(TRUE);

	mhl_rap_send_msc_msg(mhl_dev, MHL_MSC_MSG_RAP, MHL_RAP_CONTENT_ON);
}
#endif

struct mhl_device *mhl_device_register(const char *name,
	struct device *parent, void *drvdata, const struct mhl_ops *ops)
{
	struct mhl_device *mhl_dev;
	int rc;
	int i;
	struct input_dev *input;

	if (!name) {
		rc = -EFAULT;
		goto err;
	}

	if (!ops || !ops->discovery_result_get ||
		!ops->send_msc_command || !ops->tmds_control) {
		rc = -EFAULT;
		goto err;
	}

	mhl_dev = kzalloc(sizeof(struct mhl_device), GFP_KERNEL);
	if (!mhl_dev) {
		pr_err("%s: out of memory!\n", __func__);
		rc = -ENOMEM;
		goto err;
	}

	mhl_dev->dev.class = mhl_class;
	mhl_dev->dev.parent = parent;
	mhl_dev->dev.release = mhl_device_release;
	dev_set_name(&mhl_dev->dev, name);
	dev_set_drvdata(&mhl_dev->dev, drvdata);

#if defined(CONFIG_HAS_EARLYSUSPEND) && defined(CONFIG_MHL_RAP)
	mhl_dev->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN;
	mhl_dev->early_suspend.suspend = mhl_early_suspend;
	mhl_dev->early_suspend.resume = mhl_early_resume;
#endif
	register_early_suspend(&mhl_dev->early_suspend);

	rc = device_register(&mhl_dev->dev);
	if (rc) {
		pr_err("%s: Failed to register device!\n", __func__);
		goto err_register;
	}

	mhl_dev->ops = ops;
	mutex_init(&mhl_dev->ops_mutex);

	INIT_LIST_HEAD(&mhl_dev->msc_queue);
	INIT_WORK(&mhl_dev->msc_command_work, mhl_msc_command_work);
#ifdef CONFIG_MHL_RAP
	mhl_init_rap_timers(mhl_dev);
	INIT_LIST_HEAD(&mhl_dev->rap_queue);
	INIT_WORK(&mhl_dev->rap_command_work, mhl_rap_command_work);
	INIT_WORK(&mhl_dev->rap_retry_work, mhl_rap_retry_work);
#endif /* CONFIG_MHL_RAP */
	INIT_WORK(&mhl_dev->usb_online_work, mhl_usb_online_work);

#ifdef CONFIG_MHL_OSD_NAME
	INIT_WORK(&mhl_dev->scratchpad_work, mhl_scratchpad_send_work);
#endif /* CONFIG_MHL_OSD_NAME */

	/* device added */
	kobject_uevent(&mhl_dev->dev.kobj, KOBJ_ADD);

	mhl_dev->key_release_supported = 0;
	mhl_dev->mouse_enabled = 0;
	mhl_dev->mouse_speed = MOUSE_MOVE_DISTANCE;
	mhl_dev->input = input_allocate_device();
	if (!mhl_dev->input) {
		dev_err(&mhl_dev->dev, "failed to alloc input device\n");
		rc = -ENOMEM;
		goto err_input_alloc;
	}

	input = mhl_dev->input;
	input->name = "mhl-rcp";

	input->keycodesize = sizeof(u16);
	input->keycodemax = 127 + 1;

	input->evbit[0] = EV_KEY | EV_REP | EV_REL;
	input_set_capability(input, EV_REL, REL_X);
	input_set_capability(input, EV_REL, REL_Y);
	input_set_capability(input, EV_KEY, BTN_LEFT);
	for (i = 0; i < input->keycodemax; i++)
		input_set_capability(input, EV_KEY, i + MHL_KEYCODE_OFFSET);

	rc = input_register_device(input);
	if (rc) {
		dev_err(&mhl_dev->dev, "failed to register input device\n");
		goto err_input_register;
	}

	pr_info("MHL: mhl device (%s) registered\n", name);
	return mhl_dev;

err_input_register:
	input_free_device(input);
	mhl_dev->input = NULL;
err_input_alloc:
	mhl_device_unregister(mhl_dev);
err_register:
	unregister_early_suspend(&mhl_dev->early_suspend);
	kfree(mhl_dev);
err:
	return ERR_PTR(rc);
}
EXPORT_SYMBOL(mhl_device_register);

void mhl_device_unregister(struct mhl_device *mhl_dev)
{
	if (!mhl_dev)
		return;

#ifdef CONFIG_MHL_RAP
	del_timer(&mhl_dev->rap_send_timer);
	del_timer(&mhl_dev->rap_retry_timer);
#endif /* CONFIG_MHL_RAP */
	mutex_lock(&mhl_dev->ops_mutex);
	if (mhl_dev->input) {
		input_unregister_device(mhl_dev->input);
		mhl_dev->input = NULL;
	}
	mhl_dev->ops = NULL;
	mutex_unlock(&mhl_dev->ops_mutex);
	unregister_early_suspend(&mhl_dev->early_suspend);
	device_unregister(&mhl_dev->dev);
}
EXPORT_SYMBOL(mhl_device_unregister);

static struct device_attribute mhl_class_attributes[] = {
	__ATTR(discovery, 0440, mhl_show_discovery, NULL),
	__ATTR(rcp, 0660, NULL, mhl_store_rcp),
	__ATTR(rap, 0660, NULL, mhl_store_rap),
	__ATTR(device_id, 0440, mhl_show_device_id, NULL),
	__ATTR(adopter_id, 0440, mhl_show_adopter_id, NULL),
	__ATTR(mouse_mode, 0660, mhl_show_mouse_mode, mhl_store_mouse_mode),
	__ATTR_NULL,
};

static int __init mhl_class_init(void)
{
	mhl_class = class_create(THIS_MODULE, "mhl");
	if (IS_ERR(mhl_class)) {
		pr_err("%s: unable to create mhl class!\n", __func__);
		return PTR_ERR(mhl_class);
	}

	mhl_class->dev_attrs = mhl_class_attributes;

	msc_command_workqueue = create_singlethread_workqueue
					("mhl_msc_command");
#ifdef CONFIG_MHL_RAP
	rap_command_workqueue = create_singlethread_workqueue
					("mhl_rap_command");
#endif /* CONFIG_MHL_RAP */

#ifdef CONFIG_MHL_OSD_NAME
	scratchpad_workqueue = create_singlethread_workqueue
					("mhl_scratchpad");
#endif /* CONFIG_MHL_OSD_NAME */

	usb_online_workqueue = create_workqueue("mhl_usb_online");
	return 0;
}

static void __exit mhl_class_exit(void)
{
	class_destroy(mhl_class);
}

subsys_initcall(mhl_class_init);
module_exit(mhl_class_exit);

MODULE_LICENSE("GPL v2");
MODULE_VERSION("1.0");
MODULE_AUTHOR("Sony Mobile Communications AB");
MODULE_DESCRIPTION("MHL Control Abstraction");
