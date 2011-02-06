/*******************************************************************************************
Copyright 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement 
governing use of this software, this software is licensed to you under the 
terms of the GNU General Public License version 2, available at 
http://www.gnu.org/copyleft/gpl.html (the "GPL"). 

Notwithstanding the above, under no circumstances may you combine this software 
in any way with any other Broadcom software provided under a license other than 
the GPL, without Broadcom's express prior written consent.
*******************************************************************************************/

//****************** Copyright 2003 Mobilink Telecom, Inc. *********************
//
// Description:  This file encapsulates the shared memory
//
//******************************************************************************

#ifndef	_INC_SHAREDMEM_H_
#define	_INC_SHAREDMEM_H_

#include "shared.h"
#include <linux/broadcom/chip_version.h>
//--#include "dsp_feature_def.h"
typedef struct
{
	UInt16	header[2];
	UInt16	entry0;
	UInt16	entry1;
	UInt16	entry2;
	UInt16	entry3;
	UInt16  entry4;
	UInt16	findex0;
	UInt16	findex1;
	UInt16	findex2;
	UInt16	gain0;
	UInt16	gain1;
	UInt16	gain2;
	UInt16	spare;
#ifdef NEW_FRAME_ENTRY
	UInt16	rficdata0;
	UInt16	rficdata1;
	UInt16	rficdata2;
	UInt16	rficdata3;
	UInt16	rficdata4;
#endif
} TCM_Shared_FrameEntry_New_t;

void SHAREDMEM_Init( void );			// Initialize Shared Memory

void SHAREDMEM_ReadBuffer16(			// Read the received message
	UInt8 index,						// Rx buffer to use
	UInt8 *bfi_flag,					// FALSE, message error
	UInt8 *frame_index,					// Last frame index of message
	UInt8 octets,						// message size (in octets)
	UInt16 *contents						// Pointer to received message storage
	);

void SHAREDMEM_ReadBuffer(				// Read the received message
	UInt8 index,						// Rx buffer to use
	UInt8 *bfi_flag,					// FALSE, message error
	UInt8 *bec,							// Bit error count
	UInt8 *frame_index,					// Last frame index of message
	Int8 *adj,							// Frame adjust (for SCH only)
	Int16 *otd,							// Observed time difference, half-bits (for SCH only)
	RxCode_t *code,						// Decoding mode
	UInt8 *octets,						// # of octets in message
	UInt8 *cn_db16,						// Cn (in dB/16)
	UInt8 *bl_var,						// BL_VAR
	UInt8 *contents						// Pointer to received message storage
	);

void SHAREDMEM_WriteBuffer(				// Post a message to send
	UInt8 index,						// Tx buffer to use
	TxCode_t code,						// Encoding mode
	UInt8 *contents						// Pointer to message to send
	);

void SHAREDMEM_WriteBufferRach(				// Post a message to send
	UInt8 index,						// Tx buffer to use
	TxCode_t code,						// Encoding mode
	UInt8 *contents						// Pointer to message to send
	);

void SHAREDMEM_WriteGPRSBuffer(			// Post a message to send
	UInt8 index,						// Tx buffer to use
	TxCode_t coding_scheme,				// tx data encoding mode
	UInt8 *contents						// Pointer to message to send
	);

UInt16 SHAREDMEM_ReadMonStatus(			// Read the neighbor power status
	UInt8	buff_id,					// Monitor Buffer Id
	UInt8	frame_index					// Frame index to get status
	);

UInt8 SHAREDMEM_ReadFrameIndex( void );	// Return the current frame index

void SHAREDMEM_PostCmdQ(				// Post an entry to the command queue
	CmdQ_t *cmd_msg						// Entry to post
	);
void SHAREDMEM_PostCmdQ_LISR(				// Post an entry to the command queue
	CmdQ_t *cmd_msg						// Entry to post
	);

#ifdef DSP_FAST_COMMAND
void SHAREDMEM_PostFastCmdQ(			// Post an entry to the fast command queue
	CmdQ_t *cmd_msg						// Entry to post
	);
void SHAREDMEM_PostFastCmdQ_LISR(			// Post an entry to the fast command queue
	CmdQ_t *cmd_msg						// Entry to post
	);
#endif

Boolean SHAREDMEM_ReadStatusQ(			// Read an entry from the command queue
	StatQ_t *status_msg					// Entry from queue
	);									// TRUE, if entry is available

void SHAREDMEM_ReadMeasReport(			// Read the measurement report
	UInt16 *rxqual_full,				// Pointer to array of four elements
	UInt16 *rxqual_sub,					// Pointer to array of four elements
	UInt16 *rxlev_full,					// Pointer to array of four elements
	UInt16 *rxlev_sub					// Pointer to array of four elements
	);

void SHAREDMEM_WriteTestPoint(			// Write to a test point
	UInt8	tp,							// Test Point
	UInt16	value						// Test Point Value
	);

SharedMem_t *SHAREDMEM_GetSharedMemPtr( void );
										// Return pointer to shared memory
Int16 *SHAREDMEM_ReadSNR( void);

UInt16 *SHAREDMEM_ReadMetric( void);

UInt16 *SHAREDMEM_ReadBFI_BEC( void);
void SHAREDMEM_RefreshAMREntry( UInt8 cmr);

void SHAREDMEM_GetEdgeRxBufPtr(UInt8 index, EDGE_RxBuf_t **SharedMemRxBuf);

void SHAREDMEM_GetEdgeTxBufPtr(UInt8 index, EDGE_TxBuf_t **SharedMemTxBuf);

#ifdef	ARM_HS_VOCODER
UInt16 *SHAREDMEM_GetHsDecBuffer_inptr(	void );		// Return the HS decoder input ptr
Int16 *SHAREDMEM_GetHsDecBuffer_outptr( void );			// Return the HS decoder output ptr
Int16 *SHAREDMEM_GetHsEncBuffer_inptr(	void );		// Return the HS encoder input ptr
UInt16 *SHAREDMEM_GetHsEncBuffer_outptr( void );			// Return the HS encoder output ptr
#endif

void SHAREDMEM_SetAMRParms(
	UInt16 icm,
	UInt16 acs,
	UInt16 direction
	);

#if (defined(_BCM2133_) && CHIP_REVISION >= 12) || (CHIPVERSION >= CHIP_VERSION(BCM2152,10))
void SHAREDMEM_WriteDL_AMR_Speech(UInt8 rx_type,UInt8 frame_type, UInt32 *data, UInt8 length, Boolean amr_if2_enable, UInt16 *softA, UInt32 *deciphbit);
#else
void SHAREDMEM_WriteDL_AMR_Speech(UInt8 rx_type,UInt8 frame_type, UInt32 *data, UInt8 length, Boolean amr_if2_enable);
#endif
void SHAREDMEM_ReadUL_AMR_Speech(UInt32 **speech_data, Boolean amr_if2_enable);
UInt16 SHAREDMEM_ReadUL_AMR_TxType(void);
UInt16 SHAREDMEM_ReadUL_AMR_Mode(void);
void SHAREDMEM_DLAMR_Speech_Init(void);
void SHAREDMEM_ULAMR_Speech_Init(void);
void SHAREDMEM_UMTSSetOffset(UInt16	qbc);
#if ((defined(_BCM2153_) && CHIP_REVISION >= 52) || (defined(BCM2153_FAMILY_BB_CHIP_BOND)))
void SHAREDMEM_WriteDL_VoIP_Data(UInt16 codec_type, UInt16 *pSrc);
UInt8 SHAREDMEM_ReadUL_VoIP_Data(UInt16 codec_type, UInt16 *pDst);
#endif //#if ((defined(_BCM2153_) && CHIP_REVISION >= 52) || (defined(BCM2153_FAMILY_BB_CHIP_BOND)))

#if CHIPVERSION >= CHIP_VERSION(BCM2132,32) /* BCM2132C3 and later */

#define CLEAR_ENTRY_UMTS(entry)				{ (entry).entry4 &= ~0x4000; }
#define CLEAR_ENTRY_UMTS_NO_FN_INC(entry)	{ (entry).entry4 &= ~0x2000; }

#endif //CHIPVERSION >= CHIP_VERSION(BCM2132,33) /* BCM2132C3 and later */

#endif	// _INC_SHAREDMEM_H_



