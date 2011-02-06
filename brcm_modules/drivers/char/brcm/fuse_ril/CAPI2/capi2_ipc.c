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
*   @file   capi2_ipc.c
*
*   @brief  This file interface with IPC interface
*
****************************************************************************/
#include "capi2_reqrep.h"
#include "capi2_taskmsgs.h"
#include "capi2_ds_api.h"
#include "capi2_ipc_config.h"
#include "capi2_ipc.h"

#undef BYPASS_IPC
#ifndef  BYPASS_IPC
//******************************************************************************
//	 			Global defines
//******************************************************************************




/**
	CAPI2 IPC Data structure. Array of Interfaces.
**/
typedef struct
{
	Boolean isInit;					///< Is Enpoint initialized
	IPC_BufferPool  ipc_buf_pool[MAX_CHANNELS]; ///< Each Enpoint can have array of buffer pools per channel
	UInt8			ipc_buf_id[MAX_CHANNELS]; ///< Channel identifier for buffer pool ( cid or call index )
	CAPI2_PACKET_DataIndCallBackFunc_t	*pktIndCb;	///< Callback for interface buffer delivery
	CAPI2_FlowControlCallbackFunc_t		*flowControlCb; ///< Flow control callback
}CAPI2_IPCInfo_t;

typedef struct
{
	UInt32 pkt_size[MAX_CHANNELS];
	UInt32 max_pkts[MAX_CHANNELS];
	UInt32 start_threshold;
	UInt32 end_threshold;
	IPC_EndpointId_T	srcEpId;
	IPC_EndpointId_T	destEpId;
}CAPI2_IPCBufInfo_t;

static CAPI2_IPCInfo_t		ipcInfoList[INTERFACE_TOTAL];

static CAPI2_IPCBufInfo_t  ipcBufList[INTERFACE_TOTAL]={
 {{CFG_CAPI2_CMD_PKT_SIZE, CFG_CAPI2_CMD_PKT_SIZE2, CFG_CAPI2_CMD_PKT_SIZE3, 0, 0, 0, 0,0,0,0}, {CFG_CAPI2_CMD_MAX_PACKETS, CFG_CAPI2_CMD_MAX_PACKETS2, CFG_CAPI2_CMD_MAX_PACKETS3, 0, 0, 0,0,0,0,0}, CFG_CAPI2_CMD_START_THRESHOLD, CFG_CAPI2_CMD_END_THRESHOLD,0,0},
 {{CFG_CAPI2_PKTDATA_PKT_SIZE, CFG_CAPI2_PKTDATA_PKT_SIZE, CFG_CAPI2_PKTDATA_PKT_SIZE, CFG_CAPI2_PKTDATA_PKT_SIZE, CFG_CAPI2_PKTDATA_PKT_SIZE, CFG_CAPI2_PKTDATA_PKT_SIZE, CFG_CAPI2_PKTDATA_PKT_SIZE, CFG_CAPI2_PKTDATA_PKT_SIZE, CFG_CAPI2_PKTDATA_PKT_SIZE, CFG_CAPI2_PKTDATA_PKT_SIZE}, {CFG_CAPI2_PKTDATA_MAX_PACKETS, CFG_CAPI2_PKTDATA_MAX_PACKETS, CFG_CAPI2_PKTDATA_MAX_PACKETS, CFG_CAPI2_PKTDATA_MAX_PACKETS, CFG_CAPI2_PKTDATA_MAX_PACKETS, CFG_CAPI2_PKTDATA_MAX_PACKETS, CFG_CAPI2_PKTDATA_MAX_PACKETS, CFG_CAPI2_PKTDATA_MAX_PACKETS, CFG_CAPI2_PKTDATA_MAX_PACKETS, CFG_CAPI2_PKTDATA_MAX_PACKETS}, CFG_CAPI2_PKT_START_THRESHOLD, CFG_CAPI2_PKT_END_THRESHOLD,0,0},
 {{CFG_CAPI2_CSDDATA_PKT_SIZE, CFG_CAPI2_CSDDATA_PKT_SIZE, CFG_CAPI2_CSDDATA_PKT_SIZE, CFG_CAPI2_CSDDATA_PKT_SIZE, CFG_CAPI2_CSDDATA_PKT_SIZE, CFG_CAPI2_CSDDATA_PKT_SIZE, CFG_CAPI2_CSDDATA_PKT_SIZE, CFG_CAPI2_CSDDATA_PKT_SIZE, CFG_CAPI2_CSDDATA_PKT_SIZE, CFG_CAPI2_CSDDATA_PKT_SIZE}, {CFG_CAPI2_CSDDATA_MAX_PACKETS, CFG_CAPI2_CSDDATA_MAX_PACKETS, CFG_CAPI2_CSDDATA_MAX_PACKETS, CFG_CAPI2_CSDDATA_MAX_PACKETS, CFG_CAPI2_CSDDATA_MAX_PACKETS, CFG_CAPI2_CSDDATA_MAX_PACKETS, CFG_CAPI2_CSDDATA_MAX_PACKETS, CFG_CAPI2_CSDDATA_MAX_PACKETS, CFG_CAPI2_CSDDATA_MAX_PACKETS, CFG_CAPI2_CSDDATA_MAX_PACKETS}, CFG_CAPI2_CSD_START_THRESHOLD, CFG_CAPI2_CSD_END_THRESHOLD,0,0},
 {{CFG_CAPI2_LOG_PKT_SIZE, CFG_CAPI2_LOG_PKT_SIZE, CFG_CAPI2_LOG_PKT_SIZE, CFG_CAPI2_LOG_PKT_SIZE, CFG_CAPI2_LOG_PKT_SIZE, CFG_CAPI2_LOG_PKT_SIZE, CFG_CAPI2_LOG_PKT_SIZE, CFG_CAPI2_LOG_PKT_SIZE, CFG_CAPI2_LOG_PKT_SIZE, CFG_CAPI2_LOG_PKT_SIZE}, {CFG_CAPI2_LOG_MAX_PACKETS, CFG_CAPI2_LOG_MAX_PACKETS, CFG_CAPI2_LOG_MAX_PACKETS, CFG_CAPI2_LOG_MAX_PACKETS, CFG_CAPI2_LOG_MAX_PACKETS, CFG_CAPI2_LOG_MAX_PACKETS, CFG_CAPI2_LOG_MAX_PACKETS, CFG_CAPI2_LOG_MAX_PACKETS, CFG_CAPI2_LOG_MAX_PACKETS, CFG_CAPI2_LOG_MAX_PACKETS}, CFG_CAPI2_LOG_START_THRESHOLD, CFG_CAPI2_LOG_END_THRESHOLD,0,0}
};


Capi2ProcessorType_t gCapi2Type;

//static function prototypes
static void CAPI2_CreateBufferPool(PACKET_InterfaceType_t type, int channel_index);
static Int8 capi2GetPoolIndex(PACKET_InterfaceType_t interfaceType, IPC_BufferPool Pool);
static Int8 GetInterfaceType(IPC_EndpointId_T epId);
static void CAPI2_FlowCntrl(IPC_BufferPool Pool, IPC_FlowCtrlEvent_T Event);
static void CAPI2_BufferDelivery(IPC_Buffer bufHandle);


//******************************************************************************
//	 			Packet Data API Implementation
//******************************************************************************

Result_t CAPI2_PACKET_RegisterDataInd (UInt8 clientID,
										PACKET_InterfaceType_t interfaceType,
										CAPI2_PACKET_DataIndCallBackFunc_t dataIndFunc,
										CAPI2_FlowControlCallbackFunc_t	flowControlCb)
{
	if (clientID) {} //fixes compiler warnings

	if(	dataIndFunc != NULL && interfaceType < INTERFACE_TOTAL)
	{
		int i = 0;

		ipcInfoList[interfaceType].isInit = TRUE;
		ipcInfoList[interfaceType].flowControlCb = flowControlCb;
		ipcInfoList[interfaceType].pktIndCb = dataIndFunc;

		for(i = 0; i < MAX_CHANNELS; i++)
		{
			ipcInfoList[interfaceType].ipc_buf_pool[i] = 0;
			ipcInfoList[interfaceType].ipc_buf_id[i] = i+1;
		}

		return RESULT_OK;
	}
	return RESULT_ERROR;
}

Result_t CAPI2_PACKET_SendData(UInt8 clientID, PACKET_InterfaceType_t interfaceType, UInt8 channel, PACKET_BufHandle_t dataBufHandle)
{
	IPC_ReturnCode_T ipcError;

	UInt8* pCid = (UInt8*)IPC_BufferHeaderSizeSet((IPC_Buffer)dataBufHandle , 4);
	pCid[0] = channel;

	if (clientID) {} //fixes compiler warnings
	
	ipcError = IPC_SendBuffer((IPC_Buffer)dataBufHandle, IPC_PRIORITY_DEFAULT);

	return (ipcError == IPC_OK) ? RESULT_OK : RESULT_ERROR;
}

//******************************************************************************
//	 			CAPI2 PKT Buffer Management
//******************************************************************************




static void CAPI2_CreateBufferPool(PACKET_InterfaceType_t type, int channel_index)
{
	UInt32 val = 0;
	

	ipcInfoList[type].isInit = TRUE;
	ipcInfoList[type].ipc_buf_pool[channel_index] = IPC_CreateBufferPool(ipcBufList[type].srcEpId, 
																	  ipcBufList[type].destEpId, 
																	  ipcBufList[type].max_pkts[channel_index], 
																	  ipcBufList[type].pkt_size[channel_index], 
																	  ipcBufList[type].start_threshold,
																	  ipcBufList[type].end_threshold);
	val = ( (channel_index << 16 ) | (type & 0xFFFF) );

	_DBG_(CAPI2_TRACE("CAPI2_CreateBufferPool type:%d, index:%d, pool:%x\r\n", type, channel_index, ipcInfoList[type].ipc_buf_pool[channel_index]));
	
	xassert(ipcInfoList[type].ipc_buf_pool[channel_index] != NULL, val);
	
	ipcInfoList[type].ipc_buf_id[channel_index] = channel_index+1;

}

PACKET_BufHandle_t CAPI2_PACKET_AllocateBuffer(PACKET_InterfaceType_t interfaceType, UInt32 requiredSize, UInt8 channel)
{
	int index = -1;
	IPC_Buffer bufHandle = 0;

	if( interfaceType == INTERFACE_PACKET)
	{
		if(channel == 0 || channel > MAX_CHANNELS )
			return NULL;

		index = 0;//All channels use the same buffer pool
	}
	else
	{
		for(index=0; index < MAX_CHANNELS; index++)
		{
			if(ipcBufList[interfaceType].pkt_size[index] >= requiredSize)
				break;
		}
		if(index >= MAX_CHANNELS)
			return NULL;
	}

	if(ipcInfoList[interfaceType].ipc_buf_pool[index] == 0)
		CAPI2_CreateBufferPool(interfaceType, index);
	
	if(ipcInfoList[interfaceType].ipc_buf_pool[index])
	{
		// Allocate with wait for CAPI2 interface(Packet data, CSD has flow control enabled) && for CP side && for big buffers
		if(interfaceType == INTERFACE_CAPI2)
		{
			bufHandle = IPC_AllocateBufferWait(ipcInfoList[interfaceType].ipc_buf_pool[index], 100000);// 100 seconds wait enough?
		}
		else if(interfaceType == INTERFACE_PACKET )
		{
			if(DETAIL_DATA_LOG_ENABLED)
			{
				_DBG_(CAPI2_TRACE_DATA_DETAIL("CAPI2_PACKET_AllocateBuffer PKT BEFORE %d\r\n",requiredSize));
			}
			bufHandle = IPC_AllocateBufferWait(ipcInfoList[interfaceType].ipc_buf_pool[index], IPC_WAIT_FOREVER);
			if(DETAIL_DATA_LOG_ENABLED)
			{
				_DBG_(CAPI2_TRACE_DATA_DETAIL("CAPI2_PACKET_AllocateBuffer PKT AFTER %d\r\n",requiredSize));
			}
		}
		else
		{
			bufHandle = IPC_AllocateBuffer(ipcInfoList[interfaceType].ipc_buf_pool[index]);
		}
	}

	if(0 == bufHandle)
	{
		_DBG_(CAPI2_TRACE("CAPI2_PACKET_AllocateBuffer failed %d, %d, %d\r\n", interfaceType, requiredSize, index));
	}
	if(bufHandle)
		IPC_BufferSetDataSize(bufHandle, requiredSize);

	return (PACKET_BufHandle_t)bufHandle;
}


void* CAPI2_PACKET_GetBufferData(PACKET_BufHandle_t dataBufHandle)
{
	return IPC_BufferDataPointer((IPC_Buffer)dataBufHandle);
}

UInt32 CAPI2_PACKET_GetBufferLength(PACKET_BufHandle_t dataBufHandle)
{
	return IPC_BufferDataSize((IPC_Buffer)dataBufHandle);
}

void CAPI2_PACKET_SetBufferLength(PACKET_BufHandle_t dataBufHandle, UInt32 bufferSize)
{
	IPC_BufferSetDataSize((IPC_Buffer)dataBufHandle, bufferSize);
}


Result_t CAPI2_PACKET_FreeBuffer(PACKET_BufHandle_t dataBufHandle)
{
	IPC_FreeBuffer((IPC_Buffer)dataBufHandle);
	return RESULT_OK;
}


//******************************************************************************
//	 			CAPI2 Callback Implementation
//******************************************************************************

static Int8 capi2GetPoolIndex(PACKET_InterfaceType_t interfaceType, IPC_BufferPool Pool)
{
	int i;


	for(i = 0; i < MAX_CHANNELS; i++)
	{
		if(ipcInfoList[interfaceType].ipc_buf_pool[i] == Pool)
			return i;

	}
	return -1;
}

UInt32 CAPI2_GetMaxPktSize(PACKET_InterfaceType_t interfaceType, UInt32 size)
{
	int index = -1;
	for(index=0; index < MAX_CHANNELS; index++)
	{
		if(ipcBufList[interfaceType].pkt_size[index] >= size)
			return ipcBufList[interfaceType].pkt_size[index];
	}
	return 0;
}

static Int8 GetInterfaceType(IPC_EndpointId_T epId)
{
	if(epId == IPC_EP_Capi2App || epId == IPC_EP_Capi2Cp)
		return INTERFACE_CAPI2;
	
	else if(epId == IPC_EP_Capi2CpData || epId == IPC_EP_Capi2AppData)
		return INTERFACE_PACKET;
	
	else if(epId == IPC_EP_Capi2CpCSDData || epId == IPC_EP_Capi2AppCSDData)
		return INTERFACE_CSD;
	
	else if(epId == IPC_EP_LogApps || epId == IPC_EP_LogCp)
		return INTERFACE_LOGGING;

	//Add assertion here
	return -1;
}

static void CAPI2_FlowCntrl(IPC_BufferPool Pool, IPC_FlowCtrlEvent_T Event)
{
	IPC_EndpointId_T epId = IPC_PoolSourceEndpointId(Pool);
	Int8 type = GetInterfaceType(epId);
	Int8 pool_index;
    
    if(type != -1)
    {
        pool_index = capi2GetPoolIndex((PACKET_InterfaceType_t)type, Pool);

	    if(ipcInfoList[type].flowControlCb != NULL)
		    ipcInfoList[type].flowControlCb((Event == IPC_FLOW_START)?CAPI2_FLOW_START:CAPI2_FLOW_STOP, 
                                            (pool_index >= 0)?(pool_index+1) : 0);
    }

}

static void CAPI2_BufferDelivery(IPC_Buffer bufHandle)
{
	Result_t result = RESULT_ERROR;
	UInt8* pCid = (UInt8*)IPC_BufferHeaderPointer(bufHandle);
	IPC_EndpointId_T destId = IPC_BufferDestinationEndpointId(bufHandle);
	Int8 type = GetInterfaceType(destId);

	if(type != -1 && ipcInfoList[type].pktIndCb != NULL)
		result = ipcInfoList[type].pktIndCb((PACKET_InterfaceType_t)type, (UInt8)pCid[0], (PACKET_BufHandle_t)bufHandle);

	if(result != RESULT_PENDING)
		IPC_FreeBuffer(bufHandle);
}

//******************************************************************************
//	 			CAPI2 CMD Callback Implementation
//******************************************************************************

IPC_BufferPool CAPI2_InternalGetCmdPoolHandle()
{
	return 	ipcInfoList[INTERFACE_CAPI2].ipc_buf_pool[0];
}

//******************************************************************************
//	 			CAPI2 Property Implementation
//******************************************************************************
Boolean CAPI2_SetProperty(CAPI2_PropType_t type, UInt32 value)
{
	Boolean ret = FALSE;
	
	if (gCapi2Type == CAPI2_APPS)
	{
		if(type > CAPI2_PROP_START_AP && type < CAPI2_PROP_END_AP)
		{
			ret = IPC_SetProperty(type, value);
		}
	}
	else
	{
		if(type > CAPI2_PROP_START_CP && type < CAPI2_PROP_END_CP)
		{
			ret = IPC_SetProperty(type, value);
		}

	}

	return ret;
}

Boolean CAPI2_GetProperty(CAPI2_PropType_t type, UInt32 *value)
{
	//Call IPC function when ready
	return IPC_GetProperty(type, (IPC_U32 *)value);
}

//Set backlight on/off in the IPC
void CAPI2_SetBacklight(UInt32 value)
{
	CAPI2_SetProperty(CAPI2_PROP_BACKLIGHT_LEVEL, value);
}

//Get backlight on/off status from the IPC
void CAPI2_GetBacklight(UInt32 *value)
{
	CAPI2_GetProperty(CAPI2_PROP_BACKLIGHT_LEVEL, value);
}

//******************************************************************************
//	 			CAPI2 IPC End Point Init
//******************************************************************************

Result_t CAPI2_IPC_EndPointInit(Capi2ProcessorType_t capi2Type)
{
	memset(ipcInfoList, 0, sizeof(ipcInfoList));

	gCapi2Type = capi2Type;

	if (capi2Type == CAPI2_COMMS)
	{
		IPC_EndpointRegister(IPC_EP_Capi2Cp,		CAPI2_FlowCntrl,	CAPI2_BufferDelivery,4);
		IPC_EndpointRegister(IPC_EP_Capi2CpData,	CAPI2_FlowCntrl,	CAPI2_BufferDelivery,4);
		IPC_EndpointRegister(IPC_EP_Capi2CpCSDData,	CAPI2_FlowCntrl,	CAPI2_BufferDelivery,4);
	}
	else
	{
		IPC_EndpointRegister(IPC_EP_Capi2App,			CAPI2_FlowCntrl,	CAPI2_BufferDelivery, 4);
#ifndef UNDER_LINUX
		IPC_EndpointRegister(IPC_EP_Capi2AppData,		CAPI2_FlowCntrl,	CAPI2_BufferDelivery,4);
		IPC_EndpointRegister(IPC_EP_Capi2AppCSDData,	CAPI2_FlowCntrl,	CAPI2_BufferDelivery,4);
#ifndef UNDER_CE //modify for WinMo UDP log
		IPC_EndpointRegister(IPC_EP_LogApps,			CAPI2_FlowCntrl,	CAPI2_BufferDelivery,4);
#endif
#endif
	}


	return RESULT_OK;
}

//******************************************************************************
//	 			CAPI2 IPC Buffer Pool Init
//******************************************************************************
Result_t CAPI2_IPC_Init(Capi2ProcessorType_t capi2Type)
{
	//Setup endpoints
	if (capi2Type == CAPI2_COMMS)
	{

		ipcBufList[INTERFACE_CAPI2].srcEpId = IPC_EP_Capi2Cp;
		ipcBufList[INTERFACE_CAPI2].destEpId = IPC_EP_Capi2App;

		ipcBufList[INTERFACE_PACKET].srcEpId = IPC_EP_Capi2CpData;
		ipcBufList[INTERFACE_PACKET].destEpId = IPC_EP_Capi2AppData;

		ipcBufList[INTERFACE_CSD].srcEpId = IPC_EP_Capi2CpCSDData;
		ipcBufList[INTERFACE_CSD].destEpId = IPC_EP_Capi2AppCSDData;

		ipcBufList[INTERFACE_LOGGING].srcEpId = IPC_EP_LogCp;
		ipcBufList[INTERFACE_LOGGING].destEpId = IPC_EP_LogApps;

	}
	else
	{
		ipcBufList[INTERFACE_CAPI2].srcEpId = IPC_EP_Capi2App;
		ipcBufList[INTERFACE_CAPI2].destEpId = IPC_EP_Capi2Cp;
		#ifndef UNDER_LINUX
		ipcBufList[INTERFACE_PACKET].srcEpId = IPC_EP_Capi2AppData;
		ipcBufList[INTERFACE_PACKET].destEpId = IPC_EP_Capi2CpData;

		ipcBufList[INTERFACE_CSD].srcEpId = IPC_EP_Capi2AppCSDData;
		ipcBufList[INTERFACE_CSD].destEpId = IPC_EP_Capi2CpCSDData;
		#ifndef UNDER_CE //modify for UDP log
		ipcBufList[INTERFACE_LOGGING].srcEpId = IPC_EP_LogApps;
		ipcBufList[INTERFACE_LOGGING].destEpId = IPC_EP_LogCp;
		#endif
		#endif
	}

	return RESULT_OK;
}


#endif

