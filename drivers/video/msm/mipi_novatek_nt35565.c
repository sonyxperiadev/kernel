/* drivers/video/msm/mipi_novatek_nt35565.c
 *
 * Copyright (C) [2011] Sony Ericsson Mobile Communications AB.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2; as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include "msm_fb.h"
#include "mipi_dsi.h"
#include "mipi_dsi_panel.h"

static int mipi_nt35565_ic_on_disp_off(struct msm_fb_data_type *mfd)
{
	struct mipi_dsi_data *dsi_data;
	struct dsi_controller *pctrl;

	dsi_data = platform_get_drvdata(mfd->panel_pdev);
	if (!dsi_data)
		return -ENODEV;
	pctrl = dsi_data->panel->pctrl;

	if (!dsi_data->panel_detecting) {
		mipi_dsi_op_mode_config(DSI_CMD_MODE);

		if (pctrl->display_init_cmds) {
			mipi_dsi_buf_init(&dsi_data->tx_buf);
			mipi_dsi_cmds_tx(mfd, &dsi_data->tx_buf,
				pctrl->display_init_cmds,
				pctrl->display_init_cmds_size);
		}
	}

	return 0;
}

static int mipi_nt35565_ic_on_disp_on(struct msm_fb_data_type *mfd)
{
	struct mipi_dsi_data *dsi_data;
	struct dsi_controller *pctrl;

	dsi_data = platform_get_drvdata(mfd->panel_pdev);
	if (!dsi_data)
		return -ENODEV;
	pctrl = dsi_data->panel->pctrl;

	if (!dsi_data->panel_detecting) {
		mipi_dsi_op_mode_config(DSI_CMD_MODE);

		if (dsi_data->eco_mode_on && pctrl->display_on_eco_cmds) {
			mipi_dsi_buf_init(&dsi_data->tx_buf);
			mipi_dsi_cmds_tx(mfd, &dsi_data->tx_buf,
				pctrl->display_on_eco_cmds,
				pctrl->display_on_eco_cmds_size);
			dev_info(&mfd->panel_pdev->dev, "ECO MODE ON\n");
		} else {
			mipi_dsi_buf_init(&dsi_data->tx_buf);
			mipi_dsi_cmds_tx(mfd, &dsi_data->tx_buf,
				pctrl->display_on_cmds,
				pctrl->display_on_cmds_size);
			dev_info(&mfd->panel_pdev->dev, "ECO MODE OFF\n");
		}
	}

	return 0;
}

static int mipi_nt35565_disp_off(struct msm_fb_data_type *mfd)
{
	struct mipi_dsi_data *dsi_data;

	dsi_data = platform_get_drvdata(mfd->panel_pdev);

	if (!dsi_data)
		return -ENODEV;

	if (!dsi_data->panel_detecting) {
		mipi_dsi_op_mode_config(DSI_CMD_MODE);

		mipi_dsi_buf_init(&dsi_data->tx_buf);
		mipi_dsi_cmds_tx(mfd, &dsi_data->tx_buf,
			dsi_data->panel->pctrl->display_off_cmds,
			dsi_data->panel->pctrl->display_off_cmds_size);
	} else {
		dsi_data->panel_detecting = false;
	}

	return 0;
}

static int mipi_nt35565_ic_on_lcd_off(struct platform_device *pdev)
{
	int ret;
	struct msm_fb_data_type *mfd;

	mfd = platform_get_drvdata(pdev);
	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;

	ret = mipi_nt35565_ic_on_disp_off(mfd);
	if (ret)
		dev_err(&pdev->dev, "%s: Display on failed\n", __func__);

	return ret;
}

static int mipi_nt35565_ic_on_lcd_on(struct platform_device *pdev)
{
	int ret;
	struct msm_fb_data_type *mfd;

	mfd = platform_get_drvdata(pdev);
	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;

	ret = mipi_nt35565_ic_on_disp_on(mfd);
	if (ret)
		dev_err(&pdev->dev, "%s: Display on failed\n", __func__);

	return ret;
}

static int mipi_nt35565_lcd_off(struct platform_device *pdev)
{
	int ret;
	struct msm_fb_data_type *mfd;

	mfd = platform_get_drvdata(pdev);
	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;

	ret = mipi_nt35565_disp_off(mfd);
	if (ret)
		dev_err(&pdev->dev, "%s: Display off failed\n", __func__);

	return ret;
}

static int __devexit mipi_nt35565_lcd_remove(struct platform_device *pdev)
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

static int __devinit mipi_nt35565_lcd_probe(struct platform_device *pdev)
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

	dsi_data->panel_data.on = mipi_nt35565_ic_on_lcd_off;
	dsi_data->panel_data.controller_on_panel_on
					= mipi_nt35565_ic_on_lcd_on;
	dsi_data->panel_data.off = mipi_nt35565_lcd_off;
	dsi_data->panel_data.power_on_panel_at_pan = 0;
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
		goto out_free;
	}

	ret = mipi_dsi_buf_alloc(&dsi_data->rx_buf, DSI_BUF_SIZE);
	if (ret <= 0) {
		dev_err(&pdev->dev, "mipi_dsi_buf_alloc(rx) failed!\n");
		goto out_rx_release;
	}

	platform_set_drvdata(pdev, dsi_data);

	mipi_dsi_set_default_panel(dsi_data);

	ret = platform_device_add_data(pdev, &dsi_data->panel_data,
		sizeof(dsi_data->panel_data));
	if (ret) {
		dev_err(&pdev->dev,
			"platform_device_add_data failed!\n");
		goto out_tx_release;
	}
#ifdef CONFIG_FB_MSM_PANEL_ECO_MODE
	fb_pdev = msm_fb_add_device(pdev);
	eco_mode_sysfs_register(&fb_pdev->dev);
#else
	msm_fb_add_device(pdev);
#endif

#ifdef CONFIG_DEBUG_FS
	mipi_dsi_debugfs_init(pdev, "mipi_nt35565");
#endif

	return 0;
out_tx_release:
	mipi_dsi_buf_release(&dsi_data->rx_buf);
out_rx_release:
	mipi_dsi_buf_release(&dsi_data->tx_buf);
out_free:
	kfree(dsi_data);
	return ret;
}

static struct platform_driver this_driver = {
	.probe  = mipi_nt35565_lcd_probe,
	.remove = mipi_nt35565_lcd_remove,
	.driver = {
		.name   = "mipi_novatek_nt35565",
	},
};

static int __init mipi_nt35565_lcd_init(void)
{
	return platform_driver_register(&this_driver);
}

static void __exit mipi_nt35565_lcd_exit(void)
{
	platform_driver_unregister(&this_driver);
}

module_init(mipi_nt35565_lcd_init);
module_exit(mipi_nt35565_lcd_exit);

