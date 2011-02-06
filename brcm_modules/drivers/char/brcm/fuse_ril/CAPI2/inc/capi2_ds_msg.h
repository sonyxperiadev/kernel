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
*   @file   capi2_ds_msg.h
*
*   @brief  This file defines the capi2 message structure and forward declares
*	the serialization/deserialization functions.
*
****************************************************************************/
#ifndef CAPI2_DS_MSG_H
#define CAPI2_DS_MSG_H


#include "xdr.h"
#include "capi2_reqrep.h"
#include "capi2_dataacct.h"
#include "capi2_dataCommInt.h"

typedef char* char_ptr_t;
typedef unsigned char* uchar_ptr_t;
bool_t xdr_uchar_ptr_t(XDR *xdrs, unsigned char** ptr);
bool_t xdr_char_ptr_t(XDR *xdrs, char** ptr);


XDR_ENUM_DECLARE(DataAccountType_t)
XDR_ENUM_DECLARE(DataAuthenMethod_t)
XDR_ENUM_DECLARE(CSDDialType_t)
XDR_ENUM_DECLARE(CSDBaudRate_t)
XDR_ENUM_DECLARE(DC_ConnectionType_t)
XDR_ENUM_DECLARE(CSDSyncType_t)
XDR_ENUM_DECLARE(CSDConnElement_t)
XDR_ENUM_DECLARE(CSDDataCompType_t)
XDR_ENUM_DECLARE(CSDErrCorrectionType_t)
XDR_ENUM_DECLARE(DC_ConnectionStatus_t)
XDR_STRUCT_DECLARE(GPRSContext_t)
XDR_STRUCT_DECLARE(CSDContext_t)
XDR_STRUCT_DECLARE(DC_ReportCallStatus_t)
XDR_STRUCT_DECLARE(CAPI2_DATA_GetGPRSTft_Result_t)

bool_t xdr_PCHQosProfile_t(XDR *xdrs, PCHQosProfile_t*  ptr);

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
void CAPI2_DATA_GetGPRSTft_RSP(UInt32 tid, UInt8 clientID, CAPI2_DATA_GetGPRSTft_Result_t rsp);
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
void CAPI2_DC_ReportCallStatusInd(UInt32 tid, UInt8 clientID, DC_ReportCallStatus_t *status);

#endif


