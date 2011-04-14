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
#include <asm/gpio.h>
#include <mach/kona.h>
#include <mach/samoa.h>
#include <asm/mach/map.h>
#include <linux/clk.h>
#include "common.h"


/* Samoa FPGA specific platform devices */ 
static struct platform_device *samoa_fpga_plat_devices[] __initdata = {

};

/* Samoa FPGA specific i2c devices */ 
static void __init samoa_fpga_add_i2c_devices (void)
{

}

static int __init samoa_fpga_add_lateInit_devices (void)
{
#if 0
	struct i2c_adapter *adapter;
	struct i2c_client *client;

	adapter = i2c_get_adapter(1);
	if (!adapter) {
		printk(KERN_ERR "can't get i2c adapter 1 %d\n");
		return ENODEV;
	}
#ifdef CONFIG_GPIO_PCA953X
	client = i2c_new_device(adapter, pca953x_info);
	if (!client) {
		printk(KERN_ERR "an't add i2c device for pca953x\n");
	}
#endif

#ifdef CONFIG_TOUCHSCREEN_QT602240
	client = i2c_new_device(adapter, qt602240_info);
	if (!client) {
		printk(KERN_ERR "an't add i2c device for qt602240\n");
	}
#endif
	i2c_put_adapter(adapter);

	board_add_sdio_devices();
#endif
	return 0;
}


/* All samoa fpga specific devices */ 
static void __init samoa_fpga_add_devices(void)
{
	platform_add_devices(samoa_fpga_plat_devices, ARRAY_SIZE(samoa_fpga_plat_devices));
	samoa_fpga_add_i2c_devices();
}

void __init board_init(void)
{
	board_add_common_devices();
	samoa_fpga_add_devices();
	return;
}

void __init board_map_io(void)
{
	/* Map machine specific iodesc here */

	samoa_map_io();
}

late_initcall(samoa_fpga_add_lateInit_devices);

MACHINE_START(RHEA, "SamoaFpga")
	.phys_io = IO_START,
	.io_pg_offst = (IO_BASE >> 18) & 0xFFFC,
	.map_io = board_map_io,
	.init_irq = kona_init_irq,
	.timer  = &kona_timer,
	.init_machine = board_init,
MACHINE_END
