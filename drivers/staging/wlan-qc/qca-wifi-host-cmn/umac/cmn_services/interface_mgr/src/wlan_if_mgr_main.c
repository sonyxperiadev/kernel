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
 * DOC: contains core interface manager function definitions
 */
#include "wlan_if_mgr_main.h"
#include "wlan_objmgr_global_obj.h"

QDF_STATUS wlan_if_mgr_init(void)
{
	QDF_STATUS status;

	status = wlan_objmgr_register_psoc_create_handler(WLAN_UMAC_COMP_IF_MGR,
		wlan_if_mgr_psoc_created_notification, NULL);
	if (QDF_IS_STATUS_ERROR(status)) {
		ifmgr_err("Failed to register psoc create handler");
		goto fail_create_psoc;
	}

	status = wlan_objmgr_register_psoc_destroy_handler(
			WLAN_UMAC_COMP_IF_MGR,
			wlan_if_mgr_psoc_destroyed_notification, NULL);
	if (QDF_IS_STATUS_ERROR(status)) {
		ifmgr_err("Failed to create psoc delete handler");
		goto fail_psoc_destroy;
	}
	ifmgr_debug("interface mgr psoc create and delete handler registered with objmgr");

	return status;

fail_psoc_destroy:
	wlan_objmgr_unregister_psoc_create_handler(WLAN_UMAC_COMP_IF_MGR,
			wlan_if_mgr_psoc_created_notification, NULL);
fail_create_psoc:
	return status;
}

QDF_STATUS wlan_if_mgr_deinit(void)
{
	QDF_STATUS status;

	status = wlan_objmgr_unregister_psoc_create_handler(
			WLAN_UMAC_COMP_IF_MGR,
			wlan_if_mgr_psoc_created_notification, NULL);
	if (QDF_IS_STATUS_ERROR(status))
		ifmgr_err("Failed to deregister psoc create handler");

	status = wlan_objmgr_unregister_psoc_destroy_handler(
			WLAN_UMAC_COMP_IF_MGR,
			wlan_if_mgr_psoc_destroyed_notification, NULL);
	if (QDF_IS_STATUS_ERROR(status))
		ifmgr_err("Failed to deregister psoc delete handler");

	ifmgr_debug("interface mgr psoc create and delete handler deregistered with objmgr");

	return status;
}

QDF_STATUS wlan_if_mgr_psoc_created_notification(struct wlan_objmgr_psoc *psoc,
						 void *arg_list)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	return status;
}

QDF_STATUS
wlan_if_mgr_psoc_destroyed_notification(struct wlan_objmgr_psoc *psoc,
					void *arg_list)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	return status;
}

