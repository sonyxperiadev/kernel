#/************************************************************************/
/*                                                                      */
/*  Copyright 2012  Broadcom Corporation                                */
/*                                                                      */
/* Unless you and Broadcom execute a separate written software license  */
/* agreement governing use of this software, this software is licensed  */
/* to you under the terms of the GNU General Public License version 2   */
/* (the GPL), available at						*/
/*                                                                      */
/*          http://www.broadcom.com/licenses/GPLv2.php                  */
/*                                                                      */
/*  with the following added to such license:                           */
/*                                                                      */
/*  As a special exception, the copyright holders of this software give */
/*  you permission to link this software with independent modules, and  */
/*  to copy and distribute the resulting executable under terms of your */
/*  choice, provided that you also meet, for each linked independent    */
/*  module, the terms and conditions of the license of that module. An  */
/*  independent module is a module which is not derived from this       */
/*  software.  The special   exception does not apply to any            */
/*  modifications of the software.					*/
/*									*/
/*  Notwithstanding the above, under no circumstances may you combine	*/
/*  this software in any way with any other Broadcom software provided	*/
/*  under a license other than the GPL, without Broadcom's express	*/
/*  prior written consent.						*/
/*									*/
/************************************************************************/
#include <linux/version.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/mfd/bcm590xx/pmic.h>
#include <linux/of_platform.h>
#include <linux/of.h>
#include <linux/of_fdt.h>

#ifdef CONFIG_ION_BCM_NO_DT
#include <linux/ion.h>
#include <linux/broadcom/bcm_ion.h>
#endif
#ifdef CONFIG_IOMMU_API
#include <plat/bcm_iommu.h>
#endif
#include <linux/serial_8250.h>
#include <linux/i2c.h>
#include <linux/i2c-kona.h>
#include <linux/spi/spi.h>
#include <linux/clk.h>
#include <linux/bootmem.h>
#include <linux/input.h>
#include <linux/mfd/bcm590xx/core.h>
#include <asm/gpio.h>
#include <linux/gpio.h>
#include <linux/gpio_keys.h>
#include <linux/i2c-kona.h>
#include <linux/i2c.h>
#include <linux/i2c/tango_ts.h>
#include <asm/mach/arch.h>
#include <asm/mach-types.h>
#include <asm/mach/map.h>
#include <asm/hardware/gic.h>
#include <mach/hardware.h>
#include <mach/hardware.h>
#include <mach/kona_headset_pd.h>
#include <mach/kona.h>
#include <mach/sdio_platform.h>
#include <mach/hawaii.h>
#include <mach/io_map.h>
#include <mach/irqs.h>
#include <mach/rdb/brcm_rdb_uartb.h>
#include <mach/clock.h>
#include <plat/spi_kona.h>
#include <plat/chal/chal_trace.h>
#include <plat/pi_mgr.h>
#include <plat/spi_kona.h>

#include <trace/stm.h>

#include "devices.h"

#ifdef CONFIG_KEYBOARD_BCM
#include <mach/bcm_keypad.h>
#endif
#ifdef CONFIG_DMAC_PL330
#include <mach/irqs.h>
#include <plat/pl330-pdata.h>
#include <linux/dma-mapping.h>
#endif

#if defined(CONFIG_SPI_GPIO)
#include <linux/spi/spi_gpio.h>
#endif

#if defined(CONFIG_HAPTIC)
#include <linux/haptic.h>
#endif

#if (defined(CONFIG_BCM_RFKILL) || defined(CONFIG_BCM_RFKILL_MODULE))
#include <linux/broadcom/bcmbt_rfkill.h>
#endif

#ifdef CONFIG_BCM_BZHW
#include <linux/broadcom/bcm_bzhw.h>
#endif

#if defined(CONFIG_BCM_BT_LPM)
#include <linux/broadcom/bcm-bt-lpm.h>
#endif

#if defined(CONFIG_BCMI2CNFC)
#include <linux/bcmi2cnfc.h>
#endif

#if defined(CONFIG_SENSORS_BMA222)
#include <linux/bma222.h>
#endif

#if defined(CONFIG_BMP18X) || defined(CONFIG_BMP18X_I2C) || defined(CONFIG_BMP18X_I2C_MODULE)
#include <linux/bmp18x.h>
#include <mach/bmp18x_i2c_settings.h>
#endif

#if defined(CONFIG_AL3006) || defined(CONFIG_AL3006_MODULE)
#include <linux/al3006.h>
#include <mach/al3006_i2c_settings.h>
#endif

#if defined(CONFIG_INV_MPU_IIO) || defined(CONFIG_INV_MPU_IIO_MODULE)
#include <linux/mpu.h>
#include <linux/i2c/inv_mpu_settings.h>
#endif

#if defined(CONFIG_SENSORS_KIONIX_KXTIK)	\
			|| defined(CONFIG_SENSORS_KIONIX_KXTIK_MODULE)
#include <linux/kxtik.h>
#endif /* CONFIG_SENSORS_KIONIX_KXTIK */

#if defined(CONFIG_SENSORS_AK8963) || defined(CONFIG_SENSORS_AK8963_MODULE)
#include <linux/akm8963.h>
#include <linux/i2c/akm8963_i2c_settings.h>
#endif /* CONFIG_SENSORS_AK8963 */

#if defined(CONFIG_MPU_SENSORS_AMI306) || defined(CONFIG_MPU_SENSORS_AMI306_MODULE)
#include <linux/ami306_def.h>
#include <linux/ami_sensor.h>
#include <mach/ami306_settings.h>
#endif

#ifdef CONFIG_BACKLIGHT_PWM
#include <linux/pwm_backlight.h>
#endif

#ifdef CONFIG_FB_BRCM_KONA
#include <video/kona_fb_boot.h>
#include <video/kona_fb.h>
#endif

#if defined(CONFIG_BCM_ALSA_SOUND)
#include <mach/caph_platform.h>
#include <mach/caph_settings.h>
#endif

#ifdef CONFIG_UNICAM_CAMERA
#include <media/soc_camera.h>
#endif

#ifdef CONFIG_VIDEO_KONA
#include <media/v4l2-device.h>
#include <media/v4l2-subdev.h>
#include <media/kona-unicam.h>
#include <media/ov5640.h>
#endif

#ifdef CONFIG_WD_TAPPER
#include <linux/broadcom/wd-tapper.h>
#endif

#if defined(CONFIG_TOUCHSCREEN_BCM915500) || defined(CONFIG_TOUCHSCREEN_BCM915500_MODULE)
#include <linux/i2c/bcmtch15xxx.h>
#endif

#ifdef CONFIG_USB_DWC_OTG
#include <linux/usb/bcm_hsotgctrl.h>
#include <linux/usb/otg.h>
#endif

#ifdef CONFIG_BRCM_UNIFIED_DHD_SUPPORT
#include "hawaii_wifi.h"

extern int
hawaii_wifi_status_register(void (*callback) (int card_present, void *dev_id),
			    void *dev_id);
#endif

/* SD */
#define SD_CARDDET_GPIO_PIN	91

/* keypad map */
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

/* Touch */
#define TSC_GPIO_IRQ_PIN			73

#define TSC_GPIO_RESET_PIN			70
#define TANGO_I2C_TS_DRIVER_NUM_BYTES_TO_READ	14

/* NFC */
#define NFC_INT	90
#define NFC_WAKE 25
#define NFC_ENABLE 100

#ifdef CONFIG_ION_BCM_NO_DT
struct ion_platform_data ion_system_data = {
	.nr = 1,
#ifdef CONFIG_IOMMU_API
	.pdev_iommu = &iommu_mm_device,
#endif
#ifdef CONFIG_BCM_IOVMM
	.pdev_iovmm = &iovmm_mm_256mb_device,
#endif
	.heaps = {
		[0] = {
			.id    = 0,
			.type  = ION_HEAP_TYPE_SYSTEM,
			.name  = "ion-system",
			.base  = 0,
			.limit = 0,
			.size  = 0,
		},
	},
};

struct ion_platform_data ion_system_extra_data = {
	.nr = 1,
#ifdef CONFIG_IOMMU_API
	.pdev_iommu = &iommu_mm_device,
#endif
#ifdef CONFIG_BCM_IOVMM
	.pdev_iovmm = &iovmm_mm_device,
#endif
	.heaps = {
		[0] = {
			.id    = 1,
			.type  = ION_HEAP_TYPE_SYSTEM,
			.name  = "ion-system-extra",
			.base  = 0,
			.limit = 0,
			.size  = 0,
		},
	},
};

struct ion_platform_data ion_carveout_data = {
	.nr = 2,
#ifdef CONFIG_IOMMU_API
	.pdev_iommu = &iommu_mm_device,
#endif
#ifdef CONFIG_BCM_IOVMM
	.pdev_iovmm = &iovmm_mm_256mb_device,
#endif
	.heaps = {
		[0] = {
			.id    = 9,
			.type  = ION_HEAP_TYPE_CARVEOUT,
			.name  = "ion-carveout",
			.base  = 0x90000000,
			.limit = 0xa0000000,
			.size  = (16 * SZ_1M),
#ifdef CONFIG_ION_OOM_KILLER
			.lmk_enable = 0,
			.lmk_min_score_adj = 411,
			.lmk_min_free = 32,
#endif
		},
		[1] = {
			.id    = 10,
			.type  = ION_HEAP_TYPE_CARVEOUT,
			.name  = "ion-carveout-extra",
			.base  = 0,
			.limit = 0,
			.size  = (0 * SZ_1M),
#ifdef CONFIG_ION_OOM_KILLER
			.lmk_enable = 0,
			.lmk_min_score_adj = 411,
			.lmk_min_free = 32,
#endif
		},
	},
};

#ifdef CONFIG_CMA
struct ion_platform_data ion_cma_data = {
	.nr = 2,
#ifdef CONFIG_IOMMU_API
	.pdev_iommu = &iommu_mm_device,
#endif
#ifdef CONFIG_BCM_IOVMM
	.pdev_iovmm = &iovmm_mm_256mb_device,
#endif
	.heaps = {
		[0] = {
			.id = 5,
			.type  = ION_HEAP_TYPE_DMA,
			.name  = "ion-cma",
			.base  = 0x90000000,
			.limit = 0xa0000000,
			.size  = (0 * SZ_1M),
#ifdef CONFIG_ION_OOM_KILLER
			.lmk_enable = 1,
			.lmk_min_score_adj = 411,
			.lmk_min_free = 32,
#endif
		},
		[1] = {
			.id = 6,
			.type  = ION_HEAP_TYPE_DMA,
			.name  = "ion-cma-extra",
			.base  = 0,
			.limit = 0,
			.size  = (0 * SZ_1M),
#ifdef CONFIG_ION_OOM_KILLER
			.lmk_enable = 1,
			.lmk_min_score_adj = 411,
			.lmk_min_free = 32,
#endif
		},
	},
};
#endif /* CONFIG_CMA */
#if defined(CONFIG_MM_SECURE_DRIVER)
struct ion_platform_data ion_secure_data = {
	.nr = 2,
#ifdef CONFIG_IOMMU_API
	.pdev_iommu = &iommu_mm_device,
#endif
#ifdef CONFIG_BCM_IOVMM
	.pdev_iovmm = &iovmm_mm_device,
#endif
	.heaps = {
		[0] = {
			.id = 13,
			.type  = ION_HEAP_TYPE_SECURE,
			.name  = "ion-secure",
			.base  = 0,
			.limit = 0,
			.size  = (16 * SZ_1M),
		},
		[1] = {
			.id = 14,
			.type  = ION_HEAP_TYPE_SECURE,
			.name  = "ion-secure-extra",
			.base  = 0,
			.limit = 0,
			.size  = (0 * SZ_1M),
		},
	},
};
#endif /* CONFIG_MM_SECURE_DRIVER */
#endif /* CONFIG_ION_BCM_NO_DT */

#ifdef CONFIG_VIDEO_ADP1653
#define ADP1653_I2C_ADDR 0x60
static struct i2c_board_info adp1653_flash[] = {
	{
	 I2C_BOARD_INFO("adp1653", (ADP1653_I2C_ADDR >> 1))
	 },
};
#endif

#ifdef CONFIG_UNICAM_CAMERA

static struct regulator *d_gpsr_cam0_1v8;
static struct regulator *d_lvldo2_cam1_1v8;
static struct regulator *d_1v8_mmc1_vcc;
static struct regulator *d_3v0_mmc1_vcc;

#define SENSOR_0_GPIO_PWRDN             (002)
#define SENSOR_0_GPIO_RST               (111)
#define SENSOR_0_CLK                    "dig_ch0_clk"	/*DCLK1 */
#define SENSOR_0_CLK_FREQ               (13000000)
#define CSI0_LP_FREQ			(100000000)
#define CSI1_LP_FREQ			(100000000)

#define SENSOR_1_CLK                    "dig_ch0_clk"	/* DCLK1 */
#define SENSOR_1_CLK_FREQ               (26000000)

#define SENSOR_1_GPIO_PWRDN             (005)


static int hawaii_camera_power(struct device *dev, int on)
{
	unsigned int value;
	int ret = -1;
	struct clk *clock;
	struct clk *lp_clock;
	struct clk *axi_clk;
	static struct pi_mgr_dfs_node unicam_dfs_node;

	printk(KERN_INFO "%s:camera power %s\n", __func__, (on ? "on" : "off"));
	if (!unicam_dfs_node.valid) {
		ret = pi_mgr_dfs_add_request(&unicam_dfs_node, "unicam",
					     PI_MGR_PI_ID_MM,
					     PI_MGR_DFS_MIN_VALUE);
		if (ret) {
			return -1;
		}
		if (gpio_request_one(SENSOR_0_GPIO_RST, GPIOF_DIR_OUT |
				     GPIOF_INIT_LOW, "Cam0Rst")) {
			printk(KERN_ERR "Unable to get cam0 RST GPIO\n");
			return -1;
		}
		if (gpio_request_one(SENSOR_0_GPIO_PWRDN, GPIOF_DIR_OUT |
				     GPIOF_INIT_LOW, "CamPWDN")) {
			printk(KERN_ERR "Unable to get cam0 PWDN GPIO\n");
			return -1;
		}
		/*MMC1 VCC */
		d_1v8_mmc1_vcc = regulator_get(NULL, "mmc1_vcc");
		if (IS_ERR_OR_NULL(d_1v8_mmc1_vcc))
			printk(KERN_ERR "Failed to  get d_1v8_mmc1_vcc\n");
		d_3v0_mmc1_vcc = regulator_get(NULL, "mmc2_vcc");
		if (IS_ERR_OR_NULL(d_3v0_mmc1_vcc))
			printk(KERN_ERR "Failed to  get d_3v0_mmc1_vcc\n");
		d_gpsr_cam0_1v8 = regulator_get(NULL, "vsr_uc");
		if (IS_ERR_OR_NULL(d_gpsr_cam0_1v8))
			printk(KERN_ERR "Failed to  get d_gpsr_cam0_1v8\n");
	}

	ret = -1;
	lp_clock = clk_get(NULL, CSI0_LP_PERI_CLK_NAME_STR);
	if (IS_ERR_OR_NULL(lp_clock)) {
		printk(KERN_ERR "Unable to get %s clock\n",
		CSI0_LP_PERI_CLK_NAME_STR);
		goto e_clk_get;
	}

	clock = clk_get(NULL, SENSOR_0_CLK);
	if (IS_ERR_OR_NULL(clock)) {
		printk(KERN_ERR "Unable to get SENSOR_0 clock\n");
		goto e_clk_get;
	}
	axi_clk = clk_get(NULL, "csi0_axi_clk");
	if (IS_ERR_OR_NULL(axi_clk)) {
		printk(KERN_ERR "Unable to get AXI clock\n");
		goto e_clk_get;
	}
	if (on) {
		if (pi_mgr_dfs_request_update(&unicam_dfs_node, PI_OPP_TURBO))
			printk("DVFS for UNICAM failed\n");
		regulator_enable(d_3v0_mmc1_vcc);
		usleep_range(1000, 1010);
		regulator_enable(d_1v8_mmc1_vcc);
		usleep_range(1000, 1010);
		regulator_enable(d_gpsr_cam0_1v8);
		usleep_range(1000, 1010);

		if (mm_ccu_set_pll_select(CSI0_BYTE1_PLL, 8)) {
			pr_err("failed to set BYTE1\n");
			goto e_clk_pll;
		}
		if (mm_ccu_set_pll_select(CSI0_BYTE0_PLL, 8)) {
			pr_err("failed to set BYTE0\n");
			goto e_clk_pll;
		}
		if (mm_ccu_set_pll_select(CSI0_CAMPIX_PLL, 8)) {
			pr_err("failed to set PIXPLL\n");
			goto e_clk_pll;
		}

		value = clk_enable(axi_clk);
		if (value) {
			pr_err(KERN_ERR "Failed to enable axi clock\n");
			goto e_clk_axi;
		}
		value = clk_enable(lp_clock);
		if (value) {
			pr_err(KERN_ERR "Failed to enable lp clock\n");
			goto e_clk_lp;
		}

		value = clk_set_rate(lp_clock, CSI0_LP_FREQ);
		if (value) {
			pr_err("Failed to set lp clock\n");
			goto e_clk_set_lp;
		}
		value = clk_enable(clock);
		if (value) {
			pr_err("Failed to enable sensor 0 clock\n");
			goto e_clk_sensor;
		}
		value = clk_set_rate(clock, SENSOR_0_CLK_FREQ);
		if (value) {
			pr_err("Failed to set sensor0 clock\n");
			goto e_clk_set_sensor;
		}
		usleep_range(10000, 10100);
		gpio_set_value(SENSOR_0_GPIO_RST, 0);
		usleep_range(10000, 10100);
		gpio_set_value(SENSOR_0_GPIO_PWRDN, 0);
		usleep_range(5000, 5100);
		gpio_set_value(SENSOR_0_GPIO_RST, 1);
		msleep(30);
	} else {
		gpio_set_value(SENSOR_0_GPIO_PWRDN, 1);
		clk_disable(clock);
		clk_disable(lp_clock);
		clk_disable(axi_clk);
		regulator_disable(d_3v0_mmc1_vcc);
		regulator_disable(d_1v8_mmc1_vcc);
		regulator_disable(d_gpsr_cam0_1v8);
		if (pi_mgr_dfs_request_update
		    (&unicam_dfs_node, PI_MGR_DFS_MIN_VALUE)) {
			printk("Failed to set DVFS for unicam\n");
		}
	}
	return 0;

e_clk_set_sensor:
	clk_disable(clock);
e_clk_sensor:
e_clk_set_lp:
	clk_disable(lp_clock);
e_clk_lp:
	clk_disable(axi_clk);
e_clk_axi:
e_clk_pll:
e_clk_get:
	return ret;
}
static int hawaii_camera_reset(struct device *dev)
{
	/* reset the camera gpio */
	printk(KERN_INFO "%s:camera reset\n", __func__);
	return 0;
}

static int hawaii_camera_power_front(struct device *dev, int on)
{
	unsigned int value;
	int ret = -1;
	struct clk *clock;
	struct clk *axi_clk;
	struct clk *axi_clk_0;
	struct clk *lp_clock_0;
	struct clk *lp_clock_1;
	static struct pi_mgr_dfs_node unicam_dfs_node;

	printk(KERN_INFO "%s:camera power %s\n", __func__, (on ? "on" : "off"));
	if (!unicam_dfs_node.valid) {
		ret = pi_mgr_dfs_add_request(&unicam_dfs_node, "unicam",
					     PI_MGR_PI_ID_MM,
					     PI_MGR_DFS_MIN_VALUE);
		if (ret) {
			return -1;
		}
		if (gpio_request_one(SENSOR_1_GPIO_PWRDN, GPIOF_DIR_OUT |
				     GPIOF_INIT_LOW, "Cam1PWDN")) {
			printk(KERN_ERR "Unable to get CAM1PWDN\n");
			return -1;
		}
		d_lvldo2_cam1_1v8 = regulator_get(NULL, "lvldo2_uc");
		if (IS_ERR_OR_NULL(d_lvldo2_cam1_1v8))
			printk(KERN_ERR "Failed to get d_lvldo2_cam1_1v8\n");
		if (d_1v8_mmc1_vcc == NULL) {
			d_1v8_mmc1_vcc = regulator_get(NULL, "mmc1_vcc");
			if (IS_ERR_OR_NULL(d_1v8_mmc1_vcc))
				printk(KERN_ERR "Err d_1v8_mmc1_vcc\n");
		}
	}

	ret = -1;
	lp_clock_0 = clk_get(NULL, CSI0_LP_PERI_CLK_NAME_STR);
	if (IS_ERR_OR_NULL(lp_clock_0)) {
		printk(KERN_ERR "Unable to get %s clock\n",
		CSI0_LP_PERI_CLK_NAME_STR);
		goto e_clk_get;
	}

	lp_clock_1 = clk_get(NULL, CSI1_LP_PERI_CLK_NAME_STR);
	if (IS_ERR_OR_NULL(lp_clock_1)) {
		printk(KERN_ERR "Unable to get %s clock\n",
		CSI1_LP_PERI_CLK_NAME_STR);
		goto e_clk_get;
	}

	clock = clk_get(NULL, SENSOR_1_CLK);
	if (IS_ERR_OR_NULL(clock)) {
		printk(KERN_ERR "Unable to get SENSOR_1 clock\n");
		goto e_clk_get;
	}
	axi_clk_0 = clk_get(NULL, "csi0_axi_clk");
	if (IS_ERR_OR_NULL(axi_clk_0)) {
		printk(KERN_ERR "Unable to get AXI clock 0\n");
		goto e_clk_get;
	}
	axi_clk = clk_get(NULL, "csi1_axi_clk");
	if (IS_ERR_OR_NULL(axi_clk)) {
		printk(KERN_ERR "Unable to get AXI clock 1\n");
		goto e_clk_get;
	}
	if (on) {
		if (pi_mgr_dfs_request_update(&unicam_dfs_node, PI_OPP_TURBO))
			printk("DVFS for UNICAM failed\n");
		gpio_set_value(SENSOR_1_GPIO_PWRDN, 1);
		usleep_range(5000, 5010);
		regulator_enable(d_lvldo2_cam1_1v8);
		usleep_range(1000, 1010);
		regulator_enable(d_1v8_mmc1_vcc);
		usleep_range(1000, 1010);

		if (mm_ccu_set_pll_select(CSI1_BYTE1_PLL, 8)) {
			pr_err("failed to set BYTE1\n");
			goto e_clk_pll;
		}
		if (mm_ccu_set_pll_select(CSI1_BYTE0_PLL, 8)) {
			pr_err("failed to set BYTE0\n");
			goto e_clk_pll;
		}
		if (mm_ccu_set_pll_select(CSI1_CAMPIX_PLL, 8)) {
			pr_err("failed to set PIXPLL\n");
			goto e_clk_pll;
		}

		value = clk_enable(lp_clock_0);
		if (value) {
			printk(KERN_ERR "Failed to enable lp clock 0\n");
			goto e_clk_lp0;
		}

		value = clk_set_rate(lp_clock_0, CSI0_LP_FREQ);
		if (value) {
			pr_err("Failed to set lp clock 0\n");
			goto e_clk_set_lp0;
		}

		value = clk_enable(lp_clock_1);
		if (value) {
			pr_err(KERN_ERR "Failed to enable lp clock 1\n");
			goto e_clk_lp1;
		}

		value = clk_set_rate(lp_clock_1, CSI1_LP_FREQ);
		if (value) {
			pr_err("Failed to set lp clock 1\n");
			goto e_clk_set_lp1;
		}

		value = clk_enable(axi_clk_0);
		if (value) {
			printk(KERN_ERR "Failed to enable axi clock 0\n");
			goto e_clk_axi_clk_0;
		}
		value = clk_enable(axi_clk);
		if (value) {
			printk(KERN_ERR "Failed to enable axi clock 1\n");
			goto e_clk_axi;
		}
		value = clk_enable(clock);
		if (value) {
			printk("Failed to enable sensor 1 clock\n");
			goto e_clk_clock;
		}
		value = clk_set_rate(clock, SENSOR_1_CLK_FREQ);
		if (value) {
			printk("Failed to set sensor 1 clock\n");
			goto e_clk_set_clock;
		}
		usleep_range(10000, 10100);
		gpio_set_value(SENSOR_1_GPIO_PWRDN, 0);
		msleep(30);
	} else {
		gpio_set_value(SENSOR_1_GPIO_PWRDN, 1);
		clk_disable(lp_clock_0);
		clk_disable(lp_clock_1);
		clk_disable(clock);
		clk_disable(axi_clk);
		clk_disable(axi_clk_0);
		regulator_disable(d_lvldo2_cam1_1v8);
		regulator_disable(d_1v8_mmc1_vcc);
		if (pi_mgr_dfs_request_update
		    (&unicam_dfs_node, PI_MGR_DFS_MIN_VALUE)) {
			printk("Failed to set DVFS for unicam\n");
		}
	}
	return 0;

e_clk_set_clock:
	clk_disable(clock);
e_clk_clock:
	clk_disable(axi_clk);
e_clk_axi:
	clk_disable(axi_clk_0);
e_clk_axi_clk_0:
e_clk_set_lp1:
	clk_disable(lp_clock_1);
e_clk_lp1:
e_clk_set_lp0:
	clk_disable(lp_clock_0);
e_clk_lp0:
e_clk_pll:
e_clk_get:
	return ret;
}

static int hawaii_camera_reset_front(struct device *dev)
{
	/* reset the camera gpio */
	printk(KERN_INFO "%s:camera reset\n", __func__);
	return 0;
}

static struct soc_camera_link iclink_ov5640 = {
	.power = &hawaii_camera_power,
	.reset = &hawaii_camera_reset,
};

static struct soc_camera_link iclink_ov7692 = {
	.power = &hawaii_camera_power_front,
	.reset = &hawaii_camera_reset_front,
};

#endif /* CONFIG_UNICAM_CAMERA */
static const struct of_dev_auxdata hawaii_auxdata_lookup[] __initconst = {
	OF_DEV_AUXDATA("bcm,soc-camera", 0x3c,
		"soc-back-camera", &iclink_ov5640),
	OF_DEV_AUXDATA("bcm,soc-camera", 0x3e,
		"soc-front-camera", &iclink_ov7692),
	{},
};

static struct spi_kona_platform_data hawaii_ssp0_info = {
#ifdef CONFIG_DMAC_PL330
	.enable_dma = 1,
#else
	.enable_dma = 0,
#endif
	.cs_line = 1,
	.mode = SPI_LOOP | SPI_MODE_3,
};

static struct spi_kona_platform_data hawaii_ssp1_info = {
#ifdef CONFIG_DMAC_PL330
	.enable_dma = 1,
#else
	.enable_dma = 0,
#endif
};

#ifdef CONFIG_STM_TRACE
static struct stm_platform_data hawaii_stm_pdata = {
	.regs_phys_base = STM_BASE_ADDR,
	.channels_phys_base = SWSTM_BASE_ADDR,
	.id_mask = 0x0,		/* Skip ID check/match */
	.final_funnel = CHAL_TRACE_FIN_FUNNEL,
};
#endif

#if defined(CONFIG_USB_DWC_OTG)
static struct bcm_hsotgctrl_platform_data hsotgctrl_plat_data = {
	.hsotgctrl_virtual_mem_base = KONA_USB_HSOTG_CTRL_VA,
	.chipreg_virtual_mem_base = KONA_CHIPREG_VA,
	.irq = BCM_INT_ID_HSOTG_WAKEUP,
	.usb_ahb_clk_name = USB_OTG_AHB_BUS_CLK_NAME_STR,
	.mdio_mstr_clk_name = MDIOMASTER_PERI_CLK_NAME_STR,
};
#endif

struct platform_device *hawaii_common_plat_devices[] __initdata = {
	&pmu_device,
	&hawaii_ssp0_device,

#ifdef CONFIG_SENSORS_KONA
	&thermal_device,
#endif

#ifdef CONFIG_STM_TRACE
	&hawaii_stm_device,
#endif

#if defined(CONFIG_HW_RANDOM_KONA)
	&rng_device,
#endif

#if defined(CONFIG_USB_DWC_OTG)
	&hawaii_usb_phy_platform_device,
	&hawaii_hsotgctrl_platform_device,
	&hawaii_otg_platform_device,
#endif

#ifdef CONFIG_BRCM_CDC
	&brcm_cdc_device,
#endif

#ifdef CONFIG_KONA_AVS
	&avs_device,
#endif

#ifdef CONFIG_KONA_CPU_FREQ_DRV
	&kona_cpufreq_device,
#endif

#ifdef CONFIG_CRYPTO_DEV_BRCM_SPUM_HASH
	&hawaii_spum_device,
#endif

#ifdef CONFIG_CRYPTO_DEV_BRCM_SPUM_AES
	&hawaii_spum_aes_device,
#endif

#ifdef CONFIG_UNICAM
	&hawaii_unicam_device,
#endif

#ifdef CONFIG_UNICAM_CAMERA
	&hawaii_camera_device,
#endif

#ifdef CONFIG_SND_BCM_SOC
	&caph_i2s_device,
	&caph_pcm_device,
#endif
};

struct regulator_consumer_supply sd_supply[] = {
	{.supply = "sdldo_uc"},
	REGULATOR_SUPPLY("vddmmc", "sdhci.3"),	/* 0x3f1b0000.sdhci */
	{.supply = "vdd_sdio"},
};

struct regulator_consumer_supply sdx_supply[] = {
	{.supply = "sdxldo_uc"},
	REGULATOR_SUPPLY("vddo", "sdhci.3"),	/* 0x3f1b0000.sdhci */
	{.supply = "vdd_sdxc"},
	{.supply = "sddat_debug_bus"},
};

#ifdef CONFIG_KEYBOARD_BCM
static struct bcm_keymap hawaii_keymap[] = {
	{BCM_KEY_ROW_0, BCM_KEY_COL_0, "Vol Down Key", KEY_VOLUMEDOWN},
	{BCM_KEY_ROW_0, BCM_KEY_COL_1, "Vol Up Key", KEY_VOLUMEUP},
	{BCM_KEY_ROW_0, BCM_KEY_COL_2, "Search Key", KEY_SEARCH},
	{BCM_KEY_ROW_0, BCM_KEY_COL_3, "unused", 0},
	{BCM_KEY_ROW_0, BCM_KEY_COL_4, "unused", 0},
	{BCM_KEY_ROW_0, BCM_KEY_COL_5, "unused", 0},
	{BCM_KEY_ROW_0, BCM_KEY_COL_6, "unused", 0},
	{BCM_KEY_ROW_0, BCM_KEY_COL_7, "unused", 0},
	{BCM_KEY_ROW_1, BCM_KEY_COL_0, "Home-Key", KEY_HOME},
	{BCM_KEY_ROW_1, BCM_KEY_COL_1, "Back Key", KEY_BACK},
	{BCM_KEY_ROW_1, BCM_KEY_COL_2, "Menu-Key", KEY_MENU},
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
	{BCM_KEY_ROW_6, BCM_KEY_COL_6, "unused", 0},
	{BCM_KEY_ROW_6, BCM_KEY_COL_7, "unused", 0},
	{BCM_KEY_ROW_7, BCM_KEY_COL_0, "unused", 0},
	{BCM_KEY_ROW_7, BCM_KEY_COL_1, "unused", 0},
	{BCM_KEY_ROW_7, BCM_KEY_COL_2, "unused", 0},
	{BCM_KEY_ROW_7, BCM_KEY_COL_3, "unused", 0},
	{BCM_KEY_ROW_7, BCM_KEY_COL_4, "unused", 0},
	{BCM_KEY_ROW_7, BCM_KEY_COL_5, "unused", 0},
	{BCM_KEY_ROW_7, BCM_KEY_COL_6, "unused", 0},
	{BCM_KEY_ROW_7, BCM_KEY_COL_7, "unused", 0},
};

static struct bcm_keypad_platform_info hawaii_keypad_data = {
	.row_num = 2,
	.col_num = 3,
	.keymap = hawaii_keymap,
	.bcm_keypad_base = (void *)__iomem HW_IO_PHYS_TO_VIRT(KEYPAD_BASE_ADDR),
};

#endif

#if defined(CONFIG_BCMI2CNFC)
static int bcmi2cnfc_gpio_setup(void *);
static int bcmi2cnfc_gpio_clear(void *);
static struct bcmi2cnfc_i2c_platform_data bcmi2cnfc_pdata = {
	.i2c_pdata = {ADD_I2C_SLAVE_SPEED(BSC_BUS_SPEED_400K),
		      SET_CLIENT_FUNC(TX_FIFO_ENABLE | RX_FIFO_ENABLE)},
	.irq_gpio = NFC_INT,
	.en_gpio = NFC_ENABLE,
	.wake_gpio = NFC_WAKE,
	.init = bcmi2cnfc_gpio_setup,
	.reset = bcmi2cnfc_gpio_clear,
};

static int bcmi2cnfc_gpio_setup(void *this)
{
	return 0;
}
static int bcmi2cnfc_gpio_clear(void *this)
{
	return 0;
}

static struct i2c_board_info __initdata bcmi2cnfc[] = {
	{
	 I2C_BOARD_INFO("bcmi2cnfc", 0x1F0),
	 .flags = I2C_CLIENT_TEN,
	 .platform_data = (void *)&bcmi2cnfc_pdata,
	 .irq = gpio_to_irq(NFC_INT),
	 },
};
#endif

#if  defined(CONFIG_BMP18X) || defined(CONFIG_BMP18X_I2C) || defined(CONFIG_BMP18X_I2C_MODULE)
static struct i2c_board_info __initdata i2c_bmp18x_info[] = {
	{
	 I2C_BOARD_INFO(BMP18X_NAME, BMP18X_I2C_ADDRESS),
	 },
};
#endif

#if defined(CONFIG_AL3006) || defined(CONFIG_AL3006_MODULE)
static struct al3006_platform_data al3006_pdata = {
#ifdef AL3006_IRQ_GPIO
	.irq_gpio = AL3006_IRQ_GPIO,
#else
	.irq_gpio = -1,
#endif
};

static struct i2c_board_info __initdata i2c_al3006_info[] = {
	{
	 I2C_BOARD_INFO("al3006", AL3006_I2C_ADDRESS),
	 .platform_data = &al3006_pdata,
	 },
};
#endif

#if defined(CONFIG_MPU_SENSORS_AMI306) || defined(CONFIG_MPU_SENSORS_AMI306_MODULE)
static struct ami306_platform_data ami306_data = AMI306_DATA;
static struct i2c_board_info __initdata i2c_ami306_info[] = {
	{
	 I2C_BOARD_INFO(AMI_DRV_NAME, AMI_I2C_ADDRESS),
	 .platform_data = &ami306_data,
	 },
};
#endif
#if defined(CONFIG_SENSORS_KIONIX_KXTIK) \
	|| defined(CONFIG_SENSORS_KIONIX_KXTIK_MODULE)
#define KXTIK_DEVICE_MAP    2
#define KXTIK_MAP_X         ((KXTIK_DEVICE_MAP-1)%2)
#define KXTIK_MAP_Y         (KXTIK_DEVICE_MAP%2)
#define KXTIK_NEG_X         (((KXTIK_DEVICE_MAP+2)/2)%2)
#define KXTIK_NEG_Y         (((KXTIK_DEVICE_MAP+5)/4)%2)
#define KXTIK_NEG_Z         ((KXTIK_DEVICE_MAP-1)/4)

struct kxtik_platform_data kxtik_pdata = {
	.min_interval = 5,
	.poll_interval = 200,
	.axis_map_x = KXTIK_MAP_X,
	.axis_map_y = KXTIK_MAP_Y,
	.axis_map_z = 2,
	.negate_x = KXTIK_NEG_X,
	.negate_y = KXTIK_NEG_Y,
	.negate_z = KXTIK_NEG_Z,
	.res_12bit = RES_12BIT,
	.g_range = KXTIK_G_2G,
};

#define KXTIK_GPIO_IRQ_PIN          (64)
#define KXTIK_I2C_BUS_ID            (0)

static struct i2c_board_info __initdata kxtik_i2c_boardinfo[] = {
	{
	 I2C_BOARD_INFO("kxtik", KXTIK_SLAVE_ADDR),
	 .platform_data = &kxtik_pdata,
	 .irq = gpio_to_irq(KXTIK_GPIO_IRQ_PIN),
	 },
};

static int kxtik_init_platform_hw(void)
{
	int ret = 0;
	ret = gpio_request(KXTIK_GPIO_IRQ_PIN, "kxtik-irq");
	if (ret < 0) {
		printk(KERN_INFO
		       "kxtik: sensors gpio_request GPIO %d failed, err %d\n",
		       KXTIK_GPIO_IRQ_PIN, ret);
		return ret;
	}
	ret = gpio_direction_input(KXTIK_GPIO_IRQ_PIN);
	if (ret < 0) {
		printk(KERN_INFO
		       "kxtik: sensors gpio_direction_input set GPIO %d as input failed, err %d\n",
		       KXTIK_GPIO_IRQ_PIN, ret);
		gpio_free(KXTIK_GPIO_IRQ_PIN);
		return ret;
	}
	return ret;
}
#endif /* CONFIG_SENSORS_KIONIX_KXTIK */

#if defined(CONFIG_SENSORS_AK8963) || defined(CONFIG_SENSORS_AK8963_MODULE)
static struct akm8963_platform_data akm_platform_data_akm8963 = {
	.gpio_DRDY = AKM8963_IRQ_GPIO,
	.gpio_RST = AKM8963_GPIO_RESET_PIN,
	.layout = 1,
	.outbit = 1,
};

static struct i2c_board_info __initdata akm8963_info[] = {
	{
	 I2C_BOARD_INFO("akm8963", AKM8963_I2C_ADDRESS),
	 .flags = I2C_CLIENT_WAKE,
	 .platform_data = &akm_platform_data_akm8963,
	 .irq = gpio_to_irq(AKM8963_IRQ_GPIO),
	 },
};

static int akm8963_init_platform_hw(void)
{
	int ret;
	pr_info("akm8963]init_platform_hw\n");
	ret = gpio_request(AKM8963_IRQ_GPIO, "akm8963");
	gpio_direction_input(AKM8963_IRQ_GPIO);
	return 0;
}
#endif /* CONFIG_SENSORS_AK8963 */

#if defined(CONFIG_INV_MPU_IIO) || defined(CONFIG_INV_MPU_IIO_MODULE)

static struct mpu_platform_data inv_mpu_platform_data = {
	.int_config = INV_MPU_INIT_CFG,
	.level_shifter = 0,
	.orientation = INV_MPU_DRIVER_GYRO_ORIENTATION,
};

static struct i2c_board_info __initdata inv_mpu_i2c0_boardinfo[] = {
	{
	 I2C_BOARD_INFO(INV_MPU_DRIVER_NAME, INV_MPU_SLAVE_ADDR),
	 .platform_data = &inv_mpu_platform_data,
	 },
};
#endif /* CONFIG_INV_MPU_IIO */

#if defined(CONFIG_MPU_SENSORS_KXTF9)
static struct ext_slave_platform_data inv_mpu_kxtf9_data = {
	.bus = EXT_SLAVE_BUS_SECONDARY,
	.orientation = MPU3050_DRIVER_ACCEL_KXTF9_ORIENTATION,
};

static struct i2c_board_info __initdata inv_mpu_kxtf9_boardinfo[] = {
	{
	 I2C_BOARD_INFO("kxtf9", MPU_SENSORS_KXTF9_SLAVE_ADDR),
	 .platform_data = &inv_mpu_kxtf9_data,
	 },
};
#endif /* CONFIG_MPU_SENSORS_KXTF9 */

#if defined(CONFIG_SENSORS_BMA222)
static struct bma222_accl_platform_data bma_pdata = {
	.orientation = BMA_ROT_90,
	.invert = false,
};
#endif

#ifdef CONFIG_KONA_HEADSET_MULTI_BUTTON

#define HS_IRQ		gpio_to_irq(92)
#define HSB_IRQ		BCM_INT_ID_AUXMIC_COMP2
#define HSB_REL_IRQ	BCM_INT_ID_AUXMIC_COMP2_INV
static unsigned int hawaii_button_adc_values[3][2] = {
	/* SEND/END Min, Max */
	{0, 10},
	/* Volume Up  Min, Max */
	{11, 30},
	/* Volue Down Min, Max */
	{30, 680},
};

static unsigned int hawaii_button_adc_values_2_1[3][2] = {
	/* SEND/END Min, Max */
	{0, 104},
	/* Volume Up  Min, Max */
	{139, 270},
	/* Volue Down Min, Max */
	{330, 680},
};
static struct kona_headset_pd hawaii_headset_data = {
	/* GPIO state read is 0 on HS insert and 1 for
	 * HS remove
	 */

	.hs_default_state = 0,
	/*
	 * Because of the presence of the resistor in the MIC_IN line.
	 * The actual ground may not be 0, but a small offset is added to it.
	 * This needs to be subtracted from the measured voltage to determine the
	 * correct value. This will vary for different HW based on the resistor
	 * values used.
	 *
	 * if there is a resistor present on this line, please measure the load
	 * value and put it here, otherwise 0.
	 *
	 */
	.phone_ref_offset = 0,

	/*
	 * Inform the driver whether there is a GPIO present on the board to
	 * detect accessory insertion/removal _OR_ should the driver use the
	 * COMP1 for the same.
	 */
	.gpio_for_accessory_detection = 1,

	/*
	 * Pass the board specific button detection range
	 */
	.button_adc_values_low = 0,

	/*
	 * Pass the board specific button detection range
	 */
	.button_adc_values_high = hawaii_button_adc_values_2_1,

};
#endif /* CONFIG_KONA_HEADSET_MULTI_BUTTON */

#ifdef CONFIG_DMAC_PL330
static struct kona_pl330_data hawaii_pl330_pdata = {
	/* Non Secure DMAC virtual base address */
	.dmac_ns_base = KONA_DMAC_NS_VA,
	/* Secure DMAC virtual base address */
	.dmac_s_base = KONA_DMAC_S_VA,
	/* # of PL330 dmac channels 'configurable' */
	.num_pl330_chans = 8,
	/* irq number to use */
	.irq_base = BCM_INT_ID_DMAC0,
	/* # of PL330 Interrupt lines connected to GIC */
	.irq_line_count = 8,
};
#endif

#if defined(CONFIG_BCM_BT_LPM) && !defined(CONFIG_OF_DEVICE)
#define GPIO_BT_WAKE	32
#define GPIO_HOST_WAKE	72

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
};

#if defined(CONFIG_HAPTIC_SAMSUNG_PWM)
void haptic_gpio_setup(void)
{
	/* Board specific configuration like pin mux & GPIO */
}

static struct haptic_platform_data haptic_control_data = {
	/* Haptic device name: can be device-specific name like ISA1000 */
	.name = "pwm_vibra",
	/* PWM interface name to request */
	.pwm_id = 2,
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

static struct sdio_platform_cfg hawaii_sdio_param[] = {
	{
	 .id = 0,
	 .data_pullup = 0,
	 .cd_gpio = SD_CARDDET_GPIO_PIN,
	 .devtype = SDIO_DEV_TYPE_SDMMC,
	 .flags = KONA_SDIO_FLAGS_DEVICE_REMOVABLE,
	 .peri_clk_name = "sdio1_clk",
	 .ahb_clk_name = "sdio1_ahb_clk",
	 .sleep_clk_name = "sdio1_sleep_clk",
	 .peri_clk_rate = 48000000,
	 /*The SD card regulator */
	 .vddo_regulator_name = "vdd_sdio",
	 /*The SD controller regulator */
	 .vddsdxc_regulator_name = "vdd_sdxc",
	 },
	{
	 .id = 1,
	 .data_pullup = 0,
	 .is_8bit = 1,
	 .devtype = SDIO_DEV_TYPE_EMMC,
	 .flags = KONA_SDIO_FLAGS_DEVICE_NON_REMOVABLE,
	 .peri_clk_name = "sdio2_clk",
	 .ahb_clk_name = "sdio2_ahb_clk",
	 .sleep_clk_name = "sdio2_sleep_clk",
	 .peri_clk_rate = 52000000,
	 },
#ifdef CONFIG_BRCM_UNIFIED_DHD_SUPPORT
	{
	 .id = 2,
	 .data_pullup = 0,
	 .devtype = SDIO_DEV_TYPE_WIFI,
	 .flags = KONA_SDIO_FLAGS_DEVICE_REMOVABLE,
	 .peri_clk_name = "sdio3_clk",
	 .ahb_clk_name = "sdio3_ahb_clk",
	 .sleep_clk_name = "sdio3_sleep_clk",
	 .peri_clk_rate = 48000000,
	 .register_status_notify = hawaii_wifi_status_register,
	 },
#else
	{
	 .id = 2,
	 .data_pullup = 0,
	 .devtype = SDIO_DEV_TYPE_WIFI,
	 .wifi_gpio = {
		       .reset = 3,
		       .reg = -1,
		       .host_wake = 74,
		       .shutdown = -1,
		       },
	 .flags = KONA_SDIO_FLAGS_DEVICE_REMOVABLE,
	 .peri_clk_name = "sdio3_clk",
	 .ahb_clk_name = "sdio3_ahb_clk",
	 .sleep_clk_name = "sdio3_sleep_clk",
	 .peri_clk_rate = 48000000,
	 },
#endif
};

#ifdef CONFIG_BACKLIGHT_PWM

static struct platform_pwm_backlight_data hawaii_backlight_data = {
/* backlight */
	.pwm_id = 2,
	.max_brightness = 32,	/* Android calibrates to 32 levels */
	.dft_brightness = 32,
	.polarity = 1,		/* Inverted polarity */
	.pwm_period_ns = 1000000,
};

#endif /*CONFIG_BACKLIGHT_PWM */

#ifdef CONFIG_VIDEO_KONA
static struct ov5640_platform_data ov5640_cam1_pdata = {
	.s_power = hawaii_ov_cam1_power,
};

struct unicam_subdev_i2c_board_info ov5640_cam1_i2c_device = {
	.board_info = {
		       I2C_BOARD_INFO("ov5640-mc", OV5640_I2C_ADDRESS),
		       .platform_data = &ov5640_cam1_pdata,
		       },
	.i2c_adapter_id = 0,
};

static struct unicam_v4l2_subdevs_groups hawaii_unicam_subdevs[] = {
	{
	 /* ov5640 */
	 .i2c_info = &ov5640_cam1_i2c_device,
	 .interface = UNICAM_INTERFACE_CSI2_PHY1,
	 .bus = {
		 .csi2 = {
			  .lanes = CSI2_DUAL_LANE_SENSOR,
			  .port = UNICAM_PORT_AFE_0,
			  },
		 },
	 },
};

static struct unicam_platform_data hawaii_unicam_pdata = {
	.subdevs = hawaii_unicam_subdevs,
	.num_subdevs = ARRAY_SIZE(hawaii_unicam_subdevs),
};

static struct resource hawaii_unicam_rsrc[] = {
	[0] = {
	       .start = BCM_INT_ID_CSI,
	       .end = BCM_INT_ID_CSI,
	       .flags = IORESOURCE_IRQ,
	       },
};

static struct platform_device hawaii_unicam_device = {
	/* adding prefix mc to differ from char unicam interface */
	.name = "kona-unicam-mc",
	.id = 0,
	.resource = hawaii_unicam_rsrc,
	.num_resources = ARRAY_SIZE(hawaii_unicam_rsrc),
	.dev = {
		.platform_data = &hawaii_unicam_pdata,
		},
};

late_initcall(hawaii_camera_init);
#endif
/* Remove this comment when camera data for Hawaii is updated */

#if defined(CONFIG_SENSORS_BMA222)
static struct i2c_board_info __initdata bma222_accl_info[] = {
	{
	 I2C_BOARD_INFO("bma222_accl", 0x08),
	 .irq = -1,
	 .platform_data = &bma_pdata,
	 },
};
#endif

#ifdef CONFIG_TOUCHSCREEN_FT5306
static int ts_power(ts_power_status vreg_en)
{
	struct regulator *reg = NULL;
	if (!reg) {
/* Remove this comment when the regulator references are fixed here for Hawaii */
		reg = regulator_get(NULL, "hv8");
		if (!reg || IS_ERR(reg)) {
			pr_err("No Regulator available for ldo_hv8\n");
			return -1;
		}
	}
	if (reg) {
		if (vreg_en) {
			regulator_set_voltage(reg, 3000000, 3000000);
			pr_err("Turn on TP (ldo_hv8) to 2.8V\n");
			regulator_enable(reg);
		} else {
			pr_err("Turn off TP (ldo_hv8)\n");
			regulator_disable(reg);
		}
	} else {
		pr_err("TP Regulator Alloc Failed");
		return -1;
	}
	return 0;
}

static struct Synaptics_ts_platform_data ft5306_plat_data = {
	.power = ts_power,
};

static struct i2c_board_info __initdata ft5306_info[] = {
	{			/* New touch screen i2c slave address. */
	 I2C_BOARD_INFO("FocalTech-Ft5306", (0x70 >> 1)),
	 .platform_data = &ft5306_plat_data,
	 .irq = gpio_to_irq(TSC_GPIO_IRQ_PIN),
	 },
};
#endif

#if defined(CONFIG_TOUCHSCREEN_BCM915500) || defined(CONFIG_TOUCHSCREEN_BCM915500_MODULE)
static struct bcm915500_platform_data bcm915500_i2c_param = {
	.id = 3,
	.i2c_adapter_id = 3,
	.gpio_reset = TSC_GPIO_RESET_PIN,
	.gpio_interrupt = TSC_GPIO_IRQ_PIN,
};

static struct i2c_board_info bcm915500_i2c_boardinfo[] = {
	{
	 .type = BCM915500_TSC_NAME,
	 .addr = HW_BCM915500_SLAVE_SPM,
	 .platform_data = &bcm915500_i2c_param,
	 .irq = gpio_to_irq(TSC_GPIO_IRQ_PIN),
	 },
};
#endif

#if defined(CONFIG_BCM_ALSA_SOUND)
static struct caph_platform_cfg board_caph_platform_cfg =
#ifdef HW_CFG_CAPH
	HW_CFG_CAPH;
#else
{
	.aud_ctrl_plat_cfg = {
			      .ext_aud_plat_cfg = {
						   .ihf_ext_amp_gpio = -1,
						   }
			      }
};
#endif

static struct platform_device board_caph_device = {
	.name = "brcm_caph_device",
	.id = -1,		/*Indicates only one device */
	.dev = {
		.platform_data = &board_caph_platform_cfg,
		},
};

#endif /* CONFIG_BCM_ALSA_SOUND */

static struct platform_device *hawaii_devices[] __initdata = {
#ifdef CONFIG_KEYBOARD_BCM
	&hawaii_kp_device,
#endif
#ifdef CONFIG_KONA_HEADSET_MULTI_BUTTON
	&hawaii_headset_device,
#endif

#ifdef CONFIG_DMAC_PL330
	&hawaii_pl330_dmac_device,
#endif

#ifdef CONFIG_HAPTIC_SAMSUNG_PWM
	&haptic_pwm_device,
#endif

#ifdef CONFIG_BACKLIGHT_PWM
	&hawaii_backlight_device,
#endif

#if defined(CONFIG_BCM_BT_LPM) && !defined(CONFIG_OF_DEVICE)
	&board_bcmbt_lpm_device,
#endif

#ifdef CONFIG_VIDEO_KONA
	&hawaii_unicam_device,
#endif

#if defined(CONFIG_BCM_ALSA_SOUND)
	&board_caph_device,
#endif

};

struct platform_device *hawaii_sdio_devices[] __initdata = {
	&hawaii_sdio2_device,
	&hawaii_sdio3_device,
	&hawaii_sdio1_device,
};

static void __init hawaii_add_i2c_devices(void)
{

#ifdef CONFIG_VIDEO_ADP1653
	i2c_register_board_info(0, adp1653_flash, ARRAY_SIZE(adp1653_flash));
#endif

#ifdef CONFIG_TOUCHSCREEN_FT5306
	i2c_register_board_info(3, ft5306_info, ARRAY_SIZE(ft5306_info));
#endif

#ifdef CONFIG_SENSORS_BMA222
	i2c_register_board_info(2, bma222_accl_info,
				ARRAY_SIZE(bma222_accl_info));
#endif

#if defined(CONFIG_INV_MPU_IIO) || defined(CONFIG_INV_MPU_IIO_MODULE)
#if defined(INV_MPU_IRQ_GPIO)
	inv_mpu_i2c0_boardinfo[0].irq = gpio_to_irq(INV_MPU_IRQ_GPIO);
#endif
	i2c_register_board_info(INV_MPU_I2C_BUS_ID,
				inv_mpu_i2c0_boardinfo,
				ARRAY_SIZE(inv_mpu_i2c0_boardinfo));
#endif /* CONFIG_INV_MPU_IIO */

#if defined(CONFIG_MPU_SENSORS_KXTF9)
	inv_mpu_kxtf9_boardinfo[0].irq =
	    gpio_to_irq(MPU_SENSORS_KXTF9_IRQ_GPIO);
	i2c_register_board_info(0, inv_mpu_kxtf9_boardinfo,
				ARRAY_SIZE(inv_mpu_kxtf9_boardinfo));
#endif /* CONFIG_MPU_SENSORS_KXTF9 */

#if defined(CONFIG_SENSORS_KIONIX_KXTIK)	\
			|| defined(CONFIG_SENSORS_KIONIX_KXTIK_MODULE)
	i2c_register_board_info(KXTIK_I2C_BUS_ID,
				kxtik_i2c_boardinfo,
				ARRAY_SIZE(kxtik_i2c_boardinfo));
#endif /* CONFIG_SENSORS_KIONIX_KXTIK */

#if defined(CONFIG_SENSORS_AK8963) || defined(CONFIG_SENSORS_AK8963_MODULE)
	i2c_register_board_info(AKM8963_I2C_BUS_ID,
				km8963_info, ARRAY_SIZE(akm8963_info));
#endif /* CONFIG_SENSORS_AK8963 */

#if defined(CONFIG_TOUCHSCREEN_BCM915500) || defined(CONFIG_TOUCHSCREEN_BCM915500_MODULE)
	i2c_register_board_info(3, bcm915500_i2c_boardinfo,
				ARRAY_SIZE(bcm915500_i2c_boardinfo));
#endif

#if defined(CONFIG_BCMI2CNFC)
	i2c_register_board_info(1, bcmi2cnfc, ARRAY_SIZE(bcmi2cnfc));
#endif

#if  defined(CONFIG_BMP18X) || defined(CONFIG_BMP18X_I2C) || defined(CONFIG_BMP18X_I2C_MODULE)
	i2c_register_board_info(
#ifdef BMP18X_I2C_BUS_ID
				       BMP18X_I2C_BUS_ID,
#else
				       -1,
#endif
				       i2c_bmp18x_info,
				       ARRAY_SIZE(i2c_bmp18x_info));
#endif

#if defined(CONFIG_AL3006) || defined(CONFIG_AL3006_MODULE)
#ifdef AL3006_IRQ
	i2c_al3006_info[0].irq = gpio_to_irq(AL3006_IRQ_GPIO);
#else
	i2c_al3006_info[0].irq = -1;
#endif
	i2c_register_board_info(
#ifdef AL3006_I2C_BUS_ID
				       AL3006_I2C_BUS_ID,
#else
				       -1,
#endif
				       i2c_al3006_info,
				       ARRAY_SIZE(i2c_al3006_info));
#endif /* CONFIG_AL3006 */

#if  defined(CONFIG_MPU_SENSORS_AMI306) || defined(CONFIG_MPU_SENSORS_AMI306)
	i2c_register_board_info(
#ifdef AMI306_I2C_BUS_ID
				       AMI306_I2C_BUS_ID,
#else
				       -1,
#endif
				       i2c_ami306_info,
				       ARRAY_SIZE(i2c_ami306_info));
#endif

}

#ifdef CONFIG_ION_BCM_NO_DT
#ifdef CONFIG_IOMMU_API
struct bcm_iommu_pdata iommu_mm_pdata = {
	.name        = "iommu-mm",
	.iova_begin  = 0x80000000,
	.iova_size   = 0x80000000,
	.errbuf_size = 0x1000,
	.skip_enable = 1,
};
#endif
#ifdef CONFIG_BCM_IOVMM
struct bcm_iovmm_pdata iovmm_mm_pdata = {
	.name = "iovmm-mm",
	.base = 0xc0000000,
	.size = 0x30000000,
	.order = 0,
};
struct bcm_iovmm_pdata iovmm_mm_256mb_pdata = {
	.name = "iovmm-mm-256mb",
	.base = 0xf0000000,
	.size = 0x0bf00000,
	.order = 0,
};
#endif
#endif /* CONFIG_ION_BCM_NO_DT */

#define HS_IRQ		gpio_to_irq(92)

static void hawaii_add_pdata(void)
{
	hawaii_sdio1_device.dev.platform_data = &hawaii_sdio_param[0];
	hawaii_sdio2_device.dev.platform_data = &hawaii_sdio_param[1];
	hawaii_sdio3_device.dev.platform_data = &hawaii_sdio_param[2];
	hawaii_ssp0_device.dev.platform_data = &hawaii_ssp0_info;
	hawaii_ssp1_device.dev.platform_data = &hawaii_ssp1_info;
	hawaii_stm_device.dev.platform_data = &hawaii_stm_pdata;
	hawaii_headset_device.dev.platform_data = &hawaii_headset_data;
	/* The resource in position 2 (starting from 0) is used to fill
	 * the GPIO number. The driver file assumes this. So put the
	 * board specific GPIO number here
	 */
	hawaii_headset_device.resource[2].start = HS_IRQ;
	hawaii_headset_device.resource[2].end   = HS_IRQ;
	hawaii_pl330_dmac_device.dev.platform_data = &hawaii_pl330_pdata;
	hawaii_backlight_device.dev.platform_data = &hawaii_backlight_data;
#ifdef CONFIG_USB_DWC_OTG
	hawaii_hsotgctrl_platform_device.dev.platform_data =
	    &hsotgctrl_plat_data;
	hawaii_usb_phy_platform_device.dev.platform_data =
		&hsotgctrl_plat_data;
#endif
#ifdef CONFIG_ION_BCM_NO_DT
#ifdef CONFIG_IOMMU_API
	iommu_mm_device.dev.platform_data = &iommu_mm_pdata;
#endif
#ifdef CONFIG_BCM_IOVMM
	iovmm_mm_device.dev.platform_data = &iovmm_mm_pdata;
	iovmm_mm_256mb_device.dev.platform_data = &iovmm_mm_256mb_pdata;
	ion_system_device.dev.platform_data = &ion_system_data;
	ion_system_extra_device.dev.platform_data = &ion_system_extra_data;
#endif
#endif /* CONFIG_ION_BCM_NO_DT */
}

void __init hawaii_add_common_devices(void)
{
	platform_add_devices(hawaii_common_plat_devices,
			     ARRAY_SIZE(hawaii_common_plat_devices));
}

static void __init hawaii_add_devices(void)
{

	hawaii_add_pdata();

#ifdef CONFIG_KEYBOARD_BCM
	hawaii_kp_device.dev.platform_data = &hawaii_keypad_data;
#endif

#ifdef CONFIG_ION_BCM_NO_DT
#ifdef CONFIG_IOMMU_API
	platform_device_register(&iommu_mm_device);
#endif
#ifdef CONFIG_BCM_IOVMM
	platform_device_register(&iovmm_mm_device);
	platform_device_register(&iovmm_mm_256mb_device);
	platform_device_register(&ion_system_device);
	platform_device_register(&ion_system_extra_device);
#endif
	platform_device_register(&ion_carveout_device);
#ifdef CONFIG_CMA
	platform_device_register(&ion_cma_device);
#endif
#if defined(CONFIG_MM_SECURE_DRIVER)
	platform_device_register(&ion_secure_device);
#endif /* CONFIG_MM_SECURE_DRIVER */
#endif /* CONFIG_ION_BCM_NO_DT */

	platform_add_devices(hawaii_devices, ARRAY_SIZE(hawaii_devices));

	hawaii_add_i2c_devices();

	spi_register_board_info(spi_slave_board_info,
				ARRAY_SIZE(spi_slave_board_info));

}

static void __init hawaii_add_sdio_devices(void)
{
	platform_add_devices(hawaii_sdio_devices,
			     ARRAY_SIZE(hawaii_sdio_devices));
}

#ifdef CONFIG_FB_BRCM_KONA
/*
 * KONA FRAME BUFFER DISPLAY DRIVER PLATFORM CONFIG
 */
struct kona_fb_platform_data konafb_devices[] __initdata = {
	{
	 .dispdrv_name = "NT35516",
	 .dispdrv_entry = DISP_DRV_NT35516_GetFuncTable,
	 .parms = {
		   .w0 = {
			  .bits = {
				   .boot_mode = 0,
				   .bus_type = KONA_BUS_DSI,
				   .bus_no = KONA_BUS_0,
				   .bus_ch = KONA_BUS_CH_0,
				   .bus_width = 3,
				   .te_input = KONA_TE_IN_1_DSI0,
				   .col_mode_i = KONA_CM_I_XRGB888,
				   .col_mode_o = KONA_CM_O_RGB888,
				   },
			  },
		   .w1 = {
			  .bits = {
				   .api_rev = KONA_LCD_BOOT_API_REV,
				   .lcd_rst0 = 22,
				   },
			  },
		   },
	 },
};

#include "kona_fb_init.c"
#endif /* #ifdef CONFIG_FB_BRCM_KONA */

static struct of_device_id hawaii_dt_match_table[] __initdata = {
	{ .compatible = "simple-bus", },
	{}
};

static void __init hawaii_init(void)
{
	hawaii_add_devices();
#ifdef CONFIG_FB_BRCM_KONA
	konafb_init();
#endif
	hawaii_add_common_devices();
	/* Populate platform_devices from device tree data */
	of_platform_populate(NULL, hawaii_dt_match_table,
			hawaii_auxdata_lookup, NULL);
	return;
}

static int __init hawaii_add_lateinit_devices(void)
{
	hawaii_add_sdio_devices();

#ifdef CONFIG_BRCM_UNIFIED_DHD_SUPPORT
	hawaii_wlan_init();
#endif
	return 0;
}

late_initcall(hawaii_add_lateinit_devices);

static const char *hawaii_dt_compat[] = { "bcm,hawaii", NULL, };
DT_MACHINE_START(HAWAII, "hawaii")
	.map_io = hawaii_map_io,
	.init_irq = kona_init_irq,
	.handle_irq = gic_handle_irq,
	.timer = &kona_timer,
	.init_machine = hawaii_init,
	.reserve = hawaii_reserve,
	.restart = hawaii_restart,
	.dt_compat = hawaii_dt_compat,
MACHINE_END
