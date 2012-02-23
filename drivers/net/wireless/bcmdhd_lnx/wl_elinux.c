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
static int wl_elinux_get_link_speed(struct net_device *net, char *command, int total_len)
{
	int link_speed;
	int bytes_written;
	int error;

	error = wldev_get_link_speed(net, &link_speed);
	if (error)
		return -1;

	/* Convert Kbps to elinux Mbps */
	link_speed = link_speed / 1000;
	bytes_written = snprintf(command, total_len, "LinkSpeed %d", link_speed);
	DHD_INFO(("%s: command result is %s\n", __FUNCTION__, command));
	return bytes_written;
}

static int wl_elinux_get_rssi(struct net_device *net, char *command, int total_len)
{
	wlc_ssid_t ssid = {0};
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
	bytes_written += snprintf(&command[bytes_written], total_len, " rssi %d", rssi);
	DHD_INFO(("%s: command result is %s (%d)\n", __FUNCTION__, command, bytes_written));
	return bytes_written;
}

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
	if (copy_from_user(&priv_cmd, ifr->ifr_data, sizeof(wlan_driver_priv_cmd))) {
		ret = -EFAULT;
		goto exit;
	}
	command = kmalloc(priv_cmd.total_len, GFP_KERNEL);
	if (!command)
	{
		DHD_ERROR(("%s: failed to allocate memory\n", __FUNCTION__));
		ret = -ENOMEM;
		goto exit;
	}
	if (copy_from_user(command, priv_cmd.buf, priv_cmd.total_len)) {
		ret = -EFAULT;
		goto exit;
	}

	DHD_INFO(("%s: WLAN driver private cmd \"%s\" on %s\n", __FUNCTION__, command, ifr->ifr_name));

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
		bytes_written = wldev_ioctl(net, WLC_SET_PM, &val, sizeof(val), true);
	}
	else if (strnicmp(command, DRV_CMD_PM_2, strlen(DRV_CMD_PM_2)) == 0) {
		val = htod32(2);
		bytes_written = wldev_ioctl(net, WLC_SET_PM, &val, sizeof(val), true);
	}
#if 0 /* only for test */
	else if (strnicmp(command, DRV_CMD_PM_0, strlen(DRV_CMD_PM_0)) == 0) {
		val = htod32(0);
		bytes_written = wldev_ioctl(net, WLC_SET_PM, &val, sizeof(val), true);
	}
#endif /* only for test */
	else if (strnicmp(command, DRV_CMD_ARPO_ENABLE, strlen(DRV_CMD_ARPO_ENABLE)) == 0) {
		bytes_written = dhd_dev_arp_offload_enable(net, 1);
	}
	else if (strnicmp(command, DRV_CMD_ARPO_DISABLE, strlen(DRV_CMD_ARPO_DISABLE)) == 0) {
		bytes_written = dhd_dev_arp_offload_enable(net, 0);
	}
	else if (strnicmp(command, DRV_CMD_RXFILTER_ENABLE, strlen(DRV_CMD_RXFILTER_ENABLE)) == 0) {
		bytes_written = net_os_set_packet_filter(net, 1);
	}
	else if (strnicmp(command, DRV_CMD_RXFILTER_DISABLE, strlen(DRV_CMD_RXFILTER_DISABLE)) == 0) {
		bytes_written = net_os_set_packet_filter(net, 0);
	}
#if 0 /* TBD */
	else if (strnicmp(command, DRV_CMD_RXFILTER_ADD, strlen(DRV_CMD_RXFILTER_ADD)) == 0) {
		int filter_num = *(command + strlen(DRV_CMD_RXFILTER_ADD) + 1) - '0';
		bytes_written = net_os_rxfilter_add_remove(net, TRUE, filter_num);
	}
	else if (strnicmp(command, DRV_CMD_RXFILTER_REMOVE, strlen(DRV_CMD_RXFILTER_REMOVE)) == 0) {
		int filter_num = *(command + strlen(DRV_CMD_RXFILTER_REMOVE) + 1) - '0';
		bytes_written = net_os_rxfilter_add_remove(net, FALSE, filter_num);
	}
#endif /* TBD */
	else if (strnicmp(command, DRV_CMD_RSSI, strlen(DRV_CMD_RSSI)) == 0) {
		bytes_written = wl_elinux_get_rssi(net, command, priv_cmd.total_len);
	}
	else if (strnicmp(command, DRV_CMD_LINKSPEED, strlen(DRV_CMD_LINKSPEED)) == 0) {
		bytes_written = wl_elinux_get_link_speed(net, command, priv_cmd.total_len);
	}
	else {
		DHD_ERROR(("Unknown PRIVATE command %s - ignored\n", command));
		snprintf(command, 3, "OK");
		bytes_written = strlen("OK");
	}

	if (bytes_written >= 0) {
		if ((bytes_written == 0) && (priv_cmd.total_len > 0))
			command[0] = '\0';
		if (bytes_written >= priv_cmd.total_len) {
			DHD_ERROR(("%s: bytes_written = %d\n", __FUNCTION__, bytes_written));
			bytes_written = priv_cmd.total_len;
		} else {
			bytes_written++;
		}
		priv_cmd.used_len = bytes_written;
		if (copy_to_user(priv_cmd.buf, command, bytes_written)) {
			DHD_ERROR(("%s: failed to copy data to user buffer\n", __FUNCTION__));
			ret = -EFAULT;
		}
	}
	else {
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
		bcm_strncpy_s(iface_name, IFNAMSIZ, CONFIG_BCMDHD_LNX_IF_PREFIX, IFNAMSIZ);
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
