/*
 * Copyright (c) 2019 The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * DOC: wlan_vdev_mgr_rx_ops.h
 *
 * API declarations to handle WMI response event corresponding to vdev mgmt
 */

#ifndef __WLAN_VDEV_MGR_RX_OPS_H__
#define __WLAN_VDEV_MGR_RX_OPS_H__

#include <wlan_objmgr_vdev_obj.h>
#include <wlan_vdev_mgr_tgt_if_rx_defs.h>

/**
 * tgt_vdev_mgr_register_rx_ops() - API to register rx ops with lmac
 * @rx_ops: rx ops struct
 *
 * Return: none
 */
void tgt_vdev_mgr_register_rx_ops(struct wlan_lmac_if_rx_ops *rx_ops);

/**
 * tgt_vdev_mgr_ext_tbttoffset_update_handle() - API to handle ext tbtt offset
 * update event
 * @num_vdevs: number of vdevs
 * @is_ext: ext is set/reset
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS
tgt_vdev_mgr_ext_tbttoffset_update_handle(uint32_t num_vdevs, bool is_ext);

/**
 * tgt_vdev_mgr_get_response_timer_info() - API to get vdev_mgr timer info
 * @psoc: objmgr psoc object
 * @vdev_id: vdev id
 *
 * Return: struct vdev_response_timer on success else NULL
 */
struct vdev_response_timer *
tgt_vdev_mgr_get_response_timer_info(struct wlan_objmgr_psoc *psoc,
				     uint8_t vdev_id);

#endif /* __WLAN_VDEV_MGR_RX_OPS_H__ */
