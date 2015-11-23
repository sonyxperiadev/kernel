/* vendor/semc/hardware/mhl/mhl_sii8620_8061_drv/mhl_cbus_control.h
 *
 * Copyright (C) 2013 Sony Mobile Communications Inc.
 * Copyright (C) 2013 Silicon Image Inc.
 *
 * Author: [Yasuyuki Kino <yasuyuki.kino@sonymobile.com>]
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef __MHL_CBUS_CONTROL_H__
#define __MHL_CBUS_CONTROL_H__

#include "mhl_sii8620_8061_devcap.h"

/* Current */
#define CURRENT_100MA			100000
#define CURRENT_500MA			500000
#define CURRENT_700MA			700000
#define CURRENT_900MA			900000
#define CURRENT_1500MA			1500000
#define CURRENT_2000MA			2000000

#define NUM_CBUS_EVENT_QUEUE_EVENTS 16
#define NUM_BLOCK_QUEUE_REQUESTS 4

/* Discovery Interrupt Mask Register */
#define REG_DISC_INTR_MASK				REG_CBUS_DISC_INTR0_MASK
#define BIT_RGND_READY_INT_MASK			BIT_CBUS_DISC_INTR0_MASK_CBUS_DISC_INTR0_MASK6
#define BIT_CBUS_MHL12_DISCON_INT_MASK	BIT_CBUS_DISC_INTR0_MASK_CBUS_DISC_INTR0_MASK5
#define BIT_CBUS_MHL3_DISCON_INT_MASK	BIT_CBUS_DISC_INTR0_MASK_CBUS_DISC_INTR0_MASK4
#define BIT_NOT_MHL_EST_INT_MASK		BIT_CBUS_DISC_INTR0_MASK_CBUS_DISC_INTR0_MASK3
#define BIT_MHL_EST_INT_MASK			BIT_CBUS_DISC_INTR0_MASK_CBUS_DISC_INTR0_MASK2
#define BIT_MHL3_EST_INT_MASK			BIT_CBUS_DISC_INTR0_MASK_CBUS_DISC_INTR0_MASK1

/* MSC Interrupt Mask Register */
#define REG_MSC_INTR_MASK			REG_CBUS_INT_0_MASK
#define BIT_CBUS_MSC_MT_DONE_NACK	BIT_CBUS_INT_0_MASK_CBUS_INT_0_MASK7
#define BIT_CBUS_MSC_MR_SET_INT		BIT_CBUS_INT_0_MASK_CBUS_INT_0_MASK6
#define BIT_CBUS_MSC_MR_WRITE_BURST	BIT_CBUS_INT_0_MASK_CBUS_INT_0_MASK5
#define BIT_CBUS_MSC_MR_MSC_MSG		BIT_CBUS_INT_0_MASK_CBUS_INT_0_MASK4
#define BIT_CBUS_MSC_MR_WRITE_STAT	BIT_CBUS_INT_0_MASK_CBUS_INT_0_MASK3
#define BIT_CBUS_HPD_CHG			BIT_CBUS_INT_0_MASK_CBUS_INT_0_MASK2
#define BIT_CBUS_MSC_MT_DONE		BIT_CBUS_INT_0_MASK_CBUS_INT_0_MASK1
#define BIT_CBUS_CNX_CHG			BIT_CBUS_INT_0_MASK_CBUS_INT_0_MASK0

/* EDID FIFO Interrupt Register */
#define REG_EDID_FIFO_INT_MASK				REG_INTR9_MASK
#define BIT_INTR9_EDID_ERROR				BIT_INTR9_STAT6
#define BIT_INTR9_EDID_DONE					BIT_INTR9_STAT5
#define BIT_INTR9_DEVCAP_DONE				BIT_INTR9_STAT4
#define BIT_INTR9_EDID_ERROR_MASK			BIT_INTR9_MASK6
#define BIT_INTR9_EDID_DONE_MASK			BIT_INTR9_MASK5
#define BIT_INTR9_DEVCAP_DONE_MASK			BIT_INTR9_MASK4
#define VAL_INTR9_DEVCAP_DONE_MASK_ENABLE	BIT_INTR9_MASK4

/* MSC Error Interrupt Register */

#define BIT_CBUS_CMD_ABORT		BIT_CBUS_INT_1_MASK_CBUS_INT_1_MASK6
#define BIT_CBUS_MSC_ABORT_RCVD	BIT_CBUS_INT_1_MASK_CBUS_INT_1_MASK3
#define BIT_CBUS_DDC_ABORT		BIT_CBUS_INT_1_MASK_CBUS_INT_1_MASK2
#define BIT_CBUS_CEC_ABORT		BIT_CBUS_INT_1_MASK_CBUS_INT_1_MASK1

/* Upstream TMDS Status Interrupt Register */

#define BIT_INTR5_CKDT_CHANGE					BIT_INTR5_STAT1
#define BIT_INTR5_SCDT_CHANGE					BIT_INTR5_STAT0

/* Upstream HDMI InforFrame Control Register */
#define	REG_RX_HDMI_CTRL2_DEFVAL_DVI			0x30
#define REG_RX_HDMI_CTRL2_DEFVAL_HDMI			0x38

/* Upstream InfoFrame Interupt Register */
#define BIT_INTR8_CEA_NEW_VSI					BIT_INTR8_MASK2
#define BIT_INTR8_CEA_NEW_AVI					BIT_INTR8_MASK1

/* Cbus Abort Reason */

#define BIT_CBUS_MSC_MT_ABORT_INT_MSC_MT_PEER_ABORT	BIT_MSC_MT_ABORT_INT_MSC_MT_ABORT_INT_STAT7
#define BIT_PAGE_CBUS_REG_MSC_MT_ABORT_INT_STAT5	BIT_MSC_MT_ABORT_INT_MSC_MT_ABORT_INT_STAT5
#define BIT_CBUS_MSC_MT_ABORT_INT_UNDEF_CMD			BIT_MSC_MT_ABORT_INT_MSC_MT_ABORT_INT_STAT3
#define BIT_CBUS_MSC_MT_ABORT_INT_TIMEOUT			BIT_MSC_MT_ABORT_INT_MSC_MT_ABORT_INT_STAT2
#define BIT_CBUS_MSC_MT_ABORT_INT_PROTO_ERR			BIT_MSC_MT_ABORT_INT_MSC_MT_ABORT_INT_STAT1
#define BIT_CBUS_MSC_MT_ABORT_INT_MAX_FAIL			BIT_MSC_MT_ABORT_INT_MSC_MT_ABORT_INT_STAT0

#define BIT_CBUS_DDC_PEER_ABORT	BIT_DDC_ABORT_INT_DDC_ABORT_INT_STAT7

/* Write Burst */
#define BIT_MDT_RFIFO_DATA_RDY			BIT_MDT_INT_0_MDT_INT_0_0
#define BIT_MDT_IDLE_AFTER_HAWB_DISABLE	BIT_MDT_INT_0_MDT_INT_0_2
#define BIT_MDT_XFIFO_EMPTY				BIT_MDT_INT_0_MDT_INT_0_3
#define BIT_MDT_RCV_TIMEOUT				BIT_MDT_INT_1_MDT_INT_1_0
#define BIT_MDT_RCV_SM_ABORT_PKT_RCVD	BIT_MDT_INT_1_MDT_INT_1_1
#define BIT_MDT_RCV_SM_ERROR			BIT_MDT_INT_1_MDT_INT_1_2
#define BIT_MDT_XMIT_TIMEOUT			BIT_MDT_INT_1_MDT_INT_1_5
#define BIT_MDT_XMIT_SM_ABORT_PKT_RCVD	BIT_MDT_INT_1_MDT_INT_1_6
#define BIT_MDT_XMIT_SM_ERROR			BIT_MDT_INT_1_MDT_INT_1_7

#if 0 /* FIXME for BIST */
#define BIST_PENDING_IN_PROGRESS			0x01
#define BIST_PENDING_DEFER_CBUS_MODE_UP		0x02
#endif

/*
 * The typedef is shared with test code.
 * Only code of mhl_cbus_control.c and
 * test code must use it.
 */
typedef enum {
	DEVCAP_ALL_READ_DONE,
	SENT_PATH_EN_1,
	RCV_HPD,
	UPSTREAM_VIDEO_READY,
	RCV_DCAP_CHG,
	READ_ECBUS_SPEED,
	HDCP_READY,
	TMDS_READY
#if 0 /* FIXME for BIST */
	, BIST_DONE
#endif
} CBUS_CONTROL_COND;

struct block_buffer_info_t {
	uint8_t	*buffer;
	uint8_t payload_offset;
	size_t req_size;
};

/* flag control*/
void init_cond_in_cbus_control(void);
void set_cond_in_cbus_control(CBUS_CONTROL_COND cond);
void clear_cond_in_cbus_control(CBUS_CONTROL_COND cond);
bool get_cond_in_cbus_control(CBUS_CONTROL_COND cond);
void show_cond_in_cbus_control(void);

/**/
void init_cbus_queue(void);
void mhl_msc_init(void);
void mhl_msc_cbus_communication_start(void);
void mhl_cbus_communication_eCBUS_start(void);
void mhl_msc_command_done(bool is_Msc_Msg_Ack);
void mhl_msc_hpd_receive(void);
void mhl_msc_write_stat_receive(void);
void mhl_msc_msg_receive(void);
void mhl_msc_set_int_receive(void);
void mhl_cbus_abort(uint8_t cbus_err_int);
int  mhl_cbus_control_initialize(void);
void mhl_cbus_control_release(void);
uint8_t mhl_get_tdm_virt_chan_slot_counts(enum tdm_vc_assignments vc);
void send_link_mode_to_sink(void);
void set_link_mode(u8 link_mode);
u8 get_link_mode(void);
int mhl_cbus_get_highest_tmds_link_speed(void);
void mhl_cbus_set_lowest_tmds_link_speed(uint32_t pixel_clock_frequency,
											uint8_t bits_per_pixel);
bool mhl_cbus_is_sink_support_scratchpad(void);
void mhl_cbus_process_vc_assign(uint8_t *write_burst_data);
void mhl_pf_get_block_buffer_info(
		struct block_buffer_info_t  *block_buffer_info);


/**/
bool set_cbus_command(uint8_t command, uint8_t reg, uint8_t reg_data);
bool set_cbus_command_block(
	uint8_t command, uint8_t length, uint8_t burst_offset,
	void *buffer);
void exe_cbus_command(void);

/**/
void init_devcap(void);
void set_default_devcap(void);
uint8_t *get_current_devcap(void);
void change_devcap(uint8_t *devcap_values);
//int activate_hpd(void);
int deactivate_hpd(void);

/*
 * cbus write burst : public
 */
void cbus_wb_init(void);
void cbus_wb_event_handler(uint8_t intr_0);
void cbus_wb_request_write_burst(
	uint8_t burst_offset, uint8_t length, uint8_t *data);
void cbus_wb_write_gen2_xfifo(uint8_t length, uint8_t *data);
void cbus_wb_start_gen2_write_burst(void);
void cbus_wb_write_burst_send_done(void);
void cbus_wb_enable_gen2_write_burst_xmit(void);
void cbus_wb_disable_gen2_write_burst_xmit(void);

/* eMSC */
void mhl_tx_drv_enable_emsc_block(void);
int block_input_buffer_available(void);
int alloc_block_input_buffer(uint8_t **pbuffer);
void set_block_input_buffer_length(int block, int length);
void add_received_byte_count(uint16_t size);
void add_peer_blk_rx_buffer_avail(unsigned long size);
int block_input_buffer_available(void);
void mhl_tx_initialize_block_transport(void);
void mhl_tx_push_block_transactions(void);
void *mhl_tx_get_sub_payload_buffer(uint8_t size);
void mhl_tx_emsc_received(void);
void mhl_tx_send_blk_rcv_buf_info(void);
uint16_t mhl_tx_drv_get_blk_rcv_buf_size(void);

/* DCAP timer function */
void mhl_dcap_rdy_timer_start(void);

u8 *mhl_cbus_get_devcap(void);

#if 0 /* FIXME for BIST */
void mhl_cbus_set_bist_pending_flags(uint8_t val);
uint8_t mhl_cbus_get_bist_pending_flags(void);
#endif

#endif /* __MHL_CBUS_CONTROL_H__ */
