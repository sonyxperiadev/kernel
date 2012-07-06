/* arch/arm/mach-msm/board-semc_fuji.c
 *
 * Copyright (C) 2012 Sony Ericsson Mobile Communications AB.
 * Copyright (C) 2012 Sony Mobile Communications AB.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/gpio_event.h>
#include <linux/irq.h>
#include <linux/io.h>
#include <linux/msm_ssbi.h>
#include <linux/mfd/pmic8058.h>
#include <linux/mfd/pmic8901.h>
#include <linux/regulator/pmic8901-regulator.h>
#include <linux/regulator/gpio-regulator.h>
#include <linux/skbuff.h>
#include <linux/wlan_plat.h>
#include <linux/leds.h>
#include <linux/pmic8058-othc.h>
#include <linux/bootmem.h>
#include <linux/msm_adc.h>
#include <linux/m_adcproc.h>
#include <linux/mfd/marimba.h>
#include <linux/msm-charger.h>
#include <linux/i2c.h>
#include <linux/spi/spi.h>
#include <linux/bma250_ng.h>
#include <linux/apds9702.h>
#include <linux/clearpad.h>
#include <linux/dma-mapping.h>
#include <linux/lm356x.h>
#ifdef CONFIG_INPUT_AKM8975
#include <linux/akm8975.h>
#endif
#ifdef CONFIG_INPUT_AKM8972
#include <linux/akm8972.h>
#endif
#ifdef CONFIG_SENSORS_MPU3050
#include <linux/mpu.h>
#include "gyro-semc_common.h"
#endif
#include <linux/i2c/bq27520_battery.h>
#include <linux/battery_chargalg.h>
#include <linux/serial_core.h>
#include <linux/console.h>
#ifdef CONFIG_CHARGER_BQ24185
#include <linux/i2c/bq24185_charger.h>
#endif
#ifdef CONFIG_CHARGER_BQ24160
#include <linux/i2c/bq24160_charger.h>
#endif
#include <mach/semc_battery_data.h>
#include <mach/msm72k_otg.h>
#include <mach/semc_charger_usb.h>
#ifdef CONFIG_SEMC_CHARGER_CRADLE_ARCH
#include <mach/semc_charger_cradle.h>
#endif

#ifdef CONFIG_ANDROID_PMEM
#include <linux/android_pmem.h>
#endif
#include <linux/reboot.h>

#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/setup.h>
#include <asm/hardware/gic.h>

#include <mach/dma.h>
#include <mach/mpp.h>
#include <mach/bcm_bt_lpm.h>
#include <mach/board.h>
#include <mach/irqs.h>
#include <mach/msm_spi.h>
#include <mach/msm_serial_hs.h>
#include <mach/msm_serial_hs_lite.h>
#include <mach/msm_iomap.h>
#include <mach/msm_memtypes.h>
#include <asm/mach/mmc.h>
#include <mach/msm_battery.h>
#include <mach/msm_hsusb.h>
#include <mach/gpiomux.h>
#ifdef CONFIG_MSM_DSPS
#include <mach/msm_dsps.h>
#endif
#include <mach/msm_xo.h>
#include <mach/msm_bus_board.h>
#ifdef CONFIG_FB_MSM_MIPI_DSI_RENESAS_R63306
#include <mach/mipi_dsi_renesas.h>
#endif /* CONFIG_FB_MSM_MIPI_DSI_RENESAS_R63306 */
#include <mach/socinfo.h>
#include <mach/scm-io.h>
#include <linux/i2c/isl9519.h>
#ifdef CONFIG_USB_G_ANDROID
#include <linux/usb/android.h>
#include <mach/usbdiag.h>
#endif
#include <linux/regulator/consumer.h>
#include <linux/regulator/machine.h>
#include <mach/sdio_al.h>
#include <mach/rpm.h>
#include <mach/rpm-regulator.h>
#include <mach/restart.h>
#include <mach/board-msm8660.h>

#include <mach/simple_remote_msm8x60_pf.h>

#include "devices.h"
#include "devices-msm8x60.h"
#include <mach/cpuidle.h>
#include "pm.h"
#include "mpm.h"
#include "spm.h"
#include "rpm_log.h"
#include "timer.h"
#include "gpiomux-semc_fuji.h"
#include "rpm_stats.h"
#include "peripheral-loader.h"
#include <linux/platform_data/qcom_crypto_device.h>
#include "rpm_resources.h"
#include "acpuclock.h"
#include "pm-boot.h"

#include <linux/ion.h>
#include <mach/ion.h>

#ifdef CONFIG_MSM_GSBI7_UART
#include <linux/tty.h>
#endif

#ifdef CONFIG_SEMC_ONESEG_TUNER_PM
#include "oneseg-fuji_common.h"
#endif
#include <linux/leds-as3676_semc.h>
#include "leds-semc-fuji.h"
#ifdef CONFIG_TOUCHSCREEN_CLEARPAD
#include "touch-semc_fuji.h"
#endif
#include "keypad-semc_fuji.h"
#ifdef CONFIG_FUJI_PMIC_KEYPAD
#include "keypad-pmic-fuji.h"
#endif
#include "vreg-semc_fuji.h"
#include "gpiomux-fuji_pm8058.h"

#ifdef CONFIG_NFC_PN544
#include <linux/pn544.h>
#include "nfc-fuji.h"
#endif

#ifdef CONFIG_SEMC_FELICA_SUPPORT
#include "felica-fuji_common.h"
#endif

#ifdef CONFIG_PMIC8058_MIC_BIAS
#include <mach/pm8058-mic_bias.h>
#endif

#define MSM_SHARED_RAM_PHYS 0x40000000
#define BMA250_GPIO			(41)
#define AKM897X_GPIO		31
#define BMA250_DEFAULT_RATE	50
#ifdef CONFIG_INPUT_AKM8975
#define GPIO_FUNC_NAME "akm8975_drdy"
#endif
#ifdef CONFIG_INPUT_AKM8972
#define GPIO_FUNC_NAME "akm8972_drdy"
#endif

#define MDM2AP_SYNC 129

#define GPIO_ETHERNET_RESET_N_DRAGON	30
#define LCDC_SPI_GPIO_CLK				73
#define LCDC_SPI_GPIO_CS				72
#define LCDC_SPI_GPIO_MOSI				70
#define LCDC_AUO_PANEL_NAME				"lcdc_auo_wvga"
#define LCDC_SAMSUNG_OLED_PANEL_NAME	"lcdc_samsung_oled"
#define LCDC_SAMSUNG_WSVGA_PANEL_NAME	"lcdc_samsung_wsvga"
#define LCDC_SAMSUNG_SPI_DEVICE_NAME	"lcdc_samsung_ams367pe02"
#define LCDC_AUO_SPI_DEVICE_NAME		"lcdc_auo_nt35582"
#define LCDC_NT35582_PANEL_NAME			"lcdc_nt35582_wvga"

#define MIPI_CMD_NOVATEK_QHD_PANEL_NAME	"mipi_cmd_novatek_qhd"
#define MIPI_VIDEO_NOVATEK_QHD_PANEL_NAME	"mipi_video_novatek_qhd"
#define MIPI_VIDEO_TOSHIBA_WVGA_PANEL_NAME	"mipi_video_toshiba_wvga"
#define HDMI_PANEL_NAME	"hdmi_msm"
#define TVOUT_PANEL_NAME	"tvout_msm"

#define DSPS_PIL_GENERIC_NAME		"dsps"
#define DSPS_PIL_FLUID_NAME		"dsps_fluid"

#ifdef CONFIG_ION_MSM
static struct platform_device ion_dev;
#endif

struct pm8xxx_mpp_init_info {
	unsigned			mpp;
	struct pm8xxx_mpp_config_data	config;
};

#define PM8058_MPP_INIT(_mpp, _type, _level, _control) \
{ \
	.mpp	= PM8058_MPP_PM_TO_SYS(_mpp), \
	.config	= { \
		.type		= PM8XXX_MPP_TYPE_##_type, \
		.level		= _level, \
		.control	= PM8XXX_MPP_##_control, \
	} \
}

#define PM8901_MPP_INIT(_mpp, _type, _level, _control) \
{ \
	.mpp    = PM8901_MPP_PM_TO_SYS(_mpp), \
	.config = { \
		.type           = PM8XXX_MPP_TYPE_##_type, \
		.level          = _level, \
		.control        = PM8XXX_MPP_##_control, \
	} \
}

#define BT_GPIO_EN			(23)
#define BT_GPIO_WAKE			(137)
#define BT_GPIO_HOST_WAKE		(63)

#ifdef CONFIG_NFC_PN544
#define PMIC_GPIO_NFC_EN		(17)
#define PMIC_GPIO_NFC_FWDL_EN		(27)
#define PMIC_GPIO_NFC_IRQ		(28)
#endif

#ifdef CONFIG_SENSORS_MPU3050
#define MPU3050_GPIO			(30)
#endif

#define PREALLOC_WLAN_NUMBER_OF_SECTIONS	4
#define PREALLOC_WLAN_NUMBER_OF_BUFFERS		160
#define PREALLOC_WLAN_SECTION_HEADER		24

#define WLAN_SECTION_SIZE_0	(PREALLOC_WLAN_NUMBER_OF_BUFFERS * 128)
#define WLAN_SECTION_SIZE_1	(PREALLOC_WLAN_NUMBER_OF_BUFFERS * 256)
#define WLAN_SECTION_SIZE_2	(PREALLOC_WLAN_NUMBER_OF_BUFFERS * 512)
#define WLAN_SECTION_SIZE_3	(PREALLOC_WLAN_NUMBER_OF_BUFFERS * 1024)

#define WLAN_SKB_BUF_NUM	16

static struct sk_buff *wlan_static_skb[WLAN_SKB_BUF_NUM];

typedef struct wifi_mem_prealloc_struct {
	void *mem_ptr;
	unsigned long size;
} wifi_mem_prealloc_t;

static wifi_mem_prealloc_t wifi_mem_array[PREALLOC_WLAN_NUMBER_OF_SECTIONS] = {
	{ NULL, (WLAN_SECTION_SIZE_0 + PREALLOC_WLAN_SECTION_HEADER) },
	{ NULL, (WLAN_SECTION_SIZE_1 + PREALLOC_WLAN_SECTION_HEADER) },
	{ NULL, (WLAN_SECTION_SIZE_2 + PREALLOC_WLAN_SECTION_HEADER) },
	{ NULL, (WLAN_SECTION_SIZE_3 + PREALLOC_WLAN_SECTION_HEADER) }
};


#ifdef CONFIG_MMC_MSM_SDC2_SUPPORT
static void (*sdc2_status_notify_cb)(int card_present, void *dev_id);
static void *sdc2_status_notify_cb_devid;
#endif

#ifdef CONFIG_MMC_MSM_SDC5_SUPPORT
static void (*sdc5_status_notify_cb)(int card_present, void *dev_id);
static void *sdc5_status_notify_cb_devid;
#endif

/* Platform specific HW-ID GPIO mask */
static const u8 hw_id_gpios[] = {11, 15, 12, 86};

static struct msm_spm_platform_data msm_spm_data_v1[] __initdata = {
	[0] = {
		.reg_base_addr = MSM_SAW0_BASE,

#ifdef CONFIG_MSM_AVS_HW
		.reg_init_values[MSM_SPM_REG_SAW_AVS_CTL] = 0x586020FF,
#endif
		.reg_init_values[MSM_SPM_REG_SAW_CFG] = 0x0F,
		.reg_init_values[MSM_SPM_REG_SAW_SPM_CTL] = 0x68,
		.reg_init_values[MSM_SPM_REG_SAW_SPM_SLP_TMR_DLY] = 0xFFFFFFFF,
		.reg_init_values[MSM_SPM_REG_SAW_SPM_WAKE_TMR_DLY] = 0xFFFFFFFF,

		.reg_init_values[MSM_SPM_REG_SAW_SLP_CLK_EN] = 0x01,
		.reg_init_values[MSM_SPM_REG_SAW_SLP_HSFS_PRECLMP_EN] = 0x07,
		.reg_init_values[MSM_SPM_REG_SAW_SLP_HSFS_POSTCLMP_EN] = 0x00,

		.reg_init_values[MSM_SPM_REG_SAW_SLP_CLMP_EN] = 0x01,
		.reg_init_values[MSM_SPM_REG_SAW_SLP_RST_EN] = 0x00,
		.reg_init_values[MSM_SPM_REG_SAW_SPM_MPM_CFG] = 0x00,

		.awake_vlevel = 0x94,
		.retention_vlevel = 0x81,
		.collapse_vlevel = 0x20,
		.retention_mid_vlevel = 0x94,
		.collapse_mid_vlevel = 0x8C,

		.vctl_timeout_us = 50,
	},

	[1] = {
		.reg_base_addr = MSM_SAW1_BASE,

#ifdef CONFIG_MSM_AVS_HW
		.reg_init_values[MSM_SPM_REG_SAW_AVS_CTL] = 0x586020FF,
#endif
		.reg_init_values[MSM_SPM_REG_SAW_CFG] = 0x0F,
		.reg_init_values[MSM_SPM_REG_SAW_SPM_CTL] = 0x68,
		.reg_init_values[MSM_SPM_REG_SAW_SPM_SLP_TMR_DLY] = 0xFFFFFFFF,
		.reg_init_values[MSM_SPM_REG_SAW_SPM_WAKE_TMR_DLY] = 0xFFFFFFFF,

		.reg_init_values[MSM_SPM_REG_SAW_SLP_CLK_EN] = 0x13,
		.reg_init_values[MSM_SPM_REG_SAW_SLP_HSFS_PRECLMP_EN] = 0x07,
		.reg_init_values[MSM_SPM_REG_SAW_SLP_HSFS_POSTCLMP_EN] = 0x00,

		.reg_init_values[MSM_SPM_REG_SAW_SLP_CLMP_EN] = 0x01,
		.reg_init_values[MSM_SPM_REG_SAW_SLP_RST_EN] = 0x00,
		.reg_init_values[MSM_SPM_REG_SAW_SPM_MPM_CFG] = 0x00,

		.awake_vlevel = 0x94,
		.retention_vlevel = 0x81,
		.collapse_vlevel = 0x20,
		.retention_mid_vlevel = 0x94,
		.collapse_mid_vlevel = 0x8C,

		.vctl_timeout_us = 50,
	},
};

static struct msm_spm_platform_data msm_spm_data[] __initdata = {
	[0] = {
		.reg_base_addr = MSM_SAW0_BASE,

#ifdef CONFIG_MSM_AVS_HW
		.reg_init_values[MSM_SPM_REG_SAW_AVS_CTL] = 0x586020FF,
#endif
		.reg_init_values[MSM_SPM_REG_SAW_CFG] = 0x1C,
		.reg_init_values[MSM_SPM_REG_SAW_SPM_CTL] = 0x68,
		.reg_init_values[MSM_SPM_REG_SAW_SPM_SLP_TMR_DLY] = 0x0C0CFFFF,
		.reg_init_values[MSM_SPM_REG_SAW_SPM_WAKE_TMR_DLY] = 0x78780FFF,

		.reg_init_values[MSM_SPM_REG_SAW_SLP_CLK_EN] = 0x01,
		.reg_init_values[MSM_SPM_REG_SAW_SLP_HSFS_PRECLMP_EN] = 0x07,
		.reg_init_values[MSM_SPM_REG_SAW_SLP_HSFS_POSTCLMP_EN] = 0x00,

		.reg_init_values[MSM_SPM_REG_SAW_SLP_CLMP_EN] = 0x01,
		.reg_init_values[MSM_SPM_REG_SAW_SLP_RST_EN] = 0x00,
		.reg_init_values[MSM_SPM_REG_SAW_SPM_MPM_CFG] = 0x00,

		.awake_vlevel = 0xA0,
		.retention_vlevel = 0x89,
		.collapse_vlevel = 0x20,
		.retention_mid_vlevel = 0x89,
		.collapse_mid_vlevel = 0x89,

		.vctl_timeout_us = 50,
	},

	[1] = {
		.reg_base_addr = MSM_SAW1_BASE,

#ifdef CONFIG_MSM_AVS_HW
		.reg_init_values[MSM_SPM_REG_SAW_AVS_CTL] = 0x586020FF,
#endif
		.reg_init_values[MSM_SPM_REG_SAW_CFG] = 0x1C,
		.reg_init_values[MSM_SPM_REG_SAW_SPM_CTL] = 0x68,
		.reg_init_values[MSM_SPM_REG_SAW_SPM_SLP_TMR_DLY] = 0x0C0CFFFF,
		.reg_init_values[MSM_SPM_REG_SAW_SPM_WAKE_TMR_DLY] = 0x78780FFF,

		.reg_init_values[MSM_SPM_REG_SAW_SLP_CLK_EN] = 0x13,
		.reg_init_values[MSM_SPM_REG_SAW_SLP_HSFS_PRECLMP_EN] = 0x07,
		.reg_init_values[MSM_SPM_REG_SAW_SLP_HSFS_POSTCLMP_EN] = 0x00,

		.reg_init_values[MSM_SPM_REG_SAW_SLP_CLMP_EN] = 0x01,
		.reg_init_values[MSM_SPM_REG_SAW_SLP_RST_EN] = 0x00,
		.reg_init_values[MSM_SPM_REG_SAW_SPM_MPM_CFG] = 0x00,

		.awake_vlevel = 0xA0,
		.retention_vlevel = 0x89,
		.collapse_vlevel = 0x20,
		.retention_mid_vlevel = 0x89,
		.collapse_mid_vlevel = 0x89,

		.vctl_timeout_us = 50,
	},
};

/*
 * Consumer specific regulator names:
 *			 regulator name		consumer dev_name
 */
static struct regulator_consumer_supply vreg_consumers_8901_S0[] = {
	REGULATOR_SUPPLY("8901_s0",		NULL),
};
static struct regulator_consumer_supply vreg_consumers_8901_S1[] = {
	REGULATOR_SUPPLY("8901_s1",		NULL),
};

static struct regulator_init_data saw_s0_init_data = {
		.constraints = {
			.name = "8901_s0",
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE,
			.min_uV = 800000,
			.max_uV = 1325000,
		},
		.consumer_supplies = vreg_consumers_8901_S0,
		.num_consumer_supplies = ARRAY_SIZE(vreg_consumers_8901_S0),
};

static struct regulator_init_data saw_s1_init_data = {
		.constraints = {
			.name = "8901_s1",
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE,
			.min_uV = 800000,
			.max_uV = 1325000,
		},
		.consumer_supplies = vreg_consumers_8901_S1,
		.num_consumer_supplies = ARRAY_SIZE(vreg_consumers_8901_S1),
};

static struct platform_device msm_device_saw_s0 = {
	.name          = "saw-regulator",
	.id            = 0,
	.dev           = {
		.platform_data = &saw_s0_init_data,
	},
};

static struct platform_device msm_device_saw_s1 = {
	.name          = "saw-regulator",
	.id            = 1,
	.dev           = {
		.platform_data = &saw_s1_init_data,
	},
};

#if defined(CONFIG_CRYPTO_DEV_QCRYPTO) || \
		defined(CONFIG_CRYPTO_DEV_QCRYPTO_MODULE) || \
		defined(CONFIG_CRYPTO_DEV_QCEDEV) || \
		defined(CONFIG_CRYPTO_DEV_QCEDEV_MODULE)

#define QCE_SIZE		0x10000
#define QCE_0_BASE		0x18500000

#define QCE_HW_KEY_SUPPORT	0
#define QCE_SHA_HMAC_SUPPORT	0
#define QCE_SHARE_CE_RESOURCE	2
#define QCE_CE_SHARED		1

static struct resource qcrypto_resources[] = {
	[0] = {
		.start = QCE_0_BASE,
		.end = QCE_0_BASE + QCE_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.name = "crypto_channels",
		.start = DMOV_CE_IN_CHAN,
		.end = DMOV_CE_OUT_CHAN,
		.flags = IORESOURCE_DMA,
	},
	[2] = {
		.name = "crypto_crci_in",
		.start = DMOV_CE_IN_CRCI,
		.end = DMOV_CE_IN_CRCI,
		.flags = IORESOURCE_DMA,
	},
	[3] = {
		.name = "crypto_crci_out",
		.start = DMOV_CE_OUT_CRCI,
		.end = DMOV_CE_OUT_CRCI,
		.flags = IORESOURCE_DMA,
	},
	[4] = {
		.name = "crypto_crci_hash",
		.start = DMOV_CE_HASH_CRCI,
		.end = DMOV_CE_HASH_CRCI,
		.flags = IORESOURCE_DMA,
	},
};

static struct resource qcedev_resources[] = {
	[0] = {
		.start = QCE_0_BASE,
		.end = QCE_0_BASE + QCE_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.name = "crypto_channels",
		.start = DMOV_CE_IN_CHAN,
		.end = DMOV_CE_OUT_CHAN,
		.flags = IORESOURCE_DMA,
	},
	[2] = {
		.name = "crypto_crci_in",
		.start = DMOV_CE_IN_CRCI,
		.end = DMOV_CE_IN_CRCI,
		.flags = IORESOURCE_DMA,
	},
	[3] = {
		.name = "crypto_crci_out",
		.start = DMOV_CE_OUT_CRCI,
		.end = DMOV_CE_OUT_CRCI,
		.flags = IORESOURCE_DMA,
	},
	[4] = {
		.name = "crypto_crci_hash",
		.start = DMOV_CE_HASH_CRCI,
		.end = DMOV_CE_HASH_CRCI,
		.flags = IORESOURCE_DMA,
	},
};

#endif

#if defined(CONFIG_CRYPTO_DEV_QCRYPTO) || \
		defined(CONFIG_CRYPTO_DEV_QCRYPTO_MODULE)

static struct msm_ce_hw_support qcrypto_ce_hw_suppport = {
	.ce_shared = QCE_CE_SHARED,
	.shared_ce_resource = QCE_SHARE_CE_RESOURCE,
	.hw_key_support = QCE_HW_KEY_SUPPORT,
	.sha_hmac = QCE_SHA_HMAC_SUPPORT,
	.bus_scale_table = NULL,
};

static struct platform_device qcrypto_device = {
	.name		= "qcrypto",
	.id		= 0,
	.num_resources	= ARRAY_SIZE(qcrypto_resources),
	.resource	= qcrypto_resources,
	.dev		= {
		.coherent_dma_mask = DMA_BIT_MASK(32),
		.platform_data = &qcrypto_ce_hw_suppport,
	},
};
#endif

#if defined(CONFIG_CRYPTO_DEV_QCEDEV) || \
		defined(CONFIG_CRYPTO_DEV_QCEDEV_MODULE)

static struct msm_ce_hw_support qcedev_ce_hw_suppport = {
	.ce_shared = QCE_CE_SHARED,
	.shared_ce_resource = QCE_SHARE_CE_RESOURCE,
	.hw_key_support = QCE_HW_KEY_SUPPORT,
	.sha_hmac = QCE_SHA_HMAC_SUPPORT,
	.bus_scale_table = NULL,
};

static struct platform_device qcedev_device = {
	.name		= "qce",
	.id		= 0,
	.num_resources	= ARRAY_SIZE(qcedev_resources),
	.resource	= qcedev_resources,
	.dev		= {
		.coherent_dma_mask = DMA_BIT_MASK(32),
		.platform_data = &qcedev_ce_hw_suppport,
	},
};
#endif

static struct msm_pm_platform_data msm_pm_data[MSM_PM_SLEEP_MODE_NR * 2] = {
	[MSM_PM_MODE(0, MSM_PM_SLEEP_MODE_POWER_COLLAPSE)] = {
		.idle_supported = 1,
		.suspend_supported = 1,
		.idle_enabled = 0,
		.suspend_enabled = 0,
	},

	[MSM_PM_MODE(0, MSM_PM_SLEEP_MODE_POWER_COLLAPSE_STANDALONE)] = {
		.idle_supported = 1,
		.suspend_supported = 1,
		.idle_enabled = 0,
		.suspend_enabled = 0,
	},

	[MSM_PM_MODE(0, MSM_PM_SLEEP_MODE_WAIT_FOR_INTERRUPT)] = {
		.idle_supported = 1,
		.suspend_supported = 1,
		.idle_enabled = 1,
		.suspend_enabled = 1,
	},

	[MSM_PM_MODE(1, MSM_PM_SLEEP_MODE_POWER_COLLAPSE)] = {
		.idle_supported = 1,
		.suspend_supported = 1,
		.idle_enabled = 0,
		.suspend_enabled = 0,
	},

	[MSM_PM_MODE(1, MSM_PM_SLEEP_MODE_POWER_COLLAPSE_STANDALONE)] = {
		.idle_supported = 1,
		.suspend_supported = 1,
		.idle_enabled = 0,
		.suspend_enabled = 0,
	},

	[MSM_PM_MODE(1, MSM_PM_SLEEP_MODE_WAIT_FOR_INTERRUPT)] = {
		.idle_supported = 1,
		.suspend_supported = 1,
		.idle_enabled = 1,
		.suspend_enabled = 1,
	},
};

static struct msm_cpuidle_state msm_cstates[] __initdata = {
	{0, 0, "C0", "WFI",
		MSM_PM_SLEEP_MODE_WAIT_FOR_INTERRUPT},

	{0, 1, "C1", "STANDALONE_POWER_COLLAPSE",
		MSM_PM_SLEEP_MODE_POWER_COLLAPSE_STANDALONE},

	{0, 2, "C2", "POWER_COLLAPSE",
		MSM_PM_SLEEP_MODE_POWER_COLLAPSE},

	{1, 0, "C0", "WFI",
		MSM_PM_SLEEP_MODE_WAIT_FOR_INTERRUPT},

	{1, 1, "C1", "STANDALONE_POWER_COLLAPSE",
		MSM_PM_SLEEP_MODE_POWER_COLLAPSE_STANDALONE},
};

#define LM356X_HW_RESET_GPIO 138
int lm356x_request_gpio_pins(void)
{
	int result;

	result = gpio_request(LM356X_HW_RESET_GPIO, "LM356X hw reset");
	if (result)
		return result;

	gpio_set_value(LM356X_HW_RESET_GPIO, 1);

	udelay(20);
	return result;
}

int lm356x_release_gpio_pins(void)
{
	gpio_set_value(LM356X_HW_RESET_GPIO, 0);
	gpio_free(LM356X_HW_RESET_GPIO);

	return 0;
}

static struct lm356x_platform_data lm3560_platform_data = {
	.hw_enable		= lm356x_request_gpio_pins,
	.hw_disable		= lm356x_release_gpio_pins,
	.led_nums		= 2,
	.strobe_trigger		= LM356X_STROBE_TRIGGER_EDGE,
	.privacy_terminate	= LM356X_PRIVACY_MODE_TURN_BACK,
	.privacy_led_nums	= 1,
	.privacy_blink_period	= 0, /* No bliking */
	.current_limit		= 2300000, /* uA */
	.flash_sync		= LM356X_SYNC_OFF,
	.strobe_polarity	= LM356X_STROBE_POLARITY_HIGH,
	.ledintc_pin_setting	= LM356X_LEDINTC_NTC_THERMISTOR_INPUT,
	.tx1_polarity		= LM356X_TX1_POLARITY_HIGH,
	.tx2_polarity		= LM356X_TX2_POLARITY_HIGH,
	.hw_torch_mode		= LM356X_HW_TORCH_MODE_DISABLE,
};


static struct msm_rpmrs_level msm_rpmrs_levels[] __initdata = {
	{
		MSM_PM_SLEEP_MODE_WAIT_FOR_INTERRUPT,
		MSM_RPMRS_LIMITS(ON, ACTIVE, MAX, ACTIVE),
		true,
		1, 8000, 100000, 1,
	},

	{
		MSM_PM_SLEEP_MODE_POWER_COLLAPSE_STANDALONE,
		MSM_RPMRS_LIMITS(ON, ACTIVE, MAX, ACTIVE),
		true,
		1500, 5000, 60100000, 3000,
	},

	{
		MSM_PM_SLEEP_MODE_POWER_COLLAPSE,
		MSM_RPMRS_LIMITS(ON, ACTIVE, MAX, ACTIVE),
		false,
		1800, 5000, 60350000, 3500,
	},
	{
		MSM_PM_SLEEP_MODE_POWER_COLLAPSE,
		MSM_RPMRS_LIMITS(OFF, ACTIVE, MAX, ACTIVE),
		false,
		3800, 4500, 65350000, 5500,
	},

	{
		MSM_PM_SLEEP_MODE_POWER_COLLAPSE,
		MSM_RPMRS_LIMITS(ON, HSFS_OPEN, MAX, ACTIVE),
		false,
		2800, 2500, 66850000, 4800,
	},

	{
		MSM_PM_SLEEP_MODE_POWER_COLLAPSE,
		MSM_RPMRS_LIMITS(OFF, HSFS_OPEN, MAX, ACTIVE),
		false,
		4800, 2000, 71850000, 6800,
	},

	{
		MSM_PM_SLEEP_MODE_POWER_COLLAPSE,
		MSM_RPMRS_LIMITS(OFF, HSFS_OPEN, ACTIVE, RET_HIGH),
		false,
		6800, 500, 75850000, 8800,
	},

	{
		MSM_PM_SLEEP_MODE_POWER_COLLAPSE,
		MSM_RPMRS_LIMITS(OFF, HSFS_OPEN, RET_HIGH, RET_LOW),
		false,
		7800, 0, 76350000, 9800,
	},
};

static struct msm_pm_boot_platform_data msm_pm_boot_pdata __initdata = {
	.mode = MSM_PM_BOOT_CONFIG_TZ,
};

#if defined(CONFIG_USB_PEHCI_HCD) || defined(CONFIG_USB_PEHCI_HCD_MODULE)

#define ISP1763_INT_GPIO		117
#define ISP1763_RST_GPIO		152
static struct resource isp1763_resources[] = {
	[0] = {
		.flags	= IORESOURCE_MEM,
		.start	= 0x1D000000,
		.end	= 0x1D005FFF,		/* 24KB */
	},
	[1] = {
		.flags	= IORESOURCE_IRQ,
	},
};
static void __init msm8x60_cfg_isp1763(void)
{
	isp1763_resources[1].start = gpio_to_irq(ISP1763_INT_GPIO);
	isp1763_resources[1].end = gpio_to_irq(ISP1763_INT_GPIO);
}

static int isp1763_setup_gpio(int enable)
{
	int status = 0;

	if (enable) {
		status = gpio_request(ISP1763_INT_GPIO, "isp1763_usb");
		if (status) {
			pr_err("%s:Failed to request GPIO %d\n",
						__func__, ISP1763_INT_GPIO);
			return status;
		}
		status = gpio_direction_input(ISP1763_INT_GPIO);
		if (status) {
			pr_err("%s:Failed to configure GPIO %d\n",
					__func__, ISP1763_INT_GPIO);
			goto gpio_free_int;
		}
		status = gpio_request(ISP1763_RST_GPIO, "isp1763_usb");
		if (status) {
			pr_err("%s:Failed to request GPIO %d\n",
						__func__, ISP1763_RST_GPIO);
			goto gpio_free_int;
		}
		status = gpio_direction_output(ISP1763_RST_GPIO, 1);
		if (status) {
			pr_err("%s:Failed to configure GPIO %d\n",
					__func__, ISP1763_RST_GPIO);
			goto gpio_free_rst;
		}
		pr_debug("\nISP GPIO configuration done\n");
		return status;
	}

gpio_free_rst:
	gpio_free(ISP1763_RST_GPIO);
gpio_free_int:
	gpio_free(ISP1763_INT_GPIO);

	return status;
}
static struct isp1763_platform_data isp1763_pdata = {
	.reset_gpio	= ISP1763_RST_GPIO,
	.setup_gpio	= isp1763_setup_gpio
};

static struct platform_device isp1763_device = {
	.name          = "isp1763_usb",
	.num_resources = ARRAY_SIZE(isp1763_resources),
	.resource      = isp1763_resources,
	.dev           = {
		.platform_data = &isp1763_pdata
	}
};
#endif

#ifdef CONFIG_USB_NCP373
#include <linux/usb/ncp373.h>
#define GPIO_USB_OTG_EN			28
#define GPIO_USB_OTG_OVERCUR_INT	104
#define REGULATOR_NCP373_IN		"8901_mpp0"

struct ncp373_res_hdl {
	int en;
	struct regulator *in;
	int flg;
};
static struct ncp373_res_hdl ncp373_hdl;

static int ncp373_gpio_request(int gpio, char *name)
{
	int ret;

	ret = gpio_request(gpio, name);

	if (unlikely(ret < 0)) {
		pr_err("%s: failed to request gpio=%d name=%s\n", __func__,
								gpio, name);
		ret = 0;
	} else {
		pr_debug("%s: got gpio gpio=%d name=%s\n", __func__,
								gpio, name);
		ret = gpio;
	}

	return ret;
}

static struct regulator *ncp373_regulator_request(char *name)
{
	struct regulator *ret;

	ret = regulator_get(NULL, name);

	if (IS_ERR(ret)) {
		pr_err("%s: failed to get regulator name=\"%s\" ret=%d\n",
						__func__, name, (int)ret);
		ret = NULL;
	} else {
		pr_debug("%s: got regulator name=\"%s\"\n", __func__, name);
	}

	return ret;
}

static int ncp373_en_request(void)
{
	return ncp373_gpio_request(GPIO_USB_OTG_EN, "ncp373_en");
}

static struct regulator *ncp373_in_request(void)
{
	return ncp373_regulator_request(REGULATOR_NCP373_IN);
}

static int ncp373_flg_request(void)
{
	return ncp373_gpio_request(GPIO_USB_OTG_OVERCUR_INT, "ncp373_flg");
}

static int ncp373_probe(struct platform_device *pdev)
{
	/* It may not be got a resource here,
	 * due to the timeliness of the device initialization.
	*/
	if (likely(!ncp373_hdl.en))
		ncp373_hdl.en = ncp373_en_request();

	if (likely(!ncp373_hdl.in))
		ncp373_hdl.in = ncp373_in_request();

	if (likely(!ncp373_hdl.flg))
		ncp373_hdl.flg = ncp373_flg_request();

	return 0;
}

static int ncp373_en_set(int on)
{
	int ret = -EIO;

	if (unlikely(!ncp373_hdl.en))
		ncp373_hdl.en = ncp373_en_request();

	if (likely(ncp373_hdl.en)) {
		gpio_set_value(ncp373_hdl.en, !!on);
		ret = 0;
	}

	return ret;
}

static int ncp373_in_set(int on)
{
	int ret = -EIO;

	if (unlikely(!ncp373_hdl.in))
		ncp373_hdl.in = ncp373_in_request();

	if (likely(ncp373_hdl.in)) {
		if (on)
			ret = regulator_enable(ncp373_hdl.in);
		else
			ret = regulator_disable(ncp373_hdl.in);

		if (unlikely(ret < 0))
			pr_err("%s: failed to switch %s regulator %s ret=%d\n",
						__func__, on ? "on " : "off",
						REGULATOR_NCP373_IN, ret);
	}

	return ret;
}

static int ncp373_flg_get(void)
{
	int ret = -EIO;

	if (unlikely(!ncp373_hdl.flg))
		ncp373_hdl.flg = ncp373_flg_request();

	if (likely(ncp373_hdl.flg)) {
		ret = gpio_get_value(ncp373_hdl.flg);
		if (unlikely(ret < 0))
			pr_err("%s: failed to read GPIO=%d ret=%d\n",
						__func__, ncp373_hdl.flg, ret);
	}

	return ret;
}

static void ncp373_remove(void)
{
	if (likely(ncp373_hdl.en))
		gpio_free(ncp373_hdl.en);

	if (likely(ncp373_hdl.in))
		regulator_put(ncp373_hdl.in);

	if (likely(ncp373_hdl.flg))
		gpio_free(ncp373_hdl.flg);

	ncp373_hdl.en = 0;
	ncp373_hdl.in = NULL;
	ncp373_hdl.flg = 0;
}

static void ncp373_notify_flg_int(void)
{
	pr_info("%s: received over current notify\n", __func__);
	msm_otg_notify_vbus_drop();
}

static void ncp373_check_pin_state(void)
{
	int en = -1;
	int in = -1;
	int flg = -1;

	if (likely(ncp373_hdl.en))
		en = gpio_get_value(ncp373_hdl.en);

	if (likely(ncp373_hdl.in))
		in = regulator_is_enabled(ncp373_hdl.in);

	if (likely(ncp373_hdl.flg))
		flg = gpio_get_value(ncp373_hdl.flg);

	pr_debug("%s: EN=%d, IN=%d, FLG=%d\n", __func__, en, in, flg);
}

struct ncp373_platform_data ncp373_pdata = {
	.probe		= ncp373_probe,
	.remove		= ncp373_remove,
	.en_set		= ncp373_en_set,
	.in_set		= ncp373_in_set,
	.flg_get	= ncp373_flg_get,
	.notify_flg_int	= ncp373_notify_flg_int,
	.check_pin_state = ncp373_check_pin_state,
	.oc_delay_time	= 300000,
};

static struct resource ncp373_resources[] = {
	/* OTG_OVERCUR_INT */
	{
		.start	= MSM_GPIO_TO_INT(GPIO_USB_OTG_OVERCUR_INT),
		.end	= MSM_GPIO_TO_INT(GPIO_USB_OTG_OVERCUR_INT),
		.flags	= IORESOURCE_IRQ,
	},
};

struct platform_device ncp373_device = {
	.name		= NCP373_DRIVER_NAME,
	.id		= -1,
	.num_resources	= ARRAY_SIZE(ncp373_resources),
	.resource	= ncp373_resources,
	.dev		= {
		.platform_data = &ncp373_pdata,
	},
};
#endif

#if defined(CONFIG_USB_GADGET_MSM_72K) || defined(CONFIG_USB_EHCI_MSM_72K)
static struct msm_otg_platform_data msm_otg_pdata;
static struct regulator *ldo6_3p3;
static struct regulator *ldo7_1p8;
static struct regulator *vdd_cx;

static int usb_phy_susp_dig_vol = 750000;

#ifdef CONFIG_USB_EHCI_MSM_72K
#define PMIC_USB_HS_ID_DET	30
#define PMICID_INT		PM8058_GPIO_IRQ(PM8058_IRQ_BASE,\
							PMIC_USB_HS_ID_DET)
#define PMICID_GPIO		PM8058_GPIO_PM_TO_SYS(PMIC_USB_HS_ID_DET)
#define USB_PMIC_ID_DET_DELAY	msecs_to_jiffies(100)
struct delayed_work pmic_id_det;
notify_vbus_state notify_id_state_func_ptr;

static void pmic_id_detect(struct work_struct *w)
{
	int val = gpio_get_value_cansleep(PMICID_GPIO);
	pr_debug("%s(): gpio_read_value = %d\n", __func__, val);

	if (notify_id_state_func_ptr)
		(*notify_id_state_func_ptr) (val);
}

static irqreturn_t pmic_id_on_irq(int irq, void *data)
{
	/*
	 * Spurious interrupts are observed on pmic gpio line
	 * even though there is no state change on USB ID. Schedule the
	 * work to to allow debounce on gpio
	 */
	schedule_delayed_work(&pmic_id_det, USB_PMIC_ID_DET_DELAY);

	return IRQ_HANDLED;
}

static int msm_hsusb_pmic_id_notif_init(void (*callback)(int online), int init)
{
	unsigned ret = -ENODEV;

	struct pm_gpio pmic_id_cfg = {
		.direction	= PM_GPIO_DIR_IN,
		.pull		= PM_GPIO_PULL_UP_1P5,
		.function	= PM_GPIO_FUNC_NORMAL,
		.vin_sel	= 2,
		.inv_int_pol	= 0,
	};
	struct pm_gpio pmic_id_uncfg = {
		.direction	= PM_GPIO_DIR_IN,
		.pull		= PM_GPIO_PULL_NO,
		.function	= PM_GPIO_FUNC_NORMAL,
		.vin_sel	= 2,
		.inv_int_pol	= 0,
	};
	if (!callback)
		return -EINVAL;

	usb_phy_susp_dig_vol = 500000;

	if (init) {
		notify_id_state_func_ptr = callback;
		INIT_DELAYED_WORK(&pmic_id_det, pmic_id_detect);
		ret = pm8xxx_gpio_config(PMICID_GPIO, &pmic_id_cfg);
		if (ret) {
			pr_err("%s:return val of pm8xxx_gpio_config: %d\n",
						__func__,  ret);
			return ret;
		}
		ret = request_threaded_irq(PMICID_INT, NULL, pmic_id_on_irq,
			(IRQF_TRIGGER_RISING|IRQF_TRIGGER_FALLING),
						"msm_otg_id", NULL);
		if (ret) {
			pr_err("%s:pmic_usb_id interrupt registration failed",
					__func__);
			return ret;
		}
		msm_otg_pdata.pmic_id_irq = PMICID_INT;
	} else {
		usb_phy_susp_dig_vol = 750000;
		free_irq(PMICID_INT, 0);
		ret = pm8xxx_gpio_config(PMICID_GPIO, &pmic_id_uncfg);
		if (ret) {
			pr_err("%s: return val of pm8xxx_gpio_config: %d\n",
						__func__,  ret);
			return ret;
		}
		msm_otg_pdata.pmic_id_irq = 0;
		cancel_delayed_work_sync(&pmic_id_det);
		notify_id_state_func_ptr = NULL;
	}
	return 0;
}
#endif

#define PMIC_CBLPWR0_N		10
#define PMICVBUS_INT		PM8058_MPP_IRQ(PM8058_IRQ_BASE, PMIC_CBLPWR0_N)
#define PMICVBUS_GPIO		PM8058_MPP_PM_TO_SYS(PMIC_CBLPWR0_N)
#define USB_PMIC_VBUS_DET_DELAY	msecs_to_jiffies(100)
struct delayed_work pmic_vbus_det;
notify_vbus_state notify_vbus_state_func_ptr;

static void pmic_vbus_detect(struct work_struct *w)
{
	int val = !gpio_get_value_cansleep(PMICVBUS_GPIO);
	pr_debug("%s(): gpio_read_value = %d\n", __func__, val);

	if (notify_vbus_state_func_ptr)
		(*notify_vbus_state_func_ptr) (val);
}

static irqreturn_t pmic_vbus_on_irq(int irq, void *data)
{
	schedule_delayed_work(&pmic_vbus_det, USB_PMIC_VBUS_DET_DELAY);

	return IRQ_HANDLED;
}

static int msm_hsusb_pmic_vbus_notif_init(void (*callback)(int online),
								int init)
{
	unsigned ret = -ENODEV;
	struct pm8xxx_mpp_config_data pmic_vbus_cfg = {
		.type		= PM8XXX_MPP_TYPE_D_INPUT,
		.level		= PM8058_MPP_DIG_LEVEL_S3,
		.control	= PM8XXX_MPP_DIN_TO_INT,
	};

	if (!callback)
		return -EINVAL;

	if (init) {
		notify_vbus_state_func_ptr = callback;
		ret = pm8xxx_mpp_config(PMICVBUS_GPIO, &pmic_vbus_cfg);
		if (ret) {
			pr_err("%s:return val of pm8xxx_mpp_config: %d\n",
						__func__,  ret);
			return ret;
		}
		INIT_DELAYED_WORK(&pmic_vbus_det, pmic_vbus_detect);
		ret = request_threaded_irq(PMICVBUS_INT, NULL, pmic_vbus_on_irq,
			(IRQF_TRIGGER_RISING|IRQF_TRIGGER_FALLING),
						"msm_otg_vbus", NULL);
		if (ret) {
			pr_err("%s:pmic_usb_vbus interrupt registration failed",
					__func__);
			return ret;
		}
		msm_otg_pdata.pmic_vbus_irq = PMICVBUS_INT;
	} else {
		free_irq(PMICVBUS_INT, 0);
		msm_otg_pdata.pmic_vbus_irq = 0;
		cancel_delayed_work_sync(&pmic_vbus_det);
		notify_vbus_state_func_ptr = NULL;
	}
	return 0;
}

#define USB_PHY_OPERATIONAL_MIN_VDD_DIG_VOL	1000000
#define USB_PHY_MAX_VDD_DIG_VOL			1320000
static int msm_hsusb_init_vddcx(int init)
{
	int ret = 0;

	if (init) {
		vdd_cx = regulator_get(NULL, "8058_s1");
		if (IS_ERR(vdd_cx))
			return PTR_ERR(vdd_cx);

		ret = regulator_set_voltage(vdd_cx,
				USB_PHY_OPERATIONAL_MIN_VDD_DIG_VOL,
				USB_PHY_MAX_VDD_DIG_VOL);
		if (ret) {
			pr_err("%s: unable to set the voltage for regulator"
				"vdd_cx\n", __func__);
			regulator_put(vdd_cx);
			return ret;
		}

		ret = regulator_enable(vdd_cx);
		if (ret) {
			pr_err("%s: unable to enable regulator"
				"vdd_cx\n", __func__);
			regulator_put(vdd_cx);
		}
	} else {
		ret = regulator_disable(vdd_cx);
		if (ret) {
			pr_err("%s: Unable to disable the regulator:"
				"vdd_cx\n", __func__);
			return ret;
		}

		regulator_put(vdd_cx);
	}

	return ret;
}

static int msm_hsusb_config_vddcx(int high)
{
	int max_vol = USB_PHY_MAX_VDD_DIG_VOL;
	int min_vol;
	int ret;

	if (high)
		min_vol = USB_PHY_OPERATIONAL_MIN_VDD_DIG_VOL;
	else
		min_vol = usb_phy_susp_dig_vol;

	ret = regulator_set_voltage(vdd_cx, min_vol, max_vol);
	if (ret) {
		pr_err("%s: unable to set the voltage for regulator"
			"vdd_cx\n", __func__);
		return ret;
	}

	pr_debug("%s: min_vol:%d max_vol:%d\n", __func__, min_vol, max_vol);

	return ret;
}

#define USB_PHY_3P3_VOL_MIN	3050000 /* uV */
#define USB_PHY_3P3_VOL_MAX	3050000 /* uV */
#define USB_PHY_3P3_HPM_LOAD	50000	/* uA */
#define USB_PHY_3P3_LPM_LOAD	4000	/* uA */

#define USB_PHY_1P8_VOL_MIN	1800000 /* uV */
#define USB_PHY_1P8_VOL_MAX	1800000 /* uV */
#define USB_PHY_1P8_HPM_LOAD	50000	/* uA */
#define USB_PHY_1P8_LPM_LOAD	4000	/* uA */
static int msm_hsusb_ldo_init(int init)
{
	int rc = 0;

	if (init) {
		ldo6_3p3 = regulator_get(NULL, "8058_l6");
		if (IS_ERR(ldo6_3p3))
			return PTR_ERR(ldo6_3p3);

		ldo7_1p8 = regulator_get(NULL, "8058_l7");
		if (IS_ERR(ldo7_1p8)) {
			rc = PTR_ERR(ldo7_1p8);
			goto put_3p3;
		}

		rc = regulator_set_voltage(ldo6_3p3, USB_PHY_3P3_VOL_MIN,
				USB_PHY_3P3_VOL_MAX);
		if (rc) {
			pr_err("%s: Unable to set voltage level for"
				"ldo6_3p3 regulator\n", __func__);
			goto put_1p8;
		}
		rc = regulator_enable(ldo6_3p3);
		if (rc) {
			pr_err("%s: Unable to enable the regulator:"
				"ldo6_3p3\n", __func__);
			goto put_1p8;
		}
		rc = regulator_set_voltage(ldo7_1p8, USB_PHY_1P8_VOL_MIN,
				USB_PHY_1P8_VOL_MAX);
		if (rc) {
			pr_err("%s: Unable to set voltage level for"
				"ldo7_1p8 regulator\n", __func__);
			goto disable_3p3;
		}
		rc = regulator_enable(ldo7_1p8);
		if (rc) {
			pr_err("%s: Unable to enable the regulator:"
				"ldo7_1p8\n", __func__);
			goto disable_3p3;
		}

		return 0;
	}

	regulator_disable(ldo7_1p8);
disable_3p3:
	regulator_disable(ldo6_3p3);
put_1p8:
	regulator_put(ldo7_1p8);
put_3p3:
	regulator_put(ldo6_3p3);
	return rc;
}

static int msm_hsusb_ldo_enable(int on)
{
	int ret = 0;

	if (!ldo7_1p8 || IS_ERR(ldo7_1p8)) {
		pr_err("%s: ldo7_1p8 is not initialized\n", __func__);
		return -ENODEV;
	}

	if (!ldo6_3p3 || IS_ERR(ldo6_3p3)) {
		pr_err("%s: ldo6_3p3 is not initialized\n", __func__);
		return -ENODEV;
	}

	if (on) {
		ret = regulator_set_optimum_mode(ldo7_1p8,
				USB_PHY_1P8_HPM_LOAD);
		if (ret < 0) {
			pr_err("%s: Unable to set HPM of the regulator:"
				"ldo7_1p8\n", __func__);
			return ret;
		}
		ret = regulator_set_optimum_mode(ldo6_3p3,
				USB_PHY_3P3_HPM_LOAD);
		if (ret < 0) {
			pr_err("%s: Unable to set HPM of the regulator:"
				"ldo6_3p3\n", __func__);
			regulator_set_optimum_mode(ldo7_1p8,
				USB_PHY_1P8_LPM_LOAD);
			return ret;
		}
	} else {
		ret = regulator_set_optimum_mode(ldo7_1p8,
				USB_PHY_1P8_LPM_LOAD);
		if (ret < 0)
			pr_err("%s: Unable to set LPM of the regulator:"
				"ldo7_1p8\n", __func__);
		ret = regulator_set_optimum_mode(ldo6_3p3,
				USB_PHY_3P3_LPM_LOAD);
		if (ret < 0)
			pr_err("%s: Unable to set LPM of the regulator:"
				"ldo6_3p3\n", __func__);
	}

	pr_debug("reg (%s)\n", on ? "HPM" : "LPM");
	return ret < 0 ? ret : 0;
}
#endif
#ifdef CONFIG_USB_EHCI_MSM_72K
static void msm_hsusb_vbus_power(unsigned phy_info, int on)
{
#if defined(CONFIG_CHARGER_BQ24185)
	if (on)
		(void)bq24185_set_opa_mode(CHARGER_BOOST_MODE);
	else
		(void)bq24185_set_opa_mode(CHARGER_CHARGER_MODE);
#elif defined(CONFIG_CHARGER_BQ24160) && defined(CONFIG_USB_NCP373)
	if (on) {
		int ret;
		ret = bq24160_set_otg_lock(1);
		if (unlikely(ret < 0)) {
			pr_err("%s: failed to lock the bq24160 ret=%d\n",
								__func__, ret);
			goto do_vbus_off;
		}

		ret = ncp373_vbus_switch(1);
		if (unlikely(ret < 0)) {
			pr_err("%s: failed to switch the vbus load ret=%d\n",
								__func__, ret);
			goto do_vbus_off;
		}
		return;
	}
do_vbus_off:
	ncp373_vbus_switch(0);
	bq24160_set_otg_lock(0);
#endif
}

static struct msm_usb_host_platform_data msm_usb_host_pdata = {
	.phy_info	= (USB_PHY_INTEGRATED | USB_PHY_MODEL_45NM),
	.power_budget	= 300,
};
#endif

#if defined(CONFIG_USB_GADGET_MSM_72K) || defined(CONFIG_USB_EHCI_MSM_72K)
static struct msm_otg_platform_data msm_otg_pdata = {
	/* if usb link is in sps there is no need for
	 * usb pclk as dayatona fabric clock will be
	 * used instead
	 */
	.pemp_level		 = PRE_EMPHASIS_WITH_20_PERCENT,
	.cdr_autoreset		 = CDR_AUTO_RESET_DISABLE,
	.se1_gating		 = SE1_GATING_DISABLE,
	.bam_disable		 = 1,
#ifdef CONFIG_USB_EHCI_MSM_72K
	.pmic_id_notif_init = msm_hsusb_pmic_id_notif_init,
#endif
#ifdef CONFIG_USB_EHCI_MSM_72K
	.vbus_power = msm_hsusb_vbus_power,
#endif
	.ldo_init		 = msm_hsusb_ldo_init,
	.ldo_enable		 = msm_hsusb_ldo_enable,
	.config_vddcx            = msm_hsusb_config_vddcx,
	.init_vddcx              = msm_hsusb_init_vddcx,
#ifdef CONFIG_SEMC_CHARGER_USB_ARCH
	.chg_vbus_draw               = semc_charger_usb_vbus_draw,
	.chg_connected               = semc_charger_usb_connected,
	.chg_init               = semc_charger_usb_init,
#endif
#ifdef CONFIG_SEMC_CHARGER_CRADLE_ARCH
	.is_cradle_connected	 = semc_charger_cradle_is_connected,
#endif
#if defined(CONFIG_CHARGER_BQ24185)
	.chg_is_initialized	 = bq24185_charger_initialized,
#elif defined(CONFIG_CHARGER_BQ24160)
	.chg_is_initialized	 = bq24160_charger_initialized,
#endif
	.pmic_vbus_notif_init	 = msm_hsusb_pmic_vbus_notif_init,
	.phy_can_powercollapse	 = 1,
	.chg_drawable_ida	 = USB_IDCHG_MAX,
};
#endif

#ifdef CONFIG_USB_GADGET_MSM_72K
static struct msm_hsusb_gadget_platform_data msm_gadget_pdata = {
	.is_phy_status_timer_on = 1,
};
#endif

#ifdef CONFIG_USB_G_ANDROID

#define PID_MAGIC_ID		0x71432909
#define SERIAL_NUM_MAGIC_ID	0x61945374
#define SERIAL_NUMBER_LENGTH	127
#define DLOAD_USB_BASE_ADD	0x2A05F0C8

struct magic_num_struct {
	uint32_t pid;
	uint32_t serial_num;
};

struct dload_struct {
	uint32_t	reserved1;
	uint32_t	reserved2;
	uint32_t	reserved3;
	uint16_t	reserved4;
	uint16_t	pid;
	char		serial_number[SERIAL_NUMBER_LENGTH];
	uint16_t	reserved5;
	struct magic_num_struct
			magic_struct;
};

static int usb_diag_update_pid_and_serial_num(uint32_t pid, const char *snum)
{
	struct dload_struct __iomem *dload = 0;

	dload = ioremap(DLOAD_USB_BASE_ADD, sizeof(*dload));
	if (!dload) {
		pr_err("%s: cannot remap I/O memory region: %08x\n",
					__func__, DLOAD_USB_BASE_ADD);
		return -ENXIO;
	}

	pr_debug("%s: dload:%p pid:%x serial_num:%s\n",
				__func__, dload, pid, snum);
	/* update pid */
	dload->magic_struct.pid = PID_MAGIC_ID;
	dload->pid = pid;

	/* update serial number */
	dload->magic_struct.serial_num = 0;
	if (!snum)
		return 0;

	dload->magic_struct.serial_num = SERIAL_NUM_MAGIC_ID;
	strncpy(dload->serial_number, snum, SERIAL_NUMBER_LENGTH);
	dload->serial_number[SERIAL_NUMBER_LENGTH - 1] = '\0';

	iounmap(dload);

	return 0;
}

static struct android_usb_platform_data android_usb_pdata = {
	.update_pid_and_serial_num = usb_diag_update_pid_and_serial_num,
	.can_stall = 1,
};

static struct platform_device android_usb_device = {
	.name	= "android_usb",
	.id	= -1,
	.dev	= {
		.platform_data = &android_usb_pdata,
	},
};

#ifdef CONFIG_USB_ANDROID_GG
#define STARTUP_REASON_INDUS_LOG	(1<<29)
static int __init startup_reason_setup(char *startup)
{
	unsigned long startup_reason = 0;
	int rval = 0;

	rval = strict_strtoul(startup, 0, &startup_reason);
	if (!rval) {
		pr_info("%s: 0x%lx\n", __func__, startup_reason);
		if (startup_reason & STARTUP_REASON_INDUS_LOG)
			android_enable_usb_gg(0x0FCE, 0xD14C);
	}
	return 1;
}
__setup("startup=", startup_reason_setup);
#endif

#endif

#ifdef CONFIG_SEMC_CHARGER_CRADLE_ARCH
#define CHG_CRADLE_DET_GPIO 126

static int semc_chg_cradle_gpio_configure(int enable)
{
	int rc = 0;

	if (!!enable) {
		rc = gpio_request(CHG_CRADLE_DET_GPIO, "chg_cradle_det");
		if (rc)
			pr_err("%s: gpio_requeset failed, "
					"rc=%d\n", __func__, rc);
	} else {
		gpio_free(CHG_CRADLE_DET_GPIO);
	}
	return rc;
}

static char *semc_chg_cradle_supplied_to[] = {
	SEMC_CHARGER_AC_NAME,
};

struct semc_chg_cradle_platform_data semc_chg_cradle_platform_data = {
	.cradle_detect_gpio = CHG_CRADLE_DET_GPIO,
	.supplied_to = semc_chg_cradle_supplied_to,
	.num_supplicants = ARRAY_SIZE(semc_chg_cradle_supplied_to),
	.supply_current_limit_from_cradle = 2500,
	.gpio_configure = semc_chg_cradle_gpio_configure,
};

static struct platform_device semc_chg_cradle = {
	.name   = SEMC_CHG_CRADLE_NAME,
	.id	= -1,
	.dev    = {
		.platform_data = &semc_chg_cradle_platform_data,
	},
};
#endif

#ifdef CONFIG_SEMC_CHARGER_USB_ARCH
static char *semc_chg_usb_supplied_to[] = {
	BATTERY_CHARGALG_NAME,
	BQ27520_NAME,
};
#endif

static char *semc_bdata_supplied_to[] = {
	BQ27520_NAME,
	BATTERY_CHARGALG_NAME,
};

static struct semc_battery_platform_data semc_battery_platform_data = {
	.supplied_to = semc_bdata_supplied_to,
	.num_supplicants = ARRAY_SIZE(semc_bdata_supplied_to),
	.get_current_average = bq27520_get_current_average,
	.ddata = &device_data,
};

static struct platform_device bdata_driver = {
	.name = SEMC_BDATA_NAME,
	.id = -1,
	.dev = {
		.platform_data = &semc_battery_platform_data,
	},
};

#ifdef CONFIG_CHARGER_BQ24185
#define BQ24185_GPIO_IRQ	125

static char *bq24185_supplied_to[] = {
	BATTERY_CHARGALG_NAME,
	SEMC_BDATA_NAME,
};

static int bq24185_gpio_configure(int enable)
{
	int rc = 0;

	if (!!enable) {
		rc = gpio_request(BQ24185_GPIO_IRQ, "bq24185");
		if (rc)
			pr_err("%s: gpio_requeset failed, "
					"rc=%d\n", __func__, rc);
	} else {
		gpio_free(BQ24185_GPIO_IRQ);
	}
	return rc;
}

struct bq24185_platform_data bq24185_platform_data = {
	.name = BQ24185_NAME,
	.supplied_to = bq24185_supplied_to,
	.num_supplicants = ARRAY_SIZE(bq24185_supplied_to),
	.support_boot_charging = 1,
	.rsens = BQ24185_RSENS_REF,
	/* Maximum battery regulation voltage = 4200mV */
	.mbrv = BQ24185_MBRV_MV_4200,
	/* Maximum charger current sense voltage = 105.4mV */
	.mccsv = BQ24185_MCCSV_MV_13p6 | BQ24185_MCCSV_MV_54p4 |
		BQ24185_MCCSV_MV_37p4,
	.notify_vbus_drop = msm_otg_notify_vbus_drop,
	.gpio_configure = bq24185_gpio_configure,
	.vindpm_usb_compliant = VINDPM_4550MV,
	.vindpm_non_compliant = VINDPM_4390MV,
};
#endif

#ifdef CONFIG_CHARGER_BQ24160
#define BQ24160_GPIO_IRQ	125

static char *bq24160_supplied_to[] = {
	BATTERY_CHARGALG_NAME,
	SEMC_BDATA_NAME,
};

static int bq24160_gpio_configure(int enable)
{
	int rc = 0;

	if (!!enable) {
		rc = gpio_request(BQ24160_GPIO_IRQ, "bq24160");
		if (rc)
			pr_err("%s: gpio_requeset failed, "
					"rc=%d\n", __func__, rc);
	} else {
		gpio_free(BQ24160_GPIO_IRQ);
	}
	return rc;
}

struct bq24160_platform_data bq24160_platform_data = {
	.name = BQ24160_NAME,
	.supplied_to = bq24160_supplied_to,
	.num_supplicants = ARRAY_SIZE(bq24160_supplied_to),
	.support_boot_charging = 1,
	.notify_vbus_drop = msm_otg_notify_vbus_drop,
	.gpio_configure = bq24160_gpio_configure,
};
#endif

#define GPIO_BQ27520_SOC_INT 123
#define LIPO_BAT_MAX_VOLTAGE 4200
#define LIPO_BAT_MIN_VOLTAGE 3000
#define FULLY_CHARGED_AND_RECHARGE_CAP 95

static char *bq27520_supplied_to[] = {
	BATTERY_CHARGALG_NAME,
};

static struct bq27520_block_table bq27520_block_table[BQ27520_BTBL_MAX] = {
	{0x61, 0x00}, {0x3E, 0x24}, {0x3F, 0x00}, {0x42, 0x00},
	{0x43, 0x46}, {0x44, 0x00}, {0x45, 0x19}, {0x46, 0x00},
	{0x47, 0x64}, {0x48, 0x28}, {0x4B, 0xFF}, {0x4C, 0x5F},
	{0x60, 0xF4}
};

static int bq27520_gpio_configure(int enable)
{
	int rc = 0;

	if (!!enable) {
		rc = gpio_request(GPIO_BQ27520_SOC_INT, "bq27520");
		if (rc)
			pr_err("%s: gpio_requeset failed, "
					"rc=%d\n", __func__, rc);
	} else {
		gpio_free(GPIO_BQ27520_SOC_INT);
	}
	return rc;
}

struct bq27520_platform_data bq27520_platform_data = {
	.name = BQ27520_NAME,
	.supplied_to = bq27520_supplied_to,
	.num_supplicants = ARRAY_SIZE(bq27520_supplied_to),
	.lipo_bat_max_volt = LIPO_BAT_MAX_VOLTAGE,
	.lipo_bat_min_volt = LIPO_BAT_MIN_VOLTAGE,
	.battery_dev_name = SEMC_BDATA_NAME,
	.gpio_configure = bq27520_gpio_configure,
	.polling_lower_capacity = FULLY_CHARGED_AND_RECHARGE_CAP,
	.polling_upper_capacity = 100,
	.udatap = bq27520_block_table,
#ifdef CONFIG_BATTERY_CHARGALG
	.disable_algorithm = battery_chargalg_disable,
#endif
#ifdef CONFIG_SEMC_CHARGER_CRADLE_ARCH
	.get_ac_online_status = semc_charger_get_ac_online_status,
#endif
};

static char *battery_chargalg_supplied_to[] = {
	SEMC_BDATA_NAME,
};

static struct battery_chargalg_platform_data battery_chargalg_platform_data = {
	.name = BATTERY_CHARGALG_NAME,
	.supplied_to = battery_chargalg_supplied_to,
	.num_supplicants = ARRAY_SIZE(battery_chargalg_supplied_to),
	.ext_eoc_recharge_enable = 1,
	.temp_hysteresis_design = 3,
	.ddata = &device_data,
	.batt_volt_psy_name = BQ27520_NAME,
	.batt_curr_psy_name = BQ27520_NAME,
#ifdef CONFIG_CHARGER_BQ24185
	.turn_on_charger = bq24185_turn_on_charger,
	.turn_off_charger = bq24185_turn_off_charger,
	.set_charger_voltage = bq24185_set_charger_voltage,
	.set_charger_current = bq24185_set_charger_current,
	.set_input_current_limit = bq24185_set_input_current_limit,
	.set_charging_status = bq24185_set_ext_charging_status,
	.get_supply_current_limit = NULL,
	.get_restrict_ctl = NULL,
	.get_restricted_setting = NULL,
	.setup_exchanged_power_supply = NULL,
	.charge_set_current_1 = 350,
	.charge_set_current_2 = 650,
	.charge_set_current_3 = 850,
	.overvoltage_max_design = 4225,
#endif
#ifdef CONFIG_CHARGER_BQ24160
	.turn_on_charger = bq24160_turn_on_charger,
	.turn_off_charger = bq24160_turn_off_charger,
	.set_charger_voltage = bq24160_set_charger_voltage,
	.set_charger_current = bq24160_set_charger_current,
	.set_input_current_limit = bq24160_set_input_current_limit,
	.set_charging_status = bq24160_set_ext_charging_status,
	.get_supply_current_limit = NULL,
	.get_restrict_ctl = bq24160_is_restricted_by_charger_revision,
	.get_restricted_setting = bq24160_get_restricted_setting,
	.setup_exchanged_power_supply = bq24160_setup_exchanged_power_supply,
	.charge_set_current_1 = 350,
	.charge_set_current_2 = 575,
	.charge_set_current_3 = 725,
	.overvoltage_max_design = 4245,
#endif
#ifdef CONFIG_SEMC_CHARGER_USB_ARCH
	.get_supply_current_limit = semc_charger_usb_current_ma,
#endif
#ifdef CONFIG_SEMC_CHARGER_CRADLE_ARCH
	.get_ac_online_status = semc_charger_get_ac_online_status,
	.set_input_current_limit_dual = bq24160_set_input_current_limit_dual,
	.set_input_voltage_dpm_usb = bq24160_set_input_voltage_dpm_usb,
	.set_input_voltage_dpm_cradle = bq24160_set_input_voltage_dpm_in,
	.get_supply_current_limit_cradle = semc_charger_cradle_current_ma,
#endif
	.allow_dynamic_charge_current_ctrl = 1,
	.average_current_min_limit = -1,
	.average_current_max_limit = 250,
};

static struct platform_device battery_chargalg_platform_device = {
	.name = BATTERY_CHARGALG_NAME,
	.id = -1,
	.dev = {
		.platform_data = &battery_chargalg_platform_data,
	},
};

#if defined(CONFIG_MSM_VPE) || defined(CONFIG_SEMC_VPE1)
static struct resource msm_vpe_resources[] = {
	{
		.start	= 0x05300000,
		.end	= 0x05300000 + SZ_1M - 1,
		.flags	= IORESOURCE_MEM,
	},
	{
		.start	= INT_VPE,
		.end	= INT_VPE,
		.flags	= IORESOURCE_IRQ,
	},
};
#endif

#ifdef CONFIG_MSM_VPE
static struct platform_device msm_vpe_device = {
	.name = "msm_vpe",
	.id   = 0,
	.num_resources = ARRAY_SIZE(msm_vpe_resources),
	.resource = msm_vpe_resources,
};
#endif
#if defined(CONFIG_SEMC_VPE1)
static struct platform_device semc_vpe1_device = {
	.name		= "semc_vpe1",
	.id		= 0,
	.num_resources	= ARRAY_SIZE(msm_vpe_resources),
	.resource	= msm_vpe_resources,
};
#endif

#ifdef CONFIG_MSM_CAMERA
#define VFE_CAMIF_TIMER1_GPIO 29
#define VFE_CAMIF_TIMER2_GPIO 30
#define VFE_CAMIF_TIMER3_GPIO_INT 31

int msm_cam_gpio_tbl[] = {
	25, /* CHAT_CAM_RST_N */
	32, /* CAMIF_MCLK */
	47, /* CAMIF_I2C_DATA */
	48, /* CAMIF_I2C_CLK */
	106, /* CAM_RST_N */
};

enum msm_cam_stat {
	MSM_CAM_OFF,
	MSM_CAM_ON,
};

static int config_gpio_table(enum msm_cam_stat stat)
{
	int rc = 0;
	int i = 0;
	int j = 0;

	if (stat == MSM_CAM_ON) {
		for (i = 0; i < ARRAY_SIZE(msm_cam_gpio_tbl); i++) {
			rc = gpio_request(msm_cam_gpio_tbl[i], "CAM_GPIO");
			if (unlikely(rc < 0)) {
				pr_err("%s not able to get gpio\n", __func__);
				for (j = i - 1; j >= 0; j--)
					gpio_free(msm_cam_gpio_tbl[j]);
				break;
			}
		}
	} else {
		for (i = 0; i < ARRAY_SIZE(msm_cam_gpio_tbl); i++)
			gpio_free(msm_cam_gpio_tbl[i]);
	}
	return rc;
}

static int config_camera_on_gpios(void)
{
	int rc = 0;

	rc = config_gpio_table(MSM_CAM_ON);
	if (rc < 0) {
		printk(KERN_ERR "%s: CAMSENSOR gpio table request"
		"failed\n", __func__);
	}

	return rc;
}

static void config_camera_off_gpios(void)
{
	config_gpio_table(MSM_CAM_OFF);
}
#ifdef CONFIG_MSM_BUS_SCALING
static struct msm_bus_vectors cam_init_vectors[] = {
	{
		.src = MSM_BUS_MASTER_VFE,
		.dst = MSM_BUS_SLAVE_SMI,
		.ab  = 0,
		.ib  = 0,
	},
	{
		.src = MSM_BUS_MASTER_VFE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 0,
		.ib  = 0,
	},
	{
		.src = MSM_BUS_MASTER_VPE,
		.dst = MSM_BUS_SLAVE_SMI,
		.ab  = 0,
		.ib  = 0,
	},
	{
		.src = MSM_BUS_MASTER_VPE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 0,
		.ib  = 0,
	},
	{
		.src = MSM_BUS_MASTER_JPEG_ENC,
		.dst = MSM_BUS_SLAVE_SMI,
		.ab  = 0,
		.ib  = 0,
	},
	{
		.src = MSM_BUS_MASTER_JPEG_ENC,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 0,
		.ib  = 0,
	},
};

static struct msm_bus_vectors cam_preview_vectors[] = {
	{
		.src = MSM_BUS_MASTER_VFE,
		.dst = MSM_BUS_SLAVE_SMI,
		.ab  = 0,
		.ib  = 0,
	},
	{
		.src = MSM_BUS_MASTER_VFE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 484614900,
		.ib  = 2100000000,
	},
	{
		.src = MSM_BUS_MASTER_VPE,
		.dst = MSM_BUS_SLAVE_SMI,
		.ab  = 149299200,
		.ib  = 2640000000U,
	},
	{
		.src = MSM_BUS_MASTER_VPE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 223948800,
		.ib  = 335923200,
	},
	{
		.src = MSM_BUS_MASTER_JPEG_ENC,
		.dst = MSM_BUS_SLAVE_SMI,
		.ab  = 0,
		.ib  = 0,
	},
	{
		.src = MSM_BUS_MASTER_JPEG_ENC,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 0,
		.ib  = 0,
	},
};

static struct msm_bus_vectors cam_video_vectors[] = {
	{
		.src = MSM_BUS_MASTER_VFE,
		.dst = MSM_BUS_SLAVE_SMI,
		.ab  = 0,
		.ib  = 0,
	},
	{
		.src = MSM_BUS_MASTER_VFE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 656132400,
		.ib  = 2100000000,
	},
	{
		.src = MSM_BUS_MASTER_VPE,
		.dst = MSM_BUS_SLAVE_SMI,
		.ab  = 215654400,
		.ib  = 2640000000U,
	},
	{
		.src = MSM_BUS_MASTER_VPE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 323481600,
		.ib  = 485222400,
	},
	{
		.src = MSM_BUS_MASTER_JPEG_ENC,
		.dst = MSM_BUS_SLAVE_SMI,
		.ab  = 0,
		.ib  = 0,
	},
	{
		.src = MSM_BUS_MASTER_JPEG_ENC,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 0,
		.ib  = 0,
	},
};

static struct msm_bus_vectors cam_snapshot_vectors[] = {
	{
		.src = MSM_BUS_MASTER_VFE,
		.dst = MSM_BUS_SLAVE_SMI,
		.ab  = 0,
		.ib  = 0,
	},
	{
		.src = MSM_BUS_MASTER_VFE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 660234240,
		.ib  = 1056374784,
	},
	{
		.src = MSM_BUS_MASTER_VPE,
		.dst = MSM_BUS_SLAVE_SMI,
		.ab  = 79902720,
		.ib  = 127844352,
	},
	{
		.src = MSM_BUS_MASTER_VPE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 1042495488,
		.ib  = 1667992781,
	},
	{
		.src = MSM_BUS_MASTER_JPEG_ENC,
		.dst = MSM_BUS_SLAVE_SMI,
		.ab  = 320864256,
		.ib  = 513382810,
	},
	{
		.src = MSM_BUS_MASTER_JPEG_ENC,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 320864256,
		.ib  = 513382810,
	},
};

static struct msm_bus_vectors cam_zsl_vectors[] = {
	{
		.src = MSM_BUS_MASTER_VFE,
		.dst = MSM_BUS_SLAVE_SMI,
		.ab  = 566231040,
		.ib  = 905969664,
	},
	{
		.src = MSM_BUS_MASTER_VFE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 706199040,
		.ib  = 1129918464,
	},
	{
		.src = MSM_BUS_MASTER_VPE,
		.dst = MSM_BUS_SLAVE_SMI,
		.ab  = 0,
		.ib  = 0,
	},
	{
		.src = MSM_BUS_MASTER_VPE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 0,
		.ib  = 0,
	},
	{
		.src = MSM_BUS_MASTER_JPEG_ENC,
		.dst = MSM_BUS_SLAVE_SMI,
		.ab  = 320864256,
		.ib  = 513382810,
	},
	{
		.src = MSM_BUS_MASTER_JPEG_ENC,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 320864256,
		.ib  = 513382810,
	},
};

static struct msm_bus_vectors cam_stereo_video_vectors[] = {
	{
		.src = MSM_BUS_MASTER_VFE,
		.dst = MSM_BUS_SLAVE_SMI,
		.ab  = 212336640,
		.ib  = 339738624,
	},
	{
		.src = MSM_BUS_MASTER_VFE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 25090560,
		.ib  = 40144896,
	},
	{
		.src = MSM_BUS_MASTER_VPE,
		.dst = MSM_BUS_SLAVE_SMI,
		.ab  = 239708160,
		.ib  = 383533056,
	},
	{
		.src = MSM_BUS_MASTER_VPE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 79902720,
		.ib  = 127844352,
	},
	{
		.src = MSM_BUS_MASTER_JPEG_ENC,
		.dst = MSM_BUS_SLAVE_SMI,
		.ab  = 0,
		.ib  = 0,
	},
	{
		.src = MSM_BUS_MASTER_JPEG_ENC,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 0,
		.ib  = 0,
	},
};

static struct msm_bus_vectors cam_stereo_snapshot_vectors[] = {
	{
		.src = MSM_BUS_MASTER_VFE,
		.dst = MSM_BUS_SLAVE_SMI,
		.ab  = 0,
		.ib  = 0,
	},
	{
		.src = MSM_BUS_MASTER_VFE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 300902400,
		.ib  = 481443840,
	},
	{
		.src = MSM_BUS_MASTER_VPE,
		.dst = MSM_BUS_SLAVE_SMI,
		.ab  = 230307840,
		.ib  = 368492544,
	},
	{
		.src = MSM_BUS_MASTER_VPE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 245113344,
		.ib  = 392181351,
	},
	{
		.src = MSM_BUS_MASTER_JPEG_ENC,
		.dst = MSM_BUS_SLAVE_SMI,
		.ab  = 106536960,
		.ib  = 170459136,
	},
	{
		.src = MSM_BUS_MASTER_JPEG_ENC,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 106536960,
		.ib  = 170459136,
	},
};

static struct msm_bus_paths cam_bus_client_config[] = {
	{
		ARRAY_SIZE(cam_init_vectors),
		cam_init_vectors,
	},
	{
		ARRAY_SIZE(cam_preview_vectors),
		cam_preview_vectors,
	},
	{
		ARRAY_SIZE(cam_video_vectors),
		cam_video_vectors,
	},
	{
		ARRAY_SIZE(cam_snapshot_vectors),
		cam_snapshot_vectors,
	},
	{
		ARRAY_SIZE(cam_zsl_vectors),
		cam_zsl_vectors,
	},
	{
		ARRAY_SIZE(cam_stereo_video_vectors),
		cam_stereo_video_vectors,
	},
	{
		ARRAY_SIZE(cam_stereo_snapshot_vectors),
		cam_stereo_snapshot_vectors,
	},
};

static struct msm_bus_scale_pdata cam_bus_client_pdata = {
		cam_bus_client_config,
		ARRAY_SIZE(cam_bus_client_config),
		.name = "msm_camera",
};
#endif

struct msm_camera_device_platform_data msm_camera_device_data = {
	.camera_gpio_on		= config_camera_on_gpios,
	.camera_gpio_off	= config_camera_off_gpios,
	.ioext.csiphy		= 0x04800000,
	.ioext.csisz		= 0x00000400,
	.ioext.csiirq		= CSI_0_IRQ,
	.ioclk.mclk_clk_rate	= 8000000,
	.ioclk.vfe_clk_rate	= 266667000,
#ifdef CONFIG_MSM_BUS_SCALING
	.cam_bus_scale_table = &cam_bus_client_pdata,
#endif
};

struct msm_camera_device_platform_data msm_sub_camera_device_data = {
	.camera_gpio_on		= config_camera_on_gpios,
	.camera_gpio_off	= config_camera_off_gpios,
	.ioext.csiphy		= 0x04900000,
	.ioext.csisz		= 0x00000400,
	.ioext.csiirq		= CSI_1_IRQ,
	.ioclk.mclk_clk_rate	= 8000000,
	.ioclk.vfe_clk_rate	= 228570000,
#ifdef CONFIG_MSM_BUS_SCALING
	.cam_bus_scale_table = &cam_bus_client_pdata,
#endif
};

struct resource msm_camera_resources[] = {
	{
		.start	= 0x04500000,
		.end	= 0x04500000 + SZ_1M - 1,
		.flags	= IORESOURCE_MEM,
	},
	{
		.start	= VFE_IRQ,
		.end	= VFE_IRQ,
		.flags	= IORESOURCE_IRQ,
	},
};

#if defined(CONFIG_SEMC_CAM_MAIN) || defined(CONFIG_SEMC_CAM_SUB)
static struct msm_camera_sensor_flash_data flash_none = {
	.flash_type	= MSM_CAMERA_FLASH_NONE,
	.flash_src	= NULL,
};
#endif

#ifdef CONFIG_SEMC_CAM_MAIN
static struct msm_camera_sensor_info msm_camera_semc_sensor_main_data = {
	.sensor_name		= "semc_sensor_main",
	.sensor_reset		= 106,
	.sensor_pwd		= 106,
	.vcm_pwd		= 106,
	.vcm_enable		= 106,
	.mclk			= 32,
	.flash_type		= MSM_CAMERA_FLASH_NONE,
	.pdata			= &msm_camera_device_data,
	.resource		= msm_camera_resources,
	.num_resources		= ARRAY_SIZE(msm_camera_resources),
	.flash_data		= &flash_none,
	.csi_if			= 1, /* mipi interface direct */
	.csi_params = {
		.data_format	= CSI_10BIT,
		.lane_cnt	= 4,
		.lane_assign	= 0xe4,
		.settle_cnt	= 25,
		.dpcm_scheme	= 0,
	},
};

static struct platform_device msm_camera_semc_sensor_main = {
	.name = "msm_camera_semc_sensor_main",
	.dev = {
		.platform_data = &msm_camera_semc_sensor_main_data,
	},
};
#endif

#ifdef CONFIG_SEMC_CAM_SUB
static struct msm_camera_sensor_info msm_camera_semc_sensor_sub_data = {
	.sensor_name		= "semc_sensor_sub",
	.sensor_reset		= 25,
	.sensor_pwd		= 106,
	.vcm_pwd		= 106,
	.vcm_enable		= 106,
	.mclk			= 32,
	.flash_type		= MSM_CAMERA_FLASH_NONE,
	.pdata			= &msm_sub_camera_device_data,
	.resource		= msm_camera_resources,
	.num_resources		= ARRAY_SIZE(msm_camera_resources),
	.flash_data		= &flash_none,
	.csi_if			= 1, /* mipi interface direct */
	.csi_params = {
		.data_format	= CSI_10BIT,
		.lane_cnt	= 1,
		.lane_assign	= 0xe4,
		.settle_cnt	= 34,
		.dpcm_scheme	= 0,
	},
};

static struct platform_device msm_camera_semc_sensor_sub = {
	.name = "msm_camera_semc_sensor_sub",
	.dev  = {
		.platform_data = &msm_camera_semc_sensor_sub_data,
	},
};
#endif

static struct i2c_board_info msm_camera_boardinfo[] __initdata = {
#ifdef CONFIG_SEMC_CAM_MAIN
	{
		I2C_BOARD_INFO("semc_sensor_main", 0x1A),
	},
#endif
#ifdef CONFIG_SEMC_CAM_SUB
	{
		I2C_BOARD_INFO("semc_sensor_sub", 0x48),
	},
#endif
};
#endif

#ifdef CONFIG_MSM_GEMINI
static struct resource msm_gemini_resources[] = {
	{
		.start  = 0x04600000,
		.end    = 0x04600000 + SZ_1M - 1,
		.flags  = IORESOURCE_MEM,
	},
	{
		.start  = INT_JPEG,
		.end    = INT_JPEG,
		.flags  = IORESOURCE_IRQ,
	},
};

static struct platform_device msm_gemini_device = {
	.name           = "msm_gemini",
	.resource       = msm_gemini_resources,
	.num_resources  = ARRAY_SIZE(msm_gemini_resources),
};
#endif

#ifdef CONFIG_I2C_QUP
static void gsbi_qup_i2c_gpio_config(int adap_id, int config_type)
{
}

static struct msm_i2c_platform_data msm_gsbi3_qup_i2c_pdata = {
	.clk_freq = 400000,
	.src_clk_rate = 24000000,
	.msm_i2c_config_gpio = gsbi_qup_i2c_gpio_config,
};

static struct msm_i2c_platform_data msm_gsbi4_qup_i2c_pdata = {
	.clk_freq = 100000,
	.src_clk_rate = 24000000,
	.msm_i2c_config_gpio = gsbi_qup_i2c_gpio_config,
};

#ifdef CONFIG_MSM_GSBI5_I2C
static struct msm_i2c_platform_data msm_gsbi5_qup_i2c_pdata = {
	.clk_freq = 355000,
	.src_clk_rate = 24000000,
	.use_gsbi_shared_mode = 1,
	.msm_i2c_config_gpio = gsbi_qup_i2c_gpio_config,
};
#endif

static struct msm_i2c_platform_data msm_gsbi7_qup_i2c_pdata = {
	.clk_freq = 100000,
	.src_clk_rate = 24000000,
	.use_gsbi_shared_mode = 1,
	.msm_i2c_config_gpio = gsbi_qup_i2c_gpio_config,
};

static struct msm_i2c_platform_data msm_gsbi8_qup_i2c_pdata = {
	.clk_freq = 355000,
	.src_clk_rate = 24000000,
	.msm_i2c_config_gpio = gsbi_qup_i2c_gpio_config,
};

static struct msm_i2c_platform_data msm_gsbi9_qup_i2c_pdata = {
	.clk_freq = 100000,
	.src_clk_rate = 24000000,
	.msm_i2c_config_gpio = gsbi_qup_i2c_gpio_config,
};

static struct msm_i2c_platform_data msm_gsbi12_qup_i2c_pdata = {
	.clk_freq = 355000,
	.src_clk_rate = 24000000,
	.use_gsbi_shared_mode = 1,
	.msm_i2c_config_gpio = gsbi_qup_i2c_gpio_config,
};
#endif

#if defined(CONFIG_SPI_QUP) || defined(CONFIG_SPI_QUP_MODULE)
static struct msm_spi_platform_data msm_gsbi1_qup_spi_pdata = {
	.max_clock_speed = 24000000,
};

static struct msm_spi_platform_data msm_gsbi10_qup_spi_pdata = {
	.max_clock_speed = 24000000,
};
#endif

#ifdef CONFIG_I2C_SSBI
/* CODEC/TSSC SSBI */
static struct msm_i2c_ssbi_platform_data msm_ssbi3_pdata = {
	.controller_type = MSM_SBI_CTRL_SSBI,
};
#endif

#ifdef CONFIG_FB_MSM_LCDC_DSUB
/* VGA = 1440 x 900 x 4(bpp) x 2(pages)
   prim = 1024 x 600 x 4(bpp) x 2(pages)
   This is the difference. */
#define MSM_FB_DSUB_PMEM_ADDER (0xA32000-0x4B0000)
#else
#define MSM_FB_DSUB_PMEM_ADDER (0)
#endif

#ifdef CONFIG_FB_MSM_TRIPLE_BUFFER
#define MSM_FB_PRIM_BUF_SIZE (1280 * 736 * 4 * 3) /* 4 bpp x 3 pages */
#else
#define MSM_FB_PRIM_BUF_SIZE (1280 * 736 * 4 * 2) /* 4 bpp x 2 pages */
#endif

#ifdef CONFIG_FB_MSM_HDMI_MSM_PANEL
#define MSM_FB_EXT_BUF_SIZE  (1920 * 1088 * 2 * 1) /* 2 bpp x 1 page */
#elif defined(CONFIG_FB_MSM_TVOUT)
#define MSM_FB_EXT_BUF_SIZE  (720 * 576 * 2 * 2) /* 2 bpp x 2 pages */
#else
#define MSM_FB_EXT_BUFT_SIZE	0
#endif

/* Note: must be multiple of 4096 */
#define MSM_FB_SIZE roundup(MSM_FB_PRIM_BUF_SIZE + MSM_FB_EXT_BUF_SIZE + \
				MSM_FB_DSUB_PMEM_ADDER, 4096)

#define MSM_PMEM_SF_SIZE 0x6E00000 /* 110 Mbytes */
#define MSM_HDMI_PRIM_PMEM_SF_SIZE 0x4000000 /* 64 Mbytes */

#ifdef CONFIG_FB_MSM_HDMI_AS_PRIMARY
unsigned char hdmi_is_primary = 1;
#else
unsigned char hdmi_is_primary;
#endif

#ifdef CONFIG_FB_MSM_OVERLAY0_WRITEBACK
#define MSM_FB_OVERLAY0_WRITEBACK_SIZE roundup((1280 * 736 * 3 * 2), 4096)
#else
#define MSM_FB_OVERLAY0_WRITEBACK_SIZE (0)
#endif  /* CONFIG_FB_MSM_OVERLAY0_WRITEBACK */

#ifdef CONFIG_FB_MSM_OVERLAY1_WRITEBACK
#define MSM_FB_OVERLAY1_WRITEBACK_SIZE roundup((1920 * 1088 * 3 * 2), 4096)
#else
#define MSM_FB_OVERLAY1_WRITEBACK_SIZE (0)
#endif  /* CONFIG_FB_MSM_OVERLAY1_WRITEBACK */

#define MSM_PMEM_KERNEL_EBI1_SIZE  0x600000
#define MSM_PMEM_ADSP_SIZE         0x2000000
#define MSM_PMEM_CAMERA_SIZE       0x5000000
#define MSM_PMEM_AUDIO_SIZE        0x28B000
#define MSM_PMEM_SWIQI_SIZE        0x2000000

#define MSM_SMI_BASE          0x38000000
#define MSM_SMI_SIZE          0x4000000

#define KERNEL_SMI_BASE       (MSM_SMI_BASE)
#define KERNEL_SMI_SIZE       0x600000

#define USER_SMI_BASE         (KERNEL_SMI_BASE + KERNEL_SMI_SIZE)
#define USER_SMI_SIZE         (MSM_SMI_SIZE - KERNEL_SMI_SIZE)
#define MSM_PMEM_SMIPOOL_SIZE USER_SMI_SIZE

#define MSM_ION_SF_SIZE                0x4000000 /* 64MB */
#define MSM_ION_CAMERA_SIZE     MSM_PMEM_ADSP_SIZE
#define MSM_ION_MM_FW_SIZE      0x200000 /* (2MB) */
#define MSM_ION_MM_SIZE         0x3600000 /* (54MB) */
#define MSM_ION_MFC_SIZE       SZ_8K
#define MSM_ION_WB_SIZE                0x600000 /* 6MB */
#define MSM_ION_QSECOM_SIZE	0x300000 /* (3MB) */
#define MSM_ION_AUDIO_SIZE	MSM_PMEM_AUDIO_SIZE

#ifdef CONFIG_MSM_MULTIMEDIA_USE_ION
#define MSM_ION_HEAP_NUM	9
#define MSM_HDMI_PRIM_ION_SF_SIZE MSM_HDMI_PRIM_PMEM_SF_SIZE
static unsigned msm_ion_sf_size = MSM_ION_SF_SIZE;
#else
#define MSM_ION_HEAP_NUM	1
#endif

static unsigned fb_size;
static int __init fb_size_setup(char *p)
{
	fb_size = memparse(p, NULL);
	return 0;
}
early_param("fb_size", fb_size_setup);

static unsigned pmem_kernel_ebi1_size = MSM_PMEM_KERNEL_EBI1_SIZE;
static int __init pmem_kernel_ebi1_size_setup(char *p)
{
	pmem_kernel_ebi1_size = memparse(p, NULL);
	return 0;
}
early_param("pmem_kernel_ebi1_size", pmem_kernel_ebi1_size_setup);

#ifdef CONFIG_ANDROID_PMEM
static unsigned pmem_sf_size = MSM_PMEM_SF_SIZE;
static int __init pmem_sf_size_setup(char *p)
{
	pmem_sf_size = memparse(p, NULL);
	return 0;
}
early_param("pmem_sf_size", pmem_sf_size_setup);

static unsigned pmem_adsp_size = MSM_PMEM_ADSP_SIZE;

static int __init pmem_adsp_size_setup(char *p)
{
	pmem_adsp_size = memparse(p, NULL);
	return 0;
}
early_param("pmem_adsp_size", pmem_adsp_size_setup);

static unsigned pmem_camera_size = MSM_PMEM_CAMERA_SIZE;

static int __init pmem_camera_size_setup(char *p)
{
	pmem_camera_size = memparse(p, NULL);
	return 0;
}
early_param("pmem_camera_size", pmem_camera_size_setup);

static unsigned pmem_audio_size = MSM_PMEM_AUDIO_SIZE;

static int __init pmem_audio_size_setup(char *p)
{
	pmem_audio_size = memparse(p, NULL);
	return 0;
}
early_param("pmem_audio_size", pmem_audio_size_setup);

#ifdef CONFIG_SEMC_SWIQI
static unsigned pmem_swiqi_size = MSM_PMEM_SWIQI_SIZE;

static int __init pmem_swiqi_size_setup(char *p)
{
	pmem_swiqi_size = memparse(p, NULL);
	return 0;
}
early_param("pmem_swiqi_size", pmem_swiqi_size_setup);
#endif
#endif

static struct resource msm_fb_resources[] = {
	{
		.flags  = IORESOURCE_DMA,
	}
};

static int msm_fb_detect_panel(const char *name)
{
	if (!strncmp(name, LCDC_SAMSUNG_WSVGA_PANEL_NAME,
		     strnlen(LCDC_SAMSUNG_WSVGA_PANEL_NAME,
			     PANEL_NAME_MAX_LEN)))
		return 0;

#if !defined(CONFIG_FB_MSM_LCDC_AUTO_DETECT) && \
	!defined(CONFIG_FB_MSM_MIPI_PANEL_AUTO_DETECT) && \
	!defined(CONFIG_FB_MSM_LCDC_MIPI_PANEL_AUTO_DETECT)
		if (!strncmp(name, MIPI_VIDEO_TOSHIBA_WVGA_PANEL_NAME,
				strnlen(MIPI_VIDEO_TOSHIBA_WVGA_PANEL_NAME,
					PANEL_NAME_MAX_LEN)))
			return 0;

		if (!strncmp(name, MIPI_VIDEO_NOVATEK_QHD_PANEL_NAME,
				strnlen(MIPI_VIDEO_NOVATEK_QHD_PANEL_NAME,
					  PANEL_NAME_MAX_LEN)))
			return 0;

		if (!strncmp(name, MIPI_CMD_NOVATEK_QHD_PANEL_NAME,
				strnlen(MIPI_CMD_NOVATEK_QHD_PANEL_NAME,
					PANEL_NAME_MAX_LEN)))
			return 0;
#endif

	if (!strncmp(name, HDMI_PANEL_NAME,
			strnlen(HDMI_PANEL_NAME,
				PANEL_NAME_MAX_LEN)))
		return 0;

	if (!strncmp(name, TVOUT_PANEL_NAME,
			strnlen(TVOUT_PANEL_NAME,
				PANEL_NAME_MAX_LEN)))
		return 0;

	pr_warning("%s: not supported '%s'", __func__, name);
	return -ENODEV;
}

static struct msm_fb_platform_data msm_fb_pdata = {
	.detect_client = msm_fb_detect_panel,
};

static struct platform_device msm_fb_device = {
	.name   = "msm_fb",
	.id     = 0,
	.num_resources     = ARRAY_SIZE(msm_fb_resources),
	.resource          = msm_fb_resources,
	.dev.platform_data = &msm_fb_pdata,
};

#ifdef CONFIG_ANDROID_PMEM
#ifndef CONFIG_MSM_MULTIMEDIA_USE_ION
static struct android_pmem_platform_data android_pmem_pdata = {
	.name = "pmem",
	.allocator_type = PMEM_ALLOCATORTYPE_ALLORNOTHING,
	.cached = 1,
	.memory_type = MEMTYPE_EBI1,
};

static struct platform_device android_pmem_device = {
	.name = "android_pmem",
	.id = 0,
	.dev = {.platform_data = &android_pmem_pdata},
};

static struct android_pmem_platform_data android_pmem_adsp_pdata = {
	.name = "pmem_adsp",
	.allocator_type = PMEM_ALLOCATORTYPE_BITMAP,
	.cached = 0,
	.memory_type = MEMTYPE_EBI1,
	.map_on_demand = 1,
};

static struct platform_device android_pmem_adsp_device = {
	.name = "android_pmem",
	.id = 2,
	.dev = { .platform_data = &android_pmem_adsp_pdata },
};

static struct android_pmem_platform_data android_pmem_camera_pdata = {
	.name = "pmem_camera",
	.allocator_type = PMEM_ALLOCATORTYPE_BITMAP,
	.cached = 1,
	.memory_type = MEMTYPE_EBI1,
	.map_on_demand = 1,
};

static struct platform_device android_pmem_camera_device = {
	.name = "android_pmem",
	.id = 3,
	.dev = { .platform_data = &android_pmem_camera_pdata },
};
#endif

static struct android_pmem_platform_data android_pmem_audio_pdata = {
	.name = "pmem_audio",
	.allocator_type = PMEM_ALLOCATORTYPE_BITMAP,
	.cached = 0,
	.memory_type = MEMTYPE_EBI1,
};

static struct platform_device android_pmem_audio_device = {
	.name = "android_pmem",
	.id = 4,
	.dev = { .platform_data = &android_pmem_audio_pdata },
};

#ifdef CONFIG_SEMC_SWIQI
static struct android_pmem_platform_data android_pmem_swiqi_pdata = {
	.name = "pmem_swiqi",
	.allocator_type = PMEM_ALLOCATORTYPE_BITMAP,
	.cached = 1,
	.memory_type = MEMTYPE_EBI1,
	.map_on_demand = 1,
};

static struct platform_device android_pmem_swiqi_device = {
	.name = "android_pmem",
	.id = 5,
	.dev = { .platform_data = &android_pmem_swiqi_pdata },
};
#endif

#define PMEM_BUS_WIDTH(_bw) \
	{ \
		.vectors = &(struct msm_bus_vectors){ \
			.src = MSM_BUS_MASTER_AMPSS_M0, \
			.dst = MSM_BUS_SLAVE_SMI, \
			.ib = (_bw), \
			.ab = 0, \
		}, \
	.num_paths = 1, \
	}

static struct msm_bus_paths mem_smi_table[] = {
	[0] = PMEM_BUS_WIDTH(0), /* Off */
	[1] = PMEM_BUS_WIDTH(1), /* On */
};

static struct msm_bus_scale_pdata smi_client_pdata = {
	.usecase = mem_smi_table,
	.num_usecases = ARRAY_SIZE(mem_smi_table),
	.name = "mem_smi",
};

int request_smi_region(void *data)
{
	int bus_id = (int) data;

	msm_bus_scale_client_update_request(bus_id, 1);
	return 0;
}

int release_smi_region(void *data)
{
	int bus_id = (int) data;

	msm_bus_scale_client_update_request(bus_id, 0);
	return 0;
}

void *setup_smi_region(void)
{
	return (void *)msm_bus_scale_register_client(&smi_client_pdata);
}
#ifndef CONFIG_MSM_MULTIMEDIA_USE_ION
static struct android_pmem_platform_data android_pmem_smipool_pdata = {
	.name = "pmem_smipool",
	.allocator_type = PMEM_ALLOCATORTYPE_BITMAP,
	.cached = 1,
	.memory_type = MEMTYPE_SMI,
	.request_region = request_smi_region,
	.release_region = release_smi_region,
	.setup_region = setup_smi_region,
	.map_on_demand = 1,
};
static struct platform_device android_pmem_smipool_device = {
	.name = "android_pmem",
	.id = 7,
	.dev = { .platform_data = &android_pmem_smipool_pdata },
};
#endif
#endif

#define GPIO_DONGLE_PWR_EN 258

#if (!defined(CONFIG_SPI_QUP)) && \
	(defined(CONFIG_FB_MSM_LCDC_SAMSUNG_OLED_PT) || \
	defined(CONFIG_FB_MSM_LCDC_AUO_WVGA))

static int lcdc_spi_gpio_array_num[] = {
	LCDC_SPI_GPIO_CLK,
	LCDC_SPI_GPIO_CS,
	LCDC_SPI_GPIO_MOSI,
};

static uint32_t lcdc_spi_gpio_config_data[] = {
	GPIO_CFG(LCDC_SPI_GPIO_CLK, 0,
			GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
	GPIO_CFG(LCDC_SPI_GPIO_CS, 0,
			GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
	GPIO_CFG(LCDC_SPI_GPIO_MOSI, 0,
			GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
};

static void lcdc_config_spi_gpios(int enable)
{
	int n;
	for (n = 0; n < ARRAY_SIZE(lcdc_spi_gpio_config_data); ++n)
		gpio_tlmm_config(lcdc_spi_gpio_config_data[n], 0);
}
#endif

#ifdef CONFIG_FB_MSM_HDMI_MSM_PANEL
static struct resource hdmi_msm_resources[] = {
	{
		.name  = "hdmi_msm_qfprom_addr",
		.start = 0x00700000,
		.end   = 0x007060FF,
		.flags = IORESOURCE_MEM,
	},
	{
		.name  = "hdmi_msm_hdmi_addr",
		.start = 0x04A00000,
		.end   = 0x04A00FFF,
		.flags = IORESOURCE_MEM,
	},
	{
		.name  = "hdmi_msm_irq",
		.start = HDMI_IRQ,
		.end   = HDMI_IRQ,
		.flags = IORESOURCE_IRQ,
	},
};

static int hdmi_enable_5v(int on);
static int hdmi_core_power(int on, int show);
static int hdmi_cec_power(int on);

static struct msm_hdmi_platform_data hdmi_msm_data = {
	.irq = HDMI_IRQ,
	.enable_5v = hdmi_enable_5v,
	.core_power = hdmi_core_power,
	.cec_power = hdmi_cec_power,
};

static struct platform_device hdmi_msm_device = {
	.name = "hdmi_msm",
	.id = 0,
	.num_resources = ARRAY_SIZE(hdmi_msm_resources),
	.resource = hdmi_msm_resources,
	.dev.platform_data = &hdmi_msm_data,
};
#endif /* CONFIG_FB_MSM_HDMI_MSM_PANEL */

static void __init msm8x60_allocate_memory_regions(void)
{
	void *addr;
	unsigned long size;

	if (hdmi_is_primary)
		size = roundup((1920 * 1088 * 4 * 2), 4096);
	else
		size = MSM_FB_SIZE;

	addr = alloc_bootmem_align(size, 0x1000);
	msm_fb_resources[0].start = __pa(addr);
	msm_fb_resources[0].end = msm_fb_resources[0].start + size - 1;
	pr_info("allocating %lu bytes at %p (%lx physical) for fb\n",
		size, addr, __pa(addr));

}

void __init msm8x60_set_display_params(char *prim_panel, char *ext_panel)
{
	if (strnlen(prim_panel, PANEL_NAME_MAX_LEN)) {
		strlcpy(msm_fb_pdata.prim_panel_name, prim_panel,
			PANEL_NAME_MAX_LEN);
		pr_debug("msm_fb_pdata.prim_panel_name %s\n",
			msm_fb_pdata.prim_panel_name);

		if (!strncmp((char *)msm_fb_pdata.prim_panel_name,
			HDMI_PANEL_NAME, strnlen(HDMI_PANEL_NAME,
				PANEL_NAME_MAX_LEN))) {
			pr_debug("HDMI is the primary display by"
				" boot parameter\n");
			hdmi_is_primary = 1;
		}
	}
	if (strnlen(ext_panel, PANEL_NAME_MAX_LEN)) {
		strlcpy(msm_fb_pdata.ext_panel_name, ext_panel,
			PANEL_NAME_MAX_LEN);
		pr_debug("msm_fb_pdata.ext_panel_name %s\n",
			msm_fb_pdata.ext_panel_name);
	}
}

#ifdef CONFIG_TOUCHSCREEN_CLEARPAD
static struct regulator *vreg_touch_vdd;

static int clearpad_vreg_low_power_mode(int enable)
{
	int rc = 0;

	if (IS_ERR(vreg_touch_vdd)) {
		pr_err("%s: vreg_touch_vdd is not initialized\n", __func__);
		return -ENODEV;
	}

	if (enable)
		rc = regulator_set_optimum_mode(vreg_touch_vdd, 1000);
	else
		rc = regulator_set_optimum_mode(vreg_touch_vdd, 15000);

	if (rc < 0) {
		pr_err("%s: vdd: set mode (%s) failed, rc=%d\n",
			__func__, (enable ? "LPM" : "HPM"), rc);
		return rc;
	} else {
		pr_debug("%s: vdd: set mode (%s) ok, new mode=%d\n",
			__func__, (enable ? "LPM" : "HPM"), rc);
		return 0;
	}
}

static int clearpad_vreg_configure(int enable)
{
	int rc = 0;

	vreg_touch_vdd = regulator_get(NULL, "8901_l1");
	if (IS_ERR(vreg_touch_vdd)) {
		pr_err("%s: get vdd failed\n", __func__);
		return -ENODEV;
	}

	if (enable) {
		rc = regulator_set_voltage(vreg_touch_vdd, 3050000, 3050000);
		if (rc) {
			pr_err("%s: set voltage failed, rc=%d\n", __func__, rc);
			goto clearpad_vreg_configure_err;
		}
		rc = regulator_enable(vreg_touch_vdd);
		if (rc) {
			pr_err("%s: enable vdd failed, rc=%d\n", __func__, rc);
			goto clearpad_vreg_configure_err;
		}
		rc = clearpad_vreg_low_power_mode(0);
		if (rc) {
			pr_err("%s: set vdd mode failed, rc=%d\n",
				__func__, rc);
			goto clearpad_vreg_configure_err;
		}
	} else {
		rc = regulator_set_voltage(vreg_touch_vdd, 0, 3050000);
		if (rc) {
			pr_err("%s: set voltage failed, rc=%d\n", __func__, rc);
			goto clearpad_vreg_configure_err;
		}
		rc = regulator_disable(vreg_touch_vdd);
		if (rc)
			pr_err("%s: disable vdd failed, rc=%d\n",
								__func__, rc);
	}
	return rc;
clearpad_vreg_configure_err:
	regulator_put(vreg_touch_vdd);
	return rc;
}

static int clearpad_vreg_reset(void)
{
	int rc = 0;

	rc = clearpad_vreg_configure(0);
	if (rc)
		return rc;
	usleep(20000);
	rc = clearpad_vreg_configure(1);
	return rc;
}

#define SYNAPTICS_TOUCH_GPIO_IRQ	(127)
static int clearpad_gpio_configure(int enable)
{
	int rc = 0;

	if (enable) {
		rc = gpio_request(SYNAPTICS_TOUCH_GPIO_IRQ, CLEARPAD_NAME);
		if (rc)
			pr_err("%s: gpio_requeset failed, "
					"rc=%d\n", __func__, rc);
	} else {
		gpio_free(SYNAPTICS_TOUCH_GPIO_IRQ);
	}
	return rc;
}

static struct clearpad_platform_data clearpad_platform_data = {
	.irq = MSM_GPIO_TO_INT(SYNAPTICS_TOUCH_GPIO_IRQ),
	.funcarea_get = clearpad_funcarea_get,
	.vreg_configure = clearpad_vreg_configure,
	.vreg_suspend = clearpad_vreg_low_power_mode,
	.vreg_reset = clearpad_vreg_reset,
	.gpio_configure = clearpad_gpio_configure,
};

#ifdef CONFIG_NFC_PN544
int pn544_chip_config(enum pn544_state state, void *not_used)
{
	switch (state) {
	case PN544_STATE_OFF:
		gpio_set_value_cansleep(
			PM8058_GPIO_PM_TO_SYS(PMIC_GPIO_NFC_FWDL_EN - 1), 0);
		gpio_set_value_cansleep(
			PM8058_GPIO_PM_TO_SYS(PMIC_GPIO_NFC_EN - 1), 0);
		usleep(50000);
		break;
	case PN544_STATE_ON:
		gpio_set_value_cansleep(
			PM8058_GPIO_PM_TO_SYS(PMIC_GPIO_NFC_FWDL_EN - 1), 0);
		gpio_set_value_cansleep(
			PM8058_GPIO_PM_TO_SYS(PMIC_GPIO_NFC_EN - 1), 1);
		usleep(10000);
		break;
	case PN544_STATE_FWDL:
		gpio_set_value_cansleep(
			PM8058_GPIO_PM_TO_SYS(PMIC_GPIO_NFC_FWDL_EN - 1), 1);
		gpio_set_value_cansleep(
			PM8058_GPIO_PM_TO_SYS(PMIC_GPIO_NFC_EN - 1), 0);
		usleep(10000);
		gpio_set_value_cansleep(
			PM8058_GPIO_PM_TO_SYS(PMIC_GPIO_NFC_EN - 1), 1);
		break;
	default:
		pr_err("%s: undefined state %d\n", __func__, state);
		return -EINVAL;
	}
	return 0;
}

int pn544_gpio_request(void)
{
	int ret;

	ret = gpio_request(
		PM8058_GPIO_PM_TO_SYS(PMIC_GPIO_NFC_IRQ - 1), "pn544_irq");
	if (ret)
		goto err_irq;
	ret = gpio_request(
		PM8058_GPIO_PM_TO_SYS(PMIC_GPIO_NFC_EN - 1), "pn544_ven");
	if (ret)
		goto err_ven;
	ret = gpio_request(
		PM8058_GPIO_PM_TO_SYS(PMIC_GPIO_NFC_FWDL_EN - 1), "pn544_fw");
	if (ret)
		goto err_fw;
	return 0;
err_fw:
	gpio_free(PM8058_GPIO_PM_TO_SYS(PMIC_GPIO_NFC_EN - 1));
err_ven:
	gpio_free(PM8058_GPIO_PM_TO_SYS(PMIC_GPIO_NFC_IRQ - 1));
err_irq:
	pr_err("%s: gpio request err %d\n", __func__, ret);
	return ret;
}

void pn544_gpio_release(void)
{
	gpio_free(PM8058_GPIO_PM_TO_SYS(PMIC_GPIO_NFC_EN - 1));
	gpio_free(PM8058_GPIO_PM_TO_SYS(PMIC_GPIO_NFC_IRQ - 1));
	gpio_free(PM8058_GPIO_PM_TO_SYS(PMIC_GPIO_NFC_FWDL_EN - 1));
}
#endif

static struct i2c_board_info gsbi3_peripherals_info[] __initdata = {
	{
		I2C_BOARD_INFO(CLEARPADI2C_NAME, 0x58 >> 1),
		.platform_data = &clearpad_platform_data,
	}
};
#endif /* CONFIG_TOUCHSCREEN_CLEARPAD */



#define VREG8_VDDIO_VOLTAGE 1800000
#define VREG10_VDDIO_VOLTAGE 2850000
#define SHARED_VREG_ON 1
#define SHARED_VREG_OFF 0

static void shared_vreg_switch(int enable)
{
	int rc;
	static struct regulator *vreg_l8;
	static struct regulator *vreg_l10;

	if (IS_ERR_OR_NULL(vreg_l10)) {
		vreg_l10 = regulator_get(NULL, "8058_l10");
		if (IS_ERR(vreg_l10)) {
			pr_err("%s: unable to get 8058_l10\n", __func__);
			return;
		}
	}
	if (IS_ERR_OR_NULL(vreg_l8)) {
		vreg_l8 = regulator_get(NULL, "8058_l8");
		if (IS_ERR(vreg_l8)) {
			pr_err("%s: unable to get 8058_l8\n", __func__);
			goto exit_free_reg_10;
		}
	}

	if (enable) {
		rc = regulator_set_voltage(vreg_l10, VREG10_VDDIO_VOLTAGE,
					VREG10_VDDIO_VOLTAGE);
		if (rc) {
			pr_err("%s: regulator_set_voltage failed\n", __func__);
			goto exit_free_reg_10;
		}
		rc = regulator_enable(vreg_l10);
		if (rc) {
			pr_err("%s: vreg_enable failed\n", __func__);
			goto exit_free_reg_10;
		}
		mdelay(10); /* Device Spec */
		rc = regulator_set_voltage(vreg_l8, VREG8_VDDIO_VOLTAGE,
					VREG8_VDDIO_VOLTAGE);
		if (rc) {
			pr_err("%s: regulator_set_voltage failed\n", __func__);
			goto exit_free_reg_8;
		}
		rc = regulator_enable(vreg_l8);
		if (rc) {
			pr_err("%s: vreg_enable failed\n", __func__);
			goto exit_free_reg_8;
		}
	} else {
		if (regulator_is_enabled(vreg_l8)) {
			rc = regulator_disable(vreg_l8);
			if (rc) {
				pr_err("%s: vreg_disable failed\n", __func__);
				regulator_put(vreg_l8);
				vreg_l8 = NULL;
			}
		}
		mdelay(1); /* Device Spec */
		if (regulator_is_enabled(vreg_l10)) {
			rc = regulator_disable(vreg_l10);
			if (rc) {
				pr_err("%s: vreg_disable failed\n", __func__);
				goto exit_free_reg_10;
			}
		}
	}
	return;

exit_free_reg_8:
	regulator_put(vreg_l8);
	vreg_l8 = NULL;
exit_free_reg_10:
	regulator_put(vreg_l10);
	vreg_l10 = NULL;
	return;
}

static void __init shared_vreg_init(void)
{
	shared_vreg_switch(SHARED_VREG_ON);
}

static int shared_vreg_exit
	(struct notifier_block *this, unsigned long code, void *_cmd)
{
	shared_vreg_switch(SHARED_VREG_OFF);
	return NOTIFY_DONE;
}

static struct notifier_block vreg_poweroff_notifier = {
	.notifier_call = shared_vreg_exit,
};

static int bma250_gpio_setup(struct device *dev)
{
	return 0;
}

static void bma250_gpio_teardown(struct device *dev)
{
	return;
}

static void bma250_hw_config(int enable)
{
	return;
}

static void bma250_power_mode(int enable)
{
	return;
}

static struct registers bma250_reg_setup = {
	.range                = BMA250_RANGE_2G,
	.bw_sel               = BMA250_BW_250HZ,
};

static struct bma250_platform_data bma250_platform_data = {
	.setup                = bma250_gpio_setup,
	.teardown             = bma250_gpio_teardown,
	.hw_config            = bma250_hw_config,
	.power_mode           = bma250_power_mode,
	.reg                  = &bma250_reg_setup,
	.bypass_state         = mpu3050_bypassmode,
	.read_axis_data       = bma250_read_axis_from_mpu3050,
	.check_sleep_status   = check_bma250_sleep_state,
	.vote_sleep_status    = vote_bma250_sleep_state,
	.rate                 = BMA250_DEFAULT_RATE,
};

#define APDS9702_DOUT_GPIO   34

static int apds9702_gpio_setup(int request)
{
	if (request) {
		return gpio_request(APDS9702_DOUT_GPIO, "apds9702_dout");
	} else {
		gpio_free(APDS9702_DOUT_GPIO);
		return 0;
	}
}

static void apds9702_hw_config(int enable)
{
	return;
}

static void apds9702_power_mode(int enable)
{
	return;
}

static struct apds9702_platform_data apds9702_pdata = {
	.gpio_dout      = APDS9702_DOUT_GPIO,
	.is_irq_wakeup  = 1,
	.hw_config      = apds9702_hw_config,
	.power_mode     = apds9702_power_mode,
	.gpio_setup     = apds9702_gpio_setup,
	.ctl_reg = {
		.trg   = 1,
		.pwr   = 1,
		.burst = 7,
		.frq   = 3,
		.dur   = 2,
		.th    = 15,
		.rfilt = 0,
	},
	.phys_dev_path = "/sys/devices/i2c-3/3-0054"
};

static int akm897x_gpio_setup(void)
{
	int rc;

	rc = gpio_request(AKM897X_GPIO, GPIO_FUNC_NAME);
	if (rc)
		pr_err("%s: gpio_request failed rc=%d\n", __func__, rc);
	return rc;
}

static void akm897x_gpio_shutdown(void)
{
	gpio_free(AKM897X_GPIO);
}

static void akm897x_hw_config(int enable)
{
	return;
}

static void akm897x_power_mode(int enable)
{
	return;
}

#ifdef CONFIG_INPUT_AKM8975
static struct akm8975_platform_data akm8975_platform_data = {
	.setup = akm897x_gpio_setup,
	.shutdown = akm897x_gpio_shutdown,
	.hw_config = akm897x_hw_config,
};
#endif
#ifdef CONFIG_INPUT_AKM8972
static struct akm8972_platform_data akm8972_platform_data = {
	.setup = akm897x_gpio_setup,
	.shutdown = akm897x_gpio_shutdown,
	.hw_config = akm897x_hw_config,
	.power_mode = akm897x_power_mode,
};
#endif

#ifdef CONFIG_SENSORS_MPU3050
int mpu3050_gpio_setup(struct device *dev, int enable)
{

	int rc = 0;

	if (enable) {
		rc = gpio_request(MPU3050_GPIO, "MPUIRQ");
		if (rc)
			pr_err("%s: gpio_request failed. rc=%d\n",
					__func__, rc);
	} else {
		gpio_free(MPU3050_GPIO);
	}

	return rc;
}

void mpu3050_hw_config(int enable)
{
	return;
}

void mpu3050_power_mode(int enable)
{
	return;
}
#endif /* CONFIG_SENSORS_MPU3050 */

#ifdef CONFIG_I2C
static struct i2c_board_info fuji_gsbi8_peripherals_info[] __initdata = {
	{
		I2C_BOARD_INFO(APDS9702_NAME, 0xA8 >> 1),
		.platform_data = &apds9702_pdata,
		.type = APDS9702_NAME,
	},
	{
		I2C_BOARD_INFO("as3676", 0x80 >> 1),
		.platform_data = &as3676_platform_data,
	},
	{
		I2C_BOARD_INFO("lm3560", 0xA6 >> 1),
		.platform_data = &lm3560_platform_data,
	},
#ifdef CONFIG_CHARGER_BQ24185
	{
		I2C_BOARD_INFO(BQ24185_NAME, 0xD6 >> 1),
		.irq = MSM_GPIO_TO_INT(BQ24185_GPIO_IRQ),
		.platform_data = &bq24185_platform_data,
		.type = BQ24185_NAME,
	},
#endif
#ifdef CONFIG_CHARGER_BQ24160
	{
		I2C_BOARD_INFO(BQ24160_NAME, 0xD6 >> 1),
		.irq = MSM_GPIO_TO_INT(BQ24160_GPIO_IRQ),
		.platform_data = &bq24160_platform_data,
		.type = BQ24160_NAME,
	},
#endif
	{
		I2C_BOARD_INFO(BQ27520_NAME, 0xAA >> 1),
		.irq = MSM_GPIO_TO_INT(GPIO_BQ27520_SOC_INT),
		.platform_data = &bq27520_platform_data,
		.type = BQ27520_NAME,
	},
#ifdef CONFIG_NFC_PN544
	{
		I2C_BOARD_INFO(PN544_DEVICE_NAME, 0x50 >> 1),
		.irq = PM8058_GPIO_IRQ(PM8058_IRQ_BASE, PMIC_GPIO_NFC_IRQ - 1),
		.platform_data = &pn544_pdata,
	},
#endif
};
#endif

static struct i2c_board_info fuji_gsbi12_peripherals_info[] __initdata = {
	{
		I2C_BOARD_INFO("bma250", 0x30 >> 1),
		.irq = MSM_GPIO_TO_INT(BMA250_GPIO),
		.platform_data = &bma250_platform_data,
	},
	{
#ifdef CONFIG_INPUT_AKM8975
		I2C_BOARD_INFO(AKM8975_I2C_NAME, 0x18 >> 1),
		.irq = MSM_GPIO_TO_INT(AKM897X_GPIO),
		.platform_data = &akm8975_platform_data,
#endif
#ifdef CONFIG_INPUT_AKM8972
		I2C_BOARD_INFO(AKM8972_I2C_NAME, 0x18 >> 1),
		.irq = MSM_GPIO_TO_INT(AKM897X_GPIO),
		.platform_data = &akm8972_platform_data,
#endif
	},
#ifdef CONFIG_SENSORS_MPU3050
	{
		I2C_BOARD_INFO(MPU_NAME, 0xD0 >> 1),
		.irq = MSM_GPIO_TO_INT(MPU3050_GPIO),
		.platform_data = &mpu_data,
	},
#endif
};


#ifdef CONFIG_MSM_GSBI5_UART
static struct msm_serial_hslite_platform_data msm_uart_gsbi5_pdata = {
	.config_gpio	= 1,
	.uart_tx_gpio	= 49,
	.uart_rx_gpio	= 50,
	.type		= PORT_IRDA,
};
#endif

#ifdef CONFIG_MSM_GSBI7_UART
static int felica_uart_pre_startup(struct uart_port *uport)
{
	struct tty_struct *tty = uport->state->port.tty;

	mutex_lock(&tty->termios_mutex);
	tty->termios->c_ispeed = 460800;
	tty->termios->c_ospeed = 460800;
	tty->termios->c_cflag |= B460800;
	tty->termios->c_cflag &= ~0xD;
	tty->termios->c_cflag |= (CLOCAL | CREAD);
	tty->termios->c_cflag &= ~PARENB;
	tty->termios->c_cflag &= ~CSTOPB;
	tty->termios->c_cflag &= ~CSIZE;
	tty->termios->c_cflag &= ~PARODD;
	tty->termios->c_cflag |= CS8;
	tty->termios->c_lflag &= ~(ICANON | IEXTEN | ISIG | ECHO);
	tty->termios->c_oflag &= ~OPOST;
	tty->termios->c_iflag &= ~(ICRNL | INPCK | ISTRIP |
						IXON | BRKINT);
	tty->termios->c_cc[VMIN] = 0;
	tty->termios->c_cc[VTIME] = 0;
	tty->ldisc->ops->set_termios(tty, NULL);
	mutex_unlock(&tty->termios_mutex);

	return 0;
}

static int configure_uart_dm7_gpios(int on)
{
	int ret = 0, i;
	int uart_gpios[] = {57, 58};

	for (i = 0; i < ARRAY_SIZE(uart_gpios); i++) {
		if (on) {
			ret = msm_gpiomux_get(uart_gpios[i]);
			if (unlikely(ret))
				break;
		} else {
			ret = msm_gpiomux_put(uart_gpios[i]);
			if (unlikely(ret))
				return ret;
		}
	}
	if (ret)
		for (; i >= 0; i--)
			msm_gpiomux_put(uart_gpios[i]);
	return ret;
}

static struct msm_serial_hs_platform_data msm_uart_dm7_pdata = {
	.gpio_config = configure_uart_dm7_gpios,
	.inject_rx_on_wakeup = 0,
	.pre_startup	= felica_uart_pre_startup,
};
#endif

static struct msm_serial_hs_platform_data msm_uart_dm1_pdata = {
	.inject_rx_on_wakeup = 0,
	.exit_lpm_cb = bcm_bt_lpm_exit_lpm_locked,
};
static struct platform_device msm_bt_power_device = {
	.name = "bt_power",
};
static int bluetooth_power(int on)
{
	int ret = 0, i;
	int bt_power_gpios[] = {53, 54, 55, 56};
	/*	already configured, return */
	if (on == gpio_get_value(BT_GPIO_EN))
		return 0;

	if (on) {
		for (i = 0; i < ARRAY_SIZE(bt_power_gpios); i++) {
			ret = msm_gpiomux_get(bt_power_gpios[i]);
			if (unlikely(ret))
				goto error;
		}
	} else {
		for (i = 0; i < ARRAY_SIZE(bt_power_gpios); i++)
			msm_gpiomux_put(bt_power_gpios[i]);
	}

	gpio_set_value(BT_GPIO_EN, on);
	return 0;

error:
	for (; i >= 0; i--)
		msm_gpiomux_put(bt_power_gpios[i]);
	return ret;
}

static void __init bt_power_init(void)
{
	int ret = 0, i;
	int bt_gpios[] = {23, 137, 63};
	for (i = 0; i < ARRAY_SIZE(bt_gpios); i++) {
		ret = msm_gpiomux_get(bt_gpios[i]);
		if (unlikely(ret))
			goto error;
	}
	gpio_set_value(BT_GPIO_EN, 0);
	msm_bt_power_device.dev.platform_data = &bluetooth_power;
	return;
error:
	for (; i >= 0; i--)
		msm_gpiomux_put(bt_gpios[i]);
}

static struct bcm_bt_lpm_platform_data bcm_bt_lpm_pdata = {
	.gpio_wake = BT_GPIO_WAKE,
	.gpio_host_wake = BT_GPIO_HOST_WAKE,
	.request_clock_off_locked = msm_hs_request_clock_off_locked,
	.request_clock_on_locked = msm_hs_request_clock_on_locked,
};

struct platform_device bcm_bt_lpm_device = {
	.name = "bcm_bt_lpm",
	.id = 0,
	.dev = {
		.platform_data = &bcm_bt_lpm_pdata,
	},
};

#if defined(CONFIG_MSM_RPM_LOG) || defined(CONFIG_MSM_RPM_LOG_MODULE)

static struct msm_rpm_log_platform_data msm_rpm_log_pdata = {
	.phys_addr_base = 0x00106000,
	.reg_offsets = {
		[MSM_RPM_LOG_PAGE_INDICES] = 0x00000C80,
		[MSM_RPM_LOG_PAGE_BUFFER]  = 0x00000CA0,
	},
	.phys_size = SZ_8K,
	.log_len = 4096,		  /* log's buffer length in bytes */
	.log_len_mask = (4096 >> 2) - 1,  /* length mask in units of u32 */
};

static struct platform_device msm_rpm_log_device = {
	.name	= "msm_rpm_log",
	.id	= -1,
	.dev	= {
		.platform_data = &msm_rpm_log_pdata,
	},
};
#endif

#define CONSOLE_NAME "ttyHSL"
#define CONSOLE_IX 0
#define CONSOLE_OPTIONS "115200,n8"
static int __init setup_serial_console(char *console_flag)
{
	if (console_flag &&
		strlen(console_flag) >= 2 &&
		(console_flag[0] != '0' || console_flag[1] != '0'))
		add_preferred_console(CONSOLE_NAME,
			CONSOLE_IX,
			CONSOLE_OPTIONS);
	return 1;
}

/*
* The S1 Boot configuration TA unit can specify that the serial console
* enable flag will be passed as Kernel boot arg with tag babe09A9.
*/
__setup("semcandroidboot.babe09a9=", setup_serial_console);

static struct regulator_consumer_supply vreg_consumers_PM8058_S0[] = {
	REGULATOR_SUPPLY("8058_s0",		NULL),
};
static struct regulator_consumer_supply vreg_consumers_PM8058_S1[] = {
	REGULATOR_SUPPLY("8058_s1",		NULL),
};

/* RPM early regulator constraints */
static struct rpm_regulator_init_data rpm_regulator_early_init_data[] = {
	/*	 ID       a_on pd ss min_uV   max_uV   init_ip    freq */
	RPM_SMPS(PM8058_S0, 0, 1, 1,  500000, 1325000, SMPS_HMIN, 1p60),
	RPM_SMPS(PM8058_S1, 0, 1, 1,  500000, 1250000, SMPS_HMIN, 1p60),
};

static struct rpm_regulator_platform_data rpm_regulator_early_pdata = {
	.init_data		= rpm_regulator_early_init_data,
	.num_regulators		= ARRAY_SIZE(rpm_regulator_early_init_data),
	.version		= RPM_VREG_VERSION_8660,
	.vreg_id_vdd_mem	= RPM_VREG_ID_PM8058_S0,
	.vreg_id_vdd_dig	= RPM_VREG_ID_PM8058_S1,
};

static struct platform_device rpm_regulator_early_device = {
	.name	= "rpm-regulator",
	.id	= 0,
	.dev	= {
		.platform_data = &rpm_regulator_early_pdata,
	},
};

static struct platform_device *early_regulators[] __initdata = {
	&msm_device_saw_s0,
	&msm_device_saw_s1,
	&rpm_regulator_early_device,
};

static struct platform_device *early_devices[] __initdata = {
#ifdef CONFIG_MSM_BUS_SCALING
	&msm_bus_apps_fabric,
	&msm_bus_sys_fabric,
	&msm_bus_mm_fabric,
	&msm_bus_sys_fpb,
	&msm_bus_cpss_fpb,
#endif
	&msm_device_dmov_adm0,
	&msm_device_dmov_adm1,
};

static struct platform_device msm_tsens_device = {
	.name   = "tsens-tm",
	.id = -1,
};

#ifdef CONFIG_SENSORS_MSM_ADC

static struct adc_access_fn xoadc_fn = {
	pm8058_xoadc_select_chan_and_start_conv,
	pm8058_xoadc_read_adc_code,
	pm8058_xoadc_get_properties,
	pm8058_xoadc_slot_request,
	pm8058_xoadc_restore_slot,
	pm8058_xoadc_calibrate,
};

static struct msm_adc_channels msm_adc_channels_data[] = {
	{"vbatt", CHANNEL_ADC_VBATT, 0, &xoadc_fn, CHAN_PATH_TYPE2,
		ADC_CONFIG_TYPE2, ADC_CALIB_CONFIG_TYPE3, scale_default},
	{"vcoin", CHANNEL_ADC_VCOIN, 0, &xoadc_fn, CHAN_PATH_TYPE1,
		ADC_CONFIG_TYPE2, ADC_CALIB_CONFIG_TYPE2, scale_default},
	{"vcharger_channel", CHANNEL_ADC_VCHG, 0, &xoadc_fn, CHAN_PATH_TYPE3,
		ADC_CONFIG_TYPE2, ADC_CALIB_CONFIG_TYPE4, scale_default},
	{"charger_current_monitor", CHANNEL_ADC_CHG_MONITOR, 0, &xoadc_fn,
		CHAN_PATH_TYPE4,
		ADC_CONFIG_TYPE2, ADC_CALIB_CONFIG_TYPE1, scale_default},
	{"vph_pwr", CHANNEL_ADC_VPH_PWR, 0, &xoadc_fn, CHAN_PATH_TYPE5,
		ADC_CONFIG_TYPE2, ADC_CALIB_CONFIG_TYPE3, scale_default},
	{"usb_vbus", CHANNEL_ADC_USB_VBUS, 0, &xoadc_fn, CHAN_PATH_TYPE11,
		ADC_CONFIG_TYPE2, ADC_CALIB_CONFIG_TYPE3, scale_default},
	{"pmic_therm", CHANNEL_ADC_DIE_TEMP, 0, &xoadc_fn, CHAN_PATH_TYPE12,
		ADC_CONFIG_TYPE2, ADC_CALIB_CONFIG_TYPE1, scale_pmic_therm},
	{"pmic_therm_4K", CHANNEL_ADC_DIE_TEMP_4K, 0, &xoadc_fn,
		CHAN_PATH_TYPE12,
		ADC_CONFIG_TYPE1, ADC_CALIB_CONFIG_TYPE7, scale_pmic_therm},
	{"xo_therm", CHANNEL_ADC_XOTHERM, 0, &xoadc_fn, CHAN_PATH_TYPE_NONE,
		ADC_CONFIG_TYPE2, ADC_CALIB_CONFIG_TYPE5, tdkntcgtherm},
	{"xo_therm_4K", CHANNEL_ADC_XOTHERM_4K, 0, &xoadc_fn,
		CHAN_PATH_TYPE_NONE,
		ADC_CONFIG_TYPE1, ADC_CALIB_CONFIG_TYPE6, tdkntcgtherm},
	{"hdset_detect", CHANNEL_ADC_HDSET, 0, &xoadc_fn, CHAN_PATH_TYPE6,
		ADC_CONFIG_TYPE2, ADC_CALIB_CONFIG_TYPE1, scale_default},
	{"chg_batt_amon", CHANNEL_ADC_BATT_AMON, 0, &xoadc_fn, CHAN_PATH_TYPE10,
		ADC_CONFIG_TYPE2, ADC_CALIB_CONFIG_TYPE1,
		scale_xtern_chgr_cur},
	{"msm_therm", CHANNEL_ADC_MSM_THERM, 0, &xoadc_fn, CHAN_PATH_TYPE8,
		ADC_CONFIG_TYPE2, ADC_CALIB_CONFIG_TYPE2, scale_msm_therm},
	{"batt_therm", CHANNEL_ADC_BATT_THERM, 0, &xoadc_fn, CHAN_PATH_TYPE7,
		ADC_CONFIG_TYPE2, ADC_CALIB_CONFIG_TYPE2, scale_batt_therm},
	{"batt_id", CHANNEL_ADC_BATT_ID, 0, &xoadc_fn, CHAN_PATH_TYPE9,
		ADC_CONFIG_TYPE2, ADC_CALIB_CONFIG_TYPE2, scale_default},
	{"ref_625mv", CHANNEL_ADC_625_REF, 0, &xoadc_fn, CHAN_PATH_TYPE15,
		ADC_CONFIG_TYPE2, ADC_CALIB_CONFIG_TYPE2, scale_default},
	{"ref_1250mv", CHANNEL_ADC_1250_REF, 0, &xoadc_fn, CHAN_PATH_TYPE13,
		ADC_CONFIG_TYPE2, ADC_CALIB_CONFIG_TYPE2, scale_default},
	{"ref_325mv", CHANNEL_ADC_325_REF, 0, &xoadc_fn, CHAN_PATH_TYPE14,
		ADC_CONFIG_TYPE2, ADC_CALIB_CONFIG_TYPE2, scale_default},
};

static struct msm_adc_platform_data msm_adc_pdata = {
	.channel = msm_adc_channels_data,
	.num_chan_supported = ARRAY_SIZE(msm_adc_channels_data),
};

static struct platform_device msm_adc_device = {
	.name   = "msm_adc",
	.id = -1,
	.dev = {
		.platform_data = &msm_adc_pdata,
	},
};

static void pmic8058_xoadc_mpp_config(void)
{
	int rc, i;
	struct pm8xxx_mpp_init_info xoadc_mpps[] = {
		PM8058_MPP_INIT(XOADC_MPP_3, A_INPUT, PM8XXX_MPP_AIN_AMUX_CH5,
							AOUT_CTRL_DISABLE),
		PM8058_MPP_INIT(XOADC_MPP_5, A_INPUT, PM8XXX_MPP_AIN_AMUX_CH9,
							AOUT_CTRL_DISABLE),
		PM8058_MPP_INIT(XOADC_MPP_7, A_INPUT, PM8XXX_MPP_AIN_AMUX_CH6,
							AOUT_CTRL_DISABLE),
		PM8058_MPP_INIT(XOADC_MPP_8, A_INPUT, PM8XXX_MPP_AIN_AMUX_CH8,
							AOUT_CTRL_DISABLE),
		PM8058_MPP_INIT(XOADC_MPP_10, A_INPUT, PM8XXX_MPP_AIN_AMUX_CH7,
							AOUT_CTRL_DISABLE),
		PM8901_MPP_INIT(XOADC_MPP_4, D_OUTPUT, PM8901_MPP_DIG_LEVEL_S4,
							DOUT_CTRL_LOW),
	};

	for (i = 0; i < ARRAY_SIZE(xoadc_mpps); i++) {
		rc = pm8xxx_mpp_config(xoadc_mpps[i].mpp,
					&xoadc_mpps[i].config);
		if (rc) {
			pr_err("%s: Config MPP %d of PM8058 failed\n",
					__func__, xoadc_mpps[i].mpp);
		}
	}
}

static struct regulator *vreg_ldo18_adc;

static int pmic8058_xoadc_vreg_config(int on)
{
	int rc;

	if (on) {
		rc = regulator_enable(vreg_ldo18_adc);
		if (rc)
			pr_err("%s: Enable of regulator ldo18_adc "
						"failed\n", __func__);
	} else {
		rc = regulator_disable(vreg_ldo18_adc);
		if (rc)
			pr_err("%s: Disable of regulator ldo18_adc "
						"failed\n", __func__);
	}

	return rc;
}

static int pmic8058_xoadc_vreg_setup(void)
{
	int rc;

	vreg_ldo18_adc = regulator_get(NULL, "8058_l18");
	if (IS_ERR(vreg_ldo18_adc)) {
		printk(KERN_ERR "%s: vreg get failed (%ld)\n",
			__func__, PTR_ERR(vreg_ldo18_adc));
		rc = PTR_ERR(vreg_ldo18_adc);
		goto fail;
	}

	rc = regulator_set_voltage(vreg_ldo18_adc, 2200000, 2200000);
	if (rc) {
		pr_err("%s: unable to set ldo18 voltage to 2.2V\n", __func__);
		goto fail;
	}

	return rc;
fail:
	regulator_put(vreg_ldo18_adc);
	return rc;
}

static void pmic8058_xoadc_vreg_shutdown(void)
{
	regulator_put(vreg_ldo18_adc);
}

/* usec. For this ADC,
 * this time represents clk rate @ txco w/ 1024 decimation ratio.
 * Each channel has different configuration, thus at the time of starting
 * the conversion, xoadc will return actual conversion time
 * */
static struct adc_properties pm8058_xoadc_data = {
	.adc_reference          = 2200, /* milli-voltage for this adc */
	.bitresolution         = 15,
	.bipolar                = 0,
	.conversiontime         = 54,
};

static struct xoadc_platform_data pm8058_xoadc_pdata = {
	.xoadc_prop = &pm8058_xoadc_data,
	.xoadc_mpp_config = pmic8058_xoadc_mpp_config,
	.xoadc_vreg_set = pmic8058_xoadc_vreg_config,
	.xoadc_num = XOADC_PMIC_0,
	.xoadc_vreg_setup = pmic8058_xoadc_vreg_setup,
	.xoadc_vreg_shutdown = pmic8058_xoadc_vreg_shutdown,
};
#endif

#ifdef CONFIG_MSM_SDIO_AL

static unsigned mdm2ap_status = 140;

static int configure_mdm2ap_status(int on)
{
	int ret = 0;
	if (on)
		ret = msm_gpiomux_get(mdm2ap_status);
	else
		ret = msm_gpiomux_put(mdm2ap_status);

	if (ret)
		pr_err("%s: mdm2ap_status config failed, on = %d\n", __func__,
		       on);

	return ret;
}


static int get_mdm2ap_status(void)
{
	return gpio_get_value(mdm2ap_status);
}

static struct sdio_al_platform_data sdio_al_pdata = {
	.config_mdm2ap_status = configure_mdm2ap_status,
	.get_mdm2ap_status = get_mdm2ap_status,
	.allow_sdioc_version_major_2 = 0,
	.peer_sdioc_version_minor = 0x0202,
	.peer_sdioc_version_major = 0x0004,
	.peer_sdioc_boot_version_minor = 0x0001,
	.peer_sdioc_boot_version_major = 0x0003
};

struct platform_device msm_device_sdio_al = {
	.name = "msm_sdio_al",
	.id = -1,
	.dev		= {
		.parent = &msm_charm_modem.dev,
		.platform_data	= &sdio_al_pdata,
	},
};

#endif /* CONFIG_MSM_SDIO_AL */

static void *fuji_wifi_mem_prealloc(int section, unsigned long size)
{
	if (section == PREALLOC_WLAN_NUMBER_OF_SECTIONS)
		return wlan_static_skb;
	if (section < 0 || section > PREALLOC_WLAN_NUMBER_OF_SECTIONS)
		return NULL;
	if (wifi_mem_array[section].size < size)
		return NULL;
	return wifi_mem_array[section].mem_ptr;
}

int __init fuji_init_wifi_mem(void)
{
	int i;

	for (i = 0; i < WLAN_SKB_BUF_NUM; i++)
		wlan_static_skb[i] = dev_alloc_skb(4096);

	for (i = 0; i < PREALLOC_WLAN_NUMBER_OF_SECTIONS; i++) {
		wifi_mem_array[i].mem_ptr = kmalloc(wifi_mem_array[i].size,
							GFP_KERNEL);
		if (wifi_mem_array[i].mem_ptr == NULL)
			return -ENOMEM;
	}
	return 0;
}

static int fuji_wifi_cd;
static void (*wifi_status_cb)(int card_present, void *dev_id);
static void *wifi_status_cb_devid;

static int fuji_wifi_status_register(void (*callback)(int card_present, void *dev_id), void *dev_id)
{
	if (wifi_status_cb)
		return -EAGAIN;
	wifi_status_cb = callback;
	wifi_status_cb_devid = dev_id;
	return 0;
}

static unsigned int fuji_wifi_status(struct device *dev)
{
	return fuji_wifi_cd;
}

int fuji_wifi_set_carddetect(int val)
{
	fuji_wifi_cd = val;
	if (wifi_status_cb)
		wifi_status_cb(val, wifi_status_cb_devid);
	else
		printk(KERN_WARNING "%s: Nobody to notify\n", __func__);
	return 0;
}

struct wifi_platform_data fuji_wifi_control = {
	.set_carddetect = fuji_wifi_set_carddetect,
	.mem_prealloc	= fuji_wifi_mem_prealloc,
};

static int __init fuji_wifi_init(void)
{
	return fuji_init_wifi_mem();
}

late_initcall(fuji_wifi_init);

static struct platform_device fuji_wifi = {
	.name   = "bcm4330_wlan",
	.id	    = -1,
	.dev    = {
		.platform_data = &fuji_wifi_control,
	},
};

#ifdef CONFIG_FUJI_GPIO_KEYPAD
struct platform_device gpio_key_device = {
	.name	= GPIO_EVENT_DEV_NAME,
	.id	= -1,
	.dev	= {
		.platform_data	= &gpio_key_data,
	},
};
#endif /* CONFIG_FUJI_GPIO_KEYPAD */

#ifdef CONFIG_ANDROID_RAM_CONSOLE
#define MSM_RAM_CONSOLE_START   (0x80000000 - MSM_RAM_CONSOLE_SIZE)
#define MSM_RAM_CONSOLE_SIZE    (128 * SZ_1K)

static struct resource ram_console_resources[] = {
	[0] = {
		.start  = MSM_RAM_CONSOLE_START,
		.end    = MSM_RAM_CONSOLE_START + MSM_RAM_CONSOLE_SIZE - 1,
		.flags  = IORESOURCE_MEM,
	},
};

static struct platform_device ram_console_device = {
	.name           = "ram_console",
	.id             = -1,
	.num_resources  = ARRAY_SIZE(ram_console_resources),
	.resource       = ram_console_resources,
};
#endif

#ifdef CONFIG_RAMDUMP_CRASH_LOGS
#define MSM_RAMDUMP_INFO_START	0x7FF00000
#define MSM_RAMDUMP_INFO_SIZE	(4 * SZ_1K)
#define MSM_AMSS_LOG_START	(MSM_RAMDUMP_INFO_START + MSM_RAMDUMP_INFO_SIZE)
#define MSM_AMSS_LOG_SIZE	(16 * SZ_1K)

static struct resource ramdumplog_resources[] = {
	[0] = {
		.name   = "ramdumpinfo",
		.start  = MSM_RAMDUMP_INFO_START,
		.end    = MSM_RAMDUMP_INFO_START + MSM_RAMDUMP_INFO_SIZE - 1,
		.flags  = IORESOURCE_MEM,
	},
	[1] = {
		.name   = "amsslog",
		.start  = MSM_AMSS_LOG_START,
		.end    = MSM_AMSS_LOG_START + MSM_AMSS_LOG_SIZE - 1,
		.flags  = IORESOURCE_MEM,
	},
};

static struct platform_device ramdumplog_device = {
	.name           = "ramdumplog",
	.id             = -1,
	.num_resources  = ARRAY_SIZE(ramdumplog_resources),
	.resource       = ramdumplog_resources,
};
#endif

static struct platform_device *asoc_devices[] __initdata = {
	&asoc_msm_pcm,
	&asoc_msm_dai0,
	&asoc_msm_dai1,
};

static struct platform_device *fuji_devices[] __initdata = {
	&msm_device_smd,
	&msm_device_uart_dm12,
	&msm_pil_q6v3,
	&msm_pil_modem,
	&msm_pil_tzapps,
#ifdef CONFIG_I2C_QUP
	&msm_gsbi3_qup_i2c_device,
	&msm_gsbi4_qup_i2c_device,
#ifdef CONFIG_MSM_GSBI5_I2C
	&msm_gsbi5_qup_i2c_device,
#endif
	&msm_gsbi7_qup_i2c_device,
	&msm_gsbi8_qup_i2c_device,
	&msm_gsbi9_qup_i2c_device,
	&msm_gsbi12_qup_i2c_device,
#endif
	&msm_device_uart_dm1,
	&msm_bt_power_device,
	&bcm_bt_lpm_device,
#ifdef CONFIG_MSM_GSBI5_UART
	&msm_device_uart_gsbi5,
#endif
#ifdef CONFIG_SEMC_FELICA_SUPPORT
	&semc_felica_device,
#endif
#ifdef CONFIG_MSM_GSBI7_UART
	&msm_device_uart_dm7,
#endif
#ifdef CONFIG_MSM_SSBI
	&msm_device_ssbi_pmic1,
	&msm_device_ssbi_pmic2,
#endif
#ifdef CONFIG_I2C_SSBI
	&msm_device_ssbi3,
#endif

#if defined(CONFIG_USB_GADGET_MSM_72K) || defined(CONFIG_USB_EHCI_HCD)
	&msm_device_otg,
#endif
#ifdef CONFIG_USB_GADGET_MSM_72K
	&msm_device_gadget_peripheral,
#endif
#ifdef CONFIG_USB_G_ANDROID
	&android_usb_device,
#endif
#ifdef CONFIG_ANDROID_PMEM
#ifndef CONFIG_MSM_MULTIMEDIA_USE_ION
	&android_pmem_device,
	&android_pmem_adsp_device,
	&android_pmem_camera_device,
	&android_pmem_smipool_device,
#ifdef CONFIG_SEMC_SWIQI
	&android_pmem_swiqi_device,
#endif
#endif
	&android_pmem_audio_device,
#endif
#ifdef CONFIG_MSM_ROTATOR
	&msm_rotator_device,
#endif
	&msm_fb_device,
	&msm_kgsl_3d0,
	&msm_kgsl_2d0,
	&msm_kgsl_2d1,
#ifdef CONFIG_FB_MSM_HDMI_MSM_PANEL
	&hdmi_msm_device,
#endif /* CONFIG_FB_MSM_HDMI_MSM_PANEL */
#ifdef CONFIG_SEMC_CAM_MAIN
	&msm_camera_semc_sensor_main,
#endif
#ifdef CONFIG_SEMC_CAM_SUB
	&msm_camera_semc_sensor_sub,
#endif
#ifdef CONFIG_MSM_GEMINI
	&msm_gemini_device,
#endif
#ifdef CONFIG_MSM_VPE
	&msm_vpe_device,
#endif
#if defined(CONFIG_SEMC_VPE1)
	&semc_vpe1_device,
#endif

#if defined(CONFIG_MSM_RPM_LOG) || defined(CONFIG_MSM_RPM_LOG_MODULE)
	&msm_rpm_log_device,
#endif
	&msm_device_vidc,
#ifdef CONFIG_SENSORS_MSM_ADC
	&msm_adc_device,
#endif
	&rpm_regulator_device,

#if defined(CONFIG_CRYPTO_DEV_QCRYPTO) || \
		defined(CONFIG_CRYPTO_DEV_QCRYPTO_MODULE)
	&qcrypto_device,
#endif

#if defined(CONFIG_CRYPTO_DEV_QCEDEV) || \
		defined(CONFIG_CRYPTO_DEV_QCEDEV_MODULE)
	&qcedev_device,
#endif

#if defined(CONFIG_TSIF) || defined(CONFIG_TSIF_MODULE)
#ifdef CONFIG_MSM_USE_TSIF1
	&msm_device_tsif[1],
#else
	&msm_device_tsif[0],
#endif
#endif
#ifdef CONFIG_SEMC_ONESEG_TUNER_PM
	&oneseg_tunerpm_device,
#endif

#ifdef CONFIG_HW_RANDOM_MSM
	&msm_device_rng,
#endif
	&bdata_driver,
	&battery_chargalg_platform_device,
	&fuji_wifi,
#ifdef CONFIG_SEMC_CHARGER_CRADLE_ARCH
	&semc_chg_cradle,
#endif
	&msm_tsens_device,
	&msm_rpm_device,
#ifdef CONFIG_FUJI_GPIO_KEYPAD
	&gpio_key_device,
#endif /* CONFIG_FUJI_GPIO_KEYPAD */
#ifdef CONFIG_ION_MSM
	&ion_dev,
#endif
	&msm8660_device_watchdog,
#ifdef CONFIG_USB_NCP373
	&ncp373_device,
#endif
#ifdef CONFIG_ANDROID_RAM_CONSOLE
	&ram_console_device,
#endif
#ifdef CONFIG_RAMDUMP_CRASH_LOGS
	&ramdumplog_device,
#endif
#ifdef CONFIG_MSM_8x60_VOIP
	&asoc_msm_mvs,
	&asoc_mvs_dai0,
	&asoc_mvs_dai1,
#endif
};

#ifdef CONFIG_ION_MSM
#ifdef CONFIG_MSM_MULTIMEDIA_USE_ION
static struct ion_cp_heap_pdata cp_mm_ion_pdata = {
	.permission_type = IPT_TYPE_MM_CARVEOUT,
	.align = PAGE_SIZE,
	.request_region = request_smi_region,
	.release_region = release_smi_region,
	.setup_region = setup_smi_region,
};

static struct ion_cp_heap_pdata cp_mfc_ion_pdata = {
	.permission_type = IPT_TYPE_MFC_SHAREDMEM,
	.align = PAGE_SIZE,
	.request_region = request_smi_region,
	.release_region = release_smi_region,
	.setup_region = setup_smi_region,
};

static struct ion_cp_heap_pdata cp_wb_ion_pdata = {
	.permission_type = IPT_TYPE_MDP_WRITEBACK,
	.align = PAGE_SIZE,
};

static struct ion_co_heap_pdata fw_co_ion_pdata = {
	.adjacent_mem_id = ION_CP_MM_HEAP_ID,
	.align = SZ_128K,
};

static struct ion_co_heap_pdata co_ion_pdata = {
	.adjacent_mem_id = INVALID_HEAP_ID,
	.align = PAGE_SIZE,
};
#endif

/**
 * These heaps are listed in the order they will be allocated. Due to
 * video hardware restrictions and content protection the FW heap has to
 * be allocated adjacent (below) the MM heap and the MFC heap has to be
 * allocated after the MM heap to ensure MFC heap is not more than 256MB
 * away from the base address of the FW heap.
 * However, the order of FW heap and MM heap doesn't matter since these
 * two heaps are taken care of by separate code to ensure they are adjacent
 * to each other.
 * Don't swap the order unless you know what you are doing!
 */
static struct ion_platform_data ion_pdata = {
	.nr = MSM_ION_HEAP_NUM,
	.heaps = {
		{
			.id	= ION_SYSTEM_HEAP_ID,
			.type	= ION_HEAP_TYPE_SYSTEM,
			.name	= ION_VMALLOC_HEAP_NAME,
		},
#ifdef CONFIG_MSM_MULTIMEDIA_USE_ION
		{
			.id	= ION_CP_MM_HEAP_ID,
			.type   = ION_HEAP_TYPE_CP,
			.name	= ION_MM_HEAP_NAME,
			.size	= MSM_ION_MM_SIZE,
			.memory_type = ION_SMI_TYPE,
			.extra_data = (void *) &cp_mm_ion_pdata,
		},
		{
			.id     = ION_MM_FIRMWARE_HEAP_ID,
			.type   = ION_HEAP_TYPE_CARVEOUT,
			.name   = ION_MM_FIRMWARE_HEAP_NAME,
			.size   = MSM_ION_MM_FW_SIZE,
			.memory_type = ION_SMI_TYPE,
			.extra_data = (void *) &fw_co_ion_pdata,
		},
		{
			.id	= ION_CP_MFC_HEAP_ID,
			.type   = ION_HEAP_TYPE_CP,
			.name	= ION_MFC_HEAP_NAME,
			.size	= MSM_ION_MFC_SIZE,
			.memory_type = ION_SMI_TYPE,
			.extra_data = (void *) &cp_mfc_ion_pdata,
		},
		{
			.id	= ION_SF_HEAP_ID,
			.type	= ION_HEAP_TYPE_CARVEOUT,
			.name	= ION_SF_HEAP_NAME,
			.size	= MSM_ION_SF_SIZE,
			.memory_type = ION_EBI_TYPE,
			.extra_data = (void *)&co_ion_pdata,
		},
		{
			.id	= ION_CAMERA_HEAP_ID,
			.type	= ION_HEAP_TYPE_CARVEOUT,
			.name	= ION_CAMERA_HEAP_NAME,
			.size	= MSM_ION_CAMERA_SIZE,
			.memory_type = ION_EBI_TYPE,
			.extra_data = &co_ion_pdata,
		},
		{
			.id     = ION_CP_WB_HEAP_ID,
			.type   = ION_HEAP_TYPE_CP,
			.name   = ION_WB_HEAP_NAME,
			.size   = MSM_ION_WB_SIZE,
			.memory_type = ION_EBI_TYPE,
			.extra_data = (void *) &cp_wb_ion_pdata,
		},
		{
			.id	= ION_QSECOM_HEAP_ID,
			.type	= ION_HEAP_TYPE_CARVEOUT,
			.name	= ION_QSECOM_HEAP_NAME,
			.size	= MSM_ION_QSECOM_SIZE,
			.memory_type = ION_EBI_TYPE,
			.extra_data = (void *) &co_ion_pdata,
		},
		{
			.id	= ION_AUDIO_HEAP_ID,
			.type	= ION_HEAP_TYPE_CARVEOUT,
			.name	= ION_AUDIO_HEAP_NAME,
			.size	= MSM_ION_AUDIO_SIZE,
			.memory_type = ION_EBI_TYPE,
			.extra_data = (void *)&co_ion_pdata,
		},
#endif
	}
};

static struct platform_device ion_dev = {
	.name = "ion-msm",
	.id = 1,
	.dev = { .platform_data = &ion_pdata },
};
#endif

static struct memtype_reserve msm8x60_reserve_table[] __initdata = {
	/* Kernel SMI memory pool for video core, used for firmware */
	/* and encoder, decoder scratch buffers */
	/* Kernel SMI memory pool should always precede the user space */
	/* SMI memory pool, as the video core will use offset address */
	/* from the Firmware base */
	[MEMTYPE_SMI_KERNEL] = {
		.start	=	KERNEL_SMI_BASE,
		.limit	=	KERNEL_SMI_SIZE,
		.size	=	KERNEL_SMI_SIZE,
		.flags	=	MEMTYPE_FLAGS_FIXED,
	},
	/* User space SMI memory pool for video core */
	/* used for encoder, decoder input & output buffers  */
	[MEMTYPE_SMI] = {
		.start	=	USER_SMI_BASE,
		.limit	=	USER_SMI_SIZE,
		.flags	=	MEMTYPE_FLAGS_FIXED,
	},
	[MEMTYPE_EBI0] = {
		.flags	=	MEMTYPE_FLAGS_1M_ALIGN,
	},
	[MEMTYPE_EBI1] = {
		.flags	=	MEMTYPE_FLAGS_1M_ALIGN,
	},
};

static void reserve_ion_memory(void)
{
#if defined(CONFIG_ION_MSM) && defined(CONFIG_MSM_MULTIMEDIA_USE_ION)
	unsigned int i;

	if (hdmi_is_primary) {
		msm_ion_sf_size = MSM_HDMI_PRIM_ION_SF_SIZE;
		for (i = 0; i < ion_pdata.nr; i++) {
			if (ion_pdata.heaps[i].id == ION_SF_HEAP_ID) {
				ion_pdata.heaps[i].size = msm_ion_sf_size;
				pr_debug("msm_ion_sf_size 0x%x\n",
					msm_ion_sf_size);
				break;
			}
		}
	}

	msm8x60_reserve_table[MEMTYPE_EBI1].size += msm_ion_sf_size;
	msm8x60_reserve_table[MEMTYPE_SMI].size += MSM_ION_MM_FW_SIZE;
	msm8x60_reserve_table[MEMTYPE_SMI].size += MSM_ION_MM_SIZE;
	msm8x60_reserve_table[MEMTYPE_SMI].size += MSM_ION_MFC_SIZE;
	msm8x60_reserve_table[MEMTYPE_EBI1].size += MSM_ION_CAMERA_SIZE;
	msm8x60_reserve_table[MEMTYPE_EBI1].size += MSM_ION_WB_SIZE;
	msm8x60_reserve_table[MEMTYPE_EBI1].size += MSM_ION_AUDIO_SIZE;
#endif
}

static void __init size_pmem_devices(void)
{
#ifdef CONFIG_ANDROID_PMEM
#ifndef CONFIG_MSM_MULTIMEDIA_USE_ION
	android_pmem_adsp_pdata.size = pmem_adsp_size;
	android_pmem_smipool_pdata.size = MSM_PMEM_SMIPOOL_SIZE;

	if (hdmi_is_primary)
		pmem_sf_size = MSM_HDMI_PRIM_PMEM_SF_SIZE;
	android_pmem_pdata.size = pmem_sf_size;
	android_pmem_camera_pdata.size = pmem_camera_size;
	android_pmem_swiqi_pdata.size = pmem_swiqi_size;
#endif
	android_pmem_audio_pdata.size = MSM_PMEM_AUDIO_SIZE;
#endif
}

static void __init reserve_memory_for(struct android_pmem_platform_data *p)
{
	msm8x60_reserve_table[p->memory_type].size += p->size;
}

static void __init reserve_pmem_memory(void)
{
#ifdef CONFIG_ANDROID_PMEM
#ifndef CONFIG_MSM_MULTIMEDIA_USE_ION
	reserve_memory_for(&android_pmem_adsp_pdata);
	reserve_memory_for(&android_pmem_smipool_pdata);
	reserve_memory_for(&android_pmem_pdata);
	reserve_memory_for(&android_pmem_camera_pdata);
	reserve_memory_for(&android_pmem_swiqi_pdata);
#endif
	reserve_memory_for(&android_pmem_audio_pdata);
	msm8x60_reserve_table[MEMTYPE_EBI1].size += pmem_kernel_ebi1_size;
#endif
}

static void __init reserve_mdp_memory(void);

static void __init msm8x60_calculate_reserve_sizes(void)
{
	size_pmem_devices();
	reserve_pmem_memory();
	reserve_ion_memory();
	reserve_mdp_memory();
}

static int msm8x60_paddr_to_memtype(unsigned int paddr)
{
	if (paddr >= 0x40000000 && paddr < 0x60000000)
		return MEMTYPE_EBI1;
	if (paddr >= 0x38000000 && paddr < 0x40000000)
		return MEMTYPE_SMI;
	return MEMTYPE_NONE;
}

static struct reserve_info msm8x60_reserve_info __initdata = {
	.memtype_reserve_table = msm8x60_reserve_table,
	.calculate_reserve_sizes = msm8x60_calculate_reserve_sizes,
	.paddr_to_memtype = msm8x60_paddr_to_memtype,
};

static char prim_panel_name[PANEL_NAME_MAX_LEN];
static char ext_panel_name[PANEL_NAME_MAX_LEN];
static int __init prim_display_setup(char *param)
{
	if (strnlen(param, PANEL_NAME_MAX_LEN))
		strlcpy(prim_panel_name, param, PANEL_NAME_MAX_LEN);
	return 0;
}
early_param("prim_display", prim_display_setup);

static int __init ext_display_setup(char *param)
{
	if (strnlen(param, PANEL_NAME_MAX_LEN))
		strlcpy(ext_panel_name, param, PANEL_NAME_MAX_LEN);
	return 0;
}
early_param("ext_display", ext_display_setup);

static void __init msm8x60_reserve(void)
{
	msm8x60_set_display_params(prim_panel_name, ext_panel_name);
	reserve_info = &msm8x60_reserve_info;
	msm_reserve();
}

#define EXT_CHG_VALID_MPP 10
#define EXT_CHG_VALID_MPP_2 11

#if defined(CONFIG_SMB137B_CHARGER) || defined(CONFIG_SMB137B_CHARGER_MODULE)
static int smb137b_detection_setup(void)
{
	int ret = 0, i;

	for (i = 0; i < ARRAY_SIZE(isl_mpp); i++) {
		ret = pm8xxx_mpp_config(isl_mpp[i].mpp,
					&isl_mpp[i].config);
		if (ret) {
			pr_err("%s: Config MPP %d of PM8058 failed\n",
						 __func__, isl_mpp[i].mpp);
			return ret;
		}
	}

	return ret;
}

static struct smb137b_platform_data smb137b_data __initdata = {
	.chg_detection_config = smb137b_detection_setup,
	.valid_n_gpio = PM8058_MPP_PM_TO_SYS(10),
	.batt_mah_rating = 950,
};

static struct i2c_board_info smb137b_charger_i2c_info[] __initdata = {
	{
		I2C_BOARD_INFO("smb137b", 0x08),
		.irq = PM8058_IRQ_BASE + PM8058_CBLPWR_IRQ,
		.platform_data = &smb137b_data,
	},
};
#endif

#ifdef CONFIG_PMIC8058
#define PM8058_GPIO_PIN_DISABLE 0x01

static int pm8058_gpios_init(void)
{
	int i;
	int rc;
	struct pm8058_gpio_cfg {
		int		gpio;
		struct pm_gpio	cfg;
	};

	struct pm_gpio disable_cfg = {
		.disable_pin = PM8058_GPIO_PIN_DISABLE,
	};

	struct pm8058_gpio_cfg gpio_cfgs[] = {
#ifdef CONFIG_MMC_MSM_CARD_HW_DETECTION
		{
			PMIC_GPIO_SDC3_DET - 1,
			{
				.direction	= PM_GPIO_DIR_IN,
				.pull		= PM_GPIO_PULL_UP_30,
				.vin_sel	= 2,
				.function	= PM_GPIO_FUNC_NORMAL,
				.inv_int_pol	= 0,
			},
		},
#endif
		{ /* Timpani Reset */
			20,
			{
				.direction	= PM_GPIO_DIR_OUT,
				.output_value	= 1,
				.output_buffer	= PM_GPIO_OUT_BUF_CMOS,
				.pull		= PM_GPIO_PULL_DN,
				.out_strength	= PM_GPIO_STRENGTH_HIGH,
				.function	= PM_GPIO_FUNC_NORMAL,
				.vin_sel	= 2,
				.inv_int_pol	= 0,
			}
		},
		{ /* PMIC ID interrupt */
			30,
			{
				.direction	= PM_GPIO_DIR_IN,
				.pull		= PM_GPIO_PULL_NO,
				.function	= PM_GPIO_FUNC_NORMAL,
				.vin_sel	= 2,
				.inv_int_pol	= 0,
			}
		},
		{ /* BCM4330 SLEEP CLOCK - GPIO38*/
			37,
			{
				.direction	= PM_GPIO_DIR_OUT,
				.output_buffer	= PM_GPIO_OUT_BUF_CMOS,
				.output_value	= 1,
				.pull		= PM_GPIO_PULL_NO,
				.out_strength	= PM_GPIO_STRENGTH_LOW,
				.function	= PM_GPIO_FUNC_2,
				.vin_sel	= PM8058_GPIO_VIN_S3,
				.inv_int_pol	= 0,
			}
		},
#ifdef CONFIG_PMIC8058_MDM_ANTENA_SWITCH
		{/*PM8058 GPIO 33*/
			32,
			{
				.direction	= PM_GPIO_DIR_IN,
				.pull		= PM_GPIO_PULL_NO,
				.vin_sel	= PM8058_GPIO_VIN_L7,
				.function	= PM_GPIO_FUNC_NORMAL,
			}
		},
		{/*PM8058 GPIO 34*/
			33,
			{
				.direction	= PM_GPIO_DIR_IN,
				.pull		= PM_GPIO_PULL_NO,
				.vin_sel	= PM8058_GPIO_VIN_L7,
				.function	= PM_GPIO_FUNC_NORMAL,
			}
		},
		{/*PM8058 GPIO 35*/
			34,
			{
				.direction	= PM_GPIO_DIR_IN,
				.pull		= PM_GPIO_PULL_NO,
				.vin_sel	= PM8058_GPIO_VIN_L7,
				.function	= PM_GPIO_FUNC_NORMAL,
			}
		},
		{/*PM8058 GPIO 36*/
			35,
			{
				.direction	= PM_GPIO_DIR_IN,
				.pull		= PM_GPIO_PULL_NO,
				.vin_sel	= PM8058_GPIO_VIN_L7,
				.function	= PM_GPIO_FUNC_NORMAL,
			}
		},
		{/*PM8058 GPIO 37*/
			36,
			{
				.direction	= PM_GPIO_DIR_OUT,
				.pull		= PM_GPIO_PULL_NO,
				.vin_sel	= PM8058_GPIO_VIN_L6,
				.function	= PM_GPIO_FUNC_1,
				.output_buffer	= PM_GPIO_OUT_BUF_CMOS,
				.out_strength	= PM_GPIO_STRENGTH_NO,
			}
		},
#endif
#if defined(CONFIG_NFC_PN544) && !defined(CONFIG_SEMC_FELICA_SUPPORT)
		{ /* pn544 NFC_EN - GPIO17 */
			16,
			{
				.direction	= PM_GPIO_DIR_OUT,
				.pull		= PM_GPIO_PULL_NO,
#if defined(CONFIG_MACH_SEMC_NOZOMI)
				.output_buffer  = PM_GPIO_OUT_BUF_OPEN_DRAIN,
				.vin_sel	= PM8058_GPIO_VIN_VPH,
#else
				.vin_sel	= PM8058_GPIO_VIN_S3,
#endif
				.out_strength	= PM_GPIO_STRENGTH_MED,
				.function	= PM_GPIO_FUNC_NORMAL,
				.inv_int_pol	= 0,
			}
		},
		{ /* pn544 NFC_FWDL_EN - GPIO27 */
			26,
			{
				.direction	= PM_GPIO_DIR_OUT,
				.pull		= PM_GPIO_PULL_NO,
				.vin_sel	= PM8058_GPIO_VIN_S3,
				.out_strength	= PM_GPIO_STRENGTH_LOW,
				.function	= PM_GPIO_FUNC_NORMAL,
				.inv_int_pol	= 0,
			}
		},
		{ /* pn544 NFC_IRQ - GPIO28 */
			27,
			{
				.direction	= PM_GPIO_DIR_IN,
				.pull		= PM_GPIO_PULL_DN,
				.vin_sel	= PM8058_GPIO_VIN_S3,
				.out_strength	= PM_GPIO_STRENGTH_NO,
				.function	= PM_GPIO_FUNC_NORMAL,
				.inv_int_pol	= 0,
			}
		},
#endif
#if !defined(CONFIG_NFC_PN544) && defined(CONFIG_SEMC_FELICA_SUPPORT)
		{ /* FELICA_PON - GPIO17 */
			16,
			{
				.direction	= PM_GPIO_DIR_OUT,
				.pull		= PM_GPIO_PULL_NO,
				.vin_sel	= PM8058_GPIO_VIN_S3,
				.out_strength	= PM_GPIO_STRENGTH_MED,
				.function	= PM_GPIO_FUNC_NORMAL,
				.inv_int_pol	= 0,
			}
		},
		{ /* FELICA_RFS - GPIO27 */
			26,
			{
				.direction	= PM_GPIO_DIR_IN,
				.pull		= PM_GPIO_PULL_NO,
				.vin_sel	= PM8058_GPIO_VIN_S3,
				.out_strength	= PM_GPIO_STRENGTH_NO,
				.function	= PM_GPIO_FUNC_NORMAL,
				.inv_int_pol	= 0,
			}
		},
		{ /* FELICA_INT - GPIO28 */
			27,
			{
				.direction	= PM_GPIO_DIR_IN,
				.pull		= PM_GPIO_PULL_NO,
				.vin_sel	= PM8058_GPIO_VIN_S3,
				.out_strength	= PM_GPIO_STRENGTH_NO,
				.function	= PM_GPIO_FUNC_NORMAL,
				.inv_int_pol	= 0,
			}
		},
#endif
		{ /* VOLT_DET_CTRL - GPIO_40 */
			39,
			{
				.direction	= PM_GPIO_DIR_OUT,
				.output_buffer	= PM_GPIO_OUT_BUF_CMOS,
				.output_value	= 1,
				.pull		= PM_GPIO_PULL_NO,
				.vin_sel	= PM8058_GPIO_VIN_VPH,
				.out_strength	= PM_GPIO_STRENGTH_MED,
				.function	= PM_GPIO_FUNC_NORMAL,
				.inv_int_pol	= 0,
			}
		},
	};

	for (i = 0; i < pmic8058_unused_gpios.unused_gpio_num; ++i) {
		rc = pm8xxx_gpio_config(
			PM8058_GPIO_PM_TO_SYS(
				pmic8058_unused_gpios.unused_gpio[i] - 1),
			&disable_cfg);
		if (rc < 0)
			pr_err("%s Failed to disable pmic gpio\n", __func__);
	}

	for (i = 0; i < ARRAY_SIZE(gpio_cfgs); ++i) {
		rc = pm8xxx_gpio_config(
				PM8058_GPIO_PM_TO_SYS(gpio_cfgs[i].gpio),
				&gpio_cfgs[i].cfg);
		if (rc < 0) {
			pr_err("%s pmic gpio config failed\n",
				__func__);
			return rc;
		}
	}

	return 0;
}

static struct pm8xxx_vibrator_platform_data pm8058_vib_pdata = {
	.initial_vibrate_ms  = 0,
	.level_mV = 3000,
	.max_timeout_ms = 15000,
};

static struct pm8xxx_rtc_platform_data pm8058_rtc_pdata = {
	.rtc_write_enable       = false,
	.rtc_alarm_powerup	= false,
};

static struct pm8xxx_pwrkey_platform_data pm8058_pwrkey_pdata = {
	.pull_up		= 1,
	.kpd_trigger_delay_us   = 15625,
	.wakeup			= 1,
};

#define PM8058_LINE_IN_DET_GPIO	PM8058_GPIO_PM_TO_SYS(18)

static struct othc_accessory_info othc_accessories[]  = {
	{
		.accessory = OTHC_SVIDEO_OUT,
		.detect_flags = OTHC_MICBIAS_DETECT | OTHC_SWITCH_DETECT
							| OTHC_ADC_DETECT,
		.key_code = SW_VIDEOOUT_INSERT,
		.enabled = false,
		.adc_thres = {
				.min_threshold = 20,
				.max_threshold = 40,
			},
	},
	{
		.accessory = OTHC_ANC_HEADPHONE,
		.detect_flags = OTHC_MICBIAS_DETECT | OTHC_GPIO_DETECT |
							OTHC_SWITCH_DETECT,
		.gpio = PM8058_LINE_IN_DET_GPIO,
		.active_low = 1,
		.key_code = SW_HEADPHONE_INSERT,
		.enabled = true,
	},
	{
		.accessory = OTHC_ANC_HEADSET,
		.detect_flags = OTHC_MICBIAS_DETECT | OTHC_GPIO_DETECT,
		.gpio = PM8058_LINE_IN_DET_GPIO,
		.active_low = 1,
		.key_code = SW_HEADPHONE_INSERT,
		.enabled = true,
	},
	{
		.accessory = OTHC_HEADPHONE,
		.detect_flags = OTHC_MICBIAS_DETECT | OTHC_SWITCH_DETECT,
		.key_code = SW_HEADPHONE_INSERT,
		.enabled = true,
	},
	{
		.accessory = OTHC_MICROPHONE,
		.detect_flags = OTHC_GPIO_DETECT,
		.gpio = PM8058_LINE_IN_DET_GPIO,
		.active_low = 1,
		.key_code = SW_MICROPHONE_INSERT,
		.enabled = true,
	},
	{
		.accessory = OTHC_HEADSET,
		.detect_flags = OTHC_MICBIAS_DETECT,
		.key_code = SW_HEADPHONE_INSERT,
		.enabled = true,
	},
};

static struct othc_switch_info switch_info[] = {
	{
		.min_adc_threshold = 0,
		.max_adc_threshold = 100,
		.key_code = KEY_PLAYPAUSE,
	},
	{
		.min_adc_threshold = 100,
		.max_adc_threshold = 200,
		.key_code = KEY_REWIND,
	},
	{
		.min_adc_threshold = 200,
		.max_adc_threshold = 500,
		.key_code = KEY_FASTFORWARD,
	},
};

static struct othc_n_switch_config switch_config = {
	.voltage_settling_time_ms = 0,
	.num_adc_samples = 3,
	.adc_channel = CHANNEL_ADC_HDSET,
	.switch_info = switch_info,
	.num_keys = ARRAY_SIZE(switch_info),
	.default_sw_en = true,
	.default_sw_idx = 0,
};

static struct hsed_bias_config hsed_bias_config = {
	/* HSED mic bias config info */
	.othc_headset = OTHC_HEADSET_NO,
	.othc_lowcurr_thresh_uA = 100,
	.othc_highcurr_thresh_uA = 600,
	.othc_hyst_prediv_us = 7800,
	.othc_period_clkdiv_us = 62500,
	.othc_hyst_clk_us = 121000,
	.othc_period_clk_us = 312500,
	.othc_wakeup = 1,
};

static struct othc_hsed_config hsed_config_1 = {
	.hsed_bias_config = &hsed_bias_config,
	/*
	 * The detection delay and switch reporting delay are
	 * required to encounter a hardware bug (spurious switch
	 * interrupts on slow insertion/removal of the headset).
	 * This will introduce a delay in reporting the accessory
	 * insertion and removal to the userspace.
	 */
	.detection_delay_ms = 1500,
	/* Switch info */
	.switch_debounce_ms = 1500,
	.othc_support_n_switch = false,
	.switch_config = &switch_config,
	.ir_gpio = -1,
	/* Accessory info */
	.accessories_support = true,
	.accessories = othc_accessories,
	.othc_num_accessories = ARRAY_SIZE(othc_accessories),
};

static struct othc_regulator_config othc_reg = {
	.regulator	 = "8058_l5",
	.max_uV		 = 2850000,
	.min_uV		 = 2850000,
};

/* MIC_BIAS0 is configured as normal MIC BIAS */
static struct pmic8058_othc_config_pdata othc_config_pdata_0 = {
	.micbias_select = OTHC_MICBIAS_0,
	.micbias_capability = OTHC_MICBIAS,
	.micbias_enable = OTHC_SIGNAL_OFF,
	.micbias_regulator = &othc_reg,
};

/* MIC_BIAS1 is configured as HSED_BIAS for OTHC */
static struct pmic8058_othc_config_pdata othc_config_pdata_1 = {
	.micbias_select = OTHC_MICBIAS_1,
	.micbias_capability = OTHC_MICBIAS_HSED,
	.micbias_enable = OTHC_SIGNAL_PWM_TCXO,
	.micbias_regulator = &othc_reg,
	.hsed_config = &hsed_config_1,
	.hsed_name = "8660_handset",
};

/* MIC_BIAS2 is configured as normal MIC BIAS */
static struct pmic8058_othc_config_pdata othc_config_pdata_2 = {
	.micbias_select = OTHC_MICBIAS_2,
	.micbias_capability = OTHC_MICBIAS,
	.micbias_enable = OTHC_SIGNAL_OFF,
	.micbias_regulator = &othc_reg,
};

#if defined(CONFIG_PMIC8058_OTHC) || defined(CONFIG_PMIC8058_OTHC_MODULE)
static void __init msm8x60_init_pm8058_othc(void)
{
	if (SOCINFO_VERSION_MAJOR(socinfo_get_version()) == 2 ||
		machine_is_msm8x60_fluid() || machine_is_msm8x60_fusion() ||
		machine_is_msm8x60_fusn_ffa()) {
		/* 3-switch headset supported only by V2 FFA and FLUID */
		hsed_config_1.accessories_adc_support = true,
		/* ADC based accessory detection works only on V2 and FLUID */
		hsed_config_1.accessories_adc_channel = CHANNEL_ADC_HDSET,
		hsed_config_1.othc_support_n_switch = true;
	}
}
#endif

static int pm8058_pwm_config(struct pwm_device *pwm, int ch, int on)
{
	struct pm_gpio pwm_gpio_config = {
		.direction      = PM_GPIO_DIR_OUT,
		.output_buffer  = PM_GPIO_OUT_BUF_CMOS,
		.output_value   = 0,
		.pull           = PM_GPIO_PULL_NO,
		.vin_sel        = PM8058_GPIO_VIN_VPH,
		.out_strength   = PM_GPIO_STRENGTH_HIGH,
		.function       = PM_GPIO_FUNC_2,
	};

	int rc = -EINVAL;
	int id, mode, max_mA;

	id = mode = max_mA = 0;
	switch (ch) {
	case 0:
	case 1:
	case 2:
		if (on) {
			id = 24 + ch;
			rc = pm8xxx_gpio_config(PM8058_GPIO_PM_TO_SYS(id - 1),
							&pwm_gpio_config);
			if (rc)
				pr_err("%s: pm8xxx_gpio_config(%d): rc=%d\n",
					__func__, id, rc);
		}
		break;

	case 6:
		id = PM_PWM_LED_FLASH;
		mode = PM_PWM_CONF_PWM1;
		max_mA = 300;
		break;

	case 7:
		id = PM_PWM_LED_FLASH1;
		mode = PM_PWM_CONF_PWM1;
		max_mA = 300;
		break;

	default:
		break;
	}

	if (ch >= 6 && ch <= 7) {
		if (!on) {
			mode = PM_PWM_CONF_NONE;
			max_mA = 0;
		}
		rc = pm8058_pwm_config_led(pwm, id, mode, max_mA);
		if (rc)
			pr_err("%s: pm8058_pwm_config_led(ch=%d): rc=%d\n",
			       __func__, ch, rc);
	}
	return rc;

}

static struct pm8058_pwm_pdata pm8058_pwm_data = {
	.config		= pm8058_pwm_config,
};

#define PM8058_GPIO_INT           88

static struct pmic8058_led pmic8058_flash_leds[] = {
	[0] = {
		.name		= "camera:flash0",
		.max_brightness = 15,
		.id		= PMIC8058_ID_FLASH_LED_0,
	},
	[1] = {
		.name		= "camera:flash1",
		.max_brightness = 15,
		.id		= PMIC8058_ID_FLASH_LED_1,
	},
};

static struct pmic8058_leds_platform_data pm8058_flash_leds_data = {
	.num_leds = ARRAY_SIZE(pmic8058_flash_leds),
	.leds	= pmic8058_flash_leds,
};

static struct pm8xxx_misc_platform_data pm8058_misc_pdata = {
	.priority		= 0,
};

static struct pm8xxx_irq_platform_data pm8058_irq_pdata = {
	.irq_base		= PM8058_IRQ_BASE,
	.devirq			= MSM_GPIO_TO_INT(PM8058_GPIO_INT),
	.irq_trigger_flag	= IRQF_TRIGGER_LOW,
};

static struct pm8xxx_mpp_platform_data pm8058_mpp_pdata = {
	.mpp_base	= PM8058_MPP_PM_TO_SYS(0),
};

static struct pm8xxx_gpio_platform_data pm8058_gpio_pdata = {
	.gpio_base	= PM8058_GPIO_PM_TO_SYS(0),
};

#ifdef CONFIG_PMIC8058_MIC_BIAS
#define PM8058_HSED_MIC_BIAS	0xA1

struct pm8058_mic_bias_platform_data pm8058_mic_bias_pdata = {
	.mic_bias_addr = PM8058_HSED_MIC_BIAS,
};
#endif

static struct pm8058_platform_data pm8058_platform_data = {
	.irq_pdata		= &pm8058_irq_pdata,
	.gpio_pdata		= &pm8058_gpio_pdata,
	.mpp_pdata		= &pm8058_mpp_pdata,
	.rtc_pdata		= &pm8058_rtc_pdata,
	.pwrkey_pdata		= &pm8058_pwrkey_pdata,
	.othc0_pdata		= &othc_config_pdata_0,
	.othc1_pdata		= &othc_config_pdata_1,
	.othc2_pdata		= &othc_config_pdata_2,
	.pwm_pdata		= &pm8058_pwm_data,
	.misc_pdata		= &pm8058_misc_pdata,
	.simple_remote_pdata	= &simple_remote_pf_data,
#ifdef CONFIG_SENSORS_MSM_ADC
	.xoadc_pdata		= &pm8058_xoadc_pdata,
#endif
#ifdef CONFIG_PMIC8058_MIC_BIAS
	.mic_bias_pdata		= &pm8058_mic_bias_pdata,
#endif
};

#ifdef CONFIG_MSM_SSBI
static struct msm_ssbi_platform_data msm8x60_ssbi_pm8058_pdata __devinitdata = {
	.controller_type = MSM_SBI_CTRL_PMIC_ARBITER,
	.slave	= {
		.name			= "pm8058-core",
		.platform_data		= &pm8058_platform_data,
	},
};
#endif
#endif  /* CONFIG_PMIC8058 */

static int hw_id_get_mask(void)
{
	int hwid;
	unsigned int i;
	int rc;
	for (hwid = i = 0; i < ARRAY_SIZE(hw_id_gpios); i++) {
		rc = gpio_request(hw_id_gpios[i], "HWID_GPIO");
		if (rc) {
			pr_err("%s: gpio_requeset failed, "
					"rc=%d\n", __func__, rc);
			return rc;
		}
		hwid |= gpio_get_value(hw_id_gpios[i]) << i;
		gpio_free(hw_id_gpios[i]);
	}
	return hwid;
}

static ssize_t hw_id_show(struct class *class,
			  struct class_attribute *attr,
			  char *buf)
{
	int hwid;

	hwid = hw_id_get_mask();
	if (hwid < 0)
		return -EINVAL;

	pr_info("Board Fuji HW ID: 0x%02x\n", hwid);
	return snprintf(buf, PAGE_SIZE, "0x%02x\n", hwid);
}

static CLASS_ATTR(hwid, 0400, hw_id_show, NULL);
static struct class hwid_class = {.name	= "hwid",};
static void __init hw_id_class_init(void)
{
	int error;
	error = class_register(&hwid_class);
	if (error) {
		pr_err("%s: class_register failed\n", __func__);
		return;
	}
	error = class_create_file(&hwid_class, &class_attr_hwid);
	if (error) {
		pr_err("%s: class_create_file failed\n", __func__);
		class_unregister(&hwid_class);
	}
}

#define PLUG_DET_ENA_PIN -1
#define PLUG_DET_READ_PIN 61
#define BUTTON_DET_IRQ -1

#define PM8058_OTHC_CNTR_BASE1	0x134

int simple_remote_pf_initialize_gpio(struct simple_remote_platform_data *data)
{
	int err = 0;
	int i;
	int hwid;

	if (!data ||
	    -1 == data->headset_detect_read_pin) {
		pr_err("%s: Invalid inparameter (GPIO Pins)."
		       " Aborting!\n", __func__);
		return -EIO;
	}

	hwid = hw_id_get_mask();
	if (hwid < 0) {
		pr_err("%s: not able to get hw_id\n", __func__);
		return -EINVAL;
	}

	err = gpio_request(data->headset_detect_read_pin,
			   "Simple_remote_plug_detect_read");
	if (err) {
		pr_err("%s: %d Request hs-detect_read pin",
		       __func__, err);
		goto out;
	}

	err = gpio_direction_input(data->headset_detect_read_pin);
	if (err) {
		pr_err("%s: %d Set hs-detect pin as input\n",
		       __func__, err);
		goto out_hs_det_read;
	}

	for (i = 0; i < data->num_regs; i++) {
		data->regs[i].reg = regulator_get(NULL, data->regs[i].name);
		if (IS_ERR(data->regs[i].reg)) {
			pr_err("%s - Failed to find regulator %s\n",
			       __func__, data->regs[i].name);
			err = PTR_ERR(data->regs[i].reg);
			goto out_hs_det_read;
		}

		err = regulator_set_voltage(data->regs[i].reg,
					    data->regs[i].vol_min,
					    data->regs[i].vol_max);
		if (err) {
			pr_err("%s - Unable to set the voltage for regulator "
			       "%s\n", __func__, data->regs[i].name);
			goto out_hs_det_read;
		}
	}

	return err;

out_hs_det_read:
	gpio_free(data->headset_detect_read_pin);
out:
	return err;
}

void simple_remote_pf_deinitialize_gpio(
	struct simple_remote_platform_data *data)
{
	gpio_free(data->headset_detect_read_pin);
}

static struct simple_remote_platform_regulators regs[] =  {
	{
		.name = "8058_ncp",
		.vol_min = 1800000,
		.vol_max = 1800000,
	},
	{
		.name = "8058_s3",
		.vol_min = 1800000,
		.vol_max = 1800000,
	},

};

struct simple_remote_platform_data simple_remote_pf_data = {
	.headset_detect_enable_pin = PLUG_DET_ENA_PIN,
	.headset_detect_read_pin = PLUG_DET_READ_PIN,
	.button_detect_irq = BUTTON_DET_IRQ,
	.adc_channel = CHANNEL_ADC_HDSET,
	.hsed_base = PM8058_OTHC_CNTR_BASE1,
	.initialize = &simple_remote_pf_initialize_gpio,
	.deinitialize = &simple_remote_pf_deinitialize_gpio,
	.regs = regs,
	.num_regs = ARRAY_SIZE(regs),
#ifdef CONFIG_SIMPLE_REMOTE_INVERT_PLUG_DETECTION_STATE
	.invert_plug_det = 1,
#else
	.invert_plug_det = 0,
#endif
};

#define PM_GPIO_CDC_RST_N 20
#define GPIO_CDC_RST_N PM8058_GPIO_PM_TO_SYS(PM_GPIO_CDC_RST_N)

static struct regulator *vreg_timpani_1;
static struct regulator *vreg_timpani_2;

static unsigned int msm_timpani_setup_power(void)
{
	int rc;

	vreg_timpani_1 = regulator_get(NULL, "8058_l0");
	if (IS_ERR(vreg_timpani_1)) {
		pr_err("%s: Unable to get 8058_l0\n", __func__);
		return -ENODEV;
	}

	vreg_timpani_2 = regulator_get(NULL, "8058_s3");
	if (IS_ERR(vreg_timpani_2)) {
		pr_err("%s: Unable to get 8058_s3\n", __func__);
		regulator_put(vreg_timpani_1);
		return -ENODEV;
	}

	rc = regulator_set_voltage(vreg_timpani_1, 1200000, 1200000);
	if (rc) {
		pr_err("%s: unable to set L0 voltage to 1.2V\n", __func__);
		goto fail;
	}

	rc = regulator_set_voltage(vreg_timpani_2, 1800000, 1800000);
	if (rc) {
		pr_err("%s: unable to set S3 voltage to 1.8V\n", __func__);
		goto fail;
	}

	rc = regulator_enable(vreg_timpani_1);
	if (rc) {
		pr_err("%s: Enable regulator 8058_l0 failed\n", __func__);
		goto fail;
	}

	/* The settings for LDO0 should be set such that
	*  it doesn't require to reset the timpani. */
	rc = regulator_set_optimum_mode(vreg_timpani_1, 5000);
	if (rc < 0) {
		pr_err("Timpani regulator optimum mode setting failed\n");
		goto fail;
	}

	rc = regulator_enable(vreg_timpani_2);
	if (rc) {
		pr_err("%s: Enable regulator 8058_s3 failed\n", __func__);
		regulator_disable(vreg_timpani_1);
		goto fail;
	}

	rc = gpio_request(GPIO_CDC_RST_N, "CDC_RST_N");
	if (rc) {
		pr_err("%s: GPIO Request %d failed\n", __func__,
			GPIO_CDC_RST_N);
		regulator_disable(vreg_timpani_1);
		regulator_disable(vreg_timpani_2);
		goto fail;
	} else {
		gpio_direction_output(GPIO_CDC_RST_N, 1);
		usleep_range(1000, 1050);
		gpio_direction_output(GPIO_CDC_RST_N, 0);
		usleep_range(1000, 1050);
		gpio_direction_output(GPIO_CDC_RST_N, 1);
		gpio_free(GPIO_CDC_RST_N);
	}
	return rc;

fail:
	regulator_put(vreg_timpani_1);
	regulator_put(vreg_timpani_2);
	return rc;
}

static void msm_timpani_shutdown_power(void)
{
	int rc;

	rc = regulator_disable(vreg_timpani_1);
	if (rc)
		pr_err("%s: Disable regulator 8058_l0 failed\n", __func__);

	regulator_put(vreg_timpani_1);

	rc = regulator_disable(vreg_timpani_2);
	if (rc)
		pr_err("%s: Disable regulator 8058_s3 failed\n", __func__);

	regulator_put(vreg_timpani_2);
}

/* Power analog function of codec */
static struct regulator *vreg_timpani_cdc_apwr;
static int msm_timpani_codec_power(int vreg_on)
{
	int rc = 0;

	if (!vreg_timpani_cdc_apwr) {

		vreg_timpani_cdc_apwr = regulator_get(NULL, "8058_s4");

		if (IS_ERR(vreg_timpani_cdc_apwr)) {
			pr_err("%s: vreg_get failed (%ld)\n",
			__func__, PTR_ERR(vreg_timpani_cdc_apwr));
			rc = PTR_ERR(vreg_timpani_cdc_apwr);
			return rc;
		}
	}

	if (vreg_on) {

		rc = regulator_set_voltage(vreg_timpani_cdc_apwr,
				2200000, 2200000);
		if (rc) {
			pr_err("%s: unable to set 8058_s4 voltage to 2.2 V\n",
					__func__);
			goto vreg_fail;
		}

		rc = regulator_enable(vreg_timpani_cdc_apwr);
		if (rc) {
			pr_err("%s: vreg_enable failed %d\n", __func__, rc);
			goto vreg_fail;
		}
	} else {
		rc = regulator_disable(vreg_timpani_cdc_apwr);
		if (rc) {
			pr_err("%s: vreg_disable failed %d\n",
			__func__, rc);
			goto vreg_fail;
		}
	}

	return 0;

vreg_fail:
	regulator_put(vreg_timpani_cdc_apwr);
	vreg_timpani_cdc_apwr = NULL;
	return rc;
}

static struct marimba_codec_platform_data timpani_codec_pdata = {
	.marimba_codec_power =  msm_timpani_codec_power,
};

#define TIMPANI_SLAVE_ID_CDC_ADDR		0X77
#define TIMPANI_SLAVE_ID_QMEMBIST_ADDR		0X66

static struct marimba_platform_data timpani_pdata = {
	.slave_id[MARIMBA_SLAVE_ID_CDC]	= TIMPANI_SLAVE_ID_CDC_ADDR,
	.slave_id[MARIMBA_SLAVE_ID_QMEMBIST] = TIMPANI_SLAVE_ID_QMEMBIST_ADDR,
	.marimba_setup = msm_timpani_setup_power,
	.marimba_shutdown = msm_timpani_shutdown_power,
	.codec = &timpani_codec_pdata,
	.tsadc_ssbi_adap = MARIMBA_SSBI_ADAP,
};

#define TIMPANI_I2C_SLAVE_ADDR	0xD

static struct i2c_board_info msm_i2c_gsbi7_timpani_info[] = {
	{
		I2C_BOARD_INFO("timpani", TIMPANI_I2C_SLAVE_ADDR),
		.platform_data = &timpani_pdata,
	},
};

#ifdef CONFIG_SND_SOC_WM8903
static struct wm8903_platform_data wm8903_pdata = {
	.gpio_cfg[2] = 0x3A8,
};

#define WM8903_I2C_SLAVE_ADDR 0x34
static struct i2c_board_info wm8903_codec_i2c_info[] = {
	{
		I2C_BOARD_INFO("wm8903", WM8903_I2C_SLAVE_ADDR >> 1),
		.platform_data = &wm8903_pdata,
	},
};
#endif
#ifdef CONFIG_PMIC8901

#define PM8901_GPIO_INT           91

/*
 * Consumer specific regulator names:
 *			 regulator name		consumer dev_name
 */
static struct regulator_consumer_supply vreg_consumers_8901_HDMI_MVS[] = {
	REGULATOR_SUPPLY("8901_hdmi_mvs",	NULL),
};

#define PM8901_VREG_INIT(_id, _min_uV, _max_uV, _modes, _ops, _apply_uV, \
			 _always_on) \
	{ \
		.init_data = { \
			.constraints = { \
				.valid_modes_mask = _modes, \
				.valid_ops_mask = _ops, \
				.min_uV = _min_uV, \
				.max_uV = _max_uV, \
				.input_uV = _min_uV, \
				.apply_uV = _apply_uV, \
				.always_on = _always_on, \
			}, \
			.consumer_supplies = vreg_consumers_8901_##_id, \
			.num_consumer_supplies = \
				ARRAY_SIZE(vreg_consumers_8901_##_id), \
		}, \
		.id = PM8901_VREG_ID_##_id, \
	}

#define PM8901_VREG_INIT_VS(_id) \
	PM8901_VREG_INIT(_id, 0, 0, REGULATOR_MODE_NORMAL, \
			REGULATOR_CHANGE_STATUS, 0, 0)

static struct pm8901_vreg_pdata pm8901_vreg_init[] = {
	PM8901_VREG_INIT_VS(HDMI_MVS),
};

static struct pm8xxx_misc_platform_data pm8901_misc_pdata = {
	.priority		= 1,
};

static struct pm8xxx_irq_platform_data pm8901_irq_pdata = {
	.irq_base		= PM8901_IRQ_BASE,
	.devirq			= MSM_GPIO_TO_INT(PM8901_GPIO_INT),
	.irq_trigger_flag	= IRQF_TRIGGER_LOW,
};

static struct pm8xxx_mpp_platform_data pm8901_mpp_pdata = {
	.mpp_base		= PM8901_MPP_PM_TO_SYS(0),
};

static struct pm8901_platform_data pm8901_platform_data = {
	.irq_pdata		= &pm8901_irq_pdata,
	.mpp_pdata		= &pm8901_mpp_pdata,
	.regulator_pdatas	= pm8901_vreg_init,
	.num_regulators		= ARRAY_SIZE(pm8901_vreg_init),
	.misc_pdata		= &pm8901_misc_pdata,
};

static struct msm_ssbi_platform_data msm8x60_ssbi_pm8901_pdata __devinitdata = {
	.controller_type = MSM_SBI_CTRL_PMIC_ARBITER,
	.slave	= {
		.name = "pm8901-core",
		.platform_data = &pm8901_platform_data,
	},
};

#define GPIO_VREG_ID_EXT_5V            0

static struct regulator_consumer_supply vreg_consumers_EXT_5V[] = {
	REGULATOR_SUPPLY("ext_5v",      NULL),
	REGULATOR_SUPPLY("8901_mpp0",   NULL),
};

#define GPIO_VREG_INIT(_id, _reg_name, _gpio_label, _gpio, _active_low) \
	[GPIO_VREG_ID_##_id] = { \
		.init_data = { \
			.constraints = { \
				.valid_ops_mask = REGULATOR_CHANGE_STATUS, \
			}, \
			.num_consumer_supplies  = \
					ARRAY_SIZE(vreg_consumers_##_id), \
			.consumer_supplies      = vreg_consumers_##_id, \
		}, \
		.regulator_name = _reg_name, \
		.active_low     = _active_low, \
		.gpio_label     = _gpio_label, \
		.gpio           = _gpio, \
	}

/* GPIO regulator constraints */
static struct gpio_regulator_platform_data msm_gpio_regulator_pdata[] = {
	GPIO_VREG_INIT(EXT_5V, "ext_5v", "ext_5v_en",
					PM8901_MPP_PM_TO_SYS(0), 0),
};

/* GPIO regulator */
struct platform_device msm8x60_8901_mpp_vreg __devinitdata = {
	.name   = GPIO_REGULATOR_DEV_NAME,
	.id     = PM8901_MPP_PM_TO_SYS(0),
	.dev    = {
		.platform_data =
			&msm_gpio_regulator_pdata[GPIO_VREG_ID_EXT_5V],
	},
};

void __init pm8901_vreg_mpp0_init(void)
{
	int rc;

	struct pm8xxx_mpp_init_info pm8901_vreg_mpp0 = {
		.mpp    = PM8901_MPP_PM_TO_SYS(0),
		.config =  {
			.type   = PM8XXX_MPP_TYPE_D_OUTPUT,
			.level  = PM8901_MPP_DIG_LEVEL_MSMIO,
			.control = PM8XXX_MPP_DOUT_CTRL_LOW,
		},
	};

	rc = pm8xxx_mpp_config(pm8901_vreg_mpp0.mpp, &pm8901_vreg_mpp0.config);
	if (rc)
		pr_err("%s: pm8xxx_mpp_config: rc=%d\n", __func__, rc);
}

#endif /* CONFIG_PMIC8901 */

static void msm8x60_pm8901_dVdd_init(void)
{
	int hwid;

	hwid = hw_id_get_mask();
	if (hwid >= 2)
		pm8901_platform_data.pm_dVdd_unstable = 0;
	else
		pm8901_platform_data.pm_dVdd_unstable = 1;
}

#ifdef CONFIG_I2C
#define I2C_SURF 1
#define I2C_FFA  (1 << 1)
#define I2C_RUMI (1 << 2)
#define I2C_SIM  (1 << 3)
#define I2C_FLUID (1 << 4)
#define I2C_DRAGON (1 << 5)

struct i2c_registry {
	u8                     machs;
	int                    bus;
	struct i2c_board_info *info;
	int                    len;
};

static struct i2c_registry msm8x60_i2c_devices[] __initdata = {
#ifdef CONFIG_MSM_CAMERA
	{
		0,
		MSM_GSBI4_QUP_I2C_BUS_ID,
		msm_camera_boardinfo,
		ARRAY_SIZE(msm_camera_boardinfo),
	},
#endif

	{
		I2C_SURF | I2C_FFA | I2C_FLUID,
		MSM_GSBI7_QUP_I2C_BUS_ID,
		msm_i2c_gsbi7_timpani_info,
		ARRAY_SIZE(msm_i2c_gsbi7_timpani_info),
	},
#ifdef CONFIG_TOUCHSCREEN_CLEARPAD
	{
		0,
		MSM_GSBI3_QUP_I2C_BUS_ID,
		gsbi3_peripherals_info,
		ARRAY_SIZE(gsbi3_peripherals_info),
	},
#endif
#ifdef CONFIG_I2C
	{
		0,
		MSM_GSBI8_QUP_I2C_BUS_ID,
		fuji_gsbi8_peripherals_info,
		ARRAY_SIZE(fuji_gsbi8_peripherals_info)
	},
	{
		0,
		MSM_GSBI12_QUP_I2C_BUS_ID,
		fuji_gsbi12_peripherals_info,
		ARRAY_SIZE(fuji_gsbi12_peripherals_info)
	},
#endif
};
#endif /* CONFIG_I2C */

static void register_i2c_devices(void)
{
#ifdef CONFIG_I2C
	int i;

	/* Run the array and install devices as appropriate */
	for (i = 0; i < ARRAY_SIZE(msm8x60_i2c_devices); ++i) {
		i2c_register_board_info(msm8x60_i2c_devices[i].bus,
				msm8x60_i2c_devices[i].info,
				msm8x60_i2c_devices[i].len);
	}
#endif
}

#define TCSR_ADM_1_A_CRCI_MUX_SEL (MSM_TCSR_BASE + 0x078)
#define ADM1_CRCI_GSBI7_RX_SEL (1<<13)
#define ADM1_CRCI_GSBI7_TX_SEL (1<<12)
#define ADM1_CRCI_GSBI7_MASK (ADM1_CRCI_GSBI7_RX_SEL | \
				ADM1_CRCI_GSBI7_TX_SEL)
static void __init msm8x60_init_uart7dm(void)
{
	uint32_t tmp;

	/* Read the Current Register Value from TCSR_ADM_1 */
	tmp = secure_readl(TCSR_ADM_1_A_CRCI_MUX_SEL);
	/* Clear the bits for GSBI7 */
	tmp &= ~(ADM1_CRCI_GSBI7_MASK);
	/* Set GSBI7 for UART TX and RX */
	tmp |= (ADM1_CRCI_GSBI7_RX_SEL | ADM1_CRCI_GSBI7_TX_SEL);
	/* Write the Data */
	secure_writel(tmp, TCSR_ADM_1_A_CRCI_MUX_SEL);
}

static void __init msm8x60_init_uart12dm(void)
{
#if !defined(CONFIG_USB_PEHCI_HCD) && !defined(CONFIG_USB_PEHCI_HCD_MODULE)
	/* 0x1D000000 now belongs to EBI2:CS3 i.e. USB ISP Controller */
	void *fpga_mem = ioremap_nocache(0x1D000000, SZ_4K);

	if (!fpga_mem)
		pr_err("%s(): Error getting memory\n", __func__);

	/* Advanced mode */
	writew_relaxed(0xFFFF, fpga_mem + 0x15C);
	/* FPGA_UART_SEL */
	writew_relaxed(0, fpga_mem + 0x172);
	/* FPGA_GPIO_CONFIG_117 */
	writew_relaxed(1, fpga_mem + 0xEA);
	/* FPGA_GPIO_CONFIG_118 */
	writew_relaxed(1, fpga_mem + 0xEC);
	mb();
	iounmap(fpga_mem);
#endif
}

#define MSM_GSBI9_PHYS		0x19900000
#define GSBI_DUAL_MODE_CODE	0x60

static void __init msm8x60_init_buses(void)
{
#ifdef CONFIG_I2C_QUP
	void *gsbi_mem = ioremap_nocache(0x19C00000, 4);
	void *gsbi5_mem = ioremap_nocache(0x16400000, 4);
	void *gsbi7_mem = ioremap_nocache(0x16600000, 4);
	/* Setting protocol code to 0x60 for dual UART/I2C in GSBI12 */
	writel_relaxed(0x6 << 4, gsbi_mem);
	/* Ensure protocol code is written before proceeding further */
	mb();
	iounmap(gsbi_mem);
	/* Setting protocol code to 0x60 for dual UART/I2C in GSBI5 */
	writel_relaxed(0x6 << 4, gsbi5_mem);
	iounmap(gsbi5_mem);
	/* Setting protocol code to 0x60 for dual UART/I2C in GSBI7 */
	writel_relaxed(0x6 << 4, gsbi7_mem);
	iounmap(gsbi7_mem);

	msm_gsbi3_qup_i2c_device.dev.platform_data = &msm_gsbi3_qup_i2c_pdata;
	msm_gsbi4_qup_i2c_device.dev.platform_data = &msm_gsbi4_qup_i2c_pdata;
#ifdef CONFIG_MSM_GSBI5_I2C
	msm_gsbi5_qup_i2c_device.dev.platform_data = &msm_gsbi5_qup_i2c_pdata;
#endif
	msm_gsbi7_qup_i2c_device.dev.platform_data = &msm_gsbi7_qup_i2c_pdata;
	msm_gsbi8_qup_i2c_device.dev.platform_data = &msm_gsbi8_qup_i2c_pdata;
	msm_gsbi9_qup_i2c_device.dev.platform_data = &msm_gsbi9_qup_i2c_pdata;
	msm_gsbi12_qup_i2c_device.dev.platform_data = &msm_gsbi12_qup_i2c_pdata;
#endif
#if defined(CONFIG_SPI_QUP) || defined(CONFIG_SPI_QUP_MODULE)
	msm_gsbi1_qup_spi_device.dev.platform_data = &msm_gsbi1_qup_spi_pdata;
#endif
#ifdef CONFIG_I2C_SSBI
	msm_device_ssbi3.dev.platform_data = &msm_ssbi3_pdata;
#endif

#ifdef CONFIG_MSM_SSBI
	msm_device_ssbi_pmic1.dev.platform_data =
				&msm8x60_ssbi_pm8058_pdata;
	msm_device_ssbi_pmic2.dev.platform_data =
				&msm8x60_ssbi_pm8901_pdata;
#endif

#if defined(CONFIG_SPI_QUP) || defined(CONFIG_SPI_QUP_MODULE)
	msm_gsbi10_qup_spi_device.dev.platform_data =
		&msm_gsbi10_qup_spi_pdata;
#endif

#if defined(CONFIG_USB_GADGET_MSM_72K) || defined(CONFIG_USB_EHCI_HCD)
	msm_device_otg.dev.platform_data = &msm_otg_pdata;
#endif

#ifdef CONFIG_USB_GADGET_MSM_72K
	msm_device_gadget_peripheral.dev.platform_data = &msm_gadget_pdata;
#endif

#ifdef CONFIG_SERIAL_MSM_HS
	msm_uart_dm1_pdata.wakeup_irq = gpio_to_irq(54); /* GSBI6(2) */
	msm_device_uart_dm1.dev.platform_data = &msm_uart_dm1_pdata;
#endif
#ifdef CONFIG_MSM_GSBI5_UART
	msm_device_uart_gsbi5.dev.platform_data = &msm_uart_gsbi5_pdata;
#endif
#ifdef CONFIG_MSM_GSBI7_UART
	msm_device_uart_dm7.dev.platform_data = &msm_uart_dm7_pdata;
#endif

#ifdef CONFIG_MSM_BUS_SCALING

	/* RPM calls are only enabled on V2 */
	if (SOCINFO_VERSION_MAJOR(socinfo_get_version()) == 2) {
		msm_bus_apps_fabric_pdata.rpm_enabled = 1;
		msm_bus_sys_fabric_pdata.rpm_enabled = 1;
		msm_bus_mm_fabric_pdata.rpm_enabled = 1;
		msm_bus_sys_fpb_pdata.rpm_enabled = 1;
		msm_bus_cpss_fpb_pdata.rpm_enabled = 1;
	}

	msm_bus_apps_fabric.dev.platform_data = &msm_bus_apps_fabric_pdata;
	msm_bus_sys_fabric.dev.platform_data = &msm_bus_sys_fabric_pdata;
	msm_bus_mm_fabric.dev.platform_data = &msm_bus_mm_fabric_pdata;
	msm_bus_sys_fpb.dev.platform_data = &msm_bus_sys_fpb_pdata;
	msm_bus_cpss_fpb.dev.platform_data = &msm_bus_cpss_fpb_pdata;
#endif
}

static void __init msm8x60_map_io(void)
{
	msm_shared_ram_phys = MSM_SHARED_RAM_PHYS;
	msm_map_msm8x60_io();

	if (socinfo_init() < 0)
		pr_err("socinfo_init() failed!\n");
}

static void __init msm8x60_init_tlmm(void)
{
}

#if (defined(CONFIG_MMC_MSM_SDC1_SUPPORT)\
	|| defined(CONFIG_MMC_MSM_SDC2_SUPPORT)\
	|| defined(CONFIG_MMC_MSM_SDC3_SUPPORT)\
	|| defined(CONFIG_MMC_MSM_SDC4_SUPPORT)\
	|| defined(CONFIG_MMC_MSM_SDC5_SUPPORT))

/* 8x60 has 5 SDCC controllers */
#define MAX_SDCC_CONTROLLER	5

struct msm_sdcc_gpio {
	/* maximum 10 GPIOs per SDCC controller */
	s16 no;
	/* name of this GPIO */
	const char *name;
	bool always_on;
	bool is_enabled;
};

#ifdef CONFIG_MMC_MSM_SDC1_SUPPORT
static struct msm_sdcc_gpio sdc1_gpio_cfg[] = {
	{159, "sdc1_dat_0"},
	{160, "sdc1_dat_1"},
	{161, "sdc1_dat_2"},
	{162, "sdc1_dat_3"},
#ifdef CONFIG_MMC_MSM_SDC1_8_BIT_SUPPORT
	{163, "sdc1_dat_4"},
	{164, "sdc1_dat_5"},
	{165, "sdc1_dat_6"},
	{166, "sdc1_dat_7"},
#endif
	{167, "sdc1_clk"},
	{168, "sdc1_cmd"}
};
#endif

#ifdef CONFIG_MMC_MSM_SDC2_SUPPORT
static struct msm_sdcc_gpio sdc2_gpio_cfg[] = {
	{143, "sdc2_dat_0"},
	{144, "sdc2_dat_1", 1},
	{145, "sdc2_dat_2"},
	{146, "sdc2_dat_3"},
#ifdef CONFIG_MMC_MSM_SDC2_8_BIT_SUPPORT
	{147, "sdc2_dat_4"},
	{148, "sdc2_dat_5"},
	{149, "sdc2_dat_6"},
	{150, "sdc2_dat_7"},
#endif
	{151, "sdc2_cmd"},
	{152, "sdc2_clk", 1}
};
#endif

#ifdef CONFIG_MMC_MSM_SDC5_SUPPORT
static struct msm_sdcc_gpio sdc5_gpio_cfg[] = {
	{95, "sdc5_cmd"},
	{96, "sdc5_dat_3"},
	{97, "sdc5_clk", 1},
	{98, "sdc5_dat_2"},
	{99, "sdc5_dat_1", 1},
	{100, "sdc5_dat_0"}
};
#endif

struct msm_sdcc_pad_pull_cfg {
	enum msm_tlmm_pull_tgt pull;
	u32 pull_val;
};

struct msm_sdcc_pad_drv_cfg {
	enum msm_tlmm_hdrive_tgt drv;
	u32 drv_val;
};

#ifdef CONFIG_MMC_MSM_SDC3_SUPPORT
static struct msm_sdcc_pad_drv_cfg sdc3_pad_on_drv_cfg[] = {
	{TLMM_HDRV_SDC3_CLK, GPIO_CFG_8MA},
	{TLMM_HDRV_SDC3_CMD, GPIO_CFG_8MA},
	{TLMM_HDRV_SDC3_DATA, GPIO_CFG_8MA}
};

static struct msm_sdcc_pad_pull_cfg sdc3_pad_on_pull_cfg[] = {
	{TLMM_PULL_SDC3_CMD, GPIO_CFG_PULL_UP},
	{TLMM_PULL_SDC3_DATA, GPIO_CFG_PULL_UP}
};

static struct msm_sdcc_pad_drv_cfg sdc3_pad_off_drv_cfg[] = {
	{TLMM_HDRV_SDC3_CLK, GPIO_CFG_2MA},
	{TLMM_HDRV_SDC3_CMD, GPIO_CFG_2MA},
	{TLMM_HDRV_SDC3_DATA, GPIO_CFG_2MA}
};

static struct msm_sdcc_pad_pull_cfg sdc3_pad_off_pull_cfg[] = {
	{TLMM_PULL_SDC3_CMD, GPIO_CFG_PULL_DOWN},
	{TLMM_PULL_SDC3_DATA, GPIO_CFG_PULL_DOWN}
};
#endif

#ifdef CONFIG_MMC_MSM_SDC4_SUPPORT
static struct msm_sdcc_pad_drv_cfg sdc4_pad_on_drv_cfg[] = {
	{TLMM_HDRV_SDC4_CLK, GPIO_CFG_8MA},
	{TLMM_HDRV_SDC4_CMD, GPIO_CFG_8MA},
	{TLMM_HDRV_SDC4_DATA, GPIO_CFG_8MA}
};

static struct msm_sdcc_pad_pull_cfg sdc4_pad_on_pull_cfg[] = {
	{TLMM_PULL_SDC4_CMD, GPIO_CFG_PULL_UP},
	{TLMM_PULL_SDC4_DATA, GPIO_CFG_PULL_UP}
};

static struct msm_sdcc_pad_drv_cfg sdc4_pad_off_drv_cfg[] = {
	{TLMM_HDRV_SDC4_CLK, GPIO_CFG_2MA},
	{TLMM_HDRV_SDC4_CMD, GPIO_CFG_2MA},
	{TLMM_HDRV_SDC4_DATA, GPIO_CFG_2MA}
};

static struct msm_sdcc_pad_pull_cfg sdc4_pad_off_pull_cfg[] = {
	{TLMM_PULL_SDC4_CMD, GPIO_CFG_PULL_DOWN},
	{TLMM_PULL_SDC4_DATA, GPIO_CFG_PULL_DOWN}
};
#endif

struct msm_sdcc_pin_cfg {
	/*
	 * = 1 if controller pins are using gpios
	 * = 0 if controller has dedicated MSM pins
	 */
	u8 is_gpio;
	u8 cfg_sts;
	u8 gpio_data_size;
	struct msm_sdcc_gpio *gpio_data;
	struct msm_sdcc_pad_drv_cfg *pad_drv_on_data;
	struct msm_sdcc_pad_drv_cfg *pad_drv_off_data;
	struct msm_sdcc_pad_pull_cfg *pad_pull_on_data;
	struct msm_sdcc_pad_pull_cfg *pad_pull_off_data;
	u8 pad_drv_data_size;
	u8 pad_pull_data_size;
	u8 sdio_lpm_gpio_cfg;
};


static struct msm_sdcc_pin_cfg sdcc_pin_cfg_data[MAX_SDCC_CONTROLLER] = {
#ifdef CONFIG_MMC_MSM_SDC1_SUPPORT
	[0] = {
		.is_gpio = 1,
		.gpio_data_size = ARRAY_SIZE(sdc1_gpio_cfg),
		.gpio_data = sdc1_gpio_cfg
	},
#endif
#ifdef CONFIG_MMC_MSM_SDC2_SUPPORT
	[1] = {
		.is_gpio = 1,
		.gpio_data_size = ARRAY_SIZE(sdc2_gpio_cfg),
		.gpio_data = sdc2_gpio_cfg
	},
#endif
#ifdef CONFIG_MMC_MSM_SDC3_SUPPORT
	[2] = {
		.is_gpio = 0,
		.pad_drv_on_data = sdc3_pad_on_drv_cfg,
		.pad_drv_off_data = sdc3_pad_off_drv_cfg,
		.pad_pull_on_data = sdc3_pad_on_pull_cfg,
		.pad_pull_off_data = sdc3_pad_off_pull_cfg,
		.pad_drv_data_size = ARRAY_SIZE(sdc3_pad_on_drv_cfg),
		.pad_pull_data_size = ARRAY_SIZE(sdc3_pad_on_pull_cfg)
	},
#endif
#ifdef CONFIG_MMC_MSM_SDC4_SUPPORT
	[3] = {
		.is_gpio = 0,
		.pad_drv_on_data = sdc4_pad_on_drv_cfg,
		.pad_drv_off_data = sdc4_pad_off_drv_cfg,
		.pad_pull_on_data = sdc4_pad_on_pull_cfg,
		.pad_pull_off_data = sdc4_pad_off_pull_cfg,
		.pad_drv_data_size = ARRAY_SIZE(sdc4_pad_on_drv_cfg),
		.pad_pull_data_size = ARRAY_SIZE(sdc4_pad_on_pull_cfg)
	},
#endif
#ifdef CONFIG_MMC_MSM_SDC5_SUPPORT
	[4] = {
		.is_gpio = 1,
		.gpio_data_size = ARRAY_SIZE(sdc5_gpio_cfg),
		.gpio_data = sdc5_gpio_cfg
	}
#endif
};

static int msm_sdcc_setup_gpio(int dev_id, unsigned int enable)
{
	int rc = 0;
	struct msm_sdcc_pin_cfg *curr;
	int n;

	curr = &sdcc_pin_cfg_data[dev_id - 1];
	if (!curr->gpio_data)
		goto out;

	for (n = 0; n < curr->gpio_data_size; n++) {
		if (enable) {

			if (curr->gpio_data[n].always_on &&
				curr->gpio_data[n].is_enabled)
				continue;
			pr_debug("%s: enable: %s\n", __func__,
					curr->gpio_data[n].name);
			rc = gpio_request(curr->gpio_data[n].no,
				curr->gpio_data[n].name);
			if (rc) {
				pr_err("%s: gpio_request(%d, %s)"
					"failed", __func__,
					curr->gpio_data[n].no,
					curr->gpio_data[n].name);
				goto free_gpios;
			}
			/* set direction as output for all GPIOs */
			rc = gpio_direction_output(
				curr->gpio_data[n].no, 1);
			if (rc) {
				pr_err("%s: gpio_direction_output"
					"(%d, 1) failed\n", __func__,
					curr->gpio_data[n].no);
				goto free_gpios;
			}
			curr->gpio_data[n].is_enabled = 1;
		} else {
			/*
			 * now free this GPIO which will put GPIO
			 * in low power mode and will also put GPIO
			 * in input mode
			 */
			if (curr->gpio_data[n].always_on)
				continue;
			pr_debug("%s: disable: %s\n", __func__,
					curr->gpio_data[n].name);
			gpio_free(curr->gpio_data[n].no);
			curr->gpio_data[n].is_enabled = 0;
		}
	}
	curr->cfg_sts = enable;
	goto out;

free_gpios:
	for (; n >= 0; n--)
		gpio_free(curr->gpio_data[n].no);
out:
	return rc;
}

static int msm_sdcc_setup_pad(int dev_id, unsigned int enable)
{
	int rc = 0;
	struct msm_sdcc_pin_cfg *curr;
	int n;

	curr = &sdcc_pin_cfg_data[dev_id - 1];
	if (!curr->pad_drv_on_data || !curr->pad_pull_on_data)
		goto out;

	if (enable) {
		/*
		 * set up the normal driver strength and
		 * pull config for pads
		 */
		for (n = 0; n < curr->pad_drv_data_size; n++) {
			if (curr->sdio_lpm_gpio_cfg) {
				if (curr->pad_drv_on_data[n].drv ==
						TLMM_HDRV_SDC4_DATA)
					continue;
			}
			msm_tlmm_set_hdrive(curr->pad_drv_on_data[n].drv,
				curr->pad_drv_on_data[n].drv_val);
		}
		for (n = 0; n < curr->pad_pull_data_size; n++) {
			if (curr->sdio_lpm_gpio_cfg) {
				if (curr->pad_pull_on_data[n].pull ==
						TLMM_PULL_SDC4_DATA)
					continue;
			}
			msm_tlmm_set_pull(curr->pad_pull_on_data[n].pull,
				curr->pad_pull_on_data[n].pull_val);
		}
	} else {
		/* set the low power config for pads */
		for (n = 0; n < curr->pad_drv_data_size; n++) {
			if (curr->sdio_lpm_gpio_cfg) {
				if (curr->pad_drv_off_data[n].drv ==
						TLMM_HDRV_SDC4_DATA)
					continue;
			}
			msm_tlmm_set_hdrive(
				curr->pad_drv_off_data[n].drv,
				curr->pad_drv_off_data[n].drv_val);
		}
		for (n = 0; n < curr->pad_pull_data_size; n++) {
			if (curr->sdio_lpm_gpio_cfg) {
				if (curr->pad_pull_off_data[n].pull ==
						TLMM_PULL_SDC4_DATA)
					continue;
			}
			msm_tlmm_set_pull(
				curr->pad_pull_off_data[n].pull,
				curr->pad_pull_off_data[n].pull_val);
		}
	}
	curr->cfg_sts = enable;
out:
	return rc;
}

struct sdcc_reg {
	/* VDD/VCC/VCCQ regulator name on PMIC8058/PMIC8089*/
	const char *reg_name;
	/*
	 * is set voltage supported for this regulator?
	 * 0 = not supported, 1 = supported
	 */
	unsigned char set_voltage_sup;
	/* voltage level to be set */
	unsigned int level;
	/* VDD/VCC/VCCQ voltage regulator handle */
	struct regulator *reg;
	/* is this regulator enabled? */
	bool enabled;
	/* is this regulator needs to be always on? */
	bool always_on;
	/* is operating power mode setting required for this regulator? */
	bool op_pwr_mode_sup;
	/* Load values for low power and high power mode */
	unsigned int lpm_uA;
	unsigned int hpm_uA;
};
/* all SDCC controllers require VDD/VCC voltage */
static struct sdcc_reg sdcc_vdd_reg_data[MAX_SDCC_CONTROLLER];
/* only SDCC1 requires VCCQ voltage */
static struct sdcc_reg sdcc_vccq_reg_data[1];
#if defined(CONFIG_MMC_MSM_SDC3_SUPPORT)
/* all SDCC controllers may require voting for VDD PAD voltage */
static struct sdcc_reg sdcc_vddp_reg_data[MAX_SDCC_CONTROLLER];
#endif

struct sdcc_reg_data {
	struct sdcc_reg *vdd_data; /* keeps VDD/VCC regulator info */
	struct sdcc_reg *vccq_data; /* keeps VCCQ regulator info */
	struct sdcc_reg *vddp_data; /* keeps VDD Pad regulator info */
	unsigned char sts; /* regulator enable/disable status */
};
/* msm8x60 has 5 SDCC controllers */
static struct sdcc_reg_data sdcc_vreg_data[MAX_SDCC_CONTROLLER];

static int msm_sdcc_vreg_init_reg(struct sdcc_reg *vreg)
{
	int rc = 0;

	/* Get the regulator handle */
	vreg->reg = regulator_get(NULL, vreg->reg_name);
	if (IS_ERR(vreg->reg)) {
		rc = PTR_ERR(vreg->reg);
		pr_err("%s: regulator_get(%s) failed. rc=%d\n",
			__func__, vreg->reg_name, rc);
		goto out;
	}

	/* Set the voltage level if required */
	if (vreg->set_voltage_sup) {
		rc = regulator_set_voltage(vreg->reg, vreg->level,
					vreg->level);
		if (rc) {
			pr_err("%s: regulator_set_voltage(%s) failed rc=%d\n",
				__func__, vreg->reg_name, rc);
			goto vreg_put;
		}
	}
	goto out;

vreg_put:
	regulator_put(vreg->reg);
out:
	return rc;
}

static inline void msm_sdcc_vreg_deinit_reg(struct sdcc_reg *vreg)
{
	regulator_put(vreg->reg);
}

/* this init function should be called only once for each SDCC */
static int msm_sdcc_vreg_init(int dev_id, unsigned char init)
{
	int rc = 0;
	struct sdcc_reg *curr_vdd_reg, *curr_vccq_reg, *curr_vddp_reg;
	struct sdcc_reg_data *curr;

	curr = &sdcc_vreg_data[dev_id - 1];
	curr_vdd_reg = curr->vdd_data;
	curr_vccq_reg = curr->vccq_data;
	curr_vddp_reg = curr->vddp_data;

	if (init) {
		/*
		 * get the regulator handle from voltage regulator framework
		 * and then try to set the voltage level for the regulator
		 */
		if (curr_vdd_reg) {
			rc = msm_sdcc_vreg_init_reg(curr_vdd_reg);
			if (rc)
				goto out;
		}
		if (curr_vccq_reg) {
			rc = msm_sdcc_vreg_init_reg(curr_vccq_reg);
			if (rc)
				goto vdd_reg_deinit;
		}
		if (curr_vddp_reg) {
			rc = msm_sdcc_vreg_init_reg(curr_vddp_reg);
			if (rc)
				goto vccq_reg_deinit;
		}
		goto out;
	} else
		/* deregister with all regulators from regulator framework */
		goto vddp_reg_deinit;

vddp_reg_deinit:
	if (curr_vddp_reg)
		msm_sdcc_vreg_deinit_reg(curr_vddp_reg);
vccq_reg_deinit:
	if (curr_vccq_reg)
		msm_sdcc_vreg_deinit_reg(curr_vccq_reg);
vdd_reg_deinit:
	if (curr_vdd_reg)
		msm_sdcc_vreg_deinit_reg(curr_vdd_reg);
out:
	return rc;
}

static int msm_sdcc_vreg_enable(struct sdcc_reg *vreg)
{
	int rc = 0;

	if (!vreg->enabled) {
		rc = regulator_enable(vreg->reg);
		if (rc) {
			pr_err("%s: regulator_enable(%s) failed. rc=%d\n",
				__func__, vreg->reg_name, rc);
			goto out;
		}
		vreg->enabled = 1;
	}

	/* Put always_on regulator in HPM (high power mode) */
	if (vreg->always_on && vreg->op_pwr_mode_sup) {
		rc = regulator_set_optimum_mode(vreg->reg, vreg->hpm_uA);
		if (rc < 0) {
			pr_err("%s: reg=%s: HPM setting failed"
				" hpm_uA=%d, rc=%d\n",
				__func__, vreg->reg_name,
				vreg->hpm_uA, rc);
			goto vreg_disable;
		}
		rc = 0;
	}
	goto out;

vreg_disable:
	regulator_disable(vreg->reg);
	vreg->enabled = 0;
out:
	return rc;
}

static int msm_sdcc_vreg_disable(struct sdcc_reg *vreg)
{
	int rc = 0;

	/* Never disable always_on regulator */
	if (!vreg->always_on) {
		rc = regulator_disable(vreg->reg);
		if (rc) {
			pr_err("%s: regulator_disable(%s) failed. rc=%d\n",
				__func__, vreg->reg_name, rc);
			goto out;
		}
		vreg->enabled = 0;
	}

	/* Put always_on regulator in LPM (low power mode) */
	if (vreg->always_on && vreg->op_pwr_mode_sup) {
		rc = regulator_set_optimum_mode(vreg->reg, vreg->lpm_uA);
		if (rc < 0) {
			pr_err("%s: reg=%s: LPM setting failed"
				" lpm_uA=%d, rc=%d\n",
				__func__,
				vreg->reg_name,
				vreg->lpm_uA, rc);
			goto out;
		}
		rc = 0;
	}

out:
	return rc;
}

static int msm_sdcc_setup_vreg(int dev_id, unsigned char enable)
{
	int rc = 0;
	struct sdcc_reg *curr_vdd_reg, *curr_vccq_reg, *curr_vddp_reg;
	struct sdcc_reg_data *curr;

	curr = &sdcc_vreg_data[dev_id - 1];
	curr_vdd_reg = curr->vdd_data;
	curr_vccq_reg = curr->vccq_data;
	curr_vddp_reg = curr->vddp_data;

	/* check if regulators are initialized or not? */
	if ((curr_vdd_reg && !curr_vdd_reg->reg) ||
		(curr_vccq_reg && !curr_vccq_reg->reg) ||
		(curr_vddp_reg && !curr_vddp_reg->reg)) {
		/* initialize voltage regulators required for this SDCC */
		rc = msm_sdcc_vreg_init(dev_id, 1);
		if (rc) {
			pr_err("%s: regulator init failed = %d\n",
				__func__, rc);
			goto out;
		}
	}

	if (curr->sts == enable)
		goto out;

	if (curr_vdd_reg) {
		if (enable)
			rc = msm_sdcc_vreg_enable(curr_vdd_reg);
		else{
			rc = msm_sdcc_vreg_disable(curr_vdd_reg);
			if (!curr_vdd_reg->always_on)
				usleep_range(10000, 10000);
		}
		if (rc)
			goto out;
	}

	if (curr_vccq_reg) {
		if (enable)
			rc = msm_sdcc_vreg_enable(curr_vccq_reg);
		else
			rc = msm_sdcc_vreg_disable(curr_vccq_reg);
		if (rc)
			goto out;
	}

	if (curr_vddp_reg) {
		if (enable)
			rc = msm_sdcc_vreg_enable(curr_vddp_reg);
		else
			rc = msm_sdcc_vreg_disable(curr_vddp_reg);
		if (rc)
			goto out;
	}
	curr->sts = enable;

out:
	return rc;
}

static u32 msm_sdcc_setup_power(struct device *dv, unsigned int vdd)
{
	u32 rc_pin_cfg = 0;
	u32 rc_vreg_cfg = 0;
	u32 rc = 0;
	struct platform_device *pdev;
	struct msm_sdcc_pin_cfg *curr_pin_cfg;

	pdev = container_of(dv, struct platform_device, dev);

	/* setup gpio/pad */
	curr_pin_cfg = &sdcc_pin_cfg_data[pdev->id - 1];
	if (curr_pin_cfg->cfg_sts == !!vdd)
		goto setup_vreg;

	if (curr_pin_cfg->is_gpio)
		rc_pin_cfg = msm_sdcc_setup_gpio(pdev->id, !!vdd);
	else
		rc_pin_cfg = msm_sdcc_setup_pad(pdev->id, !!vdd);

setup_vreg:
	/* setup voltage regulators */
	rc_vreg_cfg = msm_sdcc_setup_vreg(pdev->id, !!vdd);

	if (rc_pin_cfg || rc_vreg_cfg)
		rc = rc_pin_cfg ? rc_pin_cfg : rc_vreg_cfg;

	return rc;
}

#if (defined(CONFIG_MMC_MSM_SDC2_SUPPORT)\
	|| defined(CONFIG_MMC_MSM_SDC5_SUPPORT))
static void msm_sdcc_sdio_lpm_gpio(struct device *dv, unsigned int active)
{
	struct msm_sdcc_pin_cfg *curr_pin_cfg;
	struct platform_device *pdev;

	pdev = container_of(dv, struct platform_device, dev);
	/* setup gpio/pad */
	curr_pin_cfg = &sdcc_pin_cfg_data[pdev->id - 1];

	if (curr_pin_cfg->cfg_sts == active)
		return;

	curr_pin_cfg->sdio_lpm_gpio_cfg = 1;
	if (curr_pin_cfg->is_gpio)
		msm_sdcc_setup_gpio(pdev->id, active);
	else
		msm_sdcc_setup_pad(pdev->id, active);
	curr_pin_cfg->sdio_lpm_gpio_cfg = 0;
}
#endif

#if defined(CONFIG_MMC_MSM_SDC3_SUPPORT)
static int msm_sdc3_get_wpswitch(struct device *dev)
{
/* Temporary comment out.
 * GPIO_SDC_WP was removed because it is GPIO number for SURF.
 * Please change GPIO_SDC_WP to appropriate GPIO number and remove comment out.

	struct platform_device *pdev;
	int status;
	pdev = container_of(dev, struct platform_device, dev);

	status = gpio_request(GPIO_SDC_WP, "SD_WP_Switch");
	if (status) {
		pr_err("%s:Failed to request GPIO %d\n",
					__func__, GPIO_SDC_WP);
	} else {
		status = gpio_direction_input(GPIO_SDC_WP);
		if (!status) {
			status = gpio_get_value_cansleep(GPIO_SDC_WP);
			pr_info("%s: WP Status for Slot %d = %d\n",
				 __func__, pdev->id, status);
		}
		gpio_free(GPIO_SDC_WP);
	}
	return status;
*/
	return 0;
}
#endif

#ifdef CONFIG_MMC_MSM_SDC5_SUPPORT
int sdc5_register_status_notify(void (*callback)(int, void *),
	void *dev_id)
{
	sdc5_status_notify_cb = callback;
	sdc5_status_notify_cb_devid = dev_id;
	return 0;
}
#endif

#ifdef CONFIG_MMC_MSM_SDC2_SUPPORT
int sdc2_register_status_notify(void (*callback)(int, void *),
	void *dev_id)
{
	sdc2_status_notify_cb = callback;
	sdc2_status_notify_cb_devid = dev_id;
	return 0;
}
#endif

/* Interrupt handler for SDC2 and SDC5 detection
 * This function uses dual-edge interrputs settings in order
 * to get SDIO detection when the GPIO is rising and SDIO removal
 * when the GPIO is falling */
static irqreturn_t msm8x60_multi_sdio_slot_status_irq(int irq, void *dev_id)
{
	int status;

	status = gpio_get_value(MDM2AP_SYNC);
	pr_info("%s: MDM2AP_SYNC Status = %d\n",
		 __func__, status);

#ifdef CONFIG_MMC_MSM_SDC2_SUPPORT
	if (sdc2_status_notify_cb) {
		pr_info("%s: calling sdc2_status_notify_cb\n", __func__);
		sdc2_status_notify_cb(status,
			sdc2_status_notify_cb_devid);
	}
#endif

#ifdef CONFIG_MMC_MSM_SDC5_SUPPORT
	if (sdc5_status_notify_cb) {
		pr_info("%s: calling sdc5_status_notify_cb\n", __func__);
		sdc5_status_notify_cb(status,
			sdc5_status_notify_cb_devid);
	}
#endif
	return IRQ_HANDLED;
}

static int msm8x60_multi_sdio_init(void)
{
	int ret, irq_num;

	ret = msm_gpiomux_get(MDM2AP_SYNC);
	if (ret) {
		pr_err("%s:Failed to request GPIO %d, ret=%d\n",
					__func__, MDM2AP_SYNC, ret);
		return ret;
	}

	irq_num = gpio_to_irq(MDM2AP_SYNC);

	ret = request_irq(irq_num,
		msm8x60_multi_sdio_slot_status_irq,
		IRQ_TYPE_EDGE_BOTH,
		"sdio_multidetection", NULL);

	if (ret) {
		pr_err("%s:Failed to request irq, ret=%d\n",
					__func__, ret);
		return ret;
	}

	return ret;
}

#ifdef CONFIG_MMC_MSM_SDC3_SUPPORT
#ifdef CONFIG_MMC_MSM_CARD_HW_DETECTION
static unsigned int msm8x60_sdcc_slot_status(struct device *dev)
{
	int status;

	status = gpio_request(PM8058_GPIO_PM_TO_SYS(PMIC_GPIO_SDC3_DET - 1)
				, "SD_HW_Detect");
	if (status) {
		pr_err("%s:Failed to request GPIO %d\n", __func__,
				PM8058_GPIO_PM_TO_SYS(PMIC_GPIO_SDC3_DET - 1));
	} else {
		status = gpio_direction_input(
				PM8058_GPIO_PM_TO_SYS(PMIC_GPIO_SDC3_DET - 1));
		if (!status)
			status = !(gpio_get_value_cansleep(
				PM8058_GPIO_PM_TO_SYS(PMIC_GPIO_SDC3_DET - 1)));
		gpio_free(PM8058_GPIO_PM_TO_SYS(PMIC_GPIO_SDC3_DET - 1));
	}
	return (unsigned int) status;
}
#endif
#endif

#ifdef	CONFIG_MMC_MSM_SDC4_SUPPORT
static int msm_sdcc_cfg_mpm_sdiowakeup(struct device *dev, unsigned mode)
{
	struct platform_device *pdev;
	enum msm_mpm_pin pin;
	int ret = 0;

	pdev = container_of(dev, struct platform_device, dev);

	/* Only SDCC4 slot connected to WLAN chip has wakeup capability */
	if (pdev->id == 4)
		pin = MSM_MPM_PIN_SDC4_DAT1;
	else
		return -EINVAL;

	switch (mode) {
	case SDC_DAT1_DISABLE:
		ret = msm_mpm_enable_pin(pin, 0);
		break;
	case SDC_DAT1_ENABLE:
		ret = msm_mpm_set_pin_type(pin, IRQ_TYPE_LEVEL_LOW);
		ret = msm_mpm_enable_pin(pin, 1);
		break;
	case SDC_DAT1_ENWAKE:
		ret = msm_mpm_set_pin_wake(pin, 1);
		break;
	case SDC_DAT1_DISWAKE:
		ret = msm_mpm_set_pin_wake(pin, 0);
		break;
	default:
		ret = -EINVAL;
		break;
	}
	return ret;
}
#endif
#endif

#ifdef CONFIG_MMC_MSM_SDC1_SUPPORT
static struct mmc_platform_data msm8x60_sdc1_data = {
	.ocr_mask       = MMC_VDD_27_28 | MMC_VDD_28_29,
	.translate_vdd  = msm_sdcc_setup_power,
#ifdef CONFIG_MMC_MSM_SDC1_8_BIT_SUPPORT
	.mmc_bus_width  = MMC_CAP_8_BIT_DATA,
#else
	.mmc_bus_width  = MMC_CAP_4_BIT_DATA,
#endif
	.msmsdcc_fmin	= 400000,
	.msmsdcc_fmid	= 24000000,
	.msmsdcc_fmax	= 48000000,
	.nonremovable	= 1,
	.pclk_src_dfab	= 1,
};
#endif

#ifdef CONFIG_MMC_MSM_SDC2_SUPPORT
static struct mmc_platform_data msm8x60_sdc2_data = {
	.ocr_mask       = MMC_VDD_27_28 | MMC_VDD_28_29 | MMC_VDD_165_195,
	.translate_vdd  = msm_sdcc_setup_power,
	.sdio_lpm_gpio_setup = msm_sdcc_sdio_lpm_gpio,
	.mmc_bus_width  = MMC_CAP_8_BIT_DATA,
	.msmsdcc_fmin	= 400000,
	.msmsdcc_fmid	= 24000000,
	.msmsdcc_fmax	= 48000000,
	.nonremovable	= 0,
	.pclk_src_dfab  = 1,
	.register_status_notify = sdc2_register_status_notify,
#ifdef CONFIG_MMC_MSM_SDC2_DUMMY52_REQUIRED
	.dummy52_required = 1,
#endif
#ifdef CONFIG_MSM_SDIO_AL
	.is_sdio_al_client = 1,
#endif
};
#endif

#ifdef CONFIG_MMC_MSM_SDC3_SUPPORT
static struct mmc_platform_data msm8x60_sdc3_data = {
	.ocr_mask       = MMC_VDD_27_28 | MMC_VDD_28_29,
	.translate_vdd  = msm_sdcc_setup_power,
	.mmc_bus_width  = MMC_CAP_4_BIT_DATA,
	.wpswitch       = msm_sdc3_get_wpswitch,
#ifdef CONFIG_MMC_MSM_CARD_HW_DETECTION
	.status      = msm8x60_sdcc_slot_status,
	.status_irq  = PM8058_GPIO_IRQ(PM8058_IRQ_BASE,
				       PMIC_GPIO_SDC3_DET - 1),
	.irq_flags   = IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
#endif
	.msmsdcc_fmin	= 400000,
	.msmsdcc_fmid	= 24000000,
	.msmsdcc_fmax	= 48000000,
	.nonremovable	= 0,
	.pclk_src_dfab  = 1,
};
#endif

#ifdef CONFIG_MMC_MSM_SDC4_SUPPORT
static struct mmc_platform_data msm8x60_sdc4_data = {
	.ocr_mask       = MMC_VDD_27_28 | MMC_VDD_28_29,
	.translate_vdd  = msm_sdcc_setup_power,
	.mmc_bus_width  = MMC_CAP_4_BIT_DATA,
	.msmsdcc_fmin	= 400000,
	.msmsdcc_fmid	= 24000000,
	.msmsdcc_fmax	= 48000000,
	.nonremovable	= 1,
	.pclk_src_dfab  = 1,
	.status			= fuji_wifi_status,
	.register_status_notify = fuji_wifi_status_register,
	.cfg_mpm_sdiowakeup = msm_sdcc_cfg_mpm_sdiowakeup,
};
#endif

#ifdef CONFIG_MMC_MSM_SDC5_SUPPORT
static struct mmc_platform_data msm8x60_sdc5_data = {
	.ocr_mask       = MMC_VDD_27_28 | MMC_VDD_28_29 | MMC_VDD_165_195,
	.translate_vdd  = msm_sdcc_setup_power,
	.sdio_lpm_gpio_setup = msm_sdcc_sdio_lpm_gpio,
	.mmc_bus_width  = MMC_CAP_4_BIT_DATA,
	.msmsdcc_fmin	= 400000,
	.msmsdcc_fmid	= 24000000,
	.msmsdcc_fmax	= 48000000,
	.nonremovable	= 0,
	.pclk_src_dfab  = 1,
	.register_status_notify = sdc5_register_status_notify,
#ifdef CONFIG_MMC_MSM_SDC5_DUMMY52_REQUIRED
	.dummy52_required = 1,
#endif
#ifdef CONFIG_MSM_SDIO_AL
	.is_sdio_al_client = 1,
#endif
};
#endif

static void __init msm8x60_init_mmc(void)
{
#ifdef CONFIG_MMC_MSM_SDC1_SUPPORT
	/* SDCC1 : eMMC card connected */
	sdcc_vreg_data[0].vdd_data = &sdcc_vdd_reg_data[0];
	sdcc_vreg_data[0].vdd_data->reg_name = "8901_l5";
	sdcc_vreg_data[0].vdd_data->set_voltage_sup = 1;
	sdcc_vreg_data[0].vdd_data->level = 2850000;
	sdcc_vreg_data[0].vdd_data->always_on = 1;
	sdcc_vreg_data[0].vdd_data->op_pwr_mode_sup = 1;
	sdcc_vreg_data[0].vdd_data->lpm_uA = 9000;
	sdcc_vreg_data[0].vdd_data->hpm_uA = 200000;

	sdcc_vreg_data[0].vccq_data = &sdcc_vccq_reg_data[0];
	sdcc_vreg_data[0].vccq_data->reg_name = "8901_lvs0";
	sdcc_vreg_data[0].vccq_data->set_voltage_sup = 0;
	sdcc_vreg_data[0].vccq_data->always_on = 1;

	msm_add_sdcc(1, &msm8x60_sdc1_data);
#endif
#ifdef CONFIG_MMC_MSM_SDC2_SUPPORT
	/*
	 * MDM SDIO client is connected to SDC2 on charm SURF/FFA
	 * and no card is connected on 8660 SURF/FFA/FLUID.
	 */
	sdcc_vreg_data[1].vdd_data = &sdcc_vdd_reg_data[1];
	sdcc_vreg_data[1].vdd_data->reg_name = "8058_s3";
	sdcc_vreg_data[1].vdd_data->set_voltage_sup = 1;
	sdcc_vreg_data[1].vdd_data->level = 1800000;

	sdcc_vreg_data[1].vccq_data = NULL;

	msm8x60_sdc2_data.msmsdcc_fmax = 24000000;
#ifdef CONFIG_MMC_MSM_SDIO_SUPPORT
	msm8x60_sdc2_data.sdiowakeup_irq = gpio_to_irq(144);
	msm_sdcc_setup_gpio(2, 1);
#endif
	msm_add_sdcc(2, &msm8x60_sdc2_data);
#endif
#ifdef CONFIG_MMC_MSM_SDC3_SUPPORT
	/* SDCC3 : External card slot connected */
	sdcc_vreg_data[2].vdd_data = &sdcc_vdd_reg_data[2];
	sdcc_vreg_data[2].vdd_data->reg_name = "8058_l14";
	sdcc_vreg_data[2].vdd_data->set_voltage_sup = 1;
	sdcc_vreg_data[2].vdd_data->level = 2850000;
	sdcc_vreg_data[2].vdd_data->always_on = 0;
	sdcc_vreg_data[2].vdd_data->op_pwr_mode_sup = 1;
	sdcc_vreg_data[2].vdd_data->lpm_uA = 9000;
	sdcc_vreg_data[2].vdd_data->hpm_uA = 200000;

	sdcc_vreg_data[2].vccq_data = NULL;

	sdcc_vreg_data[2].vddp_data = &sdcc_vddp_reg_data[2];
	sdcc_vreg_data[2].vddp_data->reg_name = "8058_l5";
	sdcc_vreg_data[2].vddp_data->set_voltage_sup = 1;
	sdcc_vreg_data[2].vddp_data->level = 2850000;
	sdcc_vreg_data[2].vddp_data->always_on = 1;
	sdcc_vreg_data[2].vddp_data->op_pwr_mode_sup = 1;
	/* Sleep current required is ~300 uA. But min. RPM
	 * vote can be in terms of mA (min. 1 mA).
	 * So let's vote for 2 mA during sleep.
	 */
	sdcc_vreg_data[2].vddp_data->lpm_uA = 2000;
	/* Max. Active current required is 16 mA */
	sdcc_vreg_data[2].vddp_data->hpm_uA = 16000;

	msm_add_sdcc(3, &msm8x60_sdc3_data);
#endif
#ifdef CONFIG_MMC_MSM_SDC4_SUPPORT
	/* SDCC4 : WLAN BCM4330 chip is connected */
	sdcc_vreg_data[3].vdd_data = &sdcc_vdd_reg_data[3];
	sdcc_vreg_data[3].vdd_data->reg_name = "8058_s3";
	sdcc_vreg_data[3].vdd_data->set_voltage_sup = 1;
	sdcc_vreg_data[3].vdd_data->level = 1800000;

	sdcc_vreg_data[3].vccq_data = NULL;

	msm_add_sdcc(4, &msm8x60_sdc4_data);
#endif
#ifdef CONFIG_MMC_MSM_SDC5_SUPPORT
	/*
	 * MDM SDIO client is connected to SDC5 on charm SURF/FFA
	 * and no card is connected on 8660 SURF/FFA/FLUID.
	 */
	sdcc_vreg_data[4].vdd_data = &sdcc_vdd_reg_data[4];
	sdcc_vreg_data[4].vdd_data->reg_name = "8058_s3";
	sdcc_vreg_data[4].vdd_data->set_voltage_sup = 1;
	sdcc_vreg_data[4].vdd_data->level = 1800000;

	sdcc_vreg_data[4].vccq_data = NULL;

	msm8x60_sdc5_data.msmsdcc_fmax = 24000000;
#ifdef CONFIG_MMC_MSM_SDIO_SUPPORT
	msm8x60_sdc5_data.sdiowakeup_irq = gpio_to_irq(99);
	msm_sdcc_setup_gpio(5, 1);
#endif
	msm_add_sdcc(5, &msm8x60_sdc5_data);
#endif
}

#ifdef CONFIG_FB_MSM_HDMI_MSM_PANEL
#define _GET_REGULATOR(var, name) do {				\
	var = regulator_get(NULL, name);			\
	if (IS_ERR(var)) {					\
		pr_err("'%s' regulator not found, rc=%ld\n",	\
			name, IS_ERR(var));			\
		var = NULL;					\
		return -ENODEV;					\
	}							\
} while (0)

static int hdmi_enable_5v(int on)
{
	static struct regulator *reg_8901_hdmi_mvs;	/* HDMI_5V */
	static struct regulator *reg_8901_mpp0;		/* External 5V */
	static int prev_on;
	int rc;

	if (on == prev_on)
		return 0;

	if (!reg_8901_hdmi_mvs)
		_GET_REGULATOR(reg_8901_hdmi_mvs, "8901_hdmi_mvs");
	if (!reg_8901_mpp0)
		_GET_REGULATOR(reg_8901_mpp0, "8901_mpp0");

	if (on) {
		rc = regulator_enable(reg_8901_mpp0);
		if (rc) {
			pr_err("'%s' regulator enable failed, rc=%d\n",
				"reg_8901_mpp0", rc);
			return rc;
		}
		rc = regulator_enable(reg_8901_hdmi_mvs);
		if (rc) {
			pr_err("'%s' regulator enable failed, rc=%d\n",
				"8901_hdmi_mvs", rc);
			return rc;
		}
		pr_info("%s(on): success\n", __func__);
	} else {
		rc = regulator_disable(reg_8901_hdmi_mvs);
		if (rc)
			pr_warning("'%s' regulator disable failed, rc=%d\n",
				"8901_hdmi_mvs", rc);
		rc = regulator_disable(reg_8901_mpp0);
		if (rc)
			pr_warning("'%s' regulator disable failed, rc=%d\n",
				"reg_8901_mpp0", rc);
		pr_info("%s(off): success\n", __func__);
	}

	prev_on = on;

	return 0;
}

static int hdmi_core_power(int on, int show)
{
	static struct regulator *reg_8058_l16;		/* VDD_HDMI */
	static int prev_on;
	int rc;

	if (on == prev_on)
		return 0;

	if (!reg_8058_l16)
		_GET_REGULATOR(reg_8058_l16, "8058_l16");

	if (on) {
		rc = regulator_set_voltage(reg_8058_l16, 1800000, 1800000);
		if (!rc)
			rc = regulator_enable(reg_8058_l16);
		if (rc) {
			pr_err("'%s' regulator enable failed, rc=%d\n",
				"8058_l16", rc);
			return rc;
		}
		rc = gpio_request(170, "HDMI_DDC_CLK");
		if (rc) {
			pr_err("'%s'(%d) gpio_request failed, rc=%d\n",
				"HDMI_DDC_CLK", 170, rc);
			goto error1;
		}
		rc = gpio_request(171, "HDMI_DDC_DATA");
		if (rc) {
			pr_err("'%s'(%d) gpio_request failed, rc=%d\n",
				"HDMI_DDC_DATA", 171, rc);
			goto error2;
		}
		rc = gpio_request(172, "HDMI_HPD");
		if (rc) {
			pr_err("'%s'(%d) gpio_request failed, rc=%d\n",
				"HDMI_HPD", 172, rc);
			goto error3;
		}
		pr_info("%s(on): success\n", __func__);
	} else {
		gpio_free(170);
		gpio_free(171);
		gpio_free(172);
		rc = regulator_disable(reg_8058_l16);
		if (rc)
			pr_warning("'%s' regulator disable failed, rc=%d\n",
				"8058_l16", rc);
		pr_info("%s(off): success\n", __func__);
	}

	prev_on = on;

	return 0;

error3:
	gpio_free(171);
error2:
	gpio_free(170);
error1:
	regulator_disable(reg_8058_l16);
	return rc;
}

static int hdmi_cec_power(int on)
{
	static int prev_on;
	int rc;

	if (on == prev_on)
		return 0;

	if (on) {
		rc = gpio_request(169, "HDMI_CEC_VAR");
		if (rc) {
			pr_err("'%s'(%d) gpio_request failed, rc=%d\n",
				"HDMI_CEC_VAR", 169, rc);
			goto error;
		}
		pr_info("%s(on): success\n", __func__);
	} else {
		gpio_free(169);
		pr_info("%s(off): success\n", __func__);
	}

	prev_on = on;

	return 0;
error:
	return rc;
}

#undef _GET_REGULATOR

#endif /* CONFIG_FB_MSM_HDMI_MSM_PANEL */

#ifdef CONFIG_MSM_BUS_SCALING

static struct msm_bus_vectors rotator_init_vectors[] = {
	{
		.src = MSM_BUS_MASTER_ROTATOR,
		.dst = MSM_BUS_SLAVE_SMI,
		.ab = 0,
		.ib = 0,
	},
	{
		.src = MSM_BUS_MASTER_ROTATOR,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab = 0,
		.ib = 0,
	},
};

static struct msm_bus_vectors rotator_ui_vectors[] = {
	{
		.src = MSM_BUS_MASTER_ROTATOR,
		.dst = MSM_BUS_SLAVE_SMI,
		.ab  = 0,
		.ib  = 0,
	},
	{
		.src = MSM_BUS_MASTER_ROTATOR,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = (1024 * 600 * 4 * 2 * 60),
		.ib  = (1024 * 600 * 4 * 2 * 60 * 1.5),
	},
};

static struct msm_bus_vectors rotator_vga_vectors[] = {
	{
		.src = MSM_BUS_MASTER_ROTATOR,
		.dst = MSM_BUS_SLAVE_SMI,
		.ab  = (640 * 480 * 2 * 2 * 30),
		.ib  = (640 * 480 * 2 * 2 * 30 * 1.5),
	},
	{
		.src = MSM_BUS_MASTER_ROTATOR,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = (640 * 480 * 2 * 2 * 30),
		.ib  = (640 * 480 * 2 * 2 * 30 * 1.5),
	},
};

static struct msm_bus_vectors rotator_720p_vectors[] = {
	{
		.src = MSM_BUS_MASTER_ROTATOR,
		.dst = MSM_BUS_SLAVE_SMI,
		.ab  = (1280 * 736 * 2 * 2 * 30),
		.ib  = (1280 * 736 * 2 * 2 * 30 * 1.5),
	},
	{
		.src = MSM_BUS_MASTER_ROTATOR,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = (1280 * 736 * 2 * 2 * 30),
		.ib  = (1280 * 736 * 2 * 2 * 30 * 1.5),
	},
};

static struct msm_bus_vectors rotator_1080p_vectors[] = {
	{
		.src = MSM_BUS_MASTER_ROTATOR,
		.dst = MSM_BUS_SLAVE_SMI,
		.ab  = (1920 * 1088 * 2 * 2 * 30),
		.ib  = (1920 * 1088 * 2 * 2 * 30 * 1.5),
	},
	{
		.src = MSM_BUS_MASTER_ROTATOR,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = (1920 * 1088 * 2 * 2 * 30),
		.ib  = (1920 * 1088 * 2 * 2 * 30 * 1.5),
	},
};

static struct msm_bus_paths rotator_bus_scale_usecases[] = {
	{
		ARRAY_SIZE(rotator_init_vectors),
		rotator_init_vectors,
	},
	{
		ARRAY_SIZE(rotator_ui_vectors),
		rotator_ui_vectors,
	},
	{
		ARRAY_SIZE(rotator_vga_vectors),
		rotator_vga_vectors,
	},
	{
		ARRAY_SIZE(rotator_720p_vectors),
		rotator_720p_vectors,
	},
	{
		ARRAY_SIZE(rotator_1080p_vectors),
		rotator_1080p_vectors,
	},
};

struct msm_bus_scale_pdata rotator_bus_scale_pdata = {
	rotator_bus_scale_usecases,
	ARRAY_SIZE(rotator_bus_scale_usecases),
	.name = "rotator",
};

static struct msm_bus_vectors mdp_init_vectors[] = {
	/* For now, 0th array entry is reserved.
	 * Please leave 0 as is and don't use it
	 */
	{
		.src = MSM_BUS_MASTER_MDP_PORT0,
		.dst = MSM_BUS_SLAVE_SMI,
		.ab = 0,
		.ib = 0,
	},
	/* Master and slaves can be from different fabrics */
	{
		.src = MSM_BUS_MASTER_MDP_PORT0,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab = 0,
		.ib = 0,
	},
};

#ifdef CONFIG_FB_MSM_HDMI_AS_PRIMARY
static struct msm_bus_vectors hdmi_as_primary_vectors[] = {
	/* If HDMI is used as primary */
	 {
		.src = MSM_BUS_MASTER_MDP_PORT0,
		.dst = MSM_BUS_SLAVE_SMI,
		.ab = 2000000000,
		.ib = 2000000000,
	 },
	 /* Master and slaves can be from different fabrics */
	 {
		.src = MSM_BUS_MASTER_MDP_PORT0,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab = 2000000000,
		.ib = 2000000000,
	 },
};

static struct msm_bus_paths mdp_bus_scale_usecases[] = {
	{
		ARRAY_SIZE(mdp_init_vectors),
		mdp_init_vectors,
	},
	{
		ARRAY_SIZE(hdmi_as_primary_vectors),
		hdmi_as_primary_vectors,
	},
	{
		ARRAY_SIZE(hdmi_as_primary_vectors),
		hdmi_as_primary_vectors,
	},
	{
		ARRAY_SIZE(hdmi_as_primary_vectors),
		hdmi_as_primary_vectors,
	},
	{
		ARRAY_SIZE(hdmi_as_primary_vectors),
		hdmi_as_primary_vectors,
	},
	{
		ARRAY_SIZE(hdmi_as_primary_vectors),
		hdmi_as_primary_vectors,
	},
};
#else
static struct msm_bus_vectors mdp_sd_smi_vectors[] = {
	/* Default case static display/UI/2d/3d if FB SMI */
	{
		.src = MSM_BUS_MASTER_MDP_PORT0,
		.dst = MSM_BUS_SLAVE_SMI,
		.ab = 407808000,
		.ib = 1019520000,
	},
	/* Master and slaves can be from different fabrics */
	{
		.src = MSM_BUS_MASTER_MDP_PORT0,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab = 0,
		.ib = 0,
	},
};

static struct msm_bus_vectors mdp_sd_ebi_vectors[] = {
	/* Default case static display/UI/2d/3d if FB SMI */
	{
		.src = MSM_BUS_MASTER_MDP_PORT0,
		.dst = MSM_BUS_SLAVE_SMI,
		.ab = 0,
		.ib = 0,
	},
	/* Master and slaves can be from different fabrics */
	{
		.src = MSM_BUS_MASTER_MDP_PORT0,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab = 407808000,
		.ib = 509760000 * 2,
	},
};
static struct msm_bus_vectors mdp_vga_vectors[] = {
	/* VGA and less video */
	{
		.src = MSM_BUS_MASTER_MDP_PORT0,
		.dst = MSM_BUS_SLAVE_SMI,
		.ab = 850000000,
		.ib = 2100000000,
	},
	{
		.src = MSM_BUS_MASTER_MDP_PORT0,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab = 850000000,
		.ib = 2100000000,
	},
};

static struct msm_bus_vectors mdp_720p_vectors[] = {
	/* 720p and less video */
	{
		.src = MSM_BUS_MASTER_MDP_PORT0,
		.dst = MSM_BUS_SLAVE_SMI,
		.ab = 850000000,
		.ib = 2100000000,
	},
	/* Master and slaves can be from different fabrics */
	{
		.src = MSM_BUS_MASTER_MDP_PORT0,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab = 850000000,
		.ib = 2100000000,
	},
};

static struct msm_bus_vectors mdp_1080p_vectors[] = {
	/* 1080p and less video */
	{
		.src = MSM_BUS_MASTER_MDP_PORT0,
		.dst = MSM_BUS_SLAVE_SMI,
		.ab = 850000000,
		.ib = 2100000000,
	},
	/* Master and slaves can be from different fabrics */
	{
		.src = MSM_BUS_MASTER_MDP_PORT0,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab = 850000000,
		.ib = 2100000000,
	},
};

static struct msm_bus_paths mdp_bus_scale_usecases[] = {
	{
		ARRAY_SIZE(mdp_init_vectors),
		mdp_init_vectors,
	},
	{
		ARRAY_SIZE(mdp_sd_smi_vectors),
		mdp_sd_smi_vectors,
	},
	{
		ARRAY_SIZE(mdp_sd_ebi_vectors),
		mdp_sd_ebi_vectors,
	},
	{
		ARRAY_SIZE(mdp_vga_vectors),
		mdp_vga_vectors,
	},
	{
		ARRAY_SIZE(mdp_720p_vectors),
		mdp_720p_vectors,
	},
	{
		ARRAY_SIZE(mdp_1080p_vectors),
		mdp_1080p_vectors,
	},
};
#endif
static struct msm_bus_scale_pdata mdp_bus_scale_pdata = {
	mdp_bus_scale_usecases,
	ARRAY_SIZE(mdp_bus_scale_usecases),
	.name = "mdp",
};

#endif
#ifdef CONFIG_MSM_BUS_SCALING
static struct msm_bus_vectors dtv_bus_init_vectors[] = {
	/* For now, 0th array entry is reserved.
	 * Please leave 0 as is and don't use it
	 */
	{
		.src = MSM_BUS_MASTER_MDP_PORT0,
		.dst = MSM_BUS_SLAVE_SMI,
		.ab = 0,
		.ib = 0,
	},
	/* Master and slaves can be from different fabrics */
	{
		.src = MSM_BUS_MASTER_MDP_PORT0,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab = 0,
		.ib = 0,
	},
};

static struct msm_bus_vectors dtv_bus_def_vectors[] = {
	/* For now, 0th array entry is reserved.
	 * Please leave 0 as is and don't use it
	 */
	{
		.src = MSM_BUS_MASTER_MDP_PORT0,
		.dst = MSM_BUS_SLAVE_SMI,
		.ab = 566092800,
		.ib = 707616000,
	},
	/* Master and slaves can be from different fabrics */
	{
		.src = MSM_BUS_MASTER_MDP_PORT0,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab = 566092800,
		.ib = 707616000,
	},
};

static struct msm_bus_vectors dtv_bus_hdmi_prim_vectors[] = {
	/* For now, 0th array entry is reserved.
	 * Please leave 0 as is and don't use it
	 */
	{
		.src = MSM_BUS_MASTER_MDP_PORT0,
		.dst = MSM_BUS_SLAVE_SMI,
		.ab = 2000000000,
		.ib = 2000000000,
	},
	/* Master and slaves can be from different fabrics */
	{
		.src = MSM_BUS_MASTER_MDP_PORT0,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab = 2000000000,
		.ib = 2000000000,
	},
};

static struct msm_bus_paths dtv_bus_scale_usecases[] = {
	{
		ARRAY_SIZE(dtv_bus_init_vectors),
		dtv_bus_init_vectors,
	},
	{
		ARRAY_SIZE(dtv_bus_def_vectors),
		dtv_bus_def_vectors,
	},
};

static struct msm_bus_scale_pdata dtv_bus_scale_pdata = {
	dtv_bus_scale_usecases,
	ARRAY_SIZE(dtv_bus_scale_usecases),
	.name = "dtv",
};

static struct lcdc_platform_data dtv_pdata = {
	.bus_scale_table = &dtv_bus_scale_pdata,
};

static struct msm_bus_paths dtv_hdmi_prim_bus_scale_usecases[] = {
	{
		ARRAY_SIZE(dtv_bus_init_vectors),
		dtv_bus_init_vectors,
	},
	{
		ARRAY_SIZE(dtv_bus_hdmi_prim_vectors),
		dtv_bus_hdmi_prim_vectors,
	},
};

static struct msm_bus_scale_pdata dtv_hdmi_prim_bus_scale_pdata = {
	dtv_hdmi_prim_bus_scale_usecases,
	ARRAY_SIZE(dtv_hdmi_prim_bus_scale_usecases),
	.name = "dtv",
};

static struct lcdc_platform_data dtv_hdmi_prim_pdata = {
	.bus_scale_table = &dtv_hdmi_prim_bus_scale_pdata,
};
#endif

#define MDP_VSYNC_GPIO			28

static int mipi_dsi_power(int on);

#ifdef CONFIG_FB_MSM_MIPI_DSI_RENESAS_R63306

#define MIPI_DSI_RENESAS_NAME "mipi_renesas_r63306"
#define LCD_PWR_EN 18
#define MLCD_RESET_N 70

static int lcd_gpio_setup(int request);
static int lcd_reset(int on);

static int r63306_vreg_power(int on)
{
	static struct regulator *vreg_lcd_vci;
	static struct regulator *vreg_lcd_vddio;
	int rc = 0;

	if (!vreg_lcd_vci) {
		vreg_lcd_vci = regulator_get(NULL, "8901_l2");
		if (IS_ERR(vreg_lcd_vci)) {
			pr_err("%s: Unable to get 8901_l2\n", __func__);
			vreg_lcd_vci = NULL;
			return -ENODEV;
		}
	}

	if (!vreg_lcd_vddio) {
		vreg_lcd_vddio = regulator_get(NULL, "8901_lvs1");
		if (IS_ERR(vreg_lcd_vddio)) {
			pr_err("%s: Unable to get 8901_lvs1\n", __func__);
			vreg_lcd_vddio = NULL;
			return -ENODEV;
		}
	}

	if (on) {
		rc = regulator_set_voltage(vreg_lcd_vci, 2850000, 2850000);
		if (rc) {
			pr_err("%s:%d unable to set L2 voltage to 2.8V\n",
				__func__, rc);
			regulator_put(vreg_lcd_vci);
			regulator_put(vreg_lcd_vddio);
			return rc;
		}

		rc = regulator_enable(vreg_lcd_vci);
		if (rc) {
			pr_err("%s: Enable regulator 8901_l2 failed\n",
				__func__);
			regulator_put(vreg_lcd_vci);
			regulator_put(vreg_lcd_vddio);
			return rc;
		}

		rc = regulator_enable(vreg_lcd_vddio);
		if (rc) {
			pr_err("%s: Enable regulator 8901_lvs1 failed\n",
				__func__);
			regulator_put(vreg_lcd_vci);
			regulator_put(vreg_lcd_vddio);
			return rc;
		}

		lcd_gpio_setup(1);
		gpio_set_value(MLCD_RESET_N, 0);
		usleep(10000);
		gpio_set_value(MLCD_RESET_N, 1);
		usleep(10000);
	} else {
		gpio_set_value(MLCD_RESET_N, 1);
		gpio_set_value(MLCD_RESET_N, 0);
		lcd_gpio_setup(0);
		usleep(10000);
		rc = regulator_disable(vreg_lcd_vci);
		if (rc)
			pr_warning("%s: '%s' regulator disable failed, rc=%d\n",
				__func__, "8901_l2", rc);
		rc = regulator_disable(vreg_lcd_vddio);
		if (rc)
			pr_warning("%s: '%s' regulator disable failed, rc=%d\n",
				__func__, "8901_lvs1", rc);
	}

	return 0;
}

static int r63306_lcd_power(int on)
{
	static int curr_power;
	int rc;

	if (curr_power == on)
		return 0;

	if (on) {
		rc = r63306_vreg_power(on);
		if (!rc)
			rc = mipi_dsi_power(on);
	} else {
		rc = mipi_dsi_power(on);
		if (!rc)
			rc = r63306_vreg_power(on);
	}

	if (!rc)
		curr_power = on;

	return rc;
}

static int lcd_gpio_setup(int request)
{
	int rc = 0;

	if (!!request) {
		/* LCD_PWR_EN */
		rc = gpio_request(LCD_PWR_EN, "lcd power gpio");
		if (rc) {
			pr_err("%s: GPIO %d: request failed. rc=%d\n",
			       __func__, LCD_PWR_EN, rc);
			return rc;
		}
		rc = gpio_direction_output(LCD_PWR_EN, 0);
		if (rc) {
			pr_err("%s: GPIO %d: direction out failed. rc=%d\n",
			       __func__, LCD_PWR_EN, rc);
			goto out_pwr;
		}
		/* MLCD_RESET_N */
		rc = gpio_request(MLCD_RESET_N, "lcd reset");
		if (rc) {
			pr_err("%s: GPIO %d: request failed. rc=%d\n",
			       __func__, MLCD_RESET_N, rc);
			goto out_pwr;
		}
		rc = gpio_direction_output(MLCD_RESET_N, 0);
		if (rc) {
			pr_err("%s: GPIO %d: direction out failed. rc=%d\n",
			       __func__, MLCD_RESET_N, rc);
			goto out_reset;
		}
	} else {
		gpio_free(LCD_PWR_EN);
		gpio_free(MLCD_RESET_N);
	}

	return rc;
out_reset:
	gpio_free(MLCD_RESET_N);
out_pwr:
	gpio_free(LCD_PWR_EN);
	return rc;
}

static int lcd_power(int on)
{
	if (on)
		gpio_set_value(LCD_PWR_EN, 1);
	else
		gpio_set_value(LCD_PWR_EN, 0);
	msleep(50);

	return 0;
}

static int lcd_reset(int on)
{
	if (on) {
		gpio_set_value(MLCD_RESET_N, 0);
		usleep(10000);
		gpio_set_value(MLCD_RESET_N, 1);
	} else {
		gpio_set_value(MLCD_RESET_N, 1);
		usleep(10000);
		gpio_set_value(MLCD_RESET_N, 0);
	}
	usleep(10000);

	return 0;
}

static const struct panel_id *default_panel_ids_r63306[] = {
	NULL,
};

static const struct panel_id *panel_ids_r63306[] = {
#ifdef CONFIG_FB_MSM_MIPI_R63306_PANEL_TMD_MDW30
	&tmd_video_wxga_mdw30_panel_id_00,
	&tmd_video_wxga_mdw30_panel_id,
#endif /* CONFIG_FB_MSM_MIPI_R63306_PANEL_TMD_MDW30 */
#ifdef CONFIG_FB_MSM_MIPI_R63306_PANEL_TMD_MDV20
	&tmd_video_wxga_mdv20_panel_id_00,
	&tmd_video_wxga_mdv20_panel_id_01,
	&tmd_video_wxga_mdv20_panel_id_02,
	&tmd_video_wxga_mdv20_panel_id,
#endif /* CONFIG_FB_MSM_MIPI_R63306_PANEL_TMD_MDV20 */
#ifdef CONFIG_FB_MSM_MIPI_R63306_PANEL_TMD_MDV22
	&tmd_video_wxga_mdv22_panel_id_00,
	&tmd_video_wxga_mdv22_panel_id_01,
	&tmd_video_wxga_mdv22_panel_id_02,
	&tmd_video_wxga_mdv22_panel_id,
#endif /* CONFIG_FB_MSM_MIPI_R63306_PANEL_TMD_MDV22 */
#ifdef CONFIG_FB_MSM_MIPI_R63306_PANEL_HITACHI_DX12D900VM
	&hitachi_dx12d900vm_panel_id,
#endif /* CONFIG_FB_MSM_MIPI_R63306_PANEL_HITACHI_DX12D900VM */
#ifdef CONFIG_FB_MSM_MIPI_R63306_PANEL_SHARP_LS046K3SX01
	&sharp_ls046k3sx01_panel_id,
#endif /* CONFIG_FB_MSM_MIPI_R63306_PANEL_SHARP_LS046K3SX01 */
#ifdef CONFIG_FB_MSM_MIPI_R63306_PANEL_SHARP_LS043K3SX01F
	&sharp_ls043k3sx01_panel_id,
#endif /* CONFIG_FB_MSM_MIPI_R63306_PANEL_SHARP_LS043K3SX01F */
#ifdef CONFIG_FB_MSM_MIPI_R63306_PANEL_TMD_MDW30F
	&tmd_mdw30_panel_id,
#endif /* CONFIG_FB_MSM_MIPI_R63306_PANEL_TMD_MDW30F */
	NULL,
};

static struct platform_device semc_fuji_lcd_device = {
	.name = MIPI_DSI_RENESAS_NAME,
	.id = 0,
};

static struct mipi_dsi_platform_data mipi_dsi_pdata = {
	.dsi_power_save   = r63306_lcd_power,
};
#endif /* CONFIG_FB_MSM_MIPI_DSI_RENESAS_R63306 */

/*
 * MIPI_DSI only use 8058_LDO0 which need always on
 * therefore it need to be put at low power mode if
 * it was not used instead of turn it off.
 */
static int mipi_dsi_power(int on)
{
	static int curr_power;
	static struct regulator *ldo0;
	int rc;

	if (curr_power == on)
		return 0;

	if (ldo0 == NULL) {	/* init */
		ldo0 = regulator_get(NULL, "8058_l0");
		if (IS_ERR(ldo0)) {
			pr_err("%s: LDO0 failed\n", __func__);
			rc = PTR_ERR(ldo0);
			return rc;
		}

		rc = regulator_set_voltage(ldo0, 1200000, 1200000);
		if (rc) {
			pr_err("%s: Unable to set voltage level for LDO0\n",
			       __func__);
			goto out_put;
		}

		rc = regulator_enable(ldo0);
		if (rc) {
			pr_err("%s: Unable to enable LDO0\n", __func__);
			goto out_put;
		}

		/* set ldo0 to HPM */
		rc = regulator_set_optimum_mode(ldo0, 100000);
		if (rc < 0) {
			pr_err("%s: Unable to set HPM of LDO0:", __func__);
			goto out_disable;
		} else {
			return 0;
		}
	}

	if (on) {
		/* set ldo0 to HPM */
		rc = regulator_set_optimum_mode(ldo0, 100000);
		if (rc < 0)
			pr_err("%s: Unable to set HPM of LDO0:", __func__);
	} else {
		/* set ldo0 to LPM */
		rc = regulator_set_optimum_mode(ldo0, 1000);
		if (rc < 0)
			pr_err("%s: Unable to set LPM of LDO0:", __func__);
	}

	curr_power = on;

	return 0;
out_disable:
	regulator_disable(ldo0);
out_put:
	regulator_put(ldo0);
	ldo0 = NULL;
	return rc;
}

#ifdef CONFIG_FB_MSM_TVOUT
static struct regulator *reg_8058_l13;

static int atv_dac_power(int on)
{
	int rc = 0;
	#define _GET_REGULATOR(var, name) do {				\
		var = regulator_get(NULL, name);			\
		if (IS_ERR(var)) {					\
			pr_info("'%s' regulator not found, rc=%ld\n",	\
				name, IS_ERR(var));			\
			var = NULL;					\
			return -ENODEV;					\
		}							\
	} while (0)

	if (!reg_8058_l13)
		_GET_REGULATOR(reg_8058_l13, "8058_l13");
	#undef _GET_REGULATOR

	if (on) {
		rc = regulator_set_voltage(reg_8058_l13, 2050000, 2050000);
		if (rc) {
			pr_info("%s: '%s' regulator set voltage failed,"
				"rc=%d\n", __func__, "8058_l13", rc);
			return rc;
		}

		rc = regulator_enable(reg_8058_l13);
		if (rc) {
			pr_err("%s: '%s' regulator enable failed,"
			       "rc=%d\n", __func__, "8058_l13", rc);
			return rc;
		}
	} else {
		rc = regulator_force_disable(reg_8058_l13);
		if (rc)
			pr_warning("%s: '%s' regulator disable failed, rc=%d\n",
				__func__, "8058_l13", rc);
	}
	return rc;

}
#endif

#ifdef CONFIG_FB_MSM_MIPI_DSI
int mdp_core_clk_rate_table[] = {
	85330000,
	128000000,
	160000000,
	200000000,
};
#else
int mdp_core_clk_rate_table[] = {
	59080000,
	128000000,
	128000000,
	200000000,
};
#endif

static struct msm_panel_common_pdata mdp_pdata = {
	.gpio = MDP_VSYNC_GPIO,
	.mdp_core_clk_rate = 59080000,
	.mdp_core_clk_table = mdp_core_clk_rate_table,
	.num_mdp_clk = ARRAY_SIZE(mdp_core_clk_rate_table),
#ifdef CONFIG_MSM_BUS_SCALING
	.mdp_bus_scale_table = &mdp_bus_scale_pdata,
#endif
	.mdp_rev = MDP_REV_41,
#ifdef CONFIG_MSM_MULTIMEDIA_USE_ION
	.mem_hid = ION_CP_WB_HEAP_ID,
#else
	.mem_hid = MEMTYPE_EBI1,
#endif
};

static void __init reserve_mdp_memory(void)
{
	mdp_pdata.ov0_wb_size = MSM_FB_OVERLAY0_WRITEBACK_SIZE;
	mdp_pdata.ov1_wb_size = MSM_FB_OVERLAY1_WRITEBACK_SIZE;
#if defined(CONFIG_ANDROID_PMEM) && !defined(CONFIG_MSM_MULTIMEDIA_USE_ION)
	msm8x60_reserve_table[mdp_pdata.mem_hid].size +=
		mdp_pdata.ov0_wb_size;
	msm8x60_reserve_table[mdp_pdata.mem_hid].size +=
		mdp_pdata.ov1_wb_size;
#endif
}

#ifdef CONFIG_FB_MSM_TVOUT

#ifdef CONFIG_MSM_BUS_SCALING
static struct msm_bus_vectors atv_bus_init_vectors[] = {
	/* For now, 0th array entry is reserved.
	 * Please leave 0 as is and don't use it
	 */
	{
		.src = MSM_BUS_MASTER_MDP_PORT0,
		.dst = MSM_BUS_SLAVE_SMI,
		.ab = 0,
		.ib = 0,
	},
	/* Master and slaves can be from different fabrics */
	{
		.src = MSM_BUS_MASTER_MDP_PORT0,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab = 0,
		.ib = 0,
	},
};
static struct msm_bus_vectors atv_bus_def_vectors[] = {
	/* For now, 0th array entry is reserved.
	 * Please leave 0 as is and don't use it
	 */
	{
		.src = MSM_BUS_MASTER_MDP_PORT0,
		.dst = MSM_BUS_SLAVE_SMI,
		.ab = 236390400,
		.ib = 265939200,
	},
	/* Master and slaves can be from different fabrics */
	{
		.src = MSM_BUS_MASTER_MDP_PORT0,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab = 236390400,
		.ib = 265939200,
	},
};
static struct msm_bus_paths atv_bus_scale_usecases[] = {
	{
		ARRAY_SIZE(atv_bus_init_vectors),
		atv_bus_init_vectors,
	},
	{
		ARRAY_SIZE(atv_bus_def_vectors),
		atv_bus_def_vectors,
	},
};
static struct msm_bus_scale_pdata atv_bus_scale_pdata = {
	atv_bus_scale_usecases,
	ARRAY_SIZE(atv_bus_scale_usecases),
	.name = "atv",
};
#endif

static struct tvenc_platform_data atv_pdata = {
	.poll		 = 0,
	.pm_vid_en	 = atv_dac_power,
#ifdef CONFIG_MSM_BUS_SCALING
	.bus_scale_table = &atv_bus_scale_pdata,
#endif
};
#endif

static void __init msm_fb_add_devices(void)
{
#ifdef CONFIG_FB_MSM_LCDC_DSUB
	mdp_pdata.mdp_core_clk_table = NULL;
	mdp_pdata.num_mdp_clk = 0;
	mdp_pdata.mdp_core_clk_rate = 200000000;
#endif
	msm_fb_register_device("mdp", &mdp_pdata);

	msm_fb_register_device("mipi_dsi", &mipi_dsi_pdata);
#ifdef CONFIG_MSM_BUS_SCALING
	if (hdmi_is_primary)
		msm_fb_register_device("dtv", &dtv_hdmi_prim_pdata);
	else
		msm_fb_register_device("dtv", &dtv_pdata);
#endif
#ifdef CONFIG_FB_MSM_TVOUT
	msm_fb_register_device("tvenc", &atv_pdata);
	msm_fb_register_device("tvout_device", NULL);
#endif
}

static void __init semc_fuji_add_lcd_device(void)
{
	int rc;
	struct lcd_panel_platform_data *pdata;

	pdata = kmalloc(sizeof(struct lcd_panel_platform_data), GFP_KERNEL);

	pdata->default_panels = default_panel_ids_r63306;
	pdata->panels = panel_ids_r63306;
	pdata->lcd_power = lcd_power;
	pdata->lcd_reset = lcd_reset;

	semc_fuji_lcd_device.dev.platform_data = pdata;

	rc = platform_device_register(&semc_fuji_lcd_device);
	if (rc)
		dev_err(&semc_fuji_lcd_device.dev,
			"%s: platform_device_register() failed = %d\n",
			__func__, rc);

}

#ifdef CONFIG_MSM_RPM
static struct msm_rpm_platform_data msm_rpm_data = {
	.reg_base_addrs = {
		[MSM_RPM_PAGE_STATUS] = MSM_RPM_BASE,
		[MSM_RPM_PAGE_CTRL] = MSM_RPM_BASE + 0x400,
		[MSM_RPM_PAGE_REQ] = MSM_RPM_BASE + 0x600,
		[MSM_RPM_PAGE_ACK] = MSM_RPM_BASE + 0xa00,
	},

	.irq_ack = RPM_SCSS_CPU0_GP_HIGH_IRQ,
	.irq_err = RPM_SCSS_CPU0_GP_LOW_IRQ,
	.irq_vmpm = RPM_SCSS_CPU0_GP_MEDIUM_IRQ,
	.msm_apps_ipc_rpm_reg = MSM_GCC_BASE + 0x008,
	.msm_apps_ipc_rpm_val = 4,
};
#endif

void msm_fusion_setup_pinctrl(void)
{
	struct msm_xo_voter *a1;

	if (socinfo_get_platform_subtype() == 0x3) {
		/*
		 * Vote for the A1 clock to be in pin control mode before
		* the external images are loaded.
		*/
		a1 = msm_xo_get(MSM_XO_TCXO_A1, "mdm");
		BUG_ON(!a1);
		msm_xo_mode_vote(a1, MSM_XO_MODE_PIN_CTRL);
	}
}

struct msm_board_data {
	struct msm_gpiomux_configs *gpiomux_cfgs;
};

static struct msm_board_data msm8x60_fuji_board_data __initdata = {
	.gpiomux_cfgs = semc_fuji_gpiomux_cfgs,
};

static void __init msm8x60_init(struct msm_board_data *board_data)
{
	uint32_t soc_platform_version;

	pmic_reset_irq = PM8058_IRQ_BASE + PM8058_RESOUT_IRQ;
	/*
	 * Initialize RPM first as other drivers and devices may need
	 * it for their initialization.
	 */
#ifdef CONFIG_MSM_RPM
	BUG_ON(msm_rpm_init(&msm_rpm_data));
#endif
	BUG_ON(msm_rpmrs_levels_init(msm_rpmrs_levels,
				ARRAY_SIZE(msm_rpmrs_levels)));
	if (msm_xo_init())
		pr_err("Failed to initialize XO votes\n");

	msm8x60_check_2d_hardware();

	/* Change SPM handling of core 1 if PMM 8160 is present. */
	soc_platform_version = socinfo_get_platform_version();
	if (SOCINFO_VERSION_MAJOR(soc_platform_version) == 1 &&
			SOCINFO_VERSION_MINOR(soc_platform_version) >= 2) {
		struct msm_spm_platform_data *spm_data;

		spm_data = &msm_spm_data_v1[1];
		spm_data->reg_init_values[MSM_SPM_REG_SAW_CFG] &= ~0x0F00UL;
		spm_data->reg_init_values[MSM_SPM_REG_SAW_CFG] |= 0x0100UL;

		spm_data = &msm_spm_data[1];
		spm_data->reg_init_values[MSM_SPM_REG_SAW_CFG] &= ~0x0F00UL;
		spm_data->reg_init_values[MSM_SPM_REG_SAW_CFG] |= 0x0100UL;
	}

	/*
	 * Initialize SPM before acpuclock as the latter calls into SPM
	 * driver to set ACPU voltages.
	 */
	if (SOCINFO_VERSION_MAJOR(socinfo_get_version()) != 1)
		msm_spm_init(msm_spm_data, ARRAY_SIZE(msm_spm_data));
	else
		msm_spm_init(msm_spm_data_v1, ARRAY_SIZE(msm_spm_data_v1));

	/*
	 * Disable regulator info printing so that regulator registration
	 * messages do not enter the kmsg log.
	 */
	regulator_suppress_info_printing();

	/* Initialize regulators needed for clock_init. */
	platform_add_devices(early_regulators, ARRAY_SIZE(early_regulators));

	msm_clock_init(&msm8x60_clock_init_data);

	/* Buses need to be initialized before early-device registration
	 * to get the platform data for fabrics.
	 */
	msm8x60_init_buses();
	platform_add_devices(early_devices, ARRAY_SIZE(early_devices));
	/* CPU frequency control is not supported on simulated targets. */
	acpuclk_init(&acpuclk_8x60_soc_data);

	msm8x60_init_tlmm();
	msm8x60_init_gpiomux(board_data->gpiomux_cfgs);
	hw_id_class_init();
	msm8x60_init_uart7dm();
	msm8x60_init_uart12dm();
	msm8x60_init_mmc();
#ifdef CONFIG_SEMC_CHARGER_USB_ARCH
	semc_chg_usb_set_supplicants(semc_chg_usb_supplied_to,
				  ARRAY_SIZE(semc_chg_usb_supplied_to));
#endif
#if defined(CONFIG_PMIC8058_OTHC) || defined(CONFIG_PMIC8058_OTHC_MODULE)
	msm8x60_init_pm8058_othc();
#endif

	pm8058_platform_data.keypad_pdata = &fuji_keypad_data;
#ifdef CONFIG_FUJI_PMIC_KEYPAD
	pm8058_platform_data.keypad_pmic_pdata = &keypad_pmic_platform_data;
#endif
	/* configure pmic leds */
	pm8058_platform_data.leds_pdata = &pm8058_flash_leds_data;
	pm8058_platform_data.vibrator_pdata = &pm8058_vib_pdata;

	msm8x60_pm8901_dVdd_init();
	platform_add_devices(asoc_devices,
				ARRAY_SIZE(asoc_devices));
	platform_add_devices(fuji_devices,
			     ARRAY_SIZE(fuji_devices));

	pm8901_vreg_mpp0_init();

	platform_device_register(&msm8x60_8901_mpp_vreg);

#ifdef CONFIG_USB_EHCI_MSM_72K
	/*
	 * Drive MPP2 pin HIGH for PHY to generate ID interrupts on 8660
	 * fluid
	 */
	msm_add_host(0, &msm_usb_host_pdata);
#endif

#if defined(CONFIG_SPI_QUP) || defined(CONFIG_SPI_QUP_MODULE)
	if (machine_is_msm8x60_fluid())
		platform_device_register(&msm_gsbi10_qup_spi_device);
	else
		platform_device_register(&msm_gsbi1_qup_spi_device);
#endif

#ifdef CONFIG_FB_MSM_MIPI_DSI_RENESAS_R63306
	semc_fuji_add_lcd_device();
#endif

	if (!machine_is_msm8x60_sim())
		msm_fb_add_devices();
	register_i2c_devices();

	msm_pm_set_platform_data(msm_pm_data, ARRAY_SIZE(msm_pm_data));
	msm_pm_set_rpm_wakeup_irq(RPM_SCSS_CPU0_WAKE_UP_IRQ);
	msm_cpuidle_set_states(msm_cstates, ARRAY_SIZE(msm_cstates),
				msm_pm_data);
	BUG_ON(msm_pm_boot_init(&msm_pm_boot_pdata));

	pm8058_gpios_init();
	shared_vreg_init();
	register_reboot_notifier(&vreg_poweroff_notifier);

#ifdef CONFIG_SENSORS_MSM_ADC
	msm_adc_pdata.target_hw = MSM_8x60;
#endif
#ifdef CONFIG_MSM8X60_AUDIO
	msm_snddev_init();
#endif
	msm8x60_multi_sdio_init();
	bt_power_init();
}

static void __init msm8x60_fuji_init(void)
{
	msm8x60_init(&msm8x60_fuji_board_data);
}

static void __init msm8x60_fuji_init_early(void)
{
	msm8x60_allocate_memory_regions();
}

static void __init semc_fuji_fixup(struct machine_desc *desc, struct tag *tags,
				   char **cmdline, struct meminfo *mi)
{
#define MSM_BANK0_BASE                  PHYS_OFFSET
#define MSM_BANK0_SIZE                  0x02C00000

#define MSM_BANK1_BASE                  0x48000000
#define MSM_BANK1_SIZE                  0x18000000

#define MSM_BANK2_BASE                  0x60000000
#define MSM_BANK2_SIZE                  0x1FF00000

	mi->nr_banks = 3;
	mi->bank[0].start = MSM_BANK0_BASE;
	mi->bank[0].size = MSM_BANK0_SIZE;

	mi->bank[1].start = MSM_BANK1_BASE;
	mi->bank[1].size = MSM_BANK1_SIZE;

	mi->bank[2].start = MSM_BANK2_BASE;
	mi->bank[2].size = MSM_BANK2_SIZE;
}

MACHINE_START(SEMC_FUJI, "fuji")
	.fixup = semc_fuji_fixup,
	.map_io = msm8x60_map_io,
	.reserve = msm8x60_reserve,
	.init_irq = msm8x60_init_irq,
	.handle_irq = gic_handle_irq,
	.init_machine = msm8x60_fuji_init,
	.timer = &msm_timer,
	.init_early = msm8x60_fuji_init_early,
MACHINE_END
