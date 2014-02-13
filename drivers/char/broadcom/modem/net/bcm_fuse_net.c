/****************************************************************************
*
*     Copyright (c) 2009 Broadcom Corporation
*
*   Unless you and Broadcom execute a separate written software license
*   agreement governing use of this software, this software is licensed to you
*   under the terms of the GNU General Public License version 2, available
*    at http://www.gnu.org/licenses/old-licenses/gpl-2.0.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*
****************************************************************************/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#ifdef CONFIG_HAS_WAKELOCK
#include <linux/wakelock.h>
#endif
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/workqueue.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/if_arp.h>

#define CSL_TYPES_H
#include <linux/broadcom/bcm_fuse_net_if.h>

#include "mobcom_types.h"
#include "resultcode.h"
#include "taskmsgs.h"

#include <linux/broadcom/ipcproperties.h>
#include "rpc_ipc.h"
#include "ipcinterface.h"

#include "xdr_porting_layer.h"
#include "xdr.h"
#include "rpc_api.h"

#include "rpc_ipc.h"

#include "bcm_net_debug.h"

#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif


static int __init bcm_fuse_net_init_module(void);
static void __exit bcm_fuse_net_exit_module(void);

/* used to generate valid RPC client ID for fuse_net driver */
extern unsigned char SYS_GenClientID(void);

/**
 * Packet Data EndPoint buffer pool info
 */
#define BCM_NET_MAX_DATA_LEN       1500	/* bytes */
#define BCM_NET_MAX_NUM_PKTS       250	/* packets */

#define BCM_DUALSIM_SIMID_NETIOCTL (SIOCDEVPRIVATE + 1)
#define BCM_MAX_SIM_ID 2

/* ip protocol header */
#define IPV6_PROTOCOL_HEADER 0x60
#define PROTOCOL_HEADER_OFFSET 0xf0

typedef enum {
	EFree = 0,
	EInUse = 1
} net_dev_entry_stat_t;

typedef struct {
	net_dev_entry_stat_t entry_stat;
	struct net_device *dev_ptr;
	uint8_t pdp_context_id;
	uint8_t sim_id;
	unsigned long ip_addr;
	struct net_device_stats stats;
} net_drvr_info_t;

struct net_tx {
	struct sk_buff_head queue;
	struct work_struct work;
	struct workqueue_struct *wq;
	unsigned long high_water_mark;
};

#define GET_NETDEV_IN_SKB_CB(__skb) \
	((struct net_device *)(*((unsigned long *)&((__skb)->cb[0]))))
#define PUT_NETDEV_IN_SKB_CB(__skb, dev) \
	(*((unsigned long *)&((__skb)->cb[0])) = (unsigned long)(dev))
#define QUEUE_MAX_SIZE  BCM_NET_MAX_NUM_PKTS

spinlock_t g_dev_lock;
static net_drvr_info_t g_net_dev_tbl[BCM_NET_MAX_PDP_CNTXS];
static struct net_tx g_net_tx;
static unsigned char g_NetClientId = 0;

static void bcm_fuse_net_fc_cb(RPC_FlowCtrlEvent_t event, unsigned char cid);
static RPC_Result_t bcm_fuse_net_bd_cb(PACKET_InterfaceType_t interfaceType,
				       unsigned char cid,
				       PACKET_BufHandle_t dataBufHandle);
static void bcm_fuse_net_init(struct net_device *dev_ptr);
static net_drvr_info_t *bcm_fuse_net_device_pdp_lookup(uint8_t pdp_context_id);
static uint8_t bcm_fuse_net_pdp_id(net_drvr_info_t *drvr_info_ptr);
static uint8_t bcm_fuse_net_find_entry(net_drvr_info_t *ndrvr_info_ptr);
static void bcm_fuse_net_free_entry(uint8_t pdp_cid);

/**
 * Definitions for bcm_fuse_net proc entry
 */
#define BCM_FUSE_NET_PROC_MAX_STR_LEN       15

static struct proc_dir_entry *bcm_fuse_net_config_proc_entry;


#ifdef CONFIG_BCM_NET_WHITELIST_SUPPORT

/*table 0-127 for tcp, 128-255 for udp, stored as array.
  each array item has either port number or 0(empty arrary space)*/
#define BCM_FUSE_NET_WL_PROC_NAME		"bcm_fuse_net_wl"
#define BCM_FUSE_NET_WL_DEBUG_PROC_NAME		"bcm_fuse_net_wl_debug"

#define MAX_WL_TCP_PORTS_CNT	128
#define MAX_WL_UDP_PORTS_CNT	128

/*ptr directly points to the IPC memory
  IPC persistent memory is arranged as
	1st byte - enable/disable flag, 1-enable, 0-disable
	2nd byte - reserved
	word - port
	word - port
  ...
  */
static uint8_t *ipc_whitelist_enable_ptr;
static uint16_t *ipc_whitelist_tbl;

/*this info list is internally used and will not be saved into IPC*/
struct whitelist_info_struct {
	unsigned int ui_enabled;	/*this is enabled from upper layer*/
	int tcp_port_end_idx;	/*so that we don't have to
				traverse whole table*/
	bool tcp_list_full;
	int udp_port_end_idx;
	bool udp_list_full;
};
static struct whitelist_info_struct	whitelist_info = {
						false, -1, false, -1, false};

static spinlock_t wl_lock;

/*proc for enable/disable whitelist*/
static struct proc_dir_entry *bcm_fuse_net_wl_proc_entry;
static ssize_t bcm_fuse_net_wl_proc_write(struct file *procFp,
				       const char __user *ubuff,
				       unsigned long len, void *data);
static int bcm_fuse_net_wl_proc_read(char *ubuff, char **start, off_t off,
				  int count, int *eof, void *data);

#ifdef CONFIG_BCM_NET_WHITELIST_DEBUG_SUPPORT
static struct proc_dir_entry *bcm_fuse_net_wl_debug_proc_entry;
static int bcm_fuse_net_wl_debug_proc_read(char *ubuff, char **start, off_t off,
				  int count, int *eof, void *data);
#endif

static int ipc_peoperty_setup(void);
static void set_ipc_property_wl_flag(uint8_t enable);
#endif


/**
 * Write function for bcm_fuse_net proc entry
 */
static ssize_t bcm_fuse_net_proc_write(struct file *procFp, const char __user * ubuff,
					size_t len, loff_t *pos)
{
	char uStr[BCM_FUSE_NET_PROC_MAX_STR_LEN];
	bool error = TRUE;
	int length = len;
	int proc_idx;
	int proc_c_id;
	int proc_sim_id;

	if (len > BCM_FUSE_NET_PROC_MAX_STR_LEN) {
		BNET_DEBUG(DBG_INFO, "%s: New settings string is too long!\n",
			   __FUNCTION__);
	} else if (copy_from_user(uStr, ubuff, len)) {
		BNET_DEBUG(DBG_INFO, "%s: Failed to copy new settings!\n",
			   __FUNCTION__);
		length = 0;
	} else if (sscanf(uStr, "%d %d %d", &proc_idx, &proc_c_id, &proc_sim_id)
		   != 3) {
		BNET_DEBUG(DBG_INFO, "%s: Failed to get new settings!\n",
				__func__);
	} else if ((proc_idx < 0) || (proc_idx >= BCM_NET_MAX_PDP_CNTXS)
		   || (proc_c_id < 1) || (proc_c_id > BCM_NET_MAX_PDP_CNTXS)
		   || (proc_sim_id < 1) || (proc_sim_id > 2)) {
		BNET_DEBUG(DBG_INFO,
			"%s: Invalid new settings! idx %d   c_id %d   sim_id %d\n",
			__func__, proc_idx, proc_c_id, proc_sim_id);
	} else {
		BNET_DEBUG(DBG_INFO,
			"%s: New settings:  idx %d   c_id %d   sim_id %d\n",
			__func__, proc_idx, proc_c_id, proc_sim_id);
		error = FALSE;
	}

	if (!error) {
		g_net_dev_tbl[proc_idx].pdp_context_id = proc_c_id;
		g_net_dev_tbl[proc_idx].sim_id = proc_sim_id;
	}

	return (ssize_t) length;
}

/**
  Read function for bcm_net proc entry
  */
static ssize_t bcm_fuse_net_proc_read(struct file *file, char __user *user_buf,
	size_t count, loff_t *ppos)
{
	unsigned int len = 0;
	int i;
	char *buf = kmalloc(count, GFP_KERNEL);

	if (!buf) {
		BNET_DEBUG(DBG_INFO, "%s: Fail to alloc mem\n", __func__);
		return 0;
	}

	len += snprintf(buf + len, count - len,
			"IFC     CID     SIM ID     Active\n");
	len += snprintf(buf + len, count - len,
			"---     ---     ------     ------\n");
	for (i = 0; i < BCM_NET_MAX_PDP_CNTXS && len < count; i++) {
		if (g_net_dev_tbl[i].dev_ptr != NULL) {
			len += snprintf(buf + len, count - len,
					"%s  %d      %d       %d\n",
					g_net_dev_tbl[i].dev_ptr->name,
					g_net_dev_tbl[i].pdp_context_id,
					g_net_dev_tbl[i].sim_id,
					g_net_dev_tbl[i].entry_stat);
		}
	}

	len = simple_read_from_buffer(user_buf, count, ppos, buf, len);

	kfree(buf);

	return len;
}

/**
   @fn void bcm_fuse_net_fc_cb(RPC_FlowCtrlEvent_t event, unsigned char8 cid);
 */
static void bcm_fuse_net_fc_cb(RPC_FlowCtrlEvent_t event, unsigned char cid)
{
	struct net_device *dev_ptr = NULL;
	int i;
/*
	net_drvr_info_t *ndrvr_info_ptr = NULL;

	if (printk_ratelimit())
	   BNET_DEBUG(DBG_INFO,"%s: flow control for all data channel\n", __FUNCTION__);

	Note that, cid here from rpc_ipc.c implmentation is the pool index, where current
	implementation in ipc is that "All channels use the same buffer pool"
	so when buffer pool is close to the limit, all channels have to stop flow.
	"cid" here means nothing then.
	leave the code there incase later on ipc supports multi pool for multi data channel
	ndrvr_info_ptr = &g_net_dev_tbl[0];//bcm_fuse_net_device_pool_lookup(pool);
*/
#if 0
	ndrvr_info_ptr = bcm_fuse_net_device_cid_lookup(cid);
	if (ndrvr_info_ptr == NULL) {
		if (printk_ratelimit())
			BNET_DEBUG(DBG_ERROR,
				   "%s: bcm_fuse_net_device_cid_lookup() failed\n",
				   __FUNCTION__);
		return;
	}

	dev_ptr = ndrvr_info_ptr->dev_ptr;
#endif

	if ((event != RPC_FLOW_START) && (event != RPC_FLOW_STOP)) {
		BNET_DEBUG(DBG_ERROR,
			   "%s: RECVD Unknown Flow Control Message !!\n",
			   __FUNCTION__);
		return;
	}

	for (i = 0; i < BCM_NET_MAX_PDP_CNTXS; i++) {
		if (g_net_dev_tbl[i].entry_stat == EInUse) {
			dev_ptr = g_net_dev_tbl[i].dev_ptr;
			if (event == RPC_FLOW_START) {
				if (printk_ratelimit())
					BNET_DEBUG(DBG_INFO,
						   "%s: RECVD RPC_FLOW_START!! cid=%d\n",
						   __FUNCTION__,
						   g_net_dev_tbl[i].
						   pdp_context_id);
				if (netif_queue_stopped(dev_ptr))
					netif_wake_queue(dev_ptr);
			} else {
				if (printk_ratelimit())
					BNET_DEBUG(DBG_INFO,
						   "%s: RECVD RPC_FLOW_STOP!! cid=%d\n",
						   __FUNCTION__,
						   g_net_dev_tbl[i].
						   pdp_context_id);
				netif_stop_queue(dev_ptr);
			}
		}
	}
	return;
}

/**
   @fn RPC_Result_t bcm_fuse_net_bd_cb(PACKET_InterfaceType_t interfaceType, unsigned char cid, PACKET_BufHandle_t dataBufHandle);
*/
static RPC_Result_t bcm_fuse_net_bd_cb(PACKET_InterfaceType_t interfaceType,
				       unsigned char cid,
				       PACKET_BufHandle_t dataBufHandle)
{
	unsigned long data_len = 0;
	struct sk_buff *skb = NULL;
	unsigned char *data_ptr = NULL;
	net_drvr_info_t *ndrvr_info_ptr = NULL;

	/* BNET_DEBUG(DBG_INFO,"%s: receive packet\n", __FUNCTION__); */

	data_len = RPC_PACKET_GetBufferLength(dataBufHandle);

	/* BNET_DEBUG(DBG_INFO,"%s: RECVD Buffer Delivery on AP Packet channel, cid[%d] size[%d]!!\n", __FUNCTION__, cid, data_len); */

	/* ndrvr_info_ptr = &g_net_dev_tbl[0]; */
	ndrvr_info_ptr = bcm_fuse_net_device_pdp_lookup(cid);
	if (ndrvr_info_ptr == NULL) {
		if (printk_ratelimit())
			BNET_DEBUG(DBG_ERROR,
				   "%s: bcm_fuse_net_device_pdp_lookup failed\n",
				   __FUNCTION__);

		return RPC_RESULT_ERROR;
	}

	skb = dev_alloc_skb(data_len);
	if (skb == NULL) {
		if (printk_ratelimit())
			BNET_DEBUG(DBG_ERROR,
				   "%s: dev_alloc_skb() failed - packet dropped\n",
				   __FUNCTION__);

		ndrvr_info_ptr->stats.rx_dropped++;
		return RPC_RESULT_ERROR;
	}

	data_ptr = (unsigned char *)RPC_PACKET_GetBufferData(dataBufHandle);
	memcpy(skb_put(skb, data_len), data_ptr, data_len);

	skb->dev = ndrvr_info_ptr->dev_ptr;
	/*skb->ip_summed = CHECKSUM_UNNECESSARY;*/	/* don't check it */
	skb->pkt_type = PACKET_HOST;
	ndrvr_info_ptr->dev_ptr->last_rx = jiffies;

	ndrvr_info_ptr->stats.rx_packets++;
	ndrvr_info_ptr->stats.rx_bytes += data_len;

	/* check if ipv4 or ipv6 */
	if ((data_ptr[0] & PROTOCOL_HEADER_OFFSET) == IPV6_PROTOCOL_HEADER) {
		BNET_DEBUG(DBG_TRACE, "%s: packet received as of ipv6\n",
			   __FUNCTION__);
		skb->protocol = htons(ETH_P_IPV6);
	} else {
		BNET_DEBUG(DBG_TRACE, "%s: packet received as of ipv4\n",
			   __FUNCTION__);
		skb->protocol = htons(ETH_P_IP);
	}

	BNET_DEBUG(DBG_TRACE, "%s: rx_bytes:%ld\n", __FUNCTION__,
		   ndrvr_info_ptr->stats.rx_bytes);

	netif_rx(skb);

	return RPC_RESULT_OK;
}

/* callback for CP silent reset events */
void bcm_fuse_net_cp_reset_cb(
	struct RpcNotificationEvent_t event)
{
	int i;
	struct net_device *dev_ptr = NULL;
	PACKET_InterfaceType_t interface = event.ifType;

	switch (event.event) {
	case RPC_CPRESET_EVT:
		BNET_DEBUG(DBG_INFO, "event %s interface %d\n",
		RPC_CPRESET_START == event.param ?
		"RPC_CPRESET_START" : "RPC_CPRESET_COMPLETE",
		interface);

	/* should just need to stop outgoing packet flow here
	   until we get RPC_CPRESET_COMPLETE
	*/
	if (event.param == RPC_CPRESET_START) {
		for (i = 0; i < BCM_NET_MAX_PDP_CNTXS; i++)
			if (g_net_dev_tbl[i].entry_stat == EInUse) {
				dev_ptr = g_net_dev_tbl[i].dev_ptr;
				netif_stop_queue(dev_ptr);
				BNET_DEBUG(DBG_INFO,
					"stopping interface %d\n", i);
			}

		/* for now, just ack... */
		RPC_PACKET_AckReadyForCPReset(0, INTERFACE_PACKET);
	} else if (event.param == RPC_CPRESET_COMPLETE) {
		for (i = 0; i < BCM_NET_MAX_PDP_CNTXS; i++)
			if (g_net_dev_tbl[i].entry_stat == EInUse) {
				dev_ptr = g_net_dev_tbl[i].dev_ptr;
				if (netif_queue_stopped(dev_ptr)) {
					netif_wake_queue(dev_ptr);
					BNET_DEBUG(DBG_INFO,
					"waking interface %d\n", i);
				}
			}
	} else
		BNET_DEBUG(DBG_INFO,
			"unexpected event param %d\n", (int)event.param);

	/* **FIXME** MAG - net interfaces should be brought down as
	   part of CP reset (for Android, RIL or DUN will do this). Are
	   there other situations where somebody else will need to bring
	   down the interface?
	*/

		break;
	default:
		BNET_DEBUG(DBG_INFO,
			"unexpected event %d\n", (int)event.event);
		break;
	}

	return;
}


static int bcm_fuse_net_open(struct net_device *dev)
{
	int i;
	net_drvr_info_t ndrvr_info_ptr;
	unsigned long flags;
	uint8_t idx = BCM_NET_MAX_PDP_CNTXS;	/* 0 */
	int ret = 0;
	static int IsFirstCall;	/* 0 */

	ndrvr_info_ptr.dev_ptr = dev;

    /**
       Register callbacks with the RPC Proxy server
    */
	if (0 == IsFirstCall) {
		g_NetClientId = SYS_GenClientID();
		ret = RPC_PACKET_RegisterDataInd(g_NetClientId,
						INTERFACE_PACKET,
						bcm_fuse_net_bd_cb,
						bcm_fuse_net_fc_cb,
						bcm_fuse_net_cp_reset_cb);
		if (RPC_RESULT_OK != ret) {
			BNET_DEBUG(DBG_ERROR,
				   "%s: first call client ID[%d] FAIL\n",
				   __FUNCTION__, g_NetClientId);
			return -1;
		}
		IsFirstCall++;
		BNET_DEBUG(DBG_INFO, "%s: first call client ID[%d]\n",
			   __FUNCTION__, g_NetClientId);
	}

	for (i = 0; i < BCM_NET_MAX_PDP_CNTXS; i++) {
		BNET_DEBUG(DBG_INFO,
			"%s: g_net_dev_tbl[%d]=0x%x,dev_ptr 0x%x, dev 0x%x, 0x%x\n",
			__FUNCTION__, i, (unsigned int)(&g_net_dev_tbl[i]),
			(unsigned int)(g_net_dev_tbl[i].dev_ptr),
			(unsigned int)dev, (unsigned int)(&ndrvr_info_ptr));
	}

	idx = bcm_fuse_net_find_entry(&ndrvr_info_ptr);
	if (idx == BCM_NET_MAX_PDP_CNTXS) {
		BNET_DEBUG(DBG_ERROR,
			"%s: No free device interface to assign for pdp_cid[%d]\n",
			__FUNCTION__, idx);
		return -EISCONN;
	}

	spin_lock_irqsave(&g_dev_lock, flags);
	/* g_net_dev_tbl[idx].pdp_context_id = idx+1; */
	g_net_dev_tbl[idx].pdp_context_id = RMNET_TO_CID(idx);

	spin_unlock_irqrestore(&g_dev_lock, flags);
	BNET_DEBUG(DBG_INFO,
		   "%s: BCM_FUSE_NET_ACTIVATE_PDP: rmnet[%d] pdp_info.cid=%d, jin hack 1\n",
		   __FUNCTION__, idx, g_net_dev_tbl[idx].pdp_context_id);

	netif_start_queue(dev);

	return 0;
}

static int bcm_fuse_net_stop(struct net_device *dev)
{
	int i;
	BNET_DEBUG(DBG_INFO, "%s: <<\n", __FUNCTION__);
	for (i = 0; i < BCM_NET_MAX_PDP_CNTXS; i++) {
		if (g_net_dev_tbl[i].dev_ptr == dev) {
			bcm_fuse_net_free_entry(g_net_dev_tbl[i].
						pdp_context_id);
			BNET_DEBUG(DBG_INFO,
				   "%s: free g_net_dev_tbl[%d].cid:%d\n",
				   __FUNCTION__, i,
				   g_net_dev_tbl[i].pdp_context_id);
			break;
		}
	}
	netif_stop_queue(dev);
	return 0;
}

static int __bcm_fuse_net_tx(struct sk_buff *skb, struct net_device *dev)
{
	void *buff_data_ptr;
	uint8_t pdp_cid = BCM_NET_MAX_PDP_CNTXS;
	PACKET_BufHandle_t buffer;

	static int sim_id;
	net_drvr_info_t *t_ndrvr_info_ptr = NULL;
	int i;

	for (i = 0; i < BCM_NET_MAX_PDP_CNTXS; i++) {
		if (g_net_dev_tbl[i].dev_ptr == dev) {
			sim_id = g_net_dev_tbl[i].sim_id;
			t_ndrvr_info_ptr = &g_net_dev_tbl[i];
			BNET_DEBUG(DBG_TRACE,
				   "%s: g_net_dev_tbl[%d]=0x%x, a_sim_id %d, sim_id %d\n",
				   __FUNCTION__, i,
				   (unsigned int)(&g_net_dev_tbl[i]),
				   g_net_dev_tbl[i].sim_id, sim_id);
			break;
		}
	}

	if (NULL == t_ndrvr_info_ptr) {
		BNET_DEBUG(DBG_ERROR, "bcm_fuse_net_tx(), no device found\n");
		return -EINVAL;
	}

	if (BCM_NET_MAX_DATA_LEN < skb->len) {
		BNET_DEBUG(DBG_ERROR,
			   "%s: len[%d] exceeds supported len[%d] failed\n",
			   __FUNCTION__, skb->len, BCM_NET_MAX_DATA_LEN);
		t_ndrvr_info_ptr->stats.tx_errors++;
		return -1;
	}

	if (0 == skb->len) {
		BNET_DEBUG(DBG_ERROR, "%s: len[%d] is zero size failed\n",
			   __FUNCTION__, skb->len);
		return -1;
	}

	pdp_cid = bcm_fuse_net_pdp_id(t_ndrvr_info_ptr);
	if (BCM_NET_INVALID_PDP_CNTX == pdp_cid) {
		BNET_DEBUG(DBG_ERROR,
			   "%s: net device to pdp context id mapping failed\n",
			   __FUNCTION__);
		t_ndrvr_info_ptr->stats.tx_errors++;
		return -1;
	}
	/* Allocate a buffer */
	buffer = RPC_PACKET_AllocateBuffer(INTERFACE_PACKET, skb->len, pdp_cid);
	if (!buffer) {
		BNET_DEBUG(DBG_ERROR, "%s: Error buffer Handle cid %d\n",
			   __FUNCTION__, pdp_cid);
		t_ndrvr_info_ptr->stats.tx_errors++;
		return -ENOBUFS;
	}
	/* transfer data from skb to ipc_buffer */
	buff_data_ptr = RPC_PACKET_GetBufferData(buffer);
	if (buff_data_ptr == NULL) {
		BNET_DEBUG(DBG_ERROR, "%s: RPC_PACKET_GetBufferData() failed\n",
			   __FUNCTION__);
		t_ndrvr_info_ptr->stats.tx_errors++;
		return -ENOBUFS;
	}

	memset(buff_data_ptr, 0, BCM_NET_MAX_DATA_LEN);
	memcpy(buff_data_ptr, skb->data, skb->len);

	RPC_PACKET_SetBufferLength(buffer, skb->len);

	dev->trans_start = jiffies;	/* save the timestamp */
	RPC_PACKET_SetContext(INTERFACE_PACKET, buffer, sim_id);
	RPC_PACKET_SendData(g_NetClientId, INTERFACE_PACKET, pdp_cid, buffer);

    /**
      The IPC buffer is freed by the receiving end point.
    */
	t_ndrvr_info_ptr->stats.tx_packets++;
	t_ndrvr_info_ptr->stats.tx_bytes += skb->len;
	BNET_DEBUG(DBG_TRACE, "%s: tx_bytes:%ld simid:%d cid:%d\n",
		   __FUNCTION__, t_ndrvr_info_ptr->stats.tx_bytes, sim_id,
		   pdp_cid);

	dev_kfree_skb(skb);

	return 0;
}

static void tx_work(struct work_struct *work)
{
	struct net_device *dev = NULL;
	struct sk_buff *skb;
	int i;

	while ((skb = skb_dequeue(&g_net_tx.queue))) {
		dev = GET_NETDEV_IN_SKB_CB(skb);
		for (i = 0; i < 2; i++) {
			if (__bcm_fuse_net_tx(skb, dev) == -ENOBUFS) {
				BNET_DEBUG(DBG_ERROR,
				"No AP-CP shared buffer, try again\n");
				msleep(32);
				continue;
			}
			break;
		}
	}

	if (dev && netif_queue_stopped(dev)) {
		BNET_DEBUG(DBG_ERROR, "Wake uper layer tx\n");
		netif_wake_queue(dev);
	}
}

static int bcm_fuse_net_tx(struct sk_buff *skb, struct net_device *dev)
{
	__u32 qlen = skb_queue_len(&g_net_tx.queue);

	if (qlen >= QUEUE_MAX_SIZE) {
		netif_stop_queue(dev);
		BNET_DEBUG(DBG_ERROR,
		"Stop uper layer tx(queue full)\n");
		return NETDEV_TX_BUSY;
	}

	PUT_NETDEV_IN_SKB_CB(skb, dev);
	skb_queue_tail(&g_net_tx.queue, skb);
	qlen = skb_queue_len(&g_net_tx.queue);
	if (qlen > g_net_tx.high_water_mark) {
		g_net_tx.high_water_mark = qlen;
		BNET_DEBUG(DBG_ERROR,
		"Update qlen high water mark: %u\n", qlen);
	}
	queue_work(g_net_tx.wq, &g_net_tx.work);

	return NETDEV_TX_OK;
}

static struct net_device_stats *bcm_fuse_net_stats(struct net_device *dev)
{
	net_drvr_info_t *ndrvr_info_ptr = NULL;
	int i;

	for (i = 0; i < BCM_NET_MAX_PDP_CNTXS; i++) {
		if (g_net_dev_tbl[i].dev_ptr == dev) {
			ndrvr_info_ptr = &g_net_dev_tbl[i];
			/* BNET_DEBUG(DBG_INFO,"%s: g_net_dev_tbl[%d]=0x%x\n", __FUNCTION__, i, &g_net_dev_tbl[i]); */
			break;
		}
	}

	return &ndrvr_info_ptr->stats;
}

/**
   @fn int bcm_fuse_net_config(struct net_device *dev_ptr, struct ifmap *map);
*/
int bcm_fuse_net_config(struct net_device *dev_ptr, struct ifmap *map)
{
	BNET_DEBUG(DBG_INFO, "%s: <<\n", __FUNCTION__);
	if (dev_ptr->flags & IFF_UP)	/* can't act on a running interface */
		return -EBUSY;

	return 0;
}

/**
   @fn int bcm_fuse_net_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd);
*/
int bcm_fuse_net_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
	int i;
	int sim_id = 3;

	BNET_DEBUG(DBG_INFO, "bcm_fuse_net_ioctl cmd=%d\n", cmd);

	if (BCM_DUALSIM_SIMID_NETIOCTL != cmd) {
		BNET_DEBUG(DBG_INFO, "%s: Incorrect IOCTL ID 0x%x\n",
			   __FUNCTION__, cmd);
		return -ENOTTY;
	}

	/*get sim_id*/
	if (copy_from_user(&sim_id, ifr->ifr_data, sizeof(int))) {
		BNET_DEBUG(DBG_ERROR, "error reading ifr_data\n");
		return -EFAULT;
	}

	if ((sim_id < 0) || (sim_id > BCM_MAX_SIM_ID)) {
		BNET_DEBUG(DBG_ERROR, "invalid sim id=%d", sim_id);
		return -EINVAL;
	}

	for (i = 0; i < BCM_NET_MAX_PDP_CNTXS; i++) {
		if (g_net_dev_tbl[i].dev_ptr == dev) {
			g_net_dev_tbl[i].sim_id = sim_id;
			BNET_DEBUG(DBG_ERROR,
				   "%s: g_net_dev_tbl[%d]=0x%x, a_sim_id:%d, sim_id:%d\n",
				   __FUNCTION__, i,
				   (unsigned int)(&g_net_dev_tbl[i]),
				   g_net_dev_tbl[i].sim_id, sim_id);
			break;
		}
	}
	if (i < BCM_NET_MAX_PDP_CNTXS)  /*To avoid Out-of-bounds read*/
	BNET_DEBUG(DBG_ERROR, "%s: Use A SIM ID:%d, SIM ID:%d\n", __FUNCTION__,
				g_net_dev_tbl[i].sim_id, sim_id);
	return 0;
}

/**
   Initialize a net device. (Called from kernel in alloc_netdev())

   @fn static void bcm_fuse_net_init(struct net_device *dev_ptr);
*/

static const struct net_device_ops bcm_netdev_ops = {
	.ndo_open = bcm_fuse_net_open,
	.ndo_stop = bcm_fuse_net_stop,
	.ndo_start_xmit = bcm_fuse_net_tx,
	.ndo_get_stats = bcm_fuse_net_stats,
	.ndo_set_config = bcm_fuse_net_config,
	.ndo_do_ioctl = bcm_fuse_net_ioctl,
};

static void bcm_fuse_net_init(struct net_device *dev)
{
	dev->netdev_ops = &bcm_netdev_ops;

	dev->mtu = BCM_NET_MAX_DATA_LEN;
	dev->tx_queue_len = BCM_NET_MAX_NUM_PKTS;
	dev->flags = IFF_NOARP | IFF_DYNAMIC;
	dev->type  = ARPHRD_NONE;   /*per MobC00183378 request*/
}

/**
   @fn static int bcm_fuse_net_attach(unsigned int dev_index);
*/
static int bcm_fuse_net_attach(unsigned int dev_index)
{
	int ret = 0;
	unsigned long flags;
	struct net_device *dev_ptr = NULL;

	if (dev_index >= BCM_NET_MAX_PDP_CNTXS) {
		BNET_DEBUG(DBG_ERROR,
			   "%s: Invalid dev_index[%d] passed as parameter\n",
			   __FUNCTION__, dev_index);
		return -1;
	}

	/* Allocate etherdev, including space for private structure */
	dev_ptr = alloc_netdev(sizeof(net_drvr_info_t *), BCM_NET_DEV_STR,
			       bcm_fuse_net_init);
	if (dev_ptr == NULL) {
		BNET_DEBUG(DBG_ERROR, "%s: Out of memory\n", __FUNCTION__);
		return -1;
	}
	/* Allocate Buffer pool for this channel */

	spin_lock_irqsave(&g_dev_lock, flags);

	/* dev_ptr->priv = &g_net_dev_tbl[dev_index]; from kernel 2.6.29 */
	g_net_dev_tbl[dev_index].dev_ptr = dev_ptr;
	g_net_dev_tbl[dev_index].entry_stat = EFree;
	g_net_dev_tbl[dev_index].pdp_context_id = BCM_NET_MAX_PDP_CNTXS;
	g_net_dev_tbl[dev_index].sim_id = 1;	/* default set to SIM ID 1 */
	BNET_DEBUG(DBG_INFO, "%s: g_net_dev_tbl[%d] = 0x%x, dev_ptr 0x%x\n",
		   __FUNCTION__, dev_index,
		   (unsigned int)(&g_net_dev_tbl[dev_index]),
		   (unsigned int)dev_ptr);
	BNET_DEBUG(DBG_INFO, "%s: entry_stat 0x%x\n", __FUNCTION__,
		   g_net_dev_tbl[dev_index].entry_stat);

	spin_unlock_irqrestore(&g_dev_lock, flags);

	ret = register_netdev(dev_ptr);
	if (ret != 0) {
		BNET_DEBUG(DBG_ERROR,
			   "%s: Error [%d] registering device \"%s\"\n",
			   __FUNCTION__, ret, BCM_NET_DEV_STR);

		/*error recovery, do clean up*/
		spin_lock_irqsave(&g_dev_lock, flags);
#ifdef FUSE_NET_NAPI
		netif_napi_del(&(g_net_dev_tbl[dev_index].rx_napi.napiInfo));
#endif
		memset(&g_net_dev_tbl[dev_index], 0, sizeof(net_drvr_info_t));
		spin_unlock_irqrestore(&g_dev_lock, flags);
		return -1;
	}

	return 0;
}

/**
   @fn static int bcm_fuse_net_deattach(unsigned int  dev_index);
*/
static int bcm_fuse_net_deattach(unsigned int dev_index)
{
	unsigned long flags;

	if (dev_index >= BCM_NET_MAX_PDP_CNTXS) {
		BNET_DEBUG(DBG_ERROR,
			   "%s: Invalid dev_index[%d] passed as parameter\n",
			   __FUNCTION__, dev_index);
		return -1;
	}

	spin_lock_irqsave(&g_dev_lock, flags);

	unregister_netdev(g_net_dev_tbl[dev_index].dev_ptr);
	free_netdev(g_net_dev_tbl[dev_index].dev_ptr);

	/* Reset most entries except for the buffer pool */
	g_net_dev_tbl[dev_index].dev_ptr = NULL;
	g_net_dev_tbl[dev_index].entry_stat = EFree;
	g_net_dev_tbl[dev_index].ip_addr = 0;
	g_net_dev_tbl[dev_index].pdp_context_id = BCM_NET_MAX_PDP_CNTXS;
	g_net_dev_tbl[dev_index].sim_id = 0;
	memset(&g_net_dev_tbl[dev_index].stats, 0,
	       sizeof(struct net_device_stats));

	spin_unlock_irqrestore(&g_dev_lock, flags);

	return 0;
}

/**
   @fn static net_drvr_info_t *bcm_fuse_net_device_pdp_lookup(uint8_t pdp_context_id);
*/
static net_drvr_info_t *bcm_fuse_net_device_pdp_lookup(uint8_t pdp_context_id)
{
	int i = 0;
	unsigned long flags;
	net_drvr_info_t *drvr_info_ptr = NULL;

	spin_lock_irqsave(&g_dev_lock, flags);

	for (i = 0; i < BCM_NET_MAX_PDP_CNTXS; i++) {
		if (g_net_dev_tbl[i].entry_stat == EFree)
			continue;

		if (g_net_dev_tbl[i].pdp_context_id == pdp_context_id) {
			drvr_info_ptr = &g_net_dev_tbl[i];
			goto FOUND_ENTRY;
		}
	}

	if (printk_ratelimit())
		BNET_DEBUG(DBG_ERROR,
		   "%s: No network device mapping for PDP context id[%d]\n",
		   __FUNCTION__, pdp_context_id);

FOUND_ENTRY:
	spin_unlock_irqrestore(&g_dev_lock, flags);

	return drvr_info_ptr;
}

/**
   @fn static uint8_t bcm_fuse_net_find_entry(net_drvr_info_t *ndrvr_info_ptr);
*/
static uint8_t bcm_fuse_net_find_entry(net_drvr_info_t *ndrvr_info_ptr)
{
	int i = 0;
	unsigned long flags;
	uint8_t dev_idx = BCM_NET_MAX_PDP_CNTXS;

	if (ndrvr_info_ptr == NULL)
		return dev_idx;

	spin_lock_irqsave(&g_dev_lock, flags);

	for (i = 0; i < BCM_NET_MAX_PDP_CNTXS; i++) {
		if ((g_net_dev_tbl[i].dev_ptr == ndrvr_info_ptr->dev_ptr) &&
		    (g_net_dev_tbl[i].entry_stat == EFree)) {
			dev_idx = i;
			g_net_dev_tbl[i].entry_stat = EInUse;
			goto FOUND_ENTRY;
		}
	}

	BNET_DEBUG(DBG_ERROR, "%s: Entry for device interface in use\n",
		   __FUNCTION__);

FOUND_ENTRY:
	spin_unlock_irqrestore(&g_dev_lock, flags);

	return dev_idx;
}

/**
   @fn static void bcm_fuse_net_free_entry(uint8_t pdp_cid);
*/
static void bcm_fuse_net_free_entry(uint8_t pdp_cid)
{
	int i = 0;
	unsigned long flags;

	if (pdp_cid == BCM_NET_INVALID_PDP_CNTX)
		return;

	spin_lock_irqsave(&g_dev_lock, flags);

	for (i = 0; i < BCM_NET_MAX_PDP_CNTXS; i++) {
		if (g_net_dev_tbl[i].pdp_context_id == pdp_cid) {
			g_net_dev_tbl[i].entry_stat = EFree;
			g_net_dev_tbl[i].pdp_context_id =
			    BCM_NET_INVALID_PDP_CNTX;
			g_net_dev_tbl[i].ip_addr = 0;
			goto FREE_ENTRY_EXIT;
		}
	}

FREE_ENTRY_EXIT:
	spin_unlock_irqrestore(&g_dev_lock, flags);

	return;
}

/**
   @fn static uint8_t bcm_fuse_net_pdp_id(net_drvr_info_t *drvr_info_ptr);
*/
static uint8_t bcm_fuse_net_pdp_id(net_drvr_info_t *drvr_info_ptr)
{
	int i = 0;
	unsigned long flags;
	uint8_t pdp_cid = BCM_NET_INVALID_PDP_CNTX;

	if (drvr_info_ptr == NULL)
		return pdp_cid;

	spin_lock_irqsave(&g_dev_lock, flags);

	for (i = 0; i < BCM_NET_MAX_PDP_CNTXS; i++) {
		if (g_net_dev_tbl[i].entry_stat == EFree)
			continue;

		if (g_net_dev_tbl[i].dev_ptr == drvr_info_ptr->dev_ptr) {
			pdp_cid = g_net_dev_tbl[i].pdp_context_id;
			goto FOUND_ENTRY;
		}
	}

	BNET_DEBUG(DBG_ERROR, "%s: No PDP context id for net devie [%p]\n",
		   __FUNCTION__, drvr_info_ptr->dev_ptr);

FOUND_ENTRY:
	spin_unlock_irqrestore(&g_dev_lock, flags);

	return pdp_cid;
}

/**
   @fn static net_drvr_info_t *bcm_fuse_net_device_cid_lookup(unsigned char cid);
*/
#ifdef INCLUDE_UNUSED_CODE
static net_drvr_info_t *bcm_fuse_net_device_cid_lookup(unsigned char cid)
{
	int i = 0;
	unsigned long flags;
	net_drvr_info_t *drvr_info_ptr = NULL;

	spin_lock_irqsave(&g_dev_lock, flags);

	for (i = 0; i < BCM_NET_MAX_PDP_CNTXS; i++) {
		if (g_net_dev_tbl[i].entry_stat == EFree)
			continue;

		if (g_net_dev_tbl[i].pdp_context_id == cid) {
			drvr_info_ptr = &g_net_dev_tbl[i];
			goto FOUND_ENTRY;
		}
	}

	BNET_DEBUG(DBG_ERROR, "%s: No network device mapping for cid[%d]\n",
		   __FUNCTION__, cid);

FOUND_ENTRY:
	spin_unlock_irqrestore(&g_dev_lock, flags);

	return drvr_info_ptr;
}
#endif /*#ifdef INCLUDE_UNUSED_CODE */

static const struct file_operations bcm_fuse_net_config_fops = {
	.read	=	bcm_fuse_net_proc_read,
	.write	=	bcm_fuse_net_proc_write,
};

#ifdef CONFIG_BCM_NET_WHITELIST_SUPPORT
static int ipc_peoperty_setup(void)
{
	IPC_PersistentDataStore_t ipc_persistent_data;

	/*get ipc persistent area for white list table access
	  in current paltform, the persistent data info does not change,
	  we can't do it in init_module as IPC may not be ready yet*/
	if (ipc_whitelist_enable_ptr == NULL) {
		IPC_GetPersistentData(&ipc_persistent_data);
		BNET_DEBUG(DBG_INFO,
			"IPC_GetPersistentData(), dataptr=0x%x, length=%d\n",
			(unsigned int)ipc_persistent_data.DataPtr,
			ipc_persistent_data.DataLength);
		ipc_whitelist_enable_ptr =
			(uint8_t *)ipc_persistent_data.DataPtr;
		ipc_whitelist_tbl = (uint16_t *)(ipc_whitelist_enable_ptr + 2);
		if (ipc_whitelist_enable_ptr == NULL)
			BNET_DEBUG(DBG_ERROR,
				"%s: NULL data ptr from IPC_GetPersistentData()\n",
				__func__);
		if (ipc_persistent_data.DataLength <
			(((MAX_WL_TCP_PORTS_CNT+MAX_WL_UDP_PORTS_CNT)*
			sizeof(uint16_t))+2)) {
			BNET_DEBUG(DBG_ERROR,
				"%s: not enough IPC memory, ipc persistent datalength = %d\n",
				__func__, ipc_persistent_data.DataLength);
			/*no need to carry on*/
			ipc_whitelist_tbl = NULL;
			return -1;
		}
	}

	return 0;
}

static void set_ipc_property_wl_flag(uint8_t enable)
{
	if (ipc_whitelist_enable_ptr == NULL) {
		if (ipc_peoperty_setup() != 0)
			return;
	}

	BNET_DEBUG(DBG_INFO, "%s: set ipc wl enable flag = %d\n",
		__func__, enable);
	*ipc_whitelist_enable_ptr = enable;
	return;
}

void bcm_net_add_or_remove_port(struct socket *sock, bool add)
{
	struct inet_sock *inet;
	__be32 src_addr;
	__u16 src_port;
	int i;
	unsigned long flags;

	if ((sock == NULL) || (sock->sk == NULL))
		return;

	inet = inet_sk(sock->sk);
	src_addr = inet->inet_rcv_saddr;
	src_port = ntohs(inet->inet_sport);

	BNET_DEBUG(DBG_TRACE,
		"%s: src_addr=%d, src_port=%d, add=%d\n",
		__func__, src_addr, src_port, add);

	if (sock->type != SOCK_STREAM) {
		BNET_DEBUG(DBG_TRACE,
		"%s: Only handles tcp for now", __func__);
		return;
	}

	if (sock->sk->sk_state != TCP_LISTEN) {
		BNET_DEBUG(DBG_TRACE,
			"%s: sk_state == %d, not TCP_LISTEN",
			__func__, sock->sk->sk_state);
		return;
	}

	/* ip is local host */
	if (src_addr == 0x0100007F) {
		BNET_DEBUG(DBG_TRACE, "%s: local host, ignore\n", __func__);
		return;
	}

	if (src_port == 0) {
		BNET_DEBUG(DBG_ERROR, "%s: port = 0, ignore\n", __func__);
		return;
	}

	/*get ipc persistent area for white list table access
	  in current paltform, the persistent data info does not change,
	  we can't do it in init_module as IPC may not be ready yet*/
	if (ipc_whitelist_enable_ptr == NULL) {
		if (ipc_peoperty_setup() != 0)
			return;
	}

	spin_lock_irqsave(&wl_lock, flags);
	if (add) {
		if (whitelist_info.tcp_list_full) {
			BNET_DEBUG(DBG_ERROR,
				"%s: tcp list full, ignore\n", __func__);
			goto WL_SPIN_UNLOCK;
		}

		for (i = 0; i <= whitelist_info.tcp_port_end_idx; i++) {
			BNET_DEBUG(DBG_TRACE, "%s: wl_tbl[%d]=%d\n",
				__func__, i, ipc_whitelist_tbl[i]);
			if (ipc_whitelist_tbl[i] == 0) {
				ipc_whitelist_tbl[i] = src_port;
				goto WL_SPIN_UNLOCK;
			}
		}

		if (i == MAX_WL_TCP_PORTS_CNT) {
			/*this is strange, why disable whitelist when list
			is full, should just ignore
			the new ones, but, it's a requirement....*/
			BNET_DEBUG(DBG_ERROR,
				"%s: tcp list full, disable whitelist\n",
				__func__);
			whitelist_info.tcp_list_full = true;
			if (whitelist_info.ui_enabled)
				set_ipc_property_wl_flag(0);
			goto WL_SPIN_UNLOCK;
		}

		ipc_whitelist_tbl[i] = src_port;
		BNET_DEBUG(DBG_TRACE,
			"%s: wl_tbl[%d]=%d\n", __func__,
			i, ipc_whitelist_tbl[i]);
		whitelist_info.tcp_port_end_idx = i;
	} else {
		for (i = 0; i <= whitelist_info.tcp_port_end_idx; i++) {
			BNET_DEBUG(DBG_TRACE,
				"%s: wl_tbl[%d]=%d\n",
				__func__, i, ipc_whitelist_tbl[i]);
			if (ipc_whitelist_tbl[i] == src_port) {
				ipc_whitelist_tbl[i] = 0;
				if (whitelist_info.tcp_list_full) {
					whitelist_info.tcp_list_full = false;
					if (whitelist_info.ui_enabled)
						set_ipc_property_wl_flag(1);
				}
				goto WL_SPIN_UNLOCK;
			}
		}

		BNET_DEBUG(DBG_ERROR, "%s: can't find this port\n", __func__);
	}

WL_SPIN_UNLOCK:
	spin_unlock_irqrestore(&wl_lock, flags);
	return;

}
EXPORT_SYMBOL(bcm_net_add_or_remove_port);


/**
 * Write function for bcm_fuse_net whitelist proc entry
   1 - enable
   0 - disable
 */
static ssize_t bcm_fuse_net_wl_proc_write(struct file *procFp,
				       const char __user *ubuff,
				       unsigned long len, void *data)
{
	char uStr[BCM_FUSE_NET_PROC_MAX_STR_LEN];
	int length = len;

	if (len > BCM_FUSE_NET_PROC_MAX_STR_LEN) {
		BNET_DEBUG(DBG_INFO, "%s: input string is too long!\n",
		__func__);
	} else if (copy_from_user(uStr, ubuff, len)) {
		BNET_DEBUG(DBG_INFO, "%s: Failed to copy user string!\n",
		__func__);
		length = 0;
	} else if (sscanf(uStr, "%u", &(whitelist_info.ui_enabled)) != 1) {
		BNET_DEBUG(DBG_INFO,
			"%s: Failed to get flag from input string!\n",
			__func__);
	} else {
		BNET_DEBUG(DBG_INFO, "%s: enable - %d\n",
			__func__, whitelist_info.ui_enabled);

		if (whitelist_info.ui_enabled &&
			(!whitelist_info.tcp_list_full))
			set_ipc_property_wl_flag(1);
		else
			set_ipc_property_wl_flag(0);
	}

	return (ssize_t) length;
}

/**
   Read function for bcm_fuse_net whiltlist proc entry
   1 - enable
   0 - disable
*/
static int bcm_fuse_net_wl_proc_read(char *ubuff, char **start, off_t off,
				  int count, int *eof, void *data)
{
	int len = 0;

	len = snprintf(ubuff, 10, "%d\n", whitelist_info.ui_enabled);
	return len;
}


#ifdef CONFIG_BCM_NET_WHITELIST_DEBUG_SUPPORT

/**
	Read function for bcm_fuse_net whitelist debug proc entry
*/
static int bcm_fuse_net_wl_debug_proc_read(
char *ubuff, char **start, off_t off,
int count, int *eof, void *data)
{
	int len = 0;
	int i;
	IPC_PersistentDataStore_t ipc_persistent_data;
	uint8_t *ptr = NULL;

	IPC_GetPersistentData(&ipc_persistent_data);
	ptr = (uint8_t *)ipc_persistent_data.DataPtr;
	if (ptr == NULL)
		BNET_DEBUG(DBG_ERROR,
			"%s: NULL data ptr from IPC_GetPersistentData()\n",
			__func__);

	for (i = 0; i < 258; i++)
		len += snprintf(ubuff + len, 50, "%2x ", ptr[i]);

	return len;
}
#endif	/*end of #ifdef CONFIG_BCM_NET_WHITELIST_DEBUG_SUPPORT*/

#endif   /*end of #ifdef CONFIG_BCM_NET_WHITELIST_SUPPORT*/


static int __init bcm_fuse_net_init_module(void)
{
	unsigned int i = 0;

	BNET_DEBUG(DBG_INFO, "%s: <<\n", __FUNCTION__);
	spin_lock_init(&g_dev_lock);

	for (i = 0; i < BCM_NET_MAX_PDP_CNTXS; i++)
		memset(&g_net_dev_tbl[i], 0, sizeof(net_drvr_info_t));

	for (i = 0; i < BCM_NET_MAX_PDP_CNTXS; i++) {
		if (bcm_fuse_net_attach(i) == -1) {
			/* no need to carry on, something is wrong already,
			 * hopefully the already attached drivers can be enough
			 * to use */
			break;
		}
	}

	/* proc entry for net config settings */
	bcm_fuse_net_config_proc_entry =
		proc_create_data("bcm_fuse_net_config", 0666, NULL,
				&bcm_fuse_net_config_fops, NULL);
	if (bcm_fuse_net_config_proc_entry == NULL) {
		BNET_DEBUG(DBG_INFO,
		   "%s: Couldn't create bcm_fuse_net_config_proc_entry!\n",
		   __FUNCTION__);
	} else {
		BNET_DEBUG(DBG_INFO,
		   "%s: bcm_fuse_net_config_proc_entry created\n",
		   __FUNCTION__);
	}

#ifdef CONFIG_BCM_NET_WHITELIST_SUPPORT
	ipc_whitelist_enable_ptr = NULL;
	ipc_whitelist_tbl = NULL;

	/* proc entry for net config settings */
	bcm_fuse_net_wl_proc_entry =
	    create_proc_entry(BCM_FUSE_NET_WL_PROC_NAME, 0666, NULL);
	if (bcm_fuse_net_wl_proc_entry == NULL) {
		BNET_DEBUG(DBG_INFO,
		"%s: Couldn't create bcm_fuse_net_wl_proc_entry!\n",
		__func__);
	} else {
		BNET_DEBUG(DBG_INFO,
		"%s: bcm_fuse_net_wl_proc_entry created\n",
		__func__);
		bcm_fuse_net_wl_proc_entry->write_proc =
					bcm_fuse_net_wl_proc_write;
		bcm_fuse_net_wl_proc_entry->read_proc =
					bcm_fuse_net_wl_proc_read;
	}

#ifdef CONFIG_BCM_NET_WHITELIST_DEBUG_SUPPORT
	/* proc entry for net config settings */
	bcm_fuse_net_wl_debug_proc_entry =
	    create_proc_entry(BCM_FUSE_NET_WL_DEBUG_PROC_NAME, 0666, NULL);
	if (bcm_fuse_net_wl_debug_proc_entry == NULL) {
		BNET_DEBUG(DBG_INFO,
			"%s: Couldn't create bcm_fuse_net_wl_debug_proc_entry!\n",
			__func__);
	} else {
		BNET_DEBUG(DBG_INFO,
			"%s: bcm_fuse_net_wl_debug_proc_entry created\n",
			__func__);
		bcm_fuse_net_wl_debug_proc_entry->read_proc =
				bcm_fuse_net_wl_debug_proc_read;
	}
#endif /*end of #ifdef CONFIG_BCM_NET_WHITELIST_DEBUG_SUPPORT*/
#endif /*end of #ifdef CONFIG_BCM_NET_WHITELIST_SUPPORT*/

	g_net_tx.wq = alloc_workqueue("tx-wq", WQ_FREEZABLE
		| WQ_NON_REENTRANT | WQ_HIGHPRI, 0);
	if (!g_net_tx.wq) {
		BNET_DEBUG(DBG_ERROR,
		"%s: Alloc tx workqueue failed!\n", __func__);
		return -ENOMEM;
	}
	INIT_WORK(&g_net_tx.work, tx_work);
	skb_queue_head_init(&g_net_tx.queue);

	return 0;
}

/**
   @fn static void __exit bcm_fuse_net_exit_module(void);
*/
static void __exit bcm_fuse_net_exit_module(void)
{
	unsigned int i = 0;

	for (i = 0; i < BCM_NET_MAX_PDP_CNTXS; i++)
		bcm_fuse_net_deattach(i);

	remove_proc_entry("bcm_fuse_net_sim", bcm_fuse_net_config_proc_entry);

#ifdef CONFIG_BCM_NET_WHITELIST_SUPPORT
	remove_proc_entry(BCM_FUSE_NET_WL_PROC_NAME,
				bcm_fuse_net_wl_proc_entry);
#ifdef CONFIG_BCM_NET_WHITELIST_DEBUG_SUPPORT
	remove_proc_entry(BCM_FUSE_NET_WL_DEBUG_PROC_NAME,
				bcm_fuse_net_wl_debug_proc_entry);
#endif
#endif

	skb_queue_purge(&g_net_tx.queue);
	flush_workqueue(g_net_tx.wq);
	destroy_workqueue(g_net_tx.wq);

	return;
}

module_init(bcm_fuse_net_init_module);
module_exit(bcm_fuse_net_exit_module);
