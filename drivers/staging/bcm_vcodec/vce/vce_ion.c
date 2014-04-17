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

#define pr_fmt(fmt) "vce: %s: " fmt "\n", __func__
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/err.h>
#include <linux/mutex.h>
#include <linux/ion.h>
#include <linux/broadcom/bcm_ion.h>
#include "vce_ion.h"

static DEFINE_MUTEX(vce_ion_mutex);
static struct ion_client *vce_ion_client;

struct ion_client *vce_ion_get_client(void)
{
	struct ion_client *client;

	/* idev might not be initialised early enough to use __init */
	mutex_lock(&vce_ion_mutex);

	client = vce_ion_client;
	if (!client) {
		BUG_ON(!idev);

		client = ion_client_create(idev, "vce");
		if (IS_ERR(client))
			pr_warn("bcm_ion_client_create failed (errno %ld)",
				PTR_ERR(client));
		else
			vce_ion_client = client;
	}

	mutex_unlock(&vce_ion_mutex);

	return client;
}

int vce_ion_get_dma_addr(struct ion_client *client, struct ion_handle *handle,
			 size_t size, dma_addr_t *addr)
{
	unsigned long da;

	/*
	 * Unused here, but kept in the function prototype because it is used in
	 * the version for newer chips.
	 */
	(void)size;

	da = bcm_ion_map_dma(client, handle);
	if (!da)
		return -ENOMEM;

	*addr = da;
	return 0;
}

static void __exit vce_ion_exit(void)
{
	if (vce_ion_client)
		ion_client_destroy(vce_ion_client);
}
