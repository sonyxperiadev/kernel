/*******************************************************************************
* Copyright 2011 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license
* * other than the GPL, without Broadcom's express prior written consent.
* *****************************************************************************/

#include "i2c-kona.h"
#define BCMNFC_MAGIC	0xFA

/*
 * BCMNFC power control via ioctl
 * BCMNFC_POWER_CTL(0): power off
 * BCMNFC_POWER_CTL(1): power on
 * BCMNFC_WAKE_CTL(0): wake off
 * BCMNFC_WAKE_CTL(1): wake on
 */
#define BCMNFC_POWER_CTL	_IO(BCMNFC_MAGIC, 0x01)
#define BCMNFC_CHANGE_ADDR  _IO(BCMNFC_MAGIC, 0x02)
#define BCMNFC_READ_FULL_PACKET       _IO(BCMNFC_MAGIC, 0x03)
#define BCMNFC_SET_WAKE_ACTIVE_STATE  _IO(BCMNFC_MAGIC, 0x04)
#define BCMNFC_WAKE_CTL               _IO(BCMNFC_MAGIC, 0x05)
#define BCMNFC_READ_MULTI_PACKETS     _IO(BCMNFC_MAGIC, 0x06)

struct bcmi2cnfc_i2c_platform_data {
	struct i2c_slave_platform_data i2c_pdata;
	unsigned int irq_gpio;
	unsigned int en_gpio;
	unsigned int wake_gpio;
	int (*init)(void *);
	int (*reset)(void *);
};
