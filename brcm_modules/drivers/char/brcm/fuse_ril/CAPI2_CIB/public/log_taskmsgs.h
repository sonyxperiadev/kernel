typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_POWER_DOWN_CNF;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_ROAMING_STATUS;

typedef MS_RxTestParam_t *T_INTER_TASK_MSG_MEASURE_REPORT_PARAM_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_MEASURE_REPORT_PARAM_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_MEASURE_REPORT_PARAM_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_MEASURE_REPORT_PARAM_IND_HEADER Param1;
} yPDef_MSG_MEASURE_REPORT_PARAM_IND;

typedef MS_LocalElemNotifyInd_t *T_INTER_TASK_MSG_MS_LOCAL_ELEM_NOTIFY_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_MS_LOCAL_ELEM_NOTIFY_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_MS_LOCAL_ELEM_NOTIFY_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_MS_LOCAL_ELEM_NOTIFY_IND_HEADER Param1;
} yPDef_MSG_MS_LOCAL_ELEM_NOTIFY_IND;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_NETWORK_REG_STATUS;

typedef Boolean *T_INTER_TASK_MSG_CIPHER_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CIPHER_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CIPHER_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CIPHER_IND_HEADER Param1;
} yPDef_MSG_CIPHER_IND;

typedef SEARCHED_PLMN_LIST_t *T_INTER_TASK_MSG_PLMNLIST_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PLMNLIST_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PLMNLIST_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PLMNLIST_IND_HEADER Param1;
} yPDef_MSG_PLMNLIST_IND;

typedef RX_SIGNAL_INFO_CHG_t *T_INTER_TASK_MSG_RX_SIGNAL_INFO_CHG_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_RX_SIGNAL_INFO_CHG_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_RX_SIGNAL_INFO_CHG_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_RX_SIGNAL_INFO_CHG_IND_HEADER Param1;
} yPDef_MSG_RX_SIGNAL_INFO_CHG_IND;

typedef RxSignalInfo_t *T_INTER_TASK_MSG_RSSI_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_RSSI_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_RSSI_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_RSSI_IND_HEADER Param1;
} yPDef_MSG_RSSI_IND;

typedef UInt16 *T_INTER_TASK_MSG_PLMN_SELECT_CNF_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PLMN_SELECT_CNF_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PLMN_SELECT_CNF_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PLMN_SELECT_CNF_HEADER Param1;
} yPDef_MSG_PLMN_SELECT_CNF;

typedef MSRegInfo_t *T_INTER_TASK_MSG_REG_GSM_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_REG_GSM_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_REG_GSM_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_REG_GSM_IND_HEADER Param1;
} yPDef_MSG_REG_GSM_IND;

typedef MSRegInfo_t *T_INTER_TASK_MSG_REG_GPRS_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_REG_GPRS_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_REG_GPRS_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_REG_GPRS_IND_HEADER Param1;
} yPDef_MSG_REG_GPRS_IND;

typedef TimeZoneDate_t *T_INTER_TASK_MSG_DATE_TIMEZONE_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_DATE_TIMEZONE_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_DATE_TIMEZONE_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_DATE_TIMEZONE_IND_HEADER Param1;
} yPDef_MSG_DATE_TIMEZONE_IND;

typedef nitzNetworkName_t *T_INTER_TASK_MSG_NETWORK_NAME_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_NETWORK_NAME_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_NETWORK_NAME_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_NETWORK_NAME_IND_HEADER Param1;
} yPDef_MSG_NETWORK_NAME_IND;

typedef MMPlmnInfo_t *T_INTER_TASK_MSG_CELL_INFO_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CELL_INFO_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CELL_INFO_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CELL_INFO_IND_HEADER Param1;
} yPDef_MSG_CELL_INFO_IND;

typedef ServingCellInfo_t *T_INTER_TASK_MSG_SERVING_CELL_INFO_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SERVING_CELL_INFO_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SERVING_CELL_INFO_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SERVING_CELL_INFO_IND_HEADER Param1;
} yPDef_MSG_SERVING_CELL_INFO_IND;

typedef PHONECTRL_CIPHER_ALG_t *T_INTER_TASK_MSG_CIPHER_ALG_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CIPHER_ALG_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CIPHER_ALG_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CIPHER_ALG_IND_HEADER Param1;
} yPDef_MSG_CIPHER_ALG_IND;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_LSA_ID_IND;

typedef UInt16 *T_INTER_TASK_MSG_NET_UARFCN_DL_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_NET_UARFCN_DL_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_NET_UARFCN_DL_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_NET_UARFCN_DL_IND_HEADER Param1;
} yPDef_MSG_NET_UARFCN_DL_IND;

typedef PHONECTRL_NMR_t *T_INTER_TASK_MSG_NW_MEAS_RESULT_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_NW_MEAS_RESULT_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_NW_MEAS_RESULT_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_NW_MEAS_RESULT_IND_HEADER Param1;
} yPDef_MSG_NW_MEAS_RESULT_IND;

typedef CallReceiveMsg_t *T_INTER_TASK_MSG_INCOMING_CALL_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_INCOMING_CALL_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_INCOMING_CALL_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_INCOMING_CALL_IND_HEADER Param1;
} yPDef_MSG_INCOMING_CALL_IND;

typedef VoiceCallConnectMsg_t *T_INTER_TASK_MSG_VOICECALL_CONNECTED_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_VOICECALL_CONNECTED_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_VOICECALL_CONNECTED_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_VOICECALL_CONNECTED_IND_HEADER Param1;
} yPDef_MSG_VOICECALL_CONNECTED_IND;

typedef VoiceCallWaitingMsg_t *T_INTER_TASK_MSG_VOICECALL_WAITING_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_VOICECALL_WAITING_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_VOICECALL_WAITING_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_VOICECALL_WAITING_IND_HEADER Param1;
} yPDef_MSG_VOICECALL_WAITING_IND;

typedef VoiceCallActionMsg_t *T_INTER_TASK_MSG_VOICECALL_ACTION_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_VOICECALL_ACTION_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_VOICECALL_ACTION_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_VOICECALL_ACTION_RSP_HEADER Param1;
} yPDef_MSG_VOICECALL_ACTION_RSP;

typedef VoiceCallPreConnectMsg_t *T_INTER_TASK_MSG_VOICECALL_PRECONNECT_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_VOICECALL_PRECONNECT_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_VOICECALL_PRECONNECT_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_VOICECALL_PRECONNECT_IND_HEADER Param1;
} yPDef_MSG_VOICECALL_PRECONNECT_IND;

typedef VoiceCallReleaseMsg_t *T_INTER_TASK_MSG_VOICECALL_RELEASE_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_VOICECALL_RELEASE_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_VOICECALL_RELEASE_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_VOICECALL_RELEASE_IND_HEADER Param1;
} yPDef_MSG_VOICECALL_RELEASE_IND;

typedef VoiceCallReleaseMsg_t *T_INTER_TASK_MSG_VOICECALL_RELEASE_CNF_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_VOICECALL_RELEASE_CNF_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_VOICECALL_RELEASE_CNF_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_VOICECALL_RELEASE_CNF_HEADER Param1;
} yPDef_MSG_VOICECALL_RELEASE_CNF;

typedef CallStatusMsg_t *T_INTER_TASK_MSG_CALL_STATUS_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CALL_STATUS_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CALL_STATUS_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CALL_STATUS_IND_HEADER Param1;
} yPDef_MSG_CALL_STATUS_IND;

typedef UInt8 *T_INTER_TASK_MSG_CALL_CONNECTEDLINEID_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CALL_CONNECTEDLINEID_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CALL_CONNECTEDLINEID_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CALL_CONNECTEDLINEID_IND_HEADER Param1;
} yPDef_MSG_CALL_CONNECTEDLINEID_IND;

typedef ApiDtmfStatus_t *T_INTER_TASK_MSG_DTMF_STATUS_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_DTMF_STATUS_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_DTMF_STATUS_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_DTMF_STATUS_IND_HEADER Param1;
} yPDef_MSG_DTMF_STATUS_IND;

typedef DataCallStatusMsg_t *T_INTER_TASK_MSG_DATACALL_STATUS_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_DATACALL_STATUS_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_DATACALL_STATUS_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_DATACALL_STATUS_IND_HEADER Param1;
} yPDef_MSG_DATACALL_STATUS_IND;

typedef DataCallConnectMsg_t *T_INTER_TASK_MSG_DATACALL_CONNECTED_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_DATACALL_CONNECTED_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_DATACALL_CONNECTED_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_DATACALL_CONNECTED_IND_HEADER Param1;
} yPDef_MSG_DATACALL_CONNECTED_IND;

typedef DataCallReleaseMsg_t *T_INTER_TASK_MSG_DATACALL_RELEASE_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_DATACALL_RELEASE_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_DATACALL_RELEASE_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_DATACALL_RELEASE_IND_HEADER Param1;
} yPDef_MSG_DATACALL_RELEASE_IND;

typedef DataECDCLinkMsg_t *T_INTER_TASK_MSG_DATACALL_ECDC_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_DATACALL_ECDC_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_DATACALL_ECDC_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_DATACALL_ECDC_IND_HEADER Param1;
} yPDef_MSG_DATACALL_ECDC_IND;

typedef ApiClientCmdInd_t *T_INTER_TASK_MSG_API_CLIENT_CMD_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_API_CLIENT_CMD_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_API_CLIENT_CMD_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_API_CLIENT_CMD_IND_HEADER Param1;
} yPDef_MSG_API_CLIENT_CMD_IND;

typedef CallCCMMsg_t *T_INTER_TASK_MSG_CALL_CCM_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CALL_CCM_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CALL_CCM_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CALL_CCM_IND_HEADER Param1;
} yPDef_MSG_CALL_CCM_IND;

typedef CallAOCStatusMsg_t *T_INTER_TASK_MSG_CALL_AOCSTATUS_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CALL_AOCSTATUS_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CALL_AOCSTATUS_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CALL_AOCSTATUS_IND_HEADER Param1;
} yPDef_MSG_CALL_AOCSTATUS_IND;

typedef SS_NotifySs_t *T_INTER_TASK_MSG_MNCC_CLIENT_NOTIFY_SS_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_MNCC_CLIENT_NOTIFY_SS_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_MNCC_CLIENT_NOTIFY_SS_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_MNCC_CLIENT_NOTIFY_SS_IND_HEADER Param1;
} yPDef_MSG_MNCC_CLIENT_NOTIFY_SS_IND;

typedef CC_FacilityInd_t *T_INTER_TASK_MSG_MNCC_CLIENT_FACILITY_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_MNCC_CLIENT_FACILITY_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_MNCC_CLIENT_FACILITY_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_MNCC_CLIENT_FACILITY_IND_HEADER Param1;
} yPDef_MSG_MNCC_CLIENT_FACILITY_IND;

typedef SmsSimMsg_t *T_INTER_TASK_MSG_SMSPP_DELIVER_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMSPP_DELIVER_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMSPP_DELIVER_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMSPP_DELIVER_IND_HEADER Param1;
} yPDef_MSG_SMSPP_DELIVER_IND;

typedef SmsIncMsgStoredResult_t *T_INTER_TASK_MSG_SMSPP_STORED_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMSPP_STORED_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMSPP_STORED_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMSPP_STORED_IND_HEADER Param1;
} yPDef_MSG_SMSPP_STORED_IND;

typedef SmsStoredSmsCb_t *T_INTER_TASK_MSG_SMSCB_DATA_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMSCB_DATA_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMSCB_DATA_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMSCB_DATA_IND_HEADER Param1;
} yPDef_MSG_SMSCB_DATA_IND;

typedef SmsIncMsgStoredResult_t *T_INTER_TASK_MSG_SMSCB_STORED_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMSCB_STORED_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMSCB_STORED_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMSCB_STORED_IND_HEADER Param1;
} yPDef_MSG_SMSCB_STORED_IND;

typedef SmsSimMsg_t *T_INTER_TASK_MSG_SMSSR_REPORT_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMSSR_REPORT_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMSSR_REPORT_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMSSR_REPORT_IND_HEADER Param1;
} yPDef_MSG_SMSSR_REPORT_IND;

typedef SmsIncMsgStoredResult_t *T_INTER_TASK_MSG_SMSSR_STORED_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMSSR_STORED_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMSSR_STORED_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMSSR_STORED_IND_HEADER Param1;
} yPDef_MSG_SMSSR_STORED_IND;

typedef SmsSubmitRspMsg_t *T_INTER_TASK_MSG_SMS_SUBMIT_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_SUBMIT_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_SUBMIT_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_SUBMIT_RSP_HEADER Param1;
} yPDef_MSG_SMS_SUBMIT_RSP;

typedef T_MN_TP_SMS_RSP *T_INTER_TASK_MSG_SMS_COMMAND_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_COMMAND_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_COMMAND_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_COMMAND_RSP_HEADER Param1;
} yPDef_MSG_SMS_COMMAND_RSP;

typedef SmsCBMsgRspType_t *T_INTER_TASK_MSG_SMS_CB_START_STOP_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_CB_START_STOP_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_CB_START_STOP_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_CB_START_STOP_RSP_HEADER Param1;
} yPDef_MSG_SMS_CB_START_STOP_RSP;

typedef SIM_SMS_DATA_t *T_INTER_TASK_MSG_SMSCB_READ_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMSCB_READ_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMSCB_READ_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMSCB_READ_RSP_HEADER Param1;
} yPDef_MSG_SMSCB_READ_RSP;

typedef SmsSimMsg_t *T_INTER_TASK_MSG_SIM_SMS_DATA_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_SMS_DATA_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_SMS_DATA_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_SMS_DATA_RSP_HEADER Param1;
} yPDef_MSG_SIM_SMS_DATA_RSP;

typedef SmsIncMsgStoredResult_t *T_INTER_TASK_MSG_SMS_WRITE_RSP_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_WRITE_RSP_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_WRITE_RSP_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_WRITE_RSP_IND_HEADER Param1;
} yPDef_MSG_SMS_WRITE_RSP_IND;

typedef HomezoneIndData_t *T_INTER_TASK_MSG_HOMEZONE_STATUS_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_HOMEZONE_STATUS_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_HOMEZONE_STATUS_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_HOMEZONE_STATUS_IND_HEADER Param1;
} yPDef_MSG_HOMEZONE_STATUS_IND;

typedef SmsVoicemailInd_t *T_INTER_TASK_MSG_VM_WAITING_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_VM_WAITING_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_VM_WAITING_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_VM_WAITING_IND_HEADER Param1;
} yPDef_MSG_VM_WAITING_IND;

typedef SIM_EFILE_UPDATE_RESULT_t *T_INTER_TASK_MSG_VMSC_UPDATE_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_VMSC_UPDATE_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_VMSC_UPDATE_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_VMSC_UPDATE_RSP_HEADER Param1;
} yPDef_MSG_VMSC_UPDATE_RSP;

typedef smsModuleReady_t *T_INTER_TASK_MSG_SMS_READY_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_READY_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_READY_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_READY_IND_HEADER Param1;
} yPDef_MSG_SMS_READY_IND;

typedef SmsSimMsg_t *T_INTER_TASK_MSG_SMSPP_OTA_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMSPP_OTA_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMSPP_OTA_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMSPP_OTA_IND_HEADER Param1;
} yPDef_MSG_SMSPP_OTA_IND;

typedef SmsSimMsg_t *T_INTER_TASK_MSG_SMSPP_REGULAR_PUSH_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMSPP_REGULAR_PUSH_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMSPP_REGULAR_PUSH_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMSPP_REGULAR_PUSH_IND_HEADER Param1;
} yPDef_MSG_SMSPP_REGULAR_PUSH_IND;

typedef SmsSimMsg_t *T_INTER_TASK_MSG_SMSPP_SECURE_PUSH_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMSPP_SECURE_PUSH_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMSPP_SECURE_PUSH_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMSPP_SECURE_PUSH_IND_HEADER Param1;
} yPDef_MSG_SMSPP_SECURE_PUSH_IND;

typedef SmsAppSpecificData_t *T_INTER_TASK_MSG_SMSPP_APP_SPECIFIC_SMS_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMSPP_APP_SPECIFIC_SMS_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMSPP_APP_SPECIFIC_SMS_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMSPP_APP_SPECIFIC_SMS_IND_HEADER Param1;
} yPDef_MSG_SMSPP_APP_SPECIFIC_SMS_IND;

typedef SmsSimMsg_t *T_INTER_TASK_MSG_SMSPP_OTA_SYNC_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMSPP_OTA_SYNC_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMSPP_OTA_SYNC_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMSPP_OTA_SYNC_IND_HEADER Param1;
} yPDef_MSG_SMSPP_OTA_SYNC_IND;

typedef MsgType_t *T_INTER_TASK_MSG_BUILD_PBK_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_BUILD_PBK_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_BUILD_PBK_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_BUILD_PBK_REQ_HEADER Param1;
} yPDef_MSG_BUILD_PBK_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_REBUILD_ADN_PBK_REQ;

typedef PBK_FIND_ALPHA_MUL_REQ_t *T_INTER_TASK_MSG_FIND_PBK_ALPHA_MUL_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_FIND_PBK_ALPHA_MUL_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_FIND_PBK_ALPHA_MUL_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_FIND_PBK_ALPHA_MUL_REQ_HEADER Param1;
} yPDef_MSG_FIND_PBK_ALPHA_MUL_REQ;

typedef PBK_FIND_ALPHA_ONE_REQ_t *T_INTER_TASK_MSG_FIND_PBK_ALPHA_ONE_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_FIND_PBK_ALPHA_ONE_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_FIND_PBK_ALPHA_ONE_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_FIND_PBK_ALPHA_ONE_REQ_HEADER Param1;
} yPDef_MSG_FIND_PBK_ALPHA_ONE_REQ;

typedef PBK_INFO_REQ_t *T_INTER_TASK_MSG_GET_PBK_INFO_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_GET_PBK_INFO_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_GET_PBK_INFO_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_GET_PBK_INFO_REQ_HEADER Param1;
} yPDef_MSG_GET_PBK_INFO_REQ;

typedef PBK_INFO_RSP_t *T_INTER_TASK_MSG_GET_PBK_INFO_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_GET_PBK_INFO_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_GET_PBK_INFO_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_GET_PBK_INFO_RSP_HEADER Param1;
} yPDef_MSG_GET_PBK_INFO_RSP;

typedef PBK_ENTRY_READ_REQ_t *T_INTER_TASK_MSG_READ_PBK_ENTRY_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_READ_PBK_ENTRY_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_READ_PBK_ENTRY_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_READ_PBK_ENTRY_REQ_HEADER Param1;
} yPDef_MSG_READ_PBK_ENTRY_REQ;

typedef PBK_ENTRY_DATA_RSP_t *T_INTER_TASK_MSG_PBK_ENTRY_DATA_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PBK_ENTRY_DATA_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PBK_ENTRY_DATA_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PBK_ENTRY_DATA_RSP_HEADER Param1;
} yPDef_MSG_PBK_ENTRY_DATA_RSP;

typedef PBK_WRITE_ENTRY_REQ_t *T_INTER_TASK_MSG_WRT_PBK_ENTRY_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_WRT_PBK_ENTRY_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_WRT_PBK_ENTRY_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_WRT_PBK_ENTRY_REQ_HEADER Param1;
} yPDef_MSG_WRT_PBK_ENTRY_REQ;

typedef MsgType_t *T_INTER_TASK_MSG_WRT_PBK_ENTRY_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_WRT_PBK_ENTRY_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_WRT_PBK_ENTRY_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_WRT_PBK_ENTRY_RSP_HEADER Param1;
} yPDef_MSG_WRT_PBK_ENTRY_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PBK_INIT_SIM_DATA_REQ;

typedef PBK_WRITE_ENTRY_IND_t *T_INTER_TASK_MSG_WRT_PBK_ENTRY_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_WRT_PBK_ENTRY_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_WRT_PBK_ENTRY_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_WRT_PBK_ENTRY_IND_HEADER Param1;
} yPDef_MSG_WRT_PBK_ENTRY_IND;

typedef PBK_CHK_NUM_DIALLABLE_REQ_t *T_INTER_TASK_MSG_CHK_PBK_DIALLED_NUM_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CHK_PBK_DIALLED_NUM_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CHK_PBK_DIALLED_NUM_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CHK_PBK_DIALLED_NUM_REQ_HEADER Param1;
} yPDef_MSG_CHK_PBK_DIALLED_NUM_REQ;

typedef PBK_CHK_NUM_DIALLABLE_RSP_t *T_INTER_TASK_MSG_CHK_PBK_DIALLED_NUM_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CHK_PBK_DIALLED_NUM_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CHK_PBK_DIALLED_NUM_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CHK_PBK_DIALLED_NUM_RSP_HEADER Param1;
} yPDef_MSG_CHK_PBK_DIALLED_NUM_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PBK_READY_IND;

typedef USIM_PBK_ALPHA_INFO_REQ_t *T_INTER_TASK_MSG_GET_USIM_PBK_ALPHA_INFO_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_GET_USIM_PBK_ALPHA_INFO_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_GET_USIM_PBK_ALPHA_INFO_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_GET_USIM_PBK_ALPHA_INFO_REQ_HEADER Param1;
} yPDef_MSG_GET_USIM_PBK_ALPHA_INFO_REQ;

typedef USIM_PBK_ALPHA_INFO_RSP_t *T_INTER_TASK_MSG_GET_USIM_PBK_ALPHA_INFO_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_GET_USIM_PBK_ALPHA_INFO_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_GET_USIM_PBK_ALPHA_INFO_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_GET_USIM_PBK_ALPHA_INFO_RSP_HEADER Param1;
} yPDef_MSG_GET_USIM_PBK_ALPHA_INFO_RSP;

typedef USIM_PBK_READ_ALPHA_ENTRY_REQ_t *T_INTER_TASK_MSG_READ_USIM_PBK_ALPHA_ENTRY_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_READ_USIM_PBK_ALPHA_ENTRY_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_READ_USIM_PBK_ALPHA_ENTRY_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_READ_USIM_PBK_ALPHA_ENTRY_REQ_HEADER Param1;
} yPDef_MSG_READ_USIM_PBK_ALPHA_ENTRY_REQ;

typedef USIM_PBK_READ_ALPHA_ENTRY_RSP_t *T_INTER_TASK_MSG_READ_USIM_PBK_ALPHA_ENTRY_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_READ_USIM_PBK_ALPHA_ENTRY_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_READ_USIM_PBK_ALPHA_ENTRY_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_READ_USIM_PBK_ALPHA_ENTRY_RSP_HEADER Param1;
} yPDef_MSG_READ_USIM_PBK_ALPHA_ENTRY_RSP;

typedef USIM_PBK_UPDATE_ALPHA_ENTRY_REQ_t *T_INTER_TASK_MSG_WRITE_USIM_PBK_ALPHA_ENTRY_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_WRITE_USIM_PBK_ALPHA_ENTRY_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_WRITE_USIM_PBK_ALPHA_ENTRY_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_WRITE_USIM_PBK_ALPHA_ENTRY_REQ_HEADER Param1;
} yPDef_MSG_WRITE_USIM_PBK_ALPHA_ENTRY_REQ;

typedef USIM_PBK_UPDATE_ALPHA_ENTRY_RSP_t *T_INTER_TASK_MSG_WRITE_USIM_PBK_ALPHA_ENTRY_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_WRITE_USIM_PBK_ALPHA_ENTRY_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_WRITE_USIM_PBK_ALPHA_ENTRY_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_WRITE_USIM_PBK_ALPHA_ENTRY_RSP_HEADER Param1;
} yPDef_MSG_WRITE_USIM_PBK_ALPHA_ENTRY_RSP;

typedef USIM_PBK_READ_HDK_ENTRY_REQ_t *T_INTER_TASK_MSG_READ_USIM_PBK_HDK_ENTRY_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_READ_USIM_PBK_HDK_ENTRY_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_READ_USIM_PBK_HDK_ENTRY_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_READ_USIM_PBK_HDK_ENTRY_REQ_HEADER Param1;
} yPDef_MSG_READ_USIM_PBK_HDK_ENTRY_REQ;

typedef USIM_PBK_UPDATE_HDK_ENTRY_REQ_t *T_INTER_TASK_MSG_WRITE_USIM_PBK_HDK_ENTRY_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_WRITE_USIM_PBK_HDK_ENTRY_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_WRITE_USIM_PBK_HDK_ENTRY_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_WRITE_USIM_PBK_HDK_ENTRY_REQ_HEADER Param1;
} yPDef_MSG_WRITE_USIM_PBK_HDK_ENTRY_REQ;

typedef USIM_PBK_READ_HDK_ENTRY_RSP_t *T_INTER_TASK_MSG_READ_USIM_PBK_HDK_ENTRY_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_READ_USIM_PBK_HDK_ENTRY_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_READ_USIM_PBK_HDK_ENTRY_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_READ_USIM_PBK_HDK_ENTRY_RSP_HEADER Param1;
} yPDef_MSG_READ_USIM_PBK_HDK_ENTRY_RSP;

typedef USIM_PBK_UPDATE_HDK_ENTRY_RSP_t *T_INTER_TASK_MSG_WRITE_USIM_PBK_HDK_ENTRY_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_WRITE_USIM_PBK_HDK_ENTRY_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_WRITE_USIM_PBK_HDK_ENTRY_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_WRITE_USIM_PBK_HDK_ENTRY_RSP_HEADER Param1;
} yPDef_MSG_WRITE_USIM_PBK_HDK_ENTRY_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PPP_OPEN_CNF;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PPP_CLOSE_CNF;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PPP_OPEN_IND;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PPP_CLOSE_IND;

typedef DC_ReportCallStatus_t *T_INTER_TASK_MSG_DC_REPORT_CALL_STATUS_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_DC_REPORT_CALL_STATUS_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_DC_REPORT_CALL_STATUS_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_DC_REPORT_CALL_STATUS_HEADER Param1;
} yPDef_MSG_DC_REPORT_CALL_STATUS;

typedef DC_IR_ShutdownDataConnection_t *T_INTER_TASK_MSG_DC_IR_SHUTDOWN_CONNECTION_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_DC_IR_SHUTDOWN_CONNECTION_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_DC_IR_SHUTDOWN_CONNECTION_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_DC_IR_SHUTDOWN_CONNECTION_REQ_HEADER Param1;
} yPDef_MSG_DC_IR_SHUTDOWN_CONNECTION_REQ;

typedef DC_IR_StartupDataConnection_t *T_INTER_TASK_MSG_DC_IR_STARTUP_CONNECTION_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_DC_IR_STARTUP_CONNECTION_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_DC_IR_STARTUP_CONNECTION_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_DC_IR_STARTUP_CONNECTION_REQ_HEADER Param1;
} yPDef_MSG_DC_IR_STARTUP_CONNECTION_REQ;

typedef GPRSActInd_t *T_INTER_TASK_MSG_GPRS_ACTIVATE_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_GPRS_ACTIVATE_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_GPRS_ACTIVATE_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_GPRS_ACTIVATE_IND_HEADER Param1;
} yPDef_MSG_GPRS_ACTIVATE_IND;

typedef GPRSDeactInd_t *T_INTER_TASK_MSG_GPRS_DEACTIVATE_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_GPRS_DEACTIVATE_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_GPRS_DEACTIVATE_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_GPRS_DEACTIVATE_IND_HEADER Param1;
} yPDef_MSG_GPRS_DEACTIVATE_IND;

typedef Inter_ModifyContextInd_t *T_INTER_TASK_MSG_GPRS_MODIFY_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_GPRS_MODIFY_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_GPRS_MODIFY_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_GPRS_MODIFY_IND_HEADER Param1;
} yPDef_MSG_GPRS_MODIFY_IND;

typedef GPRSReActInd_t *T_INTER_TASK_MSG_GPRS_REACT_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_GPRS_REACT_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_GPRS_REACT_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_GPRS_REACT_IND_HEADER Param1;
} yPDef_MSG_GPRS_REACT_IND;

typedef PDP_SendPDPActivateSecReq_Rsp_t *T_INTER_TASK_MSG_PDP_SEC_ACTIVATION_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_SEC_ACTIVATION_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_SEC_ACTIVATION_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_SEC_ACTIVATION_RSP_HEADER Param1;
} yPDef_MSG_PDP_SEC_ACTIVATION_RSP;

typedef PDP_SendPDPActivateReq_Rsp_t *T_INTER_TASK_MSG_PDP_ACTIVATION_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_ACTIVATION_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_ACTIVATION_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_ACTIVATION_RSP_HEADER Param1;
} yPDef_MSG_PDP_ACTIVATION_RSP;

typedef PDP_SendPDPDeactivateReq_Rsp_t *T_INTER_TASK_MSG_PDP_DEACTIVATION_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_DEACTIVATION_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_DEACTIVATION_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_DEACTIVATION_RSP_HEADER Param1;
} yPDef_MSG_PDP_DEACTIVATION_RSP;

typedef PDP_PDPDeactivate_Ind_t *T_INTER_TASK_MSG_PDP_DEACTIVATION_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_DEACTIVATION_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_DEACTIVATION_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_DEACTIVATION_IND_HEADER Param1;
} yPDef_MSG_PDP_DEACTIVATION_IND;

typedef PDP_DataState_t *T_INTER_TASK_MSG_PDP_ACTIVATE_SNDCP_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_ACTIVATE_SNDCP_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_ACTIVATE_SNDCP_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_ACTIVATE_SNDCP_RSP_HEADER Param1;
} yPDef_MSG_PDP_ACTIVATE_SNDCP_RSP;

typedef PDP_SendPDPModifyReq_Rsp_t *T_INTER_TASK_MSG_PDP_MODIFICATION_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_MODIFICATION_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_MODIFICATION_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_MODIFICATION_RSP_HEADER Param1;
} yPDef_MSG_PDP_MODIFICATION_RSP;

typedef PDP_SendPDPModifyReq_Rsp_t *T_INTER_TASK_MSG_PDP_PPP_SENDCLOSE_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_PPP_SENDCLOSE_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_PPP_SENDCLOSE_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_PPP_SENDCLOSE_IND_HEADER Param1;
} yPDef_MSG_PDP_PPP_SENDCLOSE_IND;

typedef PDP_SendPDPActivatePDUReq_Rsp_t *T_INTER_TASK_MSG_PDP_ACTIVATION_PDU_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_ACTIVATION_PDU_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_ACTIVATION_PDU_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_ACTIVATION_PDU_RSP_HEADER Param1;
} yPDef_MSG_PDP_ACTIVATION_PDU_RSP;

typedef PDP_ActivateNWI_Ind_t *T_INTER_TASK_MSG_PDP_ACTIVATION_NWI_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_ACTIVATION_NWI_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_ACTIVATION_NWI_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_ACTIVATION_NWI_IND_HEADER Param1;
} yPDef_MSG_PDP_ACTIVATION_NWI_IND;

typedef PDP_ActivateSecNWI_Ind_t *T_INTER_TASK_MSG_PDP_ACTIVATION_SEC_NWI_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_ACTIVATION_SEC_NWI_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_ACTIVATION_SEC_NWI_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_ACTIVATION_SEC_NWI_IND_HEADER Param1;
} yPDef_MSG_PDP_ACTIVATION_SEC_NWI_IND;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SNPDU_IND;

typedef FsVolumeUseInd_t *T_INTER_TASK_MSG_FS_VOLUME_USE_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_FS_VOLUME_USE_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_FS_VOLUME_USE_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_FS_VOLUME_USE_IND_HEADER Param1;
} yPDef_MSG_FS_VOLUME_USE_IND;

typedef FS_RemovableMediaInsertEvent_t *T_INTER_TASK_MSG_FS_REMOVABLE_MEDIA_INSERT_EVENT_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_FS_REMOVABLE_MEDIA_INSERT_EVENT_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_FS_REMOVABLE_MEDIA_INSERT_EVENT_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_FS_REMOVABLE_MEDIA_INSERT_EVENT_HEADER Param1;
} yPDef_MSG_FS_REMOVABLE_MEDIA_INSERT_EVENT;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_USB_MSC_ACTIVE;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_USB_MSC_DEACTIVE;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_USB_ADAPTER_IND;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_INTERTASK_MSG_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_INTERTASK_MSG_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_USB_AP_INTERTASK_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_ADC_DRX_SYNC_IND;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_AUDIO_CTRL_GENERIC_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_AUDIO_CTRL_GENERIC_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_AUDIO_CTRL_DSP_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_AUDIO_CTRL_DSP_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_DEV_BCD_SUCCESSFUL;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_DEV_BCD_FAILED;

typedef SATK_EventData_t *T_INTER_TASK_MSG_SATK_EVENT_DISPLAY_TEXT_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SATK_EVENT_DISPLAY_TEXT_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SATK_EVENT_DISPLAY_TEXT_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SATK_EVENT_DISPLAY_TEXT_HEADER Param1;
} yPDef_MSG_SATK_EVENT_DISPLAY_TEXT;

typedef SATK_EventData_t *T_INTER_TASK_MSG_SATK_EVENT_GET_INKEY_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SATK_EVENT_GET_INKEY_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SATK_EVENT_GET_INKEY_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SATK_EVENT_GET_INKEY_HEADER Param1;
} yPDef_MSG_SATK_EVENT_GET_INKEY;

typedef SATK_EventData_t *T_INTER_TASK_MSG_SATK_EVENT_GET_INPUT_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SATK_EVENT_GET_INPUT_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SATK_EVENT_GET_INPUT_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SATK_EVENT_GET_INPUT_HEADER Param1;
} yPDef_MSG_SATK_EVENT_GET_INPUT;

typedef SATK_EventData_t *T_INTER_TASK_MSG_SATK_EVENT_PLAY_TONE_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SATK_EVENT_PLAY_TONE_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SATK_EVENT_PLAY_TONE_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SATK_EVENT_PLAY_TONE_HEADER Param1;
} yPDef_MSG_SATK_EVENT_PLAY_TONE;

typedef SATK_EventData_t *T_INTER_TASK_MSG_SATK_EVENT_SELECT_ITEM_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SATK_EVENT_SELECT_ITEM_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SATK_EVENT_SELECT_ITEM_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SATK_EVENT_SELECT_ITEM_HEADER Param1;
} yPDef_MSG_SATK_EVENT_SELECT_ITEM;

typedef SATK_EventData_t *T_INTER_TASK_MSG_SATK_EVENT_SEND_SS_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SATK_EVENT_SEND_SS_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SATK_EVENT_SEND_SS_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SATK_EVENT_SEND_SS_HEADER Param1;
} yPDef_MSG_SATK_EVENT_SEND_SS;

typedef SATK_EventData_t *T_INTER_TASK_MSG_SATK_EVENT_SEND_USSD_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SATK_EVENT_SEND_USSD_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SATK_EVENT_SEND_USSD_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SATK_EVENT_SEND_USSD_HEADER Param1;
} yPDef_MSG_SATK_EVENT_SEND_USSD;

typedef SATK_EventData_t *T_INTER_TASK_MSG_SATK_EVENT_SETUP_CALL_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SATK_EVENT_SETUP_CALL_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SATK_EVENT_SETUP_CALL_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SATK_EVENT_SETUP_CALL_HEADER Param1;
} yPDef_MSG_SATK_EVENT_SETUP_CALL;

typedef SATK_EventData_t *T_INTER_TASK_MSG_SATK_EVENT_SETUP_MENU_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SATK_EVENT_SETUP_MENU_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SATK_EVENT_SETUP_MENU_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SATK_EVENT_SETUP_MENU_HEADER Param1;
} yPDef_MSG_SATK_EVENT_SETUP_MENU;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SATK_EVENT_MENU_SELECTION;

typedef SATK_EventData_t *T_INTER_TASK_MSG_SATK_EVENT_SEND_DTMF_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SATK_EVENT_SEND_DTMF_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SATK_EVENT_SEND_DTMF_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SATK_EVENT_SEND_DTMF_HEADER Param1;
} yPDef_MSG_SATK_EVENT_SEND_DTMF;

typedef SATK_EventData_t *T_INTER_TASK_MSG_SATK_EVENT_REFRESH_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SATK_EVENT_REFRESH_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SATK_EVENT_REFRESH_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SATK_EVENT_REFRESH_HEADER Param1;
} yPDef_MSG_SATK_EVENT_REFRESH;

typedef SATK_EventData_t *T_INTER_TASK_MSG_SATK_EVENT_SEND_SHORT_MSG_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SATK_EVENT_SEND_SHORT_MSG_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SATK_EVENT_SEND_SHORT_MSG_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SATK_EVENT_SEND_SHORT_MSG_HEADER Param1;
} yPDef_MSG_SATK_EVENT_SEND_SHORT_MSG;

typedef SATK_EventData_t *T_INTER_TASK_MSG_SATK_EVENT_STK_SESSION_END_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SATK_EVENT_STK_SESSION_END_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SATK_EVENT_STK_SESSION_END_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SATK_EVENT_STK_SESSION_END_HEADER Param1;
} yPDef_MSG_SATK_EVENT_STK_SESSION_END;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SATK_EVENT_CALL_CONTROL_RSP;

typedef SATK_EventData_t *T_INTER_TASK_MSG_SATK_EVENT_LAUNCH_BROWSER_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SATK_EVENT_LAUNCH_BROWSER_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SATK_EVENT_LAUNCH_BROWSER_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SATK_EVENT_LAUNCH_BROWSER_HEADER Param1;
} yPDef_MSG_SATK_EVENT_LAUNCH_BROWSER;

typedef SATK_EventData_t *T_INTER_TASK_MSG_SATK_EVENT_IDLEMODE_TEXT_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SATK_EVENT_IDLEMODE_TEXT_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SATK_EVENT_IDLEMODE_TEXT_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SATK_EVENT_IDLEMODE_TEXT_HEADER Param1;
} yPDef_MSG_SATK_EVENT_IDLEMODE_TEXT;

typedef SATK_EventData_t *T_INTER_TASK_MSG_SATK_EVENT_PROV_LOCAL_LANG_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SATK_EVENT_PROV_LOCAL_LANG_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SATK_EVENT_PROV_LOCAL_LANG_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SATK_EVENT_PROV_LOCAL_LANG_HEADER Param1;
} yPDef_MSG_SATK_EVENT_PROV_LOCAL_LANG;

typedef StkDataService_t *T_INTER_TASK_MSG_SATK_EVENT_DATA_SERVICE_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SATK_EVENT_DATA_SERVICE_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SATK_EVENT_DATA_SERVICE_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SATK_EVENT_DATA_SERVICE_REQ_HEADER Param1;
} yPDef_MSG_SATK_EVENT_DATA_SERVICE_REQ;

typedef SATK_EventData_t *T_INTER_TASK_MSG_SATK_EVENT_ACTIVATE_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SATK_EVENT_ACTIVATE_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SATK_EVENT_ACTIVATE_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SATK_EVENT_ACTIVATE_HEADER Param1;
} yPDef_MSG_SATK_EVENT_ACTIVATE;

typedef SATK_EventData_t *T_INTER_TASK_MSG_SATK_EVENT_PROV_LOCAL_DATE_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SATK_EVENT_PROV_LOCAL_DATE_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SATK_EVENT_PROV_LOCAL_DATE_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SATK_EVENT_PROV_LOCAL_DATE_HEADER Param1;
} yPDef_MSG_SATK_EVENT_PROV_LOCAL_DATE;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SATKCC_USSD_CC_NOT_ALLOWED;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SATKCC_USSD_CC_INIT_IND;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SATKCC_USSD_CC_ALLOWED;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SATKCC_USSD_CC_MODIFIED;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SATKCC_SMS_CC;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SATKCC_SS_CC_NOT_ALLOWED;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SATKCC_SS_CC_INIT_IND;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SATKCC_SS_CC_ALLOWED;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SATKCC_SS_CC_MODIFIED;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_STK_SEND_EMERGENCY_CALL_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_STK_REPORT_CALL_RECEIVED;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_STK_REPORT_CALL_STATUS;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_STK_REPORT_CALL_CONNECTED_ID;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_STK_REPORT_CALL_ACTION_RESULT;

typedef StkReportCallRelease_t *T_INTER_TASK_MSG_STK_REPORT_CALL_RELEASE_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_STK_REPORT_CALL_RELEASE_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_STK_REPORT_CALL_RELEASE_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_STK_REPORT_CALL_RELEASE_HEADER Param1;
} yPDef_MSG_STK_REPORT_CALL_RELEASE;

typedef StkReportSuppSvcStatus_t *T_INTER_TASK_MSG_STK_REPORT_SUPP_SVC_STATUS_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_STK_REPORT_SUPP_SVC_STATUS_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_STK_REPORT_SUPP_SVC_STATUS_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_STK_REPORT_SUPP_SVC_STATUS_HEADER Param1;
} yPDef_MSG_STK_REPORT_SUPP_SVC_STATUS;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_STK_REPORT_CALL_AOC_STATUS;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_STK_REPORT_CALL_CCM;

typedef SS_CallNotification_t *T_INTER_TASK_MSG_SS_CALL_NOTIFICATION_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SS_CALL_NOTIFICATION_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SS_CALL_NOTIFICATION_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SS_CALL_NOTIFICATION_HEADER Param1;
} yPDef_MSG_SS_CALL_NOTIFICATION;

typedef SS_CallNotification_t *T_INTER_TASK_MSG_SS_NOTIFY_CLOSED_USER_GROUP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SS_NOTIFY_CLOSED_USER_GROUP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SS_NOTIFY_CLOSED_USER_GROUP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SS_NOTIFY_CLOSED_USER_GROUP_HEADER Param1;
} yPDef_MSG_SS_NOTIFY_CLOSED_USER_GROUP;

typedef SS_CallNotification_t *T_INTER_TASK_MSG_SS_NOTIFY_EXTENDED_CALL_TRANSFER_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SS_NOTIFY_EXTENDED_CALL_TRANSFER_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SS_NOTIFY_EXTENDED_CALL_TRANSFER_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SS_NOTIFY_EXTENDED_CALL_TRANSFER_HEADER Param1;
} yPDef_MSG_SS_NOTIFY_EXTENDED_CALL_TRANSFER;

typedef SS_CallNotification_t *T_INTER_TASK_MSG_SS_NOTIFY_CALLING_NAME_PRESENT_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SS_NOTIFY_CALLING_NAME_PRESENT_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SS_NOTIFY_CALLING_NAME_PRESENT_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SS_NOTIFY_CALLING_NAME_PRESENT_HEADER Param1;
} yPDef_MSG_SS_NOTIFY_CALLING_NAME_PRESENT;

typedef NetworkCause_t *T_INTER_TASK_MSG_SS_CALL_FORWARD_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SS_CALL_FORWARD_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SS_CALL_FORWARD_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SS_CALL_FORWARD_RSP_HEADER Param1;
} yPDef_MSG_SS_CALL_FORWARD_RSP;

typedef CallForwardStatus_t *T_INTER_TASK_MSG_SS_CALL_FORWARD_STATUS_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SS_CALL_FORWARD_STATUS_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SS_CALL_FORWARD_STATUS_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SS_CALL_FORWARD_STATUS_RSP_HEADER Param1;
} yPDef_MSG_SS_CALL_FORWARD_STATUS_RSP;

typedef NetworkCause_t *T_INTER_TASK_MSG_SS_CALL_BARRING_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SS_CALL_BARRING_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SS_CALL_BARRING_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SS_CALL_BARRING_RSP_HEADER Param1;
} yPDef_MSG_SS_CALL_BARRING_RSP;

typedef CallBarringStatus_t *T_INTER_TASK_MSG_SS_CALL_BARRING_STATUS_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SS_CALL_BARRING_STATUS_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SS_CALL_BARRING_STATUS_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SS_CALL_BARRING_STATUS_RSP_HEADER Param1;
} yPDef_MSG_SS_CALL_BARRING_STATUS_RSP;

typedef NetworkCause_t *T_INTER_TASK_MSG_SS_CALL_BARRING_PWD_CHANGE_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SS_CALL_BARRING_PWD_CHANGE_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SS_CALL_BARRING_PWD_CHANGE_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SS_CALL_BARRING_PWD_CHANGE_RSP_HEADER Param1;
} yPDef_MSG_SS_CALL_BARRING_PWD_CHANGE_RSP;

typedef MsgType_t *T_INTER_TASK_MSG_SS_CALLING_LINE_ID_STATUS_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SS_CALLING_LINE_ID_STATUS_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SS_CALLING_LINE_ID_STATUS_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SS_CALLING_LINE_ID_STATUS_RSP_HEADER Param1;
} yPDef_MSG_SS_CALLING_LINE_ID_STATUS_RSP;

typedef NetworkCause_t *T_INTER_TASK_MSG_SS_CALL_WAITING_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SS_CALL_WAITING_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SS_CALL_WAITING_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SS_CALL_WAITING_RSP_HEADER Param1;
} yPDef_MSG_SS_CALL_WAITING_RSP;

typedef SS_ActivationStatus_t *T_INTER_TASK_MSG_SS_CALL_WAITING_STATUS_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SS_CALL_WAITING_STATUS_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SS_CALL_WAITING_STATUS_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SS_CALL_WAITING_STATUS_RSP_HEADER Param1;
} yPDef_MSG_SS_CALL_WAITING_STATUS_RSP;

typedef MsgType_t *T_INTER_TASK_MSG_SS_CONNECTED_LINE_STATUS_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SS_CONNECTED_LINE_STATUS_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SS_CONNECTED_LINE_STATUS_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SS_CONNECTED_LINE_STATUS_RSP_HEADER Param1;
} yPDef_MSG_SS_CONNECTED_LINE_STATUS_RSP;

typedef MsgType_t *T_INTER_TASK_MSG_SS_CALLING_LINE_RESTRICTION_STATUS_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SS_CALLING_LINE_RESTRICTION_STATUS_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SS_CALLING_LINE_RESTRICTION_STATUS_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SS_CALLING_LINE_RESTRICTION_STATUS_RSP_HEADER Param1;
} yPDef_MSG_SS_CALLING_LINE_RESTRICTION_STATUS_RSP;

typedef MsgType_t *T_INTER_TASK_MSG_SS_CONNECTED_LINE_RESTRICTION_STATUS_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SS_CONNECTED_LINE_RESTRICTION_STATUS_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SS_CONNECTED_LINE_RESTRICTION_STATUS_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SS_CONNECTED_LINE_RESTRICTION_STATUS_RSP_HEADER Param1;
} yPDef_MSG_SS_CONNECTED_LINE_RESTRICTION_STATUS_RSP;

typedef MsgType_t *T_INTER_TASK_MSG_SS_CALLING_NAME_PRESENT_STATUS_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SS_CALLING_NAME_PRESENT_STATUS_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SS_CALLING_NAME_PRESENT_STATUS_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SS_CALLING_NAME_PRESENT_STATUS_RSP_HEADER Param1;
} yPDef_MSG_SS_CALLING_NAME_PRESENT_STATUS_RSP;

typedef SS_IntParSetInd_t *T_INTER_TASK_MSG_SS_INTERNAL_PARAM_SET_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SS_INTERNAL_PARAM_SET_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SS_INTERNAL_PARAM_SET_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SS_INTERNAL_PARAM_SET_IND_HEADER Param1;
} yPDef_MSG_SS_INTERNAL_PARAM_SET_IND;

typedef SS_String_t *T_INTER_TASK_MSG_CLIENT_SATK_SS_SRV_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CLIENT_SATK_SS_SRV_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CLIENT_SATK_SS_SRV_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CLIENT_SATK_SS_SRV_REQ_HEADER Param1;
} yPDef_MSG_CLIENT_SATK_SS_SRV_REQ;

typedef SS_SrvReq_t *T_INTER_TASK_MSG_CLIENT_SATK_USSD_SRV_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CLIENT_SATK_USSD_SRV_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CLIENT_SATK_USSD_SRV_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CLIENT_SATK_USSD_SRV_REQ_HEADER Param1;
} yPDef_MSG_CLIENT_SATK_USSD_SRV_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CLIENT_MNSS_SS_SRV_REQ;

typedef SS_SrvReq_t *T_INTER_TASK_MSG_CLIENT_MNSS_SS_DATA_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CLIENT_MNSS_SS_DATA_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CLIENT_MNSS_SS_DATA_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CLIENT_MNSS_SS_DATA_REQ_HEADER Param1;
} yPDef_MSG_CLIENT_MNSS_SS_DATA_REQ;

typedef SS_SrvReq_t *T_INTER_TASK_MSG_CLIENT_MNSS_SS_REL_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CLIENT_MNSS_SS_REL_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CLIENT_MNSS_SS_REL_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CLIENT_MNSS_SS_REL_REQ_HEADER Param1;
} yPDef_MSG_CLIENT_MNSS_SS_REL_REQ;

typedef SS_SrvRsp_t *T_INTER_TASK_MSG_MNSS_CLIENT_SS_SRV_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_MNSS_CLIENT_SS_SRV_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_MNSS_CLIENT_SS_SRV_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_MNSS_CLIENT_SS_SRV_RSP_HEADER Param1;
} yPDef_MSG_MNSS_CLIENT_SS_SRV_RSP;

typedef SS_SrvRel_t *T_INTER_TASK_MSG_MNSS_CLIENT_SS_SRV_REL_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_MNSS_CLIENT_SS_SRV_REL_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_MNSS_CLIENT_SS_SRV_REL_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_MNSS_CLIENT_SS_SRV_REL_HEADER Param1;
} yPDef_MSG_MNSS_CLIENT_SS_SRV_REL;

typedef SS_SrvInd_t *T_INTER_TASK_MSG_MNSS_CLIENT_SS_SRV_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_MNSS_CLIENT_SS_SRV_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_MNSS_CLIENT_SS_SRV_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_MNSS_CLIENT_SS_SRV_IND_HEADER Param1;
} yPDef_MSG_MNSS_CLIENT_SS_SRV_IND;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_MNSS_SATK_SS_SRV_RSP;

typedef STK_SsSrvRel_t *T_INTER_TASK_MSG_MNSS_SATK_SS_SRV_REL_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_MNSS_SATK_SS_SRV_REL_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_MNSS_SATK_SS_SRV_REL_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_MNSS_SATK_SS_SRV_REL_HEADER Param1;
} yPDef_MSG_MNSS_SATK_SS_SRV_REL;

typedef LCS_SrvInd_t *T_INTER_TASK_MSG_MNSS_CLIENT_LCS_SRV_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_MNSS_CLIENT_LCS_SRV_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_MNSS_CLIENT_LCS_SRV_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_MNSS_CLIENT_LCS_SRV_IND_HEADER Param1;
} yPDef_MSG_MNSS_CLIENT_LCS_SRV_IND;

typedef LCS_SrvRsp_t *T_INTER_TASK_MSG_MNSS_CLIENT_LCS_SRV_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_MNSS_CLIENT_LCS_SRV_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_MNSS_CLIENT_LCS_SRV_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_MNSS_CLIENT_LCS_SRV_RSP_HEADER Param1;
} yPDef_MSG_MNSS_CLIENT_LCS_SRV_RSP;

typedef SS_SrvRel_t *T_INTER_TASK_MSG_MNSS_CLIENT_LCS_SRV_REL_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_MNSS_CLIENT_LCS_SRV_REL_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_MNSS_CLIENT_LCS_SRV_REL_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_MNSS_CLIENT_LCS_SRV_REL_HEADER Param1;
} yPDef_MSG_MNSS_CLIENT_LCS_SRV_REL;

typedef USSDataInfo_t *T_INTER_TASK_MSG_USSD_DATA_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_USSD_DATA_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_USSD_DATA_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_USSD_DATA_IND_HEADER Param1;
} yPDef_MSG_USSD_DATA_IND;

typedef USSDataInfo_t *T_INTER_TASK_MSG_USSD_DATA_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_USSD_DATA_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_USSD_DATA_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_USSD_DATA_RSP_HEADER Param1;
} yPDef_MSG_USSD_DATA_RSP;

typedef CallIndex_t *T_INTER_TASK_MSG_USSD_SESSION_END_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_USSD_SESSION_END_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_USSD_SESSION_END_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_USSD_SESSION_END_IND_HEADER Param1;
} yPDef_MSG_USSD_SESSION_END_IND;

typedef StkReportCallStatus_t *T_INTER_TASK_MSG_USSD_CALLINDEX_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_USSD_CALLINDEX_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_USSD_CALLINDEX_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_USSD_CALLINDEX_IND_HEADER Param1;
} yPDef_MSG_USSD_CALLINDEX_IND;

typedef CallIndex_t *T_INTER_TASK_MSG_USSD_CALLRELEASE_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_USSD_CALLRELEASE_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_USSD_CALLRELEASE_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_USSD_CALLRELEASE_IND_HEADER Param1;
} yPDef_MSG_USSD_CALLRELEASE_IND;

typedef SIM_DFILE_INFO_t *T_INTER_TASK_MSG_SIM_DFILE_INFO_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_DFILE_INFO_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_DFILE_INFO_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_DFILE_INFO_RSP_HEADER Param1;
} yPDef_MSG_SIM_DFILE_INFO_RSP;

typedef SIM_EFILE_INFO_t *T_INTER_TASK_MSG_SIM_EFILE_INFO_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_EFILE_INFO_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_EFILE_INFO_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_EFILE_INFO_RSP_HEADER Param1;
} yPDef_MSG_SIM_EFILE_INFO_RSP;

typedef SIM_EFILE_DATA_t *T_INTER_TASK_MSG_SIM_EFILE_DATA_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_EFILE_DATA_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_EFILE_DATA_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_EFILE_DATA_RSP_HEADER Param1;
} yPDef_MSG_SIM_EFILE_DATA_RSP;

typedef SIM_EFILE_UPDATE_RESULT_t *T_INTER_TASK_MSG_SIM_EFILE_UPDATE_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_EFILE_UPDATE_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_EFILE_UPDATE_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_EFILE_UPDATE_RSP_HEADER Param1;
} yPDef_MSG_SIM_EFILE_UPDATE_RSP;

typedef PIN_ATTEMPT_RESULT_t *T_INTER_TASK_MSG_SIM_PIN_ATTEMPT_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PIN_ATTEMPT_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PIN_ATTEMPT_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PIN_ATTEMPT_RSP_HEADER Param1;
} yPDef_MSG_SIM_PIN_ATTEMPT_RSP;

typedef SIM_SEEK_RECORD_DATA_t *T_INTER_TASK_MSG_SIM_SEEK_REC_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_SEEK_REC_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_SEEK_REC_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_SEEK_REC_RSP_HEADER Param1;
} yPDef_MSG_SIM_SEEK_REC_RSP;

typedef SIMAccess_t *T_INTER_TASK_MSG_SIM_SET_FDN_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_SET_FDN_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_SET_FDN_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_SET_FDN_RSP_HEADER Param1;
} yPDef_MSG_SIM_SET_FDN_RSP;

typedef SIMAccess_t *T_INTER_TASK_MSG_SIM_ENABLE_CHV_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_ENABLE_CHV_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_ENABLE_CHV_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_ENABLE_CHV_RSP_HEADER Param1;
} yPDef_MSG_SIM_ENABLE_CHV_RSP;

typedef SIMAccess_t *T_INTER_TASK_MSG_SIM_CHANGE_CHV_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_CHANGE_CHV_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_CHANGE_CHV_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_CHANGE_CHV_RSP_HEADER Param1;
} yPDef_MSG_SIM_CHANGE_CHV_RSP;

typedef SIMAccess_t *T_INTER_TASK_MSG_SIM_VERIFY_CHV_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_VERIFY_CHV_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_VERIFY_CHV_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_VERIFY_CHV_RSP_HEADER Param1;
} yPDef_MSG_SIM_VERIFY_CHV_RSP;

typedef SIMAccess_t *T_INTER_TASK_MSG_SIM_UNBLOCK_CHV_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_UNBLOCK_CHV_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_UNBLOCK_CHV_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_UNBLOCK_CHV_RSP_HEADER Param1;
} yPDef_MSG_SIM_UNBLOCK_CHV_RSP;

typedef SIM_PBK_INFO_t *T_INTER_TASK_MSG_SIM_PBK_INFO_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PBK_INFO_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PBK_INFO_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PBK_INFO_RSP_HEADER Param1;
} yPDef_MSG_SIM_PBK_INFO_RSP;

typedef SIM_PBK_DATA_t *T_INTER_TASK_MSG_SIM_PBK_DATA_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PBK_DATA_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PBK_DATA_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PBK_DATA_RSP_HEADER Param1;
} yPDef_MSG_SIM_PBK_DATA_RSP;

typedef SIM_PBK_WRITE_RESULT_t *T_INTER_TASK_MSG_SIM_PBK_WRITE_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PBK_WRITE_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PBK_WRITE_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PBK_WRITE_RSP_HEADER Param1;
} yPDef_MSG_SIM_PBK_WRITE_RSP;

typedef SIMAccess_t *T_INTER_TASK_MSG_SIM_SET_BDN_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_SET_BDN_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_SET_BDN_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_SET_BDN_RSP_HEADER Param1;
} yPDef_MSG_SIM_SET_BDN_RSP;

typedef SIM_PLMN_NUM_OF_ENTRY_t *T_INTER_TASK_MSG_SIM_PLMN_NUM_OF_ENTRY_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PLMN_NUM_OF_ENTRY_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PLMN_NUM_OF_ENTRY_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PLMN_NUM_OF_ENTRY_RSP_HEADER Param1;
} yPDef_MSG_SIM_PLMN_NUM_OF_ENTRY_RSP;

typedef SIM_PLMN_ENTRY_DATA_t *T_INTER_TASK_MSG_SIM_PLMN_ENTRY_DATA_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PLMN_ENTRY_DATA_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PLMN_ENTRY_DATA_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PLMN_ENTRY_DATA_RSP_HEADER Param1;
} yPDef_MSG_SIM_PLMN_ENTRY_DATA_RSP;

typedef SIM_MUL_PLMN_ENTRY_UPDATE_t *T_INTER_TASK_MSG_SIM_MUL_PLMN_ENTRY_UPDATE_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_MUL_PLMN_ENTRY_UPDATE_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_MUL_PLMN_ENTRY_UPDATE_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_MUL_PLMN_ENTRY_UPDATE_RSP_HEADER Param1;
} yPDef_MSG_SIM_MUL_PLMN_ENTRY_UPDATE_RSP;

typedef SIM_MUL_REC_DATA_t *T_INTER_TASK_MSG_SIM_MUL_REC_DATA_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_MUL_REC_DATA_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_MUL_REC_DATA_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_MUL_REC_DATA_RSP_HEADER Param1;
} yPDef_MSG_SIM_MUL_REC_DATA_RSP;

typedef SIM_MAX_ACM_t *T_INTER_TASK_MSG_SIM_MAX_ACM_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_MAX_ACM_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_MAX_ACM_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_MAX_ACM_RSP_HEADER Param1;
} yPDef_MSG_SIM_MAX_ACM_RSP;

typedef SIM_ACM_VALUE_t *T_INTER_TASK_MSG_SIM_ACM_VALUE_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_ACM_VALUE_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_ACM_VALUE_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_ACM_VALUE_RSP_HEADER Param1;
} yPDef_MSG_SIM_ACM_VALUE_RSP;

typedef SIMAccess_t *T_INTER_TASK_MSG_SIM_ACM_UPDATE_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_ACM_UPDATE_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_ACM_UPDATE_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_ACM_UPDATE_RSP_HEADER Param1;
} yPDef_MSG_SIM_ACM_UPDATE_RSP;

typedef MsgType_t *T_INTER_TASK_MSG_SIM_ACM_MAX_UPDATE_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_ACM_MAX_UPDATE_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_ACM_MAX_UPDATE_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_ACM_MAX_UPDATE_RSP_HEADER Param1;
} yPDef_MSG_SIM_ACM_MAX_UPDATE_RSP;

typedef MsgType_t *T_INTER_TASK_MSG_SIM_ACM_INCREASE_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_ACM_INCREASE_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_ACM_INCREASE_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_ACM_INCREASE_RSP_HEADER Param1;
} yPDef_MSG_SIM_ACM_INCREASE_RSP;

typedef SIM_SVC_PROV_NAME_t *T_INTER_TASK_MSG_SIM_SVC_PROV_NAME_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_SVC_PROV_NAME_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_SVC_PROV_NAME_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_SVC_PROV_NAME_RSP_HEADER Param1;
} yPDef_MSG_SIM_SVC_PROV_NAME_RSP;

typedef SIM_PUCT_DATA_t *T_INTER_TASK_MSG_SIM_PUCT_DATA_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PUCT_DATA_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PUCT_DATA_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PUCT_DATA_RSP_HEADER Param1;
} yPDef_MSG_SIM_PUCT_DATA_RSP;

typedef MsgType_t *T_INTER_TASK_MSG_SIM_PUCT_UPDATE_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PUCT_UPDATE_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PUCT_UPDATE_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PUCT_UPDATE_RSP_HEADER Param1;
} yPDef_MSG_SIM_PUCT_UPDATE_RSP;

typedef SIM_GENERIC_APDU_RES_INFO_t *T_INTER_TASK_MSG_SIM_POWER_ON_OFF_CARD_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_POWER_ON_OFF_CARD_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_POWER_ON_OFF_CARD_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_POWER_ON_OFF_CARD_RSP_HEADER Param1;
} yPDef_MSG_SIM_POWER_ON_OFF_CARD_RSP;

typedef SIM_GENERIC_APDU_ATR_INFO_t *T_INTER_TASK_MSG_SIM_GET_RAW_ATR_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_GET_RAW_ATR_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_GET_RAW_ATR_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_GET_RAW_ATR_RSP_HEADER Param1;
} yPDef_MSG_SIM_GET_RAW_ATR_RSP;

typedef SIM_GENERIC_APDU_XFER_RSP_t *T_INTER_TASK_MSG_SIM_SEND_GENERIC_APDU_CMD_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_SEND_GENERIC_APDU_CMD_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_SEND_GENERIC_APDU_CMD_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_SEND_GENERIC_APDU_CMD_RSP_HEADER Param1;
} yPDef_MSG_SIM_SEND_GENERIC_APDU_CMD_RSP;

typedef SIM_OPEN_SOCKET_RES_t *T_INTER_TASK_MSG_SIM_OPEN_SOCKET_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_OPEN_SOCKET_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_OPEN_SOCKET_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_OPEN_SOCKET_RSP_HEADER Param1;
} yPDef_MSG_SIM_OPEN_SOCKET_RSP;

typedef SIM_SELECT_APPLI_RES_t *T_INTER_TASK_MSG_SIM_SELECT_APPLI_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_SELECT_APPLI_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_SELECT_APPLI_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_SELECT_APPLI_RSP_HEADER Param1;
} yPDef_MSG_SIM_SELECT_APPLI_RSP;

typedef SIM_DEACTIVATE_APPLI_RES_t *T_INTER_TASK_MSG_SIM_DEACTIVATE_APPLI_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_DEACTIVATE_APPLI_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_DEACTIVATE_APPLI_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_DEACTIVATE_APPLI_RSP_HEADER Param1;
} yPDef_MSG_SIM_DEACTIVATE_APPLI_RSP;

typedef SIM_CLOSE_SOCKET_RES_t *T_INTER_TASK_MSG_SIM_CLOSE_SOCKET_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_CLOSE_SOCKET_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_CLOSE_SOCKET_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_CLOSE_SOCKET_RSP_HEADER Param1;
} yPDef_MSG_SIM_CLOSE_SOCKET_RSP;

typedef SIM_SMS_UPDATE_RESULT_t *T_INTER_TASK_MSG_SIM_SMS_WRITE_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_SMS_WRITE_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_SMS_WRITE_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_SMS_WRITE_RSP_HEADER Param1;
} yPDef_MSG_SIM_SMS_WRITE_RSP;

typedef SIM_SMS_UPDATE_RESULT_t *T_INTER_TASK_MSG_SIM_SMS_STATUS_UPD_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_SMS_STATUS_UPD_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_SMS_STATUS_UPD_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_SMS_STATUS_UPD_RSP_HEADER Param1;
} yPDef_MSG_SIM_SMS_STATUS_UPD_RSP;

typedef SIM_SMS_UPDATE_RESULT_t *T_INTER_TASK_MSG_SIM_SMS_SCA_UPD_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_SMS_SCA_UPD_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_SMS_SCA_UPD_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_SMS_SCA_UPD_RSP_HEADER Param1;
} yPDef_MSG_SIM_SMS_SCA_UPD_RSP;

typedef SIM_SMS_PARAM_t *T_INTER_TASK_MSG_SIM_SMS_PARAM_DATA_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_SMS_PARAM_DATA_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_SMS_PARAM_DATA_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_SMS_PARAM_DATA_RSP_HEADER Param1;
} yPDef_MSG_SIM_SMS_PARAM_DATA_RSP;

typedef SIM_SMS_TP_MR_t *T_INTER_TASK_MSG_SIM_SMS_TP_MR_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_SMS_TP_MR_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_SMS_TP_MR_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_SMS_TP_MR_RSP_HEADER Param1;
} yPDef_MSG_SIM_SMS_TP_MR_RSP;

typedef SIM_RESTRICTED_ACCESS_DATA_t *T_INTER_TASK_MSG_SIM_RESTRICTED_ACCESS_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_RESTRICTED_ACCESS_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_RESTRICTED_ACCESS_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_RESTRICTED_ACCESS_RSP_HEADER Param1;
} yPDef_MSG_SIM_RESTRICTED_ACCESS_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIM_GENERIC_ACCESS_RSP;

typedef USIM_FILE_UPDATE_RSP_t *T_INTER_TASK_MSG_SIM_SET_EST_SERV_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_SET_EST_SERV_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_SET_EST_SERV_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_SET_EST_SERV_RSP_HEADER Param1;
} yPDef_MSG_SIM_SET_EST_SERV_RSP;

typedef USIM_FILE_UPDATE_RSP_t *T_INTER_TASK_MSG_SIM_UPDATE_ONE_APN_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_UPDATE_ONE_APN_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_UPDATE_ONE_APN_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_UPDATE_ONE_APN_RSP_HEADER Param1;
} yPDef_MSG_SIM_UPDATE_ONE_APN_RSP;

typedef USIM_FILE_UPDATE_RSP_t *T_INTER_TASK_MSG_SIM_DELETE_ALL_APN_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_DELETE_ALL_APN_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_DELETE_ALL_APN_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_DELETE_ALL_APN_RSP_HEADER Param1;
} yPDef_MSG_SIM_DELETE_ALL_APN_RSP;

typedef ISIM_ACTIVATE_RSP_t *T_INTER_TASK_MSG_ISIM_ACTIVATE_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_ISIM_ACTIVATE_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_ISIM_ACTIVATE_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_ISIM_ACTIVATE_RSP_HEADER Param1;
} yPDef_MSG_ISIM_ACTIVATE_RSP;

typedef ISIM_AUTHEN_AKA_RSP_t *T_INTER_TASK_MSG_ISIM_AUTHEN_AKA_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_ISIM_AUTHEN_AKA_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_ISIM_AUTHEN_AKA_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_ISIM_AUTHEN_AKA_RSP_HEADER Param1;
} yPDef_MSG_ISIM_AUTHEN_AKA_RSP;

typedef ISIM_AUTHEN_HTTP_RSP_t *T_INTER_TASK_MSG_ISIM_AUTHEN_HTTP_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_ISIM_AUTHEN_HTTP_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_ISIM_AUTHEN_HTTP_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_ISIM_AUTHEN_HTTP_RSP_HEADER Param1;
} yPDef_MSG_ISIM_AUTHEN_HTTP_RSP;

typedef ISIM_AUTHEN_GBA_BOOT_RSP_t *T_INTER_TASK_MSG_ISIM_AUTHEN_GBA_BOOT_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_ISIM_AUTHEN_GBA_BOOT_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_ISIM_AUTHEN_GBA_BOOT_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_ISIM_AUTHEN_GBA_BOOT_RSP_HEADER Param1;
} yPDef_MSG_ISIM_AUTHEN_GBA_BOOT_RSP;

typedef ISIM_AUTHEN_GBA_NAF_RSP_t *T_INTER_TASK_MSG_ISIM_AUTHEN_GBA_NAF_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_ISIM_AUTHEN_GBA_NAF_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_ISIM_AUTHEN_GBA_NAF_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_ISIM_AUTHEN_GBA_NAF_RSP_HEADER Param1;
} yPDef_MSG_ISIM_AUTHEN_GBA_NAF_RSP;

typedef SIM_DETECTION_t *T_INTER_TASK_MSG_SIM_DETECTION_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_DETECTION_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_DETECTION_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_DETECTION_IND_HEADER Param1;
} yPDef_MSG_SIM_DETECTION_IND;

typedef MsgType_t *T_INTER_TASK_MSG_SIM_RESET_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_RESET_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_RESET_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_RESET_IND_HEADER Param1;
} yPDef_MSG_SIM_RESET_IND;

typedef UInt16 *T_INTER_TASK_MSG_SIM_MMI_SETUP_EVENT_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_MMI_SETUP_EVENT_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_MMI_SETUP_EVENT_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_MMI_SETUP_EVENT_IND_HEADER Param1;
} yPDef_MSG_SIM_MMI_SETUP_EVENT_IND;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIM_FATAL_ERROR_IND;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIM_CACHED_DATA_READY_IND;

typedef Int32 *T_INTER_TASK_MSG_GL_GPS_USERCOMMAND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_GL_GPS_USERCOMMAND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_GL_GPS_USERCOMMAND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_GL_GPS_USERCOMMAND_HEADER Param1;
} yPDef_MSG_GL_GPS_USERCOMMAND;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_GL_GPS_TIMEREVENT;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_GL_GPS_HANDLERXDATA;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_LCS_POSITION_FIX_READY_IND;

typedef LcsState_t *T_INTER_TASK_MSG_LCS_STATUS_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_LCS_STATUS_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_LCS_STATUS_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_LCS_STATUS_IND_HEADER Param1;
} yPDef_MSG_LCS_STATUS_IND;

typedef LcsStartFixResult_t *T_INTER_TASK_MSG_LCS_START_FIX_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_LCS_START_FIX_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_LCS_START_FIX_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_LCS_START_FIX_IND_HEADER Param1;
} yPDef_MSG_LCS_START_FIX_IND;

typedef LcsPosReqResult_t *T_INTER_TASK_MSG_LCS_REQ_RESULT_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_LCS_REQ_RESULT_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_LCS_REQ_RESULT_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_LCS_REQ_RESULT_IND_HEADER Param1;
} yPDef_MSG_LCS_REQ_RESULT_IND;

typedef LcsSuplNotificationData_t *T_INTER_TASK_MSG_LCS_SUPL_NOTIFICATION_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_LCS_SUPL_NOTIFICATION_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_LCS_SUPL_NOTIFICATION_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_LCS_SUPL_NOTIFICATION_IND_HEADER Param1;
} yPDef_MSG_LCS_SUPL_NOTIFICATION_IND;

typedef LcsNmeaData_t *T_INTER_TASK_MSG_LCS_NMEA_READY_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_LCS_NMEA_READY_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_LCS_NMEA_READY_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_LCS_NMEA_READY_IND_HEADER Param1;
} yPDef_MSG_LCS_NMEA_READY_IND;

typedef LcsSuplConnection_t *T_INTER_TASK_MSG_LCS_SUPL_CONNECT_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_LCS_SUPL_CONNECT_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_LCS_SUPL_CONNECT_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_LCS_SUPL_CONNECT_REQ_HEADER Param1;
} yPDef_MSG_LCS_SUPL_CONNECT_REQ;

typedef LcsSuplSessionInfo_t *T_INTER_TASK_MSG_LCS_SUPL_DISCONNECT_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_LCS_SUPL_DISCONNECT_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_LCS_SUPL_DISCONNECT_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_LCS_SUPL_DISCONNECT_REQ_HEADER Param1;
} yPDef_MSG_LCS_SUPL_DISCONNECT_REQ;

typedef LcsSuplData_t *T_INTER_TASK_MSG_LCS_SUPL_INIT_HMAC_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_LCS_SUPL_INIT_HMAC_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_LCS_SUPL_INIT_HMAC_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_LCS_SUPL_INIT_HMAC_REQ_HEADER Param1;
} yPDef_MSG_LCS_SUPL_INIT_HMAC_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_LCS_FACTORY_TEST_IND;

typedef LcsSuplCommData_t *T_INTER_TASK_MSG_LCS_SUPL_WRITE_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_LCS_SUPL_WRITE_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_LCS_SUPL_WRITE_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_LCS_SUPL_WRITE_REQ_HEADER Param1;
} yPDef_MSG_LCS_SUPL_WRITE_REQ;

typedef LcsPosInfo_t *T_INTER_TASK_MSG_LCS_POSITION_INFO_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_LCS_POSITION_INFO_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_LCS_POSITION_INFO_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_LCS_POSITION_INFO_IND_HEADER Param1;
} yPDef_MSG_LCS_POSITION_INFO_IND;

typedef LcsPosDetail_t *T_INTER_TASK_MSG_LCS_POSITION_DETAIL_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_LCS_POSITION_DETAIL_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_LCS_POSITION_DETAIL_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_LCS_POSITION_DETAIL_IND_HEADER Param1;
} yPDef_MSG_LCS_POSITION_DETAIL_IND;

typedef LcsLtoConnection_t *T_INTER_TASK_MSG_LCS_LTO_CONNECT_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_LCS_LTO_CONNECT_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_LCS_LTO_CONNECT_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_LCS_LTO_CONNECT_REQ_HEADER Param1;
} yPDef_MSG_LCS_LTO_CONNECT_REQ;

typedef LcsLtoSessionInfo_t *T_INTER_TASK_MSG_LCS_LTO_DISCONNECT_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_LCS_LTO_DISCONNECT_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_LCS_LTO_DISCONNECT_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_LCS_LTO_DISCONNECT_REQ_HEADER Param1;
} yPDef_MSG_LCS_LTO_DISCONNECT_REQ;

typedef LcsLtoCommData_t *T_INTER_TASK_MSG_LCS_LTO_WRITE_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_LCS_LTO_WRITE_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_LCS_LTO_WRITE_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_LCS_LTO_WRITE_REQ_HEADER Param1;
} yPDef_MSG_LCS_LTO_WRITE_REQ;

typedef LcsResult_t *T_INTER_TASK_MSG_LCS_LTO_SYNC_RESULT_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_LCS_LTO_SYNC_RESULT_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_LCS_LTO_SYNC_RESULT_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_LCS_LTO_SYNC_RESULT_IND_HEADER Param1;
} yPDef_MSG_LCS_LTO_SYNC_RESULT_IND;

typedef LcsDeviceState_t *T_INTER_TASK_MSG_LCS_DEVICE_STATE_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_LCS_DEVICE_STATE_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_LCS_DEVICE_STATE_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_LCS_DEVICE_STATE_IND_HEADER Param1;
} yPDef_MSG_LCS_DEVICE_STATE_IND;

typedef LcsMsgData_t *T_INTER_TASK_MSG_LCS_RRLP_DATA_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_LCS_RRLP_DATA_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_LCS_RRLP_DATA_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_LCS_RRLP_DATA_IND_HEADER Param1;
} yPDef_MSG_LCS_RRLP_DATA_IND;

typedef ClientInfo_t *T_INTER_TASK_MSG_LCS_RRLP_RESET_POS_STORED_INFO_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_LCS_RRLP_RESET_POS_STORED_INFO_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_LCS_RRLP_RESET_POS_STORED_INFO_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_LCS_RRLP_RESET_POS_STORED_INFO_IND_HEADER Param1;
} yPDef_MSG_LCS_RRLP_RESET_POS_STORED_INFO_IND;

typedef LcsMsgData_t *T_INTER_TASK_MSG_LCS_RRC_ASSISTANCE_DATA_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_LCS_RRC_ASSISTANCE_DATA_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_LCS_RRC_ASSISTANCE_DATA_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_LCS_RRC_ASSISTANCE_DATA_IND_HEADER Param1;
} yPDef_MSG_LCS_RRC_ASSISTANCE_DATA_IND;

typedef LcsRrcMeasurement_t *T_INTER_TASK_MSG_LCS_RRC_MEASUREMENT_CTRL_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_LCS_RRC_MEASUREMENT_CTRL_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_LCS_RRC_MEASUREMENT_CTRL_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_LCS_RRC_MEASUREMENT_CTRL_IND_HEADER Param1;
} yPDef_MSG_LCS_RRC_MEASUREMENT_CTRL_IND;

typedef LcsRrcBroadcastSysInfo_t *T_INTER_TASK_MSG_LCS_RRC_BROADCAST_SYS_INFO_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_LCS_RRC_BROADCAST_SYS_INFO_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_LCS_RRC_BROADCAST_SYS_INFO_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_LCS_RRC_BROADCAST_SYS_INFO_IND_HEADER Param1;
} yPDef_MSG_LCS_RRC_BROADCAST_SYS_INFO_IND;

typedef LcsRrcUeState_t *T_INTER_TASK_MSG_LCS_RRC_UE_STATE_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_LCS_RRC_UE_STATE_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_LCS_RRC_UE_STATE_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_LCS_RRC_UE_STATE_IND_HEADER Param1;
} yPDef_MSG_LCS_RRC_UE_STATE_IND;

typedef ClientInfo_t *T_INTER_TASK_MSG_LCS_RRC_STOP_MEASUREMENT_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_LCS_RRC_STOP_MEASUREMENT_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_LCS_RRC_STOP_MEASUREMENT_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_LCS_RRC_STOP_MEASUREMENT_IND_HEADER Param1;
} yPDef_MSG_LCS_RRC_STOP_MEASUREMENT_IND;

typedef ClientInfo_t *T_INTER_TASK_MSG_LCS_RRC_RESET_POS_STORED_INFO_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_LCS_RRC_RESET_POS_STORED_INFO_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_LCS_RRC_RESET_POS_STORED_INFO_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_LCS_RRC_RESET_POS_STORED_INFO_IND_HEADER Param1;
} yPDef_MSG_LCS_RRC_RESET_POS_STORED_INFO_IND;

typedef LcsFttResult_t *T_INTER_TASK_MSG_LCS_FTT_SYNC_RESULT_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_LCS_FTT_SYNC_RESULT_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_LCS_FTT_SYNC_RESULT_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_LCS_FTT_SYNC_RESULT_IND_HEADER Param1;
} yPDef_MSG_LCS_FTT_SYNC_RESULT_IND;

typedef ClientInfo_t *T_INTER_TASK_MSG_LCS_WLAN_DEV_OPEN_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_LCS_WLAN_DEV_OPEN_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_LCS_WLAN_DEV_OPEN_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_LCS_WLAN_DEV_OPEN_REQ_HEADER Param1;
} yPDef_MSG_LCS_WLAN_DEV_OPEN_REQ;

typedef ClientInfo_t *T_INTER_TASK_MSG_LCS_WLAN_DEV_CLOSE_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_LCS_WLAN_DEV_CLOSE_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_LCS_WLAN_DEV_CLOSE_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_LCS_WLAN_DEV_CLOSE_REQ_HEADER Param1;
} yPDef_MSG_LCS_WLAN_DEV_CLOSE_REQ;

typedef ClientInfo_t *T_INTER_TASK_MSG_LCS_WLAN_SCAN_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_LCS_WLAN_SCAN_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_LCS_WLAN_SCAN_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_LCS_WLAN_SCAN_REQ_HEADER Param1;
} yPDef_MSG_LCS_WLAN_SCAN_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_LCS_APP_START_FIX_REQ;

typedef LcsHandle_t *T_INTER_TASK_MSG_LCS_APP_STOP_FIX_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_LCS_APP_STOP_FIX_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_LCS_APP_STOP_FIX_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_LCS_APP_STOP_FIX_REQ_HEADER Param1;
} yPDef_MSG_LCS_APP_STOP_FIX_REQ;

typedef UInt16 *T_INTER_TASK_MSG_LCS_APP_SET_FIX_TIMEOUT_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_LCS_APP_SET_FIX_TIMEOUT_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_LCS_APP_SET_FIX_TIMEOUT_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_LCS_APP_SET_FIX_TIMEOUT_REQ_HEADER Param1;
} yPDef_MSG_LCS_APP_SET_FIX_TIMEOUT_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_LCS_APP_SET_FIX_PERIOD_REQ;

typedef LcsFixMode_t *T_INTER_TASK_MSG_LCS_APP_SET_FIX_MODE_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_LCS_APP_SET_FIX_MODE_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_LCS_APP_SET_FIX_MODE_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_LCS_APP_SET_FIX_MODE_REQ_HEADER Param1;
} yPDef_MSG_LCS_APP_SET_FIX_MODE_REQ;

typedef UInt32 *T_INTER_TASK_MSG_LCS_APP_SET_ACCURACY_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_LCS_APP_SET_ACCURACY_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_LCS_APP_SET_ACCURACY_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_LCS_APP_SET_ACCURACY_REQ_HEADER Param1;
} yPDef_MSG_LCS_APP_SET_ACCURACY_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_LCS_APP_STOP_SERVICE_REQ;

typedef LcsSuplAnswer_t *T_INTER_TASK_MSG_LCS_APP_SUPL_NOTIFICATION_RESP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_LCS_APP_SUPL_NOTIFICATION_RESP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_LCS_APP_SUPL_NOTIFICATION_RESP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_LCS_APP_SUPL_NOTIFICATION_RESP_HEADER Param1;
} yPDef_MSG_LCS_APP_SUPL_NOTIFICATION_RESP;

typedef LcsSuplConnectResp_t *T_INTER_TASK_MSG_LCS_APP_SUPL_CONNECT_RESP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_LCS_APP_SUPL_CONNECT_RESP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_LCS_APP_SUPL_CONNECT_RESP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_LCS_APP_SUPL_CONNECT_RESP_HEADER Param1;
} yPDef_MSG_LCS_APP_SUPL_CONNECT_RESP;

typedef LcsSuplCommData_t *T_INTER_TASK_MSG_LCS_APP_SUPL_DATA_AVAILABLE_NTF_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_LCS_APP_SUPL_DATA_AVAILABLE_NTF_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_LCS_APP_SUPL_DATA_AVAILABLE_NTF_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_LCS_APP_SUPL_DATA_AVAILABLE_NTF_HEADER Param1;
} yPDef_MSG_LCS_APP_SUPL_DATA_AVAILABLE_NTF;

typedef LcsSuplSessionInfo_t *T_INTER_TASK_MSG_LCS_APP_SUPL_DISCONNECT_NTF_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_LCS_APP_SUPL_DISCONNECT_NTF_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_LCS_APP_SUPL_DISCONNECT_NTF_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_LCS_APP_SUPL_DISCONNECT_NTF_HEADER Param1;
} yPDef_MSG_LCS_APP_SUPL_DISCONNECT_NTF;

typedef LcsSuplSessionInfo_t *T_INTER_TASK_MSG_LCS_APP_SUPL_INIT_HMAC_RESP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_LCS_APP_SUPL_INIT_HMAC_RESP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_LCS_APP_SUPL_INIT_HMAC_RESP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_LCS_APP_SUPL_INIT_HMAC_RESP_HEADER Param1;
} yPDef_MSG_LCS_APP_SUPL_INIT_HMAC_RESP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_LCS_APP_POWER_CONTROL;

typedef LcsDataItem_t *T_INTER_TASK_MSG_LCS_APP_HANDLE_SUPL_INIT_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_LCS_APP_HANDLE_SUPL_INIT_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_LCS_APP_HANDLE_SUPL_INIT_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_LCS_APP_HANDLE_SUPL_INIT_HEADER Param1;
} yPDef_MSG_LCS_APP_HANDLE_SUPL_INIT;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_LCS_APP_RESET_STORED_GPS_DATA;

typedef LcsLtoSessionInfo_t *T_INTER_TASK_MSG_LCS_APP_LTO_CONNECT_RESP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_LCS_APP_LTO_CONNECT_RESP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_LCS_APP_LTO_CONNECT_RESP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_LCS_APP_LTO_CONNECT_RESP_HEADER Param1;
} yPDef_MSG_LCS_APP_LTO_CONNECT_RESP;

typedef LcsLtoCommData_t *T_INTER_TASK_MSG_LCS_APP_LTO_DATA_AVAILABLE_NTF_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_LCS_APP_LTO_DATA_AVAILABLE_NTF_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_LCS_APP_LTO_DATA_AVAILABLE_NTF_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_LCS_APP_LTO_DATA_AVAILABLE_NTF_HEADER Param1;
} yPDef_MSG_LCS_APP_LTO_DATA_AVAILABLE_NTF;

typedef LcsLtoSessionInfo_t *T_INTER_TASK_MSG_LCS_APP_LTO_DISCONNECT_NTF_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_LCS_APP_LTO_DISCONNECT_NTF_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_LCS_APP_LTO_DISCONNECT_NTF_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_LCS_APP_LTO_DISCONNECT_NTF_HEADER Param1;
} yPDef_MSG_LCS_APP_LTO_DISCONNECT_NTF;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_LCS_APP_LTO_SYNC_REQ;

typedef LcsPalSuplLocId_t *T_INTER_TASK_MSG_LCS_APP_PAL_SUPL_LOC_ID_RESP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_LCS_APP_PAL_SUPL_LOC_ID_RESP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_LCS_APP_PAL_SUPL_LOC_ID_RESP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_LCS_APP_PAL_SUPL_LOC_ID_RESP_HEADER Param1;
} yPDef_MSG_LCS_APP_PAL_SUPL_LOC_ID_RESP;

typedef GLLBS_RIL_NOTIFY_DATA *T_INTER_TASK_MSG_LCS_APP_PAL_LBS_RIL_RESP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_LCS_APP_PAL_LBS_RIL_RESP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_LCS_APP_PAL_LBS_RIL_RESP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_LCS_APP_PAL_LBS_RIL_RESP_HEADER Param1;
} yPDef_MSG_LCS_APP_PAL_LBS_RIL_RESP;

typedef TimerID_t *T_INTER_TASK_MSG_LCS_APP_SUPL_TIMER_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_LCS_APP_SUPL_TIMER_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_LCS_APP_SUPL_TIMER_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_LCS_APP_SUPL_TIMER_IND_HEADER Param1;
} yPDef_MSG_LCS_APP_SUPL_TIMER_IND;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_LCS_APP_LBS_CONTROL_REQ;

typedef LcsWlanDevState_t *T_INTER_TASK_MSG_LCS_APP_WLAN_DEV_STATE_NTF_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_LCS_APP_WLAN_DEV_STATE_NTF_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_LCS_APP_WLAN_DEV_STATE_NTF_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_LCS_APP_WLAN_DEV_STATE_NTF_HEADER Param1;
} yPDef_MSG_LCS_APP_WLAN_DEV_STATE_NTF;

typedef GLLBS_WLAN_NOTIFY_DATA *T_INTER_TASK_MSG_LCS_APP_WLAN_SCAN_RESULT_NTF_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_LCS_APP_WLAN_SCAN_RESULT_NTF_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_LCS_APP_WLAN_SCAN_RESULT_NTF_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_LCS_APP_WLAN_SCAN_RESULT_NTF_HEADER Param1;
} yPDef_MSG_LCS_APP_WLAN_SCAN_RESULT_NTF;

typedef TimerID_t *T_INTER_TASK_MSG_LCS_APP_LBS_TIMER_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_LCS_APP_LBS_TIMER_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_LCS_APP_LBS_TIMER_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_LCS_APP_LBS_TIMER_IND_HEADER Param1;
} yPDef_MSG_LCS_APP_LBS_TIMER_IND;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_LCS_APP_WLAN_SCANDATA_READY_IND;

typedef TlsClientRsp_t *T_INTER_TASK_MSG_TLS_OPEN_CONNECTION_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_TLS_OPEN_CONNECTION_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_TLS_OPEN_CONNECTION_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_TLS_OPEN_CONNECTION_RSP_HEADER Param1;
} yPDef_MSG_TLS_OPEN_CONNECTION_RSP;

typedef TlsClientRsp_t *T_INTER_TASK_MSG_TLS_CONNECTION_STATE_NTF_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_TLS_CONNECTION_STATE_NTF_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_TLS_CONNECTION_STATE_NTF_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_TLS_CONNECTION_STATE_NTF_HEADER Param1;
} yPDef_MSG_TLS_CONNECTION_STATE_NTF;

typedef TlsClientRsp_t *T_INTER_TASK_MSG_TLS_RECVD_APP_DATA_NTF_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_TLS_RECVD_APP_DATA_NTF_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_TLS_RECVD_APP_DATA_NTF_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_TLS_RECVD_APP_DATA_NTF_HEADER Param1;
} yPDef_MSG_TLS_RECVD_APP_DATA_NTF;

typedef TlsClientHdl_t *T_INTER_TASK_MSG_TLS_INIT_HANDSHAKE_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_TLS_INIT_HANDSHAKE_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_TLS_INIT_HANDSHAKE_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_TLS_INIT_HANDSHAKE_HEADER Param1;
} yPDef_MSG_TLS_INIT_HANDSHAKE;

typedef TlsClientHdl_t *T_INTER_TASK_MSG_TLS_RECV_DATA_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_TLS_RECV_DATA_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_TLS_RECV_DATA_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_TLS_RECV_DATA_HEADER Param1;
} yPDef_MSG_TLS_RECV_DATA;

typedef TlsClientHdl_t *T_INTER_TASK_MSG_TLS_CLOSE_CONNECTION_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_TLS_CLOSE_CONNECTION_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_TLS_CLOSE_CONNECTION_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_TLS_CLOSE_CONNECTION_HEADER Param1;
} yPDef_MSG_TLS_CLOSE_CONNECTION;

typedef TlsSecurityContextHdl_t *T_INTER_TASK_MSG_TLS_UNLOAD_SECURITY_CONTEXT_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_TLS_UNLOAD_SECURITY_CONTEXT_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_TLS_UNLOAD_SECURITY_CONTEXT_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_TLS_UNLOAD_SECURITY_CONTEXT_HEADER Param1;
} yPDef_MSG_TLS_UNLOAD_SECURITY_CONTEXT;

typedef UInt32 *T_INTER_TASK_MSG_FTPC_CONNECTED_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_FTPC_CONNECTED_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_FTPC_CONNECTED_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_FTPC_CONNECTED_IND_HEADER Param1;
} yPDef_MSG_FTPC_CONNECTED_IND;

typedef FTPCStatus_t *T_INTER_TASK_MSG_FTPC_CONNECT_FAIL_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_FTPC_CONNECT_FAIL_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_FTPC_CONNECT_FAIL_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_FTPC_CONNECT_FAIL_HEADER Param1;
} yPDef_MSG_FTPC_CONNECT_FAIL;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_FTPC_DISCONNECTED_IND;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_FTPC_READY_IND;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_FTPC_LOGGED_IN_IND;

typedef FTPCStatus_t *T_INTER_TASK_MSG_FTPC_LOGIN_FAIL_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_FTPC_LOGIN_FAIL_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_FTPC_LOGIN_FAIL_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_FTPC_LOGIN_FAIL_HEADER Param1;
} yPDef_MSG_FTPC_LOGIN_FAIL;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_FTPC_GET_IND;

typedef FTPCStatus_t *T_INTER_TASK_MSG_FTPC_GET_FAIL_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_FTPC_GET_FAIL_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_FTPC_GET_FAIL_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_FTPC_GET_FAIL_HEADER Param1;
} yPDef_MSG_FTPC_GET_FAIL;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_FTPC_PUT_IND;

typedef FTPCStatus_t *T_INTER_TASK_MSG_FTPC_PUT_FAIL_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_FTPC_PUT_FAIL_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_FTPC_PUT_FAIL_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_FTPC_PUT_FAIL_HEADER Param1;
} yPDef_MSG_FTPC_PUT_FAIL;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_FTPC_CHDIR_IND;

typedef FTPCStatus_t *T_INTER_TASK_MSG_FTPC_CHDIR_FAIL_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_FTPC_CHDIR_FAIL_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_FTPC_CHDIR_FAIL_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_FTPC_CHDIR_FAIL_HEADER Param1;
} yPDef_MSG_FTPC_CHDIR_FAIL;

typedef T_NW_EMERGENCY_NUMBER_LIST *T_INTER_TASK_MSG_ECC_LIST_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_ECC_LIST_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_ECC_LIST_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_ECC_LIST_IND_HEADER Param1;
} yPDef_MSG_ECC_LIST_IND;

typedef MSUe3gStatusInd_t *T_INTER_TASK_MSG_UE_3G_STATUS_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_UE_3G_STATUS_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_UE_3G_STATUS_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_UE_3G_STATUS_IND_HEADER Param1;
} yPDef_MSG_UE_3G_STATUS_IND;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_POWER_DOWN_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_POWER_UP_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_RESET_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_STATISTIC_INFO;

typedef ATCOPSData_t *T_INTER_TASK_MSG_AT_COPS_CMD_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_AT_COPS_CMD_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_AT_COPS_CMD_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_AT_COPS_CMD_HEADER Param1;
} yPDef_MSG_AT_COPS_CMD;

typedef ATCGATTData_t *T_INTER_TASK_MSG_AT_CGATT_CMD_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_AT_CGATT_CMD_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_AT_CGATT_CMD_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_AT_CGATT_CMD_HEADER Param1;
} yPDef_MSG_AT_CGATT_CMD;

typedef ATCGACTData_t *T_INTER_TASK_MSG_AT_CGACT_CMD_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_AT_CGACT_CMD_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_AT_CGACT_CMD_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_AT_CGACT_CMD_HEADER Param1;
} yPDef_MSG_AT_CGACT_CMD;

typedef ATCCmeError_t *T_INTER_TASK_MSG_AT_CME_ERROR_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_AT_CME_ERROR_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_AT_CME_ERROR_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_AT_CME_ERROR_HEADER Param1;
} yPDef_MSG_AT_CME_ERROR;

typedef ATCGCMODData_t *T_INTER_TASK_MSG_AT_CGCMOD_CMD_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_AT_CGCMOD_CMD_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_AT_CGCMOD_CMD_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_AT_CGCMOD_CMD_HEADER Param1;
} yPDef_MSG_AT_CGCMOD_CMD;

typedef CallTimerID_t *T_INTER_TASK_MSG_AT_CALL_TIMER_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_AT_CALL_TIMER_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_AT_CALL_TIMER_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_AT_CALL_TIMER_HEADER Param1;
} yPDef_MSG_AT_CALL_TIMER;

typedef UInt8 *T_INTER_TASK_MSG_AT_CALL_ABORT_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_AT_CALL_ABORT_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_AT_CALL_ABORT_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_AT_CALL_ABORT_HEADER Param1;
} yPDef_MSG_AT_CALL_ABORT;

typedef UInt8 *T_INTER_TASK_MSG_AT_ESC_DATA_CALL_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_AT_ESC_DATA_CALL_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_AT_ESC_DATA_CALL_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_AT_ESC_DATA_CALL_HEADER Param1;
} yPDef_MSG_AT_ESC_DATA_CALL;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_AT_HANDLE_CHLD_CMD;

typedef TimerID_t *T_INTER_TASK_MSG_AT_CMMS_TIMER_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_AT_CMMS_TIMER_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_AT_CMMS_TIMER_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_AT_CMMS_TIMER_IND_HEADER Param1;
} yPDef_MSG_AT_CMMS_TIMER_IND;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_AT_CMD_STR;

typedef Boolean *T_INTER_TASK_MSG_AT_LINE_STATE_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_AT_LINE_STATE_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_AT_LINE_STATE_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_AT_LINE_STATE_IND_HEADER Param1;
} yPDef_MSG_AT_LINE_STATE_IND;

typedef MODULE_READY_STATUS_t *T_INTER_TASK_MSG_MS_READY_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_MS_READY_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_MS_READY_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_MS_READY_IND_HEADER Param1;
} yPDef_MSG_MS_READY_IND;

typedef UInt8 *T_INTER_TASK_MSG_CALL_MONITOR_STATUS_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CALL_MONITOR_STATUS_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CALL_MONITOR_STATUS_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CALL_MONITOR_STATUS_HEADER Param1;
} yPDef_MSG_CALL_MONITOR_STATUS;

typedef AtCmdInfo_t *T_INTER_TASK_MSG_AT_COMMAND_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_AT_COMMAND_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_AT_COMMAND_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_AT_COMMAND_REQ_HEADER Param1;
} yPDef_MSG_AT_COMMAND_REQ;

typedef AtCmdInfo_t *T_INTER_TASK_MSG_AT_COMMAND_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_AT_COMMAND_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_AT_COMMAND_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_AT_COMMAND_IND_HEADER Param1;
} yPDef_MSG_AT_COMMAND_IND;

typedef at_cmgsCmd_t *T_INTER_TASK_MSG_SMS_COMPOSE_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_COMPOSE_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_COMPOSE_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_COMPOSE_REQ_HEADER Param1;
} yPDef_MSG_SMS_COMPOSE_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_USR_DATA_IND;

typedef SmsParmCheckRsp_t *T_INTER_TASK_MSG_SMS_PARM_CHECK_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_PARM_CHECK_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_PARM_CHECK_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_PARM_CHECK_RSP_HEADER Param1;
} yPDef_MSG_SMS_PARM_CHECK_RSP;

typedef SmsReportInd_t *T_INTER_TASK_MSG_SMS_REPORT_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_REPORT_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_REPORT_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_REPORT_IND_HEADER Param1;
} yPDef_MSG_SMS_REPORT_IND;

typedef T_MN_TP_SMS_RSP *T_INTER_TASK_MSG_SMS_MEM_AVAIL_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_MEM_AVAIL_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_MEM_AVAIL_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_MEM_AVAIL_RSP_HEADER Param1;
} yPDef_MSG_SMS_MEM_AVAIL_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_CB_START_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_CBMI_WRITE_REQ;

typedef SmsGetMsgFromBuffer_t *T_INTER_TASK_MSG_SMS_GET_MSG_FROM_BUFFER_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_GET_MSG_FROM_BUFFER_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_GET_MSG_FROM_BUFFER_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_GET_MSG_FROM_BUFFER_HEADER Param1;
} yPDef_MSG_SMS_GET_MSG_FROM_BUFFER;

typedef SmsMsgToBeStored_t *T_INTER_TASK_MSG_SMS_STORE_SMS_MSG_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_STORE_SMS_MSG_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_STORE_SMS_MSG_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_STORE_SMS_MSG_REQ_HEADER Param1;
} yPDef_MSG_SMS_STORE_SMS_MSG_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SUBMIT_RETRY_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_EVALUATE_HOMEZONE_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_HOMEZONE_CB_TIMEOUT_IND;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_CMD_PARM_CHECK_RSP;

typedef TimerID_t *T_INTER_TASK_MSG_SMS_CNMA_TIMER_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_CNMA_TIMER_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_CNMA_TIMER_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_CNMA_TIMER_IND_HEADER Param1;
} yPDef_MSG_SMS_CNMA_TIMER_IND;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_ATC_TIMEOUT_IND;

typedef SS_SsApiReq_t *T_INTER_TASK_MSG_SS_CALL_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SS_CALL_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SS_CALL_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SS_CALL_REQ_HEADER Param1;
} yPDef_MSG_SS_CALL_REQ;

typedef SsCallReqFail_t *T_INTER_TASK_MSG_SS_CALL_REQ_FAIL_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SS_CALL_REQ_FAIL_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SS_CALL_REQ_FAIL_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SS_CALL_REQ_FAIL_HEADER Param1;
} yPDef_MSG_SS_CALL_REQ_FAIL;

typedef UssdCallReq_t *T_INTER_TASK_MSG_USSD_CALL_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_USSD_CALL_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_USSD_CALL_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_USSD_CALL_REQ_HEADER Param1;
} yPDef_MSG_USSD_CALL_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_ATTACH_IND;

typedef Inter_DetachInd_t *T_INTER_TASK_MSG_DETACH_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_DETACH_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_DETACH_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_DETACH_IND_HEADER Param1;
} yPDef_MSG_DETACH_IND;

typedef Inter_ActivateInd_t *T_INTER_TASK_MSG_ACTIVATE_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_ACTIVATE_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_ACTIVATE_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_ACTIVATE_IND_HEADER Param1;
} yPDef_MSG_ACTIVATE_IND;

typedef Inter_DeactivateInd_t *T_INTER_TASK_MSG_DEACTIVATE_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_DEACTIVATE_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_DEACTIVATE_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_DEACTIVATE_IND_HEADER Param1;
} yPDef_MSG_DEACTIVATE_IND;

typedef Inter_AttachCnf_t *T_INTER_TASK_MSG_ATTACH_CNF_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_ATTACH_CNF_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_ATTACH_CNF_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_ATTACH_CNF_HEADER Param1;
} yPDef_MSG_ATTACH_CNF;

typedef Inter_DetachCnf_t *T_INTER_TASK_MSG_DETACH_CNF_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_DETACH_CNF_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_DETACH_CNF_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_DETACH_CNF_HEADER Param1;
} yPDef_MSG_DETACH_CNF;

typedef Inter_ActivateCnf_t *T_INTER_TASK_MSG_ACTIVATE_CNF_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_ACTIVATE_CNF_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_ACTIVATE_CNF_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_ACTIVATE_CNF_HEADER Param1;
} yPDef_MSG_ACTIVATE_CNF;

typedef Inter_DeactivateCnf_t *T_INTER_TASK_MSG_DEACTIVATE_CNF_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_DEACTIVATE_CNF_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_DEACTIVATE_CNF_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_DEACTIVATE_CNF_HEADER Param1;
} yPDef_MSG_DEACTIVATE_CNF;

typedef Inter_SnXidCnf_t *T_INTER_TASK_MSG_SN_XID_CNF_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SN_XID_CNF_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SN_XID_CNF_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SN_XID_CNF_HEADER Param1;
} yPDef_MSG_SN_XID_CNF;

typedef Inter_ActivateSecCnf_t *T_INTER_TASK_MSG_ACTIVATE_SEC_CNF_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_ACTIVATE_SEC_CNF_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_ACTIVATE_SEC_CNF_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_ACTIVATE_SEC_CNF_HEADER Param1;
} yPDef_MSG_ACTIVATE_SEC_CNF;

typedef Inter_ModifyContextInd_t *T_INTER_TASK_MSG_MODIFY_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_MODIFY_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_MODIFY_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_MODIFY_IND_HEADER Param1;
} yPDef_MSG_MODIFY_IND;

typedef Inter_ServiceInd_t *T_INTER_TASK_MSG_SERVICE_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SERVICE_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SERVICE_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SERVICE_IND_HEADER Param1;
} yPDef_MSG_SERVICE_IND;

typedef Inter_CheckQoSMinInd_t *T_INTER_TASK_MSG_CHECK_QOSMIN_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CHECK_QOSMIN_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CHECK_QOSMIN_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CHECK_QOSMIN_IND_HEADER Param1;
} yPDef_MSG_CHECK_QOSMIN_IND;

typedef MMMsgParmTimeZoneInd_t *T_INTER_TASK_MSG_INT_TIMEZONE_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_INT_TIMEZONE_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_INT_TIMEZONE_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_INT_TIMEZONE_IND_HEADER Param1;
} yPDef_MSG_INT_TIMEZONE_IND;

typedef MMMsgParmDateInd_t *T_INTER_TASK_MSG_INT_DATE_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_INT_DATE_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_INT_DATE_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_INT_DATE_IND_HEADER Param1;
} yPDef_MSG_INT_DATE_IND;

typedef MMMsgParmLSAInd_t *T_INTER_TASK_MSG_INT_LSA_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_INT_LSA_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_INT_LSA_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_INT_LSA_IND_HEADER Param1;
} yPDef_MSG_INT_LSA_IND;

typedef Inter_ActivateSecInd_t *T_INTER_TASK_MSG_ACTIVATE_SEC_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_ACTIVATE_SEC_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_ACTIVATE_SEC_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_ACTIVATE_SEC_IND_HEADER Param1;
} yPDef_MSG_ACTIVATE_SEC_IND;

typedef GPRSSuspendInd_t *T_INTER_TASK_MSG_DATA_SUSPEND_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_DATA_SUSPEND_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_DATA_SUSPEND_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_DATA_SUSPEND_IND_HEADER Param1;
} yPDef_MSG_DATA_SUSPEND_IND;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_DATA_RESUME_IND;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_HSDPA_STATUS_IND;

typedef PDP_ULData_Ind_t *T_INTER_TASK_MSG_PCHEX_UL_DATA_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PCHEX_UL_DATA_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PCHEX_UL_DATA_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PCHEX_UL_DATA_IND_HEADER Param1;
} yPDef_MSG_PCHEX_UL_DATA_IND;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_DATA_STATE_TIMER;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CGSEND_TIMER;

typedef Inter_ModifyCnf_t *T_INTER_TASK_MSG_MODIFY_CNF_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_MODIFY_CNF_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_MODIFY_CNF_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_MODIFY_CNF_HEADER Param1;
} yPDef_MSG_MODIFY_CNF;

typedef MSRadioActivityInd_t *T_INTER_TASK_MSG_RADIO_ACTIVITY_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_RADIO_ACTIVITY_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_RADIO_ACTIVITY_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_RADIO_ACTIVITY_IND_HEADER Param1;
} yPDef_MSG_RADIO_ACTIVITY_IND;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_MS_STATUS_IND;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PCH_DELAYDETACH_TIMER;

typedef SIMStatus_t *T_INTER_TASK_MSG_SIM_STATUS_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_STATUS_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_STATUS_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_STATUS_IND_HEADER Param1;
} yPDef_MSG_SIM_STATUS_IND;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIM_ACM_UPDATE_IND;

typedef Boolean *T_INTER_TASK_MSG_SIM_SMS_MEM_AVAIL_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_SMS_MEM_AVAIL_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_SMS_MEM_AVAIL_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_SMS_MEM_AVAIL_IND_HEADER Param1;
} yPDef_MSG_SIM_SMS_MEM_AVAIL_IND;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIM_REFRESH_COMPL_IND;

typedef SIM_PROC_DFILE_INFO_REQ_t *T_INTER_TASK_MSG_SIM_PROC_DFILE_INFO_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PROC_DFILE_INFO_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PROC_DFILE_INFO_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PROC_DFILE_INFO_REQ_HEADER Param1;
} yPDef_MSG_SIM_PROC_DFILE_INFO_REQ;

typedef SIM_PROC_EFILE_INFO_REQ_t *T_INTER_TASK_MSG_SIM_PROC_EFILE_INFO_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PROC_EFILE_INFO_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PROC_EFILE_INFO_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PROC_EFILE_INFO_REQ_HEADER Param1;
} yPDef_MSG_SIM_PROC_EFILE_INFO_REQ;

typedef SIM_PROC_WHOLE_BINARY_FILE_REQ_t *T_INTER_TASK_MSG_SIM_PROC_WHOLE_BINARY_FILE_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PROC_WHOLE_BINARY_FILE_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PROC_WHOLE_BINARY_FILE_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PROC_WHOLE_BINARY_FILE_REQ_HEADER Param1;
} yPDef_MSG_SIM_PROC_WHOLE_BINARY_FILE_REQ;

typedef SIM_PROC_READ_BINARY_FILE_REQ_t *T_INTER_TASK_MSG_SIM_PROC_READ_BINARY_FILE_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PROC_READ_BINARY_FILE_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PROC_READ_BINARY_FILE_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PROC_READ_BINARY_FILE_REQ_HEADER Param1;
} yPDef_MSG_SIM_PROC_READ_BINARY_FILE_REQ;

typedef SIM_PROC_READ_FILE_REC_REQ_t *T_INTER_TASK_MSG_SIM_PROC_READ_FILE_REC_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PROC_READ_FILE_REC_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PROC_READ_FILE_REC_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PROC_READ_FILE_REC_REQ_HEADER Param1;
} yPDef_MSG_SIM_PROC_READ_FILE_REC_REQ;

typedef SIM_PROC_UPDATE_BINARY_FILE_REQ_t *T_INTER_TASK_MSG_SIM_PROC_UPDATE_BINARY_FILE_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PROC_UPDATE_BINARY_FILE_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PROC_UPDATE_BINARY_FILE_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PROC_UPDATE_BINARY_FILE_REQ_HEADER Param1;
} yPDef_MSG_SIM_PROC_UPDATE_BINARY_FILE_REQ;

typedef SIM_PROC_UPDATE_LINEAR_FILE_REQ_t *T_INTER_TASK_MSG_SIM_PROC_UPDATE_LINEAR_FILE_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PROC_UPDATE_LINEAR_FILE_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PROC_UPDATE_LINEAR_FILE_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PROC_UPDATE_LINEAR_FILE_REQ_HEADER Param1;
} yPDef_MSG_SIM_PROC_UPDATE_LINEAR_FILE_REQ;

typedef SIM_PROC_UPDATE_CYCLIC_FILE_REQ_t *T_INTER_TASK_MSG_SIM_PROC_UPDATE_CYCLIC_FILE_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PROC_UPDATE_CYCLIC_FILE_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PROC_UPDATE_CYCLIC_FILE_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PROC_UPDATE_CYCLIC_FILE_REQ_HEADER Param1;
} yPDef_MSG_SIM_PROC_UPDATE_CYCLIC_FILE_REQ;

typedef SIM_PROC_REMAINING_PIN_ATTEMPT_REQ_t *T_INTER_TASK_MSG_SIM_PROC_REMAINING_PIN_ATTEMPT_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PROC_REMAINING_PIN_ATTEMPT_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PROC_REMAINING_PIN_ATTEMPT_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PROC_REMAINING_PIN_ATTEMPT_REQ_HEADER Param1;
} yPDef_MSG_SIM_PROC_REMAINING_PIN_ATTEMPT_REQ;

typedef SIM_PROC_PBK_INFO_REQ_t *T_INTER_TASK_MSG_SIM_PROC_PBK_INFO_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PROC_PBK_INFO_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PROC_PBK_INFO_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PROC_PBK_INFO_REQ_HEADER Param1;
} yPDef_MSG_SIM_PROC_PBK_INFO_REQ;

typedef SIM_PROC_SET_FDN_REQ_t *T_INTER_TASK_MSG_SIM_PROC_SET_FDN_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PROC_SET_FDN_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PROC_SET_FDN_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PROC_SET_FDN_REQ_HEADER Param1;
} yPDef_MSG_SIM_PROC_SET_FDN_REQ;

typedef SIM_PROC_VERIFY_PIN_REQ_t *T_INTER_TASK_MSG_SIM_PROC_VERIFY_PIN_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PROC_VERIFY_PIN_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PROC_VERIFY_PIN_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PROC_VERIFY_PIN_REQ_HEADER Param1;
} yPDef_MSG_SIM_PROC_VERIFY_PIN_REQ;

typedef SIM_PROC_CHANGE_PIN_REQ_t *T_INTER_TASK_MSG_SIM_PROC_CHANGE_PIN_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PROC_CHANGE_PIN_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PROC_CHANGE_PIN_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PROC_CHANGE_PIN_REQ_HEADER Param1;
} yPDef_MSG_SIM_PROC_CHANGE_PIN_REQ;

typedef SIM_PROC_SET_PIN1_REQ_t *T_INTER_TASK_MSG_SIM_PROC_SET_PIN1_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PROC_SET_PIN1_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PROC_SET_PIN1_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PROC_SET_PIN1_REQ_HEADER Param1;
} yPDef_MSG_SIM_PROC_SET_PIN1_REQ;

typedef SIM_PROC_UNBLOCK_PIN_REQ_t *T_INTER_TASK_MSG_SIM_PROC_UNBLOCK_PIN_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PROC_UNBLOCK_PIN_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PROC_UNBLOCK_PIN_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PROC_UNBLOCK_PIN_REQ_HEADER Param1;
} yPDef_MSG_SIM_PROC_UNBLOCK_PIN_REQ;

typedef SIM_PROC_READ_PBK_REQ_t *T_INTER_TASK_MSG_SIM_PROC_READ_PBK_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PROC_READ_PBK_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PROC_READ_PBK_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PROC_READ_PBK_REQ_HEADER Param1;
} yPDef_MSG_SIM_PROC_READ_PBK_REQ;

typedef SIM_PROC_WRITE_PBK_REQ_t *T_INTER_TASK_MSG_SIM_PROC_WRITE_PBK_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PROC_WRITE_PBK_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PROC_WRITE_PBK_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PROC_WRITE_PBK_REQ_HEADER Param1;
} yPDef_MSG_SIM_PROC_WRITE_PBK_REQ;

typedef SIM_PROC_READ_ACM_MAX_REQ_t *T_INTER_TASK_MSG_SIM_PROC_READ_ACM_MAX_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PROC_READ_ACM_MAX_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PROC_READ_ACM_MAX_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PROC_READ_ACM_MAX_REQ_HEADER Param1;
} yPDef_MSG_SIM_PROC_READ_ACM_MAX_REQ;

typedef SIM_PROC_WRITE_ACM_MAX_REQ_t *T_INTER_TASK_MSG_SIM_PROC_WRITE_ACM_MAX_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PROC_WRITE_ACM_MAX_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PROC_WRITE_ACM_MAX_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PROC_WRITE_ACM_MAX_REQ_HEADER Param1;
} yPDef_MSG_SIM_PROC_WRITE_ACM_MAX_REQ;

typedef SIM_PROC_READ_ACM_REQ_t *T_INTER_TASK_MSG_SIM_PROC_READ_ACM_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PROC_READ_ACM_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PROC_READ_ACM_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PROC_READ_ACM_REQ_HEADER Param1;
} yPDef_MSG_SIM_PROC_READ_ACM_REQ;

typedef SIM_PROC_WRITE_ACM_REQ_t *T_INTER_TASK_MSG_SIM_PROC_WRITE_ACM_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PROC_WRITE_ACM_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PROC_WRITE_ACM_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PROC_WRITE_ACM_REQ_HEADER Param1;
} yPDef_MSG_SIM_PROC_WRITE_ACM_REQ;

typedef SIM_PROC_INCREASE_ACM_REQ_t *T_INTER_TASK_MSG_SIM_PROC_INCREASE_ACM_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PROC_INCREASE_ACM_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PROC_INCREASE_ACM_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PROC_INCREASE_ACM_REQ_HEADER Param1;
} yPDef_MSG_SIM_PROC_INCREASE_ACM_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIM_PROC_SERV_PROV_REQ;

typedef SIM_PROC_READ_PUCT_REQ_t *T_INTER_TASK_MSG_SIM_PROC_READ_PUCT_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PROC_READ_PUCT_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PROC_READ_PUCT_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PROC_READ_PUCT_REQ_HEADER Param1;
} yPDef_MSG_SIM_PROC_READ_PUCT_REQ;

typedef SIM_PROC_WRITE_PUCT_REQ_t *T_INTER_TASK_MSG_SIM_PROC_WRITE_PUCT_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PROC_WRITE_PUCT_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PROC_WRITE_PUCT_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PROC_WRITE_PUCT_REQ_HEADER Param1;
} yPDef_MSG_SIM_PROC_WRITE_PUCT_REQ;

typedef SIM_PROC_SEARCH_SMS_REQ_t *T_INTER_TASK_MSG_SIM_PROC_SEARCH_SMS_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PROC_SEARCH_SMS_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PROC_SEARCH_SMS_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PROC_SEARCH_SMS_REQ_HEADER Param1;
} yPDef_MSG_SIM_PROC_SEARCH_SMS_REQ;

typedef SIM_PROC_READ_SMS_REQ_t *T_INTER_TASK_MSG_SIM_PROC_READ_SMS_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PROC_READ_SMS_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PROC_READ_SMS_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PROC_READ_SMS_REQ_HEADER Param1;
} yPDef_MSG_SIM_PROC_READ_SMS_REQ;

typedef SIM_PROC_WRITE_SMS_REQ_t *T_INTER_TASK_MSG_SIM_PROC_WRITE_SMS_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PROC_WRITE_SMS_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PROC_WRITE_SMS_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PROC_WRITE_SMS_REQ_HEADER Param1;
} yPDef_MSG_SIM_PROC_WRITE_SMS_REQ;

typedef SIM_PROC_WRITE_SMS_STATUS_REQ_t *T_INTER_TASK_MSG_SIM_PROC_WRITE_SMS_STATUS_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PROC_WRITE_SMS_STATUS_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PROC_WRITE_SMS_STATUS_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PROC_WRITE_SMS_STATUS_REQ_HEADER Param1;
} yPDef_MSG_SIM_PROC_WRITE_SMS_STATUS_REQ;

typedef SIM_PROC_READ_SMS_PARAM_REQ_t *T_INTER_TASK_MSG_SIM_PROC_READ_SMS_PARAM_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PROC_READ_SMS_PARAM_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PROC_READ_SMS_PARAM_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PROC_READ_SMS_PARAM_REQ_HEADER Param1;
} yPDef_MSG_SIM_PROC_READ_SMS_PARAM_REQ;

typedef SIM_PROC_WRITE_SMS_SCA_REQ_t *T_INTER_TASK_MSG_SIM_PROC_WRITE_SMS_SCA_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PROC_WRITE_SMS_SCA_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PROC_WRITE_SMS_SCA_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PROC_WRITE_SMS_SCA_REQ_HEADER Param1;
} yPDef_MSG_SIM_PROC_WRITE_SMS_SCA_REQ;

typedef SIM_PROC_SMS_REF_NUM_REQ_t *T_INTER_TASK_MSG_SIM_PROC_SMS_REF_NUM_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PROC_SMS_REF_NUM_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PROC_SMS_REF_NUM_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PROC_SMS_REF_NUM_REQ_HEADER Param1;
} yPDef_MSG_SIM_PROC_SMS_REF_NUM_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIM_APDU_GENERIC_ACCESS_REQ;

typedef SIM_PROC_RESTRICTED_ACCESS_REQ_t *T_INTER_TASK_MSG_SIM_PROC_RESTRICTED_ACCESS_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PROC_RESTRICTED_ACCESS_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PROC_RESTRICTED_ACCESS_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PROC_RESTRICTED_ACCESS_REQ_HEADER Param1;
} yPDef_MSG_SIM_PROC_RESTRICTED_ACCESS_REQ;

typedef USIM_SET_EST_SERV_REQ_t *T_INTER_TASK_MSG_SIM_PROC_SET_EST_SERV_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PROC_SET_EST_SERV_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PROC_SET_EST_SERV_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PROC_SET_EST_SERV_REQ_HEADER Param1;
} yPDef_MSG_SIM_PROC_SET_EST_SERV_REQ;

typedef USIM_UPDATE_ONE_APN_REQ_t *T_INTER_TASK_MSG_SIM_PROC_UPDATE_ONE_APN_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PROC_UPDATE_ONE_APN_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PROC_UPDATE_ONE_APN_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PROC_UPDATE_ONE_APN_REQ_HEADER Param1;
} yPDef_MSG_SIM_PROC_UPDATE_ONE_APN_REQ;

typedef USIM_DELETE_ALL_APN_REQ_t *T_INTER_TASK_MSG_SIM_PROC_DELETE_ALL_APN_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PROC_DELETE_ALL_APN_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PROC_DELETE_ALL_APN_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PROC_DELETE_ALL_APN_REQ_HEADER Param1;
} yPDef_MSG_SIM_PROC_DELETE_ALL_APN_REQ;

typedef SIMGeneralServiceStatus_t *T_INTER_TASK_MSG_SIM_PROC_GENERAL_SERVICE_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PROC_GENERAL_SERVICE_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PROC_GENERAL_SERVICE_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PROC_GENERAL_SERVICE_IND_HEADER Param1;
} yPDef_MSG_SIM_PROC_GENERAL_SERVICE_IND;

typedef SIM_PROC_SEEK_RECORD_REQ_t *T_INTER_TASK_MSG_SIM_PROC_SEEK_RECORD_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PROC_SEEK_RECORD_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PROC_SEEK_RECORD_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PROC_SEEK_RECORD_REQ_HEADER Param1;
} yPDef_MSG_SIM_PROC_SEEK_RECORD_REQ;

typedef SIM_NUM_OF_PLMN_REQ_t *T_INTER_TASK_MSG_SIM_PROC_NUM_OF_PLMN_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PROC_NUM_OF_PLMN_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PROC_NUM_OF_PLMN_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PROC_NUM_OF_PLMN_REQ_HEADER Param1;
} yPDef_MSG_SIM_PROC_NUM_OF_PLMN_REQ;

typedef SIM_READ_PLMN_REQ_t *T_INTER_TASK_MSG_SIM_PROC_READ_PLMN_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PROC_READ_PLMN_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PROC_READ_PLMN_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PROC_READ_PLMN_REQ_HEADER Param1;
} yPDef_MSG_SIM_PROC_READ_PLMN_REQ;

typedef SIM_UPDATE_MUL_PLMN_REQ_t *T_INTER_TASK_MSG_SIM_PROC_UPDATE_MUL_PLMN_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PROC_UPDATE_MUL_PLMN_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PROC_UPDATE_MUL_PLMN_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PROC_UPDATE_MUL_PLMN_REQ_HEADER Param1;
} yPDef_MSG_SIM_PROC_UPDATE_MUL_PLMN_REQ;

typedef SIM_PROC_ISIM_ACTIVATE_REQ_t *T_INTER_TASK_MSG_SIM_PROC_ISIM_ACTIVATE_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PROC_ISIM_ACTIVATE_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PROC_ISIM_ACTIVATE_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PROC_ISIM_ACTIVATE_REQ_HEADER Param1;
} yPDef_MSG_SIM_PROC_ISIM_ACTIVATE_REQ;

typedef SIM_PROC_ISIM_AUTHENTICATE_REQ_t *T_INTER_TASK_MSG_SIM_PROC_ISIM_AUTHEN_AKA_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PROC_ISIM_AUTHEN_AKA_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PROC_ISIM_AUTHEN_AKA_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PROC_ISIM_AUTHEN_AKA_REQ_HEADER Param1;
} yPDef_MSG_SIM_PROC_ISIM_AUTHEN_AKA_REQ;

typedef SIM_PROC_ISIM_AUTHENTICATE_REQ_t *T_INTER_TASK_MSG_SIM_PROC_ISIM_AUTHEN_HTTP_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PROC_ISIM_AUTHEN_HTTP_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PROC_ISIM_AUTHEN_HTTP_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PROC_ISIM_AUTHEN_HTTP_REQ_HEADER Param1;
} yPDef_MSG_SIM_PROC_ISIM_AUTHEN_HTTP_REQ;

typedef SIM_PROC_ISIM_AUTHENTICATE_REQ_t *T_INTER_TASK_MSG_SIM_PROC_ISIM_AUTHEN_GBA_BOOT_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PROC_ISIM_AUTHEN_GBA_BOOT_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PROC_ISIM_AUTHEN_GBA_BOOT_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PROC_ISIM_AUTHEN_GBA_BOOT_REQ_HEADER Param1;
} yPDef_MSG_SIM_PROC_ISIM_AUTHEN_GBA_BOOT_REQ;

typedef SIM_PROC_SET_BDN_REQ_t *T_INTER_TASK_MSG_SIM_PROC_ISIM_AUTHEN_GBA_NAF_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PROC_ISIM_AUTHEN_GBA_NAF_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PROC_ISIM_AUTHEN_GBA_NAF_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PROC_ISIM_AUTHEN_GBA_NAF_REQ_HEADER Param1;
} yPDef_MSG_SIM_PROC_ISIM_AUTHEN_GBA_NAF_REQ;

typedef SIM_PROC_SET_BDN_REQ_t *T_INTER_TASK_MSG_SIM_PROC_SET_BDN_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PROC_SET_BDN_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PROC_SET_BDN_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PROC_SET_BDN_REQ_HEADER Param1;
} yPDef_MSG_SIM_PROC_SET_BDN_REQ;

typedef SIM_PROC_POWER_ON_OFF_CARD_REQ_t *T_INTER_TASK_MSG_SIM_PROC_POWER_ON_OFF_CARD_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PROC_POWER_ON_OFF_CARD_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PROC_POWER_ON_OFF_CARD_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PROC_POWER_ON_OFF_CARD_REQ_HEADER Param1;
} yPDef_MSG_SIM_PROC_POWER_ON_OFF_CARD_REQ;

typedef SIM_PROC_GET_RAW_ATR_REQ_t *T_INTER_TASK_MSG_SIM_PROC_GET_RAW_ATR_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PROC_GET_RAW_ATR_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PROC_GET_RAW_ATR_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PROC_GET_RAW_ATR_REQ_HEADER Param1;
} yPDef_MSG_SIM_PROC_GET_RAW_ATR_REQ;

typedef SIM_PROC_OPEN_SOCKET_REQ_t *T_INTER_TASK_MSG_SIM_PROC_OPEN_SOCKET_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PROC_OPEN_SOCKET_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PROC_OPEN_SOCKET_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PROC_OPEN_SOCKET_REQ_HEADER Param1;
} yPDef_MSG_SIM_PROC_OPEN_SOCKET_REQ;

typedef SIM_PROC_SELECT_APPLI_REQ_t *T_INTER_TASK_MSG_SIM_PROC_SELECT_APPLI_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PROC_SELECT_APPLI_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PROC_SELECT_APPLI_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PROC_SELECT_APPLI_REQ_HEADER Param1;
} yPDef_MSG_SIM_PROC_SELECT_APPLI_REQ;

typedef SIM_PROC_DEACTIVATE_APPLI_REQ_t *T_INTER_TASK_MSG_SIM_PROC_DEACTIVATE_APPLI_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PROC_DEACTIVATE_APPLI_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PROC_DEACTIVATE_APPLI_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PROC_DEACTIVATE_APPLI_REQ_HEADER Param1;
} yPDef_MSG_SIM_PROC_DEACTIVATE_APPLI_REQ;

typedef SIM_PROC_CLOSE_SOCKET_REQ_t *T_INTER_TASK_MSG_SIM_PROC_CLOSE_SOCKET_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PROC_CLOSE_SOCKET_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PROC_CLOSE_SOCKET_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PROC_CLOSE_SOCKET_REQ_HEADER Param1;
} yPDef_MSG_SIM_PROC_CLOSE_SOCKET_REQ;

typedef SimProcSteeringOfRoamingReq_t *T_INTER_TASK_MSG_SIM_PROC_STEERING_OF_ROAMING_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PROC_STEERING_OF_ROAMING_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PROC_STEERING_OF_ROAMING_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PROC_STEERING_OF_ROAMING_REQ_HEADER Param1;
} yPDef_MSG_SIM_PROC_STEERING_OF_ROAMING_REQ;

typedef SIM_PROC_MUL_REC_REQ_t *T_INTER_TASK_MSG_SIM_PROC_READ_MUL_REC_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PROC_READ_MUL_REC_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PROC_READ_MUL_REC_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PROC_READ_MUL_REC_REQ_HEADER Param1;
} yPDef_MSG_SIM_PROC_READ_MUL_REC_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_STK_MENU_SELECTION_RES;

typedef SIMParmDisplayTextReq_t *T_INTER_TASK_MSG_STK_DISPLAY_TEXT_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_STK_DISPLAY_TEXT_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_STK_DISPLAY_TEXT_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_STK_DISPLAY_TEXT_REQ_HEADER Param1;
} yPDef_MSG_STK_DISPLAY_TEXT_REQ;

typedef SIMParmSetupIdlemodeTextReq_t *T_INTER_TASK_MSG_STK_SETUP_IDLEMODE_TEXT_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_STK_SETUP_IDLEMODE_TEXT_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_STK_SETUP_IDLEMODE_TEXT_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_STK_SETUP_IDLEMODE_TEXT_REQ_HEADER Param1;
} yPDef_MSG_STK_SETUP_IDLEMODE_TEXT_REQ;

typedef SIMParmGetInputReq_t *T_INTER_TASK_MSG_STK_GET_INPUT_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_STK_GET_INPUT_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_STK_GET_INPUT_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_STK_GET_INPUT_REQ_HEADER Param1;
} yPDef_MSG_STK_GET_INPUT_REQ;

typedef SIMParmGetInkeyReq_t *T_INTER_TASK_MSG_STK_GET_INKEY_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_STK_GET_INKEY_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_STK_GET_INKEY_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_STK_GET_INKEY_REQ_HEADER Param1;
} yPDef_MSG_STK_GET_INKEY_REQ;

typedef SIMParmPlayToneReq_t *T_INTER_TASK_MSG_STK_PLAY_TONE_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_STK_PLAY_TONE_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_STK_PLAY_TONE_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_STK_PLAY_TONE_REQ_HEADER Param1;
} yPDef_MSG_STK_PLAY_TONE_REQ;

typedef SIMParmSelectItemReq_t *T_INTER_TASK_MSG_STK_SELECT_ITEM_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_STK_SELECT_ITEM_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_STK_SELECT_ITEM_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_STK_SELECT_ITEM_REQ_HEADER Param1;
} yPDef_MSG_STK_SELECT_ITEM_REQ;

typedef SIMParmSetupMenuReq_t *T_INTER_TASK_MSG_STK_SETUP_MENU_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_STK_SETUP_MENU_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_STK_SETUP_MENU_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_STK_SETUP_MENU_REQ_HEADER Param1;
} yPDef_MSG_STK_SETUP_MENU_REQ;

typedef SIMParmSimtoolkitRefreshReq_t *T_INTER_TASK_MSG_STK_SIMTOOLKIT_REFRESH_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_STK_SIMTOOLKIT_REFRESH_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_STK_SIMTOOLKIT_REFRESH_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_STK_SIMTOOLKIT_REFRESH_REQ_HEADER Param1;
} yPDef_MSG_STK_SIMTOOLKIT_REFRESH_REQ;

typedef SIMParmSiatSimtoolkitRefreshReq_t *T_INTER_TASK_MSG_STK_SIAT_SIMTOOLKIT_REFRESH_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_STK_SIAT_SIMTOOLKIT_REFRESH_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_STK_SIAT_SIMTOOLKIT_REFRESH_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_STK_SIAT_SIMTOOLKIT_REFRESH_REQ_HEADER Param1;
} yPDef_MSG_STK_SIAT_SIMTOOLKIT_REFRESH_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_STK_STK_END_IND;

typedef SIMParmSendSsReq_t *T_INTER_TASK_MSG_STK_SEND_SS_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_STK_SEND_SS_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_STK_SEND_SS_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_STK_SEND_SS_REQ_HEADER Param1;
} yPDef_MSG_STK_SEND_SS_REQ;

typedef SIMParmSetupCallReq_t *T_INTER_TASK_MSG_STK_SETUP_CALL_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_STK_SETUP_CALL_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_STK_SETUP_CALL_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_STK_SETUP_CALL_REQ_HEADER Param1;
} yPDef_MSG_STK_SETUP_CALL_REQ;

typedef SIMParmMoSMSAlphaInd_t *T_INTER_TASK_MSG_STK_MOSMS_ALPHA_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_STK_MOSMS_ALPHA_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_STK_MOSMS_ALPHA_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_STK_MOSMS_ALPHA_IND_HEADER Param1;
} yPDef_MSG_STK_MOSMS_ALPHA_IND;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_STK_LOCAL_DATE_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_STK_LOCAL_LANG_REQ;

typedef StkCallSetupFail_t *T_INTER_TASK_MSG_STK_CC_SETUPFAIL_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_STK_CC_SETUPFAIL_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_STK_CC_SETUPFAIL_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_STK_CC_SETUPFAIL_IND_HEADER Param1;
} yPDef_MSG_STK_CC_SETUPFAIL_IND;

typedef StkCallControlDisplay_t *T_INTER_TASK_MSG_STK_CC_DISPLAY_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_STK_CC_DISPLAY_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_STK_CC_DISPLAY_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_STK_CC_DISPLAY_IND_HEADER Param1;
} yPDef_MSG_STK_CC_DISPLAY_IND;

typedef StkSendTermRespData_t *T_INTER_TASK_MSG_STK_SEND_TERMINAL_RESPONSE_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_STK_SEND_TERMINAL_RESPONSE_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_STK_SEND_TERMINAL_RESPONSE_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_STK_SEND_TERMINAL_RESPONSE_HEADER Param1;
} yPDef_MSG_STK_SEND_TERMINAL_RESPONSE;

typedef STKLaunchBrowserReq_t *T_INTER_TASK_MSG_STK_LAUNCH_BROWSER_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_STK_LAUNCH_BROWSER_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_STK_LAUNCH_BROWSER_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_STK_LAUNCH_BROWSER_REQ_HEADER Param1;
} yPDef_MSG_STK_LAUNCH_BROWSER_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_STK_STOP_RETRY_TIMER_REQ;

typedef SIMParmSendDtmfReq_t *T_INTER_TASK_MSG_STK_SEND_DTMF_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_STK_SEND_DTMF_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_STK_SEND_DTMF_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_STK_SEND_DTMF_REQ_HEADER Param1;
} yPDef_MSG_STK_SEND_DTMF_REQ;

typedef StkCallConnectedEvt_t *T_INTER_TASK_MSG_STK_CALL_CONNECTED_EVT_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_STK_CALL_CONNECTED_EVT_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_STK_CALL_CONNECTED_EVT_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_STK_CALL_CONNECTED_EVT_HEADER Param1;
} yPDef_MSG_STK_CALL_CONNECTED_EVT;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_STK_ACTIVATE_REQ;

typedef CCParmSend_t *T_INTER_TASK_MSG_CALL_ESTABLISH_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CALL_ESTABLISH_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CALL_ESTABLISH_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CALL_ESTABLISH_REQ_HEADER Param1;
} yPDef_MSG_CALL_ESTABLISH_REQ;

typedef SATK_SendCcSetupReq_t *T_INTER_TASK_MSG_STK_SEND_CC_SETUP_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_STK_SEND_CC_SETUP_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_STK_SEND_CC_SETUP_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_STK_SEND_CC_SETUP_REQ_HEADER Param1;
} yPDef_MSG_STK_SEND_CC_SETUP_REQ;

typedef SATK_SendCcSsReq_t *T_INTER_TASK_MSG_STK_SEND_CC_SS_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_STK_SEND_CC_SS_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_STK_SEND_CC_SS_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_STK_SEND_CC_SS_REQ_HEADER Param1;
} yPDef_MSG_STK_SEND_CC_SS_REQ;

typedef SATK_SendCcUssdReq_t *T_INTER_TASK_MSG_STK_SEND_CC_USSD_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_STK_SEND_CC_USSD_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_STK_SEND_CC_USSD_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_STK_SEND_CC_USSD_REQ_HEADER Param1;
} yPDef_MSG_STK_SEND_CC_USSD_REQ;

typedef SATK_SendCcSmsReq_t *T_INTER_TASK_MSG_STK_SEND_CC_SMS_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_STK_SEND_CC_SMS_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_STK_SEND_CC_SMS_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_STK_SEND_CC_SMS_REQ_HEADER Param1;
} yPDef_MSG_STK_SEND_CC_SMS_REQ;

typedef SATK_SendEnvelopCmdReq_t *T_INTER_TASK_MSG_STK_SEND_ENVELOPE_CMD_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_STK_SEND_ENVELOPE_CMD_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_STK_SEND_ENVELOPE_CMD_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_STK_SEND_ENVELOPE_CMD_REQ_HEADER Param1;
} yPDef_MSG_STK_SEND_ENVELOPE_CMD_REQ;

typedef SATK_SendTerminalRspReq_t *T_INTER_TASK_MSG_STK_SEND_TERMINAL_RSP_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_STK_SEND_TERMINAL_RSP_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_STK_SEND_TERMINAL_RSP_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_STK_SEND_TERMINAL_RSP_REQ_HEADER Param1;
} yPDef_MSG_STK_SEND_TERMINAL_RSP_REQ;

typedef StkCallControlSetupRsp_t *T_INTER_TASK_MSG_STK_CALL_CONTROL_SETUP_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_STK_CALL_CONTROL_SETUP_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_STK_CALL_CONTROL_SETUP_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_STK_CALL_CONTROL_SETUP_RSP_HEADER Param1;
} yPDef_MSG_STK_CALL_CONTROL_SETUP_RSP;

typedef StkCallControlSsRsp_t *T_INTER_TASK_MSG_STK_CALL_CONTROL_SS_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_STK_CALL_CONTROL_SS_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_STK_CALL_CONTROL_SS_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_STK_CALL_CONTROL_SS_RSP_HEADER Param1;
} yPDef_MSG_STK_CALL_CONTROL_SS_RSP;

typedef StkCallControlUssdRsp_t *T_INTER_TASK_MSG_STK_CALL_CONTROL_USSD_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_STK_CALL_CONTROL_USSD_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_STK_CALL_CONTROL_USSD_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_STK_CALL_CONTROL_USSD_RSP_HEADER Param1;
} yPDef_MSG_STK_CALL_CONTROL_USSD_RSP;

typedef StkCallControlSmsRsp_t *T_INTER_TASK_MSG_STK_CALL_CONTROL_SMS_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_STK_CALL_CONTROL_SMS_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_STK_CALL_CONTROL_SMS_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_STK_CALL_CONTROL_SMS_RSP_HEADER Param1;
} yPDef_MSG_STK_CALL_CONTROL_SMS_RSP;

typedef ProactiveCmdData_t *T_INTER_TASK_MSG_STK_PROACTIVE_CMD_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_STK_PROACTIVE_CMD_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_STK_PROACTIVE_CMD_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_STK_PROACTIVE_CMD_IND_HEADER Param1;
} yPDef_MSG_STK_PROACTIVE_CMD_IND;

typedef EnvelopeRspData_t *T_INTER_TASK_MSG_STK_ENVELOPE_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_STK_ENVELOPE_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_STK_ENVELOPE_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_STK_ENVELOPE_RSP_HEADER Param1;
} yPDef_MSG_STK_ENVELOPE_RSP;

typedef TerminalResponseRspData_t *T_INTER_TASK_MSG_STK_TERMINAL_RESPONSE_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_STK_TERMINAL_RESPONSE_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_STK_TERMINAL_RESPONSE_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_STK_TERMINAL_RESPONSE_RSP_HEADER Param1;
} yPDef_MSG_STK_TERMINAL_RESPONSE_RSP;

typedef SIMParmOpenChannelReq_t *T_INTER_TASK_MSG_STK_OPEN_CHANNEL_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_STK_OPEN_CHANNEL_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_STK_OPEN_CHANNEL_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_STK_OPEN_CHANNEL_REQ_HEADER Param1;
} yPDef_MSG_STK_OPEN_CHANNEL_REQ;

typedef SIMParmSendDataReq_t *T_INTER_TASK_MSG_STK_SEND_DATA_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_STK_SEND_DATA_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_STK_SEND_DATA_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_STK_SEND_DATA_REQ_HEADER Param1;
} yPDef_MSG_STK_SEND_DATA_REQ;

typedef SIMParmReceiveDataReq_t *T_INTER_TASK_MSG_STK_RECEIVE_DATA_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_STK_RECEIVE_DATA_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_STK_RECEIVE_DATA_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_STK_RECEIVE_DATA_REQ_HEADER Param1;
} yPDef_MSG_STK_RECEIVE_DATA_REQ;

typedef SIMParmCloseChannelReq_t *T_INTER_TASK_MSG_STK_CLOSE_CHANNEL_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_STK_CLOSE_CHANNEL_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_STK_CLOSE_CHANNEL_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_STK_CLOSE_CHANNEL_REQ_HEADER Param1;
} yPDef_MSG_STK_CLOSE_CHANNEL_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_STK_GET_CHANNEL_STATUS_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_STK_TERMINATE_DATA_SERV_REQ;

typedef Boolean *T_INTER_TASK_MSG_STK_TERMINATE_DATA_SERV_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_STK_TERMINATE_DATA_SERV_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_STK_TERMINATE_DATA_SERV_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_STK_TERMINATE_DATA_SERV_RSP_HEADER Param1;
} yPDef_MSG_STK_TERMINATE_DATA_SERV_RSP;

typedef UInt16 *T_INTER_TASK_MSG_STK_DATA_AVAIL_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_STK_DATA_AVAIL_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_STK_DATA_AVAIL_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_STK_DATA_AVAIL_IND_HEADER Param1;
} yPDef_MSG_STK_DATA_AVAIL_IND;

typedef PCHResponseType_t *T_INTER_TASK_MSG_STK_ACTIVATE_GPRS_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_STK_ACTIVATE_GPRS_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_STK_ACTIVATE_GPRS_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_STK_ACTIVATE_GPRS_IND_HEADER Param1;
} yPDef_MSG_STK_ACTIVATE_GPRS_IND;

typedef PCHResponseType_t *T_INTER_TASK_MSG_STK_DEACTIVATE_GPRS_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_STK_DEACTIVATE_GPRS_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_STK_DEACTIVATE_GPRS_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_STK_DEACTIVATE_GPRS_IND_HEADER Param1;
} yPDef_MSG_STK_DEACTIVATE_GPRS_IND;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_STK_SHUTDOWN_LINK_TIMER_IND;

typedef RunAT_Request *T_INTER_TASK_MSG_STK_RUN_AT_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_STK_RUN_AT_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_STK_RUN_AT_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_STK_RUN_AT_REQ_HEADER Param1;
} yPDef_MSG_STK_RUN_AT_REQ;

typedef RunAT_Response *T_INTER_TASK_MSG_STK_RUN_AT_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_STK_RUN_AT_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_STK_RUN_AT_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_STK_RUN_AT_RSP_HEADER Param1;
} yPDef_MSG_STK_RUN_AT_RSP;

typedef StkRunAtCmd_t *T_INTER_TASK_MSG_STK_RUN_AT_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_STK_RUN_AT_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_STK_RUN_AT_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_STK_RUN_AT_IND_HEADER Param1;
} yPDef_MSG_STK_RUN_AT_IND;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_STK_LANG_NOTIFICATION_IND;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_STK_LOCAL_IMEISV_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_STK_LOCAL_NW_SEARCH_MODE_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_STK_LOCAL_BATTERY_STATE_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_STK_NW_SEARCH_MODE_CHG_IND;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_STK_BROWSING_STATUS_IND;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_STK_SEND_IPAT_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CAPI2_MSGIDS_START;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CAPI2_LCS_REQ_START;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_LCS_CPMOLRREQ_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_LCS_CPMOLRABORT_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_LCS_CPMOLRABORT_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_LCS_CPMTLRVERIFICATIONRSP_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_LCS_CPMTLRVERIFICATIONRSP_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_LCS_CPMTLRRSP_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_LCS_CPMTLRRSP_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_LCS_CPLOCUPDATERSP_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_LCS_CPLOCUPDATERSP_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_LCS_DECODEPOSESTIMATE_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_LCS_DECODEPOSESTIMATE_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_LCS_ENCODEASSISTANCEREQ_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_LCS_ENCODEASSISTANCEREQ_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CAPI2_LCS_REQ_END;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CAPI2_SMS_RESP_START;

typedef Boolean *T_INTER_TASK_MSG_SMS_GETSIMSMSCAPACITYEXCEEDEDFLAG_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_GETSIMSMSCAPACITYEXCEEDEDFLAG_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_GETSIMSMSCAPACITYEXCEEDEDFLAG_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_GETSIMSMSCAPACITYEXCEEDEDFLAG_RSP_HEADER Param1;
} yPDef_MSG_SMS_GETSIMSMSCAPACITYEXCEEDEDFLAG_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_ISSMSSERVICEAVAIL_RSP;

typedef SIMSMSMesgStatus_t *T_INTER_TASK_MSG_SMS_GETSMSSTOREDSTATE_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_GETSMSSTOREDSTATE_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_GETSMSSTOREDSTATE_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_GETSMSSTOREDSTATE_RSP_HEADER Param1;
} yPDef_MSG_SMS_GETSMSSTOREDSTATE_RSP;

typedef UInt8 *T_INTER_TASK_MSG_SMS_GETLASTTPMR_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_GETLASTTPMR_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_GETLASTTPMR_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_GETLASTTPMR_RSP_HEADER Param1;
} yPDef_MSG_SMS_GETLASTTPMR_RSP;

typedef SmsTxParam_t *T_INTER_TASK_MSG_SMS_GETSMSTXPARAMS_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_GETSMSTXPARAMS_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_GETSMSTXPARAMS_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_GETSMSTXPARAMS_RSP_HEADER Param1;
} yPDef_MSG_SMS_GETSMSTXPARAMS_RSP;

typedef SmsTxTextModeParms_t *T_INTER_TASK_MSG_SMS_GETTXPARAMINTEXTMODE_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_GETTXPARAMINTEXTMODE_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_GETTXPARAMINTEXTMODE_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_GETTXPARAMINTEXTMODE_RSP_HEADER Param1;
} yPDef_MSG_SMS_GETTXPARAMINTEXTMODE_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SETSMSTXPARAMPROCID_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SETSMSTXPARAMCODINGTYPE_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SETSMSTXPARAMVALIDPERIOD_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SETSMSTXPARAMCOMPRESSION_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SETSMSTXPARAMREPLYPATH_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SETSMSTXPARAMUSERDATAHDRIND_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SETSMSTXPARAMSTATUSRPTREQFLAG_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SETSMSTXPARAMREJDUPL_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_CHANGESTATUSREQ_RSP;

typedef UInt8 *T_INTER_TASK_MSG_SMS_GETNEWMSGDISPLAYPREF_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_GETNEWMSGDISPLAYPREF_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_GETNEWMSGDISPLAYPREF_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_GETNEWMSGDISPLAYPREF_RSP_HEADER Param1;
} yPDef_MSG_SMS_GETNEWMSGDISPLAYPREF_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SETSMSPREFSTORAGE_RSP;

typedef CAPI2_SMS_GetSMSStorageStatus_t *T_INTER_TASK_MSG_SMS_GETSMSSTORAGESTATUS_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_GETSMSSTORAGESTATUS_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_GETSMSSTORAGESTATUS_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_GETSMSSTORAGESTATUS_RSP_HEADER Param1;
} yPDef_MSG_SMS_GETSMSSTORAGESTATUS_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SAVESMSSERVICEPROFILE_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_RESTORESMSSERVICEPROFILE_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_CBALLOWALLCHNLREQ_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_ADDCELLBROADCASTCHNLREQ_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_REMOVECELLBROADCASTCHNLREQ_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_REMOVEALLCBCHNLFROMSEARCHLIST_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_ADDCELLBROADCASTLANGREQ_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_REMOVECELLBROADCASTLANGREQ_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SETCBIGNOREDUPLFLAG_RSP;

typedef UInt8 *T_INTER_TASK_MSG_SMS_GETCBIGNOREDUPLFLAG_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_GETCBIGNOREDUPLFLAG_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_GETCBIGNOREDUPLFLAG_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_GETCBIGNOREDUPLFLAG_RSP_HEADER Param1;
} yPDef_MSG_SMS_GETCBIGNOREDUPLFLAG_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SETVMINDONOFF_RSP;

typedef Boolean *T_INTER_TASK_MSG_SMS_ISVMINDENABLED_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_ISVMINDENABLED_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_ISVMINDENABLED_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_ISVMINDENABLED_RSP_HEADER Param1;
} yPDef_MSG_SMS_ISVMINDENABLED_RSP;

typedef SmsVoicemailInd_t *T_INTER_TASK_MSG_SMS_GETVMWAITINGSTATUS_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_GETVMWAITINGSTATUS_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_GETVMWAITINGSTATUS_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_GETVMWAITINGSTATUS_RSP_HEADER Param1;
} yPDef_MSG_SMS_GETVMWAITINGSTATUS_RSP;

typedef UInt8 *T_INTER_TASK_MSG_SMS_GETNUMOFVMSCNUMBER_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_GETNUMOFVMSCNUMBER_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_GETNUMOFVMSCNUMBER_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_GETNUMOFVMSCNUMBER_RSP_HEADER Param1;
} yPDef_MSG_SMS_GETNUMOFVMSCNUMBER_RSP;

typedef SmsAddress_t *T_INTER_TASK_MSG_SMS_GETVMSCNUMBER_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_GETVMSCNUMBER_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_GETVMSCNUMBER_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_GETVMSCNUMBER_RSP_HEADER Param1;
} yPDef_MSG_SMS_GETVMSCNUMBER_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_UPDATEVMSCNUMBERREQ_RSP;

typedef SMS_BEARER_PREFERENCE_t *T_INTER_TASK_MSG_SMS_GETSMSBEARERPREFERENCE_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_GETSMSBEARERPREFERENCE_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_GETSMSBEARERPREFERENCE_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_GETSMSBEARERPREFERENCE_RSP_HEADER Param1;
} yPDef_MSG_SMS_GETSMSBEARERPREFERENCE_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SETSMSBEARERPREFERENCE_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SETSMSREADSTATUSCHANGEMODE_RSP;

typedef Boolean *T_INTER_TASK_MSG_SMS_GETSMSREADSTATUSCHANGEMODE_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_GETSMSREADSTATUSCHANGEMODE_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_GETSMSREADSTATUSCHANGEMODE_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_GETSMSREADSTATUSCHANGEMODE_RSP_HEADER Param1;
} yPDef_MSG_SMS_GETSMSREADSTATUSCHANGEMODE_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SENDSMSSRVCENTERNUMBERUPDATEREQ_RSP;

typedef SmsAddress_t *T_INTER_TASK_MSG_SMS_GETSMSSRVCENTERNUMBER_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_GETSMSSRVCENTERNUMBER_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_GETSMSSRVCENTERNUMBER_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_GETSMSSRVCENTERNUMBER_RSP_HEADER Param1;
} yPDef_MSG_SMS_GETSMSSRVCENTERNUMBER_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SETNEWMSGDISPLAYPREF_RSP;

typedef SmsStorage_t *T_INTER_TASK_MSG_SMS_GETSMSPREFSTORAGE_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_GETSMSPREFSTORAGE_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_GETSMSPREFSTORAGE_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_GETSMSPREFSTORAGE_RSP_HEADER Param1;
} yPDef_MSG_SMS_GETSMSPREFSTORAGE_RSP;

typedef SMS_CB_MSG_IDS_t *T_INTER_TASK_MSG_SMS_GETCBMI_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_GETCBMI_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_GETCBMI_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_GETCBMI_RSP_HEADER Param1;
} yPDef_MSG_SMS_GETCBMI_RSP;

typedef MS_T_MN_CB_LANGUAGES *T_INTER_TASK_MSG_SMS_GETCBLANGUAGE_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_GETCBLANGUAGE_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_GETCBLANGUAGE_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_GETCBLANGUAGE_RSP_HEADER Param1;
} yPDef_MSG_SMS_GETCBLANGUAGE_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SETSMSSTOREDSTATE_RSP;

typedef CAPI2_SMS_GetTransactionFromClientID_t *T_INTER_TASK_MSG_SMS_GETTRANSACTIONFROMCLIENTID_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_GETTRANSACTIONFROMCLIENTID_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_GETTRANSACTIONFROMCLIENTID_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_GETTRANSACTIONFROMCLIENTID_RSP_HEADER Param1;
} yPDef_MSG_SMS_GETTRANSACTIONFROMCLIENTID_RSP;

typedef Boolean *T_INTER_TASK_MSG_SMS_ISCACHEDDATAREADY_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_ISCACHEDDATAREADY_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_ISCACHEDDATAREADY_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_ISCACHEDDATAREADY_RSP_HEADER Param1;
} yPDef_MSG_SMS_ISCACHEDDATAREADY_RSP;

typedef SmsEnhancedVMInd_t *T_INTER_TASK_MSG_SMS_GETENHANCEDVMINFOIEI_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_GETENHANCEDVMINFOIEI_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_GETENHANCEDVMINFOIEI_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_GETENHANCEDVMINFOIEI_RSP_HEADER Param1;
} yPDef_MSG_SMS_GETENHANCEDVMINFOIEI_RSP;

typedef CAPI2_SMS_340AddrToTe_Rsp_t *T_INTER_TASK_MSG_SMS_340ADDRTOTE_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_340ADDRTOTE_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_340ADDRTOTE_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_340ADDRTOTE_RSP_HEADER Param1;
} yPDef_MSG_SMS_340ADDRTOTE_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SETALLNEWMSGDISPLAYPREF_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_ACKTONETWORK_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SMSRESPONSE_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_STARTMULTISMSTRANSFER_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_STARTMULTISMSTRANSFER_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_STOPMULTISMSTRANSFER_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_STOPMULTISMSTRANSFER_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CAPI2_SMS_RESP_END;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CAPI2_PBK_RESP_START;

typedef PBK_API_Name_t *T_INTER_TASK_MSG_PBK_GETALPHA_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PBK_GETALPHA_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PBK_GETALPHA_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PBK_GETALPHA_RSP_HEADER Param1;
} yPDef_MSG_PBK_GETALPHA_RSP;

typedef Boolean *T_INTER_TASK_MSG_PBK_ISEMERGENCYCALLNUMBER_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PBK_ISEMERGENCYCALLNUMBER_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PBK_ISEMERGENCYCALLNUMBER_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PBK_ISEMERGENCYCALLNUMBER_RSP_HEADER Param1;
} yPDef_MSG_PBK_ISEMERGENCYCALLNUMBER_RSP;

typedef Boolean *T_INTER_TASK_MSG_PBK_ISPARTIALEMERGENCYCALLNUMBER_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PBK_ISPARTIALEMERGENCYCALLNUMBER_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PBK_ISPARTIALEMERGENCYCALLNUMBER_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PBK_ISPARTIALEMERGENCYCALLNUMBER_RSP_HEADER Param1;
} yPDef_MSG_PBK_ISPARTIALEMERGENCYCALLNUMBER_RSP;

typedef Boolean *T_INTER_TASK_MSG_PBK_ISNUMDIALABLE_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PBK_ISNUMDIALABLE_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PBK_ISNUMDIALABLE_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PBK_ISNUMDIALABLE_RSP_HEADER Param1;
} yPDef_MSG_PBK_ISNUMDIALABLE_RSP;

typedef Boolean *T_INTER_TASK_MSG_PBK_ISUSSDDIALLABLE_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PBK_ISUSSDDIALLABLE_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PBK_ISUSSDDIALLABLE_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PBK_ISUSSDDIALLABLE_RSP_HEADER Param1;
} yPDef_MSG_PBK_ISUSSDDIALLABLE_RSP;

typedef Boolean *T_INTER_TASK_MSG_PBK_ISNUMBARRED_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PBK_ISNUMBARRED_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PBK_ISNUMBARRED_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PBK_ISNUMBARRED_RSP_HEADER Param1;
} yPDef_MSG_PBK_ISNUMBARRED_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CAPI2_PBK_RESP_END;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CAPI2_SATK_RESP_START;

typedef SetupMenu_t *T_INTER_TASK_MSG_SATK_GETCACHEDROOTMENUPTR_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SATK_GETCACHEDROOTMENUPTR_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SATK_GETCACHEDROOTMENUPTR_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SATK_GETCACHEDROOTMENUPTR_RSP_HEADER Param1;
} yPDef_MSG_SATK_GETCACHEDROOTMENUPTR_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SATK_SENDUSERACTIVITYEVENT_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SATK_SENDIDLESCREENAVAIEVENT_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SATK_SENDLANGSELECTEVENT_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SATK_SENDBROWSERTERMEVENT_RSP;

typedef Boolean *T_INTER_TASK_MSG_SATK_DATASERVCMDRESP_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SATK_DATASERVCMDRESP_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SATK_DATASERVCMDRESP_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SATK_DATASERVCMDRESP_RSP_HEADER Param1;
} yPDef_MSG_SATK_DATASERVCMDRESP_RSP;

typedef Boolean *T_INTER_TASK_MSG_SATK_CMDRESP_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SATK_CMDRESP_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SATK_CMDRESP_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SATK_CMDRESP_RSP_HEADER Param1;
} yPDef_MSG_SATK_CMDRESP_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SATK_SETTERMPROFILE_RSP;

typedef CAPI2_TermProfile_t *T_INTER_TASK_MSG_SATK_GETTERMPROFILE_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SATK_GETTERMPROFILE_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SATK_GETTERMPROFILE_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SATK_GETTERMPROFILE_RSP_HEADER Param1;
} yPDef_MSG_SATK_GETTERMPROFILE_RSP;

typedef Boolean *T_INTER_TASK_MSG_SATK_SENDTERMINALRSP_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SATK_SENDTERMINALRSP_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SATK_SENDTERMINALRSP_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SATK_SENDTERMINALRSP_RSP_HEADER Param1;
} yPDef_MSG_SATK_SENDTERMINALRSP_RSP;

typedef Boolean *T_INTER_TASK_MSG_SATK_SENDDATASERVREQ_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SATK_SENDDATASERVREQ_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SATK_SENDDATASERVREQ_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SATK_SENDDATASERVREQ_RSP_HEADER Param1;
} yPDef_MSG_SATK_SENDDATASERVREQ_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CAPI2_SATK_RESP_END;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CAPI2_CC_RESP_START;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_SENDDTMF_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_STOPDTMF_RSP;

typedef UInt8 *T_INTER_TASK_MSG_CC_GETCURRENTCALLINDEX_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GETCURRENTCALLINDEX_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GETCURRENTCALLINDEX_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GETCURRENTCALLINDEX_RSP_HEADER Param1;
} yPDef_MSG_CC_GETCURRENTCALLINDEX_RSP;

typedef UInt8 *T_INTER_TASK_MSG_CC_GETNEXTACTIVECALLINDEX_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GETNEXTACTIVECALLINDEX_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GETNEXTACTIVECALLINDEX_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GETNEXTACTIVECALLINDEX_RSP_HEADER Param1;
} yPDef_MSG_CC_GETNEXTACTIVECALLINDEX_RSP;

typedef UInt8 *T_INTER_TASK_MSG_CC_GETNEXTHELDCALLINDEX_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GETNEXTHELDCALLINDEX_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GETNEXTHELDCALLINDEX_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GETNEXTHELDCALLINDEX_RSP_HEADER Param1;
} yPDef_MSG_CC_GETNEXTHELDCALLINDEX_RSP;

typedef UInt8 *T_INTER_TASK_MSG_CC_GETNEXTWAITCALLINDEX_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GETNEXTWAITCALLINDEX_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GETNEXTWAITCALLINDEX_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GETNEXTWAITCALLINDEX_RSP_HEADER Param1;
} yPDef_MSG_CC_GETNEXTWAITCALLINDEX_RSP;

typedef UInt8 *T_INTER_TASK_MSG_CC_GETMPTYCALLINDEX_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GETMPTYCALLINDEX_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GETMPTYCALLINDEX_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GETMPTYCALLINDEX_RSP_HEADER Param1;
} yPDef_MSG_CC_GETMPTYCALLINDEX_RSP;

typedef CCallState_t *T_INTER_TASK_MSG_CC_GETCALLSTATE_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GETCALLSTATE_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GETCALLSTATE_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GETCALLSTATE_RSP_HEADER Param1;
} yPDef_MSG_CC_GETCALLSTATE_RSP;

typedef CCallType_t *T_INTER_TASK_MSG_CC_GETCALLTYPE_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GETCALLTYPE_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GETCALLTYPE_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GETCALLTYPE_RSP_HEADER Param1;
} yPDef_MSG_CC_GETCALLTYPE_RSP;

typedef Cause_t *T_INTER_TASK_MSG_CC_GETLASTCALLEXITCAUSE_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GETLASTCALLEXITCAUSE_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GETLASTCALLEXITCAUSE_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GETLASTCALLEXITCAUSE_RSP_HEADER Param1;
} yPDef_MSG_CC_GETLASTCALLEXITCAUSE_RSP;

typedef CNAP_NAME_t *T_INTER_TASK_MSG_CC_GETCNAPNAME_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GETCNAPNAME_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GETCNAPNAME_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GETCNAPNAME_RSP_HEADER Param1;
} yPDef_MSG_CC_GETCNAPNAME_RSP;

typedef PHONE_NUMBER_STR_t *T_INTER_TASK_MSG_CC_GETCALLNUMBER_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GETCALLNUMBER_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GETCALLNUMBER_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GETCALLNUMBER_RSP_HEADER Param1;
} yPDef_MSG_CC_GETCALLNUMBER_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_SETCALLNUMBER_RSP;

typedef CallingInfo_t *T_INTER_TASK_MSG_CC_GETCALLINGINFO_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GETCALLINGINFO_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GETCALLINGINFO_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GETCALLINGINFO_RSP_HEADER Param1;
} yPDef_MSG_CC_GETCALLINGINFO_RSP;

typedef ALL_CALL_STATE_t *T_INTER_TASK_MSG_CC_GETALLCALLSTATES_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GETALLCALLSTATES_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GETALLCALLSTATES_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GETALLCALLSTATES_RSP_HEADER Param1;
} yPDef_MSG_CC_GETALLCALLSTATES_RSP;

typedef ALL_CALL_INDEX_t *T_INTER_TASK_MSG_CC_GETALLCALLINDEX_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GETALLCALLINDEX_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GETALLCALLINDEX_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GETALLCALLINDEX_RSP_HEADER Param1;
} yPDef_MSG_CC_GETALLCALLINDEX_RSP;

typedef ALL_CALL_INDEX_t *T_INTER_TASK_MSG_CC_GETALLHELDCALLINDEX_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GETALLHELDCALLINDEX_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GETALLHELDCALLINDEX_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GETALLHELDCALLINDEX_RSP_HEADER Param1;
} yPDef_MSG_CC_GETALLHELDCALLINDEX_RSP;

typedef ALL_CALL_INDEX_t *T_INTER_TASK_MSG_CC_GETALLACTIVECALLINDEX_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GETALLACTIVECALLINDEX_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GETALLACTIVECALLINDEX_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GETALLACTIVECALLINDEX_RSP_HEADER Param1;
} yPDef_MSG_CC_GETALLACTIVECALLINDEX_RSP;

typedef ALL_CALL_INDEX_t *T_INTER_TASK_MSG_CC_GETALLMPTYCALLINDEX_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GETALLMPTYCALLINDEX_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GETALLMPTYCALLINDEX_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GETALLMPTYCALLINDEX_RSP_HEADER Param1;
} yPDef_MSG_CC_GETALLMPTYCALLINDEX_RSP;

typedef UInt8 *T_INTER_TASK_MSG_CC_GETNUMOFMPTYCALLS_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GETNUMOFMPTYCALLS_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GETNUMOFMPTYCALLS_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GETNUMOFMPTYCALLS_RSP_HEADER Param1;
} yPDef_MSG_CC_GETNUMOFMPTYCALLS_RSP;

typedef UInt8 *T_INTER_TASK_MSG_CC_GETNUMOFACTIVECALLS_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GETNUMOFACTIVECALLS_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GETNUMOFACTIVECALLS_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GETNUMOFACTIVECALLS_RSP_HEADER Param1;
} yPDef_MSG_CC_GETNUMOFACTIVECALLS_RSP;

typedef UInt8 *T_INTER_TASK_MSG_CC_GETNUMOFHELDCALLS_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GETNUMOFHELDCALLS_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GETNUMOFHELDCALLS_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GETNUMOFHELDCALLS_RSP_HEADER Param1;
} yPDef_MSG_CC_GETNUMOFHELDCALLS_RSP;

typedef Boolean *T_INTER_TASK_MSG_CC_ISTHEREWAITINGCALL_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_ISTHEREWAITINGCALL_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_ISTHEREWAITINGCALL_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_ISTHEREWAITINGCALL_RSP_HEADER Param1;
} yPDef_MSG_CC_ISTHEREWAITINGCALL_RSP;

typedef Boolean *T_INTER_TASK_MSG_CC_ISTHEREALERTINGCALL_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_ISTHEREALERTINGCALL_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_ISTHEREALERTINGCALL_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_ISTHEREALERTINGCALL_RSP_HEADER Param1;
} yPDef_MSG_CC_ISTHEREALERTINGCALL_RSP;

typedef PHONE_NUMBER_STR_t *T_INTER_TASK_MSG_CC_GETCONNECTEDLINEID_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GETCONNECTEDLINEID_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GETCONNECTEDLINEID_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GETCONNECTEDLINEID_RSP_HEADER Param1;
} yPDef_MSG_CC_GETCONNECTEDLINEID_RSP;

typedef Boolean *T_INTER_TASK_MSG_CC_ISMULTIPARTYCALL_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_ISMULTIPARTYCALL_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_ISMULTIPARTYCALL_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_ISMULTIPARTYCALL_RSP_HEADER Param1;
} yPDef_MSG_CC_ISMULTIPARTYCALL_RSP;

typedef Boolean *T_INTER_TASK_MSG_CC_ISVALIDDTMF_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_ISVALIDDTMF_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_ISVALIDDTMF_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_ISVALIDDTMF_RSP_HEADER Param1;
} yPDef_MSG_CC_ISVALIDDTMF_RSP;

typedef Boolean *T_INTER_TASK_MSG_CC_ISTHEREVOICECALL_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_ISTHEREVOICECALL_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_ISTHEREVOICECALL_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_ISTHEREVOICECALL_RSP_HEADER Param1;
} yPDef_MSG_CC_ISTHEREVOICECALL_RSP;

typedef Boolean *T_INTER_TASK_MSG_CC_ISCONNECTEDLINEIDPRESENTALLOWED_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_ISCONNECTEDLINEIDPRESENTALLOWED_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_ISCONNECTEDLINEIDPRESENTALLOWED_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_ISCONNECTEDLINEIDPRESENTALLOWED_RSP_HEADER Param1;
} yPDef_MSG_CC_ISCONNECTEDLINEIDPRESENTALLOWED_RSP;

typedef UInt32 *T_INTER_TASK_MSG_CC_GETCURRENTCALLDURATIONINMILLISECONDS_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GETCURRENTCALLDURATIONINMILLISECONDS_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GETCURRENTCALLDURATIONINMILLISECONDS_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GETCURRENTCALLDURATIONINMILLISECONDS_RSP_HEADER Param1;
} yPDef_MSG_CC_GETCURRENTCALLDURATIONINMILLISECONDS_RSP;

typedef UInt32 *T_INTER_TASK_MSG_CC_GETLASTCALLCCM_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GETLASTCALLCCM_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GETLASTCALLCCM_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GETLASTCALLCCM_RSP_HEADER Param1;
} yPDef_MSG_CC_GETLASTCALLCCM_RSP;

typedef UInt32 *T_INTER_TASK_MSG_CC_GETLASTCALLDURATION_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GETLASTCALLDURATION_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GETLASTCALLDURATION_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GETLASTCALLDURATION_RSP_HEADER Param1;
} yPDef_MSG_CC_GETLASTCALLDURATION_RSP;

typedef UInt32 *T_INTER_TASK_MSG_CC_GETLASTDATACALLRXBYTES_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GETLASTDATACALLRXBYTES_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GETLASTDATACALLRXBYTES_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GETLASTDATACALLRXBYTES_RSP_HEADER Param1;
} yPDef_MSG_CC_GETLASTDATACALLRXBYTES_RSP;

typedef UInt32 *T_INTER_TASK_MSG_CC_GETLASTDATACALLTXBYTES_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GETLASTDATACALLTXBYTES_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GETLASTDATACALLTXBYTES_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GETLASTDATACALLTXBYTES_RSP_HEADER Param1;
} yPDef_MSG_CC_GETLASTDATACALLTXBYTES_RSP;

typedef UInt8 *T_INTER_TASK_MSG_CC_GETDATACALLINDEX_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GETDATACALLINDEX_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GETDATACALLINDEX_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GETDATACALLINDEX_RSP_HEADER Param1;
} yPDef_MSG_CC_GETDATACALLINDEX_RSP;

typedef UInt8 *T_INTER_TASK_MSG_CC_GETCALLCLIENTID_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GETCALLCLIENTID_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GETCALLCLIENTID_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GETCALLCLIENTID_RSP_HEADER Param1;
} yPDef_MSG_CC_GETCALLCLIENTID_RSP;

typedef UInt8 *T_INTER_TASK_MSG_CC_GETTYPEADD_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GETTYPEADD_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GETTYPEADD_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GETTYPEADD_RSP_HEADER Param1;
} yPDef_MSG_CC_GETTYPEADD_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_SETVOICECALLAUTOREJECT_RSP;

typedef Boolean *T_INTER_TASK_MSG_CC_ISVOICECALLAUTOREJECT_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_ISVOICECALLAUTOREJECT_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_ISVOICECALLAUTOREJECT_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_ISVOICECALLAUTOREJECT_RSP_HEADER Param1;
} yPDef_MSG_CC_ISVOICECALLAUTOREJECT_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_SETTTYCALL_RSP;

typedef Boolean *T_INTER_TASK_MSG_CC_ISTTYENABLE_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_ISTTYENABLE_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_ISTTYENABLE_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_ISTTYENABLE_RSP_HEADER Param1;
} yPDef_MSG_CC_ISTTYENABLE_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CCAPI_SENDDTMF_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CCAPI_STOPDTMF_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CCAPI_ABORTDTMF_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CCAPI_SETDTMFTIMER_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CCAPI_RESETDTMFTIMER_RSP;

typedef Ticks_t *T_INTER_TASK_MSG_CCAPI_GETDTMFTIMER_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CCAPI_GETDTMFTIMER_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CCAPI_GETDTMFTIMER_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CCAPI_GETDTMFTIMER_RSP_HEADER Param1;
} yPDef_MSG_CCAPI_GETDTMFTIMER_RSP;

typedef Boolean *T_INTER_TASK_MSG_CC_ISSIMORIGINEDCALL_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_ISSIMORIGINEDCALL_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_ISSIMORIGINEDCALL_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_ISSIMORIGINEDCALL_RSP_HEADER Param1;
} yPDef_MSG_CC_ISSIMORIGINEDCALL_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_SETVIDEOCALLPARAM_RSP;

typedef VideoCallParam_t *T_INTER_TASK_MSG_CC_GETVIDEOCALLPARAM_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GETVIDEOCALLPARAM_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GETVIDEOCALLPARAM_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GETVIDEOCALLPARAM_RSP_HEADER Param1;
} yPDef_MSG_CC_GETVIDEOCALLPARAM_RSP;

typedef CAPI2_CallConfig_t *T_INTER_TASK_MSG_CC_GETCALLCFG_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GETCALLCFG_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GETCALLCFG_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GETCALLCFG_RSP_HEADER Param1;
} yPDef_MSG_CC_GETCALLCFG_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_SETCALLCFG_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_RESETCALLCFG_RSP;

typedef CAPI2_CCAPI_GetTiFromCallIndex_Rsp_t *T_INTER_TASK_MSG_CCAPI_GETTIFROMCALLINDEX_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CCAPI_GETTIFROMCALLINDEX_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CCAPI_GETTIFROMCALLINDEX_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CCAPI_GETTIFROMCALLINDEX_RSP_HEADER Param1;
} yPDef_MSG_CCAPI_GETTIFROMCALLINDEX_RSP;

typedef Boolean *T_INTER_TASK_MSG_CCAPI_IS_SUPPORTED_BC_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CCAPI_IS_SUPPORTED_BC_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CCAPI_IS_SUPPORTED_BC_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CCAPI_IS_SUPPORTED_BC_RSP_HEADER Param1;
} yPDef_MSG_CCAPI_IS_SUPPORTED_BC_RSP;

typedef CAPI2_CC_GetBearerCap_Rsp_t *T_INTER_TASK_MSG_CCAPI_GET_BEARERCAP_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CCAPI_GET_BEARERCAP_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CCAPI_GET_BEARERCAP_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CCAPI_GET_BEARERCAP_RSP_HEADER Param1;
} yPDef_MSG_CCAPI_GET_BEARERCAP_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CAPI2_CC_RESP_END;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CAPI2_CC_REQ_START;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_MAKEVOICECALL_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_MAKEDATACALL_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_MAKEFAXCALL_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_MAKEVIDEOCALL_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_ENDCALL_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_ENDALLCALLS_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_ENDMPTYCALLS_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_ENDHELDCALL_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_ACCEPTVOICECALL_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_ACCEPTDATACALL_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_ACCEPTWAITINGCALL_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_ACCEPTVIDEOCALL_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_HOLDCURRENTCALL_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_HOLDCALL_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_RETRIEVENEXTHELDCALL_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_RETRIEVECALL_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_SWAPCALL_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_SPLITCALL_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_JOINCALL_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_TRANSFERCALL_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_SENDDTMF_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_STOPDTMF_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_GETCURRENTCALLINDEX_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_GETNEXTACTIVECALLINDEX_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_GETNEXTHELDCALLINDEX_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_GETNEXTWAITCALLINDEX_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_GETMPTYCALLINDEX_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_GETCALLSTATE_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_GETCALLTYPE_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_GETLASTCALLEXITCAUSE_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_GETCNAPNAME_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_GETCALLNUMBER_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_GETCALLINGINFO_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_GETALLCALLSTATES_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_GETALLCALLINDEX_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_GETALLHELDCALLINDEX_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_GETALLACTIVECALLINDEX_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_GETALLMPTYCALLINDEX_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_GETNUMOFMPTYCALLS_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_GETNUMOFACTIVECALLS_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_GETNUMOFHELDCALLS_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_ISTHEREWAITINGCALL_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_ISTHEREALERTINGCALL_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_GETCONNECTEDLINEID_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_ISMULTIPARTYCALL_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_ISVALIDDTMF_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_ISTHEREVOICECALL_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_ISCONNECTEDLINEIDPRESENTALLOWED_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_GETCURRENTCALLDURATIONINMILLISECONDS_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_GETLASTCALLCCM_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_GETLASTCALLDURATION_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_GETLASTDATACALLRXBYTES_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_GETLASTDATACALLTXBYTES_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_GETDATACALLINDEX_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_GETCALLCLIENTID_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_GETTYPEADD_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_SETVOICECALLAUTOREJECT_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_ISVOICECALLAUTOREJECT_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_SETTTYCALL_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_ISTTYENABLE_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_ISSIMORIGINEDCALL_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_SETVIDEOCALLPARAM_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_GETVIDEOCALLPARAM_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_GETCALLCFG_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_SETCALLCFG_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_RESETCALLCFG_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_GETCALLINDEXINTHISSTATE_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_GETCALLINDEXINTHISSTATE_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_GETCCM_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_GETCCM_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_GETCALLPRESENT_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_GETCALLPRESENT_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CCAPI_SENDDTMF_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CCAPI_STOPDTMF_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CCAPI_ABORTDTMF_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CCAPI_SETDTMFTIMER_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CCAPI_RESETDTMFTIMER_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CCAPI_GETDTMFTIMER_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CCAPI_ENDCALL_IMMEDIATE_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CCAPI_ENDALLCALLS_IMMEDIATE_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CCAPI_GETTIFROMCALLINDEX_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CCAPI_IS_SUPPORTED_BC_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CCAPI_GET_BEARERCAP_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CAPI2_CC_REQ_END;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CAP2_INT_PBK_START;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PBK_GETALPHA_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PBK_ISEMERGENCYCALLNUMBER_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PBK_ISPARTIALEMERGENCYCALLNUMBER_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PBK_SENDINFOREQ_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SENDFINDALPHAMATCHMULTIPLEREQ_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SENDFINDALPHAMATCHONEREQ_REQ;

typedef Boolean *T_INTER_TASK_MSG_PBK_ISREADY_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PBK_ISREADY_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PBK_ISREADY_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PBK_ISREADY_RSP_HEADER Param1;
} yPDef_MSG_PBK_ISREADY_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SENDREADENTRYREQ_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PBK_SENDWRITEENTRYREQ_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PBK_SENDISNUMDIALLABLEREQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PBK_ISNUMDIALLABLE_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PBK_ISUSSDDIALLABLE_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PBK_ISREADY_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PBK_SENDUPDATEENTRYREQ_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PBK_ISNUMBARRED_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CAP2_INT_PBK_END;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CAP2_INT_SMS_START;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SENDSMSSRVCENTERNUMBERUPDATE_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_GETSMSSRVCENTERNUMBER_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_GETSIMSMSCAPACITYEXCEEDEDFLAG_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_ISSMSSERVICEAVAIL_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_GETSMSSTOREDSTATE_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_WRITESMSPDU_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_WRITESMSREQ_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SENDSMSREQ_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SENDSMSPDUREQ_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SENDSTOREDSMSREQ_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_WRITESMSPDUTOSIM_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_GETLASTTPMR_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_GETSMSTXPARAMS_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_GETTXPARAMINTEXTMODE_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SETSMSTXPARAMPROCID_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SETSMSTXPARAMCODINGTYPE_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SETSMSTXPARAMVALIDPERIOD_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SETSMSTXPARAMCOMPRESSION_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SETSMSTXPARAMREPLYPATH_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SETSMSTXPARAMUSERDATAHDRIND_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SETSMSTXPARAMSTATUSRPTREQFLAG_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SETSMSTXPARAMREJDUPL_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_DELETESMSMSGBYINDEX_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_READSMSMSG_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_LISTSMSMSG_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SETNEWMSGDISPLAYPREF_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_GETNEWMSGDISPLAYPREF_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SETSMSPREFSTORAGE_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_GETSMSPREFSTORAGE_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_GETSMSSTORAGESTATUS_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SAVESMSSERVICEPROFILE_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_RESTORESMSSERVICEPROFILE_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SETCELLBROADCASTMSGTYPE_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_CBALLOWALLCHNLREQ_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_ADDCELLBROADCASTCHNLREQ_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_REMOVECELLBROADCASTCHNLREQ_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_REMOVEALLCBCHNLFROMSEARCHLIST_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_GETCBMI_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_GETCBLANGUAGE_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_ADDCELLBROADCASTLANGREQ_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_REMOVECELLBROADCASTLANGREQ_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_STARTRECEIVINGCELLBROADCAST_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_STOPRECEIVINGCELLBROADCAST_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SETCBIGNOREDUPLFLAG_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_GETCBIGNOREDUPLFLAG_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SETVMINDONOFF_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_ISVMINDENABLED_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_GETVMWAITINGSTATUS_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_GETNUMOFVMSCNUMBER_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_GETVMSCNUMBER_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_UPDATEVMSCNUMBERREQ_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_GETSMSBEARERPREFERENCE_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SETSMSBEARERPREFERENCE_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SETSMSREADSTATUSCHANGEMODE_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_GETSMSREADSTATUSCHANGEMODE_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_CHANGESTATUSREQ_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SENDMESTOREDSTATUSIND_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SENDMERETRIEVESMSDATAIND_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SENDMEREMOVEDSTATUSIND_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SETSMSSTOREDSTATE_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_ISCACHEDDATAREADY_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_GETENHANCEDVMINFOIEI_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_340ADDRTOTE_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SETALLNEWMSGDISPLAYPREF_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_ACKTONETWORK_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SMSRESPONSE_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SENDMERETRIEVESMSDATAIND_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SENDMEREMOVEDSTATUSIND_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SENDMESTOREDSTATUSIND_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CAP2_INT_SMS_END;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CAP2_INT_STK_START;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SATK_GETCACHEDROOTMENUPTR_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SATK_SENDUSERACTIVITYEVENT_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SATK_SENDIDLESCREENAVAIEVENT_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SATK_SENDBROWSERTERMEVENT_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SATK_CMDRESP_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SATK_DATASERVCMDRESP_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SATK_SENDLANGSELECTEVENT_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SATK_SETTERMPROFILE_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SATK_GETTERMPROFILE_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SATK_SEND_ENVELOPE_CMD_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_STK_TERMINAL_RESPONSE_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_STK_SEND_BROWSING_STATUS_EVT_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_STK_SEND_BROWSING_STATUS_EVT_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SATK_SEND_CC_SETUP_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SATK_SEND_CC_SS_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SATK_SEND_CC_USSD_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SATK_SEND_CC_SMS_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SATK_SENDTERMINALRSP_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SATK_SENDDATASERVREQ_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CAP2_INT_STK_END;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CAPI2_SS_REQ_START;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SS_SENDCALLFORWARDREQ_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SS_QUERYCALLFORWARDSTATUS_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SS_SENDCALLBARRINGREQ_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SS_QUERYCALLBARRINGSTATUS_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SS_SENDCALLBARRINGPWDCHANGEREQ_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SS_SENDCALLWAITINGREQ_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SS_QUERYCALLWAITINGSTATUS_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SS_QUERYCALLINGLINEIDSTATUS_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SS_QUERYCONNECTEDLINEIDSTATUS_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SS_QUERYCALLINGLINERESTRICTIONSTATUS_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SS_QUERYCONNECTEDLINERESTRICTIONSTATUS_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SS_QUERYCALLINGNAMEPRESENTSTATUS_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SS_SENDUSSDCONNECTREQ_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SS_ENDUSSDCONNECTREQ_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SS_SENDUSSDDATA_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SS_SETCALLINGLINEIDSTATUS_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SS_SETCALLINGLINERESTRICTIONSTATUS_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SS_SETCONNECTEDLINEIDSTATUS_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SS_SETCONNECTEDLINERESTRICTIONSTATUS_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SS_ENDUSSDCONNECTREQ_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SSAPI_DIALSTRSRVREQ_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SSAPI_DIALSTRSRVREQ_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SSAPI_SSSRVREQ_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SSAPI_SSSRVREQ_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SSAPI_USSDSRVREQ_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SSAPI_USSDSRVREQ_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SSAPI_USSDDATAREQ_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SSAPI_USSDDATAREQ_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SSAPI_SSRELEASEREQ_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SSAPI_SSRELEASEREQ_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SSAPI_DATAREQ_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SSAPI_DATAREQ_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CAPI2_SS_REQ_END;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CAPI2_ISIM_REQ_START;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_ISIM_ISISIMSUPPORTED_REQ;

typedef Boolean *T_INTER_TASK_MSG_ISIM_ISISIMSUPPORTED_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_ISIM_ISISIMSUPPORTED_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_ISIM_ISISIMSUPPORTED_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_ISIM_ISISIMSUPPORTED_RSP_HEADER Param1;
} yPDef_MSG_ISIM_ISISIMSUPPORTED_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_ISIM_ISISIMACTIVATED_REQ;

typedef UInt8 *T_INTER_TASK_MSG_ISIM_ISISIMACTIVATED_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_ISIM_ISISIMACTIVATED_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_ISIM_ISISIMACTIVATED_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_ISIM_ISISIMACTIVATED_RSP_HEADER Param1;
} yPDef_MSG_ISIM_ISISIMACTIVATED_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_ISIM_ACTIVATEISIMAPPLI_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_ISIM_SENDAUTHENAKAREQ_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_ISIM_SENDAUTHENHTTPREQ_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_ISIM_SENDAUTHENGBABOOTREQ_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_ISIM_SENDAUTHENGBANAFREQ_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CAPI2_ISIM_REQ_END;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CAPI2_PCH_REQ_START;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_GETPDPCONTEXT_REQ;

typedef PDPContext_t *T_INTER_TASK_MSG_PDP_GETPDPCONTEXT_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_GETPDPCONTEXT_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_GETPDPCONTEXT_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_GETPDPCONTEXT_RSP_HEADER Param1;
} yPDef_MSG_PDP_GETPDPCONTEXT_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_SETPDPCONTEXT_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_SETPDPCONTEXT_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_DELETEPDPCONTEXT_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_DELETEPDPCONTEXT_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_SETSECPDPCONTEXT_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_SETSECPDPCONTEXT_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_GETGPRSQOS_REQ;

typedef PCHQosProfile_t *T_INTER_TASK_MSG_PDP_GETGPRSQOS_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_GETGPRSQOS_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_GETGPRSQOS_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_GETGPRSQOS_RSP_HEADER Param1;
} yPDef_MSG_PDP_GETGPRSQOS_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_SETGPRSQOS_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_SETGPRSQOS_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_DELETEGPRSQOS_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_DELETEGPRSQOS_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_GETGPRSMINQOS_REQ;

typedef PDP_GPRSMinQoS_t *T_INTER_TASK_MSG_PDP_GETGPRSMINQOS_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_GETGPRSMINQOS_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_GETGPRSMINQOS_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_GETGPRSMINQOS_RSP_HEADER Param1;
} yPDef_MSG_PDP_GETGPRSMINQOS_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_SETGPRSMINQOS_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_SETGPRSMINQOS_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_DELETEGPRSMINQOS_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_DELETEGPRSMINQOS_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_ISSECONDARYPDPDEFINED_REQ;

typedef Boolean *T_INTER_TASK_MSG_PDP_ISSECONDARYPDPDEFINED_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_ISSECONDARYPDPDEFINED_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_ISSECONDARYPDPDEFINED_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_ISSECONDARYPDPDEFINED_RSP_HEADER Param1;
} yPDef_MSG_PDP_ISSECONDARYPDPDEFINED_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_SENDPDPACTIVATEREQ_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_SENDPDPDEACTIVATEREQ_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_SENDPDPACTIVATESECREQ_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_GETGPRSACTIVATESTATUS_REQ;

typedef PDP_GetGPRSActivateStatus_Rsp_t *T_INTER_TASK_MSG_PDP_GETGPRSACTIVATESTATUS_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_GETGPRSACTIVATESTATUS_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_GETGPRSACTIVATESTATUS_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_GETGPRSACTIVATESTATUS_RSP_HEADER Param1;
} yPDef_MSG_PDP_GETGPRSACTIVATESTATUS_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_SETMSCLASS_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_SETMSCLASS_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_GETMSCLASS_REQ;

typedef MSClass_t *T_INTER_TASK_MSG_PDP_GETMSCLASS_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_GETMSCLASS_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_GETMSCLASS_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_GETMSCLASS_RSP_HEADER Param1;
} yPDef_MSG_PDP_GETMSCLASS_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_GETUMTSTFT_REQ;

typedef PCHTrafficFlowTemplate_t *T_INTER_TASK_MSG_PDP_GETUMTSTFT_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_GETUMTSTFT_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_GETUMTSTFT_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_GETUMTSTFT_RSP_HEADER Param1;
} yPDef_MSG_PDP_GETUMTSTFT_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_SETUMTSTFT_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_SETUMTSTFT_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_DELETEUMTSTFT_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_DELETEUMTSTFT_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_ACTIVATESNDCPCONNECTION_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_DEACTIVATESNDCPCONNECTION_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_DEACTIVATE_SNDCP_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_GETPDPDEFAULTCONTEXT_REQ;

typedef PDPDefaultContext_t *T_INTER_TASK_MSG_PDP_GETPDPDEFAULTCONTEXT_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_GETPDPDEFAULTCONTEXT_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_GETPDPDEFAULTCONTEXT_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_GETPDPDEFAULTCONTEXT_RSP_HEADER Param1;
} yPDef_MSG_PDP_GETPDPDEFAULTCONTEXT_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_GETPCHCONTEXT_REQ;

typedef PDP_GetPCHContext_Rsp_t *T_INTER_TASK_MSG_PDP_GETPCHCONTEXT_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_GETPCHCONTEXT_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_GETPCHCONTEXT_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_GETPCHCONTEXT_RSP_HEADER Param1;
} yPDef_MSG_PDP_GETPCHCONTEXT_RSP;

typedef PCHContextState_t *T_INTER_TASK_MSG_PDP_GETPCHCONTEXTSTATE_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_GETPCHCONTEXTSTATE_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_GETPCHCONTEXTSTATE_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_GETPCHCONTEXTSTATE_REQ_HEADER Param1;
} yPDef_MSG_PDP_GETPCHCONTEXTSTATE_REQ;

typedef PCHContextState_t *T_INTER_TASK_MSG_PDP_GETPCHCONTEXTSTATE_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_GETPCHCONTEXTSTATE_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_GETPCHCONTEXTSTATE_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_GETPCHCONTEXTSTATE_RSP_HEADER Param1;
} yPDef_MSG_PDP_GETPCHCONTEXTSTATE_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_MS_SENDCOMBINEDATTACHREQ_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_MS_SENDCOMBINEDATTACHREQ_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_MS_SENDDETACHREQ_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_MS_SENDDETACHREQ_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_MS_GETGPRSATTACHSTATUS_REQ;

typedef AttachState_t *T_INTER_TASK_MSG_MS_GETGPRSATTACHSTATUS_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_MS_GETGPRSATTACHSTATUS_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_MS_GETGPRSATTACHSTATUS_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_MS_GETGPRSATTACHSTATUS_RSP_HEADER Param1;
} yPDef_MSG_MS_GETGPRSATTACHSTATUS_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_MS_SETATTACHMODE_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_MS_SETATTACHMODE_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_MS_GETATTACHMODE_REQ;

typedef UInt8 *T_INTER_TASK_MSG_MS_GETATTACHMODE_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_MS_GETATTACHMODE_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_MS_GETATTACHMODE_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_MS_GETATTACHMODE_RSP_HEADER Param1;
} yPDef_MSG_MS_GETATTACHMODE_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_MS_ISGPRSCALLACTIVE_REQ;

typedef Boolean *T_INTER_TASK_MSG_MS_ISGPRSCALLACTIVE_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_MS_ISGPRSCALLACTIVE_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_MS_ISGPRSCALLACTIVE_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_MS_ISGPRSCALLACTIVE_RSP_HEADER Param1;
} yPDef_MSG_MS_ISGPRSCALLACTIVE_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_MS_SETCHANGPRSCALLACTIVE_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_MS_SETCHANGPRSCALLACTIVE_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_MS_SETCIDFORGPRSACTIVECHAN_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_MS_SETCIDFORGPRSACTIVECHAN_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_MS_GETGPRSACTIVECHANFROMCID_REQ;

typedef UInt8 *T_INTER_TASK_MSG_MS_GETGPRSACTIVECHANFROMCID_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_MS_GETGPRSACTIVECHANFROMCID_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_MS_GETGPRSACTIVECHANFROMCID_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_MS_GETGPRSACTIVECHANFROMCID_RSP_HEADER Param1;
} yPDef_MSG_MS_GETGPRSACTIVECHANFROMCID_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_MS_GETCIDFROMGPRSACTIVECHAN_REQ;

typedef UInt8 *T_INTER_TASK_MSG_MS_GETCIDFROMGPRSACTIVECHAN_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_MS_GETCIDFROMGPRSACTIVECHAN_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_MS_GETCIDFROMGPRSACTIVECHAN_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_MS_GETCIDFROMGPRSACTIVECHAN_RSP_HEADER Param1;
} yPDef_MSG_MS_GETCIDFROMGPRSACTIVECHAN_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_GETPPPMODEMCID_REQ;

typedef PCHCid_t *T_INTER_TASK_MSG_PDP_GETPPPMODEMCID_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_GETPPPMODEMCID_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_GETPPPMODEMCID_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_GETPPPMODEMCID_RSP_HEADER Param1;
} yPDef_MSG_PDP_GETPPPMODEMCID_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_GETPDPADDRESS_REQ;

typedef PCHPDPAddress_t *T_INTER_TASK_MSG_PDP_GETPDPADDRESS_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_GETPDPADDRESS_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_GETPDPADDRESS_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_GETPDPADDRESS_RSP_HEADER Param1;
} yPDef_MSG_PDP_GETPDPADDRESS_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_GETR99UMTSMINQOS_REQ;

typedef PCHR99QosProfile_t *T_INTER_TASK_MSG_PDP_GETR99UMTSMINQOS_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_GETR99UMTSMINQOS_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_GETR99UMTSMINQOS_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_GETR99UMTSMINQOS_RSP_HEADER Param1;
} yPDef_MSG_PDP_GETR99UMTSMINQOS_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_GETR99UMTSQOS_REQ;

typedef PCHR99QosProfile_t *T_INTER_TASK_MSG_PDP_GETR99UMTSQOS_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_GETR99UMTSQOS_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_GETR99UMTSQOS_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_GETR99UMTSQOS_RSP_HEADER Param1;
} yPDef_MSG_PDP_GETR99UMTSQOS_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_SENDTBFDATA_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_SENDTBFDATA_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_SETR99UMTSMINQOS_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_SETR99UMTSMINQOS_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_SETR99UMTSQOS_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_SETR99UMTSQOS_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_TFTADDFILTER_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_TFTADDFILTER_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_SETPCHCONTEXTSTATE_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_SETPCHCONTEXTSTATE_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_SETDEFAULTPDPCONTEXT_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_SETDEFAULTPDPCONTEXT_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_READDECODEDPROTCONFIG_REQ;

typedef PCHDecodedProtConfig_t *T_INTER_TASK_MSG_READDECODEDPROTCONFIG_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_READDECODEDPROTCONFIG_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_READDECODEDPROTCONFIG_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_READDECODEDPROTCONFIG_RSP_HEADER Param1;
} yPDef_MSG_READDECODEDPROTCONFIG_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_BUILDPROTCONFIGOPTIONS_REQ;

typedef PCHProtConfig_t *T_INTER_TASK_MSG_BUILDPROTCONFIGOPTIONS_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_BUILDPROTCONFIGOPTIONS_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_BUILDPROTCONFIGOPTIONS_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_BUILDPROTCONFIGOPTIONS_RSP_HEADER Param1;
} yPDef_MSG_BUILDPROTCONFIGOPTIONS_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_GETNEGQOS_REQ;

typedef PCHR99QosProfile_t *T_INTER_TASK_MSG_PDP_GETNEGQOS_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_GETNEGQOS_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_GETNEGQOS_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_GETNEGQOS_RSP_HEADER Param1;
} yPDef_MSG_PDP_GETNEGQOS_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_SENDPDPMODIFYREQ_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_GETNEGOTIATEDPARMS_REQ;

typedef PCHNegotiatedParms_t *T_INTER_TASK_MSG_PDP_GETNEGOTIATEDPARMS_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_GETNEGOTIATEDPARMS_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_GETNEGOTIATEDPARMS_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_GETNEGOTIATEDPARMS_RSP_HEADER Param1;
} yPDef_MSG_PDP_GETNEGOTIATEDPARMS_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_BUILDPROTCONFIGOPTIONS2_REQ;

typedef PCHProtConfig_t *T_INTER_TASK_MSG_BUILDPROTCONFIGOPTIONS2_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_BUILDPROTCONFIGOPTIONS2_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_BUILDPROTCONFIGOPTIONS2_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_BUILDPROTCONFIGOPTIONS2_RSP_HEADER Param1;
} yPDef_MSG_BUILDPROTCONFIGOPTIONS2_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_GETUMTSMINQOS_REQ;

typedef PCHUMTSQosProfile_t *T_INTER_TASK_MSG_PDP_GETUMTSMINQOS_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_GETUMTSMINQOS_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_GETUMTSMINQOS_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_GETUMTSMINQOS_RSP_HEADER Param1;
} yPDef_MSG_PDP_GETUMTSMINQOS_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_GETUMTSQOS_REQ;

typedef PCHUMTSQosProfile_t *T_INTER_TASK_MSG_PDP_GETUMTSQOS_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_GETUMTSQOS_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_GETUMTSQOS_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_GETUMTSQOS_RSP_HEADER Param1;
} yPDef_MSG_PDP_GETUMTSQOS_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_SETUMTSMINQOS_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_SETUMTSMINQOS_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_SETUMTSQOS_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_SETUMTSQOS_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_DATA_GET_GPRS_QOS_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CAPI2_PCH_REQ_END;

typedef UInt16 *T_INTER_TASK_MSG_ADC_START_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_ADC_START_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_ADC_START_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_ADC_START_RSP_HEADER Param1;
} yPDef_MSG_ADC_START_RSP;

typedef AtResponse_t *T_INTER_TASK_MSG_CAPI2_AT_RESPONSE_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CAPI2_AT_RESPONSE_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CAPI2_AT_RESPONSE_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CAPI2_AT_RESPONSE_IND_HEADER Param1;
} yPDef_MSG_CAPI2_AT_RESPONSE_IND;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_RPC_SIMPLE_REQ_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CAPI2_FROM_IPC;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CEMU_CBK_IND;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CAPI2_RAW_MSG_IND;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CAPI2_UNDETERMINED_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CAPI2_RECV_MSG_FROM_AP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CAPI2_ACK_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_GEN_REQ_START;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_MS_GSM_REGISTERED_REQ;

typedef Boolean *T_INTER_TASK_MSG_MS_GSM_REGISTERED_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_MS_GSM_REGISTERED_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_MS_GSM_REGISTERED_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_MS_GSM_REGISTERED_RSP_HEADER Param1;
} yPDef_MSG_MS_GSM_REGISTERED_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_MS_GPRS_REGISTERED_REQ;

typedef Boolean *T_INTER_TASK_MSG_MS_GPRS_REGISTERED_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_MS_GPRS_REGISTERED_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_MS_GPRS_REGISTERED_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_MS_GPRS_REGISTERED_RSP_HEADER Param1;
} yPDef_MSG_MS_GPRS_REGISTERED_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_MS_GSM_CAUSE_REQ;

typedef NetworkCause_t *T_INTER_TASK_MSG_MS_GSM_CAUSE_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_MS_GSM_CAUSE_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_MS_GSM_CAUSE_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_MS_GSM_CAUSE_RSP_HEADER Param1;
} yPDef_MSG_MS_GSM_CAUSE_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_MS_GPRS_CAUSE_REQ;

typedef NetworkCause_t *T_INTER_TASK_MSG_MS_GPRS_CAUSE_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_MS_GPRS_CAUSE_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_MS_GPRS_CAUSE_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_MS_GPRS_CAUSE_RSP_HEADER Param1;
} yPDef_MSG_MS_GPRS_CAUSE_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_MS_REGISTERED_LAC_REQ;

typedef UInt16 *T_INTER_TASK_MSG_MS_REGISTERED_LAC_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_MS_REGISTERED_LAC_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_MS_REGISTERED_LAC_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_MS_REGISTERED_LAC_RSP_HEADER Param1;
} yPDef_MSG_MS_REGISTERED_LAC_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_MS_GET_PLMN_MCC_REQ;

typedef UInt16 *T_INTER_TASK_MSG_MS_GET_PLMN_MCC_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_MS_GET_PLMN_MCC_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_MS_GET_PLMN_MCC_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_MS_GET_PLMN_MCC_RSP_HEADER Param1;
} yPDef_MSG_MS_GET_PLMN_MCC_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_MS_GET_PLMN_MNC_REQ;

typedef UInt8 *T_INTER_TASK_MSG_MS_GET_PLMN_MNC_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_MS_GET_PLMN_MNC_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_MS_GET_PLMN_MNC_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_MS_GET_PLMN_MNC_RSP_HEADER Param1;
} yPDef_MSG_MS_GET_PLMN_MNC_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SYS_POWERDOWN_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SYS_POWERDOWN_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SYS_POWERUP_NORF_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SYS_POWERUP_NORF_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SYS_NORF_CALIB_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SYS_NORF_CALIB_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SYS_POWERUP_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SYS_POWERUP_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_MS_GPRS_ALLOWED_REQ;

typedef Boolean *T_INTER_TASK_MSG_MS_GPRS_ALLOWED_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_MS_GPRS_ALLOWED_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_MS_GPRS_ALLOWED_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_MS_GPRS_ALLOWED_RSP_HEADER Param1;
} yPDef_MSG_MS_GPRS_ALLOWED_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_MS_GET_CURRENT_RAT_REQ;

typedef UInt8 *T_INTER_TASK_MSG_MS_GET_CURRENT_RAT_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_MS_GET_CURRENT_RAT_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_MS_GET_CURRENT_RAT_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_MS_GET_CURRENT_RAT_RSP_HEADER Param1;
} yPDef_MSG_MS_GET_CURRENT_RAT_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_MS_GET_CURRENT_BAND_REQ;

typedef UInt8 *T_INTER_TASK_MSG_MS_GET_CURRENT_BAND_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_MS_GET_CURRENT_BAND_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_MS_GET_CURRENT_BAND_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_MS_GET_CURRENT_BAND_RSP_HEADER Param1;
} yPDef_MSG_MS_GET_CURRENT_BAND_RSP;

typedef CAPI2_SimApi_UpdateSMSCapExceededFlag_Req_t *T_INTER_TASK_MSG_SIM_UPDATE_SMSCAPEXC_FLAG_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_UPDATE_SMSCAPEXC_FLAG_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_UPDATE_SMSCAPEXC_FLAG_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_UPDATE_SMSCAPEXC_FLAG_REQ_HEADER Param1;
} yPDef_MSG_SIM_UPDATE_SMSCAPEXC_FLAG_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIM_UPDATE_SMSCAPEXC_FLAG_RSP;

typedef CAPI2_NetRegApi_SelectBand_Req_t *T_INTER_TASK_MSG_MS_SELECT_BAND_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_MS_SELECT_BAND_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_MS_SELECT_BAND_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_MS_SELECT_BAND_REQ_HEADER Param1;
} yPDef_MSG_MS_SELECT_BAND_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_MS_SELECT_BAND_RSP;

typedef CAPI2_NetRegApi_SetSupportedRATandBand_Req_t *T_INTER_TASK_MSG_MS_SET_RAT_BAND_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_MS_SET_RAT_BAND_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_MS_SET_RAT_BAND_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_MS_SET_RAT_BAND_REQ_HEADER Param1;
} yPDef_MSG_MS_SET_RAT_BAND_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_MS_SET_RAT_BAND_RSP;

typedef CAPI2_PLMN_GetCountryByMcc_Req_t *T_INTER_TASK_MSG_MS_GET_MCC_COUNTRY_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_MS_GET_MCC_COUNTRY_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_MS_GET_MCC_COUNTRY_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_MS_GET_MCC_COUNTRY_REQ_HEADER Param1;
} yPDef_MSG_MS_GET_MCC_COUNTRY_REQ;

typedef Int8 *T_INTER_TASK_MSG_MS_GET_MCC_COUNTRY_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_MS_GET_MCC_COUNTRY_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_MS_GET_MCC_COUNTRY_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_MS_GET_MCC_COUNTRY_RSP_HEADER Param1;
} yPDef_MSG_MS_GET_MCC_COUNTRY_RSP;

typedef CAPI2_MS_GetPLMNEntryByIndex_Req_t *T_INTER_TASK_MSG_MS_PLMN_INFO_BY_CODE_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_MS_PLMN_INFO_BY_CODE_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_MS_PLMN_INFO_BY_CODE_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_MS_PLMN_INFO_BY_CODE_REQ_HEADER Param1;
} yPDef_MSG_MS_PLMN_INFO_BY_CODE_REQ;

typedef Boolean *T_INTER_TASK_MSG_MS_PLMN_INFO_BY_CODE_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_MS_PLMN_INFO_BY_CODE_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_MS_PLMN_INFO_BY_CODE_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_MS_PLMN_INFO_BY_CODE_RSP_HEADER Param1;
} yPDef_MSG_MS_PLMN_INFO_BY_CODE_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_MS_PLMN_LIST_SIZE_REQ;

typedef UInt16 *T_INTER_TASK_MSG_MS_PLMN_LIST_SIZE_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_MS_PLMN_LIST_SIZE_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_MS_PLMN_LIST_SIZE_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_MS_PLMN_LIST_SIZE_RSP_HEADER Param1;
} yPDef_MSG_MS_PLMN_LIST_SIZE_RSP;

typedef CAPI2_MS_GetPLMNByCode_Req_t *T_INTER_TASK_MSG_MS_PLMN_INFO_BY_INDEX_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_MS_PLMN_INFO_BY_INDEX_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_MS_PLMN_INFO_BY_INDEX_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_MS_PLMN_INFO_BY_INDEX_REQ_HEADER Param1;
} yPDef_MSG_MS_PLMN_INFO_BY_INDEX_REQ;

typedef Boolean *T_INTER_TASK_MSG_MS_PLMN_INFO_BY_INDEX_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_MS_PLMN_INFO_BY_INDEX_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_MS_PLMN_INFO_BY_INDEX_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_MS_PLMN_INFO_BY_INDEX_RSP_HEADER Param1;
} yPDef_MSG_MS_PLMN_INFO_BY_INDEX_RSP;

typedef CAPI2_NetRegApi_PlmnSelect_Req_t *T_INTER_TASK_MSG_PLMN_SELECT_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PLMN_SELECT_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PLMN_SELECT_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PLMN_SELECT_REQ_HEADER Param1;
} yPDef_MSG_PLMN_SELECT_REQ;

typedef Result_t *T_INTER_TASK_MSG_PLMN_SELECT_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PLMN_SELECT_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PLMN_SELECT_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PLMN_SELECT_RSP_HEADER Param1;
} yPDef_MSG_PLMN_SELECT_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_MS_PLMN_ABORT_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_MS_PLMN_ABORT_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_MS_GET_PLMN_MODE_REQ;

typedef PlmnSelectMode_t *T_INTER_TASK_MSG_MS_GET_PLMN_MODE_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_MS_GET_PLMN_MODE_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_MS_GET_PLMN_MODE_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_MS_GET_PLMN_MODE_RSP_HEADER Param1;
} yPDef_MSG_MS_GET_PLMN_MODE_RSP;

typedef CAPI2_NetRegApi_SetPlmnMode_Req_t *T_INTER_TASK_MSG_MS_SET_PLMN_MODE_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_MS_SET_PLMN_MODE_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_MS_SET_PLMN_MODE_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_MS_SET_PLMN_MODE_REQ_HEADER Param1;
} yPDef_MSG_MS_SET_PLMN_MODE_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_MS_SET_PLMN_MODE_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_MS_GET_PLMN_FORMAT_REQ;

typedef PlmnSelectFormat_t *T_INTER_TASK_MSG_MS_GET_PLMN_FORMAT_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_MS_GET_PLMN_FORMAT_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_MS_GET_PLMN_FORMAT_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_MS_GET_PLMN_FORMAT_RSP_HEADER Param1;
} yPDef_MSG_MS_GET_PLMN_FORMAT_RSP;

typedef CAPI2_MS_SetPlmnFormat_Req_t *T_INTER_TASK_MSG_MS_SET_PLMN_FORMAT_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_MS_SET_PLMN_FORMAT_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_MS_SET_PLMN_FORMAT_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_MS_SET_PLMN_FORMAT_REQ_HEADER Param1;
} yPDef_MSG_MS_SET_PLMN_FORMAT_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_MS_SET_PLMN_FORMAT_RSP;

typedef CAPI2_MS_IsMatchedPLMN_Req_t *T_INTER_TASK_MSG_MS_MATCH_PLMN_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_MS_MATCH_PLMN_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_MS_MATCH_PLMN_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_MS_MATCH_PLMN_REQ_HEADER Param1;
} yPDef_MSG_MS_MATCH_PLMN_REQ;

typedef Boolean *T_INTER_TASK_MSG_MS_MATCH_PLMN_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_MS_MATCH_PLMN_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_MS_MATCH_PLMN_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_MS_MATCH_PLMN_RSP_HEADER Param1;
} yPDef_MSG_MS_MATCH_PLMN_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_MS_SEARCH_PLMN_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_MS_ABORT_PLMN_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_MS_ABORT_PLMN_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_MS_AUTO_SEARCH_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_MS_AUTO_SEARCH_RSP;

typedef CAPI2_NetRegApi_GetPLMNNameByCode_Req_t *T_INTER_TASK_MSG_MS_PLMN_NAME_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_MS_PLMN_NAME_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_MS_PLMN_NAME_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_MS_PLMN_NAME_REQ_HEADER Param1;
} yPDef_MSG_MS_PLMN_NAME_REQ;

typedef Boolean *T_INTER_TASK_MSG_MS_PLMN_NAME_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_MS_PLMN_NAME_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_MS_PLMN_NAME_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_MS_PLMN_NAME_RSP_HEADER Param1;
} yPDef_MSG_MS_PLMN_NAME_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SYS_GET_SYSTEM_STATE_REQ;

typedef SystemState_t *T_INTER_TASK_MSG_SYS_GET_SYSTEM_STATE_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SYS_GET_SYSTEM_STATE_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SYS_GET_SYSTEM_STATE_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SYS_GET_SYSTEM_STATE_RSP_HEADER Param1;
} yPDef_MSG_SYS_GET_SYSTEM_STATE_RSP;

typedef CAPI2_PhoneCtrlApi_SetSystemState_Req_t *T_INTER_TASK_MSG_SYS_SET_SYSTEM_STATE_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SYS_SET_SYSTEM_STATE_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SYS_SET_SYSTEM_STATE_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SYS_SET_SYSTEM_STATE_REQ_HEADER Param1;
} yPDef_MSG_SYS_SET_SYSTEM_STATE_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SYS_SET_SYSTEM_STATE_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SYS_GET_RX_LEVEL_REQ;

typedef void *T_INTER_TASK_MSG_SYS_GET_RX_LEVEL_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SYS_GET_RX_LEVEL_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SYS_GET_RX_LEVEL_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SYS_GET_RX_LEVEL_RSP_HEADER Param1;
} yPDef_MSG_SYS_GET_RX_LEVEL_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SYS_GET_GSMREG_STATUS_REQ;

typedef RegisterStatus_t *T_INTER_TASK_MSG_SYS_GET_GSMREG_STATUS_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SYS_GET_GSMREG_STATUS_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SYS_GET_GSMREG_STATUS_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SYS_GET_GSMREG_STATUS_RSP_HEADER Param1;
} yPDef_MSG_SYS_GET_GSMREG_STATUS_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SYS_GET_GPRSREG_STATUS_REQ;

typedef RegisterStatus_t *T_INTER_TASK_MSG_SYS_GET_GPRSREG_STATUS_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SYS_GET_GPRSREG_STATUS_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SYS_GET_GPRSREG_STATUS_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SYS_GET_GPRSREG_STATUS_RSP_HEADER Param1;
} yPDef_MSG_SYS_GET_GPRSREG_STATUS_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SYS_GET_REG_STATUS_REQ;

typedef Boolean *T_INTER_TASK_MSG_SYS_GET_REG_STATUS_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SYS_GET_REG_STATUS_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SYS_GET_REG_STATUS_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SYS_GET_REG_STATUS_RSP_HEADER Param1;
} yPDef_MSG_SYS_GET_REG_STATUS_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_HOME_PLMN_REG_REQ;

typedef Boolean *T_INTER_TASK_MSG_HOME_PLMN_REG_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_HOME_PLMN_REG_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_HOME_PLMN_REG_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_HOME_PLMN_REG_RSP_HEADER Param1;
} yPDef_MSG_HOME_PLMN_REG_RSP;

typedef CAPI2_PhoneCtrlApi_SetPowerDownTimer_Req_t *T_INTER_TASK_MSG_SET_POWER_DOWN_TIMER_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SET_POWER_DOWN_TIMER_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SET_POWER_DOWN_TIMER_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SET_POWER_DOWN_TIMER_REQ_HEADER Param1;
} yPDef_MSG_SET_POWER_DOWN_TIMER_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SET_POWER_DOWN_TIMER_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIM_PARAM_REC_NUM_REQ;

typedef Result_t *T_INTER_TASK_MSG_SIM_PARAM_REC_NUM_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PARAM_REC_NUM_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PARAM_REC_NUM_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PARAM_REC_NUM_RSP_HEADER Param1;
} yPDef_MSG_SIM_PARAM_REC_NUM_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIM_GET_SMSMEMEXC_FLAG_REQ;

typedef Boolean *T_INTER_TASK_MSG_SIM_GET_SMSMEMEXC_FLAG_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_GET_SMSMEMEXC_FLAG_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_GET_SMSMEMEXC_FLAG_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_GET_SMSMEMEXC_FLAG_RSP_HEADER Param1;
} yPDef_MSG_SIM_GET_SMSMEMEXC_FLAG_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIM_IS_TEST_SIM_REQ;

typedef Boolean *T_INTER_TASK_MSG_SIM_IS_TEST_SIM_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_IS_TEST_SIM_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_IS_TEST_SIM_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_IS_TEST_SIM_RSP_HEADER Param1;
} yPDef_MSG_SIM_IS_TEST_SIM_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIM_PIN_REQ_REQ;

typedef Boolean *T_INTER_TASK_MSG_SIM_PIN_REQ_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PIN_REQ_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PIN_REQ_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PIN_REQ_RSP_HEADER Param1;
} yPDef_MSG_SIM_PIN_REQ_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIM_CARD_PHASE_REQ;

typedef SIMPhase_t *T_INTER_TASK_MSG_SIM_CARD_PHASE_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_CARD_PHASE_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_CARD_PHASE_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_CARD_PHASE_RSP_HEADER Param1;
} yPDef_MSG_SIM_CARD_PHASE_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIM_TYPE_REQ;

typedef SIMType_t *T_INTER_TASK_MSG_SIM_TYPE_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_TYPE_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_TYPE_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_TYPE_RSP_HEADER Param1;
} yPDef_MSG_SIM_TYPE_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIM_PRESENT_REQ;

typedef SIMPresent_t *T_INTER_TASK_MSG_SIM_PRESENT_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PRESENT_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PRESENT_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PRESENT_RSP_HEADER Param1;
} yPDef_MSG_SIM_PRESENT_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIM_PIN_OPERATION_REQ;

typedef Boolean *T_INTER_TASK_MSG_SIM_PIN_OPERATION_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PIN_OPERATION_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PIN_OPERATION_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PIN_OPERATION_RSP_HEADER Param1;
} yPDef_MSG_SIM_PIN_OPERATION_RSP;

typedef CAPI2_SimApi_IsPINBlocked_Req_t *T_INTER_TASK_MSG_SIM_PIN_BLOCK_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PIN_BLOCK_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PIN_BLOCK_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PIN_BLOCK_REQ_HEADER Param1;
} yPDef_MSG_SIM_PIN_BLOCK_REQ;

typedef Boolean *T_INTER_TASK_MSG_SIM_PIN_BLOCK_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PIN_BLOCK_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PIN_BLOCK_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PIN_BLOCK_RSP_HEADER Param1;
} yPDef_MSG_SIM_PIN_BLOCK_RSP;

typedef CAPI2_SimApi_IsPUKBlocked_Req_t *T_INTER_TASK_MSG_SIM_PUK_BLOCK_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PUK_BLOCK_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PUK_BLOCK_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PUK_BLOCK_REQ_HEADER Param1;
} yPDef_MSG_SIM_PUK_BLOCK_REQ;

typedef Boolean *T_INTER_TASK_MSG_SIM_PUK_BLOCK_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PUK_BLOCK_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PUK_BLOCK_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PUK_BLOCK_RSP_HEADER Param1;
} yPDef_MSG_SIM_PUK_BLOCK_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIM_IS_INVALID_REQ;

typedef Boolean *T_INTER_TASK_MSG_SIM_IS_INVALID_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_IS_INVALID_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_IS_INVALID_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_IS_INVALID_RSP_HEADER Param1;
} yPDef_MSG_SIM_IS_INVALID_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIM_DETECT_REQ;

typedef Boolean *T_INTER_TASK_MSG_SIM_DETECT_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_DETECT_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_DETECT_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_DETECT_RSP_HEADER Param1;
} yPDef_MSG_SIM_DETECT_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIM_GET_RUIM_SUPP_FLAG_REQ;

typedef Boolean *T_INTER_TASK_MSG_SIM_GET_RUIM_SUPP_FLAG_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_GET_RUIM_SUPP_FLAG_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_GET_RUIM_SUPP_FLAG_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_GET_RUIM_SUPP_FLAG_RSP_HEADER Param1;
} yPDef_MSG_SIM_GET_RUIM_SUPP_FLAG_RSP;

typedef CAPI2_SimApi_SendVerifyChvReq_Req_t *T_INTER_TASK_MSG_SIM_VERIFY_CHV_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_VERIFY_CHV_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_VERIFY_CHV_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_VERIFY_CHV_REQ_HEADER Param1;
} yPDef_MSG_SIM_VERIFY_CHV_REQ;

typedef CAPI2_SimApi_SendChangeChvReq_Req_t *T_INTER_TASK_MSG_SIM_CHANGE_CHV_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_CHANGE_CHV_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_CHANGE_CHV_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_CHANGE_CHV_REQ_HEADER Param1;
} yPDef_MSG_SIM_CHANGE_CHV_REQ;

typedef CAPI2_SimApi_SendSetChv1OnOffReq_Req_t *T_INTER_TASK_MSG_SIM_ENABLE_CHV_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_ENABLE_CHV_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_ENABLE_CHV_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_ENABLE_CHV_REQ_HEADER Param1;
} yPDef_MSG_SIM_ENABLE_CHV_REQ;

typedef CAPI2_SimApi_SendUnblockChvReq_Req_t *T_INTER_TASK_MSG_SIM_UNBLOCK_CHV_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_UNBLOCK_CHV_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_UNBLOCK_CHV_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_UNBLOCK_CHV_REQ_HEADER Param1;
} yPDef_MSG_SIM_UNBLOCK_CHV_REQ;

typedef CAPI2_SimApi_SendSetOperStateReq_Req_t *T_INTER_TASK_MSG_SIM_SET_FDN_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_SET_FDN_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_SET_FDN_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_SET_FDN_REQ_HEADER Param1;
} yPDef_MSG_SIM_SET_FDN_REQ;

typedef CAPI2_SimApi_IsPbkAccessAllowed_Req_t *T_INTER_TASK_MSG_SIM_IS_PBK_ALLOWED_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_IS_PBK_ALLOWED_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_IS_PBK_ALLOWED_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_IS_PBK_ALLOWED_REQ_HEADER Param1;
} yPDef_MSG_SIM_IS_PBK_ALLOWED_REQ;

typedef Boolean *T_INTER_TASK_MSG_SIM_IS_PBK_ALLOWED_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_IS_PBK_ALLOWED_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_IS_PBK_ALLOWED_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_IS_PBK_ALLOWED_RSP_HEADER Param1;
} yPDef_MSG_SIM_IS_PBK_ALLOWED_RSP;

typedef CAPI2_SimApi_SendPbkInfoReq_Req_t *T_INTER_TASK_MSG_SIM_PBK_INFO_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PBK_INFO_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PBK_INFO_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PBK_INFO_REQ_HEADER Param1;
} yPDef_MSG_SIM_PBK_INFO_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIM_MAX_ACM_REQ;

typedef CAPI2_SimApi_SendWriteAcmMaxReq_Req_t *T_INTER_TASK_MSG_SIM_ACM_MAX_UPDATE_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_ACM_MAX_UPDATE_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_ACM_MAX_UPDATE_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_ACM_MAX_UPDATE_REQ_HEADER Param1;
} yPDef_MSG_SIM_ACM_MAX_UPDATE_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIM_ACM_VALUE_REQ;

typedef CAPI2_SimApi_SendWriteAcmReq_Req_t *T_INTER_TASK_MSG_SIM_ACM_UPDATE_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_ACM_UPDATE_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_ACM_UPDATE_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_ACM_UPDATE_REQ_HEADER Param1;
} yPDef_MSG_SIM_ACM_UPDATE_REQ;

typedef CAPI2_SimApi_SendIncreaseAcmReq_Req_t *T_INTER_TASK_MSG_SIM_ACM_INCREASE_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_ACM_INCREASE_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_ACM_INCREASE_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_ACM_INCREASE_REQ_HEADER Param1;
} yPDef_MSG_SIM_ACM_INCREASE_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIM_SVC_PROV_NAME_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIM_PUCT_DATA_REQ;

typedef CAPI2_SimApi_GetServiceStatus_Req_t *T_INTER_TASK_MSG_SIM_SERVICE_STATUS_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_SERVICE_STATUS_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_SERVICE_STATUS_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_SERVICE_STATUS_REQ_HEADER Param1;
} yPDef_MSG_SIM_SERVICE_STATUS_REQ;

typedef SIMServiceStatus_t *T_INTER_TASK_MSG_SIM_SERVICE_STATUS_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_SERVICE_STATUS_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_SERVICE_STATUS_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_SERVICE_STATUS_RSP_HEADER Param1;
} yPDef_MSG_SIM_SERVICE_STATUS_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIM_PIN_STATUS_REQ;

typedef SIM_PIN_Status_t *T_INTER_TASK_MSG_SIM_PIN_STATUS_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PIN_STATUS_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PIN_STATUS_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PIN_STATUS_RSP_HEADER Param1;
} yPDef_MSG_SIM_PIN_STATUS_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIM_PIN_OK_STATUS_REQ;

typedef Boolean *T_INTER_TASK_MSG_SIM_PIN_OK_STATUS_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PIN_OK_STATUS_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PIN_OK_STATUS_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PIN_OK_STATUS_RSP_HEADER Param1;
} yPDef_MSG_SIM_PIN_OK_STATUS_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIM_IMSI_REQ;

typedef IMSI_t *T_INTER_TASK_MSG_SIM_IMSI_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_IMSI_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_IMSI_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_IMSI_RSP_HEADER Param1;
} yPDef_MSG_SIM_IMSI_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIM_GID_DIGIT_REQ;

typedef GID_DIGIT_t *T_INTER_TASK_MSG_SIM_GID_DIGIT_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_GID_DIGIT_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_GID_DIGIT_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_GID_DIGIT_RSP_HEADER Param1;
} yPDef_MSG_SIM_GID_DIGIT_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIM_GID_DIGIT2_REQ;

typedef GID_DIGIT_t *T_INTER_TASK_MSG_SIM_GID_DIGIT2_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_GID_DIGIT2_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_GID_DIGIT2_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_GID_DIGIT2_RSP_HEADER Param1;
} yPDef_MSG_SIM_GID_DIGIT2_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIM_HOME_PLMN_REQ;

typedef void *T_INTER_TASK_MSG_SIM_HOME_PLMN_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_HOME_PLMN_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_HOME_PLMN_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_HOME_PLMN_RSP_HEADER Param1;
} yPDef_MSG_SIM_HOME_PLMN_RSP;

typedef CAPI2_simmiApi_GetMasterFileId_Req_t *T_INTER_TASK_MSG_SIM_APDU_FILEID_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_APDU_FILEID_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_APDU_FILEID_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_APDU_FILEID_REQ_HEADER Param1;
} yPDef_MSG_SIM_APDU_FILEID_REQ;

typedef APDUFileID_t *T_INTER_TASK_MSG_SIM_APDU_FILEID_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_APDU_FILEID_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_APDU_FILEID_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_APDU_FILEID_RSP_HEADER Param1;
} yPDef_MSG_SIM_APDU_FILEID_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIM_OPEN_SOCKET_REQ;

typedef CAPI2_SimApi_SendSelectAppiReq_Req_t *T_INTER_TASK_MSG_SIM_SELECT_APPLI_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_SELECT_APPLI_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_SELECT_APPLI_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_SELECT_APPLI_REQ_HEADER Param1;
} yPDef_MSG_SIM_SELECT_APPLI_REQ;

typedef CAPI2_SimApi_SendDeactivateAppiReq_Req_t *T_INTER_TASK_MSG_SIM_DEACTIVATE_APPLI_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_DEACTIVATE_APPLI_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_DEACTIVATE_APPLI_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_DEACTIVATE_APPLI_REQ_HEADER Param1;
} yPDef_MSG_SIM_DEACTIVATE_APPLI_REQ;

typedef CAPI2_SimApi_SendCloseSocketReq_Req_t *T_INTER_TASK_MSG_SIM_CLOSE_SOCKET_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_CLOSE_SOCKET_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_CLOSE_SOCKET_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_CLOSE_SOCKET_REQ_HEADER Param1;
} yPDef_MSG_SIM_CLOSE_SOCKET_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIM_ATR_DATA_REQ;

typedef SIMAccess_t *T_INTER_TASK_MSG_SIM_ATR_DATA_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_ATR_DATA_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_ATR_DATA_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_ATR_DATA_RSP_HEADER Param1;
} yPDef_MSG_SIM_ATR_DATA_RSP;

typedef CAPI2_SIM_SubmitDFileInfoReqOld_Req_t *T_INTER_TASK_MSG_SIM_DFILE_INFO_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_DFILE_INFO_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_DFILE_INFO_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_DFILE_INFO_REQ_HEADER Param1;
} yPDef_MSG_SIM_DFILE_INFO_REQ;

typedef CAPI2_SimApi_SubmitEFileInfoReq_Req_t *T_INTER_TASK_MSG_SIM_EFILE_INFO_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_EFILE_INFO_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_EFILE_INFO_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_EFILE_INFO_REQ_HEADER Param1;
} yPDef_MSG_SIM_EFILE_INFO_REQ;

typedef CAPI2_SimApi_SendEFileInfoReq_Req_t *T_INTER_TASK_MSG_SIM_SEND_EFILE_INFO_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_SEND_EFILE_INFO_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_SEND_EFILE_INFO_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_SEND_EFILE_INFO_REQ_HEADER Param1;
} yPDef_MSG_SIM_SEND_EFILE_INFO_REQ;

typedef CAPI2_SimApi_SendDFileInfoReq_Req_t *T_INTER_TASK_MSG_SIM_SEND_DFILE_INFO_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_SEND_DFILE_INFO_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_SEND_DFILE_INFO_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_SEND_DFILE_INFO_REQ_HEADER Param1;
} yPDef_MSG_SIM_SEND_DFILE_INFO_REQ;

typedef CAPI2_SimApi_SubmitWholeBinaryEFileReadReq_Req_t *T_INTER_TASK_MSG_SIM_WHOLE_EFILE_DATA_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_WHOLE_EFILE_DATA_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_WHOLE_EFILE_DATA_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_WHOLE_EFILE_DATA_REQ_HEADER Param1;
} yPDef_MSG_SIM_WHOLE_EFILE_DATA_REQ;

typedef CAPI2_SimApi_SendWholeBinaryEFileReadReq_Req_t *T_INTER_TASK_MSG_SIM_SEND_WHOLE_EFILE_DATA_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_SEND_WHOLE_EFILE_DATA_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_SEND_WHOLE_EFILE_DATA_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_SEND_WHOLE_EFILE_DATA_REQ_HEADER Param1;
} yPDef_MSG_SIM_SEND_WHOLE_EFILE_DATA_REQ;

typedef CAPI2_SimApi_SubmitBinaryEFileReadReq_Req_t *T_INTER_TASK_MSG_SIM_EFILE_DATA_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_EFILE_DATA_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_EFILE_DATA_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_EFILE_DATA_REQ_HEADER Param1;
} yPDef_MSG_SIM_EFILE_DATA_REQ;

typedef CAPI2_SimApi_SendBinaryEFileReadReq_Req_t *T_INTER_TASK_MSG_SIM_SEND_EFILE_DATA_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_SEND_EFILE_DATA_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_SEND_EFILE_DATA_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_SEND_EFILE_DATA_REQ_HEADER Param1;
} yPDef_MSG_SIM_SEND_EFILE_DATA_REQ;

typedef CAPI2_SimApi_SubmitRecordEFileReadReq_Req_t *T_INTER_TASK_MSG_SIM_RECORD_EFILE_DATA_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_RECORD_EFILE_DATA_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_RECORD_EFILE_DATA_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_RECORD_EFILE_DATA_REQ_HEADER Param1;
} yPDef_MSG_SIM_RECORD_EFILE_DATA_REQ;

typedef CAPI2_SimApi_SendRecordEFileReadReq_Req_t *T_INTER_TASK_MSG_SIM_SEND_RECORD_EFILE_DATA_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_SEND_RECORD_EFILE_DATA_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_SEND_RECORD_EFILE_DATA_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_SEND_RECORD_EFILE_DATA_REQ_HEADER Param1;
} yPDef_MSG_SIM_SEND_RECORD_EFILE_DATA_REQ;

typedef CAPI2_SimApi_SubmitBinaryEFileUpdateReq_Req_t *T_INTER_TASK_MSG_SIM_EFILE_UPDATE_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_EFILE_UPDATE_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_EFILE_UPDATE_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_EFILE_UPDATE_REQ_HEADER Param1;
} yPDef_MSG_SIM_EFILE_UPDATE_REQ;

typedef CAPI2_SimApi_SendBinaryEFileUpdateReq_Req_t *T_INTER_TASK_MSG_SIM_SEND_EFILE_UPDATE_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_SEND_EFILE_UPDATE_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_SEND_EFILE_UPDATE_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_SEND_EFILE_UPDATE_REQ_HEADER Param1;
} yPDef_MSG_SIM_SEND_EFILE_UPDATE_REQ;

typedef CAPI2_SimApi_SubmitLinearEFileUpdateReq_Req_t *T_INTER_TASK_MSG_SIM_LINEAR_EFILE_UPDATE_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_LINEAR_EFILE_UPDATE_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_LINEAR_EFILE_UPDATE_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_LINEAR_EFILE_UPDATE_REQ_HEADER Param1;
} yPDef_MSG_SIM_LINEAR_EFILE_UPDATE_REQ;

typedef CAPI2_SimApi_SendLinearEFileUpdateReq_Req_t *T_INTER_TASK_MSG_SIM_SEND_LINEAR_EFILE_UPDATE_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_SEND_LINEAR_EFILE_UPDATE_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_SEND_LINEAR_EFILE_UPDATE_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_SEND_LINEAR_EFILE_UPDATE_REQ_HEADER Param1;
} yPDef_MSG_SIM_SEND_LINEAR_EFILE_UPDATE_REQ;

typedef CAPI2_SimApi_SubmitSeekRecordReq_Req_t *T_INTER_TASK_MSG_SIM_SEEK_RECORD_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_SEEK_RECORD_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_SEEK_RECORD_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_SEEK_RECORD_REQ_HEADER Param1;
} yPDef_MSG_SIM_SEEK_RECORD_REQ;

typedef CAPI2_SimApi_SendSeekRecordReq_Req_t *T_INTER_TASK_MSG_SIM_SEND_SEEK_RECORD_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_SEND_SEEK_RECORD_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_SEND_SEEK_RECORD_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_SEND_SEEK_RECORD_REQ_HEADER Param1;
} yPDef_MSG_SIM_SEND_SEEK_RECORD_REQ;

typedef CAPI2_SimApi_SubmitCyclicEFileUpdateReq_Req_t *T_INTER_TASK_MSG_SIM_CYCLIC_EFILE_UPDATE_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_CYCLIC_EFILE_UPDATE_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_CYCLIC_EFILE_UPDATE_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_CYCLIC_EFILE_UPDATE_REQ_HEADER Param1;
} yPDef_MSG_SIM_CYCLIC_EFILE_UPDATE_REQ;

typedef CAPI2_SimApi_SendCyclicEFileUpdateReq_Req_t *T_INTER_TASK_MSG_SIM_SEND_CYCLIC_EFILE_UPDATE_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_SEND_CYCLIC_EFILE_UPDATE_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_SEND_CYCLIC_EFILE_UPDATE_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_SEND_CYCLIC_EFILE_UPDATE_REQ_HEADER Param1;
} yPDef_MSG_SIM_SEND_CYCLIC_EFILE_UPDATE_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIM_PIN_ATTEMPT_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIM_CACHE_DATA_READY_REQ;

typedef Boolean *T_INTER_TASK_MSG_SIM_CACHE_DATA_READY_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_CACHE_DATA_READY_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_CACHE_DATA_READY_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_CACHE_DATA_READY_RSP_HEADER Param1;
} yPDef_MSG_SIM_CACHE_DATA_READY_RSP;

typedef CAPI2_SimApi_GetServiceCodeStatus_Req_t *T_INTER_TASK_MSG_SIM_SERVICE_CODE_STATUS_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_SERVICE_CODE_STATUS_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_SERVICE_CODE_STATUS_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_SERVICE_CODE_STATUS_REQ_HEADER Param1;
} yPDef_MSG_SIM_SERVICE_CODE_STATUS_REQ;

typedef SERVICE_FLAG_STATUS_t *T_INTER_TASK_MSG_SIM_SERVICE_CODE_STATUS_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_SERVICE_CODE_STATUS_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_SERVICE_CODE_STATUS_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_SERVICE_CODE_STATUS_RSP_HEADER Param1;
} yPDef_MSG_SIM_SERVICE_CODE_STATUS_RSP;

typedef CAPI2_SimApi_CheckCphsService_Req_t *T_INTER_TASK_MSG_SIM_CHECK_CPHS_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_CHECK_CPHS_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_CHECK_CPHS_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_CHECK_CPHS_REQ_HEADER Param1;
} yPDef_MSG_SIM_CHECK_CPHS_REQ;

typedef Boolean *T_INTER_TASK_MSG_SIM_CHECK_CPHS_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_CHECK_CPHS_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_CHECK_CPHS_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_CHECK_CPHS_RSP_HEADER Param1;
} yPDef_MSG_SIM_CHECK_CPHS_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIM_CPHS_PHASE_REQ;

typedef UInt8 *T_INTER_TASK_MSG_SIM_CPHS_PHASE_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_CPHS_PHASE_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_CPHS_PHASE_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_CPHS_PHASE_RSP_HEADER Param1;
} yPDef_MSG_SIM_CPHS_PHASE_RSP;

typedef CAPI2_SimApi_GetSmsSca_Req_t *T_INTER_TASK_MSG_SIM_SMS_SCA_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_SMS_SCA_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_SMS_SCA_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_SMS_SCA_REQ_HEADER Param1;
} yPDef_MSG_SIM_SMS_SCA_REQ;

typedef Result_t *T_INTER_TASK_MSG_SIM_SMS_SCA_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_SMS_SCA_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_SMS_SCA_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_SMS_SCA_RSP_HEADER Param1;
} yPDef_MSG_SIM_SMS_SCA_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIM_ICCID_PARAM_REQ;

typedef Result_t *T_INTER_TASK_MSG_SIM_ICCID_PARAM_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_ICCID_PARAM_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_ICCID_PARAM_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_ICCID_PARAM_RSP_HEADER Param1;
} yPDef_MSG_SIM_ICCID_PARAM_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIM_ALS_STATUS_REQ;

typedef Boolean *T_INTER_TASK_MSG_SIM_ALS_STATUS_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_ALS_STATUS_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_ALS_STATUS_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_ALS_STATUS_RSP_HEADER Param1;
} yPDef_MSG_SIM_ALS_STATUS_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIM_ALS_DEFAULT_LINE_REQ;

typedef UInt8 *T_INTER_TASK_MSG_SIM_ALS_DEFAULT_LINE_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_ALS_DEFAULT_LINE_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_ALS_DEFAULT_LINE_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_ALS_DEFAULT_LINE_RSP_HEADER Param1;
} yPDef_MSG_SIM_ALS_DEFAULT_LINE_RSP;

typedef CAPI2_SimApi_SetAlsDefaultLine_Req_t *T_INTER_TASK_MSG_SIM_SET_ALS_DEFAULT_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_SET_ALS_DEFAULT_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_SET_ALS_DEFAULT_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_SET_ALS_DEFAULT_REQ_HEADER Param1;
} yPDef_MSG_SIM_SET_ALS_DEFAULT_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIM_SET_ALS_DEFAULT_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIM_CALLFWD_COND_FLAG_REQ;

typedef SIM_CALL_FORWARD_UNCONDITIONAL_FLAG_t *T_INTER_TASK_MSG_SIM_CALLFWD_COND_FLAG_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_CALLFWD_COND_FLAG_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_CALLFWD_COND_FLAG_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_CALLFWD_COND_FLAG_RSP_HEADER Param1;
} yPDef_MSG_SIM_CALLFWD_COND_FLAG_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIM_APP_TYPE_REQ;

typedef SIM_APPL_TYPE_t *T_INTER_TASK_MSG_SIM_APP_TYPE_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_APP_TYPE_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_APP_TYPE_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_APP_TYPE_RSP_HEADER Param1;
} yPDef_MSG_SIM_APP_TYPE_RSP;

typedef CAPI2_SimApi_SendWritePuctReq_Req_t *T_INTER_TASK_MSG_SIM_PUCT_UPDATE_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PUCT_UPDATE_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PUCT_UPDATE_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PUCT_UPDATE_REQ_HEADER Param1;
} yPDef_MSG_SIM_PUCT_UPDATE_REQ;

typedef CAPI2_SimApi_SubmitRestrictedAccessReq_Req_t *T_INTER_TASK_MSG_SIM_RESTRICTED_ACCESS_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_RESTRICTED_ACCESS_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_RESTRICTED_ACCESS_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_RESTRICTED_ACCESS_REQ_HEADER Param1;
} yPDef_MSG_SIM_RESTRICTED_ACCESS_REQ;

typedef CAPI2_ADCMGR_Start_Req_t *T_INTER_TASK_MSG_ADC_START_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_ADC_START_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_ADC_START_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_ADC_START_REQ_HEADER Param1;
} yPDef_MSG_ADC_START_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_MS_GET_SYSTEM_RAT_REQ;

typedef RATSelect_t *T_INTER_TASK_MSG_MS_GET_SYSTEM_RAT_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_MS_GET_SYSTEM_RAT_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_MS_GET_SYSTEM_RAT_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_MS_GET_SYSTEM_RAT_RSP_HEADER Param1;
} yPDef_MSG_MS_GET_SYSTEM_RAT_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_MS_GET_SUPPORTED_RAT_REQ;

typedef RATSelect_t *T_INTER_TASK_MSG_MS_GET_SUPPORTED_RAT_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_MS_GET_SUPPORTED_RAT_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_MS_GET_SUPPORTED_RAT_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_MS_GET_SUPPORTED_RAT_RSP_HEADER Param1;
} yPDef_MSG_MS_GET_SUPPORTED_RAT_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_MS_GET_SYSTEM_BAND_REQ;

typedef BandSelect_t *T_INTER_TASK_MSG_MS_GET_SYSTEM_BAND_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_MS_GET_SYSTEM_BAND_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_MS_GET_SYSTEM_BAND_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_MS_GET_SYSTEM_BAND_RSP_HEADER Param1;
} yPDef_MSG_MS_GET_SYSTEM_BAND_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_MS_GET_SUPPORTED_BAND_REQ;

typedef BandSelect_t *T_INTER_TASK_MSG_MS_GET_SUPPORTED_BAND_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_MS_GET_SUPPORTED_BAND_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_MS_GET_SUPPORTED_BAND_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_MS_GET_SUPPORTED_BAND_RSP_HEADER Param1;
} yPDef_MSG_MS_GET_SUPPORTED_BAND_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SYSPARAM_GET_MSCLASS_REQ;

typedef UInt16 *T_INTER_TASK_MSG_SYSPARAM_GET_MSCLASS_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SYSPARAM_GET_MSCLASS_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SYSPARAM_GET_MSCLASS_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SYSPARAM_GET_MSCLASS_RSP_HEADER Param1;
} yPDef_MSG_SYSPARAM_GET_MSCLASS_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SYSPARAM_GET_MNF_NAME_REQ;

typedef uchar_ptr_t *T_INTER_TASK_MSG_SYSPARAM_GET_MNF_NAME_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SYSPARAM_GET_MNF_NAME_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SYSPARAM_GET_MNF_NAME_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SYSPARAM_GET_MNF_NAME_RSP_HEADER Param1;
} yPDef_MSG_SYSPARAM_GET_MNF_NAME_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SYSPARAM_GET_MODEL_NAME_REQ;

typedef uchar_ptr_t *T_INTER_TASK_MSG_SYSPARAM_GET_MODEL_NAME_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SYSPARAM_GET_MODEL_NAME_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SYSPARAM_GET_MODEL_NAME_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SYSPARAM_GET_MODEL_NAME_RSP_HEADER Param1;
} yPDef_MSG_SYSPARAM_GET_MODEL_NAME_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SYSPARAM_GET_SW_VERSION_REQ;

typedef uchar_ptr_t *T_INTER_TASK_MSG_SYSPARAM_GET_SW_VERSION_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SYSPARAM_GET_SW_VERSION_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SYSPARAM_GET_SW_VERSION_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SYSPARAM_GET_SW_VERSION_RSP_HEADER Param1;
} yPDef_MSG_SYSPARAM_GET_SW_VERSION_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SYSPARAM_GET_EGPRS_CLASS_REQ;

typedef UInt16 *T_INTER_TASK_MSG_SYSPARAM_GET_EGPRS_CLASS_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SYSPARAM_GET_EGPRS_CLASS_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SYSPARAM_GET_EGPRS_CLASS_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SYSPARAM_GET_EGPRS_CLASS_RSP_HEADER Param1;
} yPDef_MSG_SYSPARAM_GET_EGPRS_CLASS_RSP;

typedef CAPI2_SimApi_SendNumOfPLMNEntryReq_Req_t *T_INTER_TASK_MSG_SIM_PLMN_NUM_OF_ENTRY_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PLMN_NUM_OF_ENTRY_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PLMN_NUM_OF_ENTRY_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PLMN_NUM_OF_ENTRY_REQ_HEADER Param1;
} yPDef_MSG_SIM_PLMN_NUM_OF_ENTRY_REQ;

typedef CAPI2_SimApi_SendReadPLMNEntryReq_Req_t *T_INTER_TASK_MSG_SIM_PLMN_ENTRY_DATA_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PLMN_ENTRY_DATA_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PLMN_ENTRY_DATA_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PLMN_ENTRY_DATA_REQ_HEADER Param1;
} yPDef_MSG_SIM_PLMN_ENTRY_DATA_REQ;

typedef CAPI2_SimApi_SendWriteMulPLMNEntryReq_Req_t *T_INTER_TASK_MSG_SIM_MUL_PLMN_ENTRY_UPDATE_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_MUL_PLMN_ENTRY_UPDATE_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_MUL_PLMN_ENTRY_UPDATE_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_MUL_PLMN_ENTRY_UPDATE_REQ_HEADER Param1;
} yPDef_MSG_SIM_MUL_PLMN_ENTRY_UPDATE_REQ;

typedef CAPI2_SYS_SetRegisteredEventMask_Req_t *T_INTER_TASK_MSG_SYS_SET_REG_EVENT_MASK_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SYS_SET_REG_EVENT_MASK_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SYS_SET_REG_EVENT_MASK_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SYS_SET_REG_EVENT_MASK_REQ_HEADER Param1;
} yPDef_MSG_SYS_SET_REG_EVENT_MASK_REQ;

typedef Boolean *T_INTER_TASK_MSG_SYS_SET_REG_EVENT_MASK_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SYS_SET_REG_EVENT_MASK_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SYS_SET_REG_EVENT_MASK_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SYS_SET_REG_EVENT_MASK_RSP_HEADER Param1;
} yPDef_MSG_SYS_SET_REG_EVENT_MASK_RSP;

typedef CAPI2_SYS_SetFilteredEventMask_Req_t *T_INTER_TASK_MSG_SYS_SET_REG_FILTER_MASK_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SYS_SET_REG_FILTER_MASK_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SYS_SET_REG_FILTER_MASK_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SYS_SET_REG_FILTER_MASK_REQ_HEADER Param1;
} yPDef_MSG_SYS_SET_REG_FILTER_MASK_REQ;

typedef Boolean *T_INTER_TASK_MSG_SYS_SET_REG_FILTER_MASK_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SYS_SET_REG_FILTER_MASK_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SYS_SET_REG_FILTER_MASK_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SYS_SET_REG_FILTER_MASK_RSP_HEADER Param1;
} yPDef_MSG_SYS_SET_REG_FILTER_MASK_RSP;

typedef CAPI2_PhoneCtrlApi_SetRssiThreshold_Req_t *T_INTER_TASK_MSG_SYS_SET_RSSI_THRESHOLD_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SYS_SET_RSSI_THRESHOLD_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SYS_SET_RSSI_THRESHOLD_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SYS_SET_RSSI_THRESHOLD_REQ_HEADER Param1;
} yPDef_MSG_SYS_SET_RSSI_THRESHOLD_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SYS_SET_RSSI_THRESHOLD_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SYSPARAM_GET_CHANNEL_MODE_REQ;

typedef UInt16 *T_INTER_TASK_MSG_SYSPARAM_GET_CHANNEL_MODE_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SYSPARAM_GET_CHANNEL_MODE_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SYSPARAM_GET_CHANNEL_MODE_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SYSPARAM_GET_CHANNEL_MODE_RSP_HEADER Param1;
} yPDef_MSG_SYSPARAM_GET_CHANNEL_MODE_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SYSPARAM_GET_CLASSMARK_REQ;

typedef CAPI2_Class_t *T_INTER_TASK_MSG_SYSPARAM_GET_CLASSMARK_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SYSPARAM_GET_CLASSMARK_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SYSPARAM_GET_CLASSMARK_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SYSPARAM_GET_CLASSMARK_RSP_HEADER Param1;
} yPDef_MSG_SYSPARAM_GET_CLASSMARK_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SYSPARAM_GET_IND_FILE_VER_REQ;

typedef UInt16 *T_INTER_TASK_MSG_SYSPARAM_GET_IND_FILE_VER_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SYSPARAM_GET_IND_FILE_VER_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SYSPARAM_GET_IND_FILE_VER_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SYSPARAM_GET_IND_FILE_VER_RSP_HEADER Param1;
} yPDef_MSG_SYSPARAM_GET_IND_FILE_VER_RSP;

typedef CAPI2_SYSPARM_SetDARPCfg_Req_t *T_INTER_TASK_MSG_SYS_SET_DARP_CFG_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SYS_SET_DARP_CFG_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SYS_SET_DARP_CFG_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SYS_SET_DARP_CFG_REQ_HEADER Param1;
} yPDef_MSG_SYS_SET_DARP_CFG_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SYS_SET_DARP_CFG_RSP;

typedef CAPI2_SYSPARM_SetEGPRSMSClass_Req_t *T_INTER_TASK_MSG_SYSPARAM_SET_EGPRS_MSCLASS_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SYSPARAM_SET_EGPRS_MSCLASS_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SYSPARAM_SET_EGPRS_MSCLASS_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SYSPARAM_SET_EGPRS_MSCLASS_REQ_HEADER Param1;
} yPDef_MSG_SYSPARAM_SET_EGPRS_MSCLASS_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SYSPARAM_SET_EGPRS_MSCLASS_RSP;

typedef CAPI2_SYSPARM_SetGPRSMSClass_Req_t *T_INTER_TASK_MSG_SYSPARAM_SET_GPRS_MSCLASS_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SYSPARAM_SET_GPRS_MSCLASS_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SYSPARAM_SET_GPRS_MSCLASS_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SYSPARAM_SET_GPRS_MSCLASS_REQ_HEADER Param1;
} yPDef_MSG_SYSPARAM_SET_GPRS_MSCLASS_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SYSPARAM_SET_GPRS_MSCLASS_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_TIMEZONE_DELETE_NW_NAME_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_TIMEZONE_DELETE_NW_NAME_RSP;

typedef CAPI2_TestCmds_Req_t *T_INTER_TASK_MSG_CAPI2_TEST_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CAPI2_TEST_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CAPI2_TEST_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CAPI2_TEST_REQ_HEADER Param1;
} yPDef_MSG_CAPI2_TEST_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CAPI2_TEST_RSP;

typedef CAPI2_SatkApi_SendPlayToneRes_Req_t *T_INTER_TASK_MSG_STK_SEND_PLAYTONE_RES_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_STK_SEND_PLAYTONE_RES_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_STK_SEND_PLAYTONE_RES_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_STK_SEND_PLAYTONE_RES_REQ_HEADER Param1;
} yPDef_MSG_STK_SEND_PLAYTONE_RES_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_STK_SEND_PLAYTONE_RES_RSP;

typedef CAPI2_SATK_SendSetupCallRes_Req_t *T_INTER_TASK_MSG_STK_SETUP_CALL_RES_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_STK_SETUP_CALL_RES_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_STK_SETUP_CALL_RES_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_STK_SETUP_CALL_RES_REQ_HEADER Param1;
} yPDef_MSG_STK_SETUP_CALL_RES_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_STK_SETUP_CALL_RES_RSP;

typedef CAPI2_PbkApi_SetFdnCheck_Req_t *T_INTER_TASK_MSG_PBK_SET_FDN_CHECK_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PBK_SET_FDN_CHECK_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PBK_SET_FDN_CHECK_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PBK_SET_FDN_CHECK_REQ_HEADER Param1;
} yPDef_MSG_PBK_SET_FDN_CHECK_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PBK_SET_FDN_CHECK_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PBK_GET_FDN_CHECK_REQ;

typedef Boolean *T_INTER_TASK_MSG_PBK_GET_FDN_CHECK_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PBK_GET_FDN_CHECK_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PBK_GET_FDN_CHECK_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PBK_GET_FDN_CHECK_RSP_HEADER Param1;
} yPDef_MSG_PBK_GET_FDN_CHECK_RSP;

typedef CAPI2_PMU_Battery_Register_Req_t *T_INTER_TASK_MSG_PMU_BATT_LEVEL_REGISTER_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PMU_BATT_LEVEL_REGISTER_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PMU_BATT_LEVEL_REGISTER_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PMU_BATT_LEVEL_REGISTER_REQ_HEADER Param1;
} yPDef_MSG_PMU_BATT_LEVEL_REGISTER_REQ;

typedef HAL_EM_BATTMGR_ErrorCode_en_t *T_INTER_TASK_MSG_PMU_BATT_LEVEL_REGISTER_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PMU_BATT_LEVEL_REGISTER_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PMU_BATT_LEVEL_REGISTER_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PMU_BATT_LEVEL_REGISTER_RSP_HEADER Param1;
} yPDef_MSG_PMU_BATT_LEVEL_REGISTER_RSP;

typedef HAL_EM_BatteryLevel_t *T_INTER_TASK_MSG_PMU_BATT_LEVEL_IND_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PMU_BATT_LEVEL_IND_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PMU_BATT_LEVEL_IND_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PMU_BATT_LEVEL_IND_HEADER Param1;
} yPDef_MSG_PMU_BATT_LEVEL_IND;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SEND_MEM_AVAL_IND_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SEND_MEM_AVAL_IND_RSP;

typedef CAPI2_SMS_ConfigureMEStorage_Req_t *T_INTER_TASK_MSG_SMS_CONFIGUREMESTORAGE_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_CONFIGUREMESTORAGE_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_CONFIGUREMESTORAGE_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_CONFIGUREMESTORAGE_REQ_HEADER Param1;
} yPDef_MSG_SMS_CONFIGUREMESTORAGE_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_CONFIGUREMESTORAGE_RSP;

typedef CAPI2_MsDbApi_SetElement_Req_t *T_INTER_TASK_MSG_MS_SET_ELEMENT_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_MS_SET_ELEMENT_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_MS_SET_ELEMENT_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_MS_SET_ELEMENT_REQ_HEADER Param1;
} yPDef_MSG_MS_SET_ELEMENT_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_MS_SET_ELEMENT_RSP;

typedef CAPI2_MsDbApi_GetElement_Req_t *T_INTER_TASK_MSG_MS_GET_ELEMENT_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_MS_GET_ELEMENT_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_MS_GET_ELEMENT_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_MS_GET_ELEMENT_REQ_HEADER Param1;
} yPDef_MSG_MS_GET_ELEMENT_REQ;

typedef CAPI2_MS_Element_t *T_INTER_TASK_MSG_MS_GET_ELEMENT_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_MS_GET_ELEMENT_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_MS_GET_ELEMENT_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_MS_GET_ELEMENT_RSP_HEADER Param1;
} yPDef_MSG_MS_GET_ELEMENT_RSP;

typedef CAPI2_USimApi_IsApplicationSupported_Req_t *T_INTER_TASK_MSG_USIM_IS_APP_SUPPORTED_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_USIM_IS_APP_SUPPORTED_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_USIM_IS_APP_SUPPORTED_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_USIM_IS_APP_SUPPORTED_REQ_HEADER Param1;
} yPDef_MSG_USIM_IS_APP_SUPPORTED_REQ;

typedef Boolean *T_INTER_TASK_MSG_USIM_IS_APP_SUPPORTED_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_USIM_IS_APP_SUPPORTED_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_USIM_IS_APP_SUPPORTED_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_USIM_IS_APP_SUPPORTED_RSP_HEADER Param1;
} yPDef_MSG_USIM_IS_APP_SUPPORTED_RSP;

typedef CAPI2_USimApi_IsAllowedAPN_Req_t *T_INTER_TASK_MSG_USIM_IS_APN_ALLOWED_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_USIM_IS_APN_ALLOWED_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_USIM_IS_APN_ALLOWED_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_USIM_IS_APN_ALLOWED_REQ_HEADER Param1;
} yPDef_MSG_USIM_IS_APN_ALLOWED_REQ;

typedef Boolean *T_INTER_TASK_MSG_USIM_IS_APN_ALLOWED_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_USIM_IS_APN_ALLOWED_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_USIM_IS_APN_ALLOWED_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_USIM_IS_APN_ALLOWED_RSP_HEADER Param1;
} yPDef_MSG_USIM_IS_APN_ALLOWED_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_USIM_GET_NUM_APN_REQ;

typedef UInt8 *T_INTER_TASK_MSG_USIM_GET_NUM_APN_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_USIM_GET_NUM_APN_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_USIM_GET_NUM_APN_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_USIM_GET_NUM_APN_RSP_HEADER Param1;
} yPDef_MSG_USIM_GET_NUM_APN_RSP;

typedef CAPI2_USimApi_GetAPNEntry_Req_t *T_INTER_TASK_MSG_USIM_GET_APN_ENTRY_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_USIM_GET_APN_ENTRY_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_USIM_GET_APN_ENTRY_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_USIM_GET_APN_ENTRY_REQ_HEADER Param1;
} yPDef_MSG_USIM_GET_APN_ENTRY_REQ;

typedef Result_t *T_INTER_TASK_MSG_USIM_GET_APN_ENTRY_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_USIM_GET_APN_ENTRY_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_USIM_GET_APN_ENTRY_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_USIM_GET_APN_ENTRY_RSP_HEADER Param1;
} yPDef_MSG_USIM_GET_APN_ENTRY_RSP;

typedef CAPI2_USimApi_IsEstServActivated_Req_t *T_INTER_TASK_MSG_USIM_IS_EST_SERV_ACTIVATED_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_USIM_IS_EST_SERV_ACTIVATED_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_USIM_IS_EST_SERV_ACTIVATED_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_USIM_IS_EST_SERV_ACTIVATED_REQ_HEADER Param1;
} yPDef_MSG_USIM_IS_EST_SERV_ACTIVATED_REQ;

typedef Boolean *T_INTER_TASK_MSG_USIM_IS_EST_SERV_ACTIVATED_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_USIM_IS_EST_SERV_ACTIVATED_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_USIM_IS_EST_SERV_ACTIVATED_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_USIM_IS_EST_SERV_ACTIVATED_RSP_HEADER Param1;
} yPDef_MSG_USIM_IS_EST_SERV_ACTIVATED_RSP;

typedef CAPI2_USimApi_SendSetEstServReq_Req_t *T_INTER_TASK_MSG_SIM_SET_EST_SERV_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_SET_EST_SERV_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_SET_EST_SERV_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_SET_EST_SERV_REQ_HEADER Param1;
} yPDef_MSG_SIM_SET_EST_SERV_REQ;

typedef CAPI2_USimApi_SendWriteAPNReq_Req_t *T_INTER_TASK_MSG_SIM_UPDATE_ONE_APN_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_UPDATE_ONE_APN_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_UPDATE_ONE_APN_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_UPDATE_ONE_APN_REQ_HEADER Param1;
} yPDef_MSG_SIM_UPDATE_ONE_APN_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIM_DELETE_ALL_APN_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_USIM_GET_RAT_MODE_REQ;

typedef USIM_RAT_MODE_t *T_INTER_TASK_MSG_USIM_GET_RAT_MODE_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_USIM_GET_RAT_MODE_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_USIM_GET_RAT_MODE_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_USIM_GET_RAT_MODE_RSP_HEADER Param1;
} yPDef_MSG_USIM_GET_RAT_MODE_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_MS_GET_GPRS_STATE_REQ;

typedef MSRegState_t *T_INTER_TASK_MSG_MS_GET_GPRS_STATE_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_MS_GET_GPRS_STATE_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_MS_GET_GPRS_STATE_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_MS_GET_GPRS_STATE_RSP_HEADER Param1;
} yPDef_MSG_MS_GET_GPRS_STATE_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_MS_GET_GSM_STATE_REQ;

typedef MSRegState_t *T_INTER_TASK_MSG_MS_GET_GSM_STATE_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_MS_GET_GSM_STATE_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_MS_GET_GSM_STATE_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_MS_GET_GSM_STATE_RSP_HEADER Param1;
} yPDef_MSG_MS_GET_GSM_STATE_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_MS_GET_CELL_INFO_REQ;

typedef CellInfo_t *T_INTER_TASK_MSG_MS_GET_CELL_INFO_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_MS_GET_CELL_INFO_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_MS_GET_CELL_INFO_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_MS_GET_CELL_INFO_RSP_HEADER Param1;
} yPDef_MSG_MS_GET_CELL_INFO_RSP;

typedef CAPI2_MS_SetMEPowerClass_Req_t *T_INTER_TASK_MSG_MS_SETMEPOWER_CLASS_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_MS_SETMEPOWER_CLASS_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_MS_SETMEPOWER_CLASS_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_MS_SETMEPOWER_CLASS_REQ_HEADER Param1;
} yPDef_MSG_MS_SETMEPOWER_CLASS_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_MS_SETMEPOWER_CLASS_RSP;

typedef CAPI2_USimApi_GetServiceStatus_Req_t *T_INTER_TASK_MSG_USIM_GET_SERVICE_STATUS_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_USIM_GET_SERVICE_STATUS_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_USIM_GET_SERVICE_STATUS_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_USIM_GET_SERVICE_STATUS_REQ_HEADER Param1;
} yPDef_MSG_USIM_GET_SERVICE_STATUS_REQ;

typedef SIMServiceStatus_t *T_INTER_TASK_MSG_USIM_GET_SERVICE_STATUS_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_USIM_GET_SERVICE_STATUS_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_USIM_GET_SERVICE_STATUS_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_USIM_GET_SERVICE_STATUS_RSP_HEADER Param1;
} yPDef_MSG_USIM_GET_SERVICE_STATUS_RSP;

typedef CAPI2_SimApi_IsAllowedAPN_Req_t *T_INTER_TASK_MSG_SIM_IS_ALLOWED_APN_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_IS_ALLOWED_APN_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_IS_ALLOWED_APN_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_IS_ALLOWED_APN_REQ_HEADER Param1;
} yPDef_MSG_SIM_IS_ALLOWED_APN_REQ;

typedef Boolean *T_INTER_TASK_MSG_SIM_IS_ALLOWED_APN_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_IS_ALLOWED_APN_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_IS_ALLOWED_APN_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_IS_ALLOWED_APN_RSP_HEADER Param1;
} yPDef_MSG_SIM_IS_ALLOWED_APN_RSP;

typedef CAPI2_SmsApi_GetSmsMaxCapacity_Req_t *T_INTER_TASK_MSG_SMS_GETSMSMAXCAPACITY_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_GETSMSMAXCAPACITY_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_GETSMSMAXCAPACITY_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_GETSMSMAXCAPACITY_REQ_HEADER Param1;
} yPDef_MSG_SMS_GETSMSMAXCAPACITY_REQ;

typedef UInt16 *T_INTER_TASK_MSG_SMS_GETSMSMAXCAPACITY_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_GETSMSMAXCAPACITY_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_GETSMSMAXCAPACITY_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_GETSMSMAXCAPACITY_RSP_HEADER Param1;
} yPDef_MSG_SMS_GETSMSMAXCAPACITY_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_RETRIEVEMAXCBCHNLLENGTH_REQ;

typedef Result_t *T_INTER_TASK_MSG_SMS_RETRIEVEMAXCBCHNLLENGTH_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_RETRIEVEMAXCBCHNLLENGTH_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_RETRIEVEMAXCBCHNLLENGTH_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_RETRIEVEMAXCBCHNLLENGTH_RSP_HEADER Param1;
} yPDef_MSG_SMS_RETRIEVEMAXCBCHNLLENGTH_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIM_IS_BDN_RESTRICTED_REQ;

typedef Boolean *T_INTER_TASK_MSG_SIM_IS_BDN_RESTRICTED_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_IS_BDN_RESTRICTED_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_IS_BDN_RESTRICTED_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_IS_BDN_RESTRICTED_RSP_HEADER Param1;
} yPDef_MSG_SIM_IS_BDN_RESTRICTED_RSP;

typedef CAPI2_SimApi_SendPreferredPlmnUpdateInd_Req_t *T_INTER_TASK_MSG_SIM_SEND_PLMN_UPDATE_IND_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_SEND_PLMN_UPDATE_IND_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_SEND_PLMN_UPDATE_IND_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_SEND_PLMN_UPDATE_IND_REQ_HEADER Param1;
} yPDef_MSG_SIM_SEND_PLMN_UPDATE_IND_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIM_SEND_PLMN_UPDATE_IND_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMIO_DEACTIVATE_CARD_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMIO_DEACTIVATE_CARD_RSP;

typedef CAPI2_SimApi_SendSetBdnReq_Req_t *T_INTER_TASK_MSG_SIM_SET_BDN_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_SET_BDN_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_SET_BDN_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_SET_BDN_REQ_HEADER Param1;
} yPDef_MSG_SIM_SET_BDN_REQ;

typedef CAPI2_SimApi_PowerOnOffCard_Req_t *T_INTER_TASK_MSG_SIM_POWER_ON_OFF_CARD_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_POWER_ON_OFF_CARD_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_POWER_ON_OFF_CARD_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_POWER_ON_OFF_CARD_REQ_HEADER Param1;
} yPDef_MSG_SIM_POWER_ON_OFF_CARD_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIM_GET_RAW_ATR_REQ;

typedef CAPI2_SimApi_Set_Protocol_Req_t *T_INTER_TASK_MSG_SIM_SET_PROTOCOL_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_SET_PROTOCOL_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_SET_PROTOCOL_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_SET_PROTOCOL_REQ_HEADER Param1;
} yPDef_MSG_SIM_SET_PROTOCOL_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIM_SET_PROTOCOL_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIM_GET_PROTOCOL_REQ;

typedef UInt8 *T_INTER_TASK_MSG_SIM_GET_PROTOCOL_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_GET_PROTOCOL_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_GET_PROTOCOL_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_GET_PROTOCOL_RSP_HEADER Param1;
} yPDef_MSG_SIM_GET_PROTOCOL_RSP;

typedef CAPI2_SimApi_SendGenericApduCmd_Req_t *T_INTER_TASK_MSG_SIM_SEND_GENERIC_APDU_CMD_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_SEND_GENERIC_APDU_CMD_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_SEND_GENERIC_APDU_CMD_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_SEND_GENERIC_APDU_CMD_REQ_HEADER Param1;
} yPDef_MSG_SIM_SEND_GENERIC_APDU_CMD_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIM_TERMINATE_XFER_APDU_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIM_TERMINATE_XFER_APDU_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIM_GET_SIM_INTERFACE_REQ;

typedef SIM_SIM_INTERFACE_t *T_INTER_TASK_MSG_SIM_GET_SIM_INTERFACE_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_GET_SIM_INTERFACE_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_GET_SIM_INTERFACE_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_GET_SIM_INTERFACE_RSP_HEADER Param1;
} yPDef_MSG_SIM_GET_SIM_INTERFACE_RSP;

typedef CAPI2_NetRegApi_SetPlmnSelectRat_Req_t *T_INTER_TASK_MSG_SET_PLMN_SELECT_RAT_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SET_PLMN_SELECT_RAT_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SET_PLMN_SELECT_RAT_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SET_PLMN_SELECT_RAT_REQ_HEADER Param1;
} yPDef_MSG_SET_PLMN_SELECT_RAT_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SET_PLMN_SELECT_RAT_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_IS_DEREGISTER_IN_PROGRESS_REQ;

typedef Boolean *T_INTER_TASK_MSG_IS_DEREGISTER_IN_PROGRESS_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_IS_DEREGISTER_IN_PROGRESS_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_IS_DEREGISTER_IN_PROGRESS_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_IS_DEREGISTER_IN_PROGRESS_RSP_HEADER Param1;
} yPDef_MSG_IS_DEREGISTER_IN_PROGRESS_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_IS_REGISTER_IN_PROGRESS_REQ;

typedef Boolean *T_INTER_TASK_MSG_IS_REGISTER_IN_PROGRESS_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_IS_REGISTER_IN_PROGRESS_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_IS_REGISTER_IN_PROGRESS_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_IS_REGISTER_IN_PROGRESS_RSP_HEADER Param1;
} yPDef_MSG_IS_REGISTER_IN_PROGRESS_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_READ_USIM_PBK_HDK_REQ;

typedef CAPI2_PbkApi_SendUsimHdkUpdateReq_Req_t *T_INTER_TASK_MSG_WRITE_USIM_PBK_HDK_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_WRITE_USIM_PBK_HDK_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_WRITE_USIM_PBK_HDK_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_WRITE_USIM_PBK_HDK_REQ_HEADER Param1;
} yPDef_MSG_WRITE_USIM_PBK_HDK_REQ;

typedef CAPI2_PbkApi_SendUsimAasReadReq_Req_t *T_INTER_TASK_MSG_READ_USIM_PBK_ALPHA_AAS_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_READ_USIM_PBK_ALPHA_AAS_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_READ_USIM_PBK_ALPHA_AAS_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_READ_USIM_PBK_ALPHA_AAS_REQ_HEADER Param1;
} yPDef_MSG_READ_USIM_PBK_ALPHA_AAS_REQ;

typedef CAPI2_PbkApi_SendUsimAasUpdateReq_Req_t *T_INTER_TASK_MSG_WRITE_USIM_PBK_ALPHA_AAS_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_WRITE_USIM_PBK_ALPHA_AAS_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_WRITE_USIM_PBK_ALPHA_AAS_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_WRITE_USIM_PBK_ALPHA_AAS_REQ_HEADER Param1;
} yPDef_MSG_WRITE_USIM_PBK_ALPHA_AAS_REQ;

typedef CAPI2_PbkApi_SendUsimGasReadReq_Req_t *T_INTER_TASK_MSG_READ_USIM_PBK_ALPHA_GAS_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_READ_USIM_PBK_ALPHA_GAS_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_READ_USIM_PBK_ALPHA_GAS_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_READ_USIM_PBK_ALPHA_GAS_REQ_HEADER Param1;
} yPDef_MSG_READ_USIM_PBK_ALPHA_GAS_REQ;

typedef CAPI2_PbkApi_SendUsimGasUpdateReq_Req_t *T_INTER_TASK_MSG_WRITE_USIM_PBK_ALPHA_GAS_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_WRITE_USIM_PBK_ALPHA_GAS_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_WRITE_USIM_PBK_ALPHA_GAS_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_WRITE_USIM_PBK_ALPHA_GAS_REQ_HEADER Param1;
} yPDef_MSG_WRITE_USIM_PBK_ALPHA_GAS_REQ;

typedef CAPI2_PbkApi_SendUsimAasInfoReq_Req_t *T_INTER_TASK_MSG_GET_USIM_PBK_ALPHA_INFO_AAS_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_GET_USIM_PBK_ALPHA_INFO_AAS_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_GET_USIM_PBK_ALPHA_INFO_AAS_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_GET_USIM_PBK_ALPHA_INFO_AAS_REQ_HEADER Param1;
} yPDef_MSG_GET_USIM_PBK_ALPHA_INFO_AAS_REQ;

typedef CAPI2_PbkApi_SendUsimGasInfoReq_Req_t *T_INTER_TASK_MSG_GET_USIM_PBK_ALPHA_INFO_GAS_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_GET_USIM_PBK_ALPHA_INFO_GAS_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_GET_USIM_PBK_ALPHA_INFO_GAS_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_GET_USIM_PBK_ALPHA_INFO_GAS_REQ_HEADER Param1;
} yPDef_MSG_GET_USIM_PBK_ALPHA_INFO_GAS_REQ;

typedef CAPI2_DiagApi_MeasurmentReportReq_Req_t *T_INTER_TASK_MSG_DIAG_MEASURE_REPORT_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_DIAG_MEASURE_REPORT_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_DIAG_MEASURE_REPORT_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_DIAG_MEASURE_REPORT_REQ_HEADER Param1;
} yPDef_MSG_DIAG_MEASURE_REPORT_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_DIAG_MEASURE_REPORT_RSP;

typedef CAPI2_PMU_BattChargingNotification_Req_t *T_INTER_TASK_MSG_PMU_BATT_CHARGING_NOTIFICATION_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PMU_BATT_CHARGING_NOTIFICATION_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PMU_BATT_CHARGING_NOTIFICATION_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PMU_BATT_CHARGING_NOTIFICATION_REQ_HEADER Param1;
} yPDef_MSG_PMU_BATT_CHARGING_NOTIFICATION_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PMU_BATT_CHARGING_NOTIFICATION_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_MS_INITCALLCFG_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_MS_INITCALLCFG_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_MS_INITFAXCFG_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_MS_INITFAXCFG_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_MS_INITVIDEOCALLCFG_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_MS_INITVIDEOCALLCFG_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_MS_INITCALLCFGAMPF_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_MS_INITCALLCFGAMPF_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SYSPARM_GET_ACTUAL_LOW_VOLT_REQ;

typedef UInt16 *T_INTER_TASK_MSG_SYSPARM_GET_ACTUAL_LOW_VOLT_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SYSPARM_GET_ACTUAL_LOW_VOLT_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SYSPARM_GET_ACTUAL_LOW_VOLT_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SYSPARM_GET_ACTUAL_LOW_VOLT_RSP_HEADER Param1;
} yPDef_MSG_SYSPARM_GET_ACTUAL_LOW_VOLT_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SYSPARM_GET_ACTUAL_4P2_VOLT_REQ;

typedef UInt16 *T_INTER_TASK_MSG_SYSPARM_GET_ACTUAL_4P2_VOLT_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SYSPARM_GET_ACTUAL_4P2_VOLT_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SYSPARM_GET_ACTUAL_4P2_VOLT_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SYSPARM_GET_ACTUAL_4P2_VOLT_RSP_HEADER Param1;
} yPDef_MSG_SYSPARM_GET_ACTUAL_4P2_VOLT_RSP;

typedef CAPI2_SmsApi_SendSMSCommandTxtReq_Req_t *T_INTER_TASK_MSG_SMS_SEND_COMMAND_TXT_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_SEND_COMMAND_TXT_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_SEND_COMMAND_TXT_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_SEND_COMMAND_TXT_REQ_HEADER Param1;
} yPDef_MSG_SMS_SEND_COMMAND_TXT_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SEND_COMMAND_TXT_RSP;

typedef CAPI2_SmsApi_SendSMSCommandPduReq_Req_t *T_INTER_TASK_MSG_SMS_SEND_COMMAND_PDU_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_SEND_COMMAND_PDU_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_SEND_COMMAND_PDU_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_SEND_COMMAND_PDU_REQ_HEADER Param1;
} yPDef_MSG_SMS_SEND_COMMAND_PDU_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SEND_COMMAND_PDU_RSP;

typedef CAPI2_SmsApi_SendPDUAckToNetwork_Req_t *T_INTER_TASK_MSG_SMS_SEND_ACKTONETWORK_PDU_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_SEND_ACKTONETWORK_PDU_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_SEND_ACKTONETWORK_PDU_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_SEND_ACKTONETWORK_PDU_REQ_HEADER Param1;
} yPDef_MSG_SMS_SEND_ACKTONETWORK_PDU_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SEND_ACKTONETWORK_PDU_RSP;

typedef CAPI2_SmsApi_StartCellBroadcastWithChnlReq_Req_t *T_INTER_TASK_MSG_SMS_CB_START_STOP_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_CB_START_STOP_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_CB_START_STOP_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_CB_START_STOP_REQ_HEADER Param1;
} yPDef_MSG_SMS_CB_START_STOP_REQ;

typedef CAPI2_SmsApi_SetMoSmsTpMr_Req_t *T_INTER_TASK_MSG_SMS_SET_TPMR_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_SET_TPMR_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_SET_TPMR_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_SET_TPMR_REQ_HEADER Param1;
} yPDef_MSG_SMS_SET_TPMR_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SET_TPMR_RSP;

typedef CAPI2_SIMLOCKApi_SetStatus_Req_t *T_INTER_TASK_MSG_SIMLOCK_SET_STATUS_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIMLOCK_SET_STATUS_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIMLOCK_SET_STATUS_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIMLOCK_SET_STATUS_REQ_HEADER Param1;
} yPDef_MSG_SIMLOCK_SET_STATUS_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMLOCK_SET_STATUS_RSP;

typedef CAPI2_DIAG_ApiCellLockReq_Req_t *T_INTER_TASK_MSG_DIAG_CELLLOCK_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_DIAG_CELLLOCK_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_DIAG_CELLLOCK_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_DIAG_CELLLOCK_REQ_HEADER Param1;
} yPDef_MSG_DIAG_CELLLOCK_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_DIAG_CELLLOCK_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_DIAG_CELLLOCK_STATUS_REQ;

typedef Boolean *T_INTER_TASK_MSG_DIAG_CELLLOCK_STATUS_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_DIAG_CELLLOCK_STATUS_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_DIAG_CELLLOCK_STATUS_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_DIAG_CELLLOCK_STATUS_RSP_HEADER Param1;
} yPDef_MSG_DIAG_CELLLOCK_STATUS_RSP;

typedef CAPI2_MS_SetRuaReadyTimer_Req_t *T_INTER_TASK_MSG_MS_SET_RUA_READY_TIMER_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_MS_SET_RUA_READY_TIMER_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_MS_SET_RUA_READY_TIMER_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_MS_SET_RUA_READY_TIMER_REQ_HEADER Param1;
} yPDef_MSG_MS_SET_RUA_READY_TIMER_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_MS_SET_RUA_READY_TIMER_RSP;

typedef CAPI2_LCS_RegisterRrlpDataHandler_Req_t *T_INTER_TASK_MSG_LCS_REG_RRLP_HDL_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_LCS_REG_RRLP_HDL_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_LCS_REG_RRLP_HDL_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_LCS_REG_RRLP_HDL_REQ_HEADER Param1;
} yPDef_MSG_LCS_REG_RRLP_HDL_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_LCS_REG_RRLP_HDL_RSP;

typedef CAPI2_LCS_RegisterRrcDataHandler_Req_t *T_INTER_TASK_MSG_LCS_REG_RRC_HDL_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_LCS_REG_RRC_HDL_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_LCS_REG_RRC_HDL_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_LCS_REG_RRC_HDL_REQ_HEADER Param1;
} yPDef_MSG_LCS_REG_RRC_HDL_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_LCS_REG_RRC_HDL_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_IS_THERE_EMERGENCY_CALL_REQ;

typedef Boolean *T_INTER_TASK_MSG_CC_IS_THERE_EMERGENCY_CALL_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_IS_THERE_EMERGENCY_CALL_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_IS_THERE_EMERGENCY_CALL_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_IS_THERE_EMERGENCY_CALL_RSP_HEADER Param1;
} yPDef_MSG_CC_IS_THERE_EMERGENCY_CALL_RSP;

typedef CAPI2_SYS_EnableCellInfoMsg_Req_t *T_INTER_TASK_MSG_SYS_ENABLE_CELL_INFO_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SYS_ENABLE_CELL_INFO_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SYS_ENABLE_CELL_INFO_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SYS_ENABLE_CELL_INFO_REQ_HEADER Param1;
} yPDef_MSG_SYS_ENABLE_CELL_INFO_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SYS_ENABLE_CELL_INFO_RSP;

typedef CAPI2_LCS_L1_bb_isLocked_Req_t *T_INTER_TASK_MSG_L1_BB_ISLOCKED_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_L1_BB_ISLOCKED_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_L1_BB_ISLOCKED_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_L1_BB_ISLOCKED_REQ_HEADER Param1;
} yPDef_MSG_L1_BB_ISLOCKED_REQ;

typedef Boolean *T_INTER_TASK_MSG_L1_BB_ISLOCKED_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_L1_BB_ISLOCKED_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_L1_BB_ISLOCKED_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_L1_BB_ISLOCKED_RSP_HEADER Param1;
} yPDef_MSG_L1_BB_ISLOCKED_RSP;

typedef CAPI2_DIALSTR_ParseGetCallType_Req_t *T_INTER_TASK_MSG_UTIL_DIAL_STR_PARSE_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_UTIL_DIAL_STR_PARSE_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_UTIL_DIAL_STR_PARSE_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_UTIL_DIAL_STR_PARSE_REQ_HEADER Param1;
} yPDef_MSG_UTIL_DIAL_STR_PARSE_REQ;

typedef CallType_t *T_INTER_TASK_MSG_UTIL_DIAL_STR_PARSE_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_UTIL_DIAL_STR_PARSE_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_UTIL_DIAL_STR_PARSE_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_UTIL_DIAL_STR_PARSE_RSP_HEADER Param1;
} yPDef_MSG_UTIL_DIAL_STR_PARSE_RSP;

typedef CAPI2_LCS_FttCalcDeltaTime_Req_t *T_INTER_TASK_MSG_LCS_FTT_DELTA_TIME_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_LCS_FTT_DELTA_TIME_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_LCS_FTT_DELTA_TIME_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_LCS_FTT_DELTA_TIME_REQ_HEADER Param1;
} yPDef_MSG_LCS_FTT_DELTA_TIME_REQ;

typedef UInt32 *T_INTER_TASK_MSG_LCS_FTT_DELTA_TIME_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_LCS_FTT_DELTA_TIME_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_LCS_FTT_DELTA_TIME_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_LCS_FTT_DELTA_TIME_RSP_HEADER Param1;
} yPDef_MSG_LCS_FTT_DELTA_TIME_RSP;

typedef CAPI2_NetRegApi_ForcedReadyStateReq_Req_t *T_INTER_TASK_MSG_MS_FORCEDREADYSTATE_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_MS_FORCEDREADYSTATE_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_MS_FORCEDREADYSTATE_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_MS_FORCEDREADYSTATE_REQ_HEADER Param1;
} yPDef_MSG_MS_FORCEDREADYSTATE_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_MS_FORCEDREADYSTATE_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SS_RESETSSALSFLAG_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SS_RESETSSALSFLAG_RSP;

typedef CAPI2_SIMLOCK_GetStatus_Req_t *T_INTER_TASK_MSG_SIMLOCK_GET_STATUS_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIMLOCK_GET_STATUS_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIMLOCK_GET_STATUS_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIMLOCK_GET_STATUS_REQ_HEADER Param1;
} yPDef_MSG_SIMLOCK_GET_STATUS_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMLOCK_GET_STATUS_RSP;

typedef CAPI2_DIALSTR_IsValidString_Req_t *T_INTER_TASK_MSG_DIALSTR_IS_VALID_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_DIALSTR_IS_VALID_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_DIALSTR_IS_VALID_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_DIALSTR_IS_VALID_REQ_HEADER Param1;
} yPDef_MSG_DIALSTR_IS_VALID_REQ;

typedef Boolean *T_INTER_TASK_MSG_DIALSTR_IS_VALID_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_DIALSTR_IS_VALID_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_DIALSTR_IS_VALID_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_DIALSTR_IS_VALID_RSP_HEADER Param1;
} yPDef_MSG_DIALSTR_IS_VALID_RSP;

typedef CAPI2_UTIL_Cause2NetworkCause_Req_t *T_INTER_TASK_MSG_UTIL_CONVERT_NTWK_CAUSE_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_UTIL_CONVERT_NTWK_CAUSE_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_UTIL_CONVERT_NTWK_CAUSE_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_UTIL_CONVERT_NTWK_CAUSE_REQ_HEADER Param1;
} yPDef_MSG_UTIL_CONVERT_NTWK_CAUSE_REQ;

typedef NetworkCause_t *T_INTER_TASK_MSG_UTIL_CONVERT_NTWK_CAUSE_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_UTIL_CONVERT_NTWK_CAUSE_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_UTIL_CONVERT_NTWK_CAUSE_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_UTIL_CONVERT_NTWK_CAUSE_RSP_HEADER Param1;
} yPDef_MSG_UTIL_CONVERT_NTWK_CAUSE_RSP;

typedef CAPI2_UTIL_ErrCodeToNetCause_Req_t *T_INTER_TASK_MSG_UTIL_CONVERT_ERRCODE_NTWK_CAUSE_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_UTIL_CONVERT_ERRCODE_NTWK_CAUSE_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_UTIL_CONVERT_ERRCODE_NTWK_CAUSE_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_UTIL_CONVERT_ERRCODE_NTWK_CAUSE_REQ_HEADER Param1;
} yPDef_MSG_UTIL_CONVERT_ERRCODE_NTWK_CAUSE_REQ;

typedef NetworkCause_t *T_INTER_TASK_MSG_UTIL_CONVERT_ERRCODE_NTWK_CAUSE_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_UTIL_CONVERT_ERRCODE_NTWK_CAUSE_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_UTIL_CONVERT_ERRCODE_NTWK_CAUSE_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_UTIL_CONVERT_ERRCODE_NTWK_CAUSE_RSP_HEADER Param1;
} yPDef_MSG_UTIL_CONVERT_ERRCODE_NTWK_CAUSE_RSP;

typedef CAPI2_IsGprsDialStr_Req_t *T_INTER_TASK_MSG_ISGPRS_DIAL_STR_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_ISGPRS_DIAL_STR_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_ISGPRS_DIAL_STR_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_ISGPRS_DIAL_STR_REQ_HEADER Param1;
} yPDef_MSG_ISGPRS_DIAL_STR_REQ;

typedef Boolean *T_INTER_TASK_MSG_ISGPRS_DIAL_STR_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_ISGPRS_DIAL_STR_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_ISGPRS_DIAL_STR_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_ISGPRS_DIAL_STR_RSP_HEADER Param1;
} yPDef_MSG_ISGPRS_DIAL_STR_RSP;

typedef CAPI2_UTIL_GetNumOffsetInSsStr_Req_t *T_INTER_TASK_MSG_GET_NUM_SS_STR_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_GET_NUM_SS_STR_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_GET_NUM_SS_STR_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_GET_NUM_SS_STR_REQ_HEADER Param1;
} yPDef_MSG_GET_NUM_SS_STR_REQ;

typedef UInt8 *T_INTER_TASK_MSG_GET_NUM_SS_STR_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_GET_NUM_SS_STR_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_GET_NUM_SS_STR_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_GET_NUM_SS_STR_RSP_HEADER Param1;
} yPDef_MSG_GET_NUM_SS_STR_RSP;

typedef CAPI2_IsPppLoopbackDialStr_Req_t *T_INTER_TASK_MSG_DIALSTR_IS_PPPLOOPBACK_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_DIALSTR_IS_PPPLOOPBACK_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_DIALSTR_IS_PPPLOOPBACK_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_DIALSTR_IS_PPPLOOPBACK_REQ_HEADER Param1;
} yPDef_MSG_DIALSTR_IS_PPPLOOPBACK_REQ;

typedef Boolean *T_INTER_TASK_MSG_DIALSTR_IS_PPPLOOPBACK_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_DIALSTR_IS_PPPLOOPBACK_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_DIALSTR_IS_PPPLOOPBACK_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_DIALSTR_IS_PPPLOOPBACK_RSP_HEADER Param1;
} yPDef_MSG_DIALSTR_IS_PPPLOOPBACK_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SYS_GETRIPPROCVERSION_REQ;

typedef UInt8 *T_INTER_TASK_MSG_SYS_GETRIPPROCVERSION_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SYS_GETRIPPROCVERSION_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SYS_GETRIPPROCVERSION_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SYS_GETRIPPROCVERSION_RSP_HEADER Param1;
} yPDef_MSG_SYS_GETRIPPROCVERSION_RSP;

typedef CAPI2_SYSPARM_SetHSDPAPHYCategory_Req_t *T_INTER_TASK_MSG_SYSPARM_SET_HSDPA_PHY_CAT_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SYSPARM_SET_HSDPA_PHY_CAT_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SYSPARM_SET_HSDPA_PHY_CAT_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SYSPARM_SET_HSDPA_PHY_CAT_REQ_HEADER Param1;
} yPDef_MSG_SYSPARM_SET_HSDPA_PHY_CAT_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SYSPARM_SET_HSDPA_PHY_CAT_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SYS_GET_HSDPA_CATEGORY_REQ;

typedef UInt8 *T_INTER_TASK_MSG_SYS_GET_HSDPA_CATEGORY_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SYS_GET_HSDPA_CATEGORY_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SYS_GET_HSDPA_CATEGORY_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SYS_GET_HSDPA_CATEGORY_RSP_HEADER Param1;
} yPDef_MSG_SYS_GET_HSDPA_CATEGORY_RSP;

typedef CAPI2_SmsApi_ConvertSmsMSMsgType_Req_t *T_INTER_TASK_MSG_SMS_CONVERT_MSGTYPE_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_CONVERT_MSGTYPE_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_CONVERT_MSGTYPE_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_CONVERT_MSGTYPE_REQ_HEADER Param1;
} yPDef_MSG_SMS_CONVERT_MSGTYPE_REQ;

typedef UInt8 *T_INTER_TASK_MSG_SMS_CONVERT_MSGTYPE_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_CONVERT_MSGTYPE_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_CONVERT_MSGTYPE_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_CONVERT_MSGTYPE_RSP_HEADER Param1;
} yPDef_MSG_SMS_CONVERT_MSGTYPE_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_MS_GETPREFNETSTATUS_REQ;

typedef GANStatus_t *T_INTER_TASK_MSG_MS_GETPREFNETSTATUS_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_MS_GETPREFNETSTATUS_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_MS_GETPREFNETSTATUS_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_MS_GETPREFNETSTATUS_RSP_HEADER Param1;
} yPDef_MSG_MS_GETPREFNETSTATUS_RSP;

typedef uchar_ptr_t *T_INTER_TASK_MSG_SYS_TEST_MSG_IND_1_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SYS_TEST_MSG_IND_1_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SYS_TEST_MSG_IND_1_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SYS_TEST_MSG_IND_1_HEADER Param1;
} yPDef_MSG_SYS_TEST_MSG_IND_1;

typedef uchar_ptr_t *T_INTER_TASK_MSG_SYS_TEST_MSG_IND_2_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SYS_TEST_MSG_IND_2_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SYS_TEST_MSG_IND_2_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SYS_TEST_MSG_IND_2_HEADER Param1;
} yPDef_MSG_SYS_TEST_MSG_IND_2;

typedef uchar_ptr_t *T_INTER_TASK_MSG_SYS_TEST_MSG_IND_3_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SYS_TEST_MSG_IND_3_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SYS_TEST_MSG_IND_3_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SYS_TEST_MSG_IND_3_HEADER Param1;
} yPDef_MSG_SYS_TEST_MSG_IND_3;

typedef uchar_ptr_t *T_INTER_TASK_MSG_SYS_TEST_MSG_IND_4_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SYS_TEST_MSG_IND_4_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SYS_TEST_MSG_IND_4_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SYS_TEST_MSG_IND_4_HEADER Param1;
} yPDef_MSG_SYS_TEST_MSG_IND_4;

typedef uchar_ptr_t *T_INTER_TASK_MSG_SYS_TEST_MSG_IND_5_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SYS_TEST_MSG_IND_5_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SYS_TEST_MSG_IND_5_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SYS_TEST_MSG_IND_5_HEADER Param1;
} yPDef_MSG_SYS_TEST_MSG_IND_5;

typedef uchar_ptr_t *T_INTER_TASK_MSG_SYS_TEST_MSG_IND_6_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SYS_TEST_MSG_IND_6_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SYS_TEST_MSG_IND_6_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SYS_TEST_MSG_IND_6_HEADER Param1;
} yPDef_MSG_SYS_TEST_MSG_IND_6;

typedef CAPI2_InterTaskMsgToCP_Req_t *T_INTER_TASK_MSG_INTERTASK_MSG_TO_CP_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_INTERTASK_MSG_TO_CP_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_INTERTASK_MSG_TO_CP_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_INTERTASK_MSG_TO_CP_REQ_HEADER Param1;
} yPDef_MSG_INTERTASK_MSG_TO_CP_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_INTERTASK_MSG_TO_CP_RSP;

typedef CAPI2_InterTaskMsgToAP_Req_t *T_INTER_TASK_MSG_INTERTASK_MSG_TO_AP_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_INTERTASK_MSG_TO_AP_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_INTERTASK_MSG_TO_AP_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_INTERTASK_MSG_TO_AP_REQ_HEADER Param1;
} yPDef_MSG_INTERTASK_MSG_TO_AP_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_INTERTASK_MSG_TO_AP_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_GETCURRENTCALLINDEX_REQ;

typedef UInt8 *T_INTER_TASK_MSG_CC_GETCURRENTCALLINDEX_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GETCURRENTCALLINDEX_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GETCURRENTCALLINDEX_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GETCURRENTCALLINDEX_RSP_HEADER Param1;
} yPDef_MSG_CC_GETCURRENTCALLINDEX_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_GETNEXTACTIVECALLINDEX_REQ;

typedef UInt8 *T_INTER_TASK_MSG_CC_GETNEXTACTIVECALLINDEX_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GETNEXTACTIVECALLINDEX_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GETNEXTACTIVECALLINDEX_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GETNEXTACTIVECALLINDEX_RSP_HEADER Param1;
} yPDef_MSG_CC_GETNEXTACTIVECALLINDEX_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_GETNEXTHELDCALLINDEX_REQ;

typedef UInt8 *T_INTER_TASK_MSG_CC_GETNEXTHELDCALLINDEX_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GETNEXTHELDCALLINDEX_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GETNEXTHELDCALLINDEX_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GETNEXTHELDCALLINDEX_RSP_HEADER Param1;
} yPDef_MSG_CC_GETNEXTHELDCALLINDEX_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_GETNEXTWAITCALLINDEX_REQ;

typedef UInt8 *T_INTER_TASK_MSG_CC_GETNEXTWAITCALLINDEX_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GETNEXTWAITCALLINDEX_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GETNEXTWAITCALLINDEX_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GETNEXTWAITCALLINDEX_RSP_HEADER Param1;
} yPDef_MSG_CC_GETNEXTWAITCALLINDEX_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_GETMPTYCALLINDEX_REQ;

typedef UInt8 *T_INTER_TASK_MSG_CC_GETMPTYCALLINDEX_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GETMPTYCALLINDEX_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GETMPTYCALLINDEX_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GETMPTYCALLINDEX_RSP_HEADER Param1;
} yPDef_MSG_CC_GETMPTYCALLINDEX_RSP;

typedef CAPI2_CcApi_GetCallState_Req_t *T_INTER_TASK_MSG_CC_GETCALLSTATE_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GETCALLSTATE_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GETCALLSTATE_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GETCALLSTATE_REQ_HEADER Param1;
} yPDef_MSG_CC_GETCALLSTATE_REQ;

typedef CCallState_t *T_INTER_TASK_MSG_CC_GETCALLSTATE_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GETCALLSTATE_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GETCALLSTATE_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GETCALLSTATE_RSP_HEADER Param1;
} yPDef_MSG_CC_GETCALLSTATE_RSP;

typedef CAPI2_CcApi_GetCallType_Req_t *T_INTER_TASK_MSG_CC_GETCALLTYPE_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GETCALLTYPE_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GETCALLTYPE_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GETCALLTYPE_REQ_HEADER Param1;
} yPDef_MSG_CC_GETCALLTYPE_REQ;

typedef CCallType_t *T_INTER_TASK_MSG_CC_GETCALLTYPE_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GETCALLTYPE_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GETCALLTYPE_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GETCALLTYPE_RSP_HEADER Param1;
} yPDef_MSG_CC_GETCALLTYPE_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_GETLASTCALLEXITCAUSE_REQ;

typedef Cause_t *T_INTER_TASK_MSG_CC_GETLASTCALLEXITCAUSE_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GETLASTCALLEXITCAUSE_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GETLASTCALLEXITCAUSE_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GETLASTCALLEXITCAUSE_RSP_HEADER Param1;
} yPDef_MSG_CC_GETLASTCALLEXITCAUSE_RSP;

typedef CAPI2_CcApi_GetCallNumber_Req_t *T_INTER_TASK_MSG_CC_GETCALLNUMBER_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GETCALLNUMBER_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GETCALLNUMBER_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GETCALLNUMBER_REQ_HEADER Param1;
} yPDef_MSG_CC_GETCALLNUMBER_REQ;

typedef Boolean *T_INTER_TASK_MSG_CC_GETCALLNUMBER_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GETCALLNUMBER_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GETCALLNUMBER_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GETCALLNUMBER_RSP_HEADER Param1;
} yPDef_MSG_CC_GETCALLNUMBER_RSP;

typedef CAPI2_CcApi_GetCallingInfo_Req_t *T_INTER_TASK_MSG_CC_GETCALLINGINFO_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GETCALLINGINFO_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GETCALLINGINFO_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GETCALLINGINFO_REQ_HEADER Param1;
} yPDef_MSG_CC_GETCALLINGINFO_REQ;

typedef Boolean *T_INTER_TASK_MSG_CC_GETCALLINGINFO_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GETCALLINGINFO_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GETCALLINGINFO_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GETCALLINGINFO_RSP_HEADER Param1;
} yPDef_MSG_CC_GETCALLINGINFO_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_GETALLCALLSTATES_REQ;

typedef Result_t *T_INTER_TASK_MSG_CC_GETALLCALLSTATES_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GETALLCALLSTATES_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GETALLCALLSTATES_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GETALLCALLSTATES_RSP_HEADER Param1;
} yPDef_MSG_CC_GETALLCALLSTATES_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_GETALLCALLINDEX_REQ;

typedef Result_t *T_INTER_TASK_MSG_CC_GETALLCALLINDEX_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GETALLCALLINDEX_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GETALLCALLINDEX_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GETALLCALLINDEX_RSP_HEADER Param1;
} yPDef_MSG_CC_GETALLCALLINDEX_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_GETALLHELDCALLINDEX_REQ;

typedef Result_t *T_INTER_TASK_MSG_CC_GETALLHELDCALLINDEX_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GETALLHELDCALLINDEX_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GETALLHELDCALLINDEX_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GETALLHELDCALLINDEX_RSP_HEADER Param1;
} yPDef_MSG_CC_GETALLHELDCALLINDEX_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_GETALLACTIVECALLINDEX_REQ;

typedef Result_t *T_INTER_TASK_MSG_CC_GETALLACTIVECALLINDEX_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GETALLACTIVECALLINDEX_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GETALLACTIVECALLINDEX_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GETALLACTIVECALLINDEX_RSP_HEADER Param1;
} yPDef_MSG_CC_GETALLACTIVECALLINDEX_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_GETALLMPTYCALLINDEX_REQ;

typedef Result_t *T_INTER_TASK_MSG_CC_GETALLMPTYCALLINDEX_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GETALLMPTYCALLINDEX_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GETALLMPTYCALLINDEX_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GETALLMPTYCALLINDEX_RSP_HEADER Param1;
} yPDef_MSG_CC_GETALLMPTYCALLINDEX_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_GETNUMOFMPTYCALLS_REQ;

typedef UInt8 *T_INTER_TASK_MSG_CC_GETNUMOFMPTYCALLS_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GETNUMOFMPTYCALLS_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GETNUMOFMPTYCALLS_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GETNUMOFMPTYCALLS_RSP_HEADER Param1;
} yPDef_MSG_CC_GETNUMOFMPTYCALLS_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_GETNUMOFACTIVECALLS_REQ;

typedef UInt8 *T_INTER_TASK_MSG_CC_GETNUMOFACTIVECALLS_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GETNUMOFACTIVECALLS_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GETNUMOFACTIVECALLS_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GETNUMOFACTIVECALLS_RSP_HEADER Param1;
} yPDef_MSG_CC_GETNUMOFACTIVECALLS_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_GETNUMOFHELDCALLS_REQ;

typedef UInt8 *T_INTER_TASK_MSG_CC_GETNUMOFHELDCALLS_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GETNUMOFHELDCALLS_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GETNUMOFHELDCALLS_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GETNUMOFHELDCALLS_RSP_HEADER Param1;
} yPDef_MSG_CC_GETNUMOFHELDCALLS_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_ISTHEREWAITINGCALL_REQ;

typedef Boolean *T_INTER_TASK_MSG_CC_ISTHEREWAITINGCALL_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_ISTHEREWAITINGCALL_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_ISTHEREWAITINGCALL_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_ISTHEREWAITINGCALL_RSP_HEADER Param1;
} yPDef_MSG_CC_ISTHEREWAITINGCALL_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_ISTHEREALERTINGCALL_REQ;

typedef Boolean *T_INTER_TASK_MSG_CC_ISTHEREALERTINGCALL_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_ISTHEREALERTINGCALL_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_ISTHEREALERTINGCALL_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_ISTHEREALERTINGCALL_RSP_HEADER Param1;
} yPDef_MSG_CC_ISTHEREALERTINGCALL_RSP;

typedef CAPI2_CcApi_GetConnectedLineID_Req_t *T_INTER_TASK_MSG_CC_GETCONNECTEDLINEID_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GETCONNECTEDLINEID_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GETCONNECTEDLINEID_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GETCONNECTEDLINEID_REQ_HEADER Param1;
} yPDef_MSG_CC_GETCONNECTEDLINEID_REQ;

typedef UInt8 *T_INTER_TASK_MSG_CC_GETCONNECTEDLINEID_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GETCONNECTEDLINEID_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GETCONNECTEDLINEID_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GETCONNECTEDLINEID_RSP_HEADER Param1;
} yPDef_MSG_CC_GETCONNECTEDLINEID_RSP;

typedef CAPI2_CcApi_GetCallPresent_Req_t *T_INTER_TASK_MSG_CC_GET_CALL_PRESENT_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GET_CALL_PRESENT_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GET_CALL_PRESENT_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GET_CALL_PRESENT_REQ_HEADER Param1;
} yPDef_MSG_CC_GET_CALL_PRESENT_REQ;

typedef Boolean *T_INTER_TASK_MSG_CC_GET_CALL_PRESENT_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GET_CALL_PRESENT_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GET_CALL_PRESENT_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GET_CALL_PRESENT_RSP_HEADER Param1;
} yPDef_MSG_CC_GET_CALL_PRESENT_RSP;

typedef CAPI2_CcApi_GetCallIndexInThisState_Req_t *T_INTER_TASK_MSG_CC_GET_INDEX_STATE_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GET_INDEX_STATE_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GET_INDEX_STATE_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GET_INDEX_STATE_REQ_HEADER Param1;
} yPDef_MSG_CC_GET_INDEX_STATE_REQ;

typedef Boolean *T_INTER_TASK_MSG_CC_GET_INDEX_STATE_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GET_INDEX_STATE_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GET_INDEX_STATE_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GET_INDEX_STATE_RSP_HEADER Param1;
} yPDef_MSG_CC_GET_INDEX_STATE_RSP;

typedef CAPI2_CcApi_IsMultiPartyCall_Req_t *T_INTER_TASK_MSG_CC_ISMULTIPARTYCALL_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_ISMULTIPARTYCALL_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_ISMULTIPARTYCALL_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_ISMULTIPARTYCALL_REQ_HEADER Param1;
} yPDef_MSG_CC_ISMULTIPARTYCALL_REQ;

typedef Boolean *T_INTER_TASK_MSG_CC_ISMULTIPARTYCALL_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_ISMULTIPARTYCALL_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_ISMULTIPARTYCALL_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_ISMULTIPARTYCALL_RSP_HEADER Param1;
} yPDef_MSG_CC_ISMULTIPARTYCALL_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_ISTHEREVOICECALL_REQ;

typedef Boolean *T_INTER_TASK_MSG_CC_ISTHEREVOICECALL_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_ISTHEREVOICECALL_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_ISTHEREVOICECALL_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_ISTHEREVOICECALL_RSP_HEADER Param1;
} yPDef_MSG_CC_ISTHEREVOICECALL_RSP;

typedef CAPI2_CcApi_IsConnectedLineIDPresentAllowed_Req_t *T_INTER_TASK_MSG_CC_ISCONNECTEDLINEIDPRESENTALLOWED_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_ISCONNECTEDLINEIDPRESENTALLOWED_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_ISCONNECTEDLINEIDPRESENTALLOWED_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_ISCONNECTEDLINEIDPRESENTALLOWED_REQ_HEADER Param1;
} yPDef_MSG_CC_ISCONNECTEDLINEIDPRESENTALLOWED_REQ;

typedef Boolean *T_INTER_TASK_MSG_CC_ISCONNECTEDLINEIDPRESENTALLOWED_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_ISCONNECTEDLINEIDPRESENTALLOWED_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_ISCONNECTEDLINEIDPRESENTALLOWED_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_ISCONNECTEDLINEIDPRESENTALLOWED_RSP_HEADER Param1;
} yPDef_MSG_CC_ISCONNECTEDLINEIDPRESENTALLOWED_RSP;

typedef CAPI2_CcApi_GetCurrentCallDurationInMilliSeconds_Req_t *T_INTER_TASK_MSG_CC_GETCURRENTCALLDURATIONINMILLISECONDS_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GETCURRENTCALLDURATIONINMILLISECONDS_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GETCURRENTCALLDURATIONINMILLISECONDS_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GETCURRENTCALLDURATIONINMILLISECONDS_REQ_HEADER Param1;
} yPDef_MSG_CC_GETCURRENTCALLDURATIONINMILLISECONDS_REQ;

typedef UInt32 *T_INTER_TASK_MSG_CC_GETCURRENTCALLDURATIONINMILLISECONDS_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GETCURRENTCALLDURATIONINMILLISECONDS_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GETCURRENTCALLDURATIONINMILLISECONDS_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GETCURRENTCALLDURATIONINMILLISECONDS_RSP_HEADER Param1;
} yPDef_MSG_CC_GETCURRENTCALLDURATIONINMILLISECONDS_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_GETLASTCALLCCM_REQ;

typedef UInt32 *T_INTER_TASK_MSG_CC_GETLASTCALLCCM_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GETLASTCALLCCM_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GETLASTCALLCCM_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GETLASTCALLCCM_RSP_HEADER Param1;
} yPDef_MSG_CC_GETLASTCALLCCM_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_GETLASTCALLDURATION_REQ;

typedef UInt32 *T_INTER_TASK_MSG_CC_GETLASTCALLDURATION_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GETLASTCALLDURATION_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GETLASTCALLDURATION_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GETLASTCALLDURATION_RSP_HEADER Param1;
} yPDef_MSG_CC_GETLASTCALLDURATION_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_GETLASTDATACALLRXBYTES_REQ;

typedef UInt32 *T_INTER_TASK_MSG_CC_GETLASTDATACALLRXBYTES_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GETLASTDATACALLRXBYTES_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GETLASTDATACALLRXBYTES_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GETLASTDATACALLRXBYTES_RSP_HEADER Param1;
} yPDef_MSG_CC_GETLASTDATACALLRXBYTES_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_GETLASTDATACALLTXBYTES_REQ;

typedef UInt32 *T_INTER_TASK_MSG_CC_GETLASTDATACALLTXBYTES_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GETLASTDATACALLTXBYTES_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GETLASTDATACALLTXBYTES_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GETLASTDATACALLTXBYTES_RSP_HEADER Param1;
} yPDef_MSG_CC_GETLASTDATACALLTXBYTES_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_GETDATACALLINDEX_REQ;

typedef UInt8 *T_INTER_TASK_MSG_CC_GETDATACALLINDEX_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GETDATACALLINDEX_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GETDATACALLINDEX_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GETDATACALLINDEX_RSP_HEADER Param1;
} yPDef_MSG_CC_GETDATACALLINDEX_RSP;

typedef CAPI2_CcApi_GetCallClientInfo_Req_t *T_INTER_TASK_MSG_CC_GETCALLCLIENT_INFO_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GETCALLCLIENT_INFO_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GETCALLCLIENT_INFO_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GETCALLCLIENT_INFO_REQ_HEADER Param1;
} yPDef_MSG_CC_GETCALLCLIENT_INFO_REQ;

typedef void *T_INTER_TASK_MSG_CC_GETCALLCLIENT_INFO_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GETCALLCLIENT_INFO_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GETCALLCLIENT_INFO_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GETCALLCLIENT_INFO_RSP_HEADER Param1;
} yPDef_MSG_CC_GETCALLCLIENT_INFO_RSP;

typedef CAPI2_CcApi_GetCallClientID_Req_t *T_INTER_TASK_MSG_CC_GETCALLCLIENTID_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GETCALLCLIENTID_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GETCALLCLIENTID_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GETCALLCLIENTID_REQ_HEADER Param1;
} yPDef_MSG_CC_GETCALLCLIENTID_REQ;

typedef UInt8 *T_INTER_TASK_MSG_CC_GETCALLCLIENTID_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GETCALLCLIENTID_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GETCALLCLIENTID_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GETCALLCLIENTID_RSP_HEADER Param1;
} yPDef_MSG_CC_GETCALLCLIENTID_RSP;

typedef CAPI2_CcApi_GetTypeAdd_Req_t *T_INTER_TASK_MSG_CC_GETTYPEADD_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GETTYPEADD_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GETTYPEADD_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GETTYPEADD_REQ_HEADER Param1;
} yPDef_MSG_CC_GETTYPEADD_REQ;

typedef UInt8 *T_INTER_TASK_MSG_CC_GETTYPEADD_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GETTYPEADD_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GETTYPEADD_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GETTYPEADD_RSP_HEADER Param1;
} yPDef_MSG_CC_GETTYPEADD_RSP;

typedef CAPI2_CcApi_SetVoiceCallAutoReject_Req_t *T_INTER_TASK_MSG_CC_SETVOICECALLAUTOREJECT_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_SETVOICECALLAUTOREJECT_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_SETVOICECALLAUTOREJECT_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_SETVOICECALLAUTOREJECT_REQ_HEADER Param1;
} yPDef_MSG_CC_SETVOICECALLAUTOREJECT_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_SETVOICECALLAUTOREJECT_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_ISVOICECALLAUTOREJECT_REQ;

typedef Boolean *T_INTER_TASK_MSG_CC_ISVOICECALLAUTOREJECT_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_ISVOICECALLAUTOREJECT_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_ISVOICECALLAUTOREJECT_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_ISVOICECALLAUTOREJECT_RSP_HEADER Param1;
} yPDef_MSG_CC_ISVOICECALLAUTOREJECT_RSP;

typedef CAPI2_CcApi_SetTTYCall_Req_t *T_INTER_TASK_MSG_CC_SETTTYCALL_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_SETTTYCALL_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_SETTTYCALL_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_SETTTYCALL_REQ_HEADER Param1;
} yPDef_MSG_CC_SETTTYCALL_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_SETTTYCALL_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_ISTTYENABLE_REQ;

typedef Boolean *T_INTER_TASK_MSG_CC_ISTTYENABLE_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_ISTTYENABLE_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_ISTTYENABLE_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_ISTTYENABLE_RSP_HEADER Param1;
} yPDef_MSG_CC_ISTTYENABLE_RSP;

typedef CAPI2_CcApi_IsSimOriginedCall_Req_t *T_INTER_TASK_MSG_CC_ISSIMORIGINEDCALL_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_ISSIMORIGINEDCALL_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_ISSIMORIGINEDCALL_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_ISSIMORIGINEDCALL_REQ_HEADER Param1;
} yPDef_MSG_CC_ISSIMORIGINEDCALL_REQ;

typedef Boolean *T_INTER_TASK_MSG_CC_ISSIMORIGINEDCALL_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_ISSIMORIGINEDCALL_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_ISSIMORIGINEDCALL_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_ISSIMORIGINEDCALL_RSP_HEADER Param1;
} yPDef_MSG_CC_ISSIMORIGINEDCALL_RSP;

typedef CAPI2_CcApi_SetVideoCallParam_Req_t *T_INTER_TASK_MSG_CC_SETVIDEOCALLPARAM_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_SETVIDEOCALLPARAM_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_SETVIDEOCALLPARAM_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_SETVIDEOCALLPARAM_REQ_HEADER Param1;
} yPDef_MSG_CC_SETVIDEOCALLPARAM_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_SETVIDEOCALLPARAM_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_GETVIDEOCALLPARAM_REQ;

typedef VideoCallParam_t *T_INTER_TASK_MSG_CC_GETVIDEOCALLPARAM_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GETVIDEOCALLPARAM_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GETVIDEOCALLPARAM_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GETVIDEOCALLPARAM_RSP_HEADER Param1;
} yPDef_MSG_CC_GETVIDEOCALLPARAM_RSP;

typedef CAPI2_CcApi_GetCCM_Req_t *T_INTER_TASK_MSG_CC_GETCCM_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GETCCM_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GETCCM_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GETCCM_REQ_HEADER Param1;
} yPDef_MSG_CC_GETCCM_REQ;

typedef Result_t *T_INTER_TASK_MSG_CC_GETCCM_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GETCCM_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GETCCM_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GETCCM_RSP_HEADER Param1;
} yPDef_MSG_CC_GETCCM_RSP;

typedef CAPI2_CcApi_SendDtmfTone_Req_t *T_INTER_TASK_MSG_CCAPI_SENDDTMF_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CCAPI_SENDDTMF_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CCAPI_SENDDTMF_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CCAPI_SENDDTMF_REQ_HEADER Param1;
} yPDef_MSG_CCAPI_SENDDTMF_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CCAPI_SENDDTMF_RSP;

typedef CAPI2_CcApi_StopDtmfTone_Req_t *T_INTER_TASK_MSG_CCAPI_STOPDTMF_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CCAPI_STOPDTMF_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CCAPI_STOPDTMF_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CCAPI_STOPDTMF_REQ_HEADER Param1;
} yPDef_MSG_CCAPI_STOPDTMF_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CCAPI_STOPDTMF_RSP;

typedef CAPI2_CcApi_AbortDtmf_Req_t *T_INTER_TASK_MSG_CCAPI_ABORTDTMF_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CCAPI_ABORTDTMF_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CCAPI_ABORTDTMF_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CCAPI_ABORTDTMF_REQ_HEADER Param1;
} yPDef_MSG_CCAPI_ABORTDTMF_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CCAPI_ABORTDTMF_RSP;

typedef CAPI2_CcApi_SetDtmfToneTimer_Req_t *T_INTER_TASK_MSG_CCAPI_SETDTMFTIMER_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CCAPI_SETDTMFTIMER_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CCAPI_SETDTMFTIMER_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CCAPI_SETDTMFTIMER_REQ_HEADER Param1;
} yPDef_MSG_CCAPI_SETDTMFTIMER_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CCAPI_SETDTMFTIMER_RSP;

typedef CAPI2_CcApi_ResetDtmfToneTimer_Req_t *T_INTER_TASK_MSG_CCAPI_RESETDTMFTIMER_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CCAPI_RESETDTMFTIMER_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CCAPI_RESETDTMFTIMER_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CCAPI_RESETDTMFTIMER_REQ_HEADER Param1;
} yPDef_MSG_CCAPI_RESETDTMFTIMER_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CCAPI_RESETDTMFTIMER_RSP;

typedef CAPI2_CcApi_GetDtmfToneTimer_Req_t *T_INTER_TASK_MSG_CCAPI_GETDTMFTIMER_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CCAPI_GETDTMFTIMER_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CCAPI_GETDTMFTIMER_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CCAPI_GETDTMFTIMER_REQ_HEADER Param1;
} yPDef_MSG_CCAPI_GETDTMFTIMER_REQ;

typedef Ticks_t *T_INTER_TASK_MSG_CCAPI_GETDTMFTIMER_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CCAPI_GETDTMFTIMER_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CCAPI_GETDTMFTIMER_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CCAPI_GETDTMFTIMER_RSP_HEADER Param1;
} yPDef_MSG_CCAPI_GETDTMFTIMER_RSP;

typedef CAPI2_CcApi_GetTiFromCallIndex_Req_t *T_INTER_TASK_MSG_CCAPI_GETTIFROMCALLINDEX_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CCAPI_GETTIFROMCALLINDEX_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CCAPI_GETTIFROMCALLINDEX_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CCAPI_GETTIFROMCALLINDEX_REQ_HEADER Param1;
} yPDef_MSG_CCAPI_GETTIFROMCALLINDEX_REQ;

typedef Result_t *T_INTER_TASK_MSG_CCAPI_GETTIFROMCALLINDEX_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CCAPI_GETTIFROMCALLINDEX_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CCAPI_GETTIFROMCALLINDEX_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CCAPI_GETTIFROMCALLINDEX_RSP_HEADER Param1;
} yPDef_MSG_CCAPI_GETTIFROMCALLINDEX_RSP;

typedef CAPI2_CcApi_IsSupportedBC_Req_t *T_INTER_TASK_MSG_CCAPI_IS_SUPPORTEDBC_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CCAPI_IS_SUPPORTEDBC_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CCAPI_IS_SUPPORTEDBC_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CCAPI_IS_SUPPORTEDBC_REQ_HEADER Param1;
} yPDef_MSG_CCAPI_IS_SUPPORTEDBC_REQ;

typedef Boolean *T_INTER_TASK_MSG_CCAPI_IS_SUPPORTEDBC_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CCAPI_IS_SUPPORTEDBC_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CCAPI_IS_SUPPORTEDBC_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CCAPI_IS_SUPPORTEDBC_RSP_HEADER Param1;
} yPDef_MSG_CCAPI_IS_SUPPORTEDBC_RSP;

typedef CAPI2_CcApi_GetBearerCapability_Req_t *T_INTER_TASK_MSG_CCAPI_IS_BEARER_CAPABILITY_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CCAPI_IS_BEARER_CAPABILITY_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CCAPI_IS_BEARER_CAPABILITY_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CCAPI_IS_BEARER_CAPABILITY_REQ_HEADER Param1;
} yPDef_MSG_CCAPI_IS_BEARER_CAPABILITY_REQ;

typedef Result_t *T_INTER_TASK_MSG_CCAPI_IS_BEARER_CAPABILITY_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CCAPI_IS_BEARER_CAPABILITY_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CCAPI_IS_BEARER_CAPABILITY_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CCAPI_IS_BEARER_CAPABILITY_RSP_HEADER Param1;
} yPDef_MSG_CCAPI_IS_BEARER_CAPABILITY_RSP;

typedef CAPI2_SmsApi_SendSMSSrvCenterNumberUpdateReq_Req_t *T_INTER_TASK_MSG_SMS_SENDSMSSRVCENTERNUMBERUPDATE_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_SENDSMSSRVCENTERNUMBERUPDATE_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_SENDSMSSRVCENTERNUMBERUPDATE_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_SENDSMSSRVCENTERNUMBERUPDATE_REQ_HEADER Param1;
} yPDef_MSG_SMS_SENDSMSSRVCENTERNUMBERUPDATE_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SENDSMSSRVCENTERNUMBERUPDATE_RSP;

typedef CAPI2_SmsApi_GetSMSrvCenterNumber_Req_t *T_INTER_TASK_MSG_SMS_GETSMSSRVCENTERNUMBER_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_GETSMSSRVCENTERNUMBER_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_GETSMSSRVCENTERNUMBER_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_GETSMSSRVCENTERNUMBER_REQ_HEADER Param1;
} yPDef_MSG_SMS_GETSMSSRVCENTERNUMBER_REQ;

typedef Result_t *T_INTER_TASK_MSG_SMS_GETSMSSRVCENTERNUMBER_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_GETSMSSRVCENTERNUMBER_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_GETSMSSRVCENTERNUMBER_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_GETSMSSRVCENTERNUMBER_RSP_HEADER Param1;
} yPDef_MSG_SMS_GETSMSSRVCENTERNUMBER_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_GETSIMSMSCAPACITYEXCEEDEDFLAG_REQ;

typedef Result_t *T_INTER_TASK_MSG_SMS_GETSIMSMSCAPACITYEXCEEDEDFLAG_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_GETSIMSMSCAPACITYEXCEEDEDFLAG_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_GETSIMSMSCAPACITYEXCEEDEDFLAG_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_GETSIMSMSCAPACITYEXCEEDEDFLAG_RSP_HEADER Param1;
} yPDef_MSG_SMS_GETSIMSMSCAPACITYEXCEEDEDFLAG_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_ISSMSSERVICEAVAIL_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_ISSMSSERVICEAVAIL_RSP;

typedef CAPI2_SmsApi_GetSmsStoredState_Req_t *T_INTER_TASK_MSG_SMS_GETSMSSTOREDSTATE_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_GETSMSSTOREDSTATE_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_GETSMSSTOREDSTATE_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_GETSMSSTOREDSTATE_REQ_HEADER Param1;
} yPDef_MSG_SMS_GETSMSSTOREDSTATE_REQ;

typedef Result_t *T_INTER_TASK_MSG_SMS_GETSMSSTOREDSTATE_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_GETSMSSTOREDSTATE_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_GETSMSSTOREDSTATE_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_GETSMSSTOREDSTATE_RSP_HEADER Param1;
} yPDef_MSG_SMS_GETSMSSTOREDSTATE_RSP;

typedef CAPI2_SmsApi_WriteSMSPduReq_Req_t *T_INTER_TASK_MSG_SMS_WRITESMSPDU_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_WRITESMSPDU_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_WRITESMSPDU_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_WRITESMSPDU_REQ_HEADER Param1;
} yPDef_MSG_SMS_WRITESMSPDU_REQ;

typedef CAPI2_SmsApi_WriteSMSReq_Req_t *T_INTER_TASK_MSG_SMS_WRITESMSREQ_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_WRITESMSREQ_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_WRITESMSREQ_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_WRITESMSREQ_REQ_HEADER Param1;
} yPDef_MSG_SMS_WRITESMSREQ_REQ;

typedef CAPI2_SmsApi_SendSMSReq_Req_t *T_INTER_TASK_MSG_SMS_SENDSMSREQ_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_SENDSMSREQ_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_SENDSMSREQ_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_SENDSMSREQ_REQ_HEADER Param1;
} yPDef_MSG_SMS_SENDSMSREQ_REQ;

typedef CAPI2_SmsApi_SendSMSPduReq_Req_t *T_INTER_TASK_MSG_SMS_SENDSMSPDUREQ_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_SENDSMSPDUREQ_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_SENDSMSPDUREQ_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_SENDSMSPDUREQ_REQ_HEADER Param1;
} yPDef_MSG_SMS_SENDSMSPDUREQ_REQ;

typedef CAPI2_SmsApi_SendStoredSmsReq_Req_t *T_INTER_TASK_MSG_SMS_SENDSTOREDSMSREQ_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_SENDSTOREDSMSREQ_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_SENDSTOREDSMSREQ_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_SENDSTOREDSMSREQ_REQ_HEADER Param1;
} yPDef_MSG_SMS_SENDSTOREDSMSREQ_REQ;

typedef CAPI2_SmsApi_WriteSMSPduToSIMReq_Req_t *T_INTER_TASK_MSG_SMS_WRITESMSPDUTOSIM_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_WRITESMSPDUTOSIM_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_WRITESMSPDUTOSIM_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_WRITESMSPDUTOSIM_REQ_HEADER Param1;
} yPDef_MSG_SMS_WRITESMSPDUTOSIM_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_GETLASTTPMR_REQ;

typedef UInt8 *T_INTER_TASK_MSG_SMS_GETLASTTPMR_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_GETLASTTPMR_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_GETLASTTPMR_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_GETLASTTPMR_RSP_HEADER Param1;
} yPDef_MSG_SMS_GETLASTTPMR_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_GETSMSTXPARAMS_REQ;

typedef Result_t *T_INTER_TASK_MSG_SMS_GETSMSTXPARAMS_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_GETSMSTXPARAMS_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_GETSMSTXPARAMS_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_GETSMSTXPARAMS_RSP_HEADER Param1;
} yPDef_MSG_SMS_GETSMSTXPARAMS_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_GETTXPARAMINTEXTMODE_REQ;

typedef Result_t *T_INTER_TASK_MSG_SMS_GETTXPARAMINTEXTMODE_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_GETTXPARAMINTEXTMODE_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_GETTXPARAMINTEXTMODE_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_GETTXPARAMINTEXTMODE_RSP_HEADER Param1;
} yPDef_MSG_SMS_GETTXPARAMINTEXTMODE_RSP;

typedef CAPI2_SmsApi_SetSmsTxParamProcId_Req_t *T_INTER_TASK_MSG_SMS_SETSMSTXPARAMPROCID_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_SETSMSTXPARAMPROCID_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_SETSMSTXPARAMPROCID_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_SETSMSTXPARAMPROCID_REQ_HEADER Param1;
} yPDef_MSG_SMS_SETSMSTXPARAMPROCID_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SETSMSTXPARAMPROCID_RSP;

typedef CAPI2_SmsApi_SetSmsTxParamCodingType_Req_t *T_INTER_TASK_MSG_SMS_SETSMSTXPARAMCODINGTYPE_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_SETSMSTXPARAMCODINGTYPE_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_SETSMSTXPARAMCODINGTYPE_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_SETSMSTXPARAMCODINGTYPE_REQ_HEADER Param1;
} yPDef_MSG_SMS_SETSMSTXPARAMCODINGTYPE_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SETSMSTXPARAMCODINGTYPE_RSP;

typedef CAPI2_SmsApi_SetSmsTxParamValidPeriod_Req_t *T_INTER_TASK_MSG_SMS_SETSMSTXPARAMVALIDPERIOD_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_SETSMSTXPARAMVALIDPERIOD_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_SETSMSTXPARAMVALIDPERIOD_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_SETSMSTXPARAMVALIDPERIOD_REQ_HEADER Param1;
} yPDef_MSG_SMS_SETSMSTXPARAMVALIDPERIOD_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SETSMSTXPARAMVALIDPERIOD_RSP;

typedef CAPI2_SmsApi_SetSmsTxParamCompression_Req_t *T_INTER_TASK_MSG_SMS_SETSMSTXPARAMCOMPRESSION_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_SETSMSTXPARAMCOMPRESSION_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_SETSMSTXPARAMCOMPRESSION_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_SETSMSTXPARAMCOMPRESSION_REQ_HEADER Param1;
} yPDef_MSG_SMS_SETSMSTXPARAMCOMPRESSION_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SETSMSTXPARAMCOMPRESSION_RSP;

typedef CAPI2_SmsApi_SetSmsTxParamReplyPath_Req_t *T_INTER_TASK_MSG_SMS_SETSMSTXPARAMREPLYPATH_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_SETSMSTXPARAMREPLYPATH_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_SETSMSTXPARAMREPLYPATH_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_SETSMSTXPARAMREPLYPATH_REQ_HEADER Param1;
} yPDef_MSG_SMS_SETSMSTXPARAMREPLYPATH_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SETSMSTXPARAMREPLYPATH_RSP;

typedef CAPI2_SmsApi_SetSmsTxParamUserDataHdrInd_Req_t *T_INTER_TASK_MSG_SMS_SETSMSTXPARAMUSERDATAHDRIND_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_SETSMSTXPARAMUSERDATAHDRIND_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_SETSMSTXPARAMUSERDATAHDRIND_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_SETSMSTXPARAMUSERDATAHDRIND_REQ_HEADER Param1;
} yPDef_MSG_SMS_SETSMSTXPARAMUSERDATAHDRIND_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SETSMSTXPARAMUSERDATAHDRIND_RSP;

typedef CAPI2_SmsApi_SetSmsTxParamStatusRptReqFlag_Req_t *T_INTER_TASK_MSG_SMS_SETSMSTXPARAMSTATUSRPTREQFLAG_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_SETSMSTXPARAMSTATUSRPTREQFLAG_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_SETSMSTXPARAMSTATUSRPTREQFLAG_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_SETSMSTXPARAMSTATUSRPTREQFLAG_REQ_HEADER Param1;
} yPDef_MSG_SMS_SETSMSTXPARAMSTATUSRPTREQFLAG_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SETSMSTXPARAMSTATUSRPTREQFLAG_RSP;

typedef CAPI2_SmsApi_SetSmsTxParamRejDupl_Req_t *T_INTER_TASK_MSG_SMS_SETSMSTXPARAMREJDUPL_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_SETSMSTXPARAMREJDUPL_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_SETSMSTXPARAMREJDUPL_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_SETSMSTXPARAMREJDUPL_REQ_HEADER Param1;
} yPDef_MSG_SMS_SETSMSTXPARAMREJDUPL_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SETSMSTXPARAMREJDUPL_RSP;

typedef CAPI2_SmsApi_DeleteSmsMsgByIndexReq_Req_t *T_INTER_TASK_MSG_SMS_DELETESMSMSGBYINDEX_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_DELETESMSMSGBYINDEX_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_DELETESMSMSGBYINDEX_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_DELETESMSMSGBYINDEX_REQ_HEADER Param1;
} yPDef_MSG_SMS_DELETESMSMSGBYINDEX_REQ;

typedef CAPI2_SmsApi_ReadSmsMsgReq_Req_t *T_INTER_TASK_MSG_SMS_READSMSMSG_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_READSMSMSG_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_READSMSMSG_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_READSMSMSG_REQ_HEADER Param1;
} yPDef_MSG_SMS_READSMSMSG_REQ;

typedef CAPI2_SmsApi_ListSmsMsgReq_Req_t *T_INTER_TASK_MSG_SMS_LISTSMSMSG_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_LISTSMSMSG_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_LISTSMSMSG_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_LISTSMSMSG_REQ_HEADER Param1;
} yPDef_MSG_SMS_LISTSMSMSG_REQ;

typedef CAPI2_SmsApi_SetNewMsgDisplayPref_Req_t *T_INTER_TASK_MSG_SMS_SETNEWMSGDISPLAYPREF_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_SETNEWMSGDISPLAYPREF_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_SETNEWMSGDISPLAYPREF_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_SETNEWMSGDISPLAYPREF_REQ_HEADER Param1;
} yPDef_MSG_SMS_SETNEWMSGDISPLAYPREF_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SETNEWMSGDISPLAYPREF_RSP;

typedef CAPI2_SmsApi_GetNewMsgDisplayPref_Req_t *T_INTER_TASK_MSG_SMS_GETNEWMSGDISPLAYPREF_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_GETNEWMSGDISPLAYPREF_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_GETNEWMSGDISPLAYPREF_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_GETNEWMSGDISPLAYPREF_REQ_HEADER Param1;
} yPDef_MSG_SMS_GETNEWMSGDISPLAYPREF_REQ;

typedef UInt8 *T_INTER_TASK_MSG_SMS_GETNEWMSGDISPLAYPREF_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_GETNEWMSGDISPLAYPREF_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_GETNEWMSGDISPLAYPREF_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_GETNEWMSGDISPLAYPREF_RSP_HEADER Param1;
} yPDef_MSG_SMS_GETNEWMSGDISPLAYPREF_RSP;

typedef CAPI2_SmsApi_SetSMSPrefStorage_Req_t *T_INTER_TASK_MSG_SMS_SETSMSPREFSTORAGE_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_SETSMSPREFSTORAGE_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_SETSMSPREFSTORAGE_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_SETSMSPREFSTORAGE_REQ_HEADER Param1;
} yPDef_MSG_SMS_SETSMSPREFSTORAGE_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SETSMSPREFSTORAGE_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_GETSMSPREFSTORAGE_REQ;

typedef SmsStorage_t *T_INTER_TASK_MSG_SMS_GETSMSPREFSTORAGE_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_GETSMSPREFSTORAGE_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_GETSMSPREFSTORAGE_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_GETSMSPREFSTORAGE_RSP_HEADER Param1;
} yPDef_MSG_SMS_GETSMSPREFSTORAGE_RSP;

typedef CAPI2_SmsApi_GetSMSStorageStatus_Req_t *T_INTER_TASK_MSG_SMS_GETSMSSTORAGESTATUS_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_GETSMSSTORAGESTATUS_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_GETSMSSTORAGESTATUS_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_GETSMSSTORAGESTATUS_REQ_HEADER Param1;
} yPDef_MSG_SMS_GETSMSSTORAGESTATUS_REQ;

typedef Result_t *T_INTER_TASK_MSG_SMS_GETSMSSTORAGESTATUS_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_GETSMSSTORAGESTATUS_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_GETSMSSTORAGESTATUS_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_GETSMSSTORAGESTATUS_RSP_HEADER Param1;
} yPDef_MSG_SMS_GETSMSSTORAGESTATUS_RSP;

typedef CAPI2_SmsApi_SaveSmsServiceProfile_Req_t *T_INTER_TASK_MSG_SMS_SAVESMSSERVICEPROFILE_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_SAVESMSSERVICEPROFILE_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_SAVESMSSERVICEPROFILE_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_SAVESMSSERVICEPROFILE_REQ_HEADER Param1;
} yPDef_MSG_SMS_SAVESMSSERVICEPROFILE_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SAVESMSSERVICEPROFILE_RSP;

typedef CAPI2_SmsApi_RestoreSmsServiceProfile_Req_t *T_INTER_TASK_MSG_SMS_RESTORESMSSERVICEPROFILE_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_RESTORESMSSERVICEPROFILE_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_RESTORESMSSERVICEPROFILE_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_RESTORESMSSERVICEPROFILE_REQ_HEADER Param1;
} yPDef_MSG_SMS_RESTORESMSSERVICEPROFILE_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_RESTORESMSSERVICEPROFILE_RSP;

typedef CAPI2_SmsApi_SetCellBroadcastMsgTypeReq_Req_t *T_INTER_TASK_MSG_SMS_SETCELLBROADCASTMSGTYPE_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_SETCELLBROADCASTMSGTYPE_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_SETCELLBROADCASTMSGTYPE_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_SETCELLBROADCASTMSGTYPE_REQ_HEADER Param1;
} yPDef_MSG_SMS_SETCELLBROADCASTMSGTYPE_REQ;

typedef CAPI2_SmsApi_CBAllowAllChnlReq_Req_t *T_INTER_TASK_MSG_SMS_CBALLOWALLCHNLREQ_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_CBALLOWALLCHNLREQ_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_CBALLOWALLCHNLREQ_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_CBALLOWALLCHNLREQ_REQ_HEADER Param1;
} yPDef_MSG_SMS_CBALLOWALLCHNLREQ_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_CBALLOWALLCHNLREQ_RSP;

typedef CAPI2_SmsApi_AddCellBroadcastChnlReq_Req_t *T_INTER_TASK_MSG_SMS_ADDCELLBROADCASTCHNLREQ_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_ADDCELLBROADCASTCHNLREQ_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_ADDCELLBROADCASTCHNLREQ_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_ADDCELLBROADCASTCHNLREQ_REQ_HEADER Param1;
} yPDef_MSG_SMS_ADDCELLBROADCASTCHNLREQ_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_ADDCELLBROADCASTCHNLREQ_RSP;

typedef CAPI2_SmsApi_RemoveCellBroadcastChnlReq_Req_t *T_INTER_TASK_MSG_SMS_REMOVECELLBROADCASTCHNLREQ_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_REMOVECELLBROADCASTCHNLREQ_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_REMOVECELLBROADCASTCHNLREQ_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_REMOVECELLBROADCASTCHNLREQ_REQ_HEADER Param1;
} yPDef_MSG_SMS_REMOVECELLBROADCASTCHNLREQ_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_REMOVECELLBROADCASTCHNLREQ_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_REMOVEALLCBCHNLFROMSEARCHLIST_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_REMOVEALLCBCHNLFROMSEARCHLIST_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_GETCBMI_REQ;

typedef Result_t *T_INTER_TASK_MSG_SMS_GETCBMI_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_GETCBMI_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_GETCBMI_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_GETCBMI_RSP_HEADER Param1;
} yPDef_MSG_SMS_GETCBMI_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_GETCBLANGUAGE_REQ;

typedef Result_t *T_INTER_TASK_MSG_SMS_GETCBLANGUAGE_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_GETCBLANGUAGE_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_GETCBLANGUAGE_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_GETCBLANGUAGE_RSP_HEADER Param1;
} yPDef_MSG_SMS_GETCBLANGUAGE_RSP;

typedef CAPI2_SmsApi_AddCellBroadcastLangReq_Req_t *T_INTER_TASK_MSG_SMS_ADDCELLBROADCASTLANGREQ_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_ADDCELLBROADCASTLANGREQ_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_ADDCELLBROADCASTLANGREQ_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_ADDCELLBROADCASTLANGREQ_REQ_HEADER Param1;
} yPDef_MSG_SMS_ADDCELLBROADCASTLANGREQ_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_ADDCELLBROADCASTLANGREQ_RSP;

typedef CAPI2_SmsApi_RemoveCellBroadcastLangReq_Req_t *T_INTER_TASK_MSG_SMS_REMOVECELLBROADCASTLANGREQ_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_REMOVECELLBROADCASTLANGREQ_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_REMOVECELLBROADCASTLANGREQ_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_REMOVECELLBROADCASTLANGREQ_REQ_HEADER Param1;
} yPDef_MSG_SMS_REMOVECELLBROADCASTLANGREQ_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_REMOVECELLBROADCASTLANGREQ_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_STARTRECEIVINGCELLBROADCAST_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_STOPRECEIVINGCELLBROADCAST_REQ;

typedef CAPI2_SmsApi_SetCBIgnoreDuplFlag_Req_t *T_INTER_TASK_MSG_SMS_SETCBIGNOREDUPLFLAG_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_SETCBIGNOREDUPLFLAG_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_SETCBIGNOREDUPLFLAG_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_SETCBIGNOREDUPLFLAG_REQ_HEADER Param1;
} yPDef_MSG_SMS_SETCBIGNOREDUPLFLAG_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SETCBIGNOREDUPLFLAG_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_GETCBIGNOREDUPLFLAG_REQ;

typedef Boolean *T_INTER_TASK_MSG_SMS_GETCBIGNOREDUPLFLAG_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_GETCBIGNOREDUPLFLAG_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_GETCBIGNOREDUPLFLAG_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_GETCBIGNOREDUPLFLAG_RSP_HEADER Param1;
} yPDef_MSG_SMS_GETCBIGNOREDUPLFLAG_RSP;

typedef CAPI2_SmsApi_SetVMIndOnOff_Req_t *T_INTER_TASK_MSG_SMS_SETVMINDONOFF_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_SETVMINDONOFF_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_SETVMINDONOFF_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_SETVMINDONOFF_REQ_HEADER Param1;
} yPDef_MSG_SMS_SETVMINDONOFF_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SETVMINDONOFF_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_ISVMINDENABLED_REQ;

typedef Boolean *T_INTER_TASK_MSG_SMS_ISVMINDENABLED_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_ISVMINDENABLED_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_ISVMINDENABLED_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_ISVMINDENABLED_RSP_HEADER Param1;
} yPDef_MSG_SMS_ISVMINDENABLED_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_GETVMWAITINGSTATUS_REQ;

typedef Result_t *T_INTER_TASK_MSG_SMS_GETVMWAITINGSTATUS_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_GETVMWAITINGSTATUS_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_GETVMWAITINGSTATUS_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_GETVMWAITINGSTATUS_RSP_HEADER Param1;
} yPDef_MSG_SMS_GETVMWAITINGSTATUS_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_GETNUMOFVMSCNUMBER_REQ;

typedef Result_t *T_INTER_TASK_MSG_SMS_GETNUMOFVMSCNUMBER_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_GETNUMOFVMSCNUMBER_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_GETNUMOFVMSCNUMBER_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_GETNUMOFVMSCNUMBER_RSP_HEADER Param1;
} yPDef_MSG_SMS_GETNUMOFVMSCNUMBER_RSP;

typedef CAPI2_SmsApi_GetVmscNumber_Req_t *T_INTER_TASK_MSG_SMS_GETVMSCNUMBER_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_GETVMSCNUMBER_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_GETVMSCNUMBER_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_GETVMSCNUMBER_REQ_HEADER Param1;
} yPDef_MSG_SMS_GETVMSCNUMBER_REQ;

typedef Result_t *T_INTER_TASK_MSG_SMS_GETVMSCNUMBER_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_GETVMSCNUMBER_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_GETVMSCNUMBER_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_GETVMSCNUMBER_RSP_HEADER Param1;
} yPDef_MSG_SMS_GETVMSCNUMBER_RSP;

typedef CAPI2_SmsApi_UpdateVmscNumberReq_Req_t *T_INTER_TASK_MSG_SMS_UPDATEVMSCNUMBERREQ_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_UPDATEVMSCNUMBERREQ_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_UPDATEVMSCNUMBERREQ_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_UPDATEVMSCNUMBERREQ_REQ_HEADER Param1;
} yPDef_MSG_SMS_UPDATEVMSCNUMBERREQ_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_UPDATEVMSCNUMBERREQ_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_GETSMSBEARERPREFERENCE_REQ;

typedef SMS_BEARER_PREFERENCE_t *T_INTER_TASK_MSG_SMS_GETSMSBEARERPREFERENCE_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_GETSMSBEARERPREFERENCE_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_GETSMSBEARERPREFERENCE_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_GETSMSBEARERPREFERENCE_RSP_HEADER Param1;
} yPDef_MSG_SMS_GETSMSBEARERPREFERENCE_RSP;

typedef CAPI2_SmsApi_SetSMSBearerPreference_Req_t *T_INTER_TASK_MSG_SMS_SETSMSBEARERPREFERENCE_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_SETSMSBEARERPREFERENCE_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_SETSMSBEARERPREFERENCE_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_SETSMSBEARERPREFERENCE_REQ_HEADER Param1;
} yPDef_MSG_SMS_SETSMSBEARERPREFERENCE_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SETSMSBEARERPREFERENCE_RSP;

typedef CAPI2_SmsApi_SetSmsReadStatusChangeMode_Req_t *T_INTER_TASK_MSG_SMS_SETSMSREADSTATUSCHANGEMODE_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_SETSMSREADSTATUSCHANGEMODE_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_SETSMSREADSTATUSCHANGEMODE_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_SETSMSREADSTATUSCHANGEMODE_REQ_HEADER Param1;
} yPDef_MSG_SMS_SETSMSREADSTATUSCHANGEMODE_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SETSMSREADSTATUSCHANGEMODE_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_GETSMSREADSTATUSCHANGEMODE_REQ;

typedef Boolean *T_INTER_TASK_MSG_SMS_GETSMSREADSTATUSCHANGEMODE_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_GETSMSREADSTATUSCHANGEMODE_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_GETSMSREADSTATUSCHANGEMODE_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_GETSMSREADSTATUSCHANGEMODE_RSP_HEADER Param1;
} yPDef_MSG_SMS_GETSMSREADSTATUSCHANGEMODE_RSP;

typedef CAPI2_SmsApi_ChangeSmsStatusReq_Req_t *T_INTER_TASK_MSG_SMS_CHANGESTATUSREQ_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_CHANGESTATUSREQ_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_CHANGESTATUSREQ_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_CHANGESTATUSREQ_REQ_HEADER Param1;
} yPDef_MSG_SMS_CHANGESTATUSREQ_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_CHANGESTATUSREQ_RSP;

typedef CAPI2_SmsApi_SendMEStoredStatusInd_Req_t *T_INTER_TASK_MSG_SMS_SENDMESTOREDSTATUSIND_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_SENDMESTOREDSTATUSIND_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_SENDMESTOREDSTATUSIND_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_SENDMESTOREDSTATUSIND_REQ_HEADER Param1;
} yPDef_MSG_SMS_SENDMESTOREDSTATUSIND_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SENDMESTOREDSTATUSIND_RSP;

typedef CAPI2_SmsApi_SendMERetrieveSmsDataInd_Req_t *T_INTER_TASK_MSG_SMS_SENDMERETRIEVESMSDATAIND_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_SENDMERETRIEVESMSDATAIND_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_SENDMERETRIEVESMSDATAIND_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_SENDMERETRIEVESMSDATAIND_REQ_HEADER Param1;
} yPDef_MSG_SMS_SENDMERETRIEVESMSDATAIND_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SENDMERETRIEVESMSDATAIND_RSP;

typedef CAPI2_SmsApi_SendMERemovedStatusInd_Req_t *T_INTER_TASK_MSG_SMS_SENDMEREMOVEDSTATUSIND_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_SENDMEREMOVEDSTATUSIND_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_SENDMEREMOVEDSTATUSIND_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_SENDMEREMOVEDSTATUSIND_REQ_HEADER Param1;
} yPDef_MSG_SMS_SENDMEREMOVEDSTATUSIND_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SENDMEREMOVEDSTATUSIND_RSP;

typedef CAPI2_SmsApi_SetSmsStoredState_Req_t *T_INTER_TASK_MSG_SMS_SETSMSSTOREDSTATE_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_SETSMSSTOREDSTATE_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_SETSMSSTOREDSTATE_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_SETSMSSTOREDSTATE_REQ_HEADER Param1;
} yPDef_MSG_SMS_SETSMSSTOREDSTATE_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SETSMSSTOREDSTATE_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_ISCACHEDDATAREADY_REQ;

typedef Boolean *T_INTER_TASK_MSG_SMS_ISCACHEDDATAREADY_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_ISCACHEDDATAREADY_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_ISCACHEDDATAREADY_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_ISCACHEDDATAREADY_RSP_HEADER Param1;
} yPDef_MSG_SMS_ISCACHEDDATAREADY_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_GETENHANCEDVMINFOIEI_REQ;

typedef SmsEnhancedVMInd_t *T_INTER_TASK_MSG_SMS_GETENHANCEDVMINFOIEI_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_GETENHANCEDVMINFOIEI_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_GETENHANCEDVMINFOIEI_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_GETENHANCEDVMINFOIEI_RSP_HEADER Param1;
} yPDef_MSG_SMS_GETENHANCEDVMINFOIEI_RSP;

typedef CAPI2_SmsApi_SetAllNewMsgDisplayPref_Req_t *T_INTER_TASK_MSG_SMS_SETALLNEWMSGDISPLAYPREF_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_SETALLNEWMSGDISPLAYPREF_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_SETALLNEWMSGDISPLAYPREF_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_SETALLNEWMSGDISPLAYPREF_REQ_HEADER Param1;
} yPDef_MSG_SMS_SETALLNEWMSGDISPLAYPREF_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SETALLNEWMSGDISPLAYPREF_RSP;

typedef CAPI2_SmsApi_SendAckToNetwork_Req_t *T_INTER_TASK_MSG_SMS_ACKTONETWORK_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_ACKTONETWORK_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_ACKTONETWORK_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_ACKTONETWORK_REQ_HEADER Param1;
} yPDef_MSG_SMS_ACKTONETWORK_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_ACKTONETWORK_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_STARTMULTISMSTRANSFER_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_STARTMULTISMSTRANSFER_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_STOPMULTISMSTRANSFER_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_STOPMULTISMSTRANSFER_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_START_CELL_BROADCAST_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_START_CELL_BROADCAST_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SIMINIT_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_SIMINIT_RSP;

typedef CAPI2_SMS_SetPDAStorageOverFlowFlag_Req_t *T_INTER_TASK_MSG_SMS_PDA_OVERFLOW_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_PDA_OVERFLOW_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_PDA_OVERFLOW_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_PDA_OVERFLOW_REQ_HEADER Param1;
} yPDef_MSG_SMS_PDA_OVERFLOW_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_PDA_OVERFLOW_RSP;

typedef CAPI2_ISimApi_SendAuthenAkaReq_Req_t *T_INTER_TASK_MSG_ISIM_SENDAUTHENAKAREQ_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_ISIM_SENDAUTHENAKAREQ_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_ISIM_SENDAUTHENAKAREQ_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_ISIM_SENDAUTHENAKAREQ_REQ_HEADER Param1;
} yPDef_MSG_ISIM_SENDAUTHENAKAREQ_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_ISIM_ISISIMSUPPORTED_REQ;

typedef Boolean *T_INTER_TASK_MSG_ISIM_ISISIMSUPPORTED_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_ISIM_ISISIMSUPPORTED_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_ISIM_ISISIMSUPPORTED_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_ISIM_ISISIMSUPPORTED_RSP_HEADER Param1;
} yPDef_MSG_ISIM_ISISIMSUPPORTED_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_ISIM_ISISIMACTIVATED_REQ;

typedef Boolean *T_INTER_TASK_MSG_ISIM_ISISIMACTIVATED_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_ISIM_ISISIMACTIVATED_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_ISIM_ISISIMACTIVATED_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_ISIM_ISISIMACTIVATED_RSP_HEADER Param1;
} yPDef_MSG_ISIM_ISISIMACTIVATED_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_ISIM_ACTIVATEISIMAPPLI_REQ;

typedef CAPI2_ISimApi_SendAuthenHttpReq_Req_t *T_INTER_TASK_MSG_ISIM_SENDAUTHENHTTPREQ_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_ISIM_SENDAUTHENHTTPREQ_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_ISIM_SENDAUTHENHTTPREQ_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_ISIM_SENDAUTHENHTTPREQ_REQ_HEADER Param1;
} yPDef_MSG_ISIM_SENDAUTHENHTTPREQ_REQ;

typedef CAPI2_ISimApi_SendAuthenGbaNafReq_Req_t *T_INTER_TASK_MSG_ISIM_SENDAUTHENGBANAFREQ_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_ISIM_SENDAUTHENGBANAFREQ_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_ISIM_SENDAUTHENGBANAFREQ_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_ISIM_SENDAUTHENGBANAFREQ_REQ_HEADER Param1;
} yPDef_MSG_ISIM_SENDAUTHENGBANAFREQ_REQ;

typedef CAPI2_ISimApi_SendAuthenGbaBootReq_Req_t *T_INTER_TASK_MSG_ISIM_SENDAUTHENGBABOOTREQ_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_ISIM_SENDAUTHENGBABOOTREQ_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_ISIM_SENDAUTHENGBABOOTREQ_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_ISIM_SENDAUTHENGBABOOTREQ_REQ_HEADER Param1;
} yPDef_MSG_ISIM_SENDAUTHENGBABOOTREQ_REQ;

typedef CAPI2_PbkApi_GetAlpha_Req_t *T_INTER_TASK_MSG_PBK_GETALPHA_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PBK_GETALPHA_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PBK_GETALPHA_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PBK_GETALPHA_REQ_HEADER Param1;
} yPDef_MSG_PBK_GETALPHA_REQ;

typedef void *T_INTER_TASK_MSG_PBK_GETALPHA_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PBK_GETALPHA_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PBK_GETALPHA_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PBK_GETALPHA_RSP_HEADER Param1;
} yPDef_MSG_PBK_GETALPHA_RSP;

typedef CAPI2_PbkApi_IsEmergencyCallNumber_Req_t *T_INTER_TASK_MSG_PBK_ISEMERGENCYCALLNUMBER_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PBK_ISEMERGENCYCALLNUMBER_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PBK_ISEMERGENCYCALLNUMBER_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PBK_ISEMERGENCYCALLNUMBER_REQ_HEADER Param1;
} yPDef_MSG_PBK_ISEMERGENCYCALLNUMBER_REQ;

typedef Boolean *T_INTER_TASK_MSG_PBK_ISEMERGENCYCALLNUMBER_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PBK_ISEMERGENCYCALLNUMBER_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PBK_ISEMERGENCYCALLNUMBER_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PBK_ISEMERGENCYCALLNUMBER_RSP_HEADER Param1;
} yPDef_MSG_PBK_ISEMERGENCYCALLNUMBER_RSP;

typedef CAPI2_PbkApi_IsPartialEmergencyCallNumber_Req_t *T_INTER_TASK_MSG_PBK_ISPARTIALEMERGENCYCALLNUMBER_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PBK_ISPARTIALEMERGENCYCALLNUMBER_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PBK_ISPARTIALEMERGENCYCALLNUMBER_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PBK_ISPARTIALEMERGENCYCALLNUMBER_REQ_HEADER Param1;
} yPDef_MSG_PBK_ISPARTIALEMERGENCYCALLNUMBER_REQ;

typedef Boolean *T_INTER_TASK_MSG_PBK_ISPARTIALEMERGENCYCALLNUMBER_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PBK_ISPARTIALEMERGENCYCALLNUMBER_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PBK_ISPARTIALEMERGENCYCALLNUMBER_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PBK_ISPARTIALEMERGENCYCALLNUMBER_RSP_HEADER Param1;
} yPDef_MSG_PBK_ISPARTIALEMERGENCYCALLNUMBER_RSP;

typedef CAPI2_PbkApi_SendInfoReq_Req_t *T_INTER_TASK_MSG_PBK_SENDINFOREQ_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PBK_SENDINFOREQ_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PBK_SENDINFOREQ_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PBK_SENDINFOREQ_REQ_HEADER Param1;
} yPDef_MSG_PBK_SENDINFOREQ_REQ;

typedef CAPI2_PbkApi_SendFindAlphaMatchMultipleReq_Req_t *T_INTER_TASK_MSG_SENDFINDALPHAMATCHMULTIPLEREQ_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SENDFINDALPHAMATCHMULTIPLEREQ_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SENDFINDALPHAMATCHMULTIPLEREQ_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SENDFINDALPHAMATCHMULTIPLEREQ_REQ_HEADER Param1;
} yPDef_MSG_SENDFINDALPHAMATCHMULTIPLEREQ_REQ;

typedef CAPI2_PbkApi_SendFindAlphaMatchOneReq_Req_t *T_INTER_TASK_MSG_SENDFINDALPHAMATCHONEREQ_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SENDFINDALPHAMATCHONEREQ_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SENDFINDALPHAMATCHONEREQ_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SENDFINDALPHAMATCHONEREQ_REQ_HEADER Param1;
} yPDef_MSG_SENDFINDALPHAMATCHONEREQ_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PBK_ISREADY_REQ;

typedef Boolean *T_INTER_TASK_MSG_PBK_ISREADY_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PBK_ISREADY_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PBK_ISREADY_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PBK_ISREADY_RSP_HEADER Param1;
} yPDef_MSG_PBK_ISREADY_RSP;

typedef CAPI2_PbkApi_SendReadEntryReq_Req_t *T_INTER_TASK_MSG_SENDREADENTRYREQ_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SENDREADENTRYREQ_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SENDREADENTRYREQ_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SENDREADENTRYREQ_REQ_HEADER Param1;
} yPDef_MSG_SENDREADENTRYREQ_REQ;

typedef CAPI2_PbkApi_SendWriteEntryReq_Req_t *T_INTER_TASK_MSG_PBK_SENDWRITEENTRYREQ_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PBK_SENDWRITEENTRYREQ_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PBK_SENDWRITEENTRYREQ_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PBK_SENDWRITEENTRYREQ_REQ_HEADER Param1;
} yPDef_MSG_PBK_SENDWRITEENTRYREQ_REQ;

typedef CAPI2_PbkApi_SendUpdateEntryReq_Req_t *T_INTER_TASK_MSG_PBK_SENDUPDATEENTRYREQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PBK_SENDUPDATEENTRYREQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PBK_SENDUPDATEENTRYREQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PBK_SENDUPDATEENTRYREQ_HEADER Param1;
} yPDef_MSG_PBK_SENDUPDATEENTRYREQ;

typedef CAPI2_PbkApi_SendIsNumDiallableReq_Req_t *T_INTER_TASK_MSG_PBK_SENDISNUMDIALLABLEREQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PBK_SENDISNUMDIALLABLEREQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PBK_SENDISNUMDIALLABLEREQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PBK_SENDISNUMDIALLABLEREQ_HEADER Param1;
} yPDef_MSG_PBK_SENDISNUMDIALLABLEREQ;

typedef CAPI2_PbkApi_IsNumDiallable_Req_t *T_INTER_TASK_MSG_PBK_ISNUMDIALLABLE_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PBK_ISNUMDIALLABLE_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PBK_ISNUMDIALLABLE_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PBK_ISNUMDIALLABLE_REQ_HEADER Param1;
} yPDef_MSG_PBK_ISNUMDIALLABLE_REQ;

typedef Boolean *T_INTER_TASK_MSG_PBK_ISNUMDIALLABLE_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PBK_ISNUMDIALLABLE_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PBK_ISNUMDIALLABLE_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PBK_ISNUMDIALLABLE_RSP_HEADER Param1;
} yPDef_MSG_PBK_ISNUMDIALLABLE_RSP;

typedef CAPI2_PbkApi_IsNumBarred_Req_t *T_INTER_TASK_MSG_PBK_ISNUMBARRED_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PBK_ISNUMBARRED_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PBK_ISNUMBARRED_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PBK_ISNUMBARRED_REQ_HEADER Param1;
} yPDef_MSG_PBK_ISNUMBARRED_REQ;

typedef Boolean *T_INTER_TASK_MSG_PBK_ISNUMBARRED_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PBK_ISNUMBARRED_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PBK_ISNUMBARRED_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PBK_ISNUMBARRED_RSP_HEADER Param1;
} yPDef_MSG_PBK_ISNUMBARRED_RSP;

typedef CAPI2_PbkApi_IsUssdDiallable_Req_t *T_INTER_TASK_MSG_PBK_ISUSSDDIALLABLE_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PBK_ISUSSDDIALLABLE_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PBK_ISUSSDDIALLABLE_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PBK_ISUSSDDIALLABLE_REQ_HEADER Param1;
} yPDef_MSG_PBK_ISUSSDDIALLABLE_REQ;

typedef Boolean *T_INTER_TASK_MSG_PBK_ISUSSDDIALLABLE_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PBK_ISUSSDDIALLABLE_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PBK_ISUSSDDIALLABLE_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PBK_ISUSSDDIALLABLE_RSP_HEADER Param1;
} yPDef_MSG_PBK_ISUSSDDIALLABLE_RSP;

typedef CAPI2_PdpApi_SetPDPContext_Req_t *T_INTER_TASK_MSG_PDP_SETPDPCONTEXT_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_SETPDPCONTEXT_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_SETPDPCONTEXT_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_SETPDPCONTEXT_REQ_HEADER Param1;
} yPDef_MSG_PDP_SETPDPCONTEXT_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_SETPDPCONTEXT_RSP;

typedef CAPI2_PdpApi_SetSecPDPContext_Req_t *T_INTER_TASK_MSG_PDP_SETSECPDPCONTEXT_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_SETSECPDPCONTEXT_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_SETSECPDPCONTEXT_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_SETSECPDPCONTEXT_REQ_HEADER Param1;
} yPDef_MSG_PDP_SETSECPDPCONTEXT_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_SETSECPDPCONTEXT_RSP;

typedef CAPI2_PdpApi_GetGPRSQoS_Req_t *T_INTER_TASK_MSG_PDP_GETGPRSQOS_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_GETGPRSQOS_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_GETGPRSQOS_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_GETGPRSQOS_REQ_HEADER Param1;
} yPDef_MSG_PDP_GETGPRSQOS_REQ;

typedef Result_t *T_INTER_TASK_MSG_PDP_GETGPRSQOS_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_GETGPRSQOS_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_GETGPRSQOS_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_GETGPRSQOS_RSP_HEADER Param1;
} yPDef_MSG_PDP_GETGPRSQOS_RSP;

typedef CAPI2_PdpApi_SetGPRSQoS_Req_t *T_INTER_TASK_MSG_PDP_SETGPRSQOS_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_SETGPRSQOS_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_SETGPRSQOS_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_SETGPRSQOS_REQ_HEADER Param1;
} yPDef_MSG_PDP_SETGPRSQOS_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_SETGPRSQOS_RSP;

typedef CAPI2_PdpApi_GetGPRSMinQoS_Req_t *T_INTER_TASK_MSG_PDP_GETGPRSMINQOS_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_GETGPRSMINQOS_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_GETGPRSMINQOS_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_GETGPRSMINQOS_REQ_HEADER Param1;
} yPDef_MSG_PDP_GETGPRSMINQOS_REQ;

typedef Result_t *T_INTER_TASK_MSG_PDP_GETGPRSMINQOS_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_GETGPRSMINQOS_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_GETGPRSMINQOS_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_GETGPRSMINQOS_RSP_HEADER Param1;
} yPDef_MSG_PDP_GETGPRSMINQOS_RSP;

typedef CAPI2_PdpApi_SetGPRSMinQoS_Req_t *T_INTER_TASK_MSG_PDP_SETGPRSMINQOS_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_SETGPRSMINQOS_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_SETGPRSMINQOS_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_SETGPRSMINQOS_REQ_HEADER Param1;
} yPDef_MSG_PDP_SETGPRSMINQOS_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_SETGPRSMINQOS_RSP;

typedef CAPI2_NetRegApi_SendCombinedAttachReq_Req_t *T_INTER_TASK_MSG_MS_SENDCOMBINEDATTACHREQ_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_MS_SENDCOMBINEDATTACHREQ_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_MS_SENDCOMBINEDATTACHREQ_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_MS_SENDCOMBINEDATTACHREQ_REQ_HEADER Param1;
} yPDef_MSG_MS_SENDCOMBINEDATTACHREQ_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_MS_SENDCOMBINEDATTACHREQ_RSP;

typedef CAPI2_NetRegApi_SendDetachReq_Req_t *T_INTER_TASK_MSG_MS_SENDDETACHREQ_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_MS_SENDDETACHREQ_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_MS_SENDDETACHREQ_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_MS_SENDDETACHREQ_REQ_HEADER Param1;
} yPDef_MSG_MS_SENDDETACHREQ_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_MS_SENDDETACHREQ_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_MS_GETGPRSATTACHSTATUS_REQ;

typedef AttachState_t *T_INTER_TASK_MSG_MS_GETGPRSATTACHSTATUS_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_MS_GETGPRSATTACHSTATUS_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_MS_GETGPRSATTACHSTATUS_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_MS_GETGPRSATTACHSTATUS_RSP_HEADER Param1;
} yPDef_MSG_MS_GETGPRSATTACHSTATUS_RSP;

typedef CAPI2_PdpApi_IsSecondaryPdpDefined_Req_t *T_INTER_TASK_MSG_PDP_ISSECONDARYPDPDEFINED_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_ISSECONDARYPDPDEFINED_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_ISSECONDARYPDPDEFINED_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_ISSECONDARYPDPDEFINED_REQ_HEADER Param1;
} yPDef_MSG_PDP_ISSECONDARYPDPDEFINED_REQ;

typedef Boolean *T_INTER_TASK_MSG_PDP_ISSECONDARYPDPDEFINED_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_ISSECONDARYPDPDEFINED_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_ISSECONDARYPDPDEFINED_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_ISSECONDARYPDPDEFINED_RSP_HEADER Param1;
} yPDef_MSG_PDP_ISSECONDARYPDPDEFINED_RSP;

typedef CAPI2_PchExApi_SendPDPActivateReq_Req_t *T_INTER_TASK_MSG_PCHEX_SENDPDPACTIVATEREQ_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PCHEX_SENDPDPACTIVATEREQ_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PCHEX_SENDPDPACTIVATEREQ_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PCHEX_SENDPDPACTIVATEREQ_REQ_HEADER Param1;
} yPDef_MSG_PCHEX_SENDPDPACTIVATEREQ_REQ;

typedef CAPI2_PchExApi_SendPDPModifyReq_Req_t *T_INTER_TASK_MSG_PCHEX_SENDPDPMODIFYREQ_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PCHEX_SENDPDPMODIFYREQ_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PCHEX_SENDPDPMODIFYREQ_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PCHEX_SENDPDPMODIFYREQ_REQ_HEADER Param1;
} yPDef_MSG_PCHEX_SENDPDPMODIFYREQ_REQ;

typedef CAPI2_PchExApi_SendPDPDeactivateReq_Req_t *T_INTER_TASK_MSG_PCHEX_SENDPDPDEACTIVATEREQ_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PCHEX_SENDPDPDEACTIVATEREQ_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PCHEX_SENDPDPDEACTIVATEREQ_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PCHEX_SENDPDPDEACTIVATEREQ_REQ_HEADER Param1;
} yPDef_MSG_PCHEX_SENDPDPDEACTIVATEREQ_REQ;

typedef CAPI2_PchExApi_SendPDPActivateSecReq_Req_t *T_INTER_TASK_MSG_PCHEX_SENDPDPACTIVATESECREQ_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PCHEX_SENDPDPACTIVATESECREQ_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PCHEX_SENDPDPACTIVATESECREQ_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PCHEX_SENDPDPACTIVATESECREQ_REQ_HEADER Param1;
} yPDef_MSG_PCHEX_SENDPDPACTIVATESECREQ_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_GETGPRSACTIVATESTATUS_REQ;

typedef Result_t *T_INTER_TASK_MSG_PDP_GETGPRSACTIVATESTATUS_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_GETGPRSACTIVATESTATUS_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_GETGPRSACTIVATESTATUS_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_GETGPRSACTIVATESTATUS_RSP_HEADER Param1;
} yPDef_MSG_PDP_GETGPRSACTIVATESTATUS_RSP;

typedef CAPI2_NetRegApi_SetMSClass_Req_t *T_INTER_TASK_MSG_PDP_SETMSCLASS_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_SETMSCLASS_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_SETMSCLASS_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_SETMSCLASS_REQ_HEADER Param1;
} yPDef_MSG_PDP_SETMSCLASS_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_SETMSCLASS_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_GETMSCLASS_REQ;

typedef MSClass_t *T_INTER_TASK_MSG_PDP_GETMSCLASS_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_GETMSCLASS_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_GETMSCLASS_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_GETMSCLASS_RSP_HEADER Param1;
} yPDef_MSG_PDP_GETMSCLASS_RSP;

typedef CAPI2_PdpApi_GetUMTSTft_Req_t *T_INTER_TASK_MSG_PDP_GETUMTSTFT_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_GETUMTSTFT_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_GETUMTSTFT_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_GETUMTSTFT_REQ_HEADER Param1;
} yPDef_MSG_PDP_GETUMTSTFT_REQ;

typedef Result_t *T_INTER_TASK_MSG_PDP_GETUMTSTFT_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_GETUMTSTFT_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_GETUMTSTFT_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_GETUMTSTFT_RSP_HEADER Param1;
} yPDef_MSG_PDP_GETUMTSTFT_RSP;

typedef CAPI2_PdpApi_SetUMTSTft_Req_t *T_INTER_TASK_MSG_PDP_SETUMTSTFT_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_SETUMTSTFT_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_SETUMTSTFT_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_SETUMTSTFT_REQ_HEADER Param1;
} yPDef_MSG_PDP_SETUMTSTFT_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_SETUMTSTFT_RSP;

typedef CAPI2_PdpApi_DeleteUMTSTft_Req_t *T_INTER_TASK_MSG_PDP_DELETEUMTSTFT_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_DELETEUMTSTFT_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_DELETEUMTSTFT_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_DELETEUMTSTFT_REQ_HEADER Param1;
} yPDef_MSG_PDP_DELETEUMTSTFT_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_DELETEUMTSTFT_RSP;

typedef CAPI2_PdpApi_DeactivateSNDCPConnection_Req_t *T_INTER_TASK_MSG_PDP_DEACTIVATESNDCPCONNECTION_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_DEACTIVATESNDCPCONNECTION_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_DEACTIVATESNDCPCONNECTION_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_DEACTIVATESNDCPCONNECTION_REQ_HEADER Param1;
} yPDef_MSG_PDP_DEACTIVATESNDCPCONNECTION_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_DEACTIVATESNDCPCONNECTION_RSP;

typedef CAPI2_PdpApi_GetR99UMTSMinQoS_Req_t *T_INTER_TASK_MSG_PDP_GETR99UMTSMINQOS_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_GETR99UMTSMINQOS_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_GETR99UMTSMINQOS_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_GETR99UMTSMINQOS_REQ_HEADER Param1;
} yPDef_MSG_PDP_GETR99UMTSMINQOS_REQ;

typedef Result_t *T_INTER_TASK_MSG_PDP_GETR99UMTSMINQOS_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_GETR99UMTSMINQOS_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_GETR99UMTSMINQOS_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_GETR99UMTSMINQOS_RSP_HEADER Param1;
} yPDef_MSG_PDP_GETR99UMTSMINQOS_RSP;

typedef CAPI2_PdpApi_GetR99UMTSQoS_Req_t *T_INTER_TASK_MSG_PDP_GETR99UMTSQOS_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_GETR99UMTSQOS_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_GETR99UMTSQOS_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_GETR99UMTSQOS_REQ_HEADER Param1;
} yPDef_MSG_PDP_GETR99UMTSQOS_REQ;

typedef Result_t *T_INTER_TASK_MSG_PDP_GETR99UMTSQOS_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_GETR99UMTSQOS_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_GETR99UMTSQOS_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_GETR99UMTSQOS_RSP_HEADER Param1;
} yPDef_MSG_PDP_GETR99UMTSQOS_RSP;

typedef CAPI2_PdpApi_GetUMTSMinQoS_Req_t *T_INTER_TASK_MSG_PDP_GETUMTSMINQOS_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_GETUMTSMINQOS_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_GETUMTSMINQOS_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_GETUMTSMINQOS_REQ_HEADER Param1;
} yPDef_MSG_PDP_GETUMTSMINQOS_REQ;

typedef Result_t *T_INTER_TASK_MSG_PDP_GETUMTSMINQOS_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_GETUMTSMINQOS_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_GETUMTSMINQOS_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_GETUMTSMINQOS_RSP_HEADER Param1;
} yPDef_MSG_PDP_GETUMTSMINQOS_RSP;

typedef CAPI2_PdpApi_GetUMTSQoS_Req_t *T_INTER_TASK_MSG_PDP_GETUMTSQOS_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_GETUMTSQOS_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_GETUMTSQOS_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_GETUMTSQOS_REQ_HEADER Param1;
} yPDef_MSG_PDP_GETUMTSQOS_REQ;

typedef Result_t *T_INTER_TASK_MSG_PDP_GETUMTSQOS_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_GETUMTSQOS_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_GETUMTSQOS_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_GETUMTSQOS_RSP_HEADER Param1;
} yPDef_MSG_PDP_GETUMTSQOS_RSP;

typedef CAPI2_PdpApi_GetNegQoS_Req_t *T_INTER_TASK_MSG_PDP_GETNEGQOS_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_GETNEGQOS_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_GETNEGQOS_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_GETNEGQOS_REQ_HEADER Param1;
} yPDef_MSG_PDP_GETNEGQOS_REQ;

typedef Result_t *T_INTER_TASK_MSG_PDP_GETNEGQOS_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_GETNEGQOS_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_GETNEGQOS_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_GETNEGQOS_RSP_HEADER Param1;
} yPDef_MSG_PDP_GETNEGQOS_RSP;

typedef CAPI2_PdpApi_SetR99UMTSMinQoS_Req_t *T_INTER_TASK_MSG_PDP_SETR99UMTSMINQOS_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_SETR99UMTSMINQOS_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_SETR99UMTSMINQOS_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_SETR99UMTSMINQOS_REQ_HEADER Param1;
} yPDef_MSG_PDP_SETR99UMTSMINQOS_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_SETR99UMTSMINQOS_RSP;

typedef CAPI2_PdpApi_SetR99UMTSQoS_Req_t *T_INTER_TASK_MSG_PDP_SETR99UMTSQOS_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_SETR99UMTSQOS_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_SETR99UMTSQOS_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_SETR99UMTSQOS_REQ_HEADER Param1;
} yPDef_MSG_PDP_SETR99UMTSQOS_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_SETR99UMTSQOS_RSP;

typedef CAPI2_PdpApi_SetUMTSMinQoS_Req_t *T_INTER_TASK_MSG_PDP_SETUMTSMINQOS_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_SETUMTSMINQOS_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_SETUMTSMINQOS_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_SETUMTSMINQOS_REQ_HEADER Param1;
} yPDef_MSG_PDP_SETUMTSMINQOS_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_SETUMTSMINQOS_RSP;

typedef CAPI2_PdpApi_SetUMTSQoS_Req_t *T_INTER_TASK_MSG_PDP_SETUMTSQOS_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_SETUMTSQOS_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_SETUMTSQOS_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_SETUMTSQOS_REQ_HEADER Param1;
} yPDef_MSG_PDP_SETUMTSQOS_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_SETUMTSQOS_RSP;

typedef CAPI2_PdpApi_GetNegotiatedParms_Req_t *T_INTER_TASK_MSG_PDP_GETNEGOTIATEDPARMS_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_GETNEGOTIATEDPARMS_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_GETNEGOTIATEDPARMS_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_GETNEGOTIATEDPARMS_REQ_HEADER Param1;
} yPDef_MSG_PDP_GETNEGOTIATEDPARMS_REQ;

typedef Result_t *T_INTER_TASK_MSG_PDP_GETNEGOTIATEDPARMS_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_GETNEGOTIATEDPARMS_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_GETNEGOTIATEDPARMS_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_GETNEGOTIATEDPARMS_RSP_HEADER Param1;
} yPDef_MSG_PDP_GETNEGOTIATEDPARMS_RSP;

typedef CAPI2_MS_IsGprsCallActive_Req_t *T_INTER_TASK_MSG_MS_ISGPRSCALLACTIVE_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_MS_ISGPRSCALLACTIVE_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_MS_ISGPRSCALLACTIVE_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_MS_ISGPRSCALLACTIVE_REQ_HEADER Param1;
} yPDef_MSG_MS_ISGPRSCALLACTIVE_REQ;

typedef Boolean *T_INTER_TASK_MSG_MS_ISGPRSCALLACTIVE_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_MS_ISGPRSCALLACTIVE_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_MS_ISGPRSCALLACTIVE_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_MS_ISGPRSCALLACTIVE_RSP_HEADER Param1;
} yPDef_MSG_MS_ISGPRSCALLACTIVE_RSP;

typedef CAPI2_MS_SetChanGprsCallActive_Req_t *T_INTER_TASK_MSG_MS_SETCHANGPRSCALLACTIVE_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_MS_SETCHANGPRSCALLACTIVE_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_MS_SETCHANGPRSCALLACTIVE_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_MS_SETCHANGPRSCALLACTIVE_REQ_HEADER Param1;
} yPDef_MSG_MS_SETCHANGPRSCALLACTIVE_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_MS_SETCHANGPRSCALLACTIVE_RSP;

typedef CAPI2_MS_SetCidForGprsActiveChan_Req_t *T_INTER_TASK_MSG_MS_SETCIDFORGPRSACTIVECHAN_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_MS_SETCIDFORGPRSACTIVECHAN_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_MS_SETCIDFORGPRSACTIVECHAN_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_MS_SETCIDFORGPRSACTIVECHAN_REQ_HEADER Param1;
} yPDef_MSG_MS_SETCIDFORGPRSACTIVECHAN_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_MS_SETCIDFORGPRSACTIVECHAN_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_GETPPPMODEMCID_REQ;

typedef PCHCid_t *T_INTER_TASK_MSG_PDP_GETPPPMODEMCID_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_GETPPPMODEMCID_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_GETPPPMODEMCID_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_GETPPPMODEMCID_RSP_HEADER Param1;
} yPDef_MSG_PDP_GETPPPMODEMCID_RSP;

typedef CAPI2_MS_GetGprsActiveChanFromCid_Req_t *T_INTER_TASK_MSG_MS_GETGPRSACTIVECHANFROMCID_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_MS_GETGPRSACTIVECHANFROMCID_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_MS_GETGPRSACTIVECHANFROMCID_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_MS_GETGPRSACTIVECHANFROMCID_REQ_HEADER Param1;
} yPDef_MSG_MS_GETGPRSACTIVECHANFROMCID_REQ;

typedef UInt8 *T_INTER_TASK_MSG_MS_GETGPRSACTIVECHANFROMCID_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_MS_GETGPRSACTIVECHANFROMCID_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_MS_GETGPRSACTIVECHANFROMCID_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_MS_GETGPRSACTIVECHANFROMCID_RSP_HEADER Param1;
} yPDef_MSG_MS_GETGPRSACTIVECHANFROMCID_RSP;

typedef CAPI2_MS_GetCidFromGprsActiveChan_Req_t *T_INTER_TASK_MSG_MS_GETCIDFROMGPRSACTIVECHAN_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_MS_GETCIDFROMGPRSACTIVECHAN_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_MS_GETCIDFROMGPRSACTIVECHAN_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_MS_GETCIDFROMGPRSACTIVECHAN_REQ_HEADER Param1;
} yPDef_MSG_MS_GETCIDFROMGPRSACTIVECHAN_REQ;

typedef UInt8 *T_INTER_TASK_MSG_MS_GETCIDFROMGPRSACTIVECHAN_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_MS_GETCIDFROMGPRSACTIVECHAN_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_MS_GETCIDFROMGPRSACTIVECHAN_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_MS_GETCIDFROMGPRSACTIVECHAN_RSP_HEADER Param1;
} yPDef_MSG_MS_GETCIDFROMGPRSACTIVECHAN_RSP;

typedef CAPI2_PdpApi_GetPDPAddress_Req_t *T_INTER_TASK_MSG_PDP_GETPDPADDRESS_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_GETPDPADDRESS_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_GETPDPADDRESS_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_GETPDPADDRESS_REQ_HEADER Param1;
} yPDef_MSG_PDP_GETPDPADDRESS_REQ;

typedef Result_t *T_INTER_TASK_MSG_PDP_GETPDPADDRESS_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_GETPDPADDRESS_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_GETPDPADDRESS_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_GETPDPADDRESS_RSP_HEADER Param1;
} yPDef_MSG_PDP_GETPDPADDRESS_RSP;

typedef CAPI2_PdpApi_SendTBFData_Req_t *T_INTER_TASK_MSG_PDP_SENDTBFDATA_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_SENDTBFDATA_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_SENDTBFDATA_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_SENDTBFDATA_REQ_HEADER Param1;
} yPDef_MSG_PDP_SENDTBFDATA_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_SENDTBFDATA_RSP;

typedef CAPI2_PdpApi_TftAddFilter_Req_t *T_INTER_TASK_MSG_PDP_TFTADDFILTER_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_TFTADDFILTER_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_TFTADDFILTER_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_TFTADDFILTER_REQ_HEADER Param1;
} yPDef_MSG_PDP_TFTADDFILTER_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_TFTADDFILTER_RSP;

typedef CAPI2_PdpApi_SetPCHContextState_Req_t *T_INTER_TASK_MSG_PDP_SETPCHCONTEXTSTATE_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_SETPCHCONTEXTSTATE_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_SETPCHCONTEXTSTATE_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_SETPCHCONTEXTSTATE_REQ_HEADER Param1;
} yPDef_MSG_PDP_SETPCHCONTEXTSTATE_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_SETPCHCONTEXTSTATE_RSP;

typedef CAPI2_PdpApi_SetDefaultPDPContext_Req_t *T_INTER_TASK_MSG_PDP_SETDEFAULTPDPCONTEXT_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_SETDEFAULTPDPCONTEXT_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_SETDEFAULTPDPCONTEXT_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_SETDEFAULTPDPCONTEXT_REQ_HEADER Param1;
} yPDef_MSG_PDP_SETDEFAULTPDPCONTEXT_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_SETDEFAULTPDPCONTEXT_RSP;

typedef CAPI2_PCHEx_GetDecodedProtConfig_Req_t *T_INTER_TASK_MSG_PCHEX_READDECODEDPROTCONFIG_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PCHEX_READDECODEDPROTCONFIG_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PCHEX_READDECODEDPROTCONFIG_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PCHEX_READDECODEDPROTCONFIG_REQ_HEADER Param1;
} yPDef_MSG_PCHEX_READDECODEDPROTCONFIG_REQ;

typedef Result_t *T_INTER_TASK_MSG_PCHEX_READDECODEDPROTCONFIG_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PCHEX_READDECODEDPROTCONFIG_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PCHEX_READDECODEDPROTCONFIG_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PCHEX_READDECODEDPROTCONFIG_RSP_HEADER Param1;
} yPDef_MSG_PCHEX_READDECODEDPROTCONFIG_RSP;

typedef CAPI2_PchExApi_BuildIpConfigOptions_Req_t *T_INTER_TASK_MSG_PCHEX_BUILDPROTCONFIGOPTIONS_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PCHEX_BUILDPROTCONFIGOPTIONS_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PCHEX_BUILDPROTCONFIGOPTIONS_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PCHEX_BUILDPROTCONFIGOPTIONS_REQ_HEADER Param1;
} yPDef_MSG_PCHEX_BUILDPROTCONFIGOPTIONS_REQ;

typedef Boolean *T_INTER_TASK_MSG_PCHEX_BUILDPROTCONFIGOPTIONS_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PCHEX_BUILDPROTCONFIGOPTIONS_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PCHEX_BUILDPROTCONFIGOPTIONS_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PCHEX_BUILDPROTCONFIGOPTIONS_RSP_HEADER Param1;
} yPDef_MSG_PCHEX_BUILDPROTCONFIGOPTIONS_RSP;

typedef CAPI2_PCHEx_BuildIpConfigOptions2_Req_t *T_INTER_TASK_MSG_PCHEX_BUILDPROTCONFIGOPTIONS2_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PCHEX_BUILDPROTCONFIGOPTIONS2_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PCHEX_BUILDPROTCONFIGOPTIONS2_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PCHEX_BUILDPROTCONFIGOPTIONS2_REQ_HEADER Param1;
} yPDef_MSG_PCHEX_BUILDPROTCONFIGOPTIONS2_REQ;

typedef void *T_INTER_TASK_MSG_PCHEX_BUILDPROTCONFIGOPTIONS2_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PCHEX_BUILDPROTCONFIGOPTIONS2_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PCHEX_BUILDPROTCONFIGOPTIONS2_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PCHEX_BUILDPROTCONFIGOPTIONS2_RSP_HEADER Param1;
} yPDef_MSG_PCHEX_BUILDPROTCONFIGOPTIONS2_RSP;

typedef CAPI2_PchExApi_BuildIpConfigOptionsWithChapAuthType_Req_t *T_INTER_TASK_MSG_PCHEX_BUILDPROTCONFIGOPTION_CHAP_TYPE_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PCHEX_BUILDPROTCONFIGOPTION_CHAP_TYPE_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PCHEX_BUILDPROTCONFIGOPTION_CHAP_TYPE_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PCHEX_BUILDPROTCONFIGOPTION_CHAP_TYPE_REQ_HEADER Param1;
} yPDef_MSG_PCHEX_BUILDPROTCONFIGOPTION_CHAP_TYPE_REQ;

typedef void *T_INTER_TASK_MSG_PCHEX_BUILDPROTCONFIGOPTION_CHAP_TYPE_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PCHEX_BUILDPROTCONFIGOPTION_CHAP_TYPE_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PCHEX_BUILDPROTCONFIGOPTION_CHAP_TYPE_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PCHEX_BUILDPROTCONFIGOPTION_CHAP_TYPE_RSP_HEADER Param1;
} yPDef_MSG_PCHEX_BUILDPROTCONFIGOPTION_CHAP_TYPE_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_GET_DEFAULT_QOS_REQ;

typedef void *T_INTER_TASK_MSG_PDP_GET_DEFAULT_QOS_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_GET_DEFAULT_QOS_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_GET_DEFAULT_QOS_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_GET_DEFAULT_QOS_RSP_HEADER Param1;
} yPDef_MSG_PDP_GET_DEFAULT_QOS_RSP;

typedef CAPI2_PdpApi_IsPDPContextActive_Req_t *T_INTER_TASK_MSG_PDP_ISCONTEXT_ACTIVE_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_ISCONTEXT_ACTIVE_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_ISCONTEXT_ACTIVE_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_ISCONTEXT_ACTIVE_REQ_HEADER Param1;
} yPDef_MSG_PDP_ISCONTEXT_ACTIVE_REQ;

typedef Boolean *T_INTER_TASK_MSG_PDP_ISCONTEXT_ACTIVE_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_ISCONTEXT_ACTIVE_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_ISCONTEXT_ACTIVE_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_ISCONTEXT_ACTIVE_RSP_HEADER Param1;
} yPDef_MSG_PDP_ISCONTEXT_ACTIVE_RSP;

typedef CAPI2_PdpApi_ActivateSNDCPConnection_Req_t *T_INTER_TASK_MSG_PDP_ACTIVATESNDCPCONNECTION_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_ACTIVATESNDCPCONNECTION_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_ACTIVATESNDCPCONNECTION_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_ACTIVATESNDCPCONNECTION_REQ_HEADER Param1;
} yPDef_MSG_PDP_ACTIVATESNDCPCONNECTION_REQ;

typedef CAPI2_PdpApi_GetPDPContext_Req_t *T_INTER_TASK_MSG_PDP_GETPDPCONTEXT_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_GETPDPCONTEXT_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_GETPDPCONTEXT_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_GETPDPCONTEXT_REQ_HEADER Param1;
} yPDef_MSG_PDP_GETPDPCONTEXT_REQ;

typedef PDPDefaultContext_t *T_INTER_TASK_MSG_PDP_GETPDPCONTEXT_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_GETPDPCONTEXT_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_GETPDPCONTEXT_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_GETPDPCONTEXT_RSP_HEADER Param1;
} yPDef_MSG_PDP_GETPDPCONTEXT_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_GETPDPCONTEXT_CID_LIST_REQ;

typedef Result_t *T_INTER_TASK_MSG_PDP_GETPDPCONTEXT_CID_LIST_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_GETPDPCONTEXT_CID_LIST_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_GETPDPCONTEXT_CID_LIST_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_GETPDPCONTEXT_CID_LIST_RSP_HEADER Param1;
} yPDef_MSG_PDP_GETPDPCONTEXT_CID_LIST_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_UTIL_GET_IMEI_STR_REQ;

typedef void *T_INTER_TASK_MSG_UTIL_GET_IMEI_STR_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_UTIL_GET_IMEI_STR_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_UTIL_GET_IMEI_STR_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_UTIL_GET_IMEI_STR_RSP_HEADER Param1;
} yPDef_MSG_UTIL_GET_IMEI_STR_RSP;

typedef CAPI2_SYS_GetBootLoaderVersion_Req_t *T_INTER_TASK_MSG_SYSPARAM_BOOTLOADER_VER_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SYSPARAM_BOOTLOADER_VER_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SYSPARAM_BOOTLOADER_VER_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SYSPARAM_BOOTLOADER_VER_REQ_HEADER Param1;
} yPDef_MSG_SYSPARAM_BOOTLOADER_VER_REQ;

typedef void *T_INTER_TASK_MSG_SYSPARAM_BOOTLOADER_VER_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SYSPARAM_BOOTLOADER_VER_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SYSPARAM_BOOTLOADER_VER_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SYSPARAM_BOOTLOADER_VER_RSP_HEADER Param1;
} yPDef_MSG_SYSPARAM_BOOTLOADER_VER_RSP;

typedef CAPI2_SYS_GetDSFVersion_Req_t *T_INTER_TASK_MSG_SYSPARAM_DSF_VER_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SYSPARAM_DSF_VER_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SYSPARAM_DSF_VER_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SYSPARAM_DSF_VER_REQ_HEADER Param1;
} yPDef_MSG_SYSPARAM_DSF_VER_REQ;

typedef void *T_INTER_TASK_MSG_SYSPARAM_DSF_VER_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SYSPARAM_DSF_VER_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SYSPARAM_DSF_VER_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SYSPARAM_DSF_VER_RSP_HEADER Param1;
} yPDef_MSG_SYSPARAM_DSF_VER_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_USIM_UST_DATA_REQ;

typedef UInt8 *T_INTER_TASK_MSG_USIM_UST_DATA_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_USIM_UST_DATA_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_USIM_UST_DATA_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_USIM_UST_DATA_RSP_HEADER Param1;
} yPDef_MSG_USIM_UST_DATA_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PATCH_GET_REVISION_REQ;

typedef UInt8 *T_INTER_TASK_MSG_PATCH_GET_REVISION_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PATCH_GET_REVISION_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PATCH_GET_REVISION_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PATCH_GET_REVISION_RSP_HEADER Param1;
} yPDef_MSG_PATCH_GET_REVISION_RSP;

typedef CAPI2_SS_SendCallForwardReq_Req_t *T_INTER_TASK_MSG_SS_SENDCALLFORWARDREQ_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SS_SENDCALLFORWARDREQ_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SS_SENDCALLFORWARDREQ_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SS_SENDCALLFORWARDREQ_REQ_HEADER Param1;
} yPDef_MSG_SS_SENDCALLFORWARDREQ_REQ;

typedef CAPI2_SS_QueryCallForwardStatus_Req_t *T_INTER_TASK_MSG_SS_QUERYCALLFORWARDSTATUS_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SS_QUERYCALLFORWARDSTATUS_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SS_QUERYCALLFORWARDSTATUS_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SS_QUERYCALLFORWARDSTATUS_REQ_HEADER Param1;
} yPDef_MSG_SS_QUERYCALLFORWARDSTATUS_REQ;

typedef CAPI2_SS_SendCallBarringReq_Req_t *T_INTER_TASK_MSG_SS_SENDCALLBARRINGREQ_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SS_SENDCALLBARRINGREQ_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SS_SENDCALLBARRINGREQ_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SS_SENDCALLBARRINGREQ_REQ_HEADER Param1;
} yPDef_MSG_SS_SENDCALLBARRINGREQ_REQ;

typedef CAPI2_SS_QueryCallBarringStatus_Req_t *T_INTER_TASK_MSG_SS_QUERYCALLBARRINGSTATUS_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SS_QUERYCALLBARRINGSTATUS_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SS_QUERYCALLBARRINGSTATUS_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SS_QUERYCALLBARRINGSTATUS_REQ_HEADER Param1;
} yPDef_MSG_SS_QUERYCALLBARRINGSTATUS_REQ;

typedef CAPI2_SS_SendCallBarringPWDChangeReq_Req_t *T_INTER_TASK_MSG_SS_SENDCALLBARRINGPWDCHANGEREQ_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SS_SENDCALLBARRINGPWDCHANGEREQ_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SS_SENDCALLBARRINGPWDCHANGEREQ_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SS_SENDCALLBARRINGPWDCHANGEREQ_REQ_HEADER Param1;
} yPDef_MSG_SS_SENDCALLBARRINGPWDCHANGEREQ_REQ;

typedef CAPI2_SS_SendCallWaitingReq_Req_t *T_INTER_TASK_MSG_SS_SENDCALLWAITINGREQ_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SS_SENDCALLWAITINGREQ_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SS_SENDCALLWAITINGREQ_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SS_SENDCALLWAITINGREQ_REQ_HEADER Param1;
} yPDef_MSG_SS_SENDCALLWAITINGREQ_REQ;

typedef CAPI2_SS_QueryCallWaitingStatus_Req_t *T_INTER_TASK_MSG_SS_QUERYCALLWAITINGSTATUS_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SS_QUERYCALLWAITINGSTATUS_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SS_QUERYCALLWAITINGSTATUS_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SS_QUERYCALLWAITINGSTATUS_REQ_HEADER Param1;
} yPDef_MSG_SS_QUERYCALLWAITINGSTATUS_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SS_QUERYCALLINGLINEIDSTATUS_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SS_QUERYCONNECTEDLINEIDSTATUS_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SS_QUERYCALLINGLINERESTRICTIONSTATUS_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SS_QUERYCONNECTEDLINERESTRICTIONSTATUS_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SS_QUERYCALLINGNAMEPRESENTSTATUS_REQ;

typedef CAPI2_SS_SetCallingLineIDStatus_Req_t *T_INTER_TASK_MSG_SS_SETCALLINGLINEIDSTATUS_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SS_SETCALLINGLINEIDSTATUS_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SS_SETCALLINGLINEIDSTATUS_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SS_SETCALLINGLINEIDSTATUS_REQ_HEADER Param1;
} yPDef_MSG_SS_SETCALLINGLINEIDSTATUS_REQ;

typedef CAPI2_SS_SetCallingLineRestrictionStatus_Req_t *T_INTER_TASK_MSG_SS_SETCALLINGLINERESTRICTIONSTATUS_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SS_SETCALLINGLINERESTRICTIONSTATUS_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SS_SETCALLINGLINERESTRICTIONSTATUS_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SS_SETCALLINGLINERESTRICTIONSTATUS_REQ_HEADER Param1;
} yPDef_MSG_SS_SETCALLINGLINERESTRICTIONSTATUS_REQ;

typedef CAPI2_SS_SetConnectedLineIDStatus_Req_t *T_INTER_TASK_MSG_SS_SETCONNECTEDLINEIDSTATUS_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SS_SETCONNECTEDLINEIDSTATUS_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SS_SETCONNECTEDLINEIDSTATUS_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SS_SETCONNECTEDLINEIDSTATUS_REQ_HEADER Param1;
} yPDef_MSG_SS_SETCONNECTEDLINEIDSTATUS_REQ;

typedef CAPI2_SS_SetConnectedLineRestrictionStatus_Req_t *T_INTER_TASK_MSG_SS_SETCONNECTEDLINERESTRICTIONSTATUS_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SS_SETCONNECTEDLINERESTRICTIONSTATUS_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SS_SETCONNECTEDLINERESTRICTIONSTATUS_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SS_SETCONNECTEDLINERESTRICTIONSTATUS_REQ_HEADER Param1;
} yPDef_MSG_SS_SETCONNECTEDLINERESTRICTIONSTATUS_REQ;

typedef CAPI2_SS_SendUSSDConnectReq_Req_t *T_INTER_TASK_MSG_SS_SENDUSSDCONNECTREQ_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SS_SENDUSSDCONNECTREQ_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SS_SENDUSSDCONNECTREQ_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SS_SENDUSSDCONNECTREQ_REQ_HEADER Param1;
} yPDef_MSG_SS_SENDUSSDCONNECTREQ_REQ;

typedef CAPI2_SS_SendUSSDData_Req_t *T_INTER_TASK_MSG_SS_SENDUSSDDATA_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SS_SENDUSSDDATA_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SS_SENDUSSDDATA_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SS_SENDUSSDDATA_REQ_HEADER Param1;
} yPDef_MSG_SS_SENDUSSDDATA_REQ;

typedef CAPI2_SsApi_DialStrSrvReq_Req_t *T_INTER_TASK_MSG_SSAPI_DIALSTRSRVREQ_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SSAPI_DIALSTRSRVREQ_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SSAPI_DIALSTRSRVREQ_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SSAPI_DIALSTRSRVREQ_REQ_HEADER Param1;
} yPDef_MSG_SSAPI_DIALSTRSRVREQ_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SSAPI_DIALSTRSRVREQ_RSP;

typedef CAPI2_SS_EndUSSDConnectReq_Req_t *T_INTER_TASK_MSG_SS_ENDUSSDCONNECTREQ_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SS_ENDUSSDCONNECTREQ_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SS_ENDUSSDCONNECTREQ_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SS_ENDUSSDCONNECTREQ_REQ_HEADER Param1;
} yPDef_MSG_SS_ENDUSSDCONNECTREQ_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SS_ENDUSSDCONNECTREQ_RSP;

typedef CAPI2_SsApi_SsSrvReq_Req_t *T_INTER_TASK_MSG_SSAPI_SSSRVREQ_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SSAPI_SSSRVREQ_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SSAPI_SSSRVREQ_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SSAPI_SSSRVREQ_REQ_HEADER Param1;
} yPDef_MSG_SSAPI_SSSRVREQ_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SSAPI_SSSRVREQ_RSP;

typedef CAPI2_SsApi_UssdSrvReq_Req_t *T_INTER_TASK_MSG_SSAPI_USSDSRVREQ_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SSAPI_USSDSRVREQ_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SSAPI_USSDSRVREQ_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SSAPI_USSDSRVREQ_REQ_HEADER Param1;
} yPDef_MSG_SSAPI_USSDSRVREQ_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SSAPI_USSDSRVREQ_RSP;

typedef CAPI2_SsApi_UssdDataReq_Req_t *T_INTER_TASK_MSG_SSAPI_USSDDATAREQ_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SSAPI_USSDDATAREQ_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SSAPI_USSDDATAREQ_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SSAPI_USSDDATAREQ_REQ_HEADER Param1;
} yPDef_MSG_SSAPI_USSDDATAREQ_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SSAPI_USSDDATAREQ_RSP;

typedef CAPI2_SsApi_SsReleaseReq_Req_t *T_INTER_TASK_MSG_SSAPI_SSRELEASEREQ_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SSAPI_SSRELEASEREQ_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SSAPI_SSRELEASEREQ_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SSAPI_SSRELEASEREQ_REQ_HEADER Param1;
} yPDef_MSG_SSAPI_SSRELEASEREQ_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SSAPI_SSRELEASEREQ_RSP;

typedef CAPI2_SsApi_DataReq_Req_t *T_INTER_TASK_MSG_SSAPI_DATAREQ_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SSAPI_DATAREQ_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SSAPI_DATAREQ_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SSAPI_DATAREQ_REQ_HEADER Param1;
} yPDef_MSG_SSAPI_DATAREQ_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SSAPI_DATAREQ_RSP;

typedef CAPI2_SS_SsApiReqDispatcher_Req_t *T_INTER_TASK_MSG_SSAPI_DISPATCH_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SSAPI_DISPATCH_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SSAPI_DISPATCH_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SSAPI_DISPATCH_REQ_HEADER Param1;
} yPDef_MSG_SSAPI_DISPATCH_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SSAPI_DISPATCH_RSP;

typedef CAPI2_SS_GetStr_Req_t *T_INTER_TASK_MSG_SS_GET_STR_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SS_GET_STR_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SS_GET_STR_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SS_GET_STR_REQ_HEADER Param1;
} yPDef_MSG_SS_GET_STR_REQ;

typedef UInt8 *T_INTER_TASK_MSG_SS_GET_STR_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SS_GET_STR_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SS_GET_STR_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SS_GET_STR_RSP_HEADER Param1;
} yPDef_MSG_SS_GET_STR_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SS_SETCLIENTID_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SS_SETCLIENTID_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SS_GETCLIENTID_REQ;

typedef UInt8 *T_INTER_TASK_MSG_SS_GETCLIENTID_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SS_GETCLIENTID_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SS_GETCLIENTID_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SS_GETCLIENTID_RSP_HEADER Param1;
} yPDef_MSG_SS_GETCLIENTID_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SS_RESETCLIENTID_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SS_RESETCLIENTID_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_GRP_CAPI2_SS_END;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SATK_GETCACHEDROOTMENUPTR_REQ;

typedef SetupMenu_t *T_INTER_TASK_MSG_SATK_GETCACHEDROOTMENUPTR_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SATK_GETCACHEDROOTMENUPTR_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SATK_GETCACHEDROOTMENUPTR_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SATK_GETCACHEDROOTMENUPTR_RSP_HEADER Param1;
} yPDef_MSG_SATK_GETCACHEDROOTMENUPTR_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SATK_SENDUSERACTIVITYEVENT_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SATK_SENDUSERACTIVITYEVENT_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SATK_SENDIDLESCREENAVAIEVENT_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SATK_SENDIDLESCREENAVAIEVENT_RSP;

typedef CAPI2_SatkApi_SendLangSelectEvent_Req_t *T_INTER_TASK_MSG_SATK_SENDLANGSELECTEVENT_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SATK_SENDLANGSELECTEVENT_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SATK_SENDLANGSELECTEVENT_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SATK_SENDLANGSELECTEVENT_REQ_HEADER Param1;
} yPDef_MSG_SATK_SENDLANGSELECTEVENT_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SATK_SENDLANGSELECTEVENT_RSP;

typedef CAPI2_SatkApi_SendBrowserTermEvent_Req_t *T_INTER_TASK_MSG_SATK_SENDBROWSERTERMEVENT_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SATK_SENDBROWSERTERMEVENT_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SATK_SENDBROWSERTERMEVENT_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SATK_SENDBROWSERTERMEVENT_REQ_HEADER Param1;
} yPDef_MSG_SATK_SENDBROWSERTERMEVENT_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SATK_SENDBROWSERTERMEVENT_RSP;

typedef CAPI2_SatkApi_CmdResp_Req_t *T_INTER_TASK_MSG_SATK_CMDRESP_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SATK_CMDRESP_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SATK_CMDRESP_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SATK_CMDRESP_REQ_HEADER Param1;
} yPDef_MSG_SATK_CMDRESP_REQ;

typedef Boolean *T_INTER_TASK_MSG_SATK_CMDRESP_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SATK_CMDRESP_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SATK_CMDRESP_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SATK_CMDRESP_RSP_HEADER Param1;
} yPDef_MSG_SATK_CMDRESP_RSP;

typedef CAPI2_SatkApi_DataServCmdResp_Req_t *T_INTER_TASK_MSG_SATK_DATASERVCMDRESP_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SATK_DATASERVCMDRESP_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SATK_DATASERVCMDRESP_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SATK_DATASERVCMDRESP_REQ_HEADER Param1;
} yPDef_MSG_SATK_DATASERVCMDRESP_REQ;

typedef Boolean *T_INTER_TASK_MSG_SATK_DATASERVCMDRESP_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SATK_DATASERVCMDRESP_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SATK_DATASERVCMDRESP_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SATK_DATASERVCMDRESP_RSP_HEADER Param1;
} yPDef_MSG_SATK_DATASERVCMDRESP_RSP;

typedef CAPI2_SatkApi_SendDataServReq_Req_t *T_INTER_TASK_MSG_SATK_SENDDATASERVREQ_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SATK_SENDDATASERVREQ_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SATK_SENDDATASERVREQ_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SATK_SENDDATASERVREQ_REQ_HEADER Param1;
} yPDef_MSG_SATK_SENDDATASERVREQ_REQ;

typedef Boolean *T_INTER_TASK_MSG_SATK_SENDDATASERVREQ_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SATK_SENDDATASERVREQ_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SATK_SENDDATASERVREQ_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SATK_SENDDATASERVREQ_RSP_HEADER Param1;
} yPDef_MSG_SATK_SENDDATASERVREQ_RSP;

typedef CAPI2_SatkApi_SendTerminalRsp_Req_t *T_INTER_TASK_MSG_SATK_SENDTERMINALRSP_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SATK_SENDTERMINALRSP_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SATK_SENDTERMINALRSP_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SATK_SENDTERMINALRSP_REQ_HEADER Param1;
} yPDef_MSG_SATK_SENDTERMINALRSP_REQ;

typedef Boolean *T_INTER_TASK_MSG_SATK_SENDTERMINALRSP_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SATK_SENDTERMINALRSP_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SATK_SENDTERMINALRSP_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SATK_SENDTERMINALRSP_RSP_HEADER Param1;
} yPDef_MSG_SATK_SENDTERMINALRSP_RSP;

typedef CAPI2_SatkApi_SetTermProfile_Req_t *T_INTER_TASK_MSG_SATK_SETTERMPROFILE_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SATK_SETTERMPROFILE_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SATK_SETTERMPROFILE_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SATK_SETTERMPROFILE_REQ_HEADER Param1;
} yPDef_MSG_SATK_SETTERMPROFILE_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SATK_SETTERMPROFILE_RSP;

typedef CAPI2_SatkApi_SendEnvelopeCmdReq_Req_t *T_INTER_TASK_MSG_SATK_SEND_ENVELOPE_CMD_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SATK_SEND_ENVELOPE_CMD_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SATK_SEND_ENVELOPE_CMD_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SATK_SEND_ENVELOPE_CMD_REQ_HEADER Param1;
} yPDef_MSG_SATK_SEND_ENVELOPE_CMD_REQ;

typedef CAPI2_SatkApi_SendTerminalRspReq_Req_t *T_INTER_TASK_MSG_STK_TERMINAL_RESPONSE_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_STK_TERMINAL_RESPONSE_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_STK_TERMINAL_RESPONSE_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_STK_TERMINAL_RESPONSE_REQ_HEADER Param1;
} yPDef_MSG_STK_TERMINAL_RESPONSE_REQ;

typedef CAPI2_SATK_SendBrowsingStatusEvent_Req_t *T_INTER_TASK_MSG_STK_SEND_BROWSING_STATUS_EVT_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_STK_SEND_BROWSING_STATUS_EVT_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_STK_SEND_BROWSING_STATUS_EVT_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_STK_SEND_BROWSING_STATUS_EVT_REQ_HEADER Param1;
} yPDef_MSG_STK_SEND_BROWSING_STATUS_EVT_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_STK_SEND_BROWSING_STATUS_EVT_RSP;

typedef CAPI2_SatkApi_SendCcSetupReq_Req_t *T_INTER_TASK_MSG_SATK_SEND_CC_SETUP_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SATK_SEND_CC_SETUP_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SATK_SEND_CC_SETUP_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SATK_SEND_CC_SETUP_REQ_HEADER Param1;
} yPDef_MSG_SATK_SEND_CC_SETUP_REQ;

typedef CAPI2_SatkApi_SendCcSsReq_Req_t *T_INTER_TASK_MSG_SATK_SEND_CC_SS_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SATK_SEND_CC_SS_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SATK_SEND_CC_SS_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SATK_SEND_CC_SS_REQ_HEADER Param1;
} yPDef_MSG_SATK_SEND_CC_SS_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SATK_SEND_CC_SS_RSP;

typedef CAPI2_SatkApi_SendCcUssdReq_Req_t *T_INTER_TASK_MSG_SATK_SEND_CC_USSD_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SATK_SEND_CC_USSD_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SATK_SEND_CC_USSD_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SATK_SEND_CC_USSD_REQ_HEADER Param1;
} yPDef_MSG_SATK_SEND_CC_USSD_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SATK_SEND_CC_USSD_RSP;

typedef CAPI2_SatkApi_SendCcSmsReq_Req_t *T_INTER_TASK_MSG_SATK_SEND_CC_SMS_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SATK_SEND_CC_SMS_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SATK_SEND_CC_SMS_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SATK_SEND_CC_SMS_REQ_HEADER Param1;
} yPDef_MSG_SATK_SEND_CC_SMS_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SATK_SEND_CC_SMS_RSP;

typedef CAPI2_LCS_CpMoLrReq_Req_t *T_INTER_TASK_MSG_LCS_CPMOLRREQ_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_LCS_CPMOLRREQ_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_LCS_CPMOLRREQ_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_LCS_CPMOLRREQ_REQ_HEADER Param1;
} yPDef_MSG_LCS_CPMOLRREQ_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_LCS_CPMOLRABORT_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_LCS_CPMOLRABORT_RSP;

typedef CAPI2_LCS_CpMtLrVerificationRsp_Req_t *T_INTER_TASK_MSG_LCS_CPMTLRVERIFICATIONRSP_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_LCS_CPMTLRVERIFICATIONRSP_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_LCS_CPMTLRVERIFICATIONRSP_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_LCS_CPMTLRVERIFICATIONRSP_REQ_HEADER Param1;
} yPDef_MSG_LCS_CPMTLRVERIFICATIONRSP_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_LCS_CPMTLRVERIFICATIONRSP_RSP;

typedef CAPI2_LCS_CpMtLrRsp_Req_t *T_INTER_TASK_MSG_LCS_CPMTLRRSP_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_LCS_CPMTLRRSP_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_LCS_CPMTLRRSP_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_LCS_CPMTLRRSP_REQ_HEADER Param1;
} yPDef_MSG_LCS_CPMTLRRSP_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_LCS_CPMTLRRSP_RSP;

typedef CAPI2_LCS_CpLocUpdateRsp_Req_t *T_INTER_TASK_MSG_LCS_CPLOCUPDATERSP_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_LCS_CPLOCUPDATERSP_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_LCS_CPLOCUPDATERSP_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_LCS_CPLOCUPDATERSP_REQ_HEADER Param1;
} yPDef_MSG_LCS_CPLOCUPDATERSP_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_LCS_CPLOCUPDATERSP_RSP;

typedef CAPI2_LCS_DecodePosEstimate_Req_t *T_INTER_TASK_MSG_LCS_DECODEPOSESTIMATE_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_LCS_DECODEPOSESTIMATE_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_LCS_DECODEPOSESTIMATE_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_LCS_DECODEPOSESTIMATE_REQ_HEADER Param1;
} yPDef_MSG_LCS_DECODEPOSESTIMATE_REQ;

typedef void *T_INTER_TASK_MSG_LCS_DECODEPOSESTIMATE_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_LCS_DECODEPOSESTIMATE_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_LCS_DECODEPOSESTIMATE_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_LCS_DECODEPOSESTIMATE_RSP_HEADER Param1;
} yPDef_MSG_LCS_DECODEPOSESTIMATE_RSP;

typedef CAPI2_LCS_EncodeAssistanceReq_Req_t *T_INTER_TASK_MSG_LCS_ENCODEASSISTANCEREQ_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_LCS_ENCODEASSISTANCEREQ_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_LCS_ENCODEASSISTANCEREQ_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_LCS_ENCODEASSISTANCEREQ_REQ_HEADER Param1;
} yPDef_MSG_LCS_ENCODEASSISTANCEREQ_REQ;

typedef Int32 *T_INTER_TASK_MSG_LCS_ENCODEASSISTANCEREQ_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_LCS_ENCODEASSISTANCEREQ_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_LCS_ENCODEASSISTANCEREQ_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_LCS_ENCODEASSISTANCEREQ_RSP_HEADER Param1;
} yPDef_MSG_LCS_ENCODEASSISTANCEREQ_RSP;

typedef CAPI2_LCS_SendRrlpDataToNetwork_Req_t *T_INTER_TASK_MSG_LCS_SEND_RRLP_DATA_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_LCS_SEND_RRLP_DATA_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_LCS_SEND_RRLP_DATA_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_LCS_SEND_RRLP_DATA_REQ_HEADER Param1;
} yPDef_MSG_LCS_SEND_RRLP_DATA_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_LCS_SEND_RRLP_DATA_RSP;

typedef CAPI2_LCS_RrcMeasurementReport_Req_t *T_INTER_TASK_MSG_LCS_RRC_MEAS_REPORT_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_LCS_RRC_MEAS_REPORT_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_LCS_RRC_MEAS_REPORT_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_LCS_RRC_MEAS_REPORT_REQ_HEADER Param1;
} yPDef_MSG_LCS_RRC_MEAS_REPORT_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_LCS_RRC_MEAS_REPORT_RSP;

typedef CAPI2_LCS_RrcMeasurementControlFailure_Req_t *T_INTER_TASK_MSG_LCS_RRC_MEAS_FAILURE_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_LCS_RRC_MEAS_FAILURE_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_LCS_RRC_MEAS_FAILURE_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_LCS_RRC_MEAS_FAILURE_REQ_HEADER Param1;
} yPDef_MSG_LCS_RRC_MEAS_FAILURE_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_LCS_RRC_MEAS_FAILURE_RSP;

typedef CAPI2_LCS_RrcStatus_Req_t *T_INTER_TASK_MSG_LCS_RRC_STATUS_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_LCS_RRC_STATUS_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_LCS_RRC_STATUS_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_LCS_RRC_STATUS_REQ_HEADER Param1;
} yPDef_MSG_LCS_RRC_STATUS_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_LCS_RRC_STATUS_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_LCS_FTT_SYNC_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_LCS_FTT_SYNC_RSP;

typedef CAPI2_CcApi_MakeVoiceCall_Req_t *T_INTER_TASK_MSG_CC_MAKEVOICECALL_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_MAKEVOICECALL_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_MAKEVOICECALL_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_MAKEVOICECALL_REQ_HEADER Param1;
} yPDef_MSG_CC_MAKEVOICECALL_REQ;

typedef CAPI2_CcApi_MakeDataCall_Req_t *T_INTER_TASK_MSG_CC_MAKEDATACALL_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_MAKEDATACALL_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_MAKEDATACALL_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_MAKEDATACALL_REQ_HEADER Param1;
} yPDef_MSG_CC_MAKEDATACALL_REQ;

typedef CAPI2_CcApi_MakeFaxCall_Req_t *T_INTER_TASK_MSG_CC_MAKEFAXCALL_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_MAKEFAXCALL_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_MAKEFAXCALL_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_MAKEFAXCALL_REQ_HEADER Param1;
} yPDef_MSG_CC_MAKEFAXCALL_REQ;

typedef CAPI2_CcApi_MakeVideoCall_Req_t *T_INTER_TASK_MSG_CC_MAKEVIDEOCALL_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_MAKEVIDEOCALL_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_MAKEVIDEOCALL_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_MAKEVIDEOCALL_REQ_HEADER Param1;
} yPDef_MSG_CC_MAKEVIDEOCALL_REQ;

typedef CAPI2_CcApi_EndCall_Req_t *T_INTER_TASK_MSG_CC_ENDCALL_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_ENDCALL_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_ENDCALL_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_ENDCALL_REQ_HEADER Param1;
} yPDef_MSG_CC_ENDCALL_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_ENDALLCALLS_REQ;

typedef CAPI2_CcApi_EndCallImmediate_Req_t *T_INTER_TASK_MSG_CCAPI_ENDCALL_IMMEDIATE_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CCAPI_ENDCALL_IMMEDIATE_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CCAPI_ENDCALL_IMMEDIATE_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CCAPI_ENDCALL_IMMEDIATE_REQ_HEADER Param1;
} yPDef_MSG_CCAPI_ENDCALL_IMMEDIATE_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CCAPI_ENDALLCALLS_IMMEDIATE_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_ENDMPTYCALLS_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_ENDHELDCALL_REQ;

typedef CAPI2_CcApi_AcceptVoiceCall_Req_t *T_INTER_TASK_MSG_CC_ACCEPTVOICECALL_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_ACCEPTVOICECALL_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_ACCEPTVOICECALL_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_ACCEPTVOICECALL_REQ_HEADER Param1;
} yPDef_MSG_CC_ACCEPTVOICECALL_REQ;

typedef CAPI2_CcApi_AcceptDataCall_Req_t *T_INTER_TASK_MSG_CC_ACCEPTDATACALL_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_ACCEPTDATACALL_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_ACCEPTDATACALL_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_ACCEPTDATACALL_REQ_HEADER Param1;
} yPDef_MSG_CC_ACCEPTDATACALL_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_ACCEPTWAITINGCALL_REQ;

typedef CAPI2_CcApi_AcceptVideoCall_Req_t *T_INTER_TASK_MSG_CC_ACCEPTVIDEOCALL_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_ACCEPTVIDEOCALL_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_ACCEPTVIDEOCALL_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_ACCEPTVIDEOCALL_REQ_HEADER Param1;
} yPDef_MSG_CC_ACCEPTVIDEOCALL_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_HOLDCURRENTCALL_REQ;

typedef CAPI2_CcApi_HoldCall_Req_t *T_INTER_TASK_MSG_CC_HOLDCALL_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_HOLDCALL_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_HOLDCALL_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_HOLDCALL_REQ_HEADER Param1;
} yPDef_MSG_CC_HOLDCALL_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CC_RETRIEVENEXTHELDCALL_REQ;

typedef CAPI2_CcApi_RetrieveCall_Req_t *T_INTER_TASK_MSG_CC_RETRIEVECALL_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_RETRIEVECALL_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_RETRIEVECALL_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_RETRIEVECALL_REQ_HEADER Param1;
} yPDef_MSG_CC_RETRIEVECALL_REQ;

typedef CAPI2_CcApi_SwapCall_Req_t *T_INTER_TASK_MSG_CC_SWAPCALL_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_SWAPCALL_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_SWAPCALL_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_SWAPCALL_REQ_HEADER Param1;
} yPDef_MSG_CC_SWAPCALL_REQ;

typedef CAPI2_CcApi_SplitCall_Req_t *T_INTER_TASK_MSG_CC_SPLITCALL_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_SPLITCALL_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_SPLITCALL_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_SPLITCALL_REQ_HEADER Param1;
} yPDef_MSG_CC_SPLITCALL_REQ;

typedef CAPI2_CcApi_JoinCall_Req_t *T_INTER_TASK_MSG_CC_JOINCALL_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_JOINCALL_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_JOINCALL_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_JOINCALL_REQ_HEADER Param1;
} yPDef_MSG_CC_JOINCALL_REQ;

typedef CAPI2_CcApi_TransferCall_Req_t *T_INTER_TASK_MSG_CC_TRANSFERCALL_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_TRANSFERCALL_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_TRANSFERCALL_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_TRANSFERCALL_REQ_HEADER Param1;
} yPDef_MSG_CC_TRANSFERCALL_REQ;

typedef CAPI2_CcApi_GetCNAPName_Req_t *T_INTER_TASK_MSG_CC_GETCNAPNAME_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GETCNAPNAME_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GETCNAPNAME_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GETCNAPNAME_REQ_HEADER Param1;
} yPDef_MSG_CC_GETCNAPNAME_REQ;

typedef CNAP_NAME_t *T_INTER_TASK_MSG_CC_GETCNAPNAME_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_GETCNAPNAME_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_GETCNAPNAME_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_GETCNAPNAME_RSP_HEADER Param1;
} yPDef_MSG_CC_GETCNAPNAME_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SYSPARM_GET_HSUPA_SUPPORTED_REQ;

typedef Boolean *T_INTER_TASK_MSG_SYSPARM_GET_HSUPA_SUPPORTED_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SYSPARM_GET_HSUPA_SUPPORTED_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SYSPARM_GET_HSUPA_SUPPORTED_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SYSPARM_GET_HSUPA_SUPPORTED_RSP_HEADER Param1;
} yPDef_MSG_SYSPARM_GET_HSUPA_SUPPORTED_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SYSPARM_GET_HSDPA_SUPPORTED_REQ;

typedef Boolean *T_INTER_TASK_MSG_SYSPARM_GET_HSDPA_SUPPORTED_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SYSPARM_GET_HSDPA_SUPPORTED_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SYSPARM_GET_HSDPA_SUPPORTED_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SYSPARM_GET_HSDPA_SUPPORTED_RSP_HEADER Param1;
} yPDef_MSG_SYSPARM_GET_HSDPA_SUPPORTED_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_MS_FORCE_PS_REL_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_MS_FORCE_PS_REL_RSP;

typedef CAPI2_CcApi_IsCurrentStateMpty_Req_t *T_INTER_TASK_MSG_CC_ISCURRENTSTATEMPTY_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_ISCURRENTSTATEMPTY_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_ISCURRENTSTATEMPTY_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_ISCURRENTSTATEMPTY_REQ_HEADER Param1;
} yPDef_MSG_CC_ISCURRENTSTATEMPTY_REQ;

typedef Boolean *T_INTER_TASK_MSG_CC_ISCURRENTSTATEMPTY_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CC_ISCURRENTSTATEMPTY_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CC_ISCURRENTSTATEMPTY_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CC_ISCURRENTSTATEMPTY_RSP_HEADER Param1;
} yPDef_MSG_CC_ISCURRENTSTATEMPTY_RSP;

typedef CAPI2_PdpApi_GetPCHContextState_Req_t *T_INTER_TASK_MSG_PDP_GETPCHCONTEXTSTATE_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_GETPCHCONTEXTSTATE_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_GETPCHCONTEXTSTATE_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_GETPCHCONTEXTSTATE_REQ_HEADER Param1;
} yPDef_MSG_PDP_GETPCHCONTEXTSTATE_REQ;

typedef PCHContextState_t *T_INTER_TASK_MSG_PDP_GETPCHCONTEXTSTATE_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_GETPCHCONTEXTSTATE_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_GETPCHCONTEXTSTATE_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_GETPCHCONTEXTSTATE_RSP_HEADER Param1;
} yPDef_MSG_PDP_GETPCHCONTEXTSTATE_RSP;

typedef CcApi_PdpApi_GetPDPContextEx_Req_t *T_INTER_TASK_MSG_PDP_GETPCHCONTEXT_EX_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_GETPCHCONTEXT_EX_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_GETPCHCONTEXT_EX_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_GETPCHCONTEXT_EX_REQ_HEADER Param1;
} yPDef_MSG_PDP_GETPCHCONTEXT_EX_REQ;

typedef Result_t *T_INTER_TASK_MSG_PDP_GETPCHCONTEXT_EX_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_GETPCHCONTEXT_EX_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_GETPCHCONTEXT_EX_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_GETPCHCONTEXT_EX_RSP_HEADER Param1;
} yPDef_MSG_PDP_GETPCHCONTEXT_EX_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIM_PIN_LOCK_TYPE_REQ;

typedef SIM_PIN_Status_t *T_INTER_TASK_MSG_SIM_PIN_LOCK_TYPE_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_PIN_LOCK_TYPE_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_PIN_LOCK_TYPE_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_PIN_LOCK_TYPE_RSP_HEADER Param1;
} yPDef_MSG_SIM_PIN_LOCK_TYPE_RSP;

typedef CAPI2_SimApi_SubmitSelectFileSendApduReq_Req_t *T_INTER_TASK_MSG_SIM_SEND_APDU_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_SEND_APDU_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_SEND_APDU_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_SEND_APDU_REQ_HEADER Param1;
} yPDef_MSG_SIM_SEND_APDU_REQ;

typedef CAPI2_SimApi_SendSelectApplicationReq_Req_t *T_INTER_TASK_MSG_SIM_SELECT_APPLICATION_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SIM_SELECT_APPLICATION_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SIM_SELECT_APPLICATION_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SIM_SELECT_APPLICATION_REQ_HEADER Param1;
} yPDef_MSG_SIM_SELECT_APPLICATION_REQ;

typedef CAPI2_SatkApi_SendProactiveCmdFetchingOnOffReq_Req_t *T_INTER_TASK_MSG_STK_PROACTIVE_CMD_FETCHING_ON_OFF_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_STK_PROACTIVE_CMD_FETCHING_ON_OFF_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_STK_PROACTIVE_CMD_FETCHING_ON_OFF_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_STK_PROACTIVE_CMD_FETCHING_ON_OFF_REQ_HEADER Param1;
} yPDef_MSG_STK_PROACTIVE_CMD_FETCHING_ON_OFF_REQ;

typedef CAPI2_SatkApi_SendExtProactiveCmdReq_Req_t *T_INTER_TASK_MSG_SATK_SEND_EXT_PROACTIVE_CMD_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SATK_SEND_EXT_PROACTIVE_CMD_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SATK_SEND_EXT_PROACTIVE_CMD_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SATK_SEND_EXT_PROACTIVE_CMD_REQ_HEADER Param1;
} yPDef_MSG_SATK_SEND_EXT_PROACTIVE_CMD_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SATK_SEND_EXT_PROACTIVE_CMD_RSP;

typedef CAPI2_SatkApi_SendTerminalProfileReq_Req_t *T_INTER_TASK_MSG_SATK_SEND_TERMINAL_PROFILE_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SATK_SEND_TERMINAL_PROFILE_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SATK_SEND_TERMINAL_PROFILE_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SATK_SEND_TERMINAL_PROFILE_REQ_HEADER Param1;
} yPDef_MSG_SATK_SEND_TERMINAL_PROFILE_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SATK_SEND_TERMINAL_PROFILE_RSP;

typedef CAPI2_SatkApi_SendPollingIntervalReq_Req_t *T_INTER_TASK_MSG_STK_POLLING_INTERVAL_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_STK_POLLING_INTERVAL_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_STK_POLLING_INTERVAL_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_STK_POLLING_INTERVAL_REQ_HEADER Param1;
} yPDef_MSG_STK_POLLING_INTERVAL_REQ;

typedef CAPI2_PdpApi_SetPDPActivationCallControlFlag_Req_t *T_INTER_TASK_MSG_PDP_SETPDPACTIVATIONCCFLAG_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_SETPDPACTIVATIONCCFLAG_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_SETPDPACTIVATIONCCFLAG_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_SETPDPACTIVATIONCCFLAG_REQ_HEADER Param1;
} yPDef_MSG_PDP_SETPDPACTIVATIONCCFLAG_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_SETPDPACTIVATIONCCFLAG_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_GETPDPACTIVATIONCCFLAG_REQ;

typedef Boolean *T_INTER_TASK_MSG_PDP_GETPDPACTIVATIONCCFLAG_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_GETPDPACTIVATIONCCFLAG_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_GETPDPACTIVATIONCCFLAG_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_GETPDPACTIVATIONCCFLAG_RSP_HEADER Param1;
} yPDef_MSG_PDP_GETPDPACTIVATIONCCFLAG_RSP;

typedef CAPI2_PdpApi_SendPDPActivateReq_PDU_Req_t *T_INTER_TASK_MSG_PDP_SENDPDPACTIVATIONPDU_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_SENDPDPACTIVATIONPDU_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_SENDPDPACTIVATIONPDU_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_SENDPDPACTIVATIONPDU_REQ_HEADER Param1;
} yPDef_MSG_PDP_SENDPDPACTIVATIONPDU_REQ;

typedef CAPI2_PdpApi_RejectNWIPDPActivation_Req_t *T_INTER_TASK_MSG_PDP_REJECTNWIACTIVATION_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_REJECTNWIACTIVATION_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_REJECTNWIACTIVATION_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_REJECTNWIACTIVATION_REQ_HEADER Param1;
} yPDef_MSG_PDP_REJECTNWIACTIVATION_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_REJECTNWIACTIVATION_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_GEN_REQ_END;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_CAPI2_MSGIDS_END;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_ISMESTORAGEENABLED_REQ;

typedef Boolean *T_INTER_TASK_MSG_SMS_ISMESTORAGEENABLED_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_ISMESTORAGEENABLED_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_ISMESTORAGEENABLED_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_ISMESTORAGEENABLED_RSP_HEADER Param1;
} yPDef_MSG_SMS_ISMESTORAGEENABLED_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_GETMAXMECAPACITY_REQ;

typedef UInt16 *T_INTER_TASK_MSG_SMS_GETMAXMECAPACITY_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_GETMAXMECAPACITY_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_GETMAXMECAPACITY_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_GETMAXMECAPACITY_RSP_HEADER Param1;
} yPDef_MSG_SMS_GETMAXMECAPACITY_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_GETNEXTFREESLOT_REQ;

typedef UInt16 *T_INTER_TASK_MSG_SMS_GETNEXTFREESLOT_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_GETNEXTFREESLOT_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_GETNEXTFREESLOT_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_GETNEXTFREESLOT_RSP_HEADER Param1;
} yPDef_MSG_SMS_GETNEXTFREESLOT_RSP;

typedef CAPI2_SMS_SetMeSmsStatus_Req_t *T_INTER_TASK_MSG_SMS_SETMESMSSTATUS_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_SETMESMSSTATUS_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_SETMESMSSTATUS_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_SETMESMSSTATUS_REQ_HEADER Param1;
} yPDef_MSG_SMS_SETMESMSSTATUS_REQ;

typedef Result_t *T_INTER_TASK_MSG_SMS_SETMESMSSTATUS_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_SETMESMSSTATUS_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_SETMESMSSTATUS_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_SETMESMSSTATUS_RSP_HEADER Param1;
} yPDef_MSG_SMS_SETMESMSSTATUS_RSP;

typedef CAPI2_SMS_GetMeSmsStatus_Req_t *T_INTER_TASK_MSG_SMS_GETMESMSSTATUS_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_GETMESMSSTATUS_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_GETMESMSSTATUS_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_GETMESMSSTATUS_REQ_HEADER Param1;
} yPDef_MSG_SMS_GETMESMSSTATUS_REQ;

typedef SIMSMSMesgStatus_t *T_INTER_TASK_MSG_SMS_GETMESMSSTATUS_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_GETMESMSSTATUS_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_GETMESMSSTATUS_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_GETMESMSSTATUS_RSP_HEADER Param1;
} yPDef_MSG_SMS_GETMESMSSTATUS_RSP;

typedef CAPI2_SMS_StoreSmsToMe_Req_t *T_INTER_TASK_MSG_SMS_STORESMSTOME_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_STORESMSTOME_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_STORESMSTOME_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_STORESMSTOME_REQ_HEADER Param1;
} yPDef_MSG_SMS_STORESMSTOME_REQ;

typedef Result_t *T_INTER_TASK_MSG_SMS_STORESMSTOME_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_STORESMSTOME_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_STORESMSTOME_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_STORESMSTOME_RSP_HEADER Param1;
} yPDef_MSG_SMS_STORESMSTOME_RSP;

typedef CAPI2_SMS_RetrieveSmsFromMe_Req_t *T_INTER_TASK_MSG_SMS_RETRIEVESMSFROMME_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_RETRIEVESMSFROMME_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_RETRIEVESMSFROMME_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_RETRIEVESMSFROMME_REQ_HEADER Param1;
} yPDef_MSG_SMS_RETRIEVESMSFROMME_REQ;

typedef Result_t *T_INTER_TASK_MSG_SMS_RETRIEVESMSFROMME_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_RETRIEVESMSFROMME_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_RETRIEVESMSFROMME_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_RETRIEVESMSFROMME_RSP_HEADER Param1;
} yPDef_MSG_SMS_RETRIEVESMSFROMME_RSP;

typedef CAPI2_SMS_RemoveSmsFromMe_Req_t *T_INTER_TASK_MSG_SMS_REMOVESMSFROMME_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_REMOVESMSFROMME_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_REMOVESMSFROMME_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_REMOVESMSFROMME_REQ_HEADER Param1;
} yPDef_MSG_SMS_REMOVESMSFROMME_REQ;

typedef Result_t *T_INTER_TASK_MSG_SMS_REMOVESMSFROMME_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_REMOVESMSFROMME_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_REMOVESMSFROMME_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_REMOVESMSFROMME_RSP_HEADER Param1;
} yPDef_MSG_SMS_REMOVESMSFROMME_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_ISSMSREPLACE_SUPPORTED_REQ;

typedef Boolean *T_INTER_TASK_MSG_SMS_ISSMSREPLACE_SUPPORTED_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_ISSMSREPLACE_SUPPORTED_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_ISSMSREPLACE_SUPPORTED_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_ISSMSREPLACE_SUPPORTED_RSP_HEADER Param1;
} yPDef_MSG_SMS_ISSMSREPLACE_SUPPORTED_RSP;

typedef CAPI2_SMS_GetMeSmsBufferStatus_Req_t *T_INTER_TASK_MSG_SMS_GETMESMS_BUF_STATUS_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_GETMESMS_BUF_STATUS_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_GETMESMS_BUF_STATUS_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_GETMESMS_BUF_STATUS_REQ_HEADER Param1;
} yPDef_MSG_SMS_GETMESMS_BUF_STATUS_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SMS_GETMESMS_BUF_STATUS_RSP;

typedef CAPI2_SMS_GetRecordNumberOfReplaceSMS_Req_t *T_INTER_TASK_MSG_SMS_GETRECORDNUMBEROFREPLACESMS_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_GETRECORDNUMBEROFREPLACESMS_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_GETRECORDNUMBEROFREPLACESMS_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_GETRECORDNUMBEROFREPLACESMS_REQ_HEADER Param1;
} yPDef_MSG_SMS_GETRECORDNUMBEROFREPLACESMS_REQ;

typedef UInt16 *T_INTER_TASK_MSG_SMS_GETRECORDNUMBEROFREPLACESMS_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SMS_GETRECORDNUMBEROFREPLACESMS_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SMS_GETRECORDNUMBEROFREPLACESMS_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SMS_GETRECORDNUMBEROFREPLACESMS_RSP_HEADER Param1;
} yPDef_MSG_SMS_GETRECORDNUMBEROFREPLACESMS_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PMU_BATT_LEVEL_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PMU_BATT_LEVEL_RSP;

typedef CAPI2_CPPS_Control_Req_t *T_INTER_TASK_MSG_CPPS_CONTROL_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CPPS_CONTROL_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CPPS_CONTROL_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CPPS_CONTROL_REQ_HEADER Param1;
} yPDef_MSG_CPPS_CONTROL_REQ;

typedef UInt32 *T_INTER_TASK_MSG_CPPS_CONTROL_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CPPS_CONTROL_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CPPS_CONTROL_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CPPS_CONTROL_RSP_HEADER Param1;
} yPDef_MSG_CPPS_CONTROL_RSP;

typedef CAPI2_CP2AP_PedestalMode_Control_Req_t *T_INTER_TASK_MSG_CP2AP_PEDESTALMODE_CONTROL_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CP2AP_PEDESTALMODE_CONTROL_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CP2AP_PEDESTALMODE_CONTROL_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CP2AP_PEDESTALMODE_CONTROL_REQ_HEADER Param1;
} yPDef_MSG_CP2AP_PEDESTALMODE_CONTROL_REQ;

typedef Boolean *T_INTER_TASK_MSG_CP2AP_PEDESTALMODE_CONTROL_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_CP2AP_PEDESTALMODE_CONTROL_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_CP2AP_PEDESTALMODE_CONTROL_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_CP2AP_PEDESTALMODE_CONTROL_RSP_HEADER Param1;
} yPDef_MSG_CP2AP_PEDESTALMODE_CONTROL_RSP;

typedef CAPI2_PMU_IsSIMReady_Req_t *T_INTER_TASK_MSG_PMU_IS_SIM_READY_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PMU_IS_SIM_READY_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PMU_IS_SIM_READY_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PMU_IS_SIM_READY_REQ_HEADER Param1;
} yPDef_MSG_PMU_IS_SIM_READY_REQ;

typedef Boolean *T_INTER_TASK_MSG_PMU_IS_SIM_READY_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PMU_IS_SIM_READY_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PMU_IS_SIM_READY_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PMU_IS_SIM_READY_RSP_HEADER Param1;
} yPDef_MSG_PMU_IS_SIM_READY_RSP;

typedef CAPI2_PMU_ActivateSIM_Req_t *T_INTER_TASK_MSG_PMU_ACTIVATE_SIM_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PMU_ACTIVATE_SIM_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PMU_ACTIVATE_SIM_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PMU_ACTIVATE_SIM_REQ_HEADER Param1;
} yPDef_MSG_PMU_ACTIVATE_SIM_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PMU_ACTIVATE_SIM_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PMU_ClientPowerDown_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PMU_ClientPowerDown_RSP;

typedef CAPI2_FLASH_SaveImage_Req_t *T_INTER_TASK_MSG_FLASH_SAVEIMAGE_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_FLASH_SAVEIMAGE_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_FLASH_SAVEIMAGE_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_FLASH_SAVEIMAGE_REQ_HEADER Param1;
} yPDef_MSG_FLASH_SAVEIMAGE_REQ;

typedef Boolean *T_INTER_TASK_MSG_FLASH_SAVEIMAGE_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_FLASH_SAVEIMAGE_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_FLASH_SAVEIMAGE_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_FLASH_SAVEIMAGE_RSP_HEADER Param1;
} yPDef_MSG_FLASH_SAVEIMAGE_RSP;

typedef CAPI2_USB_IpcMsg_Req_t *T_INTER_TASK_MSG_USB_IPC_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_USB_IPC_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_USB_IPC_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_USB_IPC_REQ_HEADER Param1;
} yPDef_MSG_USB_IPC_REQ;

typedef USBPayload_t *T_INTER_TASK_MSG_USB_IPC_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_USB_IPC_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_USB_IPC_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_USB_IPC_RSP_HEADER Param1;
} yPDef_MSG_USB_IPC_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SYSPARAM_GET_IMEI_REQ;

typedef CAPI2_SYSPARM_IMEI_PTR_t *T_INTER_TASK_MSG_SYSPARAM_GET_IMEI_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SYSPARAM_GET_IMEI_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SYSPARAM_GET_IMEI_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SYSPARAM_GET_IMEI_RSP_HEADER Param1;
} yPDef_MSG_SYSPARAM_GET_IMEI_RSP;

typedef SYS_Sync_SetRegisteredEventMask_Req_t *T_INTER_TASK_MSG_SYS_SYNC_SET_REG_EVENT_MASK_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SYS_SYNC_SET_REG_EVENT_MASK_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SYS_SYNC_SET_REG_EVENT_MASK_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SYS_SYNC_SET_REG_EVENT_MASK_REQ_HEADER Param1;
} yPDef_MSG_SYS_SYNC_SET_REG_EVENT_MASK_REQ;

typedef Boolean *T_INTER_TASK_MSG_SYS_SYNC_SET_REG_EVENT_MASK_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SYS_SYNC_SET_REG_EVENT_MASK_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SYS_SYNC_SET_REG_EVENT_MASK_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SYS_SYNC_SET_REG_EVENT_MASK_RSP_HEADER Param1;
} yPDef_MSG_SYS_SYNC_SET_REG_EVENT_MASK_RSP;

typedef SYS_Sync_SetFilteredEventMask_Req_t *T_INTER_TASK_MSG_SYS_SYNC_SET_REG_FILTER_MASK_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SYS_SYNC_SET_REG_FILTER_MASK_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SYS_SYNC_SET_REG_FILTER_MASK_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SYS_SYNC_SET_REG_FILTER_MASK_REQ_HEADER Param1;
} yPDef_MSG_SYS_SYNC_SET_REG_FILTER_MASK_REQ;

typedef Boolean *T_INTER_TASK_MSG_SYS_SYNC_SET_REG_FILTER_MASK_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SYS_SYNC_SET_REG_FILTER_MASK_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SYS_SYNC_SET_REG_FILTER_MASK_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SYS_SYNC_SET_REG_FILTER_MASK_RSP_HEADER Param1;
} yPDef_MSG_SYS_SYNC_SET_REG_FILTER_MASK_RSP;

typedef SYS_Sync_RegisterForMSEvent_Req_t *T_INTER_TASK_MSG_SYS_SYNC_REG_EVENT_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SYS_SYNC_REG_EVENT_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SYS_SYNC_REG_EVENT_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SYS_SYNC_REG_EVENT_REQ_HEADER Param1;
} yPDef_MSG_SYS_SYNC_REG_EVENT_REQ;

typedef Boolean *T_INTER_TASK_MSG_SYS_SYNC_REG_EVENT_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SYS_SYNC_REG_EVENT_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SYS_SYNC_REG_EVENT_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SYS_SYNC_REG_EVENT_RSP_HEADER Param1;
} yPDef_MSG_SYS_SYNC_REG_EVENT_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SYS_SYNC_DEREG_EVENT_REQ;

typedef Boolean *T_INTER_TASK_MSG_SYS_SYNC_DEREG_EVENT_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SYS_SYNC_DEREG_EVENT_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SYS_SYNC_DEREG_EVENT_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SYS_SYNC_DEREG_EVENT_RSP_HEADER Param1;
} yPDef_MSG_SYS_SYNC_DEREG_EVENT_RSP;

typedef SYS_Sync_EnableFilterMask_Req_t *T_INTER_TASK_MSG_SYS_SYNC_ENABLE_FILTER_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SYS_SYNC_ENABLE_FILTER_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SYS_SYNC_ENABLE_FILTER_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SYS_SYNC_ENABLE_FILTER_REQ_HEADER Param1;
} yPDef_MSG_SYS_SYNC_ENABLE_FILTER_REQ;

typedef Boolean *T_INTER_TASK_MSG_SYS_SYNC_ENABLE_FILTER_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SYS_SYNC_ENABLE_FILTER_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SYS_SYNC_ENABLE_FILTER_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SYS_SYNC_ENABLE_FILTER_RSP_HEADER Param1;
} yPDef_MSG_SYS_SYNC_ENABLE_FILTER_RSP;

typedef SYS_Sync_RegisterSetClientName_Req_t *T_INTER_TASK_MSG_SYS_SYNC_REGISTER_NAME_REQ_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SYS_SYNC_REGISTER_NAME_REQ_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SYS_SYNC_REGISTER_NAME_REQ_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SYS_SYNC_REGISTER_NAME_REQ_HEADER Param1;
} yPDef_MSG_SYS_SYNC_REGISTER_NAME_REQ;

typedef Boolean *T_INTER_TASK_MSG_SYS_SYNC_REGISTER_NAME_RSP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_SYS_SYNC_REGISTER_NAME_RSP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_SYS_SYNC_REGISTER_NAME_RSP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_SYS_SYNC_REGISTER_NAME_RSP_HEADER Param1;
} yPDef_MSG_SYS_SYNC_REGISTER_NAME_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_ACCEPT;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_ALLOCINTERTASKMSGFROMHEAP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_AT_CMDCLOSETRANSACTION;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_AT_CMDRSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_AT_CMDRSPCMSERROR;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_AT_CMDRSPERROR;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_AT_CMDRSPOK;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_AT_CMDRSPSYNTAXERROR;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_AT_GETCMDNAME;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_AT_GETPARM;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_AT_GETVALIDPARMSTR;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_AT_OUTPUTRSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_AT_OUTPUTSTR;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_AT_OUTPUTSTRBUFFERED;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_AT_OUTPUTUNSOLICITEDSTR;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_AT_PROCESSCMD;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_AT_RESUMECMD;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_ATC_CONFIGSERIALDEVFORATC;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_AUDIO_CLOSESESSION;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_AUDIO_CREATEPLAYBACKSESSION;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_AUDIO_CREATERECORDINGSESSION;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_AUDIO_GETMICROPHONEGAIN;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_AUDIO_GETSIDETONEGAIN;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_AUDIO_GETSPEAKERVOL;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_AUDIO_PAUSESESSION;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_AUDIO_PLAYTONE;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_AUDIO_RESUMESESSION;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_AUDIO_RUN;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_AUDIO_SELECTAUDIOCHANNEL;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_AUDIO_SETMICROPHONEGAIN;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_AUDIO_SETSIDETONEGAIN;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_AUDIO_SETSPEAKERVOL;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_AUDIO_SHUTDOWN;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_AUDIO_STARTSESSION;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_AUDIO_STOPSESSION;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_BIND;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_CALLPARSER_PARSEDIALSTR;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_CC_ACCEPTDATACALL;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_CC_ACCEPTVOICECALL;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_CC_ACCEPTWAITINGCALL;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_CC_ENDALLCALLS;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_CC_END_CALL;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_CC_ENDHELDCALL;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_CC_ENDMPTYCALLS;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_CC_GETALLACTIVECALLINDEX;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_CC_GETALLCALLINDEX;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_CC_GETALLCALLSTATES;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_CC_GETALLHELDCALLINDEX;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_CC_GETALLMPTYCALLINDEX;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_CC_GETCALLCLIENTID;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_CC_GETCALLNUMBER;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_CC_GETCALLSTATE;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_CC_GETCALLTYPE;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_CC_GETCONNECTEDLINEID;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_CC_GETCURRENTCALLINDEX;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_CC_GETLASTCALLCCM;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_CC_GETLASTCALLDURATION;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_CC_GETLASTCALLEXITCAUSE;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_CC_GETLASTDATACALLRXBYTES;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_CC_GETLASTDATACALLTXBYTES;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_CC_GETMPTYCALLINDEX;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_CC_GETNEXTACTIVECALLINDEX;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_CC_GETNEXTHELDCALLINDEX;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_CC_GETNEXTWAITCALLINDEX;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_CC_GETNUMOFACTIVECALLS;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_CC_GETNUMOFHELDCALLS;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_CC_GETNUMOFMPTYCALLS;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_CC_HOLDCALL;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_CC_HOLDCURRENTCALL;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_CC_INITCALLCONTROL;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_CC_ISCONNECTEDLINEIDPRESENTALLOWED;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_CC_ISMULTIPARTYCALL;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_CC_ISTHEREALERTINGCALL;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_CC_ISTHEREVOICECALL;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_CC_ISTHEREWAITINGCALL;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_CC_ISVALIDDTMF;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_CC_JOINCALL;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_CC_MAKEDATACALL;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_CC_MAKEFAXCALL;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_CC_MAKE_VOICE_CALL;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_CC_MUTEDTMFTONE;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_CC_RETRIEVECALL;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_CC_RETRIEVNEXTHELDCALL;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_CC_SENDDTMF;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_CC_SPLITCALL;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_CC_STOPDTMF;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_CC_SWAPCALL;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_CONNECT;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_CSD_BUFFERFREESPACE;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_CSD_GETDATA;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_CSD_REGISTERDATAIND;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_CSD_SENDDATA;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_DATA_CREATECSDDATAACCT;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_DATA_CREATEGPRSDATAACCT;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_DATA_DELETEDATAACCT;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_DATA_GETACCTTYPE;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_DATA_GETAUTHENMETHOD;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_DATA_GETCIDFROMDATAACCTID;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_DATA_GETCSDBAUDRATE;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_DATA_GETCSDCONNELEMENT;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_DATA_GETCSDCONTEXT;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_DATA_GETCSDDATACOMPTYPE;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_DATA_GETCSDDIALNUMBER;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_DATA_GETCSDDIALTYPE;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_DATA_GETCSDERRCORRECTIONTYPE;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_DATA_GETDATAACCTIDFROMCID;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_DATA_GETDATACOMPRESSION;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_DATA_GETEMPTYACCTSLOT;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_DATA_GETGPRSAPN;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_DATA_GETGPRSCONTEXT;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_DATA_GETGPRSHEADERCOMPRESSION;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_DATA_GETGPRSPDPTYPE;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_DATA_GETGPRSQOS;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_DATA_GETPASSWORD;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_DATA_GETPRIMARYDNSADDR;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_DATA_GETSECONDDNSADDR;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_DATA_GETSTATICIPADDR;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_DATA_GETUSERNAME;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_DATA_SETAUTHENMETHOD;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_DATA_SETCSDBAUDRATE;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_DATA_SETCSDCONNELEMENT;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_DATA_SETCSDDATACOMPTYPE;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_DATA_SETCSDDIALNUMBER;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_DATA_SETCSDDIALTYPE;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_DATA_SETCSDERRCORRECTIONTYPE;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_DATA_SETDATACOMPRESSION;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_DATA_SETGPRSAPN;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_DATA_SETGPRSHEADERCOMPRESSION;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_DATA_SETGPRSPDPTYPE;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_DATA_SETGPRSQOS;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_DATA_SETPASSWORD;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_DATA_SETPRIMARYDNSADDR;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_DATA_SETSECONDDNSADDR;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_DATA_SETSTATICIPADDR;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_DATA_SETUSERNAME;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_ERRNO;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_FREEINTERTASKMSG;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_FS_CLOSE;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_FS_FLUSH;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_FS_GEFREESPACE;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_FS_GETCURRENTDIR;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_FS_GETFILEDONE;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_FS_GETFILEFIRST;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_FS_GETFILENEXT;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_FS_MAKEDIR;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_FS_OPEN;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_FS_READ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_FS_REMOVE;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_FS_REMOVEDIR;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_FS_RENAME;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_FS_SEEK;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_FS_SETCURRENTDIR;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_FS_TELL;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_FS_WRITE;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_GETATCMDIDBYMPXCHANNEL;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_GETATCMDRSPMPXCHANNEL;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_GETPEERNAME;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_GETSMSBEARERTYPE;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_GETSMSRVCENTERNUMBER;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_GETSOCKNAME;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_GETSOCKOPT;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_GETV24OPERATIONMODE;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_GPIO_CLEARINT;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_GPIO_CONFIGINPUT;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_GPIO_CONFIGKEYPAD;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_GPIO_CONFIGOUTPUT;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_GPIO_DISABLEINT;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_GPIO_ENABLEINT;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_GPIO_KEYPADREGISTER;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_GPIO_READ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_GPIO_READINTSTATUS;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_GPIO_REGISTER;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_GPIO_SELECTROW;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_GPIO_WRITE;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_I2C_INIT;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_I2C_READ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_I2C_RUN;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_I2C_WRITE;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_IMAGE_CONVERT;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_IMAGE_DECODE;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_IMAGE_GETINFO;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_IMAGE_ROTATE;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_IMAGE_RUN;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_IMAGE_SCALE;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_IMAGE_SHUTDOWN;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_IN_RESHOST;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_IRQ_CLEAR;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_IRQ_CLEARALL;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_IRQ_CONTROLLER;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_IRQ_DISABLE;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_IRQ_DISABLEALL;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_IRQ_ENABLE;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_IRQ_INIT;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_IRQ_ISACTIVE;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_IRQ_ISACTIVERAW;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_IRQ_ISENABLED;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_IRQ_REGISTER;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_IRQ_RESTORE;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_IRQ_TRIGGERRIPINT;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_KPD_DRV_GETLASTKEYPRESSTIME;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_KPD_DRV_INIT;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_KPD_DRV_REGISTER;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_KPD_DRV_RUN;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_KPD_DRV_SHUTDOWN;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_LISTEN;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_LOG_DEBUGOUTPUTSTRING;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_LOG_DEBUGOUTPUTVALUE;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_LOG_ENABLELOGGING;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_LOG_ENABLERANGE;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_LOG_GETLOGGINGID;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_LOG_ISLOGGINGENABLE;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_MM_DELETE;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_MM_DELOCATE;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_MM_FREEATTRIBUTE;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_MM_GETATTRIBUTE;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_MM_INIT;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_MM_LISTATTRIBUTE;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_MM_RETRIEVE;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_MM_SETATTRIBUTE;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_MM_SHUTDOWN;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_MM_STOREASYNC;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_MM_STOREBYREF;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_MPXCONFIG;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_MPXGETMUXPARAM;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_MS_GETGPRSATTACHSTATUS;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_MS_GETPLMNCODEBYNAME;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_MS_GETPLMNENTRYBYINDEX;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_MS_GETPLMNFORMAT;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_MS_GETPLMNLISTSIZE;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_MS_GETPLMNMCC;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_MS_GETPLMNMNC;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_MS_GETPLMNMODE;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_MS_GETPLMNNAMEBYCODE;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_MS_ISGPRSREGISTERED;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_MS_ISGSMREGISTERED;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_MS_ISPLMNFORBIDDEN;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_MS_PLMNSELECT;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_MS_SEARCHAVAILABLEPLMN;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_MS_SENDCOMBINEDATTACHREQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_MS_SENDDETACHREQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_MS_SETCURRENTPLMN;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_MS_SETPLMNFORMAT;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_MS_SETPLMNMODE;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_MS_SETPOWERDOWNTIMER;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_MS_SETSTARTBAND;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_PBK_GETALPHA;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_PBK_ISEMERGENCYCALLNUMBER;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_PBK_ISREADY;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_PBK_SENDFINDALPHAMATCHMULTIPLEREQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_PBK_SENDFINDALPHAMATCHONEREQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_PBK_SENDINFOREQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_PBK_SENDISNUMDIALLABLEREQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_PBK_SENDREADENTRYREQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_PBK_SENDWRITEENTRYREQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_PDP_ACTIVATESNDCPCONNECTION;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_PDP_DEACTIVATESNDCPCONNECTION;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_PDP_GETDEFINEDPDPCONTEXTCIDLIST;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_PDP_GETGPRSACTIVATESTATUS;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_PDP_GETGPRSMINQOS;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_PDP_GETGPRSMTPDPAUTORSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_PDP_GETGPRSQOS;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_PDP_GETMSCLASS;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_PDP_GETPDPADDRESS;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_PDP_GETPDPCONTEXT;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_PDP_SENDMTPDPACTIVATIONRSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_PDP_SENDPDPACTIVATEREQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_PDP_SENDPDPDEACTIVATEREQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_PDP_SETGPRSMINQOS;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_PDP_SETGPRSMTPDPAUTORSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_PDP_SETGPRSQOS;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_PDP_SETMSCLASS;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_PDP_SETPDPCONTEXT;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_RECV;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_RTC_ALARMREGISTER;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_RTC_CACULWEEKDAY;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_RTC_CHECKTIME;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_RTC_DISABLEALARM;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_RTC_DISABLEMINUTEINT;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_RTC_ENABLEALARM;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_RTC_FIREALARM;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_RTC_ENABLEMINUTEINT;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_RTC_GETALARMTIME;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_RTC_GETCOUNT;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_RTC_GETTIME;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_RTC_OFFSETTODATE;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_RTC_SETALARMTIME;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_RTC_SETTIME;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_RTC_TIMEREGISTER;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SATKCMDRESP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SELECT;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SEND;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SENDPPPCLOSEREQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SENDPPPOPENREQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SENDPPPPDU;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SENDSMSREQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SENDSMSSRVCENTERNUMBERUPDATEREQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SENDSNPDUREQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SERIAL_CLOSEDEVICE;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SERIAL_OPENDEVICE;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SERIAL_READ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SERIAL_REGISTEREVENTHANDLER;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SERIAL_WRITE;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SETSMSBEARERPREFERENCE;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SETSOCKOPT;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SETV24DCD;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SETV24OPERATIONMODEUINT8;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SETV24RI;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SHUTDOWN;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SIM_GETCARDPHASE;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SIM_GETCPHSPHASE;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SIM_GETHOMEPLMN;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SIM_GETIMSI;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SIM_GETPINSTATUS;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SIM_GETPRESENTSTATUS;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SIM_GETSIMTYPE;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SIM_GETSMSSCA;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SIM_GETSMSSTATUS;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SIM_GETSMSTOTALNUMBER;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SIM_ISCACHEDDATAREADY;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SIM_ISOPERATIONRESTRICTED;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SIM_ISPIN2VERIFIED;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SIM_ISPINBLOCKED;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SIM_ISPINOK;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SIM_ISPINREQUIRED;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SIM_ISPINVERIFIED;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SIM_SENDBINARYEFILEREADREQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SIM_SENDBINARYEFILEUPDATEREQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SIM_SENDCHANGECHVREQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SIM_SENDCYCLICEFILEUPDATEREQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SIM_SENDEFILEINFOREQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SIM_SENDINCREASEACMREQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SIM_SENDLINEAREFILEUPDATEREQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SIM_SENDREADACMMAXREQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SIM_SENDREADACMREQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SIM_SENDREADPUCTREQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SIM_SENDREADSVCPROVNAMEREQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SIM_SENDRECORDEFILEREADREQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SIM_SENDREMAININGPINATTEMPTREQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SIM_SENDSETCHV1ONOFFREQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SIM_SENDSETOPERSTATEREQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SIM_SENDUNBLOCKCHVREQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SIM_SENDVERIFYCHVREQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SIM_SENDWHOLEBINARYEFILEREADREQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SIM_SENDWRITEACMMAXREQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SIM_SENDWRITEACMREQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SIM_SENDWRITEPUCTREQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SIMLOCKCHANGEPHONELOCKPASSWORD;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SIMLOCKCHECKALLLOCKS;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SIMLOCKGETCURRENTCLOSEDLOCK;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SIMLOCKISLOCKON;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SIMLOCKSETDEFAULTPHONELOCK;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SIMLOCKSETLOCK;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SIMLOCKUNLOCKSIM;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SLEEP_ALLOCID;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SLEEP_CONFIGDEEPSLEEP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SLEEP_DISABLEDEEPSLEEP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SLEEP_ENABLEDEEPSLEEP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SLEEP_INIT;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SMS_SEND_SMS_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SMS_SEND_SMS_PDU_REQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SMS_SEND_ACK_TO_NETWORK;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SOCKET;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SOCKETCLOSE;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SS_ENDUSSDCONNECTREQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SS_QUERYCALLBARRINGSTATUS;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SS_QUERYCALLFORWARDSTATUS;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SS_QUERYCALLINGLINEIDSTATUS;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SS_QUERYCALLINGLINERESTRICTIONSTATUS;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SS_QUERYCALLWAITINGSTATUS;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SS_QUERYCONNECTEDLINEIDSTATUS;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SS_QUERYCONNECTEDLINERESTRICTIONSTATUS;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SS_SENDCALLBARRINGPWDCHANGEREQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SS_SENDCALLBARRINGREQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SS_SENDCALLFORWARDREQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SS_SENDCALLWAITINGREQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SS_SENDUSSDCONNECTREQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SS_SENDUSSDDATA;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SYS_CLIENTINIT;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SYS_CLIENTRUN;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SYS_DEREGISTERFORMSEVENT;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SYS_GETRSSITHRESHOLD;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SYS_GETRXSIGNALINFO;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SYS_ISBANDSUPPORTED;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SYS_PROCESSNORFREQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SYS_PROCESSPOWERDOWNREQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SYS_PROCESSPOWERUPREQ;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SYS_REGISTERFORMSEVENT;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SYS_SELECTBAND;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SYS_SETRSSITHRESHOLD;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_V24REGISTERCALLBACK;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_V24SEND;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_CALLBACKMSG;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_CC_REPORTCALLMETERVAL;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SIM_GET_SMSMEMEXC_FLAG;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SIM_IS_TEST_SIM;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SIMULATOR_SIM_UPDATE_SMSCAPEXC_FLAG;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_TEST_FIRST;

typedef PDPTestCreateAccount_t *T_INTER_TASK_MSG_PDP_TEST_CREATE_DATA_ACCT_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_TEST_CREATE_DATA_ACCT_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_TEST_CREATE_DATA_ACCT_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_TEST_CREATE_DATA_ACCT_HEADER Param1;
} yPDef_MSG_PDP_TEST_CREATE_DATA_ACCT;

typedef PDPTestBasic_t *T_INTER_TASK_MSG_PDP_TEST_SETUP_CONN_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_TEST_SETUP_CONN_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_TEST_SETUP_CONN_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_TEST_SETUP_CONN_HEADER Param1;
} yPDef_MSG_PDP_TEST_SETUP_CONN;

typedef PDPTestBasic_t *T_INTER_TASK_MSG_PDP_TEST_SHUTDOWN_CONN_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_TEST_SHUTDOWN_CONN_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_TEST_SHUTDOWN_CONN_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_TEST_SHUTDOWN_CONN_HEADER Param1;
} yPDef_MSG_PDP_TEST_SHUTDOWN_CONN;

typedef PDPTestBasic_t *T_INTER_TASK_MSG_PDP_TEST_ISSecPDP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_TEST_ISSecPDP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_TEST_ISSecPDP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_TEST_ISSecPDP_HEADER Param1;
} yPDef_MSG_PDP_TEST_ISSecPDP;

typedef PDPTestBasic_t *T_INTER_TASK_MSG_PDP_TEST_GETPrimCid_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_TEST_GETPrimCid_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_TEST_GETPrimCid_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_TEST_GETPrimCid_HEADER Param1;
} yPDef_MSG_PDP_TEST_GETPrimCid;

typedef PDPTestBasic_t *T_INTER_TASK_MSG_PDP_TEST_GETTFT_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_TEST_GETTFT_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_TEST_GETTFT_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_TEST_GETTFT_HEADER Param1;
} yPDef_MSG_PDP_TEST_GETTFT;

typedef PDPSetTFT_t *T_INTER_TASK_MSG_PDP_TEST_SETTFT_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_TEST_SETTFT_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_TEST_SETTFT_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_TEST_SETTFT_HEADER Param1;
} yPDef_MSG_PDP_TEST_SETTFT;

typedef PDPTestSck_t *T_INTER_TASK_MSG_PDP_TEST_SCK_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_TEST_SCK_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_TEST_SCK_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_TEST_SCK_HEADER Param1;
} yPDef_MSG_PDP_TEST_SCK;

typedef PDPTestInjectData_t *T_INTER_TASK_MSG_PDP_TEST_INJECT_DATA_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PDP_TEST_INJECT_DATA_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PDP_TEST_INJECT_DATA_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PDP_TEST_INJECT_DATA_HEADER Param1;
} yPDef_MSG_PDP_TEST_INJECT_DATA;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PDP_TEST_LAST;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PCHEX_TEST_FIRST;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PCHEX_TEST_STARTUP;

typedef IcmpPingInfo_t *T_INTER_TASK_MSG_PCHEX_TEST_ACTIVATEPDP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PCHEX_TEST_ACTIVATEPDP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PCHEX_TEST_ACTIVATEPDP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PCHEX_TEST_ACTIVATEPDP_HEADER Param1;
} yPDef_MSG_PCHEX_TEST_ACTIVATEPDP;

typedef UdpQuoteInfo_t *T_INTER_TASK_MSG_PCHEX_TEST_ICMPPING_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PCHEX_TEST_ICMPPING_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PCHEX_TEST_ICMPPING_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PCHEX_TEST_ICMPPING_HEADER Param1;
} yPDef_MSG_PCHEX_TEST_ICMPPING;

typedef UdpQuoteInfo_t *T_INTER_TASK_MSG_PCHEX_TEST_UDPQUOTE_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PCHEX_TEST_UDPQUOTE_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PCHEX_TEST_UDPQUOTE_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PCHEX_TEST_UDPQUOTE_HEADER Param1;
} yPDef_MSG_PCHEX_TEST_UDPQUOTE;

typedef PdpDeactInfo_t *T_INTER_TASK_MSG_PCHEX_TEST_DEACTIVATEPDP_PTR;
typedef struct {
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PCHEX_TEST_DEACTIVATEPDP_PTR	dataBuf;
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
} T_INTER_TASK_MSG_PCHEX_TEST_DEACTIVATEPDP_HEADER;
typedef struct {
	SIGNAL_VARS
	T_INTER_TASK_MSG_PCHEX_TEST_DEACTIVATEPDP_HEADER Param1;
} yPDef_MSG_PCHEX_TEST_DEACTIVATEPDP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PCHEX_TEST_SHUTDOWN;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PCHEX_TEST_PINGTIMER;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_PCHEX_TEST_LAST;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SCRIPT_AT_CMD;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SCRIPT_AT_CMD_RSP;

typedef struct {
	SIGNAL_VARS
	InterTaskMsg_t Param1;
} yPDef_MSG_SCRIPT_STATUS;

