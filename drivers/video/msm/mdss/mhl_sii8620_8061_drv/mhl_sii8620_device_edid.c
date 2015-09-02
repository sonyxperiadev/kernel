/* kernel/drivers/video/msm/mdss/mhl_sii8620_8061_drv/mhl_sii8620_device_edid.c
 *
 * Copyright (C) 2013 Sony Mobile Communications AB.
 * Copyright (C) 2013 Silicon Image Inc.
 *
 * Author: [Hirokuni Kawasaki <hirokuni.kawasaki@sonymobile.com>]
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include "linux/string.h"

#include "mhl_sii8620_8061_device.h"
#include "si_8620_regs.h"
#include "mhl_platform.h"
#include "mhl_common.h"
#include <linux/delay.h>
#include <linux/errno.h>
#include "mhl_lib_timer.h"
#include "mhl_cbus_control.h"

#pragma GCC diagnostic ignored "-Wunused-function"

/* SCDT */
#define BIT_INTR_SCDT_CHANGE	BIT_PAGE_0_INTR5_INTR5_STAT0

/* EDID */
#define REG_EDID_FIFO_INT_MASK	REG_PAGE_2_INTR9_MASK /*TX_PAGE_2, 0xE1*/
#define BIT_INTR9_EDID_ERROR_MASK	BIT_PAGE_2_INTR9_MASK_INTR9_MASK6
#define BIT_INTR9_EDID_DONE_MASK	BIT_PAGE_2_INTR9_MASK_INTR9_MASK5
#define BIT_INTR9_DEVCAP_DONE_MASK	BIT_PAGE_2_INTR9_MASK_INTR9_MASK4
#define VAL_INTR9_EDID_ERROR_MASK	VAL_PAGE_2_INTR9_MASK_INTR9_MASK6_ENABLE
#define VAL_INTR9_EDID_DONE_MASK	VAL_PAGE_2_INTR9_MASK_INTR9_MASK5_ENABLE

#define BIT_INTR9_EDID_ERROR	BIT_PAGE_2_INTR9_INTR9_STAT6
#define BIT_INTR9_EDID_DONE	BIT_PAGE_2_INTR9_INTR9_STAT5
#define BIT_INTR9_DEVCAP_DONE	BIT_PAGE_2_INTR9_INTR9_STAT4

/* 6s is enough to clear edid done */
#define MHL_CRL_EDID_DONE_TIME 6000

typedef enum {
	block_0 = 0,
	block_1 = 128,
} EDID_FIFO_BLOCK_OFFSET;

static bool edid_is_ext_block(uint8_t *edid);

#define IS_BLOCK_0(_edid)   mhl_edid_check_edid_header(_edid)
#define IS_EXT_BLOCK(_edid) edid_is_ext_block(_edid)



/*[vic], [H.Pixel], [Aspect ratio | Refresh Rate], est timing1, 2, 3 */
/* todo separate standard timing info */
#define HDMI_VFRMT_640x480p60_4_3_TIMING    {1,  (uint32_t)0x31, (uint32_t)0x40, (uint32_t)0x20, 0, 0}
#define HDMI_VFRMT_720x480p60_16_9_TIMING   {3,  (uint32_t)0x3B, (uint32_t)0xC0, 0, 0, 0}
#define HDMI_VFRMT_1280x720p60_16_9_TIMING  {4,  (uint32_t)0x81, (uint32_t)0xC0, 0, 0, 0}
#define HDMI_VFRMT_1920x1080p60_16_9_TIMING	{16, (uint32_t)0xD1, (uint32_t)0xC0, 0, 0, 0}
#define HDMI_VFRMT_1920x1080p24_16_9_TIMING {32, (uint32_t)0xD1, (uint32_t)0xE0, 0, 0, 0}
#define HDMI_VFRMT_1920x1080p30_16_9_TIMING {34, (uint32_t)0xD1, (uint32_t)0xE0, 0, 0, 0}
#define HDMI_VFRMT_3840x2160p24_16_9_TIMING {93, (uint32_t)0xC1, (uint32_t)0xE0, 0, 0, 0}
#define HDMI_VFRMT_3840x2160p30_16_9_TIMING {95, (uint32_t)0xC1, (uint32_t)0xE0, 0, 0, 0}

#define RX_DPD_BITS (BIT_PAGE_0_DPD_PDNRX12 \
			  | BIT_PAGE_0_DPD_PDIDCK_N \
			  | BIT_PAGE_0_DPD_PD_MHL_CLK_N)

static const struct mhl_video_timing_info mhl_1_2_support_video[SUPPORT_MHL_1_2_VIDEO_NUM] = {
	HDMI_VFRMT_640x480p60_4_3_TIMING,
	HDMI_VFRMT_720x480p60_16_9_TIMING,
	HDMI_VFRMT_1280x720p60_16_9_TIMING,
	HDMI_VFRMT_1920x1080p60_16_9_TIMING,
	HDMI_VFRMT_1920x1080p24_16_9_TIMING,
	HDMI_VFRMT_1920x1080p30_16_9_TIMING
};

static const struct mhl_video_timing_info mhl_3_support_video[SUPPORT_MHL_3_VIDEO_NUM] = {
	/* same as MHL 1/2*/
	HDMI_VFRMT_640x480p60_4_3_TIMING,
	HDMI_VFRMT_720x480p60_16_9_TIMING,
	HDMI_VFRMT_1280x720p60_16_9_TIMING,
	HDMI_VFRMT_1920x1080p60_16_9_TIMING,
	HDMI_VFRMT_1920x1080p24_16_9_TIMING,
	HDMI_VFRMT_1920x1080p30_16_9_TIMING,
	/* newly 4k vic is added to MHL 1/2 in MHL3 */
	HDMI_VFRMT_3840x2160p24_16_9_TIMING,
	HDMI_VFRMT_3840x2160p30_16_9_TIMING
};

/*
 * supported display info. high quality display info must be aligned in the low quality order.
 * (index 0 is low quality)
 *
 * NOTE : "quality" is currently based on resolution size.
 * High resolution is currently high quality.
 *
 * TODO : it must be meged with MHL1/2 and MHL3 support display information!!
 */
const struct hdmi_edid_video_mode_property_type
	preferred_disp_info[] = {

		/* All 640 H Active */
		{HDMI_VFRMT_640x480p60_4_3, 640, 480, false, 800, 160, 525, 45,
		31465, 59940, 25175, 59940, true},

		{HDMI_VFRMT_640x480p60_4_3, 640, 480, false, 800, 160, 525, 45,
		31500, 60000, 25200, 60000, true},

		/* All 720 H Active */
		{HDMI_VFRMT_720x480p60_4_3,  720, 480, false, 858, 138, 525, 45,
		31465, 59940, 27000, 59940, true},

		{HDMI_VFRMT_1280x720p60_16_9,  1280, 720, false, 1650, 370, 750, 30,
		44955, 59940, 74176, 59940, false},

		{HDMI_VFRMT_1920x1080p24_16_9, 1920, 1080, false, 2750, 830, 1125,
		45, 27000, 24000, 74250, 24000, false},

		{HDMI_VFRMT_1920x1080p30_16_9, 1920, 1080, false, 2200, 280, 1125,
		45, 33716, 29970, 74176, 30000, false}
};

/* EDID retry */
#define EDID_RETRY_MAX 3
static int edid_retry_cnt;

static uint8_t mhl_sink_sprt_hev_vic[20];

/* support vic for mhl 1/2 */
static uint8_t mhl_1_2_support_vic_array[SUPPORT_MHL_1_2_VIDEO_NUM];

/* support vic for mhl 3 */
static uint8_t mhl_3_support_vic_array[SUPPORT_MHL_3_VIDEO_NUM];

static void mhl_dev_edid_setup_support_video(void)
{
	int i = 0;

	pr_debug("%s:mhl1/2 vic setup\n", __func__);
	for (i = 0; i < SUPPORT_MHL_1_2_VIDEO_NUM; i++) {
		mhl_1_2_support_vic_array[i] = mhl_1_2_support_video[i].vic;
		pr_debug("set support vic=%d\n", mhl_1_2_support_vic_array[i]);
	}

	pr_debug("%s:mhl3 vic setup\n", __func__);
	for (i = 0; i < SUPPORT_MHL_3_VIDEO_NUM; i++) {
		mhl_3_support_vic_array[i] = mhl_3_support_video[i].vic;
		pr_debug("set support vic=%d\n", mhl_3_support_vic_array[i]);
	}
}

static bool isMhlSuptFilter;
static void mhl_dev_edid_mhlsink_hev_init(void);


static void edid_filter_prune_ext_blk(uint8_t *edid);
static void mhl_sii8620_device_set_edid_block_size(uint8_t size);
static void edid_filter_prune_block_0(uint8_t *blk0);

/* DEBUG */
#ifdef EDID_DATA_DEBUG_PRINT
#define DUMP_EDID_BLOCK(override, pData, length) dump_EDID_block_impl(__func__, __LINE__, override, (uint8_t *)pData, length);
static void dump_EDID_block_impl(const char *pszFunction, int iLineNum, uint8_t override, uint8_t *pData, uint16_t length)
{
	uint16_t i;
	pr_debug("%s:%d EDID DATA:\n", pszFunction, iLineNum);

	for (i = 0; i < length; ) {
		uint16_t j;
		uint16_t temp = i;

		for (j = 0; (j < 16) && (i < length); ++j, ++i)
			printk("%02X ", pData[i]);

		printk(" | ");

		for (j = 0; (j < 16) && (temp < length); ++j, ++temp)
			printk("%c", ((pData[temp] >= ' ') && (pData[temp] <= 'z')) ? pData[temp] : '.');

		printk("\n");
	}
}
#else
#define DUMP_EDID_BLOCK(override, pData, length) /* nothing to do */
#endif

static uint8_t edid_block_size;

struct mhl_sii8620_device_edid_context {
	bool isReleased;
	uint8_t edid_fifo_block_number;
	int current_edid_request_block;
	uint8_t edid_block_data[EDID_BLOCK_MAX_NUM*EDID_BLOCK_SIZE];
	void *edid_done_timer;
	bool is_request_queued;
	bool is_Receive_EDID_DONE;
};

static struct mhl_sii8620_device_edid_context edid_context;

/* edid fifo control */
static void set_current_edid_request_block(int block_num)
{
	pr_debug("%s():block_num=%d\n", __func__, block_num);
	edid_context.current_edid_request_block = block_num;
}

static uint8_t increment_current_edid_request_block(void)
{
	return ++edid_context.current_edid_request_block;
}

static uint8_t get_current_edid_request_block(void)
{
	pr_debug("%s():%d\n",
		__func__,
		edid_context.current_edid_request_block);

	return edid_context.current_edid_request_block;
}

static uint8_t *get_stored_edid_block(void)
{
	return edid_context.edid_block_data;
}

int mhl_sii8620_device_edid_get_block_number(void)
{
	return edid_context.edid_fifo_block_number;
}

int mhl_sii8620_device_get_edid_fifo_partial_block(
	uint8_t start,
	uint8_t length,
	uint8_t *edid_buf)
{
	int ret_val;
	int offset;

	if (edid_context.edid_fifo_block_number & 0x01)
		offset = edid_block_size; /*128 byte*/
	else
		offset = 0;

	pr_debug("%s: initial offset=%d\n", __func__, offset);

	offset += start;
	if (edid_block_size == (offset + length))
		edid_context.edid_fifo_block_number++;

	pr_debug("%s: block_num=%d, offset=%d, length = %d, start=%d\n",
		 __func__,
		 edid_context.edid_fifo_block_number,
		 offset, length, start);

	mhl_pf_modify_reg(REG_PAGE_2_EDID_CTRL,
				BIT_PAGE_2_EDID_CTRL_DEVCAP_SEL,
				VAL_PAGE_2_EDID_CTRL_DEVCAP_SELECT_EDID);

	mhl_pf_write_reg(REG_PAGE_2_EDID_FIFO_ADDR, (uint8_t)offset);

	ret_val = mhl_pf_read_reg_block(REG_PAGE_2_EDID_FIFO_RD_DATA
								, length
								, edid_buf);

	if (mhl_dev_is_set_hpd()) {
		return MHL_SUCCESS;
	} else {
		pr_err("%s:No HPD ret_val:0x%02x\n", __func__, ret_val);
		return MHL_FAIL;
	}
}

void mhl_sii8620_device_edid_reset_ddc_fifo(void)
{
#ifndef MHL_DDC_RESET
	uint8_t	ddc_status;

	ddc_status = (uint8_t)mhl_pf_read_reg(REG_PAGE_0_DDC_STATUS);/*0,f2*/

	mhl_pf_modify_reg(REG_PAGE_0_LM_DDC,
					BIT_PAGE_0_LM_DDC_SW_TPI_EN,
					VAL_PAGE_0_LM_DDC_SW_TPI_EN_ENABLED);

	if (BIT_PAGE_0_DDC_STATUS_DDC_NO_ACK & ddc_status) {
		pr_warn("Clearing DDC ack status\n");
		mhl_pf_write_reg(REG_PAGE_0_DDC_STATUS,
			(uint8_t)(ddc_status & ~BIT_PAGE_0_DDC_STATUS_DDC_NO_ACK));
	}

	mhl_pf_modify_reg(REG_PAGE_0_DDC_CMD,
					MSK_PAGE_0_DDC_CMD_DDC_CMD,
					VAL_PAGE_0_DDC_CMD_DDC_CMD_CLEAR_FIFO);

	mhl_pf_modify_reg(REG_PAGE_0_LM_DDC,
					BIT_PAGE_0_LM_DDC_SW_TPI_EN,
					VAL_PAGE_0_LM_DDC_SW_TPI_EN_DISABLED);
#endif
}

/* block_number limits is 3 (0 - 3). 4 will be failed. */
int mhl_sii8620_device_edid_read_request(uint8_t block_number)
{
	pr_info("%s():request block number=%d\n", __func__, block_number);

	if (block_number >= EDID_BLOCK_MAX_NUM) {
		pr_err("%s:invalid blk number=%d\n", __func__, block_number);
		return MHL_FAIL;
	}

	if (mhl_dev_is_set_hpd()) {
		mhl_pf_write_reg(REG_PAGE_0_DDC_MANUAL, 0x00);
		mhl_pf_write_reg(REG_080C_HDCP1X_LB_BIST, 0x00);

		mhl_pf_write_reg(REG_PAGE_2_EDID_CTRL,
			VAL_PAGE_2_EDID_CTRL_EDID_PRIME_VALID_DISABLE
			| VAL_PAGE_2_EDID_CTRL_DEVCAP_SELECT_EDID
			| VAL_PAGE_2_EDID_CTRL_EDID_FIFO_ADDR_AUTO_ENABLE
			| ((block_number & 0x01) << 2)
			| VAL_PAGE_2_EDID_CTRL_EDID_MODE_EN_ENABLE);

		mhl_sii8620_device_edid_reset_ddc_fifo();

		mhl_pf_write_reg(REG_PAGE_5_DISC_STAT1, 0x08);
		mhl_pf_modify_reg(REG_PAGE_5_DISC_CTRL5,
			BIT_PAGE_5_DISC_CTRL5_DSM_OVRIDE,
			BIT_PAGE_5_DISC_CTRL5_DSM_OVRIDE);

		if (block_number == 0) {
			pr_debug("%s()-block 0\n", __func__);
			mhl_pf_modify_reg(REG_EDID_FIFO_INT_MASK,
				BIT_INTR9_EDID_ERROR_MASK | BIT_INTR9_EDID_DONE_MASK,
				VAL_INTR9_EDID_ERROR_MASK | VAL_INTR9_EDID_DONE_MASK);

			mhl_pf_write_reg(REG_PAGE_2_TPI_CBUS_START,
					 BIT_PAGE_2_TPI_CBUS_START_GET_EDID_START_0);
		} else {
			uint8_t param = (uint8_t)(1 << (block_number-1));
			pr_debug("%s:EDID HW Assist:Reg %02X:%02x to %02X\n",
				 __func__, REG_PAGE_2_EDID_START_EXT, param);
			mhl_pf_write_reg(REG_PAGE_2_EDID_START_EXT, param);
		}
		set_current_edid_request_block(block_number);
	} else {
		pr_warn("%s: sink does not set HPD\n", __func__);
		return MHL_FAIL;
	}
	return MHL_SUCCESS;
}

static void mhl_device_edid_init_edid_block_info(void)
{
	edid_context.edid_fifo_block_number = 0;
	set_current_edid_request_block(-1);
	memset((void *)edid_context.edid_block_data, 0, sizeof(edid_context.edid_block_data));
}


void mhl_device_edid_init_edid_done_resource(void)
{
	/*
	 * TODO : This implementation must be replaced with
	 * mhl_sii8620_device_edid_init() later.
	 * Currently, call mhl_sii8620_device_edid_release() and
	 * that xxx_edid_init() continuously has some risk of leaking
	 * timer resource. that's why temporary this API was created
	 * to reset only EDID_DONE resource.
	 */
	if (mhl_lib_timer_stop(edid_context.edid_done_timer) != 0)
		pr_warn("%s:timer stop fail\n", __func__);

	edid_context.is_Receive_EDID_DONE = true;
}

static int mhl_edid_start_edid_request(void)
{
	uint8_t req_block;

	if (mhl_lib_timer_start(edid_context.edid_done_timer,
				MHL_CRL_EDID_DONE_TIME) != 0)
		pr_err("%s: Failed to start EDID timer!.\n",
						__func__);

	req_block = get_current_edid_request_block();
	return mhl_sii8620_device_edid_read_request(req_block);
}

void mhl_device_start_edid_read(void)
{
	pr_debug("%s()\n", __func__);

	/* edid read must not be executed in hdcp engine activating,
	   otherwise the edid data could be broken.
	   Stop_video()  internally disables hdcp function.
	*/
	stop_video();
	drive_hpd_low();

	mhl_device_edid_init_edid_block_info();
	mhl_dev_set_cbusp_cond_processing(DEV_EDID_READ);

	mhl_dev_edid_mhlsink_hev_init();

	if (edid_context.is_Receive_EDID_DONE == false) {
		pr_debug("%s:queued blk 0 req\n", __func__);
		edid_context.is_request_queued = true;
		return;
	}
	edid_context.is_request_queued = false;
	edid_context.is_Receive_EDID_DONE = false;

	set_current_edid_request_block(0);
	mhl_edid_start_edid_request();
}

static void mhl_device_clear_edid_done_timer(void *callback_param)
{
	pr_warn("%s:expired\n", __func__);
	edid_context.is_Receive_EDID_DONE = true;

	mhl_dev_clear_cbusp_cond_processing(DEV_EDID_READ);

	mhl_device_chip_reset();
}

void mhl_sii8620_device_edid_init(void)
{
	int ret;

	pr_debug("%s()\n", __func__);
	mhl_sii8620_device_set_edid_block_size(EDID_BLOCK_SIZE);
	mhl_lib_edid_set_edid((const uint8_t *)get_stored_edid_block());
	mhl_device_edid_init_edid_block_info();

	edid_context.is_request_queued = false;
	edid_context.is_Receive_EDID_DONE = true;

	ret = mhl_lib_timer_create(mhl_device_clear_edid_done_timer,
					NULL,
					&edid_context.edid_done_timer);
	if (ret != 0)
		pr_err("%s: Failed to allocate EDID timer!\n", __func__);

	mhl_dev_edid_setup_support_video();

	mhl_dev_edid_mhlsink_hev_init();

	edid_retry_cnt = 0;
}

void mhl_sii8620_device_edid_release(void)
{
	int ret;

	ret = mhl_lib_timer_delete(&edid_context.edid_done_timer);
	if (ret != 0)
		pr_err("%s: Failed to release EDID timer!\n", __func__);
}

static void mhl_sii8620_device_set_edid_block_size(uint8_t size)
{
	pr_debug("%s()\n", __func__);
	edid_block_size = size;
	pr_debug("%s:edid_block_size:%d\n", __func__, edid_block_size);
}

static bool mhl_dev_edid_is_fifo_hw_good_cond(uint8_t *edid_buf)
{
	int i;
	for (i = 0; i < EDID_BLOCK_SIZE - 1; i++)
		if (*(edid_buf + i) != *(edid_buf + i + 1))
			return true;

	pr_warn("%s:edid fifo is in strange condition\n", __func__);
	return false;
}

static int read_edid_data_from_edid_fifo(uint8_t *edid_buf, EDID_FIFO_BLOCK_OFFSET block_offset)
{
	int ret_val;
	uint8_t offset;

	offset = block_offset;

	pr_warn("%s.%d:block offset :%d,fifo block number : %d ,block size:%d\n",
		 __func__, __LINE__,
		 offset, edid_context.edid_fifo_block_number, edid_block_size);

	edid_context.edid_fifo_block_number++;
	mhl_pf_write_reg(REG_PAGE_2_EDID_FIFO_ADDR, offset);

	ret_val = mhl_pf_read_reg_block(REG_PAGE_2_EDID_FIFO_RD_DATA
		, EDID_BLOCK_SIZE
		, edid_buf);

	DUMP_EDID_BLOCK(0, edid_buf, EDID_BLOCK_SIZE);

	if (!mhl_lib_edid_is_valid_checksum(edid_buf))
		return MHL_FAIL;

	if (!mhl_dev_edid_is_fifo_hw_good_cond(edid_buf))
		return MHL_FAIL;

	if (mhl_dev_is_set_hpd())
		return MHL_SUCCESS;
	else
		return MHL_FAIL;
}

static int mhl_sii8620_device_get_edid_fifo_next_block(uint8_t *edid_buf)
{
	int ret_val;
	uint8_t offset;

	offset = (uint8_t) (edid_block_size * (edid_context.edid_fifo_block_number & 0x01));
	pr_debug("offset :%d, edid_context.edid_fifo_block_number : %d, edid_block_size:%d\n", offset, edid_context.edid_fifo_block_number, edid_block_size);

	edid_context.edid_fifo_block_number++;

	mhl_pf_modify_reg(REG_PAGE_2_EDID_CTRL,
				BIT_PAGE_2_EDID_CTRL_DEVCAP_SEL,
				VAL_PAGE_2_EDID_CTRL_DEVCAP_SELECT_EDID);

	mhl_pf_write_reg(REG_PAGE_2_EDID_FIFO_ADDR, offset);

	ret_val = mhl_pf_read_reg_block(REG_PAGE_2_EDID_FIFO_RD_DATA
		, EDID_BLOCK_SIZE
		, edid_buf);

	if (mhl_dev_is_set_hpd()) {
		pr_debug("Done reading EDID from FIFO using HW_ASSIST ret_val:0x%02x\n", ret_val);
		return MHL_SUCCESS;
	} else {
		pr_err("No HPD ret_val:0x%02x\n", ret_val);
		return MHL_FAIL;
	}
}

static void mhl_sii8620_device_init_rx_regs(void)
{
	/* power up the RX */
	mhl_pf_modify_reg(REG_PAGE_0_DPD,
				RX_DPD_BITS, RX_DPD_BITS);

	/* TODO: add to PR. Default for 2A4 is 0x0f */
	mhl_pf_write_reg(REG_PAGE_2_RX_HDMI_CTRL3, 0x00);

	/*
	 * Before exposing the EDID to upstream device,setup to drop all
	 * packets. This ensures we do not get Packet Overflow interrupt.
	 * Dropping all packets means we still get the AVIF interrupts which is
	 * crucial. Packet filters must be disabled until after TMDS is enabled.
	 */
	mhl_pf_write_reg(REG_PAGE_2_PKT_FILTER_0, 0xFF);
	mhl_pf_write_reg(REG_PAGE_2_PKT_FILTER_1, 0xFF);

	/* 0x231 def=0A. changed on 1/24/2014 per char team */
	mhl_pf_write_reg(REG_PAGE_2_ALICE0_BW_I2C, 0x06);

	mhl_pf_modify_reg(REG_PAGE_2_RX_HDMI_CLR_BUFFER,
				BIT_PAGE_2_RX_HDMI_CLR_BUFFER_VSI_CLR_EN,
				VAL_PAGE_2_RX_HDMI_CLR_BUFFER_VSI_CLR_EN_CLEAR);

}

/* we are done with the EDID for now.
   We now expect to start doing HDCP, which can
   destroy the contents of our EDID buffer,
   so do another EDID read, which we know will fail,
   but that will reset the DDC fifo
   in such a way as to leave the buffer contents alone
*/
void edid_hw_sm_clean_up(void)
{
	uint8_t	ddc_status, cbus_status;
	mhl_pf_write_reg(REG_080C_HDCP1X_LB_BIST, BIT_080C_HDCP1X_LB_BIST);
	mhl_pf_write_reg(REG_PAGE_0_DDC_MANUAL,
		BIT_PAGE_0_DDC_MANUAL_MAN_DDC);
	mhl_pf_write_reg(REG_PAGE_2_INTR9, 0xFF);

	/* Disable EDID interrupt */
	mhl_pf_write_reg(REG_PAGE_2_INTR9_MASK, 0x00);

	cbus_status = mhl_pf_read_reg(REG_PAGE_5_CBUS_STATUS);
	if (!(BIT_PAGE_5_CBUS_STATUS_CBUS_CONNECTED & cbus_status)) {
		mhl_pf_write_reg(REG_PAGE_5_DISC_STAT1, 0x08);
		mhl_pf_modify_reg(REG_PAGE_5_DISC_CTRL5,
			BIT_PAGE_5_DISC_CTRL5_DSM_OVRIDE,
			BIT_PAGE_5_DISC_CTRL5_DSM_OVRIDE);
	}

	mhl_pf_write_reg(REG_PAGE_2_TPI_CBUS_START,
		BIT_PAGE_2_TPI_CBUS_START_GET_EDID_START_0);
	ddc_status = mhl_pf_read_reg(REG_PAGE_0_DDC_STATUS);

	if (!(BIT_PAGE_5_CBUS_STATUS_CBUS_CONNECTED & cbus_status)) {
		mhl_pf_modify_reg(REG_PAGE_5_DISC_CTRL5,
			BIT_PAGE_5_DISC_CTRL5_DSM_OVRIDE,
			0);
	}

	mhl_pf_write_reg(REG_PAGE_2_INTR9, 0xFF);
	mhl_pf_write_reg(REG_PAGE_0_DDC_MANUAL, 0x00);
	mhl_pf_write_reg(REG_080C_HDCP1X_LB_BIST, 0x00);

	ddc_status = mhl_pf_read_reg(REG_PAGE_0_DDC_STATUS);

	mhl_pf_modify_reg(REG_PAGE_0_LM_DDC,
		BIT_PAGE_0_LM_DDC_SW_TPI_EN,
		VAL_PAGE_0_LM_DDC_SW_TPI_EN_DISABLED);

	if (BIT_PAGE_0_DDC_STATUS_DDC_BUS_LOW & ddc_status) {
		pr_debug("%s: Clearing DDC ack status\n", __func__);
		mhl_pf_write_reg(REG_PAGE_0_DDC_STATUS,
			ddc_status &
			~BIT_PAGE_0_DDC_STATUS_DDC_BUS_LOW);
	}

	mhl_pf_modify_reg(REG_PAGE_0_LM_DDC,
		BIT_PAGE_0_LM_DDC_SW_TPI_EN,
		VAL_PAGE_0_LM_DDC_SW_TPI_EN_ENABLED);

}

static int set_upstream_edid(uint8_t *edid_buf, uint16_t length)
{
	int reg_val = -1;

	pr_debug("%s:edid write length=%d\n", __func__, length);

	drive_hpd_low();

	/* hdmi spec. 120 msec is enough. */
	msleep(120);

	mhl_sii8620_device_init_rx_regs();

	edid_hw_sm_clean_up();

	/* choose EDID instead of devcap to appear at the FIFO */
	mhl_pf_write_reg(REG_PAGE_2_EDID_CTRL,
		 VAL_PAGE_2_EDID_CTRL_EDID_PRIME_VALID_DISABLE
		| VAL_PAGE_2_EDID_CTRL_DEVCAP_SELECT_EDID
		| VAL_PAGE_2_EDID_CTRL_EDID_FIFO_ADDR_AUTO_ENABLE
		| VAL_PAGE_2_EDID_CTRL_EDID_MODE_EN_ENABLE);

	/* clear the address toward the FIFO and write edid data into the FIFO */
	mhl_pf_write_reg(REG_PAGE_2_EDID_FIFO_ADDR, 0);
	mhl_pf_write_reg_block(REG_PAGE_2_EDID_FIFO_WR_DATA, length, edid_buf);

	DUMP_EDID_BLOCK(0, edid_buf, length)

	mhl_pf_write_reg(REG_PAGE_2_EDID_CTRL,
		 VAL_PAGE_2_EDID_CTRL_EDID_PRIME_VALID_ENABLE
		| VAL_PAGE_2_EDID_CTRL_DEVCAP_SELECT_EDID
		| VAL_PAGE_2_EDID_CTRL_EDID_FIFO_ADDR_AUTO_ENABLE
		| VAL_PAGE_2_EDID_CTRL_EDID_MODE_EN_ENABLE);

	/* Turn on heartbeat polling unconditionally to meet the specs */
	enable_heartbeat();

	/* Enable SCDT interrupt to detect stable incoming clock */
	mhl_pf_write_reg(REG_PAGE_0_INTR5_MASK, BIT_PAGE_0_INTR5_MASK_INTR5_MASK0);

	/* Disable EDID interrupt */
	mhl_pf_modify_reg(REG_EDID_FIFO_INT_MASK,
					BIT_INTR9_EDID_ERROR_MASK | BIT_INTR9_EDID_DONE_MASK,
					0x00);

	/* HPD was held low all this time. Now we send an HPD high */
	reg_val = drive_hpd_high();

	/* If SCDT is already high, then we will not get an interrupt */
	if (BIT_PAGE_2_TMDS_CSTAT_P3_SCDT & reg_val) {
#if 0 /* SIMG code has this impl. But, SOMC disable . TODO qusetion to SIMG */
		pr_warn("%s:tmds clock. hpd is just actived\n", __func__);
		/* todo : the int must be executed in int handler by manually. queue? */
		int_scdt_isr(BIT_INTR_SCDT_CHANGE);
#endif
	}

	/* upstream hdmi device might not be able to respond the hpd low
	 * just after high. Experimentally,the 300 msec is enough.
	 */
	msleep(300);

	return MHL_SUCCESS;

}

/* return false : invalid data, otherwise true */
static bool mhl_dev_edid_do_edid_filter(uint8_t *edid_buf_read_ptr)
{
	pr_debug("%s()", __func__);

	if (IS_BLOCK_0(edid_buf_read_ptr)) {
		edid_filter_prune_block_0(edid_buf_read_ptr);
	} else if (IS_EXT_BLOCK(edid_buf_read_ptr)) {
		pr_debug("%s:ext blk is arrived\n", __func__);
		edid_filter_prune_ext_blk(edid_buf_read_ptr);
	} else
		/* nothing to do*/
		pr_debug("%s:no blk0, no ext blk.\n", __func__);

	return true;
}

int  mhl_8620_dev_int_edid_isr(uint8_t int_edid_devcap_status)
{
	int edid_complete_size = 0;

	if (!int_edid_devcap_status)
		/* not edid interrupt */
		return int_edid_devcap_status;

	/* stop edid done timer */
	if (mhl_lib_timer_stop(edid_context.edid_done_timer) != 0)
		pr_warn("%s:timer stop fail\n", __func__);

	/* when interrupt happens, the flag must be
	 * cleared immediately!! */
	edid_context.is_Receive_EDID_DONE = true;

	if (!mhl_dev_is_set_hpd())
		goto edid_read_stop;

	if (edid_context.is_request_queued == true) {
		/*
		 * New edid request was already issued.
		 * skip all edid receiving procedure
		 * and starts the new request.
		 */
		pr_debug("%s:queue has new req. skip\n", __func__);
		goto retry_edid_read;
	}

	if (BIT_INTR9_EDID_DONE & int_edid_devcap_status) {
		int ddcStatus;
		int num_extensions;
		uint8_t edid_offset = 0;
		uint8_t *edid_buf_read_ptr;
		EDID_FIFO_BLOCK_OFFSET block_num;

		pr_debug("%s: EDID_DONE. available in fifo\n", __func__);

		ddcStatus = mhl_pf_read_reg(REG_PAGE_0_DDC_STATUS);
		mhl_pf_modify_reg(REG_PAGE_5_DISC_CTRL5,
			BIT_PAGE_5_DISC_CTRL5_DSM_OVRIDE,
			0);

		if (BIT_PAGE_0_DDC_STATUS_DDC_NO_ACK & ddcStatus) {
			pr_warn("%s,ddc no ack\n", __func__);
			goto retry_edid_read;
		}

		/* calculate edid fifo read offset point
		 * (0 or 128 byte is set to block_num)
		 */
		block_num = block_0;
		edid_offset = edid_block_size * (get_current_edid_request_block() & 0x01);

		if (edid_offset != 0)
			block_num = block_1;
		else
			block_num = block_0;

		pr_debug("%s: fifo offset=%d, reqest block=%d\n",
			__func__ , edid_offset,
			get_current_edid_request_block());

		/* Set edid_buf_read_ptr which is somewhere address
		 * of local edid save area. The ptr is write position
		 * of the arrived edid block.
		 */
		{
			int write_offset = edid_block_size * get_current_edid_request_block();
			pr_debug("%s:write offset = %d\n",
				__func__, write_offset);
			edid_buf_read_ptr = get_stored_edid_block() + write_offset;
		}

		/* read 128 byte edid data from FIFO and
		 * write the data to "edid_block_data" */
		if (MHL_SUCCESS != read_edid_data_from_edid_fifo(
			edid_buf_read_ptr, block_num))
			goto retry_edid_read;

		/* try to read next block or finish */
		num_extensions = mhl_edid_parser_get_num_cea_861_extensions(get_stored_edid_block());
		if (num_extensions < 0) {
			pr_warn("%s: error num_extensions---\n", __func__);
			goto retry_edid_read;
		} else if (get_current_edid_request_block() < num_extensions) {
			/* EDID read next block */
			increment_current_edid_request_block();
			if (MHL_SUCCESS != mhl_edid_start_edid_request())
				goto retry_edid_read;

			goto read_next_block;
		} else {
			edid_complete_size = (1 + get_current_edid_request_block()) * edid_block_size;
			pr_debug("%s: edid read complete\n", __func__);
			goto edid_read_complete;
		}
	}

	if (BIT_INTR9_EDID_ERROR & int_edid_devcap_status) {
		pr_warn("%s:err int=0x%02x\n", __func__,
			int_edid_devcap_status);
		mhl_pf_modify_reg(REG_PAGE_5_DISC_CTRL5,
			BIT_PAGE_5_DISC_CTRL5_DSM_OVRIDE,
			0);

		goto retry_edid_read;
	}

	if (edid_context.is_request_queued == true) {
		pr_debug("%s:queued read request exists", __func__);
		goto retry_edid_read;
	}

/* keep edid read */
read_next_block:
	return int_edid_devcap_status;

/* edid read will stop */
edid_read_complete:
	/*set_upstream_edid(get_stored_edid_block(), edid_complete_size);*/
edid_read_stop:
	edid_retry_cnt = 0;
	mhl_dev_clear_cbusp_cond_processing(DEV_EDID_READ);
	return int_edid_devcap_status;

/* retry edid read from block 0 */
retry_edid_read:
	if (edid_retry_cnt < EDID_RETRY_MAX) {
		pr_warn("%s: retry edid read. cnt=0x2%x\n",
			__func__, edid_retry_cnt);
		mhl_device_start_edid_read();
		edid_retry_cnt++;
	} else {
		pr_warn("%s: retry end!! (%d retry)\n",
			__func__, edid_retry_cnt);
		goto edid_read_stop;
	}

	return int_edid_devcap_status;
}

void mhl_device_edid_set_upstream_edid(void)
{
	int total_size = (1 + get_current_edid_request_block()) * edid_block_size;
	int i;

	for (i = 0; i < 1 + get_current_edid_request_block(); i++)
		mhl_dev_edid_do_edid_filter(get_stored_edid_block() + (i * edid_block_size));

	set_upstream_edid(get_stored_edid_block(), total_size);
}

static void edid_filter_prune_block_0(uint8_t *blk0)
{
	if (!mhl_edid_check_edid_header(blk0)) {
		pr_debug("%s:not blk 0. ignore\n", __func__);
		return;
	}

	mhl_lib_edid_remove_established_timing(
		mhl_1_2_support_video,
		SUPPORT_MHL_1_2_VIDEO_NUM,
		(blk0 + 0x23));

	mhl_lib_edid_remove_standard_timing(
		mhl_1_2_support_video,
		SUPPORT_MHL_1_2_VIDEO_NUM,
		(blk0 + 0x26));

	mhl_lib_edid_replace_unsupport_descriptor_with_dummy(
		blk0,
		preferred_disp_info,
		sizeof(preferred_disp_info)/sizeof(*preferred_disp_info));

	mhl_lib_edid_set_checksum(blk0);
}

static bool edid_is_ext_block(uint8_t *edid)
{
	PCEA_extension_t p_CEA_extension = (PCEA_extension_t)edid;
	if (EDID_EXTENSION_TAG != p_CEA_extension->tag) {
		pr_info("%s:not ext block. tag=0x%02x\n",
			 __func__, p_CEA_extension->tag);
		return false;
	}
	return true;
}

/*
 * edid : 128 byte edid data
 */
static void edid_filter_prune_ext_blk(uint8_t *edid)
{
	PCEA_extension_t p_CEA_extension = (PCEA_extension_t)edid;

	if (!edid_is_ext_block(edid))
		return;

	if (EDID_REV_THREE != p_CEA_extension->revision) {
		pr_info("%s:ext rev is not edid rev 3\n" , __func__);
		return;
	}

	if (mhl_device_is_peer_device_mhl1_2()) {
		pr_debug("%s:MHL1/2\n" , __func__);

		/* remove all unsupported vic */
		mhl_lib_edid_remove_vic_from_svd(
			edid, mhl_1_2_support_vic_array,
			 sizeof(mhl_1_2_support_vic_array));

		/* remove all hdmi vic and 3d info from vsd */
		mhl_lib_edid_remove_hdmi_vic_and_3d_from_vsd(edid, NULL, 0);

		/* if either sink or source does not support pp,
		 * vic16 is removed
		 */
		if (mhl_cbus_packed_pixel_available() == false)
			mhl_edid_parser_remove_vic16_1080p60fps(edid);

	} else if (mhl_device_is_peer_device_mhl3()) {
		uint8_t length = 0;
		uint8_t *mhl_supp_4k_vic = NULL;

		pr_debug("%s:MHL3\n" , __func__);

		/* remove all unsupported vic */
		mhl_lib_edid_remove_vic_from_svd(
			edid, mhl_3_support_vic_array,
			 sizeof(mhl_3_support_vic_array));

		/* make hdmi vic filter with MHL3 4k capability */
		mhl_supp_4k_vic = (uint8_t *)mhl_device_edid_get_mhlsink_sprt_hev(&length);

		/* remove all unsuported hdmi vic and entier 3d info */
		mhl_lib_edid_remove_hdmi_vic_and_3d_from_vsd(
			edid, mhl_supp_4k_vic, length);

		return;
	} else {
		pr_warn("%s:illegal cbus status:%d\n",
			 __func__,
			 mhl_device_get_cbus_mode());
		return;
	}

	return;
}

/*
 * edid MHL SINK hev info management.
 * Those API are supposed to be used in MHL3 mode.
 */
static void mhl_dev_edid_mhlsink_hev_init(void)
{
	memset(mhl_sink_sprt_hev_vic, 0x00, sizeof(mhl_sink_sprt_hev_vic));
	isMhlSuptFilter = true;
}

static int mhl_dev_edid_get_sprt_hev_last_idx(void)
{
	int i;
	for (i = 0; i < sizeof(mhl_sink_sprt_hev_vic); i++)
		if (0x00 == mhl_sink_sprt_hev_vic[i])
			return i;

	return sizeof(mhl_sink_sprt_hev_vic);
}


static bool mhl_dev_edid_is_vic_support(const uint8_t hev)
{
	int i;
	int max = ARRAY_SIZE(mhl_3_support_video);

	for (i = 0; i < max; i++)
		if (hev == mhl_3_support_video[i].vic)
			return true;

	return false;
}

const uint8_t *mhl_device_edid_get_mhlsink_sprt_hev(uint8_t *length)
{
	if (!length)
		return NULL;

	*length = mhl_dev_edid_get_sprt_hev_last_idx();
	return mhl_sink_sprt_hev_vic;
}


static void mhl_device_edid_add_mhlsink_sprt_hev(const uint8_t *hev, int length)
{
	int i;
	int cnt;
	int last_idx;
	int max;
	if (!hev)
		return;

	last_idx =  mhl_dev_edid_get_sprt_hev_last_idx();
	cnt = last_idx;

	max = last_idx + length;

	if (max > 20) {
		max = 20;
		pr_warn("%s:exceed max", __func__);
	}

	for (i = last_idx; i < max; i++) {
		/* the filter is used in unit test */
		if (isMhlSuptFilter) {
			if (mhl_dev_edid_is_vic_support(*hev)) {
				pr_debug("%s:add hdmi vic : %d\n",
					__func__, *hev);
				mhl_sink_sprt_hev_vic[cnt++] = *hev;
			}
		} else {
			mhl_sink_sprt_hev_vic[cnt++] = *hev;
		}
		hev++;
	}
}


void mhl_device_edid_setup_sink_support_hev_vic(uint8_t *vics, uint8_t length)
{
	mhl_device_edid_add_mhlsink_sprt_hev(vics, length);
}
