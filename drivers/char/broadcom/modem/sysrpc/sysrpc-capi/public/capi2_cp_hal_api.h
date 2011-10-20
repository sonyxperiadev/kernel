/****************************************************************************
*
*     Copyright (c) 2007-2008 Broadcom Corporation
*
*   Unless you and Broadcom execute a separate written software license 
*   agreement governing use of this software, this software is licensed to you 
*   under the terms of the GNU General Public License version 2, available 
*    at http://www.gnu.org/licenses/old-licenses/gpl-2.0.html (the "GPL"). 
*
*   Notwithstanding the above, under no circumstances may you combine this 
*   software in any way with any other Broadcom software provided under a license 
*   other than the GPL, without Broadcom's express prior written consent.
*
****************************************************************************/
/**
*
*   @file   capi2_cp_hal_api.h
*
*   @brief  This file defines the capi2 api's for Communication processor
*
****************************************************************************/
/**

*   @defgroup   CAPI2_CP_APIGroup   Communication Processor API
*
*   @brief      This group defines the interfaces from Communication processor
*               to Application processor for HAL specific modules.
****************************************************************************/

#ifndef CAPI2_CP_HAL_API_H
#define CAPI2_CP_HAL_API_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup CAPI2_CP_APIGroup
 * @{
 */

//#include "hal_pmu.h"

//***************************************************************************************
/**
	Function to get activate sim or start sim IO
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		simldo (in) SIM ID
	@param		volt (in) SIM voltage
	@return		None
	@note
	The response from AP to CP should be notified using CAPI2_PMU_ActivateSIM_RSP()
**/
void CAPI2_PMU_ActivateSIM(UInt32 tid, UInt8 clientID, PMU_SIMLDO_t simldo, PMU_SIMVolt_t volt);

//***************************************************************************************
/**
	Function response for the CAPI2_PMU_ActivateSIM
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@param		status (in) The response value
	@return		Not Applicable
	@note
	@n@b MsgType_t :	::MSG_PMU_ACTIVATE_SIM_RSP
	Return or Response is not applicable
**/
void CAPI2_PMU_ActivateSIM_RSP(UInt32 tid, UInt8 clientID, Boolean status);

//***************************************************************************************
/**
	Function to get SIM ready status
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		simldo (in) SIM ID
	@return		None
	@note
	The response from AP to CP should be notified using CAPI2_PMU_IsSIMReady_RSP()
**/
void CAPI2_PMU_IsSIMReady(UInt32 tid, UInt8 clientID, PMU_SIMLDO_t simldo);

//***************************************************************************************
/**
	Function response for the CAPI2_PMU_IsSIMReady
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@param		status (in) The response value
	@return		Not Applicable
	@note
	Return or Response is not applicable
**/
void CAPI2_PMU_IsSIMReady_RSP(UInt32 tid, UInt8 clientID, Boolean status);


//***************************************************************************************
/**
	Function to set the GPIO pin number (one of lower 32) to high
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		gpio_pin (in) input pin to set ( 0 : 31 )
	@return		None
	@note
	The response from AP to CP should be notified using CAPI2_GPIO_Set_High_64Pin_RSP()
**/
void CAPI2_GPIO_Set_High_64Pin(UInt32 tid, UInt8 clientID, UInt32 gpio_pin);

//***************************************************************************************
/**
	Function response for the CAPI2_GPIO_Set_High_64Pin
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@param		status (in) The response value
	@return		Not Applicable
	@note
	Return or Response is not applicable
	@n@b Response to CP will be notified via ::MSG_GPIO_SET_HIGH_64PIN_RSP
**/
void CAPI2_GPIO_Set_High_64Pin_RSP(UInt32 tid, UInt8 clientID, Boolean status);

//***************************************************************************************
/**
	Function to set the GPIO pin number (one of lower 32) to low
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		gpio_pin (in) input pin to set ( 0 : 31 )
	@return		None
	@note
	The response from AP to CP should be notified using CAPI2_GPIO_Set_Low_64Pin_RSP()
	@n@b Request MsgType_t :	::MSG_GPIO_SET_LOW_64PIN_REQ
**/
void CAPI2_GPIO_Set_Low_64Pin(UInt32 tid, UInt8 clientID, UInt32 gpio_pin);

//***************************************************************************************
/**
	Function response for the CAPI2_GPIO_Set_Low
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@param		status (in) The response value
	@return		Not Applicable
	@note
	Return or Response is not applicable
	@n@b Response to CP will be notified via ::MSG_GPIO_SET_LOW_64PIN_RSP
**/
void CAPI2_GPIO_Set_Low_64Pin_RSP(UInt32 tid, UInt8 clientID, Boolean status);

//***************************************************************************************
/**
	Function to start PMU charging
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The response from AP to CP should be notified using CAPI2_PMU_StartCharging_RSP()
**/
void CAPI2_PMU_StartCharging(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function response for the CAPI2_PMU_StartCharging
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@param		status (in) The response value
	@return		Not Applicable
	@note
	Return or Response is not applicable
	@n@b Response to CP will be notified via ::MSG_PMU_START_CHARGING_RSP
**/
void CAPI2_PMU_StartCharging_RSP(UInt32 tid, UInt8 clientID, Boolean status);

//***************************************************************************************
/**
	Function to stop PMU charging
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The response from AP to CP should be notified using CAPI2_PMU_StopCharging_RSP()
**/
void CAPI2_PMU_StopCharging(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function response for the CAPI2_PMU_StopCharging
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@param		status (in) The response value
	@return		Not Applicable
	@note
	Return or Response is not applicable
	@n@b Response to CP will be notified via ::MSG_PMU_STOP_CHARGING_RSP
**/
void CAPI2_PMU_StopCharging_RSP(UInt32 tid, UInt8 clientID, Boolean status);


//***************************************************************************************
/**
	Function to stop PMU charging
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The response from AP to CP should be notified using CAPI2_PMU_ClientPowerDown_RSP()
**/
void CAPI2_PMU_ClientPowerDown(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function response for the CAPI2_PMU_ClientPowerDown
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@param		status (in) The response value
	@return		Not Applicable
	@note
	Return or Response is not applicable
	@n@b Response to CP will be notified via ::MSG_PMU_CLIENT_POWER_DOWN_RSP
**/
void CAPI2_PMU_ClientPowerDown_RSP(UInt32 tid, UInt8 clientID, Boolean status);

//***************************************************************************************
/**
	Function to stop PMU charging
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The response from AP to CP should be notified using CAPI2_PMU_GetPowerupCause_RSP()
**/
void CAPI2_PMU_GetPowerupCause(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function response for the CAPI2_PMU_GetPowerupCause
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@param		powerupId (in) The response value
	@return		Not Applicable
	@note
	Return or Response is not applicable
	@n@b Response to CP will be notified via ::MSG_PMU_GET_POWERUP_CAUSE_RSP
**/
void CAPI2_PMU_GetPowerupCause_RSP(UInt32 tid, UInt8 clientID, EM_PMU_PowerupId_en_t powerupId);

//***************************************************************************************
/**
	Function to stop config 64 pin gpio output
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		pin (in) gpio pin
	@return		None
	@note
	The response from AP to CP should be notified using CAPI2_GPIO_ConfigOutput_64Pin_RSP()
**/
void CAPI2_GPIO_ConfigOutput_64Pin(UInt32 tid, UInt8 clientID, UInt32 pin);
//***************************************************************************************
/**
	Function response for the CAPI2_GPIO_ConfigOutput_64Pin
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@param		status (in) True or False
	@return		Not Applicable
	@note
	Return or Response is not applicable
	@n@b Response to CP will be notified via ::MSG_GPIO_CONFIG_OUTPUT_64PIN_RSP
**/
void CAPI2_GPIO_ConfigOutput_64Pin_RSP(UInt32 tid, UInt8 clientID, Boolean status);

//***************************************************************************************
/**
	Generic Function to send CP to AP GPS commands
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The response from AP to CP should be notified using CAPI2_GPS_Control_RSP()
**/
void CAPI2_GPS_Control(UInt32 tid, UInt8 clientID, UInt32 u32Cmnd, UInt32 u32Param0, UInt32 u32Param1, UInt32 u32Param2, UInt32 u32Param3, UInt32 u32Param4);
//***************************************************************************************
/**
	Function response for the CAPI2_GPS_Control
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@param		u32Param (in) The response value
	@return		Not Applicable
	@note
	Return or Response is not applicable
	@n@b Response to CP will be notified via ::MSG_GPS_CONTROL_RSP
**/
void CAPI2_GPS_Control_RSP(UInt32 tid, UInt8 clientID, UInt32 u32Param);

//***************************************************************************************
/**
	Function to control the FFS operation from CP to AP
	@param		cmd (in) Command to be executed ( zero is to save from RAM to FLASH )
	@param		address (in) The physical address in the shared memory
	@param		offset (in) The offset from the address
	@param		size (in) The data len
	@return		UInt32
	@note
**/
UInt32 CPPS_Control(UInt32 cmd, UInt32 address, UInt32 offset, UInt32 size);

//***************************************************************************************
/**
	Function to control the pedestal mode
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		enable (in) enable or disable
	@return		None
	@note
	The response from AP to CP should be notified using CAPI2_CP2AP_PedestalMode_Control_RSP()
**/
void CAPI2_CP2AP_PedestalMode_Control(UInt32 tid, UInt8 clientID, UInt32 enable);
//***************************************************************************************
/**
	Function response for the CAPI2_CP2AP_PedestalMode_Control
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@param		status (in) The response value
	@return		Not Applicable
	@note
	Return or Response is not applicable
	@n@b Response to CP will be notified via ::MSG_CP2AP_PEDESTALMODE_CONTROL_RSP
**/
void CAPI2_CP2AP_PedestalMode_Control_RSP(UInt32 tid, UInt8 clientID, Boolean status);

//***************************************************************************************
/**
	Function saves image to flash on AP
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@param		flash_addr (in) Dest Flash address
	@param		length (in) Length of data
	@param		shared_mem_addr (in) Src shared memory address
	@return		Not Applicable
	@note
	Return or Response is not applicable
	@n@b Response to CP will be notified via ::MSG_FLASH_SAVEIMAGE_RSP
**/
void CAPI2_FLASH_SaveImage(UInt32 tid, UInt8 clientID, UInt32 flash_addr, UInt32 length, UInt32 shared_mem_addr);

//***************************************************************************************
/**
	Function response for the CAPI2_FLASH_SaveImage
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@param		status (in) The response value
	@return		Not Applicable
	@note
	Return or Response is not applicable
	@n@b Response to CP will be notified via ::MSG_FLASH_SAVEIMAGE_RSP
**/
void CAPI2_FLASH_SaveImage_RSP(UInt32 tid, UInt8 clientID, Boolean status);

/** @} */


void CP2AP_GPIO_Set_High_64Pin(UInt32 gpio_pin);



#ifdef __cplusplus
}
#endif



#endif

