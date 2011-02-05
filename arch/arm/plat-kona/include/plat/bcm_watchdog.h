/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	arch/arm/plat-bcmap/include/plat/bcm_watchdog.h
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
 *   @file   bcm_watchdog.h
 *   @brief  Broadcom platform header for watchdog.
 *
	@defgroup WatchdogTimerAPIGroup Watchdog Timer API's
	@brief Broadcom Watchdog Timer API's

Here are the API's that implement the watchdog timer via direct function call.

****************************************************************************/

#ifndef __BCM_WATCHDOG__
#define __BCM_WATCHDOG__

struct bcm_watchdog {
	unsigned int *memaddr;
};

/** @addtogroup WatchdogTimerAPIGroup
	@{
*/

/**
 * The first free watchdog is allocated.
 * Both user and kernel can therefore get any watchdog
 * depending on the order of access

 @param inten Interrupt Enable.  0 = no interrupt, 1 = generate interrupt.
 @return returns zero if no error, otherwise error
 */
extern int wdt_kopen(int inten);

/**
	turn Watchdog Timer off
	@return returns zero if no error, otherwise error
*/
extern int wdt_krelease(void);

/**
	reloads (kicks) the watchdog timer
	@param len must be greater than zero to cause the timer to reload
	@return returns zero if no error, otherwise error
*/
extern int wdt_kwrite(size_t len);

/**
	Sets the watchdog timer value
	@param secs timer value
	@return returns zero if no error
*/
extern int wdt_ksettimeout(int secs);

/** 
	Starts the watchdog timer
	@return returns zero if no error, otherwise error
*/
extern int wdt_kstart(void);

/** @} */

#endif /*__BCM_WATCHDOG__*/
