/* Copyright (c) 2012, Code Aurora Forum. All rights reserved.
 * Copyright (C) 2012-2013 Sony Mobile Communications AB.
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

#include <linux/i2c.h>
#include <linux/gpio.h>

#include <asm/mach-types.h>

#include <mach/camera.h>
#include <mach/msm_bus_board.h>
#include <mach/gpiomux.h>
#ifdef CONFIG_MSM_CAMERA
#include <mach/msm_bus_board.h>
#include <mach/camera.h>
#include <media/sony_camera_v4l2.h>
#include <linux/v4l2-mediabus.h>
#endif

#include "devices.h"
#include "board-8064.h"

#ifdef CONFIG_MSM_CAMERA
#if !defined(CONFIG_SONY_CAM_V4L2)
static struct gpiomux_setting cam_settings[] = {
	{
		.func = GPIOMUX_FUNC_GPIO, /*suspend*/
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_DOWN,
	},

	{
		.func = GPIOMUX_FUNC_1, /*active 1*/
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_NONE,
	},

	{
		.func = GPIOMUX_FUNC_GPIO, /*active 2*/
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_NONE,
	},

	{
		.func = GPIOMUX_FUNC_2, /*active 3*/
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_NONE,
	},

	{
		.func = GPIOMUX_FUNC_5, /*active 4*/
		.drv = GPIOMUX_DRV_8MA,
		.pull = GPIOMUX_PULL_UP,
	},

	{
		.func = GPIOMUX_FUNC_6, /*active 5*/
		.drv = GPIOMUX_DRV_8MA,
		.pull = GPIOMUX_PULL_UP,
	},

	{
		.func = GPIOMUX_FUNC_2, /*active 6*/
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_UP,
	},

	{
		.func = GPIOMUX_FUNC_3, /*active 7*/
		.drv = GPIOMUX_DRV_8MA,
		.pull = GPIOMUX_PULL_UP,
	},

	{
		.func = GPIOMUX_FUNC_GPIO, /*i2c suspend*/
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_KEEPER,
	},

	{
		.func = GPIOMUX_FUNC_9, /*active 9*/
		.drv = GPIOMUX_DRV_8MA,
		.pull = GPIOMUX_PULL_NONE,
	},
	{
		.func = GPIOMUX_FUNC_A, /*active 10*/
		.drv = GPIOMUX_DRV_8MA,
		.pull = GPIOMUX_PULL_NONE,
	},
	{
		.func = GPIOMUX_FUNC_6, /*active 11*/
		.drv = GPIOMUX_DRV_8MA,
		.pull = GPIOMUX_PULL_NONE,
	},
	{
		.func = GPIOMUX_FUNC_4, /*active 12*/
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_NONE,
	},

};

static struct msm_gpiomux_config apq8064_cam_common_configs[] = {
	{
		.gpio = 1,
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[2],
			[GPIOMUX_SUSPENDED] = &cam_settings[0],
		},
	},
	{
		.gpio = 2,
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[12],
			[GPIOMUX_SUSPENDED] = &cam_settings[0],
		},
	},
	{
		.gpio = 3,
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[2],
			[GPIOMUX_SUSPENDED] = &cam_settings[0],
		},
	},
	{
		.gpio = 4,
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[3],
			[GPIOMUX_SUSPENDED] = &cam_settings[0],
		},
	},
	{
		.gpio = 5,
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[1],
			[GPIOMUX_SUSPENDED] = &cam_settings[0],
		},
	},
	{
		.gpio = 34,
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[2],
			[GPIOMUX_SUSPENDED] = &cam_settings[0],
		},
	},
	{
		.gpio = 107,
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[2],
			[GPIOMUX_SUSPENDED] = &cam_settings[0],
		},
	},
	{
		.gpio = 10,
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[9],
			[GPIOMUX_SUSPENDED] = &cam_settings[8],
		},
	},
	{
		.gpio = 11,
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[10],
			[GPIOMUX_SUSPENDED] = &cam_settings[8],
		},
	},
	{
		.gpio = 12,
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[11],
			[GPIOMUX_SUSPENDED] = &cam_settings[8],
		},
	},
	{
		.gpio = 13,
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[11],
			[GPIOMUX_SUSPENDED] = &cam_settings[8],
		},
	},
};
#endif

#if defined(CONFIG_SONY_CAM_V4L2)
static struct msm_gpiomux_config apq8064_cam_2d_configs[] = {
};
#endif

#if defined(CONFIG_SONY_CAM_V4L2)
static struct msm_bus_vectors cam_init_vectors[] = {
	{
		.src = MSM_BUS_MASTER_VFE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 0,
		.ib  = 0,
	},
	{
		.src = MSM_BUS_MASTER_VPE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 0,
		.ib  = 0,
	},
	{
		.src = MSM_BUS_MASTER_JPEG_ENC,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 0,
		.ib  = 0,
	},
};

static struct msm_bus_vectors cam_preview_vectors[] = {
	{
		.src = MSM_BUS_MASTER_VFE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 155520000,
		.ib  = 622080000,
	},
	{
		.src = MSM_BUS_MASTER_VPE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 214617600,
		.ib  = 494553600,
	},
	{
		.src = MSM_BUS_MASTER_JPEG_ENC,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 0,
		.ib  = 0,
	},
};

static struct msm_bus_vectors cam_video_vectors[] = {
	{
		.src = MSM_BUS_MASTER_VFE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 192839040,
		.ib  = 771356160,
	},
	{
		.src = MSM_BUS_MASTER_VPE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 311028480,
		.ib  = 777571200,
	},
	{
		.src = MSM_BUS_MASTER_JPEG_ENC,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 311028480,
		.ib  = 311028480,
	},
};

static struct msm_bus_vectors cam_snapshot_vectors[] = {
	{
		.src = MSM_BUS_MASTER_VFE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 421749504,
		.ib  = 1686998016,
	},
	{
		.src = MSM_BUS_MASTER_VPE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 843499008,
		.ib  = 2108747520,
	},
	{
		.src = MSM_BUS_MASTER_JPEG_ENC,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 843499008,
		.ib  = 843499008,
	},
};

static struct msm_bus_vectors cam_zsl_vectors[] = {
	{
		.src = MSM_BUS_MASTER_VFE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 302071680,
		.ib  = 1208286720,
	},
	{
		.src = MSM_BUS_MASTER_VPE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 0,
		.ib  = 0,
	},
	{
		.src = MSM_BUS_MASTER_JPEG_ENC,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 1521190000,
		.ib  = 1521190000,
	},
};

static struct msm_bus_vectors cam_video_ls_vectors[] = {
	{
		.src = MSM_BUS_MASTER_VFE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 348192000,
		.ib  = 617103360,
	},
	{
		.src = MSM_BUS_MASTER_VPE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 206807040,
		.ib  = 488816640,
	},
	{
		.src = MSM_BUS_MASTER_JPEG_ENC,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 540000000,
		.ib  = 1350000000,
	},
};

static struct msm_bus_vectors cam_dual_vectors[] = {
	{
		.src = MSM_BUS_MASTER_VFE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 348192000,
		.ib  = 1208286720,
	},
	{
		.src = MSM_BUS_MASTER_VPE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 206807040,
		.ib  = 488816640,
	},
	{
		.src = MSM_BUS_MASTER_JPEG_ENC,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 540000000,
		.ib  = 1350000000,
	},
	{
		.src = MSM_BUS_MASTER_JPEG_ENC,
		.dst = MSM_BUS_SLAVE_MM_IMEM,
		.ab  = 43200000,
		.ib  = 69120000,
	},
	{
		.src = MSM_BUS_MASTER_VFE,
		.dst = MSM_BUS_SLAVE_MM_IMEM,
		.ab  = 43200000,
		.ib  = 69120000,
	},
};
static struct msm_bus_paths cam_bus_client_config[] = {
	{
		ARRAY_SIZE(cam_init_vectors),
		cam_init_vectors,
	},
	{
		ARRAY_SIZE(cam_preview_vectors),
		cam_preview_vectors,
	},
	{
		ARRAY_SIZE(cam_video_vectors),
		cam_video_vectors,
	},
	{
		ARRAY_SIZE(cam_snapshot_vectors),
		cam_snapshot_vectors,
	},
	{
		ARRAY_SIZE(cam_zsl_vectors),
		cam_zsl_vectors,
	},
	{
		ARRAY_SIZE(cam_video_ls_vectors),
		cam_video_ls_vectors,
	},
	{
		ARRAY_SIZE(cam_dual_vectors),
		cam_dual_vectors,
	},
};

static struct msm_bus_scale_pdata cam_bus_client_pdata = {
		cam_bus_client_config,
		ARRAY_SIZE(cam_bus_client_config),
		.name = "msm_camera",
};

static struct msm_camera_device_platform_data msm_camera_csi_device_data[] = {
	{
		.csid_core = 0,
#if defined(CONFIG_SONY_VPE)
		.is_vpe    = 0,
#else
		.is_vpe    = 1,
#endif
		.cam_bus_scale_table = &cam_bus_client_pdata,
	},
	{
		.csid_core = 1,
#if defined(CONFIG_SONY_VPE)
		.is_vpe    = 0,
#else
		.is_vpe    = 1,
#endif
		.cam_bus_scale_table = &cam_bus_client_pdata,
	},
};

static struct camera_vreg_t apq_8064_back_cam_vreg[] = {
	{"cam_vana", REG_LDO, 2800000, 2800000, 85600},
	{"cam_vio", REG_VS, 0, 0, 0},
	{"cam_vdig", REG_LDO, 1050000, 1200000, 105000},
	{"cam_vaf", REG_LDO, 2700000, 2800000, 300000},
};

static struct camera_vreg_t apq_8064_front_cam_vreg[] = {
	{"cam_vana", REG_LDO, 2800000, 2800000, 85600},
	{"cam_vdig", REG_LDO, 1200000, 1200000, 105000},
	{"cam_vio", REG_VS, 0, 0, 0},
};
#endif

#define CAML_RSTN PM8921_GPIO_PM_TO_SYS(28)
#define CAMR_RSTN 34

#if defined(CONFIG_SONY_CAM_V4L2)
#define CAML_AF_CE PM8921_GPIO_PM_TO_SYS(29)

static struct gpio apq8064_common_cam_gpio[] = {
};

static struct gpio apq8064_back_cam_gpio[] = {
	{5, GPIOF_DIR_IN, "CAMIF_MCLK"},
	{CAML_RSTN, GPIOF_DIR_OUT, "CAM_RESET"},
};

static struct msm_gpio_set_tbl apq8064_back_cam_gpio_set_tbl[] = {
	{CAML_RSTN, GPIOF_OUT_INIT_LOW, 1},
	{CAML_RSTN, GPIOF_OUT_INIT_HIGH, 1000},
};

static struct msm_camera_gpio_conf apq8064_back_cam_gpio_conf = {
	.cam_gpiomux_conf_tbl = apq8064_cam_2d_configs,
	.cam_gpiomux_conf_tbl_size = ARRAY_SIZE(apq8064_cam_2d_configs),
	.cam_gpio_common_tbl = apq8064_common_cam_gpio,
	.cam_gpio_common_tbl_size = ARRAY_SIZE(apq8064_common_cam_gpio),
	.cam_gpio_req_tbl = apq8064_back_cam_gpio,
	.cam_gpio_req_tbl_size = ARRAY_SIZE(apq8064_back_cam_gpio),
	.cam_gpio_set_tbl = apq8064_back_cam_gpio_set_tbl,
	.cam_gpio_set_tbl_size = ARRAY_SIZE(apq8064_back_cam_gpio_set_tbl),
};

static struct gpio apq8064_front_cam_gpio[] = {
	{4, GPIOF_DIR_IN, "CAMIF_MCLK"},
	{12, GPIOF_DIR_IN, "CAMIF_I2C_DATA"},
	{13, GPIOF_DIR_IN, "CAMIF_I2C_CLK"},
	{CAMR_RSTN, GPIOF_DIR_OUT, "CAM_RESET"},
};

static struct msm_gpio_set_tbl apq8064_front_cam_gpio_set_tbl[] = {
	{CAMR_RSTN, GPIOF_OUT_INIT_LOW, 1},
	{CAMR_RSTN, GPIOF_OUT_INIT_HIGH, 1000},

};

static struct msm_camera_gpio_conf apq8064_front_cam_gpio_conf = {
	.cam_gpiomux_conf_tbl = apq8064_cam_2d_configs,
	.cam_gpiomux_conf_tbl_size = ARRAY_SIZE(apq8064_cam_2d_configs),
	.cam_gpio_common_tbl = apq8064_common_cam_gpio,
	.cam_gpio_common_tbl_size = ARRAY_SIZE(apq8064_common_cam_gpio),
	.cam_gpio_req_tbl = apq8064_front_cam_gpio,
	.cam_gpio_req_tbl_size = ARRAY_SIZE(apq8064_front_cam_gpio),
	.cam_gpio_set_tbl = apq8064_front_cam_gpio_set_tbl,
	.cam_gpio_set_tbl_size = ARRAY_SIZE(apq8064_front_cam_gpio_set_tbl),
};

static struct msm_camera_i2c_conf apq8064_back_cam_i2c_conf = {
	.use_i2c_mux = 1,
	.mux_dev = &msm8960_device_i2c_mux_gsbi4,
	.i2c_mux_mode = MODE_L,
};

static struct msm_camera_i2c_conf apq8064_front_cam_i2c_conf = {
	.use_i2c_mux = 1,
	.mux_dev = &msm8960_device_i2c_mux_gsbi4,
	.i2c_mux_mode = MODE_R,
};

static struct msm_camera_sensor_flash_data msm_camera_flash_none = {
	.flash_type	= MSM_CAMERA_FLASH_NONE,
	.flash_src	= NULL,
};

static struct msm_camera_csi_lane_params msm_camera_csi_lane_params = {
	.csi_lane_assign = 0xE4,
	.csi_lane_mask = 0xF,
};

static struct msm_camera_sensor_platform_info
		msm_camera_sony_camera_platform_info[] = {
	{
		.mount_angle		= 90,
		.sensor_reset		= CAML_RSTN,
		.cam_vreg		= apq_8064_back_cam_vreg,
		.num_vreg		= ARRAY_SIZE(apq_8064_back_cam_vreg),
		.gpio_conf		= &apq8064_back_cam_gpio_conf,
		.i2c_conf		= &apq8064_back_cam_i2c_conf,
		.csi_lane_params	= &msm_camera_csi_lane_params,
	},
	{
		.mount_angle		= 90,
		.sensor_reset		= CAMR_RSTN,
		.cam_vreg		= apq_8064_front_cam_vreg,
		.num_vreg		= ARRAY_SIZE(apq_8064_front_cam_vreg),
		.gpio_conf		= &apq8064_front_cam_gpio_conf,
		.i2c_conf		= &apq8064_front_cam_i2c_conf,
		.csi_lane_params	= &msm_camera_csi_lane_params,
	}
};

static struct msm_camera_sensor_info msm_camera_sony_camera_sensor_info[] = {
	{
		.sensor_name		= "sony_camera_0",
		.sensor_reset		= CAML_RSTN,
		.pdata			= &msm_camera_csi_device_data[0],
		.flash_data		= &msm_camera_flash_none,
		.sensor_platform_info	=
				&msm_camera_sony_camera_platform_info[0],
		.csi_if			= 1,
		.camera_type		= BACK_CAMERA_2D,
		.sensor_type		= BAYER_SENSOR,
	},
	{
		.sensor_name		= "sony_camera_1",
		.sensor_reset		= CAMR_RSTN,
		.pdata			= &msm_camera_csi_device_data[1],
		.flash_data		= &msm_camera_flash_none,
		.sensor_platform_info	=
				&msm_camera_sony_camera_platform_info[1],
		.csi_if			= 1,
		.camera_type		= FRONT_CAMERA_2D,
		.sensor_type		= BAYER_SENSOR,
	}
};
#endif

#if defined(CONFIG_SONY_VPE)
struct resource sony_vpe_resources[] = {
	{
		.start	= 0x05300000,
		.end	= 0x05300000 + SZ_1M - 1,
		.flags	= IORESOURCE_MEM,
	},
	{
		.start	= VPE_IRQ,
		.end	= VPE_IRQ,
		.flags	= IORESOURCE_IRQ,
	},
};

struct platform_device sony_vpe_device = {
	.name		= "sony_vpe",
	.id		= 0,
	.num_resources	= ARRAY_SIZE(sony_vpe_resources),
	.resource	= sony_vpe_resources,
};
#endif

static struct platform_device msm_camera_server = {
	.name = "msm_cam_server",
	.id = 0,
};

void __init apq8064_init_cam(void)
{
#if !defined(CONFIG_SONY_CAM_V4L2)
	msm_gpiomux_install(apq8064_cam_common_configs,
			ARRAY_SIZE(apq8064_cam_common_configs));
#endif

	platform_device_register(&msm_camera_server);
	platform_device_register(&msm8960_device_i2c_mux_gsbi4);
	platform_device_register(&msm8960_device_csiphy0);
	platform_device_register(&msm8960_device_csiphy1);
	platform_device_register(&msm8960_device_csid0);
	platform_device_register(&msm8960_device_csid1);
	platform_device_register(&msm8960_device_ispif);
	platform_device_register(&msm8960_device_vfe);
#if defined(CONFIG_SONY_VPE)
	platform_device_register(&sony_vpe_device);
#else
	platform_device_register(&msm8960_device_vpe);
#endif
}

#ifdef CONFIG_I2C
static struct i2c_board_info apq8064_camera_i2c_boardinfo[] = {
	{
#if !defined(CONFIG_SONY_CAM_V4L2)
	I2C_BOARD_INFO("sc628a", 0x6E),
#else
	I2C_BOARD_INFO("sony_camera_0", 0x10),
	.platform_data = &msm_camera_sony_camera_sensor_info[0],
	},
	{
	I2C_BOARD_INFO("sony_camera_1", 0x36),
	.platform_data = &msm_camera_sony_camera_sensor_info[1],
#endif
	},
};

struct msm_camera_board_info apq8064_camera_board_info = {
	.board_info = apq8064_camera_i2c_boardinfo,
	.num_i2c_board_info = ARRAY_SIZE(apq8064_camera_i2c_boardinfo),
};
#endif

#if defined(CONFIG_SONY_CAM_V4L2)
static const struct sony_camera_seq sensor_power_off_back[] = {
	{ SONY_I2C_WRITE, 0x0100, 100 },
	{ SONY_GPIO_RESET, 0, 1 },
	{ SONY_CAM_CLK, -1, 1 },
	{ SONY_CAM_VAF, -1, 0 },
	{ SONY_CAM_VANA, -1, 1 },
	{ SONY_CAM_VIO, -1, 1 },
	{ SONY_CAM_VDIG, -1, 15 },
	{ EXIT, 0, 0 },
};

static const struct sony_camera_seq sensor_power_on_back[] = {
	{ SONY_CAM_VDIG, 1050, 1 },
	{ SONY_CAM_VIO, 0, 1 },
	{ SONY_CAM_VANA, 2800, 0 },
	{ SONY_CAM_VAF, 2700, 1 },
	{ SONY_GPIO_RESET, 1, 9 },
	{ SONY_CAM_CLK, 0, 1 },
	{ EXIT, 0, 0 },
};

static const struct sony_camera_seq sensor_power_on_front[] = {
	{ SONY_CAM_VDIG, 1200, 1 },
	{ SONY_CAM_VIO, 0, 1 },
	{ SONY_CAM_VANA, 2800, 1 },
	{ SONY_GPIO_RESET, 1, 1 },
	{ SONY_CAM_CLK, 0, 1 },
	{ EXIT, 0, 0 },
};

static const struct sony_camera_seq sensor_power_off_front[] = {
	{ SONY_I2C_WRITE, 0x0100, 100 },
	{ SONY_GPIO_RESET, 0, 1 },
	{ SONY_CAM_CLK, -1, 1 },      /*  16 us at least */
	{ SONY_CAM_VANA, -1, 1 },
	{ SONY_CAM_VIO, -1, 1 },
	{ SONY_CAM_VDIG, -1, 1 },
	{ EXIT, 0, 0 },
};

static const struct sony_camera_seq sensor_power_on_imx132[] = {
	{ SONY_CAM_VDIG, 1200, 1 },
	{ SONY_CAM_VIO, 0, 1 },
	{ SONY_CAM_VANA, 2800, 1 },
	{ SONY_GPIO_RESET, 1, 1 },
	{ SONY_CAM_CLK, 0, 1 },
	{ EXIT, 0, 0 },
};

static const struct sony_camera_seq sensor_power_off_imx132[] = {
	{ SONY_I2C_WRITE, 0x0100, 100 },
	{ SONY_GPIO_RESET, 0, 1 },
	{ SONY_CAM_CLK, -1, 1 },      /*  16 us at least */
	{ SONY_CAM_VANA, -1, 1 },
	{ SONY_CAM_VIO, -1, 1 },
	{ SONY_CAM_VDIG, -1, 1 },
	{ EXIT, 0, 0 },
};

const struct sony_camera_module sony_camera_0_modules[] = {
	{ "GENERIC",  sensor_power_on_back, sensor_power_off_back },
	{ "KMO08BN0", sensor_power_on_back, sensor_power_off_back },
	{ "SOI08BN1", sensor_power_on_back, sensor_power_off_back },
	{ "SOI13BS1", sensor_power_on_back, sensor_power_off_back },
	{ "SEM13BS0", sensor_power_on_back, sensor_power_off_back },
	{ "SOI13BS2", sensor_power_on_back, sensor_power_off_back },
	{ "SEM13BS1", sensor_power_on_back, sensor_power_off_back },
};

const struct sony_camera_module sony_camera_1_modules[] = {
	{ "GENERIC",  sensor_power_on_front, sensor_power_off_front },
	{ "LGI02BN0", sensor_power_on_imx132, sensor_power_off_imx132 },
	{ "SEM02BN0", sensor_power_on_imx132, sensor_power_off_imx132 }
};

const struct sony_camera_info camera_info[] = {
	{
		.i2c_addr = 0x10,
		.eeprom_addr = 0x50,
		.eeprom_type = 0,
		.eeprom_max_len = 2048,
		.gpio_af = 0,
		.subdev_code = V4L2_MBUS_FMT_SBGGR10_1X10,
		.modules = sony_camera_0_modules,
		.modules_num = ARRAY_SIZE(sony_camera_0_modules),
		.default_module_name = "SEM13BS1",
	},
	{
		.i2c_addr = 0x36,
		.eeprom_addr = 0x50,
		.eeprom_type = 0,
		.eeprom_max_len = 1024,
		.gpio_af = 0,
		.subdev_code = V4L2_MBUS_FMT_SBGGR10_1X10,
		.modules = sony_camera_1_modules,
		.modules_num = ARRAY_SIZE(sony_camera_1_modules),
		.default_module_name = "SEM02BN0",
	}
};

#endif
#endif
