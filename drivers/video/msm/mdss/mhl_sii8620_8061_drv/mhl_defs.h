/* kernel/drivers/video/msm/mdss/mhl_sii8620_8061_drv/mhl_defs.h
 *
 * Copyright (C) 2013 Sony Mobile Communications AB.
 * Copyright (C) 2013 Silicon Image Inc.
 *
 * Author: [Yasuyuki Kino <yasuyuki.kino@sonymobile.com>]
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef __MHL_DEFS_H__
#define __MHL_DEFS_H__

#define SI_PUSH_STRUCT_PACKING
#define SI_POP_STRUCT_PACKING
#define SI_PACK_THIS_STRUCT	__attribute__((__packed__))

enum {
	DEVCAP_OFFSET_DEV_STATE			= 0x00
	,DEVCAP_OFFSET_MHL_VERSION		= 0x01
	,DEVCAP_OFFSET_DEV_CAT			= 0x02
	,DEVCAP_OFFSET_ADOPTER_ID_H		= 0x03
	,DEVCAP_OFFSET_ADOPTER_ID_L		= 0x04
	,DEVCAP_OFFSET_VID_LINK_MODE	= 0x05
	,DEVCAP_OFFSET_AUD_LINK_MODE	= 0x06
	,DEVCAP_OFFSET_VIDEO_TYPE		= 0x07
	,DEVCAP_OFFSET_LOG_DEV_MAP		= 0x08
	,DEVCAP_OFFSET_BANDWIDTH		= 0x09
	,DEVCAP_OFFSET_FEATURE_FLAG		= 0x0A
	,DEVCAP_OFFSET_DEVICE_ID_H		= 0x0B
	,DEVCAP_OFFSET_DEVICE_ID_L		= 0x0C
	,DEVCAP_OFFSET_SCRATCHPAD_SIZE	= 0x0D
	,DEVCAP_OFFSET_INT_STAT_SIZE	= 0x0E
	,DEVCAP_OFFSET_RESERVED			= 0x0F
	/* this one must be last */
	,DEVCAP_SIZE
};

SI_PUSH_STRUCT_PACKING //(
typedef struct SI_PACK_THIS_STRUCT _MHLDevCap_t {
	uint8_t state;
	uint8_t mhl_version;
	uint8_t deviceCategory;
	uint8_t adopterIdHigh;
	uint8_t adopterIdLow;
	uint8_t vid_link_mode;
	uint8_t audLinkMode;
	uint8_t videoType;
	uint8_t logicalDeviceMap;
	uint8_t bandWidth;
	uint8_t featureFlag;
	uint8_t deviceIdHigh;
	uint8_t deviceIdLow;
	uint8_t scratchPadSize;
	uint8_t int_state_size;
	uint8_t reserved;
} MHLDevCap_t, *PMHLDevCap_t;

typedef union {
	MHLDevCap_t mdc;
	uint8_t devcap_cache[DEVCAP_SIZE];
} MHLDevCap_u, *PMHLDevCap_u;

/* Device Power State */
#define MHL_DEV_UNPOWERED				0x00
#define MHL_DEV_INACTIVE				0x01
#define MHL_DEV_QUIET					0x03
#define MHL_DEV_ACTIVE					0x04

/* Version that this chip supports */
#define MHL_VER_MAJOR						0x30
#define MHL_VER_MINOR						0x00
#define MHL_VERSION							(MHL_VER_MAJOR | MHL_VER_MINOR)

/* MHL Version mask */
#define MHL_VER_MASK_MAJOR					0xF0
#define MHL_VER_MASK_MINOR					0x0F

/* MHL Version */
#define MHL_DEV_MHL_VER_10					0x10
#define MHL_DEV_MHL_VER_20					0x20
#define MHL_DEV_MHL_VER_30					0x30

/* Device Category mask */
#define MHL_DEV_CATEGORY_MASK_DEV_TYPE		0x0F
#define MHL_DEV_CATEGORY_MASK_POW			0x10
#define MHL_DEV_CATEGORY_MASK_PLIM			0xE0

/* Device Category */
#define MHL_DEV_CATEGORY_OFFSET				DEVCAP_OFFSET_DEV_CAT
#define MHL_DEV_CATEGORY_POW_BIT			0x10
#define MHL_DEV_CATEGORY_PLIM2_0			0xE0

#define MHL_DEV_CAT_SINK					0x01
#define MHL_DEV_CAT_SOURCE					0x02
#define MHL_DEV_CAT_DONGLE					0x03
#define MHL_DEV_CAT_SELF_POWERED_DONGLE		0x13
#define MHL_DEV_CAT_DIRECT_SINK				0x05
#define MHL_DEV_CAT_DIRECT_SOURCE			0x06

/* Device Category[PLIM] */
#define MHL_DEV_CAT_PLIM_500				0x00
#define MHL_DEV_CAT_PLIM_900				0x20
#define MHL_DEV_CAT_PLIM_1500				0x40
#define MHL_DEV_CAT_PLIM_100				0x60
#define MHL_DEV_CAT_PLIM_2000				0x80

/* Video Link Mode */
#define MHL_DEV_VID_LINK_SUPPRGB444		0x01
#define MHL_DEV_VID_LINK_SUPPYCBCR444		0x02
#define MHL_DEV_VID_LINK_SUPPYCBCR422		0x04
#define MHL_DEV_VID_LINK_SUPP_PPIXEL		0x08
#define MHL_DEV_VID_LINK_SUPP_ISLANDS		0x10
#define MHL_DEV_VID_LINK_SUPP_VGA		0x20
#define MHL_DEV_VID_LINK_SUPP_16BPP		0x40

/* Audio Link Mode Support */
#define MHL_DEV_AUD_LINK_2CH				0x01
#define MHL_DEV_AUD_LINK_8CH				0x02


/* Feature Flag in the devcap */
#define MHL_DEV_FEATURE_FLAG_OFFSET			DEVCAP_OFFSET_FEATURE_FLAG
#define MHL_FEATURE_RCP_SUPPORT				0x01
#define MHL_FEATURE_RAP_SUPPORT				0x02
#define MHL_FEATURE_SP_SUPPORT				0x04
#define MHL_FEATURE_UCP_SEND_SUPPORT		0x08
#define MHL_FEATURE_UCP_RECV_SUPPORT		0x10

/* VIDEO TYPES */
#define MHL_VT_GRAPHICS						0x00
#define MHL_VT_PHOTO						0x02
#define MHL_VT_CINEMA						0x04
#define MHL_VT_GAMES						0x08
#define MHL_SUPP_VT							0x80

/* Logical Dev Map */
#define MHL_DEV_LD_DISPLAY					0x01
#define MHL_DEV_LD_VIDEO					0x02
#define MHL_DEV_LD_AUDIO					0x04
#define MHL_DEV_LD_MEDIA					0x08
#define MHL_DEV_LD_TUNER					0x10
#define MHL_DEV_LD_RECORD					0x20
#define MHL_DEV_LD_SPEAKER					0x40
#define MHL_DEV_LD_GUI						0x80

/* Bandwidth */
#define MHL_BANDWIDTH_LIMIT					22		// 225 MHz


#define MHL_STATUS_REG_CONNECTED_RDY		0x30
#define MHL_STATUS_REG_LINK_MODE			0x31
#define MHL_STATUS_REG_VERSION_STAT			0x32

#define MHL_STATUS_REG_AV_LINK_MODE_CONTROL	0x92

#define MHL_STATUS_DCAP_RDY					0x01
#define MHL_STATUS_XDEVCAPP_SUPP			0x02
#define MHL_STATUS_POW_STAT					0x04
#define MHL_STATUS_PLIM_STAT_MASK			0x38

#define MHL_STATUS_CLK_MODE_MASK			0x07
#define MHL_STATUS_CLK_MODE_PACKED_PIXEL	0x02
#define MHL_STATUS_CLK_MODE_NORMAL			0x03
#define MHL_STATUS_PATH_EN_MASK				0x08
#define MHL_STATUS_PATH_ENABLED				0x08
#define MHL_STATUS_PATH_DISABLED			0x00
#define MHL_STATUS_MUTED_MASK				0x10

#define MHL_RCHANGE_INT						0x20
#define MHL_DCHANGE_INT						0x21

#define	MHL_INT_DCAP_CHG					0x01
#define MHL_INT_DSCR_CHG					0x02
#define MHL_INT_REQ_WRT						0x04
#define MHL_INT_GRT_WRT						0x08
#define MHL2_INT_3D_REQ						0x10
#define MHL3_INT_FEAT_REQ					0x20
#define MHL3_INT_FEAT_COMPLETE				0x40

// On INTR_1 the EDID_CHG is located at BIT 0
#define MHL_INT_EDID_CHG					0x02

#define MHL_INT_AND_STATUS_SIZE				0x33	// This contains one nibble each - max offset
#define MHL_SCRATCHPAD_SIZE					16
#define MHL_MAX_BUFFER_SIZE					MHL_SCRATCHPAD_SIZE	// manually define highest number

#define SILICON_IMAGE_ADOPTER_ID			322

typedef enum {
	MHL_TEST_ADOPTER_ID = 0
	,burst_id_3D_VIC = 0x0010
	,burst_id_3D_DTD = 0x0011
	,burst_id_HEV_VIC = 0x0020
	,burst_id_HEV_DTDA = 0x0021
	,burst_id_HEV_DTDB = 0x0022
	,burst_id_VC_ASSIGN = 0x0038
	,burst_id_VC_CONFIRM = 0x0039
	,burst_id_AUD_DELAY = 0x0040
	,burst_id_ADT_BURSTID = 0x0041
	,burst_id_BIST_SETUP = 0x0051
	,burst_id_BIST_RETURN_STAT = 0x0052
	,burst_id_EMSC_SUPPORT			= 0x0061
	,burst_id_HID_PAYLOAD			= 0x0062
	,burst_id_BLK_RCV_BUFFER_INFO	= 0x0063
	,burst_id_BITS_PER_PIXEL_FMT	= 0x0064
	,LOCAL_ADOPTER_ID = SILICON_IMAGE_ADOPTER_ID

	// add new burst ID's above here

	/* Burst ID's are a 16-bit big-endian quantity.
	 * In order for the BURST_ID macro below to allow detection of
            out-of-range values with KEIL 8051 compiler
            we must have at least one enumerated value
            that has one of the bits in the high order byte set.
        Experimentally, we have found that the KEIL 8051 compiler
         treats 0xFFFF as a special case (-1 perhaps...),
         so we use a different value that has some upper bits set
    */
    ,burst_id_16_BITS_REQUIRED = 0x8000
} BurstId_e;

typedef struct SI_PACK_THIS_STRUCT _MHL_high_low_t{
	uint8_t high;
	uint8_t low;
} MHL_high_low_t, *PMHL_high_low_t;

typedef MHL_high_low_t MHL_burst_id_t,*PMHL_burst_id_t;

#define ENDIAN_CONVERT_16(a) ((((uint16_t)(a.high))<<8)|((uint16_t)(a.low)))
#define BURST_ID(bid) (BurstId_e)(ENDIAN_CONVERT_16(bid))

#define HIGH_BYTE_16(x) (uint8_t)((x >> 8) & 0xFF)
#define LOW_BYTE_16(x)  (uint8_t)( x       & 0xFF)
#define ENCODE_BURST_ID(id) {HIGH_BYTE_16(id),LOW_BYTE_16(id)}

struct SI_PACK_THIS_STRUCT standard_transport_header_t{
	uint8_t	rx_unload_ack;
	uint8_t	length_remaining;
};

#define STD_TRANSPORT_HDR_SIZE sizeof(struct SI_PACK_THIS_STRUCT standard_transport_header_t)

struct SI_PACK_THIS_STRUCT block_rcv_buffer_info_t{
	MHL_burst_id_t	burst_id; /* use the BURST_ID macro to access this */
	uint8_t blk_rcv_buffer_size_low;
	uint8_t blk_rcv_buffer_size_high;
};

// see MHL2.0 spec section 5.9.1.2
typedef struct SI_PACK_THIS_STRUCT _MHL2_video_descriptor_t {
	uint8_t reserved_high;
	unsigned char frame_sequential:1;    //FB_SUPP
	unsigned char top_bottom:1;          //TB_SUPP
	unsigned char left_right:1;          //LR_SUPP
	unsigned char reserved_low:5;
} MHL2_video_descriptor_t, *PMHL2_video_descriptor_t;

typedef struct SI_PACK_THIS_STRUCT _MHL3_burst_header_t{
	MHL_burst_id_t burst_id;
	uint8_t checksum;
	uint8_t total_entries;
	uint8_t sequence_index;
}MHL3_burst_header_t,*P_MHL3_burst_header_t;
typedef struct SI_PACK_THIS_STRUCT _MHL2_video_format_data_t {
	MHL3_burst_header_t	header;
	uint8_t num_entries_this_burst;
	MHL2_video_descriptor_t video_descriptors[5];
} MHL2_video_format_data_t, *PMHL2_video_format_data_t;

typedef struct SI_PACK_THIS_STRUCT _MHL3_hev_vic_descriptor_t {
	uint8_t	vic_cea861f;
	uint8_t reserved;
}MHL3_hev_vic_descriptor_t,*PMHL3_hev_vic_descriptor_t;

typedef struct SI_PACK_THIS_STRUCT _MHL3_hev_vic_data_t {
	MHL3_burst_header_t	header;
	uint8_t num_entries_this_burst;
	MHL3_hev_vic_descriptor_t video_descriptors[5];
} MHL3_hev_vic_data_t, *PMHL3_hev_vic_data_t;

typedef struct SI_PACK_THIS_STRUCT _MHL3_hev_dtd_a_payload_t {
	MHL_high_low_t	pixel_clock_in_MHz;
	MHL_high_low_t	h_active_in_pixels;
	MHL_high_low_t	h_blank_in_pixels;
	MHL_high_low_t	h_front_porch_in_pixels;
	MHL_high_low_t	h_sync_width_in_pixels;
	uint8_t			h_flags;
} MHL3_hev_dtd_a_payload_t , *PMHL3_hev_dtd_a_payload_t ;

typedef struct SI_PACK_THIS_STRUCT _MHL3_hev_dtd_b_payload_t {
	MHL_high_low_t	v_total_in_lines;
	uint8_t			v_blank_in_lines; /* note 7 for table 5-16 is wrong for this entry */
	uint8_t			v_front_porch_in_lines; /* note 7 for table 5-16 is wrong for this entry */
	uint8_t			v_sync_width_in_lines; /* note 7 for table 5-16 is wrong for this entry */
	uint8_t			v_refresh_rate_in_fields_per_second;
	uint8_t			v_flags;
	uint8_t			reserved[4];
} MHL3_hev_dtd_b_payload_t , *PMHL3_hev_dtd_b_payload_t ;

typedef struct SI_PACK_THIS_STRUCT _MHL3_hev_dtd_a_data_t {
	MHL3_burst_header_t	header;
	MHL3_hev_dtd_a_payload_t	payload;
} MHL3_hev_dtd_a_data_t, *PMHL3_hev_dtd_a_data_t;

typedef struct SI_PACK_THIS_STRUCT _MHL3_hev_dtd_b_data_t {
	MHL3_burst_header_t	header;
	MHL3_hev_dtd_b_payload_t payload;
} MHL3_hev_dtd_b_data_t, *PMHL3_hev_dtd_b_data_t;

typedef struct SI_PACK_THIS_STRUCT _MHL3_hev_dtd_item_t {
	MHL3_hev_dtd_a_payload_t a;
	MHL3_hev_dtd_b_payload_t b;
}MHL3_hev_dtd_item_t, *PMHL3_hev_dtd_item_t;

typedef struct SI_PACK_THIS_STRUCT  _MHL3_speaker_allocation_data_block_t{
	uint8_t cea861f_spkr_alloc[3];
}MHL3_speaker_allocation_data_block_t,*PMHL3_speaker_allocation_data_block_t;

typedef struct SI_PACK_THIS_STRUCT _MHL3_adt_payload_t{
	uint8_t	format_flags;
	union{
		uint8_t	short_descs[9];
		MHL3_speaker_allocation_data_block_t spkr_alloc_db[3];
	}descriptors;
	uint8_t reserved;
}MHL3_adt_payload_t,*PMHL3_adt_payload_t;

typedef struct SI_PACK_THIS_STRUCT  _MHL3_adt_data_t{
	MHL3_burst_header_t	header;
	MHL3_adt_payload_t	payload;
}MHL3_adt_data_t,*PMHL3_adt_data_t;

typedef struct SI_PACK_THIS_STRUCT _MHL3_emsc_support_payload_t{
	 MHL_burst_id_t burst_ids[5];
}MHL3_emsc_support_payload_t,*PMHL3_emsc_support_payload_t;

typedef struct SI_PACK_THIS_STRUCT _MHL3_emsc_support_data_t {
	MHL3_burst_header_t	header;
	uint8_t num_entries_this_burst;
	MHL3_emsc_support_payload_t payload;
}MHL3_emsc_support_data_t, *PMHL3_emsc_support_data_t ;

enum view_pixel_fmt_e{
	VIEW_PIX_FMT_24BPP = 0
	,VIEW_PIX_FMT_16BPP =1
};
typedef struct SI_PACK_THIS_STRUCT _MHL3_bits_per_pixel_fmt_descriptor_t{
	uint8_t	stream_id;
	uint8_t	stream_pixel_format;
}MHL3_bits_per_pixel_fmt_descriptor_t,*PMHL3_bits_per_pixel_fmt_descriptor_t;

typedef struct SI_PACK_THIS_STRUCT _MHL_bits_per_pixel_fmt_data_t{
	MHL3_burst_header_t	header;
	uint8_t num_entries_this_burst;

	/* reserve 5 for use with WRITE_BURST 
		actual length is variable, indicated by
		num_entries_this_burst
	*/
	MHL3_bits_per_pixel_fmt_descriptor_t descriptors[5]; 

}MHL_bits_per_pixel_fmt_data_t,*PMHL_bits_per_pixel_fmt_data_t;

typedef union SI_PACK_THIS_STRUCT {
	MHL2_video_descriptor_t		mhl2_3d_descriptor;
	MHL3_hev_vic_descriptor_t	mhl3_hev_vic_descriptor;
	MHL3_hev_dtd_item_t			mhl3_hev_dtd;
}video_burst_descriptor_u;

SI_POP_STRUCT_PACKING //)

enum {
	MHL_MSC_MSG_RCP				= 0x10,	/* RCP sub-command */
	MHL_MSC_MSG_RCPK			= 0x11,	/* RCP Acknowledge sub-command */
	MHL_MSC_MSG_RCPE			= 0x12,	/* RCP Error sub-command */
	MHL_MSC_MSG_RAP				= 0x20,	/* Mode Change Warning sub-command */
	MHL_MSC_MSG_RAPK			= 0x21,	/* MCW Acknowledge sub-command */
	MHL_MSC_MSG_RBP				= 0x22,	/* Remote Button Protocol sub-command */
	MHL_MSC_MSG_RBPK			= 0x23,	/* RBP Acknowledge sub-command */
	MHL_MSC_MSG_RBPE			= 0x24,	/* RBP Error sub-command */
	MHL_MSC_MSG_UCP				= 0x30,	/* UCP sub-command */
	MHL_MSC_MSG_UCPK			= 0x31,	/* UCP Acknowledge sub-command */
	MHL_MSC_MSG_UCPE			= 0x32,	/* UCP Error sub-command */
	MHL_MSC_MSG_RUSB			= 0x40,	/* Request USB host role */
	MHL_MSC_MSG_RUSBK			= 0x41,	/* Acknowledge request for USB host role */
	MHL_MSC_MSG_RHID			= 0x42,	/* Request HID host role */
	MHL_MSC_MSG_RHIDK			= 0x43,	/* Acknowledge request for HID host role */
	MHL_MSC_MSG_ATT				= 0x50,	/* Request attention sub-command */
	MHL_MSC_MSG_ATTK			= 0x51,	/* ATT Acknowledge sub-command */
	MHL_MSC_MSG_BIST_TRIGGER		= 0x60,
	MHL_MSC_MSG_BIST_REQUEST_STAT	= 0x61,
	MHL_MSC_MSG_BIST_READY			= 0x62,
	MHL_MSC_MSG_BIST_STOP			= 0x63,
};

#define BIST_TRIGGER_E_CBUS_TX			0x01
#define BIST_TRIGGER_E_CBUS_RX			0x02
#define BIST_TRIGGER_E_CBUS_LOOPBACK	0x04
#define BIST_TRIGGER_E_CBUS_TYPE_MASK	0x08
#define BIST_TRIGGER_TEST_E_CBUS_S		0x00
#define BIST_TRIGGER_TEST_E_CBUS_D		0x08
#define BIST_TRIGGER_AVLINK_TX			0x10
#define BIST_TRIGGER_AVLINK_RX			0x20
#define BIST_TRIGGER_IMPEDANCE_TEST		0x40


#define BIST_READY_E_CBUS_READY		0x01
#define BIST_READY_AVLINK_READY		0x02
#define BIST_READY_TERM_READY		0x04
#define BIST_READY_E_CBUS_ERROR		0x10
#define BIST_READY_AVLINK_ERROR		0x20
#define BIST_READY_TERM_ERROR		0x40

#define	RCPE_NO_ERROR				0x00
#define	RCPE_INEEFECTIVE_KEY_CODE	0x01
#define	RCPE_BUSY					0x02

#define MHL_RCP_KEY_RELEASED_MASK	0x80
#define MHL_RCP_KEY_ID_MASK			0x7F

#define T_PRESS_MODE				300

#define T_HOLD_MAINTAIN				2000
#define T_RAP_WAIT_MIN				100
#define T_RAP_WAIT_MAX				1000
//
// MHL spec related defines
//
enum {
	 MHL_ACK					= 0x33	// Command or Data byte acknowledge
	,MHL_NACK					= 0x34	// Command or Data byte not acknowledge
	,MHL_ABORT					= 0x35	// Transaction abort
	,MHL_WRITE_STAT				= 0x60 | 0x80	// 0xE0 - Write one status register strip top bit
	,MHL_SET_INT				= 0x60	// Write one interrupt register
	,MHL_READ_DEVCAP_REG		= 0x61	// Read one register
	,MHL_GET_STATE				= 0x62	// Read CBUS revision level from follower
	,MHL_GET_VENDOR_ID			= 0x63	// Read vendor ID value from follower.
	,MHL_SET_HPD				= 0x64	// Set Hot Plug Detect in follower
	,MHL_CLR_HPD				= 0x65	// Clear Hot Plug Detect in follower
	,MHL_SET_CAP_ID				= 0x66	// Set Capture ID for downstream device.
	,MHL_GET_CAP_ID				= 0x67	// Get Capture ID from downstream device.
	,MHL_MSC_MSG				= 0x68	// VS command to send RCP sub-commands
	,MHL_GET_SC1_ERRORCODE		= 0x69	// Get Vendor-Specific command error code.
	,MHL_GET_DDC_ERRORCODE		= 0x6A	// Get DDC channel command error code.
	,MHL_GET_MSC_ERRORCODE		= 0x6B	// Get MSC command error code.
	,MHL_WRITE_BURST			= 0x6C	// Write 1-16 bytes to responder's scratchpad.
	,MHL_GET_SC3_ERRORCODE		= 0x6D	// Get channel 3 command error code.
	,MHL_WRITE_XSTAT			= 0x70 /* Write one extended status register. */
	,MHL_READ_XDEVCAP_REG		= 0x71 /* Read one extended devcap register. */

	,MHL_READ_EDID_BLOCK				/* let the rest of these float, they are software specific */
	,MHL_SEND_3D_REQ_OR_FEAT_REQ
	,MHL_READ_DEVCAP
	,MHL_READ_XDEVCAP
};

/* RAP action codes */
#define MHL_RAP_POLL				0x00	// Just do an ack
#define MHL_RAP_CONTENT_ON			0x10	// Turn content streaming ON.
#define MHL_RAP_CONTENT_OFF			0x11	// Turn content streaming OFF.
#define MHL_RAP_CBUS_MODE_DOWN		0x20
#define MHL_RAP_CBUS_MODE_UP		0x21

/* RAPK status codes */
#define MHL_RAPK_NO_ERR				0x00	/* RAP action recognized & supported */
#define MHL_RAPK_UNRECOGNIZED		0x01	/* Unknown RAP action code received */
#define MHL_RAPK_UNSUPPORTED		0x02	/* Received RAP action code is not supported */
#define MHL_RAPK_BUSY				0x03	/* Responder too busy to respond */

/*
 * Error status codes for RCPE messages
 */
/* No error. (Not allowed in RCPE messages) */
#define MHL_RCPE_STATUS_NO_ERROR		0x00
/* Unsupported/unrecognized key code */
#define MHL_RCPE_STATUS_INEFFECTIVE_KEY_CODE	0x01
/* Responder busy. Initiator may retry message */
#define MHL_RCPE_STATUS_BUSY			0x02


/* Extended Device Capability Registers 7.12.1 */
enum {
	XDEVCAP_ADDR_ECBUS_SPEEDS			= 0x80
	,XDEVCAP_ADDR_TMDS_SPEEDS			= 0x81
	,XDEVCAP_ADDR_ECBUS_DEV_ROLES	                = 0x82
	,XDEVCAP_ADDR_ECBUS_DEV_MAPX		        = 0x83
	,XDEVCAP_LIMIT  /* don't hard-code this one */
	,XDEVCAP_ADDR_RESERVED_4			= 0x84
	,XDEVCAP_ADDR_RESERVED_5			= 0x85
	,XDEVCAP_ADDR_RESERVED_6			= 0x86
	,XDEVCAP_ADDR_RESERVED_7			= 0x87
	,XDEVCAP_ADDR_RESERVED_8			= 0x88
	,XDEVCAP_ADDR_RESERVED_9			= 0x89
	,XDEVCAP_ADDR_RESERVED_A			= 0x8A
	,XDEVCAP_ADDR_RESERVED_B			= 0x8B
	,XDEVCAP_ADDR_RESERVED_C			= 0x8C
	,XDEVCAP_ADDR_RESERVED_D			= 0x8D
	,XDEVCAP_ADDR_RESERVED_E			= 0x8E
	,XDEVCAP_ADDR_RESERVED_F			= 0x8F
	/* this one must be last */
	,XDEVCAP_ADDR_LAST
	,XDEVCAP_START	= XDEVCAP_ADDR_ECBUS_SPEEDS
};

#define XDEVCAP_OFFSET(reg) (reg - XDEVCAP_ADDR_ECBUS_SPEEDS)
SI_PUSH_STRUCT_PACKING //(
typedef struct SI_PACK_THIS_STRUCT _MHLXDevCap_t {
	uint8_t ecbus_speeds;
	uint8_t tmds_speeds;
	uint8_t ecbus_dev_roles;
	uint8_t log_dev_mapx;
	uint8_t reserved_4;
	uint8_t reserved_5;
	uint8_t reserved_6;
	uint8_t reserved_7;
	uint8_t reserved_8;
	uint8_t reserved_9;
	uint8_t reserved_a;
	uint8_t reserved_b;
	uint8_t reserved_c;
	uint8_t reserved_d;
	uint8_t reserved_e;
	uint8_t reserved_f;
} MHLXDevCap_t,*PMHLXDevCap_t;

typedef union {
	MHLXDevCap_t mxdc;
	uint8_t xdevcap_cache[XDEVCAP_ADDR_LAST];
} MHLXDevCap_u, *PMHLXDevCap_u;
SI_POP_STRUCT_PACKING //)

/* XDEVCAP - eCBUS Speeds 7.12.1.1 */
#define MHL_XDC_ECBUS_S_075				0x01
#define MHL_XDC_ECBUS_S_8BIT			0x02
#define MHL_XDC_ECBUS_S_12BIT			0x04
#define MHL_XDC_ECBUS_D_150				0x10
#define MHL_XDC_ECBUS_D_8BIT			0x20

/* XDEVCAP - TMDS Speeds 7.12.1.2 */
#define MHL_XDC_TMDS_000				0x00
#define MHL_XDC_TMDS_150				0x01
#define MHL_XDC_TMDS_300				0x02
#define MHL_XDC_TMDS_600				0x04

/* XDEVCAP - Device Roles 7.12.1.3?       */
#define MHL_XDC_DEV_HOST				0x01
#define MHL_XDC_DEV_DEVICE				0x02
#define MHL_XDC_DEV_CHARGER				0x04
#define MHL_XDC_HID_HOST				0x08
#define MHL_XDC_HID_DEVICE				0x10

/* XDEVCAP - Extended Logical Device Map 7.12.1.4 */
#define MHL_XDC_LD_PHONE				0x01

/* Extended Device Status Registers 18.2.2 */
enum {
	XDEVSTAT_OFFSET_CURR_ECBUS_MODE		= 0x00,
	XDEVSTAT_OFFSET_AVLINK_MODE_STATUS	= 0x01,
	XDEVSTAT_OFFSET_AVLINK_MODE_CONTROL	= 0x02,
	XDEVSTAT_OFFSET_MULTI_SINK_STATUS	= 0x03,
	XDEVSTAT_OFFSET_RESERVED_04			= 0x04,
	XDEVSTAT_OFFSET_RESERVED_05			= 0x05,
	XDEVSTAT_OFFSET_RESERVED_06			= 0x06,
	XDEVSTAT_OFFSET_RESERVED_07			= 0x07,
	XDEVSTAT_OFFSET_RESERVED_08			= 0x08,
	XDEVSTAT_OFFSET_RESERVED_09			= 0x09,
	XDEVSTAT_OFFSET_RESERVED_0A			= 0x0A,
	XDEVSTAT_OFFSET_RESERVED_0B			= 0x0B,
	XDEVSTAT_OFFSET_RESERVED_0C			= 0x0C,
	XDEVSTAT_OFFSET_RESERVED_0D			= 0x0D,
	XDEVSTAT_OFFSET_RESERVED_0E			= 0x0E,
	XDEVSTAT_OFFSET_RESERVED_0F			= 0x0F,
	XDEVSTAT_OFFSET_RESERVED_10			= 0x10,
	XDEVSTAT_OFFSET_RESERVED_11			= 0x11,
	XDEVSTAT_OFFSET_RESERVED_12			= 0x12,
	XDEVSTAT_OFFSET_RESERVED_13			= 0x13,
	XDEVSTAT_OFFSET_RESERVED_14			= 0x14,
	XDEVSTAT_OFFSET_RESERVED_15			= 0x15,
	XDEVSTAT_OFFSET_RESERVED_16			= 0x16,
	XDEVSTAT_OFFSET_RESERVED_17			= 0x17,
	XDEVSTAT_OFFSET_RESERVED_18			= 0x18,
	XDEVSTAT_OFFSET_RESERVED_19			= 0x19,
	XDEVSTAT_OFFSET_RESERVED_1A			= 0x1A,
	XDEVSTAT_OFFSET_RESERVED_1B			= 0x1B,
	XDEVSTAT_OFFSET_RESERVED_1C			= 0x1C,
	XDEVSTAT_OFFSET_RESERVED_1D			= 0x1D,
	XDEVSTAT_OFFSET_RESERVED_1E			= 0x1E,
	XDEVSTAT_OFFSET_RESERVED_1F			= 0x1F,
	/* this one must be last */
	XDEVSTAT_SIZE
};

/* XDEVSTAT - Current eCBUS Mode 18.2.2.1 */
#define MHL_XDS_SLOT_MODE_8BIT				0x00
#define MHL_XDS_SLOT_MODE_6BIT				0x01

#define MHL_XDS_LINK_CLOCK_75MHZ			0x00
#define MHL_XDS_LINK_CLOCK_150MHZ			0x10
#define MHL_XDS_LINK_CLOCK_300MHZ			0x20
#define MHL_XDS_LINK_CLOCK_600MHZ			0x30

/* XDEVSTAT - AV Link Mode Status 18.2.2.2 */
#define MHL_XDS_LINK_STATUS_NO_SIGNAL		0x00
#define MHL_XDS_LINK_STATUS_CRU_LOCKED		0x01
#define MHL_XDS_LINK_STATUS_TMDS_NORMAL		0x02
#define MHL_XDS_LINK_STATUS_TMDS_RESERVED	0x03

/* XDEVSTAT - AV Link Mode Control 18.2.2.3 */
#define MHL_XDS_LINK_RATE_1_5_GBPS			0x00
#define MHL_XDS_LINK_RATE_3_0_GBPS			0x01
#define MHL_XDS_LINK_RATE_6_0_GBPS			0x02

/* XDEVSTAT - Multi-Sink Status 18.2.2.4 */
#define MHL_XDS_SINK_STATUS_1_HPD_LOW		0x00
#define MHL_XDS_SINK_STATUS_1_HPD_HIGH		0x01
#define MHL_XDS_SINK_STATUS_2_HPD_LOW		0x00
#define MHL_XDS_SINK_STATUS_2_HPD_HIGH		0x04
#define MHL_XDS_SINK_STATUS_3_HPD_LOW		0x00
#define MHL_XDS_SINK_STATUS_3_HPD_HIGH		0x10
#define MHL_XDS_SINK_STATUS_4_HPD_LOW		0x00
#define MHL_XDS_SINK_STATUS_4_HPD_HIGH		0x40


/*
 * Define format of Write Burst used in MHL 3
 * to assign TDM slots to virtual channels.
 */
struct SI_PACK_THIS_STRUCT virt_chan_info {
	uint8_t vc_num;
	uint8_t feature_id;
#define FEATURE_ID_E_MSC					0x00
#define FEATURE_ID_USB						0x01
#define FEATURE_ID_AUDIO					0x02
#define FEATURE_ID_IP						0x03
#define FEATURE_ID_COMP_VIDEO				0x04
	union {
		uint8_t channel_size;
		uint8_t response;
#define VC_RESPONSE_ACCEPT					0x00
#define VC_RESPONSE_BAD_VC_NUM				0x01
#define VC_RESPONSE_BAD_FEATURE_ID			0x02
#define VC_RESPONSE_BAD_CHANNEL_SIZE		0x03
	} req_resp;
};

#define MAX_VC_ENTRIES 3
struct SI_PACK_THIS_STRUCT tdm_alloc_burst {
	uint8_t burst_id_h;
	uint8_t burst_id_l;
	uint8_t checksum;
	uint8_t tot_ent;
	uint8_t seq;
	uint8_t num_ent;
	struct virt_chan_info vc_info[MAX_VC_ENTRIES];
	uint8_t reserved;
};

struct __attribute__((__packed__)) bist_setup_burst {
	uint8_t	burst_id_h;
	uint8_t	burst_id_l;
	uint8_t	checksum;
	uint8_t	e_cbus_duration;
	uint8_t	e_cbus_pattern;
#define BS_PATTERN_UNSPECIFIED	0x00
#define BS_PATTERN_PRBS			0x01
#define BS_PATTERN_FIXED_8		0x02
#define BS_PATTERN_FIXED_10		0x03
#define BS_PATTERN_MAX			BS_PATTERN_FIXED_10
	uint8_t	e_cbus_fixed_h;
	uint8_t	e_cbus_fixed_l;
	uint8_t	reserved;
	uint8_t	avlink_data_rate;
	uint8_t	avlink_pattern;
#define BS_AV_PATTERN_UNSPECIFIED	0x00
#define BS_AV_PATTERN_PRBS			0x01
#define BS_AV_PATTERN_FIXED_8		0x02
#define BS_AV_PATTERN_FIXED_10		0x03
#define BS_AV_PATTERN_MAX			BS_AV_PATTERN_FIXED_10
	uint8_t	avlink_video_mode;
	uint8_t	avlink_duration;
	uint8_t	avlink_fixed_h;
	uint8_t	avlink_fixed_l;
	uint8_t	avlink_randomizer;
	uint8_t	impedance_mode;
#define IMP_MODE_AV_LINK_TX_LOW		0x00
#define IMP_MODE_AV_LINK_TX_HIGH	0x01
#define IMP_MODE_AV_LINK_RX			0x02
#define IMP_MODE_E_CBUS_D_TX_LOW	0x04
#define IMP_MODE_E_CBUS_D_TX_HIGH	0x05
#define IMP_MODE_E_CBUS_D_RX		0x06
#define IMP_MODE_E_CBUS_S_TX_LOW	0x08
#define IMP_MODE_E_CBUS_S_TX_HIGH	0x09
#define IMP_MODE_E_CBUS_S_RX		0x0A
};

struct __attribute__((__packed__)) bist_return_status_burst {
	uint8_t	burst_id_h;
	uint8_t	burst_id_l;
	uint8_t	checksum;
	uint8_t reserved[9];
	uint8_t	e_cbus_stat_h;
	uint8_t	e_cbus_stat_l;
	uint8_t	avlink_stat_h;
	uint8_t	avlink_stat_l;
};

#endif
