/* arch/arm/mach-msm/gpiomux-fuji_hikari.c
 *
 * Copyright (C) [2011] Sony Ericsson Mobile Communications AB.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */
#include <linux/module.h>
#include <mach/gpiomux.h>
#include "gpiomux-fuji_pm8058.h"

static struct gpiomux_setting unused_gpio = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir  = GPIOMUX_OUT_LOW,
};

static struct gpiomux_setting gpio_2ma_no_pull_low = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir  = GPIOMUX_OUT_LOW,
};

static struct gpiomux_setting gpio_2ma_no_pull_in = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir  = GPIOMUX_IN,
};

static struct gpiomux_setting gpio_2ma_pull_up_in = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
	.dir  = GPIOMUX_IN,
};

static struct gpiomux_setting gpio_2ma_pull_down_low = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir  = GPIOMUX_OUT_LOW,
};

static struct gpiomux_setting gpio_2ma_pull_down_in = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir  = GPIOMUX_IN,
};

static struct gpiomux_setting uart_rx = {
	.func = GPIOMUX_FUNC_2,
	.drv  = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_UP,
};

static struct gpiomux_setting uart_tx = {
	.func = GPIOMUX_FUNC_2,
	.drv  = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting sdc = {
	.func = GPIOMUX_FUNC_1,
	.drv  = GPIOMUX_DRV_10MA,
	.pull = GPIOMUX_PULL_UP,
};

static struct gpiomux_setting sdc_clk = {
	.func = GPIOMUX_FUNC_1,
	.drv  = GPIOMUX_DRV_16MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting uartdm_bt = {
	.func = GPIOMUX_FUNC_1,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting sensor_i2c = {
	.func = GPIOMUX_FUNC_2,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting generic_i2c = {
	.func = GPIOMUX_FUNC_1,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting cam_mclk = {
	.func = GPIOMUX_FUNC_1,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting uim = {
	.func = GPIOMUX_FUNC_1,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting felica_uart = {
	.func = GPIOMUX_FUNC_1,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting irda = {
	.func = GPIOMUX_FUNC_1,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting ps_hold = {
	.func = GPIOMUX_FUNC_1,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting tsif2 = {
	.func = GPIOMUX_FUNC_1,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting hdmi = {
	.func = GPIOMUX_FUNC_1,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting cdc_mclk = {
	.func = GPIOMUX_FUNC_1,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting fm_i2s = {
	.func = GPIOMUX_FUNC_1,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting audio_pcm = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting ant_sel = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct msm_gpiomux_config semc_fuji_all_cfgs[] __initdata = {
	{ /* NC */
		.gpio = 0,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* NC */
		.gpio = 1,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* EX_CHARGER_DISABLE */
		.gpio = 2,
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_2ma_no_pull_low,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_low,
		},
	},
	{ /* NC */
		.gpio = 3,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* NC */
		.gpio = 4,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* GPS_LNA_EN */
		.gpio = 5,
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_2ma_no_pull_low,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_low,
		},
	},
	{ /* NC */
		.gpio = 6,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* NC */
		.gpio = 7,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* XVS_STROBE */
		.gpio = 8,
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_2ma_pull_down_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_pull_down_in,
		},
	},
	{ /* NC */
		.gpio = 9,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* NC */
		.gpio = 10,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* HW_ID0 */
		.gpio = 11,
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_2ma_no_pull_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_in,
		},
	},
	{ /* HW_ID2 */
		.gpio = 12,
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_2ma_no_pull_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_in,
		},
	},
	{ /* FG_CE */
		.gpio = 13,
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_2ma_no_pull_low,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_low,
		},
	},
	{ /* NC */
		.gpio = 14,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* HW_ID1 */
		.gpio = 15,
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_2ma_no_pull_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_in,
		},
	},
	{ /* LOUDSPKR_AMP_R_EN */
		.gpio = 16,
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_2ma_no_pull_low,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_low,
		},
	},
	{ /* NC */
		.gpio = 17,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* LCD_PWR_EN */
		.gpio = 18,
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_2ma_no_pull_low,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_low,
		},
	},
	{ /* NC */
		.gpio = 19,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* NC */
		.gpio = 20,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* GYRO_FSYNC */
		.gpio = 21,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* NC */
		.gpio = 22,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* BT_RST_N */
		.gpio = 23,
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_2ma_no_pull_low,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_low,
		},
	},
	{ /* NC */
		.gpio = 24,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* CHAT_CAM_RST_N */
		.gpio = 25,
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_2ma_no_pull_low,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_low,
		},
	},
	{ /* SW_SERVICE */
		.gpio = 26,
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_2ma_pull_up_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_pull_up_in,
		},
	},
	{ /* DTV_RESET_N */
		.gpio = 27,
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_2ma_no_pull_low,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_low,
		},
	},
	{ /* USB_OTG_EN */
		.gpio = 28,
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_2ma_no_pull_low,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_low,
		},
	},
	{ /* NC */
		.gpio = 29,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* GYRO_INT */
		.gpio = 30,
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_2ma_no_pull_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_pull_down_in,
		},
	},
	{ /* COMPASS_IRQ */
		.gpio = 31,
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_2ma_no_pull_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_pull_down_in,
		},
	},
	{ /* CAMIF_MCLK */
		.gpio = 32,
		.settings = {
			[GPIOMUX_ACTIVE] = &cam_mclk,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_low,
		},
	},
	{ /* NC */
		.gpio = 33,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* PROX_INT */
		.gpio = 34,
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_2ma_no_pull_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_in,
		},
	},
	{ /* DEBUG_GPIO2 */
		.gpio = 35,
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_2ma_no_pull_low,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_low,
		},
	},
	{ /* DEBUG_GPIO1 */
		.gpio = 36,
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_2ma_no_pull_low,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_low,
		},
	},
	{ /* UIM1_M_DATA */
		.gpio = 37,
		.settings = {
			[GPIOMUX_ACTIVE] = &uim,
			[GPIOMUX_SUSPENDED] = &uim,
		},
	},
	{ /* UIM1_M_CLK */
		.gpio = 38,
		.settings = {
			[GPIOMUX_ACTIVE] = &uim,
			[GPIOMUX_SUSPENDED] = &uim,
		},
	},
	{ /* NC */
		.gpio = 39,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* NC */
		.gpio = 40,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* ACCEL_IRQ */
		.gpio = 41,
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_2ma_no_pull_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_pull_down_in,
		},
	},
	{ /* NC */
		.gpio = 42,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* TP_I2C_DATA */
		.gpio = 43,
		.settings = {
			[GPIOMUX_ACTIVE] = &generic_i2c,
			[GPIOMUX_SUSPENDED] = &generic_i2c,
		},
	},
	{ /* TP_I2C_CLK */
		.gpio = 44,
		.settings = {
			[GPIOMUX_ACTIVE] = &generic_i2c,
			[GPIOMUX_SUSPENDED] = &generic_i2c,
		},
	},
	{ /* NC */
		.gpio = 45,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* NC */
		.gpio = 46,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* CAMIF_I2C_DATA */
		.gpio = 47,
		.settings = {
			[GPIOMUX_ACTIVE] = &generic_i2c,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_pull_down_in,
		},
	},
	{ /* CAMIF_I2C_CLK */
		.gpio = 48,
		.settings = {
			[GPIOMUX_ACTIVE] = &generic_i2c,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_pull_down_in,
		},
	},
	{ /* IrDA_TX */
		.gpio = 49,
		.settings = {
			[GPIOMUX_ACTIVE] = &irda,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_low,
		},
	},
	{ /* IrDA_RX */
		.gpio = 50,
		.settings = {
			[GPIOMUX_ACTIVE] = &irda,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_pull_down_in,
		},
	},
	{ /* DTV_I2C_DATA */
		.gpio = 51,
		.settings = {
			[GPIOMUX_ACTIVE] = &generic_i2c,
			[GPIOMUX_SUSPENDED] = &generic_i2c,
		},
	},
	{ /* DTV_I2C_SCL */
		.gpio = 52,
		.settings = {
			[GPIOMUX_ACTIVE] = &generic_i2c,
			[GPIOMUX_SUSPENDED] = &generic_i2c,
		},
	},
	{ /* UARTDM_BT_TX */
		.gpio = 53,
		.settings = {
			[GPIOMUX_ACTIVE] = &uartdm_bt,
			[GPIOMUX_SUSPENDED] = &uartdm_bt,
		},
	},
	{ /* UARTDM_BT_RX */
		.gpio = 54,
		.settings = {
			[GPIOMUX_ACTIVE] = &uartdm_bt,
			[GPIOMUX_SUSPENDED] = &uartdm_bt,
		},
	},
	{ /* UARTDM_BT_CTS_N */
		.gpio = 55,
		.settings = {
			[GPIOMUX_ACTIVE] = &uartdm_bt,
			[GPIOMUX_SUSPENDED] = &uartdm_bt,
		},
	},
	{ /* UARTDM_BT_RFR_N */
		.gpio = 56,
		.settings = {
			[GPIOMUX_ACTIVE] = &uartdm_bt,
			[GPIOMUX_SUSPENDED] = &uartdm_bt,
		},
	},
	{ /* FELICA_UART_TX */
		.gpio = 57,
		.settings = {
			[GPIOMUX_ACTIVE] = &felica_uart,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_low,
		},
	},
	{ /* FELICA_UART_RX */
		.gpio = 58,
		.settings = {
			[GPIOMUX_ACTIVE] = &felica_uart,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_pull_down_in,
		},
	},
	{ /* AUD_I2C_DATA */
		.gpio = 59,
		.settings = {
			[GPIOMUX_ACTIVE] = &generic_i2c,
			[GPIOMUX_SUSPENDED] = &generic_i2c,
		},
	},
	{ /* AUD_I2C_CLK */
		.gpio = 60,
		.settings = {
			[GPIOMUX_ACTIVE] = &generic_i2c,
			[GPIOMUX_SUSPENDED] = &generic_i2c,
		},
	},
	{ /* HEADSET_DET_N */
		.gpio = 61,
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_2ma_no_pull_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_in,
		},
	},
	{ /* NC */
		.gpio = 62,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* BT_WAKES_MSM */
		.gpio = 63,
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_2ma_pull_down_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_pull_down_in,
		},
	},
	{ /* PERIPHERAL_I2C_DATA */
		.gpio = 64,
		.settings = {
			[GPIOMUX_ACTIVE] = &generic_i2c,
			[GPIOMUX_SUSPENDED] = &generic_i2c,
		},
	},
	{ /* PERIPHERAL_I2C_CLK */
		.gpio = 65,
		.settings = {
			[GPIOMUX_ACTIVE] = &generic_i2c,
			[GPIOMUX_SUSPENDED] = &generic_i2c,
		},
	},
	{ /* NC */
		.gpio = 66,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* NC */
		.gpio = 67,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* NC */
		.gpio = 68,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* NC */
		.gpio = 69,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* MLCD_RESET_N */
		.gpio = 70,
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_2ma_no_pull_low,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_low,
		},
	},
	{ /* NC */
		.gpio = 71,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* NC */
		.gpio = 72,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* VOL_DOWN_KEY */
		.gpio = 73,
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_2ma_no_pull_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_in,
		},
	},
	{ /* MSM_BOOT_FROM_ROM */
		.gpio = 74,
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_2ma_no_pull_low,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_low,
		},
	},
	{ /* DRX_ANT_SEL1/WDOC_DISABLE */
		.gpio = 75,
		.settings = {
			[GPIOMUX_ACTIVE] = &ant_sel,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_pull_down_in,
		},
	},
	{ /* DRX_ANT_SEL0/BOOT_CONFIG_6 */
		.gpio = 76,
		.settings = {
			[GPIOMUX_ACTIVE] = &ant_sel,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_pull_down_in,
		},
	},
	{ /* ANT_SEL3 */
		.gpio = 77,
		.settings = {
			[GPIOMUX_ACTIVE] = &ant_sel,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_pull_down_in,
		},
	},
	{ /* ANT_SEL2/BOOT_CONFIG_5 */
		.gpio = 78,
		.settings = {
			[GPIOMUX_ACTIVE] = &ant_sel,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_pull_down_in,
		},
	},
	{ /* ANT_SEL1/BOOT_CONFIG_4 */
		.gpio = 79,
		.settings = {
			[GPIOMUX_ACTIVE] = &ant_sel,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_pull_down_in,
		},
	},
	{ /* ANT_SEL0/BOOT_CONFIG_3 */
		.gpio = 80,
		.settings = {
			[GPIOMUX_ACTIVE] = &ant_sel,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_pull_down_in,
		},
	},
	{ /* BOOT_CONFIG_0 */
		.gpio = 81,
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_2ma_no_pull_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_in,
		},
	},
	{ /* BOOT_CONFIG_2 */
		.gpio = 82,
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_2ma_pull_down_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_pull_down_in,
		},
	},
	{ /* JAM_DET_IRQ */
		.gpio = 83,
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_2ma_no_pull_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_pull_down_in,
		},
	},
	{ /* BOOT_CONFIG_1 */
		.gpio = 84,
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_2ma_no_pull_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_in,
		},
	},
	{ /* NC */
		.gpio = 85,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* HW_ID3 */
		.gpio = 86,
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_2ma_no_pull_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_in,
		},
	},
	{ /* PM8058_APC_SEC_IRQ_N */
		.gpio = 87,
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_2ma_no_pull_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_in,
		},
	},
	{ /* PM8058_APC_USR_IRQ_N */
		.gpio = 88,
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_2ma_no_pull_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_in,
		},
	},
	{ /* PM8058_MDM_IRQ_N */
		.gpio = 89,
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_2ma_no_pull_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_in,
		},
	},
	{ /* PM8901_APC_SEC_IRQ_N */
		.gpio = 90,
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_2ma_no_pull_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_in,
		},
	},
	{ /* PM8901_APC_USR_IRQ_N */
		.gpio = 91,
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_2ma_no_pull_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_in,
		},
	},
	{ /* PS_HOLD */
		.gpio = 92,
		.settings = {
			[GPIOMUX_ACTIVE] = &ps_hold,
			[GPIOMUX_SUSPENDED] = &ps_hold,
		},
	},
	{ /* NC */
		.gpio = 93,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* SIM_CARD_DETECT */
		.gpio = 94,
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_2ma_no_pull_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_in,
		},
	},
	{ /* NC */
		.gpio = 95,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* NC */
		.gpio = 96,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* TSIF2_CLK */
		.gpio = 97,
		.settings = {
			[GPIOMUX_ACTIVE] = &tsif2,
			[GPIOMUX_SUSPENDED] = &tsif2,
		},
	},
	{ /* TSIF2_EN */
		.gpio = 98,
		.settings = {
			[GPIOMUX_ACTIVE] = &tsif2,
			[GPIOMUX_SUSPENDED] = &tsif2,
		},
	},
	{ /* TSIF2_DATA */
		.gpio = 99,
		.settings = {
			[GPIOMUX_ACTIVE] = &tsif2,
			[GPIOMUX_SUSPENDED] = &tsif2,
		},
	},
	{ /* NC */
		.gpio = 100,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* FM_I2S_WS */
		.gpio = 101,
		.settings = {
			[GPIOMUX_ACTIVE] = &fm_i2s,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_pull_down_in,
		},
	},
	{ /* FM_I2S_SCK */
		.gpio = 102,
		.settings = {
			[GPIOMUX_ACTIVE] = &fm_i2s,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_pull_down_in,
		},
	},
	{ /* IrDA_PWRDWN */
		.gpio = 103,
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_2ma_no_pull_low,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_low,
		},
	},
	{ /* OTG_OVERCUR_INT */
		.gpio = 104,
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_2ma_pull_up_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_pull_up_in,
		},
	},
	{ /* NC */
		.gpio = 105,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* CAM_RST_N */
		.gpio = 106,
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_2ma_no_pull_low,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_low,
		},
	},
	{ /* FM_I2S_SD */
		.gpio = 107,
		.settings = {
			[GPIOMUX_ACTIVE] = &fm_i2s,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_pull_down_in,
		},
	},
	{ /* CDC_TX_MCLK */
		.gpio = 108,
		.settings = {
			[GPIOMUX_ACTIVE] = &cdc_mclk,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_pull_down_low,
		},
	},
	{ /* CDC_RX_MCLK1 */
		.gpio = 109,
		.settings = {
			[GPIOMUX_ACTIVE] = &cdc_mclk,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_pull_down_low,
		},
	},
	{ /* NC */
		.gpio = 110,
		.settings = {
			[GPIOMUX_SUSPENDED] = &unused_gpio,
		},
	},
	{ /* AUDIO_PCM_DOUT */
		.gpio = 111,
		.settings = {
			[GPIOMUX_ACTIVE] = &audio_pcm,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_pull_down_in,
		},
	},
	{ /* AUDIO_PCM_DIN */
		.gpio = 112,
		.settings = {
			[GPIOMUX_ACTIVE] = &audio_pcm,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_pull_down_in,
		},
	},
	{ /* AUDIO_PCM_SYNC */
		.gpio = 113,
		.settings = {
			[GPIOMUX_ACTIVE] = &audio_pcm,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_pull_down_in,
		},
	},
	{ /* AUDIO_PCM_CLK */
		.gpio = 114,
		.settings = {
			[GPIOMUX_ACTIVE] = &audio_pcm,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_pull_down_in,
		},
	},
	{ /* SENSOR_I2C_CLK */
		.gpio = 115,
		.settings = {
			[GPIOMUX_ACTIVE] = &sensor_i2c,
			[GPIOMUX_SUSPENDED] = &sensor_i2c,
		},
	},
	{ /* SENSOR_I2C_DATA */
		.gpio = 116,
		.settings = {
			[GPIOMUX_ACTIVE] = &sensor_i2c,
			[GPIOMUX_SUSPENDED] = &sensor_i2c,
		},
	},
	{ /* UART_RX */
		.gpio = 117,
		.settings = {
			[GPIOMUX_ACTIVE] = &uart_rx,
			[GPIOMUX_SUSPENDED] = &uart_rx,
		},
	},
	{ /* UART_TX */
		.gpio = 118,
		.settings = {
			[GPIOMUX_ACTIVE] = &uart_tx,
			[GPIOMUX_SUSPENDED] = &uart_tx,
		},
	},
	{ /* NC */
		.gpio = 119,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* DIVANT_MAT_CNT1 */
		.gpio = 120,
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_2ma_no_pull_low,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_low,
		},
	},
	{ /* DIVANT_MAT_CNT2 */
		.gpio = 121,
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_2ma_no_pull_low,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_low,
		},
	},
	{ /* NC */
		.gpio = 122,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* FG_SOC_INT */
		.gpio = 123,
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_2ma_pull_up_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_pull_up_in,
		},
	},
	{ /* NC */
		.gpio = 124,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* EX_CHARGER_INT_N */
		.gpio = 125,
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_2ma_pull_up_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_pull_up_in,
		},
	},
	{ /* CHGPAD_DET */
		.gpio = 126,
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_2ma_pull_up_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_pull_up_in,
		},
	},
	{ /* TP_INT */
		.gpio = 127,
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_2ma_pull_up_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_pull_up_in,
		},
	},
	{ /* WL_HOST_WAKEUP */
		.gpio = 128,
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_2ma_no_pull_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_pull_down_in,
		},
	},
	{ /* NC */
		.gpio = 129,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* WL_RST_N */
		.gpio = 130,
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_2ma_no_pull_low,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_low,
		},
	},
	{ /* NC */
		.gpio = 131,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* NC */
		.gpio = 132,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* NC */
		.gpio = 133,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* NC */
		.gpio = 134,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* NC */
		.gpio = 135,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* NC */
		.gpio = 136,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* MSM_WAKES_BT */
		.gpio = 137,
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_2ma_no_pull_low,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_low,
		},
	},
	{ /* FLASH_DR_RST */
		.gpio = 138,
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_2ma_no_pull_low,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_low,
		},
	},
	{ /* NC */
		.gpio = 139,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* NC */
		.gpio = 140,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* NC */
		.gpio = 141,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* NC */
		.gpio = 142,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* NC */
		.gpio = 143,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* NC */
		.gpio = 144,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* NC */
		.gpio = 145,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* NC */
		.gpio = 146,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* NC */
		.gpio = 147,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* NC */
		.gpio = 148,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* NC */
		.gpio = 149,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* NC */
		.gpio = 150,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* NC */
		.gpio = 151,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* NC */
		.gpio = 152,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* NC */
		.gpio = 153,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* NC */
		.gpio = 154,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* NC */
		.gpio = 155,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* NC */
		.gpio = 156,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* NC */
		.gpio = 157,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* NC */
		.gpio = 158,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* SDC1_DATA0 */
		.gpio = 159,
		.settings = {
			[GPIOMUX_ACTIVE] = &sdc,
			[GPIOMUX_SUSPENDED] = &sdc,
		},
	},
	{ /* SDC1_DATA1 */
		.gpio = 160,
		.settings = {
			[GPIOMUX_ACTIVE] = &sdc,
			[GPIOMUX_SUSPENDED] = &sdc,
		},
	},
	{ /* SDC1_DATA2 */
		.gpio = 161,
		.settings = {
			[GPIOMUX_ACTIVE] = &sdc,
			[GPIOMUX_SUSPENDED] = &sdc,
		},
	},
	{ /* SDC1_DATA3 */
		.gpio = 162,
		.settings = {
			[GPIOMUX_ACTIVE] = &sdc,
			[GPIOMUX_SUSPENDED] = &sdc,
		},
	},
	{ /* SDC1_DATA4 */
		.gpio = 163,
		.settings = {
			[GPIOMUX_ACTIVE] = &sdc,
			[GPIOMUX_SUSPENDED] = &sdc,
		},
	},
	{ /* SDC1_DATA5 */
		.gpio = 164,
		.settings = {
			[GPIOMUX_ACTIVE] = &sdc,
			[GPIOMUX_SUSPENDED] = &sdc,
		},
	},
	{ /* SDC1_DATA6 */
		.gpio = 165,
		.settings = {
			[GPIOMUX_ACTIVE] = &sdc,
			[GPIOMUX_SUSPENDED] = &sdc,
		},
	},
	{ /* SDC1_DATA7 */
		.gpio = 166,
		.settings = {
			[GPIOMUX_ACTIVE] = &sdc,
			[GPIOMUX_SUSPENDED] = &sdc,
		},
	},
	{ /* SDC1_CLK */
		.gpio = 167,
		.settings = {
			[GPIOMUX_ACTIVE] = &sdc_clk,
			[GPIOMUX_SUSPENDED] = &sdc_clk,
		},
	},
	{ /* SDC1_CMD */
		.gpio = 168,
		.settings = {
			[GPIOMUX_ACTIVE] = &sdc,
			[GPIOMUX_SUSPENDED] = &sdc,
		},
	},
	{ /* HDMI_CEC */
		.gpio = 169,
		.settings = {
			[GPIOMUX_ACTIVE] = &hdmi,
			[GPIOMUX_SUSPENDED] = &hdmi,
		},
	},
	{ /* HDMI_DDC_CLOCK */
		.gpio = 170,
		.settings = {
			[GPIOMUX_ACTIVE] = &hdmi,
			[GPIOMUX_SUSPENDED] = &hdmi,
		},
	},
	{ /* HDMI_DDC_DATA */
		.gpio = 171,
		.settings = {
			[GPIOMUX_ACTIVE] = &hdmi,
			[GPIOMUX_SUSPENDED] = &hdmi,
		},
	},
	{ /* HDMI_HOT_PLUG_DETECT */
		.gpio = 172,
		.settings = {
			[GPIOMUX_ACTIVE] = &hdmi,
			[GPIOMUX_SUSPENDED] = &hdmi,
		},
	},
};

struct msm_gpiomux_configs
semc_fuji_gpiomux_cfgs[] __initdata = {
	{semc_fuji_all_cfgs, ARRAY_SIZE(semc_fuji_all_cfgs)},
	{NULL, 0},
};

static int pm8058_unused_gpio[] = {
	6, 7, 8, 14, 15, 16, 18, 19, 24, 25, 33, 34
};

struct pmic8058_unused_gpio pmic8058_unused_gpios = {
	.unused_gpio = pm8058_unused_gpio,
	.unused_gpio_num = ARRAY_SIZE(pm8058_unused_gpio),
};
