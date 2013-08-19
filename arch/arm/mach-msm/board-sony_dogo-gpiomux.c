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

#define NR_GPIO_SETTINGS	(NR_GPIO_IRQS * GPIOMUX_NSETTINGS)
#define INT_MAX			((int)(~0U>>1))
#define GPIOMUX_FOLLOW_QCT	INT_MAX

static struct gpiomux_setting __initdata *merge_set[NR_GPIO_SETTINGS];
static struct gpiomux_setting __initdata *qct_sets[NR_GPIO_SETTINGS];

#if defined(CONFIG_KS8851) || defined(CONFIG_KS8851_MODULE)
static struct gpiomux_setting gpio_eth_config = {
	.pull = GPIOMUX_PULL_NONE,
	.drv = GPIOMUX_DRV_8MA,
	.func = GPIOMUX_FUNC_GPIO,
};

/* The SPI configurations apply to GSBI 5*/
static struct gpiomux_setting gpio_spi_config = {
	.func = GPIOMUX_FUNC_2,
	.drv = GPIOMUX_DRV_12MA,
	.pull = GPIOMUX_PULL_NONE,
};

/* The SPI configurations apply to GSBI 5 chip select 2*/
static struct gpiomux_setting gpio_spi_cs2_config = {
	.func = GPIOMUX_FUNC_3,
	.drv = GPIOMUX_DRV_12MA,
	.pull = GPIOMUX_PULL_NONE,
};

/* Chip selects for SPI clients */
static struct gpiomux_setting gpio_spi_cs_config = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_12MA,
	.pull = GPIOMUX_PULL_UP,
};

struct msm_gpiomux_config apq8064_ethernet_configs[] = {
	{
		.gpio = 43,
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_eth_config,
			[GPIOMUX_ACTIVE] = &gpio_eth_config,
		}
	},
};
#endif

static struct gpiomux_setting gpio_i2c_config = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting gpio_i2c_config_sus = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_KEEPER,
};

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

static struct gpiomux_setting gsbi1_uart_config = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_16MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting ext_regulator_config = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_LOW,
};

static struct gpiomux_setting gsbi7_func1_cfg = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting gsbi7_func2_cfg = {
	.func = GPIOMUX_FUNC_2,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting gsbi3_suspended_cfg = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_KEEPER,
};

static struct gpiomux_setting gsbi3_active_cfg = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting hdmi_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct gpiomux_setting hdmi_active_1_cfg = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
};

static struct gpiomux_setting hdmi_active_2_cfg = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_16MA,
	.pull = GPIOMUX_PULL_DOWN,
};

#ifdef CONFIG_USB_EHCI_MSM_HSIC
static struct gpiomux_setting hsic_act_cfg = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_8MA,
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

static struct gpiomux_setting cyts_resout_sus_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_6MA,
	.pull = GPIOMUX_PULL_UP,
};

static struct gpiomux_setting cyts_resout_act_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_6MA,
	.pull = GPIOMUX_PULL_UP,
};

static struct gpiomux_setting cyts_sleep_sus_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_6MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct gpiomux_setting cyts_sleep_act_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_6MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct gpiomux_setting cyts_int_act_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_UP,
};

static struct gpiomux_setting cyts_int_sus_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct msm_gpiomux_config cyts_gpio_configs[] __initdata = {
	{	/* TS INTERRUPT */
		.gpio = 6,
		.settings = {
			[GPIOMUX_ACTIVE]    = &cyts_int_act_cfg,
			[GPIOMUX_SUSPENDED] = &cyts_int_sus_cfg,
		},
	},
	{	/* TS SLEEP */
		.gpio = 33,
		.settings = {
			[GPIOMUX_ACTIVE]    = &cyts_sleep_act_cfg,
			[GPIOMUX_SUSPENDED] = &cyts_sleep_sus_cfg,
		},
	},
	{	/* TS RESOUT */
		.gpio = 7,
		.settings = {
			[GPIOMUX_ACTIVE]    = &cyts_resout_act_cfg,
			[GPIOMUX_SUSPENDED] = &cyts_resout_sus_cfg,
		},
	},
};

static struct msm_gpiomux_config apq8064_hsic_configs[] = {
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

static struct gpiomux_setting mxt_reset_sus_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_6MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct gpiomux_setting mxt_reset_act_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_6MA,
	.pull = GPIOMUX_PULL_UP,
};

static struct gpiomux_setting mxt_int_sus_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct gpiomux_setting mxt_int_act_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_UP,
};

static struct msm_gpiomux_config apq8064_hdmi_configs[] __initdata = {
	{
		.gpio = 69,
		.settings = {
			[GPIOMUX_ACTIVE]    = &hdmi_active_1_cfg,
			[GPIOMUX_SUSPENDED] = &hdmi_suspend_cfg,
		},
	},
	{
		.gpio = 70,
		.settings = {
			[GPIOMUX_ACTIVE]    = &hdmi_active_1_cfg,
			[GPIOMUX_SUSPENDED] = &hdmi_suspend_cfg,
		},
	},
	{
		.gpio = 71,
		.settings = {
			[GPIOMUX_ACTIVE]    = &hdmi_active_1_cfg,
			[GPIOMUX_SUSPENDED] = &hdmi_suspend_cfg,
		},
	},
	{
		.gpio = 72,
		.settings = {
			[GPIOMUX_ACTIVE]    = &hdmi_active_2_cfg,
			[GPIOMUX_SUSPENDED] = &hdmi_suspend_cfg,
		},
	},
};

static struct msm_gpiomux_config apq8064_gsbi_configs[] __initdata = {
	{
		.gpio      = 8,			/* GSBI3 I2C QUP SDA */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gsbi3_suspended_cfg,
			[GPIOMUX_ACTIVE] = &gsbi3_active_cfg,
		},
	},
	{
		.gpio      = 9,			/* GSBI3 I2C QUP SCL */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gsbi3_suspended_cfg,
			[GPIOMUX_ACTIVE] = &gsbi3_active_cfg,
		},
	},
	{
		.gpio      = 18,		/* GSBI1 UART TX */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gsbi1_uart_config,
		},
	},
	{
		.gpio      = 19,		/* GSBI1 UART RX */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gsbi1_uart_config,
		},
	},
#if defined(CONFIG_KS8851) || defined(CONFIG_KS8851_MODULE)
	{
		.gpio      = 51,		/* GSBI5 QUP SPI_DATA_MOSI */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_spi_config,
		},
	},
	{
		.gpio      = 52,		/* GSBI5 QUP SPI_DATA_MISO */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_spi_config,
		},
	},
	{
		.gpio      = 53,		/* Funny CS0 */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_spi_config,
		},
	},
	{
		.gpio      = 31,		/* GSBI5 QUP SPI_CS2_N */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_spi_cs2_config,
		},
	},
	{
		.gpio      = 54,		/* GSBI5 QUP SPI_CLK */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_spi_config,
		},
	},
#endif
	{
		.gpio      = 30,		/* FP CS */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_spi_cs_config,
		},
	},
	{
		.gpio      = 32,		/* EPM CS */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_spi_cs_config,
		},
	},
	{
		.gpio      = 53,		/* NOR CS */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_spi_cs_config,
		},
	},
	{
		.gpio      = 82,	/* GSBI7 UART2 TX */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gsbi7_func2_cfg,
		},
	},
	{
		.gpio      = 83,	/* GSBI7 UART2 RX */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gsbi7_func1_cfg,
		},
	},
	{
		.gpio      = 21,		/* GSBI1 QUP I2C_CLK */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_i2c_config_sus,
			[GPIOMUX_ACTIVE] = &gpio_i2c_config,
		},
	},
	{
		.gpio      = 20,		/* GSBI1 QUP I2C_DATA */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_i2c_config_sus,
			[GPIOMUX_ACTIVE] = &gpio_i2c_config,
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

/* External 3.3 V regulator enable */
static struct msm_gpiomux_config apq8064_ext_regulator_configs[] __initdata = {
	{
		.gpio = APQ8064_EXT_3P3V_REG_EN_GPIO,
		.settings = {
			[GPIOMUX_SUSPENDED] = &ext_regulator_config,
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
			[GPIOMUX_ACTIVE] = &mdm2ap_status_cfg,
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

static struct msm_gpiomux_config apq8064_mxt_configs[] __initdata = {
	{	/* TS INTERRUPT */
		.gpio = 6,
		.settings = {
			[GPIOMUX_ACTIVE]    = &mxt_int_act_cfg,
			[GPIOMUX_SUSPENDED] = &mxt_int_sus_cfg,
		},
	},
	{	/* TS RESET */
		.gpio = 33,
		.settings = {
			[GPIOMUX_ACTIVE]    = &mxt_reset_act_cfg,
			[GPIOMUX_SUSPENDED] = &mxt_reset_sus_cfg,
		},
	},
};

static struct msm_gpiomux_config wcnss_5wire_interface[] = {
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

static struct gpiomux_setting apq8064_sdc3_card_det_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
};

static struct msm_gpiomux_config apq8064_sdc3_configs[] __initdata = {
	{
		.gpio      = 26,
		.settings = {
			[GPIOMUX_SUSPENDED] = &apq8064_sdc3_card_det_cfg,
			[GPIOMUX_ACTIVE] = &apq8064_sdc3_card_det_cfg,
		},
	},
};

static struct gpiomux_setting unused_gpio = {
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

static struct gpiomux_setting gpio_4ma_no_pull_in = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv  = GPIOMUX_DRV_4MA,
	.pull = GPIOMUX_PULL_NONE,
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

static struct gpiomux_setting gpio_8ma_follow_qct = {
	.func = GPIOMUX_FOLLOW_QCT,
	.drv  = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_FOLLOW_QCT,
	.dir  = GPIOMUX_FOLLOW_QCT,
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
	.drv  = GPIOMUX_DRV_4MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting gsbi2_i2c_cfg = {
	.func = GPIOMUX_FUNC_1,
	.drv  = GPIOMUX_DRV_4MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting gsbi3_i2c_cfg = {
	.func = GPIOMUX_FUNC_1,
	.drv  = GPIOMUX_DRV_4MA,
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
	.drv  = GPIOMUX_DRV_4MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir  = GPIOMUX_OUT_LOW,
};

static struct gpiomux_setting gsbi5_debug_uart_rx = {
	.func = GPIOMUX_FUNC_2,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
};

static struct gpiomux_setting hdmi_ddc = {
	.func = GPIOMUX_FUNC_1,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting hdmi_hot_plug_det = {
	.func = GPIOMUX_FUNC_1,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct msm_gpiomux_config fusion_all_configs[] __initdata = {
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
	{ /* BOOT_FROM_ROM */
		.gpio = 2,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_no_pull_out_low,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_out_low,
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
	{ /* TP_INT_N */
		.gpio = 6,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_no_pull_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_in,
		},
	},
	{ /* COMPASS_INT */
		.gpio = 7,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_pull_down_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_pull_down_in,
		},
	},
	{ /* I2C3_DATA_TS */
		.gpio = 8,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gsbi3_i2c_cfg,
			[GPIOMUX_SUSPENDED] = &gsbi3_i2c_cfg,
		},
	},
	{ /* I2C3_CLK_TS */
		.gpio = 9,
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
	{ /* FM_SSBI */
		.gpio = 14,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_follow_qct,
			[GPIOMUX_SUSPENDED] = &gpio_follow_qct,
		},
	},
	{ /* FM_DATA */
		.gpio = 15,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_follow_qct,
			[GPIOMUX_SUSPENDED] = &gpio_follow_qct,
		},
	},
	{ /* BT_CTL */
		.gpio = 16,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_follow_qct,
			[GPIOMUX_SUSPENDED] = &gpio_follow_qct,
		},
	},
	{ /* BT_DATA */
		.gpio = 17,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_follow_qct,
			[GPIOMUX_SUSPENDED] = &gpio_follow_qct,
		},
	},
	{ /* AP2MDM_ERR_FATAL */
		.gpio = 18,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_follow_qct,
			[GPIOMUX_SUSPENDED] = &gpio_follow_qct,
		},
	},
	{ /* MDM2AP_ERR_FATAL */
		.gpio = 19,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_follow_qct,
			[GPIOMUX_SUSPENDED] = &gpio_follow_qct,
		},
	},
	{ /* I2C1_DATA_APPS */
		.gpio = 20,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gsbi1_i2c_cfg,
			[GPIOMUX_SUSPENDED] = &gsbi1_i2c_cfg,
		},
	},
	{ /* I2C1_CLK_APPS */
		.gpio = 21,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gsbi1_i2c_cfg,
			[GPIOMUX_SUSPENDED] = &gsbi1_i2c_cfg,
		},
	},
	{ /* NC */
		.gpio = 22,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* NC */
		.gpio = 23,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* I2C2_DATA_SENS */
		.gpio = 24,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gsbi2_i2c_cfg,
			[GPIOMUX_SUSPENDED] = &gsbi2_i2c_cfg,
		},
	},
	{ /* I2C2_CLK_SENS */
		.gpio = 25,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gsbi2_i2c_cfg,
			[GPIOMUX_SUSPENDED] = &gsbi2_i2c_cfg,
		},
	},
	{ /* SD_CARD_DET_N */
		.gpio = 26,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_4ma_no_pull_in,
			[GPIOMUX_SUSPENDED] = &gpio_4ma_no_pull_in,
		},
	},
	{ /* AP2MDM_PON_RESET_N */
		.gpio = 27,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_follow_qct,
			[GPIOMUX_SUSPENDED] = &gpio_follow_qct,
		},
	},
	{ /* GYRO_INT_N */
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
	{ /* AP2MDM_VDDMIN */
		.gpio = 30,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_follow_qct,
			[GPIOMUX_SUSPENDED] = &gpio_follow_qct,
		},
	},
	{ /* SW_SERVICE */
		.gpio = 31,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_pull_up_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_pull_up_in,
		},
	},
	{ /* NC */
		.gpio = 32,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* NC */
		.gpio = 33,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* CHAT_CAM_RST_N */
		.gpio = 34,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_no_pull_out_low,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_out_low,
		},
	},
	{ /* AP2MDM_WAKEUP */
		.gpio = 35,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_follow_qct,
			[GPIOMUX_SUSPENDED] = &gpio_follow_qct,
		},
	},
	{ /* NC */
		.gpio = 36,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* MODEL_ID */
		.gpio = 37,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_pull_down_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_pull_down_in,
		},
	},
	{ /* MHL_INT */
		.gpio = 38,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_pull_up_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_pull_up_in,
		},
	},
	{ /* SLIMBUS1_MCLK */
		.gpio = 39,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_follow_qct,
			[GPIOMUX_SUSPENDED] = &gpio_follow_qct,
		},
	},
	{ /* SLIMBUS1_CLK */
		.gpio = 40,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_follow_qct,
			[GPIOMUX_SUSPENDED] = &gpio_follow_qct,
		},
	},
	{ /* SLIMBUS1_DATA */
		.gpio = 41,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_follow_qct,
			[GPIOMUX_SUSPENDED] = &gpio_follow_qct,
		},
	},
	{ /* CODEC_MAD_INT_N */
		.gpio = 42,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_follow_qct,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_follow_qct,
		},
	},
	{ /* NC(VIB_LIN_DET) */
		.gpio = 43,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_pull_down_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_pull_down_in,
		},
	},
	{ /* AP2MDM_IPC2 */
		.gpio = 44,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_follow_qct,
			[GPIOMUX_SUSPENDED] = &gpio_follow_qct,
		},
	},
	{ /* PROX_INT_N */
		.gpio = 45,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_pull_up_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_pull_up_in,
		},
	},
	{ /* MDM2AP_HSIC_READY */
		.gpio = 46,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_follow_qct,
			[GPIOMUX_SUSPENDED] = &gpio_follow_qct,
		},
	},
	{ /* MDM2AP_WAKEUP */
		.gpio = 47,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_follow_qct,
			[GPIOMUX_SUSPENDED] = &gpio_follow_qct,
		},
	},
	{ /* AP2MDM_STATUS */
		.gpio = 48,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_follow_qct,
			[GPIOMUX_SUSPENDED] = &gpio_follow_qct,
		},
	},
	{ /* MDM2AP_STATUS */
		.gpio = 49,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_follow_qct,
			[GPIOMUX_SUSPENDED] = &gpio_follow_qct,
		},
	},
	{ /* HW_SERVICE(BOOT_CONFIG_1) */
		.gpio = 50,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_no_pull_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_in,
		},
	},
	{ /* UART_TX_DFMS */
		.gpio = 51,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gsbi5_debug_uart_tx,
			[GPIOMUX_SUSPENDED] = &gsbi5_debug_uart_tx,
		},
	},
	{ /* UART_RX_DTMS */
		.gpio = 52,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gsbi5_debug_uart_rx,
			[GPIOMUX_SUSPENDED] = &gsbi5_debug_uart_rx,
		},
	},
	{ /* RF_ID_EXTENTION */
		.gpio = 53,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_pull_down_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_pull_down_in,
		},
	},
	{ /* LMU_INT_N */
		.gpio = 54,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_pull_down_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_pull_down_in,
		},
	},
	{ /* NC */
		.gpio = 55,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* NC */
		.gpio = 56,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* NC */
		.gpio = 57,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* WCN_PRIORITY */
		.gpio = 58,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_follow_qct,
			[GPIOMUX_SUSPENDED] = &gpio_follow_qct,
		},
	},
	{ /* DEBUG_GPIO1 */
		.gpio = 59,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_no_pull_out_low,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_out_low,
		},
	},
	{ /* MDM_LTE_FRAME_SYNC */
		.gpio = 60,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_follow_qct,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_follow_qct,
		},
	},
	{ /* MDM_LTE_ACTIVE */
		.gpio = 61,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_follow_qct,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_follow_qct,
		},
	},
	{ /* NC(Reserve for TSIF1_SYNC) */
		.gpio = 62,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* BT_SSBI */
		.gpio = 63,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_follow_qct,
			[GPIOMUX_SUSPENDED] = &gpio_follow_qct,
		},
	},
	{ /* WL_CMD_DATA2 */
		.gpio = 64,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_follow_qct,
			[GPIOMUX_SUSPENDED] = &gpio_follow_qct,
		},
	},
	{ /* WL_CMD_DATA1 */
		.gpio = 65,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_follow_qct,
			[GPIOMUX_SUSPENDED] = &gpio_follow_qct,
		},
	},
	{ /* WL_CMD_DATA0 */
		.gpio = 66,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_follow_qct,
			[GPIOMUX_SUSPENDED] = &gpio_follow_qct,
		},
	},
	{ /* WL_CMD_SET */
		.gpio = 67,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_follow_qct,
			[GPIOMUX_SUSPENDED] = &gpio_follow_qct,
		},
	},
	{ /* WL_CMD_CLK */
		.gpio = 68,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_follow_qct,
			[GPIOMUX_SUSPENDED] = &gpio_follow_qct,
		},
	},
	{ /* NC */
		.gpio = 69,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* HDMI_DDC_CLK_APQ */
		.gpio = 70,
		.settings = {
			[GPIOMUX_ACTIVE]    = &hdmi_ddc,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_in,
		},
	},
	{ /* HDMI_DDC_DATA_APQ */
		.gpio = 71,
		.settings = {
			[GPIOMUX_ACTIVE]    = &hdmi_ddc,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_in,
		},
	},
	{ /* HDMI_HOTPLUG_DET_APQ */
		.gpio = 72,
		.settings = {
			[GPIOMUX_ACTIVE]    = &hdmi_hot_plug_det,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_pull_down_in,
		},
	},
	{ /* PM8921_APC_SEC_IRQ_N */
		.gpio = 73,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_follow_qct,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_follow_qct,
		},
	},
	{ /* PM8921_APC_USR_IRQ_N */
		.gpio = 74,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_follow_qct,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_follow_qct,
		},
	},
	{ /* PM8921_MDM_IRQ_N */
		.gpio = 75,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_follow_qct,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_follow_qct,
		},
	},
	{ /* PM8821_APC_SEC_IRQ_N */
		.gpio = 76,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_follow_qct,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_follow_qct,
		},
	},
	{ /* PLUG_DET */
		.gpio = 77,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_no_pull_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_in,
		},
	},
	{ /* APQ_PS_HOLD */
		.gpio = 78,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_follow_qct,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_follow_qct,
		},
	},
	{ /* SSBI_PM8821 */
		.gpio = 79,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_follow_qct,
			[GPIOMUX_SUSPENDED] = &gpio_follow_qct,
		},
	},
	{ /* MDM2AP_VDDMIN */
		.gpio = 80,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_follow_qct,
			[GPIOMUX_SUSPENDED] = &gpio_follow_qct,
		},
	},
	{ /* AP2MDM_IPC1 */
		.gpio = 81,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_follow_qct,
			[GPIOMUX_SUSPENDED] = &gpio_follow_qct,
		},
	},
	{ /* NC */
		.gpio = 82,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* NC */
		.gpio = 83,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* MDM2AP_READY */
		.gpio = 84,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_pull_down_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_pull_down_in,
		},
	},
	{ /* NC */
		.gpio = 85,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* NC */
		.gpio = 86,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* HW_SERVICE(BOOT_CONFIG_0) */
		.gpio = 87,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_no_pull_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_in,
		},
	},
	{ /* HSIC_STROBE */
		.gpio = 88,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_8ma_follow_qct,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_follow_qct,
		},
	},
	{ /* HSIC_DATA */
		.gpio = 89,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_8ma_follow_qct,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_follow_qct,
		},
	},
};

static void __init gpiomux_free_merged_sets(void)
{
	int i;

	for (i = 0; i < NR_GPIO_SETTINGS; ++i)
		kfree(merge_set[i]);
}

static void __init gpiomux_merge_setting(unsigned idx, unsigned which)
{
	struct gpiomux_setting *pset = fusion_all_configs[idx].settings[which];
	unsigned set_slot = fusion_all_configs[idx].gpio
				* GPIOMUX_NSETTINGS + which;

	if (!pset)
		return;

	if ((pset->func == GPIOMUX_FOLLOW_QCT) &&
		(pset->drv == GPIOMUX_FOLLOW_QCT) &&
		(pset->pull == GPIOMUX_FOLLOW_QCT) &&
		(pset->dir == GPIOMUX_FOLLOW_QCT)) {
		fusion_all_configs[idx].settings[which] = qct_sets[set_slot];
	} else if ((pset->func == GPIOMUX_FOLLOW_QCT) ||
		(pset->drv == GPIOMUX_FOLLOW_QCT) ||
		(pset->pull == GPIOMUX_FOLLOW_QCT) ||
		(pset->dir == GPIOMUX_FOLLOW_QCT)) {
		if (!qct_sets[set_slot]) {
			fusion_all_configs[idx].settings[which] = NULL;
			return;
		}

		merge_set[set_slot] = kzalloc(sizeof(struct gpiomux_setting),
					   GFP_KERNEL);
		if (!merge_set[set_slot]) {
			pr_err("%s: GPIO_%d merge failure\n", __func__,
					fusion_all_configs[idx].gpio);
			return;
		}

		*merge_set[set_slot] = *pset;

		if (pset->func == GPIOMUX_FOLLOW_QCT)
			merge_set[set_slot]->func = qct_sets[set_slot]->func;
		if (pset->drv == GPIOMUX_FOLLOW_QCT)
			merge_set[set_slot]->drv = qct_sets[set_slot]->drv;
		if (pset->pull == GPIOMUX_FOLLOW_QCT)
			merge_set[set_slot]->pull = qct_sets[set_slot]->pull;
		if (pset->dir == GPIOMUX_FOLLOW_QCT)
			merge_set[set_slot]->dir = qct_sets[set_slot]->dir;

		fusion_all_configs[idx].settings[which] = merge_set[set_slot];
	}
}

static void __init gpiomux_remake_configs(unsigned nconfigs)
{
	unsigned c, s;

	for (c = 0; c < nconfigs; ++c) {
		for (s = 0; s < GPIOMUX_NSETTINGS; ++s)
			gpiomux_merge_setting(c, s);
	}
}

static void __init gpiomux_set_qct_configs(struct msm_gpiomux_config *configs,
		unsigned nconfigs)
{
	unsigned c, s, set_slot;

	for (c = 0; c < nconfigs; ++c) {
		for (s = 0; s < GPIOMUX_NSETTINGS; ++s) {
			set_slot = configs[c].gpio * GPIOMUX_NSETTINGS + s;
			qct_sets[set_slot] = configs[c].settings[s];
		}
	}
}

static void __init gpiomux_arrange_all_qct_configs(void)
{
#if defined(CONFIG_KS8851) || defined(CONFIG_KS8851_MODULE)
	gpiomux_set_qct_configs(apq8064_ethernet_configs,
			ARRAY_SIZE(apq8064_ethernet_configs));
#endif

	gpiomux_set_qct_configs(wcnss_5wire_interface,
			ARRAY_SIZE(wcnss_5wire_interface));

	gpiomux_set_qct_configs(apq8064_gsbi_configs,
			ARRAY_SIZE(apq8064_gsbi_configs));

	gpiomux_set_qct_configs(apq8064_slimbus_config,
			ARRAY_SIZE(apq8064_slimbus_config));

	gpiomux_set_qct_configs(apq8064_audio_codec_configs,
			ARRAY_SIZE(apq8064_audio_codec_configs));

	pr_debug(
	"%s(): audio-auxpcm: Include GPIO configs as audio is not the primary"
	" user for these GPIO Pins\n", __func__);

	gpiomux_set_qct_configs(apq8064_ext_regulator_configs,
			ARRAY_SIZE(apq8064_ext_regulator_configs));

	if (machine_is_apq8064_mtp() || machine_is_sony_fusion3())
		gpiomux_set_qct_configs(mdm_configs,
			ARRAY_SIZE(mdm_configs));

#ifdef CONFIG_USB_EHCI_MSM_HSIC
	if (machine_is_apq8064_mtp())
		gpiomux_set_qct_configs(cyts_gpio_configs,
				ARRAY_SIZE(cyts_gpio_configs));

	if (machine_is_apq8064_mtp() || machine_is_sony_fusion3())
		gpiomux_set_qct_configs(apq8064_hsic_configs,
				ARRAY_SIZE(apq8064_hsic_configs));
#endif

	if (machine_is_apq8064_cdp() || machine_is_apq8064_liquid())
		gpiomux_set_qct_configs(apq8064_mxt_configs,
			ARRAY_SIZE(apq8064_mxt_configs));

	gpiomux_set_qct_configs(apq8064_hdmi_configs,
			ARRAY_SIZE(apq8064_hdmi_configs));

#ifdef CONFIG_MMC_MSM_SDC2_SUPPORT
	gpiomux_set_qct_configs(apq8064_sdc2_configs,
			     ARRAY_SIZE(apq8064_sdc2_configs));
#endif

#ifdef CONFIG_MMC_MSM_SDC4_SUPPORT
	gpiomux_set_qct_configs(apq8064_sdc4_configs,
			     ARRAY_SIZE(apq8064_sdc4_configs));
#endif

	gpiomux_set_qct_configs(apq8064_sdc3_configs,
			ARRAY_SIZE(apq8064_sdc3_configs));
}

void __init apq8064_init_gpiomux(void)
{
	int rc = msm_gpiomux_init(NR_GPIO_IRQS);
	if (rc) {
		pr_err(KERN_ERR "msm_gpiomux_init failed %d\n", rc);
		return;
	}

	gpiomux_arrange_all_qct_configs();
	gpiomux_remake_configs(ARRAY_SIZE(fusion_all_configs));

	msm_gpiomux_install(fusion_all_configs,
			ARRAY_SIZE(fusion_all_configs));

	gpiomux_free_merged_sets();
}
