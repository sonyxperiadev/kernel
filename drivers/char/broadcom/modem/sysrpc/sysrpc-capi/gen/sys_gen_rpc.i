

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
	{ MSG_HAL_EM_BATTMGR_BATT_LEVEL_PERCENT_REQ,_T("MSG_HAL_EM_BATTMGR_BATT_LEVEL_PERCENT_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_HAL_EM_BATTMGR_BATT_LEVEL_PERCENT_RSP,_T("MSG_HAL_EM_BATTMGR_BATT_LEVEL_PERCENT_RSP"), (xdrproc_t)xdr_CAPI2_HAL_EM_BATTMGR_BattLevelPercent_Rsp_t, sizeof( UInt16 ), 0 },
	{ MSG_HAL_EM_BATTMGR_BATT_LEVEL_REQ,_T("MSG_HAL_EM_BATTMGR_BATT_LEVEL_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_HAL_EM_BATTMGR_BATT_LEVEL_RSP,_T("MSG_HAL_EM_BATTMGR_BATT_LEVEL_RSP"), (xdrproc_t)xdr_CAPI2_HAL_EM_BATTMGR_BattLevel_Rsp_t, sizeof( UInt16 ), 0 },
	{ MSG_HAL_EM_BATTMGR_USB_CHARGER_PRESENT_REQ,_T("MSG_HAL_EM_BATTMGR_USB_CHARGER_PRESENT_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_HAL_EM_BATTMGR_USB_CHARGER_PRESENT_RSP,_T("MSG_HAL_EM_BATTMGR_USB_CHARGER_PRESENT_RSP"), (xdrproc_t)xdr_CAPI2_HAL_EM_BATTMGR_USB_ChargerPresent_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_HAL_EM_BATTMGR_WALL_CHARGER_PRESENT_REQ,_T("MSG_HAL_EM_BATTMGR_WALL_CHARGER_PRESENT_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_HAL_EM_BATTMGR_WALL_CHARGER_PRESENT_RSP,_T("MSG_HAL_EM_BATTMGR_WALL_CHARGER_PRESENT_RSP"), (xdrproc_t)xdr_CAPI2_HAL_EM_BATTMGR_WALL_ChargerPresent_Rsp_t, sizeof( Boolean ), 0 },
	{ MSG_HAL_EM_BATTMGR_RUN_BATT_MGR_REQ,_T("MSG_HAL_EM_BATTMGR_RUN_BATT_MGR_REQ"), (xdrproc_t) xdr_CAPI2_HAL_EM_BATTMGR_Run_BattMgr_Req_t, 0, 0},
	{ MSG_HAL_EM_BATTMGR_RUN_BATT_MGR_RSP,_T("MSG_HAL_EM_BATTMGR_RUN_BATT_MGR_RSP"), (xdrproc_t)xdr_CAPI2_HAL_EM_BATTMGR_Run_BattMgr_Rsp_t, sizeof( HAL_EM_BATTMGR_ErrorCode_en_t ), 0 },
	{ MSG_HAL_EM_BATTMGR_CONFIG_BATT_MGR_REQ,_T("MSG_HAL_EM_BATTMGR_CONFIG_BATT_MGR_REQ"), (xdrproc_t) xdr_CAPI2_HAL_EM_BATTMGR_Config_BattMgr_Req_t, 0, 0},
	{ MSG_HAL_EM_BATTMGR_CONFIG_BATT_MGR_RSP,_T("MSG_HAL_EM_BATTMGR_CONFIG_BATT_MGR_RSP"), (xdrproc_t)xdr_CAPI2_HAL_EM_BATTMGR_Config_BattMgr_Rsp_t, sizeof( HAL_EM_BATTMGR_ErrorCode_en_t ), 0 },
	{ MSG_HAL_EM_BATTMGR_GET_BATT_TEMP_REQ,_T("MSG_HAL_EM_BATTMGR_GET_BATT_TEMP_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_HAL_EM_BATTMGR_GET_BATT_TEMP_RSP,_T("MSG_HAL_EM_BATTMGR_GET_BATT_TEMP_RSP"), (xdrproc_t)xdr_CAPI2_HAL_EM_BATTMGR_GetBattTemp_Rsp_t, sizeof( HAL_EM_BATTMGR_Action_GetBatteryTemp_st_t ), 0 },
	{ MSG_HAL_EM_BATTMGR_GET_CHARGE_STATUS_REQ,_T("MSG_HAL_EM_BATTMGR_GET_CHARGE_STATUS_REQ"), (xdrproc_t) xdr_default_proc, 0, 0},
	{ MSG_HAL_EM_BATTMGR_GET_CHARGE_STATUS_RSP,_T("MSG_HAL_EM_BATTMGR_GET_CHARGE_STATUS_RSP"), (xdrproc_t)xdr_CAPI2_HAL_EM_BATTMGR_GetChargingStatus_Rsp_t, sizeof( EM_BATTMGR_ChargingStatus_en_t ), 0 },
	{ MSG_HAL_EM_BATTMGR_SET_COMP_REQ,_T("MSG_HAL_EM_BATTMGR_SET_COMP_REQ"), (xdrproc_t) xdr_CAPI2_HAL_EM_BATTMGR_SetComp_Req_t, 0, 0},
	{ MSG_HAL_EM_BATTMGR_SET_COMP_RSP,_T("MSG_HAL_EM_BATTMGR_SET_COMP_RSP"), (xdrproc_t)xdr_CAPI2_HAL_EM_BATTMGR_SetComp_Rsp_t, sizeof( HAL_EM_BATTMGR_ErrorCode_en_t ), 0 },
	{ MSG_MEASMGR_GET_ADC_REQ,_T("MSG_MEASMGR_GET_ADC_REQ"), (xdrproc_t) xdr_CAPI2_SYSRPC_MEASMGR_GetDataB_Adc_Req_t, 0, 0},
	{ MSG_MEASMGR_GET_ADC_RSP,_T("MSG_MEASMGR_GET_ADC_RSP"), (xdrproc_t)xdr_CAPI2_SYSRPC_MEASMGR_GetDataB_Adc_Rsp_t, sizeof( MeasMngrCnfgRsp_t ), 0 },
	{ MSG_HAL_ADC_CTRL_REQ,_T("MSG_HAL_ADC_CTRL_REQ"), (xdrproc_t) xdr_CAPI2_SYSRPC_HAL_ADC_Ctrl_Req_t, 0, 0},
	{ MSG_HAL_ADC_CTRL_RSP,_T("MSG_HAL_ADC_CTRL_RSP"), (xdrproc_t)xdr_CAPI2_SYSRPC_HAL_ADC_Ctrl_Rsp_t, sizeof( HAL_ADC_ReadConfig_st_t ), 0 },
	{ MSG_LOG_ENABLE_RANGE_REQ,_T("MSG_LOG_ENABLE_RANGE_REQ"), (xdrproc_t) xdr_SYS_Log_ApEnableCpRange_Req_t, 0, 0},
	{ MSG_LOG_ENABLE_RANGE_RSP,_T("MSG_LOG_ENABLE_RANGE_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_HAL_EM_BATTMGR_NOTIFICATION_REQ,_T("MSG_HAL_EM_BATTMGR_NOTIFICATION_REQ"), (xdrproc_t) xdr_CAPI2_HAL_EM_BATTMGR_Notification_Req_t, 0, 0},
	{ MSG_HAL_EM_BATTMGR_NOTIFICATION_RSP,_T("MSG_HAL_EM_BATTMGR_NOTIFICATION_RSP"), (xdrproc_t) xdr_default_proc, 0, 0 },
	{ MSG_HAL_EM_BATTMGR_REGISTER_EVENT_REQ,_T("MSG_HAL_EM_BATTMGR_REGISTER_EVENT_REQ"), (xdrproc_t) xdr_CAPI2_SYS_HAL_EM_BATTMGR_RegisterEventCB_Req_t, 0, 0},
	{ MSG_HAL_EM_BATTMGR_REGISTER_EVENT_RSP,_T("MSG_HAL_EM_BATTMGR_REGISTER_EVENT_RSP"), (xdrproc_t)xdr_CAPI2_SYS_HAL_EM_BATTMGR_RegisterEventCB_Rsp_t, sizeof( HAL_EM_BATTMGR_ErrorCode_en_t ), 0 },
	{ MSG_AT_MTEST_HANDLER_REQ,_T("MSG_AT_MTEST_HANDLER_REQ"), (xdrproc_t) xdr_SYS_AT_MTEST_Handler_Req_t, 0, 0},
	{ MSG_AT_MTEST_HANDLER_RSP,_T("MSG_AT_MTEST_HANDLER_RSP"), (xdrproc_t)xdr_SYS_AT_MTEST_Handler_Rsp_t, sizeof( MtestOutput_t ), 0 },
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
	CAPI2_HAL_EM_BATTMGR_BattLevelPercent_Rsp_t   CAPI2_HAL_EM_BATTMGR_BattLevelPercent_Rsp;
	CAPI2_HAL_EM_BATTMGR_BattLevel_Rsp_t   CAPI2_HAL_EM_BATTMGR_BattLevel_Rsp;
	CAPI2_HAL_EM_BATTMGR_USB_ChargerPresent_Rsp_t   CAPI2_HAL_EM_BATTMGR_USB_ChargerPresent_Rsp;
	CAPI2_HAL_EM_BATTMGR_WALL_ChargerPresent_Rsp_t   CAPI2_HAL_EM_BATTMGR_WALL_ChargerPresent_Rsp;
	CAPI2_HAL_EM_BATTMGR_Run_BattMgr_Req_t   CAPI2_HAL_EM_BATTMGR_Run_BattMgr_Req;
	CAPI2_HAL_EM_BATTMGR_Run_BattMgr_Rsp_t   CAPI2_HAL_EM_BATTMGR_Run_BattMgr_Rsp;
	CAPI2_HAL_EM_BATTMGR_Config_BattMgr_Req_t   CAPI2_HAL_EM_BATTMGR_Config_BattMgr_Req;
	CAPI2_HAL_EM_BATTMGR_Config_BattMgr_Rsp_t   CAPI2_HAL_EM_BATTMGR_Config_BattMgr_Rsp;
	CAPI2_HAL_EM_BATTMGR_GetBattTemp_Rsp_t   CAPI2_HAL_EM_BATTMGR_GetBattTemp_Rsp;
	CAPI2_HAL_EM_BATTMGR_GetChargingStatus_Rsp_t   CAPI2_HAL_EM_BATTMGR_GetChargingStatus_Rsp;
	CAPI2_HAL_EM_BATTMGR_SetComp_Req_t   CAPI2_HAL_EM_BATTMGR_SetComp_Req;
	CAPI2_HAL_EM_BATTMGR_SetComp_Rsp_t   CAPI2_HAL_EM_BATTMGR_SetComp_Rsp;
	CAPI2_SYSRPC_MEASMGR_GetDataB_Adc_Req_t   CAPI2_SYSRPC_MEASMGR_GetDataB_Adc_Req;
	CAPI2_SYSRPC_MEASMGR_GetDataB_Adc_Rsp_t   CAPI2_SYSRPC_MEASMGR_GetDataB_Adc_Rsp;
	CAPI2_SYSRPC_HAL_ADC_Ctrl_Req_t   CAPI2_SYSRPC_HAL_ADC_Ctrl_Req;
	CAPI2_SYSRPC_HAL_ADC_Ctrl_Rsp_t   CAPI2_SYSRPC_HAL_ADC_Ctrl_Rsp;
	SYS_Log_ApEnableCpRange_Req_t   SYS_Log_ApEnableCpRange_Req;
	CAPI2_HAL_EM_BATTMGR_Notification_Req_t   CAPI2_HAL_EM_BATTMGR_Notification_Req;
	CAPI2_SYS_HAL_EM_BATTMGR_RegisterEventCB_Req_t   CAPI2_SYS_HAL_EM_BATTMGR_RegisterEventCB_Req;
	CAPI2_SYS_HAL_EM_BATTMGR_RegisterEventCB_Rsp_t   CAPI2_SYS_HAL_EM_BATTMGR_RegisterEventCB_Rsp;
	SYS_AT_MTEST_Handler_Req_t   SYS_AT_MTEST_Handler_Req;
	SYS_AT_MTEST_Handler_Rsp_t   SYS_AT_MTEST_Handler_Rsp;
#endif //DEVELOPMENT_SYSRPC_UNION_DECLARE

