/************************************************************************/
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
#include <linux/serial_8250.h>
#include <linux/i2c.h>
#include <linux/i2c-kona.h>
#include <linux/spi/spi.h>
#include <linux/clk.h>
#include <linux/bootmem.h>
#include <linux/input.h>
#include <linux/mfd/bcm590xx/core.h>
#include <linux/gpio.h>
#include <linux/gpio_keys.h>
#include <linux/i2c-kona.h>
#include <linux/i2c.h>
#include <linux/i2c/tango_ts.h>
#include <linux/i2c/melfas_ts.h>
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

#ifdef CONFIG_HAS_WAKELOCK
#include <linux/wakelock.h>
#endif

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

#if defined(CONFIG_BCMI2CNFC)
#include <linux/bcmi2cnfc.h>
#endif

#if defined(CONFIG_SENSORS_BMA2X2)
#include <linux/bma222.h>
#endif

#if defined(CONFIG_BMP18X) || defined(CONFIG_BMP18X_I2C)
	|| defined(CONFIG_BMP18X_I2C_MODULE)
#include <linux/bmp18x.h>
#include <mach/bmp18x_i2c_settings.h>
#endif

#if defined(CONFIG_AL3006) || defined(CONFIG_AL3006_MODULE)
#include <linux/al3006.h>
#include <mach/al3006_i2c_settings.h>
#endif

#if defined(CONFIG_MPU_SENSORS_MPU6050B1)
	|| defined(CONFIG_MPU_SENSORS_MPU6050B1_MODULE)
#include <linux/mpu.h>
#include <mach/mpu6050_settings.h>
#endif

#if defined(CONFIG_MPU_SENSORS_AMI306)
	|| defined(CONFIG_MPU_SENSORS_AMI306_MODULE)
#include <linux/ami306_def.h>
#include <linux/ami_sensor.h>
#include <mach/ami306_settings.h>
#endif

#if defined(CONFIG_KEYBOARD_GPIO) || defined(CONFIG_KEYBOARD_GPIO_MODULE)
#include <linux/input.h>
#include <linux/gpio_keys.h>
#endif


#ifdef CONFIG_BACKLIGHT_PWM
#include <linux/pwm_backlight.h>
#endif

#ifdef CONFIG_BACKLIGHT_KTD259B
#include <linux/ktd259b_bl.h>
#endif

#ifdef CONFIG_FB_BRCM_KONA
#include <video/kona_fb.h>
#endif

#if defined(CONFIG_BCM_ALSA_SOUND)
#include <mach/caph_platform.h>
#include <mach/caph_settings.h>
#endif
#ifdef CONFIG_VIDEO_UNICAM_CAMERA
#include <media/soc_camera.h>
#include "../../../drivers/media/video/camdrv_ss.h"
#endif

#ifdef CONFIG_VIDEO_KONA
#include <media/v4l2-device.h>
#include <media/v4l2-subdev.h>
#include <media/kona-unicam.h>
#endif

#ifdef CONFIG_WD_TAPPER
#include <linux/broadcom/wd-tapper.h>
#endif

#ifdef CONFIG_I2C_GPIO
#include <linux/i2c-gpio.h>
#endif

#if defined(CONFIG_RT8969) || defined(CONFIG_RT8973)
#include <linux/mfd/bcmpmu.h>
#include <linux/power_supply.h>
#include <linux/platform_data/rtmusc.h>
#endif

#ifdef CONFIG_USB_SWITCH_TSU6111
#include <linux/mfd/bcmpmu.h>
#include <linux/power_supply.h>
#include <linux/i2c/tsu6111.h>
#endif
#if defined(CONFIG_SEC_CHARGING_FEATURE)
/* Samsung charging feature */
#include <linux/spa_power.h>
#endif

#if defined(CONFIG_TOUCHSCREEN_BCM915500)
	|| defined(CONFIG_TOUCHSCREEN_BCM915500_MODULE)
#include <linux/i2c/bcmtch15xxx.h>
#endif

#ifdef CONFIG_USB_DWC_OTG
#include <linux/usb/bcm_hsotgctrl.h>
#include <linux/usb/otg.h>
#endif

#ifdef CONFIG_MOBICORE_DRIVER
#include <linux/broadcom/mobicore.h>
#endif


#ifdef CONFIG_BRCM_UNIFIED_DHD_SUPPORT
#include "hawaii_wifi.h"

/* void send_usb_insert_event(enum bcmpmu_event_t event, void *para);*/
void send_chrgr_insert_event(enum bcmpmu_event_t event, void *para);

#include <linux/bmm050.h>
#include <linux/bma150.h>

#include <linux/gp2ap002_dev.h>
#include <linux/gp2ap002.h>

extern int hawaii_wifi_status_register(
		void (*callback)(int card_present, void *dev_id),
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

#define KONA_UART0_PA   UARTB_BASE_ADDR
#define KONA_UART1_PA   UARTB2_BASE_ADDR
#define KONA_UART2_PA   UARTB3_BASE_ADDR

#define HAWAII_8250PORT(name, clk, freq, uart_name, power_save_en)\
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
	.private_data = power_save_en,				\
	.clk_name = clk,					\
	.port_name = uart_name,					\
}

/*This flag is added for saving power for UART GPS. If you want to save power
 * pass the address of this flag as a parameter to power_save_en*/
static bool power_save_enable = 1;

int reset_pwm_padcntrl(void)
{
	struct pin_config new_pin_config;
	int ret;
	new_pin_config.name = PN_GPIO24;
	new_pin_config.func = PF_GPIO24;
	ret = pinmux_set_pin_config(&new_pin_config);
	return ret;
}

#ifdef CONFIG_ION_BCM_NO_DT
struct ion_platform_data ion_system_data = {
	.nr = 1,
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

struct ion_platform_data ion_carveout_data = {
	.nr = 2,
	.heaps = {
		[0] = {
			.id    = 3,
			.type  = ION_HEAP_TYPE_CARVEOUT,
			.name  = "ion-carveout",
			.base  = 0xa0000000,
			.limit = 0xb0000000,
			.size  = (64 * SZ_1M),
#ifdef CONFIG_ION_OOM_KILLER
			.lmk_enable = 0,
			.lmk_min_score_adj = 411,
			.lmk_min_free = 32,
#endif
		},
		[1] = {
			.id    = 1,
			.type  = ION_HEAP_TYPE_CARVEOUT,
			.name  = "ion-carveout-extra",
			.base  = 0,
			.limit = 0xa0000000,
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
	.heaps = {
		[0] = {
			.id = 2,
			.type  = ION_HEAP_TYPE_DMA,
			.name  = "ion-cma",
			.base  = 0xa0000000,
			.limit = 0xb0000000,
			.size  = (192 * SZ_1M),
#ifdef CONFIG_ION_OOM_KILLER
			.lmk_enable = 1,
			.lmk_min_score_adj = 411,
			.lmk_min_free = 32,
#endif
		},
		[1] = {
			.id = 0,
			.type  = ION_HEAP_TYPE_DMA,
			.name  = "ion-cma-extra",
			.base  = 0x00000000,
			.limit = 0xa0000000,
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
#endif /* CONFIG_ION_BCM_NO_DT */

#ifdef CONFIG_VIDEO_UNICAM_CAMERA

#define CAMDRV_SS_MAIN_I2C_ADDRESS (0xAC>>1)
#define CAMDRV_SS_SUB_I2C_ADDRESS (0x60>>1)


#define SENSOR_0_GPIO_PWRDN             (002)
#define SENSOR_0_GPIO_RST               (111)
#define SENSOR_0_CLK                    "dig_ch0_clk" /* DCLK1 */
#define SENSOR_0_CLK_FREQ               (13000000)

#define SENSOR_1_CLK                    "dig_ch0_clk" /* DCLK1 */
#define SENSOR_1_CLK_FREQ               (26000000)

#define SENSOR_1_GPIO_PWRDN             (005)

static struct i2c_board_info hawaii_i2c_camera[] = {
	{
		I2C_BOARD_INFO("camdrv_ss", CAMDRV_SS_MAIN_I2C_ADDRESS),
	},
	{
		I2C_BOARD_INFO("camdrv_ss_sub", CAMDRV_SS_SUB_I2C_ADDRESS),
	},
};

static int hawaii_camera_power(struct device *dev, int on)
{
	static struct pi_mgr_dfs_node unicam_dfs_node;
	int ret;

	printk(KERN_INFO "%s:camera power %s, %d\n", __func__,
		(on ? "on" : "off"), unicam_dfs_node.valid);

	if (!unicam_dfs_node.valid) {
		ret = pi_mgr_dfs_add_request(&unicam_dfs_node, "unicam",
						PI_MGR_PI_ID_MM,
						PI_MGR_DFS_MIN_VALUE);
		if (ret) {
			printk(
			KERN_ERR "%s: failed to register PI DFS request\n",
			__func__
			);
			return -1;
		}
	}

	if (on) {
		if (pi_mgr_dfs_request_update(&unicam_dfs_node, PI_OPP_TURBO)) {
			printk(
			KERN_ERR "%s:failed to update dfs request for unicam\n",
			__func__
			);
			return -1;
		}
	}

	if (!camdrv_ss_power(0, (bool)on)) {
		printk(
		KERN_ERR "%s,camdrv_ss_power failed for MAIN CAM!!\n",
		__func__
		);
		return -1;
	}

	if (!on) {
		if (pi_mgr_dfs_request_update(&unicam_dfs_node,
						PI_MGR_DFS_MIN_VALUE)) {
			printk(
			KERN_ERR"%s: failed to update dfs request for unicam\n",
			__func__);
		}
	}

	return 0;
}
static int hawaii_camera_reset(struct device *dev)
{
	/* reset the camera gpio */
	printk(KERN_INFO "%s:camera reset\n", __func__);
	return 0;
}

static int hawaii_camera_power_sub(struct device *dev, int on)
{
	static struct pi_mgr_dfs_node unicam_dfs_node;
	int ret;

	printk(KERN_INFO "%s:camera power %s, %d\n", __func__,
		(on ? "on" : "off"), unicam_dfs_node.valid);

	if (!unicam_dfs_node.valid) {
		ret = pi_mgr_dfs_add_request(&unicam_dfs_node, "unicam",
						PI_MGR_PI_ID_MM,
						PI_MGR_DFS_MIN_VALUE);
		if (ret) {
			printk(
			KERN_ERR "%s: failed to register PI DFS request\n",
			__func__
			);
			return -1;
		}
	}

	if (on) {
		if (pi_mgr_dfs_request_update(&unicam_dfs_node, PI_OPP_TURBO)) {
			printk(
			KERN_ERR "%s:failed to update dfs request for unicam\n",
			 __func__
			);
			return -1;
		}
	}

	if (!camdrv_ss_power(1, (bool)on)) {
		printk(KERN_ERR "%s, camdrv_ss_power failed for SUB CAM!!\n",
		__func__);
		return -1;
	}

	if (!on) {
		if (pi_mgr_dfs_request_update(&unicam_dfs_node,
						PI_MGR_DFS_MIN_VALUE)) {
			printk(
			KERN_ERR"%s: failed to update dfs request for unicam\n",
			__func__);
		}
	}

	return 0;
}


static int hawaii_camera_reset_sub(struct device *dev)
{
	/* reset the camera gpio */
	printk(KERN_INFO "%s:camera reset\n", __func__);
	return 0;

}

static struct v4l2_subdev_sensor_interface_parms camdrv_ss_main_if_params = {
	.if_type = V4L2_SUBDEV_SENSOR_SERIAL,
	.if_mode = V4L2_SUBDEV_SENSOR_MODE_SERIAL_CSI2,
	.orientation = V4L2_SUBDEV_SENSOR_ORIENT_90,
	.facing = V4L2_SUBDEV_SENSOR_BACK,
	.parms.serial = {
		 .lanes = 2,
		 .channel = 0,
		 .phy_rate = 0,
		 .pix_clk = 0,
		 .hs_term_time = 0x7
	},
};

static struct soc_camera_link iclink_camdrv_ss_main = {
	.bus_id = 0,
	.board_info = &hawaii_i2c_camera[0],
	.i2c_adapter_id = 0,
	.module_name = "camdrv_ss",
	.power = &hawaii_camera_power,
	.reset = &hawaii_camera_reset,
	.priv =  &camdrv_ss_main_if_params,
};

static struct platform_device hawaii_camera = {
	.name = "soc-camera-pdrv",
	 .id = 0,
	 .dev = {
		 .platform_data = &iclink_camdrv_ss_main,
	 },
};

static struct v4l2_subdev_sensor_interface_parms camdrv_ss_sub_if_params = {
	.if_type = V4L2_SUBDEV_SENSOR_SERIAL,
	.if_mode = V4L2_SUBDEV_SENSOR_MODE_SERIAL_CSI2,
	.orientation = V4L2_SUBDEV_SENSOR_ORIENT_270,
	.facing = V4L2_SUBDEV_SENSOR_FRONT,
	.parms.serial = {
		.lanes = 1,
		.channel = 1,
		.phy_rate = 0,
		.pix_clk = 0,
		.hs_term_time = 0x7
	},
};
static struct soc_camera_link iclink_camdrv_ss_sub = {
	.bus_id		= 0,
	.board_info	= &hawaii_i2c_camera[1],
	.i2c_adapter_id	= 0,
	.module_name	= "camdrv_ss_sub",
	.power		= &hawaii_camera_power_sub,
	.reset		= &hawaii_camera_reset_sub,
	.priv		= &camdrv_ss_sub_if_params,
};

static struct platform_device hawaii_camera_sub = {
	.name	= "soc-camera-pdrv",
	.id		= 1,
	.dev	= {
		.platform_data = &iclink_camdrv_ss_sub,
	},
};
#endif /* CONFIG_VIDEO_UNICAM_CAMERA */



static struct plat_serial8250_port hawaii_uart_platform_data[] = {
	HAWAII_8250PORT(UART0, UARTB_PERI_CLK_NAME_STR, 48000000,
					"bluetooth", NULL),
	HAWAII_8250PORT(UART1, UARTB2_PERI_CLK_NAME_STR, 26000000,
				"gps", &power_save_enable),
	HAWAII_8250PORT(UART2, UARTB3_PERI_CLK_NAME_STR, 26000000,
				"console", NULL),
	{
		.flags = 0,
	},
};

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
	&hawaii_serial_device,
	&hawaii_i2c_adap_devices[0],
	&hawaii_i2c_adap_devices[1],
	&hawaii_i2c_adap_devices[2],
	&hawaii_i2c_adap_devices[3],
	&hawaii_i2c_adap_devices[4],
	&pmu_device,
	&hawaii_pwm_device,
	&hawaii_ssp0_device,

#ifdef CONFIG_SENSORS_KONA
	&thermal_device,
#endif

#ifdef CONFIG_STM_TRACE
/* &hawaii_stm_device, */
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

#ifdef CONFIG_UNICAM
	&hawaii_unicam_device,
#endif

#ifdef CONFIG_VIDEO_UNICAM_CAMERA
	&hawaii_camera_device,
	&hawaii_camera,
	&hawaii_camera_sub,
#endif

#ifdef CONFIG_SND_BCM_SOC
	&hawaii_audio_device,
	&caph_i2s_device,
	&caph_pcm_device,
	&spdif_dit_device,

#endif

#ifdef CONFIG_KONA_MEMC
	&kona_memc_device,
#endif
};

struct regulator_consumer_supply hv6_supply[] = {
	{.supply = "vdd_sdxc"},
	{.supply = "sddat_debug_bus"},
};

struct regulator_consumer_supply sd_supply[] = {
	{.supply = "sdldo_uc"},
	REGULATOR_SUPPLY("vddmmc", "sdhci.3"), /* 0x3f1b0000.sdhci */
	{.supply = "vdd_sdio"},
};

struct regulator_consumer_supply sdx_supply[] = {
	{.supply = "sdxldo_uc"},
	REGULATOR_SUPPLY("vddo", "sdhci.3"), /* 0x3f1b0000.sdhci */
	{.supply = "vdd_sdxc"},
};

#ifdef CONFIG_KEYBOARD_BCM
static struct bcm_keymap hawaii_keymap[] = {
	{BCM_KEY_ROW_0, BCM_KEY_COL_0, "unused", 0},
	{BCM_KEY_ROW_0, BCM_KEY_COL_1, "Vol Down Key", KEY_VOLUMEDOWN},
	{BCM_KEY_ROW_0, BCM_KEY_COL_2, "unused", 0},
	{BCM_KEY_ROW_0, BCM_KEY_COL_3, "Vol Up Key", KEY_VOLUMEUP},
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
	.row_num = 1,
	.col_num = 4,
	.keymap = hawaii_keymap,
	.bcm_keypad_base = (void *)__iomem HW_IO_PHYS_TO_VIRT(KEYPAD_BASE_ADDR),
};

#endif

#define GPIO_KEYS_SETTINGS { \
	{ KEY_HOME, 10, 1, "HOME", EV_KEY, 0, 64}, \
}

#if defined(CONFIG_KEYBOARD_GPIO) || defined(CONFIG_KEYBOARD_GPIO_MODULE)
static struct gpio_keys_button board_gpio_keys[] = GPIO_KEYS_SETTINGS;

static struct gpio_keys_platform_data gpio_keys_data = {
	.nbuttons = ARRAY_SIZE(board_gpio_keys),
	.buttons = board_gpio_keys,
};

static struct platform_device board_gpio_keys_device = {
	.name = "gpio-keys",
	.id = -1,
	.dev = {
		.platform_data = &gpio_keys_data,
	},
};
#endif

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

#if  defined(CONFIG_BMP18X) || defined(CONFIG_BMP18X_I2C)
	|| defined(CONFIG_BMP18X_I2C_MODULE)
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

#if defined(CONFIG_MPU_SENSORS_AMI306)
	|| defined(CONFIG_MPU_SENSORS_AMI306_MODULE)
static struct ami306_platform_data ami306_data = AMI306_DATA;
static struct i2c_board_info __initdata i2c_ami306_info[] = {
	{
		I2C_BOARD_INFO(AMI_DRV_NAME, AMI_I2C_ADDRESS),
		.platform_data = &ami306_data,
	},
};
#endif

#if defined(CONFIG_MPU_SENSORS_MPU6050B1)
	|| defined(CONFIG_MPU_SENSORS_MPU6050B1_MODULE)

static struct mpu_platform_data mpu6050_platform_data = {
	.int_config  = MPU6050_INIT_CFG,
	.level_shifter = 0,
	.orientation = MPU6050_DRIVER_ACCEL_GYRO_ORIENTATION,
};

/*static struct ext_slave_platform_data mpu_compass_data =
{
//	.bus = EXT_SLAVE_BUS_SECONDARY,
	.orientation = MPU6050_DRIVER_COMPASS_ORIENTATION,
};*/


static struct i2c_board_info __initdata inv_mpu_i2c0_boardinfo[] = {
	{
		I2C_BOARD_INFO("mpu6050", MPU6050_SLAVE_ADDR),
		.platform_data = &mpu6050_platform_data,
	},
/*	{
		I2C_BOARD_INFO("ami_sensor", MPU6050_COMPASS_SLAVE_ADDR),
		.platform_data = &mpu_compass_data,
		.irq =  gpio_to_irq(3),
	},*/
};

#endif /* CONFIG_MPU_SENSORS_MPU6050B1 */



#if defined(CONFIG_SENSORS_BMA2X2)

static struct bma222_accl_platform_data bma_pdata = {
	.orientation = BMA_ROT_90,
	.invert = false,
};
#endif

#if defined(CONFIG_SENSORS_BMA2X2) || defined(CONFIG_SENSORS_BMM050)

static struct i2c_board_info __initdata bmm150_boardinfo[] = {
#if defined(CONFIG_SENSORS_BMA2X2)
		{
			I2C_BOARD_INFO("bma2x2", 0x10),
			.irq = -1,
			.platform_data = &bma_pdata,
		},
#endif

#if defined(CONFIG_SENSORS_BMM050)
		I2C_BOARD_INFO("bmm050", 0x12),
#endif


};

#endif

#if defined(CONFIG_SENSORS_GP2A)
#define PROXI_INT_GPIO_PIN 89
static struct gp2ap002_platform_data gp2ap002_platform_data = {
	.irq_gpio = PROXI_INT_GPIO_PIN,
	.irq = gpio_to_irq(PROXI_INT_GPIO_PIN),
};

static struct i2c_board_info __initdata proxy_boardinfo[] = {
	{
		I2C_BOARD_INFO("gp2ap002", 0x44),
		.platform_data = &gp2ap002_platform_data,
	}
};

#endif


#ifdef CONFIG_KONA_HEADSET_MULTI_BUTTON

#define HS_IRQ		gpio_to_irq(121)
#define HSB_IRQ		BCM_INT_ID_AUXMIC_COMP2
#define HSB_REL_IRQ	BCM_INT_ID_AUXMIC_COMP2_INV
static unsigned int hawaii_button_adc_values[3][2] = {
	/* SEND/END Min, Max*/
	{0,	10},
	/* Volume Up  Min, Max*/
	{11, 30},
	/* Volue Down Min, Max*/
	{30, 680},
};

static unsigned int hawaii_button_adc_values_2_1[3][2] = {
	/* SEND/END Min, Max*/
	{0,     110},
	/* Volume Up  Min, Max*/
	{111,   250},
	/* Volue Down Min, Max*/
	{251,   500},
};
static struct kona_headset_pd hawaii_headset_data = {
	/* GPIO state read is 0 on HS insert and 1 for
	 * HS remove
	 */

	.hs_default_state = 1,
	/*
	 * Because of the presence of the resistor in the MIC_IN line.
	 * The actual ground may not be 0, but a small offset is added to it.
	 * This needs to be subtracted from the measured voltage to determine
	 * the correct value. This will vary for different HW based
	 * on the resistor values used.
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
	/* .button_adc_values_low = hawaii_button_adc_values,
	*/
	.button_adc_values_low = 0,

	/*
	 * Pass the board specific button detection range
	 */
	.button_adc_values_high = hawaii_button_adc_values_2_1,

};
#endif /* CONFIG_KONA_HEADSET_MULTI_BUTTON */

#ifdef CONFIG_DMAC_PL330
static struct kona_pl330_data hawaii_pl330_pdata =	{
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

#if (defined(CONFIG_BCM_RFKILL) || defined(CONFIG_BCM_RFKILL_MODULE))
#define BCMBT_VREG_GPIO		28
#define BCMBT_N_RESET_GPIO	71
#define BCMBT_AUX0_GPIO		(-1)   /* clk32 */
#define BCMBT_AUX1_GPIO		(-1)   /* UARTB_SEL */

static struct bcmbt_rfkill_platform_data hawaii_bcmbt_rfkill_cfg = {
	.vreg_gpio = BCMBT_VREG_GPIO,
	.n_reset_gpio = BCMBT_N_RESET_GPIO,
	.aux0_gpio = BCMBT_AUX0_GPIO,  /* CLK32 */
	.aux1_gpio = BCMBT_AUX1_GPIO,  /* UARTB_SEL, probably not required */
};

static struct platform_device hawaii_bcmbt_rfkill_device = {
	.name = "bcmbt-rfkill",
	.id = -1,
	.dev =	{
		.platform_data = &hawaii_bcmbt_rfkill_cfg,
	},
};
#endif

#ifdef CONFIG_BCM_BZHW
#define GPIO_BT_WAKE	32
#define GPIO_HOST_WAKE	72
static struct bcm_bzhw_platform_data bcm_bzhw_data = {
	.gpio_bt_wake = GPIO_BT_WAKE,
	.gpio_host_wake = GPIO_HOST_WAKE,
};

static struct platform_device hawaii_bcm_bzhw_device = {
	.name = "bcm_bzhw",
	.id = -1,
	.dev =	{
		.platform_data = &bcm_bzhw_data,
	},
};
#endif


#ifdef CONFIG_BCM_BT_LPM
#define GPIO_BT_WAKE	32
#define GPIO_HOST_WAKE	72

static struct bcmbt_platform_data brcm_bt_lpm_data = {
	.bt_wake_gpio = GPIO_BT_WAKE,
	.host_wake_gpio = GPIO_HOST_WAKE,
	.bt_uart_port = 0,
};

static struct platform_device board_bcmbt_lpm_device = {
	.name = "bcmbt-lpm",
	.id = -1,
	.dev = {
		.platform_data = &brcm_bt_lpm_data,
	},
};
#endif



#ifdef CONFIG_BYPASS_WIFI_DEVTREE
static struct board_wifi_info brcm_wifi_data = {
	.wl_reset_gpio = 3,
	.host_wake_gpio = 74,
	.board_nvram_file = "/system/vendor/firmware/fw_wifi_nvram_4330.txt",
	.module_name = "bcm-wifi",
};
static struct platform_device board_wifi_driver_device = {

	.name = "bcm_wifi",
	.id = -1,
	.dev = {
		.platform_data = &brcm_wifi_data,
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
	.name   = "samsung_pwm_haptic",
	.id     = -1,
	.dev	=	 {	.platform_data = &haptic_control_data,}
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
		/*The SD card regulator*/
		.vddo_regulator_name = "vdd_sdio",
		/*The SD controller regulator*/
		.vddsdxc_regulator_name = "vdd_sdxc",
		.configure_sdio_pullup = configure_sdio_pullup,
	},
	{
		.id = 1,
		.data_pullup = 0,
		.is_8bit = 1,
		.devtype = SDIO_DEV_TYPE_EMMC,
		.flags = KONA_SDIO_FLAGS_DEVICE_NON_REMOVABLE ,
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
	.pwm_id		= 2,
	.max_brightness	= 32,   /* Android calibrates to 32 levels*/
	.dft_brightness	= 32,
	.polarity	= 1,    /* Inverted polarity */
	.pwm_period_ns	= 1000000,
};

#endif /*CONFIG_BACKLIGHT_PWM */

#ifdef CONFIG_BACKLIGHT_KTD259B

static struct platform_ktd259b_backlight_data bcm_ktd259b_backlight_data = {
	.max_brightness = 255,
	.dft_brightness = 160,
	.ctrl_pin = 24,
};

struct platform_device hawaii_backlight_device = {
	.name = "panel",
	.id = -1,
	.dev = {
		.platform_data = &bcm_ktd259b_backlight_data,
	},
};

#endif /* CONFIG_BACKLIGHT_KTD259B */

/* Remove this comment when camera data for Hawaii is updated */

#ifdef CONFIG_WD_TAPPER
static struct wd_tapper_platform_data wd_tapper_data = {
	/* Set the count to the time equivalent to the time-out in seconds
	 * required to pet the PMU watchdog to overcome the problem of reset in
	 * suspend*/
	.count = 120,
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

#if defined(CONFIG_TOUCHKEY_BACKLIGHT)
static struct platform_device touchkeyled_device = {
	.name = "touchkey-led",
	.id = -1,
};
#endif

#if defined(CONFIG_TOUCHSCREEN_IST30XX)	\
	|| defined(CONFIG_TOUCHSCREEN_BT432_LOGAN)
#define TSP_INT_GPIO_PIN	(73)
static struct i2c_board_info __initdata zinitix_i2c_devices[] = {
	  {
		I2C_BOARD_INFO("sec_touch", 0x50),
		.irq = gpio_to_irq(TSP_INT_GPIO_PIN),
	  },
	   {
		I2C_BOARD_INFO("zinitix_touch", 0x20),
		.irq = gpio_to_irq(TSP_INT_GPIO_PIN),
	  },
};
#endif

#ifdef CONFIG_TOUCHSCREEN_FT5306
static int ts_power(ts_power_status vreg_en)
{
	struct regulator *reg = NULL;
	if (!reg) {
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
	{	/* New touch screen i2c slave address. */
		I2C_BOARD_INFO("FocalTech-Ft5306", (0x70>>1)),
		.platform_data = &ft5306_plat_data,
		.irq = gpio_to_irq(TSC_GPIO_IRQ_PIN),
	},
};
#endif

#if defined(CONFIG_TOUCHSCREEN_BCM915500) ||	\
	defined(CONFIG_TOUCHSCREEN_BCM915500_MODULE)
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
					.dock_aud_route_gpio = -1,
					.mic_sel_aud_route_gpio = -1,
					}
				}
};
#endif

static struct platform_device board_caph_device = {
	.name = "brcm_caph_device",
	.id = -1, /*Indicates only one device */
	.dev = {
		.platform_data = &board_caph_platform_cfg,
	},
};

#endif /* CONFIG_BCM_ALSA_SOUND */

#ifdef CONFIG_RT8973

#ifdef CONFIG_HAS_WAKELOCK
static struct wake_lock rt8973_jig_wakelock;
#endif
#ifdef CONFIG_KONA_PI_MGR
static struct pi_mgr_qos_node qos_node;
#endif

enum cable_type_t {
	CABLE_TYPE_USB,
	CABLE_TYPE_AC,
	CABLE_TYPE_NONE
};
static void rt8973_wakelock_init(void)
{
#ifdef CONFIG_HAS_WAKELOCK
	wake_lock_init(&rt8973_jig_wakelock, WAKE_LOCK_SUSPEND,
		"rt8973_jig_wakelock");
#endif

#ifdef CONFIG_KONA_PI_MGR
	pi_mgr_qos_add_request(&qos_node, "rt8973_jig_qos",
		PI_MGR_PI_ID_ARM_SUB_SYSTEM, PI_MGR_QOS_DEFAULT_VALUE);
#endif
}

extern int bcmpmu_accy_chrgr_type_notify(int chrgr_type);

void send_chrgr_insert_event(enum bcmpmu_event_t event, void *para)
{
	bcmpmu_accy_chrgr_type_notify(*(u32 *)para);
}

static enum cable_type_t set_cable_status;

static void usb_attach(uint8_t attached)
{
	enum bcmpmu_chrgr_type_t chrgr_type;
	enum bcmpmu_usb_type_t usb_type;

	printk(attached ? "USB attached\n" : "USB detached\n");

#if defined(CONFIG_SEC_CHARGING_FEATURE)
	int spa_data = POWER_SUPPLY_TYPE_BATTERY;
#endif
	pr_info("%s: attached %d\n", __func__, attached);

	set_cable_status = attached ? CABLE_TYPE_USB : CABLE_TYPE_NONE;

	switch (set_cable_status) {
	case CABLE_TYPE_USB:
		usb_type = PMU_USB_TYPE_SDP;
		chrgr_type = PMU_CHRGR_TYPE_SDP;
#if defined(CONFIG_SEC_CHARGING_FEATURE)
		spa_data = POWER_SUPPLY_TYPE_USB_CDP;
#endif
		pr_info("%s USB attached\n", __func__);
		/* send_usb_insert_event(BCMPMU_USB_EVENT_USB_DETECTION,
			&usb_type); */
		break;
	case CABLE_TYPE_NONE:
		usb_type = PMU_USB_TYPE_NONE;
		chrgr_type = PMU_CHRGR_TYPE_NONE;
		spa_data = POWER_SUPPLY_TYPE_BATTERY;
		pr_info("%s USB removed\n", __func__);
		/* set_usb_connection_status(&usb_type);  // only set status */
		break;
	}
	send_chrgr_insert_event(BCMPMU_CHRGR_EVENT_CHGR_DETECTION, &chrgr_type);
#if defined(CONFIG_SEC_CHARGING_FEATURE)
	spa_event_handler(SPA_EVT_CHARGER, spa_data);
#endif
	pr_info("tsu6111_usb_cb attached %d\n", attached);
}

__weak int musb_info_handler(struct notifier_block *nb,
	unsigned long event, void *para)
{
	return 0;
}

static void uart_attach(uint8_t attached)
{
	printk(attached ? "UART attached\n" : "UART detached\n");

	pr_info("%s attached %d\n", __func__, attached);
	if (attached) {
#ifndef CONFIG_SEC_MAKE_LCD_TEST
#ifdef CONFIG_HAS_WAKELOCK
		if (!wake_lock_active(&rt8973_jig_wakelock))
			wake_lock(&rt8973_jig_wakelock);
#endif
#ifdef CONFIG_KONA_PI_MGR
		pi_mgr_qos_request_update(&qos_node, 0);
#endif
#endif
		musb_info_handler(NULL, 0, 1);
	} else {
#ifndef CONFIG_SEC_MAKE_LCD_TEST
#ifdef CONFIG_HAS_WAKELOCK
		if (wake_lock_active(&rt8973_jig_wakelock))
			wake_unlock(&rt8973_jig_wakelock);
#endif
#ifdef CONFIG_KONA_PI_MGR
		pi_mgr_qos_request_update(&qos_node,
			PI_MGR_QOS_DEFAULT_VALUE);
#endif
#endif
		musb_info_handler(NULL, 0, 0);
	}
}

#if defined(CONFIG_TOUCHSCREEN_IST30XX)
extern void ist30xx_set_ta_mode(bool charging);
#endif

static void charger_attach(uint8_t attached)
{
	enum bcmpmu_chrgr_type_t chrgr_type;
	enum cable_type_t set_cable_status;

	printk(attached ? "Charger attached\n" : "Charger detached\n");

#if defined(CONFIG_SEC_CHARGING_FEATURE)
	int spa_data = POWER_SUPPLY_TYPE_BATTERY;
#endif

	pr_info("tsu6111_charger_cb attached %d\n", attached);

	set_cable_status = attached ? CABLE_TYPE_AC : CABLE_TYPE_NONE;
	switch (set_cable_status) {
	case CABLE_TYPE_AC:
		chrgr_type = PMU_CHRGR_TYPE_DCP;
		pr_info("%s TA attached\n", __func__);
		#if defined(CONFIG_TOUCHSCREEN_IST30XX)
			ist30xx_set_ta_mode(1);
		#endif
#if defined(CONFIG_SEC_CHARGING_FEATURE)
		spa_data = POWER_SUPPLY_TYPE_USB_DCP;
#endif
		break;
	case CABLE_TYPE_NONE:
		chrgr_type = PMU_CHRGR_TYPE_NONE;
		pr_info("%s TA removed\n", __func__);
		#if defined(CONFIG_TOUCHSCREEN_IST30XX)
			ist30xx_set_ta_mode(0);
		#endif
		break;
	default:
		break;
	}

	send_chrgr_insert_event(BCMPMU_CHRGR_EVENT_CHGR_DETECTION,
		&chrgr_type);
#if defined(CONFIG_SEC_CHARGING_FEATURE)
	spa_event_handler(SPA_EVT_CHARGER, spa_data);
#endif
	pr_info("tsu6111_charger_cb attached %d\n", attached);
}

static void jig_attach(uint8_t attached, uint8_t factory_mode)
{
	switch (factory_mode) {
	case RTMUSC_FM_BOOT_OFF_UART:
		printk(KERN_INFO "JIG BOOT OFF UART\n");
		break;
	case RTMUSC_FM_BOOT_OFF_USB:
		printk(KERN_INFO "JIG BOOT OFF USB\n");
		break;
	case RTMUSC_FM_BOOT_ON_UART:
		printk(KERN_INFO "JIG BOOT ON UART\n");
		break;
	case RTMUSC_FM_BOOT_ON_USB:
		printk(KERN_INFO "JIG BOOT ON USB\n");
		break;
	default:
		break;
	}
	pr_info("tsu6111_jig_cb attached %d\n", attached);
	if (attached) {
#ifndef CONFIG_SEC_MAKE_LCD_TEST
#ifdef CONFIG_HAS_WAKELOCK
		if (!wake_lock_active(&rt8973_jig_wakelock))
			wake_lock(&rt8973_jig_wakelock);
#endif
#ifdef CONFIG_KONA_PI_MGR
			pi_mgr_qos_request_update(&qos_node, 0);
#endif
#endif
	} else {
#ifndef CONFIG_SEC_MAKE_LCD_TEST
#ifdef CONFIG_HAS_WAKELOCK
		if (wake_lock_active(&rt8973_jig_wakelock))
			wake_unlock(&rt8973_jig_wakelock);
#endif
#ifdef CONFIG_KONA_PI_MGR
		pi_mgr_qos_request_update(&qos_node,
			PI_MGR_QOS_DEFAULT_VALUE);
#endif
#endif
	}
	printk(attached ? "Jig attached\n" : "Jig detached\n");
}

static void over_temperature(uint8_t detected)
{
	printk(KERN_INFO "over temperature detected = %d!\n", detected);
}

static void over_voltage(uint8_t detected)
{
	printk(KERN_INFO "over voltage = %d\n", (int32_t)detected);
	printk(KERN_INFO "OVP triggered by musb - %d\n", detected);
	spa_event_handler(SPA_EVT_OVP, detected);
}
static void set_usb_power(uint8_t on)
{
	printk(on ? "on resume() : Set USB on\n" :
		"on suspend() : Set USB off\n");
}

void uas_jig_force_sleep(void)
{
#ifdef CONFIG_HAS_WAKELOCK
	if (wake_lock_active(&rt8973_jig_wakelock)) {
		wake_unlock(&rt8973_jig_wakelock);
		pr_info("Force unlock jig_uart_wl\n");
	}
#else
	pr_info("Warning : %s - Empty function!!!\n", __func__);
#endif
#ifdef CONFIG_KONA_PI_MGR
	pi_mgr_qos_request_update(&qos_node, PI_MGR_QOS_DEFAULT_VALUE);
#endif
	return;
}

static struct rtmus_platform_data __initdata rtmus_pdata = {
	.usb_callback = &usb_attach,
	.uart_callback = &uart_attach,
	.charger_callback = &charger_attach,
	.jig_callback = &jig_attach,
	.over_temperature_callback = &over_temperature,
	.charging_complete_callback = NULL,
	.over_voltage_callback = &over_voltage,
	.usb_power = &set_usb_power,
};


/* For you device setting */
static struct i2c_board_info __initdata micro_usb_i2c_devices_info[]  = {
/* Add for Ricktek RT8969 */
#ifdef CONFIG_RT8973
	{I2C_BOARD_INFO("rt8973", 0x28>>1),
	.irq = -1,
	.platform_data = &rtmus_pdata,},
#endif
};

static struct i2c_gpio_platform_data mUSB_i2c_gpio_data = {
	.sda_pin		= GPIO_USB_I2C_SDA,
	.scl_pin		= GPIO_USB_I2C_SCL,
	.udelay			= 2,
};

static struct platform_device mUSB_i2c_gpio_device = {
	.name                   = "i2c-gpio",
	.id                     = RT_I2C_BUS_ID,
	.dev                    = {
		.platform_data  = &mUSB_i2c_gpio_data,
	},
};

static struct platform_device *mUSB_i2c_devices[] __initdata = {
	&mUSB_i2c_gpio_device,
};
#endif



#ifdef CONFIG_USB_SWITCH_TSU6111
enum cable_type_t {
		CABLE_TYPE_USB,
		CABLE_TYPE_AC,
		CABLE_TYPE_NONE
};


#define FSA9485_I2C_BUS_ID 8
#define GPIO_USB_I2C_SDA 113
#define GPIO_USB_I2C_SCL 114
#define GPIO_USB_INT 56

#ifdef CONFIG_HAS_WAKELOCK
static struct wake_lock fsa9485_jig_wakelock;
#endif

#ifdef CONFIG_KONA_PI_MGR
static struct pi_mgr_qos_node qos_node;
#endif

static void fsa9485_wakelock_init(void)
{
#ifdef CONFIG_HAS_WAKELOCK
	wake_lock_init(&fsa9485_jig_wakelock, WAKE_LOCK_SUSPEND,
		"fsa9485_jig_wakelock");
#endif

#ifdef CONFIG_KONA_PI_MGR
	pi_mgr_qos_add_request(&qos_node, "fsa9485_jig_qos",
		PI_MGR_PI_ID_ARM_SUB_SYSTEM, PI_MGR_QOS_DEFAULT_VALUE);
#endif
}

static void fsa9485_usb_cb(bool attached)
{
	pr_info("fsa9485_usb_cb attached %d\n", attached);
}

static void fsa9485_charger_cb(bool attached)
{
	pr_info("fsa9480_charger_cb attached %d\n", attached);
}

static void fsa9485_jig_cb(bool attached)
{
	pr_info("fsa9480_jig_cb attached %d\n", attached);
	if (attached) {
		#ifdef CONFIG_HAS_WAKELOCK
			if (!wake_lock_active(&fsa9485_jig_wakelock))
				wake_lock(&fsa9485_jig_wakelock);
		#endif
		#ifdef CONFIG_KONA_PI_MGR
			pi_mgr_qos_request_update(&qos_node, 0);
		#endif
	} else {
		#ifdef CONFIG_HAS_WAKELOCK
			if (wake_lock_active(&fsa9485_jig_wakelock))
				wake_unlock(&fsa9485_jig_wakelock);
		#endif
		#ifdef CONFIG_KONA_PI_MGR
			pi_mgr_qos_request_update(&qos_node,
				PI_MGR_QOS_DEFAULT_VALUE);
		#endif
	}
}

static void fsa9485_uart_cb(bool attached)
{
	pr_info("fsa9485_uart_cb attached %d\n", attached);
	if (attached) {
		#ifdef CONFIG_HAS_WAKELOCK
		if (!wake_lock_active(&fsa9485_jig_wakelock))
			wake_lock(&fsa9485_jig_wakelock);
		#endif
		#ifdef CONFIG_KONA_PI_MGR
			pi_mgr_qos_request_update(&qos_node, 0);
		#endif
	} else {
		#ifdef CONFIG_HAS_WAKELOCK
		if (wake_lock_active(&fsa9485_jig_wakelock))
			wake_unlock(&fsa9485_jig_wakelock);
		#endif
		#ifdef CONFIG_KONA_PI_MGR
			pi_mgr_qos_request_update(&qos_node,
				PI_MGR_QOS_DEFAULT_VALUE);
		#endif
		}

}

void uas_jig_force_sleep(void)
{
	#ifdef CONFIG_HAS_WAKELOCK
	if (wake_lock_active(&fsa9485_jig_wakelock)) {
		wake_unlock(&fsa9485_jig_wakelock);
		pr_info("Force unlock jig_uart_wl\n");
	}
	#endif
	#ifdef CONFIG_KONA_PI_MGR
	pi_mgr_qos_request_update(&qos_node, PI_MGR_QOS_DEFAULT_VALUE);
	#endif
}

static struct tsu6111_platform_data fsa9485_pdata = {
	.usb_cb = fsa9485_usb_cb,
	.charger_cb = fsa9485_charger_cb,
	.jig_cb = fsa9485_jig_cb,
	.uart_cb = fsa9485_uart_cb,
};

static struct i2c_board_info  __initdata micro_usb_i2c_devices_info[]  = {
	{
		I2C_BOARD_INFO("fsa9485", 0x4A >> 1),
		.platform_data = &fsa9485_pdata,
		.irq = gpio_to_irq(GPIO_USB_INT),
	},
};

static struct i2c_gpio_platform_data fsa_i2c_gpio_data = {
	.sda_pin = GPIO_USB_I2C_SDA,
	.scl_pin = GPIO_USB_I2C_SCL,
	.udelay = 2,
	};

static struct platform_device fsa_i2c_gpio_device = {
	.name = "i2c-gpio",
	.id = FSA9485_I2C_BUS_ID,
	.dev = {
	.platform_data = &fsa_i2c_gpio_data,
	},
};

static struct platform_device *mUSB_i2c_devices[] __initdata = {
	&fsa_i2c_gpio_device,
};

#endif

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

#if	defined(CONFIG_BACKLIGHT_PWM) || defined(CONFIG_BACKLIGHT_KTD259B)
	&hawaii_backlight_device,
#endif

#if (defined(CONFIG_BCM_RFKILL) || defined(CONFIG_BCM_RFKILL_MODULE))
	&hawaii_bcmbt_rfkill_device,
#endif

#ifdef CONFIG_BCM_BZHW
	&hawaii_bcm_bzhw_device,
#endif

#ifdef CONFIG_BCM_BT_LPM
	&board_bcmbt_lpm_device,
#endif

#ifdef CONFIG_BYPASS_WIFI_DEVTREE
	&board_wifi_driver_device,
#endif

#ifdef CONFIG_VIDEO_KONA
	&hawaii_unicam_device,
#endif

#ifdef CONFIG_WD_TAPPER
	&wd_tapper,
#endif

#if defined(CONFIG_BCM_ALSA_SOUND)
	&board_caph_device,
#endif

#if defined(CONFIG_TOUCHKEY_BACKLIGHT)
	&touchkeyled_device
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
#ifdef CONFIG_TOUCHSCREEN_TANGO
	i2c_register_board_info(3, tango_info, ARRAY_SIZE(tango_info));
#endif
#if defined(CONFIG_TOUCHSCREEN_IST30XX)	\
		|| defined(CONFIG_TOUCHSCREEN_BT432_LOGAN)
	i2c_register_board_info(3, zinitix_i2c_devices,
		ARRAY_SIZE(zinitix_i2c_devices));
#endif

#ifdef CONFIG_TOUCHSCREEN_FT5306
	i2c_register_board_info(3, ft5306_info, ARRAY_SIZE(ft5306_info));
#endif

#ifdef CONFIG_SENSORS_BMA222
	i2c_register_board_info(2, bma222_accl_info,
		ARRAY_SIZE(bma222_accl_info));
#endif

#if defined(CONFIG_TOUCHSCREEN_BCM915500)
	|| defined(CONFIG_TOUCHSCREEN_BCM915500_MODULE)
	i2c_register_board_info(3, bcm915500_i2c_boardinfo,
				ARRAY_SIZE(bcm915500_i2c_boardinfo));
#endif

#ifdef CONFIG_USB_SWITCH_TSU6111
	pr_info("tsu6111\n");
#ifdef CONFIG_HAS_WAKELOCK
	fsa9485_wakelock_init();
#endif
	i2c_register_board_info(FSA9485_I2C_BUS_ID, micro_usb_i2c_devices_info,
		ARRAY_SIZE(micro_usb_i2c_devices_info));
#endif

#ifdef CONFIG_RT8973
	pr_info("rt8973: micro_usb_i2c_devices_info\n");
	rt8973_wakelock_init();
	i2c_register_board_info(RT_I2C_BUS_ID, micro_usb_i2c_devices_info,
		ARRAY_SIZE(micro_usb_i2c_devices_info));
#endif

#if defined(CONFIG_USB_SWITCH_TSU6111) || defined(CONFIG_RT8973)
	pr_info("mUSB_i2c_devices\n");
	platform_add_devices(mUSB_i2c_devices, ARRAY_SIZE(mUSB_i2c_devices));
#endif

#if defined(CONFIG_BCMI2CNFC)
	i2c_register_board_info(1, bcmi2cnfc, ARRAY_SIZE(bcmi2cnfc));
#endif

#if defined(CONFIG_MPU_SENSORS_MPU6050B1)
	|| defined(CONFIG_MPU_SENSORS_MPU6050B1_MODULE)
#if defined(MPU6050_IRQ_GPIO)
	inv_mpu_i2c0_boardinfo[0].irq = gpio_to_irq(MPU6050_IRQ_GPIO);
#endif
	i2c_register_board_info(MPU6050_I2C_BUS_ID,
		inv_mpu_i2c0_boardinfo, ARRAY_SIZE(inv_mpu_i2c0_boardinfo));
#endif

#if defined(CONFIG_SENSORS_BMA2X2) || defined(CONFIG_SENSORS_BMM050)
	i2c_register_board_info(2,
	bmm150_boardinfo, ARRAY_SIZE(bmm150_boardinfo));
#endif

#if defined(CONFIG_SENSORS_GP2A)
	i2c_register_board_info(2,
		proxy_boardinfo, ARRAY_SIZE(proxy_boardinfo));
#endif

#if  defined(CONFIG_BMP18X) || defined(CONFIG_BMP18X_I2C)
	|| defined(CONFIG_BMP18X_I2C_MODULE)
	i2c_register_board_info(
#ifdef BMP18X_I2C_BUS_ID
			BMP18X_I2C_BUS_ID,
#else
			-1,
#endif
			i2c_bmp18x_info, ARRAY_SIZE(i2c_bmp18x_info));
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
		i2c_al3006_info, ARRAY_SIZE(i2c_al3006_info));
#endif /* CONFIG_AL3006 */

#if  defined(CONFIG_MPU_SENSORS_AMI306) || defined(CONFIG_MPU_SENSORS_AMI306)
	i2c_register_board_info(
#ifdef AMI306_I2C_BUS_ID
			AMI306_I2C_BUS_ID,
#else
			-1,
#endif
			i2c_ami306_info, ARRAY_SIZE(i2c_ami306_info));
#endif


}

/* The GPIO used to indicate accessory insertion in this board */
#define HS_IRQ		gpio_to_irq(121)

static void hawaii_add_pdata(void)
{
	hawaii_serial_device.dev.platform_data = &hawaii_uart_platform_data;
	hawaii_i2c_adap_devices[0].dev.platform_data = &bsc_i2c_cfg[0];
	hawaii_i2c_adap_devices[1].dev.platform_data = &bsc_i2c_cfg[1];
	hawaii_i2c_adap_devices[2].dev.platform_data = &bsc_i2c_cfg[2];
	hawaii_i2c_adap_devices[3].dev.platform_data = &bsc_i2c_cfg[3];
	hawaii_i2c_adap_devices[4].dev.platform_data = &bsc_i2c_cfg[4];
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
#ifdef CONFIG_BACKLIGHT_PWM
	hawaii_backlight_device.dev.platform_data = &hawaii_backlight_data;
#endif

#ifdef CONFIG_USB_DWC_OTG
	hawaii_hsotgctrl_platform_device.dev.platform_data
		= &hsotgctrl_plat_data;
	hawaii_usb_phy_platform_device.dev.platform_data
		= &hsotgctrl_plat_data;
#endif
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
	platform_device_register(&ion_carveout_device);
#ifdef CONFIG_CMA
	platform_device_register(&ion_cma_device);
#endif
#endif

#ifdef CONFIG_KEYBOARD_BCM
	hawaii_kp_device.dev.platform_data = &hawaii_keypad_data;
#endif

#if defined(CONFIG_KEYBOARD_GPIO) || defined(CONFIG_KEYBOARD_GPIO_MODULE)
	platform_device_register(&board_gpio_keys_device);
#endif

	platform_add_devices(hawaii_devices, ARRAY_SIZE(hawaii_devices));

	hawaii_add_i2c_devices();

	spi_register_board_info(spi_slave_board_info,
				ARRAY_SIZE(spi_slave_board_info));
}

#ifdef CONFIG_MOBICORE_OS
static void hawaii_mem_reserve(void)
{
	mobicore_device.dev.platform_data = &mobicore_plat_data;
	hawaii_reserve();
}
#endif

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
		.name = "NT35510",
		.reg_name = "cam2",
		.rst =  {
			.gpio = 22,
			.setup = 5,
			.pulse = 20,
			.hold = 10000,
			.active = false,
		},
		.vmode = false,
		.vburst = false,
		.cmnd_LP = true,
		.te_ctrl = false,
		.col_mod_i = 3,  /*DISPDRV_FB_FORMAT_xBGR8888*/
		.col_mod_o = 2, /*DISPDRV_FB_FORMAT_xRGB8888*/
		.width = 480,
		.height = 800,
		.fps = 60,
		.lanes = 2,
		.hs_bps = 500000000,
		.lp_bps = 5000000,
#ifdef CONFIG_IOMMU_API
		.pdev_iommu = &iovmm_mm_device,
#endif
#ifdef CONFIG_BCM_IOVMM
		.pdev_iovmm = &iovmm_mm_256mb_device,
#endif
	},
};

#include "kona_fb_init.c"
#endif /* #ifdef CONFIG_FB_BRCM_KONA */

static void __init hawaii_init(void)
{
	hawaii_add_devices();
#ifdef CONFIG_FB_BRCM_KONA
	konafb_init();
#endif
	hawaii_add_common_devices();

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

MACHINE_START(HAWAII, "hawaii_ss_logands")
	.atag_offset = 0x100,
	.map_io = hawaii_map_io,
	.init_irq = kona_init_irq,
	.handle_irq = gic_handle_irq,
	.timer = &kona_timer,
	.init_machine = hawaii_init,
#ifdef CONFIG_MOBICORE_OS
	.reserve = hawaii_mem_reserve,
#else
	.reserve = hawaii_reserve,
#endif
	.restart = hawaii_restart,
MACHINE_END
