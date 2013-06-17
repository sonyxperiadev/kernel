/*****************************************************************************
*  Copyright 2001 - 2012 Broadcom Corporation.  All rights reserved.
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
#include <linux/export.h>
#include <linux/irq.h>
#include <asm/mach/arch.h>
#include <asm/mach-types.h>
#include <linux/gpio.h>
#include <mach/hardware.h>
#include <linux/io.h>
#include <linux/uaccess.h>
#include <linux/i2c.h>
#include <linux/mfd/core.h>
#include <linux/mfd/bcmpmu59xxx.h>
#include <linux/mfd/bcmpmu59xxx_reg.h>
#include <linux/broadcom/bcmpmu-ponkey.h>
#ifdef CONFIG_KONA_AVS
#include <plat/kona_avs.h>
#endif
#include "pm_params.h"

#define PMU_DEVICE_I2C_ADDR	0x08
#define PMU_DEVICE_I2C_ADDR1	0x0c
#define PMU_DEVICE_INT_GPIO	29
#define PMU_DEVICE_I2C_BUSNO 2

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
static int bcmpmu_init_platform_hw(struct bcmpmu59xxx *bcmpmu);
static int bcmpmu_exit_platform_hw(struct bcmpmu59xxx *bcmpmu);

__weak struct regulator_consumer_supply rf_supply[] = {
	{.supply = "rf"},
};
static struct regulator_init_data bcm59xxx_rfldo_data = {
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
static struct regulator_init_data bcm59xxx_camldo_data = {
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
static struct regulator_init_data bcm59xxx_hv1ldo_data = {
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
static struct regulator_init_data bcm59xxx_hv2ldo_data = {
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
static struct regulator_init_data bcm59xxx_hv3ldo_data = {
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
	{.supply = "dummy"},
/* Add a dummy variable to ensure we can use an array of 3 in rhea_ray.
A hack at best to ensure we redefine the supply in board file. */

};
static struct regulator_init_data bcm59xxx_hv4ldo_data = {
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
static struct regulator_init_data bcm59xxx_hv5ldo_data = {
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
};
static struct regulator_init_data bcm59xxx_hv6ldo_data = {
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
static struct regulator_init_data bcm59xxx_hv7ldo_data = {
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
static struct regulator_init_data bcm59xxx_hv8ldo_data = {
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
static struct regulator_init_data bcm59xxx_hv9ldo_data = {
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

static struct regulator_init_data bcm59xxx_hv10ldo_data = {
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
static struct regulator_init_data bcm59xxx_simldo_data = {
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
static struct regulator_init_data bcm59xxx_csr_nm_data = {
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
static struct regulator_init_data bcm59xxx_csr_nm2_data = {
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
static struct regulator_init_data bcm59xxx_csr_lpm_data = {
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
static struct regulator_init_data bcm59xxx_iosr_nm_data = {
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
static struct regulator_init_data bcm59xxx_iosr_nm2_data = {
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
static struct regulator_init_data bcm59xxx_iosr_lpm_data = {
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

static struct regulator_init_data bcm59xxx_sdsr_nm_data = {
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

static struct regulator_init_data bcm59xxx_sdsr_nm2_data = {
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

static struct regulator_init_data bcm59xxx_sdsr_lpm_data = {
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

static struct regulator_init_data bcm59xxx_asr_nm_data = {
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

static struct regulator_init_data bcm59xxx_asr_nm2_data = {
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

static struct regulator_init_data bcm59xxx_asr_lpm_data = {
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

struct bcmpmu59xxx_regulator_init_data
bcm59xxx_regulators[BCMPMU_REGULATOR_MAX] = {
	[BCMPMU_REGULATOR_RFLDO] = {
		BCMPMU_REGULATOR_RFLDO, &bcm59xxx_rfldo_data, 0x01, 0
	},
	[BCMPMU_REGULATOR_CAMLDO] = {
		BCMPMU_REGULATOR_CAMLDO, &bcm59xxx_camldo_data, 0x11, 0
	},
	[BCMPMU_REGULATOR_HV1LDO] =	{
		BCMPMU_REGULATOR_HV1LDO, &bcm59xxx_hv1ldo_data, 0x11, 0
	},
	[BCMPMU_REGULATOR_HV2LDO] =	{
		BCMPMU_REGULATOR_HV2LDO, &bcm59xxx_hv2ldo_data, 0x11, 0
	},
	[BCMPMU_REGULATOR_HV3LDO] = {
		BCMPMU_REGULATOR_HV3LDO, &bcm59xxx_hv3ldo_data, 0xAA,
		BCMPMU_REGL_LPM_IN_DSM
	},
	[BCMPMU_REGULATOR_HV4LDO] =	{
		BCMPMU_REGULATOR_HV4LDO, &bcm59xxx_hv4ldo_data, 0xAA, 0
	},

	[BCMPMU_REGULATOR_HV5LDO] = {
		BCMPMU_REGULATOR_HV5LDO, &bcm59xxx_hv5ldo_data, 0x11, 0
	},
	[BCMPMU_REGULATOR_HV6LDO] = {
		BCMPMU_REGULATOR_HV6LDO, &bcm59xxx_hv6ldo_data, 0xAA,
		BCMPMU_REGL_LPM_IN_DSM
	},
	[BCMPMU_REGULATOR_HV7LDO] = {
		BCMPMU_REGULATOR_HV7LDO, &bcm59xxx_hv7ldo_data, 0x22, 0
	},
	[BCMPMU_REGULATOR_HV8LDO] = {
		BCMPMU_REGULATOR_HV8LDO, &bcm59xxx_hv8ldo_data, 0x11, 0
	},
	[BCMPMU_REGULATOR_HV9LDO] = {
		BCMPMU_REGULATOR_HV9LDO, &bcm59xxx_hv9ldo_data, 0x11, 0
	},
	[BCMPMU_REGULATOR_HV10LDO] = {
		BCMPMU_REGULATOR_HV10LDO, &bcm59xxx_hv10ldo_data, 0xAA,
			BCMPMU_REGL_LPM_IN_DSM
	},

/*TODO: We observed that, on Rhearay HW, interrupt from GPIO expander
is not detected by baseband if SIMLDO is disabled. As a temp. workaround
we keep SIMLDO ON by default for Rhearay till the issue is root casued*/
#ifdef CONFIG_MACH_RHEA_RAY_EDN2X
	[BCMPMU_REGULATOR_SIMLDO] = {
		BCMPMU_REGULATOR_SIMLDO, &bcm59xxx_simldo_data, 0x00,
			BCMPMU_REGL_LPM_IN_DSM
	},
#else
	[BCMPMU_REGULATOR_SIMLDO] = {
		BCMPMU_REGULATOR_SIMLDO, &bcm59xxx_simldo_data, 0xAA,
			BCMPMU_REGL_LPM_IN_DSM
	},
#endif
	[BCMPMU_REGULATOR_CSR_NM] =	{
		BCMPMU_REGULATOR_CSR_NM, &bcm59xxx_csr_nm_data, 0x11, 0
	},
	[BCMPMU_REGULATOR_CSR_NM2] = {
		BCMPMU_REGULATOR_CSR_NM2, &bcm59xxx_csr_nm2_data, 0xFF, 0
	},
	[BCMPMU_REGULATOR_CSR_LPM] = {
		BCMPMU_REGULATOR_CSR_LPM, &bcm59xxx_csr_lpm_data, 0xFF, 0
	},
#ifdef CONFIG_MACH_RHEA_STONE_EDN2X
	/*EDN2x does not use IOSR*/
	[BCMPMU_REGULATOR_IOSR_NM] = {
		BCMPMU_REGULATOR_IOSR_NM, &bcm59xxx_iosr_nm_data, 0xAA, 0
	},
#else
	[BCMPMU_REGULATOR_IOSR_NM] = {
		BCMPMU_REGULATOR_IOSR_NM, &bcm59xxx_iosr_nm_data, 0x01, 0
	},
#endif /*CONFIG_MACH_RHEA_STONE_EDN2X*/
	[BCMPMU_REGULATOR_IOSR_NM2] = {
		BCMPMU_REGULATOR_IOSR_NM2, &bcm59xxx_iosr_nm2_data, 0xFF, 0
	},
	[BCMPMU_REGULATOR_IOSR_LPM] = {
		BCMPMU_REGULATOR_IOSR_LPM, &bcm59xxx_iosr_lpm_data, 0xFF, 0
	},
	[BCMPMU_REGULATOR_SDSR_NM] = {
		BCMPMU_REGULATOR_SDSR_NM, &bcm59xxx_sdsr_nm_data, 0x11, 0
	},
	[BCMPMU_REGULATOR_SDSR_NM2] = {
		BCMPMU_REGULATOR_SDSR_NM2, &bcm59xxx_sdsr_nm2_data, 0xFF, 0
	},
	[BCMPMU_REGULATOR_SDSR_LPM] = {
		BCMPMU_REGULATOR_SDSR_LPM, &bcm59xxx_sdsr_lpm_data, 0xFF, 0
	},
#ifdef CONFIG_MACH_RHEA_STONE_EDN2X
	[BCMPMU_REGULATOR_ASR_NM] = {
		BCMPMU_REGULATOR_ASR_NM, &bcm59xxx_asr_nm_data, 0x01, 0
	},
#else
	[BCMPMU_REGULATOR_ASR_NM] = {
		BCMPMU_REGULATOR_ASR_NM, &bcm59xxx_asr_nm_data, 0x11, 0
	},
#endif /*CONFIG_MACH_RHEA_STONE_EDN2X*/

	[BCMPMU_REGULATOR_ASR_NM2] = {
		BCMPMU_REGULATOR_ASR_NM2, &bcm59xxx_asr_nm2_data, 0xFF, 0
	},
	[BCMPMU_REGULATOR_ASR_LPM] = {
		BCMPMU_REGULATOR_ASR_LPM, &bcm59xxx_asr_lpm_data, 0xFF, 0
	},
};

static struct bcmpmu59xxx_rw_data register_init_data[] = {
/* mask 0x00 is invalid value for mask */
	{.addr = PMU_REG_HOSTCTRL1, .val = 0x00, .mask = 0x01},
	{.addr = PMU_REG_PONKEYCTRL1, .val = 0x1b, .mask = 0xFF},
#if defined(CONFIG_MACH_RHEA_STONE) || defined(CONFIG_MACH_RHEA_STONE_EDN2X)
	{.addr = PMU_REG_CMPCTRL4, .val = 0x3d, .mask = 0xFF},
	{.addr = PMU_REG_CMPCTRL5, .val = 0x79, .mask = 0xFF},
	{.addr = PMU_REG_CMPCTRL6, .val = 0x20, .mask = 0xFF},
#else
	{.addr = PMU_REG_CMPCTRL4, .val = 0x43, .mask = 0xFF},
	{.addr = PMU_REG_CMPCTRL5, .val = 0x7F, .mask = 0xFF},
	{.addr = PMU_REG_CMPCTRL6 .val = 0x3B, .mask = 0xFF},
#endif /* CONFIG_MACH_RHEA_STONE */
	{.addr = PMU_REG_CMPCTRL7, .val = 0xF8, .mask = 0xFF},
	{.addr = PMU_REG_CMPCTRL14, .val = 0x09, .mask = 0xFF},
	{.addr = PMU_REG_RTC_CORE, .val = 0x00, .mask = 0xF3},
	{.addr = PMU_REG_INT1MSK, .val = 0xFF, .mask = 0xFF},
	{.addr = PMU_REG_INT2MSK, .val = 0xFF, .mask = 0xFF},
	{.addr = PMU_REG_INT3MSK, .val = 0xFF, .mask = 0xFF},
	{.addr = PMU_REG_INT4MSK, .val = 0xFF, .mask = 0xFF},
	{.addr = PMU_REG_INT5MSK, .val = 0xFF, .mask = 0xFF},
	{.addr = PMU_REG_INT6MSK, .val = 0xFF, .mask = 0xFF},
	{.addr = PMU_REG_INT7MSK, .val = 0xFF, .mask = 0xFF},
	{.addr = PMU_REG_INT8MSK, .val = 0xFF, .mask = 0xFF},
	{.addr = PMU_REG_MBCCTRL1, .val = 0x3B, .mask = 0xFF},
	{.addr = PMU_REG_MBCCTRL3, .val = 0x04, .mask = 0x04},
	{.addr = PMU_REG_MBCCTRL9, .val = 0x05, .mask = 0x0F},
	/* USB_FC_OPTION needed to be 1, in order to charge from SDP */
	{.addr = PMU_REG_MBCCTRL16, .val = 0x30, .mask = 0xFF},
	/**
	 * Switchers initializations
	 * If you modifying SDSR/ASR/IOSR/CSR values, please make sure
	 * to update rheastone-edn20-pmu-59039.dtsi file also
	 */

	/*Init SDSR NM, NM2 and LPM voltages to 1.24V
	*/
	{.addr = PMU_REG_SDSRCTRL1, .val = 0x15, .mask = 0xFF},
	{.addr = PMU_REG_SDSRCTRL2, .val = 0x15, .mask = 0xFF},
	{.addr = PMU_REG_SDSRCTRL3, .val = 0x15, .mask = 0xFF},

	/*Init CSR LPM  to 0.88 V
	CSR NM2 to 1.22V
	*/
	{.addr = PMU_REG_CSRCTRL2, .val = 0x03, .mask = 0xFF},
	{.addr = PMU_REG_CSRCTRL3, .val = 0x14, .mask = 0xFF},

	/*PLLCTRL, Clear Bit 0 to disable PLL when PC2:PC1 = 0b00*/
	{.addr = PMU_REG_PLLPMCTRL, .val = 0x0E, .mask = 0x0F},
	/*CMPCTRL13, Set bits 4, 1 for BSI Sync. Mode */
	{.addr = PMU_REG_CMPCTRL13, .val = 0x13, .mask = 0xFF},
	/*CMPCTRL12, Set bits 4, 1 for NTC Sync. Mode*/
	{.addr = PMU_REG_CMPCTRL14, .val = 0x13, .mask = 0xFF},

#ifdef CONFIG_MACH_RHEA_STONE_EDN2X
	{.addr = PMU_REG_ASRCTRL2, .val = 0x1A, .mask = 0xFF},
#else
	/*Init ASR LPM to 2.9V - for Rhea EDN10 & EDN00 and 1.8V for EDN2x
	*/
	{.addr = PMU_REG_ASRCTRL2, .val = 0x1F, .mask = 0xFF},
	/*Init IOSR NM2 and LPM voltages to 1.8V
	*/
	{.addr = PMU_REG_IOSRCTRL2, .val = 0x1A, .mask = 0xFF},
	{.addr = PMU_REG_IOSRCTRL3, .val = 0x1A, .mask = 0xFF},
#endif /*CONFIG_MACH_RHEA_STONE_EDN2X*/

	/*FGOPMODCTRL, Set bits 4, 1 for FG Sync. Mode*/
	{.addr = PMU_REG_FGOPMODCTRL, .val = 0x11, .mask = 0xFF},

};

struct bcmpmu59xxx_pok_pdata pok_pdata = {
	.hard_reset_en = -1,
	.restart_en = -1,
	.pok_hold_deb = -1,
	.pok_shtdwn_dly = -1,
	.pok_restart_dly = -1,
	.pok_restart_deb = -1,
	.pok_lock = 1, /*Keep ponkey locked by default*/
	.pok_turn_on_deb = -1,
};

struct bcmpmu59xxx_audio_pdata audio_pdata = {
	.ihf_autoseq_dis = 100,
};

struct bcmpmu59xxx_regulator_pdata rgltr_pdata = {
	.bcmpmu_rgltr = bcm59xxx_regulators,
	.num_of_rgltr = ARRAY_SIZE(bcm59xxx_regulators),
};

/* The subdevices of the bcmpmu59xxx */
static struct mfd_cell pmu59xxx_devs[] = {
	{
		.name = "bcmpmu59xxx-regulator",
		.id = -1,
		.platform_data = &rgltr_pdata,
		.pdata_size = sizeof(rgltr_pdata),
	},
	{
		.name = "bcmpmu59xxx-ponkey",
		.id = -1,
		.platform_data = &pok_pdata,
		.pdata_size = sizeof(pok_pdata),
	},
	{
		.name = "bcmpmu59xxx_rtc",
		.id = -1,
	},
	{
		.name = "bcmpmu_audio",
		.id = -1,
		.platform_data = &audio_pdata,
		.pdata_size = sizeof(audio_pdata),
	},
	{
		.name = "bcmpmu_accy",
		.id = -1,
	},
	{
		.name = "bcmpmu_otg_xceiv",
		.id = -1,
	},
};

static struct i2c_board_info pmu_i2c_companion_info[] = {
	{
	I2C_BOARD_INFO("bcmpmu_map1", PMU_DEVICE_I2C_ADDR1),
	},
};

static struct bcmpmu59xxx_platform_data bcmpmu_i2c_pdata = {
	.i2c_pdata = { ADD_I2C_SLAVE_SPEED(BSC_BUS_SPEED_400K), },
	.init = bcmpmu_init_platform_hw,
	.exit = bcmpmu_exit_platform_hw,
	.companion = BCMPMU_DUMMY_CLIENTS,
	.i2c_companion_info = pmu_i2c_companion_info,
	.i2c_adapter_id = PMU_DEVICE_I2C_BUSNO,
	.i2c_pagesize = 256,
	.init_data = register_init_data,
	.init_max = ARRAY_SIZE(register_init_data),
	.bc = BCMPMU_BC_PMU_BC12,
#ifdef CONFIG_CHARGER_BCMPMU_SPA
	.piggyback_chrg = 1,
	.piggyback_chrg_name = "bcm59039_charger",
#endif

};

static struct i2c_board_info __initdata bcmpmu_i2c_info[] = {
	{
		I2C_BOARD_INFO("bcmpmu59xxx_i2c", PMU_DEVICE_I2C_ADDR),
		.platform_data = &bcmpmu_i2c_pdata,
		.irq = gpio_to_irq(PMU_DEVICE_INT_GPIO),
	},
};

int bcmpmu_get_pmu_mfd_cell(struct mfd_cell **pmu_cell)
{
	*pmu_cell  = pmu59xxx_devs;
	return ARRAY_SIZE(pmu59xxx_devs);
}
EXPORT_SYMBOL(bcmpmu_get_pmu_mfd_cell);

const u8 *bcmpmu_get_sr_vlt_table(int sr, u32 freq_inx,
						u32 silicon_type)
{
	pr_info("%s:sr = %i, freq_inx = %d,"
			"silicon_type = %d\n", __func__,
			sr, freq_inx, silicon_type);

	BUG_ON(freq_inx > A9_FREQ_850_MHZ);

#ifdef CONFIG_KONA_AVS
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

static int bcmpmu_init_platform_hw(struct bcmpmu59xxx *bcmpmu)
{
	pr_info("REG: pmu_exit_platform_hw called\n");
	return 0;
}

static int bcmpmu_exit_platform_hw(struct bcmpmu59xxx *bcmpmu)
{
	pr_info("REG: pmu_exit_platform_hw called\n");
	return 0;
}

int board_bcm59xx_init(void)
{
	int             ret = 0;
	int             irq;

	ret = gpio_request(PMU_DEVICE_INT_GPIO, "bcmpmu59xxx-irq");
	if (ret < 0) {
		printk(KERN_ERR "<%s> failed at gpio_request\n", __func__);
		goto exit;
	}
	ret = gpio_direction_input(PMU_DEVICE_INT_GPIO);
	if (ret < 0) {

		printk(KERN_ERR "%s filed at gpio_direction_input.\n",
				__func__);
		goto exit;
	}
	irq = gpio_to_irq(PMU_DEVICE_INT_GPIO);
	bcmpmu_i2c_pdata.irq = irq;
	ret  = i2c_register_board_info(PMU_DEVICE_I2C_BUSNO,
			bcmpmu_i2c_info, ARRAY_SIZE(bcmpmu_i2c_info));
	return 0;
exit:
	return ret;
}

__init int board_pmu_init(void)
{
	return board_bcm59xx_init();
}
arch_initcall(board_pmu_init);
