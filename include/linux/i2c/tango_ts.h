/*****************************************************************************
* Copyright 2009-2010 Broadcom Corporation.  All rights reserved.
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

#ifndef _TANGO_TS_H_
#define _TANGO_TS_H_
#include <linux/i2c-kona.h>

#define I2C_TS_DRIVER_NAME			"tango_ts"
#define TANGO_S32_SLAVE_ADDR		0x5C
#define TANGO_M29_SLAVE_ADDR		0x45
#define TANGO_S32_LAYOUT			(X_RIGHT_Y_UP)
#define TANGO_M29_LAYOUT			(X_RIGHT_Y_DOWN)

/*  y
 *  ^             +---->x
 *  |      or     |
 *  |             |
 *  +--->x        v
 *                y
 * X_RIGHT_Y_UP  X_RIGHT_Y_DOWN
 */
typedef enum {
	X_RIGHT_Y_UP,
	X_RIGHT_Y_DOWN,
	X_LEFT_Y_UP,
	X_LEFT_Y_DOWN,
} SCREEN_XY_LAYOUT_e;

struct TANGO_I2C_TS_t {
	struct i2c_slave_platform_data i2c_pdata;
	int i2c_slave_address;
	int gpio_irq_pin;
	int gpio_reset_pin;
	int x_max_value;
	int y_max_value;
	SCREEN_XY_LAYOUT_e layout;
	int num_bytes_to_read;
	int is_multi_touch;
	int is_resetable;
	/* The location of bytes in the stream read from the slave. */
	int num_fingers_idx;
	int old_touching_idx;
	int x1_lo_idx;
	int x1_hi_idx;
	int y1_lo_idx;
	int y1_hi_idx;
	/* Multi touch when supported. */
	int x2_lo_idx;
	int x2_hi_idx;
	int y2_lo_idx;
	int y2_hi_idx;
	int x1_width_idx;
	int y1_width_idx;
	int x2_width_idx;
	int y2_width_idx;
	int power_mode_idx;
	int int_mode_idx;
	int int_width_idx;
	int min_finger_val;
	int max_finger_val;
	int panel_width;	/* LCD panel width in millimeters */
};

#endif /* _TANGO_TS_H_ */
