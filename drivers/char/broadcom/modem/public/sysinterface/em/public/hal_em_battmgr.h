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
*   @file   hal_em_battmgr.h
*
*   @brief  Global declaration of hardware abstraction layer for BATTMGR.
*
****************************************************************************/
/**
*   @defgroup   HAL_EM_BATTMGR   Battery Manager HAL
*   @ingroup    HALGroup	
*   @brief      HAL BATTMGR header file
*
*   Hardware Abstraction Layer of battery manager.   
*
****************************************************************************/
/**
 * @addtogroup HAL_EM_BATTMGR
 * @{
 */

#if !defined( _HAL_EM_BATTMGR_H__ )
#define _HAL_EM_BATTMGR_H__

// ---- Public Constants and Types  ----------------------------------------------------

/**  BATTMGR EVENT types
*
*	Upper layer can register a function to get notification on event below
*	Historically, only one client registered for callback (MMMI), therefore
*	this event management is designed to take one client only.
**/
typedef enum
{
	BATTMGR_CHARGER_PLUG_IN_EVENT,			///< Charger plug in event for both USB and Wall (basic notification of
											///< charging current existence to APP and
											///< show start charging dialog and animate batt icon).
											///< To be more speficic, ie. if it's USB or WAC, use PMU HAL.
	BATTMGR_CHARGER_PLUG_OUT_EVENT,			///< Charger plug out event
	BATTMGR_ENDOFCHARGE_EVENT,		   		///< End of Charge event. Battery is full - charging is done.
											///< APP uses this to show BATT FULL dialog box.
	BATTMGR_BATT_EXTREME_TEMP_EVENT,		///< BATT temp is outside window (safety) or extreme temperature (**obselete**)
	BATTMGR_LOW_BATT_EVENT,					///< BATT low is detected
	BATTMGR_EMPTY_BATT_EVENT,				///< BATT empty is detected
	BATTMGR_BATTLEVEL_CHANGE_EVENT			///< BATT level change is detected
} HAL_EM_BATTMGR_Events_en_t;


/**
*	HAL EM BATTMGR Level notification structure
**/
typedef struct
{
	HAL_EM_BATTMGR_Events_en_t	eventType; ///< The event type
	UInt8 inLevel;			///< The battery level, 0~N, depend the sysparm
	UInt16 inAdc_avg;		///< Adc value in mV. Ex, 4000 is 4.0V, 3800 is 3.8V
	UInt8 inTotal_levels;	///< total levels
}HAL_EM_BatteryLevel_t;

//! HAL EM BATTMGR charger type
typedef enum
{
	EM_BATTMGR_WALL_CHARGER = 0,
	EM_BATTMGR_USB_CHARGER
} HAL_EM_BATTMGR_Charger_t;

//! HAL EM BATTMGR charger in/out
typedef enum
{
	EM_BATTMGR_CHARGER_UNPLUGGED = 0,
	EM_BATTMGR_CHARGER_PLUGGED
} HAL_EM_BATTMGR_Charger_InOut_t;

/** 
	EM_BATTMGR CONFIGURATION AND INIT 
*/

/// EM_BATTMGR configuration and initialization data
typedef struct
{
	Boolean device_ACTIVE;			///< BATTMGR device active means it is initialized.
									///< BATTMGR active does not mean much for sleep mode. set to auto
	UInt32	performance_required;	///< performance required does not have much meaning on BATTMGR.  Set to default.
} HAL_EM_BATTMGR_Config_st_t;


//-------------------------------------------------------
// ACTIONS
//-------------------------------------------------------

/**  HAL_EM_BATTMGR Device supported ACTION
*
*	If action is not supported by the BATTMGR, function will return error.
*/
typedef enum
{
	EM_BATTMGR_ACTION_RUN_BATTMGR,					///< Run battmgr. Replacement for BATTMGR_Run
	EM_BATTMGR_ACTION_CONFIG_BATTMGR,				///<Element type {::HAL_EM_BATTMGR_Action_ConfigBattmgr_st_t},  
													///< Configure battery mgr's battery monitor with values from sysparms. Replacement for BATTMGR_Config
	EM_BATTMGR_ACTION_GET_BATTLEVEL_PERCENT,		///<Element type {::HAL_EM_BATTMGR_Action_BattLevelPercent_st_t}, Get battery level in percentage 
	EM_BATTMGR_ACTION_GET_BATTLEVEL,				///<Element type {::HAL_EM_BATTMGR_Action_BattLevel_st_t}, Get battery level in discrete units 
	EM_BATTMGR_ACTION_IS_USB_CHARGER_PRESENT,		///<Element type {::HAL_EM_BATTMGR_Action_IsUSBChargerPresent_st_t}, Is USB cable present?  
	EM_BATTMGR_ACTION_IS_WALL_CHARGER_PRESENT,		///<Element type {::HAL_EM_BATTMGR_Action_IsWallChargerPresent_st_t}, Is Wall charger present? 
	EM_BATTMGR_ACTION_GET_BATT_TEMP,				///< Get current reading of battery temperature
	EM_BATTMGR_ACTION_GET_CHARGING_STATUS,			///<Element type {::EM_BATTMGR_ChargingStatus_en_t}, Get charging status
	EM_BATTMGR_ACTION_SET_COMPENSATION,				///< Set compensation
  	EM_BATTMGR_ACTION_MAX_ACTIONS					///< Max supported action
} HAL_EM_BATTMGR_Action_en_t;

//! Action data type: EM_BATTMGR_ACTION_CONFIG_BATTMGR  
typedef struct 
{
	UInt16	bmr;				///< (in) parameter
	UInt16 	num_of_levels;			
	UInt16	*level_table;				
	UInt16	hysteresis;					
	UInt16	low_thresh;					
	UInt16	empty_thresh;	
} HAL_EM_BATTMGR_Action_ConfigBattmgr_st_t;		

//! **  Action data type:  EM_BATTMGR_ACTION_GET_BATTLEVEL	**
typedef struct 
{
	UInt16 battLevel;						   		///< (out) battery level
} HAL_EM_BATTMGR_Action_BattLevel_st_t;	   	

//! ** Action data type:  EM_BATTMGR_ACTION_GET_BATTLEVEL_PERCENT **
typedef struct 
{
	UInt16 battLevelPercent;						   	///< (out) battery level in percent
} HAL_EM_BATTMGR_Action_BattLevelPercent_st_t;	   	

//! **  Action data type:  EM_BATTMGR_ACTION_IS_USB_CHARGER_PRESENT	 **
typedef struct 
{
	Boolean isUSBChargerPresent;							///< (out) USB charger presence
} HAL_EM_BATTMGR_Action_IsUSBChargerPresent_st_t;	

//! **  Action data type:  EM_BATTMGR_ACTION_IS_WALL_CHARGER_PRESENT	 **
typedef struct 
{
	Boolean isWallChargerPresent;							///< (out) wall charger presence
} HAL_EM_BATTMGR_Action_IsWallChargerPresent_st_t;		

//! **  Action data type:  EM_BATTMGR_ACTION_GET_BATT_TEMP	 **
typedef struct 
{
	UInt16 channel_BattTempMon;							///< (out) Battery ADC channel
	UInt16 battTempLevelNumber;							///< (out) Battery temp level number
	UInt16 battTempLevelTable;							///< (out) Battery temp level value
	UInt16 battTemp_Low;								///< (out) Battery temp low threshold
	UInt16 battTemp_High;								///< (out) Battery temp high threshold
	UInt16 battTemp_RegainLow;							///< (out) Battery temp regain low threshold
	UInt16 battTemp_RegainHigh;							///< (out) Battery temp regain high threshold
	UInt16 runningBattTempAvg;							///< (out) Battery temperature
} HAL_EM_BATTMGR_Action_GetBatteryTemp_st_t;			

//! enum data for action type: EM_PMU_ACTION_GET_CHARGING_STATUS
typedef enum
{
	EM_BATTMGR_NOT_CHARGING = 0,		///< Not charging currently
	EM_BATTMGR_NORMAL_CHARGING,			///< Normal charging
	EM_BATTMGR_MAINTENANCE_CHARGING,	///< Maintenance charging
	EM_BATTMGR_ERROR_CHARGING			///< There is an error during charging 
} EM_BATTMGR_ChargingStatus_en_t;

//! (*data) type for EM_PMU_ACTION_GET_CHARGING_STATUS
typedef struct
{
	EM_BATTMGR_ChargingStatus_en_t	chargerStatus;
} HAL_EM_BATTMGR_Action_GetChargingStatus_st_t;	

//! **  Action data type:  EM_BATTMGR_ACTION_SET_COMPENSATION	**
typedef struct 
{
	Int16 compValue;						   		///< (out) battery level
} HAL_EM_BATTMGR_Action_SetCompensation_st_t;	   

//! **  Action data type:  EM_BATTMGR_ACTION_RUN_BATTMGR	**
typedef struct 
{
	UInt16 inBattVolt;						   		///< (out) battery level
} HAL_EM_BATTMGR_Action_RunBattmgr_st_t;	 

//-------------------------------------------------------
// EVENTS
//-------------------------------------------------------


//! Callback types for hardware events
typedef void (*HAL_EM_BATTMGR_Cb_t)( void );	///< Battery manager callback function for other than level change

//! Callback types for battery manager level status
typedef void (*HAL_EM_BATTMGR_BattLevelCB_t)(	///< Battery level status callback type
	UInt16 level,								///< Current battery level
	UInt16 adc_avg,								///< Running average of raw ADC values
	UInt16 total_levels							///< Total battery levels
	);

//! HAL EM BATTMGR function call result
typedef enum
{
	BATTMGR_SUCCESS = 0,						///< Successful
	BATTMGR_ERROR_ACTION_NOT_SUPPORTED,		///< Not supported by platform HW
	BATTMGR_ERROR_INTERNAL_ERROR,			///< Internal error: i2c, comm failure, etc.
	BATTMGR_ERROR_EVENT_HAS_A_CLIENT,		///< Error if trying to register more than  
												///< 1 client to an event with one client only requirement 
	BATTMGR_ERROR_OTHERS						///< Undefined error
} HAL_EM_BATTMGR_ErrorCode_en_t;

//!   Battmgr HAL Action Union  
typedef union
{
	HAL_EM_BATTMGR_Action_BattLevel_st_t				HAL_EM_BATTMGR_Action_BattLevel;					    
	HAL_EM_BATTMGR_Action_BattLevelPercent_st_t 		HAL_EM_BATTMGR_Action_BattLevelPercent;				   
	HAL_EM_BATTMGR_Action_IsUSBChargerPresent_st_t 		HAL_EM_BATTMGR_Action_IsUSBChargerPresent;				 
	HAL_EM_BATTMGR_Action_IsWallChargerPresent_st_t	   	HAL_EM_BATTMGR_Action_IsWallChargerPresent;						
} HAL_EM_BATTMGR_Control_un_t;

//---------------------------------------------------------------------------
//	HAL_EM_BATTMGR APIs  
//---------------------------------------------------------------------------

/**
*
*  This function will init BATTMGR's batt monitor state machine. 
*  @param		data    (io) device configuration structure
*
*  @return		HAL_EM_BATTMGR_ErrorCode_en_t
*
*****************************************************************************/
HAL_EM_BATTMGR_ErrorCode_en_t HAL_EM_BATTMGR_Init(
	   HAL_EM_BATTMGR_Config_st_t *data				///< (in/out) device configuration structure
	);					

/**
*
*  This function will perform specific action from defined list, copy of parameters passed thru parm structure.
*  @param		action      (in) Action request
*  @param		data        (io) Input/Output parameters associated with the action
*  @param		callback    (in) Callback function associated with the action
*
*  @return		HAL_EM_BATTMGR_ErrorCode_en_t
*
*****************************************************************************/
HAL_EM_BATTMGR_ErrorCode_en_t HAL_EM_BATTMGR_Ctrl(
	HAL_EM_BATTMGR_Action_en_t action,			///< (in) Action request
	void *data,									///< (io) Input/Output parameters associated with the action
	void *callback								///< (in) Callback function associated with the action
	);	

/**
*
*   This function shall be called by the higher layer to register callback routine for the
*   BATTMGR events
*  @param		event       (in) Event type
*  @param		callback    (in) Callback routine
*
*  @return		HAL_EM_BATTMGR_ErrorCode_en_t
*
*****************************************************************************/
HAL_EM_BATTMGR_ErrorCode_en_t HAL_EM_BATTMGR_RegisterEventCB(
	HAL_EM_BATTMGR_Events_en_t event,				///< (in) Event type
	void *callback								///< (in) Callback routine
	);

/** @} */

//------------------------------------------------------------------------------
// Global Function Prototypes
//------------------------------------------------------------------------------

// SET/GET static variables APIs to avoid global variables
void BATTMGR_SetVar_IsChargerPlugged( Boolean isPlugged );

// SET/GET static variables APIs to avoid global variables
Boolean  BATTMGR_GetVar_IsChargerPlugged( void );

// SET/GET static variables APIs to avoid global variables
void BATTMGR_SetVar_IsUSBChargerPlugged( Boolean isPlugged );

// SET/GET static variables APIs to avoid global variables
Boolean  BATTMGR_GetVar_IsUSBChargerPlugged( void );

void BATTMGR_Shutdown( void );			

// Check charger presence status
Boolean BATTMGR_IsChargerPlugIn(void);
			
// Set static var IsChargingError, if any charging error occurs at pmu driver level
void BATTMGR_SetVar_IsChargingError( Boolean isError );

// Set static var IsEOCTriggered, if EOC interrupt is fired at pmu driver level -- to notify battmgr
void BATTMGR_SetVar_IsEOCTriggered( Boolean isTriggered );
			
// Manual update of leve_cb which include the battery voltage. Previously this was only available through event callback.
void HAL_EM_BATTMGR_BattADCReq(void);

			
#endif	// _HAL_L_HAL_EM_BATTMGR_H__


