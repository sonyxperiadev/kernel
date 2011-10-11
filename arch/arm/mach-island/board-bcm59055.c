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

#define PMU_DEVICE_I2C_ADDR	0x08
#define PMU_DEVICE_I2C_ADDR1	0x0C
#define PMU_DEVICE_INT_GPIO	10

static struct bcmpmu_rw_data register_init_data[] = {
	{.map=0, .addr=0x0c, .val=0x1b, .mask=0xFF},
	{.map=0, .addr=0x40, .val=0xFF, .mask=0xFF},
	{.map=0, .addr=0x41, .val=0xFF, .mask=0xFF},
	{.map=0, .addr=0x42, .val=0xFF, .mask=0xFF},
	{.map=0, .addr=0x43, .val=0xFF, .mask=0xFF},
	{.map=0, .addr=0x44, .val=0xFF, .mask=0xFF},
	{.map=0, .addr=0x45, .val=0xFF, .mask=0xFF},
	{.map=0, .addr=0x46, .val=0xFF, .mask=0xFF},
	{.map=0, .addr=0x47, .val=0xFF, .mask=0xFF},
	{.map=0, .addr=0x48, .val=0xFF, .mask=0xFF},
	{.map=0, .addr=0x49, .val=0xFF, .mask=0xFF},
	{.map=0, .addr=0x4a, .val=0xFF, .mask=0xFF},
	{.map=0, .addr=0x4b, .val=0xFF, .mask=0xFF},
	{.map=0, .addr=0x4c, .val=0xFF, .mask=0xFF},
	{.map=0, .addr=0x4d, .val=0xFF, .mask=0xFF},
	{.map=0, .addr=0x50, .val=0x6B, .mask=0xFF}, 
	{.map=0, .addr=0x51, .val=0x03, .mask=0xFF},
	{.map=0, .addr=0x52, .val=0x08, .mask=0xFF},
	{.map=0, .addr=0x53, .val=0x00, .mask=0xFF},
	{.map=0, .addr=0x54, .val=0x03, .mask=0xFF},
	{.map=0, .addr=0x55, .val=0x08, .mask=0xFF},
	{.map=0, .addr=0x56, .val=0x08, .mask=0xFF},
	{.map=0, .addr=0x57, .val=0x07, .mask=0xFF},
	{.map=0, .addr=0x58, .val=0x01, .mask=0xFF},
	{.map=0, .addr=0x59, .val=0x00, .mask=0xFF},
	{.map=0, .addr=0x5a, .val=0x07, .mask=0xFF},
	{.map=0, .addr=0x69, .val=0x10, .mask=0xFF},
/* OTG registers */
	{.map=0, .addr=0x71, .val=0x09, .mask=0xFF},
	{.map=0, .addr=0x77, .val=0xD4, .mask=0xFF},
	{.map=0, .addr=0x78, .val=0x98, .mask=0xFF},
	{.map=0, .addr=0x79, .val=0xF0, .mask=0xFF},
	{.map=0, .addr=0x7A, .val=0x60, .mask=0xFF},
	{.map=0, .addr=0x7B, .val=0xC3, .mask=0xFF},
	{.map=0, .addr=0x7C, .val=0xA7, .mask=0xFF},
	{.map=0, .addr=0x7D, .val=0x08, .mask=0xFF},
};

static struct bcmpmu_temp_map batt_temp_map[] = {
/* This table is hardware dependent and need to get from platform team */
/*	adc		temp*/
	{932,		233},/* -40 C */
	{900,		238},/* -35 C */
	{860,		243},/* -30 C */
	{816,		248},/* -25 C */
	{760,		253},/* -20 C */
	{704,		258},/* -15 C */
	{636,		263},/* -10 C */
	{568,		268},/* -5 C */
	{500,		273},/* 0 C */
	{440,		278},/* 5 C */
	{376,		283},/* 10 C */
	{324,		288},/* 15 C */
	{272,		293},/* 20 C */
	{228,		298},/* 25 C */
	{192,		303},/* 30 C */
	{160,		308},/* 35 C */
	{132,		313},/* 40 C */
	{112,		318},/* 45 C */
	{92,		323},/* 50 C */
	{76,		328},/* 55 C */
	{64,		333},/* 60 C */
	{52,		338},/* 65 C */
	{44,		343},/* 70 C */
	{36,		348},/* 75 C */
	{32,		353},/* 80 C */
	{28,		358},/* 85 C */
	{24,		363},/* 90 C */
	{20,		368},/* 95 C */
	{16,		373},/* 100 C */
};

struct regulator_consumer_supply rf_supply[] = {
	{ .supply = "rfldo_uc"},
};
static struct regulator_init_data bcm59055_rfldo_data =  {
	.constraints = {
		.name = "rfldo",
		.min_uV = 1300000,
		.max_uV = 3300000,
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
static struct regulator_init_data bcm59055_camldo_data = {
	.constraints = {
		.name = "camldo",
		.min_uV = 1300000,
		.max_uV = 3300000,
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
static struct regulator_init_data bcm59055_hv1ldo_data = {
	.constraints = {
		.name = "hv1ldo",
		.min_uV = 1300000,
		.max_uV = 3300000,
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
static struct regulator_init_data bcm59055_hv2ldo_data = {
	.constraints = {
		.name = "hv2ldo",
		.min_uV = 1300000,
		.max_uV = 3300000,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_VOLTAGE,
		.always_on = 1,
		.valid_modes_mask = REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY | REGULATOR_MODE_IDLE
	},
	.num_consumer_supplies = ARRAY_SIZE(hv2_supply),
	.consumer_supplies = hv2_supply,
};

struct regulator_consumer_supply hv3_supply[] = {
	{.supply = "hv3ldo_uc"},
};
static struct regulator_init_data bcm59055_hv3ldo_data = {
	.constraints = {
		.name = "hv3ldo",
		.min_uV = 1300000,
		.max_uV = 3300000,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_VOLTAGE,
		.always_on = 1,
		.valid_modes_mask = REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY | REGULATOR_MODE_IDLE
	},
	.num_consumer_supplies = ARRAY_SIZE(hv3_supply),
	.consumer_supplies = hv3_supply,
};

struct regulator_consumer_supply hv4_supply[] = {
	{.supply = "hv4ldo_uc"},
};
static struct regulator_init_data bcm59055_hv4ldo_data = {
	.constraints = {
		.name = "hv4ldo",
		.min_uV = 1300000,
		.max_uV = 3300000,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_VOLTAGE,
		.always_on = 1,
		.valid_modes_mask = REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY | REGULATOR_MODE_IDLE
	},
	.num_consumer_supplies = ARRAY_SIZE(hv4_supply),
	.consumer_supplies = hv4_supply,
};

struct regulator_consumer_supply hv5_supply[] = {
	{.supply = "hv5ldo_uc"},
};
static struct regulator_init_data bcm59055_hv5ldo_data = {
	.constraints = {
		.name = "hv5ldo",
		.min_uV = 1300000,
		.max_uV = 3300000,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_VOLTAGE,
		.always_on = 1,
		.valid_modes_mask = REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY | REGULATOR_MODE_IDLE
	},
	.num_consumer_supplies = ARRAY_SIZE(hv5_supply),
	.consumer_supplies = hv5_supply,
};

struct regulator_consumer_supply hv6_supply[] = {
	{.supply = "hv6ldo_uc"},
};
static struct regulator_init_data bcm59055_hv6ldo_data = {
	.constraints = {
		.name = "hv6ldo",
		.min_uV = 1300000,
		.max_uV = 3300000,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_VOLTAGE,
		.always_on = 1,
		.valid_modes_mask = REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY | REGULATOR_MODE_IDLE
	},
	.num_consumer_supplies = ARRAY_SIZE(hv6_supply),
	.consumer_supplies = hv6_supply,
};

struct regulator_consumer_supply hv7_supply[] = {
	{.supply = "hv7ldo_uc"},
};
static struct regulator_init_data bcm59055_hv7ldo_data = {
	.constraints = {
		.name = "hv7ldo",
		.min_uV = 1300000,
		.max_uV = 3300000,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_VOLTAGE,
		.always_on = 1,
		.valid_modes_mask = REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY | REGULATOR_MODE_IDLE
	},
	.num_consumer_supplies = ARRAY_SIZE(hv7_supply),
	.consumer_supplies = hv7_supply,
};

struct regulator_consumer_supply sim_supply[] = {
	{.supply = "sim_vcc"},
};
static struct regulator_init_data bcm59055_simldo_data = {
	.constraints = {
		.name = "simldo",
		.min_uV = 1300000,
		.max_uV = 3300000,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_VOLTAGE,
		.always_on = 1,
		.valid_modes_mask = REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY | REGULATOR_MODE_IDLE
	},
	.num_consumer_supplies = ARRAY_SIZE(sim_supply),
	.consumer_supplies = sim_supply,
};



struct regulator_consumer_supply sim2_supply[] = {
	{.supply = "sim2_vcc"},
};
static struct regulator_init_data bcm59055_sim2ldo_data = {
	.constraints = {
		.name = "sim2ldo",
		.min_uV = 1300000,
		.max_uV = 3300000,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_VOLTAGE,
		.always_on = 0,
		.valid_modes_mask = REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY | REGULATOR_MODE_IDLE
	},
	.num_consumer_supplies = ARRAY_SIZE(sim2_supply),
	.consumer_supplies = sim2_supply,
};



struct regulator_consumer_supply csr_supply[] = {
	{.supply = "csr_uc"},
};
static struct regulator_init_data bcm59055_csr_data = {
	.constraints = {
		.name = "csr",
		.min_uV = 700000,
		.max_uV = 1800000,
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
static struct regulator_init_data bcm59055_iosr_data = {
	.constraints = {
		.name = "iosr",
		.min_uV = 700000,
		.max_uV = 1800000,
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
static struct regulator_init_data bcm59055_sdsr_data = {
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

struct bcmpmu_regulator_init_data bcm59055_regulators[BCMPMU_REGULATOR_MAX] = {
	{BCMPMU_REGULATOR_RFLDO, &bcm59055_rfldo_data},
	{BCMPMU_REGULATOR_CAMLDO, &bcm59055_camldo_data},
	{BCMPMU_REGULATOR_HV1LDO, &bcm59055_hv1ldo_data},
	{BCMPMU_REGULATOR_HV2LDO, &bcm59055_hv2ldo_data},
	{BCMPMU_REGULATOR_HV3LDO, &bcm59055_hv3ldo_data},
	{BCMPMU_REGULATOR_HV4LDO, &bcm59055_hv4ldo_data},
	{BCMPMU_REGULATOR_HV5LDO, &bcm59055_hv5ldo_data},
	{BCMPMU_REGULATOR_HV6LDO, &bcm59055_hv6ldo_data},
	{BCMPMU_REGULATOR_HV7LDO, &bcm59055_hv7ldo_data},
	{BCMPMU_REGULATOR_SIMLDO, &bcm59055_simldo_data},
	{BCMPMU_REGULATOR_SIM2LDO, &bcm59055_sim2ldo_data},
	{BCMPMU_REGULATOR_CSR, &bcm59055_csr_data},
	{BCMPMU_REGULATOR_IOSR, &bcm59055_iosr_data},
	{BCMPMU_REGULATOR_SDSR, &bcm59055_sdsr_data}
};

static struct platform_device bcmpmu_audio_device = {
	.name 			= "bcmpmu_audio",
	.id			= -1,
	.dev.platform_data 	= NULL,
};

static struct platform_device bcmpmu_em_device = {
	.name 			= "bcmpmu_em",
	.id			= -1,
	.dev.platform_data 	= NULL,
};

static struct platform_device bcmpmu_otg_xceiv_device = {
	.name 			= "bcmpmu_otg_xceiv",
	.id			= -1,
	.dev.platform_data 	= NULL,
};

static struct platform_device *bcmpmu_client_devices[] = {
	&bcmpmu_audio_device,
	&bcmpmu_em_device,
	&bcmpmu_otg_xceiv_device,
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

static struct i2c_board_info pmu_info_map1 = {
	I2C_BOARD_INFO("bcmpmu_map1", PMU_DEVICE_I2C_ADDR1),
};

static struct bcmpmu_adc_setting adc_setting = {
	.tx_rx_sel_addr = 0,
	.tx_delay = 0,
	.rx_delay = 0,
};

static struct bcmpmu_charge_zone chrg_zone[] = {
	{.tl = 253, .th = 333, .v = 3000, .fc = 10, .qc = 100},/* Zone QC */
	{.tl = 253, .th = 272, .v = 4100, .fc = 50, .qc = 0},/* Zone LL */
	{.tl = 273, .th = 282, .v = 4200, .fc = 50, .qc = 0},/* Zone L */
	{.tl = 283, .th = 318, .v = 4200, .fc = 100,.qc = 0},/* Zone N */
	{.tl = 319, .th = 323, .v = 4200, .fc = 50, .qc = 0},/* Zone H */
	{.tl = 324, .th = 333, .v = 4100, .fc = 50, .qc = 0},/* Zone HH */
	{.tl = 253, .th = 333, .v = 0,    .fc = 0,  .qc = 0},/* Zone OUT */
};

static struct bcmpmu_platform_data __initdata bcmpmu_plat_data = {
	.init = bcmpmu_init_platform_hw,
	.exit = bcmpmu_exit_platform_hw,
	.i2c_board_info_map1 = &pmu_info_map1,
	.i2c_adapter_id = 2,
	.i2c_pagesize = 256,
	.init_data = &register_init_data[0],
	.init_max = ARRAY_SIZE(register_init_data),
	.batt_temp_map = &batt_temp_map[0],
	.batt_temp_map_len = ARRAY_SIZE(batt_temp_map),
	.adc_setting = &adc_setting,
	.regulator_init_data = bcm59055_regulators,
	.fg_smpl_rate = 2083,
	.fg_slp_rate = 32000,
	.fg_slp_curr_ua = 1000,
	.chrg_1c_rate = 1000,
	.chrg_zone_map = &chrg_zone[0],
	.fg_capacity_full = 1000*3600,
	.support_fg = 1,
};

static struct i2c_board_info __initdata pmu_info[] =
{
	{
		I2C_BOARD_INFO("bcmpmu", PMU_DEVICE_I2C_ADDR),
		.platform_data  = &bcmpmu_plat_data,
	},
};


void __init board_pmu_init(void)
{
	int ret;
	int irq;
	ret = gpio_request(PMU_DEVICE_INT_GPIO, "bcmpmu-irq");
	if (ret < 0)
		printk(KERN_ERR "%s filed at gpio_request.\n", __FUNCTION__);

	ret = gpio_direction_input(PMU_DEVICE_INT_GPIO);
	if (ret < 0)
		printk(KERN_ERR "%s filed at gpio_direction_input.\n", __FUNCTION__);
	irq = gpio_to_irq(PMU_DEVICE_INT_GPIO);
	bcmpmu_plat_data.irq = irq;

	i2c_register_board_info(2,		// This is i2c adapter number.
				pmu_info,
				ARRAY_SIZE(pmu_info));
}

