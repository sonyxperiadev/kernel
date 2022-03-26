/*
 * Copyright (c) 2016-2017,2020 The Linux Foundation. All rights reserved.
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


#ifndef _WLAN_OSIF_PRIV_H_
#define _WLAN_OSIF_PRIV_H_

#include "qdf_net_if.h"
#include <qca_vendor.h>

struct osif_scan_pdev;
struct osif_tdls_vdev;

/**
 * struct pdev_osif_priv - OS private structure
 * @wiphy:            wiphy handle
 * @legacy_osif_priv: legacy osif private handle
 * @scan_priv:        Scan related data used by cfg80211 scan
 * @nif:              pdev net device
 */
struct pdev_osif_priv {
	struct wiphy *wiphy;
	void *legacy_osif_priv;
	struct osif_scan_pdev *osif_scan;
	struct qdf_net_if *nif;
};

/**
 * struct vdev_osif_priv - OS private structure of vdev
 * @wdev:             wireless device handle
 * @legacy_osif_priv: legacy osif private handle
 * @osif_tdls: osif tdls info
 */
struct vdev_osif_priv {
	struct wireless_dev *wdev;
	void *legacy_osif_priv;
	struct osif_tdls_vdev *osif_tdls;
};

#endif
