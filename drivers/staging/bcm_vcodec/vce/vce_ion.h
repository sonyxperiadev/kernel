/*
 * Copyright (c) 2013 Broadcom Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _VCE_ION_H_
#define _VCE_ION_H_

#include <linux/ion.h>
#include <linux/broadcom/bcm_ion.h>

#define VCE_ION_GET_HEAP_MASK()	bcm_ion_get_heapmask(ION_FLAG_256M)
#define VCE_ION_FLAGS_UNCACHED	ION_FLAG_WRITECOMBINE

struct ion_client *vce_ion_get_client(void);

int vce_ion_get_dma_addr(struct ion_client *client, struct ion_handle *handle,
			 size_t size, dma_addr_t *addr);

#endif
