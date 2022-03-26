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
#ifndef _HAL_GENERIC_API_H_
#define _HAL_GENERIC_API_H_

#include <hal_rx.h>

/**
 * hal_tx_comp_get_status() - TQM Release reason
 * @hal_desc: completion ring Tx status
 *
 * This function will parse the WBM completion descriptor and populate in
 * HAL structure
 *
 * Return: none
 */
static inline
void hal_tx_comp_get_status_generic(void *desc,
				    void *ts1,
				    struct hal_soc *hal)
{
	uint8_t rate_stats_valid = 0;
	uint32_t rate_stats = 0;
	struct hal_tx_completion_status *ts =
		(struct hal_tx_completion_status *)ts1;

	ts->ppdu_id = HAL_TX_DESC_GET(desc, WBM_RELEASE_RING_3,
			TQM_STATUS_NUMBER);
	ts->ack_frame_rssi = HAL_TX_DESC_GET(desc, WBM_RELEASE_RING_4,
			ACK_FRAME_RSSI);
	ts->first_msdu = HAL_TX_DESC_GET(desc, WBM_RELEASE_RING_4, FIRST_MSDU);
	ts->last_msdu = HAL_TX_DESC_GET(desc, WBM_RELEASE_RING_4, LAST_MSDU);
	ts->msdu_part_of_amsdu = HAL_TX_DESC_GET(desc, WBM_RELEASE_RING_4,
			MSDU_PART_OF_AMSDU);

	ts->peer_id = HAL_TX_DESC_GET(desc, WBM_RELEASE_RING_7, SW_PEER_ID);
	ts->tid = HAL_TX_DESC_GET(desc, WBM_RELEASE_RING_7, TID);
	ts->transmit_cnt = HAL_TX_DESC_GET(desc, WBM_RELEASE_RING_3,
			TRANSMIT_COUNT);

	rate_stats = HAL_TX_DESC_GET(desc, WBM_RELEASE_RING_5,
			TX_RATE_STATS);

	rate_stats_valid = HAL_TX_MS(TX_RATE_STATS_INFO_0,
			TX_RATE_STATS_INFO_VALID, rate_stats);

	ts->valid = rate_stats_valid;

	if (rate_stats_valid) {
		ts->bw = HAL_TX_MS(TX_RATE_STATS_INFO_0, TRANSMIT_BW,
				rate_stats);
		ts->pkt_type = HAL_TX_MS(TX_RATE_STATS_INFO_0,
				TRANSMIT_PKT_TYPE, rate_stats);
		ts->stbc = HAL_TX_MS(TX_RATE_STATS_INFO_0,
				TRANSMIT_STBC, rate_stats);
		ts->ldpc = HAL_TX_MS(TX_RATE_STATS_INFO_0, TRANSMIT_LDPC,
				rate_stats);
		ts->sgi = HAL_TX_MS(TX_RATE_STATS_INFO_0, TRANSMIT_SGI,
				rate_stats);
		ts->mcs = HAL_TX_MS(TX_RATE_STATS_INFO_0, TRANSMIT_MCS,
				rate_stats);
		ts->ofdma = HAL_TX_MS(TX_RATE_STATS_INFO_0, OFDMA_TRANSMISSION,
				rate_stats);
		ts->tones_in_ru = HAL_TX_MS(TX_RATE_STATS_INFO_0, TONES_IN_RU,
				rate_stats);
	}

	ts->release_src = hal_tx_comp_get_buffer_source(desc);
	ts->status = hal_tx_comp_get_release_reason(
					desc,
					hal_soc_to_hal_soc_handle(hal));

	ts->tsf = HAL_TX_DESC_GET(desc, UNIFIED_WBM_RELEASE_RING_6,
			TX_RATE_STATS_INFO_TX_RATE_STATS);
}

/**
 * hal_tx_desc_set_buf_addr - Fill Buffer Address information in Tx Descriptor
 * @desc: Handle to Tx Descriptor
 * @paddr: Physical Address
 * @pool_id: Return Buffer Manager ID
 * @desc_id: Descriptor ID
 * @type: 0 - Address points to a MSDU buffer
 *		1 - Address points to MSDU extension descriptor
 *
 * Return: void
 */
static inline void hal_tx_desc_set_buf_addr_generic(void *desc,
		dma_addr_t paddr, uint8_t rbm_id,
		uint32_t desc_id, uint8_t type)
{
	/* Set buffer_addr_info.buffer_addr_31_0 */
	HAL_SET_FLD(desc, UNIFIED_TCL_DATA_CMD_0, BUFFER_ADDR_INFO_BUF_ADDR_INFO) =
		HAL_TX_SM(UNIFIED_BUFFER_ADDR_INFO_0, BUFFER_ADDR_31_0, paddr);

	/* Set buffer_addr_info.buffer_addr_39_32 */
	HAL_SET_FLD(desc, UNIFIED_TCL_DATA_CMD_1,
			 BUFFER_ADDR_INFO_BUF_ADDR_INFO) |=
		HAL_TX_SM(UNIFIED_BUFFER_ADDR_INFO_1, BUFFER_ADDR_39_32,
		       (((uint64_t) paddr) >> 32));

	/* Set buffer_addr_info.return_buffer_manager = rbm id */
	HAL_SET_FLD(desc, UNIFIED_TCL_DATA_CMD_1,
			 BUFFER_ADDR_INFO_BUF_ADDR_INFO) |=
		HAL_TX_SM(UNIFIED_BUFFER_ADDR_INFO_1,
		       RETURN_BUFFER_MANAGER, rbm_id);

	/* Set buffer_addr_info.sw_buffer_cookie = desc_id */
	HAL_SET_FLD(desc, UNIFIED_TCL_DATA_CMD_1,
			BUFFER_ADDR_INFO_BUF_ADDR_INFO) |=
		HAL_TX_SM(UNIFIED_BUFFER_ADDR_INFO_1, SW_BUFFER_COOKIE, desc_id);

	/* Set  Buffer or Ext Descriptor Type */
	HAL_SET_FLD(desc, UNIFIED_TCL_DATA_CMD_2,
			BUF_OR_EXT_DESC_TYPE) |=
		HAL_TX_SM(UNIFIED_TCL_DATA_CMD_2, BUF_OR_EXT_DESC_TYPE, type);
}

#if defined(QCA_WIFI_QCA6290_11AX_MU_UL) && defined(QCA_WIFI_QCA6290_11AX)
/**
 * hal_rx_handle_other_tlvs() - handle special TLVs like MU_UL
 * tlv_tag: Taf of the TLVs
 * rx_tlv: the pointer to the TLVs
 * @ppdu_info: pointer to ppdu_info
 *
 * Return: true if the tlv is handled, false if not
 */
static inline bool
hal_rx_handle_other_tlvs(uint32_t tlv_tag, void *rx_tlv,
			 struct hal_rx_ppdu_info *ppdu_info)
{
	uint32_t value;

	switch (tlv_tag) {
	case WIFIPHYRX_HE_SIG_A_MU_UL_E:
	{
		uint8_t *he_sig_a_mu_ul_info =
			(uint8_t *)rx_tlv +
			HAL_RX_OFFSET(PHYRX_HE_SIG_A_MU_UL_0,
					  HE_SIG_A_MU_UL_INFO_PHYRX_HE_SIG_A_MU_UL_INFO_DETAILS);
		ppdu_info->rx_status.he_flags = 1;

		value = HAL_RX_GET(he_sig_a_mu_ul_info, HE_SIG_A_MU_UL_INFO_0,
				   FORMAT_INDICATION);
		if (value == 0) {
			ppdu_info->rx_status.he_data1 =
				QDF_MON_STATUS_HE_TRIG_FORMAT_TYPE;
		} else {
			 ppdu_info->rx_status.he_data1 =
				QDF_MON_STATUS_HE_SU_FORMAT_TYPE;
		}

		/* data1 */
		ppdu_info->rx_status.he_data1 |=
			QDF_MON_STATUS_HE_BSS_COLOR_KNOWN |
			QDF_MON_STATUS_HE_DL_UL_KNOWN |
			QDF_MON_STATUS_HE_DATA_BW_RU_KNOWN;

		/* data2 */
		ppdu_info->rx_status.he_data2 |=
			QDF_MON_STATUS_TXOP_KNOWN;

		/*data3*/
		value = HAL_RX_GET(he_sig_a_mu_ul_info,
				   HE_SIG_A_MU_UL_INFO_0, BSS_COLOR_ID);
		ppdu_info->rx_status.he_data3 = value;
		/* 1 for UL and 0 for DL */
		value = 1;
		value = value << QDF_MON_STATUS_DL_UL_SHIFT;
		ppdu_info->rx_status.he_data3 |= value;

		/*data4*/
		value = HAL_RX_GET(he_sig_a_mu_ul_info, HE_SIG_A_MU_UL_INFO_0,
				   SPATIAL_REUSE);
		ppdu_info->rx_status.he_data4 = value;

		/*data5*/
		value = HAL_RX_GET(he_sig_a_mu_ul_info,
				   HE_SIG_A_MU_UL_INFO_0, TRANSMIT_BW);
		ppdu_info->rx_status.he_data5 = value;
		ppdu_info->rx_status.bw = value;

		/*data6*/
		value = HAL_RX_GET(he_sig_a_mu_ul_info, HE_SIG_A_MU_UL_INFO_1,
				   TXOP_DURATION);
		value = value << QDF_MON_STATUS_TXOP_SHIFT;
		ppdu_info->rx_status.he_data6 |= value;
		return true;
	}
	default:
		return false;
	}
}
#else
static inline bool
hal_rx_handle_other_tlvs(uint32_t tlv_tag, void *rx_tlv,
			 struct hal_rx_ppdu_info *ppdu_info)
{
	return false;
}
#endif /* QCA_WIFI_QCA6290_11AX_MU_UL && QCA_WIFI_QCA6290_11AX */

#if defined(RX_PPDU_END_USER_STATS_1_OFDMA_INFO_VALID_OFFSET) && \
defined(RX_PPDU_END_USER_STATS_22_SW_RESPONSE_REFERENCE_PTR_EXT_OFFSET)

static inline void
hal_rx_handle_mu_ul_info(
	void *rx_tlv,
	struct mon_rx_user_status *mon_rx_user_status)
{
	mon_rx_user_status->mu_ul_user_v0_word0 =
		HAL_RX_GET(rx_tlv, RX_PPDU_END_USER_STATS_11,
			   SW_RESPONSE_REFERENCE_PTR);

	mon_rx_user_status->mu_ul_user_v0_word1 =
		HAL_RX_GET(rx_tlv, RX_PPDU_END_USER_STATS_22,
			   SW_RESPONSE_REFERENCE_PTR_EXT);
}

static inline void
hal_rx_populate_byte_count(void *rx_tlv, void *ppduinfo,
			   struct mon_rx_user_status *mon_rx_user_status)
{
	uint32_t mpdu_ok_byte_count;
	uint32_t mpdu_err_byte_count;

	mpdu_ok_byte_count = HAL_RX_GET(rx_tlv,
					RX_PPDU_END_USER_STATS_17,
					MPDU_OK_BYTE_COUNT);
	mpdu_err_byte_count = HAL_RX_GET(rx_tlv,
					 RX_PPDU_END_USER_STATS_19,
					 MPDU_ERR_BYTE_COUNT);

	mon_rx_user_status->mpdu_ok_byte_count = mpdu_ok_byte_count;
	mon_rx_user_status->mpdu_err_byte_count = mpdu_err_byte_count;
}
#else
static inline void
hal_rx_handle_mu_ul_info(void *rx_tlv,
			 struct mon_rx_user_status *mon_rx_user_status)
{
}

static inline void
hal_rx_populate_byte_count(void *rx_tlv, void *ppduinfo,
			   struct mon_rx_user_status *mon_rx_user_status)
{
	struct hal_rx_ppdu_info *ppdu_info =
			(struct hal_rx_ppdu_info *)ppduinfo;

	/* HKV1: doesn't support mpdu byte count */
	mon_rx_user_status->mpdu_ok_byte_count = ppdu_info->rx_status.ppdu_len;
	mon_rx_user_status->mpdu_err_byte_count = 0;
}
#endif

static inline void
hal_rx_populate_mu_user_info(void *rx_tlv, void *ppduinfo, uint32_t user_id,
			     struct mon_rx_user_status *mon_rx_user_status)
{
	struct mon_rx_info *mon_rx_info;
	struct mon_rx_user_info *mon_rx_user_info;
	struct hal_rx_ppdu_info *ppdu_info =
			(struct hal_rx_ppdu_info *)ppduinfo;

	mon_rx_info = &ppdu_info->rx_info;
	mon_rx_user_info = &ppdu_info->rx_user_info[user_id];
	mon_rx_user_info->qos_control_info_valid =
		mon_rx_info->qos_control_info_valid;
	mon_rx_user_info->qos_control =  mon_rx_info->qos_control;

	mon_rx_user_status->ast_index = ppdu_info->rx_status.ast_index;
	mon_rx_user_status->tid = ppdu_info->rx_status.tid;
	mon_rx_user_status->tcp_msdu_count =
		ppdu_info->rx_status.tcp_msdu_count;
	mon_rx_user_status->udp_msdu_count =
		ppdu_info->rx_status.udp_msdu_count;
	mon_rx_user_status->other_msdu_count =
		ppdu_info->rx_status.other_msdu_count;
	mon_rx_user_status->frame_control = ppdu_info->rx_status.frame_control;
	mon_rx_user_status->frame_control_info_valid =
		ppdu_info->rx_status.frame_control_info_valid;
	mon_rx_user_status->data_sequence_control_info_valid =
		ppdu_info->rx_status.data_sequence_control_info_valid;
	mon_rx_user_status->first_data_seq_ctrl =
		ppdu_info->rx_status.first_data_seq_ctrl;
	mon_rx_user_status->preamble_type = ppdu_info->rx_status.preamble_type;
	mon_rx_user_status->ht_flags = ppdu_info->rx_status.ht_flags;
	mon_rx_user_status->rtap_flags = ppdu_info->rx_status.rtap_flags;
	mon_rx_user_status->vht_flags = ppdu_info->rx_status.vht_flags;
	mon_rx_user_status->he_flags = ppdu_info->rx_status.he_flags;
	mon_rx_user_status->rs_flags = ppdu_info->rx_status.rs_flags;

	mon_rx_user_status->mpdu_cnt_fcs_ok =
		ppdu_info->com_info.mpdu_cnt_fcs_ok;
	mon_rx_user_status->mpdu_cnt_fcs_err =
		ppdu_info->com_info.mpdu_cnt_fcs_err;
	qdf_mem_copy(&mon_rx_user_status->mpdu_fcs_ok_bitmap,
		     &ppdu_info->com_info.mpdu_fcs_ok_bitmap,
		     HAL_RX_NUM_WORDS_PER_PPDU_BITMAP *
		     sizeof(ppdu_info->com_info.mpdu_fcs_ok_bitmap[0]));

	hal_rx_populate_byte_count(rx_tlv, ppdu_info, mon_rx_user_status);
}

#define HAL_RX_UPDATE_RSSI_PER_CHAIN_BW(chain, word_1, word_2, \
					ppdu_info, rssi_info_tlv) \
	{						\
	ppdu_info->rx_status.rssi_chain[chain][0] = \
			HAL_RX_GET(rssi_info_tlv, RECEIVE_RSSI_INFO_##word_1,\
				   RSSI_PRI20_CHAIN##chain); \
	ppdu_info->rx_status.rssi_chain[chain][1] = \
			HAL_RX_GET(rssi_info_tlv, RECEIVE_RSSI_INFO_##word_1,\
				   RSSI_EXT20_CHAIN##chain); \
	ppdu_info->rx_status.rssi_chain[chain][2] = \
			HAL_RX_GET(rssi_info_tlv, RECEIVE_RSSI_INFO_##word_1,\
				   RSSI_EXT40_LOW20_CHAIN##chain); \
	ppdu_info->rx_status.rssi_chain[chain][3] = \
			HAL_RX_GET(rssi_info_tlv, RECEIVE_RSSI_INFO_##word_1,\
				   RSSI_EXT40_HIGH20_CHAIN##chain); \
	ppdu_info->rx_status.rssi_chain[chain][4] = \
			HAL_RX_GET(rssi_info_tlv, RECEIVE_RSSI_INFO_##word_2,\
				   RSSI_EXT80_LOW20_CHAIN##chain); \
	ppdu_info->rx_status.rssi_chain[chain][5] = \
			HAL_RX_GET(rssi_info_tlv, RECEIVE_RSSI_INFO_##word_2,\
				   RSSI_EXT80_LOW_HIGH20_CHAIN##chain); \
	ppdu_info->rx_status.rssi_chain[chain][6] = \
			HAL_RX_GET(rssi_info_tlv, RECEIVE_RSSI_INFO_##word_2,\
				   RSSI_EXT80_HIGH_LOW20_CHAIN##chain); \
	ppdu_info->rx_status.rssi_chain[chain][7] = \
			HAL_RX_GET(rssi_info_tlv, RECEIVE_RSSI_INFO_##word_2,\
				   RSSI_EXT80_HIGH20_CHAIN##chain); \
	}						\

#define HAL_RX_PPDU_UPDATE_RSSI(ppdu_info, rssi_info_tlv) \
	{HAL_RX_UPDATE_RSSI_PER_CHAIN_BW(0, 0, 1, ppdu_info, rssi_info_tlv) \
	HAL_RX_UPDATE_RSSI_PER_CHAIN_BW(1, 2, 3, ppdu_info, rssi_info_tlv) \
	HAL_RX_UPDATE_RSSI_PER_CHAIN_BW(2, 4, 5, ppdu_info, rssi_info_tlv) \
	HAL_RX_UPDATE_RSSI_PER_CHAIN_BW(3, 6, 7, ppdu_info, rssi_info_tlv) \
	HAL_RX_UPDATE_RSSI_PER_CHAIN_BW(4, 8, 9, ppdu_info, rssi_info_tlv) \
	HAL_RX_UPDATE_RSSI_PER_CHAIN_BW(5, 10, 11, ppdu_info, rssi_info_tlv) \
	HAL_RX_UPDATE_RSSI_PER_CHAIN_BW(6, 12, 13, ppdu_info, rssi_info_tlv) \
	HAL_RX_UPDATE_RSSI_PER_CHAIN_BW(7, 14, 15, ppdu_info, rssi_info_tlv)} \

static inline uint32_t
hal_rx_update_rssi_chain(struct hal_rx_ppdu_info *ppdu_info,
			 uint8_t *rssi_info_tlv)
{
	HAL_RX_PPDU_UPDATE_RSSI(ppdu_info, rssi_info_tlv)
	return 0;
}

#ifdef WLAN_TX_PKT_CAPTURE_ENH
static inline void
hal_get_qos_control(void *rx_tlv,
		    struct hal_rx_ppdu_info *ppdu_info)
{
	ppdu_info->rx_info.qos_control_info_valid =
		HAL_RX_GET(rx_tlv, RX_PPDU_END_USER_STATS_3,
			   QOS_CONTROL_INFO_VALID);

	if (ppdu_info->rx_info.qos_control_info_valid)
		ppdu_info->rx_info.qos_control =
			HAL_RX_GET(rx_tlv,
				   RX_PPDU_END_USER_STATS_5,
				   QOS_CONTROL_FIELD);
}

static inline void
hal_get_mac_addr1(uint8_t *rx_mpdu_start,
		  struct hal_rx_ppdu_info *ppdu_info)
{
	if ((ppdu_info->sw_frame_group_id
	     == HAL_MPDU_SW_FRAME_GROUP_MGMT_PROBE_REQ) ||
	    (ppdu_info->sw_frame_group_id ==
	     HAL_MPDU_SW_FRAME_GROUP_CTRL_RTS)) {
		ppdu_info->rx_info.mac_addr1_valid =
				HAL_RX_GET_MAC_ADDR1_VALID(rx_mpdu_start);

		*(uint32_t *)&ppdu_info->rx_info.mac_addr1[0] =
			HAL_RX_GET(rx_mpdu_start,
				   RX_MPDU_INFO_15,
				   MAC_ADDR_AD1_31_0);
		if (ppdu_info->sw_frame_group_id ==
		    HAL_MPDU_SW_FRAME_GROUP_CTRL_RTS) {
			*(uint32_t *)&ppdu_info->rx_info.mac_addr1[4] =
				HAL_RX_GET(rx_mpdu_start,
					   RX_MPDU_INFO_16,
					   MAC_ADDR_AD1_47_32);
		}
	}
}
#else
static inline void
hal_get_qos_control(void *rx_tlv,
		    struct hal_rx_ppdu_info *ppdu_info)
{
}

static inline void
hal_get_mac_addr1(uint8_t *rx_mpdu_start,
		  struct hal_rx_ppdu_info *ppdu_info)
{
}
#endif

/**
 * hal_get_radiotap_he_gi_ltf() - Convert HE ltf and GI value
 * from stats enum to radiotap enum
 * @he_gi: HE GI value used in stats
 * @he_ltf: HE LTF value used in stats
 *
 * Return: void
 */
static inline void hal_get_radiotap_he_gi_ltf(uint16_t *he_gi, uint16_t *he_ltf)
{
	switch (*he_gi) {
	case HE_GI_0_8:
		*he_gi = HE_GI_RADIOTAP_0_8;
		break;
	case HE_GI_1_6:
		*he_gi = HE_GI_RADIOTAP_1_6;
		break;
	case HE_GI_3_2:
		*he_gi = HE_GI_RADIOTAP_3_2;
		break;
	default:
		*he_gi = HE_GI_RADIOTAP_RESERVED;
	}

	switch (*he_ltf) {
	case HE_LTF_1_X:
		*he_ltf = HE_LTF_RADIOTAP_1_X;
		break;
	case HE_LTF_2_X:
		*he_ltf = HE_LTF_RADIOTAP_2_X;
		break;
	case HE_LTF_4_X:
		*he_ltf = HE_LTF_RADIOTAP_4_X;
		break;
	default:
		*he_ltf = HE_LTF_RADIOTAP_UNKNOWN;
	}
}

/* channel number to freq conversion */
#define CHANNEL_NUM_14 14
#define CHANNEL_NUM_15 15
#define CHANNEL_NUM_27 27
#define CHANNEL_NUM_35 35
#define CHANNEL_NUM_182 182
#define CHANNEL_NUM_197 197
#define CHANNEL_FREQ_2484 2484
#define CHANNEL_FREQ_2407 2407
#define CHANNEL_FREQ_2512 2512
#define CHANNEL_FREQ_5000 5000
#define CHANNEL_FREQ_5950 5950
#define CHANNEL_FREQ_4000 4000
#define CHANNEL_FREQ_5150 5150
#define CHANNEL_FREQ_5920 5920
#define CHANNEL_FREQ_5935 5935
#define FREQ_MULTIPLIER_CONST_5MHZ 5
#define FREQ_MULTIPLIER_CONST_20MHZ 20
/**
 * hal_rx_radiotap_num_to_freq() - Get frequency from chan number
 * @chan_num - Input channel number
 * @center_freq - Input Channel Center frequency
 *
 * Return - Channel frequency in Mhz
 */
static uint16_t
hal_rx_radiotap_num_to_freq(uint16_t chan_num, qdf_freq_t center_freq)
{
	if (center_freq > CHANNEL_FREQ_5920 && center_freq < CHANNEL_FREQ_5950)
		return CHANNEL_FREQ_5935;

	if (center_freq < CHANNEL_FREQ_5950) {
		if (chan_num == CHANNEL_NUM_14)
			return CHANNEL_FREQ_2484;
		if (chan_num < CHANNEL_NUM_14)
			return CHANNEL_FREQ_2407 +
				(chan_num * FREQ_MULTIPLIER_CONST_5MHZ);

		if (chan_num < CHANNEL_NUM_27)
			return CHANNEL_FREQ_2512 +
				((chan_num - CHANNEL_NUM_15) *
					FREQ_MULTIPLIER_CONST_20MHZ);

		if (chan_num > CHANNEL_NUM_182 &&
		    chan_num < CHANNEL_NUM_197)
			return ((chan_num * FREQ_MULTIPLIER_CONST_5MHZ) +
				CHANNEL_FREQ_4000);

		return CHANNEL_FREQ_5000 +
			(chan_num * FREQ_MULTIPLIER_CONST_5MHZ);
	} else {
		return CHANNEL_FREQ_5950 +
			(chan_num * FREQ_MULTIPLIER_CONST_5MHZ);
	}
}

/**
 * hal_rx_status_get_tlv_info() - process receive info TLV
 * @rx_tlv_hdr: pointer to TLV header
 * @ppdu_info: pointer to ppdu_info
 *
 * Return: HAL_TLV_STATUS_PPDU_NOT_DONE or HAL_TLV_STATUS_PPDU_DONE from tlv
 */
static inline uint32_t
hal_rx_status_get_tlv_info_generic(void *rx_tlv_hdr, void *ppduinfo,
				   hal_soc_handle_t hal_soc_hdl,
				   qdf_nbuf_t nbuf)
{
	struct hal_soc *hal = (struct hal_soc *)hal_soc_hdl;
	uint32_t tlv_tag, user_id, tlv_len, value;
	uint8_t group_id = 0;
	uint8_t he_dcm = 0;
	uint8_t he_stbc = 0;
	uint16_t he_gi = 0;
	uint16_t he_ltf = 0;
	void *rx_tlv;
	bool unhandled = false;
	struct mon_rx_user_status *mon_rx_user_status;
	struct hal_rx_ppdu_info *ppdu_info =
			(struct hal_rx_ppdu_info *)ppduinfo;

	tlv_tag = HAL_RX_GET_USER_TLV32_TYPE(rx_tlv_hdr);
	user_id = HAL_RX_GET_USER_TLV32_USERID(rx_tlv_hdr);
	tlv_len = HAL_RX_GET_USER_TLV32_LEN(rx_tlv_hdr);

	rx_tlv = (uint8_t *)rx_tlv_hdr + HAL_RX_TLV32_HDR_SIZE;

	qdf_trace_hex_dump(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			   rx_tlv, tlv_len);

	switch (tlv_tag) {

	case WIFIRX_PPDU_START_E:
	{
		if (qdf_unlikely(ppdu_info->com_info.last_ppdu_id ==
		    HAL_RX_GET(rx_tlv, RX_PPDU_START_0, PHY_PPDU_ID)))
			hal_err("Matching ppdu_id(%u) detected",
				 ppdu_info->com_info.last_ppdu_id);

		/* Reset ppdu_info before processing the ppdu */
		qdf_mem_zero(ppdu_info,
			     sizeof(struct hal_rx_ppdu_info));

		ppdu_info->com_info.last_ppdu_id =
			ppdu_info->com_info.ppdu_id =
				HAL_RX_GET(rx_tlv, RX_PPDU_START_0,
					PHY_PPDU_ID);

		/* channel number is set in PHY meta data */
		ppdu_info->rx_status.chan_num =
			(HAL_RX_GET(rx_tlv, RX_PPDU_START_1,
				SW_PHY_META_DATA) & 0x0000FFFF);
		ppdu_info->rx_status.chan_freq =
			(HAL_RX_GET(rx_tlv, RX_PPDU_START_1,
				SW_PHY_META_DATA) & 0xFFFF0000)>>16;
		if (ppdu_info->rx_status.chan_num &&
		    ppdu_info->rx_status.chan_freq) {
			ppdu_info->rx_status.chan_freq =
				hal_rx_radiotap_num_to_freq(
				ppdu_info->rx_status.chan_num,
				 ppdu_info->rx_status.chan_freq);
		}
		ppdu_info->com_info.ppdu_timestamp =
			HAL_RX_GET(rx_tlv, RX_PPDU_START_2,
				PPDU_START_TIMESTAMP);
		ppdu_info->rx_status.ppdu_timestamp =
			ppdu_info->com_info.ppdu_timestamp;
		ppdu_info->rx_state = HAL_RX_MON_PPDU_START;

		break;
	}

	case WIFIRX_PPDU_START_USER_INFO_E:
		break;

	case WIFIRX_PPDU_END_E:
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			"[%s][%d] ppdu_end_e len=%d",
				__func__, __LINE__, tlv_len);
		/* This is followed by sub-TLVs of PPDU_END */
		ppdu_info->rx_state = HAL_RX_MON_PPDU_END;
		break;

	case WIFIPHYRX_PKT_END_E:
		hal_rx_get_rtt_info(hal_soc_hdl, rx_tlv, ppdu_info);
		break;

	case WIFIRXPCU_PPDU_END_INFO_E:
		ppdu_info->rx_status.rx_antenna =
			HAL_RX_GET(rx_tlv, RXPCU_PPDU_END_INFO_2, RX_ANTENNA);
		ppdu_info->rx_status.tsft =
			HAL_RX_GET(rx_tlv, RXPCU_PPDU_END_INFO_1,
				WB_TIMESTAMP_UPPER_32);
		ppdu_info->rx_status.tsft = (ppdu_info->rx_status.tsft << 32) |
			HAL_RX_GET(rx_tlv, RXPCU_PPDU_END_INFO_0,
				WB_TIMESTAMP_LOWER_32);
		ppdu_info->rx_status.duration =
			HAL_RX_GET(rx_tlv, UNIFIED_RXPCU_PPDU_END_INFO_8,
				RX_PPDU_DURATION);
		hal_rx_get_bb_info(hal_soc_hdl, rx_tlv, ppdu_info);
		break;

	/*
	 * WIFIRX_PPDU_END_USER_STATS_E comes for each user received.
	 * for MU, based on num users we see this tlv that many times.
	 */
	case WIFIRX_PPDU_END_USER_STATS_E:
	{
		unsigned long tid = 0;
		uint16_t seq = 0;

		ppdu_info->rx_status.ast_index =
				HAL_RX_GET(rx_tlv, RX_PPDU_END_USER_STATS_4,
						AST_INDEX);

		tid = HAL_RX_GET(rx_tlv, RX_PPDU_END_USER_STATS_12,
				RECEIVED_QOS_DATA_TID_BITMAP);
		ppdu_info->rx_status.tid = qdf_find_first_bit(&tid, sizeof(tid)*8);

		if (ppdu_info->rx_status.tid == (sizeof(tid) * 8))
			ppdu_info->rx_status.tid = HAL_TID_INVALID;

		ppdu_info->rx_status.tcp_msdu_count =
			HAL_RX_GET(rx_tlv, RX_PPDU_END_USER_STATS_9,
					TCP_MSDU_COUNT) +
			HAL_RX_GET(rx_tlv, RX_PPDU_END_USER_STATS_10,
					TCP_ACK_MSDU_COUNT);
		ppdu_info->rx_status.udp_msdu_count =
			HAL_RX_GET(rx_tlv, RX_PPDU_END_USER_STATS_9,
						UDP_MSDU_COUNT);
		ppdu_info->rx_status.other_msdu_count =
			HAL_RX_GET(rx_tlv, RX_PPDU_END_USER_STATS_10,
					OTHER_MSDU_COUNT);

		if (ppdu_info->sw_frame_group_id
		    != HAL_MPDU_SW_FRAME_GROUP_NULL_DATA) {
			ppdu_info->rx_status.frame_control_info_valid =
				HAL_RX_GET(rx_tlv, RX_PPDU_END_USER_STATS_3,
					   FRAME_CONTROL_INFO_VALID);

			if (ppdu_info->rx_status.frame_control_info_valid)
				ppdu_info->rx_status.frame_control =
					HAL_RX_GET(rx_tlv,
						   RX_PPDU_END_USER_STATS_4,
						   FRAME_CONTROL_FIELD);

			hal_get_qos_control(rx_tlv, ppdu_info);
		}

		ppdu_info->rx_status.data_sequence_control_info_valid =
			HAL_RX_GET(rx_tlv, RX_PPDU_END_USER_STATS_3,
				   DATA_SEQUENCE_CONTROL_INFO_VALID);

		seq = HAL_RX_GET(rx_tlv, RX_PPDU_END_USER_STATS_5,
				 FIRST_DATA_SEQ_CTRL);
		if (ppdu_info->rx_status.data_sequence_control_info_valid)
			ppdu_info->rx_status.first_data_seq_ctrl = seq;

		ppdu_info->rx_status.preamble_type =
			HAL_RX_GET(rx_tlv, RX_PPDU_END_USER_STATS_3,
						HT_CONTROL_FIELD_PKT_TYPE);
		switch (ppdu_info->rx_status.preamble_type) {
		case HAL_RX_PKT_TYPE_11N:
			ppdu_info->rx_status.ht_flags = 1;
			ppdu_info->rx_status.rtap_flags |= HT_SGI_PRESENT;
			break;
		case HAL_RX_PKT_TYPE_11AC:
			ppdu_info->rx_status.vht_flags = 1;
			break;
		case HAL_RX_PKT_TYPE_11AX:
			ppdu_info->rx_status.he_flags = 1;
			break;
		default:
			break;
		}

		ppdu_info->com_info.mpdu_cnt_fcs_ok =
			HAL_RX_GET(rx_tlv, RX_PPDU_END_USER_STATS_3,
					MPDU_CNT_FCS_OK);
		ppdu_info->com_info.mpdu_cnt_fcs_err =
			HAL_RX_GET(rx_tlv, RX_PPDU_END_USER_STATS_2,
					MPDU_CNT_FCS_ERR);
		if ((ppdu_info->com_info.mpdu_cnt_fcs_ok |
			ppdu_info->com_info.mpdu_cnt_fcs_err) > 1)
			ppdu_info->rx_status.rs_flags |= IEEE80211_AMPDU_FLAG;
		else
			ppdu_info->rx_status.rs_flags &=
				(~IEEE80211_AMPDU_FLAG);

		ppdu_info->com_info.mpdu_fcs_ok_bitmap[0] =
				HAL_RX_GET(rx_tlv, RX_PPDU_END_USER_STATS_7,
					   FCS_OK_BITMAP_31_0);

		ppdu_info->com_info.mpdu_fcs_ok_bitmap[1] =
				HAL_RX_GET(rx_tlv, RX_PPDU_END_USER_STATS_8,
					   FCS_OK_BITMAP_63_32);

		if (user_id < HAL_MAX_UL_MU_USERS) {
			mon_rx_user_status =
				&ppdu_info->rx_user_status[user_id];

			hal_rx_handle_mu_ul_info(rx_tlv, mon_rx_user_status);

			ppdu_info->com_info.num_users++;

			hal_rx_populate_mu_user_info(rx_tlv, ppdu_info,
						     user_id,
						     mon_rx_user_status);

		}
		break;
	}

	case WIFIRX_PPDU_END_USER_STATS_EXT_E:
		ppdu_info->com_info.mpdu_fcs_ok_bitmap[2] =
			HAL_RX_GET(rx_tlv, RX_PPDU_END_USER_STATS_EXT_1,
				   FCS_OK_BITMAP_95_64);

		ppdu_info->com_info.mpdu_fcs_ok_bitmap[3] =
			 HAL_RX_GET(rx_tlv, RX_PPDU_END_USER_STATS_EXT_2,
				    FCS_OK_BITMAP_127_96);

		ppdu_info->com_info.mpdu_fcs_ok_bitmap[4] =
			HAL_RX_GET(rx_tlv, RX_PPDU_END_USER_STATS_EXT_3,
				   FCS_OK_BITMAP_159_128);

		ppdu_info->com_info.mpdu_fcs_ok_bitmap[5] =
			 HAL_RX_GET(rx_tlv, RX_PPDU_END_USER_STATS_EXT_4,
				    FCS_OK_BITMAP_191_160);

		ppdu_info->com_info.mpdu_fcs_ok_bitmap[6] =
			HAL_RX_GET(rx_tlv, RX_PPDU_END_USER_STATS_EXT_5,
				   FCS_OK_BITMAP_223_192);

		ppdu_info->com_info.mpdu_fcs_ok_bitmap[7] =
			 HAL_RX_GET(rx_tlv, RX_PPDU_END_USER_STATS_EXT_6,
				    FCS_OK_BITMAP_255_224);
		break;

	case WIFIRX_PPDU_END_STATUS_DONE_E:
		return HAL_TLV_STATUS_PPDU_DONE;

	case WIFIDUMMY_E:
		return HAL_TLV_STATUS_BUF_DONE;

	case WIFIPHYRX_HT_SIG_E:
	{
		uint8_t *ht_sig_info = (uint8_t *)rx_tlv +
				HAL_RX_OFFSET(UNIFIED_PHYRX_HT_SIG_0,
				HT_SIG_INFO_PHYRX_HT_SIG_INFO_DETAILS);
		value = HAL_RX_GET(ht_sig_info, HT_SIG_INFO_1,
				FEC_CODING);
		ppdu_info->rx_status.ldpc = (value == HAL_SU_MU_CODING_LDPC) ?
			1 : 0;
		ppdu_info->rx_status.mcs = HAL_RX_GET(ht_sig_info,
				HT_SIG_INFO_0, MCS);
		ppdu_info->rx_status.ht_mcs = ppdu_info->rx_status.mcs;
		ppdu_info->rx_status.bw = HAL_RX_GET(ht_sig_info,
				HT_SIG_INFO_0, CBW);
		ppdu_info->rx_status.sgi = HAL_RX_GET(ht_sig_info,
				HT_SIG_INFO_1, SHORT_GI);
		ppdu_info->rx_status.reception_type = HAL_RX_TYPE_SU;
		ppdu_info->rx_status.nss = ((ppdu_info->rx_status.mcs) >>
				HT_SIG_SU_NSS_SHIFT) + 1;
		ppdu_info->rx_status.mcs &= ((1 << HT_SIG_SU_NSS_SHIFT) - 1);
		break;
	}

	case WIFIPHYRX_L_SIG_B_E:
	{
		uint8_t *l_sig_b_info = (uint8_t *)rx_tlv +
				HAL_RX_OFFSET(UNIFIED_PHYRX_L_SIG_B_0,
				L_SIG_B_INFO_PHYRX_L_SIG_B_INFO_DETAILS);

		value = HAL_RX_GET(l_sig_b_info, L_SIG_B_INFO_0, RATE);
		ppdu_info->rx_status.l_sig_b_info = *((uint32_t *)l_sig_b_info);
		switch (value) {
		case 1:
			ppdu_info->rx_status.rate = HAL_11B_RATE_3MCS;
			ppdu_info->rx_status.mcs = HAL_LEGACY_MCS3;
			break;
		case 2:
			ppdu_info->rx_status.rate = HAL_11B_RATE_2MCS;
			ppdu_info->rx_status.mcs = HAL_LEGACY_MCS2;
			break;
		case 3:
			ppdu_info->rx_status.rate = HAL_11B_RATE_1MCS;
			ppdu_info->rx_status.mcs = HAL_LEGACY_MCS1;
			break;
		case 4:
			ppdu_info->rx_status.rate = HAL_11B_RATE_0MCS;
			ppdu_info->rx_status.mcs = HAL_LEGACY_MCS0;
			break;
		case 5:
			ppdu_info->rx_status.rate = HAL_11B_RATE_6MCS;
			ppdu_info->rx_status.mcs = HAL_LEGACY_MCS6;
			break;
		case 6:
			ppdu_info->rx_status.rate = HAL_11B_RATE_5MCS;
			ppdu_info->rx_status.mcs = HAL_LEGACY_MCS5;
			break;
		case 7:
			ppdu_info->rx_status.rate = HAL_11B_RATE_4MCS;
			ppdu_info->rx_status.mcs = HAL_LEGACY_MCS4;
			break;
		default:
			break;
		}
		ppdu_info->rx_status.cck_flag = 1;
		ppdu_info->rx_status.reception_type = HAL_RX_TYPE_SU;
	break;
	}

	case WIFIPHYRX_L_SIG_A_E:
	{
		uint8_t *l_sig_a_info = (uint8_t *)rx_tlv +
				HAL_RX_OFFSET(UNIFIED_PHYRX_L_SIG_A_0,
				L_SIG_A_INFO_PHYRX_L_SIG_A_INFO_DETAILS);

		value = HAL_RX_GET(l_sig_a_info, L_SIG_A_INFO_0, RATE);
		ppdu_info->rx_status.l_sig_a_info = *((uint32_t *)l_sig_a_info);
		switch (value) {
		case 8:
			ppdu_info->rx_status.rate = HAL_11A_RATE_0MCS;
			ppdu_info->rx_status.mcs = HAL_LEGACY_MCS0;
			break;
		case 9:
			ppdu_info->rx_status.rate = HAL_11A_RATE_1MCS;
			ppdu_info->rx_status.mcs = HAL_LEGACY_MCS1;
			break;
		case 10:
			ppdu_info->rx_status.rate = HAL_11A_RATE_2MCS;
			ppdu_info->rx_status.mcs = HAL_LEGACY_MCS2;
			break;
		case 11:
			ppdu_info->rx_status.rate = HAL_11A_RATE_3MCS;
			ppdu_info->rx_status.mcs = HAL_LEGACY_MCS3;
			break;
		case 12:
			ppdu_info->rx_status.rate = HAL_11A_RATE_4MCS;
			ppdu_info->rx_status.mcs = HAL_LEGACY_MCS4;
			break;
		case 13:
			ppdu_info->rx_status.rate = HAL_11A_RATE_5MCS;
			ppdu_info->rx_status.mcs = HAL_LEGACY_MCS5;
			break;
		case 14:
			ppdu_info->rx_status.rate = HAL_11A_RATE_6MCS;
			ppdu_info->rx_status.mcs = HAL_LEGACY_MCS6;
			break;
		case 15:
			ppdu_info->rx_status.rate = HAL_11A_RATE_7MCS;
			ppdu_info->rx_status.mcs = HAL_LEGACY_MCS7;
			break;
		default:
			break;
		}
		ppdu_info->rx_status.ofdm_flag = 1;
		ppdu_info->rx_status.reception_type = HAL_RX_TYPE_SU;
	break;
	}

	case WIFIPHYRX_VHT_SIG_A_E:
	{
		uint8_t *vht_sig_a_info = (uint8_t *)rx_tlv +
				HAL_RX_OFFSET(UNIFIED_PHYRX_VHT_SIG_A_0,
				VHT_SIG_A_INFO_PHYRX_VHT_SIG_A_INFO_DETAILS);

		value = HAL_RX_GET(vht_sig_a_info, VHT_SIG_A_INFO_1,
				SU_MU_CODING);
		ppdu_info->rx_status.ldpc = (value == HAL_SU_MU_CODING_LDPC) ?
			1 : 0;
		group_id = HAL_RX_GET(vht_sig_a_info, VHT_SIG_A_INFO_0, GROUP_ID);
		ppdu_info->rx_status.vht_flag_values5 = group_id;
		ppdu_info->rx_status.mcs = HAL_RX_GET(vht_sig_a_info,
				VHT_SIG_A_INFO_1, MCS);
		ppdu_info->rx_status.sgi = HAL_RX_GET(vht_sig_a_info,
				VHT_SIG_A_INFO_1, GI_SETTING);

		switch (hal->target_type) {
		case TARGET_TYPE_QCA8074:
		case TARGET_TYPE_QCA8074V2:
		case TARGET_TYPE_QCA6018:
		case TARGET_TYPE_QCA5018:
		case TARGET_TYPE_QCN9000:
		case TARGET_TYPE_QCN9100:
#ifdef QCA_WIFI_QCA6390
		case TARGET_TYPE_QCA6390:
#endif
			ppdu_info->rx_status.is_stbc =
				HAL_RX_GET(vht_sig_a_info,
					   VHT_SIG_A_INFO_0, STBC);
			value =  HAL_RX_GET(vht_sig_a_info,
					    VHT_SIG_A_INFO_0, N_STS);
			value = value & VHT_SIG_SU_NSS_MASK;
			if (ppdu_info->rx_status.is_stbc && (value > 0))
				value = ((value + 1) >> 1) - 1;
			ppdu_info->rx_status.nss =
				((value & VHT_SIG_SU_NSS_MASK) + 1);

			break;
		case TARGET_TYPE_QCA6290:
#if !defined(QCA_WIFI_QCA6290_11AX)
			ppdu_info->rx_status.is_stbc =
				HAL_RX_GET(vht_sig_a_info,
					   VHT_SIG_A_INFO_0, STBC);
			value =  HAL_RX_GET(vht_sig_a_info,
					    VHT_SIG_A_INFO_0, N_STS);
			value = value & VHT_SIG_SU_NSS_MASK;
			if (ppdu_info->rx_status.is_stbc && (value > 0))
				value = ((value + 1) >> 1) - 1;
			ppdu_info->rx_status.nss =
				((value & VHT_SIG_SU_NSS_MASK) + 1);
#else
			ppdu_info->rx_status.nss = 0;
#endif
			break;
		case TARGET_TYPE_QCA6490:
		case TARGET_TYPE_QCA6750:
			ppdu_info->rx_status.nss = 0;
			break;
		default:
			break;
		}
		ppdu_info->rx_status.vht_flag_values3[0] =
				(((ppdu_info->rx_status.mcs) << 4)
				| ppdu_info->rx_status.nss);
		ppdu_info->rx_status.bw = HAL_RX_GET(vht_sig_a_info,
				VHT_SIG_A_INFO_0, BANDWIDTH);
		ppdu_info->rx_status.vht_flag_values2 =
			ppdu_info->rx_status.bw;
		ppdu_info->rx_status.vht_flag_values4 =
			HAL_RX_GET(vht_sig_a_info,
				  VHT_SIG_A_INFO_1, SU_MU_CODING);

		ppdu_info->rx_status.beamformed = HAL_RX_GET(vht_sig_a_info,
				VHT_SIG_A_INFO_1, BEAMFORMED);
		if (group_id == 0 || group_id == 63)
			ppdu_info->rx_status.reception_type = HAL_RX_TYPE_SU;
		else
			ppdu_info->rx_status.reception_type =
				HAL_RX_TYPE_MU_MIMO;

		break;
	}
	case WIFIPHYRX_HE_SIG_A_SU_E:
	{
		uint8_t *he_sig_a_su_info = (uint8_t *)rx_tlv +
			HAL_RX_OFFSET(UNIFIED_PHYRX_HE_SIG_A_SU_0,
			HE_SIG_A_SU_INFO_PHYRX_HE_SIG_A_SU_INFO_DETAILS);
		ppdu_info->rx_status.he_flags = 1;
		value = HAL_RX_GET(he_sig_a_su_info, HE_SIG_A_SU_INFO_0,
			FORMAT_INDICATION);
		if (value == 0) {
			ppdu_info->rx_status.he_data1 =
				QDF_MON_STATUS_HE_TRIG_FORMAT_TYPE;
		} else {
			 ppdu_info->rx_status.he_data1 =
				 QDF_MON_STATUS_HE_SU_FORMAT_TYPE;
		}

		/* data1 */
		ppdu_info->rx_status.he_data1 |=
			QDF_MON_STATUS_HE_BSS_COLOR_KNOWN |
			QDF_MON_STATUS_HE_BEAM_CHANGE_KNOWN |
			QDF_MON_STATUS_HE_DL_UL_KNOWN |
			QDF_MON_STATUS_HE_MCS_KNOWN |
			QDF_MON_STATUS_HE_DCM_KNOWN |
			QDF_MON_STATUS_HE_CODING_KNOWN |
			QDF_MON_STATUS_HE_LDPC_EXTRA_SYMBOL_KNOWN |
			QDF_MON_STATUS_HE_STBC_KNOWN |
			QDF_MON_STATUS_HE_DATA_BW_RU_KNOWN |
			QDF_MON_STATUS_HE_DOPPLER_KNOWN;

		/* data2 */
		ppdu_info->rx_status.he_data2 =
			QDF_MON_STATUS_HE_GI_KNOWN;
		ppdu_info->rx_status.he_data2 |=
			QDF_MON_STATUS_TXBF_KNOWN |
			QDF_MON_STATUS_PE_DISAMBIGUITY_KNOWN |
			QDF_MON_STATUS_TXOP_KNOWN |
			QDF_MON_STATUS_LTF_SYMBOLS_KNOWN |
			QDF_MON_STATUS_PRE_FEC_PADDING_KNOWN |
			QDF_MON_STATUS_MIDABLE_PERIODICITY_KNOWN;

		/* data3 */
		value = HAL_RX_GET(he_sig_a_su_info,
				HE_SIG_A_SU_INFO_0, BSS_COLOR_ID);
		ppdu_info->rx_status.he_data3 = value;
		value = HAL_RX_GET(he_sig_a_su_info,
				HE_SIG_A_SU_INFO_0, BEAM_CHANGE);
		value = value << QDF_MON_STATUS_BEAM_CHANGE_SHIFT;
		ppdu_info->rx_status.he_data3 |= value;
		value = HAL_RX_GET(he_sig_a_su_info,
				HE_SIG_A_SU_INFO_0, DL_UL_FLAG);
		value = value << QDF_MON_STATUS_DL_UL_SHIFT;
		ppdu_info->rx_status.he_data3 |= value;

		value = HAL_RX_GET(he_sig_a_su_info,
				HE_SIG_A_SU_INFO_0, TRANSMIT_MCS);
		ppdu_info->rx_status.mcs = value;
		value = value << QDF_MON_STATUS_TRANSMIT_MCS_SHIFT;
		ppdu_info->rx_status.he_data3 |= value;

		value = HAL_RX_GET(he_sig_a_su_info,
				HE_SIG_A_SU_INFO_0, DCM);
		he_dcm = value;
		value = value << QDF_MON_STATUS_DCM_SHIFT;
		ppdu_info->rx_status.he_data3 |= value;
		value = HAL_RX_GET(he_sig_a_su_info,
				HE_SIG_A_SU_INFO_1, CODING);
		ppdu_info->rx_status.ldpc = (value == HAL_SU_MU_CODING_LDPC) ?
			1 : 0;
		value = value << QDF_MON_STATUS_CODING_SHIFT;
		ppdu_info->rx_status.he_data3 |= value;
		value = HAL_RX_GET(he_sig_a_su_info,
				HE_SIG_A_SU_INFO_1,
				LDPC_EXTRA_SYMBOL);
		value = value << QDF_MON_STATUS_LDPC_EXTRA_SYMBOL_SHIFT;
		ppdu_info->rx_status.he_data3 |= value;
		value = HAL_RX_GET(he_sig_a_su_info,
				HE_SIG_A_SU_INFO_1, STBC);
		he_stbc = value;
		value = value << QDF_MON_STATUS_STBC_SHIFT;
		ppdu_info->rx_status.he_data3 |= value;

		/* data4 */
		value = HAL_RX_GET(he_sig_a_su_info, HE_SIG_A_SU_INFO_0,
							SPATIAL_REUSE);
		ppdu_info->rx_status.he_data4 = value;

		/* data5 */
		value = HAL_RX_GET(he_sig_a_su_info,
				HE_SIG_A_SU_INFO_0, TRANSMIT_BW);
		ppdu_info->rx_status.he_data5 = value;
		ppdu_info->rx_status.bw = value;
		value = HAL_RX_GET(he_sig_a_su_info,
				HE_SIG_A_SU_INFO_0, CP_LTF_SIZE);
		switch (value) {
		case 0:
				he_gi = HE_GI_0_8;
				he_ltf = HE_LTF_1_X;
				break;
		case 1:
				he_gi = HE_GI_0_8;
				he_ltf = HE_LTF_2_X;
				break;
		case 2:
				he_gi = HE_GI_1_6;
				he_ltf = HE_LTF_2_X;
				break;
		case 3:
				if (he_dcm && he_stbc) {
					he_gi = HE_GI_0_8;
					he_ltf = HE_LTF_4_X;
				} else {
					he_gi = HE_GI_3_2;
					he_ltf = HE_LTF_4_X;
				}
				break;
		}
		ppdu_info->rx_status.sgi = he_gi;
		ppdu_info->rx_status.ltf_size = he_ltf;
		hal_get_radiotap_he_gi_ltf(&he_gi, &he_ltf);
		value = he_gi << QDF_MON_STATUS_GI_SHIFT;
		ppdu_info->rx_status.he_data5 |= value;
		value = he_ltf << QDF_MON_STATUS_HE_LTF_SIZE_SHIFT;
		ppdu_info->rx_status.he_data5 |= value;

		value = HAL_RX_GET(he_sig_a_su_info, HE_SIG_A_SU_INFO_0, NSTS);
		value = (value << QDF_MON_STATUS_HE_LTF_SYM_SHIFT);
		ppdu_info->rx_status.he_data5 |= value;

		value = HAL_RX_GET(he_sig_a_su_info, HE_SIG_A_SU_INFO_1,
						PACKET_EXTENSION_A_FACTOR);
		value = value << QDF_MON_STATUS_PRE_FEC_PAD_SHIFT;
		ppdu_info->rx_status.he_data5 |= value;

		value = HAL_RX_GET(he_sig_a_su_info, HE_SIG_A_SU_INFO_1, TXBF);
		value = value << QDF_MON_STATUS_TXBF_SHIFT;
		ppdu_info->rx_status.he_data5 |= value;
		value = HAL_RX_GET(he_sig_a_su_info, HE_SIG_A_SU_INFO_1,
					PACKET_EXTENSION_PE_DISAMBIGUITY);
		value = value << QDF_MON_STATUS_PE_DISAMBIGUITY_SHIFT;
		ppdu_info->rx_status.he_data5 |= value;

		/* data6 */
		value = HAL_RX_GET(he_sig_a_su_info, HE_SIG_A_SU_INFO_0, NSTS);
		value++;
		ppdu_info->rx_status.nss = value;
		ppdu_info->rx_status.he_data6 = value;
		value = HAL_RX_GET(he_sig_a_su_info, HE_SIG_A_SU_INFO_1,
							DOPPLER_INDICATION);
		value = value << QDF_MON_STATUS_DOPPLER_SHIFT;
		ppdu_info->rx_status.he_data6 |= value;
		value = HAL_RX_GET(he_sig_a_su_info, HE_SIG_A_SU_INFO_1,
							TXOP_DURATION);
		value = value << QDF_MON_STATUS_TXOP_SHIFT;
		ppdu_info->rx_status.he_data6 |= value;

		ppdu_info->rx_status.beamformed = HAL_RX_GET(he_sig_a_su_info,
					HE_SIG_A_SU_INFO_1, TXBF);
		ppdu_info->rx_status.reception_type = HAL_RX_TYPE_SU;
		break;
	}
	case WIFIPHYRX_HE_SIG_A_MU_DL_E:
	{
		uint8_t *he_sig_a_mu_dl_info = (uint8_t *)rx_tlv +
			HAL_RX_OFFSET(UNIFIED_PHYRX_HE_SIG_A_MU_DL_0,
			HE_SIG_A_MU_DL_INFO_PHYRX_HE_SIG_A_MU_DL_INFO_DETAILS);

		ppdu_info->rx_status.he_mu_flags = 1;

		/* HE Flags */
		/*data1*/
		ppdu_info->rx_status.he_data1 =
					QDF_MON_STATUS_HE_MU_FORMAT_TYPE;
		ppdu_info->rx_status.he_data1 |=
			QDF_MON_STATUS_HE_BSS_COLOR_KNOWN |
			QDF_MON_STATUS_HE_DL_UL_KNOWN |
			QDF_MON_STATUS_HE_LDPC_EXTRA_SYMBOL_KNOWN |
			QDF_MON_STATUS_HE_STBC_KNOWN |
			QDF_MON_STATUS_HE_DATA_BW_RU_KNOWN |
			QDF_MON_STATUS_HE_DOPPLER_KNOWN;

		/* data2 */
		ppdu_info->rx_status.he_data2 =
			QDF_MON_STATUS_HE_GI_KNOWN;
		ppdu_info->rx_status.he_data2 |=
			QDF_MON_STATUS_LTF_SYMBOLS_KNOWN |
			QDF_MON_STATUS_PRE_FEC_PADDING_KNOWN |
			QDF_MON_STATUS_PE_DISAMBIGUITY_KNOWN |
			QDF_MON_STATUS_TXOP_KNOWN |
			QDF_MON_STATUS_MIDABLE_PERIODICITY_KNOWN;

		/*data3*/
		value = HAL_RX_GET(he_sig_a_mu_dl_info,
				HE_SIG_A_MU_DL_INFO_0, BSS_COLOR_ID);
		ppdu_info->rx_status.he_data3 = value;

		value = HAL_RX_GET(he_sig_a_mu_dl_info,
				HE_SIG_A_MU_DL_INFO_0, DL_UL_FLAG);
		value = value << QDF_MON_STATUS_DL_UL_SHIFT;
		ppdu_info->rx_status.he_data3 |= value;

		value = HAL_RX_GET(he_sig_a_mu_dl_info,
				HE_SIG_A_MU_DL_INFO_1,
				LDPC_EXTRA_SYMBOL);
		value = value << QDF_MON_STATUS_LDPC_EXTRA_SYMBOL_SHIFT;
		ppdu_info->rx_status.he_data3 |= value;

		value = HAL_RX_GET(he_sig_a_mu_dl_info,
				HE_SIG_A_MU_DL_INFO_1, STBC);
		he_stbc = value;
		value = value << QDF_MON_STATUS_STBC_SHIFT;
		ppdu_info->rx_status.he_data3 |= value;

		/*data4*/
		value = HAL_RX_GET(he_sig_a_mu_dl_info, HE_SIG_A_MU_DL_INFO_0,
							SPATIAL_REUSE);
		ppdu_info->rx_status.he_data4 = value;

		/*data5*/
		value = HAL_RX_GET(he_sig_a_mu_dl_info,
				HE_SIG_A_MU_DL_INFO_0, TRANSMIT_BW);
		ppdu_info->rx_status.he_data5 = value;
		ppdu_info->rx_status.bw = value;

		value = HAL_RX_GET(he_sig_a_mu_dl_info,
				HE_SIG_A_MU_DL_INFO_0, CP_LTF_SIZE);
		switch (value) {
		case 0:
			he_gi = HE_GI_0_8;
			he_ltf = HE_LTF_4_X;
			break;
		case 1:
			he_gi = HE_GI_0_8;
			he_ltf = HE_LTF_2_X;
			break;
		case 2:
			he_gi = HE_GI_1_6;
			he_ltf = HE_LTF_2_X;
			break;
		case 3:
			he_gi = HE_GI_3_2;
			he_ltf = HE_LTF_4_X;
			break;
		}
		ppdu_info->rx_status.sgi = he_gi;
		ppdu_info->rx_status.ltf_size = he_ltf;
		hal_get_radiotap_he_gi_ltf(&he_gi, &he_ltf);
		value = he_gi << QDF_MON_STATUS_GI_SHIFT;
		ppdu_info->rx_status.he_data5 |= value;

		value = he_ltf << QDF_MON_STATUS_HE_LTF_SIZE_SHIFT;
		ppdu_info->rx_status.he_data5 |= value;

		value = HAL_RX_GET(he_sig_a_mu_dl_info,
				   HE_SIG_A_MU_DL_INFO_1, NUM_LTF_SYMBOLS);
		value = (value << QDF_MON_STATUS_HE_LTF_SYM_SHIFT);
		ppdu_info->rx_status.he_data5 |= value;

		value = HAL_RX_GET(he_sig_a_mu_dl_info, HE_SIG_A_MU_DL_INFO_1,
				   PACKET_EXTENSION_A_FACTOR);
		value = value << QDF_MON_STATUS_PRE_FEC_PAD_SHIFT;
		ppdu_info->rx_status.he_data5 |= value;


		value = HAL_RX_GET(he_sig_a_mu_dl_info, HE_SIG_A_MU_DL_INFO_1,
				   PACKET_EXTENSION_PE_DISAMBIGUITY);
		value = value << QDF_MON_STATUS_PE_DISAMBIGUITY_SHIFT;
		ppdu_info->rx_status.he_data5 |= value;

		/*data6*/
		value = HAL_RX_GET(he_sig_a_mu_dl_info, HE_SIG_A_MU_DL_INFO_0,
							DOPPLER_INDICATION);
		value = value << QDF_MON_STATUS_DOPPLER_SHIFT;
		ppdu_info->rx_status.he_data6 |= value;

		value = HAL_RX_GET(he_sig_a_mu_dl_info, HE_SIG_A_MU_DL_INFO_1,
							TXOP_DURATION);
		value = value << QDF_MON_STATUS_TXOP_SHIFT;
		ppdu_info->rx_status.he_data6 |= value;

		/* HE-MU Flags */
		/* HE-MU-flags1 */
		ppdu_info->rx_status.he_flags1 =
			QDF_MON_STATUS_SIG_B_MCS_KNOWN |
			QDF_MON_STATUS_SIG_B_DCM_KNOWN |
			QDF_MON_STATUS_SIG_B_COMPRESSION_FLAG_1_KNOWN |
			QDF_MON_STATUS_SIG_B_SYM_NUM_KNOWN |
			QDF_MON_STATUS_RU_0_KNOWN;

		value = HAL_RX_GET(he_sig_a_mu_dl_info,
				HE_SIG_A_MU_DL_INFO_0, MCS_OF_SIG_B);
		ppdu_info->rx_status.he_flags1 |= value;
		value = HAL_RX_GET(he_sig_a_mu_dl_info,
				HE_SIG_A_MU_DL_INFO_0, DCM_OF_SIG_B);
		value = value << QDF_MON_STATUS_DCM_FLAG_1_SHIFT;
		ppdu_info->rx_status.he_flags1 |= value;

		/* HE-MU-flags2 */
		ppdu_info->rx_status.he_flags2 =
			QDF_MON_STATUS_BW_KNOWN;

		value = HAL_RX_GET(he_sig_a_mu_dl_info,
				HE_SIG_A_MU_DL_INFO_0, TRANSMIT_BW);
		ppdu_info->rx_status.he_flags2 |= value;
		value = HAL_RX_GET(he_sig_a_mu_dl_info,
				HE_SIG_A_MU_DL_INFO_0, COMP_MODE_SIG_B);
		value = value << QDF_MON_STATUS_SIG_B_COMPRESSION_FLAG_2_SHIFT;
		ppdu_info->rx_status.he_flags2 |= value;
		value = HAL_RX_GET(he_sig_a_mu_dl_info,
				HE_SIG_A_MU_DL_INFO_0, NUM_SIG_B_SYMBOLS);
		value = value - 1;
		value = value << QDF_MON_STATUS_NUM_SIG_B_SYMBOLS_SHIFT;
		ppdu_info->rx_status.he_flags2 |= value;
		ppdu_info->rx_status.reception_type = HAL_RX_TYPE_MU_MIMO;
		break;
	}
	case WIFIPHYRX_HE_SIG_B1_MU_E:
	{

		uint8_t *he_sig_b1_mu_info = (uint8_t *)rx_tlv +
			HAL_RX_OFFSET(UNIFIED_PHYRX_HE_SIG_B1_MU_0,
			HE_SIG_B1_MU_INFO_PHYRX_HE_SIG_B1_MU_INFO_DETAILS);

		ppdu_info->rx_status.he_sig_b_common_known |=
			QDF_MON_STATUS_HE_SIG_B_COMMON_KNOWN_RU0;
		/* TODO: Check on the availability of other fields in
		 * sig_b_common
		 */

		value = HAL_RX_GET(he_sig_b1_mu_info,
				HE_SIG_B1_MU_INFO_0, RU_ALLOCATION);
		ppdu_info->rx_status.he_RU[0] = value;
		ppdu_info->rx_status.reception_type = HAL_RX_TYPE_MU_MIMO;
		break;
	}
	case WIFIPHYRX_HE_SIG_B2_MU_E:
	{
		uint8_t *he_sig_b2_mu_info = (uint8_t *)rx_tlv +
			HAL_RX_OFFSET(UNIFIED_PHYRX_HE_SIG_B2_MU_0,
			HE_SIG_B2_MU_INFO_PHYRX_HE_SIG_B2_MU_INFO_DETAILS);
		/*
		 * Not all "HE" fields can be updated from
		 * WIFIPHYRX_HE_SIG_A_MU_DL_E TLV. Use WIFIPHYRX_HE_SIG_B2_MU_E
		 * to populate rest of the "HE" fields for MU scenarios.
		 */

		/* HE-data1 */
		ppdu_info->rx_status.he_data1 |=
			QDF_MON_STATUS_HE_MCS_KNOWN |
			QDF_MON_STATUS_HE_CODING_KNOWN;

		/* HE-data2 */

		/* HE-data3 */
		value = HAL_RX_GET(he_sig_b2_mu_info,
				HE_SIG_B2_MU_INFO_0, STA_MCS);
		ppdu_info->rx_status.mcs = value;
		value = value << QDF_MON_STATUS_TRANSMIT_MCS_SHIFT;
		ppdu_info->rx_status.he_data3 |= value;


		value = HAL_RX_GET(he_sig_b2_mu_info,
				HE_SIG_B2_MU_INFO_0, STA_CODING);
		value = value << QDF_MON_STATUS_CODING_SHIFT;
		ppdu_info->rx_status.he_data3 |= value;

		/* HE-data4 */
		value = HAL_RX_GET(he_sig_b2_mu_info,
				HE_SIG_B2_MU_INFO_0, STA_ID);
		value = value << QDF_MON_STATUS_STA_ID_SHIFT;
		ppdu_info->rx_status.he_data4 |= value;

		/* HE-data5 */

		/* HE-data6 */
		value = HAL_RX_GET(he_sig_b2_mu_info,
				   HE_SIG_B2_MU_INFO_0, NSTS);
		/* value n indicates n+1 spatial streams */
		value++;
		ppdu_info->rx_status.nss = value;
		ppdu_info->rx_status.he_data6 |= value;

		break;

	}
	case WIFIPHYRX_HE_SIG_B2_OFDMA_E:
	{
		uint8_t *he_sig_b2_ofdma_info =
		(uint8_t *)rx_tlv +
		HAL_RX_OFFSET(UNIFIED_PHYRX_HE_SIG_B2_OFDMA_0,
		HE_SIG_B2_OFDMA_INFO_PHYRX_HE_SIG_B2_OFDMA_INFO_DETAILS);

		/*
		 * Not all "HE" fields can be updated from
		 * WIFIPHYRX_HE_SIG_A_MU_DL_E TLV. Use WIFIPHYRX_HE_SIG_B2_MU_E
		 * to populate rest of "HE" fields for MU OFDMA scenarios.
		 */

		/* HE-data1 */
		ppdu_info->rx_status.he_data1 |=
			QDF_MON_STATUS_HE_MCS_KNOWN |
			QDF_MON_STATUS_HE_DCM_KNOWN |
			QDF_MON_STATUS_HE_CODING_KNOWN;

		/* HE-data2 */
		ppdu_info->rx_status.he_data2 |=
					QDF_MON_STATUS_TXBF_KNOWN;

		/* HE-data3 */
		value = HAL_RX_GET(he_sig_b2_ofdma_info,
				HE_SIG_B2_OFDMA_INFO_0, STA_MCS);
		ppdu_info->rx_status.mcs = value;
		value = value << QDF_MON_STATUS_TRANSMIT_MCS_SHIFT;
		ppdu_info->rx_status.he_data3 |= value;

		value = HAL_RX_GET(he_sig_b2_ofdma_info,
				HE_SIG_B2_OFDMA_INFO_0, STA_DCM);
		he_dcm = value;
		value = value << QDF_MON_STATUS_DCM_SHIFT;
		ppdu_info->rx_status.he_data3 |= value;

		value = HAL_RX_GET(he_sig_b2_ofdma_info,
				HE_SIG_B2_OFDMA_INFO_0, STA_CODING);
		value = value << QDF_MON_STATUS_CODING_SHIFT;
		ppdu_info->rx_status.he_data3 |= value;

		/* HE-data4 */
		value = HAL_RX_GET(he_sig_b2_ofdma_info,
				HE_SIG_B2_OFDMA_INFO_0, STA_ID);
		value = value << QDF_MON_STATUS_STA_ID_SHIFT;
		ppdu_info->rx_status.he_data4 |= value;

		/* HE-data5 */
		value = HAL_RX_GET(he_sig_b2_ofdma_info,
				   HE_SIG_B2_OFDMA_INFO_0, TXBF);
		value = value << QDF_MON_STATUS_TXBF_SHIFT;
		ppdu_info->rx_status.he_data5 |= value;

		/* HE-data6 */
		value = HAL_RX_GET(he_sig_b2_ofdma_info,
				   HE_SIG_B2_OFDMA_INFO_0, NSTS);
		/* value n indicates n+1 spatial streams */
		value++;
		ppdu_info->rx_status.nss = value;
		ppdu_info->rx_status.he_data6 |= value;
		ppdu_info->rx_status.reception_type = HAL_RX_TYPE_MU_OFDMA;
		break;
	}
	case WIFIPHYRX_RSSI_LEGACY_E:
	{
		uint8_t reception_type;
		int8_t rssi_value;
		uint8_t *rssi_info_tlv = (uint8_t *)rx_tlv +
			HAL_RX_OFFSET(UNIFIED_PHYRX_RSSI_LEGACY_19,
				RECEIVE_RSSI_INFO_PREAMBLE_RSSI_INFO_DETAILS);

		ppdu_info->rx_status.rssi_comb = HAL_RX_GET(rx_tlv,
			PHYRX_RSSI_LEGACY_35, RSSI_COMB);
		ppdu_info->rx_status.bw = hal->ops->hal_rx_get_tlv(rx_tlv);
		ppdu_info->rx_status.he_re = 0;

		reception_type = HAL_RX_GET(rx_tlv,
					    PHYRX_RSSI_LEGACY_0,
					    RECEPTION_TYPE);
		switch (reception_type) {
		case QDF_RECEPTION_TYPE_ULOFMDA:
			ppdu_info->rx_status.reception_type =
				HAL_RX_TYPE_MU_OFDMA;
			ppdu_info->rx_status.ulofdma_flag = 1;
			ppdu_info->rx_status.he_data1 =
				QDF_MON_STATUS_HE_TRIG_FORMAT_TYPE;
			break;
		case QDF_RECEPTION_TYPE_ULMIMO:
			ppdu_info->rx_status.reception_type =
				HAL_RX_TYPE_MU_MIMO;
			ppdu_info->rx_status.he_data1 =
				QDF_MON_STATUS_HE_MU_FORMAT_TYPE;
			break;
		default:
			ppdu_info->rx_status.reception_type =
				HAL_RX_TYPE_SU;
			break;
		}
		hal_rx_update_rssi_chain(ppdu_info, rssi_info_tlv);
		rssi_value = HAL_RX_GET(rssi_info_tlv,
					RECEIVE_RSSI_INFO_0, RSSI_PRI20_CHAIN0);
		ppdu_info->rx_status.rssi[0] = rssi_value;
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			  "RSSI_PRI20_CHAIN0: %d\n", rssi_value);

		rssi_value = HAL_RX_GET(rssi_info_tlv,
					RECEIVE_RSSI_INFO_2, RSSI_PRI20_CHAIN1);
		ppdu_info->rx_status.rssi[1] = rssi_value;
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			  "RSSI_PRI20_CHAIN1: %d\n", rssi_value);

		rssi_value = HAL_RX_GET(rssi_info_tlv,
					RECEIVE_RSSI_INFO_4, RSSI_PRI20_CHAIN2);
		ppdu_info->rx_status.rssi[2] = rssi_value;
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			  "RSSI_PRI20_CHAIN2: %d\n", rssi_value);

		rssi_value = HAL_RX_GET(rssi_info_tlv,
					RECEIVE_RSSI_INFO_6, RSSI_PRI20_CHAIN3);
		ppdu_info->rx_status.rssi[3] = rssi_value;
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			  "RSSI_PRI20_CHAIN3: %d\n", rssi_value);

		rssi_value = HAL_RX_GET(rssi_info_tlv,
					RECEIVE_RSSI_INFO_8, RSSI_PRI20_CHAIN4);
		ppdu_info->rx_status.rssi[4] = rssi_value;
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			  "RSSI_PRI20_CHAIN4: %d\n", rssi_value);

		rssi_value = HAL_RX_GET(rssi_info_tlv,
					RECEIVE_RSSI_INFO_10,
					RSSI_PRI20_CHAIN5);
		ppdu_info->rx_status.rssi[5] = rssi_value;
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			  "RSSI_PRI20_CHAIN5: %d\n", rssi_value);

		rssi_value = HAL_RX_GET(rssi_info_tlv,
					RECEIVE_RSSI_INFO_12,
					RSSI_PRI20_CHAIN6);
		ppdu_info->rx_status.rssi[6] = rssi_value;
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			  "RSSI_PRI20_CHAIN6: %d\n", rssi_value);

		rssi_value = HAL_RX_GET(rssi_info_tlv,
					RECEIVE_RSSI_INFO_14,
					RSSI_PRI20_CHAIN7);
		ppdu_info->rx_status.rssi[7] = rssi_value;
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			  "RSSI_PRI20_CHAIN7: %d\n", rssi_value);
		break;
	}
	case WIFIPHYRX_OTHER_RECEIVE_INFO_E:
		hal_rx_proc_phyrx_other_receive_info_tlv(hal, rx_tlv_hdr,
								ppdu_info);
		break;
	case WIFIRX_HEADER_E:
	{
		struct hal_rx_ppdu_common_info *com_info = &ppdu_info->com_info;

		if (ppdu_info->fcs_ok_cnt >=
		    HAL_RX_MAX_MPDU_H_PER_STATUS_BUFFER) {
			hal_err("Number of MPDUs(%d) per status buff exceeded",
				ppdu_info->fcs_ok_cnt);
			break;
		}

		/* Update first_msdu_payload for every mpdu and increment
		 * com_info->mpdu_cnt for every WIFIRX_HEADER_E TLV
		 */
		ppdu_info->ppdu_msdu_info[ppdu_info->fcs_ok_cnt].first_msdu_payload =
			rx_tlv;
		ppdu_info->ppdu_msdu_info[ppdu_info->fcs_ok_cnt].payload_len = tlv_len;
		ppdu_info->msdu_info.first_msdu_payload = rx_tlv;
		ppdu_info->msdu_info.payload_len = tlv_len;
		ppdu_info->user_id = user_id;
		ppdu_info->hdr_len = tlv_len;
		ppdu_info->data = rx_tlv;
		ppdu_info->data += 4;

		/* for every RX_HEADER TLV increment mpdu_cnt */
		com_info->mpdu_cnt++;
		return HAL_TLV_STATUS_HEADER;
	}
	case WIFIRX_MPDU_START_E:
	{
		uint8_t *rx_mpdu_start = (uint8_t *)rx_tlv;
		uint32_t ppdu_id = HAL_RX_GET_PPDU_ID(rx_mpdu_start);
		uint8_t filter_category = 0;

		ppdu_info->nac_info.fc_valid =
				HAL_RX_GET_FC_VALID(rx_mpdu_start);

		ppdu_info->nac_info.to_ds_flag =
				HAL_RX_GET_TO_DS_FLAG(rx_mpdu_start);

		ppdu_info->nac_info.frame_control =
			HAL_RX_GET(rx_mpdu_start,
				   RX_MPDU_INFO_14,
				   MPDU_FRAME_CONTROL_FIELD);

		ppdu_info->sw_frame_group_id =
			HAL_RX_GET_SW_FRAME_GROUP_ID(rx_mpdu_start);

		if (ppdu_info->sw_frame_group_id ==
		    HAL_MPDU_SW_FRAME_GROUP_NULL_DATA) {
			ppdu_info->rx_status.frame_control_info_valid =
				ppdu_info->nac_info.fc_valid;
			ppdu_info->rx_status.frame_control =
				ppdu_info->nac_info.frame_control;
		}

		hal_get_mac_addr1(rx_mpdu_start,
				  ppdu_info);

		ppdu_info->nac_info.mac_addr2_valid =
				HAL_RX_GET_MAC_ADDR2_VALID(rx_mpdu_start);

		*(uint16_t *)&ppdu_info->nac_info.mac_addr2[0] =
			HAL_RX_GET(rx_mpdu_start,
				   RX_MPDU_INFO_16,
				   MAC_ADDR_AD2_15_0);

		*(uint32_t *)&ppdu_info->nac_info.mac_addr2[2] =
			HAL_RX_GET(rx_mpdu_start,
				   RX_MPDU_INFO_17,
				   MAC_ADDR_AD2_47_16);

		if (ppdu_info->rx_status.prev_ppdu_id != ppdu_id) {
			ppdu_info->rx_status.prev_ppdu_id = ppdu_id;
			ppdu_info->rx_status.ppdu_len =
				HAL_RX_GET(rx_mpdu_start, RX_MPDU_INFO_13,
					   MPDU_LENGTH);
		} else {
			ppdu_info->rx_status.ppdu_len +=
				HAL_RX_GET(rx_mpdu_start, RX_MPDU_INFO_13,
					   MPDU_LENGTH);
		}

		filter_category =
				HAL_RX_GET_FILTER_CATEGORY(rx_mpdu_start);

		if (filter_category == 0)
			ppdu_info->rx_status.rxpcu_filter_pass = 1;
		else if (filter_category == 1)
			ppdu_info->rx_status.monitor_direct_used = 1;

		ppdu_info->nac_info.mcast_bcast =
			HAL_RX_GET(rx_mpdu_start,
				   RX_MPDU_INFO_13,
				   MCAST_BCAST);
		break;
	}
	case WIFIRX_MPDU_END_E:
		ppdu_info->user_id = user_id;
		ppdu_info->fcs_err =
			HAL_RX_GET(rx_tlv, RX_MPDU_END_1,
				   FCS_ERR);
		return HAL_TLV_STATUS_MPDU_END;
	case WIFIRX_MSDU_END_E:
		if (user_id < HAL_MAX_UL_MU_USERS) {
			ppdu_info->rx_msdu_info[user_id].cce_metadata =
				HAL_RX_MSDU_END_CCE_METADATA_GET(rx_tlv);
			ppdu_info->rx_msdu_info[user_id].fse_metadata =
				HAL_RX_MSDU_END_FSE_METADATA_GET(rx_tlv);
			ppdu_info->rx_msdu_info[user_id].is_flow_idx_timeout =
				HAL_RX_MSDU_END_FLOW_IDX_TIMEOUT_GET(rx_tlv);
			ppdu_info->rx_msdu_info[user_id].is_flow_idx_invalid =
				HAL_RX_MSDU_END_FLOW_IDX_INVALID_GET(rx_tlv);
			ppdu_info->rx_msdu_info[user_id].flow_idx =
				HAL_RX_MSDU_END_FLOW_IDX_GET(rx_tlv);
		}
		return HAL_TLV_STATUS_MSDU_END;
	case 0:
		return HAL_TLV_STATUS_PPDU_DONE;

	default:
		if (hal_rx_handle_other_tlvs(tlv_tag, rx_tlv, ppdu_info))
			unhandled = false;
		else
			unhandled = true;
		break;
	}

	if (!unhandled)
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			  "%s TLV type: %d, TLV len:%d %s",
			  __func__, tlv_tag, tlv_len,
			  unhandled == true ? "unhandled" : "");

	qdf_trace_hex_dump(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
				rx_tlv, tlv_len);

	return HAL_TLV_STATUS_PPDU_NOT_DONE;
}

/**
 * hal_reo_setup - Initialize HW REO block
 *
 * @hal_soc: Opaque HAL SOC handle
 * @reo_params: parameters needed by HAL for REO config
 */
static void hal_reo_setup_generic(struct hal_soc *soc,
				  void *reoparams)
{
	uint32_t reg_val;
	struct hal_reo_params *reo_params = (struct hal_reo_params *)reoparams;

	reg_val = HAL_REG_READ(soc, HWIO_REO_R0_GENERAL_ENABLE_ADDR(
		SEQ_WCSS_UMAC_REO_REG_OFFSET));

	hal_reo_config(soc, reg_val, reo_params);
	/* Other ring enable bits and REO_ENABLE will be set by FW */

	/* TODO: Setup destination ring mapping if enabled */

	/* TODO: Error destination ring setting is left to default.
	 * Default setting is to send all errors to release ring.
	 */

	HAL_REG_WRITE(soc,
		HWIO_REO_R0_AGING_THRESHOLD_IX_0_ADDR(
		SEQ_WCSS_UMAC_REO_REG_OFFSET),
		HAL_DEFAULT_BE_BK_VI_REO_TIMEOUT_MS * 1000);

	HAL_REG_WRITE(soc,
		HWIO_REO_R0_AGING_THRESHOLD_IX_1_ADDR(
		SEQ_WCSS_UMAC_REO_REG_OFFSET),
		(HAL_DEFAULT_BE_BK_VI_REO_TIMEOUT_MS * 1000));

	HAL_REG_WRITE(soc,
		HWIO_REO_R0_AGING_THRESHOLD_IX_2_ADDR(
		SEQ_WCSS_UMAC_REO_REG_OFFSET),
		(HAL_DEFAULT_BE_BK_VI_REO_TIMEOUT_MS * 1000));

	HAL_REG_WRITE(soc,
		HWIO_REO_R0_AGING_THRESHOLD_IX_3_ADDR(
		SEQ_WCSS_UMAC_REO_REG_OFFSET),
		(HAL_DEFAULT_VO_REO_TIMEOUT_MS * 1000));

	/*
	 * When hash based routing is enabled, routing of the rx packet
	 * is done based on the following value: 1 _ _ _ _ The last 4
	 * bits are based on hash[3:0]. This means the possible values
	 * are 0x10 to 0x1f. This value is used to look-up the
	 * ring ID configured in Destination_Ring_Ctrl_IX_* register.
	 * The Destination_Ring_Ctrl_IX_2 and Destination_Ring_Ctrl_IX_3
	 * registers need to be configured to set-up the 16 entries to
	 * map the hash values to a ring number. There are 3 bits per
	 * hash entry  which are mapped as follows:
	 * 0: TCL, 1:SW1, 2:SW2, * 3:SW3, 4:SW4, 5:Release, 6:FW(WIFI),
	 * 7: NOT_USED.
	*/
	if (reo_params->rx_hash_enabled) {
		HAL_REG_WRITE(soc,
			HWIO_REO_R0_DESTINATION_RING_CTRL_IX_2_ADDR(
			SEQ_WCSS_UMAC_REO_REG_OFFSET),
			reo_params->remap1);

		hal_debug("HWIO_REO_R0_DESTINATION_RING_CTRL_IX_2_ADDR 0x%x",
			  HAL_REG_READ(soc,
				       HWIO_REO_R0_DESTINATION_RING_CTRL_IX_2_ADDR(
				       SEQ_WCSS_UMAC_REO_REG_OFFSET)));

		HAL_REG_WRITE(soc,
			HWIO_REO_R0_DESTINATION_RING_CTRL_IX_3_ADDR(
			SEQ_WCSS_UMAC_REO_REG_OFFSET),
			reo_params->remap2);

		hal_debug("HWIO_REO_R0_DESTINATION_RING_CTRL_IX_3_ADDR 0x%x",
			  HAL_REG_READ(soc,
				       HWIO_REO_R0_DESTINATION_RING_CTRL_IX_3_ADDR(
				       SEQ_WCSS_UMAC_REO_REG_OFFSET)));
	}

	/* TODO: Check if the following registers shoould be setup by host:
	 * AGING_CONTROL
	 * HIGH_MEMORY_THRESHOLD
	 * GLOBAL_LINK_DESC_COUNT_THRESH_IX_0[1,2]
	 * GLOBAL_LINK_DESC_COUNT_CTRL
	 */
}

/**
 * hal_get_hw_hptp_generic()  - Get HW head and tail pointer value for any ring
 * @hal_soc: Opaque HAL SOC handle
 * @hal_ring: Source ring pointer
 * @headp: Head Pointer
 * @tailp: Tail Pointer
 * @ring: Ring type
 *
 * Return: Update tail pointer and head pointer in arguments.
 */
static inline
void hal_get_hw_hptp_generic(struct hal_soc *hal_soc,
			     hal_ring_handle_t hal_ring_hdl,
			     uint32_t *headp, uint32_t *tailp,
			     uint8_t ring)
{
	struct hal_srng *srng = (struct hal_srng *)hal_ring_hdl;
	struct hal_hw_srng_config *ring_config;
	enum hal_ring_type ring_type = (enum hal_ring_type)ring;

	if (!hal_soc  || !srng) {
		QDF_TRACE(QDF_MODULE_ID_HAL, QDF_TRACE_LEVEL_ERROR,
			  "%s: Context is Null", __func__);
		return;
	}

	ring_config = HAL_SRNG_CONFIG(hal_soc, ring_type);
	if (!ring_config->lmac_ring) {
		if (srng->ring_dir == HAL_SRNG_SRC_RING) {
			*headp = SRNG_SRC_REG_READ(srng, HP);
			*tailp = SRNG_SRC_REG_READ(srng, TP);
		} else {
			*headp = SRNG_DST_REG_READ(srng, HP);
			*tailp = SRNG_DST_REG_READ(srng, TP);
		}
	}
}

#if defined(WBM_IDLE_LSB_WRITE_CONFIRM_WAR)
/**
 * hal_wbm_idle_lsb_write_confirm() - Check and update WBM_IDLE_LINK ring LSB
 * @srng: srng handle
 *
 * Return: None
 */
static void hal_wbm_idle_lsb_write_confirm(struct hal_srng *srng)
{
	if (srng->ring_id == HAL_SRNG_WBM_IDLE_LINK) {
		while (SRNG_SRC_REG_READ(srng, BASE_LSB) !=
		       ((unsigned int)srng->ring_base_paddr & 0xffffffff))
				SRNG_SRC_REG_WRITE(srng, BASE_LSB,
						   srng->ring_base_paddr &
						   0xffffffff);
	}
}
#else
static void hal_wbm_idle_lsb_write_confirm(struct hal_srng *srng)
{
}
#endif

/**
 * hal_srng_src_hw_init - Private function to initialize SRNG
 * source ring HW
 * @hal_soc: HAL SOC handle
 * @srng: SRNG ring pointer
 */
static inline
void hal_srng_src_hw_init_generic(struct hal_soc *hal,
				  struct hal_srng *srng)
{
	uint32_t reg_val = 0;
	uint64_t tp_addr = 0;

	hal_debug("hw_init srng %d", srng->ring_id);

	if (srng->flags & HAL_SRNG_MSI_INTR) {
		SRNG_SRC_REG_WRITE(srng, MSI1_BASE_LSB,
			srng->msi_addr & 0xffffffff);
		reg_val = SRNG_SM(SRNG_SRC_FLD(MSI1_BASE_MSB, ADDR),
			(uint64_t)(srng->msi_addr) >> 32) |
			SRNG_SM(SRNG_SRC_FLD(MSI1_BASE_MSB,
			MSI1_ENABLE), 1);
		SRNG_SRC_REG_WRITE(srng, MSI1_BASE_MSB, reg_val);
		SRNG_SRC_REG_WRITE(srng, MSI1_DATA, srng->msi_data);
	}

	SRNG_SRC_REG_WRITE(srng, BASE_LSB, srng->ring_base_paddr & 0xffffffff);
	hal_wbm_idle_lsb_write_confirm(srng);

	reg_val = SRNG_SM(SRNG_SRC_FLD(BASE_MSB, RING_BASE_ADDR_MSB),
		((uint64_t)(srng->ring_base_paddr) >> 32)) |
		SRNG_SM(SRNG_SRC_FLD(BASE_MSB, RING_SIZE),
		srng->entry_size * srng->num_entries);
	SRNG_SRC_REG_WRITE(srng, BASE_MSB, reg_val);

	reg_val = SRNG_SM(SRNG_SRC_FLD(ID, ENTRY_SIZE), srng->entry_size);
	SRNG_SRC_REG_WRITE(srng, ID, reg_val);

	/**
	 * Interrupt setup:
	 * Default interrupt mode is 'pulse'. Need to setup SW_INTERRUPT_MODE
	 * if level mode is required
	 */
	reg_val = 0;

	/*
	 * WAR - Hawkeye v1 has a hardware bug which requires timer value to be
	 * programmed in terms of 1us resolution instead of 8us resolution as
	 * given in MLD.
	 */
	if (srng->intr_timer_thres_us) {
		reg_val |= SRNG_SM(SRNG_SRC_FLD(CONSUMER_INT_SETUP_IX0,
			INTERRUPT_TIMER_THRESHOLD),
			srng->intr_timer_thres_us);
		/* For HK v2 this should be (srng->intr_timer_thres_us >> 3) */
	}

	if (srng->intr_batch_cntr_thres_entries) {
		reg_val |= SRNG_SM(SRNG_SRC_FLD(CONSUMER_INT_SETUP_IX0,
			BATCH_COUNTER_THRESHOLD),
			srng->intr_batch_cntr_thres_entries *
			srng->entry_size);
	}
	SRNG_SRC_REG_WRITE(srng, CONSUMER_INT_SETUP_IX0, reg_val);

	reg_val = 0;
	if (srng->flags & HAL_SRNG_LOW_THRES_INTR_ENABLE) {
		reg_val |= SRNG_SM(SRNG_SRC_FLD(CONSUMER_INT_SETUP_IX1,
			LOW_THRESHOLD), srng->u.src_ring.low_threshold);
	}

	SRNG_SRC_REG_WRITE(srng, CONSUMER_INT_SETUP_IX1, reg_val);

	/* As per HW team, TP_ADDR and HP_ADDR for Idle link ring should
	 * remain 0 to avoid some WBM stability issues. Remote head/tail
	 * pointers are not required since this ring is completely managed
	 * by WBM HW
	 */
	reg_val = 0;
	if (srng->ring_id != HAL_SRNG_WBM_IDLE_LINK) {
		tp_addr = (uint64_t)(hal->shadow_rdptr_mem_paddr +
			((unsigned long)(srng->u.src_ring.tp_addr) -
			(unsigned long)(hal->shadow_rdptr_mem_vaddr)));
		SRNG_SRC_REG_WRITE(srng, TP_ADDR_LSB, tp_addr & 0xffffffff);
		SRNG_SRC_REG_WRITE(srng, TP_ADDR_MSB, tp_addr >> 32);
	} else {
		reg_val |= SRNG_SM(SRNG_SRC_FLD(MISC, RING_ID_DISABLE), 1);
	}

	/* Initilaize head and tail pointers to indicate ring is empty */
	SRNG_SRC_REG_WRITE(srng, HP, 0);
	SRNG_SRC_REG_WRITE(srng, TP, 0);
	*(srng->u.src_ring.tp_addr) = 0;

	reg_val |= ((srng->flags & HAL_SRNG_DATA_TLV_SWAP) ?
			SRNG_SM(SRNG_SRC_FLD(MISC, DATA_TLV_SWAP_BIT), 1) : 0) |
			((srng->flags & HAL_SRNG_RING_PTR_SWAP) ?
			SRNG_SM(SRNG_SRC_FLD(MISC, HOST_FW_SWAP_BIT), 1) : 0) |
			((srng->flags & HAL_SRNG_MSI_SWAP) ?
			SRNG_SM(SRNG_SRC_FLD(MISC, MSI_SWAP_BIT), 1) : 0);

	/* Loop count is not used for SRC rings */
	reg_val |= SRNG_SM(SRNG_SRC_FLD(MISC, LOOPCNT_DISABLE), 1);

	/*
	 * reg_val |= SRNG_SM(SRNG_SRC_FLD(MISC, SRNG_ENABLE), 1);
	 * todo: update fw_api and replace with above line
	 * (when SRNG_ENABLE field for the MISC register is available in fw_api)
	 * (WCSS_UMAC_CE_0_SRC_WFSS_CE_CHANNEL_SRC_R0_SRC_RING_MISC)
	 */
	reg_val |= 0x40;

	SRNG_SRC_REG_WRITE(srng, MISC, reg_val);
}

/**
 * hal_srng_dst_hw_init - Private function to initialize SRNG
 * destination ring HW
 * @hal_soc: HAL SOC handle
 * @srng: SRNG ring pointer
 */
static inline
void hal_srng_dst_hw_init_generic(struct hal_soc *hal,
				  struct hal_srng *srng)
{
	uint32_t reg_val = 0;
	uint64_t hp_addr = 0;

	hal_debug("hw_init srng %d", srng->ring_id);

	if (srng->flags & HAL_SRNG_MSI_INTR) {
		SRNG_DST_REG_WRITE(srng, MSI1_BASE_LSB,
			srng->msi_addr & 0xffffffff);
		reg_val = SRNG_SM(SRNG_DST_FLD(MSI1_BASE_MSB, ADDR),
			(uint64_t)(srng->msi_addr) >> 32) |
			SRNG_SM(SRNG_DST_FLD(MSI1_BASE_MSB,
			MSI1_ENABLE), 1);
		SRNG_DST_REG_WRITE(srng, MSI1_BASE_MSB, reg_val);
		SRNG_DST_REG_WRITE(srng, MSI1_DATA, srng->msi_data);
	}

	SRNG_DST_REG_WRITE(srng, BASE_LSB, srng->ring_base_paddr & 0xffffffff);
	reg_val = SRNG_SM(SRNG_DST_FLD(BASE_MSB, RING_BASE_ADDR_MSB),
		((uint64_t)(srng->ring_base_paddr) >> 32)) |
		SRNG_SM(SRNG_DST_FLD(BASE_MSB, RING_SIZE),
		srng->entry_size * srng->num_entries);
	SRNG_DST_REG_WRITE(srng, BASE_MSB, reg_val);

	reg_val = SRNG_SM(SRNG_DST_FLD(ID, RING_ID), srng->ring_id) |
		SRNG_SM(SRNG_DST_FLD(ID, ENTRY_SIZE), srng->entry_size);
	SRNG_DST_REG_WRITE(srng, ID, reg_val);


	/**
	 * Interrupt setup:
	 * Default interrupt mode is 'pulse'. Need to setup SW_INTERRUPT_MODE
	 * if level mode is required
	 */
	reg_val = 0;
	if (srng->intr_timer_thres_us) {
		reg_val |= SRNG_SM(SRNG_DST_FLD(PRODUCER_INT_SETUP,
			INTERRUPT_TIMER_THRESHOLD),
			srng->intr_timer_thres_us >> 3);
	}

	if (srng->intr_batch_cntr_thres_entries) {
		reg_val |= SRNG_SM(SRNG_DST_FLD(PRODUCER_INT_SETUP,
			BATCH_COUNTER_THRESHOLD),
			srng->intr_batch_cntr_thres_entries *
			srng->entry_size);
	}

	SRNG_DST_REG_WRITE(srng, PRODUCER_INT_SETUP, reg_val);
	hp_addr = (uint64_t)(hal->shadow_rdptr_mem_paddr +
		((unsigned long)(srng->u.dst_ring.hp_addr) -
		(unsigned long)(hal->shadow_rdptr_mem_vaddr)));
	SRNG_DST_REG_WRITE(srng, HP_ADDR_LSB, hp_addr & 0xffffffff);
	SRNG_DST_REG_WRITE(srng, HP_ADDR_MSB, hp_addr >> 32);

	/* Initilaize head and tail pointers to indicate ring is empty */
	SRNG_DST_REG_WRITE(srng, HP, 0);
	SRNG_DST_REG_WRITE(srng, TP, 0);
	*(srng->u.dst_ring.hp_addr) = 0;

	reg_val = ((srng->flags & HAL_SRNG_DATA_TLV_SWAP) ?
			SRNG_SM(SRNG_DST_FLD(MISC, DATA_TLV_SWAP_BIT), 1) : 0) |
			((srng->flags & HAL_SRNG_RING_PTR_SWAP) ?
			SRNG_SM(SRNG_DST_FLD(MISC, HOST_FW_SWAP_BIT), 1) : 0) |
			((srng->flags & HAL_SRNG_MSI_SWAP) ?
			SRNG_SM(SRNG_DST_FLD(MISC, MSI_SWAP_BIT), 1) : 0);

	/*
	 * reg_val |= SRNG_SM(SRNG_SRC_FLD(MISC, SRNG_ENABLE), 1);
	 * todo: update fw_api and replace with above line
	 * (when SRNG_ENABLE field for the MISC register is available in fw_api)
	 * (WCSS_UMAC_CE_0_SRC_WFSS_CE_CHANNEL_SRC_R0_SRC_RING_MISC)
	 */
	reg_val |= 0x40;

	SRNG_DST_REG_WRITE(srng, MISC, reg_val);

}

#define HAL_RX_WBM_ERR_SRC_GET(wbm_desc) (((*(((uint32_t *) wbm_desc)+ \
		(WBM_RELEASE_RING_2_RELEASE_SOURCE_MODULE_OFFSET >> 2))) & \
		WBM_RELEASE_RING_2_RELEASE_SOURCE_MODULE_MASK) >> \
		WBM_RELEASE_RING_2_RELEASE_SOURCE_MODULE_LSB)

#define HAL_RX_WBM_REO_PUSH_REASON_GET(wbm_desc) (((*(((uint32_t *) wbm_desc)+ \
		(WBM_RELEASE_RING_2_REO_PUSH_REASON_OFFSET >> 2))) & \
		WBM_RELEASE_RING_2_REO_PUSH_REASON_MASK) >> \
		WBM_RELEASE_RING_2_REO_PUSH_REASON_LSB)

#define HAL_RX_WBM_REO_ERROR_CODE_GET(wbm_desc) (((*(((uint32_t *) wbm_desc)+ \
		(WBM_RELEASE_RING_2_REO_ERROR_CODE_OFFSET >> 2))) & \
		WBM_RELEASE_RING_2_REO_ERROR_CODE_MASK) >> \
		WBM_RELEASE_RING_2_REO_ERROR_CODE_LSB)

#define HAL_RX_WBM_RXDMA_PUSH_REASON_GET(wbm_desc)	\
	(((*(((uint32_t *) wbm_desc) +			\
	(WBM_RELEASE_RING_2_RXDMA_PUSH_REASON_OFFSET >> 2))) & \
	WBM_RELEASE_RING_2_RXDMA_PUSH_REASON_MASK) >>	\
	WBM_RELEASE_RING_2_RXDMA_PUSH_REASON_LSB)

#define HAL_RX_WBM_RXDMA_ERROR_CODE_GET(wbm_desc)	\
	(((*(((uint32_t *) wbm_desc) +			\
	(WBM_RELEASE_RING_2_RXDMA_ERROR_CODE_OFFSET >> 2))) & \
	WBM_RELEASE_RING_2_RXDMA_ERROR_CODE_MASK) >>	\
	WBM_RELEASE_RING_2_RXDMA_ERROR_CODE_LSB)

/**
 * hal_rx_wbm_err_info_get_generic(): Retrieves WBM error code and reason and
 *	save it to hal_wbm_err_desc_info structure passed by caller
 * @wbm_desc: wbm ring descriptor
 * @wbm_er_info1: hal_wbm_err_desc_info structure, output parameter.
 * Return: void
 */
static inline void hal_rx_wbm_err_info_get_generic(void *wbm_desc,
				void *wbm_er_info1)
{
	struct hal_wbm_err_desc_info *wbm_er_info =
		(struct hal_wbm_err_desc_info *)wbm_er_info1;

	wbm_er_info->wbm_err_src = HAL_RX_WBM_ERR_SRC_GET(wbm_desc);
	wbm_er_info->reo_psh_rsn = HAL_RX_WBM_REO_PUSH_REASON_GET(wbm_desc);
	wbm_er_info->reo_err_code = HAL_RX_WBM_REO_ERROR_CODE_GET(wbm_desc);
	wbm_er_info->rxdma_psh_rsn = HAL_RX_WBM_RXDMA_PUSH_REASON_GET(wbm_desc);
	wbm_er_info->rxdma_err_code = HAL_RX_WBM_RXDMA_ERROR_CODE_GET(wbm_desc);
}

/**
 * hal_tx_comp_get_release_reason_generic() - TQM Release reason
 * @hal_desc: completion ring descriptor pointer
 *
 * This function will return the type of pointer - buffer or descriptor
 *
 * Return: buffer type
 */
static inline uint8_t hal_tx_comp_get_release_reason_generic(void *hal_desc)
{
	uint32_t comp_desc =
		*(uint32_t *) (((uint8_t *) hal_desc) +
			       WBM_RELEASE_RING_2_TQM_RELEASE_REASON_OFFSET);

	return (comp_desc & WBM_RELEASE_RING_2_TQM_RELEASE_REASON_MASK) >>
		WBM_RELEASE_RING_2_TQM_RELEASE_REASON_LSB;
}

/**
 * hal_get_wbm_internal_error_generic() - is WBM internal error
 * @hal_desc: completion ring descriptor pointer
 *
 * This function will return 0 or 1  - is it WBM internal error or not
 *
 * Return: uint8_t
 */
static inline uint8_t hal_get_wbm_internal_error_generic(void *hal_desc)
{
	uint32_t comp_desc =
		*(uint32_t *)(((uint8_t *)hal_desc) +
			      WBM_RELEASE_RING_2_WBM_INTERNAL_ERROR_OFFSET);

	return (comp_desc & WBM_RELEASE_RING_2_WBM_INTERNAL_ERROR_MASK) >>
		WBM_RELEASE_RING_2_WBM_INTERNAL_ERROR_LSB;
}

/**
 * hal_rx_dump_mpdu_start_tlv_generic: dump RX mpdu_start TLV in structured
 *			       human readable format.
 * @mpdu_start: pointer the rx_attention TLV in pkt.
 * @dbg_level: log level.
 *
 * Return: void
 */
static inline void hal_rx_dump_mpdu_start_tlv_generic(void *mpdustart,
						      uint8_t dbg_level)
{
	struct rx_mpdu_start *mpdu_start = (struct rx_mpdu_start *)mpdustart;
	struct rx_mpdu_info *mpdu_info =
		(struct rx_mpdu_info *)&mpdu_start->rx_mpdu_info_details;

	hal_verbose_debug(
			  "rx_mpdu_start tlv (1/5) - "
			  "rxpcu_mpdu_filter_in_category: %x "
			  "sw_frame_group_id: %x "
			  "ndp_frame: %x "
			  "phy_err: %x "
			  "phy_err_during_mpdu_header: %x "
			  "protocol_version_err: %x "
			  "ast_based_lookup_valid: %x "
			  "phy_ppdu_id: %x "
			  "ast_index: %x "
			  "sw_peer_id: %x "
			  "mpdu_frame_control_valid: %x "
			  "mpdu_duration_valid: %x "
			  "mac_addr_ad1_valid: %x "
			  "mac_addr_ad2_valid: %x "
			  "mac_addr_ad3_valid: %x "
			  "mac_addr_ad4_valid: %x "
			  "mpdu_sequence_control_valid: %x "
			  "mpdu_qos_control_valid: %x "
			  "mpdu_ht_control_valid: %x "
			  "frame_encryption_info_valid: %x ",
			  mpdu_info->rxpcu_mpdu_filter_in_category,
			  mpdu_info->sw_frame_group_id,
			  mpdu_info->ndp_frame,
			  mpdu_info->phy_err,
			  mpdu_info->phy_err_during_mpdu_header,
			  mpdu_info->protocol_version_err,
			  mpdu_info->ast_based_lookup_valid,
			  mpdu_info->phy_ppdu_id,
			  mpdu_info->ast_index,
			  mpdu_info->sw_peer_id,
			  mpdu_info->mpdu_frame_control_valid,
			  mpdu_info->mpdu_duration_valid,
			  mpdu_info->mac_addr_ad1_valid,
			  mpdu_info->mac_addr_ad2_valid,
			  mpdu_info->mac_addr_ad3_valid,
			  mpdu_info->mac_addr_ad4_valid,
			  mpdu_info->mpdu_sequence_control_valid,
			  mpdu_info->mpdu_qos_control_valid,
			  mpdu_info->mpdu_ht_control_valid,
			  mpdu_info->frame_encryption_info_valid);

	hal_verbose_debug(
			  "rx_mpdu_start tlv (2/5) - "
			  "fr_ds: %x "
			  "to_ds: %x "
			  "encrypted: %x "
			  "mpdu_retry: %x "
			  "mpdu_sequence_number: %x "
			  "epd_en: %x "
			  "all_frames_shall_be_encrypted: %x "
			  "encrypt_type: %x "
			  "mesh_sta: %x "
			  "bssid_hit: %x "
			  "bssid_number: %x "
			  "tid: %x "
			  "pn_31_0: %x "
			  "pn_63_32: %x "
			  "pn_95_64: %x "
			  "pn_127_96: %x "
			  "peer_meta_data: %x "
			  "rxpt_classify_info.reo_destination_indication: %x "
			  "rxpt_classify_info.use_flow_id_toeplitz_clfy: %x "
			  "rx_reo_queue_desc_addr_31_0: %x ",
			  mpdu_info->fr_ds,
			  mpdu_info->to_ds,
			  mpdu_info->encrypted,
			  mpdu_info->mpdu_retry,
			  mpdu_info->mpdu_sequence_number,
			  mpdu_info->epd_en,
			  mpdu_info->all_frames_shall_be_encrypted,
			  mpdu_info->encrypt_type,
			  mpdu_info->mesh_sta,
			  mpdu_info->bssid_hit,
			  mpdu_info->bssid_number,
			  mpdu_info->tid,
			  mpdu_info->pn_31_0,
			  mpdu_info->pn_63_32,
			  mpdu_info->pn_95_64,
			  mpdu_info->pn_127_96,
			  mpdu_info->peer_meta_data,
			  mpdu_info->rxpt_classify_info_details.reo_destination_indication,
			  mpdu_info->rxpt_classify_info_details.use_flow_id_toeplitz_clfy,
			  mpdu_info->rx_reo_queue_desc_addr_31_0);

	hal_verbose_debug(
			  "rx_mpdu_start tlv (3/5) - "
			  "rx_reo_queue_desc_addr_39_32: %x "
			  "receive_queue_number: %x "
			  "pre_delim_err_warning: %x "
			  "first_delim_err: %x "
			  "key_id_octet: %x "
			  "new_peer_entry: %x "
			  "decrypt_needed: %x "
			  "decap_type: %x "
			  "rx_insert_vlan_c_tag_padding: %x "
			  "rx_insert_vlan_s_tag_padding: %x "
			  "strip_vlan_c_tag_decap: %x "
			  "strip_vlan_s_tag_decap: %x "
			  "pre_delim_count: %x "
			  "ampdu_flag: %x "
			  "bar_frame: %x "
			  "mpdu_length: %x "
			  "first_mpdu: %x "
			  "mcast_bcast: %x "
			  "ast_index_not_found: %x "
			  "ast_index_timeout: %x ",
			  mpdu_info->rx_reo_queue_desc_addr_39_32,
			  mpdu_info->receive_queue_number,
			  mpdu_info->pre_delim_err_warning,
			  mpdu_info->first_delim_err,
			  mpdu_info->key_id_octet,
			  mpdu_info->new_peer_entry,
			  mpdu_info->decrypt_needed,
			  mpdu_info->decap_type,
			  mpdu_info->rx_insert_vlan_c_tag_padding,
			  mpdu_info->rx_insert_vlan_s_tag_padding,
			  mpdu_info->strip_vlan_c_tag_decap,
			  mpdu_info->strip_vlan_s_tag_decap,
			  mpdu_info->pre_delim_count,
			  mpdu_info->ampdu_flag,
			  mpdu_info->bar_frame,
			  mpdu_info->mpdu_length,
			  mpdu_info->first_mpdu,
			  mpdu_info->mcast_bcast,
			  mpdu_info->ast_index_not_found,
			  mpdu_info->ast_index_timeout);

	hal_verbose_debug(
			  "rx_mpdu_start tlv (4/5) - "
			  "power_mgmt: %x "
			  "non_qos: %x "
			  "null_data: %x "
			  "mgmt_type: %x "
			  "ctrl_type: %x "
			  "more_data: %x "
			  "eosp: %x "
			  "fragment_flag: %x "
			  "order: %x "
			  "u_apsd_trigger: %x "
			  "encrypt_required: %x "
			  "directed: %x "
			  "mpdu_frame_control_field: %x "
			  "mpdu_duration_field: %x "
			  "mac_addr_ad1_31_0: %x "
			  "mac_addr_ad1_47_32: %x "
			  "mac_addr_ad2_15_0: %x "
			  "mac_addr_ad2_47_16: %x "
			  "mac_addr_ad3_31_0: %x "
			  "mac_addr_ad3_47_32: %x ",
			  mpdu_info->power_mgmt,
			  mpdu_info->non_qos,
			  mpdu_info->null_data,
			  mpdu_info->mgmt_type,
			  mpdu_info->ctrl_type,
			  mpdu_info->more_data,
			  mpdu_info->eosp,
			  mpdu_info->fragment_flag,
			  mpdu_info->order,
			  mpdu_info->u_apsd_trigger,
			  mpdu_info->encrypt_required,
			  mpdu_info->directed,
			  mpdu_info->mpdu_frame_control_field,
			  mpdu_info->mpdu_duration_field,
			  mpdu_info->mac_addr_ad1_31_0,
			  mpdu_info->mac_addr_ad1_47_32,
			  mpdu_info->mac_addr_ad2_15_0,
			  mpdu_info->mac_addr_ad2_47_16,
			  mpdu_info->mac_addr_ad3_31_0,
			  mpdu_info->mac_addr_ad3_47_32);

	hal_verbose_debug(
			  "rx_mpdu_start tlv (5/5) - "
			  "mpdu_sequence_control_field: %x "
			  "mac_addr_ad4_31_0: %x "
			  "mac_addr_ad4_47_32: %x "
			  "mpdu_qos_control_field: %x "
			  "mpdu_ht_control_field: %x ",
			  mpdu_info->mpdu_sequence_control_field,
			  mpdu_info->mac_addr_ad4_31_0,
			  mpdu_info->mac_addr_ad4_47_32,
			  mpdu_info->mpdu_qos_control_field,
			  mpdu_info->mpdu_ht_control_field);
}

/**
 * hal_tx_desc_set_search_type - Set the search type value
 * @desc: Handle to Tx Descriptor
 * @search_type: search type
 *		     0 – Normal search
 *		     1 – Index based address search
 *		     2 – Index based flow search
 *
 * Return: void
 */
#ifdef TCL_DATA_CMD_2_SEARCH_TYPE_OFFSET
static void hal_tx_desc_set_search_type_generic(void *desc,
						uint8_t search_type)
{
	HAL_SET_FLD(desc, TCL_DATA_CMD_2, SEARCH_TYPE) |=
		HAL_TX_SM(TCL_DATA_CMD_2, SEARCH_TYPE, search_type);
}
#else
static void hal_tx_desc_set_search_type_generic(void *desc,
						uint8_t search_type)
{
}

#endif

/**
 * hal_tx_desc_set_search_index - Set the search index value
 * @desc: Handle to Tx Descriptor
 * @search_index: The index that will be used for index based address or
 *                flow search. The field is valid when 'search_type' is
 *                1 0r 2
 *
 * Return: void
 */
#ifdef TCL_DATA_CMD_5_SEARCH_INDEX_OFFSET
static void hal_tx_desc_set_search_index_generic(void *desc,
						 uint32_t search_index)
{
	HAL_SET_FLD(desc, TCL_DATA_CMD_5, SEARCH_INDEX) |=
		HAL_TX_SM(TCL_DATA_CMD_5, SEARCH_INDEX, search_index);
}
#else
static void hal_tx_desc_set_search_index_generic(void *desc,
						 uint32_t search_index)
{
}
#endif

/**
 * hal_tx_desc_set_cache_set_num_generic - Set the cache-set-num value
 * @desc: Handle to Tx Descriptor
 * @cache_num: Cache set number that should be used to cache the index
 *                based search results, for address and flow search.
 *                This value should be equal to LSB four bits of the hash value
 *                of match data, in case of search index points to an entry
 *                which may be used in content based search also. The value can
 *                be anything when the entry pointed by search index will not be
 *                used for content based search.
 *
 * Return: void
 */
#ifdef TCL_DATA_CMD_5_CACHE_SET_NUM_OFFSET
static void hal_tx_desc_set_cache_set_num_generic(void *desc,
						  uint8_t cache_num)
{
	HAL_SET_FLD(desc, TCL_DATA_CMD_5, CACHE_SET_NUM) |=
		HAL_TX_SM(TCL_DATA_CMD_5, CACHE_SET_NUM, cache_num);
}
#else
static void hal_tx_desc_set_cache_set_num_generic(void *desc,
						  uint8_t cache_num)
{
}
#endif

/**
 * hal_tx_set_pcp_tid_map_generic() - Configure default PCP to TID map table
 * @soc: HAL SoC context
 * @map: PCP-TID mapping table
 *
 * PCP are mapped to 8 TID values using TID values programmed
 * in one set of mapping registers PCP_TID_MAP_<0 to 6>
 * The mapping register has TID mapping for 8 PCP values
 *
 * Return: none
 */
static void hal_tx_set_pcp_tid_map_generic(struct hal_soc *soc, uint8_t *map)
{
	uint32_t addr, value;

	addr = HWIO_TCL_R0_PCP_TID_MAP_ADDR(
				SEQ_WCSS_UMAC_MAC_TCL_REG_OFFSET);

	value = (map[0] |
		(map[1] << HWIO_TCL_R0_PCP_TID_MAP_PCP_1_SHFT) |
		(map[2] << HWIO_TCL_R0_PCP_TID_MAP_PCP_2_SHFT) |
		(map[3] << HWIO_TCL_R0_PCP_TID_MAP_PCP_3_SHFT) |
		(map[4] << HWIO_TCL_R0_PCP_TID_MAP_PCP_4_SHFT) |
		(map[5] << HWIO_TCL_R0_PCP_TID_MAP_PCP_5_SHFT) |
		(map[6] << HWIO_TCL_R0_PCP_TID_MAP_PCP_6_SHFT) |
		(map[7] << HWIO_TCL_R0_PCP_TID_MAP_PCP_7_SHFT));

	HAL_REG_WRITE(soc, addr, (value & HWIO_TCL_R0_PCP_TID_MAP_RMSK));
}

/**
 * hal_tx_update_pcp_tid_generic() - Update the pcp tid map table with
 *					value received from user-space
 * @soc: HAL SoC context
 * @pcp: pcp value
 * @tid : tid value
 *
 * Return: void
 */
static
void hal_tx_update_pcp_tid_generic(struct hal_soc *soc,
				   uint8_t pcp, uint8_t tid)
{
	uint32_t addr, value, regval;

	addr = HWIO_TCL_R0_PCP_TID_MAP_ADDR(
				SEQ_WCSS_UMAC_MAC_TCL_REG_OFFSET);

	value = (uint32_t)tid << (HAL_TX_BITS_PER_TID * pcp);

	/* Read back previous PCP TID config and update
	 * with new config.
	 */
	regval = HAL_REG_READ(soc, addr);
	regval &= ~(HAL_TX_TID_BITS_MASK << (HAL_TX_BITS_PER_TID * pcp));
	regval |= value;

	HAL_REG_WRITE(soc, addr,
		      (regval & HWIO_TCL_R0_PCP_TID_MAP_RMSK));
}

/**
 * hal_tx_update_tidmap_prty_generic() - Update the tid map priority
 * @soc: HAL SoC context
 * @val: priority value
 *
 * Return: void
 */
static
void hal_tx_update_tidmap_prty_generic(struct hal_soc *soc, uint8_t value)
{
	uint32_t addr;

	addr = HWIO_TCL_R0_TID_MAP_PRTY_ADDR(
				SEQ_WCSS_UMAC_MAC_TCL_REG_OFFSET);

	HAL_REG_WRITE(soc, addr,
		      (value & HWIO_TCL_R0_TID_MAP_PRTY_RMSK));
}

/**
 * hal_rx_msdu_packet_metadata_get(): API to get the
 * msdu information from rx_msdu_end TLV
 *
 * @ buf: pointer to the start of RX PKT TLV headers
 * @ hal_rx_msdu_metadata: pointer to the msdu info structure
 */
static void
hal_rx_msdu_packet_metadata_get_generic(uint8_t *buf,
					void *pkt_msdu_metadata)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_end *msdu_end = &pkt_tlvs->msdu_end_tlv.rx_msdu_end;
	struct hal_rx_msdu_metadata *msdu_metadata =
		(struct hal_rx_msdu_metadata *)pkt_msdu_metadata;

	msdu_metadata->l3_hdr_pad =
		HAL_RX_MSDU_END_L3_HEADER_PADDING_GET(msdu_end);
	msdu_metadata->sa_idx = HAL_RX_MSDU_END_SA_IDX_GET(msdu_end);
	msdu_metadata->da_idx = HAL_RX_MSDU_END_DA_IDX_GET(msdu_end);
	msdu_metadata->sa_sw_peer_id =
		HAL_RX_MSDU_END_SA_SW_PEER_ID_GET(msdu_end);
}

/**
 * hal_rx_msdu_end_offset_get_generic(): API to get the
 * msdu_end structure offset rx_pkt_tlv structure
 *
 * NOTE: API returns offset of msdu_end TLV from structure
 * rx_pkt_tlvs
 */
static uint32_t hal_rx_msdu_end_offset_get_generic(void)
{
	return RX_PKT_TLV_OFFSET(msdu_end_tlv);
}

/**
 * hal_rx_attn_offset_get_generic(): API to get the
 * msdu_end structure offset rx_pkt_tlv structure
 *
 * NOTE: API returns offset of attn TLV from structure
 * rx_pkt_tlvs
 */
static uint32_t hal_rx_attn_offset_get_generic(void)
{
	return RX_PKT_TLV_OFFSET(attn_tlv);
}

/**
 * hal_rx_msdu_start_offset_get_generic(): API to get the
 * msdu_start structure offset rx_pkt_tlv structure
 *
 * NOTE: API returns offset of attn TLV from structure
 * rx_pkt_tlvs
 */
static uint32_t hal_rx_msdu_start_offset_get_generic(void)
{
	return RX_PKT_TLV_OFFSET(msdu_start_tlv);
}

/**
 * hal_rx_mpdu_start_offset_get_generic(): API to get the
 * mpdu_start structure offset rx_pkt_tlv structure
 *
 * NOTE: API returns offset of attn TLV from structure
 * rx_pkt_tlvs
 */
static uint32_t	hal_rx_mpdu_start_offset_get_generic(void)
{
	return RX_PKT_TLV_OFFSET(mpdu_start_tlv);
}

/**
 * hal_rx_mpdu_end_offset_get_generic(): API to get the
 * mpdu_end structure offset rx_pkt_tlv structure
 *
 * NOTE: API returns offset of attn TLV from structure
 * rx_pkt_tlvs
 */
static uint32_t	hal_rx_mpdu_end_offset_get_generic(void)
{
	return RX_PKT_TLV_OFFSET(mpdu_end_tlv);
}
#endif /* HAL_GENERIC_API_H_ */
