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
#include <linux/serial_8250.h>

#include <linux/i2c.h>
#include <linux/irq.h>
#include <linux/gpio_keys.h>
#include <linux/input.h>
#include <linux/i2c/tsc2007.h>
#include <linux/i2c/tango_ts.h>


#include <mach/hardware.h>
#include <asm/mach/arch.h>
#include <asm/mach-types.h>
#include <asm/gpio.h>

#include <mach/kona.h>
#include <mach/sdio_platform.h>
#include <mach/rdb/brcm_rdb_uartb.h>

#include <linux/mfd/bcm590xx/core.h>
#include <linux/mfd/bcm590xx/pmic.h>

#ifdef CONFIG_BACKLIGHT_PWM
#include <linux/pwm_backlight.h>
#endif

#include "island.h"

// #include <linux/regulator/machine.h>
// #include <linux/regulator/consumer.h>
// #include <linux/regulator/userspace-consumer.h>

#ifdef CONFIG_REGULATOR_BCM_PMU59055_A0
#include <linux/mfd/bcm590xx/bcm59055_A0.h>
#endif

/*
 * todo: 8250 driver has problem autodetecting the UART type -> have to 
 * use FIXED type
 * confuses it as an XSCALE UART.  Problem seems to be that it reads
 * bit6 in IER as non-zero sometimes when it's supposed to be 0.
 */
#define KONA_UART0_PA	UARTB_BASE_ADDR
#define KONA_UART1_PA	UARTB2_BASE_ADDR
#define KONA_SDIO1_PA	SDIO2_BASE_ADDR
#define SDIO_CORE_REG_SIZE 0x10000

#define BSC_CORE_REG_SIZE      0x100

#define KONA_8250PORT(name)				\
{								\
	.membase    = (void __iomem *)(KONA_##name##_VA), 	\
	.mapbase    = (resource_size_t)(KONA_##name##_PA),    	\
	.irq	    = BCM_INT_ID_##name,               		\
	.uartclk    = 13000000,					\
	.regshift   = 2,					\
	.iotype	    = UPIO_DWAPB,					\
	.type	    = PORT_16550A,          			\
	.flags	    = UPF_BOOT_AUTOCONF | UPF_FIXED_TYPE | UPF_SKIP_TEST,	\
	.private_data = (void __iomem *)((KONA_##name##_VA) + UARTB_USR_OFFSET), \
}

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
#define TSC2007_PEN_DOWN_GPIO_PIN	0

#define TANGO_GPIO_IRQ_PIN    		0
#define TANGO_GPIO_RESET_PIN  		1
#define TANGO_I2C_TS_DRIVER_NUM_BYTES_TO_READ 14

/*
 * Set to 0 for active high (pull-down) mode
 *        1 for active low (pull-up) mode
 */
#define HW_NUM_GPIO_KEYS     6

#define HW_KEYPAD_ACTIVE_MODE     0

/* 32 ~ 64 ms gives appropriate debouncing */
#define HW_KEYPAD_DEBOUNCE_TIME   KEYPAD_DEBOUNCE_64MS

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

static struct plat_serial8250_port uart_data[] = {
	KONA_8250PORT(UART0),
	KONA_8250PORT(UART1),
	{
		.flags		= 0,
	},
};

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
   [0] =
   {
      .start = BSC2_BASE_ADDR,
      .end = BSC2_BASE_ADDR + BSC_CORE_REG_SIZE - 1,
      .flags = IORESOURCE_MEM,
   },
   [1] = 
   {
      .start = BCM_INT_ID_I2C1,
      .end = BCM_INT_ID_I2C1,
      .flags = IORESOURCE_IRQ,
   },
};

static struct resource board_pmu_bsc_resource[] = {
   [0] =
   {
      .start = PMU_BSC_BASE_ADDR,
      .end = PMU_BSC_BASE_ADDR + BSC_CORE_REG_SIZE - 1,
      .flags = IORESOURCE_MEM,
   },
   [1] = 
   {
      .start = BCM_INT_ID_PM_I2C,
      .end = BCM_INT_ID_PM_I2C,
      .flags = IORESOURCE_IRQ,
   },
};

static struct platform_device board_i2c_adap_devices[] =
{
   {  /* for BSC0 */
      .name = "bsc-i2c",
      .id = 0,
      .resource = board_i2c0_resource,
      .num_resources	= ARRAY_SIZE(board_i2c0_resource),
   },
   {  /* for BSC0 */
      .name = "bsc-i2c",
      .id = 1,
      .resource = board_i2c1_resource,
      .num_resources	= ARRAY_SIZE(board_i2c1_resource),
   },
   {  /* for PMU BSC */
      .name = "bsc-i2c",
      .id = 2,
      .resource = board_pmu_bsc_resource,
      .num_resources	= ARRAY_SIZE(board_pmu_bsc_resource),
   },
};

#if defined(CONFIG_KEYBOARD_GPIO)
static struct gpio_keys_button board_gpio_keys_button[] = {
   { KEY_HOME, 1, 1, "Home", EV_KEY, 0, 64},
   { KEY_SEARCH, 2, 1, "Search", EV_KEY, 0, 64},
   { KEY_MENU, 3, 1, "Menu", EV_KEY, 0, 64},
   { KEY_BACK, 4, 1, "Back", EV_KEY, 0, 64},
   { KEY_VOLUMEUP, 5, 1, "Volume up", EV_KEY, 0, 64},
   { KEY_VOLUMEDOWN, 6, 1, "Volume down", EV_KEY, 0, 64},
};

static struct gpio_keys_platform_data board_gpio_keys = {
	.buttons = board_gpio_keys_button,
	.nbuttons = HW_NUM_GPIO_KEYS,
	.rep = 1,		
};

static struct platform_device board_gpio_keys_device = {
   .name = "gpio-keys",
   .id = -1,
   .dev = {
      .platform_data = &board_gpio_keys,
   },
};

#endif

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
   if (rc < 0)
   {
      printk(KERN_ERR "set_irq_type failed with irq %d\n", BCM_INT_ID_GPIO1);
      return rc;
   }
   rc = gpio_request(TSC2007_PEN_DOWN_GPIO_PIN, "ts_pen_down");
   if (rc < 0)
   {
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
	.model			= 2007,
	.x_plate_ohms	= 510, // For Sharp K3889TP Touch panel device
	.get_pendown_state = NULL,
	.clear_penirq = NULL,
	.init_platform_hw = tsc2007_init_platform_hw,
	.exit_platform_hw = tsc2007_exit_platform_hw,
	.clear_penirq = tsc2007_clear_penirq,
};

static struct i2c_board_info __initdata tsc2007_info[] = 
{
   {  /* New touch screen i2c slave address. */
      I2C_BOARD_INFO("tsc2007", 0x48),
      .platform_data  = &tsc_plat_data,
      .irq = gpio_to_irq(TSC2007_PEN_DOWN_GPIO_PIN),
   },
};

#endif

#ifdef CONFIG_TOUCHSCREEN_TANGO_S32
static struct TANGO_I2C_TS_t tango_plat_data = {
   .i2c_slave_address = 0,
   .gpio_irq_pin      = TANGO_GPIO_IRQ_PIN,
   .gpio_reset_pin    = TANGO_GPIO_RESET_PIN,
   .x_max_value       = 480,
   .y_max_value       = 800,
   .layout            = X_RIGHT_Y_UP,
   .num_bytes_to_read = TANGO_I2C_TS_DRIVER_NUM_BYTES_TO_READ,
   .is_multi_touch    = IS_MULTI_TOUCH,
   .is_resetable      = 1,
   .num_fingers_idx   = 0,
   .old_touching_idx  = 1,
   .x1_lo_idx         = 2,
   .x1_hi_idx         = 3,
   .y1_lo_idx         = 4,
   .y1_hi_idx         = 5,
   .x2_lo_idx         = 6,
   .x2_hi_idx         = 7,
   .y2_lo_idx         = 8,
   .y2_hi_idx         = 9,
   .x1_width_idx      = 10,  // X1 coordinate touch area of the first finger
   .y1_width_idx      = 11,  // Y1 coordinate touch area of the first finger
   .x2_width_idx      = 12,  // X2 coordinate touch area of the first finger
   .y2_width_idx      = 13,  // Y2 coordinate touch area of the first finger
   .power_mode_idx    = 20,   
   .int_mode_idx      = 21,   // INT)mode register
   .int_width_idx     = 22,   // Interrupt pulse width  
   .min_finger_val    = 0,
   .max_finger_val    = MAX_NUM_FINGERS,
   .panel_width       = 56,
};

static struct i2c_board_info __initdata tango_info[] = 
{
   {  /* New touch screen i2c slave address. */
      I2C_BOARD_INFO(I2C_TS_DRIVER_NAME, 0x5C),
      .platform_data  = &tango_plat_data,
      .irq = gpio_to_irq(TANGO_GPIO_IRQ_PIN),
   },
};

#endif

static struct platform_device board_serial_device = {
	.name		= "serial8250",
	.id		= PLAT8250_DEV_PLATFORM,
	.dev		= {
		.platform_data = uart_data,
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

static struct sdio_platform_cfg board_sdio_param[] = {
   { /* SDIO1 */
      .id = 1,
      .data_pullup = 0,
      .devtype = SDIO_DEV_TYPE_EMMC,
   },
};

static struct platform_device island_sdio1_device = {
	.name = "sdhci",
	.id = 1,
	.resource = board_sdio1_resource,
	.num_resources	= ARRAY_SIZE(board_sdio1_resource),
	.dev		= {
		.platform_data = &board_sdio_param[0],
	},
};

#ifdef CONFIG_REGULATOR_BCM_PMU590XX
#define PMU_DEVICE_I2C_ADDR   0x08 
static int __init bcm590xx_init_platform_hw(struct bcm590xx *bcm590xx)
{
    // int i;
    printk("REG: pmu_init_platform_hw called \n") ;
#ifdef CONFIG_REGULATOR_BCM_PMU59055_A0
    bcm59055_reg_init_dev_init(bcm590xx)  ;
#endif

    return 0 ;
}

static struct bcm590xx_platform_data __initdata bcm590xx_plat_data = {
	.init = bcm590xx_init_platform_hw,
};


static struct i2c_board_info __initdata pmu_info[] = 
{
   {  /* New touch screen i2c slave address. */
      I2C_BOARD_INFO("bcm590xx", PMU_DEVICE_I2C_ADDR ), 
      .platform_data  = &bcm590xx_plat_data,
   },
};

#endif

void __init board_map_io(void)
{
	/* Map machine specific iodesc here */

	island_map_io();
}

static struct platform_device *board_devices[] __initdata = {
	&board_serial_device,
#if defined(CONFIG_BACKLIGHT_PWM)
	&pwm_backlight_device,
#endif
	&board_i2c_adap_devices[0],
	&board_i2c_adap_devices[1],
    &board_i2c_adap_devices[2],
	&island_sdio1_device,
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

#if defined(CONFIG_KEYBOARD_GPIO)
   platform_device_register(&board_gpio_keys_device);
#endif

#ifdef CONFIG_REGULATOR_BCM_PMU590XX
   printk("REG: i2c_register_board_info for pmu called \n") ;

   i2c_register_board_info(1,              // This is i2c adapter number. For fpga put it on i2c 1.
                           pmu_info,
                           ARRAY_SIZE(pmu_info));
#endif
}

void __init board_init(void)
{
	board_add_devices();
	return;
}

MACHINE_START(ISLAND, "Island FPGA")
	.phys_io = IO_START,
	.io_pg_offst = (IO_BASE >> 18) & 0xFFFC,
	.map_io = board_map_io,
	.init_irq = kona_init_irq,
	.timer  = &kona_timer,
	.init_machine = board_init,
MACHINE_END
