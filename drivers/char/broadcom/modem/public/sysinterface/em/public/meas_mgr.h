/******************************************************************************
*************
Copyright 2008 Broadcom Corporation.  All rights reserved.
This program is the proprietary software of Broadcom Corporation and/or its licensors, and may 
only be used, duplicated, modified or distributed pursuant to the terms and conditions of a 
separate, written license agreement executed between you and Broadcom (an "Authorized 
License").

Except as set forth in an Authorized License, Broadcom grants no license(express or implied), 
right to use, or waiver of any kind with respect to the Software, and Broadcom expressly 
reserves all rights in and to the Software and all intellectual property rights therein.  IF YOU HAVE 
NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, 
AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
  
 Except as expressly set forth in the Authorized License,
1. This program, including its structure, sequence and organization, constitutes the valuable trade 
secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof, 
and to use this information only in connection with your use of Broadcom integrated circuit products.

2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS" AND WITH 
ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR WARRANTIES, EITHER 
EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM
SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, 
NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR 
COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO 
DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF 
THE SOFTWARE.

3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS 
LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR 
EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR 
USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF 
THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY 
PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL 
APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
*********************************************************************************
**********/

/**
*
*  @file   meas_mgr.h
*
*  @brief  This file contains the public information related to the meas_mgr
*          component.
*	     
*
*  @note  
*         
*         
*
****************************************************************************/
//! Configure and status info
typedef struct
{
	UInt16 	adc_ch; 	///< ADC channel
	UInt16	adc_trg;	///< ADC sample trigger
	UInt16	o_rd_data;	///< output ADC read data
}MeasMngrCnfg_t;

//! Function Eror results 
typedef enum
{
	MEAS_RESULT_PENDING,	///< Pending result.
	MEAS_RESULT_COMPLETE,	///< Complete result
	MEAS_RESULT_ERROR		///< Error result
} MeasMgrResult_t;

//! Function Error for Database
typedef enum
{
	MEAS_DB_RESULT_OK,		///< Complete/OK result
	MEAS_DB_RESULT_INIT,	///< Init State result
	MEAS_DB_RESULT_TIMEOUT,	///< Timed Out result
	MEAS_DB_RESULT_ERROR	///< Error result
} MeasMgrDbaseResult_t;


//***************************************************************************
/**
 *  Function Name:   MEASMGR_Init
 *   Description: 	Initialize the Measurement Manager Task, DEEP_SLEEP.
 *	 @return  void
 * 			
 */
void MEASMGR_Init(void);
//***************************************************************************
/**
 *  Function Name:   MEASMGR_ShutDown
 *   Description: 	Disables the Measurement Manager Task, DEEP_SLEEP.
 *	 @return  void
 * 			
 */
void MEASMGR_ShutDown(void);
//*********************************************************************
/**
*
*   Description: MEASMGR_WakeMeasService is called by the RF DRX signal & 
*	expiring_of_5sec_timer.
*	This function retrieves the Wakes the Measurement Manager Task.
*   @param void
*
*   @return   Result_t          Function status
*   @note      Releases the completed semaphore to allow the Measurement Manager to
*			run.
*
**********************************************************************/
void MEASMGR_WakeMeasService( void );
//*********************************************************************
/**
*
*   Description: MEASMGR_RunMeas performs the MeasureMent Manager Service.
*	This function retrieves the results of the required ADCs and  calls the basic services.
*   @param void
*
*   @param    void
*   @return   Result_t           Result_ok,ERROR
*   @note      
*
**********************************************************************/
void MEASMGR_Task(void);

//*********************************************************************
/**
*
*   Description: MEASMGR_GetDataB_Adc API is called by the user to retrieve the output 
*		ADC read data for each channel that was updated by the Measurement manager.
*
*	This function is ran after the DRX signal is received from the CP.
*   @param void
*
*   @return   MeasMgrDbaseResult_t          Function status
*   @param1    HAL_ADC_rd_cnfg_st_t		pointer to ADC read configuration
*   @note      Releases the completed semaphore to allow the Measurement Manager to
*			run.
*
**********************************************************************/

MeasMgrDbaseResult_t MEASMGR_GetDataB_Adc(MeasMngrCnfg_t* pIoAdc);



