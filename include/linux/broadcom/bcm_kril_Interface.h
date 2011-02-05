/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	include/linux/broadcom/bcm_kril_Interface.h
*
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

#ifndef _BCM_KRIL_INTERFACE_H
#define _BCM_KRIL_INTERFACE_H

#define  BCM_KRIL_CLIENT    0xFF
#define  BCM_URIL_CLIENT    0
#define  BCM_AUDIO_CLIENT   1
#define  BCM_POWER_CLIENT   2
#define  BCM_AT_CLIENT      3
#define  BCM_AT_URIL_CLIENT      4	/* used to identify AT requests passed through URIL */

#define  TOTAL_BCMDEVICE_NUM   4

#define RIL_SUPL_HMAC_LEN                                    8	/* LCS_SUPL_HMAC_LEN */
#define RIL_SUPL_NOTIFICATION_ITEM_LEN                       50	/* LCS_SUPL_NOTIFICATION_ITEM_LEN */
#define RIL_SUPL_WCHAR_NOTIFY_ITEM_LEN                       (RIL_SUPL_NOTIFICATION_ITEM_LEN*8/7 + 1)

/* ----------------------------------------------------------------------------- */
/* @doc EXTERNAL */
/* @constants Notification Class | Notification classes */
/* @comm None */
/* ----------------------------------------------------------------------------- */

/* #define RIL_NOTIFY_FUNCRESULT                       (0x00000000)      API call results */
/* #define RIL_NOTIFY_CALLCTRL                         (0x00010000)      Call control notifications */
/* #define RIL_NOTIFY_MESSAGE                          (0x00020000)      Messaging notifications */
/* #define RIL_NOTIFY_NETWORK                          (0x00040000)      Network-related notifications */
/* #define RIL_NOTIFY_SUPSERVICE                       (0x00080000)      Supplementary service notifications */
/* #define RIL_NOTIFY_PHONEBOOK                        (0x00100000)      Phonebook notifications */
/* #define RIL_NOTIFY_SIMTOOLKIT                       (0x00200000)      SIM Toolkit notifications */
/* #define RIL_NOTIFY_MISC                             (0x00400000)      Miscellaneous notifications */
/* #define RIL_NOTIFY_RADIOSTATE                       (0x00800000)      Notifications Pertaining to changes in Radio State */
#define RIL_NOTIFY_DEVSPECIFIC                      (0x80000000)	/* Reserved for device specific notifications */
/* #define RIL_NOTIFY_ALL                              (0x00FF0000)      All notification classes (except DevSpecifc) */

/* ----------------------------------------------------------------------------- */
/* RIL_DEVSPECIFICPARAM_XXX : (IOCtrl value | RIL_DEVSPECIFICPARAM classes) */
/* define all RIL_DEVSPECIFIC_BCM_CLASS */
/* --------------------------------------------------------------------------------------------------------------- */
#define RIL_DEVSPECIFIC_BCM_CLASS_APPLICATION                (0x00001000)	/* for Application use */
#define RIL_DEVSPECIFIC_BCM_CLASS_DRIVER                     (0x00002000)	/* for Driver use */
/* #define RIL_DEVSPECIFIC_BCM_CLASS_CP                         (0x00004000)      for CP use */
/* #define RIL_DEVSPECIFIC_BCM_CLASS_Reserve                    (0x00008000)      Windows Mobile Reserve */
/* --------------------------------------------------------------------------------------------------------------- */

/* define for AP class */
/* ---------------------------------------------------------------------------------------------------------------- */
/* #define RIL_DEVSPECIFIC_AP_VT                                (0x00000100 | RIL_DEVSPECIFIC_BCM_CLASS_APPLICATION) */
/* #define RIL_DEVSPECIFIC_AP_BANDSWITCH                        (0x00000200 | RIL_DEVSPECIFIC_BCM_CLASS_APPLICATION) */
#define RIL_DEVSPECIFIC_AP_ATCMDTOOL                         (0x00000300 | RIL_DEVSPECIFIC_BCM_CLASS_APPLICATION)
/* #define RIL_DEVSPECIFIC_AP_TTY                               (0x00000400 | RIL_DEVSPECIFIC_BCM_CLASS_APPLICATION) */
/* #define RIL_DEVSPECIFIC_AP_TEST                              (0x00000500 | RIL_DEVSPECIFIC_BCM_CLASS_APPLICATION) */
/* #define RIL_DEVSPECIFIC_AP_SIMTKIT                           (0x00000600 | RIL_DEVSPECIFIC_BCM_CLASS_APPLICATION) */
/* ---------------------------------------------------------------------------------------------------------------- */

/* define for Driver class */
/* ---------------------------------------------------------------------------------------------------------------- */
#define RIL_DEVSPECIFIC_DRIVER_PMU                           (0x00000100 | RIL_DEVSPECIFIC_BCM_CLASS_DRIVER)
/* #define RIL_DEVSPECIFIC_DRIVER_IPC                           (0x00000200 | RIL_DEVSPECIFIC_BCM_CLASS_DRIVER) */
/* #define RIL_DEVSPECIFIC_DRIVER_CAMERA                        (0x00000300 | RIL_DEVSPECIFIC_BCM_CLASS_DRIVER) */
/* #define RIL_DEVSPECIFIC_DRIVER_AUDIO                         (0x00000400 | RIL_DEVSPECIFIC_BCM_CLASS_DRIVER) */
/* #define RIL_DEVSPECIFIC_DRIVER_AT_PARSER                     (0x00000500 | RIL_DEVSPECIFIC_BCM_CLASS_DRIVER) */
/* ---------------------------------------------------------------------------------------------------------------- */

/* define for CP class */
/* ---------------------------------------------------------------------------------------------------------------- */
/* #define RIL_DEVSPECIFIC_CP_SUPL                              (0x00000100 | RIL_DEVSPECIFIC_BCM_CLASS_CP) */
/* #define RIL_DEVSPECIFIC_CP_BASEBAND                          (0x00000200 | RIL_DEVSPECIFIC_BCM_CLASS_CP) */
/* ---------------------------------------------------------------------------------------------------------------- */

/* IO control for Application */
/* ---------------------------------------------------------------------------------------------------------------- */
/* #define RIL_DEVSPECIFICPARAM_BCM_VT_GET_INCOMING_CALL_NUM          (0x00000024 | RIL_DEVSPECIFIC_AP_VT) */
/* #define RIL_DEVSPECIFICPARAM_GET_SUPPORTED_BAND                    (0x00000001 | RIL_DEVSPECIFIC_AP_BANDSWITCH) */
/* #define RIL_DEVSPECIFICPARAM_GET_SYSTEM_BAND                       (0x00000002 | RIL_DEVSPECIFIC_AP_BANDSWITCH) */
/* #define RIL_DEVSPECIFICPARAM_SET_BAND                              (0x00000003 | RIL_DEVSPECIFIC_AP_BANDSWITCH) */
/* #define RIL_DEVSPECIFICPARAM_GET_SUPPORTED_RAT                     (0x00000004 | RIL_DEVSPECIFIC_AP_BANDSWITCH) */
/* #define RIL_DEVSPECIFICPARAM_GET_SYSTEM_RAT                        (0x00000005 | RIL_DEVSPECIFIC_AP_BANDSWITCH) */
/* #define RIL_DEVSPECIFICPARAM_SET_SUPPORTED_RAT_BAND                (0x00000006 | RIL_DEVSPECIFIC_AP_BANDSWITCH) */
/* #define RIL_DEVSPECIFICPARAM_SET_ATCMD_MOD                         (0x00000001 | RIL_DEVSPECIFIC_AP_ATCMDTOOL) */
#define RIL_DEVSPECIFICPARAM_AT_PROCESS_CMD                        (0x00000002 | RIL_DEVSPECIFIC_AP_ATCMDTOOL)
/* #define RIL_DEVSPECIFICPARAM_QUERY_TTY                             (0x00000001 | RIL_DEVSPECIFIC_AP_TTY) */
/* #define RIL_DEVSPECIFICPARAM_SET_TTY                               (0x00000002 | RIL_DEVSPECIFIC_AP_TTY) */
/* #define RIL_DEVSPECIFICPARAM_SHAREMEMORYCOPY                       (0x00000001 | RIL_DEVSPECIFIC_AP_TEST) */
/* #define RIL_DEVSPECIFICPARAM_TRIGGERSIMTKITEVENT                   (0x00000002 | RIL_DEVSPECIFIC_AP_TEST) */
/* #define RIL_DEVSPECIFICPARAM_QUERY_PIN_REMAINING                   (0x00000003 | RIL_DEVSPECIFIC_AP_TEST) */
/* #define RIL_DEVSPECIFICPARAM_SIMTKIT_INIT                          (0x00000001 | RIL_DEVSPECIFIC_AP_SIMTKIT) */
/* ---------------------------------------------------------------------------------------------------------------- */

/* IO control for Driver */
/* ---------------------------------------------------------------------------------------------------------------- */

/* Map to CAPI2_PMU_BattADCReq, ResultData: None. The notification will be sent via */
/* MSG_PMU_BATT_LEVEL_IND with HAL_EM_BatteryLevel_t as payload */
#define RIL_DEVSPECIFICPARAM_BCM_PMU_GET_BATT_ADC                  (0x00000001 | RIL_DEVSPECIFIC_DRIVER_PMU)

/* Map to CAPI2_SYSPARM_GetActualLowVoltReading, ResultData: UInt16 (Low Volt reading) */
#define RIL_DEVSPECIFICPARAM_BCM_PMU_GET_ACTUAL_LOW_VOLT           (0x00000002 | RIL_DEVSPECIFIC_DRIVER_PMU)

/* #define RIL_DEVSPECIFICPARAM_BCM_PMU_GET_ACTUAL_4P2_VOLT           (0x00000003 | RIL_DEVSPECIFIC_DRIVER_PMU) */

/* Map to CAPI2_SYSPARM_GetBattLowThresh,  ResultData: ::UInt16 (Low Volt reading) */
#define RIL_DEVSPECIFICPARAM_BCM_PMU_GET_BATT_LOW_THRESH           (0x00000004 | RIL_DEVSPECIFIC_DRIVER_PMU)

/* #define RIL_DEVSPECIFICPARAM_BCM_PMU_GET_DEFAULT_4P2_VOLT          (0x00000005 | RIL_DEVSPECIFIC_DRIVER_PMU) */
/* #define RIL_DEVSPECIFICPARAM_BCM_PMU_SET_BATTERY_REGISTER          (0x00000006 | RIL_DEVSPECIFIC_DRIVER_PMU) */

/* #define RIL_DEVSPECIFICPARAM_BCM_IPC_CPCRASHED_IND                 (0x00000001 | RIL_DEVSPECIFIC_DRIVER_IPC) */
/* #define RIL_DEVSPECIFICPARAM_BCM_IPC_SET_AP_ASSERT                 (0x00000002 | RIL_DEVSPECIFIC_DRIVER_IPC) */

/* #define RIL_DEVSPECIFICPARAM_BCM_SET_WCDMA_SLEEP_MODE              (0x00000001 | RIL_DEVSPECIFIC_DRIVER_CAMERA) */

/* #define RIL_DEVSPECIFICPARAM_BCM_AUDIO_STATE_IND                   (0x00000001 | RIL_DEVSPECIFIC_DRIVER_AUDIO) */
/* #define RIL_DEVSPECIFICPARAM_BCM_AUDIO_IGNORE_IND                  (0x00000002 | RIL_DEVSPECIFIC_DRIVER_AUDIO) */
/* #define RIL_DEVSPECIFICPARAM_BCM_AUDIO_UNHOLD_IND                  (0x00000003 | RIL_DEVSPECIFIC_DRIVER_AUDIO) */
/* #define RIL_DEVSPECIFICPARAM_BCM_AUDIO_ANSWERCLL_IND               (0x00000004 | RIL_DEVSPECIFIC_DRIVER_AUDIO) */
/* #define RIL_DEVSPECIFICPARAM_BCM_AUDIO_MAKECALL_IND                (0x00000005 | RIL_DEVSPECIFIC_DRIVER_AUDIO) */
/* #define RIL_DEVSPECIFICPARAM_BCM_AUDIO_GET_SYSPARAM_REQ            (0x00000006 | RIL_DEVSPECIFIC_DRIVER_AUDIO) */

/* Map to CAPI2_program_equalizer, ResultData: None. */
/*#define RIL_DEVSPECIFICPARAM_BCM_AUDIO_SET_EQUALIZER_REQ           (0x00000007 | RIL_DEVSPECIFIC_DRIVER_AUDIO) */

/* Map to CAPI2_program_poly_equalizer, ResultData: None. */
/*#define RIL_DEVSPECIFICPARAM_BCM_AUDIO_SET_POLY_EQUALIZER_REQ      (0x00000008 | RIL_DEVSPECIFIC_DRIVER_AUDIO) */

/* Map to CAPI2_AUDIO_ASIC_SetAudioMode, ResultData: None. */
/*#define RIL_DEVSPECIFICPARAM_BCM_AUDIO_ASIC_SET_AUDIO_MODE         (0x00000009 | RIL_DEVSPECIFIC_DRIVER_AUDIO) */

/* Map to CAPI2_AUDIO_Turn_EC_NS_OnOff, ResultData: None. */
/*#define RIL_DEVSPECIFICPARAM_BCM_TURN_ECNS_ONOFF                   (0x0000000A | RIL_DEVSPECIFIC_DRIVER_AUDIO) */

/* Map to CAPI2_RIPCMDQ_Connect_Uplink, ResultData: None. */
/*#define RIL_DEVSPECIFICPARAM_BCM_RIPCMDQ_CONNECT_UPLINK            (0x0000000B | RIL_DEVSPECIFIC_DRIVER_AUDIO) */

/* Map to CAPI2_RIPCMDQ_Connect_Downlink, ResultData: None. */
/*#define RIL_DEVSPECIFICPARAM_BCM_RIPCMDQ_CONNECT_DOWNLINK          (0x0000000C | RIL_DEVSPECIFIC_DRIVER_AUDIO) */

/* Map to CAPI2_audio_control_dsp, payload is ::UInt32 */
/*#define RIL_DEVSPECIFICPARAM_BCM_AUDIO_CONTROL_DSP                 (0x0000000D | RIL_DEVSPECIFIC_DRIVER_AUDIO) */

/* Map to CAPI2_program_FIR_IIR_filter, payload is ::UInt32 */
/*#define RIL_DEVSPECIFICPARAM_BCM_PROGRAM_FIR_IIR_FILTER             (0x0000000E | RIL_DEVSPECIFIC_DRIVER_AUDIO) */

/* Map to CAPI2_program_poly_FIR_IIR_filter, payload is ::UInt32 */
/*#define RIL_DEVSPECIFICPARAM_BCM_PROGRAM_POLY_FIR_IIR_FILTER        (0x0000000F | RIL_DEVSPECIFIC_DRIVER_AUDIO) */

/* Map to CAPI2_VOLUMECTRL_SetBasebandVolume */
/*#define RIL_DEVSPECIFICPARAM_BCM_VOLUMECTRL_SETBASEBANDVOLUME        (0x00000010 | RIL_DEVSPECIFIC_DRIVER_AUDIO) */

/* Map to CAPI2_audio_control_generic, payload is ::structure */
/*#define RIL_DEVSPECIFICPARAM_BCM_AUDIO_CONTROL_GENERIC				   (0x00000011 | RIL_DEVSPECIFIC_DRIVER_AUDIO) */

/* #define RIL_DEVSPECIFICPARAM_BCM_AT_PARSER_MS_SET_EBASE_REQ        (0x00000001 | RIL_DEVSPECIFIC_DRIVER_AT_PARSER) */
/* #define RIL_DEVSPECIFICPARAM_BCM_AT_PARSER_MS_GET_EBASE_REQ        (0x00000002 | RIL_DEVSPECIFIC_DRIVER_AT_PARSER) */
/* ---------------------------------------------------------------------------------------------------------------- */

/* IO control for CP */
/* ---------------------------------------------------------------------------------------------------------------- */
/* #define RIL_DEVSPECIFICPARAM_SUPL_CONN_REQ                   (0x00000001 | RIL_DEVSPECIFIC_CP_SUPL) */
/* #define RIL_DEVSPECIFICPARAM_SUPL_CONN_RSP                   (0x00000002 | RIL_DEVSPECIFIC_CP_SUPL) */
/* #define RIL_DEVSPECIFICPARAM_SUPL_WRITE_REQ                  (0x00000003 | RIL_DEVSPECIFIC_CP_SUPL) */
/* #define RIL_DEVSPECIFICPARAM_SUPL_DATA_AVAIL                 (0x00000004 | RIL_DEVSPECIFIC_CP_SUPL) */
/* #define RIL_DEVSPECIFICPARAM_SUPL_DISCONN_REQ                (0x00000005 | RIL_DEVSPECIFIC_CP_SUPL) */
/* #define RIL_DEVSPECIFICPARAM_SUPL_INITHMAC_REQ               (0x00000006 | RIL_DEVSPECIFIC_CP_SUPL) */
/* #define RIL_DEVSPECIFICPARAM_SUPL_INITHMAC_RSP               (0x00000007 | RIL_DEVSPECIFIC_CP_SUPL) */
/* #define RIL_DEVSPECIFICPARAM_SUPL_NOTIFY_IND                 (0x00000008 | RIL_DEVSPECIFIC_CP_SUPL) */
/* #define RIL_DEVSPECIFICPARAM_SUPL_VERIFY_RSP                 (0x00000009 | RIL_DEVSPECIFIC_CP_SUPL) */
/* #define RIL_DEVSPECIFICPARAM_SUPL_SERVER_DISCONN             (0x0000000a | RIL_DEVSPECIFIC_CP_SUPL) */
/* #define RIL_DEVSPECIFICPARAM_LCS_SERVICE_CONTROL             (0x0000000b | RIL_DEVSPECIFIC_CP_SUPL) */

/* #define RIL_DEVSPECIFICPARAM_BCM_SET_BASEBAND_VOL            (0x00000001 | RIL_DEVSPECIFIC_CP_BASEBAND) */
/* ---------------------------------------------------------------------------------------------------------------- */

/* BCM defined notification */
/* ---------------------------------------------------------------------------------------------------------------- */
/* #define RIL_NOTIFY_SUPL_CONN_REQ                             (RIL_DEVSPECIFICPARAM_SUPL_CONN_REQ | RIL_NOTIFY_DEVSPECIFIC) */
/* #define RIL_NOTIFY_SUPL_WRITE_REQ                            (RIL_DEVSPECIFICPARAM_SUPL_WRITE_REQ | RIL_NOTIFY_DEVSPECIFIC) */
/* #define RIL_NOTIFY_SUPL_DISCONN_REQ                          (RIL_DEVSPECIFICPARAM_SUPL_DISCONN_REQ | RIL_NOTIFY_DEVSPECIFIC) */
/* #define RIL_NOTIFY_SUPL_INITHMAC_REQ                         (RIL_DEVSPECIFICPARAM_SUPL_INITHMAC_REQ | RIL_NOTIFY_DEVSPECIFIC) */
/* #define RIL_NOTIFY_SUPL_NOTIFY_IND                           (RIL_DEVSPECIFICPARAM_SUPL_NOTIFY_IND | RIL_NOTIFY_DEVSPECIFIC) */

/* For Audio */
/* #define RIL_NOTIFY_AUDIO_CALLSTATE_IND                       (RIL_DEVSPECIFICPARAM_BCM_AUDIO_STATE_IND | RIL_NOTIFY_DEVSPECIFIC) */
/* #define RIL_NOTIFY_AUDIO_IGNORE_IND                          (RIL_DEVSPECIFICPARAM_BCM_AUDIO_IGNORE_IND | RIL_NOTIFY_DEVSPECIFIC) */
/* #define RIL_NOTIFY_AUDIO_UNHOLD_IND                          (RIL_DEVSPECIFICPARAM_BCM_AUDIO_UNHOLD_IND | RIL_NOTIFY_DEVSPECIFIC) */
/* #define RIL_NOTIFY_AUDIO_ANSWERCLL_IND                       (RIL_DEVSPECIFICPARAM_BCM_AUDIO_ANSWERCLL_IND | RIL_NOTIFY_DEVSPECIFIC) */
/* #define RIL_NOTIFY_AUDIO_MAKECALL_IND                        (RIL_DEVSPECIFICPARAM_BCM_AUDIO_MAKECALL_IND | RIL_NOTIFY_DEVSPECIFIC) */

/* For ATCMD Tool */
/* Map to MSG_CAPI2_AT_RESPONSE_IND, payload is ::AtResponse_t */
#define RIL_NOTIFY_DEVSPECIFIC_ATCMD_RESP                    (RIL_DEVSPECIFICPARAM_AT_PROCESS_CMD | RIL_NOTIFY_DEVSPECIFIC)

/* For Battery */
/* Map to MSG_PMU_BATT_LEVEL_IND, payload is ::HAL_EM_BatteryLevel_t */
#define RIL_NOTIFY_DEVSPECIFIC_BATT_LEVEL                    (RIL_DEVSPECIFICPARAM_BCM_PMU_GET_BATT_ADC | RIL_NOTIFY_DEVSPECIFIC)

/* For VT */
/* #define RIL_NOTIFY_INCOMINGVIDEOCALL                         (0x0000000F | RIL_NOTIFY_CALLCTRL)  @constdefine CPI notification; lpData points to RILCALLINFO */
/* #define RIL_NOTIFY_VIDEOCALLCONNECTING                       (0x00000010 | RIL_NOTIFY_CALLCTRL)  @constdefine CPI notification; lpData points to RILCALLINFO */

/* For STK call control result */
/* #define RIL_NOTIFY_VOICECALL_CONNECTED_IND                   (0x00000005 | RIL_NOTIFY_SIMTOOLKIT)  Voice call connected */
/* #define RIL_NOTIFY_VOICECALL_RELEASE_IND                     (0x00000006 | RIL_NOTIFY_SIMTOOLKIT)  Voice call release */
/* #define RIL_NOTIFY_CC_END_CALL_SUCCESS                       (0x00000007 | RIL_NOTIFY_SIMTOOLKIT)  End call success */
/* #define RIL_NOTIFY_CC_END_CALL_FAIL                          (0x00000008 | RIL_NOTIFY_SIMTOOLKIT)  End call fail */
/* #define RIL_NOTIFY_CC_HOLD_CALL_SUCCESS                      (0x00000009 | RIL_NOTIFY_SIMTOOLKIT)  Hold call success */
/* #define RIL_NOTIFY_CC_HOLD_CALL_FAIL                         (0x0000000A | RIL_NOTIFY_SIMTOOLKIT)  Hold call fail */
/* #define RIL_NOTIFY_STK_CC_SETUPFAIL_IND                      (0x0000000B | RIL_NOTIFY_SIMTOOLKIT)  STK setup call failed */
/* #define RIL_NOTIFY_STK_CC_DISPLAY_IND                        (0x0000000C | RIL_NOTIFY_SIMTOOLKIT)  STK setup call need to dispaly information */

/* For STK responses to proactive SIM commands */
/* #define SIM_RESPONSE_ERR_USERBUSY                            (0x00000040) */
/* #define SIM_RESPONSE_ERR_BUSYONCALL                          (0x00000041) */
/* #define SIM_RESPONSE_ERR_INTERACTWITHSIMCCTEMPPROBLEM        (0x00000042) */
/* #define SIM_RESPONSE_ERR_INTERACTWITHSIMCCPERMPROBLEM        (0x00000043) */

#define RIL_CLIENTID_INDEX(CLIENT_ID) (CLIENT_ID - 1)

typedef void (RILResponseCallbackFunc) (UInt32 CmdID,
					int result,
					void *dataBuf, UInt32 dataLength);

typedef void (RILNotifyCallbackFunc) (UInt32 msgType,
				      int result,
				      void *dataBuf, UInt32 dataLength);

typedef struct {
	int registered;
	RILResponseCallbackFunc *resultCb;
	RILNotifyCallbackFunc *notifyCb;
	UInt32 *notifyid_list;
	int notifyid_list_len;
} bcm_kril_dev_result_t;

typedef struct {
	UInt8 chan;		/* /<    at channel */
	UInt8 ATCmd;		/* /<    starting byte of at command to be processed, NULL ended */
} bcm_kril_at_cmd_req_t;

/* ****************************************************************************** */
/* Function Name: KRIL_Register */
/* Description  : For BCM device driver register */
/* PARAMETERS   : clientID      Device driver client ID */
/*              : resultCb      Function callback for receiving CAPI2 response */
/*              : notifyCb      Function callback for receiving unsolicited */
/*                              notifications */
/*              : notifyid_list The notify ID list that device driver want */
/*                              to receive */
/*              : notifyid_list_len The number of notify ID in this list. */
/* RETURN       : 1 register successfully ; 0 register failed */
/* Notes: */
/* ****************************************************************************** */
int KRIL_Register(UInt32 clientID,
		  RILResponseCallbackFunc resultCb,
		  RILNotifyCallbackFunc notifyCb,
		  UInt32 *notifyid_list, int notifyid_list_len);

/* ****************************************************************************** */
/* Function Name: KRIL_DevSpecific_Cmd */
/* Description  : For BCM device driver implement CAPI2 command */
/* PARAMETERS   : clientID      Device driver client ID. */
/*              : CmdID         The device specific command ID for CAPI2. */
/*              : data          Command data buffer point . */
/*              : datalen       Command data length. */
/* RETURN       : 1 register successfully ; 0 register failed */
/* Notes: */
/* ****************************************************************************** */
int KRIL_DevSpecific_Cmd(unsigned short client, UInt32 CmdID, void *data,
			 size_t datalen);

#endif /* _BCM_KRIL_INTERFACE_H */
