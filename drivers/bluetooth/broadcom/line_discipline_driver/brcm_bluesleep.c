/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.


 *  Copyright (C) 2009-2014 Broadcom Corporation
 */


/******************************************************************************
*
*  Filename:      brcm_bluesleep.c
*
*  Description:   Interface Module for sleep control.Sleep control is board dependant.
*                 Based on the config entry different sleep controls are selected for
*                 different platforms
*
*******************************************************************************/
#include "../include/brcm_ldisc_sh.h"

#ifdef LPM_BLUESLEEP
    extern void bluesleep_outgoing_data(void);
    extern int bluesleep_start(void);
    extern void bluesleep_stop(void);
#endif

/**
 * Handles proper timer action when outgoing data is delivered to the
 * HCI line discipline and wake the chip from sleep.
 **/
void brcm_btsleep_wake( enum sleep_type type)
{
#ifdef LPM_BLUESLEEP
    if(type == SLEEP_BLUESLEEP)
         bluesleep_outgoing_data();
#endif
}

/**
 * Starts the Sleep-Mode Protocol on the Host.
**/
void brcm_btsleep_start(enum sleep_type type)
{
#ifdef LPM_BLUESLEEP
    if(type == SLEEP_BLUESLEEP)
        bluesleep_start();
#endif
}
/**
 * Stops the Sleep-Mode Protocol on the Host.
 */
void brcm_btsleep_stop(enum sleep_type type)
{
#ifdef LPM_BLUESLEEP
    if(type == SLEEP_BLUESLEEP)
        bluesleep_stop();
#endif
}












