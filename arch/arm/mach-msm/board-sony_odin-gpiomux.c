/* Copyright (c) 2011-2012, Code Aurora Forum. All rights reserved.
 * Copyright (C) 2012 Sony Mobile Communications AB.
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
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/bootmem.h>
#include <linux/gpio.h>
#include <asm/mach-types.h>
#include <asm/mach/mmc.h>
#include <mach/msm_bus_board.h>
#include <mach/board.h>
#include <mach/gpiomux.h>
#include <mach/socinfo.h>
#include "devices.h"
#include "board-8064.h"
#include "board-sony_fusion3.h"


static struct gpiomux_setting cdc_mclk = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting wcnss_5wire_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
};

static struct gpiomux_setting wcnss_5wire_active_cfg = {
	.func = GPIOMUX_FUNC_1,
	.drv  = GPIOMUX_DRV_6MA,
	.pull = GPIOMUX_PULL_DOWN,
};


static struct gpiomux_setting slimbus = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_KEEPER,
};

static struct gpiomux_setting hdmi_suspend_1_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting hdmi_active_1_cfg = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting hdmi_suspend_2_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct gpiomux_setting hdmi_active_2_cfg = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct gpiomux_setting rmi4_int_act_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_IN,
};

#ifdef CONFIG_USB_EHCI_MSM_HSIC
static struct gpiomux_setting hsic_act_cfg = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_10MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting hsic_sus_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_OUT_LOW,
};

static struct gpiomux_setting hsic_wakeup_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_IN,
};

static struct msm_gpiomux_config apq8064_hsic_configs[] __initdata = {
	{
		.gpio = 88,               /*HSIC_STROBE */
		.settings = {
			[GPIOMUX_ACTIVE] = &hsic_act_cfg,
			[GPIOMUX_SUSPENDED] = &hsic_sus_cfg,
		},
	},
	{
		.gpio = 89,               /* HSIC_DATA */
		.settings = {
			[GPIOMUX_ACTIVE] = &hsic_act_cfg,
			[GPIOMUX_SUSPENDED] = &hsic_sus_cfg,
		},
	},
	{
		.gpio = 47,              /* wake up */
		.settings = {
			[GPIOMUX_ACTIVE] = &hsic_wakeup_cfg,
			[GPIOMUX_SUSPENDED] = &hsic_wakeup_cfg,
		},
	},
};
#endif

static struct msm_gpiomux_config rmi4_gpio_configs[] __initdata = {
	{	/* TS ATTENTION INTERRUPT */
		.gpio = 6,
		.settings = {
			[GPIOMUX_ACTIVE]    = &rmi4_int_act_cfg,
			[GPIOMUX_SUSPENDED] = &rmi4_int_act_cfg,
		},
	},
};

static struct msm_gpiomux_config apq8064_slimbus_config[] __initdata = {
	{
		.gpio   = 40,           /* slimbus clk */
		.settings = {
			[GPIOMUX_SUSPENDED] = &slimbus,
		},
	},
	{
		.gpio   = 41,           /* slimbus data */
		.settings = {
			[GPIOMUX_SUSPENDED] = &slimbus,
		},
	},
};

static struct msm_gpiomux_config apq8064_audio_codec_configs[] __initdata = {
	{
		.gpio = 39,
		.settings = {
			[GPIOMUX_SUSPENDED] = &cdc_mclk,
		},
	},
};

static struct gpiomux_setting ap2mdm_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct gpiomux_setting mdm2ap_status_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct gpiomux_setting mdm2ap_errfatal_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_16MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct gpiomux_setting mdm2ap_pblrdy = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_16MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct gpiomux_setting ap2mdm_pon_reset_n_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct gpiomux_setting ap2mdm_wakeup = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct msm_gpiomux_config mdm_configs[] __initdata = {
	/* AP2MDM_STATUS */
	{
		.gpio = 48,
		.settings = {
			[GPIOMUX_SUSPENDED] = &ap2mdm_cfg,
		}
	},
	/* MDM2AP_STATUS */
	{
		.gpio = 49,
		.settings = {
			[GPIOMUX_SUSPENDED] = &mdm2ap_status_cfg,
		}
	},
	/* MDM2AP_ERRFATAL */
	{
		.gpio = 19,
		.settings = {
			[GPIOMUX_SUSPENDED] = &mdm2ap_errfatal_cfg,
		}
	},
	/* AP2MDM_ERRFATAL */
	{
		.gpio = 18,
		.settings = {
			[GPIOMUX_SUSPENDED] = &ap2mdm_cfg,
		}
	},
	/* AP2MDM_PON_RESET_N */
	{
		.gpio = 27,
		.settings = {
			[GPIOMUX_SUSPENDED] = &ap2mdm_pon_reset_n_cfg,
		}
	},
	/* AP2MDM_WAKEUP */
	{
		.gpio = 35,
		.settings = {
			[GPIOMUX_SUSPENDED] = &ap2mdm_wakeup,
		}
	},
	/* MDM2AP_PBL_READY*/
	{
		.gpio = 46,
		.settings = {
			[GPIOMUX_SUSPENDED] = &mdm2ap_pblrdy,
		}
	},
};

static struct msm_gpiomux_config wcnss_5wire_interface[] __initdata = {
	{
		.gpio = 64,
		.settings = {
			[GPIOMUX_ACTIVE]    = &wcnss_5wire_active_cfg,
			[GPIOMUX_SUSPENDED] = &wcnss_5wire_suspend_cfg,
		},
	},
	{
		.gpio = 65,
		.settings = {
			[GPIOMUX_ACTIVE]    = &wcnss_5wire_active_cfg,
			[GPIOMUX_SUSPENDED] = &wcnss_5wire_suspend_cfg,
		},
	},
	{
		.gpio = 66,
		.settings = {
			[GPIOMUX_ACTIVE]    = &wcnss_5wire_active_cfg,
			[GPIOMUX_SUSPENDED] = &wcnss_5wire_suspend_cfg,
		},
	},
	{
		.gpio = 67,
		.settings = {
			[GPIOMUX_ACTIVE]    = &wcnss_5wire_active_cfg,
			[GPIOMUX_SUSPENDED] = &wcnss_5wire_suspend_cfg,
		},
	},
	{
		.gpio = 68,
		.settings = {
			[GPIOMUX_ACTIVE]    = &wcnss_5wire_active_cfg,
			[GPIOMUX_SUSPENDED] = &wcnss_5wire_suspend_cfg,
		},
	},
};

/* Best setup for APQ GPIO pin from a power consumption perspective */
/* According to QCT case 00818101 */
static struct gpiomux_setting gpio_unused_input = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir  = GPIOMUX_IN,
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

static struct gpiomux_setting gpio_2ma_pull_up_in = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
	.dir  = GPIOMUX_IN,
};

static struct gpiomux_setting gpio_2ma_pull_down_in = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir  = GPIOMUX_IN,
};

static struct gpiomux_setting cam_mclk0 = {
	.func = GPIOMUX_FUNC_1,
	.drv  = GPIOMUX_DRV_6MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting cam_mclk1 = {
	.func = GPIOMUX_FUNC_2,
	.drv  = GPIOMUX_DRV_6MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting gsbi1_i2c_cfg = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_4MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting gsbi2_i2c_cfg = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting gsbi3_i2c_cfg = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_4MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting gsbi4_cam_i2c_sda_l = {
	.func = GPIOMUX_FUNC_9,
	.drv  = GPIOMUX_DRV_4MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting gsbi4_cam_i2c_scl_l = {
	.func = GPIOMUX_FUNC_A,
	.drv  = GPIOMUX_DRV_4MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting gsbi4_cam_i2c_r = {
	.func = GPIOMUX_FUNC_6,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting gsbi5_debug_uart_tx = {
	.func = GPIOMUX_FUNC_2,
	.drv = GPIOMUX_DRV_4MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting gsbi5_debug_uart_rx = {
	.func = GPIOMUX_FUNC_2,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
};

static struct gpiomux_setting gsbi2_ir_uart_tx = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting gsbi2_ir_uart_rx = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct msm_gpiomux_config apq8064_all_other_configs[] __initdata = {
	{ /* RESET_STATUS_N */
		.gpio = 0,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_no_pull_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_in,
		},
	},
	{ /* FLASH_TRG */
		.gpio = 1,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_pull_down_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_pull_down_in,
		},
	},
	{ /* BOOT_FROM_ROM, used only by S1Boot */
		.gpio = 2,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_unused_input,
			[GPIOMUX_SUSPENDED] = &gpio_unused_input,
		},
	},
	{ /* FLASH_DR_RST_N */
		.gpio = 3,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_no_pull_out_low,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_out_low,
		},
	},
	{ /* CAM_MCLK1 */
		.gpio = 4,
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_mclk1,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_out_low,
		},
	},
	{ /* CAM_MCLK0 */
		.gpio = 5,
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_mclk0,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_out_low,
		},
	},

	/* GPIO6 handled by rmi4_gpio_configs */

	{ /* COMPASS_INT */
		.gpio = 7,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_pull_down_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_pull_down_in,
		},
	},
	{ /* I2C3_DATA_TS */
		.gpio = 8,			/* GSBI3 I2C QUP SDA */
		.settings = {
			[GPIOMUX_ACTIVE]    = &gsbi3_i2c_cfg,
			[GPIOMUX_SUSPENDED] = &gsbi3_i2c_cfg,
		},
	},
	{ /* I2C3_CLK_TS */
		.gpio = 9,			/* GSBI3 I2C QUP SCL */
		.settings = {
			[GPIOMUX_ACTIVE]    = &gsbi3_i2c_cfg,
			[GPIOMUX_SUSPENDED] = &gsbi3_i2c_cfg,
		},
	},
	{ /* I2C4_DATA_CAM */
		.gpio = 10,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gsbi4_cam_i2c_sda_l,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_in,
		},
	},
	{ /* I2C4_CLK_CAM */
		.gpio = 11,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gsbi4_cam_i2c_scl_l,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_in,
		},
	},
	{ /* I2C4_DATA_CHAT_CAM */
		.gpio = 12,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gsbi4_cam_i2c_r,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_in,
		},
	},
	{ /* I2C4_CLK_CHAT_CAM */
		.gpio = 13,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gsbi4_cam_i2c_r,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_in,
		},
	},

	/* GPIO14-17 handled by QCT according to HW Config Spec */
	/* GPIO18-19 handled by QCT, configured in mdm_configs */

	{ /* I2C1_DATA_APPS */
		.gpio = 20,			/* GSBI1 QUP I2C_DATA */
		.settings = {
			[GPIOMUX_ACTIVE]    = &gsbi1_i2c_cfg,
			[GPIOMUX_SUSPENDED] = &gsbi1_i2c_cfg,
		},
	},
	{ /* I2C1_CLK_APPS */
		.gpio = 21,			/* GSBI1 QUP I2C_CLK */
		.settings = {
			[GPIOMUX_ACTIVE]    = &gsbi1_i2c_cfg,
			[GPIOMUX_SUSPENDED] = &gsbi1_i2c_cfg,
		},
	},
	{ /* IR remote UART TX */
		.gpio = 22,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gsbi2_ir_uart_tx,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_out_high,
		},
	},
	{ /* IR remote UART RX */
		.gpio = 23,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gsbi2_ir_uart_rx,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_pull_up_in,
		},
	},
	{ /* I2C2_DATA_SENS */
		.gpio = 24,			/* GSBI2 QUP I2C_DATA */
		.settings = {
			[GPIOMUX_ACTIVE]    = &gsbi2_i2c_cfg,
			[GPIOMUX_SUSPENDED] = &gsbi2_i2c_cfg,
		},
	},
	{ /* I2C2_CLK_SENS */
		.gpio = 25,			/* GSBI2 QUP I2C_CLK */
		.settings = {
			[GPIOMUX_ACTIVE]    = &gsbi2_i2c_cfg,
			[GPIOMUX_SUSPENDED] = &gsbi2_i2c_cfg,
		},
	},
	{ /* SD_CARD_DET_N */
		.gpio = 26,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_no_pull_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_in,
		},
	},

	/* GPIO27 handled by QCT, configured in mdm_configs */

	{ /* GYRO/ACC_INT_N */
		.gpio = 28,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_no_pull_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_pull_down_in,
		},
	},
	{ /* NFC_IRQ_FELICA_INT */
		.gpio = 29,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_no_pull_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_in,
		},
	},

	/* GPIO30 handled by QCT according to HW Config Spec */

	{ /* SW_SERVICE */
		.gpio = 31,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_pull_up_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_pull_up_in,
		},
	},
	{ /* NC */
		.gpio = 32,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_unused_input,
			[GPIOMUX_SUSPENDED] = &gpio_unused_input,
		},
	},
	{ /* NC */
		.gpio = 33,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_unused_input,
			[GPIOMUX_SUSPENDED] = &gpio_unused_input,
		},
	},
	{ /* CHAT_CAM_RST_N */
		.gpio = 34,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_no_pull_out_low,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_out_low,
		},
	},

	/* GPIO35 handled by QCT according to HW Config Spec */

	{ /* NC */
		.gpio = 36,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_unused_input,
			[GPIOMUX_SUSPENDED] = &gpio_unused_input,
		},
	},
	{ /* NC */
		.gpio = 37,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_unused_input,
			[GPIOMUX_SUSPENDED] = &gpio_unused_input,
		},
	},
	{ /* MHL_INT */
		.gpio = 38,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_pull_up_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_pull_up_in,
		},
	},

	/* GPIO39 handled by QCT, configured in apq8064_audio_codec_configs */
	/* GPIO40-41 handled by QCT, configured in apq8064_slimbus_config */
	/* GPIO42 handled by QCT according to HW Config Spec */

	{ /* VIB_DETECT */
		.gpio = 43,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_no_pull_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_in,
		},
	},
	{ /* NC */
		.gpio = 44,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_unused_input,
			[GPIOMUX_SUSPENDED] = &gpio_unused_input,
		},
	},
	{ /* PROX_INT_N */
		.gpio = 45,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_pull_up_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_pull_up_in,
		},
	},

	/* GPIO46 handled by QCT according to HW Config Spec */
	/* GPIO47 handled by QCT according to HW Config Spec */
	/* GPIO48-49 handled by QCT, configured in mdm_configs */

	{ /* HW_SERVICE (BOOT_CONFIG_1) */
		.gpio = 50,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_no_pull_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_in,
		},
	},
	{ /* UART_TX_DFMS */
		.gpio = 51,			/* GSBI5 UART TX */
		.settings = {
			[GPIOMUX_ACTIVE]    = &gsbi5_debug_uart_tx,
			[GPIOMUX_SUSPENDED] = &gsbi5_debug_uart_tx,
		},
	},
	{ /* UART_RX_DTMS */
		.gpio = 52,			/* GSBI5 UART RX */
		.settings = {
			[GPIOMUX_ACTIVE]    = &gsbi5_debug_uart_rx,
			[GPIOMUX_SUSPENDED] = &gsbi5_debug_uart_rx,
		},
	},
	{ /* NC */
		.gpio = 53,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_unused_input,
			[GPIOMUX_SUSPENDED] = &gpio_unused_input,
		},
	},
	/* GPIO_54 set in apq8064_odin_*_configs */
	{ /* NC */
		.gpio = 55,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_unused_input,
			[GPIOMUX_SUSPENDED] = &gpio_unused_input,
		},
	},
	{ /* NC */
		.gpio = 56,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_unused_input,
			[GPIOMUX_SUSPENDED] = &gpio_unused_input,
		},
	},
	{ /* NC */
		.gpio = 57,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_unused_input,
			[GPIOMUX_SUSPENDED] = &gpio_unused_input,
		},
	},

	/* GPIO58 handled by QCT according to HW Config Spec */

	{ /* NC */
		.gpio = 59,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_unused_input,
			[GPIOMUX_SUSPENDED] = &gpio_unused_input,
		},
	},

	/* GPIO60-61 handled by QCT according to HW Config Spec */

	{ /* NC */
		.gpio = 62,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_unused_input,
			[GPIOMUX_SUSPENDED] = &gpio_unused_input,
		},
	},

	/* GPIO63 handled by QCT according to HW Config Spec */
	/* GPIO64-68 handled by QCT, configured in wcnss_5wire_interface */

	{ /* NC */
		.gpio = 69,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_unused_input,
			[GPIOMUX_SUSPENDED] = &gpio_unused_input,
		},
	},
	{ /* HDMI_DDC_CLK_APQ */
		.gpio = 70,
		.settings = {
			[GPIOMUX_ACTIVE]    = &hdmi_active_1_cfg,
			[GPIOMUX_SUSPENDED] = &hdmi_suspend_1_cfg,
		},
	},
	{ /* HDMI_DDC_DATA_APQ */
		.gpio = 71,
		.settings = {
			[GPIOMUX_ACTIVE]    = &hdmi_active_1_cfg,
			[GPIOMUX_SUSPENDED] = &hdmi_suspend_1_cfg,
		},
	},
	{ /* HDMI_HOTPLUG_DET_APQ */
		.gpio = 72,
		.settings = {
			[GPIOMUX_ACTIVE]    = &hdmi_active_2_cfg,
			[GPIOMUX_SUSPENDED] = &hdmi_suspend_2_cfg,
		},
	},

	/* GPIO73-76 handled by QCT according to HW Config Spec */

	{ /* PLUG_DET */
		.gpio = 77,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_no_pull_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_in,
		},
	},

	/* GPIO78-80 handled by QCT according to HW Config Spec */

	{ /* NC */
		.gpio = 81,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_unused_input,
			[GPIOMUX_SUSPENDED] = &gpio_unused_input,
		},
	},
	{ /* NC */
		.gpio = 82,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_unused_input,
			[GPIOMUX_SUSPENDED] = &gpio_unused_input,
		},
	},
	{ /* NC */
		.gpio = 83,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_unused_input,
			[GPIOMUX_SUSPENDED] = &gpio_unused_input,
		},
	},
	{ /* NC */
		.gpio = 84,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_unused_input,
			[GPIOMUX_SUSPENDED] = &gpio_unused_input,
		},
	},
	{ /* NC */
		.gpio = 85,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_unused_input,
			[GPIOMUX_SUSPENDED] = &gpio_unused_input,
		},
	},
	{ /* NC */
		.gpio = 86,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_unused_input,
			[GPIOMUX_SUSPENDED] = &gpio_unused_input,
		},
	},
	{ /* HW_SERVICE (BOOT_CONFIG_0) */
		.gpio = 87,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_no_pull_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_in,
		},
	},

	/* GPIO88-89 handled by QCT, configured in apq8064_hsic_configs */
};

#ifdef CONFIG_MMC_MSM_SDC2_SUPPORT
static struct gpiomux_setting sdc2_clk_active_cfg = {
	.func = GPIOMUX_FUNC_2,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting sdc2_cmd_data_0_3_active_cfg = {
	.func = GPIOMUX_FUNC_2,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_UP,
};

static struct gpiomux_setting sdc2_suspended_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct gpiomux_setting sdc2_data_1_suspended_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
};

static struct msm_gpiomux_config apq8064_sdc2_configs[] __initdata = {
	{
		.gpio      = 59,
		.settings = {
			[GPIOMUX_ACTIVE] = &sdc2_clk_active_cfg,
			[GPIOMUX_SUSPENDED] = &sdc2_suspended_cfg,
		},
	},
	{
		.gpio      = 57,
		.settings = {
			[GPIOMUX_ACTIVE] = &sdc2_cmd_data_0_3_active_cfg,
			[GPIOMUX_SUSPENDED] = &sdc2_suspended_cfg,
		},

	},
	{
		.gpio      = 62,
		.settings = {
			[GPIOMUX_ACTIVE] = &sdc2_cmd_data_0_3_active_cfg,
			[GPIOMUX_SUSPENDED] = &sdc2_suspended_cfg,
		},
	},
	{
		.gpio      = 61,
		.settings = {
			[GPIOMUX_ACTIVE] = &sdc2_cmd_data_0_3_active_cfg,
			[GPIOMUX_SUSPENDED] = &sdc2_data_1_suspended_cfg,
		},
	},
	{
		.gpio      = 60,
		.settings = {
			[GPIOMUX_ACTIVE] = &sdc2_cmd_data_0_3_active_cfg,
			[GPIOMUX_SUSPENDED] = &sdc2_suspended_cfg,
		},
	},
	{
		.gpio      = 58,
		.settings = {
			[GPIOMUX_ACTIVE] = &sdc2_cmd_data_0_3_active_cfg,
			[GPIOMUX_SUSPENDED] = &sdc2_suspended_cfg,
		},
	},
};
#endif


#ifdef CONFIG_MMC_MSM_SDC4_SUPPORT
static struct gpiomux_setting sdc4_clk_active_cfg = {
	.func = GPIOMUX_FUNC_2,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting sdc4_cmd_data_0_3_active_cfg = {
	.func = GPIOMUX_FUNC_2,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_UP,
};

static struct gpiomux_setting sdc4_suspended_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct gpiomux_setting sdc4_data_1_suspended_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
};

static struct msm_gpiomux_config apq8064_sdc4_configs[] __initdata = {
	{
		.gpio      = 68,
		.settings = {
			[GPIOMUX_ACTIVE] = &sdc4_clk_active_cfg,
			[GPIOMUX_SUSPENDED] = &sdc4_suspended_cfg,
		},
	},
	{
		.gpio      = 67,
		.settings = {
			[GPIOMUX_ACTIVE] = &sdc4_cmd_data_0_3_active_cfg,
			[GPIOMUX_SUSPENDED] = &sdc4_suspended_cfg,
		},

	},
	{
		.gpio      = 66,
		.settings = {
			[GPIOMUX_ACTIVE] = &sdc4_cmd_data_0_3_active_cfg,
			[GPIOMUX_SUSPENDED] = &sdc4_suspended_cfg,
		},
	},
	{
		.gpio      = 65,
		.settings = {
			[GPIOMUX_ACTIVE] = &sdc4_cmd_data_0_3_active_cfg,
			[GPIOMUX_SUSPENDED] = &sdc4_data_1_suspended_cfg,
		},
	},
	{
		.gpio      = 64,
		.settings = {
			[GPIOMUX_ACTIVE] = &sdc4_cmd_data_0_3_active_cfg,
			[GPIOMUX_SUSPENDED] = &sdc4_suspended_cfg,
		},
	},
	{
		.gpio      = 63,
		.settings = {
			[GPIOMUX_ACTIVE] = &sdc4_cmd_data_0_3_active_cfg,
			[GPIOMUX_SUSPENDED] = &sdc4_suspended_cfg,
		},
	},
};
#endif

static struct msm_gpiomux_config apq8064_odin_sp1_configs[] __initdata = {
	{ /* LMU_INT_N */
		.gpio = 54,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_no_pull_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_pull_down_in,
		},
	},
};
static struct msm_gpiomux_config apq8064_odin_sp11_configs[] __initdata = {
	{ /* Vibrator enable */
		.gpio = 54,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_no_pull_out_low,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_out_low,
		},
	},
};

void __init apq8064_init_gpiomux(void)
{
	int rc;

	rc = msm_gpiomux_init(NR_GPIO_IRQS);
	if (rc) {
		pr_err(KERN_ERR "msm_gpiomux_init failed %d\n", rc);
		return;
	}

	msm_gpiomux_install(apq8064_all_other_configs,
			ARRAY_SIZE(apq8064_all_other_configs));

	if (sony_hw_rev() <= HW_REV_ODIN_SP1_PB)
		msm_gpiomux_install(apq8064_odin_sp1_configs,
				ARRAY_SIZE(apq8064_odin_sp1_configs));
	else
		msm_gpiomux_install(apq8064_odin_sp11_configs,
				ARRAY_SIZE(apq8064_odin_sp11_configs));

	msm_gpiomux_install(wcnss_5wire_interface,
			ARRAY_SIZE(wcnss_5wire_interface));

	msm_gpiomux_install(apq8064_slimbus_config,
			ARRAY_SIZE(apq8064_slimbus_config));

	msm_gpiomux_install(apq8064_audio_codec_configs,
			ARRAY_SIZE(apq8064_audio_codec_configs));

	pr_debug("%s(): audio-auxpcm: Include GPIO configs"
		" as audio is not the primary user"
		" for these GPIO Pins\n", __func__);

	msm_gpiomux_install(mdm_configs,
		ARRAY_SIZE(mdm_configs));

	if (machine_is_apq8064_mtp() || machine_is_sony_fusion3())
		msm_gpiomux_install(rmi4_gpio_configs,
				ARRAY_SIZE(rmi4_gpio_configs));

#ifdef CONFIG_USB_EHCI_MSM_HSIC
	msm_gpiomux_install(apq8064_hsic_configs,
			ARRAY_SIZE(apq8064_hsic_configs));
#endif

#ifdef CONFIG_MMC_MSM_SDC2_SUPPORT
	 msm_gpiomux_install(apq8064_sdc2_configs,
			     ARRAY_SIZE(apq8064_sdc2_configs));
#endif

#ifdef CONFIG_MMC_MSM_SDC4_SUPPORT
	 msm_gpiomux_install(apq8064_sdc4_configs,
			     ARRAY_SIZE(apq8064_sdc4_configs));
#endif
}

