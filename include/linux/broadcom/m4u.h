/*
 * include/linux/broadcom/m4u.h
 *
 * Copyright (C) 2011 Broadcom, Inc.
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

#ifndef _LINUX_M4U_H
#define _LINUX_M4U_H

#include <linux/scatterlist.h>
#include <linux/uaccess.h>

struct m4u_region {
	u32 pa;
	u32 mma;
	u32 size;
	u32 page_size;
};

struct m4u_platform_data {
	u32 mma_start;
	u32 mma_end;
	u32 mma_sg_start;
	u32 xfifo_size;
	int nr;
	struct m4u_region *regions;
};

struct m4u_device;
extern struct m4u_device *g_mdev;

int m4u_add_region(struct m4u_device *mdev, struct m4u_region *region);
int m4u_map(struct m4u_device *mdev, struct sg_table *sgt, u32 size, u32 align);
void m4u_unmap(struct m4u_device *mdev, u32 mma, u32 size);


#endif /* _LINUX_M4U_H */

