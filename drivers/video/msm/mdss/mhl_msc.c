/* Copyright (c) 2013, The Linux Foundation. All rights reserved.
 * Copyright (C) 2013 Sony Mobile Communications AB.
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
 * NOTE: This file has been modified by Sony Mobile Communications AB.
 * Modifications are licensed under the License.
 */

#include <linux/types.h>
#include <linux/mhl_8334.h>
#include <linux/vmalloc.h>
#include <linux/input.h>
#include "mhl_msc.h"
#include "mdss_hdmi_mhl.h"

#define PRINT_DEVCAP

#define MHL_TX_EVENT_DSCR_CHG	1
#define MHL_ADOPTER_ID_SOMC		0x03A7
#define MHL_OSD_NAME_SIZE		12
#define MHL_OSD_NAME			"Xperia"	/* len : 1<=n<=12 */
#define MHL_SCPD_GIVE_OSD_NAME		0xA1
#define MHL_SCPD_SET_OSD_NAME		0xA2

struct mhl_osd_msg {
	u16 adopter_id;
	u8 command_id;
	u8 data[MHL_OSD_NAME_SIZE];
	u8 decrement_cnt;
};

static struct mhl_tx_ctrl *mhl_ctrl;
static DEFINE_MUTEX(msc_send_workqueue_mutex);
struct workqueue_struct *scratchpad_workqueue;
struct workqueue_struct *screen_ctrl_workqueue;

static void mhl_notify_event(struct mhl_tx_ctrl *mhl_ctrl, int event);

/*
 * Android defines [DEFAULT_LONG_PRESS_TIMEOUT = 500].
 * 450ms is enought not to let system detect LongPress.
 */
#define RCP_KEY_RELEASE_TIME1	450

/*
 * In samsung dongle case, cursor moves a lot than normal situation with
 * pressing direction key twice in a short time.
 * Please add following explanation after "twice" .
 * Android defines [DEFAULT_LONG_PRESS_TIMEOUT = 500], so if release event is
 * not injected within 500ms, then the time out occurs and system become to
 * detect the event as LongPress. For that reason, we set the release timer as
 * 200ms not to let system judge it is LongPress.
 */
#define RCP_KEY_RELEASE_TIME2	200
#define RCP_KEY_INVALID			-1

static DEFINE_MUTEX(rcp_key_release_mutex);
struct workqueue_struct *rcp_key_release_workqueue;

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

static void mhl_print_devcap(u8 offset, u8 devcap)
{
	switch (offset) {
	case DEVCAP_OFFSET_DEV_CAT:
		pr_info("DCAP: %02X %s: %02X DEV_TYPE=%X POW=%s\n",
			offset, devcap_reg_name[offset], devcap,
			devcap & 0x0F, (devcap & 0x10) ? "y" : "n");
		break;
	case DEVCAP_OFFSET_FEATURE_FLAG:
		pr_info("DCAP: %02X %s: %02X RCP=%s RAP=%s SP=%s\n",
			offset, devcap_reg_name[offset], devcap,
			(devcap & 0x01) ? "y" : "n",
			(devcap & 0x02) ? "y" : "n",
			(devcap & 0x04) ? "y" : "n");
		break;
	default:
		pr_info("DCAP: %02X %s: %02X\n",
			offset, devcap_reg_name[offset], devcap);
		break;
	}
}
#else
static inline void mhl_print_devcap(u8 offset, u8 devcap) {}
#endif

static bool mhl_qualify_path_enable(struct mhl_tx_ctrl *mhl_ctrl)
{
	int rc = false;

	/* The points to distinguish whether it is Samsung MHL 1.0 Sink
	 * Devices are as follows.
	 * 1.MHL_VERSION = 0x10
	 * 2.INT_STAT_SIZE = 0x44
	 *   In SOMC investigation, only this Samsung model has this
	 *   INT_STAT_SIZE value. This value is not compliant with MHL spec.
	 *   It means illegal. So, other model product wouldn't have the 0x44.
	 */
	if (mhl_ctrl->tmds_en_state)
		rc = true;
	else if (mhl_ctrl->devcap[DEVCAP_OFFSET_MHL_VERSION] == 0x10 &&
		mhl_ctrl->devcap[DEVCAP_OFFSET_INT_STAT_SIZE] == 0x44)
		rc = true;
	return rc;
}

void mhl_register_msc(struct mhl_tx_ctrl *ctrl)
{
	if (ctrl)
		mhl_ctrl = ctrl;
}

static int mhl_flag_scrpd_burst_req(struct mhl_tx_ctrl *mhl_ctrl,
		struct msc_command_struct *req)
{
	int postpone_send = 0;

	if ((req->command == MHL_SET_INT) &&
	    (req->offset == MHL_RCHANGE_INT)) {
		if (mhl_ctrl->scrpd_busy) {
			/* reduce priority */
			if (req->payload.data[0] == MHL_INT_REQ_WRT)
				postpone_send = 1;
		} else {
			if (req->payload.data[0] == MHL_INT_REQ_WRT) {
				mhl_ctrl->scrpd_busy = true;
				mhl_ctrl->wr_burst_pending = true;
			} else if (req->payload.data[0] == MHL_INT_GRT_WRT) {
					mhl_ctrl->scrpd_busy = true;
			}
		}
	}
	return postpone_send;
}

void mhl_msc_send_work(struct work_struct *work)
{
	struct mhl_tx_ctrl *mhl_ctrl =
		container_of(work, struct mhl_tx_ctrl, mhl_msc_send_work);
	struct msc_cmd_envelope *cmd_env;
	int ret, postpone_send;
	/*
	 * Remove item from the queue
	 * and schedule it
	 */
	mutex_lock(&msc_send_workqueue_mutex);
	while (!list_empty(&mhl_ctrl->list_cmd)) {
		cmd_env = list_first_entry(&mhl_ctrl->list_cmd,
					   struct msc_cmd_envelope,
					   msc_queue_envelope);
		list_del(&cmd_env->msc_queue_envelope);
		mutex_unlock(&msc_send_workqueue_mutex);

		postpone_send = mhl_flag_scrpd_burst_req(
			mhl_ctrl,
			&cmd_env->msc_cmd_msg);
		if (postpone_send) {
			if (cmd_env->msc_cmd_msg.retry-- > 0) {
				mutex_lock(&msc_send_workqueue_mutex);
				list_add_tail(
					&cmd_env->msc_queue_envelope,
					&mhl_ctrl->list_cmd);
				mutex_unlock(&msc_send_workqueue_mutex);
			} else {
				pr_err("%s: max scrpd retry out\n",
				       __func__);
			}
		} else {
			ret = mhl_send_msc_command(mhl_ctrl,
						   &cmd_env->msc_cmd_msg);
			if (ret == -EAGAIN) {
				int retry = 2;
				while (retry--) {
					ret = mhl_send_msc_command(
						mhl_ctrl,
						&cmd_env->msc_cmd_msg);
					if (ret != -EAGAIN)
						break;
				}
			}
			if (ret == -EAGAIN)
				pr_err("%s: send_msc_command retry out!\n",
				       __func__);
			vfree(cmd_env);
		}

		mutex_lock(&msc_send_workqueue_mutex);
	}
	mutex_unlock(&msc_send_workqueue_mutex);
}

int mhl_queue_msc_command(struct mhl_tx_ctrl *mhl_ctrl,
			  struct msc_command_struct *req,
			  int priority_send)
{
	struct msc_cmd_envelope *cmd_env;

	mutex_lock(&msc_send_workqueue_mutex);
	cmd_env = vmalloc(sizeof(struct msc_cmd_envelope));
	if (!cmd_env) {
		pr_err("%s: out of memory!\n", __func__);
		mutex_unlock(&msc_send_workqueue_mutex);
		return -ENOMEM;
	}

	memcpy(&cmd_env->msc_cmd_msg, req,
	       sizeof(struct msc_command_struct));

	if (priority_send)
		list_add(&cmd_env->msc_queue_envelope,
			 &mhl_ctrl->list_cmd);
	else
		list_add_tail(&cmd_env->msc_queue_envelope,
			      &mhl_ctrl->list_cmd);
	mutex_unlock(&msc_send_workqueue_mutex);
	queue_work(mhl_ctrl->msc_send_workqueue, &mhl_ctrl->mhl_msc_send_work);

	return 0;
}

static int mhl_update_devcap(struct mhl_tx_ctrl *mhl_ctrl,
	int offset, u8 devcap)
{
	if (!mhl_ctrl)
		return -EFAULT;
	if (offset < 0 || offset > 15)
		return -EFAULT;
	mhl_ctrl->devcap[offset] = devcap;
#ifdef PRINT_DEVCAP
	mhl_print_devcap(offset, mhl_ctrl->devcap[offset]);
#endif

	return 0;
}

int mhl_msc_clear(struct mhl_tx_ctrl *mhl_ctrl)
{
	if (!mhl_ctrl)
		return -EFAULT;

	memset(mhl_ctrl->devcap, 0, 16);
	mhl_ctrl->devcap_state = 0;
	mhl_ctrl->tmds_ctrl_en = false;
	mhl_ctrl->path_en_state = 0;
	mhl_ctrl->status[0] = 0;
	mhl_ctrl->status[1] = 0;
	mhl_ctrl->scrpd_busy = 0;
	mhl_ctrl->wr_burst_pending = 0;

	return 0;
}

static int mhl_rap_send_msc_msg(struct mhl_tx_ctrl *mhl_ctrl, u8 cmd_data)
{
	struct msc_command_struct req;

	if ((mhl_ctrl->devcap[DEVCAP_OFFSET_FEATURE_FLAG] &
		MHL_FEATURE_RAP_SUPPORT) == 0 ||
		!mhl_ctrl->screen_mode)
		return -ENXIO;

	req.command = MHL_MSC_MSG;
	req.offset = 0;
	req.payload.data[0] = MHL_MSC_MSG_RAP;
	req.payload.data[1] = cmd_data;
	return mhl_queue_msc_command(mhl_ctrl, &req, MSC_NORMAL_SEND);
}

int mhl_msc_command_done(struct mhl_tx_ctrl *mhl_ctrl,
			 struct msc_command_struct *req)
{
	switch (req->command) {
	case MHL_WRITE_STAT:
		if (req->offset == MHL_STATUS_REG_LINK_MODE) {
			if (req->payload.data[0]
			    & MHL_STATUS_PATH_ENABLED) {
				/* Enable TMDS output */
				mhl_tmds_ctrl(mhl_ctrl, TMDS_ENABLE);
				if (mhl_ctrl->devcap_state == MHL_DEVCAP_ALL) {
					mhl_drive_hpd(mhl_ctrl, HPD_UP);
					mhl_ctrl->tmds_ctrl_en = true;
				}
			} else {
				/* Disable TMDS output */
				mhl_tmds_ctrl(mhl_ctrl, TMDS_DISABLE);
				mhl_drive_hpd(mhl_ctrl, HPD_DOWN);
			}
		}
		break;
	case MHL_READ_DEVCAP:
		mhl_update_devcap(mhl_ctrl,
			req->offset, req->retval);
		mhl_ctrl->devcap_state |= BIT(req->offset);
		switch (req->offset) {
		case MHL_DEV_CATEGORY_OFFSET:
			if (req->retval & MHL_DEV_CATEGORY_POW_BIT)
				pr_debug("%s: devcap pow bit set\n",
					 __func__);
			else
				pr_debug("%s: devcap pow bit unset\n",
					 __func__);
			power_supply_set_present(&mhl_ctrl->mhl_psy,
				!!(req->retval & MHL_DEV_CATEGORY_POW_BIT));
			break;
		case DEVCAP_OFFSET_FEATURE_FLAG:
			mhl_rap_send_msc_msg(mhl_ctrl, MHL_RAP_CONTENT_ON);
			break;
		case DEVCAP_OFFSET_RESERVED:
			if (mhl_qualify_path_enable(mhl_ctrl)) {
				mhl_tmds_ctrl(mhl_ctrl, TMDS_ENABLE);
				mhl_drive_hpd(mhl_ctrl, HPD_UP);
				mhl_ctrl->tmds_ctrl_en = true;
			}
			break;
		}
		break;
	case MHL_WRITE_BURST:
		mhl_msc_send_set_int(
			mhl_ctrl,
			MHL_RCHANGE_INT,
			MHL_INT_DSCR_CHG,
			MSC_PRIORITY_SEND);
		break;
	}
	return 0;
}

int mhl_msc_send_set_int(struct mhl_tx_ctrl *mhl_ctrl,
			 u8 offset, u8 mask, u8 prior)
{
	struct msc_command_struct req;
	req.command = MHL_SET_INT;
	req.offset = offset;
	req.payload.data[0] = mask;
	return mhl_queue_msc_command(mhl_ctrl, &req, prior);
}

int mhl_msc_send_write_stat(struct mhl_tx_ctrl *mhl_ctrl,
			    u8 offset, u8 value)
{
	struct msc_command_struct req;
	req.command = MHL_WRITE_STAT;
	req.offset = offset;
	req.payload.data[0] = value;
	return mhl_queue_msc_command(mhl_ctrl, &req, MSC_NORMAL_SEND);
}

static int mhl_msc_write_burst(struct mhl_tx_ctrl *mhl_ctrl,
	u8 offset, u8 *data, u8 length)
{
	struct msc_command_struct req;
	if (!mhl_ctrl)
		return -EFAULT;

	if (!mhl_ctrl->wr_burst_pending)
		return -EFAULT;

	req.command = MHL_WRITE_BURST;
	req.offset = offset;
	req.length = length;
	req.payload.burst_data = data;
	mhl_queue_msc_command(mhl_ctrl, &req, MSC_PRIORITY_SEND);
	mhl_ctrl->wr_burst_pending = false;
	return 0;
}

int mhl_msc_send_msc_msg(struct mhl_tx_ctrl *mhl_ctrl,
			 u8 sub_cmd, u8 cmd_data)
{
	struct msc_command_struct req;
	req.command = MHL_MSC_MSG;
	req.payload.data[0] = sub_cmd;
	req.payload.data[1] = cmd_data;
	return mhl_queue_msc_command(mhl_ctrl, &req, MSC_NORMAL_SEND);
}

void mhl_screen_notify(struct mhl_tx_ctrl *mhl_ctrl, int screen_mode)
{
	if (!mhl_ctrl) {
		pr_err("%s: invalid input\n", __func__);
		return;
	}

	if (screen_mode) {
		mhl_ctrl->screen_mode = true;
		if (mhl_ctrl->cur_state == POWER_STATE_D0_MHL) {
			if (mhl_ctrl->tmds_ctrl_en) {
				mhl_tmds_ctrl(mhl_ctrl, TMDS_ENABLE);
				mhl_drive_hpd(mhl_ctrl, HPD_UP);
			}
			mhl_rap_send_msc_msg(mhl_ctrl, MHL_RAP_CONTENT_ON);
		}
	} else {
		if (!mhl_ctrl->tmds_en_state || !mhl_ctrl->screen_mode) {
			mhl_ctrl->screen_mode = false;
			return;
		}
		if (mhl_ctrl->cur_state == POWER_STATE_D0_MHL) {
			mhl_drive_hpd(mhl_ctrl, HPD_DOWN);
			mhl_tmds_ctrl(mhl_ctrl, TMDS_DISABLE);
			mhl_rap_send_msc_msg(mhl_ctrl, MHL_RAP_CONTENT_OFF);
		}
		/* NACK any RAP call until chagne to screen on */
		mhl_ctrl->screen_mode = false;
	}
}

/*
 * Certain MSC msgs such as RCPK, RCPE and RAPK
 * should be transmitted as a high priority
 * because these msgs should be sent within
 * 1000ms of a receipt of RCP/RAP. So such msgs can
 * be added to the head of msc cmd queue.
 */
static int mhl_msc_send_prior_msc_msg(struct mhl_tx_ctrl *mhl_ctrl,
				      u8 sub_cmd, u8 cmd_data)
{
	struct msc_command_struct req;
	req.command = MHL_MSC_MSG;
	req.payload.data[0] = sub_cmd;
	req.payload.data[1] = cmd_data;
	return mhl_queue_msc_command(mhl_ctrl, &req, MSC_PRIORITY_SEND);
}

int mhl_msc_read_devcap(struct mhl_tx_ctrl *mhl_ctrl, u8 offset)
{
	struct msc_command_struct req;
	if (offset < 0 || offset > 15)
		return -EFAULT;
	req.command = MHL_READ_DEVCAP;
	req.offset = offset;
	req.payload.data[0] = 0;
	return mhl_queue_msc_command(mhl_ctrl, &req, MSC_NORMAL_SEND);
}

int mhl_msc_read_devcap_all(struct mhl_tx_ctrl *mhl_ctrl)
{
	int offset;
	int ret;

	for (offset = 0; offset < DEVCAP_SIZE; offset++) {
		ret = mhl_msc_read_devcap(mhl_ctrl, offset);
		if (ret == -EBUSY)
			pr_err("%s: queue busy!\n", __func__);
	}
	return ret;
}

void mhl_rcp_key_release_work(struct work_struct *work)
{
	struct mhl_tx_ctrl *mhl_ctrl =
		container_of(work, struct mhl_tx_ctrl, rcp_key_release_work);

	if (!mhl_ctrl)
		return;

	mutex_lock(&rcp_key_release_mutex);
	if (mhl_ctrl->rcp_key_release != true) {
		mutex_unlock(&rcp_key_release_mutex);
		return;
	}
	if (mhl_ctrl->rcp_pre_input_key == RCP_KEY_INVALID) {
		mutex_unlock(&rcp_key_release_mutex);
		return;
	}
	input_report_key(mhl_ctrl->input, mhl_ctrl->rcp_pre_input_key, 0);
	input_sync(mhl_ctrl->input);
	mhl_ctrl->rcp_key_release = false;
	mhl_ctrl->rcp_pre_input_key = RCP_KEY_INVALID;
	mutex_unlock(&rcp_key_release_mutex);
}

void mhl_rcp_key_release_timer(unsigned long data)
{
	struct mhl_tx_ctrl *mhl_ctrl = (struct mhl_tx_ctrl *)data;
	if (!mhl_ctrl)
		return;

	queue_work(rcp_key_release_workqueue, &mhl_ctrl->rcp_key_release_work);
}

void mhl_set_mouse_move_distance_dx(struct mhl_tx_ctrl *mhl_ctrl,
				    int mouse_move_distance_dx)
{
	if (!mhl_ctrl) {
		pr_err("%s: invalid input\n", __func__);
		return;
	}

	mhl_ctrl->mouse_move_distance_dx = mouse_move_distance_dx;
}

void mhl_set_mouse_move_distance_dy(struct mhl_tx_ctrl *mhl_ctrl,
				    int mouse_move_distance_dy)
{
	if (!mhl_ctrl) {
		pr_err("%s: invalid input\n", __func__);
		return;
	}

	mhl_ctrl->mouse_move_distance_dy = mouse_move_distance_dy;
}

static void mhl_handle_input(struct mhl_tx_ctrl *mhl_ctrl,
			     u8 key_code, u16 input_key_code)
{
	int axis = REL_X, distance = -1;
	bool mouse_event = false;
	int key_press = (key_code & 0x80) == 0;

	if (mhl_ctrl->mouse_mode) {
		switch (input_key_code) {
		case KEY_UP:
			axis = REL_Y;
			distance = -mhl_ctrl->mouse_move_distance_dy;
			mouse_event = true;
			break;
		case KEY_DOWN:
			axis = REL_Y;
			distance = mhl_ctrl->mouse_move_distance_dy;
			mouse_event = true;
			break;
		case KEY_LEFT:
			axis = REL_X;
			distance = -mhl_ctrl->mouse_move_distance_dx;
			mouse_event = true;
			break;
		case KEY_RIGHT:
			axis = REL_X;
			distance = mhl_ctrl->mouse_move_distance_dx;
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
			input_report_rel(mhl_ctrl->input, axis, distance);
			input_sync(mhl_ctrl->input);
			return;
		} else {
			input_key_code = BTN_LEFT;
		}
	}

	if (key_press) {
		mutex_lock(&rcp_key_release_mutex);
		if (mhl_ctrl->rcp_pre_input_key != input_key_code &&
			mhl_ctrl->rcp_key_release) {
			/* Release previous press key code if current press key
			 * differs from previous press key.
			 */
			input_report_key(mhl_ctrl->input,
				mhl_ctrl->rcp_pre_input_key, 0);
			input_sync(mhl_ctrl->input);
		}
		mhl_ctrl->rcp_key_release = true;
		mhl_ctrl->rcp_pre_input_key = input_key_code;
		mutex_unlock(&rcp_key_release_mutex);
		/* rcp key release timer start */
		switch (input_key_code) {
		case KEY_UP:
		case KEY_DOWN:
		case KEY_LEFT:
		case KEY_RIGHT:
		case BTN_LEFT:
			mod_timer(&mhl_ctrl->rcp_key_release_timer,
				jiffies +
				msecs_to_jiffies(RCP_KEY_RELEASE_TIME2));
			break;
		default:
			mod_timer(&mhl_ctrl->rcp_key_release_timer,
				jiffies +
				msecs_to_jiffies(RCP_KEY_RELEASE_TIME1));
			break;
		}
	} else {
		mutex_lock(&rcp_key_release_mutex);
		mhl_ctrl->rcp_key_release = false;
		mhl_ctrl->rcp_pre_input_key = RCP_KEY_INVALID;
		mutex_unlock(&rcp_key_release_mutex);
	}

	pr_debug("%s: send key events[%x][%x][%d]\n",
		 __func__, key_code, input_key_code, key_press);
	mutex_lock(&rcp_key_release_mutex);
	input_report_key(mhl_ctrl->input, input_key_code, key_press);
	input_sync(mhl_ctrl->input);
	mutex_unlock(&rcp_key_release_mutex);
}

int mhl_rcp_recv(struct mhl_tx_ctrl *mhl_ctrl, u8 key_code)
{
	u8 index = key_code & 0x7f;
	u16 input_key_code;

	if (!mhl_ctrl->rcp_key_code_tbl) {
		pr_err("%s: RCP Key Code Table not initialized\n", __func__);
		return -EINVAL;
	}

	input_key_code = mhl_ctrl->rcp_key_code_tbl[index];

	if ((index < mhl_ctrl->rcp_key_code_tbl_len) &&
	    (input_key_code > 0)) {
		/* prior send rcpk */
		mhl_msc_send_prior_msc_msg(
			mhl_ctrl,
			MHL_MSC_MSG_RCPK,
			key_code);

		if (mhl_ctrl->input)
			mhl_handle_input(mhl_ctrl, key_code, input_key_code);
	} else {
		/* prior send rcpe */
		mhl_msc_send_prior_msc_msg(
			mhl_ctrl,
			MHL_MSC_MSG_RCPE,
			MHL_RCPE_INEFFECTIVE_KEY_CODE);

		/* send rcpk after rcpe send */
		mhl_msc_send_prior_msc_msg(
			mhl_ctrl,
			MHL_MSC_MSG_RCPK,
			key_code);
	}
	return 0;
}


static void mhl_screen_control_work(struct work_struct *work)
{
	struct mhl_tx_ctrl *mhl_ctrl = container_of
		(work, struct mhl_tx_ctrl, screen_work);
	bool mode;
	int timeout = 50;

	if (!mhl_ctrl)
		return;

	/* Currently, if same screen mode, no action */
	if (mhl_ctrl->screen_control == MHL_RAP_CONTENT_ON)
		mode = true;
	else
		mode = false;
	if (mhl_ctrl->screen_mode == mode) {
		pr_debug("%s: same screen mode\n", __func__);
		return;
	}

	/* send power key event */
	input_report_key(mhl_ctrl->input, KEY_VENDOR, 1);
	input_sync(mhl_ctrl->input);
	input_report_key(mhl_ctrl->input, KEY_VENDOR, 0);
	input_sync(mhl_ctrl->input);

	/* wait until contorl mode matches screen_mode */
	do {
		if (mode) {
			if (mhl_ctrl->screen_mode == true)
				break;
		} else {
			if (mhl_ctrl->screen_mode == false)
				break;
		}
		msleep(20);
	} while (--timeout);
	if (!timeout)
		pr_warn("screen_mode change timeout!\n");
}

static int mhl_rap_action(struct mhl_tx_ctrl *mhl_ctrl, u8 action_code)
{
	switch (action_code) {
	case MHL_RAP_CONTENT_ON:
		mhl_tmds_ctrl(mhl_ctrl, TMDS_ENABLE);
		mhl_drive_hpd(mhl_ctrl, HPD_UP);
		mhl_ctrl->screen_control = MHL_RAP_CONTENT_ON;
		queue_work(screen_ctrl_workqueue, &mhl_ctrl->screen_work);
		break;
	case MHL_RAP_CONTENT_OFF:
		mhl_drive_hpd(mhl_ctrl, HPD_DOWN);
		mhl_tmds_ctrl(mhl_ctrl, TMDS_DISABLE);
		mhl_ctrl->screen_control = MHL_RAP_CONTENT_OFF;
		queue_work(screen_ctrl_workqueue, &mhl_ctrl->screen_work);
		break;
	default:
		break;
	}
	return 0;
}

static int mhl_rap_recv(struct mhl_tx_ctrl *mhl_ctrl, u8 action_code)
{
	u8 error_code;

	switch (action_code) {
	case MHL_RAP_POLL:
	case MHL_RAP_CONTENT_ON:
	case MHL_RAP_CONTENT_OFF:
		mhl_rap_action(mhl_ctrl, action_code);
		error_code = MHL_RAPK_NO_ERROR;
		break;
	default:
		error_code = MHL_RAPK_UNRECOGNIZED_ACTION_CODE;
		break;
	}
	/* prior send rapk */
	return mhl_msc_send_prior_msc_msg(
		mhl_ctrl,
		MHL_MSC_MSG_RAPK,
		error_code);
}

int mhl_msc_recv_msc_msg(struct mhl_tx_ctrl *mhl_ctrl,
			 u8 sub_cmd, u8 cmd_data)
{
	int rc = 0;
	switch (sub_cmd) {
	case MHL_MSC_MSG_RCP:
		pr_debug("MHL: receive RCP(0x%02x)\n", cmd_data);
		rc = mhl_rcp_recv(mhl_ctrl, cmd_data);
		break;
	case MHL_MSC_MSG_RCPK:
		pr_debug("MHL: receive RCPK(0x%02x)\n", cmd_data);
		break;
	case MHL_MSC_MSG_RCPE:
		pr_debug("MHL: receive RCPE(0x%02x)\n", cmd_data);
		break;
	case MHL_MSC_MSG_RAP:
		pr_debug("MHL: receive RAP(0x%02x)\n", cmd_data);
		rc = mhl_rap_recv(mhl_ctrl, cmd_data);
		break;
	case MHL_MSC_MSG_RAPK:
		pr_debug("MHL: receive RAPK(0x%02x)\n", cmd_data);
		break;
	default:
		break;
	}
	return rc;
}

int mhl_msc_recv_set_int(struct mhl_tx_ctrl *mhl_ctrl,
			 u8 offset, u8 set_int)
{
	int prior;
	if (offset >= 2)
		return -EFAULT;

	switch (offset) {
	case 0:
		if (set_int & MHL_INT_DCAP_CHG) {
			/* peer dcap has changed */
			mhl_ctrl->devcap_state = 0;
			mhl_msc_read_devcap_all(mhl_ctrl);
		}
		if (set_int & MHL_INT_DSCR_CHG) {
			/* peer's scratchpad reg changed */
			pr_debug("%s: dscr chg\n", __func__);
			mhl_read_scratchpad(mhl_ctrl);
			mhl_ctrl->scrpd_busy = false;
			mhl_notify_event(mhl_ctrl, MHL_TX_EVENT_DSCR_CHG);
		}
		if (set_int & MHL_INT_REQ_WRT) {
			/* SET_INT: REQ_WRT */
			if (mhl_ctrl->scrpd_busy) {
				prior = MSC_NORMAL_SEND;
			} else {
				prior = MSC_PRIORITY_SEND;
				mhl_ctrl->scrpd_busy = true;
			}
			mhl_msc_send_set_int(
				mhl_ctrl,
				MHL_RCHANGE_INT,
				MHL_INT_GRT_WRT,
				prior);
		}
		if (set_int & MHL_INT_GRT_WRT) {
			/* SET_INT: GRT_WRT */
			pr_debug("%s: recvd req to permit/grant write",
				 __func__);
			complete_all(&mhl_ctrl->req_write_done);
			mhl_msc_write_burst(
				mhl_ctrl,
				MHL_SCRATCHPAD_OFFSET,
				mhl_ctrl->scrpd.data,
				mhl_ctrl->scrpd.length);
		}
		break;
	case 1:
		if (set_int & MHL_INT_EDID_CHG) {
			/* peer EDID has changed
			 * toggle HPD to read EDID
			 */
			pr_debug("%s: EDID CHG\n", __func__);
			mhl_drive_hpd(mhl_ctrl, HPD_DOWN);
			msleep(110);
			mhl_drive_hpd(mhl_ctrl, HPD_UP);
		}
	}
	return 0;
}

int mhl_msc_recv_write_stat(struct mhl_tx_ctrl *mhl_ctrl,
			    u8 offset, u8 value)
{
	if (offset >= 2)
		return -EFAULT;

	switch (offset) {
	case 0:
		/*
		 * connected device bits
		 * changed and DEVCAP READY
		 */
		if (((value ^ mhl_ctrl->status[offset]) &
		     MHL_STATUS_DCAP_RDY)) {
			if (value & MHL_STATUS_DCAP_RDY) {
				mhl_ctrl->devcap_state = 0;
				mhl_msc_read_devcap_all(mhl_ctrl);
			} else {
				/*
				 * peer dcap turned not ready
				 * use old devap state
				 */
				pr_debug("%s: DCAP RDY bit cleared\n",
					 __func__);
			}
		}
		break;
	case 1:
		/*
		 * connected device bits
		 * changed and PATH ENABLED
		 * bit set
		 */
		if ((value ^ mhl_ctrl->status[offset])
			& MHL_STATUS_PATH_ENABLED) {
			if (value & MHL_STATUS_PATH_ENABLED) {
				mhl_ctrl->path_en_state
					|= (MHL_STATUS_PATH_ENABLED |
					    MHL_STATUS_CLK_MODE_NORMAL);
				mhl_msc_send_write_stat(
					mhl_ctrl,
					MHL_STATUS_REG_LINK_MODE,
					mhl_ctrl->path_en_state);
				mhl_rap_send_msc_msg(
					mhl_ctrl, MHL_RAP_CONTENT_ON);
			} else {
				mhl_ctrl->path_en_state
					&= ~(MHL_STATUS_PATH_ENABLED |
					     MHL_STATUS_CLK_MODE_NORMAL);
				mhl_msc_send_write_stat(
					mhl_ctrl,
					MHL_STATUS_REG_LINK_MODE,
					mhl_ctrl->path_en_state);
			}
		}
		break;
	}
	mhl_ctrl->status[offset] = value;
	return 0;
}

static int mhl_request_write_burst(struct mhl_tx_ctrl *mhl_ctrl,
				   u8 start_reg,
				   u8 length, u8 *data)
{
	int i, reg;
	int timeout, retry = 20;

	if (!(mhl_ctrl->devcap[DEVCAP_OFFSET_FEATURE_FLAG] &
	      MHL_FEATURE_SP_SUPPORT)) {
		pr_debug("MHL: SCRATCHPAD_NOT_SUPPORTED\n");
		return -EFAULT;
	}

	/*
	 * scratchpad remains busy as long as a peer's permission or
	 * write bursts are pending; experimentally it was found that
	 * 50ms is optimal
	 */
	while (mhl_ctrl->scrpd_busy && retry--)
		msleep(50);
	if (!retry) {
		pr_debug("MHL: scratchpad_busy\n");
		return -EBUSY;
	}

	for (i = 0, reg = start_reg; (i < length) &&
		     (reg < MHL_SCRATCHPAD_SIZE); i++, reg++)
		mhl_ctrl->scrpd.data[reg] = data[i];
	mhl_ctrl->scrpd.length = length;
	mhl_ctrl->scrpd.offset = start_reg;

	retry = 5;
	do {
		init_completion(&mhl_ctrl->req_write_done);
		mhl_msc_send_set_int(
			mhl_ctrl,
			MHL_RCHANGE_INT,
			MHL_INT_REQ_WRT,
			MSC_PRIORITY_SEND);
		timeout = wait_for_completion_interruptible_timeout(
			&mhl_ctrl->req_write_done,
			msecs_to_jiffies(MHL_BURST_WAIT));
		if (!timeout)
			mhl_ctrl->scrpd_busy = false;
	} while (retry-- && timeout == 0);
	if (!timeout) {
		pr_err("%s: timed out!\n", __func__);
		return -EAGAIN;
	}

	return 0;
}

/* write scratchpad entry */
int mhl_write_scratchpad(struct mhl_tx_ctrl *mhl_ctrl,
			  u8 offset, u8 length, u8 *data)
{
	int rc;

	if ((length < ADOPTER_ID_SIZE) ||
	    (length > MAX_SCRATCHPAD_TRANSFER_SIZE) ||
	    (offset > (MAX_SCRATCHPAD_TRANSFER_SIZE - ADOPTER_ID_SIZE)) ||
	    ((offset + length) > MAX_SCRATCHPAD_TRANSFER_SIZE)) {
		pr_debug("MHL: write_burst (0x%02x)\n", -EINVAL);
		return  -EINVAL;
	}

	rc = mhl_request_write_burst(mhl_ctrl, offset, length, data);

	return rc;
}

static void mhl_scratchpad_send_work(struct work_struct *work)
{
	struct mhl_tx_ctrl *mhl_ctrl =
		container_of(work, struct mhl_tx_ctrl, scratchpad_work);


	int retryCount = 0;
	int rc;
	u8 offset = mhl_ctrl->scrpd.offset;
	u8 length = mhl_ctrl->scrpd.length;
	u8 *data = mhl_ctrl->scrpd.data;

	do {
		rc = mhl_request_write_burst(mhl_ctrl, offset, length, data);
		switch (rc) {
		case -EBUSY:
			pr_debug("%s: scratchpad write busy\n", __func__);
			break;
		default:
			pr_debug("%s: scratchpad write error:%d\n",
				__func__, rc);
			break;
		}
		msleep(100);
	} while ((retryCount++ < 5) && rc);
}

static void mhl_set_osd_name_send(struct mhl_tx_ctrl *mhl_ctrl, u8 *osd_name)
{
	unsigned char buf[MHL_OSD_NAME_SIZE];
	int i;

	memset(buf, 0, sizeof(buf));
	mhl_ctrl->scrpd.data[0] =
		mhl_ctrl->devcap[DEVCAP_OFFSET_ADOPTER_ID_H];
	mhl_ctrl->scrpd.data[1] =
		mhl_ctrl->devcap[DEVCAP_OFFSET_ADOPTER_ID_L];
	mhl_ctrl->scrpd.data[2] = MHL_SCPD_SET_OSD_NAME;
	for (i = 0; i < MHL_OSD_NAME_SIZE; i++) {
		if ('\0' == osd_name[i])
			break;
		mhl_ctrl->scrpd.data[3+i] = osd_name[i];
	}
	mhl_ctrl->scrpd.offset = 0;
	mhl_ctrl->scrpd.length = MHL_SCRATCHPAD_SIZE;

	queue_work(scratchpad_workqueue, &mhl_ctrl->scratchpad_work);
}

static void mhl_notify_event(struct mhl_tx_ctrl *mhl_ctrl, int event)
{
	struct mhl_osd_msg osd;

	switch (event) {
	case MHL_TX_EVENT_DSCR_CHG:
		osd.adopter_id = (mhl_ctrl->scrpd.data[0] << 8) |
				  mhl_ctrl->scrpd.data[1];
		osd.command_id = mhl_ctrl->scrpd.data[2];

		if (MHL_ADOPTER_ID_SOMC != osd.adopter_id) {
			pr_info("%s: adopter_id is mismatched(%04x)",
				__func__, osd.adopter_id);
		}

		if (osd.command_id == MHL_SCPD_GIVE_OSD_NAME)
			mhl_set_osd_name_send(mhl_ctrl, (u8 *)MHL_OSD_NAME);
		else
			pr_debug("%s: unknown command_id=(%#x)\n",
				__func__, osd.command_id);
		break;
	default:
		break;
	}
}

int mhl_msc_init(struct mhl_tx_ctrl *mhl_ctrl)
{
	/* RCP release */
	init_timer(&mhl_ctrl->rcp_key_release_timer);
	mhl_ctrl->rcp_key_release_timer.function =
		mhl_rcp_key_release_timer;
	mhl_ctrl->rcp_key_release_timer.data = (unsigned long)mhl_ctrl;
	mhl_ctrl->rcp_key_release_timer.expires = 0xffffffffL;
	add_timer(&mhl_ctrl->rcp_key_release_timer);
	mhl_ctrl->rcp_key_release = false;
	mhl_ctrl->rcp_pre_input_key = RCP_KEY_INVALID;
	rcp_key_release_workqueue = create_singlethread_workqueue
					("mhl_rcp_key_release");
	INIT_WORK(&mhl_ctrl->rcp_key_release_work, mhl_rcp_key_release_work);

	scratchpad_workqueue = create_singlethread_workqueue("mhl_scratchpad");
	INIT_WORK(&mhl_ctrl->scratchpad_work, mhl_scratchpad_send_work);

	screen_ctrl_workqueue = create_singlethread_workqueue
					("mhl_screencontrol");
	INIT_WORK(&mhl_ctrl->screen_work, mhl_screen_control_work);

	return 0;
}
