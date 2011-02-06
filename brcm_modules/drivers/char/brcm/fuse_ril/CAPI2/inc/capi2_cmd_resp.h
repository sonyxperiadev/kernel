/****************************************************************************
*
*     Copyright (c) 2004 Broadcom Corporation
*           All Rights Reserved
*
*     No portions of this material may be reproduced in any form without the
*     written permission of:
*
*           Broadcom Corporation
*           16215 Alton Parkway
*           P.O. Box 57013
*           Irvine, California 92619-7013
*
*     All information contained in this document is Broadcom Corporation
*     company private, proprietary, and trade secret.
*
****************************************************************************/
/**
*
*   @file   capi2_cmd_resp.h
*
*   @brief  This file dispatches the CAPI2 request messages.
*
****************************************************************************/
#ifndef _CAPI2_CMD_RESP
#define _CAPI2_CMD_RESP

#include "capi2_reqrep.h"
#include "capi2_taskmsgs.h"

#define CHECK_CMD_RESP_DUP

typedef struct
{
	UInt32 tid;
	UInt8 clientID;
	MsgType_t reqId;
	MsgType_t rspId;
	UInt8 callIndex;
	UInt8 CPClientID;
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

#ifdef CHECK_CMD_RESP_DUP
Boolean CAPI2_CMD_RSP_CheckDupCmdRespEntry( MsgType_t rspId);
Boolean CAPI2_CC_CMD_RSP_CheckDupCmdRespEntry( MsgType_t rspId, UInt8 callIndex);
#endif

//CP Client manage functions
UInt8 Get_CPClientID(UInt8 APClientID);
void CAPI2_CSD_RegisterClient(UInt8 ClientID);;
UInt8 Get_APClientID(UInt8 CPClientID);

#endif
