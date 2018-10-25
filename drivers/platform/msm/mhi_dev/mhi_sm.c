/* Copyright (c) 2015-2016, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/export.h>
#include <linux/workqueue.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/debugfs.h>
#include <linux/ipa_mhi.h>
#include "mhi_hwio.h"
#include "mhi_sm.h"

#define MHI_SM_DBG(fmt, args...) \
	mhi_log(MHI_MSG_DBG, fmt, ##args)

#define MHI_SM_ERR(fmt, args...) \
	mhi_log(MHI_MSG_ERROR, fmt, ##args)

#define MHI_SM_FUNC_ENTRY() MHI_SM_DBG("ENTRY\n")
#define MHI_SM_FUNC_EXIT() MHI_SM_DBG("EXIT\n")


static inline const char *mhi_sm_dev_event_str(enum mhi_dev_event state)
{
	const char *str;

	switch (state) {
	case MHI_DEV_EVENT_CTRL_TRIG:
		str = "MHI_DEV_EVENT_CTRL_TRIG";
		break;
	case MHI_DEV_EVENT_M0_STATE:
		str = "MHI_DEV_EVENT_M0_STATE";
		break;
	case MHI_DEV_EVENT_M1_STATE:
		str = "MHI_DEV_EVENT_M1_STATE";
		break;
	case MHI_DEV_EVENT_M2_STATE:
		str = "MHI_DEV_EVENT_M2_STATE";
		break;
	case MHI_DEV_EVENT_M3_STATE:
		str = "MHI_DEV_EVENT_M3_STATE";
		break;
	case MHI_DEV_EVENT_HW_ACC_WAKEUP:
		str = "MHI_DEV_EVENT_HW_ACC_WAKEUP";
		break;
	case MHI_DEV_EVENT_CORE_WAKEUP:
		str = "MHI_DEV_EVENT_CORE_WAKEUP";
		break;
	default:
		str = "INVALID MHI_DEV_EVENT";
	}

	return str;
}

static inline const char *mhi_sm_mstate_str(enum mhi_dev_state state)
{
	const char *str;

	switch (state) {
	case MHI_DEV_RESET_STATE:
		str = "RESET";
		break;
	case MHI_DEV_READY_STATE:
		str = "READY";
		break;
	case MHI_DEV_M0_STATE:
		str = "M0";
		break;
	case MHI_DEV_M1_STATE:
		str = "M1";
		break;
	case MHI_DEV_M2_STATE:
		str = "M2";
		break;
	case MHI_DEV_M3_STATE:
		str = "M3";
		break;
	case MHI_DEV_SYSERR_STATE:
		str = "SYSTEM ERROR";
		break;
	default:
		str = "INVALID";
		break;
	}

	return str;
}
enum mhi_sm_ep_pcie_state {
	MHI_SM_EP_PCIE_LINK_DISABLE,
	MHI_SM_EP_PCIE_D0_STATE,
	MHI_SM_EP_PCIE_D3_HOT_STATE,
	MHI_SM_EP_PCIE_D3_COLD_STATE,
};

static inline const char *mhi_sm_dstate_str(enum mhi_sm_ep_pcie_state state)
{
	const char *str;

	switch (state) {
	case MHI_SM_EP_PCIE_LINK_DISABLE:
		str = "EP_PCIE_LINK_DISABLE";
		break;
	case MHI_SM_EP_PCIE_D0_STATE:
		str = "D0_STATE";
		break;
	case MHI_SM_EP_PCIE_D3_HOT_STATE:
		str = "D3_HOT_STATE";
		break;
	case MHI_SM_EP_PCIE_D3_COLD_STATE:
		str = "D3_COLD_STATE";
		break;
	default:
		str = "INVALID D-STATE";
		break;
	}

	return str;
}

static inline const char *mhi_sm_pcie_event_str(enum ep_pcie_event event)
{
	const char *str;

	switch (event) {
	case EP_PCIE_EVENT_LINKDOWN:
		str = "EP_PCIE_LINKDOWN_EVENT";
		break;
	case EP_PCIE_EVENT_LINKUP:
		str = "EP_PCIE_LINKUP_EVENT";
		break;
	case EP_PCIE_EVENT_PM_D3_HOT:
		str = "EP_PCIE_PM_D3_HOT_EVENT";
		break;
	case EP_PCIE_EVENT_PM_D3_COLD:
		str = "EP_PCIE_PM_D3_COLD_EVENT";
		break;
	case EP_PCIE_EVENT_PM_RST_DEAST:
		str = "EP_PCIE_PM_RST_DEAST_EVENT";
		break;
	case EP_PCIE_EVENT_PM_D0:
		str = "EP_PCIE_PM_D0_EVENT";
		break;
	case EP_PCIE_EVENT_MHI_A7:
		str = "EP_PCIE_MHI_A7";
		break;
	default:
		str = "INVALID_PCIE_EVENT";
		break;
	}

	return str;
}

/**
 * struct mhi_sm_device_event - mhi-core event work
 * @event: mhi core state change event
 * @work: work struct
 *
 * used to add work for mhi state change event to mhi_sm_wq
 */
struct mhi_sm_device_event {
	enum mhi_dev_event event;
	struct work_struct work;
};

/**
 * struct mhi_sm_ep_pcie_event - ep-pcie event work
 * @event: ep-pcie link state change event
 * @work: work struct
 *
 * used to add work for ep-pcie link state change event to mhi_sm_wq
 */
struct mhi_sm_ep_pcie_event {
	enum ep_pcie_event event;
	struct work_struct work;
};

/**
 * struct mhi_sm_stats - MHI state machine statistics, viewable using debugfs
 * @m0_event_cnt: total number of MHI_DEV_EVENT_M0_STATE events
 * @m3_event_cnt: total number of MHI_DEV_EVENT_M3_STATE events
 * @hw_acc_wakeup_event_cnt: total number of MHI_DEV_EVENT_HW_ACC_WAKEUP events
 * @mhi_core_wakeup_event_cnt: total number of MHI_DEV_EVENT_CORE_WAKEUP events
 * @linkup_event_cnt: total number of EP_PCIE_EVENT_LINKUP events
 * @rst_deast_event_cnt: total number of EP_PCIE_EVENT_PM_RST_DEAST events
 * @d3_hot_event_cnt: total number of EP_PCIE_EVENT_PM_D3_HOT events
 * @d3_cold_event_cnt: total number of EP_PCIE_EVENT_PM_D3_COLD events
 * @d0_event_cnt: total number of EP_PCIE_EVENT_PM_D0 events
 * @linkdown_event_cnt: total number of EP_PCIE_EVENT_LINKDOWN events
 */
struct mhi_sm_stats {
	int m0_event_cnt;
	int m3_event_cnt;
	int hw_acc_wakeup_event_cnt;
	int mhi_core_wakeup_event_cnt;
	int linkup_event_cnt;
	int rst_deast_event_cnt;
	int d3_hot_event_cnt;
	int d3_cold_event_cnt;
	int d0_event_cnt;
	int linkdown_event_cnt;
};

/**
 * struct mhi_sm_dev - MHI state manager context information
 * @mhi_state: MHI M state of the MHI device
 * @d_state: EP-PCIe D state of the MHI device
 * @mhi_dev: MHI device struct pointer
 * @mhi_state_lock: mutex for mhi_state
 * @syserr_occurred:flag to indicate if a syserr condition has occurred.
 * @mhi_sm_wq: workqueue for state change events
 * @pending_device_events: number of pending mhi state change events in sm_wq
 * @pending_pcie_events: number of pending mhi state change events in sm_wq
 * @stats: stats on the handled and pending events
 */
struct mhi_sm_dev {
	enum mhi_dev_state mhi_state;
	enum mhi_sm_ep_pcie_state d_state;
	struct mhi_dev *mhi_dev;
	struct mutex mhi_state_lock;
	bool syserr_occurred;
	struct workqueue_struct *mhi_sm_wq;
	atomic_t pending_device_events;
	atomic_t pending_pcie_events;
	struct mhi_sm_stats stats;
};
static struct mhi_sm_dev *mhi_sm_ctx;


#ifdef CONFIG_DEBUG_FS
#define MHI_SM_MAX_MSG_LEN 1024
static char dbg_buff[MHI_SM_MAX_MSG_LEN];
static struct dentry *dent;
static struct dentry *dfile_stats;

static ssize_t mhi_sm_debugfs_read(struct file *file, char __user *ubuf,
				size_t count, loff_t *ppos);
static ssize_t mhi_sm_debugfs_write(struct file *file,
			const char __user *ubuf, size_t count, loff_t *ppos);

const struct file_operations mhi_sm_stats_ops = {
	.read = mhi_sm_debugfs_read,
	.write = mhi_sm_debugfs_write,
};

static void mhi_sm_debugfs_init(void)
{
	const mode_t read_write_mode = S_IRUSR | S_IRGRP | S_IROTH |
			S_IWUSR | S_IWGRP | S_IWOTH;

	dent = debugfs_create_dir("mhi_sm", 0);
	if (IS_ERR(dent)) {
		MHI_SM_ERR("fail to create folder mhi_sm\n");
		return;
	}

	dfile_stats =
		debugfs_create_file("stats", read_write_mode, dent,
				0, &mhi_sm_stats_ops);
	if (!dfile_stats || IS_ERR(dfile_stats)) {
		MHI_SM_ERR("fail to create file stats\n");
		goto fail;
	}
	return;
fail:
	debugfs_remove_recursive(dent);
}

static void mhi_sm_debugfs_destroy(void)
{
	debugfs_remove_recursive(dent);
}
#else
static inline void mhi_sm_debugfs_init(void) {}
static inline void mhi_sm_debugfs_destroy(void) {}
#endif /*CONFIG_DEBUG_FS*/


static void mhi_sm_mmio_set_mhistatus(enum mhi_dev_state state)
{
	struct mhi_dev *dev = mhi_sm_ctx->mhi_dev;

	MHI_SM_FUNC_ENTRY();

	switch (state) {
	case MHI_DEV_READY_STATE:
		MHI_SM_DBG("set MHISTATUS to READY mode\n");
		mhi_dev_mmio_masked_write(dev, MHISTATUS,
				MHISTATUS_READY_MASK,
				MHISTATUS_READY_SHIFT, 1);

		mhi_dev_mmio_masked_write(dev, MHISTATUS,
				MHISTATUS_MHISTATE_MASK,
				MHISTATUS_MHISTATE_SHIFT, state);
		break;
	case MHI_DEV_SYSERR_STATE:
		MHI_SM_DBG("set MHISTATUS to SYSTEM ERROR mode\n");
		mhi_dev_mmio_masked_write(dev, MHISTATUS,
				MHISTATUS_SYSERR_MASK,
				MHISTATUS_SYSERR_SHIFT, 1);

		mhi_dev_mmio_masked_write(dev, MHISTATUS,
				MHISTATUS_MHISTATE_MASK,
				MHISTATUS_MHISTATE_SHIFT, state);
		break;
	case MHI_DEV_M1_STATE:
	case MHI_DEV_M2_STATE:
		MHI_SM_ERR("Not supported state, can't set MHISTATUS to %s\n",
			mhi_sm_mstate_str(state));
		goto exit;
	case MHI_DEV_M0_STATE:
	case MHI_DEV_M3_STATE:
		MHI_SM_DBG("set MHISTATUS.MHISTATE to %s state\n",
			mhi_sm_mstate_str(state));
		mhi_dev_mmio_masked_write(dev, MHISTATUS,
				MHISTATUS_MHISTATE_MASK,
				MHISTATUS_MHISTATE_SHIFT, state);
		break;
	default:
		MHI_SM_ERR("Invalid mhi state: 0x%x state", state);
		goto exit;
	}

	mhi_sm_ctx->mhi_state = state;

exit:
	MHI_SM_FUNC_EXIT();
}

/**
 * mhi_sm_is_legal_event_on_state() - Determine if MHI state transition is valid
 * @curr_state: current MHI state
 * @event: MHI state change event
 *
 * Determine according to MHI state management if the state change event
 * is valid on the current mhi state.
 * Note: The decision doesn't take into account M1 and M2 states.
 *
 * Return:	true: transition is valid
 *		false: transition is not valid
 */
static bool mhi_sm_is_legal_event_on_state(enum mhi_dev_state curr_state,
	enum mhi_dev_event event)
{
	bool res;

	switch (event) {
	case MHI_DEV_EVENT_M0_STATE:
		res = (mhi_sm_ctx->d_state == MHI_SM_EP_PCIE_D0_STATE &&
			curr_state != MHI_DEV_RESET_STATE);
		break;
	case MHI_DEV_EVENT_M3_STATE:
	case MHI_DEV_EVENT_HW_ACC_WAKEUP:
	case MHI_DEV_EVENT_CORE_WAKEUP:
		res = (curr_state == MHI_DEV_M3_STATE ||
			curr_state == MHI_DEV_M0_STATE);
		break;
	default:
		MHI_SM_ERR("Received invalid event: %s\n",
			mhi_sm_dev_event_str(event));
		res = false;
		break;
	}

	return res;
}

/**
 * mhi_sm_is_legal_pcie_event_on_state() - Determine if EP-PCIe linke state
 * transition is valid on the current system state.
 * @curr_mstate: current MHI state
 * @curr_dstate: current ep-pcie link, d, state
 * @event: ep-pcie link state change event
 *
 * Return:	true: transition is valid
 *		false: transition is not valid
 */
static bool mhi_sm_is_legal_pcie_event_on_state(enum mhi_dev_state curr_mstate,
	enum mhi_sm_ep_pcie_state curr_dstate, enum ep_pcie_event event)
{
	bool res;

	switch (event) {
	case EP_PCIE_EVENT_LINKUP:
	case EP_PCIE_EVENT_LINKDOWN:
		res = true;
		break;
	case EP_PCIE_EVENT_PM_D3_HOT:
		res = ((curr_mstate == MHI_DEV_M3_STATE ||
			curr_mstate == MHI_DEV_READY_STATE ||
			curr_mstate == MHI_DEV_RESET_STATE) &&
			curr_dstate != MHI_SM_EP_PCIE_LINK_DISABLE);
		break;
	case EP_PCIE_EVENT_PM_D3_COLD:
		res = (curr_dstate == MHI_SM_EP_PCIE_D3_HOT_STATE ||
			curr_dstate == MHI_SM_EP_PCIE_D3_COLD_STATE ||
			curr_dstate == MHI_SM_EP_PCIE_D0_STATE);
		break;
	case EP_PCIE_EVENT_PM_RST_DEAST:
		res = (curr_dstate == MHI_SM_EP_PCIE_D0_STATE ||
			curr_dstate == MHI_SM_EP_PCIE_D3_HOT_STATE ||
			curr_dstate == MHI_SM_EP_PCIE_D3_COLD_STATE);
		break;
	case EP_PCIE_EVENT_PM_D0:
		res = (curr_dstate == MHI_SM_EP_PCIE_D0_STATE ||
			curr_dstate == MHI_SM_EP_PCIE_D3_HOT_STATE);
		break;
	case EP_PCIE_EVENT_MHI_A7:
		res = true;
		break;
	default:
		MHI_SM_ERR("Invalid ep_pcie event, received: %s\n",
			mhi_sm_pcie_event_str(event));
		res = false;
		break;
	}

	return res;
}

/**
 * mhi_sm_change_to_M0() - switch to M0 state.
 *
 * Switch MHI-device state to M0, if possible according to MHI state machine.
 * Notify the MHI-host on the transition, in case MHI is suspended- resume MHI.
 *
 * Return:	0: success
 *		negative: failure
 */
static int mhi_sm_change_to_M0(void)
{
	enum mhi_dev_state old_state;
	struct ep_pcie_msi_config cfg;
	int res;

	MHI_SM_FUNC_ENTRY();

	old_state = mhi_sm_ctx->mhi_state;

	if (old_state == MHI_DEV_M0_STATE) {
		MHI_SM_DBG("Nothing to do, already in M0 state\n");
		res = 0;
		goto exit;
	} else if (old_state == MHI_DEV_M3_STATE ||
				old_state == MHI_DEV_READY_STATE) {
		/*  Retrieve MHI configuration*/
		res = mhi_dev_config_outbound_iatu(mhi_sm_ctx->mhi_dev);
		if (res) {
			MHI_SM_ERR("Fail to configure iATU, returned %d\n",
			res);
			goto exit;
		}
		res = ep_pcie_get_msi_config(mhi_sm_ctx->mhi_dev->phandle,
			&cfg);
		if (res) {
			MHI_SM_ERR("Error retrieving pcie msi logic\n");
			goto exit;
		}
		res = mhi_pcie_config_db_routing(mhi_sm_ctx->mhi_dev);
		if (res) {
			MHI_SM_ERR("Error configuring db routing\n");
			goto exit;

		}
	} else {
		MHI_SM_ERR("unexpected old_state: %s\n",
			mhi_sm_mstate_str(old_state));
		goto exit;
	}
	mhi_sm_mmio_set_mhistatus(MHI_DEV_M0_STATE);

	/* Tell the host, device move to M0 */
	res = mhi_dev_send_state_change_event(mhi_sm_ctx->mhi_dev,
				MHI_DEV_M0_STATE);
	if (res) {
		MHI_SM_ERR("Failed to send event %s to host, returned %d\n",
			mhi_sm_dev_event_str(MHI_DEV_EVENT_M0_STATE), res);
		goto exit;
	}

	if (old_state == MHI_DEV_READY_STATE) {
		/* Tell the host the EE */
		res = mhi_dev_send_ee_event(mhi_sm_ctx->mhi_dev, 2);
		if (res) {
			MHI_SM_ERR("failed sending EE event to host\n");
			goto exit;
		}
	} else if (old_state == MHI_DEV_M3_STATE) {
		/*Resuming MHI operation*/
		res = mhi_dev_resume(mhi_sm_ctx->mhi_dev);
		if (res) {
			MHI_SM_ERR("Failed resuming mhi core, returned %d",
				res);
			goto exit;
		}
		res = ipa_mhi_resume();
		if (res) {
			MHI_SM_ERR("Failed resuming ipa_mhi, returned %d",
				res);
			goto exit;
		}
	}
	res  = 0;

exit:
	MHI_SM_FUNC_EXIT();
	return res;
}

/**
 * mhi_sm_change_to_M3() - switch to M3 state
 *
 * Switch MHI-device state to M3, if possible according to MHI state machine.
 * Suspend MHI traffic and notify the host on the transition.
 *
 * Return:	0: success
 *		negative: failure
 */
static int mhi_sm_change_to_M3(void)
{
	enum mhi_dev_state old_state;
	int res = 0;

	MHI_SM_FUNC_ENTRY();

	old_state = mhi_sm_ctx->mhi_state;
	if (old_state == MHI_DEV_M3_STATE) {
		MHI_SM_DBG("Nothing to do, already in M3 state\n");
		res = 0;
		goto exit;
	}
	/* Suspending MHI operation*/
	res = mhi_dev_suspend(mhi_sm_ctx->mhi_dev);
	if (res) {
		MHI_SM_ERR("Failed to suspend mhi_core, returned %d\n", res);
		goto exit;
	}
	res = ipa_mhi_suspend(true);
	if (res) {
		MHI_SM_ERR("Failed to suspend ipa_mhi, returned %d\n", res);
		goto exit;
	}
	mhi_sm_mmio_set_mhistatus(MHI_DEV_M3_STATE);

	 /* tell the host, device move to M3 */
	res = mhi_dev_send_state_change_event(mhi_sm_ctx->mhi_dev,
				MHI_DEV_M3_STATE);
	if (res) {
		MHI_SM_ERR("Failed sendind event: %s to mhi_host\n",
			mhi_sm_dev_event_str(MHI_DEV_EVENT_M3_STATE));
		goto exit;
	}

exit:
	MHI_SM_FUNC_EXIT();
	return res;
}

/**
 * mhi_sm_wakeup_host() - wakeup MHI-host
 *@event: MHI state chenge event
 *
 * Sends wekup event to MHI-host via EP-PCIe, in case MHI is in M3 state.
 *
 * Return:	0:success
 *		negative: failure
 */
static int mhi_sm_wakeup_host(enum mhi_dev_event event)
{
	int res = 0;

	MHI_SM_FUNC_ENTRY();

	if (mhi_sm_ctx->mhi_state == MHI_DEV_M3_STATE) {
		/*
		  * ep_pcie driver is responsible to send the right wakeup
		  * event, assert WAKE#, according to Link state
		  */
		res = ep_pcie_wakeup_host(mhi_sm_ctx->mhi_dev->phandle);
		if (res) {
			MHI_SM_ERR("Failed to wakeup MHI host, returned %d\n",
				res);
			goto exit;
		}
	} else {
		MHI_SM_DBG("Nothing to do, Host is already awake\n");
	}

exit:
	MHI_SM_FUNC_EXIT();
	return res;
}

/**
 * mhi_sm_handle_syserr() - switch to system error state.
 *
 * Called on system error condition.
 * Switch MHI to SYSERR state, notify MHI-host and ASSERT on the device.
 * Synchronic function.
 *
 * Return:	0: success
 *		negative: failure
 */
static int mhi_sm_handle_syserr(void)
{
	int res;
	enum ep_pcie_link_status link_status;
	bool link_enabled = false;

	MHI_SM_FUNC_ENTRY();

	MHI_SM_ERR("Start handling SYSERR, MHI state: %s and %s",
		mhi_sm_mstate_str(mhi_sm_ctx->mhi_state),
		mhi_sm_dstate_str(mhi_sm_ctx->d_state));

	if (mhi_sm_ctx->mhi_state == MHI_DEV_SYSERR_STATE) {
		MHI_SM_DBG("Nothing to do, already in SYSERR state\n");
		return 0;
	}

	mhi_sm_ctx->syserr_occurred = true;
	link_status = ep_pcie_get_linkstatus(mhi_sm_ctx->mhi_dev->phandle);
	if (link_status == EP_PCIE_LINK_DISABLED) {
		/* try to power on ep-pcie, restore mmio, and wakup host */
		res = ep_pcie_enable_endpoint(mhi_sm_ctx->mhi_dev->phandle,
			EP_PCIE_OPT_POWER_ON);
		if (res) {
			MHI_SM_ERR("Failed to power on ep-pcie, returned %d\n",
				res);
			goto exit;
		}
		mhi_dev_restore_mmio(mhi_sm_ctx->mhi_dev);
		res = ep_pcie_enable_endpoint(mhi_sm_ctx->mhi_dev->phandle,
			EP_PCIE_OPT_AST_WAKE | EP_PCIE_OPT_ENUM);
		if (res) {
			MHI_SM_ERR("Failed to wakup host and enable ep-pcie\n");
			goto exit;
		}
	}

	link_enabled = true;
	mhi_sm_mmio_set_mhistatus(MHI_DEV_SYSERR_STATE);

	/* Tell the host, device move to SYSERR state */
	res = mhi_dev_send_state_change_event(mhi_sm_ctx->mhi_dev,
				MHI_DEV_SYSERR_STATE);
	if (res) {
		MHI_SM_ERR("Failed to send %s state change event to host\n",
			mhi_sm_mstate_str(MHI_DEV_SYSERR_STATE));
		goto exit;
	}

exit:
	if (!link_enabled)
		MHI_SM_ERR("EP-PCIE Link is disable cannot set MMIO to %s\n",
			mhi_sm_mstate_str(MHI_DEV_SYSERR_STATE));

	MHI_SM_ERR("/n/n/nASSERT ON DEVICE !!!!/n/n/n");
	WARN_ON();

	MHI_SM_FUNC_EXIT();
	return res;
}

/**
 * mhi_sm_dev_event_manager() - performs MHI state change
 * @work: work_struct used by the work queue
 *
 * This function is called from mhi_sm_wq, and performs mhi state change
 * if possible according to MHI state machine
 */
static void mhi_sm_dev_event_manager(struct work_struct *work)
{
	int res;
	struct mhi_sm_device_event *chg_event = container_of(work,
		struct mhi_sm_device_event, work);

	MHI_SM_FUNC_ENTRY();

	mutex_lock(&mhi_sm_ctx->mhi_state_lock);
	MHI_SM_DBG("Start handling %s event, current states: %s & %s\n",
		mhi_sm_dev_event_str(chg_event->event),
		mhi_sm_mstate_str(mhi_sm_ctx->mhi_state),
		mhi_sm_dstate_str(mhi_sm_ctx->d_state));

	if (mhi_sm_ctx->syserr_occurred) {
		MHI_SM_DBG("syserr occurred, Ignoring %s\n",
			mhi_sm_dev_event_str(chg_event->event));
		goto unlock_and_exit;
	}

	if (!mhi_sm_is_legal_event_on_state(mhi_sm_ctx->mhi_state,
		chg_event->event)) {
		MHI_SM_ERR("%s: illegal in current MHI state: %s and %s\n",
			mhi_sm_dev_event_str(chg_event->event),
			mhi_sm_mstate_str(mhi_sm_ctx->mhi_state),
			mhi_sm_dstate_str(mhi_sm_ctx->d_state));
		res = mhi_sm_handle_syserr();
		if (res)
			MHI_SM_ERR("Failed switching to SYSERR state\n");
		goto unlock_and_exit;
	}

	switch (chg_event->event) {
	case MHI_DEV_EVENT_M0_STATE:
		res = mhi_sm_change_to_M0();
		if (res)
			MHI_SM_ERR("Failed switching to M0 state\n");
		break;
	case MHI_DEV_EVENT_M3_STATE:
		res = mhi_sm_change_to_M3();
		if (res)
			MHI_SM_ERR("Failed switching to M3 state\n");
		break;
	case MHI_DEV_EVENT_HW_ACC_WAKEUP:
	case MHI_DEV_EVENT_CORE_WAKEUP:
		res = mhi_sm_wakeup_host(chg_event->event);
		if (res)
			MHI_SM_ERR("Failed to wakeup MHI host\n");
		break;
	case MHI_DEV_EVENT_CTRL_TRIG:
	case MHI_DEV_EVENT_M1_STATE:
	case MHI_DEV_EVENT_M2_STATE:
		MHI_SM_ERR("Error: %s event is not supported\n",
			mhi_sm_dev_event_str(chg_event->event));
		break;
	default:
		MHI_SM_ERR("Error: Invalid event, 0x%x", chg_event->event);
		break;
	}
unlock_and_exit:
	mutex_unlock(&mhi_sm_ctx->mhi_state_lock);
	atomic_dec(&mhi_sm_ctx->pending_device_events);
	kfree(chg_event);

	MHI_SM_FUNC_EXIT();
}

/**
 * mhi_sm_pcie_event_manager() - performs EP-PCIe linke state change
 * @work: work_struct used by the work queue
 *
 * This function is called from mhi_sm_wq, and performs ep-pcie link state
 * change if possible according to current system state and MHI state machine
 */
static void mhi_sm_pcie_event_manager(struct work_struct *work)
{
	int res;
	enum mhi_sm_ep_pcie_state old_dstate;
	struct mhi_sm_ep_pcie_event *chg_event = container_of(work,
		struct mhi_sm_ep_pcie_event, work);
	enum ep_pcie_event pcie_event = chg_event->event;

	MHI_SM_FUNC_ENTRY();

	mutex_lock(&mhi_sm_ctx->mhi_state_lock);
	old_dstate = mhi_sm_ctx->d_state;

	MHI_SM_DBG("Start handling %s event, current MHI state %s and %s\n",
		mhi_sm_pcie_event_str(chg_event->event),
		mhi_sm_mstate_str(mhi_sm_ctx->mhi_state),
		mhi_sm_dstate_str(old_dstate));

	if (mhi_sm_ctx->syserr_occurred &&
			pcie_event != EP_PCIE_EVENT_LINKDOWN) {
		MHI_SM_DBG("SYSERR occurred. Ignoring %s",
			mhi_sm_pcie_event_str(pcie_event));
		goto unlock_and_exit;
	}

	if (!mhi_sm_is_legal_pcie_event_on_state(mhi_sm_ctx->mhi_state,
		old_dstate, pcie_event)) {
		MHI_SM_ERR("%s: illegal in current MHI state: %s and %s\n",
			mhi_sm_pcie_event_str(pcie_event),
			mhi_sm_mstate_str(mhi_sm_ctx->mhi_state),
			mhi_sm_dstate_str(old_dstate));
		res = mhi_sm_handle_syserr();
		if (res)
			MHI_SM_ERR("Failed switching to SYSERR state\n");
		goto unlock_and_exit;
	}

	switch (pcie_event) {
	case EP_PCIE_EVENT_LINKUP:
		if (mhi_sm_ctx->d_state == MHI_SM_EP_PCIE_LINK_DISABLE)
			mhi_sm_ctx->d_state = MHI_SM_EP_PCIE_D0_STATE;
		break;
	case EP_PCIE_EVENT_LINKDOWN:
		res = mhi_sm_handle_syserr();
		if (res)
			MHI_SM_ERR("Failed switching to SYSERR state\n");
		goto unlock_and_exit;
	case EP_PCIE_EVENT_PM_D3_HOT:
		if (old_dstate == MHI_SM_EP_PCIE_D3_HOT_STATE) {
			MHI_SM_DBG("cannot move to D3_HOT from D3_COLD\n");
			break;
		}
		/* Backup MMIO is done on the callback function*/
		mhi_sm_ctx->d_state = MHI_SM_EP_PCIE_D3_HOT_STATE;
		break;
	case EP_PCIE_EVENT_PM_D3_COLD:
		if (old_dstate == MHI_SM_EP_PCIE_D3_COLD_STATE) {
			MHI_SM_DBG("Nothing to do, already in D3_COLD state\n");
			break;
		}
		ep_pcie_disable_endpoint(mhi_sm_ctx->mhi_dev->phandle);
		mhi_sm_ctx->d_state = MHI_SM_EP_PCIE_D3_COLD_STATE;
		break;
	case EP_PCIE_EVENT_PM_RST_DEAST:
		if (old_dstate == MHI_SM_EP_PCIE_D0_STATE) {
			MHI_SM_DBG("Nothing to do, already in D0 state\n");
			break;
		}
		res = ep_pcie_enable_endpoint(mhi_sm_ctx->mhi_dev->phandle,
			EP_PCIE_OPT_POWER_ON);
		if (res) {
			MHI_SM_ERR("Failed to power on ep_pcie, returned %d\n",
				res);
			goto unlock_and_exit;
		}

		mhi_dev_restore_mmio(mhi_sm_ctx->mhi_dev);

		res = ep_pcie_enable_endpoint(mhi_sm_ctx->mhi_dev->phandle,
			EP_PCIE_OPT_ENUM);
		if (res) {
			MHI_SM_ERR("ep-pcie failed to link train, return %d\n",
				res);
			goto unlock_and_exit;
		}
		mhi_sm_ctx->d_state = MHI_SM_EP_PCIE_D0_STATE;
		break;
	case EP_PCIE_EVENT_PM_D0:
		if (old_dstate == MHI_SM_EP_PCIE_D0_STATE) {
			MHI_SM_DBG("Nothing to do, already in D0 state\n");
			break;
		}
		mhi_sm_ctx->d_state = MHI_SM_EP_PCIE_D0_STATE;
		break;
	default:
		MHI_SM_ERR("Invalid EP_PCIE event, received 0x%x\n",
			pcie_event);
		break;
	}

unlock_and_exit:
	mutex_unlock(&mhi_sm_ctx->mhi_state_lock);
	atomic_dec(&mhi_sm_ctx->pending_pcie_events);
	kfree(chg_event);

	MHI_SM_FUNC_EXIT();
}

/**
 * mhi_dev_sm_init() - Initialize MHI state machine.
 * @mhi_dev: pointer to mhi device instance
 *
 * Assuming MHISTATUS register is in RESET state.
 *
 * Return:	0 success
 *		-EINVAL: invalid param
 *		-ENOMEM: allocating memory error
 */
int mhi_dev_sm_init(struct mhi_dev *mhi_dev)
{
	int res;
	enum ep_pcie_link_status link_state;

	MHI_SM_FUNC_ENTRY();

	if (!mhi_dev) {
		MHI_SM_ERR("Fail: Null argument\n");
		return -EINVAL;
	}

	mhi_sm_ctx = devm_kzalloc(mhi_dev->dev, sizeof(*mhi_sm_ctx),
		GFP_KERNEL);
	if (!mhi_sm_ctx) {
		MHI_SM_ERR("devm_kzalloc err: mhi_sm_ctx\n");
		return -ENOMEM;
	}

	/*init debugfs*/
	mhi_sm_debugfs_init();
	mhi_sm_ctx->mhi_sm_wq = create_singlethread_workqueue("mhi_sm_wq");
	if (!mhi_sm_ctx->mhi_sm_wq) {
		MHI_SM_ERR("Failed to create singlethread_workqueue: sm_wq\n");
		res = -ENOMEM;
		goto fail_init_wq;
	}

	mutex_init(&mhi_sm_ctx->mhi_state_lock);
	mhi_sm_ctx->mhi_dev = mhi_dev;
	mhi_sm_ctx->mhi_state = MHI_DEV_RESET_STATE;
	mhi_sm_ctx->syserr_occurred = false;
	atomic_set(&mhi_sm_ctx->pending_device_events, 0);
	atomic_set(&mhi_sm_ctx->pending_pcie_events, 0);

	link_state = ep_pcie_get_linkstatus(mhi_sm_ctx->mhi_dev->phandle);
	if (link_state == EP_PCIE_LINK_ENABLED)
		mhi_sm_ctx->d_state = MHI_SM_EP_PCIE_D0_STATE;
	else
		mhi_sm_ctx->d_state = MHI_SM_EP_PCIE_LINK_DISABLE;

	MHI_SM_FUNC_EXIT();
	return 0;

fail_init_wq:
	mhi_sm_ctx = NULL;
	mhi_sm_debugfs_destroy();
	return res;
}
EXPORT_SYMBOL(mhi_dev_sm_init);

/**
 * mhi_dev_sm_get_mhi_state() -Get current MHI state.
 * @state: return param
 *
 * Returns the current MHI state of the state machine.
 *
 * Return:	0 success
 *		-EINVAL: invalid param
 *		-EFAULT: state machine isn't initialized
 */
int mhi_dev_sm_get_mhi_state(enum mhi_dev_state *state)
{
	MHI_SM_FUNC_ENTRY();

	if (!state) {
		MHI_SM_ERR("Fail: Null argument\n");
		return -EINVAL;
	}
	if (!mhi_sm_ctx) {
		MHI_SM_ERR("Fail: MHI SM is not initialized\n");
		return -EFAULT;
	}
	*state = mhi_sm_ctx->mhi_state;
	MHI_SM_DBG("state machine states are: %s and %s\n",
		mhi_sm_mstate_str(*state),
		mhi_sm_dstate_str(mhi_sm_ctx->d_state));

	MHI_SM_FUNC_EXIT();
	return 0;
}
EXPORT_SYMBOL(mhi_dev_sm_get_mhi_state);

/**
 * mhi_dev_sm_set_ready() -Set MHI state to ready.
 *
 * Set MHISTATUS register in mmio to READY.
 * Synchronic function.
 *
 * Return:	0: success
 *		EINVAL: mhi state manager is not initialized
 *		EPERM: Operation not permitted as EP PCIE link is desable.
 *		EFAULT: MHI state is not RESET
 *		negative: other failure
 */
int mhi_dev_sm_set_ready(void)
{
	int res;
	int is_ready;
	enum mhi_dev_state state;

	MHI_SM_FUNC_ENTRY();

	if (!mhi_sm_ctx) {
		MHI_SM_ERR("Failed, MHI SM isn't initialized\n");
		return -EINVAL;
	}

	mutex_lock(&mhi_sm_ctx->mhi_state_lock);
	if (mhi_sm_ctx->mhi_state != MHI_DEV_RESET_STATE) {
		MHI_SM_ERR("Can not switch to READY state from %s state\n",
			mhi_sm_mstate_str(mhi_sm_ctx->mhi_state));
		res = -EFAULT;
		goto unlock_and_exit;
	}

	if (mhi_sm_ctx->d_state != MHI_SM_EP_PCIE_D0_STATE) {
		if (ep_pcie_get_linkstatus(mhi_sm_ctx->mhi_dev->phandle) ==
		    EP_PCIE_LINK_ENABLED) {
			mhi_sm_ctx->d_state = MHI_SM_EP_PCIE_D0_STATE;
		} else {
			MHI_SM_ERR("ERROR: ep-pcie link is not enabled\n");
			res = -EPERM;
			goto unlock_and_exit;
		}
	}

	/* verify that MHISTATUS is configured to RESET*/
	mhi_dev_mmio_masked_read(mhi_sm_ctx->mhi_dev,
		MHISTATUS, MHISTATUS_MHISTATE_MASK,
		MHISTATUS_MHISTATE_SHIFT, &state);

	mhi_dev_mmio_masked_read(mhi_sm_ctx->mhi_dev, MHISTATUS,
		MHISTATUS_READY_MASK,
		MHISTATUS_READY_SHIFT, &is_ready);

	if (state != MHI_DEV_RESET_STATE || is_ready) {
		MHI_SM_ERR("Cannot switch to READY, MHI is not in RESET state");
		MHI_SM_ERR("-MHISTATE: %s, READY bit: 0x%x\n",
			mhi_sm_mstate_str(state), is_ready);
		res = -EFAULT;
		goto unlock_and_exit;
	}
	mhi_sm_mmio_set_mhistatus(MHI_DEV_READY_STATE);

unlock_and_exit:
	mutex_unlock(&mhi_sm_ctx->mhi_state_lock);
	MHI_SM_FUNC_EXIT();
	return res;
}
EXPORT_SYMBOL(mhi_dev_sm_set_ready);

/**
 * mhi_dev_notify_sm_event() - MHI-core notify SM on trigger occurred
 * @event - enum of the requierd operation.
 *
 * Asynchronic function.
 * No trigger is sent after operation is done.
 *
 * Return:	0: success
 *		-EFAULT: SM isn't initialized or event isn't supported
 *		-ENOMEM: allocating memory error
 *		-EINVAL: invalied event
 */
int mhi_dev_notify_sm_event(enum mhi_dev_event event)
{
	struct mhi_sm_device_event *state_change_event;
	int res;

	MHI_SM_FUNC_ENTRY();

	if (!mhi_sm_ctx) {
		MHI_SM_ERR("Failed, MHI SM is not initialized\n");
		return -EFAULT;
	 }

	MHI_SM_DBG("received: %s\n",
		mhi_sm_dev_event_str(event));

	switch (event) {
	case MHI_DEV_EVENT_M0_STATE:
		mhi_sm_ctx->stats.m0_event_cnt++;
		break;
	case MHI_DEV_EVENT_M3_STATE:
		mhi_sm_ctx->stats.m3_event_cnt++;
		break;
	case MHI_DEV_EVENT_HW_ACC_WAKEUP:
		mhi_sm_ctx->stats.hw_acc_wakeup_event_cnt++;
		break;
	case MHI_DEV_EVENT_CORE_WAKEUP:
		mhi_sm_ctx->stats.mhi_core_wakeup_event_cnt++;
		break;
	case MHI_DEV_EVENT_CTRL_TRIG:
	case MHI_DEV_EVENT_M1_STATE:
	case MHI_DEV_EVENT_M2_STATE:
		MHI_SM_ERR("Not supported event: %s\n",
			mhi_sm_dev_event_str(event));
		res  = -EFAULT;
		goto exit;
	default:
		MHI_SM_ERR("Invalid event, received: 0x%x event\n", event);
		res =  -EINVAL;
		goto exit;
	}

	/*init work and push to queue*/
	state_change_event = kzalloc(sizeof(*state_change_event), GFP_ATOMIC);
	if (!state_change_event) {
		MHI_SM_ERR("kzalloc error\n");
		res = -ENOMEM;
		goto exit;
	}

	state_change_event->event = event;
	INIT_WORK(&state_change_event->work, mhi_sm_dev_event_manager);
	atomic_inc(&mhi_sm_ctx->pending_device_events);
	queue_work(mhi_sm_ctx->mhi_sm_wq, &state_change_event->work);
	res = 0;

exit:
	MHI_SM_FUNC_EXIT();
	return res;
}
EXPORT_SYMBOL(mhi_dev_notify_sm_event);

/**
 * mhi_dev_sm_pcie_handler() - handler of ep_pcie events
 * @notify - pointer to structure contains the ep_pcie event
 *
 * Callback function, called by ep_pcie driver to notify on pcie state change
 * Asynchronic function
 */
void mhi_dev_sm_pcie_handler(struct ep_pcie_notify *notify)
{
	struct mhi_sm_ep_pcie_event *dstate_change_evt;
	enum ep_pcie_event event;

	MHI_SM_FUNC_ENTRY();

	if (!notify) {
		MHI_SM_ERR("Null argument - notify\n");
		return;
	}

	if (!mhi_sm_ctx) {
		MHI_SM_ERR("Failed, MHI SM is not initialized\n");
		return;
	}

	event = notify->event;
	MHI_SM_DBG("received: %s\n",
		mhi_sm_pcie_event_str(event));

	dstate_change_evt = kzalloc(sizeof(*dstate_change_evt), GFP_ATOMIC);
	if (!dstate_change_evt) {
		MHI_SM_ERR("kzalloc error\n");
		goto exit;
	}

	switch (event) {
	case EP_PCIE_EVENT_LINKUP:
		mhi_sm_ctx->stats.linkup_event_cnt++;
		break;
	case EP_PCIE_EVENT_PM_D3_COLD:
		mhi_sm_ctx->stats.d3_cold_event_cnt++;
		break;
	case EP_PCIE_EVENT_PM_D3_HOT:
		mhi_sm_ctx->stats.d3_hot_event_cnt++;
		mhi_dev_backup_mmio(mhi_sm_ctx->mhi_dev);
		break;
	case EP_PCIE_EVENT_PM_RST_DEAST:
		mhi_sm_ctx->stats.rst_deast_event_cnt++;
		break;
	case EP_PCIE_EVENT_PM_D0:
		mhi_sm_ctx->stats.d0_event_cnt++;
		break;
	case EP_PCIE_EVENT_LINKDOWN:
		mhi_sm_ctx->stats.linkdown_event_cnt++;
		mhi_sm_ctx->syserr_occurred = true;
		MHI_SM_ERR("got %s, ERROR occurred\n",
			mhi_sm_pcie_event_str(event));
		break;
	case EP_PCIE_EVENT_MHI_A7:
		ep_pcie_mask_irq_event(mhi_sm_ctx->mhi_dev->phandle,
				EP_PCIE_INT_EVT_MHI_A7, false);
		mhi_dev_notify_a7_event(mhi_sm_ctx->mhi_dev);
		goto exit;
	default:
		MHI_SM_ERR("Invalid ep_pcie event, received 0x%x event\n",
			event);
		kfree(dstate_change_evt);
		goto exit;
	}

	dstate_change_evt->event = event;
	INIT_WORK(&dstate_change_evt->work, mhi_sm_pcie_event_manager);
	queue_work(mhi_sm_ctx->mhi_sm_wq, &dstate_change_evt->work);
	atomic_inc(&mhi_sm_ctx->pending_pcie_events);

exit:
	MHI_SM_FUNC_EXIT();
}
EXPORT_SYMBOL(mhi_dev_sm_pcie_handler);

/**
 * mhi_dev_sm_syserr() - switch to system error state.
 *
 * Called on system error condition.
 * Switch MHI to SYSERR state, notify MHI-host and ASSERT on the device.
 * Synchronic function.
 *
 * Return:	0: success
 *		negative: failure
 */
int mhi_dev_sm_syserr(void)
{
	int res;

	MHI_SM_FUNC_ENTRY();

	if (!mhi_sm_ctx) {
		MHI_SM_ERR("Failed, MHI SM is not initialized\n");
		return -EFAULT;
	}

	mutex_lock(&mhi_sm_ctx->mhi_state_lock);
	res = mhi_sm_handle_syserr();
	if (res)
		MHI_SM_ERR("mhi_sm_handle_syserr failed %d\n", res);
	mutex_unlock(&mhi_sm_ctx->mhi_state_lock);

	MHI_SM_FUNC_EXIT();
	return res;
}
EXPORT_SYMBOL(mhi_dev_sm_syserr);

static ssize_t mhi_sm_debugfs_read(struct file *file, char __user *ubuf,
				size_t count, loff_t *ppos)
{
	int nbytes = 0;

	if (!mhi_sm_ctx) {
		nbytes = scnprintf(dbg_buff, MHI_SM_MAX_MSG_LEN,
				"Not initialized\n");
	} else {
		nbytes += scnprintf(dbg_buff + nbytes,
			MHI_SM_MAX_MSG_LEN - nbytes,
			"*************** MHI State machine status ***************\n");
		nbytes += scnprintf(dbg_buff + nbytes,
			MHI_SM_MAX_MSG_LEN - nbytes,
			"D state: %s\n",
			mhi_sm_dstate_str(mhi_sm_ctx->d_state));
		nbytes += scnprintf(dbg_buff + nbytes,
			MHI_SM_MAX_MSG_LEN - nbytes,
			"M state: %s\n",
			mhi_sm_mstate_str(mhi_sm_ctx->mhi_state));
		nbytes += scnprintf(dbg_buff + nbytes,
			MHI_SM_MAX_MSG_LEN - nbytes,
			"pending device events: %d\n",
			atomic_read(&mhi_sm_ctx->pending_device_events));
		nbytes += scnprintf(dbg_buff + nbytes,
			MHI_SM_MAX_MSG_LEN - nbytes,
			"pending pcie events: %d\n",
			atomic_read(&mhi_sm_ctx->pending_pcie_events));
		nbytes += scnprintf(dbg_buff + nbytes,
			MHI_SM_MAX_MSG_LEN - nbytes,
			"*************** Statistics ***************\n");
		nbytes += scnprintf(dbg_buff + nbytes,
			MHI_SM_MAX_MSG_LEN - nbytes,
			"M0 events: %d\n", mhi_sm_ctx->stats.m0_event_cnt);
		nbytes += scnprintf(dbg_buff + nbytes,
			MHI_SM_MAX_MSG_LEN - nbytes,
			"M3 events: %d\n", mhi_sm_ctx->stats.m3_event_cnt);
		nbytes += scnprintf(dbg_buff + nbytes,
			MHI_SM_MAX_MSG_LEN - nbytes,
			"HW_ACC wakeup events: %d\n",
			mhi_sm_ctx->stats.hw_acc_wakeup_event_cnt);
		nbytes += scnprintf(dbg_buff + nbytes,
			MHI_SM_MAX_MSG_LEN - nbytes,
			"CORE wakeup events: %d\n",
			mhi_sm_ctx->stats.mhi_core_wakeup_event_cnt);
		nbytes += scnprintf(dbg_buff + nbytes,
			MHI_SM_MAX_MSG_LEN - nbytes,
			"Linkup events: %d\n",
			mhi_sm_ctx->stats.linkup_event_cnt);
		nbytes += scnprintf(dbg_buff + nbytes,
			MHI_SM_MAX_MSG_LEN - nbytes,
			"De-assert PERST events: %d\n",
			mhi_sm_ctx->stats.rst_deast_event_cnt);
		nbytes += scnprintf(dbg_buff + nbytes,
			MHI_SM_MAX_MSG_LEN - nbytes,
			"D0 events: %d\n",
			mhi_sm_ctx->stats.d0_event_cnt);
		nbytes += scnprintf(dbg_buff + nbytes,
			MHI_SM_MAX_MSG_LEN - nbytes,
			"D3_HOT events: %d\n",
			mhi_sm_ctx->stats.d3_hot_event_cnt);
		nbytes += scnprintf(dbg_buff + nbytes,
			MHI_SM_MAX_MSG_LEN - nbytes,
			"D3_COLD events:%d\n",
			mhi_sm_ctx->stats.d3_cold_event_cnt);
		nbytes += scnprintf(dbg_buff + nbytes,
			MHI_SM_MAX_MSG_LEN - nbytes,
			"Linkdown events: %d\n",
			mhi_sm_ctx->stats.linkdown_event_cnt);
	}

	return simple_read_from_buffer(ubuf, count, ppos, dbg_buff, nbytes);
}

static ssize_t mhi_sm_debugfs_write(struct file *file,
					const char __user *ubuf,
					size_t count,
					loff_t *ppos)
{
	unsigned long missing;
	s8 in_num = 0;

	if (!mhi_sm_ctx) {
		MHI_SM_ERR("Not initialized\n");
		return -EFAULT;
	}

	if (sizeof(dbg_buff) < count + 1)
		return -EFAULT;

	missing = copy_from_user(dbg_buff, ubuf, count);
	if (missing)
		return -EFAULT;

	dbg_buff[count] = '\0';
	if (kstrtos8(dbg_buff, 0, &in_num))
		return -EFAULT;

	switch (in_num) {
	case 0:
		if (atomic_read(&mhi_sm_ctx->pending_device_events) ||
			atomic_read(&mhi_sm_ctx->pending_pcie_events))
			MHI_SM_DBG("Note, there are pending events in sm_wq\n");

		memset(&mhi_sm_ctx->stats, 0, sizeof(struct mhi_sm_stats));
		break;
	default:
		MHI_SM_ERR("invalid argument: To reset statistics echo 0\n");
		break;
	}

	return count;
}
