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

#define AKM8963_I2C_BUS_ID	    (0)
#define AKM8963_I2C_ADDRESS	    (0x0C)

#define AKM8963_GPIO_RESET_PIN  (143)

/* Disable interrupt as HAL doesn't expect this. Otherwise Android will hang */
#define AKM8963_IRQ

#ifdef AKM8963_IRQ
#define AKM8963_IRQ_GPIO	(135)	/* MPHI_RUN1 */
#else
#undef AKM8963_IRQ_GPIO
#endif

#endif /* _AKM8963_I2C_SETTINGS_H_ */
