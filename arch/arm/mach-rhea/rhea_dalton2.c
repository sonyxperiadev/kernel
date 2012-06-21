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
#include <mach/sdio_platform.h>
#ifdef CONFIG_GPIO_PCA953X
#include <linux/i2c/pca953x.h>
#endif
#ifdef CONFIG_TOUCHSCREEN_QT602240
#include <linux/i2c/qt602240_ts.h>
#endif
#ifdef CONFIG_REGULATOR_TPS728XX
#include <linux/regulator/tps728xx.h>
#endif
#include <mach/kona_headset_pd.h>
#include <mach/kona.h>
#include <mach/rhea.h>
#include <asm/mach/map.h>
#include <linux/power_supply.h>
#include <linux/mfd/bcm590xx/core.h>
#include <linux/mfd/bcm590xx/pmic.h>
#include <linux/mfd/bcm590xx/bcm59055_A0.h>
#include <linux/broadcom/bcm59055-power.h>
#include <linux/clk.h>
#include <linux/bootmem.h>
#include <plat/pi_mgr.h>
#include "common.h"
#ifdef CONFIG_KEYBOARD_BCM
#include <mach/bcm_keypad.h>
#endif
#ifdef CONFIG_DMAC_PL330
#include <mach/irqs.h>
#include <plat/pl330-pdata.h>
#include <linux/dma-mapping.h>
#endif
#include <linux/spi/spi.h>
#if defined (CONFIG_HAPTIC)
#include <linux/haptic.h>
#endif
#if defined (CONFIG_BMP18X)
#include <linux/bmp18x.h>
#endif
#if defined (CONFIG_AL3006)
#include <linux/al3006.h>
#endif
#if (defined(CONFIG_MPU_SENSORS_MPU6050A2) || defined(CONFIG_MPU_SENSORS_MPU6050B1))
#include <linux/mpu.h>
#endif
#define _RHEA_
#include <mach/comms/platform_mconfig.h>

#if (defined(CONFIG_BCM_RFKILL) || defined(CONFIG_BCM_RFKILL_MODULE))
#include <linux/broadcom/bcmbt_rfkill.h>
#endif

#ifdef CONFIG_BCM_BZHW
#include <linux/broadcom/bcm_bzhw.h>
#endif

#ifdef CONFIG_BCM_BT_LPM
#include <linux/broadcom/bcmbt_lpm.h>
#endif

#ifdef CONFIG_FB_BRCM_RHEA
#include <video/kona_fb_boot.h>
#include <video/kona_fb.h>
#endif

#include <linux/pwm_backlight.h>

#ifdef CONFIG_GPIO_PCA953X
#define SD_CARDDET_GPIO_PIN      (KONA_MAX_GPIO + 15)
#else
#define SD_CARDDET_GPIO_PIN      75
#endif

#include <media/soc_camera.h>
#include <mach/rdb/brcm_rdb_sysmap.h>
#include <mach/rdb/brcm_rdb_padctrlreg.h>
#include <linux/delay.h>

#ifdef CONFIG_WD_TAPPER
#include <linux/broadcom/wd-tapper.h>
#endif

#include <mach/rdb/brcm_rdb_keypad.h>
#include <linux/gpio_event.h>

#ifdef CONFIG_BRCM_UNIFIED_DHD_SUPPORT

#include "board-Rhea_wifi.h"
extern int rhea_wifi_status_register(void (*callback) (int card_present, void *dev_id),
			  void *dev_id);

#endif

#define PMU_DEVICE_I2C_ADDR_0   0x08
#define PMU_IRQ_PIN           29

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

#ifdef CONFIG_MFD_BCM_PMU590XX
static int bcm590xx_event_callback(int flag, int param)
{
	int ret;
	printk("REG: pmu_init_platform_hw called \n");
	switch (flag) {
	case BCM590XX_INITIALIZATION:
		ret = gpio_request(PMU_IRQ_PIN, "pmu_irq");
		if (ret < 0) {
			printk(KERN_ERR "%s unable to request GPIO pin %d\n",
			       __FUNCTION__, PMU_IRQ_PIN);
			return ret;
		}
		gpio_direction_input(PMU_IRQ_PIN);
		break;
	case BCM590XX_CHARGER_INSERT:
		pr_info("%s: BCM590XX_CHARGER_INSERT\n", __func__);
		break;
	default:
		return -EPERM;
	}
	return 0;
}

#ifdef CONFIG_BATTERY_BCM59055
static struct bcm590xx_battery_pdata bcm590xx_battery_plat_data = {
	.batt_min_volt = 3200,
	.batt_max_volt = 4200,
	.batt_technology = POWER_SUPPLY_TECHNOLOGY_LION,
	.usb_cc = CURRENT_500_MA,
	.wac_cc = CURRENT_900_MA,
	/* 1500mA = 5400 coloumb
	 * 1Ah = 3600 coloumb
	 */
	.batt_max_capacity = 5400,
};
#endif

#ifdef CONFIG_REGULATOR_BCM_PMU59055
/* Regulator registration */
struct regulator_consumer_supply sim_supply[] = {
	{.supply = "sim_vcc"},
	{.supply = "simldo_uc"},
};

static struct regulator_init_data bcm59055_simldo_data = {
	.constraints = {
			.name = "simldo",
			.min_uV = 1300000,
			.max_uV = 3300000,
			.valid_ops_mask = REGULATOR_CHANGE_STATUS |
			REGULATOR_CHANGE_VOLTAGE,
			.always_on = 1,
			.boot_on = 0,
			},
	.num_consumer_supplies = ARRAY_SIZE(sim_supply),
	.consumer_supplies = sim_supply,
};

struct regulator_consumer_supply h6ldo_supply[] = {
	{.supply = "vdd_sdxc"},
};

struct regulator_consumer_supply hv4_supply[] = {
	{.supply = "hv4ldo_uc"},
	{.supply = "vdd_sdio"},
};

static struct regulator_init_data bcm59055_h6ldo_data = {
	.constraints = {
			.name = "h6ldo",
			.min_uV = 1800000,
			.max_uV = 3300000,
			.valid_ops_mask = REGULATOR_CHANGE_STATUS |
			REGULATOR_CHANGE_VOLTAGE,
			.always_on = 0,
			.boot_on = 0,
			},
	.num_consumer_supplies = ARRAY_SIZE(h6ldo_supply),
	.consumer_supplies = h6ldo_supply,
};

static struct bcm590xx_regulator_init_data bcm59055_regulators[] = {
	{BCM59055_SIMLDO, &bcm59055_simldo_data, BCM590XX_REGL_LPM_IN_DSM},
	{BCM59055_HV6LDO, &bcm59055_h6ldo_data, BCM590XX_REGL_OFF_IN_DSM},
};

static struct bcm590xx_regulator_pdata bcm59055_regl_pdata = {
	.num_regulator = ARRAY_SIZE(bcm59055_regulators),
	.init = bcm59055_regulators,
	.default_pmmode = {
			   [BCM59055_RFLDO] = 0x00,
			   [BCM59055_CAMLDO] = 0x00,
			   [BCM59055_HV1LDO] = 0x00,
			   [BCM59055_HV2LDO] = 0x00,
			   [BCM59055_HV3LDO] = 0x00,
			   [BCM59055_HV4LDO] = 0x00,
			   [BCM59055_HV5LDO] = 0x00,
			   [BCM59055_HV6LDO] = 0x00,
			   [BCM59055_HV7LDO] = 0x00,
			   [BCM59055_SIMLDO] = 0x00,
			   [BCM59055_CSR] = 0x31,
			   [BCM59055_IOSR] = 0x00,
			   [BCM59055_SDSR] = 0x00,
			   },
/*Set default values for SS silicon type*/
	.csr_nm_volt = 0x14,
	.csr_lpm_volt = 4,
	.csr_turbo_volt = 0x14,
};

/* Register userspace and virtual consumer for SIMLDO */
#ifdef CONFIG_REGULATOR_USERSPACE_CONSUMER
static struct regulator_bulk_data bcm59055_bd_sim = {
	.supply = "simldo_uc",
};

static struct regulator_userspace_consumer_data bcm59055_uc_data_sim = {
	.name = "simldo",
	.num_supplies = 1,
	.supplies = &bcm59055_bd_sim,
	.init_on = 0
};

static struct platform_device bcm59055_uc_device_sim = {
	.name = "reg-userspace-consumer",
	.id = BCM59055_SIMLDO,
	.dev = {
		.platform_data = &bcm59055_uc_data_sim,
		},
};
#endif
#ifdef CONFIG_REGULATOR_VIRTUAL_CONSUMER
static struct platform_device bcm59055_vc_device_sim = {
	.name = "reg-virt-consumer",
	.id = BCM59055_SIMLDO,
	.dev = {
		.platform_data = "simldo_uc"},
};
#endif
#endif

static const char *pmu_clients[] = {
#ifdef CONFIG_BCM59055_WATCHDOG
	"bcm59055-wdog",
#endif
	"bcmpmu_usb",
#ifdef CONFIG_INPUT_BCM59055_ONKEY
	"bcm590xx-onkey",
#endif
#ifdef CONFIG_BCM59055_FUELGAUGE
	"bcm590xx-fg",
#endif
#ifdef CONFIG_BCM59055_SARADC
	"bcm590xx-saradc",
#endif
#ifdef CONFIG_REGULATOR_BCM_PMU59055
	"bcm590xx-regulator",
#endif
#ifdef CONFIG_BCM59055_AUDIO
	"bcm590xx-audio",
#endif
#ifdef CONFIG_RTC_DRV_BCM59055
	"bcm59055-rtc",
#endif
#ifdef CONFIG_BATTERY_BCM59055
	"bcm590xx-power",
#endif
#ifdef CONFIG_BCM59055_ADC_CHIPSET_API
	"bcm59055-adc_chipset_api",
#endif
#ifdef CONFIG_BCMPMU_OTG_XCEIV
	"bcmpmu_otg_xceiv",
#endif
#ifdef CONFIG_BCM59055_SELFTEST
	"bcm59055-selftest",
#endif
};

static struct bcm590xx_platform_data bcm590xx_plat_data = {
#ifdef CONFIG_KONA_PMU_BSC_HS_MODE
	/*
	 * PMU in High Speed (HS) mode. I2C CLK is 3.25MHz
	 * derived from 26MHz input clock.
	 *
	 * Rhea: PMBSC is always in HS mode, i2c_pdata is not in use.
	 */
	.i2c_pdata = {ADD_I2C_SLAVE_SPEED(BSC_BUS_SPEED_HS),},
#else
	.i2c_pdata = {ADD_I2C_SLAVE_SPEED(BSC_BUS_SPEED_400K),},
#endif
	.pmu_event_cb = bcm590xx_event_callback,
#ifdef CONFIG_BATTERY_BCM59055
	.battery_pdata = &bcm590xx_battery_plat_data,
#endif
#ifdef CONFIG_REGULATOR_BCM_PMU59055
	.regl_pdata = &bcm59055_regl_pdata,
#endif
	.clients = pmu_clients,
	.clients_num = ARRAY_SIZE(pmu_clients),
};

static struct i2c_board_info __initdata pmu_info[] = {
	{
	 I2C_BOARD_INFO("bcm59055", PMU_DEVICE_I2C_ADDR_0),
	 .irq = gpio_to_irq(PMU_IRQ_PIN),
	 .platform_data = &bcm590xx_plat_data,
	 },
};
#endif

#ifdef CONFIG_MFD_BCM59039
struct regulator_consumer_supply hv6_supply[] = {
	{.supply = "vdd_sdxc"},
};

struct regulator_consumer_supply hv4_supply[] = {
	{.supply = "hv4"},
	{.supply = "2v9_vibra"},
	{.supply = "vdd_sdio"},
};
#endif

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
	GPIO08, GPIO09, GPIO10 and GPIO11, SSP3 and camera used GPIO06,
	GPIO07, GPIO12, GPIO13, for now keypad can only be set as a 2x2 matrix
	by using pin GPIO04, GPIO05, GPIO14 and GPIO15 */
static struct bcm_keymap newKeymap[] = {
#ifndef CONFIG_MACH_RHEA_RAY_DEMO
	{BCM_KEY_ROW_0, BCM_KEY_COL_0, "Search Key", KEY_SEARCH},
	{BCM_KEY_ROW_0, BCM_KEY_COL_1, "Back Key", KEY_BACK},
	{BCM_KEY_ROW_0, BCM_KEY_COL_2, "Menu-Key", KEY_MENU},
	{BCM_KEY_ROW_0, BCM_KEY_COL_3, "Home-Key", KEY_HOME},
#else
	{BCM_KEY_ROW_0, BCM_KEY_COL_0, "unused", 0},
	{BCM_KEY_ROW_0, BCM_KEY_COL_1, "unused", 0},
	{BCM_KEY_ROW_0, BCM_KEY_COL_2, "unused", 0},
	{BCM_KEY_ROW_0, BCM_KEY_COL_3, "unused", 0},
#endif
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
	{BCM_KEY_ROW_4, BCM_KEY_COL_4, "unused", 0},
	{BCM_KEY_ROW_4, BCM_KEY_COL_5, "unused", 0},
	{BCM_KEY_ROW_4, BCM_KEY_COL_6, "unused", 0},
	{BCM_KEY_ROW_4, BCM_KEY_COL_7, "unused", 0},
	{BCM_KEY_ROW_5, BCM_KEY_COL_0, "unused", 0},
	{BCM_KEY_ROW_5, BCM_KEY_COL_1, "unused", 0},
	{BCM_KEY_ROW_5, BCM_KEY_COL_2, "unused", 0},
	{BCM_KEY_ROW_5, BCM_KEY_COL_3, "unused", 0},
	{BCM_KEY_ROW_5, BCM_KEY_COL_4, "unused", 0},
	{BCM_KEY_ROW_5, BCM_KEY_COL_5, "unused", 0},
	{BCM_KEY_ROW_5, BCM_KEY_COL_6, "unused", 0},
	{BCM_KEY_ROW_5, BCM_KEY_COL_7, "unused", 0},
	{BCM_KEY_ROW_6, BCM_KEY_COL_0, "unused", 0},
	{BCM_KEY_ROW_6, BCM_KEY_COL_1, "unused", 0},
	{BCM_KEY_ROW_6, BCM_KEY_COL_2, "unused", 0},
	{BCM_KEY_ROW_6, BCM_KEY_COL_3, "unused", 0},
	{BCM_KEY_ROW_6, BCM_KEY_COL_4, "unused", 0},
	{BCM_KEY_ROW_6, BCM_KEY_COL_5, "unused", 0},
#ifndef CONFIG_MACH_RHEA_RAY_DEMO
	{BCM_KEY_ROW_6, BCM_KEY_COL_6, "unused", 0},
	{BCM_KEY_ROW_6, BCM_KEY_COL_7, "unused", 0},
#else
	{BCM_KEY_ROW_6, BCM_KEY_COL_6, "Search Key", KEY_SEARCH},
	{BCM_KEY_ROW_6, BCM_KEY_COL_7, "Back Key", KEY_BACK},
#endif
	{BCM_KEY_ROW_7, BCM_KEY_COL_0, "unused", 0},
	{BCM_KEY_ROW_7, BCM_KEY_COL_1, "unused", 0},
	{BCM_KEY_ROW_7, BCM_KEY_COL_2, "unused", 0},
	{BCM_KEY_ROW_7, BCM_KEY_COL_3, "unused", 0},
	{BCM_KEY_ROW_7, BCM_KEY_COL_4, "unused", 0},
	{BCM_KEY_ROW_7, BCM_KEY_COL_5, "unused", 0},
#ifndef CONFIG_MACH_RHEA_RAY_DEMO
	{BCM_KEY_ROW_7, BCM_KEY_COL_6, "unused", 0},
	{BCM_KEY_ROW_7, BCM_KEY_COL_7, "unused", 0},
#else
	{BCM_KEY_ROW_7, BCM_KEY_COL_6, "Menu-Key", KEY_MENU},
	{BCM_KEY_ROW_7, BCM_KEY_COL_7, "Home-Key", KEY_HOME},
#endif
};

static struct bcm_keypad_platform_info bcm_keypad_data = {
	.row_num = 8,
	.col_num = 8,
	.keymap = newKeymap,
	.bcm_keypad_base = (void *)__iomem HW_IO_PHYS_TO_VIRT(KEYPAD_BASE_ADDR),
};

#elif defined CONFIG_INPUT_GPIO

static void bcm_keypad_config_iocr(int row, int col)
{
	unsigned row_width, col_width, val;

	/* Calculate the row width and the column width to be programmed */
	row_width = ((row - 1) << KEYPAD_KPCR_ROWWIDTH_SHIFT) &
			KEYPAD_KPCR_ROWWIDTH_MASK;
	col_width = ((col - 1) << KEYPAD_KPCR_COLUMNWIDTH_SHIFT) &
			KEYPAD_KPCR_COLUMNWIDTH_MASK;

	/* First enable the Keypad Control and then configure the remaining
	 * values */
	writel((KEYPAD_KPCR_ENABLE_MASK << KEYPAD_KPCR_ENABLE_SHIFT),
		KONA_KEYPAD_VA + KEYPAD_KPCR_OFFSET);

	/* set IOMODE(bit 3 ) to enable GPIO mode for Keypad
	 * Set SwapRowColumn bit in KPCR to enable swap of row/column
	 * Enable Status Filtering and configure row-width & column width
	 * (as row -1 & col -1)*/
	val = readl(KONA_KEYPAD_VA + KEYPAD_KPCR_OFFSET) | row_width | col_width
			| KEYPAD_KPCR_IOMODE_MASK |
			KEYPAD_KPCR_STATUSFILTERENABLE_MASK;

	writel(val, KONA_KEYPAD_VA + KEYPAD_KPCR_OFFSET);

	/* Set all the row GPIOs as output and all column GPIOs as input */
	writel((KEYPAD_KPIOR_ROWOCONTRL_MASK &
		~(KEYPAD_KPIOR_COLUMNOCONTRL_MASK)),
		KONA_KEYPAD_VA + KEYPAD_KPIOR_OFFSET);
}

static unsigned int rhea_row_gpios[] = {0, 1, 2, 3, 4, 5, 6, 7};
static unsigned int rhea_col_gpios[] = {8, 9, 10, 11, 12, 13, 14, 15};
#define KEYMAP_INDEX(row, col)	(((col) * ARRAY_SIZE(rhea_col_gpios)) + (row))

static const unsigned short
	newKeymap[ARRAY_SIZE(rhea_row_gpios) * ARRAY_SIZE(rhea_col_gpios)] = {
	[KEYMAP_INDEX(BCM_KEY_ROW_0, BCM_KEY_COL_0)] =	KEY_Y,
	[KEYMAP_INDEX(BCM_KEY_ROW_0, BCM_KEY_COL_1)] =	KEY_T,
	[KEYMAP_INDEX(BCM_KEY_ROW_0, BCM_KEY_COL_2)] =  KEY_R,
	[KEYMAP_INDEX(BCM_KEY_ROW_0, BCM_KEY_COL_3)] =	KEY_E,
	[KEYMAP_INDEX(BCM_KEY_ROW_0, BCM_KEY_COL_4)] =  KEY_W,
	[KEYMAP_INDEX(BCM_KEY_ROW_0, BCM_KEY_COL_5)] =  KEY_Q,
	[KEYMAP_INDEX(BCM_KEY_ROW_0, BCM_KEY_COL_6)] =  KEY_RESERVED,
	[KEYMAP_INDEX(BCM_KEY_ROW_1, BCM_KEY_COL_0)] =	KEY_H,
	[KEYMAP_INDEX(BCM_KEY_ROW_1, BCM_KEY_COL_1)] =	KEY_G,
	[KEYMAP_INDEX(BCM_KEY_ROW_1, BCM_KEY_COL_2)] =	KEY_F,
	[KEYMAP_INDEX(BCM_KEY_ROW_1, BCM_KEY_COL_3)] =	KEY_D,
	[KEYMAP_INDEX(BCM_KEY_ROW_1, BCM_KEY_COL_4)] =	KEY_S,
	[KEYMAP_INDEX(BCM_KEY_ROW_1, BCM_KEY_COL_5)] =	KEY_A,
	[KEYMAP_INDEX(BCM_KEY_ROW_1, BCM_KEY_COL_6)] =	KEY_CAMERA,
	[KEYMAP_INDEX(BCM_KEY_ROW_2, BCM_KEY_COL_0)] =	KEY_B,
	[KEYMAP_INDEX(BCM_KEY_ROW_2, BCM_KEY_COL_1)] =	KEY_V,
	[KEYMAP_INDEX(BCM_KEY_ROW_2, BCM_KEY_COL_2)] =	KEY_C,
	[KEYMAP_INDEX(BCM_KEY_ROW_2, BCM_KEY_COL_3)] =	KEY_X,
	[KEYMAP_INDEX(BCM_KEY_ROW_2, BCM_KEY_COL_4)] =	KEY_Z,
	[KEYMAP_INDEX(BCM_KEY_ROW_2, BCM_KEY_COL_5)] =	KEY_LEFTSHIFT,
	[KEYMAP_INDEX(BCM_KEY_ROW_2, BCM_KEY_COL_6)] =	KEY_VOLUMEUP,
	[KEYMAP_INDEX(BCM_KEY_ROW_3, BCM_KEY_COL_0)] =	KEY_RESERVED,
	[KEYMAP_INDEX(BCM_KEY_ROW_3, BCM_KEY_COL_1)] =	KEY_SPACE,
	[KEYMAP_INDEX(BCM_KEY_ROW_3, BCM_KEY_COL_2)] =	KEY_SPACE,
	[KEYMAP_INDEX(BCM_KEY_ROW_3, BCM_KEY_COL_3)] =	KEY_RESERVED,
	[KEYMAP_INDEX(BCM_KEY_ROW_3, BCM_KEY_COL_4)] =	KEY_MENU,
	[KEYMAP_INDEX(BCM_KEY_ROW_3, BCM_KEY_COL_5)] =	KEY_HOME,
	[KEYMAP_INDEX(BCM_KEY_ROW_3, BCM_KEY_COL_6)] =	KEY_VOLUMEDOWN,
	[KEYMAP_INDEX(BCM_KEY_ROW_4, BCM_KEY_COL_0)] =	KEY_BACK,
	[KEYMAP_INDEX(BCM_KEY_ROW_4, BCM_KEY_COL_1)] =	KEY_DOT,
	[KEYMAP_INDEX(BCM_KEY_ROW_4, BCM_KEY_COL_2)] =	KEY_P,
	[KEYMAP_INDEX(BCM_KEY_ROW_4, BCM_KEY_COL_3)] =	KEY_O,
	[KEYMAP_INDEX(BCM_KEY_ROW_4, BCM_KEY_COL_4)] =	KEY_I,
	[KEYMAP_INDEX(BCM_KEY_ROW_4, BCM_KEY_COL_5)] =	KEY_U,
	[KEYMAP_INDEX(BCM_KEY_ROW_4, BCM_KEY_COL_6)] =  KEY_RESERVED,
	[KEYMAP_INDEX(BCM_KEY_ROW_5, BCM_KEY_COL_0)] =	KEY_SEND,
	[KEYMAP_INDEX(BCM_KEY_ROW_5, BCM_KEY_COL_1)] =	KEY_SEMICOLON,
	[KEYMAP_INDEX(BCM_KEY_ROW_5, BCM_KEY_COL_2)] =	KEY_BACKSPACE,
	[KEYMAP_INDEX(BCM_KEY_ROW_5, BCM_KEY_COL_3)] =	KEY_L,
	[KEYMAP_INDEX(BCM_KEY_ROW_5, BCM_KEY_COL_4)] =	KEY_K,
	[KEYMAP_INDEX(BCM_KEY_ROW_5, BCM_KEY_COL_5)] =	KEY_J,
	[KEYMAP_INDEX(BCM_KEY_ROW_5, BCM_KEY_COL_6)] =  KEY_RESERVED,
	[KEYMAP_INDEX(BCM_KEY_ROW_6, BCM_KEY_COL_0)] =	KEY_RESERVED,
	[KEYMAP_INDEX(BCM_KEY_ROW_6, BCM_KEY_COL_1)] =	KEY_RESERVED,
	[KEYMAP_INDEX(BCM_KEY_ROW_6, BCM_KEY_COL_2)] =	KEY_ENTER,
	[KEYMAP_INDEX(BCM_KEY_ROW_6, BCM_KEY_COL_3)] =	KEY_COMMA,
	[KEYMAP_INDEX(BCM_KEY_ROW_6, BCM_KEY_COL_4)] =	KEY_M,
	[KEYMAP_INDEX(BCM_KEY_ROW_6, BCM_KEY_COL_5)] =	KEY_N,
	[KEYMAP_INDEX(BCM_KEY_ROW_6, BCM_KEY_COL_6)] =	KEY_RESERVED,
};

static struct gpio_event_matrix_info rhea_keypad_matrix_info = {
	.info.func = gpio_event_matrix_func,
	.keymap = newKeymap,
	.output_gpios = rhea_col_gpios,
	.input_gpios = rhea_row_gpios,
	.noutputs = ARRAY_SIZE(rhea_col_gpios),
	.ninputs = ARRAY_SIZE(rhea_row_gpios),
	.settle_time.tv64 = 40 * NSEC_PER_USEC,
	.poll_time.tv64 = 20 * NSEC_PER_MSEC,
	.debounce_delay.tv64 = 5 * NSEC_PER_MSEC,
	.flags =  GPIOKPF_REMOVE_PHANTOM_KEYS | GPIOKPF_PRINT_MAPPED_KEYS,

};
static struct gpio_event_info *rhea_keypad_info[] = {
	&rhea_keypad_matrix_info.info,
};

static struct gpio_event_platform_data rhea_keypad_data = {
	.name = "bcm_keypad",
	.info = rhea_keypad_info,
	.info_count = ARRAY_SIZE(rhea_keypad_info),
};

static struct platform_device rhea_keypad_device = {
	.name = GPIO_EVENT_DEV_NAME,
	.id = 0,
	.dev		= {
		.platform_data	= &rhea_keypad_data,
	},
};

#endif

#ifdef CONFIG_GPIO_PCA953X

#if defined(CONFIG_MACH_RHEA_RAY_EDN1X) || defined(CONFIG_MACH_RHEA_RAY_EDN2X)
#define GPIO_PCA953X_GPIO_PIN      121	/* Configure pad MMC1DAT4 to GPIO74 */
#define GPIO_PCA953X_2_GPIO_PIN      122	/* Configure ICUSBDM pad to GPIO122 */
#else
#define GPIO_PCA953X_GPIO_PIN      74	/* Configure pad MMC1DAT4 to GPIO74 */
#endif

#define SENSOR_0_GPIO_PWRDN		12
#define SENSOR_0_GPIO_RST		(KONA_MAX_GPIO + 10)
#define SENSOR_0_CLK			"dig_ch0_clk"
#define SENSOR_0_CLK_FREQ		(13000000)

#define SENSOR_1_GPIO_PWRDN		13
#define SENSOR_1_CLK			"dig_ch0_clk"

static int pca953x_platform_init_hw(struct i2c_client *client,
				    unsigned gpio, unsigned ngpio,
				    void *context)
{
	int rc;
	rc = gpio_request(GPIO_PCA953X_GPIO_PIN, "gpio_expander");
	if (rc < 0) {
		printk(KERN_ERR "unable to request GPIO pin %d\n",
		       GPIO_PCA953X_GPIO_PIN);
		return rc;
	}
	gpio_direction_input(GPIO_PCA953X_GPIO_PIN);

	/*init sensor gpio here to be off */
	rc = gpio_request(SENSOR_0_GPIO_PWRDN, "CAM_STANDBY0");
	if (rc < 0)
		printk(KERN_ERR "unable to request GPIO pin %d\n",
		       SENSOR_0_GPIO_PWRDN);

	gpio_direction_output(SENSOR_0_GPIO_PWRDN, 0);
	gpio_set_value(SENSOR_0_GPIO_PWRDN, 0);

	rc = gpio_request(SENSOR_0_GPIO_RST, "CAM_RESET0");
	if (rc < 0)
		printk(KERN_ERR "unable to request GPIO pin %d\n",
		       SENSOR_0_GPIO_RST);

	gpio_direction_output(SENSOR_0_GPIO_RST, 0);
	gpio_set_value(SENSOR_0_GPIO_RST, 0);

	rc = gpio_request(SENSOR_1_GPIO_PWRDN, "CAM_STANDBY1");
	if (rc < 0)
		printk(KERN_ERR "unable to request GPIO pin %d\n",
		       SENSOR_1_GPIO_PWRDN);

	gpio_direction_output(SENSOR_1_GPIO_PWRDN, 0);
	gpio_set_value(SENSOR_1_GPIO_PWRDN, 0);

	return 0;
}

static int pca953x_platform_exit_hw(struct i2c_client *client,
				    unsigned gpio, unsigned ngpio,
				    void *context)
{
	gpio_free(GPIO_PCA953X_GPIO_PIN);
	return 0;
}

static struct pca953x_platform_data board_expander_info = {
	.i2c_pdata = {ADD_I2C_SLAVE_SPEED(BSC_BUS_SPEED_100K),},
	.gpio_base = KONA_MAX_GPIO,
	.irq_base = gpio_to_irq(KONA_MAX_GPIO),
	.setup = pca953x_platform_init_hw,
	.teardown = pca953x_platform_exit_hw,
};

static struct i2c_board_info __initdata pca953x_info[] = {
	{
	 I2C_BOARD_INFO("pca9539", 0x74),
	 .irq = gpio_to_irq(GPIO_PCA953X_GPIO_PIN),
	 .platform_data = &board_expander_info,
	 },
};

#ifdef CONFIG_MACH_RHEA_RAY_EDN1X
/* Expander #2 on RheaRay EDN1X*/
static int pca953x_2_platform_init_hw(struct i2c_client *client,
				      unsigned gpio, unsigned ngpio,
				      void *context)
{
	int rc;
	rc = gpio_request(GPIO_PCA953X_2_GPIO_PIN, "gpio_expander_2");
	if (rc < 0) {
		printk(KERN_ERR "unable to request GPIO pin %d\n",
		       GPIO_PCA953X_2_GPIO_PIN);
		return rc;
	}
	gpio_direction_input(GPIO_PCA953X_2_GPIO_PIN);
	return 0;
}

static int pca953x_2_platform_exit_hw(struct i2c_client *client,
				      unsigned gpio, unsigned ngpio,
				      void *context)
{
	gpio_free(GPIO_PCA953X_2_GPIO_PIN);
	return 0;
}

static struct pca953x_platform_data board_expander_2_info = {
	.i2c_pdata = {ADD_I2C_SLAVE_SPEED(BSC_BUS_SPEED_100K),},
	.gpio_base = KONA_MAX_GPIO + 16,
	.irq_base = gpio_to_irq(KONA_MAX_GPIO + 16),
	.setup = pca953x_2_platform_init_hw,
	.teardown = pca953x_2_platform_exit_hw,
};

static struct i2c_board_info __initdata pca953x_2_info[] = {
	{
	 I2C_BOARD_INFO("pca9539", 0x75),
	 .irq = gpio_to_irq(GPIO_PCA953X_2_GPIO_PIN),
	 .platform_data = &board_expander_2_info,
	 },
};
#endif
#endif /* CONFIG_GPIO_PCA953X */

#ifdef CONFIG_TOUCHSCREEN_QT602240
#ifdef CONFIG_GPIO_PCA953X
#define QT602240_INT_GPIO_PIN      (KONA_MAX_GPIO + 8)
#else
#define QT602240_INT_GPIO_PIN      74	/* skip expander chip */
#endif
static int qt602240_platform_init_hw(void)
{
	int rc;
	rc = gpio_request(QT602240_INT_GPIO_PIN, "ts_qt602240");
	if (rc < 0) {
		printk(KERN_ERR "unable to request GPIO pin %d\n",
		       QT602240_INT_GPIO_PIN);
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
	.i2c_pdata = {ADD_I2C_SLAVE_SPEED(BSC_BUS_SPEED_100K),},
	.x_line = 15,
	.y_line = 11,
	.x_size = 1023,
	.y_size = 1023,
	.x_min = 90,
	.y_min = 90,
	.x_max = 0x3ff,
	.y_max = 0x3ff,
	.max_area = 0xff,
	.blen = 33,
	.threshold = 70,
	.voltage = 2700000,	/* 2.8V */
	.orient = QT602240_DIAGONAL_COUNTER,
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
#ifdef CONFIG_BMP18X
static struct i2c_board_info __initdata bmp18x_info[] = {
	{
	 I2C_BOARD_INFO("bmp18x", 0x77),
	 /*.irq = */
	 },
};
#endif
#ifdef CONFIG_AL3006
#ifdef CONFIG_GPIO_PCA953X
#define AL3006_INT_GPIO_PIN		(KONA_MAX_GPIO + 16 + 6)
#else
#define AL3006_INT_GPIO_PIN		122	/*  skip expander chip */
#endif
static int al3006_platform_init_hw(void)
{
	int rc;
	rc = gpio_request(AL3006_INT_GPIO_PIN, "al3006");
	if (rc < 0) {
		printk(KERN_ERR "unable to request GPIO pin %d\n",
		       AL3006_INT_GPIO_PIN);
		return rc;
	}
	gpio_direction_input(AL3006_INT_GPIO_PIN);

	return 0;
}

static void al3006_platform_exit_hw(void)
{
	gpio_free(AL3006_INT_GPIO_PIN);
}

static struct al3006_platform_data al3006_platform_data = {
	.i2c_pdata = {ADD_I2C_SLAVE_SPEED(BSC_BUS_SPEED_100K),},
	.init_platform_hw = al3006_platform_init_hw,
	.exit_platform_hw = al3006_platform_exit_hw,
};

static struct i2c_board_info __initdata al3006_info[] = {
	{
	 I2C_BOARD_INFO("al3006", 0x1d),
	 .platform_data = &al3006_platform_data,
	 .irq = gpio_to_irq(AL3006_INT_GPIO_PIN),
	 },
};
#endif
#if (defined(CONFIG_MPU_SENSORS_MPU6050A2) || defined(CONFIG_MPU_SENSORS_MPU6050B1))
static struct mpu_platform_data mpu6050_data = {
	.int_config = 0x10,
	.orientation = {0, 1, 0,
			1, 0, 0,
			0, 0, -1},
	.level_shifter = 0,

#ifdef CONFIG_GPIO_PCA953X
	.accel = {
		  /*.get_slave_descr = mpu_get_slave_descr, */
		  .adapt_num = 2,
		  .bus = EXT_SLAVE_BUS_SECONDARY,
		  .address = 0x38,
		  .orientation = {0, 1, 0,
				  1, 0, 0,
				  0, 0, -1},
		  },
	.compass = {
		    /*.get_slave_descr = compass_get_slave_descr, */
		    .adapt_num = 2,
		    .bus = EXT_SLAVE_BUS_PRIMARY,
		    .address = (0x50 >> 1),
		    .orientation = {0, 1, 0,
				    1, 0, 0,
				    0, 0, -1},
		    },
#endif /* CONFIG_GPIO_PCA953X */
};

static struct i2c_board_info __initdata mpu6050_info[] = {
	{
	 I2C_BOARD_INFO("mpu6050", 0x68),
	 /*.irq = */
	 .platform_data = &mpu6050_data,
	 },
};
#endif

#ifdef CONFIG_KONA_HEADSET
#define HS_IRQ		gpio_to_irq(71)
#define HSB_IRQ		BCM_INT_ID_AUXMIC_COMP2
#define HSB_REL_IRQ 	BCM_INT_ID_AUXMIC_COMP2_INV
static struct kona_headset_pd headset_data = {
	/* GPIO state read is 0 on HS insert and 1 for
	 * HS remove
	 */

	.hs_default_state = 1,
	/*
	 * Because of the presence of the resistor in the MIC_IN line.
	 * The actual ground is not 0, but a small offset is added to it.
	 * This needs to be subtracted from the measured voltage to determine the
	 * correct value. This will vary for different HW based on the resistor
	 * values used.
	 *
	 * What this means to Rhearay?
	 * From the schematics looks like there is no such resistor put on
	 * Rhearay. That means technically there is no need to subtract any extra load
	 * from the read Voltages. On other HW, if there is a resistor present
	 * on this line, please measure the load value and put it here.
	 */
	.phone_ref_offset = 0,
};

static struct resource board_headset_resource[] = {
	{			/* For AUXMIC */
	 .start = AUXMIC_BASE_ADDR,
	 .end = AUXMIC_BASE_ADDR + SZ_4K - 1,
	 .flags = IORESOURCE_MEM,
	 },
	{			/* For ACI */
	 .start = ACI_BASE_ADDR,
	 .end = ACI_BASE_ADDR + SZ_4K - 1,
	 .flags = IORESOURCE_MEM,
	 },
	{			/* For Headset IRQ */
	 .start = HS_IRQ,
	 .end = HS_IRQ,
	 .flags = IORESOURCE_IRQ,
	 },
	{			/* For Headset button  press IRQ */
	 .start = HSB_IRQ,
	 .end = HSB_IRQ,
	 .flags = IORESOURCE_IRQ,
	 },
	{			/* For Headset button  release IRQ */
	 .start = HSB_REL_IRQ,
	 .end = HSB_REL_IRQ,
	 .flags = IORESOURCE_IRQ,
	 },

};

struct platform_device headset_device = {
	.name = "konaaciheadset",
	.id = -1,
	.resource = board_headset_resource,
	.num_resources = ARRAY_SIZE(board_headset_resource),
	.dev = {
		.platform_data = &headset_data,
		},
};
#endif /* CONFIG_KONA_HEADSET */

#ifdef CONFIG_DMAC_PL330
static struct kona_pl330_data rhea_pl330_pdata = {
	/* Non Secure DMAC virtual base address */
	.dmac_ns_base = KONA_DMAC_NS_VA,
	/* Secure DMAC virtual base address */
	.dmac_s_base = KONA_DMAC_S_VA,
	/* # of PL330 dmac channels 'configurable' */
	.num_pl330_chans = 8,
	/* irq number to use */
	.irq_base = BCM_INT_ID_RESERVED184,
	/* # of PL330 Interrupt lines connected to GIC */
	.irq_line_count = 8,
};

static struct platform_device pl330_dmac_device = {
	.name = "kona-dmac-pl330",
	.id = 0,
	.dev = {
		.platform_data = &rhea_pl330_pdata,
		.coherent_dma_mask = DMA_BIT_MASK(64),
		},
};
#endif

/*
 * SPI board info for the slaves
 */
static struct spi_board_info spi_slave_board_info[] __initdata = {
#ifdef CONFIG_SPI_SPIDEV
	{
	 .modalias = "spidev",	/* use spidev generic driver */
	 .max_speed_hz = 13000000,	/* use max speed */
	 .bus_num = 0,		/* framework bus number */
	 .chip_select = 0,	/* for each slave */
	 .platform_data = NULL,	/* no spi_driver specific */
	 .irq = 0,		/* IRQ for this device */
	 .mode = SPI_LOOP,	/* SPI mode 0 */
	 },
#endif
	/* TODO: adding more slaves here */
};

#if defined (CONFIG_HAPTIC_SAMSUNG_PWM)
void haptic_gpio_setup(void)
{
	/* Board specific configuration like pin mux & GPIO */
}

static struct haptic_platform_data haptic_control_data = {
	/* Haptic device name: can be device-specific name like ISA1000 */
	.name = "pwm_vibra",
	/* PWM interface name to request */
	.pwm_name = "kona_pwmc:4",
	/* Invalid gpio for now, pass valid gpio number if connected */
	.gpio = ARCH_NR_GPIOS,
	.setup_pin = haptic_gpio_setup,
};

struct platform_device haptic_pwm_device = {
	.name = "samsung_pwm_haptic",
	.id = -1,
	.dev = {.platform_data = &haptic_control_data,}
};

#endif /* CONFIG_HAPTIC_SAMSUNG_PWM */

#if 1
static struct resource board_sdio1_resource[] = {
	[0] = {
	       .start = SDIO1_BASE_ADDR,
	       .end = SDIO1_BASE_ADDR + SZ_64K - 1,
	       .flags = IORESOURCE_MEM,
	       },
	[1] = {
	       .start = BCM_INT_ID_SDIO0,
	       .end = BCM_INT_ID_SDIO0,
	       .flags = IORESOURCE_IRQ,
	       },
};

static struct resource board_sdio2_resource[] = {
	[0] = {
	       .start = SDIO2_BASE_ADDR,
	       .end = SDIO2_BASE_ADDR + SZ_64K - 1,
	       .flags = IORESOURCE_MEM,
	       },
	[1] = {
	       .start = BCM_INT_ID_SDIO1,
	       .end = BCM_INT_ID_SDIO1,
	       .flags = IORESOURCE_IRQ,
	       },
};

#ifdef CONFIG_MACH_RHEA_RAY_EDN1X
static struct resource board_sdio3_resource[] = {
	[0] = {
	       .start = SDIO3_BASE_ADDR,
	       .end = SDIO3_BASE_ADDR + SZ_64K - 1,
	       .flags = IORESOURCE_MEM,
	       },
	[1] = {
	       .start = BCM_INT_ID_SDIO_NAND,
	       .end = BCM_INT_ID_SDIO_NAND,
	       .flags = IORESOURCE_IRQ,
	       },
};
#endif

#ifdef CONFIG_MACH_RHEA_RAY_EDN2X
static struct resource board_sdio3_resource[] = {
	[0] = {
	       .start = SDIO3_BASE_ADDR,
	       .end = SDIO3_BASE_ADDR + SZ_64K - 1,
	       .flags = IORESOURCE_MEM,
	       },
	[1] = {
	       .start = BCM_INT_ID_SDIO_NAND,
	       .end = BCM_INT_ID_SDIO_NAND,
	       .flags = IORESOURCE_IRQ,
	       },
};
#endif

#ifdef CONFIG_MACH_RHEA_RAY_EDN2XT
static struct resource board_sdio4_resource[] = {
	[0] = {
	       .start = SDIO4_BASE_ADDR,
	       .end = SDIO4_BASE_ADDR + SZ_64K - 1,
	       .flags = IORESOURCE_MEM,
	       },
	[1] = {
	       .start = BCM_INT_ID_SDIO_MMC,
	       .end = BCM_INT_ID_SDIO_MMC,
	       .flags = IORESOURCE_IRQ,
	       },
};
#endif

static struct sdio_platform_cfg board_sdio_param[] = {
	{			/* SDIO1 */
	 .id = 0,
	 .data_pullup = 0,
	 .cd_gpio = SD_CARDDET_GPIO_PIN,
	 .devtype = SDIO_DEV_TYPE_SDMMC,
	 .flags = KONA_SDIO_FLAGS_DEVICE_REMOVABLE,
	 .peri_clk_name = "sdio1_clk",
	 .ahb_clk_name = "sdio1_ahb_clk",
	 .sleep_clk_name = "sdio1_sleep_clk",
	 .peri_clk_rate = 48000000,
	 /*The SD card regulator*/
	 .vddo_regulator_name = "vdd_sdio",
	 /*The SD controller regulator*/
	 .vddsdxc_regulator_name = "vdd_sdxc",
	 },
	{			/* SDIO2 */
	 .id = 1,
	 .data_pullup = 0,
	 .is_8bit = 0,
	 .devtype = SDIO_DEV_TYPE_EMMC,
	 .flags = KONA_SDIO_FLAGS_DEVICE_NON_REMOVABLE,
	 .peri_clk_name = "sdio2_clk",
	 .ahb_clk_name = "sdio2_ahb_clk",
	 .sleep_clk_name = "sdio2_sleep_clk",
	 .peri_clk_rate = 52000000,
	 },
#ifdef CONFIG_MACH_RHEA_RAY_EDN1X
	{			/* SDIO3 */
	 .id = 2,
	 .data_pullup = 0,
	 .devtype = SDIO_DEV_TYPE_WIFI,
	 .wifi_gpio = {
		       .reset = 70,
		       .reg = -1,
		       .host_wake = -1,
		       .shutdown = -1,
		       },
	 .flags = KONA_SDIO_FLAGS_DEVICE_NON_REMOVABLE,
	 .peri_clk_name = "sdio3_clk",
	 .ahb_clk_name = "sdio3_ahb_clk",
	 .sleep_clk_name = "sdio3_sleep_clk",
	 .peri_clk_rate = 48000000,
	 },
#endif

#ifdef CONFIG_MACH_RHEA_RAY_EDN2X

#ifdef CONFIG_BRCM_UNIFIED_DHD_SUPPORT

	{			/* SDIO3 */
	 .id = 2,
	 .data_pullup = 0,
	 .devtype = SDIO_DEV_TYPE_WIFI,
	 .flags = KONA_SDIO_FLAGS_DEVICE_REMOVABLE,
	 .peri_clk_name = "sdio3_clk",
	 .ahb_clk_name = "sdio3_ahb_clk",
	 .sleep_clk_name = "sdio3_sleep_clk",
	 .peri_clk_rate = 48000000,
	 .register_status_notify = rhea_wifi_status_register,
	 },

#else

	{			/* SDIO3 */
	 .id = 2,
	 .data_pullup = 0,
	 .devtype = SDIO_DEV_TYPE_WIFI,
	 .wifi_gpio = {
		       .reset = 70,
		       .reg = -1,
		       .host_wake = -1,
		       .shutdown = -1,
		       },
	 .flags = KONA_SDIO_FLAGS_DEVICE_REMOVABLE,
	 .peri_clk_name = "sdio3_clk",
	 .ahb_clk_name = "sdio3_ahb_clk",
	 .sleep_clk_name = "sdio3_sleep_clk",
	 .peri_clk_rate = 48000000,
	 },

#endif

#endif

#ifdef CONFIG_BCMDHD_43362_LINUX
	{			/* SDIO3 */
	 .id = 2,
	 .data_pullup = 0,
	 .devtype = SDIO_DEV_TYPE_WIFI,
	 .wifi_gpio = {
		       .reset = 70,
		       .reg = -1,
		       .host_wake = 85,
		       .shutdown = -1,
		       },
	 .flags = KONA_SDIO_FLAGS_DEVICE_NON_REMOVABLE,
	 .peri_clk_name = "sdio3_clk",
	 .ahb_clk_name = "sdio3_ahb_clk",
	 .sleep_clk_name = "sdio3_sleep_clk",
	 .peri_clk_rate = 48000000,
	 },
#endif

};

static struct platform_device board_sdio1_device = {
	.name = "sdhci",
	.id = 0,
	.resource = board_sdio1_resource,
	.num_resources = ARRAY_SIZE(board_sdio1_resource),
	.dev = {
		.platform_data = &board_sdio_param[0],
		},
};

static struct platform_device board_sdio2_device = {
	.name = "sdhci",
	.id = 1,
	.resource = board_sdio2_resource,
	.num_resources = ARRAY_SIZE(board_sdio2_resource),
	.dev = {
		.platform_data = &board_sdio_param[1],
		},
};

static struct platform_device board_sdio3_device = {
	.name = "sdhci",
	.id = 2,
#ifdef CONFIG_MACH_RHEA_RAY_EDN1X
	.resource = board_sdio3_resource,
	.num_resources = ARRAY_SIZE(board_sdio3_resource),
#endif
#ifdef CONFIG_MACH_RHEA_RAY_EDN2X
	.resource = board_sdio3_resource,
	.num_resources = ARRAY_SIZE(board_sdio3_resource),
#endif
	.dev = {
		.platform_data = &board_sdio_param[2],
		},
};

/* Common devices among all the Rhea boards (Rhea Ray, Rhea Berri, etc.) */
static struct platform_device *board_sdio_plat_devices[] __initdata = {
	&board_sdio2_device,
	&board_sdio3_device,
	&board_sdio1_device,
};

void __init board_add_sdio_devices(void)
{
	platform_add_devices(board_sdio_plat_devices,
			     ARRAY_SIZE(board_sdio_plat_devices));
}

#endif

#ifdef CONFIG_BACKLIGHT_PWM

static struct platform_pwm_backlight_data bcm_backlight_data = {
/* backlight */
	.pwm_name = "kona_pwmc:4",
	.max_brightness = 32,	/* Android calibrates to 32 levels */
	.dft_brightness = 32,
#ifdef CONFIG_MACH_RHEA_RAY_EDN1X
	.polarity = 1,		/* Inverted polarity */
#endif
	.pwm_period_ns = 5000000,
};

static struct platform_device bcm_backlight_devices = {
	.name = "pwm-backlight",
	.id = 0,
	.dev = {
		.platform_data = &bcm_backlight_data,
		},
};

#endif /*CONFIG_BACKLIGHT_PWM */

#if defined (CONFIG_REGULATOR_TPS728XX)
#if defined(CONFIG_MACH_RHEA_RAY) || defined(CONFIG_MACH_RHEA_RAY_EDN1X) \
	|| defined(CONFIG_MACH_RHEA_FARADAY_EB10) \
	|| defined(CONFIG_MACH_RHEA_DALTON2_EB30) \
	|| defined(CONFIG_MACH_RHEA_DALTON) || defined(CONFIG_MACH_RHEA_RAY_EDN2X) \
	|| defined(CONFIG_MACH_RHEA_RAY_DEMO)
#define GPIO_SIM2LDO_EN		99
#endif
#ifdef CONFIG_GPIO_PCA953X
#define GPIO_SIM2LDOVSET	(KONA_MAX_GPIO + 7)
#endif
#define TPS728XX_REGL_ID        (BCM59055_MAX_LDO + 0)
struct regulator_consumer_supply sim2_supply[] = {
	{.supply = "sim2_vcc"},
	{.supply = "sim2ldo_uc"},
};

static struct regulator_init_data tps728xx_regl_initdata = {
	.constraints = {
			.name = "sim2ldo",
			.min_uV = 1300000,
			.max_uV = 3300000,
			.valid_ops_mask = REGULATOR_CHANGE_STATUS |
			REGULATOR_CHANGE_VOLTAGE,
			.always_on = 0,
			.boot_on = 0,
			},
	.num_consumer_supplies = ARRAY_SIZE(sim2_supply),
	.consumer_supplies = sim2_supply,
};

struct tps728xx_plat_data tps728xx_data = {
	.gpio_vset = GPIO_SIM2LDOVSET,
	.gpio_en = GPIO_SIM2LDO_EN,
	.vout0 = 1800000,
	.vout1 = 3000000,
	.initdata = &tps728xx_regl_initdata,
};

struct platform_device tps728xx_device = {
	.name = "tps728xx-regulator",
	.id = -1,
	.dev = {
		.platform_data = &tps728xx_data,
		},
};

/* Register userspace and virtual consumer for SIM2LDO */
#ifdef CONFIG_REGULATOR_USERSPACE_CONSUMER
static struct regulator_bulk_data tps728xx_bd_sim2 = {
	.supply = "sim2ldo_uc",
};

static struct regulator_userspace_consumer_data tps728xx_uc_data_sim2 = {
	.name = "sim2ldo",
	.num_supplies = 1,
	.supplies = &tps728xx_bd_sim2,
	.init_on = 0
};

static struct platform_device tps728xx_uc_device_sim2 = {
	.name = "reg-userspace-consumer",
	.id = TPS728XX_REGL_ID,
	.dev = {
		.platform_data = &tps728xx_uc_data_sim2,
		},
};
#endif
#ifdef CONFIG_REGULATOR_VIRTUAL_CONSUMER
static struct platform_device tps728xx_vc_device_sim2 = {
	.name = "reg-virt-consumer",
	.id = TPS728XX_REGL_ID,
	.dev = {
		.platform_data = "sim2ldo_uc"},
};
#endif
#endif /* CONFIG_REGULATOR_TPS728XX */

#if 0
#ifdef CONFIG_FB_BRCM_RHEA
static struct kona_fb_platform_data alex_dsi_display_fb_data = {
	.get_dispdrv_func_tbl = &DISP_DRV_BCM91008_ALEX_GetFuncTable,
	.screen_width = 360,
	.screen_height = 640,
	.bytes_per_pixel = 4,
	.gpio = (KONA_MAX_GPIO + 3),
	.pixel_format = XRGB8888,
};

static struct platform_device alex_dsi_display_device = {
	.name = "rhea_fb",
	.id = 0,
	.dev = {
		.platform_data = &alex_dsi_display_fb_data,
		.dma_mask = (u64 *)~(u32)0,
		.coherent_dma_mask = ~(u32)0,
		},
};

static struct kona_fb_platform_data nt35582_smi16_display_fb_data = {
	.get_dispdrv_func_tbl = &DISP_DRV_NT35582_WVGA_SMI_GetFuncTable,
	.screen_width = 480,
	.screen_height = 800,
	.bytes_per_pixel = 2,
	.gpio = 41,
	.pixel_format = RGB565,
	.bus_width = 16,
};

static struct platform_device nt35582_smi16_display_device = {
	.name = "rhea_fb",
	.id = 1,
	.dev = {
		.platform_data = &nt35582_smi16_display_fb_data,
		.dma_mask = (u64 *)~(u32)0,
		.coherent_dma_mask = ~(u32)0,
		},
};

static struct kona_fb_platform_data nt35582_smi8_display_fb_data = {
	.get_dispdrv_func_tbl = &DISP_DRV_NT35582_WVGA_SMI_GetFuncTable,
	.screen_width = 480,
	.screen_height = 800,
	.bytes_per_pixel = 2,
	.gpio = 41,
	.pixel_format = RGB565,
	.bus_width = 8,
};

static struct platform_device nt35582_smi8_display_device = {
	.name = "rhea_fb",
	.id = 2,
	.dev = {
		.platform_data = &nt35582_smi8_display_fb_data,
		.dma_mask = (u64 *)~(u32)0,
		.coherent_dma_mask = ~(u32)0,
		},
};

static struct kona_fb_platform_data r61581_smi16_display_fb_data = {
	.get_dispdrv_func_tbl = &DISP_DRV_R61581_HVGA_SMI_GetFuncTable,
	.screen_width = 320,
	.screen_height = 480,
	.bytes_per_pixel = 2,
	.gpio = 41,
	.pixel_format = RGB565,
	.bus_width = 16,
};

static struct platform_device r61581_smi16_display_device = {
	.name = "rhea_fb",
	.id = 3,
	.dev = {
		.platform_data = &r61581_smi16_display_fb_data,
		.dma_mask = (u64 *)~(u32)0,
		.coherent_dma_mask = ~(u32)0,
		},
};

static struct kona_fb_platform_data r61581_smi8_display_fb_data = {
	.get_dispdrv_func_tbl = &DISP_DRV_R61581_HVGA_SMI_GetFuncTable,
	.screen_width = 320,
	.screen_height = 480,
	.bytes_per_pixel = 2,
	.gpio = 41,
	.pixel_format = RGB565,
	.bus_width = 8,
};

static struct platform_device r61581_smi8_display_device = {
	.name = "rhea_fb",
	.id = 4,
	.dev = {
		.platform_data = &r61581_smi8_display_fb_data,
		.dma_mask = (u64 *)~(u32)0,
		.coherent_dma_mask = ~(u32)0,
		},
};
#endif
#endif

#if (defined(CONFIG_BCM_RFKILL) || defined(CONFIG_BCM_RFKILL_MODULE))

#define BCMBT_VREG_GPIO       (KONA_MAX_GPIO +4)
#define BCMBT_N_RESET_GPIO    (KONA_MAX_GPIO + 14)
#define BCMBT_AUX0_GPIO        (-1)	/* clk32 */
#define BCMBT_AUX1_GPIO        (-1)	/* UARTB_SEL */

static struct bcmbt_rfkill_platform_data board_bcmbt_rfkill_cfg = {
	.vreg_gpio = BCMBT_VREG_GPIO,
	.n_reset_gpio = BCMBT_N_RESET_GPIO,
	.aux0_gpio = BCMBT_AUX0_GPIO,	/* CLK32 */
	.aux1_gpio = BCMBT_AUX1_GPIO,	/* UARTB_SEL, probably not required */
};

static struct platform_device board_bcmbt_rfkill_device = {
	.name = "bcmbt-rfkill",
	.id = -1,
	.dev = {
		.platform_data = &board_bcmbt_rfkill_cfg,
		},
};
#endif

#ifdef CONFIG_BCM_BZHW
#define GPIO_BT_WAKE 04
#define GPIO_HOST_WAKE 111
static struct bcm_bzhw_platform_data bcm_bzhw_data = {
	.gpio_bt_wake = GPIO_BT_WAKE,
	.gpio_host_wake = GPIO_HOST_WAKE,
};

static struct platform_device board_bcm_bzhw_device = {
	.name = "bcm_bzhw",
	.id = -1,
	.dev = {
		.platform_data = &bcm_bzhw_data,
		},
};
#endif

#ifdef CONFIG_BCM_BT_LPM
#define GPIO_BT_WAKE 04
#define GPIO_HOST_WAKE 111

static struct bcm_bt_lpm_platform_data brcm_bt_lpm_data = {
	.gpio_bt_wake = GPIO_BT_WAKE,
	.gpio_host_wake = GPIO_HOST_WAKE,
};

static struct platform_device board_bcmbt_lpm_device = {
	.name = "bcmbt-lpm",
	.id = -1,
	.dev = {
		.platform_data = &brcm_bt_lpm_data,
		},
};
#endif

#define OV5640_I2C_ADDRESS (0x3C)

static struct i2c_board_info rhea_i2c_camera[] = {
	{
	 I2C_BOARD_INFO("ov5640", OV5640_I2C_ADDRESS),
	 },
};

static int rhea_camera_power(struct device *dev, int on)
{
#ifdef CONFIG_GPIO_PCA953X
	unsigned int value;
	int ret = -1;
	struct clk *clock;
	struct clk *axi_clk;
	static struct pi_mgr_dfs_node unicam_dfs_node;

	printk(KERN_INFO "%s:camera power %s\n", __func__, (on ? "on" : "off"));

	if (!unicam_dfs_node.valid) {
		ret =
		    pi_mgr_dfs_add_request(&unicam_dfs_node, "unicam",
					   PI_MGR_PI_ID_MM,
					   PI_MGR_DFS_MIN_VALUE);
		if (ret) {
			printk(KERN_ERR
			       "%s: failed to register PI DFS request\n",
			       __func__);
			return -1;
		}
	}

	clock = clk_get(NULL, SENSOR_0_CLK);
	if (IS_ERR_OR_NULL(clock)) {
		printk(KERN_ERR "%s: unable to get clock %s\n", __func__,
		       SENSOR_0_CLK);
		return -1;
	}

	axi_clk = clk_get(NULL, "csi0_axi_clk");
	if (IS_ERR_OR_NULL(axi_clk)) {
		printk(KERN_ERR "%s:unable to get clock csi0_axi_clk\n",
		       __func__);
		return -1;
	}

	if (on) {

		if (pi_mgr_dfs_request_update(&unicam_dfs_node, PI_OPP_TURBO)) {
			printk(KERN_ERR
			       "%s:failed to update dfs request for unicam\n",
			       __func__);
			return -1;
		}

		value = clk_enable(axi_clk);
		if (value) {
			printk(KERN_ERR "%s:failed to enable csi2 axi clock\n",
			       __func__);
			return -1;
		}

		/* enable clk */
		value = clk_enable(clock);
		if (value) {
			printk(KERN_ERR "%s: failed to enabled clock %s\n",
			       __func__, SENSOR_0_CLK);
			return -1;
		}
		value = clk_set_rate(clock, SENSOR_0_CLK_FREQ);
		if (value) {
			printk(KERN_ERR
			       "%s: failed to set the clock %s to freq %d\n",
			       __func__, SENSOR_0_CLK, SENSOR_0_CLK_FREQ);
			return -1;
		}

		/* Delay for clk to start */
		msleep(10);

		/* enable reset gpio */
		gpio_set_value(SENSOR_0_GPIO_RST, 0);
		msleep(10);

		/* disable power down gpio */
		gpio_set_value(SENSOR_0_GPIO_PWRDN, 1);
		msleep(5);

		/* disable reset gpio */
		gpio_set_value(SENSOR_0_GPIO_RST, 1);

		/* wait for sensor to come up */
		msleep(30);

	} else {
		/* enable reset gpio */
		gpio_set_value(SENSOR_0_GPIO_RST, 0);
		msleep(1);

		/* enable power down gpio */
		gpio_set_value(SENSOR_0_GPIO_PWRDN, 0);

		clk_disable(clock);

		clk_disable(axi_clk);

		if (pi_mgr_dfs_request_update
		    (&unicam_dfs_node, PI_MGR_DFS_MIN_VALUE)) {
			printk(KERN_ERR
			       "%s: failed to update dfs request for unicam\n",
			       __func__);
		}
	}

#endif /* CONFIG_GPIO_PCA953X */
	return 0;
}

static int rhea_camera_reset(struct device *dev)
{
	/* reset the camera gpio */
	printk(KERN_INFO "%s:camera reset\n", __func__);
	return 0;
}

static struct v4l2_subdev_sensor_interface_parms ov5640_if_params = {
	.if_type = V4L2_SUBDEV_SENSOR_SERIAL,
	.if_mode = V4L2_SUBDEV_SENSOR_MODE_SERIAL_CSI2,
	.orientation = V4L2_SUBDEV_SENSOR_PORTRAIT,
	.facing = V4L2_SUBDEV_SENSOR_BACK,
	.parms.serial = {
			 .lanes = 2,
			 .channel = 0,
			 .phy_rate = 0,
			 .pix_clk = 0},
};

static struct soc_camera_link iclink_ov5640 = {
	.bus_id = 0,
	.board_info = &rhea_i2c_camera[0],
	.i2c_adapter_id = 0,
	.module_name = "ov5640",
	.power = &rhea_camera_power,
	.reset = &rhea_camera_reset,
	.priv = &ov5640_if_params,
};

static struct platform_device rhea_camera = {
	.name = "soc-camera-pdrv",
	.id = 0,
	.dev = {
		.platform_data = &iclink_ov5640,
		},
};

#ifdef CONFIG_WD_TAPPER
static struct wd_tapper_platform_data wd_tapper_data = {
	/* Set the count to the time equivalent to the time-out in milliseconds
	 * required to pet the PMU watchdog to overcome the problem of reset in
	 * suspend*/
	.count = 28000,
	.ch_num = 1,
	.name = "aon-timer",
};

static struct platform_device wd_tapper = {
	.name = "wd_tapper",
	.id = 0,
	.dev = {
		.platform_data = &wd_tapper_data,
		},
};
#endif

/* Rhea Ray specific platform devices */
static struct platform_device *rhea_ray_plat_devices[] __initdata = {
#ifdef CONFIG_KEYBOARD_BCM
	&bcm_kp_device,
#elif defined CONFIG_INPUT_GPIO
	&rhea_keypad_device,
#endif

#ifdef CONFIG_KONA_HEADSET
	&headset_device,
#endif

#ifdef CONFIG_DMAC_PL330
	&pl330_dmac_device,
#endif
#ifdef CONFIG_HAPTIC_SAMSUNG_PWM
	&haptic_pwm_device,
#endif
#ifdef CONFIG_BACKLIGHT_PWM
	&bcm_backlight_devices,
#endif
/* TPS728XX device registration */
#ifdef CONFIG_REGULATOR_TPS728XX
	&tps728xx_device,
#endif

#if 0
#ifdef CONFIG_FB_BRCM_RHEA
	&alex_dsi_display_device,
	&nt35582_smi16_display_device,
	&nt35582_smi8_display_device,
	&r61581_smi8_display_device,
	&r61581_smi16_display_device,
#endif
#endif

#if (defined(CONFIG_BCM_RFKILL) || defined(CONFIG_BCM_RFKILL_MODULE))
	&board_bcmbt_rfkill_device,
#endif

#ifdef CONFIG_BCM_BZHW
	&board_bcm_bzhw_device,
#endif

#ifdef CONFIG_BCM_BT_LPM
	&board_bcmbt_lpm_device,
#endif
	&rhea_camera,

#ifdef CONFIG_WD_TAPPER
	&wd_tapper,
#endif

};

/* Add all userspace regulator consumer devices here */
#ifdef CONFIG_REGULATOR_USERSPACE_CONSUMER
struct platform_device *rhea_ray_userspace_consumer_devices[] __initdata = {
#if defined(CONFIG_REGULATOR_BCM_PMU59055) && defined(CONFIG_MFD_BCM_PMU590XX)
	&bcm59055_uc_device_sim,
#endif
#ifdef CONFIG_REGULATOR_TPS728XX
	&tps728xx_uc_device_sim2,
#endif
};
#endif

/* Add all virtual regulator consumer devices here */
#ifdef CONFIG_REGULATOR_VIRTUAL_CONSUMER
struct platform_device *rhea_ray_virtual_consumer_devices[] __initdata = {
#if defined(CONFIG_REGULATOR_BCM_PMU59055) && defined(CONFIG_MFD_BCM_PMU590XX)
	&bcm59055_vc_device_sim,
#endif
#ifdef CONFIG_REGULATOR_TPS728XX
	&tps728xx_vc_device_sim2,
#endif
};
#endif

/* Rhea Ray specific i2c devices */
static void __init rhea_ray_add_i2c_devices(void)
{
	/* 59055 on BSC - PMU */
#ifdef CONFIG_MFD_BCM_PMU590XX
	i2c_register_board_info(2, pmu_info, ARRAY_SIZE(pmu_info));
#endif

#ifdef CONFIG_GPIO_PCA953X
	i2c_register_board_info(1, pca953x_info, ARRAY_SIZE(pca953x_info));
#ifdef CONFIG_MACH_RHEA_RAY_EDN1X
	i2c_register_board_info(1, pca953x_2_info, ARRAY_SIZE(pca953x_2_info));
#endif
#endif
#ifdef CONFIG_TOUCHSCREEN_QT602240
	i2c_register_board_info(1, qt602240_info, ARRAY_SIZE(qt602240_info));
#endif
#ifdef CONFIG_BMP18X_I2C
	i2c_register_board_info(1, bmp18x_info, ARRAY_SIZE(bmp18x_info));
#endif
#ifdef CONFIG_AL3006
	i2c_register_board_info(1, al3006_info, ARRAY_SIZE(al3006_info));
#endif
#if (defined(CONFIG_MPU_SENSORS_MPU6050A2) || defined(CONFIG_MPU_SENSORS_MPU6050B1))
	i2c_register_board_info(1, mpu6050_info, ARRAY_SIZE(mpu6050_info));
#endif
}

static int __init rhea_ray_add_lateInit_devices(void)
{
	board_add_sdio_devices();
#ifdef CONFIG_BRCM_UNIFIED_DHD_SUPPORT

	printk(KERN_ERR "Calling WLAN_INIT!\n");

	rhea_wlan_init();
	printk(KERN_ERR "DONE WLAN_INIT!\n");
#endif
	return 0;
}

static void __init rhea_ray_reserve(void)
{
	board_common_reserve();
}

/* All Rhea Ray specific devices */
static void __init rhea_ray_add_devices(void)
{

#ifdef CONFIG_KEYBOARD_BCM
	bcm_kp_device.dev.platform_data = &bcm_keypad_data;
#endif
	platform_add_devices(rhea_ray_plat_devices,
			     ARRAY_SIZE(rhea_ray_plat_devices));

	rhea_ray_add_i2c_devices();
#ifdef CONFIG_REGULATOR_USERSPACE_CONSUMER
	platform_add_devices(rhea_ray_userspace_consumer_devices,
			     ARRAY_SIZE(rhea_ray_userspace_consumer_devices));
#endif
#ifdef CONFIG_REGULATOR_VIRTUAL_CONSUMER
	platform_add_devices(rhea_ray_virtual_consumer_devices,
			     ARRAY_SIZE(rhea_ray_virtual_consumer_devices));
#endif
	spi_register_board_info(spi_slave_board_info,
				ARRAY_SIZE(spi_slave_board_info));
}

#if 0
#ifdef CONFIG_FB_BRCM_RHEA
/*
 *   	     KONA FRAME BUFFER DISPLAY DRIVER PLATFORM CONFIG
 */
struct kona_fb_platform_data konafb_devices[] __initdata = {
	{
	 .dispdrv_name = "BCM91008_ALEX",
	 .dispdrv_entry = DISP_DRV_BCM91008_ALEX_GetFuncTable,
	 .parms = {
		   .w0 = {
			  .bits = {
				   .boot_mode = 0,
				   .bus_type = RHEA_BUS_DSI,
				   .bus_no = RHEA_BUS_0,
				   .bus_ch = RHEA_BUS_CH_0,
				   .bus_width = 0,
				   .te_input = RHEA_TE_IN_0_LCD,
				   .col_mode_i = RHEA_CM_I_XRGB888,
				   .col_mode_o = RHEA_CM_O_RGB888,
				   },
			  },
		   .w1 = {
			  .bits = {
				   .api_rev = RHEA_LCD_BOOT_API_REV,
				   .lcd_rst0 = (KONA_MAX_GPIO + 3),
				   },
			  },
		   },
	 },

	{
	 .dispdrv_name = "NT35582_WVGA_SMI",
	 .dispdrv_entry = DISP_DRV_NT35582_WVGA_SMI_GetFuncTable,
	 .parms = {
		   .w0 = {
			  .bits = {
				   .boot_mode = 0,
				   .bus_type = RHEA_BUS_SMI,
				   .bus_no = RHEA_BUS_0,
				   .bus_ch = RHEA_BUS_CH_0,
				   .bus_width = RHEA_BUS_WIDTH_16,
				   .te_input = RHEA_TE_IN_0_LCD,
				   .col_mode_i = RHEA_CM_I_RGB565,
				   .col_mode_o = RHEA_CM_O_RGB565,
				   },
			  },
		   .w1 = {
			  .bits = {
				   .api_rev = RHEA_LCD_BOOT_API_REV,
				   .lcd_rst0 = 41,
				   },
			  },
		   },
	 },

	{
	 .dispdrv_name = "NT35582_WVGA_SMI",
	 .dispdrv_entry = DISP_DRV_NT35582_WVGA_SMI_GetFuncTable,
	 .parms = {
		   .w0 = {
			  .bits = {
				   .boot_mode = 0,
				   .bus_type = RHEA_BUS_SMI,
				   .bus_no = RHEA_BUS_0,
				   .bus_ch = RHEA_BUS_CH_0,
				   .bus_width = RHEA_BUS_WIDTH_08,
				   .te_input = RHEA_TE_IN_0_LCD,
				   .col_mode_i = RHEA_CM_I_RGB565,
				   .col_mode_o = RHEA_CM_O_RGB565,
				   },
			  },
		   .w1 = {
			  .bits = {
				   .api_rev = RHEA_LCD_BOOT_API_REV,
				   .lcd_rst0 = 41,
				   },
			  },
		   },
	 },

	{
	 .dispdrv_name = "R61581_HVGA_SMI",
	 .dispdrv_entry = DISP_DRV_R61581_HVGA_SMI_GetFuncTable,
	 .parms = {
		   .w0 = {
			  .bits = {
				   .boot_mode = 0,
				   .bus_type = RHEA_BUS_SMI,
				   .bus_no = RHEA_BUS_0,
				   .bus_ch = RHEA_BUS_CH_0,
				   .bus_width = RHEA_BUS_WIDTH_08,
				   .te_input = RHEA_TE_IN_0_LCD,
				   .col_mode_i = RHEA_CM_I_RGB565,
				   .col_mode_o = RHEA_CM_O_RGB565,
				   },
			  },
		   .w1 = {
			  .bits = {
				   .api_rev = RHEA_LCD_BOOT_API_REV,
				   .lcd_rst0 = 41,
				   },
			  },
		   },
	 },

	{
	 .dispdrv_name = "R61581_HVGA_SMI",
	 .dispdrv_entry = DISP_DRV_R61581_HVGA_SMI_GetFuncTable,
	 .parms = {
		   .w0 = {
			  .bits = {
				   .boot_mode = 0,
				   .bus_type = RHEA_BUS_SMI,
				   .bus_no = RHEA_BUS_0,
				   .bus_ch = RHEA_BUS_CH_0,
				   .bus_width = RHEA_BUS_WIDTH_16,
				   .te_input = RHEA_TE_IN_0_LCD,
				   .col_mode_i = RHEA_CM_I_RGB565,
				   .col_mode_o = RHEA_CM_O_RGB565,
				   },
			  },
		   .w1 = {
			  .bits = {
				   .api_rev = RHEA_LCD_BOOT_API_REV,
				   .lcd_rst0 = 41,
				   },
			  },
		   },
	 },

};

#include "rhea_fb_init.c"
#endif /* #ifdef CONFIG_FB_BRCM_RHEA */
#endif

void __init board_init(void)
{
	board_add_common_devices();
#ifdef CONFIG_INPUT_GPIO
	bcm_keypad_config_iocr(8, 8);
#endif
	rhea_ray_add_devices();
#if 0
#ifdef CONFIG_FB_BRCM_RHEA
	/* rhea_fb_init.c */
	konafb_init();
#endif
#endif
	return;
}

void __init board_map_io(void)
{
	/* Map machine specific iodesc here */

	rhea_map_io();
}

late_initcall(rhea_ray_add_lateInit_devices);

MACHINE_START(RHEA, "RheaRay")
    .map_io = board_map_io,.init_irq = kona_init_irq,.timer =
    &kona_timer,.init_machine = board_init,.reserve =
    rhea_ray_reserve MACHINE_END
