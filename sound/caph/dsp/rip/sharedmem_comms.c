/****************************************************************************
 * ©1999-2008 Broadcom Corporation
* This program is the proprietary software of Broadcom Corporation and/or its licensors, and may only be used, duplicated, 
* modified or distributed pursuant to the terms and conditions of a separate, written license agreement executed between 
* you and Broadcom (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants no license 
* (express or implied), right to use, or waiver of any kind with respect to the Software, and Broadcom expressly reserves all 
* rights in and to the Software and all intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU 
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE 
* ALL USE OF THE SOFTWARE.	
* 
* Except as expressly set forth in the Authorized License,
* 
* 1.	 This program, including its structure, sequence and organization, constitutes the valuable trade secrets of Broadcom,
*		  and you shall use all reasonable efforts to protect the confidentiality thereof, and to use this information only in connection with your
*		  use of Broadcom integrated circuit products.
* 
* 2.	 TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO 
*		  PROMISES, REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO THE SOFTWARE. 
*		  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A 
*		  PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE 
*		  TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3.	 TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, 
*		  INCIDENTAL, SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR USE OF OR 
*		  INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN 
*		  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY 
*		  NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
*
****************************************************************************/

#include <string.h>
#include "assert.h"
#include "mobcom_types.h"
#include "chip_version.h"
#include "ms.h"
#include "dsp_feature_def.h"
#include "consts.h"
#include "shared.h"
#include "sharedmem_comms.h"
#include "sharedmem.h"
#ifdef WIN32
#else
#include "memmap.h"
#include "ostypes.h"
#include "timer.h"
#endif
#include "irqctrl.h"
#include "xassert.h"
#include "sysparm.h"
//#include "phytypes.h"
#include "rficisr.h"
#include "data.h"
#include "amrproc.h"
#include "phprim.h"
#include "frameutil.h"

// CQ MobC00098656, Andy Shen, 12/30/09
// Need include file for MAX_MULTISLOT_CLASS definition
#include "ms_config.h"
// CQ MobC00098656, Andy Shen, 12/30/09

#define ENABLE_LOGGING
#include "log.h"
#include "log_sig_code.h" 

//******************************************************************************
// Local Definitions
//******************************************************************************
static const UInt8 RXCODE_OCTET_TABLE[] =
{
	0,									// 0x00 = RXCODE_FR	
	0,									// 0x01 = RXCODE_EFR	
	30,									// 0x02 = RXCODE_D9_6	
	15,									// 0x03 = RXCODE_D4_8	
	9,									// 0x04 = RXCODE_D2_4	
	37,									// 0x05 = RXCODE_D14_4
	34,									// 0x06 = RXCODE_CS2	
	40,									// 0x07 = RXCODE_CS3	
	54,									// 0x08 = RXCODE_CS4	
	23,									// 0x09 = RXCODE_CCCH	
	23,									// 0x0a = RXCODE_CS1	
	23,									// 0x0b = RXCODE_FACCH
	4,									// 0x0c = RXCODE_SCH	
	0,									// 0x0d
	0,									// 0x0e
	0,									// 0x0f
	6,									// 0x10 = RXCODE_RATSCCH
	0,									// 0x11
	0,									// 0x12
	0,									// 0x13
	0,									// 0x14
	23									// 0x15	= RXCODE_EPC_SACCH
};

static const UInt8 TXCODE_OCTET_TABLE[] =
{
	4,									// 0x00 = TXCODE_RACH_HO
	23,									// 0x01 = TXCODE_CCCH
	23,									// 0x02 = TXCODE_EPC_SACCH
	23,									// 0x03 = TXCODE_FACCH
	30,									// 0x04 = TXCODE_D9_6		
	15,									// 0x05 = TXCODE_D4_8		
	9,									// 0x06 = TXCODE_D2_4		
	37,									// 0x07 = TXCODE_D14_4	
	4,									// 0x08 = TXCODE_PRACH	
	23,									// 0x09 = TXCODE_CS1		
	34,									// 0x0a = TXCODE_CS2		
	40,									// 0x0b = TXCODE_CS3		
	54,									// 0x0c = TXCODE_CS4		
	0,									// 0x0d
	0,									// 0x0e
	0,                                  // 0x0f
	0,									// 0x10
	6									// 0x11 = TXCODE_RATSCCH
};

//******************************************************************************
// Local Variables
//******************************************************************************
Shared_FrameEntry_t shared_frame_copy[ 128 ];

#ifdef DEVELOPMENT_ONLY
// Adjust GSM RXLEV
int gsm_adjust_db = 0;   // Adjust GSM measurement RXlev
int random_gsm_adjust = 0;   // Random GSM Adjust flag
int random_gsm_start_delay = 0;
int random_gsm_stop_delay = 0;
int random_gsm_mask = 0xff;
#endif
//#if defined(STACK_wedge)
//static UInt8 amr_length[9]={12,13,15,17,19,20,26,31,5}; // Bytes
//#endif

static Unpaged_SharedMem_t	*unpaged_shared_mem;
static SharedMem_t	*shared_mem; 
static PAGE14_SharedMem_t *pg14_shared_mem;

void SHAREDMEM_InitComms(void)
{
	unpaged_shared_mem = SHAREDMEM_GetUnpagedSharedMemPtr();
	shared_mem = SHAREDMEM_GetSharedMemPtr();
	pg14_shared_mem = SHAREDMEM_GetPage14SharedMemPtr();
	RFICISR_Init();
}
//******************************************************************************
//
// Function Name:	SHAREDMEM_WriteSleepEntry
//
// Description: Writes a sleep frame entry to shared memory.
//
// Notes:
//
//******************************************************************************
void SHAREDMEM_WriteSleepEntry(				// Write a frame entry
	UInt8 frame_index,					// Selected frame entry
	UInt16 sleep_frames
	)
{
	//checked with Ronish, once detecting sleep frame, DSP only cares the sleep numbers.
	tcm_shared_frame[frame_index].header[0] = shared_mem->shared_frame[frame_index].header[0] = 0; //FRAMEPATT_SLEEP;
	tcm_shared_frame[frame_index].entry0 = shared_mem->shared_frame[frame_index].entry0 = sleep_frames - 1;
#if (defined UMTS) //Clear the RXMODE and TXMODE for IRAT 
	tcm_shared_frame[frame_index].entry1 = shared_mem->shared_frame[frame_index].entry1 = 0; 
	tcm_shared_frame[frame_index].entry2 = shared_mem->shared_frame[frame_index].entry2 = 0;
#endif
	//clear nullpage flag
	tcm_shared_frame[frame_index].entry4 = shared_mem->shared_frame[frame_index].entry4 = 0;
}

//******************************************************************************
//
// Function Name:	SHAREDMEM_WriteEntry
//
// Description: Writes a frame entry to shared memory.
//
// Notes:
//
//******************************************************************************

void SHAREDMEM_WriteEntry(				// Write a frame entry
	UInt8 frame_index,					// Selected frame entry
	FrameEntry_t *frame_entry			// Pointer to frame entry
	)
{

		memcpy( &shared_mem->shared_frame[frame_index], frame_entry,sizeof( Shared_FrameEntry_t ) );	
		memcpy( &tcm_shared_frame[frame_index], frame_entry,sizeof( FrameEntry_t ) );	


}
//******************************************************************************
//
// Function Name:	SHAREDMEM_GetFrameIndexFromBuffer
//
// Description:		Read the frame index from the RX Buffer
//
// Notes:
//
//******************************************************************************

UInt8 SHAREDMEM_GetFrameIndexFromBuffer(				// Read the received message
	UInt8 buf_index,						// Rx buffer to use
	UInt8 buf_offset
	)
{
	RxBuf_t *buf;
	EDGE_RxBuf_t *edge_buf;

	assert( buf_index <= RX_BUFF_CNT );

	if( buf_index >= EGPRS_BUF_START)
	{
		if( DATA_IsDataServiceOn())
		{
			buf = &pg14_shared_mem->shared_rx_buff_high.U.gprs_rx_buf[buf_index - 8 ];

		}
		else
		{
			if( pg14_shared_mem->shared_rx_buff_high.Present == EGPRS_RXBUF)
			{
				edge_buf= &pg14_shared_mem->shared_rx_buff_high.U.egprs_rx_buf[ buf_index+buf_offset - 8 ];
				return ( (UInt8)GET_RXMSG_INDEX( edge_buf ));
			}
			else
				buf = &pg14_shared_mem->shared_rx_buff_high.U.gprs_rx_buf[buf_index+buf_offset - 8 ];
		}
	}
	else
		buf = &pg14_shared_mem->shared_rx_buff[ buf_index ];


	
	return( (UInt8)GET_RXMSG_INDEX( buf ));
}

//******************************************************************************
//
// Function Name:	SHAREDMEM_ReadBufferIndexOTD
//
// Description:		Read the frame index and OTD  from received message
//
// Notes:
//
//******************************************************************************

void SHAREDMEM_ReadBufferIndexOTD(				// Read the received message
	UInt8 index,						// Rx buffer to use
	UInt8 *frame_index,					// Last frame index of message
	Int16 *otd							// Observed time difference, half-bits (for SCH only)
)
{
	RxBuf_t *buf;

	assert( index < 8 );	// right now, OTD only updated in idle rx buffer

	buf = &pg14_shared_mem->shared_rx_buff[ index ];
	
	*frame_index = (UInt8)GET_RXMSG_INDEX( buf );
#ifndef EDGE_TEST
	xassert( *frame_index < SYSPARM_GetFrameEntryCnt(), (*frame_index)|(SYSPARM_GetFrameEntryCnt()<<16) );
#endif
	*otd = (Int16)GET_RXMSG_OTD( buf );
}

//******************************************************************************
//
// Function Name:	SHAREDMEM_ReadBuffer
//
// Description:		Read the received message
//
// Notes:
//
//******************************************************************************

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
	)
{
	RxBuf_t *buf;

	assert( index <= RX_BUFF_CNT );

	if( index >= 8)
		buf = &pg14_shared_mem->shared_rx_buff_high.U.gprs_rx_buf[index - 8 ];
	else
		buf = &pg14_shared_mem->shared_rx_buff[ index ];

	*bfi_flag = (UInt8)GET_RXMSG_BFI( buf );

	*bec = (UInt8)GET_RXMSG_BEC( buf );

//	Don't corrupt BEC b4 we read it out.
#ifndef WIN32
	SET_RXMSG_BEC_BFI( buf, 1);
#endif
	
	*frame_index = (UInt8)GET_RXMSG_INDEX( buf );
#ifndef EDGE_TEST
	xassert( *frame_index < SYSPARM_GetFrameEntryCnt(), (*frame_index)|(SYSPARM_GetFrameEntryCnt()<<16) );
#endif
	*adj = (Int8)GET_RXMSG_ADJUST( buf );
	*otd = (Int16)GET_RXMSG_OTD( buf );
	*code = (RxCode_t)GET_RXMSG_CODE( buf );
	*cn_db16 = (UInt8)GET_RXMSG_CN_DB16( buf );
	*bl_var = (UInt8)GET_RXMSG_BL_VAR( buf );
	*octets = RXCODE_OCTET_TABLE[ *code ];

#if (defined(ATHENA_FPGA_VERSION))
	if (index == 1)
	{
		L1_LOGV4("PCH 		BLOCK BFI, CODE, OTD, BEC ", *bfi_flag, *code, *otd, *bec);
	}
	else if (index == 2)
	{
		L1_LOGV4("PPCH	 	BLOCK BFI, CODE, OTD, BEC ", *bfi_flag, *code, *otd, *bec);
	}
	else if (index == 3)
	{
		L1_LOGV4("SDCCH	  	BLOCK BFI, CODE, OTD, BEC ", *bfi_flag, *code, *otd, *bec);
		L1_LOGV4("PTCCH	   	BLOCK BFI, CODE, OTD, BEC ", *bfi_flag, *code, *otd, *bec);
	}
	else if (index == 4)
	{
		L1_LOGV4("EXT_BCCH	BLOCK BFI, CODE, OTD, BEC ", *bfi_flag, *code, *otd, *bec);
		L1_LOGV4("DATA0	    BLOCK BFI, CODE, OTD, BEC ", *bfi_flag, *code, *otd, *bec);
	}
	else if (index == 5)
	{
		L1_LOGV4("AGCH	   	BLOCK BFI, CODE, OTD, BEC ", *bfi_flag, *code, *otd, *bec);
		L1_LOGV4("PAGCH	   	BLOCK BFI, CODE, OTD, BEC ", *bfi_flag, *code, *otd, *bec);
	}
	else if (index == 6)
	{
		L1_LOGV4("BCCH	   	BLOCK BFI, CODE, OTD, BEC ", *bfi_flag, *code, *otd, *bec);
		L1_LOGV4("PBCCH	   	BLOCK BFI, CODE, OTD, BEC ", *bfi_flag, *code, *otd, *bec);
	}
	else if (index == 7)
	{
		L1_LOGV4("CBCH	    BLOCK BFI, CODE, OTD, BEC ", *bfi_flag, *code, *otd, *bec);
		L1_LOGV4("SACCH	    BLOCK BFI, CODE, OTD, BEC ", *bfi_flag, *code, *otd, *bec);
	}
#endif





#ifdef DSP_FEATURE_LITTLE_ENDIAN 

	//contents/buf->report.msg should be UInt16 aligned, if not, ARM will assert.
	//use UInt16 operation to speed up memcpy
	memcpy((UInt16 *)contents,
			 (UInt16 *)buf->report.msg,
			 (*octets+1)&0xFE);

#else 
	{
		UInt8 i;
		UInt8 j;
		UInt8 len;
		UInt16 temp;

		len = *octets >> 1; 

		for ( i = 0, j = 0 ; i < len ; i++ )
		{
			temp = buf->report.msg[i];					// hold the half-word from shared RAM
			contents[j++] = (UInt8)(temp >> 8);	// swap the high-order
			contents[j++] = (UInt8)temp;		// and low-order quarter-words
		}

		if ( *octets & 0x01 )
		{
			contents[j] = (UInt8)(buf->report.msg[i] >> 8);
		}
	}
#endif	//#ifdef DSP_FEATURE_LITTLE_ENDIAN 

	_TP_OUTPUT( LA_GENERAL, TP_RX_BUFF_INDEX = index;	\
							memcpy( (void *)&TP_READ_BUFF, (void *)contents, *octets);	\
							TP_RX_BUFF_BFI = *bfi_flag;	\
							TP_RX_BUFF_CODE	= *code;	\
							TP_RX_BUFF_OTD = *otd;	\
							TP_RX_BUFF_ADJ = *adj; )
}

//******************************************************************************
//
// Function Name:	SHAREDMEM_ReadBufferEnh
//
// Description:		Read the received message and Rxlev and SNR
//
// Notes:
//
//******************************************************************************

void SHAREDMEM_ReadBufferEnh(			// Read the received message
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
	UInt8 *contents,					// Pointer to received message storage
	UInt16 *rxlev,						// Pointer to 4 rxlev storage
	Int16 *snr							// Pointer to 4 SNR storage
	)
{
	RxBuf_t *buf;
	UInt8 i;

	assert( index < RX_BUFF_CNT );

	if( index >= 8)
		buf = &pg14_shared_mem->shared_rx_buff_high.U.gprs_rx_buf[index - 8 ];
	else
		buf = &pg14_shared_mem->shared_rx_buff[ index ];

	*bfi_flag = (UInt8)GET_RXMSG_BFI( buf );

	*bec = (UInt8)GET_RXMSG_BEC( buf );

//	Don't corrupt BEC b4 we read it out.
#ifndef WIN32
	SET_RXMSG_BEC_BFI( buf, 1);
#endif
	
	*frame_index = (UInt8)GET_RXMSG_INDEX( buf );
#ifndef EDGE_TEST
	assert( *frame_index < SYSPARM_GetFrameEntryCnt() );
#endif
	*adj = (Int8)GET_RXMSG_ADJUST( buf );
	*otd = (Int16)GET_RXMSG_OTD( buf );
	*code = (RxCode_t)GET_RXMSG_CODE( buf );
	*cn_db16 = (UInt8)GET_RXMSG_CN_DB16( buf );
	*bl_var = (UInt8)GET_RXMSG_BL_VAR( buf );
	*octets = RXCODE_OCTET_TABLE[ *code ];
#ifdef DSP_FEATURE_LITTLE_ENDIAN 
	memcpy((UInt16 *)contents,
			 (UInt16 *)buf->report.msg,
			 (*octets+1)&0xFE);
#else 
	{
		UInt8 j;
		UInt8 len;
		UInt16 temp;
		
		len = *octets >> 1;
		
		for ( i = 0, j = 0 ; i < len ; i++ )
		{
			temp = buf->report.msg[i];					// hold the half-word from shared RAM
			contents[j++] = (UInt8)(temp >> 8);	// swap the high-order
			contents[j++] = (UInt8)temp;		// and low-order quarter-words
		}

		if ( *octets & 0x01 )
		{
			contents[j] = (UInt8)(buf->report.msg[i] >> 8);
		}
	}
#endif //#ifdef DSP_FEATURE_LITTLE_ENDIAN 

	for ( i = 0; i < 4; i++ )
	{
		if ( *bfi_flag == FALSE )
			rxlev[i] = buf->report.measurement.rxlev[i] | MON_STATUS_IS_RSSR_FLAG;
		else
			rxlev[i] = buf->report.measurement.rxlev[i];
		snr[i] = buf->report.measurement.SNR[i];
	}

	_TP_OUTPUT( LA_GENERAL, TP_RX_BUFF_INDEX = index;	\
							memcpy( (void *)&TP_READ_BUFF, (void *)contents, *octets);	\
							TP_RX_BUFF_BFI = *bfi_flag;	\
							TP_RX_BUFF_CODE	= *code;	\
							TP_RX_BUFF_OTD = *otd;	\
							TP_RX_BUFF_ADJ = *adj; )
}


//******************************************************************************
//
// Function Name:	SHAREDMEM_WriteBuffer
//
// Description:		Post a message to send
//
// Notes:
//
//******************************************************************************

void SHAREDMEM_WriteBuffer(				// Post a message to send
	UInt8 index,						// Tx buffer to use
	TxCode_t code,						// Encoding mode
	UInt8 *contents						// Pointer to message to send
	)
{
	UInt8 octets;
// coverity[var_decl]
	TxBuf_t	*buf;
// coverity[var_decl]
	EDGE_TxBuf_t *ebuf;

	// set to avoid compiler varning
	ebuf = &unpaged_shared_mem->shared_tx_buff_high.U.egprs_tx_buf[0 ];
	buf  = &unpaged_shared_mem->shared_tx_buff_high.U.gprs_tx_buf[0 ];
	octets = TXCODE_OCTET_TABLE[ code ];
	if( index >= 8)
	{
		if (PHYFRAME_GetTBFMode() == AT_EGPRS)
		{
			unpaged_shared_mem->shared_tx_buff_high.Present = EGPRS_TXBUF;
			ebuf = &unpaged_shared_mem->shared_tx_buff_high.U.egprs_tx_buf[index - 8 ];
		}
		else
		{
			unpaged_shared_mem->shared_tx_buff_high.Present = GPRS_TXBUF;
			buf = &unpaged_shared_mem->shared_tx_buff_high.U.gprs_tx_buf[index - 8 ];
		}

	}
	else
	{
		buf = &unpaged_shared_mem->shared_tx_buff[ index ];
	}
	
	
   if ((PHYFRAME_GetTBFMode() == AT_EGPRS)&&(index >= 8))
   {
	   SET_TXMSG_CODE( ebuf, code );
	   ebuf->seq_id = FRAMEUTIL_GetFreeFrameIndex();
	   
#ifdef DSP_FEATURE_LITTLE_ENDIAN 
		memcpy((UInt16 *)ebuf->msg,
			 (UInt16 *)contents,
			 (octets+1)&0xFE);
#else 
	{
	UInt8 i;
	UInt8 j;
	UInt8 len;

	len = octets >> 1;
	
		for ( i = 0, j = 0; i <= len; i++, j+=2 )
		{
			ebuf->msg[i] = (((contents[j] << 8) & 0xff00) | (contents[j + 1] & 0x00ff));
		}

		if ( octets & 0x01 )
		{
			ebuf->msg[i] = ((contents[j] << 8) & 0xff00);
		}
	}
#endif //#ifdef DSP_FEATURE_LITTLE_ENDIAN 
		
   }
   else
   {
	   SET_TXMSG_CODE( buf, code );
	   if( index >= 8)
	   	buf->seq_id = FRAMEUTIL_GetFreeFrameIndex();
	   else
	   	buf->seq_id = get_seq_id();

#ifdef DSP_FEATURE_LITTLE_ENDIAN 
		memcpy((UInt16 *)buf->msg,
			 (UInt16 *)contents,
			 (octets+1)&0xFE);
#else 
		{
			UInt8 i;
			UInt8 j;
			UInt8 len;

			len = octets >> 1;

		   for ( i = 0, j = 0; i <= len; i++, j+=2 )
			{
				buf->msg[i] = (((contents[j] << 8) & 0xff00) | (contents[j + 1] & 0x00ff));
			}

			if ( octets & 0x01 )
			{
				buf->msg[i] = ((contents[j] << 8) & 0xff00);
			}
		}
#endif //#ifdef DSP_FEATURE_LITTLE_ENDIAN 

   }

	_TP_OUTPUT( LA_GENERAL, TP_TX_BUFF_INDEX = index; \
							memcpy( (void *)&TP_WRITE_BUFF, (void *)contents, octets ); \
							TP_TX_BUFF_CODE	= code;	)
}


void SHAREDMEM_WriteBufferRach(				// Post a message to send
	UInt8 index,						// Tx buffer to use
	TxCode_t code,						// Encoding mode
	UInt8 *contents						// Pointer to message to send
	)
{
	UInt8 octets;
// coverity[var_decl]
	TxBuf_t	*buf;
// coverity[var_decl]
	EDGE_TxBuf_t *ebuf;
	UInt8 i;
	UInt8 j;
	UInt8 len;

	// set to avoid compiler varning
	ebuf = &unpaged_shared_mem->shared_tx_buff_high.U.egprs_tx_buf[0 ];
	buf  = &unpaged_shared_mem->shared_tx_buff_high.U.gprs_tx_buf[0 ];
	octets = TXCODE_OCTET_TABLE[ code ];
	if( index >= 8)
	{
		if (PHYFRAME_GetTBFMode() == AT_EGPRS)
		{
			unpaged_shared_mem->shared_tx_buff_high.Present = EGPRS_TXBUF;
			ebuf = &unpaged_shared_mem->shared_tx_buff_high.U.egprs_tx_buf[index - 8 ];

		}
		else
		{
			unpaged_shared_mem->shared_tx_buff_high.Present = GPRS_TXBUF;
			buf = &unpaged_shared_mem->shared_tx_buff_high.U.gprs_tx_buf[index - 8 ];
		}

	}
	else
	{
		buf = &unpaged_shared_mem->shared_tx_buff[ index ];
	}
	
	len = octets >> 1;
	
   if ((PHYFRAME_GetTBFMode() == AT_EGPRS)&&(index >= 8))
   {
	   SET_TXMSG_CODE( ebuf, code );
	   ebuf->seq_id = FRAMEUTIL_GetFreeFrameIndex();
	   
		for ( i = 0, j = 0; i <= len; i++, j+=2 )
		{
			ebuf->msg[i] = (((contents[j] << 8) & 0xff00) | (contents[j + 1] & 0x00ff));
		}

		if ( octets & 0x01 )
		{
			ebuf->msg[i] = ((contents[j] << 8) & 0xff00);
		}
		
   }
   else
   {
	   SET_TXMSG_CODE( buf, code );
	   buf->seq_id = FRAMEUTIL_GetFreeFrameIndex();

	   for ( i = 0, j = 0; i <= len; i++, j+=2 )
		{
			buf->msg[i] = (((contents[j] << 8) & 0xff00) | (contents[j + 1] & 0x00ff));
		}

		if ( octets & 0x01 )
		{
			buf->msg[i] = ((contents[j] << 8) & 0xff00);
		}

   }

	_TP_OUTPUT( LA_GENERAL, TP_TX_BUFF_INDEX = index; \
							memcpy( (void *)&TP_WRITE_BUFF, (void *)contents, octets ); \
							TP_TX_BUFF_CODE	= code;	)
}

//******************************************************************************
//
// Function Name:	SHAREDMEM_WriteGPRSBuffer
//
// Description:		Post a message to send
//
// Notes:
//
//******************************************************************************

void SHAREDMEM_WriteGPRSBuffer(			// Post a message to send
	UInt8 index,						// Tx buffer to use
	TxCode_t coding_scheme,				// tx data encoding mode
	UInt8 *contents						// Pointer to message to send
	)
{
	UInt8 octets;
	TxBuf_t	*buf;

	octets = TXCODE_OCTET_TABLE[ coding_scheme ];

	buf = &unpaged_shared_mem->shared_tx_buff[ index ];


	SET_TXMSG_CODE( buf, coding_scheme );

#ifdef DSP_FEATURE_LITTLE_ENDIAN 
	memcpy((UInt16 *)buf->msg,
			 (UInt16 *)contents,
			 (octets+1)&0xFE);
#else 
	{
		UInt8	i, j, len;

		len = octets >> 1;
		
		for ( i = 0, j = 0; i <= len ; i++, j+=2 )
		{
			buf->msg[i] = (((contents[j] << 8) & 0xff00) | (contents[j + 1] & 0x00ff));
		}

		if ( octets & 0x01 ) buf->msg[i] = ((contents[j] << 8) & 0xff00);
	}
#endif //#ifdef DSP_FEATURE_LITTLE_ENDIAN 
}

//******************************************************************************
//
// Function Name:	SHAREDMEM_ReadMonStatus
//
// Description:		Read the power monitor status
//
// Notes:
//
//******************************************************************************

UInt16 SHAREDMEM_ReadMonStatus(			// Read the power monitor status
	UInt8 buff_id,						// Monitor Buffer Id(0-3 are ncell monitoring;
										// 0, 4,5,6 are for interference measurement for GPRS)
	UInt8 frame_index					// Frame index to get status
	)
{
	UInt16 result;
#if defined (DEVELOPMENT_ONLY) && defined (PLATFORM_TEST)
	int rxlev_temp;
	static int random_gsm_tick=0;
#endif
#ifndef EDGE_TEST	
	assert( frame_index < SYSPARM_GetFrameEntryCnt() );
#endif

	switch( buff_id )
	{
		case 0:
			result = unpaged_shared_mem->shared_mon1[ frame_index ];
			break;

		case 1:
			result = unpaged_shared_mem->shared_mon2[ frame_index ];
			break;

		case 2:
			result = unpaged_shared_mem->shared_mon3[ frame_index ];
			break;

		case 3:
			result = unpaged_shared_mem->shared_mon1_1[ frame_index ];
			break;

		case 4:
			result = unpaged_shared_mem->shared_mon1_2[ frame_index ];
			break;

		case 5:
			result = unpaged_shared_mem->shared_mon1_3[ frame_index ];
			break;

#if MAX_MULTISLOT_CLASS>=30 && MAX_MULTISLOT_CLASS<=34
		case 6:
			result = unpaged_shared_mem->shared_mon1_4[ frame_index ];
			break;
#endif
		default :
			result = 0;
			assert( 0 );
			break;
	}
#if defined (DEVELOPMENT_ONLY) && defined (PLATFORM_TEST)
	if ( IS_MON_STATUS_VALID( result ) )
	{
		if (random_gsm_adjust>0)
		{
			random_gsm_tick++;
			if ((random_gsm_tick & random_gsm_mask)==0)		// mask 8 is about 30 seconds
			{
				if (random_gsm_start_delay>0)
					random_gsm_start_delay -=1;			// initial delay
				else
				{
					gsm_adjust_db = TIMER_GetValue() & 0x3f;   // random positive adjust 8 db to 39 db
					if (gsm_adjust_db & 1)
						gsm_adjust_db = 8 + (gsm_adjust_db>>1);
					else
						gsm_adjust_db = -8 -(gsm_adjust_db >>1);	// random negative adjust -8 db to -39 db

					if (random_gsm_stop_delay>0)
						random_gsm_stop_delay -=1;
					if (random_gsm_stop_delay==0)
					{
						random_gsm_adjust=0;		// stop the loop
						gsm_adjust_db=0;
					}	
				}
			 CAL_LOGV4("RandomAdjust GSM Mask - Start - Stop - Adjust",random_gsm_mask, random_gsm_start_delay,random_gsm_stop_delay,gsm_adjust_db);
			}
		}
		rxlev_temp = (RxLev_t)READ_MON_STATUS_VALUE( result ) + gsm_adjust_db;
		if(rxlev_temp < 0) rxlev_temp =0;
		result &= 0xE000;
		result |= rxlev_temp;
	}
#endif
	return(result);
}

//******************************************************************************
//
// Function Name:	SHAREDMEM_ReadFrameIndex
//
// Description:		Return the current frame index
//
// Notes:
//
//******************************************************************************

UInt8 SHAREDMEM_ReadFrameIndex()		// Return the current frame index
{
	UInt8	result;

#ifndef EDGE_TEST	
	assert( unpaged_shared_mem->shared_frame_out < SYSPARM_GetFrameEntryCnt() );
#endif
	result = (UInt8)unpaged_shared_mem->shared_frame_out;
	return(result);			   
	
}

//******************************************************************************
//
// Function Name:	SHAREDMEM_IsNullPageDetected
//
// Description:		Null paging is not implemented on the ML2029
//					so this function always returns false.
//
// Notes:
//
//******************************************************************************


Boolean SHAREDMEM_IsNullPageDetected( void )
{
	// return FALSE;

	return shared_mem->shared_null_pg_burst_activity_mask != 0;
}


//******************************************************************************
//
// Function Name:	SHAREDMEM_ReadMeasReport
//
// Description:		Read the measurement report
//
// Notes:
//
//******************************************************************************

void SHAREDMEM_ReadMeasReport(			// Read the measurement report
	UInt16 *rxqual_full,				// Pointer to array of four elements
	UInt16 *rxqual_sub,					// Pointer to array of four elements
	UInt16 *rxlev_full,					// Pointer to array of four elements
	UInt16 *rxlev_sub					// Pointer to array of four elements
	)
{
	UInt16 slot;

	for ( slot = 0 ; slot < MAX_RX_SLOTS ; slot++ )
	{
		rxqual_full[ slot ] = shared_mem->shared_rxqual_full[ slot ];
		rxqual_sub[  slot ] = shared_mem->shared_rxqual_sub[  slot ];
		rxlev_full[  slot ] = shared_mem->shared_rxlev_full[  slot ];
		rxlev_sub[   slot ] = shared_mem->shared_rxlev_sub[   slot ];
	}
}

void SHAREDMEM_GetGprsRxBufPtr(UInt8 index, RxBuf_t **SharedMemRxBuf)
{
	*SharedMemRxBuf = (RxBuf_t *) &pg14_shared_mem->shared_rx_buff_high.U.gprs_rx_buf[index];
}


void SHAREDMEM_GetEdgeRxBufPtr(UInt8 index, EDGE_RxBuf_t **SharedMemRxBuf)
{
	*SharedMemRxBuf = (EDGE_RxBuf_t *)&pg14_shared_mem->shared_rx_buff_high.U.egprs_rx_buf[ index ];
}

void SHAREDMEM_GetEdgeTxBufPtr(UInt8 index, EDGE_TxBuf_t **SharedMemTxBuf)
{
	*SharedMemTxBuf = (EDGE_TxBuf_t *)&unpaged_shared_mem->shared_tx_buff_high.U.egprs_tx_buf[ index ];
	unpaged_shared_mem->shared_tx_buff_high.Present = EGPRS_TXBUF;
}


UInt16 SHAREDMEM_EdgePresent()

{
	return (pg14_shared_mem->shared_rx_buff_high.Present);
}


Int16 *SHAREDMEM_ReadSNR( void)
{
	return(shared_mem->shared_SNR );			
}


UInt16 *SHAREDMEM_ReadMetric( void)
{
	return(shared_mem->shared_vit_metric );
}

UInt16 *SHAREDMEM_ReadBFI_BEC( void)
{
	return(shared_mem->shared_bec_bfi );
}
//******************************************************************************
//
// Function Name:	SHAREDMEM_RefreshAMREntry
//
// Description:		Read the received message
//
// Notes:
//
//******************************************************************************

void SHAREDMEM_RefreshAMREntry( UInt8 cmr)
{
	int i;
	i = SHAREDMEM_ReadFrameIndex();
	i = i-2;  // This is for DSP SMC advance reading frame entry --  kaiping
	if( i < 0) i += SYSPARM_GetFrameEntryCnt();

	while(i != FRAMEUTIL_GetFreeFrameIndex())
	{
		SET_ENTRY_AMR_CMR(shared_mem->shared_frame[i], cmr);
		i++;
		if( i >= SYSPARM_GetFrameEntryCnt()) i -= SYSPARM_GetFrameEntryCnt();
	}

}

//******************************************************************************
//
// Function Name:	SHAREDMEM_RefreshAMREntry_Confirm
//
// Description:		Refresh the CMR value and confirm whether it is refreshed successfully.
//
// Notes:
//
//******************************************************************************

Boolean SHAREDMEM_RefreshAMREntry_Confirm( UInt8 cmr)
{
	int i;
	Boolean cmr_refreshed =FALSE;
	
	i = SHAREDMEM_ReadFrameIndex();
//	i = i-1;  // This is for DSP SMC advance reading frame entry --  kaiping
	if( i < 0) i += SYSPARM_GetFrameEntryCnt();

	while(i != FRAMEUTIL_GetFreeFrameIndex())
	{
		SET_ENTRY_AMR_CMR(shared_mem->shared_frame[i], cmr);
		if( GET_ENTRY_CMI_CMR_U(shared_mem->shared_frame[i]))
		{
			cmr_refreshed = TRUE;
			L1_LOGV("CMR index", i);
		}
		i++;
		if( i >= SYSPARM_GetFrameEntryCnt()) i -= SYSPARM_GetFrameEntryCnt();
	}
	return cmr_refreshed;
}

//******************************************************************************
//
// Function Name:	SHAREDMEM_SetAMRParms
//
// Description:		Set AMR code set parameters
//
// Notes:
//
//******************************************************************************

void SHAREDMEM_SetAMRParms( 
	UInt16 icm,
	UInt16 acs,
	UInt16 direction
	)
{
	if ( direction == AMR_CONFIG_UP )
	{
		shared_mem->shared_AMR_ulink_ICM_ACS = (icm << 8) | acs;
	}
	else if ( direction == AMR_CONFIG_DOWN )
	{
		shared_mem->shared_AMR_dlink_ICM_ACS = (icm << 8) | acs;
	}
	else
	{
		xassert(FALSE, direction);
	}
}

void SHAREDMEM_SetRatscchMarker_DisableFlag( Boolean flag )
{
	shared_mem->shared_ahs_ratscch_marker_disable = flag;
}


void SHAREDMEM_UMTSSetOffset(UInt16	qbc)
{
	shared_mem->shared_qbc_umts_adjust = qbc;
}
