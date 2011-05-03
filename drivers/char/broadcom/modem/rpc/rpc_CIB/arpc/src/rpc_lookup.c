/****************************************************************************
*
*     Copyright (c) 2007-2008 Broadcom Corporation
*
*   Unless you and Broadcom execute a separate written software license 
*   agreement governing use of this software, this software is licensed to you 
*   under the terms of the GNU General Public License version 2, available 
*    at http://www.gnu.org/licenses/old-licenses/gpl-2.0.html (the "GPL"). 
*
*   Notwithstanding the above, under no circumstances may you combine this 
*   software in any way with any other Broadcom software provided under a license 
*   other than the GPL, without Broadcom's express prior written consent.
*
****************************************************************************/
/**
*
*   @file   rpc_ser.c
*
*   @brief  This file define the request/respose structure for 
*	serialize/deserialize.
*
****************************************************************************/
#ifndef UNDER_LINUX
#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* for strlen */
#endif
#include "mobcom_types.h"
#include "rpc_global.h"

#include "resultcode.h"
#include "taskmsgs.h"
#include "ipcinterface.h"
#include "ipcproperties.h"

#include "rpc_ipc.h"
#include "xdr_porting_layer.h"
#include "xdr.h"
#include "rpc_api.h"
#include "rpc_internal_api.h"
#include "logapi.h"

static UInt32** tableBase = NULL;

/*
 *	Lookup table indexed by MsgType and organized into two levels
 *	to optimize the memory usage.
	Lookup table entry for MsgType = MSG_PBK_GETALPHA_REQ = 0xf601
	tableBase(+0x00)-->NULL
		|
		|(+0x01)
		------->NULL
		|
		|		...
		|
		|(+0xf6)		   (+0x00)
		|------>tableBase2------->NULL
		|			|(+0x01)
		|			|------------>UInt32
		|			|
		|			|------------>...
		|			|(+0xff)
		|			|------------>NULL
		|
		|(0xff)
		|------>NULL
		

 */


bool_t rpc_build_lookup(const RPC_XdrInfo_t *tbl, UInt16 table_size, UInt16 clientIndex)
{
	bool_t bRetVal = TRUE;
	if(tableBase == NULL)
	{
		tableBase = (UInt32**)capi2_malloc(256*sizeof(UInt32));

		if(tableBase)
		{
			int i;
			for(i=0; i < 256; i++)
				tableBase[i] = 0;
		}
	}

	if(tableBase)
	{
		int i;
		for(i=0; i < (int)(table_size); i++)
		{
			unsigned short id = (unsigned short)tbl[i].msgType;
			unsigned short id1 = ( (unsigned short)(( id & (unsigned short)0xff00 ) >> 8));
			unsigned short id2 = id & ((unsigned short)0x00ff);
			UInt32* tableBase2 = NULL;
			if(tableBase[id1] == 0)
			{
				int j;
				tableBase[id1] = (UInt32*)capi2_malloc(256*sizeof(UInt32));
				tableBase2 = tableBase[id1];
				if(tableBase2)
				{
					for(j=0; j < 256; j++)
						tableBase2[j] = 0xFFFF;
				}
			}

			tableBase2 = tableBase[id1];
			
			if(tableBase2 == 0)
			{
				bRetVal = FALSE;
			}
			else
			{
				UInt32 val = 0;

//				if(tableBase2[id2] != 0xFFFF)
//					_DBG_(RPC_TRACE("xdr_build_lookup duplicate entry = 0x%x, 0x%x, 0x%x\r\n", id, id1, id2));

				val = (UInt32)( (UInt32)((clientIndex << 16)&0xFFFF0000) | (UInt32)(((UInt16)i)&0xFFFF) ); 
				tableBase2[id2] = val;

				//_DBG_(RPC_TRACE("rpc_build_lookup = 0x%x, 0x%x, 0x%x\r\n", id, id1, id2));
			}
		}
	}
	else
	{
		bRetVal = FALSE;
	}
	return bRetVal;
}



Boolean rpc_register_xdr(UInt8 clientIndex, const RPC_XdrInfo_t *tbl, UInt16 table_size)
{
	return rpc_build_lookup(tbl, table_size, clientIndex);
}


UInt32 rpc_basic_fast_lookup(UInt16 dscm)
{
	UInt32 nodeVal = 0;
	unsigned short id = (unsigned short)dscm;
	unsigned short id1 = ((unsigned short)( (id & (unsigned short)(((unsigned short)0xff00))) >> 8));
	unsigned short id2 = id & ((unsigned short)0x00ff);
	UInt32* tableBase2 = NULL;
	
	/* Make sure the table is built up */
	if(!tableBase)
		return 0;

	tableBase2 = tableBase[id1];
	
	if(tableBase2 == NULL)
	{
		/* message not handled in RPC */
		_DBG_(RPC_TRACE("xdr_fast_lookup no RPC entry (tableBase2 is NULL) = 0x%x, 0x%x, 0x%x\r\n", id, id1, id2));
		return 0xFFFF;
	}
	else if(tableBase2[id2] != 0xFFFF)
	{
		nodeVal = tableBase2[id2];
	}
	else
	{
		/* message not handled in RPC */
		_DBG_(RPC_TRACE("xdr_fast_lookup no RPC entry = 0x%x, 0x%x, 0x%x\r\n", id, id1, id2));
		return 0xFFFF;
	}
	return nodeVal;
}

UInt32 rpc_basic_set_val(UInt16 dscm, UInt8 val)
{
	UInt32 nodeVal = 0;
	unsigned short id = (unsigned short)dscm;
	unsigned short id1 = ((unsigned short)( (id & (unsigned short)(((unsigned short)0xff00))) >> 8));
	unsigned short id2 = id & ((unsigned short)0x00ff);
	UInt32* tableBase2 = NULL;
	
	/* Make sure the table is built up */
	if(!tableBase)
		return 0;

	tableBase2 = tableBase[id1];
	
	if(tableBase2 == NULL)
	{
		/* message not handled in RPC */
		_DBG_(RPC_TRACE("rpc_basic_set_val no RPC entry (tableBase2 is NULL) = 0x%x, 0x%x, 0x%x\r\n", id, id1, id2));
		return 0xFFFF;
	}
	else if(tableBase2[id2] != 0xFFFF)
	{
		UInt32 mask = val;
		mask = (mask << 24 );
		tableBase2[id2] |= (mask & 0xFF000000);
		nodeVal = tableBase2[id2];
	}
	else
	{
		/* message not handled in RPC */
		_DBG_(RPC_TRACE("rpc_basic_set_val no RPC entry = 0x%x, 0x%x, 0x%x\r\n", id, id1, id2));
		return 0xFFFF;
	}
	return nodeVal;
}

Boolean rpc_fast_lookup(UInt16 dscm, RPC_InternalXdrInfo_t* outParam)
{
	if(outParam)
	{
		RPC_XdrInfo_t* pXdr;
		UInt32 val = rpc_basic_fast_lookup(dscm);

		memset(outParam, 0, sizeof(RPC_InternalXdrInfo_t));
		if(val != 0xFFFF)
		{
			UInt8 mask = (UInt8)((val >> 24) & 0x000000FF);
			UInt8 clientIndex = (UInt8)((val >> 16) & 0x000000FF);
			UInt16 index = (UInt16)(val & 0xFFFF);

			if(clientIndex == 0)
			{
				pXdr =  RPC_InternalXdr(index);
				outParam->xdrInfo = pXdr;
			}
			else
			{
				XdrClientInfo_t clientInfo;

				Boolean ret = RPC_SYS_LookupXdr(clientIndex, index, &clientInfo);
				if(!ret)
					return FALSE;

				outParam->clientIndex = clientIndex;
				outParam->mainProc = clientInfo.mainProc;
				outParam->xdrInfo = clientInfo.xdrEntry;
				outParam->maxDataBufSize = clientInfo.maxDataBufSize;
				outParam->mask = mask;
			}

			return TRUE;
		}
	}
	return FALSE;
}

Boolean rpc_lookup_set_mask(const UInt16 *msgIds, UInt8 listSize, UInt8 maskValue)
{
	if(msgIds && listSize > 0)
	{
		int j;

		for (j = 0; j < listSize; j++)
		{
			UInt16 dscm = msgIds[j];
			UInt32 val = rpc_basic_fast_lookup(dscm);
			UInt32 val2 = rpc_basic_set_val(dscm, maskValue);
		
			_DBG_(RPC_TRACE("rpc_lookup_set_mask dscm=%x val=%x val2=%x", dscm, val, val2));
		}

		return TRUE;
	}
	return FALSE;
}
