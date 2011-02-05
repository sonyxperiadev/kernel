/*******************************************************************************************
Copyright 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement
governing use of this software, this software is licensed to you under the
terms of the GNU General Public License version 2, available at
http://www.gnu.org/copyleft/gpl.html (the "GPL").

Notwithstanding the above, under no circumstances may you combine this software
in any way with any other Broadcom software provided under a license other than
the GPL, without Broadcom's express prior written consent.
*******************************************************************************************/

//***************************************************************************
/**
*
*  @file   atc_kernel.h
*
*  @brief  Interface to the kernel atc kernel driver.
*
*   @note	This driver is typically used for handling AT command through capi2 api.
*
****************************************************************************/


#ifndef __ATC_KERNEL_H
#define __ATC_KERNEL_H

#ifdef __cplusplus
extern "C" 
{
#endif // __cplusplus

#define ATC_KERNEL_MODULE_NAME  "bcm_atc"
/**
 *
 *  ioctl commands
 *
 **/
#define ATC_KERNEL_SEND_AT_CMD		100					///<	Send AT command to CP 
#define ATC_KERNEL_Get_AT_RESP		101					///<	Get AT command resp from CP 

#define ATC_KERNEL_RESULT_BUFFER_LEN_MAX 1000	///< result max byte

/**
 *  for ioctl cmd ATC_KERNEL_SEND_AT_CMD, a variable of this type
 *	is passed as the 'arg' to ioctl()
 */
typedef struct
{
  	uint8_t fChan;							///<	at channel
	char* fATCmdStr;						///<	at command string to be processed
}	ATC_KERNEL_ATCmd_t ;


/**
 *  for ioctl cmd ATC_KERNEL_Get_AT_RESP, a variable of this type
 *	is passed as the 'arg' to ioctl()
 */
typedef struct
{
	char* fATRespStr;						///<	at command response
}	ATC_KERNEL_ATResp_t ;


#ifdef __cplusplus
}
#endif // __cplusplus


#endif // __ATC_KERNEL_H 
