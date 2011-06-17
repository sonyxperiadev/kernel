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
*   @file   hal_pmu_private.h
*
*   @brief  contains APIs only meant to be used by other HALs, not to be used directly
*
*   It contains APIs for functions that are only SOMETIMES in the PMU and sometimes
*	elsewhere in the platform.
*
****************************************************************************/
/**
*   @defgroup   HAL_PMU_Private   HAL PMU Private
*   @ingroup    HALGroup	
*   @brief      HAL PMU Control header file for private functions
*
*   Hardware Abstraction Layer of PMU.   
*
****************************************************************************/

#if !defined( _HAL_PMU_PRIVATE_H__ )
#define _HAL_PMU_PRIVATE_H__

//#include "pmu.h"
// ---- Include Files -------------------------------------------------------
//#include "consts.h"
//#include "mobcom_types.h"
//#include "chip_version.h"
//#include "hal_pmu.h"

// ---- Constants and Types  ----------------------------------------------------

//--------------------------------------------------------------
/// ACTION PARAMETERS definitions
//--------------------------------------------------------------

/**
 * @addtogroup HAL_PMU_Private
 * @{
 */

/**  HAL_EM_PMU Device ACTION PRIVATE definitions
*
*	If action is not supported by the pmu device on-board, function will return error.
*/
typedef enum
{
	EM_PMU_SET_REGISTER,				///< Set a pmu register. Wrapper for PMU_Write
	EM_PMU_GET_REGISTER,				///< Get a pmu register. Wrapper for PMU_Read
	EM_PMU_ACTION_SET_LED,				///< Set LED 
	EM_PMU_SET_PWM,						///< Set PWM in pmu
	EM_PMU_ACTION_KEYLIGHT,				///< Set keylight 
	EM_PMU_ACTION_BACKLIGHT,			///< Set LCD backlight if controlled by pmu (return error if not)
	EM_PMU_ACTION_ADC_GET_DATA,			///< Get continuous mode ADC data
	EM_PMU_ACTION_ADC_GET_MULT_DATA,	///< Get continuous mode multiple ADC data
	EM_PMU_ACTION_ADC_ENABLE_GSMBURST,	///< Enable GSM debounce conversion
	EM_PMU_ACTION_ADC_DISABLE_GSMBURST,	///< Disable GSM burst conversion
	EM_PMU_ACTION_RTM_SET_DELAY,		///< Set the RTM delay
	EM_PMU_ACTION_RTM_SET_CH,			///< Set the RTM channel
	EM_PMU_ACTION_RTM_GET_DATA,			///< Get RTM ADC data for the selected channel
	EM_PMU_ACTION_IHF_BYPASS,			///< Enable/Disable IHF Bypass mode
	EM_PMU_ACTION_IHF_SET_GAIN,			///< Set gain for IHF	
	EM_PMU_ACTION_IHF_POWER_UP,			///< Power up IHF block
	EM_PMU_ACTION_IHF_POWER_DOWN,		///< Power down IHF block
	EM_PMU_ACTION_HS_SET_INPUT_MODE,	///< Set HS input mode differential/single ended
	EM_PMU_ACTION_HS_CLASS_SEL_METHOD,	///< Headset Class AB vs G selection method
	EM_PMU_ACTION_HS_POWER,				///< Headset power up/down
	EM_PMU_ACTION_HS_SET_GAIN,			///< Set gain for Headset
	EM_PMU_ACTION_HS_SHTCKT_THRESH,		///< Set Headset short circuit threshold
	EM_PMU_ACTION_HS_SHTCKT,			///< Enable/Disable Headset short circuit detection
	EM_PMU_ACTION_USB_CHARGER_DETECTION,
	EM_PMU_ACTION_USB_DCD_ENABLE,
	EM_PMU_ACTION_MUIC_DETECTION,
	EM_PMU_ACTION_USN_HS_ENABLE,
	EM_PMU_ACTION_ACTIVE_POWER_SUPPLY,
	EM_PMU_ACTION_DACTIVE_POWER_SUPPLY,
	EM_PMU_ACTION_EXT_SPEAKER_PREAMP_PGA,
	EM_PMU_MAX_ACTIONS_PRIVATE
} HAL_EM_PMU_Action_Private_en_t;

//! ** EM_PMU_SET_LED **
//! Select ID of LED. Typically, PMU only provides 2 LED drivers.  Please select ID1 as default if only 1 LED is used on board. 
typedef enum{
    EM_PMULED_ID1,		///< Default value, if only 1 LED is used on the board.  
    EM_PMULED_ID2,		///< To access the second LED, if board uses it.
    EM_PMULED_ID_MAX
} EM_PMULedID_en_t;

//! LED cycle time or repeat period every [cycle] sec. 
//! LED will light on with pattern set as EM_PMU_PMULedPattern_t
//! Then, wait as long as [EM_PMU_PMULedCycle_t] sec, before pattern repeats again. 
typedef enum{
    EM_PMU_PMULED_CYC_0p4s,		///< 0.4sec cycle time
    EM_PMU_PMULED_CYC_1s,		///< 1sec cycle time
    EM_PMU_PMULED_CYC_1p2s,		///< 1.2sec	cycle time
    EM_PMU_PMULED_CYC_2s,		///< 2sec cycle time
    EM_PMU_PMULED_CYC_2p6s,		///< 2.6s cycle time
    EM_PMU_PMULED_CYC_4s,		///< 4sec cycle time
    EM_PMU_PMULED_CYC_6s,		///< 6sec cycle time
    EM_PMU_PMULED_CYC_8s		///< 8sec cycle time
} EM_PMU_PMULedCycle_en_t;


typedef enum{
    EM_PMU_PMULED_PATT_On50msOff,					///< ON 50ms, OFF for the rest of period
    EM_PMU_PMULED_PATT_On100msOff,					///< ON 100ms, OFF 	for the rest of period	
    EM_PMU_PMULED_PATT_On200msOff,					///< ON 200ms, OFF 	for the rest of period
    EM_PMU_PMULED_PATT_On500msOff,					///< ON 500ms, OFF 	for the rest of period
    EM_PMU_PMULED_PATT_On50msOff50msOn50msOff,		///< ON 50ms, OFF 50ms, ON 50ms, OFF rest of period
    EM_PMU_PMULED_PATT_On100msOff100msOn100msOff,	///< ON 100ms, OFF 100ms, ON 100ms, OFF rest of period
    EM_PMU_PMULED_PATT_On200msOff200msOn200msOff,	///< ON 200ms, OFF 200ms, ON 200ms, OFF rest of period
    EM_PMU_PMULED_PATT_On                       	///< Always ON
} EM_PMU_PMULedPattern_en_t;

typedef enum {
	EM_PMU_PMULED_OPER_OFF,			///< Turn off LED
	EM_PMU_PMULED_OPER_CHARGER,		///< LED is on when charger is plugged. BCM PMU does not support. Will return error, if called. 
	EM_PMU_PMULED_OPER_ACTIVE		///< Turn on LED
} EM_PMU_PMULedOperMode_en_t;

typedef enum {
	EM_PMU_PWMID1,			///< ID of PWM1
	EM_PMU_PWMID2		///< ID of PWM 2
} EM_PMU_PWMID_t;

//! *data
typedef struct
{
	EM_PMULedID_en_t			ledID;		///< ID of the LED
	EM_PMU_PMULedCycle_en_t		cycle;		///< LED cycle as defined in typedef  
	EM_PMU_PMULedPattern_en_t	pattern;	///< LED pattern as defined in typedef 
	EM_PMU_PMULedOperMode_en_t	opmode;		///< operation mode. Active, active only when chger plugged, or off
} HAL_EM_PMU_Action_Private_SetLED_st_t;

//--------------------------------------------------------------
/// Result types or Error code
//--------------------------------------------------------------
//! HAL EM PMU driver function call result or error code
typedef enum
{
	EM_PMU_PRV_SUCCESS = 0,							///< Successful
	EM_PMU_PRV_ERROR_ACTION_NOT_SUPPORTED,			///< Not supported by platform HW or PMU device 
												///< (e.g, API is unavailable due to obselete device)
	EM_PMU_PRV_ERROR_INTERNAL_ERROR,				///< Internal error: i2c, comm failure, etc.
	EM_PMU_PRV_ERROR_PMU_HASBEEN_INITIALIZED,		///< PMU has been initialized once.
	EM_PMU_PRV_ERROR_EVENT_NOT_SUPPORTED,			///< Event is not supported by selected PMU device 
												///< or client exceeds max # client allowed.
	EM_PMU_PRV_ERROR_GPOPIN_UNAVAILABLE,			///< GPO pin does not exist in PMU (physically unavailable). 
	EM_PMU_PRV_ERROR_GPO_OUTPUTTYPE_NOT_SUPPORTED, 	///< GPO output type is not supported by PMU
	EM_PMU_PRV_ERROR_NO_REGISTERED_CLIENT,			///< No client is registered for event notification
	EM_PMU_PRV_ERROR_EVENT_HAS_A_CLIENT,			///< Event has a registered client. 
												///< This is for event with one client allowed only.
	EM_PMU_PRV_ERROR_OTHERS					   		///< Undefined error
} HAL_EM_PMU_Result_Private_en_t;

//! ** EM_PMEM_PMU_KEYLIGHT	**
//! (*data) type to be passed
typedef struct
{
	Boolean     StatusOn;		///< on/off  
	UInt8       level;			///< 0 to 255 intensity value 
} HAL_EM_PMU_Action_Private_SetKeylight_st_t;


typedef struct
{
	EM_PMU_PWMID_t  PWMID;		///< ID of PWM
	Boolean     StatusOn;		///< on/off  
	UInt8       level;			///< 0 to 255 intensity value 

}HAL_EM_PMU_Action_Private_PWM_st_t;

//! ** EM_PMU_BACKLIGHT **
//! (*data) type to be passed
typedef struct
{
	Boolean     StatusOn;	   	///< on/off  
	UInt8       level; 		   	///< 0 to 255 intensity value
	UInt32 		ramping_time;  	///< total ramping on/off time (ms)
} HAL_EM_PMU_Action_Private_SetBacklight_st_t;


						   

#if defined(PMU_BCM59055)
//! Callback routine definition for ID change event
typedef void (*HAL_EM_PMU_IdChange_cb_t)(PMU_OTG_Id_t data1, PMU_OTG_Id_t data2);
#endif
//--------------------------------------------------------------
///   HAL_EM_PMU APIs
//--------------------------------------------------------------

/**
*  This function will perform specific PRIVATE action from defined list, copy of parameters passed thru parm structure.
*	@param			action		(in)	action ID 
*	@param			data		(in)	address to the data structure required for the action. Pass NULL as default.
*	@param			callback	(in)	Callback function associated with the action. Pass NULL as default.
*
*	@return			Error code as defined in   HAL_EM_PMU_Result_en_t 
*/
HAL_EM_PMU_Result_Private_en_t HAL_EM_PMU_Ctrl_Private(
	HAL_EM_PMU_Action_Private_en_t action,			///< (in) Action request
	void *data,										///< (io) Input/Output parameters associated with the action
	void *callback									///< (in) Callback function associated with the action
);	

/**
*   This function shall be called by the higher layer to register callback routine for the
*   PMU hardware events
*	@param			eventID		(in) 	event ID user would like to subscribe if event occurs
*	@param			callback	(in)	callback function associated with event to be registered 
*	@return			Error code as defined in   HAL_EM_PMU_Result_en_t 
*/
HAL_EM_PMU_Result_Private_en_t HAL_EM_PMU_RegisterEventCB_Private(
	PMU_DRV_Event_Private_en_t 	eventID,		///< (in) Event type
	PMU_DRV_Private_cb_t			callback					///< (in) Callback routine
	);

/**
*	Function for lower layer: PMU driver to call for any occuring event
*	@param			eventID		(in)	event ID client wants to subscribe 
*	@note 			Called by PMU driver. Architecturally, PMU driver only need to call 
*					this function when an event occurs for the purpose of driver simplicity. 
*					HAL PMU job is to broadcast to its HAL peers and/or upper layer clients
*/
void HAL_EM_PMU_BroadcastPrivateEvent( PMU_DRV_Event_Private_en_t eventID );

#if defined(PMU_BCM59055)
/**
*   This function shall be called by the higher layer to register callback routine for the
*   PMU ID change event
*	@param			callback	(in)	callback function associated with event to be registered 
*	@return			Error code as defined in   HAL_EM_PMU_Result_en_t 
*/
HAL_EM_PMU_Result_Private_en_t HAL_EM_PMU_RegisterIDChangEventCB(
	HAL_EM_PMU_IdChange_cb_t			callback				
	);

/**
*   Function for lower layer: PMU driver to call for ID change event
*	@param			prevID	(in)  previous ID state
*	@param			currID	(in)  current ID state
*	@note 			Called by PMU driver. Architecturally, PMU driver only need to call 
*					this function when an event occurs for the purpose of driver simplicity. 
*					HAL PMU job is to broadcast to its HAL peers and/or upper layer clients
*/
void HAL_EM_PMU_BroadcastIdEvent( PMU_OTG_Id_t prevID, PMU_OTG_Id_t currID );
#endif

/**
* 	HAL EM PMU Private module init function
*/
void HAL_EM_PMU_Init_Private( void );

/** @} */

#endif	// _HAL_L_HAL_EM_PMU_H__



