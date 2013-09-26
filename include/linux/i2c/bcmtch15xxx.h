/*
 * Copyright 2013 Broadcom Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as
 * published by the Free Software Foundation (the "GPL").
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * A copy of the GPL is available at
 * http://www.broadcom.com/licenses/GPLv2.php, or by writing to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#ifndef BCMTCH15XXX_H
#define BCMTCH15XXX_H

#define BCMTCH15XXX_NAME		"bcmtch15xxx"

#define BCMTCH_VENDOR_ID		0x0A5C

#define BCMTCH_CHIPID_PID_MASK	0x00FFFF00
#define BCMTCH_CHIPID_PID_SHIFT	8



/* --------------------------------- */
/* - Touch Controller Chip Version - */
/* --------------------------------- */

/* chip constants */
#define	BCMTCH_HW_BCM15200A0		0x015200a0
#define	BCMTCH_HW_BCM15500A0		0x015500a0
#define	BCMTCH_HW_BCM15500A1		0x015500a1


#define BCMTCH_AXIS_FLAG_X_BIT_POS			0
#define BCMTCH_AXIS_FLAG_Y_BIT_POS			1
#define BCMTCH_AXIS_FLAG_X_Y_BIT_POS		2

#define BCMTCH_AXIS_FLAG_X_REVERSED_MASK	0x01
#define BCMTCH_AXIS_FLAG_Y_REVERSED_MASK	0x02
#define BCMTCH_AXIS_FLAG_X_Y_SWAPPED_MASK	0x04

#define BCMTCH_MAX_BUTTON_COUNT				16


typedef int (*bcmtch_power_on_t)(bool);

/* structure for BCM Touch Controller platform hardware configuration */
struct bcmtch_platform_data {
	int i2c_bus_id;
	int i2c_addr_sys;
	int i2c_addr_spm;
	int gpio_reset_pin;
	int gpio_interrupt_pin;
	int gpio_interrupt_trigger;
	int gpio_reset_polarity;
	int gpio_reset_time_ms;
	int axis_orientation_flag;
	int ext_button_count;
	const int *ext_button_map;
	int touch_irq;
	bcmtch_power_on_t bcmtch_on;
};

#endif
