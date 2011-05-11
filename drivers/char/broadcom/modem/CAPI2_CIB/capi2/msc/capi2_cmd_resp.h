//*********************************************************************
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
*   @file   capi2_cmd_resp.h
*
*   @brief  This file dispatches the CAPI2 request messages.
*
****************************************************************************/
#ifndef _CAPI2_CMD_RESP
#define _CAPI2_CMD_RESP


typedef struct
{
	UInt32 tid;
	UInt8 clientID;
	MsgType_t reqId;
	MsgType_t rspId;
	UInt8 callIndex;
	SimNumber_t	simId;
}CAPI2_CmdRespInfo_t;

void CAPI2_CMD_RSP_IntCmdRespTable(void);

Boolean CAPI2_CMD_RSP_AddCmdRespEntry(CAPI2_CmdRespInfo_t *info);

Boolean CAPI2_CMD_RSP_FindCmdRespMapping(MsgType_t rspId, 
										 CAPI2_CmdRespInfo_t *info,
										 Boolean removeEntry);

Boolean CAPI2_CMD_RSP_RemoveCmdRespMapping(MsgType_t rspId);
Boolean CAPI2_CMD_RSP_FindCmdRespMappingbyCmd(MsgType_t reqId, CAPI2_CmdRespInfo_t *info, Boolean removeEntry);
Boolean CAPI2_CMD_RSP_FindCmdRespMappingbyCallIndex(UInt8 callIndex, CAPI2_CmdRespInfo_t *info, Boolean removeEntry);
Boolean CAPI2_CMD_RSP_FindCmdRespMappingbyCallIndexRespId(UInt8 callIndex, MsgType_t rspId, CAPI2_CmdRespInfo_t *info, Boolean removeEntry);

Boolean CAPI2_CMD_RSP_FindCmdRespMappingbyCmdRange(MsgType_t StartreqId, MsgType_t EndreqId, CAPI2_CmdRespInfo_t *info, Boolean removeEntry);

Boolean CAPI2_CMD_RSP_CheckDupCmdRespEntry( MsgType_t rspId, SimNumber_t simId);
Boolean CAPI2_CC_CMD_RSP_CheckDupCmdRespEntry( MsgType_t rspId, UInt8 callIndex, SimNumber_t simId);

UInt32 CAPI2_CMD_RSP_GetCmdRespMappingbyCallIndexRespId( UInt8 callIndex, MsgType_t rspId, SimNumber_t simId);
void CAPI2_CSD_RegisterClient(UInt8 ClientID);;

#endif
