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
#include <linux/irq.h>
#include <linux/kernel_stat.h>
#include <asm/mach/arch.h>
#include <asm/mach-types.h>
#include <mach/hardware.h>
#include <linux/i2c.h>
#include <linux/i2c-kona.h>
#include <linux/gpio_keys.h>
#include <linux/input.h>
#include <asm/gpio.h>
#ifdef CONFIG_GPIO_PCA953X
#include <linux/i2c/pca953x.h>
#endif
#ifdef CONFIG_TOUCHSCREEN_QT602240
#include <linux/i2c/qt602240_ts.h>
#endif
#include <mach/kona_headset_pd.h>
#include <mach/kona.h>
#include <mach/rhea.h>
#include <asm/mach/map.h>
#include <linux/mfd/bcm590xx/core.h>
#include <linux/mfd/bcm590xx/pmic.h>
#include <linux/mfd/bcm590xx/bcm59055_A0.h>
#include <linux/clk.h>
#include "common.h"
#ifdef CONFIG_KEYBOARD_BCM
#include <mach/bcm_keypad.h>
#endif
#ifdef CONFIG_DMAC_PL330
#include <mach/irqs.h>
#include <plat/pl330-pdata.h>
#include <linux/dma-mapping.h>
#endif
#include <linux/spi/spi.h>
#include <mach/rdb/brcm_rdb_csr.h>

#define PMU_DEVICE_I2C_ADDR_0   0x08
#define PMU_IRQ_PIN           29

// keypad map
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

static int __init bcm590xx_init_platform_hw(struct bcm590xx *bcm590xx, int flag)
{
	int ret;
	printk("REG: pmu_init_platform_hw called \n") ;
	switch (flag) {
		case BCM590XX_INITIALIZATION:
			ret = gpio_request(PMU_IRQ_PIN, "pmu_irq");
			if (ret < 0) {
				printk(KERN_ERR "%s unable to request GPIO pin %d\n", __FUNCTION__, PMU_IRQ_PIN);
				return ret ;
			}
			gpio_direction_input(PMU_IRQ_PIN);
			break;
		default:
			return -EPERM;
	}
	return 0 ;
}

#ifdef CONFIG_BATTERY_BCM59055
/* wall charging and vbus are wired together on FF board
     we monitor USB activity to make sure it is not USB cable that is inserted
 */
static int can_start_charging(void* data)
{
#define INTERVAL (HZ/10)
	int cpu, usb_otg_int[4], i;
	for_each_present_cpu(cpu)
		usb_otg_int[cpu] =  kstat_irqs_cpu(
				BCM_INT_ID_USB_HSOTG, cpu);

	for (i=0; i<10; i++) {
		schedule_timeout_interruptible(INTERVAL);
		for_each_present_cpu(cpu)
			if (usb_otg_int[cpu]!= kstat_irqs_cpu(
						BCM_INT_ID_USB_HSOTG, cpu))
				return 0;
	}
	return 1;
}

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

/* Regulator registration */
struct regulator_consumer_supply sim_supply[] = {
	{ .supply = "sim_vcc" },
};

static struct regulator_init_data bcm59055_simldo_data = {
	.constraints = {
		.name = "simldo",
		.min_uV = 1300000,
		.max_uV = 3300000,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS |
			REGULATOR_CHANGE_VOLTAGE,
		.always_on = 0,
		.boot_on = 0,
	},
	.num_consumer_supplies = ARRAY_SIZE(sim_supply),
	.consumer_supplies = sim_supply,
};

static struct bcm590xx_regulator_init_data bcm59055_regulators[] =
{
	{BCM59055_SIMLDO, &bcm59055_simldo_data, BCM590XX_REGL_LPM_IN_DSM},
};

static struct bcm590xx_regulator_pdata bcm59055_regl_pdata = {
	.num_regulator	= ARRAY_SIZE(bcm59055_regulators),
	.init			= &bcm59055_regulators,
	.default_pmmode = {
		[BCM59055_RFLDO]	= 0x00,
		[BCM59055_CAMLDO] 	= 0x00,
		[BCM59055_HV1LDO]	= 0x00,
		[BCM59055_HV2LDO]	= 0x00,
		[BCM59055_HV3LDO]	= 0x00,
		[BCM59055_HV4LDO]	= 0x00,
		[BCM59055_HV5LDO]	= 0x00,
		[BCM59055_HV6LDO]	= 0x00,
		[BCM59055_HV7LDO]	= 0x00,
		[BCM59055_SIMLDO]	= 0x00,
		[BCM59055_CSR]		= 0x00,
		[BCM59055_IOSR]		= 0x00,
		[BCM59055_SDSR]		= 0x00,
	},
};

static struct bcm590xx_platform_data bcm590xx_plat_data = {
	.init = bcm590xx_init_platform_hw,
	.flag = BCM590XX_USE_REGULATORS | BCM590XX_ENABLE_AUDIO |
	BCM590XX_USE_PONKEY | BCM590XX_USE_RTC,
#ifdef CONFIG_BATTERY_BCM59055
	.battery_pdata = &bcm590xx_battery_plat_data,
#endif
	.regl_pdata = &bcm59055_regl_pdata,
};


static struct i2c_board_info __initdata pmu_info[] =
{
	{
		I2C_BOARD_INFO("bcm59055", PMU_DEVICE_I2C_ADDR_0 ),
		.irq = gpio_to_irq(PMU_IRQ_PIN),
		.platform_data  = &bcm590xx_plat_data,
	},
};

#ifdef CONFIG_KEYBOARD_BCM
/*!
 * The keyboard definition structure.
 */
struct platform_device bcm_kp_device = {
	.name = "bcm_keypad",
	.id = -1,
};

/*	Keymap for Ray board plug-in 64-key keypad.
	Since LCD block has used pin GPIO00, GPIO01, GPIO02, GPIO03,
	GPIO08, GPIO09, GPIO10 and GPIO11, Keypad can be set as 4x4 matric by
	using pin GPIO04, GPIO05, GPIO06, GPIO07, GPIO12, GPIO13, GPIO14 and
	GPIO15 */
static struct bcm_keymap newKeymap[] = {
	{BCM_KEY_ROW_0, BCM_KEY_COL_0, "unused", 0},
	{BCM_KEY_ROW_0, BCM_KEY_COL_1, "unused", 0},
	{BCM_KEY_ROW_0, BCM_KEY_COL_2, "unused", 0},
	{BCM_KEY_ROW_0, BCM_KEY_COL_3, "unused", 0},
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
	{BCM_KEY_ROW_4, BCM_KEY_COL_4, "Search Key", KEY_SEARCH},
	{BCM_KEY_ROW_4, BCM_KEY_COL_5, "Back Key", KEY_BACK},
	{BCM_KEY_ROW_4, BCM_KEY_COL_6, "Forward key", KEY_FORWARD},
	{BCM_KEY_ROW_4, BCM_KEY_COL_7, "Home Key", KEY_HOME},
	{BCM_KEY_ROW_5, BCM_KEY_COL_0, "unused", 0},
	{BCM_KEY_ROW_5, BCM_KEY_COL_1, "unused", 0},
	{BCM_KEY_ROW_5, BCM_KEY_COL_2, "unused", 0},
	{BCM_KEY_ROW_5, BCM_KEY_COL_3, "unused", 0},
	{BCM_KEY_ROW_5, BCM_KEY_COL_4, "Menu-Key", KEY_MENU},
	{BCM_KEY_ROW_5, BCM_KEY_COL_5, "VolumnUp-Key", KEY_VOLUMEUP},
	{BCM_KEY_ROW_5, BCM_KEY_COL_6, "VolumnDown-Key", KEY_VOLUMEDOWN},
	{BCM_KEY_ROW_5, BCM_KEY_COL_7, "key mute", KEY_MUTE},
	{BCM_KEY_ROW_6, BCM_KEY_COL_0, "unused", 0},
	{BCM_KEY_ROW_6, BCM_KEY_COL_1, "unused", 0},
	{BCM_KEY_ROW_6, BCM_KEY_COL_2, "unused", 0},
	{BCM_KEY_ROW_6, BCM_KEY_COL_3, "unused", 0},
	{BCM_KEY_ROW_6, BCM_KEY_COL_4, "key space", KEY_SPACE},
	{BCM_KEY_ROW_6, BCM_KEY_COL_5, "key power", KEY_POWER},
	{BCM_KEY_ROW_6, BCM_KEY_COL_6, "key sleep", KEY_SLEEP},
	{BCM_KEY_ROW_6, BCM_KEY_COL_7, "key wakeup", KEY_WAKEUP},
	{BCM_KEY_ROW_7, BCM_KEY_COL_0, "unused", 0},
	{BCM_KEY_ROW_7, BCM_KEY_COL_1, "unused", 0},
	{BCM_KEY_ROW_7, BCM_KEY_COL_2, "unused", 0},
	{BCM_KEY_ROW_7, BCM_KEY_COL_3, "unused", 0},
	{BCM_KEY_ROW_7, BCM_KEY_COL_4, "unused", 0},
	{BCM_KEY_ROW_7, BCM_KEY_COL_5, "unused", 0},
	{BCM_KEY_ROW_7, BCM_KEY_COL_6, "unused", 0},
	{BCM_KEY_ROW_7, BCM_KEY_COL_7, "unused", 0},
};

static struct bcm_keypad_platform_info bcm_keypad_data = {
	.row_num = 8,
	.col_num = 8,
	.keymap = newKeymap,
	.bcm_keypad_base = (void *)__iomem HW_IO_PHYS_TO_VIRT(KEYPAD_BASE_ADDR),
};

#endif

#ifdef CONFIG_GPIO_PCA953X
#define GPIO_PCA953X_GPIO_PIN      74 /* Configure pad MMC1DAT4 to GPIO74 */
static int pca953x_platform_init_hw(struct i2c_client *client,
		unsigned gpio, unsigned ngpio, void *context)
{
	int rc;
	rc = gpio_request(GPIO_PCA953X_GPIO_PIN, "gpio_expander");
	if (rc < 0)
	{
		printk(KERN_ERR "unable to request GPIO pin %d\n", GPIO_PCA953X_GPIO_PIN);
		return rc;
	}
	gpio_direction_input(GPIO_PCA953X_GPIO_PIN);
	return 0;
}

static int pca953x_platform_exit_hw(struct i2c_client *client,
		unsigned gpio, unsigned ngpio, void *context)
{
	gpio_free(GPIO_PCA953X_GPIO_PIN);
	return 0;
}

static struct pca953x_platform_data board_expander_info = {
	.gpio_base	= KONA_MAX_GPIO,
	.irq_base	= gpio_to_irq(KONA_MAX_GPIO),
	.setup		= pca953x_platform_init_hw,
	.teardown	= pca953x_platform_exit_hw,
};

static struct i2c_board_info __initdata pca953x_info[] = {
	{
		I2C_BOARD_INFO("pca9539", 0x74),
		.irq = gpio_to_irq(GPIO_PCA953X_GPIO_PIN),
		.platform_data = &board_expander_info,
	},
};
#endif /* CONFIG_GPIO_PCA953X */

#ifdef CONFIG_TOUCHSCREEN_QT602240
#ifdef CONFIG_GPIO_PCA953X
#define QT602240_INT_GPIO_PIN      (KONA_MAX_GPIO + 8)
#else
#define QT602240_INT_GPIO_PIN      74 /* skip expander chip */
#endif
static int qt602240_platform_init_hw(void)
{
	int rc;
	rc = gpio_request(QT602240_INT_GPIO_PIN, "ts_qt602240");
	if (rc < 0)
	{
		printk(KERN_ERR "unable to request GPIO pin %d\n", QT602240_INT_GPIO_PIN);
		return rc;
	}
	gpio_direction_input(QT602240_INT_GPIO_PIN);

	return 0;
}

static void qt602240_platform_exit_hw(void)
{
	gpio_free(QT602240_INT_GPIO_PIN);
}

static struct qt602240_platform_data qt602240_platform_data = {
	.x_line		= 17,
	.y_line		= 11,
	.x_size		= 800,
	.y_size		= 480,
	.blen		= 0x21,
	.threshold	= 0x28,
	.voltage	= 2800000,              /* 2.8V */
	.orient		= QT602240_DIAGONAL_COUNTER,
	.init_platform_hw = qt602240_platform_init_hw,
	.exit_platform_hw = qt602240_platform_exit_hw,
};

static struct i2c_board_info __initdata qt602240_info[] = {
	{
		I2C_BOARD_INFO("qt602240_ts", 0x4a),
		.platform_data = &qt602240_platform_data,
		.irq = gpio_to_irq(QT602240_INT_GPIO_PIN),
	},
};
#endif /* CONFIG_TOUCHSCREEN_QT602240 */

#ifdef CONFIG_KONA_HEADSET
#define HS_IRQ	gpio_to_irq(71)
#define HSB_IRQ	BCM_INT_ID_AUXMIC_COMP1
static struct kona_headset_pd headset_data = {
	.hs_default_state = 1, /* GPIO state read is 0 on HS insert and 1 for
							* HS remove*/
};

static struct resource board_headset_resource[] = {
	{	/* For AUXMIC */
		.start = AUXMIC_BASE_ADDR,
		.end = AUXMIC_BASE_ADDR + SZ_4K - 1,
		.flags = IORESOURCE_MEM,
	},
	{	/* For ACI */
		.start = ACI_BASE_ADDR,
		.end = ACI_BASE_ADDR + SZ_4K - 1,
		.flags = IORESOURCE_MEM,
	},
	{	/* For Headset IRQ */
		.start = HS_IRQ,
		.end = HS_IRQ,
		.flags = IORESOURCE_IRQ,
	},
	{	/* For Headset button IRQ */
		.start = HSB_IRQ,
		.end = HSB_IRQ,
		.flags = IORESOURCE_IRQ,
	},
};

struct platform_device headset_device = {
	.name = "konaheadset",
	.id = -1,
	.resource = board_headset_resource,
	.num_resources	= ARRAY_SIZE(board_headset_resource),
	.dev	=	{
		.platform_data = &headset_data,
	},
};
#endif /* CONFIG_KONA_HEADSET */

#ifdef CONFIG_DMAC_PL330
static struct kona_pl330_data rhea_pl330_pdata =	{
	/* Non Secure DMAC virtual base address */
	.dmac_ns_base = KONA_DMAC_NS_VA,
	/* Secure DMAC virtual base address */
	.dmac_s_base = KONA_DMAC_S_VA,
	/* # of PL330 dmac channels 'configurable' */
	.num_pl330_chans = 8,
	/* irq number to use */
	.irq_base = BCM_INT_ID_RESERVED184,
	/* # of PL330 Interrupt lines connected to GIC */
	.irq_line_count = 8,
};

static struct platform_device pl330_dmac_device = {
	.name = "kona-dmac-pl330",
	.id = 0,
	.dev = {
		.platform_data = &rhea_pl330_pdata,
		.coherent_dma_mask  = DMA_BIT_MASK(64),
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
	/* TODO: adding more slaves here */
};

/* Rhea Ray specific platform devices */
static struct platform_device *rhea_ray_plat_devices[] __initdata = {
#ifdef CONFIG_KEYBOARD_BCM
	&bcm_kp_device,
#endif

#ifdef CONFIG_KONA_HEADSET
	&headset_device,
#endif

#ifdef CONFIG_DMAC_PL330
	&pl330_dmac_device,
#endif
};

/* Rhea Ray specific i2c devices */
static void __init rhea_ray_add_i2c_devices (void)
{
	/* 59055 on BSC - PMU */
	i2c_register_board_info(2,
			pmu_info,
			ARRAY_SIZE(pmu_info));
}

static int __init rhea_ray_add_lateInit_devices (void)
{
	struct i2c_adapter *adapter;
	struct i2c_client *client;

	adapter = i2c_get_adapter(1);
	if (!adapter) {
		printk(KERN_ERR "can't get i2c adapter 1\n");
		return ENODEV;
	}
#ifdef CONFIG_GPIO_PCA953X
	client = i2c_new_device(adapter, pca953x_info);
	if (!client) {
		printk(KERN_ERR "an't add i2c device for pca953x\n");
	}
#endif

#ifdef CONFIG_TOUCHSCREEN_QT602240
	client = i2c_new_device(adapter, qt602240_info);
	if (!client) {
		printk(KERN_ERR "an't add i2c device for qt602240\n");
	}
#endif
	i2c_put_adapter(adapter);

	board_add_sdio_devices();
	return 0;
}

static void enable_smi_display_clks(void)
{
	struct clk *smi_axi;
	struct clk *mm_dma;
	struct clk *smi;

	smi_axi = clk_get (NULL, "smi_axi_clk");
	mm_dma = clk_get (NULL, "mm_dma_axi_clk");

	smi = clk_get (NULL, "smi_clk");
	BUG_ON (!smi_axi || !smi || !mm_dma);


	clk_set_rate (smi, 250000000);

	clk_enable (smi_axi);
	clk_enable (smi);
	clk_enable(mm_dma);
}

static void boost_memc(void)
{
	volatile uint32_t hw_freq_chng_ctl;
        volatile uint32_t hw_pwr_down_ctl;

	writel(0x3, (KONA_MEMC0_NS_VA + CSR_MEMC_MAX_PWR_STATE_OFFSET));
	writel(0x3, (KONA_MEMC0_NS_VA + CSR_APPS_MIN_PWR_STATE_OFFSET));
	writel(0x3, (KONA_MEMC0_NS_VA + CSR_MODEM_MIN_PWR_STATE_OFFSET));

	hw_freq_chng_ctl = readl(KONA_MEMC0_NS_VA + CSR_HW_FREQ_CHANGE_CNTRL_OFFSET);
	hw_freq_chng_ctl &= ~(CSR_HW_FREQ_CHANGE_CNTRL_HW_AUTO_PWR_TRANSITION_MASK);
	writel(hw_freq_chng_ctl, (KONA_MEMC0_NS_VA + CSR_HW_FREQ_CHANGE_CNTRL_OFFSET));

    	hw_pwr_down_ctl = readl(KONA_MEMC0_NS_VA + CSR_DDR_SW_POWER_DOWN_CONTROL_OFFSET);
    	hw_pwr_down_ctl |= (1 << CSR_DDR_SW_POWER_DOWN_CONTROL_DISABLE_CLOCK_GATING_SHIFT);
    	writel(hw_pwr_down_ctl, KONA_MEMC0_NS_VA + CSR_DDR_SW_POWER_DOWN_CONTROL_OFFSET);
}

/* All Rhea Ray specific devices */
static void __init rhea_ray_add_devices(void)
{
	boost_memc();
	enable_smi_display_clks();

#ifdef CONFIG_KEYBOARD_BCM
	bcm_kp_device.dev.platform_data = &bcm_keypad_data;
#endif
	platform_add_devices(rhea_ray_plat_devices, ARRAY_SIZE(rhea_ray_plat_devices));

	rhea_ray_add_i2c_devices();

	spi_register_board_info(spi_slave_board_info,
				ARRAY_SIZE(spi_slave_board_info));
}

void __init board_init(void)
{
	board_add_common_devices();
	rhea_ray_add_devices();
	return;
}

void __init board_map_io(void)
{
	/* Map machine specific iodesc here */

	rhea_map_io();
}

late_initcall(rhea_ray_add_lateInit_devices);

MACHINE_START(RHEA, "RheaRay")
	.phys_io = IO_START,
	.io_pg_offst = (IO_BASE >> 18) & 0xFFFC,
	.map_io = board_map_io,
	.init_irq = kona_init_irq,
	.timer  = &kona_timer,
	.init_machine = board_init,
MACHINE_END
