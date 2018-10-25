/* Copyright (c) 2012-2018, The Linux Foundation. All rights reserved.
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

#include <linux/delay.h>
#include <linux/device.h>
#include <linux/dmapool.h>
#include <linux/list.h>
#include <linux/netdevice.h>
#include <linux/msm_gsi.h>
#include "ipa_i.h"
#include "ipa_trace.h"
#include "ipahal/ipahal.h"
#include "ipahal/ipahal_fltrt.h"

#define IPA_WAN_AGGR_PKT_CNT 5
#define IPA_LAST_DESC_CNT 0xFFFF
#define POLLING_INACTIVITY_RX 40
#define POLLING_MIN_SLEEP_RX 1010
#define POLLING_MAX_SLEEP_RX 1050
#define POLLING_INACTIVITY_TX 40
#define POLLING_MIN_SLEEP_TX 400
#define POLLING_MAX_SLEEP_TX 500
/* 8K less 1 nominal MTU (1500 bytes) rounded to units of KB */
#define IPA_MTU 1500
#define IPA_GENERIC_AGGR_BYTE_LIMIT 6
#define IPA_GENERIC_AGGR_TIME_LIMIT 1
#define IPA_GENERIC_AGGR_PKT_LIMIT 0

#define IPA_GSB_AGGR_BYTE_LIMIT 14
#define IPA_GSB_RX_BUFF_BASE_SZ 16384

#define IPA_GENERIC_RX_BUFF_BASE_SZ 8192
#define IPA_REAL_GENERIC_RX_BUFF_SZ(X) (SKB_DATA_ALIGN(\
		(X) + NET_SKB_PAD) +\
		SKB_DATA_ALIGN(sizeof(struct skb_shared_info)))
#define IPA_GENERIC_RX_BUFF_SZ(X) ((X) -\
		(IPA_REAL_GENERIC_RX_BUFF_SZ(X) - (X)))
#define IPA_GENERIC_RX_BUFF_LIMIT (\
		IPA_REAL_GENERIC_RX_BUFF_SZ(\
		IPA_GENERIC_RX_BUFF_BASE_SZ) -\
		IPA_GENERIC_RX_BUFF_BASE_SZ)

/* less 1 nominal MTU (1500 bytes) rounded to units of KB */
#define IPA_ADJUST_AGGR_BYTE_LIMIT(X) (((X) - IPA_MTU)/1000)

#define IPA_RX_BUFF_CLIENT_HEADROOM 256

#define IPA_WLAN_RX_POOL_SZ 100
#define IPA_WLAN_RX_POOL_SZ_LOW_WM 5
#define IPA_WLAN_RX_BUFF_SZ 2048
#define IPA_WLAN_COMM_RX_POOL_LOW 100
#define IPA_WLAN_COMM_RX_POOL_HIGH 900

#define IPA_ODU_RX_BUFF_SZ 2048
#define IPA_ODU_RX_POOL_SZ 64
#define IPA_SIZE_DL_CSUM_META_TRAILER 8

#define IPA_GSI_MAX_CH_LOW_WEIGHT 15
#define IPA_GSI_EVT_RING_INT_MODT (32 * 1) /* 1ms under 32KHz clock */

#define IPA_GSI_CH_20_WA_NUM_CH_TO_ALLOC 10
/* The below virtual channel cannot be used by any entity */
#define IPA_GSI_CH_20_WA_VIRT_CHAN 29

#define IPA_DEFAULT_SYS_YELLOW_WM 32
#define IPA_REPL_XFER_THRESH 10

#define IPA_TX_SEND_COMPL_NOP_DELAY_NS (2 * 1000 * 1000)

#define IPA_APPS_BW_FOR_PM 700

static struct sk_buff *ipa3_get_skb_ipa_rx(unsigned int len, gfp_t flags);
static void ipa3_replenish_wlan_rx_cache(struct ipa3_sys_context *sys);
static void ipa3_replenish_rx_cache(struct ipa3_sys_context *sys);
static void ipa3_replenish_rx_work_func(struct work_struct *work);
static void ipa3_fast_replenish_rx_cache(struct ipa3_sys_context *sys);
static void ipa3_wq_handle_rx(struct work_struct *work);
static void ipa3_wq_rx_common(struct ipa3_sys_context *sys, u32 size);
static void ipa3_wlan_wq_rx_common(struct ipa3_sys_context *sys,
				u32 size);
static int ipa3_assign_policy(struct ipa_sys_connect_params *in,
		struct ipa3_sys_context *sys);
static void ipa3_cleanup_rx(struct ipa3_sys_context *sys);
static void ipa3_wq_rx_avail(struct work_struct *work);
static void ipa3_alloc_wlan_rx_common_cache(u32 size);
static void ipa3_cleanup_wlan_rx_common_cache(void);
static void ipa3_wq_repl_rx(struct work_struct *work);
static void ipa3_dma_memcpy_notify(struct ipa3_sys_context *sys,
		struct ipa_mem_buffer *mem_info);
static int ipa_gsi_setup_channel(struct ipa_sys_connect_params *in,
	struct ipa3_ep_context *ep);
static int ipa_populate_tag_field(struct ipa3_desc *desc,
		struct ipa3_tx_pkt_wrapper *tx_pkt,
		struct ipahal_imm_cmd_pyld **tag_pyld_ret);
static int ipa_poll_gsi_pkt(struct ipa3_sys_context *sys,
	struct ipa_mem_buffer *mem_info);
static unsigned long tag_to_pointer_wa(uint64_t tag);
static uint64_t pointer_to_tag_wa(struct ipa3_tx_pkt_wrapper *tx_pkt);

static u32 ipa_adjust_ra_buff_base_sz(u32 aggr_byte_limit);

static void ipa3_wq_write_done_common(struct ipa3_sys_context *sys,
				struct ipa3_tx_pkt_wrapper *tx_pkt)
{
	struct ipa3_tx_pkt_wrapper *next_pkt;
	int i, cnt;

	if (unlikely(tx_pkt == NULL)) {
		IPAERR("tx_pkt is NULL\n");
		return;
	}

	cnt = tx_pkt->cnt;
	IPADBG_LOW("cnt: %d\n", cnt);
	for (i = 0; i < cnt; i++) {
		spin_lock_bh(&sys->spinlock);
		if (unlikely(list_empty(&sys->head_desc_list))) {
			spin_unlock_bh(&sys->spinlock);
			return;
		}
		next_pkt = list_next_entry(tx_pkt, link);
		list_del(&tx_pkt->link);
		sys->len--;
		spin_unlock_bh(&sys->spinlock);
		if (!tx_pkt->no_unmap_dma) {
			if (tx_pkt->type != IPA_DATA_DESC_SKB_PAGED) {
				dma_unmap_single(ipa3_ctx->pdev,
					tx_pkt->mem.phys_base,
					tx_pkt->mem.size,
					DMA_TO_DEVICE);
			} else {
				dma_unmap_page(ipa3_ctx->pdev,
					tx_pkt->mem.phys_base,
					tx_pkt->mem.size,
					DMA_TO_DEVICE);
			}
		}
		if (tx_pkt->callback)
			tx_pkt->callback(tx_pkt->user1, tx_pkt->user2);

		kmem_cache_free(ipa3_ctx->tx_pkt_wrapper_cache, tx_pkt);
		tx_pkt = next_pkt;
	}
}

static void ipa3_wq_write_done_status(int src_pipe,
			struct ipa3_tx_pkt_wrapper *tx_pkt)
{
	struct ipa3_sys_context *sys;

	WARN_ON(src_pipe >= ipa3_ctx->ipa_num_pipes);

	if (!ipa3_ctx->ep[src_pipe].status.status_en)
		return;

	sys = ipa3_ctx->ep[src_pipe].sys;
	if (!sys)
		return;

	ipa3_wq_write_done_common(sys, tx_pkt);
}

/**
 * ipa_write_done() - this function will be (eventually) called when a Tx
 * operation is complete
 * * @work:	work_struct used by the work queue
 *
 * Will be called in deferred context.
 * - invoke the callback supplied by the client who sent this command
 * - iterate over all packets and validate that
 *   the order for sent packet is the same as expected
 * - delete all the tx packet descriptors from the system
 *   pipe context (not needed anymore)
 */
static void ipa3_wq_write_done(struct work_struct *work)
{
	struct ipa3_tx_pkt_wrapper *tx_pkt;
	struct ipa3_sys_context *sys;
	struct ipa3_tx_pkt_wrapper *this_pkt;

	tx_pkt = container_of(work, struct ipa3_tx_pkt_wrapper, work);
	sys = tx_pkt->sys;
	spin_lock_bh(&sys->spinlock);
	this_pkt = list_first_entry(&sys->head_desc_list,
		struct ipa3_tx_pkt_wrapper, link);
	while (tx_pkt != this_pkt) {
		spin_unlock_bh(&sys->spinlock);
		ipa3_wq_write_done_common(sys, this_pkt);
		spin_lock_bh(&sys->spinlock);
		this_pkt = list_first_entry(&sys->head_desc_list,
			struct ipa3_tx_pkt_wrapper, link);
	}
	spin_unlock_bh(&sys->spinlock);
	ipa3_wq_write_done_common(sys, tx_pkt);
}


static void ipa3_send_nop_desc(struct work_struct *work)
{
	struct ipa3_sys_context *sys = container_of(work,
		struct ipa3_sys_context, work);
	struct gsi_xfer_elem nop_xfer;
	struct ipa3_tx_pkt_wrapper *tx_pkt;

	IPADBG_LOW("gsi send NOP for ch: %lu\n", sys->ep->gsi_chan_hdl);
	tx_pkt = kmem_cache_zalloc(ipa3_ctx->tx_pkt_wrapper_cache, GFP_KERNEL);
	if (!tx_pkt) {
		IPAERR("failed to alloc tx wrapper\n");
		queue_work(sys->wq, &sys->work);
		return;
	}

	INIT_LIST_HEAD(&tx_pkt->link);
	tx_pkt->cnt = 1;
	INIT_WORK(&tx_pkt->work, ipa3_wq_write_done);
	tx_pkt->no_unmap_dma = true;
	tx_pkt->sys = sys;
	spin_lock_bh(&sys->spinlock);
	if (unlikely(!sys->nop_pending)) {
		spin_unlock_bh(&sys->spinlock);
		kmem_cache_free(ipa3_ctx->tx_pkt_wrapper_cache, tx_pkt);
		return;
	}
	list_add_tail(&tx_pkt->link, &sys->head_desc_list);
	sys->nop_pending = false;
	spin_unlock_bh(&sys->spinlock);

	memset(&nop_xfer, 0, sizeof(nop_xfer));
	nop_xfer.type = GSI_XFER_ELEM_NOP;
	nop_xfer.flags = GSI_XFER_FLAG_EOT;
	nop_xfer.xfer_user_data = tx_pkt;
	if (gsi_queue_xfer(sys->ep->gsi_chan_hdl, 1, &nop_xfer, true)) {
		IPAERR("gsi_queue_xfer for ch:%lu failed\n",
			sys->ep->gsi_chan_hdl);
		queue_work(sys->wq, &sys->work);
		return;
	}
	sys->len_pending_xfer = 0;
	/* make sure TAG process is sent before clocks are gated */
	ipa3_ctx->tag_process_before_gating = true;

}


/**
 * ipa3_send() - Send multiple descriptors in one HW transaction
 * @sys: system pipe context
 * @num_desc: number of packets
 * @desc: packets to send (may be immediate command or data)
 * @in_atomic:  whether caller is in atomic context
 *
 * This function is used for GPI connection.
 * - ipa3_tx_pkt_wrapper will be used for each ipa
 *   descriptor (allocated from wrappers cache)
 * - The wrapper struct will be configured for each ipa-desc payload and will
 *   contain information which will be later used by the user callbacks
 * - Each packet (command or data) that will be sent will also be saved in
 *   ipa3_sys_context for later check that all data was sent
 *
 * Return codes: 0: success, -EFAULT: failure
 */
int ipa3_send(struct ipa3_sys_context *sys,
		u32 num_desc,
		struct ipa3_desc *desc,
		bool in_atomic)
{
	struct ipa3_tx_pkt_wrapper *tx_pkt, *tx_pkt_first;
	struct ipahal_imm_cmd_pyld *tag_pyld_ret = NULL;
	struct ipa3_tx_pkt_wrapper *next_pkt;
	struct gsi_xfer_elem *gsi_xfer_elem_array = NULL;
	int i = 0;
	int j;
	int result;
	u32 mem_flag = GFP_ATOMIC;
	const struct ipa_gsi_ep_config *gsi_ep_cfg;
	bool send_nop = false;

	if (unlikely(!in_atomic))
		mem_flag = GFP_KERNEL;

	gsi_ep_cfg = ipa3_get_gsi_ep_info(sys->ep->client);
	if (unlikely(!gsi_ep_cfg)) {
		IPAERR("failed to get gsi EP config for client=%d\n",
			sys->ep->client);
		return -EFAULT;
	}
	if (unlikely(num_desc > gsi_ep_cfg->ipa_if_tlv)) {
		IPAERR("Too many chained descriptors need=%d max=%d\n",
			num_desc, gsi_ep_cfg->ipa_if_tlv);
		WARN_ON(1);
		return -EPERM;
	}

	gsi_xfer_elem_array =
		kzalloc(num_desc * sizeof(struct gsi_xfer_elem),
		mem_flag);
	if (!gsi_xfer_elem_array) {
		IPAERR("Failed to alloc mem for gsi xfer array.\n");
		return -ENOMEM;
	}

	spin_lock_bh(&sys->spinlock);

	for (i = 0; i < num_desc; i++) {
		tx_pkt = kmem_cache_zalloc(ipa3_ctx->tx_pkt_wrapper_cache,
					   GFP_ATOMIC);
		if (!tx_pkt) {
			IPAERR("failed to alloc tx wrapper\n");
			result = -ENOMEM;
			goto failure;
		}

		INIT_LIST_HEAD(&tx_pkt->link);

		if (i == 0) {
			tx_pkt_first = tx_pkt;
			tx_pkt->cnt = num_desc;
			INIT_WORK(&tx_pkt->work, ipa3_wq_write_done);
		}

		/* populate tag field */
		if (desc[i].is_tag_status) {
			if (ipa_populate_tag_field(&desc[i], tx_pkt,
				&tag_pyld_ret)) {
				IPAERR("Failed to populate tag field\n");
				result = -EFAULT;
				goto failure_dma_map;
			}
		}

		tx_pkt->type = desc[i].type;

		if (desc[i].type != IPA_DATA_DESC_SKB_PAGED) {
			tx_pkt->mem.base = desc[i].pyld;
			tx_pkt->mem.size = desc[i].len;

			if (!desc[i].dma_address_valid) {
				tx_pkt->mem.phys_base =
					dma_map_single(ipa3_ctx->pdev,
					tx_pkt->mem.base,
					tx_pkt->mem.size,
					DMA_TO_DEVICE);
			} else {
					tx_pkt->mem.phys_base =
						desc[i].dma_address;
					tx_pkt->no_unmap_dma = true;
			}
		} else {
			tx_pkt->mem.base = desc[i].frag;
			tx_pkt->mem.size = desc[i].len;

			if (!desc[i].dma_address_valid) {
				tx_pkt->mem.phys_base =
					skb_frag_dma_map(ipa3_ctx->pdev,
					desc[i].frag,
					0, tx_pkt->mem.size,
					DMA_TO_DEVICE);
			} else {
				tx_pkt->mem.phys_base =
					desc[i].dma_address;
				tx_pkt->no_unmap_dma = true;
			}
		}
		if (dma_mapping_error(ipa3_ctx->pdev, tx_pkt->mem.phys_base)) {
			IPAERR("failed to do dma map.\n");
			result = -EFAULT;
			goto failure_dma_map;
		}

		tx_pkt->sys = sys;
		tx_pkt->callback = desc[i].callback;
		tx_pkt->user1 = desc[i].user1;
		tx_pkt->user2 = desc[i].user2;

		list_add_tail(&tx_pkt->link, &sys->head_desc_list);

		gsi_xfer_elem_array[i].addr = tx_pkt->mem.phys_base;

		/*
		 * Special treatment for immediate commands, where
		 * the structure of the descriptor is different
		 */
		if (desc[i].type == IPA_IMM_CMD_DESC) {
			gsi_xfer_elem_array[i].len = desc[i].opcode;
			gsi_xfer_elem_array[i].type =
				GSI_XFER_ELEM_IMME_CMD;
		} else {
			gsi_xfer_elem_array[i].len = desc[i].len;
			gsi_xfer_elem_array[i].type =
				GSI_XFER_ELEM_DATA;
		}

		if (i == (num_desc - 1)) {
			if (!sys->use_comm_evt_ring) {
				gsi_xfer_elem_array[i].flags |=
					GSI_XFER_FLAG_EOT;
				gsi_xfer_elem_array[i].flags |=
					GSI_XFER_FLAG_BEI;
			}
			gsi_xfer_elem_array[i].xfer_user_data =
				tx_pkt_first;
		} else {
				gsi_xfer_elem_array[i].flags |=
					GSI_XFER_FLAG_CHAIN;
		}
	}

	IPADBG_LOW("ch:%lu queue xfer\n", sys->ep->gsi_chan_hdl);
	result = gsi_queue_xfer(sys->ep->gsi_chan_hdl, num_desc,
			gsi_xfer_elem_array, true);
	if (result != GSI_STATUS_SUCCESS) {
		IPAERR("GSI xfer failed.\n");
		result = -EFAULT;
		goto failure;
	}
	kfree(gsi_xfer_elem_array);

	if (sys->use_comm_evt_ring && !sys->nop_pending) {
		sys->nop_pending = true;
		send_nop = true;
	}
	spin_unlock_bh(&sys->spinlock);

	/* set the timer for sending the NOP descriptor */
	if (send_nop) {
		ktime_t time = ktime_set(0, IPA_TX_SEND_COMPL_NOP_DELAY_NS);

		IPADBG_LOW("scheduling timer for ch %lu\n",
			sys->ep->gsi_chan_hdl);
		hrtimer_start(&sys->db_timer, time, HRTIMER_MODE_REL);
	}

	/* make sure TAG process is sent before clocks are gated */
	ipa3_ctx->tag_process_before_gating = true;

	return 0;

failure_dma_map:
		kmem_cache_free(ipa3_ctx->tx_pkt_wrapper_cache, tx_pkt);

failure:
	ipahal_destroy_imm_cmd(tag_pyld_ret);
	tx_pkt = tx_pkt_first;
	for (j = 0; j < i; j++) {
		next_pkt = list_next_entry(tx_pkt, link);
		list_del(&tx_pkt->link);

		if (!tx_pkt->no_unmap_dma) {
			if (desc[j].type != IPA_DATA_DESC_SKB_PAGED) {
				dma_unmap_single(ipa3_ctx->pdev,
					tx_pkt->mem.phys_base,
					tx_pkt->mem.size, DMA_TO_DEVICE);
			} else {
				dma_unmap_page(ipa3_ctx->pdev,
					tx_pkt->mem.phys_base,
					tx_pkt->mem.size,
					DMA_TO_DEVICE);
			}
		}
		kmem_cache_free(ipa3_ctx->tx_pkt_wrapper_cache, tx_pkt);
		tx_pkt = next_pkt;
	}

	kfree(gsi_xfer_elem_array);

	spin_unlock_bh(&sys->spinlock);
	return result;
}

/**
 * ipa3_send_one() - Send a single descriptor
 * @sys:	system pipe context
 * @desc:	descriptor to send
 * @in_atomic:  whether caller is in atomic context
 *
 * - Allocate tx_packet wrapper
 * - transfer data to the IPA
 * - after the transfer was done the SPS will
 *   notify the sending user via ipa_sps_irq_comp_tx()
 *
 * Return codes: 0: success, -EFAULT: failure
 */
int ipa3_send_one(struct ipa3_sys_context *sys, struct ipa3_desc *desc,
	bool in_atomic)
{
	return ipa3_send(sys, 1, desc, in_atomic);
}

/**
 * ipa3_transport_irq_cmd_ack - callback function which will be called by
 * the transport driver after an immediate command is complete.
 * @user1:	pointer to the descriptor of the transfer
 * @user2:
 *
 * Complete the immediate commands completion object, this will release the
 * thread which waits on this completion object (ipa3_send_cmd())
 */
static void ipa3_transport_irq_cmd_ack(void *user1, int user2)
{
	struct ipa3_desc *desc = (struct ipa3_desc *)user1;

	if (!desc) {
		IPAERR("desc is NULL\n");
		WARN_ON(1);
		return;
	}
	IPADBG_LOW("got ack for cmd=%d\n", desc->opcode);
	complete(&desc->xfer_done);
}

/**
 * ipa3_transport_irq_cmd_ack_free - callback function which will be
 * called by the transport driver after an immediate command is complete.
 * This function will also free the completion object once it is done.
 * @tag_comp: pointer to the completion object
 * @ignored: parameter not used
 *
 * Complete the immediate commands completion object, this will release the
 * thread which waits on this completion object (ipa3_send_cmd())
 */
static void ipa3_transport_irq_cmd_ack_free(void *tag_comp, int ignored)
{
	struct ipa3_tag_completion *comp = tag_comp;

	if (!comp) {
		IPAERR("comp is NULL\n");
		return;
	}

	complete(&comp->comp);
	if (atomic_dec_return(&comp->cnt) == 0)
		kfree(comp);
}

/**
 * ipa3_send_cmd - send immediate commands
 * @num_desc:	number of descriptors within the desc struct
 * @descr:	descriptor structure
 *
 * Function will block till command gets ACK from IPA HW, caller needs
 * to free any resources it allocated after function returns
 * The callback in ipa3_desc should not be set by the caller
 * for this function.
 */
int ipa3_send_cmd(u16 num_desc, struct ipa3_desc *descr)
{
	struct ipa3_desc *desc;
	int i, result = 0;
	struct ipa3_sys_context *sys;
	int ep_idx;

	for (i = 0; i < num_desc; i++)
		IPADBG("sending imm cmd %d\n", descr[i].opcode);

	ep_idx = ipa3_get_ep_mapping(IPA_CLIENT_APPS_CMD_PROD);
	if (-1 == ep_idx) {
		IPAERR("Client %u is not mapped\n",
			IPA_CLIENT_APPS_CMD_PROD);
		return -EFAULT;
	}

	sys = ipa3_ctx->ep[ep_idx].sys;
	IPA_ACTIVE_CLIENTS_INC_SIMPLE();

	if (num_desc == 1) {
		init_completion(&descr->xfer_done);

		if (descr->callback || descr->user1)
			WARN_ON(1);

		descr->callback = ipa3_transport_irq_cmd_ack;
		descr->user1 = descr;
		if (ipa3_send_one(sys, descr, true)) {
			IPAERR("fail to send immediate command\n");
			result = -EFAULT;
			goto bail;
		}
		wait_for_completion(&descr->xfer_done);
	} else {
		desc = &descr[num_desc - 1];
		init_completion(&desc->xfer_done);

		if (desc->callback || desc->user1)
			WARN_ON(1);

		desc->callback = ipa3_transport_irq_cmd_ack;
		desc->user1 = desc;
		if (ipa3_send(sys, num_desc, descr, true)) {
			IPAERR("fail to send multiple immediate command set\n");
			result = -EFAULT;
			goto bail;
		}
		wait_for_completion(&desc->xfer_done);
	}

bail:
		IPA_ACTIVE_CLIENTS_DEC_SIMPLE();
		return result;
}

/**
 * ipa3_send_cmd_timeout - send immediate commands with limited time
 *	waiting for ACK from IPA HW
 * @num_desc:	number of descriptors within the desc struct
 * @descr:	descriptor structure
 * @timeout:	millisecond to wait till get ACK from IPA HW
 *
 * Function will block till command gets ACK from IPA HW or timeout.
 * Caller needs to free any resources it allocated after function returns
 * The callback in ipa3_desc should not be set by the caller
 * for this function.
 */
int ipa3_send_cmd_timeout(u16 num_desc, struct ipa3_desc *descr, u32 timeout)
{
	struct ipa3_desc *desc;
	int i, result = 0;
	struct ipa3_sys_context *sys;
	int ep_idx;
	int completed;
	struct ipa3_tag_completion *comp;

	for (i = 0; i < num_desc; i++)
		IPADBG("sending imm cmd %d\n", descr[i].opcode);

	ep_idx = ipa3_get_ep_mapping(IPA_CLIENT_APPS_CMD_PROD);
	if (-1 == ep_idx) {
		IPAERR("Client %u is not mapped\n",
			IPA_CLIENT_APPS_CMD_PROD);
		return -EFAULT;
	}

	comp = kzalloc(sizeof(*comp), GFP_ATOMIC);
	if (!comp) {
		IPAERR("no mem\n");
		return -ENOMEM;
	}
	init_completion(&comp->comp);

	/* completion needs to be released from both here and in ack callback */
	atomic_set(&comp->cnt, 2);

	sys = ipa3_ctx->ep[ep_idx].sys;

	if (num_desc == 1) {
		if (descr->callback || descr->user1)
			WARN_ON(1);

		descr->callback = ipa3_transport_irq_cmd_ack_free;
		descr->user1 = comp;
		if (ipa3_send_one(sys, descr, true)) {
			IPAERR("fail to send immediate command\n");
			kfree(comp);
			result = -EFAULT;
			goto bail;
		}
	} else {
		desc = &descr[num_desc - 1];

		if (desc->callback || desc->user1)
			WARN_ON(1);

		desc->callback = ipa3_transport_irq_cmd_ack_free;
		desc->user1 = comp;
		if (ipa3_send(sys, num_desc, descr, true)) {
			IPAERR("fail to send multiple immediate command set\n");
			kfree(comp);
			result = -EFAULT;
			goto bail;
		}
	}

	completed = wait_for_completion_timeout(
		&comp->comp, msecs_to_jiffies(timeout));
	if (!completed)
		IPADBG("timeout waiting for imm-cmd ACK\n");

	if (atomic_dec_return(&comp->cnt) == 0)
		kfree(comp);

bail:
	return result;
}

/**
 * ipa3_handle_rx_core() - The core functionality of packet reception. This
 * function is read from multiple code paths.
 *
 * All the packets on the Rx data path are received on the IPA_A5_LAN_WAN_IN
 * endpoint. The function runs as long as there are packets in the pipe.
 * For each packet:
 *  - Disconnect the packet from the system pipe linked list
 *  - Unmap the packets skb, make it non DMAable
 *  - Free the packet from the cache
 *  - Prepare a proper skb
 *  - Call the endpoints notify function, passing the skb in the parameters
 *  - Replenish the rx cache
 */
static int ipa3_handle_rx_core(struct ipa3_sys_context *sys, bool process_all,
		bool in_poll_state)
{
	int ret;
	int cnt = 0;
	struct ipa_mem_buffer mem_info = { 0 };

	while ((in_poll_state ? atomic_read(&sys->curr_polling_state) :
		!atomic_read(&sys->curr_polling_state))) {
		if (cnt && !process_all)
			break;

		ret = ipa_poll_gsi_pkt(sys, &mem_info);
		if (ret)
			break;

		if (IPA_CLIENT_IS_MEMCPY_DMA_CONS(sys->ep->client))
			ipa3_dma_memcpy_notify(sys, &mem_info);
		else if (IPA_CLIENT_IS_WLAN_CONS(sys->ep->client))
			ipa3_wlan_wq_rx_common(sys, mem_info.size);
		else
			ipa3_wq_rx_common(sys, mem_info.size);

		++cnt;
	}
	return cnt;
}

/**
 * ipa3_rx_switch_to_intr_mode() - Operate the Rx data path in interrupt mode
 */
static void ipa3_rx_switch_to_intr_mode(struct ipa3_sys_context *sys)
{
	int ret;

	if (!atomic_read(&sys->curr_polling_state)) {
		IPAERR("already in intr mode\n");
		goto fail;
	}
	atomic_set(&sys->curr_polling_state, 0);
	ipa3_dec_release_wakelock();
	ret = gsi_config_channel_mode(sys->ep->gsi_chan_hdl,
		GSI_CHAN_MODE_CALLBACK);
	if (ret != GSI_STATUS_SUCCESS) {
		IPAERR("Failed to switch to intr mode.\n");
		goto fail;
	}
	return;

fail:
	queue_delayed_work(sys->wq, &sys->switch_to_intr_work,
			msecs_to_jiffies(1));
}

/**
 * ipa3_handle_rx() - handle packet reception. This function is executed in the
 * context of a work queue.
 * @work: work struct needed by the work queue
 *
 * ipa3_handle_rx_core() is run in polling mode. After all packets has been
 * received, the driver switches back to interrupt mode.
 */
static void ipa3_handle_rx(struct ipa3_sys_context *sys)
{
	int inactive_cycles = 0;
	int cnt;

	if (ipa3_ctx->use_ipa_pm)
		ipa_pm_activate_sync(sys->pm_hdl);
	else
		IPA_ACTIVE_CLIENTS_INC_SIMPLE();
	do {
		cnt = ipa3_handle_rx_core(sys, true, true);
		if (cnt == 0)
			inactive_cycles++;
		else
			inactive_cycles = 0;

		trace_idle_sleep_enter3(sys->ep->client);
		usleep_range(POLLING_MIN_SLEEP_RX, POLLING_MAX_SLEEP_RX);
		trace_idle_sleep_exit3(sys->ep->client);

		/*
		 * if pipe is out of buffers there is no point polling for
		 * completed descs; release the worker so delayed work can
		 * run in a timely manner
		 */
		if (sys->len - sys->len_pending_xfer == 0)
			break;

	} while (inactive_cycles <= POLLING_INACTIVITY_RX);

	trace_poll_to_intr3(sys->ep->client);
	ipa3_rx_switch_to_intr_mode(sys);
	if (ipa3_ctx->use_ipa_pm)
		ipa_pm_deferred_deactivate(sys->pm_hdl);
	else
		IPA_ACTIVE_CLIENTS_DEC_SIMPLE();
}

static void ipa3_switch_to_intr_rx_work_func(struct work_struct *work)
{
	struct delayed_work *dwork;
	struct ipa3_sys_context *sys;

	dwork = container_of(work, struct delayed_work, work);
	sys = container_of(dwork, struct ipa3_sys_context, switch_to_intr_work);

	if (sys->ep->napi_enabled) {
		/* interrupt mode is done in ipa3_rx_poll context */
		ipa_assert();
	} else
		ipa3_handle_rx(sys);
}

enum hrtimer_restart ipa3_ring_doorbell_timer_fn(struct hrtimer *param)
{
	struct ipa3_sys_context *sys = container_of(param,
		struct ipa3_sys_context, db_timer);

	queue_work(sys->wq, &sys->work);
	return HRTIMER_NORESTART;
}

static void ipa_pm_sys_pipe_cb(void *p, enum ipa_pm_cb_event event)
{
	struct ipa3_sys_context *sys = (struct ipa3_sys_context *)p;

	switch (event) {
	case IPA_PM_CLIENT_ACTIVATED:
		/*
		 * this event is ignored as the sync version of activation
		 * will be used.
		 */
		break;
	case IPA_PM_REQUEST_WAKEUP:
		/*
		 * pipe will be unsuspended as part of
		 * enabling IPA clocks
		 */
		ipa_pm_activate(sys->pm_hdl);
		ipa_pm_deferred_deactivate(sys->pm_hdl);
		break;
	default:
		IPAERR("Unexpected event %d\n", event);
		WARN_ON(1);
		return;
	}
}

/**
 * ipa3_setup_sys_pipe() - Setup an IPA GPI pipe and perform
 * IPA EP configuration
 * @sys_in:	[in] input needed to setup the pipe and configure EP
 * @clnt_hdl:	[out] client handle
 *
 *  - configure the end-point registers with the supplied
 *    parameters from the user.
 *  - Creates a GPI connection with IPA.
 *  - allocate descriptor FIFO
 *
 * Returns:	0 on success, negative on failure
 */
int ipa3_setup_sys_pipe(struct ipa_sys_connect_params *sys_in, u32 *clnt_hdl)
{
	struct ipa3_ep_context *ep;
	int ipa_ep_idx;
	int result = -EINVAL;
	char buff[IPA_RESOURCE_NAME_MAX];

	if (sys_in == NULL || clnt_hdl == NULL) {
		IPAERR("NULL args\n");
		goto fail_gen;
	}

	if (sys_in->client >= IPA_CLIENT_MAX || sys_in->desc_fifo_sz == 0) {
		IPAERR("bad parm client:%d fifo_sz:%d\n",
			sys_in->client, sys_in->desc_fifo_sz);
		goto fail_gen;
	}

	ipa_ep_idx = ipa3_get_ep_mapping(sys_in->client);
	if (ipa_ep_idx == -1) {
		IPAERR("Invalid client.\n");
		goto fail_gen;
	}

	ep = &ipa3_ctx->ep[ipa_ep_idx];
	if (ep->valid == 1) {
		IPAERR("EP %d already allocated.\n", ipa_ep_idx);
		goto fail_gen;
	}

	IPA_ACTIVE_CLIENTS_INC_EP(sys_in->client);
	memset(ep, 0, offsetof(struct ipa3_ep_context, sys));

	if (!ep->sys) {
		struct ipa_pm_register_params pm_reg;

		memset(&pm_reg, 0, sizeof(pm_reg));
		ep->sys = kzalloc(sizeof(struct ipa3_sys_context), GFP_KERNEL);
		if (!ep->sys) {
			IPAERR("failed to sys ctx for client %d\n",
					sys_in->client);
			result = -ENOMEM;
			goto fail_and_disable_clocks;
		}

		ep->sys->ep = ep;
		snprintf(buff, IPA_RESOURCE_NAME_MAX, "ipawq%d",
				sys_in->client);
		ep->sys->wq = alloc_workqueue(buff,
				WQ_MEM_RECLAIM | WQ_UNBOUND | WQ_SYSFS, 1);

		if (!ep->sys->wq) {
			IPAERR("failed to create wq for client %d\n",
					sys_in->client);
			result = -EFAULT;
			goto fail_wq;
		}

		snprintf(buff, IPA_RESOURCE_NAME_MAX, "iparepwq%d",
				sys_in->client);
		ep->sys->repl_wq = alloc_workqueue(buff,
				WQ_MEM_RECLAIM | WQ_UNBOUND | WQ_SYSFS, 1);
		if (!ep->sys->repl_wq) {
			IPAERR("failed to create rep wq for client %d\n",
					sys_in->client);
			result = -EFAULT;
			goto fail_wq2;
		}

		INIT_LIST_HEAD(&ep->sys->head_desc_list);
		INIT_LIST_HEAD(&ep->sys->rcycl_list);
		spin_lock_init(&ep->sys->spinlock);
		hrtimer_init(&ep->sys->db_timer, CLOCK_MONOTONIC,
			HRTIMER_MODE_REL);
		ep->sys->db_timer.function = ipa3_ring_doorbell_timer_fn;

		/* create IPA PM resources for handling polling mode */
		if (ipa3_ctx->use_ipa_pm &&
			IPA_CLIENT_IS_CONS(sys_in->client)) {
			pm_reg.name = ipa_clients_strings[sys_in->client];
			pm_reg.callback = ipa_pm_sys_pipe_cb;
			pm_reg.user_data = ep->sys;
			pm_reg.group = IPA_PM_GROUP_APPS;
			result = ipa_pm_register(&pm_reg, &ep->sys->pm_hdl);
			if (result) {
				IPAERR("failed to create IPA PM client %d\n",
					result);
				goto fail_pm;
			}

			if (IPA_CLIENT_IS_APPS_CONS(sys_in->client)) {
				result = ipa_pm_associate_ipa_cons_to_client(
					ep->sys->pm_hdl, sys_in->client);
				if (result) {
					IPAERR("failed to associate\n");
					goto fail_gen2;
				}
			}

			result = ipa_pm_set_perf_profile(ep->sys->pm_hdl,
				IPA_APPS_BW_FOR_PM);
			if (result) {
				IPAERR("failed to set profile IPA PM client\n");
				goto fail_gen2;
			}
		}
	} else {
		memset(ep->sys, 0, offsetof(struct ipa3_sys_context, ep));
	}

	ep->skip_ep_cfg = sys_in->skip_ep_cfg;
	if (ipa3_assign_policy(sys_in, ep->sys)) {
		IPAERR("failed to sys ctx for client %d\n", sys_in->client);
		result = -ENOMEM;
		goto fail_gen2;
	}

	ep->valid = 1;
	ep->client = sys_in->client;
	ep->client_notify = sys_in->notify;
	ep->napi_enabled = sys_in->napi_enabled;
	ep->priv = sys_in->priv;
	ep->keep_ipa_awake = sys_in->keep_ipa_awake;
	atomic_set(&ep->avail_fifo_desc,
		((sys_in->desc_fifo_sz / IPA_FIFO_ELEMENT_SIZE) - 1));

	if (ep->status.status_en && IPA_CLIENT_IS_CONS(ep->client) &&
	    ep->sys->status_stat == NULL) {
		ep->sys->status_stat =
			kzalloc(sizeof(struct ipa3_status_stats), GFP_KERNEL);
		if (!ep->sys->status_stat) {
			IPAERR("no memory\n");
			goto fail_gen2;
		}
	}

	if (!ep->skip_ep_cfg) {
		if (ipa3_cfg_ep(ipa_ep_idx, &sys_in->ipa_ep_cfg)) {
			IPAERR("fail to configure EP.\n");
			goto fail_gen2;
		}
		if (ipa3_cfg_ep_status(ipa_ep_idx, &ep->status)) {
			IPAERR("fail to configure status of EP.\n");
			goto fail_gen2;
		}
		IPADBG("ep %d configuration successful\n", ipa_ep_idx);
	} else {
		IPADBG("skipping ep %d configuration\n", ipa_ep_idx);
	}

	result = ipa_gsi_setup_channel(sys_in, ep);
	if (result) {
		IPAERR("Failed to setup GSI channel\n");
		goto fail_gen2;
	}

	*clnt_hdl = ipa_ep_idx;

	if (ep->sys->repl_hdlr == ipa3_fast_replenish_rx_cache) {
		ep->sys->repl.capacity = ep->sys->rx_pool_sz + 1;
		ep->sys->repl.cache = kzalloc(ep->sys->repl.capacity *
				sizeof(void *), GFP_KERNEL);
		if (!ep->sys->repl.cache) {
			IPAERR("ep=%d fail to alloc repl cache\n", ipa_ep_idx);
			ep->sys->repl_hdlr = ipa3_replenish_rx_cache;
			ep->sys->repl.capacity = 0;
		} else {
			atomic_set(&ep->sys->repl.head_idx, 0);
			atomic_set(&ep->sys->repl.tail_idx, 0);
			ipa3_wq_repl_rx(&ep->sys->repl_work);
		}
	}

	if (IPA_CLIENT_IS_CONS(sys_in->client))
		ipa3_replenish_rx_cache(ep->sys);

	if (IPA_CLIENT_IS_WLAN_CONS(sys_in->client)) {
		ipa3_alloc_wlan_rx_common_cache(IPA_WLAN_COMM_RX_POOL_LOW);
		atomic_inc(&ipa3_ctx->wc_memb.active_clnt_cnt);
	}

	ipa3_ctx->skip_ep_cfg_shadow[ipa_ep_idx] = ep->skip_ep_cfg;
	if (!ep->skip_ep_cfg && IPA_CLIENT_IS_PROD(sys_in->client)) {
		if (ipa3_ctx->modem_cfg_emb_pipe_flt &&
			sys_in->client == IPA_CLIENT_APPS_WAN_PROD)
			IPADBG("modem cfg emb pipe flt\n");
		else
			ipa3_install_dflt_flt_rules(ipa_ep_idx);
	}

	result = ipa3_enable_data_path(ipa_ep_idx);
	if (result) {
		IPAERR("enable data path failed res=%d ep=%d.\n", result,
			ipa_ep_idx);
		goto fail_gen2;
	}

	result = gsi_start_channel(ep->gsi_chan_hdl);
	if (result != GSI_STATUS_SUCCESS) {
		IPAERR("gsi_start_channel failed res=%d ep=%d.\n", result,
			ipa_ep_idx);
		goto fail_gen2;
	}

	if (!ep->keep_ipa_awake)
		IPA_ACTIVE_CLIENTS_DEC_EP(sys_in->client);

	IPADBG("client %d (ep: %d) connected sys=%p\n", sys_in->client,
			ipa_ep_idx, ep->sys);

	return 0;

fail_gen2:
	if (ipa3_ctx->use_ipa_pm)
		ipa_pm_deregister(ep->sys->pm_hdl);
fail_pm:
	destroy_workqueue(ep->sys->repl_wq);
fail_wq2:
	destroy_workqueue(ep->sys->wq);
fail_wq:
	kfree(ep->sys);
	memset(&ipa3_ctx->ep[ipa_ep_idx], 0, sizeof(struct ipa3_ep_context));
fail_and_disable_clocks:
	IPA_ACTIVE_CLIENTS_DEC_EP(sys_in->client);
fail_gen:
	return result;
}

/**
 * ipa3_teardown_sys_pipe() - Teardown the GPI pipe and cleanup IPA EP
 * @clnt_hdl:	[in] the handle obtained from ipa3_setup_sys_pipe
 *
 * Returns:	0 on success, negative on failure
 */
int ipa3_teardown_sys_pipe(u32 clnt_hdl)
{
	struct ipa3_ep_context *ep;
	int empty;
	int result;
	int i;

	if (clnt_hdl >= ipa3_ctx->ipa_num_pipes ||
	    ipa3_ctx->ep[clnt_hdl].valid == 0) {
		IPAERR("bad parm.\n");
		return -EINVAL;
	}

	ep = &ipa3_ctx->ep[clnt_hdl];

	if (!ep->keep_ipa_awake)
		IPA_ACTIVE_CLIENTS_INC_EP(ipa3_get_client_mapping(clnt_hdl));

	ipa3_disable_data_path(clnt_hdl);

	if (IPA_CLIENT_IS_PROD(ep->client)) {
		do {
			spin_lock_bh(&ep->sys->spinlock);
			empty = list_empty(&ep->sys->head_desc_list);
			spin_unlock_bh(&ep->sys->spinlock);
			if (!empty)
				usleep_range(95, 105);
			else
				break;
		} while (1);
	}

	/* channel stop might fail on timeout if IPA is busy */
	for (i = 0; i < IPA_GSI_CHANNEL_STOP_MAX_RETRY; i++) {
		result = ipa3_stop_gsi_channel(clnt_hdl);
		if (result == GSI_STATUS_SUCCESS)
			break;

		if (result != -GSI_STATUS_AGAIN &&
			result != -GSI_STATUS_TIMED_OUT)
			break;
	}

	if (result != GSI_STATUS_SUCCESS) {
		IPAERR("GSI stop chan err: %d.\n", result);
		ipa_assert();
		return result;
	}

	if (ep->napi_enabled) {
		do {
			usleep_range(95, 105);
		} while (atomic_read(&ep->sys->curr_polling_state));
	}

	if (IPA_CLIENT_IS_CONS(ep->client))
		cancel_delayed_work_sync(&ep->sys->replenish_rx_work);
	flush_workqueue(ep->sys->wq);

	result = ipa3_reset_gsi_channel(clnt_hdl);
	if (result != GSI_STATUS_SUCCESS) {
		IPAERR("Failed to reset chan: %d.\n", result);
		ipa_assert();
		return result;
	}
	dma_free_coherent(ipa3_ctx->pdev,
		ep->gsi_mem_info.chan_ring_len,
		ep->gsi_mem_info.chan_ring_base_vaddr,
		ep->gsi_mem_info.chan_ring_base_addr);
	result = gsi_dealloc_channel(ep->gsi_chan_hdl);
	if (result != GSI_STATUS_SUCCESS) {
		IPAERR("Failed to dealloc chan: %d.\n", result);
		ipa_assert();
		return result;
	}

	/* free event ring only when it is present */
	if (ep->sys->use_comm_evt_ring) {
		ipa3_ctx->gsi_evt_comm_ring_rem +=
			ep->gsi_mem_info.chan_ring_len;
	} else if (ep->gsi_evt_ring_hdl != ~0) {
		result = gsi_reset_evt_ring(ep->gsi_evt_ring_hdl);
		if (result != GSI_STATUS_SUCCESS) {
			IPAERR("Failed to reset evt ring: %d.\n",
					result);
			BUG();
			return result;
		}
		dma_free_coherent(ipa3_ctx->pdev,
			ep->gsi_mem_info.evt_ring_len,
			ep->gsi_mem_info.evt_ring_base_vaddr,
			ep->gsi_mem_info.evt_ring_base_addr);
		result = gsi_dealloc_evt_ring(ep->gsi_evt_ring_hdl);
		if (result != GSI_STATUS_SUCCESS) {
			IPAERR("Failed to dealloc evt ring: %d.\n",
					result);
			BUG();
			return result;
		}
	}
	if (ep->sys->repl_wq)
		flush_workqueue(ep->sys->repl_wq);
	if (IPA_CLIENT_IS_CONS(ep->client))
		ipa3_cleanup_rx(ep->sys);

	if (!ep->skip_ep_cfg && IPA_CLIENT_IS_PROD(ep->client)) {
		if (ipa3_ctx->modem_cfg_emb_pipe_flt &&
			ep->client == IPA_CLIENT_APPS_WAN_PROD)
			IPADBG("modem cfg emb pipe flt\n");
		else
			ipa3_delete_dflt_flt_rules(clnt_hdl);
	}

	if (IPA_CLIENT_IS_WLAN_CONS(ep->client))
		atomic_dec(&ipa3_ctx->wc_memb.active_clnt_cnt);

	memset(&ep->wstats, 0, sizeof(struct ipa3_wlan_stats));

	if (!atomic_read(&ipa3_ctx->wc_memb.active_clnt_cnt))
		ipa3_cleanup_wlan_rx_common_cache();

	ep->valid = 0;
	IPA_ACTIVE_CLIENTS_DEC_EP(ipa3_get_client_mapping(clnt_hdl));

	IPADBG("client (ep: %d) disconnected\n", clnt_hdl);

	return 0;
}

/**
 * ipa3_tx_comp_usr_notify_release() - Callback function which will call the
 * user supplied callback function to release the skb, or release it on
 * its own if no callback function was supplied.
 * @user1
 * @user2
 *
 * This notified callback is for the destination client.
 */
static void ipa3_tx_comp_usr_notify_release(void *user1, int user2)
{
	struct sk_buff *skb = (struct sk_buff *)user1;
	int ep_idx = user2;

	IPADBG_LOW("skb=%p ep=%d\n", skb, ep_idx);

	IPA_STATS_INC_CNT(ipa3_ctx->stats.tx_pkts_compl);

	if (ipa3_ctx->ep[ep_idx].client_notify)
		ipa3_ctx->ep[ep_idx].client_notify(ipa3_ctx->ep[ep_idx].priv,
				IPA_WRITE_DONE, (unsigned long)skb);
	else
		dev_kfree_skb_any(skb);
}

void ipa3_tx_cmd_comp(void *user1, int user2)
{
	ipahal_destroy_imm_cmd(user1);
}

/**
 * ipa3_tx_dp() - Data-path tx handler
 * @dst:	[in] which IPA destination to route tx packets to
 * @skb:	[in] the packet to send
 * @metadata:	[in] TX packet meta-data
 *
 * Data-path tx handler, this is used for both SW data-path which by-passes most
 * IPA HW blocks AND the regular HW data-path for WLAN AMPDU traffic only. If
 * dst is a "valid" CONS type, then SW data-path is used. If dst is the
 * WLAN_AMPDU PROD type, then HW data-path for WLAN AMPDU is used. Anything else
 * is an error. For errors, client needs to free the skb as needed. For success,
 * IPA driver will later invoke client callback if one was supplied. That
 * callback should free the skb. If no callback supplied, IPA driver will free
 * the skb internally
 *
 * The function will use two descriptors for this send command
 * (for A5_WLAN_AMPDU_PROD only one desciprtor will be sent),
 * the first descriptor will be used to inform the IPA hardware that
 * apps need to push data into the IPA (IP_PACKET_INIT immediate command).
 * Once this send was done from transport point-of-view the IPA driver will
 * get notified by the supplied callback.
 *
 * Returns:	0 on success, negative on failure
 */
int ipa3_tx_dp(enum ipa_client_type dst, struct sk_buff *skb,
		struct ipa_tx_meta *meta)
{
	struct ipa3_desc *desc;
	struct ipa3_desc _desc[3];
	int dst_ep_idx;
	struct ipahal_imm_cmd_pyld *cmd_pyld = NULL;
	struct ipa3_sys_context *sys;
	int src_ep_idx;
	int num_frags, f;
	const struct ipa_gsi_ep_config *gsi_ep;
	int data_idx;

	if (unlikely(!ipa3_ctx)) {
		IPAERR("IPA3 driver was not initialized\n");
		return -EINVAL;
	}

	if (skb->len == 0) {
		IPAERR("packet size is 0\n");
		return -EINVAL;
	}

	/*
	 * USB_CONS: PKT_INIT ep_idx = dst pipe
	 * Q6_CONS: PKT_INIT ep_idx = sender pipe
	 * A5_LAN_WAN_PROD: HW path ep_idx = sender pipe
	 *
	 * LAN TX: all PKT_INIT
	 * WAN TX: PKT_INIT (cmd) + HW (data)
	 *
	 */
	if (IPA_CLIENT_IS_CONS(dst)) {
		src_ep_idx = ipa3_get_ep_mapping(IPA_CLIENT_APPS_LAN_PROD);
		if (-1 == src_ep_idx) {
			IPAERR("Client %u is not mapped\n",
				IPA_CLIENT_APPS_LAN_PROD);
			goto fail_gen;
		}
		dst_ep_idx = ipa3_get_ep_mapping(dst);
	} else {
		src_ep_idx = ipa3_get_ep_mapping(dst);
		if (-1 == src_ep_idx) {
			IPAERR("Client %u is not mapped\n", dst);
			goto fail_gen;
		}
		if (meta && meta->pkt_init_dst_ep_valid)
			dst_ep_idx = meta->pkt_init_dst_ep;
		else
			dst_ep_idx = -1;
	}

	sys = ipa3_ctx->ep[src_ep_idx].sys;

	if (!sys->ep->valid) {
		IPAERR("pipe not valid\n");
		goto fail_gen;
	}

	num_frags = skb_shinfo(skb)->nr_frags;
	/*
	 * make sure TLV FIFO supports the needed frags.
	 * 2 descriptors are needed for IP_PACKET_INIT and TAG_STATUS.
	 * 1 descriptor needed for the linear portion of skb.
	 */
	gsi_ep = ipa3_get_gsi_ep_info(ipa3_ctx->ep[src_ep_idx].client);
	if (gsi_ep && (num_frags + 3 > gsi_ep->ipa_if_tlv)) {
		if (skb_linearize(skb)) {
			IPAERR("Failed to linear skb with %d frags\n",
				num_frags);
			goto fail_gen;
		}
		num_frags = 0;
	}
	if (num_frags) {
		/* 1 desc for tag to resolve status out-of-order issue;
		 * 1 desc is needed for the linear portion of skb;
		 * 1 desc may be needed for the PACKET_INIT;
		 * 1 desc for each frag
		 */
		desc = kzalloc(sizeof(*desc) * (num_frags + 3), GFP_ATOMIC);
		if (!desc) {
			IPAERR("failed to alloc desc array\n");
			goto fail_gen;
		}
	} else {
		memset(_desc, 0, 3 * sizeof(struct ipa3_desc));
		desc = &_desc[0];
	}

	if (dst_ep_idx != -1) {
		/* SW data path */
		data_idx = 0;
		if (sys->policy == IPA_POLICY_NOINTR_MODE) {
			/*
			 * For non-interrupt mode channel (where there is no
			 * event ring) TAG STATUS are used for completion
			 * notification. IPA will generate a status packet with
			 * tag info as a result of the TAG STATUS command.
			 */
			desc[data_idx].is_tag_status = true;
			data_idx++;
		}
		desc[data_idx].opcode = ipa3_ctx->pkt_init_imm_opcode;
		desc[data_idx].dma_address_valid = true;
		desc[data_idx].dma_address = ipa3_ctx->pkt_init_imm[dst_ep_idx];
		desc[data_idx].type = IPA_IMM_CMD_DESC;
		desc[data_idx].callback = NULL;
		data_idx++;
		desc[data_idx].pyld = skb->data;
		desc[data_idx].len = skb_headlen(skb);
		desc[data_idx].type = IPA_DATA_DESC_SKB;
		desc[data_idx].callback = ipa3_tx_comp_usr_notify_release;
		desc[data_idx].user1 = skb;
		desc[data_idx].user2 = (meta && meta->pkt_init_dst_ep_valid &&
				meta->pkt_init_dst_ep_remote) ?
				src_ep_idx :
				dst_ep_idx;
		if (meta && meta->dma_address_valid) {
			desc[data_idx].dma_address_valid = true;
			desc[data_idx].dma_address = meta->dma_address;
		}
		data_idx++;

		for (f = 0; f < num_frags; f++) {
			desc[data_idx + f].frag = &skb_shinfo(skb)->frags[f];
			desc[data_idx + f].type = IPA_DATA_DESC_SKB_PAGED;
			desc[data_idx + f].len =
				skb_frag_size(desc[data_idx + f].frag);
		}
		/* don't free skb till frag mappings are released */
		if (num_frags) {
			desc[data_idx + f - 1].callback = desc[2].callback;
			desc[data_idx + f - 1].user1 = desc[2].user1;
			desc[data_idx + f - 1].user2 = desc[2].user2;
			desc[data_idx - 1].callback = NULL;
		}

		if (ipa3_send(sys, num_frags + data_idx, desc, true)) {
			IPAERR("fail to send skb %p num_frags %u SWP\n",
				skb, num_frags);
			goto fail_send;
		}
		IPA_STATS_INC_CNT(ipa3_ctx->stats.tx_sw_pkts);
	} else {
		/* HW data path */
		data_idx = 0;
		if (sys->policy == IPA_POLICY_NOINTR_MODE) {
			/*
			 * For non-interrupt mode channel (where there is no
			 * event ring) TAG STATUS are used for completion
			 * notification. IPA will generate a status packet with
			 * tag info as a result of the TAG STATUS command.
			 */
			desc[data_idx].is_tag_status = true;
			data_idx++;
		}
		desc[data_idx].pyld = skb->data;
		desc[data_idx].len = skb_headlen(skb);
		desc[data_idx].type = IPA_DATA_DESC_SKB;
		desc[data_idx].callback = ipa3_tx_comp_usr_notify_release;
		desc[data_idx].user1 = skb;
		desc[data_idx].user2 = src_ep_idx;

		if (meta && meta->dma_address_valid) {
			desc[data_idx].dma_address_valid = true;
			desc[data_idx].dma_address = meta->dma_address;
		}
		if (num_frags == 0) {
			if (ipa3_send(sys, data_idx + 1, desc, true)) {
				IPAERR("fail to send skb %p HWP\n", skb);
				goto fail_mem;
			}
		} else {
			for (f = 0; f < num_frags; f++) {
				desc[data_idx+f+1].frag =
					&skb_shinfo(skb)->frags[f];
				desc[data_idx+f+1].type =
					IPA_DATA_DESC_SKB_PAGED;
				desc[data_idx+f+1].len =
					skb_frag_size(desc[data_idx+f+1].frag);
			}
			/* don't free skb till frag mappings are released */
			desc[data_idx+f].callback = desc[data_idx].callback;
			desc[data_idx+f].user1 = desc[data_idx].user1;
			desc[data_idx+f].user2 = desc[data_idx].user2;
			desc[data_idx].callback = NULL;

			if (ipa3_send(sys, num_frags + data_idx + 1,
				desc, true)) {
				IPAERR("fail to send skb %p num_frags %u HWP\n",
					skb, num_frags);
				goto fail_mem;
			}
		}
		IPA_STATS_INC_CNT(ipa3_ctx->stats.tx_hw_pkts);
	}

	if (num_frags) {
		kfree(desc);
		IPA_STATS_INC_CNT(ipa3_ctx->stats.tx_non_linear);
	}
	return 0;

fail_send:
	ipahal_destroy_imm_cmd(cmd_pyld);
fail_mem:
	if (num_frags)
		kfree(desc);
fail_gen:
	return -EFAULT;
}

static void ipa3_wq_handle_rx(struct work_struct *work)
{
	struct ipa3_sys_context *sys;

	sys = container_of(work, struct ipa3_sys_context, work);

	if (sys->ep->napi_enabled) {
		if (!ipa3_ctx->use_ipa_pm)
			IPA_ACTIVE_CLIENTS_INC_SPECIAL("NAPI");
		sys->ep->client_notify(sys->ep->priv,
				IPA_CLIENT_START_POLL, 0);
	} else
		ipa3_handle_rx(sys);
}

static void ipa3_wq_repl_rx(struct work_struct *work)
{
	struct ipa3_sys_context *sys;
	void *ptr;
	struct ipa3_rx_pkt_wrapper *rx_pkt;
	gfp_t flag = GFP_KERNEL;
	u32 next;
	u32 curr;

	sys = container_of(work, struct ipa3_sys_context, repl_work);
	curr = atomic_read(&sys->repl.tail_idx);

begin:
	while (1) {
		next = (curr + 1) % sys->repl.capacity;
		if (next == atomic_read(&sys->repl.head_idx))
			goto fail_kmem_cache_alloc;

		rx_pkt = kmem_cache_zalloc(ipa3_ctx->rx_pkt_wrapper_cache,
					   flag);
		if (!rx_pkt) {
			pr_err_ratelimited("%s fail alloc rx wrapper sys=%p\n",
					__func__, sys);
			goto fail_kmem_cache_alloc;
		}

		INIT_LIST_HEAD(&rx_pkt->link);
		INIT_WORK(&rx_pkt->work, ipa3_wq_rx_avail);
		rx_pkt->sys = sys;

		rx_pkt->data.skb = sys->get_skb(sys->rx_buff_sz, flag);
		if (rx_pkt->data.skb == NULL) {
			pr_err_ratelimited("%s fail alloc skb sys=%p\n",
					__func__, sys);
			goto fail_skb_alloc;
		}
		ptr = skb_put(rx_pkt->data.skb, sys->rx_buff_sz);
		rx_pkt->data.dma_addr = dma_map_single(ipa3_ctx->pdev, ptr,
						     sys->rx_buff_sz,
						     DMA_FROM_DEVICE);
		if (dma_mapping_error(ipa3_ctx->pdev, rx_pkt->data.dma_addr)) {
			pr_err_ratelimited("%s dma map fail %p for %p sys=%p\n",
			       __func__, (void *)rx_pkt->data.dma_addr,
			       ptr, sys);
			goto fail_dma_mapping;
		}

		sys->repl.cache[curr] = rx_pkt;
		curr = next;
		/* ensure write is done before setting tail index */
		mb();
		atomic_set(&sys->repl.tail_idx, next);
	}

	return;

fail_dma_mapping:
	sys->free_skb(rx_pkt->data.skb);
fail_skb_alloc:
	kmem_cache_free(ipa3_ctx->rx_pkt_wrapper_cache, rx_pkt);
fail_kmem_cache_alloc:
	if (atomic_read(&sys->repl.tail_idx) ==
			atomic_read(&sys->repl.head_idx)) {
		if (sys->ep->client == IPA_CLIENT_APPS_WAN_CONS)
			IPA_STATS_INC_CNT(ipa3_ctx->stats.wan_repl_rx_empty);
		else if (sys->ep->client == IPA_CLIENT_APPS_LAN_CONS)
			IPA_STATS_INC_CNT(ipa3_ctx->stats.lan_repl_rx_empty);
		else
			WARN_ON(1);
		pr_err_ratelimited("%s sys=%p repl ring empty\n",
				__func__, sys);
		goto begin;
	}
}

static void ipa3_replenish_wlan_rx_cache(struct ipa3_sys_context *sys)
{
	struct ipa3_rx_pkt_wrapper *rx_pkt = NULL;
	struct ipa3_rx_pkt_wrapper *tmp;
	int ret;
	struct gsi_xfer_elem gsi_xfer_elem_one;
	u32 rx_len_cached = 0;

	IPADBG_LOW("\n");

	spin_lock_bh(&ipa3_ctx->wc_memb.wlan_spinlock);
	rx_len_cached = sys->len;

	if (rx_len_cached < sys->rx_pool_sz) {
		list_for_each_entry_safe(rx_pkt, tmp,
			&ipa3_ctx->wc_memb.wlan_comm_desc_list, link) {
			list_del(&rx_pkt->link);

			if (ipa3_ctx->wc_memb.wlan_comm_free_cnt > 0)
				ipa3_ctx->wc_memb.wlan_comm_free_cnt--;

			INIT_LIST_HEAD(&rx_pkt->link);
			rx_pkt->len = 0;
			rx_pkt->sys = sys;

			list_add_tail(&rx_pkt->link, &sys->head_desc_list);
			memset(&gsi_xfer_elem_one, 0,
				sizeof(gsi_xfer_elem_one));
			gsi_xfer_elem_one.addr = rx_pkt->data.dma_addr;
			gsi_xfer_elem_one.len = IPA_WLAN_RX_BUFF_SZ;
			gsi_xfer_elem_one.flags |= GSI_XFER_FLAG_EOT;
			gsi_xfer_elem_one.flags |= GSI_XFER_FLAG_EOB;
			gsi_xfer_elem_one.type = GSI_XFER_ELEM_DATA;
			gsi_xfer_elem_one.xfer_user_data = rx_pkt;

			ret = gsi_queue_xfer(sys->ep->gsi_chan_hdl, 1,
				&gsi_xfer_elem_one, true);

			if (ret) {
				IPAERR("failed to provide buffer: %d\n", ret);
				goto fail_provide_rx_buffer;
			}

			rx_len_cached = ++sys->len;

			if (rx_len_cached >= sys->rx_pool_sz) {
				spin_unlock_bh(
					&ipa3_ctx->wc_memb.wlan_spinlock);
				return;
			}
		}
	}
	spin_unlock_bh(&ipa3_ctx->wc_memb.wlan_spinlock);

	if (rx_len_cached < sys->rx_pool_sz &&
			ipa3_ctx->wc_memb.wlan_comm_total_cnt <
			 IPA_WLAN_COMM_RX_POOL_HIGH) {
		ipa3_replenish_rx_cache(sys);
		ipa3_ctx->wc_memb.wlan_comm_total_cnt +=
			(sys->rx_pool_sz - rx_len_cached);
	}

	return;

fail_provide_rx_buffer:
	list_del(&rx_pkt->link);
	spin_unlock_bh(&ipa3_ctx->wc_memb.wlan_spinlock);
}

static void ipa3_cleanup_wlan_rx_common_cache(void)
{
	struct ipa3_rx_pkt_wrapper *rx_pkt;
	struct ipa3_rx_pkt_wrapper *tmp;

	spin_lock_bh(&ipa3_ctx->wc_memb.wlan_spinlock);

	list_for_each_entry_safe(rx_pkt, tmp,
		&ipa3_ctx->wc_memb.wlan_comm_desc_list, link) {
		list_del(&rx_pkt->link);
		dma_unmap_single(ipa3_ctx->pdev, rx_pkt->data.dma_addr,
				IPA_WLAN_COMM_RX_POOL_LOW, DMA_FROM_DEVICE);
		dev_kfree_skb_any(rx_pkt->data.skb);
		kmem_cache_free(ipa3_ctx->rx_pkt_wrapper_cache, rx_pkt);
		ipa3_ctx->wc_memb.wlan_comm_free_cnt--;
		ipa3_ctx->wc_memb.wlan_comm_total_cnt--;
	}
	ipa3_ctx->wc_memb.total_tx_pkts_freed = 0;

	if (ipa3_ctx->wc_memb.wlan_comm_free_cnt != 0)
		IPAERR("wlan comm buff free cnt: %d\n",
			ipa3_ctx->wc_memb.wlan_comm_free_cnt);

	if (ipa3_ctx->wc_memb.wlan_comm_total_cnt != 0)
		IPAERR("wlan comm buff total cnt: %d\n",
			ipa3_ctx->wc_memb.wlan_comm_total_cnt);

	spin_unlock_bh(&ipa3_ctx->wc_memb.wlan_spinlock);

}

static void ipa3_alloc_wlan_rx_common_cache(u32 size)
{
	void *ptr;
	struct ipa3_rx_pkt_wrapper *rx_pkt;
	int rx_len_cached = 0;
	gfp_t flag = GFP_NOWAIT | __GFP_NOWARN;

	rx_len_cached = ipa3_ctx->wc_memb.wlan_comm_total_cnt;
	while (rx_len_cached < size) {
		rx_pkt = kmem_cache_zalloc(ipa3_ctx->rx_pkt_wrapper_cache,
					   flag);
		if (!rx_pkt) {
			IPAERR("failed to alloc rx wrapper\n");
			goto fail_kmem_cache_alloc;
		}

		INIT_LIST_HEAD(&rx_pkt->link);
		INIT_WORK(&rx_pkt->work, ipa3_wq_rx_avail);

		rx_pkt->data.skb =
			ipa3_get_skb_ipa_rx(IPA_WLAN_RX_BUFF_SZ,
						flag);
		if (rx_pkt->data.skb == NULL) {
			IPAERR("failed to alloc skb\n");
			goto fail_skb_alloc;
		}
		ptr = skb_put(rx_pkt->data.skb, IPA_WLAN_RX_BUFF_SZ);
		rx_pkt->data.dma_addr = dma_map_single(ipa3_ctx->pdev, ptr,
				IPA_WLAN_RX_BUFF_SZ, DMA_FROM_DEVICE);
		if (dma_mapping_error(ipa3_ctx->pdev, rx_pkt->data.dma_addr)) {
			IPAERR("dma_map_single failure %p for %p\n",
			       (void *)rx_pkt->data.dma_addr, ptr);
			goto fail_dma_mapping;
		}

		spin_lock_bh(&ipa3_ctx->wc_memb.wlan_spinlock);
		list_add_tail(&rx_pkt->link,
			&ipa3_ctx->wc_memb.wlan_comm_desc_list);
		rx_len_cached = ++ipa3_ctx->wc_memb.wlan_comm_total_cnt;

		ipa3_ctx->wc_memb.wlan_comm_free_cnt++;
		spin_unlock_bh(&ipa3_ctx->wc_memb.wlan_spinlock);

	}

	return;

fail_dma_mapping:
	dev_kfree_skb_any(rx_pkt->data.skb);
fail_skb_alloc:
	kmem_cache_free(ipa3_ctx->rx_pkt_wrapper_cache, rx_pkt);
fail_kmem_cache_alloc:
	return;
}


/**
 * ipa3_replenish_rx_cache() - Replenish the Rx packets cache.
 *
 * The function allocates buffers in the rx_pkt_wrapper_cache cache until there
 * are IPA_RX_POOL_CEIL buffers in the cache.
 *   - Allocate a buffer in the cache
 *   - Initialized the packets link
 *   - Initialize the packets work struct
 *   - Allocate the packets socket buffer (skb)
 *   - Fill the packets skb with data
 *   - Make the packet DMAable
 *   - Add the packet to the system pipe linked list
 */
static void ipa3_replenish_rx_cache(struct ipa3_sys_context *sys)
{
	void *ptr;
	struct ipa3_rx_pkt_wrapper *rx_pkt;
	int ret;
	int rx_len_cached = 0;
	struct gsi_xfer_elem gsi_xfer_elem_one;
	gfp_t flag = GFP_NOWAIT | __GFP_NOWARN;

	rx_len_cached = sys->len;

	while (rx_len_cached < sys->rx_pool_sz) {
		rx_pkt = kmem_cache_zalloc(ipa3_ctx->rx_pkt_wrapper_cache,
					   flag);
		if (!rx_pkt) {
			IPAERR("failed to alloc rx wrapper\n");
			goto fail_kmem_cache_alloc;
		}

		INIT_LIST_HEAD(&rx_pkt->link);
		INIT_WORK(&rx_pkt->work, ipa3_wq_rx_avail);
		rx_pkt->sys = sys;

		rx_pkt->data.skb = sys->get_skb(sys->rx_buff_sz, flag);
		if (rx_pkt->data.skb == NULL) {
			IPAERR("failed to alloc skb\n");
			goto fail_skb_alloc;
		}
		ptr = skb_put(rx_pkt->data.skb, sys->rx_buff_sz);
		rx_pkt->data.dma_addr = dma_map_single(ipa3_ctx->pdev, ptr,
						     sys->rx_buff_sz,
						     DMA_FROM_DEVICE);
		if (dma_mapping_error(ipa3_ctx->pdev, rx_pkt->data.dma_addr)) {
			IPAERR("dma_map_single failure %p for %p\n",
			       (void *)rx_pkt->data.dma_addr, ptr);
			goto fail_dma_mapping;
		}

		list_add_tail(&rx_pkt->link, &sys->head_desc_list);
		rx_len_cached = ++sys->len;

		memset(&gsi_xfer_elem_one, 0,
			sizeof(gsi_xfer_elem_one));
		gsi_xfer_elem_one.addr = rx_pkt->data.dma_addr;
		gsi_xfer_elem_one.len = sys->rx_buff_sz;
		gsi_xfer_elem_one.flags |= GSI_XFER_FLAG_EOT;
		gsi_xfer_elem_one.flags |= GSI_XFER_FLAG_EOB;
		gsi_xfer_elem_one.type = GSI_XFER_ELEM_DATA;
		gsi_xfer_elem_one.xfer_user_data = rx_pkt;

		ret = gsi_queue_xfer(sys->ep->gsi_chan_hdl,
				1, &gsi_xfer_elem_one, false);
		if (ret != GSI_STATUS_SUCCESS) {
			IPAERR("failed to provide buffer: %d\n",
				ret);
			goto fail_provide_rx_buffer;
		}

		/*
		 * As doorbell is a costly operation, notify to GSI
		 * of new buffers if threshold is exceeded
		 */
		if (++sys->len_pending_xfer >= IPA_REPL_XFER_THRESH) {
			sys->len_pending_xfer = 0;
			gsi_start_xfer(sys->ep->gsi_chan_hdl);
		}
	}

	return;

fail_provide_rx_buffer:
	list_del(&rx_pkt->link);
	rx_len_cached = --sys->len;
	dma_unmap_single(ipa3_ctx->pdev, rx_pkt->data.dma_addr,
			sys->rx_buff_sz, DMA_FROM_DEVICE);
fail_dma_mapping:
	sys->free_skb(rx_pkt->data.skb);
fail_skb_alloc:
	kmem_cache_free(ipa3_ctx->rx_pkt_wrapper_cache, rx_pkt);
fail_kmem_cache_alloc:
	if (rx_len_cached - sys->len_pending_xfer == 0)
		queue_delayed_work(sys->wq, &sys->replenish_rx_work,
				msecs_to_jiffies(1));
}

static void ipa3_replenish_rx_cache_recycle(struct ipa3_sys_context *sys)
{
	void *ptr;
	struct ipa3_rx_pkt_wrapper *rx_pkt;
	int ret;
	int rx_len_cached = 0;
	struct gsi_xfer_elem gsi_xfer_elem_one;
	gfp_t flag = GFP_NOWAIT | __GFP_NOWARN;

	rx_len_cached = sys->len;

	while (rx_len_cached < sys->rx_pool_sz) {
		if (list_empty(&sys->rcycl_list)) {
			rx_pkt = kmem_cache_zalloc(
				ipa3_ctx->rx_pkt_wrapper_cache, flag);
			if (!rx_pkt) {
				IPAERR("failed to alloc rx wrapper\n");
				goto fail_kmem_cache_alloc;
			}

			INIT_LIST_HEAD(&rx_pkt->link);
			INIT_WORK(&rx_pkt->work, ipa3_wq_rx_avail);
			rx_pkt->sys = sys;

			rx_pkt->data.skb = sys->get_skb(sys->rx_buff_sz, flag);
			if (rx_pkt->data.skb == NULL) {
				IPAERR("failed to alloc skb\n");
				kmem_cache_free(ipa3_ctx->rx_pkt_wrapper_cache,
					rx_pkt);
				goto fail_kmem_cache_alloc;
			}
			ptr = skb_put(rx_pkt->data.skb, sys->rx_buff_sz);
			rx_pkt->data.dma_addr = dma_map_single(ipa3_ctx->pdev,
				ptr, sys->rx_buff_sz, DMA_FROM_DEVICE);
			if (dma_mapping_error(ipa3_ctx->pdev,
				rx_pkt->data.dma_addr)) {
				IPAERR("dma_map_single failure %p for %p\n",
					(void *)rx_pkt->data.dma_addr, ptr);
				goto fail_dma_mapping;
			}
		} else {
			spin_lock_bh(&sys->spinlock);
			rx_pkt = list_first_entry(&sys->rcycl_list,
				struct ipa3_rx_pkt_wrapper, link);
			list_del(&rx_pkt->link);
			spin_unlock_bh(&sys->spinlock);
			INIT_LIST_HEAD(&rx_pkt->link);
			ptr = skb_put(rx_pkt->data.skb, sys->rx_buff_sz);
			rx_pkt->data.dma_addr = dma_map_single(ipa3_ctx->pdev,
				ptr, sys->rx_buff_sz, DMA_FROM_DEVICE);
			if (dma_mapping_error(ipa3_ctx->pdev,
				rx_pkt->data.dma_addr)) {
				IPAERR("dma_map_single failure %p for %p\n",
					(void *)rx_pkt->data.dma_addr, ptr);
				goto fail_dma_mapping;
			}
		}

		list_add_tail(&rx_pkt->link, &sys->head_desc_list);
		rx_len_cached = ++sys->len;
		memset(&gsi_xfer_elem_one, 0,
			sizeof(gsi_xfer_elem_one));
		gsi_xfer_elem_one.addr = rx_pkt->data.dma_addr;
		gsi_xfer_elem_one.len = sys->rx_buff_sz;
		gsi_xfer_elem_one.flags |= GSI_XFER_FLAG_EOT;
		gsi_xfer_elem_one.flags |= GSI_XFER_FLAG_EOB;
		gsi_xfer_elem_one.type = GSI_XFER_ELEM_DATA;
		gsi_xfer_elem_one.xfer_user_data = rx_pkt;

		ret = gsi_queue_xfer(sys->ep->gsi_chan_hdl,
				1, &gsi_xfer_elem_one, false);
		if (ret != GSI_STATUS_SUCCESS) {
			IPAERR("failed to provide buffer: %d\n",
				ret);
			goto fail_provide_rx_buffer;
		}

		/*
		 * As doorbell is a costly operation, notify to GSI
		 * of new buffers if threshold is exceeded
		 */
		if (++sys->len_pending_xfer >= IPA_REPL_XFER_THRESH) {
			sys->len_pending_xfer = 0;
			gsi_start_xfer(sys->ep->gsi_chan_hdl);
		}
	}

	return;
fail_provide_rx_buffer:
	rx_len_cached = --sys->len;
	list_del(&rx_pkt->link);
	INIT_LIST_HEAD(&rx_pkt->link);
	dma_unmap_single(ipa3_ctx->pdev, rx_pkt->data.dma_addr,
		sys->rx_buff_sz, DMA_FROM_DEVICE);
fail_dma_mapping:
	spin_lock_bh(&sys->spinlock);
	list_add_tail(&rx_pkt->link, &sys->rcycl_list);
	INIT_LIST_HEAD(&rx_pkt->link);
	spin_unlock_bh(&sys->spinlock);
fail_kmem_cache_alloc:
	if (rx_len_cached - sys->len_pending_xfer == 0)
		queue_delayed_work(sys->wq, &sys->replenish_rx_work,
		msecs_to_jiffies(1));
}

static void ipa3_fast_replenish_rx_cache(struct ipa3_sys_context *sys)
{
	struct ipa3_rx_pkt_wrapper *rx_pkt;
	int ret;
	int rx_len_cached = 0;
	struct gsi_xfer_elem gsi_xfer_elem_one;
	u32 curr;

	spin_lock_bh(&sys->spinlock);

	rx_len_cached = sys->len;
	curr = atomic_read(&sys->repl.head_idx);

	while (rx_len_cached < sys->rx_pool_sz) {
		if (curr == atomic_read(&sys->repl.tail_idx))
			break;

		rx_pkt = sys->repl.cache[curr];
		list_add_tail(&rx_pkt->link, &sys->head_desc_list);

		memset(&gsi_xfer_elem_one, 0,
			sizeof(gsi_xfer_elem_one));
		gsi_xfer_elem_one.addr = rx_pkt->data.dma_addr;
		gsi_xfer_elem_one.len = sys->rx_buff_sz;
		gsi_xfer_elem_one.flags |= GSI_XFER_FLAG_EOT;
		gsi_xfer_elem_one.flags |= GSI_XFER_FLAG_EOB;
		gsi_xfer_elem_one.type = GSI_XFER_ELEM_DATA;
		gsi_xfer_elem_one.xfer_user_data = rx_pkt;

		ret = gsi_queue_xfer(sys->ep->gsi_chan_hdl, 1,
			&gsi_xfer_elem_one, false);
		if (ret != GSI_STATUS_SUCCESS) {
			IPAERR("failed to provide buffer: %d\n",
				ret);
			break;
		}

		/*
		 * As doorbell is a costly operation, notify to GSI
		 * of new buffers if threshold is exceeded
		 */
		if (++sys->len_pending_xfer >= IPA_REPL_XFER_THRESH) {
			sys->len_pending_xfer = 0;
			gsi_start_xfer(sys->ep->gsi_chan_hdl);
		}

		rx_len_cached = ++sys->len;
		curr = (curr + 1) % sys->repl.capacity;
		/* ensure write is done before setting head index */
		mb();
		atomic_set(&sys->repl.head_idx, curr);
	}
	spin_unlock_bh(&sys->spinlock);

	queue_work(sys->repl_wq, &sys->repl_work);

	if (rx_len_cached - sys->len_pending_xfer
		<= IPA_DEFAULT_SYS_YELLOW_WM) {
		if (sys->ep->client == IPA_CLIENT_APPS_WAN_CONS)
			IPA_STATS_INC_CNT(ipa3_ctx->stats.wan_rx_empty);
		else if (sys->ep->client == IPA_CLIENT_APPS_LAN_CONS)
			IPA_STATS_INC_CNT(ipa3_ctx->stats.lan_rx_empty);
		else
			WARN_ON(1);
		queue_delayed_work(sys->wq, &sys->replenish_rx_work,
				msecs_to_jiffies(1));
	}
}

static void ipa3_replenish_rx_work_func(struct work_struct *work)
{
	struct delayed_work *dwork;
	struct ipa3_sys_context *sys;

	dwork = container_of(work, struct delayed_work, work);
	sys = container_of(dwork, struct ipa3_sys_context, replenish_rx_work);
	IPA_ACTIVE_CLIENTS_INC_SIMPLE();
	sys->repl_hdlr(sys);
	IPA_ACTIVE_CLIENTS_DEC_SIMPLE();
}

/**
 * ipa3_cleanup_rx() - release RX queue resources
 *
 */
static void ipa3_cleanup_rx(struct ipa3_sys_context *sys)
{
	struct ipa3_rx_pkt_wrapper *rx_pkt;
	struct ipa3_rx_pkt_wrapper *r;
	u32 head;
	u32 tail;

	list_for_each_entry_safe(rx_pkt, r,
				 &sys->head_desc_list, link) {
		list_del(&rx_pkt->link);
		dma_unmap_single(ipa3_ctx->pdev, rx_pkt->data.dma_addr,
			sys->rx_buff_sz, DMA_FROM_DEVICE);
		sys->free_skb(rx_pkt->data.skb);
		kmem_cache_free(ipa3_ctx->rx_pkt_wrapper_cache, rx_pkt);
	}

	list_for_each_entry_safe(rx_pkt, r,
				 &sys->rcycl_list, link) {
		list_del(&rx_pkt->link);
		dma_unmap_single(ipa3_ctx->pdev, rx_pkt->data.dma_addr,
			sys->rx_buff_sz, DMA_FROM_DEVICE);
		sys->free_skb(rx_pkt->data.skb);
		kmem_cache_free(ipa3_ctx->rx_pkt_wrapper_cache, rx_pkt);
	}

	if (sys->repl.cache) {
		head = atomic_read(&sys->repl.head_idx);
		tail = atomic_read(&sys->repl.tail_idx);
		while (head != tail) {
			rx_pkt = sys->repl.cache[head];
			dma_unmap_single(ipa3_ctx->pdev, rx_pkt->data.dma_addr,
					sys->rx_buff_sz, DMA_FROM_DEVICE);
			sys->free_skb(rx_pkt->data.skb);
			kmem_cache_free(ipa3_ctx->rx_pkt_wrapper_cache, rx_pkt);
			head = (head + 1) % sys->repl.capacity;
		}
		kfree(sys->repl.cache);
	}
}

static struct sk_buff *ipa3_skb_copy_for_client(struct sk_buff *skb, int len)
{
	struct sk_buff *skb2 = NULL;

	skb2 = __dev_alloc_skb(len + IPA_RX_BUFF_CLIENT_HEADROOM, GFP_KERNEL);
	if (likely(skb2)) {
		/* Set the data pointer */
		skb_reserve(skb2, IPA_RX_BUFF_CLIENT_HEADROOM);
		memcpy(skb2->data, skb->data, len);
		skb2->len = len;
		skb_set_tail_pointer(skb2, len);
	}

	return skb2;
}

static int ipa3_lan_rx_pyld_hdlr(struct sk_buff *skb,
		struct ipa3_sys_context *sys)
{
	int rc = 0;
	struct ipahal_pkt_status status;
	u32 pkt_status_sz;
	struct sk_buff *skb2;
	int pad_len_byte;
	int len;
	unsigned char *buf;
	int src_pipe;
	unsigned int used = *(unsigned int *)skb->cb;
	unsigned int used_align = ALIGN(used, 32);
	unsigned long unused = IPA_GENERIC_RX_BUFF_BASE_SZ - used;
	struct ipa3_tx_pkt_wrapper *tx_pkt = NULL;
	unsigned long ptr;

	IPA_DUMP_BUFF(skb->data, 0, skb->len);

	if (skb->len == 0) {
		IPAERR("ZLT\n");
		return rc;
	}

	if (sys->len_partial) {
		IPADBG_LOW("len_partial %d\n", sys->len_partial);
		buf = skb_push(skb, sys->len_partial);
		memcpy(buf, sys->prev_skb->data, sys->len_partial);
		sys->len_partial = 0;
		sys->free_skb(sys->prev_skb);
		sys->prev_skb = NULL;
		goto begin;
	}

	/* this pipe has TX comp (status only) + mux-ed LAN RX data
	 * (status+data)
	 */
	if (sys->len_rem) {
		IPADBG_LOW("rem %d skb %d pad %d\n", sys->len_rem, skb->len,
				sys->len_pad);
		if (sys->len_rem <= skb->len) {
			if (sys->prev_skb) {
				skb2 = skb_copy_expand(sys->prev_skb, 0,
						sys->len_rem, GFP_KERNEL);
				if (likely(skb2)) {
					memcpy(skb_put(skb2, sys->len_rem),
						skb->data, sys->len_rem);
					skb_trim(skb2,
						skb2->len - sys->len_pad);
					skb2->truesize = skb2->len +
						sizeof(struct sk_buff);
					if (sys->drop_packet)
						dev_kfree_skb_any(skb2);
					else
						sys->ep->client_notify(
							sys->ep->priv,
							IPA_RECEIVE,
							(unsigned long)(skb2));
				} else {
					IPAERR("copy expand failed\n");
				}
				dev_kfree_skb_any(sys->prev_skb);
			}
			skb_pull(skb, sys->len_rem);
			sys->prev_skb = NULL;
			sys->len_rem = 0;
			sys->len_pad = 0;
		} else {
			if (sys->prev_skb) {
				skb2 = skb_copy_expand(sys->prev_skb, 0,
					skb->len, GFP_KERNEL);
				if (likely(skb2)) {
					memcpy(skb_put(skb2, skb->len),
						skb->data, skb->len);
				} else {
					IPAERR("copy expand failed\n");
				}
				dev_kfree_skb_any(sys->prev_skb);
				sys->prev_skb = skb2;
			}
			sys->len_rem -= skb->len;
			return rc;
		}
	}

begin:
	pkt_status_sz = ipahal_pkt_status_get_size();
	while (skb->len) {
		sys->drop_packet = false;
		IPADBG_LOW("LEN_REM %d\n", skb->len);

		if (skb->len < pkt_status_sz) {
			WARN_ON(sys->prev_skb != NULL);
			IPADBG_LOW("status straddles buffer\n");
			sys->prev_skb = skb_copy(skb, GFP_KERNEL);
			sys->len_partial = skb->len;
			return rc;
		}

		ipahal_pkt_status_parse(skb->data, &status);
		IPADBG_LOW("STATUS opcode=%d src=%d dst=%d len=%d\n",
				status.status_opcode, status.endp_src_idx,
				status.endp_dest_idx, status.pkt_len);
		if (sys->status_stat) {
			sys->status_stat->status[sys->status_stat->curr] =
				status;
			sys->status_stat->curr++;
			if (sys->status_stat->curr == IPA_MAX_STATUS_STAT_NUM)
				sys->status_stat->curr = 0;
		}

		if ((status.status_opcode !=
			IPAHAL_PKT_STATUS_OPCODE_DROPPED_PACKET) &&
			(status.status_opcode !=
			IPAHAL_PKT_STATUS_OPCODE_PACKET) &&
			(status.status_opcode !=
			IPAHAL_PKT_STATUS_OPCODE_SUSPENDED_PACKET) &&
			(status.status_opcode !=
			IPAHAL_PKT_STATUS_OPCODE_PACKET_2ND_PASS)) {
			IPAERR("unsupported opcode(%d)\n",
				status.status_opcode);
			skb_pull(skb, pkt_status_sz);
			continue;
		}
		IPA_STATS_EXCP_CNT(status.exception,
				ipa3_ctx->stats.rx_excp_pkts);
		if (status.endp_dest_idx >= ipa3_ctx->ipa_num_pipes ||
			status.endp_src_idx >= ipa3_ctx->ipa_num_pipes) {
			IPAERR("status fields invalid\n");
			IPAERR("STATUS opcode=%d src=%d dst=%d len=%d\n",
				status.status_opcode, status.endp_src_idx,
				status.endp_dest_idx, status.pkt_len);
			WARN_ON(1);
			BUG();
		}
		if (IPAHAL_PKT_STATUS_MASK_FLAG_VAL(
			IPAHAL_PKT_STATUS_MASK_TAG_VALID_SHFT, &status)) {
			struct ipa3_tag_completion *comp;

			IPADBG_LOW("TAG packet arrived\n");
			if (status.tag_info == IPA_COOKIE) {
				skb_pull(skb, pkt_status_sz);
				if (skb->len < sizeof(comp)) {
					IPAERR("TAG arrived without packet\n");
					return rc;
				}
				memcpy(&comp, skb->data, sizeof(comp));
				skb_pull(skb, sizeof(comp) +
						IPA_SIZE_DL_CSUM_META_TRAILER);
				complete(&comp->comp);
				if (atomic_dec_return(&comp->cnt) == 0)
					kfree(comp);
				continue;
			} else {
				ptr = tag_to_pointer_wa(status.tag_info);
				tx_pkt = (struct ipa3_tx_pkt_wrapper *)ptr;
				IPADBG_LOW("tx_pkt recv = %p\n", tx_pkt);
			}
		}
		if (status.pkt_len == 0) {
			IPADBG_LOW("Skip aggr close status\n");
			skb_pull(skb, pkt_status_sz);
			IPA_STATS_INC_CNT(ipa3_ctx->stats.aggr_close);
			IPA_STATS_DEC_CNT(ipa3_ctx->stats.rx_excp_pkts
				[IPAHAL_PKT_STATUS_EXCEPTION_NONE]);
			continue;
		}

		if (status.endp_dest_idx == (sys->ep - ipa3_ctx->ep)) {
			/* RX data */
			src_pipe = status.endp_src_idx;

			/*
			 * A packet which is received back to the AP after
			 * there was no route match.
			 */
			if (status.exception ==
				IPAHAL_PKT_STATUS_EXCEPTION_NONE &&
				ipahal_is_rule_miss_id(status.rt_rule_id))
				sys->drop_packet = true;

			if (skb->len == pkt_status_sz &&
				status.exception ==
				IPAHAL_PKT_STATUS_EXCEPTION_NONE) {
				WARN_ON(sys->prev_skb != NULL);
				IPADBG_LOW("Ins header in next buffer\n");
				sys->prev_skb = skb_copy(skb, GFP_KERNEL);
				sys->len_partial = skb->len;
				return rc;
			}

			pad_len_byte = ((status.pkt_len + 3) & ~3) -
					status.pkt_len;

			len = status.pkt_len + pad_len_byte +
				IPA_SIZE_DL_CSUM_META_TRAILER;
			IPADBG_LOW("pad %d pkt_len %d len %d\n", pad_len_byte,
					status.pkt_len, len);

			if (status.exception ==
					IPAHAL_PKT_STATUS_EXCEPTION_DEAGGR) {
				IPADBG_LOW(
					"Dropping packet on DeAggr Exception\n");
				sys->drop_packet = true;
			}

			skb2 = ipa3_skb_copy_for_client(skb,
				min(status.pkt_len + pkt_status_sz, skb->len));
			if (likely(skb2)) {
				if (skb->len < len + pkt_status_sz) {
					IPADBG_LOW("SPL skb len %d len %d\n",
							skb->len, len);
					sys->prev_skb = skb2;
					sys->len_rem = len - skb->len +
						pkt_status_sz;
					sys->len_pad = pad_len_byte;
					skb_pull(skb, skb->len);
				} else {
					skb_trim(skb2, status.pkt_len +
							pkt_status_sz);
					IPADBG_LOW("rx avail for %d\n",
							status.endp_dest_idx);
					if (sys->drop_packet) {
						dev_kfree_skb_any(skb2);
					} else if (status.pkt_len >
						   IPA_GENERIC_AGGR_BYTE_LIMIT *
						   1024) {
						IPAERR("packet size invalid\n");
						IPAERR("STATUS opcode=%d\n",
							status.status_opcode);
						IPAERR("src=%d dst=%d len=%d\n",
							status.endp_src_idx,
							status.endp_dest_idx,
							status.pkt_len);
						BUG();
					} else {
					skb2->truesize = skb2->len +
						sizeof(struct sk_buff) +
						(ALIGN(len +
						pkt_status_sz, 32) *
						unused / used_align);
						sys->ep->client_notify(
							sys->ep->priv,
							IPA_RECEIVE,
							(unsigned long)(skb2));
					}
					skb_pull(skb, len + pkt_status_sz);
				}
			} else {
				IPAERR("fail to alloc skb\n");
				if (skb->len < len) {
					sys->prev_skb = NULL;
					sys->len_rem = len - skb->len +
						pkt_status_sz;
					sys->len_pad = pad_len_byte;
					skb_pull(skb, skb->len);
				} else {
					skb_pull(skb, len + pkt_status_sz);
				}
			}
			/* TX comp */
			ipa3_wq_write_done_status(src_pipe, tx_pkt);
			IPADBG_LOW("tx comp imp for %d\n", src_pipe);
		} else {
			/* TX comp */
			ipa3_wq_write_done_status(status.endp_src_idx, tx_pkt);
			IPADBG_LOW("tx comp exp for %d\n",
				status.endp_src_idx);
			skb_pull(skb, pkt_status_sz);
			IPA_STATS_INC_CNT(ipa3_ctx->stats.stat_compl);
			IPA_STATS_DEC_CNT(ipa3_ctx->stats.rx_excp_pkts
				[IPAHAL_PKT_STATUS_EXCEPTION_NONE]);
		}
		tx_pkt = NULL;
	};

	return rc;
}

static struct sk_buff *ipa3_join_prev_skb(struct sk_buff *prev_skb,
		struct sk_buff *skb, unsigned int len)
{
	struct sk_buff *skb2;

	skb2 = skb_copy_expand(prev_skb, 0,
			len, GFP_KERNEL);
	if (likely(skb2)) {
		memcpy(skb_put(skb2, len),
			skb->data, len);
	} else {
		IPAERR("copy expand failed\n");
		skb2 = NULL;
	}
	dev_kfree_skb_any(prev_skb);

	return skb2;
}

static void ipa3_wan_rx_handle_splt_pyld(struct sk_buff *skb,
		struct ipa3_sys_context *sys)
{
	struct sk_buff *skb2;

	IPADBG_LOW("rem %d skb %d\n", sys->len_rem, skb->len);
	if (sys->len_rem <= skb->len) {
		if (sys->prev_skb) {
			skb2 = ipa3_join_prev_skb(sys->prev_skb, skb,
					sys->len_rem);
			if (likely(skb2)) {
				IPADBG_LOW(
					"removing Status element from skb and sending to WAN client");
				skb_pull(skb2, ipahal_pkt_status_get_size());
				skb2->truesize = skb2->len +
					sizeof(struct sk_buff);
				sys->ep->client_notify(sys->ep->priv,
					IPA_RECEIVE,
					(unsigned long)(skb2));
			}
		}
		skb_pull(skb, sys->len_rem);
		sys->prev_skb = NULL;
		sys->len_rem = 0;
	} else {
		if (sys->prev_skb) {
			skb2 = ipa3_join_prev_skb(sys->prev_skb, skb,
					skb->len);
			sys->prev_skb = skb2;
		}
		sys->len_rem -= skb->len;
		skb_pull(skb, skb->len);
	}
}

static int ipa3_wan_rx_pyld_hdlr(struct sk_buff *skb,
		struct ipa3_sys_context *sys)
{
	int rc = 0;
	struct ipahal_pkt_status status;
	unsigned char *skb_data;
	u32 pkt_status_sz;
	struct sk_buff *skb2;
	u16 pkt_len_with_pad;
	u32 qmap_hdr;
	int checksum_trailer_exists;
	int frame_len;
	int ep_idx;
	unsigned int used = *(unsigned int *)skb->cb;
	unsigned int used_align = ALIGN(used, 32);
	unsigned long unused = IPA_GENERIC_RX_BUFF_BASE_SZ - used;

	IPA_DUMP_BUFF(skb->data, 0, skb->len);
	if (skb->len == 0) {
		IPAERR("ZLT\n");
		goto bail;
	}

	if (ipa3_ctx->ipa_client_apps_wan_cons_agg_gro) {
		sys->ep->client_notify(sys->ep->priv,
			IPA_RECEIVE, (unsigned long)(skb));
		return rc;
	}
	if (sys->repl_hdlr == ipa3_replenish_rx_cache_recycle) {
		IPAERR("Recycle should enable only with GRO Aggr\n");
		ipa_assert();
	}

	/*
	 * payload splits across 2 buff or more,
	 * take the start of the payload from prev_skb
	 */
	if (sys->len_rem)
		ipa3_wan_rx_handle_splt_pyld(skb, sys);

	pkt_status_sz = ipahal_pkt_status_get_size();
	while (skb->len) {
		IPADBG_LOW("LEN_REM %d\n", skb->len);
		if (skb->len < pkt_status_sz) {
			IPAERR("status straddles buffer\n");
			WARN_ON(1);
			goto bail;
		}
		ipahal_pkt_status_parse(skb->data, &status);
		skb_data = skb->data;
		IPADBG_LOW("STATUS opcode=%d src=%d dst=%d len=%d\n",
				status.status_opcode, status.endp_src_idx,
				status.endp_dest_idx, status.pkt_len);

		if (sys->status_stat) {
			sys->status_stat->status[sys->status_stat->curr] =
				status;
			sys->status_stat->curr++;
			if (sys->status_stat->curr == IPA_MAX_STATUS_STAT_NUM)
				sys->status_stat->curr = 0;
		}

		if ((status.status_opcode !=
			IPAHAL_PKT_STATUS_OPCODE_DROPPED_PACKET) &&
			(status.status_opcode !=
			IPAHAL_PKT_STATUS_OPCODE_PACKET) &&
			(status.status_opcode !=
			IPAHAL_PKT_STATUS_OPCODE_PACKET_2ND_PASS)) {
			IPAERR("unsupported opcode(%d)\n",
				status.status_opcode);
			skb_pull(skb, pkt_status_sz);
			continue;
		}

		IPA_STATS_INC_CNT(ipa3_ctx->stats.rx_pkts);
		if (status.endp_dest_idx >= ipa3_ctx->ipa_num_pipes ||
			status.endp_src_idx >= ipa3_ctx->ipa_num_pipes ||
			status.pkt_len > IPA_GENERIC_AGGR_BYTE_LIMIT * 1024) {
			IPAERR("status fields invalid\n");
			WARN_ON(1);
			goto bail;
		}
		if (status.pkt_len == 0) {
			IPADBG_LOW("Skip aggr close status\n");
			skb_pull(skb, pkt_status_sz);
			IPA_STATS_DEC_CNT(ipa3_ctx->stats.rx_pkts);
			IPA_STATS_INC_CNT(ipa3_ctx->stats.wan_aggr_close);
			continue;
		}
		ep_idx = ipa3_get_ep_mapping(IPA_CLIENT_APPS_WAN_CONS);
		if (status.endp_dest_idx != ep_idx) {
			IPAERR("expected endp_dest_idx %d received %d\n",
					ep_idx, status.endp_dest_idx);
			WARN_ON(1);
			goto bail;
		}
		/* RX data */
		if (skb->len == pkt_status_sz) {
			IPAERR("Ins header in next buffer\n");
			WARN_ON(1);
			goto bail;
		}
		qmap_hdr = *(u32 *)(skb_data + pkt_status_sz);
		/*
		 * Take the pkt_len_with_pad from the last 2 bytes of the QMAP
		 * header
		 */

		/*QMAP is BE: convert the pkt_len field from BE to LE*/
		pkt_len_with_pad = ntohs((qmap_hdr>>16) & 0xffff);
		IPADBG_LOW("pkt_len with pad %d\n", pkt_len_with_pad);
		/*get the CHECKSUM_PROCESS bit*/
		checksum_trailer_exists = IPAHAL_PKT_STATUS_MASK_FLAG_VAL(
			IPAHAL_PKT_STATUS_MASK_CKSUM_PROCESS_SHFT, &status);
		IPADBG_LOW("checksum_trailer_exists %d\n",
				checksum_trailer_exists);

		frame_len = pkt_status_sz + IPA_QMAP_HEADER_LENGTH +
			    pkt_len_with_pad;
		if (checksum_trailer_exists)
			frame_len += IPA_DL_CHECKSUM_LENGTH;
		IPADBG_LOW("frame_len %d\n", frame_len);

		skb2 = skb_clone(skb, GFP_KERNEL);
		if (likely(skb2)) {
			/*
			 * the len of actual data is smaller than expected
			 * payload split across 2 buff
			 */
			if (skb->len < frame_len) {
				IPADBG_LOW("SPL skb len %d len %d\n",
						skb->len, frame_len);
				sys->prev_skb = skb2;
				sys->len_rem = frame_len - skb->len;
				skb_pull(skb, skb->len);
			} else {
				skb_trim(skb2, frame_len);
				IPADBG_LOW("rx avail for %d\n",
						status.endp_dest_idx);
				IPADBG_LOW(
					"removing Status element from skb and sending to WAN client");
				skb_pull(skb2, pkt_status_sz);
				skb2->truesize = skb2->len +
					sizeof(struct sk_buff) +
					(ALIGN(frame_len, 32) *
					 unused / used_align);
				sys->ep->client_notify(sys->ep->priv,
					IPA_RECEIVE, (unsigned long)(skb2));
				skb_pull(skb, frame_len);
			}
		} else {
			IPAERR("fail to clone\n");
			if (skb->len < frame_len) {
				sys->prev_skb = NULL;
				sys->len_rem = frame_len - skb->len;
				skb_pull(skb, skb->len);
			} else {
				skb_pull(skb, frame_len);
			}
		}
	};
bail:
	sys->free_skb(skb);
	return rc;
}

static struct sk_buff *ipa3_get_skb_ipa_rx(unsigned int len, gfp_t flags)
{
	return __dev_alloc_skb(len, flags);
}

static void ipa3_free_skb_rx(struct sk_buff *skb)
{
	dev_kfree_skb_any(skb);
}

void ipa3_lan_rx_cb(void *priv, enum ipa_dp_evt_type evt, unsigned long data)
{
	struct sk_buff *rx_skb = (struct sk_buff *)data;
	struct ipahal_pkt_status status;
	struct ipa3_ep_context *ep;
	unsigned int src_pipe;
	u32 metadata;
	u8 ucp;

	ipahal_pkt_status_parse(rx_skb->data, &status);
	src_pipe = status.endp_src_idx;
	metadata = status.metadata;
	ucp = status.ucp;
	ep = &ipa3_ctx->ep[src_pipe];
	if (unlikely(src_pipe >= ipa3_ctx->ipa_num_pipes ||
		!ep->valid ||
		!ep->client_notify)) {
		IPAERR("drop pipe=%d ep_valid=%d client_notify=%p\n",
		  src_pipe, ep->valid, ep->client_notify);
		dev_kfree_skb_any(rx_skb);
		return;
	}
	if (status.exception == IPAHAL_PKT_STATUS_EXCEPTION_NONE)
		skb_pull(rx_skb, ipahal_pkt_status_get_size() +
				IPA_LAN_RX_HEADER_LENGTH);
	else
		skb_pull(rx_skb, ipahal_pkt_status_get_size());

	/* Metadata Info
	 *  ------------------------------------------
	 *  |   3     |   2     |    1        |  0   |
	 *  | fw_desc | vdev_id | qmap mux id | Resv |
	 *  ------------------------------------------
	 */
	*(u16 *)rx_skb->cb = ((metadata >> 16) & 0xFFFF);
	*(u8 *)(rx_skb->cb + 4) = ucp;
	IPADBG_LOW("meta_data: 0x%x cb: 0x%x\n",
			metadata, *(u32 *)rx_skb->cb);
	IPADBG_LOW("ucp: %d\n", *(u8 *)(rx_skb->cb + 4));

	ep->client_notify(ep->priv, IPA_RECEIVE, (unsigned long)(rx_skb));
}

static void ipa3_recycle_rx_wrapper(struct ipa3_rx_pkt_wrapper *rx_pkt)
{
	rx_pkt->data.dma_addr = 0;
	ipa3_skb_recycle(rx_pkt->data.skb);
	INIT_LIST_HEAD(&rx_pkt->link);
	spin_lock_bh(&rx_pkt->sys->spinlock);
	list_add_tail(&rx_pkt->link, &rx_pkt->sys->rcycl_list);
	spin_unlock_bh(&rx_pkt->sys->spinlock);
}

void ipa3_recycle_wan_skb(struct sk_buff *skb)
{
	struct ipa3_rx_pkt_wrapper *rx_pkt;
	int ep_idx = ipa3_get_ep_mapping(
	   IPA_CLIENT_APPS_WAN_CONS);
	gfp_t flag = GFP_NOWAIT | __GFP_NOWARN;

	if (unlikely(ep_idx == -1)) {
		IPAERR("dest EP does not exist\n");
		ipa_assert();
	}

	rx_pkt = kmem_cache_zalloc(ipa3_ctx->rx_pkt_wrapper_cache,
					flag);
	if (!rx_pkt)
		ipa_assert();

	INIT_WORK(&rx_pkt->work, ipa3_wq_rx_avail);
	rx_pkt->sys = ipa3_ctx->ep[ep_idx].sys;

	rx_pkt->data.skb = skb;
	ipa3_recycle_rx_wrapper(rx_pkt);
}

static void ipa3_wq_rx_common(struct ipa3_sys_context *sys, u32 size)
{
	struct ipa3_rx_pkt_wrapper *rx_pkt_expected;
	struct sk_buff *rx_skb;

	if (unlikely(list_empty(&sys->head_desc_list))) {
		WARN_ON(1);
		return;
	}
	spin_lock_bh(&sys->spinlock);
	rx_pkt_expected = list_first_entry(&sys->head_desc_list,
					   struct ipa3_rx_pkt_wrapper,
					   link);
	list_del(&rx_pkt_expected->link);
	sys->len--;
	if (size)
		rx_pkt_expected->len = size;
	spin_unlock_bh(&sys->spinlock);
	rx_skb = rx_pkt_expected->data.skb;
	dma_unmap_single(ipa3_ctx->pdev, rx_pkt_expected->data.dma_addr,
			sys->rx_buff_sz, DMA_FROM_DEVICE);
	skb_set_tail_pointer(rx_skb, rx_pkt_expected->len);
	rx_skb->len = rx_pkt_expected->len;
	*(unsigned int *)rx_skb->cb = rx_skb->len;
	rx_skb->truesize = rx_pkt_expected->len + sizeof(struct sk_buff);
	sys->pyld_hdlr(rx_skb, sys);
	sys->free_rx_wrapper(rx_pkt_expected);
	sys->repl_hdlr(sys);
}

static void ipa3_wlan_wq_rx_common(struct ipa3_sys_context *sys, u32 size)
{
	struct ipa3_rx_pkt_wrapper *rx_pkt_expected;
	struct sk_buff *rx_skb;

	if (unlikely(list_empty(&sys->head_desc_list))) {
		WARN_ON(1);
		return;
	}
	rx_pkt_expected = list_first_entry(&sys->head_desc_list,
					   struct ipa3_rx_pkt_wrapper,
					   link);
	list_del(&rx_pkt_expected->link);
	sys->len--;

	if (size)
		rx_pkt_expected->len = size;

	rx_skb = rx_pkt_expected->data.skb;
	skb_set_tail_pointer(rx_skb, rx_pkt_expected->len);
	rx_skb->len = rx_pkt_expected->len;
	rx_skb->truesize = rx_pkt_expected->len + sizeof(struct sk_buff);
	sys->ep->wstats.tx_pkts_rcvd++;
	if (sys->len <= IPA_WLAN_RX_POOL_SZ_LOW_WM) {
		ipa3_free_skb(&rx_pkt_expected->data);
		sys->ep->wstats.tx_pkts_dropped++;
	} else {
		sys->ep->wstats.tx_pkts_sent++;
		sys->ep->client_notify(sys->ep->priv, IPA_RECEIVE,
				(unsigned long)(&rx_pkt_expected->data));
	}
	ipa3_replenish_wlan_rx_cache(sys);
}

static void ipa3_dma_memcpy_notify(struct ipa3_sys_context *sys,
	struct ipa_mem_buffer *mem_info)
{
	IPADBG_LOW("ENTER.\n");
	if (unlikely(list_empty(&sys->head_desc_list))) {
		IPAERR("descriptor list is empty!\n");
		WARN_ON(1);
		return;
	}
	sys->ep->client_notify(sys->ep->priv, IPA_RECEIVE,
				(unsigned long)(mem_info));
	IPADBG_LOW("EXIT\n");
}

static void ipa3_wq_rx_avail(struct work_struct *work)
{
	struct ipa3_rx_pkt_wrapper *rx_pkt;
	struct ipa3_sys_context *sys;

	rx_pkt = container_of(work, struct ipa3_rx_pkt_wrapper, work);
	if (unlikely(rx_pkt == NULL))
		WARN_ON(1);
	sys = rx_pkt->sys;
	ipa3_wq_rx_common(sys, 0);
}

static int ipa3_odu_rx_pyld_hdlr(struct sk_buff *rx_skb,
	struct ipa3_sys_context *sys)
{
	if (sys->ep->client_notify) {
		sys->ep->client_notify(sys->ep->priv, IPA_RECEIVE,
			(unsigned long)(rx_skb));
	} else {
		dev_kfree_skb_any(rx_skb);
		WARN_ON(1);
	}

	return 0;
}

static void ipa3_free_rx_wrapper(struct ipa3_rx_pkt_wrapper *rk_pkt)
{
	kmem_cache_free(ipa3_ctx->rx_pkt_wrapper_cache, rk_pkt);
}

static int ipa3_assign_policy(struct ipa_sys_connect_params *in,
		struct ipa3_sys_context *sys)
{
	if (in->client == IPA_CLIENT_APPS_CMD_PROD) {
		sys->policy = IPA_POLICY_INTR_MODE;
		sys->use_comm_evt_ring = false;
		return 0;
	}

	if (in->client == IPA_CLIENT_APPS_WAN_PROD) {
		sys->policy = IPA_POLICY_INTR_MODE;
		sys->use_comm_evt_ring = true;
		INIT_WORK(&sys->work, ipa3_send_nop_desc);

		/*
		 * enable source notification status for exception packets
		 * (i.e. QMAP commands) to be routed to modem.
		 */
		sys->ep->status.status_en = true;
		sys->ep->status.status_ep =
			ipa3_get_ep_mapping(IPA_CLIENT_Q6_WAN_CONS);
		return 0;
	}

	if (IPA_CLIENT_IS_MEMCPY_DMA_PROD(in->client)) {
		sys->policy = IPA_POLICY_NOINTR_MODE;
		return 0;
	}

	if (IPA_CLIENT_IS_PROD(in->client)) {
		if (sys->ep->skip_ep_cfg) {
			sys->policy = IPA_POLICY_INTR_POLL_MODE;
			sys->use_comm_evt_ring = true;
			atomic_set(&sys->curr_polling_state, 0);
		} else {
			sys->policy = IPA_POLICY_INTR_MODE;
			sys->use_comm_evt_ring = true;
			INIT_WORK(&sys->work, ipa3_send_nop_desc);
		}
	} else {
		if (in->client == IPA_CLIENT_APPS_LAN_CONS ||
		    in->client == IPA_CLIENT_APPS_WAN_CONS) {
			sys->ep->status.status_en = true;
			sys->policy = IPA_POLICY_INTR_POLL_MODE;
			INIT_WORK(&sys->work, ipa3_wq_handle_rx);
			INIT_DELAYED_WORK(&sys->switch_to_intr_work,
				ipa3_switch_to_intr_rx_work_func);
			INIT_DELAYED_WORK(&sys->replenish_rx_work,
					ipa3_replenish_rx_work_func);
			INIT_WORK(&sys->repl_work, ipa3_wq_repl_rx);
			atomic_set(&sys->curr_polling_state, 0);
			sys->rx_buff_sz = IPA_GENERIC_RX_BUFF_SZ(
				IPA_GENERIC_RX_BUFF_BASE_SZ);
			sys->get_skb = ipa3_get_skb_ipa_rx;
			sys->free_skb = ipa3_free_skb_rx;
			in->ipa_ep_cfg.aggr.aggr_en = IPA_ENABLE_AGGR;
			in->ipa_ep_cfg.aggr.aggr = IPA_GENERIC;
			in->ipa_ep_cfg.aggr.aggr_time_limit =
				IPA_GENERIC_AGGR_TIME_LIMIT;
			if (in->client == IPA_CLIENT_APPS_LAN_CONS) {
				sys->pyld_hdlr = ipa3_lan_rx_pyld_hdlr;
				sys->repl_hdlr =
					ipa3_replenish_rx_cache_recycle;
				sys->free_rx_wrapper =
					ipa3_recycle_rx_wrapper;
				sys->rx_pool_sz =
					ipa3_ctx->lan_rx_ring_size;
				in->ipa_ep_cfg.aggr.aggr_byte_limit =
				IPA_GENERIC_AGGR_BYTE_LIMIT;
				in->ipa_ep_cfg.aggr.aggr_pkt_limit =
				IPA_GENERIC_AGGR_PKT_LIMIT;
			} else if (in->client ==
					IPA_CLIENT_APPS_WAN_CONS) {
				sys->pyld_hdlr = ipa3_wan_rx_pyld_hdlr;
				sys->free_rx_wrapper = ipa3_free_rx_wrapper;
				sys->rx_pool_sz = ipa3_ctx->wan_rx_ring_size;
				if (nr_cpu_ids > 1) {
					sys->repl_hdlr =
					   ipa3_fast_replenish_rx_cache;
				} else {
					sys->repl_hdlr =
					   ipa3_replenish_rx_cache;
				}
				if (in->napi_enabled && in->recycle_enabled)
					sys->repl_hdlr =
					 ipa3_replenish_rx_cache_recycle;
				in->ipa_ep_cfg.aggr.aggr_sw_eof_active
					= true;
				if (ipa3_ctx->
				ipa_client_apps_wan_cons_agg_gro) {
					IPAERR("get close-by %u\n",
					ipa_adjust_ra_buff_base_sz(
					in->ipa_ep_cfg.aggr.
					aggr_byte_limit));
					IPAERR("set rx_buff_sz %lu\n",
					(unsigned long int)
					IPA_GENERIC_RX_BUFF_SZ(
					ipa_adjust_ra_buff_base_sz(
					in->ipa_ep_cfg.
						aggr.aggr_byte_limit)));
					/* disable ipa_status */
					sys->ep->status.
						status_en = false;
					sys->rx_buff_sz =
					IPA_GENERIC_RX_BUFF_SZ(
					ipa_adjust_ra_buff_base_sz(
					in->ipa_ep_cfg.aggr.
						aggr_byte_limit));
					in->ipa_ep_cfg.aggr.
						aggr_byte_limit =
					sys->rx_buff_sz < in->
					ipa_ep_cfg.aggr.
					aggr_byte_limit ?
					IPA_ADJUST_AGGR_BYTE_LIMIT(
					sys->rx_buff_sz) :
					IPA_ADJUST_AGGR_BYTE_LIMIT(
					in->ipa_ep_cfg.
					aggr.aggr_byte_limit);
					IPAERR("set aggr_limit %lu\n",
					(unsigned long int)
					in->ipa_ep_cfg.aggr.
					aggr_byte_limit);
				} else {
					in->ipa_ep_cfg.aggr.
						aggr_byte_limit =
					IPA_GENERIC_AGGR_BYTE_LIMIT;
					in->ipa_ep_cfg.aggr.
						aggr_pkt_limit =
					IPA_GENERIC_AGGR_PKT_LIMIT;
				}
			}
		} else if (IPA_CLIENT_IS_WLAN_CONS(in->client)) {
			IPADBG("assigning policy to client:%d",
				in->client);

			sys->policy = IPA_POLICY_INTR_POLL_MODE;
			INIT_WORK(&sys->work, ipa3_wq_handle_rx);
			INIT_DELAYED_WORK(&sys->switch_to_intr_work,
				ipa3_switch_to_intr_rx_work_func);
			INIT_DELAYED_WORK(&sys->replenish_rx_work,
				ipa3_replenish_rx_work_func);
			atomic_set(&sys->curr_polling_state, 0);
			sys->rx_buff_sz = IPA_WLAN_RX_BUFF_SZ;
			sys->rx_pool_sz = in->desc_fifo_sz /
				IPA_FIFO_ELEMENT_SIZE - 1;
			if (sys->rx_pool_sz > IPA_WLAN_RX_POOL_SZ)
				sys->rx_pool_sz = IPA_WLAN_RX_POOL_SZ;
			sys->pyld_hdlr = NULL;
			sys->repl_hdlr = ipa3_replenish_wlan_rx_cache;
			sys->get_skb = ipa3_get_skb_ipa_rx;
			sys->free_skb = ipa3_free_skb_rx;
			sys->free_rx_wrapper = ipa3_free_rx_wrapper;
			in->ipa_ep_cfg.aggr.aggr_en = IPA_BYPASS_AGGR;
		} else if (IPA_CLIENT_IS_ODU_CONS(in->client)) {
			IPADBG("assigning policy to client:%d",
				in->client);

			sys->policy = IPA_POLICY_INTR_POLL_MODE;
			INIT_WORK(&sys->work, ipa3_wq_handle_rx);
			INIT_DELAYED_WORK(&sys->switch_to_intr_work,
				ipa3_switch_to_intr_rx_work_func);
			INIT_DELAYED_WORK(&sys->replenish_rx_work,
				ipa3_replenish_rx_work_func);
			atomic_set(&sys->curr_polling_state, 0);
			sys->rx_pool_sz = in->desc_fifo_sz /
				IPA_FIFO_ELEMENT_SIZE - 1;
			if (sys->rx_pool_sz > IPA_ODU_RX_POOL_SZ)
				sys->rx_pool_sz = IPA_ODU_RX_POOL_SZ;
			sys->pyld_hdlr = ipa3_odu_rx_pyld_hdlr;
			sys->get_skb = ipa3_get_skb_ipa_rx;
			sys->free_skb = ipa3_free_skb_rx;
			/* recycle skb for GSB use case */
			if (ipa3_ctx->ipa_hw_type >= IPA_HW_v4_0) {
				sys->free_rx_wrapper =
					ipa3_free_rx_wrapper;
				sys->repl_hdlr =
					ipa3_replenish_rx_cache;
				/* Overwrite buffer size & aggr limit for GSB */
				sys->rx_buff_sz = IPA_GENERIC_RX_BUFF_SZ(
					IPA_GSB_RX_BUFF_BASE_SZ);
				in->ipa_ep_cfg.aggr.aggr_byte_limit =
					IPA_GSB_AGGR_BYTE_LIMIT;
			} else {
				sys->free_rx_wrapper =
					ipa3_free_rx_wrapper;
				sys->repl_hdlr = ipa3_replenish_rx_cache;
				sys->rx_buff_sz = IPA_ODU_RX_BUFF_SZ;
			}
		} else if (in->client ==
				IPA_CLIENT_MEMCPY_DMA_ASYNC_CONS) {
			IPADBG("assigning policy to client:%d",
				in->client);

			sys->policy = IPA_POLICY_INTR_POLL_MODE;
			INIT_WORK(&sys->work, ipa3_wq_handle_rx);
			INIT_DELAYED_WORK(&sys->switch_to_intr_work,
				ipa3_switch_to_intr_rx_work_func);
		} else if (in->client ==
				IPA_CLIENT_MEMCPY_DMA_SYNC_CONS) {
			IPADBG("assigning policy to client:%d",
				in->client);

			sys->policy = IPA_POLICY_NOINTR_MODE;
		} else {
			IPAERR("Need to install a RX pipe hdlr\n");
			WARN_ON(1);
			return -EINVAL;
		}
	}

	return 0;
}

/**
 * ipa3_tx_client_rx_notify_release() - Callback function
 * which will call the user supplied callback function to
 * release the skb, or release it on its own if no callback
 * function was supplied
 *
 * @user1: [in] - Data Descriptor
 * @user2: [in] - endpoint idx
 *
 * This notified callback is for the destination client
 * This function is supplied in ipa3_tx_dp_mul
 */
static void ipa3_tx_client_rx_notify_release(void *user1, int user2)
{
	struct ipa_tx_data_desc *dd = (struct ipa_tx_data_desc *)user1;
	int ep_idx = user2;

	IPADBG_LOW("Received data desc anchor:%p\n", dd);

	atomic_inc(&ipa3_ctx->ep[ep_idx].avail_fifo_desc);
	ipa3_ctx->ep[ep_idx].wstats.rx_pkts_status_rcvd++;

  /* wlan host driver waits till tx complete before unload */
	IPADBG_LOW("ep=%d fifo_desc_free_count=%d\n",
		ep_idx, atomic_read(&ipa3_ctx->ep[ep_idx].avail_fifo_desc));
	IPADBG_LOW("calling client notify callback with priv:%p\n",
		ipa3_ctx->ep[ep_idx].priv);

	if (ipa3_ctx->ep[ep_idx].client_notify) {
		ipa3_ctx->ep[ep_idx].client_notify(ipa3_ctx->ep[ep_idx].priv,
				IPA_WRITE_DONE, (unsigned long)user1);
		ipa3_ctx->ep[ep_idx].wstats.rx_hd_reply++;
	}
}
/**
 * ipa3_tx_client_rx_pkt_status() - Callback function
 * which will call the user supplied callback function to
 * increase the available fifo descriptor
 *
 * @user1: [in] - Data Descriptor
 * @user2: [in] - endpoint idx
 *
 * This notified callback is for the destination client
 * This function is supplied in ipa3_tx_dp_mul
 */
static void ipa3_tx_client_rx_pkt_status(void *user1, int user2)
{
	int ep_idx = user2;

	atomic_inc(&ipa3_ctx->ep[ep_idx].avail_fifo_desc);
	ipa3_ctx->ep[ep_idx].wstats.rx_pkts_status_rcvd++;
}


/**
 * ipa3_tx_dp_mul() - Data-path tx handler for multiple packets
 * @src: [in] - Client that is sending data
 * @ipa_tx_data_desc:	[in] data descriptors from wlan
 *
 * this is used for to transfer data descriptors that received
 * from WLAN1_PROD pipe to IPA HW
 *
 * The function will send data descriptors from WLAN1_PROD (one
 * at a time). Will set EOT flag for last descriptor Once this send was done
 * from transport point-of-view the IPA driver will get notified by the
 * supplied callback - ipa_gsi_irq_tx_notify_cb()
 *
 * ipa_gsi_irq_tx_notify_cb will call to the user supplied callback
 *
 * Returns:	0 on success, negative on failure
 */
int ipa3_tx_dp_mul(enum ipa_client_type src,
			struct ipa_tx_data_desc *data_desc)
{
	/* The second byte in wlan header holds qmap id */
#define IPA_WLAN_HDR_QMAP_ID_OFFSET 1
	struct ipa_tx_data_desc *entry;
	struct ipa3_sys_context *sys;
	struct ipa3_desc desc[2];
	u32 num_desc, cnt;
	int ep_idx;

	IPADBG_LOW("Received data desc anchor:%p\n", data_desc);

	spin_lock_bh(&ipa3_ctx->wc_memb.ipa_tx_mul_spinlock);

	ep_idx = ipa3_get_ep_mapping(src);
	if (unlikely(ep_idx == -1)) {
		IPAERR("dest EP does not exist.\n");
		goto fail_send;
	}
	IPADBG_LOW("ep idx:%d\n", ep_idx);
	sys = ipa3_ctx->ep[ep_idx].sys;

	if (unlikely(ipa3_ctx->ep[ep_idx].valid == 0)) {
		IPAERR("dest EP not valid.\n");
		goto fail_send;
	}
	sys->ep->wstats.rx_hd_rcvd++;

	/* Calculate the number of descriptors */
	num_desc = 0;
	list_for_each_entry(entry, &data_desc->link, link) {
		num_desc++;
	}
	IPADBG_LOW("Number of Data Descriptors:%d", num_desc);

	if (atomic_read(&sys->ep->avail_fifo_desc) < num_desc) {
		IPAERR("Insufficient data descriptors available\n");
		goto fail_send;
	}

	/* Assign callback only for last data descriptor */
	cnt = 0;
	list_for_each_entry(entry, &data_desc->link, link) {
		memset(desc, 0, 2 * sizeof(struct ipa3_desc));

		IPADBG_LOW("Parsing data desc :%d\n", cnt);
		cnt++;
		((u8 *)entry->pyld_buffer)[IPA_WLAN_HDR_QMAP_ID_OFFSET] =
			(u8)sys->ep->cfg.meta.qmap_id;

		/* the tag field will be populated in ipa3_send() function */
		desc[0].is_tag_status = true;
		desc[1].pyld = entry->pyld_buffer;
		desc[1].len = entry->pyld_len;
		desc[1].type = IPA_DATA_DESC_SKB;
		desc[1].user1 = data_desc;
		desc[1].user2 = ep_idx;
		IPADBG_LOW("priv:%p pyld_buf:0x%p pyld_len:%d\n",
			entry->priv, desc[1].pyld, desc[1].len);

		/* In case of last descriptor populate callback */
		if (cnt == num_desc) {
			IPADBG_LOW("data desc:%p\n", data_desc);
			desc[1].callback = ipa3_tx_client_rx_notify_release;
		} else {
			desc[1].callback = ipa3_tx_client_rx_pkt_status;
		}

		IPADBG_LOW("calling ipa3_send_one()\n");
		if (ipa3_send(sys, 2, desc, true)) {
			IPAERR("fail to send skb\n");
			sys->ep->wstats.rx_pkt_leak += (cnt-1);
			sys->ep->wstats.rx_dp_fail++;
			goto fail_send;
		}

		if (atomic_read(&sys->ep->avail_fifo_desc) >= 0)
			atomic_dec(&sys->ep->avail_fifo_desc);

		sys->ep->wstats.rx_pkts_rcvd++;
		IPADBG_LOW("ep=%d fifo desc=%d\n",
			ep_idx, atomic_read(&sys->ep->avail_fifo_desc));
	}

	sys->ep->wstats.rx_hd_processed++;
	spin_unlock_bh(&ipa3_ctx->wc_memb.ipa_tx_mul_spinlock);
	return 0;

fail_send:
	spin_unlock_bh(&ipa3_ctx->wc_memb.ipa_tx_mul_spinlock);
	return -EFAULT;

}

void ipa3_free_skb(struct ipa_rx_data *data)
{
	struct ipa3_rx_pkt_wrapper *rx_pkt;

	spin_lock_bh(&ipa3_ctx->wc_memb.wlan_spinlock);

	ipa3_ctx->wc_memb.total_tx_pkts_freed++;
	rx_pkt = container_of(data, struct ipa3_rx_pkt_wrapper, data);

	ipa3_skb_recycle(rx_pkt->data.skb);
	(void)skb_put(rx_pkt->data.skb, IPA_WLAN_RX_BUFF_SZ);

	list_add_tail(&rx_pkt->link,
		&ipa3_ctx->wc_memb.wlan_comm_desc_list);
	ipa3_ctx->wc_memb.wlan_comm_free_cnt++;

	spin_unlock_bh(&ipa3_ctx->wc_memb.wlan_spinlock);
}

/* Functions added to support kernel tests */

int ipa3_sys_setup(struct ipa_sys_connect_params *sys_in,
			unsigned long *ipa_transport_hdl,
			u32 *ipa_pipe_num, u32 *clnt_hdl, bool en_status)
{
	struct ipa3_ep_context *ep;
	int ipa_ep_idx;
	int result = -EINVAL;

	if (sys_in == NULL || clnt_hdl == NULL) {
		IPAERR("NULL args\n");
		goto fail_gen;
	}

	if (ipa_transport_hdl == NULL || ipa_pipe_num == NULL) {
		IPAERR("NULL args\n");
		goto fail_gen;
	}
	if (sys_in->client >= IPA_CLIENT_MAX) {
		IPAERR("bad parm client:%d\n", sys_in->client);
		goto fail_gen;
	}

	ipa_ep_idx = ipa3_get_ep_mapping(sys_in->client);
	if (ipa_ep_idx == -1) {
		IPAERR("Invalid client :%d\n", sys_in->client);
		goto fail_gen;
	}

	ep = &ipa3_ctx->ep[ipa_ep_idx];
	IPA_ACTIVE_CLIENTS_INC_EP(sys_in->client);

	if (ep->valid == 1) {
		if (sys_in->client != IPA_CLIENT_APPS_WAN_PROD) {
			IPAERR("EP %d already allocated\n", ipa_ep_idx);
			goto fail_and_disable_clocks;
		} else {
			if (ipa3_cfg_ep_hdr(ipa_ep_idx,
						&sys_in->ipa_ep_cfg.hdr)) {
				IPAERR("fail to configure hdr prop of EP %d\n",
						ipa_ep_idx);
				result = -EFAULT;
				goto fail_and_disable_clocks;
			}
			if (ipa3_cfg_ep_cfg(ipa_ep_idx,
						&sys_in->ipa_ep_cfg.cfg)) {
				IPAERR("fail to configure cfg prop of EP %d\n",
						ipa_ep_idx);
				result = -EFAULT;
				goto fail_and_disable_clocks;
			}
			IPAERR("client %d (ep: %d) overlay ok sys=%p\n",
					sys_in->client, ipa_ep_idx, ep->sys);
			ep->client_notify = sys_in->notify;
			ep->priv = sys_in->priv;
			*clnt_hdl = ipa_ep_idx;
			if (!ep->keep_ipa_awake)
				IPA_ACTIVE_CLIENTS_DEC_EP(sys_in->client);

			return 0;
		}
	}

	memset(ep, 0, offsetof(struct ipa3_ep_context, sys));

	ep->valid = 1;
	ep->client = sys_in->client;
	ep->client_notify = sys_in->notify;
	ep->priv = sys_in->priv;
	ep->keep_ipa_awake = true;
	if (en_status) {
		ep->status.status_en = true;
		ep->status.status_ep = ipa_ep_idx;
	}

	result = ipa3_enable_data_path(ipa_ep_idx);
	if (result) {
		IPAERR("enable data path failed res=%d clnt=%d.\n",
				 result, ipa_ep_idx);
		goto fail_gen2;
	}

	if (!ep->skip_ep_cfg) {
		if (ipa3_cfg_ep(ipa_ep_idx, &sys_in->ipa_ep_cfg)) {
			IPAERR("fail to configure EP.\n");
			goto fail_gen2;
		}
		if (ipa3_cfg_ep_status(ipa_ep_idx, &ep->status)) {
			IPAERR("fail to configure status of EP.\n");
			goto fail_gen2;
		}
		IPADBG("ep configuration successful\n");
	} else {
		IPADBG("skipping ep configuration\n");
	}

	*clnt_hdl = ipa_ep_idx;

	*ipa_pipe_num = ipa_ep_idx;
	*ipa_transport_hdl = ipa3_ctx->gsi_dev_hdl;

	if (!ep->keep_ipa_awake)
		IPA_ACTIVE_CLIENTS_DEC_EP(sys_in->client);

	ipa3_ctx->skip_ep_cfg_shadow[ipa_ep_idx] = ep->skip_ep_cfg;
	IPADBG("client %d (ep: %d) connected sys=%p\n", sys_in->client,
			ipa_ep_idx, ep->sys);

	return 0;

fail_gen2:
fail_and_disable_clocks:
	IPA_ACTIVE_CLIENTS_DEC_EP(sys_in->client);
fail_gen:
	return result;
}

int ipa3_sys_teardown(u32 clnt_hdl)
{
	struct ipa3_ep_context *ep;

	if (clnt_hdl >= ipa3_ctx->ipa_num_pipes ||
	    ipa3_ctx->ep[clnt_hdl].valid == 0) {
		IPAERR("bad parm(Either endpoint or client hdl invalid)\n");
		return -EINVAL;
	}

	ep = &ipa3_ctx->ep[clnt_hdl];

	if (!ep->keep_ipa_awake)
		IPA_ACTIVE_CLIENTS_INC_EP(ipa3_get_client_mapping(clnt_hdl));

	ipa3_disable_data_path(clnt_hdl);
	ep->valid = 0;

	IPA_ACTIVE_CLIENTS_DEC_EP(ipa3_get_client_mapping(clnt_hdl));

	IPADBG("client (ep: %d) disconnected\n", clnt_hdl);

	return 0;
}

int ipa3_sys_update_gsi_hdls(u32 clnt_hdl, unsigned long gsi_ch_hdl,
	unsigned long gsi_ev_hdl)
{
	struct ipa3_ep_context *ep;

	if (clnt_hdl >= ipa3_ctx->ipa_num_pipes ||
		ipa3_ctx->ep[clnt_hdl].valid == 0) {
		IPAERR("bad parm(Either endpoint or client hdl invalid)\n");
		return -EINVAL;
	}

	ep = &ipa3_ctx->ep[clnt_hdl];

	ep->gsi_chan_hdl = gsi_ch_hdl;
	ep->gsi_evt_ring_hdl = gsi_ev_hdl;

	return 0;
}

static void ipa_gsi_evt_ring_err_cb(struct gsi_evt_err_notify *notify)
{
	switch (notify->evt_id) {
	case GSI_EVT_OUT_OF_BUFFERS_ERR:
		IPAERR("Got GSI_EVT_OUT_OF_BUFFERS_ERR\n");
		break;
	case GSI_EVT_OUT_OF_RESOURCES_ERR:
		IPAERR("Got GSI_EVT_OUT_OF_RESOURCES_ERR\n");
		break;
	case GSI_EVT_UNSUPPORTED_INTER_EE_OP_ERR:
		IPAERR("Got GSI_EVT_UNSUPPORTED_INTER_EE_OP_ERR\n");
		break;
	case GSI_EVT_EVT_RING_EMPTY_ERR:
		IPAERR("Got GSI_EVT_EVT_RING_EMPTY_ERR\n");
		break;
	default:
		IPAERR("Unexpected err evt: %d\n", notify->evt_id);
	}
}

static void ipa_gsi_chan_err_cb(struct gsi_chan_err_notify *notify)
{
	switch (notify->evt_id) {
	case GSI_CHAN_INVALID_TRE_ERR:
		IPAERR("Got GSI_CHAN_INVALID_TRE_ERR\n");
		break;
	case GSI_CHAN_NON_ALLOCATED_EVT_ACCESS_ERR:
		IPAERR("Got GSI_CHAN_NON_ALLOCATED_EVT_ACCESS_ERR\n");
		break;
	case GSI_CHAN_OUT_OF_BUFFERS_ERR:
		IPAERR("Got GSI_CHAN_OUT_OF_BUFFERS_ERR\n");
		break;
	case GSI_CHAN_OUT_OF_RESOURCES_ERR:
		IPAERR("Got GSI_CHAN_OUT_OF_RESOURCES_ERR\n");
		break;
	case GSI_CHAN_UNSUPPORTED_INTER_EE_OP_ERR:
		IPAERR("Got GSI_CHAN_UNSUPPORTED_INTER_EE_OP_ERR\n");
		break;
	case GSI_CHAN_HWO_1_ERR:
		IPAERR("Got GSI_CHAN_HWO_1_ERR\n");
		break;
	default:
		IPAERR("Unexpected err evt: %d\n", notify->evt_id);
	}
}

static void ipa_gsi_irq_tx_notify_cb(struct gsi_chan_xfer_notify *notify)
{
	struct ipa3_tx_pkt_wrapper *tx_pkt;

	IPADBG_LOW("event %d notified\n", notify->evt_id);

	switch (notify->evt_id) {
	case GSI_CHAN_EVT_EOT:
		atomic_set(&ipa3_ctx->transport_pm.eot_activity, 1);
		tx_pkt = notify->xfer_user_data;
		queue_work(tx_pkt->sys->wq, &tx_pkt->work);
		break;
	default:
		IPAERR("received unexpected event id %d\n", notify->evt_id);
	}
}

void __ipa_gsi_irq_rx_scedule_poll(struct ipa3_sys_context *sys)
{
	bool clk_off;

	atomic_set(&sys->curr_polling_state, 1);
	ipa3_inc_acquire_wakelock();

	/*
	 * pm deactivate is done in wq context
	 * or after NAPI poll
	 */
	if (ipa3_ctx->use_ipa_pm) {
		clk_off = ipa_pm_activate(sys->pm_hdl);
		if (!clk_off && sys->ep->napi_enabled) {
			sys->ep->client_notify(sys->ep->priv,
				IPA_CLIENT_START_POLL, 0);
			return;
		}
		queue_work(sys->wq, &sys->work);
		return;
	}

	if (sys->ep->napi_enabled) {
		struct ipa_active_client_logging_info log;

		IPA_ACTIVE_CLIENTS_PREP_SPECIAL(log, "NAPI");
		clk_off = ipa3_inc_client_enable_clks_no_block(
			&log);
		if (!clk_off) {
			sys->ep->client_notify(sys->ep->priv,
				IPA_CLIENT_START_POLL, 0);
			return;
		}
	}

	queue_work(sys->wq, &sys->work);
}

static void ipa_gsi_irq_rx_notify_cb(struct gsi_chan_xfer_notify *notify)
{
	struct ipa3_sys_context *sys;
	struct ipa3_rx_pkt_wrapper *rx_pkt_expected, *rx_pkt_rcvd;

	if (!notify) {
		IPAERR("gsi notify is NULL.\n");
		return;
	}
	IPADBG_LOW("event %d notified\n", notify->evt_id);

	sys = (struct ipa3_sys_context *)notify->chan_user_data;
	rx_pkt_expected = list_first_entry(&sys->head_desc_list,
					   struct ipa3_rx_pkt_wrapper, link);
	rx_pkt_rcvd = (struct ipa3_rx_pkt_wrapper *)notify->xfer_user_data;

	if (rx_pkt_expected != rx_pkt_rcvd) {
		IPAERR("Pkt was not filled in head of rx buffer.\n");
		WARN_ON(1);
		return;
	}
	sys->ep->bytes_xfered_valid = true;
	sys->ep->bytes_xfered = notify->bytes_xfered;
	sys->ep->phys_base = rx_pkt_rcvd->data.dma_addr;

	switch (notify->evt_id) {
	case GSI_CHAN_EVT_EOT:
	case GSI_CHAN_EVT_EOB:
		atomic_set(&ipa3_ctx->transport_pm.eot_activity, 1);
		if (!atomic_read(&sys->curr_polling_state)) {
			/* put the gsi channel into polling mode */
			gsi_config_channel_mode(sys->ep->gsi_chan_hdl,
				GSI_CHAN_MODE_POLL);
			__ipa_gsi_irq_rx_scedule_poll(sys);
		}
		break;
	default:
		IPAERR("received unexpected event id %d\n", notify->evt_id);
	}
}

static void ipa_dma_gsi_irq_rx_notify_cb(struct gsi_chan_xfer_notify *notify)
{
	struct ipa3_sys_context *sys;
	struct ipa3_dma_xfer_wrapper *rx_pkt_expected, *rx_pkt_rcvd;

	if (!notify) {
		IPAERR("gsi notify is NULL.\n");
		return;
	}
	IPADBG_LOW("event %d notified\n", notify->evt_id);

	sys = (struct ipa3_sys_context *)notify->chan_user_data;
	if (sys->ep->client == IPA_CLIENT_MEMCPY_DMA_SYNC_CONS) {
		IPAERR("IRQ_RX Callback was called for DMA_SYNC_CONS.\n");
		return;
	}
	rx_pkt_expected = list_first_entry(&sys->head_desc_list,
	struct ipa3_dma_xfer_wrapper, link);
		rx_pkt_rcvd = (struct ipa3_dma_xfer_wrapper *)notify
			->xfer_user_data;
	if (rx_pkt_expected != rx_pkt_rcvd) {
		IPAERR("Pkt was not filled in head of rx buffer.\n");
		WARN_ON(1);
		return;
	}

	sys->ep->bytes_xfered_valid = true;
	sys->ep->bytes_xfered = notify->bytes_xfered;
	sys->ep->phys_base = rx_pkt_rcvd->phys_addr_dest;

	switch (notify->evt_id) {
	case GSI_CHAN_EVT_EOT:
		if (!atomic_read(&sys->curr_polling_state)) {
			/* put the gsi channel into polling mode */
			gsi_config_channel_mode(sys->ep->gsi_chan_hdl,
				GSI_CHAN_MODE_POLL);
			ipa3_inc_acquire_wakelock();
			atomic_set(&sys->curr_polling_state, 1);
			queue_work(sys->wq, &sys->work);
		}
		break;
	default:
		IPAERR("received unexpected event id %d\n", notify->evt_id);
	}
}

int ipa3_alloc_common_event_ring(void)
{
	struct gsi_evt_ring_props gsi_evt_ring_props;
	dma_addr_t evt_dma_addr;
	int result;

	memset(&gsi_evt_ring_props, 0, sizeof(gsi_evt_ring_props));
	gsi_evt_ring_props.intf = GSI_EVT_CHTYPE_GPI_EV;
	gsi_evt_ring_props.intr = GSI_INTR_IRQ;
	gsi_evt_ring_props.re_size = GSI_EVT_RING_RE_SIZE_16B;

	gsi_evt_ring_props.ring_len = IPA_COMMON_EVENT_RING_SIZE;

	gsi_evt_ring_props.ring_base_vaddr =
		dma_alloc_coherent(ipa3_ctx->pdev,
		gsi_evt_ring_props.ring_len, &evt_dma_addr, GFP_KERNEL);
	if (!gsi_evt_ring_props.ring_base_vaddr) {
		IPAERR("fail to dma alloc %u bytes\n",
			gsi_evt_ring_props.ring_len);
		return -ENOMEM;
	}
	gsi_evt_ring_props.ring_base_addr = evt_dma_addr;
	gsi_evt_ring_props.int_modt = 0;
	gsi_evt_ring_props.int_modc = 1; /* moderation comes from channel*/
	gsi_evt_ring_props.rp_update_addr = 0;
	gsi_evt_ring_props.exclusive = false;
	gsi_evt_ring_props.err_cb = ipa_gsi_evt_ring_err_cb;
	gsi_evt_ring_props.user_data = NULL;

	result = gsi_alloc_evt_ring(&gsi_evt_ring_props,
		ipa3_ctx->gsi_dev_hdl, &ipa3_ctx->gsi_evt_comm_hdl);
	if (result) {
		IPAERR("gsi_alloc_evt_ring failed %d\n", result);
		return result;
	}
	ipa3_ctx->gsi_evt_comm_ring_rem = IPA_COMMON_EVENT_RING_SIZE;

	return 0;
}

static int ipa_gsi_setup_channel(struct ipa_sys_connect_params *in,
	struct ipa3_ep_context *ep)
{
	struct gsi_evt_ring_props gsi_evt_ring_props;
	struct gsi_chan_props gsi_channel_props;
	union __packed gsi_channel_scratch ch_scratch;
	const struct ipa_gsi_ep_config *gsi_ep_info;
	dma_addr_t dma_addr;
	dma_addr_t evt_dma_addr;
	int result;
	gfp_t mem_flag = GFP_KERNEL;

	if (in->client == IPA_CLIENT_APPS_WAN_CONS ||
		in->client == IPA_CLIENT_APPS_WAN_PROD)
		mem_flag = GFP_ATOMIC;

	if (!ep) {
		IPAERR("EP context is empty\n");
		return -EINVAL;
	}

	evt_dma_addr = 0;
	ep->gsi_evt_ring_hdl = ~0;
	memset(&gsi_evt_ring_props, 0, sizeof(gsi_evt_ring_props));
	if (ep->sys->use_comm_evt_ring) {
		if (ipa3_ctx->gsi_evt_comm_ring_rem < 2 * in->desc_fifo_sz) {
			IPAERR("not enough space in common event ring\n");
			IPAERR("available: %d needed: %d\n",
				ipa3_ctx->gsi_evt_comm_ring_rem,
				2 * in->desc_fifo_sz);
			WARN_ON(1);
			return -EFAULT;
		}
		ipa3_ctx->gsi_evt_comm_ring_rem -= (2 * in->desc_fifo_sz);
		ep->gsi_evt_ring_hdl = ipa3_ctx->gsi_evt_comm_hdl;
	} else if (ep->sys->policy != IPA_POLICY_NOINTR_MODE ||
	     IPA_CLIENT_IS_CONS(ep->client)) {
		gsi_evt_ring_props.intf = GSI_EVT_CHTYPE_GPI_EV;
		gsi_evt_ring_props.intr = GSI_INTR_IRQ;
		gsi_evt_ring_props.re_size =
			GSI_EVT_RING_RE_SIZE_16B;

		/*
		 * GSI ring length is calculated based on the desc_fifo_sz
		 * which was meant to define the BAM desc fifo. GSI descriptors
		 * are 16B as opposed to 8B for BAM.
		 */
		gsi_evt_ring_props.ring_len = 2 * in->desc_fifo_sz;

		gsi_evt_ring_props.ring_base_vaddr =
			dma_alloc_coherent(ipa3_ctx->pdev,
			gsi_evt_ring_props.ring_len,
			&evt_dma_addr, mem_flag);
		if (!gsi_evt_ring_props.ring_base_vaddr) {
			IPAERR("fail to dma alloc %u bytes\n",
				gsi_evt_ring_props.ring_len);
			return -ENOMEM;
		}
		gsi_evt_ring_props.ring_base_addr = evt_dma_addr;

		/* copy mem info */
		ep->gsi_mem_info.evt_ring_len = gsi_evt_ring_props.ring_len;
		ep->gsi_mem_info.evt_ring_base_addr =
			gsi_evt_ring_props.ring_base_addr;
		ep->gsi_mem_info.evt_ring_base_vaddr =
			gsi_evt_ring_props.ring_base_vaddr;

		gsi_evt_ring_props.int_modt = IPA_GSI_EVT_RING_INT_MODT;
		gsi_evt_ring_props.int_modc = 1;

		IPADBG("client=%d moderation threshold cycles=%u cnt=%u\n",
			ep->client,
			gsi_evt_ring_props.int_modt,
			gsi_evt_ring_props.int_modc);
		gsi_evt_ring_props.rp_update_addr = 0;
		gsi_evt_ring_props.exclusive = true;
		gsi_evt_ring_props.err_cb = ipa_gsi_evt_ring_err_cb;
		gsi_evt_ring_props.user_data = NULL;

		result = gsi_alloc_evt_ring(&gsi_evt_ring_props,
			ipa3_ctx->gsi_dev_hdl, &ep->gsi_evt_ring_hdl);
		if (result != GSI_STATUS_SUCCESS)
			goto fail_alloc_evt_ring;
	}

	memset(&gsi_channel_props, 0, sizeof(gsi_channel_props));
	gsi_channel_props.prot = GSI_CHAN_PROT_GPI;
	if (IPA_CLIENT_IS_PROD(ep->client)) {
		gsi_channel_props.dir = GSI_CHAN_DIR_TO_GSI;
	} else {
		gsi_channel_props.dir = GSI_CHAN_DIR_FROM_GSI;
		gsi_channel_props.max_re_expected = ep->sys->rx_pool_sz;
	}

	gsi_ep_info = ipa3_get_gsi_ep_info(ep->client);
	if (!gsi_ep_info) {
		IPAERR("Failed getting GSI EP info for client=%d\n",
		       ep->client);
		result = -EINVAL;
		goto fail_get_gsi_ep_info;
	} else
		gsi_channel_props.ch_id = gsi_ep_info->ipa_gsi_chan_num;

	gsi_channel_props.evt_ring_hdl = ep->gsi_evt_ring_hdl;
	gsi_channel_props.re_size = GSI_CHAN_RE_SIZE_16B;

	/*
	 * GSI ring length is calculated based on the desc_fifo_sz which was
	 * meant to define the BAM desc fifo. GSI descriptors are 16B as opposed
	 * to 8B for BAM. For PROD pipes there is also an additional descriptor
	 * for TAG STATUS immediate command. APPS_WAN_PROD pipe is an exception
	 * as this pipe do not use TAG STATUS for completion. Instead it uses
	 * event ring based completions.
	 */
	if (ep->client == IPA_CLIENT_APPS_WAN_PROD)
		gsi_channel_props.ring_len = 2 * in->desc_fifo_sz;
	else if (IPA_CLIENT_IS_PROD(ep->client))
		gsi_channel_props.ring_len = 4 * in->desc_fifo_sz;
	else
		gsi_channel_props.ring_len = 2 * in->desc_fifo_sz;
	gsi_channel_props.ring_base_vaddr =
		dma_alloc_coherent(ipa3_ctx->pdev, gsi_channel_props.ring_len,
			&dma_addr, mem_flag);
	if (!gsi_channel_props.ring_base_vaddr) {
		IPAERR("fail to dma alloc %u bytes\n",
			gsi_channel_props.ring_len);
		result = -ENOMEM;
		goto fail_alloc_channel_ring;
	}
	gsi_channel_props.ring_base_addr = dma_addr;

	/* copy mem info */
	ep->gsi_mem_info.chan_ring_len = gsi_channel_props.ring_len;
	ep->gsi_mem_info.chan_ring_base_addr =
		gsi_channel_props.ring_base_addr;
	ep->gsi_mem_info.chan_ring_base_vaddr =
		gsi_channel_props.ring_base_vaddr;

	if (ipa3_ctx->ipa_hw_type >= IPA_HW_v4_0)
		gsi_channel_props.use_db_eng = GSI_CHAN_DIRECT_MODE;
	else
		gsi_channel_props.use_db_eng = GSI_CHAN_DB_MODE;
	gsi_channel_props.max_prefetch = GSI_ONE_PREFETCH_SEG;
	gsi_channel_props.prefetch_mode =
		ipa_get_ep_prefetch_mode(ep->client);
	if (ep->client == IPA_CLIENT_APPS_CMD_PROD)
		gsi_channel_props.low_weight = IPA_GSI_MAX_CH_LOW_WEIGHT;
	else
		gsi_channel_props.low_weight = 1;
	gsi_channel_props.chan_user_data = ep->sys;
	gsi_channel_props.err_cb = ipa_gsi_chan_err_cb;
	if (IPA_CLIENT_IS_PROD(ep->client))
		gsi_channel_props.xfer_cb = ipa_gsi_irq_tx_notify_cb;
	else
		gsi_channel_props.xfer_cb = ipa_gsi_irq_rx_notify_cb;
	if (IPA_CLIENT_IS_MEMCPY_DMA_CONS(ep->client))
		gsi_channel_props.xfer_cb = ipa_dma_gsi_irq_rx_notify_cb;
	result = gsi_alloc_channel(&gsi_channel_props, ipa3_ctx->gsi_dev_hdl,
		&ep->gsi_chan_hdl);
	if (result != GSI_STATUS_SUCCESS)
		goto fail_alloc_channel;

	memset(&ch_scratch, 0, sizeof(ch_scratch));
	ch_scratch.gpi.max_outstanding_tre = gsi_ep_info->ipa_if_tlv *
		GSI_CHAN_RE_SIZE_16B;
	ch_scratch.gpi.outstanding_threshold = 2 * GSI_CHAN_RE_SIZE_16B;

	if (ipa3_ctx->ipa_hw_type >= IPA_HW_v4_0) {
		ch_scratch.gpi.max_outstanding_tre = 0;
		ch_scratch.gpi.outstanding_threshold = 0;
	}

	result = gsi_write_channel_scratch(ep->gsi_chan_hdl, ch_scratch);
	if (result != GSI_STATUS_SUCCESS) {
		IPAERR("failed to write scratch %d\n", result);
		goto fail_write_channel_scratch;
	}

	if (ep->client == IPA_CLIENT_MEMCPY_DMA_SYNC_CONS)
		gsi_config_channel_mode(ep->gsi_chan_hdl,
				GSI_CHAN_MODE_POLL);
	return 0;

fail_write_channel_scratch:
	if (gsi_dealloc_channel(ep->gsi_chan_hdl)
		!= GSI_STATUS_SUCCESS) {
		IPAERR("Failed to dealloc GSI chan.\n");
		BUG();
	}
fail_alloc_channel:
	dma_free_coherent(ipa3_ctx->pdev, gsi_channel_props.ring_len,
			gsi_channel_props.ring_base_vaddr, dma_addr);
fail_alloc_channel_ring:
fail_get_gsi_ep_info:
	if (ep->gsi_evt_ring_hdl != ~0) {
		gsi_dealloc_evt_ring(ep->gsi_evt_ring_hdl);
		ep->gsi_evt_ring_hdl = ~0;
	}
fail_alloc_evt_ring:
	if (gsi_evt_ring_props.ring_base_vaddr)
		dma_free_coherent(ipa3_ctx->pdev, gsi_evt_ring_props.ring_len,
			gsi_evt_ring_props.ring_base_vaddr, evt_dma_addr);
	IPAERR("Return with err: %d\n", result);
	return result;
}

static int ipa_populate_tag_field(struct ipa3_desc *desc,
		struct ipa3_tx_pkt_wrapper *tx_pkt,
		struct ipahal_imm_cmd_pyld **tag_pyld_ret)
{
	struct ipahal_imm_cmd_pyld *tag_pyld;
	struct ipahal_imm_cmd_ip_packet_tag_status tag_cmd = {0};

	/* populate tag field only if it is NULL */
	if (desc->pyld == NULL) {
		tag_cmd.tag = pointer_to_tag_wa(tx_pkt);
		tag_pyld = ipahal_construct_imm_cmd(
			IPA_IMM_CMD_IP_PACKET_TAG_STATUS, &tag_cmd, true);
		if (unlikely(!tag_pyld)) {
			IPAERR("Failed to construct ip_packet_tag_status\n");
			return -EFAULT;
		}
		/*
		 * This is for 32-bit pointer, will need special
		 * handling if 64-bit pointer is used
		 */
		IPADBG_LOW("tx_pkt sent in tag: 0x%p\n", tx_pkt);
		desc->pyld = tag_pyld->data;
		desc->opcode = tag_pyld->opcode;
		desc->len = tag_pyld->len;
		desc->user1 = tag_pyld;
		desc->type = IPA_IMM_CMD_DESC;
		desc->callback = ipa3_tag_destroy_imm;

		*tag_pyld_ret = tag_pyld;
	}
	return 0;
}

static int ipa_poll_gsi_pkt(struct ipa3_sys_context *sys,
		struct ipa_mem_buffer *mem_info)
{
	int ret;
	struct gsi_chan_xfer_notify xfer_notify;
	struct ipa3_rx_pkt_wrapper *rx_pkt;

	if (sys->ep->bytes_xfered_valid) {
		mem_info->phys_base = sys->ep->phys_base;
		mem_info->size = (u32)sys->ep->bytes_xfered;
		sys->ep->bytes_xfered_valid = false;
		return GSI_STATUS_SUCCESS;
	}

	ret = gsi_poll_channel(sys->ep->gsi_chan_hdl,
		&xfer_notify);
	if (ret == GSI_STATUS_POLL_EMPTY)
		return ret;
	else if (ret != GSI_STATUS_SUCCESS) {
		IPAERR("Poll channel err: %d\n", ret);
		return ret;
	}

	rx_pkt = (struct ipa3_rx_pkt_wrapper *)
		xfer_notify.xfer_user_data;
	mem_info->phys_base = rx_pkt->data.dma_addr;
	mem_info->size = xfer_notify.bytes_xfered;

	return ret;
}

/**
 * ipa3_rx_poll() - Poll the rx packets from IPA HW. This
 * function is exectued in the softirq context
 *
 * if input budget is zero, the driver switches back to
 * interrupt mode.
 *
 * return number of polled packets, on error 0(zero)
 */
int ipa3_rx_poll(u32 clnt_hdl, int weight)
{
	struct ipa3_ep_context *ep;
	int ret;
	int cnt = 0;
	struct ipa_mem_buffer mem_info = {0};
	static int total_cnt;
	struct ipa_active_client_logging_info log;

	IPA_ACTIVE_CLIENTS_PREP_SPECIAL(log, "NAPI");

	if (clnt_hdl >= ipa3_ctx->ipa_num_pipes ||
		ipa3_ctx->ep[clnt_hdl].valid == 0) {
		IPAERR("bad parm 0x%x\n", clnt_hdl);
		return cnt;
	}

	ep = &ipa3_ctx->ep[clnt_hdl];

	while (cnt < weight &&
		   atomic_read(&ep->sys->curr_polling_state)) {

		atomic_set(&ipa3_ctx->transport_pm.eot_activity, 1);
		ret = ipa_poll_gsi_pkt(ep->sys, &mem_info);
		if (ret)
			break;

		ipa3_wq_rx_common(ep->sys, mem_info.size);
		cnt += IPA_WAN_AGGR_PKT_CNT;
		total_cnt++;

		if (ep->sys->len == 0) {
			total_cnt = 0;
			cnt = cnt-1;
			break;
		}
	};

	if (cnt < weight) {
		ep->client_notify(ep->priv, IPA_CLIENT_COMP_NAPI, 0);
		ipa3_rx_switch_to_intr_mode(ep->sys);
		if (ipa3_ctx->use_ipa_pm)
			ipa_pm_deferred_deactivate(ep->sys->pm_hdl);
		else
			ipa3_dec_client_disable_clks_no_block(&log);
	}

	return cnt;
}

static unsigned long tag_to_pointer_wa(uint64_t tag)
{
	return 0xFFFF000000000000 | (unsigned long) tag;
}

static uint64_t pointer_to_tag_wa(struct ipa3_tx_pkt_wrapper *tx_pkt)
{
	u16 temp;
	/* Add the check but it might have throughput issue */
	if (ipa3_is_msm_device()) {
		temp = (u16) (~((unsigned long) tx_pkt &
			0xFFFF000000000000) >> 48);
		if (temp) {
			IPAERR("The 16 prefix is not all 1s (%p)\n",
			tx_pkt);
			BUG();
		}
	}
	return (unsigned long)tx_pkt & 0x0000FFFFFFFFFFFF;
}

/**
 * ipa_gsi_ch20_wa() - software workaround for IPA GSI channel 20
 *
 * A hardware limitation requires to avoid using GSI physical channel 20.
 * This function allocates GSI physical channel 20 and holds it to prevent
 * others to use it.
 *
 * Return codes: 0 on success, negative on failure
 */
int ipa_gsi_ch20_wa(void)
{
	struct gsi_chan_props gsi_channel_props;
	dma_addr_t dma_addr;
	int result;
	int i;
	unsigned long chan_hdl[IPA_GSI_CH_20_WA_NUM_CH_TO_ALLOC];
	unsigned long chan_hdl_to_keep;


	memset(&gsi_channel_props, 0, sizeof(gsi_channel_props));
	gsi_channel_props.prot = GSI_CHAN_PROT_GPI;
	gsi_channel_props.dir = GSI_CHAN_DIR_TO_GSI;
	gsi_channel_props.evt_ring_hdl = ~0;
	gsi_channel_props.re_size = GSI_CHAN_RE_SIZE_16B;
	gsi_channel_props.ring_len = 4 * gsi_channel_props.re_size;
	gsi_channel_props.ring_base_vaddr =
		dma_alloc_coherent(ipa3_ctx->pdev, gsi_channel_props.ring_len,
		&dma_addr, 0);
	gsi_channel_props.ring_base_addr = dma_addr;

	if (ipa3_ctx->ipa_hw_type >= IPA_HW_v4_0)
		gsi_channel_props.use_db_eng = GSI_CHAN_DIRECT_MODE;
	else
		gsi_channel_props.use_db_eng = GSI_CHAN_DB_MODE;

	gsi_channel_props.max_prefetch = GSI_ONE_PREFETCH_SEG;
	gsi_channel_props.low_weight = 1;
	gsi_channel_props.err_cb = ipa_gsi_chan_err_cb;
	gsi_channel_props.xfer_cb = ipa_gsi_irq_tx_notify_cb;

	/* first allocate channels up to channel 20 */
	for (i = 0; i < IPA_GSI_CH_20_WA_NUM_CH_TO_ALLOC; i++) {
		gsi_channel_props.ch_id = i;
		result = gsi_alloc_channel(&gsi_channel_props,
			ipa3_ctx->gsi_dev_hdl,
			&chan_hdl[i]);
		if (result != GSI_STATUS_SUCCESS) {
			IPAERR("failed to alloc channel %d err %d\n",
				i, result);
			return result;
		}
	}

	/* allocate channel 20 */
	gsi_channel_props.ch_id = IPA_GSI_CH_20_WA_VIRT_CHAN;
	result = gsi_alloc_channel(&gsi_channel_props, ipa3_ctx->gsi_dev_hdl,
		&chan_hdl_to_keep);
	if (result != GSI_STATUS_SUCCESS) {
		IPAERR("failed to alloc channel %d err %d\n",
			i, result);
		return result;
	}

	/* release all other channels */
	for (i = 0; i < IPA_GSI_CH_20_WA_NUM_CH_TO_ALLOC; i++) {
		result = gsi_dealloc_channel(chan_hdl[i]);
		if (result != GSI_STATUS_SUCCESS) {
			IPAERR("failed to dealloc channel %d err %d\n",
				i, result);
			return result;
		}
	}

	/* DMA memory shall not be freed as it is used by channel 20 */
	return 0;
}

/**
 * ipa_adjust_ra_buff_base_sz()
 *
 * Return value: the largest power of two which is smaller
 * than the input value
 */
static u32 ipa_adjust_ra_buff_base_sz(u32 aggr_byte_limit)
{
	aggr_byte_limit += IPA_MTU;
	aggr_byte_limit += IPA_GENERIC_RX_BUFF_LIMIT;
	aggr_byte_limit--;
	aggr_byte_limit |= aggr_byte_limit >> 1;
	aggr_byte_limit |= aggr_byte_limit >> 2;
	aggr_byte_limit |= aggr_byte_limit >> 4;
	aggr_byte_limit |= aggr_byte_limit >> 8;
	aggr_byte_limit |= aggr_byte_limit >> 16;
	aggr_byte_limit++;
	return aggr_byte_limit >> 1;
}
