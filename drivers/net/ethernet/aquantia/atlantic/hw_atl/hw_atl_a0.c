/*
 * aQuantia Corporation Network Driver
 * Copyright (C) 2014-2017 aQuantia Corporation. All rights reserved
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 */

/* File hw_atl_a0.c: Definition of Atlantic hardware specific functions. */

#include "../aq_hw.h"
#include "../aq_hw_utils.h"
#include "../aq_ring.h"
#include "hw_atl_a0.h"
#include "hw_atl_utils.h"
#include "hw_atl_llh.h"
#include "hw_atl_a0_internal.h"

static int hw_atl_a0_get_hw_caps(struct aq_hw_s *self,
				 struct aq_hw_caps_s *aq_hw_caps,
				 unsigned short device,
				 unsigned short subsystem_device)
{
	memcpy(aq_hw_caps, &hw_atl_a0_hw_caps_, sizeof(*aq_hw_caps));

	if (device == HW_ATL_DEVICE_ID_D108 && subsystem_device == 0x0001)
		aq_hw_caps->link_speed_msk &= ~HW_ATL_A0_RATE_10G;

	if (device == HW_ATL_DEVICE_ID_D109 && subsystem_device == 0x0001) {
		aq_hw_caps->link_speed_msk &= ~HW_ATL_A0_RATE_10G;
		aq_hw_caps->link_speed_msk &= ~HW_ATL_A0_RATE_5G;
	}

	return 0;
}

static struct aq_hw_s *hw_atl_a0_create(struct aq_pci_func_s *aq_pci_func,
					unsigned int port,
					struct aq_hw_ops *ops)
{
	struct hw_atl_s *self = NULL;

	self = kzalloc(sizeof(*self), GFP_KERNEL);
	if (!self)
		goto err_exit;

	self->base.aq_pci_func = aq_pci_func;

	self->base.not_ff_addr = 0x10U;

err_exit:
	return (struct aq_hw_s *)self;
}

static void hw_atl_a0_destroy(struct aq_hw_s *self)
{
	kfree(self);
}

static int hw_atl_a0_hw_reset(struct aq_hw_s *self)
{
	int err = 0;

	glb_glb_reg_res_dis_set(self, 1U);
	pci_pci_reg_res_dis_set(self, 0U);
	rx_rx_reg_res_dis_set(self, 0U);
	tx_tx_reg_res_dis_set(self, 0U);

	HW_ATL_FLUSH();
	glb_soft_res_set(self, 1);

	/* check 10 times by 1ms */
	AQ_HW_WAIT_FOR(glb_soft_res_get(self) == 0, 1000U, 10U);
	if (err < 0)
		goto err_exit;

	itr_irq_reg_res_dis_set(self, 0U);
	itr_res_irq_set(self, 1U);

	/* check 10 times by 1ms */
	AQ_HW_WAIT_FOR(itr_res_irq_get(self) == 0, 1000U, 10U);
	if (err < 0)
		goto err_exit;

	hw_atl_utils_mpi_set(self, MPI_RESET, 0x0U);

	err = aq_hw_err_from_flags(self);

err_exit:
	return err;
}

static int hw_atl_a0_hw_qos_set(struct aq_hw_s *self)
{
	u32 tc = 0U;
	u32 buff_size = 0U;
	unsigned int i_priority = 0U;
	bool is_rx_flow_control = false;

	/* TPS Descriptor rate init */
	tps_tx_pkt_shed_desc_rate_curr_time_res_set(self, 0x0U);
	tps_tx_pkt_shed_desc_rate_lim_set(self, 0xA);

	/* TPS VM init */
	tps_tx_pkt_shed_desc_vm_arb_mode_set(self, 0U);

	/* TPS TC credits init */
	tps_tx_pkt_shed_desc_tc_arb_mode_set(self, 0U);
	tps_tx_pkt_shed_data_arb_mode_set(self, 0U);

	tps_tx_pkt_shed_tc_data_max_credit_set(self, 0xFFF, 0U);
	tps_tx_pkt_shed_tc_data_weight_set(self, 0x64, 0U);
	tps_tx_pkt_shed_desc_tc_max_credit_set(self, 0x50, 0U);
	tps_tx_pkt_shed_desc_tc_weight_set(self, 0x1E, 0U);

	/* Tx buf size */
	buff_size = HW_ATL_A0_TXBUF_MAX;

	tpb_tx_pkt_buff_size_per_tc_set(self, buff_size, tc);
	tpb_tx_buff_hi_threshold_per_tc_set(self,
					    (buff_size * (1024 / 32U) * 66U) /
					    100U, tc);
	tpb_tx_buff_lo_threshold_per_tc_set(self,
					    (buff_size * (1024 / 32U) * 50U) /
					    100U, tc);

	/* QoS Rx buf size per TC */
	tc = 0;
	is_rx_flow_control = (AQ_NIC_FC_RX & self->aq_nic_cfg->flow_control);
	buff_size = HW_ATL_A0_RXBUF_MAX;

	rpb_rx_pkt_buff_size_per_tc_set(self, buff_size, tc);
	rpb_rx_buff_hi_threshold_per_tc_set(self,
					    (buff_size *
					    (1024U / 32U) * 66U) /
					    100U, tc);
	rpb_rx_buff_lo_threshold_per_tc_set(self,
					    (buff_size *
					    (1024U / 32U) * 50U) /
					    100U, tc);
	rpb_rx_xoff_en_per_tc_set(self, is_rx_flow_control ? 1U : 0U, tc);

	/* QoS 802.1p priority -> TC mapping */
	for (i_priority = 8U; i_priority--;)
		rpf_rpb_user_priority_tc_map_set(self, i_priority, 0U);

	return aq_hw_err_from_flags(self);
}

static int hw_atl_a0_hw_rss_hash_set(struct aq_hw_s *self,
				     struct aq_rss_parameters *rss_params)
{
	struct aq_nic_cfg_s *cfg = NULL;
	int err = 0;
	unsigned int i = 0U;
	unsigned int addr = 0U;

	cfg = self->aq_nic_cfg;

	for (i = 10, addr = 0U; i--; ++addr) {
		u32 key_data = cfg->is_rss ?
			__swab32(rss_params->hash_secret_key[i]) : 0U;
		rpf_rss_key_wr_data_set(self, key_data);
		rpf_rss_key_addr_set(self, addr);
		rpf_rss_key_wr_en_set(self, 1U);
		AQ_HW_WAIT_FOR(rpf_rss_key_wr_en_get(self) == 0, 1000U, 10U);
		if (err < 0)
			goto err_exit;
	}

	err = aq_hw_err_from_flags(self);

err_exit:
	return err;
}

static int hw_atl_a0_hw_rss_set(struct aq_hw_s *self,
				struct aq_rss_parameters *rss_params)
{
	u8 *indirection_table =	rss_params->indirection_table;
	u32 i = 0U;
	u32 num_rss_queues = max(1U, self->aq_nic_cfg->num_rss_queues);
	int err = 0;
	u16 bitary[1 + (HW_ATL_A0_RSS_REDIRECTION_MAX *
		   HW_ATL_A0_RSS_REDIRECTION_BITS / 16U)];

	memset(bitary, 0, sizeof(bitary));

	for (i = HW_ATL_A0_RSS_REDIRECTION_MAX; i--; ) {
		(*(u32 *)(bitary + ((i * 3U) / 16U))) |=
			((indirection_table[i] % num_rss_queues) <<
			((i * 3U) & 0xFU));
	}

	for (i = AQ_DIMOF(bitary); i--;) {
		rpf_rss_redir_tbl_wr_data_set(self, bitary[i]);
		rpf_rss_redir_tbl_addr_set(self, i);
		rpf_rss_redir_wr_en_set(self, 1U);
		AQ_HW_WAIT_FOR(rpf_rss_redir_wr_en_get(self) == 0, 1000U, 10U);
		if (err < 0)
			goto err_exit;
	}

	err = aq_hw_err_from_flags(self);

err_exit:
	return err;
}

static int hw_atl_a0_hw_offload_set(struct aq_hw_s *self,
				    struct aq_nic_cfg_s *aq_nic_cfg)
{
	/* TX checksums offloads*/
	tpo_ipv4header_crc_offload_en_set(self, 1);
	tpo_tcp_udp_crc_offload_en_set(self, 1);

	/* RX checksums offloads*/
	rpo_ipv4header_crc_offload_en_set(self, 1);
	rpo_tcp_udp_crc_offload_en_set(self, 1);

	/* LSO offloads*/
	tdm_large_send_offload_en_set(self, 0xFFFFFFFFU);

	return aq_hw_err_from_flags(self);
}

static int hw_atl_a0_hw_init_tx_path(struct aq_hw_s *self)
{
	thm_lso_tcp_flag_of_first_pkt_set(self, 0x0FF6U);
	thm_lso_tcp_flag_of_middle_pkt_set(self, 0x0FF6U);
	thm_lso_tcp_flag_of_last_pkt_set(self, 0x0F7FU);

	/* Tx interrupts */
	tdm_tx_desc_wr_wb_irq_en_set(self, 1U);

	/* misc */
	aq_hw_write_reg(self, 0x00007040U, IS_CHIP_FEATURE(TPO2) ?
			0x00010000U : 0x00000000U);
	tdm_tx_dca_en_set(self, 0U);
	tdm_tx_dca_mode_set(self, 0U);

	tpb_tx_path_scp_ins_en_set(self, 1U);

	return aq_hw_err_from_flags(self);
}

static int hw_atl_a0_hw_init_rx_path(struct aq_hw_s *self)
{
	struct aq_nic_cfg_s *cfg = self->aq_nic_cfg;
	int i;

	/* Rx TC/RSS number config */
	rpb_rpf_rx_traf_class_mode_set(self, 1U);

	/* Rx flow control */
	rpb_rx_flow_ctl_mode_set(self, 1U);

	/* RSS Ring selection */
	reg_rx_flr_rss_control1set(self, cfg->is_rss ?
					0xB3333333U : 0x00000000U);

	/* Multicast filters */
	for (i = HW_ATL_A0_MAC_MAX; i--;) {
		rpfl2_uc_flr_en_set(self, (i == 0U) ? 1U : 0U, i);
		rpfl2unicast_flr_act_set(self, 1U, i);
	}

	reg_rx_flr_mcst_flr_msk_set(self, 0x00000000U);
	reg_rx_flr_mcst_flr_set(self, 0x00010FFFU, 0U);

	/* Vlan filters */
	rpf_vlan_outer_etht_set(self, 0x88A8U);
	rpf_vlan_inner_etht_set(self, 0x8100U);
	rpf_vlan_prom_mode_en_set(self, 1);

	/* Rx Interrupts */
	rdm_rx_desc_wr_wb_irq_en_set(self, 1U);

	/* misc */
	rpfl2broadcast_flr_act_set(self, 1U);
	rpfl2broadcast_count_threshold_set(self, 0xFFFFU & (~0U / 256U));

	rdm_rx_dca_en_set(self, 0U);
	rdm_rx_dca_mode_set(self, 0U);

	return aq_hw_err_from_flags(self);
}

static int hw_atl_a0_hw_mac_addr_set(struct aq_hw_s *self, u8 *mac_addr)
{
	int err = 0;
	unsigned int h = 0U;
	unsigned int l = 0U;

	if (!mac_addr) {
		err = -EINVAL;
		goto err_exit;
	}
	h = (mac_addr[0] << 8) | (mac_addr[1]);
	l = (mac_addr[2] << 24) | (mac_addr[3] << 16) |
		(mac_addr[4] << 8) | mac_addr[5];

	rpfl2_uc_flr_en_set(self, 0U, HW_ATL_A0_MAC);
	rpfl2unicast_dest_addresslsw_set(self, l, HW_ATL_A0_MAC);
	rpfl2unicast_dest_addressmsw_set(self, h, HW_ATL_A0_MAC);
	rpfl2_uc_flr_en_set(self, 1U, HW_ATL_A0_MAC);

	err = aq_hw_err_from_flags(self);

err_exit:
	return err;
}

static int hw_atl_a0_hw_init(struct aq_hw_s *self,
			     struct aq_nic_cfg_s *aq_nic_cfg,
			     u8 *mac_addr)
{
	static u32 aq_hw_atl_igcr_table_[4][2] = {
		{ 0x20000000U, 0x20000000U }, /* AQ_IRQ_INVALID */
		{ 0x20000080U, 0x20000080U }, /* AQ_IRQ_LEGACY */
		{ 0x20000021U, 0x20000025U }, /* AQ_IRQ_MSI */
		{ 0x20000022U, 0x20000026U }  /* AQ_IRQ_MSIX */
	};

	int err = 0;

	self->aq_nic_cfg = aq_nic_cfg;

	hw_atl_utils_hw_chip_features_init(self,
					   &PHAL_ATLANTIC_A0->chip_features);

	hw_atl_a0_hw_init_tx_path(self);
	hw_atl_a0_hw_init_rx_path(self);

	hw_atl_a0_hw_mac_addr_set(self, mac_addr);

	hw_atl_utils_mpi_set(self, MPI_INIT, aq_nic_cfg->link_speed_msk);

	reg_tx_dma_debug_ctl_set(self, 0x800000b8U);
	reg_tx_dma_debug_ctl_set(self, 0x000000b8U);

	hw_atl_a0_hw_qos_set(self);
	hw_atl_a0_hw_rss_set(self, &aq_nic_cfg->aq_rss);
	hw_atl_a0_hw_rss_hash_set(self, &aq_nic_cfg->aq_rss);

	err = aq_hw_err_from_flags(self);
	if (err < 0)
		goto err_exit;

	/* Interrupts */
	reg_irq_glb_ctl_set(self,
			    aq_hw_atl_igcr_table_[aq_nic_cfg->irq_type]
						 [(aq_nic_cfg->vecs > 1U) ?
						 1 : 0]);

	itr_irq_auto_masklsw_set(self, aq_nic_cfg->aq_hw_caps->irq_mask);

	/* Interrupts */
	reg_gen_irq_map_set(self,
			    ((HW_ATL_A0_ERR_INT << 0x18) | (1U << 0x1F)) |
			    ((HW_ATL_A0_ERR_INT << 0x10) | (1U << 0x17)) |
			    ((HW_ATL_A0_ERR_INT << 8) | (1U << 0xF)) |
			    ((HW_ATL_A0_ERR_INT) | (1U << 0x7)), 0U);

	hw_atl_a0_hw_offload_set(self, aq_nic_cfg);

err_exit:
	return err;
}

static int hw_atl_a0_hw_ring_tx_start(struct aq_hw_s *self,
				      struct aq_ring_s *ring)
{
	tdm_tx_desc_en_set(self, 1, ring->idx);
	return aq_hw_err_from_flags(self);
}

static int hw_atl_a0_hw_ring_rx_start(struct aq_hw_s *self,
				      struct aq_ring_s *ring)
{
	rdm_rx_desc_en_set(self, 1, ring->idx);
	return aq_hw_err_from_flags(self);
}

static int hw_atl_a0_hw_start(struct aq_hw_s *self)
{
	tpb_tx_buff_en_set(self, 1);
	rpb_rx_buff_en_set(self, 1);
	return aq_hw_err_from_flags(self);
}

static int hw_atl_a0_hw_tx_ring_tail_update(struct aq_hw_s *self,
					    struct aq_ring_s *ring)
{
	reg_tx_dma_desc_tail_ptr_set(self, ring->sw_tail, ring->idx);
	return 0;
}

static int hw_atl_a0_hw_ring_tx_xmit(struct aq_hw_s *self,
				     struct aq_ring_s *ring,
				     unsigned int frags)
{
	struct aq_ring_buff_s *buff = NULL;
	struct hw_atl_txd_s *txd = NULL;
	unsigned int buff_pa_len = 0U;
	unsigned int pkt_len = 0U;
	unsigned int frag_count = 0U;
	bool is_gso = false;

	buff = &ring->buff_ring[ring->sw_tail];
	pkt_len = (buff->is_eop && buff->is_sop) ? buff->len : buff->len_pkt;

	for (frag_count = 0; frag_count < frags; frag_count++) {
		txd = (struct hw_atl_txd_s *)&ring->dx_ring[ring->sw_tail *
						HW_ATL_A0_TXD_SIZE];
		txd->ctl = 0;
		txd->ctl2 = 0;
		txd->buf_addr = 0;

		buff = &ring->buff_ring[ring->sw_tail];

		if (buff->is_txc) {
			txd->ctl |= (buff->len_l3 << 31) |
				(buff->len_l2 << 24) |
				HW_ATL_A0_TXD_CTL_CMD_TCP |
				HW_ATL_A0_TXD_CTL_DESC_TYPE_TXC;
			txd->ctl2 |= (buff->mss << 16) |
				(buff->len_l4 << 8) |
				(buff->len_l3 >> 1);

			pkt_len -= (buff->len_l4 +
				    buff->len_l3 +
				    buff->len_l2);
			is_gso = true;

			if (buff->is_ipv6)
				txd->ctl |= HW_ATL_A0_TXD_CTL_CMD_IPV6;
		} else {
			buff_pa_len = buff->len;

			txd->buf_addr = buff->pa;
			txd->ctl |= (HW_ATL_A0_TXD_CTL_BLEN &
						((u32)buff_pa_len << 4));
			txd->ctl |= HW_ATL_A0_TXD_CTL_DESC_TYPE_TXD;
			/* PAY_LEN */
			txd->ctl2 |= HW_ATL_A0_TXD_CTL2_LEN & (pkt_len << 14);

			if (is_gso) {
				txd->ctl |= HW_ATL_A0_TXD_CTL_CMD_LSO;
				txd->ctl2 |= HW_ATL_A0_TXD_CTL2_CTX_EN;
			}

			/* Tx checksum offloads */
			if (buff->is_ip_cso)
				txd->ctl |= HW_ATL_A0_TXD_CTL_CMD_IPCSO;

			if (buff->is_udp_cso || buff->is_tcp_cso)
				txd->ctl |= HW_ATL_A0_TXD_CTL_CMD_TUCSO;

			if (unlikely(buff->is_eop)) {
				txd->ctl |= HW_ATL_A0_TXD_CTL_EOP;
				txd->ctl |= HW_ATL_A0_TXD_CTL_CMD_WB;
				is_gso = false;
			}
		}

		ring->sw_tail = aq_ring_next_dx(ring, ring->sw_tail);
	}

	hw_atl_a0_hw_tx_ring_tail_update(self, ring);
	return aq_hw_err_from_flags(self);
}

static int hw_atl_a0_hw_ring_rx_init(struct aq_hw_s *self,
				     struct aq_ring_s *aq_ring,
				     struct aq_ring_param_s *aq_ring_param)
{
	u32 dma_desc_addr_lsw = (u32)aq_ring->dx_ring_pa;
	u32 dma_desc_addr_msw = (u32)(((u64)aq_ring->dx_ring_pa) >> 32);

	rdm_rx_desc_en_set(self, false, aq_ring->idx);

	rdm_rx_desc_head_splitting_set(self, 0U, aq_ring->idx);

	reg_rx_dma_desc_base_addresslswset(self, dma_desc_addr_lsw,
					   aq_ring->idx);

	reg_rx_dma_desc_base_addressmswset(self,
					   dma_desc_addr_msw, aq_ring->idx);

	rdm_rx_desc_len_set(self, aq_ring->size / 8U, aq_ring->idx);

	rdm_rx_desc_data_buff_size_set(self,
				       AQ_CFG_RX_FRAME_MAX / 1024U,
				       aq_ring->idx);

	rdm_rx_desc_head_buff_size_set(self, 0U, aq_ring->idx);
	rdm_rx_desc_head_splitting_set(self, 0U, aq_ring->idx);
	rpo_rx_desc_vlan_stripping_set(self, 0U, aq_ring->idx);

	/* Rx ring set mode */

	/* Mapping interrupt vector */
	itr_irq_map_rx_set(self, aq_ring_param->vec_idx, aq_ring->idx);
	itr_irq_map_en_rx_set(self, true, aq_ring->idx);

	rdm_cpu_id_set(self, aq_ring_param->cpu, aq_ring->idx);
	rdm_rx_desc_dca_en_set(self, 0U, aq_ring->idx);
	rdm_rx_head_dca_en_set(self, 0U, aq_ring->idx);
	rdm_rx_pld_dca_en_set(self, 0U, aq_ring->idx);

	return aq_hw_err_from_flags(self);
}

static int hw_atl_a0_hw_ring_tx_init(struct aq_hw_s *self,
				     struct aq_ring_s *aq_ring,
				     struct aq_ring_param_s *aq_ring_param)
{
	u32 dma_desc_lsw_addr = (u32)aq_ring->dx_ring_pa;
	u32 dma_desc_msw_addr = (u32)(((u64)aq_ring->dx_ring_pa) >> 32);

	reg_tx_dma_desc_base_addresslswset(self, dma_desc_lsw_addr,
					   aq_ring->idx);

	reg_tx_dma_desc_base_addressmswset(self, dma_desc_msw_addr,
					   aq_ring->idx);

	tdm_tx_desc_len_set(self, aq_ring->size / 8U, aq_ring->idx);

	hw_atl_a0_hw_tx_ring_tail_update(self, aq_ring);

	/* Set Tx threshold */
	tdm_tx_desc_wr_wb_threshold_set(self, 0U, aq_ring->idx);

	/* Mapping interrupt vector */
	itr_irq_map_tx_set(self, aq_ring_param->vec_idx, aq_ring->idx);
	itr_irq_map_en_tx_set(self, true, aq_ring->idx);

	tdm_cpu_id_set(self, aq_ring_param->cpu, aq_ring->idx);
	tdm_tx_desc_dca_en_set(self, 0U, aq_ring->idx);

	return aq_hw_err_from_flags(self);
}

static int hw_atl_a0_hw_ring_rx_fill(struct aq_hw_s *self,
				     struct aq_ring_s *ring,
				     unsigned int sw_tail_old)
{
	for (; sw_tail_old != ring->sw_tail;
		sw_tail_old = aq_ring_next_dx(ring, sw_tail_old)) {
		struct hw_atl_rxd_s *rxd =
			(struct hw_atl_rxd_s *)&ring->dx_ring[sw_tail_old *
							HW_ATL_A0_RXD_SIZE];

		struct aq_ring_buff_s *buff = &ring->buff_ring[sw_tail_old];

		rxd->buf_addr = buff->pa;
		rxd->hdr_addr = 0U;
	}

	reg_rx_dma_desc_tail_ptr_set(self, sw_tail_old, ring->idx);

	return aq_hw_err_from_flags(self);
}

static int hw_atl_a0_hw_ring_tx_head_update(struct aq_hw_s *self,
					    struct aq_ring_s *ring)
{
	int err = 0;
	unsigned int hw_head_ = tdm_tx_desc_head_ptr_get(self, ring->idx);

	if (aq_utils_obj_test(&self->header.flags, AQ_HW_FLAG_ERR_UNPLUG)) {
		err = -ENXIO;
		goto err_exit;
	}
	ring->hw_head = hw_head_;
	err = aq_hw_err_from_flags(self);

err_exit:
	return err;
}

static int hw_atl_a0_hw_ring_rx_receive(struct aq_hw_s *self,
					struct aq_ring_s *ring)
{
	struct device *ndev = aq_nic_get_dev(ring->aq_nic);

	for (; ring->hw_head != ring->sw_tail;
		ring->hw_head = aq_ring_next_dx(ring, ring->hw_head)) {
		struct aq_ring_buff_s *buff = NULL;
		struct hw_atl_rxd_wb_s *rxd_wb = (struct hw_atl_rxd_wb_s *)
			&ring->dx_ring[ring->hw_head * HW_ATL_A0_RXD_SIZE];

		unsigned int is_err = 1U;
		unsigned int is_rx_check_sum_enabled = 0U;
		unsigned int pkt_type = 0U;

		if (!(rxd_wb->status & 0x5U)) { /* RxD is not done */
			if ((1U << 4) &
				reg_rx_dma_desc_status_get(self, ring->idx)) {
			rdm_rx_desc_en_set(self, false, ring->idx);
			rdm_rx_desc_res_set(self, true, ring->idx);
			rdm_rx_desc_res_set(self, false, ring->idx);
			rdm_rx_desc_en_set(self, true, ring->idx);
			}

			if (ring->hw_head ||
			    (rdm_rx_desc_head_ptr_get(self, ring->idx) < 2U)) {
				break;
			} else if (!(rxd_wb->status & 0x1U)) {
				struct hw_atl_rxd_wb_s *rxd_wb1 =
					(struct hw_atl_rxd_wb_s *)
					(&ring->dx_ring[(1U) *
						HW_ATL_A0_RXD_SIZE]);

				if ((rxd_wb1->status & 0x1U)) {
					rxd_wb->pkt_len = 1514U;
					rxd_wb->status = 3U;
				} else {
					break;
				}
			}
		}

		buff = &ring->buff_ring[ring->hw_head];

		if (0x3U != (rxd_wb->status & 0x3U))
			rxd_wb->status |= 4;

		is_err = (0x0000001CU & rxd_wb->status);
		is_rx_check_sum_enabled = (rxd_wb->type) & (0x3U << 19);
		pkt_type = 0xFFU & (rxd_wb->type >> 4);

		if (is_rx_check_sum_enabled) {
			if (0x0U == (pkt_type & 0x3U))
				buff->is_ip_cso = (is_err & 0x08U) ? 0 : 1;

			if (0x4U == (pkt_type & 0x1CU))
				buff->is_udp_cso = (is_err & 0x10U) ? 0 : 1;
			else if (0x0U == (pkt_type & 0x1CU))
				buff->is_tcp_cso = (is_err & 0x10U) ? 0 : 1;

			/* Checksum offload workaround for small packets */
			if (rxd_wb->pkt_len <= 60) {
				buff->is_ip_cso = 0U;
				buff->is_cso_err = 0U;
			}
		}

		is_err &= ~0x18U;
		is_err &= ~0x04U;

		dma_unmap_page(ndev, buff->pa, buff->len, DMA_FROM_DEVICE);

		if (is_err || rxd_wb->type & 0x1000U) {
			/* status error or DMA error */
			buff->is_error = 1U;
		} else {
			if (self->aq_nic_cfg->is_rss) {
				/* last 4 byte */
				u16 rss_type = rxd_wb->type & 0xFU;

				if (rss_type && rss_type < 0x8U) {
					buff->is_hash_l4 = (rss_type == 0x4 ||
							rss_type == 0x5);
					buff->rss_hash = rxd_wb->rss_hash;
				}
			}

			if (HW_ATL_A0_RXD_WB_STAT2_EOP & rxd_wb->status) {
				buff->len = rxd_wb->pkt_len %
					AQ_CFG_RX_FRAME_MAX;
				buff->len = buff->len ?
					buff->len : AQ_CFG_RX_FRAME_MAX;
				buff->next = 0U;
				buff->is_eop = 1U;
			} else {
				/* jumbo */
				buff->next = aq_ring_next_dx(ring,
							     ring->hw_head);
				++ring->stats.rx.jumbo_packets;
			}
		}
	}

	return aq_hw_err_from_flags(self);
}

static int hw_atl_a0_hw_irq_enable(struct aq_hw_s *self, u64 mask)
{
	itr_irq_msk_setlsw_set(self, LODWORD(mask) |
			       (1U << HW_ATL_A0_ERR_INT));
	return aq_hw_err_from_flags(self);
}

static int hw_atl_a0_hw_irq_disable(struct aq_hw_s *self, u64 mask)
{
	itr_irq_msk_clearlsw_set(self, LODWORD(mask));
	itr_irq_status_clearlsw_set(self, LODWORD(mask));

	if ((1U << 16) & reg_gen_irq_status_get(self))

		atomic_inc(&PHAL_ATLANTIC_A0->dpc);

	return aq_hw_err_from_flags(self);
}

static int hw_atl_a0_hw_irq_read(struct aq_hw_s *self, u64 *mask)
{
	*mask = itr_irq_statuslsw_get(self);
	return aq_hw_err_from_flags(self);
}

#define IS_FILTER_ENABLED(_F_) ((packet_filter & (_F_)) ? 1U : 0U)

static int hw_atl_a0_hw_packet_filter_set(struct aq_hw_s *self,
					  unsigned int packet_filter)
{
	unsigned int i = 0U;

	rpfl2promiscuous_mode_en_set(self, IS_FILTER_ENABLED(IFF_PROMISC));
	rpfl2multicast_flr_en_set(self, IS_FILTER_ENABLED(IFF_MULTICAST), 0);
	rpfl2broadcast_en_set(self, IS_FILTER_ENABLED(IFF_BROADCAST));

	self->aq_nic_cfg->is_mc_list_enabled =
			IS_FILTER_ENABLED(IFF_MULTICAST);

	for (i = HW_ATL_A0_MAC_MIN; i < HW_ATL_A0_MAC_MAX; ++i)
		rpfl2_uc_flr_en_set(self,
				    (self->aq_nic_cfg->is_mc_list_enabled &&
				    (i <= self->aq_nic_cfg->mc_list_count)) ?
				    1U : 0U, i);

	return aq_hw_err_from_flags(self);
}

#undef IS_FILTER_ENABLED

static int hw_atl_a0_hw_multicast_list_set(struct aq_hw_s *self,
					   u8 ar_mac
					   [AQ_CFG_MULTICAST_ADDRESS_MAX]
					   [ETH_ALEN],
					   u32 count)
{
	int err = 0;

	if (count > (HW_ATL_A0_MAC_MAX - HW_ATL_A0_MAC_MIN)) {
		err = -EBADRQC;
		goto err_exit;
	}
	for (self->aq_nic_cfg->mc_list_count = 0U;
			self->aq_nic_cfg->mc_list_count < count;
			++self->aq_nic_cfg->mc_list_count) {
		u32 i = self->aq_nic_cfg->mc_list_count;
		u32 h = (ar_mac[i][0] << 8) | (ar_mac[i][1]);
		u32 l = (ar_mac[i][2] << 24) | (ar_mac[i][3] << 16) |
					(ar_mac[i][4] << 8) | ar_mac[i][5];

		rpfl2_uc_flr_en_set(self, 0U, HW_ATL_A0_MAC_MIN + i);

		rpfl2unicast_dest_addresslsw_set(self,
						 l, HW_ATL_A0_MAC_MIN + i);

		rpfl2unicast_dest_addressmsw_set(self,
						 h, HW_ATL_A0_MAC_MIN + i);

		rpfl2_uc_flr_en_set(self,
				    (self->aq_nic_cfg->is_mc_list_enabled),
				    HW_ATL_A0_MAC_MIN + i);
	}

	err = aq_hw_err_from_flags(self);

err_exit:
	return err;
}

static int hw_atl_a0_hw_interrupt_moderation_set(struct aq_hw_s *self)
{
	unsigned int i = 0U;
	u32 itr_rx;

	if (self->aq_nic_cfg->itr) {
		if (self->aq_nic_cfg->itr != AQ_CFG_INTERRUPT_MODERATION_AUTO) {
			u32 itr_ = (self->aq_nic_cfg->itr >> 1);

			itr_ = min(AQ_CFG_IRQ_MASK, itr_);

			itr_rx = 0x80000000U | (itr_ << 0x10);
		} else  {
			u32 n = 0xFFFFU & aq_hw_read_reg(self, 0x00002A00U);

			if (n < self->aq_link_status.mbps) {
				itr_rx = 0U;
			} else {
				static unsigned int hw_timers_tbl_[] = {
					0x01CU, /* 10Gbit */
					0x039U, /* 5Gbit */
					0x039U, /* 5Gbit 5GS */
					0x073U, /* 2.5Gbit */
					0x120U, /* 1Gbit */
					0x1FFU, /* 100Mbit */
				};

				unsigned int speed_index =
					hw_atl_utils_mbps_2_speed_index(
						self->aq_link_status.mbps);

				itr_rx = 0x80000000U |
					(hw_timers_tbl_[speed_index] << 0x10U);
			}

			aq_hw_write_reg(self, 0x00002A00U, 0x40000000U);
			aq_hw_write_reg(self, 0x00002A00U, 0x8D000000U);
		}
	} else {
		itr_rx = 0U;
	}

	for (i = HW_ATL_A0_RINGS_MAX; i--;)
		reg_irq_thr_set(self, itr_rx, i);

	return aq_hw_err_from_flags(self);
}

static int hw_atl_a0_hw_stop(struct aq_hw_s *self)
{
	hw_atl_a0_hw_irq_disable(self, HW_ATL_A0_INT_MASK);
	return aq_hw_err_from_flags(self);
}

static int hw_atl_a0_hw_ring_tx_stop(struct aq_hw_s *self,
				     struct aq_ring_s *ring)
{
	tdm_tx_desc_en_set(self, 0U, ring->idx);
	return aq_hw_err_from_flags(self);
}

static int hw_atl_a0_hw_ring_rx_stop(struct aq_hw_s *self,
				     struct aq_ring_s *ring)
{
	rdm_rx_desc_en_set(self, 0U, ring->idx);
	return aq_hw_err_from_flags(self);
}

static int hw_atl_a0_hw_set_speed(struct aq_hw_s *self, u32 speed)
{
	int err = 0;

	err = hw_atl_utils_mpi_set_speed(self, speed, MPI_INIT);
	if (err < 0)
		goto err_exit;

err_exit:
	return err;
}

static struct aq_hw_ops hw_atl_ops_ = {
	.create               = hw_atl_a0_create,
	.destroy              = hw_atl_a0_destroy,
	.get_hw_caps          = hw_atl_a0_get_hw_caps,

	.hw_get_mac_permanent = hw_atl_utils_get_mac_permanent,
	.hw_set_mac_address   = hw_atl_a0_hw_mac_addr_set,
	.hw_get_link_status   = hw_atl_utils_mpi_get_link_status,
	.hw_set_link_speed    = hw_atl_a0_hw_set_speed,
	.hw_init              = hw_atl_a0_hw_init,
	.hw_deinit            = hw_atl_utils_hw_deinit,
	.hw_set_power         = hw_atl_utils_hw_set_power,
	.hw_reset             = hw_atl_a0_hw_reset,
	.hw_start             = hw_atl_a0_hw_start,
	.hw_ring_tx_start     = hw_atl_a0_hw_ring_tx_start,
	.hw_ring_tx_stop      = hw_atl_a0_hw_ring_tx_stop,
	.hw_ring_rx_start     = hw_atl_a0_hw_ring_rx_start,
	.hw_ring_rx_stop      = hw_atl_a0_hw_ring_rx_stop,
	.hw_stop              = hw_atl_a0_hw_stop,

	.hw_ring_tx_xmit         = hw_atl_a0_hw_ring_tx_xmit,
	.hw_ring_tx_head_update  = hw_atl_a0_hw_ring_tx_head_update,

	.hw_ring_rx_receive      = hw_atl_a0_hw_ring_rx_receive,
	.hw_ring_rx_fill         = hw_atl_a0_hw_ring_rx_fill,

	.hw_irq_enable           = hw_atl_a0_hw_irq_enable,
	.hw_irq_disable          = hw_atl_a0_hw_irq_disable,
	.hw_irq_read             = hw_atl_a0_hw_irq_read,

	.hw_ring_rx_init             = hw_atl_a0_hw_ring_rx_init,
	.hw_ring_tx_init             = hw_atl_a0_hw_ring_tx_init,
	.hw_packet_filter_set        = hw_atl_a0_hw_packet_filter_set,
	.hw_multicast_list_set       = hw_atl_a0_hw_multicast_list_set,
	.hw_interrupt_moderation_set = hw_atl_a0_hw_interrupt_moderation_set,
	.hw_rss_set                  = hw_atl_a0_hw_rss_set,
	.hw_rss_hash_set             = hw_atl_a0_hw_rss_hash_set,
	.hw_get_regs                 = hw_atl_utils_hw_get_regs,
	.hw_update_stats             = hw_atl_utils_update_stats,
	.hw_get_hw_stats             = hw_atl_utils_get_hw_stats,
	.hw_get_fw_version           = hw_atl_utils_get_fw_version,
};

struct aq_hw_ops *hw_atl_a0_get_ops_by_id(struct pci_dev *pdev)
{
	bool is_vid_ok = (pdev->vendor == PCI_VENDOR_ID_AQUANTIA);
	bool is_did_ok = ((pdev->device == HW_ATL_DEVICE_ID_0001) ||
			(pdev->device == HW_ATL_DEVICE_ID_D100) ||
			(pdev->device == HW_ATL_DEVICE_ID_D107) ||
			(pdev->device == HW_ATL_DEVICE_ID_D108) ||
			(pdev->device == HW_ATL_DEVICE_ID_D109));

	bool is_rev_ok = (pdev->revision == 1U);

	return (is_vid_ok && is_did_ok && is_rev_ok) ? &hw_atl_ops_ : NULL;
}
