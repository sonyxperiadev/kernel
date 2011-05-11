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
/****************************************************************************
*																			
*     WARNING!!!! Generated File ( Do NOT Modify !!!! )					
*																			
****************************************************************************/
#ifndef SYS_COMMON_RPC_H
#define SYS_COMMON_RPC_H



#define _DBG_(a) a	//by default logs are enabled

#if defined(UNDER_CE) || defined(WIN32)
#define snprintf _snprintf
#endif


#ifdef WIN32
//Windows
#define SYS_TRACE	printf
extern UInt32 g_dwLogLEVEL;

#else
//Target
extern int Log_DebugPrintf(UInt16 logID, char *fmt, ...);
extern void	Log_DebugOutputString(UInt16 logID, char* dbgString);
extern Boolean Log_IsLoggingEnable(UInt16 logID);

#define SYS_TRACE(...) Log_DebugPrintf(LOGID_MISC, __VA_ARGS__)

#endif

void SYS_GenGetPayloadInfo(void* dataBuf, MsgType_t msgType, void** ppBuf, UInt32* len);




UInt8 SYS_GetClientId(void);
void sysGetXdrStruct(RPC_XdrInfo_t** ptr, UInt16* size);


typedef struct
{
	UInt16 	adc_ch; 	///< ADC channel
	UInt16	adc_trg;	///< ADC sample trigger
}MeasMngrCnfgReq_t;

typedef struct
{
	MeasMgrDbaseResult_t	res;
	UInt16	o_rd_data;	///< output ADC read data
}MeasMngrCnfgRsp_t;

typedef struct
{
	Int32 res;
	UInt8* data;
	UInt32 len;
}MtestOutput_t;


UInt16 HAL_EM_BATTMGR_BattLevelPercent(void);
UInt16 HAL_EM_BATTMGR_BattLevel(void);
Boolean HAL_EM_BATTMGR_USB_ChargerPresent(void);
Boolean HAL_EM_BATTMGR_WALL_ChargerPresent(void);
HAL_EM_BATTMGR_ErrorCode_en_t HAL_EM_BATTMGR_Run_BattMgr(UInt16 inBattVolt);
HAL_EM_BATTMGR_ErrorCode_en_t HAL_EM_BATTMGR_Config_BattMgr(HAL_EM_BATTMGR_Action_ConfigBattmgr_st_t *inBattVolt);
EM_BATTMGR_ChargingStatus_en_t HAL_EM_BATTMGR_GetChargingStatus(void);
HAL_EM_BATTMGR_ErrorCode_en_t HAL_EM_BATTMGR_SetComp(Int16 compValue);
void HAL_EM_BATTMGR_GetBattTemp(HAL_EM_BATTMGR_Action_GetBatteryTemp_st_t* val);
void SYSRPC_MEASMGR_GetDataB_Adc(MeasMngrCnfgReq_t *req, MeasMngrCnfgRsp_t* rsp);
void SYSRPC_HAL_ADC_Ctrl(HAL_ADC_Action_en_t action, HAL_ADC_ReadConfig_st_t *req, HAL_ADC_ReadConfig_st_t *rsp);
HAL_EM_BATTMGR_ErrorCode_en_t SYS_HAL_EM_BATTMGR_RegisterEventCB(HAL_EM_BATTMGR_Events_en_t event, Boolean validCbk);



//***************** < 1 > **********************



#endif
