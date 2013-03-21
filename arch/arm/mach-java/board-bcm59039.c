/*****************************************************************************
*  Copyright 2001 - 2011 Broadcom Corporation.  All rights reserved.
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

#include <linux/version.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <asm/mach/arch.h>
#include <asm/mach-types.h>
#include <linux/gpio.h>
#include <mach/hardware.h>
#include <linux/io.h>
#include <linux/uaccess.h>
#include <linux/i2c.h>
#include <linux/mfd/bcmpmu.h>
#include <linux/broadcom/bcmpmu-ponkey.h>
#ifdef CONFIG_KONA_AVS
#include <mach/avs.h>
#endif
#include "pm_params.h"

#define PMU_DEVICE_I2C_ADDR	0x08
#define PMU_DEVICE_I2C_ADDR1	0x0C
#define PMU_DEVICE_INT_GPIO	29
#define PMU_DEVICE_I2C_BUSNO 2
static int vlt_tbl_init;

static struct bcmpmu_rw_data register_init_data[] = {
	{.map = 0, .addr = 0x01, .val = 0x00, .mask = 0x01},
	{.map = 0, .addr = 0x0c, .val = 0x1b, .mask = 0xFF},
	{.map = 0, .addr = 0x13, .val = 0x43, .mask = 0xFF},
	{.map = 0, .addr = 0x14, .val = 0x7F, .mask = 0xFF},
	{.map = 0, .addr = 0x15, .val = 0x3B, .mask = 0xFF},
	{.map = 0, .addr = 0x16, .val = 0xF8, .mask = 0xFF},
	{.map = 0, .addr = 0x1D, .val = 0x09, .mask = 0xFF},
	{.map = 0, .addr = 0x40, .val = 0xFF, .mask = 0xFF},
	{.map = 0, .addr = 0x41, .val = 0xFF, .mask = 0xFF},
	{.map = 0, .addr = 0x42, .val = 0xFF, .mask = 0xFF},
	{.map = 0, .addr = 0x43, .val = 0xFF, .mask = 0xFF},
	{.map = 0, .addr = 0x44, .val = 0xFF, .mask = 0xFF},
	{.map = 0, .addr = 0x45, .val = 0xFF, .mask = 0xFF},
	{.map = 0, .addr = 0x46, .val = 0xFF, .mask = 0xFF},
	{.map = 0, .addr = 0x47, .val = 0xFF, .mask = 0xFF},
	{.map = 0, .addr = 0x52, .val = 0x04, .mask = 0x04},
	{.map = 0, .addr = 0x58, .val = 0x0F, .mask = 0x0F},
	/*
	* temp workaround for LDOs, to be revisited once final
		OTP value available
	*/
	{.map = 0, .addr = 0xB1, .val = 0x4B, .mask = 0xFF},
	{.map = 0, .addr = 0xB2, .val = 0x04, .mask = 0xFF},
	{.map = 0, .addr = 0xB3, .val = 0x25, .mask = 0xFF},
	{.map = 0, .addr = 0xB4, .val = 0x27, .mask = 0xFF},
	{.map = 0, .addr = 0xB5, .val = 0x05, .mask = 0xFF},
	{.map = 0, .addr = 0xB6, .val = 0x07, .mask = 0xFF},
	{.map = 0, .addr = 0xB7, .val = 0x25, .mask = 0xFF},
	{.map = 0, .addr = 0xB8, .val = 0x06, .mask = 0xFF},
	{.map = 0, .addr = 0xB9, .val = 0x07, .mask = 0xFF},
	{.map = 0, .addr = 0xBD, .val = 0x21, .mask = 0xFF},

	/*Init SDSR NM, NM2 and LPM voltages to 1.2V
	*/
	{.map = 0, .addr = 0xD0, .val = 0x13, .mask = 0xFF},
	{.map = 0, .addr = 0xD1, .val = 0x13, .mask = 0xFF},
	{.map = 0, .addr = 0xD2, .val = 0x13, .mask = 0xFF},

	/*Init CSR LPM  to 0.9 V
	CSR NM2 to 1.22V
	*/
	{.map = 0, .addr = 0xC1, .val = 0x04, .mask = 0xFF},
	{.map = 0, .addr = 0xC2, .val = 0x14, .mask = 0xFF},

	/*PLLCTRL, Clear Bit 0 to disable PLL when PC2:PC1 = 0b00*/
	{.map = 0, .addr = 0x0A, .val = 0x0E, .mask = 0x0F},
	/*CMPCTRL13, Set bits 4, 1 for BSI Sync. Mode */
	{.map = 0, .addr = 0x1C, .val = 0x13, .mask = 0xFF},
	/*CMPCTRL12, Set bits 4, 1 for NTC Sync. Mode*/
	{.map = 0, .addr = 0x1B, .val = 0x13, .mask = 0xFF},

	/*Init ASR LPM to 2.9V - */
	{.map = 0, .addr = 0xD9, .val = 0x1F, .mask = 0xFF},
	/*Init IOSR NM2 and LPM voltages to 1.8V
	*/
	{.map = 0, .addr = 0xC9, .val = 0x1A, .mask = 0xFF},
	{.map = 0, .addr = 0xCA, .val = 0x1A, .mask = 0xFF},

	/*FGOPMODCTRL, Set bits 4, 1 for FG Sync. Mode*/
	{.map = 1, .addr = 0x42, .val = 0x15, .mask = 0xFF},

};

static struct bcmpmu_temp_map batt_temp_map[] = {
	/*
	* This table is hardware dependent and need to get from platform team
	*/
	/*
	* adc temp
	*/
	{932, 233},			/* -40 C */
	{900, 238},			/* -35 C */
	{860, 243},			/* -30 C */
	{816, 248},			/* -25 C */
	{760, 253},			/* -20 C */
	{704, 258},			/* -15 C */
	{636, 263},			/* -10 C */
	{568, 268},			/* -5 C */
	{500, 273},			/* 0 C */
	{440, 278},			/* 5 C */
	{376, 283},			/* 10 C */
	{324, 288},			/* 15 C */
	{272, 293},			/* 20 C */
	{228, 298},			/* 25 C */
	{192, 303},			/* 30 C */
	{160, 308},			/* 35 C */
	{132, 313},			/* 40 C */
	{112, 318},			/* 45 C */
	{92, 323},			/* 50 C */
	{76, 328},			/* 55 C */
	{64, 333},			/* 60 C */
	{52, 338},			/* 65 C */
	{44, 343},			/* 70 C */
	{36, 348},			/* 75 C */
	{32, 353},			/* 80 C */
	{28, 358},			/* 85 C */
	{24, 363},			/* 90 C */
	{20, 368},			/* 95 C */
	{16, 373},			/* 100 C */
};

__weak struct regulator_consumer_supply rf_supply[] = {
	{.supply = "rf"},
};
static struct regulator_init_data bcm59039_rfldo_data = {
	.constraints = {
			.name = "rfldo",
			.min_uV = 1300000,
			.max_uV = 3300000,
			.valid_ops_mask = REGULATOR_CHANGE_STATUS |
			REGULATOR_CHANGE_VOLTAGE,
			.always_on = 1,
			},
	.num_consumer_supplies = ARRAY_SIZE(rf_supply),
	.consumer_supplies = rf_supply,
};

__weak struct regulator_consumer_supply cam_supply[] = {
	{.supply = "cam"},
};
static struct regulator_init_data bcm59039_camldo_data = {
	.constraints = {
			.name = "camldo",
			.min_uV = 1300000,
			.max_uV = 3300000,
			.valid_ops_mask =
			REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_MODE |
			REGULATOR_CHANGE_VOLTAGE,
			.always_on = 1,
			},
	.num_consumer_supplies = ARRAY_SIZE(cam_supply),
	.consumer_supplies = cam_supply,
};


__weak struct regulator_consumer_supply hv1_supply[] = {
	{.supply = "hv1"},
};
static struct regulator_init_data bcm59039_hv1ldo_data = {
	.constraints = {
			.name = "hv1ldo",
			.min_uV = 1300000,
			.max_uV = 3300000,
			.valid_ops_mask =
			REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_MODE |
			REGULATOR_CHANGE_VOLTAGE,
			.always_on = 1,
			},
	.num_consumer_supplies = ARRAY_SIZE(hv1_supply),
	.consumer_supplies = hv1_supply,
};

__weak struct regulator_consumer_supply hv2_supply[] = {
	{.supply = "hv2"},
};
static struct regulator_init_data bcm59039_hv2ldo_data = {
	.constraints = {
			.name = "hv2ldo",
			.min_uV = 1300000,
			.max_uV = 3300000,
			.valid_ops_mask =
			REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE,
			.always_on = 1,
			},
	.num_consumer_supplies = ARRAY_SIZE(hv2_supply),
	.consumer_supplies = hv2_supply,
};

__weak struct regulator_consumer_supply hv3_supply[] = {
	{.supply = "hv3"},
	{.supply = "dummy"}, /* Overridden in board file */
};
static struct regulator_init_data bcm59039_hv3ldo_data = {
	.constraints = {
			.name = "hv3ldo",
			.min_uV = 1300000,
			.max_uV = 3300000,
			.valid_ops_mask =
			REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE,
			.always_on = 0,
			},
	.num_consumer_supplies = ARRAY_SIZE(hv3_supply),
	.consumer_supplies = hv3_supply,
};

__weak struct regulator_consumer_supply hv4_supply[] = {
	{.supply = "hv4"},
	{.supply = "2v9_vibra"},
	{.supply = "dummy"}, /* Add a dummy variable to ensure we can use an array of 3 in hawaii_ray.
		  A hack at best to ensure we redefine the supply in board file. */
};
static struct regulator_init_data bcm59039_hv4ldo_data = {
	.constraints = {
			.name = "hv4ldo",
			.min_uV = 1300000,
			.max_uV = 3300000,
			.valid_ops_mask =
			REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE,
			.always_on = 0,
		},
	.num_consumer_supplies = ARRAY_SIZE(hv4_supply),
	.consumer_supplies = hv4_supply,
};

__weak struct regulator_consumer_supply hv5_supply[] = {
	{.supply = "hv5"},
};
static struct regulator_init_data bcm59039_hv5ldo_data = {
	.constraints = {
			.name = "hv5ldo",
			.min_uV = 1300000,
			.max_uV = 3300000,
			.valid_ops_mask =
			REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE,
			.always_on = 1,
			},
	.num_consumer_supplies = ARRAY_SIZE(hv5_supply),
	.consumer_supplies = hv5_supply,
};

__weak struct regulator_consumer_supply hv6_supply[] = {
	{.supply = "vdd_sdxc"},
	{.supply = "dummy"},
};
static struct regulator_init_data bcm59039_hv6ldo_data = {
	.constraints = {
			.name = "hv6ldo",
			.min_uV = 1300000,
			.max_uV = 3300000,
			.valid_ops_mask =
			REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE,
			.always_on = 1,
			},
	.num_consumer_supplies = ARRAY_SIZE(hv6_supply),
	.consumer_supplies = hv6_supply,
};

__weak struct regulator_consumer_supply hv7_supply[] = {
	{.supply = "hv7"},
};
static struct regulator_init_data bcm59039_hv7ldo_data = {
	.constraints = {
			.name = "hv7ldo",
			.min_uV = 1300000,
			.max_uV = 3300000,
			.valid_ops_mask =
			REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE,
			.always_on = 1,
			},
	.num_consumer_supplies = ARRAY_SIZE(hv7_supply),
	.consumer_supplies = hv7_supply,
};

__weak struct regulator_consumer_supply hv8_supply[] = {
	{.supply = "hv8"},
};
static struct regulator_init_data bcm59039_hv8ldo_data = {
	.constraints = {
			.name = "hv8ldo",
			.min_uV = 1300000,
			.max_uV = 3300000,
			.valid_ops_mask =
			REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE,
			.always_on = 1,
			},
	.num_consumer_supplies = ARRAY_SIZE(hv8_supply),
	.consumer_supplies = hv8_supply,
};

__weak struct regulator_consumer_supply hv9_supply[] = {
	{.supply = "hv9"},
};
static struct regulator_init_data bcm59039_hv9ldo_data = {
	.constraints = {
			.name = "hv9ldo",
			.min_uV = 1300000,
			.max_uV = 3300000,
			.valid_ops_mask =
			REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE,
			.always_on = 1,
			},
	.num_consumer_supplies = ARRAY_SIZE(hv9_supply),
	.consumer_supplies = hv9_supply,
};

__weak struct regulator_consumer_supply hv10_supply[] = {
	{.supply = "hv10"},
};

static struct regulator_init_data bcm59039_hv10ldo_data = {
	.constraints = {
			.name = "hv10ldo",
			.min_uV = 1300000,
			.max_uV = 3300000,
			.valid_ops_mask =
			REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE,

			},
	.num_consumer_supplies = ARRAY_SIZE(hv10_supply),
	.consumer_supplies = hv10_supply,
};

__weak struct regulator_consumer_supply sim_supply[] = {
	{.supply = "sim_vcc"},
};
static struct regulator_init_data bcm59039_simldo_data = {
	.constraints = {
			.name = "simldo",
			.min_uV = 1300000,
			.max_uV = 3300000,
			.valid_ops_mask =
			REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE,
			.always_on = 0,

			},
	.num_consumer_supplies = ARRAY_SIZE(sim_supply),
	.consumer_supplies = sim_supply,
};

struct regulator_consumer_supply csr_nm_supply[] = {
	{.supply = "csr_nm_uc"},
};
static struct regulator_init_data bcm59039_csr_nm_data = {
	.constraints = {
			.name = "csr_nm",
			.min_uV = 700000,
			.max_uV = 1800000,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE,
			.always_on = 1,
			},
	.num_consumer_supplies = ARRAY_SIZE(csr_nm_supply),
	.consumer_supplies = csr_nm_supply,
};

struct regulator_consumer_supply csr_nm2_supply[] = {
	{.supply = "csr_nm2_uc"},
};
static struct regulator_init_data bcm59039_csr_nm2_data = {
	.constraints = {
			.name = "csr_nm2",
			.min_uV = 700000,
			.max_uV = 1800000,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE,
			.always_on = 1,
			},
	.num_consumer_supplies = ARRAY_SIZE(csr_nm2_supply),
	.consumer_supplies = csr_nm2_supply,
};

struct regulator_consumer_supply csr_lpm_supply[] = {
	{.supply = "csr_lpm_uc"},
};
static struct regulator_init_data bcm59039_csr_lpm_data = {
	.constraints = {
			.name = "csr_lpm",
			.min_uV = 700000,
			.max_uV = 1800000,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE,
			.always_on = 1,
			},
	.num_consumer_supplies = ARRAY_SIZE(csr_lpm_supply),
	.consumer_supplies = csr_lpm_supply,
};


struct regulator_consumer_supply iosr_nm_supply[] = {
	{.supply = "iosr_nm_uc"},
};
static struct regulator_init_data bcm59039_iosr_nm_data = {
	.constraints = {
			.name = "iosr_nm",
			.min_uV = 700000,
			.max_uV = 1800000,
			.valid_ops_mask =
			REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_VOLTAGE,
			.always_on = 1,
			},
	.num_consumer_supplies = ARRAY_SIZE(iosr_nm_supply),
	.consumer_supplies = iosr_nm_supply,
};

struct regulator_consumer_supply iosr_nm2_supply[] = {
	{.supply = "iosr_nm2_uc"},
};
static struct regulator_init_data bcm59039_iosr_nm2_data = {
	.constraints = {
			.name = "iosr_nm2",
			.min_uV = 700000,
			.max_uV = 1800000,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE,
			.always_on = 1,
			},
	.num_consumer_supplies = ARRAY_SIZE(iosr_nm2_supply),
	.consumer_supplies = iosr_nm2_supply,
};
struct regulator_consumer_supply iosr_lpm_supply[] = {
	{.supply = "iosr_lmp_uc"},
};
static struct regulator_init_data bcm59039_iosr_lpm_data = {
	.constraints = {
			.name = "iosr_lmp",
			.min_uV = 700000,
			.max_uV = 1800000,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE,
			.always_on = 1,
			},
	.num_consumer_supplies = ARRAY_SIZE(iosr_lpm_supply),
	.consumer_supplies = iosr_lpm_supply,
};

struct regulator_consumer_supply sdsr_nm_supply[] = {
	{.supply = "sdsr_nm_uc"},
};

static struct regulator_init_data bcm59039_sdsr_nm_data = {
	.constraints = {
			.name = "sdsr_nm",
			.min_uV = 700000,
			.max_uV = 1800000,
			.valid_ops_mask =
			REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_VOLTAGE,
			.always_on = 1,
			},
	.num_consumer_supplies = ARRAY_SIZE(sdsr_nm_supply),
	.consumer_supplies = sdsr_nm_supply,
};

struct regulator_consumer_supply sdsr_nm2_supply[] = {
	{.supply = "sdsr_nm2_uc"},
};

static struct regulator_init_data bcm59039_sdsr_nm2_data = {
	.constraints = {
			.name = "sdsr_nm2",
			.min_uV = 700000,
			.max_uV = 1800000,
			.valid_ops_mask =
			REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_VOLTAGE,
			.always_on = 1,
			},
	.num_consumer_supplies = ARRAY_SIZE(sdsr_nm2_supply),
	.consumer_supplies = sdsr_nm2_supply,
};

struct regulator_consumer_supply sdsr_lpm_supply[] = {
	{.supply = "sdsr_lpm_uc"},
};

static struct regulator_init_data bcm59039_sdsr_lpm_data = {
	.constraints = {
			.name = "sdsr_lpm",
			.min_uV = 700000,
			.max_uV = 1800000,
			.valid_ops_mask =
			REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_VOLTAGE,
			.always_on = 1,
			},
	.num_consumer_supplies = ARRAY_SIZE(sdsr_lpm_supply),
	.consumer_supplies = sdsr_lpm_supply,
};

struct regulator_consumer_supply asr_nm_supply[] = {
	{.supply = "asr_nm_uc"},
};

static struct regulator_init_data bcm59039_asr_nm_data = {
	.constraints = {
			.name = "asr_nm",
			.min_uV = 700000,
			.max_uV = 2900000,
			.valid_ops_mask =
			REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_VOLTAGE,
			.always_on = 1,
			},
	.num_consumer_supplies = ARRAY_SIZE(asr_nm_supply),
	.consumer_supplies = asr_nm_supply,
};

struct regulator_consumer_supply asr_nm2_supply[] = {
	{.supply = "asr_nm2_uc"},
};

static struct regulator_init_data bcm59039_asr_nm2_data = {
	.constraints = {
			.name = "asr_nm2",
			.min_uV = 700000,
			.max_uV = 2900000,
			.valid_ops_mask =
			REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_VOLTAGE,
			.always_on = 1,
			},
	.num_consumer_supplies = ARRAY_SIZE(asr_nm2_supply),
	.consumer_supplies = asr_nm2_supply,
};

struct regulator_consumer_supply asr_lpm_supply[] = {
	{.supply = "asr_lpm_uc"},
};

static struct regulator_init_data bcm59039_asr_lpm_data = {
	.constraints = {
			.name = "asr_lpm",
			.min_uV = 700000,
			.max_uV = 2900000,
			.valid_ops_mask =
			REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_VOLTAGE,
			.always_on = 1,
			},
	.num_consumer_supplies = ARRAY_SIZE(asr_lpm_supply),
	.consumer_supplies = asr_lpm_supply,
};

struct bcmpmu_regulator_init_data bcm59039_regulators[BCMPMU_REGULATOR_MAX] = {
	[BCMPMU_REGULATOR_RFLDO] = {
		BCMPMU_REGULATOR_RFLDO, &bcm59039_rfldo_data, 0x01, 0
	},
	[BCMPMU_REGULATOR_CAMLDO] = {
		BCMPMU_REGULATOR_CAMLDO, &bcm59039_camldo_data, 0x11, 0
	},
	[BCMPMU_REGULATOR_HV1LDO] =	{
		BCMPMU_REGULATOR_HV1LDO, &bcm59039_hv1ldo_data, 0x11, 0
	},
	[BCMPMU_REGULATOR_HV2LDO] =	{
		BCMPMU_REGULATOR_HV2LDO, &bcm59039_hv2ldo_data, 0x11, 0
	},
	[BCMPMU_REGULATOR_HV3LDO] = {
		BCMPMU_REGULATOR_HV3LDO, &bcm59039_hv3ldo_data, 0x11, 0
	},
	[BCMPMU_REGULATOR_HV4LDO] =	{
		BCMPMU_REGULATOR_HV4LDO, &bcm59039_hv4ldo_data, 0xAA, 0
	},
	[BCMPMU_REGULATOR_HV5LDO] = {
		BCMPMU_REGULATOR_HV5LDO, &bcm59039_hv5ldo_data, 0x11, 0
	},
	[BCMPMU_REGULATOR_HV6LDO] = {
		BCMPMU_REGULATOR_HV6LDO, &bcm59039_hv6ldo_data, 0x11, 0
	},
	[BCMPMU_REGULATOR_HV7LDO] = {
		BCMPMU_REGULATOR_HV7LDO, &bcm59039_hv7ldo_data, 0x22, 0
	},
	[BCMPMU_REGULATOR_HV8LDO] = {
		BCMPMU_REGULATOR_HV8LDO, &bcm59039_hv8ldo_data, 0x11, 0
	},
	[BCMPMU_REGULATOR_HV9LDO] = {
		BCMPMU_REGULATOR_HV9LDO, &bcm59039_hv9ldo_data, 0x11, 0
	},
	[BCMPMU_REGULATOR_HV10LDO] = {
		BCMPMU_REGULATOR_HV10LDO, &bcm59039_hv10ldo_data, 0xAA, 0
	},

	[BCMPMU_REGULATOR_SIMLDO] = {
		BCMPMU_REGULATOR_SIMLDO, &bcm59039_simldo_data, 0xAA,
			BCMPMU_REGL_LPM_IN_DSM
	},
	[BCMPMU_REGULATOR_CSR_NM] =	{
		BCMPMU_REGULATOR_CSR_NM, &bcm59039_csr_nm_data, 0x11, 0
	},
	[BCMPMU_REGULATOR_CSR_NM2] = {
		BCMPMU_REGULATOR_CSR_NM2, &bcm59039_csr_nm2_data, 0xFF, 0
	},
	[BCMPMU_REGULATOR_CSR_LPM] = {
		BCMPMU_REGULATOR_CSR_LPM, &bcm59039_csr_lpm_data, 0xFF, 0
	},
	[BCMPMU_REGULATOR_IOSR_NM] = {
		BCMPMU_REGULATOR_IOSR_NM, &bcm59039_iosr_nm_data, 0x01, 0
	},
	[BCMPMU_REGULATOR_IOSR_NM2] = {
		BCMPMU_REGULATOR_IOSR_NM2, &bcm59039_iosr_nm2_data, 0xFF, 0
	},
	[BCMPMU_REGULATOR_IOSR_LPM] = {
		BCMPMU_REGULATOR_IOSR_LPM, &bcm59039_iosr_lpm_data, 0xFF, 0
	},
	[BCMPMU_REGULATOR_SDSR_NM] = {
		BCMPMU_REGULATOR_SDSR_NM, &bcm59039_sdsr_nm_data, 0x11, 0
	},
	[BCMPMU_REGULATOR_SDSR_NM2] = {
		BCMPMU_REGULATOR_SDSR_NM2, &bcm59039_sdsr_nm2_data, 0xFF, 0
	},
	[BCMPMU_REGULATOR_SDSR_LPM] = {
		BCMPMU_REGULATOR_SDSR_LPM, &bcm59039_sdsr_lpm_data, 0xFF, 0
	},
	[BCMPMU_REGULATOR_ASR_NM] = {
		BCMPMU_REGULATOR_ASR_NM, &bcm59039_asr_nm_data, 0x11, 0
	},

	[BCMPMU_REGULATOR_ASR_NM2] = {
		BCMPMU_REGULATOR_ASR_NM2, &bcm59039_asr_nm2_data, 0xFF, 0
	},
	[BCMPMU_REGULATOR_ASR_LPM] = {
		BCMPMU_REGULATOR_ASR_LPM, &bcm59039_asr_lpm_data, 0xFF, 0
	},
};


static struct bcmpmu_wd_setting bcm59039_wd_setting = {
	.watchdog_timeout = 127,
};


static struct platform_device bcmpmu_audio_device = {
	.name = "bcmpmu_audio",
	.id = -1,
	.dev.platform_data = NULL,
};

static struct platform_device bcmpmu_em_device = {
	.name = "bcmpmu_em",
	.id = -1,
	.dev.platform_data = NULL,
};

static struct platform_device bcmpmu_otg_xceiv_device = {
	.name = "bcmpmu_otg_xceiv",
	.id = -1,
	.dev.platform_data = NULL,
};

#ifdef CONFIG_BCMPMU_RPC
static struct platform_device bcmpmu_rpc = {
	.name = "bcmpmu_rpc",
	.id = -1,
	.dev.platform_data = NULL,
};
#endif

static struct platform_device *bcmpmu_client_devices[] = {
	&bcmpmu_audio_device,
	&bcmpmu_em_device,
	&bcmpmu_otg_xceiv_device,
#ifdef CONFIG_BCMPMU_RPC
	&bcmpmu_rpc,
#endif
};



static int bcmpmu_exit_platform_hw(struct bcmpmu *bcmpmu)
{
	pr_info("REG: pmu_init_platform_hw called\n");
	return 0;
}

static struct i2c_board_info pmu_info_map1 = {
	I2C_BOARD_INFO("bcmpmu_map1", PMU_DEVICE_I2C_ADDR1),
};

static struct bcmpmu_adc_setting adc_setting = {
	.tx_rx_sel_addr = 0,
	.tx_delay = 0,
	.rx_delay = 0,
};

static struct bcmpmu_charge_zone chrg_zone[] = {
	{.tl = 268, .th = 333, .v = 3000, .fc = 10, .qc = 100},	/* Zone QC */
	{.tl = 268, .th = 272, .v = 4200, .fc = 100, .qc = 0},	/* Zone LL */
	{.tl = 273, .th = 282, .v = 4200, .fc = 100, .qc = 0},	/* Zone L */
	{.tl = 283, .th = 318, .v = 4200, .fc = 100, .qc = 0},	/* Zone N */
	{.tl = 319, .th = 323, .v = 4200, .fc = 100, .qc = 0},	/* Zone H */
	{.tl = 324, .th = 333, .v = 4200, .fc = 100, .qc = 0},	/* Zone HH */
	{.tl = 268, .th = 333, .v = 0, .fc = 0, .qc = 0},	/* Zone OUT */
};

static struct bcmpmu_voltcap_map batt_voltcap_map[] = {
	/*
	* Battery data for 1300mAH
	*/
	/*
	* volt capacity
	*/
	{4160, 100},
	{4122, 95},
	{4072, 90},
	{4031, 85},
	{3978, 80},
	{3937, 75},
	{3903, 70},
	{3866, 65},
	{3834, 60},
	{3808, 55},
	{3787, 50},
	{3769, 45},
	{3753, 40},
	{3741, 35},
	{3730, 30},
	{3710, 25},
	{3670, 20},
	{3648, 15},
	{3590, 10},
	{3542, 8},
	{3480, 6},
	{3400, 4},
	{3310, 2},
	{3200, 0},
};

static int bcmpmu_init_platform_hw(struct bcmpmu *);

static struct bcmpmu_fg_zone fg_zone[FG_TMP_ZONE_MAX+1] = {
/* This table is default data, the real data from board file or device tree*/
	{.temp = 253,
	 .reset = 0, .fct = 520, .guardband = 100,
	 .esr_vl_lvl = 3600, .esr_vm_lvl = 3800, .esr_vh_lvl = 4000,
	 .esr_vl = 140, .esr_vl_slope = 100, .esr_vl_offset = 0,
	 .esr_vm = 140, .esr_vm_slope = 100, .esr_vm_offset = 0,
	 .esr_vh = 140, .esr_vh_slope = 100, .esr_vh_offset = 0,
	 .esr_vf = 140, .esr_vf_slope = 100, .esr_vf_offset = 0,
	 .vcmap = &batt_voltcap_map[0],
	 .maplen = ARRAY_SIZE(batt_voltcap_map)},/* -20 */
	{.temp = 258,
	 .reset = 0, .fct = 620, .guardband = 100,
	 .esr_vl_lvl = 3600, .esr_vm_lvl = 3800, .esr_vh_lvl = 4000,
	 .esr_vl = 140, .esr_vl_slope = 100, .esr_vl_offset = 0,
	 .esr_vm = 140, .esr_vm_slope = 100, .esr_vm_offset = 0,
	 .esr_vh = 140, .esr_vh_slope = 100, .esr_vh_offset = 0,
	 .esr_vf = 140, .esr_vf_slope = 100, .esr_vf_offset = 0,
	 .vcmap = &batt_voltcap_map[0],
	 .maplen = ARRAY_SIZE(batt_voltcap_map)},/* -15 */
	{.temp = 263,
	 .reset = 0, .fct = 720, .guardband = 100,
	 .esr_vl_lvl = 3600, .esr_vm_lvl = 3800, .esr_vh_lvl = 4000,
	 .esr_vl = 140, .esr_vl_slope = 100, .esr_vl_offset = 0,
	 .esr_vm = 140, .esr_vm_slope = 100, .esr_vm_offset = 0,
	 .esr_vh = 140, .esr_vh_slope = 100, .esr_vh_offset = 0,
	 .esr_vf = 140, .esr_vf_slope = 100, .esr_vf_offset = 0,
	 .vcmap = &batt_voltcap_map[0],
	 .maplen = ARRAY_SIZE(batt_voltcap_map)},/* -10 */
	{.temp = 268,
	 .reset = 0, .fct = 782, .guardband = 100,
	 .esr_vl_lvl = 3600, .esr_vm_lvl = 3800, .esr_vh_lvl = 4000,
	 .esr_vl = 140, .esr_vl_slope = 100, .esr_vl_offset = 0,
	 .esr_vm = 140, .esr_vm_slope = 100, .esr_vm_offset = 0,
	 .esr_vh = 140, .esr_vh_slope = 100, .esr_vh_offset = 0,
	 .esr_vf = 140, .esr_vf_slope = 100, .esr_vf_offset = 0,
	 .vcmap = &batt_voltcap_map[0],
	 .maplen = ARRAY_SIZE(batt_voltcap_map)},/* -5 */
	{.temp = 273,
	 .reset = 0, .fct = 845, .guardband = 100,
	 .esr_vl_lvl = 3600, .esr_vm_lvl = 3800, .esr_vh_lvl = 4000,
	 .esr_vl = 140, .esr_vl_slope = 100, .esr_vl_offset = 0,
	 .esr_vm = 140, .esr_vm_slope = 100, .esr_vm_offset = 0,
	 .esr_vh = 140, .esr_vh_slope = 100, .esr_vh_offset = 0,
	 .esr_vf = 140, .esr_vf_slope = 100, .esr_vf_offset = 0,
	 .vcmap = &batt_voltcap_map[0],
	 .maplen = ARRAY_SIZE(batt_voltcap_map)},/* 0 */
	{.temp = 278,
	 .reset = 0, .fct = 880, .guardband = 100,
	 .esr_vl_lvl = 3600, .esr_vm_lvl = 3800, .esr_vh_lvl = 4000,
	 .esr_vl = 140, .esr_vl_slope = 100, .esr_vl_offset = 0,
	 .esr_vm = 140, .esr_vm_slope = 100, .esr_vm_offset = 0,
	 .esr_vh = 140, .esr_vh_slope = 100, .esr_vh_offset = 0,
	 .esr_vf = 140, .esr_vf_slope = 100, .esr_vf_offset = 0,
	 .vcmap = &batt_voltcap_map[0],
	 .maplen = ARRAY_SIZE(batt_voltcap_map)},/* 5 */
	{.temp = 283,
	 .reset = 0, .fct = 915, .guardband = 30,
	 .esr_vl_lvl = 3600, .esr_vm_lvl = 3800, .esr_vh_lvl = 4000,
	 .esr_vl = 140, .esr_vl_slope = 100, .esr_vl_offset = 0,
	 .esr_vm = 140, .esr_vm_slope = 100, .esr_vm_offset = 0,
	 .esr_vh = 140, .esr_vh_slope = 100, .esr_vh_offset = 0,
	 .esr_vf = 140, .esr_vf_slope = 100, .esr_vf_offset = 0,
	 .vcmap = &batt_voltcap_map[0],
	 .maplen = ARRAY_SIZE(batt_voltcap_map)},/* 10 */
	{.temp = 288,
	 .reset = 0, .fct = 957, .guardband = 30,
	 .esr_vl_lvl = 3600, .esr_vm_lvl = 3800, .esr_vh_lvl = 4000,
	 .esr_vl = 140, .esr_vl_slope = 100, .esr_vl_offset = 0,
	 .esr_vm = 140, .esr_vm_slope = 100, .esr_vm_offset = 0,
	 .esr_vh = 140, .esr_vh_slope = 100, .esr_vh_offset = 0,
	 .esr_vf = 140, .esr_vf_slope = 100, .esr_vf_offset = 0,
	 .vcmap = &batt_voltcap_map[0],
	 .maplen = ARRAY_SIZE(batt_voltcap_map)},/* 15 */
	{.temp = 293,
	 .reset = 0, .fct = 1000, .guardband = 30,
	 .esr_vl_lvl = 3600, .esr_vm_lvl = 3800, .esr_vh_lvl = 4000,
	 .esr_vl = 140, .esr_vl_slope = 100, .esr_vl_offset = 0,
	 .esr_vm = 140, .esr_vm_slope = 100, .esr_vm_offset = 0,
	 .esr_vh = 140, .esr_vh_slope = 100, .esr_vh_offset = 0,
	 .esr_vf = 140, .esr_vf_slope = 100, .esr_vf_offset = 0,
	 .vcmap = &batt_voltcap_map[0],
	 .maplen = ARRAY_SIZE(batt_voltcap_map)},/* 20 */
};

static struct bcmpmu_platform_data bcmpmu_plat_data = {
	.i2c_pdata = { ADD_I2C_SLAVE_SPEED(BSC_BUS_SPEED_400K), },
	.init = bcmpmu_init_platform_hw,
	.exit = bcmpmu_exit_platform_hw,
	.i2c_board_info_map1 = &pmu_info_map1,
	.i2c_adapter_id = PMU_DEVICE_I2C_BUSNO,
	.i2c_pagesize = 256,
	.init_data = &register_init_data[0],
	/* # of registers defined in register_init_data.
	   This value will come from device tree */
	.init_max = ARRAY_SIZE(register_init_data),
	.batt_temp_map = &batt_temp_map[0],
	.batt_temp_map_len = ARRAY_SIZE(batt_temp_map),
	.adc_setting = &adc_setting,
	.num_of_regl = ARRAY_SIZE(bcm59039_regulators),
	.regulator_init_data = &bcm59039_regulators[0],
	.fg_smpl_rate = 2083,
	.fg_slp_rate = 32000,
	.fg_slp_curr_ua = 1000,
	.fg_factor = 976,
	.fg_sns_res = 10,
	.batt_voltcap_map = &batt_voltcap_map[0],
	.batt_voltcap_map_len = ARRAY_SIZE(batt_voltcap_map),
	.batt_impedence = 140,
	.chrg_1c_rate = 1300,
	.chrg_eoc = 65,
	.chrg_zone_map = &chrg_zone[0],
	.fg_capacity_full = 1300 * 3600,
	.support_fg = 1,
	.support_chrg_maint = 1,
	.wd_setting = &bcm59039_wd_setting,
	.chrg_resume_lvl = 4150,
	.fg_support_tc = 1,
	.fg_tc_dn_lvl = 278,
	.fg_tc_up_lvl = 288,
	.fg_zone_settle_tm = 60,
	.fg_zone_info = &fg_zone[0],
	.bc = BCMPMU_BC_PMU_BC12,
	.batt_model = "Unknown",
	.cutoff_volt = 3300,
	.cutoff_count_max = 3,
	.hard_reset_en = -1,
	.restart_en = -1,
	.pok_hold_deb = -1,
	.pok_shtdwn_dly = -1,
	.pok_restart_dly = -1,
	.pok_restart_deb = -1,
	.pok_lock = 1, /*Keep ponkey locked by default*/
};

static struct i2c_board_info __initdata pmu_info[] = {
	{
		I2C_BOARD_INFO("bcmpmu", PMU_DEVICE_I2C_ADDR),
		.platform_data = &bcmpmu_plat_data,
		.irq = gpio_to_irq(PMU_DEVICE_INT_GPIO),
	},
};


/*800 Mhz CSR voltage definitions....*/

#define CSR_VAL_RETN_SS_800M	0x3 /*0.88V*/
#define CSR_VAL_RETN_TT_800M	0x3 /*0.88V*/
#define CSR_VAL_RETN_FF_800M	0x3 /*0.88V*/

#define CSR_VAL_ECO_SS_800M		0xd /*1.08V*/
#define CSR_VAL_ECO_TT_800M		0x8 /*0.98V*/
#define CSR_VAL_ECO_FF_800M		0x8 /*0.98V*/

#define CSR_VAL_NRML_SS_800M	0x11 /*1.16V*/
#define CSR_VAL_NRML_TT_800M	0x0b /*1.04V*/
#define CSR_VAL_NRML_FF_800M	0x8 /*0.98V*/

#define CSR_VAL_TURBO_SS_800M		0x1A /*1.34V*/
#define B0_CSR_VAL_TURBO_SS_800M	0x19 /*1.32V*/
#define CSR_VAL_TURBO_TT_800M		0x14 /*1.22V*/
#define CSR_VAL_TURBO_FF_800M		0x0F /*1.12V*/



#define PMU_CSR_VLT_TBL_SS_800M	ARRAY_LIST(\
					CSR_VAL_RETN_SS_800M,\
					CSR_VAL_RETN_SS_800M,\
					CSR_VAL_RETN_SS_800M,\
					CSR_VAL_RETN_SS_800M,\
					CSR_VAL_RETN_SS_800M,\
					CSR_VAL_RETN_SS_800M,\
					CSR_VAL_RETN_SS_800M,\
					CSR_VAL_RETN_SS_800M,\
					CSR_VAL_ECO_SS_800M,\
					CSR_VAL_ECO_SS_800M,\
					CSR_VAL_ECO_SS_800M,\
					CSR_VAL_NRML_SS_800M,\
					CSR_VAL_NRML_SS_800M,\
					CSR_VAL_NRML_SS_800M,\
					CSR_VAL_TURBO_SS_800M,\
					CSR_VAL_TURBO_SS_800M)


#define PMU_CSR_VLT_TBL_TT_800M	ARRAY_LIST(\
					CSR_VAL_RETN_TT_800M,\
					CSR_VAL_RETN_TT_800M,\
					CSR_VAL_RETN_TT_800M,\
					CSR_VAL_RETN_TT_800M,\
					CSR_VAL_RETN_TT_800M,\
					CSR_VAL_RETN_TT_800M,\
					CSR_VAL_RETN_TT_800M,\
					CSR_VAL_RETN_TT_800M,\
					CSR_VAL_ECO_TT_800M,\
					CSR_VAL_ECO_TT_800M,\
					CSR_VAL_ECO_TT_800M,\
					CSR_VAL_NRML_TT_800M,\
					CSR_VAL_NRML_TT_800M,\
					CSR_VAL_NRML_TT_800M,\
					CSR_VAL_TURBO_TT_800M,\
					CSR_VAL_TURBO_TT_800M)

#define PMU_CSR_VLT_TBL_FF_800M	ARRAY_LIST(\
					CSR_VAL_RETN_FF_800M,\
					CSR_VAL_RETN_FF_800M,\
					CSR_VAL_RETN_FF_800M,\
					CSR_VAL_RETN_FF_800M,\
					CSR_VAL_RETN_FF_800M,\
					CSR_VAL_RETN_FF_800M,\
					CSR_VAL_RETN_FF_800M,\
					CSR_VAL_RETN_FF_800M,\
					CSR_VAL_ECO_FF_800M,\
					CSR_VAL_ECO_FF_800M,\
					CSR_VAL_ECO_FF_800M,\
					CSR_VAL_NRML_FF_800M,\
					CSR_VAL_NRML_FF_800M,\
					CSR_VAL_NRML_FF_800M,\
					CSR_VAL_TURBO_FF_800M,\
					CSR_VAL_TURBO_FF_800M)


/*850 Mhz CSR voltage definitions....*/

#define CSR_VAL_RETN_SS_850M	0x3 /*0.88V*/
#define CSR_VAL_RETN_TT_850M	0x3 /*0.88V*/
#define CSR_VAL_RETN_FF_850M	0x3 /*0.88V*/

#define CSR_VAL_ECO_SS_850M		0xd /*1.08V*/
#define CSR_VAL_ECO_TT_850M		0x8 /*0.98V*/
#define CSR_VAL_ECO_FF_850M		0x8 /*0.98V*/

#define CSR_VAL_NRML_SS_850M	0x11 /*1.16V*/
#define CSR_VAL_NRML_TT_850M	0x0b /*1.04V*/
#define CSR_VAL_NRML_FF_850M	0x8 /*0.98V*/

#define CSR_VAL_TURBO_SS_850M		0x1B /*1.36V*/
#define B0_CSR_VAL_TURBO_SS_850M	0x19 /*1.32V*/
#define CSR_VAL_TURBO_TT_850M		0x15 /*1.24V*/
#define CSR_VAL_TURBO_FF_850M		0x10 /*1.14V*/



#define PMU_CSR_VLT_TBL_SS_850M	ARRAY_LIST(\
					CSR_VAL_RETN_SS_850M,\
					CSR_VAL_RETN_SS_850M,\
					CSR_VAL_RETN_SS_850M,\
					CSR_VAL_RETN_SS_850M,\
					CSR_VAL_RETN_SS_850M,\
					CSR_VAL_RETN_SS_850M,\
					CSR_VAL_RETN_SS_850M,\
					CSR_VAL_RETN_SS_850M,\
					CSR_VAL_ECO_SS_850M,\
					CSR_VAL_ECO_SS_850M,\
					CSR_VAL_ECO_SS_850M,\
					CSR_VAL_NRML_SS_850M,\
					CSR_VAL_NRML_SS_850M,\
					CSR_VAL_NRML_SS_850M,\
					CSR_VAL_TURBO_SS_850M,\
					CSR_VAL_TURBO_SS_850M)


#define PMU_CSR_VLT_TBL_TT_850M	ARRAY_LIST(\
					CSR_VAL_RETN_TT_850M,\
					CSR_VAL_RETN_TT_850M,\
					CSR_VAL_RETN_TT_850M,\
					CSR_VAL_RETN_TT_850M,\
					CSR_VAL_RETN_TT_850M,\
					CSR_VAL_RETN_TT_850M,\
					CSR_VAL_RETN_TT_850M,\
					CSR_VAL_RETN_TT_850M,\
					CSR_VAL_ECO_TT_850M,\
					CSR_VAL_ECO_TT_850M,\
					CSR_VAL_ECO_TT_850M,\
					CSR_VAL_NRML_TT_850M,\
					CSR_VAL_NRML_TT_850M,\
					CSR_VAL_NRML_TT_850M,\
					CSR_VAL_TURBO_TT_850M,\
					CSR_VAL_TURBO_TT_850M)

#define PMU_CSR_VLT_TBL_FF_850M	ARRAY_LIST(\
						CSR_VAL_RETN_FF_850M,\
						CSR_VAL_RETN_FF_850M,\
						CSR_VAL_RETN_FF_850M,\
						CSR_VAL_RETN_FF_850M,\
						CSR_VAL_RETN_FF_850M,\
						CSR_VAL_RETN_FF_850M,\
						CSR_VAL_RETN_FF_850M,\
						CSR_VAL_RETN_FF_850M,\
						CSR_VAL_ECO_FF_850M,\
						CSR_VAL_ECO_FF_850M,\
						CSR_VAL_ECO_FF_850M,\
						CSR_VAL_NRML_FF_850M,\
						CSR_VAL_NRML_FF_850M,\
						CSR_VAL_NRML_FF_850M,\
						CSR_VAL_TURBO_FF_850M,\
						CSR_VAL_TURBO_FF_850M)

/*1 Ghz CSR voltage definitions....*/

#define CSR_VAL_RETN_SS_1G	0x3 /*0.88V*/
#define CSR_VAL_RETN_TT_1G	0x3 /*0.88V*/
#define CSR_VAL_RETN_FF_1G	0x3 /*0.88V*/

#define CSR_VAL_ECO_SS_1G	0xd /*1.08V*/
#define CSR_VAL_ECO_TT_1G	0x8 /*0.98V*/
#define CSR_VAL_ECO_FF_1G	0x8 /*0.98V*/

#define CSR_VAL_NRML_SS_1G	0x11 /*1.16V*/
#define CSR_VAL_NRML_TT_1G	0x0b /*1.04V*/
#define CSR_VAL_NRML_FF_1G	0x8	/*0.98V*/

#define CSR_VAL_TURBO_SS_1G		0x1B /*1.36V*/
#define B0_CSR_VAL_TURBO_SS_1G	0x19 /*1.32V*/
#define CSR_VAL_TURBO_TT_1G		0x1B /*1.36V*/
#define B0_CSR_VAL_TURBO_TT_1G	0x19 /*1.32V*/
#define CSR_VAL_TURBO_FF_1G		0x15 /*1.24V*/



#define PMU_CSR_VLT_TBL_SS_1G	ARRAY_LIST(\
						CSR_VAL_RETN_SS_1G,\
						CSR_VAL_RETN_SS_1G,\
						CSR_VAL_RETN_SS_1G,\
						CSR_VAL_RETN_SS_1G,\
						CSR_VAL_RETN_SS_1G,\
						CSR_VAL_RETN_SS_1G,\
						CSR_VAL_RETN_SS_1G,\
						CSR_VAL_RETN_SS_1G,\
						CSR_VAL_ECO_SS_1G,\
						CSR_VAL_ECO_SS_1G,\
						CSR_VAL_ECO_SS_1G,\
						CSR_VAL_NRML_SS_1G,\
						CSR_VAL_NRML_SS_1G,\
						CSR_VAL_NRML_SS_1G,\
						CSR_VAL_TURBO_SS_1G,\
						CSR_VAL_TURBO_SS_1G)

#define PMU_CSR_VLT_TBL_TT_1G	ARRAY_LIST(\
						CSR_VAL_RETN_TT_1G,\
						CSR_VAL_RETN_TT_1G,\
						CSR_VAL_RETN_TT_1G,\
						CSR_VAL_RETN_TT_1G,\
						CSR_VAL_RETN_TT_1G,\
						CSR_VAL_RETN_TT_1G,\
						CSR_VAL_RETN_TT_1G,\
						CSR_VAL_RETN_TT_1G,\
						CSR_VAL_ECO_TT_1G,\
						CSR_VAL_ECO_TT_1G,\
						CSR_VAL_ECO_TT_1G,\
						CSR_VAL_NRML_TT_1G,\
						CSR_VAL_NRML_TT_1G,\
						CSR_VAL_NRML_TT_1G,\
						CSR_VAL_TURBO_TT_1G,\
						CSR_VAL_TURBO_TT_1G)

#define PMU_CSR_VLT_TBL_FF_1G	ARRAY_LIST(\
						CSR_VAL_RETN_FF_1G,\
						CSR_VAL_RETN_FF_1G,\
						CSR_VAL_RETN_FF_1G,\
						CSR_VAL_RETN_FF_1G,\
						CSR_VAL_RETN_FF_1G,\
						CSR_VAL_RETN_FF_1G,\
						CSR_VAL_RETN_FF_1G,\
						CSR_VAL_RETN_FF_1G,\
						CSR_VAL_ECO_FF_1G,\
						CSR_VAL_ECO_FF_1G,\
						CSR_VAL_ECO_FF_1G,\
						CSR_VAL_NRML_FF_1G,\
						CSR_VAL_NRML_FF_1G,\
						CSR_VAL_NRML_FF_1G,\
						CSR_VAL_TURBO_FF_1G,\
						CSR_VAL_TURBO_FF_1G)

u8 csr_vlt_table_ss[A9_FREQ_MAX][SR_VLT_LUT_SIZE] = {
	[A9_FREQ_800_MHZ]	= PMU_CSR_VLT_TBL_SS_800M,
	[A9_FREQ_850_MHZ]	= PMU_CSR_VLT_TBL_SS_850M,
	[A9_FREQ_1_GHZ]		= PMU_CSR_VLT_TBL_SS_1G,
};

u8 csr_vlt_table_tt[A9_FREQ_MAX][SR_VLT_LUT_SIZE] = {
	[A9_FREQ_800_MHZ]	= PMU_CSR_VLT_TBL_TT_800M,
	[A9_FREQ_850_MHZ]	= PMU_CSR_VLT_TBL_TT_850M,
	[A9_FREQ_1_GHZ]		= PMU_CSR_VLT_TBL_TT_1G,
};

u8 csr_vlt_table_ff[A9_FREQ_MAX][SR_VLT_LUT_SIZE] = {
	[A9_FREQ_800_MHZ]	= PMU_CSR_VLT_TBL_FF_800M,
	[A9_FREQ_850_MHZ]	= PMU_CSR_VLT_TBL_FF_850M,
	[A9_FREQ_1_GHZ]		= PMU_CSR_VLT_TBL_FF_1G,
};

const u8 *bcmpmu_get_sr_vlt_table(int sr, u32 freq_inx,
						u32 silicon_type)
{
	pr_info("%s:sr = %i, freq_inx = %d,"
			"silicon_type = %d\n", __func__,
			sr, freq_inx, silicon_type);

	BUG_ON(!vlt_tbl_init ||
		freq_inx > A9_FREQ_1_GHZ);

#ifdef CONFIG_KONA_AVS
	switch (silicon_type) {
	case SILICON_TYPE_SLOW:
		return csr_vlt_table_ss[freq_inx];

	case SILICON_TYPE_TYPICAL:
		return csr_vlt_table_tt[freq_inx];

	case SILICON_TYPE_FAST:
		return csr_vlt_table_ff[freq_inx];

	default:
		BUG();
	}
#else
	return csr_vlt_table_ss[freq_inx];
#endif
}

int bcmpmu_init_platform_hw(struct bcmpmu *bcmpmu)
{
	int             i;
	printk(KERN_INFO "%s: called.\n", __func__);

	/* Samsung requirement for PMU restart should be enabled.
	 * Will get configured only 59039C0 or above version
	*/

	if (bcmpmu->rev_info.dig_rev >= BCM59039_CO_DIG_REV) {
		bcmpmu->pdata->restart_en = 1;
		bcmpmu->pdata->pok_restart_dly = POK_RESTRT_DLY_4SEC;
		bcmpmu->pdata->pok_restart_deb = POK_RESTRT_DEB_4SEC;
		bcmpmu->pdata->pok_lock = 1;
		bcmpmu->pdata->hard_reset_en = 0;
	} else {

		memset(&csr_vlt_table_ss[A9_FREQ_800_MHZ][SR_TURBO_INX_START],
			B0_CSR_VAL_TURBO_SS_800M,
			(SR_TURBO_INX_END - SR_TURBO_INX_START) + 1);

		memset(&csr_vlt_table_ss[A9_FREQ_850_MHZ][SR_TURBO_INX_START],
			B0_CSR_VAL_TURBO_SS_850M,
			(SR_TURBO_INX_END - SR_TURBO_INX_START) + 1);

		memset(&csr_vlt_table_ss[A9_FREQ_1_GHZ][SR_TURBO_INX_START],
			B0_CSR_VAL_TURBO_SS_1G,
			(SR_TURBO_INX_END - SR_TURBO_INX_START) + 1);

		memset(&csr_vlt_table_tt[A9_FREQ_1_GHZ][SR_TURBO_INX_START],
			B0_CSR_VAL_TURBO_TT_1G,
			(SR_TURBO_INX_END - SR_TURBO_INX_START) + 1);

	}

	vlt_tbl_init = 1;

	for (i = 0; i < ARRAY_SIZE(bcmpmu_client_devices); i++)
		bcmpmu_client_devices[i]->dev.platform_data = bcmpmu;
	platform_add_devices(bcmpmu_client_devices,
			ARRAY_SIZE(bcmpmu_client_devices));

	return 0;
}



__init int board_pmu_init(void)
{
	int             ret;
	int             irq;

#ifdef CONFIG_KONA_DT_BCMPMU
	bcmpmu_update_pdata_dt_batt(&bcmpmu_plat_data);
	bcmpmu_update_pdata_dt_pmu(&bcmpmu_plat_data);
#endif
	ret = gpio_request(PMU_DEVICE_INT_GPIO, "bcmpmu-irq");
	if (ret < 0) {

		printk(KERN_ERR "%s filed at gpio_request.\n", __func__);
		goto exit;
	}
	ret = gpio_direction_input(PMU_DEVICE_INT_GPIO);
	if (ret < 0) {

		printk(KERN_ERR "%s filed at gpio_direction_input.\n",
				__func__);
		goto exit;
	}
	irq = gpio_to_irq(PMU_DEVICE_INT_GPIO);
	bcmpmu_plat_data.irq = irq;

	i2c_register_board_info(PMU_DEVICE_I2C_BUSNO,
				pmu_info, ARRAY_SIZE(pmu_info));
exit:
	return ret;
}


arch_initcall(board_pmu_init);
