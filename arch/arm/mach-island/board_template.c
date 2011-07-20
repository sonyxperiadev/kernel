/*****************************************************************************
* Copyright 2003 - 2011 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/

/*
 * A board template for adding devices and pass their associated board
 * dependent confgiurations as platform_data into the drivers
 *
 * This file needs to be included by the board specific source code
 */

#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/serial_8250.h>
#include <linux/i2c.h>
#include <linux/i2c-kona.h>

#include <asm/memory.h>
#include <asm/sizes.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/hardware/gic.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>

#include <mach/hardware.h>
#include <mach/irqs.h>
#include <mach/kona.h>
#include <mach/dma_mmap.h>
#include <mach/sdma.h>
#include <mach/sdio_platform.h>
#include <mach/usbh_cfg.h>
#include <mach/halaudio_audioh_platform.h>
#include <mach/halaudio_pcm_platform.h>

#include <sdio_settings.h>

#include <i2c_settings.h>
#include <usbh_settings.h>

#include <halaudio_settings.h>
#include <halaudio_audioh_settings.h>
#include <halaudio_pcm_settings.h>

#if defined(CONFIG_BCMBLT_RFKILL) || defined(CONFIG_BCMBLT_RFKILL_MODULE)
#include <linux/broadcom/bcmblt-rfkill.h>
#include <bcmblt_rfkill_settings.h>
#endif

#if defined(CONFIG_TOUCHSCREEN_EGALAX_I2C) || defined(CONFIG_TOUCHSCREEN_EGALAX_I2C_MODULE)
#include <linux/i2c/egalax_i2c_ts.h>
#include <egalax_i2c_ts_settings.h>
#endif

#if defined(CONFIG_SENSORS_BMA150) || defined(CONFIG_SENSORS_BMA150_MODULE)
#include <linux/bma150.h>
#include <sensors_bma150_i2c_settings.h>
#endif

#if defined(CONFIG_SENSORS_BH1715) || defined(CONFIG_SENSORS_BH1715_MODULE)
#include <linux/bh1715.h>
#include <bh1715_i2c_settings.h>
#endif

#if defined(CONFIG_SENSORS_MPU3050) || defined(CONFIG_SENSORS_MPU3050_MODULE)
#include <linux/mpu3050.h>
#include <mpu3050_i2c_settings.h>
#endif

#if defined(CONFIG_BMP18X_I2C) || defined(CONFIG_BMP18X_I2C_MODULE)
#include <linux/bmp18x.h>
#include <bmp18x_i2c_settings.h>
#endif

#if defined(CONFIG_NET_ISLAND)
#include <mach/net_platform.h>
#include <net_settings.h>
#endif

#if defined(CONFIG_MAX3353) || defined(CONFIG_MAX3353_MODULE)
#include <otg_settings.h>
#include <linux/i2c/max3353.h>
#endif

#if defined(CONFIG_LEDS_GPIO) || defined(CONFIG_LEDS_GPIO_MODULE)
#include <leds_gpio_settings.h>
#include <linux/leds.h>
#endif

#if defined(CONFIG_KEYBOARD_GPIO) || defined(CONFIG_KEYBOARD_GPIO_MODULE)
#include <linux/input.h>
#include <linux/gpio_keys.h>
#include <gpio_keys_settings.h>
#endif

#if defined(CONFIG_VC_VCHIQ) || defined(CONFIG_VC_VCHIQ_MODULE)
#include <mach/io_map.h>
#include <mach/aram_layout.h>

#include <linux/vceb_platform_data_hana.h>
#include <linux/vchiq_platform_data_hana.h>
#include <linux/vchiq_platform_data_memdrv_hana.h>
#endif

#if defined(CONFIG_KEYBOARD_KONA) || defined(CONFIG_KEYBOARD_KONA_MODULE)
#include <linux/kona_keypad.h>
#include <keymap_settings.h>
#endif

#if defined(CONFIG_MONITOR_ADC121C021_I2C)  || defined(CONFIG_MONITOR_ADC121C021_I2C_MODULE)
#include <linux/broadcom/adc121c021_driver.h>
#include <adc121c021_settings.h>
#include <battery_settings.h>
#endif

#if defined(CONFIG_MONITOR_BQ27541_I2C) || defined(CONFIG_MONITOR_BQ27541_I2C_MODULE)
#include <linux/broadcom/bq27541.h>
#include <bq27541_i2c_settings.h>
#endif

#if defined(CONFIG_BATTERY_MAX17040) || defined(CONFIG_BATTERY_MAX17040_MODULE)
#include <linux/max17040_battery.h>
#include <max17040_settings.h>
#endif

#if defined(CONFIG_BCM_CMP_BATTERY_MULTI) || defined(CONFIG_BCM_CMP_BATTERY_MULTI_MODULE)
#include <linux/broadcom/cmp_battery_multi.h>
#include <battery_settings.h>
#endif
 
#if defined(CONFIG_BCM_CMP_BATTERY_BQ24616) || defined(CONFIG_BCM_CMP_BATTERY_BQ24616_MODULE)
#include <linux/broadcom/cmp_battery_bq24616.h>
#include <battery_settings.h>
/* Until proper solution for docking station is found, compal docking station is
   handled in bq24616 battery driver */
#include <dock_settings.h>
#endif

#if defined(CONFIG_BCM_GPS) || defined(CONFIG_BCM_GPS_MODULE)
#include <gps_settings.h>
#include <linux/broadcom/gps.h>
#endif

#if defined(CONFIG_BCM_HAPTICS) || defined(CONFIG_BCM_HAPTICS_MODULE)
#include <linux/broadcom/bcm_haptics.h>
#include <bcm_haptics_settings.h>
#endif

#include "island.h"
#include "common.h"

#ifndef ISLAND_BOARD_ID
#error ISLAND_BOARD_ID needs to be defined in board_xxx.c
#endif

/*
 * Since this board template is included by each board_xxx.c. We concatenate
 * ISLAND_BOARD_ID to help debugging when multiple boards are compiled into
 * a single image
 */
#define concatenate_again(a, b) a ## b
#define concatenate(a, b) concatenate_again(a, b)

/* number of SDIO devices */
#define MAX_SDIO_DEVICES      3

/*
 * The SDIO index starts from 1 in CHAL, which is really not by convention
 * Re-define them here to avoid confusions
 */
#define PHYS_ADDR_SDIO0        SDIO1_BASE_ADDR
#define PHYS_ADDR_SDIO1        SDIO2_BASE_ADDR
#define PHYS_ADDR_SDIO2        SDIO3_BASE_ADDR
#define SDIO_CORE_REG_SIZE     0x10000


/* number of I2C adapters (hosts/masters) */
#define MAX_I2C_ADAPS    3

/*
 * The BSC (I2C) index starts from 1 in CHAL, which is really not by
 * convention. Re-define them here to avoid confusions
 */
#define PHYS_ADDR_BSC0         BSC1_BASE_ADDR
#define PHYS_ADDR_BSC1         BSC2_BASE_ADDR
#define PHYS_ADDR_BSC2         PMU_BSC_BASE_ADDR
#define BSC_CORE_REG_SIZE      0x100

#define USBH_EHCI_CORE_REG_SIZE    0x90
#define USBH_OHCI_CORE_REG_SIZE    0x1000
#define USBH_DWC_REG_OFFSET        USBH_EHCI_CORE_REG_SIZE
#define USBH_DWC_BASE_ADDR         (EHCI_BASE_ADDR + USBH_DWC_REG_OFFSET)
#define USBH_DWC_CORE_REG_SIZE     0x20
#define USBH_CTRL_REG_OFFSET       0x8000
#define USBH_CTRL_BASE_ADDR        (EHCI_BASE_ADDR + USBH_CTRL_REG_OFFSET)
#define USBH_CTRL_CORE_REG_SIZE    0x20

#define OTG_CTRL_CORE_REG_SIZE     0x100

static struct resource sdio0_resource[] = {
	[0] = {
		.start = PHYS_ADDR_SDIO0,
		.end = PHYS_ADDR_SDIO0 + SDIO_CORE_REG_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = BCM_INT_ID_SDIO0,
		.end = BCM_INT_ID_SDIO0,
		.flags = IORESOURCE_IRQ,
	},
};

static struct resource sdio1_resource[] = {
	[0] = {
		.start = PHYS_ADDR_SDIO1,
		.end = PHYS_ADDR_SDIO1 + SDIO_CORE_REG_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = BCM_INT_ID_SDIO1,
		.end = BCM_INT_ID_SDIO1,
		.flags = IORESOURCE_IRQ,
	},
};

static struct resource sdio2_resource[] = {
        [0] = {
                .start = PHYS_ADDR_SDIO2,
                .end = PHYS_ADDR_SDIO2 + SDIO_CORE_REG_SIZE - 1,
                .flags = IORESOURCE_MEM,
        },
        [1] = {
                .start = BCM_INT_ID_SDIO_NAND,
                .end = BCM_INT_ID_SDIO_NAND,
                .flags = IORESOURCE_IRQ,
        },
};

static struct sdio_platform_cfg sdio_param[] =
#ifdef HW_SDIO_PARAM
	HW_SDIO_PARAM;
#else
	{};
#endif

static struct platform_device sdio_devices[MAX_SDIO_DEVICES] =
{
   { /* SDIO0 */
      .name = "sdhci",
      .id = 0,
      .resource = sdio0_resource,
      .num_resources	= ARRAY_SIZE(sdio0_resource),
   },
   { /* SDIO1 */
      .name = "sdhci",
      .id = 1,
      .resource = sdio1_resource,
      .num_resources	= ARRAY_SIZE(sdio1_resource),
   },
   { /* SDIO2 */
      .name = "sdhci",
      .id = 2,
      .resource = sdio2_resource,
      .num_resources    = ARRAY_SIZE(sdio1_resource),
   },
};

#if defined(CONFIG_NET_ISLAND)
static struct island_net_hw_cfg island_net_data =
#ifdef HW_CFG_ISLAND_NET
   HW_CFG_ISLAND_NET;
#else
{
   .addrPhy0 = 0,
   .addrPhy1 = 1,
   .gpioPhy0 = -1,
   .gpioPhy1 = -1,
};
#endif

static struct platform_device net_device =
{
   .name = "island-net",
   .id = -1,
   .dev =
   {
      .platform_data = &island_net_data,
   },
};
#endif /* CONFIG_NET_ISLAND */

static struct bsc_adap_cfg i2c_adap_param[] =
#ifdef HW_I2C_ADAP_PARAM
	HW_I2C_ADAP_PARAM;
#else
	{};
#endif

static struct resource i2c0_resource[] = {
   [0] =
   {
      .start = PHYS_ADDR_BSC0,
      .end = PHYS_ADDR_BSC0 + BSC_CORE_REG_SIZE - 1,
      .flags = IORESOURCE_MEM,
   },
   [1] = 
   {
      .start = BCM_INT_ID_I2C0,
      .end = BCM_INT_ID_I2C0,
      .flags = IORESOURCE_IRQ,
   },
};

static struct resource i2c1_resource[] = {
   [0] =
   {
      .start = PHYS_ADDR_BSC1,
      .end = PHYS_ADDR_BSC1 + BSC_CORE_REG_SIZE - 1,
      .flags = IORESOURCE_MEM,
   },
   [1] = 
   {
      .start = BCM_INT_ID_I2C1,
      .end = BCM_INT_ID_I2C1,
      .flags = IORESOURCE_IRQ,
   },
};

static struct resource i2c2_resource[] = {
   [0] =
   {
      .start = PHYS_ADDR_BSC2,
      .end = PHYS_ADDR_BSC2 + BSC_CORE_REG_SIZE - 1,
      .flags = IORESOURCE_MEM,
   },
   [1] =
   {
      .start = BCM_INT_ID_PM_I2C,
      .end = BCM_INT_ID_PM_I2C,
      .flags = IORESOURCE_IRQ,
   },
};

static struct platform_device i2c_adap_devices[MAX_I2C_ADAPS] =
{
   {  /* for BSC0 */
      .name = "bsc-i2c",
      .id = 0,
      .resource = i2c0_resource,
      .num_resources	= ARRAY_SIZE(i2c0_resource),
   },
   {  /* for BSC1 */
      .name = "bsc-i2c",
      .id = 1,
      .resource = i2c1_resource,
      .num_resources	= ARRAY_SIZE(i2c1_resource),
   },
   {  /* for PMBSC */
      .name = "bsc-i2c",
      .id = 2,
      .resource = i2c2_resource,
      .num_resources	= ARRAY_SIZE(i2c2_resource),
   },
};

static struct usbh_cfg usbh_param =
#ifdef HW_USBH_PARAM
	HW_USBH_PARAM;
#else
	{};
#endif

static struct resource usbh_resource[] = {
	[0] = {
		.start = USBH_CTRL_BASE_ADDR,
		.end = USBH_CTRL_BASE_ADDR + USBH_CTRL_CORE_REG_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
};

static struct platform_device usbh_device =
{
	.name = "usbh",
	.id = -1,
	.resource = usbh_resource,
	.num_resources = ARRAY_SIZE(usbh_resource),
	.dev = {
		.platform_data = &usbh_param,
	},
};

static u64 ehci_dmamask = DMA_BIT_MASK(32);

static struct resource usbh_ehci_resource[] = {
	[0] = {
		.start = EHCI_BASE_ADDR,
		.end = EHCI_BASE_ADDR + USBH_EHCI_CORE_REG_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = BCM_INT_ID_ULPI_EHCI,
		.end = BCM_INT_ID_ULPI_EHCI,
		.flags = IORESOURCE_IRQ,
	},
};

static struct platform_device usbh_ehci_device =
{
	.name = "bcm-ehci",
	.id = 0,
	.resource = usbh_ehci_resource,
	.num_resources = ARRAY_SIZE(usbh_ehci_resource),
	.dev = {
		.dma_mask = &ehci_dmamask,
		.coherent_dma_mask = DMA_BIT_MASK(32),
	},
};

static u64 ohci_dmamask = DMA_BIT_MASK(32);

static struct resource usbh_ohci_resource[] = {
	[0] = {
		.start = OHCI_BASE_ADDR,
		.end = OHCI_BASE_ADDR + USBH_OHCI_CORE_REG_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = BCM_INT_ID_ULPI_OHCI,
		.end = BCM_INT_ID_ULPI_OHCI,
		.flags = IORESOURCE_IRQ,
	},
};

static struct platform_device usbh_ohci_device =
{
	.name = "bcm-ohci",
	.id = 0,
	.resource = usbh_ohci_resource,
	.num_resources = ARRAY_SIZE(usbh_ohci_resource),
	.dev = {
		.dma_mask = &ohci_dmamask,
		.coherent_dma_mask = DMA_BIT_MASK(32),
	},
};

#if defined(CONFIG_TOUCHSCREEN_EGALAX_I2C) || defined(CONFIG_TOUCHSCREEN_EGALAX_I2C_MODULE)
static struct egalax_i2c_ts_cfg egalax_i2c_param =
{
	.id = -1,
	.gpio = {
		.reset = -1,
		.event = -1,
	},
};

static struct i2c_board_info egalax_i2c_boardinfo[] =
{
	{
		.type = "egalax_i2c",
		.addr = 0x04,
		.platform_data = &egalax_i2c_param,
	},
};
#endif

#if defined(CONFIG_MAX3353) || defined(CONFIG_MAX3353_MODULE)
static struct max3353_platform_data max3353_info = {
	.mode = HW_OTG_MAX3353_MODE,
};

static struct i2c_board_info max3353_i2c_boardinfo[] = {
	{
		I2C_BOARD_INFO(MAX3353_DRIVER_NAME, MAX3353_I2C_ADDR_BASE),
		.platform_data  = &max3353_info,
	},
};
#endif

#if defined(CONFIG_LEDS_GPIO) || defined(CONFIG_LEDS_GPIO_MODULE)
#define board_gpio_leds concatenate(ISLAND_BOARD_ID, _board_gpio_leds)
static struct gpio_led board_gpio_leds[] = GPIO_LEDS_SETTINGS;

#define leds_gpio_data concatenate(ISLAND_BOARD_ID, _leds_gpio_data)
static struct gpio_led_platform_data leds_gpio_data =
{
    .num_leds = ARRAY_SIZE(board_gpio_leds),
    .leds = board_gpio_leds,
};

#define board_leds_gpio_device concatenate(ISLAND_BOARD_ID, _leds_gpio_device)
static struct platform_device board_leds_gpio_device = {
   .name = "leds-gpio",
   .id = -1,
   .dev = {
      .platform_data = &leds_gpio_data,
   },
};
#endif

#if defined(CONFIG_KEYBOARD_GPIO) || defined(CONFIG_KEYBOARD_GPIO_MODULE)
#define board_gpio_keys concatenate(ISLAND_BOARD_ID, _board_gpio_keys)
static struct gpio_keys_button board_gpio_keys[] = GPIO_KEYS_SETTINGS;

#define gpio_keys_data concatenate(ISLAND_BOARD_ID, _gpio_keys_data)
static struct gpio_keys_platform_data gpio_keys_data =
{
    .nbuttons = ARRAY_SIZE(board_gpio_keys),
    .buttons = board_gpio_keys,
};

#define board_gpio_keys_device concatenate(ISLAND_BOARD_ID, _gpio_keys_device)
static struct platform_device board_gpio_keys_device = {
   .name = "gpio-keys",
   .id = -1,
   .dev = {
      .platform_data = &gpio_keys_data,
   },
};
#endif

#if defined(CONFIG_KEYBOARD_KONA) || defined(CONFIG_KEYBOARD_KONA_MODULE)

#define board_keypad_keymap concatenate(ISLAND_BOARD_ID, _keypad_keymap)
static struct KEYMAP board_keypad_keymap[] = HW_DEFAULT_KEYMAP;

#define board_keypad_pwroff concatenate(ISLAND_BOARD_ID, _keypad_pwroff)
static unsigned int board_keypad_pwroff[] = HW_DEFAULT_POWEROFF;

#define board_keypad_param concatenate(ISLAND_BOARD_ID, _keypad_param)
static struct KEYPAD_DATA board_keypad_param =
{
    .active_mode = 0,
    .keymap      = board_keypad_keymap,
    .keymap_cnt  = ARRAY_SIZE(board_keypad_keymap),
    .pwroff      = board_keypad_pwroff,
    .pwroff_cnt  = ARRAY_SIZE(board_keypad_pwroff),
    .clock       = "gpiokp_apb_clk",
};

#define board_keypad_device_resource concatenate(ISLAND_BOARD_ID, _keypad_device_resource)
static struct resource board_keypad_device_resource[] = {
    [0] = {
        .start = KEYPAD_BASE_ADDR,
        .end   = KEYPAD_BASE_ADDR + 0xD0,
        .flags = IORESOURCE_MEM,
    },
    [1] = {
        .start = BCM_INT_ID_KEYPAD,
        .end   = BCM_INT_ID_KEYPAD,
        .flags = IORESOURCE_IRQ,
    },
};

#define board_keypad_device concatenate(ISLAND_BOARD_ID, _keypad_device)
static struct platform_device board_keypad_device =
{
   .name          = "kona_keypad",
   .id            = -1,
   .resource      = board_keypad_device_resource,
   .num_resources = ARRAY_SIZE(board_keypad_device_resource),
   .dev = {
      .platform_data = &board_keypad_param,
   },
};
#endif

#if defined(CONFIG_BCM_AAA) || defined(CONFIG_BCM_AAA_MODULE)
static struct platform_device board_bcm_aaa_device = {
   .name = "bcm-aaa",
   .id = -1,
};
#endif

#if defined(CONFIG_BCM_GPS) || defined(CONFIG_BCM_GPS_MODULE)
#define board_hana_gps_info concatenate(ISLAND_BOARD_ID, _board_hana_gps_info)
static struct gps_platform_data board_hana_gps_info = GPS_PLATFORM_DATA_SETTINGS;

#define platform_device_gps concatenate(ISLAND_BOARD_ID, _platform_device_gps)
static struct platform_device platform_device_gps = 
{
   .name = "gps",
   .id = -1,
   .dev = {
      .platform_data = &board_hana_gps_info,
   },
};
#endif

#if defined(CONFIG_BCM_HAPTICS) || defined(CONFIG_BCM_HAPTICS_MODULE)
#define board_bcm_haptics_device concatenate(ISLAND_BOARD_ID, _bcm_haptics_device)

#define board_bcm_haptics_data concatenate(ISLAND_BOARD_ID, _board_bcm_haptics_data)
static struct bcm_haptics_data board_bcm_haptics_data = BCM_HAPTICS_SETTINGS;

static struct platform_device board_bcm_haptics_device = {
   .name = BCM_HAPTICS_DRIVER_NAME,
   .id = -1,
   .dev = {
      .platform_data = &board_bcm_haptics_data,
   },
};
#endif

#if defined(CONFIG_KONA_OTG_CP) || defined(CONFIG_KONA_OTG_CP_MODULE)
static struct resource otg_cp_resource[] = {
	[0] = {
		.start = HSOTG_CTRL_BASE_ADDR,
		.end = HSOTG_CTRL_BASE_ADDR + OTG_CTRL_CORE_REG_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = BCM_INT_ID_USB_OTG_DRV_VBUS,
		.end = BCM_INT_ID_USB_OTG_DRV_VBUS,
		.flags = IORESOURCE_IRQ,
	},
};

static struct platform_device otg_cp_device =
{
	.name = "kona-otg-cp",
	.id = -1,
	.resource = otg_cp_resource,
	.num_resources = ARRAY_SIZE(otg_cp_resource),
};
#endif

#if defined(CONFIG_VC_VCHIQ) || defined(CONFIG_VC_VCHIQ_MODULE)

/****************************************************************************
*
*   VCEB display device
*
***************************************************************************/

#if defined( CONFIG_VC_VCHIQ_MEMDRV_HANA ) || defined( CONFIG_VC_VCHIQ_MEMDRV_HANA_MODULE ) \
 || defined( CONFIG_VC_VCHIQ_BUSDRV_SHAREDMEM ) || defined( CONFIG_VC_VCHIQ_BUSDRV_SHAREDMEM_MODULE )

/* Internal videocore is defined - Assume that it's for display */

#define  VCEB_DISPLAY_DEVICE

static VCEB_PLATFORM_DATA_HANA_T vceb_hana_display_data =
{
    .create_params =
    {
        .instance_name = "display",
        .videocore_param = "vca",
        .host_param = &vceb_hana_display_data,
    },

    .vcMemAddr          = KONA_VC_EMI,
    .vcSramAddr         = KONA_INT_SRAM_BASE + BCMHANA_ARAM_VC_OFFSET,

    .bootFromKernel     = 1,
#if 0
    /* commenting out for the time being since GPIO mux group is hardcoded for the time being */
    .gpiomux_lcd_group  = gpiomux_group_vc_dpi_rgb,
    .gpiomux_lcd_id     = 0,
    .gpiomux_lcd_label  = "vc-lcd",

    .gpiomux_jtag_group  = gpiomux_group_vc_jtag,
    .gpiomux_jtag_id     = 0,
    .gpiomux_jtag_label  = "vc-jtag",
#endif
};

static struct platform_device vceb_display_device = {
   .name = "vceb_hana",
   .id = 0,
    .dev = {
       .platform_data = &vceb_hana_display_data,
    },
};

#endif
/****************************************************************************
*
*   VCHIQ display device
*
***************************************************************************/
#if defined( CONFIG_VC_VCHIQ_MEMDRV_HANA ) || defined( CONFIG_VC_VCHIQ_MEMDRV_HANA_MODULE )

/*
 * Internal videocore using the vchiq_arm stack
 */

#define  VCHIQ_DISPLAY_DEVICE

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

#elif defined( CONFIG_VC_VCHIQ_BUSDRV_SHAREDMEM ) || defined( CONFIG_VC_VCHIQ_BUSDRV_SHAREDMEM_MODULE )

/*
 * Internal videocore using the vchiq stack.
 */

#define  VCHIQ_DISPLAY_DEVICE

static VCHIQ_PLATFORM_DATA_HANA_T vchiq_display_data_shared_mem = {
    .common = {
        .instance_name  = "display",
        .dev_type       = VCHIQ_DEVICE_TYPE_HOST_PORT,
    },
};

static struct platform_device vchiq_display_device = {
    .name = "vchiq_busdrv_sharedmem",
    .id = 0,
    .dev = {
        .platform_data = &vchiq_display_data_shared_mem,
    },
};

#endif

/****************************************************************************
*
*   VCEB framebuffer device
*
***************************************************************************/

#if defined( CONFIG_FB_VCEB ) || defined( CONFIG_FB_VCEB_MODULE )

#define VCEB_FB_DEVICE

static struct platform_device vceb_fb_device = {
   .name = "vceb_fb",
   .id = -1,
};
#endif  /* CONFIG_FB_VCEB */

struct platform_device * vchiq_devices[] __initdata = { &vceb_display_device, &vchiq_display_device, &vceb_fb_device };

#endif  /* CONFIG_VC_VCHIQ */


#if defined(CONFIG_SENSORS_BMA150) || defined(CONFIG_SENSORS_BMA150_MODULE)

#define board_bma150_axis_change concatenate(ISLAND_BOARD_ID, _bma150_axis_change)

#ifdef BMA150_DRIVER_AXIS_SETTINGS
   static struct t_bma150_axis_change board_bma150_axis_change = BMA150_DRIVER_AXIS_SETTINGS;
#endif

static struct i2c_board_info __initdata i2c_bma150_info[] =
{
   {
      I2C_BOARD_INFO(BMA150_DRIVER_NAME, BMA150_DRIVER_SLAVE_NUMBER_0x38),
#ifdef BMA150_DRIVER_AXIS_SETTINGS
      .platform_data  = &board_bma150_axis_change,
#endif
   }, 
};
#endif

#if defined(CONFIG_SENSORS_BH1715) || defined(CONFIG_SENSORS_BH1715_MODULE)
static struct i2c_board_info __initdata i2c_bh1715_info[] =
{
	{
		I2C_BOARD_INFO(BH1715_DRV_NAME, BH1715_I2C_ADDR),
	},
};
#endif

#if defined(CONFIG_SENSORS_MPU3050) || defined(CONFIG_SENSORS_MPU3050_MODULE)

#define board_mpu3050_data concatenate(ISLAND_BOARD_ID, _mpu3050_data)

#ifdef MPU3050_DRIVER_AXIS_SETTINGS
   static struct t_mpu3050_axis_change board_mpu3050_axis_change = MPU3050_DRIVER_AXIS_SETTINGS;
#endif

static struct mpu3050_platform_data board_mpu3050_data = 
{ 
   .gpio_irq_pin = MPU3050_GPIO_IRQ_PIN,
   .scale        = MPU3050_SCALE,
#ifdef MPU3050_DRIVER_AXIS_SETTINGS
   .p_axis_change = &board_mpu3050_axis_change,
#else
   .p_axis_change = 0,
#endif
};

static struct i2c_board_info __initdata i2c_mpu3050_info[] = 
{
	{
		I2C_BOARD_INFO(MPU3050_DRV_NAME, MPU3050_I2C_ADDR),
		.platform_data  = &board_mpu3050_data,
	},
};
#endif

#if defined(CONFIG_BMP18X_I2C) || defined(CONFIG_BMP18X_I2C_MODULE)
static struct i2c_board_info __initdata i2c_bmp18x_info[] = 
{
	{
		I2C_BOARD_INFO(BMP18X_NAME, BMP18X_I2C_ADDRESS),
	},
};
#endif

#if defined(CONFIG_BCMBLT_RFKILL) || defined(CONFIG_BCMBLT_RFKILL_MODULE)
#define board_bcmblt_rfkill_cfg concatenate(ISLAND_BOARD_ID, _bcmblt_rfkill_cfg)
static struct bcmblt_rfkill_platform_data board_bcmblt_rfkill_cfg =
{
#ifdef BCMBLT_RFKILL_GPIO
   .gpio = BCMBLT_RFKILL_GPIO,
#endif
};
#define board_bcmblt_rfkill_device concatenate(ISLAND_BOARD_ID, _bcmblt_rfkill_device)
static struct platform_device board_bcmblt_rfkill_device = 
{
   .name = "bcmblt-rfkill",
   .id = 1,
   .dev =
   {
      .platform_data = &board_bcmblt_rfkill_cfg,
   },
}; 

static void __init board_add_bcmblt_rfkill_device(void)
{
   platform_device_register(&board_bcmblt_rfkill_device);
}
#endif


#if defined(CONFIG_MONITOR_ADC121C021_I2C) || defined(CONFIG_MONITOR_ADC121C021_I2C_MODULE)

#define board_adc121c021_i2c_param concatenate(ISLAND_BOARD_ID, _adc121c021_i2c_param)
static struct I2C_ADC121C021_t board_adc121c021_i2c_param;

#define board_adc121c021_i2c_boardinfo concatenate(ISLAND_BOARD_ID, _adc121c021_i2c_boardinfo)
static struct i2c_board_info board_adc121c021_i2c_boardinfo[] =
{
   {
      .type = I2C_ADC121C021_DRIVER_NAME,          /* "adc121c021_i2c_drvr" */
      .addr = I2C_ADC121C021_DRIVER_SLAVE_NUMBER,  /* 0x54                  */
      .platform_data = &board_adc121c021_i2c_param,
   },
};
#endif

#if defined(CONFIG_MONITOR_BQ27541_I2C) || defined(CONFIG_MONITOR_BQ27541_I2C_MODULE)
#define board_bq27541_i2c_boardinfo concatenate(ISLAND_BOARD_ID, _bq27541_i2c_boardinfo)
static struct i2c_board_info board_bq27541_i2c_boardinfo[] =
{
   {
      .type = I2C_BQ27541_DRIVER_NAME,          /* "bq27541_i2c_drvr" */
      .addr = I2C_BQ27541_DRIVER_SLAVE_NUMBER,  /* 0x55               */
   },
};
#endif

#if defined(CONFIG_BATTERY_MAX17040) || defined(CONFIG_BATTERY_MAX17040_MODULE)
#define board_hana_max17040_info concatenate(ISLAND_BOARD_ID, _hana_max17040_info)
static struct max17040_platform_data board_hana_max17040_info = 
{     /* Function pointers used to discover battery or AC power status using GPIOs */
      .battery_online = NULL,
      .charger_online = NULL,
      .charger_enable = NULL,
};

#define board_max17040_i2c_boardinfo concatenate(ISLAND_BOARD_ID, _max17040_i2c_boardinfo)
static struct i2c_board_info board_max17040_i2c_boardinfo[] =
{
   {
      .type = HW_MAX17040_DRIVER_NAME,             /* "max17040" */
      .addr = HW_MAX17040_SLAVE_ADDR,              /* 0x36       */  
      .platform_data = &board_hana_max17040_info,
   },
};
#endif

#if defined(CONFIG_BCM_CMP_BATTERY_MULTI) || defined(CONFIG_BCM_CMP_BATTERY_MULTI_MODULE)
#define board_hana_cmp_battery_multi_info concatenate(ISLAND_BOARD_ID, _board_hana_cmp_battery_multi_info)
static struct cbm_platform_data board_hana_cmp_battery_multi_info = CMP_BATTERY_MULTI_SETTINGS;

#define board_battery_multi concatenate(ISLAND_BOARD_ID, _board_battery_multi)
static struct platform_device board_battery_multi = 
{
   .name = "cmp-battery",
   .id = -1,
   .dev = {
      .platform_data = &board_hana_cmp_battery_multi_info,
   },
};
#endif

#if defined(CONFIG_BCM_CMP_BATTERY_BQ24616) || defined(CONFIG_BCM_CMP_BATTERY_BQ24616_MODULE)
#define battery_bq24616_data concatenate(ISLAND_BOARD_ID, _battery_bq24616_data)
static struct battery_bq24616_cfg battery_bq24616_data = 
{
    .gpio_ctl_pwr = HW_BATTERY_CTL_PWR,
    .gpio_acp_shd = HW_BATTERY_ACP_SHDN,
    .gpio_docking_station_power_3_3v = GPIO_DOCKING_STATION_POWER_3_3V,
};

#define board_battery_bq24616 concatenate(ISLAND_BOARD_ID, _board_battery_bq24616)
static struct platform_device board_battery_bq24616 = 
{
   .name = "cmp-battery",
   .id = -1,
   .dev = {
      .platform_data = &battery_bq24616_data,
   },
};
#endif

static void __init add_sdio_device(void)
{
   unsigned int i, id, num_devices;

   num_devices = ARRAY_SIZE(sdio_param);
   if (num_devices > MAX_SDIO_DEVICES)
      num_devices = MAX_SDIO_DEVICES;

   /*
    * Need to register eMMC as the first SDIO device so it grabs mmcblk0 when
    * it's installed. This required for rootfs to be mounted properly
    * 
    * Ask Darwin for why we need to do this
    */
   for (i = 0; i < num_devices; i++)
   {
      id = sdio_param[i].id;
      if (id < MAX_SDIO_DEVICES)
      {
         if (sdio_param[i].devtype == SDIO_DEV_TYPE_EMMC)
         {
            sdio_devices[id].dev.platform_data = &sdio_param[i];
            platform_device_register(&sdio_devices[id]);
         }
      }
   }

   for (i = 0; i < num_devices; i++)
   {
      id = sdio_param[i].id;

      /* skip eMMC as it has been registered */
      if (sdio_param[i].devtype == SDIO_DEV_TYPE_EMMC)
         continue;

      if (id < MAX_SDIO_DEVICES)
      {
         if (sdio_param[i].devtype == SDIO_DEV_TYPE_WIFI)
         {
            struct sdio_wifi_gpio_cfg *wifi_gpio =
               &sdio_param[i].wifi_gpio;

#ifdef HW_WLAN_GPIO_RESET_PIN
            wifi_gpio->reset = HW_WLAN_GPIO_RESET_PIN;
#else
            wifi_gpio->reset = -1;
#endif
#ifdef HW_WLAN_GPIO_SHUTDOWN_PIN
            wifi_gpio->shutdown = HW_WLAN_GPIO_SHUTDOWN_PIN;
#else
            wifi_gpio->shutdown = -1;
#endif
#ifdef HW_WLAN_GPIO_REG_PIN
            wifi_gpio->reg = HW_WLAN_GPIO_REG_PIN;
#else
            wifi_gpio->reg = -1;
#endif
#ifdef HW_WLAN_GPIO_HOST_WAKE_PIN      
            wifi_gpio->host_wake = HW_WLAN_GPIO_HOST_WAKE_PIN;
#else
            wifi_gpio->host_wake = -1;
#endif
         }
         sdio_devices[id].dev.platform_data = &sdio_param[i];
         platform_device_register(&sdio_devices[id]);
      }
   }
}

static void __init add_i2c_device(void)
{
	unsigned int i, num_devices;

	num_devices = ARRAY_SIZE(i2c_adap_param);
	if (num_devices == 0)
		return;
	if (num_devices > MAX_I2C_ADAPS)
 		num_devices = MAX_I2C_ADAPS;

	for (i = 0; i < num_devices; i++) {
		/* DO NOT register the I2C device if it is disabled */
		if (i2c_adap_param[i].disable == 1)
		continue;

		i2c_adap_devices[i].dev.platform_data = &i2c_adap_param[i];
		platform_device_register(&i2c_adap_devices[i]);
	}

#if defined(CONFIG_TOUCHSCREEN_EGALAX_I2C) || defined(CONFIG_TOUCHSCREEN_EGALAX_I2C_MODULE)
#ifdef HW_EGALAX_I2C_BUS_ID
	egalax_i2c_param.id = HW_EGALAX_I2C_BUS_ID;
#endif

#ifdef HW_EGALAX_GPIO_RESET
	egalax_i2c_param.gpio.reset = HW_EGALAX_GPIO_RESET;
#endif

#ifdef HW_EGALAX_GPIO_EVENT
	egalax_i2c_param.gpio.event = HW_EGALAX_GPIO_EVENT;
#endif
	
	egalax_i2c_boardinfo[0].irq =
		gpio_to_irq(egalax_i2c_param.gpio.event);

	i2c_register_board_info(egalax_i2c_param.id, egalax_i2c_boardinfo,
		ARRAY_SIZE(egalax_i2c_boardinfo));
#endif

#if defined(CONFIG_SENSORS_BMA150) || defined(CONFIG_SENSORS_BMA150_MODULE)

   i2c_register_board_info(
#ifdef SENSORS_BMA150_I2C_BUS_ID
      SENSORS_BMA150_I2C_BUS_ID,
#else
      -1,
#endif
      i2c_bma150_info, ARRAY_SIZE(i2c_bma150_info));
#endif

#if defined(CONFIG_SENSORS_BH1715) || defined(CONFIG_SENSORS_BH1715_MODULE)
   i2c_register_board_info(
#ifdef BH1715_I2C_BUS_ID
      BH1715_I2C_BUS_ID,
#else
      -1,
#endif
      i2c_bh1715_info, ARRAY_SIZE(i2c_bh1715_info));
#endif

#if defined(CONFIG_SENSORS_MPU3050) || defined(CONFIG_SENSORS_MPU3050_MODULE)
   i2c_register_board_info(
#ifdef MPU3050_I2C_BUS_ID
      MPU3050_I2C_BUS_ID,  
#else
      -1,
#endif
      i2c_mpu3050_info, ARRAY_SIZE(i2c_mpu3050_info));
#endif

#if defined(CONFIG_BMP18X_I2C) || defined(CONFIG_BMP18X_I2C_MODULE)
			i2c_register_board_info(
#ifdef BMP18X_I2C_BUS_ID
      BMP18X_I2C_BUS_ID,
#else
      -1,
#endif
      i2c_bmp18x_info, ARRAY_SIZE(i2c_bmp18x_info));
#endif

#if defined(CONFIG_MONITOR_ADC121C021_I2C)  || defined(CONFIG_MONITOR_ADC121C021_I2C_MODULE)
   board_adc121c021_i2c_param.id                = HW_ADC121C021_I2C_BUS_ID; 
   board_adc121c021_i2c_param.gpio_irq_pin      = HW_ADC121C021_GPIO_EVENT;
   board_adc121c021_i2c_param.num_bytes_to_read = HW_ADC121C021_BYTES_TO_READ;
   board_adc121c021_i2c_param.i2c_slave_address = I2C_ADC121C021_DRIVER_SLAVE_NUMBER;
   board_adc121c021_i2c_param.battery_max_voltage = HW_BATTERY_MAX_VOLTAGE;
   board_adc121c021_i2c_param.battery_min_voltage = HW_BATTERY_MIN_VOLTAGE;
   board_adc121c021_i2c_param.resistor_1          = HW_ADC121C021_RESISTOR_1;
   board_adc121c021_i2c_param.resistor_2          = HW_ADC121C021_RESISTOR_2;

   printk("board_template.c %s() IRQ pin %d\n", __FUNCTION__, board_adc121c021_i2c_param.gpio_irq_pin);
   board_adc121c021_i2c_boardinfo[0].irq = 
       gpio_to_irq(board_adc121c021_i2c_param.gpio_irq_pin);

   i2c_register_board_info(board_adc121c021_i2c_param.id,
                           board_adc121c021_i2c_boardinfo,
                           ARRAY_SIZE(board_adc121c021_i2c_boardinfo));         
#endif

#if defined(CONFIG_MONITOR_BQ27541_I2C) || defined(CONFIG_MONITOR_BQ27541_I2C_MODULE)
   i2c_register_board_info(HW_BQ27541_I2C_BUS_ID,
                           board_bq27541_i2c_boardinfo,
                           ARRAY_SIZE(board_bq27541_i2c_boardinfo));
#endif

#if defined(CONFIG_BATTERY_MAX17040) || defined(CONFIG_BATTERY_MAX17040_MODULE)
   i2c_register_board_info(HW_MAX17040_I2C_BUS_ID,
                           board_max17040_i2c_boardinfo,
                           ARRAY_SIZE(board_max17040_i2c_boardinfo));
#endif

}

#if defined(CONFIG_LEDS_GPIO) || defined(CONFIG_LEDS_GPIO_MODULE)
#define board_add_led_device concatenate(ISLAND_BOARD_ID, _add_led_device)
static void __init board_add_led_device(void)
{
   platform_device_register(&board_leds_gpio_device);
}
#endif

#if defined(CONFIG_KEYBOARD_GPIO) || defined(CONFIG_KEYBOARD_GPIO_MODULE)
#define board_add_keys_device concatenate(ISLAND_BOARD_ID, _add_keyboard_device)
static void __init board_add_keys_device(void)
{
   platform_device_register(&board_gpio_keys_device);
}
#endif

#if defined(CONFIG_KEYBOARD_KONA) || defined(CONFIG_KEYBOARD_KONA_MODULE)
#define board_add_keyboard_kona concatenate(ISLAND_BOARD_ID, _add_keyboard_kona)
static void __init board_add_keyboard_kona(void)
{
   platform_device_register(&board_keypad_device);
}
#endif


static void __init add_usbh_device(void)
{
	/*
	 * Always register the low level USB host device before EHCI/OHCI
	 * devices. Also, always add EHCI device before OHCI
	 */
	platform_device_register(&usbh_device);
	platform_device_register(&usbh_ehci_device);
	platform_device_register(&usbh_ohci_device);
}

static void __init add_usb_otg_device(void)
{
#if defined(CONFIG_KONA_OTG_CP) || defined(CONFIG_KONA_OTG_CP_MODULE)
	platform_device_register(&otg_cp_device);
#endif

#if defined(CONFIG_MAX3353) || defined(CONFIG_MAX3353_MODULE)
#ifdef HW_OTG_MAX3353_I2C_BUS_ID
	max3353_info.id = HW_OTG_MAX3353_I2C_BUS_ID;
#else
	max3353_info.id = -1;
#endif
#ifdef HW_OTG_MAX3353_GPIO_INT
	max3353_info.irq_gpio_num = HW_OTG_MAX3353_GPIO_INT;
#else
	max3353_info.irq_gpio_num = -1;
#endif
	i2c_register_board_info(max3353_info.id, max3353_i2c_boardinfo, ARRAY_SIZE(max3353_i2c_boardinfo));
#endif
}

#define board_halaudio_dev_list concatenate(ISLAND_BOARD_ID, _halaudio_dev_list)
static HALAUDIO_DEV_CFG board_halaudio_dev_list[] =
#ifdef HALAUDIO_DEV_LIST
   HALAUDIO_DEV_LIST;
#else
   NULL;
#endif

#define board_halaudio_cfg concatenate(ISLAND_BOARD_ID, _halaudio_cfg)
static HALAUDIO_CFG board_halaudio_cfg;

#define board_halaudio_device concatenate(ISLAND_BOARD_ID, _halaudio_device)
static struct platform_device board_halaudio_device =
{
   .name = "bcm-halaudio",
   .id = -1, /* to indicate there's only one such device */
   .dev =
   {
      .platform_data = &board_halaudio_cfg,
   },
};

#define board_halaudio_audio_info concatenate(ISLAND_BOARD_ID, _halaudio_audioh_info)
static HALAUDIO_AUDIOH_PLATFORM_INFO board_halaudio_audioh_info =
{
   .spk_en_gpio =
   {
#ifdef HALAUDIO_AUDIOH_SETTINGS_GPIO_HANDSFREE_LEFT_EN
      .handsfree_left_en = HALAUDIO_AUDIOH_SETTINGS_GPIO_HANDSFREE_LEFT_EN,
#else
      .handsfree_left_en = -1,
#endif
#ifdef HALAUDIO_AUDIOH_SETTINGS_GPIO_HANDSFREE_RIGHT_EN
      .handsfree_right_en = HALAUDIO_AUDIOH_SETTINGS_GPIO_HANDSFREE_RIGHT_EN,
#else
      .handsfree_right_en = -1,
#endif

#ifdef HALAUDIO_AUDIOH_SETTINGS_GPIO_HEADSET_EN
      .headset_en = HALAUDIO_AUDIOH_SETTINGS_GPIO_HEADSET_EN,
#else
      .headset_en = -1,
#endif
   },
};

#define board_halaudio_audioh_device concatenate(ISLAND_BOARD_ID, _halaudio_audioh_device)
static struct platform_device board_halaudio_audioh_device =
{
   .name = "bcm-halaudio-audioh",
   .id = -1, /* to indicate there's only one such device */
   .dev =
   {
      .platform_data = &board_halaudio_audioh_info,
   },
};

#define board_halaudio_pcm_info concatenate(ISLAND_BOARD_ID, _halaudio_pcm_info)
static HALAUDIO_PCM_PLATFORM_INFO board_halaudio_pcm_info =
{
#ifdef HALAUDIO_PCM_SETTINGS_CORE_ID_SELECT
   .core_id_select = HALAUDIO_PCM_SETTINGS_CORE_ID_SELECT,
#else
   .core_id_select = -1,
#endif
#ifdef HALAUDIO_PCM_SETTINGS_CHANS_SUPPORTED
   .channels = HALAUDIO_PCM_SETTINGS_CHANS_SUPPORTED,
#endif
#ifdef HALAUDIO_PCM_SETTINGS_CHAN_SELECT
   .channel_select = HALAUDIO_PCM_SETTINGS_CHAN_SELECT,
#endif
   .bt_gpio =
   {
#ifdef HALAUDIO_PCM_SETTINGS_GPIO_BT_RST_B
      .rst_b = HALAUDIO_PCM_SETTINGS_GPIO_BT_RST_B,
#else
      .rst_b = -1,
#endif
#ifdef HALAUDIO_PCM_SETTINGS_GPIO_BT_VREG_CTL
      .vreg_ctl = HALAUDIO_PCM_SETTINGS_GPIO_BT_VREG_CTL,
#else
      .vreg_ctl = -1,
#endif
#ifdef HALAUDIO_PCM_SETTINGS_GPIO_BT_WAKE
      .wake = HALAUDIO_PCM_SETTINGS_GPIO_BT_WAKE,
#else
      .wake = -1,
#endif
   },
#ifdef HALAUDIO_PCM_SETTINGS_BT_REQ_UART_GPIO_GROUP
   .bt_req_uart_gpio_group = HALAUDIO_PCM_SETTINGS_BT_REQ_UART_GPIO_GROUP,
#else
   .bt_req_uart_gpio_group = -1,
#endif
};

#ifndef HALAUDIO_PCM_SETTINGS_GPIO_BT_RST_B
#define BT_SUPPORT    0
#else
#define BT_SUPPORT    1
#endif

#define board_halaudio_pcm_device concatenate(ISLAND_BOARD_ID, _halaudio_pcm_device)
static struct platform_device board_halaudio_pcm_device =
{
   .name = "bcm-halaudio-pcm",
   .id = -1, /* to indicate there's only one such device */
   .dev =
   {
      .platform_data = &board_halaudio_pcm_info,
   },
};

#define board_add_halaudio_device concatenate(ISLAND_BOARD_ID, _add_halaudio_device)
static void __init board_add_halaudio_device(void)
{
   board_halaudio_cfg.numdev = ARRAY_SIZE(board_halaudio_dev_list);
   board_halaudio_cfg.devlist = board_halaudio_dev_list;
   platform_device_register(&board_halaudio_device);
   platform_device_register(&board_halaudio_audioh_device);
   platform_device_register(&board_halaudio_pcm_device);
}

static void __init add_devices(void)
{
#ifdef HW_SDIO_PARAM
	add_sdio_device();
#endif

#ifdef HW_I2C_ADAP_PARAM
	add_i2c_device();
#endif

#if defined(CONFIG_LEDS_GPIO) || defined(CONFIG_LEDS_GPIO_MODULE)
        board_add_led_device();
#endif

#if defined(CONFIG_KEYBOARD_GPIO) || defined(CONFIG_KEYBOARD_GPIO_MODULE)
        board_add_keys_device();
#endif

#if defined(CONFIG_KEYBOARD_KONA) || defined(CONFIG_KEYBOARD_KONA_MODULE)
        board_add_keyboard_kona();
#endif

#if defined(CONFIG_BCMBLT_RFKILL) || defined(CONFIG_BCMBLT_RFKILL_MODULE)
        board_add_bcmblt_rfkill_device();
#endif

#if defined(CONFIG_BCM_CMP_BATTERY_MULTI) || defined(CONFIG_BCM_CMP_BATTERY_MULTI_MODULE)
        platform_device_register(&board_battery_multi);
#endif

#if defined(CONFIG_BCM_CMP_BATTERY_BQ24616) || defined(CONFIG_BCM_CMP_BATTERY_BQ24616_MODULE)
        platform_device_register(&board_battery_bq24616);
#endif

#if defined(CONFIG_BCM_HAPTICS) || defined(CONFIG_BCM_HAPTICS_MODULE)
   platform_device_register(&board_bcm_haptics_device);
#endif

	add_usbh_device();
	add_usb_otg_device();

   board_add_halaudio_device();

#ifdef CONFIG_NET_ISLAND
	platform_device_register(&net_device);
#endif

#if defined(CONFIG_BCM_AAA) || defined(CONFIG_BCM_AAA_MODULE)
   platform_device_register(&board_bcm_aaa_device);
#endif   

#if defined(CONFIG_BCM_GPS) || defined(CONFIG_BCM_GPS_MODULE)
   platform_device_register(&platform_device_gps);
#endif

#if defined(CONFIG_VC_VCHIQ) || defined(CONFIG_VC_VCHIQ_MODULE)
   platform_add_devices( vchiq_devices, ARRAY_SIZE( vchiq_devices ) );
#endif
}

static void __init board_init(void)
{
#ifdef CONFIG_MAP_SDMA
	dma_mmap_init();
	sdma_init();
#endif
	/*
	 * Add common platform devices that do not have board dependent HW
	 * configurations
	 */
	board_add_common_devices();

	/* add devices with board dependent HW configurations */
	add_devices();
}

/*
 * Template used by board-xxx.c to create new board instance
 */
#define CREATE_BOARD_INSTANCE(name) \
MACHINE_START(name, #name) \
	.phys_io = IO_START, \
	.io_pg_offst = (IO_BASE >> 18) & 0xFFFC, \
	.map_io = island_map_io, \
	.init_irq = kona_init_irq, \
	.timer  = &kona_timer, \
	.init_machine = board_init, \
MACHINE_END
