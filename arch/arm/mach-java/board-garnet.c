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
#include <mach/pinmux.h>

#ifdef CONFIG_ANDROID_PMEM
#include <linux/android_pmem.h>
#endif
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
#ifdef CONFIG_TOUCHSCREEN_TANGO
#include <linux/i2c/tango_ts.h>
#endif
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

#ifdef CONFIG_BCM_BT_LPM
#include <linux/broadcom/bcmbt_lpm.h>
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

#ifdef CONFIG_BACKLIGHT_PWM
#include <linux/pwm_backlight.h>
#endif

#if defined(CONFIG_BCM_ALSA_SOUND)
#include <mach/caph_platform.h>
#include <mach/caph_settings.h>
#endif
#ifdef CONFIG_VIDEO_UNICAM_CAMERA
#include <media/soc_camera.h>
#endif

#ifdef CONFIG_VIDEO_KONA
#include <media/v4l2-device.h>
#include <media/v4l2-subdev.h>
#include <media/kona-unicam.h>
#ifdef CONFIG_SOC_CAMERA_OV5648
	#include <media/ov5648.h>
#endif
#ifdef CONFIG_SOC_CAMERA_OV5640
	#include <media/ov5640.h>
#endif
#ifdef CONFIG_SOC_CAMERA_OV8825
	#include <media/ov8825.h>
#endif
#endif

#ifdef CONFIG_VIDEO_A3907
#include <media/a3907.h>
#endif

#ifdef CONFIG_VIDEO_DW9714
#include <media/dw9714.h>
#endif

#ifdef CONFIG_WD_TAPPER
#include <linux/broadcom/wd-tapper.h>
#endif

#if defined(CONFIG_TOUCHSCREEN_BCM15500) || \
defined(CONFIG_TOUCHSCREEN_BCM15500_MODULE)
#include <linux/i2c/bcmtch15xxx.h>
#include <linux/i2c/bcm15500_settings.h>
#endif

#ifdef CONFIG_USB_DWC_OTG
#include <linux/usb/bcm_hsotgctrl.h>
#include <linux/usb/otg.h>
#endif

#if defined(CONFIG_TMD2771)
#include <linux/i2c/taos_common.h>
#endif

#ifdef CONFIG_KONA_SECURE_MEMC
#include <plat/kona_secure_memc.h>
#include <plat/kona_secure_memc_settings.h>
#endif

#ifdef CONFIG_BRCM_UNIFIED_DHD_SUPPORT
#include "java_wifi.h"

extern int
hawaii_wifi_status_register(void (*callback) (int card_present, void *dev_id),
			    void *dev_id);
#endif

/* SD */
#define SD_CARDDET_GPIO_PIN	91

#ifndef CONFIG_BRD_NAME
#define CONFIG_BRD_NAME "hawaii"
#endif

/* Touch */
#define TSC_GPIO_IRQ_PIN			73

#define TSC_GPIO_RESET_PIN			70
#define TSC_GPIO_WAKEUP_PIN         70

#define TANGO_I2C_TS_DRIVER_NUM_BYTES_TO_READ	14

#ifdef CONFIG_ANDROID_PMEM
struct android_pmem_platform_data android_pmem_data = {
	.name = "pmem",
	.cmasize = 0,
	.carveout_base = 0x9e000000,
	.carveout_size = 0,
};
#endif

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

#ifdef CONFIG_VIDEO_AS3643
#define AS3643_I2C_ADDR 0x60
static struct i2c_board_info as3643_flash[] = {
	{
	 I2C_BOARD_INFO("as3643", (AS3643_I2C_ADDR >> 1))
	 },
};
#endif
#ifdef CONFIG_VIDEO_UNICAM_CAMERA


static struct regulator *d_gpsr_cam0_1v8;
static struct regulator *d_lvldo2_cam1_1v8;
static struct regulator *d_1v8_mmc1_vcc;
static struct regulator *d_3v0_mmc1_vcc;

/* The pre-div clock rate needs to satisfy
   the rate requirements of all digital
   channel clocks in use. */
#define SENSOR_PREDIV_CLK               "dig_prediv_clk"

#define SENSOR_0_GPIO_PWRDN             (002)
#define SENSOR_0_GPIO_RST               (111)
#define SENSOR_0_CLK                    "dig_ch0_clk"	/*DCLK1 */
#if defined(CONFIG_SOC_CAMERA_OV2675) || defined(CONFIG_SOC_CAMERA_GC2035)
#define SENSOR_PREDIV_CLK_FREQ         (312000000)
#define SENSOR_0_CLK_FREQ               (26000000)
#elif defined(CONFIG_SOC_CAMERA_OV5648)
#define SENSOR_PREDIV_CLK_FREQ         (312000000)
#define SENSOR_0_CLK_FREQ               (24000000)
#elif defined(CONFIG_SOC_CAMERA_OV5640) || defined(CONFIG_SOC_CAMERA_OV8825)
#define SENSOR_PREDIV_CLK_FREQ          (26000000)
#define SENSOR_0_CLK_FREQ               (13000000)
#else
#error "SENSOR_0_CLK_FREQ not defined"
#endif
#define CSI0_LP_FREQ			(100000000)
#define CSI1_LP_FREQ			(100000000)

#define SENSOR_1_CLK                    "dig_ch0_clk"	/* DCLK1 */
#define SENSOR_1_CLK_FREQ               (26000000)

#define SENSOR_1_GPIO_PWRDN             (005)
#define SENSOR_1_GPIO_RST               (121)

#if defined(CONFIG_MACH_JAVA_C_LC1)

#define MAIN_CAM_AF_ENABLE			(33)
#define TORCH_EN (10)
#define FLASH_EN (11)

void set_af_enable(int on)
{

	if (on) {
		gpio_set_value(MAIN_CAM_AF_ENABLE, 1);
		usleep_range(10000, 10010);
		} else {
		gpio_set_value(MAIN_CAM_AF_ENABLE, 0);
		usleep_range(10000, 10010);
		}
}
#endif

static int hawaii_camera_power(struct device *dev, int on)
{
	unsigned int value;
	int ret = -1;
	struct clk *clock;
	struct clk *prediv_clock;
	struct clk *lp_clock;
	struct clk *axi_clk;
	static struct pi_mgr_dfs_node unicam_dfs_node;
	struct soc_camera_device *icd = to_soc_camera_dev(dev);
	struct soc_camera_link *icl = to_soc_camera_link(icd);

	printk(KERN_INFO "%s:camera power %s\n", __func__, (on ? "on" : "off"));
	if (!unicam_dfs_node.valid) {
		ret = pi_mgr_dfs_add_request(&unicam_dfs_node, "unicam",
					     PI_MGR_PI_ID_MM,
					     PI_MGR_DFS_MIN_VALUE);
		if (ret) {
			return -1;
		}
	#ifdef CONFIG_SOC_CAMERA_OV5648
		if (gpio_request_one(SENSOR_0_GPIO_RST, GPIOF_DIR_OUT |
				     GPIOF_INIT_HIGH, "Cam0Rst")) {
			printk(KERN_ERR "Unable to get cam0 RST GPIO\n");
			return -1;
		}
		if (gpio_request_one(SENSOR_0_GPIO_PWRDN, GPIOF_DIR_OUT |
				     GPIOF_INIT_LOW, "CamPWDN")) {
			printk(KERN_ERR "Unable to get cam0 PWDN GPIO\n");
			return -1;
		}
	#if defined(CONFIG_MACH_JAVA_C_LC1)
		if (gpio_request_one(MAIN_CAM_AF_ENABLE, GPIOF_DIR_OUT |
				     GPIOF_INIT_LOW, "Cam0_af_enable")) {
			printk(KERN_ERR "Unable to get cam0 af enable GPIO\n");
			return -1;
		}
		if (gpio_request_one(TORCH_EN, GPIOF_DIR_OUT |
				     GPIOF_INIT_LOW, "cam0_torch_enable")) {
			printk(KERN_ERR "Unable to get cam0 torch enable GPIO\n");
			return -1;
		}

		if (gpio_request_one(FLASH_EN, GPIOF_DIR_OUT |
				     GPIOF_INIT_LOW, "cam0_flash_enable")) {
			printk(KERN_ERR "Unable to get cam0 torch enable GPIO\n");
			return -1;
		}
	#endif
	#endif
	#ifdef CONFIG_SOC_CAMERA_OV5640
		if (gpio_request_one(SENSOR_0_GPIO_RST, GPIOF_DIR_OUT |
				     GPIOF_INIT_LOW, "Cam0Rst")) {
			printk(KERN_ERR "Unable to get cam0 RST GPIO\n");
			return -1;
		}
		if (gpio_request_one(SENSOR_0_GPIO_PWRDN, GPIOF_DIR_OUT |
				     GPIOF_INIT_HIGH, "CamPWDN")) {
			printk(KERN_ERR "Unable to get cam0 PWDN GPIO\n");
			return -1;
		}
	#endif

	#ifdef CONFIG_SOC_CAMERA_OV8825
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
	#endif

		if (gpio_request_one(SENSOR_1_GPIO_PWRDN, GPIOF_DIR_OUT |
				 GPIOF_INIT_LOW, "Cam1PWDN")) {
			printk(KERN_ERR "Unable to get CAM1PWDN\n");
			return -1;
		}

		/*MMC1 VCC */
		d_1v8_mmc1_vcc = regulator_get(NULL, icl->regulators[1].supply);
		if (IS_ERR_OR_NULL(d_1v8_mmc1_vcc))
			printk(KERN_ERR "Failed to  get d_1v8_mmc1_vcc\n");
		d_3v0_mmc1_vcc = regulator_get(NULL, icl->regulators[2].supply);
		if (IS_ERR_OR_NULL(d_3v0_mmc1_vcc))
			printk(KERN_ERR "Failed to  get d_3v0_mmc1_vcc\n");
		d_gpsr_cam0_1v8 = regulator_get(NULL,
			icl->regulators[0].supply);
		if (IS_ERR_OR_NULL(d_gpsr_cam0_1v8))
			printk(KERN_ERR "Failed to  get d_gpsr_cam0_1v8\n");
		if (d_lvldo2_cam1_1v8 == NULL) {
			d_lvldo2_cam1_1v8 = regulator_get(NULL,
			icl->regulators[3].supply);
			if (IS_ERR_OR_NULL(d_lvldo2_cam1_1v8))
				printk(KERN_ERR "Fd_lvldo2_cam1_1v8 cam\n");
		}
	}

	ret = -1;
	lp_clock = clk_get(NULL, CSI0_LP_PERI_CLK_NAME_STR);
	if (IS_ERR_OR_NULL(lp_clock)) {
		printk(KERN_ERR "Unable to get %s clock\n",
		CSI0_LP_PERI_CLK_NAME_STR);
		goto e_clk_get;
	}
	prediv_clock = clk_get(NULL, SENSOR_PREDIV_CLK);
	if (IS_ERR_OR_NULL(prediv_clock)) {
		printk(KERN_ERR "Unable to get SENSOR_PREDIV_CLK clock\n");
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
		regulator_enable(d_gpsr_cam0_1v8);
		usleep_range(1000, 1010);
		regulator_enable(d_1v8_mmc1_vcc);
		usleep_range(1000, 1010);
		regulator_enable(d_lvldo2_cam1_1v8);
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
		value = clk_enable(prediv_clock);
		if (value) {
			pr_err("Failed to enable prediv clock\n");
			goto e_clk_prediv;
		}
		value = clk_enable(clock);
		if (value) {
			pr_err("Failed to enable sensor 0 clock\n");
			goto e_clk_sensor;
		}
		value = clk_set_rate(prediv_clock, SENSOR_PREDIV_CLK_FREQ);
		if (value) {
			pr_err("Failed to set prediv clock\n");
			goto e_clk_set_prediv;
		}
		value = clk_set_rate(clock, SENSOR_0_CLK_FREQ);
		if (value) {
			pr_err("Failed to set sensor0 clock\n");
			goto e_clk_set_sensor;
		}
		usleep_range(10000, 10100);
#ifdef CONFIG_SOC_CAMERA_OV5640
		gpio_set_value(SENSOR_0_GPIO_RST, 0);
		usleep_range(10000, 10100);
		gpio_set_value(SENSOR_0_GPIO_PWRDN, 0);
		usleep_range(5000, 5100);
		gpio_set_value(SENSOR_0_GPIO_RST, 1);
#endif
#ifdef CONFIG_SOC_CAMERA_OV5648
		gpio_set_value(SENSOR_0_GPIO_PWRDN, 1);
		usleep_range(5000, 5100);
		gpio_set_value(SENSOR_0_GPIO_RST, 1);
#endif
#ifdef CONFIG_SOC_CAMERA_OV8825
		gpio_set_value(SENSOR_0_GPIO_PWRDN, 1);
		usleep_range(5000, 5100);
		gpio_set_value(SENSOR_0_GPIO_RST, 1);
#endif
		msleep(30);

		regulator_enable(d_3v0_mmc1_vcc);
		usleep_range(1000, 1010);

#if defined(CONFIG_SOC_CAMERA_OV7695)
		gpio_set_value(SENSOR_1_GPIO_PWRDN, 0);
#else
		gpio_set_value(SENSOR_1_GPIO_PWRDN, 1);/*disable front camera*/
#endif


#ifdef CONFIG_MACH_JAVA_C_LC1
		set_af_enable(1);
#endif

#ifdef CONFIG_VIDEO_A3907
		a3907_enable(1);
#endif

#ifdef CONFIG_VIDEO_DW9714
		dw9714_enable(1);
#endif

	} else {
#ifdef CONFIG_VIDEO_A3907
		a3907_enable(0);
#endif

#ifdef CONFIG_VIDEO_DW9714
		dw9714_enable(0);
#endif

#ifdef CONFIG_MACH_JAVA_C_LC1
		set_af_enable(0);
#endif
#ifdef CONFIG_SOC_CAMERA_OV5640
		gpio_set_value(SENSOR_0_GPIO_RST, 0);
		usleep_range(1000, 1100);
		gpio_set_value(SENSOR_0_GPIO_PWRDN, 1);
#endif
#ifdef CONFIG_SOC_CAMERA_OV5648
		usleep_range(5000, 5100);
		gpio_set_value(SENSOR_0_GPIO_PWRDN, 0);
		usleep_range(1000, 1100);
		gpio_set_value(SENSOR_0_GPIO_RST, 0);
#endif

#ifdef CONFIG_SOC_CAMERA_OV8825
		gpio_set_value(SENSOR_0_GPIO_PWRDN, 0);
#endif

		gpio_set_value(SENSOR_1_GPIO_PWRDN, 0);/*disable front camera*/

		clk_disable(prediv_clock);
		clk_disable(clock);
		clk_disable(lp_clock);
		clk_disable(axi_clk);
		regulator_disable(d_3v0_mmc1_vcc);
		regulator_disable(d_lvldo2_cam1_1v8);
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
e_clk_set_prediv:
	clk_disable(prediv_clock);
e_clk_prediv:
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
	struct soc_camera_device *icd = to_soc_camera_dev(dev);
	struct soc_camera_link *icl = to_soc_camera_link(icd);


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
#if defined(CONFIG_SOC_CAMERA_GC2035)
		if (gpio_request_one(SENSOR_1_GPIO_RST, GPIOF_DIR_OUT |
				     GPIOF_INIT_LOW, "Cam1PWDN")) {
			printk(KERN_ERR "Unable to get CAM1PWDN\n");
			return -1;
		}
#endif

		d_lvldo2_cam1_1v8 = regulator_get(NULL,
			icl->regulators[0].supply);
		if (IS_ERR_OR_NULL(d_lvldo2_cam1_1v8))
			printk(KERN_ERR "Failed to get d_lvldo2_cam1_1v8\n");
		if (d_1v8_mmc1_vcc == NULL) {
			d_1v8_mmc1_vcc = regulator_get(NULL,
				icl->regulators[1].supply);
			if (IS_ERR_OR_NULL(d_1v8_mmc1_vcc))
				printk(KERN_ERR "Err d_1v8_mmc1_vcc\n");
		}
		if (d_3v0_mmc1_vcc == NULL) {
			d_3v0_mmc1_vcc = regulator_get(NULL,
			icl->regulators[2].supply);
			if (IS_ERR_OR_NULL(d_3v0_mmc1_vcc))
				printk(KERN_ERR "d_3v0_mmc1_vcc");
		}
		if (d_gpsr_cam0_1v8 == NULL) {
			d_gpsr_cam0_1v8 = regulator_get(NULL,
			icl->regulators[3].supply);
			if (IS_ERR_OR_NULL(d_gpsr_cam0_1v8))
				printk(KERN_ERR "Fl d_gpsr_cam0_1v8 get	fail");
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
		#if defined(CONFIG_SOC_CAMERA_OV7695) || \
			defined(CONFIG_SOC_CAMERA_GC2035)
		gpio_set_value(SENSOR_1_GPIO_PWRDN, 0);
		#else
		gpio_set_value(SENSOR_1_GPIO_PWRDN, 1);
		#endif
#if defined(CONFIG_SOC_CAMERA_GC2035)
		gpio_set_value(SENSOR_1_GPIO_RST, 0);
#endif

		usleep_range(5000, 5010);
		regulator_enable(d_lvldo2_cam1_1v8);
		usleep_range(1000, 1010);
		regulator_enable(d_1v8_mmc1_vcc);
		usleep_range(1000, 1010);
		/* Secondary cam addition */
		regulator_enable(d_gpsr_cam0_1v8);
		usleep_range(1000, 1010);
		regulator_enable(d_3v0_mmc1_vcc);
		usleep_range(1000, 1010);

#if defined(CONFIG_SOC_CAMERA_GC2035)
		gpio_set_value(SENSOR_1_GPIO_RST, 1);
#endif

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
		#if defined(CONFIG_SOC_CAMERA_OV7695)
		gpio_set_value(SENSOR_1_GPIO_PWRDN, 1);
		#else
		gpio_set_value(SENSOR_1_GPIO_PWRDN, 0);
		#endif
		msleep(30);
	} else {
		#if defined(CONFIG_SOC_CAMERA_OV7695)
		gpio_set_value(SENSOR_1_GPIO_PWRDN, 0);
		#else
		gpio_set_value(SENSOR_1_GPIO_PWRDN, 1);
		#endif
		clk_disable(lp_clock_0);
		clk_disable(lp_clock_1);
		clk_disable(clock);
		clk_disable(axi_clk);
		clk_disable(axi_clk_0);
		regulator_disable(d_lvldo2_cam1_1v8);
		regulator_disable(d_1v8_mmc1_vcc);
		regulator_disable(d_gpsr_cam0_1v8);
		regulator_disable(d_3v0_mmc1_vcc);
		usleep_range(1000, 1010);
		gpio_set_value(SENSOR_1_GPIO_PWRDN, 0);
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

static struct soc_camera_link iclink_main = {
	.power = &hawaii_camera_power,
	.reset = &hawaii_camera_reset,
};
static struct soc_camera_link iclink_front = {

	.power = &hawaii_camera_power_front,
	.reset = &hawaii_camera_reset_front,
};

#ifdef CONFIG_SOC_CAMERA_OV8825
static struct soc_camera_link iclink_ov8825 = {
	.power = &hawaii_camera_power,
	.reset = &hawaii_camera_reset,
};
#endif

#endif /* CONFIG_VIDEO_UNICAM_CAMERA */

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

#ifdef CONFIG_BRCM_CDC
	&brcm_cdc_device,
#endif

#ifdef CONFIG_UNICAM
	&hawaii_unicam_device,
#endif
#ifdef CONFIG_VIDEO_UNICAM_CAMERA
	&hawaii_camera_device,
#endif

#ifdef CONFIG_SND_BCM_SOC
	&hawaii_audio_device,
	&caph_i2s_device,
	&caph_pcm_device,
	&spdif_dit_device,
#endif

#ifdef CONFIG_KONA_SECURE_MEMC
	&kona_secure_memc_device,
#endif
};



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
#ifdef CONFIG_KONA_HEADSET_DEFAULT_STATE
	.hs_default_state = 1,
#else
	.hs_default_state = 0,
#endif
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

	/* AUDLDO supply id for changing regulator mode*/
	.ldo_id = "audldo_uc",

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

#ifdef CONFIG_BCM_BT_LPM
#define GPIO_BT_WAKE	32
#define GPIO_HOST_WAKE	72

static struct bcmbt_platform_data brcm_bt_lpm_data = {
	.bt_wake_gpio = GPIO_BT_WAKE,
	.host_wake_gpio = GPIO_HOST_WAKE,
	.bt_uart_port = 1,
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

static struct sdio_platform_cfg hawaii_sdio_param = {

#ifdef CONFIG_BRCM_UNIFIED_DHD_SUPPORT
	.register_status_notify = hawaii_wifi_status_register,
#endif

};

static struct sdio_platform_cfg hawaii_sdio0_param = {
	.configure_sdio_pullup = configure_sdio_pullup,
};

static const struct of_dev_auxdata hawaii_auxdata_lookup[] __initconst = {

	OF_DEV_AUXDATA("bcm,pwm-backlight", 0x0,
		"pwm-backlight.0", NULL),

	OF_DEV_AUXDATA("bcm,sdhci", 0x3F190000,
		"sdhci.1", NULL),

	OF_DEV_AUXDATA("bcm,sdhci", 0x3F1A0000,
		"sdhci.2", &hawaii_sdio_param),

	OF_DEV_AUXDATA("bcm,sdhci", 0x3F180000,
		"sdhci.0", &hawaii_sdio0_param),

#ifdef CONFIG_SOC_CAMERA_OV5640
	OF_DEV_AUXDATA("bcm,soc-camera", 0x3c,
		"soc-back-camera", &iclink_main),
#endif
#ifdef CONFIG_SOC_CAMERA_OV5648
	OF_DEV_AUXDATA("bcm,soc-camera", 0x36,
		"soc-back-camera", &iclink_main),
#endif
#ifdef CONFIG_SOC_CAMERA_OV7695
	OF_DEV_AUXDATA("bcm,soc-camera", 0x21,
		"soc-front-camera", &iclink_front),
#endif
#ifdef CONFIG_SOC_CAMERA_OV8825
	OF_DEV_AUXDATA("bcm,soc-camera", 0x36,
		"soc-back-camera", &iclink_ov8825),
#endif
#ifdef CONFIG_SOC_CAMERA_GC2035
	OF_DEV_AUXDATA("bcm,soc-camera", 0x3c,
		"soc-front-camera", &iclink_front),
#endif

	OF_DEV_AUXDATA("bcm,soc-camera", 0x3e,
		"soc-front-camera", &iclink_front),
	{},
};

#ifdef CONFIG_VIDEO_KONA
#ifdef CONFIG_SOC_CAMERA_OV5640
static struct ov5648_platform_data ov5640_cam1_pdata = {
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
#endif
#ifdef CONFIG_SOC_CAMERA_OV5648
static struct ov5648_platform_data ov5648_cam1_pdata = {
	.s_power = hawaii_ov_cam1_power,
};

struct unicam_subdev_i2c_board_info ov5648_cam1_i2c_device = {
	.board_info = {
		       I2C_BOARD_INFO("ov5648-mc", OV5648_I2C_ADDRESS),
		       .platform_data = &ov5648_cam1_pdata,
		       },
	.i2c_adapter_id = 0,
};

static struct unicam_v4l2_subdevs_groups hawaii_unicam_subdevs[] = {
	{
	 /* ov5648 */
	 .i2c_info = &ov5648_cam1_i2c_device,
	 .interface = UNICAM_INTERFACE_CSI2_PHY1,
	 .bus = {
		 .csi2 = {
			  .lanes = CSI2_DUAL_LANE_SENSOR,
			  .port = UNICAM_PORT_AFE_0,
			  },
		 },
	 },
};
#endif

#ifdef CONFIG_SOC_CAMERA_OV8825
static struct ov8825_platform_data ov8825_cam1_pdata = {
	.s_power = hawaii_ov_cam1_power,
};

struct unicam_subdev_i2c_board_info ov8825_cam1_i2c_device = {
	.board_info = {
		       I2C_BOARD_INFO("ov8825-mc", OV8825_I2C_ADDRESS),
		       .platform_data = &ov8825_cam1_pdata,
		       },
	.i2c_adapter_id = 0,
};

static struct unicam_v4l2_subdevs_groups hawaii_unicam_subdevs[] = {
	{
	 /* ov8825 */
	 .i2c_info = &ov8825_cam1_i2c_device,
	 .interface = UNICAM_INTERFACE_CSI2_PHY1,
	 .bus = {
		 .csi2 = {
			  .lanes = CSI2_DUAL_LANE_SENSOR,
			  .port = UNICAM_PORT_AFE_0,
			  },
		 },
	 },
};
#endif


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



#if defined(CONFIG_TOUCHSCREEN_BCM15500) ||\
defined(CONFIG_TOUCHSCREEN_BCM15500_MODULE)
static int BCMTCH_TSP_PowerOnOff(bool on)
{
	/* PLACE TOUCH CONTROLLER REGULATOR CODE HERE . SEE STEP 6 */
	return 0;
}

static struct bcmtch_platform_data bcm15500_i2c_platform_data = {
	.i2c_bus_id       = BCMTCH_HW_I2C_BUS_ID,
	.i2c_addr_spm = BCMTCH_HW_I2C_ADDR_SPM,
	.i2c_addr_sys   = BCMTCH_HW_I2C_ADDR_SYS,

	.gpio_interrupt_pin       = BCMTCH_HW_GPIO_INTERRUPT_PIN,
	.gpio_interrupt_trigger = BCMTCH_HW_GPIO_INTERRUPT_TRIGGER,

	.gpio_reset_pin           = BCMTCH_HW_GPIO_RESET_PIN,
	.gpio_reset_polarity   = BCMTCH_HW_GPIO_RESET_POLARITY,
	.gpio_reset_time_ms = BCMTCH_HW_GPIO_RESET_TIME_MS,

	.ext_button_count = BCMTCH_HW_BUTTON_COUNT,
	.ext_button_map   = bcmtch_hw_button_map,

	.axis_orientation_flag =
		((BCMTCH_HW_AXIS_REVERSE_X << BCMTCH_AXIS_FLAG_X_BIT_POS)
		|(BCMTCH_HW_AXIS_REVERSE_Y << BCMTCH_AXIS_FLAG_Y_BIT_POS)
		|(BCMTCH_HW_AXIS_SWAP_X_Y << BCMTCH_AXIS_FLAG_X_Y_BIT_POS)),

	.bcmtch_on = BCMTCH_TSP_PowerOnOff,
};

static struct i2c_board_info __initdata bcm15500_i2c_boardinfo[] = {
	{
		I2C_BOARD_INFO(BCMTCH15XXX_NAME, BCMTCH_HW_I2C_ADDR_SPM),
		.platform_data  = &bcm15500_i2c_platform_data,
		.irq	= gpio_to_irq(BCMTCH_HW_GPIO_INTERRUPT_PIN),
	},
};
#endif

#if defined(CONFIG_BCM_ALSA_SOUND)
static struct caph_platform_cfg board_caph_platform_cfg =
#if defined(CONFIG_MACH_HAWAII_GARNET_C_M530)
{
	.aud_ctrl_plat_cfg = {
			      .ext_aud_plat_cfg = {
						   .ihf_ext_amp_gpio = -1,
#if defined(CONFIG_GPIO_2IN1_SPK)
						   .spk_2in1_gpio = 11,
#endif
						   }
			      }
};
#else
#ifdef HW_CFG_CAPH
	HW_CFG_CAPH;
#else
{
	.aud_ctrl_plat_cfg = {
			      .ext_aud_plat_cfg = {
						   .ihf_ext_amp_gpio = -1,
#if defined(CONFIG_GPIO_2IN1_SPK)
						   .spk_2in1_gpio = -1,
#endif
						   }
			      }
};
#endif
#endif

static struct platform_device board_caph_device = {
	.name = "brcm_caph_device",
	.id = -1,		/*Indicates only one device */
	.dev = {
		.platform_data = &board_caph_platform_cfg,
		},
};

#ifdef CONFIG_KONA_SECURE_MEMC
struct kona_secure_memc_pdata k_s_memc_plat_data = {
	.kona_s_memc_base = KONA_MEMC0_S_VA,
	.num_of_memc_ports = NUM_OF_MEMC_PORTS,
	.num_of_groups = NUM_OF_GROUPS,
	.num_of_regions = NUM_OF_REGIONS,
	.cp_area_start = 0x80000000,
	.cp_area_end = 0x81FFFFFF,
	.ap_area_start = 0x82000000,
	.ap_area_end = 0xBFFFFFFF,
	.ddr_start = 0x80000000,
	.ddr_end = 0xBFFFFFFF,
	.masters = {
		MASTER_A7,
		MASTER_COMMS,
		MASTER_FABRIC,
		MASTER_MM,
	},
	.default_master_map = {
		MASTER_FABRIC,
		MASTER_A7,
		MASTER_COMMS,
		MASTER_MM,
	},
	/* following is static memc configuration.
	 * be careful with the same if you need to
	 * change.
	 * set the groupmask carefully.
	 * index of static_memc_master
	 * acts as a group number. */
	.static_memc_config = {
		{"0x80000000", "0x801FFFFF",
			"3", "0x03"},
		{"0x80200000", "0x811FFFFF",
			"3", "0x01"},
		{"0x81200000", "0x81FFFFFF",
			"3", "0x03"},
		{"0x82000000", "0xBFFFFFFF",
			"3", "0xFE"},
	},
	.static_memc_masters = {
		"comms ",
		"a7 ",
	},
	/* following enables user to
	 * enable static configuration listed above.
	 */
	.static_config = 1,
};
#endif

#endif /* CONFIG_BCM_ALSA_SOUND */

static struct platform_device *hawaii_devices[] __initdata = {
#ifdef CONFIG_KONA_HEADSET_MULTI_BUTTON
	&hawaii_headset_device,
#endif

#ifdef CONFIG_DMAC_PL330
	&hawaii_pl330_dmac_device,
#endif

#ifdef CONFIG_HAPTIC_SAMSUNG_PWM
	&haptic_pwm_device,
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

static void __init hawaii_add_i2c_devices(void)
{

#ifdef CONFIG_VIDEO_ADP1653
	i2c_register_board_info(0, adp1653_flash, ARRAY_SIZE(adp1653_flash));
#endif
#ifdef CONFIG_VIDEO_AS3643
	i2c_register_board_info(0, as3643_flash, ARRAY_SIZE(as3643_flash));
#endif



#if defined(CONFIG_TOUCHSCREEN_BCM15500) ||\
defined(CONFIG_TOUCHSCREEN_BCM15500_MODULE)
	i2c_register_board_info(bcm15500_i2c_platform_data.i2c_bus_id,
		bcm15500_i2c_boardinfo,
		ARRAY_SIZE(bcm15500_i2c_boardinfo));
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
	hawaii_ssp0_device.dev.platform_data = &hawaii_ssp0_info;
	hawaii_ssp1_device.dev.platform_data = &hawaii_ssp1_info;
#ifdef CONFIG_BCM_STM
	hawaii_stm_device.dev.platform_data = &hawaii_stm_pdata;
#endif
	hawaii_headset_device.dev.platform_data = &hawaii_headset_data;
	/* The resource in position 2 (starting from 0) is used to fill
	 * the GPIO number. The driver file assumes this. So put the
	 * board specific GPIO number here
	 */
	hawaii_headset_device.resource[2].start = HS_IRQ;
	hawaii_headset_device.resource[2].end   = HS_IRQ;

	hawaii_pl330_dmac_device.dev.platform_data = &hawaii_pl330_pdata;
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
#ifdef CONFIG_ANDROID_PMEM
	platform_device_register(&android_pmem);
#endif

	platform_add_devices(hawaii_common_plat_devices,
			     ARRAY_SIZE(hawaii_common_plat_devices));
}

static void __init hawaii_add_devices(void)
{

	hawaii_add_pdata();

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


static struct of_device_id hawaii_dt_match_table[] __initdata = {
	{ .compatible = "simple-bus", },
	{}
};
static void __init java_init(void)
{
	hawaii_add_devices();
	hawaii_add_common_devices();
	/* Populate platform_devices from device tree data */
	of_platform_populate(NULL, hawaii_dt_match_table,
			hawaii_auxdata_lookup, &platform_bus);
	return;
}

static int __init hawaii_add_lateinit_devices(void)
{

#ifdef CONFIG_BRCM_UNIFIED_DHD_SUPPORT
	hawaii_wlan_init();
#endif
	return 0;
}

late_initcall(hawaii_add_lateinit_devices);

static const char * const java_dt_compat[] = { "bcm,java", NULL, };
DT_MACHINE_START(HAWAII, CONFIG_BRD_NAME)
	.map_io = hawaii_map_io,
	.init_irq = kona_init_irq,
	.handle_irq = gic_handle_irq,
	.timer = &kona_timer,
	.init_machine = java_init,
	.reserve = hawaii_reserve,
	.restart = hawaii_restart,
	.dt_compat = java_dt_compat,
MACHINE_END
