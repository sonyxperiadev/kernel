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
#include <linux/serial_8250.h>
#include <mach/rdb/brcm_rdb_uartb.h>
#include <mach/sdio_platform.h>
#ifdef CONFIG_GPIO_PCA953X
#include <linux/i2c/pca953x.h>
#endif
#ifdef CONFIG_REGULATOR_TPS728XX
#include <linux/regulator/tps728xx.h>
#endif
#include <mach/kona_headset_pd.h>
#include <mach/kona.h>
#include <mach/hawaii.h>
#include <asm/mach/map.h>
#include <linux/power_supply.h>
#include <linux/mfd/bcm590xx/core.h>
#include <linux/mfd/bcm590xx/pmic.h>
#include <linux/clk.h>
#include <linux/bootmem.h>
#include <plat/pi_mgr.h>
#include "devices.h"
#include <plat/chal/chal_trace.h>
#include <linux/spi/spi.h>
#include <plat/spi_kona.h>
#include <linux/spi/spi.h>
#include <plat/spi_kona.h>
#ifdef CONFIG_TOUCHSCREEN_QT602240
#include <linux/i2c/qt602240_ts.h>
#endif
#ifdef CONFIG_KONA_AVS
#include <plat/kona_avs.h>
#include "pm_params.h"
#endif
#ifdef CONFIG_STM_TRACE
#include <trace/stm.h>
#endif
#ifdef CONFIG_KEYBOARD_BCM
#include <mach/bcm_keypad.h>
#endif
#ifdef CONFIG_DMAC_PL330
#include <mach/irqs.h>
#include <plat/pl330-pdata.h>
#include <linux/dma-mapping.h>
#endif
#include <linux/spi/spi.h>
#if defined (CONFIG_HAPTIC)
#include <linux/haptic.h>
#endif
#if (defined(CONFIG_MPU_SENSORS_MPU6050A2) || defined(CONFIG_MPU_SENSORS_MPU6050B1))
#include <linux/mpu.h>
#endif
#define _HAWAII_
#include <mach/comms/platform_mconfig.h>

#include <linux/broadcom/chip_version.h>
#if (defined(CONFIG_BCM_RFKILL) || defined(CONFIG_BCM_RFKILL_MODULE))
#include <linux/broadcom/bcmbt_rfkill.h>
#endif

#ifdef CONFIG_BCM_BZHW
#include <linux/broadcom/bcm_bzhw.h>
#endif

#ifdef CONFIG_BCM_BT_LPM
#include <linux/broadcom/bcmbt_lpm.h>
#endif

#ifdef CONFIG_FB_BRCM_KONA
#include <video/kona_fb_boot.h>
#include <video/kona_fb.h>
#endif

#include <linux/pwm_backlight.h>

#ifdef CONFIG_GPIO_PCA953X
#define SD_CARDDET_GPIO_PIN      (KONA_MAX_GPIO + 15)
#else
#define SD_CARDDET_GPIO_PIN      75
#endif

#include <media/soc_camera.h>
#include <mach/rdb/brcm_rdb_sysmap.h>
#include <mach/rdb/brcm_rdb_padctrlreg.h>
#include <linux/delay.h>

#ifdef CONFIG_WD_TAPPER
#include <linux/broadcom/wd-tapper.h>
#endif


#ifdef CONFIG_BRCM_UNIFIED_DHD_SUPPORT

#include "board-Hawaii_wifi.h"
extern int hawaii_wifi_status_register(void (*callback) (int card_present, void *dev_id),
			  void *dev_id);

#endif

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

#ifdef CONFIG_MFD_BCM_PMU590XX
static int bcm590xx_event_callback(int flag, int param)
{
	int ret;
	printk("REG: pmu_init_platform_hw called \n");
	switch (flag) {
	case BCM590XX_INITIALIZATION:
		ret = gpio_request(PMU_IRQ_PIN, "pmu_irq");
		if (ret < 0) {
			printk(KERN_ERR "%s unable to request GPIO pin %d\n",
			       __FUNCTION__, PMU_IRQ_PIN);
			return ret;
		}
		gpio_direction_input(PMU_IRQ_PIN);
		break;
	case BCM590XX_CHARGER_INSERT:
		pr_info("%s: BCM590XX_CHARGER_INSERT\n", __func__);
		break;
	default:
		return -EPERM;
	}
	return 0;
}

static const char *pmu_clients[] = {
#ifdef CONFIG_BCMPMU_OTG_XCEIV
	"bcmpmu_otg_xceiv",
#endif
};

static struct bcm590xx_platform_data bcm590xx_plat_data = {
#ifdef CONFIG_KONA_PMU_BSC_HS_MODE
	/*
	 * PMU in High Speed (HS) mode. I2C CLK is 3.25MHz
	 * derived from 26MHz input clock.
	 *
	 * Hawaii: PMBSC is always in HS mode, i2c_pdata is not in use.
	 */
	.i2c_pdata = {ADD_I2C_SLAVE_SPEED(BSC_BUS_SPEED_HS),},
#else
	.i2c_pdata = {ADD_I2C_SLAVE_SPEED(BSC_BUS_SPEED_400K),},
#endif
	.pmu_event_cb = bcm590xx_event_callback,
	.clients = pmu_clients,
	.clients_num = ARRAY_SIZE(pmu_clients),
};

#endif

#ifdef CONFIG_MFD_BCM59039
struct regulator_consumer_supply hv6_supply[] = {
	{.supply = "vdd_sdxc"},
};

struct regulator_consumer_supply hv4_supply[] = {
	{.supply = "hv4"},
	{.supply = "2v9_vibra"},
	{.supply = "vdd_sdio"},
};
#endif

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
	GPIO08, GPIO09, GPIO10 and GPIO11, SSP3 and camera used GPIO06,
	GPIO07, GPIO12, GPIO13, for now keypad can only be set as a 2x2 matrix
	by using pin GPIO04, GPIO05, GPIO14 and GPIO15 */
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

static struct bcm_keypad_platform_info bcm_keypad_data = {
	.row_num = 8,
	.col_num = 8,
	.keymap = newKeymap,
	.bcm_keypad_base = (void *)__iomem HW_IO_PHYS_TO_VIRT(KEYPAD_BASE_ADDR),
};

#endif

#define FREQ_MHZ(x) ((x)*1000*1000)
unsigned long clock_get_xtal(void)
{
	return FREQ_MHZ(26);
}

static struct plat_serial8250_port uart_data[] = {
	KONA_8250PORT(UART0, "uartb_clk"),
	KONA_8250PORT(UART1, "uartb2_clk"),
	KONA_8250PORT(UART2, "uartb3_clk"),
	{
	 .flags = 0,
	 },
};


#ifdef CONFIG_GPIO_PCA953X

#if defined(CONFIG_MACH_HAWAII_RAY)
#define GPIO_PCA953X_GPIO_PIN      121	/* Configure pad MMC1DAT4 to GPIO74 */
#define GPIO_PCA953X_2_GPIO_PIN      122	/* Configure ICUSBDM pad to GPIO122 */
#endif

#define SENSOR_0_GPIO_PWRDN		12
#define SENSOR_0_GPIO_RST		(KONA_MAX_GPIO + 10)
#define SENSOR_0_CLK			"dig_ch0_clk"
#define SENSOR_0_CLK_FREQ		(13000000)

#define SENSOR_1_GPIO_PWRDN		13
#define SENSOR_1_CLK			"dig_ch0_clk"

static int pca953x_platform_init_hw(struct i2c_client *client,
				    unsigned gpio, unsigned ngpio,
				    void *context)
{
	int rc;
	rc = gpio_request(GPIO_PCA953X_GPIO_PIN, "gpio_expander");
	if (rc < 0) {
		printk(KERN_ERR "unable to request GPIO pin %d\n",
		       GPIO_PCA953X_GPIO_PIN);
		return rc;
	}
	gpio_direction_input(GPIO_PCA953X_GPIO_PIN);

	/*init sensor gpio here to be off */
	rc = gpio_request(SENSOR_0_GPIO_PWRDN, "CAM_STANDBY0");
	if (rc < 0)
		printk(KERN_ERR "unable to request GPIO pin %d\n",
		       SENSOR_0_GPIO_PWRDN);

	gpio_direction_output(SENSOR_0_GPIO_PWRDN, 0);
	gpio_set_value(SENSOR_0_GPIO_PWRDN, 0);

	rc = gpio_request(SENSOR_0_GPIO_RST, "CAM_RESET0");
	if (rc < 0)
		printk(KERN_ERR "unable to request GPIO pin %d\n",
		       SENSOR_0_GPIO_RST);

	gpio_direction_output(SENSOR_0_GPIO_RST, 0);
	gpio_set_value(SENSOR_0_GPIO_RST, 0);

	rc = gpio_request(SENSOR_1_GPIO_PWRDN, "CAM_STANDBY1");
	if (rc < 0)
		printk(KERN_ERR "unable to request GPIO pin %d\n",
		       SENSOR_1_GPIO_PWRDN);

	gpio_direction_output(SENSOR_1_GPIO_PWRDN, 0);
	gpio_set_value(SENSOR_1_GPIO_PWRDN, 0);

	return 0;
}

static int pca953x_platform_exit_hw(struct i2c_client *client,
				    unsigned gpio, unsigned ngpio,
				    void *context)
{
	gpio_free(GPIO_PCA953X_GPIO_PIN);
	return 0;
}

static struct pca953x_platform_data board_expander_info = {
	.i2c_pdata = {ADD_I2C_SLAVE_SPEED(BSC_BUS_SPEED_100K),},
	.gpio_base = KONA_MAX_GPIO,
	.irq_base = gpio_to_irq(KONA_MAX_GPIO),
	.setup = pca953x_platform_init_hw,
	.teardown = pca953x_platform_exit_hw,
};

static struct i2c_board_info __initdata pca953x_info[] = {
	{
	 I2C_BOARD_INFO("pca9539", 0x74),
	 .irq = gpio_to_irq(GPIO_PCA953X_GPIO_PIN),
	 .platform_data = &board_expander_info,
	 },
};

#ifdef CONFIG_MACH_HAWAII_RAY
/* Expander #2 */
static int pca953x_2_platform_init_hw(struct i2c_client *client,
				      unsigned gpio, unsigned ngpio,
				      void *context)
{
	int rc;
	rc = gpio_request(GPIO_PCA953X_2_GPIO_PIN, "gpio_expander_2");
	if (rc < 0) {
		printk(KERN_ERR "unable to request GPIO pin %d\n",
		       GPIO_PCA953X_2_GPIO_PIN);
		return rc;
	}
	gpio_direction_input(GPIO_PCA953X_2_GPIO_PIN);
	return 0;
}

static int pca953x_2_platform_exit_hw(struct i2c_client *client,
				      unsigned gpio, unsigned ngpio,
				      void *context)
{
	gpio_free(GPIO_PCA953X_2_GPIO_PIN);
	return 0;
}

static struct pca953x_platform_data board_expander_2_info = {
	.i2c_pdata = {ADD_I2C_SLAVE_SPEED(BSC_BUS_SPEED_100K),},
	.gpio_base = KONA_MAX_GPIO + 16,
	.irq_base = gpio_to_irq(KONA_MAX_GPIO + 16),
	.setup = pca953x_2_platform_init_hw,
	.teardown = pca953x_2_platform_exit_hw,
};

static struct i2c_board_info __initdata pca953x_2_info[] = {
	{
	 I2C_BOARD_INFO("pca9539", 0x75),
	 .irq = gpio_to_irq(GPIO_PCA953X_2_GPIO_PIN),
	 .platform_data = &board_expander_2_info,
	 },
};
#endif
#endif /* CONFIG_GPIO_PCA953X */

#ifdef CONFIG_TOUCHSCREEN_QT602240
#ifdef CONFIG_GPIO_PCA953X
#define QT602240_INT_GPIO_PIN      (KONA_MAX_GPIO + 8)
#else
#define QT602240_INT_GPIO_PIN      74	/* skip expander chip */
#endif
static int qt602240_platform_init_hw(void)
{
	int rc;
	rc = gpio_request(QT602240_INT_GPIO_PIN, "ts_qt602240");
	if (rc < 0) {
		printk(KERN_ERR "unable to request GPIO pin %d\n",
		       QT602240_INT_GPIO_PIN);
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
	.i2c_pdata = {ADD_I2C_SLAVE_SPEED(BSC_BUS_SPEED_100K),},
	.x_line = 15,
	.y_line = 11,
	.x_size = 1023,
	.y_size = 1023,
	.x_min = 90,
	.y_min = 90,
	.x_max = 0x3ff,
	.y_max = 0x3ff,
	.max_area = 0xff,
	.blen = 33,
	.threshold = 70,
	.voltage = 2700000,	/* 2.8V */
	.orient = QT602240_DIAGONAL_COUNTER,
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
#if (defined(CONFIG_MPU_SENSORS_MPU6050A2) || defined(CONFIG_MPU_SENSORS_MPU6050B1))
static struct mpu_platform_data mpu6050_data = {
	.int_config = 0x10,
	.orientation = {0, 1, 0,
			1, 0, 0,
			0, 0, -1},
	.level_shifter = 0,

	.accel = {
		  /*.get_slave_descr = mpu_get_slave_descr, */
		  .adapt_num = 2,
		  .bus = EXT_SLAVE_BUS_SECONDARY,
		  .address = 0x38,
		  .orientation = {0, 1, 0,
				  1, 0, 0,
				  0, 0, -1},
		  },
	.compass = {
		    /*.get_slave_descr = compass_get_slave_descr, */
		    .adapt_num = 2,
		    .bus = EXT_SLAVE_BUS_PRIMARY,
		    .address = (0x50 >> 1),
		    .orientation = {0, 1, 0,
				    1, 0, 0,
				    0, 0, -1},
		    },
};

static struct i2c_board_info __initdata mpu6050_info[] = {
	{
	 I2C_BOARD_INFO("mpu6050", 0x68),
	 /*.irq = */
	 .platform_data = &mpu6050_data,
	 },
};
#endif

static struct bsc_adap_cfg bsc_i2c_cfg[] = {
	{			/* for BSC0 */
	 .speed = BSC_BUS_SPEED_400K,
	 .dynamic_speed = 1,
	 .bsc_clk = "bsc1_clk",
	 .bsc_apb_clk = "bsc1_apb_clk",
	 .retries = 1,
	 .is_pmu_i2c = false,
	 },
	{			/* for BSC1 */
	 .speed = BSC_BUS_SPEED_400K,
	 .dynamic_speed = 1,
	 .bsc_clk = "bsc2_clk",
	 .bsc_apb_clk = "bsc2_apb_clk",
	 .retries = 3,
	 .is_pmu_i2c = false,
	 },
	{			/* for PMU */
#ifdef CONFIG_KONA_PMU_BSC_HS_MODE
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
#else
	 .speed = BSC_BUS_SPEED_50K,
	 .dynamic_speed = 1,
	 .retries = 3,
#endif
	 .bsc_clk = "pmu_bsc_clk",
	 .bsc_apb_clk = "pmu_bsc_apb",
	 .is_pmu_i2c = true,
	 },
};

#ifdef CONFIG_KONA_HEADSET
#define HS_IRQ		gpio_to_irq(71)
#define HSB_IRQ		BCM_INT_ID_AUXMIC_COMP2
#define HSB_REL_IRQ 	BCM_INT_ID_AUXMIC_COMP2_INV
static struct kona_headset_pd headset_data = {
	/* GPIO state read is 0 on HS insert and 1 for
	 * HS remove
	 */

	.hs_default_state = 1,
	/*
	 * Because of the presence of the resistor in the MIC_IN line.
	 * The actual ground is not 0, but a small offset is added to it.
	 * This needs to be subtracted from the measured voltage to determine the
	 * correct value. This will vary for different HW based on the resistor
	 * values used.
	 */
	.phone_ref_offset = 0,
};

static struct resource board_headset_resource[] = {
	{			/* For AUXMIC */
	 .start = AUXMIC_BASE_ADDR,
	 .end = AUXMIC_BASE_ADDR + SZ_4K - 1,
	 .flags = IORESOURCE_MEM,
	 },
	{			/* For ACI */
	 .start = ACI_BASE_ADDR,
	 .end = ACI_BASE_ADDR + SZ_4K - 1,
	 .flags = IORESOURCE_MEM,
	 },
	{			/* For Headset IRQ */
	 .start = HS_IRQ,
	 .end = HS_IRQ,
	 .flags = IORESOURCE_IRQ,
	 },
	{			/* For Headset button  press IRQ */
	 .start = HSB_IRQ,
	 .end = HSB_IRQ,
	 .flags = IORESOURCE_IRQ,
	 },
	{			/* For Headset button  release IRQ */
	 .start = HSB_REL_IRQ,
	 .end = HSB_REL_IRQ,
	 .flags = IORESOURCE_IRQ,
	 },

};

#ifdef CONFIG_SENSORS_KONA
static struct therm_data thermal_pdata = {
	.flags = 0,
	.thermal_update_interval = 0,
	.num_sensors = 4,
	.sensors = sensor_data,
};
#endif

struct platform_device headset_device = {
	.name = "konaaciheadset",
	.id = -1,
	.resource = board_headset_resource,
	.num_resources = ARRAY_SIZE(board_headset_resource),
	.dev = {
		.platform_data = &headset_data,
		},
};
#endif /* CONFIG_KONA_HEADSET */

#ifdef CONFIG_STM_TRACE
static struct stm_platform_data stm_pdata = {
	.regs_phys_base = STM_BASE_ADDR,
	.channels_phys_base = SWSTM_BASE_ADDR,
	.id_mask = 0x0,		/* Skip ID check/match */
	.final_funnel = CHAL_TRACE_FIN_FUNNEL,
};
#endif

static struct spi_kona_platform_data sspi_spi0_info = {
#ifdef CONFIG_DMAC_PL330
	.enable_dma = 1,
#else
	.enable_dma = 0,
#endif
	.cs_line = 1,
	.mode = SPI_LOOP | SPI_MODE_3,
};

static struct spi_kona_platform_data sspi_spi2_info = {
	.enable_dma = 1,
	.cs_line = 1,
	.mode = SPI_LOOP | SPI_MODE_3,
};

#ifdef CONFIG_DMAC_PL330
static struct kona_pl330_data hawaii_pl330_pdata = {
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
		.platform_data = &hawaii_pl330_pdata,
		.coherent_dma_mask = DMA_BIT_MASK(64),
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

#if defined (CONFIG_HAPTIC_SAMSUNG_PWM)
void haptic_gpio_setup(void)
{
	/* Board specific configuration like pin mux & GPIO */
}

static struct haptic_platform_data haptic_control_data = {
	/* Haptic device name: can be device-specific name like ISA1000 */
	.name = "pwm_vibra",
	/* PWM interface name to request */
	.pwm_name = "kona_pwmc:4",
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

static struct resource board_sdio1_resource[] = {
	[0] = {
	       .start = SDIO1_BASE_ADDR,
	       .end = SDIO1_BASE_ADDR + SZ_64K - 1,
	       .flags = IORESOURCE_MEM,
	       },
	[1] = {
	       .start = BCM_INT_ID_SDIO0,
	       .end = BCM_INT_ID_SDIO0,
	       .flags = IORESOURCE_IRQ,
	       },
};

static struct resource board_sdio2_resource[] = {
	[0] = {
	       .start = SDIO2_BASE_ADDR,
	       .end = SDIO2_BASE_ADDR + SZ_64K - 1,
	       .flags = IORESOURCE_MEM,
	       },
	[1] = {
	       .start = BCM_INT_ID_SDIO1,
	       .end = BCM_INT_ID_SDIO1,
	       .flags = IORESOURCE_IRQ,
	       },
};

#ifdef CONFIG_MACH_HAWAII_RAY
static struct resource board_sdio3_resource[] = {
	[0] = {
	       .start = SDIO3_BASE_ADDR,
	       .end = SDIO3_BASE_ADDR + SZ_64K - 1,
	       .flags = IORESOURCE_MEM,
	       },
	[1] = {
	       .start = BCM_INT_ID_SDIO_NAND,
	       .end = BCM_INT_ID_SDIO_NAND,
	       .flags = IORESOURCE_IRQ,
	       },
};
#endif

#ifdef CONFIG_KONA_CPU_FREQ_DRV
struct kona_cpufreq_drv_pdata kona_cpufreq_drv_pdata = {

	.freq_tbl = kona_freq_tbl,
	.num_freqs = ARRAY_SIZE(kona_freq_tbl),
	/*FIX ME: To be changed according to the cpu latency */
	.latency = 10 * 1000,
	.pi_id = PI_MGR_PI_ID_ARM_CORE,
	.cpufreq_init = hawaii_cpufreq_init,
};
#endif

static struct sdio_platform_cfg board_sdio_param[] = {
	{			/* SDIO1 */
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
	 },
	{			/* SDIO2 */
	 .id = 1,
	 .data_pullup = 0,
	 .is_8bit = 1,
	 .devtype = SDIO_DEV_TYPE_EMMC,
	 .flags = KONA_SDIO_FLAGS_DEVICE_NON_REMOVABLE,
	 .peri_clk_name = "sdio2_clk",
	 .ahb_clk_name = "sdio2_ahb_clk",
	 .sleep_clk_name = "sdio2_sleep_clk",
	 .peri_clk_rate = 52000000,
	 },
#ifdef CONFIG_MACH_HAWAII_RAY
	{			/* SDIO3 */
	 .id = 2,
	 .data_pullup = 0,
	 .devtype = SDIO_DEV_TYPE_WIFI,
	 .wifi_gpio = {
		       .reset = 70,
		       .reg = -1,
		       .host_wake = -1,
		       .shutdown = -1,
		       },
	 .flags = KONA_SDIO_FLAGS_DEVICE_NON_REMOVABLE,
	 .peri_clk_name = "sdio3_clk",
	 .ahb_clk_name = "sdio3_ahb_clk",
	 .sleep_clk_name = "sdio3_sleep_clk",
	 .peri_clk_rate = 48000000,
	 },
#endif

#ifdef CONFIG_BCMDHD_43362_LINUX
	{			/* SDIO3 */
	 .id = 2,
	 .data_pullup = 0,
	 .devtype = SDIO_DEV_TYPE_WIFI,
	 .wifi_gpio = {
		       .reset = 70,
		       .reg = -1,
		       .host_wake = 85,
		       .shutdown = -1,
		       },
	 .flags = KONA_SDIO_FLAGS_DEVICE_NON_REMOVABLE,
	 .peri_clk_name = "sdio3_clk",
	 .ahb_clk_name = "sdio3_ahb_clk",
	 .sleep_clk_name = "sdio3_sleep_clk",
	 .peri_clk_rate = 48000000,
	 },
#endif

};

static struct platform_device board_sdio1_device = {
	.name = "sdhci",
	.id = 0,
	.resource = board_sdio1_resource,
	.num_resources = ARRAY_SIZE(board_sdio1_resource),
	.dev = {
		.platform_data = &board_sdio_param[0],
		},
};

static struct platform_device board_sdio2_device = {
	.name = "sdhci",
	.id = 1,
	.resource = board_sdio2_resource,
	.num_resources = ARRAY_SIZE(board_sdio2_resource),
	.dev = {
		.platform_data = &board_sdio_param[1],
		},
};

static struct platform_device board_sdio3_device = {
	.name = "sdhci",
	.id = 2,
#ifdef CONFIG_MACH_HAWAII_RAY
	.resource = board_sdio3_resource,
	.num_resources = ARRAY_SIZE(board_sdio3_resource),
#endif
	.dev = {
		.platform_data = &board_sdio_param[2],
		},
};

/* Common devices among all the boards */
static struct platform_device *board_sdio_plat_devices[] __initdata = {
	&board_sdio2_device,
	&board_sdio3_device,
	&board_sdio1_device,
};

void __init board_add_sdio_devices(void)
{
	platform_add_devices(board_sdio_plat_devices,
			     ARRAY_SIZE(board_sdio_plat_devices));
}

#ifdef CONFIG_BACKLIGHT_PWM

static struct platform_pwm_backlight_data bcm_backlight_data = {
/* backlight */
	.pwm_name = "kona_pwmc:4",
	.max_brightness = 32,	/* Android calibrates to 32 levels */
	.dft_brightness = 32,
#ifdef CONFIG_MACH_HAWAII_RAY
	.polarity = 1,		/* Inverted polarity */
#endif
	.pwm_period_ns = 5000000,
};

static struct platform_device bcm_backlight_devices = {
	.name = "pwm-backlight",
	.id = 0,
	.dev = {
		.platform_data = &bcm_backlight_data,
		},
};

#endif /*CONFIG_BACKLIGHT_PWM */

#ifdef CONFIG_KONA_AVS
static struct kona_avs_pdata avs_pdata = {
	.flags = AVS_TYPE_OPEN | AVS_READ_FROM_MEM,
	.param = 0x3404BFA8,	/*AVS_READ_FROM_MEM - Address
			location where monitor values are copied by ABI */
	.nmos_bin_size = 3,
	.pmos_bin_size = 3,

	.svt_pmos_bin = svt_pmos_bin,
	.svt_nmos_bin = svt_nmos_bin,

	.lvt_pmos_bin = lvt_pmos_bin,
	.lvt_nmos_bin = lvt_nmos_bin,

	.svt_silicon_type_lut = svt_silicon_type_lut,
	.lvt_silicon_type_lut = lvt_silicon_type_lut,


	.silicon_type_notify = avs_silicon_type_notify,
};
#endif

#if defined (CONFIG_REGULATOR_TPS728XX)
#if defined(CONFIG_MACH_HAWAII_RAY)
#define GPIO_SIM2LDO_EN		99
#endif
#ifdef CONFIG_GPIO_PCA953X
#define GPIO_SIM2LDOVSET	(KONA_MAX_GPIO + 7)
#endif
#define TPS728XX_REGL_ID        (BCM59055_MAX_LDO + 0)
struct regulator_consumer_supply sim2_supply[] = {
	{.supply = "sim2_vcc"},
	{.supply = "sim2ldo_uc"},
};

static struct regulator_init_data tps728xx_regl_initdata = {
	.constraints = {
			.name = "sim2ldo",
			.min_uV = 1300000,
			.max_uV = 3300000,
			.valid_ops_mask = REGULATOR_CHANGE_STATUS |
			REGULATOR_CHANGE_VOLTAGE,
			.always_on = 0,
			.boot_on = 0,
			},
	.num_consumer_supplies = ARRAY_SIZE(sim2_supply),
	.consumer_supplies = sim2_supply,
};

struct tps728xx_plat_data tps728xx_data = {
	.gpio_vset = GPIO_SIM2LDOVSET,
	.gpio_en = GPIO_SIM2LDO_EN,
	.vout0 = 1800000,
	.vout1 = 3000000,
	.initdata = &tps728xx_regl_initdata,
};

struct platform_device tps728xx_device = {
	.name = "tps728xx-regulator",
	.id = -1,
	.dev = {
		.platform_data = &tps728xx_data,
		},
};

#ifdef CONFIG_UNICAM
static struct kona_unicam_platform_data unicam_pdata = {
	.csi0_gpio = 12,
	.csi1_gpio = 13,
};
#endif

/* Register userspace and virtual consumer for SIM2LDO */
#ifdef CONFIG_REGULATOR_USERSPACE_CONSUMER
static struct regulator_bulk_data tps728xx_bd_sim2 = {
	.supply = "sim2ldo_uc",
};

static struct regulator_userspace_consumer_data tps728xx_uc_data_sim2 = {
	.name = "sim2ldo",
	.num_supplies = 1,
	.supplies = &tps728xx_bd_sim2,
	.init_on = 0
};

static struct platform_device tps728xx_uc_device_sim2 = {
	.name = "reg-userspace-consumer",
	.id = TPS728XX_REGL_ID,
	.dev = {
		.platform_data = &tps728xx_uc_data_sim2,
		},
};
#endif
#ifdef CONFIG_REGULATOR_VIRTUAL_CONSUMER
static struct platform_device tps728xx_vc_device_sim2 = {
	.name = "reg-virt-consumer",
	.id = TPS728XX_REGL_ID,
	.dev = {
		.platform_data = "sim2ldo_uc"},
};
#endif
#endif /* CONFIG_REGULATOR_TPS728XX */


#if (defined(CONFIG_BCM_RFKILL) || defined(CONFIG_BCM_RFKILL_MODULE))

#define BCMBT_VREG_GPIO       (KONA_MAX_GPIO +4)
#define BCMBT_N_RESET_GPIO    (KONA_MAX_GPIO + 14)
#define BCMBT_AUX0_GPIO        (-1)	/* clk32 */
#define BCMBT_AUX1_GPIO        (-1)	/* UARTB_SEL */

static struct bcmbt_rfkill_platform_data board_bcmbt_rfkill_cfg = {
	.vreg_gpio = BCMBT_VREG_GPIO,
	.n_reset_gpio = BCMBT_N_RESET_GPIO,
	.aux0_gpio = BCMBT_AUX0_GPIO,	/* CLK32 */
	.aux1_gpio = BCMBT_AUX1_GPIO,	/* UARTB_SEL, probably not required */
};

static struct platform_device board_bcmbt_rfkill_device = {
	.name = "bcmbt-rfkill",
	.id = -1,
	.dev = {
		.platform_data = &board_bcmbt_rfkill_cfg,
		},
};
#endif

#ifdef CONFIG_BCM_BZHW
#define GPIO_BT_WAKE 04
#define GPIO_HOST_WAKE 111
static struct bcm_bzhw_platform_data bcm_bzhw_data = {
	.gpio_bt_wake = GPIO_BT_WAKE,
	.gpio_host_wake = GPIO_HOST_WAKE,
};

static struct platform_device board_bcm_bzhw_device = {
	.name = "bcm_bzhw",
	.id = -1,
	.dev = {
		.platform_data = &bcm_bzhw_data,
		},
};
#endif

#ifdef CONFIG_BCM_BT_LPM
#define GPIO_BT_WAKE 04
#define GPIO_HOST_WAKE 111

static struct bcm_bt_lpm_platform_data brcm_bt_lpm_data = {
	.gpio_bt_wake = GPIO_BT_WAKE,
	.gpio_host_wake = GPIO_HOST_WAKE,
};

static struct platform_device board_bcmbt_lpm_device = {
	.name = "bcmbt-lpm",
	.id = -1,
	.dev = {
		.platform_data = &brcm_bt_lpm_data,
		},
};
#endif

#define OV5640_I2C_ADDRESS (0x3C)

static struct i2c_board_info hawaii_i2c_camera[] = {
	{
	 I2C_BOARD_INFO("ov5640", OV5640_I2C_ADDRESS),
	 },
};

static int hawaii_camera_power(struct device *dev, int on)
{
	unsigned int value;
	int ret = -1;
	struct clk *clock;
	struct clk *axi_clk;
	static struct pi_mgr_dfs_node unicam_dfs_node;

	printk(KERN_INFO "%s:camera power %s\n", __func__, (on ? "on" : "off"));

	if (!unicam_dfs_node.valid) {
		ret =
		    pi_mgr_dfs_add_request(&unicam_dfs_node, "unicam",
					   PI_MGR_PI_ID_MM,
					   PI_MGR_DFS_MIN_VALUE);
		if (ret) {
			printk(KERN_ERR
			       "%s: failed to register PI DFS request\n",
			       __func__);
			return -1;
		}
	}

	clock = clk_get(NULL, SENSOR_0_CLK);
	if (!clock) {
		printk(KERN_ERR "%s: unable to get clock %s\n", __func__,
		       SENSOR_0_CLK);
		return -1;
	}

	axi_clk = clk_get(NULL, "csi0_axi_clk");
	if (!axi_clk) {
		printk(KERN_ERR "%s:unable to get clock csi0_axi_clk\n",
		       __func__);
		return -1;
	}

	if (on) {

		if (pi_mgr_dfs_request_update(&unicam_dfs_node, PI_OPP_TURBO)) {
			printk(KERN_ERR
			       "%s:failed to update dfs request for unicam\n",
			       __func__);
			return -1;
		}

		value = clk_enable(axi_clk);
		if (value) {
			printk(KERN_ERR "%s:failed to enable csi2 axi clock\n",
			       __func__);
			return -1;
		}

		/* enable clk */
		value = clk_enable(clock);
		if (value) {
			printk(KERN_ERR "%s: failed to enabled clock %s\n",
			       __func__, SENSOR_0_CLK);
			return -1;
		}
		value = clk_set_rate(clock, SENSOR_0_CLK_FREQ);
		if (value) {
			printk(KERN_ERR
			       "%s: failed to set the clock %s to freq %d\n",
			       __func__, SENSOR_0_CLK, SENSOR_0_CLK_FREQ);
			return -1;
		}

		/* Delay for clk to start */
		msleep(10);

		/* enable reset gpio */
		gpio_set_value(SENSOR_0_GPIO_RST, 0);
		msleep(10);

		/* disable power down gpio */
		gpio_set_value(SENSOR_0_GPIO_PWRDN, 1);
		msleep(5);

		/* disable reset gpio */
		gpio_set_value(SENSOR_0_GPIO_RST, 1);

		/* wait for sensor to come up */
		msleep(30);

	} else {
		/* enable reset gpio */
		gpio_set_value(SENSOR_0_GPIO_RST, 0);
		msleep(1);

		/* enable power down gpio */
		gpio_set_value(SENSOR_0_GPIO_PWRDN, 0);

		clk_disable(clock);

		clk_disable(axi_clk);

		if (pi_mgr_dfs_request_update
		    (&unicam_dfs_node, PI_MGR_DFS_MIN_VALUE)) {
			printk(KERN_ERR
			       "%s: failed to update dfs request for unicam\n",
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

static struct v4l2_subdev_sensor_interface_parms ov5640_if_params = {
	.if_type = V4L2_SUBDEV_SENSOR_SERIAL,
	.if_mode = V4L2_SUBDEV_SENSOR_MODE_SERIAL_CSI2,
	.orientation = V4L2_SUBDEV_SENSOR_PORTRAIT,
	.facing = V4L2_SUBDEV_SENSOR_BACK,
	.parms.serial = {
			 .lanes = 2,
			 .channel = 0,
			 .phy_rate = 0,
			 .pix_clk = 0},
};

static struct soc_camera_link iclink_ov5640 = {
	.bus_id = 0,
	.board_info = &hawaii_i2c_camera[0],
	.i2c_adapter_id = 0,
	.module_name = "ov5640",
	.power = &hawaii_camera_power,
	.reset = &hawaii_camera_reset,
	.priv = &ov5640_if_params,
};

static struct platform_device hawaii_camera = {
	.name = "soc-camera-pdrv",
	.id = 0,
	.dev = {
		.platform_data = &iclink_ov5640,
		},
};

#ifdef CONFIG_WD_TAPPER
static struct wd_tapper_platform_data wd_tapper_data = {
	/* Set the count to the time equivalent to the time-out in milliseconds
	 * required to pet the PMU watchdog to overcome the problem of reset in
	 * suspend*/
	.count = 28000,
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

/* Hawaii Ray specific platform devices */
static struct platform_device *hawaii_ray_plat_devices[] __initdata = {
	&board_serial_device,
	&board_i2c_adap_devices[0],
	&board_i2c_adap_devices[1],
	&board_i2c_adap_devices[2],
	&pmu_device,
	&kona_pwm_device,
/*	&kona_sspi_spi0_device,
	&kona_sspi_spi2_device, */
#ifdef CONFIG_DMAC_PL330
	&pl330_dmac_device,
#endif
#if 0
#ifdef CONFIG_SENSORS_KONA
	&tmon_device,
	&thermal_device,
#endif
#ifdef CONFIG_STM_TRACE
	&kona_stm_device,
#endif
#if defined(CONFIG_HW_RANDOM_KONA)
	&rng_device,
#endif

#ifdef CONFIG_USB_DWC_OTG
	&board_kona_hsotgctrl_platform_device,
	&board_kona_otg_platform_device,
#endif

#ifdef CONFIG_KONA_AVS
	&kona_avs_device,
#endif

#ifdef CONFIG_KONA_CPU_FREQ_DRV
	&kona_cpufreq_device,
#endif

#ifdef CONFIG_CRYPTO_DEV_BRCM_SPUM_HASH
	&board_spum_device,
#endif

#ifdef CONFIG_CRYPTO_DEV_BRCM_SPUM_AES
	&board_spum_aes_device,
#endif

#ifdef CONFIG_UNICAM
	&board_unicam_device,
#endif

#ifdef CONFIG_VIDEO_UNICAM_CAMERA
	&unicam_camera_device,
#endif

#ifdef CONFIG_SND_BCM_SOC
	&caph_i2s_device,
	&caph_pcm_device,
#endif
#ifdef CONFIG_KEYBOARD_BCM
	&bcm_kp_device,
#endif

#ifdef CONFIG_KONA_HEADSET
	&headset_device,
#endif

#ifdef CONFIG_HAPTIC_SAMSUNG_PWM
	&haptic_pwm_device,
#endif
#ifdef CONFIG_BACKLIGHT_PWM
	&bcm_backlight_devices,
#endif
/* TPS728XX device registration */
#ifdef CONFIG_REGULATOR_TPS728XX
	&tps728xx_device,
#endif

#if (defined(CONFIG_BCM_RFKILL) || defined(CONFIG_BCM_RFKILL_MODULE))
	&board_bcmbt_rfkill_device,
#endif

#ifdef CONFIG_BCM_BZHW
	&board_bcm_bzhw_device,
#endif

#ifdef CONFIG_BCM_BT_LPM
	&board_bcmbt_lpm_device,
#endif
	&hawaii_camera,

#ifdef CONFIG_WD_TAPPER
	&wd_tapper,
#endif
#endif
};

/* Add all userspace regulator consumer devices here */
#ifdef CONFIG_REGULATOR_USERSPACE_CONSUMER
struct platform_device *hawaii_ray_userspace_consumer_devices[] __initdata = {
#ifdef CONFIG_REGULATOR_TPS728XX
	&tps728xx_uc_device_sim2,
#endif
};
#endif

/* Add all virtual regulator consumer devices here */
#ifdef CONFIG_REGULATOR_VIRTUAL_CONSUMER
struct platform_device *hawaii_ray_virtual_consumer_devices[] __initdata = {
#ifdef CONFIG_REGULATOR_TPS728XX
	&tps728xx_vc_device_sim2,
#endif
};
#endif

/* Hawaii Ray specific i2c devices */
static void __init hawaii_ray_add_i2c_devices(void)
{
	/* 59055 on BSC - PMU */
#ifdef CONFIG_MFD_BCM_PMU590XX
	i2c_register_board_info(2, pmu_info, ARRAY_SIZE(pmu_info));
#endif

#ifdef CONFIG_GPIO_PCA953X
	i2c_register_board_info(1, pca953x_info, ARRAY_SIZE(pca953x_info));
#endif
#if (defined(CONFIG_MPU_SENSORS_MPU6050A2) || defined(CONFIG_MPU_SENSORS_MPU6050B1))
	i2c_register_board_info(1, mpu6050_info, ARRAY_SIZE(mpu6050_info));
#endif
}

static int __init hawaii_ray_add_lateInit_devices(void)
{
#ifdef CONFIG_BRCM_UNIFIED_DHD_SUPPORT

	printk(KERN_ERR "Calling WLAN_INIT!\n");

	hawaii_wlan_init();
	printk(KERN_ERR "DONE WLAN_INIT!\n");
#endif
	/* TODO: */
/*	board_add_sdio_devices(); */
	return 0;
}

static void __init hawaii_ray_reserve(void)
{
	board_common_reserve();
}

static void hawaii_ray_add_pdata(void)
{
	board_serial_device.dev.platform_data = &uart_data;
	board_i2c_adap_devices[0].dev.platform_data = &bsc_i2c_cfg[0];
	board_i2c_adap_devices[1].dev.platform_data = &bsc_i2c_cfg[1];
	board_i2c_adap_devices[2].dev.platform_data = &bsc_i2c_cfg[2];
	kona_sspi_spi0_device.dev.platform_data = &sspi_spi0_info;
	kona_sspi_spi2_device.dev.platform_data = &sspi_spi2_info;
#ifdef CONFIG_SENSORS_KONA
	thermal_device.dev.platform_data = &thermal_pdata;
#endif
#ifdef CONFIG_STM_TRACE
	kona_stm_device.dev.platform_data = &stm_pdata;
#endif
#ifdef CONFIG_KONA_CPU_FREQ_DRV
	kona_cpufreq_device.dev.platform_data = &kona_cpufreq_drv_pdata;
#endif
#ifdef CONFIG_KONA_AVS
	kona_avs_device.dev.platform_data = &avs_pdata;
#endif
#ifdef CONFIG_UNICAM
	board_unicam_device.dev.platform_data = &unicam_pdata;
#endif
#ifdef CONFIG_KEYBOARD_BCM
	bcm_kp_device.dev.platform_data = &bcm_keypad_data;
#endif
}

/* All Hawaii Ray specific devices */
static void __init hawaii_ray_add_devices(void)
{
	hawaii_ray_add_pdata();
	platform_add_devices(hawaii_ray_plat_devices,
			     ARRAY_SIZE(hawaii_ray_plat_devices));

	/* TODO: */
/*	hawaii_ray_add_i2c_devices(); */
#ifdef CONFIG_REGULATOR_USERSPACE_CONSUMER
	platform_add_devices(hawaii_ray_userspace_consumer_devices,
			     ARRAY_SIZE(hawaii_ray_userspace_consumer_devices));
#endif
#ifdef CONFIG_REGULATOR_VIRTUAL_CONSUMER
	platform_add_devices(hawaii_ray_virtual_consumer_devices,
			     ARRAY_SIZE(hawaii_ray_virtual_consumer_devices));
#endif
	spi_register_board_info(spi_slave_board_info,
				ARRAY_SIZE(spi_slave_board_info));
}

#ifdef CONFIG_FB_BRCM_KONA
/*
 *   	     KONA FRAME BUFFER DISPLAY DRIVER PLATFORM CONFIG
 */
struct kona_fb_platform_data konafb_devices[] __initdata = {
	{
	 .dispdrv_name = "BCM91008_ALEX",
	 .dispdrv_entry = DISP_DRV_BCM91008_ALEX_GetFuncTable,
	 .parms = {
		   .w0 = {
			  .bits = {
				   .boot_mode = 0,
				   .bus_type = HAWAII_BUS_DSI,
				   .bus_no = HAWAII_BUS_0,
				   .bus_ch = HAWAII_BUS_CH_0,
				   .bus_width = 0,
				   .te_input = HAWAII_TE_IN_0_LCD,
				   .col_mode_i = HAWAII_CM_I_XRGB888,
				   .col_mode_o = HAWAII_CM_O_RGB888,
				   },
			  },
		   .w1 = {
			  .bits = {
				   .api_rev = HAWAII_LCD_BOOT_API_REV,
				   .lcd_rst0 = (KONA_MAX_GPIO + 3),
				   },
			  },
		   },
	 },

	{
	 .dispdrv_name = "NT35582_WVGA_SMI",
	 .dispdrv_entry = DISP_DRV_NT35582_WVGA_SMI_GetFuncTable,
	 .parms = {
		   .w0 = {
			  .bits = {
				   .boot_mode = 0,
				   .bus_type = HAWAII_BUS_SMI,
				   .bus_no = HAWAII_BUS_0,
				   .bus_ch = HAWAII_BUS_CH_0,
				   .bus_width = HAWAII_BUS_WIDTH_16,
				   .te_input = HAWAII_TE_IN_0_LCD,
				   .col_mode_i = HAWAII_CM_I_RGB565,
				   .col_mode_o = HAWAII_CM_O_RGB565,
				   },
			  },
		   .w1 = {
			  .bits = {
				   .api_rev = HAWAII_LCD_BOOT_API_REV,
				   .lcd_rst0 = 41,
				   },
			  },
		   },
	 },

	{
	 .dispdrv_name = "NT35582_WVGA_SMI",
	 .dispdrv_entry = DISP_DRV_NT35582_WVGA_SMI_GetFuncTable,
	 .parms = {
		   .w0 = {
			  .bits = {
				   .boot_mode = 0,
				   .bus_type = HAWAII_BUS_SMI,
				   .bus_no = HAWAII_BUS_0,
				   .bus_ch = HAWAII_BUS_CH_0,
				   .bus_width = HAWAII_BUS_WIDTH_08,
				   .te_input = HAWAII_TE_IN_0_LCD,
				   .col_mode_i = HAWAII_CM_I_RGB565,
				   .col_mode_o = HAWAII_CM_O_RGB565,
				   },
			  },
		   .w1 = {
			  .bits = {
				   .api_rev = HAWAII_LCD_BOOT_API_REV,
				   .lcd_rst0 = 41,
				   },
			  },
		   },
	 },

	{
	 .dispdrv_name = "R61581_HVGA_SMI",
	 .dispdrv_entry = DISP_DRV_R61581_HVGA_SMI_GetFuncTable,
	 .parms = {
		   .w0 = {
			  .bits = {
				   .boot_mode = 0,
				   .bus_type = HAWAII_BUS_SMI,
				   .bus_no = HAWAII_BUS_0,
				   .bus_ch = HAWAII_BUS_CH_0,
				   .bus_width = HAWAII_BUS_WIDTH_08,
				   .te_input = HAWAII_TE_IN_0_LCD,
				   .col_mode_i = HAWAII_CM_I_RGB565,
				   .col_mode_o = HAWAII_CM_O_RGB565,
				   },
			  },
		   .w1 = {
			  .bits = {
				   .api_rev = HAWAII_LCD_BOOT_API_REV,
				   .lcd_rst0 = 41,
				   },
			  },
		   },
	 },

	{
	 .dispdrv_name = "R61581_HVGA_SMI",
	 .dispdrv_entry = DISP_DRV_R61581_HVGA_SMI_GetFuncTable,
	 .parms = {
		   .w0 = {
			  .bits = {
				   .boot_mode = 0,
				   .bus_type = HAWAII_BUS_SMI,
				   .bus_no = HAWAII_BUS_0,
				   .bus_ch = HAWAII_BUS_CH_0,
				   .bus_width = HAWAII_BUS_WIDTH_16,
				   .te_input = HAWAII_TE_IN_0_LCD,
				   .col_mode_i = HAWAII_CM_I_RGB565,
				   .col_mode_o = HAWAII_CM_O_RGB565,
				   },
			  },
		   .w1 = {
			  .bits = {
				   .api_rev = HAWAII_LCD_BOOT_API_REV,
				   .lcd_rst0 = 41,
				   },
			  },
		   },
	 },

};

#include "hawaii_fb_init.c"
#endif /* #ifdef CONFIG_FB_BRCM_KONA */

void __init board_init(void)
{
	hawaii_ray_add_devices();
	board_add_common_devices();
#ifdef CONFIG_FB_BRCM_KONA
	/* hawaii_fb_init.c */
	konafb_init();
#endif
	return;
}

void __init board_map_io(void)
{
	/* Map machine specific iodesc here */

	hawaii_map_io();
}

late_initcall(hawaii_ray_add_lateInit_devices);

MACHINE_START(HAWAII, "HawaiiRay")
    .map_io = board_map_io,
    .init_irq = kona_init_irq,
    .timer = &kona_timer,
    .init_machine = board_init,
    .reserve = hawaii_ray_reserve
MACHINE_END
