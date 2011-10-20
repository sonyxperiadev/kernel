//*********************************************************************
//
//	Copyright © 2008 Broadcom Corporation
//	
//	This program is the proprietary software of Broadcom Corporation 
//	and/or its licensors, and may only be used, duplicated, modified 
//	or distributed pursuant to the terms and conditions of a separate, 
//	written license agreement executed between you and Broadcom (an 
//	"Authorized License").  Except as set forth in an Authorized 
//	License, Broadcom grants no license (express or implied), right 
//	to use, or waiver of any kind with respect to the Software, and 
//	Broadcom expressly reserves all rights in and to the Software and 
//	all intellectual property rights therein.  IF YOU HAVE NO 
//	AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE 
//	IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE 
//	ALL USE OF THE SOFTWARE.  
//	
//	Except as expressly set forth in the Authorized License,
//	
//	1.	This program, including its structure, sequence and 
//		organization, constitutes the valuable trade secrets 
//		of Broadcom, and you shall use all reasonable efforts 
//		to protect the confidentiality thereof, and to use 
//		this information only in connection with your use 
//		of Broadcom integrated circuit products.
//	
//	2.	TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE 
//		IS PROVIDED "AS IS" AND WITH ALL FAULTS AND BROADCOM 
//		MAKES NO PROMISES, REPRESENTATIONS OR WARRANTIES, 
//		EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, 
//		WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY 
//		DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, 
//		MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A 
//		PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR 
//		COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR 
//		CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE 
//		RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.  
//
//	3.	TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT 
//		SHALL BROADCOM OR ITS LICENSORS BE LIABLE FOR 
//		(i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR 
//		EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY 
//		WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE 
//		SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE 
//		POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN 
//		EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE 
//		ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE 
//		LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE 
//		OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
//
//***************************************************************************
/**
*
*   @file   common_ecdc.h
*
*   @brief  This file defines structures for call configuration used by both CAPI and CAPI2
*
****************************************************************************/


#ifndef _COMMON_ECDC_
#define _COMMON_ECDC_

/**
	Error correction parameter requested

	- EC: either V42 or MNP protocol
	- AUTO: try EC and if EC fails fallback to Normal or Direct
	- RELIABLE: try EC and if EC fails Disconnect
	- DIRECT: No EC, non-buffered (not supported by our MS)
	- NORMAL: No EC, buffered
	- V42: only V42 protocol is used for Error Correction
	- MNP: only one of the MNP protocols is used for Error Correction
**/
typedef enum
{
	ECORIGREQ_DIRECT = 0,			///<  No Error Correction, DTE-DCE match line rate
	ECORIGREQ_NORMAL = 1,			///< No Error Correction, buffered
	ECORIGREQ_V42_NO_DETECTION = 2,	///< V42 Error Correction, no detection phase
	ECORIGREQ_V42 = 3,				///< V42 Error Correction
	ECORIGREQ_ALTERNATIVE = 4		///< Alternative Error Correction, i.e. MNP
} ECOrigReq_t;						///< Originator Request, see AT+ES

/**
	Error correction parameter fall back value if negotiation is not possible

	- EC: either V42 or MNP protocol
	- AUTO: try EC and if EC fails fallback to Normal or Direct
	- RELIABLE: try EC and if EC fails Disconnect
	- DIRECT: No EC, non-buffered (not supported by our MS)
	- NORMAL: No EC, buffered
	- V42: only V42 protocol is used for Error Correction
	- MNP: only one of the MNP protocols is used for Error Correction
**/
typedef enum
{
	ECORIGFB_AUTO_NORMAL = 0,		///< MO, if EC fails, Normal mode
	ECORIGFB_AUTO_DIRECT = 1,		///< MO, if EC fails, Direct mode
	ECORIGFB_RELIABLE = 2,			///< MO, if EC fails, Disconnect
	ECORIGFB_RELIABLE_V42 = 3,		///< MO, if V42 fails, Disconnect
	ECORIGFB_RELIABLE_MNP = 4		///< MO, if MNP fails, Disconnect
} ECOrigFB_t;						///< Originator Fallback, see AT+ES

/**
	Error correction parameter answer fall back value

	- EC: either V42 or MNP protocol
	- AUTO: try EC and if EC fails fallback to Normal or Direct
	- RELIABLE: try EC and if EC fails Disconnect
	- DIRECT: No EC, non-buffered (not supported by our MS)
	- NORMAL: No EC, buffered
	- V42: only V42 protocol is used for Error Correction
	- MNP: only one of the MNP protocols is used for Error Correction
**/
typedef enum
{
	ECANSFB_DIRECT = 0,				///< MT, Direct
	ECANSFB_NORMAL = 1,				///< MT, Normal
	ECANSFB_AUTO_NORMAL = 2,		///< MT, EC mode, if EC fails, then Normal
	ECANSFB_AUTO_DIRECT = 3,		///< MT, EC mode, if EC fails, then Direct
	ECANSFB_RELIABLE = 4,			///< MT, V42/MNP mode, if EC fails, Disconnect
	ECANSFB_RELIABLE_V42 = 5,		///< MT, V42 mode, if V42 fails, Disconnect
	ECANSFB_RELIABLE_MNP = 6		///< MT, MNP mode, if MNP fails, Disconnect
} ECAnsFB_t;						///< Answer Fallback, see AT+ES

/// Error Correction Mode
typedef enum
{
	ECMODE_UNKNOWN = 0,  			///< Unknown,
	ECMODE_DIRECT = 1,  			///< Direct, No Error Correction, DTE-DCE match line rate
	ECMODE_NORMAL = 2,  			///< Normal, No Error Correction, buffered
	ECMODE_MNP2 = 3,	   			///< MNP2
	ECMODE_MNP3 = 4,				///< MNP3
	ECMODE_V42 = 5		   			///< V42, also called LAPM
} ECMode_t;

///	Data Compression Mode
typedef enum
{
	DCMODE_NONE = 0,				///< No Data Compression
	DCMODE_COMPRESSION_ONLY = 1,	///< Data Compression Only
	DCMODE_DECOMPRESSION_ONLY = 2,	///< Data Decompression Only
	DCMODE_BOTH = 3					///< Compression/Decompression
} DCMode_t;							///< Data Compression Mode (P0), see AT+DS

/// Error correction Parameters
typedef struct
{
	  ECOrigReq_t  ES0;		///< Error correction parameter requested
	  ECOrigFB_t   ES1;		///< Error correction parameter fall back value if negotiation is not possible
	  ECAnsFB_t	   ES2;		///< Error correction parameter answer fall back value
}ECInfo_t;


#endif
