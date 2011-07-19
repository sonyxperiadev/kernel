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
#ifdef CONFIG_GPIO_TC3589X
#include <linux/mfd/tc3589x.h>
#endif

#include <mach/rdb/brcm_rdb_kproc_clk_mgr_reg.h>
#include <mach/rdb/brcm_rdb_bmdm_clk_mgr_reg.h>
#include <mach/rdb/brcm_rdb_kps_clk_mgr_reg.h>
#include <mach/rdb/brcm_rdb_kpm_clk_mgr_reg.h>
#include <mach/rdb/brcm_rdb_khubaon_clk_mgr_reg.h>
#include <mach/rdb/brcm_rdb_ana_audio_reg.h> 
#include <mach/rdb/brcm_rdb_auxmic.h> 
#include <mach/rdb/brcm_rdb_aci.h> 
#include <mach/rdb/brcm_rdb_audioh.h>   
#include <mach/rdb/brcm_rdb_khub_clk_mgr_reg.h> 

#include <mach/kona.h>
#include <mach/samoa.h>
#include <asm/mach/map.h>
#include <linux/clk.h>
#include "common.h"
#ifdef CONFIG_KEYBOARD_BCM
#include <mach/bcm_keypad.h>
#endif

#define _SAMOA_  /* needed by platform_mconfig.h */
#include <linux/broadcom/bcm_fuse_memmap.h>
#include <linux/broadcom/platform_mconfig.h>

#ifdef CONFIG_KEYBOARD_BCM
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


/*!
 * The keyboard definition structure.
 */
struct platform_device bcm_kp_device = {
	.name = "bcm_keypad",
	.id = -1,
};

/*	Keymap for Ray board plug-in 64-key keypad. 
	Use full 8x8 matrix, but only 16 keys in keymap*/
static struct bcm_keymap newKeymap[] = {
	{BCM_KEY_ROW_0, BCM_KEY_COL_0, "unused", 0},
	{BCM_KEY_ROW_0, BCM_KEY_COL_1, "unused", 0},
	{BCM_KEY_ROW_0, BCM_KEY_COL_2, "unused", 0},
	{BCM_KEY_ROW_0, BCM_KEY_COL_3, "unused", 0},
	{BCM_KEY_ROW_0, BCM_KEY_COL_4, "unused", 0},
	{BCM_KEY_ROW_0, BCM_KEY_COL_5, "unused", 0},
	{BCM_KEY_ROW_0, BCM_KEY_COL_6, "unused", 0},
	{BCM_KEY_ROW_0, BCM_KEY_COL_7, "unused", 0},
	{BCM_KEY_ROW_1, BCM_KEY_COL_0, "unused", 0},
	{BCM_KEY_ROW_1, BCM_KEY_COL_1, "unused", 0},
	{BCM_KEY_ROW_1, BCM_KEY_COL_2, "unused", 0},
	{BCM_KEY_ROW_1, BCM_KEY_COL_3, "unused", 0},
	{BCM_KEY_ROW_1, BCM_KEY_COL_4, "unused", 0},
	{BCM_KEY_ROW_1, BCM_KEY_COL_5, "unused", 0},
	{BCM_KEY_ROW_1, BCM_KEY_COL_6, "unused", 0},
	{BCM_KEY_ROW_1, BCM_KEY_COL_7, "unused", 0},
	{BCM_KEY_ROW_2, BCM_KEY_COL_0, "unused", 0},
	{BCM_KEY_ROW_2, BCM_KEY_COL_1, "unused", 0},
	{BCM_KEY_ROW_2, BCM_KEY_COL_2, "unused", 0},
	{BCM_KEY_ROW_2, BCM_KEY_COL_3, "unused", 0},
	{BCM_KEY_ROW_2, BCM_KEY_COL_4, "unused", 0},
	{BCM_KEY_ROW_2, BCM_KEY_COL_5, "unused", 0},
	{BCM_KEY_ROW_2, BCM_KEY_COL_6, "unused", 0},
	{BCM_KEY_ROW_2, BCM_KEY_COL_7, "unused", 0},
	{BCM_KEY_ROW_3, BCM_KEY_COL_0, "unused", 0},
	{BCM_KEY_ROW_3, BCM_KEY_COL_1, "unused", 0},
	{BCM_KEY_ROW_3, BCM_KEY_COL_2, "unused", 0},
	{BCM_KEY_ROW_3, BCM_KEY_COL_3, "unused", 0},
	{BCM_KEY_ROW_3, BCM_KEY_COL_4, "unused", 0},
	{BCM_KEY_ROW_3, BCM_KEY_COL_5, "unused", 0},
	{BCM_KEY_ROW_3, BCM_KEY_COL_6, "unused", 0},
	{BCM_KEY_ROW_3, BCM_KEY_COL_7, "unused", 0},
	{BCM_KEY_ROW_4, BCM_KEY_COL_0, "unused", 0},
	{BCM_KEY_ROW_4, BCM_KEY_COL_1, "unused", 0},
	{BCM_KEY_ROW_4, BCM_KEY_COL_2, "unused", 0},
	{BCM_KEY_ROW_4, BCM_KEY_COL_3, "unused", 0},
	{BCM_KEY_ROW_4, BCM_KEY_COL_4, "Search Key", KEY_SEARCH},
	{BCM_KEY_ROW_4, BCM_KEY_COL_5, "Back Key", KEY_BACK},
	{BCM_KEY_ROW_4, BCM_KEY_COL_6, "Forward key", KEY_FORWARD},
	{BCM_KEY_ROW_4, BCM_KEY_COL_7, "Home Key", KEY_HOME},
	{BCM_KEY_ROW_5, BCM_KEY_COL_0, "unused", 0},
	{BCM_KEY_ROW_5, BCM_KEY_COL_1, "unused", 0},
	{BCM_KEY_ROW_5, BCM_KEY_COL_2, "unused", 0},
	{BCM_KEY_ROW_5, BCM_KEY_COL_3, "unused", 0},
	{BCM_KEY_ROW_5, BCM_KEY_COL_4, "Menu-Key", KEY_MENU},
	{BCM_KEY_ROW_5, BCM_KEY_COL_5, "VolumnUp-Key", KEY_VOLUMEUP},
	{BCM_KEY_ROW_5, BCM_KEY_COL_6, "VolumnDown-Key", KEY_VOLUMEDOWN},
	{BCM_KEY_ROW_5, BCM_KEY_COL_7, "key mute", KEY_MUTE},
	{BCM_KEY_ROW_6, BCM_KEY_COL_0, "unused", 0},
	{BCM_KEY_ROW_6, BCM_KEY_COL_1, "unused", 0},
	{BCM_KEY_ROW_6, BCM_KEY_COL_2, "unused", 0},
	{BCM_KEY_ROW_6, BCM_KEY_COL_3, "unused", 0},
	{BCM_KEY_ROW_6, BCM_KEY_COL_4, "key space", KEY_SPACE},
	{BCM_KEY_ROW_6, BCM_KEY_COL_5, "key power", KEY_POWER},
	{BCM_KEY_ROW_6, BCM_KEY_COL_6, "key sleep", KEY_SLEEP},
	{BCM_KEY_ROW_6, BCM_KEY_COL_7, "key wakeup", KEY_WAKEUP},
	{BCM_KEY_ROW_7, BCM_KEY_COL_0, "unused", 0},
	{BCM_KEY_ROW_7, BCM_KEY_COL_1, "unused", 0},
	{BCM_KEY_ROW_7, BCM_KEY_COL_2, "unused", 0},
	{BCM_KEY_ROW_7, BCM_KEY_COL_3, "unused", 0},
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

#if defined(CONFIG_MFD_TC3589X) && defined(CONFIG_GPIO_TC3589X)
#ifdef CONFIG_BCM_KEYBOARD
#define GPIO_TC3589X_GPIO_PIN      22 /* Configure BB gpio for IOexpander IRQ */
#else
#define GPIO_TC3589X_GPIO_PIN      1  /* Configure BB gpio for IOexpander IRQ */
#endif

static void tc3589x_init(struct tc3589x *tc3589x, unsigned int base)
{
	//FIXME? could move dev int setup to here?
}

static struct tc3589x_gpio_platform_data tc3589x_gpio_data = {
	.gpio_base	= KONA_MAX_GPIO,
	.setup		= tc3589x_init,
};

static struct tc3589x_platform_data tc3589x_data = {
	.block		= TC3589x_BLOCK_GPIO,
	.gpio		= &tc3589x_gpio_data,
	.irq_base	= gpio_to_irq(KONA_MAX_GPIO),
};

static struct i2c_board_info __initdata tc3589x_info[] = {
	{
		I2C_BOARD_INFO("tc3589x", 0x45),
		.irq = gpio_to_irq(GPIO_TC3589X_GPIO_PIN),
		.platform_data = &tc3589x_data,
	},
};

#endif /* CONFIG_MFD_TC3589X && CONFIG_GPIO_TC3589X*/



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

#ifdef CONFIG_MFD_TC3589X
	i2c_register_board_info(1, tc3589x_info, ARRAY_SIZE(tc3589x_info));
#endif

}


/* All Samoa Ray specific devices */ 
static void __init samoa_ray_add_devices(void)
{
#ifdef CONFIG_KEYBOARD_BCM
	bcm_kp_device.dev.platform_data = &bcm_keypad_data;
#endif
	platform_add_devices(samoa_ray_plat_devices, ARRAY_SIZE(samoa_ray_plat_devices));

	samoa_ray_add_i2c_devices();

#ifdef CONFIG_MFD_BCMSAMOA	
	board_pmu_init();
#endif
}

/* Stub clock API to allow drivers to build */

#if 0	/* ssg, bringup clock */
void clk_disable(struct clk *clk) {;}
int clk_enable(struct clk *clk) {return 0;}
unsigned long clk_get_rate(struct clk *clk) {return 1000000;}
int clk_set_rate(struct clk *clk, unsigned long rate) {return 0;}
EXPORT_SYMBOL(clk_disable);
EXPORT_SYMBOL(clk_enable);
EXPORT_SYMBOL(clk_get_rate);
EXPORT_SYMBOL(clk_set_rate);
#endif


void __init board_proc_clk_print(void)
{
	u32 proc_clk_mgr_base_v;
	u32 bmdm_clk_mgr_base_v;

	proc_clk_mgr_base_v = (u32)ioremap(PROC_CLK_BASE_ADDR, 0x1000);
	bmdm_clk_mgr_base_v = (u32)ioremap(BMDM_CCU_BASE_ADDR, 0x1000);

	/* print proc_clk set up by the boot loader */
	printk(KERN_ERR "KPROC: FREQ=0x%x\n", readl(proc_clk_mgr_base_v+KPROC_CLK_MGR_REG_POLICY_FREQ_OFFSET));
	printk(KERN_ERR "KPROC: POLICY_CTL=0x%x\n", readl(proc_clk_mgr_base_v+KPROC_CLK_MGR_REG_POLICY_CTL_OFFSET));
	printk(KERN_ERR "KPROC: PLLARMA=0x%x\n", readl(proc_clk_mgr_base_v+KPROC_CLK_MGR_REG_PLLARMA_OFFSET));
	printk(KERN_ERR "KPROC: PLLARMB=0x%x\n", readl(proc_clk_mgr_base_v+KPROC_CLK_MGR_REG_PLLARMB_OFFSET));
	printk(KERN_ERR "KPROC: PLLARMC=0x%x\n", readl(proc_clk_mgr_base_v+KPROC_CLK_MGR_REG_PLLARMC_OFFSET));
	printk(KERN_ERR "KPROC: PLLARMCTRL5=0x%x\n", readl(proc_clk_mgr_base_v+KPROC_CLK_MGR_REG_PLLARMCTRL5_OFFSET));
	printk(KERN_ERR "KPROC: ARM_DIV=0x%x\n", readl(proc_clk_mgr_base_v+KPROC_CLK_MGR_REG_ARM_DIV_OFFSET));

	/* registers needed ASIC team to debug A5 speed issue */
	printk(KERN_ERR "KPROC: PL310_DIV=0x%x\n", readl(proc_clk_mgr_base_v+KPROC_CLK_MGR_REG_PL310_DIV_OFFSET));
	printk(KERN_ERR "KPROC: PL310_TRIGGER=0x%x\n", readl(proc_clk_mgr_base_v+KPROC_CLK_MGR_REG_PL310_TRIGGER_OFFSET));
	printk(KERN_ERR "KPROC: ACTIVITY_MON1=0x%x\n", readl(proc_clk_mgr_base_v+KPROC_CLK_MGR_REG_ACTIVITY_MON1_OFFSET));
	printk(KERN_ERR "KPROC: CLKGATE_DBG=0x%x\n", readl(proc_clk_mgr_base_v+KPROC_CLK_MGR_REG_CLKGATE_DBG_OFFSET));
	printk(KERN_ERR "KPROC: PB_CLKGATE_DBG1=0x%x\n", readl(proc_clk_mgr_base_v+KPROC_CLK_MGR_REG_APB_CLKGATE_DBG1_OFFSET));
	printk(KERN_ERR "KPROC: POLICY_DBG=0x%x\n", readl(proc_clk_mgr_base_v+KPROC_CLK_MGR_REG_POLICY_DBG_OFFSET));

	/* print bmdm_clk set up by the boot loader */
	printk(KERN_ERR "BMDM_CCU: FREQ=0x%x\n", readl(bmdm_clk_mgr_base_v+BMDM_CLK_MGR_REG_POLICY_FREQ_OFFSET));

	iounmap((void *)proc_clk_mgr_base_v);
	iounmap((void *)bmdm_clk_mgr_base_v);
}

#define CLK_MGR_REG_WR_ACCESS_OFFSET 0
#define CLK_MGR_REG_POLICY_CTL_OFFSET 0xc
#define CLK_MGR_REG_LVM_EN_OFFSET     0x34
void __init board_configure(void)
{
    void __iomem *base;
    int val;

	/* print hubaon_timer */
	printk(KERN_ERR "HAON_CCU: HUB_TIMER_DIV=0x%x\n", readl(KONA_AON_CLK_VA+KHUBAON_CLK_MGR_REG_HUB_TIMER_DIV_OFFSET));
	/* print peri_timer */
	printk(KERN_ERR "KPS_CCU: TIMERS_DIV=0x%x\n", readl(KONA_KPS_CLK_VA+KPS_CLK_MGR_REG_TIMERS_DIV_OFFSET));

	/* enable SDIO2 (for SD card) clocks before clock driver is ready */
	printk(KERN_ERR "SDIO1_CLKGATE =0x%x\n", readl(KONA_KPM_CLK_VA+KPM_CLK_MGR_REG_SDIO1_CLKGATE_OFFSET));

	writel(0xA5A501, (KONA_KPM_CLK_VA+KPM_CLK_MGR_REG_WR_ACCESS_OFFSET));
	writel(readl(KONA_KPM_CLK_VA+KPM_CLK_MGR_REG_KPM_POLICY3_MASK_OFFSET) |
		KPM_CLK_MGR_REG_KPM_POLICY3_MASK_SDIO1_POLICY3_MASK_MASK,
		(KONA_KPM_CLK_VA+KPM_CLK_MGR_REG_KPM_POLICY3_MASK_OFFSET));
	writel(0x1, (KONA_KPM_CLK_VA+KPM_CLK_MGR_REG_LVM_EN_OFFSET));
	while ((readl(KONA_KPM_CLK_VA+KPM_CLK_MGR_REG_LVM_EN_OFFSET)&0x1) == 0x1 );
	writel(0x5, (KONA_KPM_CLK_VA+KPM_CLK_MGR_REG_POLICY_CTL_OFFSET));
	writel(0x1f, (KONA_KPM_CLK_VA+KPM_CLK_MGR_REG_SDIO1_CLKGATE_OFFSET));
	printk(KERN_ERR "SDIO1_CLKGATE =0x%x\n", readl(KONA_KPM_CLK_VA+KPM_CLK_MGR_REG_SDIO1_CLKGATE_OFFSET));
	printk(KERN_ERR "SDIO1_DIV =0x%x\n", readl(KONA_KPM_CLK_VA+KPM_CLK_MGR_REG_SDIO1_DIV_OFFSET));

	/* eMMC on SDIO2. Print clock*/
	printk(KERN_ERR "SDIO2_CLKGATE =0x%x\n", readl(KONA_KPM_CLK_VA+KPM_CLK_MGR_REG_SDIO2_CLKGATE_OFFSET));
	printk(KERN_ERR "SDIO2_DIV =0x%x\n", readl(KONA_KPM_CLK_VA+KPM_CLK_MGR_REG_SDIO2_DIV_OFFSET));

	/* enable I2C clocks before clock driver is ready */
	// BSC1 & BSC2 clocks
	printk(KERN_ERR "I2C: BSC1_CLKGATE=0x%x\n", readl(KONA_KPS_CLK_VA+KPS_CLK_MGR_REG_BSC1_CLKGATE_OFFSET));
	printk(KERN_ERR "I2C: BSC2_CLKGATE=0x%x\n", readl(KONA_KPS_CLK_VA+KPS_CLK_MGR_REG_BSC2_CLKGATE_OFFSET));
#ifdef CONFIG_MACH_SAMOA_RAY_TEST_ON_RHEA_RAY
	writel(0xA5A501, (KONA_KPS_CLK_VA+CLK_MGR_REG_WR_ACCESS_OFFSET));
#else
	/* On Samoa we use kpm to control kps clocks */
#endif

	writel(readl(KONA_KPS_CLK_VA+KPS_CLK_MGR_REG_KPS_POLICY3_MASK_OFFSET) |
		KPS_CLK_MGR_REG_KPS_POLICY3_MASK_BSC1_POLICY3_MASK_MASK |
		KPS_CLK_MGR_REG_KPS_POLICY3_MASK_BSC2_POLICY3_MASK_MASK,
		(KONA_KPS_CLK_VA+KPS_CLK_MGR_REG_KPS_POLICY3_MASK_OFFSET));
#ifdef CONFIG_MACH_SAMOA_RAY_TEST_ON_RHEA_RAY
	writel(0x1, (KONA_KPS_CLK_VA+CLK_MGR_REG_LVM_EN_OFFSET));
	while ((readl(KONA_KPS_CLK_VA+CLK_MGR_REG_LVM_EN_OFFSET)&0x1) == 0x1 );
	writel(0x5, (KONA_KPS_CLK_VA+CLK_MGR_REG_POLICY_CTL_OFFSET));
#else
	/* Samoa chip is diffeent. Use kpm to control kps clocks */
	writel(0x1, (KONA_KPM_CLK_VA+CLK_MGR_REG_LVM_EN_OFFSET));
	while ((readl(KONA_KPM_CLK_VA+CLK_MGR_REG_LVM_EN_OFFSET)&0x1) == 0x1 );
	writel(0x5, (KONA_KPM_CLK_VA+CLK_MGR_REG_POLICY_CTL_OFFSET));
#endif
	writel(readl(KONA_KPS_CLK_VA+KPS_CLK_MGR_REG_KPS_POLICY3_MASK_OFFSET) |
                                KPS_CLK_MGR_REG_KPS_POLICY3_MASK_BSC1_POLICY3_MASK_MASK |
                                KPS_CLK_MGR_REG_KPS_POLICY3_MASK_BSC2_POLICY3_MASK_MASK |
                                KPS_CLK_MGR_REG_KPS_POLICY3_MASK_UARTB2_POLICY3_MASK_MASK,
                                (KONA_KPS_CLK_VA+KPS_CLK_MGR_REG_KPS_POLICY3_MASK_OFFSET));
	writel(0x30f, (KONA_KPS_CLK_VA+KPS_CLK_MGR_REG_BSC1_CLKGATE_OFFSET));
	writel(0x30f, (KONA_KPS_CLK_VA+KPS_CLK_MGR_REG_BSC2_CLKGATE_OFFSET));
	writel(0x00f, (KONA_KPS_CLK_VA+ KPS_CLK_MGR_REG_UARTB2_CLKGATE_OFFSET));
	writel(0xB101, (KONA_KPS_CLK_VA+ KPS_CLK_MGR_REG_UARTB2_DIV_OFFSET));  
	writel(0xf, (KONA_KPS_CLK_VA+ KPS_CLK_MGR_REG_UARTB2_CLKGATE_OFFSET));

    /* caph clock enable.  The following code is temporary until clk_xxx APIs are ready */ 
    /* Set the frequency policy */ 
    writel(0xA5A501, (KONA_KPM_CLK_VA+KPM_CLK_MGR_REG_WR_ACCESS_OFFSET)); 

    writel(0x00000011, (KONA_HUB_CLK_VA+KHUB_CLK_MGR_REG_CAPH_DIV_OFFSET)); 
    writel(0x00100000, (KONA_HUB_CLK_VA+KHUB_CLK_MGR_REG_PERIPH_SEG_TRG_OFFSET)); 
    writel(0x3030, (KONA_HUB_CLK_VA+KHUB_CLK_MGR_REG_CAPH_CLKGATE_OFFSET)); 

    writel(0x7FFFFFFF, (KONA_HUB_CLK_VA+KHUB_CLK_MGR_REG_KHUB_POLICY0_MASK1_OFFSET)); 
    writel(0x7FFFFFFF, (KONA_HUB_CLK_VA+KHUB_CLK_MGR_REG_KHUB_POLICY1_MASK1_OFFSET)); 
    writel(0x7FFFFFFF, (KONA_HUB_CLK_VA+KHUB_CLK_MGR_REG_KHUB_POLICY2_MASK1_OFFSET)); 
    writel(0x7FFFFFFF, (KONA_HUB_CLK_VA+KHUB_CLK_MGR_REG_KHUB_POLICY3_MASK1_OFFSET)); 

    writel(0x7FFFFFFF, (KONA_HUB_CLK_VA+KHUB_CLK_MGR_REG_KHUB_POLICY0_MASK2_OFFSET)); 
    writel(0x7FFFFFFF, (KONA_HUB_CLK_VA+KHUB_CLK_MGR_REG_KHUB_POLICY1_MASK2_OFFSET)); 
    writel(0x7FFFFFFF, (KONA_HUB_CLK_VA+KHUB_CLK_MGR_REG_KHUB_POLICY2_MASK2_OFFSET)); 
    writel(0x7FFFFFFF, (KONA_HUB_CLK_VA+KHUB_CLK_MGR_REG_KHUB_POLICY3_MASK2_OFFSET)); 

    /* Samoa chip is diffeent. Use kpm to control kps clocks */ 
    writel(0x1, (KONA_KPM_CLK_VA+CLK_MGR_REG_LVM_EN_OFFSET)); 
    while ((readl(KONA_KPM_CLK_VA+CLK_MGR_REG_LVM_EN_OFFSET)&0x1) == 0x1 ); 
    writel(0x3, (KONA_KPM_CLK_VA+CLK_MGR_REG_POLICY_CTL_OFFSET)); 

    writel(0x0000FFFF, (KONA_HUB_CLK_VA+KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_OFFSET)); 

    writel(0xA5A501, (KONA_KPM_CLK_VA+KPM_CLK_MGR_REG_WR_ACCESS_OFFSET)); 
    writel(0x1, (KONA_KPM_CLK_VA+CLK_MGR_REG_LVM_EN_OFFSET)); 
    while ((readl(KONA_KPM_CLK_VA+CLK_MGR_REG_LVM_EN_OFFSET)&0x1) == 0x1 ); 
    writel(0x3, (KONA_KPM_CLK_VA+CLK_MGR_REG_POLICY_CTL_OFFSET)); 

    writel(0x3, (KONA_HUB_CLK_VA+KHUB_CLK_MGR_REG_AUDREG_CTRL_CLKGATE_OFFSET)); 

    /* enable PMU audio related registers.  Once PMU related audio driver is done, can  
    move this elsewhere */ 
    writel(0x1, (KONA_ANA_AUDIO_REG_BASE_VA+ANA_AUDIO_REG_AUD_SYS_CTRL_OFFSET)); 
    writel(0x3, (KONA_ANA_AUDIO_REG_BASE_VA+ANA_AUDIO_REG_AUD_HVBG_CTRL_OFFSET)); 
    writel(0x1, (KONA_ANA_AUDIO_REG_BASE_VA+ANA_AUDIO_REG_AUD_ANALDO_CTRL_OFFSET)); 
    writel(0x0, (KONA_ANA_AUDIO_REG_BASE_VA+ANA_AUDIO_REG_AUD_ANALDO_TEST_OFFSET)); 
    writel(0x1, (KONA_ANA_AUDIO_REG_BASE_VA+ANA_AUDIO_REG_AUD_EPHSLDO_CTRL_OFFSET)); 
    writel(0x1, (KONA_ANA_AUDIO_REG_BASE_VA+ANA_AUDIO_REG_AUD_REFBG_CTRL_OFFSET)); 
    writel(0x0, (KONA_ANA_AUDIO_REG_BASE_VA+ANA_AUDIO_REG_AUD_HS_CTRL_OFFSET)); 
    writel(0x7, (KONA_ANA_AUDIO_REG_BASE_VA+ANA_AUDIO_REG_AUD_EP_CTRL_OFFSET)); 
    writel(0x0, (KONA_ANA_AUDIO_REG_BASE_VA+ANA_AUDIO_REG_AUD_IHF_CTRL_OFFSET)); 
    writel(0x0, (KONA_ANA_AUDIO_REG_BASE_VA+ANA_AUDIO_REG_AUD_IHF_TEST0_OFFSET)); 
    writel(0X8000007, (KONA_AUDIOH_BASE_VA+AUDIOH_AUDIORX_VRX1_OFFSET)); 
    writel(0x0, (KONA_AUDIOH_BASE_VA+AUDIOH_AUDIORX_BIAS_OFFSET)); 
    writel(0x5, (KONA_AUDIOH_BASE_VA+AUDIOH_AUDIORX_VMIC_OFFSET)); 
    writel(0x0, (KONA_AUXMIC_VA+AUXMIC_AUXEN_OFFSET)); 
    writel(0x1, (KONA_AUXMIC_VA+AUXMIC_F_PWRDWN_OFFSET)); 

    writel(0x1, (KONA_ACI_VA+ACI_MIC_BIAS_OFFSET)); 
    writel(0x1, (KONA_ACI_VA+ACI_ACI_CTRL_OFFSET)); 
    writel(0x1, (KONA_ACI_VA+ACI_ADC_PWD_OFFSET)); 
    writel(0x1, (KONA_ACI_VA+ACI_COMP_PWD_OFFSET)); 

    printk(KERN_ERR "Caph: KHUB_POLICY0=0x%x\n", readl(KONA_HUB_CLK_VA+KHUB_CLK_MGR_REG_KHUB_POLICY0_MASK1_OFFSET)); 
    printk(KERN_ERR "Caph: KHUB_POLICY1=0x%x\n", readl(KONA_HUB_CLK_VA+KHUB_CLK_MGR_REG_KHUB_POLICY1_MASK1_OFFSET)); 
    printk(KERN_ERR "Caph: KHUB_POLICY2=0x%x\n", readl(KONA_HUB_CLK_VA+KHUB_CLK_MGR_REG_KHUB_POLICY2_MASK1_OFFSET)); 
    printk(KERN_ERR "Caph: KHUB_POLICY3=0x%x\n", readl(KONA_HUB_CLK_VA+KHUB_CLK_MGR_REG_KHUB_POLICY3_MASK1_OFFSET)); 
    printk(KERN_ERR "Caph: KHUB_POLICY0=0x%x\n", readl(KONA_HUB_CLK_VA+KHUB_CLK_MGR_REG_KHUB_POLICY0_MASK2_OFFSET)); 
    printk(KERN_ERR "Caph: KHUB_POLICY1=0x%x\n", readl(KONA_HUB_CLK_VA+KHUB_CLK_MGR_REG_KHUB_POLICY1_MASK2_OFFSET)); 
    printk(KERN_ERR "Caph: KHUB_POLICY2=0x%x\n", readl(KONA_HUB_CLK_VA+KHUB_CLK_MGR_REG_KHUB_POLICY2_MASK2_OFFSET)); 
    printk(KERN_ERR "Caph: KHUB_POLICY3=0x%x\n", readl(KONA_HUB_CLK_VA+KHUB_CLK_MGR_REG_KHUB_POLICY3_MASK2_OFFSET)); 
    printk(KERN_ERR "Caph: AUDIOH_CLKGATE=0x%x\n", readl(KONA_HUB_CLK_VA+KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_OFFSET)); 
    printk(KERN_ERR "Caph: CAPH_DIV=0x%x\n", readl(KONA_HUB_CLK_VA+KHUB_CLK_MGR_REG_CAPH_DIV_OFFSET)); 
    printk(KERN_ERR "Caph: PERIPH_SEG_TRG=0x%x\n", readl(KONA_HUB_CLK_VA+KHUB_CLK_MGR_REG_PERIPH_SEG_TRG_OFFSET)); 
    printk(KERN_ERR "Caph: CAPH_CLKGATE=0x%x\n", readl(KONA_HUB_CLK_VA+KHUB_CLK_MGR_REG_CAPH_CLKGATE_OFFSET)); 

	printk(KERN_ERR "I2C: BSC1_CLKGATE=0x%x\n", readl(KONA_KPS_CLK_VA+KPS_CLK_MGR_REG_BSC1_CLKGATE_OFFSET));
	printk(KERN_ERR "I2C: BSC2_CLKGATE=0x%x\n", readl(KONA_KPS_CLK_VA+KPS_CLK_MGR_REG_BSC2_CLKGATE_OFFSET));
	printk(KERN_ERR "I2C: BSC1_DIV=0x%x\n", readl(KONA_KPS_CLK_VA+KPS_CLK_MGR_REG_BSC1_DIV_OFFSET));
	printk(KERN_ERR "I2C: BSC2_DIV=0x%x\n", readl(KONA_KPS_CLK_VA+KPS_CLK_MGR_REG_BSC2_DIV_OFFSET));

	writel(0, (KONA_KPM_CLK_VA+CLK_MGR_REG_WR_ACCESS_OFFSET));
#ifdef CONFIG_MACH_SAMOA_RAY_TEST_ON_RHEA_RAY
	writel(0, (KONA_KPS_CLK_VA+CLK_MGR_REG_WR_ACCESS_OFFSET));
#endif
	/* turn on USB clock */
 	base = (void __iomem *)KONA_MST_CLK_BASE_VA;

	writel(0x00A5A501, base);

	writel(KPM_CLK_MGR_REG_LVM_EN_POLICY_CONFIG_EN_MASK, base+KPM_CLK_MGR_REG_LVM_EN_OFFSET);
	while (readl(base+KPM_CLK_MGR_REG_LVM_EN_OFFSET)&KPM_CLK_MGR_REG_LVM_EN_POLICY_CONFIG_EN_MASK);

	val = readl(base+KPM_CLK_MGR_REG_KPM_POLICY0_MASK_OFFSET);
	val |=KPM_CLK_MGR_REG_KPM_POLICY0_MASK_USBH_POLICY0_MASK_MASK;
	writel(val, base+KPM_CLK_MGR_REG_KPM_POLICY0_MASK_OFFSET);

	val = readl(base+KPM_CLK_MGR_REG_KPM_POLICY1_MASK_OFFSET);
	val |=KPM_CLK_MGR_REG_KPM_POLICY1_MASK_USBH_POLICY1_MASK_MASK;
	writel(val, base+KPM_CLK_MGR_REG_KPM_POLICY1_MASK_OFFSET);

	val = readl(base+KPM_CLK_MGR_REG_KPM_POLICY0_MASK_OFFSET);
	val |=KPM_CLK_MGR_REG_KPM_POLICY2_MASK_USBH_POLICY2_MASK_MASK;
	writel(val, base+KPM_CLK_MGR_REG_KPM_POLICY2_MASK_OFFSET);

	val = readl(base+KPM_CLK_MGR_REG_KPM_POLICY3_MASK_OFFSET);
	val |=KPM_CLK_MGR_REG_KPM_POLICY3_MASK_USBH_POLICY3_MASK_MASK;
	writel(val, base+KPM_CLK_MGR_REG_KPM_POLICY3_MASK_OFFSET);

	val = readl(base+KPM_CLK_MGR_REG_POLICY_CTL_OFFSET);
	val |=KPM_CLK_MGR_REG_POLICY_CTL_GO_MASK|KPM_CLK_MGR_REG_POLICY_CTL_GO_AC_MASK;
	writel(val, base+KPM_CLK_MGR_REG_POLICY_CTL_OFFSET);

	while (readl(base+KPM_CLK_MGR_REG_POLICY_CTL_OFFSET)&KPM_CLK_MGR_REG_POLICY_CTL_GO_MASK);

	val = readl(base+KPM_CLK_MGR_REG_USB_EHCI_CLKGATE_OFFSET);
	val |=KPM_CLK_MGR_REG_USB_EHCI_CLKGATE_USBH_AHB_CLK_EN_MASK;
	writel(val, base+KPM_CLK_MGR_REG_USB_EHCI_CLKGATE_OFFSET);

	writel(0x00A5A500, base);

	printk("%s: USB Clock Enable Done\n", __func__);
    

}

#ifndef CONFIG_MACH_SAMOA_FPGA
static void Comms_Start(void)
{
    void __iomem *apcp_shmem = ioremap_nocache(IPC_BASE, IPC_SIZE);
    void __iomem *cp_boot_base;

    if (!apcp_shmem) {
        printk(KERN_ERR "%s: ioremap shmem failed\n", __func__);
        return;
    }
    /* clear first (9) 32-bit words in shared memory */
    memset(apcp_shmem, 0, IPC_SIZE);
    iounmap(apcp_shmem);

    cp_boot_base = ioremap(MODEM_DTCM_ADDRESS, CP_BOOT_BASE_SIZE);
    if (!cp_boot_base) {
        printk(KERN_ERR "%s: ioremap error\n", __func__);
        return;
    }

    /* Start the CP, Code taken from Nucleus BSP */
    *(unsigned int *)(cp_boot_base+INIT_ADDRESS_OFFSET) =
        *(unsigned int *)(cp_boot_base+MAIN_ADDRESS_OFFSET);

    iounmap(cp_boot_base);
    printk(KERN_ALERT "%s: modem (R4 COMMS) started....\n", __func__);
}
#endif

void __init board_init(void)
{
	/* remove this call later */
	board_proc_clk_print();
	board_configure();

#ifndef CONFIG_MACH_SAMOA_FPGA
	Comms_Start();
#endif

	board_add_common_devices();
	samoa_ray_add_devices();
	return;
}

void __init board_map_io(void)
{
	/* Map machine specific iodesc here */

	samoa_map_io();
}

MACHINE_START(BCM21455, "SamoaRay")
	.phys_io = IO_START,
	.io_pg_offst = (IO_BASE >> 18) & 0xFFFC,
	.map_io = board_map_io,
	.init_irq = kona_init_irq,
	.timer  = &kona_timer,
	.init_machine = board_init,
MACHINE_END
