/* Copyright (c) 2012-2013, The Linux Foundation. All rights reserved.
 * Copyright (C) 2013 Sony Mobile Communications Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/init.h>
#include <mach/board.h>
#include <mach/gpio.h>
#include <mach/gpiomux.h>
#include "board-sony_shinano-hw.h"
#include "board-sony_sirius-gpiomux-diff.h"
#include "sony_gpiomux.h"

static struct gpiomux_setting unused_gpio = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir  = GPIOMUX_OUT_LOW,
};

static struct gpiomux_setting gpio_2ma_no_pull_out_low = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir  = GPIOMUX_OUT_LOW,
};

static struct gpiomux_setting gpio_2ma_no_pull_out_high = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir  = GPIOMUX_OUT_HIGH,
};

static struct gpiomux_setting gpio_2ma_no_pull_in = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir  = GPIOMUX_IN,
};

static struct gpiomux_setting gpio_2ma_pull_down_in = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir  = GPIOMUX_IN,
};

static struct gpiomux_setting gpio_2ma_pull_up_in = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
	.dir  = GPIOMUX_IN,
};

static struct gpiomux_setting gpio_follow_qct = {
	.func = GPIOMUX_FOLLOW_QCT,
	.drv  = GPIOMUX_FOLLOW_QCT,
	.pull = GPIOMUX_FOLLOW_QCT,
	.dir  = GPIOMUX_FOLLOW_QCT,
};

static struct gpiomux_setting gpio_2ma_follow_qct = {
	.func = GPIOMUX_FOLLOW_QCT,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_FOLLOW_QCT,
	.dir  = GPIOMUX_FOLLOW_QCT,
};

static struct gpiomux_setting mhl_spi = {
	.func = GPIOMUX_FUNC_1,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting debug_uart_tx = {
	.func = GPIOMUX_FUNC_2,
	.drv  = GPIOMUX_DRV_4MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting debug_uart_rx = {
	.func = GPIOMUX_FUNC_2,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
};

static struct gpiomux_setting cam_mclk = {
	.func = GPIOMUX_FUNC_1,
	.drv  = GPIOMUX_DRV_6MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting cam_i2c = {
	.func = GPIOMUX_FUNC_1,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting nfc_i2c = {
	.func = GPIOMUX_FUNC_3,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting wlan_sdio_active = {
	.func = GPIOMUX_FUNC_2,
	.drv  = GPIOMUX_DRV_10MA,
	.pull = GPIOMUX_PULL_UP,
};

static struct gpiomux_setting wlan_sdio_suspend = {
	.func = GPIOMUX_FUNC_2,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
};

static struct gpiomux_setting wlan_sdio_clk_active = {
	.func = GPIOMUX_FUNC_2,
	.drv  = GPIOMUX_DRV_10MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting wlan_sdio_clk_suspend = {
	.func = GPIOMUX_FUNC_2,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting bt_uart_tx = {
	.func = GPIOMUX_FUNC_3,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting bt_uart_rx = {
	.func = GPIOMUX_FUNC_3,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
};

static struct gpiomux_setting bt_uart_cts = {
	.func = GPIOMUX_FUNC_3,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
};

static struct gpiomux_setting bt_uart_rts = {
	.func = GPIOMUX_FUNC_3,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting ts_i2c = {
	.func = GPIOMUX_FUNC_3,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting damp_i2s = {
	.func = GPIOMUX_FUNC_1,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting bt_pcm = {
	.func = GPIOMUX_FUNC_1,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting peripheral_i2c = {
	.func = GPIOMUX_FUNC_3,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting sensors_i2c = {
	.func = GPIOMUX_FUNC_3,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting hsic = {
	.func = GPIOMUX_FUNC_1,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct msm_gpiomux_config shinano_all_configs[] __initdata = {
	{ /* MHL_SPI_MOSI */
		.gpio = 0,
		.settings = {
			[GPIOMUX_ACTIVE]    = &mhl_spi,
			[GPIOMUX_SUSPENDED] = &mhl_spi,
		},
	},
	{ /* MHL_SPI_MISO */
		.gpio = 1,
		.settings = {
			[GPIOMUX_ACTIVE]    = &mhl_spi,
			[GPIOMUX_SUSPENDED] = &mhl_spi,
		},
	},
	{ /* MHL_SPI_CS_N */
		.gpio = 2,
		.settings = {
			[GPIOMUX_ACTIVE]    = &mhl_spi,
			[GPIOMUX_SUSPENDED] = &mhl_spi,
		},
	},
	{ /* MHL_SPI_CLK */
		.gpio = 3,
		.settings = {
			[GPIOMUX_ACTIVE]    = &mhl_spi,
			[GPIOMUX_SUSPENDED] = &mhl_spi,
		},
	},
	{ /* UART_TX_DFMS */
		.gpio = 4,
		.settings = {
			[GPIOMUX_ACTIVE]    = &debug_uart_tx,
			[GPIOMUX_SUSPENDED] = &debug_uart_tx,
		},
	},
	{ /* UART_RX_DTMS */
		.gpio = 5,
		.settings = {
			[GPIOMUX_ACTIVE]    = &debug_uart_rx,
			[GPIOMUX_SUSPENDED] = &debug_uart_rx,
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
	{ /* SW_SERVICE */
		.gpio = 8,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_pull_up_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_pull_up_in,
		},
	},
	{ /* UIM1_DETECT */
		.gpio = 9,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_no_pull_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_in,
		},
	},
	{ /* MHL_SWITCH_SEL_1 */
		.gpio = 10,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_no_pull_out_low,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_out_low,
		},
	},
	{ /* MHL_SWITCH_SEL_2 */
		.gpio = 11,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_no_pull_out_low,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_out_low,
		},
	},
	{ /* NC */
		.gpio = 12,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* NC */
		.gpio = 13,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* NC */
		.gpio = 14,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* CAM0_MCLK0 */
		.gpio = 15,
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_mclk,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_out_low,
		},
	},
	{ /* MHL_RST_N */
		.gpio = 16,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_no_pull_out_low,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_out_low,
		},
	},
	{ /* CAM1_MCLK2 */
		.gpio = 17,
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_mclk,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_out_low,
		},
	},
	{ /* CHATCAM_RESET_N */
		.gpio = 18,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_no_pull_out_low,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_out_low,
		},
	},
	{ /* CAM0_I2C_SDA0 */
		.gpio = 19,
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_i2c,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_pull_down_in,
		},
	},
	{ /* CAM0_I2C_SCL0 */
		.gpio = 20,
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_i2c,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_pull_down_in,
		},
	},
	{ /* CAM1_I2C_SDA1 */
		.gpio = 21,
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_i2c,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_pull_down_in,
		},
	},
	{ /* CAM1_I2C_SCL1 */
		.gpio = 22,
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_i2c,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_pull_down_in,
		},
	},
	{ /* MHL_PWR_EN */
		.gpio = 23,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_no_pull_out_low,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_out_low,
		},
	},
	{ /* NFC_IRQ_FELICA_INT_N */
		.gpio = 24,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_no_pull_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_in,
		},
	},
	{ /* PON_VOLTAGE_SEL */
		.gpio = 25,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_no_pull_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_in,
		},
	},
	{ /* LCD_ID */
		.gpio = 26,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_pull_up_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_pull_up_in,
		},
	},
	{ /* NC */
		.gpio = 27,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* NC */
		.gpio = 28,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* NFC_I2C_SDA */
		.gpio = 29,
		.settings = {
			[GPIOMUX_ACTIVE]    = &nfc_i2c,
			[GPIOMUX_SUSPENDED] = &nfc_i2c,
		},
	},
	{ /* NFC_I2C_SCL */
		.gpio = 30,
		.settings = {
			[GPIOMUX_ACTIVE]    = &nfc_i2c,
			[GPIOMUX_SUSPENDED] = &nfc_i2c,
		},
	},
	{ /* MHL_FW_WAKE */
		.gpio = 31,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_no_pull_out_low,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_out_low,
		},
	},
	{ /* HDMI_DDC_CLK */
		.gpio = 32,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_follow_qct,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_follow_qct,
		},
	},
	{ /* HDMI_DDC_DATA */
		.gpio = 33,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_follow_qct,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_follow_qct,
		},
	},
	{ /* HDMI_HOT_PLUG_DET */
		.gpio = 34,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_follow_qct,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_follow_qct,
		},
	},
	{ /* WLAN_SDIO_DATA_3 */
		.gpio = 35,
		.settings = {
			[GPIOMUX_ACTIVE]    = &wlan_sdio_active,
			[GPIOMUX_SUSPENDED] = &wlan_sdio_suspend,
		},
	},
	{ /* WLAN_SDIO_DATA_2 */
		.gpio = 36,
		.settings = {
			[GPIOMUX_ACTIVE]    = &wlan_sdio_active,
			[GPIOMUX_SUSPENDED] = &wlan_sdio_suspend,
		},
	},
	{ /* WLAN_SDIO_DATA_1 */
		.gpio = 37,
		.settings = {
			[GPIOMUX_ACTIVE]    = &wlan_sdio_active,
			[GPIOMUX_SUSPENDED] = &wlan_sdio_suspend,
		},
	},
	{ /* WLAN_SDIO_DATA_0 */
		.gpio = 38,
		.settings = {
			[GPIOMUX_ACTIVE]    = &wlan_sdio_active,
			[GPIOMUX_SUSPENDED] = &wlan_sdio_suspend,
		},
	},
	{ /* WLAN_SDIO_CMD */
		.gpio = 39,
		.settings = {
			[GPIOMUX_ACTIVE]    = &wlan_sdio_active,
			[GPIOMUX_SUSPENDED] = &wlan_sdio_suspend,
		},
	},
	{ /* WLAN_SDIO_CLK */
		.gpio = 40,
		.settings = {
			[GPIOMUX_ACTIVE]    = &wlan_sdio_clk_active,
			[GPIOMUX_SUSPENDED] = &wlan_sdio_clk_suspend,
		},
	},
	{ /* BT_UART_TX */
		.gpio = 41,
		.settings = {
			[GPIOMUX_ACTIVE]    = &bt_uart_tx,
			[GPIOMUX_SUSPENDED] = &bt_uart_tx,
		},
	},
	{ /* BT_UART_RX */
		.gpio = 42,
		.settings = {
			[GPIOMUX_ACTIVE]    = &bt_uart_rx,
			[GPIOMUX_SUSPENDED] = &bt_uart_rx,
		},
	},
	{ /* BT_UART_CTS_N */
		.gpio = 43,
		.settings = {
			[GPIOMUX_ACTIVE]    = &bt_uart_cts,
			[GPIOMUX_SUSPENDED] = &bt_uart_cts,
		},
	},
	{ /* BT_UART_RTS_N */
		.gpio = 44,
		.settings = {
			[GPIOMUX_ACTIVE]    = &bt_uart_rts,
			[GPIOMUX_SUSPENDED] = &bt_uart_rts,
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
	{ /* TS_I2C_SDA */
		.gpio = 47,
		.settings = {
			[GPIOMUX_ACTIVE]    = &ts_i2c,
			[GPIOMUX_SUSPENDED] = &ts_i2c,
		},
	},
	{ /* TS_I2C_SCL */
		.gpio = 48,
		.settings = {
			[GPIOMUX_ACTIVE]    = &ts_i2c,
			[GPIOMUX_SUSPENDED] = &ts_i2c,
		},
	},
	{ /* NC(UIM2_DATA) */
		.gpio = 49,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_no_pull_out_low,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_pull_down_in,
		},
	},
	{ /* NC(UIM2_CLK) */
		.gpio = 50,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_no_pull_out_low,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_pull_down_in,
		},
	},
	{ /* NC(UIM2_RST) */
		.gpio = 51,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_no_pull_out_low,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_pull_down_in,
		},
	},
	{ /* NC(UIM2_DETECT) */
		.gpio = 52,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_no_pull_out_low,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_pull_down_in,
		},
	},
	{ /* NC */
		.gpio = 53,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* NC */
		.gpio = 54,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* DEBUG_GPIO0 */
		.gpio = 55,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_no_pull_out_low,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_out_low,
		},
	},
	{ /* DEBUG_GPIO1 */
		.gpio = 56,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_no_pull_out_low,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_out_low,
		},
	},
	{ /* NFC_DWLD_EN */
		.gpio = 57,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_no_pull_out_low,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_out_low,
		},
	},
	{ /* DAMP_I2S_SCLK */
		.gpio = 58,
		.settings = {
			[GPIOMUX_ACTIVE]    = &damp_i2s,
			[GPIOMUX_SUSPENDED] = &damp_i2s,
		},
	},
	{ /* DAMP_I2S_WS */
		.gpio = 59,
		.settings = {
			[GPIOMUX_ACTIVE]    = &damp_i2s,
			[GPIOMUX_SUSPENDED] = &damp_i2s,
		},
	},
	{ /* DAMP_I2S_D0 */
		.gpio = 60,
		.settings = {
			[GPIOMUX_ACTIVE]    = &damp_i2s,
			[GPIOMUX_SUSPENDED] = &damp_i2s,
		},
	},
	{ /* DAMP_I2S_D1 */
		.gpio = 61,
		.settings = {
			[GPIOMUX_ACTIVE]    = &damp_i2s,
			[GPIOMUX_SUSPENDED] = &damp_i2s,
		},
	},
	{ /* SD_CARD_DET_N */
		.gpio = 62,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_no_pull_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_in,
		},
	},
	{ /* CODEC_RESET_N */
		.gpio = 63,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_follow_qct,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_follow_qct,
		},
	},
	{ /* MHL_INT */
		.gpio = 64,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_pull_up_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_pull_up_in,
		},
	},
	{ /* ACCEL_INT2 */
		.gpio = 65,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_no_pull_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_in,
		},
	},
	{ /* GYRO_INT1 */
		.gpio = 66,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_pull_down_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_pull_down_in,
		},
	},
	{ /* WL_HOST_WAKE */
		.gpio = 67,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_pull_down_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_pull_down_in,
		},
	},
	{ /* NC */
		.gpio = 68,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* NC */
		.gpio = 69,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* SLIMBUS_CLK */
		.gpio = 70,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_follow_qct,
			[GPIOMUX_SUSPENDED] = &gpio_follow_qct,
		},
	},
	{ /* SLIMBUS_DATA */
		.gpio = 71,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_follow_qct,
			[GPIOMUX_SUSPENDED] = &gpio_follow_qct,
		},
	},
	{ /* CODEC_INT1_N */
		.gpio = 72,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_follow_qct,
			[GPIOMUX_SUSPENDED] = &gpio_follow_qct,
		},
	},
	{ /* ACCEL_INT1 */
		.gpio = 73,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_no_pull_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_in,
		},
	},
	{ /* PROX_ALS_INT_N */
		.gpio = 74,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_pull_up_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_pull_up_in,
		},
	},
	{ /* GYRO_INT2 */
		.gpio = 75,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_pull_down_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_pull_down_in,
		},
	},
	{ /* NC */
		.gpio = 76,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* NC */
		.gpio = 77,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* NC */
		.gpio = 78,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* BT_PCM_SCLK */
		.gpio = 79,
		.settings = {
			[GPIOMUX_ACTIVE]    = &bt_pcm,
			[GPIOMUX_SUSPENDED] = &bt_pcm,
		},
	},
	{ /* BT_PCM_SYNC */
		.gpio = 80,
		.settings = {
			[GPIOMUX_ACTIVE]    = &bt_pcm,
			[GPIOMUX_SUSPENDED] = &bt_pcm,
		},
	},
	{ /* BT_PCM_DIN */
		.gpio = 81,
		.settings = {
			[GPIOMUX_ACTIVE]    = &bt_pcm,
			[GPIOMUX_SUSPENDED] = &bt_pcm,
		},
	},
	{ /* BT_PCM_DOUT */
		.gpio = 82,
		.settings = {
			[GPIOMUX_ACTIVE]    = &bt_pcm,
			[GPIOMUX_SUSPENDED] = &bt_pcm,
		},
	},
	{ /* PERIPHERAL_I2C_SDA */
		.gpio = 83,
		.settings = {
			[GPIOMUX_ACTIVE]    = &peripheral_i2c,
			[GPIOMUX_SUSPENDED] = &peripheral_i2c,
		},
	},
	{ /* PERIPHERAL_I2C_SCL */
		.gpio = 84,
		.settings = {
			[GPIOMUX_ACTIVE]    = &peripheral_i2c,
			[GPIOMUX_SUSPENDED] = &peripheral_i2c,
		},
	},
	{ /* TP_RESET */
		.gpio = 85,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_no_pull_out_high,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_out_high,
		},
	},
	{ /* TS_INT_N */
		.gpio = 86,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_no_pull_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_in,
		},
	},
	{ /* SENSORS_I2C_SDA */
		.gpio = 87,
		.settings = {
			[GPIOMUX_ACTIVE]    = &sensors_i2c,
			[GPIOMUX_SUSPENDED] = &sensors_i2c,
		},
	},
	{ /* SENSORS_I2C_SCL */
		.gpio = 88,
		.settings = {
			[GPIOMUX_ACTIVE]    = &sensors_i2c,
			[GPIOMUX_SUSPENDED] = &sensors_i2c,
		},
	},
	{ /* NC */
		.gpio = 89,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* NC */
		.gpio = 90,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* NC */
		.gpio = 91,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* NC */
		.gpio = 92,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* CODEC_INT2_N */
		.gpio = 93,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_follow_qct,
			[GPIOMUX_SUSPENDED] = &gpio_follow_qct,
		},
	},
	{ /* CAM0_RST_N */
		.gpio = 94,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_no_pull_out_low,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_out_low,
		},
	},
	{ /* BT_HOST_WAKE */
		.gpio = 95,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_no_pull_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_pull_down_in,
		},
	},
	{ /* BT_DEV_WAKE */
		.gpio = 96,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_no_pull_out_low,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_out_low,
		},
	},
	{ /* UIM1_DATA */
		.gpio = 97,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_follow_qct,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_follow_qct,
		},
	},
	{ /* UIM1_CLK */
		.gpio = 98,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_follow_qct,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_follow_qct,
		},
	},
	{ /* UIM1_RST */
		.gpio = 99,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_follow_qct,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_follow_qct,
		},
	},
	{ /* UIM_HOT_SWAP */
		.gpio = 100,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_pull_up_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_pull_down_in,
		},
	},
	{ /* BATT_REM_ALARM */
		.gpio = 101,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_follow_qct,
			[GPIOMUX_SUSPENDED] = &gpio_follow_qct,
		},
	},
	{ /* MHL_SPI_DVLD */
		.gpio = 102,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_no_pull_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_pull_down_in,
		},
	},
	{ /* FORCED_USB_BOOT */
		.gpio = 103,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_pull_down_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_pull_down_in,
		},
	},
	{ /* NC */
		.gpio = 104,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* NC */
		.gpio = 105,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* NC */
		.gpio = 106,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* NC */
		.gpio = 107,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* SW_TX_LB4 */
		.gpio = 108,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_follow_qct,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_follow_qct,
		},
	},
	{ /* NC */
		.gpio = 109,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* NC */
		.gpio = 110,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* NC */
		.gpio = 111,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* GRFC8 [WDOG_DISABLE] */
		.gpio = 112,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_follow_qct,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_follow_qct,
		},
	},
	{ /* NC [BOOT_CONFIG_1] */
		.gpio = 113,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* NC [BOOT_CONFIG_2] */
		.gpio = 114,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* NC [BOOT_CONFIG_3] */
		.gpio = 115,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* TX_GTR_THRES [BOOT_CONFIG_4] */
		.gpio = 116,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_follow_qct,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_follow_qct,
		},
	},
	{ /* NC(Reserved for ANT_TUNE0) */
		.gpio = 117,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* SW_WB_CPL */
		.gpio = 118,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_follow_qct,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_follow_qct,
		},
	},
	{ /* NC */
		.gpio = 119,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* SW_PRX_LB3 */
		.gpio = 120,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_follow_qct,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_follow_qct,
		},
	},
	{ /* SW_PRX_LB41 */
		.gpio = 121,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_follow_qct,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_follow_qct,
		},
	},
	{ /* NC */
		.gpio = 122,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* NC */
		.gpio = 123,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* NC */
		.gpio = 124,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* NC */
		.gpio = 125,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* NC */
		.gpio = 126,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* NC */
		.gpio = 127,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* EXT_GPS_LNA_EN */
		.gpio = 128,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_follow_qct,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_follow_qct,
		},
	},
	{ /* NC */
		.gpio = 129,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* LTE_ACTIVE */
		.gpio = 130,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_no_pull_out_low,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_out_low,
		},
	},
	{ /* LTE_TX_COEX_WCN */
		.gpio = 131,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_no_pull_out_low,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_out_low,
		},
	},
	{ /* WCN_TX_COEX_LTE */
		.gpio = 132,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_pull_down_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_pull_down_in,
		},
	},
	{ /* WTR_SSBI1_TX_GPS */
		.gpio = 133,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_follow_qct,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_follow_qct,
		},
	},
	{ /* WTR_SSBI2_PRX_DRX */
		.gpio = 134,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_follow_qct,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_follow_qct,
		},
	},
	{ /* NC */
		.gpio = 135,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* WFR_SSBI */
		.gpio = 136,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_follow_qct,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_follow_qct,
		},
	},
	{ /* NC */
		.gpio = 137,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* GSM_TX_PHASE_D1 */
		.gpio = 138,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_follow_qct,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_follow_qct,
		},
	},
	{ /* GSM_TX_PHASE_D0 */
		.gpio = 139,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_follow_qct,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_follow_qct,
		},
	},
	{ /* RFFE1_CLK */
		.gpio = 140,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_follow_qct,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_follow_qct,
		},
	},
	{ /* RFFE1_DATA */
		.gpio = 141,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_follow_qct,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_follow_qct,
		},
	},
	{ /* RFFE2_CLK */
		.gpio = 142,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_follow_qct,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_follow_qct,
		},
	},
	{ /* RFFE2_DATA */
		.gpio = 143,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_follow_qct,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_follow_qct,
		},
	},
	{ /* HSIC_STROBE */
		.gpio = 144,
		.settings = {
			[GPIOMUX_ACTIVE]    = &hsic,
			[GPIOMUX_SUSPENDED] = &hsic,
		},
	},
	{ /* HSIC_DATA */
		.gpio = 145,
		.settings = {
			[GPIOMUX_ACTIVE]    = &hsic,
			[GPIOMUX_SUSPENDED] = &hsic,
		},
	},
};

void __init msm_8974_init_gpiomux(void)
{
	int rc, hw;
	struct msm_gpiomux_configs base;

	base.cfg = shinano_all_configs;
	base.ncfg = ARRAY_SIZE(shinano_all_configs);

	hw = get_sony_hw();

	if (hw == HW_SIRIUS_SAMBA)
		overwrite_configs(&base, &sirius_samba_gpiomux_cfgs);

	rc = sony_init_gpiomux(shinano_all_configs,
			ARRAY_SIZE(shinano_all_configs));
	if (rc) {
		pr_err("%s failed %d\n", __func__, rc);
		return;
	}
}
