

//***************** < 6 > **********************





#ifdef DEVELOPMENT_SYSRPC_UNION_MAPPING

	{ MSG_AT_MTEST_HANDLER_REQ,_T("MSG_AT_MTEST_HANDLER_REQ"), (xdrproc_t) xdr_SYS_AT_MTEST_Handler_Req_t, 0, 0},
	{ MSG_AT_MTEST_HANDLER_RSP,_T("MSG_AT_MTEST_HANDLER_RSP"), (xdrproc_t)xdr_SYS_AT_MTEST_Handler_Rsp_t, sizeof(struct MtestOutput_t), 0 },
	{ MSG_SYS_SIMLOCK_GET_STATUS_REQ,_T("MSG_SYS_SIMLOCK_GET_STATUS_REQ"), (xdrproc_t) xdr_SYS_SimLockApi_GetStatus_Req_t, 0, 0},
	{ MSG_SYS_SIMLOCK_GET_STATUS_RSP,_T("MSG_SYS_SIMLOCK_GET_STATUS_RSP"), (xdrproc_t)xdr_SYS_SimLockApi_GetStatus_Rsp_t, sizeof( SYS_SIMLOCK_STATE_t ), 0 },
    { MSG_SYS_SIMLOCK_SET_STATUS_REQ,_T("MSG_SYS_SIMLOCK_SET_STATUS_REQ"), (xdrproc_t) xdr_SYS_SIMLOCKApi_SetStatusEx_Req_t, 0, 0},
    { MSG_SYS_SIMLOCK_SET_STATUS_RSP,_T("MSG_SYS_SIMLOCK_SET_STATUS_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
    { MSG_SYS_GET_CUR_SIMLOCK_TYPE_REQ,_T("MSG_SYS_GET_CUR_SIMLOCK_TYPE_REQ"), (xdrproc_t) xdr_SYS_SimApi_GetCurrLockedSimlockTypeEx_Req_t, 0, 0},
	{ MSG_SYS_GET_CUR_SIMLOCK_TYPE_RSP,_T("MSG_SYS_GET_CUR_SIMLOCK_TYPE_RSP"), (xdrproc_t)xdr_SYS_SimApi_GetCurrLockedSimlockType_Rsp_t, sizeof( UInt32 ), 0 },
    { MSG_SYS_GET_CUR_SIMLOCK_TYPE_RSP,_T("MSG_SYS_GET_CUR_SIMLOCK_TYPE_RSP"), (xdrproc_t)xdr_SYS_SimApi_GetCurrLockedSimlockTypeEx_Rsp_t, sizeof( UInt32 ), 0 },
	
#endif //DEVELOPMENT_SYSRPC_UNION_MAPPING


//***************** < 7 > **********************





#ifdef DEVELOPMENT_SYSRPC_UNION_DECLARE

	struct SYS_AT_MTEST_Handler_Req_t   SYS_AT_MTEST_Handler_Req;
	struct SYS_AT_MTEST_Handler_Rsp_t   SYS_AT_MTEST_Handler_Rsp;
	SYS_SimLockApi_GetStatus_Req_t   SYS_SimLockApi_GetStatus_Req;
	SYS_SimLockApi_GetStatus_Rsp_t   SYS_SimLockApi_GetStatus_Rsp;
    SYS_SIMLOCKApi_SetStatusEx_Req_t   SYS_SIMLOCKApi_SetStatusEx_Req;
    SYS_SimApi_GetCurrLockedSimlockTypeEx_Req_t   SYS_SimApi_GetCurrLockedSimlockTypeEx_Req;
    SYS_SimApi_GetCurrLockedSimlockTypeEx_Rsp_t   SYS_SimApi_GetCurrLockedSimlockTypeEx_Rsp;

#endif //DEVELOPMENT_SYSRPC_UNION_DECLARE

