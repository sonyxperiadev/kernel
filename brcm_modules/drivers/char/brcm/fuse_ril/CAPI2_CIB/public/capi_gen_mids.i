

//***************** < 8 > **********************





#ifdef DEFINE_CAPI_GEN_MIDS

	//MSG_GEN_REQ_START = 18944,

	 /** 
	api is CAPI2_SMS_IsMeStorageEnabled 
	**/
	MSG_SMS_ISMESTORAGEENABLED_REQ  = 0x4A00,
	 /** 
	payload is ::Boolean 
	**/
	MSG_SMS_ISMESTORAGEENABLED_RSP  = 0x4A01,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_SMS_GetMaxMeCapacity 
	**/
	MSG_SMS_GETMAXMECAPACITY_REQ  = 0x4A02,
	 /** 
	payload is ::UInt16 
	**/
	MSG_SMS_GETMAXMECAPACITY_RSP  = 0x4A03,	///<Payload type {::UInt16}
	 /** 
	api is CAPI2_SMS_GetNextFreeSlot 
	**/
	MSG_SMS_GETNEXTFREESLOT_REQ  = 0x4A04,
	 /** 
	payload is ::UInt16 
	**/
	MSG_SMS_GETNEXTFREESLOT_RSP  = 0x4A05,	///<Payload type {::UInt16}
	 /** 
	api is CAPI2_SMS_SetMeSmsStatus 
	**/
	MSG_SMS_SETMESMSSTATUS_REQ  = 0x4A06,	///<Payload type {::CAPI2_SMS_SetMeSmsStatus_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_SETMESMSSTATUS_RSP  = 0x4A07,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_SMS_GetMeSmsStatus 
	**/
	MSG_SMS_GETMESMSSTATUS_REQ  = 0x4A08,	///<Payload type {::CAPI2_SMS_GetMeSmsStatus_Req_t}
	 /** 
	payload is ::SIMSMSMesgStatus_t 
	**/
	MSG_SMS_GETMESMSSTATUS_RSP  = 0x4A09,	///<Payload type {::SIMSMSMesgStatus_t}
	 /** 
	api is CAPI2_SMS_StoreSmsToMe 
	**/
	MSG_SMS_STORESMSTOME_REQ  = 0x4A0A,	///<Payload type {::CAPI2_SMS_StoreSmsToMe_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_STORESMSTOME_RSP  = 0x4A0B,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_SMS_RetrieveSmsFromMe 
	**/
	MSG_SMS_RETRIEVESMSFROMME_REQ  = 0x4A0C,	///<Payload type {::CAPI2_SMS_RetrieveSmsFromMe_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_RETRIEVESMSFROMME_RSP  = 0x4A0D,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_SMS_RemoveSmsFromMe 
	**/
	MSG_SMS_REMOVESMSFROMME_REQ  = 0x4A0E,	///<Payload type {::CAPI2_SMS_RemoveSmsFromMe_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_REMOVESMSFROMME_RSP  = 0x4A0F,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_SMS_IsSmsReplSupported 
	**/
	MSG_SMS_ISSMSREPLACE_SUPPORTED_REQ  = 0x4A10,
	 /** 
	payload is ::Boolean 
	**/
	MSG_SMS_ISSMSREPLACE_SUPPORTED_RSP  = 0x4A11,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_SMS_GetMeSmsBufferStatus 
	**/
	MSG_SMS_GETMESMS_BUF_STATUS_REQ  = 0x4A12,	///<Payload type {::CAPI2_SMS_GetMeSmsBufferStatus_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_SMS_GETMESMS_BUF_STATUS_RSP  = 0x4A13,
	 /** 
	api is CAPI2_SMS_GetRecordNumberOfReplaceSMS 
	**/
	MSG_SMS_GETRECORDNUMBEROFREPLACESMS_REQ  = 0x4A14,	///<Payload type {::CAPI2_SMS_GetRecordNumberOfReplaceSMS_Req_t}
	 /** 
	payload is ::UInt16 
	**/
	MSG_SMS_GETRECORDNUMBEROFREPLACESMS_RSP  = 0x4A15,	///<Payload type {::UInt16}

	//MSG_GEN_REQ_END = 0x4AFF

#endif //DEFINE_CAPI_GEN_MIDS

