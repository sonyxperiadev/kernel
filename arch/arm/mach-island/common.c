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
#include <linux/init.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/serial_8250.h>
#include <linux/clk.h>
#include <linux/pwm_backlight.h>
#include <asm/mach-types.h>
#include <mach/hardware.h>
#include <mach/kona.h>
#include <mach/rdb/brcm_rdb_uartb.h>
#include <mach/irqs.h>

#if defined(CONFIG_USB_ANDROID)
#include <linux/usb/android_composite.h>
#endif

#define KONA_UART0_PA   UARTB_BASE_ADDR
#define KONA_UART1_PA   UARTB2_BASE_ADDR
#define KONA_UART2_PA   UARTB3_BASE_ADDR
#define KONA_UART3_PA   UARTB4_BASE_ADDR

#define UART_CLOCK_RATE 13000000

#define KONA_8250PORT(name,clk)                                                   \
{                                                                             \
   .membase    = (void __iomem *)(KONA_##name##_VA),                          \
   .mapbase    = (resource_size_t)(KONA_##name##_PA),                         \
   .irq        = BCM_INT_ID_##name,                                           \
   .uartclk    = UART_CLOCK_RATE,                                             \
   .regshift   = 2,                                                           \
   .iotype     = UPIO_DWAPB,                                                  \
   .type       = PORT_16550A,                                                 \
   .flags      = UPF_BOOT_AUTOCONF | UPF_FIXED_TYPE | UPF_SKIP_TEST,          \
   .private_data = (void __iomem *)((KONA_##name##_VA) + UARTB_USR_OFFSET),   \
   .clk_name	= clk,	\
}

struct uart_clk_cfg {
	char *bus_clk;
	char *peri_clk;
};

static struct uart_clk_cfg uart_clk[] = {
	[0] = {
		.bus_clk = "uartb_apb_clk",
		.peri_clk = "uartb_clk",
	},
	[1] = {
                .bus_clk = "uartb2_apb_clk",
                .peri_clk = "uartb2_clk",
        },
	[2] = {
                .bus_clk = "uartb3_apb_clk",
                .peri_clk = "uartb3_clk",
        },
	[3] = {
                .bus_clk = "uartb4_apb_clk",
                .peri_clk = "uartb4_clk",
        },
};

static struct plat_serial8250_port uart_data[] = {
	KONA_8250PORT(UART0,"uartb_clk"),
	KONA_8250PORT(UART1,"uartb2_clk"),
	KONA_8250PORT(UART2,"uartb3_clk"),
	KONA_8250PORT(UART3,"uartb4_clk"),
	{ .flags = 0, },
};

static struct platform_device board_serial_device = {
	.name = "serial8250",
	.id = PLAT8250_DEV_PLATFORM,
	.dev = {
		.platform_data = uart_data,
	},
};

#if defined(CONFIG_HW_RANDOM_KONA)
static struct resource rng_device_resource[] = {
    [0] = {
        .start = SEC_RNG_BASE_ADDR,
        .end   = SEC_RNG_BASE_ADDR + 0x14,
        .flags = IORESOURCE_MEM,
    },
    [1] = {
        .start = BCM_INT_ID_SECURE_TRAP1,
        .end   = BCM_INT_ID_SECURE_TRAP1,
        .flags = IORESOURCE_IRQ,
    },
};

static struct platform_device rng_device =
{
   .name          = "kona_rng",
   .id            = -1,
   .resource	  = rng_device_resource,
   .num_resources = ARRAY_SIZE(rng_device_resource),
};
#endif

#if defined(CONFIG_KONA_PWMC)
static struct resource pwm_device_resource[] = {
    [0] = {
        .start = PWM_BASE_ADDR,
        .end   = PWM_BASE_ADDR + 0x10,
        .flags = IORESOURCE_MEM,
    },
};

static struct platform_device pwm_device =
{
   .name          = "kona_pwmc",
   .id            = -1,
   .resource	  = pwm_device_resource,
   .num_resources = ARRAY_SIZE(pwm_device_resource),
};
#endif

#if defined(CONFIG_BACKLIGHT_PWM)
static struct platform_pwm_backlight_data pwm_backlight_data =
{
	.pwm_name	= "kona_pwmc:2",
	.max_brightness	= 255,
	.dft_brightness	= 255,
	.pwm_period_ns	= 5000000,
};

static struct platform_device pwm_backlight_device =
{
	.name     = "pwm-backlight",
	.id       = -1,
	.dev      =
		{
		.platform_data = &pwm_backlight_data,
	},
};
#endif

#if defined(CONFIG_W1_MASTER_DS1WM)
static struct resource d1w_device_resource[] = {
    [0] = {
        .start = D1W_BASE_ADDR,
        .end   = D1W_BASE_ADDR + 0x10,
        .flags = IORESOURCE_MEM,
    },
    [1] = {
        .start = BCM_INT_ID_DALLAS_1_WIRE,
        .end   = BCM_INT_ID_DALLAS_1_WIRE,
        .flags = IORESOURCE_IRQ,
    },
};

static struct platform_device d1w_device =
{
   .name          = "ds1wm",
   .id            = -1,
   .resource	  = d1w_device_resource,
   .num_resources = ARRAY_SIZE(d1w_device_resource),
};
#endif

#if defined(CONFIG_MPCORE_WATCHDOG)
static struct resource wdt_device_resource[] = {
    [0] = {
        .start = PTIM_BASE_ADDR,
        .end   = PTIM_BASE_ADDR + 0x34,
        .flags = IORESOURCE_MEM,
    },
    [1] = {
        .start = BCM_INT_ID_PPI14,
        .end   = BCM_INT_ID_PPI14,
        .flags = IORESOURCE_IRQ,
    },
};

static struct platform_device wdt_device =
{
   .name          = "mpcore_wdt",
   .id            = -1,
   .resource	  = wdt_device_resource,
   .num_resources = ARRAY_SIZE(wdt_device_resource),
   .dev = {
        .platform_data = "arm_periph_clk",
    },
};
#endif

#if defined(CONFIG_RTC_DRV_ISLAND)
static struct resource rtc_device_resource[] = {
    [0] = {
        .start = BBL_BASE_ADDR,
        .end   = BBL_BASE_ADDR + 0x24,
        .flags = IORESOURCE_MEM,
    },
    [1] = {
        .start = BCM_INT_ID_BBL1,
        .end   = BCM_INT_ID_BBL1,
        .flags = IORESOURCE_IRQ,
    },
    [2] = {
        .start = BCM_INT_ID_BBL2,
        .end   = BCM_INT_ID_BBL2,
        .flags = IORESOURCE_IRQ,
    },
};

static struct platform_device rtc_device =
{
   .name          = "bcmhana-rtc",
   .id            = -1,
   .resource	  = rtc_device_resource,
   .num_resources = ARRAY_SIZE(rtc_device_resource),
};
#endif

#if defined(CONFIG_USB_ANDROID)
/* FIXME borrow GOOGLE vendor ID to use windows driver */
#define GOOGLE_VENDOR_ID        0x18d1
#define VENDOR_ID               GOOGLE_VENDOR_ID
#define PRODUCT_ID              0x0001

/* FIXME need revise these IDs*/
#define UMS_PRODUCT_ID          PRODUCT_ID
#define UMS_ADB_PRODUCT_ID      0x0002
#define RNDIS_PRODUCT_ID        0x0ffe
#define RNDIS_ADB_PRODUCT_ID    0x0ffc

#if defined(CONFIG_USB_ANDROID_MASS_STORAGE)
static struct usb_mass_storage_platform_data mass_storage_pdata = {
   .nluns = 1,
   .vendor = "Broadcom",
   .product = "Media Broadcom Reference Design",
   .release = 0x0100,
};

static struct platform_device usb_mass_storage_device = {
   .name = "usb_mass_storage",
   .id = -1,
   .dev = {
             .platform_data = &mass_storage_pdata,
          },
};
#endif /* CONFIG_USB_ANDROID_MASS_STORAGE */

#if defined(CONFIG_USB_ANDROID_RNDIS)
static struct usb_ether_platform_data rndis_pdata = {
   /* ethaddr is filled by board_serialno_setup */
   .vendorID       = __constant_cpu_tole16(VENDOR_ID),
   .vendorDescr    = "Broadcom",
};

static struct platform_device rndis_device = {
   .name   = "rndis",
   .id     = -1,
   .dev    = {
                .platform_data = &rndis_pdata,
             },
};
#endif /* CONFIG_USB_ANDROID_RNDIS */

static char *usb_functions_ums[] = {
#if defined(CONFIG_USB_ANDROID_MASS_STORAGE)
   "usb_mass_storage",
#endif
};

static char *usb_functions_ums_adb[] = {
#if defined(CONFIG_USB_ANDROID_MASS_STORAGE)
   "usb_mass_storage",
#endif
#if defined(CONFIG_USB_ANDROID_ADB)
   "adb",
#endif
};

static char *usb_functions_rndis[] = {
#if defined(CONFIG_USB_ANDROID_RNDIS)
   "rndis",
#endif
};

static char *usb_functions_rndis_adb[] = {
#if defined(CONFIG_USB_ANDROID_RNDIS)
   "rndis",
#endif
#if defined(CONFIG_USB_ANDROID_ADB)
   "adb",
#endif
};

static char *usb_functions_all[] = {
#if defined(CONFIG_USB_ANDROID_MASS_STORAGE)
   "usb_mass_storage",
#endif
#if defined(CONFIG_USB_ANDROID_ADB)
   "adb",
#endif
#if defined(CONFIG_USB_ANDROID_RNDIS)
   "rndis",
#endif
#if defined(CONFIG_USB_ANDROID_ACM)
   "acm",
#endif
};

static struct android_usb_product usb_products[] = {
   {
      .product_id     = __constant_cpu_to_le16(UMS_PRODUCT_ID),
      .num_functions  = ARRAY_SIZE(usb_functions_ums),
      .functions      = usb_functions_ums,
   },
   {
      .product_id     = __constant_cpu_to_le16(UMS_ADB_PRODUCT_ID),
      .num_functions  = ARRAY_SIZE(usb_functions_ums_adb),
      .functions      = usb_functions_ums_adb,
   },
   {
      .product_id     = __constant_cpu_to_le16(RNDIS_PRODUCT_ID),
      .num_functions  = ARRAY_SIZE(usb_functions_rndis),
      .functions      = usb_functions_rndis,
   },
   {
      .product_id     = __constant_cpu_to_le16(RNDIS_ADB_PRODUCT_ID),
      .num_functions  = ARRAY_SIZE(usb_functions_rndis_adb),
      .functions      = usb_functions_rndis_adb,
   },
};

static struct android_usb_platform_data android_usb_pdata = {
   .vendor_id         = __constant_cpu_to_le16(VENDOR_ID),
   .product_id        = __constant_cpu_to_le16(PRODUCT_ID),
   .version           = 0x0100,
   .product_name      = "Media Broadcom Reference Design",
   .manufacturer_name = "Broadcom",
   .serial_number     = "0123456789ABCDEF",
   .num_products      = ARRAY_SIZE(usb_products),
   .products          = usb_products,
   .num_functions     = ARRAY_SIZE(usb_functions_all),
   .functions         = usb_functions_all,
};

static struct platform_device android_usb_device = {
   .name   = "android_usb",
   .id     = -1,
   .dev    = {
                .platform_data = &android_usb_pdata,
             },
};
#endif /* CONFIG_USB_ANDROID */

/* Common devices among all Island boards */
static struct platform_device *board_common_plat_devices[] __initdata = {
	&board_serial_device,
#if defined(CONFIG_MPCORE_WATCHDOG)
        &wdt_device,
#endif
#if defined(CONFIG_W1_MASTER_DS1WM)
        &d1w_device,
#endif
#if defined(CONFIG_HW_RANDOM_KONA)
        &rng_device,
#endif
#if defined(CONFIG_RTC_DRV_ISLAND)
        &rtc_device,
#endif
#if defined(CONFIG_KONA_PWMC)
        &pwm_device,
#endif
#if defined(CONFIG_BACKLIGHT_PWM)
	&pwm_backlight_device,
#endif
#if defined(CONFIG_USB_ANDROID)
        &android_usb_device,
#if defined(CONFIG_USB_ANDROID_MASS_STORAGE)
        &usb_mass_storage_device,
#endif /* CONFIG_USB_ANDROID_MASS_STORAGE */
#if defined(CONFIG_USB_ANDROID_RNDIS)
        &rndis_device,
#endif /* CONFIG_USB_ANDROID_RNDIS */
#endif /* CONFIG_USB_ANDROID */
};

void __init board_add_common_devices(void)
{
	struct clk *temp_clk;

	/*
	 * Pre-configure UARTB3 core clock to 29.5 MHz. Bluetooth needs to run
	 * at 921K which can be derived from 29.5 MHz but not 13 MHz
	 */
	temp_clk = clk_get(NULL, uart_clk[2].bus_clk);
	clk_enable(temp_clk);
	clk_put(temp_clk);

	temp_clk = clk_get(NULL, uart_clk[2].peri_clk);
	clk_set_rate(temp_clk, 29500000);
	uart_data[2].uartclk = 29500000;
	clk_enable(temp_clk);
	clk_put(temp_clk);

	platform_add_devices(board_common_plat_devices,
			ARRAY_SIZE(board_common_plat_devices));
}
