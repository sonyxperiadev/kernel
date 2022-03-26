/*
 * Copyright (c) 2020 The Linux Foundation. All rights reserved.
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
 * DOC : target_if_cfr_6490.c
 *
 * Target interface of CFR for QCA6490 implementation
 *
 */

#include <cdp_txrx_ctrl.h>
#include "target_if_cfr.h"
#include <qdf_nbuf.h>
#include "wlan_cfr_utils_api.h"
#include "target_if_cfr_6490.h"
#include "target_if_cfr_enh.h"
#include "init_deinit_lmac.h"
#include "cfg_ucfg_api.h"
#include "cfr_cfg.h"

#ifdef WLAN_ENH_CFR_ENABLE
#ifdef CFR_USE_FIXED_FOLDER
static wdi_event_subscribe g_cfr_subscribe;

static void target_cfr_callback(void *pdev_obj, enum WDI_EVENT event,
				void *data, u_int16_t peer_id,
				uint32_t status)
{
	struct wlan_objmgr_pdev *pdev;
	qdf_nbuf_t nbuf = (qdf_nbuf_t)data;
	qdf_nbuf_t data_clone;

	pdev = (struct wlan_objmgr_pdev *)pdev_obj;
	if (qdf_unlikely((!pdev || !data))) {
		cfr_err("Invalid pdev %pK or data %pK for event %d",
			pdev, data, event);
		qdf_nbuf_free(nbuf);
		return;
	}

	if (event != WDI_EVENT_RX_PPDU_DESC) {
		cfr_debug("event is %d", event);
		qdf_nbuf_free(nbuf);
		return;
	}

	data_clone = qdf_nbuf_clone(nbuf);
	if (data_clone)
		wlan_cfr_rx_tlv_process(pdev, (void *)data_clone);

	qdf_nbuf_free(nbuf);
}

QDF_STATUS
target_if_cfr_subscribe_ppdu_desc(struct wlan_objmgr_pdev *pdev,
				  bool is_subscribe)
{
	ol_txrx_soc_handle soc;
	struct wlan_objmgr_psoc *psoc;
	struct pdev_cfr *pcfr;

	if (!pdev) {
		cfr_err("Null pdev");
		return QDF_STATUS_E_INVAL;
	}

	pcfr = wlan_objmgr_pdev_get_comp_private_obj(
				pdev, WLAN_UMAC_COMP_CFR);
	if (!pcfr) {
		cfr_err("pcfr is NULL");
		return QDF_STATUS_E_INVAL;
	}

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		cfr_err("Null psoc");
		return QDF_STATUS_E_INVAL;
	}

	soc = wlan_psoc_get_dp_handle(psoc);
	if (!soc) {
		cfr_err("Null soc");
		return QDF_STATUS_E_INVAL;
	}

	g_cfr_subscribe.callback = target_cfr_callback;
	g_cfr_subscribe.context = pdev;
	cdp_set_cfr_rcc(soc, 0, is_subscribe);
	cdp_enable_mon_reap_timer(soc, 0, is_subscribe);
	if (is_subscribe) {
		if (cdp_wdi_event_sub(soc, 0, &g_cfr_subscribe,
				      WDI_EVENT_RX_PPDU_DESC)) {
			cfr_err("wdi event sub fail");
			return QDF_STATUS_E_FAILURE;
		}
	} else {
		if (cdp_wdi_event_unsub(soc, 0, &g_cfr_subscribe,
					WDI_EVENT_RX_PPDU_DESC)) {
			cfr_err("wdi event unsub fail");
			return QDF_STATUS_E_FAILURE;
		}
	}

	return QDF_STATUS_SUCCESS;
}
#endif
#endif


