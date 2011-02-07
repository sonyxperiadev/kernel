

//***************** < 6 > **********************





#ifdef DEVELOPMENT_SYSRPC_UNION_MAPPING

	{ MSG_PMU_BATT_LEVEL_REQ,_T("MSG_PMU_BATT_LEVEL_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_PMU_BATT_LEVEL_RSP,_T("MSG_PMU_BATT_LEVEL_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_CPPS_CONTROL_REQ,_T("MSG_CPPS_CONTROL_REQ"), (xdrproc_t) xdr_CAPI2_CPPS_Control_Req_t, 0, 0},
	{ MSG_CPPS_CONTROL_RSP,_T("MSG_CPPS_CONTROL_RSP"), (xdrproc_t)xdr_CAPI2_CPPS_Control_Rsp_t, sizeof( UInt32 ), 0 },
	{ MSG_CP2AP_PEDESTALMODE_CONTROL_REQ,_T("MSG_CP2AP_PEDESTALMODE_CONTROL_REQ"), (xdrproc_t) xdr_CAPI2_CP2AP_PedestalMode_Control_Req_t, 0, 0},
	{ MSG_CP2AP_PEDESTALMODE_CONTROL_RSP,_T("MSG_CP2AP_PEDESTALMODE_CONTROL_RSP"), (xdrproc_t)xdr_CAPI2_CP2AP_PedestalMode_Control_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_PMU_IS_SIM_READY_REQ,_T("MSG_PMU_IS_SIM_READY_REQ"), (xdrproc_t) xdr_CAPI2_PMU_IsSIMReady_Req_t, 0, 0},
	{ MSG_PMU_IS_SIM_READY_RSP,_T("MSG_PMU_IS_SIM_READY_RSP"), (xdrproc_t)xdr_CAPI2_PMU_IsSIMReady_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_PMU_ACTIVATE_SIM_REQ,_T("MSG_PMU_ACTIVATE_SIM_REQ"), (xdrproc_t) xdr_CAPI2_PMU_ActivateSIM_Req_t, 0, 0},
	{ MSG_PMU_ACTIVATE_SIM_RSP,_T("MSG_PMU_ACTIVATE_SIM_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_PMU_ClientPowerDown_REQ,_T("MSG_PMU_ClientPowerDown_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_PMU_ClientPowerDown_RSP,_T("MSG_PMU_ClientPowerDown_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_FLASH_SAVEIMAGE_REQ,_T("MSG_FLASH_SAVEIMAGE_REQ"), (xdrproc_t) xdr_CAPI2_FLASH_SaveImage_Req_t, 0, 0},
	{ MSG_FLASH_SAVEIMAGE_RSP,_T("MSG_FLASH_SAVEIMAGE_RSP"), (xdrproc_t)xdr_CAPI2_FLASH_SaveImage_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_USB_IPC_REQ,_T("MSG_USB_IPC_REQ"), (xdrproc_t) xdr_CAPI2_USB_IpcMsg_Req_t, 0, 6000},
	{ MSG_USB_IPC_RSP,_T("MSG_USB_IPC_RSP"), (xdrproc_t)xdr_CAPI2_USB_IpcMsg_Rsp_t, sizeof( USBPayload_t ), 6000 },
	{ MSG_SYSPARAM_GET_IMEI_REQ,_T("MSG_SYSPARAM_GET_IMEI_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SYSPARAM_GET_IMEI_RSP,_T("MSG_SYSPARAM_GET_IMEI_RSP"), (xdrproc_t)xdr_CAPI2_SYSPARM_GetIMEI_Rsp_t, sizeof( CAPI2_SYSPARM_IMEI_PTR_t ), 0 },
	{ MSG_SYS_SYNC_SET_REG_EVENT_MASK_REQ,_T("MSG_SYS_SYNC_SET_REG_EVENT_MASK_REQ"), (xdrproc_t) xdr_SYS_Sync_SetRegisteredEventMask_Req_t, 0, 0},
	{ MSG_SYS_SYNC_SET_REG_EVENT_MASK_RSP,_T("MSG_SYS_SYNC_SET_REG_EVENT_MASK_RSP"), (xdrproc_t)xdr_SYS_Sync_SetRegisteredEventMask_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_SYS_SYNC_SET_REG_FILTER_MASK_REQ,_T("MSG_SYS_SYNC_SET_REG_FILTER_MASK_REQ"), (xdrproc_t) xdr_SYS_Sync_SetFilteredEventMask_Req_t, 0, 0},
	{ MSG_SYS_SYNC_SET_REG_FILTER_MASK_RSP,_T("MSG_SYS_SYNC_SET_REG_FILTER_MASK_RSP"), (xdrproc_t)xdr_SYS_Sync_SetFilteredEventMask_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_SYS_SYNC_REG_EVENT_REQ,_T("MSG_SYS_SYNC_REG_EVENT_REQ"), (xdrproc_t) xdr_SYS_Sync_RegisterForMSEvent_Req_t, 0, 0},
	{ MSG_SYS_SYNC_REG_EVENT_RSP,_T("MSG_SYS_SYNC_REG_EVENT_RSP"), (xdrproc_t)xdr_SYS_Sync_RegisterForMSEvent_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_SYS_SYNC_DEREG_EVENT_REQ,_T("MSG_SYS_SYNC_DEREG_EVENT_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_SYS_SYNC_DEREG_EVENT_RSP,_T("MSG_SYS_SYNC_DEREG_EVENT_RSP"), (xdrproc_t)xdr_SYS_Sync_DeRegisterForMSEvent_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_SYS_SYNC_ENABLE_FILTER_REQ,_T("MSG_SYS_SYNC_ENABLE_FILTER_REQ"), (xdrproc_t) xdr_SYS_Sync_EnableFilterMask_Req_t, 0, 0},
	{ MSG_SYS_SYNC_ENABLE_FILTER_RSP,_T("MSG_SYS_SYNC_ENABLE_FILTER_RSP"), (xdrproc_t)xdr_SYS_Sync_EnableFilterMask_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_SYS_SYNC_REGISTER_NAME_REQ,_T("MSG_SYS_SYNC_REGISTER_NAME_REQ"), (xdrproc_t) xdr_SYS_Sync_RegisterSetClientName_Req_t, 0, 0},
	{ MSG_SYS_SYNC_REGISTER_NAME_RSP,_T("MSG_SYS_SYNC_REGISTER_NAME_RSP"), (xdrproc_t)xdr_SYS_Sync_RegisterSetClientName_Rsp_t, sizeof( Boolean ), 0 },
#endif //DEVELOPMENT_SYSRPC_UNION_MAPPING


//***************** < 7 > **********************





#ifdef DEVELOPMENT_SYSRPC_UNION_DECLARE

	CAPI2_CPPS_Control_Req_t   CAPI2_CPPS_Control_Req;
	CAPI2_CPPS_Control_Rsp_t   CAPI2_CPPS_Control_Rsp;
	CAPI2_CP2AP_PedestalMode_Control_Req_t   CAPI2_CP2AP_PedestalMode_Control_Req;
	CAPI2_CP2AP_PedestalMode_Control_Rsp_t   CAPI2_CP2AP_PedestalMode_Control_Rsp;
	CAPI2_PMU_IsSIMReady_Req_t   CAPI2_PMU_IsSIMReady_Req;
	CAPI2_PMU_IsSIMReady_Rsp_t   CAPI2_PMU_IsSIMReady_Rsp;
	CAPI2_PMU_ActivateSIM_Req_t   CAPI2_PMU_ActivateSIM_Req;
	CAPI2_FLASH_SaveImage_Req_t   CAPI2_FLASH_SaveImage_Req;
	CAPI2_FLASH_SaveImage_Rsp_t   CAPI2_FLASH_SaveImage_Rsp;
	CAPI2_USB_IpcMsg_Req_t   CAPI2_USB_IpcMsg_Req;
	CAPI2_USB_IpcMsg_Rsp_t   CAPI2_USB_IpcMsg_Rsp;
	CAPI2_SYSPARM_GetIMEI_Rsp_t   CAPI2_SYSPARM_GetIMEI_Rsp;
	SYS_Sync_SetRegisteredEventMask_Req_t   SYS_Sync_SetRegisteredEventMask_Req;
	SYS_Sync_SetRegisteredEventMask_Rsp_t   SYS_Sync_SetRegisteredEventMask_Rsp;
	SYS_Sync_SetFilteredEventMask_Req_t   SYS_Sync_SetFilteredEventMask_Req;
	SYS_Sync_SetFilteredEventMask_Rsp_t   SYS_Sync_SetFilteredEventMask_Rsp;
	SYS_Sync_RegisterForMSEvent_Req_t   SYS_Sync_RegisterForMSEvent_Req;
	SYS_Sync_RegisterForMSEvent_Rsp_t   SYS_Sync_RegisterForMSEvent_Rsp;
	SYS_Sync_DeRegisterForMSEvent_Rsp_t   SYS_Sync_DeRegisterForMSEvent_Rsp;
	SYS_Sync_EnableFilterMask_Req_t   SYS_Sync_EnableFilterMask_Req;
	SYS_Sync_EnableFilterMask_Rsp_t   SYS_Sync_EnableFilterMask_Rsp;
	SYS_Sync_RegisterSetClientName_Req_t   SYS_Sync_RegisterSetClientName_Req;
	SYS_Sync_RegisterSetClientName_Rsp_t   SYS_Sync_RegisterSetClientName_Rsp;
#endif //DEVELOPMENT_SYSRPC_UNION_DECLARE

