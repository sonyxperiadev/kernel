/*
 * Copyright (c) 2017-2020 The Linux Foundation. All rights reserved.
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
 * DOC: API for interacting with target interface.
 *
 */

#include "target_if.h"
#include "target_type.h"
#ifdef WLAN_ATF_ENABLE
#include "target_if_atf.h"
#endif
#ifdef WLAN_SA_API_ENABLE
#include "target_if_sa_api.h"
#endif
#ifdef WLAN_CFR_ENABLE
#include "target_if_cfr.h"
#endif
#ifdef WLAN_CONV_SPECTRAL_ENABLE
#include "target_if_spectral.h"
#endif

#ifdef WLAN_IOT_SIM_SUPPORT
#include <target_if_iot_sim.h>
#endif
#include <target_if_reg.h>
#include <target_if_scan.h>
#include <target_if_ftm.h>
#ifdef DFS_COMPONENT_ENABLE
#include <target_if_dfs.h>
#endif

#ifdef CONVERGED_P2P_ENABLE
#include "target_if_p2p.h"
#endif

#ifdef WIFI_POS_CONVERGED
#include "target_if_wifi_pos.h"
#endif

#ifdef FEATURE_WLAN_TDLS
#include "target_if_tdls.h"
#endif
#ifdef QCA_SUPPORT_SON
#include <target_if_son.h>
#endif
#ifdef WLAN_OFFCHAN_TXRX_ENABLE
#include <target_if_offchan_txrx_api.h>
#endif
#ifdef WLAN_SUPPORT_GREEN_AP
#include <target_if_green_ap.h>
#endif
#include <init_deinit_lmac.h>
#include <service_ready_util.h>

#ifdef DIRECT_BUF_RX_ENABLE
#include <target_if_direct_buf_rx_api.h>
#endif

#ifdef WLAN_SUPPORT_FILS
#include <target_if_fd.h>
#endif
#include "qdf_module.h"

#include <target_if_cp_stats.h>
#ifdef CRYPTO_SET_KEY_CONVERGED
#include <target_if_crypto.h>
#endif
#include <target_if_vdev_mgr_tx_ops.h>

#ifdef FEATURE_COEX
#include <target_if_coex.h>
#endif
#include <wlan_utility.h>

#ifdef DCS_INTERFERENCE_DETECTION
#include <target_if_dcs.h>
#endif

static struct target_if_ctx *g_target_if_ctx;

struct target_if_ctx *target_if_get_ctx()
{
	return g_target_if_ctx;
}

struct wlan_objmgr_psoc *target_if_get_psoc_from_scn_hdl(void *scn_handle)
{
	struct wlan_objmgr_psoc *psoc;

	qdf_spin_lock_bh(&g_target_if_ctx->lock);
	if (scn_handle && g_target_if_ctx->get_psoc_hdl_cb)
		psoc = g_target_if_ctx->get_psoc_hdl_cb(scn_handle);
	else
		psoc = NULL;
	qdf_spin_unlock_bh(&g_target_if_ctx->lock);

	return psoc;
}

struct wlan_objmgr_pdev *target_if_get_pdev_from_scn_hdl(void *scn_handle)
{
	struct wlan_objmgr_pdev *pdev;

	qdf_spin_lock_bh(&g_target_if_ctx->lock);
	if (scn_handle && g_target_if_ctx->get_pdev_hdl_cb)
		pdev = g_target_if_ctx->get_pdev_hdl_cb(scn_handle);
	else
		pdev = NULL;
	qdf_spin_unlock_bh(&g_target_if_ctx->lock);

	return pdev;
}

#ifdef DIRECT_BUF_RX_ENABLE
static QDF_STATUS target_if_direct_buf_rx_init(void)
{
	return direct_buf_rx_init();
}

static QDF_STATUS target_if_direct_buf_rx_deinit(void)
{
	return direct_buf_rx_deinit();
}
#else
static QDF_STATUS target_if_direct_buf_rx_init(void)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS target_if_direct_buf_rx_deinit(void)
{
	return QDF_STATUS_SUCCESS;
}
#endif /* DIRECT_BUF_RX_ENABLE */

QDF_STATUS target_if_init(get_psoc_handle_callback psoc_hdl_cb)
{
	g_target_if_ctx = qdf_mem_malloc(sizeof(*g_target_if_ctx));
	if (!g_target_if_ctx) {
		QDF_ASSERT(0);
		return QDF_STATUS_E_NOMEM;
	}

	qdf_spinlock_create(&g_target_if_ctx->lock);

	qdf_spin_lock_bh(&g_target_if_ctx->lock);
	g_target_if_ctx->magic = TGT_MAGIC;
	g_target_if_ctx->get_psoc_hdl_cb = psoc_hdl_cb;
	qdf_spin_unlock_bh(&g_target_if_ctx->lock);

	target_if_direct_buf_rx_init();

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS target_if_deinit(void)
{
	if (!g_target_if_ctx) {
		QDF_ASSERT(0);
		target_if_err("target if ctx is null");
		return QDF_STATUS_E_INVAL;
	}

	qdf_spin_lock_bh(&g_target_if_ctx->lock);
	g_target_if_ctx->magic = 0;
	g_target_if_ctx->get_psoc_hdl_cb = NULL;
	g_target_if_ctx->get_pdev_hdl_cb = NULL;
	g_target_if_ctx->service_ready_cb = NULL;
	qdf_spin_unlock_bh(&g_target_if_ctx->lock);

	qdf_spinlock_destroy(&g_target_if_ctx->lock);
	qdf_mem_free(g_target_if_ctx);
	g_target_if_ctx = NULL;

	target_if_direct_buf_rx_deinit();

	return QDF_STATUS_SUCCESS;
}

qdf_export_symbol(target_if_deinit);

QDF_STATUS target_if_store_pdev_target_if_ctx(
		get_pdev_handle_callback pdev_hdl_cb)
{
	if (!g_target_if_ctx) {
		QDF_ASSERT(0);
		target_if_err("target if ctx is null");
		return QDF_STATUS_E_INVAL;
	}

	qdf_spin_lock_bh(&g_target_if_ctx->lock);
	g_target_if_ctx->get_pdev_hdl_cb = pdev_hdl_cb;
	qdf_spin_unlock_bh(&g_target_if_ctx->lock);

	return QDF_STATUS_SUCCESS;
}

#ifndef WLAN_OFFCHAN_TXRX_ENABLE
static void target_if_offchan_txrx_ops_register(
					struct wlan_lmac_if_tx_ops *tx_ops)
{
}
#endif /* WLAN_OFFCHAN_TXRX_ENABLE */

#ifndef WLAN_ATF_ENABLE
static void target_if_atf_tx_ops_register(struct wlan_lmac_if_tx_ops *tx_ops)
{
}
#endif /* WLAN_ATF_ENABLE */

#ifndef WLAN_SA_API_ENABLE
static void target_if_sa_api_tx_ops_register(struct wlan_lmac_if_tx_ops *tx_ops)
{
}
#endif /* WLAN_SA_API_ENABLE */

#ifndef WLAN_CFR_ENABLE
static void target_if_cfr_tx_ops_register(struct wlan_lmac_if_tx_ops *tx_ops)
{
}
#endif

#ifdef WLAN_SUPPORT_FILS
static void target_if_fd_tx_ops_register(struct wlan_lmac_if_tx_ops *tx_ops)
{
	target_if_fd_register_tx_ops(tx_ops);
}
#else
static void target_if_fd_tx_ops_register(struct wlan_lmac_if_tx_ops *tx_ops)
{
}
#endif

#ifdef WIFI_POS_CONVERGED
static void target_if_wifi_pos_tx_ops_register(
			struct wlan_lmac_if_tx_ops *tx_ops)
{
	target_if_wifi_pos_register_tx_ops(tx_ops);
}
#else
static void target_if_wifi_pos_tx_ops_register(
			struct wlan_lmac_if_tx_ops *tx_ops)
{
}
#endif
#ifdef QCA_SUPPORT_SON
static void target_if_son_tx_ops_register(
			struct wlan_lmac_if_tx_ops *tx_ops)
{
	target_if_son_register_tx_ops(tx_ops);
	return;
}
#else
static void target_if_son_tx_ops_register(
			struct wlan_lmac_if_tx_ops *tx_ops)
{
	return;
}
#endif

#ifdef FEATURE_WLAN_TDLS
static void target_if_tdls_tx_ops_register(struct wlan_lmac_if_tx_ops *tx_ops)
{
	target_if_tdls_register_tx_ops(tx_ops);
}
#else
static void target_if_tdls_tx_ops_register(struct wlan_lmac_if_tx_ops *tx_ops)
{
}
#endif /* FEATURE_WLAN_TDLS */

#ifdef DFS_COMPONENT_ENABLE
static void target_if_dfs_tx_ops_register(
				struct wlan_lmac_if_tx_ops *tx_ops)
{
	target_if_register_dfs_tx_ops(tx_ops);
}
#else
static void target_if_dfs_tx_ops_register(
				struct wlan_lmac_if_tx_ops *tx_ops)
{
}
#endif /* DFS_COMPONENT_ENABLE */

#ifdef WLAN_CONV_SPECTRAL_ENABLE
static void target_if_sptrl_tx_ops_register(
				struct wlan_lmac_if_tx_ops *tx_ops)
{
	target_if_sptrl_register_tx_ops(tx_ops);
}
#else
static void target_if_sptrl_tx_ops_register(
				struct wlan_lmac_if_tx_ops *tx_ops)
{
}
#endif /* WLAN_CONV_SPECTRAL_ENABLE */

#ifdef WLAN_IOT_SIM_SUPPORT
static void target_if_iot_sim_tx_ops_register(
				struct wlan_lmac_if_tx_ops *tx_ops)
{
	target_if_iot_sim_register_tx_ops(tx_ops);
}
#else
static void target_if_iot_sim_tx_ops_register(
				struct wlan_lmac_if_tx_ops *tx_ops)
{
}
#endif

#ifdef DIRECT_BUF_RX_ENABLE
static void target_if_direct_buf_rx_tx_ops_register(
				struct wlan_lmac_if_tx_ops *tx_ops)
{
	target_if_direct_buf_rx_register_tx_ops(tx_ops);
}
#else
static void target_if_direct_buf_rx_tx_ops_register(
				struct wlan_lmac_if_tx_ops *tx_ops)
{
}
#endif /* DIRECT_BUF_RX_ENABLE */

#ifdef WLAN_SUPPORT_GREEN_AP
static QDF_STATUS target_if_green_ap_tx_ops_register(
				struct wlan_lmac_if_tx_ops *tx_ops)
{
	return target_if_register_green_ap_tx_ops(tx_ops);
}
#else
static QDF_STATUS target_if_green_ap_tx_ops_register(
				struct wlan_lmac_if_tx_ops *tx_ops)
{
	return QDF_STATUS_SUCCESS;
}
#endif /* WLAN_SUPPORT_GREEN_AP */
#if defined(WLAN_CONV_CRYPTO_SUPPORTED) && defined(CRYPTO_SET_KEY_CONVERGED)
static void target_if_crypto_tx_ops_register(
				struct wlan_lmac_if_tx_ops *tx_ops)
{
	target_if_crypto_register_tx_ops(tx_ops);
}
#else
static inline void target_if_crypto_tx_ops_register(
				struct wlan_lmac_if_tx_ops *tx_ops)
{
}
#endif

#ifdef FEATURE_COEX
static QDF_STATUS
target_if_coex_tx_ops_register(struct wlan_lmac_if_tx_ops *tx_ops)
{
	return target_if_coex_register_tx_ops(tx_ops);
}
#else
static inline QDF_STATUS
target_if_coex_tx_ops_register(struct wlan_lmac_if_tx_ops *tx_ops)
{
	return QDF_STATUS_SUCCESS;
}
#endif

static void target_if_target_tx_ops_register(
		struct wlan_lmac_if_tx_ops *tx_ops)
{
	struct wlan_lmac_if_target_tx_ops *target_tx_ops;

	if (!tx_ops) {
		target_if_err("invalid tx_ops");
		return;
	}

	target_tx_ops = &tx_ops->target_tx_ops;

	target_tx_ops->tgt_is_tgt_type_ar900b =
		target_is_tgt_type_ar900b;

	target_tx_ops->tgt_is_tgt_type_ipq4019 =
		target_is_tgt_type_ipq4019;

	target_tx_ops->tgt_is_tgt_type_qca9984 =
		target_is_tgt_type_qca9984;

	target_tx_ops->tgt_is_tgt_type_qca9888 =
		target_is_tgt_type_qca9888;

	target_tx_ops->tgt_is_tgt_type_adrastea =
		target_is_tgt_type_adrastea;

	target_tx_ops->tgt_is_tgt_type_qcn9000 =
		target_is_tgt_type_qcn9000;

	target_tx_ops->tgt_is_tgt_type_qcn9100 =
		target_is_tgt_type_qcn9100;

	target_tx_ops->tgt_get_tgt_type =
		lmac_get_tgt_type;

	target_tx_ops->tgt_get_tgt_version =
		lmac_get_tgt_version;

	target_tx_ops->tgt_get_tgt_revision =
		lmac_get_tgt_revision;
}

static QDF_STATUS
target_if_cp_stats_tx_ops_register(struct wlan_lmac_if_tx_ops *tx_ops)
{
	return target_if_cp_stats_register_tx_ops(tx_ops);
}

#ifdef DCS_INTERFERENCE_DETECTION
static QDF_STATUS
target_if_dcs_tx_ops_register(struct wlan_lmac_if_tx_ops *tx_ops)
{
	return target_if_dcs_register_tx_ops(tx_ops);
}
#else
static QDF_STATUS
target_if_dcs_tx_ops_register(struct wlan_lmac_if_tx_ops *tx_ops)
{
	return QDF_STATUS_SUCCESS;
}
#endif

static QDF_STATUS
target_if_vdev_mgr_tx_ops_register(struct wlan_lmac_if_tx_ops *tx_ops)
{
	return target_if_vdev_mgr_register_tx_ops(tx_ops);
}

#ifdef QCA_WIFI_FTM
static
void target_if_ftm_tx_ops_register(struct wlan_lmac_if_tx_ops *tx_ops)
{
	target_if_ftm_register_tx_ops(tx_ops);
}
#else
static
void target_if_ftm_tx_ops_register(struct wlan_lmac_if_tx_ops *tx_ops)
{
}
#endif

static
QDF_STATUS target_if_register_umac_tx_ops(struct wlan_lmac_if_tx_ops *tx_ops)
{
	/* call regulatory callback to register tx ops */
	target_if_register_regulatory_tx_ops(tx_ops);

	/* call umac callback to register legacy tx ops */
	wlan_lmac_if_umac_tx_ops_register(tx_ops);

	/* Register scan tx ops */
	target_if_scan_tx_ops_register(tx_ops);

	target_if_atf_tx_ops_register(tx_ops);

	target_if_sa_api_tx_ops_register(tx_ops);

	target_if_cfr_tx_ops_register(tx_ops);

	target_if_wifi_pos_tx_ops_register(tx_ops);

	target_if_dfs_tx_ops_register(tx_ops);

	target_if_son_tx_ops_register(tx_ops);

	target_if_tdls_tx_ops_register(tx_ops);

	target_if_fd_tx_ops_register(tx_ops);

	target_if_target_tx_ops_register(tx_ops);

	target_if_offchan_txrx_ops_register(tx_ops);

	target_if_green_ap_tx_ops_register(tx_ops);

	target_if_ftm_tx_ops_register(tx_ops);

	target_if_cp_stats_tx_ops_register(tx_ops);

	target_if_dcs_tx_ops_register(tx_ops);

	target_if_crypto_tx_ops_register(tx_ops);

	target_if_vdev_mgr_tx_ops_register(tx_ops);

	target_if_coex_tx_ops_register(tx_ops);

	/* Converged UMAC components to register their TX-ops here */
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS target_if_register_tx_ops(struct wlan_lmac_if_tx_ops *tx_ops)
{
	/* Converged UMAC components to register their TX-ops */
	target_if_register_umac_tx_ops(tx_ops);

	/* Components parallel to UMAC to register their TX-ops here */
	target_if_sptrl_tx_ops_register(tx_ops);

	target_if_iot_sim_tx_ops_register(tx_ops);

	/* Register direct buffer rx component tx ops here */
	target_if_direct_buf_rx_tx_ops_register(tx_ops);

#ifdef CONVERGED_P2P_ENABLE
	/* Converged UMAC components to register P2P TX-ops */
	target_if_p2p_register_tx_ops(tx_ops);
#endif

	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(target_if_register_tx_ops);

wmi_legacy_service_ready_callback
target_if_get_psoc_legacy_service_ready_cb(void)
{
	wmi_legacy_service_ready_callback service_ready_cb;

	qdf_spin_lock_bh(&g_target_if_ctx->lock);
	if (g_target_if_ctx->service_ready_cb)
		service_ready_cb = g_target_if_ctx->service_ready_cb;
	else
		service_ready_cb = NULL;
	qdf_spin_unlock_bh(&g_target_if_ctx->lock);

	return service_ready_cb;
}
qdf_export_symbol(target_if_get_psoc_legacy_service_ready_cb);

QDF_STATUS target_if_register_legacy_service_ready_cb(
	wmi_legacy_service_ready_callback service_ready_cb)
{
	qdf_spin_lock_bh(&g_target_if_ctx->lock);
	g_target_if_ctx->service_ready_cb = service_ready_cb;
	qdf_spin_unlock_bh(&g_target_if_ctx->lock);

	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(target_if_register_legacy_service_ready_cb);

QDF_STATUS target_if_alloc_pdev_tgt_info(struct wlan_objmgr_pdev *pdev)
{
	struct target_pdev_info *tgt_pdev_info;

	if (!pdev) {
		target_if_err("pdev is null");
		return QDF_STATUS_E_INVAL;
	}

	tgt_pdev_info = qdf_mem_malloc(sizeof(*tgt_pdev_info));

	if (!tgt_pdev_info)
		return QDF_STATUS_E_NOMEM;

	wlan_pdev_set_tgt_if_handle(pdev, tgt_pdev_info);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS target_if_free_pdev_tgt_info(struct wlan_objmgr_pdev *pdev)
{
	struct target_pdev_info *tgt_pdev_info;

	if (!pdev) {
		target_if_err("pdev is null");
		return QDF_STATUS_E_INVAL;
	}

	tgt_pdev_info = wlan_pdev_get_tgt_if_handle(pdev);

	wlan_pdev_set_tgt_if_handle(pdev, NULL);

	qdf_mem_free(tgt_pdev_info);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS target_if_alloc_psoc_tgt_info(struct wlan_objmgr_psoc *psoc)
{
	struct target_psoc_info *tgt_psoc_info;

	if (!psoc) {
		target_if_err("psoc is null");
		return QDF_STATUS_E_INVAL;
	}

	tgt_psoc_info = qdf_mem_malloc(sizeof(*tgt_psoc_info));

	if (!tgt_psoc_info)
		return QDF_STATUS_E_NOMEM;

	wlan_psoc_set_tgt_if_handle(psoc, tgt_psoc_info);
	target_psoc_set_preferred_hw_mode(tgt_psoc_info, WMI_HOST_HW_MODE_MAX);
	wlan_minidump_log(tgt_psoc_info,
			  sizeof(*tgt_psoc_info), psoc,
			  WLAN_MD_OBJMGR_PSOC_TGT_INFO, "target_psoc_info");

	qdf_event_create(&tgt_psoc_info->info.event);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS target_if_free_psoc_tgt_info(struct wlan_objmgr_psoc *psoc)
{
	struct target_psoc_info *tgt_psoc_info;
	struct wlan_psoc_host_service_ext_param *ext_param;

	if (!psoc) {
		target_if_err("psoc is null");
		return QDF_STATUS_E_INVAL;
	}

	tgt_psoc_info = wlan_psoc_get_tgt_if_handle(psoc);

	ext_param = target_psoc_get_service_ext_param(tgt_psoc_info);
	if (!ext_param) {
		target_if_err("tgt_psoc_info is NULL");
		return QDF_STATUS_E_INVAL;
	}
	init_deinit_chainmask_table_free(ext_param);
	init_deinit_dbr_ring_cap_free(tgt_psoc_info);
	init_deinit_spectral_scaling_params_free(tgt_psoc_info);
	init_deinit_scan_radio_cap_free(tgt_psoc_info);

	qdf_event_destroy(&tgt_psoc_info->info.event);

	wlan_psoc_set_tgt_if_handle(psoc, NULL);

	wlan_minidump_remove(tgt_psoc_info);
	qdf_mem_free(tgt_psoc_info);

	return QDF_STATUS_SUCCESS;
}

bool target_is_tgt_type_ar900b(uint32_t target_type)
{
	return target_type == TARGET_TYPE_AR900B;
}

bool target_is_tgt_type_ipq4019(uint32_t target_type)
{
	return target_type == TARGET_TYPE_IPQ4019;
}

bool target_is_tgt_type_qca9984(uint32_t target_type)
{
	return target_type == TARGET_TYPE_QCA9984;
}

bool target_is_tgt_type_qca9888(uint32_t target_type)
{
	return target_type == TARGET_TYPE_QCA9888;
}

bool target_is_tgt_type_adrastea(uint32_t target_type)
{
	return target_type == TARGET_TYPE_ADRASTEA;
}

bool target_is_tgt_type_qcn9000(uint32_t target_type)
{
	return target_type == TARGET_TYPE_QCN9000;
}

bool target_is_tgt_type_qcn9100(uint32_t target_type)
{
	return target_type == TARGET_TYPE_QCN9100;
}

QDF_STATUS
target_pdev_is_scan_radio_supported(struct wlan_objmgr_pdev *pdev,
				    bool *is_scan_radio_supported)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_psoc_host_scan_radio_caps *scan_radio_caps;
	uint8_t cap_idx;
	uint32_t num_scan_radio_caps;
	int32_t phy_id;
	struct target_psoc_info *tgt_psoc_info;
	struct target_pdev_info *tgt_pdev;

	if (!is_scan_radio_supported) {
		target_if_err("input argument is null");
		return QDF_STATUS_E_INVAL;
	}
	*is_scan_radio_supported = false;

	if (!pdev) {
		target_if_err("pdev is null");
		return QDF_STATUS_E_INVAL;
	}

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		target_if_err("psoc is null");
		return QDF_STATUS_E_INVAL;
	}

	tgt_psoc_info = wlan_psoc_get_tgt_if_handle(psoc);
	if (!tgt_psoc_info) {
		target_if_err("target_psoc_info is null");
		return QDF_STATUS_E_INVAL;
	}

	num_scan_radio_caps =
		target_psoc_get_num_scan_radio_caps(tgt_psoc_info);
	if (!num_scan_radio_caps)
		return QDF_STATUS_SUCCESS;

	scan_radio_caps = target_psoc_get_scan_radio_caps(tgt_psoc_info);
	if (!scan_radio_caps) {
		target_if_err("scan radio capabilities is null");
		return QDF_STATUS_E_INVAL;
	}

	tgt_pdev = (struct target_pdev_info *)wlan_pdev_get_tgt_if_handle(pdev);
	if (!tgt_pdev) {
		target_if_err("target_pdev_info is null");
		return QDF_STATUS_E_INVAL;
	}

	phy_id = target_pdev_get_phy_idx(tgt_pdev);
	if (phy_id < 0) {
		target_if_err("phy_id is invalid");
		return QDF_STATUS_E_INVAL;
	}

	for (cap_idx = 0; cap_idx < num_scan_radio_caps; cap_idx++)
		if (scan_radio_caps[cap_idx].phy_id == phy_id)
			*is_scan_radio_supported =
				scan_radio_caps[cap_idx].scan_radio_supported;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
target_pdev_scan_radio_is_dfs_enabled(struct wlan_objmgr_pdev *pdev,
				      bool *is_dfs_en)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_psoc_host_scan_radio_caps *scan_radio_caps;
	uint8_t cap_idx;
	uint32_t num_scan_radio_caps, pdev_id;
	int32_t phy_id;
	struct target_psoc_info *tgt_psoc_info;
	struct target_pdev_info *tgt_pdev;

	if (!is_dfs_en) {
		target_if_err("input argument is null");
		return QDF_STATUS_E_INVAL;
	}
	*is_dfs_en = true;

	if (!pdev) {
		target_if_err("pdev is null");
		return QDF_STATUS_E_INVAL;
	}

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		target_if_err("psoc is null");
		return QDF_STATUS_E_INVAL;
	}

	tgt_psoc_info = wlan_psoc_get_tgt_if_handle(psoc);
	if (!tgt_psoc_info) {
		target_if_err("target_psoc_info is null");
		return QDF_STATUS_E_INVAL;
	}

	num_scan_radio_caps =
		target_psoc_get_num_scan_radio_caps(tgt_psoc_info);
	if (!num_scan_radio_caps) {
		target_if_err("scan radio not supported for psoc");
		return QDF_STATUS_E_INVAL;
	}

	scan_radio_caps = target_psoc_get_scan_radio_caps(tgt_psoc_info);
	if (!scan_radio_caps) {
		target_if_err("scan radio capabilities is null");
		return QDF_STATUS_E_INVAL;
	}

	tgt_pdev = (struct target_pdev_info *)wlan_pdev_get_tgt_if_handle(pdev);
	if (!tgt_pdev) {
		target_if_err("target_pdev_info is null");
		return QDF_STATUS_E_INVAL;
	}

	pdev_id = wlan_objmgr_pdev_get_pdev_id(pdev);
	phy_id = target_pdev_get_phy_idx(tgt_pdev);
	if (phy_id < 0) {
		target_if_err("phy_id is invalid");
		return QDF_STATUS_E_INVAL;
	}

	for (cap_idx = 0; cap_idx < num_scan_radio_caps; cap_idx++)
		if (scan_radio_caps[cap_idx].phy_id == phy_id) {
			*is_dfs_en = scan_radio_caps[cap_idx].dfs_en;
			return QDF_STATUS_SUCCESS;
		}

	target_if_err("No scan radio cap found in pdev %d", pdev_id);

	return QDF_STATUS_E_INVAL;
}
