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
*   @file   capi2_comm.c
*
*   @brief  This file dispatches the CAPI2 request messages.
*
****************************************************************************/
#include "capi2_cmd_resp.h"
#include "capi2_old_phonectrl.h"
#include "xassert.h"

#ifndef  UNIT_TEST 
#include "ostask.h"
#endif

static Boolean isInit = FALSE;


#define MAX_CMD_RESP_ENTRIES 40
static CAPI2_CmdRespInfo_t CAPI2_MapTable[MAX_CMD_RESP_ENTRIES];
static UInt8 endofTable = 0;
static void CAPI2_CMD_RSP_RemoveEntry(int index, Boolean removeEntry, const char* dbgStr);




void CAPI2_CMD_RSP_Dump(void)
{
	CAPI2_CmdRespInfo_t info;
	int i;

	if(isInit)
	{
		for(i=0;i<endofTable;i++)
		{
			info = CAPI2_MapTable[i];
			_DBG_(CAPI2_TRACE("CAPI2_CMD_RSP_Dump tid=%d cid=%d reqId=%d rspId=%d \r\n",
						info.tid,info.clientID,info.reqId,info.rspId));
#ifndef  BYPASS_IPC 
			OSTASK_Sleep(1);
#endif
		}
	}

}

void CAPI2_CMD_RSP_IntCmdRespTable(void)
{
	memset(&CAPI2_MapTable[0],0,sizeof(CAPI2_MapTable));
	isInit = TRUE;
	endofTable = 0;
	_DBG_(CAPI2_TRACE("[CAPI2_CMD_RSP] IntCmdRespTable\r\n"));
}

static int CAPI2_CMD_RSP_FindFreeEntry(void)
{
	if ( endofTable >= MAX_CMD_RESP_ENTRIES )
	{
		//Fixme: Assert
		return -1;
	}	
	endofTable++;
	
	return endofTable - 1;
}

Boolean CAPI2_CMD_RSP_AddCmdRespEntry(CAPI2_CmdRespInfo_t *info)
{
	int index;

//	if(info->rspId == 0 || info->rspId == MSG_INVALID)
	if(info->rspId == 0)
	{
		_DBG_(CAPI2_TRACE("CAPI2_CMD_RSP_AddCmdRespEntry fail 1 tid=%d cid=%d reqId=%d rspId=%d \r\n",
						info->tid,info->clientID,info->reqId,info->rspId));
		return FALSE;
	}


	if(!isInit)
		CAPI2_CMD_RSP_IntCmdRespTable();

	index = CAPI2_CMD_RSP_FindFreeEntry();

	if(index != -1)
	{
	
		CAPI2_MapTable[index] = *info;
		_DBG_(CAPI2_TRACE("[CAPI2_CMD_RSP][Add] AddCmdRespEntry index=%d tid=%d cid=%d reqId=%d rspId=%d \r\n",
						index, info->tid,info->clientID,info->reqId,info->rspId));
		return TRUE;
	}

	_DBG_(CAPI2_TRACE("CAPI2_CMD_RSP_AddCmdRespEntry fail 2 tid=%d cid=%d reqId=%d rspId=%d \r\n",
					info->tid,info->clientID,info->reqId,info->rspId));

	CAPI2_CMD_RSP_Dump();

#ifndef  BYPASS_IPC 
	OSTASK_Sleep(5000);
#endif

	xassert(0, info->tid);
	return FALSE;
}

Boolean CAPI2_CMD_RSP_RemoveCmdRespMapping(MsgType_t rspId)
{
	int i;
	
	if(rspId == 0 || rspId == MSG_INVALID)
		return FALSE;

	if(!isInit)
		CAPI2_CMD_RSP_IntCmdRespTable();

	for(i=0;i<endofTable;i++)
	{
		if( rspId == CAPI2_MapTable[i].rspId)
		{
			CAPI2_CMD_RSP_RemoveEntry(i, TRUE,"RemoveCmdRespMapping");
			return TRUE;
		}
	}

	_DBG_(CAPI2_TRACE("[CAPI2_CMD_RSP] RemoveCmdRespMapping (nm) rspId=%d\r\n",	rspId));
	
	return FALSE;

}


Boolean CAPI2_CMD_RSP_FindCmdRespMapping(MsgType_t rspId, 
										 CAPI2_CmdRespInfo_t *info,
										 Boolean removeEntry)
{
	int i;

	if(rspId == 0 || rspId == MSG_INVALID)
	{
		_DBG_(CAPI2_TRACE("[CAPI2_CMD_RSP] FindCmdRespMapping 1 fail rspId=%d rm=%d\r\n",
						rspId, removeEntry));
		return FALSE;
	}

	if(!isInit)
		CAPI2_CMD_RSP_IntCmdRespTable();


	for(i=0;i<endofTable;i++)
	{
		if(rspId == CAPI2_MapTable[i].rspId)
		{
			*info = CAPI2_MapTable[i];
			CAPI2_CMD_RSP_RemoveEntry(i, removeEntry,"FindCmdRespMapping");
			return TRUE;
		}
	}

	_DBG_(CAPI2_TRACE("[CAPI2_CMD_RSP] FindCmdRespMapping (nm) rspId=%d rm=%d\r\n",
					rspId, removeEntry));

	return FALSE;
}

Boolean CAPI2_CMD_RSP_FindCmdRespMappingbyCmd(MsgType_t reqId, 
										 CAPI2_CmdRespInfo_t *info,
										 Boolean removeEntry)
{
	int i;

	if(reqId == 0 || reqId == MSG_INVALID)
	{
		_DBG_(CAPI2_TRACE("[CAPI2_CMD_RSP] FindCmdRespMappingbyCmd 1 fail reqId=%d rm=%d\r\n",
						reqId, removeEntry));
		return FALSE;
	}

	if(!isInit)
		CAPI2_CMD_RSP_IntCmdRespTable();

	for(i=0;i<endofTable;i++)
	{
		if(reqId == CAPI2_MapTable[i].reqId)
		{
			*info = CAPI2_MapTable[i];
			CAPI2_CMD_RSP_RemoveEntry(i, removeEntry,"FindCmdRespMappingbyCmd");
			return TRUE;
		}
	}

	_DBG_(CAPI2_TRACE("[CAPI2_CMD_RSP] FindCmdRespMappingbyCmd fail 2 reqId=%d rm=%d\r\n",
					reqId, removeEntry));

	return FALSE;
}

Boolean CAPI2_CMD_RSP_FindCmdRespMappingbyCmdRange(MsgType_t StartreqId, 
										MsgType_t EndreqId, 
										 CAPI2_CmdRespInfo_t *info,
										 Boolean removeEntry)
{
	int i;

	if(StartreqId == 0 || StartreqId == MSG_INVALID || EndreqId == 0 || EndreqId == MSG_INVALID)
	{
		_DBG_(CAPI2_TRACE("[CAPI2_CMD_RSP] FindCmdRespMappingbyCmdRange fail 1 reqId=%d endReqid=%d rm=%d\r\n",
						StartreqId, EndreqId, removeEntry));
		return FALSE;
	}

	if(!isInit)
		CAPI2_CMD_RSP_IntCmdRespTable();

	for(i=0;i<endofTable;i++)
	{
		if(StartreqId < CAPI2_MapTable[i].reqId && EndreqId > CAPI2_MapTable[i].reqId)
		{
			*info = CAPI2_MapTable[i];
			CAPI2_CMD_RSP_RemoveEntry(i, removeEntry,"FindCmdRespMappingbyCmdRange");
			return TRUE;
		}
	}
	_DBG_(CAPI2_TRACE("[CAPI2_CMD_RSP] FindCmdRespMappingbyCmdRange fail 2 reqId=%d endReqid=%d rm=%d\r\n",
					StartreqId, EndreqId, removeEntry));
	return FALSE;
}

Boolean CAPI2_CMD_RSP_FindCmdRespMappingbyCallIndex( UInt8 callIndex,
										 CAPI2_CmdRespInfo_t *info,
										 Boolean removeEntry)
{
	int i;

	if(callIndex == INVALID_CALL)
	{
		_DBG_(CAPI2_TRACE("[CAPI2_CMD_RSP] FindCmdRespMappingbyCallIndex 1 fail callIndex=%d rm=%d\r\n",
						callIndex, removeEntry));
		return FALSE;
	}

	if(!isInit)
		CAPI2_CMD_RSP_IntCmdRespTable();

	for(i=0;i<endofTable;i++)
	{
		if(callIndex == CAPI2_MapTable[i].callIndex)
		{
			*info = CAPI2_MapTable[i];
			CAPI2_CMD_RSP_RemoveEntry(i, removeEntry,"FindCmdRespMappingbyCallIndex");
			return TRUE;
		}
	}
	_DBG_(CAPI2_TRACE("[CAPI2_CMD_RSP] FindCmdRespMappingbyCallIndex 2 fail callIndex=%d rm=%d\r\n",
					callIndex, removeEntry));
	return FALSE;
}

Boolean CAPI2_CMD_RSP_FindCmdRespMappingbyCallIndexRespId( UInt8 callIndex,
										 MsgType_t rspId, 
										 CAPI2_CmdRespInfo_t *info,
										 Boolean removeEntry)
{
	int i;

	if(callIndex == INVALID_CALL)
	{
		_DBG_(CAPI2_TRACE("[CAPI2_CMD_RSP] FindCmdRespMappingbyCallIndexRespId 1 fail callIndex=%d rm=%d\r\n",
						callIndex, removeEntry));
		return FALSE;
	}

	if(!isInit)
		CAPI2_CMD_RSP_IntCmdRespTable();

	for(i=0;i<endofTable;i++)
	{
		if((callIndex == CAPI2_MapTable[i].callIndex ) && (rspId == CAPI2_MapTable[i].rspId))
		{
			*info = CAPI2_MapTable[i];
			CAPI2_CMD_RSP_RemoveEntry(i, removeEntry,"FindCmdRespMappingbyCallIndexRespId");
			return TRUE;
		}
	}
	_DBG_(CAPI2_TRACE("[CAPI2_CMD_RSP] FindCmdRespMappingbyCallIndexRespId 2 fail callIndex=%d rm=%d\r\n",
					callIndex, removeEntry));
	return FALSE;
}

#ifdef CHECK_CMD_RESP_DUP
Boolean CAPI2_CMD_RSP_CheckDupCmdRespEntry( MsgType_t rspId)
{
	int i;

	if(!isInit)
		return FALSE;

	for(i=0;i<endofTable;i++)
	{
		if(rspId == CAPI2_MapTable[i].rspId )  
		{
			return TRUE;
		}
	}
	return FALSE;
}

Boolean CAPI2_CC_CMD_RSP_CheckDupCmdRespEntry( MsgType_t rspId, UInt8 callIndex)
{
	int i;

	if(!isInit)
		return FALSE;

	for(i=0;i<endofTable;i++)
	{
		if(	rspId == CAPI2_MapTable[i].rspId &&
			callIndex == CAPI2_MapTable[i].callIndex)  //Fixme : Condition
		{
			return TRUE;
		}
	}
	return FALSE;
}

#endif

void CAPI2_CMD_RSP_RemoveEntry(int index, Boolean removeEntry, const char* dbgStr)
{
	int j;
	CAPI2_CmdRespInfo_t *info;
	
	xassert(index < endofTable, index);
	
	info = &CAPI2_MapTable[index];

	if(!removeEntry)
	{
		_DBG_(CAPI2_TRACE("[CAPI2_CMD_RSP] %s Lookup index=%d tid=%d cid=%d reqId=%d rspId=%d callIndex=%d\r\n",
						dbgStr, index, info->tid,info->clientID,info->reqId,info->rspId,info->callIndex));
		return;
	}

	_DBG_(CAPI2_TRACE("[CAPI2_CMD_RSP][Rm] %s index=%d tid=%d cid=%d reqId=%d rspId=%d callIndex=%d\r\n",
					dbgStr, index, info->tid,info->clientID,info->reqId,info->rspId,info->callIndex));

	for(j=index+1;j<endofTable;j++,index++)
	{
		CAPI2_MapTable[index] = CAPI2_MapTable[j];
	}
	endofTable = index;
}

