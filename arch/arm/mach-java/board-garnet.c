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
#include <plat/kona_smp.h>

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

#ifdef CONFIG_MOBICORE_DRIVER
#include <linux/broadcom/mobicore.h>
#endif
#if defined(CONFIG_SENSORS_BMP18X) || defined(CONFIG_SENSORS_BMP18X_I2C) \
			|| defined(CONFIG_SENSORS_BMP18X_I2C_MODULE)
#include <linux/bmp18x.h>
#include <mach/bmp18x_i2c_settings.h>
#endif

#if defined(CONFIG_SENSORS_AL3006) || defined(CONFIG_SENSORS_AL3006_MODULE)
#include <linux/al3006.h>
#include <mach/al3006_i2c_settings.h>
#endif

#if defined(CONFIG_INV_MPU_IIO) || defined(CONFIG_INV_MPU_IIO_MODULE)
#include <linux/mpu.h>
#include <linux/i2c/inv_mpu_settings.h>
#endif

#if defined(CONFIG_SENSORS_KXTIK1004)	\
			|| defined(CONFIG_SENSORS_KXTIK1004_MODULE)
#include <linux/kxtik.h>
#endif /* CONFIG_SENSORS_KXTIK1004 */

#if defined(CONFIG_SENSORS_AK8975) || defined(CONFIG_SENSORS_AK8975_MODULE)
#include <linux/akm8975.h>
#endif

#if defined(CONFIG_SENSORS_BMA222)
#include <linux/bma222.h>
#endif


#ifdef CONFIG_BACKLIGHT_PWM
#include <linux/pwm_backlight.h>
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

#if defined(CONFIG_TOUCHSCREEN_BCMTCH15XXX)		|| \
defined(CONFIG_TOUCHSCREEN_BCMTCH15XXX_MODULE)
#include <linux/i2c/bcmtch15xxx.h>
#include <linux/i2c/bcmtch15xxx_settings.h>
#endif

#ifdef CONFIG_USB_DWC_OTG
#include <linux/usb/bcm_hsotgctrl.h>
#include <linux/usb/otg.h>
#endif

#if defined(CONFIG_SENSORS_TMD2771)
#include <linux/i2c/taos_common.h>
#endif

#ifdef CONFIG_KONA_SECURE_MEMC
#include <plat/kona_secure_memc.h>
#include <plat/kona_secure_memc_settings.h>
#endif

#ifdef CONFIG_BRCM_UNIFIED_DHD_SUPPORT
#include "java_wifi.h"

#ifdef CONFIG_FB_BRCM_KONA
#include <video/kona_fb.h>
#endif

#if defined(CONFIG_BCMI2CNFC)
#include <linux/bcmi2cnfc.h>
#endif

#ifdef CONFIG_TOUCHSCREEN_FT5306
#include <linux/i2c/ft5306.h>
#endif

extern int
hawaii_wifi_status_register(void (*callback) (int card_present, void *dev_id),
			    void *dev_id);
#endif
extern void java_timer_init(void);

#if defined(CONFIG_IHF_EXT_PA_TPA2026D2)
#include <linux/mfd/tpa2026d2.h>
#endif

/* SD */
#define SD_CARDDET_GPIO_PIN	91

#ifndef CONFIG_BRD_NAME
#define CONFIG_BRD_NAME "hawaii"
#endif

#define FT5306_MAX_X 720
#define FT5306_MAX_Y 1280

/* Touch */
#define TSC_GPIO_IRQ_PIN			73

#define TSC_GPIO_RESET_PIN			70
#define TSC_GPIO_WAKEUP_PIN         70

#define TANGO_I2C_TS_DRIVER_NUM_BYTES_TO_READ	14

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

#include <linux/broadcom/secure_memory.h>

#ifdef CONFIG_MOBICORE_DRIVER
struct mobicore_data mobicore_plat_data = {
	.name = "mobicore",
	.mobicore_base = 0x9d900000,
	.mobicore_size = 0x00300000,
};
#endif

/* Platform Data for Secure Mem */
struct secure_mem_data secure_mem_plat_data = {
	.name = "secure_mem",
	.mem_base = 0x9d800000,
	.mem_size = 0x100000,
};

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

#ifdef CONFIG_UNICAM_CAMERA

static struct regulator *d_gpsr_cam0_1v8;
static struct regulator *d_lvldo2_cam1_1v8;
static struct regulator *d_1v8_mmc1_vcc;
static struct regulator *d_3v0_mmc1_vcc;

/* The pre-div clock rate needs to satisfy
   the rate requirements of all digital
   channel clocks in use. */
#define SENSOR_PREDIV_CLK               "dig_prediv_clk"
#define SENSOR_0_CLK                    "dig_ch0_clk"	/*DCLK1 */
#define SENSOR_1_CLK                    "dig_ch0_clk"	/* DCLK1 */

#define SENSOR_0_GPIO_PWRDN             (002)
#define SENSOR_0_GPIO_RST               (111)
#define SENSOR_1_GPIO_PWRDN             (005)
#define SENSOR_1_GPIO_RST               (121)

#ifdef CONFIG_MM_312M_SOURCE_CLK
/* Move the clock speed to 104Mhz to be derived from 312 with a div 3 */
#define CSI0_LP_FREQ					(104000000)
#define CSI1_LP_FREQ					(104000000)
#else
#define CSI0_LP_FREQ					(100000000)
#define CSI1_LP_FREQ					(100000000)
#endif

struct cameracfg_s {
	char *name;
	unsigned int prediv_clk;
	unsigned int clk;
	unsigned short pwdn_active;
	unsigned short rst_active;
};

static const struct cameracfg_s cameras[] = {
	{"ov5640", 26000000, 13000000, 1, 0},
	{"ov5648", 26000000, 26000000, 0, 0},
	{"ov2675", 312000000, 26000000, 1, 0},
	{"ov7692", 26000000, 26000000, 1, 0},
	{"ov7695", 26000000, 26000000, 0, 0},
	{"ov8825", 26000000, 13000000, 0, 0},
	{"gc2035", 312000000, 26000000, 1, 0},
	{"sp0a28", 26000000, 26000000, 1, 0},
	{},
};

static struct cameracfg_s *getcameracfg(const char *cameraname)
{
	struct cameracfg_s *pcamera = &cameras[0];
	while (pcamera->name && cameraname) {
		if (0 == strcmp(cameraname, pcamera->name))
			return pcamera;
		else
			pcamera++;
	}
	return NULL;
}

/* target board related cam power supplies */
#if defined(CONFIG_MACH_JAVA_GARNET_C_EDN000) || \
	defined(CONFIG_MACH_JAVA_GARNET_EDN000) || \
	defined(CONFIG_MACH_JAVA_C_LC1) || \
	defined(CONFIG_MACH_JAVA_C_LC2) || \
	defined(CONFIG_MACH_JAVA_C_5609A) || \
	defined(CONFIG_MACH_JAVA_C_5606)
static struct regulator_bulk_data backcam_regulator_data[] = {
	[0] = {
		.supply = "lvldo1_uc",
	},
	[1] = {
		.supply = "mmc1_vcc",
	},
	[2] = {
		.supply = "mmc2_vcc",
	},
	[3] = {
		.supply = "lvldo2_uc",
	},
};

static struct regulator_bulk_data frontcam_regulator_data[] = {
	[0] = {
		.supply = "lvldo2_uc",
	},
	[1] = {
		.supply = "mmc1_vcc",
	},
	[2] = {
		.supply = "mmc2_vcc",
	},
	[3] = {
		.supply = "lvldo2_uc",
	},
};
#endif

#if defined(CONFIG_MACH_JAVA_C_LC1) || defined(CONFIG_MACH_JAVA_C_5609A) \
	|| defined(CONFIG_MACH_JAVA_C_5606)

#define MAIN_CAM_AF_ENABLE			(33)

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
	static int sensor_on = -1;
	struct soc_camera_subdev_desc *ssd = dev->platform_data;

	if (sensor_on == on) {
		pr_info("hawaii_camera_power already in same state: %s\n",
							(on ? "on" : "off"));
		return 0;
	}

	sensor_on = on;

	printk(KERN_INFO "%s:camera power %s\n", __func__, (on ? "on" : "off"));

	struct cameracfg_s *thiscfg = getcameracfg(dev->driver->name);
	if (NULL == thiscfg) {
		pr_err("No cfg for [%s]\n", dev->driver->name);
		return -1;
	}

	if (!unicam_dfs_node.valid) {
		ret = pi_mgr_dfs_add_request(&unicam_dfs_node, "unicam",
					     PI_MGR_PI_ID_MM,
					     PI_MGR_DFS_MIN_VALUE);
		if (ret) {
			return -1;
		}

		if (gpio_request_one(SENSOR_0_GPIO_RST, GPIOF_DIR_OUT |
				(thiscfg->rst_active<<1), "Cam0Rst")) {
			pr_err("Unable to get cam0 RST GPIO\n");
			return -1;
		}
		if (gpio_request_one(SENSOR_0_GPIO_PWRDN, GPIOF_DIR_OUT |
				(thiscfg->pwdn_active<<1), "Cam0PWDN")) {
			pr_err("Unable to get cam0 PWDN GPIO\n");
			return -1;
		}
	#if defined(CONFIG_MACH_JAVA_C_LC1) \
	|| defined(CONFIG_MACH_JAVA_C_5609A) \
	|| defined(CONFIG_MACH_JAVA_C_5606)
		if (gpio_request_one(MAIN_CAM_AF_ENABLE, GPIOF_DIR_OUT |
				     GPIOF_INIT_LOW, "Cam0_af_enable")) {
			pr_err("Unable to get cam0 af enable GPIO\n");
			return -1;
		}
	#endif

		/*MMC1 VCC */
		d_1v8_mmc1_vcc = regulator_get(NULL,
					backcam_regulator_data[1].supply);
		if (IS_ERR_OR_NULL(d_1v8_mmc1_vcc))
			pr_err("Failed to  get d_1v8_mmc1_vcc\n");
		d_3v0_mmc1_vcc = regulator_get(NULL,
					backcam_regulator_data[2].supply);
		if (IS_ERR_OR_NULL(d_3v0_mmc1_vcc))
			pr_err("Failed to  get d_3v0_mmc1_vcc\n");
		d_gpsr_cam0_1v8 = regulator_get(NULL,
					backcam_regulator_data[0].supply);
		if (IS_ERR_OR_NULL(d_gpsr_cam0_1v8))
			pr_err("Failed to  get d_gpsr_cam0_1v8\n");
		if (d_lvldo2_cam1_1v8 == NULL) {
			d_lvldo2_cam1_1v8 = regulator_get(NULL,
					backcam_regulator_data[3].supply);
			if (IS_ERR_OR_NULL(d_lvldo2_cam1_1v8))
				pr_err("Fd_lvldo2_cam1_1v8 cam\n");
		}
	}

	ret = -1;
	lp_clock = clk_get(NULL, CSI0_LP_PERI_CLK_NAME_STR);
	if (IS_ERR_OR_NULL(lp_clock)) {
		pr_err("Unable to get %s clock\n",
		CSI0_LP_PERI_CLK_NAME_STR);
		goto e_clk_get;
	}
	prediv_clock = clk_get(NULL, SENSOR_PREDIV_CLK);
	if (IS_ERR_OR_NULL(prediv_clock)) {
		pr_err("Unable to get SENSOR_PREDIV_CLK clock\n");
		goto e_clk_get;
	}
	clock = clk_get(NULL, SENSOR_0_CLK);
	if (IS_ERR_OR_NULL(clock)) {
		pr_err("Unable to get SENSOR_0 clock\n");
		goto e_clk_get;
	}
	axi_clk = clk_get(NULL, "csi0_axi_clk");
	if (IS_ERR_OR_NULL(axi_clk)) {
		pr_err("Unable to get AXI clock\n");
		goto e_clk_get;
	}
	if (on) {
		if (pi_mgr_dfs_request_update(&unicam_dfs_node, PI_OPP_ECONOMY))
			printk("DVFS for UNICAM failed\n");
		regulator_enable(d_gpsr_cam0_1v8);
		usleep_range(1000, 1010);
		regulator_enable(d_1v8_mmc1_vcc);
		usleep_range(1000, 1010);
		regulator_enable(d_lvldo2_cam1_1v8);
		usleep_range(1000, 1010);

		if (mm_ccu_set_pll_select(CSI0_BYTE1_PLL, 6)) {
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
		value = clk_set_rate(prediv_clock, thiscfg->prediv_clk);
		if (value) {
			pr_err("Failed to set prediv clock\n");
			goto e_clk_set_prediv;
		}
		value = clk_set_rate(clock, thiscfg->clk);
		if (value) {
			pr_err("Failed to set sensor0 clock\n");
			goto e_clk_set_sensor;
		}
		usleep_range(10000, 10100);
		gpio_set_value(SENSOR_0_GPIO_RST, thiscfg->rst_active);
		usleep_range(10000, 10100);
		gpio_set_value(SENSOR_0_GPIO_PWRDN,
			thiscfg->pwdn_active ? 0 : 1);
		usleep_range(5000, 5100);
		gpio_set_value(SENSOR_0_GPIO_RST,
			thiscfg->rst_active ? 0 : 1);
		msleep(30);

		regulator_enable(d_3v0_mmc1_vcc);
		usleep_range(1000, 1010);


#if defined(CONFIG_MACH_JAVA_C_LC1) || defined(CONFIG_MACH_JAVA_C_5609A) \
	|| defined(CONFIG_MACH_JAVA_C_5606)
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

#if defined(CONFIG_MACH_JAVA_C_LC1) || defined(CONFIG_MACH_JAVA_C_5609A) \
	|| defined(CONFIG_MACH_JAVA_C_5606)
		set_af_enable(0);
#endif
		usleep_range(5000, 5100);
		gpio_set_value(SENSOR_0_GPIO_PWRDN, thiscfg->pwdn_active);
		usleep_range(1000, 1100);
		gpio_set_value(SENSOR_0_GPIO_RST, thiscfg->rst_active);

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
	static int sensor_on = -1;
	struct soc_camera_subdev_desc *ssd = dev->platform_data;

	if (sensor_on == on) {
		pr_info("hawaii_camera_power_front already in same state: %s\n",
				(on ? "on" : "off"));
		return 0;
	}

	printk(KERN_INFO "%s:camera power %s\n", __func__, (on ? "on" : "off"));

	struct cameracfg_s *thiscfg = getcameracfg(dev->driver->name);
	if (NULL == thiscfg) {
		pr_err("No cfg for [%s]\n", dev->driver->name);
		 return -1;
	}

	if (!unicam_dfs_node.valid) {
		ret = pi_mgr_dfs_add_request(&unicam_dfs_node, "unicam",
					     PI_MGR_PI_ID_MM,
					     PI_MGR_DFS_MIN_VALUE);
		if (ret) {
			return -1;
		}
		if (gpio_request_one(SENSOR_1_GPIO_PWRDN, GPIOF_DIR_OUT |
				(thiscfg->pwdn_active<<1), "Cam1PWDN")) {
			pr_err("Unable to get CAM1PWDN\n");
			return -1;
		}
		if (gpio_request_one(SENSOR_1_GPIO_RST, GPIOF_DIR_OUT |
					(thiscfg->rst_active<<1), "Cam1RST")) {
			pr_err("Unable to get Cam1RST\n");
			return -1;
		}

		d_lvldo2_cam1_1v8 = regulator_get(NULL,
			frontcam_regulator_data[0].supply);
		if (IS_ERR_OR_NULL(d_lvldo2_cam1_1v8))
			pr_err("Failed to get d_lvldo2_cam1_1v8\n");
		if (d_1v8_mmc1_vcc == NULL) {
			d_1v8_mmc1_vcc = regulator_get(NULL,
				frontcam_regulator_data[1].supply);
			if (IS_ERR_OR_NULL(d_1v8_mmc1_vcc))
				pr_err("Err d_1v8_mmc1_vcc\n");
		}
		if (d_3v0_mmc1_vcc == NULL) {
			d_3v0_mmc1_vcc = regulator_get(NULL,
			frontcam_regulator_data[2].supply);
			if (IS_ERR_OR_NULL(d_3v0_mmc1_vcc))
				pr_err("d_3v0_mmc1_vcc");
		}
		if (d_gpsr_cam0_1v8 == NULL) {
			d_gpsr_cam0_1v8 = regulator_get(NULL,
			frontcam_regulator_data[3].supply);
			if (IS_ERR_OR_NULL(d_gpsr_cam0_1v8))
				pr_err("Fl d_gpsr_cam0_1v8 get	fail");
		}
	}

	ret = -1;
	lp_clock_0 = clk_get(NULL, CSI0_LP_PERI_CLK_NAME_STR);
	if (IS_ERR_OR_NULL(lp_clock_0)) {
		pr_err("Unable to get %s clock\n",
		CSI0_LP_PERI_CLK_NAME_STR);
		goto e_clk_get;
	}

	lp_clock_1 = clk_get(NULL, CSI1_LP_PERI_CLK_NAME_STR);
	if (IS_ERR_OR_NULL(lp_clock_1)) {
		pr_err("Unable to get %s clock\n",
		CSI1_LP_PERI_CLK_NAME_STR);
		goto e_clk_get;
	}

	clock = clk_get(NULL, SENSOR_1_CLK);
	if (IS_ERR_OR_NULL(clock)) {
		pr_err("Unable to get SENSOR_1 clock\n");
		goto e_clk_get;
	}
	axi_clk_0 = clk_get(NULL, "csi0_axi_clk");
	if (IS_ERR_OR_NULL(axi_clk_0)) {
		pr_err("Unable to get AXI clock 0\n");
		goto e_clk_get;
	}
	axi_clk = clk_get(NULL, "csi1_axi_clk");
	if (IS_ERR_OR_NULL(axi_clk)) {
		pr_err("Unable to get AXI clock 1\n");
		goto e_clk_get;
	}
	if (on) {
		if (pi_mgr_dfs_request_update(&unicam_dfs_node, PI_OPP_ECONOMY))
			printk("DVFS for UNICAM failed\n");
		gpio_set_value(SENSOR_1_GPIO_PWRDN, thiscfg->pwdn_active);
		usleep_range(1000, 1010);
		gpio_set_value(SENSOR_1_GPIO_RST, thiscfg->rst_active);
#ifndef CONFIG_SOC_CAMERA_GC2035
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

		gpio_set_value(SENSOR_1_GPIO_RST,
			thiscfg->rst_active ? 0 : 1);
#else
		usleep_range(5000, 5010);
		regulator_enable(d_lvldo2_cam1_1v8);
		usleep_range(50000, 50010);
		regulator_enable(d_gpsr_cam0_1v8);
		usleep_range(10000, 10100);
		/* Secondary cam addition */
		regulator_enable(d_1v8_mmc1_vcc);
		usleep_range(1000, 1010);
		regulator_enable(d_3v0_mmc1_vcc);
		usleep_range(1000, 1010);
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
			pr_err("Failed to enable lp clock 0\n");
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
			pr_err("Failed to enable axi clock 0\n");
			goto e_clk_axi_clk_0;
		}
		value = clk_enable(axi_clk);
		if (value) {
			pr_err("Failed to enable axi clock 1\n");
			goto e_clk_axi;
		}
		value = clk_enable(clock);
		if (value) {
			printk("Failed to enable sensor 1 clock\n");
			goto e_clk_clock;
		}
		value = clk_set_rate(clock, thiscfg->clk);
		if (value) {
			printk("Failed to set sensor 1 clock\n");
			goto e_clk_set_clock;
		}
		usleep_range(10000, 10100);
		gpio_set_value(SENSOR_1_GPIO_PWRDN,
			thiscfg->pwdn_active ? 0 : 1);
		msleep(30);
	#ifdef CONFIG_SOC_CAMERA_GC2035
		gpio_set_value(SENSOR_1_GPIO_RST,
			thiscfg->rst_active ? 0 : 1);
	#endif
	} else {
		gpio_set_value(SENSOR_1_GPIO_PWRDN, thiscfg->pwdn_active);
		usleep_range(1000, 1010);
		gpio_set_value(SENSOR_1_GPIO_RST, thiscfg->rst_active);

		clk_disable(lp_clock_0);
		clk_disable(lp_clock_1);
		clk_disable(clock);
		clk_disable(axi_clk);
		clk_disable(axi_clk_0);
	#ifndef CONFIG_SOC_CAMERA_GC2035
		regulator_disable(d_lvldo2_cam1_1v8);
		regulator_disable(d_1v8_mmc1_vcc);
		regulator_disable(d_gpsr_cam0_1v8);
		regulator_disable(d_3v0_mmc1_vcc);
	#else
		regulator_disable(d_1v8_mmc1_vcc);
		usleep_range(10000, 10100);
		regulator_disable(d_gpsr_cam0_1v8);
		usleep_range(50000, 50100);
		regulator_disable(d_lvldo2_cam1_1v8);
		regulator_disable(d_3v0_mmc1_vcc);
	#endif
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

#ifdef CONFIG_SOC_CAMERA_OV5648
#define OV5648_I2C_ADDRESS              (0x36)
static struct i2c_board_info ov5648_board_info = {
	I2C_BOARD_INFO("ov5648", OV5648_I2C_ADDRESS)
};
static const char ov5648_name[] = "ov5648";
#endif

#ifdef CONFIG_SOC_CAMERA_OV8825
#define OV8825_I2C_ADDRESS              (0x36)
static struct i2c_board_info ov8825_board_info = {
	I2C_BOARD_INFO("ov8825", OV8825_I2C_ADDRESS)
};
static const char ov8825_name[] = "ov8825";
#endif

#ifdef CONFIG_SOC_CAMERA_OV7692
#define OV7692_I2C_ADDRESS              (0x3e)
static struct i2c_board_info ov7692_board_info = {
	I2C_BOARD_INFO("ov7692", OV7692_I2C_ADDRESS)
};
static const char ov7692_name[] = "ov7692";
#endif

#ifdef CONFIG_SOC_CAMERA_OV7695
#define OV7695_I2C_ADDRESS              (0x21)
static struct i2c_board_info ov7695_board_info = {
	I2C_BOARD_INFO("ov7695", OV7695_I2C_ADDRESS)
};
static const char ov7695_name[] = "ov7695";
#endif

#ifdef CONFIG_SOC_CAMERA_GC2035
#define GC2035_I2C_ADDRESS              (0x3c)
static struct i2c_board_info gc2035_board_info = {
	I2C_BOARD_INFO("gc2035", GC2035_I2C_ADDRESS)
};
static const char gc2035_name[] = "gc2035";
#endif

#define _CAM_JOIN(cam, cstruct)   cam##cstruct
#define CAM_JOIN(cam, cstruct)   _CAM_JOIN(cam, cstruct)

#define backcam_name            CAM_JOIN(BACK_CAM, _name)
#define backcam_board_info      CAM_JOIN(BACK_CAM, _board_info)
#define frontcam_name           CAM_JOIN(FRONT_CAM, _name)
#define frontcam_board_info     CAM_JOIN(FRONT_CAM, _board_info)

/* target board related camera configurations */
#if defined(CONFIG_MACH_JAVA_GARNET_C_EDN000) || \
	defined(CONFIG_MACH_JAVA_GARNET_EDN000)
#ifdef CONFIG_SOC_CAMERA_OV5648
#define BACK_CAM    ov5648
#endif
#ifdef CONFIG_SOC_CAMERA_OV7692
#define FRONT_CAM   ov7692
#endif
#endif /* CONFIG_MACH_JAVA_GARNET_C_EDN000 */
#ifdef CONFIG_MACH_JAVA_C_LC1
#define BACK_CAM    ov5648
#define FRONT_CAM   ov7695
#endif /* CONFIG_MACH_JAVA_C_LC1 */
#ifdef CONFIG_MACH_JAVA_C_LC2
#define BACK_CAM    ov8825
#define FRONT_CAM   gc2035
#endif /* CONFIG_MACH_JAVA_C_LC2 */

/* cam interface descriptor */
static struct v4l2_subdev_sensor_interface_parms backcam_if_params = {
	.if_type = V4L2_SUBDEV_SENSOR_SERIAL,
	.if_mode = V4L2_SUBDEV_SENSOR_MODE_SERIAL_CSI2,
	.orientation = V4L2_SUBDEV_SENSOR_PORTRAIT,
	.facing = V4L2_SUBDEV_SENSOR_BACK,
	.parms.serial = {
		.lanes = 2,
		.channel = 0,
		.phy_rate = 0,
		.pix_clk = 0,
	},
};
static struct v4l2_subdev_sensor_interface_parms frontcam_if_params = {
	.if_type = V4L2_SUBDEV_SENSOR_SERIAL,
	.if_mode = V4L2_SUBDEV_SENSOR_MODE_SERIAL_CSI2,
	.orientation = V4L2_SUBDEV_SENSOR_LANDSCAPE,
	.facing = V4L2_SUBDEV_SENSOR_FRONT,
	.parms.serial = {
		.lanes = 1,
		.channel = 1,
		.phy_rate = 0,
		.pix_clk = 0,
	},
};

/* back camera is specified */
#ifdef BACK_CAM
/* soc camera descriptor for back camera */
static struct soc_camera_desc iclink_back = {
	.host_desc = {
		.bus_id = 0,
		.board_info = &backcam_board_info,
		.i2c_adapter_id = 0,
		.module_name = backcam_name,
	},
	.subdev_desc = {
		.power = &hawaii_camera_power,
		.reset = &hawaii_camera_reset,
		.drv_priv = &backcam_if_params,
	},
};

static struct platform_device hawaii_camera_back = {
	.name = "soc-camera-pdrv",
	.id = 0,
	.dev = {
		.platform_data = &iclink_back,
	},
};
#endif /* BACK_CAM */

/* front camera is specified */
#ifdef FRONT_CAM
/* soc camera descriptor for front camera */
static struct soc_camera_desc iclink_front = {
	.host_desc = {
		.bus_id = 0,
		.board_info = &frontcam_board_info,
		.i2c_adapter_id = 0,
		.module_name = frontcam_name,
	},
	.subdev_desc = {
		.power = &hawaii_camera_power_front,
		.reset = &hawaii_camera_reset_front,
		.drv_priv = &frontcam_if_params,
	},
};
static struct platform_device hawaii_camera_front = {
	.name = "soc-camera-pdrv",
	.id = 1,
	.dev = {
		.platform_data = &iclink_front,
	},
};
#endif /* FRONT_CAM */
#endif /* CONFIG_UNICAM_CAMERA */

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
/* Following are the devices which has bot platform data
 * and device tree node, and to avoid re-registration of
 * same device twice these are put under preprocessor
 * directives such that following devices will be registered
 * only when DT Support is not there
 */
#ifndef CONFIG_OF
	&hawaii_serial_device,
	&hawaii_i2c_adap_devices[0],
	&hawaii_i2c_adap_devices[1],
	&hawaii_i2c_adap_devices[2],
	&hawaii_i2c_adap_devices[3],
	&hawaii_i2c_adap_devices[4],
#ifdef CONFIG_KONA_MEMC
	&kona_memc_device,
#endif

#ifdef CONFIG_KONA_TMON
	&kona_tmon_device,
#endif
	&hawaii_pwm_device,

#endif /* End of CONFIG_OF */

	&pmu_device,
#ifndef CONFIG_OF
	&hawaii_ssp0_device,
#endif

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
#ifndef CONFIG_OF
	&hawaii_usb_phy_platform_device,
	&hawaii_hsotgctrl_platform_device,
	&hawaii_otg_platform_device,
#endif
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
#ifdef CONFIG_UNICAM_CAMERA
	&hawaii_camera_device,
	#ifdef BACK_CAM
	&hawaii_camera_back,
	#endif
	#ifdef FRONT_CAM
	&hawaii_camera_front,
	#endif
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

static struct bcm_bt_lpm_platform_data brcm_bt_lpm_data = {
	.bt_wake_gpio = GPIO_BT_WAKE,
	.host_wake_gpio = GPIO_HOST_WAKE,
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

/* Platform Data for SDMMC,EMMC and Wifi */

static struct sdio_platform_cfg hawaii_sdio_param = {
#ifdef CONFIG_BRCM_UNIFIED_DHD_SUPPORT
	.register_status_notify = hawaii_wifi_status_register,
#endif
};

#ifdef CONFIG_OF
static struct sdio_platform_cfg hawaii_sdio0_param = {
	.configure_sdio_pullup = configure_sdio_pullup,
};
#endif

#ifdef CONFIG_OF
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
#if 0
#ifdef CONFIG_SOC_CAMERA_OV5648
	OF_DEV_AUXDATA("bcm,soc-camera", 0x36,
		"soc-back-camera", &iclink_main),
#endif
#endif
#ifdef CONFIG_SOC_CAMERA_OV7695
	OF_DEV_AUXDATA("bcm,soc-camera", 0x21,
		"soc-front-camera", &iclink_front),
#endif
#ifdef CONFIG_SOC_CAMERA_GC2035
	OF_DEV_AUXDATA("bcm,soc-camera", 0x3c,
		"soc-front-camera", &iclink_front),
#endif

	{},
};
#endif

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



#if defined(CONFIG_TOUCHSCREEN_BCMTCH15XXX)		|| \
defined(CONFIG_TOUCHSCREEN_BCMTCH15XXX_MODULE)
static int BCMTCH_TSP_PowerOnOff(bool on)
{
	/* PLACE TOUCH CONTROLLER REGULATOR CODE HERE . SEE STEP 6 */
	return 0;
}

static struct bcmtch_platform_data bcmtch15xxx_i2c_platform_data = {
	.i2c_bus_id		= BCMTCH_HW_I2C_BUS_ID,
	.i2c_addr_spm		= BCMTCH_HW_I2C_ADDR_SPM,
	.i2c_addr_sys		= BCMTCH_HW_I2C_ADDR_SYS,

	.gpio_interrupt_pin	= BCMTCH_HW_GPIO_INTERRUPT_PIN,
	.gpio_interrupt_trigger	= BCMTCH_HW_GPIO_INTERRUPT_TRIGGER,

	.gpio_reset_pin		= BCMTCH_HW_GPIO_RESET_PIN,
	.gpio_reset_polarity	= BCMTCH_HW_GPIO_RESET_POLARITY,
	.gpio_reset_time_ms	= BCMTCH_HW_GPIO_RESET_TIME_MS,

	.ext_button_count	= BCMTCH_HW_BUTTON_COUNT,
	.ext_button_map		= bcmtch_hw_button_map,

	.axis_orientation_flag	=
		((BCMTCH_HW_AXIS_REVERSE_X << BCMTCH_AXIS_FLAG_X_BIT_POS)
		|(BCMTCH_HW_AXIS_REVERSE_Y << BCMTCH_AXIS_FLAG_Y_BIT_POS)
		|(BCMTCH_HW_AXIS_SWAP_X_Y << BCMTCH_AXIS_FLAG_X_Y_BIT_POS)),
	.bcmtch_on = BCMTCH_TSP_PowerOnOff,

};

static struct i2c_board_info __initdata bcmtch15xxx_i2c_boardinfo[] = {
	{
		I2C_BOARD_INFO(BCMTCH15XXX_NAME, BCMTCH_HW_I2C_ADDR_SPM),
		.platform_data	= &bcmtch15xxx_i2c_platform_data,
		.irq		= gpio_to_irq(BCMTCH_HW_GPIO_INTERRUPT_PIN),
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


/* Platform Data For UART Drivers */

#define KONA_UART0_PA   UARTB_BASE_ADDR
#define KONA_UART1_PA   UARTB2_BASE_ADDR
#define KONA_UART2_PA   UARTB3_BASE_ADDR

#define HAWAII_8250PORT(name, clk, freq, uart_name)		\
{								\
	.membase    = (void __iomem *)(KONA_##name##_VA),	\
	.mapbase    = (resource_size_t)(KONA_##name##_PA),	\
	.irq        = BCM_INT_ID_##name,			\
	.uartclk    = freq,					\
	.regshift   = 2,					\
	.iotype     = UPIO_MEM32,				\
	.type       = PORT_16550A,				\
	.flags      = UPF_BOOT_AUTOCONF | UPF_BUG_THRE |	\
			UPF_FIXED_TYPE | UPF_SKIP_TEST,		\
	.private_data = (void __iomem *)((KONA_##name##_VA) +	\
					UARTB_USR_OFFSET),	\
	.clk_name = clk,					\
	.port_name = uart_name,					\
}

static struct plat_serial8250_port hawaii_uart_platform_data[] = {
	HAWAII_8250PORT(UART0, UARTB_PERI_CLK_NAME_STR, 26000000, "console"),
	HAWAII_8250PORT(UART1, UARTB2_PERI_CLK_NAME_STR, 48000000, "bluetooth"),
	HAWAII_8250PORT(UART2, UARTB3_PERI_CLK_NAME_STR, 26000000, "gps"),
	{
	 .flags = 0,
	},
};

/* Platform Data For I2C */

static struct bsc_adap_cfg bsc_i2c_cfg[] = {
	{
	 .speed = BSC_BUS_SPEED_400K,
	 .dynamic_speed = 1,
	 .bsc_clk = "bsc1_clk",
	 .bsc_apb_clk = "bsc1_apb_clk",
	 .retries = 1,
	 .is_pmu_i2c = false,
	 .fs_ref = BSC_BUS_REF_13MHZ,
	 .hs_ref = BSC_BUS_REF_104MHZ,
	 },

	{
	 .speed = BSC_BUS_SPEED_400K,
	 .dynamic_speed = 1,
	 .bsc_clk = "bsc2_clk",
	 .bsc_apb_clk = "bsc2_apb_clk",
	 .retries = 3,
	 .is_pmu_i2c = false,
	 .fs_ref = BSC_BUS_REF_13MHZ,
	 .hs_ref = BSC_BUS_REF_104MHZ,
	 },

	{
	 .speed = BSC_BUS_SPEED_400K,
	 .dynamic_speed = 1,
	 .bsc_clk = "bsc3_clk",
	 .bsc_apb_clk = "bsc3_apb_clk",
	 .retries = 1,
	 .is_pmu_i2c = false,
	 .fs_ref = BSC_BUS_REF_13MHZ,
	 .hs_ref = BSC_BUS_REF_104MHZ,
	 },

	{
	 .speed = BSC_BUS_SPEED_400K,
	 .dynamic_speed = 1,
	 .bsc_clk = "bsc4_clk",
	 .bsc_apb_clk = "bsc4_apb_clk",
	 .retries = 1,
	 .is_pmu_i2c = false,
	 .fs_ref = BSC_BUS_REF_13MHZ,
	 .hs_ref = BSC_BUS_REF_104MHZ,
	 },

	{
#if defined(CONFIG_KONA_PMU_BSC_HS_MODE)
	 .speed = BSC_BUS_SPEED_HS,
	 /* No dynamic speed in HS mode */
	 .dynamic_speed = 0,
	 /*
	  * PMU can NAK certain I2C read commands, while write
	  * is in progress; and it takes a while to synchronise
	  * writes between HS clock domain(3.25MHz) and
	  * internal clock domains (32k). In such cases, we retry
	  * PMU reads until the writes are through. PMU need more
	  * retry counts in HS mode to handle this.
	  */
	 .retries = 5,
#elif defined(CONFIG_KONA_PMU_BSC_HS_1MHZ)
	 .speed = BSC_BUS_SPEED_HS_1MHZ,
	 .dynamic_speed = 0,
	 .retries = 5,
#elif defined(CONFIG_KONA_PMU_BSC_HS_1625KHZ)
	 .speed = BSC_BUS_SPEED_HS_1625KHZ,
	 .dynamic_speed = 0,
	 .retries = 5,
#else
	 .speed = BSC_BUS_SPEED_50K,
	 .dynamic_speed = 1,
	 .retries = 3,
#endif
	 .bsc_clk = "pmu_bsc_clk",
	 .bsc_apb_clk = "pmu_bsc_apb",
	 .is_pmu_i2c = true,
	 .fs_ref = BSC_BUS_REF_13MHZ,
	 .hs_ref = BSC_BUS_REF_26MHZ,
	 },
};

/* Platform Data for NFC */

/* NFC */
#define NFC_INT	90
#define NFC_WAKE 25
#define NFC_ENABLE 100

#if defined(CONFIG_BCMI2CNFC)
static int bcmi2cnfc_gpio_setup(void *);
static int bcmi2cnfc_gpio_clear(void *);
static struct bcmi2cnfc_i2c_platform_data bcmi2cnfc_pdata = {
	.i2c_pdata	= {ADD_I2C_SLAVE_SPEED(BSC_BUS_SPEED_400K),
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

/* Platform Data for FT5306 */

#ifdef CONFIG_TOUCHSCREEN_FT5306
static int ts_power(ts_power_status vreg_en)
{
	struct regulator *reg = NULL;
	if (!reg) {
		/* Remove this comment when the
		 * regulator references are fixed
		 * here for Hawaii
		 * */
		reg = regulator_get(NULL, "camldo2");
		if (!reg || IS_ERR(reg)) {
			pr_err("No Regulator available for ldo_camldo2\n");
			return -1;
		}
	}
	if (reg) {
		if (vreg_en) {
			regulator_set_voltage(reg, 3000000, 3000000);
			pr_err("Turn on TP (ldo_camldo2) to 2.8V\n");
			regulator_enable(reg);
		} else {
			pr_err("Turn off TP (ldo_camldo2)\n");
			regulator_disable(reg);
		}
	} else {
		pr_err("TP Regulator Alloc Failed");
		return -1;
	}
	return 0;
}

static struct Synaptics_ts_platform_data ft5306_plat_data = {
	.gpio_irq_pin		= TSC_GPIO_IRQ_PIN,
	.gpio_reset_pin		= TSC_GPIO_RESET_PIN,
	.gpio_wakeup_pin    = TSC_GPIO_WAKEUP_PIN,
	.x_max_value		= FT5306_MAX_X-1,
	.y_max_value	= FT5306_MAX_Y-1,
	.power              = ts_power,
};

static struct i2c_board_info __initdata ft5306_info[] = {
	{			/* New touch screen i2c slave address. */
	 I2C_BOARD_INFO("FocalTech-Ft5306", (0x70 >> 1)),
	 .platform_data = &ft5306_plat_data,
	 .irq = gpio_to_irq(TSC_GPIO_IRQ_PIN),
	 },
};
#endif

/* Platform Data for Invensense MPU6050 sensor */

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
	 .irq = 33,
	 },
};
#endif /* CONFIG_INV_MPU_IIO */

/* Platform Data For AL3006 ALS and Proximity sensor */

#if defined(CONFIG_SENSORS_AL3006) || defined(CONFIG_SENSORS_AL3006_MODULE)
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
#endif /* CONFIG_SENSORS_AL3006 */

/* Platform Data for BMP180 pressure sensor */

#if  defined(CONFIG_SENSORS_BMP18X) || defined(CONFIG_SENSORS_BMP18X_I2C) \
				|| defined(CONFIG_SENSORS_BMP18X_I2C_MODULE)
static struct i2c_board_info __initdata i2c_bmp18x_info[] = {
	{
	 I2C_BOARD_INFO(BMP18X_NAME, BMP18X_I2C_ADDRESS),
	 },
};
#endif /* CONFIG_BMP18X */

/* Platform Data for TMD2771 */

#if defined(CONFIG_SENSORS_TMD2771)
static struct taos_cfg taos_cfg_data = {
	.calibrate_target = 300000,
	.als_time = 200,
	.scale_factor = 1,
	.gain_trim = 512,
	.filter_history = 3,
	.filter_count = 1,
	.gain = 2,
	.prox_threshold_hi = 646,
	.prox_threshold_lo = 638,
	.als_threshold_hi = 1,
	.als_threshold_lo = 0,
	.prox_int_time = 238,
	.prox_adc_time = 255,
	.prox_wait_time = 238,
	.prox_intr_filter = 17,
	.prox_config = 0,
	.prox_pulse_cnt = 96,
	.prox_gain = 34,
	.prox_win_sw = 100,
};

static struct tmd2771_platform_data tmd2771_pdata = {
	.cfg_data = &taos_cfg_data,
};
static struct i2c_board_info __initdata i2c_tmd2771_info[] = {
	{
		I2C_BOARD_INFO("tmd2771", 0x39),
		.platform_data = &tmd2771_pdata,
		.irq = 89,
	},
};
#endif

/* Platform Data form KXTIX I2C Slave */

#if defined(CONFIG_SENSORS_KXTIK1004) \
	|| defined(CONFIG_SENSORS_KXTIK1004_MODULE)
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

#define KXTIK_GPIO_IRQ_PIN          (0)
#define KXTIK_I2C_BUS_ID            (2)

static struct i2c_board_info __initdata kxtik_i2c_boardinfo[] = {
	{
	 I2C_BOARD_INFO("kxtik", KXTIK_SLAVE_ADDR),
	 .platform_data = &kxtik_pdata,
	 },
};
#endif /* CONFIG_SENSORS_KXTIK1004 */

/* Platform Data for AK8975 */

#if defined(CONFIG_SENSORS_AK8975) || defined(CONFIG_SENSORS_AK8975_MODULE)
static struct akm8975_platform_data akm_platform_data_akm8975 = {
	.gpio_DRDY = 4,
	.layout = 6,
};

static struct i2c_board_info __initdata akm8975_info[] = {
	{
	 I2C_BOARD_INFO("akm8975", 0x0c),
	 .flags = I2C_CLIENT_WAKE,
	 .platform_data = &akm_platform_data_akm8975,
	 .irq = gpio_to_irq(4),
	 },
};
#endif
/* CONFIG_SENSORS_AK8975 */

/* Platform Datafor BMA222 Sensor */

#if defined(CONFIG_SENSORS_BMA222)
static struct bma222_accl_platform_data bma_pdata = {
	.orientation = BMA_ORI_XYSWITCH_NOINVERSE,
	.invert = false,
};
#endif

/* Platform Data for GPIO LEDs */

#if defined(CONFIG_LEDS_GPIO) || defined(CONFIG_LEDS_GPIO_MODULE)
#define BLUE_LED_GPIO		22
#define GREEN_LED_GPIO		9
static struct gpio_led gpio_leds[] = {
	{
		.name	= "blue",
		.default_trigger = "timer",
		.gpio	= BLUE_LED_GPIO ,
		.active_low = 0,
	},
	{
		.name	= "green",
		.default_trigger = "timer",
		.gpio	= GREEN_LED_GPIO ,
		.active_low = 0,
	},
};

static struct gpio_led_platform_data gpio_led_info = {
	.leds		= gpio_leds,
	.num_leds	= ARRAY_SIZE(gpio_leds),
};

static struct platform_device leds_gpio = {
	.name	= "leds-gpio",
	.id	= -1,
	.dev	= {
		.platform_data	= &gpio_led_info,
	},
};
#endif

/* Platform Data for Backlight driver */

#ifdef CONFIG_BACKLIGHT_PWM

static struct platform_pwm_backlight_data hawaii_backlight_data = {
/* backlight */
	.pwm_id = 2,
	.max_brightness = 32,	/* Android calibrates to 32 levels */
	.dft_brightness = 32,
	.polarity = 1,		/* Inverted polarity */
	.pwm_period_ns = 99900,		/* ~10010 Hz */
	.bl_delay_on = 7000,
	.pwm_request_label = "backlight",
};

#endif /*CONFIG_BACKLIGHT_PWM */

/* Platform Data for Keypad */

#ifdef CONFIG_KEYBOARD_BCM
static struct bcm_keymap hawaii_keymap[] = {
	{BCM_KEY_ROW_0, BCM_KEY_COL_0, "Vol Down Key", KEY_VOLUMEDOWN},
	{BCM_KEY_ROW_0, BCM_KEY_COL_1, "Vol Up Key", KEY_VOLUMEUP},
	{BCM_KEY_ROW_0, BCM_KEY_COL_2, "unused", 0},
	{BCM_KEY_ROW_0, BCM_KEY_COL_3, "unused", 0},
	{BCM_KEY_ROW_0, BCM_KEY_COL_4, "unused", 0},
	{BCM_KEY_ROW_0, BCM_KEY_COL_5, "unused", 0},
	{BCM_KEY_ROW_0, BCM_KEY_COL_6, "unused", 0},
	{BCM_KEY_ROW_0, BCM_KEY_COL_7, "unused", 0},
	{BCM_KEY_ROW_1, BCM_KEY_COL_0, "Camera Focus Key", KEY_CAMERA_FOCUS},
	{BCM_KEY_ROW_1, BCM_KEY_COL_1, "Camera Key", KEY_CAMERA},
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

/* Platform Data For Wifi Driver */

static struct board_wifi_info brcm_wifi_data = {
	.wl_reset_gpio = 3,
	.host_wake_gpio = 74,
	.board_nvram_file = "/system/vendor/firmware/fw_wifi_nvram_4330.txt",
	.module_name = "bcmdhd_4330",
};
static struct platform_device board_wifi_driver_device = {

	.name = "bcm_wifi",
	.id = -1,
	.dev = {
		.platform_data = &brcm_wifi_data,
	},
};



/* Platform Data for WD-Tapper */

#ifdef CONFIG_WD_TAPPER
static struct wd_tapper_platform_data wd_tapper_data = {
	/* Set the count to the time equivalent to the time-out in seconds
	 * required to pet the PMU watchdog to overcome the problem of reset in
	 * suspend*/
	.count = 300,
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

#if defined(CONFIG_BCM_BT_LPM)
	&board_bcmbt_lpm_device,
#endif

#ifdef CONFIG_VIDEO_KONA
	&hawaii_unicam_device,
#endif

#if defined(CONFIG_BCM_ALSA_SOUND)
	&board_caph_device,
#endif

/* Following are the devices which has bot platform data
 * and device tree node, and to avoid re-registration of
 * same device twice these are put under preprocessor
 * directives such that following devices will be registered
 * only when DT Support is not there
 */

#ifndef CONFIG_OF

#ifdef CONFIG_WD_TAPPER
	&wd_tapper,
#endif

#ifdef CONFIG_BACKLIGHT_PWM
	&hawaii_backlight_device,
#endif

#ifdef CONFIG_KEYBOARD_BCM
	&hawaii_kp_device,
#endif
	&board_wifi_driver_device,

#if defined(CONFIG_LEDS_GPIO) || defined(CONFIG_LEDS_GPIO_MODULE)
	&leds_gpio,
#endif

#endif

};

#ifdef CONFIG_IHF_EXT_PA_TPA2026D2
static struct tpa2026d2_platform_data tpa2026d2_i2c_platform_data = {
	.i2c_bus_id = 1,
	.shutdown_gpio = 91
};

static struct i2c_board_info tpa2026d2_i2c_boardinfo[] = {
	{
		I2C_BOARD_INFO("tpa2026d2", (TPA2026D2_I2C_ADDR >> 1)),
		.platform_data = &tpa2026d2_i2c_platform_data
	},
};
#endif

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
#ifdef CONFIG_VIDEO_AS3643
	i2c_register_board_info(0, as3643_flash, ARRAY_SIZE(as3643_flash));
#endif

#if defined(CONFIG_TOUCHSCREEN_BCMTCH15XXX)	|| \
defined(CONFIG_TOUCHSCREEN_BCMTCH15XXX_MODULE)
	i2c_register_board_info(bcmtch15xxx_i2c_platform_data.i2c_bus_id,
		bcmtch15xxx_i2c_boardinfo,
		ARRAY_SIZE(bcmtch15xxx_i2c_boardinfo));
#endif

#ifdef CONFIG_IHF_EXT_PA_TPA2026D2
	i2c_register_board_info(tpa2026d2_i2c_platform_data.i2c_bus_id,
		tpa2026d2_i2c_boardinfo,
		ARRAY_SIZE(tpa2026d2_i2c_boardinfo));
#endif

/* Registers following i2c slaves only when DT Support is not there */

#ifndef CONFIG_OF

#ifdef CONFIG_TOUCHSCREEN_FT5306
	i2c_register_board_info(3, ft5306_info, ARRAY_SIZE(ft5306_info));
#endif

#if defined(CONFIG_BCMI2CNFC)
	i2c_register_board_info(1, bcmi2cnfc, ARRAY_SIZE(bcmi2cnfc));
#endif

#if defined(CONFIG_INV_MPU_IIO) || defined(CONFIG_INV_MPU_IIO_MODULE)
#if defined(INV_MPU_IRQ_GPIO)
	inv_mpu_i2c0_boardinfo[0].irq = gpio_to_irq(INV_MPU_IRQ_GPIO);
#endif
	i2c_register_board_info(INV_MPU_I2C_BUS_ID,
				inv_mpu_i2c0_boardinfo,
				ARRAY_SIZE(inv_mpu_i2c0_boardinfo));
#endif /* CONFIG_INV_MPU_IIO */

#if  defined(CONFIG_SENSORS_BMP18X) || defined(CONFIG_SENSORS_BMP18X_I2C) \
				|| defined(CONFIG_SENSORS_BMP18X_I2C_MODULE)
	i2c_register_board_info(
#ifdef BMP18X_I2C_BUS_ID
				       BMP18X_I2C_BUS_ID,
#else
				       -1,
#endif
				       i2c_bmp18x_info,
				       ARRAY_SIZE(i2c_bmp18x_info));
#endif

#if defined(CONFIG_SENSORS_KXTIK1004)	\
			|| defined(CONFIG_SENSORS_KXTIK1004_MODULE)
	i2c_register_board_info(2,
			kxtik_i2c_boardinfo,
			ARRAY_SIZE(kxtik_i2c_boardinfo));
#endif /* CONFIG_SENSORS_KXTIK1004 */

#if defined(CONFIG_SENSORS_AK8975) || defined(CONFIG_SENSORS_AK8975_MODULE)
	i2c_register_board_info(2,
				akm8975_info, ARRAY_SIZE(akm8975_info));
#endif /* CONFIG_SENSORS_AK8975 */

#if defined(CONFIG_SENSORS_TMD2771)
	i2c_register_board_info(2,
					i2c_tmd2771_info,
					ARRAY_SIZE(i2c_tmd2771_info));
#endif

#if defined(CONFIG_SENSORS_AL3006) || defined(CONFIG_SENSORS_AL3006_MODULE)
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
#endif /* CONFIG_SENSORS_AL3006 */
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
	.base = 0x80000000,
	.size = 0x70000000,
	.order = 0,
};
struct bcm_iovmm_pdata iovmm_mm_256mb_pdata = {
	.name = "iovmm-mm-256mb",
	.base = 0xf0000000,
	.size = 0x0ff00000,
	.order = 0,
};
#endif
#endif /* CONFIG_ION_BCM_NO_DT */

#define HS_IRQ		gpio_to_irq(92)


static void hawaii_add_pdata(void)
{
	hawaii_serial_device.dev.platform_data = &hawaii_uart_platform_data;
	hawaii_i2c_adap_devices[0].dev.platform_data = &bsc_i2c_cfg[0];
	hawaii_i2c_adap_devices[1].dev.platform_data = &bsc_i2c_cfg[1];
	hawaii_i2c_adap_devices[2].dev.platform_data = &bsc_i2c_cfg[2];
	hawaii_i2c_adap_devices[3].dev.platform_data = &bsc_i2c_cfg[3];
	hawaii_i2c_adap_devices[4].dev.platform_data = &bsc_i2c_cfg[4];
#ifdef CONFIG_BACKLIGHT_PWM
	hawaii_backlight_device.dev.platform_data = &hawaii_backlight_data;
#endif
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

#ifdef CONFIG_KEYBOARD_BCM
	hawaii_kp_device.dev.platform_data = &hawaii_keypad_data;
#endif

	platform_add_devices(hawaii_devices, ARRAY_SIZE(hawaii_devices));

	hawaii_add_i2c_devices();
#ifndef CONFIG_OF
	spi_register_board_info(spi_slave_board_info,
				ARRAY_SIZE(spi_slave_board_info));
#endif

}

static void __init hawaii_add_sdio_devices(void)
{
	platform_add_devices(hawaii_sdio_devices,
			     ARRAY_SIZE(hawaii_sdio_devices));
}

static struct of_device_id hawaii_dt_match_table[] __initdata = {
	{ .compatible = "simple-bus", },
	{}
};

#ifndef CONFIG_OF
#ifdef CONFIG_FB_BRCM_KONA
/*
 * KONA FRAME BUFFER DISPLAY DRIVER PLATFORM CONFIG
 */
struct kona_fb_platform_data konafb_devices[] __initdata = {
	{
		.name = "NT35512",
		.reg_name = "cam2",
		.rst =  {
			.gpio = 8,
			.setup = 700,
			.pulse = 1000,
			.hold = 100000,
			.active = false,
		},
		.vmode = true,
		.vburst = false,
		.cmnd_LP = true,
		.te_ctrl = false,
		.col_mod_i = 3,  /*DISPDRV_FB_FORMAT_xBGR8888*/
		.col_mod_o = 2, /*DISPDRV_FB_FORMAT_xRGB8888*/
		.width = 480,
		.height = 854,
		.fps = 60,
		.lanes = 2,
		.hs_bps = 333000000,
		.lp_bps = 8000000,
		.desense_offset = 2400000,
		.rotation = 180,
#ifdef CONFIG_IOMMU_API
		.pdev_iommu = &iommu_mm_device,
#endif
#ifdef CONFIG_BCM_IOVMM
		.pdev_iovmm = &iovmm_mm_device,
#endif
	},
	{
		.name = "OTM8018B",
		.reg_name = "cam2",
		.rst =  {
			.gpio = 8,
			.setup = 700,
			.pulse = 1000,
			.hold = 100000,
			.active = false,
		},
		.vmode = true,
		.vburst = false,
		.cmnd_LP = true,
		.te_ctrl = false,
		.col_mod_i = 3,  /*DISPDRV_FB_FORMAT_xBGR8888*/
		.col_mod_o = 2, /*DISPDRV_FB_FORMAT_xRGB8888*/
		.width = 480,
		.height = 854,
		.fps = 60,
		.lanes = 2,
		.hs_bps = 333000000,
		.lp_bps = 8000000,
		.desense_offset = 2400000,
		.rotation = 180,
#ifdef CONFIG_IOMMU_API
		.pdev_iommu = &iommu_mm_device,
#endif
#ifdef CONFIG_BCM_IOVMM
		.pdev_iovmm = &iovmm_mm_device,
#endif
	},

};

#include "kona_fb_init.c"

#endif /* #ifdef CONFIG_FB_BRCM_KONA */
#endif

static void __init java_init(void)
{
	hawaii_add_devices();

#ifndef CONFIG_OF
#ifdef CONFIG_FB_BRCM_KONA
	konafb_init();
#endif
#endif
	hawaii_add_common_devices();
#ifdef CONFIG_OF
	/* Populate platform_devices from device tree data */
	of_platform_populate(NULL, hawaii_dt_match_table,
			hawaii_auxdata_lookup, &platform_bus);
#endif
	return;
}

static int __init hawaii_add_lateinit_devices(void)
{
#ifndef CONFIG_OF
	hawaii_add_sdio_devices();
#endif

#ifdef CONFIG_BRCM_UNIFIED_DHD_SUPPORT
	hawaii_wlan_init();
#endif
	return 0;
}

late_initcall(hawaii_add_lateinit_devices);

#ifdef CONFIG_OF
static const char * const java_dt_compat[] = { "bcm,java", NULL, };
DT_MACHINE_START(HAWAII, CONFIG_BRD_NAME)
	.dt_compat = java_dt_compat,
#else
MACHINE_START(HAWAII, CONFIG_BRD_NAME)
	.atag_offset = 0x100,
#endif
	.smp = smp_ops(kona_smp_ops),
	.map_io = hawaii_map_io,
	.init_irq = kona_init_irq,
	.init_time = java_timer_init,
	.init_machine = java_init,
	.reserve = hawaii_reserve,
	.restart = hawaii_restart,
MACHINE_END
