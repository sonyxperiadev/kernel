/*
 * Copyright (c) 2012-2018 The Linux Foundation. All rights reserved.
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
 * DOC: This file contains UCFG APIs of EXTSCAN component
 */

#ifndef _WLAN_EXTSCAN_UCFG_API_H_
#define _WLAN_EXTSCAN_UCFG_API_H_

#include "wlan_extscan_api.h"
#include <wlan_objmgr_psoc_obj.h>
#include <wlan_objmgr_pdev_obj.h>
#include <wlan_objmgr_vdev_obj.h>

#ifdef FEATURE_WLAN_EXTSCAN

/**
 * ucfg_extscan_get_enable() - API to get extscan enable value
 * @psoc: pointer to psoc object
 *
 * Return: true if enabled else false.
 */
static inline
bool ucfg_extscan_get_enable(struct wlan_objmgr_psoc *psoc)
{
	return extscan_get_enable(psoc);
}

/**
 * ucfg_extscan_get_passive_max_time() - API to get passive
 *					 max channel time
 * @psoc: pointer to psoc object
 * @passive_max_chn_time: extscan passive max channel time
 *
 * Return: none
 */
static inline
void ucfg_extscan_get_passive_max_time(struct wlan_objmgr_psoc *psoc,
				       uint32_t *passive_max_chn_time)
{
	extscan_get_passive_max_time(psoc,
				     passive_max_chn_time);
}

/**
 * ucfg_extscan_get_active_max_time() - API to get active
 *					max channel time
 * @psoc: pointer to psoc object
 * @active_max_chn_time: extscan active max channel time
 *
 * Return: none
 */
static inline
void ucfg_extscan_get_active_max_time(struct wlan_objmgr_psoc *psoc,
				      uint32_t *active_max_chn_time)
{
	extscan_get_active_max_time(psoc,
				    active_max_chn_time);
}

/**
 * ucfg_extscan_get_active_min_time() - API to set active
 *					min channel time
 * @psoc: pointer to psoc object
 * @active_min_chn_time: extscan active min channel time
 *
 * Return: none
 */
static inline
void ucfg_extscan_get_active_min_time(struct wlan_objmgr_psoc *psoc,
				      uint32_t *active_min_chn_time)
{
	extscan_get_active_min_time(psoc,
				    active_min_chn_time);
}

#else

static inline
bool ucfg_extscan_get_enable(struct wlan_objmgr_psoc *psoc)
{
	return false;
}

#endif
#endif
