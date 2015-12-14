/*
 * Author: Michikatsu Kimura <michikatsu.x.kimura@sonymobile.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */
/*
 * Copyright (C) 2014 Sony Mobile Communications Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation.
 */

#ifndef MDSS_DSI_PANEL_DEBUGFS_H
#define MDSS_DSI_PANEL_DEBUGFS_H

#include <linux/wait.h>

struct blackscreen_det {
	u16 threshold;
	u16 cnt_timeout;
	u16 cnt_crash;
	ktime_t timestamp;
	int done;
	wait_queue_head_t wait_queue;
};

struct first_frame_flushed_det {
	ktime_t timestamp;
	int done;
	wait_queue_head_t wait_queue;
};

u32 panel_cmd_read(struct mdss_dsi_ctrl_pdata *ctrl,
		struct dsi_cmd_desc *cmds, void (*fxn)(int),
		char *rbuf, int len);
#endif /* MDSS_DSI_PANEL_DEBUGFS_H */
