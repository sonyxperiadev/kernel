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


#ifndef _PMU_MEMMAP_INC_	//Temp code to avoid multiple def in pmu.h

 typedef enum {
    PMU_POWERUP_POWERKEY = 0,
    PMU_POWERUP_ALARM,
    PMU_POWERUP_CHARGER,
    PMU_POWERUP_ONREQ,
    PMU_POWERUP_CHARGER_AND_ONREQ,
    PMU_SYSTEM_RESET
} PMU_PowerupId_t;

#endif

#define FFS_LCS_LTO_FILE_ID         0x01        ///< The LTO file ID
#define LCS_MAX_LTO_SIZE            63*1024     ///< Maximum LTO file size

/// The input parameter structure passed for CAPI2_FFS_Read().
typedef struct
{
	Int32 mFileId;      ///< Id of the file to be read.
	UInt32 mMaxLength; ///< The maximum length of data to be read from the file.
} FFS_ReadReq_t;

///The input parameter structure passed forCAPI2_FFS_Read_RSP()
typedef struct
{
    UInt8* mData;       ///< The data buffer
    UInt32 mDataLen;    ///< Length of the data in mData.
}FFS_Data_t;

 /**
 * @addtogroup CAPI2_CP_APIGroup
 * @{
 */

//***************************************************************************************
/**
	Function to get activate sim or start sim IO
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		volt (in) SIM voltage
	@return		None
	@note
	The response from AP to CP should be notified using CAPI2_PMU_ActivateSIM_RSP()
**/
void CAPI2_PMU_ActivateSIM(UInt32 tid, UInt8 clientID, Int32 volt);

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
	@return		None
	@note
	The response from AP to CP should be notified using CAPI2_PMU_IsSIMReady_RSP()
**/
void CAPI2_PMU_IsSIMReady(UInt32 tid, UInt8 clientID);

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
	Function to Deactivate Sim
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The response from AP to CP should be notified using CAPI2_PMU_DeactivateSIM_RSP()
**/
void CAPI2_PMU_DeactivateSIM(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function response for the CAPI2_PMU_DeactivateSIM
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@param		status (in) The response value
	@return		Not Applicable
	@note
	Return or Response is not applicable
**/
void CAPI2_PMU_DeactivateSIM_RSP(UInt32 tid, UInt8 clientID, Boolean status);

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
void CAPI2_PMU_GetPowerupCause_RSP(UInt32 tid, UInt8 clientID, PMU_PowerupId_t powerupId);

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
	@param		u32Cmnd (in) Command ID is TBD
	@param		u32Param0 (in) param is TBD
	@param		u32Param1 (in) param is TBD
	@param		u32Param2 (in) param is TBD
	@param		u32Param3 (in) param is TBD
	@param		u32Param4 (in) param is TBD
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
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		cmd (in) Command to be executed ( zero is to save from RAM to FLASH )
	@param		address (in) The physical address in the shared memory
	@param		offset (in) The offset from the address
	@param		size (in) The data len
	@return		None
	@note
	The response from AP to CP should be notified using CAPI2_FFS_Control_RSP()
**/
void CAPI2_FFS_Control(UInt32 tid, UInt8 clientID, UInt32 cmd, UInt32 address, UInt32 offset, UInt32 size);
//***************************************************************************************
/**
	Function response for the CAPI2_FFS_Control
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@param		param (in) The response value
	@return		Not Applicable
	@note
	Return or Response is not applicable
	@n@b Response to CP will be notified via MSG_FFS_CONTROL_RSP
**/
void CAPI2_FFS_Control_RSP(UInt32 tid, UInt8 clientID, UInt32 param);

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
	Function to read a file.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		ffsReadReq (in) The read request parameter.
	@return		None
	@note
	The response from AP to CP should be notified using CAPI2_FFS_Read_RSP()
**/
void CAPI2_FFS_Read(UInt32 tid, UInt8 clientID, FFS_ReadReq_t *ffsReadReq);

//***************************************************************************************
/**
	Function response for the CAPI2_FFS_Read
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@param		ffsReadRsp (in) The response data.
	@return		Not Applicable
	@note
	Return or Response is not applicable
	@n@b Response to CP will be notified via MSG_FFS_READ_RSP
**/
void CAPI2_FFS_Read_RSP(UInt32 tid, UInt8 clientID, FFS_Data_t *ffsReadRsp);

//***************************************************************************************
/**
	The API function on CP side to read a file.
	@param		inReadReq (in) The read request parameter.
    @param		ioDataBuf (in/out) Poiter of the buffer that will hold the read result.
	@return		The length of the read data.
**/
UInt32 FFS_Read(FFS_ReadReq_t inReadReq, UInt8* ioDataBuf);

/** @} */


#ifdef __cplusplus
}
#endif



#endif

