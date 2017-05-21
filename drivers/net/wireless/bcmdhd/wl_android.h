/*
 * Linux cfg80211 driver - Android related functions
 *
 * Copyright (C) 1999-2017, Broadcom Corporation
 * 
 *      Unless you and Broadcom execute a separate written software license
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
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: wl_android.h 587146 2015-09-18 03:50:09Z $
 */

#include <linux/module.h>
#include <linux/netdevice.h>
#include <wldev_common.h>

/* If any feature uses the Generic Netlink Interface, put it here to enable WL_GENL
 * automatically
 */
#if defined(BT_WIFI_HANDOVER)
#define WL_GENL
#endif


#ifdef WL_GENL
#include <net/genetlink.h>
#endif

typedef struct _android_wifi_priv_cmd {
    char *buf;
    int used_len;
    int total_len;
} android_wifi_priv_cmd;

#ifdef CONFIG_COMPAT
typedef struct _compat_android_wifi_priv_cmd {
    compat_caddr_t buf;
    int used_len;
    int total_len;
} compat_android_wifi_priv_cmd;
#endif /* CONFIG_COMPAT */

/**
 * Android platform dependent functions, feel free to add Android specific functions here
 * (save the macros in dhd). Please do NOT declare functions that are NOT exposed to dhd
 * or cfg, define them as static in wl_android.c
 */

/**
 * wl_android_init will be called from module init function (dhd_module_init now), similarly
 * wl_android_exit will be called from module exit function (dhd_module_cleanup now)
 */
int wl_android_init(void);
int wl_android_exit(void);
void wl_android_post_init(void);
int wl_android_wifi_on(struct net_device *dev);
int wl_android_wifi_off(struct net_device *dev, bool on_failure);
int wl_android_priv_cmd(struct net_device *net, struct ifreq *ifr, int cmd);
int wl_handle_private_cmd(struct net_device *net, char *command, u32 cmd_len);

#ifdef WL_GENL
typedef struct bcm_event_hdr {
	u16 event_type;
	u16 len;
} bcm_event_hdr_t;

/* attributes (variables): the index in this enum is used as a reference for the type,
 *             userspace application has to indicate the corresponding type
 *             the policy is used for security considerations
 */
enum {
	BCM_GENL_ATTR_UNSPEC,
	BCM_GENL_ATTR_STRING,
	BCM_GENL_ATTR_MSG,
	__BCM_GENL_ATTR_MAX
};
#define BCM_GENL_ATTR_MAX (__BCM_GENL_ATTR_MAX - 1)

/* commands: enumeration of all commands (functions),
 * used by userspace application to identify command to be ececuted
 */
enum {
	BCM_GENL_CMD_UNSPEC,
	BCM_GENL_CMD_MSG,
	__BCM_GENL_CMD_MAX
};
#define BCM_GENL_CMD_MAX (__BCM_GENL_CMD_MAX - 1)

/* Enum values used by the BCM supplicant to identify the events */
enum {
	BCM_E_UNSPEC,
	BCM_E_SVC_FOUND,
	BCM_E_DEV_FOUND,
	BCM_E_DEV_LOST,
#ifdef BT_WIFI_HANDOVER
	BCM_E_DEV_BT_WIFI_HO_REQ,
#endif
	BCM_E_MAX
};

s32 wl_genl_send_msg(struct net_device *ndev, u32 event_type,
	u8 *string, u16 len, u8 *hdr, u16 hdrlen);
#endif /* WL_GENL */
s32 wl_netlink_send_msg(int pid, int type, int seq, void *data, size_t size);

/* hostap mac mode */
#define MACLIST_MODE_DISABLED   0
#define MACLIST_MODE_DENY       1
#define MACLIST_MODE_ALLOW      2

/* max number of assoc list */
#define MAX_NUM_OF_ASSOCLIST    64

/* Bandwidth */
#define WL_CH_BANDWIDTH_20MHZ 20
#define WL_CH_BANDWIDTH_40MHZ 40
#define WL_CH_BANDWIDTH_80MHZ 80
/* max number of mac filter list
 * restrict max number to 10 as maximum cmd string size is 255
 */
#define MAX_NUM_MAC_FILT        10

int wl_android_set_ap_mac_list(struct net_device *dev, int macmode, struct maclist *maclist);
