

//***************** < 6 > **********************





#ifdef DEVELOPMENT_MODEM_CAPI_XDR_UNION_MAPPING

	{ MSG_SMS_ISMESTORAGEENABLED_REQ,_T("MSG_SMS_ISMESTORAGEENABLED_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SMS_ISMESTORAGEENABLED_RSP,_T("MSG_SMS_ISMESTORAGEENABLED_RSP"), (xdrproc_t)xdr_CAPI2_SMS_IsMeStorageEnabled_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_SMS_GETMAXMECAPACITY_REQ,_T("MSG_SMS_GETMAXMECAPACITY_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SMS_GETMAXMECAPACITY_RSP,_T("MSG_SMS_GETMAXMECAPACITY_RSP"), (xdrproc_t)xdr_CAPI2_SMS_GetMaxMeCapacity_Rsp_t, sizeof( UInt16 ), 0 },
	{ MSG_SMS_GETNEXTFREESLOT_REQ,_T("MSG_SMS_GETNEXTFREESLOT_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SMS_GETNEXTFREESLOT_RSP,_T("MSG_SMS_GETNEXTFREESLOT_RSP"), (xdrproc_t)xdr_CAPI2_SMS_GetNextFreeSlot_Rsp_t, sizeof( UInt16 ), 0 },
	{ MSG_SMS_SETMESMSSTATUS_REQ,_T("MSG_SMS_SETMESMSSTATUS_REQ"), (xdrproc_t) xdr_CAPI2_SMS_SetMeSmsStatus_Req_t, 0, 0},
	{ MSG_SMS_SETMESMSSTATUS_RSP,_T("MSG_SMS_SETMESMSSTATUS_RSP"), (xdrproc_t)xdr_CAPI2_SMS_SetMeSmsStatus_Rsp_t, sizeof( Result_t ), 0 },
	{ MSG_SMS_GETMESMSSTATUS_REQ,_T("MSG_SMS_GETMESMSSTATUS_REQ"), (xdrproc_t) xdr_CAPI2_SMS_GetMeSmsStatus_Req_t, 0, 0},
	{ MSG_SMS_GETMESMSSTATUS_RSP,_T("MSG_SMS_GETMESMSSTATUS_RSP"), (xdrproc_t)xdr_CAPI2_SMS_GetMeSmsStatus_Rsp_t, sizeof( SIMSMSMesgStatus_t ), 0 },
	{ MSG_SMS_STORESMSTOME_REQ,_T("MSG_SMS_STORESMSTOME_REQ"), (xdrproc_t) xdr_CAPI2_SMS_StoreSmsToMe_Req_t, 0, 0},
	{ MSG_SMS_STORESMSTOME_RSP,_T("MSG_SMS_STORESMSTOME_RSP"), (xdrproc_t)xdr_CAPI2_SMS_StoreSmsToMe_Rsp_t, sizeof( Result_t ), 0 },
	{ MSG_SMS_RETRIEVESMSFROMME_REQ,_T("MSG_SMS_RETRIEVESMSFROMME_REQ"), (xdrproc_t) xdr_CAPI2_SMS_RetrieveSmsFromMe_Req_t, 0, 0},
	{ MSG_SMS_RETRIEVESMSFROMME_RSP,_T("MSG_SMS_RETRIEVESMSFROMME_RSP"), (xdrproc_t)xdr_CAPI2_SMS_RetrieveSmsFromMe_Rsp_t, sizeof( Result_t ), 0 },
	{ MSG_SMS_REMOVESMSFROMME_REQ,_T("MSG_SMS_REMOVESMSFROMME_REQ"), (xdrproc_t) xdr_CAPI2_SMS_RemoveSmsFromMe_Req_t, 0, 0},
	{ MSG_SMS_REMOVESMSFROMME_RSP,_T("MSG_SMS_REMOVESMSFROMME_RSP"), (xdrproc_t)xdr_CAPI2_SMS_RemoveSmsFromMe_Rsp_t, sizeof( Result_t ), 0 },
	{ MSG_SMS_ISSMSREPLACE_SUPPORTED_REQ,_T("MSG_SMS_ISSMSREPLACE_SUPPORTED_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SMS_ISSMSREPLACE_SUPPORTED_RSP,_T("MSG_SMS_ISSMSREPLACE_SUPPORTED_RSP"), (xdrproc_t)xdr_CAPI2_SMS_IsSmsReplSupported_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_SMS_GETMESMS_BUF_STATUS_REQ,_T("MSG_SMS_GETMESMS_BUF_STATUS_REQ"), (xdrproc_t) xdr_CAPI2_SMS_GetMeSmsBufferStatus_Req_t, 0, 0},
	{ MSG_SMS_GETMESMS_BUF_STATUS_RSP,_T("MSG_SMS_GETMESMS_BUF_STATUS_RSP"), (xdrproc_t)xdr_CAPI2_SMS_GetMeSmsBufferStatus_RSP_Rsp_t, sizeof( CAPI2_SMS_GetMeSmsBufferStatus_RSP_Rsp_t ), 0 },
	{ MSG_SMS_GETRECORDNUMBEROFREPLACESMS_REQ,_T("MSG_SMS_GETRECORDNUMBEROFREPLACESMS_REQ"), (xdrproc_t) xdr_CAPI2_SMS_GetRecordNumberOfReplaceSMS_Req_t, 0, 0},
	{ MSG_SMS_GETRECORDNUMBEROFREPLACESMS_RSP,_T("MSG_SMS_GETRECORDNUMBEROFREPLACESMS_RSP"), (xdrproc_t)xdr_CAPI2_SMS_GetRecordNumberOfReplaceSMS_Rsp_t, sizeof( UInt16 ), 0 },
#endif //DEVELOPMENT_MODEM_CAPI_XDR_UNION_MAPPING


//***************** < 7 > **********************





#ifdef DEVELOPMENT_MODEM_CAPI_XDR_UNION_DECLARE

	CAPI2_SMS_IsMeStorageEnabled_Rsp_t   CAPI2_SMS_IsMeStorageEnabled_Rsp;
	CAPI2_SMS_GetMaxMeCapacity_Rsp_t   CAPI2_SMS_GetMaxMeCapacity_Rsp;
	CAPI2_SMS_GetNextFreeSlot_Rsp_t   CAPI2_SMS_GetNextFreeSlot_Rsp;
	CAPI2_SMS_SetMeSmsStatus_Req_t   CAPI2_SMS_SetMeSmsStatus_Req;
	CAPI2_SMS_SetMeSmsStatus_Rsp_t   CAPI2_SMS_SetMeSmsStatus_Rsp;
	CAPI2_SMS_GetMeSmsStatus_Req_t   CAPI2_SMS_GetMeSmsStatus_Req;
	CAPI2_SMS_GetMeSmsStatus_Rsp_t   CAPI2_SMS_GetMeSmsStatus_Rsp;
	CAPI2_SMS_StoreSmsToMe_Req_t   CAPI2_SMS_StoreSmsToMe_Req;
	CAPI2_SMS_StoreSmsToMe_Rsp_t   CAPI2_SMS_StoreSmsToMe_Rsp;
	CAPI2_SMS_RetrieveSmsFromMe_Req_t   CAPI2_SMS_RetrieveSmsFromMe_Req;
	CAPI2_SMS_RetrieveSmsFromMe_Rsp_t   CAPI2_SMS_RetrieveSmsFromMe_Rsp;
	CAPI2_SMS_RemoveSmsFromMe_Req_t   CAPI2_SMS_RemoveSmsFromMe_Req;
	CAPI2_SMS_RemoveSmsFromMe_Rsp_t   CAPI2_SMS_RemoveSmsFromMe_Rsp;
	CAPI2_SMS_IsSmsReplSupported_Rsp_t   CAPI2_SMS_IsSmsReplSupported_Rsp;
	CAPI2_SMS_GetMeSmsBufferStatus_Req_t   CAPI2_SMS_GetMeSmsBufferStatus_Req;
	CAPI2_SMS_GetMeSmsBufferStatus_RSP_Rsp_t   CAPI2_SMS_GetMeSmsBufferStatus_RSP_Rsp;
	CAPI2_SMS_GetRecordNumberOfReplaceSMS_Req_t   CAPI2_SMS_GetRecordNumberOfReplaceSMS_Req;
	CAPI2_SMS_GetRecordNumberOfReplaceSMS_Rsp_t   CAPI2_SMS_GetRecordNumberOfReplaceSMS_Rsp;
#endif //DEVELOPMENT_MODEM_CAPI_XDR_UNION_DECLARE

