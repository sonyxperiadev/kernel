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

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/of.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/types.h>
#include <linux/io.h>
#include <linux/of_irq.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/completion.h>
#include <linux/platform_device.h>
#include <linux/msm_ep_pcie.h>
#include <linux/ipa_mhi.h>
#include <linux/vmalloc.h>

#include "mhi.h"
#include "mhi_hwio.h"
#include "mhi_sm.h"

/* Wait time on the device for Host to set M0 state */
#define MHI_M0_WAIT_MIN_USLEEP		20000000
#define MHI_M0_WAIT_MAX_USLEEP		25000000
#define MHI_DEV_M0_MAX_CNT		30
/* Wait time before suspend/resume is complete */
#define MHI_SUSPEND_WAIT_MIN		3100
#define MHI_SUSPEND_WAIT_MAX		3200
#define MHI_SUSPEND_WAIT_TIMEOUT	500
#define MHI_MASK_CH_EV_LEN		32
#define MHI_RING_CMD_ID			0
#define MHI_RING_PRIMARY_EVT_ID		1
#define MHI_1K_SIZE			0x1000
/* Updated Specification for event start is NER - 2 and end - NER -1 */
#define MHI_HW_ACC_EVT_RING_START	2
#define MHI_HW_ACC_EVT_RING_END		1

#define MHI_HOST_REGION_NUM             2

#define MHI_MMIO_CTRL_INT_STATUS_A7_MSK	0x1
#define MHI_MMIO_CTRL_CRDB_STATUS_MSK	0x2

#define HOST_ADDR(lsb, msb)		((lsb) | ((uint64_t)(msb) << 32))
#define HOST_ADDR_LSB(addr)		(addr & 0xFFFFFFFF)
#define HOST_ADDR_MSB(addr)		((addr >> 32) & 0xFFFFFFFF)

#define MHI_IPC_LOG_PAGES		(100)
enum mhi_msg_level mhi_msg_lvl = MHI_MSG_ERROR;
enum mhi_msg_level mhi_ipc_msg_lvl = MHI_MSG_VERBOSE;
void *mhi_ipc_log;

static struct mhi_dev *mhi_ctx;
static void mhi_hwc_cb(void *priv, enum ipa_mhi_event_type event,
	unsigned long data);
static void mhi_ring_init_cb(void *user_data);

void mhi_dev_read_from_host(struct mhi_addr *host, dma_addr_t dev, size_t size)
{
	int rc = 0;
	uint64_t bit_40 = ((u64) 1) << 40, host_addr_pa = 0;

	host_addr_pa = ((u64) host->host_pa) | bit_40;

	mhi_log(MHI_MSG_ERROR, "device 0x%x <<-- host 0x%llx, size %d\n",
		dev, host_addr_pa, size);

	rc = ipa_dma_sync_memcpy((u64) dev, host_addr_pa, (int) size);
	if (rc)
		pr_err("error while reading from host:%d\n", rc);
}
EXPORT_SYMBOL(mhi_dev_read_from_host);

void mhi_dev_write_to_host(struct mhi_addr *host, void *dev, size_t size,
						struct mhi_dev *mhi)
{
	int rc = 0;
	uint64_t bit_40 = ((u64) 1) << 40, host_addr_pa = 0;

	if (!mhi) {
		pr_err("invalid MHI ctx\n");
		return;
	}

	host_addr_pa = ((u64) host->host_pa) | bit_40;
	/* Copy the device content to a local device physical address */
	memcpy(mhi->dma_cache, dev, size);
	mhi_log(MHI_MSG_ERROR, "device 0x%llx --> host 0x%llx, size %d\n",
		(uint64_t) mhi->cache_dma_handle, host_addr_pa, (int) size);

	rc = ipa_dma_sync_memcpy(host_addr_pa, (u64) mhi->cache_dma_handle,
								(int) size);
	if (rc)
		pr_err("error while reading from host:%d\n", rc);
}
EXPORT_SYMBOL(mhi_dev_write_to_host);

int mhi_transfer_host_to_device(void *dev, uint64_t host_pa, uint32_t len,
							struct mhi_dev *mhi)
{
	int rc = 0;
	uint64_t bit_40 = ((u64) 1) << 40, host_addr_pa = 0;

	if (!mhi) {
		pr_err("Invalid mhi device\n");
		return -EINVAL;
	}

	if (!dev) {
		pr_err("Invalid virt device\n");
		return -EINVAL;
	}

	if (!host_pa) {
		pr_err("Invalid host pa device\n");
		return -EINVAL;
	}

	host_addr_pa = host_pa | bit_40;
	mhi_log(MHI_MSG_ERROR, "device 0x%llx <-- host 0x%llx, size %d\n",
		(uint64_t) mhi->read_dma_handle, host_addr_pa, (int) len);
	rc = ipa_dma_sync_memcpy((u64) mhi->read_dma_handle,
			host_addr_pa, (int) len);
	if (rc) {
		pr_err("error while reading from host:%d\n", rc);
		return rc;
	}

	memcpy(dev, mhi->read_handle, len);

	return rc;
}
EXPORT_SYMBOL(mhi_transfer_host_to_device);

int mhi_transfer_device_to_host(uint64_t host_addr, void *dev, uint32_t len,
						struct mhi_dev *mhi)
{
	int rc = 0;
	uint64_t bit_40 = ((u64) 1) << 40, host_addr_pa = 0;

	if (!mhi || !dev || !host_addr) {
		pr_err("%sInvalid parameters\n", __func__);
		return -EINVAL;
	}

	host_addr_pa = host_addr | bit_40;
	memcpy(mhi->write_handle, dev, len);

	mhi_log(MHI_MSG_ERROR, "device 0x%llx ---> host 0x%llx, size %d\n",
		(uint64_t) mhi->write_dma_handle, host_addr_pa, (int) len);
	rc = ipa_dma_sync_memcpy(host_addr_pa,
			(u64) mhi->write_dma_handle,
			(int) len);
	if (rc)
		pr_err("error while reading from host:%d\n", rc);

	return rc;
}
EXPORT_SYMBOL(mhi_transfer_device_to_host);

int mhi_dev_is_list_empty(void)
{

	if (list_empty(&mhi_ctx->event_ring_list) &&
			list_empty(&mhi_ctx->process_ring_list))
		return 0;
	else
		return 1;
}
EXPORT_SYMBOL(mhi_dev_is_list_empty);

static void mhi_dev_get_erdb_db_cfg(struct mhi_dev *mhi,
				struct ep_pcie_db_config *erdb_cfg)
{
	switch (mhi->cfg.event_rings) {
	case NUM_CHANNELS:
		erdb_cfg->base = HW_CHANNEL_BASE;
		erdb_cfg->end = HW_CHANNEL_END;
		break;
	default:
		erdb_cfg->base = mhi->cfg.event_rings -
					MHI_HW_ACC_EVT_RING_START;
		erdb_cfg->end =  mhi->cfg.event_rings -
					MHI_HW_ACC_EVT_RING_END;
		break;
	}
}

int mhi_pcie_config_db_routing(struct mhi_dev *mhi)
{
	int rc = 0;
	struct ep_pcie_db_config chdb_cfg, erdb_cfg;

	if (!mhi) {
		pr_err("Invalid MHI context\n");
		return -EINVAL;
	}

	/* Configure Doorbell routing */
	chdb_cfg.base = HW_CHANNEL_BASE;
	chdb_cfg.end = HW_CHANNEL_END;
	chdb_cfg.tgt_addr = (uint32_t) mhi->ipa_uc_mbox_crdb;

	mhi_dev_get_erdb_db_cfg(mhi, &erdb_cfg);

	mhi_log(MHI_MSG_ERROR,
		"Event rings 0x%x => er_base 0x%x, er_end %d\n",
		mhi->cfg.event_rings, erdb_cfg.base, erdb_cfg.end);
	erdb_cfg.tgt_addr = (uint32_t) mhi->ipa_uc_mbox_erdb;
	ep_pcie_config_db_routing(mhi_ctx->phandle, chdb_cfg, erdb_cfg);

	return rc;
}
EXPORT_SYMBOL(mhi_pcie_config_db_routing);

static int mhi_hwc_init(struct mhi_dev *mhi)
{
	int rc = 0;
	struct ep_pcie_msi_config cfg;
	struct ipa_mhi_init_params ipa_init_params;
	struct ep_pcie_db_config erdb_cfg;

	/* Call IPA HW_ACC Init with MSI Address and db routing info */
	rc = ep_pcie_get_msi_config(mhi_ctx->phandle, &cfg);
	if (rc) {
		pr_err("Error retrieving pcie msi logic\n");
		return rc;
	}

	rc = mhi_pcie_config_db_routing(mhi);
	if (rc) {
		pr_err("Error configuring DB routing\n");
		return rc;
	}

	mhi_dev_get_erdb_db_cfg(mhi, &erdb_cfg);
	mhi_log(MHI_MSG_ERROR,
		"Event rings 0x%x => er_base 0x%x, er_end %d\n",
		mhi->cfg.event_rings, erdb_cfg.base, erdb_cfg.end);

	erdb_cfg.tgt_addr = (uint32_t) mhi->ipa_uc_mbox_erdb;
	memset(&ipa_init_params, 0, sizeof(ipa_init_params));
	ipa_init_params.msi.addr_hi = cfg.upper;
	ipa_init_params.msi.addr_low = cfg.lower;
	ipa_init_params.msi.data = cfg.data;
	ipa_init_params.msi.mask = ((1 << cfg.msg_num) - 1);
	ipa_init_params.first_er_idx = erdb_cfg.base;
	ipa_init_params.first_ch_idx = HW_CHANNEL_BASE;
	ipa_init_params.mmio_addr = ((uint32_t) mhi_ctx->mmio_base_pa_addr);
	ipa_init_params.assert_bit40 = true;

	mhi_log(MHI_MSG_ERROR,
		"MMIO Addr 0x%x, MSI config: U:0x%x L: 0x%x D: 0x%x\n",
		ipa_init_params.mmio_addr, cfg.upper, cfg.lower, cfg.data);
	ipa_init_params.notify = mhi_hwc_cb;
	ipa_init_params.priv = mhi;

	rc = ipa_mhi_init(&ipa_init_params);
	if (rc) {
		pr_err("Error initializing IPA\n");
		return rc;
	}

	return rc;
}

static int mhi_hwc_start(struct mhi_dev *mhi)
{
	int rc = 0;
	struct ipa_mhi_start_params ipa_start_params;

	memset(&ipa_start_params, 0, sizeof(ipa_start_params));

	ipa_start_params.channel_context_array_addr =
				mhi->ch_ctx_shadow.host_pa;
	ipa_start_params.event_context_array_addr =
				mhi->ev_ctx_shadow.host_pa;

	rc = ipa_mhi_start(&ipa_start_params);
	if (rc)
		pr_err("Error starting IPA (rc = 0x%X)\n", rc);

	return rc;
}

static void mhi_hwc_cb(void *priv, enum ipa_mhi_event_type event,
	unsigned long data)
{
	int rc = 0;

	switch (event) {
	case IPA_MHI_EVENT_READY:
		mhi_log(MHI_MSG_ERROR,
			"HW Channel uC is ready event=0x%X\n", event);
		rc = mhi_hwc_start(mhi_ctx);
		if (rc) {
			pr_err("hwc_init start failed with %d\n", rc);
			return;
		}

		rc = mhi_dev_mmio_enable_chdb_interrupts(mhi_ctx);
		if (rc) {
			pr_err("Failed to enable channel db\n");
			return;
		}

		rc = mhi_dev_mmio_enable_ctrl_interrupt(mhi_ctx);
		if (rc) {
			pr_err("Failed to enable control interrupt\n");
			return;
		}

		rc = mhi_dev_mmio_enable_cmdb_interrupt(mhi_ctx);

		if (rc) {
			pr_err("Failed to enable command db\n");
			return;
		}
		break;
	case IPA_MHI_EVENT_DATA_AVAILABLE:
		rc = mhi_dev_notify_sm_event(MHI_DEV_EVENT_HW_ACC_WAKEUP);
		if (rc) {
			pr_err("Event HW_ACC_WAKEUP failed with %d\n", rc);
			return;
		}
		break;
	default:
		pr_err("HW Channel uC unknown event 0x%X\n", event);
		break;
	}
}

static int mhi_hwc_chcmd(struct mhi_dev *mhi, uint chid,
				enum mhi_dev_ring_element_type_id type)
{
	int rc = 0;
	struct ipa_mhi_connect_params connect_params;

	memset(&connect_params, 0, sizeof(connect_params));

	switch (type) {
	case MHI_DEV_RING_EL_STOP:
		rc = ipa_mhi_disconnect_pipe(
			mhi->ipa_clnt_hndl[chid-HW_CHANNEL_BASE]);
		if (rc)
			pr_err("Stopping HW Channel%d failed 0x%X\n",
							chid, rc);
		break;
	case MHI_DEV_RING_EL_START:
		connect_params.channel_id = chid;
		connect_params.sys.skip_ep_cfg = true;
		if ((chid % 2) == 0x0)
			connect_params.sys.client = IPA_CLIENT_MHI_PROD;
		else
			connect_params.sys.client = IPA_CLIENT_MHI_CONS;

		rc = ipa_mhi_connect_pipe(&connect_params,
			&mhi->ipa_clnt_hndl[chid-HW_CHANNEL_BASE]);
		if (rc)
			pr_err("HW Channel%d start failed 0x%X\n",
							chid, rc);
		break;
	case MHI_DEV_RING_EL_INVALID:
	default:
		pr_err("Invalid Ring Element type = 0x%X\n", type);
		break;
	}

	return rc;
}

static void mhi_dev_core_ack_ctrl_interrupts(struct mhi_dev *dev,
							uint32_t *int_value)
{
	int rc = 0;

	rc = mhi_dev_mmio_read(dev, MHI_CTRL_INT_STATUS_A7, int_value);
	if (rc) {
		pr_err("Failed to read A7 status\n");
		return;
	}

	mhi_dev_mmio_write(dev, MHI_CTRL_INT_CLEAR_A7, *int_value);
	if (rc) {
		pr_err("Failed to clear A7 status\n");
		return;
	}
}

static void mhi_dev_fetch_ch_ctx(struct mhi_dev *mhi, uint32_t ch_id)
{
	struct mhi_addr addr;

	addr.host_pa = mhi->ch_ctx_shadow.host_pa +
					sizeof(struct mhi_dev_ch_ctx) * ch_id;
	addr.size  = sizeof(struct mhi_dev_ch_ctx);
	/* Fetch the channel ctx (*dst, *src, size) */
	mhi_dev_read_from_host(&addr, mhi->ch_ctx_cache_dma_handle +
					(sizeof(struct mhi_dev_ch_ctx) * ch_id),
				sizeof(struct mhi_dev_ch_ctx));
}

int mhi_dev_syserr(struct mhi_dev *mhi)
{

	if (!mhi) {
		pr_err("%s: Invalid MHI ctx\n", __func__);
		return -EINVAL;
	}

	mhi_dev_dump_mmio(mhi);
	pr_err("MHI dev sys error\n");

	return 0;
}
EXPORT_SYMBOL(mhi_dev_syserr);

int mhi_dev_send_event(struct mhi_dev *mhi, int evnt_ring,
					union mhi_dev_ring_element_type *el)
{
	int rc = 0;
	uint64_t evnt_ring_idx = mhi->ev_ring_start + evnt_ring;
	struct mhi_dev_ring *ring = &mhi->ring[evnt_ring_idx];
	union mhi_dev_ring_ctx *ctx;
	struct ep_pcie_msi_config cfg;
	struct mhi_addr msi_addr;
	uint32_t msi = 0;
	struct mhi_addr host_rp_addr;

	rc = ep_pcie_get_msi_config(mhi->phandle,
			&cfg);
		if (rc) {
			pr_err("Error retrieving pcie msi logic\n");
			return rc;
		}

	if (evnt_ring_idx > mhi->cfg.event_rings) {
		pr_err("Invalid event ring idx: %lld\n", evnt_ring_idx);
		return -EINVAL;
	}

	if (RING_STATE_UINT == mhi_ring_get_state(ring)) {
		ctx = (union mhi_dev_ring_ctx *)&mhi->ev_ctx_cache[evnt_ring];
		rc = mhi_ring_start(ring, ctx, mhi);
		if (rc) {
			mhi_log(MHI_MSG_ERROR,
				"error starting event ring %d\n", evnt_ring);
			return rc;
		}
	}

	mutex_lock(&mhi->mhi_event_lock);
	/* add the ring element */
	mhi_dev_add_element(ring, el);

	ring->ring_ctx_shadow->ev.rp =  (ring->rd_offset *
				sizeof(union mhi_dev_ring_element_type)) +
				ring->ring_ctx->generic.rbase;

	mhi_log(MHI_MSG_ERROR, "ev.rp = %llx for %lld\n",
				ring->ring_ctx_shadow->ev.rp, evnt_ring_idx);

	host_rp_addr.host_pa = (mhi->ev_ctx_shadow.host_pa +
			sizeof(struct mhi_dev_ev_ctx) *
			evnt_ring) + (uint32_t) &ring->ring_ctx->ev.rp -
			(uint32_t) ring->ring_ctx;
	mhi_dev_write_to_host(&host_rp_addr, &ring->ring_ctx_shadow->ev.rp,
						sizeof(uint64_t),
						mhi);

	/*
	 * rp update in host memory should be flushed
	 * before sending a MSI to the host
	 */
	wmb();

	mutex_unlock(&mhi->mhi_event_lock);
	mhi_log(MHI_MSG_ERROR, "event sent:\n");
	mhi_log(MHI_MSG_ERROR, "evnt ptr : 0x%llx\n", el->evt_tr_comp.ptr);
	mhi_log(MHI_MSG_ERROR, "evnt len : 0x%x\n", el->evt_tr_comp.len);
	mhi_log(MHI_MSG_ERROR, "evnt code :0x%x\n", el->evt_tr_comp.code);
	mhi_log(MHI_MSG_ERROR, "evnt type :0x%x\n", el->evt_tr_comp.type);
	mhi_log(MHI_MSG_ERROR, "evnt chid :0x%x\n", el->evt_tr_comp.chid);

	msi_addr.host_pa = (uint64_t)((uint64_t)cfg.upper << 32) |
						(uint64_t)cfg.lower;
	msi = cfg.data + mhi_ctx->mhi_ep_msi_num;
	mhi_log(MHI_MSG_ERROR, "Sending MSI %d to 0x%llx as data = 0x%x\n",
			mhi_ctx->mhi_ep_msi_num, msi_addr.host_pa, msi);
	mhi_dev_write_to_host(&msi_addr, &msi, 4, mhi);
	/*
	rc = ep_pcie_trigger_msi(mhi_ctx->phandle, mhi_ctx->mhi_ep_msi_num);
	if (rc) {
		pr_err("%s: error sending msi\n", __func__);
		return rc;
	}
	*/
	return rc;
}

static int mhi_dev_send_completion_event(struct mhi_dev_channel *ch,
			uint32_t rd_ofst, uint32_t len,
			enum mhi_dev_cmd_completion_code code)
{
	int rc = 0;
	union mhi_dev_ring_element_type compl_event;
	struct mhi_dev *mhi = ch->ring->mhi_dev;

	compl_event.evt_tr_comp.chid = ch->ch_id;
	compl_event.evt_tr_comp.type =
				MHI_DEV_RING_EL_TRANSFER_COMPLETION_EVENT;
	compl_event.evt_tr_comp.len = len;
	compl_event.evt_tr_comp.code = code;
	compl_event.evt_tr_comp.ptr = ch->ring->ring_ctx->generic.rbase +
			rd_ofst * sizeof(struct mhi_dev_transfer_ring_element);

	rc = mhi_dev_send_event(mhi,
			mhi->ch_ctx_cache[ch->ch_id].err_indx, &compl_event);

	return rc;
}

int mhi_dev_send_state_change_event(struct mhi_dev *mhi,
						enum mhi_dev_state state)
{
	union mhi_dev_ring_element_type event;
	int rc = 0;

	event.evt_state_change.type = MHI_DEV_RING_EL_MHI_STATE_CHG;
	event.evt_state_change.mhistate = state;

	rc = mhi_dev_send_event(mhi, 0, &event);
	if (rc) {
		pr_err("Sending state change event failed\n");
		return rc;
	}

	return rc;
}
EXPORT_SYMBOL(mhi_dev_send_state_change_event);

int mhi_dev_send_ee_event(struct mhi_dev *mhi, enum mhi_dev_execenv exec_env)
{
	union mhi_dev_ring_element_type event;
	int rc = 0;

	event.evt_ee_state.type = MHI_DEV_RING_EL_EE_STATE_CHANGE_NOTIFY;
	event.evt_ee_state.execenv = exec_env;

	rc = mhi_dev_send_event(mhi, 0, &event);
	if (rc) {
		pr_err("Sending EE change event failed\n");
		return rc;
	}

	return rc;
}
EXPORT_SYMBOL(mhi_dev_send_ee_event);

int mhi_dev_trigger_hw_acc_wakeup(struct mhi_dev *mhi)
{
	int rc = 0;

	/*
	 * Expected usuage is when there is HW ACC traffic IPA uC notifes
	 * Q6 -> IPA A7 -> MHI core -> MHI SM
	 */
	rc = mhi_dev_notify_sm_event(MHI_DEV_EVENT_HW_ACC_WAKEUP);
	if (rc) {
		pr_err("error sending SM event\n");
		return rc;
	}

	return rc;
}
EXPORT_SYMBOL(mhi_dev_trigger_hw_acc_wakeup);

static int mhi_dev_send_cmd_comp_event(struct mhi_dev *mhi)
{
	int rc = 0;
	union mhi_dev_ring_element_type event;

	/* send the command completion event to the host */
	event.evt_cmd_comp.ptr = mhi->cmd_ctx_cache->rbase
			+ (mhi->ring[MHI_RING_CMD_ID].rd_offset *
			(sizeof(union mhi_dev_ring_element_type)));
	mhi_log(MHI_MSG_ERROR, "evt cmd comp ptr :%d\n",
			(uint32_t) event.evt_cmd_comp.ptr);
	event.evt_cmd_comp.type = MHI_DEV_RING_EL_CMD_COMPLETION_EVT;
	event.evt_cmd_comp.code = MHI_CMD_COMPL_CODE_SUCCESS;

	rc = mhi_dev_send_event(mhi, 0, &event);
	if (rc)
		pr_err("channel start command faied\n");

	return rc;
}

static int mhi_dev_process_stop_cmd(struct mhi_dev_ring *ring, uint32_t ch_id,
							struct mhi_dev *mhi)
{
	int rc = 0;
	struct mhi_addr host_addr;

	if (ring->rd_offset != ring->wr_offset &&
		mhi->ch_ctx_cache[ch_id].ch_type ==
				MHI_DEV_CH_TYPE_OUTBOUND_CHANNEL) {
		mhi_log(MHI_MSG_INFO, "Pending transaction to be processed\n");
		return 0;
	} else if (mhi->ch_ctx_cache[ch_id].ch_type ==
			MHI_DEV_CH_TYPE_INBOUND_CHANNEL &&
			mhi->ch[ch_id].wr_request_active) {
		return 0;
	}

	/* set the channel to stop */
	mhi->ch_ctx_cache[ch_id].ch_state = MHI_DEV_CH_STATE_STOP;

	host_addr.host_pa = mhi->ch_ctx_shadow.host_pa +
				sizeof(struct mhi_dev_ch_ctx) * ch_id;
	/* update the channel state in the host */
	mhi_dev_write_to_host(&host_addr, &mhi->ch_ctx_cache[ch_id].ch_state,
				sizeof(enum mhi_dev_ch_ctx_state), mhi);

	/* send the completion event to the host */
	rc = mhi_dev_send_cmd_comp_event(mhi);
	if (rc)
		pr_err("Error sending command completion event\n");

	return rc;
}

static void mhi_dev_process_cmd_ring(struct mhi_dev *mhi,
			union mhi_dev_ring_element_type *el, void *ctx)
{
	int rc = 0;
	uint32_t ch_id = 0;
	union mhi_dev_ring_element_type event;
	struct mhi_addr host_addr;

	mhi_log(MHI_MSG_ERROR, "for channel:%d and cmd:%d\n",
		ch_id, el->generic.type);
	ch_id = el->generic.chid;

	switch (el->generic.type) {
	case MHI_DEV_RING_EL_START:
		mhi_log(MHI_MSG_ERROR, "recived start cmd for channel %d\n",
								ch_id);
		if (ch_id >= (HW_CHANNEL_BASE)) {
			rc = mhi_hwc_chcmd(mhi, ch_id, el->generic.type);
			if (rc) {
				pr_err("Error with HW channel cmd :%d\n", rc);
				return;
			}
			goto send_start_completion_event;
		}

		/* fetch the channel context from host */
		mhi_dev_fetch_ch_ctx(mhi, ch_id);

		/* Initialize and configure the corresponding channel ring */
		rc = mhi_ring_start(&mhi->ring[mhi->ch_ring_start + ch_id],
			(union mhi_dev_ring_ctx *)&mhi->ch_ctx_cache[ch_id],
			mhi);
		if (rc) {
			mhi_log(MHI_MSG_ERROR,
				"start ring failed for ch %d\n", ch_id);
			return;
		}

		mhi->ring[mhi->ch_ring_start + ch_id].state =
						RING_STATE_PENDING;

		/* set the channel to running */
		mhi->ch_ctx_cache[ch_id].ch_state = MHI_DEV_CH_STATE_RUNNING;
		mhi->ch[ch_id].ch_id = ch_id;
		mhi->ch[ch_id].ring = &mhi->ring[mhi->ch_ring_start + ch_id];
		mhi->ch[ch_id].ch_type = mhi->ch_ctx_cache[ch_id].ch_type;

		/* enable DB for event ring */
		rc = mhi_dev_mmio_enable_chdb_a7(mhi, ch_id);
		if (rc) {
			pr_err("Failed to enable channel db\n");
			return;
		}

		host_addr.host_pa = mhi->ch_ctx_shadow.host_pa +
					sizeof(struct mhi_dev_ch_ctx) * ch_id;
		mhi_dev_write_to_host(&host_addr,
					&mhi->ch_ctx_cache[ch_id].ch_state,
					sizeof(enum mhi_dev_ch_ctx_state), mhi);

send_start_completion_event:
		rc = mhi_dev_send_cmd_comp_event(mhi);
		if (rc)
			pr_err("Error sending command completion event\n");

		break;
	case MHI_DEV_RING_EL_STOP:
		if (ch_id >= HW_CHANNEL_BASE) {
			rc = mhi_hwc_chcmd(mhi, ch_id, el->generic.type);
			if (rc) {
				mhi_log(MHI_MSG_ERROR,
					"send channel stop cmd event failed\n");
				return;
			}

			/* send the completion event to the host */
			event.evt_cmd_comp.ptr = mhi->cmd_ctx_cache->rbase +
				(mhi->ring[MHI_RING_CMD_ID].rd_offset *
				(sizeof(union mhi_dev_ring_element_type)));
			event.evt_cmd_comp.type =
					MHI_DEV_RING_EL_CMD_COMPLETION_EVT;
			if (rc == 0)
				event.evt_cmd_comp.code =
					MHI_CMD_COMPL_CODE_SUCCESS;
			else
				event.evt_cmd_comp.code =
					MHI_CMD_COMPL_CODE_UNDEFINED;

			rc = mhi_dev_send_event(mhi, 0, &event);
			if (rc) {
				pr_err("stop event send failed\n");
				return;
			}
		} else {
			/*
			 * Check if there are any pending transactions for the
			 * ring associated with the channel. If no, proceed to
			 * write disable the channel state else send stop
			 * channel command to check if one can suspend the
			 * command.
			 */
			mhi->ch[ch_id].state = MHI_DEV_CH_PENDING_STOP;
			rc = mhi_dev_process_stop_cmd(
				&mhi->ring[mhi->ch_ring_start + ch_id],
				ch_id, mhi);
			if (rc) {
				pr_err("stop event send failed\n");
				return;
			}
		}
		break;
	case MHI_DEV_RING_EL_RESET:
		/* hard stop and set the channel to stop */
		mhi->ch_ctx_cache[ch_id].ch_state = MHI_DEV_CH_STATE_STOP;
		host_addr.host_pa = mhi->ch_ctx_shadow.host_pa +
				sizeof(struct mhi_dev_ch_ctx) * ch_id;

		/* update the channel state in the host */
		mhi_dev_write_to_host(&host_addr,
				&mhi->ch_ctx_cache[ch_id].ch_state,
				sizeof(enum mhi_dev_ch_ctx_state), mhi);

		/* send the completion event to the host */
		rc = mhi_dev_send_cmd_comp_event(mhi);
		if (rc)
			pr_err("Error sending command completion event\n");
		break;
	default:
		pr_err("%s: Invalid command:%d\n", __func__, el->generic.type);
		break;
	}
}

static void mhi_dev_process_tre_ring(struct mhi_dev *mhi,
			union mhi_dev_ring_element_type *el, void *ctx)
{
	struct mhi_dev_ring *ring = (struct mhi_dev_ring *)ctx;
	struct mhi_dev_channel *ch;
	struct mhi_dev_client_cb_reason reason;

	if (ring->id < mhi->ch_ring_start) {
		mhi_log(MHI_MSG_ERROR,
			"invalid channel ring id (%d), should be < %d\n",
			ring->id, mhi->ch_ring_start);
		return;
	}

	ch = &mhi->ch[ring->id - mhi->ch_ring_start];
	reason.ch_id = ch->ch_id;
	reason.reason = MHI_DEV_TRE_AVAILABLE;

	/* Invoke a callback to let the client know its data is ready.
	 * Copy this event to the clients context so that it can be
	 * sent out once the client has fetch the data. Update the rp
	 * before sending the data as part of the event completion
	 */
	if (ch->active_client && ch->active_client->event_trigger != NULL)
		ch->active_client->event_trigger(&reason);
}

static void mhi_dev_process_ring_pending(struct work_struct *work)
{
	struct mhi_dev *mhi = container_of(work,
				struct mhi_dev, pending_work);
	struct list_head *cp, *q;
	struct mhi_dev_ring *ring;
	struct mhi_dev_channel *ch;
	int rc = 0;

	mutex_lock(&mhi_ctx->mhi_lock);
	rc = mhi_dev_process_ring(&mhi->ring[mhi->cmd_ring_idx]);
	if (rc) {
		mhi_log(MHI_MSG_ERROR, "error processing command ring\n");
		goto exit;
	}

	list_for_each_safe(cp, q, &mhi->process_ring_list) {
		ring = list_entry(cp, struct mhi_dev_ring, list);
		list_del(cp);
		mhi_log(MHI_MSG_ERROR, "processing ring %d\n", ring->id);
		rc = mhi_dev_process_ring(ring);
		if (rc) {
			mhi_log(MHI_MSG_ERROR,
				"error processing ring %d\n", ring->id);
			goto exit;
		}

		if (ring->id < mhi->ch_ring_start) {
			mhi_log(MHI_MSG_ERROR,
				"ring (%d) is not a channel ring\n", ring->id);
			goto exit;
		}

		ch = &mhi->ch[ring->id - mhi->ch_ring_start];
		rc = mhi_dev_mmio_enable_chdb_a7(mhi, ch->ch_id);
		if (rc) {
			mhi_log(MHI_MSG_ERROR,
			"error enabling chdb interrupt for %d\n", ch->ch_id);
			goto exit;
		}
	}

exit:
	mutex_unlock(&mhi_ctx->mhi_lock);
}

static int mhi_dev_get_event_notify(enum mhi_dev_state state,
						enum mhi_dev_event *event)
{
	int rc = 0;

	switch (state) {
	case MHI_DEV_M0_STATE:
		*event = MHI_DEV_EVENT_M0_STATE;
		break;
	case MHI_DEV_M1_STATE:
		*event = MHI_DEV_EVENT_M1_STATE;
		break;
	case MHI_DEV_M2_STATE:
		*event = MHI_DEV_EVENT_M2_STATE;
		break;
	case MHI_DEV_M3_STATE:
		*event = MHI_DEV_EVENT_M3_STATE;
		break;
	default:
		rc = -EINVAL;
		break;
	}

	return rc;
}

static void mhi_dev_queue_channel_db(struct mhi_dev *mhi,
					uint32_t chintr_value, uint32_t ch_num)
{
	struct mhi_dev_ring *ring;
	int rc = 0;

	for (; chintr_value; ch_num++, chintr_value >>= 1) {
		if (chintr_value & 1) {
			ring = &mhi->ring[ch_num + mhi->ch_ring_start];
			if (ring->state == RING_STATE_UINT) {
				pr_err("Channel not opened for %d\n", ch_num);
				break;
			}
			mhi_ring_set_state(ring, RING_STATE_PENDING);
			list_add(&ring->list, &mhi->process_ring_list);
			rc = mhi_dev_mmio_disable_chdb_a7(mhi, ch_num);
			if (rc) {
				pr_err("Error disabling chdb\n");
				return;
			}
			queue_work(mhi->pending_ring_wq, &mhi->pending_work);
		}
	}
}

static void mhi_dev_check_channel_interrupt(struct mhi_dev *mhi)
{
	int i, rc = 0;
	uint32_t chintr_value = 0, ch_num = 0;

	rc = mhi_dev_mmio_read_chdb_status_interrupts(mhi);
	if (rc) {
		pr_err("Read channel db\n");
		return;
	}

	for (i = 0; i < MHI_MASK_ROWS_CH_EV_DB; i++) {
		ch_num = i * MHI_MASK_CH_EV_LEN;
		chintr_value = mhi->chdb[i].status;
		if (chintr_value) {
			mhi_log(MHI_MSG_ERROR,
				"processing id: %d, ch interrupt 0x%x\n",
							i, chintr_value);
			mhi_dev_queue_channel_db(mhi, chintr_value, ch_num);
			rc = mhi_dev_mmio_write(mhi, MHI_CHDB_INT_CLEAR_A7_n(i),
							mhi->chdb[i].status);
			if (rc) {
				pr_err("Error writing interrupt clear for A7\n");
				return;
			}
		}
	}
}

static void mhi_dev_scheduler(struct work_struct *work)
{
	struct mhi_dev *mhi = container_of(work,
				struct mhi_dev, chdb_ctrl_work);
	int rc = 0;
	uint32_t int_value = 0;
	struct mhi_dev_ring *ring;
	enum mhi_dev_state state;
	enum mhi_dev_event event = 0;

	mutex_lock(&mhi_ctx->mhi_lock);
	/* Check for interrupts */
	mhi_dev_core_ack_ctrl_interrupts(mhi, &int_value);

	if (int_value & MHI_MMIO_CTRL_INT_STATUS_A7_MSK) {
		mhi_log(MHI_MSG_ERROR,
			"processing ctrl interrupt with %d\n", int_value);
		rc = mhi_dev_mmio_get_mhi_state(mhi, &state);
		if (rc) {
			pr_err("%s: get mhi state failed\n", __func__);
			mutex_unlock(&mhi_ctx->mhi_lock);
			return;
		}

		rc = mhi_dev_get_event_notify(state, &event);
		if (rc) {
			pr_err("unsupported state :%d\n", state);
			mutex_unlock(&mhi_ctx->mhi_lock);
			return;
		}

		rc = mhi_dev_notify_sm_event(event);
		if (rc) {
			pr_err("error sending SM event\n");
			mutex_unlock(&mhi_ctx->mhi_lock);
			return;
		}
	}

	if (int_value & MHI_MMIO_CTRL_CRDB_STATUS_MSK) {
		mhi_log(MHI_MSG_ERROR,
			"processing cmd db interrupt with %d\n", int_value);
		ring = &mhi->ring[MHI_RING_CMD_ID];
		ring->state = RING_STATE_PENDING;
		queue_work(mhi->pending_ring_wq, &mhi->pending_work);
	}

	/* get the specific channel interrupts */
	mhi_dev_check_channel_interrupt(mhi);

	mutex_unlock(&mhi_ctx->mhi_lock);
	ep_pcie_mask_irq_event(mhi->phandle,
				EP_PCIE_INT_EVT_MHI_A7, true);
}

void mhi_dev_notify_a7_event(struct mhi_dev *mhi)
{
	schedule_work(&mhi->chdb_ctrl_work);
	mhi_log(MHI_MSG_ERROR, "mhi irq triggered\n");
}
EXPORT_SYMBOL(mhi_dev_notify_a7_event);

int mhi_dev_config_outbound_iatu(struct mhi_dev *mhi)
{
	return 0;
}
EXPORT_SYMBOL(mhi_dev_config_outbound_iatu);

static int mhi_dev_cache_host_cfg(struct mhi_dev *mhi)
{
	int rc = 0;
	struct platform_device *pdev;
	uint64_t addr1 = 0;

	pdev = mhi->pdev;

	/* Get host memory region configuration */
	mhi_dev_get_mhi_addr(mhi);

	mhi->ctrl_base.host_pa  = HOST_ADDR(mhi->host_addr.ctrl_base_lsb,
						mhi->host_addr.ctrl_base_msb);
	mhi->data_base.host_pa  = HOST_ADDR(mhi->host_addr.data_base_lsb,
						mhi->host_addr.data_base_msb);

	addr1 = HOST_ADDR(mhi->host_addr.ctrl_limit_lsb,
					mhi->host_addr.ctrl_limit_msb);
	mhi->ctrl_base.size = addr1 - mhi->ctrl_base.host_pa;
	addr1 = HOST_ADDR(mhi->host_addr.data_limit_lsb,
					mhi->host_addr.data_limit_msb);
	mhi->data_base.size = addr1 - mhi->data_base.host_pa;

	/* Get Channel, event and command context base pointer */
	rc = mhi_dev_mmio_get_chc_base(mhi);
	if (rc) {
		pr_err("Fetching channel context failed\n");
		return rc;
	}

	rc = mhi_dev_mmio_get_erc_base(mhi);
	if (rc) {
		pr_err("Fetching event ring context failed\n");
		return rc;
	}

	rc = mhi_dev_mmio_get_crc_base(mhi);
	if (rc) {
		pr_err("Fetching command ring context failed\n");
		return rc;
	}

	rc = mhi_dev_update_ner(mhi);
	if (rc) {
		pr_err("Fetching NER failed\n");
		return rc;
	}

	mhi->cmd_ctx_shadow.size = sizeof(struct mhi_dev_cmd_ctx);
	mhi->ev_ctx_shadow.size = sizeof(struct mhi_dev_ev_ctx) *
					mhi->cfg.event_rings;
	mhi->ch_ctx_shadow.size = sizeof(struct mhi_dev_ch_ctx) *
					mhi->cfg.channels;

	mhi->cmd_ctx_cache = dma_alloc_coherent(&pdev->dev,
				sizeof(struct mhi_dev_cmd_ctx),
				&mhi->cmd_ctx_cache_dma_handle,
				GFP_KERNEL);
	if (!mhi->cmd_ctx_cache) {
		pr_err("no memory while allocating cmd ctx\n");
		return -ENOMEM;
	}
	memset(mhi->cmd_ctx_cache, 0, sizeof(struct mhi_dev_cmd_ctx));

	mhi->ev_ctx_cache = dma_alloc_coherent(&pdev->dev,
				sizeof(struct mhi_dev_ev_ctx) *
				mhi->cfg.event_rings,
				&mhi->ev_ctx_cache_dma_handle,
				GFP_KERNEL);
	if (!mhi->ev_ctx_cache)
		return -ENOMEM;

	mhi->ch_ctx_cache = dma_alloc_coherent(&pdev->dev,
				sizeof(struct mhi_dev_ch_ctx) *
				mhi->cfg.channels,
				&mhi->ch_ctx_cache_dma_handle,
				GFP_KERNEL);
	if (!mhi_ctx->ch_ctx_cache)
		return -ENOMEM;

	/* Cache the command and event context */
	mhi_dev_read_from_host(&mhi->cmd_ctx_shadow,
				mhi->cmd_ctx_cache_dma_handle,
				mhi->cmd_ctx_shadow.size);

	mhi_dev_read_from_host(&mhi->ev_ctx_shadow,
				mhi->ev_ctx_cache_dma_handle,
				mhi->ev_ctx_shadow.size);

	mhi_log(MHI_MSG_ERROR,
			"cmd ring_base:0x%llx, rp:0x%llx, wp:0x%llx\n",
					mhi->cmd_ctx_cache->rbase,
					mhi->cmd_ctx_cache->rp,
					mhi->cmd_ctx_cache->wp);
	mhi_log(MHI_MSG_ERROR,
			"ev ring_base:0x%llx, rp:0x%llx, wp:0x%llx\n",
					mhi_ctx->ev_ctx_cache->rbase,
					mhi->ev_ctx_cache->rp,
					mhi->ev_ctx_cache->wp);

	rc = mhi_ring_start(&mhi->ring[0],
			(union mhi_dev_ring_ctx *)mhi->cmd_ctx_cache, mhi);
	if (rc) {
		pr_err("error in ring start\n");
		return rc;
	}

	return 0;
}

int mhi_dev_suspend(struct mhi_dev *mhi)
{
	int ch_id = 0, rc = 0;
	struct mhi_addr host_addr;

	mutex_lock(&mhi_ctx->mhi_write_test);
	atomic_set(&mhi->is_suspended, 1);

	for (ch_id = 0; ch_id < mhi->cfg.channels; ch_id++) {
		if (mhi->ch_ctx_cache[ch_id].ch_state !=
						MHI_DEV_CH_STATE_RUNNING)
			continue;

		mhi->ch_ctx_cache[ch_id].ch_state = MHI_DEV_CH_STATE_SUSPENDED;

		host_addr.host_pa = mhi->ch_ctx_shadow.host_pa +
				sizeof(struct mhi_dev_ch_ctx) * ch_id;

		/* update the channel state in the host */
		mhi_dev_write_to_host(&host_addr,
			&mhi->ch_ctx_cache[ch_id].ch_state,
			sizeof(enum mhi_dev_ch_ctx_state), mhi);

	}

	rc = ipa_dma_disable();
	if (rc)
		pr_err("Disable IPA failed\n");

	mutex_unlock(&mhi_ctx->mhi_write_test);

	return rc;
}
EXPORT_SYMBOL(mhi_dev_suspend);

int mhi_dev_resume(struct mhi_dev *mhi)
{
	int ch_id = 0, rc = 0;
	struct mhi_addr host_addr;

	rc = ipa_dma_enable();
	if (rc) {
		pr_err("IPA enable failed\n");
		return rc;
	}

	for (ch_id = 0; ch_id < mhi->cfg.channels; ch_id++) {
		if (mhi->ch_ctx_cache[ch_id].ch_state !=
				MHI_DEV_CH_STATE_SUSPENDED)
			continue;

		mhi->ch_ctx_cache[ch_id].ch_state = MHI_DEV_CH_STATE_RUNNING;
		host_addr.host_pa = mhi->ch_ctx_shadow.host_pa +
				sizeof(struct mhi_dev_ch_ctx) * ch_id;

		/* update the channel state in the host */
		mhi_dev_write_to_host(&host_addr,
				&mhi->ch_ctx_cache[ch_id].ch_state,
				sizeof(enum mhi_dev_ch_ctx_state), mhi);
	}

	atomic_set(&mhi->is_suspended, 0);

	return rc;
}
EXPORT_SYMBOL(mhi_dev_resume);

static int mhi_dev_ring_init(struct mhi_dev *dev)
{
	int i = 0;

	mhi_log(MHI_MSG_INFO, "initializing all rings");
	dev->cmd_ring_idx = 0;
	dev->ev_ring_start = 1;
	dev->ch_ring_start = dev->ev_ring_start + dev->cfg.event_rings;

	/* Initialize CMD ring */
	mhi_ring_init(&dev->ring[dev->cmd_ring_idx],
				RING_TYPE_CMD, dev->cmd_ring_idx);

	mhi_ring_set_cb(&dev->ring[dev->cmd_ring_idx],
				mhi_dev_process_cmd_ring);

	/* Initialize Event ring */
	for (i = dev->ev_ring_start; i < (dev->cfg.event_rings
					+ dev->ev_ring_start); i++)
		mhi_ring_init(&dev->ring[i], RING_TYPE_ER, i);

	/* Initialize CH */
	for (i = dev->ch_ring_start; i < (dev->cfg.channels
					+ dev->ch_ring_start); i++) {
		mhi_ring_init(&dev->ring[i], RING_TYPE_CH, i);
		mhi_ring_set_cb(&dev->ring[i], mhi_dev_process_tre_ring);
	}


	return 0;
}

int mhi_dev_open_channel(uint32_t chan_id,
			struct mhi_dev_client **handle_client,
			void (*mhi_dev_client_cb_reason)
			(struct mhi_dev_client_cb_reason *cb))
{
	int rc = 0;
	struct mhi_dev_channel *ch;
	struct platform_device *pdev;

	pdev = mhi_ctx->pdev;
	ch = &mhi_ctx->ch[chan_id];

	mutex_lock(&ch->ch_lock);

	if (ch->active_client) {
		mhi_log(MHI_MSG_ERROR,
			"Channel (%d) already opened by client\n", chan_id);
		rc = -EINVAL;
		goto exit;
	}

	/* Initialize the channel, client and state information */
	*handle_client = kzalloc(sizeof(struct mhi_dev_client), GFP_KERNEL);
	if (!(*handle_client)) {
		dev_err(&pdev->dev, "can not allocate mhi_dev memory\n");
		rc = -ENOMEM;
		goto exit;
	}

	ch->active_client = (*handle_client);
	(*handle_client)->channel = ch;
	(*handle_client)->event_trigger = mhi_dev_client_cb_reason;

	if (ch->state == MHI_DEV_CH_UNINT) {
		ch->ring = &mhi_ctx->ring[chan_id + mhi_ctx->ch_ring_start];
		ch->state = MHI_DEV_CH_PENDING_START;
	} else if (ch->state == MHI_DEV_CH_CLOSED)
		ch->state = MHI_DEV_CH_STARTED;
	else if (ch->state == MHI_DEV_CH_STOPPED)
		ch->state = MHI_DEV_CH_PENDING_START;

exit:
	mutex_unlock(&ch->ch_lock);
	return rc;
}
EXPORT_SYMBOL(mhi_dev_open_channel);

int mhi_dev_channel_isempty(struct mhi_dev_client *handle)
{
	struct mhi_dev_channel *ch;
	int rc;

	ch = handle->channel;

	rc = ch->ring->rd_offset == ch->ring->wr_offset;

	return rc;
}
EXPORT_SYMBOL(mhi_dev_channel_isempty);

int mhi_dev_close_channel(struct mhi_dev_client *handle)
{
	struct mhi_dev_channel *ch;
	int rc = 0;

	ch = handle->channel;

	mutex_lock(&ch->ch_lock);
	if (ch->state != MHI_DEV_CH_PENDING_START) {
		if (ch->ch_type == MHI_DEV_CH_TYPE_OUTBOUND_CHANNEL &&
					!mhi_dev_channel_isempty(handle)) {
			mhi_log(MHI_MSG_ERROR,
				"Trying to close an active channel (%d)\n",
				ch->ch_id);
			mutex_unlock(&ch->ch_lock);
			rc = -EAGAIN;
			goto exit;
		} else if (ch->tre_loc) {
			mhi_log(MHI_MSG_ERROR,
				"Trying to close channel (%d) when a TRE is active",
				ch->ch_id);
			mutex_unlock(&ch->ch_lock);
			rc = -EAGAIN;
			goto exit;
		}
	}

	ch->state = MHI_DEV_CH_CLOSED;
	ch->active_client = NULL;
	kfree(handle);
exit:
	mutex_unlock(&ch->ch_lock);
	return rc;
}
EXPORT_SYMBOL(mhi_dev_close_channel);

static int mhi_dev_check_tre_bytes_left(struct mhi_dev_channel *ch,
		struct mhi_dev_ring *ring, union mhi_dev_ring_element_type *el,
		uint32_t *chain)
{
	uint32_t td_done = 0;

	/*
	 * A full TRE worth of data was consumed.
	 * Check if we are at a TD boundary.
	 */
	if (ch->tre_bytes_left == 0) {
		if (el->tre.chain) {
			if (el->tre.ieob)
				mhi_dev_send_completion_event(ch,
					ring->rd_offset, el->tre.len,
					MHI_CMD_COMPL_CODE_EOB);
				*chain = 1;
		} else {
			if (el->tre.ieot)
				mhi_dev_send_completion_event(
					ch, ring->rd_offset, el->tre.len,
					MHI_CMD_COMPL_CODE_EOT);
				td_done = 1;
				*chain = 0;
		}
		mhi_dev_ring_inc_index(ring, ring->rd_offset);
		ch->tre_bytes_left = 0;
		ch->tre_loc = 0;
	}

	return td_done;
}

int mhi_dev_read_channel(struct mhi_dev_client *handle_client,
				void *buf, uint32_t buf_size, uint32_t *chain)
{
	struct mhi_dev_channel *ch;
	struct mhi_dev_ring *ring;
	union mhi_dev_ring_element_type *el;
	uint32_t ch_id;
	size_t bytes_to_read, addr_offset;
	uint64_t read_from_loc;
	ssize_t bytes_read = 0;
	uint32_t write_to_loc = 0;
	size_t usr_buf_remaining = buf_size;
	int td_done = 0, rc = 0;

	if (!handle_client) {
		mhi_log(MHI_MSG_ERROR, "invalid client handle\n");
		return -ENXIO;
	}

	ch = handle_client->channel;
	ring = ch->ring;
	ch_id = ch->ch_id;
	*chain = 0;

	mutex_lock(&ch->ch_lock);

	do {
		el = &ring->ring_cache[ring->rd_offset];
		if (ch->tre_loc) {
			bytes_to_read = min(usr_buf_remaining,
						ch->tre_bytes_left);
			*chain = 1;
			mhi_log(MHI_MSG_ERROR,
				"remaining buffered data size %d\n",
				(int) ch->tre_bytes_left);
		} else {
			if (ring->rd_offset == ring->wr_offset) {
				mhi_log(MHI_MSG_ERROR,
					"nothing to read, returning\n");
				bytes_read = 0;
				goto exit;
			}

			if (ch->state == MHI_DEV_CH_STOPPED) {
				mhi_log(MHI_MSG_ERROR,
					"channel (%d) already stopped\n",
					ch_id);
				bytes_read = -1;
				goto exit;
			}

			ch->tre_loc = el->tre.data_buf_ptr;
			ch->tre_size = el->tre.len;
			ch->tre_bytes_left = ch->tre_size;

			mhi_log(MHI_MSG_ERROR,
			"user_buf_remaining %d, ch->tre_size %d\n",
			usr_buf_remaining, ch->tre_size);
			bytes_to_read = min(usr_buf_remaining, ch->tre_size);
		}

		addr_offset = ch->tre_size - ch->tre_bytes_left;
		read_from_loc = ch->tre_loc + addr_offset;
		write_to_loc = (uint32_t) buf + (buf_size - usr_buf_remaining);

		mhi_log(MHI_MSG_ERROR, "reading %d bytes from chan %d\n",
				bytes_to_read, ch_id);

		mhi_transfer_host_to_device((void *) write_to_loc,
			read_from_loc, bytes_to_read, mhi_ctx);

		bytes_read += bytes_to_read;
		ch->tre_bytes_left -= bytes_to_read;
		usr_buf_remaining -= bytes_to_read;
		td_done = mhi_dev_check_tre_bytes_left(ch, ring, el, chain);
	} while (usr_buf_remaining  && !td_done);

	if (td_done && ch->state == MHI_DEV_CH_PENDING_STOP) {
		ch->state = MHI_DEV_CH_STOPPED;
		rc = mhi_dev_process_stop_cmd(ring, ch_id, mhi_ctx);
		if (rc) {
			mhi_log(MHI_MSG_ERROR,
				"Error while stopping channel (%d)\n", ch_id);
			bytes_read = -1;
		}
	}
exit:
	mutex_unlock(&ch->ch_lock);
	return bytes_read;
}
EXPORT_SYMBOL(mhi_dev_read_channel);

static void skip_to_next_td(struct mhi_dev_channel *ch)
{
	struct mhi_dev_ring *ring = ch->ring;
	union mhi_dev_ring_element_type *el;
	uint32_t td_boundary_reached = 0;

	ch->skip_td = 1;
	el = &ring->ring_cache[ring->rd_offset];
	while (ring->rd_offset != ring->wr_offset) {
		if (td_boundary_reached) {
			ch->skip_td = 0;
			break;
		}
		if (!el->tre.chain)
			td_boundary_reached = 1;
		mhi_dev_ring_inc_index(ring, ring->rd_offset);
		el = &ring->ring_cache[ring->rd_offset];
	}
}

int mhi_dev_write_channel(struct mhi_dev_client *handle_client,
						void *buf, size_t buf_size)
{
	struct mhi_dev_channel *ch;
	struct mhi_dev_ring *ring;
	union mhi_dev_ring_element_type *el;
	enum mhi_dev_cmd_completion_code code = MHI_CMD_COMPL_CODE_INVALID;
	int rc = 0;
	uint64_t ch_id, skip_tres = 0, write_to_loc;
	uint32_t read_from_loc;
	size_t usr_buf_remaining = buf_size;
	size_t usr_buf_offset = 0;
	size_t bytes_to_write = 0;
	size_t bytes_written = 0;
	uint32_t tre_len = 0, suspend_wait_timeout = 0;

	if (!handle_client) {
		pr_err("%s: invalid client handle\n", __func__);
		return -ENXIO;
	}

	if (!buf) {
		pr_err("%s: invalid buffer to write data\n", __func__);
		return -ENXIO;
	}

	mutex_lock(&mhi_ctx->mhi_write_test);

	if (atomic_read(&mhi_ctx->is_suspended)) {
		/*
		 * Expected usage is when there is a write
		 * to the MHI core -> notify SM.
		 */
		rc = mhi_dev_notify_sm_event(MHI_DEV_EVENT_CORE_WAKEUP);
		if (rc) {
			pr_err("error sending core wakeup event\n");
			mutex_unlock(&mhi_ctx->mhi_write_test);
			return rc;
		}
	}

	atomic_inc(&mhi_ctx->write_active);
	while (atomic_read(&mhi_ctx->is_suspended) &&
			suspend_wait_timeout < MHI_SUSPEND_WAIT_TIMEOUT) {
		/* wait for the suspend to finish */
		usleep_range(MHI_SUSPEND_WAIT_MIN, MHI_SUSPEND_WAIT_MAX);
		suspend_wait_timeout++;
	}

	ch = handle_client->channel;
	ch->wr_request_active = true;

	ring = ch->ring;
	ch_id = ch->ch_id;

	mutex_lock(&ch->ch_lock);

	if (ch->state == MHI_DEV_CH_STOPPED) {
		mhi_log(MHI_MSG_ERROR,
			"channel (%lld) already stopped\n", ch_id);
		bytes_written = -1;
		goto exit;
	}

	if (ch->state == MHI_DEV_CH_PENDING_STOP) {
		if (mhi_dev_process_stop_cmd(ring, ch_id, mhi_ctx) < 0)
			bytes_written = -1;
		goto exit;
	}

	if (ch->skip_td)
		skip_to_next_td(ch);

	do {
		if (ring->rd_offset == ring->wr_offset) {
			mhi_log(MHI_MSG_INFO, "No TREs available\n");
			break;
		}

		el = &ring->ring_cache[ring->rd_offset];
		tre_len = el->tre.len;

		bytes_to_write = min(usr_buf_remaining, tre_len);
		usr_buf_offset = buf_size - bytes_to_write;
		read_from_loc = (uint32_t) buf + usr_buf_offset;
		write_to_loc = el->tre.data_buf_ptr;
		mhi_transfer_device_to_host(write_to_loc,
						(void *) read_from_loc,
						bytes_to_write, mhi_ctx);
		bytes_written += bytes_to_write;
		usr_buf_remaining -= bytes_to_write;

		if (usr_buf_remaining) {
			if (!el->tre.chain)
				code = MHI_CMD_COMPL_CODE_OVERFLOW;
			else if (el->tre.ieob)
				code = MHI_CMD_COMPL_CODE_EOB;
		} else {
			if (el->tre.chain)
				skip_tres = 1;
			code = MHI_CMD_COMPL_CODE_EOT;
		}

		if (mhi_dev_send_completion_event(ch,
				ring->rd_offset, bytes_to_write, code) < 0) {
			mhi_log(MHI_MSG_ERROR,
				"error sending completion event ch_id:%lld\n",
				ch_id);
		}

		if (ch->state == MHI_DEV_CH_PENDING_STOP)
			break;

		mhi_dev_ring_inc_index(ring, ring->rd_offset);
	} while (!skip_tres && usr_buf_remaining);

	if (skip_tres)
		skip_to_next_td(ch);

	if (ch->state == MHI_DEV_CH_PENDING_STOP) {
		rc = mhi_dev_process_stop_cmd(ring, ch_id, mhi_ctx);
		if (rc) {
			mhi_log(MHI_MSG_ERROR,
				"channel (%lld) stop failed\n", ch_id);
		}
	}
exit:
	mutex_unlock(&ch->ch_lock);
	atomic_dec(&mhi_ctx->write_active);
	mutex_unlock(&mhi_ctx->mhi_write_test);
	return bytes_written;
}
EXPORT_SYMBOL(mhi_dev_write_channel);

static void mhi_dev_enable(struct work_struct *work)
{
	int rc = 0;
	struct ep_pcie_msi_config msi_cfg;
	struct mhi_dev *mhi = container_of(work,
				struct mhi_dev, ring_init_cb_work);

	enum mhi_dev_state state;
	uint32_t max_cnt = 0;


	rc = ipa_dma_init();
	if (rc) {
		pr_err("ipa dma init failed\n");
		return;
	}

	rc = ipa_dma_enable();
	if (rc) {
		pr_err("ipa enable failed\n");
		return;
	}

	rc = mhi_dev_ring_init(mhi);
	if (rc) {
		pr_err("MHI dev ring init failed\n");
		return;
	}

	/* Invoke MHI SM when device is in RESET state */
	mhi_dev_sm_init(mhi);

	/* set the env before setting the ready bit */
	rc = mhi_dev_mmio_set_env(mhi, MHI_ENV_VALUE);
	if (rc) {
		pr_err("%s: env setting failed\n", __func__);
		return;
	}
	mhi_uci_init();

	/* All set...let's notify the host */
	mhi_dev_sm_set_ready();

	rc = ep_pcie_get_msi_config(mhi->phandle, &msi_cfg);
	if (rc)
		pr_warn("MHI: error geting msi configs\n");
	else {
		rc = ep_pcie_trigger_msi(mhi->phandle, mhi->mhi_ep_msi_num);
		if (rc)
			return;
	}

	rc = mhi_dev_mmio_get_mhi_state(mhi, &state);
	if (rc) {
		pr_err("%s: get mhi state failed\n", __func__);
		return;
	}

	while (state != MHI_DEV_M0_STATE && max_cnt < MHI_DEV_M0_MAX_CNT) {
		/* Wait for Host to set the M0 state */
		usleep_range(MHI_M0_WAIT_MIN_USLEEP, MHI_M0_WAIT_MAX_USLEEP);
		rc = mhi_dev_mmio_get_mhi_state(mhi, &state);
		if (rc) {
			pr_err("%s: get mhi state failed\n", __func__);
			return;
		}
		max_cnt++;
	}

	mhi_log(MHI_MSG_INFO, "state:%d\n", state);

	if (state == MHI_DEV_M0_STATE) {
		rc = mhi_dev_cache_host_cfg(mhi);
		if (rc) {
			pr_err("Failed to cache the host config\n");
			return;
		}

		rc = mhi_dev_mmio_set_env(mhi, MHI_ENV_VALUE);
		if (rc) {
			pr_err("%s: env setting failed\n", __func__);
			return;
		}
	} else {
		pr_err("MHI device failed to enter M0\n");
		return;
	}

	rc = mhi_hwc_init(mhi_ctx);
	if (rc) {
		pr_err("error during hwc_init\n");
		return;
	}
}

static void mhi_ring_init_cb(void *data)
{
	struct mhi_dev *mhi = data;

	if (!mhi) {
		pr_err("Invalid MHI ctx\n");
		return;
	}

	queue_work(mhi->ring_init_wq, &mhi->ring_init_cb_work);
}

static int get_device_tree_data(struct platform_device *pdev)
{
	struct mhi_dev *mhi;
	int rc = 0;
	struct resource *res_mem = NULL;

	mhi = devm_kzalloc(&pdev->dev,
			sizeof(struct mhi_dev), GFP_KERNEL);
	if (!mhi)
		return -ENOMEM;

	mhi->pdev = pdev;
	mhi->dev = &pdev->dev;
	res_mem = platform_get_resource_byname(pdev,
					IORESOURCE_MEM, "mhi_mmio_base");
	if (!res_mem) {
		rc = -EINVAL;
		pr_err("Request MHI MMIO physical memory region failed\n");
		return rc;
	}

	mhi->mmio_base_pa_addr = res_mem->start;
	mhi->mmio_base_addr = ioremap_nocache(res_mem->start, MHI_1K_SIZE);
	if (!mhi->mmio_base_addr) {
		pr_err("Failed to IO map MMIO registers.\n");
		rc = -EINVAL;
		return rc;
	}

	res_mem = platform_get_resource_byname(pdev,
					IORESOURCE_MEM, "ipa_uc_mbox_crdb");
	if (!res_mem) {
		rc = -EINVAL;
		pr_err("Request IPA_UC_MBOX CRDB physical region failed\n");
		return rc;
	}

	mhi->ipa_uc_mbox_crdb = res_mem->start;

	res_mem = platform_get_resource_byname(pdev,
					IORESOURCE_MEM, "ipa_uc_mbox_erdb");
	if (!res_mem) {
		rc = -EINVAL;
		pr_err("Request IPA_UC_MBOX ERDB physical region failed\n");
		return rc;
	}

	mhi->ipa_uc_mbox_erdb = res_mem->start;

	mhi_ctx = mhi;

	rc = of_property_read_u32((&pdev->dev)->of_node,
				"qcom,mhi-ifc-id",
				&mhi_ctx->ifc_id);

	if (rc) {
		pr_err("qcom,mhi-ifc-id does not exist.\n");
		return rc;
	}

	rc = of_property_read_u32((&pdev->dev)->of_node,
				"qcom,mhi-ep-msi",
				&mhi_ctx->mhi_ep_msi_num);
	if (rc) {
		pr_err("qcom,mhi-ep-msi does not exist.\n");
		return rc;
	}

	rc = of_property_read_u32((&pdev->dev)->of_node,
				"qcom,mhi-version",
				&mhi_ctx->mhi_version);
	if (rc) {
		pr_err("qcom,mhi-version does not exist.\n");
		return rc;
	}

	return 0;
}

static int mhi_init(struct mhi_dev *mhi)
{
	int rc = 0, i = 0;
	struct platform_device *pdev = mhi->pdev;


	rc = mhi_dev_mmio_init(mhi);
	if (rc) {
		pr_err("Failed to update the MMIO init\n");
		return rc;
	}


	mhi->ring = devm_kzalloc(&pdev->dev,
			(sizeof(struct mhi_dev_ring) *
			(mhi->cfg.channels + mhi->cfg.event_rings + 1)),
			GFP_KERNEL);
	if (!mhi->ring)
		return -ENOMEM;

	mhi->ch = devm_kzalloc(&pdev->dev,
			(sizeof(struct mhi_dev_channel) *
			(mhi->cfg.channels)), GFP_KERNEL);
	if (!mhi->ch)
		return -ENOMEM;

	for (i = 0; i < mhi->cfg.channels; i++)
		mutex_init(&mhi->ch[i].ch_lock);

	mhi->mmio_backup = devm_kzalloc(&pdev->dev, MHI_DEV_MMIO_RANGE,
								GFP_KERNEL);
	if (!mhi->mmio_backup)
		return -ENOMEM;

#ifdef CONFIG_IPC_LOGGING
	mhi_ipc_log = ipc_log_context_create(MHI_IPC_LOG_PAGES, "mhi", 0);
	if (mhi_ipc_log == NULL) {
		dev_err(&pdev->dev,
				"Failed to create IPC logging context\n");
	}
#endif

	return 0;
}

static int mhi_dev_probe(struct platform_device *pdev)
{
	int rc = 0;

	if (pdev->dev.of_node) {
		rc = get_device_tree_data(pdev);
		if (rc) {
			pr_err("Error reading MHI Dev DT\n");
			return rc;
		}
	}

	mhi_ctx->phandle = ep_pcie_get_phandle(mhi_ctx->ifc_id);
	if (!mhi_ctx->phandle) {
		pr_err("PCIe driver is not ready yet.\n");
		return -EPROBE_DEFER;
	}

	if (ep_pcie_get_linkstatus(mhi_ctx->phandle) != EP_PCIE_LINK_ENABLED) {
		pr_err("PCIe link is not ready to use.\n");
		return -EPROBE_DEFER;
	}

	INIT_WORK(&mhi_ctx->chdb_ctrl_work, mhi_dev_scheduler);

	mhi_ctx->pending_ring_wq = alloc_workqueue("mhi_pending_wq",
							WQ_HIGHPRI, 0);
	if (!mhi_ctx->pending_ring_wq) {
		rc = -ENOMEM;
		return rc;
	}

	INIT_WORK(&mhi_ctx->pending_work, mhi_dev_process_ring_pending);

	INIT_WORK(&mhi_ctx->ring_init_cb_work, mhi_dev_enable);

	mhi_ctx->ring_init_wq = alloc_workqueue("mhi_ring_init_cb_wq",
							WQ_HIGHPRI, 0);
	if (!mhi_ctx->ring_init_wq) {
		rc = -ENOMEM;
		return rc;
	}

	INIT_LIST_HEAD(&mhi_ctx->event_ring_list);
	INIT_LIST_HEAD(&mhi_ctx->process_ring_list);
	mutex_init(&mhi_ctx->mhi_lock);
	mutex_init(&mhi_ctx->mhi_event_lock);
	mutex_init(&mhi_ctx->mhi_write_test);

	rc = mhi_init(mhi_ctx);
	if (rc)
		return rc;

	mhi_ctx->dma_cache = dma_alloc_coherent(&pdev->dev,
			(TRB_MAX_DATA_SIZE * 4),
			&mhi_ctx->cache_dma_handle, GFP_KERNEL);
	if (!mhi_ctx->dma_cache)
		return -ENOMEM;

	mhi_ctx->read_handle = dma_alloc_coherent(&pdev->dev,
			(TRB_MAX_DATA_SIZE * 4),
			&mhi_ctx->read_dma_handle,
			GFP_KERNEL);
	if (!mhi_ctx->read_handle)
		return -ENOMEM;

	mhi_ctx->write_handle = dma_alloc_coherent(&pdev->dev,
			(TRB_MAX_DATA_SIZE * 24),
			&mhi_ctx->write_dma_handle,
			GFP_KERNEL);
	if (!mhi_ctx->write_handle)
		return -ENOMEM;

	rc = mhi_dev_mmio_write(mhi_ctx, MHIVER, mhi_ctx->mhi_version);
	if (rc) {
		pr_err("Failed to update the MHI version\n");
		return rc;
	}

	mhi_ctx->event_reg.events = EP_PCIE_EVENT_PM_D3_HOT |
		EP_PCIE_EVENT_PM_D3_COLD |
		EP_PCIE_EVENT_PM_D0 |
		EP_PCIE_EVENT_PM_RST_DEAST |
		EP_PCIE_EVENT_MHI_A7 |
		EP_PCIE_EVENT_LINKDOWN;
	mhi_ctx->event_reg.user = mhi_ctx;
	mhi_ctx->event_reg.mode = EP_PCIE_TRIGGER_CALLBACK;
	mhi_ctx->event_reg.callback = mhi_dev_sm_pcie_handler;

	rc = ep_pcie_register_event(mhi_ctx->phandle, &mhi_ctx->event_reg);
	if (rc) {
		pr_err("Failed to register for events from PCIe\n");
		return rc;
	}

	pr_err("Registering with IPA\n");

	rc = ipa_register_ipa_ready_cb(mhi_ring_init_cb, mhi_ctx);
	if (rc < 0) {
		if (rc == -EEXIST) {
			mhi_ring_init_cb(mhi_ctx);
		} else {
			pr_err("Error calling IPA cb with %d\n", rc);
			return rc;
		}
	}

	return 0;
}

static int mhi_dev_remove(struct platform_device *pdev)
{
	platform_set_drvdata(pdev, NULL);

	return 0;
}

static const struct of_device_id mhi_dev_match_table[] = {
	{	.compatible = "qcom,msm-mhi-dev" },
	{}
};

static struct platform_driver mhi_dev_driver = {
	.driver		= {
		.name	= "qcom,msm-mhi-dev",
		.of_match_table = mhi_dev_match_table,
	},
	.probe		= mhi_dev_probe,
	.remove		= mhi_dev_remove,
};

module_param(mhi_msg_lvl , uint, S_IRUGO | S_IWUSR);
module_param(mhi_ipc_msg_lvl, uint, S_IRUGO | S_IWUSR);

MODULE_PARM_DESC(mhi_msg_lvl, "mhi msg lvl");
MODULE_PARM_DESC(mhi_ipc_msg_lvl, "mhi ipc msg lvl");

static int __init mhi_dev_init(void)
{
	return platform_driver_register(&mhi_dev_driver);
}
module_init(mhi_dev_init);

static void __exit mhi_dev_exit(void)
{
	platform_driver_unregister(&mhi_dev_driver);
}
module_exit(mhi_dev_exit);

MODULE_DESCRIPTION("MHI device driver");
MODULE_LICENSE("GPL v2");
