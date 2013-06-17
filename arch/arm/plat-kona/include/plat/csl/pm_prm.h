/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*             @file     arch/arm/plat-kona/include/plat/csl/pm_prm.h
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
/**
*
* @file   pm_prm.h
* @brief  Power Resource Manager API
*
*****************************************************************************/

/**
*
* @defgroup PowerResourceManagementFrameworkGroup
*
* @brief    This group defines API for PRM
*
* @ingroup  PRMFGroup
*****************************************************************************/

#ifndef	_PM_PRM_H_
#define	_PM_PRM_H_

/**
*
* @addtogroup PowerResourceManagementFrameworkGroup
* @{
*/

#ifdef __cplusplus
extern "C" {
#endif

/**

API Summary

Initialization
==============
void       PRM_init(void);

Client Registration
===================
CLIENT_ID  PRM_client_register(char *client_name);
PRM_RETURN PRM_client_deregister(CLIENT_ID client_id);
  
Power and Clock Management for Peripheral Resource
==================================================
PRM_RETURN PRM_set_power_state(CLIENT_ID client_id, PERIPHERAL_ID resource_id, POWER_STATE state);
PRM_RETURN PRM_set_clock_state(CLIENT_ID client_id, PERIPHERAL_ID resource_id, CLOCK_STATE clock_state);
PRM_RETURN PRM_set_clock_speed(CLIENT_ID client_id, PERIPHERAL_ID resource_id, int freq);
PRM_RETURN PRM_set_clock_divider(CLIENT_ID client_id, PERIPHERAL_ID resource_id, int pll, int pre_divider_terminal, int divider_terminal, int fraction);
int        PRM_get_power_clock_state(CLIENT_ID client_id, PERIPHERAL_ID resource_id, POWER_CLOCK_STATE state);
PRM_RETURN PRM_request_bus(CLIENT_ID client_id, PERIPHERAL_ID resource_id, BUS_REQ req);
PRM_RETURN PRM_enable_event(CLIENT_ID client_id, WAKEUP_EVENT_ID event_id, EVENT_ENABLE enable);
PRM_RETURN PRM_register_wakeup_callbk(CLIENT_ID client_id, WAKEUP_EVENT_ID event_id, void (*func)(WAKEUP_EVENT_ID event_id));
PRM_RETURN PRM_deregister_wakeup_callbk(CLIENT_ID client_id, WAKEUP_EVENT_ID event_id, void (*func)(WAKEUP_EVENT_ID event_id));
PRM_RETURN PRM_set_forced_wakeup_latency(CLIENT_ID client_id, PERIPHERAL_ID id, int latency);

Operating Point Management for Performance Resource
===================================================
PRM_RETURN PRM_request_opp(CLIENT_ID client_id, PERFORMANCE_ID resource_id, int opp_int, void (*func)(PRM_RETURN ret));
PRM_RETURN PRM_force_opp(CLIENT_ID client_id, PERFORMANCE_ID resource_id, int opp_int, void (*func)(PRM_RETURN ret));
PRM_RETURN PRM_release_opp(CLIENT_ID client_id, PERFORMANCE_ID resource_id);
OPP_STATE  PRM_get_opp(CLIENT_ID client_id, PERFORMANCE_ID resource_id);
PRM_RETURN PRM_disable_opp_change(CLIENT_ID client_id, PERFORMANCE_ID resource_id);
PRM_RETURN PRM_enable_opp_change(CLIENT_ID client_id, PERFORMANCE_ID resource_id);
int        PRM_get_num_of_opp(CLIENT_ID client_id, PERFORMANCE_ID resource_id );
int        PRM_get_opp_speed(CLIENT_ID client_id, PERFORMANCE_ID resource_id, OPP_STATE opp);
PRM_RETURN PRM_opp_change_prepare_callbk_register(CLIENT_ID client_id, PERFORMANCE_ID resource_id, void (*func)(OPP_STATE opp));
PRM_RETURN PRM_opp_change_prepare_callbk_deregister(CLIENT_ID client_id, PERFORMANCE_ID resource_id, void (*func)(OPP_STATE opp));
PRM_RETURN PRM_opp_change_finish_callbk_register(CLIENT_ID client_id, PERFORMANCE_ID resource_id, void (*func)(OPP_STATE opp));
PRM_RETURN PRM_opp_change_finish_callbk_deregister(CLIENT_ID client_id, PERFORMANCE_ID resource_id, void (*func)(OPP_STATE opp));

Reset Services
==============
PRM_RETURN PRM_system_reset(CLIENT_ID client_id);
PRM_RETURN PRM_get_system_reset_reason(CLIENT_ID client_id, int *reason);
PRM_RETURN PRM_resource_reset(CLIENT_ID client_id, PERIPHERAL_ID resource_id);

Sleep Management for Performance Resource
=========================================
PRM_RETURN PRM_allowed_wakeup_latency(CLIENT_ID client_id, PERFORMANCE_ID resource_id, int latency);
int        PRM_get_wakeup_latency(CLIENT_ID client_id, PERFORMANCE_ID resource_id, SLEEP_STATE state);
PRM_RETURN PRM_allowed_sleep_state(CLIENT_ID client_id, PERFORMANCE_ID resource_id, SLEEP_STATE state);
PRM_RETURN PRM_force_sleep_state(CLIENT_ID client_id, PERFORMANCE_ID resource_id, SLEEP_STATE state);
SLEEP_STATE PRM_get_sleep_state(CLIENT_ID client_id, PERFORMANCE_ID resource_id);
PRM_RETURN PRM_sleep_state_change_callbk_register(CLIENT_ID client_id, PERFORMANCE_ID resource_id, SLEEP_STATE state, void (*func)(SLEEP_STATE state));
PRM_RETURN PRM_sleep_state_change_callbk_deregister(CLIENT_ID client_id, PERFORMANCE_ID resource_id, SLEEP_STATE state, void (*func)(SLEEP_STATE state));

Debug Services
=================
PRM_RETURN PRM_debug (PRM_DEBUG type, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7)
**/

// ** Performance ID enums for COMMS and APPS **
#ifdef FUSE_COMMS_PROCESSOR

// COMMS Performance resource IDs 
	typedef enum {
		RESOURCE_BMODEM,	// BMODEM CCU
		RESOURCE_WCDMA,	// WCDMA CCU, for sleep control only           
		RESOURCE_DSP,	// DSP CCU
		NUM_PERF_RESOURCE
	} PERFORMANCE_ID;

#else

// APPS Performance resource IDs
	typedef enum {
		RESOURCE_SYSTEM,	// whole system, for sleep testing only
		RESOURCE_BUS,	// Frabric/Slave CCUs on Hera             
		RESOURCE_HUB,	// HubAON/Hub CCUs on Hera             
		RESOURCE_VIDEO,	// Multimedia CCU
		RESOURCE_AUDIO,	// Not applicable on Hera
		RESOURCE_MEMORY,	// Not applicable on Hera
		RESOURCE_CPU,	// Cortex A9 CCU
		NUM_PERF_RESOURCE
	} PERFORMANCE_ID;

#endif				// #ifdef FUSE_COMMS_PROCESSOR

// ** Resource ID enums for both COMMS and APPS **
#ifdef FUSE_COMMS_PROCESSOR

// Peripherals Resource IDs for COMMS
	typedef enum {
		// peripheral clocks
		RESOURCE_GEA_PCLK,
		RESOURCE_WCDMA_CIPHER,
		RESOURCE_WCDMA_DATAPACKER,
		RESOURCE_WCDMA_HUCM,
		RESOURCE_EDGE_MP,
		RESOURCE_RFBB_52M,
		RESOURCE_SCLKCAL,
		RESOURCE_SCLKCAL_23G,
		RESOURCE_WCDMA_CLK,
		RESOURCE_WCDMA_26M,
		RESOURCE_WCDMA_32K,

		RESOURCE_DSP_CLK,
		RESOURCE_TRANSCEIVER_26M,
		RESOURCE_TRANSCEIVER_52M,

		// UART control is by standalone CP image only
		RESOURCE_UARTB,
		RESOURCE_UARTB2,
		RESOURCE_UARTB3,

		NUM_PERI_RESOURCE
	} PERIPHERAL_ID;

#else

// Resource IDs for APPS
	typedef enum {
		// peripheral clocks
		RESOURCE_GPIOKP,
		RESOURCE_PMU_BSC,
		RESOURCE_SIM,
		RESOURCE_SIM2,
		RESOURCE_ACI,
		RESOURCE_HSI_TX,
		RESOURCE_HSI_RX,
		RESOURCE_MDIOMASTER,
		RESOURCE_BT_SLIM,
		RESOURCE_SSP3,
		RESOURCE_SSP3_AUDIO,
#if defined(_RHEA_)
		RESOURCE_SSP4,
		RESOURCE_SSP4_AUDIO,
		RESOURCE_CAPH,
#endif
		RESOURCE_AUDIOH_2P4M,	// audio_2p4m clock
		RESOURCE_AUDIOH,	// audio_26m clock
		RESOURCE_AUDIOH_156M,	// audio_156m clock
		RESOURCE_TMON,
		RESOURCE_SPUM_SEC,
		RESOURCE_SPUM_OPEN,
		RESOURCE_SSP0,
		RESOURCE_SSP0_AUDIO,
#if defined(_HERA_)
		RESOURCE_SSP1,
		RESOURCE_SSP1_AUDIO,
#endif
		RESOURCE_UARTB,
		RESOURCE_UARTB2,
		RESOURCE_UARTB3,
		RESOURCE_PWM,
		RESOURCE_BSC1,
		RESOURCE_BSC2,
		RESOURCE_DMA,
		RESOURCE_SDIO1,
		RESOURCE_SDIO2,
		RESOURCE_SDIO3,
		RESOURCE_USB_IC,
		RESOURCE_USB_OTG,
		RESOURCE_DSI0,
		RESOURCE_DSI1,
		RESOURCE_DSI0_AXI,
		RESOURCE_DSI1_AXI,
		RESOURCE_SPI,
		RESOURCE_CSI0,
		RESOURCE_CSI1,
		RESOURCE_CSI0_AXI,
		RESOURCE_CSI1_AXI,
		RESOURCE_CSI0_CAMPIX,
		RESOURCE_CSI0_BYTE1,
		RESOURCE_CSI0_BYTE0,
		RESOURCE_CSI1_CAMPIX,
		RESOURCE_CSI1_BYTE1,
		RESOURCE_CSI1_BYTE0,
		RESOURCE_SMI,
		RESOURCE_V3D,
		RESOURCE_ISP,
		RESOURCE_VCE,
		RESOURCE_MM_DMA,
		RESOURCE_DSI_PLL,
		RESOURCE_DIGI_CH0,
		RESOURCE_DIGI_CH1,
		RESOURCE_DIGI_CH2,
		RESOURCE_DIGI_CH3,

		// peripheral clock users
		RESOURCE_CAMERA1,
		RESOURCE_CAMERA2,
		RESOURCE_DISPLAY1,
		RESOURCE_DISPLAY2,
		RESOURCE_BT,
		RESOURCE_FM,
		RESOURCE_GPS,
		RESOURCE_WLAN,
		RESOURCE_HEADSET,
		RESOURCE_MIC,
		RESOURCE_LOGGING,
		RESOURCE_AT,
		RESOURCE_HSOTG,
		RESOURCE_FSHOST,
		RESOURCE_HSHOST,
		RESOURCE_LIGHT,
		RESOURCE_VIBRA,
		RESOURCE_EMMC1,
		RESOURCE_EMMC2,
		RESOURCE_NFC,
		RESOURCE_TOUCHSCREEN,
		RESOURCE_TVOUT,
		RESOURCE_ACCELEROMETER,

		NUM_PERI_RESOURCE
	} PERIPHERAL_ID;

#endif				// #ifdef FUSE_COMMS_PROCESSOR

	typedef enum {
		POWER_ON,
		POWER_OFF
	} POWER_STATE;

	typedef enum {
		CLOCK_ON,	// HW auto-gating off and clock is gated on
		CLOCK_OFF,	// HW auto-gating off and clock is gated off
		CLOCK_AUTO	// HW auto-gating on
	} CLOCK_STATE;

	typedef enum {
		BUS1 = 1,	// highest bandwidth request            
		BUS2 = 2,	// a driver uses this for requesting bandwidth of system bus
		BUS3 = 3,
		BUS4 = 4,
		BUS5 = 5,
		BUS6 = 6,
		BUS7 = 7,
		BUS8 = 8,	// lowest bandwidth request     
		NUM_OF_BUS = 8
	} BUS_REQ;

// OPP_STATE mapping to CCU frequency ID
// ============================================================================================================
// RESOURCE     CPU                     HUB                                             BUS                                                             VIDEO           BMODEM          DSP
// CCU          A9              Hub             AON                     Fabric                  Slave                   MM                      BModem          DSP 
// ============================================================================================================
// OPP8         A9_26           N/A                     N/A             AXI_26                  AXI_26                  N/A                     N/A                     N/A
// OPP7         A9_52           AXI_26          AXI_26          AXI_52                  AXI_26                  N/A                     N/A                     N/A
// OPP6         A9_104          AXI_52          AXI_26          AXI_104_AHB_52  AXI_26                  R4_52           N/A                     N/A
// OPP5         A9_156          AXI_104         AXI_26          AXI_156_AHB_156 AXI_52                  AXI_50          R4_104          DSP_52
// OPP4         A9_208          AXI_156         AXI_52          AXI_156_AHB_78  AXI_78                  AXI_83          R4_156          DSP_104
// OPP3         A9_312          AXI_104_ALL     AXI_78          AXI_208_AHB_104 AXI_104                 AXI_100         R4_208          DSP_156
// OPP2         A9_467          AXI_208         AXI_104         AXI_312_AHB_104 AXI_156_APB_52  AXI_166         R4_312          DSP_208
// OPP1         A9_700          AXI_312         AXI_156         AXI_312_AHB_156 AXI_156_APB_78  AXI_250         R4_416          DSP_312 
	typedef enum {
		OPP1 = 1,	// highest performance
		OPP2 = 2,
		OPP3 = 3,
		OPP4 = 4,
		OPP5 = 5,
		OPP6 = 6,
		OPP7 = 7,
		OPP8 = 8,	// lowest performance
		NUM_OF_OPP = 8,
		OPP_INVALID = -1
	} OPP_STATE;

// SLEEP_STATE mapping to CCU low-power state (based on definition published by Anatoly)
// ===================================================================================================================
// RESOURCE                                     CPU                                     HUB                     BUS                             VIDEO           BMODEM          WCDMA           DSP
// CCU                                          A9                              Hub/AON         Fabric/Slave    MM                      BModem          WCDMA           DSP 
// ===================================================================================================================
// SLEEP_STATE_ACTIVE           RUN                                     RUN                     RUN                             RUN                     RUN                     RUN                     RUN
// SLEEP_STATE_1                        SUSPEND                         N/A                     N/A                             N/A                     N/A                     N/A                     N/A
// SLEEP_STATE_2                        SUSPEND_DORMANT         N/A                     N/A                             RETENTION       SUSPEND         N/A                     N/A
// SLEEP_STATE_DEEP             DEEPSLEEP_DORMANT       RETENTION       RETENTION               OFF                     RETENTION       RETENTION       RETENTION
	typedef enum {
		SLEEP_STATE_ACTIVE = 0,	// run state
		SLEEP_STATE_1 = 1,	// lightest sleep state                                 
		SLEEP_STATE_2 = 2,	// 2nd lowest sleep state                                      
		SLEEP_STATE_DEEP = 3,	// deepest sleep state
		NUM_OF_SLEEP_STATE = 4,
		SLEEP_STATE_INVALID = -1
	} SLEEP_STATE;

#define A9_DEEPSLEEP_DORMANT		SLEEP_STATE_DEEP
#define A9_SUSPEND_DORMANT			SLEEP_STATE_2
#define A9_SUSPEND					SLEEP_STATE_1
#define A9_RUN						SLEEP_STATE_ACTIVE

#define MM_OFF						SLEEP_STATE_DEEP
#define MM_RETENTION				SLEEP_STATE_2
#define MM_RUN						SLEEP_STATE_ACTIVE

#define BUS_RETENTION				SLEEP_STATE_DEEP
#define BUS_RUN						SLEEP_STATE_ACTIVE

#define HUB_RETENTION				SLEEP_STATE_DEEP
#define HUB_RUN						SLEEP_STATE_ACTIVE

#define R4_RETENTION				SLEEP_STATE_DEEP
#define R4_SUSPEND					SLEEP_STATE_2
#define	R4_RUN						SLEEP_STATE_ACTIVE

#define WCDMAS_RETENTION			SLEEP_STATE_DEEP
#define WCDMA_RUN					SLEEP_STATE_ACTIVE

#define DSP_RETENTION				SLEEP_STATE_DEEP
#define DSP_RUN						SLEEP_STATE_ACTIVE

	typedef enum {
		RESOURCE_POWER_STATE,
		RESOURCE_CLOCK_STATE,
		RESOURCE_CLOCK_SPEED
	} POWER_CLOCK_STATE;

#ifdef FUSE_COMMS_PROCESSOR
	typedef enum {
		BMDM_TIMER_3G_WAKEUP_EVENT = 0,
		BMDM_TIMER_2G_WAKEUP_EVENT,
		BMDM_CP_WAKEUP_EVENT,
		BMDM_WCDMA_WAKEUP_EVENT,
		BMDM_DSP_WAKEUP_EVENT,
		BMDM_PERIPH1_WAKEUP_EVENT,
		BMDM_PERIPH2_WAKEUP_EVENT,
		BMDM_SOFTWARE_0_WAKEUP_EVENT,
		BMDM_SOFTWARE_1_WAKEUP_EVENT,
		BMDM_SOFTWARE_2_WAKEUP_EVENT,
		BMDM_R4_INT_WAKEUP_WAKEUP_EVENT,
		BMDM_DSP_INT_WAKEUP_WAKEUP_EVENT,
		BMDM_SPARE_INT_WAKEUP_WAKEUP_EVENT,
		BMDM_HUB_WAKEUP_WAKEUP_EVENT,
		BMDM_EXT_WAKEUP_WAKEUP_EVENT,
		BMDM_EXT_SPARE_1_WAKEUP_EVENT,
		BMDM_EXT_SPARE_2_WAKEUP_EVENT,
		BMDM_EXT_SPARE_3_WAKEUP_EVENT,
		BMDM_EXT_SPARE_4_WAKEUP_EVENT,
		NUM_WAKEUP_EVENT
	} WAKEUP_EVENT_ID;
#else				// #ifdef FUSE_COMMS_PROCESSOR
	typedef enum {
		LCDTE_WAKEUP_EVENT = 0,
		SSP2SYN_WAKEUP_EVENT,
		SSP2DI_WAKEUP_EVENT,
		SSP2CK_WAKEUP_EVENT,
		SSP1SYN_WAKEUP_EVENT,
		SSP1DI_WAKEUP_EVENT,
		SSP1CK_WAKEUP_EVENT,
		SSP0SYN_WAKEUP_EVENT,
		SSP0DI_WAKEUP_EVENT,
		SSP0CK_WAKEUP_EVENT,
		DIGCLKREQ_WAKEUP_EVENT,
		ANA_SYS_REQ_WAKEUP_EVENT,
		SYSCLKREQ_WAKEUP_EVENT,
		UBRX_WAKEUP_EVENT,
		UBCTSN_WAKEUP_EVENT,
		UB2RX_WAKEUP_EVENT,
		UB2CTSN_WAKEUP_EVENT,
		SIMDET_WAKEUP_EVENT,
		SIM2DET_WAKEUP_EVENT,
		MMC0D3_WAKEUP_EVENT,
		MMC0D1_WAKEUP_EVENT,
		MMC1D3_WAKEUP_EVENT,
		MMC1D1_WAKEUP_EVENT,
		SDDAT3_WAKEUP_EVENT,
		SDDAT1_WAKEUP_EVENT,
		SLB1CLK_WAKEUP_EVENT,
		SLB1DAT_WAKEUP_EVENT,
		SWCLKTCK_WAKEUP_EVENT,
		SWDIOTMS_WAKEUP_EVENT,
		KEY_R0_WAKEUP_EVENT,
		KEY_R1_WAKEUP_EVENT,
		KEY_R2_WAKEUP_EVENT,
		KEY_R3_WAKEUP_EVENT,
		KEY_R4_WAKEUP_EVENT,
		KEY_R5_WAKEUP_EVENT,
		KEY_R6_WAKEUP_EVENT,
		KEY_R7_WAKEUP_EVENT,
		CAWAKE_WAKEUP_EVENT,
		CAREADY_WAKEUP_EVENT,
		CAFLAG_WAKEUP_EVENT,
		BATRM_WAKEUP_EVENT,
		USBDP_WAKEUP_EVENT,
		USBDN_WAKEUP_EVENT,
		RXD_WAKEUP_EVENT,
		GPIO29_A_WAKEUP_EVENT,
		GPIO32_A_WAKEUP_EVENT,
		GPIO33_A_WAKEUP_EVENT,
		GPIO43_A_WAKEUP_EVENT,
		GPIO44_A_WAKEUP_EVENT,
		GPIO45_A_WAKEUP_EVENT,
		GPIO46_A_WAKEUP_EVENT,
		GPIO47_A_WAKEUP_EVENT,
		GPIO48_A_WAKEUP_EVENT,
		GPIO71_A_WAKEUP_EVENT,
		GPIO72_A_WAKEUP_EVENT,
		GPIO73_A_WAKEUP_EVENT,
		GPIO74_A_WAKEUP_EVENT,
		GPIO95_A_WAKEUP_EVENT,
		GPIO96_A_WAKEUP_EVENT,
		GPIO99_A_WAKEUP_EVENT,
		GPIO100_A_WAKEUP_EVENT,
		GPIO111_A_WAKEUP_EVENT,
		GPIO18_A_WAKEUP_EVENT,
		GPIO19_A_WAKEUP_EVENT,
		GPIO20_A_WAKEUP_EVENT,
		GPIO89_A_WAKEUP_EVENT,
		GPIO90_A_WAKEUP_EVENT,
		GPIO91_A_WAKEUP_EVENT,
		GPIO92_A_WAKEUP_EVENT,
		GPIO93_A_WAKEUP_EVENT,
		GPIO18_B_WAKEUP_EVENT,
		GPIO19_B_WAKEUP_EVENT,
		GPIO20_B_WAKEUP_EVENT,
		GPIO89_B_WAKEUP_EVENT,
		GPIO90_B_WAKEUP_EVENT,
		GPIO91_B_WAKEUP_EVENT,
		GPIO92_B_WAKEUP_EVENT,
		GPIO93_B_WAKEUP_EVENT,
		GPIO29_B_WAKEUP_EVENT,
		GPIO32_B_WAKEUP_EVENT,
		GPIO33_B_WAKEUP_EVENT,
		GPIO43_B_WAKEUP_EVENT,
		GPIO44_B_WAKEUP_EVENT,
		GPIO45_B_WAKEUP_EVENT,
		GPIO46_B_WAKEUP_EVENT,
		GPIO47_B_WAKEUP_EVENT,
		GPIO48_B_WAKEUP_EVENT,
		GPIO71_B_WAKEUP_EVENT,
		GPIO72_B_WAKEUP_EVENT,
		GPIO73_B_WAKEUP_EVENT,
		GPIO74_B_WAKEUP_EVENT,
		GPIO95_B_WAKEUP_EVENT,
		GPIO96_B_WAKEUP_EVENT,
		GPIO99_B_WAKEUP_EVENT,
		GPIO100_B_WAKEUP_EVENT,
		GPIO111_B_WAKEUP_EVENT,
		COMMON_TIMER_0_WAKEUP_EVENT,
		COMMON_TIMER_1_WAKEUP_EVENT,
		COMMON_TIMER_2_WAKEUP_EVENT,
		COMMON_TIMER_3_WAKEUP_EVENT,
		COMMON_TIMER_4_WAKEUP_EVENT,
		COMMON_INT_TO_AC_WAKEUP_EVENT,
		TZCFG_INT_TO_AC_WAKEUP_EVENT,
		DMA_REQUEST_WAKEUP_EVENT,
		MODEM1_WAKEUP_EVENT,
		MODEM2_WAKEUP_EVENT,
		MODEM_UART_WAKEUP_EVENT,
		BRIDGE_TO_AC_WAKEUP_EVENT,
		BRIDGE_TO_MODEM_WAKEUP_EVENT,
		VREQ_NONZERO_PI_MODEM_WAKEUP_EVENT,
		USBOTG_WAKEUP_EVENT,
		GPIO_EXP_IRQ_WAKEUP_EVENT,
		DBR_IRQ_WAKEUP_EVENT,
		ACI_WAKEUP_EVENT,
		PHY_RESUME_WAKEUP_EVENT,
		MODEMBUS_ACTIVE_WAKEUP_EVENT,
		SOFTWARE_0_WAKEUP_EVENT,
		SOFTWARE_1_WAKEUP_EVENT,
		SOFTWARE_2_WAKEUP_EVENT,
		NUM_WAKEUP_EVENT
	} WAKEUP_EVENT_ID;
#endif				// #ifdef FUSE_COMMS_PROCESSOR

	typedef enum {
		EVENT_DISABLED,
		EVENT_POSEDGE_ENABLED,
		EVENT_NEGEDGE_ENABLED,
		EVENT_POSNEGEDGE_ENABLED
	} EVENT_ENABLE;

	typedef enum {
		PRM_ILLEGAL_CLOCK_STATE = -13,	// Clock state is not valid
		PRM_ILLEGAL_CLOCK_SPEED = -12,	// Clock speed is not supported
		PRM_ILLEGAL_POWER_STATE = -11,	// Power state is not supported
		PRM_ILLEGAL_SLEEP_STATE = -10,	// Sleep state is not supported
		PRM_OPP_CHANGE_DISABLED = -9,	// OPP change has been disabled
		PRM_ILLEGAL_OPP = -8,	// OPP is not valid
		PRM_ILLEGAL_RESOURCE = -7,	// Resource not supported
		PRM_NONEXISTENT_ID = -6,	// Client ID is valid but has not registered
		PRM_ILLEGAL_ID = -5,	// Client ID is not valid
		PRM_MAX_ID_REACHED = -4,	// Maximum number of clients, 128, has reached
		PRM_NAME_IN_USE = -3,	// Name already registered by other client
		PRM_ILLEGAL_NAME = -2,	// NULL, non-ASCII or longer than 64 char name
		PRM_FAIL = -1,	// Operation failed
		PRM_OK = 0	// Operation succeeded                  
	} PRM_RETURN;

	typedef enum {
		CLIENT_API_TEST,
		PERIPHERAL_API_TEST,
		OPP_API_TEST,
		SLEEP_API_TEST,
		RESET_API_TEST,

		CLIENT_SNAPSHOT,
		RESOURCE_SNAPSHOT,
		CLOCK_SNAPSHOT,
		SLEEP_SNAPSHOT,

		CONFIG_WAKEUP_EVENT,
		CONFIG_PM_DEBUG_BUS,
		READ_OTP_CLOCK_SPEED,
		FORCE_CCU_POLICY,
		ENABLE_MOMDEM_CCU_PROFILE,
		ENABLE_CLOCK_MONITOR
	} PRM_DEBUG;

	typedef int CLIENT_ID;

#define	NUMBER_OF_RESERVED_CLIENTS	4
#define	RTOS_CLIENT			0	// reserved client ID for RTOS scheduler
#define	DFS_CLIENT			1	// reserved client ID for DFS
#define	INIT_CLIENT			2	// reserved client ID for PRM initialization
#define	ANONYMOUS_CLIENT	3	// reserved client ID for anonymous user
#define	NULL_CLIENT			-1

/**
*  @brief  Initialize PRMF.This API should be called once during boot before interrupts are enabled 
*
*  @param  
*
*  @return 
*
*  @note   
*
****************************************************************************/
	void PRM_init(void);

/**
*  @brief  Process client registration to PRMF 
*
*  @param  client_name			(in) pointer to ASCII name provided to identify this client
*
*  @return Client ID, or PRM_FAIL if registration failed.
*
*  @note   
****************************************************************************/
	CLIENT_ID PRM_client_register(char *client_name);

/**
*  @brief  Process client de-registration to PRMF 
*
*  @param  client_id			(in) client ID obtained after registration
*
*  @return PRM_OK or error code 
*
*  @note   
*
****************************************************************************/
	PRM_RETURN PRM_client_deregister(CLIENT_ID client_id);

/**
*  @brief  Set power state of a peripheral resource  
*
*  @param  client_id			(in) client ID obtained after registration
*  @param  resource_id  		(in) peripheral resource ID
*  @param  state  				(in) new power state
*
*  @return PRM_OK or error code 
*
*  @note   
*
****************************************************************************/
	PRM_RETURN PRM_set_power_state(CLIENT_ID client_id,
				       PERIPHERAL_ID resource_id,
				       POWER_STATE state);

/**
*  @brief  Set clock state of a peripheral resource  
*
*  @param  client_id			(in) client ID obtained after registration
*  @param  resource_id  		(in) peripheral resource ID
*  @param  clock_state		  	(in) new clock state
*                              
*  @return PRM_OK or error code 
*
*  @note   
*
****************************************************************************/
	PRM_RETURN PRM_set_clock_state(CLIENT_ID client_id,
				       PERIPHERAL_ID resource_id,
				       CLOCK_STATE clock_state);

/**
*  @brief  Set clock speed of a peripheral resource  
*
*  @param  client_id			(in) client ID obtained after registration
*  @param  resource_id  		(in) peripheral resource ID
*  @param  freq		  			(in) frequency in Hz
*                              
*  @return PRM_OK or error code 
*
*  @note   
*
****************************************************************************/
	PRM_RETURN PRM_set_clock_speed(CLIENT_ID client_id,
				       PERIPHERAL_ID resource_id, int freq);

/**
*  @brief  Set clock divider of a peripheral resource  
*
*  @param  client_id			(in) client ID obtained after registration
*  @param  resource_id  		(in) peripheral resource ID
*  @param  pll		  			(in) pll selection
*  @param  pre_divider_terminal	(in) pre_divider terminal count value, pre_divider_terminal+1 is used as divisor
*  @param  divider_terminal		(in) divider terminal count value, divider_terminal+1 is used as divisor
*  @param  fraction		  		(in) fraction of a fractional divider
*                              
*  @return PRM_OK or error code 
*
*  @note   
*
****************************************************************************/
	PRM_RETURN PRM_set_clock_divider(CLIENT_ID client_id,
					 PERIPHERAL_ID resource_id, int pll,
					 int pre_divider_terminal,
					 int divider_terminal, int fraction);

/**
*  @brief  Request current power or clock state of a peripheral resource. 
*
*  @param  client_id			(in) client ID obtained after registration
*  @param  resource_id  		(in) peripheral resource ID
*  @param  state				(in) type of requested state - power, clock or speed
*
*  @return requested power/clock state or error code
*
*  @note   
*
****************************************************************************/
	int PRM_get_power_clock_state(CLIENT_ID client_id,
				      PERIPHERAL_ID resource_id,
				      POWER_CLOCK_STATE state);

/**
*  @brief  Request bandwidth of system bus for a peripheral
*
*  @param  client_id			(in) client ID obtained after registration
*  @param  resource_id  		(in) peripheral resource ID
*  @param  req					(in) bus bandwidth request
*
*  @return PRM_OK or error code
*
*  @note   
*
****************************************************************************/
	PRM_RETURN PRM_request_bus(CLIENT_ID client_id,
				   PERIPHERAL_ID resource_id, BUS_REQ req);

/**
*  @brief  disable or enable an event 
*
*  @param  client_id			(in) client ID obtained after registration
*  @param  event_id  		    (in) event ID
*  @param  enable				(in) rising and/or falling edge enabled or disabled
*
*  @return PRM_OK or error code
*
*  @note   
*
****************************************************************************/
	PRM_RETURN PRM_enable_event(CLIENT_ID client_id,
				    WAKEUP_EVENT_ID event_id,
				    EVENT_ENABLE enable);

/**
*  @brief  register a callback for an event 
*          the callback get called after the event has triggered a CCU policy change
*
*  @param  client_id			(in) client ID obtained after registration
*  @param  event_id  		    (in) event ID
*  @param  func					(in) callback
*
*  @return PRM_OK or error code
*
*  @note   
*
****************************************************************************/
	PRM_RETURN PRM_register_wakeup_callbk(CLIENT_ID client_id,
					      WAKEUP_EVENT_ID event_id,
					      void (*func) (WAKEUP_EVENT_ID
							    event_id));

/**
*  @brief  de-register a callback for an event 
*
*  @param  client_id			(in) client ID obtained after registration
*  @param  event_id  		    (in) event ID
*  @param  func					(in) callback
*
*  @return PRM_OK or error code
*
*  @note   
*
****************************************************************************/
	PRM_RETURN PRM_deregister_wakeup_callbk(CLIENT_ID client_id,
						WAKEUP_EVENT_ID event_id,
						void (*func) (WAKEUP_EVENT_ID
							      event_id));

/**
*  @brief  force the subsystem for a peripheral to stay active after wakeup for the duration of "latency"(us) 
*
*  @param  client_id			(in) client ID obtained after registration
*  @param  resource_id  		(in) peripheral resource ID
*  @param  latency				(in) latency in us
*
*  @return PRM_OK or error code
*
*  @note   
*
****************************************************************************/
	PRM_RETURN PRM_set_forced_wakeup_latency(CLIENT_ID client_id,
						 PERIPHERAL_ID resource_id,
						 int latency);

/**
*  @brief  Request minimum operating point of a performance resource 
*
*  @param  client_id			(in) client ID obtained after registration
*  @param  resource_id  		(in) performance resource ID
*  @param  opp_int				(in) operating point
*  @param  func					(in) NULL if synchronous operation requested, otherwise callback function for asynchronous operation
*
*  @return PRM_OK or error code
*
*  @note   
*
****************************************************************************/
	PRM_RETURN PRM_request_opp(CLIENT_ID client_id,
				   PERFORMANCE_ID resource_id, int opp_int,
				   void (*func) (PRM_RETURN ret));

/**
*  @brief  Force a performance resource to a specific operating point 
*
*  @param  client_id			(in) client ID obtained after registration
*  @param  resource_id  		(in) performance resource ID
*  @param  opp_int				(in) operating point
*  @param  func					(in) NULL if synchronous operation requested, otherwise callback function for asynchronous operation
*
*  @return PRM_OK or error code
*
*  @note   
*
****************************************************************************/
	PRM_RETURN PRM_force_opp(CLIENT_ID client_id,
				 PERFORMANCE_ID resource_id, int opp_int,
				 void (*func) (PRM_RETURN ret));

/**
*  @brief  Release operating point previously requested by PRM_request_opp or forced by PRM_force_opp 
*
*  @param  client_id			(in) client ID obtained after registration
*  @param  resource_id  		(in) performance resource ID
*
*  @return PRM_OK or error code
*
*  @note   
*
****************************************************************************/
	PRM_RETURN PRM_release_opp(CLIENT_ID client_id,
				   PERFORMANCE_ID resource_id);

/**
*  @brief  Get current active operating point of a performance resource 
*
*  @param  client_id			(in) client ID obtained after registration
*  @param  resource_id  		(in) performance resource ID
*
*  @return active operating point or error code
*
*  @note   
*
****************************************************************************/
	OPP_STATE PRM_get_opp(CLIENT_ID client_id, PERFORMANCE_ID resource_id);

/**
*  @brief  Disable operating point change of a performance resource. 
*          This function is used to temporarily block any operating point change. 
*          During this time any other client request will be queued and served after resource status change is again allowed 
*
*  @param  client_id			(in) client ID obtained after registration
*  @param  resource_id  		(in) performance resource ID
*
*  @return PRM_OK or error code
*
*  @note   
*
****************************************************************************/
	PRM_RETURN PRM_disable_opp_change(CLIENT_ID client_id,
					  PERFORMANCE_ID resource_id);

/**
*  @brief  Re-enable operating point change of a performance resource. 
*
*  @param  client_id			(in) client ID obtained after registration
*  @param  resource_id  		(in) performance resource ID
*
*  @return PRM_OK or error code
*
*  @note   
*
****************************************************************************/
	PRM_RETURN PRM_enable_opp_change(CLIENT_ID client_id,
					 PERFORMANCE_ID resource_id);

/**
*  @brief  Request number of operating points supported by a performance resource. 
*
*  @param  client_id			(in) client ID obtained after registration
*  @param  resource_id  		(in) performance resource ID
*
*  @return Number of operating points or error code
*
*  @note   
*
****************************************************************************/
	int PRM_get_num_of_opp(CLIENT_ID client_id, PERFORMANCE_ID resource_id);

/**
*  @brief  Get frequency value of a operating point of a performance resource. 
*
*  @param  client_id			(in) client ID obtained after registration
*  @param  resource_id  		(in) performance resource ID
*  @param  opp					(in) operating point
*
*  @return Number of operating points or error code
*
*  @note   
*
****************************************************************************/
	int PRM_get_opp_speed(CLIENT_ID client_id, PERFORMANCE_ID resource_id,
			      OPP_STATE opp);

/**
*  @brief  Register a callback for receiving notification that operating point of a performance resource is about to change. 
*
*  @param  client_id			(in) client ID obtained after registration
*  @param  resource_id  		(in) performance resource ID
*  @param  func					(in) callback function
*
*  @return PRM_OK or error code
*
*  @note   
*
****************************************************************************/
	PRM_RETURN PRM_opp_change_prepare_callbk_register(CLIENT_ID client_id,
							  PERFORMANCE_ID
							  resource_id,
							  void (*func)
							  (OPP_STATE opp));

/**
*  @brief  De-register a callback for receiving notification that operating point of a performance resource is about to change. 
*
*  @param  client_id			(in) client ID obtained after registration
*  @param  resource_id  		(in) performance resource ID
*  @param  func					(in) callback function
*
*  @return PRM_OK or error code
*
*  @note   
*
****************************************************************************/
	PRM_RETURN PRM_opp_change_prepare_callbk_deregister(CLIENT_ID client_id,
							    PERFORMANCE_ID
							    resource_id,
							    void (*func)
							    (OPP_STATE opp));

/**
*  @brief  Register a callback for receiving notification that operating point of a performance resource has changed. 
*
*  @param  client_id			(in) client ID obtained after registration
*  @param  resource_id  		(in) performance resource ID
*  @param  func					(in) callback function
*
*  @return PRM_OK or error code
*
*  @note   
*
****************************************************************************/
	PRM_RETURN PRM_opp_change_finish_callbk_register(CLIENT_ID client_id,
							 PERFORMANCE_ID
							 resource_id,
							 void (*func) (OPP_STATE
								       opp));

/**
*  @brief  De-register a callback for receiving notification that operating point of a performance resource has changed. 
*
*  @param  client_id			(in) client ID obtained after registration
*  @param  resource_id  		(in) performance resource ID
*  @param  func					(in) callback function
*
*  @return PRM_OK or error code
*
*  @note   
*
****************************************************************************/
	PRM_RETURN PRM_opp_change_finish_callbk_deregister(CLIENT_ID client_id,
							   PERFORMANCE_ID
							   resource_id,
							   void (*func)
							   (OPP_STATE opp));

/**
*  @brief  Reset the while chip. 
*
*  @param  client_id			(in) client ID obtained after registration
*
*  @return PRM_OK or error code
*
*  @note   
*
****************************************************************************/
	PRM_RETURN PRM_system_reset(CLIENT_ID client_id);

/**
*  @brief  Get the reason of previous reset. 
*
*  @param  client_id			(in) client ID obtained after registration
*  @param  reason  				(out) pointer to reason of reset
*
*  @return PRM_OK or error code
*
*  @note   
*
****************************************************************************/
	PRM_RETURN PRM_get_system_reset_reason(CLIENT_ID client_id,
					       int *reason);

/**
*  @brief  Reset a peripheral resource 
*
*  @param  client_id			(in) client ID obtained after registration
*  @param  resource_id  		(in) peripheral resource ID
*
*  @return PRM_OK or error code
*
*  @note   
*
****************************************************************************/
	PRM_RETURN PRM_resource_reset(CLIENT_ID client_id,
				      PERIPHERAL_ID resource_id);

/**
*  @brief  This function sets maximum allowed wake-up latency of a performance resource. 
*          It defines how fast a performance resource must be able to wake-up from sleep to active mode. 
*          0 us latency would mean that sleep state will never be entered. 
*
*  @param  client_id			(in) client ID obtained after registration
*  @param  resource_id  		(in) performance resource ID
*  @param  latency		  		(in) in us
*
*  @return PRM_OK or error code
*
*  @note   
*
****************************************************************************/
	PRM_RETURN PRM_allowed_wakeup_latency(CLIENT_ID client_id,
					      PERFORMANCE_ID resource_id,
					      int latency);

/**
*  @brief  This function get hw wake-up latency from a sleep state 
*          choose RESOURCE_SYSTEM for latency from deep sleep with 26MHz turned off
*
*  @param  client_id			(in) client ID obtained after registration
*  @param  resource_id  		(in) performance resource ID
*  @param  state		  		(in) sleep state
*
*  @return latency in us or 0 if not supported
*
*  @note   
*
****************************************************************************/
	int PRM_get_wakeup_latency(CLIENT_ID client_id,
				   PERFORMANCE_ID resource_id,
				   SLEEP_STATE state);

/**
*  @brief  This function sets allowed sleep state of a performance resource. 
*          All sleep states below allowed state are disabled. For example, if allowed state is set to LIGHT_SLEEP, a performance is not allowed to enter DEEP_SLEEP state. 
*
*  @param  client_id			(in) client ID obtained after registration
*  @param  resource_id  		(in) performance resource ID
*  @param  state		  		(in) sleep state
*
*  @return PRM_OK or error code
*
*  @note   
*
****************************************************************************/
	PRM_RETURN PRM_allowed_sleep_state(CLIENT_ID client_id,
					   PERFORMANCE_ID resource_id,
					   SLEEP_STATE state);

/**
*  @brief  This API puts a performance resource to a specific sleep state. 
*
*  @param  client_id			(in) client ID obtained after registration
*  @param  resource_id  		(in) performance resource ID
*  @param  state		  		(in) sleep state
*
*  @return PRM_OK or error code
*
*  @note   
*
****************************************************************************/
	PRM_RETURN PRM_force_sleep_state(CLIENT_ID client_id,
					 PERFORMANCE_ID resource_id,
					 SLEEP_STATE state);

/**
*  @brief  This API returns current sleep state of a performance resource. 
*
*  @param  client_id			(in) client ID obtained after registration
*  @param  resource_id  		(in) performance resource ID
*
*  @return sleep state
*
*  @note   
*
****************************************************************************/
	SLEEP_STATE PRM_get_sleep_state(CLIENT_ID client_id,
					PERFORMANCE_ID resource_id);

/**
*  @brief  Register a callback for receiving notification of sleep state change 
*
*  @param  client_id			(in) client ID obtained after registration
*  @param  resource_id  		(in) performance resource ID
*  @param  state		  		(in) sleep state
*  @param  func					(in) callback function
*
*  @return PRM_OK or error code
*
*  @note   
*
****************************************************************************/
	PRM_RETURN PRM_sleep_state_change_callbk_register(CLIENT_ID client_id,
							  PERFORMANCE_ID
							  resource_id,
							  SLEEP_STATE state,
							  void (*func)
							  (SLEEP_STATE state));

/**
*  @brief  De-register a callback for receiving notification of sleep state change. 
*
*  @param  client_id			(in) client ID obtained after registration
*  @param  resource_id  		(in) performance resource ID
*  @param  state		  		(in) sleep state
*  @param  func					(in) callback function
*
*  @return PRM_OK or error code
*
*  @note   
*
****************************************************************************/
	PRM_RETURN PRM_sleep_state_change_callbk_deregister(CLIENT_ID client_id,
							    PERFORMANCE_ID
							    resource_id,
							    SLEEP_STATE state,
							    void (*func)
							    (SLEEP_STATE
							     state));

/**
*  @brief  Debug service provided by PRM. 
*
*  @param  type					(in) type of service
*
*
*  @note   
*
****************************************************************************/
	PRM_RETURN PRM_debug(PRM_DEBUG type, int arg1, int arg2, int arg3,
			     int arg4, int arg5, int arg6, int arg7);

#ifdef __cplusplus
}
#endif
#endif				// _PM_PRM_H_
