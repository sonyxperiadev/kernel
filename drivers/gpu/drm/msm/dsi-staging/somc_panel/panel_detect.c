/*
 * Restructured unified display panel driver for Xperia Open Devices
 *                     *** Panel Detection ***
 *
 * This driver is based on various SoMC implementations found in
 * copyleft archives for various devices.
 *
 * Copyright (c) 2012-2014, The Linux Foundation. All rights reserved.
 * Copyright (c) Sony Mobile Communications Inc. All rights reserved.
 * Copyright (C) 2014-2018, AngeloGioacchino Del Regno <kholk11@gmail.com>
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

#include <linux/sysfs.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/notifier.h>
#include <linux/export.h>
#include "somc_panel_exts.h"
#include "dsi_display.h"
#include "dsi_panel.h"

#define PANEL_SKIP_ID		0xff
#define ADC_RNG_MIN		0
#define ADC_RNG_MAX		1
#define ADC_PNUM		2

static uint32_t lcdid_adc = 0;
static bool use_adc_detection;

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

static int adc_panel_detect(struct dsi_display *display,
			    struct device_node **node, u32 dev_index)
{
	u32 res[ADC_PNUM];
	int rc = -ENOENT;
	struct device_node *this = *node;
	struct device_node *next;
	int i, count;
	u32 dsi_index = 0;
	u32 adc_uv = 0;

	adc_uv = lcdid_adc;
	pr_info("%s: Found panel ADC: %d\n", __func__, adc_uv);

	count = of_count_phandle_with_args(this, "somc,dsi-panel-list",  NULL);
	pr_info("Found %d panels in DT!\n", count);

	for (i = 0; i < count; i++) {
		next = of_parse_phandle(this, "somc,dsi-panel-list", i);

		rc = of_property_read_u32(next, "somc,dsi-index", &dsi_index);
		if (rc)
			dsi_index = 0;
		if (dsi_index != dev_index)
			continue;

		rc = of_property_read_u32_array(next,
				"somc,lcd-id-adc", res, ADC_PNUM);
		if (rc || adc_uv < res[ADC_RNG_MIN] ||
					res[ADC_RNG_MAX] < adc_uv)
			continue;

		display->panel_of = next;

		/* If we have just detected the default panel, go on */
		if (res[ADC_RNG_MIN] == 0 && res[ADC_RNG_MAX] == 0x7fffffff)
			continue;
		break;
	}

	return rc;
}

static inline int single_panel_setup(struct dsi_display *display,
			    struct device_node **node)
{
	struct device_node *this = *node;
	struct device_node *pan =
		of_parse_phandle(this, "qcom,dsi-panel", 0);

	if (pan == NULL) {
		pr_err("%s: Cannot find phandle to DSI panel!!!\n", __func__);
		return -ENOENT;
	}

	display->panel_of = pan;

	return 0;
}

int somc_panel_detect(struct dsi_display *display,
		      struct device_node **node, u32 cell_idx)
{
	bool use_cmd_detect =
		of_property_read_bool(*node, "somc,panel-id-read-cmds") &&
		!use_adc_detection;
	bool use_dric_only =
		of_property_read_bool(*node, "somc,dric-only-detect");

	int rc = 0;

	if (of_property_read_bool(*node, "somc,bootloader-panel-detect")) {
		pr_err("Bootloader gives us the panel name. Nice job!\n");
		return single_panel_setup(display, node);
	};

	if (use_cmd_detect || use_dric_only) {
		pr_err("Command detection or DrIC detection not yet supported "
		       "in this version of the somc_panel driver.\n");
		rc = -EINVAL;
		goto end;
	}

	pr_info("%s: Starting ADC Panel Detection...\n", __func__);
	rc = adc_panel_detect(display, node, cell_idx);
	if (rc < 0) {
		pr_err("%s: ADC Detection failed!!!\n", __func__);
		goto end;
	}
end:
	return rc;
}
