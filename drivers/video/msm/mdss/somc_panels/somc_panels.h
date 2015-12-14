/* Copyright (c) 2012-2015, The Linux Foundation. All rights reserved.
 * Copyright (C) 2014 Sony Mobile Communications Inc.
 * Copyright (c) 2015 AngeloGioacchino Del Regno <kholk11@gmail.com>
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

/* Main */
void mdss_dsi_panel_cmds_send(struct mdss_dsi_ctrl_pdata *ctrl,
			struct dsi_panel_cmds *pcmds);

int mdss_panel_parse_dt(struct device_node *np,
			struct mdss_dsi_ctrl_pdata *ctrl_pdata);

/* Detection */
int do_panel_detect(struct device_node **node,
		struct platform_device *pdev,
		struct mdss_dsi_ctrl_pdata *ctrl_pdata,
		bool cmd_detect_quirk, u32 cell_idx);

/* Polling */
int panel_polling_init(struct mdss_dsi_ctrl_pdata *ctrl_pdata,
			bool cont_splash_on, bool is_dsi_master);
