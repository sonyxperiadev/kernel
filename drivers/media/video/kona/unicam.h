/*****************************************************************************
* Copyright 2006 - 2009 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/

#ifndef _KONA_UNICAM_H_
#define _KONA_UNICAM_H_

#include <linux/device.h>
#include <media/kona-unicam.h>
#include <plat/pi_mgr.h>
#include <linux/clk.h>
#include <media/media-device.h>
#include <media/v4l2-device.h>
#include "unicam_csi2.h"

struct unicam_device {
	struct unicam_platform_data *pdata;
	struct device *dev;
	unsigned int irq_num;
	u32 revision;
	u64 raw_dmamask;

	/* ref count to enable/disable clocks */
	struct mutex unicam_mutex;
	int ref_count;

	/* clock related stuff */
	struct pi_mgr_dfs_node dfs_client;

	struct clk *csi2_axi_clk;

	/* unicam modules */
	struct unicam_csi2_device csi2a;

	/* media controller */
	struct media_device media_dev;
	struct v4l2_device v4l2_dev;
};

/* unicam core operations */
struct unicam_device *kona_unicam_get(struct unicam_device *unicam);
void kona_unicam_put(struct unicam_device *unicam);
int kona_unicam_pipeline_pm_use(struct media_entity *entity, int use);
int kona_unicam_pipeline_set_stream(struct unicam_pipeline *pipe,
		enum unicam_pipeline_stream_state state);
#endif /* _KONA_UNICAM_H_ */
