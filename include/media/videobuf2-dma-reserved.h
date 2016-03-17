/*
 * include/media/videobuf2-dma-reserved.h
 *
 * Copyright (C) 2012 Broadcom, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef _MEDIA_VIDEOBUF2_DMA_RESERVED_H
#define _MEDIA_VIDEOBUF2_DMA_RESERVED_H

#include <media/videobuf2-core.h>
#define V4L2_BUF_FLAG_DMA_RESERVED	(0x8000)

static inline dma_addr_t
vb2_dma_reserved_plane_dma_addr(struct vb2_buffer *vb, unsigned int plane_no)
{
	if (vb == NULL) {
		pr_err("ERROR: vb2_buffer is NULL in vb2-dma-reserved\n");
		return 0;
	}

	if (plane_no != 0) {
		pr_err("ERROR: Plane number(%d) >0 not supported in vb2-dma-reserved\n",
				plane_no);
		return 0;
	}

#if 0
	if ((vb->v4l2_buf.flags & V4L2_BUF_FLAG_DMA_RESERVED) == 0) {
		pr_err("ERROR: Driver uses dma-reserved. v4l2_buffer flags mismatch\n");
		return 0;
	}
#endif

	return vb->v4l2_buf.reserved;
}

extern const struct vb2_mem_ops vb2_dma_reserved_memops;

#endif
