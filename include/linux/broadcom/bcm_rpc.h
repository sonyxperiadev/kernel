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
*  @file   bcm_rpc.h
*
*  @brief  Interface to the kernel rpc driver.
*
*
****************************************************************************/


#ifndef __BCM_RPC_H
#define __BCM_RPC_H

/***************************************************************************/
/**
 *  Called by Linux power management system when AP enters and exits sleep.
 *  Api will notify CP of current AP sleep state, and CP will suspend 
 *  unnecessary CP->AP notifications to avoid waking AP.
 *   
 *  @param  inSleep (in)   TRUE if AP entering deep sleep, FALSE if exiting
 */
void BcmRpc_SetApSleep( bool inSleep ); 

#endif // __BCM_RPC_H 
