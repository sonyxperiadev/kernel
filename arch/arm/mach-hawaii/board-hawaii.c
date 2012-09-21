#/************************************************************************/
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
#include <linux/android_pmem.h>
#include <linux/serial_8250.h>
#include <linux/i2c.h>
#include <linux/i2c-kona.h>
#include <linux/spi/spi.h>
#include <linux/clk.h>
#include <linux/bootmem.h>
#include <linux/mfd/bcm590xx/core.h>
#include <linux/gpio_keys.h>
#include <linux/i2c-kona.h>
#include <linux/i2c.h>
#include <linux/i2c/tango_ts.h>
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
#include <plat/spi_kona.h>
#include <plat/chal/chal_trace.h>
#include <plat/pi_mgr.h>
#include <plat/spi_kona.h>

#include <trace/stm.h>

#include "devices.h"

#if defined(CONFIG_BCM_ALSA_SOUND)
#include <mach/caph_platform.h>
#include <mach/caph_settings.h>
#endif	

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

#if defined(CONFIG_SENSORS_BMA222)
#include <linux/bma222.h>
#endif

#if defined(CONFIG_BMP18X_I2C) || defined(CONFIG_BMP18X_I2C_MODULE)
#include <linux/bmp18x.h>
#include <mach/bmp18x_i2c_settings.h>
#endif

#if defined(CONFIG_AL3006) || defined(CONFIG_AL3006_MODULE)
#include <mach/al3006_i2c_settings.h>
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

#ifdef CONFIG_VIDEO_UNICAM_CAMERA
#include <media/soc_camera.h>
#endif

#ifdef CONFIG_VIDEO_KONA
#include <media/v4l2-device.h>
#include <media/v4l2-subdev.h>
#include <media/kona-unicam.h>
#include <media/ov5640.h>
#endif

#ifdef CONFIG_WD_TAPPER
#include <linux/broadcom/wd-tapper.h>
#endif

#if defined(CONFIG_TOUCHSCREEN_BCM915500) || defined(CONFIG_TOUCHSCREEN_BCM915500_MODULE)
#include <linux/i2c/bcm15500_i2c_ts.h>
#endif

#ifdef CONFIG_USB_DWC_OTG
#include <mach/clock.h>
#include <linux/usb/bcm_hsotgctrl.h>
#include <linux/usb/otg.h>
#endif

#ifdef CONFIG_BRCM_UNIFIED_DHD_SUPPORT
#include "hawaii_wifi.h"

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

#define HAWAII_8250PORT(name, clk)				\
{								\
	.membase    = (void __iomem *)(KONA_##name##_VA),	\
	.mapbase    = (resource_size_t)(KONA_##name##_PA),	\
	.irq        = BCM_INT_ID_##name,			\
	.uartclk    = 26000000,					\
	.regshift   = 2,					\
	.iotype     = UPIO_DWAPB,				\
	.type       = PORT_16550A,				\
	.flags      = UPF_BOOT_AUTOCONF | UPF_BUG_THRE |	\
			UPF_FIXED_TYPE | UPF_SKIP_TEST,		\
	.private_data = (void __iomem *)((KONA_##name##_VA) +	\
					UARTB_USR_OFFSET),	\
	.clk_name = clk,					\
}

static struct plat_serial8250_port hawaii_uart_platform_data[] = {
	HAWAII_8250PORT(UART0, "uart0_clk"),
	HAWAII_8250PORT(UART1, "uart1_clk"),
	HAWAII_8250PORT(UART2, "uart2_clk"),
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
	&hawaii_tmon_device,
	&thermal_device,
#endif

59 	


#ifdef CONFIG_STM_TRACE
	&hawaii_stm_device,
#endif

#if defined(CONFIG_HW_RANDOM_KONA)
	&rng_device,
#endif

#if defined(CONFIG_USB_DWC_OTG)
	&hawaii_hsotgctrl_platform_device,
	&hawaii_otg_platform_device,
#endif

#ifdef CONFIG_KONA_AVS
	&kona_avs_device,
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
#endif

#ifdef CONFIG_SND_BCM_SOC
	&caph_i2s_device,
	&caph_pcm_device,
#endif
};


#ifdef CONFIG_KEYBOARD_BCM
/*
 * The keyboard definition structure.
 */
struct platform_device hawaii_kp_device = {
	.name = "bcm_keypad",
	.id = -1,
};

static struct bcm_keymap hawaii_keymap[] = {
	{BCM_KEY_ROW_0, BCM_KEY_COL_0, "Vol Down Key", KEY_VOLUMEDOWN},
	{BCM_KEY_ROW_0, BCM_KEY_COL_1, "Vol Up Key", KEY_VOLUMEUP},
	{BCM_KEY_ROW_0, BCM_KEY_COL_2, "Search Key", KEY_SEARCH},
	{BCM_KEY_ROW_0, BCM_KEY_COL_3, "unused", 0},
	{BCM_KEY_ROW_0, BCM_KEY_COL_4, "unused", 0},
	{BCM_KEY_ROW_0, BCM_KEY_COL_5, "unused", 0},
	{BCM_KEY_ROW_0, BCM_KEY_COL_6, "unused", 0},
	{BCM_KEY_ROW_0, BCM_KEY_COL_7, "unused", 0},
	{BCM_KEY_ROW_1, BCM_KEY_COL_0, "Home-Key", KEY_HOME},
	{BCM_KEY_ROW_1, BCM_KEY_COL_1, "Back Key", KEY_BACK},
	{BCM_KEY_ROW_1, BCM_KEY_COL_2, "Menu-Key", KEY_MENU},
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
	.row_num = 2,
	.col_num = 3,
	.keymap = hawaii_keymap,
	.bcm_keypad_base = (void *)__iomem HW_IO_PHYS_TO_VIRT(KEYPAD_BASE_ADDR),
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
		.irq = gpio_to_irq(AL3006_IRQ_GPIO),
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

static struct mpu_platform_data mpu6050_platform_data = {
	.int_config  = MPU6050_INIT_CFG,
	.level_shifter = 0,
	.orientation = MPU6050_DRIVER_ACCEL_GYRO_ORIENTATION,
};

static struct ext_slave_platform_data mpu_compass_data = {
	.bus = EXT_SLAVE_BUS_SECONDARY,
	.orientation = MPU6050_DRIVER_COMPASS_ORIENTATION,
};

static struct i2c_board_info __initdata inv_mpu_i2c0_boardinfo[] = {
	{
		I2C_BOARD_INFO("mpu6050", MPU6050_SLAVE_ADDR),
		.platform_data = &mpu6050_platform_data,
		.irq = gpio_to_irq(MPU6050_IRQ_GPIO)
	},
	{
		I2C_BOARD_INFO("ami306", MPU6050_COMPASS_SLAVE_ADDR),
		.platform_data = &mpu_compass_data,
	},
};

#endif /* CONFIG_MPU_SENSORS_MPU6050B1 */

#ifdef CONFIG_KONA_HEADSET_MULTI_BUTTON

#define HS_IRQ		gpio_to_irq(92)
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
	 * The actual ground may not be 0, but a small offset is added to it.
	 * This needs to be subtracted from the measured voltage to determine the
	 * correct value. This will vary for different HW based on the resistor
	 * values used.
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
	.button_adc_values_low = hawaii_button_adc_values,

	/*
	 * Pass the board specific button detection range
	 */
	.button_adc_values_high = hawaii_button_adc_values_2_1,

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

static struct platform_device pl330_dmac_device = {
	.name = "kona-dmac-pl330",
	.id = 0,
	.dev = {
		.platform_data = &hawaii_pl330_pdata,
		.coherent_dma_mask  = DMA_BIT_MASK(64),
	},
};
#endif

#if (defined(CONFIG_BCM_RFKILL) || defined(CONFIG_BCM_RFKILL_MODULE))
#define BCMBT_VREG_GPIO		26
#define BCMBT_N_RESET_GPIO	(-1)	/* Unused */
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

static struct resource hawaii_sdio1_resource[] = {
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

static struct resource hawaii_sdio2_resource[] = {
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

static struct resource hawaii_sdio3_resource[] = {
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
			.reset		= 3,
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
};

static struct platform_device hawaii_sdio1_device = {
	.name = "sdhci",
	.id = 0,
	.resource = hawaii_sdio1_resource,
	.num_resources   = ARRAY_SIZE(hawaii_sdio1_resource),
	.dev      = {
		.platform_data = &hawaii_sdio_param[0],
	},
};

static struct platform_device hawaii_sdio2_device = {
	.name = "sdhci",
	.id = 1,
	.resource = hawaii_sdio2_resource,
	.num_resources   = ARRAY_SIZE(hawaii_sdio2_resource),
	.dev      = {
		.platform_data = &hawaii_sdio_param[1],
	},
};

static struct platform_device hawaii_sdio3_device = {
	.name = "sdhci",
	.id = 2,
	.resource = hawaii_sdio3_resource,
	.num_resources   = ARRAY_SIZE(hawaii_sdio3_resource),
	.dev      = {
		.platform_data = &hawaii_sdio_param[2],
	},
};

static struct platform_device *hawaii_sdio_devices[] __initdata = {
	&hawaii_sdio2_device,
	&hawaii_sdio3_device,
	&hawaii_sdio1_device,
};

static void __init hawaii_add_sdio_devices(void)
{
	platform_add_devices(hawaii_sdio_devices,
			ARRAY_SIZE(hawaii_sdio_devices));
}

#ifdef CONFIG_BACKLIGHT_PWM

static struct platform_pwm_backlight_data bcm_backlight_data = {
/* backlight */
	.pwm_id		= 2,
	.max_brightness	= 32,   /* Android calibrates to 32 levels*/
	.dft_brightness	= 32,
	.polarity	= 1,    /* Inverted polarity */
	.pwm_period_ns	=  5000000,
};

static struct platform_device bcm_backlight_devices = {
	.name	= "pwm-backlight",
	.id	= 0,
	.dev	= {
		.platform_data = &bcm_backlight_data,
	},
};

#endif /*CONFIG_BACKLIGHT_PWM */

#ifdef CONFIG_VIDEO_KONA
static struct ov5640_platform_data ov5640_cam1_pdata = {
	.s_power = hawaii_ov_cam1_power,
};

struct unicam_subdev_i2c_board_info ov5640_cam1_i2c_device = {
	.board_info = {
		I2C_BOARD_INFO("ov5640-mc", OV5640_I2C_ADDRESS),
		.platform_data = &ov5640_cam1_pdata,
	},
	.i2c_adapter_id = 0,
};

static struct unicam_v4l2_subdevs_groups hawaii_unicam_subdevs[] = {
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

static struct unicam_platform_data hawaii_unicam_pdata = {
	.subdevs = hawaii_unicam_subdevs,
	.num_subdevs = ARRAY_SIZE(hawaii_unicam_subdevs),
};

static struct resource hawaii_unicam_rsrc[] = {
	[0] = {
		.start = BCM_INT_ID_CSI,
		.end = BCM_INT_ID_CSI,
		.flags = IORESOURCE_IRQ,
	},
};

static struct platform_device hawaii_unicam_device = {
	 /* adding prefix mc to differ from char unicam interface*/
	.name = "kona-unicam-mc",
	.id = 0,
	.resource = hawaii_unicam_rsrc,
	.num_resources = ARRAY_SIZE(hawaii_unicam_rsrc),
	.dev = {
		.platform_data = &hawaii_unicam_pdata,
	},
};

late_initcall(hawaii_camera_init);
#endif
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

#ifdef CONFIG_TOUCHSCREEN_TANGO
static struct TANGO_I2C_TS_t tango_plat_data = {
	.i2c_pdata	= {ADD_I2C_SLAVE_SPEED(BSC_BUS_SPEED_400K),},
	.i2c_slave_address	= 0,
	.gpio_irq_pin		= TSC_GPIO_IRQ_PIN,
	.gpio_reset_pin		= TSC_GPIO_RESET_PIN,
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
	.x1_width_idx		= 10,	/* X1 coordinate touch area of the first finger */
	.y1_width_idx		= 11,	/* Y1 coordinate touch area of the first finger */
	.x2_width_idx		= 12,	/* X2 coordinate touch area of the first finger */
	.y2_width_idx		= 13,	/* Y2 coordinate touch area of the first finger */
	.power_mode_idx		= 20,
	.int_mode_idx		= 21,	/* INT)mode register */
	.int_width_idx		= 22,	/* Interrupt pulse width */
	.min_finger_val		= 0,
	.max_finger_val		= 2,
	.panel_width		= 56,
};

static struct i2c_board_info __initdata tango_info[] = {
	{
		I2C_BOARD_INFO(I2C_TS_DRIVER_NAME, TANGO_M29_SLAVE_ADDR),
		.platform_data = &tango_plat_data,
		.irq = gpio_to_irq(TSC_GPIO_IRQ_PIN),
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
/* Remove this comment when the regulator references are fixed here for Hawaii */
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

#if defined(CONFIG_TOUCHSCREEN_BCM915500) || defined(CONFIG_TOUCHSCREEN_BCM915500_MODULE)
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

static struct platform_device *hawaii_devices[] __initdata = {
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

#if (defined(CONFIG_BCM_RFKILL) || defined(CONFIG_BCM_RFKILL_MODULE))
	&hawaii_bcmbt_rfkill_device,
#endif

#ifdef CONFIG_BCM_BZHW
	&hawaii_bcm_bzhw_device,
#endif

#ifdef CONFIG_BCM_BT_LPM
	&board_bcmbt_lpm_device,
#endif

#ifdef CONFIG_VIDEO_KONA
	&hawaii_unicam_device,
#endif

#ifdef CONFIG_WD_TAPPER
	&wd_tapper,
#endif

#ifdef(CONFIG_BCM_ALSA_SOUND)	
	&board_caph_device,
#endif	
};

static void __init hawaii_add_i2c_devices(void)
{

#ifdef CONFIG_TOUCHSCREEN_TANGO
	i2c_register_board_info(3, tango_info, ARRAY_SIZE(tango_info));
#endif

#ifdef CONFIG_TOUCHSCREEN_FT5306
	i2c_register_board_info(3, ft5306_info, ARRAY_SIZE(ft5306_info));
#endif

#ifdef CONFIG_SENSORS_BMA222
	i2c_register_board_info(2, bma222_accl_info, ARRAY_SIZE(bma222_accl_info));
#endif

#if defined(CONFIG_TOUCHSCREEN_BCM915500) || defined(CONFIG_TOUCHSCREEN_BCM915500_MODULE)
	i2c_register_board_info(3, bcm915500_i2c_boardinfo,
				ARRAY_SIZE(bcm915500_i2c_boardinfo));
#endif

#if defined(CONFIG_BCMI2CNFC)
	i2c_register_board_info(1, bcmi2cnfc, ARRAY_SIZE(bcmi2cnfc));
#endif

#if defined(CONFIG_MPU_SENSORS_MPU6050B1) || defined(CONFIG_MPU_SENSORS_MPU6050B1_MODULE)
	i2c_register_board_info(MPU6050_I2C_BUS_ID, inv_mpu_i2c0_boardinfo,
			ARRAY_SIZE(inv_mpu_i2c0_boardinfo));
#endif

#if defined(CONFIG_BMP18X_I2C) || defined(CONFIG_BMP18X_I2C_MODULE)
	i2c_register_board_info(BMP18X_I2C_BUS_ID, i2c_bmp18x_info,
			ARRAY_SIZE(i2c_bmp18x_info));
#endif

#if defined(CONFIG_AL3006) || defined(CONFIG_AL3006_MODULE)
	i2c_register_board_info(AL3006_I2C_BUS_ID, i2c_al3006_info,
			ARRAY_SIZE(i2c_al3006_info));
#endif

}



static void hawaii_add_pdata(void)
{
	hawaii_serial_device.dev.platform_data = &hawaii_uart_platform_data;
	hawaii_i2c_adap_devices[0].dev.platform_data = &bsc_i2c_cfg[0];
	hawaii_i2c_adap_devices[1].dev.platform_data = &bsc_i2c_cfg[1];
	hawaii_i2c_adap_devices[2].dev.platform_data = &bsc_i2c_cfg[2];
	hawaii_i2c_adap_devices[3].dev.platform_data = &bsc_i2c_cfg[3];
	hawaii_i2c_adap_devices[4].dev.platform_data = &bsc_i2c_cfg[4];
	hawaii_ssp0_device.dev.platform_data = &hawaii_ssp0_info;
	hawaii_ssp1_device.dev.platform_data = &hawaii_ssp1_info;
	hawaii_stm_device.dev.platform_data = &hawaii_stm_pdata;
#ifdef CONFIG_USB_DWC_OTG
	hawaii_hsotgctrl_platform_device.dev.platform_data = &hsotgctrl_plat_data;
#endif
}

/* Remove below extern when pmem pdata is moved to this file */
extern struct android_pmem_platform_data android_pmem_data;
void __init hawaii_add_common_devices(void)
{
	unsigned long pmem_size = android_pmem_data.cmasize;

	platform_device_register(&android_pmem);
	printk(KERN_EMERG"PMEM : CMA size (0x%08lx, %lu pages)\n",
				pmem_size, (pmem_size >> PAGE_SHIFT));

#ifdef CONFIG_ION
	platform_device_register(&ion_device0);
#endif

	platform_add_devices(hawaii_common_plat_devices,
			ARRAY_SIZE(hawaii_common_plat_devices));

}

static void __init hawaii_add_devices(void)
{

	hawaii_add_pdata();

#ifdef CONFIG_KEYBOARD_BCM
	hawaii_kp_device.dev.platform_data = &hawaii_keypad_data;
#endif
	platform_add_devices(hawaii_devices, ARRAY_SIZE(hawaii_devices));

	hawaii_add_i2c_devices();

	spi_register_board_info(spi_slave_board_info,
				ARRAY_SIZE(spi_slave_board_info));

}

#ifdef CONFIG_FB_BRCM_KONA
/*
 * KONA FRAME BUFFER DISPLAY DRIVER PLATFORM CONFIG
 */
struct kona_fb_platform_data konafb_devices[] __initdata = {
	{
		.dispdrv_name  = "NT35516",
		.dispdrv_entry = DISP_DRV_NT35516_GetFuncTable,
		.parms = {
			.w0 = {
				.bits = {
					.boot_mode  = 0,
					.bus_type   = KONA_BUS_DSI,
					.bus_no     = KONA_BUS_0,
					.bus_ch     = KONA_BUS_CH_0,
					.bus_width  = 3,
					.te_input   = KONA_TE_IN_1_DSI0,
					.col_mode_i = KONA_CM_I_XRGB888,
					.col_mode_o = KONA_CM_O_RGB888,
				},
			},
			.w1 = {
			.bits = {
					.api_rev  =  KONA_LCD_BOOT_API_REV,
					.lcd_rst0 =  22,
				},
			},
		},
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

MACHINE_START(HAWAII, "Hawaii")
	.atag_offset = 0x100,
	.map_io = hawaii_map_io,
	.init_irq = kona_init_irq,
	.handle_irq = gic_handle_irq,
	.timer = &kona_timer,
	.init_machine = hawaii_init,
	.reserve = hawaii_reserve,
	.restart = hawaii_restart,
MACHINE_END
