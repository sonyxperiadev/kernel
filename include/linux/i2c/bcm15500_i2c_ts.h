/****************************************************************************
*
*	Copyright (c) 1999-2008 Broadcom Corporation
*
*   Unless you and Broadcom execute a separate written software license
*   agreement governing use of this software, this software is licensed to you
*   under the terms of the GNU General Public License version 2, available
*   at http://www.gnu.org/licenses/old-licenses/gpl-2.0.html (the "GPL").
*
*   Notwithstanding the above, under no circumstances may you combine this
*   software in any way with any other Broadcom software provided under a
*   license other than the GPL, without Broadcom's express prior written
*   consent.
*
****************************************************************************/

#ifndef BCM915500_I2C_TS_H
#define BCM915500_I2C_TS_H

#define BCM915500_TSC_NAME "bcm915500"

/* hardware / platform specific - MUST be set according to hardware platform */

#define HW_BCM915500_SLAVE_SPM        0x66	/* SPM */
#define HW_BCM915500_SLAVE_AHB        0x55	/* AHB */

#define HW_BCM915500_AXIS_SWAP_X_Y  0
#define HW_BCM915500_AXIS_SWAP_Y    0
#define HW_BCM915500_AXIS_SWAP_X    0

/* hardware / platform specific - end */

struct bcm915500_platform_data {
	int id;
	int i2c_adapter_id;
	int gpio_reset;
	int gpio_interrupt;
	struct i2c_board_info *p_i2c_board_info_map1;
};

typedef enum {
	BCMTCH_COM_I2C_INTERFACE,
	BCMTCH_COM_SPI_INTERFACE,

} bcmtch_com_e;

typedef struct {

	bcmtch_com_e com_interface;

} bcmtch_data_t;

#endif
