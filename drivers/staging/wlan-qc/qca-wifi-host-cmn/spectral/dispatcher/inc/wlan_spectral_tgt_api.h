/*
 * Copyright (c) 2017-2020 The Linux Foundation. All rights reserved.
 *
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

#ifndef _WLAN_SPECTRAL_TGT_API_H_
#define _WLAN_SPECTRAL_TGT_API_H_

#include <wlan_objmgr_cmn.h>
#include <qdf_types.h>
#include "../../core/spectral_cmn_api_i.h"

/**
 * tgt_get_pdev_target_handle() - Get pdev target_if handle
 * @pdev: Pointer to pdev
 *
 * Get handle to pdev target_if internal Spectral data
 *
 * Return: Handle to pdev target_if internal Spectral data on success, NULL on
 * failure
 */
void *tgt_get_pdev_target_handle(struct wlan_objmgr_pdev *pdev);

/**
 * tgt_get_psoc_target_handle() - Get psoc target_if handle
 * @psoc: Pointer to psoc
 *
 * Get handle to psoc target_if internal Spectral data
 *
 * Return: Handle to psoc target_if internal Spectral data on success, NULL on
 * failure
 */
void *tgt_get_psoc_target_handle(struct wlan_objmgr_psoc *psoc);

/**
 * tgt_spectral_control()- handler for demultiplexing requests from higher layer
 * @pdev: Reference to global pdev object
 * @sscan_req: pointer to Spectral scan request
 *
 * This function processes the spectral config command
 * and appropriate handlers are invoked.
 *
 * Return: QDF_STATUS_SUCCESS/QDF_STATUS_E_FAILURE
 */
QDF_STATUS tgt_spectral_control(struct wlan_objmgr_pdev *pdev,
				struct spectral_cp_request *sscan_req);

/**
 * tgt_pdev_spectral_init() - implementation for spectral init
 * @pdev: Pointer to pdev
 *
 * Return: On success, pointer to Spectral target_if internal private data, on
 * failure, NULL
 */
void *tgt_pdev_spectral_init(struct wlan_objmgr_pdev *pdev);

/**
 * tgt_pdev_spectral_deinit() - implementation for spectral de-init
 * @pdev: Pointer to pdev
 *
 * Return: None
 */
void tgt_pdev_spectral_deinit(struct wlan_objmgr_pdev *pdev);

/**
 * tgt_psoc_spectral_init() - implementation for spectral init
 * @psoc: Pointer to psoc
 *
 * Return: On success, pointer to Spectral psoc target_if internal private data,
 * on failure, NULL
 */
void *tgt_psoc_spectral_init(struct wlan_objmgr_psoc *psoc);

/**
 * tgt_psoc_spectral_deinit() - implementation for spectral de-init
 * @psoc: Pointer to psoc
 *
 * Return: None
 */
void tgt_psoc_spectral_deinit(struct wlan_objmgr_psoc *psoc);

/**
 * tgt_set_spectral_config() - Set spectral config
 * @pdev: Pointer to pdev object
 * @param: Pointer object describing Spectral parameter
 * @smode: Spectral scan mode
 * @err: Spectral control path error code
 *
 * Implementation for setting spectral config
 *
 * Return: QDF_STATUS_SUCCESS on success, else QDF_STATUS_E_FAILURE
 */
QDF_STATUS tgt_set_spectral_config(struct wlan_objmgr_pdev *pdev,
				   const struct spectral_cp_param *param,
				   const enum spectral_scan_mode smode,
				   enum spectral_cp_error_code *err);

/**
 * tgt_get_spectral_config() - Get spectral configuration
 * @pdev: Pointer to pdev object
 * @param: Pointer to spectral_config structure in which the configuration
 * should be returned
 * @smode: Spectral scan mode
 *
 * Implementation for getting the current spectral configuration
 *
 * Return: QDF_STATUS_SUCCESS on success, else QDF_STATUS_E_FAILURE
 */
QDF_STATUS tgt_get_spectral_config(struct wlan_objmgr_pdev *pdev,
				   struct spectral_config *sptrl_config,
				   const enum spectral_scan_mode smode);

/**
 * tgt_start_spectral_scan() - Start spectral scan
 * @pdev: Pointer to pdev object
 * @vdev_id: VDEV id
 * @smode: Spectral scan mode
 * @err: Spectral control path error code
 *
 * Implementation for starting spectral scan
 *
 * Return: QDF_STATUS_SUCCESS on success, else QDF_STATUS_E_FAILURE
 */
QDF_STATUS tgt_start_spectral_scan(struct wlan_objmgr_pdev *pdev,
				   uint8_t vdev_id,
				   enum spectral_scan_mode smode,
				   enum spectral_cp_error_code *err);

/**
 * tgt_stop_spectral_scan() - Stop spectral scan
 * @pdev: Pointer to pdev object
 * @smode: Spectral scan mode
 * @err: Spectral control path error code
 *
 * Implementation for stop spectral scan
 *
 * Return: QDF_STATUS_SUCCESS on success, else QDF_STATUS_E_FAILURE
 */
QDF_STATUS tgt_stop_spectral_scan(struct wlan_objmgr_pdev *pdev,
				  enum spectral_scan_mode smode,
				  enum spectral_cp_error_code *err);

/**
 * tgt_is_spectral_active() - Get whether Spectral is active
 * @pdev: Pointer to pdev object
 * @smode: Spectral scan mode
 *
 * Implementation to get whether Spectral is active
 *
 * Return: True if Spectral is active, false if Spectral is not active
 */
bool tgt_is_spectral_active(struct wlan_objmgr_pdev *pdev,
			    enum spectral_scan_mode smode);

/**
 * tgt_is_spectral_enabled() - Get whether Spectral is active
 * @pdev: Pointer to pdev object
 * @smode: Spectral scan mode
 *
 * Implementation to get whether Spectral is active
 *
 * Return: True if Spectral is active, false if Spectral is not active
 */
bool tgt_is_spectral_enabled(struct wlan_objmgr_pdev *pdev,
			     enum spectral_scan_mode smode);

/**
 * tgt_set_debug_level() - Set debug level for Spectral
 * @pdev: Pointer to pdev object
 * @debug_level: Debug level
 *
 * Implementation to set the debug level for Spectral
 *
 * Return: QDF_STATUS_SUCCESS on success, else QDF_STATUS_E_FAILURE
 */
QDF_STATUS tgt_set_debug_level(struct wlan_objmgr_pdev *pdev,
			       u_int32_t debug_level);

/**
 * tgt_get_debug_level() - Get debug level for Spectral
 * @pdev: Pointer to pdev object
 *
 * Implementation to get the debug level for Spectral
 *
 * Return: Current debug level
 */
uint32_t tgt_get_debug_level(struct wlan_objmgr_pdev *pdev);

/**
 * tgt_get_spectral_capinfo() - Get Spectral capability information
 * @pdev: Pointer to pdev object
 * @scaps: Buffer into which data should be copied
 *
 * Implementation to get the spectral capability information
 *
 * Return: QDF_STATUS_SUCCESS on success, else QDF_STATUS_E_FAILURE
 */
QDF_STATUS tgt_get_spectral_capinfo(struct wlan_objmgr_pdev *pdev,
				    struct spectral_caps *scaps);

/**
 * tgt_get_spectral_diagstats() - Get Spectral diagnostic statistics
 * @pdev:  Pointer to pdev object
 * @stats: Buffer into which data should be copied
 *
 * Implementation to get the spectral diagnostic statistics
 *
 * Return: QDF_STATUS_SUCCESS on success, else QDF_STATUS_E_FAILURE
 */
QDF_STATUS tgt_get_spectral_diagstats(struct wlan_objmgr_pdev *pdev,
				      struct spectral_diag_stats *stats);

/**
 * tgt_register_spectral_wmi_ops() - Register Spectral WMI operations
 * @psoc: Pointer to psoc bject
 * @wmi_ops: Pointer to the structure having Spectral WMI operations
 *
 * Implementation to register Spectral WMI operations in spectral
 * internal data structure
 *
 * Return: QDF_STATUS
 */
QDF_STATUS tgt_register_spectral_wmi_ops(struct wlan_objmgr_psoc *psoc,
					 struct spectral_wmi_ops *wmi_ops);

/**
 * tgt_register_spectral_tgt_ops() - Register Spectral target operations
 * @psoc: Pointer to psoc bject
 * @tgt_ops: Pointer to the structure having Spectral target operations
 *
 * Implementation to register Spectral target operations in spectral
 * internal data structure
 *
 * Return: QDF_STATUS
 */
QDF_STATUS tgt_register_spectral_tgt_ops(struct wlan_objmgr_psoc *psoc,
					 struct spectral_tgt_ops *tgt_ops);

/**
 * tgt_spectral_register_nl_cb() - Register Netlink callbacks
 * @pdev: Pointer to pdev object
 * @nl_cb: Netlink callbacks to register
 *
 * Return: void
 */
void tgt_spectral_register_nl_cb(struct wlan_objmgr_pdev *pdev,
				 struct spectral_nl_cb *nl_cb);

/**
 * tgt_spectral_use_nl_bcast() - Get whether to use broadcast/unicast while
 * sending Netlink messages to the application layer
 * @pdev: Pointer to pdev object
 *
 * Return: true for broadcast, false for unicast
 */
bool tgt_spectral_use_nl_bcast(struct wlan_objmgr_pdev *pdev);

/**
 * tgt_spectral_deregister_nl_cb() - De-register Netlink callbacks
 * @pdev: Pointer to pdev object
 *
 * Return: void
 */
void tgt_spectral_deregister_nl_cb(struct wlan_objmgr_pdev *pdev);

/**
 * tgt_spectral_process_report() - Process spectral report
 * @pdev: Pointer to pdev object
 * @payload: Pointer to spectral report buffer
 *
 * Return: status
 */
int
tgt_spectral_process_report(struct wlan_objmgr_pdev *pdev,
			    void *payload);

/**
 * tgt_spectral_register_to_dbr() - Register to direct DMA
 * @pdev: Pointer to pdev object
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
tgt_spectral_register_to_dbr(struct wlan_objmgr_pdev *pdev);

/**
 * tgt_spectral_unregister_to_dbr() - Register to direct DMA
 * @pdev: Pointer to pdev object
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
tgt_spectral_unregister_to_dbr(struct wlan_objmgr_pdev *pdev);

/**
 * tgt_spectral_get_target_type() - Get target type
 * @psoc: Pointer to psoc object
 *
 * Return: target type
 */
uint32_t
tgt_spectral_get_target_type(struct wlan_objmgr_psoc *psoc);

/**
 * tgt_set_spectral_dma_debug() - Set DMA debug for Spectral
 * @pdev: Pointer to pdev object
 * @dma_debug_type: Type of Spectral DMA debug i.e., ring or buffer debug
 * @dma_debug_enable: Value to be set for @dma_debug_type
 *
 * Return: QDF_STATUS of operation
 */
QDF_STATUS tgt_set_spectral_dma_debug(struct wlan_objmgr_pdev *pdev,
				      enum spectral_dma_debug dma_debug_type,
				      bool dma_debug_enable);

/**
 * tgt_spectral_register_events() - Register Spectral WMI event handlers
 * @psoc: Pointer to psoc object
 *
 * Return: QDF_STATUS of operation
 */
QDF_STATUS
tgt_spectral_register_events(struct wlan_objmgr_psoc *psoc);

/**
 * tgt_spectral_unregister_events() - Unregister Spectral WMI event handlers
 * @psoc: Pointer to psoc object
 *
 * Return: QDF_STATUS of operation
 */
QDF_STATUS
tgt_spectral_unregister_events(struct wlan_objmgr_psoc *psoc);
#endif /* _WLAN_SPECTRAL_TGT_API_H_ */
