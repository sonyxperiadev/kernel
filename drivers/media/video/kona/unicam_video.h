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

#ifndef _KONA_UNICAM_VIDEO_H_
#define _KONA_UNICAM_VIDEO_H_

#include <linux/list.h>
#include <linux/videodev2.h>
#include <linux/spinlock.h>
#include <media/videobuf2-core.h>
#include <media/v4l2-dev.h>

#define UNICAM_VIDEO_DRIVER_NAME "unicamvideo"
#define UNICAM_BUF_MAGIC		0xBABEFACE

struct unicam_video;

enum unicam_pipeline_stream_state {
	UNICAM_PIPELINE_STREAM_STOPPED = 0,
	UNICAM_PIPELINE_STREAM_CONTINUOUS = 1,
	UNICAM_PIPELINE_STREAM_SINGLESHOT = 2,
};

struct unicam_pipeline {
	struct media_pipeline pipe;
	struct unicam_video *output;
	enum unicam_pipeline_stream_state stream_state;
	spinlock_t lock;
	atomic_t frame_number;
	bool error;
	struct v4l2_subdev *external;
};

#define to_unicam_pipeline(__e) \
	container_of((__e)->pipe, struct unicam_pipeline, pipe)

struct unicam_format_info {
	enum v4l2_mbus_pixelcode code;
	u32 pixelformat;
	unsigned int bpp;
};

struct unicam_buffer {
	struct vb2_buffer vb;
	struct list_head list;
	dma_addr_t unicam_addr;
};

struct unicam_video_fh {
	struct v4l2_fh vfh;
	struct unicam_video *video;
	struct vb2_queue queue;
	struct v4l2_format format;
};

#define to_unicam_video_fh(fh) container_of(fh, struct unicam_video_fh, vfh)

struct unicam_video {

	struct video_device video;
	struct media_pad pad;

	/* to be filed in by client */
	enum v4l2_buf_type type;
	const struct unicam_video_operations *ops;
	unsigned int bpl_alignment; /* alignment value */
	unsigned int bpl_zero_padding; /* whether alignment is optional */
	unsigned int bpl_max; /* max bytes per line value */
	unsigned int bpl_value;
	unsigned int height;
	struct unicam_device *unicam;

	/* format and crop settings */
	struct mutex mutex;
	atomic_t active;

	unsigned int streaming:1;

	/* pipeline state */
	struct unicam_pipeline pipe;
	struct mutex stream_lock;

	/* buffer related */
	struct vb2_queue *queue;
	struct list_head dmaqueue;
	spinlock_t qlock;

	struct vb2_alloc_ctx *alloc_ctx;

};

struct unicam_video_operations {
	int (*queue)(struct unicam_video *video, struct unicam_buffer *buffer);
};

int kona_unicam_video_register(struct unicam_video *video,
		struct v4l2_device *vdev);
void kona_unicam_video_unregister(struct unicam_video *video);
int kona_unicam_video_init(struct unicam_video *video, const char *name);
void kona_unicam_video_cleanup(struct unicam_video *video);
struct unicam_buffer *kona_unicam_video_buffer_next(struct unicam_video *video);
#endif /* _KONA_UNICAM_VIDEO_H_*/
