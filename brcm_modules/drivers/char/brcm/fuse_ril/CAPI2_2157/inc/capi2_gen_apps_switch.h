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
#include "capi2_gen_common.h"
#ifndef CAPI2_GEN_MSG_AP_H
#define CAPI2_GEN_MSG_AP_H

#ifdef  UNIT_TEST
#define _DEF(a) AP_ ## a
#else
#define _DEF(a) a
#endif



void _DEF(CAPI2_PMU_IsSIMReady)(UInt32 tid, UInt8 clientID);
void _DEF(CAPI2_PMU_ActivateSIM)(UInt32 tid, UInt8 clientID, Int32 volt);
void _DEF(CAPI2_PMU_DeactivateSIM)(UInt32 tid, UInt8 clientID);
void _DEF(CAPI2_GPIO_Set_High_64Pin)(UInt32 tid, UInt8 clientID, UInt32 gpio_pin);
void _DEF(CAPI2_GPIO_Set_Low_64Pin)(UInt32 tid, UInt8 clientID, UInt32 gpio_pin);
void _DEF(CAPI2_PMU_StartCharging)(UInt32 tid, UInt8 clientID);
void _DEF(CAPI2_PMU_StopCharging)(UInt32 tid, UInt8 clientID);
void _DEF(CAPI2_SMS_IsMeStorageEnabled)(UInt32 tid, UInt8 clientID);
void _DEF(CAPI2_SMS_GetMaxMeCapacity)(UInt32 tid, UInt8 clientID);
void _DEF(CAPI2_SMS_GetNextFreeSlot)(UInt32 tid, UInt8 clientID);
void _DEF(CAPI2_SMS_SetMeSmsStatus)(UInt32 tid, UInt8 clientID, UInt16 slotNumber, SIMSMSMesgStatus_t status);
void _DEF(CAPI2_SMS_GetMeSmsStatus)(UInt32 tid, UInt8 clientID, UInt16 slotNumber);
void _DEF(CAPI2_SMS_StoreSmsToMe)(UInt32 tid, UInt8 clientID, UInt8 *inSms, UInt16 inLength, SIMSMSMesgStatus_t status, UInt16 slotNumber);
void _DEF(CAPI2_SMS_RetrieveSmsFromMe)(UInt32 tid, UInt8 clientID, UInt16 slotNumber);
void _DEF(CAPI2_SMS_RemoveSmsFromMe)(UInt32 tid, UInt8 clientID, UInt16 slotNumber);
void _DEF(CAPI2_PMU_ClientPowerDown)(UInt32 tid, UInt8 clientID);
void _DEF(CAPI2_PMU_GetPowerupCause)(UInt32 tid, UInt8 clientID);
void _DEF(CAPI2_SOCKET_Open)(UInt32 tid, UInt8 clientID, UInt8 domain, UInt8 type, UInt8 protocol);
void _DEF(CAPI2_SOCKET_Bind)(UInt32 tid, UInt8 clientID, Int32 descriptor, sockaddr *addr);
void _DEF(CAPI2_SOCKET_Listen)(UInt32 tid, UInt8 clientID, Int32 descriptor, UInt32 backlog);
void _DEF(CAPI2_SOCKET_Accept)(UInt32 tid, UInt8 clientID, Int32 descriptor);
void _DEF(CAPI2_SOCKET_Connect)(UInt32 tid, UInt8 clientID, Int32 descriptor, sockaddr *name);
void _DEF(CAPI2_SOCKET_GetPeerName)(UInt32 tid, UInt8 clientID, Int32 descriptor);
void _DEF(CAPI2_SOCKET_GetSockName)(UInt32 tid, UInt8 clientID, Int32 descriptor);
void _DEF(CAPI2_SOCKET_SetSockOpt)(UInt32 tid, UInt8 clientID, Int32 descriptor, UInt16 optname, SockOptVal_t *optval);
void _DEF(CAPI2_SOCKET_GetSockOpt)(UInt32 tid, UInt8 clientID, Int32 descriptor, UInt16 optname);
void _DEF(CAPI2_SOCKET_Send)(UInt32 tid, UInt8 clientID, SocketSendReq_t *sockSendReq);
void _DEF(CAPI2_SOCKET_SendTo)(UInt32 tid, UInt8 clientID, SocketSendReq_t *sockSendReq, sockaddr *to);
void _DEF(CAPI2_SOCKET_Recv)(UInt32 tid, UInt8 clientID, SocketRecvReq_t *sockRecvReq);
void _DEF(CAPI2_SOCKET_RecvFrom)(UInt32 tid, UInt8 clientID, SocketRecvReq_t *sockRecvReq, sockaddr *from);
void _DEF(CAPI2_SOCKET_Close)(UInt32 tid, UInt8 clientID, Int32 descriptor);
void _DEF(CAPI2_SOCKET_Shutdown)(UInt32 tid, UInt8 clientID, Int32 descriptor, UInt8 how);
void _DEF(CAPI2_SOCKET_Errno)(UInt32 tid, UInt8 clientID, Int32 descriptor);
void _DEF(CAPI2_SOCKET_SO2LONG)(UInt32 tid, UInt8 clientID, Int32 socket);
void _DEF(CAPI2_SOCKET_GetSocketSendBufferSpace)(UInt32 tid, UInt8 clientID, Int32 bufferSpace);
void _DEF(CAPI2_SOCKET_ParseIPAddr)(UInt32 tid, UInt8 clientID, char_ptr_t ipString);
void _DEF(CAPI2_DC_SetupDataConnection)(UInt32 tid, UInt8 clientID, UInt8 inClientID, UInt8 acctID, DC_ConnectionType_t linkType);
void _DEF(CAPI2_DC_SetupDataConnectionEx)(UInt32 tid, UInt8 clientID, UInt8 inClientID, UInt8 acctID, DC_ConnectionType_t linkType, uchar_ptr_t apnCheck, uchar_ptr_t actDCAcctId);
void _DEF(CAPI2_DC_ShutdownDataConnection)(UInt32 tid, UInt8 clientID, UInt8 inClientID, UInt8 acctID);
void _DEF(CAPI2_DATA_IsAcctIDValid)(UInt32 tid, UInt8 clientID, UInt8 acctID);
void _DEF(CAPI2_DATA_CreateGPRSDataAcct)(UInt32 tid, UInt8 clientID, UInt8 acctID, GPRSContext_t *pGprsSetting);
void _DEF(CAPI2_DATA_CreateCSDDataAcct)(UInt32 tid, UInt8 clientID, UInt8 acctID, CSDContext_t *pCsdSetting);
void _DEF(CAPI2_DATA_DeleteDataAcct)(UInt32 tid, UInt8 clientID, UInt8 acctID);
void _DEF(CAPI2_DATA_SetUsername)(UInt32 tid, UInt8 clientID, UInt8 acctID, uchar_ptr_t username);
void _DEF(CAPI2_DATA_GetUsername)(UInt32 tid, UInt8 clientID, UInt8 acctID);
void _DEF(CAPI2_DATA_SetPassword)(UInt32 tid, UInt8 clientID, UInt8 acctID, uchar_ptr_t password);
void _DEF(CAPI2_DATA_GetPassword)(UInt32 tid, UInt8 clientID, UInt8 acctID);
void _DEF(CAPI2_DATA_SetStaticIPAddr)(UInt32 tid, UInt8 clientID, UInt8 acctID, uchar_ptr_t staticIPAddr);
void _DEF(CAPI2_DATA_GetStaticIPAddr)(UInt32 tid, UInt8 clientID, UInt8 acctID);
void _DEF(CAPI2_DATA_SetPrimaryDnsAddr)(UInt32 tid, UInt8 clientID, UInt8 acctID, uchar_ptr_t priDnsAddr);
void _DEF(CAPI2_DATA_GetPrimaryDnsAddr)(UInt32 tid, UInt8 clientID, UInt8 acctID);
void _DEF(CAPI2_DATA_SetSecondDnsAddr)(UInt32 tid, UInt8 clientID, UInt8 acctID, uchar_ptr_t sndDnsAddr);
void _DEF(CAPI2_DATA_GetSecondDnsAddr)(UInt32 tid, UInt8 clientID, UInt8 acctID);
void _DEF(CAPI2_DATA_SetDataCompression)(UInt32 tid, UInt8 clientID, UInt8 acctID, Boolean dataCompEnable);
void _DEF(CAPI2_DATA_GetDataCompression)(UInt32 tid, UInt8 clientID, UInt8 acctID);
void _DEF(CAPI2_DATA_GetAcctType)(UInt32 tid, UInt8 clientID, UInt8 acctID);
void _DEF(CAPI2_DATA_GetEmptyAcctSlot)(UInt32 tid, UInt8 clientID);
void _DEF(CAPI2_DATA_GetCidFromDataAcctID)(UInt32 tid, UInt8 clientID, UInt8 acctID);
void _DEF(CAPI2_DATA_GetDataAcctIDFromCid)(UInt32 tid, UInt8 clientID, UInt8 contextID);
void _DEF(CAPI2_DATA_GetPrimaryCidFromDataAcctID)(UInt32 tid, UInt8 clientID, UInt8 acctID);
void _DEF(CAPI2_DATA_IsSecondaryDataAcct)(UInt32 tid, UInt8 clientID, UInt8 acctID);
void _DEF(CAPI2_DATA_GetDataSentSize)(UInt32 tid, UInt8 clientID, UInt8 acctID);
void _DEF(CAPI2_DATA_GetDataRcvSize)(UInt32 tid, UInt8 clientID, UInt8 acctID);
void _DEF(CAPI2_DATA_SetGPRSPdpType)(UInt32 tid, UInt8 clientID, UInt8 acctID, uchar_ptr_t pdpType);
void _DEF(CAPI2_DATA_GetGPRSPdpType)(UInt32 tid, UInt8 clientID, UInt8 acctID);
void _DEF(CAPI2_DATA_SetGPRSApn)(UInt32 tid, UInt8 clientID, UInt8 acctID, uchar_ptr_t apn);
void _DEF(CAPI2_DATA_GetGPRSApn)(UInt32 tid, UInt8 clientID, UInt8 acctID);
void _DEF(CAPI2_DATA_SetAuthenMethod)(UInt32 tid, UInt8 clientID, UInt8 acctID, DataAuthenMethod_t authenMethod);
void _DEF(CAPI2_DATA_GetAuthenMethod)(UInt32 tid, UInt8 clientID, UInt8 acctID);
void _DEF(CAPI2_DATA_SetGPRSHeaderCompression)(UInt32 tid, UInt8 clientID, UInt8 acctID, Boolean headerCompEnable);
void _DEF(CAPI2_DATA_GetGPRSHeaderCompression)(UInt32 tid, UInt8 clientID, UInt8 acctID);
void _DEF(CAPI2_DATA_SetGPRSQos)(UInt32 tid, UInt8 clientID, UInt8 acctID, PCHQosProfile_t qos);
void _DEF(CAPI2_DATA_GetGPRSQos)(UInt32 tid, UInt8 clientID, UInt8 acctID);
void _DEF(CAPI2_DATA_SetAcctLock)(UInt32 tid, UInt8 clientID, UInt8 acctID, Boolean acctLock);
void _DEF(CAPI2_DATA_GetAcctLock)(UInt32 tid, UInt8 clientID, UInt8 acctID);
void _DEF(CAPI2_DATA_SetGprsOnly)(UInt32 tid, UInt8 clientID, UInt8 acctID, Boolean gprsOnly);
void _DEF(CAPI2_DATA_GetGprsOnly)(UInt32 tid, UInt8 clientID, UInt8 acctID);
void _DEF(CAPI2_DATA_SetGPRSTft)(UInt32 tid, UInt8 clientID, UInt8 acctID, PCHTrafficFlowTemplate_t *pTft);
void _DEF(CAPI2_DATA_GetGPRSTft)(UInt32 tid, UInt8 clientID, UInt8 acctID);
void _DEF(CAPI2_DATA_SetCSDDialNumber)(UInt32 tid, UInt8 clientID, UInt8 acctID, uchar_ptr_t dialNumber);
void _DEF(CAPI2_DATA_GetCSDDialNumber)(UInt32 tid, UInt8 clientID, UInt8 acctID);
void _DEF(CAPI2_DATA_SetCSDDialType)(UInt32 tid, UInt8 clientID, UInt8 acctID, CSDDialType_t csdDialType);
void _DEF(CAPI2_DATA_GetCSDDialType)(UInt32 tid, UInt8 clientID, UInt8 acctID);
void _DEF(CAPI2_DATA_SetCSDBaudRate)(UInt32 tid, UInt8 clientID, UInt8 acctID, CSDBaudRate_t csdBaudRate);
void _DEF(CAPI2_DATA_GetCSDBaudRate)(UInt32 tid, UInt8 clientID, UInt8 acctID);
void _DEF(CAPI2_DATA_SetCSDSyncType)(UInt32 tid, UInt8 clientID, UInt8 acctID, CSDSyncType_t csdSyncType);
void _DEF(CAPI2_DATA_GetCSDSyncType)(UInt32 tid, UInt8 clientID, UInt8 acctID);
void _DEF(CAPI2_DATA_SetCSDErrorCorrection)(UInt32 tid, UInt8 clientID, UInt8 acctID, Boolean enable);
void _DEF(CAPI2_DATA_GetCSDErrorCorrection)(UInt32 tid, UInt8 clientID, UInt8 acctID);
void _DEF(CAPI2_DATA_SetCSDErrCorrectionType)(UInt32 tid, UInt8 clientID, UInt8 acctID, CSDErrCorrectionType_t errCorrectionType);
void _DEF(CAPI2_DATA_GetCSDErrCorrectionType)(UInt32 tid, UInt8 clientID, UInt8 acctID);
void _DEF(CAPI2_DATA_SetCSDDataCompType)(UInt32 tid, UInt8 clientID, UInt8 acctID, CSDDataCompType_t dataCompType);
void _DEF(CAPI2_DATA_GetCSDDataCompType)(UInt32 tid, UInt8 clientID, UInt8 acctID);
void _DEF(CAPI2_DATA_SetCSDConnElement)(UInt32 tid, UInt8 clientID, UInt8 acctID, CSDConnElement_t connElement);
void _DEF(CAPI2_DATA_GetCSDConnElement)(UInt32 tid, UInt8 clientID, UInt8 acctID);
void _DEF(CAPI2_DATA_UpdateAccountToFileSystem)(UInt32 tid, UInt8 clientID);
void _DEF(CAPI2_resetDataSize)(UInt32 tid, UInt8 clientID, UInt8 cid);
void _DEF(CAPI2_addDataSentSizebyCid)(UInt32 tid, UInt8 clientID, UInt8 cid, UInt32 size);
void _DEF(CAPI2_addDataRcvSizebyCid)(UInt32 tid, UInt8 clientID, UInt8 cid, UInt32 size);
void _DEF(CAPI2_GPIO_ConfigOutput_64Pin)(UInt32 tid, UInt8 clientID, UInt32 pin);
void _DEF(CAPI2_GPS_Control)(UInt32 tid, UInt8 clientID, UInt32 u32Cmnd, UInt32 u32Param0, UInt32 u32Param1, UInt32 u32Param2, UInt32 u32Param3, UInt32 u32Param4);
void _DEF(CAPI2_FFS_Control)(UInt32 tid, UInt8 clientID, UInt32 cmd, UInt32 address, UInt32 offset, UInt32 size);
void _DEF(CAPI2_CP2AP_PedestalMode_Control)(UInt32 tid, UInt8 clientID, UInt32 enable);
void _DEF(CAPI2_RTC_SetTime)(UInt32 tid, UInt8 clientID, RTCTime_t *inTime);
void _DEF(CAPI2_RTC_SetDST)(UInt32 tid, UInt8 clientID, UInt8 inDST);
void _DEF(CAPI2_RTC_SetTimeZone)(UInt32 tid, UInt8 clientID, Int8 inTimezone);
void _DEF(CAPI2_RTC_GetTime)(UInt32 tid, UInt8 clientID, RTCTime_t *time);
void _DEF(CAPI2_RTC_GetTimeZone)(UInt32 tid, UInt8 clientID);
void _DEF(CAPI2_RTC_GetDST)(UInt32 tid, UInt8 clientID);
void _DEF(CAPI2_SMS_GetMeSmsBufferStatus)(UInt32 tid, UInt8 clientID, UInt16 cmd);
void _DEF(CAPI2_SMS_GetRecordNumberOfReplaceSMS)(UInt32 tid, UInt8 clientID, SmsStorage_t storageType, UInt8 tp_pid, uchar_ptr_t oaddress);
void _DEF(CAPI2_FLASH_SaveImage)(UInt32 tid, UInt8 clientID, UInt32 flash_addr, UInt32 length, UInt32 shared_mem_addr);
void _DEF(CAPI2_SIMLOCK_GetStatus)(UInt32 tid, UInt8 clientID, SIMLOCK_SIM_DATA_t *sim_data);
void _DEF(CAPI2_FFS_Read)(UInt32 tid, UInt8 clientID, FFS_ReadReq_t *ffsReadReq);
void _DEF(CAPI2_AT_ProcessCmdToAP)(UInt32 tid, UInt8 clientID, UInt8 channel, uchar_ptr_t cmdStr);
void _DEF(CAPI2_InterTaskMsgToAP)(UInt32 tid, UInt8 clientID, InterTaskMsg_t *inPtrMsg);
#endif
