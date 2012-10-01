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

#ifndef _KONA_UNICAM_CSI2_H_
#define _KONA_UNICAM_CSI2_H_

#include <linux/v4l2-mediabus.h>
#include <media/v4l2-subdev.h>
#include <media/media-entity.h>
#include <linux/broadcom/mobcom_types.h>
#include <plat/csl/csl_cam.h>
#include "unicam_video.h"

#define CSI2_PAD_SINK		0
#define CSI2_PAD_SOURCE		1
#define CSI2_PADS_NUM		2

struct unicam_csi2_device {
	struct v4l2_subdev subdev;
	struct media_pad pads[CSI2_PADS_NUM];
	struct v4l2_mbus_framefmt formats[CSI2_PADS_NUM];

	struct unicam_video video_out;
	struct unicam_device *unicam;

	unsigned int frame_skip;
	CSL_CAM_HANDLE cslCamHandle;
	struct unicam_buffer *active_buf;

	/* state */
	enum unicam_pipeline_stream_state state;
};


void kona_unicam_csi2_isr(struct unicam_csi2_device *csi2);
int kona_unicam_csi2_register_entities(struct unicam_csi2_device *csi2,
		struct v4l2_device *vdev);
void kona_unicam_csi2_unregister_entities(struct unicam_csi2_device *csi2);
int kona_unicam_csi2_init(struct unicam_device *unicam);
void kona_unicam_csi2_cleanup(struct unicam_device *unicam);

#endif /* _KONA_UNICAM_CSI2_H_ */
