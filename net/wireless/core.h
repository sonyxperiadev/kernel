/*
 * Wireless configuration interface internals.
 *
 * Copyright 2006-2010	Johannes Berg <johannes@sipsolutions.net>
 */
#ifndef __NET_WIRELESS_CORE_H
#define __NET_WIRELESS_CORE_H
#include <linux/mutex.h>
#include <linux/list.h>
#include <linux/netdevice.h>
#include <linux/rbtree.h>
#include <linux/debugfs.h>
#include <linux/rfkill.h>
#include <linux/workqueue.h>
#include <linux/rtnetlink.h>
#include <net/genetlink.h>
#include <net/cfg80211.h>
#include "reg.h"


#define WIPHY_IDX_INVALID	-1

struct cfg80211_registered_device {
	const struct cfg80211_ops *ops;
	struct list_head list;
	/* we hold this mutex during any call so that
	 * we cannot do multiple calls at once, and also
	 * to avoid the deregister call to proceed while
	 * any call is in progress */
	struct mutex mtx;

	/* rfkill support */
	struct rfkill_ops rfkill_ops;
	struct rfkill *rfkill;
	struct work_struct rfkill_sync;

	/* ISO / IEC 3166 alpha2 for which this device is receiving
	 * country IEs on, this can help disregard country IEs from APs
	 * on the same alpha2 quickly. The alpha2 may differ from
	 * cfg80211_regdomain's alpha2 when an intersection has occurred.
	 * If the AP is reconfigured this can also be used to tell us if
	 * the country on the country IE changed. */
	char country_ie_alpha2[2];

	/* If a Country IE has been received this tells us the environment
	 * which its telling us its in. This defaults to ENVIRON_ANY */
	enum environment_cap env;

	/* wiphy index, internal only */
	int wiphy_idx;

	/* associated wireless interfaces */
	struct mutex devlist_mtx;
	/* protected by devlist_mtx or RCU */
	struct list_head wdev_list;
	int devlist_generation, wdev_id;
	int opencount; /* also protected by devlist_mtx */
	wait_queue_head_t dev_wait;

	struct list_head beacon_registrations;
	spinlock_t beacon_registrations_lock;

	/* protected by RTNL only */
	int num_running_ifaces;
	int num_running_monitor_ifaces;

	/* BSSes/scanning */
	spinlock_t bss_lock;
	struct list_head bss_list;
	struct rb_root bss_tree;
	u32 bss_generation;
	u32 bss_entries;
	struct cfg80211_scan_request *scan_req; /* protected by RTNL */
	struct cfg80211_sched_scan_request *sched_scan_req;
	unsigned long suspend_at;
	struct work_struct scan_done_wk;
	struct work_struct sched_scan_results_wk;

	struct mutex sched_scan_mtx;

	struct genl_info *cur_cmd_info;

	struct work_struct conn_work;
	struct work_struct event_work;

	struct cfg80211_wowlan *wowlan;

	struct delayed_work dfs_update_channels_wk;

	/* netlink port which started critical protocol (0 means not started) */
	u32 crit_proto_nlportid;

	spinlock_t destroy_list_lock;
	struct list_head destroy_list;
	struct work_struct destroy_work;

	struct work_struct sched_scan_stop_wk;

	/* must be last because of the way we do wiphy_priv(),
	 * and it should at least be aligned to NETDEV_ALIGN */
	struct wiphy wiphy __aligned(NETDEV_ALIGN);
};

static inline
struct cfg80211_registered_device *wiphy_to_dev(struct wiphy *wiphy)
{
	BUG_ON(!wiphy);
	return container_of(wiphy, struct cfg80211_registered_device, wiphy);
}

static inline void
cfg80211_rdev_free_wowlan(struct cfg80211_registered_device *rdev)
{
	int i;

	if (!rdev->wowlan)
		return;
	for (i = 0; i < rdev->wowlan->n_patterns; i++)
		kfree(rdev->wowlan->patterns[i].mask);
	kfree(rdev->wowlan->patterns);
	if (rdev->wowlan->tcp && rdev->wowlan->tcp->sock)
		sock_release(rdev->wowlan->tcp->sock);
	kfree(rdev->wowlan->tcp);
	kfree(rdev->wowlan);
}

extern struct workqueue_struct *cfg80211_wq;
extern struct mutex cfg80211_mutex;
extern struct list_head cfg80211_rdev_list;
extern int cfg80211_rdev_list_generation;

static inline void assert_cfg80211_lock(void)
{
	lockdep_assert_held(&cfg80211_mutex);
}

struct cfg80211_internal_bss {
	struct list_head list;
	struct list_head hidden_list;
	struct rb_node rbn;
	unsigned long ts;
	unsigned long refcount;
	atomic_t hold;

	/* must be last because of priv member */
	struct cfg80211_bss pub;
};

static inline struct cfg80211_internal_bss *bss_from_pub(struct cfg80211_bss *pub)
{
	return container_of(pub, struct cfg80211_internal_bss, pub);
}

static inline void cfg80211_hold_bss(struct cfg80211_internal_bss *bss)
{
	atomic_inc(&bss->hold);
}

static inline void cfg80211_unhold_bss(struct cfg80211_internal_bss *bss)
{
	int r = atomic_dec_return(&bss->hold);
	WARN_ON(r < 0);
}


struct cfg80211_registered_device *cfg80211_rdev_by_wiphy_idx(int wiphy_idx);
int get_wiphy_idx(struct wiphy *wiphy);

/* requires cfg80211_rdev_mutex to be held! */
struct wiphy *wiphy_idx_to_wiphy(int wiphy_idx);

/* identical to cfg80211_get_dev_from_info but only operate on ifindex */
extern struct cfg80211_registered_device *
cfg80211_get_dev_from_ifindex(struct net *net, int ifindex);

int cfg80211_switch_netns(struct cfg80211_registered_device *rdev,
			  struct net *net);

static inline void cfg80211_lock_rdev(struct cfg80211_registered_device *rdev)
{
	mutex_lock(&rdev->mtx);
}

static inline void cfg80211_unlock_rdev(struct cfg80211_registered_device *rdev)
{
	BUG_ON(IS_ERR(rdev) || !rdev);
	mutex_unlock(&rdev->mtx);
}

static inline void wdev_lock(struct wireless_dev *wdev)
	__acquires(wdev)
{
	mutex_lock(&wdev->mtx);
	__acquire(wdev->mtx);
}

static inline void wdev_unlock(struct wireless_dev *wdev)
	__releases(wdev)
{
	__release(wdev->mtx);
	mutex_unlock(&wdev->mtx);
}

#define ASSERT_RDEV_LOCK(rdev) lockdep_assert_held(&(rdev)->mtx)
#define ASSERT_WDEV_LOCK(wdev) lockdep_assert_held(&(wdev)->mtx)

static inline bool cfg80211_has_monitors_only(struct cfg80211_registered_device *rdev)
{
	ASSERT_RTNL();

	return rdev->num_running_ifaces == rdev->num_running_monitor_ifaces &&
	       rdev->num_running_ifaces > 0;
}

enum cfg80211_event_type {
	EVENT_CONNECT_RESULT,
	EVENT_ROAMED,
	EVENT_DISCONNECTED,
	EVENT_IBSS_JOINED,
};

struct cfg80211_event {
	struct list_head list;
	enum cfg80211_event_type type;

	union {
		struct {
			u8 bssid[ETH_ALEN];
			const u8 *req_ie;
			const u8 *resp_ie;
			size_t req_ie_len;
			size_t resp_ie_len;
			u16 status;
		} cr;
		struct {
			const u8 *req_ie;
			const u8 *resp_ie;
			size_t req_ie_len;
			size_t resp_ie_len;
			struct cfg80211_bss *bss;
		} rm;
		struct {
			const u8 *ie;
			size_t ie_len;
			u16 reason;
		} dc;
		struct {
			u8 bssid[ETH_ALEN];
		} ij;
	};
};

struct cfg80211_cached_keys {
	struct key_params params[6];
	u8 data[6][WLAN_MAX_KEY_LEN];
	int def, defmgmt;
};

enum cfg80211_chan_mode {
	CHAN_MODE_UNDEFINED,
	CHAN_MODE_SHARED,
	CHAN_MODE_EXCLUSIVE,
};

struct cfg80211_beacon_registration {
	struct list_head list;
	u32 nlportid;
};

struct cfg80211_iface_destroy {
	struct list_head list;
	u32 nlportid;
};

void cfg80211_destroy_ifaces(struct cfg80211_registered_device *rdev);

/* free object */
extern void cfg80211_dev_free(struct cfg80211_registered_device *rdev);

extern int cfg80211_dev_rename(struct cfg80211_registered_device *rdev,
			       char *newname);

void ieee80211_set_bitrate_flags(struct wiphy *wiphy);

void cfg80211_bss_expire(struct cfg80211_registered_device *dev);
void cfg80211_bss_age(struct cfg80211_registered_device *dev,
                      unsigned long age_secs);

/* IBSS */
int __cfg80211_join_ibss(struct cfg80211_registered_device *rdev,
			 struct net_device *dev,
			 struct cfg80211_ibss_params *params,
			 struct cfg80211_cached_keys *connkeys);
int cfg80211_join_ibss(struct cfg80211_registered_device *rdev,
		       struct net_device *dev,
		       struct cfg80211_ibss_params *params,
		       struct cfg80211_cached_keys *connkeys);
void cfg80211_clear_ibss(struct net_device *dev, bool nowext);
int __cfg80211_leave_ibss(struct cfg80211_registered_device *rdev,
			  struct net_device *dev, bool nowext);
int cfg80211_leave_ibss(struct cfg80211_registered_device *rdev,
			struct net_device *dev, bool nowext);
void __cfg80211_ibss_joined(struct net_device *dev, const u8 *bssid);
int cfg80211_ibss_wext_join(struct cfg80211_registered_device *rdev,
			    struct wireless_dev *wdev);

/* mesh */
extern const struct mesh_config default_mesh_config;
extern const struct mesh_setup default_mesh_setup;
int __cfg80211_join_mesh(struct cfg80211_registered_device *rdev,
			 struct net_device *dev,
			 struct mesh_setup *setup,
			 const struct mesh_config *conf);
int cfg80211_join_mesh(struct cfg80211_registered_device *rdev,
		       struct net_device *dev,
		       struct mesh_setup *setup,
		       const struct mesh_config *conf);
int cfg80211_leave_mesh(struct cfg80211_registered_device *rdev,
			struct net_device *dev);
int cfg80211_set_mesh_channel(struct cfg80211_registered_device *rdev,
			      struct wireless_dev *wdev,
			      struct cfg80211_chan_def *chandef);

/* AP */
int cfg80211_stop_ap(struct cfg80211_registered_device *rdev,
		     struct net_device *dev);

/* MLME */
int __cfg80211_mlme_auth(struct cfg80211_registered_device *rdev,
			 struct net_device *dev,
			 struct ieee80211_channel *chan,
			 enum nl80211_auth_type auth_type,
			 const u8 *bssid,
			 const u8 *ssid, int ssid_len,
			 const u8 *ie, int ie_len,
			 const u8 *key, int key_len, int key_idx,
			 const u8 *sae_data, int sae_data_len);
int cfg80211_mlme_auth(struct cfg80211_registered_device *rdev,
		       struct net_device *dev, struct ieee80211_channel *chan,
		       enum nl80211_auth_type auth_type, const u8 *bssid,
		       const u8 *ssid, int ssid_len,
		       const u8 *ie, int ie_len,
		       const u8 *key, int key_len, int key_idx,
		       const u8 *sae_data, int sae_data_len);
int __cfg80211_mlme_assoc(struct cfg80211_registered_device *rdev,
			  struct net_device *dev,
			  struct ieee80211_channel *chan,
			  const u8 *bssid,
			  const u8 *ssid, int ssid_len,
			  struct cfg80211_assoc_request *req);
int cfg80211_mlme_assoc(struct cfg80211_registered_device *rdev,
			struct net_device *dev,
			struct ieee80211_channel *chan,
			const u8 *bssid,
			const u8 *ssid, int ssid_len,
			struct cfg80211_assoc_request *req);
int __cfg80211_mlme_deauth(struct cfg80211_registered_device *rdev,
			   struct net_device *dev, const u8 *bssid,
			   const u8 *ie, int ie_len, u16 reason,
			   bool local_state_change);
int cfg80211_mlme_deauth(struct cfg80211_registered_device *rdev,
			 struct net_device *dev, const u8 *bssid,
			 const u8 *ie, int ie_len, u16 reason,
			 bool local_state_change);
int cfg80211_mlme_disassoc(struct cfg80211_registered_device *rdev,
			   struct net_device *dev, const u8 *bssid,
			   const u8 *ie, int ie_len, u16 reason,
			   bool local_state_change);
void cfg80211_mlme_down(struct cfg80211_registered_device *rdev,
			struct net_device *dev);
void __cfg80211_connect_result(struct net_device *dev, const u8 *bssid,
			       const u8 *req_ie, size_t req_ie_len,
			       const u8 *resp_ie, size_t resp_ie_len,
			       u16 status, bool wextev,
			       struct cfg80211_bss *bss);
int cfg80211_mlme_register_mgmt(struct wireless_dev *wdev, u32 snd_pid,
				u16 frame_type, const u8 *match_data,
				int match_len);
void cfg80211_mlme_unregister_socket(struct wireless_dev *wdev, u32 nlpid);
void cfg80211_mlme_purge_registrations(struct wireless_dev *wdev);
int cfg80211_mlme_mgmt_tx(struct cfg80211_registered_device *rdev,
			  struct wireless_dev *wdev,
			  struct ieee80211_channel *chan, bool offchan,
			  unsigned int wait, const u8 *buf, size_t len,
			  bool no_cck, bool dont_wait_for_ack, u64 *cookie);
void cfg80211_oper_and_ht_capa(struct ieee80211_ht_cap *ht_capa,
			       const struct ieee80211_ht_cap *ht_capa_mask);
void cfg80211_oper_and_vht_capa(struct ieee80211_vht_cap *vht_capa,
				const struct ieee80211_vht_cap *vht_capa_mask);

/* SME */
int __cfg80211_connect(struct cfg80211_registered_device *rdev,
		       struct net_device *dev,
		       struct cfg80211_connect_params *connect,
		       struct cfg80211_cached_keys *connkeys,
		       const u8 *prev_bssid);
int cfg80211_connect(struct cfg80211_registered_device *rdev,
		     struct net_device *dev,
		     struct cfg80211_connect_params *connect,
		     struct cfg80211_cached_keys *connkeys);
int __cfg80211_disconnect(struct cfg80211_registered_device *rdev,
			  struct net_device *dev, u16 reason,
			  bool wextev);
int cfg80211_disconnect(struct cfg80211_registered_device *rdev,
			struct net_device *dev, u16 reason,
			bool wextev);
void __cfg80211_roamed(struct wireless_dev *wdev,
		       struct cfg80211_bss *bss,
		       const u8 *req_ie, size_t req_ie_len,
		       const u8 *resp_ie, size_t resp_ie_len);
int cfg80211_mgd_wext_connect(struct cfg80211_registered_device *rdev,
			      struct wireless_dev *wdev);

void cfg80211_conn_work(struct work_struct *work);
void cfg80211_sme_failed_assoc(struct wireless_dev *wdev);
bool cfg80211_sme_failed_reassoc(struct wireless_dev *wdev);

/* internal helpers */
bool cfg80211_supported_cipher_suite(struct wiphy *wiphy, u32 cipher);
int cfg80211_validate_key_settings(struct cfg80211_registered_device *rdev,
				   struct key_params *params, int key_idx,
				   bool pairwise, const u8 *mac_addr);
void __cfg80211_disconnected(struct net_device *dev, const u8 *ie,
			     size_t ie_len, u16 reason, bool from_ap);
void cfg80211_sme_scan_done(struct net_device *dev);
void cfg80211_sme_rx_auth(struct net_device *dev, const u8 *buf, size_t len);
void cfg80211_sme_disassoc(struct net_device *dev,
			   struct cfg80211_internal_bss *bss);
void __cfg80211_scan_done(struct work_struct *wk);
void ___cfg80211_scan_done(struct cfg80211_registered_device *rdev, bool leak);
void __cfg80211_sched_scan_results(struct work_struct *wk);
int __cfg80211_stop_sched_scan(struct cfg80211_registered_device *rdev,
			       bool driver_initiated);
void cfg80211_upload_connect_keys(struct wireless_dev *wdev);
int cfg80211_change_iface(struct cfg80211_registered_device *rdev,
			  struct net_device *dev, enum nl80211_iftype ntype,
			  u32 *flags, struct vif_params *params);
void cfg80211_process_rdev_events(struct cfg80211_registered_device *rdev);
void cfg80211_process_wdev_events(struct wireless_dev *wdev);

int cfg80211_can_use_iftype_chan(struct cfg80211_registered_device *rdev,
				 struct wireless_dev *wdev,
				 enum nl80211_iftype iftype,
				 struct ieee80211_channel *chan,
				 enum cfg80211_chan_mode chanmode,
				 u8 radar_detect);

/**
 * cfg80211_chandef_dfs_required - checks if radar detection is required
 * @wiphy: the wiphy to validate against
 * @chandef: the channel definition to check
 * Return: 1 if radar detection is required, 0 if it is not, < 0 on error
 */
int cfg80211_chandef_dfs_required(struct wiphy *wiphy,
				  const struct cfg80211_chan_def *c);

void cfg80211_set_dfs_state(struct wiphy *wiphy,
			    const struct cfg80211_chan_def *chandef,
			    enum nl80211_dfs_state dfs_state);

void cfg80211_dfs_channels_update_work(struct work_struct *work);


static inline int
cfg80211_can_change_interface(struct cfg80211_registered_device *rdev,
			      struct wireless_dev *wdev,
			      enum nl80211_iftype iftype)
{
	return cfg80211_can_use_iftype_chan(rdev, wdev, iftype, NULL,
					    CHAN_MODE_UNDEFINED, 0);
}

static inline int
cfg80211_can_add_interface(struct cfg80211_registered_device *rdev,
			   enum nl80211_iftype iftype)
{
	return cfg80211_can_change_interface(rdev, NULL, iftype);
}

static inline int
cfg80211_can_use_chan(struct cfg80211_registered_device *rdev,
		      struct wireless_dev *wdev,
		      struct ieee80211_channel *chan,
		      enum cfg80211_chan_mode chanmode)
{
	return cfg80211_can_use_iftype_chan(rdev, wdev, wdev->iftype,
					    chan, chanmode, 0);
}

static inline unsigned int elapsed_jiffies_msecs(unsigned long start)
{
	unsigned long end = jiffies;

	if (end >= start)
		return jiffies_to_msecs(end - start);

	return jiffies_to_msecs(end + (MAX_JIFFY_OFFSET - start) + 1);
}

void
cfg80211_get_chan_state(struct wireless_dev *wdev,
		        struct ieee80211_channel **chan,
		        enum cfg80211_chan_mode *chanmode);

int cfg80211_set_monitor_channel(struct cfg80211_registered_device *rdev,
				 struct cfg80211_chan_def *chandef);
int ieee80211_get_ratemask(struct ieee80211_supported_band *sband,
			   const u8 *rates, unsigned int n_rates,
			   u32 *mask);

int ieee80211_get_ratemask(struct ieee80211_supported_band *sband,
			   const u8 *rates, unsigned int n_rates,
			   u32 *mask);

int cfg80211_validate_beacon_int(struct cfg80211_registered_device *rdev,
				 u32 beacon_int);

void cfg80211_update_iface_num(struct cfg80211_registered_device *rdev,
			       enum nl80211_iftype iftype, int num);

void cfg80211_stop_p2p_device(struct cfg80211_registered_device *rdev,
			      struct wireless_dev *wdev);

#define CFG80211_MAX_NUM_DIFFERENT_CHANNELS 10

#ifdef CONFIG_CFG80211_DEVELOPER_WARNINGS
#define CFG80211_DEV_WARN_ON(cond)	WARN_ON(cond)
#else
/*
 * Trick to enable using it as a condition,
 * and also not give a warning when it's
 * not used that way.
 */
#define CFG80211_DEV_WARN_ON(cond)	({bool __r = (cond); __r; })
#endif

#endif /* __NET_WIRELESS_CORE_H */
