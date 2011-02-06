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
*  @file   xscript_kernel.h
*
*  @brief  Interface to the xscript kernel driver.
*
*   @note	This driver is typically used for handling xScript kernel APIs.
*
****************************************************************************/


#ifndef XSCRIPT_KERNEL_H__
#define XSCRIPT_KERNEL_H__

#ifdef __cplusplus
extern "C" 
{
#endif // __cplusplus

#define XSCRIPT_KERNEL_MODULE_NAME  "bcm_xscript"

/**
 *
 *  ioctl commands
 *
 **/
enum
{
	XSCRIPT_KERNEL_CMD_CLK_GET			= 200	///< Get clock. Data: XS_KERNEL_ClkGet_t
	,XSCRIPT_KERNEL_CMD_CLK_GET_RATE	= 201	///< Get clock rate. Data: XS_KERNEL_ClkGetRate_t
	,XSCRIPT_KERNEL_CMD_CLK_SET_RATE	= 202	///< Set clock rate. Data: XS_KERNEL_ClkSetRate_t
};

/**
 *  for ioctl cmd XSCRIPT_KERNEL_CMD_CLK_GET, a variable of this type
 *	is passed as the 'arg' to ioctl()
 */
typedef struct
{
	struct device * fDev;		///< device for clock "consumer"
	XS_BcmClk_t 	fId;		///< clock comsumer ID
	struct clk *	fOutClk;	///< (output) clock source

} XS_KERNEL_ClkGet_t;

/**
 *  for ioctl cmd XSCRIPT_KERNEL_CMD_CLK_GET_RATE, a variable of this type
 *	is passed as the 'arg' to ioctl()
 */
typedef struct
{
	struct clk *	fClk;		///< clock source
	unsigned long	fOutRate;	///< (output) clock rate in Hz


} XS_KERNEL_ClkGetRate_t;

/**
 *  for ioctl cmd XSCRIPT_KERNEL_CMD_CLK_SET_RATE, a variable of this type
 *	is passed as the 'arg' to ioctl()
 */
typedef struct
{
	struct clk * fClk;		///< clock source
	unsigned long fRate;	///< desired clock rate in Hz
	unsigned long fOutErrno;///< (output) 0 on success, or errno if error occurred.

} XS_KERNEL_ClkSetRate_t;




#ifdef __cplusplus
}
#endif // __cplusplus


#endif // XSCRIPT_KERNEL_H__ 
