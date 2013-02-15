/*
 * drivers/media/video/videobuf2-dma-reserved.c
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

#define pr_fmt(fmt) "vb2-dma-reserved: " fmt

#include <linux/module.h>
#include <media/videobuf2-core.h>
#include <media/videobuf2-dma-reserved.h>
#include <media/videobuf2-memops.h>

#define DUMMY_ADDRESS (0xF0000000)

static void *vb2_dma_reserved_alloc(void *alloc_ctx, unsigned long size)
{
	pr_err("%s Not supported\n", __func__);
	return NULL;
}

static void vb2_dma_reserved_put(void *buf_priv)
{
	pr_err("%s Not supported\n", __func__);
}

static void *vb2_dma_reserved_vaddr(void *buf_priv)
{
	pr_err("%s Not supported\n", __func__);
	return NULL;
}

static int vb2_dma_reserved_mmap(void *buf_priv, struct vm_area_struct *vma)
{
	pr_err("%s Not supported\n", __func__);
	return -EINVAL;
}

static void *vb2_dma_reserved_cookie(void *buf_priv)
{
	pr_err("%s Not supported\n", __func__);
	return NULL;
}

static void *vb2_dma_reserved_get_userptr(void *alloc_ctx, unsigned long vaddr,
					unsigned long size, int write)
{
	pr_debug("%s\n", __func__);
	return (void *)DUMMY_ADDRESS;
}

static void vb2_dma_reserved_put_userptr(void *mem_priv)
{
	pr_debug("%s\n", __func__);
}

const struct vb2_mem_ops vb2_dma_reserved_memops = {
	.get_userptr	= vb2_dma_reserved_get_userptr,
	.put_userptr	= vb2_dma_reserved_put_userptr,
	.alloc		= vb2_dma_reserved_alloc,
	.put		= vb2_dma_reserved_put,
	.cookie		= vb2_dma_reserved_cookie,
	.vaddr		= vb2_dma_reserved_vaddr,
	.mmap		= vb2_dma_reserved_mmap,
};
EXPORT_SYMBOL_GPL(vb2_dma_reserved_memops);

MODULE_DESCRIPTION("Dma address handling routines for videobuf2");
MODULE_AUTHOR("Nishanth Peethambaran");
MODULE_LICENSE("GPL");
