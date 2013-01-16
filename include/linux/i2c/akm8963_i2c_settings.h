/*****************************************************************************
* Copyright 2012 Broadcom Corporation.  All rights reserved.
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
#ifndef _AKM8963_I2C_SETTINGS_H_
#define _AKM8963_I2C_SETTINGS_H_
#if defined(CONFIG_MACH_HAWAII_GARNET)
#define AKM8963_I2C_BUS_ID	    (2)
#define AKM8963_I2C_ADDRESS	    (0x0C)
#define AKM8963_GPIO_RESET_PIN  (0)
#else
#define AKM8963_I2C_BUS_ID	    (0)
#define AKM8963_I2C_ADDRESS	    (0x0C)
#define AKM8963_GPIO_RESET_PIN  (143)
#endif

/* Disable interrupt as HAL doesn't expect this. Otherwise Android will hang */
#define AKM8963_IRQ

#ifdef AKM8963_IRQ
#if defined(CONFIG_MACH_HAWAII_GARNET)
#define AKM8963_IRQ_GPIO	(4)
#else
#define AKM8963_IRQ_GPIO	(135)	/* MPHI_RUN1 */
#endif
#else
#undef AKM8963_IRQ_GPIO
#endif

#endif /* _AKM8963_I2C_SETTINGS_H_ */
