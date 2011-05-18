/****************************************************************************
*
*     Copyright (c) 2009 Broadcom Corporation
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

#ifndef _BCM_KRIL_CP_CMD_HANDLER_H
#define _BCM_KRIL_CP_CMD_HANDLER_H

#include "bcm_kril_common.h"
#include "bcm_kril_cmd_handler.h"

#include "mobcom_types.h"
#include "resultcode.h"
#include "common_defs.h"
#include "uelbs_api.h"
#include "capi2_stk_ds.h"
#include "capi2_pch_msg.h"
#include "capi2_gen_msg.h"
#include	"capi2_reqrep.h"
#include	"dc_api.h"
#include	"dataacct_def.h"

#include <capi2_sim_api.h>
#include <capi2_cp_api.h>
#include <capi2_sms_ds.h>
#include <capi2_pch_api.h>

//one of this includes include generates warning - CAPI2 is not yet ported properly
//and hence ignoring the warning for now..
#include <capi2_gen_apps_switch.h>

#define _DEF(a) a

#ifndef CONFIG_BRCM_FUSE_RIL_CIB
void _DEF(CAPI2_PMU_IsSIMReady)(UInt32 tid, UInt8 clientID);
void _DEF(CAPI2_PMU_ActivateSIM)(UInt32 tid, UInt8 clientID, Int32 volt);
#endif
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
#if 0
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
#endif
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
void _DEF(CAPI2_SMS_GetMeSmsBufferStatus)(UInt32 tid, UInt8 clientID, UInt16 cmd);
void _DEF(CAPI2_RTC_GetTime)(UInt32 tid, UInt8 clientID, RTCTime_t *time);
void _DEF(CAPI2_RTC_GetTimeZone)(UInt32 tid, UInt8 clientID);
void _DEF(CAPI2_FLASH_SaveImage)(UInt32 tid, UInt8 clientID, UInt32 flash_addr, UInt32 length, UInt32 shared_mem_addr);
#ifndef CONFIG_BRCM_FUSE_RIL_CIB
void _DEF(CAPI2_FFS_Read)(UInt32 tid, UInt8 clientID, FFS_ReadReq_t *ffsReadReq);
void _DEF(CAPI2_SIMLOCK_GetStatus)(UInt32 tid, UInt8 clientID, SIMLOCK_SIM_DATA_t *sim_data);
#else
void _DEF(CAPI2_SIMLOCK_GetStatus)(UInt32 tid, UInt8 clientID, SIMLOCK_SIM_DATA_t *sim_data, Boolean is_testsim);
#endif
void _DEF(CAPI2_InterTaskMsgToAP)(UInt32 tid, UInt8 clientID, InterTaskMsg_t *inPtrMsg);

void CAPI2_DATA_IsAcctIDValid_RSP(UInt32 tid, UInt8 clientID, Boolean isValid);
void CAPI2_DATA_CreateGPRSDataAcct_RSP(UInt32 tid, UInt8 clientID, UInt8 resultVal);
void CAPI2_DATA_CreateCSDDataAcct_RSP(UInt32 tid, UInt8 clientID, UInt8 resultVal);
void CAPI2_DATA_DeleteDataAcct_RSP(UInt32 tid, UInt8 clientID, UInt8 resultVal);
void CAPI2_DATA_SetUsername_RSP(UInt32 tid, UInt8 clientID, UInt8 resultVal);
void CAPI2_DATA_GetUsername_RSP(UInt32 tid, UInt8 clientID, uchar_ptr_t username);
void CAPI2_DATA_SetPassword_RSP(UInt32 tid, UInt8 clientID, UInt8 resultVal);
void CAPI2_DATA_GetPassword_RSP(UInt32 tid, UInt8 clientID, uchar_ptr_t password);
void CAPI2_DATA_SetStaticIPAddr_RSP(UInt32 tid, UInt8 clientID, UInt8 resultVal);
void CAPI2_DATA_GetStaticIPAddr_RSP(UInt32 tid, UInt8 clientID, uchar_ptr_t staticIPAddr);
void CAPI2_DATA_SetPrimaryDnsAddr_RSP(UInt32 tid, UInt8 clientID, UInt8 resultVal);
void CAPI2_DATA_GetPrimaryDnsAddr_RSP(UInt32 tid, UInt8 clientID, uchar_ptr_t priDnsAddr);
void CAPI2_DATA_SetSecondDnsAddr_RSP(UInt32 tid, UInt8 clientID, UInt8 resultVal);
void CAPI2_DATA_GetSecondDnsAddr_RSP(UInt32 tid, UInt8 clientID, uchar_ptr_t sndDnsAddr);
void CAPI2_DATA_SetDataCompression_RSP(UInt32 tid, UInt8 clientID, UInt8 resultVal);
void CAPI2_DATA_GetDataCompression_RSP(UInt32 tid, UInt8 clientID, Boolean dataCompEnable);
void CAPI2_DATA_GetAcctType_RSP(UInt32 tid, UInt8 clientID, DataAccountType_t dataAcctType);
void CAPI2_DATA_GetEmptyAcctSlot_RSP(UInt32 tid, UInt8 clientID, UInt8 emptySlot);
void CAPI2_DATA_GetCidFromDataAcctID_RSP(UInt32 tid, UInt8 clientID, UInt8 contextID);
void CAPI2_DATA_GetDataAcctIDFromCid_RSP(UInt32 tid, UInt8 clientID, UInt8 acctID);
void CAPI2_DATA_GetPrimaryCidFromDataAcctID_RSP(UInt32 tid, UInt8 clientID, UInt8 priContextID);
void CAPI2_DATA_IsSecondaryDataAcct_RSP(UInt32 tid, UInt8 clientID, Boolean isSndDataAcct);
void CAPI2_DATA_GetDataSentSize_RSP(UInt32 tid, UInt8 clientID, UInt32 dataSentSize);
void CAPI2_DATA_GetDataRcvSize_RSP(UInt32 tid, UInt8 clientID, UInt32 dataRcvSize);
void CAPI2_DATA_SetGPRSPdpType_RSP(UInt32 tid, UInt8 clientID, UInt8 resultVal);
void CAPI2_DATA_GetGPRSPdpType_RSP(UInt32 tid, UInt8 clientID, uchar_ptr_t pdpType);
void CAPI2_DATA_SetGPRSApn_RSP(UInt32 tid, UInt8 clientID, UInt8 resultVal);
void CAPI2_DATA_GetGPRSApn_RSP(UInt32 tid, UInt8 clientID, uchar_ptr_t apn);
void CAPI2_DATA_SetAuthenMethod_RSP(UInt32 tid, UInt8 clientID, UInt8 resultVal);
void CAPI2_DATA_GetAuthenMethod_RSP(UInt32 tid, UInt8 clientID, DataAuthenMethod_t authenMethod);
void CAPI2_DATA_SetGPRSHeaderCompression_RSP(UInt32 tid, UInt8 clientID, UInt8 resultVal);
void CAPI2_DATA_GetGPRSHeaderCompression_RSP(UInt32 tid, UInt8 clientID, Boolean headerCompEnable);
void CAPI2_DATA_SetGPRSQos_RSP(UInt32 tid, UInt8 clientID, UInt8 resultVal);
void CAPI2_DATA_GetGPRSQos_RSP(UInt32 tid, UInt8 clientID, PCHQosProfile_t qos);
void CAPI2_DATA_SetAcctLock_RSP(UInt32 tid, UInt8 clientID, UInt8 resultVal);
void CAPI2_DATA_GetAcctLock_RSP(UInt32 tid, UInt8 clientID, Boolean acctLock);
void CAPI2_DATA_SetGprsOnly_RSP(UInt32 tid, UInt8 clientID, UInt8 resultVal);
void CAPI2_DATA_GetGprsOnly_RSP(UInt32 tid, UInt8 clientID, Boolean gprsOnly);
void CAPI2_DATA_SetGPRSTft_RSP(UInt32 tid, UInt8 clientID, UInt8 resultVal);
#ifndef CONFIG_BRCM_FUSE_RIL_CIB
void CAPI2_DATA_GetGPRSTft_RSP(UInt32 tid, UInt8 clientID, CAPI2_DATA_GetGPRSTft_Result_t rsp);
#endif
void CAPI2_DATA_CheckTft_RSP(UInt32 tid, UInt8 clientID, Boolean tftIsValid);
void CAPI2_DATA_SetCSDDialNumber_RSP(UInt32 tid, UInt8 clientID, UInt8 resultVal);
void CAPI2_DATA_GetCSDDialNumber_RSP(UInt32 tid, UInt8 clientID, uchar_ptr_t dialNumber);
void CAPI2_DATA_SetCSDDialType_RSP(UInt32 tid, UInt8 clientID, UInt8 resultVal);
void CAPI2_DATA_GetCSDDialType_RSP(UInt32 tid, UInt8 clientID, CSDDialType_t csdDialType);
void CAPI2_DATA_SetCSDBaudRate_RSP(UInt32 tid, UInt8 clientID, UInt8 resultVal);
void CAPI2_DATA_GetCSDBaudRate_RSP(UInt32 tid, UInt8 clientID, CSDBaudRate_t csdBaudRate);
void CAPI2_DATA_SetCSDSyncType_RSP(UInt32 tid, UInt8 clientID, UInt8 resultVal);
void CAPI2_DATA_GetCSDSyncType_RSP(UInt32 tid, UInt8 clientID, CSDSyncType_t csdSyncType);
void CAPI2_DATA_SetCSDErrorCorrection_RSP(UInt32 tid, UInt8 clientID, UInt8 resultVal);
void CAPI2_DATA_GetCSDErrorCorrection_RSP(UInt32 tid, UInt8 clientID, Boolean enabled);
void CAPI2_DATA_SetCSDErrCorrectionType_RSP(UInt32 tid, UInt8 clientID, UInt8 resultVal);
void CAPI2_DATA_GetCSDErrCorrectionType_RSP(UInt32 tid, UInt8 clientID, CSDErrCorrectionType_t errCorrectionType);
void CAPI2_DATA_SetCSDDataCompType_RSP(UInt32 tid, UInt8 clientID, UInt8 resultVal);
void CAPI2_DATA_GetCSDDataCompType_RSP(UInt32 tid, UInt8 clientID, CSDDataCompType_t dataCompType);
void CAPI2_DATA_SetCSDConnElement_RSP(UInt32 tid, UInt8 clientID, UInt8 resultVal);
void CAPI2_DATA_GetCSDConnElement_RSP(UInt32 tid, UInt8 clientID, CSDConnElement_t connElement);
void CAPI2_DATA_UpdateAccountToFileSystem_RSP(UInt32 tid, UInt8 clientID, UInt8 resultVal);
void CAPI2_resetDataSize_RSP(UInt32 tid, UInt8 clientID, UInt8 resultVal);
void CAPI2_addDataSentSizebyCid_RSP(UInt32 tid, UInt8 clientID, UInt8 resultVal);
void CAPI2_addDataRcvSizebyCid_RSP(UInt32 tid, UInt8 clientID, UInt8 resultVal);
void CAPI2_DC_SetupDataConnection_RSP(UInt32 tid, UInt8 clientID, UInt8 resultVal);
void CAPI2_DC_SetupDataConnectionEx_RSP(UInt32 tid, UInt8 clientID, UInt8 resultVal, UInt8 actDCAcctId);
void CAPI2_DC_ShutdownDataConnection_RSP(UInt32 tid, UInt8 clientID, UInt8 resultVal);

void CAPI2_DC_ShutdownDataConnection_RSP(UInt32 tid, UInt8 clientID, UInt8 resultVal);
void CAPI2_SOCKET_Open_RSP(UInt32 tid, UInt8 clientID, Int32 descriptor);
void CAPI2_SOCKET_Bind_RSP(UInt32 tid, UInt8 clientID, Int8 status);
void CAPI2_SOCKET_Listen_RSP(UInt32 tid, UInt8 clientID, Int8 status);
void CAPI2_SOCKET_Connect_RSP(UInt32 tid, UInt8 clientID, Int8 status);
void CAPI2_SOCKET_SetSockOpt_RSP(UInt32 tid, UInt8 clientID, Int8 status);
//void CAPI2_SOCKET_GetSockOpt_RSP(UInt32 tid, UInt8 clientID, SockOptVal_t* optval, Int8 status);
void CAPI2_SOCKET_Close_RSP(UInt32 tid, UInt8 clientID, Int8 status);
void CAPI2_SOCKET_Shutdown_RSP(UInt32 tid, UInt8 clientID, Int8 status);
void CAPI2_SOCKET_Errno_RSP(UInt32 tid, UInt8 clientID, Int32 error);
void CAPI2_SOCKET_SO2LONG_RSP(UInt32 tid, UInt8 clientID, Int32 descriptor);
void CAPI2_SOCKET_GetSocketSendBufferSpace_RSP(UInt32 tid, UInt8 clientID, Int32 bufferSpace);
void CAPI2_DATA_SetSecondDnsAddr_RSP(UInt32 tid, UInt8 clientID, UInt8 resultVal);
void CAPI2_RTC_SetTime_RSP(UInt32 tid, UInt8 clientID, Boolean status);
void CAPI2_RTC_SetDST_RSP(UInt32 tid, UInt8 clientID, Boolean status);
void CAPI2_RTC_SetTimeZone_RSP(UInt32 tid, UInt8 clientID, Boolean status);
void CAPI2_SMS_GetMeSmsBufferStatus_RSP(UInt32 tid, UInt8 clientID, UInt32 bfree, UInt32 bused);
void CAPI2_RTC_GetTime_RSP(UInt32 tid, UInt8 clientID, RTCTime_t time);
void CAPI2_RTC_GetTimeZone_RSP(UInt32 tid, UInt8 clientID, Int8 timeZone);
void CAPI2_FLASH_SaveImage_RSP(UInt32 tid, UInt8 clientID, Boolean status);
void CAPI2_SIMLOCK_GetStatus_RSP(UInt32 tid, UInt8 clientID, SIMLOCK_STATE_t simlock_state);
#ifndef CONFIG_BRCM_FUSE_RIL_CIB
void CAPI2_FFS_Read_RSP(UInt32 tid, UInt8 clientID, FFS_Data_t *ffsReadRsp);
#endif
void CAPI2_RTC_GetDST_RSP(UInt32 tid, UInt8 clientID, UInt8 dst);


extern void bcm_cp_cmd_handler_init(void);


/// The GPS hardware abstraction layer function call returns the following operation status
typedef enum
{
    HAL_GPS_PASS = 0, ///< Successful
    HAL_GPS_FAIL,    ///< Generic failure code
} HAL_GPS_Result_en_t;


// The FFS_Control() function call returns the following operation status
typedef enum
{
    FFS_CONTROL_PASS = 0,             ///< Successful
    FFS_CONTROL_INVALID_PARAM,        ///< invalid parameters
    FFS_CONTROL_UNKNOWN_CMD,            ///< unknown command
    FFS_CONTROL_NO_FFS_SUPPORT_ON_AP,   ///< no FFS support on AP
    FFS_CONTROL_FILE_OP_FAIL,         ///< one of the file operation failed
    FFS_CONTROL_IPCAP_PDS_OP_FAIL,      ///< IPCAP_PersistentDataStore operation failed

} FFS_Control_Result_en_t;


/// various commands for FFS_Control()
typedef enum
{
    FFS_CONTROL_COPY_NVS_FILE_TO_SM = 0,    ///< copy the NVS file to shared memory
    FFS_CONTROL_COPY_SM_TO_NVS_FILE,        ///< copy shared memory to the NVS file

} FFS_Control_en_t;


// for FFS Control
typedef struct{
    unsigned long   tid;
    unsigned short  clientID;
    unsigned long   cmd; 
    unsigned long   address;
    unsigned long   offset;
    unsigned long   size;
}KrilFFSControlCmd_t;

#ifndef CONFIG_BRCM_FUSE_RIL_CIB
typedef enum {
    PMU_SIM3P0Volt = 0,
    PMU_SIM2P5Volt,
    PMU_SIM0P0Volt,
    PMU_SIM1P8Volt
} PMU_SIMVolt_t;
#endif

#endif //_BCM_KRIL_CP_CMD_HANDLER_H
