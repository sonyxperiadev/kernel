/*****************************************************************************
* Copyright 2011 Broadcom Corporation.  All rights reserved.
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

#ifndef AMI306_SETTINGS_H
#define AMI306_SETTINGS_H

/* I2C */
#define AMI306_I2C_BUS_ID  2

#define AMI_I2C_ADDRESS                 0x0E    /* 7-bit Addressing */

/* GPIO */
#define AMI_GPIO_INT                    0   /* Ball MPHI_RUN1 */
#define AMI_GPIO_DRDY                   0

/* sensor position */
/* sensor axis in relation to device axis */
/* bit 7-6 : unused
 * bit 5-4 : device x-axis
 * bit 3-2 : device y-axis
 * bit 1-0 : device z-axis
 * sensor-axis: x-axis: 00b, y-axis:01b z-azis:10b
 */
#define AMI_DIR                         6


/* sensor polarity in relation to device polarity */
/* bit 7-3 : unused
 * bit 2   : x-axis polarity
 * bit 1   : y-axis polarity
 * bit 0   : z-axis polarity
 * where 0 = reverse, 1 = forward
 *
 * y
 * ^  AMI306 chip
 * |----------+
 * |         o|
 * | 3  6  0  |
 * | 1  0  1  |
 * |          |
 * z------------>x
 */
#define AMI_POLARITY                    2

#define AMI306_DATA \
{ \
	.gpio_intr = AMI_GPIO_INT, \
	.gpio_drdy = AMI_GPIO_DRDY, \
	.dir = AMI_DIR, \
	.polarity = AMI_POLARITY, \
}
#endif
