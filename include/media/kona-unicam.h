/*****************************************************************************
* Copyright 2006 - 2009 Broadcom Corporation.  All rights reserved.
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

#ifndef ARCH_ARM_PLAT_KONA_UNICAM_H
#define ARCH_ARM_PLAT_KONA_UNICAM_H

#include <linux/i2c.h>

enum unicam_interface_type {
	UNICAM_INTERFACE_CSI2_PHY1,
};

/*
 * single or dual lane sensor
 */
enum csi2_sensor_lanes {
	CSI2_SINGLE_LANE_SENSOR,
	CSI2_DUAL_LANE_SENSOR,
};

/*
 * virtual port on a phy
 */
enum unicam_afe_port {
	UNICAM_PORT_AFE_0,
	UNICAM_PORT_AFE_1,
};

/*
 * TODO: find out do we ever need hs_term_time in sensor?
 * for now leave it
 */
struct unicam_csi2_platform_data {
	enum csi2_sensor_lanes lanes;
	enum unicam_afe_port port;
};

struct unicam_subdev_i2c_board_info {
	struct i2c_board_info board_info;
	int i2c_adapter_id;
};

struct unicam_v4l2_subdevs_groups {
	struct unicam_subdev_i2c_board_info *i2c_info;
	enum unicam_interface_type interface;
	union {
		struct unicam_csi2_platform_data csi2;
	} bus;
};

struct unicam_platform_data {
	struct unicam_v4l2_subdevs_groups *subdevs;
	unsigned int num_subdevs;
};
#endif
