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

#ifndef _AL3006_I2C_SETTINGS_H_
#define _AL3006_I2C_SETTINGS_H_

#define AL3006_I2C_BUS_ID       2
#define AL3006_I2C_ADDRESS	0x1d
#define AL3006_IRQ_GPIO	31 /* SAT1 */

/* Disable interrupt even though it is connected.
 * HAL doesn't expect interrupt. Otherwise Android will hang */
#undef AL3006_IRQ

#endif /* _AL3006_I2C_SETTINGS_H_ */
