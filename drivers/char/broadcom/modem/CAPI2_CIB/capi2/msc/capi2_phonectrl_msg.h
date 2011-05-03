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
*   @file   capi2_phonectrl_msg.h
*
*   @brief  This file defines the capi2 message structure and forward declares
*	the serialization/deserialization functions.
*
****************************************************************************/
#ifndef CAPI2_PHONECTRL_MSG_H
#define CAPI2_PHONECTRL_MSG_H


#define MAX_NETREQ_QUERY_SIZE 512


typedef PLMN_NAME_t PLMN_NAME_PTR_t;

#define NULL_capi2_proc_t ((capi2_proc_t)0)

XDR_ENUM_DECLARE(MSStatusInd_t)
XDR_ENUM_DECLARE(RATSelect_t)
XDR_ENUM_DECLARE(BandSelect_t)
XDR_ENUM_DECLARE(SystemState_t)
XDR_ENUM_DECLARE(PowerDownState_t)

XDR_ENUM_DECLARE(PlmnSelectFormat_t)
XDR_ENUM_DECLARE(PlmnSelectMode_t)
XDR_ENUM_DECLARE(SysFilterEnable_t)
XDR_ENUM_DECLARE(GPRS_CiphAlg_t)
XDR_ENUM_DECLARE(PhonectrlDomain_t)
XDR_ENUM_DECLARE(PhonectrlCipherContext_t)
XDR_ENUM_DECLARE(GANSelect_t)
XDR_ENUM_DECLARE(GANStatus_t)

XDR_STRUCT_DECLARE(MsRxLevelData_t)
XDR_STRUCT_DECLARE(MSRegInfo_t)
XDR_STRUCT_DECLARE(nitzNetworkName_t)
XDR_STRUCT_DECLARE(lsaIdentity_t)
XDR_STRUCT_DECLARE(RxSignalInfo_t)
XDR_STRUCT_DECLARE(RX_SIGNAL_INFO_CHG_t)
XDR_STRUCT_DECLARE(SEARCHED_PLMN_LIST_t)
XDR_STRUCT_DECLARE(TimeZoneDate_t)
XDR_STRUCT_DECLARE(AtResponse_t)
XDR_STRUCT_DECLARE(MSRegStateInfo_t)
XDR_STRUCT_DECLARE(MSImeiStr_t)
XDR_STRUCT_DECLARE(CAPI2_Class_t)
XDR_STRUCT_DECLARE(InterTaskMsg_t)
XDR_STRUCT_DECLARE(MSRadioActivityInd_t)

XDR_ENUM_DECLARE(RegisterStatus_t)
XDR_ENUM_DECLARE(PCHRejectCause_t)
XDR_ENUM_DECLARE(TimeZoneUpdateMode_t)


XDR_ENUM_DECLARE(MS_Element_t)
XDR_STRUCT_DECLARE(MSRadioActivityInd_t)


XDR_ENUM_DECLARE(PLMNNameType_t)

XDR_ENUM_DECLARE(MSAttachMode_t)

bool_t xdr_CAPI2_MS_Element_t(XDR* xdrs, CAPI2_MS_Element_t *rsp);

bool_t xdr_PLMN_NAME_PTR_t(XDR *xdrs, PLMN_NAME_PTR_t* ptr);

bool_t xdr_MsPlmnInfo_t(XDR *xdrs, MsPlmnInfo_t* ptr);
bool_t xdr_PLMN_NAME_t(XDR *xdrs, PLMN_NAME_t* ptr);
bool_t xdr_MsPlmnName_t(XDR *xdrs, MsPlmnName_t* ptr);

bool_t xdr_MS_TestChan_t( XDR* xdrs, MS_TestChan_t* data);
bool_t xdr_MS_AMRParam_t(XDR* xdrs, MS_AMRParam_t *param);
bool_t xdr_MS_GPRSPacketParam_t(void* xdrs, MS_GPRSPacketParam_t *param);
bool_t xdr_MS_NcellMeas_t(void* xdrs, MS_NcellMeas_t *data);
bool_t xdr_MS_NcellList_t(void* xdrs, MS_NcellList_t *list);
bool_t xdr_MS_MA_t(void* xdrs, MS_MA_t *param);
bool_t xdr_MS_GSMParam_t(void* xdrs, MS_GSMParam_t *param);
bool_t xdr_MS_MMParam_t(void* xdrs, MS_MMParam_t *param);
bool_t xdr_MS_RxTestParam_t(void* xdrs, MS_RxTestParam_t *param);
bool_t xdr_MS_EDGEPacketParam_t(void* xdrs, MS_EDGEPacketParam_t *param);
bool_t xdr_MS_EDGEParam_t(void* xdrs, MS_EDGEParam_t *param);
bool_t xdr_MS_GenMeasParam_t(void* xdrs, MS_GenMeasParam_t *param);
bool_t xdr_MS_UmtsDchReport_t(void* xdrs, MS_UmtsDchReport_t *param);
bool_t xdr_MS_UmtsMeasIdParam_t(void* xdrs, MS_UmtsMeasIdParam_t *param);
bool_t xdr_MS_UmtsGsmNcell_t(void* xdrs, MS_UmtsGsmNcell_t *param);
bool_t xdr_MS_UmtsGsmNcellList_t(void* xdrs, MS_UmtsGsmNcellList_t *param);
bool_t xdr_MS_UmtsNcell_t(void* xdrs, MS_UmtsNcell_t *param);
bool_t xdr_MS_UmtsNcellList_t(void* xdrs, MS_UmtsNcellList_t *param);
bool_t xdr_MS_UMTSParam_t(void* xdrs, MS_UMTSParam_t *param);
bool_t xdr_MS_RxTestParam_t(void* xdrs, MS_RxTestParam_t *param);
bool_t xdr_MS_UmtsMeasReport_t(void* xdrs, MS_UmtsMeasReport_t *param);
bool_t xdr_T_USF_ARRAY(void* xdrs, T_USF_ARRAY *param);
bool_t xdr_MS_Ext_MACParam_t(void* xdrs, MS_Ext_MACParam_t *param);
bool_t xdr_MS_Ext_SMParam_t(void* xdrs, MS_Ext_SMParam_t *param);
bool_t xdr_MS_Ext_L1Param_t(void* xdrs, MS_Ext_L1Param_t *param);
bool_t xdr_MS_Ext_RLCParam_t(void* xdrs, MS_Ext_RLCParam_t *param);
bool_t xdr_MS_Ext_RRParam_t(void* xdrs, MS_Ext_RRParam_t *param);
bool_t xdr_EQUIV_PLMN_LIST_t(void* xdrs, EQUIV_PLMN_LIST_t *param);
bool_t xdr_MS_Ext_MMParam_t(void* xdrs, MS_Ext_MMParam_t *param);
bool_t xdr_MS_Ext_SMSParam_t(void* xdrs, MS_Ext_SMSParam_t *param);
bool_t xdr_MS_ExtParam_t(void* xdrs, MS_ExtParam_t *param);
bool_t xdr_MS_StatusIndication_t(XDR* xdrs, MS_StatusIndication_t* data);
bool_t xdr_TMSI_Octets_t(void* xdrs, TMSI_Octets_t *param);
bool_t xdr_ACK_NACK_CNT_t(void* xdrs, ACK_NACK_CNT_t *param);
bool_t xdr_ACK_NACK_Testparam_t(void* xdrs, ACK_NACK_Testparam_t *param);
bool_t xdr_RLC_State_Variable_List_t(void* xdrs, RLC_State_Variable_List_t *param);
bool_t xdr_RLC_ACK_NACK_List_t(void* xdrs, RLC_ACK_NACK_List_t *list);
bool_t xdr_Access_Tech_List_t(void* xdrs, Access_Tech_List_t *param);
bool_t xdr_RLC_Testparam_t(void* xdrs, RLC_Testparam_t *param);
bool_t xdr_Uas_Conn_Info(void* xdrs, Uas_Conn_Info *param);
bool_t xdr_MSUe3gStatusInd_t(void* xdrs, MSUe3gStatusInd_t *param);
bool_t xdr_MS_ElemGprsParam_t(XDR* xdrs, MS_ElemGprsParam_t *param);
bool_t xdr_PhonectrlCipherAlg_t(XDR* xdrs, PhonectrlCipherAlg_t* param);
bool_t xdr_Phonectrl_Nmr_t(XDR* xdrs, Phonectrl_Nmr_t* param);

#define xdr_CellInfo_t xdr_u_int16_t

#endif

