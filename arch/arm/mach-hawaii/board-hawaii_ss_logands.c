/************************************************************************/
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
#include <linux/of_platform.h>
#include <linux/of.h>
#include <linux/of_fdt.h>
#include <mach/pinmux.h>

#ifdef CONFIG_ION_BCM_NO_DT
#include <linux/ion.h>
#include <linux/broadcom/bcm_ion.h>
#endif
#ifdef CONFIG_IOMMU_API
#include <plat/bcm_iommu.h>
#endif
#include <linux/serial_8250.h>
#include <linux/i2c.h>
#include <linux/i2c-kona.h>
#include <linux/spi/spi.h>
#include <linux/clk.h>
#include <linux/bootmem.h>
#include <linux/input.h>
#include <linux/mfd/bcm590xx/core.h>
#include <linux/gpio.h>
#include <linux/gpio_keys.h>
#include <linux/i2c-kona.h>
#include <linux/i2c.h>
#include <linux/i2c/tango_ts.h>
#include <linux/i2c/melfas_ts.h>
#include <asm/mach/arch.h>
#include <asm/mach-types.h>
#include <asm/mach/map.h>
#include <mach/hardware.h>
#include <mach/hardware.h>
#include <mach/kona_headset_pd.h>
#include <mach/kona.h>
#include <mach/sdio_platform.h>
#include <mach/hawaii.h>
#include <mach/io_map.h>
#include <mach/irqs.h>
#include <mach/rdb/brcm_rdb_uartb.h>
#include <mach/clock.h>
#include <plat/spi_kona.h>
#include <plat/chal/chal_trace.h>
#include <plat/pi_mgr.h>
#include <plat/spi_kona.h>
#include <plat/kona_smp.h>

#include <trace/stm.h>

#ifdef CONFIG_HAS_WAKELOCK
#include <linux/wakelock.h>
#endif

#include "devices.h"

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

#if defined(CONFIG_BCMI2CNFC)
#include <linux/bcmi2cnfc.h>
#endif

#if defined(CONFIG_SENSORS_BMC150)
#include <linux/bst_sensor_common.h>
#endif

#if defined(CONFIG_SENSORS_GP2AP002)
#include <linux/gp2ap002_dev.h>
#endif

#if defined(CONFIG_BMP18X) || defined(CONFIG_BMP18X_I2C)
	|| defined(CONFIG_BMP18X_I2C_MODULE)
#include <linux/bmp18x.h>
#include <mach/bmp18x_i2c_settings.h>
#endif

#if defined(CONFIG_AL3006) || defined(CONFIG_AL3006_MODULE)
#include <linux/al3006.h>
#include <mach/al3006_i2c_settings.h>
#endif

#if defined(CONFIG_MPU_SENSORS_MPU6050B1)
	|| defined(CONFIG_MPU_SENSORS_MPU6050B1_MODULE)
#include <linux/mpu.h>
#include <mach/mpu6050_settings.h>
#endif

#if defined(CONFIG_MPU_SENSORS_AMI306)
	|| defined(CONFIG_MPU_SENSORS_AMI306_MODULE)
#include <linux/ami306_def.h>
#include <linux/ami_sensor.h>
#include <mach/ami306_settings.h>
#endif

#if defined(CONFIG_KEYBOARD_GPIO) || defined(CONFIG_KEYBOARD_GPIO_MODULE)
#include <linux/input.h>
#include <linux/gpio_keys.h>
#endif


#ifdef CONFIG_BACKLIGHT_PWM
#include <linux/pwm_backlight.h>
#endif

#ifdef CONFIG_BACKLIGHT_KTD259B
#include <linux/ktd259b_bl.h>
#endif

#ifdef CONFIG_BACKLIGHT_TPS61158
#include <linux/tps61158_pwm_bl.h>
#endif
#ifdef CONFIG_FB_BRCM_KONA
#include <video/kona_fb.h>
#endif

#if defined(CONFIG_BCM_ALSA_SOUND)
#include <mach/caph_platform.h>
#include <mach/caph_settings.h>
#endif
#ifdef CONFIG_UNICAM_CAMERA
#include <media/soc_camera.h>
#include "../../../drivers/media/i2c/soc_camera/camdrv_ss.h"
#endif

#ifdef CONFIG_VIDEO_KONA
#include <media/v4l2-device.h>
#include <media/v4l2-subdev.h>
#include <media/kona-unicam.h>
#endif


#ifdef CONFIG_I2C_GPIO
#include <linux/i2c-gpio.h>
#endif

#if defined(CONFIG_RT8969) || defined(CONFIG_RT8973)
#include <linux/mfd/bcmpmu.h>
#include <linux/power_supply.h>
#include <linux/platform_data/rtmusc.h>
#endif

#ifdef CONFIG_USB_SWITCH_TSU6111
#include <linux/mfd/bcmpmu.h>
#include <linux/power_supply.h>
#include <linux/i2c/tsu6111.h>
#endif
#if defined(CONFIG_SEC_CHARGING_FEATURE)
/* Samsung charging feature */
#include <linux/spa_power.h>
#endif

#if defined(CONFIG_TOUCHSCREEN_BCM915500)
	|| defined(CONFIG_TOUCHSCREEN_BCM915500_MODULE)
#include <linux/i2c/bcmtch15xxx.h>
#endif

#ifdef CONFIG_USB_DWC_OTG
#include <linux/usb/bcm_hsotgctrl.h>
#include <linux/usb/otg.h>
#endif

#ifdef CONFIG_MOBICORE_DRIVER
#include <linux/broadcom/mobicore.h>
#endif


#ifdef CONFIG_BRCM_UNIFIED_DHD_SUPPORT
#include "hawaii_wifi.h"
extern int hawaii_wifi_status_register(
		void (*callback)(int card_present, void *dev_id),
		void *dev_id);
#endif

/* void send_usb_insert_event(enum bcmpmu_event_t event, void *para);*/
void send_chrgr_insert_event(enum bcmpmu_event_t event, void *para);
extern void  hawaii_timer_init(void);




#include <mach/dev-hawaii_logands_bt.h>
#include <mach/dev-hawaii_logands_camera.h>
#include <mach/dev-hawaii_logands_nfc.h>
#include <mach/dev-hawaii_logands_tsp.h>
#include <mach/dev-hawaii_sensor.h>


extern struct platform_device hawaii_camera;
extern struct platform_device hawaii_camera_sub;

extern void hawaii_bt_init(void);
extern void hawaii_keyboard_init(void);
extern void hawaii_touchscreen_init(void);
extern void hawaii_muic_init(void);
extern void hawaii_nfc_init(void);
extern void hawaii_sensor_init(void);
extern void hawaii_cam_init(void);



/* SD */
#define SD_CARDDET_GPIO_PIN	91


int reset_pwm_padcntrl(void)
{
	struct pin_config new_pin_config;
	int ret;
	new_pin_config.name = PN_GPIO24;
	new_pin_config.func = PF_GPIO24;
	ret = pinmux_set_pin_config(&new_pin_config);
	return ret;
}

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
	.hsotgctrl_virtual_mem_base = (void *)KONA_USB_HSOTG_CTRL_VA,
	.chipreg_virtual_mem_base = (void *)KONA_CHIPREG_VA,
	.irq = BCM_INT_ID_HSOTG_WAKEUP,
	.usb_ahb_clk_name = USB_OTG_AHB_BUS_CLK_NAME_STR,
	.mdio_mstr_clk_name = MDIOMASTER_PERI_CLK_NAME_STR,
};
#endif


#define GPIO_KEYS_SETTINGS { \
	{ KEY_HOME, 10, 1, "HOME", EV_KEY, 0, 64}, \
}

#if defined(CONFIG_KEYBOARD_GPIO) || defined(CONFIG_KEYBOARD_GPIO_MODULE)
static struct gpio_keys_button board_gpio_keys[] = GPIO_KEYS_SETTINGS;

static struct gpio_keys_platform_data gpio_keys_data = {
	.nbuttons = ARRAY_SIZE(board_gpio_keys),
	.buttons = board_gpio_keys,
};

static struct platform_device board_gpio_keys_device = {
	.name = "gpio-keys",
	.id = -1,
	.dev = {
		.platform_data = &gpio_keys_data,
	},
};
#endif


struct platform_device *hawaii_common_plat_devices[] __initdata = {
	&pmu_device,
	&hawaii_ssp0_device,

#ifdef CONFIG_SENSORS_KONA
	&thermal_device,
#endif

#ifdef CONFIG_STM_TRACE
	&hawaii_stm_device,
#endif

#if defined(CONFIG_HW_RANDOM_KONA)
	&rng_device,
#endif

#if defined(CONFIG_USB_DWC_OTG)
#ifndef CONFIG_OF
	&hawaii_usb_phy_platform_device,
	&hawaii_hsotgctrl_platform_device,
	&hawaii_otg_platform_device,
#endif
#endif

#ifdef CONFIG_KONA_AVS
	&avs_device,
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


#ifdef CONFIG_SND_BCM_SOC
	&hawaii_audio_device,
	&caph_i2s_device,
	&caph_pcm_device,
	&spdif_dit_device,

#endif
};

#ifdef CONFIG_KONA_HEADSET_MULTI_BUTTON

#define HS_IRQ		gpio_to_irq(121)
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
	{0,     110},
	/* Volume Up  Min, Max*/
	{111,   250},
	/* Volue Down Min, Max*/
	{251,   500},
};
static struct kona_headset_pd hawaii_headset_data = {
	/* GPIO state read is 0 on HS insert and 1 for
	 * HS remove
	 */

	.hs_default_state = 1,
	/*
	 * Because of the presence of the resistor in the MIC_IN line.
	 * The actual ground may not be 0, but a small offset is added to it.
	 * This needs to be subtracted from the measured voltage to determine
	 * the correct value. This will vary for different HW based
	 * on the resistor values used.
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
	/* .button_adc_values_low = hawaii_button_adc_values,
	*/
	.button_adc_values_low = 0,

	/*
	 * Pass the board specific button detection range
	 */
	.button_adc_values_high = hawaii_button_adc_values_2_1,

	/* AUDLDO supply id for changing regulator mode*/
	.ldo_id = "audldo_uc",

};
#endif /* CONFIG_KONA_HEADSET_MULTI_BUTTON */

#ifdef CONFIG_DMAC_PL330
static struct kona_pl330_data hawaii_pl330_pdata =	{
	/* Non Secure DMAC virtual base address */
	.dmac_ns_base = (void *) KONA_DMAC_NS_VA,
	/* Secure DMAC virtual base address */
	.dmac_s_base = (void *) KONA_DMAC_S_VA,
	/* # of PL330 dmac channels 'configurable' */
	.num_pl330_chans = 8,
	/* irq number to use */
	.irq_base = BCM_INT_ID_DMAC0,
	/* # of PL330 Interrupt lines connected to GIC */
	.irq_line_count = 8,
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

static struct sdio_platform_cfg hawaii_sdio0_param = {
	.configure_sdio_pullup = configure_sdio_pullup,
};

static struct sdio_platform_cfg hawaii_sdio_param = {

#ifdef CONFIG_BRCM_UNIFIED_DHD_SUPPORT
	.register_status_notify = hawaii_wifi_status_register,
#endif

};
static const struct of_dev_auxdata hawaii_auxdata_lookup[] __initconst = {

	OF_DEV_AUXDATA("bcm,pwm-backlight", 0x0,
		"pwm-backlight.0", NULL),

	OF_DEV_AUXDATA("bcm,sdhci", 0x3F190000,
		"sdhci.1", NULL),

	OF_DEV_AUXDATA("bcm,sdhci", 0x3F1A0000,
		"sdhci.2", &hawaii_sdio_param),

	OF_DEV_AUXDATA("bcm,sdhci", 0x3F180000,
		"sdhci.0", &hawaii_sdio0_param),
	{},
};


#ifdef CONFIG_BACKLIGHT_KTD259B

static struct platform_ktd259b_backlight_data bcm_ktd259b_backlight_data = {
	.max_brightness = 255,
	.dft_brightness = 160,
	.ctrl_pin = 24,
};

struct platform_device hawaii_backlight_device = {
	.name = "panel",
	.id = -1,
	.dev = {
		.platform_data = &bcm_ktd259b_backlight_data,
	},
};

#endif /* CONFIG_BACKLIGHT_KTD259B */

#if defined(CONFIG_BCM_ALSA_SOUND)
static struct caph_platform_cfg board_caph_platform_cfg =
#ifdef HW_CFG_CAPH
HW_CFG_CAPH;
#else
{
	.aud_ctrl_plat_cfg = {
				.ext_aud_plat_cfg = {
					.ihf_ext_amp_gpio = -1,
					.dock_aud_route_gpio = -1,
					.mic_sel_aud_route_gpio = -1,
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

#ifdef CONFIG_RT8973
extern int bcmpmu_accy_chrgr_type_notify(int chrgr_type);

void send_chrgr_insert_event(enum bcmpmu_event_t event, void *para)
{
	bcmpmu_accy_chrgr_type_notify(*(u32 *)para);
}
#endif

static struct platform_device *hawaii_devices[] __initdata = {
#ifdef CONFIG_KEYBOARD_BCM
	&hawaii_kp_device,
#endif
#ifdef CONFIG_KONA_HEADSET_MULTI_BUTTON
	&hawaii_headset_device,
#endif

#ifdef CONFIG_DMAC_PL330
	&hawaii_pl330_dmac_device,
#endif

#ifdef CONFIG_HAPTIC_SAMSUNG_PWM
	&haptic_pwm_device,
#endif

#if	defined(CONFIG_BACKLIGHT_KTD259B)
	&hawaii_backlight_device,
#endif

#ifdef CONFIG_VIDEO_KONA
	&hawaii_unicam_device,
#endif

#if defined(CONFIG_BCM_ALSA_SOUND)
	&board_caph_device,
#endif

#if defined(CONFIG_TOUCHKEY_BACKLIGHT)
	&touchkeyled_device
#endif
};

/* The GPIO used to indicate accessory insertion in this board */
#define HS_IRQ		gpio_to_irq(121)

static void hawaii_add_pdata(void)
{
	hawaii_ssp0_device.dev.platform_data = &hawaii_ssp0_info;
	hawaii_ssp1_device.dev.platform_data = &hawaii_ssp1_info;
#ifdef CONFIG_BCM_STM
	hawaii_stm_device.dev.platform_data = &hawaii_stm_pdata;
#endif
	hawaii_headset_device.dev.platform_data = &hawaii_headset_data;
	/* The resource in position 2 (starting from 0) is used to fill
	 * the GPIO number. The driver file assumes this. So put the
	 * board specific GPIO number here
	 */
	hawaii_headset_device.resource[2].start = HS_IRQ;
	hawaii_headset_device.resource[2].end   = HS_IRQ;

	hawaii_pl330_dmac_device.dev.platform_data = &hawaii_pl330_pdata;
#ifdef CONFIG_USB_DWC_OTG
	hawaii_hsotgctrl_platform_device.dev.platform_data =
	    &hsotgctrl_plat_data;
	hawaii_usb_phy_platform_device.dev.platform_data =
		&hsotgctrl_plat_data;
#endif
}

void __init hawaii_add_common_devices(void)
{
	platform_add_devices(hawaii_common_plat_devices,
			     ARRAY_SIZE(hawaii_common_plat_devices));
}

static void __init hawaii_add_devices(void)
{

	hawaii_add_pdata();

	platform_add_devices(hawaii_devices, ARRAY_SIZE(hawaii_devices));

	spi_register_board_info(spi_slave_board_info,
				ARRAY_SIZE(spi_slave_board_info));

}

static struct of_device_id hawaii_dt_match_table[] __initdata = {
	{ .compatible = "simple-bus", },
	{}
};

static void __init hawaii_init(void)
{
	hawaii_add_devices();
	hawaii_add_common_devices();
	/* Populate platform_devices from device tree data */
	of_platform_populate(NULL, hawaii_dt_match_table,
			hawaii_auxdata_lookup, &platform_bus);
#if defined(CONFIG_KEYBOARD_GPIO) || defined(CONFIG_KEYBOARD_GPIO_MODULE)
		platform_device_register(&board_gpio_keys_device);
#endif
#ifdef CONFIG_VIDEO_ADP1653
	i2c_register_board_info(0, adp1653_flash, ARRAY_SIZE(adp1653_flash));
#endif
		/* tsp init I2C*/
	hawaii_touchscreen_init();
		/*  init usb */
	hawaii_muic_init();
		/*  init nfc */
	hawaii_nfc_init();
		/*  init sensor */
	hawaii_sensor_init();
		/*  init camera */
	hawaii_cam_init();

	return;
}

static int __init hawaii_add_lateinit_devices(void)
{

#ifdef CONFIG_BRCM_UNIFIED_DHD_SUPPORT
	hawaii_wlan_init();
#endif
	return 0;
}

late_initcall(hawaii_add_lateinit_devices);
static const char *hawaii_dt_compat[] = { "bcm,hawaii", NULL, };
MACHINE_START(HAWAII, "hawaii_ss_logands")
	.smp = smp_ops(kona_smp_ops),
	.map_io = hawaii_map_io,
	.init_irq = kona_init_irq,
	.init_time = hawaii_timer_init,
	.init_machine = hawaii_init,
	.reserve = hawaii_reserve,
	.restart = hawaii_restart,
	.dt_compat = hawaii_dt_compat,
MACHINE_END
