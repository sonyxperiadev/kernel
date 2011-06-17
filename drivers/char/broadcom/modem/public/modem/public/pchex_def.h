//***************************************************************************
//
//	Copyright © 2004-2008 Broadcom Corporation
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
*   @file   pchex_def.h
*
*   @brief  This file defines the extended capi api's related definition to establish 
*			data connection and data path for PS data.
*
****************************************************************************/
/**
********************************************************************************************/
#ifndef _PCHEX_DEF_H_
#define _PCHEX_DEF_H_
#ifdef __cplusplus
extern "C" {
#endif


/// Send/Receive Octet count
typedef struct {
	UInt32				dataSentSize;					///< Data sent size
	UInt32				dataRcvSize;					///< Data Receive Size
} PSDataByteCount_t;

/// UL Data Indication
typedef struct
{
	PCHCid_t 			cid;
	Boolean				bFree;
	UInt16				datalen;
	UInt8*				pData;
}PDP_ULData_Ind_t;


/// Flow control events
typedef enum
{
	PSDATA_FLOW_START,	///< Start flow control
	PSDATA_STOP			///< Stop flow control
} PCHEx_FlowCtrlEvent_t;


/// Flow control event callback function
typedef void		(*PCHEx_FlowCntrl_t)			(UInt8 cid, PCHEx_FlowCtrlEvent_t Event);
typedef void		(*PCHExApi_FlowCntrl_t)			(ClientInfo_t	*clientInfoPtr, UInt8 cid, PCHEx_FlowCtrlEvent_t Event);
typedef void (*PCHExApi_RouteSNDataCb_t) (
	ClientInfo_t			*clientInfoPtr,
	UInt8 				cid, 			
	MS_T_NPDU_PTR		*npdu_ptr, 
	UInt8 				rat);

typedef PCHActivateReason_t (*PCHEx_GetActivateReason_t)	(UInt8 cid);
typedef void (*PCHGPRS_TM_RouteSNDataCb_t) (UInt8 cid,UInt16 dataLen, UInt8 * pData);

/// Parameters to make the protocol config options with CHAP authentication type
typedef struct {
	UInt8* challengeData;	///<  challenge data
	UInt8 challengeLen;		///<  challenge data length
	UInt8 challengeId;		///<  challenge id
	UInt8* rspData;			///<  challenge response data(Final MD5 hash)
	UInt8 rspLen;			///<  challenge response length
	UInt8 rspId;			///<  challenge response id
	UInt8* usrNameData;		///<  user name
	UInt8 usrNameLen;		///<  username length
} PCHEx_ChapAuthType_t;



#ifdef __cplusplus
}
#endif

#endif //_PCHEX_DEF_H_
