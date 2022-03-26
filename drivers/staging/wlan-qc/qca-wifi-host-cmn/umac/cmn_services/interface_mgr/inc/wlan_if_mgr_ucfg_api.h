/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * DOC: contains interface manager public api
 */

#ifndef _WLAN_IF_MGR_UCFG_API_H_
#define _WLAN_IF_MGR_UCFG_API_H_

#include <scheduler_api.h>
#include <wlan_objmgr_psoc_obj.h>
#include <wlan_objmgr_pdev_obj.h>
#include <wlan_objmgr_vdev_obj.h>
#include "wlan_if_mgr_api.h"

/**
 * ucfg_if_mgr_deliver_event() - interface mgr event handler
 * @vdev: vdev object
 * @event: interface mangaer event
 * @event_data: Interface mgr event data
 *
 * Return: QDF_STATUS
 */
static inline
QDF_STATUS ucfg_if_mgr_deliver_event(struct wlan_objmgr_vdev *vdev,
				     enum wlan_if_mgr_evt event,
				     struct if_mgr_event_data *event_data)
{
	return if_mgr_deliver_event(vdev, event, event_data);
}

#endif
