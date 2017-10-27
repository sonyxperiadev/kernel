/*
 * Restructured unified display panel driver for Xperia Open Devices
 *                     *** Panel Detection ***
 *
 * This driver is based on various SoMC implementations found in
 * copyleft archives for various devices.
 *
 * Copyright (c) 2012-2014, The Linux Foundation. All rights reserved.
 * Copyright (c) Sony Mobile Communications Inc. All rights reserved.
 * Copyright (C) 2014-2016, AngeloGioacchino Del Regno <kholk11@gmail.com>
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

#include <linux/module.h>
#include <linux/delay.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/of_platform.h>
#include <linux/gpio.h>

#include "../mdss_dsi.h"
#include "somc_panels.h"

#define PANEL_SKIP_ID		0xff
#define ADC_RNG_MIN		0
#define ADC_RNG_MAX		1
#define ADC_PNUM		2

static uint32_t lcdid_adc;
static bool use_adc_detection;
static bool cmd_det_quirk;

static int __init lcdid_adc_setup(char *str)
{
	unsigned long res;

	if (!*str)
		return 0;
	if (!kstrtoul(str, 0, &res)) {
		lcdid_adc = res;
		use_adc_detection = true;
	}

	return 1;
}
__setup("lcdid_adc=", lcdid_adc_setup);

static int panel_detect_setup(struct device_node **node,
			struct mdss_panel_specific_pdata *spec_pdata,
			struct platform_device *ctrl_pdev)
{
	struct device_node *parent = of_get_parent(*node);
	int rc, lcd_id;

	if (of_machine_is_compatible("somc,fih-board")) {
		spec_pdata->driver_ic = DRIVER_IC_FIH;
		spec_pdata->lcd_id = spec_pdata->driver_ic;
		return 0;
	}

	spec_pdata->driver_ic = PANEL_DRIVER_IC_NONE;

	lcd_id = of_get_named_gpio(parent, "somc,dric-gpio", 0);
	if (!gpio_is_valid(lcd_id)) {
		pr_err("%s:%d, DriverIC gpio not specified\n",
						__func__, __LINE__);
		if (of_machine_is_compatible("somc,amami-row"))
			return 0;

		return -ENODEV;
	}

	rc = gpio_request(lcd_id, "lcd_id");
	if (rc) {
		pr_err("%s: request lcd id gpio failed, rc=%d\n",
			__func__, rc);
		return rc;
	}

	rc = gpio_direction_input(lcd_id);
	if (rc) {
		pr_err("%s: set_direction for lcd_id gpio failed, rc=%d\n",
			__func__, rc);
		goto out;
	}
	usleep_range(20, 30);

	mdss_dsi_panel_power_detect(ctrl_pdev, 1);
	spec_pdata->driver_ic = gpio_get_value(lcd_id);
	mdss_dsi_panel_power_detect(ctrl_pdev, 0);

	pr_info("%s: DriverIC GPIO: %d\n", __func__, spec_pdata->driver_ic);

	spec_pdata->lcd_id = spec_pdata->driver_ic;

out:
	gpio_free(lcd_id);
	return rc;
}


static int dric_panel_detect(struct device_node **node,
				struct mdss_panel_specific_pdata *spec_pdata)
{
	struct device_node *parent;
	struct device_node *next;
	int rc = -EINVAL, dric = -EINVAL;

	parent = of_get_parent(*node);

	for_each_child_of_node(parent, next) {
		rc = of_property_read_u32(next,
				"somc,driver-ic", &dric);
		if (spec_pdata->driver_ic != dric)
			continue;

		break;
	}

	*node = next;
	spec_pdata->detected = true;

	return rc;
}

static char dcs_cmd_DA[2] = {0xDA, 0x00}; /* DTYPE_DCS_READ */

static void panel_id_store(int data)
{
	pr_debug("panel_id_store read data: 0x%x\n", data);
}

static int cmd_panel_detect(struct mdss_panel_data *pdata)
{
	struct device_node *np, *parent, *next;
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	struct mdss_panel_specific_pdata *spec_pdata = NULL;
	const char *data;
	char *rx_data;
	u32 tmp;
	int i, len, rc = 0;

	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);

	spec_pdata = ctrl_pdata->spec_pdata;
	if (!spec_pdata) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	if (!spec_pdata->panel_detect)
		return 0;

	/* Quirk: DSI has to send bad RX for the next code to work */
	if (cmd_det_quirk)
		mdss_dsi_panel_cmd_read(ctrl_pdata, dcs_cmd_DA[0], dcs_cmd_DA[1],
						panel_id_store, ctrl_pdata->rx_buf.data, 1);

	pr_debug("%s: Panel ID: ", __func__);
	mdss_dsi_op_mode_config(DSI_CMD_MODE, pdata);
	mdss_dsi_cmds_rx(ctrl_pdata,
			 spec_pdata->id_read_cmds.cmds, 10, 0);

	pr_debug("0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X",
		ctrl_pdata->rx_buf.data[0], ctrl_pdata->rx_buf.data[1],
		ctrl_pdata->rx_buf.data[2], ctrl_pdata->rx_buf.data[3],
		ctrl_pdata->rx_buf.data[4], ctrl_pdata->rx_buf.data[5],
		ctrl_pdata->rx_buf.data[6], ctrl_pdata->rx_buf.data[7]);

	np = of_parse_phandle(
			pdata->panel_pdev->dev.of_node,
			"qcom,dsi-pref-prim-pan", 0);

	if (of_machine_is_compatible("somc,tulip"))
		ctrl_pdata->rx_buf.data[0] = ctrl_pdata->rx_buf.data[2];

	rx_data = ctrl_pdata->rx_buf.data;

	parent = of_get_parent(np);

	for_each_child_of_node(parent, next) {
		rc = of_property_read_u32(next, "somc,driver-ic", &tmp);
		if (rc)
			continue;

		if (spec_pdata->driver_ic != tmp)
			continue;

		data = of_get_property(next, "somc,panel-id", &len);
		if (!data) {
			pr_err("%s:%d, panel not read\n",
				__func__, __LINE__);
			return -EINVAL;
		}
		if (data && rx_data) {
			rc = 0;
			if ((len == 1) && (data[0] == PANEL_SKIP_ID))
				continue;
			for (i = 0; i < len; i++) {
				pr_debug("Read data:0x%02X DT data:0x%02X ",
					rx_data[i], data[i]);
				if ((rx_data[i] != data[i]) &&
					(data[i] != PANEL_SKIP_ID))
				{
					rc = -ENODEV;
					break;
				}
				/* Data matches, increment count */
				rc++;
			}
			/* If ALL data is matching, it's our panel */
			if (rc == len)
				goto parse;
			else
				continue;
		} else if (data && !rx_data) {
			if ((len != 1) || (data[0] != PANEL_SKIP_ID))
				continue;
			else {
				rc = of_property_read_u32(next,
					"somc,panel-detect", &tmp);
				spec_pdata->panel_detect =
						!rc ? tmp : 0;
			}
		}
	}
parse:
	if (unlikely(rc != len)) {
		pr_err("%s: WARNING: Cannot detect panel." \
			" Falling back to the default entry!\n", __func__);
		next = np;
	} else
		pr_info("%s: Panel detected!\n", __func__);

	spec_pdata->detected = true;
	rc = mdss_panel_parse_dt(next, ctrl_pdata);

	return rc;
}

static int postdetect_update_panel(struct mdss_panel_data *pdata)
{
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	struct mdss_panel_specific_pdata *spec_pdata = NULL;
	struct mdss_panel_info *pinfo;
	struct mipi_panel_info *mipi;
	int rc = 0;

	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);

	spec_pdata = ctrl_pdata->spec_pdata;
	if (!spec_pdata) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	if (!spec_pdata->panel_detect)
		return 0;

	pinfo = &(ctrl_pdata->panel_data.panel_info);

	mipi  = &(pinfo->mipi);

	pinfo->type =
		((mipi->mode == DSI_VIDEO_MODE)
			? MIPI_VIDEO_PANEL : MIPI_CMD_PANEL);

	rc = mdss_dsi_clk_div_config(pinfo, mipi->frame_rate);
	if (rc) {
		pr_err("%s: unable to initialize the clk dividers\n",
								__func__);
		return rc;
	}

	ctrl_pdata->pclk_rate = mipi->dsi_pclk_rate;
	ctrl_pdata->byte_clk_rate = pinfo->clk_rate / 8;

	spec_pdata->reset(pdata, 1);

	return 0;
}

static int adc_panel_detect(struct device_node **node,
		struct mdss_panel_specific_pdata *spec_pdata,
		u32 dev_index)
{
	struct device_node *parent;
	struct device_node *next;
	u32 res[ADC_PNUM], index, scaling = 1;
	int rc = 0;

	parent = of_get_parent(*node);

	rc = of_property_read_u32(parent,
			"somc,mul-channel-scaling", &scaling);
	if (rc)
		pr_err("%s: Unable to read somc,mul-channel-scaling\n",
				__func__);

	spec_pdata->adc_uv = lcdid_adc * scaling;
	pr_info("%s: Found panel ADC: %d\n", __func__, spec_pdata->adc_uv);

	for_each_child_of_node(parent, next) {
		rc = of_property_read_u32(next, "somc,dsi-index", &index);
		if (rc)
			index = 0;
		if (index != dev_index)
			continue;
		rc = of_property_read_u32(next, "somc,lcd-id", res);
		if (rc || *res != spec_pdata->lcd_id)
			continue;
		rc = of_property_read_u32_array(next,
				"somc,lcd-id-adc", res, ADC_PNUM);
		if (rc || spec_pdata->adc_uv < res[ADC_RNG_MIN] ||
				res[ADC_RNG_MAX] < spec_pdata->adc_uv)
			continue;
		break;
	}
	*node = next;
	spec_pdata->detected = true;

	return rc;
}

int do_panel_detect(struct device_node **node,
		struct platform_device *pdev,
		struct mdss_dsi_ctrl_pdata *ctrl_pdata,
		bool cmd_detect_quirk, u32 cell_idx)
{
	int rc = 0;
	bool use_cmd_detect =
		of_property_read_bool(*node, "somc,panel-id-read-cmds") &&
		!use_adc_detection;
	bool use_dric_only =
		of_property_read_bool(*node, "somc,dric-only-detect");

	rc = panel_detect_setup(node,
			 ctrl_pdata->spec_pdata, pdev);
	if (rc < 0)
		pr_err("%s: WARNING: Panel detection LCD ID setup failed.\n",
			__func__);


	if (use_dric_only) {
		pr_info("%s: Starting DriverIC-only " \
			"Panel Detection...\n", __func__);
		rc = dric_panel_detect(node, ctrl_pdata->spec_pdata);
		if (rc < 0) {
			pr_err("%s: DrIC Detection failed!!!\n",
				__func__);
			goto end;
		}
	} else if (!use_cmd_detect && !cmd_detect_quirk) {
		pr_info("%s: Starting ADC Panel Detection...\n", __func__);
		rc = adc_panel_detect(node, ctrl_pdata->spec_pdata, cell_idx);
		if (rc < 0) {
			pr_err("%s: ADC Detection failed!!!\n",
				__func__);
			goto end;
		}
	} else {
		pr_info("%s: Setting up for CMD Panel Detection...\n",
				__func__);
		cmd_det_quirk = cmd_detect_quirk;
		ctrl_pdata->spec_pdata->detect = cmd_panel_detect;
		ctrl_pdata->spec_pdata->update_panel = postdetect_update_panel;
	};
end:
	return rc;
}
