/* drivers/video/msm/mipi_renesas_r63306.c
 *
 * Copyright (C) [2011] Sony Ericsson Mobile Communications AB.
 * Author: Yosuke Hatanaka <yosuke.hatanaka@sonyericsson.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2; as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include "msm_fb.h"
#include "mipi_dsi.h"
#include "mipi_dsi_panel.h"

static int mipi_r63306_disp_on(struct msm_fb_data_type *mfd)
{
	int ret = 0;
	struct mipi_dsi_data *dsi_data;
	struct dsi_controller *pctrl;

	dsi_data = platform_get_drvdata(mfd->panel_pdev);
	if (!dsi_data || !dsi_data->lcd_power) {
		ret = -ENODEV;
		goto disp_on_fail;
	}
	pctrl = dsi_data->panel->pctrl;

	if (!dsi_data->panel_detecting) {
		ret = dsi_data->lcd_power(TRUE);

		if (ret)
			goto disp_on_fail;

		mipi_dsi_op_mode_config(DSI_CMD_MODE);

		if (pctrl->display_init_cmds) {
			mipi_dsi_buf_init(&dsi_data->tx_buf);
			mipi_dsi_cmds_tx(&dsi_data->tx_buf,
				pctrl->display_init_cmds,
				pctrl->display_init_cmds_size);
		}
		if (dsi_data->eco_mode_on && pctrl->display_on_eco_cmds) {
			mipi_dsi_buf_init(&dsi_data->tx_buf);
			mipi_dsi_cmds_tx(&dsi_data->tx_buf,
				pctrl->display_on_eco_cmds,
				pctrl->display_on_eco_cmds_size);
			dev_info(&mfd->panel_pdev->dev, "ECO MODE ON\n");
		} else {
			mipi_dsi_buf_init(&dsi_data->tx_buf);
			mipi_dsi_cmds_tx(&dsi_data->tx_buf,
				pctrl->display_on_cmds,
				pctrl->display_on_cmds_size);
			dev_info(&mfd->panel_pdev->dev, "ECO MODE OFF\n");
		}
	}

disp_on_fail:
	return ret;
}

static int mipi_r63306_disp_off(struct msm_fb_data_type *mfd)
{
	int ret = 0;
	struct mipi_dsi_data *dsi_data;

	dsi_data = platform_get_drvdata(mfd->panel_pdev);

	if (!dsi_data || !dsi_data->lcd_power)
		return -ENODEV;

	if (!dsi_data->panel_detecting) {
		mipi_dsi_op_mode_config(DSI_CMD_MODE);

		mipi_dsi_buf_init(&dsi_data->tx_buf);
		mipi_dsi_cmds_tx(&dsi_data->tx_buf,
			dsi_data->panel->pctrl->display_off_cmds,
			dsi_data->panel->pctrl->display_off_cmds_size);
		ret = dsi_data->lcd_power(FALSE);
	} else {
		dsi_data->panel_detecting = false;
		ret = 0;
	}

	return ret;
}

static int mipi_r63306_lcd_on(struct platform_device *pdev)
{
	int ret;
	struct msm_fb_data_type *mfd;

	mfd = platform_get_drvdata(pdev);
	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;

	ret = mipi_r63306_disp_on(mfd);
	if (ret)
		dev_err(&pdev->dev, "%s: Display on failed\n", __func__);

	return ret;
}

static int mipi_r63306_lcd_off(struct platform_device *pdev)
{
	int ret;
	struct msm_fb_data_type *mfd;

	mfd = platform_get_drvdata(pdev);
	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;

	ret = mipi_r63306_disp_off(mfd);
	if (ret)
		dev_err(&pdev->dev, "%s: Display off failed\n", __func__);

	return ret;
}

static int __devexit mipi_r63306_lcd_remove(struct platform_device *pdev)
{
	struct mipi_dsi_data *dsi_data;

	dsi_data = platform_get_drvdata(pdev);
	if (!dsi_data)
		return -ENODEV;

#ifdef CONFIG_DEBUG_FS
	mipi_dsi_debugfs_exit(pdev);
#endif

	platform_set_drvdata(pdev, NULL);
	mipi_dsi_buf_release(&dsi_data->tx_buf);
	mipi_dsi_buf_release(&dsi_data->rx_buf);
	kfree(dsi_data);
	return 0;
}

static int __devinit mipi_r63306_lcd_probe(struct platform_device *pdev)
{
	int ret;
	struct lcd_panel_platform_data *platform_data;
	struct mipi_dsi_data *dsi_data;
#ifdef CONFIG_FB_MSM_PANEL_ECO_MODE
	struct platform_device *fb_pdev;
#endif

	platform_data = pdev->dev.platform_data;
	if (platform_data == NULL)
		return -EINVAL;

	dsi_data = kzalloc(sizeof(struct mipi_dsi_data), GFP_KERNEL);
	if (dsi_data == NULL)
		return -ENOMEM;

	dsi_data->panel_data.on = mipi_r63306_lcd_on;
	dsi_data->panel_data.off = mipi_r63306_lcd_off;
	dsi_data->default_panels = platform_data->default_panels;
	dsi_data->panels = platform_data->panels;
	dsi_data->lcd_power = platform_data->lcd_power;
	dsi_data->lcd_reset = platform_data->lcd_reset;
	if (mipi_dsi_need_detect_panel(dsi_data->panels)) {
		dsi_data->panel_data.panel_detect = mipi_dsi_detect_panel;
		dsi_data->panel_data.update_panel = mipi_dsi_update_panel;
		dsi_data->panel_detecting = true;
	} else {
		dev_info(&pdev->dev, "no need to detect panel\n");
	}

	ret = mipi_dsi_buf_alloc(&dsi_data->tx_buf, DSI_BUF_SIZE);
	if (ret <= 0) {
		dev_err(&pdev->dev, "mipi_dsi_buf_alloc(tx) failed!\n");
		goto error1;
	}

	ret = mipi_dsi_buf_alloc(&dsi_data->rx_buf, DSI_BUF_SIZE);
	if (ret <= 0) {
		dev_err(&pdev->dev, "mipi_dsi_buf_alloc(rx) failed!\n");
		goto error2;
	}

	platform_set_drvdata(pdev, dsi_data);

	mipi_dsi_set_default_panel(dsi_data);

	ret = platform_device_add_data(pdev, &dsi_data->panel_data,
		sizeof(dsi_data->panel_data));
	if (ret) {
		dev_err(&pdev->dev,
			"platform_device_add_data failed!\n");
		goto error3;
	}
#ifdef CONFIG_FB_MSM_PANEL_ECO_MODE
	fb_pdev = msm_fb_add_device(pdev);
	eco_mode_sysfs_register(&fb_pdev->dev);
#else
	msm_fb_add_device(pdev);
#endif

#ifdef CONFIG_DEBUG_FS
	mipi_dsi_debugfs_init(pdev, "mipi_r63306");
#endif

	return 0;
error3:
	mipi_dsi_buf_release(&dsi_data->rx_buf);
error2:
	mipi_dsi_buf_release(&dsi_data->tx_buf);
error1:
	kfree(dsi_data);
	return ret;
}

static struct platform_driver this_driver = {
	.probe  = mipi_r63306_lcd_probe,
	.remove = mipi_r63306_lcd_remove,
	.driver = {
		.name   = "mipi_renesas_r63306",
	},
};

static int __init mipi_r63306_lcd_init(void)
{
	return platform_driver_register(&this_driver);
}

static void __exit mipi_r63306_lcd_exit(void)
{
	platform_driver_unregister(&this_driver);
}

module_init(mipi_r63306_lcd_init);
module_exit(mipi_r63306_lcd_exit);

