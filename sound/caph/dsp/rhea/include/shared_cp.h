//****************************************************************************
//*
//*     Copyright (c) 2007 Broadcom Corporation
//*           All Rights Reserved
//*
//*           Broadcom Corporation
//*           16215 Alton Parkway
//*           P.O. Box 57013
//*           Irvine, California 92619-7013
//*
//*        This program is the proprietary software of Broadcom Corporation
//*        and/or its licensors, and may only be used, duplicated, modified
//*        or distributed pursuant to the terms and conditions of a separate,
//*        written license agreement executed between you and
//*        Broadcom (an "Authorized License").
//*
//*        Except as set forth in an Authorized License, Broadcom grants no
//*        license (express or implied), right to use, or waiver of any kind
//*        with respect to the Software, and Broadcom expressly reserves all
//*        rights in and to the Software and all intellectual property rights
//*        therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT
//*        TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
//*        BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
//*
//*        Except as expressly set forth in the Authorized License,
//*
//*        1. This program, including its structure, sequence and
//*           organization, constitutes the valuable trade secrets of
//*           Broadcom, and you shall use all reasonable efforts to protect
//*           the confidentiality thereof, and to use this information only
//*           in connection with your use of Broadcom integrated circuit products.
//*
//*        2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
//*           "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
//*           REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY,
//*           OR OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
//*           DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
//*           NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
//*           ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
//*           CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
//*           OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
//*
//*        3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
//*           OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
//*           SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF
//*           OR IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE
//*           SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF
//*           SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY
//*           PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER.
//*           THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
//*           ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
//*

#ifndef    _INC_SHARED_CP_H_
#define    _INC_SHARED_CP_H_

#include "types.h"
#include "consts.h"

/**
 * \defgroup CP_Shared_Memory CP_Shared_Memory
 * @{
 */

//******************************************************************************
// The RIP definition of the shared memory simply places each variable in a
//        special data section.
//***********************************************************************************************************
// THE DEFINES ARE JUST FOR TAGS ONLY. THERE IS NO PAGING IN TLIII MEMORY ARCH.
// PLEASE DO NOT USE THEM AS PAGING CONCEPT
//           DO NOT FORCE DEFINES TO HARD CODED ADDRESSES 
// MODEM
//        SEC_CMD_STATUS
//        SEC_GEN_MODEM
//        SEC_MST_SAIC
//        SEC_PARAMET_MODEM
//        SEC_SMC
//        SEC_AFC_AGC_RFIC
//    AUDIO
//        SEC_GEN_AUDIO 
//        SEC_NS 
//        SEC_NLP 
//        SEC_ECHO 
//        SEC_VPU
//        SEC_AUX_AUDIO 
// 
//    DEBUG 
//        SEC_DIAGNOSIS     
//        SEC_DSP_GEN_DEBUG     
//
//
//***********************************************************************************************************
#ifdef RIP
// Modem
#define SHARED_SEC_CMD_STATUS          __attribute__( ( section( ".DSECT shared_sec_cmd_status") ) )          
#define SHARED_SEC_GEN_MODEM           __attribute__( ( section( ".DSECT shared_sec_gen_modem") ) )          
#define SHARED_SEC_MST_SAIC            __attribute__( ( section( ".DSECT shared_sec_mst_saic") ) )          
#define SHARED_SEC_PARAMET_MODEM       __attribute__( ( section( ".DSECT shared_sec_paramet_modem") ) )      
#define SHARED_SEC_SMC                 __attribute__( ( section( ".DSECT shared_sec_smc") ) )                    
#define SHARED_SEC_AGC_AFC_RFIC        __attribute__( ( section( ".DSECT shared_sec_afc_agc_rfic") ) )      
// Audio                                    
#define SHARED_SEC_GEN_AUDIO           __attribute__( ( section( ".DSECT shared_sec_gen_audio") ) )          
// Debug
#define SHARED_SEC_DIAGNOS             __attribute__( ( section( ".DSECT shared_sec_diagnos") ) )          
#define SHARED_SEC_DSP_DEBUG           __attribute__( ( section( ".DSECT shared_sec_dsp_debug") ) )          

#    undef EXTERN
#    ifdef RIP_EXTERN_DECLARE
#        define EXTERN
#    else
#        define    EXTERN    extern
#    endif
#endif	// RIP
 

#ifdef MSP
#define EXTERN
#define    SHARED_SEC_CMD_STATUS        
#define    SHARED_SEC_GEN_MODEM        
#define    SHARED_SEC_MST_SAIC            
#define    SHARED_SEC_PARAMET_MODEM    
#define    SHARED_SEC_SMC                  
#define    SHARED_SEC_AGC_AFC_RFIC        
#define    SHARED_SEC_GEN_AUDIO        
#define    SHARED_SEC_DIAGNOS            
#define    SHARED_SEC_DSP_DEBUG   
#endif

//******************************************************************************
// Shared memory RFIC constants
//******************************************************************************
#define NULL_PAGING_BLOCK       ((UInt16) 1  )  // Last paging block was a NULL
#define VALID_PAGING_BLOCK      ((UInt16) 0  )  // Last paging block possibly contained a valid page

//*******************************************************************************
//    Shared memory CTM constants
//*******************************************************************************
#define        BAUDOT_TX_ZERO      ((UInt16) 0x0000)
#define        BAUDOT_TX_BYPASS    ((UInt16) 0x0100)
#define        BAUDOT_TX_ON        ((UInt16) 0x0200)

#define        CTM_TX_ZERO         ((UInt16) 0)
#define        CTM_TX_BYPASS       ((UInt16) 1)
#define        CTM_TX_ON           ((UInt16) 2)


#define FAST_CMDQ_SIZE         ((UInt16) 16    )    // Number of INT1 fast command queue entries
#define CMDQ_SIZE              ((UInt16) 128   )     // Number of command queue entries
#define STATUSQ_SIZE           ((UInt16) 32    )    // Number of status queue entries

// Frame Entry Constants
#define ENTRY_SLEEP					0
#define ENTRY_NORMAL				1
#define ENTRY_MS					1

#define ENTRY_HEADER_INT			(1 << 11)

// Masks for Frame Entry 0
#define E0_MR_ON					(1 << 15)
#define E0_TAFT						(1 << 14)
#define E0_DTX						(1 << 13)

#define E0_MON_ON					(1 << 0)

// Definitions for RX Mode
#define ENTRY_RXMODE_NOP			0
#define ENTRY_RXMODE_SYNC			1
#define ENTRY_RXMODE_START			2
#define ENTRY_RXMODE_SRCH			3
#define ENTRY_RXMODE_SCH			4
#define ENTRY_RXMODE_CCH			5
#define ENTRY_RXMODE_CCH_MON		6
#define ENTRY_RXMODE_CCH_MON_MON	7
#define ENTRY_RXMODE_TCH			8
#define ENTRY_RXMODE_TCH_MON		9
#define ENTRY_RXMODE_MON			10
#define ENTRY_RXMODE_MON_MON		11
#define ENTRY_RXMODE_MON_MON_MON	12
#define ENTRY_RXMODE_SRCH_END		13
#define ENTRY_RXMODE_SCH_EOTD_IDL	14		//Note: only used if EOTD is defined!!!
#define ENTRY_RXMODE_SCH_EOTD_DED	15		//Note: only used if EOTD is defined!!!

#define EGPRS_BUF_START				8		// EDGE/GPRS logical buffer start number
#define HSCSD_BUF_START 			13 		// HSCSD     logical buffer start number

#define ENTRY_RXBUF_SCH				0
#define ENTRY_RXBUF_PCH				1
#define ENTRY_RXBUF_FACCH			1
#define ENTRY_RXBUF_PPCH			2
#define ENTRY_RXBUF_SDCCH			3
#define ENTRY_RXBUF_PTCCH			3
#define ENTRY_RXBUF_EXT_BCCH		4
#define ENTRY_RXBUF_DATA0			4
#define ENTRY_RXBUF_AGCH			5
#define ENTRY_RXBUF_PAGCH			5
#define ENTRY_RXBUF_BCCH			6
#define ENTRY_RXBUF_PBCCH			6
#define ENTRY_RXBUF_CBCH			7
#define ENTRY_RXBUF_SACCH			7
#define	ENTRY_RXBUF_EOTD_IDL		4		//Note: only used if EOTD is defined!!!
#define	ENTRY_RXBUF_EOTD_DED		5		//Note: only used if EOTD is defined!!!

#define ENTRY_RXBUF_TCH0			EGPRS_BUF_START
#define ENTRY_RXBUF_TCH1			EGPRS_BUF_START+1
#define ENTRY_RXBUF_TCH2			EGPRS_BUF_START+2
#define ENTRY_RXBUF_TCH3			EGPRS_BUF_START+3
#define ENTRY_RXBUF_SACCH0			HSCSD_BUF_START
#define ENTRY_RXBUF_SACCH1			HSCSD_BUF_START+1
#define ENTRY_RXBUF_SACCH2			HSCSD_BUF_START+2
#define ENTRY_RXBUF_SACCH3			HSCSD_BUF_START+3
#define ENTRY_RXBUF_PDTCH0			EGPRS_BUF_START
#define ENTRY_RXBUF_PDTCH1			EGPRS_BUF_START+1
#define ENTRY_RXBUF_PDTCH2			EGPRS_BUF_START+2
#define ENTRY_RXBUF_PDTCH3			EGPRS_BUF_START+3
#define ENTRY_RXBUF_PDTCH4			EGPRS_BUF_START+4

#define ENTRY_RXBUF_SPEECH			128	 //Support release'99 enhanced measurement to indicate SNR and rxlev ready for speech TCH.

#define ENTRY_RXBUF_SACCH_M			255

#define ENTRY_TXMODE_NOP			0
#define ENTRY_TXMODE_CCH			1
#define ENTRY_TXMODE_AB				2
#define ENTRY_TXMODE_TCH			3
#define ENTRY_TXMODE_PRACH			4
#define ENTRY_TXMODE_AB1 			5
#define ENTRY_TXMODE_AB2 			6


#define ENTRY_TXBUF_RACH			0		
#define ENTRY_TXBUF_PRACH			0
#define ENTRY_TXBUF_HO				0
#define ENTRY_TXBUF_FACCH			1
#define ENTRY_TXBUF_SACCH			2
#define ENTRY_TXBUF_SDCCH			3
#define ENTRY_TXBUF_PTCCH			2
#define ENTRY_TXBUF_DATA0			4

#define ENTRY_TXBUF_TCH0			EGPRS_BUF_START
#define ENTRY_TXBUF_TCH1			EGPRS_BUF_START+1
#define ENTRY_TXBUF_TCH2			EGPRS_BUF_START+2
#define ENTRY_TXBUF_TCH3			EGPRS_BUF_START+3
#define ENTRY_TXBUF_SACCH0			HSCSD_BUF_START
#define ENTRY_TXBUF_SACCH1			HSCSD_BUF_START+1
#define ENTRY_TXBUF_SACCH2			HSCSD_BUF_START+2
#define ENTRY_TXBUF_SACCH3			HSCSD_BUF_START+3

#define ENTRY_TXBUF_PDTCH0			EGPRS_BUF_START
#define ENTRY_TXBUF_PDTCH1			EGPRS_BUF_START+1
#define ENTRY_TXBUF_PDTCH2			EGPRS_BUF_START+2
#define ENTRY_TXBUF_PDTCH3			EGPRS_BUF_START+3
#define ENTRY_TXBUF_RATSCCH         5

#define ENTRY_RXBUF_RATSCCH         5

// Masks for Frequency Entries

#define ENTRY_GAIN_NOM				8
#define	ENTRY_DCS1800				( 1 << 14 )
#define ENTRY_BCCH_MON				( 1 << 15 )

#define	TRACK_DATA_SIZE			sizeof(	Shared_TrackData_t)		// Size of tracking data buffer
#define	SACCH_TRACK_DATA_SIZE	sizeof(	Shared_SacchTrackData_t)// Size of sacch tracking data buffer



// EOTD IQ read modes
#define EOTD_IQMODE0				0		//Note: only used if EOTD is defined!!!
#define EOTD_IQMODE1				1		//Note: only used if EOTD is defined!!!



typedef enum
{
	TONE_DTMF_0		= 0,	// 0x00 - 
	TONE_DTMF_1,			// 0x01 - 
	TONE_DTMF_2,			// 0x02 - 
	TONE_DTMF_3,			// 0x03 - 
	TONE_DTMF_4,			// 0x04 - 
	TONE_DTMF_5,			// 0x05 - 
	TONE_DTMF_6,			// 0x06 - 
	TONE_DTMF_7,			// 0x07 - 
	TONE_DTMF_8,			// 0x08 - 
	TONE_DTMF_9,			// 0x09 - 
	TONE_DTMF_STAR,			// 0x0A - 
	TONE_DTMF_POND,			// 0x0B - 
	TONE_BUSY		= 16,	// 0x10 - 
	TONE_CONGESTION,		// 0x11 - 
	TONE_RADIO_ACKN,		// 0x12 - 
	TONE_ERROR,				// 0x13 - 
	TONE_RINGING,			// 0x14 - 
	TONE_NO_RADIO_PATH,		// 0x15 - 
	TONE_CALL_WAITING,		// 0x16 - 
	TONE_DIALING			// 0x17 - 
} OakTone_t;


typedef enum
{
	WFS_MODE_660  	= 0,		//WFS6.60
    WFS_MODE_885	= 1, 		//WFS8.85    
    WFS_MODE_1265	= 2			//WFS12.65
}Mode_WFS_t;


//******************************************************************************
// Handy Shared memory macros for Frame entry access
//******************************************************************************
// Frame Entry Get Operations
//===========================
#define GET_ENTRY_USF(entry)			( ( (entry).header[0]  >> 15 ) & 0x01 )
#define GET_ENTRY_MS(entry)				( ( (entry).header[0]  >> 14 ) & 0x01 )
#define GET_ENTRY_DTM(entry)			( ( (entry).header[0]  >> 13 ) & 0x01 )
#define GET_ENTRY_FRAME(entry)			( ( (entry).header[0]  >> 12 ) & 0x01 )
#define GET_ENTRY_INT_FLAG(entry)		( ( (entry).header[0]  >> 11 ) & 0x01 )
#define GET_ENTRY_SIZ(entry)			( ( (entry).header[0]  >> 10 ) & 0x01 )
#define GET_ENTRY_EXT(entry)			( ( (entry).header[0]  >>  9 ) & 0x01 )
#define GET_ENTRY_CRYPTO_MODE(entry)	( ( (entry).header[0]  >>  6 ) & 0x07 )
#define GET_ENTRY_FN_HI(entry)			( ( (entry).header[0]        ) & 0x3f )

#define GET_ENTRY_FN_LO(entry)			( ( (entry).header[1]        )        )

#define GET_ENTRY_MR(entry)				( ( (entry).entry0 >> 15 ) & 0x01 )
#define GET_ENTRY_TAFT(entry)			( ( (entry).entry0 >> 14 ) & 0x01 )
#define GET_ENTRY_DTX(entry)			( ( (entry).entry0 >> 13 ) & 0x01 )
#define GET_ENTRY_CODE0(entry)			( ( (entry).entry0 >>  8 ) & 0x1f )
#define GET_ENTRY_NEW_SLEEP_FLAG(entry)	( ( (entry).entry0 >> 13 ) & 0x01 )
#define GET_ENTRY_SLEEP_SLOT(entry)		( ( (entry).entry0 >> 10 ) & 0x07 )
#define GET_ENTRY_DELAY(entry)			( ( (entry).entry0       ) & 0x3ff )
#define GET_ENTRY_SRCH_CNT(entry)		( ( (entry).entry0 >>  2 ) & 0x3f )
#define GET_ENTRY_TSC(entry)			( ( (entry).entry0 >>  5 ) & 0x07 )
#define GET_ENTRY_SLOT(entry)			( ( (entry).entry0 >>  2 ) & 0x07 )
#define GET_ENTRY_RXTCH(entry)			( ( (entry).entry0 >>  1 ) & 0x01 )
#define GET_ENTRY_MON_ON(entry)			( ( (entry).entry0       ) & 0x01 )

#define GET_ENTRY_RXMODE(entry)			( ( (entry).entry1 >> 11 ) & 0x1f )
#define GET_ENTRY_RXBUF(entry)			( ( (entry).entry1 >>  8 ) & 0x07 )
#define GET_ENTRY_USFONLY5(entry)		( ( (entry).entry1 >>  7 ) & 0x01 )
#define GET_ENTRY_USFONLY4(entry)		( ( (entry).entry1 >>  6 ) & 0x01 )
#define GET_ENTRY_USFONLY3(entry)		( ( (entry).entry1 >>  5 ) & 0x01 )
#define GET_ENTRY_USFONLY2(entry)		( ( (entry).entry1 >>  4 ) & 0x01 )
#define GET_ENTRY_USFONLY1(entry)		( ( (entry).entry1 >>  3 ) & 0x01 )
#define GET_ENTRY_USFONLY(entry)		( ( (entry).entry1 >>  2 ) & 0x01 )
#define GET_ENTRY_RXBURST(entry)		( ( (entry).entry1       ) & 0x03 )

#define GET_ENTRY_TXMODE(entry)			( ( (entry).entry2 >> 11 ) & 0x1f )
#define GET_ENTRY_TXBUF(entry)			( ( (entry).entry2 >>  8 ) & 0x07 )

#define GET_ENTRY_EPC_MR(entry)			( ( (entry).entry2 >> 6 ) & 0x01 )
#define GET_ENTRY_RX3(entry)			( ( (entry).entry2 >> 5 ) & 0x01 )
#define GET_ENTRY_NORX0(entry)			( ( (entry).entry2 >> 4 ) & 0x01 )
#define GET_ENTRY_PPCH(entry)			( ( (entry).entry2 >> 3 ) & 0x01 )
#define GET_ENTRY_PDCH(entry)			( ( (entry).entry2 >> 2 ) & 0x01 )
#define GET_ENTRY_TXBURST(entry)		( ( (entry).entry2      ) & 0x03 )

#define GET_ENTRY_BCCH1(entry)			( ( (entry).entry3 >> 15 ) & 0x01 )
#define GET_ENTRY_BCCH2(entry)			( ( (entry).entry3 >> 14 ) & 0x01 )
#define GET_ENTRY_BCCH3(entry)			( ( (entry).entry3 >> 13 ) & 0x01 )
//#define GET_ENTRY_DTMSPHSLOT(entry)		( ( (entry).entry3 >> 10 ) & 0x07 )
#define GET_ENTRY_CMI_CMR_U(entry)		( ( (entry).entry3 >> 11 ) & 0x01 )
#define GET_ENTRY_CMI_CMC_D(entry)		( ( (entry).entry3 >> 10 ) & 0x01 )
#define GET_ENTRY_NEW_AMR_DLINK(entry)	( ( (entry).entry3 >> 9  ) & 0x01 )
#define GET_ENTRY_NEW_AMR_ULINK(entry)	( ( (entry).entry3 >> 8  ) & 0x01 )
#define GET_ENTRY_AMR_CMR(entry)		( ( (entry).entry3 >> 6  ) & 0x03 )
#define GET_ENTRY_NEW_SERV_FLAG(entry)	( ( (entry).entry3 >> 5  ) & 0x01 )
#define GET_ENTRY_CELL(entry)			( ( (entry).entry3       ) & 0x1f )

#define GET_ENTRY_NULL_PG(entry)		( ( (entry).entry4 >> 15 ) & 0x01 )
#define GET_ENTRY_UMTS(entry)			( ( (entry).entry4 >> 14 ) & 0x01 )
#define GET_ENTRY_UMTS_NO_FN_INC(entry)	( ( (entry).entry4 >> 13 ) & 0x01 )
#define GET_ENTRY_UMTS_FIRST_SRCH(entry)( ( (entry).entry4 >> 12 ) & 0x01 )				
#define GET_ENTRY_UMTS_SLOTS(entry)		( ( (entry).entry4 >> 8  ) & 0x0f )				
#define GET_ENTRY_DATAMODE_IND(entry)	( ( (entry).entry4 >> 7  ) & 0x01 )
#define GET_ENTRY_MEAS_INT(entry)		( ( (entry).entry4 >> 6  ) & 0x01 )
#define GET_ENTRY_CVALUE_SLOT(entry)	( ( (entry).entry4 >> 3  ) & 0x07 )
#define GET_ENTRY_SERV_SLOT(entry)		( ( (entry).entry4       ) & 0x07 )

// Frame Entry Set Operations
//===========================
#define SET_ENTRY_USF(entry)			( (entry).header[0] |= (1 << 15) )
#define SET_ENTRY_MS(entry)				( (entry).header[0] |= (1 << 14) )
#define SET_ENTRY_DTM(entry)			( (entry).header[0] |= (1 << 13) )
#define SET_ENTRY_INT(entry)			( (entry).header[0] |= ENTRY_HEADER_INT )
#define CLEAR_ENTRY_INT(entry)			( (entry).header[0] &= ~ENTRY_HEADER_INT )
#define SET_ENTRY_SIZ(entry)			( (entry).header[0] |= (1 << 10) )
#define SET_ENTRY_EXT(entry)			( (entry).header[0] |= (1 <<  9) )
#define SET_ENTRY_CRYPTO_MODE(entry, m)	{( (entry).header[0] &= 0xfe3f );\
										( (entry).header[0] |= ( (m) << 6 ) );}
  
#define SET_ENTRY_FN(entry, fn)			{( (entry).header[0] &= 0xffc0 );				\
										( (entry).header[0] |= (((fn) >> 16) & 0x3f));	\
										( (entry).header[1] = (UInt16)((fn) & 0xffff) );}

#define SET_ENTRY_MR(entry)				( (entry).entry0 |= E0_MR_ON )
#define CLEAR_ENTRY_MR(entry)			( (entry).entry0 &= ( ~E0_MR_ON ))
#define SET_ENTRY_TAFT(entry)			( (entry).entry0 |= E0_TAFT )
#define SET_ENTRY_DTX(entry)			( (entry).entry0 |= E0_DTX )
#define CLEAR_ENTRY_DTX(entry)			( (entry).entry0 &= (~E0_DTX) )
#define SET_ENTRY_CODE0(entry, m)		{( (entry).entry0 &= 0xe0ff );		 \
										( (entry).entry0 |= ( (m) << 8 ) );}
#define SET_ENTRY_TSC(entry, tsc)		{( (entry).entry0 &= 0xff1f );    \
										( (entry).entry0 |= (tsc)  << 5 );}

#define SET_ENTRY_SLOT(entry, slot)		{(entry).entry0 &= ~(7 << 2); (entry).entry0 |= (slot) << 2 ;}

#define SET_ENTRY_NEW_SLEEP_FLAG(entry, b)	{( (entry).entry0 &= 0xdfff ); ( (entry).entry0 |= (b) << 13 );}
#define SET_ENTRY_SLEEP_SLOT(entry, slot)	{( (entry).entry0 &= 0xe3ff );		 \
										( (entry).entry0 |= ( (slot) << 10 ) );}
#define SET_ENTRY_DELAY(entry, delay)	{(entry).entry0 &= ~0x3ff; (entry).entry0 |= (delay);}  
#define SET_ENTRY_SRCH_CNT(entry, cnt)	{(entry).entry0 &= 0xff03; ( (entry).entry0 |= (cnt)  << 2 );}

#define SET_ENTRY_RXTCH(entry, b)		{( (entry).entry0 &= 0xfffd );		\
										( (entry).entry0 |= (b) << 1 );}	
#define SET_ENTRY_MON(entry)			( (entry).entry0 |= E0_MON_ON )
#define CLR_ENTRY_MON(entry)			( (entry).entry0 &= ~E0_MON_ON )


#define SET_ENTRY_RXMODE(entry, m)		{( (entry).entry1 &= 0x07ff );		\
										( (entry).entry1 |= ((m) << 11) ); }
#define SET_ENTRY_RXBUF( entry, b )		{( (entry).entry1 &= 0xf8ff );		\
										( (entry).entry1 |= (b) << 8 );}	

#define SET_ENTRY_USFONLY(entry)		(entry).entry1 |= 0x0004
#define SET_ENTRY_USFONLY1(entry)		(entry).entry1 |= 0x0008
#define SET_ENTRY_USFONLY2(entry)		(entry).entry1 |= 0x0010
#define SET_ENTRY_USFONLY3(entry)		(entry).entry1 |= 0x0020
#define SET_ENTRY_USFONLY4(entry)		(entry).entry1 |= 0x0040
#define SET_ENTRY_USFONLY5(entry)		(entry).entry1 |= 0x0080

#define SET_ENTRY_RXBURST( entry, b )	{( (entry).entry1 &= 0xfffc );		\
											( (entry).entry1 |= (b) );}

//SET_ENTRY_TXMODE, SET_ENTTRY_TXBURST and SET_TX_PRACH should match with each other
#define SET_ENTRY_TXMODE(entry, m)		{( (entry).entry2 &= 0x07ff );		\
										( (entry).entry2 |= ((m) << 11) );}

#define SET_TX_PRACH( entry, burst_no) 	(entry.frame_entry.entry2 = ((UInt16)((entry.frame_entry.entry2 & 0x07ff)|(ENTRY_TXMODE_PRACH << 11)) |	\
										(UInt16)((entry.frame_entry.entry2 & 0x0003) | burst_no)	\
										))

#define SET_TX_AB1( entry, burst_no) 	(entry.frame_entry.entry2 = ((UInt16)((entry.frame_entry.entry2 & 0x07ff)|(ENTRY_TXMODE_AB1 << 11)) |	\
										(UInt16)((entry.frame_entry.entry2 & 0x0003) | burst_no)	\
										))
#define SET_TX_AB2( entry, burst_no) 	(entry.frame_entry.entry2 = ((UInt16)((entry.frame_entry.entry2 & 0x07ff)|(ENTRY_TXMODE_AB2 << 11)) |	\
										(UInt16)((entry.frame_entry.entry2 & 0x0003) | burst_no)	\
										))

#define SET_ENTRY_TXBUF(entry, b)		{( (entry).entry2 &= 0xf8ff );		\
										( (entry).entry2 |= (b) << 8 );}

#define SET_ENTRY_EPC_MR(entry, b)		{( (entry).entry2 &= 0xffbf ); ( (entry).entry2 |= (b) << 6 );}
#define SET_ENTRY_RX3(entry, b)			{( (entry).entry2 &= 0xffdf ); ( (entry).entry2 |= (b) << 5 );}
#define SET_ENTRY_NORX0(entry, b)		{( (entry).entry2 &= 0xffef ); ( (entry).entry2 |= (b) << 4 );}
//#define SET_ENTRY_PPCH(entry, b)		{( (entry).entry2 &= 0xfff7 ); ( (entry).entry2 |= (b) << 3	);}
//#define SET_ENTRY_PDCH(entry, b)		{( (entry).entry2 &= 0xfffb ); ( (entry).entry2 |= (b) << 2 );}
#define SET_ENTRY_PPCH(entry)			(entry).entry2 |= 1 << 3
#define SET_ENTRY_PDCH(entry)			(entry).entry2 |= 1 << 2
#define SET_ENTRY_TXBURST(entry, b)		{( (entry).entry2 &= 0xfffc ); ( (entry).entry2 |= (b) );}


#define SET_ENTRY_BCCH1(entry)			(entry).entry3 |= 0x8000
#define SET_ENTRY_BCCH1_ZERO(entry)		(entry).entry3 &= 0x7fff
#define SET_ENTRY_BCCH2(entry)			(entry).entry3 |= 0x4000
#define SET_ENTRY_BCCH2_ZERO(entry)		(entry).entry3 &= 0xbfff
#define SET_ENTRY_BCCH3(entry)			(entry).entry3 |= 0x2000
#define SET_ENTRY_BCCH3_ZERO(entry)		(entry).entry3 &= 0xdfff

//#define SET_ENTRY_DTMSPHSLOT(entry,slot)	{( (entry).entry3 &=0xe3ff) ; \
//										( (entry).entry3 |= slot <<10);}
#define SET_ENTRY_CMI_CMR_U(entry,cu)		{( (entry).entry3 &= 0xf7ff); \
											( (entry).entry3 |= cu << 11);}
#define SET_ENTRY_CMI_CMC_D(entry,cd)		{( (entry).entry3 &= 0xfbff); \
											( (entry).entry3 |= cd << 10);}													

#define SET_ENTRY_NEW_AMR_DLINK(entry,flag)	{( (entry).entry3 &= 0xfdff); \
											( (entry).entry3 |= flag << 9 );}
									
#define SET_ENTRY_NEW_AMR_ULINK(entry,flag)	{( (entry).entry3 &= 0xfeff); \
											( (entry).entry3 |= flag << 8 );}

#define SET_ENTRY_AMR_CMR(entry,amr_cmr)	{( (entry).entry3 &= 0xff3f); \
											( (entry).entry3 |= amr_cmr << 6);}

#define SET_ENTRY_NEW_SERV_FLAG(entry, b)	{( (entry).entry3 &= 0xffdf ); ( (entry).entry3 |= (b) << 5 );}
#define SET_ENTRY_CELL(entry, cell)			{((entry).entry3 &= 0xffe0 ); \
											( (entry).entry3 |= cell  );}


#define SET_ENTRY_NULL_PG(entry,null_pg)	{ (entry).entry4 &= 0x7fff; (entry).entry4 |= (null_pg<<15); }
#define SET_ENTRY_UMTS(entry)				{ (entry).entry4 &= 0xbfff; (entry).entry4 |= 0x4000; }
#define SET_ENTRY_UMTS_NO_FN_INC(entry)		{ (entry).entry4 &= 0xdfff; (entry).entry4 |= 0x2000; }
#define SET_ENTRY_UMTS_FIRST_SRCH(entry) 	{ (entry).entry4 &= 0xefff; (entry).entry4 |= 0x1000; }
#define CLEAR_ENTRY_UMTS_FIRST_SRCH(entry) 	{ (entry).entry4 &= 0xefff; }
#define SET_ENTRY_UMTS_SLOTS(entry, slots) 	{ (entry).entry4 &= 0xf0ff; (entry).entry4 |= (slots << 8); }

#define SET_ENTRY_DATAMODE_IND(entry,data_mode)	{( (entry).entry4 &= 0xff7f); \
												( (entry).entry4 |= data_mode << 7)	;}

#define SET_ENTRY_MEAS_INT(entry)	    	( (entry).entry4 |= 1<<6 )

#define SET_ENTRY_CVALUE_SLOT(entry, b)		{( (entry).entry4 &= 0xffc7); \
											( (entry).entry4 |= ( b << 3) );}

#define SET_ENTRY_SERV_SLOT(entry, b)		{( (entry).entry4 &= 0xfff8 ); \
											( (entry).entry4 |=  b		);}

//******************************************************************************
// Handy Shared memory macros for message buffer access
//******************************************************************************
// Receive message buffer macros
// =============================

#define SET_RXMSG_BEC_BFI( buf_ptr, b )	( (buf_ptr)->bfi = (b) )
#define SET_RXMSG_INDEX(   buf_ptr, i )	( (buf_ptr)->index = ( (buf_ptr)->index & 0xff00 ) | ( (i) & 0x00ff ) )
#define SET_RXMSG_ADJUST(  buf_ptr, a )	( (buf_ptr)->index = ( (buf_ptr)->index & 0x00ff ) | ( ( (a) & 0x00ff ) << 8 ) )
#define SET_RXMSG_OTD(     buf_ptr, o )	( (buf_ptr)->otd = (o) )
#define SET_RXMSG_CODE(    buf_ptr, c )	( (buf_ptr)->code = (c) )
#define SET_RXMSG_CN_DB16( buf_ptr, c )	( (buf_ptr)->cn_db16 = (c) )
#define SET_RXMSG_BL_VAR(  buf_ptr, c )	( (buf_ptr)->bl_var = (c) )
#define SET_RXMSG_BURST_BEP(  buf_ptr, i, c )	( (buf_ptr)->BEP[ (i) ] = (c) )
#define SET_RXMSG_SNR(     buf_ptr, c )	( (buf_ptr)->SNR = (c) )

#define GET_RXMSG_BFI(    buf_ptr )		( ( (buf_ptr)->bfi        ) & 0x00ff )
#define GET_RXMSG_BEC(    buf_ptr )		( ( (buf_ptr)->bfi   >> 8 ) & 0x00ff )
#define GET_RXMSG_INDEX(  buf_ptr )		( ( (buf_ptr)->index      ) & 0x00ff )
#define GET_RXMSG_ADJUST( buf_ptr )		( ( (buf_ptr)->index >> 8 ) & 0x00ff )
#define GET_RXMSG_OTD(    buf_ptr )		( ( (buf_ptr)->otd        )          )
#define GET_RXMSG_CODE(   buf_ptr )		( ( (buf_ptr)->code       )          )
#define GET_RXMSG_CN_DB16(buf_ptr )		( ( (buf_ptr)->cn_db16    )          )
#define GET_RXMSG_BL_VAR( buf_ptr )		( ( (buf_ptr)->bl_var     )          )
#define GET_RXMSG_EDGE_MSGPTR(buf_ptr)	( &(buf_ptr)->msg[ 0 ]    )
#define GET_RXMSG_MSGPTR( buf_ptr )		( &(buf_ptr)->report.msg[ 0 ]    	 		)
#define GET_RXMSG_SNRPTR( buf_ptr )		( &(buf_ptr)->report.measurement.SNR[ 0 ]   )
#define GET_RXMSG_RXLEVPTR( buf_ptr )	( &(buf_ptr)->report.measurement.rxlev[ 0 ] )

// Transmit message buffer macros
// ==============================
#define SET_TXMSG_CODE( buf_ptr, c )	( (buf_ptr)->code = (c) )

#define GET_TXMSG_CODE( buf_ptr )		( (buf_ptr)->code )
#define GET_TXMSG_MSGPTR( buf_ptr )		( &(buf_ptr)->msg[ 0 ] )

//******************************************************************************
// Handy Shared memory macros for monitor access
//******************************************************************************
#define MON_STATUS_IS_INVALID			0x8000
#define MON_STATUS_IS_RSSR_FLAG			0x4000
#define MON_STATUS_IS_OVERFLOW			0x2000
#define MON_STATUS_VALUE_MASK			0x1fff
#define MON_DIG_PWR_IS_INVALID			0xffff
#define IS_MON_STATUS_VALID( m )		( ( (m) & MON_STATUS_IS_INVALID  ) == 0 )
#define IS_MON_STATUS_RSSR_FLAG( m )	( ( (m) & MON_STATUS_IS_RSSR_FLAG ) != 0 )
#define IS_MON_STATUS_OVERFLOW( m )		( ( (m) & MON_STATUS_IS_OVERFLOW ) != 0 )
#define READ_MON_STATUS_VALUE( m )		( (m) & MON_STATUS_VALUE_MASK )

//******************************************************************************
// Handy Shared memory macros for RFIC data access
//******************************************************************************
// RFIC data Get Operations
//===========================
#define GET_RFIC_DATA_USF_CODE_FLAG(data)	( ( (data).flag ) & 0x0001 )
#define GET_RFIC_DATA_TXPWR_FLAG(data)		( ( (data).flag >> 1 ) & 0x0001 )
#define GET_RFIC_DATA_USF_DET_FLAG(data)	( ( (data).flag >> 2 ) & 0x0001 )
#define GET_RFIC_DATA_TA_FLAG(data)			( ( (data).flag >> 3 ) & 0x0001 )
#define GET_RFIC_DATA_USF_CODE0(data)		( ( (data).data0 ) & 0x000f )
#define GET_RFIC_DATA_USF_CODE1(data)		( ( (data).data0 >> 4 ) & 0x000f )
#define GET_RFIC_DATA_USF_CODE2(data)		( ( (data).data0 >> 8 ) & 0x000f )
#define GET_RFIC_DATA_USF_CODE3(data)		( ( (data).data0 >> 12 ) & 0x000f )
#define GET_RFIC_DATA_USF_CODE4(data)		( ( (data).data1 ) & 0x000f )
#define GET_RFIC_DATA_USF_CODE5(data)		( ( (data).data1 >> 4 ) & 0x000f )
#define GET_RFIC_DATA_TIMING_ADV(data)		( ( (data).data1 >> 10 ) & 0x003f )
#define GET_RFIC_DATA_TXPWR0(data)			( ( (data).data2 ) & 0x001f )
#define GET_RFIC_DATA_TXPWR1(data)			( ( (data).data2 >> 5 ) & 0x001f )
#define GET_RFIC_DATA_TXPWR2(data)			( ( (data).data2 >>10 ) & 0x001f )
#define GET_RFIC_DATA_TXPWR3(data)			( ( (data).data3 ) & 0x001f )
#define GET_RFIC_USF5DET(data)				( ( (data).data3 >>  15 ) & 0x01 )
#define GET_RFIC_USF4DET(data)				( ( (data).data3 >>  14 ) & 0x01 )
#define GET_RFIC_USF3DET(data)				( ( (data).data3 >>  13 ) & 0x01 )
#define GET_RFIC_USF2DET(data)				( ( (data).data3 >>  12 ) & 0x01 )
#define GET_RFIC_USF1DET(data)				( ( (data).data3 >>  11 ) & 0x01 )
#define GET_RFIC_USF0DET(data) 				( ( (data).data3 >>  10 ) & 0x01 )
#define GET_RFIC_RXSPAN(data)				( ( (data).data4 >>  8 ) & 0x07 )
#define GET_RFIC_MSPATT(data)				( ( (data).data4       ) & 0xff )
#define GET_RFIC_RXPATT(data)				( ( (data).data4 >>  2 ) & 0x3f )
#define GET_RFIC_TXPATT(data)				( ( (data).data4       ) & 0x3f )

// RFIC data Set Operations
//===========================
#define SET_RFIC_DATA_USF_CODE_FLAG(data)	( (data).flag |= 1 )
#define SET_RFIC_DATA_TXPWR_FLAG(data)		( (data).flag |= (1 << 1) )
#define SET_RFIC_DATA_USF_DET_FLAG(data)	( (data).flag |= (1 << 2) )
#define SET_RFIC_USF_CODE0(data, code)		{( (data).data0 &= 0xfff0 ); \
											( (data).data0 |= code );}
#define SET_RFIC_USF_CODE1(data, code)		{( (data).data0 &= 0xff0f ); \
											( (data).data0 |= (code) << 4 );}
#define SET_RFIC_USF_CODE2(data, code)		{( (data).data0 &= 0xf0ff ); \
											( (data).data0 |= (code) << 8 );}
#define SET_RFIC_USF_CODE3(data, code)		{( (data).data0 &= 0x0fff ); \
											( (data).data0 |= (code) << 12 );}
#define SET_RFIC_USF_CODE4(data, code)		{( (data).data1 &= 0xfff0 ); \
											( (data).data1 |= code );}
#define SET_RFIC_USF_CODE5(data, code)		{( (data).data1 &= 0xff0f ); \
											( (data).data1 |= (code) << 4 );}
#define SET_RFIC_DATA_TXPWR0(data, pwr)		{( (data).data2 &= 0xffe0 ); \
											( (data).data2 |= (pwr) );}
#define SET_RFIC_DATA_TXPWR1(data, pwr)		{( (data).data2 &= 0xfc1f ); \
											( (data).data2 |= (pwr) << 5 );}
#define SET_RFIC_DATA_TXPWR2(data, pwr)		{( (data).data2 &= 0x83ff ); \
											( (data).data2 |= (pwr) << 10 );}
#define SET_RFIC_DATA_TXPWR3(data, pwr)		{( (data).data3 &= 0xffe0 ); \
											( (data).data3 |= (pwr) );}
#define SET_RFIC_USF0DET(data, b) 			{( (data).data3 &= 0xfbff );  \
          										( (data).data3 |= (b) << 10 );}
#define SET_RFIC_USF1DET(data, b) 			{( (data).data3 &= 0xf7ff );  \
          										( (data).data3 |= (b) << 11 );}
#define SET_RFIC_USF2DET(data, b) 			{( (data).data3 &= 0xefff );  \
          										( (data).data3 |= (b) << 12 );}
#define SET_RFIC_USF3DET(data, b) 			{( (data).data3 &= 0xdfff );  \
          										( (data).data3 |= (b) << 13 );}
#define SET_RFIC_USF4DET(data, b) 			{( (data).data3 &= 0xbfff );  \
          										( (data).data3 |= (b) << 14 );}
#define SET_RFIC_USF5DET(data, b) 			{( (data).data3 &= 0x7fff );  \
          										( (data).data3 |= (b) << 15 );}
#define SET_RFIC_RXSPAN(data, rxspan)		{(data).data4 &= ~0x0700 ; (data).data4 |= (rxspan) <<  8;}
#define SET_RFIC_MSPATT(data, mspatt)		{(data).data4 &= ~0x00ff ; (data).data4 |= (mspatt) <<  0;}
// RFIC data flag clear Operations
//===========================
#define CLEAR_RFIC_DATA_USF_CODE_FLAG(data)	( ( (data).flag ) &= 0xfffe )
#define CLEAR_RFIC_DATA_TXPWR_FLAG(data)	( ( (data).flag ) &= 0xfffd )
#define CLEAR_RFIC_DATA_USF_DET_FLAG(data)	( ( (data).flag ) &= 0xfffb )
#define CLEAR_RFIC_DATA_TA_FLAG(data)		( ( (data).flag ) &= 0xfff7 )


typedef struct
    {
    UInt16    flag;
    UInt16    data0;
    UInt16    data1;
    UInt16    data2;
    UInt16    data3;
    UInt16    data4;
} Shared_RFIC_t;

// These are flags which can be set by the Arm to control the DSP's Noise suppressor output 
typedef struct
{
    UInt16 sfr0;
    UInt16 sfr1;
    UInt16 sfr2;
} SMC_Data_t;

// The SMC records are indexed via the smc_header which indicates
// where to write the records in the HW SMC and how many records
// to write at a time.
// Total memory required is 60 + 36 = 96 16 bit words
typedef struct
{
    SMC_Data_t data[80];
} SMC_Records_t;

typedef struct
{
    UInt32 data[80];
} FSC_Records_t;


/**
 * @addtogroup Queues
 * @{
 */

/**
 * @addtogroup Main_Queues
 * @{
 */

/**
 * @addtogroup Main_Command_Queue
 * @{
 */

//******************************************************************************
// Shared memory enumerations
//******************************************************************************
typedef enum
{// Command						   Code		Arguments (** = unused command)
 // ========================	   ====		====================================
   /** \par Module
    *                    Modem 
    *  \par Command Code         
    *                    0x00  
    *  \par Description 
    *              To Start Event timer and start SMC processing
    *              
    *              @param UInt16 smc_curr_sfr_page
    *              @param UInt16 smc_event_separation 
    *  
    */
	COMMAND_START_FRAMES,		// 0x00 	( )
   /** \HR */
   /** \par Module
    *                    Modem 
    *  \par Command Code         
    *                    0x01  
    *  \par Description 
    *              To Stop Event timer and stop SMC processing
    *              
    *              @param  None
    */
	COMMAND_STOP_FRAMES,		// 0x01 	( )
   /** \HR */
   /** \par Module
    *                    Modem 
    *  \par Command Code         
    *                    0x02  
    *  \par Description 
    *              To Set Idle Mode
    *              
    *              @param  UInt16 new_serv_cell
    *              @param  UInt16 new_serv_slot
    *              @param  UInt16 ( new_serv_band << 4 ) | dtx_enable
    */
	COMMAND_SET_IDLE_MODE,		// 0x02		( cell, slot, ( freq_band << 4 ) | dtx_enable )
   /** \HR */
   /** \par Module
    *                    Modem 
    *  \par Command Code         
    *                    0x03  
    *  \par Description 
    *              To Set Crypto-key High
    *              
    *              @param  UInt16 new_crypto_kc[0]
    *              @param  UInt16 new_crypto_kc[1]
    */
	COMMAND_SET_KC_HI,			// 0x03		( kc0, kc1 )
   /** \HR */
   /** \par Module
    *                    Modem 
    *  \par Command Code         
    *                    0x04  
    *  \par Description 
    *              To Set Crypto-key Low
    *              
    *              @param  UInt16 new_crypto_kc[2]
    *              @param  UInt16 new_crypto_kc[3]
    */
	COMMAND_SET_KC_LO,			// 0x04		( kc2, kc3 ), load Kc
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x05  
    *  \par Description 
    *       Configures tone generation state machine for DTMF tone or 
    *       supervisory tone
    *              
    *              
    *              @param  UInt16 {Bit 15: UL superimpose | Bit14: DL superimpose | Bits 13-0 Tone Type 
    *                      (< 16: DTMF, >= 16: Supervisory Tone)}
    *              @param  UInt16 Tone_Duration - used only for DTMF tones
    *              @param  UInt16 pg_aud_tone_scale
    */
	COMMAND_GEN_TONE,			// 0x05		( tone_id, duration )
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x06  
    *  \par Description 
    *       Stops Tone-generator
    *              
    *              
    *              @param  None
    */
	COMMAND_STOP_TONE,			// 0x06		( )
	COMMAND_CLOSE_TCH_LOOP,		// 0x07		( tch_loop_mode )
	COMMAND_OPEN_TCH_LOOP,		// 0x08		( )
	COMMAND_START_DAI_TEST,		// 0x09		( dai_mode )
	COMMAND_STOP_DAI_TEST,		// 0x0A		( )
	COMMAND_READ,				// 0x0B		( address, any_val )
	COMMAND_WRITE,				// 0x0C		( address, write_val )
	COMMAND_LOCAL_LOOP,			// 0x0D		( )
	COMMAND_SYNC,				// 0x0E		( any_val )
	COMMAND_TX_FULL,			// 0x0F		( buffer_index, delay, option )
	COMMAND_RX_EMPTY,			// 0x10		( buffer_index )
	COMMAND_SET_SLEEP_CASR,		// 0x11		( cacmp_shift, casr_hi, casr_lo )
	COMMAND_SET_SLEEP_RATIO,	// 0x12		( ratio_hi, ratio_lo ), load new calibration
	COMMAND_FLUSH_BUFFERS,		// 0x13		( )
	COMMAND_RESET_RIP,			// 0x14		( )
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x15  
    *  \par Description 
    *       This command enables/disables the Audio interrupts and configures the hardware audio paths.
    *       Currently this command comes before the COMMAND_AUDIO_CONNECT.
    *              
    *              @param  UInt16 AUDIO_DAC_ENABLE = 1 - Enable DAC,\BR  
    *                             AUDIO_ADC_ENABLE = 2 - Enable ADC,\BR 
    *                             0 - Disable both ADC and DAC
    *              @param  Boolean 16k_sampling - 0 = 8k mode, 1 = 16k mode
    *              @param  Boolean WB_DUAL_MIC_ENABLE - Enables Wide-band dual mics. 
    *                                    When enabled, always uses 16k mode irrespective of what is 
    *                                    set in 16k_sampling argument above
    *
    */
	COMMAND_AUDIO_ENABLE,		// 0x15		( enable )
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x16  
    *  \par Description 
    *       Connect/disconnect the audio input/output from ADC/DAC
    *              
    *              @param  Boolean pg_aud_in_enable  - Used to enable or disable the input samples from ADC
    *              @param  Boolean pg_aud_out_enable - Used to enable or disable the input samples to DAC
    */
	COMMAND_AUDIO_CONNECT,		// 0x16		( in_enable, out_enable )
	COMMAND_SCELL_TIME_TRACK,	// 0x17		( count, shift, limit )
	COMMAND_NCELL_TIME_TRACK,	// 0x18		( count, shift, limit )
	COMMAND_SCH_TIME_TRACK,		// 0x19		( limit )
	COMMAND_FREQ_TRACK1,		// 0x1A		( mult1, shift1 )
	COMMAND_FREQ_TRACK2,		// 0x1B		( mult2, shift2 )
	COMMAND_GET_VERSION_ID,		// 0x1C		( )
	COMMAND_DEBUG_MODE,			// 0x1D		( mode ), starts PROM/DROM checksum
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x1E  
    *  \par Description 
    *       NOT USED ANYMORE
    *              
    *              @param  None
    */
	COMMAND_ECHO_SUPPRESS,		// 0x1E		( "not used" )
	COMMAND_GPIO_FRAME_ACTIVE,	// 0x1F		( GPWR addr, active_value, value_mask )
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x20
    *  \par Description 
    *       Enables DTX
    *              
    *              @param  None
    */
	COMMAND_DTX_ENABLE,			// 0x20		( enable )
	COMMAND_RESET_AFC_DAC,		// 0x21		( )
	COMMAND_MS_USFMODE,			// 0x22		( "not used" )
	COMMAND_MS_MSGMODE,			// 0x23		( "not used" )
	NOT_USE_COMMAND_24,			// 0x24	
	COMMAND_PRAM_WRITE,			// 0x25		( addr, value )
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x26
    *  \par Description 
    *       Enables VPU
    *              
    *              @param  None
    */
	NOT_USE_COMMAND_26,			// 0x26		( )
	COMMAND_GEN_TIMING_PULSE,	// 0x27		( frame_index, qbc_delay, qmcr_value )
	COMMAND_TX_ABORT,			// 0x28		( buffer_index )
	COMMAND_CLOSE_MS_LOOP,		// 0x29		( mode, rxindex0/txindex0, rxindex1/txindex1 )
	COMMAND_OPEN_MS_LOOP,		// 0x2A		( )
	NOT_USE_COMMAND_2B,			// 0x2B
	COMMAND_SYNC_OFFSET,		// 0x2C		( max_sync_offset )
	COMMAND_RELOAD_TX_PARMS,	// 0x2D		( )
	COMMAND_PWRCTRL_IDLE,		// 0x2E		( reset_cn_flag, idle_average, pb(dB) )
	COMMAND_PWRCTRL_TXFER,		// 0x2F		( bcch_flag, txfer_average )
	COMMAND_PWRCTRL_ALPHA,		// 0x30		( alpha(tenths) )
	COMMAND_PWRCTRL_PMAX,		// 0x31		( pmax(dB) )
	COMMAND_PWRCTRL_GAMMA,		// 0x32		( abs_slot, gamma_ch(dB) )
	COMMAND_PWRCTRL_LOAD,		// 0x33		( )
	COMMAND_NOISE_SUPPRESSION,	// 0x34		( cmd_action )
	COMMAND_ECHO_CANCELLATION,	// 0x35		( cmd_action, mode )
	COMMAND_FREQ_TRACK1_HR,		// 0x36		( mult1_hr, shift1_hr )
	COMMAND_FREQ_TRACK2_HR,		// 0x37		( mult2_hr, shift2_hr, avg_enable )
	COMMAND_SCELL_TIME_TRACK1_HR,// 0x38	( acc_cnt_hr, acc_shift_hr, mag_limit_hr )
	COMMAND_SCELL_TIME_TRACK2_HR,// 0x39    ( error_mag_hr, mag_limit_hr, shift_hr )
	COMMAND_START_EOTD,			// 0x3A		( cell id of serving cell )	Note: only do something if EOTD is defined!!!
	COMMAND_STOP_EOTD,			// 0x3B		( ) Note: only do something if EOTD is defined!!!
	COMMAND_SET_4BIT_CELLID,	// 0x3C		( send TRUE in arg0 to use 4 bit, FALSE to disable, DSP by default uses 3 bit)
	COMMAND_SET_EOTD_MODE,		// 0x3D		( set arg=0 to use derotated IQ or arg=1 for raw IQ for correlation results ) Note: only do something if EOTD is defined!!!
	COMMAND_SET_EOTD_RESET,		// 0x3E		( ) Note: only do something if EOTD is defined!!!
    COMMAND_SET_AMR_PARMS,      // 0x3F     ( Set AMR Parms  arg0=ICM, arg1=ACS )
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x40 
    *  \par Description 
    *       This command configures the first 3 of the 10 sidetone Double Biquad filter's parameters.
    *              
    *              @param  UInt16 Coefficient 0
    *              @param  UInt16 Coefficient 1
    *              @param  UInt16 Coefficient 2
    *
    *  \see Software_Sidetone
    */
	COMMAND_SIDETONE_COEFS_012, // 0x40		( SIDETONE_FILTER Biquad filter coefs 0, 1 and 2 )
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x41 
    *  \par Description 
    *       This command configures the second 3 of the 10 sidetone Double Biquad filter's parameters.
    *              
    *              @param  UInt16 Coefficient 3
    *              @param  UInt16 Coefficient 4
    *              @param  UInt16 Coefficient 5
    *
    *  \see Software_Sidetone
    */
	COMMAND_SIDETONE_COEFS_345, // 0x41		( SIDETONE_FILTER Biquad filter coefs 3, 4 and 5 )
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x42 
    *  \par Description 
    *       This command configures the third 3 of the 10 sidetone Double Biquad filter's parameters.
    *              
    *              @param  UInt16 Coefficient 6
    *              @param  UInt16 Coefficient 7
    *              @param  UInt16 Coefficient 8
    *
    *  \see Software_Sidetone
    */
	COMMAND_SIDETONE_COEFS_678, // 0x42		( SIDETONE_FILTER Biquad filter coefs 6, 7 and 8 )
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x43
    *  \par Description 
    *       This command configures the last (10th) sidetone Double Biquad filter's parameter.
    *              
    *              @param  UInt16 Coefficient 9
    *
    *  \see Software_Sidetone
    */
	COMMAND_SIDETONE_COEFS_9,   // 0x43		( SIDETONE_FILTER Biquad filter coef 9 )
	COMMAND_INIT_RXQUAL,		// 0x44		( Init rxqual and rxqual_sub )
	COMMAND_ECHO_CANCEL_COEFS_012, 	// 0x45	( ECHO_CANCELLER Biquad filter coefs 0, 1 and 2 )
	COMMAND_ECHO_CANCEL_COEFS_345, 	// 0x46	( ECHO_CANCELLER Biquad filter coefs 3, 4 and 5 )
	COMMAND_ECHO_CANCEL_COEFS_678, 	// 0x47	( ECHO_CANCELLER Biquad filter coefs 6, 7 and 8 )
	COMMAND_ECHO_CANCEL_COEFS_9,	// 0x48	( ECHO_CANCELLER Biquad filter coef 9 )
    COMMAND_AMR_CLEAR_FLAG,			// 0x49 ( clear AMR flags after AMR call )
	COMMAND_UPDATE_NULL_PAGE_TEMPLATE,// 0x4a ( Indicates the NULL paging block template has changed and needs to be updated)
	COMMAND_GEN_TONE_GENERIC,	// 0x4b		( freq0, freq1, freq2 )
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x4c 
    *  \par Description 
    *       Configures parameters for generic tri-tone generation
    *              
    *              @param  UInt16 {Bit 1: UL superimpose | Bit0: DL superimpose}
    *              @param  UInt16 tone_Duration
    *              @param  UInt16 pg_aud_tone_scale
    */
	COMMAND_PARM_TONE_GENERIC,	// 0x4c		( superimpose_flag, duration, tone_scale )
	COMMAND_SEGMENT_TEST,  		// 0x4d		temporary test  
	COMMAND_CTM_READY,			// 0x4e
	COMMAND_BLOCK_COPY,			// 0x4f		( shared memory page to switch to, address of block copy header: mode(ram->ram/ram->pram), length, src_ptr, dst_ptr )
	COMMAND_SET_WAS_SPEECH_IDLE,// 0x50
    COMMAND_ENHANCED_MEAS_ENABLE,//0x51		( enable )
    COMMAND_BITWISE_WRITE,		// 0x52		( address, mask, value)
    COMMAND_MIC_SELECT,		    // 0x53		( AUDSEL 0 == Handset MIC or == 1 for AUX MIC)
    COMMAND_AUDIO_INPUT_GAIN,	// 0x54		( AUDIG gain)
    COMMAND_AUX_OUTPUT_GAIN,	// 0x55		( AUXOG gain)
    COMMAND_AUDIO_OUTPUT_GAIN,	// 0x56		( AUDOG gain)
    COMMAND_SIDETONE_GAIN,		// 0x57		( STG gain)
	COMMAND_FLUSH_FACCH_RATSCCH,// 0x58     ( Flush out coded but not transmitted or partially transmitted FACCH(arg0=TXCODE_FACCH) or RATSCCH(arg0=TXCODE_RATSCCH) )
    COMMAND_PATCH,		        // 0x59		( patch_select, parm0, parm1 )
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x5a
    *  \par Description 
    *       This command runs the main AMR vocoder during GSM idle to support WCDMA/VOIP voice call. This command has to be 
    *       sent by the MCU every-time data is sent to the DSP to do a decode for WCDMA/VOIP call. The input to the speech 
    *       decoder comes in DL_MainAMR_buf.param, and the output of the encoder is stored in UL_MainAMR_buf.param. 
    *
    *  \note Before getting this command ARM should have enabled the Audio Interrupts using COMMAND_AUDIO_ENABLE, and 
    *       the audio inputs and outputs enabled by COMMAND_AUDIO_CONNECT.
    *              
    *              @param  UInt16 For VOIP: {bit15-bit8: VOIP mode, bit0 - Voip_DTX_Enable} \BR
    *                             For Non-VOIP: {bit15-bit8:0, bit4: 1/0=WB_AMR/NB_AFS, bit3-0: AMR UL Mode set}
    *                             
    *              @param  UInt16 AMR_IF2_flag   - AMR Iterface Format 2 flag
    *              @param  UInt16 WCDMA_MST_flag - WCDMA MST SD decoding flag \BR 
    *                                            = 2 - MainAMR_WCDMA_MST_flag = TRUE, MainAMR_WCDMA_MST_init = TRUE; \BR
    *                                            = 1 - MainAMR_WCDMA_MST_flag = TRUE, MainAMR_WCDMA_MST_init = FALSE; \BR
    *                                            = 0 - MainAMR_WCDMA_MST_flag = FALSE, MainAMR_WCDMA_MST_init = FALSE;
    *   \par Associated Replies
    *        For every COMMAND_MAIN_AMR_RUN command, an associated STATUS_MAIN_AMR_DONE reply would be sent in the status 
    *        queue.
    *   \sa  DL_MainAMR_buf, UL_MainAMR_buf, STATUS_MAIN_AMR_DONE
    */    
    COMMAND_MAIN_AMR_RUN,		// 0x5a		( run main AMR vocoder during GSM idle to support WCDMA voice call, arg0=UL AMR codec mode request )
    COMMAND_AUDIO_ALIGNMENT,	// 0x5b		( align audio input and/or output buffer, arg0: input buf, arg1: output buf )
	COMMAND_VOICE_FILTER_COEFS,	// 0x5c		( )
	COMMAND_POLYRINGER_STARTPLAY,	// 0x5d	( Polyringer Play )
	COMMAND_POLYRINGER_CANCELPLAY,	// 0x5e	( Polyringer Cancel )
    COMMAND_MST_FLAG_EFR,		// 0x5f		( value(0,1,2,3,4,7,9) )
    COMMAND_MST_FLAG_AFS,		// 0x60		( value(0,1,2,3,4,7,9) )
    COMMAND_MST_FLAG_AHS,		// 0x61		( value(0,1,2,3,4,7,9) )
    COMMAND_MST_FLAG_CS1,		// 0x62		( value(0,1,2,3,4,7,9) )
	COMMAND_DL_MAIN_DEC_OUT_PCM_ENABLE,//0x63 ( enable copy downlink primary vocoder pcm output to shared RAM )
	COMMAND_ADJ_SCELL_TIME_OFFSET,	// 0x64	( QBC adjustment value )
    COMMAND_FLAG_SAIC,			// 0x65		
    COMMAND_PATCH_ENABLE,		// 0x66	
	COMMAND_WAKEUP_SMC,			// 0x67
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x68
    *  \par Description 
    *      This command loads the coefficients of the various Biquad Filters
    *
    *              @param  UInt16 Load_Biquad_Coefficients_Of:
    *                             - = 1: Load 10 coefficients for Bluetooth UL Double-Biquad filter from shared_bluetooth_ul_filter_coef \BR
    *                             - = 2: Load 10 coefficients for Bluetooth DL Double-Biquad filter from shared_bluetooth_dl_filter_coef \BR
    *                             - = 3: Load 10 coefficients for Echo Far-In filter from shared_echo_farIn_filt_coefs \BR
    *                             - = 4: Load 10 coefficients for Compressor Pre-filter from shared_comp_filter_coef \BR
    *
    *              \see shared_bluetooth_ul_filter_coef, shared_bluetooth_dl_filter_coef, shared_echo_farIn_filt_coefs, 
    *                   shared_comp_filter_coef
    */
	COMMAND_BIQUAD_FILT_COEFS,	// 0x68		( Biquad filter coef 0-9, arg0 = 1(Bluetooth UL), =2(Bluetooth DL), =3 (ECHO FARIN FILT), =4 (DL filter))
	COMMAND_SET_PDMA,			// 0x69		( setup PDAM, arg0=ADDR_H, arg1=ADDR_L, arg2=LEN )	
	COMMAND_START_PDMA,			// 0x6a		( lunch PDMA, arg0=PRAM_OFFSET )
	COMMAND_SET_DDMA,			// 0x6b		( setup DDAM, arg0=ADDR_H, arg1=ADDR_L, arg2=LEN )
	COMMAND_START_DDMA,			// 0x6c		( lunch DDMA, arg0=DRAM_OFFSET, arg1=[1/0]=[DMA_WRITE/DMA_READ] )
	COMMAND_START_PRAM_FUNCT,	// 0x6d		( PDMA load and funct call, arg0=PRAM_Addr_H, arg1=PRAM_Addr_L, arg2=PDMA_len )
	COMMAND_STOP_PRAM_FUNCT,	// 0x6e		( set pram_mode=0 )
    COMMAND_LOAD_SYSPARM,		// 0x6f ( Force DSP re-init sysparm defined in SYSPARM_Init() function )
	COMMAND_EC_DEEMP_PREEMP_FILT_COEFS,	// 0x70	( de_emp_filt_coef, pre_emp_filt_coef )
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x71
    *  \par Description 
    *      This command starts the NEWAUDFIFO interface to play PCM data to the speaker path.
    *      Along with sending this command to the DSP, AP has to enable the Audio interrupts to
    *      the DSP. Currently in products earlier to Athena, it is done by directly poking into 
    *      DSP registers.
    *
    *      \note This command is not supported in Athena.
    *              
    *              @param  UInt16 channel_index =[0,1]=[shared_pram_codec_out0, shared_newpr_codec_out0]
    *              @param  UInt16 AudioChannelMode \BR
    *                       { bit15: buffer_source=[0,1]=[sharedmem, downloadable xram] (JunoB0) (for seamless STW) \BR
    *                         bit14: non-interleaved stereo PCM=[0,1]=[disable,enable] (JunoD0/ JunoB0+patch) \BR
    *                          bit0: channel_mode=[0,1]=[Mono, Stero/Dual_Mono] }
    *              @param  UInt16 { \BR
    *                          bit15   : Audio_SW_FIFO_LOW_reported \BR
    *                          bits14-0: soft FIFO threshould, (default 8192) - DSP sends STATUS_NEWAUDFIFO_SW_FIFO_LOW  
    *                                    when amount of soft data < threshold }
    *
    *              \see NEWAUDFIFO_Interface, STATUS_NEWAUDFIFO_SW_FIFO_LOW, STATUS_NEWAUDFIFO_SW_FIFO_EMPTY, 
    *                   COMMAND_NEWAUDFIFO_CANCEL, COMMAND_NEWAUDFIFO_PAUSE, COMMAND_NEWAUDFIFO_RESUME, 
    *                   STATUS_NEWAUDFIFO_CANCELPLAY, STATUS_NEWAUDFIFO_DONEPLAY
    */
	COMMAND_NEWAUDFIFO_START,		// 0x71	( arg0=channel_index(0,1,2), arg1=[0,1]=[Mono,Stero/Dual_Mono], arg2=soft FIFO threshould )
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x72
    *  \par Description 
    *      This command pauses the NEWAUDFIFO interface from play PCM data to the speaker path.
    *
    *      \note This command is not supported in Athena.
    *              
    *              @param  UInt16 channel_index =[0,1]=[shared_pram_codec_out0, shared_newpr_codec_out0]
    *
    *              \see NEWAUDFIFO_Interface, STATUS_NEWAUDFIFO_SW_FIFO_LOW, STATUS_NEWAUDFIFO_SW_FIFO_EMPTY, 
    *                   COMMAND_NEWAUDFIFO_START, COMMAND_NEWAUDFIFO_CANCEL, COMMAND_NEWAUDFIFO_RESUME,
    *                   STATUS_NEWAUDFIFO_CANCELPLAY, STATUS_NEWAUDFIFO_DONEPLAY
    */
	COMMAND_NEWAUDFIFO_PAUSE,		// 0x72	
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x73
    *  \par Description 
    *      This command resumes playback on the NEWAUDFIFO interface to the speaker path.
    *
    *      \note This command is not supported in Athena.
    *              
    *              @param  UInt16 channel_index =[0,1]=[shared_pram_codec_out0, shared_newpr_codec_out0]
    *
    *              \see NEWAUDFIFO_Interface, STATUS_NEWAUDFIFO_SW_FIFO_LOW, STATUS_NEWAUDFIFO_SW_FIFO_EMPTY, 
    *                   COMMAND_NEWAUDFIFO_START, COMMAND_NEWAUDFIFO_CANCEL, COMMAND_NEWAUDFIFO_PAUSE,
    *                   STATUS_NEWAUDFIFO_CANCELPLAY, STATUS_NEWAUDFIFO_DONEPLAY
    */
	COMMAND_NEWAUDFIFO_RESUME,		// 0x73	
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x74
    *  \par Description 
    *      This command cancels playback on the NEWAUDFIFO interface to the speaker path.
    *      AP has to disable the hardware interrupts from coming to the DSP. In platforms
    *      prior to Athena, this was done by poking to DSP registers. \BR
    *
    *      DSP will response with STATUS_NEWAUDFIFO_CANCELPLAY.
    *
    *      \note This command is not supported in Athena.
    *              
    * 	   \note The playback will be stopped right away. There may be un-finished 
    * 	         PCM data in shared_pram_codec_out0 or shared_newpr_codec_out0.
    *
    *              @param  UInt16 channel_index =[0,1]=[shared_pram_codec_out0, shared_newpr_codec_out0]
    *
    *              \see NEWAUDFIFO_Interface, STATUS_NEWAUDFIFO_SW_FIFO_LOW, 
    *                   STATUS_NEWAUDFIFO_SW_FIFO_EMPTY, COMMAND_NEWAUDFIFO_START, 
    *                   COMMAND_NEWAUDFIFO_CANCEL, COMMAND_NEWAUDFIFO_PAUSE,
    *                   COMMAND_NEWAUDFIFO_RESUME, STATUS_NEWAUDFIFO_DONEPLAY
    */
	COMMAND_NEWAUDFIFO_CANCEL,		// 0x74	
	COMMAND_INIT_TONE_RAMP,		// 0x75		( tone_ramp_delta )
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x76  
    *  \par Description 
    *      This command is no longer needed, as there are no longer any downloadable codecs in the DSP.
    *
    *      @param None
    *              
    */
	COMMAND_AUDIO_TASK_START_REQUEST,		// 0x76
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x77  
    *  \par Description 
    *      This command Enables the Uplink High Pass Filter in the microphone path of the transfer function:\BR
    *      Y(z)/X(z) = b*( 1 - z^(-1) ) / ( 1 - a*z^(-1) )
    *
    *
    *      @param UInt16 Enable: 1=Enable, 0 = Disable
    *      @param UInt16 coef[0] (b)
    *      @param UInt16 coef[1] (a)
    *              
    */
	COMMAND_SET_UL_HPF_COEF,	// 0x77		( coef B, coef A )
	COMMAND_SET_DL_HPF_COEF,	// 0x78		( coef B, coef A )
	COMMAND_LOAD_UL_EQ_GAINS,	// 0x79		( )
	COMMAND_SET_RF_TX_CAL,		// 0x7A		( RF TX Calibration Offset )
    COMMAND_CHECK_SUM_TEST,		// 0x7B		( DSP shared RAM page number, DSP shared address )
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x7C  
    *  \par Description 
    *      THIS COMMAND IS NO LONGER USED.
    *
    *      @param  UInt16 PRAM_codec_InBuf_low_th
    *              
    */
	COMMAND_SET_TH_PRAM_CODEC_INPUT_LOW,	// 0x7C		( set threshold for VP_STATUS_PRAM_CODEC_INPUT_LOW )
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x7D  
    *  \par Description 
    *       This command routes the output of the hardware mixer which combines the voice, polyringer and the audio 
    *       streams to the ARM. It sets the Bluetooth Mixer's buffer size and the mode for each Bluetooth Mixer 
    *       session. This command starts this transfer. In the DSP code the hardware mixers FIFO interrupts 
    *       (BTAPS_INT_BIT) are always enabled. ARM has to configure the BTM FIFO Interrupt (BTAPS_INT_BIT).
    *       The DSP stores the Bluetooth Mixer data in shared_BTMIXER_OutputBuffer0 and shared_BTMIXER_OutputBuffer1
    *       buffers, and DSP interrupts ARM with STATUS_BTM_FRAME_DONE reply
    *       whenever finishing one BTM buffer in one sharedmem page (buffer starting at the beginning of 4kw 
    *       sharedmem page). The finished BTM buffer is in page BTM_page, which indicates page index offset 
    *       relative to the first sharedmem page of  shared_BTMIXER_OutputBuffer0/1[4096]. 
    *       ARM reads BTM buffer from BTM_page and sends for SBC coding for BlueTooth. 
    *       To stop BTM-WB: ARM disables HW BTM FIFO INT. 
    *              
    *              
    *              @param  UInt16 BTM_buffer_size  - Must be less than 4096. Number of samples to collect 
    *                                                from the Bluetooth Mixer.
    *              @param  UInt16 BTM_page_n_mode  - {bit14: 0/1=interleaved/non-interleaved stereo, 
    *                                                 bit 1: enable STATUS, bit 0: 0/1=mono/stereo}
    *
    *
    *  \see shared_BTMIXER_OutputBuffer0, shared_BTMIXER_OutputBuffer1, STATUS_BTM_FRAME_DONE
    *
    *  \note This function needs to be modified for TKL3 for non-paged mode of operation.
    *  \note This function would not be necessary in Rhea as ARM can directly route the output of the hardware
    *        mixer to where-ever it wants
    */    
    COMMAND_SET_BTM_BUFFER_SIZE,               // 0x7D        ( arg0=BTM_buffer_size (<=4096) based on BT mixer sample rate, arg1=[b1|b0]=[enable STATUS|0/1=mono/stereo] )
    NOT_USE_COMMAND_7E,                        // 0x7E     
    COMMAND_AUDIO16K_INIT_NOT_USED,            // 0X7F     This command was used for testing puurpos of wb_amr encoder with 16KISR and a dead command
    COMMAND_UPLOAD_AUDIO_DEBUG_DATA,           // 0x80     (  )
    COMMAND_ENABLE_RFIC_DEBUG_DATA,            // 0x81
    COMMAND_EPC,                               // 0x82     ( setup EPC mode, arg0 = EPC mode enable/disable, arg1 = FPC enable/disable )
    NOT_USE_COMMAND_83,				// 0x83		
    COMMAND_MIC2SPK,				// 0x84		(  )
	COMMAND_ENABLE_DSP_TESTPOINTS,	// 0x85	( arg0 = testpoint mode, if arg0 == 2 then arg1 = shared_testpoint_data_buf_wrap_idx )
	COMMAND_RESET_HQ_ADC,			// 0x86	( arg0 = HQ_ADC_mode = [b1|b0]=[enable(1) STATUS_HQ_ADC_PAGE_DONE|1/0=stereo/mono(L)] )
	NOT_USE_COMMAND_87,				// 0x87	
	COMMAND_SET_AUDIR2,				// 0x88	( arg0=[b1|b0]=[enable(1) STATUS_AUDIR2_BUFFER_DONE at 20ms|enable(1)/disable(0)] )
	COMMAND_VIBRA_ENABLE,			// 0x89	( arg0=[x|14:12|x|10:0]=[x|dup_bit,gain_bit,enable_bit|x|DGAIN], arg1=VIBRA_CFG, arg2=[DUPDATA(bypass)/FIFOMODE(normal)])
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x8a 
    *  \par Description 
    *      This command is part of the tone generator in the DSP. It configures the manner in which
    *      the tone gets super-imposed. \BR
    *
    *      \note This command sets pg_aud_superimpose.[b3|x|b1|b0]=[recording tone|x|superimpose_ul|superimpose_dl] <= enable(1)/disable)(0)
    *
    *      @param  UInt16 mask
    *      @param  UINT16 value. 
    *
    */
	COMMAND_GEN_TONE_SUPERIMPOSE,	// 	 0x8a ( arg0=mask, arg1=value. set pg_aud_superimpose.[b3|x|b1|b0]=[|enable(1)/disable)(0) recording tone|x|superimpose_ul|superimpose_dl] )
	COMMAND_ENABLE_RF_RX_TEST,      // 0x8b ( arg0 = rx test flag )
	COMMAND_ENABLE_RF_TX_TEST,      // 0x8c ( arg0 = tx test flag )
	COMMAND_C_VECTOR, 				// 0x8d ( "not used" )
	COMMAND_INIT_COMPANDER,			// 0x8e
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x8f 
    *  \par Description 
    *      This command initializes the sidetone expander, from the settings present in 
    *      the shared_sidetone_expander_flag and sidetone_expander shared memory variables.\BR
    *
    *      It can be used for enabling, disabling, as well as configuring the Sideband Expander.
    *
    *      \see sidetone_expander, shared_sidetone_expander_flag, Software_Sidetone
    */
	COMMAND_INIT_SIDETONE_EXPANDER,		// 0x8f
	COMMAND_INIT_BIQUAD_FILTER_MEMORY,	// 0x90
	COMMAND_MUSIC_SUBBAND_SPEAKER_PROTECTION_VECTOR,	// 0x91 ( arg0 = music subband_speaker_protection_vector )
	COMMAND_RESET_STACK_DEPTH_CHECK_RES,	// 0x92	( arg0 = 0: Disable stack depth checking, 1: Enable stack depth checking, 2: Upload Current minimum stack depth to shared memory )
	COMMAND_GET_SHARED_MEM_SIZE,			// 0x93
    NOT_USE_COMMAND_94,						// 0x94
	NOT_USE_COMMAND_95,		                // 0x95
	CORE_TL3210_DUMP,						// 0x96 Core dump to MTT at any time of processing
	COMMAND_CLEAR_VOIPMODE,					// 0x97 (arg0 = 0 to clear VOIPmode)
 	COMMAND_SMC_FCWR_CNFG_ENABLE,			// 0x98 (arg0 = 1 to enable)
 	COMMAND_SET_20MS_TRIGGER,				// 0x99 (arg0=[1/0]=[enable/disable] 20ms status trigger to arm)
	COMMAND_MODEM_LOGGING_ENABLE,			// 0x9A
	NOT_USE_COMMAND_9B,						// 0x9B	
	COMMAND_DSP_VIS_ENABLE,					// 0x9C (arg0 = TRACE/ATB SELECTION, arg1 = ATB ID/TRACE CONTROL, arg2= ATB CONTROL)		
	COMMAND_AUDIO_ENHANCEMENT_ENABLE,		// 0x9D
	NOT_USE_COMMAND_9E,						// 0x9E
	NOT_USE_COMMAND_9F,						// 0x9F    
#ifndef tempIntefrace_DSP_FEATURE_SP
	  COMMAND_SP,		// 0xA0	  (enable) arg0 = 0 Disable, 1 enable; arg1 = oper_mode; arg2 = init flag
#else
	  NOT_USE_COMMAND_A0,
#endif
	  COMMAND_ENABLE_DUAL_MIC,  //0xA1 (enable) arg0=0 Disable, 1 enable;
	  COMMAND_QBC_STAR_STOPCNT,	//0xA2 (start/stop event timer QBC count) arg0=1 start; arg0=2 stop count; otherwise no effect on QBC count;
	  COMMAND_DUAL_SIM_TRACK	//0xA3 (arg0=Kd, arg1=delta_limit, arg2= high byte is 2nd_sim_id & lower byte is delta_flag with zero means disabled)
} Command_t;
/**
 * @}
 */
/**
 * @}
 */

/**
 * @addtogroup Fast_Queues
 * @{
 */

/**
 * @addtogroup Fast_Command_Queue
 * @{
 */

typedef enum
{// Command						   Code		Arguments (** = unused command)
 // ========================	   ====		====================================
	FAST_COMMAND_READ,			// 0x00		( address, any_val )
	FAST_COMMAND_WRITE,			// 0x01		( address, write_val )
    FAST_COMMAND_BITWISE_WRITE,	// 0x02		( address, mask, value)
	FAST_COMMAND_TX_FULL,		// 0x03		( buffer_index, delay, option )
	FAST_COMMAND_FLUSH_BUFFERS,	// 0x04		( )
    FAST_COMMAND_TX_ABORT,		// 0x05		( buffer_index )
    FAST_COMMAND_AUDIO_ALIGNMENT,	// 0x06		( align audio input and/or output buffer, arg0: input buf, arg1: output buf )
	FAST_COMMAND_PRAM_WRITE,	// 0x07		( addr, value )
	FAST_COMMAND_BLOCK_COPY,	// 0x08		( shared memory page to switch to, address of block copy header: mode(ram->ram/ram->pram), length, src_ptr, dst_ptr )
	FAST_COMMAND_DISABLE_DSP_SLEEP,	// 0x09	( enable/disable )
    FAST_COMMAND_INIT_DSP_PARMS,	// 0x0A		( audio init enable/disable, ISR parms init enable/disable, Msgproc init enable/disable )
	FAST_COMMAND_READ_DSPMEM    // 0x0B     ( DSP address, any_val )
} FastCommand_t;

/**
 * @}
 */
/**
 * @}
 */
/**
 * @}
 */


//******************************************************************************
// Shared Memory Definition 
//******************************************************************************
// SINCE TLIII CORE/MSS, SHARED MEMORY PAGING IS REMOVED.
// THREE BIG CATERGORIES ARE DEFINED:
//
//
//                 1. MODEM
//                    SEC_CMD_STATUS
//                    SEC_MST_SAIC_GEN_MODEM
//                    SEC_MODEM_MET
//                    SEC_RESERVED_PART1                
//                    SEC_SMC_AFC_AGC
//                    SEC_RFIC
//                    SEC_RESERVED_PART2
//
//                2. AUDIO
//                    SEC_GEN_AUDIO
//                    SEC_NS
//                    SEC_NLP
//                    SEC_ECHO
//                    SEC_VPU
//                    SEC_AUX_AUDIO
//
//                3. DEBUG
//                    SEC_DIAGNOSIS    
//                    SEC_DSP_GEN_DEBUG    
//
//******************************************************************************
#ifdef MSP
typedef struct
{
#endif

EXTERN UInt32 shared_memory_start                                SHARED_SEC_CMD_STATUS;

/*-----------------------------------------------------------------------------------------------*/
/**
 * @addtogroup Queues
 * @{
 */

/**
 * @addtogroup Main_Queues
 * @{
 */

/**
 * @addtogroup Main_Command_Queue
 * @{ 
 */
EXTERN UInt16 shared_cmdq_in                                     SHARED_SEC_CMD_STATUS;            ///< Command queue input index
EXTERN UInt16 shared_cmdq_out                                    SHARED_SEC_CMD_STATUS;            ///< Command queue output index
/**
 * @}
 */
/**
 * @}
 */

/**
 * @addtogroup Fast_Queues
 * @{
 */
/**
 * @addtogroup Fast_Command_Queue
 * @{ 
 */
EXTERN UInt16 shared_fast_cmdq_in                                SHARED_SEC_CMD_STATUS;            ///< INT1 fast command queue input index
EXTERN UInt16 shared_fast_cmdq_out                               SHARED_SEC_CMD_STATUS;            ///< INT1 fast command queue output index
/**
 * @}
 */
/**
 * @}
 */


/**
 * @addtogroup Main_Queues
 * @{
 */
/**
 * @addtogroup Main_Status_Queue
 * @{ 
 */
EXTERN UInt16 shared_statusq_in                                  SHARED_SEC_CMD_STATUS;            ///< Status queue input index
EXTERN UInt16 shared_statusq_out                                 SHARED_SEC_CMD_STATUS;            ///< Status queue output index
/**
 * @}
 */
/**
 * @}
 */



/**
 * @addtogroup Main_Queues
 * @{
 */
/**
 * @addtogroup Main_Command_Queue
 * @{ 
 */
EXTERN CmdQ_t shared_cmdq[ CMDQ_SIZE ]                           SHARED_SEC_CMD_STATUS;            ///< Circular buffer of command queue elements
/**
 * @}
 */
/**
 * @}
 */

/**
 * @addtogroup Fast_Queues
 * @{
 */
/**
 * @addtogroup Fast_Command_Queue
 * @{ 
 */
EXTERN CmdQ_t shared_fast_cmdq[ FAST_CMDQ_SIZE ]                 SHARED_SEC_CMD_STATUS;            ///< Circular buffer of INT1 fast command queue elements
/**
 * @}
 */
/**
 * @}
 */


/**
 * @addtogroup Main_Queues
 * @{
 */
/**
 * @addtogroup Main_Status_Queue
 * @{ 
 */
EXTERN StatQ_t shared_statusq[ STATUSQ_SIZE ]                    SHARED_SEC_CMD_STATUS;            ///< Circular buffer of status queue elements
/**
 * @}
 */
/**
 * @}
 */


/**
 * @}
 */


EXTERN UInt16 shared_rfic_request								SHARED_SEC_GEN_MODEM;			// DSP to ARM to indicate that this IRQ is a rfic request
EXTERN UInt16 shared_rip_request								SHARED_SEC_GEN_MODEM;			// DSP to ARM to indicate that this IRQ is a general request
EXTERN UInt16 shared_fiq_response								SHARED_SEC_GEN_MODEM;			// Arm to DSP flag to indicate that this interrupt is a response to a DSP-->Arm FIQ 
EXTERN UInt16 shared_frame_out									SHARED_SEC_GEN_MODEM;			// Frame entry index (entry being received)
EXTERN Shared_FrameEntry_t shared_frame[ FRAME_ENTRY_CNT ]		SHARED_SEC_GEN_MODEM;			// Circular buffer of frame entries
EXTERN EventInfo_t shared_event_mode[EVENT_SEQ_SIZE]			SHARED_SEC_GEN_MODEM;			// Events difined (written) by rficisr.c and used (read) by DSP.
EXTERN Shared_RFIC_t shared_rfic_data							SHARED_SEC_GEN_MODEM;	
EXTERN UInt16 shared_mon1[ FRAME_ENTRY_CNT ]					SHARED_SEC_GEN_MODEM;			 // Circular buffer of power meas. #1
EXTERN UInt16 shared_mon2[ FRAME_ENTRY_CNT ]					SHARED_SEC_GEN_MODEM;			 // Circular buffer of power meas. #2
EXTERN UInt16 shared_mon3[ FRAME_ENTRY_CNT ]					SHARED_SEC_GEN_MODEM;			 // Circular buffer of power meas. #3
EXTERN UInt16 shared_mon1_1[ FRAME_ENTRY_CNT ]					SHARED_SEC_GEN_MODEM;			 // Circular buffer of power meas. #1-2nd slot
EXTERN UInt16 shared_mon1_2[ FRAME_ENTRY_CNT ]					SHARED_SEC_GEN_MODEM;			 // Circular buffer of power meas. #1-3rd slot
EXTERN UInt16 shared_mon1_3[ FRAME_ENTRY_CNT ]					SHARED_SEC_GEN_MODEM;			 // Circular buffer of power meas. #1-4th slot
EXTERN UInt16 shared_mon1_4[ FRAME_ENTRY_CNT ]					SHARED_SEC_GEN_MODEM;			 // Circular buffer of power meas. #1-5th slot
EXTERN UInt16 shared_mon_dig_pwr1[ FRAME_ENTRY_CNT ]			SHARED_SEC_GEN_MODEM;			 // Circular buffer of digital power meas. #1
EXTERN UInt16 shared_mon_dig_pwr2[ FRAME_ENTRY_CNT ]			SHARED_SEC_GEN_MODEM;			 // Circular buffer of digital power meas. #2
EXTERN UInt16 shared_mon_dig_pwr3[ FRAME_ENTRY_CNT ]			SHARED_SEC_GEN_MODEM;			 // Circular buffer of digital power meas. #3
EXTERN T_NEW_TXBUF shared_tx_buff_high							SHARED_SEC_GEN_MODEM;	
EXTERN T_NEW_RXBUF shared_rx_buff_high							SHARED_SEC_GEN_MODEM;	
EXTERN TxBuf_t shared_tx_buff[ EDGE_TX_BUFF_CNT]				SHARED_SEC_GEN_MODEM;	
EXTERN RxBuf_t shared_rx_buff[ EDGE_RX_BUFF_CNT]				SHARED_SEC_GEN_MODEM;	
EXTERN UInt16 shared_rxqual_full[ MAX_RX_SLOTS ]				SHARED_SEC_GEN_MODEM;	
EXTERN UInt16 shared_rxqual_sub[  MAX_RX_SLOTS ]				SHARED_SEC_GEN_MODEM;	
EXTERN UInt16 shared_rxlev_full[  MAX_RX_SLOTS ]				SHARED_SEC_GEN_MODEM;	
EXTERN UInt16 shared_rxlev_sub[   MAX_RX_SLOTS ]				SHARED_SEC_GEN_MODEM;	
EXTERN UInt16 shared_FrameInt_flag    			   				SHARED_SEC_GEN_MODEM;	
EXTERN UInt16 shared_selectively_enable_class12_mods			SHARED_SEC_GEN_MODEM;			 //bit3: set class 12 only otherwise class 33;
EXTERN UInt16 shared_fiq_decoded_usf							SHARED_SEC_GEN_MODEM;			 // Contains the decoded USF information passed up from the DSP to the ARM
EXTERN UInt16 shared_data_dtx_enabled_flag						SHARED_SEC_GEN_MODEM;	
EXTERN Int16 shared_EFR2FR_transition_flag						SHARED_SEC_GEN_MODEM;	
EXTERN UInt16 shared_LSDCoffset_cancel_enable  					SHARED_SEC_GEN_MODEM;			 //LS DC offset cancellation flag
EXTERN UInt16 shared_pre_filter_bypass_enable  					SHARED_SEC_GEN_MODEM;			 //bypass the pre emphysis filter
EXTERN UInt16 shared_ff_freq_derot_8psk_enable 					SHARED_SEC_GEN_MODEM;			 //8PSK FF freq correction flag
EXTERN UInt16 shared_ahs_ratscch_marker_disable	   				SHARED_SEC_GEN_MODEM;	
EXTERN UInt16 shared_new_serv_band								SHARED_SEC_GEN_MODEM;
EXTERN UInt16	shared_dsp_arm_burst_sig_pow	   				SHARED_SEC_GEN_MODEM;	
EXTERN UInt16 shared_power_thresh_db16			   				SHARED_SEC_GEN_MODEM;	
EXTERN Int16 shared_snr_thresh_db16_8PSK		   				SHARED_SEC_GEN_MODEM;			  // This one has been used to pass new SNR threshold in dB16 for 8PSK tracking algorithm
EXTERN UInt16 shared_dynamic_tx_spectrum_inversion 				SHARED_SEC_GEN_MODEM;			  // Enables dynamic TX spectrum inversion code. It is off by default.
EXTERN UInt16 shared_FACCH_combine								SHARED_SEC_GEN_MODEM;			  // FACCH combining control
EXTERN UInt16 shared_SACCH_combine   							SHARED_SEC_GEN_MODEM;			  // SACCH combining control
EXTERN UInt16 shared_status_FACCH_combine						SHARED_SEC_GEN_MODEM;	
EXTERN UInt16 shared_status_SACCH_combine						SHARED_SEC_GEN_MODEM;	
EXTERN UInt16	shared_prev_FACCH_frame_min						SHARED_SEC_GEN_MODEM;	
EXTERN UInt16	shared_prev_FACCH_frame_max						SHARED_SEC_GEN_MODEM;	
EXTERN UInt16 shared_dc_offset_alpha							SHARED_SEC_GEN_MODEM;	
EXTERN UInt16 shared_dc_offset_correction_mode					SHARED_SEC_GEN_MODEM;	
EXTERN Int16 shared_dsp_arm_burst_snr							SHARED_SEC_GEN_MODEM;	
EXTERN UInt16 shared_rxsamp_short_rx_cnt						SHARED_SEC_GEN_MODEM;	
EXTERN Int16  shared_derot_scale								SHARED_SEC_GEN_MODEM;			 // Derotator offset scale
EXTERN Int16  shared_derot_gmsk									SHARED_SEC_GEN_MODEM;			 // Derotator gmsk offset (to derotate)
EXTERN Int16  shared_fft_binscale								SHARED_SEC_GEN_MODEM;			 // EstFreq:  squared FFT-bin scaling
EXTERN Int16  shared_fft_bin_db16								SHARED_SEC_GEN_MODEM;	
EXTERN Int16  shared_fft_thresh_db16							SHARED_SEC_GEN_MODEM;	
EXTERN Int16  shared_sch_thresh_db16							SHARED_SEC_GEN_MODEM;	
EXTERN UInt16 shared_adapt_target								SHARED_SEC_GEN_MODEM;			 // Target bit position for equalizer
EXTERN UInt16 shared_adapt_k2									SHARED_SEC_GEN_MODEM;			 // K^2 for equalizer
EXTERN UInt16 shared_ber_qlevel[ BER_SIZE ]						SHARED_SEC_GEN_MODEM;			 // Table for converting BER to 3-bit RXQUAL
EXTERN UInt16 shared_quant_scale_8PSK							SHARED_SEC_GEN_MODEM;			 // Quantization space for EDGE 8PSK equalizer. Default 12
EXTERN UInt16 shared_num_accum_blind							SHARED_SEC_GEN_MODEM;			 // # of bursts used for accumulation of blind detection. Default 3
EXTERN Int16  shared_usf_snr_thresh_db16_8PSK					SHARED_SEC_GEN_MODEM;			 // SNR threshold to count bad burst for USF detection of 8PSK. Default 48=3*16
EXTERN Int16 shared_usf_snr_thresh_db16_GMSK					SHARED_SEC_GEN_MODEM;	
EXTERN UInt16 shared_null_pg_blk_last_blk_ind					SHARED_SEC_GEN_MODEM;	
EXTERN UInt16 shared_null_pg_blk_error_cnt[MAX_NUM_TEMPLATES]	SHARED_SEC_GEN_MODEM;	
EXTERN Int16 shared_null_pg_blk_burst_snr						SHARED_SEC_GEN_MODEM;	
EXTERN UInt16 shared_null_pg_burst_activity_mask				SHARED_SEC_GEN_MODEM;	
EXTERN UInt16 shared_null_pg_blk_templates_to_match				SHARED_SEC_GEN_MODEM;	
EXTERN UInt16 shared_null_pg_blk_error_cnt_th[MAX_NUM_TEMPLATES] SHARED_SEC_GEN_MODEM;	
EXTERN UInt16 shared_null_pg_matched_template_idx				SHARED_SEC_GEN_MODEM;	
EXTERN UInt16 shared_bec_bfi[1]									SHARED_SEC_GEN_MODEM;			 //bec and bfi of last RX block.
EXTERN UInt16 shared_vit_metric[1]								SHARED_SEC_GEN_MODEM;			 //Viterbi decoded path metric of last block.
EXTERN Int16 shared_SNR[4]						    			SHARED_SEC_GEN_MODEM;			 //downlink burst SNRs (4 in full rate, 2 in half rate) in db16.


EXTERN UInt16 shared_pg_corr_size								SHARED_SEC_GEN_MODEM;			 // # of correlation taps
EXTERN Int16  shared_eqc_center									SHARED_SEC_GEN_MODEM;	
EXTERN UInt16 shared_eqc_disable32								SHARED_SEC_GEN_MODEM;	
EXTERN UInt16 shared_eqc_range									SHARED_SEC_GEN_MODEM;	
EXTERN UInt16 shared_eqc_reduce_range							SHARED_SEC_GEN_MODEM;	
EXTERN Int16  shared_qbc_umts_adjust							SHARED_SEC_GEN_MODEM;	
EXTERN UInt16 shared_qbc_short									SHARED_SEC_GEN_MODEM;			 // Shortest frame size (depends on tune_time)
EXTERN Int16 shared_thresh_timing_diff				 			SHARED_SEC_GEN_MODEM;								 // threshold for adaptive selection of the number of GMSK EQ. ( 300 = 3*100 )
EXTERN Int16 shared_asymmetric_ms_tx_flag			  			SHARED_SEC_GEN_MODEM;								 // Enable/Disable adding 1 symbol offset for the TX slots after the first one for asymmetric uplink tester
EXTERN Int16 shared_adap_fir[ 15 ]								SHARED_SEC_GEN_MODEM;
EXTERN Int16 shared_thresh_SNR_dc_est[ 4 ] 						SHARED_SEC_GEN_MODEM;
EXTERN Int16 shared_wgh_LMMSE[ 5 ] 								SHARED_SEC_GEN_MODEM;
EXTERN Int16 shared_length_adap_fir								SHARED_SEC_GEN_MODEM;
EXTERN Int16 shared_thresh_SNR_diff_adj_8PSK 					SHARED_SEC_GEN_MODEM;
EXTERN Int16 shared_thresh_SNR_diff_adj_GMSK					SHARED_SEC_GEN_MODEM;
EXTERN Int16 shared_thresh_SNR_PS_diff_adj_GMSK					SHARED_SEC_GEN_MODEM;								// needs to be initialized for the packet switch
EXTERN UInt16 shared_flag_adap_filter							SHARED_SEC_GEN_MODEM;
EXTERN UInt16 	shared_half_facch_thresh						SHARED_SEC_GEN_MODEM;
EXTERN UInt16 	shared_half_facch_metric_scale					SHARED_SEC_GEN_MODEM;								// Normallized by 256 in DSP, i.e. right shift 8
EXTERN Int16	shared_snr_thresh_sacch							SHARED_SEC_GEN_MODEM;
EXTERN UInt16 shared_frame_tx_ind								SHARED_SEC_GEN_MODEM;								//Indicator if the next frame will TX or not.
EXTERN UInt16 shared_gmsk_one_qbc_THRESH						SHARED_SEC_GEN_MODEM;
EXTERN UInt16 shared_gmsk_two_qbc_THRESH						SHARED_SEC_GEN_MODEM;
EXTERN UInt16 shared_8psk_one_qbc_THRESH						SHARED_SEC_GEN_MODEM;
EXTERN UInt16 shared_8psk_two_qbc_THRESH						SHARED_SEC_GEN_MODEM;
EXTERN Int16 shared_asymmetric_qbc_offset						SHARED_SEC_GEN_MODEM;
EXTERN UInt16 shared_gprs_enc_time_margin						SHARED_SEC_GEN_MODEM;
EXTERN Int16		shared_ctm_mode								SHARED_SEC_GEN_MODEM;
EXTERN ctmRx_t		shared_ctmRx								SHARED_SEC_GEN_MODEM;
EXTERN ctmTx_t		shared_ctmTx								SHARED_SEC_GEN_MODEM;
EXTERN Int16		shared_ctmBuff[16]							SHARED_SEC_GEN_MODEM;
EXTERN UInt16	shared_ch_gain[16]								SHARED_SEC_GEN_MODEM;
EXTERN Int16 shared_null_pg_snr_th                             	SHARED_SEC_GEN_MODEM;
EXTERN UInt16 shared_null_pg_qbc_offset_th                      SHARED_SEC_GEN_MODEM;
EXTERN Int16 shared_null_pg_qbc_offset[MAX_NUM_TEMPLATES]     	SHARED_SEC_GEN_MODEM;
EXTERN UInt16 shared_ncell_adjust[DSP_SYNC_LIST_SIZE/4]			SHARED_SEC_GEN_MODEM;
EXTERN UInt16 shared_ncell_freq_track_limit						SHARED_SEC_GEN_MODEM;
EXTERN UInt16 shared_ncell_freq_track_shift						SHARED_SEC_GEN_MODEM;
EXTERN Int16 shared_ncell_freq_error_limit						SHARED_SEC_GEN_MODEM;
EXTERN UInt16 shared_frame_entry_refresh_flag					SHARED_SEC_GEN_MODEM;

EXTERN UInt16 shared_MST_flag_EFR								SHARED_SEC_MST_SAIC	;
EXTERN UInt16 shared_MST_flag_AFS								SHARED_SEC_MST_SAIC	;
EXTERN UInt16 shared_MST_flag_AHS								SHARED_SEC_MST_SAIC	;
EXTERN UInt16 shared_MST_flag_CS1								SHARED_SEC_MST_SAIC	;
EXTERN UInt16 shared_MST_EC_AFS   								SHARED_SEC_MST_SAIC	;
EXTERN UInt16 shared_MST_EC_AHS   								SHARED_SEC_MST_SAIC	;
EXTERN UInt16 shared_MST_EC_CS1   								SHARED_SEC_MST_SAIC	;
EXTERN Int16 shared_MST_MaxCyc   								SHARED_SEC_MST_SAIC	;
EXTERN UInt16 shared_MST_GlobAmrTrs0 							SHARED_SEC_MST_SAIC	;
EXTERN UInt16 shared_MST_GlobAmrTrs5 							SHARED_SEC_MST_SAIC	;
EXTERN UInt16 shared_MST_GlobAmrTrs6 							SHARED_SEC_MST_SAIC	;
EXTERN UInt16 shared_MST_BER_FACTOR 							SHARED_SEC_MST_SAIC	;
EXTERN UInt16 shared_flag_SAIC									SHARED_SEC_MST_SAIC	;												 // flag for SAIC, burst & frame; 0x0001: Enable SAIC burst; 0x0010: Enable SAIC frame
EXTERN UInt16 shared_flag_SAIC_ps_iter       					SHARED_SEC_MST_SAIC	;
EXTERN UInt16 shared_saic_adap_rxlev							SHARED_SEC_MST_SAIC	;												// range of rxlev to adaptively adjust the SNR threshold for SAIC
EXTERN Int16  shared_flag_SAIC_ps_iter_SNR_thr					SHARED_SEC_MST_SAIC	;
EXTERN UInt16 shared_snr_thresh_freq_est_SAIC					SHARED_SEC_MST_SAIC	;												// SNR threshold in dB16 for frequency estimate of SAIC; shared_snr_thresh_freq_est_SAIC=0dB
EXTERN Int16 shared_saic_fp_le_DL_1								SHARED_SEC_MST_SAIC	;												// SAIC FP LE Diagonal loading factor for snr<shared_saic_fp_le_DL_SNR_T_1
EXTERN Int16 shared_saic_fp_le_DL_SNR_T_1						SHARED_SEC_MST_SAIC	;
EXTERN Int16 shared_saic_fp_le_DL_2								SHARED_SEC_MST_SAIC	;												// SAIC FP LE Diagonal loading factor for snr<shared_saic_fp_le_DL_SNR_T_2
EXTERN Int16 shared_saic_fp_le_DL_SNR_T_2						SHARED_SEC_MST_SAIC	;
EXTERN Int16 shared_saic_fp_le_DL_3								SHARED_SEC_MST_SAIC	;												// SAIC FP LE Diagonal loading factor for snr>shared_saic_fp_le_DL_SNR_T_2
EXTERN saic_thres_t shared_saic_sw_fs_hi						SHARED_SEC_MST_SAIC	;												// SAIC switch thresholds for AFS hi rate
EXTERN saic_thres_t shared_saic_sw_fs_lo						SHARED_SEC_MST_SAIC	;												// SAIC switch thresholds for AFS lo rate
EXTERN saic_thres_t shared_saic_sw_hs_hi						SHARED_SEC_MST_SAIC	;												// SAIC switch thresholds for AHS hi rate
EXTERN saic_thres_t shared_saic_sw_hs_lo						SHARED_SEC_MST_SAIC	;												// SAIC switch thresholds for AHS lo rate
EXTERN Int16 shared_saic_bp_snr_gain_th_FS						SHARED_SEC_MST_SAIC	;												// SAIC new adapation FS SNR gain threshould
EXTERN Int16 shared_saic_bp_snr_gain_th_HS						SHARED_SEC_MST_SAIC	;												// SAIC new adapation HS SNR gain threshould
EXTERN Int16 shared_saic_ps_snr_thr								SHARED_SEC_MST_SAIC	;												// SAIC new adapation PS SNR gain threshould
EXTERN UInt16 shared_saic_bp_adap_ht3_rxlv_thr					SHARED_SEC_MST_SAIC	;												// 23  SAIC new adapation HT3 rxlv threshould
EXTERN Int16 shared_saic_bp_adap_ht3_snr_thr 					SHARED_SEC_MST_SAIC	;												// 240 SAIC new adapation HT3 SNR threshould
EXTERN Int16 shared_saic_bp_adap_ht3_timing_thr					SHARED_SEC_MST_SAIC	;												// 300 SAIC new adapation timing jitter threshould
EXTERN saic_fp_burst_data_t saic_fp_burst_data[2][4] 			SHARED_SEC_MST_SAIC	;
EXTERN UInt16 shared_saic_bias_fr				   				SHARED_SEC_MST_SAIC	;													// FR speech bec bias   
EXTERN UInt16 shared_saic_bias_efr				   				SHARED_SEC_MST_SAIC	;													// EFR speech bec bias   
EXTERN UInt16 shared_saic_bec_bias_amr[4]						SHARED_SEC_MST_SAIC	;
EXTERN UInt16 shared_saic_bec_rxlv_thresh						SHARED_SEC_MST_SAIC	;
EXTERN Int16  shared_saic_bp_snr_bias							SHARED_SEC_MST_SAIC	;
EXTERN UInt16  shared_saic_soft_symbol_scaling					SHARED_SEC_MST_SAIC	;
EXTERN Int16  shared_saic_rx_DL_snr								SHARED_SEC_MST_SAIC	;
EXTERN Int16  shared_saic_rx_DL_hi								SHARED_SEC_MST_SAIC	;
EXTERN Int16  shared_saic_rx_DL_lo								SHARED_SEC_MST_SAIC	;
EXTERN Int16 shared_SAIC_filter_tap_length						SHARED_SEC_MST_SAIC	;
EXTERN Int16  shared_SAIC_DL_SNR_th								SHARED_SEC_MST_SAIC	;
EXTERN Int16  shared_SAIC_lo_SNR_DL								SHARED_SEC_MST_SAIC	;
EXTERN Int16  shared_SAIC_hi_SNR_DL								SHARED_SEC_MST_SAIC	;
EXTERN Int16  shared_SAIC_BP_timing								SHARED_SEC_MST_SAIC	;
EXTERN Int16  shared_SAIC_soft_th								SHARED_SEC_MST_SAIC	;
EXTERN Int16  shared_SAIC_softBitScale							SHARED_SEC_MST_SAIC	;
EXTERN UInt16 shared_CS4_modulation_flag						SHARED_SEC_MST_SAIC	;																					
																
EXTERN UInt16 shared_ufi_th_set[4]		 	        		   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_MST_ufi_th_set[4]		 	    		   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_ufi_th_dtx_set[4]		 				   SHARED_SEC_PARAMET_MODEM;								
EXTERN UInt16 shared_ufi_thresh_hr							   SHARED_SEC_PARAMET_MODEM;									// Half rate speech unreliable frame indicator
EXTERN UInt16 shared_ufi_thresh_dtx_hr						   SHARED_SEC_PARAMET_MODEM;									// DTX HS unreliable frame indicator
EXTERN UInt16 shared_ufi_thresh_dtx_flp_hr					   SHARED_SEC_PARAMET_MODEM;									// DTX changing period HS unreliable frame indicator
EXTERN UInt16 shared_sf_fr									   SHARED_SEC_PARAMET_MODEM;									// Full rate speech scale factor
EXTERN UInt16 shared_sf_hr									   SHARED_SEC_PARAMET_MODEM;									// was constant in HS.
EXTERN Int16  shared_MST_sf_fr                   			   SHARED_SEC_PARAMET_MODEM;									// Full rate speech SNR     threshold in        dB16  
EXTERN UInt16 shared_sf_dtx_fr								   SHARED_SEC_PARAMET_MODEM;									// DTX FR scale factor
EXTERN UInt16 shared_sf_dtx_hr								   SHARED_SEC_PARAMET_MODEM;									// DTX HS scale factor
EXTERN UInt16 shared_sf_dtx_flp_hr							   SHARED_SEC_PARAMET_MODEM;									// DTX changing period HS scale factor
EXTERN UInt16 shared_sf_set[4]		 		        		   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_MST_sf_set[4]		 		    		   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_sf_dtx_set[4]		 					   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_MST_bfi_bec_thresh_dtx_EFR				   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_MST_bfi_bec_thresh_voice_EFR			   SHARED_SEC_PARAMET_MODEM;
EXTERN Int16  shared_bfi_snr_thresh_db16_fr					   SHARED_SEC_PARAMET_MODEM;									// Full rate speech SNR threshold in dB16
EXTERN UInt16 shared_MST_bfi_snr_thresh_db16_fr  			   SHARED_SEC_PARAMET_MODEM;									// Full rate speech SNR     counter   threshold
EXTERN Int16  shared_bfi_snr_thresh_db16_hr					   SHARED_SEC_PARAMET_MODEM;									// Half rate speech SNR threshold in dB16
EXTERN Int16  shared_bfi_snr_thresh_db16_dtx_fr				   SHARED_SEC_PARAMET_MODEM;									// DTX FR SNR in db16
EXTERN Int16  shared_bfi_snr_thresh_db16_dtx_hr				   SHARED_SEC_PARAMET_MODEM;									// DTX HS SNR in db16
EXTERN Int16  shared_bfi_snr_thresh_db16_dtx_flp_hr			   SHARED_SEC_PARAMET_MODEM;									// DTX changing period HS SNR in db16
EXTERN UInt16 shared_bfi_thresh_dtx_fr						   SHARED_SEC_PARAMET_MODEM;									// DTX FR bad frame indicator
EXTERN UInt16 shared_bfi_thresh_dtx_hr						   SHARED_SEC_PARAMET_MODEM;									// DTX HS bad frame indicator
EXTERN UInt16 shared_bfi_thresh_fr							   SHARED_SEC_PARAMET_MODEM;									// Full rate speech bad frame indicator
EXTERN UInt16 shared_MST_bfi_thresh_fr           			   SHARED_SEC_PARAMET_MODEM;									// Full rate speech viterbi decoder   final     metric upper bound
EXTERN UInt16 shared_bfi_thresh_hr							   SHARED_SEC_PARAMET_MODEM;									// Half rate speech bad frame indicator
EXTERN UInt16 shared_MST_bfi_snr_cnt_thresh_fr   			   SHARED_SEC_PARAMET_MODEM;									// Full rate speech scale   factor   
EXTERN UInt16 shared_bfi_snr_cnt_thresh_fr					   SHARED_SEC_PARAMET_MODEM;									// Full rate speech SNR counter threshold
EXTERN UInt16 shared_bfi_snr_cnt_thresh_hr					   SHARED_SEC_PARAMET_MODEM;									// Half rate speech SNR counter threshold
EXTERN UInt16 shared_bfi_snr_cnt_thresh_dtx_fr				   SHARED_SEC_PARAMET_MODEM;									// DTX FR SNR counter
EXTERN UInt16 shared_bfi_snr_cnt_thresh_dtx_hr				   SHARED_SEC_PARAMET_MODEM;									// DTX HS SNR counter
EXTERN UInt16 shared_bfi_snr_cnt_thresh_dtx_flp_hr			   SHARED_SEC_PARAMET_MODEM;									// DTX changing period HS SNR counter
EXTERN UInt16 shared_bfi_vocoder_metric_bound_m_fr  		   SHARED_SEC_PARAMET_MODEM;									// Full rate speech pproc final metric threshold
EXTERN UInt16 shared_bfi_vocoder_reencode_bound_m_fr		   SHARED_SEC_PARAMET_MODEM;									// Full rate speech BEC threshold  
EXTERN UInt16 shared_bfi_vocoder_metric_bound_h_fr  		   SHARED_SEC_PARAMET_MODEM;									// Full rate speech pproc final metric  threshold
EXTERN UInt16 shared_bfi_vocoder_reencode_bound_h_fr		   SHARED_SEC_PARAMET_MODEM;									// Full rate speech BEC threshold   
EXTERN UInt16 shared_bfi_delta_amp_sum_th_fr  				   SHARED_SEC_PARAMET_MODEM;									// Full rate speech SNR blokc amp variation threshold
EXTERN UInt16 shared_bfi_thresh_dtx_flp_hr					   SHARED_SEC_PARAMET_MODEM;									// DTX changing period HS bad frame indicator
EXTERN UInt16 shared_bfi_th_dtx_set[4]		 				   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_MST_bfi_th_set[4]		 	  			   SHARED_SEC_PARAMET_MODEM;
EXTERN Int16  shared_bfi_snr_thresh_db16_dtx_set[4]			   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_bfi_snr_cnt_thresh_dtx_set[4]			   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_bfi_th_set[4]		 	  				   SHARED_SEC_PARAMET_MODEM;
EXTERN Int16  shared_MST_bfi_snr_thresh_db16_set[4]			   SHARED_SEC_PARAMET_MODEM;
EXTERN Int16  shared_bfi_snr_thresh_db16_set[4]				   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_bfi_snr_cnt_thresh_set[4]				   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_MST_bfi_snr_cnt_thresh_set[4]		       SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_ber_reencode_bound_l					   SHARED_SEC_PARAMET_MODEM;									// Half rate speech BER lower bound
EXTERN UInt16 shared_ber_reencode_bound_h					   SHARED_SEC_PARAMET_MODEM;									// Half rate speech BER upper bound
EXTERN UInt16 shared_ber_reencode_bound_l_hr				   SHARED_SEC_PARAMET_MODEM;									// Half rate speech BER lower bound
EXTERN UInt16 shared_ber_reencode_bound_h_hr				   SHARED_SEC_PARAMET_MODEM;									// Half rate speech BER upper bound
EXTERN UInt16 shared_ber_reencode_bound_l_dtx_hr 			   SHARED_SEC_PARAMET_MODEM;									// DTX HS BER lower bound
EXTERN UInt16 shared_ber_reencode_bound_h_dtx_hr			   SHARED_SEC_PARAMET_MODEM;									// DTX HS BER upper bound
EXTERN UInt16 shared_ber_reencode_bound_l_dtx_flp_hr 		   SHARED_SEC_PARAMET_MODEM;												// DTX changing period HS BER lower bound
EXTERN UInt16 shared_ber_reencode_bound_h_dtx_flp_hr		   SHARED_SEC_PARAMET_MODEM;												// DTX changing period HS BER upper bound
EXTERN UInt16 shared_ber_reencode_bound_l_fr				   SHARED_SEC_PARAMET_MODEM;									// Full rate speech BER lower bound
EXTERN UInt16 shared_ber_reencode_bound_h_fr				   SHARED_SEC_PARAMET_MODEM;									// Full rate speech BER upper bound
EXTERN UInt16 shared_ber_reencode_bound_l_dtx_fr 			   SHARED_SEC_PARAMET_MODEM;									// DTX FR BER lower bound
EXTERN UInt16 shared_ber_reencode_bound_h_dtx_fr			   SHARED_SEC_PARAMET_MODEM;									// DTX FR BER upper bound
EXTERN UInt16 shared_MST_ber_reencode_bound_l_fr 			   SHARED_SEC_PARAMET_MODEM;									// Full rate speech BER     upper     bound    
EXTERN UInt16 shared_MST_ber_reencode_bound_h_fr 			   SHARED_SEC_PARAMET_MODEM;									// Full rate speech viterbi decoder   final     metric lower bound
EXTERN UInt16 shared_vit_metric_bound_l         			   SHARED_SEC_PARAMET_MODEM;									// Half rate speech viterbi decoder final metric lower bound
EXTERN UInt16 shared_vit_metric_bound_h         			   SHARED_SEC_PARAMET_MODEM;									// Half rate speech viterbi decoder final metric upper bound
EXTERN UInt16 shared_vit_metric_bound_l_hr         			   SHARED_SEC_PARAMET_MODEM;									// Half rate speech viterbi decoder final metric lower bound
EXTERN UInt16 shared_vit_metric_bound_h_hr         			   SHARED_SEC_PARAMET_MODEM;									// Half rate speech viterbi decoder final metric upper bound
EXTERN UInt16 shared_vit_metric_bound_l_dtx_hr     			   SHARED_SEC_PARAMET_MODEM;									// DTX HS viterbi decoder final metric lower bound
EXTERN UInt16 shared_vit_metric_bound_h_dtx_hr     			   SHARED_SEC_PARAMET_MODEM;									// DTX HS viterbi decoder final metric upper bound
EXTERN UInt16 shared_vit_metric_bound_h_dtx_flp_hr  		   SHARED_SEC_PARAMET_MODEM;									// DTX changing period HS viterbi decoder final metric upper bound
EXTERN UInt16 shared_vit_metric_bound_l_dtx_flp_hr  		   SHARED_SEC_PARAMET_MODEM;									// DTX changing period HS viterbi decoder final metric lower bound
EXTERN UInt16 shared_MST_vit_metric_bound_l_fr   			   SHARED_SEC_PARAMET_MODEM;									// Full rate speech bad     frame     indicator
EXTERN UInt16 shared_MST_vit_metric_bound_h_fr   			   SHARED_SEC_PARAMET_MODEM;									// Full rate speech BER     lower     bound    
EXTERN UInt16 shared_vit_metric_bound_l_fr         			   SHARED_SEC_PARAMET_MODEM;									// Full rate speech viterbi decoder final metric lower bound
EXTERN UInt16 shared_vit_metric_bound_h_fr         			   SHARED_SEC_PARAMET_MODEM;									// Full rate speech viterbi decoder final metric upper bound
EXTERN UInt16 shared_vit_metric_bound_l_dtx_fr     			   SHARED_SEC_PARAMET_MODEM;									// DTX FR viterbi decoder final metric lower bound
EXTERN UInt16 shared_vit_metric_bound_h_dtx_fr     			   SHARED_SEC_PARAMET_MODEM;									// DTX FR viterbi decoder final metric upper bound
EXTERN UInt16 shared_MST_reencode_bound_l_set[4] 			   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_MST_reencode_bound_h_set[4] 			   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_reencode_bound_l_set[4] 				   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_reencode_bound_h_set[4] 				   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_reencode_bound_l_dtx_set[4] 			   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_reencode_bound_h_dtx_set[4] 			   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_metric_bound_l_set[4]    				   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_metric_bound_h_set[4]   				   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_MST_metric_bound_l_set[4]    			   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_MST_metric_bound_h_set[4]   			   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_metric_bound_l_dtx_set[4]      		   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_metric_bound_h_dtx_set[4]   			   SHARED_SEC_PARAMET_MODEM;
EXTERN Int16 shared_amr_cgain[7]		 	        		   SHARED_SEC_PARAMET_MODEM;	
EXTERN Int16 shared_amr_pgain[7]		 	  				   SHARED_SEC_PARAMET_MODEM;		
EXTERN UInt16 shared_AMR_dlink_ICM_ACS  					   SHARED_SEC_PARAMET_MODEM;	// 2 LSBs in high byte is ICM, low byte is ACS.
EXTERN UInt16 shared_AMR_ulink_ICM_ACS  					   SHARED_SEC_PARAMET_MODEM;	// 2 LSBs in high byte is ICM, low byte is ACS. 
EXTERN UInt16  shared_AHS_sid_update_reencode_bound_h		   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_p_ALPHA_AFS  							   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_p_BETA_AFS  							   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_variance_INBAND_AFS  					   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_p_ALPHA_AHS  							   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_p_BETA_AHS  							   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_variance_INBAND_AHS  					   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_distance_L_SID_UPDATE_AFS  			   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_distance_H_SID_UPDATE_AFS  			   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_distance_L_SID_FIRST_AFS  				   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_distance_L_ONSET_AFS  					   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_distance_L_RATSCCH_AFS  				   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_distance_H_RATSCCH_AFS  				   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_distance_L_SID_UPDATE_AHS  			   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_distance_H_SID_UPDATE_AHS  			   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_distance_L_SID_UPDATE_INH_AHS  		   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_distance_L_SID_FIRST_P1_AHS  			   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_distance_L_SID_FIRST_INH_AHS  			   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_distance_L_ONSET_AHS  					   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_distance_L_RATSCCH_MARKER_AHS  		   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_distance_H_RATSCCH_MARKER_AHS  		   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_inband_distance_thresh					   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_serv_cell								   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_cell_index								   SHARED_SEC_PARAMET_MODEM;
EXTERN baudotTx_t	shared_baudotTx							   SHARED_SEC_PARAMET_MODEM;
EXTERN baudotRx_t	shared_baudotRx							   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_flag_adap_DC							   SHARED_SEC_PARAMET_MODEM;
EXTERN Int16  shared_pdch_C_thresh							   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_bfi_steal_bec_thresh					   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_facch_thresh							   SHARED_SEC_PARAMET_MODEM;		// FACCH detection threshold.
EXTERN Int16 shared_bfi_snr_thresh_db16						   SHARED_SEC_PARAMET_MODEM;
EXTERN Int16 shared_MST_bfi_snr_thresh_db16					   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_bfi_snr_cnt_thresh						   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_bfi_steal_bec_thresh1					   SHARED_SEC_PARAMET_MODEM;		// Unused 
EXTERN UInt16 shared_bfi_steal_bit_low_thresh				   SHARED_SEC_PARAMET_MODEM;		// Unused
EXTERN UInt16 shared_bfi_steal_bit_high_thresh				   SHARED_SEC_PARAMET_MODEM;		// Unused
EXTERN UInt16 shared_bfi_bec_thresh_voice					   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_bfi_bec_thresh_dtx						   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_continuous_bfi_thresh					   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_mute_FACCH_noise_enable				   SHARED_SEC_PARAMET_MODEM;	// HR mute frame cnt
EXTERN UInt16 shared_mute_FACCH_noise_cnt					   SHARED_SEC_PARAMET_MODEM;	// mute frame cnt
EXTERN UInt16 shared_mute_FACCH_noise_FR					   SHARED_SEC_PARAMET_MODEM;	// FR mute frame cnt
EXTERN UInt16 shared_mute_FACCH_noise_HR					   SHARED_SEC_PARAMET_MODEM;	// HR mute frame cnt
EXTERN UInt16 shared_sid_thresh0_hr							   SHARED_SEC_PARAMET_MODEM;		// Half rate speech SID threshold
EXTERN UInt16 shared_sid_thresh1_hr							   SHARED_SEC_PARAMET_MODEM;		// Half rate speech SID threshold
EXTERN UInt16 shared_EOTD_Timestamp							   SHARED_SEC_PARAMET_MODEM;		//Note: only used if EOTD is defined!!!
EXTERN UInt16 shared_HT_time_jitter_thr_lo 					   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_HT_time_jitter_thr_hi 					   SHARED_SEC_PARAMET_MODEM;
//********************* Slow Clock (32KHz) tracking parameters **********************
EXTERN UInt16 shared_slow_clock_track_enable			SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_slow_clock_alpha					SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_cal_ratio_scale					SHARED_SEC_PARAMET_MODEM;
EXTERN UInt32 shared_slow_clock_limit					SHARED_SEC_PARAMET_MODEM;
EXTERN UInt32 shared_accum_clks_cnt						SHARED_SEC_PARAMET_MODEM;
EXTERN Int32 shared_cal_ratio_delta						SHARED_SEC_PARAMET_MODEM;
EXTERN Int16 shared_accum_time_adj						SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_slow_clock_track_flag				SHARED_SEC_PARAMET_MODEM;
EXTERN UInt32 shared_cal_ratio_out						SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_8psk_equalizer_step_size			SHARED_SEC_PARAMET_MODEM;

EXTERN UInt16 RESERVED_PART1[SIZE_OF_RESERVED_ZONE-17]			   SHARED_SEC_PARAMET_MODEM;	

				
EXTERN FSC_Records_t shared_fsc_records						 SHARED_SEC_SMC;		// 80 UInt32 word array
EXTERN SMC_Records_t shared_smc_records						 SHARED_SEC_SMC;		// 80 UInt16 word array
//folowwing one will be not needed in SMC_NPG 
EXTERN UInt16 shared_curr_smc_idx_and_page					 SHARED_SEC_SMC;				// Log the current HW SMC page and index when the DSP generates the RFIC interrupt
EXTERN UInt16 shared_min_sleep_frames						 SHARED_SEC_SMC;
EXTERN UInt16 shared_deep_sleep_3wire_bus_pwr_dwn_enable  	 SHARED_SEC_SMC;
EXTERN UInt16 shared_deep_sleep_CHBR						 SHARED_SEC_SMC;	// 
EXTERN UInt16 shared_deep_sleep_wakeup_CHBR					 SHARED_SEC_SMC;	// 
EXTERN UInt16 shared_deep_sleep_wakeup_frame_len_qbc		 SHARED_SEC_SMC;	// 
EXTERN UInt16 shared_deep_sleep_cnt							 SHARED_SEC_SMC;	// 
EXTERN UInt16 shared_deep_sleep_SLEEPCAL_IsReady			 SHARED_SEC_SMC;	// 
EXTERN UInt16 shared_deep_sleep_pg_aud_tone_status			 SHARED_SEC_SMC;	// 
EXTERN UInt16 shared_deep_sleep_RFIC_request				 SHARED_SEC_SMC;	// 
EXTERN UInt16 shared_deep_sleep_RFIC_return					 SHARED_SEC_SMC;	// 
EXTERN UInt16 shared_deep_sleep_num_smc_rec					 SHARED_SEC_SMC;	// 
EXTERN UInt16 shared_enable_RFIC_deep_sleep_plotting		 SHARED_SEC_SMC;	// 
EXTERN UInt16 shared_deep_sleep_FQCR_val  					 SHARED_SEC_SMC;
EXTERN UInt16 shared_deep_sleep_FQC2R_val  					 SHARED_SEC_SMC;
EXTERN UInt16 shared_wakeup_FQCR_val				  		 SHARED_SEC_SMC;
EXTERN UInt16 shared_wakeup_FQC2R_val				  		 SHARED_SEC_SMC;
EXTERN Int16  shared_wakeup_qbc_fudge			   			 SHARED_SEC_SMC;
//folowwing two will be not needed in SMC_NPG 
EXTERN UInt16 shared_fs_index_extra				   			 SHARED_SEC_SMC;	// Indicates extra FSC words which need to get loded after the RX and before the MON event
EXTERN UInt16 shared_fsc_extra_records[20]		   			 SHARED_SEC_SMC;
EXTERN UInt16 shared_entry_index				   			 SHARED_SEC_SMC;
EXTERN UInt16 shared_qbc_frame					   			 SHARED_SEC_SMC;
EXTERN UInt16 shared_rx_start_delay				   			 SHARED_SEC_SMC;
EXTERN UInt16 shared_tx_start_delay				   			 SHARED_SEC_SMC;
EXTERN UInt16 shared_tx_stop_delay				   			 SHARED_SEC_SMC;
EXTERN UInt16 shared_mon_start_delay			   			 SHARED_SEC_SMC;
EXTERN UInt16 shared_fcwr_value			   			 		 SHARED_SEC_SMC;
EXTERN UInt16 shared_sfr_index_rfwakeup			   			 SHARED_SEC_SMC;
EXTERN UInt16 shared_sfr_index_phase1			   			 SHARED_SEC_SMC;
EXTERN UInt16 shared_sfr_index_phase2			   			 SHARED_SEC_SMC;
EXTERN UInt16 shared_smc_tx_record_start					 SHARED_SEC_SMC;	
EXTERN UInt16 shared_smc_tx_record_length					 SHARED_SEC_SMC;	
//folowwing three will be not needed in SMC_NPG 
EXTERN UInt16 shared_fs_index_rfwakeup			   			 SHARED_SEC_SMC;
EXTERN UInt16 shared_fs_index_phase1			   			 SHARED_SEC_SMC;
EXTERN UInt16 shared_fs_index_phase2			   			 SHARED_SEC_SMC;
EXTERN UInt16 shared_next_rxspan				   			 SHARED_SEC_SMC;
EXTERN UInt16 shared_next_mspatt				   			 SHARED_SEC_SMC;
EXTERN UInt16 shared_next_txspan				   			 SHARED_SEC_SMC;
EXTERN UInt16 shared_usf_match				   				 SHARED_SEC_SMC;
EXTERN UInt16 shared_rxtxgap					   			 SHARED_SEC_SMC;
EXTERN UInt16 shared_txmode						   			 SHARED_SEC_SMC;
EXTERN UInt16 shared_rxmode						   			 SHARED_SEC_SMC;
EXTERN UInt16 shared_sfr_index					   			 SHARED_SEC_SMC;
EXTERN UInt16 shared_phase1						   			 SHARED_SEC_SMC;
EXTERN UInt16 shared_phase2						   			 SHARED_SEC_SMC;
EXTERN UInt16 shared_rfwakeup					   			 SHARED_SEC_SMC;
//folowwing three will be not needed in SMC_NPG 
EXTERN UInt16 shared_rfic_records_extend		   			 SHARED_SEC_SMC;
EXTERN UInt16 shared_rfic_tx_records_page		   			 SHARED_SEC_SMC;
EXTERN UInt16 shared_rfic_rx_records_page		   			 SHARED_SEC_SMC;
EXTERN UInt16 shared_rfic_request_timestamp		   			 SHARED_SEC_SMC;					// Add a timestamp when the DSP generates the RFIC interrupt, does not need to be initialized.

EXTERN Int16 shared_freq_limit_FF				   			 SHARED_SEC_AGC_AFC_RFIC;				// feed-forward frequency correction limit for GMSK. ( 242 = (1000/SYM_RATE)*512*128 )
EXTERN UInt16 shared_flag_DC_freq_FF_GMSK		   			 SHARED_SEC_AGC_AFC_RFIC;				// flag for GMSK DC & FF frequency corrrection; 0x0010: Enable FF frequency; 0x0001: Enable DC
EXTERN Int16  shared_freq_dacstep_thresh		   			 SHARED_SEC_AGC_AFC_RFIC;				// FreqDAC:  freq. threshold to adjust FreqDAC
EXTERN Int16  shared_freq_derotmult				   			 SHARED_SEC_AGC_AFC_RFIC;				// Run(FCCH): multiplier (convert freq_est
EXTERN Int16  shared_freq_derotshift			   			 SHARED_SEC_AGC_AFC_RFIC;				// Run(FCCH): shift        to derot_offset)
EXTERN UInt16 shared_freq_invert				   			 SHARED_SEC_AGC_AFC_RFIC;				// Frequency band Q-sample inversion
EXTERN Int16  shared_freq_dacmult				   			 SHARED_SEC_AGC_AFC_RFIC;				// FreqDAC:  multiplier of frequency sum
EXTERN Int16  shared_freq_dacshift				   			 SHARED_SEC_AGC_AFC_RFIC;				// FreqDAC:  shift of frequency sum
EXTERN Int16  shared_freq_daczero				   			 SHARED_SEC_AGC_AFC_RFIC;				// FreqDAC:  shift of frequency sum
EXTERN Int16  shared_freq_daclsb				   			 SHARED_SEC_AGC_AFC_RFIC;				// FreqDAC:  step size to adjust FreqDAC
EXTERN UInt16 shared_freq_dcsmult				   			 SHARED_SEC_AGC_AFC_RFIC;
EXTERN Int16  shared_freq_dcsshift				   			 SHARED_SEC_AGC_AFC_RFIC;
EXTERN Int16 shared_freq_limit_FB				   			 SHARED_SEC_AGC_AFC_RFIC;				// Freq estimation uplimit
EXTERN Int16 shared_freq_dac_offset				   			 SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_max_AFC_adj_loop_cnt		   			 SHARED_SEC_AGC_AFC_RFIC;
EXTERN Int16  shared_txfreq_mult[  N_BANDS ][N_MODUS]		 SHARED_SEC_AGC_AFC_RFIC;
EXTERN Int16  shared_txfreq_shift[ N_BANDS ][N_MODUS]		 SHARED_SEC_AGC_AFC_RFIC;
EXTERN Int16 shared_freqloop_offset[ DSP_SYNC_LIST_SIZE ]	 SHARED_SEC_AGC_AFC_RFIC;
EXTERN Int16 shared_K1_mult1_null_pg						 SHARED_SEC_AGC_AFC_RFIC;
EXTERN Int16 shared_K1_shift1_null_pg						 SHARED_SEC_AGC_AFC_RFIC;
EXTERN Int16 shared_K2_mult2_null_pg 						 SHARED_SEC_AGC_AFC_RFIC;
EXTERN Int16 shared_K2_shift2_null_pg						 SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_avgflag_null_pg						 SHARED_SEC_AGC_AFC_RFIC;
EXTERN Int16 shared_K1_mult1_TBF							 SHARED_SEC_AGC_AFC_RFIC;
EXTERN Int16 shared_K1_shift1_TBF							 SHARED_SEC_AGC_AFC_RFIC;
EXTERN Int16 shared_K2_mult2_TBF							 SHARED_SEC_AGC_AFC_RFIC;
EXTERN Int16 shared_K2_shift2_TBF							 SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_avgflag_TBF							 SHARED_SEC_AGC_AFC_RFIC;
EXTERN Int16  shared_snr_thresh_db16						 SHARED_SEC_AGC_AFC_RFIC;						// SNR threshold in dB16 for tracking algorithm
EXTERN Int16  shared_leak_thresh_db16						 SHARED_SEC_AGC_AFC_RFIC;
EXTERN Int16 shared_first_path_timing						 SHARED_SEC_AGC_AFC_RFIC;
EXTERN Int16 shared_snr_thresh_db16_8PSK_timing				 SHARED_SEC_AGC_AFC_RFIC;						// This one has been used to pass new SNR threshold in dB16 for 8PSK tracking algorithm
EXTERN Int16 shared_long_term_time_accum					 SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16	shared_null_pg_time_tracking_num			 SHARED_SEC_AGC_AFC_RFIC;	
EXTERN Int16	shared_null_pg_time_tracking_shift			 SHARED_SEC_AGC_AFC_RFIC;	
EXTERN UInt16	shared_null_pg_time_tracking_mag			 SHARED_SEC_AGC_AFC_RFIC;		
EXTERN UInt16	shared_null_pg_time_tracking_scell_error_mag		SHARED_SEC_AGC_AFC_RFIC;	 
EXTERN Int16	shared_null_pg_time_tracking_scell_accum2_shift	 	SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16	shared_null_pg_time_tracking_scell_leak_thresh	 	SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16	shared_null_pg_time_tracking_enable				 	SHARED_SEC_AGC_AFC_RFIC;
EXTERN Int16  shared_agc_sat_drop_db16							 	SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_agc_nominal								 	SHARED_SEC_AGC_AFC_RFIC;			// Nominal AGC gain index on AGC reset
EXTERN UInt16 shared_agc_target									 	SHARED_SEC_AGC_AFC_RFIC;			// AGC power target, in dB16
EXTERN Int16  shared_agc_snr_thresh								 	SHARED_SEC_AGC_AFC_RFIC;			// AGC tracking SNR threshold
EXTERN Int16  shared_agc_maxstepup_db16							 	SHARED_SEC_AGC_AFC_RFIC;			// AGC tracking step-up maximum
EXTERN AGC_dat_t shared_agc_digital_pwr							 	SHARED_SEC_AGC_AFC_RFIC;
EXTERN Int16  shared_agc_maxstepup_db16_hr						 	SHARED_SEC_AGC_AFC_RFIC;			// AGC tracking step-up maximum for half rate
EXTERN Int16 shared_next_agc1							 			SHARED_SEC_AGC_AFC_RFIC;
EXTERN Int16 shared_next_agc2							 			SHARED_SEC_AGC_AFC_RFIC;
EXTERN Int16 shared_next_agc3							 			SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_cell_agc									 	SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_tch_agc									 	SHARED_SEC_AGC_AFC_RFIC;
EXTERN Int16 shared_g_sys[ N_BANDS ][ AGC_GAIN_SIZE ]			 	SHARED_SEC_AGC_AFC_RFIC;			// System gain table (changes on temperature)
EXTERN Int16 shared_raw_digital_RX_power							SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_tx_dyn_pwr_levels[MAX_TX_SLOTS]			 	SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16	shared_default_th							  	 	SHARED_SEC_AGC_AFC_RFIC;			// 
EXTERN UInt16 shared_rfic_rx_bc_ind_and_shutdn_span				 	SHARED_SEC_AGC_AFC_RFIC; 			// RFIC inform DSP SFR2 BC index (bit 15 to 6 is index, bit 5 is valid flag), and RX shutdown span (bit 0 to 3 is span, bit 4 is valid flag) for dynamic RX/TX split
EXTERN UInt16 shared_tsdr_content								 	SHARED_SEC_AGC_AFC_RFIC;				 // Output TSDR register contents for RF timing control
EXTERN UInt16 shared_rfrcor_hi_band								 	SHARED_SEC_AGC_AFC_RFIC;				 // DCS/PCS band RFIC RX dc offset compensation value
EXTERN UInt16 shared_rfrcor_lo_band								 	SHARED_SEC_AGC_AFC_RFIC;				 // GSM850/GSM band RFIC RX dc offset compensation value
EXTERN UInt16	shared_force_reload_rx_gpio_flag					SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_rx_gpio_delay[ N_BANDS ][ N_RXPATTS ][ RXGPIO_SIZE ] SHARED_SEC_AGC_AFC_RFIC; 	
EXTERN UInt16 shared_rx_gpio_state[ N_BANDS ][ N_RXPATTS ][ RXGPIO_SIZE ] SHARED_SEC_AGC_AFC_RFIC;	
EXTERN UInt16 shared_rx_gpio_size										  SHARED_SEC_AGC_AFC_RFIC;	
EXTERN UInt16 shared_tx_gpio_setup_size									  SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_tx_gpio_txon_size									  SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_tx_gpio_txoff_size									  SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_tx_gpio_shutdown_size																	SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_tx_gpio_setup_state   [ N_TX_BANDS ][ N_MODUS ][ N_PER_TXGPIO_GROUP ]					SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_tx_gpio_txon_state    [ N_TX_BANDS ][ N_MODUS ][ N_PER_TXGPIO_GROUP ]					SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_tx_gpio_txoff_state   [ N_TX_BANDS ][ N_MODUS ][ N_PER_TXGPIO_GROUP ]					SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_tx_gpio_shutdown_state[ N_TX_BANDS ][ N_MODUS ][ N_PER_TXGPIO_GROUP ]					SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_tx_gpio_setup_delay   [ N_TX_BANDS ][ N_MODUS ]               [ N_PER_TXGPIO_GROUP ] 	SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_tx_gpio_txon_delay    [ N_TX_BANDS ][ N_MODUS ][ N_PL_GROUPS ][ N_PER_TXGPIO_GROUP ] 	SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_tx_gpio_txoff_delay   [ N_TX_BANDS ][ N_MODUS ][ N_PL_GROUPS ][ N_PER_TXGPIO_GROUP ] 	SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_tx_gpio_shutdown_delay[ N_TX_BANDS ][ N_MODUS ][ N_PL_GROUPS ][ N_PER_TXGPIO_GROUP ] 	SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_tx_gpio_size																			SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_arm_TGPR_state[16]																		SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_arm_TGPR_delay[16]																		SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_arm_TGPR_flag																			SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_tx_db_convert [ N_TX_BANDS ][ N_MODUS ][ TX_DB_DAC_SIZE ]								SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_tx_pedestal[ N_TX_BANDS ][ N_MODUS ][ N_TX_LEVELS ]									SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_tx_mult	[ N_BANDS 	 ][ N_TX_LEVELS ]												SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_tx_base    [ N_TX_BANDS ][ N_MODUS ][ N_TX_LEVELS ]									SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_tx_peak_hi [ N_TX_BANDS ][ N_MODUS ][ N_TX_LEVELS ]									SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_tx_peak_lo [ N_TX_BANDS ][ N_MODUS ][ N_TX_LEVELS ]									SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_tx_data    [ N_TX_BANDS ][ N_MODUS ][ N_TX_LEVELS ]									SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_tx_off     [ N_TX_BANDS ][ N_MODUS ][ N_TX_LEVELS ]									SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_tx_gpio    [ N_TX_BANDS ][ N_MODUS ][ N_TX_LEVELS ]									SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_tx_db_scale																			SHARED_SEC_AGC_AFC_RFIC;
EXTERN Int16  shared_tx_trdr0[ N_MODUS ]																	SHARED_SEC_AGC_AFC_RFIC;
EXTERN Int16  shared_tx_trdr_adjust[ N_MS_DN_RAMPS ]														SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_tx_profile_up0[ N_MODUS ][ TX_PROFILE_SIZE - 3 ]										SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_tx_profile_up1[ N_MODUS ][ TX_PROFILE_SIZE - 4 ]										SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_tx_profile_up2[ N_MODUS ][ TX_PROFILE_SIZE - 5 ]										SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_tx_profile_up3[ N_MODUS ][ TX_PROFILE_SIZE - 6 ]										SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_tx_profile_up4[ N_MODUS ][ TX_PROFILE_SIZE - 7 ]										SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_tx_profile_up5[ N_MODUS ][ TX_PROFILE_SIZE - 8 ]										SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_tx_profile_dn0[ N_MODUS ][ TX_PROFILE_SIZE - 3 ]										SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_tx_profile_dn1[ N_MODUS ][ TX_PROFILE_SIZE - 4 ]										SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_tx_profile_dn2[ N_MODUS ][ TX_PROFILE_SIZE - 5 ]										SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_tx_profile_dn3[ N_MODUS ][ TX_PROFILE_SIZE - 6 ]										SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_tx_profile_dn4[ N_MODUS ][ TX_PROFILE_SIZE - 7 ]										SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_tx_profile_dn5[ N_MODUS ][ TX_PROFILE_SIZE - 8 ]										SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_mixed_tx_profile_up_hi[ N_MODUS ][ N_TX_LEVELS ][ TX_PROFILE_SIZE - 8 ]				SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_mixed_tx_profile_up_lo[ N_MODUS ][ N_TX_LEVELS ][ TX_PROFILE_SIZE - 8 ]				SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_mixed_tx_profile_dn_hi[ N_MODUS ][ N_TX_LEVELS ][ TX_PROFILE_SIZE - 8 ]				SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_mixed_tx_profile_dn_lo[ N_MODUS ][ N_TX_LEVELS ][ TX_PROFILE_SIZE - 8 ]				SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16	shared_APCSWG [2][4]																		SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16	shared_APCSWP [2][4]																		SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16	shared_MSWTR  [2][4]		  																SHARED_SEC_AGC_AFC_RFIC;	// just in case that 8psk and gmsk times are different
EXTERN UInt16	shared_APCPDFR[2]																			SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16	shared_APCGDFR[2]																			SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16	shared_flag_MIXED_TX																		SHARED_SEC_AGC_AFC_RFIC;
EXTERN Int16	shared_apcswx_adjust[N_MODUS]		 														SHARED_SEC_AGC_AFC_RFIC;
EXTERN Int16	shared_gpen_adjust[8]																		SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16	shared_gpen_mask[3]	  																		SHARED_SEC_AGC_AFC_RFIC;
EXTERN Int16	shared_mixed_trdr_adjust[N_MODUS]															SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_SCKR_HI 																				SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_SCKR_LO 																				SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_SFCR 																					SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_rf_mode_ctrl[2]																		SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_rf_ppostamble_len[2]																	SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_rf_preamble_hi[2]																		SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_rf_preamble_lo[2]																		SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_rf_postamble_hi[2]																		SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_rf_postamble_lo[2]																		SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_timing_advance_min																		SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_timing_advance_max																		SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_tx_gpio_txmixed_size																	SHARED_SEC_AGC_AFC_RFIC;		
EXTERN UInt16 shared_tx_gpio_txmixed_state    [ N_TX_BANDS ][ N_MODUS ][ N_PER_TXMIXEDGPIO_GROUP ]					SHARED_SEC_AGC_AFC_RFIC;		
EXTERN UInt16 shared_tx_gpio_txmixed_delay    [ N_TX_BANDS ][ N_MODUS ][ N_PL_GROUPS ][ N_PER_TXMIXEDGPIO_GROUP ] 	SHARED_SEC_AGC_AFC_RFIC;
EXTERN SPI_Records_t		shared_spi_cmd																	SHARED_SEC_AGC_AFC_RFIC;	//   set by aARM for controling MIPI interface to RF
EXTERN UInt32	shared_spi_dump[4][50]						 	SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt32	shared_cn_db16								 	SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt32	shared_2091_rfic_readback[6]					SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16	shared_fsc_readback_sfr_index[4]				SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_audio_in_lo							SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_audio_in_hi							SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_audio_out_lo							SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_audio_out_hi							SHARED_SEC_GEN_AUDIO;
/**
 * @addtogroup Audio_Blocks_in_CP
 * @{
 */
/**
 * @addtogroup Software_Sidetone
 * @{
 */
/**
 * This variable controls the enable and disable of the Sidetone path in 
 * software. It also enables the sidetone double-biquad filter and the expander.
 *
 * \see COMMAND_SIDETONE_COEFS_012, COMMAND_SIDETONE_COEFS_345, 
 *      COMMAND_SIDETONE_COEFS_678, COMMAND_SIDETONE_COEFS_9, 
 *      shared_sidetone_output_gain, shared_sidetone_biquad_scale_factor,
 *      COMMAND_INIT_SIDETONE_EXPANDER, shared_sidetone_expander_flag, 
 *      sidetone_expander
 */
EXTERN UInt16 shared_dsp_sidetone_enable					SHARED_SEC_GEN_AUDIO;
/** @} */
/** @} */

/** 
 * This parameter is not using used in the code currently. 
 */
EXTERN UInt16 shared_sidetone_biquad_sys_gain	  			SHARED_SEC_GEN_AUDIO;
/** 
 * @addtogroup Audio_Gains_in_CP 
 * @{
 */
/**
 * This is the input gain for the sidetone double-biquad filter. \BR
 *
 * It is read by the DSP only at reset. \BR
 *
 * \note parm_audio.txt: SIDETONE_BIQUAD_SCALE_FACTOR
 *
 * \see Software_Sidetone
 */
EXTERN UInt16 shared_sidetone_biquad_scale_factor			SHARED_SEC_GEN_AUDIO;
/** 
 * This is the output gain of the sidetone double-biquad filter. \BR
 *
 * It is read by the DSP every 20ms speech frame. \BR
 *
 * \note parm_dsp.txt: SIDETONE_OUTPUT_GAIN \BR
 *       default value = 0x800 = 0.5
 *
 * \see Software_Sidetone
 *
 */
EXTERN UInt16 shared_sidetone_output_gain					SHARED_SEC_GEN_AUDIO;      
/**
 * This is the output gain of the speaker path. \BR
 *
 * It is read by the DSP every 20ms speech frame. \BR
 *
 * It gets applied to Voice, PCM(Bluetooth), as well as USB speaker paths. \BR
 *
 * \note The gain is in Q1.15 format
 */
EXTERN UInt16 shared_audio_output_gain						SHARED_SEC_GEN_AUDIO;
/**
 * @}
 */
EXTERN UInt16 shared_BTM_total_page 			    		SHARED_SEC_GEN_AUDIO;    							// BT mixer output total pages
EXTERN UInt16 shared_special_mods             				SHARED_SEC_GEN_AUDIO;								//usf related switches.

/******************************************
//Shared memory used by Echo canceller 
******************************************/ 
EXTERN UInt16 shared_echo_cancel_mode_control				SHARED_SEC_GEN_AUDIO;								// Selects LEGACY mode for the DT detector, single or dual filter echo canceller structure

/** 
 * \ingroup Audio_Gains_in_CP 
 *
 * This gain is not used anymore in the code
 */
EXTERN UInt16 shared_echo_NLP_gain							SHARED_SEC_GEN_AUDIO;								// Used to switch in loss after the EC/NS (whichever is later)
EXTERN Int16 shared_echo_cng_filt_coefs[10]					SHARED_SEC_GEN_AUDIO;								// Filter coeffs for the comfort noise generator of the EC NLP bock 
EXTERN UInt16 shared_echo_NLP_ctrl_parms_pending			SHARED_SEC_GEN_AUDIO;								// Used to indicate DSP is awaiting NLP control parameters from the ARM
EXTERN UInt16 shared_echo_NLP_timeout_val					SHARED_SEC_GEN_AUDIO;								// Timeout value for the EC NLP.  DSP Background waits a maximum of shared_echo_NLP_timeout_val audio interrupts
/** 
 * @addtogroup Audio_Gains_in_CP 
 * @{
 */
/**
 * This gain is applied at the input of the Echo Canceller (to the microphone samples).
 *
 * \note This gain is in Q14.6 format 
 */
EXTERN UInt16 shared_echo_cancel_input_gain		   			SHARED_SEC_GEN_AUDIO;
/**
 * This gain is applied at the output of the Echo Canceller (to the microphone samples).
 *
 * \note This gain is in Q14.6 format 
 */
EXTERN UInt16 shared_echo_cancel_output_gain	   			SHARED_SEC_GEN_AUDIO;
/**
 * This gain is applied at the input of the Echo Canceller on the feed forward (DL) path (to the speaker samples).
 *
 * \note This gain is in Q14.6 format 
 */
EXTERN UInt16 shared_echo_cancel_feed_forward_gain 			SHARED_SEC_GEN_AUDIO;
/**
 * This gain is applied at the input of the Echo Canceller (to the second microphone samples).
 *
 * \note This gain is in Q14.6 format 
 */
EXTERN UInt16 shared_echo_cancel_mic2_input_gain		   	SHARED_SEC_GEN_AUDIO;
/**
 * This gain is applied at the output of the Echo Canceller (to the second microphone samples).
 *
 * \note This gain is in Q14.6 format 
 */
EXTERN UInt16 shared_echo_cancel_mic2_output_gain	   	   	SHARED_SEC_GEN_AUDIO;
/**
 * This gain is applied at the input of the Echo Canceller on the feed forward (DL) path (to the speaker samples).
 *
 * \note This gain is in Q14.6 format 
 */
EXTERN UInt16 shared_echo_cancel_mic2_feed_forward_gain    	SHARED_SEC_GEN_AUDIO;
/**
 * This gain is applied at the output of the pre-compressor filter on the DL path.
 *
 * \note This gain is in Q14.6 format 
 */


EXTERN UInt16 shared_echo_spkr_phone_input_gain				SHARED_SEC_GEN_AUDIO;
/** 
 * @}
 */
EXTERN UInt16 shared_echo_digital_input_clip_level			SHARED_SEC_GEN_AUDIO;

EXTERN UInt16 shared_ecLen                   				SHARED_SEC_GEN_AUDIO;					    		// EC length
EXTERN Int16 shared_DT_TH_ERL_dB             				SHARED_SEC_GEN_AUDIO;								// DT ERL Threshold in DB
/** 
 * \ingroup Audio_Gains_in_CP 
 *
 * Echo stepsize gain
 */
EXTERN Int16 shared_echo_step_size_gain   					SHARED_SEC_GEN_AUDIO; 								// Echo stepsize gain
EXTERN Int16 shared_echo_farIn_max_lb						SHARED_SEC_GEN_AUDIO;							    // Max FarIn energy for low band
EXTERN Int16 shared_echo_farOut_max_lb						SHARED_SEC_GEN_AUDIO;							    // Max FarOut energy for low band
EXTERN Int16 shared_echo_nearIn_max						    SHARED_SEC_GEN_AUDIO;								// Max NearIn energy for low band
EXTERN Int16 shared_echo_onlineOut_max_lb					SHARED_SEC_GEN_AUDIO;								// Max echo_onlineOut energy for low band

EXTERN Int16 shared_echo_farIn_max_hb						SHARED_SEC_GEN_AUDIO;								// Max FarIn energy for high band
EXTERN Int16 shared_echo_farOut_max_hb						SHARED_SEC_GEN_AUDIO;								// Max FarOut energy for high band
EXTERN Int16 shared_echo_nearIn_max_hb						SHARED_SEC_GEN_AUDIO;								// Max NearIn energy for high band
EXTERN Int16 shared_echo_onlineOut_max_hb					SHARED_SEC_GEN_AUDIO;								// Max echo_onlineOut energy for high band

EXTERN Int16 shared_adaptive_ec_coef[128]			   		SHARED_SEC_GEN_AUDIO;								// Holds latest 20ms worth of Noise Suppressed data 
EXTERN Int16 shared_echo_input_data[320]	 				SHARED_SEC_GEN_AUDIO;								// Holds latest 20ms worth of Uplink and Downlink speech samples 
EXTERN Int16 shared_echo_output_data[320]	 				SHARED_SEC_GEN_AUDIO;								// Holds latest 20ms worth of Echo Cancelled data 
EXTERN Int16 shared_ec_tmp_buf[320]							SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_ec_bulk_delay_sample_num				SHARED_SEC_GEN_AUDIO;								//Specifies the bulk delay in units of PCM samples, ie, with a granularity of 125us
EXTERN Int16 shared_ec_bulk_delay_buf[MAX_EC_BULK_DELAY_BUF_SIZE*2]		 SHARED_SEC_GEN_AUDIO;					//EC buffer for absorbing upto 80ms of bulk delay


/******************************************
//Shared memory used by Noise suppressor
******************************************/
/** 
 * @addtogroup Audio_Gains_in_CP 
 * @{
 */
EXTERN UInt16 shared_gain_slope							 	SHARED_SEC_GEN_AUDIO;
/**
 * This gain is applied at the input of the Uplink Noise suppressor.
 *
 * \note This gain is in Q14.6 format 
 */
EXTERN UInt16 shared_noise_supp_input_gain					SHARED_SEC_GEN_AUDIO;
/**
 * This gain is applied at the output of the Uplink Noise suppressor.
 *
 * \note This gain is in Q14.6 format 
 */
EXTERN UInt16 shared_noise_supp_output_gain					SHARED_SEC_GEN_AUDIO;
/**
 * @}
 */
EXTERN Int16  shared_noise_max_supp_dB[24]					SHARED_SEC_GEN_AUDIO;								//Maximum amount of suppression by band in dBQ5
EXTERN UInt16 shared_noise_supp_ul_min_pwr					SHARED_SEC_GEN_AUDIO;								/* Number of samples in a frame */
EXTERN UInt16 shared_noise_spec_var[2]						SHARED_SEC_GEN_AUDIO;
EXTERN Int16 shared_idev_thld								SHARED_SEC_GEN_AUDIO;								// NS background noise estimate adaptation performance fine tuning
EXTERN Int16 shared_iupdate_thld 							SHARED_SEC_GEN_AUDIO;								// NS background noise estimate adaptation performance fine tuning
EXTERN Int16 shared_ihyster_cnt_thld 						SHARED_SEC_GEN_AUDIO;								// NS background noise estimate adaptation performance fine tuning
EXTERN UInt16 shared_update_cnt_thld						SHARED_SEC_GEN_AUDIO;
EXTERN Int16 shared_update_noise_flag						SHARED_SEC_GEN_AUDIO;								// TRUE==NS background noise estimate is being updated
EXTERN Int16 shared_noise_output_power[5]					SHARED_SEC_GEN_AUDIO;								// Used for comfort noise generator noise floor estimation
EXTERN Int16 shared_noise_output_data[320]	 				SHARED_SEC_GEN_AUDIO;								// Holds latest 20ms worth of Noise Suppressed data 



/******************************************
//Shared memory used by SB-NLP
******************************************/ 
EXTERN UInt16 shared_nlp_mode								 SHARED_SEC_GEN_AUDIO;								//0 == FULLBAND mode; 1 == SUBBAND_MODE
EXTERN UInt16 shared_subband_nlp_reset						 SHARED_SEC_GEN_AUDIO;								// TRUE==reset subband nlp gains
EXTERN UInt16 shared_subband_nlp_erl_erle_min				 SHARED_SEC_GEN_AUDIO;								// Minimum ERL_ERLE for each subband
EXTERN UInt16 shared_subband_nlp_erl_erle_nominal			 SHARED_SEC_GEN_AUDIO;								// Nominal ERL_ERLE for each subband
EXTERN UInt16 shared_subband_nlp_dl_delay_adj				 SHARED_SEC_GEN_AUDIO;								// delay adjustment needed in 10ms frames to align DL energy with the UL
EXTERN UInt16 shared_subband_nlp_filtered_gain_alpha		 SHARED_SEC_GEN_AUDIO;								// Subband NLP gain smoothing alpha
EXTERN UInt16 shared_subband_nlp_dl_distortion_adj_th		 SHARED_SEC_GEN_AUDIO;								// Threshold to exceed before adjusting ERL_ERLE downwards
EXTERN UInt16 shared_subband_nlp_fast_ns_adaptation_th		 SHARED_SEC_GEN_AUDIO;								// Threshold to exceed before faster NS adaptation logic kicks in
EXTERN UInt16 shared_subband_nlp_hpf_coef_a 				 SHARED_SEC_GEN_AUDIO;								// HPF filter coef A
EXTERN UInt16 shared_subband_nlp_hpf_coef_b 				 SHARED_SEC_GEN_AUDIO;								// HPF filter coef B
EXTERN UInt16 shared_echo_NLP_ComfortNoise_flag				 SHARED_SEC_GEN_AUDIO;
EXTERN Int16 shared_trigger_erl_erle_reset					 SHARED_SEC_GEN_AUDIO;								// ERL_ERLE gets set to ERL_MIN by the DSP if ERLE < this threshold in dB16
/** 
 * \ingroup Audio_Gains_in_CP 
 * 
 * This gain is applied at the input to the uplink compander
 *
 */
EXTERN Int16 shared_echo_fast_NLP_gain[5]					 SHARED_SEC_GEN_AUDIO;								// Used to switch in loss after the EC/NS (whichever is later)
/** 
 * \ingroup Audio_Gains_in_CP 
 * 
 * This gain is applied at the output to the downlink expander
 *
 */
EXTERN Int16 shared_echo_NLP_fast_downlink_volume_ctrl[5] 	 SHARED_SEC_GEN_AUDIO;
EXTERN Int16 shared_data_buffer[5]							 SHARED_SEC_GEN_AUDIO;								// Holds latest 20ms worth of Noise Suppressed data 
EXTERN UInt16 shared_subband_dl_energy_window_profile[8]	 SHARED_SEC_GEN_AUDIO;								// 16 band subband energy window profile

EXTERN UInt16 shared_ul_subband_energy_dB_0[24]	 			 SHARED_SEC_GEN_AUDIO;								//24 band subband energy, dB
EXTERN UInt16 shared_ul_subband_energy_dB_1[24]	 			 SHARED_SEC_GEN_AUDIO;								//24 band subband energy, dB
EXTERN UInt16 shared_dl_subband_energy_dB_0[24]				 SHARED_SEC_GEN_AUDIO;								//24 band subband energy, dB
EXTERN UInt16 shared_dl_subband_energy_dB_1[24]				 SHARED_SEC_GEN_AUDIO;								//24 band subband energy, dB
EXTERN UInt16 shared_subband_erl_erle[24]					 SHARED_SEC_GEN_AUDIO;								//24 band subband erl and erle in dB
EXTERN UInt16 shared_subband_nlp_noise_margin[24]			 SHARED_SEC_GEN_AUDIO;								//MARGIN below the noise floor used when calculating NLP loss
EXTERN UInt16 shared_subband_nlp_UL_margin[24]				 SHARED_SEC_GEN_AUDIO;								//MARGIN below the UL estimated signal level used when calculating NLP loss
EXTERN UInt16 shared_subband_nlp_erl_erle_adj[24]			 SHARED_SEC_GEN_AUDIO;								//24 band subband erl_erle adjustment in dB 
EXTERN UInt16 shared_subband_nlp_distortion_adj[24]			 SHARED_SEC_GEN_AUDIO;								//24 band subband erl_erle adjustment in dB based purely on DL distortion estimation
EXTERN UInt16 shared_subband_nlp_erl_min[24]				 SHARED_SEC_GEN_AUDIO;								//24 band subband minimum erl in dB
/** 
 * \ingroup Audio_Gains_in_CP 
 * 
 */
EXTERN Int16  shared_subband_nlp_gains[24]					 SHARED_SEC_GEN_AUDIO;								//Pass the final g_nlp[] gains to the ARM for extra logging capability
EXTERN Int16  shared_subband_final_ns_gains[24]				 SHARED_SEC_GEN_AUDIO;								//Pass the final g_tot[] gains to the ARM for extra logging capability	
EXTERN UInt16 shared_echoNlpParams_subband_nlp_UL_margin[24] SHARED_SEC_GEN_AUDIO;								//MARGIN below the UL estimated signal level used when calculating NLP loss
EXTERN UInt16 shared_echoNlpParams_subband_nlp_distortion_thresh[24] SHARED_SEC_GEN_AUDIO;						//
EXTERN UInt16 shared_subband_nlp_dt_fine_control			 SHARED_SEC_GEN_AUDIO;

EXTERN UInt16 shared_echo_nlp_min_dl_pwr					 SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_echo_nlp_min_ul_pwr					 SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_curr_dl_min_pwr						 SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_curr_ul_min_pwr						 SHARED_SEC_GEN_AUDIO;
EXTERN Int16  shared_curr_min_ul_sb_energy[24]			 	 SHARED_SEC_GEN_AUDIO;
EXTERN Int16  shared_uplink_inst_block_pwr				     SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_echoNlp_expander_upper_limit_ul		 SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_echoNlp_expander_upper_limit			 SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_audio_mode								 SHARED_SEC_GEN_AUDIO;

/******************************************
//Shared memory used by Compressor/Expander/
******************************************/ 


EXTERN UInt16 shared_DL_compander_flag 	   		 			 SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_DL_subband_compander_flag				 SHARED_SEC_GEN_AUDIO;								// Enable/Disable Subband compressor
EXTERN UInt16 shared_DL_compander_biquad_flag 				 SHARED_SEC_GEN_AUDIO;								// Enable/disable the compander biquad
EXTERN UInt16 shared_UL_compander_flag 			 			 SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_compressor_alg							 SHARED_SEC_GEN_AUDIO;								// Flag to switch to run the interpolation based version instead of dB based version

EXTERN Shared_Compressor_t 	compress_UL						 SHARED_SEC_GEN_AUDIO;
EXTERN Shared_Compressor_t 	compress_DL						 SHARED_SEC_GEN_AUDIO;


EXTERN Shared_Compressor_t	shared_compress_DL_FIR1			 SHARED_SEC_GEN_AUDIO;								// Compressor parms for FIR1
EXTERN Shared_Compressor_t	shared_compress_DL_FIR2			 SHARED_SEC_GEN_AUDIO;								// Compressor parms for FIR2
EXTERN Shared_Compressor_t	shared_compress_DL_STREAM3		 SHARED_SEC_GEN_AUDIO;								// Compressor parms for the 2rd stream
EXTERN compressor_parm 	compress_vp							 SHARED_SEC_GEN_AUDIO;								// VP memo recording compressor struct

EXTERN expander_parm	expander_vp							 SHARED_SEC_GEN_AUDIO;								// VP memo recording expander struct
EXTERN expander_parm	expander_DL							 SHARED_SEC_GEN_AUDIO;
EXTERN expander_parm	expander_UL							 SHARED_SEC_GEN_AUDIO;


EXTERN Int16 shared_coef_FIR2[11]							 SHARED_SEC_GEN_AUDIO;								// N+1/2 Filter coefs for FIR2

// Omega Voice data
EXTERN UInt16 shared_audio_enhancement_enable				 SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_omega_voice_max_allowed_gain_spread_dB	 SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_omega_voice[30]						 SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_ul_Etotal[5]							 SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_ul_pitch_lag[5]						 SHARED_SEC_GEN_AUDIO;	//pitch lag
EXTERN UInt16 shared_ul_Ntotal[5] 							 SHARED_SEC_GEN_AUDIO;	//Ntotal
EXTERN UInt16 shared_ul_vmsum[2] 							 SHARED_SEC_GEN_AUDIO;	//vmsum for 1st and last 10ms frame
EXTERN Int16 shared_CNG_seed 								 SHARED_SEC_GEN_AUDIO;	//Filtered and scaled CNG from the ARM
EXTERN Int16 shared_CNG_bias	 							 SHARED_SEC_GEN_AUDIO;	//Filtered and scaled CNG from the ARM

EXTERN UInt16 shared_ns_forced_init_etot_th					 	SHARED_SEC_GEN_AUDIO;	//etot threshold for NS forced adap
EXTERN UInt16 shared_ns_forced_update_noise_est_flag		 	SHARED_SEC_GEN_AUDIO;	//flag to force an update of the NS noise estimate
EXTERN Int16 shared_echo_farIn[5]							 SHARED_SEC_GEN_AUDIO;	// Uplink level measurement before the Echo canceller
EXTERN Int16 shared_echo_farOut[5]    						 SHARED_SEC_GEN_AUDIO;	// Uplink level measurement after the Echo canceller (offline filter)
EXTERN Int16 shared_echo_nearIn[5]    						 SHARED_SEC_GEN_AUDIO;	// Downlink level measurement into the Echo canceller

EXTERN UInt16	shared_alpha_voice_gain_scale					SHARED_SEC_GEN_AUDIO;	//Provide the means to scale the NS gain
EXTERN Int16	shared_ns_gain_dB_prev[16]						SHARED_SEC_GEN_AUDIO;
EXTERN UInt16	shared_alpha_voice_enable						SHARED_SEC_GEN_AUDIO;
EXTERN Int16	shared_alpha_voice_pitch_delay_buf[5]			SHARED_SEC_GEN_AUDIO;	//pitch delay line
EXTERN Int16	shared_alpha_voice_pitch_hangover_ptr			SHARED_SEC_GEN_AUDIO;
EXTERN Int16	shared_alpha_voice_max_supp[16]					SHARED_SEC_GEN_AUDIO;
EXTERN Int16	shared_alpha_voice_snr_thresh[16]				SHARED_SEC_GEN_AUDIO;
EXTERN Int16	shared_alpha_voice_med_filt_enable				SHARED_SEC_GEN_AUDIO;
/** 
 * \ingroup Audio_Gains_in_CP 
 * 
 */
EXTERN UInt16	shared_kappa_voice_dl_gain[5]					SHARED_SEC_GEN_AUDIO;
EXTERN UInt16	shared_alpha_voice_forced_noise_update_flag[24]	SHARED_SEC_GEN_AUDIO;	//flag to force an update of the NS noise estimate subband by subband
EXTERN UInt16	shared_alpha_voice_mean_diff_th					SHARED_SEC_GEN_AUDIO;
EXTERN UInt16	shared_alpha_voice_mean_pt_th					SHARED_SEC_GEN_AUDIO;
EXTERN UInt16	shared_alpha_voice_curr_pt_th					SHARED_SEC_GEN_AUDIO;
EXTERN UInt16	shared_alpha_voice_etot_th						SHARED_SEC_GEN_AUDIO;
EXTERN UInt16	shared_alpha_voice_ns_alpha						SHARED_SEC_GEN_AUDIO;
EXTERN UInt16	shared_alpha_voice_ns_alpha_minus_1				SHARED_SEC_GEN_AUDIO;
/** 
 * \ingroup Audio_Gains_in_CP 
 * 
 *  This gain is not used in the code.
 */
EXTERN UInt16	shared_subband_dl_analysis_gain					SHARED_SEC_GEN_AUDIO;
EXTERN UInt16   shared_dual_mic_subband_energy_dB_0[16]			SHARED_SEC_GEN_AUDIO;	//16 band subband energy, dB
EXTERN UInt16   shared_dual_mic_subband_energy_dB_1[16]			SHARED_SEC_GEN_AUDIO;	//16 band subband energy, dB
EXTERN Int16    shared_comp_output_power[5]						SHARED_SEC_GEN_AUDIO;
EXTERN comp_parms_t fir1										SHARED_SEC_GEN_AUDIO;	// Parsed and precalculated intermediary results for low band compressor curve
EXTERN comp_parms_t fir2										SHARED_SEC_GEN_AUDIO;	// Parsed and precalculated intermediary results for mid band compressor curve
EXTERN comp_parms_t stream3										SHARED_SEC_GEN_AUDIO;	// Parsed and precalculated intermediary results for high band compressor curve
EXTERN comp_parms_t compress_ul									SHARED_SEC_GEN_AUDIO;	// Parsed and precalculated intermediary results for UL compressor curve
EXTERN comp_parms_t compress_dl									SHARED_SEC_GEN_AUDIO;	// Parsed and precalculated intermediary results for DL compressor curve
EXTERN OmegaVoice_t	shared_omega_voice_mem						SHARED_SEC_GEN_AUDIO;
EXTERN AlphaVoice_t	shared_alpha_voice_mem						SHARED_SEC_GEN_AUDIO;
EXTERN KappaVoice_t	shared_kappa_voice_mem						SHARED_SEC_GEN_AUDIO;
EXTERN Shared_Compressor_t	compress_ul_g2t2					SHARED_SEC_GEN_AUDIO;
// following "informational" values written by the noise
// suppressor when enabled in shared_ns_control

EXTERN UInt16	shared_noise_dyn_range_enh_enable				SHARED_SEC_GEN_AUDIO;	// Used to enable the dynamic range enhancement
EXTERN UInt16	shared_vm_sum 									SHARED_SEC_GEN_AUDIO;	
EXTERN UInt16	shared_gamma_l 									SHARED_SEC_GEN_AUDIO;	
EXTERN UInt16	shared_gamma_h 									SHARED_SEC_GEN_AUDIO;
EXTERN UInt16	shared_audio_debug_ctrl							SHARED_SEC_GEN_AUDIO;
EXTERN UInt16	shared_audio_debug[500]							SHARED_SEC_GEN_AUDIO;	


/**
 * @addtogroup Audio_Blocks_in_CP
 * @{
 */
/**
 * @addtogroup Software_Sidetone
 * @{
 */
/**
 * This variable enables the Sidetone Expander. This is read by the DSP on getting 
 * the COMMAND_INIT_SIDETONE_EXPANDER command.
 *
 * \see COMMAND_INIT_SIDETONE_EXPANDER
 */
EXTERN UInt16			shared_sidetone_expander_flag  		 SHARED_SEC_GEN_AUDIO;
/**
 * This variable contains the Sidetone Expander's static memory. This is read by the DSP on getting 
 * the COMMAND_INIT_SIDETONE_EXPANDER command.
 *
 * \see COMMAND_INIT_SIDETONE_EXPANDER, expander_parm
 */

EXTERN expander_parm	sidetone_expander			   		 SHARED_SEC_GEN_AUDIO;
/** @} */
/** @} */
/**
 * @addtogroup Audio_Gains_in_CP
 * @{
 */
/**
 * This is the gain of the Bluetooth Double-Biquad Filter in the downlink 
 * (right before the Limiter)
 *
 * \see shared_bluetooth_filter_enable, shared_bluetooth_dl_filter_coef, COMMAND_BIQUAD_FILT_COEFS
 */
EXTERN UInt16	shared_bluetooth_dl_biquad_gain		   		 SHARED_SEC_GEN_AUDIO;
/**
 * This is the output gain after the Bluetooth Double-Biquad Filter in the downlink 
 * (right before the Limiter)
 *
 * \see shared_bluetooth_filter_enable, shared_bluetooth_dl_filter_coef, COMMAND_BIQUAD_FILT_COEFS
 */
EXTERN UInt16	shared_bluetooth_dl_biquad_output_gain		   		 SHARED_SEC_GEN_AUDIO;

/**
 * This is the gain of the Bluetooth Double-Biquad Filter in the uplink 
 * (right after the High Pass Filter)
 *
 * \see shared_bluetooth_filter_enable, shared_bluetooth_ul_filter_coef, COMMAND_BIQUAD_FILT_COEFS
 */
EXTERN UInt16	shared_bluetooth_ul_biquad_gain		   		 		SHARED_SEC_GEN_AUDIO;
/**
 * This is the output gain after the Bluetooth Double-Biquad Filter in the uplink 
 * 
 *
 * \see shared_bluetooth_filter_enable, shared_bluetooth_ul_filter_coef, COMMAND_BIQUAD_FILT_COEFS
 */
EXTERN UInt16	shared_bluetooth_ul_biquad_output_gain		   		 SHARED_SEC_GEN_AUDIO;

/** @} */
/**
 * @addtogroup Audio_Blocks_in_CP
 * @{
 */
/**
 * @addtogroup Bluetooth_Double_BiQuad_Filters_in_UL_AND_DL
 * @{
 */
/**
 * This variable enables 
 * - (== 1) the Bluetooth Double-Biquad Filter in the uplink (right after the High Pass Filter), and 
 * - (!= 0) the Bluetooth Double-Biquad Filter in the downlink (right before the Limiter). 
 *
 * \see shared_bluetooth_ul_biquad_gain, shared_bluetooth_ul_filter_coef, COMMAND_BIQUAD_FILT_COEFS
 * \see shared_bluetooth_dl_biquad_gain, shared_bluetooth_dl_filter_coef
 */
EXTERN UInt16	shared_bluetooth_filter_enable		   		 SHARED_SEC_GEN_AUDIO;
/**
 * This array contains the coefficients of the Bluetooth Double-Biquad Filter in the downlink 
 * (right before the Limiter)
 *
 * \see shared_bluetooth_dl_biquad_gain, shared_bluetooth_filter_enable, COMMAND_BIQUAD_FILT_COEFS
 */
EXTERN UInt16	shared_bluetooth_dl_filter_coef[10]	   		 SHARED_SEC_GEN_AUDIO;
/**
 * This array contains the coefficients of the Bluetooth Double-Biquad Filter in the uplink 
 * (right after the High Pass Filter)
 *
 * \see shared_bluetooth_ul_biquad_gain, shared_bluetooth_filter_enable, COMMAND_BIQUAD_FILT_COEFS
 */
EXTERN UInt16	shared_bluetooth_ul_filter_coef[10]	   		 SHARED_SEC_GEN_AUDIO;
/** @} */
/** @} */

/** 
 * \ingroup Audio_Gains_in_CP 
 * 
 * This gain is applied at the input of the compressor pre-filter in the DL.
 */
EXTERN UInt16	shared_comp_biquad_gain						 SHARED_SEC_GEN_AUDIO;
EXTERN UInt16	shared_comp_filter_coef[10]					 SHARED_SEC_GEN_AUDIO;
/** 
 * @addtogroup Audio_Gains_in_CP 
 * @{
 */
/** 
 * This gain is not used any more. */
EXTERN UInt16	shared_second_amr_out_gain					 SHARED_SEC_GEN_AUDIO;
/** 
 * This gain, is applied on the VPU playback path on the downlink path.\BR
 * 
 * This gain is a ramped gain in Q14 format as shown below (Q14 means 2.14 format).\BR
 *
 * {
 * -   Int16 Start_Gain_in_Q14, 
 * -   Int16 Target_Gain_in_Q14, 
 * -   Int16 Step_size_to_increment_or_decrement_the_Gain_in_Q14,  (<= 0 make Start_Gain_in_Q14 = Target_Gain_in_Q14) 
 * -   Int16 Reserved, 
 * -   Int16 Reserved \BR
 * }
 *
 */
EXTERN Int16	shared_second_amr_out_gain_dl[5]			 SHARED_SEC_GEN_AUDIO;
/** 
 * This gain, is applied on the VPU playback path on the uplink path.\BR
 * 
 * This gain is a ramped gain in Q14 format as shown below (Q14 means 2.14 format).\BR
 *
 * {
 * -   Int16 Start_Gain_in_Q14, 
 * -   Int16 Target_Gain_in_Q14, 
 * -   Int16 Step_size_to_increment_or_decrement_the_Gain_in_Q14,  (<= 0 make Start_Gain_in_Q14 = Target_Gain_in_Q14) 
 * -   Int16 Reserved, 
 * -   Int16 Reserved \BR
 * }
 *
 */
EXTERN Int16	shared_second_amr_out_gain_ul[5]			 SHARED_SEC_GEN_AUDIO;
/** 
 * This gain, is applied on the VPU playback path on the downlink path before going into record.\BR
 * 
 * This gain is a ramped gain in Q14 format as shown below (Q14 means 2.14 format).\BR
 *
 * {
 * -   Int16 Start_Gain_in_Q14, 
 * -   Int16 Target_Gain_in_Q14, 
 * -   Int16 Step_size_to_increment_or_decrement_the_Gain_in_Q14,  (<= 0 make Start_Gain_in_Q14 = Target_Gain_in_Q14) 
 * -   Int16 Reserved, 
 * -   Int16 Reserved \BR
 * }
 *
 */
EXTERN Int16	shared_speech_rec_gain_dl[5]				 SHARED_SEC_GEN_AUDIO;
/** 
 * This gain, is applied on the VPU playback path on the uplink path before going into record.\BR
 * 
 * This gain is a ramped gain in Q14 format as shown below (Q14 means 2.14 format).\BR
 *
 * {
 * -   Int16 Start_Gain_in_Q14, 
 * -   Int16 Target_Gain_in_Q14, 
 * -   Int16 Step_size_to_increment_or_decrement_the_Gain_in_Q14,  (<= 0 make Start_Gain_in_Q14 = Target_Gain_in_Q14) 
 * -   Int16 Reserved, 
 * -   Int16 Reserved \BR
 * }
 *
 */
EXTERN Int16	shared_speech_rec_gain_ul[5]				 SHARED_SEC_GEN_AUDIO;

/** 
 * The gain factor, shared_arm2speech_call_gain, is not used any more. */
EXTERN UInt16	shared_arm2speech_call_gain					 SHARED_SEC_GEN_AUDIO;
/** 
 * The gain factor, shared_arm2speech_call_gain_dl is applied on the shared_Arm2SP_InBuf[1280] PCM data 
 * on the downlink path (does not matter whether the ARM2SP data is added before or after audio processing.\BR
 * 
 * This gain is a ramped gain in Q14 format as shown below (Q14 means 2.14 format).\BR
 *
 * {
 * -   Int16 Start_Gain_in_Q14, 
 * -   Int16 Target_Gain_in_Q14, 
 * -   Int16 Step_size_to_increment_or_decrement_the_Gain_in_Q14,  (<= 0 make Start_Gain_in_Q14 = Target_Gain_in_Q14) 
 * -   Int16 Reserved, 
 * -   Int16 Reserved \BR
 * }
 *
 */
EXTERN Int16	shared_arm2speech_call_gain_dl[5]			 SHARED_SEC_GEN_AUDIO;
/** 
 * The gain factor, shared_arm2speech_call_gain_ul is applied on the shared_Arm2SP_InBuf[1280] PCM data 
 * on the uplink path (does not matter whether the ARM2SP data is added before or after audio processing.\BR
 * 
 * This gain is a ramped gain in Q14 format as shown below (Q14 means 2.14 format).\BR
 *
 * {
 * -   Int16 Start_Gain_in_Q14,
 * -   Int16 Target_Gain_in_Q14,
 * -   Int16 Step_size_to_increment_or_decrement_the_Gain_in_Q14,  (<= 0 make Start_Gain_in_Q14 = Target_Gain_in_Q14)
 * -   Int16 Reserved,
 * -   Int16 Reserved \BR
 * }
 *
 */
EXTERN Int16	shared_arm2speech_call_gain_ul[5]			 SHARED_SEC_GEN_AUDIO;
/** 
 * The gain factor, shared_arm2speech_call_gain_rec is applied on the ARM2SP PCM data 
 * getting recorded.\BR
 * 
 * This gain is a ramped gain in Q14 format as shown below (Q14 means 2.14 format).\BR
 *
 * {
 * -   Int16 Start_Gain_in_Q14,
 * -   Int16 Target_Gain_in_Q14,
 * -   Int16 Step_size_to_increment_or_decrement_the_Gain_in_Q14,  (<= 0 make Start_Gain_in_Q14 = Target_Gain_in_Q14)
 * -   Int16 Reserved,
 * -   Int16 Reserved \BR
 * }
 *
 */
EXTERN Int16	shared_arm2speech_call_gain_rec[5]			 SHARED_SEC_GEN_AUDIO;
/** 
 * The gain factor, shared_btnb_gain_dl is applied on the Bluetooth NB PCM data 
 * in the downlink path.\BR
 * 
 * This gain is a ramped gain in Q14 format as shown below (Q14 means 2.14 format).\BR
 *
 * {
 * -   Int16 Start_Gain_in_Q14,
 * -   Int16 Target_Gain_in_Q14,
 * -   Int16 Step_size_to_increment_or_decrement_the_Gain_in_Q14,  (<= 0 make Start_Gain_in_Q14 = Target_Gain_in_Q14)
 * -   Int16 Reserved,
 * -   Int16 Reserved \BR
 * }
 *
 */
EXTERN Int16	shared_btnb_gain_dl[5]						 SHARED_SEC_GEN_AUDIO;
/** 
 * The gain factor, shared_btnb_gain_dl is applied on the Bluetooth NB PCM data 
 * in the uplink path. \BR
 * 
 * This gain is a ramped gain in Q14 format as shown below (Q14 means 2.14 format).\BR
 *
 * {
 * -   Int16 Start_Gain_in_Q14,
 * -   Int16 Target_Gain_in_Q14,
 * -   Int16 Step_size_to_increment_or_decrement_the_Gain_in_Q14,  (<= 0 make Start_Gain_in_Q14 = Target_Gain_in_Q14)
 * -   Int16 Reserved,
 * -   Int16 Reserved \BR
 * }
 *
 */
EXTERN Int16	shared_btnb_gain_ul[5]			  			 SHARED_SEC_GEN_AUDIO;
/**
 * @}
 */
EXTERN UInt16	shared_UL_audio_clip_level					 SHARED_SEC_GEN_AUDIO;

/** 
 * \ingroup Audio_Gains_in_CP 
 * This gain is applied at the input of the DL limiter (currently used only in the VoIP path).
 *
 * \note This gain is in Q14.6 format 
 * 
 */
EXTERN UInt16 shared_dl_clip_gain							 SHARED_SEC_GEN_AUDIO;								// Gain stage prior to the DL clipper
EXTERN UInt16 shared_volume_control_in_q14_format			 SHARED_SEC_GEN_AUDIO;								// Flag to switch to run Q14 or Q6 volume control functions
/** 
 * @addtogroup Audio_Gains_in_CP 
 * @{
 * 
 */
/** 
 * The gain factor, shared_gen_tone_gain_rec is applied on the tones being recorded.\BR
 * 
 * This gain is a ramped gain in Q14 format as shown below (Q14 means 2.14 format).\BR
 *
 * {
 * -   Int16 Start_Gain_in_Q14,
 * -   Int16 Target_Gain_in_Q14,
 * -   Int16 Step_size_to_increment_or_decrement_the_Gain_in_Q14,  (<= 0 make Start_Gain_in_Q14 = Target_Gain_in_Q14)
 * -   Int16 Reserved,
 * -   Int16 Reserved \BR
 * }
 *
 */
EXTERN Int16	shared_gen_tone_gain_rec[5]					 SHARED_SEC_GEN_AUDIO;
/** 
 * The gain factor, shared_gen_tone_gain_ul is applied on the tones in the uplink path. \BR
 * 
 * This gain is a ramped gain in Q14 format as shown below (Q14 means 2.14 format).\BR
 *
 * {
 * -   Int16 Start_Gain_in_Q14,
 * -   Int16 Target_Gain_in_Q14,
 * -   Int16 Step_size_to_increment_or_decrement_the_Gain_in_Q14,  (<= 0 make Start_Gain_in_Q14 = Target_Gain_in_Q14)
 * -   Int16 Reserved,
 * -   Int16 Reserved \BR
 * }
 *
 */
EXTERN Int16	shared_gen_tone_gain_ul[5]					 SHARED_SEC_GEN_AUDIO;
/**
 * @}
 */
EXTERN UInt16 shared_tone_ul[640]							 SHARED_SEC_GEN_AUDIO;
EXTERN VR_Lin_PCM_t	DL_MainDecOut_buf    					SHARED_SEC_GEN_AUDIO;								// Downlink primary AMR decoded output PCM data buf during GSM idle (WCDMA voice call) 

/** @addtogroup Shared_Audio_Buffers_Still_in_CP 
 * @{ */
/** @addtogroup NEWAUDFIFO_Interface
 *
 *  This interface can be used to play Audio directly from the ARM to the Audio path.
 *
 *  \note This interface is not supported in Athena.
 *
 * \section Play To start PCM play:
 * - ARM writes PCM data into shared_pram_codec_out0 or shared_newpr_codec_out0, 
 *   updates input pointer(s) shared_NEWAUD_OutBuf_in[i], i=0,1.
 * - ARM configures and enables HW audio codec directly by poking into DSP registers 
 *   in products before Athena.
 * - ARM configures DSP ISR for HW audio FIFO using COMMAND_NEWAUDFIFO_START. The same
 *   command also configures the soft FIFO thresholds.
 * - DSP ISR reads  shared_pram_codec_out0 or shared_newpr_codec_out0 and writes to 
 *   HW audio FIFO registers when HW audio FIFO Interrupt happens. 
 *   DSP ISR will update shared_NEWAUD_OutBuf_out[i], i=0,1.
 * - DSP interrupts ARM with STATUS_NEWAUDFIFO_SW_FIFO_LOW when unused data in 
 *   shared_pram_codec_out0 or shared_newpr_codec_out0 < soft FIFO threshold \BR
 *   or STATUS_NEWAUDFIFO_SW_FIFO_EMPTY when shared_pram_codec_out0 or 
 *   shared_newpr_codec_out0 are empty.
 * - ARM fills shared_pram_codec_out0 or shared_newpr_codec_out0 when 
 *   STATUS_NEWAUDFIFO_SW_FIFO_LOW or STATUS_NEWAUDFIFO_SW_FIFO_EMPTY.
 * 
 * \section STOP To stop playback:
 *
 * 	\Bold{Quick stop:}
 * 	- ARM disables HW audio codec.
 * 	- ARM disables DSP ISR for HW audio FIFO using COMMAND_NEWAUDFIFO_CANCEL. 
 * 	- DSP will response with STATUS_NEWAUDFIFO_CANCELPLAY.
 * 	
 * 	\note The playback will be stopped right away. There may be un-finished 
 * 	PCM data in shared_pram_codec_out0 or shared_newpr_codec_out0.
 *
 * 	\Bold{Finishing stop:}
 * 	- After finishing filling PCM data, ARM set shared_NEWAUD_InBuf_done_flag[i] to 1 
 * 	  to indicate the end of PCM data.
 * 	- DSP ISR empties shared_pram_codec_out0 or shared_newpr_codec_out0. 
 * 	- DSP sends STATUS_NEWAUDFIFO_DONEPLAY instead of STATUS_NEWAUDFIFO_SW_FIFO_EMPTY.
 * 	- ARM disables HW audio codec.
 * 	- ARM disables DSP ISR for HW audio FIFO using COMMAND_NEWAUDFIFO_CANCEL. 
 * 	- DSP will response with STATUS_NEWAUDFIFO_CANCELPLAY.
 *
 * \note The shared memory buffers used by this interface are re-used in the
 *       MM_VPU_PLAYBACK interface
 *
 * \see MM_VPU_PLAYBACK, COMMAND_NEWAUDFIFO_START, COMMAND_NEWAUDFIFO_PAUSE, 
 *      COMMAND_NEWAUDFIFO_RESUME, COMMAND_NEWAUDFIFO_CANCEL, STATUS_NEWAUDFIFO_SW_FIFO_LOW,
 *      STATUS_NEWAUDFIFO_SW_FIFO_EMPTY, STATUS_NEWAUDFIFO_CANCELPLAY,
 *      STATUS_NEWAUDFIFO_DONEPLAY
 *
 *  @{
 */
/**
 * This buffer is used for BOTH NEWAUDFIFO_Interface and 
 * MM_VPU_PLAYBACK interface (depending on the settings in shared_WB_AMR_Ctrl_state).\BR
 *
 * In both the cases it is used to store un-compressed PCM data.\BR
 * 
 * It's associated read and write pointers are shared_NEWAUD_OutBuf_in[0] and 
 * shared_NEWAUD_OutBuf_out[0].
 *
 * \see NEWAUDFIFO_Interface, MM_VPU_PLAYBACK, shared_WB_AMR_Ctrl_state, shared_NEWAUD_OutBuf_in,
 *      shared_NEWAUD_OutBuf_out
 */
EXTERN UInt16 shared_pram_codec_out0[AUDIO_SIZE_PER_PAGE]  	 SHARED_SEC_GEN_AUDIO;
/**
 * @}
 */
EXTERN UInt16 shared_audir_2nd[640]						   	 SHARED_SEC_GEN_AUDIO;

/**
 * \ingroup NEWAUDFIFO_Interface
 *
 * This buffer is used for BOTH NEWAUDFIFO_Interface and 
 * MM_VPU_PLAYBACK interface (depending on the settings in shared_WB_AMR_Ctrl_state).\BR
 *
 * In both the cases it is used to store un-compressed PCM data.\BR
 * 
 * It's associated read and write pointers are shared_NEWAUD_OutBuf_in[1] and 
 * shared_NEWAUD_OutBuf_out[1].
 *
 * \see NEWAUDFIFO_Interface, MM_VPU_PLAYBACK, shared_WB_AMR_Ctrl_state, shared_NEWAUD_OutBuf_in,
 *      shared_NEWAUD_OutBuf_out
 */
EXTERN UInt16 shared_newpr_codec_out0[1] 	 SHARED_SEC_GEN_AUDIO; // This buffer can be removed later on - currently changed to only 1 word

EXTERN UInt16 audio_input_16k_buf[1280]						  SHARED_SEC_GEN_AUDIO;
 
/**
 * \note THIS BUFFER IS NOT BEING USED AND CAN BE REMOVED !!!!!
 */
EXTERN UInt16 shared_encoder_InputBuffer[AUDIO_SIZE_PER_PAGE-1280] SHARED_SEC_GEN_AUDIO;

/** 
 * @}
 */
/** 
 * @addtogroup Audio_Gains_in_CP 
 * @{
 */
/** 
 * The gain factor, shared_usb_headset_gain_ul[1] is applied on the USB speech data
 * in the uplink path. \BR
 * 
 * This gain is a FIXED gain in Q14 format (Q14 means 2.14 format).\BR
 *
 */
EXTERN Int16  shared_usb_headset_gain_ul[5]				     SHARED_SEC_GEN_AUDIO;
/** 
 * The gain factor, shared_usb_headset_gain_dl[1] is applied on the USB speech data
 * in the downlink path. \BR
 * 
 * This gain is a FIXED gain in Q14 format (Q14 means 2.14 format).\BR
 *
 */
EXTERN Int16  shared_usb_headset_gain_dl[5]				     SHARED_SEC_GEN_AUDIO;
/** 
 * @}
 */

/** @addtogroup Shared_Audio_Buffers_Still_in_CP 
 * @{ */
/** @addtogroup BlueTooth_Mixer_Interface 
 * @{ */
EXTERN UInt16 shared_BTMIXER_OutputBuffer0[AUDIO_SIZE_PER_PAGE] SHARED_SEC_GEN_AUDIO;//!< Buffer containing the mono/left channel
                                                                                 //!< from the Bluetooth Mixer ADC.
                                                                                 //!< The stereo data can be either 
                                                                                 //!< interleaved or be non-interleaved
                                                                                 //!< \note This function would not be necessary 
                                                                                 //!< in Rhea as ARM can directly route the 
                                                                                 //!< output of the hardware
                                                                                 //!< mixer to where-ever it wants
EXTERN UInt16 shared_BTMIXER_OutputBuffer1[AUDIO_SIZE_PER_PAGE] SHARED_SEC_GEN_AUDIO; //!< Buffer containing the right channel
                                                                                 //!< of the non-interleaved stereo data
                                                                                 //!< or second half of the interleaved
                                                                                 //!< Bluetooth Mixer ADC stereo data
                                                                                 //!< \note This function would not be necessary 
                                                                                 //!< in Rhea as ARM can directly route the 
                                                                                 //!< output of the hardware
                                                                                 //!< mixer to where-ever it wants
/** @} */
/** @} */
EXTERN UInt16 shared_dac_mixer_filt_coef1[124]             SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_dac_mixer_filt_coef2[124]             SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_dac_mixer_filt_coef3[124]             SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_dac_mixer_filt_coef4[124]             SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_audio_adc_filt_coef1[54]              SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_audio_adc_filt_coef2[54]              SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_voice_adc_filt_coef1[39]              SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_voice_adc_filt_coef2[39]              SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_dac_audiir_filt_coef[25+4]            SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_dac_audfir_filt_coef[65+4]            SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_dac_audeq_filt_coef[25+4]             SHARED_SEC_GEN_AUDIO;
EXTERN Int16 shared_voice_filt_coefs[120]                  SHARED_SEC_GEN_AUDIO;
/** @addtogroup Shared_Audio_Buffers_Still_in_CP 
 * @{ */
/** @addtogroup WCDMA_encoded_data_interface_in_CP 
 * 
 * This interface is provided to transfer data back and forth simultaneously between the ARM and the DSP
 * for WCDMA or VoIP call. The data being transfered is in \Bold{AMR-NB\, AMR-WB\, FR or PCM formats only}. \BR
 *
 * The COMMAND_MAIN_AMR_RUN 
 * has to be sent by the MCU every-time data is sent to the DSP to do a decode for WCDMA/VOIP call. 
 * The input to the speech decoder comes in DL_MainAMR_buf.param, and the output of the encoder is 
 * stored in UL_MainAMR_buf.param \BR
 *
 * For every COMMAND_MAIN_AMR_RUN command, an associated STATUS_MAIN_AMR_DONE reply would be sent in the 
 * status queue. 
 * 
 * \note This interface can be only used for simultaneous capture and playback. It cannot be used for only
 *       capture or only playback of AMR/VoIP data.
 *
 * \BR
 *
 * \note This interface has to be able to run independently of the record and playback interface, as there
 *       can be use cases of record and playback during a WCDMA or VoIP call.
 * 
 * \see COMMAND_MAIN_AMR_RUN, DL_MainAMR_buf, UL_MainAMR_buf, STATUS_MAIN_AMR_DONE.
 * @{ */
EXTERN VR_Frame_AMR_WB_t DL_MainAMR_buf    				   SHARED_SEC_GEN_AUDIO; //!< This Buffer is used 
                                                                                 //!< to transfer the input to
                                                                                 //!< the speech decoder in the 
                                                                                 //!< DSP from 
                                                                                 //!< WCDMA channel decoder
                                                                                 //!< This buffer is filled by
                                                                                 //!< the ARM (as well as emtying
                                                                                 //!< the UL_MainAMR_buf buffer)
                                                                                 //!< and then it sends
                                                                                 //!< the COMMAND_MAIN_AMR_RUN
                                                                                 //!< command to the DSP to start
                                                                                 //!< decoding the encoded 
                                                                                 //!< AMR data
                                                                                 //!< \sa COMMAND_MAIN_AMR_RUN,
                                                                                 //!< STATUS_MAIN_AMR_DONE,
                                                                                 //!< UL_MainAMR_buf
EXTERN VR_Frame_AMR_WB_t UL_MainAMR_buf    				   SHARED_SEC_GEN_AUDIO; //!< This Buffer is used 
                                                                                 //!< to transfer the output of
                                                                                 //!< the speech encoder in 
                                                                                 //!< the DSP to 
                                                                                 //!< WCDMA channel encoder
                                                                                 //!< This buffer is filled by
                                                                                 //!< the DSP (as well as emtying
                                                                                 //!< the DL_MainAMR_buf buffer)
                                                                                 //!< and then it sends
                                                                                 //!< the STATUS_MAIN_AMR_DONE
                                                                                 //!< reply to the ARM to start
                                                                                 //!< channel encoding the 
                                                                                 //!< AMR data
                                                                                 //!< \sa COMMAND_MAIN_AMR_RUN,
                                                                                 //!< STATUS_MAIN_AMR_DONE,
                                                                                 //!< DL_MainAMR_buf
/** @} */
/** @} */
EXTERN VR_Frame_WCDMA_MST_AMR_t    DL_wcdma_mst_amr_buf    SHARED_SEC_GEN_AUDIO;
EXTERN UInt16    shared_vpu_memo_record_agc_data[5]        SHARED_SEC_GEN_AUDIO; // Uplink level measurement when the VPU is active
/** 
 * \ingroup Audio_Gains_in_CP 
 * 
 * This gain is not used anymore
 */
EXTERN UInt16    shared_vpu_memo_record_input_gain         SHARED_SEC_GEN_AUDIO; // Uplink gain block used during a memo record
EXTERN UInt16    shared_vp_audio_clip_level                SHARED_SEC_GEN_AUDIO; // Uplink clip level used during a memo record
EXTERN UInt16            shared_vp_compander_flag          SHARED_SEC_GEN_AUDIO; // VP memo recording compander enable/disable
/** @addtogroup Shared_Audio_Buffers_Still_in_CP 
 * @{ */

/** @addtogroup MM_VPU_Interface_in_CP 
 * @{ */
/**
 * \ingroup MM_VPU_PLAYBACK_in_CP
 * The use of this index is dependent on the settings in the shared_WB_AMR_Ctrl_state 
 * register. \BR
 * 
 * The first index is used to send back MM_VPU decoder's decoded data from the DSP to the AP. \BR
 *
 * The DSP writes the encoded data in shared_decoder_OutputBuffer with an index from shared_decodedSamples_buffer_in[0]. 
 * DSP updates the first index.
 *
 * \see shared_WB_AMR_Ctrl_state, shared_decoder_OutputBuffer 
 */
EXTERN UInt16 shared_decodedSamples_buffer_in[2]                         SHARED_SEC_GEN_AUDIO;
/**
 * \ingroup MM_VPU_PLAYBACK_in_CP
 * The use of this index is dependent on the settings in the shared_WB_AMR_Ctrl_state 
 * register. \BR
 * 
 * The first index is used to send back MM_VPU decoder's decoded data from the DSP to the AP. \BR
 *
 * The ARM reads the encoded data from the shared_decoder_OutputBuffer with an index from 
 * shared_decodedSamples_buffer_out[0]. 
 * ARM updates the first index.
 *
 * \see shared_WB_AMR_Ctrl_state, shared_decoder_OutputBuffer 
 */
EXTERN UInt16 shared_decodedSamples_buffer_out[2]                         SHARED_SEC_GEN_AUDIO;

/**
 * \ingroup MM_VPU_RECORD_in_CP
 */
/**
 * The first flag (shared_decodedSamples_done_flag[0]) is used while doing 
 * MM_VPU recording (encoding). This flag 
 * notifies the ARM that the DSP has completed receiving 320 samples 
 * from the microphone interface, and it still needs to be encoded. 
 *
 * \note The first flag (shared_decodedSamples_done_flag[0]) is currently only used 
 * by the DSP, and is not useful for the ARM. */
EXTERN UInt16 shared_decodedSamples_done_flag[2]                         SHARED_SEC_GEN_AUDIO;

/**
 * \ingroup MM_VPU_PLAYBACK_in_CP
 */
/**
 * This flag is used during MM_VPU playback, to indicate that the ARM has finished 
 * playback of the encoded data, and the DSP should not expect any more data to come
 * from the ARM side. This use of this flag depends on the settings in the 
 * shared_WB_AMR_Ctrl_state register
 *
 * \see shared_WB_AMR_Ctrl_state
 */
EXTERN UInt16 shared_encodedSamples_done_flag[2]                         SHARED_SEC_GEN_AUDIO;
/** @} */
/** @} */
EXTERN UInt16 shared_stereoWidening_buffer_in[2]                         SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_stereoWidening_buffer_out[2]                         SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_stereoWidening_done_flag[2]                         SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_resvd_buffer_in[2]                                     SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_resvd_buffer_out[2]                                 SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_resvd_buffer_done_flag[2]                             SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_sw_fifo_out[2]                                         SHARED_SEC_GEN_AUDIO;
EXTERN Int16 shared_aacenc_bt_page                                         SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_aacenc_bt_buffersize                                 SHARED_SEC_GEN_AUDIO;                        //    Should be <= 4096 words
EXTERN UInt16 shared_PCM_OutBuf_Select                                     SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_Which_DSP_Select                                     SHARED_SEC_GEN_AUDIO;
EXTERN Int16 shared_adap_fir_8PSK[ 15 ]                                     SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_rom_codec_vec0                                         SHARED_SEC_GEN_AUDIO;                        // prgrammable states
EXTERN UInt16 shared_rom_codec_vec1                                         SHARED_SEC_GEN_AUDIO;                        // prgrammable states
EXTERN UInt16 shared_rom_codec_vec2                                         SHARED_SEC_GEN_AUDIO;                        // prgrammable states
EXTERN UInt16 shared_rom_codec_vec3                                         SHARED_SEC_GEN_AUDIO;                        // prgrammable states
EXTERN UInt16 shared_rom_codec_vec4                                         SHARED_SEC_GEN_AUDIO;                        // prgrammable states

/** @addtogroup Shared_Audio_Buffers_Still_in_CP 
 * @{ */
/** @addtogroup MM_VPU_Interface_in_CP 
 * @{ */
/** 
 * This flag is used internally in the MM VPU interface by the DSP to store local 
 * information about the state - e.g. whether record or playback is in progress
 */
EXTERN UInt16 shared_16ISR_state                                            SHARED_SEC_GEN_AUDIO;                        // prgrammable states
/** @} */
/** @} */
EXTERN UInt16 shared_rom_codec_vec5                                         SHARED_SEC_GEN_AUDIO;                        // prgrammable states
EXTERN UInt16 shared_rom_codec_vec6                                         SHARED_SEC_GEN_AUDIO;                        // prgrammable states
EXTERN UInt16 shared_rom_codec_vec7                                         SHARED_SEC_GEN_AUDIO;                        // prgrammable pointers
//EXTERN Int16 shared_CNG_seed                                              SHARED_SEC_GEN_AUDIO;                        // Filtered and scaled CNG from the ARM
//EXTERN Int16 shared_CNG_bias                                              SHARED_SEC_GEN_AUDIO; 

/** @addtogroup Shared_Audio_Buffers_Still_in_CP 
 * @{ */
/**
 * @addtogroup NEWAUDFIFO_Interface
 * @{
 */ 

/**
 * These Read indices are used for BOTH NEWAUDFIFO_Interface and 
 * MM_VPU_PLAYBACK interface (depending on the settings in shared_WB_AMR_Ctrl_state).\BR
 *
 * In both the cases they are used to the point till which un-compressed PCM data is written.\BR
 * 
 * Their associated buffers are shared_pram_codec_out0 and shared_newpr_codec_out0 respectively.\BR
 *
 * In case of NEWAUDFIFO_Interface interface, ARM updates these indices, whereas in case of 
 * MM_VPU_PLAYBACK interface, DSP updates these indices.
 *
 * \see NEWAUDFIFO_Interface, MM_VPU_PLAYBACK, shared_WB_AMR_Ctrl_state, shared_pram_codec_out0, 
 *      shared_newpr_codec_out0
 */
EXTERN Int16 shared_NEWAUD_OutBuf_in[2]                                  SHARED_SEC_GEN_AUDIO;                        //dsp/arm updates after adding decoded output samples

/**
 * These Write indices are used for BOTH NEWAUDFIFO_Interface and 
 * MM_VPU_PLAYBACK interface (depending on the settings in shared_WB_AMR_Ctrl_state).\BR
 *
 * In both the cases they are used to the point from which un-compressed PCM data is to be read.\BR
 * 
 * Their associated buffers are shared_pram_codec_out0 and shared_newpr_codec_out0 respectively.\BR
 *
 * In case of NEWAUDFIFO_Interface interface, DSP updates these indices, whereas in case of 
 * MM_VPU_PLAYBACK interface, ARM updates these indices.
 *
 * \see NEWAUDFIFO_Interface, MM_VPU_PLAYBACK, shared_WB_AMR_Ctrl_state, shared_pram_codec_out0, 
 *      shared_newpr_codec_out0
 */
EXTERN Int16 shared_NEWAUD_OutBuf_out[2]                                 SHARED_SEC_GEN_AUDIO;                        //arm(or dsp FIFO ISR) updates after using decoded output samples

/** @} */
/** @} */

EXTERN UInt16 shared_STW_execution_Flag                                     SHARED_SEC_GEN_AUDIO;                        //0 == BYPASS; 1 == ENGAGE
EXTERN UInt16 shared_STW_headset_config                                     SHARED_SEC_GEN_AUDIO;                        //0 == Speakers; 1 == Headphones
EXTERN UInt16 shared_STW_compressor_Flag                                 	SHARED_SEC_GEN_AUDIO;                        //0 == DISABLE; 1 == ENABLE
EXTERN UInt16 shared_STW_sample_rate_L                                     	SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_STW_sample_rate_H                                     	SHARED_SEC_GEN_AUDIO;
EXTERN Int16 shared_STW_delay                                             	SHARED_SEC_GEN_AUDIO;
EXTERN Int16 shared_STW_inputScale                                         	SHARED_SEC_GEN_AUDIO;                        //Input signal scaling factor
EXTERN Int16 shared_STW_monoScale                                         	SHARED_SEC_GEN_AUDIO;
EXTERN Int16 shared_STW_center                                            	SHARED_SEC_GEN_AUDIO;
EXTERN Int16 shared_STW_spread                                             	SHARED_SEC_GEN_AUDIO;
EXTERN Int16 shared_STW_rsvd_parms_1                                     	SHARED_SEC_GEN_AUDIO;
EXTERN Int16 shared_STW_rsvd_parms_2                                     	SHARED_SEC_GEN_AUDIO;

EXTERN Int16 shared_Outbuf_Freq_Sts_TH                                   	SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_dac_polyiir_filt_coef[25+4]                         	SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_dac_polyfir_filt_coef[65+4]                         	SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_dac_polyeq_filt_coef[25+4]                          	SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_voice_dac_filt_coef1[39]                            	SHARED_SEC_GEN_AUDIO;
EXTERN Int16 shared_coef_FIR1[11]                                        	SHARED_SEC_GEN_AUDIO;    					// N+1/2 Filter coefs for FIR1

/******************************************
//Shared memory used by Dual Mic
******************************************/
EXTERN Int16 shared_dmic_slope_min											SHARED_SEC_GEN_AUDIO;
EXTERN Int16 shared_dmic_slope_max											SHARED_SEC_GEN_AUDIO;
EXTERN Int16 shared_dmic_R_max												SHARED_SEC_GEN_AUDIO;
EXTERN Int16 shared_dmic_nlp_parms[64]										SHARED_SEC_GEN_AUDIO;
EXTERN Int16 shared_dmic_init[200]											SHARED_SEC_GEN_AUDIO;
/******************************************
//End Shared memory used by Dual Mic
******************************************/
/** 
 * \ingroup Audio_Gains_in_CP 
 * 
 * This gain is applied to the MM-VPU WB-AMR decoder's output
 */
EXTERN UInt16 shared_wb_mm_output_gain[5]									SHARED_SEC_GEN_AUDIO;

/**
 * This memory section describes the setting of the voice compensation filter for 16/8K Voice only.
 * The filter is the form DF2 and programmable stages for UL and DL and Aux Ul for the second MIC path.
 * The memory is as follows:
 * 							input gain[]
 *							Coef Fwd B[][]
 *							Coef Bwd A[][]
 *							OutputSelect
 *							Maximum IIR Stage supported = 12 in each direction
 * WARNING DO NOT CHANGE THE ORDER. it will break the order of the copy for swapping ext<->int memory.
 */

EXTERN Int32  shared_dl_coef_fw_8k[12][3]									SHARED_SEC_GEN_AUDIO;
EXTERN Int32  shared_dl_coef_bw_8k[12][2]									SHARED_SEC_GEN_AUDIO;
EXTERN Int16  shared_dl_comp_filter_gain_8k[12]								SHARED_SEC_GEN_AUDIO;
EXTERN Int32  shared_dl_output_bit_select_8k								SHARED_SEC_GEN_AUDIO;

EXTERN Int32  shared_dl_coef_fw_16k[12][3]									SHARED_SEC_GEN_AUDIO;
EXTERN Int32  shared_dl_coef_bw_16k[12][2]									SHARED_SEC_GEN_AUDIO;
EXTERN Int16  shared_dl_comp_filter_gain_16k[12]							SHARED_SEC_GEN_AUDIO;
EXTERN Int32  shared_dl_output_bit_select_16k								SHARED_SEC_GEN_AUDIO;


EXTERN Int32  shared_ul_coef_fw_8k[12][3]									SHARED_SEC_GEN_AUDIO;
EXTERN Int32  shared_ul_coef_bw_8k[12][2]									SHARED_SEC_GEN_AUDIO;
EXTERN Int16  shared_ul_comp_filter_gain_8k[12]								SHARED_SEC_GEN_AUDIO;
EXTERN Int32  shared_ul_output_bit_select_8k								SHARED_SEC_GEN_AUDIO;


EXTERN Int32  shared_ul_coef_fw_16k[12][3]									SHARED_SEC_GEN_AUDIO;
EXTERN Int32  shared_ul_coef_bw_16k[12][2]									SHARED_SEC_GEN_AUDIO;
EXTERN Int16  shared_ul_comp_filter_gain_16k[12]							SHARED_SEC_GEN_AUDIO;
EXTERN Int32  shared_ul_output_bit_select_16k								SHARED_SEC_GEN_AUDIO;


EXTERN Int32  shared_ul_Auxcoef_fw_8k[12][3]								SHARED_SEC_GEN_AUDIO;
EXTERN Int32  shared_ul_Auxcoef_bw_8k[12][2]								SHARED_SEC_GEN_AUDIO;
EXTERN Int16  shared_ul_Auxcomp_filter_gain_8k[12]							SHARED_SEC_GEN_AUDIO;
EXTERN Int32  shared_ul_Auxoutput_bit_select_8k								SHARED_SEC_GEN_AUDIO;


EXTERN Int32  shared_ul_Auxcoef_fw_16k[12][3]								SHARED_SEC_GEN_AUDIO;
EXTERN Int32  shared_ul_Auxcoef_bw_16k[12][2]								SHARED_SEC_GEN_AUDIO;
EXTERN Int16  shared_ul_Auxcomp_filter_gain_16k[12]							SHARED_SEC_GEN_AUDIO;
EXTERN Int32  shared_ul_Auxoutput_bit_select_16k							SHARED_SEC_GEN_AUDIO;



EXTERN UInt16 shared_dl_nstage_filter										SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_ul_nstage_filter										SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_ul_Auxnstage_filter									SHARED_SEC_GEN_AUDIO;



EXTERN UInt16 shared_DTMF_SV_tone_scale_mode	   							SHARED_SEC_GEN_AUDIO;			 // 1: new mode (sacle factor coming from arm); 0: old mode (DSP set scale factor)
EXTERN UInt16 shared_RF_test_buf_ind                    					SHARED_SEC_DIAGNOS;
EXTERN Shared_RF_rx_test_data_t shared_RF_rx_test_data[2]                 	SHARED_SEC_DIAGNOS;                        //RF rx test data buffer
EXTERN Shared_RF_tx_test_data_t shared_RF_tx_test_data[MAX_TX_SLOTS]      	SHARED_SEC_DIAGNOS;                        //RF tx test data buffer
EXTERN UInt16 shared_track_data_ind                                         SHARED_SEC_DIAGNOS;
EXTERN UInt16 shared_sacch_track_data_ind                                   SHARED_SEC_DIAGNOS;    
EXTERN Shared_TrackData_t shared_track_data[2]                              SHARED_SEC_DIAGNOS;    
EXTERN Shared_SacchTrackData_t shared_sacch_track_data[2]                  	SHARED_SEC_DIAGNOS;
EXTERN UInt16 shared_speech_dlink_info[3]                                  	SHARED_SEC_DIAGNOS;    
EXTERN UInt16 shared_speech_ulink_info[3]                                  	SHARED_SEC_DIAGNOS;    
EXTERN UInt16 shared_inband_dist[4]                                         SHARED_SEC_DIAGNOS;
EXTERN UInt16 shared_FER_counter                                          	SHARED_SEC_DIAGNOS;    
EXTERN UInt16 shared_FER_counter_CRC                                      	SHARED_SEC_DIAGNOS;    
EXTERN UInt16 shared_total_speech_frame                                     SHARED_SEC_DIAGNOS;
EXTERN UInt16 shared_reset_speech_counter_flag                              SHARED_SEC_DIAGNOS;    
EXTERN UInt16 shared_continuous_bfi_counter                                 SHARED_SEC_DIAGNOS;    
EXTERN UInt16 shared_FER_counter_wcdma_hw                                  	SHARED_SEC_DIAGNOS;
EXTERN UInt16 shared_FER_counter_wcdma_sw       							SHARED_SEC_DIAGNOS;
EXTERN UInt16 shared_total_speech_frame_wcdma_hw                          	SHARED_SEC_DIAGNOS;    
EXTERN UInt16 	shared_select_SAIC_flag										SHARED_SEC_DIAGNOS;							//MSB is frame SAIC flag, bit14=0/1=saic_bp_buffer/mst_bp_buffer, with bit11 to bit8 for its burst 3 to 0; bit 3 to bit 0 for newly RX bursts
EXTERN Int16 	shared_SNR_before_SAIC[4]									SHARED_SEC_DIAGNOS;	
EXTERN UInt16   shared_sum_vit_input                           				SHARED_SEC_DIAGNOS;
EXTERN UInt16   shared_sum_vit_input_dec_SAIC                  				SHARED_SEC_DIAGNOS; 
EXTERN UInt16   shared_sum_vit_input_dec_MST                   				SHARED_SEC_DIAGNOS;
EXTERN UInt16   shared_vit_metric_SAIC                         				SHARED_SEC_DIAGNOS;
EXTERN UInt16   shared_vit_metric_MST                          				SHARED_SEC_DIAGNOS;
EXTERN UInt16   shared_bec_SAIC                                				SHARED_SEC_DIAGNOS;
EXTERN UInt16   shared_bec_MST                                 				SHARED_SEC_DIAGNOS;
EXTERN UInt16 shared_SCH_track_snr_rxlev            						SHARED_SEC_DIAGNOS;
EXTERN UInt16 shared_SCH_track_timing                  						SHARED_SEC_DIAGNOS;
EXTERN Int16 shared_SCH_track_freq_offset[2]          						SHARED_SEC_DIAGNOS;
EXTERN Shared_BP_TrackData_t shared_BP_track_data[2]						SHARED_SEC_DIAGNOS;
                                                                          
EXTERN UInt16 shared_dbg_voice_dac[25]                              		SHARED_SEC_DSP_DEBUG;    
EXTERN UInt16 shared_testpoint_data_buf_in_idx                              SHARED_SEC_DSP_DEBUG;                        // shared_testpoint_data_buf_idx-1 == last location in shared_testpoint_data_buf written by DSP
EXTERN UInt16 shared_testpoint_data_buf_out_idx                             SHARED_SEC_DSP_DEBUG;                        // shared_testpoint_data_buf_idx-1 == last location in shared_testpoint_data_buf written by DSP
EXTERN UInt16 shared_testpoint_data_buf_wrap_idx                            SHARED_SEC_DSP_DEBUG;                        // Defines the last valid location within the testpoint buffer. Allows run-time resizing of the testpoint buffer
EXTERN UInt16 shared_debug[ 148 ]                                           SHARED_SEC_DSP_DEBUG;                        //debug points
EXTERN UInt16 shared_dbg_xram_BT_DL_filt_coefs[10]                          SHARED_SEC_DSP_DEBUG;    
EXTERN UInt16 shared_dbg_xram_BT_UL_filt_coefs[10]                          SHARED_SEC_DSP_DEBUG;    
EXTERN UInt16 shared_dbg_xram_comp_filt_coefs[10]                          	SHARED_SEC_DSP_DEBUG;
EXTERN UInt16 shared_dbg_xram_ec_deemp_preemp_filt[4]                      	SHARED_SEC_DSP_DEBUG;    
EXTERN expander_parm shared_dbg_xram_sidetone_expander                      SHARED_SEC_DSP_DEBUG;    
EXTERN UInt16 shared_dbg_xram_sidetone_expander_flag                      	SHARED_SEC_DSP_DEBUG;
EXTERN UInt16 shared_dbg_xram_sidetone_coefs[10]                          	SHARED_SEC_DSP_DEBUG;    
EXTERN UInt16 shared_dbg_pg_aud_in_enable                                  	SHARED_SEC_DSP_DEBUG;    
EXTERN UInt16 shared_dbg_pg_aud_out_enable                                  SHARED_SEC_DSP_DEBUG;
EXTERN UInt16 shared_dbg_pg_dsp_sidetone_enable                             SHARED_SEC_DSP_DEBUG;    
EXTERN UInt16 shared_dbg_pg_sidetone_biquad_sys_gain                      	SHARED_SEC_DSP_DEBUG;    
EXTERN UInt16 shared_dbg_pg_sidetone_biquad_scale_factor                  	SHARED_SEC_DSP_DEBUG;
EXTERN UInt16 shared_dbg_pg_sidetone_output_gain                          	SHARED_SEC_DSP_DEBUG;
EXTERN UInt16 shared_dbg_pg_aud_tone_scale                                  SHARED_SEC_DSP_DEBUG;
EXTERN UInt16 shared_dbg_pg_aud_tone_status                                 SHARED_SEC_DSP_DEBUG;
EXTERN UInt16 shared_dbg_pg_aud_silence                                     SHARED_SEC_DSP_DEBUG;
EXTERN UInt16 shared_dbg_voice_adc[35]                                      SHARED_SEC_DSP_DEBUG;
EXTERN UInt16 shared_dbg_pg_aud_superimpose                                 SHARED_SEC_DSP_DEBUG;
EXTERN UInt16 shared_dbg_g1_flags                                          	SHARED_SEC_DSP_DEBUG;
EXTERN UInt16 shared_dbg_msgproc_audio_flags                              	SHARED_SEC_DSP_DEBUG;
EXTERN UInt16 shared_dbg_pg_aud_sdsen_enable                              	SHARED_SEC_DSP_DEBUG;
EXTERN UInt16 shared_dbg_AMCR                                              	SHARED_SEC_DSP_DEBUG;
EXTERN UInt16 shared_stack_depth_check_thresh_buf[5]                      	SHARED_SEC_DSP_DEBUG;
EXTERN UInt16 shared_stack_depth_check_res_buf[5]                          	SHARED_SEC_DSP_DEBUG;
EXTERN UInt32 shared_modem_logging_align_flag								SHARED_SEC_DSP_DEBUG;
EXTERN UInt16 shared_modem_logging_buf[1280]								SHARED_SEC_DSP_DEBUG;
EXTERN UInt16 shared_testpoint_data_buf[2048]                              	SHARED_SEC_DSP_DEBUG;                        // 32bit address Aligned Testpoint data buffer for implementing DSP testpoints via the ARM

EXTERN UInt16 shared_modem_logging_dsp_write_idx							SHARED_SEC_DSP_DEBUG;
EXTERN UInt16 shared_modem_logging_mcu_read_idx								SHARED_SEC_DSP_DEBUG;
EXTERN UInt16 shared_modem_logging_resvd_idx								SHARED_SEC_DSP_DEBUG;
EXTERN UInt16 shared_modem_logging_enable									SHARED_SEC_DSP_DEBUG;
EXTERN UInt16 shared_memory_size                                     		SHARED_SEC_DSP_DEBUG;            
EXTERN UInt16 shared_access_kludge                                    		SHARED_SEC_DSP_DEBUG;

/**
 * @addtogroup Audio_Gains_in_CP
 * @{
 */
/** 
 * The gain factor, shared_arm2speech2_call_gain_dl is applied on the shared_Arm2SP2_InBuf[1280] PCM data 
 * on the downlink path (does not matter whether the ARM2SP2 data is added before or after audio processing.\BR
 * 
 * This gain is a ramped gain in Q14 format as shown below (Q14 means 2.14 format).\BR
 *
 * {
 * -   Int16 Start_Gain_in_Q14,
 * -   Int16 Target_Gain_in_Q14,
 * -   Int16 Step_size_to_increment_or_decrement_the_Gain_in_Q14,  (<= 0 make Start_Gain_in_Q14 = Target_Gain_in_Q14)
 * -   Int16 Reserved,
 * -   Int16 Reserved \BR
 * }
 *
 */
EXTERN Int16	shared_arm2speech2_call_gain_dl[5]			 				SHARED_SEC_DSP_DEBUG;
/** 
 * The gain factor, shared_arm2speech2_call_gain_ul is applied on the shared_Arm2SP2_InBuf[1280] PCM data 
 * on the uplink path (does not matter whether the ARM2SP2 data is added before or after audio processing.\BR
 * 
 * This gain is a ramped gain in Q14 format as shown below (Q14 means 2.14 format).\BR
 *
 * {
 * -   Int16 Start_Gain_in_Q14,
 * -   Int16 Target_Gain_in_Q14,
 * -   Int16 Step_size_to_increment_or_decrement_the_Gain_in_Q14,  (<= 0 make Start_Gain_in_Q14 = Target_Gain_in_Q14)
 * -   Int16 Reserved,
 * -   Int16 Reserved \BR
 * }
 *
 */
EXTERN Int16	shared_arm2speech2_call_gain_ul[5]			 				SHARED_SEC_DSP_DEBUG;
/** 
 * The gain factor, shared_arm2speech2_call_gain_rec is applied on the ARM2SP2 PCM data 
 * getting recorded.\BR
 * 
 * This gain is a ramped gain in Q14 format as shown below (Q14 means 2.14 format).\BR
 *
 * {
 * -   Int16 Start_Gain_in_Q14,
 * -   Int16 Target_Gain_in_Q14,
 * -   Int16 Step_size_to_increment_or_decrement_the_Gain_in_Q14,  (<= 0 make Start_Gain_in_Q14 = Target_Gain_in_Q14)
 * -   Int16 Reserved,
 * -   Int16 Reserved \BR
 * }
 *
 */
EXTERN Int16	shared_arm2speech2_call_gain_rec[5]			 				SHARED_SEC_DSP_DEBUG;
/**
 * @}
 */
EXTERN UInt16 shared_g_sys_flag												SHARED_SEC_DSP_DEBUG;


// PART TO BE REMOVED LATER AFTER VALIDATION
EXTERN Int16 sharedPR_eventIn                                          		SHARED_SEC_DSP_DEBUG;	// TO BE REMOVED
EXTERN Int16 sharedPR_eventOut                                          	SHARED_SEC_DSP_DEBUG;
EXTERN UInt16 sharedPR_voices_stopped[5]                              		SHARED_SEC_DSP_DEBUG;
EXTERN UInt16 sharedPR_paused                                          		SHARED_SEC_DSP_DEBUG;
/** 
 * @addtogroup Audio_Gains_in_CP 
 * @{
 */
/** 
 * This gain is not used anymore
 */
EXTERN Int16    shared_polyringer_out_gain_dl                         		SHARED_SEC_DSP_DEBUG;
/** 
 * This gain is not used anymore
 */
EXTERN Int16    shared_polyringer_out_gain_ul                         		SHARED_SEC_DSP_DEBUG;
/**
 * @}
 */
EXTERN UInt16 shared_coef_poly_DAC_IIR[ 25 ]                          		SHARED_SEC_DSP_DEBUG;
EXTERN Int16  shared_polyringer_stop_flag                             		SHARED_SEC_DSP_DEBUG;
EXTERN UInt16 shared_polyringer_out_lo                                 		SHARED_SEC_DSP_DEBUG;
EXTERN UInt16 shared_polyringer_out_hi                                 		SHARED_SEC_DSP_DEBUG;
EXTERN UInt16	sharedPR_codec_buffer_out                             		SHARED_SEC_DSP_DEBUG;
EXTERN UInt16	sharedPR_codec_buffer[PR_SW_FIFO_SIZE]                 	SHARED_SEC_DSP_DEBUG;
EXTERN UInt16	sharedPR_codec_buffer_in                             		SHARED_SEC_DSP_DEBUG;
EXTERN UInt16	sharedPR_codec_buffer_out_ul                         		SHARED_SEC_DSP_DEBUG;
EXTERN UInt16	shared_debug_sfr_records_read								SHARED_SEC_DSP_DEBUG;
EXTERN UInt16	shared_sfr_records_write_idx								SHARED_SEC_DSP_DEBUG;
EXTERN UInt16	shared_dsp_dump_ctrl                                		SHARED_SEC_DSP_DEBUG;
EXTERN UInt16	shared_dump[100]                                        	SHARED_SEC_DSP_DEBUG;
EXTERN UInt32 shared_memory_end                                        		SHARED_SEC_DSP_DEBUG;


#ifdef MSP
} Dsp_SharedMem_t;
#endif

#ifdef MSP

#if (defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_COMMS_PROCESSOR))
typedef        Dsp_SharedMem_t Unpaged_SharedMem_t;  
typedef        Dsp_SharedMem_t SharedMem_t        ;
typedef        Dsp_SharedMem_t VPSharedMem_t ;
typedef        Dsp_SharedMem_t PAGE1_SharedMem_t  ;
typedef        Dsp_SharedMem_t Shared_poly_events_t;       
typedef        Dsp_SharedMem_t Shared_RF_rxtx_test_Page_t; 
typedef        Dsp_SharedMem_t Shared_poly_audio_t;
typedef        Dsp_SharedMem_t PAGE5_SharedMem_t  ;
typedef        Dsp_SharedMem_t PAGE6_SharedMem_t  ;
typedef        Dsp_SharedMem_t PAGE7_SharedMem_t  ;
typedef        Dsp_SharedMem_t PAGE10_SharedMem_t ;
typedef        Dsp_SharedMem_t PAGE11_SharedMem_t ;
typedef        Dsp_SharedMem_t PAGE14_SharedMem_t ;
typedef        Dsp_SharedMem_t PAGE15_SharedMem_t ;
typedef        Dsp_SharedMem_t PAGE18_SharedMem_t ;
typedef        Dsp_SharedMem_t PAGE21_SharedMem_t ;
typedef        Dsp_SharedMem_t PAGE24_SharedMem_t ;
typedef        Dsp_SharedMem_t PAGE27_SharedMem_t ;
typedef        Dsp_SharedMem_t PAGE28_SharedMem_t ;
typedef        Dsp_SharedMem_t PAGE29_SharedMem_t ;
typedef        Dsp_SharedMem_t PAGE30_SharedMem_t ;
typedef        Dsp_SharedMem_t PAGE31_SharedMem_t ;
typedef        Dsp_SharedMem_t PAGE32_SharedMem_t ;
typedef        Dsp_SharedMem_t PAGE33_SharedMem_t ;
typedef        Dsp_SharedMem_t PAGE34_SharedMem_t ;
typedef        Dsp_SharedMem_t PAGE35_SharedMem_t ;
typedef        Dsp_SharedMem_t PAGE36_SharedMem_t ;
typedef        Dsp_SharedMem_t PAGE37_SharedMem_t ;
typedef        Dsp_SharedMem_t PAGE38_SharedMem_t ;


void SHAREDMEM_Init( void );            // Initialize Shared Memory

void SHAREDMEM_ReadBuffer16(            // Read the received message
    UInt8 index,                        // Rx buffer to use
    UInt8 *bfi_flag,                    // FALSE, message error
    UInt8 *frame_index,                    // Last frame index of message
    UInt8 octets,                        // message size (in octets)
    UInt16 *contents                        // Pointer to received message storage
    );

void SHAREDMEM_ReadBuffer(                // Read the received message
    UInt8 index,                        // Rx buffer to use
    UInt8 *bfi_flag,                    // FALSE, message error
    UInt8 *bec,                            // Bit error count
    UInt8 *frame_index,                    // Last frame index of message
    Int8 *adj,                            // Frame adjust (for SCH only)
    Int16 *otd,                            // Observed time difference, half-bits (for SCH only)
    RxCode_t *code,                        // Decoding mode
    UInt8 *octets,                        // # of octets in message
    UInt8 *cn_db16,                        // Cn (in dB/16)
    UInt8 *bl_var,                        // BL_VAR
    UInt8 *contents                        // Pointer to received message storage
    );

void SHAREDMEM_WriteBuffer(                // Post a message to send
    UInt8 index,                        // Tx buffer to use
    TxCode_t code,                        // Encoding mode
    UInt8 *contents                        // Pointer to message to send
    );

void SHAREDMEM_WriteGPRSBuffer(            // Post a message to send
    UInt8 index,                        // Tx buffer to use
    TxCode_t coding_scheme,                // tx data encoding mode
    UInt8 *contents                        // Pointer to message to send
    );

UInt16 SHAREDMEM_ReadMonStatus(            // Read the neighbor power status
    UInt8    buff_id,                    // Monitor Buffer Id
    UInt8    frame_index                    // Frame index to get status
    );

UInt8 SHAREDMEM_ReadFrameIndex( void );    // Return the current frame index

void SHAREDMEM_PostCmdQ(                // Post an entry to the command queue
    CmdQ_t *cmd_msg                        // Entry to post
    );

Boolean SHAREDMEM_ReadStatusQ(            // Read an entry from the command queue
    StatQ_t *status_msg                    // Entry from queue
    );                                    // TRUE, if entry is available

void SHAREDMEM_ReadMeasReport(            // Read the measurement report
    UInt16 *rxqual_full,                // Pointer to array of four elements
    UInt16 *rxqual_sub,                    // Pointer to array of four elements
    UInt16 *rxlev_full,                    // Pointer to array of four elements
    UInt16 *rxlev_sub                    // Pointer to array of four elements
    );

void SHAREDMEM_WriteTestPoint(            // Write to a test point
    UInt8    tp,                            // Test Point
    UInt16    value                        // Test Point Value
    );


Int16 *SHAREDMEM_ReadSNR( void );

UInt16 *SHAREDMEM_ReadMetric( void );

UInt16 *SHAREDMEM_ReadBFI_BEC( void );
void SHAREDMEM_RefreshAMREntry( UInt8 cmr);

void SHAREDMEM_GetEdgeRxBufPtr(UInt8 index, EDGE_RxBuf_t **SharedMemRxBuf);

void SHAREDMEM_GetEdgeTxBufPtr(UInt8 index, EDGE_TxBuf_t **SharedMemTxBuf);

void SHAREDMEM_SetAMRParms( 
    UInt16 icm,
    UInt16 acs,
    UInt16 direction
    );
    
UInt16 *SHAREDMEM_GetUlinkSpeechInfoPtr( void );
UInt16 *SHAREDMEM_GetDlinkSpeechInfoPtr( void );
UInt16 *SHAREDMEM_GetInbandDistPtr( void );

void SHAREDMEM_SetRatscchMarker_DisableFlag( Boolean flag );
SharedMem_t             *SHAREDMEM_GetSharedMemPtr( void );// Return pointer to shared memory
Unpaged_SharedMem_t     *SHAREDMEM_GetUnpagedSharedMemPtr(void);// Return pointer to unpaged shared memory
PAGE1_SharedMem_t         *SHAREDMEM_GetPage1SharedMemPtr(void);// Return pointer to Page1 shared memory
Shared_poly_events_t    *SHAREDMEM_Get_poly_events_MemPtr(void);// Return pointer to polyringer event buffer
Shared_poly_audio_t        *SHAREDMEM_GetPage4SharedMemPtr(void);// Return pointer tp page4 shared memory
PAGE5_SharedMem_t         *SHAREDMEM_GetPage5SharedMemPtr(void);// Return pointer to Page5 shared memory
PAGE6_SharedMem_t         *SHAREDMEM_GetPage6SharedMemPtr(void);// Return pointer to Page6 shared memory
PAGE7_SharedMem_t         *SHAREDMEM_GetPage7SharedMemPtr(void);// Return pointer to Page7 shared memory
PAGE10_SharedMem_t         *SHAREDMEM_GetPage10SharedMemPtr(void);// Return pointer to Page10 shared memory
PAGE11_SharedMem_t         *SHAREDMEM_GetPage11SharedMemPtr(void);// Return pointer to Page11 shared memory
PAGE14_SharedMem_t         *SHAREDMEM_GetPage14SharedMemPtr(void);// Return pointer to Page14 shared memory
PAGE15_SharedMem_t         *SHAREDMEM_GetPage15SharedMemPtr(void);// Return pointer to Page15 shared memory
PAGE18_SharedMem_t         *SHAREDMEM_GetPage18SharedMemPtr(void);// Return pointer to Page18 shared memory
PAGE21_SharedMem_t         *SHAREDMEM_GetPage21SharedMemPtr(void);// Return pointer to Page21 shared memory
PAGE24_SharedMem_t         *SHAREDMEM_GetPage24SharedMemPtr(void);// Return pointer to Page24 shared memory
PAGE27_SharedMem_t         *SHAREDMEM_GetPage27SharedMemPtr(void);// Return pointer to Page27 shared memory
PAGE28_SharedMem_t         *SHAREDMEM_GetPage28SharedMemPtr(void);// Return pointer to Page28 shared memory
PAGE29_SharedMem_t         *SHAREDMEM_GetPage29SharedMemPtr(void);// Return pointer to Page29 shared memory
PAGE30_SharedMem_t         *SHAREDMEM_GetPage30SharedMemPtr(void);// Return pointer to Page30 shared memory
SharedMem_t                *SHAREDMEM_GetRFTest_MemPtr(void);

Dsp_SharedMem_t         *SHAREDMEM_GetDsp_SharedMemPtr( void );                    // Return pointer to shared memory

SPI_Records_t *SHAREDMEM_GetSharedMemRfSpiPtr(void);			// Return poiter to RF SPI (MIPI) structure

extern  Dsp_SharedMem_t    *vp_shared_mem;

#endif    // if (defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_COMMS_PROCESSOR))

#endif	// MSP

//******************************************************************************
// Function Prototypes
//******************************************************************************
#ifdef RIP
void SHARED_Init( void );                // Initialize the shared memory
#endif    // RIP



/**
 * @}
 */

#endif	// _INC_SHARED_CP_H_
