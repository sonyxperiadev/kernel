/*
 * Copyright (c) 2016-2020 The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include "hal_hw_headers.h"
#include "dp_types.h"
#include "dp_rx.h"
#include "dp_peer.h"
#include "hal_rx.h"
#include "hal_api.h"
#include "qdf_nbuf.h"
#ifdef MESH_MODE_SUPPORT
#include "if_meta_hdr.h"
#endif
#include "dp_internal.h"
#include "dp_rx_mon.h"
#include "dp_ipa.h"
#ifdef FEATURE_WDS
#include "dp_txrx_wds.h"
#endif
#include "dp_hist.h"
#include "dp_rx_buffer_pool.h"

#ifdef ATH_RX_PRI_SAVE
#define DP_RX_TID_SAVE(_nbuf, _tid) \
	(qdf_nbuf_set_priority(_nbuf, _tid))
#else
#define DP_RX_TID_SAVE(_nbuf, _tid)
#endif

#ifdef DP_RX_DISABLE_NDI_MDNS_FORWARDING
static inline
bool dp_rx_check_ndi_mdns_fwding(struct dp_peer *ta_peer, qdf_nbuf_t nbuf)
{
	if (ta_peer->vdev->opmode == wlan_op_mode_ndi &&
	    qdf_nbuf_is_ipv6_mdns_pkt(nbuf)) {
		DP_STATS_INC(ta_peer, rx.intra_bss.mdns_no_fwd, 1);
		return false;
	}
		return true;
}
#else
static inline
bool dp_rx_check_ndi_mdns_fwding(struct dp_peer *ta_peer, qdf_nbuf_t nbuf)
{
	return true;
}
#endif
static inline bool dp_rx_check_ap_bridge(struct dp_vdev *vdev)
{
	return vdev->ap_bridge_enabled;
}

#ifdef DUP_RX_DESC_WAR
void dp_rx_dump_info_and_assert(struct dp_soc *soc,
				hal_ring_handle_t hal_ring,
				hal_ring_desc_t ring_desc,
				struct dp_rx_desc *rx_desc)
{
	void *hal_soc = soc->hal_soc;

	hal_srng_dump_ring_desc(hal_soc, hal_ring, ring_desc);
	dp_rx_desc_dump(rx_desc);
}
#else
void dp_rx_dump_info_and_assert(struct dp_soc *soc,
				hal_ring_handle_t hal_ring_hdl,
				hal_ring_desc_t ring_desc,
				struct dp_rx_desc *rx_desc)
{
	hal_soc_handle_t hal_soc = soc->hal_soc;

	dp_rx_desc_dump(rx_desc);
	hal_srng_dump_ring_desc(hal_soc, hal_ring_hdl, ring_desc);
	hal_srng_dump_ring(hal_soc, hal_ring_hdl);
	qdf_assert_always(0);
}
#endif

#ifdef RX_DESC_SANITY_WAR
static inline
QDF_STATUS dp_rx_desc_sanity(struct dp_soc *soc, hal_soc_handle_t hal_soc,
			     hal_ring_handle_t hal_ring_hdl,
			     hal_ring_desc_t ring_desc,
			     struct dp_rx_desc *rx_desc)
{
	uint8_t return_buffer_manager;

	if (qdf_unlikely(!rx_desc)) {
		/*
		 * This is an unlikely case where the cookie obtained
		 * from the ring_desc is invalid and hence we are not
		 * able to find the corresponding rx_desc
		 */
		goto fail;
	}

	return_buffer_manager = hal_rx_ret_buf_manager_get(ring_desc);
	if (qdf_unlikely(!(return_buffer_manager == HAL_RX_BUF_RBM_SW1_BM ||
			 return_buffer_manager == HAL_RX_BUF_RBM_SW3_BM))) {
		goto fail;
	}

	return QDF_STATUS_SUCCESS;

fail:
	DP_STATS_INC(soc, rx.err.invalid_cookie, 1);
	dp_err("Ring Desc:");
	hal_srng_dump_ring_desc(hal_soc, hal_ring_hdl,
				ring_desc);
	return QDF_STATUS_E_NULL_VALUE;

}
#else
static inline
QDF_STATUS dp_rx_desc_sanity(struct dp_soc *soc, hal_soc_handle_t hal_soc,
			     hal_ring_handle_t hal_ring_hdl,
			     hal_ring_desc_t ring_desc,
			     struct dp_rx_desc *rx_desc)
{
	return QDF_STATUS_SUCCESS;
}
#endif

/**
 * dp_pdev_frag_alloc_and_map() - Allocate frag for desc buffer and map
 *
 * @dp_soc: struct dp_soc *
 * @nbuf_frag_info_t: nbuf frag info
 * @dp_pdev: struct dp_pdev *
 * @rx_desc_pool: Rx desc pool
 *
 * Return: QDF_STATUS
 */
#ifdef DP_RX_MON_MEM_FRAG
static inline QDF_STATUS
dp_pdev_frag_alloc_and_map(struct dp_soc *dp_soc,
			   struct dp_rx_nbuf_frag_info *nbuf_frag_info_t,
			   struct dp_pdev *dp_pdev,
			   struct rx_desc_pool *rx_desc_pool)
{
	QDF_STATUS ret = QDF_STATUS_E_FAILURE;

	(nbuf_frag_info_t->virt_addr).vaddr =
			qdf_frag_alloc(rx_desc_pool->buf_size);

	if (!((nbuf_frag_info_t->virt_addr).vaddr)) {
		dp_err("Frag alloc failed");
		DP_STATS_INC(dp_pdev, replenish.frag_alloc_fail, 1);
		return QDF_STATUS_E_NOMEM;
	}

	ret = qdf_mem_map_page(dp_soc->osdev,
			       (nbuf_frag_info_t->virt_addr).vaddr,
			       QDF_DMA_FROM_DEVICE,
			       rx_desc_pool->buf_size,
			       &nbuf_frag_info_t->paddr);

	if (qdf_unlikely(QDF_IS_STATUS_ERROR(ret))) {
		qdf_frag_free((nbuf_frag_info_t->virt_addr).vaddr);
		dp_err("Frag map failed");
		DP_STATS_INC(dp_pdev, replenish.map_err, 1);
		return QDF_STATUS_E_FAULT;
	}

	return QDF_STATUS_SUCCESS;
}
#else
static inline QDF_STATUS
dp_pdev_frag_alloc_and_map(struct dp_soc *dp_soc,
			   struct dp_rx_nbuf_frag_info *nbuf_frag_info_t,
			   struct dp_pdev *dp_pdev,
			   struct rx_desc_pool *rx_desc_pool)
{
	return QDF_STATUS_SUCCESS;
}
#endif /* DP_RX_MON_MEM_FRAG */

/**
 * dp_pdev_nbuf_alloc_and_map() - Allocate nbuf for desc buffer and map
 *
 * @dp_soc: struct dp_soc *
 * @mac_id: Mac id
 * @num_entries_avail: num_entries_avail
 * @nbuf_frag_info_t: nbuf frag info
 * @dp_pdev: struct dp_pdev *
 * @rx_desc_pool: Rx desc pool
 *
 * Return: QDF_STATUS
 */
static inline QDF_STATUS
dp_pdev_nbuf_alloc_and_map_replenish(struct dp_soc *dp_soc,
				     uint32_t mac_id,
				     uint32_t num_entries_avail,
				     struct dp_rx_nbuf_frag_info *nbuf_frag_info_t,
				     struct dp_pdev *dp_pdev,
				     struct rx_desc_pool *rx_desc_pool)
{
	QDF_STATUS ret = QDF_STATUS_E_FAILURE;

	(nbuf_frag_info_t->virt_addr).nbuf =
		dp_rx_buffer_pool_nbuf_alloc(dp_soc,
					     mac_id,
					     rx_desc_pool,
					     num_entries_avail);
	if (!((nbuf_frag_info_t->virt_addr).nbuf)) {
		dp_err("nbuf alloc failed");
		DP_STATS_INC(dp_pdev, replenish.nbuf_alloc_fail, 1);
		return QDF_STATUS_E_NOMEM;
	}

	ret = qdf_nbuf_map_nbytes_single(dp_soc->osdev,
					 (nbuf_frag_info_t->virt_addr).nbuf,
					 QDF_DMA_FROM_DEVICE,
					 rx_desc_pool->buf_size);

	if (qdf_unlikely(QDF_IS_STATUS_ERROR(ret))) {
		dp_rx_buffer_pool_nbuf_free(dp_soc,
			(nbuf_frag_info_t->virt_addr).nbuf, mac_id);
		dp_err("nbuf map failed");
		DP_STATS_INC(dp_pdev, replenish.map_err, 1);
		return QDF_STATUS_E_FAULT;
	}

	nbuf_frag_info_t->paddr =
		qdf_nbuf_get_frag_paddr((nbuf_frag_info_t->virt_addr).nbuf, 0);

	dp_ipa_handle_rx_buf_smmu_mapping(dp_soc,
			(qdf_nbuf_t)((nbuf_frag_info_t->virt_addr).nbuf),
					  rx_desc_pool->buf_size,
					  true);

	ret = check_x86_paddr(dp_soc, &((nbuf_frag_info_t->virt_addr).nbuf),
			      &nbuf_frag_info_t->paddr,
			      rx_desc_pool);
	if (ret == QDF_STATUS_E_FAILURE) {
		qdf_nbuf_unmap_nbytes_single(dp_soc->osdev,
					     (nbuf_frag_info_t->virt_addr).nbuf,
					     QDF_DMA_FROM_DEVICE,
					     rx_desc_pool->buf_size);
		DP_STATS_INC(dp_pdev, replenish.x86_fail, 1);
		return QDF_STATUS_E_ADDRNOTAVAIL;
	}

	return QDF_STATUS_SUCCESS;
}

/*
 * dp_rx_buffers_replenish() - replenish rxdma ring with rx nbufs
 *			       called during dp rx initialization
 *			       and at the end of dp_rx_process.
 *
 * @soc: core txrx main context
 * @mac_id: mac_id which is one of 3 mac_ids
 * @dp_rxdma_srng: dp rxdma circular ring
 * @rx_desc_pool: Pointer to free Rx descriptor pool
 * @num_req_buffers: number of buffer to be replenished
 * @desc_list: list of descs if called from dp_rx_process
 *	       or NULL during dp rx initialization or out of buffer
 *	       interrupt.
 * @tail: tail of descs list
 * @func_name: name of the caller function
 * Return: return success or failure
 */
QDF_STATUS __dp_rx_buffers_replenish(struct dp_soc *dp_soc, uint32_t mac_id,
				struct dp_srng *dp_rxdma_srng,
				struct rx_desc_pool *rx_desc_pool,
				uint32_t num_req_buffers,
				union dp_rx_desc_list_elem_t **desc_list,
				union dp_rx_desc_list_elem_t **tail,
				const char *func_name)
{
	uint32_t num_alloc_desc;
	uint16_t num_desc_to_free = 0;
	struct dp_pdev *dp_pdev = dp_get_pdev_for_lmac_id(dp_soc, mac_id);
	uint32_t num_entries_avail;
	uint32_t count;
	int sync_hw_ptr = 1;
	struct dp_rx_nbuf_frag_info nbuf_frag_info = {0};
	void *rxdma_ring_entry;
	union dp_rx_desc_list_elem_t *next;
	QDF_STATUS ret;
	void *rxdma_srng;

	rxdma_srng = dp_rxdma_srng->hal_srng;

	if (!rxdma_srng) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
				  "rxdma srng not initialized");
		DP_STATS_INC(dp_pdev, replenish.rxdma_err, num_req_buffers);
		return QDF_STATUS_E_FAILURE;
	}

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
		"requested %d buffers for replenish", num_req_buffers);

	hal_srng_access_start(dp_soc->hal_soc, rxdma_srng);
	num_entries_avail = hal_srng_src_num_avail(dp_soc->hal_soc,
						   rxdma_srng,
						   sync_hw_ptr);

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
		"no of available entries in rxdma ring: %d",
		num_entries_avail);

	if (!(*desc_list) && (num_entries_avail >
		((dp_rxdma_srng->num_entries * 3) / 4))) {
		num_req_buffers = num_entries_avail;
	} else if (num_entries_avail < num_req_buffers) {
		num_desc_to_free = num_req_buffers - num_entries_avail;
		num_req_buffers = num_entries_avail;
	}

	if (qdf_unlikely(!num_req_buffers)) {
		num_desc_to_free = num_req_buffers;
		hal_srng_access_end(dp_soc->hal_soc, rxdma_srng);
		goto free_descs;
	}

	/*
	 * if desc_list is NULL, allocate the descs from freelist
	 */
	if (!(*desc_list)) {
		num_alloc_desc = dp_rx_get_free_desc_list(dp_soc, mac_id,
							  rx_desc_pool,
							  num_req_buffers,
							  desc_list,
							  tail);

		if (!num_alloc_desc) {
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				"no free rx_descs in freelist");
			DP_STATS_INC(dp_pdev, err.desc_alloc_fail,
					num_req_buffers);
			hal_srng_access_end(dp_soc->hal_soc, rxdma_srng);
			return QDF_STATUS_E_NOMEM;
		}

		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			"%d rx desc allocated", num_alloc_desc);
		num_req_buffers = num_alloc_desc;
	}


	count = 0;

	while (count < num_req_buffers) {
		/* Flag is set while pdev rx_desc_pool initialization */
		if (qdf_unlikely(rx_desc_pool->rx_mon_dest_frag_enable))
			ret = dp_pdev_frag_alloc_and_map(dp_soc,
							 &nbuf_frag_info,
							 dp_pdev,
							 rx_desc_pool);
		else
			ret = dp_pdev_nbuf_alloc_and_map_replenish(dp_soc,
								   mac_id,
					num_entries_avail, &nbuf_frag_info,
					dp_pdev, rx_desc_pool);

		if (qdf_unlikely(QDF_IS_STATUS_ERROR(ret))) {
			if (qdf_unlikely(ret  == QDF_STATUS_E_FAULT))
				continue;
			break;
		}

		count++;

		rxdma_ring_entry = hal_srng_src_get_next(dp_soc->hal_soc,
							 rxdma_srng);
		qdf_assert_always(rxdma_ring_entry);

		next = (*desc_list)->next;

		/* Flag is set while pdev rx_desc_pool initialization */
		if (qdf_unlikely(rx_desc_pool->rx_mon_dest_frag_enable))
			dp_rx_desc_frag_prep(&((*desc_list)->rx_desc),
					     &nbuf_frag_info);
		else
			dp_rx_desc_prep(&((*desc_list)->rx_desc),
					&nbuf_frag_info);

		/* rx_desc.in_use should be zero at this time*/
		qdf_assert_always((*desc_list)->rx_desc.in_use == 0);

		(*desc_list)->rx_desc.in_use = 1;
		(*desc_list)->rx_desc.in_err_state = 0;
		dp_rx_desc_update_dbg_info(&(*desc_list)->rx_desc,
					   func_name, RX_DESC_REPLENISHED);
		dp_verbose_debug("rx_netbuf=%pK, paddr=0x%llx, cookie=%d",
				 nbuf_frag_info.virt_addr.nbuf,
				 (unsigned long long)(nbuf_frag_info.paddr),
				 (*desc_list)->rx_desc.cookie);

		hal_rxdma_buff_addr_info_set(rxdma_ring_entry,
					     nbuf_frag_info.paddr,
						(*desc_list)->rx_desc.cookie,
						rx_desc_pool->owner);

		*desc_list = next;

	}

	hal_srng_access_end(dp_soc->hal_soc, rxdma_srng);

	dp_verbose_debug("replenished buffers %d, rx desc added back to free list %u",
			 count, num_desc_to_free);

	/* No need to count the number of bytes received during replenish.
	 * Therefore set replenish.pkts.bytes as 0.
	 */
	DP_STATS_INC_PKT(dp_pdev, replenish.pkts, count, 0);

free_descs:
	DP_STATS_INC(dp_pdev, buf_freelist, num_desc_to_free);
	/*
	 * add any available free desc back to the free list
	 */
	if (*desc_list)
		dp_rx_add_desc_list_to_free_list(dp_soc, desc_list, tail,
			mac_id, rx_desc_pool);

	return QDF_STATUS_SUCCESS;
}

/*
 * dp_rx_deliver_raw() - process RAW mode pkts and hand over the
 *				pkts to RAW mode simulation to
 *				decapsulate the pkt.
 *
 * @vdev: vdev on which RAW mode is enabled
 * @nbuf_list: list of RAW pkts to process
 * @peer: peer object from which the pkt is rx
 *
 * Return: void
 */
void
dp_rx_deliver_raw(struct dp_vdev *vdev, qdf_nbuf_t nbuf_list,
					struct dp_peer *peer)
{
	qdf_nbuf_t deliver_list_head = NULL;
	qdf_nbuf_t deliver_list_tail = NULL;
	qdf_nbuf_t nbuf;

	nbuf = nbuf_list;
	while (nbuf) {
		qdf_nbuf_t next = qdf_nbuf_next(nbuf);

		DP_RX_LIST_APPEND(deliver_list_head, deliver_list_tail, nbuf);

		DP_STATS_INC(vdev->pdev, rx_raw_pkts, 1);
		DP_STATS_INC_PKT(peer, rx.raw, 1, qdf_nbuf_len(nbuf));
		/*
		 * reset the chfrag_start and chfrag_end bits in nbuf cb
		 * as this is a non-amsdu pkt and RAW mode simulation expects
		 * these bit s to be 0 for non-amsdu pkt.
		 */
		if (qdf_nbuf_is_rx_chfrag_start(nbuf) &&
			 qdf_nbuf_is_rx_chfrag_end(nbuf)) {
			qdf_nbuf_set_rx_chfrag_start(nbuf, 0);
			qdf_nbuf_set_rx_chfrag_end(nbuf, 0);
		}

		nbuf = next;
	}

	vdev->osif_rsim_rx_decap(vdev->osif_vdev, &deliver_list_head,
				 &deliver_list_tail, peer->mac_addr.raw);

	vdev->osif_rx(vdev->osif_vdev, deliver_list_head);
}

#ifndef FEATURE_WDS
static void
dp_rx_da_learn(struct dp_soc *soc,
	       uint8_t *rx_tlv_hdr,
	       struct dp_peer *ta_peer,
	       qdf_nbuf_t nbuf)
{
}
#endif
/*
 * dp_rx_intrabss_fwd() - Implements the Intra-BSS forwarding logic
 *
 * @soc: core txrx main context
 * @ta_peer	: source peer entry
 * @rx_tlv_hdr	: start address of rx tlvs
 * @nbuf	: nbuf that has to be intrabss forwarded
 *
 * Return: bool: true if it is forwarded else false
 */
static bool
dp_rx_intrabss_fwd(struct dp_soc *soc,
			struct dp_peer *ta_peer,
			uint8_t *rx_tlv_hdr,
			qdf_nbuf_t nbuf,
			struct hal_rx_msdu_metadata msdu_metadata)
{
	uint16_t len;
	uint8_t is_frag;
	uint16_t da_peer_id = HTT_INVALID_PEER;
	struct dp_peer *da_peer = NULL;
	bool is_da_bss_peer = false;
	struct dp_ast_entry *ast_entry;
	qdf_nbuf_t nbuf_copy;
	uint8_t tid = qdf_nbuf_get_tid_val(nbuf);
	uint8_t ring_id = QDF_NBUF_CB_RX_CTX_ID(nbuf);
	struct cdp_tid_rx_stats *tid_stats = &ta_peer->vdev->pdev->stats.
					tid_stats.tid_rx_stats[ring_id][tid];

	/* check if the destination peer is available in peer table
	 * and also check if the source peer and destination peer
	 * belong to the same vap and destination peer is not bss peer.
	 */

	if ((qdf_nbuf_is_da_valid(nbuf) && !qdf_nbuf_is_da_mcbc(nbuf))) {

		ast_entry = soc->ast_table[msdu_metadata.da_idx];
		if (!ast_entry)
			return false;

		if (ast_entry->type == CDP_TXRX_AST_TYPE_DA) {
			ast_entry->is_active = TRUE;
			return false;
		}

		da_peer_id = ast_entry->peer_id;

		if (da_peer_id == HTT_INVALID_PEER)
			return false;
		/* TA peer cannot be same as peer(DA) on which AST is present
		 * this indicates a change in topology and that AST entries
		 * are yet to be updated.
		 */
		if (da_peer_id == ta_peer->peer_id)
			return false;

		if (ast_entry->vdev_id != ta_peer->vdev->vdev_id)
			return false;

		da_peer = dp_peer_get_ref_by_id(soc, da_peer_id,
						DP_MOD_ID_RX);
		if (!da_peer)
			return false;
		is_da_bss_peer = da_peer->bss_peer;
		dp_peer_unref_delete(da_peer, DP_MOD_ID_RX);

		if (!is_da_bss_peer) {
			len = QDF_NBUF_CB_RX_PKT_LEN(nbuf);
			is_frag = qdf_nbuf_is_frag(nbuf);
			memset(nbuf->cb, 0x0, sizeof(nbuf->cb));

			/* If the source or destination peer in the isolation
			 * list then dont forward instead push to bridge stack.
			 */
			if (dp_get_peer_isolation(ta_peer) ||
			    dp_get_peer_isolation(da_peer))
				return false;

			/* linearize the nbuf just before we send to
			 * dp_tx_send()
			 */
			if (qdf_unlikely(is_frag)) {
				if (qdf_nbuf_linearize(nbuf) == -ENOMEM)
					return false;

				nbuf = qdf_nbuf_unshare(nbuf);
				if (!nbuf) {
					DP_STATS_INC_PKT(ta_peer,
							 rx.intra_bss.fail,
							 1,
							 len);
					/* return true even though the pkt is
					 * not forwarded. Basically skb_unshare
					 * failed and we want to continue with
					 * next nbuf.
					 */
					tid_stats->fail_cnt[INTRABSS_DROP]++;
					return true;
				}
			}

			if (!dp_tx_send((struct cdp_soc_t *)soc,
					ta_peer->vdev->vdev_id, nbuf)) {
				DP_STATS_INC_PKT(ta_peer, rx.intra_bss.pkts, 1,
						 len);
				return true;
			} else {
				DP_STATS_INC_PKT(ta_peer, rx.intra_bss.fail, 1,
						len);
				tid_stats->fail_cnt[INTRABSS_DROP]++;
				return false;
			}
		}
	}
	/* if it is a broadcast pkt (eg: ARP) and it is not its own
	 * source, then clone the pkt and send the cloned pkt for
	 * intra BSS forwarding and original pkt up the network stack
	 * Note: how do we handle multicast pkts. do we forward
	 * all multicast pkts as is or let a higher layer module
	 * like igmpsnoop decide whether to forward or not with
	 * Mcast enhancement.
	 */
	else if (qdf_unlikely((qdf_nbuf_is_da_mcbc(nbuf) &&
			       !ta_peer->bss_peer))) {
		if (!dp_rx_check_ndi_mdns_fwding(ta_peer, nbuf))
			goto end;

		/* If the source peer in the isolation list
		 * then dont forward instead push to bridge stack
		 */
		if (dp_get_peer_isolation(ta_peer))
			goto end;

		nbuf_copy = qdf_nbuf_copy(nbuf);
		if (!nbuf_copy)
			goto end;

		len = QDF_NBUF_CB_RX_PKT_LEN(nbuf);
		memset(nbuf_copy->cb, 0x0, sizeof(nbuf_copy->cb));

		/* Set cb->ftype to intrabss FWD */
		qdf_nbuf_set_tx_ftype(nbuf_copy, CB_FTYPE_INTRABSS_FWD);
		if (dp_tx_send((struct cdp_soc_t *)soc,
			       ta_peer->vdev->vdev_id, nbuf_copy)) {
			DP_STATS_INC_PKT(ta_peer, rx.intra_bss.fail, 1, len);
			tid_stats->fail_cnt[INTRABSS_DROP]++;
			qdf_nbuf_free(nbuf_copy);
		} else {
			DP_STATS_INC_PKT(ta_peer, rx.intra_bss.pkts, 1, len);
			tid_stats->intrabss_cnt++;
		}
	}

end:
	/* return false as we have to still send the original pkt
	 * up the stack
	 */
	return false;
}

#ifdef MESH_MODE_SUPPORT

/**
 * dp_rx_fill_mesh_stats() - Fills the mesh per packet receive stats
 *
 * @vdev: DP Virtual device handle
 * @nbuf: Buffer pointer
 * @rx_tlv_hdr: start of rx tlv header
 * @peer: pointer to peer
 *
 * This function allocated memory for mesh receive stats and fill the
 * required stats. Stores the memory address in skb cb.
 *
 * Return: void
 */

void dp_rx_fill_mesh_stats(struct dp_vdev *vdev, qdf_nbuf_t nbuf,
				uint8_t *rx_tlv_hdr, struct dp_peer *peer)
{
	struct mesh_recv_hdr_s *rx_info = NULL;
	uint32_t pkt_type;
	uint32_t nss;
	uint32_t rate_mcs;
	uint32_t bw;
	uint8_t primary_chan_num;
	uint32_t center_chan_freq;
	struct dp_soc *soc;

	/* fill recv mesh stats */
	rx_info = qdf_mem_malloc(sizeof(struct mesh_recv_hdr_s));

	/* upper layers are resposible to free this memory */

	if (!rx_info) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			"Memory allocation failed for mesh rx stats");
		DP_STATS_INC(vdev->pdev, mesh_mem_alloc, 1);
		return;
	}

	rx_info->rs_flags = MESH_RXHDR_VER1;
	if (qdf_nbuf_is_rx_chfrag_start(nbuf))
		rx_info->rs_flags |= MESH_RX_FIRST_MSDU;

	if (qdf_nbuf_is_rx_chfrag_end(nbuf))
		rx_info->rs_flags |= MESH_RX_LAST_MSDU;

	if (hal_rx_attn_msdu_get_is_decrypted(rx_tlv_hdr)) {
		rx_info->rs_flags |= MESH_RX_DECRYPTED;
		rx_info->rs_keyix = hal_rx_msdu_get_keyid(rx_tlv_hdr);
		if (vdev->osif_get_key)
			vdev->osif_get_key(vdev->osif_vdev,
					&rx_info->rs_decryptkey[0],
					&peer->mac_addr.raw[0],
					rx_info->rs_keyix);
	}

	rx_info->rs_rssi = peer->stats.rx.rssi;

	soc = vdev->pdev->soc;
	primary_chan_num = hal_rx_msdu_start_get_freq(rx_tlv_hdr);
	center_chan_freq = hal_rx_msdu_start_get_freq(rx_tlv_hdr) >> 16;

	if (soc->cdp_soc.ol_ops && soc->cdp_soc.ol_ops->freq_to_band) {
		rx_info->rs_band = soc->cdp_soc.ol_ops->freq_to_band(
							soc->ctrl_psoc,
							vdev->pdev->pdev_id,
							center_chan_freq);
	}
	rx_info->rs_channel = primary_chan_num;
	pkt_type = hal_rx_msdu_start_get_pkt_type(rx_tlv_hdr);
	rate_mcs = hal_rx_msdu_start_rate_mcs_get(rx_tlv_hdr);
	bw = hal_rx_msdu_start_bw_get(rx_tlv_hdr);
	nss = hal_rx_msdu_start_nss_get(vdev->pdev->soc->hal_soc, rx_tlv_hdr);
	rx_info->rs_ratephy1 = rate_mcs | (nss << 0x8) | (pkt_type << 16) |
				(bw << 24);

	qdf_nbuf_set_rx_fctx_type(nbuf, (void *)rx_info, CB_FTYPE_MESH_RX_INFO);

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO_MED,
		FL("Mesh rx stats: flags %x, rssi %x, chn %x, rate %x, kix %x"),
						rx_info->rs_flags,
						rx_info->rs_rssi,
						rx_info->rs_channel,
						rx_info->rs_ratephy1,
						rx_info->rs_keyix);

}

/**
 * dp_rx_filter_mesh_packets() - Filters mesh unwanted packets
 *
 * @vdev: DP Virtual device handle
 * @nbuf: Buffer pointer
 * @rx_tlv_hdr: start of rx tlv header
 *
 * This checks if the received packet is matching any filter out
 * catogery and and drop the packet if it matches.
 *
 * Return: status(0 indicates drop, 1 indicate to no drop)
 */

QDF_STATUS dp_rx_filter_mesh_packets(struct dp_vdev *vdev, qdf_nbuf_t nbuf,
					uint8_t *rx_tlv_hdr)
{
	union dp_align_mac_addr mac_addr;
	struct dp_soc *soc = vdev->pdev->soc;

	if (qdf_unlikely(vdev->mesh_rx_filter)) {
		if (vdev->mesh_rx_filter & MESH_FILTER_OUT_FROMDS)
			if (hal_rx_mpdu_get_fr_ds(soc->hal_soc,
						  rx_tlv_hdr))
				return  QDF_STATUS_SUCCESS;

		if (vdev->mesh_rx_filter & MESH_FILTER_OUT_TODS)
			if (hal_rx_mpdu_get_to_ds(soc->hal_soc,
						  rx_tlv_hdr))
				return  QDF_STATUS_SUCCESS;

		if (vdev->mesh_rx_filter & MESH_FILTER_OUT_NODS)
			if (!hal_rx_mpdu_get_fr_ds(soc->hal_soc,
						   rx_tlv_hdr) &&
			    !hal_rx_mpdu_get_to_ds(soc->hal_soc,
						   rx_tlv_hdr))
				return  QDF_STATUS_SUCCESS;

		if (vdev->mesh_rx_filter & MESH_FILTER_OUT_RA) {
			if (hal_rx_mpdu_get_addr1(soc->hal_soc,
						  rx_tlv_hdr,
					&mac_addr.raw[0]))
				return QDF_STATUS_E_FAILURE;

			if (!qdf_mem_cmp(&mac_addr.raw[0],
					&vdev->mac_addr.raw[0],
					QDF_MAC_ADDR_SIZE))
				return  QDF_STATUS_SUCCESS;
		}

		if (vdev->mesh_rx_filter & MESH_FILTER_OUT_TA) {
			if (hal_rx_mpdu_get_addr2(soc->hal_soc,
						  rx_tlv_hdr,
						  &mac_addr.raw[0]))
				return QDF_STATUS_E_FAILURE;

			if (!qdf_mem_cmp(&mac_addr.raw[0],
					&vdev->mac_addr.raw[0],
					QDF_MAC_ADDR_SIZE))
				return  QDF_STATUS_SUCCESS;
		}
	}

	return QDF_STATUS_E_FAILURE;
}

#else
void dp_rx_fill_mesh_stats(struct dp_vdev *vdev, qdf_nbuf_t nbuf,
				uint8_t *rx_tlv_hdr, struct dp_peer *peer)
{
}

QDF_STATUS dp_rx_filter_mesh_packets(struct dp_vdev *vdev, qdf_nbuf_t nbuf,
					uint8_t *rx_tlv_hdr)
{
	return QDF_STATUS_E_FAILURE;
}

#endif

#ifdef FEATURE_NAC_RSSI
/**
 * dp_rx_nac_filter(): Function to perform filtering of non-associated
 * clients
 * @pdev: DP pdev handle
 * @rx_pkt_hdr: Rx packet Header
 *
 * return: dp_vdev*
 */
static
struct dp_vdev *dp_rx_nac_filter(struct dp_pdev *pdev,
		uint8_t *rx_pkt_hdr)
{
	struct ieee80211_frame *wh;
	struct dp_neighbour_peer *peer = NULL;

	wh = (struct ieee80211_frame *)rx_pkt_hdr;

	if ((wh->i_fc[1] & IEEE80211_FC1_DIR_MASK) != IEEE80211_FC1_DIR_TODS)
		return NULL;

	qdf_spin_lock_bh(&pdev->neighbour_peer_mutex);
	TAILQ_FOREACH(peer, &pdev->neighbour_peers_list,
				neighbour_peer_list_elem) {
		if (qdf_mem_cmp(&peer->neighbour_peers_macaddr.raw[0],
				wh->i_addr2, QDF_MAC_ADDR_SIZE) == 0) {
			QDF_TRACE(
				QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
				FL("NAC configuration matched for mac-%2x:%2x:%2x:%2x:%2x:%2x"),
				peer->neighbour_peers_macaddr.raw[0],
				peer->neighbour_peers_macaddr.raw[1],
				peer->neighbour_peers_macaddr.raw[2],
				peer->neighbour_peers_macaddr.raw[3],
				peer->neighbour_peers_macaddr.raw[4],
				peer->neighbour_peers_macaddr.raw[5]);

				qdf_spin_unlock_bh(&pdev->neighbour_peer_mutex);

			return pdev->monitor_vdev;
		}
	}
	qdf_spin_unlock_bh(&pdev->neighbour_peer_mutex);

	return NULL;
}

/**
 * dp_rx_process_invalid_peer(): Function to pass invalid peer list to umac
 * @soc: DP SOC handle
 * @mpdu: mpdu for which peer is invalid
 * @mac_id: mac_id which is one of 3 mac_ids(Assuming mac_id and
 * pool_id has same mapping)
 *
 * return: integer type
 */
uint8_t dp_rx_process_invalid_peer(struct dp_soc *soc, qdf_nbuf_t mpdu,
				   uint8_t mac_id)
{
	struct dp_invalid_peer_msg msg;
	struct dp_vdev *vdev = NULL;
	struct dp_pdev *pdev = NULL;
	struct ieee80211_frame *wh;
	qdf_nbuf_t curr_nbuf, next_nbuf;
	uint8_t *rx_tlv_hdr = qdf_nbuf_data(mpdu);
	uint8_t *rx_pkt_hdr = hal_rx_pkt_hdr_get(rx_tlv_hdr);

	rx_pkt_hdr = hal_rx_pkt_hdr_get(rx_tlv_hdr);

	if (!HAL_IS_DECAP_FORMAT_RAW(soc->hal_soc, rx_tlv_hdr)) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			  "Drop decapped frames");
		goto free;
	}

	wh = (struct ieee80211_frame *)rx_pkt_hdr;

	if (!DP_FRAME_IS_DATA(wh)) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			  "NAWDS valid only for data frames");
		goto free;
	}

	if (qdf_nbuf_len(mpdu) < sizeof(struct ieee80211_frame)) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			"Invalid nbuf length");
		goto free;
	}

	pdev = dp_get_pdev_for_lmac_id(soc, mac_id);

	if (!pdev || qdf_unlikely(pdev->is_pdev_down)) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			  "PDEV %s", !pdev ? "not found" : "down");
		goto free;
	}

	if (pdev->filter_neighbour_peers) {
		/* Next Hop scenario not yet handle */
		vdev = dp_rx_nac_filter(pdev, rx_pkt_hdr);
		if (vdev) {
			dp_rx_mon_deliver(soc, pdev->pdev_id,
					  pdev->invalid_peer_head_msdu,
					  pdev->invalid_peer_tail_msdu);

			pdev->invalid_peer_head_msdu = NULL;
			pdev->invalid_peer_tail_msdu = NULL;

			return 0;
		}
	}

	TAILQ_FOREACH(vdev, &pdev->vdev_list, vdev_list_elem) {

		if (qdf_mem_cmp(wh->i_addr1, vdev->mac_addr.raw,
				QDF_MAC_ADDR_SIZE) == 0) {
			goto out;
		}
	}

	if (!vdev) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			"VDEV not found");
		goto free;
	}

out:
	msg.wh = wh;
	qdf_nbuf_pull_head(mpdu, RX_PKT_TLVS_LEN);
	msg.nbuf = mpdu;
	msg.vdev_id = vdev->vdev_id;
	if (pdev->soc->cdp_soc.ol_ops->rx_invalid_peer)
		pdev->soc->cdp_soc.ol_ops->rx_invalid_peer(
				(struct cdp_ctrl_objmgr_psoc *)soc->ctrl_psoc,
				pdev->pdev_id, &msg);

free:
	/* Drop and free packet */
	curr_nbuf = mpdu;
	while (curr_nbuf) {
		next_nbuf = qdf_nbuf_next(curr_nbuf);
		qdf_nbuf_free(curr_nbuf);
		curr_nbuf = next_nbuf;
	}

	return 0;
}

/**
 * dp_rx_process_invalid_peer_wrapper(): Function to wrap invalid peer handler
 * @soc: DP SOC handle
 * @mpdu: mpdu for which peer is invalid
 * @mpdu_done: if an mpdu is completed
 * @mac_id: mac_id which is one of 3 mac_ids(Assuming mac_id and
 * pool_id has same mapping)
 *
 * return: integer type
 */
void dp_rx_process_invalid_peer_wrapper(struct dp_soc *soc,
					qdf_nbuf_t mpdu, bool mpdu_done,
					uint8_t mac_id)
{
	/* Only trigger the process when mpdu is completed */
	if (mpdu_done)
		dp_rx_process_invalid_peer(soc, mpdu, mac_id);
}
#else
uint8_t dp_rx_process_invalid_peer(struct dp_soc *soc, qdf_nbuf_t mpdu,
				   uint8_t mac_id)
{
	qdf_nbuf_t curr_nbuf, next_nbuf;
	struct dp_pdev *pdev;
	struct dp_vdev *vdev = NULL;
	struct ieee80211_frame *wh;
	uint8_t *rx_tlv_hdr = qdf_nbuf_data(mpdu);
	uint8_t *rx_pkt_hdr = hal_rx_pkt_hdr_get(rx_tlv_hdr);

	wh = (struct ieee80211_frame *)rx_pkt_hdr;

	if (!DP_FRAME_IS_DATA(wh)) {
		QDF_TRACE_ERROR_RL(QDF_MODULE_ID_DP,
				   "only for data frames");
		goto free;
	}

	if (qdf_nbuf_len(mpdu) < sizeof(struct ieee80211_frame)) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			  "Invalid nbuf length");
		goto free;
	}

	pdev = dp_get_pdev_for_lmac_id(soc, mac_id);
	if (!pdev) {
		QDF_TRACE(QDF_MODULE_ID_DP,
			  QDF_TRACE_LEVEL_ERROR,
			  "PDEV not found");
		goto free;
	}

	qdf_spin_lock_bh(&pdev->vdev_list_lock);
	DP_PDEV_ITERATE_VDEV_LIST(pdev, vdev) {
		if (qdf_mem_cmp(wh->i_addr1, vdev->mac_addr.raw,
				QDF_MAC_ADDR_SIZE) == 0) {
			qdf_spin_unlock_bh(&pdev->vdev_list_lock);
			goto out;
		}
	}
	qdf_spin_unlock_bh(&pdev->vdev_list_lock);

	if (!vdev) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			  "VDEV not found");
		goto free;
	}

out:
	if (soc->cdp_soc.ol_ops->rx_invalid_peer)
		soc->cdp_soc.ol_ops->rx_invalid_peer(vdev->vdev_id, wh);
free:
	/* reset the head and tail pointers */
	pdev = dp_get_pdev_for_lmac_id(soc, mac_id);
	if (pdev) {
		pdev->invalid_peer_head_msdu = NULL;
		pdev->invalid_peer_tail_msdu = NULL;
	}

	/* Drop and free packet */
	curr_nbuf = mpdu;
	while (curr_nbuf) {
		next_nbuf = qdf_nbuf_next(curr_nbuf);
		qdf_nbuf_free(curr_nbuf);
		curr_nbuf = next_nbuf;
	}

	/* Reset the head and tail pointers */
	pdev = dp_get_pdev_for_lmac_id(soc, mac_id);
	if (pdev) {
		pdev->invalid_peer_head_msdu = NULL;
		pdev->invalid_peer_tail_msdu = NULL;
	}

	return 0;
}

void dp_rx_process_invalid_peer_wrapper(struct dp_soc *soc,
					qdf_nbuf_t mpdu, bool mpdu_done,
					uint8_t mac_id)
{
	/* Process the nbuf */
	dp_rx_process_invalid_peer(soc, mpdu, mac_id);
}
#endif

#ifdef RECEIVE_OFFLOAD
/**
 * dp_rx_print_offload_info() - Print offload info from RX TLV
 * @soc: dp soc handle
 * @rx_tlv: RX TLV for which offload information is to be printed
 *
 * Return: None
 */
static void dp_rx_print_offload_info(struct dp_soc *soc, uint8_t *rx_tlv)
{
	dp_verbose_debug("----------------------RX DESC LRO/GRO----------------------");
	dp_verbose_debug("lro_eligible 0x%x", HAL_RX_TLV_GET_LRO_ELIGIBLE(rx_tlv));
	dp_verbose_debug("pure_ack 0x%x", HAL_RX_TLV_GET_TCP_PURE_ACK(rx_tlv));
	dp_verbose_debug("chksum 0x%x", hal_rx_tlv_get_tcp_chksum(soc->hal_soc,
								  rx_tlv));
	dp_verbose_debug("TCP seq num 0x%x", HAL_RX_TLV_GET_TCP_SEQ(rx_tlv));
	dp_verbose_debug("TCP ack num 0x%x", HAL_RX_TLV_GET_TCP_ACK(rx_tlv));
	dp_verbose_debug("TCP window 0x%x", HAL_RX_TLV_GET_TCP_WIN(rx_tlv));
	dp_verbose_debug("TCP protocol 0x%x", HAL_RX_TLV_GET_TCP_PROTO(rx_tlv));
	dp_verbose_debug("TCP offset 0x%x", HAL_RX_TLV_GET_TCP_OFFSET(rx_tlv));
	dp_verbose_debug("toeplitz 0x%x", HAL_RX_TLV_GET_FLOW_ID_TOEPLITZ(rx_tlv));
	dp_verbose_debug("---------------------------------------------------------");
}

/**
 * dp_rx_fill_gro_info() - Fill GRO info from RX TLV into skb->cb
 * @soc: DP SOC handle
 * @rx_tlv: RX TLV received for the msdu
 * @msdu: msdu for which GRO info needs to be filled
 * @rx_ol_pkt_cnt: counter to be incremented for GRO eligible packets
 *
 * Return: None
 */
static
void dp_rx_fill_gro_info(struct dp_soc *soc, uint8_t *rx_tlv,
			 qdf_nbuf_t msdu, uint32_t *rx_ol_pkt_cnt)
{
	if (!wlan_cfg_is_gro_enabled(soc->wlan_cfg_ctx))
		return;

	/* Filling up RX offload info only for TCP packets */
	if (!HAL_RX_TLV_GET_TCP_PROTO(rx_tlv))
		return;

	*rx_ol_pkt_cnt = *rx_ol_pkt_cnt + 1;

	QDF_NBUF_CB_RX_LRO_ELIGIBLE(msdu) =
		 HAL_RX_TLV_GET_LRO_ELIGIBLE(rx_tlv);
	QDF_NBUF_CB_RX_TCP_PURE_ACK(msdu) =
			HAL_RX_TLV_GET_TCP_PURE_ACK(rx_tlv);
	QDF_NBUF_CB_RX_TCP_CHKSUM(msdu) =
			hal_rx_tlv_get_tcp_chksum(soc->hal_soc,
						  rx_tlv);
	QDF_NBUF_CB_RX_TCP_SEQ_NUM(msdu) =
			 HAL_RX_TLV_GET_TCP_SEQ(rx_tlv);
	QDF_NBUF_CB_RX_TCP_ACK_NUM(msdu) =
			 HAL_RX_TLV_GET_TCP_ACK(rx_tlv);
	QDF_NBUF_CB_RX_TCP_WIN(msdu) =
			 HAL_RX_TLV_GET_TCP_WIN(rx_tlv);
	QDF_NBUF_CB_RX_TCP_PROTO(msdu) =
			 HAL_RX_TLV_GET_TCP_PROTO(rx_tlv);
	QDF_NBUF_CB_RX_IPV6_PROTO(msdu) =
			 HAL_RX_TLV_GET_IPV6(rx_tlv);
	QDF_NBUF_CB_RX_TCP_OFFSET(msdu) =
			 HAL_RX_TLV_GET_TCP_OFFSET(rx_tlv);
	QDF_NBUF_CB_RX_FLOW_ID(msdu) =
			 HAL_RX_TLV_GET_FLOW_ID_TOEPLITZ(rx_tlv);

	dp_rx_print_offload_info(soc, rx_tlv);
}
#else
static void dp_rx_fill_gro_info(struct dp_soc *soc, uint8_t *rx_tlv,
				qdf_nbuf_t msdu, uint32_t *rx_ol_pkt_cnt)
{
}
#endif /* RECEIVE_OFFLOAD */

/**
 * dp_rx_adjust_nbuf_len() - set appropriate msdu length in nbuf.
 *
 * @nbuf: pointer to msdu.
 * @mpdu_len: mpdu length
 *
 * Return: returns true if nbuf is last msdu of mpdu else retuns false.
 */
static inline bool dp_rx_adjust_nbuf_len(qdf_nbuf_t nbuf, uint16_t *mpdu_len)
{
	bool last_nbuf;

	if (*mpdu_len > (RX_DATA_BUFFER_SIZE - RX_PKT_TLVS_LEN)) {
		qdf_nbuf_set_pktlen(nbuf, RX_DATA_BUFFER_SIZE);
		last_nbuf = false;
	} else {
		qdf_nbuf_set_pktlen(nbuf, (*mpdu_len + RX_PKT_TLVS_LEN));
		last_nbuf = true;
	}

	*mpdu_len -= (RX_DATA_BUFFER_SIZE - RX_PKT_TLVS_LEN);

	return last_nbuf;
}

/**
 * dp_rx_sg_create() - create a frag_list for MSDUs which are spread across
 *		     multiple nbufs.
 * @nbuf: pointer to the first msdu of an amsdu.
 *
 * This function implements the creation of RX frag_list for cases
 * where an MSDU is spread across multiple nbufs.
 *
 * Return: returns the head nbuf which contains complete frag_list.
 */
qdf_nbuf_t dp_rx_sg_create(qdf_nbuf_t nbuf)
{
	qdf_nbuf_t parent, frag_list, next = NULL;
	uint16_t frag_list_len = 0;
	uint16_t mpdu_len;
	bool last_nbuf;

	/*
	 * Use msdu len got from REO entry descriptor instead since
	 * there is case the RX PKT TLV is corrupted while msdu_len
	 * from REO descriptor is right for non-raw RX scatter msdu.
	 */
	mpdu_len = QDF_NBUF_CB_RX_PKT_LEN(nbuf);
	/*
	 * this is a case where the complete msdu fits in one single nbuf.
	 * in this case HW sets both start and end bit and we only need to
	 * reset these bits for RAW mode simulator to decap the pkt
	 */
	if (qdf_nbuf_is_rx_chfrag_start(nbuf) &&
					qdf_nbuf_is_rx_chfrag_end(nbuf)) {
		qdf_nbuf_set_pktlen(nbuf, mpdu_len + RX_PKT_TLVS_LEN);
		qdf_nbuf_pull_head(nbuf, RX_PKT_TLVS_LEN);
		return nbuf;
	}

	/*
	 * This is a case where we have multiple msdus (A-MSDU) spread across
	 * multiple nbufs. here we create a fraglist out of these nbufs.
	 *
	 * the moment we encounter a nbuf with continuation bit set we
	 * know for sure we have an MSDU which is spread across multiple
	 * nbufs. We loop through and reap nbufs till we reach last nbuf.
	 */
	parent = nbuf;
	frag_list = nbuf->next;
	nbuf = nbuf->next;

	/*
	 * set the start bit in the first nbuf we encounter with continuation
	 * bit set. This has the proper mpdu length set as it is the first
	 * msdu of the mpdu. this becomes the parent nbuf and the subsequent
	 * nbufs will form the frag_list of the parent nbuf.
	 */
	qdf_nbuf_set_rx_chfrag_start(parent, 1);
	last_nbuf = dp_rx_adjust_nbuf_len(parent, &mpdu_len);

	/*
	 * this is where we set the length of the fragments which are
	 * associated to the parent nbuf. We iterate through the frag_list
	 * till we hit the last_nbuf of the list.
	 */
	do {
		last_nbuf = dp_rx_adjust_nbuf_len(nbuf, &mpdu_len);
		qdf_nbuf_pull_head(nbuf, RX_PKT_TLVS_LEN);
		frag_list_len += qdf_nbuf_len(nbuf);

		if (last_nbuf) {
			next = nbuf->next;
			nbuf->next = NULL;
			break;
		}

		nbuf = nbuf->next;
	} while (!last_nbuf);

	qdf_nbuf_set_rx_chfrag_start(nbuf, 0);
	qdf_nbuf_append_ext_list(parent, frag_list, frag_list_len);
	parent->next = next;

	qdf_nbuf_pull_head(parent, RX_PKT_TLVS_LEN);
	return parent;
}

#ifdef QCA_PEER_EXT_STATS
/*
 * dp_rx_compute_tid_delay - Computer per TID delay stats
 * @peer: DP soc context
 * @nbuf: NBuffer
 *
 * Return: Void
 */
void dp_rx_compute_tid_delay(struct cdp_delay_tid_stats *stats,
			     qdf_nbuf_t nbuf)
{
	struct cdp_delay_rx_stats  *rx_delay = &stats->rx_delay;
	uint32_t to_stack = qdf_nbuf_get_timedelta_ms(nbuf);

	dp_hist_update_stats(&rx_delay->to_stack_delay, to_stack);
}
#endif /* QCA_PEER_EXT_STATS */

/**
 * dp_rx_compute_delay() - Compute and fill in all timestamps
 *				to pass in correct fields
 *
 * @vdev: pdev handle
 * @tx_desc: tx descriptor
 * @tid: tid value
 * Return: none
 */
void dp_rx_compute_delay(struct dp_vdev *vdev, qdf_nbuf_t nbuf)
{
	uint8_t ring_id = QDF_NBUF_CB_RX_CTX_ID(nbuf);
	int64_t current_ts = qdf_ktime_to_ms(qdf_ktime_get());
	uint32_t to_stack = qdf_nbuf_get_timedelta_ms(nbuf);
	uint8_t tid = qdf_nbuf_get_tid_val(nbuf);
	uint32_t interframe_delay =
		(uint32_t)(current_ts - vdev->prev_rx_deliver_tstamp);

	dp_update_delay_stats(vdev->pdev, to_stack, tid,
			      CDP_DELAY_STATS_REAP_STACK, ring_id);
	/*
	 * Update interframe delay stats calculated at deliver_data_ol point.
	 * Value of vdev->prev_rx_deliver_tstamp will be 0 for 1st frame, so
	 * interframe delay will not be calculate correctly for 1st frame.
	 * On the other side, this will help in avoiding extra per packet check
	 * of vdev->prev_rx_deliver_tstamp.
	 */
	dp_update_delay_stats(vdev->pdev, interframe_delay, tid,
			      CDP_DELAY_STATS_RX_INTERFRAME, ring_id);
	vdev->prev_rx_deliver_tstamp = current_ts;
}

/**
 * dp_rx_drop_nbuf_list() - drop an nbuf list
 * @pdev: dp pdev reference
 * @buf_list: buffer list to be dropepd
 *
 * Return: int (number of bufs dropped)
 */
static inline int dp_rx_drop_nbuf_list(struct dp_pdev *pdev,
				       qdf_nbuf_t buf_list)
{
	struct cdp_tid_rx_stats *stats = NULL;
	uint8_t tid = 0, ring_id = 0;
	int num_dropped = 0;
	qdf_nbuf_t buf, next_buf;

	buf = buf_list;
	while (buf) {
		ring_id = QDF_NBUF_CB_RX_CTX_ID(buf);
		next_buf = qdf_nbuf_queue_next(buf);
		tid = qdf_nbuf_get_tid_val(buf);
		if (qdf_likely(pdev)) {
			stats = &pdev->stats.tid_stats.tid_rx_stats[ring_id][tid];
			stats->fail_cnt[INVALID_PEER_VDEV]++;
			stats->delivered_to_stack--;
		}
		qdf_nbuf_free(buf);
		buf = next_buf;
		num_dropped++;
	}

	return num_dropped;
}

#ifdef QCA_SUPPORT_WDS_EXTENDED
/**
 * dp_rx_wds_ext() - Make different lists for 4-address and 3-address frames
 * @nbuf_head: skb list head
 * @vdev: vdev
 * @peer: peer
 * @peer_id: peer id of new received frame
 * @vdev_id: vdev_id of new received frame
 *
 * Return: true if peer_ids are different.
 */
static inline bool
dp_rx_is_list_ready(qdf_nbuf_t nbuf_head,
		    struct dp_vdev *vdev,
		    struct dp_peer *peer,
		    uint16_t peer_id,
		    uint8_t vdev_id)
{
	if (nbuf_head && peer && (peer->peer_id != peer_id))
		return true;

	return false;
}

/**
 * dp_rx_deliver_to_stack_ext() - Deliver to netdev per sta
 * @soc: core txrx main context
 * @vdev: vdev
 * @peer: peer
 * @nbuf_head: skb list head
 *
 * Return: true if packet is delivered to netdev per STA.
 */
static inline bool
dp_rx_deliver_to_stack_ext(struct dp_soc *soc, struct dp_vdev *vdev,
			   struct dp_peer *peer, qdf_nbuf_t nbuf_head)
{
	/*
	 * When extended WDS is disabled, frames are sent to AP netdevice.
	 */
	if (qdf_likely(!vdev->wds_ext_enabled))
		return false;

	/*
	 * There can be 2 cases:
	 * 1. Send frame to parent netdev if its not for netdev per STA
	 * 2. If frame is meant for netdev per STA:
	 *    a. Send frame to appropriate netdev using registered fp.
	 *    b. If fp is NULL, drop the frames.
	 */
	if (!peer->wds_ext.init)
		return false;

	if (peer->osif_rx)
		peer->osif_rx(peer->wds_ext.osif_peer, nbuf_head);
	else
		dp_rx_drop_nbuf_list(vdev->pdev, nbuf_head);

	return true;
}

#else
static inline bool
dp_rx_is_list_ready(qdf_nbuf_t nbuf_head,
		    struct dp_vdev *vdev,
		    struct dp_peer *peer,
		    uint16_t peer_id,
		    uint8_t vdev_id)
{
	if (nbuf_head && vdev && (vdev->vdev_id != vdev_id))
		return true;

	return false;
}

static inline bool
dp_rx_deliver_to_stack_ext(struct dp_soc *soc, struct dp_vdev *vdev,
			   struct dp_peer *peer, qdf_nbuf_t nbuf_head)
{
	return false;
}
#endif

#ifdef PEER_CACHE_RX_PKTS
/**
 * dp_rx_flush_rx_cached() - flush cached rx frames
 * @peer: peer
 * @drop: flag to drop frames or forward to net stack
 *
 * Return: None
 */
void dp_rx_flush_rx_cached(struct dp_peer *peer, bool drop)
{
	struct dp_peer_cached_bufq *bufqi;
	struct dp_rx_cached_buf *cache_buf = NULL;
	ol_txrx_rx_fp data_rx = NULL;
	int num_buff_elem;
	QDF_STATUS status;

	if (qdf_atomic_inc_return(&peer->flush_in_progress) > 1) {
		qdf_atomic_dec(&peer->flush_in_progress);
		return;
	}

	qdf_spin_lock_bh(&peer->peer_info_lock);
	if (peer->state >= OL_TXRX_PEER_STATE_CONN && peer->vdev->osif_rx)
		data_rx = peer->vdev->osif_rx;
	else
		drop = true;
	qdf_spin_unlock_bh(&peer->peer_info_lock);

	bufqi = &peer->bufq_info;

	qdf_spin_lock_bh(&bufqi->bufq_lock);
	qdf_list_remove_front(&bufqi->cached_bufq,
			      (qdf_list_node_t **)&cache_buf);
	while (cache_buf) {
		num_buff_elem = QDF_NBUF_CB_RX_NUM_ELEMENTS_IN_LIST(
								cache_buf->buf);
		bufqi->entries -= num_buff_elem;
		qdf_spin_unlock_bh(&bufqi->bufq_lock);
		if (drop) {
			bufqi->dropped = dp_rx_drop_nbuf_list(peer->vdev->pdev,
							      cache_buf->buf);
		} else {
			/* Flush the cached frames to OSIF DEV */
			status = data_rx(peer->vdev->osif_vdev, cache_buf->buf);
			if (status != QDF_STATUS_SUCCESS)
				bufqi->dropped = dp_rx_drop_nbuf_list(
							peer->vdev->pdev,
							cache_buf->buf);
		}
		qdf_mem_free(cache_buf);
		cache_buf = NULL;
		qdf_spin_lock_bh(&bufqi->bufq_lock);
		qdf_list_remove_front(&bufqi->cached_bufq,
				      (qdf_list_node_t **)&cache_buf);
	}
	qdf_spin_unlock_bh(&bufqi->bufq_lock);
	qdf_atomic_dec(&peer->flush_in_progress);
}

/**
 * dp_rx_enqueue_rx() - cache rx frames
 * @peer: peer
 * @rx_buf_list: cache buffer list
 *
 * Return: None
 */
static QDF_STATUS
dp_rx_enqueue_rx(struct dp_peer *peer, qdf_nbuf_t rx_buf_list)
{
	struct dp_rx_cached_buf *cache_buf;
	struct dp_peer_cached_bufq *bufqi = &peer->bufq_info;
	int num_buff_elem;

	dp_debug_rl("bufq->curr %d bufq->drops %d", bufqi->entries,
		    bufqi->dropped);
	if (!peer->valid) {
		bufqi->dropped = dp_rx_drop_nbuf_list(peer->vdev->pdev,
						      rx_buf_list);
		return QDF_STATUS_E_INVAL;
	}

	qdf_spin_lock_bh(&bufqi->bufq_lock);
	if (bufqi->entries >= bufqi->thresh) {
		bufqi->dropped = dp_rx_drop_nbuf_list(peer->vdev->pdev,
						      rx_buf_list);
		qdf_spin_unlock_bh(&bufqi->bufq_lock);
		return QDF_STATUS_E_RESOURCES;
	}
	qdf_spin_unlock_bh(&bufqi->bufq_lock);

	num_buff_elem = QDF_NBUF_CB_RX_NUM_ELEMENTS_IN_LIST(rx_buf_list);

	cache_buf = qdf_mem_malloc_atomic(sizeof(*cache_buf));
	if (!cache_buf) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "Failed to allocate buf to cache rx frames");
		bufqi->dropped = dp_rx_drop_nbuf_list(peer->vdev->pdev,
						      rx_buf_list);
		return QDF_STATUS_E_NOMEM;
	}

	cache_buf->buf = rx_buf_list;

	qdf_spin_lock_bh(&bufqi->bufq_lock);
	qdf_list_insert_back(&bufqi->cached_bufq,
			     &cache_buf->node);
	bufqi->entries += num_buff_elem;
	qdf_spin_unlock_bh(&bufqi->bufq_lock);

	return QDF_STATUS_SUCCESS;
}

static inline
bool dp_rx_is_peer_cache_bufq_supported(void)
{
	return true;
}
#else
static inline
bool dp_rx_is_peer_cache_bufq_supported(void)
{
	return false;
}

static inline QDF_STATUS
dp_rx_enqueue_rx(struct dp_peer *peer, qdf_nbuf_t rx_buf_list)
{
	return QDF_STATUS_SUCCESS;
}
#endif

#ifndef DELIVERY_TO_STACK_STATUS_CHECK
/**
 * dp_rx_check_delivery_to_stack() - Deliver pkts to network
 * using the appropriate call back functions.
 * @soc: soc
 * @vdev: vdev
 * @peer: peer
 * @nbuf_head: skb list head
 * @nbuf_tail: skb list tail
 *
 * Return: None
 */
static void dp_rx_check_delivery_to_stack(struct dp_soc *soc,
					  struct dp_vdev *vdev,
					  struct dp_peer *peer,
					  qdf_nbuf_t nbuf_head)
{
	if (qdf_unlikely(dp_rx_deliver_to_stack_ext(soc, vdev,
						    peer, nbuf_head)))
		return;

	/* Function pointer initialized only when FISA is enabled */
	if (vdev->osif_fisa_rx)
		/* on failure send it via regular path */
		vdev->osif_fisa_rx(soc, vdev, nbuf_head);
	else
		vdev->osif_rx(vdev->osif_vdev, nbuf_head);
}

#else
/**
 * dp_rx_check_delivery_to_stack() - Deliver pkts to network
 * using the appropriate call back functions.
 * @soc: soc
 * @vdev: vdev
 * @peer: peer
 * @nbuf_head: skb list head
 * @nbuf_tail: skb list tail
 *
 * Check the return status of the call back function and drop
 * the packets if the return status indicates a failure.
 *
 * Return: None
 */
static void dp_rx_check_delivery_to_stack(struct dp_soc *soc,
					  struct dp_vdev *vdev,
					  struct dp_peer *peer,
					  qdf_nbuf_t nbuf_head)
{
	int num_nbuf = 0;
	QDF_STATUS ret_val = QDF_STATUS_E_FAILURE;

	/* Function pointer initialized only when FISA is enabled */
	if (vdev->osif_fisa_rx)
		/* on failure send it via regular path */
		ret_val = vdev->osif_fisa_rx(soc, vdev, nbuf_head);
	else if (vdev->osif_rx)
		ret_val = vdev->osif_rx(vdev->osif_vdev, nbuf_head);

	if (!QDF_IS_STATUS_SUCCESS(ret_val)) {
		num_nbuf = dp_rx_drop_nbuf_list(vdev->pdev, nbuf_head);
		DP_STATS_INC(soc, rx.err.rejected, num_nbuf);
		if (peer)
			DP_STATS_DEC(peer, rx.to_stack.num, num_nbuf);
	}
}
#endif /* ifdef DELIVERY_TO_STACK_STATUS_CHECK */

void dp_rx_deliver_to_stack(struct dp_soc *soc,
			    struct dp_vdev *vdev,
			    struct dp_peer *peer,
			    qdf_nbuf_t nbuf_head,
			    qdf_nbuf_t nbuf_tail)
{
	int num_nbuf = 0;

	if (qdf_unlikely(!vdev || vdev->delete.pending)) {
		num_nbuf = dp_rx_drop_nbuf_list(NULL, nbuf_head);
		/*
		 * This is a special case where vdev is invalid,
		 * so we cannot know the pdev to which this packet
		 * belonged. Hence we update the soc rx error stats.
		 */
		DP_STATS_INC(soc, rx.err.invalid_vdev, num_nbuf);
		return;
	}

	/*
	 * highly unlikely to have a vdev without a registered rx
	 * callback function. if so let us free the nbuf_list.
	 */
	if (qdf_unlikely(!vdev->osif_rx)) {
		if (peer && dp_rx_is_peer_cache_bufq_supported()) {
			dp_rx_enqueue_rx(peer, nbuf_head);
		} else {
			num_nbuf = dp_rx_drop_nbuf_list(vdev->pdev,
							nbuf_head);
			DP_STATS_DEC(peer, rx.to_stack.num, num_nbuf);
		}
		return;
	}

	if (qdf_unlikely(vdev->rx_decap_type == htt_cmn_pkt_type_raw) ||
			(vdev->rx_decap_type == htt_cmn_pkt_type_native_wifi)) {
		vdev->osif_rsim_rx_decap(vdev->osif_vdev, &nbuf_head,
				&nbuf_tail, peer->mac_addr.raw);
	}

	dp_rx_check_delivery_to_stack(soc, vdev, peer, nbuf_head);
}

/**
 * dp_rx_cksum_offload() - set the nbuf checksum as defined by hardware.
 * @nbuf: pointer to the first msdu of an amsdu.
 * @rx_tlv_hdr: pointer to the start of RX TLV headers.
 *
 * The ipsumed field of the skb is set based on whether HW validated the
 * IP/TCP/UDP checksum.
 *
 * Return: void
 */
static inline void dp_rx_cksum_offload(struct dp_pdev *pdev,
				       qdf_nbuf_t nbuf,
				       uint8_t *rx_tlv_hdr)
{
	qdf_nbuf_rx_cksum_t cksum = {0};
	bool ip_csum_err = hal_rx_attn_ip_cksum_fail_get(rx_tlv_hdr);
	bool tcp_udp_csum_er = hal_rx_attn_tcp_udp_cksum_fail_get(rx_tlv_hdr);

	if (qdf_likely(!ip_csum_err && !tcp_udp_csum_er)) {
		cksum.l4_result = QDF_NBUF_RX_CKSUM_TCP_UDP_UNNECESSARY;
		qdf_nbuf_set_rx_cksum(nbuf, &cksum);
	} else {
		DP_STATS_INCC(pdev, err.ip_csum_err, 1, ip_csum_err);
		DP_STATS_INCC(pdev, err.tcp_udp_csum_err, 1, tcp_udp_csum_er);
	}
}

#ifdef VDEV_PEER_PROTOCOL_COUNT
#define dp_rx_msdu_stats_update_prot_cnts(vdev_hdl, nbuf, peer) \
{ \
	qdf_nbuf_t nbuf_local; \
	struct dp_peer *peer_local; \
	struct dp_vdev *vdev_local = vdev_hdl; \
	do { \
		if (qdf_likely(!((vdev_local)->peer_protocol_count_track))) \
			break; \
		nbuf_local = nbuf; \
		peer_local = peer; \
		if (qdf_unlikely(qdf_nbuf_is_frag((nbuf_local)))) \
			break; \
		else if (qdf_unlikely(qdf_nbuf_is_raw_frame((nbuf_local)))) \
			break; \
		dp_vdev_peer_stats_update_protocol_cnt((vdev_local), \
						       (nbuf_local), \
						       (peer_local), 0, 1); \
	} while (0); \
}
#else
#define dp_rx_msdu_stats_update_prot_cnts(vdev_hdl, nbuf, peer)
#endif

/**
 * dp_rx_msdu_stats_update() - update per msdu stats.
 * @soc: core txrx main context
 * @nbuf: pointer to the first msdu of an amsdu.
 * @rx_tlv_hdr: pointer to the start of RX TLV headers.
 * @peer: pointer to the peer object.
 * @ring_id: reo dest ring number on which pkt is reaped.
 * @tid_stats: per tid rx stats.
 *
 * update all the per msdu stats for that nbuf.
 * Return: void
 */
static void dp_rx_msdu_stats_update(struct dp_soc *soc,
				    qdf_nbuf_t nbuf,
				    uint8_t *rx_tlv_hdr,
				    struct dp_peer *peer,
				    uint8_t ring_id,
				    struct cdp_tid_rx_stats *tid_stats)
{
	bool is_ampdu, is_not_amsdu;
	uint32_t sgi, mcs, tid, nss, bw, reception_type, pkt_type;
	struct dp_vdev *vdev = peer->vdev;
	qdf_ether_header_t *eh;
	uint16_t msdu_len = QDF_NBUF_CB_RX_PKT_LEN(nbuf);

	dp_rx_msdu_stats_update_prot_cnts(vdev, nbuf, peer);
	is_not_amsdu = qdf_nbuf_is_rx_chfrag_start(nbuf) &
			qdf_nbuf_is_rx_chfrag_end(nbuf);

	DP_STATS_INC_PKT(peer, rx.rcvd_reo[ring_id], 1, msdu_len);
	DP_STATS_INCC(peer, rx.non_amsdu_cnt, 1, is_not_amsdu);
	DP_STATS_INCC(peer, rx.amsdu_cnt, 1, !is_not_amsdu);
	DP_STATS_INCC(peer, rx.rx_retries, 1, qdf_nbuf_is_rx_retry_flag(nbuf));

	tid_stats->msdu_cnt++;
	if (qdf_unlikely(qdf_nbuf_is_da_mcbc(nbuf) &&
			 (vdev->rx_decap_type == htt_cmn_pkt_type_ethernet))) {
		eh = (qdf_ether_header_t *)qdf_nbuf_data(nbuf);
		DP_STATS_INC_PKT(peer, rx.multicast, 1, msdu_len);
		tid_stats->mcast_msdu_cnt++;
		if (QDF_IS_ADDR_BROADCAST(eh->ether_dhost)) {
			DP_STATS_INC_PKT(peer, rx.bcast, 1, msdu_len);
			tid_stats->bcast_msdu_cnt++;
		}
	}

	/*
	 * currently we can return from here as we have similar stats
	 * updated at per ppdu level instead of msdu level
	 */
	if (!soc->process_rx_status)
		return;

	is_ampdu = hal_rx_mpdu_info_ampdu_flag_get(rx_tlv_hdr);
	DP_STATS_INCC(peer, rx.ampdu_cnt, 1, is_ampdu);
	DP_STATS_INCC(peer, rx.non_ampdu_cnt, 1, !(is_ampdu));

	sgi = hal_rx_msdu_start_sgi_get(rx_tlv_hdr);
	mcs = hal_rx_msdu_start_rate_mcs_get(rx_tlv_hdr);
	tid = qdf_nbuf_get_tid_val(nbuf);
	bw = hal_rx_msdu_start_bw_get(rx_tlv_hdr);
	reception_type = hal_rx_msdu_start_reception_type_get(soc->hal_soc,
							      rx_tlv_hdr);
	nss = hal_rx_msdu_start_nss_get(soc->hal_soc, rx_tlv_hdr);
	pkt_type = hal_rx_msdu_start_get_pkt_type(rx_tlv_hdr);

	DP_STATS_INCC(peer, rx.rx_mpdu_cnt[mcs], 1,
		      ((mcs < MAX_MCS) && QDF_NBUF_CB_RX_CHFRAG_START(nbuf)));
	DP_STATS_INCC(peer, rx.rx_mpdu_cnt[MAX_MCS - 1], 1,
		      ((mcs >= MAX_MCS) && QDF_NBUF_CB_RX_CHFRAG_START(nbuf)));
	DP_STATS_INC(peer, rx.bw[bw], 1);
	/*
	 * only if nss > 0 and pkt_type is 11N/AC/AX,
	 * then increase index [nss - 1] in array counter.
	 */
	if (nss > 0 && (pkt_type == DOT11_N ||
			pkt_type == DOT11_AC ||
			pkt_type == DOT11_AX))
		DP_STATS_INC(peer, rx.nss[nss - 1], 1);

	DP_STATS_INC(peer, rx.sgi_count[sgi], 1);
	DP_STATS_INCC(peer, rx.err.mic_err, 1,
		      hal_rx_mpdu_end_mic_err_get(rx_tlv_hdr));
	DP_STATS_INCC(peer, rx.err.decrypt_err, 1,
		      hal_rx_mpdu_end_decrypt_err_get(rx_tlv_hdr));

	DP_STATS_INC(peer, rx.wme_ac_type[TID_TO_WME_AC(tid)], 1);
	DP_STATS_INC(peer, rx.reception_type[reception_type], 1);

	DP_STATS_INCC(peer, rx.pkt_type[pkt_type].mcs_count[MAX_MCS - 1], 1,
		      ((mcs >= MAX_MCS_11A) && (pkt_type == DOT11_A)));
	DP_STATS_INCC(peer, rx.pkt_type[pkt_type].mcs_count[mcs], 1,
		      ((mcs <= MAX_MCS_11A) && (pkt_type == DOT11_A)));
	DP_STATS_INCC(peer, rx.pkt_type[pkt_type].mcs_count[MAX_MCS - 1], 1,
		      ((mcs >= MAX_MCS_11B) && (pkt_type == DOT11_B)));
	DP_STATS_INCC(peer, rx.pkt_type[pkt_type].mcs_count[mcs], 1,
		      ((mcs <= MAX_MCS_11B) && (pkt_type == DOT11_B)));
	DP_STATS_INCC(peer, rx.pkt_type[pkt_type].mcs_count[MAX_MCS - 1], 1,
		      ((mcs >= MAX_MCS_11A) && (pkt_type == DOT11_N)));
	DP_STATS_INCC(peer, rx.pkt_type[pkt_type].mcs_count[mcs], 1,
		      ((mcs <= MAX_MCS_11A) && (pkt_type == DOT11_N)));
	DP_STATS_INCC(peer, rx.pkt_type[pkt_type].mcs_count[MAX_MCS - 1], 1,
		      ((mcs >= MAX_MCS_11AC) && (pkt_type == DOT11_AC)));
	DP_STATS_INCC(peer, rx.pkt_type[pkt_type].mcs_count[mcs], 1,
		      ((mcs <= MAX_MCS_11AC) && (pkt_type == DOT11_AC)));
	DP_STATS_INCC(peer, rx.pkt_type[pkt_type].mcs_count[MAX_MCS - 1], 1,
		      ((mcs >= MAX_MCS) && (pkt_type == DOT11_AX)));
	DP_STATS_INCC(peer, rx.pkt_type[pkt_type].mcs_count[mcs], 1,
		      ((mcs < MAX_MCS) && (pkt_type == DOT11_AX)));

	if ((soc->process_rx_status) &&
	    hal_rx_attn_first_mpdu_get(rx_tlv_hdr)) {
#if defined(FEATURE_PERPKT_INFO) && WDI_EVENT_ENABLE
		if (!vdev->pdev)
			return;

		dp_wdi_event_handler(WDI_EVENT_UPDATE_DP_STATS, vdev->pdev->soc,
				     &peer->stats, peer->peer_id,
				     UPDATE_PEER_STATS,
				     vdev->pdev->pdev_id);
#endif

	}
}

static inline bool is_sa_da_idx_valid(struct dp_soc *soc,
				      uint8_t *rx_tlv_hdr,
				      qdf_nbuf_t nbuf,
				      struct hal_rx_msdu_metadata msdu_info)
{
	if ((qdf_nbuf_is_sa_valid(nbuf) &&
	    (msdu_info.sa_idx > wlan_cfg_get_max_ast_idx(soc->wlan_cfg_ctx))) ||
	    (!qdf_nbuf_is_da_mcbc(nbuf) &&
	     qdf_nbuf_is_da_valid(nbuf) &&
	     (msdu_info.da_idx > wlan_cfg_get_max_ast_idx(soc->wlan_cfg_ctx))))
		return false;

	return true;
}

#ifndef WDS_VENDOR_EXTENSION
int dp_wds_rx_policy_check(uint8_t *rx_tlv_hdr,
			   struct dp_vdev *vdev,
			   struct dp_peer *peer)
{
	return 1;
}
#endif

#ifdef RX_DESC_DEBUG_CHECK
/**
 * dp_rx_desc_nbuf_sanity_check - Add sanity check to catch REO rx_desc paddr
 *				  corruption
 *
 * @ring_desc: REO ring descriptor
 * @rx_desc: Rx descriptor
 *
 * Return: NONE
 */
static inline
QDF_STATUS dp_rx_desc_nbuf_sanity_check(hal_ring_desc_t ring_desc,
					struct dp_rx_desc *rx_desc)
{
	struct hal_buf_info hbi;

	hal_rx_reo_buf_paddr_get(ring_desc, &hbi);
	/* Sanity check for possible buffer paddr corruption */
	if (dp_rx_desc_paddr_sanity_check(rx_desc, (&hbi)->paddr))
		return QDF_STATUS_SUCCESS;

	return QDF_STATUS_E_FAILURE;
}
#else
static inline
QDF_STATUS dp_rx_desc_nbuf_sanity_check(hal_ring_desc_t ring_desc,
					struct dp_rx_desc *rx_desc)
{
	return QDF_STATUS_SUCCESS;
}
#endif

#ifdef WLAN_FEATURE_RX_SOFTIRQ_TIME_LIMIT
static inline
bool dp_rx_reap_loop_pkt_limit_hit(struct dp_soc *soc, int num_reaped)
{
	bool limit_hit = false;
	struct wlan_cfg_dp_soc_ctxt *cfg = soc->wlan_cfg_ctx;

	limit_hit =
		(num_reaped >= cfg->rx_reap_loop_pkt_limit) ? true : false;

	if (limit_hit)
		DP_STATS_INC(soc, rx.reap_loop_pkt_limit_hit, 1)

	return limit_hit;
}

static inline bool dp_rx_enable_eol_data_check(struct dp_soc *soc)
{
	return soc->wlan_cfg_ctx->rx_enable_eol_data_check;
}

#else
static inline
bool dp_rx_reap_loop_pkt_limit_hit(struct dp_soc *soc, int num_reaped)
{
	return false;
}

static inline bool dp_rx_enable_eol_data_check(struct dp_soc *soc)
{
	return false;
}

#endif /* WLAN_FEATURE_RX_SOFTIRQ_TIME_LIMIT */

#ifdef DP_RX_PKT_NO_PEER_DELIVER
/**
 * dp_rx_deliver_to_stack_no_peer() - try deliver rx data even if
 *				      no corresbonding peer found
 * @soc: core txrx main context
 * @nbuf: pkt skb pointer
 *
 * This function will try to deliver some RX special frames to stack
 * even there is no peer matched found. for instance, LFR case, some
 * eapol data will be sent to host before peer_map done.
 *
 * Return: None
 */
static
void dp_rx_deliver_to_stack_no_peer(struct dp_soc *soc, qdf_nbuf_t nbuf)
{
	uint16_t peer_id;
	uint8_t vdev_id;
	struct dp_vdev *vdev = NULL;
	uint32_t l2_hdr_offset = 0;
	uint16_t msdu_len = 0;
	uint32_t pkt_len = 0;
	uint8_t *rx_tlv_hdr;
	uint32_t frame_mask = FRAME_MASK_IPV4_ARP | FRAME_MASK_IPV4_DHCP |
				FRAME_MASK_IPV4_EAPOL | FRAME_MASK_IPV6_DHCP;

	peer_id = QDF_NBUF_CB_RX_PEER_ID(nbuf);
	if (peer_id > soc->max_peers)
		goto deliver_fail;

	vdev_id = QDF_NBUF_CB_RX_VDEV_ID(nbuf);
	vdev = dp_vdev_get_ref_by_id(soc, vdev_id, DP_MOD_ID_RX);
	if (!vdev || vdev->delete.pending || !vdev->osif_rx)
		goto deliver_fail;

	if (qdf_unlikely(qdf_nbuf_is_frag(nbuf)))
		goto deliver_fail;

	rx_tlv_hdr = qdf_nbuf_data(nbuf);
	l2_hdr_offset =
		hal_rx_msdu_end_l3_hdr_padding_get(soc->hal_soc, rx_tlv_hdr);

	msdu_len = QDF_NBUF_CB_RX_PKT_LEN(nbuf);
	pkt_len = msdu_len + l2_hdr_offset + RX_PKT_TLVS_LEN;
	QDF_NBUF_CB_RX_NUM_ELEMENTS_IN_LIST(nbuf) = 1;

	qdf_nbuf_set_pktlen(nbuf, pkt_len);
	qdf_nbuf_pull_head(nbuf,
			   RX_PKT_TLVS_LEN +
			   l2_hdr_offset);

	if (dp_rx_is_special_frame(nbuf, frame_mask)) {
		qdf_nbuf_set_exc_frame(nbuf, 1);
		if (QDF_STATUS_SUCCESS !=
		    vdev->osif_rx(vdev->osif_vdev, nbuf))
			goto deliver_fail;
		DP_STATS_INC(soc, rx.err.pkt_delivered_no_peer, 1);
		dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_RX);
		return;
	}

deliver_fail:
	DP_STATS_INC_PKT(soc, rx.err.rx_invalid_peer, 1,
			 QDF_NBUF_CB_RX_PKT_LEN(nbuf));
	qdf_nbuf_free(nbuf);
	if (vdev)
		dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_RX);
}
#else
static inline
void dp_rx_deliver_to_stack_no_peer(struct dp_soc *soc, qdf_nbuf_t nbuf)
{
	DP_STATS_INC_PKT(soc, rx.err.rx_invalid_peer, 1,
			 QDF_NBUF_CB_RX_PKT_LEN(nbuf));
	qdf_nbuf_free(nbuf);
}
#endif

/**
 * dp_rx_srng_get_num_pending() - get number of pending entries
 * @hal_soc: hal soc opaque pointer
 * @hal_ring: opaque pointer to the HAL Rx Ring
 * @num_entries: number of entries in the hal_ring.
 * @near_full: pointer to a boolean. This is set if ring is near full.
 *
 * The function returns the number of entries in a destination ring which are
 * yet to be reaped. The function also checks if the ring is near full.
 * If more than half of the ring needs to be reaped, the ring is considered
 * approaching full.
 * The function useses hal_srng_dst_num_valid_locked to get the number of valid
 * entries. It should not be called within a SRNG lock. HW pointer value is
 * synced into cached_hp.
 *
 * Return: Number of pending entries if any
 */
static
uint32_t dp_rx_srng_get_num_pending(hal_soc_handle_t hal_soc,
				    hal_ring_handle_t hal_ring_hdl,
				    uint32_t num_entries,
				    bool *near_full)
{
	uint32_t num_pending = 0;

	num_pending = hal_srng_dst_num_valid_locked(hal_soc,
						    hal_ring_hdl,
						    true);

	if (num_entries && (num_pending >= num_entries >> 1))
		*near_full = true;
	else
		*near_full = false;

	return num_pending;
}

#ifdef WLAN_SUPPORT_RX_FISA
void dp_rx_skip_tlvs(qdf_nbuf_t nbuf, uint32_t l3_padding)
{
	QDF_NBUF_CB_RX_PACKET_L3_HDR_PAD(nbuf) = l3_padding;
	qdf_nbuf_pull_head(nbuf, l3_padding + RX_PKT_TLVS_LEN);
}

/**
 * dp_rx_set_hdr_pad() - set l3 padding in nbuf cb
 * @nbuf: pkt skb pointer
 * @l3_padding: l3 padding
 *
 * Return: None
 */
static inline
void dp_rx_set_hdr_pad(qdf_nbuf_t nbuf, uint32_t l3_padding)
{
	QDF_NBUF_CB_RX_PACKET_L3_HDR_PAD(nbuf) = l3_padding;
}
#else
void dp_rx_skip_tlvs(qdf_nbuf_t nbuf, uint32_t l3_padding)
{
	qdf_nbuf_pull_head(nbuf, l3_padding + RX_PKT_TLVS_LEN);
}

static inline
void dp_rx_set_hdr_pad(qdf_nbuf_t nbuf, uint32_t l3_padding)
{
}
#endif

#ifdef DP_RX_DROP_RAW_FRM
/**
 * dp_rx_is_raw_frame_dropped() - if raw frame nbuf, free and drop
 * @nbuf: pkt skb pointer
 *
 * Return: true - raw frame, dropped
 *	   false - not raw frame, do nothing
 */
static inline
bool dp_rx_is_raw_frame_dropped(qdf_nbuf_t nbuf)
{
	if (qdf_nbuf_is_raw_frame(nbuf)) {
		qdf_nbuf_free(nbuf);
		return true;
	}

	return false;
}
#else
static inline
bool dp_rx_is_raw_frame_dropped(qdf_nbuf_t nbuf)
{
	return false;
}
#endif

#ifdef WLAN_FEATURE_DP_RX_RING_HISTORY
/**
 * dp_rx_ring_record_entry() - Record an entry into the rx ring history.
 * @soc: Datapath soc structure
 * @ring_num: REO ring number
 * @ring_desc: REO ring descriptor
 *
 * Returns: None
 */
static inline void
dp_rx_ring_record_entry(struct dp_soc *soc, uint8_t ring_num,
			hal_ring_desc_t ring_desc)
{
	struct dp_buf_info_record *record;
	uint8_t rbm;
	struct hal_buf_info hbi;
	uint32_t idx;

	if (qdf_unlikely(!soc->rx_ring_history[ring_num]))
		return;

	hal_rx_reo_buf_paddr_get(ring_desc, &hbi);
	rbm = hal_rx_ret_buf_manager_get(ring_desc);

	idx = dp_history_get_next_index(&soc->rx_ring_history[ring_num]->index,
					DP_RX_HIST_MAX);

	/* No NULL check needed for record since its an array */
	record = &soc->rx_ring_history[ring_num]->entry[idx];

	record->timestamp = qdf_get_log_timestamp();
	record->hbi.paddr = hbi.paddr;
	record->hbi.sw_cookie = hbi.sw_cookie;
	record->hbi.rbm = rbm;
}
#else
static inline void
dp_rx_ring_record_entry(struct dp_soc *soc, uint8_t ring_num,
			hal_ring_desc_t ring_desc)
{
}
#endif

#ifdef WLAN_DP_FEATURE_SW_LATENCY_MGR
/**
 * dp_rx_update_stats() - Update soc level rx packet count
 * @soc: DP soc handle
 * @nbuf: nbuf received
 *
 * Returns: none
 */
static inline void dp_rx_update_stats(struct dp_soc *soc,
				      qdf_nbuf_t nbuf)
{
	DP_STATS_INC_PKT(soc, rx.ingress, 1,
			 QDF_NBUF_CB_RX_PKT_LEN(nbuf));
}
#else
static inline void dp_rx_update_stats(struct dp_soc *soc,
				      qdf_nbuf_t nbuf)
{
}
#endif

#ifdef WLAN_FEATURE_PKT_CAPTURE_LITHIUM
/**
 * dp_rx_deliver_to_pkt_capture() - deliver rx packet to packet capture
 * @soc : dp_soc handle
 * @pdev: dp_pdev handle
 * @peer_id: peer_id of the peer for which completion came
 * @ppdu_id: ppdu_id
 * @netbuf: Buffer pointer
 *
 * This function is used to deliver rx packet to packet capture
 */
void dp_rx_deliver_to_pkt_capture(struct dp_soc *soc,  struct dp_pdev *pdev,
				  uint16_t peer_id, uint32_t ppdu_id,
				  qdf_nbuf_t netbuf)
{
	dp_wdi_event_handler(WDI_EVENT_PKT_CAPTURE_RX_DATA, soc, netbuf,
			     peer_id, WDI_NO_VAL, pdev->pdev_id);
}
#endif

/**
 * dp_rx_process() - Brain of the Rx processing functionality
 *		     Called from the bottom half (tasklet/NET_RX_SOFTIRQ)
 * @int_ctx: per interrupt context
 * @hal_ring: opaque pointer to the HAL Rx Ring, which will be serviced
 * @reo_ring_num: ring number (0, 1, 2 or 3) of the reo ring.
 * @quota: No. of units (packets) that can be serviced in one shot.
 *
 * This function implements the core of Rx functionality. This is
 * expected to handle only non-error frames.
 *
 * Return: uint32_t: No. of elements processed
 */
uint32_t dp_rx_process(struct dp_intr *int_ctx, hal_ring_handle_t hal_ring_hdl,
			    uint8_t reo_ring_num, uint32_t quota)
{
	hal_ring_desc_t ring_desc;
	hal_soc_handle_t hal_soc;
	struct dp_rx_desc *rx_desc = NULL;
	qdf_nbuf_t nbuf, next;
	bool near_full;
	union dp_rx_desc_list_elem_t *head[MAX_PDEV_CNT];
	union dp_rx_desc_list_elem_t *tail[MAX_PDEV_CNT];
	uint32_t num_pending;
	uint32_t rx_bufs_used = 0, rx_buf_cookie;
	uint16_t msdu_len = 0;
	uint16_t peer_id;
	uint8_t vdev_id;
	struct dp_peer *peer;
	struct dp_vdev *vdev;
	uint32_t pkt_len = 0;
	struct hal_rx_mpdu_desc_info mpdu_desc_info;
	struct hal_rx_msdu_desc_info msdu_desc_info;
	enum hal_reo_error_status error;
	uint32_t peer_mdata;
	uint8_t *rx_tlv_hdr;
	uint32_t rx_bufs_reaped[MAX_PDEV_CNT];
	uint8_t mac_id = 0;
	struct dp_pdev *rx_pdev;
	struct dp_srng *dp_rxdma_srng;
	struct rx_desc_pool *rx_desc_pool;
	struct dp_soc *soc = int_ctx->soc;
	uint8_t ring_id = 0;
	uint8_t core_id = 0;
	struct cdp_tid_rx_stats *tid_stats;
	qdf_nbuf_t nbuf_head;
	qdf_nbuf_t nbuf_tail;
	qdf_nbuf_t deliver_list_head;
	qdf_nbuf_t deliver_list_tail;
	uint32_t num_rx_bufs_reaped = 0;
	uint32_t intr_id;
	struct hif_opaque_softc *scn;
	int32_t tid = 0;
	bool is_prev_msdu_last = true;
	uint32_t num_entries_avail = 0;
	uint32_t rx_ol_pkt_cnt = 0;
	uint32_t num_entries = 0;
	struct hal_rx_msdu_metadata msdu_metadata;
	QDF_STATUS status;
	qdf_nbuf_t ebuf_head;
	qdf_nbuf_t ebuf_tail;

	DP_HIST_INIT();

	qdf_assert_always(soc && hal_ring_hdl);
	hal_soc = soc->hal_soc;
	qdf_assert_always(hal_soc);

	scn = soc->hif_handle;
	hif_pm_runtime_mark_dp_rx_busy(scn);
	intr_id = int_ctx->dp_intr_id;
	num_entries = hal_srng_get_num_entries(hal_soc, hal_ring_hdl);

more_data:
	/* reset local variables here to be re-used in the function */
	nbuf_head = NULL;
	nbuf_tail = NULL;
	deliver_list_head = NULL;
	deliver_list_tail = NULL;
	peer = NULL;
	vdev = NULL;
	num_rx_bufs_reaped = 0;
	ebuf_head = NULL;
	ebuf_tail = NULL;

	qdf_mem_zero(rx_bufs_reaped, sizeof(rx_bufs_reaped));
	qdf_mem_zero(&mpdu_desc_info, sizeof(mpdu_desc_info));
	qdf_mem_zero(&msdu_desc_info, sizeof(msdu_desc_info));
	qdf_mem_zero(head, sizeof(head));
	qdf_mem_zero(tail, sizeof(tail));

	if (qdf_unlikely(dp_rx_srng_access_start(int_ctx, soc, hal_ring_hdl))) {

		/*
		 * Need API to convert from hal_ring pointer to
		 * Ring Type / Ring Id combo
		 */
		DP_STATS_INC(soc, rx.err.hal_ring_access_fail, 1);
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			FL("HAL RING Access Failed -- %pK"), hal_ring_hdl);
		goto done;
	}

	/*
	 * start reaping the buffers from reo ring and queue
	 * them in per vdev queue.
	 * Process the received pkts in a different per vdev loop.
	 */
	while (qdf_likely(quota &&
			  (ring_desc = hal_srng_dst_peek(hal_soc,
							 hal_ring_hdl)))) {

		error = HAL_RX_ERROR_STATUS_GET(ring_desc);
		ring_id = hal_srng_ring_id_get(hal_ring_hdl);

		if (qdf_unlikely(error == HAL_REO_ERROR_DETECTED)) {
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			FL("HAL RING 0x%pK:error %d"), hal_ring_hdl, error);
			DP_STATS_INC(soc, rx.err.hal_reo_error[ring_id], 1);
			/* Don't know how to deal with this -- assert */
			qdf_assert(0);
		}

		dp_rx_ring_record_entry(soc, reo_ring_num, ring_desc);
		rx_buf_cookie = HAL_RX_REO_BUF_COOKIE_GET(ring_desc);
		status = dp_rx_cookie_check_and_invalidate(ring_desc);
		if (qdf_unlikely(QDF_IS_STATUS_ERROR(status))) {
			DP_STATS_INC(soc, rx.err.stale_cookie, 1);
			break;
		}

		rx_desc = dp_rx_cookie_2_va_rxdma_buf(soc, rx_buf_cookie);
		status = dp_rx_desc_sanity(soc, hal_soc, hal_ring_hdl,
					   ring_desc, rx_desc);
		if (QDF_IS_STATUS_ERROR(status)) {
			if (qdf_unlikely(rx_desc && rx_desc->nbuf)) {
				qdf_assert_always(rx_desc->unmapped);
				dp_ipa_handle_rx_buf_smmu_mapping(
							soc,
							rx_desc->nbuf,
							RX_DATA_BUFFER_SIZE,
							false);
				qdf_nbuf_unmap_nbytes_single(
							soc->osdev,
							rx_desc->nbuf,
							QDF_DMA_FROM_DEVICE,
							RX_DATA_BUFFER_SIZE);
				rx_desc->unmapped = 1;
				dp_rx_buffer_pool_nbuf_free(soc, rx_desc->nbuf,
							    rx_desc->pool_id);
				dp_rx_add_to_free_desc_list(
							&head[rx_desc->pool_id],
							&tail[rx_desc->pool_id],
							rx_desc);
			}
			hal_srng_dst_get_next(hal_soc, hal_ring_hdl);
			continue;
		}

		/*
		 * this is a unlikely scenario where the host is reaping
		 * a descriptor which it already reaped just a while ago
		 * but is yet to replenish it back to HW.
		 * In this case host will dump the last 128 descriptors
		 * including the software descriptor rx_desc and assert.
		 */

		if (qdf_unlikely(!rx_desc->in_use)) {
			DP_STATS_INC(soc, rx.err.hal_reo_dest_dup, 1);
			dp_info_rl("Reaping rx_desc not in use!");
			dp_rx_dump_info_and_assert(soc, hal_ring_hdl,
						   ring_desc, rx_desc);
			/* ignore duplicate RX desc and continue to process */
			/* Pop out the descriptor */
			hal_srng_dst_get_next(hal_soc, hal_ring_hdl);
			continue;
		}

		status = dp_rx_desc_nbuf_sanity_check(ring_desc, rx_desc);
		if (qdf_unlikely(QDF_IS_STATUS_ERROR(status))) {
			DP_STATS_INC(soc, rx.err.nbuf_sanity_fail, 1);
			dp_info_rl("Nbuf sanity check failure!");
			dp_rx_dump_info_and_assert(soc, hal_ring_hdl,
						   ring_desc, rx_desc);
			rx_desc->in_err_state = 1;
			hal_srng_dst_get_next(hal_soc, hal_ring_hdl);
			continue;
		}

		if (qdf_unlikely(!dp_rx_desc_check_magic(rx_desc))) {
			dp_err("Invalid rx_desc cookie=%d", rx_buf_cookie);
			DP_STATS_INC(soc, rx.err.rx_desc_invalid_magic, 1);
			dp_rx_dump_info_and_assert(soc, hal_ring_hdl,
						   ring_desc, rx_desc);
		}

		/* Get MPDU DESC info */
		hal_rx_mpdu_desc_info_get(ring_desc, &mpdu_desc_info);

		/* Get MSDU DESC info */
		hal_rx_msdu_desc_info_get(ring_desc, &msdu_desc_info);

		if (qdf_unlikely(msdu_desc_info.msdu_flags &
				 HAL_MSDU_F_MSDU_CONTINUATION)) {
			/* previous msdu has end bit set, so current one is
			 * the new MPDU
			 */
			if (is_prev_msdu_last) {
				/* Get number of entries available in HW ring */
				num_entries_avail =
				hal_srng_dst_num_valid(hal_soc,
						       hal_ring_hdl, 1);

				/* For new MPDU check if we can read complete
				 * MPDU by comparing the number of buffers
				 * available and number of buffers needed to
				 * reap this MPDU
				 */
				if (((msdu_desc_info.msdu_len /
				     (RX_DATA_BUFFER_SIZE - RX_PKT_TLVS_LEN) +
				     1)) > num_entries_avail) {
					DP_STATS_INC(
						soc,
						rx.msdu_scatter_wait_break,
						1);
					break;
				}
				is_prev_msdu_last = false;
			}

		}

		core_id = smp_processor_id();
		DP_STATS_INC(soc, rx.ring_packets[core_id][ring_id], 1);

		if (mpdu_desc_info.mpdu_flags & HAL_MPDU_F_RETRY_BIT)
			qdf_nbuf_set_rx_retry_flag(rx_desc->nbuf, 1);

		if (qdf_unlikely(mpdu_desc_info.mpdu_flags &
				 HAL_MPDU_F_RAW_AMPDU))
			qdf_nbuf_set_raw_frame(rx_desc->nbuf, 1);

		if (!is_prev_msdu_last &&
		    msdu_desc_info.msdu_flags & HAL_MSDU_F_LAST_MSDU_IN_MPDU)
			is_prev_msdu_last = true;

		/* Pop out the descriptor*/
		hal_srng_dst_get_next(hal_soc, hal_ring_hdl);

		rx_bufs_reaped[rx_desc->pool_id]++;
		peer_mdata = mpdu_desc_info.peer_meta_data;
		QDF_NBUF_CB_RX_PEER_ID(rx_desc->nbuf) =
			DP_PEER_METADATA_PEER_ID_GET(peer_mdata);
		QDF_NBUF_CB_RX_VDEV_ID(rx_desc->nbuf) =
			DP_PEER_METADATA_VDEV_ID_GET(peer_mdata);

		/*
		 * save msdu flags first, last and continuation msdu in
		 * nbuf->cb, also save mcbc, is_da_valid, is_sa_valid and
		 * length to nbuf->cb. This ensures the info required for
		 * per pkt processing is always in the same cache line.
		 * This helps in improving throughput for smaller pkt
		 * sizes.
		 */
		if (msdu_desc_info.msdu_flags & HAL_MSDU_F_FIRST_MSDU_IN_MPDU)
			qdf_nbuf_set_rx_chfrag_start(rx_desc->nbuf, 1);

		if (msdu_desc_info.msdu_flags & HAL_MSDU_F_MSDU_CONTINUATION)
			qdf_nbuf_set_rx_chfrag_cont(rx_desc->nbuf, 1);

		if (msdu_desc_info.msdu_flags & HAL_MSDU_F_LAST_MSDU_IN_MPDU)
			qdf_nbuf_set_rx_chfrag_end(rx_desc->nbuf, 1);

		if (msdu_desc_info.msdu_flags & HAL_MSDU_F_DA_IS_MCBC)
			qdf_nbuf_set_da_mcbc(rx_desc->nbuf, 1);

		if (msdu_desc_info.msdu_flags & HAL_MSDU_F_DA_IS_VALID)
			qdf_nbuf_set_da_valid(rx_desc->nbuf, 1);

		if (msdu_desc_info.msdu_flags & HAL_MSDU_F_SA_IS_VALID)
			qdf_nbuf_set_sa_valid(rx_desc->nbuf, 1);

		qdf_nbuf_set_tid_val(rx_desc->nbuf,
				     HAL_RX_REO_QUEUE_NUMBER_GET(ring_desc));
		qdf_nbuf_set_rx_reo_dest_ind(
				rx_desc->nbuf,
				HAL_RX_REO_MSDU_REO_DST_IND_GET(ring_desc));

		QDF_NBUF_CB_RX_PKT_LEN(rx_desc->nbuf) = msdu_desc_info.msdu_len;

		QDF_NBUF_CB_RX_CTX_ID(rx_desc->nbuf) = reo_ring_num;

		/*
		 * move unmap after scattered msdu waiting break logic
		 * in case double skb unmap happened.
		 */
		rx_desc_pool = &soc->rx_desc_buf[rx_desc->pool_id];
		dp_ipa_handle_rx_buf_smmu_mapping(soc, rx_desc->nbuf,
						  rx_desc_pool->buf_size,
						  false);
		qdf_nbuf_unmap_nbytes_single(soc->osdev, rx_desc->nbuf,
					     QDF_DMA_FROM_DEVICE,
					     rx_desc_pool->buf_size);
		rx_desc->unmapped = 1;
		DP_RX_PROCESS_NBUF(soc, nbuf_head, nbuf_tail, ebuf_head,
				   ebuf_tail, rx_desc);
		/*
		 * if continuation bit is set then we have MSDU spread
		 * across multiple buffers, let us not decrement quota
		 * till we reap all buffers of that MSDU.
		 */
		if (qdf_likely(!qdf_nbuf_is_rx_chfrag_cont(rx_desc->nbuf)))
			quota -= 1;

		dp_rx_add_to_free_desc_list(&head[rx_desc->pool_id],
						&tail[rx_desc->pool_id],
						rx_desc);

		num_rx_bufs_reaped++;
		/*
		 * only if complete msdu is received for scatter case,
		 * then allow break.
		 */
		if (is_prev_msdu_last &&
		    dp_rx_reap_loop_pkt_limit_hit(soc, num_rx_bufs_reaped))
			break;
	}
done:
	dp_rx_srng_access_end(int_ctx, soc, hal_ring_hdl);

	for (mac_id = 0; mac_id < MAX_PDEV_CNT; mac_id++) {
		/*
		 * continue with next mac_id if no pkts were reaped
		 * from that pool
		 */
		if (!rx_bufs_reaped[mac_id])
			continue;

		dp_rxdma_srng = &soc->rx_refill_buf_ring[mac_id];

		rx_desc_pool = &soc->rx_desc_buf[mac_id];

		dp_rx_buffers_replenish(soc, mac_id, dp_rxdma_srng,
					rx_desc_pool, rx_bufs_reaped[mac_id],
					&head[mac_id], &tail[mac_id]);
	}

	dp_verbose_debug("replenished %u\n", rx_bufs_reaped[0]);
	/* Peer can be NULL is case of LFR */
	if (qdf_likely(peer))
		vdev = NULL;

	/*
	 * BIG loop where each nbuf is dequeued from global queue,
	 * processed and queued back on a per vdev basis. These nbufs
	 * are sent to stack as and when we run out of nbufs
	 * or a new nbuf dequeued from global queue has a different
	 * vdev when compared to previous nbuf.
	 */
	nbuf = nbuf_head;
	while (nbuf) {
		next = nbuf->next;
		if (qdf_unlikely(dp_rx_is_raw_frame_dropped(nbuf))) {
			nbuf = next;
			DP_STATS_INC(soc, rx.err.raw_frm_drop, 1);
			continue;
		}

		rx_tlv_hdr = qdf_nbuf_data(nbuf);
		vdev_id = QDF_NBUF_CB_RX_VDEV_ID(nbuf);
		peer_id =  QDF_NBUF_CB_RX_PEER_ID(nbuf);

		if (dp_rx_is_list_ready(deliver_list_head, vdev, peer,
					peer_id, vdev_id)) {
			dp_rx_deliver_to_stack(soc, vdev, peer,
					       deliver_list_head,
					       deliver_list_tail);
			deliver_list_head = NULL;
			deliver_list_tail = NULL;
		}

		/* Get TID from struct cb->tid_val, save to tid */
		if (qdf_nbuf_is_rx_chfrag_start(nbuf))
			tid = qdf_nbuf_get_tid_val(nbuf);

		if (qdf_unlikely(!peer)) {
			peer = dp_peer_get_ref_by_id(soc, peer_id,
						     DP_MOD_ID_RX);
		} else if (peer && peer->peer_id != peer_id) {
			dp_peer_unref_delete(peer, DP_MOD_ID_RX);
			peer = dp_peer_get_ref_by_id(soc, peer_id,
						     DP_MOD_ID_RX);
		}

		if (peer) {
			QDF_NBUF_CB_DP_TRACE_PRINT(nbuf) = false;
			qdf_dp_trace_set_track(nbuf, QDF_RX);
			QDF_NBUF_CB_RX_DP_TRACE(nbuf) = 1;
			QDF_NBUF_CB_RX_PACKET_TRACK(nbuf) =
				QDF_NBUF_RX_PKT_DATA_TRACK;
		}

		rx_bufs_used++;

		if (qdf_likely(peer)) {
			vdev = peer->vdev;
		} else {
			nbuf->next = NULL;
			dp_rx_deliver_to_stack_no_peer(soc, nbuf);
			nbuf = next;
			continue;
		}

		if (qdf_unlikely(!vdev)) {
			qdf_nbuf_free(nbuf);
			nbuf = next;
			DP_STATS_INC(soc, rx.err.invalid_vdev, 1);
			continue;
		}

		/* when hlos tid override is enabled, save tid in
		 * skb->priority
		 */
		if (qdf_unlikely(vdev->skip_sw_tid_classification &
					DP_TXRX_HLOS_TID_OVERRIDE_ENABLED))
			qdf_nbuf_set_priority(nbuf, tid);

		rx_pdev = vdev->pdev;
		DP_RX_TID_SAVE(nbuf, tid);
		if (qdf_unlikely(rx_pdev->delay_stats_flag) ||
		    qdf_unlikely(wlan_cfg_is_peer_ext_stats_enabled(
				 soc->wlan_cfg_ctx)))
			qdf_nbuf_set_timestamp(nbuf);

		ring_id = QDF_NBUF_CB_RX_CTX_ID(nbuf);
		tid_stats =
			&rx_pdev->stats.tid_stats.tid_rx_stats[ring_id][tid];

		/*
		 * Check if DMA completed -- msdu_done is the last bit
		 * to be written
		 */
		if (qdf_unlikely(!qdf_nbuf_is_rx_chfrag_cont(nbuf) &&
				 !hal_rx_attn_msdu_done_get(rx_tlv_hdr))) {
			dp_err("MSDU DONE failure");
			DP_STATS_INC(soc, rx.err.msdu_done_fail, 1);
			hal_rx_dump_pkt_tlvs(hal_soc, rx_tlv_hdr,
					     QDF_TRACE_LEVEL_INFO);
			tid_stats->fail_cnt[MSDU_DONE_FAILURE]++;
			qdf_nbuf_free(nbuf);
			qdf_assert(0);
			nbuf = next;
			continue;
		}

		DP_HIST_PACKET_COUNT_INC(vdev->pdev->pdev_id);
		/*
		 * First IF condition:
		 * 802.11 Fragmented pkts are reinjected to REO
		 * HW block as SG pkts and for these pkts we only
		 * need to pull the RX TLVS header length.
		 * Second IF condition:
		 * The below condition happens when an MSDU is spread
		 * across multiple buffers. This can happen in two cases
		 * 1. The nbuf size is smaller then the received msdu.
		 *    ex: we have set the nbuf size to 2048 during
		 *        nbuf_alloc. but we received an msdu which is
		 *        2304 bytes in size then this msdu is spread
		 *        across 2 nbufs.
		 *
		 * 2. AMSDUs when RAW mode is enabled.
		 *    ex: 1st MSDU is in 1st nbuf and 2nd MSDU is spread
		 *        across 1st nbuf and 2nd nbuf and last MSDU is
		 *        spread across 2nd nbuf and 3rd nbuf.
		 *
		 * for these scenarios let us create a skb frag_list and
		 * append these buffers till the last MSDU of the AMSDU
		 * Third condition:
		 * This is the most likely case, we receive 802.3 pkts
		 * decapsulated by HW, here we need to set the pkt length.
		 */
		hal_rx_msdu_metadata_get(hal_soc, rx_tlv_hdr, &msdu_metadata);
		if (qdf_unlikely(qdf_nbuf_is_frag(nbuf))) {
			bool is_mcbc, is_sa_vld, is_da_vld;

			is_mcbc = hal_rx_msdu_end_da_is_mcbc_get(soc->hal_soc,
								 rx_tlv_hdr);
			is_sa_vld =
				hal_rx_msdu_end_sa_is_valid_get(soc->hal_soc,
								rx_tlv_hdr);
			is_da_vld =
				hal_rx_msdu_end_da_is_valid_get(soc->hal_soc,
								rx_tlv_hdr);

			qdf_nbuf_set_da_mcbc(nbuf, is_mcbc);
			qdf_nbuf_set_da_valid(nbuf, is_da_vld);
			qdf_nbuf_set_sa_valid(nbuf, is_sa_vld);

			qdf_nbuf_pull_head(nbuf, RX_PKT_TLVS_LEN);
		} else if (qdf_nbuf_is_rx_chfrag_cont(nbuf)) {
			msdu_len = QDF_NBUF_CB_RX_PKT_LEN(nbuf);
			nbuf = dp_rx_sg_create(nbuf);
			next = nbuf->next;

			if (qdf_nbuf_is_raw_frame(nbuf)) {
				DP_STATS_INC(vdev->pdev, rx_raw_pkts, 1);
				DP_STATS_INC_PKT(peer, rx.raw, 1, msdu_len);
			} else {
				qdf_nbuf_free(nbuf);
				DP_STATS_INC(soc, rx.err.scatter_msdu, 1);
				dp_info_rl("scatter msdu len %d, dropped",
					   msdu_len);
				nbuf = next;
				continue;
			}
		} else {

			msdu_len = QDF_NBUF_CB_RX_PKT_LEN(nbuf);
			pkt_len = msdu_len +
				  msdu_metadata.l3_hdr_pad +
				  RX_PKT_TLVS_LEN;

			qdf_nbuf_set_pktlen(nbuf, pkt_len);
			dp_rx_skip_tlvs(nbuf, msdu_metadata.l3_hdr_pad);
		}

		/*
		 * process frame for mulitpass phrase processing
		 */
		if (qdf_unlikely(vdev->multipass_en)) {
			if (dp_rx_multipass_process(peer, nbuf, tid) == false) {
				DP_STATS_INC(peer, rx.multipass_rx_pkt_drop, 1);
				qdf_nbuf_free(nbuf);
				nbuf = next;
				continue;
			}
		}

		if (!dp_wds_rx_policy_check(rx_tlv_hdr, vdev, peer)) {
			QDF_TRACE(QDF_MODULE_ID_DP,
					QDF_TRACE_LEVEL_ERROR,
					FL("Policy Check Drop pkt"));
			tid_stats->fail_cnt[POLICY_CHECK_DROP]++;
			/* Drop & free packet */
			qdf_nbuf_free(nbuf);
			/* Statistics */
			nbuf = next;
			continue;
		}

		if (qdf_unlikely(peer && (peer->nawds_enabled) &&
				 (qdf_nbuf_is_da_mcbc(nbuf)) &&
				 (hal_rx_get_mpdu_mac_ad4_valid(soc->hal_soc,
								rx_tlv_hdr) ==
				  false))) {
			tid_stats->fail_cnt[NAWDS_MCAST_DROP]++;
			DP_STATS_INC(peer, rx.nawds_mcast_drop, 1);
			qdf_nbuf_free(nbuf);
			nbuf = next;
			continue;
		}

		if (soc->process_rx_status)
			dp_rx_cksum_offload(vdev->pdev, nbuf, rx_tlv_hdr);

		/* Update the protocol tag in SKB based on CCE metadata */
		dp_rx_update_protocol_tag(soc, vdev, nbuf, rx_tlv_hdr,
					  reo_ring_num, false, true);

		/* Update the flow tag in SKB based on FSE metadata */
		dp_rx_update_flow_tag(soc, vdev, nbuf, rx_tlv_hdr, true);

		dp_rx_msdu_stats_update(soc, nbuf, rx_tlv_hdr, peer,
					ring_id, tid_stats);

		if (qdf_unlikely(vdev->mesh_vdev)) {
			if (dp_rx_filter_mesh_packets(vdev, nbuf, rx_tlv_hdr)
					== QDF_STATUS_SUCCESS) {
				QDF_TRACE(QDF_MODULE_ID_DP,
						QDF_TRACE_LEVEL_INFO_MED,
						FL("mesh pkt filtered"));
				tid_stats->fail_cnt[MESH_FILTER_DROP]++;
				DP_STATS_INC(vdev->pdev, dropped.mesh_filter,
					     1);

				qdf_nbuf_free(nbuf);
				nbuf = next;
				continue;
			}
			dp_rx_fill_mesh_stats(vdev, nbuf, rx_tlv_hdr, peer);
		}

		if (qdf_likely(vdev->rx_decap_type ==
			       htt_cmn_pkt_type_ethernet) &&
		    qdf_likely(!vdev->mesh_vdev)) {
			/* WDS Destination Address Learning */
			dp_rx_da_learn(soc, rx_tlv_hdr, peer, nbuf);

			/* Due to HW issue, sometimes we see that the sa_idx
			 * and da_idx are invalid with sa_valid and da_valid
			 * bits set
			 *
			 * in this case we also see that value of
			 * sa_sw_peer_id is set as 0
			 *
			 * Drop the packet if sa_idx and da_idx OOB or
			 * sa_sw_peerid is 0
			 */
			if (!is_sa_da_idx_valid(soc, rx_tlv_hdr, nbuf,
						msdu_metadata)) {
				qdf_nbuf_free(nbuf);
				nbuf = next;
				DP_STATS_INC(soc, rx.err.invalid_sa_da_idx, 1);
				continue;
			}
			/* WDS Source Port Learning */
			if (qdf_likely(vdev->wds_enabled))
				dp_rx_wds_srcport_learn(soc,
							rx_tlv_hdr,
							peer,
							nbuf,
							msdu_metadata);

			/* Intrabss-fwd */
			if (dp_rx_check_ap_bridge(vdev))
				if (dp_rx_intrabss_fwd(soc,
							peer,
							rx_tlv_hdr,
							nbuf,
							msdu_metadata)) {
					nbuf = next;
					tid_stats->intrabss_cnt++;
					continue; /* Get next desc */
				}
		}

		dp_rx_fill_gro_info(soc, rx_tlv_hdr, nbuf, &rx_ol_pkt_cnt);

		dp_rx_update_stats(soc, nbuf);
		DP_RX_LIST_APPEND(deliver_list_head,
				  deliver_list_tail,
				  nbuf);
		DP_STATS_INC_PKT(peer, rx.to_stack, 1,
				 QDF_NBUF_CB_RX_PKT_LEN(nbuf));
		if (qdf_unlikely(peer->in_twt))
			DP_STATS_INC_PKT(peer, rx.to_stack_twt, 1,
					 QDF_NBUF_CB_RX_PKT_LEN(nbuf));

		tid_stats->delivered_to_stack++;
		nbuf = next;
	}

	if (qdf_likely(deliver_list_head)) {
		if (qdf_likely(peer)) {
			dp_rx_deliver_to_pkt_capture(soc, vdev->pdev, peer_id,
						     0, deliver_list_head);
			dp_rx_deliver_to_stack(soc, vdev, peer,
					       deliver_list_head,
					       deliver_list_tail);
		}
		else {
			nbuf = deliver_list_head;
			while (nbuf) {
				next = nbuf->next;
				nbuf->next = NULL;
				dp_rx_deliver_to_stack_no_peer(soc, nbuf);
				nbuf = next;
			}
		}
	}

	if (qdf_likely(peer))
		dp_peer_unref_delete(peer, DP_MOD_ID_RX);

	if (dp_rx_enable_eol_data_check(soc) && rx_bufs_used) {
		if (quota) {
			num_pending =
				dp_rx_srng_get_num_pending(hal_soc,
							   hal_ring_hdl,
							   num_entries,
							   &near_full);
			if (num_pending) {
				DP_STATS_INC(soc, rx.hp_oos2, 1);

				if (!hif_exec_should_yield(scn, intr_id))
					goto more_data;

				if (qdf_unlikely(near_full)) {
					DP_STATS_INC(soc, rx.near_full, 1);
					goto more_data;
				}
			}
		}

		if (vdev && vdev->osif_fisa_flush)
			vdev->osif_fisa_flush(soc, reo_ring_num);

		if (vdev && vdev->osif_gro_flush && rx_ol_pkt_cnt) {
			vdev->osif_gro_flush(vdev->osif_vdev,
					     reo_ring_num);
		}
	}

	/* Update histogram statistics by looping through pdev's */
	DP_RX_HIST_STATS_PER_PDEV();

	return rx_bufs_used; /* Assume no scale factor for now */
}

QDF_STATUS dp_rx_vdev_detach(struct dp_vdev *vdev)
{
	QDF_STATUS ret;

	if (vdev->osif_rx_flush) {
		ret = vdev->osif_rx_flush(vdev->osif_vdev, vdev->vdev_id);
		if (!QDF_IS_STATUS_SUCCESS(ret)) {
			dp_err("Failed to flush rx pkts for vdev %d\n",
			       vdev->vdev_id);
			return ret;
		}
	}

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS
dp_pdev_nbuf_alloc_and_map(struct dp_soc *dp_soc,
			   struct dp_rx_nbuf_frag_info *nbuf_frag_info_t,
			   struct dp_pdev *dp_pdev,
			   struct rx_desc_pool *rx_desc_pool)
{
	QDF_STATUS ret = QDF_STATUS_E_FAILURE;

	(nbuf_frag_info_t->virt_addr).nbuf =
		qdf_nbuf_alloc(dp_soc->osdev, rx_desc_pool->buf_size,
			       RX_BUFFER_RESERVATION,
			       rx_desc_pool->buf_alignment, FALSE);
	if (!((nbuf_frag_info_t->virt_addr).nbuf)) {
		dp_err("nbuf alloc failed");
		DP_STATS_INC(dp_pdev, replenish.nbuf_alloc_fail, 1);
		return ret;
	}

	ret = qdf_nbuf_map_nbytes_single(dp_soc->osdev,
					 (nbuf_frag_info_t->virt_addr).nbuf,
					 QDF_DMA_FROM_DEVICE,
					 rx_desc_pool->buf_size);

	if (qdf_unlikely(QDF_IS_STATUS_ERROR(ret))) {
		qdf_nbuf_free((nbuf_frag_info_t->virt_addr).nbuf);
		dp_err("nbuf map failed");
		DP_STATS_INC(dp_pdev, replenish.map_err, 1);
		return ret;
	}

	nbuf_frag_info_t->paddr =
		qdf_nbuf_get_frag_paddr((nbuf_frag_info_t->virt_addr).nbuf, 0);

	ret = check_x86_paddr(dp_soc, &((nbuf_frag_info_t->virt_addr).nbuf),
			      &nbuf_frag_info_t->paddr,
			      rx_desc_pool);
	if (ret == QDF_STATUS_E_FAILURE) {
		qdf_nbuf_unmap_nbytes_single(dp_soc->osdev,
					     (nbuf_frag_info_t->virt_addr).nbuf,
					     QDF_DMA_FROM_DEVICE,
					     rx_desc_pool->buf_size);
		qdf_nbuf_free((nbuf_frag_info_t->virt_addr).nbuf);
		dp_err("nbuf check x86 failed");
		DP_STATS_INC(dp_pdev, replenish.x86_fail, 1);
		return ret;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
dp_pdev_rx_buffers_attach(struct dp_soc *dp_soc, uint32_t mac_id,
			  struct dp_srng *dp_rxdma_srng,
			  struct rx_desc_pool *rx_desc_pool,
			  uint32_t num_req_buffers)
{
	struct dp_pdev *dp_pdev = dp_get_pdev_for_lmac_id(dp_soc, mac_id);
	hal_ring_handle_t rxdma_srng = dp_rxdma_srng->hal_srng;
	union dp_rx_desc_list_elem_t *next;
	void *rxdma_ring_entry;
	qdf_dma_addr_t paddr;
	struct dp_rx_nbuf_frag_info *nf_info;
	uint32_t nr_descs, nr_nbuf = 0, nr_nbuf_total = 0;
	uint32_t buffer_index, nbuf_ptrs_per_page;
	qdf_nbuf_t nbuf;
	QDF_STATUS ret;
	int page_idx, total_pages;
	union dp_rx_desc_list_elem_t *desc_list = NULL;
	union dp_rx_desc_list_elem_t *tail = NULL;
	int sync_hw_ptr = 1;
	uint32_t num_entries_avail;

	if (qdf_unlikely(!rxdma_srng)) {
		DP_STATS_INC(dp_pdev, replenish.rxdma_err, num_req_buffers);
		return QDF_STATUS_E_FAILURE;
	}

	dp_debug("requested %u RX buffers for driver attach", num_req_buffers);

	hal_srng_access_start(dp_soc->hal_soc, rxdma_srng);
	num_entries_avail = hal_srng_src_num_avail(dp_soc->hal_soc,
						   rxdma_srng,
						   sync_hw_ptr);
	hal_srng_access_end(dp_soc->hal_soc, rxdma_srng);

	if (!num_entries_avail) {
		dp_err("Num of available entries is zero, nothing to do");
		return QDF_STATUS_E_NOMEM;
	}

	if (num_entries_avail < num_req_buffers)
		num_req_buffers = num_entries_avail;

	nr_descs = dp_rx_get_free_desc_list(dp_soc, mac_id, rx_desc_pool,
					    num_req_buffers, &desc_list, &tail);
	if (!nr_descs) {
		dp_err("no free rx_descs in freelist");
		DP_STATS_INC(dp_pdev, err.desc_alloc_fail, num_req_buffers);
		return QDF_STATUS_E_NOMEM;
	}

	dp_debug("got %u RX descs for driver attach", nr_descs);

	/*
	 * Try to allocate pointers to the nbuf one page at a time.
	 * Take pointers that can fit in one page of memory and
	 * iterate through the total descriptors that need to be
	 * allocated in order of pages. Reuse the pointers that
	 * have been allocated to fit in one page across each
	 * iteration to index into the nbuf.
	 */
	total_pages = (nr_descs * sizeof(*nf_info)) / PAGE_SIZE;

	/*
	 * Add an extra page to store the remainder if any
	 */
	if ((nr_descs * sizeof(*nf_info)) % PAGE_SIZE)
		total_pages++;
	nf_info = qdf_mem_malloc(PAGE_SIZE);
	if (!nf_info) {
		dp_err("failed to allocate nbuf array");
		DP_STATS_INC(dp_pdev, replenish.rxdma_err, num_req_buffers);
		QDF_BUG(0);
		return QDF_STATUS_E_NOMEM;
	}
	nbuf_ptrs_per_page = PAGE_SIZE / sizeof(*nf_info);

	for (page_idx = 0; page_idx < total_pages; page_idx++) {
		qdf_mem_zero(nf_info, PAGE_SIZE);

		for (nr_nbuf = 0; nr_nbuf < nbuf_ptrs_per_page; nr_nbuf++) {
			/*
			 * The last page of buffer pointers may not be required
			 * completely based on the number of descriptors. Below
			 * check will ensure we are allocating only the
			 * required number of descriptors.
			 */
			if (nr_nbuf_total >= nr_descs)
				break;
			/* Flag is set while pdev rx_desc_pool initialization */
			if (qdf_unlikely(rx_desc_pool->rx_mon_dest_frag_enable))
				ret = dp_pdev_frag_alloc_and_map(dp_soc,
						&nf_info[nr_nbuf], dp_pdev,
						rx_desc_pool);
			else
				ret = dp_pdev_nbuf_alloc_and_map(dp_soc,
						&nf_info[nr_nbuf], dp_pdev,
						rx_desc_pool);
			if (QDF_IS_STATUS_ERROR(ret))
				break;

			nr_nbuf_total++;
		}

		hal_srng_access_start(dp_soc->hal_soc, rxdma_srng);

		for (buffer_index = 0; buffer_index < nr_nbuf; buffer_index++) {
			rxdma_ring_entry =
				hal_srng_src_get_next(dp_soc->hal_soc,
						      rxdma_srng);
			qdf_assert_always(rxdma_ring_entry);

			next = desc_list->next;
			paddr = nf_info[buffer_index].paddr;
			nbuf = nf_info[buffer_index].virt_addr.nbuf;

			/* Flag is set while pdev rx_desc_pool initialization */
			if (qdf_unlikely(rx_desc_pool->rx_mon_dest_frag_enable))
				dp_rx_desc_frag_prep(&desc_list->rx_desc,
						     &nf_info[buffer_index]);
			else
				dp_rx_desc_prep(&desc_list->rx_desc,
						&nf_info[buffer_index]);
			desc_list->rx_desc.in_use = 1;
			dp_rx_desc_alloc_dbg_info(&desc_list->rx_desc);
			dp_rx_desc_update_dbg_info(&desc_list->rx_desc,
						   __func__,
						   RX_DESC_REPLENISHED);

			hal_rxdma_buff_addr_info_set(rxdma_ring_entry, paddr,
						     desc_list->rx_desc.cookie,
						     rx_desc_pool->owner);
			dp_ipa_handle_rx_buf_smmu_mapping(
						dp_soc, nbuf,
						rx_desc_pool->buf_size,
						true);

			desc_list = next;
		}

		hal_srng_access_end(dp_soc->hal_soc, rxdma_srng);
	}

	dp_info("filled %u RX buffers for driver attach", nr_nbuf_total);
	qdf_mem_free(nf_info);

	if (!nr_nbuf_total) {
		dp_err("No nbuf's allocated");
		QDF_BUG(0);
		return QDF_STATUS_E_RESOURCES;
	}

	/* No need to count the number of bytes received during replenish.
	 * Therefore set replenish.pkts.bytes as 0.
	 */
	DP_STATS_INC_PKT(dp_pdev, replenish.pkts, nr_nbuf, 0);

	return QDF_STATUS_SUCCESS;
}

/**
 * dp_rx_enable_mon_dest_frag() - Enable frag processing for
 *              monitor destination ring via frag.
 *
 * Enable this flag only for monitor destination buffer processing
 * if DP_RX_MON_MEM_FRAG feature is enabled.
 * If flag is set then frag based function will be called for alloc,
 * map, prep desc and free ops for desc buffer else normal nbuf based
 * function will be called.
 *
 * @rx_desc_pool: Rx desc pool
 * @is_mon_dest_desc: Is it for monitor dest buffer
 *
 * Return: None
 */
#ifdef DP_RX_MON_MEM_FRAG
void dp_rx_enable_mon_dest_frag(struct rx_desc_pool *rx_desc_pool,
				bool is_mon_dest_desc)
{
	rx_desc_pool->rx_mon_dest_frag_enable = is_mon_dest_desc;
	if (is_mon_dest_desc)
		dp_alert("Feature DP_RX_MON_MEM_FRAG for mon_dest is enabled");
}
#else
void dp_rx_enable_mon_dest_frag(struct rx_desc_pool *rx_desc_pool,
				bool is_mon_dest_desc)
{
	rx_desc_pool->rx_mon_dest_frag_enable = false;
	if (is_mon_dest_desc)
		dp_alert("Feature DP_RX_MON_MEM_FRAG for mon_dest is disabled");
}
#endif

/*
 * dp_rx_pdev_desc_pool_alloc() -  allocate memory for software rx descriptor
 *				   pool
 *
 * @pdev: core txrx pdev context
 *
 * Return: QDF_STATUS - QDF_STATUS_SUCCESS
 *			QDF_STATUS_E_NOMEM
 */
QDF_STATUS
dp_rx_pdev_desc_pool_alloc(struct dp_pdev *pdev)
{
	struct dp_soc *soc = pdev->soc;
	uint32_t rxdma_entries;
	uint32_t rx_sw_desc_num;
	struct dp_srng *dp_rxdma_srng;
	struct rx_desc_pool *rx_desc_pool;
	uint32_t status = QDF_STATUS_SUCCESS;
	int mac_for_pdev;

	mac_for_pdev = pdev->lmac_id;
	if (wlan_cfg_get_dp_pdev_nss_enabled(pdev->wlan_cfg_ctx)) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
			  "nss-wifi<4> skip Rx refil %d", mac_for_pdev);
		return status;
	}

	dp_rxdma_srng = &soc->rx_refill_buf_ring[mac_for_pdev];
	rxdma_entries = dp_rxdma_srng->num_entries;

	rx_desc_pool = &soc->rx_desc_buf[mac_for_pdev];
	rx_sw_desc_num = wlan_cfg_get_dp_soc_rx_sw_desc_num(soc->wlan_cfg_ctx);

	rx_desc_pool->desc_type = DP_RX_DESC_BUF_TYPE;
	status = dp_rx_desc_pool_alloc(soc,
				       rx_sw_desc_num,
				       rx_desc_pool);
	if (status != QDF_STATUS_SUCCESS)
		return status;

	return status;
}

/*
 * dp_rx_pdev_desc_pool_free() - free software rx descriptor pool
 *
 * @pdev: core txrx pdev context
 */
void dp_rx_pdev_desc_pool_free(struct dp_pdev *pdev)
{
	int mac_for_pdev = pdev->lmac_id;
	struct dp_soc *soc = pdev->soc;
	struct rx_desc_pool *rx_desc_pool;

	rx_desc_pool = &soc->rx_desc_buf[mac_for_pdev];

	dp_rx_desc_pool_free(soc, rx_desc_pool);
}

/*
 * dp_rx_pdev_desc_pool_init() - initialize software rx descriptors
 *
 * @pdev: core txrx pdev context
 *
 * Return: QDF_STATUS - QDF_STATUS_SUCCESS
 *			QDF_STATUS_E_NOMEM
 */
QDF_STATUS dp_rx_pdev_desc_pool_init(struct dp_pdev *pdev)
{
	int mac_for_pdev = pdev->lmac_id;
	struct dp_soc *soc = pdev->soc;
	uint32_t rxdma_entries;
	uint32_t rx_sw_desc_num;
	struct dp_srng *dp_rxdma_srng;
	struct rx_desc_pool *rx_desc_pool;

	if (wlan_cfg_get_dp_pdev_nss_enabled(pdev->wlan_cfg_ctx)) {
		/**
		 * If NSS is enabled, rx_desc_pool is already filled.
		 * Hence, just disable desc_pool frag flag.
		 */
		rx_desc_pool = &soc->rx_desc_buf[mac_for_pdev];
		dp_rx_enable_mon_dest_frag(rx_desc_pool, false);

		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
			  "nss-wifi<4> skip Rx refil %d", mac_for_pdev);
		return QDF_STATUS_SUCCESS;
	}

	rx_desc_pool = &soc->rx_desc_buf[mac_for_pdev];
	if (dp_rx_desc_pool_is_allocated(rx_desc_pool) == QDF_STATUS_E_NOMEM)
		return QDF_STATUS_E_NOMEM;

	dp_rxdma_srng = &soc->rx_refill_buf_ring[mac_for_pdev];
	rxdma_entries = dp_rxdma_srng->num_entries;

	soc->process_rx_status = CONFIG_PROCESS_RX_STATUS;

	rx_sw_desc_num =
	wlan_cfg_get_dp_soc_rx_sw_desc_num(soc->wlan_cfg_ctx);

	rx_desc_pool->owner = DP_WBM2SW_RBM;
	rx_desc_pool->buf_size = RX_DATA_BUFFER_SIZE;
	rx_desc_pool->buf_alignment = RX_DATA_BUFFER_ALIGNMENT;
	/* Disable monitor dest processing via frag */
	dp_rx_enable_mon_dest_frag(rx_desc_pool, false);

	dp_rx_desc_pool_init(soc, mac_for_pdev,
			     rx_sw_desc_num, rx_desc_pool);
	return QDF_STATUS_SUCCESS;
}

/*
 * dp_rx_pdev_desc_pool_deinit() - de-initialize software rx descriptor pools
 * @pdev: core txrx pdev context
 *
 * This function resets the freelist of rx descriptors and destroys locks
 * associated with this list of descriptors.
 */
void dp_rx_pdev_desc_pool_deinit(struct dp_pdev *pdev)
{
	int mac_for_pdev = pdev->lmac_id;
	struct dp_soc *soc = pdev->soc;
	struct rx_desc_pool *rx_desc_pool;

	rx_desc_pool = &soc->rx_desc_buf[mac_for_pdev];

	dp_rx_desc_pool_deinit(soc, rx_desc_pool);
}

/*
 * dp_rx_pdev_buffers_alloc() - Allocate nbufs (skbs) and replenish RxDMA ring
 *
 * @pdev: core txrx pdev context
 *
 * Return: QDF_STATUS - QDF_STATUS_SUCCESS
 *			QDF_STATUS_E_NOMEM
 */
QDF_STATUS
dp_rx_pdev_buffers_alloc(struct dp_pdev *pdev)
{
	int mac_for_pdev = pdev->lmac_id;
	struct dp_soc *soc = pdev->soc;
	struct dp_srng *dp_rxdma_srng;
	struct rx_desc_pool *rx_desc_pool;
	uint32_t rxdma_entries;

	dp_rxdma_srng = &soc->rx_refill_buf_ring[mac_for_pdev];
	rxdma_entries = dp_rxdma_srng->num_entries;

	rx_desc_pool = &soc->rx_desc_buf[mac_for_pdev];

	/* Initialize RX buffer pool which will be
	 * used during low memory conditions
	 */
	dp_rx_buffer_pool_init(soc, mac_for_pdev);

	return dp_pdev_rx_buffers_attach(soc, mac_for_pdev, dp_rxdma_srng,
					 rx_desc_pool, rxdma_entries - 1);
}

/*
 * dp_rx_pdev_buffers_free - Free nbufs (skbs)
 *
 * @pdev: core txrx pdev context
 */
void
dp_rx_pdev_buffers_free(struct dp_pdev *pdev)
{
	int mac_for_pdev = pdev->lmac_id;
	struct dp_soc *soc = pdev->soc;
	struct rx_desc_pool *rx_desc_pool;

	rx_desc_pool = &soc->rx_desc_buf[mac_for_pdev];

	dp_rx_desc_nbuf_free(soc, rx_desc_pool);
	dp_rx_buffer_pool_deinit(soc, mac_for_pdev);
}

#ifdef DP_RX_SPECIAL_FRAME_NEED
bool dp_rx_deliver_special_frame(struct dp_soc *soc, struct dp_peer *peer,
				 qdf_nbuf_t nbuf, uint32_t frame_mask,
				 uint8_t *rx_tlv_hdr)
{
	uint32_t l2_hdr_offset = 0;
	uint16_t msdu_len = 0;
	uint32_t skip_len;

	l2_hdr_offset =
		hal_rx_msdu_end_l3_hdr_padding_get(soc->hal_soc, rx_tlv_hdr);

	if (qdf_unlikely(qdf_nbuf_is_frag(nbuf))) {
		skip_len = l2_hdr_offset;
	} else {
		msdu_len = QDF_NBUF_CB_RX_PKT_LEN(nbuf);
		skip_len = l2_hdr_offset + RX_PKT_TLVS_LEN;
		qdf_nbuf_set_pktlen(nbuf, msdu_len + skip_len);
	}

	QDF_NBUF_CB_RX_NUM_ELEMENTS_IN_LIST(nbuf) = 1;
	dp_rx_set_hdr_pad(nbuf, l2_hdr_offset);
	qdf_nbuf_pull_head(nbuf, skip_len);

	if (dp_rx_is_special_frame(nbuf, frame_mask)) {
		qdf_nbuf_set_exc_frame(nbuf, 1);
		dp_rx_deliver_to_stack(soc, peer->vdev, peer,
				       nbuf, NULL);
		return true;
	}

	return false;
}
#endif
