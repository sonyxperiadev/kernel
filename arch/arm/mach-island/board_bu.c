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
#include <linux/irq.h>
#include <linux/gpio_keys.h>
#include <linux/input.h>
#include <linux/i2c/tsc2007.h>
#include <mach/hardware.h>
#include <asm/mach/arch.h>
#include <asm/mach-types.h>
#include <asm/gpio.h>

#include <mach/kona.h>
#include <mach/sdio_platform.h>
#include <asm/io.h>
#include <mach/rdb/brcm_rdb_uartb.h>
#include <mach/rdb/brcm_rdb_chipreg.h>

#include <linux/mfd/bcm590xx/core.h>
#include <linux/mfd/bcm590xx/pmic.h>
#include <linux/android_pmem.h>

#include <linux/smb380.h>
#include "island.h"
#include "common.h"

// #include <linux/regulator/machine.h>
// #include <linux/regulator/consumer.h>
// #include <linux/regulator/userspace-consumer.h>

#ifdef CONFIG_REGULATOR_BCM_PMU59055_A0
#include <linux/mfd/bcm590xx/bcm59055_A0.h>
#endif

#include <linux/regulator/max8649.h>

#define KONA_SDIO0_PA   SDIO1_BASE_ADDR
#define KONA_SDIO1_PA   SDIO2_BASE_ADDR
#define KONA_SDIO2_PA   SDIO3_BASE_ADDR
#define SDIO_CORE_REG_SIZE 0x10000

#define BSC_CORE_REG_SIZE      0x100

enum ALT_FUNC {
   ALT_1 = 0x0 << CHIPREG_GPIO_7_PINSEL_2_0_SHIFT,
   ALT_2 = 0x1 << CHIPREG_GPIO_7_PINSEL_2_0_SHIFT,
   ALT_3 = 0x2 << CHIPREG_GPIO_7_PINSEL_2_0_SHIFT,
   ALT_4 = 0x3 << CHIPREG_GPIO_7_PINSEL_2_0_SHIFT,
   ALT_GPIO = ALT_4,
};

#define GPIO_PULL_UP CHIPREG_GPIO_7_PUP_GPIO_7_MASK
#define GPIO_DRIVE_STRENGTH 0x3 << CHIPREG_GPIO_7_SEL_2_0_SHIFT

/*
 * GPIO pin for Touch screen pen down interrupt
 */
#define TSC2007_PEN_DOWN_GPIO_PIN   143

/*
 * Set to 0 for active high (pull-down) mode
 *        1 for active low (pull-up) mode
 */
#define HW_NUM_GPIO_KEYS     4

#define HW_KEYPAD_ACTIVE_MODE     0

/* 32 ~ 64 ms gives appropriate debouncing */
#define HW_KEYPAD_DEBOUNCE_TIME   KEYPAD_DEBOUNCE_64MS

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
      .num_resources = ARRAY_SIZE(board_i2c0_resource),
   },
   {  /* for BSC1 */
      .name = "bsc-i2c",
      .id = 1,
      .resource = board_i2c1_resource,
      .num_resources = ARRAY_SIZE(board_i2c1_resource),
   },
   {  /* for PMU BSC */
      .name = "bsc-i2c",
      .id = 2,
      .resource = board_pmu_bsc_resource,
      .num_resources = ARRAY_SIZE(board_pmu_bsc_resource),
   },
};

#if defined(CONFIG_KEYBOARD_GPIO)
static struct gpio_keys_button board_gpio_keys_button[] = {
   { KEY_HOME, 166, 1, "Home", EV_KEY, 0, 64},
   { KEY_SEARCH, 167, 1, "Search", EV_KEY, 0, 64},
   { KEY_MENU, 172, 1, "Menu", EV_KEY, 0, 64},
   { KEY_BACK, 164, 1, "Back", EV_KEY, 0, 64},
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
      printk(KERN_ERR "set_irq_type failed with irq %d\n",
                     gpio_to_irq(TSC2007_PEN_DOWN_GPIO_PIN));
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
   .model = 2007,
   .x_plate_ohms = 510, // For Sharp K3889TP Touch panel device
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

#if 0
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
#endif

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

#ifdef CONFIG_REGULATOR_MAX8649 
#ifdef CONFIG_MAX8649_SUPPORT_CHANGE_VID_MODE
void island_maxim_platform_hw_init_1(void ) ;
void island_maxim_platform_hw_init_2(void ) ;

struct regulator_consumer_supply max8649_supply1 = { .supply = "vc_core" };
struct regulator_init_data max8649_init_data1 = {
    .constraints	= 
    {
        .name = "vc_core", 
        .min_uV = 1210000, 
        .max_uV	= 1280000, 
        .always_on = 0, 
        .boot_on = 0, 
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
    .init = island_maxim_platform_hw_init_1, 
} ;

struct platform_device max8649_vc1 =  { 
    .name = "reg-virt-consumer",      
    .id = 0,           
    .dev = 
    { 
        .platform_data = "vc_core" , 
    }, 
};

struct i2c_board_info max_switch_info_1[] = { 
{ 
    .type		= "max8649", 
    .addr		= 0x60, 
    .platform_data	= &max8649_info1, 
    }, 
};

/***** Second Maxim part init data ( ARM part )*********/
struct regulator_consumer_supply max8649_supply2 = { .supply = "arm_core" };

struct regulator_init_data max8649_init_data2 = {
    .constraints	= 
    {
        .name = "arm_core", 
        .min_uV = 1210000, 
        .max_uV = 1280000, 
        .always_on = 0, 
        .boot_on = 0, 
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
    .init = island_maxim_platform_hw_init_2, 
} ;

struct platform_device max8649_vc2 =  { 
    .name = "reg-virt-consumer",      
    .id = 1,           
    .dev = 
    { 
        .platform_data = "arm_core" , 
    }, 
};

struct i2c_board_info max_switch_info_2[] = { 
    { 
        .type		= "max8649", 
        .addr		= 0x62, 
        .platform_data	= &max8649_info2, 
    }, 
};

struct platform_device *maxim_devices_1[] __initdata = { &max8649_vc1 } ;
struct platform_device *maxim_devices_2[] __initdata = { &max8649_vc2 };

void island_maxim_platform_hw_init_1(void )
{
    printk("REG: island_maxim_platform_hw_init for VC called\n") ;
    platform_add_devices(maxim_devices_1, ARRAY_SIZE(maxim_devices_1));
}

void island_maxim_platform_hw_init_2(void )
{
    printk("REG: island_maxim_platform_hw_init for ARM called \n") ;
    platform_add_devices(maxim_devices_2, ARRAY_SIZE(maxim_devices_2));
}

#endif
#endif

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

#ifdef CONFIG_INPUT_SMB380
#define BMA150_IRQ_PIN 140

static struct smb380_platform_data bma150_plat_data = {
   .range = RANGE_2G,
   .bandwidth = BW_375HZ, 
   .enable_adv_int = 1,
   .new_data_int = 0 ,
   .hg_int = 1 ,
   .lg_int = 1 ,
   .lg_dur = 150 ,
   .lg_thres = 20 ,
   .lg_hyst = 0 ,
   .hg_dur = 60 ,
   .hg_thres = 160 ,
   .hg_hyst = 0 ,
   .any_motion_dur  = 2 ,
   .any_motion_thres  = 30 ,
   .any_motion_int  = 1 ,
};

static struct i2c_board_info __initdata bma150_info[] = 
{
   {  
      I2C_BOARD_INFO("smb380", 0x38 ), 
      .platform_data  = &bma150_plat_data,
      .irq = gpio_to_irq(BMA150_IRQ_PIN),
   },
};
#endif

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
void __init board_map_io(void)
{
   /* Map machine specific iodesc here */

   island_map_io();
}

static struct platform_device *board_devices[] __initdata = {
   &board_i2c_adap_devices[0],
   &board_i2c_adap_devices[1],
   &board_i2c_adap_devices[2],
	&island_sdio1_device,
	&island_sdio2_device,

   &island_ipc_device,
	&android_pmem,
};

static void __init board_add_devices(void)
{
   platform_add_devices(board_devices, ARRAY_SIZE(board_devices));
#ifdef CONFIG_TOUCHSCREEN_TSC2007
   i2c_register_board_info(1, 
                           tsc2007_info,
                           ARRAY_SIZE(tsc2007_info));
#endif

#if defined(CONFIG_KEYBOARD_GPIO)
   platform_device_register(&board_gpio_keys_device);
#endif

#ifdef CONFIG_REGULATOR_MAX8649 
#ifdef CONFIG_MAX8649_SUPPORT_CHANGE_VID_MODE
   i2c_register_board_info(2,              
                           max_switch_info_1,
                           ARRAY_SIZE(max_switch_info_1));
   i2c_register_board_info(2,              
                           max_switch_info_2,
                           ARRAY_SIZE(max_switch_info_2));
#endif
#endif

#ifdef CONFIG_REGULATOR_BCM_PMU590XX
   printk("REG: i2c_register_board_info for pmu called \n") ;

   i2c_register_board_info(2,              
                           pmu_info,
                           ARRAY_SIZE(pmu_info));
#endif

#ifdef CONFIG_INPUT_SMB380
   printk("REG: i2c_register_board_info for BMA150 called \n") ;
   i2c_register_board_info(2,              
                           bma150_info,
                           ARRAY_SIZE(bma150_info));

   // Setup GPIO properties for interrupt from sensor.
   int rc = 0 ; 
   rc = set_irq_type(gpio_to_irq(BMA150_IRQ_PIN), IRQ_TYPE_EDGE_FALLING);
   if (rc < 0)
   {
      printk("set_irq_type failed with irq %d\n", gpio_to_irq(BMA150_IRQ_PIN));
      return ;
   }
   rc = gpio_request(BMA150_IRQ_PIN, "bma_pen_down");
   if (rc < 0)
   {
      printk("unable to request GPIO pin %d\n", BMA150_IRQ_PIN);
      return ;
   }
   gpio_direction_input(BMA150_IRQ_PIN);

#endif

}

void __init board_init(void)
{
	void __iomem *chipRegBase;

	chipRegBase = IOMEM(KONA_CHIPREG_VA);
#ifdef CONFIG_TOUCHSCREEN_TSC2007
	// Set pinmux for GPIO_143 which is used for TSC2007's interrupt pin
	writel(ALT_GPIO | GPIO_PULL_UP | GPIO_DRIVE_STRENGTH,  chipRegBase + CHIPREG_SIM2_RESETN_OFFSET);
#endif

#ifdef CONFIG_KEYBOARD_GPIO
	// Set pinmux for GPIO_166 which is used for <Home> key
	writel(ALT_GPIO | GPIO_PULL_UP | GPIO_DRIVE_STRENGTH,  chipRegBase + CHIPREG_NORFLSH_ADLAT_EN_OFFSET);
	// Set pinmux for GPIO_167 which is used for <Search> key
	writel(ALT_GPIO | GPIO_PULL_UP | GPIO_DRIVE_STRENGTH,  chipRegBase + CHIPREG_NORFLSH_AADLAT_EN_OFFSET);
	// Set pinmux for GPIO_172 which is used for <Menu> key
	writel(ALT_GPIO | GPIO_PULL_UP | GPIO_DRIVE_STRENGTH,  chipRegBase + CHIPREG_NORFLSH_ADDR_20_OFFSET);
	// Set pinmux for GPIO_164 which is used for <Back> key
	writel(ALT_GPIO | GPIO_PULL_UP | GPIO_DRIVE_STRENGTH,  chipRegBase + CHIPREG_NORFLSH_AD_14_OFFSET);
#endif

#ifdef CONFIG_INPUT_SMB380
   // Setup pin muxing for sensor interrupt pin.
   int val = 0 ;
   val = val | ( 3 << CHIPREG_SIM2_DET_PINSEL_2_0_SHIFT ) |      // Set ALT value to 3.
               ( 3 << CHIPREG_SIM2_DET_SEL_2_0_SHIFT )    ;      // Set drive strength to 3.
   writel( val,  ( chipRegBase + CHIPREG_SIM2_DET_OFFSET ) ) ;
#endif
	/*
	 * Add common platform devices that do not have board dependent HW
	 * configurations
	 */
	board_add_common_devices();
	board_add_devices();
	return;
}


MACHINE_START(ISLAND, "Island BU")
   .phys_io = IO_START,
   .io_pg_offst = (IO_BASE >> 18) & 0xFFFC,
   .map_io = board_map_io,
   .init_irq = kona_init_irq,
   .timer  = &kona_timer,
   .init_machine = board_init,
MACHINE_END
