/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2010  Broadcom Corporation                                                        */
/*                                                                                              */
/*     Unless you and Broadcom execute a separate written software license agreement governing  */
/*     use of this software, this software is licensed to you under the terms of the GNU        */
/*     General Public License version 2 (the GPL), available at                                 */
/*                                                                                              */
/*          http://www.broadcom.com/licenses/GPLv2.php                                          */
/*                                                                                              */
/*     with the following added to such license:                                                */
/*                                                                                              */
/*     As a special exception, the copyright holders of this software give you permission to    */
/*     link this software with independent modules, and to copy and distribute the resulting    */
/*     executable under terms of your choice, provided that you also meet, for each linked      */
/*     independent module, the terms and conditions of the license of that module.              */
/*     An independent module is a module which is not derived from this software.  The special  */
/*     exception does not apply to any modifications of the software.                           */
/*                                                                                              */
/*     Notwithstanding the above, under no circumstances may you combine this software in any   */
/*     way with any other Broadcom software provided under a license other than the GPL,        */
/*     without Broadcom's express prior written consent.                                        */
/*                                                                                              */
/************************************************************************************************/
#include <linux/version.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/sysdev.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <asm/mach/arch.h>
#include <asm/mach-types.h>
#include <asm/gpio.h>
#include <mach/hardware.h>
#include <linux/io.h>
#include <linux/uaccess.h>
#include <linux/i2c.h>
#include <linux/mfd/bcmpmu.h>

#define PMU_DEVICE_INT_GPIO	29
#define PMU_INT_IRQ_NUM			1 /* TODO - find out from Mehran */
#define PMU_BASE_ADDR			KONA_PMU_PMU_VA /* TODO - find out from Mehran */

static const struct bcmpmu_rw_data register_init_data[] = {
	{.map=0, .addr=0x284, .val=0x0, .mask=0xFFFF},
	{.map=0, .addr=0x288, .val=0x0, .mask=0xFF},
	{.map=0, .addr=0x28c, .val=0x0, .mask=0xFF},
	{.map=0, .addr=0x290, .val=0x0, .mask=0xFFFF},
	{.map=0, .addr=0x294, .val=0x5800, .mask=0xFFFF},
	{.map=0, .addr=0x298, .val=0x5800, .mask=0xFFFF},
	{.map=0, .addr=0x29c, .val=0x0, .mask=0xFF},
};

static const struct bcmpmu_temp_map batt_temp_map[] = {
/* This table is hardware dependent and need to get from platform team */
/*	adc		temp*/
	{0x3FF,		233},/* -40 C */
	{0x3C0,		238},/* -35 C */
	{0x380,		243},/* -30 C */
	{0x340,		248},/* -25 C */
	{0x300,		253},/* -20 C */
	{0x2C0,		258},/* -15 C */
	{0x280,		263},/* -10 C */
	{0x240,		268},/* -5 C */
	{0x200,		273},/* 0 C */
	{0x1C0,		278},/* 5 C */
	{0x180,		283},/* 10 C */
	{0x140,		288},/* 15 C */
	{0x100,		293},/* 20 C */
	{0xF0,		298},/* 25 C */
	{0xE0,		303},/* 30 C */
	{0xD0,		308},/* 35 C */
	{0xC0,		313},/* 40 C */
	{0xB0,		318},/* 45 C */
	{0xA0,		323},/* 50 C */
	{0x90,		328},/* 55 C */
	{0x86,		333},/* 60 C */
	{0x70,		338},/* 65 C */
	{0x60,		343},/* 70 C */
	{0x50,		348},/* 75 C */
	{0x40,		353},/* 80 C */
	{0x30,		358},/* 85 C */
	{0x20,		363},/* 90 C */
	{0x10,		368},/* 95 C */
	{0x00,		373},/* 100 C */
};

struct regulator_consumer_supply rf_supply[] = {
	{ .supply = "rfldo_uc"},
};
static struct regulator_init_data samoa_rfldo_data =  {
	.constraints = {
		.name = "rfldo",
		.min_uV = 2500000,
		.max_uV = 2700000,
		.valid_ops_mask = REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_VOLTAGE,
		.always_on = 1,
		.valid_modes_mask = REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY
	},
	.num_consumer_supplies = ARRAY_SIZE(rf_supply),
	.consumer_supplies = rf_supply,
};

struct regulator_consumer_supply cam_supply[] = {
	{.supply = "camldo_uc"},
};
static struct regulator_init_data samoa_camldo_data = {
	.constraints = {
		.name = "camldo",
		.min_uV = 2600000,
		.max_uV = 2800000,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS |REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_VOLTAGE,
		.always_on = 1,
		.valid_modes_mask = REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY | REGULATOR_MODE_IDLE
	},
	.num_consumer_supplies = ARRAY_SIZE(cam_supply),
	.consumer_supplies = cam_supply,
};


struct regulator_consumer_supply hv1_supply[] = {
	{.supply = "hv1ldo_uc"},
};
static struct regulator_init_data samoa_hv1ldo_data = {
	.constraints = {
		.name = "hv1ldo",
		.min_uV = 1800000,
		.max_uV = 3050000,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS |REGULATOR_CHANGE_MODE |  REGULATOR_CHANGE_VOLTAGE,
		.always_on = 1,
		.valid_modes_mask = REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY | REGULATOR_MODE_IDLE
	},
	.num_consumer_supplies = ARRAY_SIZE(hv1_supply),
	.consumer_supplies = hv1_supply,
};

struct regulator_consumer_supply hv2_supply[] = {
	{.supply = "hv2ldo_uc"},
};
static struct regulator_init_data samoa_hv2ldo_data = {
	.constraints = {
		.name = "hv2ldo",
		.min_uV = 1800000,
		.max_uV = 3050000,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_VOLTAGE,
		.always_on = 1,
		.valid_modes_mask = REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY | REGULATOR_MODE_IDLE
	},
	.num_consumer_supplies = ARRAY_SIZE(hv2_supply),
	.consumer_supplies = hv2_supply,
};

struct regulator_consumer_supply usb_supply[] = {
	{.supply = "usbldo_uc"},
};
static struct regulator_init_data samoa_usbldo_data = {
	.constraints = {
		.name = "usbldo",
		.min_uV = 2900000,
		.max_uV = 3600000,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_VOLTAGE,
		.always_on = 1,
		.valid_modes_mask = REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY | REGULATOR_MODE_IDLE
	},
	.num_consumer_supplies = ARRAY_SIZE(usb_supply),
	.consumer_supplies = usb_supply,
};

struct regulator_consumer_supply bcd_supply[] = {
	{.supply = "bcdldo_uc"},
};
static struct regulator_init_data samoa_bcdldo_data = {
	.constraints = {
		.name = "bcdldo",
		.min_uV = 2900000,
		.max_uV = 3600000,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_VOLTAGE,
		.always_on = 1,
		.valid_modes_mask = REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY | REGULATOR_MODE_IDLE
	},
	.num_consumer_supplies = ARRAY_SIZE(bcd_supply),
	.consumer_supplies = bcd_supply,
};

struct regulator_consumer_supply sim1_supply[] = {
	{.supply = "sim1ldo_uc"},
};
static struct regulator_init_data samoa_sim1ldo_data = {
	.constraints = {
		.name = "sim1ldo",
		.min_uV = 1800000,
		.max_uV = 3050000,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_VOLTAGE,
		.always_on = 1,
		.valid_modes_mask = REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY | REGULATOR_MODE_IDLE
	},
	.num_consumer_supplies = ARRAY_SIZE(sim1_supply),
	.consumer_supplies = sim1_supply,
};

struct regulator_consumer_supply sim2_supply[] = {
	{.supply = "sim2ldo_uc"},
};
static struct regulator_init_data samoa_sim2ldo_data = {
	.constraints = {
		.name = "sim2ldo",
		.min_uV = 1800000,
		.max_uV = 3050000,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_VOLTAGE,
		.always_on = 1,
		.valid_modes_mask = REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY | REGULATOR_MODE_IDLE
	},
	.num_consumer_supplies = ARRAY_SIZE(sim2_supply),
	.consumer_supplies = sim2_supply,
};

struct regulator_consumer_supply dvs1_supply[] = {
	{.supply = "dvs1ldo_uc"},
};
static struct regulator_init_data samoa_dvs1ldo_data = {
	.constraints = {
		.name = "dvs1ldo",
		.min_uV = 700000,
		.max_uV = 1280000,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_VOLTAGE,
		.always_on = 1,
		.valid_modes_mask = REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY | REGULATOR_MODE_IDLE
	},
	.num_consumer_supplies = ARRAY_SIZE(dvs1_supply),
	.consumer_supplies = dvs1_supply,
};

struct regulator_consumer_supply dvs2_supply[] = {
	{.supply = "dvs2ldo_uc"},
};
static struct regulator_init_data samoa_dvs2ldo_data = {
	.constraints = {
		.name = "dvs2ldo",
		.min_uV = 700000,
		.max_uV = 1280000,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_VOLTAGE,
		.always_on = 1,
		.valid_modes_mask = REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY | REGULATOR_MODE_IDLE
	},
	.num_consumer_supplies = ARRAY_SIZE(dvs2_supply),
	.consumer_supplies = dvs2_supply,
};

struct regulator_consumer_supply csr_supply[] = {
	{.supply = "csr_uc"},
};
static struct regulator_init_data samoa_csr_data = {
	.constraints = {
		.name = "csr",
		.min_uV = 1155000,
		.max_uV = 1365000,
		.valid_ops_mask = REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_VOLTAGE,
		.always_on = 1,
		.valid_modes_mask = REGULATOR_MODE_NORMAL | REGULATOR_MODE_FAST
	},
	.num_consumer_supplies = ARRAY_SIZE(csr_supply),
	.consumer_supplies = csr_supply,
};

struct regulator_consumer_supply iosr_supply[] = {
	{.supply = "iosr_uc"},
};
static struct regulator_init_data samoa_iosr_data = {
	.constraints = {
		.name = "iosr",
		.min_uV = 1745000,
		.max_uV = 1885000,
		.valid_ops_mask = REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_VOLTAGE,
		.always_on = 1,
		.valid_modes_mask = REGULATOR_MODE_NORMAL | REGULATOR_MODE_FAST
	},
	.num_consumer_supplies = ARRAY_SIZE(iosr_supply),
	.consumer_supplies = iosr_supply,
};

struct regulator_consumer_supply sdsr_supply[] = {
	{.supply = "sdsr_uc"},
};
static struct regulator_init_data samoa_sdsr_data = {
	.constraints = {
		.name = "sdsr",
		.min_uV = 700000,
		.max_uV = 1800000,
		.valid_ops_mask = REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_VOLTAGE,
		.always_on = 1,
		.valid_modes_mask = REGULATOR_MODE_NORMAL | REGULATOR_MODE_FAST
		},
	.num_consumer_supplies = ARRAY_SIZE(sdsr_supply),
	.consumer_supplies = sdsr_supply,
};

struct bcmpmu_regulator_init_data samoa_regulators[BCMPMU_REGULATOR_MAX] = {
	{BCMPMU_REGULATOR_RFLDO, &samoa_rfldo_data},
	{BCMPMU_REGULATOR_CAMLDO, &samoa_camldo_data},
	{BCMPMU_REGULATOR_HV1LDO, &samoa_hv1ldo_data},
	{BCMPMU_REGULATOR_HV2LDO, &samoa_hv2ldo_data},
	{BCMPMU_REGULATOR_USBLDO, &samoa_usbldo_data}, 
	{BCMPMU_REGULATOR_BCDLDO, &samoa_bcdldo_data},
	{BCMPMU_REGULATOR_DVS1LDO, &samoa_dvs1ldo_data},
	{BCMPMU_REGULATOR_DVS2LDO, &samoa_dvs2ldo_data},
	{BCMPMU_REGULATOR_SIMLDO, &samoa_sim1ldo_data},
	{BCMPMU_REGULATOR_SIM2LDO, &samoa_sim2ldo_data},
	{BCMPMU_REGULATOR_CSR, &samoa_csr_data},
	{BCMPMU_REGULATOR_IOSR, &samoa_iosr_data},
/*
	{BCMPMU_REGULATOR_SDSR, &samoa_sdsr_data}
*/
};


static struct platform_device bcmpmu_em_device = {
	.name 			= "bcmpmu_em",
	.id			= -1,
	.dev.platform_data 	= NULL,
};

static struct platform_device *bcmpmu_client_devices[] = {
	&bcmpmu_em_device,
};

static int __init bcmpmu_init_platform_hw(struct bcmpmu *bcmpmu)
{
	int i;
	printk(KERN_INFO "%s: called.\n", __func__);
	bcmpmu_reg_dev_init(bcmpmu);

	for (i = 0; i <ARRAY_SIZE(bcmpmu_client_devices); i++)
		bcmpmu_client_devices[i]->dev.platform_data = bcmpmu;
	platform_add_devices(bcmpmu_client_devices, ARRAY_SIZE(bcmpmu_client_devices));
	return 0;
}

static int __init bcmpmu_exit_platform_hw(struct bcmpmu *bcmpmu)
{
	printk("REG: pmu_init_platform_hw called \n");
	bcmpmu_reg_dev_exit(bcmpmu);
	return 0;
}

static const struct bcmpmu_adc_setting adc_setting = {
	.tx_rx_sel_addr = 0,
	.tx_delay = 0,
	.rx_delay = 0,
};

static struct bcmpmu_platform_data __initdata bcmpmu_plat_data = {
	.init = bcmpmu_init_platform_hw,
	.exit = bcmpmu_exit_platform_hw,
	.init_data = &register_init_data,
	.init_max = ARRAY_SIZE(register_init_data),
	.batt_temp_map = &batt_temp_map,
	.batt_temp_map_len = ARRAY_SIZE(batt_temp_map),
	.adc_setting = &adc_setting,
	.regulator_init_data = &samoa_regulators,
};

static struct platform_device bcmpmu_samoa_device = {
	.name 			= "bcmpmu_samoa",
	.id			= -1,
	.dev.platform_data 	= NULL,
};

void __init board_pmu_init(void)
{
	bcmpmu_plat_data.irq = PMU_INT_IRQ_NUM;
	bcmpmu_plat_data.baseaddr = PMU_BASE_ADDR;

	bcmpmu_samoa_device.dev.platform_data = &bcmpmu_plat_data;
	platform_device_register(&bcmpmu_samoa_device);
}
