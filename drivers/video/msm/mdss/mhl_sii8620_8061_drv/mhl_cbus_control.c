/* kernel/drivers/video/msm/mdss/mhl_sii8620_8061_drv/mhl_cbus_control.c
 *
 * Copyright (C) 2013 Sony Mobile Communications AB.
 * Copyright (C) 2013 Silicon Image Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include "mhl_cbus_control.h"
#include "mhl_common.h"
#include "mhl_platform.h"
#include "mhl_sii8620_8061_devcap.h"
#include "mhl_sii8620_8061_device.h"
#include "mhl_tx.h"
#include "mhl_defs.h"
#include "mhl_lib_timer.h"
#include "mhl_lib_infoframe.h"
#include "si_emsc.h"

#define BIT_COC_PLL_LOCK_STATUS_CHANGE			0x01
#define BIT_COC_CALIBRATION_DONE				0x02
#define BIT_COC_8b_10b_ERROR					0x04
#define BIT_COC_CALIBRATION_COMMA1				0x08
#define BIT_COC_CALIBRATION_COMMA2				0x10

#define VAL_PAGE_3_M3_CTRL_MHL1_2_VALUE (BIT_PAGE_3_M3_CTRL_SW_MHL3_SEL \
						| BIT_PAGE_3_M3_CTRL_ENC_TMDS)

#define FEATURE_ID_E_MSC			0x00
#define FEATURE_ID_USB				0x01
#define FEATURE_ID_AUDIO			0x02
#define FEATURE_ID_IP				0x03
#define FEATURE_ID_COMP_VIDEO			0x04
#define FEATURE_ID_HID				0x05
#define FEATURE_ID_LAST				FEATURE_ID_HID
#define VC_RESPONSE_ACCEPT			0x00
#define VC_RESPONSE_BAD_VC_NUM			0x01
#define VC_RESPONSE_BAD_FEATURE_ID		0x02
#define VC_RESPONSE_BAD_CHANNEL_SIZE		0x03

/* Others */
#define BIT_PAGE_3_M3_P0CTRL_MHL3_P0_UNLIMIT_EN_PP         0x08
#define BIT_PAGE_3_M3_P0CTRL_MHL3_P0_UNLIMIT_EN_NORM       0x00


/* DEVCAP */
static void read_devcap_fifo(uint8_t *dev_cap_buf, u8 size_dev, bool xdev);

/* Timer */
static int mhl_create_timer(void);
static int mhl_release_timer(void);

/* rap control */
static int send_content_on(void);
static bool is_rap_supported(void);

/* cbus context */
static struct cbus_control_context cbus_context;

/* cbus error */
#define DDC_ABORT_THRESHOLD	10
#define MSC_ABORT_THRESHOLD	10

#define MAX_I2C_PAYLOAD_SIZE	256
#define MAX_I2C_CMD_SIZE		0
#define MAX_I2C_EMSC_BLOCK_SIZE (MAX_I2C_CMD_SIZE + MAX_I2C_PAYLOAD_SIZE)

static void process_cbus_abort(void);
static void cbus_abort_timer_callback(void *callback_param);

struct i2c_xfer_mem {
	uint8_t *block_tx_buffers;
} i2c_mem;

struct cbus_req {
	struct list_head	link;
	union {
		struct {
			uint8_t	cancel:1;
			uint8_t	resvd:7;
		} flags;
		uint8_t	as_uint8;
	} status;
	uint8_t	retry_count;
	uint8_t	command;	/* VS_CMD or RCP opcode */
	uint8_t	reg;
	uint8_t	reg_data;
	uint8_t	offset;		/* register offset */
	uint8_t	length;		/* Only applicable to write burst */
	uint8_t	msg_data[16];	/* scratch pad data area. */
	int	sequence;
};

struct SI_PACK_THIS_STRUCT tport_hdr_and_burst_id_t {
	/*
	   waste two bytes to save on memory copying
	   when submitting BLOCK transactions
	*/
	struct SI_PACK_THIS_STRUCT standard_transport_header_t tport_hdr;

	/* sub-payloads start here */
	MHL_burst_id_t	burst_id;
};

union SI_PACK_THIS_STRUCT emsc_payload_t {
	struct SI_PACK_THIS_STRUCT tport_hdr_and_burst_id_t hdr_and_burst_id;
	uint8_t as_bytes[256];
};

struct SI_PACK_THIS_STRUCT block_req {
	struct list_head	link;
	const char *function;
	int line;
	int	sequence;
	uint8_t count; /* (size - 1) see MHL spec section 13.5.7.2 */
	uint8_t space_remaining;
	uint8_t	sub_payload_size;
	uint8_t *platform_header;
	union SI_PACK_THIS_STRUCT emsc_payload_t *payload;
};

struct cbus_control_context {
	/* status register*/
	u8			status_0;
	u8			status_1;
	u8			status_2;
	u8			xstatus_1;
	u8			xstatus_3;
	u8			link_mode;
	u8			cbus_status;
	MHLDevCap_u		devcap;
	MHLXDevCap_u	xdevcap;


	/* cbus queue */
	struct list_head	cbus_queue;
	struct list_head	cbus_free_list;
	struct cbus_req		cbus_req_entries[NUM_CBUS_EVENT_QUEUE_EVENTS];
	struct cbus_req		*current_cbus_req;
	int			sequence;

	/* flag control */
	struct {
		bool devcap_all;
		bool path_en;
		bool hpd;
		bool video_ready;
		bool cbus_mode_up;
		bool dcap_chg_rcv;
		bool hdcp_ready;
		bool tmds_ready;
	} flag;

	/* RAP*/
	u8	msc_msg_last_send_data;
	u8	msc_msg_last_rcv_data;

	/* TDM allocation */
	uint8_t	virt_chan_slot_counts[TDM_VC_MAX];

	/* cbus abort */
	void *cbus_abort_timer;
	bool cbus_abort_delay_active;
	int msc_abort_count;
	int ddc_abort_count;

	struct {
		int				sequence;
		unsigned long local_blk_rx_buffer_size;
		struct list_head	queue;
		struct list_head	free_list;
		struct block_req	*marshalling_req;
		struct block_req	req_entries[NUM_BLOCK_QUEUE_REQUESTS];
	} block_protocol;

	struct		{
		uint16_t received_byte_count;
		unsigned long peer_blk_rx_buffer_avail;

#define NUM_BLOCK_INPUT_BUFFERS 8
		uint8_t		input_buffers[NUM_BLOCK_INPUT_BUFFERS][256];
		int		input_buffer_lengths[NUM_BLOCK_INPUT_BUFFERS];
		uint16_t	head;
		uint16_t	tail;
	} hw_block_protocol;

};

uint8_t dev_cap_values[] = {
	DEVCAP_VAL_DEV_STATE,
	DEVCAP_VAL_MHL_VERSION,
	DEVCAP_VAL_DEV_CAT,
	DEVCAP_VAL_ADOPTER_ID_H,
	DEVCAP_VAL_ADOPTER_ID_L,
	DEVCAP_VAL_VID_LINK_MODE,
	DEVCAP_VAL_AUD_LINK_MODE,
	DEVCAP_VAL_VIDEO_TYPE,
	DEVCAP_VAL_LOG_DEV_MAP,
	DEVCAP_VAL_BANDWIDTH,
	DEVCAP_VAL_FEATURE_FLAG,
	DEVCAP_VAL_DEVICE_ID_H,
	DEVCAP_VAL_DEVICE_ID_L,
	DEVCAP_VAL_SCRATCHPAD_SIZE,
	DEVCAP_VAL_INT_STAT_SIZE,
	DEVCAP_VAL_RESERVED
};

uint8_t xdev_cap_values[] = {
	XDEVCAP_VAL_ECBUS_SPEEDS,
	XDEVCAP_VAL_TMDS_SPEEDS,
	XDEVCAP_VAL_DEV_ROLES,
	XDEVCAP_VAL_LOG_DEV_MAPX,
	XDEVCAP_VAL_RESERVE
};

static char *get_cbus_command_string(int command)
{
#define CBUS_COMMAND_CASE(command) case command: return #command;
	switch (command) {
	CBUS_COMMAND_CASE(MHL_ACK)
	CBUS_COMMAND_CASE(MHL_NACK)
	CBUS_COMMAND_CASE(MHL_ABORT)
	CBUS_COMMAND_CASE(MHL_WRITE_STAT)
	CBUS_COMMAND_CASE(MHL_SET_INT)
	CBUS_COMMAND_CASE(MHL_READ_DEVCAP)
	CBUS_COMMAND_CASE(MHL_GET_STATE)
	CBUS_COMMAND_CASE(MHL_GET_VENDOR_ID)
	CBUS_COMMAND_CASE(MHL_SET_HPD)
	CBUS_COMMAND_CASE(MHL_CLR_HPD)
	CBUS_COMMAND_CASE(MHL_SET_CAP_ID)
	CBUS_COMMAND_CASE(MHL_GET_CAP_ID)
	CBUS_COMMAND_CASE(MHL_MSC_MSG)
	CBUS_COMMAND_CASE(MHL_GET_SC1_ERRORCODE)
	CBUS_COMMAND_CASE(MHL_GET_DDC_ERRORCODE)
	CBUS_COMMAND_CASE(MHL_GET_MSC_ERRORCODE)
	CBUS_COMMAND_CASE(MHL_WRITE_BURST)
	CBUS_COMMAND_CASE(MHL_GET_SC3_ERRORCODE)
	CBUS_COMMAND_CASE(MHL_WRITE_XSTAT)
	CBUS_COMMAND_CASE(MHL_READ_XDEVCAP)
	CBUS_COMMAND_CASE(MHL_READ_EDID_BLOCK)
	CBUS_COMMAND_CASE(MHL_READ_XDEVCAP_REG)
	CBUS_COMMAND_CASE(MHL_SEND_3D_REQ_OR_FEAT_REQ)
	}
	return "unknown";
}

#define RAP_RECEIVE_TIME 1000
static void *rapk_receive_timer;

static void mhl_rapk_receive_timer(void *callback_param)
{
	pr_warn("%s: expired\n", __func__);
	mhl_dev_clear_cbusp_cond_processing(DEV_RAP_SEND);
	pr_warn("%s: msc_msg_last_send_data = 0x%2x\n",
		__func__, cbus_context.msc_msg_last_send_data);
	cbus_context.current_cbus_req = NULL;
	exe_cbus_command();
}

static void mhl_rapk_receive_timer_start(void)
{
	int ret;

	ret = mhl_lib_timer_start(rapk_receive_timer, RAP_RECEIVE_TIME);
	if (ret != 0)
		pr_err("%s: Failed to start rapk receive timer!\n", __func__);
}

static void mhl_rapk_receive_timer_stop(void)
{
	int ret;

	ret = mhl_lib_timer_stop(rapk_receive_timer);
	if (ret != 0)
		pr_err("%s: Failed to stop rapk receive timer!\n", __func__);
}

static void mhl_rapk_receive_timer_init(void)
{
	int ret;

	pr_debug("%s:\n", __func__);

	ret = mhl_lib_timer_create(mhl_rapk_receive_timer,
					NULL,
					&rapk_receive_timer);
	if (ret != 0)
		pr_err("%s: Failed to create rapk receive timer!\n", __func__);
}

static void mhl_rapk_receive_timer_release(void)
{
	int ret;

	pr_debug("%s:\n", __func__);

	ret = mhl_lib_timer_delete(&rapk_receive_timer);
	if (ret != 0)
		pr_err("%s: Failed to delete rapk receive timer!\n", __func__);
}

void mhl_pf_get_block_buffer_info(
			struct block_buffer_info_t  *block_buffer_info)
{
	block_buffer_info->buffer = i2c_mem.block_tx_buffers;
	block_buffer_info->req_size = MAX_I2C_EMSC_BLOCK_SIZE;
	block_buffer_info->payload_offset = MAX_I2C_CMD_SIZE;
}

static bool is_SamsungTV(void)
{
	if (cbus_context.devcap.mdc.mhl_version == 0x10 &&
		cbus_context.devcap.mdc.int_state_size == 0x44)
		return true;
	else
		return false;
}

/*
 *  CBUS QUEUE CONTROL
 *  NOTE : following cbus queue control is not
 *  thread safe (list_xx). So, those API must not
 *  be used in multithread context.
 */

void init_cbus_queue(void)
{
	struct cbus_req	*entry;
	int idx;

	INIT_LIST_HEAD(&cbus_context.cbus_queue);
	INIT_LIST_HEAD(&cbus_context.cbus_free_list);

	if (list_empty(&cbus_context.cbus_queue)) {
		pr_debug("%s:Queue empty\n", __func__);
	}

	cbus_context.current_cbus_req = NULL;

	/* Place pre-allocated CBUS queue entries on the free list */
	for (idx = 0; idx < NUM_CBUS_EVENT_QUEUE_EVENTS; idx++) {

		entry = &cbus_context.cbus_req_entries[idx];
		memset(entry, 0, sizeof(struct cbus_req));
		list_add(&entry->link, &cbus_context.cbus_free_list);
	}
}

struct cbus_req *get_free_cbus_queue_entry(void)
{
	struct cbus_req *req;
	struct list_head *entry;

	if (list_empty(&cbus_context.cbus_free_list)) {
		int i;
		list_for_each(entry, &cbus_context.cbus_queue)
		{
			req = list_entry(entry, struct cbus_req, link);
		}
		for (i = 0; i < ARRAY_SIZE(cbus_context.cbus_req_entries); ++i) {
			req = &cbus_context.cbus_req_entries[i];
		}
		return NULL;
	}

	entry = cbus_context.cbus_free_list.next;
	list_del(entry);
	req = list_entry(entry, struct cbus_req, link);

	/* Start clean */
	req->status.flags.cancel = 0;
	req->sequence = cbus_context.sequence++;

	return req;
}

void return_cbus_queue_entry(struct cbus_req *pReq)
{
	list_add(&pReq->link, &cbus_context.cbus_free_list);
}

void queue_cbus_transaction(struct cbus_req *pReq)
{
	list_add_tail(&pReq->link, &cbus_context.cbus_queue);
	exe_cbus_command();
}

void queue_priority_cbus_transaction(struct cbus_req *req)
{
	list_add(&req->link, &cbus_context.cbus_queue);
}

struct cbus_req *get_next_cbus_transaction(void)
{
	struct cbus_req *req;
	struct list_head *entry;

	if (list_empty(&cbus_context.cbus_queue)) {
		pr_debug("%s:Queue empty\n", __func__);
		return NULL;
	}

	entry = cbus_context.cbus_queue.next;
	list_del(entry);
	req = list_entry(entry, struct cbus_req, link);

	return req;
}

/*
 *
 *  COMMAND SET & EXECUTION Utility
 *
 */

void mhl_msc_init()
{
	init_cbus_queue();
	init_cond_in_cbus_control();
	cbus_context.status_0 = 0;
	cbus_context.status_1 = 0;
	cbus_context.xstatus_1 = 0;
	cbus_context.xstatus_3 = 0;
	cbus_context.cbus_status = 0;
	cbus_context.link_mode = MHL_STATUS_CLK_MODE_NORMAL;
	memset(&cbus_context.devcap , 0x00 , sizeof(cbus_context.devcap));
	memset(&cbus_context.xdevcap , 0x00 , sizeof(cbus_context.xdevcap));

	cbus_context.cbus_abort_delay_active = false;
	cbus_context.msc_abort_count = 0;
	cbus_context.ddc_abort_count = 0;

	cbus_wb_init();
}

bool set_cbus_command(uint8_t command, uint8_t reg, uint8_t reg_data)
{
	struct cbus_req *req;

	req = get_free_cbus_queue_entry();

	if (req == NULL) {
		pr_debug("%s:CBUS free queue exhausted\n", __func__);
		return false;
	}

	req->retry_count	= 2;
	req->command		= command;
	req->reg		= reg;
	req->reg_data		= reg_data;

	pr_info("[CBUS] set %s reg: %X reg_data %X\n",
		 get_cbus_command_string(req->command),
		 req->reg, req->reg_data);

	if ((command == MHL_SET_INT)
		&& (reg == MHL_RCHANGE_INT)
		&& (reg_data == MHL_INT_GRT_WRT)) {
		pr_debug("%s:Priority cbus transaction\n", __func__);
		queue_priority_cbus_transaction(req);
	} else {
		queue_cbus_transaction(req);
	}

	return true;
}

bool set_cbus_command_block(
	uint8_t command, uint8_t length, uint8_t burst_offset, void *buffer)
{
	struct cbus_req *req;

	req = get_free_cbus_queue_entry();

	if (req == NULL) {
		pr_warn("%s:CBUS free queue exhausted\n", __func__);
		return false;
	}

	req->retry_count	= 1;
	req->command		= command;
	req->length		= length;
	req->offset		= burst_offset;
	memcpy(req->msg_data, buffer, length);

	pr_info("[CBUS] set %s\n", get_cbus_command_string(req->command));

	queue_cbus_transaction(req);

	return true;
}

static void mhl_cbus_send_cbus_mode_up(void)
{
	/* Correspondence not to transmit CBUS_MODE_UP two time */
	if (cbus_context.flag.cbus_mode_up != true) {
		set_cbus_command(MHL_MSC_MSG, MHL_MSC_MSG_RAP,
					MHL_RAP_CBUS_MODE_UP);
		cbus_context.flag.cbus_mode_up = true;
	} else
		pr_warn("%s: already CBUS_MODE_UP\n", __func__);
}

void exe_cbus_command(void){
	struct cbus_req *req;

	if (cbus_context.cbus_abort_delay_active) {
		pr_warn("%s CBUS abort delay in progress "		\
			 "can't send any messages\n", __func__);
		return;
	}

	req = cbus_context.current_cbus_req;

	if (req != NULL) {
		pr_info("%s:%s is in progress\n",
			 __func__, get_cbus_command_string(req->command));
		return;
	}

	req = get_next_cbus_transaction();

	if (req == NULL) {
		return;
	}

	if (MHL_MSC_MSG == req->command) {
		cbus_context.msc_msg_last_send_data = req->reg_data;
	}

	if (req) {
		bool success = true;

		cbus_context.current_cbus_req = req;

		pr_info("[CBUS] exe %s reg: %X reg_data %X\n",
			 get_cbus_command_string(req->command),
			 req->reg, req->reg_data);

		switch (req->command) {
		case MHL_WRITE_STAT:
		case MHL_WRITE_XSTAT:
		case MHL_SET_INT:
		case MHL_SEND_3D_REQ_OR_FEAT_REQ:
			if (MHL3_INT_FEAT_COMPLETE == req->reg_data)
				pr_debug("%s: Sent FEAT_COMPLETE\n", __func__);

			mhl_pf_write_reg(REG_PAGE_5_MSC_CMD_OR_OFFSET,
					 req->reg);
			mhl_pf_write_reg(REG_PAGE_5_MSC_1ST_TRANSMIT_DATA,
					 req->reg_data);
			mhl_pf_write_reg(REG_PAGE_5_MSC_COMMAND_START,
					 BIT_PAGE_5_MSC_COMMAND_START_MSC_WRITE_STAT_CMD);
			break;
		case MHL_READ_DEVCAP:
				pr_debug("%s: Trigger DEVCAP Read\n", __func__);
				mhl_pf_modify_reg(REG_EDID_FIFO_INT_MASK,
					BIT_INTR9_DEVCAP_DONE_MASK,
					VAL_INTR9_DEVCAP_DONE_MASK_ENABLE);

				/* don't call si_mhl_tx_drv_reset_ddc_fifo here */
				mhl_pf_write_reg(REG_PAGE_2_EDID_CTRL,
					VAL_PAGE_2_EDID_CTRL_EDID_PRIME_VALID_DISABLE |
					VAL_PAGE_2_EDID_CTRL_DEVCAP_SELECT_DEVCAP |
					VAL_PAGE_2_EDID_CTRL_EDID_FIFO_ADDR_AUTO_ENABLE |
					VAL_PAGE_2_EDID_CTRL_EDID_MODE_EN_ENABLE
				);

			/* read the entire DEVCAP array	in one command */
				mhl_pf_write_reg(REG_PAGE_2_TPI_CBUS_START,
				BIT_PAGE_2_TPI_CBUS_START_GET_DEVCAP_START);
			break;
		case MHL_READ_XDEVCAP:
			pr_debug("%s: Trigger XDEVCAP Read\n", __func__);
				mhl_pf_modify_reg(REG_EDID_FIFO_INT_MASK,
					BIT_INTR9_DEVCAP_DONE_MASK,
					VAL_INTR9_DEVCAP_DONE_MASK_ENABLE);

				/* don't call si_mhl_tx_drv_reset_ddc_fifo here */
				mhl_pf_write_reg(REG_PAGE_2_EDID_CTRL,
					VAL_PAGE_2_EDID_CTRL_EDID_PRIME_VALID_DISABLE |
					BIT_PAGE_2_EDID_CTRL_XDEVCAP_EN |
					VAL_PAGE_2_EDID_CTRL_DEVCAP_SELECT_DEVCAP |
					VAL_PAGE_2_EDID_CTRL_EDID_FIFO_ADDR_AUTO_ENABLE |
					VAL_PAGE_2_EDID_CTRL_EDID_MODE_EN_ENABLE
				);

			/* read the entire DEVCAP array	in one command */
				mhl_pf_write_reg(REG_PAGE_2_TPI_CBUS_START,
				BIT_PAGE_2_TPI_CBUS_START_GET_DEVCAP_START);
			break;
		case MHL_READ_XDEVCAP_REG:
			pr_debug("%s: Read XDEVCAP_REG (0x%02x)\n", __func__,
				req->reg);
			mhl_pf_write_reg(REG_PAGE_5_MSC_CMD_OR_OFFSET,
				req->reg);
			mhl_pf_write_reg(REG_PAGE_5_MSC_COMMAND_START,
			BIT_PAGE_5_MSC_COMMAND_START_MSC_READ_DEVCAP_CMD);
			break;
		case MHL_MSC_MSG:
			if (req->reg == MHL_MSC_MSG_RAP) {
				mhl_dev_set_cbusp_cond_processing(DEV_RAP_SEND);
				mhl_rapk_receive_timer_start();
			}

			mhl_pf_write_reg(REG_PAGE_5_MSC_CMD_OR_OFFSET,
					MHL_MSC_MSG);
			mhl_pf_write_reg(REG_PAGE_5_MSC_1ST_TRANSMIT_DATA,
					req->reg);
			mhl_pf_write_reg(REG_PAGE_5_MSC_2ND_TRANSMIT_DATA,
					req->reg_data);
			mhl_pf_write_reg(REG_PAGE_5_MSC_COMMAND_START,
				BIT_PAGE_5_MSC_COMMAND_START_MSC_MSC_MSG_CMD);
			break;
		case MHL_READ_EDID_BLOCK:
			mhl_device_start_edid_read();
			break;
		case MHL_WRITE_BURST:
			pr_debug("%s: Sent WRITE_BURST\n", __func__);
			cbus_wb_enable_gen2_write_burst_xmit();
			cbus_wb_write_gen2_xfifo(req->length, req->msg_data);
			break;
		case MHL_GET_STATE:
		case MHL_GET_VENDOR_ID:
		case MHL_SET_HPD:
		case MHL_CLR_HPD:
		case MHL_GET_SC1_ERRORCODE:
		case MHL_GET_DDC_ERRORCODE:
		case MHL_GET_MSC_ERRORCODE:
		case MHL_GET_SC3_ERRORCODE:
			pr_debug("%s: Sending MSC command %02x, %02x, %02x\n",
				 __func__, req->command, req->reg, req->reg_data);
			mhl_pf_write_reg(REG_PAGE_5_MSC_CMD_OR_OFFSET, req->command);
			mhl_pf_write_reg(REG_PAGE_5_MSC_1ST_TRANSMIT_DATA, req->reg_data);
			mhl_pf_write_reg(REG_PAGE_5_MSC_COMMAND_START,
					 BIT_PAGE_5_MSC_COMMAND_START_MSC_PEER_CMD);
			break;
		default:
			success = false;
			pr_err("%s:Not Implemented\n", __func__);
		}

		if (!success) {
			return_cbus_queue_entry(req);
			cbus_context.current_cbus_req = NULL;
		}
	}
}

/*
 *
 *  CBUS Communication Logic
 *
 */

void mhl_msc_cbus_communication_start()
{
	set_cbus_command(MHL_WRITE_STAT,
			 MHL_STATUS_REG_VERSION_STAT,
			 MHL_VERSION);

	set_cbus_command(MHL_WRITE_STAT,
			 MHL_STATUS_REG_CONNECTED_RDY,
			 MHL_STATUS_DCAP_RDY
			 | MHL_STATUS_XDEVCAPP_SUPP
			 | ((DEVCAP_VAL_DEV_CAT
			     &
			     (MHL_DEV_CATEGORY_PLIM2_0
			      | MHL_DEV_CATEGORY_POW_BIT)
			     )>>2)
			 );
	set_cbus_command(MHL_SET_INT,
			 MHL_RCHANGE_INT,
			 MHL_INT_DCAP_CHG);
}

void mhl_cbus_communication_eCBUS_start(void)
{
	pr_debug("%s called\n", __func__);

	if (MHL_STATUS_DCAP_RDY & cbus_context.status_0) {
		set_cbus_command(MHL_READ_XDEVCAP, 0, 0);
	}

	cbus_wb_start_gen2_write_burst();

#ifdef EMSC_WORKAROUND
	/*
	 * This register should be set
	 * in si_mhl_tx_initialize_block_transport().
	 */
	mhl_pf_modify_reg(REG_PAGE_3_GENCTL, BIT_PAGE_3_GENCTL_EMSC_EN, 0x01);
#endif
	/* FIXME
	   We're investigating whether block transaction is
	   mandatory or not
	   si_mhl_tx_initialize_block_transport(dev_context); */
}

static bool mhl_cbus_support_e_cbus_d(void)
{
	return false;
}

static void mhl_cbus_slot_init(bool ecbus_d_speeds)
{
	/*
	 * Set default eCBUS virtual channel slot assignments
	 * based on the capabilities of both the transmitter
	 * and receiver.
	 */
	if ((ecbus_d_speeds == true) && mhl_cbus_support_e_cbus_d()) {
		pr_debug("CBUS slot set to eCBUS_D\n");
		cbus_context.virt_chan_slot_counts[TDM_VC_CBUS1] = 1;
		cbus_context.virt_chan_slot_counts[TDM_VC_E_MSC] = 39;
		cbus_context.virt_chan_slot_counts[TDM_VC_T_CBUS] = 160;
	} else {
		pr_debug("CBUS slot set to eCBUS_S\n");
		cbus_context.virt_chan_slot_counts[TDM_VC_CBUS1] = 1;
		cbus_context.virt_chan_slot_counts[TDM_VC_E_MSC] = 4;
		cbus_context.virt_chan_slot_counts[TDM_VC_T_CBUS] = 20;
	}
}

static int mhl_cbus_set_tdm_slot_allocation(
				uint8_t *vc_slot_counts, bool resync)
{
	int		status = -EINVAL;
	uint16_t	slot_total = 0;
	uint8_t		idx;

	/* To the extent we can sanity check the slot allocation request */
	if (vc_slot_counts[TDM_VC_CBUS1] != 1) {
		pr_err("%s: CBUS slot size is not 1\n", __func__);
		goto done;
	}

	for (idx = 0; idx < TDM_VC_MAX; idx++)
		slot_total += vc_slot_counts[idx];
	if (slot_total > 200) {
		pr_err("%s: Total slot size is more than 200\n", __func__);
		goto done;
	}

	if (!resync) {
		/*
		 * Since we're not being asked to perform TDM re-synchronization
		 * we don't know which eCBUS mode will be used with this slot
		 * allocation so we can't sanity check it further.  Just save
		 * it for later use.
		 */
		status = 0;
	} else {
		switch (mhl_device_get_cbus_mode()) {
		case CBUS_eCBUS_S:
			if (slot_total > 25) {
				pr_err("%s: Total slot size is more than 25\n",
					__func__);
				goto done;
			}
			break;
		case CBUS_eCBUS_D:
			break;
		default:
			pr_err("%s: CBUS mode is not eCBUS\n", __func__);
			goto done;
		}
		status = 0;

		mhl_pf_write_reg(REG_PAGE_1_TTXSPINUMS,
			cbus_context.virt_chan_slot_counts[TDM_VC_E_MSC]);
		mhl_pf_write_reg(REG_PAGE_1_TTXHSICNUMS,
			cbus_context.virt_chan_slot_counts[TDM_VC_T_CBUS]);
		mhl_pf_write_reg(REG_PAGE_1_TTXTOTNUMS, slot_total - 1);
	}

done:
	return status;
}

void mhl_cbus_process_vc_assign(uint8_t *write_burst_data)
{
	struct tdm_alloc_burst	*tdm_burst;
	uint8_t			save_channel_size;
	uint8_t			idx;

	tdm_burst = (struct tdm_alloc_burst *)write_burst_data;

	/*
	 * The virtual channel assignment in the WRITE_BURST may contain one,
	 * two or three channel allocations
	*/
	if (tdm_burst->num_ent > 3) {
		pr_err("%s: Bad number of assignment requests in virtual channel assign",
				__func__);
		return;
	}

	for (idx = 0; idx < tdm_burst->num_ent; idx++) {
		tdm_burst->vc_info[idx].req_resp.response = VC_RESPONSE_ACCEPT;

		if (tdm_burst->vc_info[idx].feature_id > FEATURE_ID_LAST) {
			pr_warn("%s: Source rejected request to assign"\
					"CBUS virtual channel %d, invalid feature id 0x%02x\n",
					__func__,
					tdm_burst->vc_info[idx].vc_num,
					tdm_burst->vc_info[idx].feature_id);
			tdm_burst->vc_info[idx].req_resp.response
					= VC_RESPONSE_BAD_FEATURE_ID;
			continue;
		}

		if ((tdm_burst->vc_info[idx].feature_id == FEATURE_ID_E_MSC)
			|| (tdm_burst->vc_info[idx].feature_id
						== FEATURE_ID_USB)) {
			/*
			 * changing slot allocations may result in a loss
			 * of data; however, link should self-synchronize
			*/
			save_channel_size = 0;

			switch (tdm_burst->vc_info[idx].feature_id) {
			case FEATURE_ID_E_MSC:
				save_channel_size
					= cbus_context.virt_chan_slot_counts[TDM_VC_E_MSC];
				cbus_context.virt_chan_slot_counts[TDM_VC_E_MSC]
					= tdm_burst->vc_info[idx].req_resp.channel_size;
				break;
			case FEATURE_ID_USB:
				save_channel_size
					= cbus_context.virt_chan_slot_counts[TDM_VC_T_CBUS];
				cbus_context.virt_chan_slot_counts[TDM_VC_T_CBUS]
					= tdm_burst->vc_info[idx].req_resp.channel_size;
				break;
			}

			if (mhl_cbus_set_tdm_slot_allocation(
				cbus_context.virt_chan_slot_counts, true)) {
				pr_warn("%s: Source rejected request to assign"\
					"CBUS virtual channel %d, bad channel size: 0x%02x\n",
					__func__,
					tdm_burst->vc_info[idx].vc_num,
					tdm_burst->vc_info[idx].req_resp.channel_size);
				switch (tdm_burst->vc_info[idx].feature_id) {
				case FEATURE_ID_E_MSC:
					cbus_context.virt_chan_slot_counts[TDM_VC_E_MSC]
						= save_channel_size;
					break;
				case FEATURE_ID_USB:
					cbus_context.virt_chan_slot_counts[TDM_VC_T_CBUS]
						= save_channel_size;
					break;
				}
				tdm_burst->vc_info[idx].req_resp.response
					= VC_RESPONSE_BAD_CHANNEL_SIZE;
			}
		} else {
			pr_warn("%s: Source rejected request to assign"\
					"CBUS virtual channel %d, bad virtual channel\n",
					__func__,
					tdm_burst->vc_info[idx].vc_num);
			tdm_burst->vc_info[idx].req_resp.response
					= VC_RESPONSE_BAD_VC_NUM;
		}

		if (tdm_burst->vc_info[idx].req_resp.response
						== VC_RESPONSE_ACCEPT) {
			pr_debug("%s: Source accepted requested virtual"\
					"channel %d reassignment\n",
					__func__,
					tdm_burst->vc_info[idx].vc_num);
		}
	}

	/* Respond back to requester to indicate acceptance or rejection */
	tdm_burst->burst_id_h = burst_id_VC_CONFIRM >> 8;
	tdm_burst->burst_id_l = (uint8_t)burst_id_VC_CONFIRM;
	tdm_burst->checksum = calculate_generic_checksum(
				(uint8_t *)(tdm_burst), 0, sizeof(*tdm_burst));

	cbus_wb_request_write_burst(
				0, sizeof(*tdm_burst), (uint8_t *)(tdm_burst));
}

void mhl_msc_command_done(bool is_Msc_Msg_Ack)
{
	struct cbus_req *req;

	req = cbus_context.current_cbus_req;
	if (req == NULL) {
		pr_warn("No message to associate with completion notification\n");
		return;
	}

	pr_info("complete cbus req: %s reg: %X reg_data %X\n",
			get_cbus_command_string(req->command),
			req->reg, req->reg_data);

	/* In the case of NACK,
	   re-try process must be executed otherwise
	   if one command is executing,
	   the command done process is skipped. */
	if (is_Msc_Msg_Ack) {
		/* skip if command is not finished */
		if (mhl_device_is_cbusb_executing()) {
			pr_info("%s:cbus is executing. skip!", __func__);
			return;
		}
	}

	cbus_context.current_cbus_req = NULL;

	/* FIXME
	   Cancel Process needs to be implemented */

	if (MHL_WRITE_STAT == req->command) {
		if (MHL_STATUS_REG_CONNECTED_RDY == req->reg) {
			if (MHL_STATUS_DCAP_RDY == req->reg_data) {
				set_cbus_command(MHL_SET_INT,
						 MHL_RCHANGE_INT,
						 MHL_INT_DCAP_CHG);
			}
		} else if (MHL_STATUS_REG_LINK_MODE == req->reg) {
			if (MHL_STATUS_PATH_ENABLED & req->reg_data) {
			}
		}
	} else if (MHL_WRITE_XSTAT == req->command) {
	} else if (MHL_READ_DEVCAP == req->command) {
		bool temp;
		int i;
		MHLDevCap_u	old_devcap, devcap_changes;

		old_devcap = cbus_context.devcap;
		read_devcap_fifo(cbus_context.devcap.devcap_cache,
			DEVCAP_SIZE,
			false/*devcap*/);

		/* devcap log */
		pr_info("devcap");
		for (i = 0; i < sizeof(cbus_context.devcap.devcap_cache); i++) {
			pr_info(" [0x%X:%.2X]", i,
				 cbus_context.devcap.devcap_cache[i]);
		}

		mhl_platform_power_start_charge(
			cbus_context.devcap.devcap_cache);

		/* Generate a change mask between the old and new devcaps */
		for (i = 0; i < sizeof(old_devcap); ++i) {
			devcap_changes.devcap_cache[i]
				= cbus_context.devcap.devcap_cache[i]
				^ old_devcap.devcap_cache[i];
		}

		temp = 0;
		for (i = 0; i < sizeof(devcap_changes); ++i) {
			temp |= devcap_changes.devcap_cache[i];
		}
		if (temp) {
			set_cond_in_cbus_control(DEVCAP_ALL_READ_DONE);
		} else {
			pr_debug("%s:devcap did not change\n", __func__);
		}

		/*
		 * To distinguish Samsung MHL 1.0 Sink, followings
		 * in device capability register values are referred.
		 *
		 * 1.MHL_VERSION = 0x10
		 * 2.INT_STAT_SIZE = 0x44
		 *
		 * In SOMC investigation, only this Samsung model has the
		 * INT_STAT_SIZE value (0x44).
		 */
		if (is_SamsungTV()) {
			pr_debug("Samsung TV detected\n");
			set_cond_in_cbus_control(SENT_PATH_EN_1);
		}
	} else if (MHL_READ_XDEVCAP == req->command) {
		bool temp;
		int i;
		MHLXDevCap_u old_xdevcap, xdevcap_changes;

		old_xdevcap = cbus_context.xdevcap;
		read_devcap_fifo(cbus_context.xdevcap.xdevcap_cache,
			XDEVCAP_OFFSET(XDEVCAP_LIMIT),
			true/*xdevcap*/);

		/* xdevcap log */
		pr_info("xdevcap");
		for (i = 0; i < XDEVCAP_OFFSET(XDEVCAP_LIMIT); i++) {
			pr_info(" [0x%X:%.2X]", i,
				cbus_context.xdevcap.xdevcap_cache[i]);
		}

		/* Generate a change mask between the old and new devcaps */
		for (i = 0; i < XDEVCAP_OFFSET(XDEVCAP_LIMIT); ++i) {
			xdevcap_changes.xdevcap_cache[i]
				= cbus_context.xdevcap.xdevcap_cache[i]
				^ old_xdevcap.xdevcap_cache[i];
		}

		temp = 0;
		for (i = 0; i < sizeof(xdevcap_changes); ++i)
			temp |= xdevcap_changes.xdevcap_cache[i];
		if (temp)
			/* Not implemented */
			pr_debug("%s:Not implemented\n", __func__);
		else
			pr_debug("%s:xdevcap did not change\n", __func__);

			set_cbus_command(MHL_READ_DEVCAP, 0, 0);
			pr_debug("%s: DEVCAP Read, after XDEVCAP Read.\n",
				__func__);
	} else if (MHL_READ_XDEVCAP_REG == req->command) {
		MHLXDevCap_u xdev_cap_cache;

		xdev_cap_cache.xdevcap_cache[XDEVCAP_OFFSET(req->reg)] =
			mhl_pf_read_reg(REG_PAGE_5_MSC_MT_RCVD_DATA0);

		if (XDEVCAP_ADDR_ECBUS_SPEEDS == req->reg) {
			mhl_cbus_slot_init((xdev_cap_cache.mxdc.ecbus_speeds
						& MHL_XDC_ECBUS_D_150) != 0);

			mhl_cbus_set_tdm_slot_allocation(
				cbus_context.virt_chan_slot_counts, false);

			pr_debug("%s: ECUBS_SPEED: %02X\n",
				 __func__,
				 xdev_cap_cache.mxdc.ecbus_speeds);

			set_cond_in_cbus_control(READ_ECBUS_SPEED);
		}

	} else if (MHL_READ_EDID_BLOCK == req->command) {
		if (!mhl_drv_connection_is_mhl3())
			mhl_device_edid_set_upstream_edid();
		else
			set_cbus_command(MHL_SEND_3D_REQ_OR_FEAT_REQ,
				MHL_RCHANGE_INT, MHL3_INT_FEAT_REQ);
	} else if (MHL_SET_INT == req->command) {
		/* FIXME Needs to be implemented */
		/* Related to Write Burst */
	} else if (MHL_MSC_MSG == req->command) {
		if (is_Msc_Msg_Ack == false) {
			msleep(1000);
			pr_debug("%s: MSC_NAK, re-trying...\n", __func__);
			/*
			 * Request must be retried, so place it back
			 * on the front of the queue.
			*/
			req->status.as_uint8 = 0;
			queue_priority_cbus_transaction(req);
			req = NULL;
		} else {
			if (MHL_MSC_MSG_RCPK == req->reg) {
			} else if (MHL_MSC_MSG_RCPE == req->reg) {
			} else if (MHL_MSC_MSG_RAP == req->reg) {
			} else if (MHL_MSC_MSG_RAPK == req->reg) {
				if (MHL_RAP_CBUS_MODE_DOWN == cbus_context.msc_msg_last_rcv_data) {
					mhl_device_switch_cbus_mode(CBUS_oCBUS_PEER_IS_MHL3);
					cbus_context.flag.cbus_mode_up = false;
				} else if (MHL_RAP_CBUS_MODE_UP == cbus_context.msc_msg_last_rcv_data) {
					mhl_cbus_send_cbus_mode_up();
				}
			} else if (MHL_MSC_MSG_BIST_REQUEST_STAT == req->reg) {
			} else if (MHL_MSC_MSG_BIST_READY == req->reg) {
			} else {
			}
		}
	} else if (MHL_WRITE_BURST == req->command) {
		/* FIXME Needs to be implemented */
		/* Related to Write Burst */
		cbus_wb_disable_gen2_write_burst_xmit();
	} else if (MHL_SEND_3D_REQ_OR_FEAT_REQ == req->command) {
		pr_debug("%s: Got FEAT_REQ ACK\n", __func__);
	}

	if (req != NULL)
		return_cbus_queue_entry(req);

}

void mhl_msc_hpd_receive()
{
	uint8_t cbus_status;
	uint8_t status;

	cbus_status = mhl_pf_read_reg(REG_PAGE_5_CBUS_STATUS);
	status = cbus_status & BIT_PAGE_5_CBUS_STATUS_CBUS_HPD;

	if (BIT_PAGE_5_CBUS_STATUS_CBUS_HPD
		& (cbus_context.cbus_status ^ cbus_status)) {
		/* bugzilla 27396
		 * No HPD interrupt has been missed yet.
		 * Clear BIT_PAGE_5_CBUS_INT_0_CBUS_INT_0_STAT2.
		 */
		mhl_pf_write_reg(REG_PAGE_5_CBUS_INT_0,
					BIT_PAGE_5_CBUS_INT_0_CBUS_INT_0_STAT2);
		pr_debug("%s: HPD change\n", __func__);
	} else {
		pr_err("%s: missed HPD change\n", __func__);

		/* leave the BIT_PAGE_5_CBUS_INT_0_CBUS_INT_0_STAT2 interrupt
		 * uncleared, so that we get another interrupt
		 */
		/* whatever we missed, it's the inverse of what we got */
		status ^= BIT_PAGE_5_CBUS_STATUS_CBUS_HPD;
		cbus_status ^= BIT_PAGE_5_CBUS_STATUS_CBUS_HPD;
	}

	if (0 == status) {
		pr_info("[CBUS] got CLR_HPD\n");
		clear_cond_in_cbus_control(RCV_HPD);

		/* FIXME
		   Stop EDID Read if EDID read is being done
		   Clear EDID (this process seems to be not necessary
		   if clear is done before EDID read starts) */

	} else {
		pr_info("[CBUS] got SET_HPD\n");
		set_cond_in_cbus_control(RCV_HPD);
	}

	cbus_context.cbus_status = cbus_status;
}

void mhl_msc_write_stat_receive()
{
	uint8_t status_change_bit_mask_0;
	uint8_t status_change_bit_mask_1;
	uint8_t xstatus_change_bit_mask_1;
	uint8_t xstatus_change_bit_mask_3;

	uint8_t status_0;
	uint8_t status_1;
	uint8_t status_2;
	uint8_t xstatus_1;
	uint8_t xstatus_3;

	uint8_t		write_stat[3];
	uint8_t		write_xstat[4];

	mhl_pf_read_reg_block(REG_PAGE_4_MHL_STAT_0,
			      ARRAY_SIZE(write_stat),
			      write_stat);

	mhl_pf_read_reg_block(REG_PAGE_4_MHL_EXTSTAT_0,
			      ARRAY_SIZE(write_xstat),
			      write_xstat);

	status_0 = write_stat[0];
	status_1 = write_stat[1];
	status_2 = write_stat[2];
	xstatus_1 = write_xstat[1];
	xstatus_3 = write_xstat[3];

	status_change_bit_mask_0 = status_0 ^ cbus_context.status_0;
	status_change_bit_mask_1 = status_1 ^ cbus_context.status_1;
	xstatus_change_bit_mask_1 = xstatus_1 ^ cbus_context.xstatus_1;
	xstatus_change_bit_mask_3 = xstatus_3 ^ cbus_context.xstatus_3;

	cbus_context.status_0 = status_0;
	cbus_context.status_1 = status_1;
	cbus_context.status_2 = status_2;
	cbus_context.xstatus_1 = xstatus_1;
	cbus_context.xstatus_3 = xstatus_3;

	pr_info("  write_stat [0:0x%02x][1:0x%02x][2:0x%02x]\n",
		write_stat[0], write_stat[1], write_stat[2]);
	pr_info("  Wwrite_stat[0:0x%02x][1:0x%02x][2:0x%02x][3:0x%02x]\n",
		write_xstat[0], write_xstat[1], write_xstat[2], write_xstat[3]);

	if (MHL_STATUS_DCAP_RDY & status_change_bit_mask_0) {
		pr_info("DCAP_RDY changed\n");
		if (MHL_STATUS_DCAP_RDY & status_0) {
			pr_debug("got DCAP_RDY\n");

			switch (mhl_device_get_cbus_mode()) {
			case CBUS_oCBUS_PEER_VERSION_PENDING:
				if (cbus_context.status_2 >= 0x30) {
					if (MHL_STATUS_XDEVCAPP_SUPP & status_0) {
						pr_debug("%s: speer is MHL3 or newer\n",
							__func__);
						/* Our peer is an MHL3.0 or newer device.
						   Here we enable DEVCAP_X and STATUS_X operations */
						mhl_device_set_cbus_mode(CBUS_oCBUS_PEER_IS_MHL3);

						mhl_pf_write_reg( REG_PAGE_3_M3_CTRL,
							VAL_PAGE_3_M3_CTRL_MHL3_VALUE);
					}
				}
				break;
			default:
				break;
			}

			/* after the above does not indicate MHL3,
			   then it's MHL1.x or MHL2.x */
			if (CBUS_oCBUS_PEER_VERSION_PENDING == mhl_device_get_cbus_mode()) {
				pr_debug("%s: speer is MHL2 or older\n",
					__func__);

				mhl_device_switch_cbus_mode(
					CBUS_oCBUS_PEER_IS_MHL1_2);
			}

			switch (mhl_device_get_cbus_mode()) {
			case CBUS_oCBUS_PEER_IS_MHL1_2:
				set_cbus_command(MHL_READ_DEVCAP, 0, 0);
				break;
			case CBUS_oCBUS_PEER_IS_MHL3:
				set_cbus_command(MHL_READ_XDEVCAP_REG,
					XDEVCAP_ADDR_ECBUS_SPEEDS,
					0);
				break;
			default:
				break;
			}
			/*
				Now that we have positively identified
					the MHL version of the peer,
					we re-evaluate our settings.
			*/

			peer_specific_init();

			mhl_pf_write_reg(REG_EDID_FIFO_INT_MASK,
					 (BIT_INTR9_DEVCAP_DONE_MASK
					 | BIT_INTR9_EDID_DONE_MASK
					 | BIT_INTR9_EDID_ERROR));
		}
	}

	if (MHL_STATUS_PATH_ENABLED & status_change_bit_mask_1) {
		pr_info("PATH_EN changed\n");

		if (MHL_STATUS_PATH_ENABLED & status_1) {
			pr_info("got PATH_EN 1\n");

			set_cond_in_cbus_control(SENT_PATH_EN_1);
			cbus_context.link_mode |= MHL_STATUS_PATH_ENABLED;
			set_cbus_command(MHL_WRITE_STAT, MHL_STATUS_REG_LINK_MODE,
					 cbus_context.link_mode);

		} else{
			pr_info("got PATH_EN 0\n");

			clear_cond_in_cbus_control(SENT_PATH_EN_1);
			cbus_context.link_mode &= ~MHL_STATUS_PATH_ENABLED;
			set_cbus_command(MHL_WRITE_STAT, MHL_STATUS_REG_LINK_MODE,
					 cbus_context.link_mode);
		}
	}

	if (MHL_XDS_LINK_STATUS_TMDS_NORMAL & xstatus_1) {
		if (mhl_device_get_hdcp_status()) {
			pr_debug("AV LINK_MODE_STATUS is NORMAL\n");

			set_cond_in_cbus_control(TMDS_READY);
			if (get_cond_in_cbus_control(HDCP_READY)) {
				pr_debug("Start HDCP\n");
				start_hdcp();
			} else {
				pr_debug("HDCP is not ready\n");
			}
		}
	} else {
		pr_debug("AV LINK_MODE_STATUS not NORMAL\n");
	}

}

void mhl_msc_msg_receive()
{
	int st = 0;
	uint8_t sub_cmd, cmd_data;
	sub_cmd = mhl_pf_read_reg(REG_PAGE_5_MSC_MR_MSC_MSG_RCVD_1ST_DATA);
	cmd_data = mhl_pf_read_reg(REG_PAGE_5_MSC_MR_MSC_MSG_RCVD_2ND_DATA);

	switch (sub_cmd) {
	case MHL_MSC_MSG_RCP:
		pr_info("%s: MHL: receive RCP(0x%02x)\n", __func__, cmd_data);
		st = mhl_tx_rcp_input(cmd_data);
		if (st == 0) {
			set_cbus_command(MHL_MSC_MSG, MHL_MSC_MSG_RCPK,
					cmd_data);
		} else{
			if (set_cbus_command(MHL_MSC_MSG, MHL_MSC_MSG_RCPE,
					MHL_RCPE_STATUS_INEFFECTIVE_KEY_CODE)){
				set_cbus_command(MHL_MSC_MSG,
						MHL_MSC_MSG_RCPK, cmd_data);
			}
		}
		break;
	case MHL_MSC_MSG_RCPK:
		pr_info("%s: MHL: receive RCPK(0x%02x)\n",
				__func__, cmd_data);
		break;
	case MHL_MSC_MSG_RCPE:
		pr_info("%s: MHL: receive RCPE(0x%02x)\n",
				__func__, cmd_data);
		break;
	case MHL_MSC_MSG_RAP:
		pr_info("%s: MHL: receive RAP(0x%02x)\n",
				__func__, cmd_data);
		cbus_context.msc_msg_last_rcv_data = cmd_data;
		set_cbus_command(MHL_MSC_MSG, MHL_MSC_MSG_RAPK,
						MHL_RAPK_NO_ERR);

		switch (cmd_data) {
		case MHL_RAP_CONTENT_ON:
			if (cbus_context.flag.video_ready) {
				if (is_video_muted() && (is_tmds_active() == false))
					start_video();
			}
			break;
		case MHL_RAP_CONTENT_OFF:
			if (cbus_context.flag.video_ready) {
				if (is_video_muted() == false) {
					stop_video();
				} else {
					pr_debug("%s:Video has been already muted\n",
						 __func__);
				}
			} else {
			pr_debug("%s:Video has been not ready yet\n", __func__);
			}
			break;
		}
		break;
	case MHL_MSC_MSG_RAPK:
		pr_info("%s: MHL: receive RAPK(0x%02x)\n",
				__func__, cmd_data);
		mhl_rapk_receive_timer_stop();
		mhl_dev_clear_cbusp_cond_processing(DEV_RAP_SEND);

		if (MHL_RAP_CBUS_MODE_DOWN == cbus_context.msc_msg_last_send_data) {
			mhl_device_switch_cbus_mode(CBUS_oCBUS_PEER_IS_MHL3);
			cbus_context.flag.cbus_mode_up = false;
		} else if (MHL_RAP_CBUS_MODE_UP == cbus_context.msc_msg_last_send_data) {
			if (MHL_RAPK_NO_ERR == cmd_data) {
				pr_debug("%s: CBUS_MODE_UP register setting starts\n", __func__);
				mhl_device_switch_cbus_mode(CBUS_eCBUS_S);
			} else if (MHL_RAPK_BUSY == cmd_data) {
				set_cbus_command(MHL_MSC_MSG, MHL_MSC_MSG_RAP,
						 MHL_RAP_CBUS_MODE_UP);
			} else {
				pr_debug("%s: waiting from RAP{CBUS_MODE_UP} from sink\n",
					 __func__);
			}
		}
		break;
	default:
		pr_warn("%s: MHL: receive invalid command 0x%02x (0x%02x)\n",
				__func__, sub_cmd, cmd_data);
		break;
	}
}

void mhl_msc_set_int_receive()
{
	uint8_t		int_msg[2];
	uint8_t intr_0;
	uint8_t intr_1;

	/* read SET_INT bits */
	mhl_pf_read_reg_block(REG_PAGE_4_MHL_INT_0,
			      ARRAY_SIZE(int_msg),
			      int_msg);

	/* clear the individual SET_INT bits */
	mhl_pf_write_reg_block(REG_PAGE_4_MHL_INT_0,
			       ARRAY_SIZE(int_msg),
			       int_msg);

	intr_0 = int_msg[0];
	intr_1 = int_msg[1];

	if (MHL_INT_DCAP_CHG & intr_0) {
		pr_info("got DCAP_CHG\n");

		if (MHL_STATUS_DCAP_RDY & cbus_context.status_0) {
			pr_debug("got DCAP_CHG & DCAP_RDY\n");

			if (mhl_device_is_peer_device_mhl3()) {
				if (mhl_device_get_cbus_mode() < CBUS_eCBUS_S) {
					set_cond_in_cbus_control(RCV_DCAP_CHG);
					set_cbus_command(MHL_READ_XDEVCAP_REG, XDEVCAP_ADDR_ECBUS_SPEEDS, 0);
				} else {
					set_cbus_command(MHL_READ_XDEVCAP, 0, 0);
				}
			} else {
				set_cbus_command(MHL_READ_DEVCAP, 0, 0);
			}
		}
	}

	if ((MHL_INT_DSCR_CHG | MHL_INT_REQ_WRT
		| MHL3_INT_FEAT_COMPLETE| MHL3_INT_FEAT_REQ)
		& intr_0) {
		cbus_wb_event_handler(intr_0);
	}

	if (MHL_INT_EDID_CHG & intr_1) {
		pr_info("got EDID_CHG\n");
		clear_cond_in_cbus_control(RCV_HPD);
		set_cond_in_cbus_control(RCV_HPD);
	}
}

/*
 *
 *  Others
 *
 */



int get_device_id(void)
{
	int ret_val;
	uint16_t number;

	ret_val = mhl_pf_read_reg(REG_PAGE_0_DEV_IDH);
	if (ret_val < 0) {
		pr_err("%s:I2C error 0x%x\n", __func__, ret_val);
		return ret_val;
	}
	number = ret_val << 8;

	ret_val = mhl_pf_read_reg(REG_PAGE_0_DEV_IDL);
	if (ret_val < 0) {
		pr_err("%s:I2C error 0x%x\n", __func__, ret_val);
		return ret_val;
	}
	ret_val |= number;

	return ret_val;
}

#define DEVCAP_REG(x) REG_PAGE_4_MHL_DEVCAP_0 | DEVCAP_OFFSET_##x
#define XDEVCAP_REG(x) REG_PAGE_4_MHL_EXTDEVCAP_0 | XDEVCAP_OFFSET(XDEVCAP_ADDR_##x)

void init_devcap(void){
	/* Setup local DEVCAP registers */

	mhl_pf_write_reg_block(DEVCAP_REG(DEV_STATE),
			       ARRAY_SIZE(dev_cap_values), dev_cap_values);

	/* Setup local XDEVCAP registers */

	mhl_pf_write_reg_block(XDEVCAP_REG(ECBUS_SPEEDS),
			       ARRAY_SIZE(xdev_cap_values), xdev_cap_values);
}

static void read_devcap_fifo(uint8_t *dev_cap_buf, u8 size_dev, bool xdev)
{
	pr_debug("%s called\n", __func__);

	mhl_pf_write_reg(REG_EDID_FIFO_INT_MASK,
			 (BIT_INTR9_DEVCAP_DONE_MASK
			 | BIT_INTR9_EDID_DONE_MASK
			 | BIT_INTR9_EDID_ERROR));

	if (xdev)
		/* choose xdevcap instead of EDID to appear at the FIFO */
		mhl_pf_write_reg(REG_PAGE_2_EDID_CTRL,
			VAL_PAGE_2_EDID_CTRL_EDID_PRIME_VALID_DISABLE |
			BIT_PAGE_2_EDID_CTRL_XDEVCAP_EN |
			VAL_PAGE_2_EDID_CTRL_DEVCAP_SELECT_DEVCAP |
			VAL_PAGE_2_EDID_CTRL_EDID_FIFO_ADDR_AUTO_ENABLE |
			VAL_PAGE_2_EDID_CTRL_EDID_MODE_EN_ENABLE
		);
	else
		/* choose devcap instead of EDID to appear at the FIFO */
		mhl_pf_write_reg(REG_PAGE_2_EDID_CTRL,
			VAL_PAGE_2_EDID_CTRL_EDID_PRIME_VALID_DISABLE |
			VAL_PAGE_2_EDID_CTRL_DEVCAP_SELECT_DEVCAP |
			VAL_PAGE_2_EDID_CTRL_EDID_FIFO_ADDR_AUTO_ENABLE |
			VAL_PAGE_2_EDID_CTRL_EDID_MODE_EN_ENABLE
		);

		mhl_pf_write_reg(REG_PAGE_2_EDID_FIFO_ADDR, 0);

		/* Retrieve the DEVCAP register values from the FIFO */
		mhl_pf_read_reg_block(REG_PAGE_2_EDID_FIFO_RD_DATA,
		size_dev,
		dev_cap_buf);
}

void mhl_cbus_abort(uint8_t cbus_err_int)
{
	uint8_t msc_abort_reason = 0;

	if (cbus_err_int & BIT_CBUS_DDC_ABORT) {
		uint8_t ddc_abort_reason = 0;

		ddc_abort_reason = mhl_pf_read_reg(REG_PAGE_5_DDC_ABORT_INT);
		pr_err("%s: CBUS DDC ABORT. Reason = %02X\n",
			__func__, ddc_abort_reason);

		if (DDC_ABORT_THRESHOLD < ++cbus_context.ddc_abort_count) {
			mhl_sii8620_device_edid_reset_ddc_fifo();
			cbus_context.ddc_abort_count = 0;
			pr_err("%s: Please Reset Sink Device\n", __func__);
			/* FIMXE
			   if error persists, reset the chip. */
		} else if (cbus_context.current_cbus_req != NULL) {
			if (MHL_READ_EDID_BLOCK ==
				cbus_context.current_cbus_req->command) {
				mhl_sii8620_device_edid_reset_ddc_fifo();
				pr_warn("%s: CBUS DDC ABORT. Count = %d\n",
					__func__, cbus_context.ddc_abort_count);
			/* FIXME
			   hw_context->intr_info->flags |= DRV_INTR_FLAG_MSC_DONE;
			   setting 1 indicates there was an error
			   hw_context->intr_info->msc_done_data =1; */
			}
		}
	}
	if (cbus_err_int & BIT_CBUS_MSC_ABORT_RCVD) {
		/*
		 * For MSC Receive time ABORTs
		 * - Defer submission of new commands by 2 seconds per MHL specs
		 * - This is not even worth reporting to SoC. Action is in the hands of peer.
		 */

		msc_abort_reason = mhl_pf_read_reg(REG_PAGE_5_MSC_MR_ABORT_INT);
		++cbus_context.msc_abort_count;
		pr_debug("%s: #%d: ABORT during MSC RCV. Reason = %02X\n",
			 __func__, cbus_context.msc_abort_count, msc_abort_reason);
		/* Retry CBUS command*/
		process_cbus_abort();

		/* FIXME
		   msc_abort_count is not initialized anywhere */
	}
	if (cbus_err_int & BIT_CBUS_CMD_ABORT) {
		/*
		 * 1. Defer submission of new commands by 2 seconds per MHL specs
		 * 2. For API operations such as RCP/UCP etc., report the situation to
		 *    SoC and let the decision be there. Internal retries have been already
		 *    done.
		 */

		msc_abort_reason = mhl_pf_read_reg(REG_PAGE_5_MSC_MT_ABORT_INT);
		pr_debug("%s: CBUS ABORT during MSC SEND. Reason = %02X\n",
			 __func__, msc_abort_reason);
		mhl_pf_write_reg(REG_PAGE_5_MSC_MT_ABORT_INT, msc_abort_reason);
		/* Retry CBUS command*/
		process_cbus_abort();
	}

/* Print the reason for information */
	if (msc_abort_reason) {
		if (BIT_CBUS_MSC_MT_ABORT_INT_MAX_FAIL & msc_abort_reason) {
			pr_warn("%s: Retry threshold exceeded\n", __func__);
		}
		if (BIT_CBUS_MSC_MT_ABORT_INT_PROTO_ERR & msc_abort_reason) {
			pr_warn("%s: Protocol Error\n", __func__);
		}
		if (BIT_CBUS_MSC_MT_ABORT_INT_TIMEOUT & msc_abort_reason) {
			pr_warn("%s: Translation layer timeout\n", __func__);
		}
		if (BIT_CBUS_MSC_MT_ABORT_INT_UNDEF_CMD & msc_abort_reason) {
			pr_warn("%s: Undefined opcode\n", __func__);
		}
		if (BIT_CBUS_MSC_MT_ABORT_INT_MSC_MT_PEER_ABORT & msc_abort_reason) {
			pr_warn("%s: MSC Peer sent an ABORT\n", __func__);
		}
	}
}

static void process_cbus_abort(void)
{
	struct cbus_req *req;

	/*
	 * Place the CBUS message that errored back on
	 * transmit queue if it has any retries left.
	 */
	if (cbus_context.current_cbus_req != NULL) {
		req = cbus_context.current_cbus_req;
		cbus_context.current_cbus_req = NULL;
		if (req->retry_count) {
			req->retry_count -= 1;
			queue_priority_cbus_transaction(req);
		} else {
			return_cbus_queue_entry(req);
		}
	}

	/* Delay the sending of any new CBUS messages for 2 seconds */
	cbus_context.cbus_abort_delay_active = true;
	mhl_lib_timer_start(cbus_context.cbus_abort_timer, 2000);

}

bool mhl_cbus_is_sink_support_scratchpad(void)
{
	return cbus_context.devcap.mdc.featureFlag & MHL_FEATURE_SP_SUPPORT;
}

bool mhl_cbus_is_sink_support_ppixel(void)
{
	return (cbus_context.devcap.mdc.vid_link_mode &
		MHL_DEV_VID_LINK_SUPP_PPIXEL);
}

bool mhl_cbus_is_source_support_ppixel(void)
{
	return DEVCAP_VAL_VID_LINK_MODE & MHL_DEV_VID_LINK_SUPP_PPIXEL;
}

bool mhl_cbus_packed_pixel_available(void)
{
	if (mhl_cbus_is_sink_support_ppixel() &&
	    mhl_cbus_is_source_support_ppixel()) {
		return true;
	} else{
		return false;
	}
}

bool mhl_cbus_is_sink_support_16bpp(void)
{
	return cbus_context.devcap.mdc.vid_link_mode
			& MHL_DEV_VID_LINK_SUPP_16BPP;
}

bool mhl_cbus_is_source_support_16bpp(void)
{
	return DEVCAP_VAL_VID_LINK_MODE & MHL_DEV_VID_LINK_SUPP_16BPP;
}

bool mhl_cbus_16bpp_available(void)
{
	if (mhl_cbus_is_sink_support_16bpp() &&
	    mhl_cbus_is_source_support_16bpp()) {
		return true;
	} else{
		return false;
	}
}

static int send_content_on(void)
{
	bool ret = false;
	pr_debug("%s called\n", __func__);

	ret = set_cbus_command(MHL_MSC_MSG, MHL_MSC_MSG_RAP,
			MHL_RAP_CONTENT_ON);
	if (!ret) {
		pr_err("%s: Failed to send rap_content_on\n", __func__);
		return MHL_FAIL;
	}

	return 0;
}

static bool is_rap_supported(void)
{
	bool ret;
	int rap;

	rap = cbus_context.devcap.devcap_cache[MHL_DEV_FEATURE_FLAG_OFFSET] &
			MHL_FEATURE_RAP_SUPPORT;
	if (MHL_FEATURE_RAP_SUPPORT == rap) {
		ret = true;
	} else {
		pr_debug("%s: Not supported RAP\n", __func__);
		ret = false;
	}

	return ret;
}

static bool is_ready_for_edid_read(void)
{
	bool ret = false;

	if (get_cond_in_cbus_control(DEVCAP_ALL_READ_DONE) &
	    get_cond_in_cbus_control(RCV_HPD)) {
		ret = true;
	}

	return ret;
}

void init_cond_in_cbus_control(void)
{
	cbus_context.flag.devcap_all = false;
	cbus_context.flag.path_en = false;
	cbus_context.flag.hpd = false;
	cbus_context.flag.video_ready = false;
	cbus_context.flag.dcap_chg_rcv = false;
	cbus_context.flag.cbus_mode_up = false;
	cbus_context.flag.hdcp_ready = false;
	cbus_context.flag.tmds_ready = false;
}

void set_cond_in_cbus_control(CBUS_CONTROL_COND cond)
{
	pr_debug("%s()\n", __func__);
	show_cond_in_cbus_control();

	switch (cond) {
	case DEVCAP_ALL_READ_DONE:
		pr_debug("%s:devcap_all\n", __func__);
		cbus_context.flag.devcap_all = true;
		if (is_ready_for_edid_read())
			set_cbus_command(MHL_READ_EDID_BLOCK, 0, 0);

		if (is_rap_supported()) {
			pr_debug("%s:rap_content_on request started\n",
					__func__);
			send_content_on();
		}
		break;

	case SENT_PATH_EN_1:
		pr_debug("%s:path en\n", __func__);
		cbus_context.flag.path_en = true;
		if (cbus_context.flag.video_ready) {
			if (is_video_muted() && (is_tmds_active() == false))
				start_video();
		}
		break;

	case RCV_HPD:
		pr_debug("%s:receive hpd\n", __func__);
		cbus_context.flag.hpd = true;

		if (is_ready_for_edid_read())
			set_cbus_command(MHL_READ_EDID_BLOCK, 0, 0);
		else
			pr_debug("%s:Waiting for the complete of DEVCAP Read\n", __func__);
		break;

	case UPSTREAM_VIDEO_READY:
		pr_debug("%s:video ready\n", __func__);
		cbus_context.flag.video_ready = true;
		break;
	case RCV_DCAP_CHG:
		pr_debug("%s:receive DCAP_CHG\n", __func__);
		cbus_context.flag.dcap_chg_rcv = true;
		break;
	case READ_ECBUS_SPEED:
		pr_debug("%s:read ECBUS_SPEED\n", __func__);
		if (cbus_context.flag.dcap_chg_rcv)
			mhl_cbus_send_cbus_mode_up();
		break;
	case HDCP_READY:
		cbus_context.flag.hdcp_ready = true;
		break;
	case TMDS_READY:
		cbus_context.flag.tmds_ready = true;
		break;
	default:
		pr_warn("%s: unknown condition=%d\n", __func__, cond);
		return;
	}

	show_cond_in_cbus_control();
}

void clear_cond_in_cbus_control(CBUS_CONTROL_COND cond)
{
	pr_debug("%s()\n", __func__);

	show_cond_in_cbus_control();

	switch (cond) {
	case DEVCAP_ALL_READ_DONE:
		pr_debug("%s:devcap_all\n", __func__);
		cbus_context.flag.devcap_all = false;
		break;

	case SENT_PATH_EN_1:
		pr_debug("%s:path en\n", __func__);
		cbus_context.flag.path_en = false;
		if (cbus_context.flag.video_ready) {
			if (is_video_muted() == false) {
				stop_video();
			} else {
				pr_debug("%s:Video has been already muted\n",
					 __func__);
			}
		} else {
			pr_debug("%s:Video has been not ready yet\n", __func__);
		}
		break;

	case RCV_HPD:
		pr_debug("%s:receice hpd\n", __func__);
		cbus_context.flag.hpd = false;
		cbus_context.flag.video_ready = false;
		cbus_context.flag.tmds_ready = false;
		cbus_context.flag.hdcp_ready = false;
		stop_video();
		drive_hpd_low();

		break;

	case UPSTREAM_VIDEO_READY:
		pr_debug("%s:video ready\n", __func__);
		cbus_context.flag.video_ready = false;
		break;

	case RCV_DCAP_CHG:
		pr_debug("%s:receive DCAP_CHG\n", __func__);
		cbus_context.flag.dcap_chg_rcv = false;
		break;

	case HDCP_READY:
		cbus_context.flag.hdcp_ready = false;
		break;

	case TMDS_READY:
		cbus_context.flag.tmds_ready = false;
		break;

	default:
		pr_warn("%s: unknown condition=%d\n", __func__, cond);
	}

	show_cond_in_cbus_control();
}

bool get_cond_in_cbus_control(CBUS_CONTROL_COND cond)
{
	bool flag = false;

	pr_debug("%s()\n", __func__);

	switch (cond) {
	case DEVCAP_ALL_READ_DONE:
		pr_debug("%s:devcap_all\n", __func__);
		flag = cbus_context.flag.devcap_all;
		break;
	case SENT_PATH_EN_1:
		pr_debug("%s:path en\n", __func__);
		flag = cbus_context.flag.path_en;
		break;
	case RCV_HPD:
		pr_debug("%s:receice hpd\n", __func__);
		flag = cbus_context.flag.hpd;
		break;
	case UPSTREAM_VIDEO_READY:
		pr_debug("%s:video ready\n", __func__);
		flag = cbus_context.flag.video_ready;
		break;
	case RCV_DCAP_CHG:
		pr_debug("%s:receive DCAP_CHG\n", __func__);
		flag = cbus_context.flag.dcap_chg_rcv;
		break;
	case HDCP_READY:
		pr_debug("%s:hdcp ready\n", __func__);
		flag = cbus_context.flag.hdcp_ready;
		break;
	case TMDS_READY:
		pr_debug("%s:tmds ready\n", __func__);
		flag = cbus_context.flag.tmds_ready;
		break;
	default:
		pr_warn("%s: unknown condition=%d\n", __func__, cond);
	}

	return flag;
}

void show_cond_in_cbus_control(void){
	pr_debug("%s DEVCAP_ALL_READ_DONE %d\n",
		 __func__, cbus_context.flag.devcap_all);
	pr_debug("%s SENT_PATH_EN_1       %d\n",
		 __func__, cbus_context.flag.path_en);
	pr_debug("%s RCV_HPD              %d\n",
		 __func__, cbus_context.flag.hpd);
	pr_debug("%s UPSTREAM_VIDEO_READY %d\n",
		 __func__, cbus_context.flag.video_ready);
	pr_debug("%s receive DCAP_CHG %d\n",
		 __func__, cbus_context.flag.dcap_chg_rcv);
	pr_debug("%s HDCP2.2_READY %d\n",
		 __func__, cbus_context.flag.hdcp_ready);
	pr_debug("%s TMDS_READY %d\n",
		 __func__, cbus_context.flag.tmds_ready);
}

int mhl_cbus_control_initialize(void)
{
	int rc = 0;

	mhl_create_timer();

	i2c_mem.block_tx_buffers
		= kmalloc(MAX_I2C_EMSC_BLOCK_SIZE
				* NUM_BLOCK_QUEUE_REQUESTS, GFP_KERNEL);
	if (NULL == i2c_mem.block_tx_buffers) {
		kfree(i2c_mem.block_tx_buffers);
		rc = -ENOMEM;
	}

	mhl_rapk_receive_timer_init();

	return rc;
}

void mhl_cbus_control_release(void)
{
	mhl_release_timer();

	if (!i2c_mem.block_tx_buffers) {
		kfree(i2c_mem.block_tx_buffers);
	}

	mhl_rapk_receive_timer_release();
}

static int mhl_release_timer(void)
{
	int ret;

	ret = mhl_lib_timer_delete(&cbus_context.cbus_abort_timer);

	if (ret != 0) {
		pr_err("%s: Failed to release CBUS abort timer!\n", __func__);
		return ret;
	}

	return ret;
}

static int mhl_create_timer(void)
{
	int ret;

	ret = mhl_lib_timer_create(cbus_abort_timer_callback,
				  NULL,
				  &cbus_context.cbus_abort_timer);

	if (ret != 0) {
		pr_err("%s: Failed to allocate CBUS abort timer!\n", __func__);
		return ret;
	}

	return ret;
}

static void cbus_abort_timer_callback(void *callback_param)
{
	pr_warn("%s: CBUS abort timer expired, enable CBUS messaging\n",
		 __func__);

	cbus_context.cbus_abort_delay_active = false;
	exe_cbus_command();
}

uint8_t mhl_get_tdm_virt_chan_slot_counts(enum tdm_vc_assignments vc)
{
	uint8_t ret = 1;
	switch (vc) {
	case TDM_VC_CBUS1:
		ret = cbus_context.virt_chan_slot_counts[TDM_VC_CBUS1];
		break;
	case TDM_VC_E_MSC:
		ret = cbus_context.virt_chan_slot_counts[TDM_VC_E_MSC];
		break;
	case TDM_VC_T_CBUS:
		ret = cbus_context.virt_chan_slot_counts[TDM_VC_T_CBUS];
		break;
	default:
		break;
	}

	return ret;
}

void send_link_mode_to_sink(void)
{
	if (!is_SamsungTV())
		set_cbus_command(MHL_WRITE_STAT, MHL_STATUS_REG_LINK_MODE,
				cbus_context.link_mode);
}

void set_link_mode(u8 link_mode)
{
	cbus_context.link_mode = link_mode;
}

u8 get_link_mode(void)
{
	return 	cbus_context.link_mode;
}

int mhl_cbus_get_highest_tmds_link_speed(void)
{
	int link_speed = MHL_XDC_TMDS_000;

	if (cbus_context.xdevcap.xdevcap_cache[XDEVCAP_OFFSET(XDEVCAP_ADDR_TMDS_SPEEDS)] & MHL_XDC_TMDS_600)
		link_speed = MHL_XDC_TMDS_600;
	else if (cbus_context.xdevcap.xdevcap_cache[XDEVCAP_OFFSET(XDEVCAP_ADDR_TMDS_SPEEDS)] & MHL_XDC_TMDS_300)
		link_speed = MHL_XDC_TMDS_300;
	else if (cbus_context.xdevcap.xdevcap_cache[XDEVCAP_OFFSET(XDEVCAP_ADDR_TMDS_SPEEDS)] & MHL_XDC_TMDS_150)
		link_speed = MHL_XDC_TMDS_150;

	return link_speed;
}

void mhl_cbus_set_lowest_tmds_link_speed(uint32_t pixel_clock_frequency,
											uint8_t bits_per_pixel)
{
	uint32_t link_clock_frequency;
	/* That is based on the MHL protocol limitation.
	Safety value is recommended as default.*/
	uint32_t top_clock_frequency = 147000000;
	uint8_t reg_val;
	bool found_fit;

	link_clock_frequency = pixel_clock_frequency * ((uint32_t)(bits_per_pixel >> 3));
	found_fit = false;

	/*
	  find lowest fit giving up only if module parameter
	  forces us into a supported link speed
	*/
	if (cbus_context.xdevcap.xdevcap_cache[XDEVCAP_OFFSET(XDEVCAP_ADDR_TMDS_SPEEDS)] & MHL_XDC_TMDS_150) {

		pr_debug("%s: XDEVCAP TMDS Link Speed = 1.5Gbps is supported\n",
				__func__);
		top_clock_frequency = 147000000;/* 1500000 * 98 */

		if (xdev_cap_values[XDEVCAP_OFFSET(XDEVCAP_ADDR_TMDS_SPEEDS)]
				& MHL_XDC_TMDS_150) {
			if ((link_clock_frequency <= 150000000)
				&& (!found_fit)) {
				pr_info("%s: Mode fits TMDS Link Speed = 1.5Gbps (%d)\n",
					__func__, link_clock_frequency);
				reg_val = VAL_PAGE_3_TX_ZONE_CTL3_TX_ZONE_1_5GBPS;
				found_fit = true;
			}
		}
	}
	if (cbus_context.xdevcap.xdevcap_cache[XDEVCAP_OFFSET(XDEVCAP_ADDR_TMDS_SPEEDS)] & MHL_XDC_TMDS_300) {

		pr_debug("%s: XDEVCAP TMDS Link Speed = 3.0Gbps is supported\n",
				__func__);
		top_clock_frequency = 294000000;/* 3000000 * 98 */

		if (xdev_cap_values[XDEVCAP_OFFSET(XDEVCAP_ADDR_TMDS_SPEEDS)]
				& MHL_XDC_TMDS_300) {
			if ((link_clock_frequency <= 300000000)
				&& (!found_fit)) {
				pr_info("%s: Mode fits TMDS Link Speed = 3.0Gbps (%d)\n",
					__func__, link_clock_frequency);
				reg_val = VAL_PAGE_3_TX_ZONE_CTL3_TX_ZONE_3GBPS;
				found_fit = true;
			}
		}
	}
	if (cbus_context.xdevcap.xdevcap_cache[XDEVCAP_OFFSET(XDEVCAP_ADDR_TMDS_SPEEDS)] & MHL_XDC_TMDS_600) {

		pr_debug("%s: XDEVCAP TMDS Link Speed = 6.0Gbps is supported\n",
				__func__);
		top_clock_frequency = 588000000;/* 6000000 * 98 */

		if (xdev_cap_values[XDEVCAP_OFFSET(XDEVCAP_ADDR_TMDS_SPEEDS)]
			& MHL_XDC_TMDS_600) {
			if ((link_clock_frequency <= 600000000)
				&& (!found_fit)) {
				pr_info("%s: Mode fits TMDS Link Speed = 6.0Gbps (%d)\n",
					__func__, link_clock_frequency);
				reg_val = VAL_PAGE_3_TX_ZONE_CTL3_TX_ZONE_6GBPS;
				found_fit = true;
			}
		}
	}

	if (!found_fit) {
		pr_info("%s: Cannot fit mode to any supported TMDS Link Speeds\n",
				__func__);
		pr_info("%s: Forcing TMDS Link Speed = 6.0Gbps\n",
				__func__);
		reg_val = VAL_PAGE_3_TX_ZONE_CTL3_TX_ZONE_6GBPS;
	}

	mhl_pf_write_reg(REG_PAGE_3_MHL3_TX_ZONE_CTL, reg_val);

	/* set unlimited packet size only if not enough overhead */
	pr_debug("%s: lcf = %d, tcf = %d\n",
		__func__,
		link_clock_frequency,
		top_clock_frequency);

	if (link_clock_frequency >= top_clock_frequency) {
		pr_debug("%s: Program 3E1[3] = 1 --- UNLIMITED MODE ON\n",
			__func__);
		mhl_pf_modify_reg(REG_PAGE_3_M3_P0CTRL,
			BIT_PAGE_3_M3_P0CTRL_MHL3_P0_PORT_EN |
			BIT_PAGE_3_M3_P0CTRL_MHL3_P0_UNLIMIT_EN,
			BIT_PAGE_3_M3_P0CTRL_MHL3_P0_PORT_EN |
			BIT_PAGE_3_M3_P0CTRL_MHL3_P0_UNLIMIT_EN_PP);
	} else {
		pr_debug("%s: Program 3E1[3] = 0 --- UNLIMITED MODE OFF\n",
			__func__);
		mhl_pf_modify_reg(REG_PAGE_3_M3_P0CTRL,
			BIT_PAGE_3_M3_P0CTRL_MHL3_P0_PORT_EN |
			BIT_PAGE_3_M3_P0CTRL_MHL3_P0_UNLIMIT_EN,
			BIT_PAGE_3_M3_P0CTRL_MHL3_P0_PORT_EN |
			BIT_PAGE_3_M3_P0CTRL_MHL3_P0_UNLIMIT_EN_NORM);
	}

	/* set write stat indicating this change */
	switch (reg_val) {
	case VAL_PAGE_3_TX_ZONE_CTL3_TX_ZONE_6GBPS:
		set_cbus_command(MHL_WRITE_XSTAT,
			MHL_STATUS_REG_AV_LINK_MODE_CONTROL,
			0x02);
		break;
	case VAL_PAGE_3_TX_ZONE_CTL3_TX_ZONE_3GBPS:
		set_cbus_command(MHL_WRITE_XSTAT,
			MHL_STATUS_REG_AV_LINK_MODE_CONTROL,
			0x01);
		break;
	case VAL_PAGE_3_TX_ZONE_CTL3_TX_ZONE_1_5GBPS:
		set_cbus_command(MHL_WRITE_XSTAT,
			MHL_STATUS_REG_AV_LINK_MODE_CONTROL,
			0x00);
		break;
	}

}

int block_input_buffer_available(void)
{
	uint16_t head, tail;
	head = cbus_context.hw_block_protocol.head;
	tail = cbus_context.hw_block_protocol.tail;
	if (head == (tail+1)) {
		/* full case */
		return 0;
	}
	return 1;
}

int alloc_block_input_buffer(uint8_t **pbuffer)
{
	uint16_t head, tail;
	int index;
	head = cbus_context.hw_block_protocol.head;
	tail = cbus_context.hw_block_protocol.tail;

	if (!block_input_buffer_available()) {
		/* full case */
		return MHL_FAIL;
	}
	index = tail;

	if (++tail >= NUM_BLOCK_INPUT_BUFFERS)
		tail = 0;

	cbus_context.hw_block_protocol.tail = tail;

	*pbuffer = &cbus_context.hw_block_protocol.input_buffers[index][0];
	return index;
}

void set_block_input_buffer_length(int block, int length)
{
	if ((block < 0) || (block > (NUM_BLOCK_INPUT_BUFFERS - 1)))
		return;

	cbus_context.hw_block_protocol.input_buffer_lengths[block] = length;
}

void add_received_byte_count(uint16_t size)
{
	cbus_context.hw_block_protocol.received_byte_count += size;
	pr_debug("%s: received_byte_count->%d",
			__func__,
			cbus_context.hw_block_protocol.received_byte_count);
}

void add_peer_blk_rx_buffer_avail(unsigned long size)
{
	cbus_context.hw_block_protocol.peer_blk_rx_buffer_avail += size;
}

static void return_block_queue_entry_impl(struct block_req *pReq,
							const char *function,
							int line)
{
	list_add(&pReq->link, &cbus_context.block_protocol.free_list);

}
#define return_block_queue_entry(req) return_block_queue_entry_impl(req, __func__, __LINE__)

/*
 * mhl_tx_drv_send_block
 *
 * Write the specified Sideband Channel command to the CBUS.
 * such as READ_DEVCAP, SET_INT, WRITE_STAT, etc.
 * Command can be a MSC_MSG command (RCP/RAP/RCPK/RCPE/RAPK), or another command
 * Parameters:
 *              req     - Pointer to a cbus_req_t structure containing the
 *                        command to write
 * Returns:     true    - successful write
 *              false   - write failed
 */
void mhl_tx_drv_send_block(struct block_req *req)
{
	uint8_t count;

	count = req->sub_payload_size;
	pr_debug(" req->sub_payload_size: %d req->count: %d\n",
			count, req->count);

	pr_debug("total bytes to ack: %d\n",
			cbus_context.hw_block_protocol.received_byte_count);
	if (cbus_context.hw_block_protocol.received_byte_count >= 256) {

		/*
		  'can't represent numbers >= 256 in 8 bits,
		  so ack as much as we can ...
		*/
		req->payload->hdr_and_burst_id.tport_hdr.rx_unload_ack = 255;
		cbus_context.hw_block_protocol.received_byte_count -= 255;
	} else {
		req->payload->hdr_and_burst_id.tport_hdr.rx_unload_ack
			= (uint8_t)cbus_context.
				hw_block_protocol.received_byte_count;
		cbus_context.hw_block_protocol.received_byte_count = 0;
	}

	pr_debug("rx_unload_ack: %d\n",
			req->payload->hdr_and_burst_id.tport_hdr.rx_unload_ack);
	/* Enable I2C communication with EMSC instead of SPI */
	mhl_pf_modify_reg(REG_PAGE_3_COMMECNT
			, BIT_PAGE_3_COMMECNT_I2C_TO_EMSC_EN
			, BIT_PAGE_3_COMMECNT_I2C_TO_EMSC_EN);
	/* eMSC Transmitter Write Port for I2C */
	mhl_pf_write_reg_block(REG_PAGE_3_EMSC_XMIT_WRITE_PORT
						, req->count
						, &req->payload->as_bytes[0]
						);
}

static struct block_req *start_new_block_marshalling_req(void)
{
	struct block_req *req;
	struct list_head *entry;
	union SI_PACK_THIS_STRUCT emsc_payload_t *payload;

	if (list_empty(&cbus_context.block_protocol.free_list)) {
		int i;
		pr_debug("%s: No free block queue entries available\n",
				__func__);
		list_for_each(entry, &cbus_context.block_protocol.queue)
		{
			req = list_entry(entry, struct block_req, link);
		}
		for (i = 0; i < ARRAY_SIZE(cbus_context.block_protocol.req_entries); ++i)
			req = &cbus_context.block_protocol.req_entries[i];

		return NULL;
	}

	entry = cbus_context.block_protocol.free_list.next;
	list_del(entry);
	req = list_entry(entry, struct block_req, link);

	payload = req->payload;
	req->function = __func__;
	req->line = __LINE__;
	req->sequence = cbus_context.block_protocol.sequence++;
	req->sub_payload_size = 0;
	req->space_remaining = sizeof(payload->as_bytes)
				- sizeof(struct SI_PACK_THIS_STRUCT
					standard_transport_header_t);
	cbus_context.block_protocol.marshalling_req = req;
	pr_debug("%s: q %d get:0x%p\n", __func__, req->sequence, req);
	return req;
}

void mhl_tx_push_block_transactions(void)
{
	struct block_req *req;
	struct list_head *entry;
	uint16_t ack_byte_count;

	/*
		Send the requests out, starting with those in the queue
	*/
	ack_byte_count = cbus_context.hw_block_protocol.received_byte_count;
	req = cbus_context.block_protocol.marshalling_req;
	if (NULL == req) {
		pr_debug("%s: wayward pointer\n", __func__);
		return;
	}
	pr_debug("================= sub_payload_size: %d, ack count: %d, queue empty %d\n",
			req->sub_payload_size,
			ack_byte_count,
			list_empty(&cbus_context.block_protocol.queue)
			);
	/* Need to send the unload count even if no other payload.	*/
    /* If there is no payload and 2 or less unload bytes, don't bother --
     * they will be unloaded with the next payload write.
     * This could violate the MHL3 eMSC block transfer protocol requirement
     * to ACK within 50ms, but it can also cause an CK feedback loop
     * between the two peer devices.  If the unload count is larger,
     * go ahead and send it even if it does cause an extra response
     * from the other side.
     */
	if ((ack_byte_count > BLK_STD_HEADER_LENGTH) || req->sub_payload_size) {
		/* don't use queue_block_transaction here */
		list_add_tail(&req->link, &cbus_context.block_protocol.queue);
		cbus_context.block_protocol.marshalling_req = NULL;
	}
	while (!list_empty(&cbus_context.block_protocol.queue)) {
		uint8_t payload_size;
		entry = cbus_context.block_protocol.queue.next;
		req = list_entry(entry, struct block_req, link);
		payload_size = sizeof(req->payload->hdr_and_burst_id.tport_hdr)
					+ req->sub_payload_size;
		if (cbus_context.hw_block_protocol.peer_blk_rx_buffer_avail
			< payload_size) {
			/* not enough space in peer's receive buffer,
				so wait to send until later
			*/
			pr_debug("==== not enough space in peer's receive buffer, send later\n"
			);
			break;
		}
		list_del(entry);
		cbus_context.hw_block_protocol.peer_blk_rx_buffer_avail
			-= payload_size;

		req->payload->hdr_and_burst_id.tport_hdr.length_remaining
			= req->sub_payload_size;
		req->count = payload_size;
		/* The driver layer will fill in the rx_unload_ack field */
		mhl_tx_drv_send_block(req);
		/* return request to free list */
		return_block_queue_entry(req);

	}
	if (NULL == cbus_context.block_protocol.marshalling_req) {
		/* now start a new marshalling request */
		req =  start_new_block_marshalling_req();
		if (NULL == req)
			pr_debug("%s: block free list exhausted!\n", __func__);
	}
}

void *mhl_tx_get_sub_payload_buffer(uint8_t size)
{
	void *buffer;
	struct SI_PACK_THIS_STRUCT block_req *req;
	union SI_PACK_THIS_STRUCT emsc_payload_t *payload;
	req = cbus_context.block_protocol.marshalling_req;
	if (NULL == req) {
		/* this can only happen if we run out of free requests */
		mhl_tx_push_block_transactions();

		/*
		  TODO: Lee - can't call this here, because the first
			thing it does is chewck to see
			if dev_context->block_protocol.marshalling_req == NULL,
			which we know it is, and if it finds NULL,
			it prints an error and returns.  So why bother?
		*/

		req = cbus_context.block_protocol.marshalling_req;
		if (NULL == req) {
			pr_debug("%s: block free list exhausted!\n", __func__);
			return NULL;
		}
	}
	/* check to see if there is enough space in the marshalling request */
	if (size > req->space_remaining) {
		/*
		  TODO: Lee - this assumes that (size < 254), if it is not,
			the code after this 'if' will fail dramatically
		*/
		/* not enough space, so queue the current req */

		list_add_tail(&req->link, &cbus_context.block_protocol.queue);

		/* try to send immediately, if possible */
		mhl_tx_push_block_transactions();

		/* now start a new marshalling request */
		req =  start_new_block_marshalling_req();
		if (NULL == req) {
			pr_debug("%s: block free list exhausted!\n", __func__);
			return NULL;
		}
	}
	payload = req->payload;
	buffer = &payload->as_bytes[sizeof(payload->as_bytes)
						- req->space_remaining];
	req->space_remaining -= size;
	req->sub_payload_size += size;
	return buffer;
}

uint16_t mhl_tx_drv_get_blk_rcv_buf_size(void)
{
	return 288;
}

/*
 * Enable interrupts and turn on the engine
 *
 * Clearly this must be moved to si_8620_drv.c because all
 * the definitions I need are there.
 */
void mhl_tx_drv_enable_emsc_block(void)
{
	uint8_t	intStatus;
	pr_debug("%s: Enabling EMSC and EMSC interrupts\n", __func__);

	mhl_pf_modify_reg(REG_PAGE_3_GENCTL,
		BIT_PAGE_3_GENCTL_EMSC_EN |
		BIT_PAGE_3_GENCTL_CLR_EMSC_RFIFO |
		BIT_PAGE_3_GENCTL_CLR_EMSC_XFIFO,
		BIT_PAGE_3_GENCTL_EMSC_EN |
		BIT_PAGE_3_GENCTL_CLR_EMSC_RFIFO |
		BIT_PAGE_3_GENCTL_CLR_EMSC_XFIFO);
	mhl_pf_modify_reg(REG_PAGE_3_GENCTL,
		BIT_PAGE_3_GENCTL_CLR_EMSC_RFIFO
			| BIT_PAGE_3_GENCTL_CLR_EMSC_XFIFO,
		0);
	mhl_pf_modify_reg(REG_PAGE_3_COMMECNT,
		BIT_PAGE_3_COMMECNT_I2C_TO_EMSC_EN,
		BIT_PAGE_3_COMMECNT_I2C_TO_EMSC_EN);
	intStatus = mhl_pf_read_reg(REG_PAGE_3_EMSCINTR);
	mhl_pf_write_reg(REG_PAGE_3_EMSCINTR, intStatus);
	mhl_pf_write_reg(REG_PAGE_3_EMSCINTRMASK,
		BIT_PAGE_3_EMSCINTR_SPI_DVLD);
}

void mhl_tx_initialize_block_transport(void)
{
	struct SI_PACK_THIS_STRUCT block_req *req;
	uint8_t buffer_size;
	int idx;
	struct block_buffer_info_t  block_buffer_info;
	struct SI_PACK_THIS_STRUCT block_rcv_buffer_info_t *blk_rcv_buf_info;

	cbus_context.hw_block_protocol.received_byte_count = 0;

	mhl_pf_get_block_buffer_info(&block_buffer_info);

	/* section 13.5.7.2.4 */

	cbus_context.hw_block_protocol.peer_blk_rx_buffer_avail
		= EMSC_RCV_BUFFER_DEFAULT;

	INIT_LIST_HEAD(&cbus_context.block_protocol.queue);
	INIT_LIST_HEAD(&cbus_context.block_protocol.free_list);

	/* Place pre-allocated BLOCK queue entries on the free list */
	for (idx = 0; idx < NUM_BLOCK_QUEUE_REQUESTS; idx++) {

		req = &cbus_context.block_protocol.req_entries[idx];
		memset(req, 0, sizeof(*req));
		req->platform_header = block_buffer_info.buffer
					+ block_buffer_info.req_size*idx;
		req->payload = (union SI_PACK_THIS_STRUCT emsc_payload_t *)
				(req->platform_header
					+ block_buffer_info.payload_offset);
		list_add(&req->link, &cbus_context.block_protocol.free_list);
	}
	buffer_size = sizeof(struct SI_PACK_THIS_STRUCT
				standard_transport_header_t)
				+sizeof(struct SI_PACK_THIS_STRUCT
				block_rcv_buffer_info_t);
	if (buffer_size != 6)
		pr_debug("%s: check structure packing\n", __func__);

	/* we just initialized the free list, this call cannot fail */
	req = start_new_block_marshalling_req();

	blk_rcv_buf_info
		= mhl_tx_get_sub_payload_buffer(sizeof(*blk_rcv_buf_info));
	if (NULL == blk_rcv_buf_info) {
		pr_debug("%s: mhl_tx_get_sub_payload_buffer failed\n",
			__func__);
	} else {
		uint16_t local_rcv_buffer_size;

		blk_rcv_buf_info->burst_id.low
			= (uint8_t)(burst_id_BLK_RCV_BUFFER_INFO & 0xFF);
		blk_rcv_buf_info->burst_id.high
			= (uint8_t)(burst_id_BLK_RCV_BUFFER_INFO >> 8);

		local_rcv_buffer_size = mhl_tx_drv_get_blk_rcv_buf_size();
		blk_rcv_buf_info->blk_rcv_buffer_size_low
			= (uint8_t)(local_rcv_buffer_size & 0xFF);
		blk_rcv_buf_info->blk_rcv_buffer_size_high
			= (uint8_t)(local_rcv_buffer_size >> 8);
		pr_debug("blk_rcv_buffer_info: id:0x%02x%02x size: 0x%02x%02x\n",
				blk_rcv_buf_info->burst_id.high,
				blk_rcv_buf_info->burst_id.low,
				blk_rcv_buf_info->blk_rcv_buffer_size_high,
				blk_rcv_buf_info->blk_rcv_buffer_size_low);
	}

	mhl_tx_drv_enable_emsc_block();
}

/*
  call this from mhl_supp.c to during processing
  of DRV_INTR_FLAG_EMSC_INCOMING
*/
int mhl_tx_drv_peek_block_input_buffer(uint8_t **buffer, int *length)
{
	uint16_t head, tail, index;
	head = cbus_context.hw_block_protocol.head;
	tail = cbus_context.hw_block_protocol.tail;
	if (head == tail)
		return MHL_FAIL;

	index = head;

	*buffer = &cbus_context.hw_block_protocol.input_buffers[index][0];
	*length = cbus_context.hw_block_protocol.input_buffer_lengths[index];
	return 0;
}

/*
  call this from mhl_supp.c to during processing
  of DRV_INTR_FLAG_EMSC_INCOMING
*/
void mhl_tx_drv_free_block_input_buffer(void)
{
	uint16_t head;
	head = cbus_context.hw_block_protocol.head;
	if (++head >= NUM_BLOCK_INPUT_BUFFERS)
		head = 0;

	cbus_context.hw_block_protocol.head = head;
}

/*
 * Called from the Titan interrupt handler to parse data
 * received from the eSMC BLOCK hardware. If multiple messages are
 * sent in the same BLOCK Command buffer, they are sorted out here also.
 */
void mhl_tx_emsc_received(void)
{
	int ret, length, index;
	uint16_t burst_id;
	uint8_t *prbuf, *pmsg;

	ret = mhl_tx_drv_peek_block_input_buffer(&prbuf, &length);
	if (ret == 0) {
		index = 0;
		do {
			pmsg = &prbuf[index];
			burst_id = (((uint16_t)pmsg[0]) << 8) | pmsg[1];
			index += 2;	/* Move past the BURST ID	*/

			switch (burst_id) {
			case burst_id_HID_PAYLOAD:
				pr_debug("%s: burst_id_HID_PAYLOAD\n",
					 __func__);
				/* Include the payload length */
				index += (1 + pmsg[2]);
				break;

			case burst_id_BLK_RCV_BUFFER_INFO:
				cbus_context.hw_block_protocol.peer_blk_rx_buffer_avail
					= pmsg[3];
				cbus_context.hw_block_protocol.peer_blk_rx_buffer_avail
					<<= 8;
				cbus_context.hw_block_protocol.peer_blk_rx_buffer_avail
					|= pmsg[2];
				index += 2;
				break;

			case burst_id_BITS_PER_PIXEL_FMT:
				/*
				  TODO: Lee - What do we do with this data?
				  TODO: Lee - Use #define values
				*/
				index += (4 + (2 * pmsg[5]));
				break;
			default:
				pr_debug("Unsupported BURST ID: %04X\n",
					burst_id);
				index = length;
				break;
			}

			length -= index;
		} while (length > 0);
		mhl_tx_drv_free_block_input_buffer();
	}
}
