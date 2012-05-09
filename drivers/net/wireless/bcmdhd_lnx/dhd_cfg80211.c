/*
 * Linux cfg80211 driver - Dongle Host Driver (DHD) related
 *
 * Copyright (C) 1999-2011, Broadcom Corporation
 * 
 *         Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed to you
 * under the terms of the GNU General Public License version 2 (the "GPL"),
 * available at http://www.broadcom.com/licenses/GPLv2.php, with the
 * following added to such license:
 * 
 *      As a special exception, the copyright holders of this software give you
 * permission to link this software with independent modules, and to copy and
 * distribute the resulting executable under terms of your choice, provided that
 * you also meet, for each linked independent module, the terms and conditions of
 * the license of that module.  An independent module is a module which is not
 * derived from this software.  The special exception does not apply to any
 * modifications of the software.
 * 
 *      Notwithstanding the above, under no circumstances may you combine this
 * software in any way with any other Broadcom software provided under a license
 * other than the GPL, without Broadcom's express prior written consent.
 *
 * $Id: wl_cfg80211.c,v 1.1.4.1.2.14 2011/02/09 01:40:07 Exp $
 */

#include <net/rtnetlink.h>

#include <bcmutils.h>
#include <wldev_common.h>
#include <wl_cfg80211.h>
#include <dhd_cfg80211.h>
extern struct wl_priv *wlcfg_drv_priv;
static int dhd_dongle_up = FALSE;

#include <dngl_stats.h>
#include <dhd.h>
#include <dhdioctl.h>
#include <wlioctl.h>
#include <dhd_cfg80211.h>

static s32 wl_dongle_up(struct net_device *ndev, u32 up);
#if !defined(OEM_EMBEDDED_LINUX)
static s32 wl_dongle_power(struct net_device *ndev, u32 power_mode);
static s32 wl_dongle_glom(struct net_device *ndev, u32 glom, u32 dongle_align);
static s32 wl_dongle_roam(struct net_device *ndev, u32 roamvar,
			  u32 bcn_timeout);
static s32 wl_dongle_scantime(struct net_device *ndev, s32 scan_assoc_time,
			      s32 scan_unassoc_time);
static s32 wl_dongle_offload(struct net_device *ndev, s32 arpoe, s32 arp_ol);
static s32 wl_pattern_atoh(s8 * src, s8 * dst);
static s32 wl_dongle_filter(struct net_device *ndev, u32 filter_mode);
#endif

/**
 * Function implementations
 */

s32 dhd_cfg80211_init(struct wl_priv *wl)
{
	dhd_dongle_up = FALSE;
	return 0;
}

s32 dhd_cfg80211_deinit(struct wl_priv *wl)
{
	dhd_dongle_up = FALSE;
	return 0;
}

s32 dhd_cfg80211_get_opmode(struct wl_priv *wl)
{
	dhd_pub_t *dhd = (dhd_pub_t *)(wl->pub);
	return dhd->op_mode;
}

s32 dhd_cfg80211_down(struct wl_priv *wl)
{
	dhd_dongle_up = FALSE;
	return 0;
}

s32 dhd_cfg80211_set_p2p_info(struct wl_priv *wl, int val)
{
	dhd_pub_t *dhd = (dhd_pub_t *)(wl->pub);
	int bcn_timeout = DHD_BEACON_TIMEOUT_HIGH;
	char iovbuf[30];

	dhd->op_mode |= val;
	WL_ERR(("Set : op_mode=%d\n", dhd->op_mode));

#ifdef ARP_OFFLOAD_SUPPORT
	/* IF P2P is enabled, disable arpoe */
	dhd_arp_offload_set(dhd, 0);
	dhd_arp_offload_enable(dhd, false);
#endif /* ARP_OFFLOAD_SUPPORT */

	/* Setup timeout if Beacons are lost and roam is off to report link down */
	bcm_mkiovar("bcn_timeout", (char *)&bcn_timeout, 4, iovbuf,
		    sizeof(iovbuf));
	dhd_wl_ioctl_cmd(dhd, WLC_SET_VAR, iovbuf, sizeof(iovbuf), TRUE, 0);

	return 0;
}

s32 dhd_cfg80211_clean_p2p_info(struct wl_priv *wl)
{
	dhd_pub_t *dhd = (dhd_pub_t *)(wl->pub);
	int bcn_timeout = DHD_BEACON_TIMEOUT_NORMAL;
	char iovbuf[30];

	dhd->op_mode &= ~CONCURENT_MASK;
	WL_ERR(("Clean : op_mode=%d\n", dhd->op_mode));

#ifdef ARP_OFFLOAD_SUPPORT
	/* IF P2P is disabled, enable arpoe back for STA mode. */
	dhd_arp_offload_set(dhd, dhd_arp_mode);
	dhd_arp_offload_enable(dhd, true);
#endif /* ARP_OFFLOAD_SUPPORT */

	/* Setup timeout if Beacons are lost and roam is off to report link down */
	bcm_mkiovar("bcn_timeout", (char *)&bcn_timeout, 4, iovbuf,
		    sizeof(iovbuf));
	dhd_wl_ioctl_cmd(dhd, WLC_SET_VAR, iovbuf, sizeof(iovbuf), TRUE, 0);

	return 0;
}

static s32 wl_dongle_up(struct net_device *ndev, u32 up)
{
	s32 err = 0;

	err = wldev_ioctl(ndev, WLC_UP, &up, sizeof(up), true);
	if (unlikely(err)) {
		WL_ERR(("WLC_UP error (%d)\n", err));
	}
	return err;
}

#if !defined(OEM_EMBEDDED_LINUX)
static s32 wl_dongle_power(struct net_device *ndev, u32 power_mode)
{
	s32 err = 0;

	WL_TRACE(("In\n"));
	err =
	    wldev_ioctl(ndev, WLC_SET_PM, &power_mode, sizeof(power_mode),
			true);
	if (unlikely(err)) {
		WL_ERR(("WLC_SET_PM error (%d)\n", err));
	}
	return err;
}

static s32 wl_dongle_glom(struct net_device *ndev, u32 glom, u32 dongle_align)
{
	s8 iovbuf[WL_EVENTING_MASK_LEN + 12];

	s32 err = 0;

	/* Match Host and Dongle rx alignment */
	bcm_mkiovar("bus:txglomalign", (char *)&dongle_align, 4, iovbuf,
		    sizeof(iovbuf));
	err = wldev_ioctl(ndev, WLC_SET_VAR, iovbuf, sizeof(iovbuf), true);
	if (unlikely(err)) {
		WL_ERR(("txglomalign error (%d)\n", err));
		goto dongle_glom_out;
	}
	/* disable glom option per default */
	bcm_mkiovar("bus:txglom", (char *)&glom, 4, iovbuf, sizeof(iovbuf));
	err = wldev_ioctl(ndev, WLC_SET_VAR, iovbuf, sizeof(iovbuf), true);
	if (unlikely(err)) {
		WL_ERR(("txglom error (%d)\n", err));
		goto dongle_glom_out;
	}
dongle_glom_out:
	return err;
}

static s32 wl_dongle_roam(struct net_device *ndev, u32 roamvar, u32 bcn_timeout)
{
	s8 iovbuf[WL_EVENTING_MASK_LEN + 12];

	s32 err = 0;

	/* Setup timeout if Beacons are lost and roam is off to report link down */
	if (roamvar) {
		bcm_mkiovar("bcn_timeout", (char *)&bcn_timeout, 4, iovbuf,
			    sizeof(iovbuf));
		err =
		    wldev_ioctl(ndev, WLC_SET_VAR, iovbuf, sizeof(iovbuf),
				true);
		if (unlikely(err)) {
			WL_ERR(("bcn_timeout error (%d)\n", err));
			goto dongle_rom_out;
		}
	}
	/* Enable/Disable built-in roaming to allow supplicant to take care of roaming */
	bcm_mkiovar("roam_off", (char *)&roamvar, 4, iovbuf, sizeof(iovbuf));
	err = wldev_ioctl(ndev, WLC_SET_VAR, iovbuf, sizeof(iovbuf), true);
	if (unlikely(err)) {
		WL_ERR(("roam_off error (%d)\n", err));
		goto dongle_rom_out;
	}
dongle_rom_out:
	return err;
}

static s32
wl_dongle_scantime(struct net_device *ndev, s32 scan_assoc_time,
		   s32 scan_unassoc_time)
{
	s32 err = 0;

	err = wldev_ioctl(ndev, WLC_SET_SCAN_CHANNEL_TIME, &scan_assoc_time,
			  sizeof(scan_assoc_time), true);
	if (err) {
		if (err == -EOPNOTSUPP) {
			WL_INFO(("Scan assoc time is not supported\n"));
		} else {
			WL_ERR(("Scan assoc time error (%d)\n", err));
		}
		goto dongle_scantime_out;
	}
	err = wldev_ioctl(ndev, WLC_SET_SCAN_UNASSOC_TIME, &scan_unassoc_time,
			  sizeof(scan_unassoc_time), true);
	if (err) {
		if (err == -EOPNOTSUPP) {
			WL_INFO(("Scan unassoc time is not supported\n"));
		} else {
			WL_ERR(("Scan unassoc time error (%d)\n", err));
		}
		goto dongle_scantime_out;
	}

dongle_scantime_out:
	return err;
}

static s32 wl_dongle_offload(struct net_device *ndev, s32 arpoe, s32 arp_ol)
{
	/* Room for "event_msgs" + '\0' + bitvec */
	s8 iovbuf[WL_EVENTING_MASK_LEN + 12];

	s32 err = 0;

	/* Set ARP offload */
	bcm_mkiovar("arpoe", (char *)&arpoe, 4, iovbuf, sizeof(iovbuf));
	err = wldev_ioctl(ndev, WLC_SET_VAR, iovbuf, sizeof(iovbuf), true);
	if (err) {
		if (err == -EOPNOTSUPP)
			WL_INFO(("arpoe is not supported\n"));
		else
			WL_ERR(("arpoe error (%d)\n", err));

		goto dongle_offload_out;
	}
	bcm_mkiovar("arp_ol", (char *)&arp_ol, 4, iovbuf, sizeof(iovbuf));
	err = wldev_ioctl(ndev, WLC_SET_VAR, iovbuf, sizeof(iovbuf), true);
	if (err) {
		if (err == -EOPNOTSUPP)
			WL_INFO(("arp_ol is not supported\n"));
		else
			WL_ERR(("arp_ol error (%d)\n", err));

		goto dongle_offload_out;
	}

dongle_offload_out:
	return err;
}

static s32 wl_pattern_atoh(s8 * src, s8 * dst)
{
	int i;
	if (strncmp(src, "0x", 2) != 0 && strncmp(src, "0X", 2) != 0) {
		WL_ERR(("Mask invalid format. Needs to start with 0x\n"));
		return -1;
	}
	src = src + 2;		/* Skip past 0x */
	if (strlen(src) % 2 != 0) {
		WL_ERR(("Mask invalid format. Needs to be of even length\n"));
		return -1;
	}
	for (i = 0; *src != '\0'; i++) {
		char num[3];
		strncpy(num, src, 2);
		num[2] = '\0';
		dst[i] = (u8)simple_strtoul(num, NULL, 16);
		src += 2;
	}
	return i;
}

static s32 wl_dongle_filter(struct net_device *ndev, u32 filter_mode)
{
	/* Room for "event_msgs" + '\0' + bitvec */
	s8 iovbuf[WL_EVENTING_MASK_LEN + 12];

	const s8 *str;
	struct wl_pkt_filter pkt_filter;
	struct wl_pkt_filter *pkt_filterp;
	s32 buf_len;
	s32 str_len;
	u32 mask_size;
	u32 pattern_size;
	s8 buf[256];
	s32 err = 0;

	/* add a default packet filter pattern */
	str = "pkt_filter_add";
	str_len = strlen(str);
	strncpy(buf, str, str_len);
	buf[str_len] = '\0';
	buf_len = str_len + 1;

	pkt_filterp = (struct wl_pkt_filter *)(buf + str_len + 1);

	/* Parse packet filter id. */
	pkt_filter.id = htod32(100);

	/* Parse filter polarity. */
	pkt_filter.negate_match = htod32(0);

	/* Parse filter type. */
	pkt_filter.type = htod32(0);

	/* Parse pattern filter offset. */
	pkt_filter.u.pattern.offset = htod32(0);

	/* Parse pattern filter mask. */
	mask_size = htod32(wl_pattern_atoh("0xff",
					   (char *)pkt_filterp->u.pattern.
					   mask_and_pattern));

	/* Parse pattern filter pattern. */
	pattern_size = htod32(wl_pattern_atoh("0x00",
					      (char *)&pkt_filterp->u.
					      pattern.mask_and_pattern
					      [mask_size]));

	if (mask_size != pattern_size) {
		WL_ERR(("Mask and pattern not the same size\n"));
		err = -EINVAL;
		goto dongle_filter_out;
	}

	pkt_filter.u.pattern.size_bytes = mask_size;
	buf_len += WL_PKT_FILTER_FIXED_LEN;
	buf_len += (WL_PKT_FILTER_PATTERN_FIXED_LEN + 2 * mask_size);

	/* Keep-alive attributes are set in local
	 * variable (keep_alive_pkt), and
	 * then memcpy'ed into buffer (keep_alive_pktp) since there is no
	 * guarantee that the buffer is properly aligned.
	 */
	memcpy((char *)pkt_filterp, &pkt_filter,
	       WL_PKT_FILTER_FIXED_LEN + WL_PKT_FILTER_PATTERN_FIXED_LEN);

	err = wldev_ioctl(ndev, WLC_SET_VAR, buf, buf_len, true);
	if (err) {
		if (err == -EOPNOTSUPP) {
			WL_INFO(("filter not supported\n"));
		} else {
			WL_ERR(("filter (%d)\n", err));
		}
		goto dongle_filter_out;
	}

	/* set mode to allow pattern */
	bcm_mkiovar("pkt_filter_mode", (char *)&filter_mode, 4, iovbuf,
		    sizeof(iovbuf));
	err = wldev_ioctl(ndev, WLC_SET_VAR, iovbuf, sizeof(iovbuf), true);
	if (err) {
		if (err == -EOPNOTSUPP) {
			WL_INFO(("filter_mode not supported\n"));
		} else {
			WL_ERR(("filter_mode (%d)\n", err));
		}
		goto dongle_filter_out;
	}

dongle_filter_out:
	return err;
}
#endif

s32 dhd_config_dongle(struct wl_priv *wl, bool need_lock)
{
#ifndef DHD_SDALIGN
#define DHD_SDALIGN	32
#endif
	struct net_device *ndev;
	s32 err = 0;

	WL_TRACE(("In\n"));
	if (dhd_dongle_up) {
		WL_ERR(("Dongle is already up\n"));
		return err;
	}

	ndev = wl_to_prmry_ndev(wl);

	if (need_lock)
		rtnl_lock();

	err = wl_dongle_up(ndev, 0);
	if (unlikely(err)) {
		WL_ERR(("wl_dongle_up failed\n"));
		goto default_conf_out;
	}
#if !defined(OEM_EMBEDDED_LINUX)
	err = wl_dongle_power(ndev, PM_FAST);
	if (unlikely(err)) {
		WL_ERR(("wl_dongle_power failed\n"));
		goto default_conf_out;
	}
	err = wl_dongle_glom(ndev, 0, DHD_SDALIGN);
	if (unlikely(err)) {
		WL_ERR(("wl_dongle_glom failed\n"));
		goto default_conf_out;
	}
	err = wl_dongle_roam(ndev, (wl->roam_on ? 0 : 1), 3);
	if (unlikely(err)) {
		WL_ERR(("wl_dongle_roam failed\n"));
		goto default_conf_out;
	}
	wl_dongle_scantime(ndev, 40, 80);
	wl_dongle_offload(ndev, 1, 0xf);
	wl_dongle_filter(ndev, 1);
#endif
	dhd_dongle_up = true;

default_conf_out:
	if (need_lock)
		rtnl_unlock();
	return err;

}

/* TODO: clean up the BT-Coex code, it still have some legacy ioctl/iovar functions */
#define COEX_DHCP
