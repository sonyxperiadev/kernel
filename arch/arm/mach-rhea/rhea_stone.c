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
#include <mach/kona_headset_pd.h>
#include <mach/kona.h>
#include <mach/rhea.h>
#include <mach/pinmux.h>
#include <asm/mach/map.h>
#include <linux/power_supply.h>
#include <linux/mfd/bcm590xx/core.h>
#include <linux/mfd/bcm590xx/pmic.h>
#include <linux/mfd/bcm590xx/bcm59055_A0.h>
#include <linux/broadcom/bcm59055-power.h>
#include <linux/clk.h>
#include <linux/bootmem.h>
#include "common.h"
#include <mach/sdio_platform.h>
#include <linux/i2c/tango_ts.h>
#include <asm/hardware/gic.h>

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

#include <linux/spi/spi.h>
#if defined (CONFIG_HAPTIC)
#include <linux/haptic.h>
#endif

#define _RHEA_
#include <mach/comms/platform_mconfig.h>


#if (defined(CONFIG_BCM_RFKILL) || defined(CONFIG_BCM_RFKILL_MODULE))
#include <linux/broadcom/bcmbt_rfkill.h>
#endif

#ifdef CONFIG_GPS_IRQ
#include <linux/broadcom/gps.h>
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

#if defined(CONFIG_SENSORS_BMA222)
#include <linux/bma222.h>
#endif

#if defined(CONFIG_BMP18X_I2C) || defined(CONFIG_BMP18X_I2C_MODULE)
#include <linux/bmp18x.h>
#include <mach/rheastone/bmp18x_i2c_settings.h>
#endif

#if defined(CONFIG_AL3006) || defined(CONFIG_AL3006_MODULE)
#include <mach/rheastone/al3006_i2c_settings.h>
#endif

#if defined(CONFIG_MPU_SENSORS_MPU6050B1) || defined(CONFIG_MPU_SENSORS_MPU6050B1_MODULE)
#include <linux/mpu.h>
#include <mach/mpu6050_settings.h>
#endif

#ifdef CONFIG_BACKLIGHT_PWM
#include <linux/pwm_backlight.h>
#endif

#ifdef CONFIG_FB_BRCM_KONA
#include <video/kona_fb_boot.h>
#include <video/kona_fb.h>
#endif

#include <plat/pi_mgr.h>

#ifdef CONFIG_VIDEO_UNICAM_CAMERA
#include <media/soc_camera.h>
#endif

#ifdef CONFIG_VIDEO_KONA
#include <media/v4l2-device.h>
#include <media/v4l2-subdev.h>
#include <media/kona-unicam.h>
#include <media/ov5640.h>
#endif

#include <linux/delay.h>
#include <linux/gpio.h>

#ifdef CONFIG_WD_TAPPER
#include <linux/broadcom/wd-tapper.h>
#endif

#ifdef CONFIG_BRCM_UNIFIED_DHD_SUPPORT

#include "board-Rhea_wifi.h"
extern int rhea_wifi_status_register(
		void (*callback)(int card_present, void *dev_id),
		void *dev_id);

#endif




#define PMU_DEVICE_I2C_ADDR_0   0x08
#define PMU_IRQ_PIN           29


#define SD_CARDDET_GPIO_PIN	38

#if defined(CONFIG_TOUCHSCREEN_BCM915500) || defined(CONFIG_TOUCHSCREEN_BCM915500_MODULE)
#include <linux/i2c/bcmtch15xxx.h>
#define HW_BCM915500_GPIO_RESET        70
#define HW_BCM915500_GPIO_INTERRUPT    71
#define HW_BCM915500_I2C_BUS_ID        1
#endif

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


static int configure_sdio_pullup(bool pull_up);
/*
 * GPIO pin for Touch screen pen down interrupt
 */
#define TANGO_GPIO_IRQ_PIN			71
#define TANGO_GPIO_RESET_PIN			70
#define TANGO_I2C_TS_DRIVER_NUM_BYTES_TO_READ 	14

#if (defined(CONFIG_MFD_BCM59039) || defined(CONFIG_MFD_BCM59042) || \
	 defined(CONFIG_MFD_BCM_PMU59xxx))
struct regulator_consumer_supply hv6_supply[] = {
	{.supply = "vdd_sdxc"},
	{.supply = "sddat_debug_bus"},
};

struct regulator_consumer_supply hv3_supply[] = {
	{.supply = "hv3"},
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

static struct bcm_keymap newKeymap[] = {
	{BCM_KEY_ROW_0, BCM_KEY_COL_0, "Vol Down Key", 	KEY_VOLUMEDOWN},
	{BCM_KEY_ROW_0, BCM_KEY_COL_1, "Vol Up Key", 	KEY_VOLUMEUP},
	{BCM_KEY_ROW_0, BCM_KEY_COL_2, "Search Key", 	KEY_SEARCH},
	{BCM_KEY_ROW_0, BCM_KEY_COL_3, "unused", 0},
	{BCM_KEY_ROW_0, BCM_KEY_COL_4, "unused", 0},
	{BCM_KEY_ROW_0, BCM_KEY_COL_5, "unused", 0},
	{BCM_KEY_ROW_0, BCM_KEY_COL_6, "unused", 0},
	{BCM_KEY_ROW_0, BCM_KEY_COL_7, "unused", 0},

	{BCM_KEY_ROW_1, BCM_KEY_COL_0, "Home-Key", 	KEY_HOME},
	{BCM_KEY_ROW_1, BCM_KEY_COL_1, "Back Key", 	KEY_BACK},
	{BCM_KEY_ROW_1, BCM_KEY_COL_2, "Menu-Key", 	KEY_MENU},
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
	.row_num = 2,
	.col_num = 3,
	.keymap = newKeymap,
	.bcm_keypad_base = (void *)__iomem HW_IO_PHYS_TO_VIRT(KEYPAD_BASE_ADDR),
};

#endif
/* Regulator consumers differ across the platforms.
 * The following definitions will override the weak
 * consumer definitions in PMIC board files */
#ifdef CONFIG_MFD_BCMPMU

struct regulator_consumer_supply hv10_supply[] = {
	{ .supply = "sim2_vcc"},
};

#endif



#if defined(CONFIG_BCMI2CNFC)

static int bcmi2cnfc_gpio_setup(void *);
static int bcmi2cnfc_gpio_clear(void *);
static struct bcmi2cnfc_i2c_platform_data bcmi2cnfc_pdata = {
	.i2c_pdata	= {ADD_I2C_SLAVE_SPEED(BSC_BUS_SPEED_400K),
		SET_CLIENT_FUNC(TX_FIFO_ENABLE | RX_FIFO_ENABLE)},
	.irq_gpio = 4,
	.en_gpio = 100,
	.wake_gpio = 73,
	.init = bcmi2cnfc_gpio_setup,
	.reset = bcmi2cnfc_gpio_clear,
};


static int bcmi2cnfc_gpio_setup(void *this)
{
/*
	struct bcmi2cnfc_i2c_platform_data *p;
	p = (struct bcmi2cnfc_i2c_platform_data *) this;
	if (!p)
		return -1;
	pr_info("bcmi2cnfc_gpio_setup nfc en %d, wake %d, irq %d\n",
		p->en_gpio, p->wake_gpio, p->irq_gpio);

	gpio_request(p->irq_gpio, "nfc_irq");
	gpio_direction_input(p->irq_gpio);

	gpio_request(p->en_gpio, "nfc_en");
	gpio_direction_output(p->en_gpio, 1);

	gpio_request(p->wake_gpio, "nfc_wake");
	gpio_direction_output(p->wake_gpio, 0);
*/
	return 0;
}
static int bcmi2cnfc_gpio_clear(void *this)
{
/*
	struct bcmi2cnfc_i2c_platform_data *p;
	p = (struct bcmi2cnfc_i2c_platform_data *) this;
	if (!p)
		return -1;

	pr_info("bcmi2cnfc_gpio_clear nfc en %d, wake %d, irq %d\n",
		p->en_gpio, p->wake_gpio, p->irq_gpio);

	gpio_direction_output(p->en_gpio, 0);
	gpio_direction_output(p->wake_gpio, 1);
	gpio_free(p->en_gpio);
	gpio_free(p->wake_gpio);
	gpio_free(p->irq_gpio);
*/
	return 0;
}

static struct i2c_board_info __initdata bcmi2cnfc[] = {
	{
	 I2C_BOARD_INFO("bcmi2cnfc", 0x1F0),
	 .flags = I2C_CLIENT_TEN,
	 .platform_data = (void *)&bcmi2cnfc_pdata,
	 .irq = gpio_to_irq(4),
	 },

};
#endif

#if defined(CONFIG_BMP18X_I2C) || defined(CONFIG_BMP18X_I2C_MODULE)
static struct i2c_board_info __initdata i2c_bmp18x_info[] = {
	{
		I2C_BOARD_INFO(BMP18X_NAME, BMP18X_I2C_ADDRESS),
	},
};
#endif

#if defined(CONFIG_AL3006) || defined(CONFIG_AL3006_MODULE)
static struct i2c_board_info __initdata i2c_al3006_info[] = {
	{
		I2C_BOARD_INFO("al3006", AL3006_I2C_ADDRESS),
	},
};
#endif

#if defined(CONFIG_SENSORS_BMA222)
static struct bma222_accl_platform_data bma_pdata = {
	.orientation = BMA_ROT_90,
	.invert = false,
};
#endif

#if defined(CONFIG_MPU_SENSORS_MPU6050B1) || defined(CONFIG_MPU_SENSORS_MPU6050B1_MODULE)

static struct mpu_platform_data mpu6050_platform_data =
{
	.int_config  = MPU6050_INIT_CFG,
	.level_shifter = 0,
	.orientation = MPU6050_DRIVER_ACCEL_GYRO_ORIENTATION,
};

static struct ext_slave_platform_data mpu_compass_data =
{
	.bus = EXT_SLAVE_BUS_SECONDARY,
	.orientation = MPU6050_DRIVER_COMPASS_ORIENTATION,
};


static struct i2c_board_info __initdata inv_mpu_i2c0_boardinfo[] =
{
	{
		I2C_BOARD_INFO("mpu6050", MPU6050_SLAVE_ADDR),
		.platform_data = &mpu6050_platform_data,
	},
	{
		I2C_BOARD_INFO("ami306", MPU6050_COMPASS_SLAVE_ADDR),
		.platform_data = &mpu_compass_data,
	},
};
#endif /* CONFIG_MPU_SENSORS_MPU6050B1 */

#ifdef CONFIG_KONA_HEADSET_MULTI_BUTTON

#define HS_IRQ		gpio_to_irq(39)
#define HSB_IRQ		BCM_INT_ID_AUXMIC_COMP2
#define HSB_REL_IRQ 	BCM_INT_ID_AUXMIC_COMP2_INV
static unsigned int rheass_button_adc_values[3][2] = {
	/* SEND/END Min, Max*/
	{0,	10},
	/* Volume Up  Min, Max*/
	{11, 30},
	/* Volue Down Min, Max*/
	{30, 680},
};

static unsigned int rheass_button_adc_values_2_1[3][2] = {
	/* SEND/END Min, Max*/
	{0,     104},
	/* Volume Up  Min, Max*/
	{139,   270},
	/* Volue Down Min, Max*/
	{330,   680},
};
static struct kona_headset_pd headset_data = {
	/* GPIO state read is 0 on HS insert and 1 for
	 * HS remove
	 */

	.hs_default_state = 0,
	/*
	 * Because of the presence of the resistor in the MIC_IN line.
	 * The actual ground is not 0, but a small offset is added to it.
	 * This needs to be subtracted from the measured voltage to determine the
	 * correct value. This will vary for different HW based on the resistor
	 * values used.
	 *
	 * What this means to Rhearay?
	 * From the schematics looks like there is no such resistor put on
	 * Rhearay. That means technically there is no need to subtract any extra load
	 * from the read Voltages. On other HW, if there is a resistor present
	 * on this line, please measure the load value and put it here.
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
	.button_adc_values_low = rheass_button_adc_values,

	/*
	 * Pass the board specific button detection range
	 */
	.button_adc_values_high = rheass_button_adc_values_2_1,

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
	{	/* For Headset button  press IRQ */
		.start = HSB_IRQ,
		.end = HSB_IRQ,
		.flags = IORESOURCE_IRQ,
	},
	{	/* For Headset button  release IRQ */
		.start = HSB_REL_IRQ,
		.end = HSB_REL_IRQ,
		.flags = IORESOURCE_IRQ,
	},
		/* For backward compatibility keep COMP1
		 * as the last resource. The driver which
		 * uses only GPIO and COMP2, might not use this at all
		 */
	{	/* COMP1 for type detection */
		.start = BCM_INT_ID_AUXMIC_COMP1,
		.end = HSB_REL_IRQ,
		.flags = IORESOURCE_IRQ,
	},
};

struct platform_device headset_device = {
	.name = "konaaciheadset",
	.id = -1,
	.resource = board_headset_resource,
	.num_resources	= ARRAY_SIZE(board_headset_resource),
	.dev	=	{
		.platform_data = &headset_data,
	},
};
#endif /* CONFIG_KONA_HEADSET_MULTI_BUTTON */

#ifdef CONFIG_DMAC_PL330
static struct kona_pl330_data rhea_pl330_pdata =	{
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

static struct platform_device pl330_dmac_device = {
	.name = "kona-dmac-pl330",
	.id = 0,
	.dev = {
		.platform_data = &rhea_pl330_pdata,
		.coherent_dma_mask  = DMA_BIT_MASK(64),
	},
};
#endif

#if (defined(CONFIG_BCM_RFKILL) || defined(CONFIG_BCM_RFKILL_MODULE))
#define BCMBT_VREG_GPIO       (41)
#define BCMBT_N_RESET_GPIO    (40)
#define BCMBT_AUX0_GPIO        (-1)   /* clk32 */
#define BCMBT_AUX1_GPIO        (-1)    /* UARTB_SEL */

static struct bcmbt_rfkill_platform_data board_bcmbt_rfkill_cfg = {
        .vreg_gpio = BCMBT_VREG_GPIO,
        .n_reset_gpio = BCMBT_N_RESET_GPIO,
        .aux0_gpio = BCMBT_AUX0_GPIO,  /* CLK32 */
        .aux1_gpio = BCMBT_AUX1_GPIO,  /* UARTB_SEL, probably not required */
};

static struct platform_device board_bcmbt_rfkill_device = {
        .name = "bcmbt-rfkill",
        .id = -1,
        .dev =
        {
                .platform_data=&board_bcmbt_rfkill_cfg,
        },
};
#endif

#ifdef CONFIG_BCM_BZHW
#define GPIO_BT_WAKE 27
#define GPIO_HOST_WAKE 72
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
#define GPIO_BT_WAKE	27
#define GPIO_HOST_WAKE 	72

static struct bcm_bt_lpm_platform_data brcm_bt_lpm_data = {
        .gpio_bt_wake = GPIO_BT_WAKE,
        .gpio_host_wake = GPIO_HOST_WAKE,
};

static struct platform_device board_bcmbt_lpm_device = {
        .name = "bcmbt-lpm",
        .id = -1,
        .dev =
        {
                .platform_data=&brcm_bt_lpm_data,
        },
};
#endif



#ifdef CONFIG_GPS_IRQ

#define GPIO_GPS_HOST_WAKE 88

static struct gps_platform_data gps_hostwake_data= {
        .gpio_interrupt = GPIO_GPS_HOST_WAKE,
	    .i2c_pdata	= {ADD_I2C_SLAVE_SPEED(BSC_BUS_SPEED_400K),},
};

static struct i2c_board_info __initdata gpsi2c[] = {
	{
	 I2C_BOARD_INFO("gpsi2c", 0x1FA),
	 .flags = I2C_CLIENT_TEN,
	 .platform_data = (void *)&gps_hostwake_data,
	 .irq = gpio_to_irq(88),
	 },

};

static struct platform_device gps_hostwake= {
        .name = "gps-hostwake",
        .id = -1,
        .dev =
        {
                .platform_data=&gps_hostwake_data,
        },
};
#endif

#if defined(CONFIG_SPI_GPIO)
/*
 * SPI-BitBang For Sharp LCD
 */

#define SPI_BB_MISO	(92)
#define SPI_BB_MOSI	(91)
#define SPI_BB_SCL	(90)
#define SPI_BB_CS       (89)
#define SPI_BB_BUS_NUM	(3)


static struct spi_gpio_platform_data spi_gpio_pdata = {
	.sck		= SPI_BB_SCL,
	.mosi		= SPI_BB_MOSI,
	.miso		= SPI_BB_MISO,
	.num_chipselect	= 1,
};

static struct platform_device spi_gpio = {
	.name		= "spi_gpio",
	.id		= SPI_BB_BUS_NUM,
	.dev		= {
		.platform_data	= &spi_gpio_pdata,
	},
};

static struct spi_board_info lq043y1dx01_spi_devices[] __initdata = {
	{
		.modalias		= "lq043y1dx01_spi",
		.max_speed_hz		= 1000000,
		.bus_num		= SPI_BB_BUS_NUM,
		.chip_select		= 0,
		.controller_data	= (void *)SPI_BB_CS,
	},
};

static void __init rheastone_add_lcd_spi(void)
{
	spi_register_board_info(lq043y1dx01_spi_devices,
					ARRAY_SIZE(lq043y1dx01_spi_devices));
	platform_device_register(&spi_gpio);
}
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
	.pwm_id = 4,
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

void dump_rhea_pin_config(struct pin_config *debug_pin_config)
{

	pr_debug("drv_sth:%d, input_dis:%d, slew_rate_ctrl:%d,"
		   "pull_up:%d, pull_dn:%d, hys_en:%d, sel:%d\n",
		debug_pin_config->reg.b.drv_sth,
		debug_pin_config->reg.b.input_dis,
		debug_pin_config->reg.b.slew_rate_ctrl,
		debug_pin_config->reg.b.pull_up,
		debug_pin_config->reg.b.pull_dn,
		debug_pin_config->reg.b.hys_en,
		debug_pin_config->reg.b.sel);

}

int configure_sdio_pullup(bool pull_up)
{
	int ret = 0;
	char i;
	struct pin_config new_pin_config;

	if (pull_up)
		pr_debug("%s, Pull-up enable for SD card pins!\n", __func__);
	else
		pr_debug("%s, Pull-down enable for SD card pins!\n", __func__);

	new_pin_config.name = PN_SDCMD;

	ret = pinmux_get_pin_config(&new_pin_config);
	if (ret)	{
		pr_err("%s, Error pinmux_get_pin_config!%d\n", __func__, ret);
		return ret;
	}

	pr_debug("%s: Old SDCMD pin settings\n", __func__);
	dump_rhea_pin_config(&new_pin_config);

	if (pull_up)	{
		new_pin_config.reg.b.pull_up = PULL_UP_ON;
		new_pin_config.reg.b.pull_dn = PULL_DN_OFF;
	} else {
		new_pin_config.reg.b.pull_up = PULL_UP_OFF;
		new_pin_config.reg.b.pull_dn = PULL_DN_ON;
	}

	ret = pinmux_set_pin_config(&new_pin_config);
	if (ret)	{
		pr_err("%s Failed to configure SDCMD:%d\n", __func__, ret);
		return ret;
	}

	ret = pinmux_get_pin_config(&new_pin_config);
	if (ret)	{
		pr_err("%s, Error pinmux_get_pin_config!%d\n", __func__, ret);
		return ret;
	}
	pr_debug("%s: New SDCMD pin settings\n", __func__);
	dump_rhea_pin_config(&new_pin_config);

	for (i = 0; i < 4; i++)	{
		new_pin_config.name = (PN_SDDAT0 + i);
		ret = pinmux_get_pin_config(&new_pin_config);
		if (ret)	{
			pr_info("%s, Error pinmux_get_pin_config():%d\n",
				__func__, ret);
			return ret;
		}
		pr_debug("%s: Old SDDAT%d pin setting\n", __func__, i);
		dump_rhea_pin_config(&new_pin_config);
		if (pull_up)	{
			new_pin_config.reg.b.pull_up = PULL_UP_ON;
			new_pin_config.reg.b.pull_dn = PULL_DN_OFF;
		} else	{
			new_pin_config.reg.b.pull_up = PULL_UP_OFF;
			new_pin_config.reg.b.pull_dn = PULL_DN_ON;
		}

		ret = pinmux_set_pin_config(&new_pin_config);
		if (ret)	{
			pr_err("%s: Failed to configure SDDAT%d:%d\n",
				__func__, i, ret);
			return ret;
		}

		ret = pinmux_get_pin_config(&new_pin_config);
		if (ret)	{
			pr_err("%s, Error pinmux_get_pin_config!%d\n",
				__func__, ret);
			dump_rhea_pin_config(&new_pin_config);
			return ret;
		}
		pr_debug("%s: New SDDAT%d pin setting\n", __func__, i);
		dump_rhea_pin_config(&new_pin_config);
	}

	return ret;
}

static struct resource board_sdio0_resource[] = {
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

static struct resource board_sdio1_resource[] = {
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

static struct resource board_sdio2_resource[] = {
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
static struct sdio_platform_cfg board_sdio_param[] = {
	{ /* SDIO0 */
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
	{ /* SDIO1 */
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

		{ /* SDIO4 */
			.id = 2,
			.data_pullup = 0,
			.devtype = SDIO_DEV_TYPE_WIFI,
 			.flags = KONA_SDIO_FLAGS_DEVICE_REMOVABLE,
			.peri_clk_name = "sdio3_clk",
			.ahb_clk_name = "sdio3_ahb_clk",
			.sleep_clk_name = "sdio3_sleep_clk",
			.peri_clk_rate = 48000000,
			.register_status_notify = rhea_wifi_status_register,
		},


#else
		{ /* SDIO4 */
			.id = 2,
			.data_pullup = 0,
			.devtype = SDIO_DEV_TYPE_WIFI,
			.wifi_gpio = {
				.reset		= 42,
				.reg		= -1,
				.host_wake	= 74,
				.shutdown	= -1,
			},
			.flags = KONA_SDIO_FLAGS_DEVICE_REMOVABLE,
			.peri_clk_name = "sdio3_clk",
			.ahb_clk_name = "sdio3_ahb_clk",
			.sleep_clk_name = "sdio3_sleep_clk",
			.peri_clk_rate = 48000000,
		},


#endif





#if 0

	{ /* SDIO2 */
		.id = 2,
		.data_pullup = 0,
		.devtype = SDIO_DEV_TYPE_WIFI,
		.wifi_gpio = {
			.reset		= 42,
			.reg		= -1,
			.host_wake	= 74,
			.shutdown	= -1,
		},
		.flags = KONA_SDIO_FLAGS_DEVICE_NON_REMOVABLE,
		.peri_clk_name = "sdio3_clk",
		.ahb_clk_name = "sdio3_ahb_clk",
		.sleep_clk_name = "sdio3_sleep_clk",
		.peri_clk_rate = 48000000,
	},
#endif

};

static struct platform_device board_sdio0_device = {
	.name = "sdhci",
	.id = 0,
	.resource = board_sdio0_resource,
	.num_resources   = ARRAY_SIZE(board_sdio0_resource),
	.dev      = {
		.platform_data = &board_sdio_param[0],
	},
};

static struct platform_device board_sdio1_device = {
	.name = "sdhci",
	.id = 1,
	.resource = board_sdio1_resource,
	.num_resources   = ARRAY_SIZE(board_sdio1_resource),
	.dev      = {
		.platform_data = &board_sdio_param[1],
	},
};

static struct platform_device board_sdio2_device = {
	.name = "sdhci",
	.id = 2,
	.resource = board_sdio2_resource,
	.num_resources   = ARRAY_SIZE(board_sdio2_resource),
	.dev      = {
		.platform_data = &board_sdio_param[2],
	},
};


/* Common devices among all the Rhea boards (Rhea Ray, Rhea Berri, etc.) */
static struct platform_device *board_sdio_plat_devices[] __initdata = {
	&board_sdio1_device,
	&board_sdio2_device,
	&board_sdio0_device,
};

void __init board_add_sdio_devices(void)
{
	platform_add_devices(board_sdio_plat_devices, ARRAY_SIZE(board_sdio_plat_devices));
}

#ifdef CONFIG_BACKLIGHT_PWM

static struct platform_pwm_backlight_data bcm_backlight_data = {
/* backlight */
	.pwm_id 	= 4,
	.max_brightness = 32,   /* Android calibrates to 32 levels*/
	.dft_brightness = 32,
	.polarity       = 1,    /* Inverted polarity */
	.pwm_period_ns 	=  5000000,
};

static struct platform_device bcm_backlight_devices = {
	.name 	= "pwm-backlight",
	.id 	= 0,
	.dev 	= {
		.platform_data  =       &bcm_backlight_data,
	},
};

#endif /*CONFIG_BACKLIGHT_PWM */

#ifdef CONFIG_FB_BRCM_KONA

#if 0
static struct kona_fb_platform_data lq043y1dx01_dsi_display_fb_data = {
	.get_dispdrv_func_tbl	= &DISP_DRV_LQ043Y1DX01_GetFuncTable,
	.screen_width		= 480,
	.screen_height		= 800,
	.bytes_per_pixel	= 2,
	.gpio			= 0,
	.pixel_format		= RGB565,
};

static struct platform_device lq043y1dx01_dsi_display_device = {
	.name    = "rhea_fb",
	.id      = 0,
	.dev = {
		.platform_data		= &lq043y1dx01__display_fb_data,
		.dma_mask		= (u64 *) ~(u32)0,
		.coherent_dma_mask	= ~(u32)0,
	},
};
#endif

#endif

/*
 * Camera Specifc
 */
#define SENSOR_0_GPIO_PWRDN		(121)
#define SENSOR_0_GPIO_RST		(111)
#define SENSOR_0_CLK			"dig_ch0_clk"
#define SENSOR_0_CLK_FREQ		(13000000)

#define SENSOR_1_GPIO_PWRDN            (122)
#define SENSOR_1_GPIO_RST              (99)
#define SENSOR_1_CLK                   "dig_ch1_clk"
#define SENSOR_1_CLK_FREQ              (13000000)

#define TCM9001_I2C_ADDRESS (0x7C)
#define OV5640_I2C_ADDRESS (0x3C)

#ifdef CONFIG_VIDEO_UNICAM_CAMERA
static struct i2c_board_info rhea_i2c_camera[] = {
	{
	 I2C_BOARD_INFO("ov5640", OV5640_I2C_ADDRESS),
	 },
	{
	I2C_BOARD_INFO("tcm9001", (TCM9001_I2C_ADDRESS >> 1)),
	},

};

static int rhea_camera_power(struct device *dev, int on)
{
	unsigned int value;
	int ret = -1;
	struct clk *clock;
	struct clk *axi_clk;
	static struct pi_mgr_dfs_node unicam_dfs_node;
	static int do_cam_reset = 1;

	printk(KERN_INFO "%s:camera power %s\n", __func__, (on ? "on" : "off"));

	if (!unicam_dfs_node.valid) {
		ret =
		    pi_mgr_dfs_add_request(&unicam_dfs_node,"unicam", PI_MGR_PI_ID_MM,
					   PI_MGR_DFS_MIN_VALUE);
		if (ret) {
			printk(KERN_ERR
			       "%s: failed to register PI DFS request\n",
			       __func__);
			return -1;
		}
		if (gpio_request_one
		    (SENSOR_0_GPIO_RST, GPIOF_DIR_OUT | GPIOF_INIT_LOW,
		     "CamRst")) {
			printk(KERN_ERR "%s: failed to get (CamRst) gpio\n",
			       __func__);
			return -1;
		}
		if (gpio_request_one(SENSOR_0_GPIO_PWRDN, GPIOF_DIR_OUT |
				     GPIOF_INIT_HIGH, "CamPwr")) {
			printk(KERN_ERR "%s: failed to get (CamPwr) gpio\n",
			       __func__);
			return -1;
		}
	}

	clock = clk_get(NULL, SENSOR_0_CLK);
	if (IS_ERR_OR_NULL(clock)) {
		printk(KERN_ERR "%s: unable to get clock %s\n", __func__,
		       SENSOR_0_CLK);
		return -1;
	}

	axi_clk = clk_get(NULL, "csi0_axi_clk");
	if (IS_ERR_OR_NULL(axi_clk)) {
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
		if (do_cam_reset) {
			gpio_set_value(SENSOR_0_GPIO_RST, 0);
			msleep(10);
		}

		/* disable power down gpio */
		gpio_set_value(SENSOR_0_GPIO_PWRDN, 0);
		msleep(5);

		/* disable reset gpio */
		if (do_cam_reset) {
			gpio_set_value(SENSOR_0_GPIO_RST, 1);
			do_cam_reset = 0;
		}

		/* wait for sensor to come up */
		msleep(30);

	} else {
		/* enable reset gpio */
	//	gpio_set_value(SENSOR_0_GPIO_RST, 0);
	//	msleep(1);

		/* enable power down gpio */
		gpio_set_value(SENSOR_0_GPIO_PWRDN, 1);

		clk_disable(clock);

		clk_disable(axi_clk);

		if (pi_mgr_dfs_request_update(&unicam_dfs_node,
					      PI_MGR_DFS_MIN_VALUE)) {
			printk(KERN_ERR "%s: failed to update dfs request for unicam\n",
				 __func__);
		}
	}

	return 0;
}

static int rhea_camera_reset(struct device *dev)
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
		.pix_clk = 0
	},
};

static struct soc_camera_link iclink_ov5640 = {
	.bus_id = 0,
	.board_info = &rhea_i2c_camera[0],
	.i2c_adapter_id = 0,
	.module_name = "ov5640",
	.power = &rhea_camera_power,
	.reset = &rhea_camera_reset,
	.priv =  &ov5640_if_params,
};

static struct platform_device rhea_camera_back = {
	.name = "soc-camera-pdrv",
	.id = 0,
	.dev = {
		.platform_data = &iclink_ov5640,
		},
};

static int rhea_camera_power_front(struct device *dev, int on)
{
	unsigned int value;
	int ret = -1;
	struct clk *clock;
	struct clk *axi_clk;
	static struct pi_mgr_dfs_node unicam_dfs_node;
	printk(KERN_INFO "Rhea stone front camera power start %d\n", on);
	if (!unicam_dfs_node.valid) {
		ret =
			pi_mgr_dfs_add_request(&unicam_dfs_node,
				"unicam", PI_MGR_PI_ID_MM,
				PI_MGR_DFS_MIN_VALUE);
		if (ret) {
			printk(KERN_INFO "PI DFS for front cl am\n");
			return -1;
		}
		if (gpio_request_one
			(SENSOR_1_GPIO_RST, GPIOF_DIR_OUT | GPIOF_INIT_LOW,
				"Cam1Rst")) {
			printk(KERN_ERR "SENSOR_1_GPIO_RST failed\n");
			return -1;
		}
		if (gpio_request_one(SENSOR_1_GPIO_PWRDN, GPIOF_DIR_OUT |
			GPIOF_INIT_LOW, "Cam1Pwr")) {
			printk(KERN_ERR "SENSOR_1_GPIO_PWDN failed\n");
			return -1;
		}
	}
	/* Power on sequence start */
	clock = clk_get(NULL, SENSOR_1_CLK);
	if (IS_ERR_OR_NULL(clock)) {
		printk(KERN_ERR "SENSOR_1_CLK get failed\n");
		return -1;
	} else {
		printk(KERN_INFO "Got clock %s\n", SENSOR_1_CLK);
	}
	axi_clk = clk_get(NULL, "csi0_axi_clk");
	if (IS_ERR_OR_NULL(axi_clk)) {
		printk(KERN_ERR "unable to get clock csi0_axi_clk\n");
		return -1;
	}
	if (on) {
		if (pi_mgr_dfs_request_update(&unicam_dfs_node, PI_OPP_TURBO)) {
			printk(KERN_ERR "Unicam dfs update failed\n");
			return -1;
		}
		value = clk_enable(axi_clk);
		if (value) {
			printk(KERN_ERR "AXI clock enable failed\n");
			return -1;
		}
		clk_disable(clock);
		/* Actual power up sequence starts here */
		usleep_range(1000, 2000);
		gpio_set_value(SENSOR_1_GPIO_RST, 0);
		printk(KERN_INFO "PWDN set to HIGH\n");
		value = 0;
		value = clk_set_rate(clock, SENSOR_1_CLK_FREQ);
		if (value)
			printk(KERN_INFO "front cam rate fail %d\n", value);
		value = clk_enable(clock);
		if (value)
			printk(KERN_ERR "enabling clock for front cam fail\n");
		else
			printk(KERN_INFO "Enabled clock for front camera\n");
		usleep_range(5000, 10000);
		gpio_set_value(SENSOR_1_GPIO_PWRDN, 0);
		/* clk_set_rate returns the clock in the
		same state it was in before calling */
		/* So enable the clock now */
		usleep_range(5000, 10000);
		gpio_set_value(SENSOR_1_GPIO_RST, 1);
	} else {
		gpio_set_value(SENSOR_1_GPIO_RST, 1);
		gpio_set_value(SENSOR_1_GPIO_PWRDN, 1);
		clk_disable(clock);
		clk_disable(axi_clk);
		if (pi_mgr_dfs_request_update(&unicam_dfs_node,
			PI_MGR_DFS_MIN_VALUE)) {
			printk(KERN_ERR "Unicam dfs update failed\n");
		}
	}
	return 0;
}

static int rhea_camera_reset_front(struct device *dev)
{
	/* reset the camera gpio */
	printk(KERN_INFO "%s:Front camera reset\n", __func__);
	return 0;
}
static struct v4l2_subdev_sensor_interface_parms tcm9001_if_params = {
	.if_type = V4L2_SUBDEV_SENSOR_SERIAL,
	.if_mode = V4L2_SUBDEV_SENSOR_MODE_SERIAL_CSI1,
	.orientation = V4L2_SUBDEV_SENSOR_PORTRAIT,
	.facing = V4L2_SUBDEV_SENSOR_FRONT,
	.parms.serial = {
		.lanes = 1,
		.channel = 1,
		/* maps to CSI2 AFE 1, 0 for rear camera */
		.phy_rate = 0,
		.pix_clk = 0
	},
};

static struct soc_camera_link iclink_tcm9001 = {
	.bus_id = 0,
	.board_info = &rhea_i2c_camera[1],
	.i2c_adapter_id = 0,
	.module_name = "tcm9001",
	.power = &rhea_camera_power_front,
	.reset = &rhea_camera_reset_front,
	.priv =  &tcm9001_if_params,
};

static struct platform_device rhea_camera_front = {
	.name = "soc-camera-pdrv",
	.id = 1,
	.dev = {
		.platform_data = &iclink_tcm9001,
		},
};
#endif

#ifdef CONFIG_VIDEO_KONA
static struct clk *dig_ch0_clk;

static int rhea_stone_ov_cam1_power(struct v4l2_subdev *subdev, int on)
{
	struct device *dev = subdev->v4l2_dev->dev;
	int ret;

	if (on) {
		/* reset gpio */
		gpio_set_value(SENSOR_0_GPIO_RST, 0);
		usleep_range(10000, 20000);
		/* disable power down gpio */
		gpio_set_value(SENSOR_0_GPIO_PWRDN, 0);
		usleep_range(5000, 10000);
		/* disable reset gpio */
		gpio_set_value(SENSOR_0_GPIO_RST, 1);
		/* wait for sensor to come up */
		msleep(30);
		/* enable clk*/
		ret = clk_enable(dig_ch0_clk);
		if (ret) {
			dev_err(dev, "error in clk_enable dig_ch0_clk\n");
			return -EINVAL;
		}
		/* delay to start the clk */
		usleep_range(10000, 20000);

	} else {
		gpio_set_value(SENSOR_0_GPIO_RST, 0);
		usleep_range(1000, 2000);
		/* power down gpio */
		gpio_set_value(SENSOR_0_GPIO_PWRDN, 1);

		clk_disable(dig_ch0_clk);
	}

	return 0;
}

static struct ov5640_platform_data ov5640_cam1_pdata = {
	.s_power = rhea_stone_ov_cam1_power,
};

struct unicam_subdev_i2c_board_info ov5640_cam1_i2c_device = {
	.board_info = {
		I2C_BOARD_INFO("ov5640-mc", OV5640_I2C_ADDRESS),
		.platform_data = &ov5640_cam1_pdata,
	},
	.i2c_adapter_id = 0,
};

static struct unicam_v4l2_subdevs_groups rhea_stone_unicam_subdevs[] = {
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

static struct unicam_platform_data rhea_stone_unicam_pdata = {
	.subdevs = rhea_stone_unicam_subdevs,
	.num_subdevs = ARRAY_SIZE(rhea_stone_unicam_subdevs),
};

static struct resource rhea_stone_unicam_rsrc[] = {
	[0] = {
		.start = BCM_INT_ID_CSI,
		.end = BCM_INT_ID_CSI,
		.flags = IORESOURCE_IRQ,
	},
};

static struct platform_device rhea_stone_unicam_device = {
	 /* adding prefix mc to differ from char unicam interface*/
	.name = "kona-unicam-mc",
	.id = 0,
	.resource = rhea_stone_unicam_rsrc,
	.num_resources = ARRAY_SIZE(rhea_stone_unicam_rsrc),
	.dev = {
		.platform_data = &rhea_stone_unicam_pdata,
	},
};

static int __init rhea_stone_camera_init(void)
{
	dig_ch0_clk = clk_get(NULL, "dig_ch0_clk");
	if (IS_ERR_OR_NULL(dig_ch0_clk)) {
		printk(KERN_ERR "unable to get dig_ch0_clk\n");
		return -EINVAL;
	}

	if (clk_set_rate(dig_ch0_clk, 13000000)) {
		printk(KERN_ERR "unable to set clk rate 13MHz\n");
		clk_put(dig_ch0_clk);
		return -EINVAL;
	}

	if (gpio_request(SENSOR_0_GPIO_RST, "CamRst")) {
		printk(KERN_WARNING "cannot request gpio %d\n",
				SENSOR_0_GPIO_RST);
		return -EINVAL;
	} else
		gpio_direction_output(SENSOR_0_GPIO_RST, GPIOF_INIT_LOW);

	if (gpio_request(SENSOR_0_GPIO_PWRDN, "CamPwr")) {
		printk(KERN_WARNING "cannot request gpio %d\n",
				SENSOR_0_GPIO_RST);
		return -EINVAL;
	} else
		gpio_direction_output(SENSOR_0_GPIO_PWRDN, GPIOF_INIT_HIGH);

	return 0;
}
late_initcall(rhea_stone_camera_init);
#endif

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

/* Rhea Ray specific platform devices */
static struct platform_device *rhea_stone_plat_devices[] __initdata = {
#ifdef CONFIG_KEYBOARD_BCM
	&bcm_kp_device,
#endif
#ifdef CONFIG_KONA_HEADSET_MULTI_BUTTON
	&headset_device,
#endif

#ifdef CONFIG_DMAC_PL330
	&pl330_dmac_device,
#endif
#ifdef CONFIG_HAPTIC_SAMSUNG_PWM
	&haptic_pwm_device,
#endif
#ifdef CONFIG_BACKLIGHT_PWM
	&bcm_backlight_devices,
#endif

#if 0
#ifdef CONFIG_FB_BRCM_KONA
	&lq043y1dx01_dsi_display_device,
#endif
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

#ifdef CONFIG_VIDEO_UNICAM_CAMERA
	&rhea_camera_back,
	&rhea_camera_front,
#endif

#ifdef CONFIG_VIDEO_KONA
	&rhea_stone_unicam_device,
#endif

#ifdef CONFIG_GPS_IRQ
	&gps_hostwake,
#endif

#ifdef CONFIG_WD_TAPPER
	&wd_tapper,
#endif

};

#ifdef CONFIG_TOUCHSCREEN_TANGO
static struct TANGO_I2C_TS_t tango_plat_data = {
	.i2c_pdata	= {ADD_I2C_SLAVE_SPEED(BSC_BUS_SPEED_400K),},
	.i2c_slave_address	= 0,
	.gpio_irq_pin		= TANGO_GPIO_IRQ_PIN,
	.gpio_reset_pin		= TANGO_GPIO_RESET_PIN,
	.x_max_value		= 480,
	.y_max_value		= 800,
	.layout			= TANGO_S32_LAYOUT,
	.num_bytes_to_read = TANGO_I2C_TS_DRIVER_NUM_BYTES_TO_READ,
	.is_multi_touch		= 1,
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
	.max_finger_val		= 2,
	.panel_width		= 56,
};

static struct i2c_board_info __initdata tango_info[] =
{
	{	/* New touch screen i2c slave address. */
		I2C_BOARD_INFO(I2C_TS_DRIVER_NAME, TANGO_M29_SLAVE_ADDR),
		.platform_data = &tango_plat_data,
		.irq = gpio_to_irq(TANGO_GPIO_IRQ_PIN),
	},
};
#endif

#if defined(CONFIG_SENSORS_BMA222)
static struct i2c_board_info __initdata bma222_accl_info[] = {
	{
		I2C_BOARD_INFO("bma222_accl", 0x08),
		.irq = -1,
		.platform_data = &bma_pdata,
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
			printk(KERN_ERR "No Regulator available for ldo_hv8\n");
			return -1;
		}
	}
	if (reg) {
		if (vreg_en) {
			regulator_set_voltage(reg, 3000000, 3000000);
			printk(KERN_ERR "Turn on TP (ldo_hv8) to 2.8V\n");
			regulator_enable(reg);
		} else {
			printk(KERN_ERR "Turn off TP (ldo_hv8)\n");
			regulator_disable(reg);
		}
	} else {
		printk(KERN_ERR "TP Regulator Alloc Failed");
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
		.irq = gpio_to_irq(TANGO_GPIO_IRQ_PIN),
	},
};
#endif

#if defined(CONFIG_TOUCHSCREEN_BCM915500)|| defined(CONFIG_TOUCHSCREEN_BCM915500_MODULE)
static struct bcm915500_platform_data bcm915500_i2c_param =
{
	.id = -1,
#ifdef HW_BCM915500_I2C_BUS_ID
	.i2c_adapter_id = HW_BCM915500_I2C_BUS_ID,
#endif
};

static struct i2c_board_info bcm915500_i2c_boardinfo[] =
{
	{
		.type = BCM915500_TSC_NAME,
		.addr = HW_BCM915500_SLAVE_SPM,
		.platform_data = &bcm915500_i2c_param,
	},
};
#endif


/* Rhea Ray specific i2c devices */
static void __init rhea_stone_add_i2c_devices (void)
{

#ifdef CONFIG_TOUCHSCREEN_TANGO
	i2c_register_board_info(1,
		tango_info,
		ARRAY_SIZE(tango_info));
#endif
#ifdef CONFIG_TOUCHSCREEN_FT5306
	i2c_register_board_info(1,
		ft5306_info,
		ARRAY_SIZE(ft5306_info));
#endif
#ifdef  CONFIG_SENSORS_BMA222
        i2c_register_board_info(1,
                 bma222_accl_info,
                 ARRAY_SIZE(bma222_accl_info));
#endif
#if defined(CONFIG_TOUCHSCREEN_BCM915500) || defined(CONFIG_TOUCHSCREEN_BCM915500_MODULE)
#ifdef HW_BCM915500_I2C_BUS_ID /* Temporary: in bcm915500_i2c_ts.h */
	bcm915500_i2c_param.id = HW_BCM915500_I2C_BUS_ID;
#endif

#ifdef HW_BCM915500_I2C_BUS_ID
	bcm915500_i2c_param.i2c_adapter_id = HW_BCM915500_I2C_BUS_ID,
#endif

#ifdef HW_BCM915500_GPIO_RESET
	bcm915500_i2c_param.gpio_reset = HW_BCM915500_GPIO_RESET,
#endif

#ifdef HW_BCM915500_GPIO_INTERRUPT
	bcm915500_i2c_param.gpio_interrupt = HW_BCM915500_GPIO_INTERRUPT,
#endif

	bcm915500_i2c_boardinfo[0].irq =
		gpio_to_irq(bcm915500_i2c_param.gpio_interrupt);

	printk("PPTEST %s() bcm915500_i2c_boardinfo[0].irq: %d\n",
		__func__, bcm915500_i2c_boardinfo[0].irq);

	i2c_register_board_info(bcm915500_i2c_param.id,
				bcm915500_i2c_boardinfo,
				ARRAY_SIZE(bcm915500_i2c_boardinfo));
#endif

#if defined(CONFIG_BCMI2CNFC)
#if defined(CONFIG_MACH_RHEA_STONE_EDN2X)
	i2c_register_board_info(0, bcmi2cnfc, ARRAY_SIZE(bcmi2cnfc));
#else
	i2c_register_board_info(1, bcmi2cnfc, ARRAY_SIZE(bcmi2cnfc));
#endif
#endif

#if defined(CONFIG_GPS_IRQ)
	i2c_register_board_info(0, gpsi2c, ARRAY_SIZE(gpsi2c));
#endif

#if defined(CONFIG_MPU_SENSORS_MPU6050B1) || defined(CONFIG_MPU_SENSORS_MPU6050B1_MODULE)
#if defined(MPU6050_IRQ_GPIO)
	inv_mpu_i2c0_boardinfo[0].irq = gpio_to_irq(MPU6050_IRQ_GPIO);
#endif
	i2c_register_board_info(MPU6050_I2C_BUS_ID,
			inv_mpu_i2c0_boardinfo, ARRAY_SIZE(inv_mpu_i2c0_boardinfo));
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

#if defined(CONFIG_AL3006) || defined(CONFIG_AL3006_MODULE)
#ifdef AL3006_IRQ_GPIO
	i2c_al3006_info[0].irq = gpio_to_irq(AL3006_IRQ_GPIO);
#endif
	i2c_register_board_info(
#ifdef AL3006_I2C_BUS_ID
		AL3006_I2C_BUS_ID,
#else
		-1,
#endif
		i2c_al3006_info, ARRAY_SIZE(i2c_al3006_info));
#endif

}

static int __init rhea_stone_add_lateInit_devices (void)
{
	board_add_sdio_devices();
#ifdef CONFIG_BRCM_UNIFIED_DHD_SUPPORT

			printk(KERN_ERR "Calling WLAN_INIT!\n");

			 rhea_wlan_init();
				printk(KERN_ERR "DONE WLAN_INIT!\n");
#endif
	return 0;
}

static void __init rhea_stone_reserve(void)
{
	board_common_reserve();
}

/* All Rhea Ray specific devices */
static void __init rhea_stone_add_devices(void)
{
#if defined(CONFIG_SPI_GPIO)
	rheastone_add_lcd_spi();
#endif

#ifdef CONFIG_KEYBOARD_BCM
	bcm_kp_device.dev.platform_data = &bcm_keypad_data;
#endif
	platform_add_devices(rhea_stone_plat_devices, ARRAY_SIZE(rhea_stone_plat_devices));

	rhea_stone_add_i2c_devices();

	spi_register_board_info(spi_slave_board_info,
				ARRAY_SIZE(spi_slave_board_info));
}

#ifdef CONFIG_FB_BRCM_KONA
/*
 *   KONA FRAME BUFFER DSIPLAY DRIVER PLATFORM CONFIG
 */
struct kona_fb_platform_data konafb_devices[] __initdata = {
#ifdef CONFIG_LCD_HX8369_SUPPORT
	{
		.dispdrv_name  = "HX8369A",
		.dispdrv_entry = LCD_DISPDRV_GetFuncTable,
		 .parms = {
			.w0 = {
				.bits = {
					.boot_mode	= 0,
					.bus_type	= KONA_BUS_DSI,
					.bus_no = KONA_BUS_0,
					.bus_ch = KONA_BUS_CH_0,
					.bus_width	= 0,
					.te_input	= KONA_TE_IN_1_DSI0,
					.col_mode_i = KONA_CM_I_XRGB888,
					.col_mode_o = KONA_CM_O_RGB888,
				},
			},
			.w1 = {
			.bits = {
					.api_rev  =  KONA_LCD_BOOT_API_REV,
					.lcd_rst0 =  12,
				},
			},
		},
	},
#else
	{
		.dispdrv_name  = "LQ043Y1DX01",
		.dispdrv_entry = DISP_DRV_LQ043Y1DX01_GetFuncTable,
		.parms = {
			.w0 = {
				.bits = {
					.boot_mode  = 0,
					.bus_type   = KONA_BUS_DSI,
					.bus_no     = KONA_BUS_0,
					.bus_ch     = KONA_BUS_CH_0,
					.bus_width  = 0,
					.te_input   = KONA_TE_IN_1_DSI0,
					.col_mode_i = KONA_CM_I_RGB565,
					.col_mode_o = KONA_CM_O_RGB565,
				},
			},
			.w1 = {
			.bits = {
					.api_rev  =  KONA_LCD_BOOT_API_REV,
					.lcd_rst0 =  25, /* DSI_BRIDGE_PON   */
					.lcd_rst1 =  12, /* DSI_BRIDGE_RESET */
					.lcd_rst2 =  13, /* SHARP_RESET      */
				},
			},
		},
	},
#endif
};

#include "rhea_fb_init.c"
#endif /* #ifdef CONFIG_FB_BRCM_KONA */

void __init board_init(void)
{
	board_add_common_devices();
#ifdef CONFIG_FB_BRCM_KONA
	/* rhea_fb_init.c */
	konafb_init();
#endif
	rhea_stone_add_devices();
	return;
}

void __init board_map_io(void)
{
	/* Map machine specific iodesc here */

	rhea_map_io();
}

late_initcall(rhea_stone_add_lateInit_devices);

MACHINE_START(RHEA_STONE, "rheastone")
	.atag_offset = 0x100,
	.map_io = board_map_io,
	.init_irq = kona_init_irq,
	.handle_irq = gic_handle_irq,
	.timer  = &kona_timer,
	.init_machine = board_init,
	.reserve = rhea_stone_reserve,
	.restart = rhea_restart,
MACHINE_END
