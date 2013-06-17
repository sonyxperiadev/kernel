/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2011  Broadcom Corporation                                                        */
/*                                                                                              */
/*     Unless you and Broadcom execute a separate written software license agreement governing  */
/*     use of this software, this software is licensed to you under the terms of the GNU        */
/*     General Public License version 2 (the GPL), available at                                 */
/*                                                                                              */
/*          http://www.broadcom.com/licenses/GPLv2.php                                          */
/*                                                                                              */
/*     with the following added to such license:                                                */
/*                                                                                              */
/*     As a special exception, the copyright holders of this software give you permission to    */
/*     link this software with independent modules, and to copy and distribute the resulting    */
/*     executable under terms of your choice, provided that you also meet, for each linked      */
/*     independent module, the terms and conditions of the license of that module.              */
/*     An independent module is a module which is not derived from this software.  The special  */
/*     exception does not apply to any modifications of the software.                           */
/*                                                                                              */
/*     Notwithstanding the above, under no circumstances may you combine this software in any   */
/*     way with any other Broadcom software provided under a license other than the GPL,        */
/*     without Broadcom's express prior written consent.                                        */
/*                                                                                              */
/************************************************************************************************/
/**
*
*   @file   csl_cam.h
*
*   @brief  CAM CSL layer 
*
****************************************************************************/
/**
*
* @defgroup CAMGroup Broadcom Camera Controller
*
* @brief This group defines the APIs for Camera CSI2, CCP2, CPI interfaces.
*
* @ingroup CSLGroup
*****************************************************************************/
#ifndef __CSL_CAM_H
#define __CSL_CAM_H

#ifdef __cplusplus
extern "C" {
#endif

/** 
 * @addtogroup CAMGroup 
 * @{
 */

#define MAX_PACKET_CAPTURE      2
#define CAM_MAX_HANDLES         1
#define CAM_BASE_ADDR           MM_CSI0_BASE_ADDR
// ACP Memory Type Define    
#define CSL_MEM_ACP_BITS        0xF0000000
#define CSL_CAM_MEM_ACP_TYPE    0x40000000

#define CSL_CAM_BUFFER_BASE     0x00000020
#define CSL_CAM_ISP_LINESTRIDE_BASE     0x00000020
//#define CSL_CAM_ENABLE_DBL_BUFFER       1

/******************************************************************************
Global types
*****************************************************************************/

/**
*
* CSL CAM Interface Result  
*
*****************************************************************************/
	typedef enum {
		CSL_CAM_OK = 0,	///< OK, no Error
		CSL_CAM_ERR = (1 << 0),	///< CSL Generic Error
		CSL_CAM_OP_INVALID = (1 << 1),	///< Operation Invalid
		CSL_CAM_NOT_INIT = (1 << 2),	///< CSL not Init
		CSL_CAM_NOT_OPEN = (1 << 3),	///< CSL not Open
		CSL_CAM_IS_OPEN = (1 << 4),	///< CSL Already Open
		CSL_CAM_OS_TOUT = (1 << 5),	///< Generic OS TimeOut
		CSL_CAM_OS_ERR = (1 << 6),	///< Generic OS Err 
		CSL_CAM_ID = (1 << 7),	///< invalid ID
		CSL_CAM_BAD_CLK = (1 << 8),	///< Clock Operation Invalid
		CSL_CAM_BAD_HANDLE = (1 << 10),	///< Invalid handle
		CSL_CAM_BAD_STATE = (1 << 11),	///< Action Not Supported In Curr. Interface State
		CSL_CAM_BAD_PARAM = (1 << 12),	///< bad parameter passed to function
		CSL_CAM_INST_COUNT = (1 << 13),	///< open failure due to instance count
		CSL_CAM_CHAL_ERR = (1 << 14),	///< CHAL Generic Error
	} CSL_CAM_RES_T;

//Interface mode
	typedef enum {
		CSL_CAM_INIT_MODE = 0,
		CSL_CAM_OPEN_MODE = 1,
		CSL_CAM_IDLE_MODE = 2,
		CSL_CAM_STREAM_MODE = 3,
		CSL_CAM_CLOSE_MODE = 4,
	} CSL_CAM_DEVICE_MODE_T;

//the list of AFE ports
	typedef enum {
		CSL_CAM_PORT_AFE_0 = (1 << 0),	///< Port 0 select
		CSL_CAM_PORT_AFE_1 = (1 << 1),	///< Port 1 select
		CSL_CAM_PORT_AFE_MAX
	} CSL_CAM_PORT_AFE_T;

//the list of interfaces
	typedef enum {
		CSL_CAM_INTF_CSI = (1 << 0),
		CSL_CAM_INTF_CCP = (1 << 1),
		CSL_CAM_INTF_CPI = (1 << 2),
	} CSL_CAM_INTF_T;

//the list of port channels
	typedef enum {
		CSL_CAM_PORT_CHAN_0 = (1 << 0),	///< Port Channel 0 select
		CSL_CAM_PORT_CHAN_1 = (1 << 1),	///< Port Channel 1 select
	} CSL_CAM_PORT_CHAN_T;

/**
* CAM Input Mode
*****************************************************************************/
	typedef enum {
		CSL_CAM_NO_MODE_SELECT = 0,	///< CPI input mode
		CSL_CAM_INPUT_MODE_DATA_STROBE = (1 << 0),	///< CCP2 Data Strobe mode
		CSL_CAM_INPUT_MODE_DATA_CLOCK = (1 << 1),	///< CCP2 Data Clock mode
		CSL_CAM_INPUT_SINGLE_LANE = (1 << 2),	///< CSI2 Single Lane
		CSL_CAM_INPUT_DUAL_LANE = (1 << 3),	///< CSI2 Dual Lane
	} CSL_CAM_INPUT_MODE_t;

/**
* CAM Config Select
*****************************************************************************/
	typedef enum {
		CSL_CAM_NO_SELECT = 0,
		CSL_CAM_CLOCK = (1 << 0),	///< Clock select
		CSL_CAM_DATA_LANE_0 = (1 << 1),	///< Lane 0 select
		CSL_CAM_DATA_LANE_1 = (1 << 2),	///< Lane 1 select
		CSL_CAM_IMAGE = (1 << 3),	///< Image select
		CSL_CAM_DATA = (1 << 4),	///< Data select
		CSL_CAM_PACKET = (1 << 5),	///< Packet 0 select
		CSL_CAM_ANALOG = (1 << 6),	///< Analog select
		CSL_CAM_BANDGAP = (1 << 7),	///< Bandgap select
	} CSL_CAM_SELECT_t;

//the frame capture mode
	typedef enum {
		CSL_CAM_CAPTURE_MODE_NORMAL = 0,
		CSL_CAM_CAPTURE_MODE_TRIGGER = 1,
	} CSL_CAM_CAPTURE_MODE_T;

/**
* CAM memory type
*****************************************************************************/
	typedef enum {
		CSL_CAM_MEM_TYPE_NONE = 0,	///< No memory type defined
		CSL_CAM_MEM_TYPE_CACHEABLE_BUFFERABLE = (1 << 0),	///< cacheable & bufferable
		CSL_CAM_MEM_TYPE_UNCACHEABLE_UNBUFFERABLE = (1 << 1),	///< uncacheable & unbufferable
		CSL_CAM_MEM_TYPE_ACP_COMPATIBLE = (1 << 2),	///< ACP compatible
// Memory pointer type
		CSL_CAM_MEM_TYPE_BRALLOC_HANDLE = (1 << 16),	///< BRALLOC HANDLE
	} CSL_CAM_MEM_TYPE_T;

/**
* CAM reset type
*****************************************************************************/
	typedef enum {
		CSL_CAM_RESET_SWR = (1 << 0),	///< Software reset entire receiver
		CSL_CAM_RESET_ARST = (1 << 1),	///< Analog reset entire receiver
	} CSL_CAM_RESET_t;

//interrupts
	typedef enum {
		CSL_CAM_INT_DISABLE = 0,	///< Disable Interrupts
		CSL_CAM_INT_FRAME_START = (1 << 0),	///< Frame Start Interrupt
		CSL_CAM_INT_FRAME_END = (1 << 1),	///< Frame End Interrupt
		CSL_CAM_INT_LINE_START = (1 << 2),	///< Line Start Interrupt
		CSL_CAM_INT_LINE_END = (1 << 3),	///< Line End Interrupt
		CSL_CAM_INT_LINE_COUNT = (1 << 4),	///< Line Count Interrupt
		CSL_CAM_INT_DATA = (1 << 5),	///< Data Interrupt (status only)
		CSL_CAM_INT_PACKET = (1 << 6),	///< Packet Interrupt
		CSL_CAM_INT_FRAME_ERROR = (1 << 7),	///< Error Interrupt
		CSL_CAM_INT_TASK_ERROR = (1 << 8),	///< Task Frame Interrupt Timeout Error
	} CSL_CAM_INTERRUPT_t;

//receiver status
	typedef enum {
		CSL_CAM_RX_INT = (1 << 0),	///< Frame Interrupt pending
		CSL_CAM_RX_INT_DATA = (1 << 1),	///< Embedded Data Interrupt
		CSL_CAM_RX_INT_PKT = (1 << 2),	///< Packet Interrupt pending
		CSL_CAM_RX_PKT_ERROR = (1 << 3),	///< Packet Error
		CSL_CAM_RX_PANIC = (1 << 4),	///< Panic State
		CSL_CAM_RX_BUSY = (1 << 5),	///< Busy
		CSL_CAM_RX_CRC_ERROR = (1 << 6),	///< CRC error
		CSL_CAM_RX_FIFO_ERROR = (1 << 7),	///< FIFO error
		CSL_CAM_RX_PARITY_ERROR = (1 << 8),	///< Parity Bit error
		CSL_CAM_RX_BIT_ERROR = (1 << 9),	///< Single Bit error, corrected
		CSL_CAM_RX_CLK_PRESENT = (1 << 10),	///< High Speed Clock status
		CSL_CAM_RX_SYNCD = (1 << 11),	///< Synchronisation status
		CSL_CAM_RX_ERROR = (1 << 12),	///< Receiver Error status
		CSL_CAM_RX_BUF0_RDY = (1 << 13),	///< Buffer 0 Ready
		CSL_CAM_RX_BUF0_NO = (1 << 14),	///< Buffer 0 Frame Number
		CSL_CAM_RX_BUF1_RDY = (1 << 15),	///< Buffer 1 Ready
		CSL_CAM_RX_BUF1_NO = (1 << 16),	///< Buffer 1 Frame Number
	} CSL_CAM_RX_STATUS_t;

/**
* CAM lane control
*****************************************************************************/
	typedef enum {
		CSL_CAM_LANE_NOCHANGE = 0,	///< Lane No Change in State
		CSL_CAM_LANE_EN = (1 << 0),	///< Lane Enable
		CSL_CAM_LANE_PD = (1 << 1),	///< Lane Power Down
		CSL_CAM_LANE_LPEN = (1 << 2),	///< Lane Low Power Enable
		CSL_CAM_LANE_HS = (1 << 3),	///< Lane High Speed
		CSL_CAM_LANE_TERM_EN = (1 << 4),	///< Lane Termination setting
		CSL_CAM_LANE_SYNC_MATCHING = (1 << 5),	///< Lane Sync Matching Exact setting
		CSL_CAM_LANE_ANALOG_BIAS = (1 << 6),	///< Lane Analog Bias setting
		CSL_CAM_LANE_HS_RX_TIME = (1 << 7),	///< Lane High Speed Data Reception delay time
		CSL_CAM_LANE_HS_SETTLE_TIME = (1 << 8),	///< Lane High Speed Reception Settle time
		CSL_CAM_LANE_HS_TERM_TIME = (1 << 9),	///< Lane High Speed Reception Termination enable time
	} CSL_CAM_LANE_CONTROL_t;

/**
* CAM lane status
*****************************************************************************/
	typedef enum {
		CSL_CAM_LANE_STATUS_PD = (1 << 0),	///< Lane Power Down
		CSL_CAM_LANE_STATUS_EN = (1 << 1),	///< Lane Enable
		CSL_CAM_LANE_STATUS_ULP = (1 << 2),	///< Lane Ultra-Low Power State
		CSL_CAM_LANE_STATUS_HS = (1 << 3),	///< Lane High Speed
		CSL_CAM_LANE_STATUS_TRANSITION_ERROR = (1 << 4),	///< Lane Low Power State Transition error
		CSL_CAM_LANE_STATUS_ERROR = (1 << 5),	///< Lane Error
		CSL_CAM_LANE_STATUS_FF_ERROR = (1 << 6),	///< Lane Fifo overflow error
		CSL_CAM_LANE_STATUS_SYNC_ERROR = (1 << 7),	///< Lane Sync error
	} CSL_CAM_LANE_STATUS_t;

//data size
	typedef enum {
		CSL_CAM_PIXEL_NONE = 0,	///< No pixel packing defined
		CSL_CAM_PIXEL_6BIT = (1 << 0),	///< 6 bit data
		CSL_CAM_PIXEL_7BIT = (1 << 1),	///< 7 bit data
		CSL_CAM_PIXEL_8BIT = (1 << 2),	///< 8 bit data
		CSL_CAM_PIXEL_10BIT = (1 << 3),	///< 10 bit data
		CSL_CAM_PIXEL_12BIT = (1 << 4),	///< 12 bit data
		CSL_CAM_PIXEL_14BIT = (1 << 5),	///< 14 bit data
		CSL_CAM_PIXEL_16BIT = (1 << 6),	///< 16 bit data
	} CSL_CAM_PIXEL_SIZE_T;

//Encoder processing
	typedef enum {
		CSL_CAM_ENC_NONE = 0,	///< No DPCM Encoding
		CSL_CAM_10_8_DPCM_ENC = (1 << 0),	///< 10-8 bit DPCM Encode
		CSL_CAM_12_8_DPCM_ENC = (1 << 1),	///< 12-8 bit DPCM Encode
		CSL_CAM_14_8_DPCM_ENC = (1 << 2),	///< 12-8 bit DPCM Encode
	} CSL_CAM_ENCODER_T;

//Decoder processing
	typedef enum {
		CSL_CAM_DEC_NONE = 0,	///< No DPCM Decode
		CSL_CAM_6_10_DPCM_DEC = (1 << 0),	///< 6-10 bit DPCM Decode
		CSL_CAM_6_12_DPCM_DEC = (1 << 1),	///< 6-12 bit DPCM Decode
		CSL_CAM_7_10_DPCM_DEC = (1 << 2),	///< 7-10 bit DPCM Decode
		CSL_CAM_7_12_DPCM_DEC = (1 << 3),	///< 7-12 bit DPCM Decode
		CSL_CAM_8_10_DPCM_DEC = (1 << 4),	///< 8-10 bit DPCM Decode
		CSL_CAM_8_12_DPCM_DEC = (1 << 5),	///< 8-12 bit DPCM Decode
		CSL_CAM_8_14_DPCM_DEC = (1 << 6),	///< 8-14 bit DPCM Decode
		CSL_CAM_8_16_DPCM_DEC = (1 << 7),	///< 8-16 bit DPCM Decode
		CSL_CAM_10_14_DPCM_DEC = (1 << 8),	///< 10-14 bit DPCM Decode
		CSL_CAM_10_16_DPCM_DEC = (1 << 9),	///< 10-16 bit DPCM Decode
		CSL_CAM_12_16_DPCM_DEC = (1 << 10),	///< 12-16 bit DPCM Decode
		CSL_CAM_FSP_DEC = (1 << 11),	///< FSP Decode (needed for JPEG)
	} CSL_CAM_DECODER_T;

/**
* CAM Parallel sync trigger
*****************************************************************************/
	typedef enum {
		CSL_CAM_SYNC_ACTIVE_LOW = (1 << 0),	///< Sync active low
		CSL_CAM_SYNC_ACTIVE_HIGH = (1 << 1),	///< Sync active high
		CSL_CAM_SYNC_EDGE_NEG = (1 << 2),	///< Sync to Negative edge
		CSL_CAM_SYNC_EDGE_POS = (1 << 3),	///< Sync to Positive edge
	} CSL_CAM_SYNC_TRIGGER_t;

/**
* CAM Parallel sync signal definition
*****************************************************************************/
	typedef enum {
		CSL_CAM_SYNC_DEFINES_ACTIVE = (1 << 0),	///< Sync signal defines active data
		CSL_CAM_SYNC_DEFINES_SYNC = (1 << 1),	///< Sync signal defined by sync signal
		CSL_CAM_SYNC_START_DATA = (1 << 2),	///< Sync signals start of active data
		CSL_CAM_SYNC_IGNORED = (1 << 3),	///< Sync signal ignored
	} CSL_CAM_SYNC_SIGNAL_t;

/**
* CAM Parallel sync mode
*****************************************************************************/
	typedef enum {
		CSL_CAM_SYNC_EXTERNAL = (1 << 0),	///< Sync uses H and V syncs
		CSL_CAM_SYNC_EMBEDDED = (1 << 1),	///< Sync uses embedded syncs (ITU BU.601)
	} CSL_CAM_SYNC_MODE_t;

/**
* CAM Parallel field mode
*****************************************************************************/
	typedef enum {
		CSL_CAM_FIELD_H_V = (1 << 0),	///< Field calculated from H and V syncs
		CSL_CAM_FIELD_EMBEDDED = (1 << 1),	///< Field calculated from embedded CCIR656
		CSL_CAM_FIELD_EVEN = (1 << 2),	///< Field is always even
		CSL_CAM_FIELD_ODD = (1 << 3),	///< Field is always odd
	} CSL_CAM_FMODE_t;

/**
* CAM Parallel field mode
*****************************************************************************/
	typedef enum {
		CSL_CAM_SCOPE_DISABLED = (1 << 0),	///< Scope mode disabled
		CSL_CAM_SCOPE_ENABLED = (1 << 1),	///< Scope mode enabled
	} CSL_CAM_SCOPE_t;

/**
* CAM Parallel clock sample edge
*****************************************************************************/
	typedef enum {
		CSL_CAM_CLK_EDGE_NEG = (1 << 0),	///< Sample Negative edge
		CSL_CAM_CLK_EDGE_POS = (1 << 1),	///< Sample Positive edge
	} CSL_CAM_CLK_EDGE_t;

	typedef struct {
		CSL_CAM_SYNC_MODE_t sync_mode;	///< Sync External or Embedded
		CSL_CAM_SYNC_SIGNAL_t hsync_signal;	///< How HSYNC defines active data
		CSL_CAM_SYNC_TRIGGER_t hsync_trigger;	///< How HSYNC is triggered (level, edge)
		CSL_CAM_SYNC_SIGNAL_t vsync_signal;	///< How VSYNC defines active data
		CSL_CAM_SYNC_TRIGGER_t vsync_trigger;	///< How VSYNC is triggered (level, edge)
		CSL_CAM_CLK_EDGE_t clock_edge;	///< Edge Clock is sampled
		CSL_CAM_PIXEL_SIZE_T bit_width;	///< bit width selection
		Int32 data_shift;	///< data shift (+) left shift  (-) right shift
		CSL_CAM_FMODE_t fmode;	///< field mode
		CSL_CAM_SCOPE_t smode;	///< scope mode
	} CSL_CAM_CPI_INTF_st_t;

/**
* CAM Clock Output Selection Settings
*****************************************************************************/
	typedef enum {
		CSL_CAM_CLK_12MHz,	///< 12 Mhz clock
		CSL_CAM_CLK_13MHz,	///< 13 Mhz clock
		CSL_CAM_CLK_24MHz,	///< 24 Mhz clock
		CSL_CAM_CLK_26MHz,	///< 26 Mhz clock
		CSL_CAM_CLK_48MHz,	///< 48 Mhz clock
		CSL_CAM_CLK_Disabled	///< No clock
	} CSL_CAM_CLK_SPEED_T;

/**
* CAM Callback Events
*****************************************************************************/
	typedef enum {
		CSL_CAM_LISR = (1 << 0),	///< Callback from LISR, Time critical, status and setting Start Address
		CSL_CAM_TASK = (1 << 1),	///< Callback from TASK, Not time critical
	} CSL_CAM_CB_EVENT_t;

/**
*
* CSL CAM Handle 
*
*****************************************************************************/
	typedef void *CSL_CAM_HANDLE;	///< csl cam handle

//the setup for this peripheral
	typedef struct {
// which input mode are we using?
		CSL_CAM_INTF_T intf;
// Which Camera port
		CSL_CAM_PORT_AFE_T afe_port;
// data input mode
		CSL_CAM_INPUT_MODE_t input_mode;
// Frame Timeout (ms)
		UInt32 frame_time_out;
//parallel input mode (CPI if available, else = NULL)
		CSL_CAM_CPI_INTF_st_t *p_cpi_intf_st;
	} CSL_CAM_INTF_CFG_st_t, *pCSL_CAM_INTF_CFG_st;

/**
* CAM Interface Control
*****************************************************************************/
	typedef struct {
// RX/TX settings
		UInt8 packet_timeout;	///< Packet Frame Timeout, 0=Disabled, n=# HS clocks
		Boolean line_start_suppress;	///< 1=Suppress Line Start Codes
		UInt8 burst_length;	///< Output Engine burst length (mod 2, 0->16)
		UInt8 burst_space;	///< Output Engine burst spacing (mod 2, 0->1024)
		UInt8 norm_pr;	///< Normal Priority signal 4-bit
		UInt8 panic_pr;	///< Panic Priority signal 4-bi
		UInt8 panic_thr;	///< Panic Threshold
		Boolean panic_enable;	///< Panic Enable
	} CSL_CAM_INTF_CNTRL_st_t, *pCSL_CAM_INTF_CNTRL_st;

/**
* CAM Input Control
*****************************************************************************/
	typedef struct {
		CSL_CAM_INPUT_MODE_t input_mode;	///< Input Mode
		UInt32 frame_time_out;	///< Frame time-out
//parallel input mode (CPI if available, else = NULL)
		CSL_CAM_CPI_INTF_st_t *p_cpi_intf_st;
	} CSL_CAM_INPUT_st_t, *pCSL_CAM_INPUT_st;

/**
* CAM Frame Control
*****************************************************************************/
	typedef struct {
		CSL_CAM_INTERRUPT_t int_enable;	///< Frame Interrupt Enables (CSL_CAM_INT_DISABLE|CSL_CAM_INT_FRAME_xxxx|CSL_CAM_INT_LINE_xxxx)
		CSL_CAM_CAPTURE_MODE_T capture_mode;	///< Frame capture mode
		UInt16 int_line_count;	///< Interrupt Line Count (if enabled)
		UInt32 capture_size;	///< Estimated Capture size in bytes
	} CSL_CAM_FRAME_st_t, *pCSL_CAM_FRAME_st;

/**
* CAM Pipeline 
*****************************************************************************/
	typedef struct {
		CSL_CAM_DECODER_T decode;	///< decoding mode
		CSL_CAM_PIXEL_SIZE_T unpack;	///< decode unpack 
		Boolean dec_adv_predictor;	///< enable decode advanced predictor
		CSL_CAM_ENCODER_T encode;	///< encoding mode
		CSL_CAM_PIXEL_SIZE_T pack;	///< encode pack 
		Boolean enc_adv_predictor;	///< enable encode advanced predictor
		UInt32 encode_blk_size;	///< encoding block size (multple of 8, <=512)
	} CSL_CAM_PIPELINE_st_t, *pCSL_CAM_PIPELINE_st;

/**
* CAM Buffer, can set each frame
*****************************************************************************/
	typedef struct {
		UInt32 start_addr;	///< (in) start address
		UInt32 size;	///< (in) buffer size in bytes
		UInt32 line_stride;	///< (in) line stride (CSL_CAM_IMAGE only)
		Boolean buffer_wrap_en;	///< (in) buffer wrap enable (circular)
		CSL_CAM_MEM_TYPE_T mem_type;	///< (in) memory type
	} CSL_CAM_BUFFER_st_t, *pCSL_CAM_BUFFER_st;

/**
* CAM Buffers, For setting Double Buffering
*****************************************************************************/
	typedef struct {
		CSL_CAM_BUFFER_st_t *image0Buff;
		CSL_CAM_BUFFER_st_t *image1Buff;
		CSL_CAM_BUFFER_st_t *data0Buff;
		CSL_CAM_BUFFER_st_t *data1Buff;
	} CSL_CAM_BUFFER_PTR_st_t, *pCSL_CAM_BUFFER_PTR_st;

/**
* CAM Buffer, can set each frame
*****************************************************************************/
	typedef struct {
		CSL_CAM_BUFFER_st_t buffer_st;	///< (out) buffer info
		UInt32 write_ptr;	///< (out) write pointer
		UInt16 bytes_per_line;	///< (out) bytes per line (CSL_CAM_IMAGE only)
		UInt16 lines_per_frame;	///< (out) lines per frame (CSL_CAM_IMAGE only)
	} CSL_CAM_BUFFER_STATUS_st_t, *pCSL_CAM_BUFFER_STATUS_st;

/**
* CAM Window, can set each frame
*****************************************************************************/
	typedef struct {
		Boolean enable;	///< Windowing Enable:  0 = Disable  1 = Enable
		UInt16 horizontal_start_byte;	///< horizontal start byte (must be even #) 
		UInt16 horizontal_size_bytes;	///< horizontal size in bytes (must be even #)
		UInt16 vertical_start_line;	///< vertical start line
		UInt16 vertical_size_lines;	///< vertical size in lines
	} CSL_CAM_WINDOW_st_t, *pCSL_CAM_WINDOW_st;

/**
* CAM Clock Output
*****************************************************************************/
	typedef struct {
		UInt8 clock_select;	///< Clock Device Select
		Boolean enable;	///< Clock Enable:  0 = Disable  1 = Enable
		CSL_CAM_CLK_SPEED_T speed;	///< Clock speed 
	} CSL_CAM_CLOCK_OUT_st_t, *pCSL_CAM_CLOCK_OUT_st;

/**
* CAM Image ID  Logical Channel/Packet Header Identifier
*   CCP=logical channel 0-7,   CSI=Packet Header 7:6=[VC] 5:0=[Data Type]
*****************************************************************************/
	typedef struct {
		UInt8 image_data_id0;	///< Image Data Identifier 0 (CCP=logical channel, CSI=Packet Data Identifier 7:6=[VC] 5:0=[Data Type] )
		UInt8 image_data_id1;	///< Image Data Identifier 1 CSI=Packet Data Identifier)
		UInt8 image_data_id2;	///< Image Data Identifier 2 (CSI=Packet Data Identifier)
		UInt8 image_data_id3;	///< Image Data Identifier 3 (CSI=Packet Data Identifier)
		UInt8 image_data_id4;	///< Image Data Identifier 4 (CSI=Packet Data Identifier)
		UInt8 image_data_id5;	///< Image Data Identifier 5 (CSI=Packet Data Identifier)
		UInt8 image_data_id6;	///< Image Data Identifier 6 (CSI=Packet Data Identifier)
		UInt8 image_data_id7;	///< Image Data Identifier 7 (CSI=Packet Data Identifier)
	} CSL_CAM_IMAGE_ID_st_t, *pCSL_CAM_IMAGE_ID_st;

/**
* CAM Embedded Data Capture
*****************************************************************************/
	typedef struct {
		CSL_CAM_INTERRUPT_t int_enable;	///< Data Interrupt Enables (CSL_CAM_INT_FRAME_END || CSL_CAM_INT_DATA)
		UInt8 line_count;	///< Embedded line count 0=No embedded data
		UInt8 data_id;	///< Packet Data ID 7:6=[VC] 5:0=Data Type
		CSL_CAM_PIXEL_SIZE_T data_size;	///< Data size
		Boolean fsp_decode_enable;	///< FSP Decode Enable:  0 = Disable  1 = Enable
	} CSL_CAM_DATA_st_t, *pCSL_CAM_DATA_st;

/**
* CAM Packet Header Capture
*****************************************************************************/
	typedef struct {
		Boolean capture_header_enable;	///< Enable packet header capture
		CSL_CAM_INTERRUPT_t int_enable;	///< Interrupt Enable (CSL_CAM_INT_PACKET || CSL_CAM_INT_DISABLE)
		Boolean packet_compare_enable;	///< Enable packet compare
		UInt8 data_id;	///< Packet Data ID 7:6=[VC] 5:0=Data Type
	} CSL_CAM_PACKET_CAPTURE_st_t;

/**
* CAM Packet Header Capture
*****************************************************************************/
	typedef struct {
		CSL_CAM_PACKET_CAPTURE_st_t packet_capture[MAX_PACKET_CAPTURE];
	} CSL_CAM_PACKET_st_t, *pCSL_CAM_PACKET_st;

/**
* CAM Lane Control 
*****************************************************************************/
	typedef struct {
		CSL_CAM_SELECT_t lane_select;	///< Lane selected
		CSL_CAM_LANE_CONTROL_t lane_control;	///< Lane control selection
		UInt8 param;	///< Lane control parameter if needed
	} CSL_CAM_LANE_CONTROL_st_t, *pCSL_CAM_LANE_CONTROL_st;

/**
* CAM Buffer, can set each frame
*****************************************************************************/
	typedef struct {
		UInt32 intr_status;	///< interrupt status
		UInt32 rx_status;	///< receiver status
		UInt32 image_addr;	///< image addr
		UInt32 image_size;	///< image size in bytes
		UInt32 image_stride;	///< line stride
		UInt32 data_addr;	///< embedded data addr
		UInt32 data_size;	///< embedded data size in bytes
		UInt32 data_stride;	///< line stride
		UInt32 raw_intr_status;	///< raw interrupt status
		UInt32 raw_rx_status;	///< raw receiver status
		UInt32 dropped_frames;	///< frames dropped
	} CSL_CAM_CB_st_t, *pCSL_CB_st;

/**
* CAM GPIO Control
*****************************************************************************/
	typedef struct {
		UInt32 select;	///< GPIO selected
		UInt32 enable;	///< GPIO on/off
	} CSL_CAM_GPIO_st_t, *pCSL_CAM_GPIO_st;

/**
* CAM Sensor Control
*****************************************************************************/
	typedef struct {
		UInt32 sensor_id;	///< sensor select
		Boolean gpio_ctrl;	///< select GPIO control
		CSL_CAM_GPIO_st_t sensor_gpio_ctrl_st;	///< GPIO settings if selected
		Boolean clk_ctrl;	///< select CLK control
		CSL_CAM_CLOCK_OUT_st_t sensor_clk_ctrl_st;	///< Clk setting if selected
	} CSL_CAM_SENSOR_CNTRL_st_t, *pCSL_CAM_SENSOR_CNTRL_st;

#if  0
//the setup for this peripheral
	typedef struct {
// which input mode are we using?
		CSL_CAM_INTF_T intf;
// Which Camera port
		CSL_CAM_PORT_AFE_T afe_port;
// data input mode
		CSL_CAM_INPUT_MODE_t input_mode;
// Data Lane Timings
		UInt32 hs_rx_delay;	// = 0
		UInt32 hs_settle_delay;	// = 0
		UInt32 hs_term_delay;	// = 5
// Image Id's    
		UInt32 image_ids;
// Interrupts Enabled
		CSL_CAM_INTERRUPT_t interrupts;
// Line Count if enabled (ms)
		UInt32 line_count;

// Set these pointers = NULL if do not need to be configured    
// Image Buffer Info
		CSL_CAM_BUFFER_st_t *image_buf_0;
		CSL_CAM_BUFFER_st_t *image_buf_1;
// Embedded Data Buffer Info
		CSL_CAM_BUFFER_st_t *data_buf_0;
		CSL_CAM_BUFFER_st_t *data_buf_1;
		CSL_CAM_PIPELINE_st_t *pipleline;
		CSL_CAM_DATA_st_t *embdd_data;
	} CSL_CAM_INTF_SETUP_st_t, *pCSL_CAM_INTF_SETUP_st;
#endif

#if (defined CSL_CAM_ENABLE_DBL_BUFFER)
	typedef void (*cslCamCB_t) (CSL_CAM_CB_st_t csl_cb_st, void *userdata);
#else
	typedef void (*cslCamCB_t) (UInt32 intr_status, UInt32 rx_status,
				    UInt32 image_addr, UInt32 image_size,
				    UInt32 raw_intr_status,
				    UInt32 raw_rx_status, void *userdata);
#endif
/******************************************************************************
 API
 *****************************************************************************/

//Init driver
	Int32 csl_cam_init(void);

//Exit driver
	Int32 csl_cam_exit(void);

//Driver info
	Int32 csl_cam_info(const char **driver_name, UInt32 * version_major,
			   UInt32 * version_minor);

//Open driver
	Int32 csl_cam_open(pCSL_CAM_INTF_CFG_st intfCfg,
			   CSL_CAM_HANDLE * cslCamH);
//Close driver
	Int32 csl_cam_close(CSL_CAM_HANDLE cslCamH);

//Driver control functions
	Int32 csl_cam_set_intf_control(CSL_CAM_HANDLE cslCamH,
				       pCSL_CAM_INTF_CNTRL_st intfCtrl);
	Int32 csl_cam_set_input_mode(CSL_CAM_HANDLE cslCamH,
				     pCSL_CAM_INPUT_st inputMode);
	Int32 csl_cam_set_input_addr(CSL_CAM_HANDLE cslCamH,
				     pCSL_CAM_BUFFER_st imageAddr_0,
				     pCSL_CAM_BUFFER_st imageAddr_1,
				     pCSL_CAM_BUFFER_st dataAddr);
	Int32 csl_cam_set_dbl_buf(CSL_CAM_HANDLE cslCamH,
				  CSL_CAM_BUFFER_PTR_st_t cslBuffers);
	Int32 csl_cam_set_frame_control(CSL_CAM_HANDLE cslCamH,
					pCSL_CAM_FRAME_st frameCtrl);
	Int32 csl_cam_set_image_type_control(CSL_CAM_HANDLE cslCamH,
					     pCSL_CAM_IMAGE_ID_st imageCtrl);
	Int32 csl_cam_set_data_type_control(CSL_CAM_HANDLE cslCamH,
					    pCSL_CAM_DATA_st dataCtrl);
	Int32 csl_cam_set_packet_control(CSL_CAM_HANDLE cslCamH,
					 pCSL_CAM_PACKET_st packetCtrl);
	Int32 csl_cam_set_lane_control(CSL_CAM_HANDLE cslCamH,
				       pCSL_CAM_LANE_CONTROL_st laneCtrl);
	Int32 csl_cam_set_pipeline_control(CSL_CAM_HANDLE cslCamH,
					   pCSL_CAM_PIPELINE_st pipelineCtrl);
	Int32 csl_cam_set_image_window(CSL_CAM_HANDLE cslCamH,
				       pCSL_CAM_WINDOW_st imageWindow);
	Int32 csl_cam_get_buffer_status(CSL_CAM_HANDLE cslCamH,
					CSL_CAM_SELECT_t status_select,
					pCSL_CAM_BUFFER_STATUS_st bufStatus);
	UInt32 csl_cam_get_intr_status(CSL_CAM_HANDLE cslCamH,
				       CSL_CAM_INTERRUPT_t * intStatus);
	UInt32 csl_cam_get_rx_status(CSL_CAM_HANDLE cslCamH,
				     CSL_CAM_RX_STATUS_t * rxStatus);
	Int32 csl_cam_get_lane_status(CSL_CAM_HANDLE cslCamH,
				      CSL_CAM_SELECT_t lane,
				      CSL_CAM_LANE_STATUS_t * laneStatus);
	Int32 csl_cam_set_clk(pCSL_CAM_CLOCK_OUT_st clockSet);
	Int32 csl_cam_register_display(CSL_CAM_HANDLE cslCamH);
	Int32 csl_cam_trigger_capture(CSL_CAM_HANDLE cslCamH);
	Int32 csl_cam_rx_start(CSL_CAM_HANDLE cslCamH);
	Int32 csl_cam_rx_stop(CSL_CAM_HANDLE cslCamH);
	Int32 csl_cam_channel_stop(CSL_CAM_HANDLE cslCamH);
	Int32 csl_cam_reset(CSL_CAM_HANDLE cslCamH, CSL_CAM_RESET_t mode);
	Int32 csl_cam_register_event_callback(CSL_CAM_HANDLE cslCamH,
					      CSL_CAM_CB_EVENT_t event,
					      cslCamCB_t callback,
					      void *userdata);
	Int32 csl_cam_get_handle(CSL_CAM_HANDLE * cslCamH);	// for testing only!!!
	Int32 csl_cam_set_sensor_control(CSL_CAM_HANDLE cslCamH,
					 pCSL_CAM_SENSOR_CNTRL_st sensorCtrl);

#ifdef __cplusplus
}
#endif
#endif				// CSL_CAM_H
