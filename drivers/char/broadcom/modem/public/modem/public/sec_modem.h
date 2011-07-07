//***************************************************************************
//
//	Copyright (c) 2002-2008 Broadcom Corporation
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
*   @file   sec_modem.h
*
*   @brief  This file contains definitions for Modem Security Driver Cross Process API.
*
*	This file provides the function prototypes for Modem Security AP-CP communication.
*
****************************************************************************/


#ifndef _SEC_MODEM_
#define _SEC_MODEM_

#include "common_defs.h"

#define SEC_MODEM_SECHOST_BUFFER_READY      0x01
#define SEC_MODEM_SECHOST_BUFFER_ERROR      0x02
#define SEC_MODEM_SECMODEM_BUFFER_READY     0x03
#define SEC_MODEM_SECMODEM_BUFFER_ERROR	    0x04
#define SEC_MODEM_MODEM_REQ_FN_REGISTER     0x10


#define SEC_MODEM_UNIT_TEST
#ifdef SEC_MODEM_UNIT_TEST

#define SEC_MODEM_U_T_MODEM_HOST_REQ_OK     0x20
#define SEC_MODEM_U_T_MODEM_HOST_REQ_EH     0x21
#define SEC_MODEM_U_T_MODEM_HOST_REQ_EM     0x22
#define SEC_MODEM_U_T_HOST_MODEM_REQ_OK     0x23

#define SEC_MODEM_U_T_MIN   SEC_MODEM_U_T_MODEM_HOST_REQ_OK
#define SEC_MODEM_U_T_MAX   SEC_MODEM_U_T_HOST_MODEM_REQ_OK

#endif //  GLUE_SEC_UNIT_TEST


/* AP-CP communication functions */
void SEC_HostToModemInd(UInt8 state);
void SEC_ModemToHostInd(UInt8 state);

#endif // #define _SEC_MODEM_

