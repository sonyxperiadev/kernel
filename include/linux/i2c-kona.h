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

/* Adapter specific reference clock enum */
enum bsc_ref_clk {
	BSC_BUS_REF_13MHZ = 13000000,
	BSC_BUS_REF_26MHZ = 26000000,
	BSC_BUS_REF_104MHZ = 104000000,
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

	/* Ref clock selection for HS and FS/SS modes */
	enum bsc_ref_clk hs_ref;
	enum bsc_ref_clk fs_ref;

	/* BSC clocks */
	char *bsc_clk;
	char *bsc_apb_clk;
	int retries;
};

#define TIMEOUT_DISABLE		(0x1 << 0)
#define TX_FIFO_ENABLE		(0x1 << 1)
#define RX_FIFO_ENABLE		(0x1 << 2)
#define POLLING_ENABLE		(0x1 << 3)
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
	/* Magic number to validate the client specific misc data */
	unsigned long client_func_magic;
	/* Adding a bit map to enable/disable client supported
	 * functionalities
	 * Currently supported functionalities:
	 * client_func_map[0] : Autosense timeout disable
	 * client_func_map[1] : TX FIFO enable
	 * client_func_map[2] : RX FIFO enable */
	unsigned int client_func_map;
};

/* Magic number = "spd" in ascii codes */
#define SLAVE_SPD_MAGIC_NUM           0x00647073
/* Magic number = "flag" in ascii codes */
#define CLIENT_FUNC_MAGIC_NUM		0x666C6167

#define ADD_I2C_SLAVE_SPEED(s)        .spd_magic = SLAVE_SPD_MAGIC_NUM,       \
                                      .i2c_speed = s

#define SET_CLIENT_FUNC(s)	.client_func_magic = CLIENT_FUNC_MAGIC_NUM,   \
				.client_func_map = s

#define i2c_speed_is_valid(x)  (x->spd_magic == SLAVE_SPD_MAGIC_NUM)

#define disable_timeout(x)  (x->client_func_magic == CLIENT_FUNC_MAGIC_NUM && \
				x->client_func_map & TIMEOUT_DISABLE)

#define enable_tx_fifo(x)	(x->client_func_magic == \
				CLIENT_FUNC_MAGIC_NUM && \
				(x->client_func_map & \
				TX_FIFO_ENABLE))

#define enable_rx_fifo(x)	(x->client_func_magic == \
				CLIENT_FUNC_MAGIC_NUM && \
				(x->client_func_map & \
				RX_FIFO_ENABLE))

#define enable_polling_mode(x)	(x->client_func_magic == \
				CLIENT_FUNC_MAGIC_NUM && \
				(x->client_func_map & \
				POLLING_ENABLE))

#endif // _I2C_KONA_H_
