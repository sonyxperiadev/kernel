/****************************************************************************
*
*     Copyright (c) 2007 Broadcom Corporation
*           All Rights Reserved
*
*     No portions of this material may be reproduced in any form without the
*     written permission of:
*
*           Broadcom Corporation
*           5300 California Avenue
*           Irvine, California 92617
*
*     All information contained in this document is Broadcom Corporation
*     company private, proprietary, and trade secret.
*
****************************************************************************/
/**
*
*   @file   hal_pmu.h
*
*   @brief  Global declaration of hardware abstraction layer for PMU driver.
*
****************************************************************************/
/**
*   @defgroup   HAL_PMU   PMU HAL Interface 
*   @ingroup    HALGroup	
*   @brief      HAL PMU Control header file
*
*   Hardware Abstraction Layer of PMU.   
*
****************************************************************************/
/**
 * @addtogroup HAL_PMU	
 * @{
 */
//******************************** History *************************************
//
// $Log:  $ 
//
// 01-06-2011	  Raja Ramaubramanian
//		-	Added ADP action command EM_PMU_ACTION_CTRL_ADP_SET_PROBE_CYCLE 
//		-     Modified HAL_EM_PMU_Action_CtrlADPProbe_st_t,  to have new argument
//		-	Modified HAL_EM_PMU_Action_ADPRefProbe_st_t to have new argument
//		-	Modfied HAL_EM_PMU_Action_setAdpCompMethod_st_t argument
//		-	Added HAL_EM_PMU_Action_ADPSetProbeCycle_st_t to support new action
//******************************************************************************

//#include "pmu.h"

#if !defined( _HAL_EM_PMU_H__ )
#define _HAL_EM_PMU_H__

//#define tempINTERFACE_UPDATE_PMU_HAL_ADP 1

// ---- Public Constants and Types  ----------------------------------------------------

/** 
	EM_PMU CONFIGURATION AND INIT 
*/

//! EM_PMU configuration and initialization data
typedef struct
{
	Boolean device_ACTIVE;			//< pmu device active means it is initialized. pmu active does not mean much for sleep mode. set to auto
	UInt32	performance_required;	//< performance required does not have much meaning on pmu.  set to default auto.
} HAL_EM_PMU_Config_st_t;


//-----------------------------------------------------------------------------------
/// ACTIONS
//-----------------------------------------------------------------------------------

/**  HAL_EM_PMU Device ACTION definitions
*
*   If action is not supported by the pmu device on-board, function will return error.
*   The content 'Element type {::type}' in the comment is used in generating void pointer
*   in xScript.
*/
typedef enum
{
	EM_PMU_ACTION_REINIT,						///< Reinitialize pmu driver (could be called due to i2c malfunction like in fm radio case)
	EM_PMU_ACTION_POWER_DOWN,					///< Power down pmu (calling clientpowerdown) 
	EM_PMU_ACTION_GET_POWERUPCAUSE,				///< Get power up cause 
	EM_PMU_ACTION_ACTIVATE_SIMLDO,				///< Actiavate SIM LDO  
	EM_PMU_ACTION_IS_SIM_POWER_READY,			///< Is SIM power ready/up?
	EM_PMU_ACTION_SET_VOLTAGE_IN_ACTIVEMODE,	///< Set voltage in active modes ( set it based on sysparm values )
	EM_PMU_ACTION_SET_GPO,						///< Set GPO on pmu to specific setting 
	EM_PMU_ACTION_CONFIGURE_GPO,				///< Configure GPO on pmu to specific setting 
	EM_PMU_ACTION_IS_CHARGER_PRESENT,			///< Get charger presense for both usb and wall chger (callback param will indicate which one)
	EM_PMU_ACTION_USB_START_FASTCHARGE,			///< Start USB charger in fast charge mode - 500mA typically (CB). 
												///< Provided for USB middleware or upper layer to call. Not for USB low-level driver to call. 
												///< USB low-level driver should call PMU driver directly (same architectural level)
	EM_PMU_ACTION_USB_START_SLOWCHARGE,			///< Start USB charger in slow charge mode - 100mA typically (CB)
	EM_PMU_ACTION_STOP_CHARGING,				///< Start Charging for either USB or WALL charger. Same function for both.
	EM_PMU_ACTION_START_CHARGING,				///< Stop Charging for either USB or WALL charger. Same function for both.
    EM_PMU_ACTION_USB_ENABLE_CHARGING,			///< Enable USB Charging.
    EM_PMU_ACTION_USB_DISABLE_CHARGING,			///< Disable USB Charging.
	EM_PMU_ACTION_GET_ONKEY_STATUS,			    ///<Element type {::HAL_EM_PMU_Action_GetOnkeyStatus_st_t}, Get ONKEY instant status.  Return TRUE if pressed, 
												// FALSE if not pressed (result passed in *data)
#if (defined(PMU_BCM59055) ||defined (PMU_MAX8986))
    EM_PMU_ACTION_SW_UP,                        ///< Tell PMU HW that SW is up and running
    EM_PMU_ACTION_IS_DETECT_LDO_ON,             ///< Find out if detect_LDO is on or off
    EM_PMU_ACTION_IS_BATTERY_WEAK,              ///< Find out if battery is weak or good
    EM_PMU_ACTION_IS_DBP,			            ///< Find out if Dead Battery Provision (DBP) apply
    EM_PMU_ACTION_CTRL_DETECT_LDO,			    ///< Start or stop detect LDO
    EM_PMU_ACTION_SET_MAX_USB_CHRG_CURRENT,		///< Set maximum USB charging current 
    EM_PMU_ACTION_SET_MAX_WALL_CHRG_CURRENT,	///< Set maximum Wall charging current   
    EM_PMU_ACTION_SET_MAX_CHRG_VOLTAGE,		    ///< Set maximum charging voltgae
    EM_PMU_ACTION_STROBE_CHARGER_WATCHDOG,      ///< Strobe charger watchdog
    EM_PMU_ACTION_STROBE_SYSTEM_WATCHDOG,       ///< Strobe system watchdog
    EM_PMU_ACTION_SET_USB_CHARGER_TYPE,         ///< Set USB charger type 
    EM_PMU_ACTION_GET_USB_ID,                   ///< Get USB ID 
    EM_PMU_ACTION_CTRL_ADP_BLOCK,               ///< Control (enable/disable) ADP block 
	EM_PMU_ACTION_CTRL_ADP_PROBE,               ///< Control (start/stop) ADP probe 
	EM_PMU_ACTION_CTRL_ADP_SENSE,               ///< Control (start/stop) ADP sense
    EM_PMU_ACTION_IS_ADP_ATTACHED,              ///< Find out if ADP attached
    EM_PMU_ACTION_IS_REMOTE_ADP_PROBING,        ///< Find out if remote device is doing ADP probing
	EM_PMU_ACTION_GET_ADP_PROBE_SETTING,		/// < Return Probe setting 
	EM_PMU_ACTION_GET_ADP_SENSE_SETTING,		/// < Return Sense setting 
	EM_PMU_ACTION_GET_ADP_PROBE_MEASUREMENT_REGISTER_VALUES, /// < Return ADP Probe measurement register values 
	EM_PMU_ACTION_CTRL_RESET_ADP_REGS,			/// < Reset ADP Registers
	EM_PMU_ACTION_CTRL_SET_TPROBE_MAX,			/// <Set Tprobe Max 
	EM_PMU_ACTION_CTRL_SET_ADP_COMP_METHOD, 	/// <Set ADP Probe comparison method
	EM_PMU_ACTION_CTRL_ADP_REF_PROBE,			/// < ADP Reference probe
	EM_PMU_ACTION_CTRL_ADP_SENSE_PROBE,			/// < ADP Sense followed by ADP Probe
	EM_PMU_ACTION_CTRL_ADP_STOP_PROBE,			///< Stop the ADP Probe	
	EM_PMU_ACTION_CTRL_ADP_SET_PROBE_CYCLE,		///< Sets probing cycle to short or long
    EM_PMU_ACTION_READ_FGSMPL,					///< Read FGSMPL
	EM_PMU_ACTION_START_IBAT_READING,			///< Start battery current measurement
	EM_PMU_ACTION_STOP_IBAT_READING,			///< Stop battery current measurement
#endif
#if defined (PMU_BCM59001) || defined (PMU_BCM59035)  || defined (PMU_BCM59036) || defined(PMU_BCM59038) || defined(PMU_BCM59055) || defined(PMU_MAX8986)                                            
    EM_PMU_ACTION_TURNON_MICBIAS,               ///< Turn on Mic bias
    EM_PMU_ACTION_TURNOFF_MICBIAS,              ///< Turn off Mic bias
#if defined(PMU_BCM59036) || defined (BCM59035_REV_B0) || defined(PMU_BCM59038) || defined(PMU_BCM59055) || defined(PMU_MAX8986)
    EM_PMU_ACTION_TURNON_KEYLOCK,               ///< Turn on keylock function
    EM_PMU_ACTION_TURNOFF_KEYLOCK,              ///< Turn off keylock function
    EM_PMU_ACTION_CTRL_EOC_CURRENT,             ///< Control End of Charge current
    EM_PMU_ACTION_CTRL_MAINT_CHARGE,            ///< Control maintenance charge enable/disable and maintenance voltage
    EM_PMU_ACTION_ENABLE_NTC,                   ///< Control NTC LDO enable or disable
	EM_PMU_ACTION_PAUSE_CHARGING,               ///< Pause or resume charging control
    EM_PMU_ACTION_ENABLE_PMUINT,                ///< Enable GPIO interrupt which is tied to PMU interrupt pin
	EM_PMU_ACTION_DISABLE_PMUINT,               ///< Disable GPIO interrupt which is tied to PMU interrupt pin
	EM_PMU_ACTION_SET_USB_CHARGING_VOLTAGE,     ///< Set USB charging voltage 
	EM_PMU_ACTION_SET_WALL_CHARGING_VOLTAGE,    ///< Set Wall charging voltage
	EM_PMU_ACTION_SET_USB_CHARGING_CURRENT,     ///< Set USB charging current
	EM_PMU_ACTION_SET_WALL_CHARGING_CURRENT,    ///< Set Wall charging current
	EM_PMU_ACTION_GET_CHARGER_IN_USE,           ///< Get charger in use
	EM_PMU_ACTION_OTG_ENABLE_BLOCK,             ///< Enable OTG block
	EM_PMU_ACTION_OTG_DISABLE_BLOCK,            ///< Disable OTG block
    EM_PMU_ACTION_OTG_RESET_BLOCK,              ///< Reset OTG block                                                
#elif defined (PMU_BCM59001)
    EM_PMU_ACTION_SET_USB_CHARGING_CURRENT,     ///< Set USB charging current
    EM_PMU_ACTION_GET_USB_CHARGING_CURRENT,     ///< Get USB charging current
    EM_PMU_ACTION_SET_USB_RC_ENUM_CURRENT,      ///< Set USB rapid charging current after successful enumeration
    EM_PMU_ACTION_GET_USB_RC_ENUM_CURRENT,      ///< Get USB rapid charging current to be used after successful enumeration
#endif
#if defined (PMU_BCM59036)
	EM_PMU_ACTION_SET_USB_SW_CONTROL_PULSING,		///< Set 59036 in software controlled pulsing mode for USB
	EM_PMU_ACTION_SET_USB_HW_CONTROL_PULSING,		///< Set 59036 in hardware controlled pulsing mode for USB (default)
	EM_PMU_ACTION_SET_WALL_SW_CONTROL_PULSING,		///< Set 59036 in software controlled pulsing mode for Wall
	EM_PMU_ACTION_SET_WALL_HW_CONTROL_PULSING,		///< Set 59036 in hardware controlled pulsing mode for Wall (default)
#endif
#endif
       EM_PMU_ACTION_SET_USB_RC_PRE_ENUM_CURRENT,  ///< Set USB rapid charging current to be used before enumeration
       EM_PMU_ACTION_GET_USB_RC_CURRENT_IN_MA,     ///< Get USB rapid charging current in milli Amp
       EM_PMU_ACTION_SET_USB_RC_ENUM_CURRENT,      ///< Set USB rapid charging current after successful enumeration
       EM_PMU_ACTION_GET_USB_RC_ENUM_CURRENT,      ///< Get USB rapid charging current in milli Amp to be used after successful enumeration                                         
       EM_PMU_ACTION_OTG_SW_CTRL,                  ///< Set OTG control to software based
       EM_PMU_ACTION_OTG_HW_CTRL,                  ///< Set OTG control to hardware based
	EM_PMU_ACTION_OTG_ENABLE_VBUS_BOOST,        ///< Enable Vbus boost circuit 
	EM_PMU_ACTION_OTG_DISABLE_VBUS_BOOST,       ///< Disable Vbus boost circuit 
	EM_PMU_ACTION_OTG_ENABLE_VBUS_PULSE,        ///< Enable Vbus pulsing
	EM_PMU_ACTION_OTG_DISABLE_VBUS_PULSE,       ///< Disable Vbus pulsing
	EM_PMU_ACTION_OTG_ENABLE_SRP_PD,            ///< Enable SRP Pull Down resistor on VBus
	EM_PMU_ACTION_OTG_DISABLE_SRP_PD,           ///< Disable SRP Pull Down resistor on VBus
	EM_PMU_ACTION_EXTERNAL_BOOST_ON,			///< Notify PMU that external BOOST is going to be turned on
	EM_PMU_ACTION_EXTERNAL_BOOST_OFF,			///< Notify PMU that external boost is off
	EM_PMU_ACTION_GET_OTG_ENV_STATUS,			///< Get VBUS/ID/OTG status to be used by OTG SW
	EM_PMU_ACTION_CLEAR_USB_OVERCURRENT_COUNT,
	EM_PMU_ACTION_SET_USB_ENUM_STARTED,
	EM_PMU_ACTION_CHECK_SELF_POWERED,
	EM_PMU_ACTION_MAX_ACTIONS
} HAL_EM_PMU_Action_en_t;

//--------------------------------------------------------------
/// ACTION typedefs
/// typedefs for (*data) and (*callback) parm for action keywords: 
/// HAL_EM_PMU_Ctrl( action, *data, *callback ) 
//--------------------------------------------------------------

//! Action: EM_PMU_ACTION_GET_POWERUPCAUSE
//! typedef for EM_PMU_Powerup_cb_t 
typedef enum {
    EM_PMU_POWERUP_POWERKEY,			///< powerup power key
    EM_PMU_POWERUP_ALARM,				///< powerup alarm
    EM_PMU_POWERUP_CHARGER,				///< powerup charger
    EM_PMU_POWERUP_ONREQ,				///< powerup on request
    EM_PMU_POWERUP_CHARGER_AND_ONREQ,	///< powerup charger and on request
    EM_PMU_POWERUP_SMPL,                ///< powerup SMPL
    EM_PMU_SYSTEM_RESET					///< system reset
} EM_PMU_PowerupId_en_t;

/**
	Action: EM_PMU_ACTION_GET_POWERUPCAUSE.  
	 
	Callback for HAL_EM_PMU_Action_PowerupCause_st_t parm typedef
*/
typedef void (*EM_PMU_Powerup_cb_t)( EM_PMU_PowerupId_en_t );

/** 
	Action: EM_PMU_ACTION_GET_POWERUPCAUSE.  

	Typedef of Data Structure (*data) to be passed as param to PMU driver to get powerup cause

	Refer to HAL_EM_PMU_Action_en_t
*/
typedef struct
{
	EM_PMU_Powerup_cb_t powerupCb;
} HAL_EM_PMU_Action_PowerupCause_st_t;

/** Action: EM_PMU_ACTION_ACTIVATE_SIMLDO.  

	Refer to HAL_EM_PMU_Action_en_t
*/
#if (defined(PMU_BCM59035) && defined(BCM59035_REV_B0)) || defined(PMU_BCM59036) || defined(PMU_BCM59038) || defined(PMU_MAX8986)
typedef enum {
      EM_PMU_SIM3P0Volt,	///< 3.0v
      EM_PMU_SIM2P5Volt,	///< 2.5v
      EM_PMU_SIM0P0Volt,	///< 0.0v
      EM_PMU_SIM1P8Volt		///< 1.8v
} EM_PMU_SIMVolt_en_t;
#else
typedef enum {
      EM_PMU_SIM3P0Volt,	///< 3.0v
      EM_PMU_SIM1P8Volt,	///< 1.8v
      EM_PMU_SIM0P0Volt		///< 0.0v
} EM_PMU_SIMVolt_en_t;
#endif

//! Typedef of Data Structure (*data) to be passed as param for EM_PMU_ACTION_ACTIVATE_SIMLDO action
typedef struct
{
	PMU_SIMLDO_t simldo;	///< SIM ID
	EM_PMU_SIMVolt_en_t voltage;			///< Sim LDO voltage selection
} HAL_EM_PMU_Action_ActivateSIM_st_t;

//! Typedef of Data Structure (*data) to be passed as param for EM_PMU_ACTION_IS_SIM_POWER_READY action
typedef struct
{
	PMU_SIMLDO_t simldo;	///< SIM ID
	Boolean simIsReady;
} HAL_EM_PMU_Action_IsSIMReady_st_t;

#if defined (PMU_BCM59055)
//! Typedef of Data Structure (*data) to be passed as param for EM_PMU_ACTION_SET_USB_CHARGER_TYPE action
typedef struct
{
	USB_Connector_Type_t usbChargerType;
} HAL_EM_PMU_Action_SetUsbChargerType_st_t;

//! Typedef of Data Structure (*data) to be passed as param for EM_PMU_ACTION_CTRL_ADP_BLOCK action
typedef struct
{
	Boolean enable;
} HAL_EM_PMU_Action_CtrlADPBlock_st_t;

#if defined (tempINTERFACE_UPDATE_PMU_HAL_ADP)

//! Typedef of Data Structure (*data) to be passed as param for EM_PMU_ACTION_CTRL_ADP_PROBE action
typedef struct
{
	Boolean start;
    Boolean adpOneShot;
	Boolean forceCalibration;
} HAL_EM_PMU_Action_CtrlADPProbe_st_t;
#endif

//! Typedef of Data Structure (*data) to be passed as param for EM_PMU_ACTION_CTRL_ADP_REF_PROBE action
typedef struct
{
	Boolean postEvent;
} HAL_EM_PMU_Action_ADPRefProbe_st_t;

//! Typedef of Data Structure (*data) to be passed as param for EM_PMU_ACTION_CTRL_ADP_SET_PROBE_CYCLE action
typedef struct
{
	Boolean shortCycle; //TRUE Short cycle; FALSE long cycle
} HAL_EM_PMU_Action_ADPSetProbeCycle_st_t;

//! Typedef of Data Structure (*data) to be passed as param for EM_PMU_ACTION_CTRL_ADP_SENSE action
typedef struct
{
	Boolean start;
    Boolean adpSnsAON;
} HAL_EM_PMU_Action_CtrlADPSense_st_t;

//! Typedef of Data Structure (*data) to be passed as param for EM_PMU_ACTION_IS_ADP_ATTACHED action
typedef struct
{
	Boolean isADPAttach;
} HAL_EM_PMU_Action_isADPAttach_st_t;

//! Typedef of Data Structure (*data) to be passed as param for EM_PMU_ACTION_IS_REMOTE_ADP_PROBING action
typedef struct
{
	Boolean isRemoteADPProbing;
} HAL_EM_PMU_Action_isRemoteADPProbing_st_t;


#if defined (tempINTERFACE_UPDATE_PMU_HAL_ADP)

//! Typedef of Data Structure (*data) to be passed as param for 
// EM_PMU_ACTION_GET_ADP_PROBE_MEASUREMENT_REGISTER_VALUES action
typedef struct
{
	ADP_Tprobe_t ADPTProbe;
} HAL_EM_PMU_Action_getADPMeasurementProbe_st_t;

//! Typedef of Data Structure (*data) to be passed as param for 
// EM_PMU_ACTION_CTRL_SET_TPROBE_MAX action
typedef struct
{
	UInt16 TProbeMax;
} HAL_EM_PMU_Action_setTprobeMax_st_t;

//! Typedef of Data Structure (*data) to be passed as param for 
// EM_PMU_ACTION_CTRL_SET_ADP_COMP_METHOD action
typedef struct
{
	UInt16	compMethod;
} HAL_EM_PMU_Action_setAdpCompMethod_st_t;
#endif

//! Typedef of Data Structure (*data) to be passed as param for EM_PMU_ACTION_IS_DETECT_LDO_ON action
typedef struct
{
	Boolean isDetectLdoOn;
} HAL_EM_PMU_Action_isDetectLdoOn_st_t;

//! Typedef of Data Structure (*data) to be passed as param for EM_PMU_ACTION_IS_BATTERY_WEAK action
typedef struct
{
	Boolean isBatteryWeak;
} HAL_EM_PMU_Action_isBatteryWeak_st_t;

//! Typedef of Data Structure (*data) to be passed as param for EM_PMU_ACTION_IS_DBP action
typedef struct
{
	Boolean isDeadBatteryProv;
} HAL_EM_PMU_Action_isDBP_st_t;

typedef struct
{
	Boolean start;			///< start (1) or stop (0)
} HAL_EM_PMU_Action_CtrlDetectLDO_st_t;

#endif

#if (defined(PMU_BCM59035) && defined(BCM59035_REV_B0)) || defined(PMU_BCM59036) || defined(PMU_BCM59038) || defined(PMU_BCM59055) || defined(PMU_MAX8986)
/** Action: EM_PMU_ACTION_ENABLE_NTC.  

	Refer to HAL_EM_PMU_Action_NTCEnable_st_t
*/

//! Typedef of Data Structure (*data) to be passed as param for EM_PMU_ACTION_ENABLE_NTC action

typedef enum {
      EM_PMU_NTC_OFF = 0,
      EM_PMU_NTC_ON
} EM_PMU_NTC_en_t;

typedef struct
{
	EM_PMU_NTC_en_t ntcOn;			///< NTC Enable/Disable
} HAL_EM_PMU_Action_NTCEnable_st_t;


/** Action: EM_PMU_ACTION_PAUSE_CHARGING.  

	Refer to HAL_EM_PMU_Action_PauseCharging_st_t
*/

//! Typedef of Data Structure (*data) to be passed as param for EM_PMU_ACTION_PAUSE_CHARGING action

typedef enum {
      EM_PMU_PAUSE_OFF = 0,
      EM_PMU_PAUSE_ON
} EM_PMU_Pause_en_t;

typedef struct
{
	EM_PMU_Pause_en_t pause;			///< Pause/Resume charging
} HAL_EM_PMU_Action_PauseCharging_st_t;

/** Action: EM_PMU_ACTION_SET_USB_CHARGING_VOLTAGE.  

	Refer to HAL_EM_PMU_Action_USBChargingVoltage_st_t
*/

//! Typedef of Data Structure (*data) to be passed as param for EM_PMU_ACTION_SET_USB_CHARGING_VOLTAGE action

#if !defined(PMU_BCM59055)
typedef enum {
      EM_PMU_USB_VOLTAGE_4P2V = 0,
      EM_PMU_USB_VOLTAGE_4P15V,
      EM_PMU_USB_VOLTAGE_4P1V,
      EM_PMU_USB_VOLTAGE_4P05V,
      EM_PMU_USB_VOLTAGE_4V,
      EM_PMU_USB_VOLTAGE_3P95V,
      EM_PMU_USB_VOLTAGE_3P9V,
      EM_PMU_USB_VOLTAGE_3P2V,
      EM_PMU_USB_VOLTAGE_3P3V,
      EM_PMU_USB_VOLTAGE_3P4V,
      EM_PMU_USB_VOLTAGE_3P5V,
      EM_PMU_USB_VOLTAGE_3P6V
} EM_PMU_USBVoltage_en_t;
#else
typedef enum {
      EM_PMU_USB_VOLTAGE_3P6V = 0,
      EM_PMU_USB_VOLTAGE_3P9V,
      EM_PMU_USB_VOLTAGE_4P0V,
      EM_PMU_USB_VOLTAGE_4P05V,
      EM_PMU_USB_VOLTAGE_4P1V,
      EM_PMU_USB_VOLTAGE_4P125V,
      EM_PMU_USB_VOLTAGE_4P15V,
      EM_PMU_USB_VOLTAGE_4P175V,
      EM_PMU_USB_VOLTAGE_4P2V,
      EM_PMU_USB_VOLTAGE_4P225V,
      EM_PMU_USB_VOLTAGE_4P25V,
      EM_PMU_USB_VOLTAGE_4P275V,
      EM_PMU_USB_VOLTAGE_4P3V,
      EM_PMU_USB_VOLTAGE_4P325V,
      EM_PMU_USB_VOLTAGE_4P35V,
      EM_PMU_USB_VOLTAGE_4P375V
} EM_PMU_USBVoltage_en_t;
#endif

typedef struct
{
	EM_PMU_USBVoltage_en_t usbVoltage;			///< charging voltage
} HAL_EM_PMU_Action_USBChargingVoltage_st_t;

/** Action: EM_PMU_ACTION_SET_WALL_CHARGING_VOLTAGE.  

	Refer to HAL_EM_PMU_Action_WallChargingVoltage_st_t
*/

//! Typedef of Data Structure (*data) to be passed as param for EM_PMU_ACTION_SET_WALL_CHARGING_VOLTAGE action

#if !defined(PMU_BCM59055)
typedef enum {
      EM_PMU_WALL_VOLTAGE_4P2V = 0,
      EM_PMU_WALL_VOLTAGE_4P15V,
      EM_PMU_WALL_VOLTAGE_4P1V,
      EM_PMU_WALL_VOLTAGE_4P05V,
      EM_PMU_WALL_VOLTAGE_4V,
      EM_PMU_WALL_VOLTAGE_3P95V,
      EM_PMU_WALL_VOLTAGE_3P9V,
      EM_PMU_WALL_VOLTAGE_3P2V,
      EM_PMU_WALL_VOLTAGE_3P3V,
      EM_PMU_WALL_VOLTAGE_3P4V,
      EM_PMU_WALL_VOLTAGE_3P5V,
      EM_PMU_WALL_VOLTAGE_3P6V
} EM_PMU_WallVoltage_en_t;
#else
typedef enum {
      EM_PMU_WALL_VOLTAGE_3P6V = 0,
      EM_PMU_WALL_VOLTAGE_3P9V,
      EM_PMU_WALL_VOLTAGE_4P0V,
      EM_PMU_WALL_VOLTAGE_4P05V,
      EM_PMU_WALL_VOLTAGE_4P1V,
      EM_PMU_WALL_VOLTAGE_4P125V,
      EM_PMU_WALL_VOLTAGE_4P15V,
      EM_PMU_WALL_VOLTAGE_4P175V,
      EM_PMU_WALL_VOLTAGE_4P2V,
      EM_PMU_WALL_VOLTAGE_4P225V,
      EM_PMU_WALL_VOLTAGE_4P25V,
      EM_PMU_WALL_VOLTAGE_4P275V,
      EM_PMU_WALL_VOLTAGE_4P3V,
      EM_PMU_WALL_VOLTAGE_4P325V,
      EM_PMU_WALL_VOLTAGE_4P35V,
      EM_PMU_WALL_VOLTAGE_4P375V
} EM_PMU_WallVoltage_en_t;
#endif

typedef struct
{
	EM_PMU_WallVoltage_en_t wallVoltage;			///< charging voltage
} HAL_EM_PMU_Action_WallChargingVoltage_st_t;

/** Action: EM_PMU_ACTION_SET_USB_CHARGING_CURRENT.  

	Refer to HAL_EM_PMU_Action_USBChargingCurrent_st_t
*/

//! Typedef of Data Structure (*data) to be passed as param for EM_PMU_ACTION_SET_USB_CHARGING_CURRENT action
#if defined(PMU_BCM59055)
typedef enum {
      EM_PMU_USB_CURRENT_100mA = 0,
      EM_PMU_USB_CURRENT_350mA,
      EM_PMU_USB_CURRENT_500mA,
      EM_PMU_USB_CURRENT_600mA,
      EM_PMU_USB_CURRENT_800mA,
      EM_PMU_USB_CURRENT_900mA

} EM_PMU_USBCurrent_en_t;
#elif  defined(PMU_MAX8986) 
typedef enum {
      EM_PMU_USB_CURRENT_200mA = 0,
      EM_PMU_USB_CURRENT_250mA,
      EM_PMU_USB_CURRENT_300mA,
      EM_PMU_USB_CURRENT_350mA,
      EM_PMU_USB_CURRENT_400mA,
      EM_PMU_USB_CURRENT_450mA,
      EM_PMU_USB_CURRENT_500mA,
      EM_PMU_USB_CURRENT_550mA,
      EM_PMU_USB_CURRENT_600mA,
      EM_PMU_USB_CURRENT_650mA,
      EM_PMU_USB_CURRENT_700mA,
      EM_PMU_USB_CURRENT_750mA,
      EM_PMU_USB_CURRENT_800mA,
      EM_PMU_USB_CURRENT_850mA,
      EM_PMU_USB_CURRENT_900mA,
      EM_PMU_USB_CURRENT_950mA,
      EM_PMU_USB_CURRENT_90mA
} EM_PMU_USBCurrent_en_t;
#else
typedef enum {
      EM_PMU_USB_CURRENT_50mA = 0,
      EM_PMU_USB_CURRENT_60mA,
      EM_PMU_USB_CURRENT_70mA,
      EM_PMU_USB_CURRENT_80mA,
      EM_PMU_USB_CURRENT_90mA,
      EM_PMU_USB_CURRENT_95mA,
      EM_PMU_USB_CURRENT_110mA,
      EM_PMU_USB_CURRENT_120mA,
      EM_PMU_USB_CURRENT_130mA,
      EM_PMU_USB_CURRENT_140mA,
      EM_PMU_USB_CURRENT_150mA,
      EM_PMU_USB_CURRENT_160mA,
      EM_PMU_USB_CURRENT_170mA,
      EM_PMU_USB_CURRENT_180mA,
      EM_PMU_USB_CURRENT_190mA,
      EM_PMU_USB_CURRENT_200mA,
      EM_PMU_USB_CURRENT_250mA,
      EM_PMU_USB_CURRENT_300mA,
      EM_PMU_USB_CURRENT_350mA,
      EM_PMU_USB_CURRENT_400mA,
      EM_PMU_USB_CURRENT_450mA,
      EM_PMU_USB_CURRENT_475mA,
      EM_PMU_USB_CURRENT_550mA,
      EM_PMU_USB_CURRENT_600mA,
      EM_PMU_USB_CURRENT_650mA,
      EM_PMU_USB_CURRENT_700mA,
      EM_PMU_USB_CURRENT_750mA,
      EM_PMU_USB_CURRENT_800mA,
      EM_PMU_USB_CURRENT_850mA,
      EM_PMU_USB_CURRENT_900mA,
      EM_PMU_USB_CURRENT_950mA,
      EM_PMU_USB_CURRENT_1000mA
} EM_PMU_USBCurrent_en_t;
#endif

typedef struct
{
	EM_PMU_USBCurrent_en_t usbCurrent;			///< charging current
} HAL_EM_PMU_Action_USBChargingCurrent_st_t;


/** Action: EM_PMU_ACTION_SET_WALL_CHARGING_CURRENT.  

	Refer to HAL_EM_PMU_Action_WallChargingCurrent_st_t
*/

//! Typedef of Data Structure (*data) to be passed as param for EM_PMU_ACTION_SET_WALL_CHARGING_CURRENT action
#if defined(PMU_BCM59055)
typedef enum {
      EM_PMU_WALL_CURRENT_100mA = 0,
      EM_PMU_WALL_CURRENT_350mA,
      EM_PMU_WALL_CURRENT_500mA,
      EM_PMU_WALL_CURRENT_600mA,
      EM_PMU_WALL_CURRENT_800mA,
      EM_PMU_WALL_CURRENT_900mA
} EM_PMU_WallCurrent_en_t;

#else
typedef enum {
      EM_PMU_WALL_CURRENT_50mA = 0,
      EM_PMU_WALL_CURRENT_60mA,
      EM_PMU_WALL_CURRENT_70mA,
      EM_PMU_WALL_CURRENT_80mA,
      EM_PMU_WALL_CURRENT_90mA,
      EM_PMU_WALL_CURRENT_100mA,
      EM_PMU_WALL_CURRENT_110mA,
      EM_PMU_WALL_CURRENT_120mA,
      EM_PMU_WALL_CURRENT_130mA,
      EM_PMU_WALL_CURRENT_140mA,
      EM_PMU_WALL_CURRENT_150mA,
      EM_PMU_WALL_CURRENT_160mA,
      EM_PMU_WALL_CURRENT_170mA,
      EM_PMU_WALL_CURRENT_180mA,
      EM_PMU_WALL_CURRENT_190mA,
      EM_PMU_WALL_CURRENT_200mA,
      EM_PMU_WALL_CURRENT_250mA,
      EM_PMU_WALL_CURRENT_300mA,
      EM_PMU_WALL_CURRENT_350mA,
      EM_PMU_WALL_CURRENT_400mA,
      EM_PMU_WALL_CURRENT_450mA,
      EM_PMU_WALL_CURRENT_500mA,
      EM_PMU_WALL_CURRENT_550mA,
      EM_PMU_WALL_CURRENT_600mA,
      EM_PMU_WALL_CURRENT_650mA,
      EM_PMU_WALL_CURRENT_700mA,
      EM_PMU_WALL_CURRENT_750mA,
      EM_PMU_WALL_CURRENT_800mA,
      EM_PMU_WALL_CURRENT_850mA,
      EM_PMU_WALL_CURRENT_900mA,
      EM_PMU_WALL_CURRENT_950mA,
      EM_PMU_WALL_CURRENT_1000mA
} EM_PMU_WallCurrent_en_t;
#endif

typedef struct
{
	EM_PMU_WallCurrent_en_t wallCurrent;			///< charging current
} HAL_EM_PMU_Action_WallChargingCurrent_st_t;
#endif

//! GPO pin selection enum for EM_PMU_ACTION_SET_GPO action
typedef enum {
	EM_PMU_GPO1 = 1,	///< Select GPO1 -- If GPO1 doesn't exist in PMU, HAL will return EM_PMU_ERROR_GPOPIN_UNAVAILABLE
	EM_PMU_GPO2 = 2,	///< Select GPO2 -- If GPO2 doesn't exist in PMU, HAL will return EM_PMU_ERROR_GPOPIN_UNAVAILABLE
	EM_PMU_GPO3 = 3		///< Select GPO3 -- If GPO3 doesn't exist in PMU, HAL will return EM_PMU_ERROR_GPOPIN_UNAVAILABLE 
} EM_PMU_GPOpin_en_t;
typedef enum {
	EM_PMU_CON_GPO1 = 1,	///< Select GPO1 -- If GPO1 doesn't exist in PMU, HAL will return EM_PMU_ERROR_GPOPIN_UNAVAILABLE
	EM_PMU_CON_GPO2 = 2,	///< Select GPO2 -- If GPO2 doesn't exist in PMU, HAL will return EM_PMU_ERROR_GPOPIN_UNAVAILABLE
} EM_PMU_GPOpin_conf_t;

//! GPO output type. If unavailable, then will return error EM_PMU_ERROR_GPO_OUTPUTTYPE_NOT_SUPPORTED
typedef enum {
	EM_PMU_GPO_ACTIVE_LOW  = 0,		///< active low
	EM_PMU_GPO_LED1_OUTPUT,			///< led1 output
	EM_PMU_GPO_LED2_OUTPUT,			///< led2 output
	EM_PMU_GPO_PWM1_OUTPUT,			///< pwm1 output
	EM_PMU_GPO_N_PWM1_OUTPUT,		///< pwm1_n output
	EM_PMU_GPO_PWM2_OUTPUT,			///< pwm2 output
	EM_PMU_GPO_N_PWM2_OUTPUT,		///< pwm2_n output
	EM_PMU_GPO_HIGH_IMPEDANCE		///< high impedance
} EM_PMU_GPOutput_en_t;
typedef enum {
	EM_PMU_GPO_INPUT  = 0,		///< Configure GPIO to be input
	EM_PMU_GPO_OUTPUT			///< Configure GPIO to be output
} EM_PMU_GPOtype_conf_t;

//! Typedef of Data Structure (*data) to be passed as param for EM_PMU_ACTION_SET_GPO action 
typedef struct
{
	EM_PMU_GPOpin_conf_t pin;			///< pin number or GPOx number
	EM_PMU_GPOtype_conf_t mode;		///< pin type: configure it as output/input/interrupt
} HAL_EM_PMU_Action_Conf_GPO_st_t;

typedef struct
{
	EM_PMU_GPOpin_en_t pinnumber;			///< pin number or GPOx number
	EM_PMU_GPOutput_en_t outputtype;		///< output type: configure it as PWM, LED, active low, or high impedance
} HAL_EM_PMU_Action_SetGPO_st_t;

//! Enum for EM_PMU_ACTION_IS_CHARGER_PRESENT
typedef enum {
	EM_PMU_NO_CHARGER_IS_PRESENT = 0,		///< No charger is plugged
	EM_PMU_USB_IS_PRESENT = 1,				///< USB charger/cable is present/plugged
	EM_PMU_WAC_IS_PRESENT = 2,				///< WAC (wall adapter charger) is present
	EM_PMU_USB_AND_WAC_ARE_PRESENT = 3		///< Both USB and WAC are present - impossible in 
											///< handset with one connector, but added for completeness
} EM_PMU_ChargerPresence_en_t;

//! Enum for I2C direct\\indirect call
typedef enum {
	EM_PMU_I2C_INDIRECTCALL = 0,		///< indirect call
	EM_PMU_I2C_DIRECTCALL	 = 1		///< direct call
} EM_PMU_I2CDirectCall_en_t;

//! (*callback) type for EM_PMU_ACTION_IS_CHARGER_PRESENT action
typedef void ( *EM_PMU_IsChargerPresent_cb_t )( EM_PMU_ChargerPresence_en_t );

//! (*data) type for EM_PMU_ACTION_IS_CHARGER_PRESENT  action
typedef struct
{
	EM_PMU_I2CDirectCall_en_t i2cdirectcall;		///< direct I2C call or indirect call (always use indirect 
													///< via I2C task, if used for non-critical purpose.)
} HAL_EM_PMU_Action_IsChargerPresent_st_t;
 
//!  (*data) type for  EM_PMU_ACTION_GET_ONKEY_STATUS
typedef struct
{
	Boolean isOnkeyDown;
} HAL_EM_PMU_Action_GetOnkeyStatus_st_t;

#if defined (PMU_BCM59001)
/** Action: EM_PMU_ACTION_SET_USB_CHARGING_CURRENT.  

	Refer to HAL_EM_PMU_Action_USBChargingCurrent_st_t
*/
typedef enum {
      EM_PMU_USB_CURRENT_500mA = 1,
      EM_PMU_USB_CURRENT_400mA,
      EM_PMU_USB_CURRENT_300mA,
      EM_PMU_USB_CURRENT_200mA,
      EM_PMU_USB_CURRENT_100mA,
      EM_PMU_USB_CURRENT_50mA
} EM_PMU_USBCurrent_en_t;

//! Typedef of Data Structure (*data) to be passed as param for EM_PMU_ACTION_SET_USB_CHARGING_CURRENT action
typedef struct
{
	EM_PMU_USBCurrent_en_t usbCurrent;			///< charging current
} HAL_EM_PMU_Action_USBChargingCurrent_st_t;

#endif

#if (defined(PMU_BCM59035) && defined(BCM59035_REV_B0)) || defined(PMU_BCM59036) || defined(PMU_BCM59038) || defined(PMU_BCM59055) || defined(PMU_MAX8986)
/** Action: EM_PMU_ACTION_CTRL_EOC_CURRENT.  

	Refer to HAL_EM_PMU_Action_en_t
*/
typedef enum{
    EM_PMU_EOCS_50MA,
    EM_PMU_EOCS_60MA,
    EM_PMU_EOCS_70MA,
    EM_PMU_EOCS_80MA,
    EM_PMU_EOCS_90MA,
    EM_PMU_EOCS_100MA,
    EM_PMU_EOCS_110MA,
    EM_PMU_EOCS_120MA,
    EM_PMU_EOCS_130MA,
    EM_PMU_EOCS_140MA,
    EM_PMU_EOCS_150MA,
    EM_PMU_EOCS_160MA,
    EM_PMU_EOCS_170MA,
    EM_PMU_EOCS_180MA,
    EM_PMU_EOCS_190MA,
    EM_PMU_EOCS_200MA
} EM_PMU_EOCS_en_t;

//! Typedef of Data Structure (*data) to be passed as param for EM_PMU_ACTION_CTRL_EOC_CURRENT action

#if !defined(PMU_BCM59055)
// **FIXME** for some reason, GCC barfs on this when compiling for Android
// ../modules/drivers/char/brcm/fuse_ril/CAPI2_CIB/sysinterface/hal/pmu/public/hal_pmu.h:679: warning: function declaration isn't a prototype
//../modules/drivers/char/brcm/fuse_ril/CAPI2_CIB/sysinterface/hal/pmu/public/hal_pmu.h:679: error: field 'get_current' declared as a function
#ifndef UNDER_LINUX
typedef struct
{
	EM_PMU_EOCS_en_t current2;			///< End of Charge Current selection
} HAL_EM_PMU_Action_CtrlEOCCurrent_st_t;
#endif
#else
typedef struct
{
	UInt8 current2;			///< End of Charge Current selection
} HAL_EM_PMU_Action_CtrlEOCCurrent_st_t;
#endif

/** Action: EM_PMU_ACTION_GET_CHARGER_IN_USE.  

	Refer to HAL_EM_PMU_Action_en_t
*/
#if defined(PMU_MAX8986)
typedef enum {
	EM_PMU_NO_CHARGER_IS_INUSE = 0,		///< No charger is in use
	EM_PMU_USB_IS_INUSE,					///< USB charger/cable is in use
	EM_PMU_WALL_IS_INUSE,					///< WALL charger is in use
    EM_PMU_SDP_500mA_IS_INUSE,             ///< Special 500mA charger: current 500mA max
    EM_PMU_SDP_1A_IS_INUSE,                ///< Special 1A charger: current 1A max
    EM_PMU_DCP_IN_INUSE                    ///< Dead Battery Carging - 100mA max
} EM_PMU_ChargerInUse_en_t;
#else
typedef enum {
	EM_PMU_NO_CHARGER_IS_INUSE = 0,		///< No charger is in use
	EM_PMU_USB_IS_INUSE,				///< USB charger/cable is in use
	EM_PMU_WALL_IS_INUSE				///< WALL charger is in use
} EM_PMU_ChargerInUse_en_t;
#endif
//!  (*data) type for  EM_PMU_ACTION_GET_CHARGER_IN_USE
typedef struct
{
	EM_PMU_ChargerInUse_en_t chargerInUse;
} HAL_EM_PMU_Action_GetChargerInUse_st_t;
#endif

/** Action: EM_PMU_ACTION_CTRL_MAINT_CHARGE.  

	Refer to HAL_EM_PMU_Action_en_t
*/
typedef enum{
    EM_PMU_MAINT_CHARGE_DISABLE,
    EM_PMU_MAINT_CHARGE_ENABLE
} EM_PMU_MaintChargeEnable_conf_t;

typedef enum{
    EM_PMU_MBMCVS_3P95,
    EM_PMU_MBMCVS_4P00,
    EM_PMU_MBMCVS_4P05,
    EM_PMU_MBMCVS_4P10
} EM_PMU_MaintChargeVoltage_conf_t;

//! Typedef of Data Structure (*data) to be passed as param for EM_PMU_ACTION_CTRL_MAINT_CHARGE action
typedef struct
{
    EM_PMU_MaintChargeEnable_conf_t enable;	 ///< configure it as enable or disable
	EM_PMU_MaintChargeVoltage_conf_t voltage;///< configure maintenance charge voltage
} HAL_EM_PMU_Action_CtrlMaintCharge_st_t;

//--------------------------------------------------------------
/// Result types or Error code
//--------------------------------------------------------------
//! HAL EM PMU driver function call result or error code
typedef enum
{
	EM_PMU_SUCCESS = 0,							///< Successful
	EM_PMU_ERROR_ACTION_NOT_SUPPORTED,			///< Not supported by platform HW or PMU device 
												///< (e.g, API is unavailable due to obselete device)
	EM_PMU_ERROR_INTERNAL_ERROR,				///< Internal error: i2c, comm failure, etc.
	EM_PMU_ERROR_PMU_HASBEEN_INITIALIZED,		///< PMU has been initialized once.
	EM_PMU_ERROR_EVENT_NOT_SUPPORTED,			///< Event is not supported by selected PMU device 
												///< or client exceeds max # client allowed.
	EM_PMU_ERROR_GPOPIN_UNAVAILABLE,			///< GPO pin does not exist in PMU (physically unavailable). 
	EM_PMU_ERROR_GPO_OUTPUTTYPE_NOT_SUPPORTED, 	///< GPO output type is not supported by PMU
	EM_PMU_ERROR_NO_REGISTERED_CLIENT,			///< No client is registered for event notification
	EM_PMU_ERROR_EVENT_HAS_A_CLIENT,			///< Event has a registered client. 
												///< This is for event with one client allowed only.
	EM_PMU_ERROR_OTHERS					   		///< Undefined error
} HAL_EM_PMU_Result_en_t;


//--------------------------------------------------------------
// PMU HAL Action Union type
//--------------------------------------------------------------

/**   PMU HAL Action Union Type  
*/
typedef union
{
	HAL_EM_PMU_Action_ActivateSIM_st_t			HAL_EM_PMU_Action_ActivateSIM;				 
	HAL_EM_PMU_Action_SetGPO_st_t 				HAL_EM_PMU_Action_SetGPO;					 
	HAL_EM_PMU_Action_IsChargerPresent_st_t 	HAL_EM_PMU_Action_IsChargerPresent;			 
	HAL_EM_PMU_Action_PowerupCause_st_t			HAL_EM_PMU_Action_PowerupCause;
	HAL_EM_PMU_Action_IsSIMReady_st_t			HAL_EM_PMU_Action_IsSIMReady;
#if defined (PMU_BCM59055)
	HAL_EM_PMU_Action_isDBP_st_t			    HAL_EM_PMU_Action_IsDBP;
    HAL_EM_PMU_Action_CtrlDetectLDO_st_t        HAL_EM_PMU_Action_CtrlDetectLDO;
#endif
} HAL_EM_PMU_Control_un_t;


//---------------------------------------------------------------------------
//   HAL_EM_PMU APIs 
//---------------------------------------------------------------------------

/**
*  This function will init pmu driver. 
*
*	@param			*data	(in)	address to "HAL_EM_PMU_Config_st_t" structure that user provides
*
*	@return			Error code as defined in   HAL_EM_PMU_Result_en_t
*/
HAL_EM_PMU_Result_en_t HAL_EM_PMU_Init(
	   HAL_EM_PMU_Config_st_t *data				
		);					

/**
*  This function will perform specific action from defined list, copy of parameters passed thru parm structure.
*
*	@param			action		(in)	action IDthe data structure required for the action. Pass NULL as default.												  
*	@param			data		(io)	address to data structure required for the action
*	@param			callback	(in)	Callback function associated with the action. Pass NULL as default.
*
*	@return			Error code as defined in   HAL_EM_PMU_Result_en_t    
*/
HAL_EM_PMU_Result_en_t HAL_EM_PMU_Ctrl(
	HAL_EM_PMU_Action_en_t action,			
	void *data,							
	void *callback						
);	

/**
*   This function shall be called by the higher layer to register callback routine for the
*   PMU hardware events
*
*	@param			eventID		(in) 	event ID user would like to subscribe if event occurs
*	@param			callback	(in)	callback function associated with event to be registered 
*
*	@return			HAL_EM_PMU_Result_en_t Error Code
*/
HAL_EM_PMU_Result_en_t HAL_EM_PMU_RegisterEventCB(
	PMU_DRV_Event_en_t 	eventID,		
	PMU_DRV_cb_t			callback					
	);

/**
* 	Function for lower layer: PMU driver to call for any occuring event
*
*	@param			eventID 		(in) event ID client wants to subscribe
*/
void HAL_EM_PMU_BroadcastEvent( PMU_DRV_Event_en_t eventID );
		
/**
* 	Function for AT cmd and test code
*/
void HAL_EM_PMU_PrintEventDataStruct( void );
#endif	// _HAL_L_HAL_EM_PMU_H__

/** @} */


