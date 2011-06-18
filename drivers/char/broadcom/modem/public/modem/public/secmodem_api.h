//
//	Copyright ?2004-2011 Broadcom Corporation
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
*   @file   SECMODEM_API.h
*
*   @brief  This file defines the interface for Security API functions.
*
****************************************************************************/

/**

*   @defgroup   SECAPIGroup   SEC API
*   @ingroup    SECGroup
*
*   @brief      This group defines the interfaces to the modem security API.

*
****************************************************************************/

#ifndef _SECMODEM_API_H_
#define _SECMODEM_API_H_

// Status for SECMODEM_CONFIG_MODEM_RSP
#define SECMODEM_CONFIG_MODEM_STATUS_OK                 0x00
#define SECMODEM_CONFIG_MODEM_STATUS_ERROR              0x01
#define SECMODEM_CONFIG_MODEM_STATUS_PARTIAL_SUCCESS    0x02

// Status for SECMODEM_SIMLOCK_STATUS_IND
#define SECMODEM_SIMLOCK_STATUS_OK                      0x00
#define SECMODEM_SIMLOCK_STATUS_RESTRICTED              0x01

// Status for SECMODEM_XSIM_STATUS_IND
#define SECMODEM_XSIM_STATUS_INACTIVE                   0x00
#define SECMODEM_XSIM_STATUS_ACTIVE                     0x01

typedef struct
{
    CallbackFunc_t *secModemAccessCb;
} SecModemConfigModemReq_t;


//---------------------------------------------------------------------------------------
//							Synchronous Function Prototype
//---------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------
//						ASynchronous Function Prototype
//---------------------------------------------------------------------------------------

//***************************************************************************************
/**
    This function requests to use secure channel to configure modem software.

	@param		clientInfoPtr (in) Pointer to client info
	@param		secmodemAccessCb (in) SEC access callback
	@return		Result_t
	@note
	The SIM task returns the status in the ::MSG_SEC_CONFIG_MODEM_RSP message by calling
	the passed callback function.
**/	
Result_t SecModemApi_ConfigModemReq( ClientInfo_t* clientInfoPtr, 
                                     CallbackFunc_t* secModemAccessCb );

#endif  // _SECMODEM_API_H_
