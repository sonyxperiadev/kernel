

//***************** < 8 > **********************





#ifdef DEFINE_SYS_GEN_MIDS

	//MSG_GEN_REQ_START = 19200,

	 /** 
	api is CAPI2_PMU_BattADCReq 
	**/
	MSG_PMU_BATT_LEVEL_REQ  = 0x4B00,
	 /** 
	payload is ::default_proc 
	**/
	MSG_PMU_BATT_LEVEL_RSP  = 0x4B01,
	 /** 
	api is CAPI2_CPPS_Control 
	**/
	MSG_CPPS_CONTROL_REQ  = 0x4B02,	///<Payload type {::CAPI2_CPPS_Control_Req_t}
	 /** 
	payload is ::UInt32 
	**/
	MSG_CPPS_CONTROL_RSP  = 0x4B03,	///<Payload type {::UInt32}
	 /** 
	api is CAPI2_CP2AP_PedestalMode_Control 
	**/
	MSG_CP2AP_PEDESTALMODE_CONTROL_REQ  = 0x4B04,	///<Payload type {::CAPI2_CP2AP_PedestalMode_Control_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_CP2AP_PEDESTALMODE_CONTROL_RSP  = 0x4B05,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_PMU_IsSIMReady 
	**/
	MSG_PMU_IS_SIM_READY_REQ  = 0x4B06,	///<Payload type {::CAPI2_PMU_IsSIMReady_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_PMU_IS_SIM_READY_RSP  = 0x4B07,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_PMU_ActivateSIM 
	**/
	MSG_PMU_ACTIVATE_SIM_REQ  = 0x4B08,	///<Payload type {::CAPI2_PMU_ActivateSIM_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_PMU_ACTIVATE_SIM_RSP  = 0x4B09,
	 /** 
	api is CAPI2_PMU_ClientPowerDown 
	**/
	MSG_PMU_ClientPowerDown_REQ  = 0x4B0A,
	 /** 
	payload is ::default_proc 
	**/
	MSG_PMU_ClientPowerDown_RSP  = 0x4B0B,
	 /** 
	api is CAPI2_FLASH_SaveImage 
	**/
	MSG_FLASH_SAVEIMAGE_REQ  = 0x4B0C,	///<Payload type {::CAPI2_FLASH_SaveImage_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_FLASH_SAVEIMAGE_RSP  = 0x4B0D,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_USB_IpcMsg 
	**/
	MSG_USB_IPC_REQ  = 0x4B0E,	///<Payload type {::CAPI2_USB_IpcMsg_Req_t}
	 /** 
	payload is ::USBPayload_t 
	**/
	MSG_USB_IPC_RSP  = 0x4B0F,	///<Payload type {::USBPayload_t}
	 /** 
	api is CAPI2_SYSPARM_GetIMEI 
	**/
	MSG_SYSPARAM_GET_IMEI_REQ  = 0x4B10,
	 /** 
	payload is ::CAPI2_SYSPARM_IMEI_PTR_t 
	**/
	MSG_SYSPARAM_GET_IMEI_RSP  = 0x4B11,	///<Payload type {::CAPI2_SYSPARM_IMEI_PTR_t}
	 /** 
	api is SYS_Sync_SetRegisteredEventMask 
	**/
	MSG_SYS_SYNC_SET_REG_EVENT_MASK_REQ  = 0x4B12,	///<Payload type {::SYS_Sync_SetRegisteredEventMask_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_SYS_SYNC_SET_REG_EVENT_MASK_RSP  = 0x4B13,	///<Payload type {::Boolean}
	 /** 
	api is SYS_Sync_SetFilteredEventMask 
	**/
	MSG_SYS_SYNC_SET_REG_FILTER_MASK_REQ  = 0x4B14,	///<Payload type {::SYS_Sync_SetFilteredEventMask_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_SYS_SYNC_SET_REG_FILTER_MASK_RSP  = 0x4B15,	///<Payload type {::Boolean}
	 /** 
	api is SYS_Sync_RegisterForMSEvent 
	**/
	MSG_SYS_SYNC_REG_EVENT_REQ  = 0x4B16,	///<Payload type {::SYS_Sync_RegisterForMSEvent_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_SYS_SYNC_REG_EVENT_RSP  = 0x4B17,	///<Payload type {::Boolean}
	 /** 
	api is SYS_Sync_DeRegisterForMSEvent 
	**/
	MSG_SYS_SYNC_DEREG_EVENT_REQ  = 0x4B18,
	 /** 
	payload is ::Boolean 
	**/
	MSG_SYS_SYNC_DEREG_EVENT_RSP  = 0x4B19,	///<Payload type {::Boolean}
	 /** 
	api is SYS_Sync_EnableFilterMask 
	**/
	MSG_SYS_SYNC_ENABLE_FILTER_REQ  = 0x4B1A,	///<Payload type {::SYS_Sync_EnableFilterMask_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_SYS_SYNC_ENABLE_FILTER_RSP  = 0x4B1B,	///<Payload type {::Boolean}
	 /** 
	api is SYS_Sync_RegisterSetClientName 
	**/
	MSG_SYS_SYNC_REGISTER_NAME_REQ  = 0x4B1C,	///<Payload type {::SYS_Sync_RegisterSetClientName_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_SYS_SYNC_REGISTER_NAME_RSP  = 0x4B1D,	///<Payload type {::Boolean}

	//MSG_GEN_REQ_END = 0x4BFF

#endif //DEFINE_SYS_GEN_MIDS

