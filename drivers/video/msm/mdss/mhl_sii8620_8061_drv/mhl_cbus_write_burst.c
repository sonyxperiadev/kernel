/* kernel/drivers/video/msm/mdss/mhl_sii8620_8061_drv/mhl_cbus_write_burst.c
 *
 * Copyright (C) 2013 Sony Mobile Communications AB.
 * Copyright (C) 2013 Silicon Image Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include "si_8620_regs.h"
#include "mhl_sii8620_8061_device.h"
#include "mhl_cbus_control.h"
#include "mhl_platform.h"
#include "mhl_lib_infoframe.h"
#include "mhl_common.h"

static uint8_t	write_burst_data[MHL_SCRATCHPAD_SIZE];
static bool	gen2_write_burst_xmit_flag;

/* This function is called when mhl connection started. */
void cbus_wb_init(void)
{
	mhl_dev_clear_cbusp_cond_processing(DEV_WRITE_BURST_SEND);
}

/*
* Calculte checksum of the 16 byte block pointed to by the
* pointer passed as parameter
*
* p_HEV_VIC_block_data : Pointer to a 16 byte block whose checksum
* needs to be calculated
*
* RETURNS : true if chcksum is 0. false if not.
*/
bool cbus_wb_is_valid_checksum(uint8_t *p_HEV_VIC_block_data)
{
	uint8_t i;
	uint8_t checksum = 0;

	for (i = 0; i < MHL_SCRATCHPAD_SIZE; i++)
		checksum += p_HEV_VIC_block_data[i];

	if (checksum) {
		pr_err("%s: false, 0x%x\n", __func__, checksum);
		return false;
	}

	return true;
}

uint8_t *cbus_wb_get_stored_scrpad(uint8_t *size)
{
	*size =  ARRAY_SIZE(write_burst_data);
	return write_burst_data;
}

static uint8_t cbus_wb_get_vic(uint8_t *in_hev_vic, uint8_t *out_hev_vic)
{
	int i;
	int length		= 0;
	int HEV_VIC_MAX_SIZE	= 5;
	int POS_NUM_ENT		= 5;
	int POS_FIRST_VIC_DATA	= 7;
	uint8_t vic;

	if (!cbus_wb_is_valid_checksum(in_hev_vic))
		return 0;

	for (i = 0; i < in_hev_vic[POS_NUM_ENT] && i < HEV_VIC_MAX_SIZE; i++) {
		vic = in_hev_vic[POS_FIRST_VIC_DATA+(i*2)];
		if ((vic != 0) && (IsQualifiedMhlVIC(vic))){
			out_hev_vic[i] = vic;
			length++;
			pr_debug("%s: HEV VIC Data: %d\n",
					__func__, out_hev_vic[i]);
		}
	}

	pr_debug("%s: HEV VIC Data length: %d\n", __func__, length);
	return length;
}

static void cbus_wb_handle_scr_data(uint8_t *scrpad)
{
	BurstId_e burst_id;
	uint8_t length = 0;
	uint8_t out_data[MHL_SCRATCHPAD_SIZE];

	burst_id = (BurstId_e)(((uint16_t)scrpad[0]<<8)|((uint16_t)scrpad[1]));

	pr_debug("%s: BURST_ID: 0x%x\n", __func__, burst_id);

	switch (burst_id) {
	case burst_id_HEV_VIC:
		length = cbus_wb_get_vic(scrpad, out_data);
		mhl_device_edid_setup_sink_support_hev_vic(out_data, length);
		break;
	case burst_id_VC_ASSIGN:
		if (!cbus_wb_is_valid_checksum(scrpad)) {
			pr_err("%s: Bad checksum in virtual channel assign",
				__func__);
			return;
		}
		mhl_cbus_process_vc_assign(scrpad);
		break;
	case burst_id_3D_VIC:
	case burst_id_3D_DTD:
	case burst_id_HEV_DTDA:
	case burst_id_HEV_DTDB:
	case burst_id_VC_CONFIRM:
	case burst_id_AUD_DELAY:
	case burst_id_ADT_BURSTID:
	case burst_id_BIST_SETUP:
	case burst_id_BIST_RETURN_STAT:
	case burst_id_EMSC_SUPPORT:
	case burst_id_HID_PAYLOAD:
	case burst_id_BLK_RCV_BUFFER_INFO:
	case burst_id_BITS_PER_PIXEL_FMT:
	case LOCAL_ADOPTER_ID:
	case MHL_TEST_ADOPTER_ID:
	case burst_id_16_BITS_REQUIRED:
	default:
		break;
	}
}

void cbus_wb_event_handler(uint8_t intr_0)
{
	if (MHL_INT_REQ_WRT & intr_0) {
		pr_debug("%s: got REQ_WRT\n", __func__);
		set_cbus_command(MHL_SET_INT, MHL_RCHANGE_INT, MHL_INT_GRT_WRT);
	}

	if (MHL_INT_DSCR_CHG & intr_0) {
		pr_debug("%s: got DSCR_CHG\n", __func__);
		mhl_pf_read_reg_block(REG_PAGE_4_MHL_SCRPAD_0,
			ARRAY_SIZE(write_burst_data), write_burst_data);
		cbus_wb_handle_scr_data(write_burst_data);
	}

	if (MHL3_INT_FEAT_COMPLETE & intr_0) {
		pr_debug("%s: got FEAT_COMPLETE\n", __func__);
		mhl_device_edid_set_upstream_edid();
	}

	if (MHL3_INT_FEAT_REQ & intr_0) {
		pr_debug("%s: got MHL3_INT_FEAT_REQ\n", __func__);

		/*
		  FIXME
		  Should send EMSC_SUPPORT here,
		  according to MHL3 specifications.
		*/
		set_cbus_command(MHL_SET_INT, MHL_RCHANGE_INT,
					MHL3_INT_FEAT_COMPLETE);
	}
}

void cbus_wb_request_write_burst(
			uint8_t burst_offset, uint8_t length, uint8_t *data)
{
	if (mhl_device_is_peer_device_mhl3() &&
		!mhl_cbus_is_sink_support_scratchpad()) {
		pr_err("%s: failed SCRATCHPAD_NOT_SUPPORTED\n", __func__);
	} else if ((burst_offset + length) > MHL_SCRATCHPAD_SIZE) {
		pr_err("%s: invalid offset + length\n", __func__);
	} else {
		pr_debug("%s: Request Write Burst command.\n", __func__);
		set_cbus_command_block(
			MHL_WRITE_BURST, MHL_SCRATCHPAD_SIZE, 0, data);
	}
}

void cbus_wb_write_gen2_xfifo(uint8_t length, uint8_t *data)
{
	mhl_pf_write_reg_block(REG_PAGE_5_MDT_XMIT_WRITE_PORT, length, data);
	mhl_dev_set_cbusp_cond_processing(DEV_WRITE_BURST_SEND);
}

void cbus_wb_write_burst_send_done(void)
{
	pr_debug("%s: HAWB XFIFO empty. XFIFO_STAT: 0x%02x\n",
		__func__, mhl_pf_read_reg(REG_PAGE_5_MDT_XFIFO_STAT));
	mhl_dev_clear_cbusp_cond_processing(DEV_WRITE_BURST_SEND);
}

void cbus_wb_start_gen2_write_burst(void)
{
	mhl_pf_write_reg(REG_PAGE_5_MDT_INT_1_MASK,
		 BIT_MDT_RCV_TIMEOUT | BIT_MDT_RCV_SM_ABORT_PKT_RCVD
		 | BIT_MDT_RCV_SM_ERROR | BIT_MDT_XMIT_TIMEOUT
		 | BIT_MDT_XMIT_SM_ABORT_PKT_RCVD | BIT_MDT_XMIT_SM_ERROR);
	mhl_pf_write_reg(REG_PAGE_5_MDT_INT_0_MASK,
		 BIT_MDT_XFIFO_EMPTY | BIT_MDT_IDLE_AFTER_HAWB_DISABLE
		 | BIT_MDT_RFIFO_DATA_RDY);
}

void cbus_wb_enable_gen2_write_burst_xmit(void)
{
	/* enable Gen2 Write Burst interrupt, MSC and EDID interrupts. */
	if (!gen2_write_burst_xmit_flag) {

		mhl_pf_write_reg(REG_PAGE_5_MDT_XMIT_CONTROL,
			 BIT_PAGE_5_MDT_XMIT_CONTROL_MDT_XMIT_EN
			 |BIT_PAGE_5_MDT_XMIT_CONTROL_MDT_XMIT_FIXED_BURST_LEN
			);
		pr_debug("%s: enabled GEN2 xmit\n", __func__);
		gen2_write_burst_xmit_flag = true;
	}
}

void cbus_wb_disable_gen2_write_burst_xmit(void)
{
	if (gen2_write_burst_xmit_flag) {
		/*
		* disable Gen2 Write Burst engine to allow
		* normal CBUS traffic
		*/
		mhl_pf_write_reg(REG_PAGE_5_MDT_XMIT_CONTROL, 0);
		pr_debug("%s: disabled GEN2 xmit\n", __func__);
		gen2_write_burst_xmit_flag = false;
	}
}
