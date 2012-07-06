/*
 * Copyright (C) 2011 Sony Ericsson Mobile Communications AB.
 * Copyright (C) 2012 Sony Mobile Communications AB.
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

#define PRINT_DEVCAP

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

static DEFINE_MUTEX(usb_online_mutex);
struct workqueue_struct *usb_online_workqueue;

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
		switch (req->offset) {
		case MHL_DEV_CATEGORY_OFFSET:
			if (req->retval & MHL_DEV_CATEGORY_POW_BIT) {
				if (mhl_dev->ops->charging_control)
					mhl_dev->ops->charging_control
						(TRUE, 500);
			} else {
				if (mhl_dev->ops->charging_control)
					mhl_dev->ops->charging_control
						(FALSE, 0);
			}
			mhl_set_mhl_state(mhl_dev, MHL_PEER_DCAP_READ);
			break;
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
			if (value & MHL_STATUS_DCAP_RDY)
				mhl_msc_read_devcap_all(mhl_dev);
			else {
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
			mhl_msc_read_devcap_all(mhl_dev);
		}
		/* DSCR_CHG */
		if (mask & MHL_INT_DSCR_CHG)
			mhl_dev->write_burst_requested = FALSE;
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
				mutex_lock(&mhl_dev->ops_mutex);
				mhl_dev->ops->hpd_control(FALSE);
				mutex_unlock(&mhl_dev->ops_mutex);
				msleep(110);
				mutex_lock(&mhl_dev->ops_mutex);
				mhl_dev->ops->hpd_control(TRUE);
				mutex_unlock(&mhl_dev->ops_mutex);
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
	if (offset < 0 || offset > 15)
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

static int mhl_notify_rcp_recv(struct mhl_device *mhl_dev, u8 key_code)
{
	char *envp[2];
	if (!mhl_dev)
		return -EFAULT;
	envp[0] = kmalloc(128, GFP_KERNEL);
	if (!envp[0])
		return -ENOMEM;
	snprintf(envp[0], 128, "RCP_KEYCODE=%x", key_code);
	envp[1] = NULL;
	kobject_uevent_env(&mhl_dev->dev.kobj, KOBJ_CHANGE, envp);
	kfree(envp[0]);
	return 0;
}

/* supported RCP key code */
static const u8 support_rcp_key_code_tbl[] = {
	1, 1, 1, 1, 1, 0, 0, 0,		/* 0x00~0x07 */
	0, 1, 1, 0, 0, 1, 0, 0,		/* 0x08~0x0f */
	0, 0, 0, 0, 0, 0, 0, 0,		/* 0x10~0x17 */
	0, 0, 0, 0, 0, 0, 0, 0,		/* 0x18~0x1f */
	0, 0, 0, 0, 0, 0, 0, 0,		/* 0x20~0x27 */
	0, 0, 0, 0, 0, 0, 0, 0,		/* 0x28~0x2f */
	0, 0, 0, 0, 0, 0, 0, 0,		/* 0x30~0x37 */
	0, 0, 0, 0, 0, 0, 0, 0,		/* 0x38~0x3f */
	0, 0, 0, 0, 1, 1, 1, 0,		/* 0x40~0x47 */
	1, 1, 0, 1, 1, 0, 0, 0,		/* 0x48~0x4f */
	0, 0, 0, 0, 0, 0, 0, 0,		/* 0x50~0x57 */
	0, 0, 0, 0, 0, 0, 0, 0,		/* 0x58~0x5f */
	1, 1, 0, 0, 1, 0, 0, 0,		/* 0x60~0x67 */
	0, 0, 0, 0, 0, 0, 0, 0,		/* 0x68~0x6f */
	0, 1, 1, 1, 1, 0, 0, 0,		/* 0x70~0x77 */
	0, 0, 0, 0, 0, 0, 0, 0		/* 0x78~0x7f */
};

static int mhl_rcp_recv(struct mhl_device *mhl_dev, u8 key_code)
{
	int rc;
	if (support_rcp_key_code_tbl[(key_code & 0x7f)]) {
		/* notify userspace */
		mhl_notify_rcp_recv(mhl_dev, key_code);
		/* prior send rcpk */
		rc = mhl_prior_send_msc_command_msc_msg(
			mhl_dev,
			MHL_MSC_MSG_RCPK,
			key_code);
	} else {
		/* prior send rcpe */
		rc = mhl_prior_send_msc_command_msc_msg(
			mhl_dev,
			MHL_MSC_MSG_RCPE,
			MHL_RCPE_INEFFECTIVE_KEY_CODE);
		if (rc)
			return rc;
		/* send rcpk after rcpe send */
		rc = mhl_msc_send_msc_msg(
			mhl_dev,
			MHL_MSC_MSG_RCPK,
			key_code);
	}
	return rc;
}

static int mhl_rap_action(struct mhl_device *mhl_dev, u8 action_code)
{
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
}

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

static int mhl_rap_recv(struct mhl_device *mhl_dev, u8 action_code)
{
	u8 error_code;

	switch (action_code) {
	case MHL_RAP_POLL:
	case MHL_RAP_CONTENT_ON:
	case MHL_RAP_CONTENT_OFF:
		mhl_rap_action(mhl_dev, action_code);
		error_code = MHL_RAPK_NO_ERROR;
		/* notify userspace */
		mhl_notify_rap_recv(mhl_dev, action_code);
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
	ssize_t ret = strnlen(buf, PAGE_SIZE);

	key_code = (u8) atoi(buf);
	key_code &= 0x7f;

	if (peer_features & MHL_FEATURE_RCP_SUPPORT)
		mhl_msc_send_msc_msg(
			mhl_dev,
			MHL_MSC_MSG_RCP,
			key_code);
	else
		return -EFAULT;

	return ret;
}

static ssize_t mhl_store_rap(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct mhl_device *mhl_dev = to_mhl_device(dev);
	u8 peer_features =
		mhl_dev->state.peer_devcap[MHL_DEV_FEATURE_FLAG_OFFSET];
	u8 action_code;
	ssize_t ret = strnlen(buf, PAGE_SIZE);

	action_code = (u8) atoi(buf);

	if (peer_features & MHL_FEATURE_RAP_SUPPORT) {
		switch (action_code) {
		case MHL_RAP_CONTENT_ON:
		case MHL_RAP_CONTENT_OFF:
			mhl_msc_send_msc_msg(
				mhl_dev,
				MHL_MSC_MSG_RAP,
				action_code);
			break;
		default:
			return -EFAULT;
		}
	} else
		return -EFAULT;

	return ret;
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

		init_completion(&mhl_dev->req_write_done);
		/* SET_INT: REQ_WRT */
		mhl_msc_send_set_int(
			mhl_dev,
			MHL_RCHANGE_INT,
			MHL_INT_REQ_WRT);
		timeout = wait_for_completion_interruptible_timeout
			(&mhl_dev->req_write_done, HZ);
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

	if (mhl_dev->notify_usb_online) {
		pr_info("%s: mhl usb online(%d)\n",
			__func__, !!(mhl_dev->mhl_online & MHL_PLUGGED));
		mhl_dev->notify_usb_online
			(!!(mhl_dev->mhl_online & MHL_PLUGGED));
	}
}

int mhl_register_callback(const char *name, void (*callback)(int on))
{
	struct mhl_device *mhl_dev;
	int ret = 0;

	mhl_dev = mhl_get_dev(name);
	if (!mhl_dev)
		return -EFAULT;

	if (!mhl_dev->notify_usb_online) {
		mhl_dev->notify_usb_online = callback;
		mhl_dev->notify_usb_online
			(!!(mhl_dev->mhl_online & MHL_PLUGGED));
	} else {
		pr_err("%s: callback is already registered!\n", __func__);
		ret = -EFAULT;
	}

	return ret;
}
EXPORT_SYMBOL(mhl_register_callback);

int mhl_unregister_callback(const char *name)
{
	struct mhl_device *mhl_dev;
	int ret = 0;

	mhl_dev = mhl_get_dev(name);
	if (!mhl_dev)
		return -EFAULT;

	if (mhl_dev->notify_usb_online)
		mhl_dev->notify_usb_online = NULL;
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

	return 0;
}
EXPORT_SYMBOL(mhl_full_operation);

/********************************
 * MHL class driver
 ********************************/

static void mhl_device_release(struct device *dev)
{
	struct mhl_device *mhl_dev = to_mhl_device(dev);
	kfree(mhl_dev);
}

struct mhl_device *mhl_device_register(const char *name,
	struct device *parent, void *drvdata, const struct mhl_ops *ops)
{
	struct mhl_device *mhl_dev;
	int rc;

	if (!name)
		return ERR_PTR(-EFAULT);

	if (!ops || !ops->discovery_result_get ||
		!ops->send_msc_command || !ops->tmds_control)
		return ERR_PTR(-EFAULT);

	mhl_dev = kzalloc(sizeof(struct mhl_device), GFP_KERNEL);
	if (!mhl_dev) {
		pr_err("%s: out of memory!\n", __func__);
		return ERR_PTR(-ENOMEM);
	}

	mhl_dev->dev.class = mhl_class;
	mhl_dev->dev.parent = parent;
	mhl_dev->dev.release = mhl_device_release;
	dev_set_name(&mhl_dev->dev, name);
	dev_set_drvdata(&mhl_dev->dev, drvdata);

	rc = device_register(&mhl_dev->dev);
	if (rc) {
		kfree(mhl_dev);
		return ERR_PTR(rc);
	}

	mhl_dev->ops = ops;
	mutex_init(&mhl_dev->ops_mutex);

	INIT_LIST_HEAD(&mhl_dev->msc_queue);
	INIT_WORK(&mhl_dev->msc_command_work, mhl_msc_command_work);
	INIT_WORK(&mhl_dev->usb_online_work, mhl_usb_online_work);

	/* device added */
	kobject_uevent(&mhl_dev->dev.kobj, KOBJ_ADD);

	pr_info("MHL: mhl device (%s) registered\n", name);

	return mhl_dev;
}
EXPORT_SYMBOL(mhl_device_register);

void mhl_device_unregister(struct mhl_device *mhl_dev)
{
	if (!mhl_dev)
		return;

	mutex_lock(&mhl_dev->ops_mutex);
	mhl_dev->ops = NULL;
	mutex_unlock(&mhl_dev->ops_mutex);
	device_unregister(&mhl_dev->dev);
}
EXPORT_SYMBOL(mhl_device_unregister);

static struct device_attribute mhl_class_attributes[] = {
	__ATTR(discovery, 0440, mhl_show_discovery, NULL),
	__ATTR(rcp, 0660, NULL, mhl_store_rcp),
	__ATTR(rap, 0660, NULL, mhl_store_rap),
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
MODULE_AUTHOR("Sony Ericsson Mobile Communications AB");
MODULE_DESCRIPTION("MHL Control Abstruction");
