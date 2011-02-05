/*****************************************************************************
*  Copyright 2001 - 2008 Broadcom Corporation.  All rights reserved.
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2, available at
*  http://www.gnu.org/licenses/old-license/gpl-2.0.html (the "GPL").
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
*****************************************************************************/
/**
*
*  @file  chal_cam.h
*  @brief CAM cHAL interface
* \note
*****************************************************************************/
#ifndef _CHAL_CAM_H_
#define _CHAL_CAM_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup cHAL_Interface 
 * @{
 */

// Maximum Channels Available for Athena
#define CHAL_MAX_CHANNELS           = 2

#define CHAL_MAX_CSI_LANES       = 2
#define CHAL_MAX_CCP_LANES       = 1

// Data Type and Virtual Channel
#define CHAL_CAM_DATA_TYPE_MASK             0x3F
#define CHAL_CAM_DATA_TYPE_SHIFT            0x00
#define CHAL_CAM_VIRTUAL_CHANNEL_MASK       0xE0
#define CHAL_CAM_VIRTUAL_CHANNEL_SHIFT      0x05

/**
* CAM Status Codes
*****************************************************************************/
#ifndef CHAL_CAM_STATUS_CODES
typedef enum {
    CHAL_OP_OK = 0,
    CHAL_OP_FAILED,
    CHAL_OP_INVALID,
    CHAL_OP_INVALID_PARAMETER,
    CHAL_OP_DRIVER_NOT_REGISTERED,
    CHAL_OP_WRONG_ORDER,   
    /* Add more error codes here */
    
    CHAL_OP_MAX = 255
} CHAL_CAM_STATUS_CODES;
#endif

/**
* CAM Interface mode
*****************************************************************************/
typedef enum
{
    CHAL_CAM_INTF_CSI               = (1 << 0),     ///< CSI-2 Serial Interface Mode
    CHAL_CAM_INTF_CCP               = (1 << 1),     ///< CCP-2 Serial Interface Mode
    CHAL_CAM_INTF_CPI               = (1 << 2),     ///< CPI Parallel Interface Mode
} CHAL_CAM_INTF_t;

/**
* CAM Channel Select
*****************************************************************************/
typedef enum
{
    CHAL_CAM_CHAN_0                 = (1 << 0),     ///< Channel 0 select
    CHAL_CAM_CHAN_1                 = (1 << 1),     ///< Channel 1 select
} CHAL_CAM_CHAN_t;


/**
* CAM Lane Select
*****************************************************************************/
typedef enum
{
    CHAL_CAM_DATA_LANE_0            = (1 << 0),     ///< Lane 0 select
    CHAL_CAM_DATA_LANE_1            = (1 << 1),     ///< Lane 1 select
    CHAL_CAM_CLK_LANE               = (1 << 2),     ///< Clock Lane select
} CHAL_CAM_LANE_SELECT_t;

/**
* CAM Config Select
*****************************************************************************/
typedef enum
{
    CHAL_CAM_CLOCK                  = (1 << 0),     ///< Clock select
    CHAL_CAM_IMAGE                  = (1 << 1),     ///< Image select
    CHAL_CAM_DATA                   = (1 << 2),     ///< Data select
    CHAL_CAM_PACKET                 = (1 << 3),     ///< Packet select
    CHAL_CAM_ANALOG                 = (1 << 4),     ///< Analog select
    CHAL_CAM_BANDGAP                = (1 << 5),     ///< Bandgap select
} CHAL_CAM_CFG_SELECT_t;


/**
* CAM Packet Compare Select
*****************************************************************************/
typedef enum
{
    CHAL_CAM_PKT_CMP_0              = (1 << 0),     ///< Select Packet Compare 0
    CHAL_CAM_PKT_CMP_1              = (1 << 1),     ///< Select Packet Compare 1
} CHAL_CAM_PKT_CMP_SELECT_t;

/**
* CAM Shot Capture Mode
*****************************************************************************/
typedef enum
{
    CHAL_CAM_SHOT_MODE_SINGLE       = (1 << 0),     ///< Single Shot Capture Mode
    CHAL_CAM_SHOT_MODE_CONTINOUS    = (1 << 1),     ///< Continuous Shot Capture Mode
} CHAL_CAM_SHOT_MODE_t;

/**
* CAM Input Mode
*****************************************************************************/
typedef enum
{
    CHAL_CAM_INPUT_SINGLE_LANE      = (1 << 0),     ///< CSI2 Single Lane
    CHAL_CAM_INPUT_DUAL_LANE        = (1 << 1),     ///< CSI2 Dual Lane
} CHAL_CAM_INPUT_MODE_t;

/**
* CAM Clock Mode
*****************************************************************************/
typedef enum
{
    CHAL_CAM_DATA_STROBE            = (1 << 0),     ///< Data/Strobe
    CHAL_CAM_DATA_CLOCK             = (1 << 1),     ///< Data/Clock
} CHAL_CAM_CLOCK_MODE_t;


/**
* CAM Data Packing Type 
*****************************************************************************/
typedef enum
{
    CHAL_CAM_PIXEL_NONE             = 0,            ///< No pixel packing defined
    CHAL_CAM_PIXEL_6BIT             = (1 << 0),     ///< 6 bit data
    CHAL_CAM_PIXEL_7BIT             = (1 << 1),     ///< 7 bit data
    CHAL_CAM_PIXEL_8BIT             = (1 << 2),     ///< 8 bit data
    CHAL_CAM_PIXEL_10BIT            = (1 << 3),     ///< 10 bit data
    CHAL_CAM_PIXEL_12BIT            = (1 << 4),     ///< 12 bit data
    CHAL_CAM_PIXEL_14BIT            = (1 << 5),     ///< 14 bit data
    CHAL_CAM_PIXEL_16BIT            = (1 << 6),     ///< 16 bit data
} CHAL_CAM_PIXEL_SIZE_t;

/**
* CAM Decode Formats
*****************************************************************************/
typedef enum
{
    CHAL_CAM_DEC_NONE               = 0,            ///< No DPCM Decode
    CHAL_CAM_6_10_DPCM_DEC          = (1 << 0),     ///< 6-10 bit DPCM Decode
    CHAL_CAM_6_12_DPCM_DEC          = (1 << 1),     ///< 6-12 bit DPCM Decode
    CHAL_CAM_7_10_DPCM_DEC          = (1 << 2),     ///< 7-10 bit DPCM Decode
    CHAL_CAM_7_12_DPCM_DEC          = (1 << 3),     ///< 7-12 bit DPCM Decode
    CHAL_CAM_8_10_DPCM_DEC          = (1 << 4),     ///< 8-10 bit DPCM Decode
    CHAL_CAM_8_12_DPCM_DEC          = (1 << 5),     ///< 8-12 bit DPCM Decode
    CHAL_CAM_8_14_DPCM_DEC          = (1 << 6),     ///< 8-14 bit DPCM Decode
    CHAL_CAM_8_16_DPCM_DEC          = (1 << 7),     ///< 8-16 bit DPCM Decode
    CHAL_CAM_10_14_DPCM_DEC         = (1 << 8),     ///< 10-14 bit DPCM Decode
    CHAL_CAM_10_16_DPCM_DEC         = (1 << 9),    ///< 10-16 bit DPCM Decode
    CHAL_CAM_12_16_DPCM_DEC         = (1 << 10),    ///< 12-16 bit DPCM Decode
    CHAL_CAM_FSP_DEC                = (1 << 11),    ///< FSP Decode
} CHAL_CAM_DECODE_t;

/**
* CAM Encode Formats
*****************************************************************************/
typedef enum
{
    CHAL_CAM_ENC_NONE               = 0,            ///< No DPCM Encoding
    CHAL_CAM_10_8_DPCM_ENC          = (1 << 0),     ///< 10-8 bit DPCM Encode
    CHAL_CAM_12_8_DPCM_ENC          = (1 << 1),     ///< 12-8 bit DPCM Encode
    CHAL_CAM_14_8_DPCM_ENC          = (1 << 2),     ///< 12-8 bit DPCM Encode
} CHAL_CAM_ENCODE_t;


/**
* CAM Encode Block length
*****************************************************************************/
typedef enum
{
    CHAL_CAM_ENC_BLK_LTH_0 = 0,                         ///< line encoding
    CHAL_CAM_ENC_BLK_LTH_8 = 8,                         ///< 8 pixel block encoding
    CHAL_CAM_ENC_BLK_LTH_16 = 16,                       ///< 16 pixel block encoding
    CHAL_CAM_ENC_BLK_LTH_32 = 32,                       ///< 32 pixel block encoding
    CHAL_CAM_ENC_BLK_LTH_64 = 64,                       ///< 64 pixel block encoding
    CHAL_CAM_ENC_BLK_LTH_128 = 128,                     ///< 128 pixel block encoding
    CHAL_CAM_ENC_BLK_LTH_256 = 256,                     ///< 256 pixel block encoding
    CHAL_CAM_ENC_BLK_LTH_512 = 512,                     ///< 512 pixel block encoding
} CHAL_CAM_ENC_BLK_LTH_t;

/**
* CAM receiver status
*****************************************************************************/
typedef enum
{
    CHAL_CAM_RX_INT_CHAN_0          = (1 << 0),     ///< Interrupt pending
    CHAL_CAM_RX_INT_CHAN_1          = (1 << 0),     ///< Channel 1 Interrupt pending (if present)
    CHAL_CAM_RX_INT_PKT_0           = (1 << 1),     ///< Interrupt pending Packet 0
    CHAL_CAM_RX_INT_PKT_1           = (1 << 2),     ///< Interrupt pending Packet 1
    CHAL_CAM_RX_PKT_ERROR           = (1 << 3),     ///< Packet Error
    CHAL_CAM_RX_PANIC               = (1 << 4),     ///< Panic State
    CHAL_CAM_RX_BUSY                = (1 << 5),     ///< Busy
    CHAL_CAM_RX_CRC_ERROR           = (1 << 6),     ///< CRC error
    CHAL_CAM_RX_FIFO_ERROR          = (1 << 7),     ///< FIFO error
    CHAL_CAM_RX_PARITY_ERROR        = (1 << 8),     ///< Parity Bit error
    CHAL_CAM_RX_BIT_ERROR           = (1 << 9),     ///< Single Bit error, corrected
    CHAL_CAM_RX_CLK_PRESENT         = (1 << 10),    ///< High Speed Clock status
    CHAL_CAM_RX_SYNCD               = (1 << 11),    ///< Syncronisation status
    CHAL_CAM_RX_ERROR               = (1 << 12),    ///< General Receiver Error
} CHAL_CAM_RX_STATUS_t;

/**
* CAM lane state request
*****************************************************************************/
typedef enum
{
    CHAL_CAM_LANE_NOCHANGE          = 0,            ///< Lane No Change in State
    CHAL_CAM_LANE_ULP               = (1 << 0),     ///< Lane Ultra-Low Power State
    CHAL_CAM_LANE_HS                = (1 << 1),     ///< Lane High Speed
    CHAL_CAM_LANE_STATE_ERROR       = (1 << 2),     ///< Lane Low Power State Transition error
    CHAL_CAM_LANE_ERROR             = (1 << 3),     ///< Lane Error
    CHAL_CAM_LANE_FF_ERROR          = (1 << 4),     ///< Lane Fifo overflow error
    CHAL_CAM_LANE_PD                = (1 << 5),     ///< Lane Power Down
    CHAL_CAM_LANE_EN                = (1 << 6),     ///< Lane Enable
    CHAL_CAM_LANE_LPEN              = (1 << 7),     ///< Lane Low Power Enable
    CHAL_CAM_LANE_TERM_EN           = (1 << 8),     ///< Lane Termination setting
    CHAL_CAM_LANE_SYNC_MATCHING     = (1 << 9),     ///< Lane Sync Matching Exact setting
    CHAL_CAM_LANE_ANALOG_BIAS       = (1 << 10),    ///< Lane Analog Bias setting
    CHAL_CAM_LANE_HS_RX_TIME        = (1 << 11),    ///< Lane High Speed Data Reception delay time
    CHAL_CAM_LANE_HS_SETTLE_TIME    = (1 << 12),    ///< Lane High Speed Reception Settle time
    CHAL_CAM_LANE_HS_TERM_TIME      = (1 << 13),    ///< Lane High Speed Reception Termination enable time
    CHAL_CAM_LANE_HS_CLK_TIME       = (1 << 14),    ///< Lane High Speed Clock Timeout
} CHAL_CAM_LANE_STATE_t;

/**
* CAM channel interrupt type
*****************************************************************************/
typedef enum
{
    CHAL_CAM_INT_FRAME_START        = (1 << 0),     ///< Frame Start Interrupt
    CHAL_CAM_INT_FRAME_END          = (1 << 1),     ///< Frame End Interrupt
    CHAL_CAM_INT_LINE_COUNT         = (1 << 2),     ///< Line Count Interrupt

    CHAL_CAM_INT_LINE_START         = (1 << 3),     ///< Line Start Interrupt
    CHAL_CAM_INT_LINE_END           = (1 << 4),     ///< Line End Interrupt

    CHAL_CAM_INT_DATA_END           = (1 << 5),     ///< Embedded Data End Interrupt
    CHAL_CAM_INT_DATA_FRAME_END     = (1 << 6),     ///< Embedded Data Frame End Interrupt

    CHAL_CAM_INT_PKT                = (1 << 7),     ///< Generic Packet Interrupt
    CHAL_CAM_INT_FRAME_ERROR        = (1 << 8),     ///< Frame ERROR
} CHAL_CAM_INTERRUPT_t;

/**
* CAM reset type
*****************************************************************************/
typedef enum
{
    CHAL_CAM_RESET_RX               = (1 << 0),     ///< Reset entire receiver, Ints disabled, Panic zero'd
    CHAL_CAM_RESET_ARST             = (1 << 1),     ///< Analog reset entire receiver
    CHAL_CAM_RESET_IMAGE            = (1 << 2),     ///< Analog reset entire receiver
    CHAL_CAM_RESET_DATA             = (1 << 3),     ///< Analog reset entire receiver
} CHAL_CAM_RESET_t;

/**
* CAM Burst length
*****************************************************************************/
typedef enum
{
    CHAL_CAM_BURST_LENGTH_2         = (1 << 0),     ///< 2 words
    CHAL_CAM_BURST_LENGTH_4         = (1 << 1),     ///< 4 words
    CHAL_CAM_BURST_LENGTH_8         = (1 << 2),     ///< 8 words
    CHAL_CAM_BURST_LENGTH_16        = (1 << 3),     ///< 16 words
} CHAL_CAM_BURST_LENGTH_t;

/**
* CAM Burst spacing
*****************************************************************************/
typedef enum
{
    CHAL_CAM_BURST_SPACE_2          = (1 << 0),     ///< 2 cycles
    CHAL_CAM_BURST_SPACE_4          = (1 << 1),     ///< 4 cycles
    CHAL_CAM_BURST_SPACE_8          = (1 << 2),     ///< 8 cycles
    CHAL_CAM_BURST_SPACE_16         = (1 << 3),     ///< 16 cycles
    CHAL_CAM_BURST_SPACE_32         = (1 << 4),     ///< 32 cycles
    CHAL_CAM_BURST_SPACE_64         = (1 << 5),     ///< 64 cycles
    CHAL_CAM_BURST_SPACE_128        = (1 << 6),     ///< 128 cycles
    CHAL_CAM_BURST_SPACE_256        = (1 << 7),     ///< 256 cycles
    CHAL_CAM_BURST_SPACE_512        = (1 << 8),     ///< 512 cycles
    CHAL_CAM_BURST_SPACE_1024       = (1 << 9),     ///< 1024 cycles
} CHAL_CAM_BURST_SPACE_t;

/**
* CAM CPI sync trigger
*****************************************************************************/
typedef enum
{
    CHAL_CAM_SYNC_ACTIVE_LOW        = (1 << 0),     ///< Sync active low
    CHAL_CAM_SYNC_ACTIVE_HIGH       = (1 << 1),     ///< Sync active high
    CHAL_CAM_SYNC_EDGE_NEG          = (1 << 2),     ///< Sync to Negative edge
    CHAL_CAM_SYNC_EDGE_POS          = (1 << 3),     ///< Sync to Positive edge
} CHAL_CAM_SYNC_TRIGGER_t;

/**
* CAM CPI sync signal definition
*****************************************************************************/
typedef enum
{
    CHAL_CAM_SYNC_DEFINES_ACTIVE    = (1 << 0),     ///< Sync signal defines active data
    CHAL_CAM_SYNC_DEFINES_SYNC      = (1 << 1),     ///< Sync signal defined by sync signal
    CHAL_CAM_SYNC_START_DATA        = (1 << 2),     ///< Sync signals start of active data
    CHAL_CAM_SYNC_IGNORED           = (1 << 3),     ///< Sync signal ignored
} CHAL_CAM_SYNC_SIGNAL_t;

/**
* CAM CPI sync mode
*****************************************************************************/
typedef enum
{
    CHAL_CAM_SYNC_EXTERNAL          = (1 << 0),     ///< Sync uses H and V syncs
    CHAL_CAM_SYNC_EMBEDDED          = (1 << 1),     ///< Sync uses embedded syncs (ITU BU.601)
} CHAL_CAM_SYNC_MODE_t;

/**
* CAM CPI field mode
*****************************************************************************/
typedef enum
{
    CHAL_CAM_FIELD_H_V              = (1 << 0),     ///< Field calculated from H and V syncs
    CHAL_CAM_FIELD_EMBEDDED         = (1 << 1),     ///< Field calculated from embedded CCIR656
    CHAL_CAM_FIELD_EVEN             = (1 << 2),     ///< Field is always even
    CHAL_CAM_FIELD_ODD              = (1 << 3),     ///< Field is always odd
} CHAL_CAM_FMODE_t;

/**
* CAM CPI field mode
*****************************************************************************/
typedef enum
{
    CHAL_CAM_SCOPE_DISABLED         = (1 << 0),     ///< Scope mode disabled
    CHAL_CAM_SCOPE_ENABLED          = (1 << 1),     ///< Scope mode enabled
} CHAL_CAM_SCOPE_t;

/**
* CAM CPI clock sample edge
*****************************************************************************/
typedef enum
{
    CHAL_CAM_CLK_EDGE_NEG           = (1 << 0),     ///< Sample Negative edge
    CHAL_CAM_CLK_EDGE_POS           = (1 << 1),     ///< Sample Positive edge
} CHAL_CAM_CLK_EDGE_t;


/**
* CAM Receiver Interface Config
*****************************************************************************/
typedef struct 
{
    CHAL_CAM_INTF_t                 intf;               ///< Camera Interface
    CHAL_CAM_CLOCK_MODE_t           clk_mode;           ///< Receiver clock mode
    CHAL_CAM_INPUT_MODE_t           input_mode;         ///< Receiver clock mode
    void*                           p_cpi_cfg_st;       ///< CPI configuration structure (if available)
} CHAL_CAM_CFG_INTF_st_t;               


/**
* CAM Receiver Control Config
*****************************************************************************/
typedef struct 
{
    CHAL_CAM_INTF_t                 intf;               ///< Camera Interface
    cUInt32                         packet_timeout;         ///< Packet Frame Timeout, 0=Disabled, n=# HS clocks
    cBool                           line_start_suppress;    ///< 1=Suppress Line Start Codes
    CHAL_CAM_BURST_LENGTH_t         burst_length;           ///< Output Engine burst length
    CHAL_CAM_BURST_SPACE_t          burst_space;            ///< Output Engine burst spacing
    cUInt32                         norm_pr;                ///< Normal Priority signal 1=enabled
    cUInt32                         panic_pr;               ///< Panic Priority signal 0=disabled
    cUInt32                         panic_thr;              ///< Panic Threshold
    cBool                           panic_enable;           ///< Panic Enable
} CHAL_CAM_CFG_CNTRL_st_t;               


typedef struct 
{
    cUInt32              start_addr;             ///< (in) start address
    cUInt32              size;                   ///< (in) buffer size in bytes
    cUInt32              line_stride;            ///< (in) line stride (CSL_CAM_IMAGE only)
    cBool                buf_wrap_enable;        ///< (in) buffer wrap enable (circular)
} CHAL_CAM_BUFFER_st_t;      

typedef struct
{
    CHAL_CAM_BUFFER_st_t     *image0Buff;
    CHAL_CAM_BUFFER_st_t     *image1Buff;    
    CHAL_CAM_BUFFER_st_t     *dataBuff;    
} CHAL_CAM_Buffers_st_t;

/**
* CAM CSI, CCP, CPI channel cfg, set each frame
*****************************************************************************/
typedef struct 
{
    CHAL_CAM_INTF_t                 intf;               ///< Camera Interface
    CHAL_CAM_CHAN_t                 chan;               ///< Camera Channel
    CHAL_CAM_Buffers_st_t           buffers;            ///< Image and data buffer configuration
    UInt32                          write_ptr;          ///< (out) write pointer
    UInt32                          bytes_per_line;     ///< (out) bytes per line (CSL_CAM_IMAGE only)
    UInt32                          lines_per_frame;    ///< (out) lines per frame (CSL_CAM_IMAGE only)
} CHAL_CAM_BUFFER_CFG_st_t;               

/**
* CAM CSI, CCP, CPI frame cfg, set each frame
*****************************************************************************/
typedef struct 
{
    CHAL_CAM_INTF_t                 intf;               ///< Camera Interface
    CHAL_CAM_CHAN_t                 chan;               ///< Camera Channel
    CHAL_CAM_INTERRUPT_t            interrupts;         ///< interrupts enabled
    cUInt32                         line_count;         ///< line count interrupt
    CHAL_CAM_SHOT_MODE_t            mode;               ///< frame capture mode
} CHAL_CAM_FRAME_CFG_st_t;               

/**
* CAM Image ID (virtual channels + data type) cfg, set each frame
*****************************************************************************/
typedef struct 
{
    CHAL_CAM_INTF_t                 intf;               ///< Camera Interface
    CHAL_CAM_CHAN_t                 chan;               ///< Camera Channel
    cUInt8                          image_data_id_0;    ///< image data id 0 7:6=[VC]   5:0=Data Type=0=No Override
    cUInt8                          image_data_id_1;    ///< image data id 1
    cUInt8                          image_data_id_2;    ///< image data id 2
    cUInt8                          image_data_id_3;    ///< image data id 3
} CHAL_CAM_IMAGE_ID_st_t;               


/**
* CAM Data cfg, set each frame
*****************************************************************************/
typedef struct 
{
    CHAL_CAM_INTF_t                 intf;               ///< Camera Interface
    CHAL_CAM_CHAN_t                 chan;               ///< Camera Channel
    cUInt8                          data_id;            ///< Embedded Data ID 7:6=[VC]   5:0=Data Type=0=No Override
    CHAL_CAM_PIXEL_SIZE_t           data_size;          ///< Data size
    cUInt32                         line_count;         ///< Embedded data lines (0=No embedded data)
    CHAL_CAM_INTERRUPT_t            intr_enable;        ///< Interrupt Enable (CHAL_CAM_INT_DATA_END  || CHAL_CAM_INT_DATA_FRAME_END)
    Boolean                         fsp_decode_enable;  ///< FSP Decode Enable:  0 = Disable  1 = Enable
} CHAL_CAM_DATA_CFG_st_t;               

/**
* CAM CPI window cfg
*****************************************************************************/
typedef struct 
{
    CHAL_CAM_INTF_t                 intf;               ///< Camera Interface
    CHAL_CAM_CHAN_t                 chan;               ///< Camera Channel (unused)
    Boolean                         enable;             ///< Windowing Enable:  FALSE = Disable  TRUE = Enable
    cBool                           field_gating;       ///< Enable Field Gating
    cUInt32                         h_start_sample;     ///< horizontal start sample
    cUInt32                         h_end_sample;       ///< horizontal end sample
    cUInt32                         v_start_sample;     ///< vertical start sample
    cUInt32                         v_end_sample;       ///< vertical end sample
} CHAL_CAM_WINDOW_CFG_st_t;               

/**
* CAM Pkt Capture Configuration
*****************************************************************************/
typedef struct 
{
    CHAL_CAM_INTF_t                 intf;               ///< Camera Interface
    CHAL_CAM_CHAN_t                 chan;               ///< Camera Channel (unused)
    CHAL_CAM_PKT_CMP_SELECT_t       instance;           ///< Packet Compare Instance
    cBool                           cmp_enable;         ///< Packet Compare Enable
    cBool                           intr_enable;        ///< Interrupt Enable
    cBool                           capture_enable;     ///< Header Capture Enable
    cUInt8                          data_id;            ///< Packet Data ID 7:6=[VC] 5:0=Data Type
} CHAL_CAM_PKT_CFG_st_t;               

/**
* CAM CSI Short Pkt
*****************************************************************************/
typedef struct 
{
    CHAL_CAM_INTF_t                 intf;               ///< Camera Interface
    CHAL_CAM_CHAN_t                 chan;               ///< Camera Channel (unused)
    cUInt32                         word_count;         ///< Long Packet word count/Short packet data field
    cUInt32                         data_id;            ///< Packet Data ID 7:6=[VC] 5:0=Data Type
    cUInt32                         ecc;                ///< ECC
} CHAL_CAM_PKT_st_t;               


/**
* CAM Receive Channel Pipeline Config
*****************************************************************************/
typedef struct 
{
    CHAL_CAM_INTF_t                 intf;               ///< Camera Interface
    CHAL_CAM_CHAN_t                 chan;               ///< Camera Channel
    CHAL_CAM_ENC_BLK_LTH_t          enc_blk_lnth;       ///< Encode block length
    CHAL_CAM_ENCODE_t               enc_proc;           ///< Encode Processing
    CHAL_CAM_PIXEL_SIZE_t           enc_pixel_pack;     ///< Encode Pixel Pack size
    cBool                           enc_predictor;      ///< Enable Advanced Predictor
    CHAL_CAM_DECODE_t               dec_proc;           ///< Decode Processing
    CHAL_CAM_PIXEL_SIZE_t           dec_pixel_unpack;   ///< Decode Pixel Unpack size
    cBool                           dec_predictor;      ///< Enable Advanced Predictor
//    CHAL_CAM_DECODE_t               data_dec_proc;      ///< Decode Processing on embedded data capture
} CHAL_CAM_PIPELINE_CFG_st_t;               

/**
* CAM AFE Config
*****************************************************************************/
typedef struct 
{
    CHAL_CAM_INTF_t                 intf;               ///< Camera Interface
    cUInt32                         bandgap_bias;       ///< Band Gap Control 
    cBool                           data_lane_disable;  ///< Disable Data Lane output
} CHAL_CAM_AFE_CFG_st_t;               

/**
* CAM Get/Set Lane Control Structure
*****************************************************************************/
typedef struct 
{
    CHAL_CAM_INTF_t                 intf;               ///< Camera Interface
    CHAL_CAM_CHAN_t                 chan;               ///< Camera Channel (unused)
//    CHAL_CAM_CLOCK_MODE_t           clk_mode;           ///< Receiver clock mode
    CHAL_CAM_LANE_SELECT_t          lane;               ///< Camera Lane Select
    CHAL_CAM_LANE_STATE_t           cntrl_state;        ///< Lane state to configure
    cUInt32                         param;              ///< configure param
} CHAL_CAM_LANE_CNTRL_st_t;               

/**
* CAM Get\Set Parameter Structure
*****************************************************************************/
typedef struct 
{
    CHAL_CAM_INTF_t                 intf;               ///< Camera Interface
    CHAL_CAM_CHAN_t                 chan;               ///< Camera Channel
    CHAL_CAM_LANE_SELECT_t          lane;               ///< Camera Lane Select
    CHAL_CAM_CFG_SELECT_t           select;             ///< Camera Data Input Select
    cUInt32                         virt_chan;          ///< virtual channel #
    cUInt32                         param;              ///< In/Out Parameter or Pointer to additional parameters
} CHAL_CAM_PARAM_st_t;               


/**
* CAM CPI Receiver Controller Config
*****************************************************************************/
typedef struct 
{
    CHAL_CAM_SYNC_MODE_t            sync_mode;      ///< Sync External or Embedded
    CHAL_CAM_SYNC_SIGNAL_t          hsync_signal;   ///< How HSYNC defines active data
    CHAL_CAM_SYNC_TRIGGER_t         hsync_trigger;  ///< How HSYNC is triggered (level, edge)
    CHAL_CAM_SYNC_SIGNAL_t          vsync_signal;   ///< How VSYNC defines active data
    CHAL_CAM_SYNC_TRIGGER_t         vsync_trigger;  ///< How VSYNC is triggered (level, edge)
    CHAL_CAM_CLK_EDGE_t             clock_edge;     ///< Edge Clock is sampled
    CHAL_CAM_PIXEL_SIZE_t           bit_width;      ///< bit width selection
    cInt32                          data_shift;     ///< data shift (+) left shift  (-) right shift
    CHAL_CAM_FMODE_t                fmode;          ///< field mode
    CHAL_CAM_SCOPE_t                smode;          ///< scope mode
} CHAL_CAM_CPI_CNTRL_CFG_st_t;               

/**
* CAM CSI-2 Channels
*****************************************************************************/
#define CHAL_CAM_CSI_RECEIVERS      1
#define CHAL_CAM_CSI_DATA_LANES     2
#define CHAL_CAM_CSI_VIRT_CHANNELS   4

/**
* CAM CCP-2 Channels
*****************************************************************************/
#define CHAL_CAM_CCP_RECEIVERS      1
#define CHAL_CAM_CCP_CHANNELS       1
/**
* CAM PARALLEL Channels
*****************************************************************************/
#define CHAL_CAM_PARALLEL_CHANNELS  0

/**
*
*  @brief  Initialize CHAL CAM for the passed CAM instance
*
*  @param  baseAddr (in) mapped address of CAM block
*
*  @return handle of this CAM instance      
*
*  @note 
*****************************************************************************/
CHAL_HANDLE chal_cam_init(cUInt32 baseAddr);

/**
*
*  @brief  De-Initialize CHAL CAM for the passed CAM instance
*
*  @param  handle (in) handle of this CAM instance
*
*  @return none
*
*  @note 
*****************************************************************************/
cVoid chal_cam_deinit(CHAL_HANDLE handle);


/**
*
*  @brief  Configure CAM Rx/Tx Control
*
*  @param  handle   (in) handle of this CAM instance
*  @param  cfg      (in) configuration parameters to be set
*
*  @return none
*
*  @note 
*****************************************************************************/
CHAL_CAM_STATUS_CODES chal_cam_cfg_intf(CHAL_HANDLE handle, CHAL_CAM_CFG_INTF_st_t* cfg);

/**
*
*  @brief  Configure CAM Rx/Tx Control
*
*  @param  handle   (in) handle of this CAM instance
*  @param  cfg      (in) configuration parameters to be set
*
*  @return none
*
*  @note 
*****************************************************************************/
CHAL_CAM_STATUS_CODES chal_cam_cfg_cntrl(CHAL_HANDLE handle, CHAL_CAM_CFG_CNTRL_st_t* cfg);

/**
*
*  @brief  Configure CAM Channel encoder/decoder pipeline
*
*  @param  handle   (in) handle of this CAM instance
*  @param  cfg      (in) configuration parameters to be set
*
*  @return none
*
*  @note 
*****************************************************************************/
CHAL_CAM_STATUS_CODES chal_cam_cfg_pipeline(CHAL_HANDLE handle, CHAL_CAM_PIPELINE_CFG_st_t* cfg);

/**
*
*  @brief  Configure CAM x for Receive Data
*
*  @param  handle   (in) handle of this CAM instance
*  @param  cfg      (in) configuration parameters to be set
*
*  @return none
*
*  @note 
*****************************************************************************/
CHAL_CAM_STATUS_CODES chal_cam_cfg_buffer(CHAL_HANDLE handle, CHAL_CAM_BUFFER_CFG_st_t* cfg);

/**
*
*  @brief  Get CAM Configuration for Buffer
*
*  @param  handle   (in) handle of this CAM instance
*  @param  cfg      (out) configuration parameters to be read
*
*  @return none
*
*  @note 
*****************************************************************************/
CHAL_CAM_STATUS_CODES chal_cam_get_buffer_cfg(CHAL_HANDLE handle, CHAL_CAM_BUFFER_CFG_st_t* cfg);

/**
*
*  @brief  Configure CAM Channel x for Receive
*
*  @param  handle   (in) handle of this CAM instance
*  @param  cfg      (in) configuration parameters to be set
*
*  @return none
*
*  @note 
*****************************************************************************/
CHAL_CAM_STATUS_CODES chal_cam_cfg_frame(CHAL_HANDLE handle, CHAL_CAM_FRAME_CFG_st_t* cfg);

/**
*
*  @brief  Configure CAM Channel x for Receive
*
*  @param  handle   (in) handle of this CAM instance
*  @param  cfg      (in) configuration parameters to be set
*
*  @return none
*
*  @note 
*****************************************************************************/
CHAL_CAM_STATUS_CODES chal_cam_cfg_data(CHAL_HANDLE handle, CHAL_CAM_DATA_CFG_st_t* cfg);

/**
*
*  @brief  Configure CAM Channel Image Id's for Receive
*
*  @param  handle   (in) handle of this CAM instance
*  @param  cfg      (in) configuration parameters to be set
*
*  @return none
*
*  @note 
*****************************************************************************/
CHAL_CAM_STATUS_CODES chal_cam_cfg_image_id(CHAL_HANDLE handle, CHAL_CAM_IMAGE_ID_st_t* cfg);

/**
*
*  @brief  Configure CAM Channel x windowing
*
*  @param  handle   (in) handle of this CAM instance
*  @param  cfg      (in) window configuration parameters
*
*  @return none
*
*  @note 
*****************************************************************************/
CHAL_CAM_STATUS_CODES chal_cam_cfg_window(CHAL_HANDLE handle, CHAL_CAM_WINDOW_CFG_st_t* cfg);

/**
*
*  @brief  Configure CAM module CSI AFE Controller
*
*  @param  handle   (in) handle of this CAM instance
*  @param  cfg      (in) configuration parameters to be set
*
*  @return none
*
*  @note 
*****************************************************************************/
CHAL_CAM_STATUS_CODES chal_cam_cfg_afe_cntrl(CHAL_HANDLE handle, CHAL_CAM_AFE_CFG_st_t* cfg);

/**
*
*  @brief  CAM module Set Analog Power
*
*  @param  handle (in) handle of this CAM instance
*  @param  *param (in) parameter struct indicating on/off
*
*  @return none
*
*  @note 
*****************************************************************************/
CHAL_CAM_STATUS_CODES chal_cam_set_analog_pwr(CHAL_HANDLE handle, CHAL_CAM_PARAM_st_t* param);

/**
*
*  @brief  CAM module Set Bandgap Power
*
*  @param  handle (in) handle of this CAM instance
*  @param  *param (in) parameter struct indicating on/off
*
*  @return none
*
*  @note 
*****************************************************************************/
CHAL_CAM_STATUS_CODES chal_cam_set_bandgap_pwr(CHAL_HANDLE handle, CHAL_CAM_PARAM_st_t* param);

/**
*
*  @brief  CAM module Set Lane Enable\Disable
*
*  @param  handle (in) handle of this CAM instance
*  @param  *param (in) parameter struct lanes=lanes selected, param=lanes on/off
*
*  @return none
*
*  @note 
*****************************************************************************/
CHAL_CAM_STATUS_CODES chal_cam_lane_enable(CHAL_HANDLE handle, CHAL_CAM_PARAM_st_t* param);

/**
*
*  @brief  CAM module Set Lane Control
*
*  @param  handle (in) handle of this CAM instance
*  @param  cfg      (in) configuration parameters to be set
*
*  @return none
*
*  @note 
*****************************************************************************/
CHAL_CAM_STATUS_CODES chal_cam_set_lane_cntrl(CHAL_HANDLE handle, CHAL_CAM_LANE_CNTRL_st_t* cfg);


/**
*
*  @brief  Set CAM module Input Intf
*
*  @param  handle (in) handle of this CAM instance
*  @param  cfg      (in) configuration parameters to be set
*
*  @return none
*
*  @note 
*****************************************************************************/
CHAL_CAM_STATUS_CODES chal_cam_set_intf(CHAL_HANDLE handle, CHAL_CAM_CFG_INTF_st_t* cfg);

/**
*
*  @brief  CAM module Configure Short Packet Capture
*
*  @param  handle       (in) handle of this CAM instance
*  @param  cfg          (in) configuration parameters to be set
*
*  @return none
*
*  @note 
*****************************************************************************/
CHAL_CAM_STATUS_CODES chal_cam_cfg_short_pkt(CHAL_HANDLE handle, CHAL_CAM_PKT_CFG_st_t* cfg);

/**
*
*  @brief  CAM module Get Short Packet
*
*  @param  handle       (in) handle of this CAM instance
*  @param  short_pkt    (in) packet struct to fill in
*
*  @return none
*
*  @note 
*****************************************************************************/
CHAL_CAM_STATUS_CODES chal_cam_get_short_pkt(CHAL_HANDLE handle, CHAL_CAM_PKT_st_t* short_pkt);

/**
*
*  @brief  CAM module Get Receiver status
*
*  @param  handle (in) handle of this CAM instance
*  @param  *param (in) parameter struct to rx status
*
*  @return none
*
*  @note 
*****************************************************************************/
UInt32 chal_cam_get_rx_status(CHAL_HANDLE handle, CHAL_CAM_PARAM_st_t* param);

/**
*   \brief 
*  		   	CAM module Get Receiver Lane status
*   \note 
*          	Parameters:   
*  				handle          (in) this CAM instance
*  				param.intf      (in) interface select
*  				param.chan      (in) unused
*  				param.lane      (in) lane select: CLK, Lane 1-2
*  				param.param     (out) OR'd Lane Status's for receiver
*           Return:
*               CHAL_CAM_STATUS_CODES
*****************************************************************************/
CHAL_CAM_STATUS_CODES chal_cam_get_lane_status(CHAL_HANDLE handle, CHAL_CAM_PARAM_st_t* param);

/**
*
*  @brief  CAM module Get Receiver Channel status
*
*  @param  handle (in) handle of this CAM instance
*  @param  *param (in) parameter struct to rx status
*
*  @return none
*
*  @note 
*****************************************************************************/
UInt32 chal_cam_get_chan_status(CHAL_HANDLE handle, CHAL_CAM_PARAM_st_t* param);

/**
*
*  @brief  CAM module Receiver Software Reset
*
*  @param  handle (in) handle of this CAM instance
*  @param  *param (in) parameter struct to pass interface, channel, and reset type
*
*  @return none
*
*  @note 
*****************************************************************************/
CHAL_CAM_STATUS_CODES chal_cam_reset(CHAL_HANDLE handle, CHAL_CAM_PARAM_st_t* param);


/**
*
*  @brief  CAM module Receiver Initialize
*
*  @param  handle (in) handle of this CAM instance
*  @param  *param (in) parameter struct to pass interface & channel to reset
*
*  @return none
*
*  @note 
*****************************************************************************/
CHAL_CAM_STATUS_CODES chal_cam_rx_reset(CHAL_HANDLE handle, CHAL_CAM_PARAM_st_t* param);

/**
*
*  @brief  CAM module Receiver channel number of bytes written in current frame
*
*  @param  handle (in) handle of this CAM instance
*  @param  *param (out) parameter struct to get bytes written
*
*  @return none
*
*  @note 
*****************************************************************************/
CHAL_CAM_STATUS_CODES chal_cam_rx_bytes_written(CHAL_HANDLE handle, CHAL_CAM_PARAM_st_t* param);

/**
*
*  @brief  CAM module Receiver force trigger
*
*  @param  handle (in) handle of this CAM instance
*  @param  *param (out) parameter struct for interface and channel
*
*  @return none
*
*  @note 
*****************************************************************************/
CHAL_CAM_STATUS_CODES chal_cam_channel_trigger(CHAL_HANDLE handle, CHAL_CAM_PARAM_st_t* param);

/**
*
*  @brief  CAM module Receiver Start
*
*  @param  handle (in) handle of this CAM instance
*  @param  *param (in) parameter struct to pass interface & channel to start
*
*  @return none
*
*  @note 
*****************************************************************************/
CHAL_CAM_STATUS_CODES chal_cam_rx_start(CHAL_HANDLE handle, CHAL_CAM_PARAM_st_t* param);

/**
*
*  @brief  CAM module Receiver Stop
*
*  @param  handle (in) handle of this CAM instance
*  @param  *param (in) parameter struct to pass interface & channel to stop
*
*  @return none
*
*  @note 
*****************************************************************************/
CHAL_CAM_STATUS_CODES chal_cam_rx_stop(CHAL_HANDLE handle, CHAL_CAM_PARAM_st_t* param);

/**
*
*  @brief  CAM module Register Output
*
*  @param  handle (in) handle of this CAM instance
*  @param  *param (in) parameter struct to pass interface & channel to report register status
*
*  @return none
*
*  @note 
*****************************************************************************/
CHAL_CAM_STATUS_CODES chal_cam_register_display(CHAL_HANDLE handle, CHAL_CAM_PARAM_st_t* param);

/** @} */

#ifdef __cplusplus
}
#endif

#endif // _CHAL_CAM_H_

