/*****************************************************************************
*  Copyright 2011 Broadcom Corporation.  All rights reserved.
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2, available at
*  http://www.gnu.org/licenses/old-license/gpl-2.0.html (the "GPL").
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
*****************************************************************************/
#ifndef _I2C_KONA_H_
#define	_I2C_KONA_H_

enum bsc_bus_speed {
	BSC_BUS_SPEED_32K,
	BSC_BUS_SPEED_50K,
	BSC_BUS_SPEED_100K,
	BSC_BUS_SPEED_230K,
	BSC_BUS_SPEED_380K,
	BSC_BUS_SPEED_400K,
	BSC_BUS_SPEED_430K,
	BSC_BUS_SPEED_HS,	/* high-speed */
	BSC_BUS_SPEED_HS_1MHZ,
	BSC_BUS_SPEED_HS_2MHZ,
	BSC_BUS_SPEED_HS_1625KHZ,
	BSC_BUS_SPEED_HS_2600KHZ,

	/* for FPGA where only the 26 MHz core clock is available */
	BSC_BUS_SPEED_100K_FPGA,
	BSC_BUS_SPEED_400K_FPGA,
	BSC_BUS_SPEED_HS_FPGA,

	/* used as internal array index so do not reorder this */
	BSC_BUS_SPEED_MAX,
};

/*
 * Board dependent configuration for the Broadcom Kona BSC (I2C) adapter
 */
struct bsc_adap_cfg {
	/* to explicitly disable the I2C adapter */
	int disable;

	/* I2C bus speed */
	enum bsc_bus_speed speed;

	/*
	 * flag to turn on dynamic speed support. If this flag is turned on
	 * in the bus driver, all slave drivers using the bus driver need to
	 * set its i2c_speed in its platform_data data structure
	 */
	int dynamic_speed;

	/*
	 * flag to inform whether the controller is for talking to PMU
	 * or not. This is needed to acquire the HW semaphore to sync 
	 * with the Power Manager sequencer.
	 */
	bool is_pmu_i2c;

	/* BSC clocks */
	char *bsc_clk;
	char *bsc_apb_clk;
	int retries;
};

/*
 * I2C slave platform data, for I2C slaves to specify the bus speed.
 *
 *  NOTE: This struct should be included as first member of client
 *  specific platform data. Otherwise, default speed of the adapter
 *  will be used for that slave device.
 */
struct i2c_slave_platform_data {
	/* Magic number to validate dynamic speed */
	unsigned long spd_magic;
	enum bsc_bus_speed i2c_speed;
	/* Magic number to validate the timeout enable */
	unsigned long timeout_magic;
	int autosense_timeout_enable;
};

/* Magic number = "spd" in ascii codes */
#define SLAVE_SPD_MAGIC_NUM           0x00647073
/* Magic number = "tout" in ascii codes */
#define TIMEOUT_MAGIC_NUM             0x746F7574

#define ADD_I2C_SLAVE_SPEED(s)        .spd_magic = SLAVE_SPD_MAGIC_NUM,       \
                                      .i2c_speed = s

#define ENABLE_AUTOSENSE_TIMEOUT(s)   .timeout_magic = TIMEOUT_MAGIC_NUM,       \
                                      .autosense_timeout_enable = s

#define I2C_SPEED_IS_VALID(x)  (x->spd_magic == SLAVE_SPD_MAGIC_NUM)

#define TIMEOUT_IS_VALID(x)  (x->timeout_magic == TIMEOUT_MAGIC_NUM)

#endif // _I2C_KONA_H_
