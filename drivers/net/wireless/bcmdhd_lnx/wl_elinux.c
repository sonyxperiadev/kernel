/*
 * Linux cfg80211 driver - eLinux related functions
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
 * $Id: wl_elinux.c,v 1.1.4.1.2.14 2011/02/09 01:40:07 Exp $
 */

#include <linux/module.h>
#include <linux/netdevice.h>

#include <wldev_common.h>
#include <wlioctl.h>
#include <bcmutils.h>
#include <linux_osl.h>
#include <dhd_dbg.h>
#include <dngl_stats.h>
#include <dhd.h>
#include <bcmsdbus.h>
#ifdef WL_CFG80211
#include <wl_cfg80211.h>
#endif

/*
 * Driver private command strings, PLEASE define new private commands here
 * so they can be updated easily in the future (if needed)
 */

#define DRV_CMD_TEST				"TEST"
#define DRV_CMD_PM_0				"PM-0"	/* only for test */
#define DRV_CMD_PM_1				"PM-1"
#define DRV_CMD_PM_2				"PM-2"
#define DRV_CMD_ARPO_ENABLE		"ARPOFFLOAD-ENABLE"
#define DRV_CMD_ARPO_DISABLE		"ARPOFFLOAD-DISABLE"
#define DRV_CMD_RXFILTER_ENABLE	"RXFILTER-ENABLE"
#define DRV_CMD_RXFILTER_DISABLE	"RXFILTER-DISABLE"
#define DRV_CMD_RXFILTER_ADD		"RXFILTER-ADD"
#define DRV_CMD_RXFILTER_REMOVE	"RXFILTER-REMOVE"
#define DRV_CMD_RSSI				"RSSI"
#define DRV_CMD_LINKSPEED			"LINKSPEED"

#ifdef PNO_SUPPORT
#define DRV_CMD_PNO_SSIDCLR			"PNO-SSIDCLR"
#define DRV_CMD_PNO_SETUP			"PNO-SETUP"
#define DRV_CMD_PNO_ENABLE			"PNO-ENABLE"
#define DRV_CMD_PNO_DISABLE			"PNO-DISABLE"
#define DRV_CMD_PNO_DEBUG			"PNO-DEBUG"

#define PNO_TLV_PREFIX				'S'
#define PNO_TLV_VERSION				'1'
#define PNO_TLV_SUBVERSION 			'2'
#define PNO_TLV_RESERVED			'0'
#define PNO_TLV_TYPE_SSID_IE		"SSID"
#define PNO_TLV_TYPE_INTERVAL		"INTERVAL"
#define PNO_TLV_TYPE_REPEAT			"REPEAT"
#define PNO_TLV_TYPE_EXPO_MAX		"EXPO_MAX"
#define PNO_TLV_TYPE_LOST_TIME		"LOST"

#define PNO_MAX_SSID_ENTRY		16

typedef struct cmd_tlv {
	char prefix;
	char version;
	char subver;
	char reserved;
} cmd_tlv_t;

#ifdef PNO_SET_DEBUG
char pno_setup_example[] =
    "PNO-SETUP"
    ",S120"
    ",SSID,12,pno-test-psk,WPA-PSK"
    ",SSID,13,pno-test-psk2,WPA2-PSK"
    ",SSID,12,pno-test-wpa,WPA"
    ",SSID,13,pno-test-wpa2,WPA2"
    ",SSID,13,pno-test-open,OPEN"
    ",SSID,12,pno-test-wep,WEP"
    ",SSID,12,pno-test-any,ANY"
    ",INTERVAL,15" ",REPEAT,2" ",EXPO_MAX,3" ",LOST,120";
#endif /* PNO_SET_DEBUG */
#endif /* PNO_SUPPORT */

typedef struct wlan_driver_priv_cmd {
	char *buf;
	int used_len;
	int total_len;
} wlan_driver_priv_cmd;

/**
 * Extern function declarations (TODO: move them to dhd_linux.h)
 */
void dhd_customer_gpio_wlan_ctrl(int onoff);
uint dhd_dev_reset(struct net_device *dev, uint8 flag);
void dhd_dev_init_ioctl(struct net_device *dev);

#ifdef CONFIG_BCMDHD_LNX_IF_PREFIX
extern char iface_name[IFNAMSIZ];
#endif

/**
 * Local (static) functions and variables
 */

/* Initialize g_wifi_on to 1 so dhd_bus_start will be called for the first
 * time (only) in dhd_open, subsequential wifi on will be handled by
 * wl_elinux_wifi_on
 */
static int g_wifi_on = TRUE;

/**
 * Local (static) function definitions
 */
static int wl_elinux_get_link_speed(struct net_device *net, char *command,
				    int total_len)
{
	int link_speed;
	int bytes_written;
	int error;

	error = wldev_get_link_speed(net, &link_speed);
	if (error)
		return -1;

	/* Convert Kbps to elinux Mbps */
	link_speed = link_speed / 1000;
	bytes_written =
	    snprintf(command, total_len, "LinkSpeed %d", link_speed);
	DHD_INFO(("%s: command result is %s\n", __FUNCTION__, command));
	return bytes_written;
}

static int wl_elinux_get_rssi(struct net_device *net, char *command,
			      int total_len)
{
	wlc_ssid_t ssid = { 0 };
	int rssi;
	int bytes_written = 0;
	int error;

	error = wldev_get_rssi(net, &rssi);
	if (error)
		return -1;

	error = wldev_get_ssid(net, &ssid);
	if (error)
		return -1;
	if ((ssid.SSID_len == 0) || (ssid.SSID_len > DOT11_MAX_SSID_LEN)) {
		DHD_ERROR(("%s: wldev_get_ssid failed\n", __FUNCTION__));
	} else {
		memcpy(command, ssid.SSID, ssid.SSID_len);
		bytes_written = ssid.SSID_len;
	}
	bytes_written +=
	    snprintf(&command[bytes_written], total_len, " rssi %d", rssi);
	DHD_INFO(("%s: command result is %s (%d)\n", __FUNCTION__, command,
		  bytes_written));
	return bytes_written;
}

#if defined(PNO_SUPPORT) && !defined(WL_SCHED_SCAN)
#ifndef strtok_r
static char *strtok_r(char *s1, const char *s2, char **lasts)
{
	char *ret;

	if (s1 == NULL)
		s1 = *lasts;
	while (*s1 && strchr(s2, *s1))
		++s1;
	if (*s1 == '\0')
		return NULL;
	ret = s1;
	while (*s1 && !strchr(s2, *s1))
		++s1;
	if (*s1)
		*s1++ = '\0';
	*lasts = s1;
	return ret;
}
#endif

static int wl_elinux_set_pno_setup(struct net_device *dev, char *command,
				   int total_len)
{
	wl_pfn_t ssidnet[MAX_PFN_LIST_COUNT];
	int res = -1;
	int nssid = 0;
	cmd_tlv_t *cmd_tlv_temp;
	char *str_ptr;
	char *saved_ptr;
	int i;
	int pno_interval = 0;
	int pno_repeat = 0;
	int pno_expo_max = 0;
	int pno_lost_time = 0;

	DHD_INFO(("%s: command=%s, len=%d\n", __FUNCTION__, command,
		  total_len));

	if (total_len < (strlen(DRV_CMD_PNO_SETUP) + 1 + sizeof(cmd_tlv_t))) {
		DHD_ERROR(("%s argument=%d less min size\n", __FUNCTION__,
			   total_len));
		goto exit_proc;
	}

	/* eat command name */
	str_ptr = command + strlen(DRV_CMD_PNO_SETUP) + 1;
	/* check TLV version */
	str_ptr = strtok_r(str_ptr, ",", &saved_ptr);
	if (str_ptr) {
		cmd_tlv_temp = (cmd_tlv_t *) str_ptr;
		if ((cmd_tlv_temp->prefix == PNO_TLV_PREFIX) &&
		    (cmd_tlv_temp->version == PNO_TLV_VERSION) &&
		    (cmd_tlv_temp->subver == PNO_TLV_SUBVERSION)) {
			DHD_INFO(("%s: TLV version match: %s\n", __FUNCTION__,
				  str_ptr));
		} else {
			DHD_ERROR(("%s Invalid TLV version: %s\n", __FUNCTION__,
				   str_ptr));
			goto exit_proc;
		}
	}

	memset(ssidnet, 0, sizeof(ssidnet));

	/* next token */
	str_ptr = strtok_r(NULL, ",", &saved_ptr);
	if (!str_ptr) {
		DHD_ERROR(("%s Insufficient parameter\n", __FUNCTION__));
		goto exit_proc;
	}

	/* parse ssid element */
	while (nssid < PNO_MAX_SSID_ENTRY
	       && str_ptr && strcmp(PNO_TLV_TYPE_SSID_IE, str_ptr) == 0) {
		uint32 ssid_len = 0;

		/* get ssid len */
		str_ptr = strtok_r(NULL, ",", &saved_ptr);
		if (!str_ptr) {
			DHD_ERROR(("%s No SSID length found\n", __FUNCTION__));
			goto exit_proc;
		}

		ssid_len = simple_strtoul(str_ptr, &str_ptr, 10);
		if (ssid_len == 0 || ssid_len > DOT11_MAX_SSID_LEN) {
			DHD_ERROR(("%s Invalid SSID length %d\n", __FUNCTION__,
				   ssid_len));
			goto exit_proc;
		}

		/* get ssid - ssid can not be parsed with deliminator. use ssid_len */
		memcpy(ssidnet[nssid].ssid.SSID, saved_ptr, ssid_len);
		ssidnet[nssid].ssid.SSID_len = ssid_len;
		str_ptr = saved_ptr + ssid_len + 1;

		/* get security type */
		str_ptr = strtok_r(str_ptr, ",", &saved_ptr);
		if (!str_ptr) {
			DHD_ERROR(("%s No SSID Security type found\n",
				   __FUNCTION__));
			goto exit_proc;
		}
		if (strcmp("ANY", str_ptr) == 0) {
			ssidnet[nssid].wpa_auth = WPA_AUTH_PFN_ANY;
			ssidnet[nssid].wsec = 0;
		} else if (strcmp("OPEN", str_ptr) == 0) {
			ssidnet[nssid].wpa_auth = 0;
			ssidnet[nssid].wsec = 0;
		} else if (strcmp("WEP", str_ptr) == 0) {
			ssidnet[nssid].wpa_auth = 0;
			ssidnet[nssid].wsec = WEP_ENABLED;
		} else if (strcmp("WPA-PSK", str_ptr) == 0) {
			ssidnet[nssid].wpa_auth = WPA_AUTH_PSK;
			ssidnet[nssid].wsec = 0;
		} else if (strcmp("WPA2-PSK", str_ptr) == 0) {
			ssidnet[nssid].wpa_auth = WPA2_AUTH_PSK;
			ssidnet[nssid].wsec = 0;
		} else if (strcmp("WPA", str_ptr) == 0) {
			ssidnet[nssid].wpa_auth = WPA_AUTH_UNSPECIFIED;
			ssidnet[nssid].wsec = 0;
		} else if (strcmp("WPA2", str_ptr) == 0) {
			ssidnet[nssid].wpa_auth = WPA2_AUTH_UNSPECIFIED;
			ssidnet[nssid].wsec = 0;
		} else {
			DHD_ERROR(("%s Invalid SSID Security type found: %s\n",
				   __FUNCTION__, str_ptr));
			goto exit_proc;
		}

		ssidnet[nssid].flags = (ENABLE << WL_PFN_HIDDEN_BIT);
		ssidnet[nssid].infra = 1;
		ssidnet[nssid].auth = DOT11_OPEN_SYSTEM;
		nssid++;

		/* next token */
		str_ptr = strtok_r(NULL, ",", &saved_ptr);
	}

	/* parse PNO scan interval */
	if (str_ptr && strcmp(PNO_TLV_TYPE_INTERVAL, str_ptr) == 0) {
		str_ptr = strtok_r(NULL, ",", &saved_ptr);
		if (!str_ptr) {
			DHD_ERROR(("%s Invalid scan interval\n", __FUNCTION__));
			goto exit_proc;
		}
		pno_interval = simple_strtoul(str_ptr, &str_ptr, 10);

		/* next token */
		str_ptr = strtok_r(NULL, ",", &saved_ptr);
	} else {
		DHD_ERROR(("%s No scan interval found\n", __FUNCTION__));
		goto exit_proc;
	}

	/* parse PNO repeat (optional) */
	if (str_ptr && strcmp(PNO_TLV_TYPE_REPEAT, str_ptr) == 0) {
		str_ptr = strtok_r(NULL, ",", &saved_ptr);
		if (!str_ptr) {
			DHD_ERROR(("%s Invalid interval repeat\n",
				   __FUNCTION__));
			goto exit_proc;
		}
		pno_repeat = simple_strtoul(str_ptr, &str_ptr, 10);

		/* next token */
		str_ptr = strtok_r(NULL, ",", &saved_ptr);
	}

	/* parse PNO exponent max interval (optional) */
	if (str_ptr && strcmp(PNO_TLV_TYPE_EXPO_MAX, str_ptr) == 0) {
		str_ptr = strtok_r(NULL, ",", &saved_ptr);
		if (!str_ptr) {
			DHD_ERROR(("%s Invalid exponent max interval\n",
				   __FUNCTION__));
			goto exit_proc;
		}
		pno_expo_max = simple_strtoul(str_ptr, &str_ptr, 10);

		/* next token */
		str_ptr = strtok_r(NULL, ",", &saved_ptr);
	}

	/* parse PNO network lost time (optional) */
	if (str_ptr && strcmp(PNO_TLV_TYPE_LOST_TIME, str_ptr) == 0) {
		str_ptr = strtok_r(NULL, ",", &saved_ptr);
		if (!str_ptr) {
			DHD_ERROR(("%s Invalid network lost time\n",
				   __FUNCTION__));
			goto exit_proc;
		}
		pno_lost_time = simple_strtoul(str_ptr, &str_ptr, 10);
	}

	for (i = 0; i < nssid; i++) {
		DHD_INFO(("%s: [%d] SSID: %s\n", __FUNCTION__, i,
			  ssidnet[i].ssid.SSID));
		DHD_INFO(("%s: [%d] SSID_len: %d\n", __FUNCTION__, i,
			  ssidnet[i].ssid.SSID_len));
		DHD_INFO(("%s: [%d] flags: %x\n", __FUNCTION__, i,
			  ssidnet[i].flags));
		DHD_INFO(("%s: [%d] infra: %x\n", __FUNCTION__, i,
			  ssidnet[i].infra));
		DHD_INFO(("%s: [%d] auth: %x\n", __FUNCTION__, i,
			  ssidnet[i].auth));
		DHD_INFO(("%s: [%d] wpa_auth: %x\n", __FUNCTION__, i,
			  ssidnet[i].wpa_auth));
		DHD_INFO(("%s: [%d] wsec: %x\n", __FUNCTION__, i,
			  ssidnet[i].wsec));
	}
	DHD_INFO(("%s: scan interval: %d sec\n", __FUNCTION__, pno_interval));
	DHD_INFO(("%s: interval repeat: %d times\n", __FUNCTION__, pno_repeat));
	DHD_INFO(("%s: exponent max interval: %d x 2^%d sec\n", __FUNCTION__,
		  pno_interval, pno_expo_max));
	DHD_INFO(("%s: network lost time: %d sec\n", __FUNCTION__,
		  pno_lost_time));

	res =
	    dhd_dev_pno_set_ex(dev, ssidnet, nssid, pno_interval, pno_repeat,
			       pno_expo_max, pno_lost_time);

exit_proc:

	return res;
}
#endif /* PNO_SUPPORT && !WL_SCHED_SCAN */

/**
 * Global function definitions (declared in wl_elinux.h)
 */

int wl_elinux_wifi_on(struct net_device *dev)
{
	int ret = 0;

	printk("%s in\n", __FUNCTION__);
	if (!dev) {
		DHD_ERROR(("%s: dev is null\n", __FUNCTION__));
		return -EINVAL;
	}

	dhd_net_if_lock(dev);
	if (!g_wifi_on) {
		dhd_customer_gpio_wlan_ctrl(WLAN_RESET_ON);
		sdioh_start(NULL, 0);
		ret = dhd_dev_reset(dev, FALSE);
		sdioh_start(NULL, 1);
		if (!ret)
			dhd_dev_init_ioctl(dev);
		g_wifi_on = 1;
	}
	dhd_net_if_unlock(dev);

	return ret;
}

int wl_elinux_wifi_off(struct net_device *dev)
{
	int ret = 0;

	printk("%s in\n", __FUNCTION__);
	if (!dev) {
		DHD_TRACE(("%s: dev is null\n", __FUNCTION__));
		return -EINVAL;
	}

	dhd_net_if_lock(dev);
	if (g_wifi_on) {
		ret = dhd_dev_reset(dev, TRUE);
		sdioh_stop(NULL);
		dhd_customer_gpio_wlan_ctrl(WLAN_RESET_OFF);
		g_wifi_on = 0;
	}
	dhd_net_if_unlock(dev);

	return ret;
}

int wl_elinux_priv_cmd(struct net_device *net, struct ifreq *ifr, int cmd)
{
	int ret = 0;
	char *command = NULL;
	int bytes_written = 0;
	int val;
	wlan_driver_priv_cmd priv_cmd;

	net_os_wake_lock(net);

	if (!ifr->ifr_data) {
		ret = -EINVAL;
		goto exit;
	}
	if (copy_from_user
	    (&priv_cmd, ifr->ifr_data, sizeof(wlan_driver_priv_cmd))) {
		ret = -EFAULT;
		goto exit;
	}
	command = kmalloc(priv_cmd.total_len, GFP_KERNEL);
	if (!command) {
		DHD_ERROR(("%s: failed to allocate memory\n", __FUNCTION__));
		ret = -ENOMEM;
		goto exit;
	}
	if (copy_from_user(command, priv_cmd.buf, priv_cmd.total_len)) {
		ret = -EFAULT;
		goto exit;
	}

	DHD_INFO(("%s: WLAN driver private cmd \"%s\" on %s\n", __FUNCTION__,
		  command, ifr->ifr_name));

	if (strnicmp(command, DRV_CMD_TEST, strlen(DRV_CMD_TEST)) == 0) {
		DHD_ERROR(("%s, Received TEST command\n", __FUNCTION__));
	}

	if (!g_wifi_on) {
		DHD_ERROR(("%s: Ignore private cmd \"%s\" - iface %s is down\n",
			   __FUNCTION__, command, ifr->ifr_name));
		ret = 0;
		goto exit;
	}

	if (strnicmp(command, DRV_CMD_PM_1, strlen(DRV_CMD_PM_1)) == 0) {
		val = htod32(1);
		bytes_written =
		    wldev_ioctl(net, WLC_SET_PM, &val, sizeof(val), true);
	} else if (strnicmp(command, DRV_CMD_PM_2, strlen(DRV_CMD_PM_2)) == 0) {
		val = htod32(2);
		bytes_written =
		    wldev_ioctl(net, WLC_SET_PM, &val, sizeof(val), true);
	} else
	    if (strnicmp
		(command, DRV_CMD_ARPO_ENABLE,
		 strlen(DRV_CMD_ARPO_ENABLE)) == 0) {
		bytes_written = dhd_dev_arp_offload_enable(net, 1);
	} else
	    if (strnicmp
		(command, DRV_CMD_ARPO_DISABLE,
		 strlen(DRV_CMD_ARPO_DISABLE)) == 0) {
		bytes_written = dhd_dev_arp_offload_enable(net, 0);
	} else
	    if (strnicmp
		(command, DRV_CMD_RXFILTER_ENABLE,
		 strlen(DRV_CMD_RXFILTER_ENABLE)) == 0) {
		bytes_written = net_os_set_packet_filter(net, 1);
	} else
	    if (strnicmp
		(command, DRV_CMD_RXFILTER_DISABLE,
		 strlen(DRV_CMD_RXFILTER_DISABLE)) == 0) {
		bytes_written = net_os_set_packet_filter(net, 0);
	} else if (strnicmp(command, DRV_CMD_RSSI, strlen(DRV_CMD_RSSI)) == 0) {
		bytes_written =
		    wl_elinux_get_rssi(net, command, priv_cmd.total_len);
	} else
	    if (strnicmp(command, DRV_CMD_LINKSPEED, strlen(DRV_CMD_LINKSPEED))
		== 0) {
		bytes_written =
		    wl_elinux_get_link_speed(net, command, priv_cmd.total_len);
	}
#if defined(PNO_SUPPORT) && !defined(WL_SCHED_SCAN)
	else if (strnicmp
		 (command, DRV_CMD_PNO_SSIDCLR,
		  strlen(DRV_CMD_PNO_SSIDCLR)) == 0) {
		bytes_written = dhd_dev_pno_reset(net);
	} else
	    if (strnicmp(command, DRV_CMD_PNO_SETUP, strlen(DRV_CMD_PNO_SETUP))
		== 0) {
		bytes_written =
		    wl_elinux_set_pno_setup(net, command, priv_cmd.used_len);
	} else
	    if (strnicmp
		(command, DRV_CMD_PNO_ENABLE,
		 strlen(DRV_CMD_PNO_ENABLE)) == 0) {
		bytes_written = dhd_dev_pno_enable(net, 1);
	} else
	    if (strnicmp
		(command, DRV_CMD_PNO_DISABLE,
		 strlen(DRV_CMD_PNO_DISABLE)) == 0) {
		bytes_written = dhd_dev_pno_enable(net, 0);
	}
#ifdef PNO_SET_DEBUG
	else if (strnicmp(command, DRV_CMD_PNO_DEBUG, strlen(DRV_CMD_PNO_DEBUG))
		 == 0) {
		if (priv_cmd.total_len >= sizeof(pno_setup_example)) {
			/* command buffer is mangled by wl_elinux_set_pno_setup.
			 * copy example string to command buffer and pass it */
			memcpy(command, pno_setup_example,
			       sizeof(pno_setup_example));
			priv_cmd.used_len = sizeof(pno_setup_example);
		}
		bytes_written =
		    wl_elinux_set_pno_setup(net, command, priv_cmd.used_len);
	}
#endif /* PNO_SET_DEBUG */
#endif
	else {
		DHD_ERROR(("Unknown PRIVATE command %s - ignored\n", command));
		snprintf(command, 3, "OK");
		bytes_written = strlen("OK");
	}

	if (bytes_written >= 0) {
		if ((bytes_written == 0) && (priv_cmd.total_len > 0))
			command[0] = '\0';
		if (bytes_written >= priv_cmd.total_len) {
			DHD_ERROR(("%s: bytes_written = %d\n", __FUNCTION__,
				   bytes_written));
			bytes_written = priv_cmd.total_len;
		} else {
			bytes_written++;
		}
		priv_cmd.used_len = bytes_written;
		if (copy_to_user(priv_cmd.buf, command, bytes_written)) {
			DHD_ERROR(("%s: failed to copy data to user buffer\n",
				   __FUNCTION__));
			ret = -EFAULT;
		}
	} else {
		ret = bytes_written;
	}

exit:
	net_os_wake_unlock(net);
	if (command) {
		kfree(command);
	}

	return ret;
}

int wl_elinux_init(void)
{
	int ret = 0;

	dhd_msg_level |= DHD_ERROR_VAL;
#ifdef ENABLE_INSMOD_NO_FW_LOAD
	dhd_download_fw_on_driverload = FALSE;
#endif /* ENABLE_INSMOD_NO_FW_LOAD */
#ifdef CONFIG_BCMDHD_LNX_IF_PREFIX
	if (!iface_name[0]) {
		memset(iface_name, 0, IFNAMSIZ);
		bcm_strncpy_s(iface_name, IFNAMSIZ, CONFIG_BCMDHD_LNX_IF_PREFIX,
			      IFNAMSIZ);
	}
#endif /* CONFIG_BCMDHD_LNX_IF_PREFIX */
	return ret;
}

int wl_elinux_exit(void)
{
	int ret = 0;

	return ret;
}

void wl_elinux_post_init(void)
{
	if (!dhd_download_fw_on_driverload) {
		/* Call customer gpio to turn off power with WL_REG_ON signal */
		dhd_customer_gpio_wlan_ctrl(WLAN_RESET_OFF);
		g_wifi_on = 0;
	}
}
