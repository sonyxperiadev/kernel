#include <linux/apq8064.h>
#ifdef CONFIG_LEDS_LM3533
#include <linux/kernel.h>
#include <linux/gpio.h>
#include "board-8064.h"

#include <linux/leds-lm3533_ng.h>

#define LM3533_HWEN_PM_GPIO 26
#define LM3533_HWEN_GPIO PM8921_GPIO_PM_TO_SYS(LM3533_HWEN_PM_GPIO)

static int lm3533_setup(struct device *dev)
{
	int rc = gpio_request(LM3533_HWEN_GPIO, "lm3533_hwen");
	if (rc)
		dev_err(dev, "failed to request gpio %d\n", LM3533_HWEN_GPIO);
	return rc;
}
static void lm3533_teardown(struct device *dev)
{
	gpio_free(LM3533_HWEN_GPIO);
	return;
}
static int lm3533_power_on(struct device *dev)
{
	gpio_set_value_cansleep(LM3533_HWEN_GPIO, 1);
	return 0;
}
static int lm3533_power_off(struct device *dev)
{
	gpio_set_value_cansleep(LM3533_HWEN_GPIO, 0);
	return 0;
}

static struct lm3533_startup_brightenss lm3533_startup_brightnesses[] = {
				[0] = {"lm3533-lcd-bl", 255},
				[1] = {"lm3533-red", 0},
				[2] = { NULL, 0} };

struct lm3533_platform_data lm3533_pdata = {
	.b_cnf = {
		[LM3533_CBNKA] = {
			.pwm = 0,
			.ctl = LM3533_HVA_MAP_LIN | LM3533_HVA_BR_CTL,
			.fsc =  I_UA_TO_FSC(20200),
			.iname = "lm3533-lcd-bl",
		},
		[LM3533_CBNKB] = {
			.pwm = LM3533_PWM_ALL,
			.ctl = LM3533_HVB_MAP_EXP | LM3533_HVB_BR_CTL,
			.fsc =  I_UA_TO_FSC(0),
			.iname = "not-connected",
		},
		[LM3533_CBNKC] = {
			.pwm = 0,
			.ctl = LM3533_LV_MAP_LIN | LM3533_LV_BR_CTL,
			/* 1ma in spec, but this is not possible */
			.fsc =  I_UA_TO_FSC(6600),
			.iname = "lm3533-red",
		},
		[LM3533_CBNKD] = {
			.pwm = 0,
			.ctl = LM3533_LV_MAP_LIN | LM3533_LV_BR_CTL,
			/* 1ma in spec, but this is not possible */
			.fsc =  I_UA_TO_FSC(7400),
			.iname = "lm3533-green",
		},
		[LM3533_CBNKE] = {
			.pwm = 0,
			.ctl = LM3533_LV_MAP_LIN | LM3533_LV_BR_CTL,
			/* 1ma in spec, but this is not possible */
			.fsc =  I_UA_TO_FSC(7400),
			.iname = "lm3533-blue",
		},
		[LM3533_CBNKF] = {
			.pwm = 0,
			.ctl = LM3533_LV_MAP_LIN | LM3533_LV_BR_CTL,
			.fsc =  I_UA_TO_FSC(0),
			.iname = "not-connected",
		},
	},
	.l_cnf = {
		[LM3533_HVLED1] = {
			.connected = true,
			.cpout = true,
			.bank =  LM3533_CBNKA,
		},
		[LM3533_HVLED2] = {
			.connected = true,
			.cpout = true,
			.bank =  LM3533_CBNKA,
		},
		[LM3533_LVLED1] = {
			.connected = true,
			.cpout = true,
			.bank =  LM3533_CBNKC,
		},
		[LM3533_LVLED2] = {
			.connected = true,
			.cpout = true,
			.bank =  LM3533_CBNKD,
		},
		[LM3533_LVLED3] = {
			.connected = true,
			.cpout = true,
			.bank =  LM3533_CBNKE,
		},
		[LM3533_LVLED4] = {
			.connected = false,
			.cpout = true,
			.bank =  LM3533_CBNKF,
		},
		[LM3533_LVLED5] = {
			.connected = false,
			.cpout = true,
			.bank =  LM3533_CBNKF,
		},
	},
	.ovp_boost_pwm = LM3533_BOOST_500KHZ | LM3533_OVP_24V | LM3533_PWM_HIGH,
	.led_fault = LM3533_OPEN_DISABLED | LM3533_SHORT_DISABLED,
	.setup = lm3533_setup,
	.teardown = lm3533_teardown,
	.power_on = lm3533_power_on,
	.power_off = lm3533_power_off,
	.als_control = LM3533_ALS_17920,
	.als_input_current = ALS_CUR_UA_TO_REG(150),
	.startup_brightness = lm3533_startup_brightnesses,
};

int usb_phy_init_seq_host[] = {
	0x74, 0x80, /* PARAMETER_OVERRIDE_A */
	0x30, 0x81, /* PARAMETER_OVERRIDE_B */
	0x30, 0x82, /* PARAMETER_OVERRIDE_C */
	0x33, 0x83, /* PARAMETER_OVERRIDE_D */
	-1
};

#define VBUS_BIT 0x04
static int __init startup_rgb(char *str)
{
	int vbus;
	if (get_option(&str, &vbus)) {
		if (vbus & VBUS_BIT)
			lm3533_startup_brightnesses[1].brightness = 50;
		return 0;
	}
	return -EINVAL;
}

early_param("startup", startup_rgb);
#endif
struct apq8064_data apq8064_data = {
	.v_hs_max = 2450,
	.hs_detect_extn_cable = true,
};
