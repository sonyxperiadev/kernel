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
#ifdef CONFIG_RHEA_AVS
#include <mach/rhea_avs.h>
#endif
#include "pm_params.h"

#define PMU_DEVICE_I2C_ADDR	0x08
#define PMU_DEVICE_I2C_ADDR1	0x0C
#define PMU_DEVICE_INT_GPIO	29
#define PMU_DEVICE_I2C_BUSNO 2

static struct bcmpmu_rw_data register_init_data[] = {
	{.map = 0, .addr = 0x01, .val = 0x00, .mask = 0x01},
	{.map = 0, .addr = 0x0c, .val = 0x1b, .mask = 0xFF},
#if defined(CONFIG_MACH_RHEA_STONE) || defined(CONFIG_MACH_RHEA_STONE_EDN2X)
	{.map = 0, .addr = 0x13, .val = 0x3d, .mask = 0xFF},
	{.map = 0, .addr = 0x14, .val = 0x79, .mask = 0xFF},
	{.map = 0, .addr = 0x15, .val = 0x20, .mask = 0xFF},
#else
	{.map = 0, .addr = 0x13, .val = 0x43, .mask = 0xFF},
	{.map = 0, .addr = 0x14, .val = 0x7F, .mask = 0xFF},
	{.map = 0, .addr = 0x15, .val = 0x3B, .mask = 0xFF},
#endif /* CONFIG_MACH_RHEA_STONE */
	{.map = 0, .addr = 0x16, .val = 0xF8, .mask = 0xFF},
	{.map = 0, .addr = 0x1D, .val = 0x09, .mask = 0xFF},
	{.map = 0, .addr = 0x2D, .val = 0x00, .mask = 0xF3},
	{.map = 0, .addr = 0x40, .val = 0xFF, .mask = 0xFF},
	{.map = 0, .addr = 0x41, .val = 0xFF, .mask = 0xFF},
	{.map = 0, .addr = 0x42, .val = 0xFF, .mask = 0xFF},
	{.map = 0, .addr = 0x43, .val = 0xFF, .mask = 0xFF},
	{.map = 0, .addr = 0x44, .val = 0xFF, .mask = 0xFF},
	{.map = 0, .addr = 0x45, .val = 0xFF, .mask = 0xFF},
	{.map = 0, .addr = 0x46, .val = 0xFF, .mask = 0xFF},
	{.map = 0, .addr = 0x47, .val = 0xFF, .mask = 0xFF},
	{.map = 0, .addr = 0x50, .val = 0x3B, .mask = 0xFF},
	{.map = 0, .addr = 0x52, .val = 0x04, .mask = 0x04},
	{.map = 0, .addr = 0x58, .val = 0x05, .mask = 0x0F},
	/* USB_FC_OPTION needed to be 1, in order to charge from SDP */
	{.map = 0, .addr = 0x5E, .val = 0x30, .mask = 0xFF},

	/**
	 * Switchers initializations
	 * If you modifying SDSR/ASR/IOSR/CSR values, please make sure
	 * to update rheastone-edn20-pmu-59039.dtsi file also
	 */

	/*Init SDSR NM, NM2 and LPM voltages to 1.24V
	*/
	{.map = 0, .addr = 0xD0, .val = 0x15, .mask = 0xFF},
	{.map = 0, .addr = 0xD1, .val = 0x15, .mask = 0xFF},
	{.map = 0, .addr = 0xD2, .val = 0x15, .mask = 0xFF},

	/*Init CSR LPM  to 0.88 V
	CSR NM2 to 1.22V
	*/
	{.map = 0, .addr = 0xC1, .val = 0x03, .mask = 0xFF},
	{.map = 0, .addr = 0xC2, .val = 0x14, .mask = 0xFF},

	/*PLLCTRL, Clear Bit 0 to disable PLL when PC2:PC1 = 0b00*/
	{.map = 0, .addr = 0x0A, .val = 0x0E, .mask = 0x0F},
	/*CMPCTRL13, Set bits 4, 1 for BSI Sync. Mode */
	{.map = 0, .addr = 0x1C, .val = 0x13, .mask = 0xFF},
	/*CMPCTRL12, Set bits 4, 1 for NTC Sync. Mode*/
	{.map = 0, .addr = 0x1B, .val = 0x13, .mask = 0xFF},

#ifdef CONFIG_MACH_RHEA_STONE_EDN2X
	{.map = 0, .addr = 0xD9, .val = 0x1A, .mask = 0xFF},
#else
	/*Init ASR LPM to 2.9V - for Rhea EDN10 & EDN00 and 1.8V for EDN2x
	*/
	{.map = 0, .addr = 0xD9, .val = 0x1F, .mask = 0xFF},
	/*Init IOSR NM2 and LPM voltages to 1.8V
	*/
	{.map = 0, .addr = 0xC9, .val = 0x1A, .mask = 0xFF},
	{.map = 0, .addr = 0xCA, .val = 0x1A, .mask = 0xFF},
#endif /*CONFIG_MACH_RHEA_STONE_EDN2X*/

	/*FGOPMODCTRL, Set bits 4, 1 for FG Sync. Mode*/
	{.map = 1, .addr = 0x42, .val = 0x11, .mask = 0xFF},

};

static struct bcmpmu_temp_map batt_temp_map[] = {
	/*
	* This table is hardware dependent and need to get from platform team
	*/
	/*
	* adc temp
	*/
	{932, -400},			/* -40 C */
	{900, -350},			/* -35 C */
	{860, -300},			/* -30 C */
	{816, -250},			/* -25 C */
	{760, -200},			/* -20 C */
	{704, -150},			/* -15 C */
	{636, -100},			/* -10 C */
	{568, -50},			/* -5 C */
	{500, 0},			/* 0 C */
	{440, 50},			/* 5 C */
	{376, 100},			/* 10 C */
	{324, 150},			/* 15 C */
	{272, 200},			/* 20 C */
	{228, 250},			/* 25 C */
	{192, 300},			/* 30 C */
	{160, 350},			/* 35 C */
	{132, 400},			/* 40 C */
	{112, 450},			/* 45 C */
	{92, 500},			/* 50 C */
	{76, 550},			/* 55 C */
	{64, 600},			/* 60 C */
	{52, 650},			/* 65 C */
	{44, 700},			/* 70 C */
	{36, 750},			/* 75 C */
	{32, 800},			/* 80 C */
	{28, 850},			/* 85 C */
	{24, 900},			/* 90 C */
	{20, 950},			/* 95 C */
	{16, 1000},			/* 100 C */
};

static struct bcmpmu_temp_map  pmu_temp_sensor[] = {

	{466,	-40},
	{476,	-35},
	{485,	-30},
	{495,	-25},
	{505,	-20},
	{515,	-15},
	{524,	-10},
	{534,	-5},
	{544,	0},
	{554,	5},
	{564,	10},
	{574,	15},
	{584,	20},
	{594,	25},
	{604,	30},
	{614,	35},
	{624,	40},
	{634,	45},
	{644,	50},
	{655,	55},
	{665,	60},
	{675,	65},
	{685,	70},
	{696,	75},
	{706,	80},
	{716,	85},
	{727,	90},
	{737,	95},
	{747,	100},
	{758,	105},
	{768,	110},
	{778,	115},
	{789,	120},
	{799,	125},
	{809,	130},
	{819,	135},
	{829,	140},
	{839,	145},
	{849,	150},
	{859,	155},
	{869,	160},
	{879,	165},
	{889,	170},
	{899,	175},

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
	{.supply = "hv2ldo_uc"},
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
	{.supply = "dummy"}, /* Add a dummy variable to ensure we can use an array of 3 in rhea_ray.
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
			.always_on = 0,
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
/*TODO: We observed that, on Rhearay HW, interrupt from GPIO expander
is not detected by baseband if SIMLDO is disabled. As a temp. workaround
we keep SIMLDO ON by default for Rhearay till the issue is root casued*/
#ifdef CONFIG_MACH_RHEA_RAY_EDN2X
			.always_on = 1,
#endif

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
#ifndef CONFIG_MACH_RHEA_STONE_EDN2X
			.always_on = 1,
#endif
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
#ifndef CONFIG_MACH_RHEA_STONE_EDN2X
			.always_on = 1,
#endif
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
#ifndef CONFIG_MACH_RHEA_STONE_EDN2X
			.always_on = 1,
#endif
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
		BCMPMU_REGULATOR_HV3LDO, &bcm59039_hv3ldo_data, 0xAA,
		BCMPMU_REGL_LPM_IN_DSM
	},
	[BCMPMU_REGULATOR_HV4LDO] =	{
		BCMPMU_REGULATOR_HV4LDO, &bcm59039_hv4ldo_data, 0xAA, 0
	},
	[BCMPMU_REGULATOR_HV5LDO] = {
		BCMPMU_REGULATOR_HV5LDO, &bcm59039_hv5ldo_data, 0x11, 0
	},
	[BCMPMU_REGULATOR_HV6LDO] = {
		BCMPMU_REGULATOR_HV6LDO, &bcm59039_hv6ldo_data, 0xAA,
		BCMPMU_REGL_LPM_IN_DSM
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
		BCMPMU_REGULATOR_HV10LDO, &bcm59039_hv10ldo_data, 0xAA,
			BCMPMU_REGL_LPM_IN_DSM
	},

/*TODO: We observed that, on Rhearay HW, interrupt from GPIO expander
is not detected by baseband if SIMLDO is disabled. As a temp. workaround
we keep SIMLDO ON by default for Rhearay till the issue is root casued*/
#ifdef CONFIG_MACH_RHEA_RAY_EDN2X
	[BCMPMU_REGULATOR_SIMLDO] = {
		BCMPMU_REGULATOR_SIMLDO, &bcm59039_simldo_data, 0x00,
			BCMPMU_REGL_LPM_IN_DSM
	},
#else
	[BCMPMU_REGULATOR_SIMLDO] = {
		BCMPMU_REGULATOR_SIMLDO, &bcm59039_simldo_data, 0xAA,
			BCMPMU_REGL_LPM_IN_DSM
	},
#endif
	[BCMPMU_REGULATOR_CSR_NM] =	{
		BCMPMU_REGULATOR_CSR_NM, &bcm59039_csr_nm_data, 0x11, 0
	},
	[BCMPMU_REGULATOR_CSR_NM2] = {
		BCMPMU_REGULATOR_CSR_NM2, &bcm59039_csr_nm2_data,
		BCMPMU_REGU_OPMODE_DC, 0
	},
	[BCMPMU_REGULATOR_CSR_LPM] = {
		BCMPMU_REGULATOR_CSR_LPM, &bcm59039_csr_lpm_data,
		BCMPMU_REGU_OPMODE_DC, 0
	},
#ifdef CONFIG_MACH_RHEA_STONE_EDN2X
	/*EDN2x does not use IOSR*/
	[BCMPMU_REGULATOR_IOSR_NM] = {
		BCMPMU_REGULATOR_IOSR_NM, &bcm59039_iosr_nm_data, 0xAA, 0
	},
#else
	[BCMPMU_REGULATOR_IOSR_NM] = {
		BCMPMU_REGULATOR_IOSR_NM, &bcm59039_iosr_nm_data, 0x01, 0
	},
#endif /*CONFIG_MACH_RHEA_STONE_EDN2X*/
	[BCMPMU_REGULATOR_IOSR_NM2] = {
		BCMPMU_REGULATOR_IOSR_NM2, &bcm59039_iosr_nm2_data,
		BCMPMU_REGU_OPMODE_DC, 0
	},
	[BCMPMU_REGULATOR_IOSR_LPM] = {
		BCMPMU_REGULATOR_IOSR_LPM, &bcm59039_iosr_lpm_data,
		BCMPMU_REGU_OPMODE_DC, 0
	},
	[BCMPMU_REGULATOR_SDSR_NM] = {
		BCMPMU_REGULATOR_SDSR_NM, &bcm59039_sdsr_nm_data, 0x11, 0
	},
	[BCMPMU_REGULATOR_SDSR_NM2] = {
		BCMPMU_REGULATOR_SDSR_NM2, &bcm59039_sdsr_nm2_data,
		BCMPMU_REGU_OPMODE_DC, 0
	},
	[BCMPMU_REGULATOR_SDSR_LPM] = {
		BCMPMU_REGULATOR_SDSR_LPM, &bcm59039_sdsr_lpm_data,
		BCMPMU_REGU_OPMODE_DC, 0
	},
#ifdef CONFIG_MACH_RHEA_STONE_EDN2X
	[BCMPMU_REGULATOR_ASR_NM] = {
		BCMPMU_REGULATOR_ASR_NM, &bcm59039_asr_nm_data, 0x01, 0
	},
#else
	[BCMPMU_REGULATOR_ASR_NM] = {
		BCMPMU_REGULATOR_ASR_NM, &bcm59039_asr_nm_data, 0x11, 0
	},
#endif /*CONFIG_MACH_RHEA_STONE_EDN2X*/

	[BCMPMU_REGULATOR_ASR_NM2] = {
		BCMPMU_REGULATOR_ASR_NM2, &bcm59039_asr_nm2_data,
		BCMPMU_REGU_OPMODE_DC, 0
	},
	[BCMPMU_REGULATOR_ASR_LPM] = {
		BCMPMU_REGULATOR_ASR_LPM, &bcm59039_asr_lpm_data,
		BCMPMU_REGU_OPMODE_DC, 0
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

#ifdef CONFIG_CHARGER_BCMPMU_SPA
static struct platform_device bcmpmu_chrgr_spa_device = {
	.name = "bcmpmu_chrgr_pb",
	.id = -1,
	.dev.platform_data = NULL,
};
#endif

static struct platform_device *bcmpmu_client_devices[] = {
	&bcmpmu_audio_device,
#ifdef CONFIG_CHARGER_BCMPMU_SPA
	&bcmpmu_chrgr_spa_device,
#endif
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
	{.tl = -50, .th = 600, .v = 3000, .fc = 10, .qc = 100},	/* Zone QC */
	{.tl = -50, .th = -1, .v = 4200, .fc = 100, .qc = 0},	/* Zone LL */
	{.tl = 0, .th = 99, .v = 4200, .fc = 100, .qc = 0},	/* Zone L */
	{.tl = 100, .th = 450, .v = 4200, .fc = 100, .qc = 0},	/* Zone N */
	{.tl = 451, .th = 500, .v = 4200, .fc = 100, .qc = 0},	/* Zone H */
	{.tl = 501, .th = 600, .v = 4200, .fc = 100, .qc = 0},	/* Zone HH */
	{.tl = -50, .th = 600, .v = 0, .fc = 0, .qc = 0},	/* Zone OUT */
};

static struct bcmpmu_voltcap_map batt_voltcap_map[] = {
	{4153, 100},
	{4088, 95},
	{4045, 90},
	{4007, 85},
	{3975, 81},
	{3946, 76},
	{3919, 71},
	{3896, 66},
	{3870, 61},
	{3839, 56},
	{3814, 51},
	{3798, 47},
	{3788, 42},
	{3781, 37},
	{3775, 32},
	{3764, 27},
	{3742, 22},
	{3721, 17},
	{3687, 13},
	{3679, 11},
	{3675, 10},
	{3672, 9},
	{3668, 8},
	{3663, 7},
	{3657, 6},
	{3642, 5},
	{3608, 4},
	{3562, 2},
	{3503, 1},
	{3400, 0},
};

static int bcmpmu_init_platform_hw(struct bcmpmu *);

static struct bcmpmu_fg_zone fg_zone[FG_TMP_ZONE_MAX+1] = {
/* This table is default data, the real data from board file or device tree*/
	{.temp = -200,
	 .reset = 0, .fct = 26, .guardband = 100,
	 .esr_vl_lvl = 2970, .esr_vm_lvl = 3207, .esr_vh_lvl = 3326,
	 .esr_vl = 140, .esr_vl_slope = -1766, .esr_vl_offset = 6953,
	 .esr_vm = 140, .esr_vm_slope = -628, .esr_vm_offset = 3573,
	 .esr_vh = 140, .esr_vh_slope = -122, .esr_vh_offset = 1950,
	 .esr_vf = 140, .esr_vf_slope = -1606, .esr_vf_offset = 6886,
	 .vcmap = &batt_voltcap_map[0],
	 .maplen = ARRAY_SIZE(batt_voltcap_map)},/* -20 */
	{.temp = -150,
	 .reset = 0, .fct = 210, .guardband = 100,
	 .esr_vl_lvl = 2970, .esr_vm_lvl = 3207, .esr_vh_lvl = 3326,
	 .esr_vl = 140, .esr_vl_slope = -1766, .esr_vl_offset = 6953,
	 .esr_vm = 140, .esr_vm_slope = -628, .esr_vm_offset = 3573,
	 .esr_vh = 140, .esr_vh_slope = -122, .esr_vh_offset = 1950,
	 .esr_vf = 140, .esr_vf_slope = -1606, .esr_vf_offset = 6886,
	 .vcmap = &batt_voltcap_map[0],
	 .maplen = ARRAY_SIZE(batt_voltcap_map)},/* -15 */
	{.temp = -100,
	 .reset = 0, .fct = 394, .guardband = 100,
	 .esr_vl_lvl = 3317, .esr_vm_lvl = 3382, .esr_vh_lvl = 3621,
	 .esr_vl = 140, .esr_vl_slope = -1590, .esr_vl_offset = 6251,
	 .esr_vm = 140, .esr_vm_slope = -834, .esr_vm_offset = 3743,
	 .esr_vh = 140, .esr_vh_slope = -271, .esr_vh_offset = 1839,
	 .esr_vf = 140, .esr_vf_slope = -978, .esr_vf_offset = 4399,
	 .vcmap = &batt_voltcap_map[0],
	 .maplen = ARRAY_SIZE(batt_voltcap_map)},/* -10 */
	{.temp = -50,
	 .reset = 0, .fct = 565, .guardband = 100,
	 .esr_vl_lvl = 3317, .esr_vm_lvl = 3382, .esr_vh_lvl = 3621,
	 .esr_vl = 140, .esr_vl_slope = -1590, .esr_vl_offset = 6251,
	 .esr_vm = 140, .esr_vm_slope = -834, .esr_vm_offset = 3743,
	 .esr_vh = 140, .esr_vh_slope = -271, .esr_vh_offset = 1839,
	 .esr_vf = 140, .esr_vf_slope = -978, .esr_vf_offset = 4399,
	 .vcmap = &batt_voltcap_map[0],
	 .maplen = ARRAY_SIZE(batt_voltcap_map)},/* -5 */
	{.temp = 0,
	 .reset = 0, .fct = 736, .guardband = 100,
	 .esr_vl_lvl = 3411, .esr_vm_lvl = 3562, .esr_vh_lvl = 3813,
	 .esr_vl = 140, .esr_vl_slope = -1675, .esr_vl_offset = 6427,
	 .esr_vm = 140, .esr_vm_slope = -1342, .esr_vm_offset = 5292,
	 .esr_vh = 140, .esr_vh_slope = -176, .esr_vh_offset = 1139,
	 .esr_vf = 140, .esr_vf_slope = -841, .esr_vf_offset = 3675,
	 .vcmap = &batt_voltcap_map[0],
	 .maplen = ARRAY_SIZE(batt_voltcap_map)},/* 0 */
	{.temp = 50,
	 .reset = 0, .fct = 811, .guardband = 100,
	 .esr_vl_lvl = 3411, .esr_vm_lvl = 3634, .esr_vh_lvl = 3715,
	 .esr_vl = 140, .esr_vl_slope = -1824, .esr_vl_offset = 6782,
	 .esr_vm = 140, .esr_vm_slope = -1020, .esr_vm_offset = 4040,
	 .esr_vh = 140, .esr_vh_slope = 414, .esr_vh_offset = -1171,
	 .esr_vf = 140, .esr_vf_slope = -488, .esr_vf_offset = 2180,
	 .vcmap = &batt_voltcap_map[0],
	 .maplen = ARRAY_SIZE(batt_voltcap_map)},/* 5 */
	{.temp = 100,
	 .reset = 0, .fct = 887, .guardband = 30,
	 .esr_vl_lvl = 3411, .esr_vm_lvl = 3634, .esr_vh_lvl = 3715,
	 .esr_vl = 140, .esr_vl_slope = -1824, .esr_vl_offset = 6782,
	 .esr_vm = 140, .esr_vm_slope = -1020, .esr_vm_offset = 4040,
	 .esr_vh = 140, .esr_vh_slope = 414, .esr_vh_offset = -1171,
	 .esr_vf = 140, .esr_vf_slope = -488, .esr_vf_offset = 2180,
	 .vcmap = &batt_voltcap_map[0],
	 .maplen = ARRAY_SIZE(batt_voltcap_map)},/* 10 */
	{.temp = 150,
	 .reset = 0, .fct = 1000, .guardband = 30,
	 .esr_vl_lvl = 3657, .esr_vm_lvl = 3699, .esr_vh_lvl = 3781,
	 .esr_vl = 140, .esr_vl_slope = -655, .esr_vl_offset = 2562,
	 .esr_vm = 140, .esr_vm_slope = -211, .esr_vm_offset = 982,
	 .esr_vh = 140, .esr_vh_slope = 366, .esr_vh_offset = -1152,
	 .esr_vf = 140, .esr_vf_slope = -346, .esr_vf_offset = 1541,
	 .vcmap = &batt_voltcap_map[0],
	 .maplen = ARRAY_SIZE(batt_voltcap_map)},/* 15 */
	{.temp = 200,
	 .reset = 0, .fct = 1000, .guardband = 30,
	 .esr_vl_lvl = 3657, .esr_vm_lvl = 3699, .esr_vh_lvl = 3781,
	 .esr_vl = 140, .esr_vl_slope = -655, .esr_vl_offset = 2562,
	 .esr_vm = 140, .esr_vm_slope = -211, .esr_vm_offset = 982,
	 .esr_vh = 140, .esr_vh_slope = 366, .esr_vh_offset = -1152,
	 .esr_vf = 140, .esr_vf_slope = -346, .esr_vf_offset = 1541,
	 .vcmap = &batt_voltcap_map[0],
	 .maplen = ARRAY_SIZE(batt_voltcap_map)},/* 20 */
};

#ifdef CONFIG_CHARGER_BCMPMU_SPA
static void notify_spa(enum bcmpmu_event_t event, int data)
{
	printk(KERN_INFO "%s event=%d, data=%d\n",
		__func__, event, data);

	switch (event) {
	case BCMPMU_CHRGR_EVENT_CHGR_DETECTION:
		/* notify SPA driver
		spa_event_handler(SPA_EVT_CHARGER, data)
		*/
		break;
	case BCMPMU_CHRGR_EVENT_MBTEMP:
		/* notify SPA driver
		spa_event_handler(SPA_EVT_TEMP, data);
		*/
		break;
	case BCMPMU_CHRGR_EVENT_MBOV:
		/* notify SPA driver
		spa_event_handler(SPA_EVT_OVP, data);
		*/
		break;
	case BCMPMU_CHRGR_EVENT_USBOV:
		/* notify SPA driver
		spa_event_handler(SPA_EVT_OVP, data);
		*/
		break;
	case BCMPMU_CHRGR_EVENT_EOC:
		/* notify SPA driver
		spa_event_handler(SPA_EVT_EOC, 0);
		*/
		break;
	case BCMPMU_CHRGR_EVENT_CAPACITY:
		/* notify SPA driver
		spa_event_handler(SPA_EVT_CAPACITY, (void *)data);
		*/
		break;
	default:
		break;
	}
}
#endif

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
	.batt_temp_in_celsius = 1,
	.batt_temp_map = &batt_temp_map[0],
	.batt_temp_map_len = ARRAY_SIZE(batt_temp_map),
	.pmu_temp_map = &pmu_temp_sensor[0],
	.pmu_temp_map_len = ARRAY_SIZE(pmu_temp_sensor),
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
	.chrg_1c_rate = 1500,
	.chrg_eoc = 75,
	.support_hw_eoc = 0,
	.chrg_zone_map = &chrg_zone[0],
	.fg_capacity_full = 1500 * 3600,
	.support_fg = 1,
	.support_chrg_maint = 1,
	.wd_setting = &bcm59039_wd_setting,
	.chrg_resume_lvl = 4150,
	.fg_support_tc = 1,
	.fg_tc_dn_lvl = 50, /* 5c */
	.fg_tc_up_lvl = 200, /* 20c */
	.fg_zone_settle_tm = 60,
	.fg_zone_info = &fg_zone[0],
	.fg_poll_hbat = 112000,
	.fg_poll_lbat = 5000,
	.fg_lbat_lvl = 3550,
	.fg_fbat_lvl = 4150,
	.bc = BCMPMU_BC_PMU_BC12,
	.batt_model = "BRCM BL-84 1500mAH",
	.cutoff_volt = 3400,
	.cutoff_count_max = 3,
	.hard_reset_en = -1,
	.restart_en = -1,
	.pok_hold_deb = -1,
	.pok_shtdwn_dly = -1,
	.pok_restart_dly = -1,
	.pok_restart_deb = -1,
	.pok_lock = 1, /*Keep ponkey locked by default*/
	.pok_turn_on_deb = -1,
	.ihf_autoseq_dis = 1,
#ifdef CONFIG_CHARGER_BCMPMU_SPA
	.piggyback_chrg = 1,
	.piggyback_chrg_name = "bcm59039_charger",
	.piggyback_notify = notify_spa,
#endif
};

static struct i2c_board_info __initdata pmu_info[] = {
	{
		I2C_BOARD_INFO("bcmpmu", PMU_DEVICE_I2C_ADDR),
		.platform_data = &bcmpmu_plat_data,
		.irq = gpio_to_irq(PMU_DEVICE_INT_GPIO),
	},
};



/*850 Mhz CSR voltage definitions....*/

#define CSR_VAL_RETN_SS_850M	0x3 /*0.88V*/
#define CSR_VAL_RETN_TT_850M	0x3 /*0.88V*/
#define CSR_VAL_RETN_FF_850M	0x3 /*0.88V*/

#define CSR_VAL_ECO_SS_850M	0xd /*1.08V*/
#define CSR_VAL_ECO_TT_850M	0x8 /*0.98V*/
#define CSR_VAL_ECO_FF_850M	0x8 /*0.98V*/

#define CSR_VAL_NRML_SS_850M	0x10 /*1.14V*/
#define CSR_VAL_NRML_TT_850M	0x0E /*1.10V*/
#define CSR_VAL_NRML_FF_850M	0xA  /*1.02V*/

#define CSR_VAL_TURBO_SS_850M	0x1B /*1.36V*/
#define CSR_VAL_TURBO_TT_850M	0x17 /*1.28V*/
#define CSR_VAL_TURBO_FF_850M	0x11 /*1.16V*/



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


u8 csr_vlt_table_ss[SR_VLT_LUT_SIZE] = PMU_CSR_VLT_TBL_SS_850M;
u8 csr_vlt_table_tt[SR_VLT_LUT_SIZE] = PMU_CSR_VLT_TBL_TT_850M;
u8 csr_vlt_table_ff[SR_VLT_LUT_SIZE] = PMU_CSR_VLT_TBL_FF_850M;


const u8 *bcmpmu_get_sr_vlt_table(int sr, u32 freq_inx,
						u32 silicon_type)
{
	pr_info("%s:sr = %i, freq_inx = %d,"
			"silicon_type = %d\n", __func__,
			sr, freq_inx, silicon_type);

	BUG_ON(freq_inx > A9_FREQ_850_MHZ);

#ifdef CONFIG_RHEA_AVS
	switch (silicon_type) {
	case SILICON_TYPE_SLOW:
		return csr_vlt_table_ss;

	case SILICON_TYPE_TYPICAL:
		return csr_vlt_table_tt;

	case SILICON_TYPE_FAST:
		return csr_vlt_table_ff;

	default:
		BUG();
	}
#else
	return csr_vlt_table_ss;
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
		bcmpmu->pdata->restart_en = 0;
		bcmpmu->pdata->pok_restart_dly = POK_RESTRT_DLY_4SEC;
		bcmpmu->pdata->pok_restart_deb = POK_RESTRT_DEB_10SEC;
		bcmpmu->pdata->pok_lock = 1;
		bcmpmu->pdata->hard_reset_en = 1;
	}

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
/*
	bcmpmu_update_pdata_dt_batt(&bcmpmu_plat_data);
*/
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
