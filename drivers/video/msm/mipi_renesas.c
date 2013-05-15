/* Copyright (c) 2011-2012, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "msm_fb.h"
#include "mipi_dsi.h"
#include "mipi_renesas.h"
#include <mach/socinfo.h>

#define RENESAS_CMD_DELAY 0 /* 50 */
#define RENESAS_SLEEP_OFF_DELAY 120
#define RENESAS_SLEEP_ON_DELAY 120
static struct msm_panel_common_pdata *mipi_renesas_pdata;

static struct dsi_buf renesas_tx_buf;
static struct dsi_buf renesas_rx_buf;

static int mipi_renesas_lcd_init(void);

static char config_sleep_in[2] = {0x10, 0x00};
static char config_sleep_out[2] = {0x11, 0x00};

static char config_display_off[2] = {0x28, 0x00};
static char config_display_on[2] = {0x29, 0x00};

static struct dsi_cmd_desc renesas_sleep_on_cmds[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, RENESAS_SLEEP_ON_DELAY,
		sizeof(config_sleep_in), config_sleep_in }
};

static struct dsi_cmd_desc renesas_sleep_off_cmds[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, RENESAS_SLEEP_OFF_DELAY,
		sizeof(config_sleep_out), config_sleep_out }
};

static struct dsi_cmd_desc renesas_display_off_cmds[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, RENESAS_CMD_DELAY,
		sizeof(config_display_off), config_display_off },
};

static struct dsi_cmd_desc renesas_display_on_cmds[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, RENESAS_CMD_DELAY,
		sizeof(config_display_on), config_display_on },

};

static int mipi_renesas_lcd_on(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;
	struct mipi_panel_info *mipi;

	mfd = platform_get_drvdata(pdev);
	mipi  = &mfd->panel_info.mipi;

	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;

	mipi_dsi_cmds_tx(&renesas_tx_buf, renesas_sleep_off_cmds,
			ARRAY_SIZE(renesas_sleep_off_cmds));

	mipi_set_tx_power_mode(1);
	mipi_dsi_cmds_tx(&renesas_tx_buf, renesas_display_on_cmds,
			ARRAY_SIZE(renesas_display_on_cmds));
	mipi_set_tx_power_mode(0);

	return 0;
}

static int mipi_renesas_lcd_off(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;

	mfd = platform_get_drvdata(pdev);

	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;

	mipi_dsi_cmds_tx(&renesas_tx_buf, renesas_display_off_cmds,
			ARRAY_SIZE(renesas_display_off_cmds));

	mipi_dsi_cmds_tx(mfd, &renesas_tx_buf, renesas_sleep_on_cmds,
			ARRAY_SIZE(renesas_sleep_on_cmds));

	return 0;
}

static int __devinit mipi_renesas_lcd_probe(struct platform_device *pdev)
{
	if (pdev->id == 0) {
		mipi_renesas_pdata = pdev->dev.platform_data;
		return 0;
	}

	msm_fb_add_device(pdev);

	return 0;
}

static void mipi_renesas_set_backlight(struct msm_fb_data_type *mfd)
{
	int ret = -EPERM;
	int bl_level;

	bl_level = mfd->bl_level;

	if (mipi_renesas_pdata && mipi_renesas_pdata->pmic_backlight)
		ret = mipi_renesas_pdata->pmic_backlight(bl_level);
	else
		pr_err("%s(): Backlight level set failed", __func__);
}

static struct platform_driver this_driver = {
	.probe  = mipi_renesas_lcd_probe,
	.driver = {
		.name   = "mipi_renesas",
	},
};

static struct msm_fb_panel_data renesas_panel_data = {
	.on		= mipi_renesas_lcd_on,
	.off	= mipi_renesas_lcd_off,
	.set_backlight = mipi_renesas_set_backlight,
};

static int ch_used[3];

int mipi_renesas_device_register(struct msm_panel_info *pinfo,
					u32 channel, u32 panel)
{
	struct platform_device *pdev = NULL;
	int ret;
	if ((channel >= 3) || ch_used[channel])
		return -ENODEV;

	ch_used[channel] = TRUE;

	ret = mipi_renesas_lcd_init();
	if (ret) {
		pr_err("mipi_renesas_lcd_init() failed with ret %u\n", ret);
		return ret;
	}

	pdev = platform_device_alloc("mipi_renesas", (panel << 8)|channel);
	if (!pdev)
		return -ENOMEM;

	renesas_panel_data.panel_info = *pinfo;

	ret = platform_device_add_data(pdev, &renesas_panel_data,
		sizeof(renesas_panel_data));
	if (ret) {
		pr_err("%s: platform_device_add_data failed!\n", __func__);
		goto err_device_put;
	}

	ret = platform_device_add(pdev);
	if (ret) {
		pr_err("%s: platform_device_register failed!\n", __func__);
		goto err_device_put;
	}

	return 0;

err_device_put:
	platform_device_put(pdev);
	return ret;
}

static int mipi_renesas_lcd_init(void)
{
	mipi_dsi_buf_alloc(&renesas_tx_buf, DSI_BUF_SIZE);
	mipi_dsi_buf_alloc(&renesas_rx_buf, DSI_BUF_SIZE);

	return platform_driver_register(&this_driver);
}
