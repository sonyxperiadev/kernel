/*****************************************************************************
* Copyright 2004 - 2011 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/


/****************************************************************************/
/**
*   @file   memc_qos.h
*
*   @brief  API definitions for the memory controller QOS interface
*/
/****************************************************************************/

#if !defined( ASM_ARM_ARCH_BCMHANA_MEMC_QOS_H )
#define ASM_ARM_ARCH_BCMHANA_MEMC_QOS_H

/* ---- Function Prototypes ---------------------------------------------- */

#if defined( __KERNEL__ )

extern int memc_qos_init( void );

#endif  /* defined( __KERNEL__ ) */

#endif  /* ASM_ARM_ARCH_BCMHANA_MEMC_QOS_H */
