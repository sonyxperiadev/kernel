/****************************************************************************
*
*     Copyright (c) 2007-2008 Broadcom Corporation
*
*   Unless you and Broadcom execute a separate written software license
*   agreement governing use of this software, this software is licensed to you
*   under the terms of the GNU General Public License version 2, available
*    at http://www.gnu.org/licenses/old-licenses/gpl-2.0.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
*   other than the GPL, without Broadcom's express prior written consent.
*
****************************************************************************/
/****************************************************************************
*
*     WARNING!!!! Generated File ( Do NOT Modify !!!! )
*
****************************************************************************/
#ifndef SYS_GEN_MSG_H
#define SYS_GEN_MSG_H

/****************** < 1 > **********************/

typedef struct {
	UInt32 cmd;
	UInt32 address;
	UInt32 offset;
	UInt32 size;
} CAPI2_CPPS_Control_Req_t;

typedef struct {
	UInt32 val;
} CAPI2_CPPS_Control_Rsp_t;

typedef struct {
	PMU_SIMLDO_t simldo;
} CAPI2_SYSRPC_PMU_IsSIMReady_Req_t;

typedef struct {
	Boolean val;
} CAPI2_SYSRPC_PMU_IsSIMReady_Rsp_t;

typedef struct {
	PMU_SIMLDO_t simldo;
	PMU_SIMVolt_t volt;
} CAPI2_SYSRPC_PMU_ActivateSIM_Req_t;

typedef struct {
	UInt32 flash_addr;
	UInt32 length;
	UInt32 shared_mem_addr;
} CAPI2_FLASH_SaveImage_Req_t;

typedef struct {
	Boolean val;
} CAPI2_FLASH_SaveImage_Rsp_t;

typedef struct {
	UInt8 simId;
	SYS_SIMLOCK_SIM_DATA_t *sim_data;
	Boolean is_testsim;
} SYS_SimLockApi_GetStatus_Req_t;

typedef struct {
	SYS_SIMLOCK_STATE_t val;
} SYS_SimLockApi_GetStatus_Rsp_t;

typedef struct {
	UInt8 simId;
	SYS_SIMLOCK_STATE_t *simlock_state;
} SYS_SIMLOCKApi_SetStatusEx_Req_t;

typedef struct {
	UInt8 simId;
} SYS_SimApi_GetCurrLockedSimlockTypeEx_Req_t;

typedef struct {
	UInt32 val;
} SYS_SimApi_GetCurrLockedSimlockTypeEx_Rsp_t;

/****************** < 2 > **********************/

bool_t xdr_CAPI2_CPPS_Control_Req_t(void *xdrs, CAPI2_CPPS_Control_Req_t *rsp);
bool_t xdr_CAPI2_CPPS_Control_Rsp_t(void *xdrs, CAPI2_CPPS_Control_Rsp_t *rsp);
bool_t xdr_CAPI2_SYSRPC_PMU_IsSIMReady_Req_t(void *xdrs,
					     CAPI2_SYSRPC_PMU_IsSIMReady_Req_t *
					     rsp);
bool_t xdr_CAPI2_SYSRPC_PMU_IsSIMReady_Rsp_t(void *xdrs,
					     CAPI2_SYSRPC_PMU_IsSIMReady_Rsp_t *
					     rsp);
bool_t xdr_CAPI2_SYSRPC_PMU_ActivateSIM_Req_t(void *xdrs,
					      CAPI2_SYSRPC_PMU_ActivateSIM_Req_t
					      *rsp);
bool_t xdr_CAPI2_FLASH_SaveImage_Req_t(void *xdrs,
				       CAPI2_FLASH_SaveImage_Req_t *rsp);
bool_t xdr_CAPI2_FLASH_SaveImage_Rsp_t(void *xdrs,
				       CAPI2_FLASH_SaveImage_Rsp_t *rsp);
bool_t xdr_SYS_SimLockApi_GetStatus_Req_t(void *xdrs,
					  SYS_SimLockApi_GetStatus_Req_t *rsp);
bool_t xdr_SYS_SimLockApi_GetStatus_Rsp_t(void *xdrs,
					  SYS_SimLockApi_GetStatus_Rsp_t *rsp);
bool_t xdr_SYS_SIMLOCKApi_SetStatusEx_Req_t(void *xdrs,
					    SYS_SIMLOCKApi_SetStatusEx_Req_t *
					    rsp);
bool_t xdr_SYS_SimApi_GetCurrLockedSimlockTypeEx_Req_t(void *xdrs,
						       SYS_SimApi_GetCurrLockedSimlockTypeEx_Req_t
						       *rsp);
bool_t xdr_SYS_SimApi_GetCurrLockedSimlockTypeEx_Rsp_t(void *xdrs,
						       SYS_SimApi_GetCurrLockedSimlockTypeEx_Rsp_t
						       *rsp);

/****************** < 3 > **********************/

Result_t Handle_CAPI2_CPPS_Control(RPC_Msg_t *pReqMsg, UInt32 cmd,
				   UInt32 address, UInt32 offset, UInt32 size);
Result_t Handle_CAPI2_SYSRPC_PMU_IsSIMReady(RPC_Msg_t *pReqMsg,
					    PMU_SIMLDO_t simldo);
Result_t Handle_CAPI2_SYSRPC_PMU_ActivateSIM(RPC_Msg_t *pReqMsg,
					     PMU_SIMLDO_t simldo,
					     PMU_SIMVolt_t volt);
Result_t Handle_CAPI2_FLASH_SaveImage(RPC_Msg_t *pReqMsg, UInt32 flash_addr,
				      UInt32 length, UInt32 shared_mem_addr);
Result_t Handle_SYS_SimLockApi_GetStatus(RPC_Msg_t *pReqMsg, UInt8 simId,
					 SYS_SIMLOCK_SIM_DATA_t *sim_data,
					 Boolean is_testsim);
Result_t Handle_SYS_SIMLOCKApi_SetStatusEx(RPC_Msg_t *pReqMsg, UInt8 simId,
					   SYS_SIMLOCK_STATE_t *simlock_state);
Result_t Handle_SYS_SimApi_GetCurrLockedSimlockTypeEx(RPC_Msg_t *pReqMsg,
						      UInt8 simId);

/****************** < 12 > **********************/

/******************************************************************************/
/**
	Function response for the CAPI2_SYSRPC_PMU_IsSIMReady
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@param		simldo(in) param of type PMU_SIMLDO_t
	@return		Not Applicable
	@note
	Payload: Boolean
	@n Response to CP will be notified via ::MSG_PMU_IS_SIM_READY_RSP
**/
void CAPI2_SYSRPC_PMU_IsSIMReady(UInt32 tid, UInt8 clientID,
				 PMU_SIMLDO_t simldo);

/******************************************************************************/
/**
	Function response for the CAPI2_SYSRPC_PMU_ActivateSIM
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@param		simldo(in) param of type PMU_SIMLDO_t
	@param		volt(in) param of type PMU_SIMVolt_t
	@return		Not Applicable
	@note
	Payload: default_proc
	@n Response to CP will be notified via ::MSG_PMU_ACTIVATE_SIM_RSP
**/
void CAPI2_SYSRPC_PMU_ActivateSIM(UInt32 tid, UInt8 clientID,
				  PMU_SIMLDO_t simldo, PMU_SIMVolt_t volt);

/******************************************************************************/
/**
	Function response for the SYS_SimLockApi_GetStatus
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@param		simId(in) param of type UInt8
	@param		sim_data(in) param of type SYS_SIMLOCK_SIM_DATA_t
	@param		is_testsim(in) param of type Boolean
	@return		Not Applicable
	@note
	Payload: SYS_SIMLOCK_STATE_t
	@n Response to CP will be notified via ::MSG_SYS_SIMLOCK_GET_STATUS_RSP
**/
void SYS_SimLockApi_GetStatus(UInt32 tid, UInt8 clientID, UInt8 simId,
			      SYS_SIMLOCK_SIM_DATA_t *sim_data,
			      Boolean is_testsim);

/******************************************************************************/
/**
	Function response for the SYS_SIMLOCKApi_SetStatusEx
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@param		simId(in) param of type UInt8
	@param		simlock_state(in) param of type SYS_SIMLOCK_STATE_t
	@return		Not Applicable
	@note
	Payload: default_proc
	@n Response to CP will be notified via ::MSG_SYS_SIMLOCK_SET_STATUS_RSP
**/
void SYS_SIMLOCKApi_SetStatusEx(UInt32 tid, UInt8 clientID, UInt8 simId,
				SYS_SIMLOCK_STATE_t *simlock_state);

/******************************************************************************/
/**
	Function response for the SYS_SimApi_GetCurrLockedSimlockTypeEx
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@param		simId(in) param of type UInt8
	@return		Not Applicable
	@note
	Payload: UInt32
	@n Response to CP will be notified via ::MSG_SYS_GET_CUR_SIMLOCK_TYPE_RSP
**/
void SYS_SimApi_GetCurrLockedSimlockTypeEx(UInt32 tid, UInt8 clientID,
					   UInt8 simId);

/****************** < 16 > **********************/

#endif
