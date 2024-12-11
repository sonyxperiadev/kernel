/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyrights (C) 2021 Silicon Mitus, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __SM5038_PD_H__
#define __SM5038_PD_H__

#define USBPD_REV_20	(1)
#define USBPD_REV_30	(2)
#define PD_SID		(0xFF00)
#define PD_SID_1	(0xFF01)

#define MAX_INPUT_DATA (255)
#define MAX_CHARGING_VOLT		9000 /* 9V */
#define USBPD_VOLT_UNIT			50 /* 50mV */
#define USBPD_CURRENT_UNIT		10 /* 10mA */
#define USBPD_PPS_VOLT_UNIT		100	/* 100mV */
#define USBPD_OUT_VOLT_UNIT		20	/* 20mV */
#define USBPD_PPS_CURRENT_UNIT	50	/* 50mV */

#define USBPD_MAX_COUNT_MSG_OBJECT	(8) /* 0..7 */
#define USBPD_MAX_COUNT_RX_PAYLOAD	(28)

/* Counter */
#define USBPD_nMessageIDCount		(7)
#define USBPD_nRetryCount		(3)
#define USBPD_nHardResetCount		(2)
#define USBPD_nCapsCount		(30)
#define USBPD_nDiscoverIdentityCount	(20)

/* Timer */
#define tSrcTransition			(35)	/* 25~35 ms */
#define tPSSourceOn				(440)	/* 390~480 ms */
#define tPSSourceOff			(760)	/* 750~960 ms */
#define tSenderResponse			(27)	/* 24~30 ms */
#define tSendSourceCap			(150)	/* 100 ~ 200 ms */
#define tPSHardReset			(25)	/* 25~35 ms */
#define tSinkWaitCap			(550)	/* 310~620 ms  */
#define tPSTransition			(490)	/* 450~550 ms */
#define tVCONNSourceOff			(25)	/* 25 ms */
#define tVCONNSourceOn			(10)	/* ~50 ms */
#define tVCONNSourceTimeout 	(100)	/* 100~200 ms */
#define tVDMSenderResponse		(30)	/* 24~30 ms */
#define tVDMWaitModeEntry		(50)	/* 40~50  ms */
#define tVDMWaitModeExit		(50)    /* 40~50  ms */
#define tDiscoverIdentity		(50)	/* 40~50  ms */
#define tSwapSourceStart		(20)	/* 20  ms */
#define tSwapSinkReady			(10)	/* 10 ms */
#define tSrcRecover				(670)	/* 660~1000 ms */

typedef enum {
	POWER_TYPE_FIXED = 0,
	POWER_TYPE_BATTERY,
	POWER_TYPE_VARIABLE,
	POWER_TYPE_APDO,
} pdo_supply_type;

typedef enum {
	SOP_TYPE_SOP,
	SOP_TYPE_SOP1,
	SOP_TYPE_SOP2,
	SOP_TYPE_SOP1_DEBUG,
	SOP_TYPE_SOP2_DEBUG
} sop_type;

enum usbpd_check_mode {
	MODE_MSG = 0,
	MODE_CMD = 1,
};

enum usbpd_bist_mode {
	BIST_Carrier_Mode2	= 0x5,
	BIST_Test_Mode		= 0x8
};

enum usbpd_data_role {
	USBPD_UFP,
	USBPD_DFP,
};

enum usbpd_power_role {
	USBPD_SINK,
	USBPD_SOURCE,
};

enum usbpd_control_msg_type {
	USBPD_GoodCRC			= 0x1,
	USBPD_GotoMin			= 0x2,
	USBPD_Accept			= 0x3,
	USBPD_Reject			= 0x4,
	USBPD_Ping				= 0x5,
	USBPD_PS_RDY			= 0x6,
	USBPD_Get_Source_Cap	= 0x7,
	USBPD_Get_Sink_Cap		= 0x8,
	USBPD_DR_Swap			= 0x9,
	USBPD_PR_Swap			= 0xA,
	USBPD_VCONN_Swap		= 0xB,
	USBPD_Wait				= 0xC,
	USBPD_Soft_Reset		= 0xD,
	USBPD_UVDM_MSG			= 0xE,
	USBPD_Not_Supported		= 0x10,
	USBPD_Get_Src_Cap_Ext	= 0x11,
	USBPD_Get_Status		= 0x12,
	USBPD_FR_Swap			= 0x13,
	USBPD_Get_PPS_Status	= 0x14,
	USBPD_Get_Country_Codes	= 0x15,
	USBPD_Get_Sink_Cap_Ext	= 0x16,
};

enum usbpd_extended_msg_type {
	USBPD_Source_Cap_Ext	= 0x1,
	USBPD_Status			= 0x2,
	USBPD_Get_Battery_Cap	= 0x3,
	USBPD_Get_Batt_Status	= 0x4,
	USBPD_Battery_Cap		= 0x5,
	USBPD_Get_Manuf_Info	= 0x6,
	USBPD_Manuf_Info		= 0x7,
	USBPD_Security_Request	= 0x8,
	USBPD_Security_Response	= 0x9,
	USBPD_FW_Update_Req		= 0xA,
	USBPD_FW_Update_Res		= 0xB,
	USBPD_PPS_Status		= 0xC,
	USBPD_Country_Info		= 0xD,
	USBPD_Country_Codes		= 0xE,
	USBPD_Sink_Cap_Ext		= 0xF,
};

enum usbpd_check_msg_pass {
	NONE_CHECK_MSG_PASS,
	CHECK_MSG_PASS,
};

enum usbpd_port_role {
	USBPD_Rp	= 0x01,
	USBPD_Rd	= 0x01 << 1,
	USBPD_Ra	= 0x01 << 2,
};

enum vdm_port_capability {
	Reserved_Capable	= 0,
	UFP_D_Capable		= 1,
	DFP_D_Capable		= 2,
	DFP_D_and_UFP_D_Capable	= 3
};

enum vdm_port_connected {
	Adaptor_Disable		= 0,
	Connect_DFP_D		= 1,
	Connect_UFP_D		= 2,
	Connect_DFP_D_and_UFP_D	= 3
};

enum vdm_receptacle_indication {
	USB_TYPE_C_PLUG			= 0,
	USB_TYPE_C_Receptacle	= 1
};

enum vdm_command_type {
	Initiator		= 0,
	Responder_ACK	= 1,
	Responder_NAK	= 2,
	Responder_BUSY	= 3
};

enum vdm_type {
	Unstructured_VDM = 0,
	Structured_VDM = 1
};

enum vdm_configure_type {
	USB				= 0,
	USB_U_AS_DFP_D	= 1,
	USB_U_AS_UFP_D	= 2
};

enum vdm_displayport_protocol {
	UNSPECIFIED	= 0,
	DP_V_1_3	= 1,
	GEN_2		= 1 << 1
};

enum dp_pinconf{
	PDIC_NOTIFY_DP_PIN_UNKNOWN	= 0,
	PDIC_NOTIFY_DP_PIN_A		= 1,
	PDIC_NOTIFY_DP_PIN_B		= 2,
	PDIC_NOTIFY_DP_PIN_C		= 3,
	PDIC_NOTIFY_DP_PIN_D		= 4,
	PDIC_NOTIFY_DP_PIN_E		= 5,
	PDIC_NOTIFY_DP_PIN_F		= 6,
};

enum vdm_pin_assignment {
	DE_SELECT_PIN		= 0,
	PIN_ASSIGNMENT_A	= 1,
	PIN_ASSIGNMENT_B	= 1 << 1,
	PIN_ASSIGNMENT_C	= 1 << 2,
	PIN_ASSIGNMENT_D	= 1 << 3,
	PIN_ASSIGNMENT_E	= 1 << 4,
	PIN_ASSIGNMENT_F	= 1 << 5,
};

enum vdm_command_msg {
	Discover_Identity			= 1,
	Discover_SVIDs				= 2,
	Discover_Modes				= 3,
	Enter_Mode					= 4,
	Exit_Mode					= 5,
	Attention					= 6,
	DisplayPort_Status_Update	= 0x10,
	DisplayPort_Configure		= 0x11,
};

enum usbpd_data_msg_type {
	USBPD_Source_Capabilities	= 0x1,
	USBPD_Request				= 0x2,
	USBPD_BIST					= 0x3,
	USBPD_Sink_Capabilities		= 0x4,
	USBPD_Battery_Status		= 0x5,
	USBPD_Alert					= 0x6,
	USBPD_Get_Country_Info		= 0x7,
	USBPD_Vendor_Defined		= 0xF,
};

enum alert_type {
	Battery_Status_Change		= 1 << 1,
	OCP_event					= 1 << 2, /* Source only, for Sink Reserved and Shall be set to zero */
	OTP_event					= 1 << 3,
	Operating_Condition_Change	= 1 << 4,
	Source_Input_Change_Event	= 1 << 5,
	OVP_event					= 1 << 6,
};

/* Policy Engine States */
typedef enum {
	/* SRC_VDM */
	PE_SRC_VDM_Identity_Request	= 20,
	PE_SRC_VDM_Identity_ACKed	= 21,
	PE_SRC_VDM_Identity_NAKed	= 22,

	/* Source */
	PE_SRC_Startup			= 0x30,
	PE_SRC_Discovery		= 0x31,
	PE_SRC_Send_Capabilities	= 0x32,
	PE_SRC_Negotiate_Capability	= 0x33,
	PE_SRC_Transition_Supply	= 0x34,
	PE_SRC_Ready			= 0x35,
	PE_SRC_Disabled			= 0x36,
	PE_SRC_Capability_Response	= 0x37,
	PE_SRC_Hard_Reset		= 0x38,
	PE_SRC_Hard_Reset_Received	= 0x39,
	PE_SRC_Transition_to_default	= 0x3A,
	PE_SRC_Give_Source_Cap		= 0x3B,
	PE_SRC_Get_Sink_Cap		= 0x3C,
	PE_SRC_Wait_New_Capabilities	= 0x3D,
	PE_SRC_Give_Sink_Cap		= 0x3E,

	/* Sink */
	PE_SNK_Startup			= 0x40,
	PE_SNK_Discovery		= 0x41,
	PE_SNK_Wait_for_Capabilities	= 0x42,
	PE_SNK_Evaluate_Capability	= 0x43,
	PE_SNK_Select_Capability	= 0x44,
	PE_SNK_Transition_Sink		= 0x45,
	PE_SNK_Ready			= 0x46,
	PE_SNK_Hard_Reset		= 0x47,
	PE_SNK_Transition_to_default	= 0x48,
	PE_SNK_Give_Sink_Cap		= 0x49,
	PE_SNK_Get_Source_Cap		= 0x4A,
	PE_SNK_Give_Source_Cap		= 0x4B,

	/* Source Soft Reset */
	PE_SRC_Send_Soft_Reset		= 0x50,
	PE_SRC_Soft_Reset		= 0x51,

	/* Sink Soft Reset */
	PE_SNK_Send_Soft_Reset		= 0x60,
	PE_SNK_Soft_Reset		= 0x61,

	/* UFP VDM */
	PE_UFP_VDM_Get_Identity		= 0x70,
	PE_UFP_VDM_Send_Identity	= 0x71,
	PE_UFP_VDM_Get_Identity_NAK	= 0x72,
	PE_UFP_VDM_Get_SVIDs		= 0x73,
	PE_UFP_VDM_Send_SVIDs		= 0x74,
	PE_UFP_VDM_Get_SVIDs_NAK	= 0x75,
	PE_UFP_VDM_Get_Modes		= 0x76,
	PE_UFP_VDM_Send_Modes		= 0x77,
	PE_UFP_VDM_Get_Modes_NAK	= 0x78,
	PE_UFP_VDM_Evaluate_Mode_Entry	= 0x79,
	PE_UFP_VDM_Mode_Entry_ACK	= 0x7A,
	PE_UFP_VDM_Mode_Entry_NAK	= 0x7B,
	PE_UFP_VDM_Mode_Exit		= 0x7C,
	PE_UFP_VDM_Mode_Exit_ACK	= 0x7D,
	PE_UFP_VDM_Mode_Exit_NAK	= 0x7E,
	PE_UFP_VDM_Attention_Request	= 0x7F,
	PE_UFP_VDM_Evaluate_Status	= 0x80,
	PE_UFP_VDM_Status_ACK		= 0x81,
	PE_UFP_VDM_Status_NAK		= 0x82,
	PE_UFP_VDM_Evaluate_Configure	= 0x83,
	PE_UFP_VDM_Configure_ACK	= 0x84,
	PE_UFP_VDM_Configure_NAK	= 0x85,

	/* DFP VDM */
	PE_DFP_VDM_Identity_Request		= 0x8A,
	PE_DFP_VDM_Identity_ACKed		= 0x8B,
	PE_DFP_VDM_Identity_NAKed		= 0x8C,
	PE_DFP_VDM_SVIDs_Request		= 0x8D,
	PE_DFP_VDM_SVIDs_ACKed			= 0x8E,
	PE_DFP_VDM_SVIDs_NAKed			= 0x8F,
	PE_DFP_VDM_Modes_Request		= 0x90,
	PE_DFP_VDM_Modes_ACKed			= 0x91,
	PE_DFP_VDM_Modes_NAKed			= 0x92,
	PE_DFP_VDM_Mode_Entry_Request		= 0x93,
	PE_DFP_VDM_Mode_Entry_ACKed		= 0x94,
	PE_DFP_VDM_Mode_Entry_NAKed		= 0x95,
	PE_DFP_VDM_Mode_Exit_Request		= 0x96,
	PE_DFP_VDM_Mode_Exit_ACKed		= 0x97,
	PE_DFP_VDM_Mode_Exit_NAKed		= 0x98,
	PE_DFP_VDM_Status_Update		= 0x99,
	PE_DFP_VDM_Status_Update_ACKed		= 0x9A,
	PE_DFP_VDM_Status_Update_NAKed		= 0x9B,
	PE_DFP_VDM_DisplayPort_Configure	= 0x9C,
	PE_DFP_VDM_DisplayPort_Configure_ACKed	= 0x9D,
	PE_DFP_VDM_DisplayPort_Configure_NAKed	= 0x9E,
	PE_DFP_VDM_Attention_Request		= 0x9F,

	/* Power Role Swap */
	PE_PRS_SRC_SNK_Reject_PR_Swap	= 0xA0,
	PE_PRS_SRC_SNK_Evaluate_Swap	= 0xA1,
	PE_PRS_SRC_SNK_Send_Swap	= 0xA2,
	PE_PRS_SRC_SNK_Accept_Swap	= 0xA3,
	PE_PRS_SRC_SNK_Transition_off	= 0xA4,
	PE_PRS_SRC_SNK_Assert_Rd	= 0xA5,
	PE_PRS_SRC_SNK_Wait_Source_on	= 0xA6,
	PE_PRS_SNK_SRC_Reject_Swap	= 0xA7,
	PE_PRS_SNK_SRC_Evaluate_Swap	= 0xA8,
	PE_PRS_SNK_SRC_Send_Swap	= 0xA9,
	PE_PRS_SNK_SRC_Accept_Swap	= 0xAA,
	PE_PRS_SNK_SRC_Transition_off	= 0xAB,
	PE_PRS_SNK_SRC_Assert_Rp	= 0xAC,
	PE_PRS_SNK_SRC_Source_on	= 0xAD,

	/* Data Role Swap */
	PE_DRS_DFP_UFP_Evaluate_DR_Swap	= 0xAE,
	PE_DRS_DFP_UFP_Accept_DR_Swap	= 0xAF,
	PE_DRS_DFP_UFP_Change_to_UFP	= 0xB0,
	PE_DRS_DFP_UFP_Send_DR_Swap	= 0xB1,
	PE_DRS_DFP_UFP_Reject_DR_Swap	= 0xB2,
	PE_DRS_UFP_DFP_Evaluate_DR_Swap	= 0xB3,
	PE_DRS_UFP_DFP_Accept_DR_Swap	= 0xB4,
	PE_DRS_UFP_DFP_Change_to_DFP	= 0xB5,
	PE_DRS_UFP_DFP_Send_DR_Swap	= 0xB6,
	PE_DRS_UFP_DFP_Reject_DR_Swap	= 0xB7,
	PE_DRS_Evaluate_Port		= 0xB8,
	PE_DRS_Evaluate_Send_Port	= 0xB9,

	/* Vconn Source Swap */
	PE_VCS_Evaluate_Swap		= 0xC0,
	PE_VCS_Accept_Swap		= 0xC1,
	PE_VCS_Wait_for_VCONN		= 0xC2,
	PE_VCS_Turn_Off_VCONN		= 0xC3,
	PE_VCS_Turn_On_VCONN		= 0xC4,
	PE_VCS_Send_PS_RDY		= 0xC5,
	PE_VCS_Send_Swap		= 0xC6,
	PE_VCS_Reject_VCONN_Swap	= 0xC7,

	/* UVDM Message */
	PE_DFP_UVDM_Send_Message	= 0xD0,
	PE_DFP_UVDM_Receive_Message	= 0xD1,

	PE_SNK_Get_Source_Cap_Ext		= 0xD2,
	PE_SNK_Get_Source_Status		= 0xD3,
	PE_SNK_Get_Source_PPS_Status	= 0xD4,

	/* BIST Message */
	PE_BIST_CARRIER_M2		= 0xE0,

	Error_Recovery			= 0xFF
} policy_state;

typedef enum sm5038_usbpd_manager_command {
	MANAGER_REQ_GET_SNKCAP					= 1,
	MANAGER_REQ_GOTOMIN						= 2,
	MANAGER_REQ_SRCCAP_CHANGE				= 3,
	MANAGER_REQ_PR_SWAP						= 4,
	MANAGER_REQ_DR_SWAP						= 5,
	MANAGER_REQ_VCONN_SWAP					= 6,
	MANAGER_REQ_VDM_DISCOVER_IDENTITY		= 7,
	MANAGER_REQ_VDM_DISCOVER_SVID			= 8,
	MANAGER_REQ_VDM_DISCOVER_MODE			= 9,
	MANAGER_REQ_VDM_ENTER_MODE				= 10,
	MANAGER_REQ_VDM_EXIT_MODE				= 11,
	MANAGER_REQ_VDM_ATTENTION				= 12,
	MANAGER_REQ_VDM_STATUS_UPDATE			= 13,
	MANAGER_REQ_VDM_DisplayPort_Configure	= 14,
	MANAGER_REQ_NEW_POWER_SRC				= 15,
	MANAGER_REQ_UVDM_SEND_MESSAGE			= 16,
} sm5038_usbpd_manager_command_type;

typedef enum sm5038_usbpd_manager_event {
	MANAGER_DISCOVER_IDENTITY_ACKED			= 0,
	MANAGER_DISCOVER_IDENTITY_NAKED			= 1,
	MANAGER_DISCOVER_SVID_ACKED				= 2,
	MANAGER_DISCOVER_SVID_NAKED				= 3,
	MANAGER_DISCOVER_MODE_ACKED				= 4,
	MANAGER_DISCOVER_MODE_NAKED				= 5,
	MANAGER_ENTER_MODE_ACKED				= 6,
	MANAGER_ENTER_MODE_NAKED				= 7,
	MANAGER_EXIT_MODE_ACKED					= 8,
	MANAGER_EXIT_MODE_NAKED					= 9,
	MANAGER_ATTENTION_REQUEST				= 10,
	MANAGER_STATUS_UPDATE_ACKED				= 11,
	MANAGER_STATUS_UPDATE_NAKED				= 12,
	MANAGER_DisplayPort_Configure_ACKED		= 13,
	MANAGER_DisplayPort_Configure_NACKED	= 14,
	MANAGER_NEW_POWER_SRC					= 15,
	MANAGER_UVDM_SEND_MESSAGE				= 16,
	MANAGER_UVDM_RECEIVE_MESSAGE			= 17,
	MANAGER_PR_SWAP_REQUEST 				= 18,
	MANAGER_DR_SWAP_REQUEST 				= 19,
	MANAGER_SEND_DISCOVER_IDENTITY			= 20,
} sm5038_usbpd_manager_event_type;

enum usbpd_msg_status {
	MSG_GOODCRC				= 1<<0,
	MSG_ACCEPT				= 1<<1,
	MSG_PSRDY				= 1<<2,
	MSG_REQUEST				= 1<<3,
	MSG_REJECT				= 1<<4,
	MSG_WAIT				= 1<<5,
	MSG_ERROR				= 1<<6,
	MSG_PING				= 1<<7,
	MSG_GET_SNK_CAP			= 1<<8,
	MSG_GET_SRC_CAP			= 1<<9,
	MSG_SNK_CAP				= 1<<10,
	MSG_SRC_CAP				= 1<<11,
	MSG_PR_SWAP				= 1<<12,
	MSG_DR_SWAP				= 1<<13,
	MSG_VCONN_SWAP			= 1<<14,
	VDM_DISCOVER_IDENTITY	= 1<<15,
	VDM_DISCOVER_SVID		= 1<<16,
	VDM_DISCOVER_MODE		= 1<<17,
	VDM_ENTER_MODE			= 1<<18,
	VDM_EXIT_MODE			= 1<<19,
	VDM_ATTENTION			= 1<<20,
	VDM_DP_STATUS_UPDATE	= 1<<21,
	VDM_DP_CONFIGURE		= 1<<22,
	MSG_SOFTRESET			= 1<<23,
	PLUG_DETACH				= 1<<24,
	PLUG_ATTACH				= 1<<25,
	MSG_HARDRESET			= 1<<26,
	MSG_BIST_M2				= 1<<27,
	UVDM_MSG				= 1<<28,
	MSG_PASS				= 1<<29,
	MSG_RID					= 1<<30,
	MSG_NONE				= 1<<31,
	MSG_GET_BAT_STATUS		= 3<<2,
	MSG_GET_STATUS			= 3<<3,
	MSG_GET_PPS_STATUS		= 3<<4,
	MSG_GET_SRC_CAP_EXT		= 3<<5,
};

/* Timer */
enum usbpd_timer_id {
	DISCOVER_IDENTITY_TIMER		= 1,
	HARD_RESET_COMPLETE_TIMER	= 2,
	NO_RESPONSE_TIMER			= 3,
	PS_HARD_RESET_TIMER			= 4,
	PS_SOURCE_OFF_TIMER			= 5,
	PS_SOURCE_ON_TIMER			= 6,
	PS_TRANSITION_TIMER			= 7,
	SENDER_RESPONSE_TIMER		= 8,
	SINK_ACTIVITY_TIMER			= 9,
	SINK_REQUEST_TIMER			= 10,
	SINK_WAIT_CAP_TIMER			= 11,
	SOURCE_ACTIVITY_TIMER		= 12,
	SOURCE_CAPABILITY_TIMER		= 13,
	SWAP_RECOVERY_TIMER			= 14,
	SWAP_SOURCE_START_TIMER		= 15,
	VCONN_ON_TIMER				= 16,
	VDM_MODE_ENTRY_TIMER		= 17,
	VDM_MODE_EXIT_TIMER			= 18,
	VDM_RESPONSE_TIMER			= 19,
	USBPD_TIMER_MAX_COUNT
};

enum sm5038_usbpd_protocol_status {
	DEFAULT_PROTOCOL_NONE	= 0,
	MESSAGE_SENT			= 1,
	TRANSMISSION_ERROR		= 2
};

enum sm5038_usbpd_policy_informed {
	DEFAULT_POLICY_NONE	= 0,
	HARDRESET_RECEIVED	= 1,
	SOFTRESET_RECEIVED	= 2,
	PLUG_EVENT			= 3,
	PLUG_ATTACHED		= 4,
	PLUG_DETACHED		= 5,
};

typedef union {
	u32 object;
	u16 word[2];
	u8  byte[4];

	struct {
		unsigned:30;
		unsigned supply_type:2;
	} power_data_obj_supply_type;

	struct {
		unsigned max_current:10;        /* 10mA units */
		unsigned voltage:10;            /* 50mV units */
		unsigned peak_current:2;
		unsigned reserved:3;
		unsigned data_role_swap:1;
		unsigned usb_comm_capable:1;
		unsigned externally_powered:1;
		unsigned usb_suspend_support:1;
		unsigned dual_role_power:1;
		unsigned supply:2;
	} power_data_obj;

	struct {
		unsigned op_current:10;	/* 10mA units */
		unsigned voltage:10;	/* 50mV units */
		unsigned reserved:5;
		unsigned data_role_swap:1;
		unsigned usb_comm_capable:1;
		unsigned externally_powered:1;
		unsigned higher_capability:1;
		unsigned dual_role_power:1;
		unsigned supply_type:2;
	} power_data_obj_sink;

	struct {
		unsigned max_current:10;	/* 10mA units */
		unsigned min_voltage:10;	/* 50mV units */
		unsigned max_voltage:10;	/* 50mV units */
		unsigned supply_type:2;
	} power_data_obj_variable;

	struct {
		unsigned max_power:10;		/* 250mW units */
		unsigned min_voltage:10;	/* 50mV units  */
		unsigned max_voltage:10;	/* 50mV units  */
		unsigned supply_type:2;
	} power_data_obj_battery;

	struct {
		unsigned max_current:7;		/* 50mA units */
		unsigned reserved1:1;
		unsigned min_voltage:8;		/* 100mV units  */
		unsigned reserved2:1;
		unsigned max_voltage:8;		/* 100mV units  */
		unsigned reserved3:2;
		unsigned pps_power_limited:1;
		unsigned pps_supply:2;
		unsigned supply_type:2;
	} power_data_obj_programmable;

	struct {
		unsigned op_current:7;		/* 50mA units */
		unsigned reserved1:2;
		unsigned output_voltage:11;	/* 20mV units */
		unsigned reserved2:3;
		unsigned unchunked_ext_msg_support:1;
		unsigned no_usb_suspend:1;
		unsigned usb_comm_capable:1;
		unsigned capability_mismatch:1;
		unsigned reserved3:1;
		unsigned object_position:3;
		unsigned reserved4:1;
	} request_data_object_programmable;

	struct {
		unsigned reserved:16;
		unsigned hot_swappable_batteries:4;
		unsigned fixed_batteries:4;
		unsigned type_of_alert:8;
	} alert_data_obejct;

	struct {
		unsigned min_current:10;	/* 10mA units */
		unsigned op_current:10;		/* 10mA units */
		unsigned:4;
		unsigned no_usb_suspend:1;
		unsigned usb_comm_capable:1;
		unsigned capability_mismatch:1;
		unsigned give_back:1;
		unsigned object_position:3;
		unsigned:1;
	} request_data_object;

	struct {
		unsigned max_power:10;		/* 250mW units */
		unsigned op_power:10;		/* 250mW units */
		unsigned:4;
		unsigned no_usb_suspend:1;
		unsigned usb_comm_capable:1;
		unsigned capability_mismatch:1;
		unsigned give_back:1;
		unsigned object_position:3;
		unsigned:1;
	} request_data_object_battery;

	struct {
		unsigned vendor_defined:15;
		unsigned vdm_type:1;
		unsigned vendor_id:16;
	} unstructured_vdm;

	struct {
		unsigned command:5;
		unsigned reserved1:1;
		unsigned command_type:2;
		unsigned obj_pos:3;
		unsigned reserved2:2;
		unsigned version:2;
		unsigned vdm_type:1;
		unsigned svid:16;
	} structured_vdm;

	struct {
		unsigned usb_vendor_id:16;
		unsigned reserved:10;
		unsigned modal_operation_supported:1;
		unsigned product_type:3;
		unsigned data_capable_usb_device:1;
		unsigned data_capable_usb_host:1;
	} id_header;

	struct {
		unsigned cert_test_id:20;
		unsigned reserved:12;
	} cert_stat_vdo;

	struct {
		unsigned device_version:16;
		unsigned product_id:16;
	} product_vdo;

	struct {
		unsigned port_capability:2;
		unsigned displayport_protocol:4;
		unsigned receptacle_indication:1;
		unsigned usb_r2_signaling:1;
		unsigned dfp_d_pin_assignments:8;
		unsigned ufp_d_pin_assignments:8;
		unsigned rsvd:8;
	} displayport_capabilities;

	struct {
		unsigned port_connected:2;
		unsigned power_low:1;
		unsigned enabled:1;
		unsigned multi_function_preferred:1;
		unsigned usb_configuration_request:1;
		unsigned exit_displayport_mode_request:1;
		unsigned hpd_state:1;
		unsigned irq_hpd:1;
		unsigned rsvd:23;
	} displayport_status;

	struct{
		unsigned select_configuration:2;
		unsigned displayport_protocol:4;
		unsigned rsvd1:2;
		unsigned ufp_u_pin_assignment:8;
		unsigned rsvd2:16;
	} displayport_configurations;

	struct{
		unsigned svid_1:16;
		unsigned svid_0:16;
	} vdm_svid;

	struct{
		unsigned reserved:28;
		unsigned bist_mode:4;
	} bist_data_object;

	struct{
		unsigned VID:16;
		unsigned PID:16;
	} source_capabilities_extended_data1;

	struct{
		unsigned XID:32;
	} source_capabilities_extended_data2;

	struct{
		unsigned fw_version:8;
		unsigned hw_version:8;
		unsigned voltage_regulation:8;
		unsigned holdup_time:8;
	} source_capabilities_extended_data3;

	struct{
		unsigned compliance:8;
		unsigned torch_current:8;
		unsigned peak_current1:16;
	} source_capabilities_extended_data4;

	struct{
		unsigned peak_current2:16;
		unsigned peak_current3:16;
	} source_capabilities_extended_data5;

	struct{
		unsigned touch_temp:8;
		unsigned source_inputs:8;
		unsigned number_battery:8;
		unsigned source_pdp_rating:8;
	} source_capabilities_extended_data6;

} data_obj_type;

typedef union {
	u16 word;
	u8  byte[2];

	struct {
		unsigned msg_type:5;
		unsigned port_data_role:1;
		unsigned spec_revision:2;
		unsigned port_power_role:1;
		unsigned msg_id:3;
		unsigned num_data_objs:3;
		unsigned extended:1;
	};
} msg_header;

typedef union {
	u16 word;
	u8  byte[2];

	struct {
		unsigned data_size:9;
		unsigned rsvd:1;
		unsigned request_chunk:1;
		unsigned chunk_number:4;
		unsigned chunked:1;
	};
} ext_msg_header_type;

typedef struct _pd_power_list {
	int max_voltage;
	int min_voltage;
	int max_current;
	int apdo;
} PD_POWER_LIST;

typedef struct usbpd_phy_ops {
	/*    1st param should be 'usbpd_data *'    */
	int    (*tx_msg)(void *, msg_header *, data_obj_type *);
	int    (*rx_msg)(void *, msg_header *, data_obj_type *);
	int    (*hard_reset)(void *);
	int    (*set_power_role)(void *, int);
	int    (*get_power_role)(void *, int *);
	int    (*set_data_role)(void *, int);
	int    (*get_data_role)(void *, int *);
	int    (*set_vconn_source)(void *, int);
	int    (*get_vconn_source)(void *, int *);
	int    (*set_check_msg_pass)(void *, int);
	unsigned int  (*get_status)(void *pd_data, unsigned int status);
	bool   (*poll_status)(void *, int);
	void   (*driver_reset)(void *);
	void   (*get_short_state)(void *, bool *);
} usbpd_phy_ops_type;

struct sm5038_policy_data {
	policy_state	state;
	policy_state	last_state;
	msg_header		tx_msg_header;
	msg_header		rx_msg_header;
	ext_msg_header_type	rx_msg_ext_header;
	data_obj_type	tx_data_obj[USBPD_MAX_COUNT_MSG_OBJECT];
	data_obj_type	rx_data_obj[USBPD_MAX_COUNT_MSG_OBJECT];
	bool			rx_hardreset;
	bool			rx_softreset;
	bool			plug;
	bool			plug_valid;
	bool			modal_operation;
	bool			abnormal_state;
	u8				origin_message;
};

struct sm5038_protocol_data {
	unsigned int		stored_message_id;
	msg_header			msg_header;
	data_obj_type		data_obj[USBPD_MAX_COUNT_MSG_OBJECT];
	unsigned int		status;
};

struct sm5038_usbpd_counter {
	unsigned int	retry_counter;
	unsigned int	message_id_counter;
	unsigned int	caps_counter;
	unsigned int	hard_reset_counter;
	unsigned int	discover_identity_counter;
	unsigned int	swap_hard_reset_counter;
};

struct sm5038_usbpd_manager_data {
	sm5038_usbpd_manager_command_type cmd;  /* request to policy engine */
	sm5038_usbpd_manager_event_type   event;    /* policy engine infromed */

	msg_header			uvdm_msg_header;
	data_obj_type		uvdm_data_obj[USBPD_MAX_COUNT_MSG_OBJECT];

	int origin_selected_pdo_num;
	int fled_torch_enable;
	int fled_flash_enable;

	/* power role swap*/
	bool power_role_swap;
	/* data role swap*/
	bool data_role_swap;
	bool vconn_source_swap;
	int alt_sended;
	int vdm_en;
	int ext_sended;
	uint16_t Standard_Vendor_ID;
	uint16_t Vendor_ID;
	uint16_t Product_ID;
	uint16_t Device_Version;
	uint16_t SVID_0;
	uint16_t SVID_1;
	uint16_t SVID_DP;
	u32 acc_type;
	u32 dp_selected_pin;
	uint32_t is_sent_pin_configuration;
	u32 dp_is_connect;
	u32 dp_hs_connect;
	u8 pin_assignment;
	struct sm5038_usbpd_data *pd_data;
	struct delayed_work start_discover_msg_handler;
	struct delayed_work	new_power_handler;
	int vbus_adc;
};

struct sm5038_usbpd_data {
	struct device		dev;
	void			*phy_driver_data;
	struct sm5038_usbpd_counter	counter;
	struct hrtimer		timers[USBPD_TIMER_MAX_COUNT];
	u32					received_pdos[7];
	data_obj_type		sink_rdo;
	unsigned int		expired_timers;
	usbpd_phy_ops_type	phy_ops;
	struct sm5038_protocol_data	protocol_tx;
	struct sm5038_protocol_data	protocol_rx;
	struct sm5038_policy_data	policy;
	msg_header			source_msg_header;
	data_obj_type		source_data_obj;
	data_obj_type		source_request_obj;
	struct sm5038_usbpd_manager_data	manager;
	struct work_struct	worker;
	struct completion	msg_arrived;
	struct completion	pd_completion;
	unsigned int            wait_for_msg_arrived;
	int			specification_revision;
	PD_POWER_LIST power_list[USBPD_MAX_COUNT_MSG_OBJECT];
	u16			src_cap_id;
	int			current_pdo_num;
	int			selected_pdo_num;
	int			available_pdo_num;
	int			request_pps_vol;
	int			request_pps_cur;
};

static inline struct sm5038_usbpd_data *protocol_rx_to_usbpd(
		struct sm5038_protocol_data *rx)
{
	return container_of(rx, struct sm5038_usbpd_data, protocol_rx);
}

static inline struct sm5038_usbpd_data *protocol_tx_to_usbpd(
		struct sm5038_protocol_data *tx)
{
	return container_of(tx, struct sm5038_usbpd_data, protocol_tx);
}

static inline struct sm5038_usbpd_data *policy_to_usbpd(
		struct sm5038_policy_data *policy)
{
	return container_of(policy, struct sm5038_usbpd_data, policy);
}

static inline struct sm5038_usbpd_data *manager_to_usbpd(
		struct sm5038_usbpd_manager_data *manager)
{
	return container_of(manager, struct sm5038_usbpd_data, manager);
}

extern void sm5038_usbpd_uevent_notifier(struct device *dev);
extern void sm5038_usbpd_class_unregister(struct sm5038_usbpd_data *pd_data);
extern void sm5038_usbpd_device_unregister(struct sm5038_usbpd_data *pd_data);
extern int sm5038_usbpd_init(struct device *dev, void *phy_driver_data);
extern void sm5038_usbpd_init_policy(struct sm5038_usbpd_data *pd_data);
extern void sm5038_usbpd_dp_detach(struct device *dev);
extern void sm5038_usbpd_acc_detach(struct device *dev);
extern int sm5038_usbpd_check_accessory(
		struct sm5038_usbpd_manager_data *manager);
extern void sm5038_usbpd_power_ready(struct device *dev,
	enum usbpd_power_role power_role);
extern int  sm5038_usbpd_match_request(struct sm5038_usbpd_data *pd_data);
extern bool sm5038_usbpd_power_role_swap(struct sm5038_usbpd_data *pd_data);
extern bool sm5038_usbpd_vconn_source_swap(struct sm5038_usbpd_data *pd_data);
extern void sm5038_usbpd_turn_on_source(struct sm5038_usbpd_data *pd_data);
extern void sm5038_usbpd_turn_off_power_supply(
		struct sm5038_usbpd_data *pd_data);
extern void sm5038_usbpd_turn_off_power_sink(
		struct sm5038_usbpd_data *pd_data);
extern bool sm5038_usbpd_data_role_swap(struct sm5038_usbpd_data *pd_data);
extern void sm5038_usbpd_inform_event(struct sm5038_usbpd_data *pd_data,
		sm5038_usbpd_manager_event_type event);
extern int sm5038_usbpd_evaluate_capability(struct sm5038_usbpd_data *pd_data);
extern data_obj_type sm5038_usbpd_select_capability(
		struct sm5038_usbpd_data *pd_data);
extern bool sm5038_usbpd_vdm_request_enabled(
		struct sm5038_usbpd_data *pd_data);
extern bool sm5038_usbpd_ext_request_enabled(
		struct sm5038_usbpd_data *pd_data);
extern bool sm5038_usbpd_dex_vdm_request(struct sm5038_usbpd_data *pd_data);
extern void sm5038_usbpd_set_rp_scr_sel(struct sm5038_usbpd_data *pd_data,
		int scr_sel);
extern void sm5038_usbpd_policy_work(struct work_struct *work_s);
extern void sm5038_usbpd_protocol_rx(struct sm5038_usbpd_data *pd_data);
extern void sm5038_usbpd_kick_policy_work(struct device *dev);
extern void sm5038_usbpd_rx_hard_reset(struct device *dev);
extern void sm5038_usbpd_rx_soft_reset(struct sm5038_usbpd_data *pd_data);
extern void sm5038_usbpd_policy_reset(struct sm5038_usbpd_data *pd_data,
		unsigned int flag);
extern void sm5038_usbpd_tx_request_discard(struct sm5038_usbpd_data *pd_data);
extern void sm5038_usbpd_set_ops(struct device *dev, usbpd_phy_ops_type *ops);
extern bool sm5038_usbpd_send_msg(struct sm5038_usbpd_data *pd_data,
		msg_header *h, data_obj_type *obj);
extern bool sm5038_usbpd_send_ctrl_msg(struct sm5038_usbpd_data *pd_data,
		msg_header *h, unsigned int msg, unsigned int dr,
		unsigned int pr);
extern unsigned int sm5038_usbpd_wait_msg(struct sm5038_usbpd_data *pd_data,
		unsigned int msg_status, unsigned int ms);
extern void sm5038_usbpd_reinit(struct device *dev);
extern void sm5038_usbpd_init_protocol(struct sm5038_usbpd_data *pd_data);
#if 0
extern int (*fp_sec_pd_get_apdo_max_power)(unsigned int *pdo_pos,
		unsigned int *taMaxVol, unsigned int *taMaxCur, unsigned int *taMaxPwr);
#endif
#endif
