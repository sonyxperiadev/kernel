/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2011  Broadcom Corporation                                                        */
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
#ifdef CONFIG_GPIO_PCA953X
#include <linux/i2c/pca953x.h>
#endif
#ifdef CONFIG_TOUCHSCREEN_QT602240
#include <linux/i2c/qt602240_ts.h>
#endif
#include <mach/kona.h>
#include <mach/samoa.h>
#include <asm/mach/map.h>
#include <linux/mfd/bcm590xx/core.h>
#include <linux/mfd/bcm590xx/pmic.h>
#include <linux/clk.h>
#include "common.h"
#ifdef CONFIG_KEYBOARD_BCM
#include <mach/bcm_keypad.h>
#endif

// keypad map
#define BCM_KEY_ROW_0  0
#define BCM_KEY_ROW_1  1
#define BCM_KEY_ROW_2  2
#define BCM_KEY_ROW_3  3
#define BCM_KEY_ROW_4  4
#define BCM_KEY_ROW_5  5
#define BCM_KEY_ROW_6  6
#define BCM_KEY_ROW_7  7

#define BCM_KEY_COL_0  0
#define BCM_KEY_COL_1  1
#define BCM_KEY_COL_2  2
#define BCM_KEY_COL_3  3
#define BCM_KEY_COL_4  4
#define BCM_KEY_COL_5  5
#define BCM_KEY_COL_6  6
#define BCM_KEY_COL_7  7


#ifdef CONFIG_KEYBOARD_BCM
/*!
 * The keyboard definition structure.
 */
struct platform_device bcm_kp_device = {
	.name = "bcm_keypad",
	.id = -1,
};

/*	Keymap for Ray board plug-in 64-key keypad. 
	Since LCD block has used pin GPIO00, GPIO01, GPIO02, GPIO03,
	GPIO08, GPIO09, GPIO10 and GPIO11, Keypad can be set as 4x4 matric by
	using pin GPIO04, GPIO05, GPIO06, GPIO07, GPIO12, GPIO13, GPIO14 and
	GPIO15 */
static struct bcm_keymap newKeymap[] = {
	{BCM_KEY_ROW_4, BCM_KEY_COL_4, "Search Key", KEY_SEARCH},
	{BCM_KEY_ROW_4, BCM_KEY_COL_5, "Back Key", KEY_BACK},
	{BCM_KEY_ROW_4, BCM_KEY_COL_6, "Forward key", KEY_FORWARD},
	{BCM_KEY_ROW_4, BCM_KEY_COL_7, "Home Key", KEY_HOME},
	{BCM_KEY_ROW_5, BCM_KEY_COL_4, "Menu-Key", KEY_MENU},
	{BCM_KEY_ROW_5, BCM_KEY_COL_5, "VolumnUp-Key", KEY_VOLUMEUP},
	{BCM_KEY_ROW_5, BCM_KEY_COL_6, "VolumnDown-Key", KEY_VOLUMEDOWN},
	{BCM_KEY_ROW_5, BCM_KEY_COL_7, "key mute", KEY_MUTE},
	{BCM_KEY_ROW_6, BCM_KEY_COL_4, "key space", KEY_SPACE},
	{BCM_KEY_ROW_6, BCM_KEY_COL_5, "key power", KEY_POWER},
	{BCM_KEY_ROW_6, BCM_KEY_COL_6, "key sleep", KEY_SLEEP},
	{BCM_KEY_ROW_6, BCM_KEY_COL_7, "key wakeup", KEY_WAKEUP},
	{BCM_KEY_ROW_7, BCM_KEY_COL_4, "unused", 0},
	{BCM_KEY_ROW_7, BCM_KEY_COL_5, "unused", 0},
	{BCM_KEY_ROW_7, BCM_KEY_COL_6, "unused", 0},
	{BCM_KEY_ROW_7, BCM_KEY_COL_7, "unused", 0},
};

static struct bcm_keypad_platform_info bcm_keypad_data = {
	.row_num = 8,
	.col_num = 8,
	.keymap = newKeymap,
	.bcm_keypad_base = (void *)__iomem HW_IO_PHYS_TO_VIRT(KEYPAD_BASE_ADDR),
};

#endif


#ifdef CONFIG_GPIO_PCA953X
#define GPIO_PCA953X_GPIO_PIN      74 /* Configure pad MMC1DAT4 to GPIO74 */
static int pca953x_platform_init_hw(struct i2c_client *client,
		unsigned gpio, unsigned ngpio, void *context)
{
	int rc;
	rc = gpio_request(GPIO_PCA953X_GPIO_PIN, "gpio_expander");
	if (rc < 0)
	{
		printk(KERN_ERR "unable to request GPIO pin %d\n", GPIO_PCA953X_GPIO_PIN);
		return rc;
	}
	gpio_direction_input(GPIO_PCA953X_GPIO_PIN);
	return 0;
}

static int pca953x_platform_exit_hw(struct i2c_client *client,
		unsigned gpio, unsigned ngpio, void *context)
{
	gpio_free(GPIO_PCA953X_GPIO_PIN);
	return 0;
}

static struct pca953x_platform_data board_expander_info = {
	.gpio_base	= KONA_MAX_GPIO,
	.irq_base	= gpio_to_irq(KONA_MAX_GPIO),
	.setup		= pca953x_platform_init_hw,
	.teardown	= pca953x_platform_exit_hw,
};

static struct i2c_board_info __initdata pca953x_info[] = {
	{
		I2C_BOARD_INFO("pca9539", 0x74),
		.irq = gpio_to_irq(GPIO_PCA953X_GPIO_PIN),
		.platform_data = &board_expander_info,
	},
};
#endif /* CONFIG_GPIO_PCA953X */

#ifdef CONFIG_TOUCHSCREEN_QT602240
#ifdef CONFIG_GPIO_PCA953X
#define QT602240_INT_GPIO_PIN      (KONA_MAX_GPIO + 8)
#else
#define QT602240_INT_GPIO_PIN      74 /* skip expander chip */
#endif
static int qt602240_platform_init_hw(void)
{
	int rc;
	rc = gpio_request(QT602240_INT_GPIO_PIN, "ts_qt602240");
	if (rc < 0)
	{
		printk(KERN_ERR "unable to request GPIO pin %d\n", QT602240_INT_GPIO_PIN);
		return rc;
	}
	gpio_direction_input(QT602240_INT_GPIO_PIN);

	return 0;
}

static void qt602240_platform_exit_hw(void)
{
	gpio_free(QT602240_INT_GPIO_PIN);
}

static struct qt602240_platform_data qt602240_platform_data = {
	.x_line		= 17,
	.y_line		= 11,
	.x_size		= 800,
	.y_size		= 480,
	.blen		= 0x21,
	.threshold	= 0x28,
	.voltage	= 2800000,              /* 2.8V */
	.orient		= QT602240_DIAGONAL_COUNTER,
	.init_platform_hw = qt602240_platform_init_hw,
	.exit_platform_hw = qt602240_platform_exit_hw,
};

static struct i2c_board_info __initdata qt602240_info[] = {
	{
		I2C_BOARD_INFO("qt602240_ts", 0x4a),
		.platform_data = &qt602240_platform_data,
		.irq = gpio_to_irq(QT602240_INT_GPIO_PIN),
	},
};
#endif /* CONFIG_TOUCHSCREEN_QT602240 */

/* Samoa Ray specific platform devices */ 
static struct platform_device *samoa_ray_plat_devices[] __initdata = {
#ifdef CONFIG_KEYBOARD_BCM
	&bcm_kp_device,
#endif

};

/* Samoa Ray specific i2c devices */ 
static void __init samoa_ray_add_i2c_devices (void)
{
#ifdef CONFIG_GPIO_PCA953X
	i2c_register_board_info(1, pca953x_info, ARRAY_SIZE(pca953x_info));
#endif

#ifdef CONFIG_TOUCHSCREEN_QT602240
	i2c_register_board_info(1, qt602240_info, ARRAY_SIZE(qt602240_info));
#endif
}

static void enable_smi_display_clks(void)
{
#if 0
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
#endif
}

/* All Samoa Ray specific devices */ 
static void __init samoa_ray_add_devices(void)
{
	enable_smi_display_clks();

#ifdef CONFIG_KEYBOARD_BCM
	bcm_kp_device.dev.platform_data = &bcm_keypad_data;
#endif
	platform_add_devices(samoa_ray_plat_devices, ARRAY_SIZE(samoa_ray_plat_devices));

	samoa_ray_add_i2c_devices();
}

void __init board_init(void)
{
	board_add_common_devices();
	samoa_ray_add_devices();
	return;
}

void __init board_map_io(void)
{
	/* Map machine specific iodesc here */

	samoa_map_io();
}

/* use RHEA ID for now */
MACHINE_START(RHEA, "SamoaRay")
	.phys_io = IO_START,
	.io_pg_offst = (IO_BASE >> 18) & 0xFFFC,
	.map_io = board_map_io,
	.init_irq = kona_init_irq,
	.timer  = &kona_timer,
	.init_machine = board_init,
MACHINE_END
