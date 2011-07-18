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
#include <linux/i2c.h>
#include <linux/i2c-kona.h>
#include <linux/irq.h>
#include <linux/gpio_keys.h>
#include <linux/input.h>
#include <linux/i2c/tsc2007.h>
#include <linux/i2c/tango_s32.h>
#include <linux/i2c/bcm2850_mic_detect.h>
#include <linux/smb380.h>
#include <linux/akm8975.h>
#include <mach/dma_mmap.h>
#include <mach/sdma.h>

#include <asm/mach/arch.h>
#include <asm/mach-types.h>
#include <asm/gpio.h>
#include <asm/io.h>

#include <mach/hardware.h>
#include <mach/kona.h>
#include <mach/sdio_platform.h>
#include <mach/rdb/brcm_rdb_uartb.h>
#include <mach/rdb/brcm_rdb_chipreg.h>

#include <linux/mfd/bcm590xx/core.h>
#include <linux/mfd/bcm590xx/pmic.h>
#include <linux/mfd/bcm590xx/bcm59055_A0.h>
#include <linux/regulator/max8649.h>
#include <linux/usb/android_composite.h>
#include <linux/kernel_stat.h>
#include <linux/android_pmem.h>

#include <asm/mach/map.h>
#include <linux/broadcom/bcm_fuse_memmap.h>
#include <linux/broadcom/ipcinterface.h>

#include <linux/power_supply.h>

#include "island.h"
#include "common.h"

#include <mach/io_map.h>
#include <mach/aram_layout.h>

#include <linux/vchiq_platform_data_hana.h>
#include <linux/vchiq_platform_data_memdrv_hana.h>

#define KONA_SDIO0_PA   SDIO1_BASE_ADDR
#define KONA_SDIO1_PA   SDIO2_BASE_ADDR
#define KONA_SDIO2_PA   SDIO3_BASE_ADDR
#define SDIO_CORE_REG_SIZE 0x10000

#define BSC_CORE_REG_SIZE      0x100

#define BCM_INT_ID_RESERVED131         (131 + BCM_INT_ID_PPI_MAX)
#define BCM_INT_ID_RESERVED132         (132 + BCM_INT_ID_PPI_MAX)

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29)
#define IS_MULTI_TOUCH  1
#define MAX_NUM_FINGERS 2
#else
#define IS_MULTI_TOUCH  0
#define MAX_NUM_FINGERS 1
#endif

/*
 * GPIO pin for Touch screen pen down interrupt
 */
#define TSC2007_PEN_DOWN_GPIO_PIN   112

#define TANGO_GPIO_IRQ_PIN          112
#define TANGO_GPIO_RESET_PIN        113
#define TANGO_I2C_TS_DRIVER_NUM_BYTES_TO_READ 14

/*
 * Set to 0 for active high (pull-down) mode
 *        1 for active low (pull-up) mode
 */
#define HW_NUM_GPIO_KEYS     6

#define HW_KEYPAD_ACTIVE_MODE     0

/* 32 ~ 64 ms gives appropriate debouncing */
#define HW_KEYPAD_DEBOUNCE_TIME   KEYPAD_DEBOUNCE_64MS

/*
 * by convention, On Island
 *     MAXIM LDO is numbered 0 and 1
 *     BCM59055 LDO is numbered starting from 2
 */
#ifdef CONFIG_REGULATOR_MAX8649
#define MAX8649_LDO_TOTAL	2
#else
#define MAX8649_LDO_TOTAL	0
#endif
#define BCM59055_LDO_OFFSET	MAX8649_LDO_TOTAL

static struct resource board_i2c0_resource[] = {
	[0] =
	{
		.start = BSC1_BASE_ADDR,
		.end = BSC1_BASE_ADDR + BSC_CORE_REG_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] =
	{
		.start = BCM_INT_ID_I2C0,
		.end = BCM_INT_ID_I2C0,
		.flags = IORESOURCE_IRQ,
	 },
};

static struct resource board_i2c1_resource[] = {
	[0] = {
		.start = BSC2_BASE_ADDR,
		.end = BSC2_BASE_ADDR + BSC_CORE_REG_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = BCM_INT_ID_I2C1,
		.end = BCM_INT_ID_I2C1,
		.flags = IORESOURCE_IRQ,
	},
};

static struct resource board_pmu_bsc_resource[] = {
	[0] =	{
		.start = PMU_BSC_BASE_ADDR,
		.end = PMU_BSC_BASE_ADDR + BSC_CORE_REG_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = BCM_INT_ID_PM_I2C,
		.end = BCM_INT_ID_PM_I2C,
		.flags = IORESOURCE_IRQ,
	},
};

#define SSP0_CORE_REG_SIZE 0x1000
static struct resource board_sspi_i2c_resource[] = {
	[0] = {
		.start = SSP0_BASE_ADDR,
		.end = SSP0_BASE_ADDR + SSP0_CORE_REG_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = BCM_INT_ID_SSP0 ,
		.end = BCM_INT_ID_SSP0 ,
		.flags = IORESOURCE_IRQ,
	},
};

static struct bsc_adap_cfg bsc_i2c_cfg[] = {
	[0] = { /* for BSC0 */
		.speed = BSC_BUS_SPEED_50K,
		.bsc_clk = "bsc1_clk",
		.bsc_apb_clk = "bsc1_apb_clk",
	},
	[1] = { /* for BSC1*/
		.speed = BSC_BUS_SPEED_50K,
		.bsc_clk = "bsc2_clk",
		.bsc_apb_clk = "bsc2_apb_clk",
	},
	[2] = { /* for PMU */
		.speed = BSC_BUS_SPEED_50K,
	},
};

static struct platform_device board_i2c_adap_devices[] =
{
	[0] = {	 /* for BSC0 */
		.name = "bsc-i2c",
		.id = 0,
		.resource = board_i2c0_resource,
		.num_resources = ARRAY_SIZE(board_i2c0_resource),
		.dev = {
			.platform_data = &bsc_i2c_cfg[0],
		},
	},
	[1] = {	/* for BSC1 */
		.name = "bsc-i2c",
		.id = 1,
		.resource = board_i2c1_resource,
		.num_resources = ARRAY_SIZE(board_i2c1_resource),
		.dev = {
			.platform_data = &bsc_i2c_cfg[1],
		},
	},
	[2] = {	/* for PMU BSC */
		.name = "bsc-i2c",
		.id = 2,
		.resource = board_pmu_bsc_resource,
		.num_resources = ARRAY_SIZE(board_pmu_bsc_resource),
	},
	[3] = {	/* for SSPI i2c */
		.name = "sspi-i2c",
		.id = 3,
		.resource = board_sspi_i2c_resource,
		.num_resources = ARRAY_SIZE(board_sspi_i2c_resource),
	},
};

static struct gpio_keys_button board_gpio_keys_button[] = {
	{ KEY_HOME, 154, 1, "Home", EV_KEY, 0, 64},
	{ KEY_SEARCH, 157, 1, "Search", EV_KEY, 0, 64},
	{ KEY_MENU, 158, 1, "Menu", EV_KEY, 0, 64},
	{ KEY_BACK, 166, 1, "Back", EV_KEY, 0, 64},
	{ KEY_VOLUMEUP, 159, 1, "Volume up", EV_KEY, 0, 64},
	{ KEY_VOLUMEDOWN, 160, 1, "Volume down", EV_KEY, 0, 64},
};

static struct gpio_keys_platform_data board_gpio_keys = {
	.buttons = board_gpio_keys_button,
	.nbuttons = HW_NUM_GPIO_KEYS,
	.rep = 1
};

static struct platform_device board_gpio_keys_device = {
	.name = "gpio-keys",
	.id = -1,
	.dev = {
		.platform_data = &board_gpio_keys,
	},
};

static struct platform_device islands_leds_device = {
	.name = "islands_ff-led",
	.id = -1,
	.dev = {
		.dma_mask = (u64 *) ~(u32)0,
		.coherent_dma_mask = ~(u32)0,
	},
	.num_resources = 0,
};


/*
 * Touchscreen device
 */
#ifdef CONFIG_TOUCHSCREEN_TSC2007
/*
 * I2C Touchscreen device
 */
static int tsc2007_init_platform_hw(void)
{
	int rc;

	rc = set_irq_type(gpio_to_irq(TSC2007_PEN_DOWN_GPIO_PIN), IRQ_TYPE_EDGE_FALLING);
	if (rc < 0)	{
		printk(KERN_ERR "set_irq_type failed with irq %d\n", BCM_INT_ID_GPIO1);
		return rc;
	}
	rc = gpio_request(TSC2007_PEN_DOWN_GPIO_PIN, "ts_pen_down");
	if (rc < 0) {
		printk(KERN_ERR "unable to request GPIO pin %d\n", TSC2007_PEN_DOWN_GPIO_PIN);
		 return rc;
	}
	gpio_direction_input(TSC2007_PEN_DOWN_GPIO_PIN);
	return 0;
}

static void tsc2007_exit_platform_hw(void)
{
	gpio_free(TSC2007_PEN_DOWN_GPIO_PIN);
}

static void tsc2007_clear_penirq(void)
{
	struct irq_desc *desc = irq_to_desc(gpio_to_irq(TSC2007_PEN_DOWN_GPIO_PIN));
	desc->chip->ack(gpio_to_irq(TSC2007_PEN_DOWN_GPIO_PIN));
}

static struct tsc2007_platform_data tsc_plat_data = {
	.model = 2007,
	.x_plate_ohms		= 510, // For Sharp K3889TP Touch panel device
	.get_pendown_state	= NULL,
	.clear_penirq		= NULL,
	.init_platform_hw	= tsc2007_init_platform_hw,
	.exit_platform_hw	= tsc2007_exit_platform_hw,
	.clear_penirq		= tsc2007_clear_penirq,
};

static struct i2c_board_info __initdata tsc2007_info[] =
{
	{	/* New touch screen i2c slave address. */
		I2C_BOARD_INFO("tsc2007", 0x48),
		.platform_data = &tsc_plat_data,
		.irq = gpio_to_irq(TSC2007_PEN_DOWN_GPIO_PIN),
	},
};

#endif

#ifdef CONFIG_TOUCHSCREEN_TANGO_S32
static struct TANGO_I2C_TS_t tango_plat_data = {
	.i2c_slave_address	= 0,
	.gpio_irq_pin		= TANGO_GPIO_IRQ_PIN,
	.gpio_reset_pin		= TANGO_GPIO_RESET_PIN,
	.x_max_value		= 480,
	.y_max_value		= 800,
	.layout			= X_RIGHT_Y_UP,
	.num_bytes_to_read = TANGO_I2C_TS_DRIVER_NUM_BYTES_TO_READ,
	.is_multi_touch		= IS_MULTI_TOUCH,
	.is_resetable		= 1,
	.num_fingers_idx	= 0,
	.old_touching_idx	= 1,
	.x1_lo_idx		= 2,
	.x1_hi_idx		= 3,
	.y1_lo_idx		= 4,
	.y1_hi_idx		= 5,
	.x2_lo_idx		= 6,
	.x2_hi_idx		= 7,
	.y2_lo_idx		= 8,
	.y2_hi_idx		= 9,
	.x1_width_idx		= 10,	// X1 coordinate touch area of the first finger
	.y1_width_idx		= 11,	// Y1 coordinate touch area of the first finger
	.x2_width_idx		= 12,	// X2 coordinate touch area of the first finger
	.y2_width_idx		= 13,	// Y2 coordinate touch area of the first finger
	.power_mode_idx		= 20,
	.int_mode_idx		= 21,	// INT)mode register
	.int_width_idx		= 22,	// Interrupt pulse width
	.min_finger_val		= 0,
	.max_finger_val		= MAX_NUM_FINGERS,
	.panel_width		 = 56,
};

static struct i2c_board_info __initdata tango_info[] =
{
	{	/* New touch screen i2c slave address. */
		I2C_BOARD_INFO(I2C_TS_DRIVER_NAME, 0x5C),
		.platform_data = &tango_plat_data,
		.irq = gpio_to_irq(TANGO_GPIO_IRQ_PIN),
	},
};

#endif

static struct MIC_DET_t mic_det_plat_data = {
	.comp1_irq = BCM_INT_ID_RESERVED131,
	.comp2_irq = BCM_INT_ID_RESERVED132,
	.comp1_threshold = 0xCB,
	.comp2_threshold = 0xB3,
	.reg_base = KONA_ACI_VA,
};

static struct i2c_board_info __initdata mic_det_info[] =
{
	{	/* The codec's i2c slave address. */
		I2C_BOARD_INFO(MIC_DET_DRIVER_NAME, 0x1A),
		.platform_data = &mic_det_plat_data,
	},
};

static struct resource board_sdio0_resource[] = {
	[0] = {
		.start = KONA_SDIO0_PA,
		.end = KONA_SDIO0_PA + SDIO_CORE_REG_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = BCM_INT_ID_SDIO0,
		.end = BCM_INT_ID_SDIO0,
		.flags = IORESOURCE_IRQ,
	},
};

static struct resource board_sdio1_resource[] = {
	[0] = {
		.start = KONA_SDIO1_PA,
		.end = KONA_SDIO1_PA + SDIO_CORE_REG_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = BCM_INT_ID_SDIO1,
		.end = BCM_INT_ID_SDIO1,
		.flags = IORESOURCE_IRQ,
	},
};

static struct resource board_sdio2_resource[] = {
	[0] = {
		.start = KONA_SDIO2_PA,
		.end = KONA_SDIO2_PA + SDIO_CORE_REG_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = BCM_INT_ID_SDIO_NAND,
		.end = BCM_INT_ID_SDIO_NAND,
		.flags = IORESOURCE_IRQ,
	},
};

static struct sdio_platform_cfg board_sdio_param[] = {
	{ /* SDIO0 */
		.id = 0,
		.data_pullup = 0,
		.devtype = SDIO_DEV_TYPE_WIFI,
		.wifi_gpio = {
			.reset		= 179,
			.reg		= 177,
			.host_wake	= 178,
		},
		.peri_clk_name = "sdio1_clk",
		.ahb_clk_name = "sdio1_ahb_clk",
		.sleep_clk_name = "sdio1_sleep_clk",
		.peri_clk_rate = 20000000,
	},
	{ /* SDIO1 */
		.id = 1,
		.data_pullup = 0,
		.is_8bit = 1,
		.devtype = SDIO_DEV_TYPE_EMMC,
		.peri_clk_name = "sdio2_clk",
		.ahb_clk_name = "sdio2_ahb_clk",
		.sleep_clk_name = "sdio2_sleep_clk",
		.peri_clk_rate = 52000000,
	},
	{ /* SDIO2 */
		.id = 2,
		.data_pullup = 0,
		.cd_gpio = 106,
		.devtype = SDIO_DEV_TYPE_SDMMC,
		.peri_clk_name = "sdio3_clk",
		.ahb_clk_name = "sdio3_ahb_clk",
		.sleep_clk_name = "sdio3_sleep_clk",
		.peri_clk_rate = 48000000,
	},
};

static struct platform_device island_sdio0_device = {
	.name = "sdhci",
	.id = 0,
	.resource = board_sdio0_resource,
	.num_resources = ARRAY_SIZE(board_sdio0_resource),
	.dev = {
		.platform_data = &board_sdio_param[0],
	},
};

static struct platform_device island_sdio1_device = {
	.name = "sdhci",
	.id = 1,
	.resource = board_sdio1_resource,
	.num_resources = ARRAY_SIZE(board_sdio1_resource),
	.dev = {
		.platform_data = &board_sdio_param[1],
	},
};

static struct platform_device island_sdio2_device = {
	.name = "sdhci",
	.id = 2,
	.resource = board_sdio2_resource,
	.num_resources = ARRAY_SIZE(board_sdio2_resource),
	.dev = {
		.platform_data = &board_sdio_param[2],
	},
};

#ifdef CONFIG_MAX8649_SUPPORT_CHANGE_VID_MODE
void island_maxim_platform_hw_init_1(void ) ;
void island_maxim_platform_hw_init_2(void ) ;
#endif

#define BCM590XX_REG_ENABLED		1
#define BCM590XX_REG_DISABLED		0

#define BCM59055_RFLDO_OTP_VAL		BCM590XX_REG_ENABLED
#define BCM59055_CAMLDO_OTP_VAL		BCM590XX_REG_ENABLED
#define BCM59055_HV1LDO_OTP_VAL		BCM590XX_REG_ENABLED
#define BCM59055_HV2LDO_OTP_VAL		BCM590XX_REG_ENABLED
#define BCM59055_HV3LDO_OTP_VAL		BCM590XX_REG_DISABLED
#define BCM59055_HV4LDO_OTP_VAL		BCM590XX_REG_ENABLED
#define BCM59055_HV5LDO_OTP_VAL		BCM590XX_REG_ENABLED
#define BCM59055_HV6LDO_OTP_VAL		BCM590XX_REG_DISABLED
#define BCM59055_HV7LDO_OTP_VAL		BCM590XX_REG_DISABLED
#define BCM59055_SIMLDO_OTP_VAL		BCM590XX_REG_ENABLED
#define BCM59055_CSR_OTP_VAL		BCM590XX_REG_ENABLED
#define BCM59055_IOSR_OTP_VAL		BCM590XX_REG_ENABLED
#define BCM59055_SDSR_OTP_VAL		BCM590XX_REG_ENABLED

/*********** RFLDO ***********/

#ifdef CONFIG_REGULATOR_USERSPACE_CONSUMER
static struct regulator_bulk_data bcm59055_bd_rf = {
	.supply = "rfldo_uc",
};

static struct regulator_userspace_consumer_data bcm59055_uc_data_rf = {
	.name = "rfldo",
	.num_supplies = 1,
	.supplies = &bcm59055_bd_rf,
	.init_on = BCM59055_RFLDO_OTP_VAL
};
static struct platform_device bcm59055_uc_device_rf = {
	.name = "reg-userspace-consumer",
	.id = 0 + BCM59055_LDO_OFFSET,
	.dev = { .platform_data = &bcm59055_uc_data_rf, },
};
#endif
#ifdef CONFIG_REGULATOR_VIRTUAL_CONSUMER
static struct platform_device bcm59055_vc_device_rf = {
	.name = "reg-virt-consumer",
	.id = 0 + BCM59055_LDO_OFFSET,
	.dev = { .platform_data = "rfldo_uc"  },
};
#endif
/*********** CAMLDO **************/

#ifdef CONFIG_REGULATOR_USERSPACE_CONSUMER
static struct regulator_bulk_data bcm59055_bd_cam = {
	.supply = "camldo_uc"
};

static struct regulator_userspace_consumer_data bcm59055_uc_data_cam = {
	.name = "camldo",
	.num_supplies = 1,
	.supplies = &bcm59055_bd_cam,
	.init_on = BCM59055_CAMLDO_OTP_VAL
};

static struct platform_device bcm59055_uc_device_cam = {
	.name = "reg-userspace-consumer",
	.id = 1 + BCM59055_LDO_OFFSET,
	.dev = {
		.platform_data = &bcm59055_uc_data_cam
	}
};
#endif
#ifdef CONFIG_REGULATOR_VIRTUAL_CONSUMER
static struct platform_device bcm59055_vc_device_cam = {
	.name = "reg-virt-consumer",
	.id = 1 + BCM59055_LDO_OFFSET,
	.dev = {
		.platform_data = "camldo_uc"
	}
};
#endif
/*********** HV1LDO ***************/

#ifdef CONFIG_REGULATOR_USERSPACE_CONSUMER
static struct regulator_bulk_data bcm59055_bd_hv1 = {
	.supply = "hv1ldo_uc"
};

static struct regulator_userspace_consumer_data bcm59055_uc_data_hv1 = {
	.name = "hv1ldo",
	.num_supplies = 1,
	.supplies = &bcm59055_bd_hv1,
	.init_on = BCM59055_HV1LDO_OTP_VAL
};

static struct platform_device bcm59055_uc_device_hv1 = {
	.name = "reg-userspace-consumer",
	.id = 2 + BCM59055_LDO_OFFSET,
	.dev = {
		.platform_data = &bcm59055_uc_data_hv1
	},
};
#endif
#ifdef CONFIG_REGULATOR_VIRTUAL_CONSUMER
static struct platform_device bcm59055_vc_device_hv1 = {
	.name = "reg-virt-consumer",
	.id = 2 + BCM59055_LDO_OFFSET,
	.dev = {
		.platform_data = "hv1ldo_uc"
	}
};
#endif
/*********** HV2LDO **************/

#ifdef CONFIG_REGULATOR_USERSPACE_CONSUMER
static struct regulator_bulk_data bcm59055_bd_hv2 = {
	.supply = "hv2ldo_uc"
};

static struct regulator_userspace_consumer_data bcm59055_uc_data_hv2 = {
	.name = "hv2ldo",
	.num_supplies = 1,
	.supplies = &bcm59055_bd_hv2,
	.init_on = BCM59055_HV2LDO_OTP_VAL,
};

static struct platform_device bcm59055_uc_device_hv2 = {
	.name = "reg-userspace-consumer",
	.id = 3 + BCM59055_LDO_OFFSET,
	.dev = {
		.platform_data = &bcm59055_uc_data_hv2
	}
};
#endif
#ifdef CONFIG_REGULATOR_VIRTUAL_CONSUMER
static struct platform_device bcm59055_vc_device_hv2 = {
	.name = "reg-virt-consumer",
	.id = 3 + BCM59055_LDO_OFFSET,
	.dev = {
		.platform_data = "hv2ldo_uc"
	}
};
#endif
/*********** HV3LDO ********************/

#ifdef CONFIG_REGULATOR_USERSPACE_CONSUMER
static struct regulator_bulk_data bcm59055_bd_hv3 = {
	.supply = "hv3ldo_uc"
};

static struct regulator_userspace_consumer_data bcm59055_uc_data_hv3 = {
	.name = "hv3ldo",
	.num_supplies = 1,
	.supplies = &bcm59055_bd_hv3,
	.init_on = BCM59055_HV3LDO_OTP_VAL,
};

static struct platform_device bcm59055_uc_device_hv3 = {
	.name = "reg-userspace-consumer",
	.id = 4 + BCM59055_LDO_OFFSET,
	.dev = {
		.platform_data = &bcm59055_uc_data_hv3
	},
};
#endif
#ifdef CONFIG_REGULATOR_VIRTUAL_CONSUMER
static struct platform_device bcm59055_vc_device_hv3 = {
	.name = "reg-virt-consumer",
	.id = 4 + BCM59055_LDO_OFFSET,
	.dev = {
		.platform_data = "hv3ldo_uc"
	}
};
#endif
/*********** HV4LDO ***************/

#ifdef CONFIG_REGULATOR_USERSPACE_CONSUMER
static struct regulator_bulk_data bcm59055_bd_hv4 = {
	.supply = "hv4ldo_uc"
};

static struct regulator_userspace_consumer_data bcm59055_uc_data_hv4 = {
	.name = "hv4ldo",
	.num_supplies = 1,
	.supplies = &bcm59055_bd_hv4,
	.init_on = BCM59055_HV4LDO_OTP_VAL
};
static struct platform_device bcm59055_uc_device_hv4 = {
	.name = "reg-userspace-consumer",
	.id = 5 + BCM59055_LDO_OFFSET,
	.dev = {
		.platform_data = &bcm59055_uc_data_hv4
	}
};
#endif
#ifdef CONFIG_REGULATOR_VIRTUAL_CONSUMER
static struct platform_device bcm59055_vc_device_hv4 = {
	.name = "reg-virt-consumer",
	.id = 5 + BCM59055_LDO_OFFSET,
	.dev = {
		.platform_data = "hv4ldo_uc"
	}
};
#endif
/*********** HV5LDO ****************/

#ifdef CONFIG_REGULATOR_USERSPACE_CONSUMER
static struct regulator_bulk_data bcm59055_bd_hv5 = {
	.supply = "hv5ldo_uc"
};

static struct regulator_userspace_consumer_data bcm59055_uc_data_hv5 = {
	.name = "hv5ldo",
	.num_supplies = 1,
	.supplies = &bcm59055_bd_hv5,
	.init_on = BCM59055_HV5LDO_OTP_VAL
};

static struct platform_device bcm59055_uc_device_hv5 = {
	.name = "reg-userspace-consumer",
	.id = 6 + BCM59055_LDO_OFFSET,
	.dev = {
		.platform_data = &bcm59055_uc_data_hv5
	}
};
#endif
#ifdef CONFIG_REGULATOR_VIRTUAL_CONSUMER
static struct platform_device bcm59055_vc_device_hv5 = {
	.name = "reg-virt-consumer",
	.id = 6 + BCM59055_LDO_OFFSET,
	.dev = {
		.platform_data = "hv5ldo_uc"
	}
};
#endif
/*********** HV6LDO *******************/

#ifdef CONFIG_REGULATOR_USERSPACE_CONSUMER
static struct regulator_bulk_data bcm59055_bd_hv6 = {
	.supply = "hv6ldo_uc"
};

static struct regulator_userspace_consumer_data bcm59055_uc_data_hv6 = {
	.name = "hv6ldo",
	.num_supplies = 1,
	.supplies = &bcm59055_bd_hv6,
	.init_on = BCM59055_HV6LDO_OTP_VAL,
};

static struct platform_device bcm59055_uc_device_hv6 = {
	.name = "reg-userspace-consumer",
	.id = 7 + BCM59055_LDO_OFFSET,
	.dev = {
		.platform_data = &bcm59055_uc_data_hv6
	},
};
#endif
#ifdef CONFIG_REGULATOR_VIRTUAL_CONSUMER
static struct platform_device bcm59055_vc_device_hv6 = {
	.name = "reg-virt-consumer",
	.id = 7 + BCM59055_LDO_OFFSET,
	.dev = {
		.platform_data = "hv6ldo_uc"
	}
};
#endif
/*********** HV7LDO **************/

#ifdef CONFIG_REGULATOR_USERSPACE_CONSUMER
static struct regulator_bulk_data bcm59055_bd_hv7 = {
	.supply = "hv7ldo_uc"
};

static struct regulator_userspace_consumer_data bcm59055_uc_data_hv7 = {
	.name = "hv7ldo",
	.num_supplies = 1,
	.supplies = &bcm59055_bd_hv7,
	.init_on = BCM59055_HV7LDO_OTP_VAL,
};

static struct platform_device bcm59055_uc_device_hv7 = {
	.name = "reg-userspace-consumer",
	.id = 8 + BCM59055_LDO_OFFSET,
	.dev = {
		.platform_data = &bcm59055_uc_data_hv7
	}
};
#endif
#ifdef CONFIG_REGULATOR_VIRTUAL_CONSUMER
static struct platform_device bcm59055_vc_device_hv7 = {
	.name = "reg-virt-consumer",
	.id = 8 + BCM59055_LDO_OFFSET,
	.dev = {
		.platform_data = "hv7ldo_uc"
	}
};
#endif
/*********** SIMLDO **************/

#ifdef CONFIG_REGULATOR_USERSPACE_CONSUMER
static struct regulator_bulk_data bcm59055_bd_sim = {
	.supply = "simldo_uc"
};

static struct regulator_userspace_consumer_data bcm59055_uc_data_sim = {
	.name = "simldo",
	.num_supplies = 1,
	.supplies = &bcm59055_bd_sim,
	.init_on = BCM59055_SIMLDO_OTP_VAL,
};

static struct platform_device bcm59055_uc_device_sim = {
	.name = "reg-userspace-consumer",
	.id = 9 + BCM59055_LDO_OFFSET,
	.dev = {
		.platform_data = &bcm59055_uc_data_sim
	}
};
#endif
#ifdef CONFIG_REGULATOR_VIRTUAL_CONSUMER
static struct platform_device bcm59055_vc_device_sim = {
	.name = "reg-virt-consumer",
	.id = 9 + BCM59055_LDO_OFFSET,
	.dev = {
		.platform_data = "simldo_uc"
	},
};
#endif
/*********** CSR *************/

#ifdef CONFIG_REGULATOR_USERSPACE_CONSUMER
static struct regulator_bulk_data bcm59055_bd_csr = {
	.supply = "csr_uc"
};

static struct regulator_userspace_consumer_data bcm59055_uc_data_csr = {
	.name = "csr",
	.num_supplies = 1,
	.supplies = &bcm59055_bd_csr,
	.init_on = BCM59055_CSR_OTP_VAL
};

static struct platform_device bcm59055_uc_device_csr = {
	.name = "reg-userspace-consumer",
	.id = 10 + BCM59055_LDO_OFFSET,
	.dev = {
		 .platform_data = &bcm59055_uc_data_csr
	}
};
#endif
#ifdef CONFIG_REGULATOR_VIRTUAL_CONSUMER
static struct platform_device bcm59055_vc_device_csr = {
	.name = "reg-virt-consumer",
	.id = 10 + BCM59055_LDO_OFFSET,
	.dev = {
		.platform_data = "csr_uc"
	},
};
#endif
/*********** IOSR *************/


#ifdef CONFIG_REGULATOR_USERSPACE_CONSUMER
static struct regulator_bulk_data bcm59055_bd_iosr = {
	.supply = "iosr_uc"
};

static struct regulator_userspace_consumer_data bcm59055_uc_data_iosr = {
	.name = "iosr",
	.num_supplies = 1,
	.supplies = &bcm59055_bd_iosr,
	.init_on = BCM59055_IOSR_OTP_VAL
};

static struct platform_device bcm59055_uc_device_iosr = {
	.name = "reg-userspace-consumer",
	.id = 11 + BCM59055_LDO_OFFSET,
	.dev = {
		.platform_data = &bcm59055_uc_data_iosr
	}
};
#endif
#ifdef CONFIG_REGULATOR_VIRTUAL_CONSUMER
static struct platform_device bcm59055_vc_device_iosr = {
	.name = "reg-virt-consumer",
	.id = 11 + BCM59055_LDO_OFFSET,
	.dev = {
		.platform_data = "iosr_uc"
	}
};
#endif
/*********** SDSR ***********/

#ifdef CONFIG_REGULATOR_USERSPACE_CONSUMER
static struct regulator_bulk_data bcm59055_bd_sdsr = {
	.supply = "sdsr_uc"
};

static struct regulator_userspace_consumer_data bcm59055_uc_data_sdsr = {
	.name = "sdsr",
	.num_supplies = 1,
	.supplies = &bcm59055_bd_sdsr,
	.init_on = BCM59055_SDSR_OTP_VAL
};

static struct platform_device bcm59055_uc_device_sdsr = {
	.name = "reg-userspace-consumer",
	.id = 12 + BCM59055_LDO_OFFSET,
	.dev = {
		.platform_data = &bcm59055_uc_data_sdsr
	}
};
#endif
#ifdef CONFIG_REGULATOR_VIRTUAL_CONSUMER
static struct platform_device bcm59055_vc_device_sdsr = {
	.name = "reg-virt-consumer",
	.id = 12 + BCM59055_LDO_OFFSET,
	.dev = {
		.platform_data = "sdsr_uc"
	}
};
#endif


#ifdef CONFIG_REGULATOR_USERSPACE_CONSUMER
struct platform_device *bcm59055_userspace_consumer_devices[] __initdata = {
	&bcm59055_uc_device_rf,
	&bcm59055_uc_device_cam,
	&bcm59055_uc_device_hv1,
	&bcm59055_uc_device_hv2,
	&bcm59055_uc_device_hv3,
	&bcm59055_uc_device_hv4,
	&bcm59055_uc_device_hv5,
	&bcm59055_uc_device_hv6,
	&bcm59055_uc_device_hv7,
	&bcm59055_uc_device_sim,
	&bcm59055_uc_device_csr,
	&bcm59055_uc_device_iosr,
	&bcm59055_uc_device_sdsr
};
#endif
#ifdef CONFIG_REGULATOR_VIRTUAL_CONSUMER
struct platform_device *bcm59055_virtual_consumer_devices[] __initdata = {
	&bcm59055_vc_device_rf,
	&bcm59055_vc_device_cam,
	&bcm59055_vc_device_hv1,
	&bcm59055_vc_device_hv2,
	&bcm59055_vc_device_hv3,
	&bcm59055_vc_device_hv4,
	&bcm59055_vc_device_hv5,
	&bcm59055_vc_device_hv6,
	&bcm59055_vc_device_hv7,
	&bcm59055_vc_device_sim,
	&bcm59055_vc_device_csr,
	&bcm59055_vc_device_iosr,
	&bcm59055_vc_device_sdsr,
};
#endif

struct regulator_consumer_supply max8649_supply1 = {
	.supply = "vc_core"
};

struct regulator_init_data max8649_init_data1 = {
	.constraints	=	{
		.name = "vc_core",
		.min_uV = 750000,
		.max_uV	= 1380000,
		.always_on = 1,
		.boot_on = 1,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE|REGULATOR_CHANGE_MODE ,
		.valid_modes_mask = REGULATOR_MODE_NORMAL|REGULATOR_MODE_FAST ,
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies	= &max8649_supply1,
};

struct max8649_platform_data max8649_info1 = {
	.mode = 2,
	.extclk	= 0,
	.ramp_timing = MAX8649_RAMP_32MV,
	.regulator = &max8649_init_data1 ,
#if defined(CONFIG_MAX8649_SUPPORT_CHANGE_VID_MODE) && defined(CONFIG_REGULATOR_VIRTUAL_CONSUMER)
	.init = island_maxim_platform_hw_init_1,
#endif
};
#ifdef CONFIG_REGULATOR_VIRTUAL_CONSUMER
struct platform_device max8649_vc1 = {
	.name = "reg-virt-consumer",
	.id = 0,
	.dev = {
		.platform_data = "vc_core" ,
	}
};
#endif
struct i2c_board_info max_switch_info_1[] = {
	[0] = {
		.type		= "max8649",
		.addr		= 0x60,
		.platform_data	= &max8649_info1,
	},
};

/***** Second Maxim part init data ( ARM part )*********/
struct regulator_consumer_supply max8649_supply2 = {
	.supply = "arm_core"
};

struct regulator_init_data max8649_init_data2 = {
	.constraints	= {
		.name = "arm_core",
		.min_uV = 750000,
		.max_uV = 1380000,
		.always_on = 1,
		.boot_on = 1,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE|REGULATOR_CHANGE_MODE ,
		.valid_modes_mask = REGULATOR_MODE_NORMAL|REGULATOR_MODE_FAST,
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies	= &max8649_supply2,
};

struct max8649_platform_data max8649_info2 =
{
	.mode = 2,	/* VID1 = 1, VID0 = 0 */
	.extclk		= 0,
	.ramp_timing	= MAX8649_RAMP_32MV,
	.regulator	= &max8649_init_data2 ,
#if defined(CONFIG_MAX8649_SUPPORT_CHANGE_VID_MODE) && defined(CONFIG_REGULATOR_VIRTUAL_CONSUMER)
	.init = island_maxim_platform_hw_init_2,
#endif
} ;
#ifdef CONFIG_REGULATOR_VIRTUAL_CONSUMER
struct platform_device max8649_vc2 = {
	.name = "reg-virt-consumer",
	.id = 1,
	.dev = {
		.platform_data = "arm_core"
	}
};
#endif

struct i2c_board_info max_switch_info_2[] = {
	[0] = {
		.type		= "max8649",
		.addr		= 0x62,
		.platform_data	= &max8649_info2
	}
};
#ifdef CONFIG_REGULATOR_VIRTUAL_CONSUMER
struct platform_device *maxim_devices_1[] __initdata = { &max8649_vc1 } ;
struct platform_device *maxim_devices_2[] __initdata = { &max8649_vc2 };
void island_maxim_platform_hw_init_1(void )
{
	printk(KERN_INFO "REG: island_maxim_platform_hw_init for VC called\n") ;
	platform_add_devices(maxim_devices_1, ARRAY_SIZE(maxim_devices_1));
}

void island_maxim_platform_hw_init_2(void )
{
	printk(KERN_INFO "REG: island_maxim_platform_hw_init for ARM called \n") ;
	platform_add_devices(maxim_devices_2, ARRAY_SIZE(maxim_devices_2));
}
#endif


#ifdef CONFIG_BATTERY_BCM59055
static struct mv_percent mv_percent_table[] =
{
    { 3800 , 5 },
    { 3850 , 25 },
    { 3900 , 50 },
    { 3950 , 70 },
    { 4000 , 90 },
    { 4100 , 100 },
} ;

static struct bcm590xx_battery_pdata bcm590xx_battery_plat_data = {
	.can_start_charging = can_start_charging,
	.vp_table = mv_percent_table ,
	.vp_table_cnt = ARRAY_SIZE(mv_percent_table) ,
        .batt_min_volt = 3200 ,
        .batt_max_volt = 4200 ,
        .batt_technology = POWER_SUPPLY_TECHNOLOGY_LION ,
};
#endif



#define IPC_SHARED_CHANNEL_VIRT     ( KONA_INT_SRAM_BASE + BCMHANA_ARAM_VC_OFFSET )
#define IPC_SHARED_CHANNEL_PHYS     ( INT_SRAM_BASE + BCMHANA_ARAM_VC_OFFSET )

static VCHIQ_PLATFORM_DATA_MEMDRV_HANA_T vchiq_display_data_memdrv_hana = {
    .memdrv = {
        .common = {
            .instance_name = "display",
            .dev_type      = VCHIQ_DEVICE_TYPE_SHARED_MEM,
        },
        .sharedMemVirt  = (void *)(IPC_SHARED_CHANNEL_VIRT),
        .sharedMemPhys  = IPC_SHARED_CHANNEL_PHYS,
    },
    .ipcIrq                =  BCM_INT_ID_IPC_OPEN,
};

static struct platform_device vchiq_display_device = {
    .name = "vchiq_memdrv_hana",
    .id = 0,
    .dev = {
        .platform_data = &vchiq_display_data_memdrv_hana,
    },
};

struct platform_device * vchiq_devices[] __initdata = {&vchiq_display_device};

#define BMA150_IRQ_PIN 120

static struct smb380_platform_data bma150_plat_data = {
	.range = RANGE_2G,
	.bandwidth = BW_375HZ,
	.enable_adv_int = 1,
	.new_data_int = 0 ,
	.hg_int = 0 ,
	.lg_int = 0 ,
	.lg_dur = 150 ,
	.lg_thres = 20 ,
	.lg_hyst = 0 ,
	.hg_dur = 60 ,
	.hg_thres = 160 ,
	.hg_hyst = 0 ,
	.any_motion_dur = 1 ,
	.any_motion_thres = 20 ,
	.any_motion_int = 1 ,
};

static struct i2c_board_info __initdata bma150_info[] =
{
	[0] = {
		I2C_BOARD_INFO("smb380", 0x38 ),
		.platform_data = &bma150_plat_data,
		.irq = gpio_to_irq(BMA150_IRQ_PIN)
	}
};

#define AKM8975_IRQ_PIN 155

static struct akm8975_platform_data akm8975_plat_data = {
	.init = NULL, 
	.exit = NULL, 
	.power_on = NULL, 
	.power_off = NULL,
}; 

static struct i2c_board_info __initdata akm8975_info[] =
{
	[0] = {
		I2C_BOARD_INFO("akm8975", 0x0C ),
		.platform_data = &akm8975_plat_data,
		.irq = gpio_to_irq(AKM8975_IRQ_PIN),
	},
};


static char *android_function_rndis[] = {
#ifdef CONFIG_USB_ANDROID_RNDIS
	"rndis"
#endif
};

static char *android_function_acm[] = {
#ifdef CONFIG_USB_ANDROID_ACM
	"acm"
#endif
};

static char *android_function_adb_msc[] = {
#ifdef CONFIG_USB_ANDROID_MASS_STORAGE
	"usb_mass_storage",
#endif
#ifdef CONFIG_USB_ANDROID_ADB
	"adb",
#endif
};

static char *android_functions_all[] = {
#ifdef CONFIG_USB_ANDROID_MASS_STORAGE
	"usb_mass_storage",
#endif
#ifdef CONFIG_USB_ANDROID_ADB
	"adb",
#endif
#ifdef CONFIG_USB_ANDROID_RNDIS
	"rndis",
#endif
#ifdef CONFIG_USB_ANDROID_ACM
	"acm",
#endif
};

#define	BRCM_VENDOR_ID		0x0a5c
#define	BIG_ISLAND_PRODUCT_ID	0x2816

/* FIXME borrow Google Nexus One ID to use windows driver */
#define	GOOGLE_VENDOR_ID	0x18d1
#define	NEXUS_ONE_PROD_ID	0x0d02

#define	VENDOR_ID		GOOGLE_VENDOR_ID
#define	PRODUCT_ID		NEXUS_ONE_PROD_ID

/* use a seprate PID for RNDIS */
#define RNDIS_PRODUCT_ID	0x4e13
#define ACM_PRODUCT_ID		0x8888


static struct usb_mass_storage_platform_data android_mass_storage_pdata = {
	.nluns		=	1,
	.vendor		=	"Broadcom",
	.product	=	"Big Island",
	.release	=	0x0100
};

static struct platform_device android_mass_storage_device = {
	.name	=	"usb_mass_storage",
	.id	=	-1,
	.dev	=	{
		.platform_data	=	&android_mass_storage_pdata,
	}
};

static struct usb_ether_platform_data android_rndis_pdata = {
	/* ethaddr FIXME */
	.vendorID = __constant_cpu_to_le16(VENDOR_ID),
	.vendorDescr = "Broadcom RNDIS",
};

static struct platform_device android_rndis_device = {
	.name	= "rndis",
	.id	= -1,
	.dev	= {
		.platform_data = &android_rndis_pdata,
	},
};

static struct android_usb_product android_products[] = {
	{
		.product_id	= 	__constant_cpu_to_le16(PRODUCT_ID),
		.num_functions	=	ARRAY_SIZE(android_function_adb_msc),
		.functions	=	android_function_adb_msc,
	},
	{
		.product_id	= 	__constant_cpu_to_le16(RNDIS_PRODUCT_ID),
		.num_functions	=	ARRAY_SIZE(android_function_rndis),
		.functions	=	android_function_rndis,
	},
	{
		.product_id	= 	__constant_cpu_to_le16(ACM_PRODUCT_ID),
		.num_functions	=	ARRAY_SIZE(android_function_acm),
		.functions	=	android_function_acm,
	},
};

static struct android_usb_platform_data android_usb_data = {
	.vendor_id		= 	__constant_cpu_to_le16(VENDOR_ID),
	.product_id		=	__constant_cpu_to_le16(PRODUCT_ID),
	.version		=	0,
	.product_name		=	"Big Island",
	.manufacturer_name	= 	"Broadcom",
	.serial_number		=	"0123456789ABCDEF",

	.num_products		=	ARRAY_SIZE(android_products),
	.products		=	android_products,

	.num_functions		=	ARRAY_SIZE(android_functions_all),
	.functions		=	android_functions_all,
};

static struct platform_device android_usb = {
	.name 	= "android_usb",
	.id	= 1,
	.dev	= {
		.platform_data = &android_usb_data,
	},
};

static struct android_pmem_platform_data android_pmem_data = {
	.name = "pmem",
	.start = 0x9C000000,
	.size = SZ_64M,
	.no_allocator = 0,
	.cached = 0,
	.buffered = 0,
};

static struct platform_device android_pmem = {
	.name 	= "android_pmem",
	.id	= 0,
	.dev	= {
		.platform_data = &android_pmem_data,
	},
};

static struct gpio_led island_gpio_leds[] = {
	{
		.name			= "green-led",
		.gpio			= 105,
	},
	{
		.name			= "red-led",
		.gpio			= 114,
	},
};
static struct gpio_led_platform_data island_gpio_led_data = {
	.leds		= island_gpio_leds,
	.num_leds	= ARRAY_SIZE(island_gpio_leds),
};
static struct platform_device island_leds_gpio_device = {
	.name	= "leds-gpio",
	.id	= -1,
	.dev	= {
		.platform_data	= &island_gpio_led_data,
	},
};

void __init board_map_io(void)
{
	/* Map machine specific iodesc here */

	island_map_io();
}

static struct platform_device *board_devices[] __initdata = {
	&board_i2c_adap_devices[0],
	&board_i2c_adap_devices[1],
	&board_i2c_adap_devices[2],
	&board_i2c_adap_devices[3],
	&island_sdio1_device,
	&island_sdio2_device,
//	&island_ipc_device,
	&board_gpio_keys_device,
	&islands_leds_device,
	&android_rndis_device,
	&android_mass_storage_device,
	&android_usb,
	&android_pmem,
	&island_leds_gpio_device,
	&island_sdio0_device,
};

static void __init board_add_devices(void)
{
	platform_add_devices(board_devices, ARRAY_SIZE(board_devices));

#ifdef CONFIG_TOUCHSCREEN_TSC2007
	i2c_register_board_info(1,
		tsc2007_info,
		ARRAY_SIZE(tsc2007_info));
#endif

#ifdef CONFIG_TOUCHSCREEN_TANGO_S32
	i2c_register_board_info(1,
		tango_info,
		ARRAY_SIZE(tango_info));
#endif

	i2c_register_board_info(1,
		mic_det_info,
		ARRAY_SIZE(mic_det_info));

	i2c_register_board_info(2,
		max_switch_info_1,
		ARRAY_SIZE(max_switch_info_1));

	i2c_register_board_info(2,
		max_switch_info_2,
		ARRAY_SIZE(max_switch_info_2));

	/*i2c_register_board_info(2,
		pmu_info,
		ARRAY_SIZE(pmu_info));*/
    board_pmu_init();

	i2c_register_board_info(3,
		bma150_info,
		ARRAY_SIZE(bma150_info));

	i2c_register_board_info(3,
		akm8975_info,
		ARRAY_SIZE(akm8975_info));
	
#ifdef CONFIG_REGULATOR_USERSPACE_CONSUMER
	platform_add_devices(bcm59055_userspace_consumer_devices, ARRAY_SIZE(bcm59055_userspace_consumer_devices));
#endif
#ifdef CONFIG_REGULATOR_VIRTUAL_CONSUMER
	platform_add_devices(bcm59055_virtual_consumer_devices, ARRAY_SIZE(bcm59055_virtual_consumer_devices));
#endif

   platform_add_devices( vchiq_devices, ARRAY_SIZE( vchiq_devices ) );

}

void __init pinmux_setup(void)
{
	void __iomem *chipRegBase = IOMEM(KONA_CHIPREG_VA);
	uint32_t val;

	/* Setup pin muxing for PMU interrupt pin.
	*/
	val = ( 3 << CHIPREG_PMU_INT_PINSEL_2_0_SHIFT ) |
		( 1 << CHIPREG_PMU_INT_PUP_PMU_INT_SHIFT ) |
		( 3 << CHIPREG_PMU_INT_SEL_2_0_SHIFT ) ;
	writel( val, chipRegBase + CHIPREG_PMU_INT_OFFSET );

	/* Setup pin muxing for sensor interrupt pin.
	*/
	val = ( 3 << CHIPREG_SIM2_DET_PINSEL_2_0_SHIFT ) |
	( 3 << CHIPREG_SIM2_DET_SEL_2_0_SHIFT );
	writel( val, chipRegBase + CHIPREG_SIM2_DET_OFFSET ) ;

	/* Setup pin muxing for compass data ready pin.
	*/
	val = ( 3 << CHIPREG_NORFLSH_AD_05_PINSEL_2_0_SHIFT ) |
	( 3 << CHIPREG_NORFLSH_AD_05_SEL_2_0_SHIFT )	 ;
	writel( val, chipRegBase + CHIPREG_NORFLSH_AD_05_OFFSET ) ;
}

void __init board_init(void)
{
	pinmux_setup();
	dma_mmap_init();
	sdma_init();	
	/*
	 * Add common platform devices that do not have board dependent HW
	 * configurations
	 */
	board_add_common_devices();
	board_add_devices();
}

MACHINE_START(ISLAND, "Island DK/FF")
	.phys_io = IO_START,
	.io_pg_offst = (IO_BASE >> 18) & 0xFFFC,
	.map_io = board_map_io,
	.init_irq = kona_init_irq,
	.timer = &kona_timer,
	.init_machine = board_init,
MACHINE_END
