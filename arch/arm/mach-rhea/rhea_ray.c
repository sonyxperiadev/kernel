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
#include <linux/kernel_stat.h>
#include <asm/mach/arch.h>
#include <asm/mach-types.h>
#include <mach/hardware.h>
#include <linux/i2c.h>
#include <linux/i2c-kona.h>
#include <linux/gpio_keys.h>
#include <linux/input.h>
#include <mach/kona.h>
#include <mach/rhea.h>
#include <asm/mach/map.h>
#include <linux/mfd/bcm590xx/core.h>
#include <linux/mfd/bcm590xx/pmic.h>
#include <linux/mfd/bcm590xx/bcm59055_A0.h>
#include <linux/clk.h>
#include "common.h"

#define PMU_DEVICE_I2C_ADDR_0   0x08
#define PMU_DEVICE_I2C_ADDR_1   0x0C
#define PMU_IRQ_PIN           10

static int __init bcm590xx_init_platform_hw(struct bcm590xx *bcm590xx)
{
	printk("REG: pmu_init_platform_hw called \n") ;
	bcm59055_reg_init_dev_init(bcm590xx)  ;

	return 0 ;
}

/* wall charging and vbus are wired together on FF board
     we monitor USB activity to make sure it is not USB cable that is inserted
 */
static int can_start_charging(void* data)
{
#define INTERVAL (HZ/10)
	int cpu, usb_otg_int[4], i;
	for_each_present_cpu(cpu)
		usb_otg_int[cpu] =  kstat_irqs_cpu(
		BCM_INT_ID_USB_HSOTG, cpu);

	for (i=0; i<10; i++) {
		schedule_timeout_interruptible(INTERVAL);
		for_each_present_cpu(cpu)
			if (usb_otg_int[cpu]!= kstat_irqs_cpu(
				BCM_INT_ID_USB_HSOTG, cpu))
				return 0;
	}
	return 1;
}

static struct bcm590xx_battery_pdata bcm590xx_battery_plat_data = {
	.can_start_charging = can_start_charging,
};

static struct bcm590xx_platform_data bcm590xx_plat_data = {
	.init = bcm590xx_init_platform_hw,
	.slave = 0 ,
	.battery_pdata = &bcm590xx_battery_plat_data,
};

static struct bcm590xx_platform_data bcm590xx_plat_data_sl1 = {
	.slave = 1 ,
};

static struct i2c_board_info __initdata pmu_info[] =
{
	[0] = {
		I2C_BOARD_INFO("bcm590xx", PMU_DEVICE_I2C_ADDR_1 ),
		.irq = gpio_to_irq(PMU_IRQ_PIN),
		.platform_data  = &bcm590xx_plat_data_sl1,
	},
	[1] = {
		I2C_BOARD_INFO("bcm590xx", PMU_DEVICE_I2C_ADDR_0 ),
		.irq = gpio_to_irq(PMU_IRQ_PIN),
		.platform_data  = &bcm590xx_plat_data,
	},
};

/* Rhea Ray specific platform devices */ 
static struct platform_device *rhea_ray_plat_devices[] __initdata = {

};

/* Rhea Ray specific i2c devices */ 
static void __init rhea_ray_add_i2c_devices (void)
{
	/* 59055 on BSC - PMU */
	i2c_register_board_info(2,
		pmu_info,
		ARRAY_SIZE(pmu_info));
}

static void enable_smi_display_clks(void)
{
	struct clk *smi_axi;
	struct clk *mm_dma;
	struct clk *smi;

	smi_axi = clk_get (NULL, "smi_axi_clk");
	mm_dma = clk_get (NULL, "mm_dma_axi_clk");

	smi = clk_get (NULL, "smi_clk");
	BUG_ON (!smi_axi || !smi || !mm_dma);


	clk_set_rate (smi, 250000000);

	clk_enable (smi_axi);
	clk_enable (smi);
	clk_enable(mm_dma);
}

/* All Rhea Ray specific devices */ 
static void __init rhea_ray_add_devices(void)
{
	enable_smi_display_clks();

	platform_add_devices(rhea_ray_plat_devices, ARRAY_SIZE(rhea_ray_plat_devices));

	rhea_ray_add_i2c_devices();
}

void __init board_init(void)
{
	board_add_common_devices();
	rhea_ray_add_devices();
	return;
}

void __init board_map_io(void)
{
	/* Map machine specific iodesc here */

	rhea_map_io();
}

MACHINE_START(RHEA, "RheaRay")
	.phys_io = IO_START,
	.io_pg_offst = (IO_BASE >> 18) & 0xFFFC,
	.map_io = board_map_io,
	.init_irq = kona_init_irq,
	.timer  = &kona_timer,
	.init_machine = board_init,
MACHINE_END
