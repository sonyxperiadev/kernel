/*
 * u_ether.c -- Ethernet-over-USB link layer utilities for Gadget stack
 *
 * Copyright (C) 2003-2005,2008 David Brownell
 * Copyright (C) 2003-2004 Robert Schwebel, Benedikt Spranger
 * Copyright (C) 2008 Nokia Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/* #define VERBOSE_DEBUG */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/gfp.h>
#include <linux/device.h>
#include <linux/ctype.h>
#include <linux/etherdevice.h>
#include <linux/ethtool.h>
#include <linux/if_vlan.h>
#include <linux/if_arp.h>
#include <linux/msm_rmnet.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>

#include "u_ether.h"


/*
 * This component encapsulates the Ethernet link glue needed to provide
 * one (!) network link through the USB gadget stack, normally "usb0".
 *
 * The control and data models are handled by the function driver which
 * connects to this code; such as CDC Ethernet (ECM or EEM),
 * "CDC Subset", or RNDIS.  That includes all descriptor and endpoint
 * management.
 *
 * Link level addressing is handled by this component using module
 * parameters; if no such parameters are provided, random link level
 * addresses are used.  Each end of the link uses one address.  The
 * host end address is exported in various ways, and is often recorded
 * in configuration databases.
 *
 * The driver which assembles each configuration using such a link is
 * responsible for ensuring that each configuration includes at most one
 * instance of is network link.  (The network layer provides ways for
 * this single "physical" link to be used by multiple virtual links.)
 */

#define UETH__VERSION	"29-May-2008"

/* Experiments show that both Linux and Windows hosts allow up to 16k
 * frame sizes. Set the max MTU size to 15k+52 to prevent allocating 32k
 * blocks and still have efficient handling. */
#define GETHER_MAX_MTU_SIZE 15412
#define GETHER_MAX_ETH_FRAME_LEN (GETHER_MAX_MTU_SIZE + ETH_HLEN)

static struct workqueue_struct	*uether_wq;

/* Extra buffer size to allocate for tx */
#define EXTRA_ALLOCATION_SIZE_U_ETH	128

struct eth_dev {
	/* lock is held while accessing port_usb
	 */
	spinlock_t		lock;
	struct gether		*port_usb;

	struct net_device	*net;
	struct usb_gadget	*gadget;

	spinlock_t		req_lock;	/* guard {rx,tx}_reqs */
	struct list_head	tx_reqs, rx_reqs;
	unsigned int		tx_qlen;
/* Minimum number of TX USB request queued to UDC */
#define MAX_TX_REQ_WITH_NO_INT	5
	int			no_tx_req_used;
	int			tx_skb_hold_count;
	u32			tx_req_bufsize;

	struct sk_buff_head	rx_frames;

	unsigned		qmult;

	unsigned		header_len;
	unsigned int		ul_max_pkts_per_xfer;
	unsigned int		dl_max_pkts_per_xfer;
	struct sk_buff		*(*wrap)(struct gether *, struct sk_buff *skb);
	int			(*unwrap)(struct gether *,
						struct sk_buff *skb,
						struct sk_buff_head *list);

	struct work_struct	work;
	struct work_struct	rx_work;

	unsigned long		todo;
	unsigned long		flags;
	unsigned short		rx_needed_headroom;
#define	WORK_RX_MEMORY		0

	bool			zlp;
	bool			no_skb_reserve;
	u8			host_mac[ETH_ALEN];
	u8			dev_mac[ETH_ALEN];
	unsigned long		tx_throttle;
	unsigned long		rx_throttle;
	unsigned int		tx_pkts_rcvd;
	unsigned long		skb_expand_cnt;
	struct dentry		*uether_dent;
	struct dentry		*uether_dfile;
};

static void uether_debugfs_init(struct eth_dev *dev);
static void uether_debugfs_exit(struct eth_dev *dev);

/*-------------------------------------------------------------------------*/

#define RX_EXTRA	20	/* bytes guarding against rx overflows */

#define DEFAULT_QLEN	2	/* double buffering by default */

/*
 * Usually downlink rates are higher than uplink rates and it
 * deserve higher number of requests. For CAT-6 data rates of
 * 300Mbps (~30 packets per milli-sec) 40 usb request may not
 * be sufficient. At this rate and with interrupt moderation
 * of interconnect, data can be very bursty. tx_qmult is the
 * additional multipler on qmult.
 */
static unsigned int tx_qmult = 2;
module_param(tx_qmult, uint, 0644);
MODULE_PARM_DESC(tx_qmult, "Additional queue length multiplier for tx");

/* for dual-speed hardware, use deeper queues at high/super speed */
static inline int qlen(struct usb_gadget *gadget, unsigned qmult)
{
	if (gadget_is_dualspeed(gadget) && (gadget->speed == USB_SPEED_HIGH ||
					    gadget->speed >= USB_SPEED_SUPER))
		return qmult * DEFAULT_QLEN;
	else
		return DEFAULT_QLEN;
}

/*-------------------------------------------------------------------------*/
#define U_ETHER_RX_PENDING_TSHOLD 500

static unsigned int u_ether_rx_pending_thld = U_ETHER_RX_PENDING_TSHOLD;
module_param(u_ether_rx_pending_thld, uint, 0644);

/* REVISIT there must be a better way than having two sets
 * of debug calls ...
 */

#undef DBG
#undef VDBG
#undef ERROR
#undef INFO

#define xprintk(d, level, fmt, args...) \
	printk(level "%s: " fmt , (d)->net->name , ## args)

#ifdef DEBUG
#undef DEBUG
#define DBG(dev, fmt, args...) \
	xprintk(dev , KERN_DEBUG , fmt , ## args)
#else
#define DBG(dev, fmt, args...) \
	do { } while (0)
#endif /* DEBUG */

#ifdef VERBOSE_DEBUG
#define VDBG	DBG
#else
#define VDBG(dev, fmt, args...) \
	do { } while (0)
#endif /* DEBUG */

#define ERROR(dev, fmt, args...) \
	xprintk(dev , KERN_ERR , fmt , ## args)
#define INFO(dev, fmt, args...) \
	xprintk(dev , KERN_INFO , fmt , ## args)

/*-------------------------------------------------------------------------*/

/* NETWORK DRIVER HOOKUP (to the layer above this driver) */

static int ueth_change_mtu(struct net_device *net, int new_mtu)
{
	if (new_mtu <= ETH_HLEN || new_mtu > GETHER_MAX_ETH_FRAME_LEN)
		return -ERANGE;
	net->mtu = new_mtu;

	return 0;
}

static int ueth_change_mtu_ip(struct net_device *net, int new_mtu)
{
	struct eth_dev	*dev = netdev_priv(net);
	unsigned long	flags;
	int		status = 0;

	spin_lock_irqsave(&dev->lock, flags);
	if (new_mtu <= 0)
		status = -EINVAL;
	else
		net->mtu = new_mtu;

	DBG(dev, "[%s] MTU change: old=%d new=%d\n", net->name,
					net->mtu, new_mtu);
	spin_unlock_irqrestore(&dev->lock, flags);

	return status;
}

static void eth_get_drvinfo(struct net_device *net, struct ethtool_drvinfo *p)
{
	struct eth_dev *dev = netdev_priv(net);

	strlcpy(p->driver, "g_ether", sizeof(p->driver));
	strlcpy(p->version, UETH__VERSION, sizeof(p->version));
	strlcpy(p->fw_version, dev->gadget->name, sizeof(p->fw_version));
	strlcpy(p->bus_info, dev_name(&dev->gadget->dev), sizeof(p->bus_info));
}

/* REVISIT can also support:
 *   - WOL (by tracking suspends and issuing remote wakeup)
 *   - msglevel (implies updated messaging)
 *   - ... probably more ethtool ops
 */

static const struct ethtool_ops ops = {
	.get_drvinfo = eth_get_drvinfo,
	.get_link = ethtool_op_get_link,
};

static void defer_kevent(struct eth_dev *dev, int flag)
{
	if (test_and_set_bit(flag, &dev->todo))
		return;
	if (!schedule_work(&dev->work))
		ERROR(dev, "kevent %d may have been dropped\n", flag);
	else
		DBG(dev, "kevent %d scheduled\n", flag);
}

static void rx_complete(struct usb_ep *ep, struct usb_request *req);
static void tx_complete(struct usb_ep *ep, struct usb_request *req);

static int
rx_submit(struct eth_dev *dev, struct usb_request *req, gfp_t gfp_flags)
{
	struct sk_buff	*skb;
	int		retval = -ENOMEM;
	size_t		size = 0;
	struct usb_ep	*out;
	unsigned long	flags;

	spin_lock_irqsave(&dev->lock, flags);
	if (dev->port_usb)
		out = dev->port_usb->out_ep;
	else
		out = NULL;

	if (!out)
	{
		spin_unlock_irqrestore(&dev->lock, flags);
		return -ENOTCONN;
	}

	/* Padding up to RX_EXTRA handles minor disagreements with host.
	 * Normally we use the USB "terminate on short read" convention;
	 * so allow up to (N*maxpacket), since that memory is normally
	 * already allocated.  Some hardware doesn't deal well with short
	 * reads (e.g. DMA must be N*maxpacket), so for now don't trim a
	 * byte off the end (to force hardware errors on overflow).
	 *
	 * RNDIS uses internal framing, and explicitly allows senders to
	 * pad to end-of-packet.  That's potentially nice for speed, but
	 * means receivers can't recover lost synch on their own (because
	 * new packets don't only start after a short RX).
	 */
	size += sizeof(struct ethhdr) + dev->net->mtu + RX_EXTRA;
	size += dev->port_usb->header_len;

		size += out->maxpacket - 1;
		size -= size % out->maxpacket;

	if (dev->ul_max_pkts_per_xfer)
		size *= dev->ul_max_pkts_per_xfer;

	if (dev->port_usb->is_fixed)
		size = max_t(size_t, size, dev->port_usb->fixed_out_len);
	spin_unlock_irqrestore(&dev->lock, flags);

	DBG(dev, "%s: size: %zd\n", __func__, size);
	skb = alloc_skb(size, gfp_flags);
	if (skb == NULL) {
		DBG(dev, "no rx skb\n");
		goto enomem;
	}

	/* Some platforms perform better when IP packets are aligned,
	 * but on at least one, checksumming fails otherwise.  Note:
	 * RNDIS headers involve variable numbers of LE32 values.
	 */
	if (likely(!dev->no_skb_reserve))
		//skb_reserve(skb, NET_IP_ALIGN);
		skb_reserve(skb, 0);

	req->buf = skb->data;
	req->length = size;
	req->context = skb;

	retval = usb_ep_queue(out, req, gfp_flags);
	if (retval == -ENOMEM)
enomem:
		defer_kevent(dev, WORK_RX_MEMORY);
	if (retval) {
		DBG(dev, "rx submit --> %d\n", retval);
		if (skb)
			dev_kfree_skb_any(skb);
	}
	return retval;
}

static void rx_complete(struct usb_ep *ep, struct usb_request *req)
{
	struct sk_buff	*skb = req->context;
	struct eth_dev	*dev = ep->driver_data;
	int		status = req->status;
	bool		queue = 0;

	switch (status) {

	/* normal completion */
	case 0:
		skb_put(skb, req->actual);

		if (dev->unwrap) {
			unsigned long	flags;

			spin_lock_irqsave(&dev->lock, flags);
			if (dev->port_usb) {
				status = dev->unwrap(dev->port_usb,
							skb,
							&dev->rx_frames);
				if (status == -EINVAL)
					dev->net->stats.rx_errors++;
				else if (status == -EOVERFLOW)
					dev->net->stats.rx_over_errors++;
			} else {
				dev_kfree_skb_any(skb);
				status = -ENOTCONN;
			}
			spin_unlock_irqrestore(&dev->lock, flags);
		} else {
			skb_queue_tail(&dev->rx_frames, skb);
		}

		if (!status)
			queue = 1;
		break;

	/* software-driven interface shutdown */
	case -ECONNRESET:		/* unlink */
	case -ESHUTDOWN:		/* disconnect etc */
		VDBG(dev, "rx shutdown, code %d\n", status);
		goto quiesce;

	/* for hardware automagic (such as pxa) */
	case -ECONNABORTED:		/* endpoint reset */
		DBG(dev, "rx %s reset\n", ep->name);
		defer_kevent(dev, WORK_RX_MEMORY);
quiesce:
		dev_kfree_skb_any(skb);
		goto clean;

	/* data overrun */
	case -EOVERFLOW:
		dev->net->stats.rx_over_errors++;
		/* FALLTHROUGH */

	default:
		queue = 1;
		dev_kfree_skb_any(skb);
		dev->net->stats.rx_errors++;
		DBG(dev, "rx status %d\n", status);
		break;
	}

clean:
	if (queue && dev->rx_frames.qlen <= u_ether_rx_pending_thld) {
		if (rx_submit(dev, req, GFP_ATOMIC) < 0) {
			spin_lock(&dev->req_lock);
			list_add(&req->list, &dev->rx_reqs);
			spin_unlock(&dev->req_lock);
		}
	} else {
		/* rx buffers draining is delayed,defer further queuing to wq */
		if (queue)
			dev->rx_throttle++;
		spin_lock(&dev->req_lock);
		list_add(&req->list, &dev->rx_reqs);
		spin_unlock(&dev->req_lock);
		req = NULL;
	}

	if (queue)
		queue_work(uether_wq, &dev->rx_work);
}

static int prealloc(struct list_head *list, struct usb_ep *ep, unsigned n)
{
	unsigned		i;
	struct usb_request	*req;
	bool			usb_in;

	if (!n)
		return -ENOMEM;

	/* queue/recycle up to N requests */
	i = n;
	list_for_each_entry(req, list, list) {
		if (i-- == 0)
			goto extra;
	}
	if (ep->desc->bEndpointAddress & USB_DIR_IN)
		usb_in = true;
	else
		usb_in = false;
	while (i--) {
		req = usb_ep_alloc_request(ep, GFP_ATOMIC);
		if (!req)
			return list_empty(list) ? -ENOMEM : 0;
				/* update completion handler */
		if (usb_in)
			req->complete = tx_complete;
		else
			req->complete = rx_complete;
		list_add(&req->list, list);
	}
	return 0;

extra:
	/* free extras */
	for (;;) {
		struct list_head	*next;

		next = req->list.next;
		list_del(&req->list);
		usb_ep_free_request(ep, req);

		if (next == list)
			break;

		req = container_of(next, struct usb_request, list);
	}
	return 0;
}

static int alloc_requests(struct eth_dev *dev, struct gether *link, unsigned n)
{
		int	status = 0;

	spin_lock(&dev->req_lock);
	if (link->in_ep) {
		status = prealloc(&dev->tx_reqs, link->in_ep, n * tx_qmult);
		if (status < 0)
			goto fail;
	}

	if (link->out_ep) {
		status = prealloc(&dev->rx_reqs, link->out_ep, n);

		if (status < 0)
			goto fail;
	}
	goto done;
fail:
	DBG(dev, "can't alloc requests\n");
done:
	spin_unlock(&dev->req_lock);
	return status;
}

static void rx_fill(struct eth_dev *dev, gfp_t gfp_flags)
{
	struct usb_request	*req;
	unsigned long		flags;
	int			req_cnt = 0;

	/* fill unused rxq slots with some skb */
	spin_lock_irqsave(&dev->req_lock, flags);
	while (!list_empty(&dev->rx_reqs)) {
	/* break the nexus of continuous completion and re-submission*/
		if (++req_cnt > qlen(dev->gadget, dev->qmult))
			break;

		req = container_of(dev->rx_reqs.next,
				struct usb_request, list);
		list_del_init(&req->list);
		spin_unlock_irqrestore(&dev->req_lock, flags);

		if (rx_submit(dev, req, gfp_flags) < 0) {
			spin_lock_irqsave(&dev->req_lock, flags);
			list_add(&req->list, &dev->rx_reqs);
			spin_unlock_irqrestore(&dev->req_lock, flags);
			defer_kevent(dev, WORK_RX_MEMORY);
			return;
		}

		spin_lock_irqsave(&dev->req_lock, flags);
	}
	spin_unlock_irqrestore(&dev->req_lock, flags);
}

static __be16 ether_ip_type_trans(struct sk_buff *skb,
	struct net_device *dev)
{
	__be16	protocol = 0;

	skb->dev = dev;

	switch (skb->data[0] & 0xf0) {
	case 0x40:
		protocol = htons(ETH_P_IP);
		break;
	case 0x60:
		protocol = htons(ETH_P_IPV6);
		break;
	default:
		if ((skb->data[0] & 0x40) == 0x00)
			protocol = htons(ETH_P_MAP);
		else
			pr_debug_ratelimited("[%s] L3 protocol decode error: 0x%02x",
					dev->name, skb->data[0] & 0xf0);
	}

	return protocol;
}

static void process_rx_w(struct work_struct *work)
{
	struct eth_dev	*dev = container_of(work, struct eth_dev, rx_work);
	struct sk_buff	*skb;
	int		status = 0;

	if (!dev->port_usb)
		return;

	while ((skb = skb_dequeue(&dev->rx_frames))) {
		if (status < 0
				|| ETH_HLEN > skb->len
				|| skb->len > ETH_FRAME_LEN) {
			dev->net->stats.rx_errors++;
			dev->net->stats.rx_length_errors++;
			DBG(dev, "rx length %d\n", skb->len);
			dev_kfree_skb_any(skb);
			continue;
		}

		if (test_bit(RMNET_MODE_LLP_IP, &dev->flags))
			skb->protocol = ether_ip_type_trans(skb, dev->net);
		else
			skb->protocol = eth_type_trans(skb, dev->net);

		dev->net->stats.rx_packets++;
		dev->net->stats.rx_bytes += skb->len;

		status = netif_rx_ni(skb);
	}

	if (netif_running(dev->net))
		rx_fill(dev, GFP_KERNEL);
}

static void eth_work(struct work_struct *work)
{
	struct eth_dev	*dev = container_of(work, struct eth_dev, work);

	if (test_and_clear_bit(WORK_RX_MEMORY, &dev->todo)) {
		if (netif_running(dev->net))
			rx_fill(dev, GFP_KERNEL);
	}

	if (dev->todo)
		DBG(dev, "work done, flags = 0x%lx\n", dev->todo);
}

static void tx_complete(struct usb_ep *ep, struct usb_request *req)
{
	struct sk_buff	*skb;
	struct eth_dev	*dev = ep->driver_data;
	struct net_device *net = dev->net;
	struct usb_request *new_req;
	struct usb_ep *in;
	int length;
	int retval;

	if (!dev->port_usb) {
		usb_ep_free_request(ep, req);
		return;
	}
	switch (req->status) {
	default:
		dev->net->stats.tx_errors++;
		VDBG(dev, "tx err %d\n", req->status);
		/* FALLTHROUGH */
	case -ECONNRESET:		/* unlink */
	case -ESHUTDOWN:		/* disconnect etc */
		break;
	case 0:
		if (!req->zero)
			dev->net->stats.tx_bytes += req->actual-1;
		else
			dev->net->stats.tx_bytes += req->actual;
	}
	dev->net->stats.tx_packets++;

	spin_lock(&dev->req_lock);

	if (dev->port_usb->multi_pkt_xfer && !req->context) {
		dev->no_tx_req_used--;
		req->length = 0;
		in = dev->port_usb->in_ep;

		/* Do not process further if no_interrupt is set */
		if (!req->no_interrupt && !list_empty(&dev->tx_reqs)) {
			new_req = container_of(dev->tx_reqs.next,
					struct usb_request, list);
			list_del(&new_req->list);
			spin_unlock(&dev->req_lock);
			if (new_req->length > 0) {
				length = new_req->length;

		/* NCM requires no zlp if transfer is dwNtbInMaxSize */
				if (dev->port_usb->is_fixed &&
					length == dev->port_usb->fixed_in_len &&
					(length % in->maxpacket) == 0)
					new_req->zero = 0;
				else
					new_req->zero = 1;

				/* use zlp framing on tx for strict CDC-Ether
				 * conformance, though any robust network rx
				 * path ignores extra padding. and some hardware
				 * doesn't like to write zlps.
				 */
				if (new_req->zero && !dev->zlp &&
						(length % in->maxpacket) == 0) {
					new_req->zero = 0;
					length++;
				}

				/* set when tx completion interrupt needed */
				spin_lock(&dev->req_lock);
				dev->tx_qlen++;
				if (dev->tx_qlen == MAX_TX_REQ_WITH_NO_INT) {
					new_req->no_interrupt = 0;
					dev->tx_qlen = 0;
				} else {
					new_req->no_interrupt = 1;
				}
				spin_unlock(&dev->req_lock);
				new_req->length = length;
				new_req->complete = tx_complete;
				retval = usb_ep_queue(in, new_req, GFP_ATOMIC);
				switch (retval) {
				default:
					DBG(dev, "tx queue err %d\n", retval);
					new_req->length = 0;
					spin_lock(&dev->req_lock);
					list_add_tail(&new_req->list,
							&dev->tx_reqs);
					spin_unlock(&dev->req_lock);
					break;
				case 0:
					spin_lock(&dev->req_lock);
					dev->no_tx_req_used++;
					spin_unlock(&dev->req_lock);
					netif_trans_update(net);
				}
			} else {
				spin_lock(&dev->req_lock);
				/*
				 * Put the idle request at the back of the
				 * queue. The xmit function will put the
				 * unfinished request at the beginning of the
				 * queue.
				 */
				list_add_tail(&new_req->list, &dev->tx_reqs);
				spin_unlock(&dev->req_lock);
			}
		} else {
			spin_unlock(&dev->req_lock);
		}
	} else {
		skb = req->context;
		/* Is aggregation already enabled and buffers allocated ? */
		if (dev->port_usb->multi_pkt_xfer && dev->tx_req_bufsize) {
			req->buf = kzalloc(dev->tx_req_bufsize
				+ dev->gadget->extra_buf_alloc, GFP_ATOMIC);
			req->context = NULL;
		} else {
			req->buf = NULL;
		}

		spin_unlock(&dev->req_lock);
		dev_kfree_skb_any(skb);
	}

	/* put the completed req back to tx_reqs tail pool */
	spin_lock(&dev->req_lock);
	list_add_tail(&req->list, &dev->tx_reqs);
	spin_unlock(&dev->req_lock);

	if (netif_carrier_ok(dev->net))
		netif_wake_queue(dev->net);
}

static inline int is_promisc(u16 cdc_filter)
{
	return cdc_filter & USB_CDC_PACKET_TYPE_PROMISCUOUS;
}

static int alloc_tx_buffer(struct eth_dev *dev)
{
	struct list_head	*act;
	struct usb_request	*req;

	dev->tx_req_bufsize = (dev->dl_max_pkts_per_xfer *
				(dev->net->mtu
				+ sizeof(struct ethhdr)
				/* size of rndis_packet_msg_type */
				+ 44
				+ 22));

	list_for_each(act, &dev->tx_reqs) {
		req = container_of(act, struct usb_request, list);
		if (!req->buf)
			req->buf = kzalloc(dev->tx_req_bufsize
				+ dev->gadget->extra_buf_alloc, GFP_ATOMIC);

		if (!req->buf)
			goto free_buf;

		/* req->context is not used for multi_pkt_xfers */
		req->context = NULL;
	}
	return 0;

free_buf:
	/* tx_req_bufsize = 0 retries mem alloc on next eth_start_xmit */
	dev->tx_req_bufsize = 0;
	list_for_each(act, &dev->tx_reqs) {
		req = container_of(act, struct usb_request, list);
		kfree(req->buf);
		req->buf = NULL;
	}
	return -ENOMEM;
}

static netdev_tx_t eth_start_xmit(struct sk_buff *skb,
					struct net_device *net)
{
	struct eth_dev		*dev = netdev_priv(net);
	int			length = 0;
	int			tail_room = 0;
	int			extra_alloc = 0;
	int			retval;
	struct usb_request	*req = NULL;
	unsigned long		flags;
	struct sk_buff		*new_skb;
	struct usb_ep		*in = NULL;
	u16			cdc_filter = 0;
	bool			multi_pkt_xfer = false;
	u32			fixed_in_len = 0;
	bool			is_fixed = false;



	spin_lock_irqsave(&dev->lock, flags);
	if (dev->port_usb) {
		in = dev->port_usb->in_ep;
		cdc_filter = dev->port_usb->cdc_filter;
		is_fixed = dev->port_usb->is_fixed;
		fixed_in_len = dev->port_usb->fixed_in_len;
		multi_pkt_xfer = dev->port_usb->multi_pkt_xfer;
	}
	spin_unlock_irqrestore(&dev->lock, flags);

	if (!in) {
		if (skb)
			dev_kfree_skb_any(skb);
		return NETDEV_TX_OK;
	}

	/* apply outgoing CDC or RNDIS filters */
	if (!test_bit(RMNET_MODE_LLP_IP, &dev->flags) &&
			!is_promisc(cdc_filter)) {
		u8		*dest = skb->data;

		if (is_multicast_ether_addr(dest)) {
			u16	type;

			/* ignores USB_CDC_PACKET_TYPE_MULTICAST and host
			 * SET_ETHERNET_MULTICAST_FILTERS requests
			 */
			if (is_broadcast_ether_addr(dest))
				type = USB_CDC_PACKET_TYPE_BROADCAST;
			else
				type = USB_CDC_PACKET_TYPE_ALL_MULTICAST;
			if (!(cdc_filter & type)) {
				dev->net->stats.tx_dropped++;
				dev_kfree_skb_any(skb);
				return NETDEV_TX_OK;
			}
		}
		/* ignores USB_CDC_PACKET_TYPE_DIRECTED */
	}

	dev->tx_pkts_rcvd++;

	/* Allocate memory for tx_reqs to support multi packet transfer */
	spin_lock_irqsave(&dev->req_lock, flags);
	if (multi_pkt_xfer && !dev->tx_req_bufsize) {
		retval = alloc_tx_buffer(dev);
		if (retval < 0) {
			spin_unlock_irqrestore(&dev->req_lock, flags);
			return -ENOMEM;
		}
	}

	/*
	 * this freelist can be empty if an interrupt triggered disconnect()
	 * and reconfigured the gadget (shutting down this queue) after the
	 * network stack decided to xmit but before we got the spinlock.
	 */
	if (list_empty(&dev->tx_reqs)) {
		spin_unlock_irqrestore(&dev->req_lock, flags);
		return NETDEV_TX_BUSY;
	}

	req = container_of(dev->tx_reqs.next, struct usb_request, list);
	list_del(&req->list);

	/* temporarily stop TX queue when the freelist empties */
	if (list_empty(&dev->tx_reqs)) {
		/*
		 * tx_throttle gives info about number of times u_ether
		 * asked network layer to stop queueing packets to it
		 * when transmit resources are unavailable
		 */
		dev->tx_throttle++;
	netif_stop_queue(net);
	}
	spin_unlock_irqrestore(&dev->req_lock, flags);

	/* no buffer copies needed, unless the network stack did it
	 * or the hardware can't use skb buffers.
	 * or there's not enough space for extra headers we need
	 */
	spin_lock_irqsave(&dev->lock, flags);
	if (dev->wrap) {
		if (dev->port_usb)
			skb = dev->wrap(dev->port_usb, skb);
		if (!skb) {
			spin_unlock_irqrestore(&dev->lock, flags);
			/* Multi frame CDC protocols may store the frame for
			 * later which is not a dropped frame.
			 */
			if (dev->port_usb &&
				dev->port_usb->supports_multi_frame)
				goto multiframe;
			goto drop;
		}
	}

	dev->tx_skb_hold_count++;
	spin_unlock_irqrestore(&dev->lock, flags);

	if (multi_pkt_xfer) {
		pr_debug("req->length:%d header_len:%u\n"
				"skb->len:%d skb->data_len:%d\n",
				req->length, dev->header_len,
				skb->len, skb->data_len);
		/* Add RNDIS Header */
		memcpy(req->buf + req->length, dev->port_usb->header,
						dev->header_len);
		/* Increment req length by header size */
		req->length += dev->header_len;
		/* Copy received IP data from SKB */
		memcpy(req->buf + req->length, skb->data, skb->len);
		/* Increment req length by skb data length */
		req->length += skb->len;
		length = req->length;
		dev_kfree_skb_any(skb);

		spin_lock_irqsave(&dev->req_lock, flags);
		if (dev->tx_skb_hold_count < dev->dl_max_pkts_per_xfer) {
			/*
			 * should allow aggregation only, if the number of
			 * requests queued more than the tx requests that can
			 * be queued with no interrupt flag set sequentially.
			 * Otherwise, packets may be blocked forever.
			 */
			if (dev->no_tx_req_used > MAX_TX_REQ_WITH_NO_INT) {
				list_add(&req->list, &dev->tx_reqs);
				spin_unlock_irqrestore(&dev->req_lock, flags);
				goto success;
			}
		}

		dev->no_tx_req_used++;
		spin_unlock_irqrestore(&dev->req_lock, flags);

		spin_lock_irqsave(&dev->lock, flags);
		dev->tx_skb_hold_count = 0;
		spin_unlock_irqrestore(&dev->lock, flags);
	} else {
		bool do_align = false;

		/* Check if TX buffer should be aligned before queuing to hw */
		if (dev->gadget->is_chipidea &&
		    !IS_ALIGNED((size_t)skb->data, 4))
			do_align = true;

		/*
		 * Some UDC requires allocation of some extra bytes for
		 * TX buffer due to hardware requirement. Check if extra
		 * bytes are already there, otherwise allocate new buffer
		 * with extra bytes and do memcpy to align skb as well.
		 */
		if (dev->gadget->extra_buf_alloc)
			extra_alloc = EXTRA_ALLOCATION_SIZE_U_ETH;
		tail_room = skb_tailroom(skb);
		if (do_align || tail_room < extra_alloc) {
			pr_debug("%s:align skb and update tail_room %d to %d\n",
					__func__, tail_room, extra_alloc);
			tail_room = extra_alloc;
			new_skb = skb_copy_expand(skb, 0, tail_room,
						  GFP_ATOMIC);
			if (!new_skb)
				return -ENOMEM;
			dev_kfree_skb_any(skb);
			skb = new_skb;
			dev->skb_expand_cnt++;
		}

		length = skb->len;
		req->buf = skb->data;
		req->context = skb;
	}
	/* NCM requires no zlp if transfer is dwNtbInMaxSize */
	if (is_fixed && length == fixed_in_len &&
		(length % in->maxpacket) == 0)
		req->zero = 0;
	else
		req->zero = 1;

	/* use zlp framing on tx for strict CDC-Ether conformance,
	 * though any robust network rx path ignores extra padding.
	 * and some hardware doesn't like to write zlps.
	 */
	if (req->zero && !dev->zlp && (length % in->maxpacket) == 0) {
		req->zero = 0;
		length++;
	}

	req->length = length;
	/* throttle highspeed IRQ rate back slightly */
	if (gadget_is_dualspeed(dev->gadget) &&
			 (dev->gadget->speed == USB_SPEED_HIGH)) {
		spin_lock_irqsave(&dev->req_lock, flags);
		dev->tx_qlen++;
		if (dev->tx_qlen == MAX_TX_REQ_WITH_NO_INT) {
			req->no_interrupt = 0;
			dev->tx_qlen = 0;
		} else {
			req->no_interrupt = 1;
		}
		spin_unlock_irqrestore(&dev->req_lock, flags);
	} else {
		req->no_interrupt = 0;
	}
	retval = usb_ep_queue(in, req, GFP_ATOMIC);
	switch (retval) {
	default:
		DBG(dev, "tx queue err %d\n", retval);
		break;
	case 0:
		netif_trans_update(net);
	}

	if (retval) {

		if (!multi_pkt_xfer)

			dev_kfree_skb_any(skb);
		else
			req->length = 0;
drop:
		dev->net->stats.tx_dropped++;
multiframe:
		spin_lock_irqsave(&dev->req_lock, flags);
		if (list_empty(&dev->tx_reqs))
			netif_start_queue(net);
		list_add_tail(&req->list, &dev->tx_reqs);
		spin_unlock_irqrestore(&dev->req_lock, flags);
	}
success:
	return NETDEV_TX_OK;
}

/*-------------------------------------------------------------------------*/

static void eth_start(struct eth_dev *dev, gfp_t gfp_flags)
{
	DBG(dev, "%s\n", __func__);

	/* fill the rx queue */
	rx_fill(dev, gfp_flags);

	/* and open the tx floodgates */
	dev->tx_qlen = 0;
	netif_wake_queue(dev->net);
}

static int eth_open(struct net_device *net)
{
	struct eth_dev	*dev = netdev_priv(net);
	struct gether	*link;

	DBG(dev, "%s\n", __func__);
	if (netif_carrier_ok(dev->net))
		eth_start(dev, GFP_KERNEL);

	spin_lock_irq(&dev->lock);
	link = dev->port_usb;
	if (link && link->open)
		link->open(link);
	spin_unlock_irq(&dev->lock);

	return 0;
}

static int eth_stop(struct net_device *net)
{
	struct eth_dev	*dev = netdev_priv(net);
	unsigned long	flags;

	VDBG(dev, "%s\n", __func__);
	netif_stop_queue(net);

	DBG(dev, "stop stats: rx/tx %ld/%ld, errs %ld/%ld\n",
		dev->net->stats.rx_packets, dev->net->stats.tx_packets,
		dev->net->stats.rx_errors, dev->net->stats.tx_errors
		);

	/* ensure there are no more active requests */
	spin_lock_irqsave(&dev->lock, flags);
	if (dev->port_usb) {
		struct gether	*link = dev->port_usb;
		const struct usb_endpoint_descriptor *in;
		const struct usb_endpoint_descriptor *out;

		if (link->close)
			link->close(link);

		/* NOTE:  we have no abort-queue primitive we could use
		 * to cancel all pending I/O.  Instead, we disable then
		 * reenable the endpoints ... this idiom may leave toggle
		 * wrong, but that's a self-correcting error.
		 *
		 * REVISIT:  we *COULD* just let the transfers complete at
		 * their own pace; the network stack can handle old packets.
		 * For the moment we leave this here, since it works.
		 */
		if (link->in_ep) {
			in = link->in_ep->desc;
			usb_ep_disable(link->in_ep);
			if (netif_carrier_ok(net)) {
				DBG(dev, "host still using in endpoints\n");
				link->in_ep->desc = in;
				usb_ep_enable(link->in_ep);
			}
		}

		if (link->out_ep) {
			out = link->out_ep->desc;
			usb_ep_disable(link->out_ep);
			if (netif_carrier_ok(net)) {
				DBG(dev, "host still using out endpoints\n");
				link->out_ep->desc = out;
				usb_ep_enable(link->out_ep);
			}
		}
	}
	spin_unlock_irqrestore(&dev->lock, flags);

	return 0;
}

/*-------------------------------------------------------------------------*/

static int get_ether_addr(const char *str, u8 *dev_addr)
{
	if (str) {
		unsigned	i;

		for (i = 0; i < 6; i++) {
			unsigned char num;

			if ((*str == '.') || (*str == ':'))
				str++;
			num = hex_to_bin(*str++) << 4;
			num |= hex_to_bin(*str++);
			dev_addr [i] = num;
		}
		if (is_valid_ether_addr(dev_addr))
			return 0;
	}
	eth_random_addr(dev_addr);
	return 1;
}

static int get_ether_addr_str(u8 dev_addr[ETH_ALEN], char *str, int len)
{
	if (len < 18)
		return -EINVAL;

	snprintf(str, len, "%pM", dev_addr);
	return 18;
}

static int ether_ioctl(struct net_device *, struct ifreq *, int);

static const struct net_device_ops eth_netdev_ops = {
	.ndo_open		= eth_open,
	.ndo_stop		= eth_stop,
	.ndo_start_xmit		= eth_start_xmit,
	.ndo_do_ioctl		= ether_ioctl,
	.ndo_change_mtu		= ueth_change_mtu,
	.ndo_set_mac_address 	= eth_mac_addr,
	.ndo_validate_addr	= eth_validate_addr,
};

static const struct net_device_ops eth_netdev_ops_ip = {
	.ndo_open		= eth_open,
	.ndo_stop		= eth_stop,
	.ndo_start_xmit		= eth_start_xmit,
	.ndo_do_ioctl		= ether_ioctl,
	.ndo_change_mtu		= ueth_change_mtu_ip,
	.ndo_set_mac_address	= NULL,
	.ndo_validate_addr	= NULL,
};

static int rmnet_ioctl_extended(struct net_device *dev, struct ifreq *ifr)
{
	struct rmnet_ioctl_extended_s ext_cmd;
	struct eth_dev *eth_dev = netdev_priv(dev);
	int rc = 0;

	rc = copy_from_user(&ext_cmd, ifr->ifr_ifru.ifru_data,
			    sizeof(struct rmnet_ioctl_extended_s));

	if (rc) {
		DBG(eth_dev, "%s(): copy_from_user() failed\n", __func__);
		return rc;
	}

	switch (ext_cmd.extended_ioctl) {
	case RMNET_IOCTL_GET_SUPPORTED_FEATURES:
		ext_cmd.u.data = 0;
		break;

	case RMNET_IOCTL_SET_MRU:
		if (netif_running(dev))
			return -EBUSY;

		/* 16K max */
		if ((size_t)ext_cmd.u.data > 0x4000)
			return -EINVAL;

		if (eth_dev->port_usb) {
			eth_dev->port_usb->is_fixed = true;
			eth_dev->port_usb->fixed_out_len =
				(size_t) ext_cmd.u.data;
			DBG(eth_dev, "[%s] rmnet_ioctl(): SET MRU to %u\n",
				dev->name, eth_dev->port_usb->fixed_out_len);
		} else {
			pr_err("[%s]: %s: SET MRU failed. Cable disconnected\n",
				dev->name, __func__);
			return -ENODEV;
		}
		break;

	case RMNET_IOCTL_GET_MRU:
		if (eth_dev->port_usb) {
			ext_cmd.u.data = eth_dev->port_usb->is_fixed ?
					eth_dev->port_usb->fixed_out_len :
					dev->mtu;
		} else {
			pr_err("[%s]: %s: GET MRU failed. Cable disconnected\n",
				dev->name, __func__);
			return -ENODEV;
		}
		break;

	case RMNET_IOCTL_GET_DRIVER_NAME:
		strlcpy(ext_cmd.u.if_name, dev->name,
			sizeof(ext_cmd.u.if_name));
		break;

	default:
		break;
	}

	rc = copy_to_user(ifr->ifr_ifru.ifru_data, &ext_cmd,
			  sizeof(struct rmnet_ioctl_extended_s));

	if (rc)
		DBG(eth_dev, "%s(): copy_to_user() failed\n", __func__);
	return rc;
}

static int ether_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
	struct eth_dev	*eth_dev = netdev_priv(dev);
	void __user *addr = (void __user *) ifr->ifr_ifru.ifru_data;
	int		prev_mtu = dev->mtu;
	u32		state, old_opmode;
	int		rc = -EFAULT;

	old_opmode = eth_dev->flags;
	/* Process IOCTL command */
	switch (cmd) {
	case RMNET_IOCTL_SET_LLP_ETHERNET:	/*Set Ethernet protocol*/
		/* Perform Ethernet config only if in IP mode currently*/
		if (test_bit(RMNET_MODE_LLP_IP, &eth_dev->flags)) {
			ether_setup(dev);
			dev->mtu = prev_mtu;
			dev->netdev_ops = &eth_netdev_ops;
			clear_bit(RMNET_MODE_LLP_IP, &eth_dev->flags);
			set_bit(RMNET_MODE_LLP_ETH, &eth_dev->flags);
			DBG(eth_dev, "[%s] ioctl(): set Ethernet proto mode\n",
					dev->name);
		}
		if (test_bit(RMNET_MODE_LLP_ETH, &eth_dev->flags))
			rc = 0;
		break;

	case RMNET_IOCTL_SET_LLP_IP:		/* Set RAWIP protocol*/
		/* Perform IP config only if in Ethernet mode currently*/
		if (test_bit(RMNET_MODE_LLP_ETH, &eth_dev->flags)) {
			/* Undo config done in ether_setup() */
			dev->header_ops = NULL;  /* No header */
			dev->type = ARPHRD_RAWIP;
			dev->hard_header_len = 0;
			dev->mtu = prev_mtu;
			dev->addr_len = 0;
			dev->flags &= ~(IFF_BROADCAST | IFF_MULTICAST);
			dev->netdev_ops = &eth_netdev_ops_ip;
			clear_bit(RMNET_MODE_LLP_ETH, &eth_dev->flags);
			set_bit(RMNET_MODE_LLP_IP, &eth_dev->flags);
			DBG(eth_dev, "[%s] ioctl(): set IP protocol mode\n",
					dev->name);
		}
		if (test_bit(RMNET_MODE_LLP_IP, &eth_dev->flags))
			rc = 0;
		break;

	case RMNET_IOCTL_GET_LLP:	/* Get link protocol state */
		state = eth_dev->flags & (RMNET_MODE_LLP_ETH
						| RMNET_MODE_LLP_IP);
		if (copy_to_user(addr, &state, sizeof(state)))
			break;
		rc = 0;
		break;

	case RMNET_IOCTL_SET_RX_HEADROOM:	/* Set RX headroom */
		if (copy_from_user(&eth_dev->rx_needed_headroom, addr,
					sizeof(eth_dev->rx_needed_headroom)))
			break;
		DBG(eth_dev, "[%s] ioctl(): set RX HEADROOM: %x\n",
				dev->name, eth_dev->rx_needed_headroom);
		rc = 0;
		break;

	case RMNET_IOCTL_EXTENDED:
		rc = rmnet_ioctl_extended(dev, ifr);
		break;

	default:
		pr_err("[%s] error: ioctl called for unsupported cmd[%d]",
			dev->name, cmd);
		rc = -EINVAL;
	}

	DBG(eth_dev, "[%s] %s: cmd=0x%x opmode old=0x%08x new=0x%08lx\n",
		dev->name, __func__, cmd, old_opmode, eth_dev->flags);

	return rc;
}


static struct device_type gadget_type = {
	.name	= "gadget",
};

/**
 * gether_setup_name - initialize one ethernet-over-usb link
 * @g: gadget to associated with these links
 * @ethaddr: NULL, or a buffer in which the ethernet address of the
 *	host side of the link is recorded
 * @netname: name for network device (for example, "usb")
 * Context: may sleep
 *
 * This sets up the single network link that may be exported by a
 * gadget driver using this framework.  The link layer addresses are
 * set up using module parameters.
 *
 * Returns an eth_dev pointer on success, or an ERR_PTR on failure.
 */
struct eth_dev *gether_setup_name(struct usb_gadget *g,
		const char *dev_addr, const char *host_addr,
		u8 ethaddr[ETH_ALEN], unsigned qmult, const char *netname)
{
	struct eth_dev		*dev;
	struct net_device	*net;
	int			status;

	net = alloc_etherdev(sizeof *dev);
	if (!net)
		return ERR_PTR(-ENOMEM);

	dev = netdev_priv(net);
	spin_lock_init(&dev->lock);
	spin_lock_init(&dev->req_lock);
	INIT_WORK(&dev->work, eth_work);
	INIT_WORK(&dev->rx_work, process_rx_w);
	INIT_LIST_HEAD(&dev->tx_reqs);
	INIT_LIST_HEAD(&dev->rx_reqs);

	skb_queue_head_init(&dev->rx_frames);

	/* network device setup */
	dev->net = net;
	dev->qmult = qmult;
	snprintf(net->name, sizeof(net->name), "%s%%d", netname);

	if (get_ether_addr(dev_addr, net->dev_addr))
		dev_warn(&g->dev,
			"using random %s ethernet address\n", "self");
	if (get_ether_addr(host_addr, dev->host_mac))
		dev_warn(&g->dev,
			"using random %s ethernet address\n", "host");

	if (ethaddr)
		memcpy(ethaddr, dev->host_mac, ETH_ALEN);

	net->netdev_ops = &eth_netdev_ops;

	net->ethtool_ops = &ops;

	/* set operation mode to eth by default */
	set_bit(RMNET_MODE_LLP_ETH, &dev->flags);

	/* MTU range: 14 - 15412 */
	net->min_mtu = ETH_HLEN;
	net->max_mtu = GETHER_MAX_MTU_SIZE;

	dev->gadget = g;
	SET_NETDEV_DEV(net, &g->dev);
	SET_NETDEV_DEVTYPE(net, &gadget_type);

	status = register_netdev(net);
	if (status < 0) {
		dev_dbg(&g->dev, "register_netdev failed, %d\n", status);
		free_netdev(net);
		dev = ERR_PTR(status);
	} else {
		INFO(dev, "MAC %pM\n", net->dev_addr);
		INFO(dev, "HOST MAC %pM\n", dev->host_mac);

		/*
		 * two kinds of host-initiated state changes:
		 *  - iff DATA transfer is active, carrier is "on"
		 *  - tx queueing enabled if open *and* carrier is "on"
		 */
		netif_carrier_off(net);
		uether_debugfs_init(dev);
	}

	return dev;
}
EXPORT_SYMBOL_GPL(gether_setup_name);

struct net_device *gether_setup_name_default(const char *netname)
{
	struct net_device	*net;
	struct eth_dev		*dev;

	net = alloc_etherdev(sizeof(*dev));
	if (!net)
		return ERR_PTR(-ENOMEM);

	dev = netdev_priv(net);
	spin_lock_init(&dev->lock);
	spin_lock_init(&dev->req_lock);
	INIT_WORK(&dev->work, eth_work);
	INIT_WORK(&dev->rx_work, process_rx_w);
	INIT_LIST_HEAD(&dev->tx_reqs);
	INIT_LIST_HEAD(&dev->rx_reqs);

	skb_queue_head_init(&dev->rx_frames);

	/* network device setup */
	dev->net = net;
	dev->qmult = QMULT_DEFAULT;
	snprintf(net->name, sizeof(net->name), "%s%%d", netname);

	eth_random_addr(dev->dev_mac);
	pr_warn("using random %s ethernet address\n", "self");
	eth_random_addr(dev->host_mac);
	pr_warn("using random %s ethernet address\n", "host");

	net->netdev_ops = &eth_netdev_ops;

	net->ethtool_ops = &ops;

	/* set operation mode to eth by default */
	set_bit(RMNET_MODE_LLP_ETH, &dev->flags);

	SET_NETDEV_DEVTYPE(net, &gadget_type);

	/* MTU range: 14 - 15412 */
	net->min_mtu = ETH_HLEN;
	net->max_mtu = GETHER_MAX_MTU_SIZE;

	return net;
}
EXPORT_SYMBOL_GPL(gether_setup_name_default);

int gether_register_netdev(struct net_device *net)
{
	struct eth_dev *dev;
	struct usb_gadget *g;
	int status;

	if (!net->dev.parent)
		return -EINVAL;
	dev = netdev_priv(net);
	g = dev->gadget;

	memcpy(net->dev_addr, dev->dev_mac, ETH_ALEN);
	net->addr_assign_type = NET_ADDR_RANDOM;

	status = register_netdev(net);
	if (status < 0) {
		dev_dbg(&g->dev, "register_netdev failed, %d\n", status);
		return status;
	} else {
		INFO(dev, "HOST MAC %pM\n", dev->host_mac);
		INFO(dev, "MAC %pM\n", dev->dev_mac);

		/* two kinds of host-initiated state changes:
		 *  - iff DATA transfer is active, carrier is "on"
		 *  - tx queueing enabled if open *and* carrier is "on"
		 */
		netif_carrier_off(net);
	}

	uether_debugfs_init(dev);

	return status;
}
EXPORT_SYMBOL_GPL(gether_register_netdev);

void gether_set_gadget(struct net_device *net, struct usb_gadget *g)
{
	struct eth_dev *dev;

	dev = netdev_priv(net);
	dev->gadget = g;
	SET_NETDEV_DEV(net, &g->dev);
}
EXPORT_SYMBOL_GPL(gether_set_gadget);

int gether_set_dev_addr(struct net_device *net, const char *dev_addr)
{
	struct eth_dev *dev;
	u8 new_addr[ETH_ALEN];

	dev = netdev_priv(net);
	if (get_ether_addr(dev_addr, new_addr))
		return -EINVAL;
	memcpy(dev->dev_mac, new_addr, ETH_ALEN);
	return 0;
}
EXPORT_SYMBOL_GPL(gether_set_dev_addr);

int gether_get_dev_addr(struct net_device *net, char *dev_addr, int len)
{
	struct eth_dev *dev;

	dev = netdev_priv(net);
	return get_ether_addr_str(dev->dev_mac, dev_addr, len);
}
EXPORT_SYMBOL_GPL(gether_get_dev_addr);

int gether_set_host_addr(struct net_device *net, const char *host_addr)
{
	struct eth_dev *dev;
	u8 new_addr[ETH_ALEN];

	dev = netdev_priv(net);
	if (get_ether_addr(host_addr, new_addr))
		return -EINVAL;
	memcpy(dev->host_mac, new_addr, ETH_ALEN);
	return 0;
}
EXPORT_SYMBOL_GPL(gether_set_host_addr);

int gether_get_host_addr(struct net_device *net, char *host_addr, int len)
{
	struct eth_dev *dev;
	dev = netdev_priv(net);
	return get_ether_addr_str(dev->host_mac, host_addr, len);
}
EXPORT_SYMBOL_GPL(gether_get_host_addr);

int gether_get_host_addr_cdc(struct net_device *net, char *host_addr, int len)
{
	struct eth_dev *dev;

	if (len < 13)
		return -EINVAL;

	dev = netdev_priv(net);
	snprintf(host_addr, len, "%pm", dev->host_mac);

	return strlen(host_addr);
}
EXPORT_SYMBOL_GPL(gether_get_host_addr_cdc);

void gether_get_host_addr_u8(struct net_device *net, u8 host_mac[ETH_ALEN])
{
	struct eth_dev *dev;

	dev = netdev_priv(net);
	memcpy(host_mac, dev->host_mac, ETH_ALEN);
}
EXPORT_SYMBOL_GPL(gether_get_host_addr_u8);

void gether_set_qmult(struct net_device *net, unsigned qmult)
{
	struct eth_dev *dev;

	dev = netdev_priv(net);
	dev->qmult = qmult;
}
EXPORT_SYMBOL_GPL(gether_set_qmult);

unsigned gether_get_qmult(struct net_device *net)
{
	struct eth_dev *dev;

	dev = netdev_priv(net);
	return dev->qmult;
}
EXPORT_SYMBOL_GPL(gether_get_qmult);

int gether_get_ifname(struct net_device *net, char *name, int len)
{

	rtnl_lock();
	strlcpy(name, netdev_name(net), len);
	rtnl_unlock();
	return strlen(name);
}
EXPORT_SYMBOL_GPL(gether_get_ifname);

/**
 * gether_cleanup - remove Ethernet-over-USB device
 * Context: may sleep
 *
 * This is called to free all resources allocated by @gether_setup().
 */
void gether_cleanup(struct eth_dev *dev)
{
	if (!dev)
		return;

	uether_debugfs_exit(dev);
	unregister_netdev(dev->net);
	flush_work(&dev->work);
	cancel_work_sync(&dev->rx_work);
	free_netdev(dev->net);
}
EXPORT_SYMBOL_GPL(gether_cleanup);

/**
 * gether_connect - notify network layer that USB link is active
 * @link: the USB link, set up with endpoints, descriptors matching
 *	current device speed, and any framing wrapper(s) set up.
 * Context: irqs blocked
 *
 * This is called to activate endpoints and let the network layer know
 * the connection is active ("carrier detect").  It may cause the I/O
 * queues to open and start letting network packets flow, but will in
 * any case activate the endpoints so that they respond properly to the
 * USB host.
 *
 * Verify net_device pointer returned using IS_ERR().  If it doesn't
 * indicate some error code (negative errno), ep->driver_data values
 * have been overwritten.
 */
struct net_device *gether_connect(struct gether *link)
{
	struct eth_dev		*dev = link->ioport;
	int			result = 0;

	if (!dev)
		return ERR_PTR(-EINVAL);

	/* size of rndis_packet_msg_type is 44 */
	link->header = kzalloc(44, GFP_ATOMIC);
	if (!link->header)
		return ERR_PTR(-ENOMEM);

	if (link->in_ep) {
		link->in_ep->driver_data = dev;
		result = usb_ep_enable(link->in_ep);
		if (result != 0) {
			DBG(dev, "enable %s --> %d\n",
				link->in_ep->name, result);
			goto fail0;
		}
	}

	if (link->out_ep) {
		link->out_ep->driver_data = dev;
		result = usb_ep_enable(link->out_ep);
		if (result != 0) {
			DBG(dev, "enable %s --> %d\n",
				link->out_ep->name, result);
			goto fail1;
		}
	}
	if (result == 0)
		result = alloc_requests(dev, link, qlen(dev->gadget,
					dev->qmult));

	if (result == 0) {
		dev->zlp = link->is_zlp_ok;
		dev->no_skb_reserve = link->no_skb_reserve;
		DBG(dev, "qlen %d\n", qlen(dev->gadget, dev->qmult));

		dev->header_len = link->header_len;
		dev->unwrap = link->unwrap;
		dev->wrap = link->wrap;
		dev->ul_max_pkts_per_xfer = link->ul_max_pkts_per_xfer;
		dev->dl_max_pkts_per_xfer = link->dl_max_pkts_per_xfer;

		spin_lock(&dev->lock);
		dev->tx_skb_hold_count = 0;
		dev->no_tx_req_used = 0;
		dev->tx_req_bufsize = 0;
		dev->port_usb = link;
		if (netif_running(dev->net)) {
			if (link->open)
				link->open(link);
		} else {
			if (link->close)
				link->close(link);
		}
		spin_unlock(&dev->lock);

		netif_carrier_on(dev->net);
		if (netif_running(dev->net))
			eth_start(dev, GFP_ATOMIC);

	/* on error, disable any endpoints  */
	} else {

		if (link->out_ep)
			(void) usb_ep_disable(link->out_ep);
fail1:

		if (link->in_ep)
			(void) usb_ep_disable(link->in_ep);
	}
fail0:
	/* caller is responsible for cleanup on error */
	if (result < 0) {
		kfree(link->header);

		return ERR_PTR(result);
	}

	return dev->net;
}
EXPORT_SYMBOL_GPL(gether_connect);

/**
 * gether_disconnect - notify network layer that USB link is inactive
 * @link: the USB link, on which gether_connect() was called
 * Context: irqs blocked
 *
 * This is called to deactivate endpoints and let the network layer know
 * the connection went inactive ("no carrier").
 *
 * On return, the state is as if gether_connect() had never been called.
 * The endpoints are inactive, and accordingly without active USB I/O.
 * Pointers to endpoint descriptors and endpoint private data are nulled.
 */
void gether_disconnect(struct gether *link)
{
	struct eth_dev		*dev = link->ioport;
	struct usb_request	*req;
	struct sk_buff		*skb;

	WARN_ON(!dev);
	if (!dev)
		return;

	DBG(dev, "%s\n", __func__);

	netif_stop_queue(dev->net);
	netif_carrier_off(dev->net);

	/* disable endpoints, forcing (synchronous) completion
	 * of all pending i/o.  then free the request objects
	 * and forget about the endpoints.
	 */
	if (link->in_ep) {
		usb_ep_disable(link->in_ep);
		spin_lock(&dev->req_lock);
		while (!list_empty(&dev->tx_reqs)) {
			req = container_of(dev->tx_reqs.next,
						struct usb_request, list);
			list_del(&req->list);

			spin_unlock(&dev->req_lock);
			if (link->multi_pkt_xfer) {
				kfree(req->buf);
				req->buf = NULL;
			}
			usb_ep_free_request(link->in_ep, req);
			spin_lock(&dev->req_lock);
		}
		spin_unlock(&dev->req_lock);
		/* Free rndis header buffer memory */
		kfree(link->header);
		link->header = NULL;
		link->in_ep->desc = NULL;
	}

	if (link->out_ep) {
		usb_ep_disable(link->out_ep);
		spin_lock(&dev->req_lock);
		while (!list_empty(&dev->rx_reqs)) {
			req = container_of(dev->rx_reqs.next,
						struct usb_request, list);
			list_del(&req->list);

			spin_unlock(&dev->req_lock);
			usb_ep_free_request(link->out_ep, req);
			spin_lock(&dev->req_lock);
		}
		spin_unlock(&dev->req_lock);

		spin_lock(&dev->rx_frames.lock);
		while ((skb = __skb_dequeue(&dev->rx_frames)))
			dev_kfree_skb_any(skb);
		spin_unlock(&dev->rx_frames.lock);

		link->out_ep->desc = NULL;
	}

	pr_debug("%s(): tx_throttle count= %lu", __func__,
					dev->tx_throttle);
	/* reset tx_throttle count */
	dev->tx_throttle = 0;
	dev->rx_throttle = 0;

	/* finish forgetting about this USB link episode */
	dev->header_len = 0;
	dev->unwrap = NULL;
	dev->wrap = NULL;

	spin_lock(&dev->lock);
	dev->port_usb = NULL;
	spin_unlock(&dev->lock);
}
EXPORT_SYMBOL_GPL(gether_disconnect);

static int uether_stat_show(struct seq_file *s, void *unused)
{
	struct eth_dev *dev = s->private;
	int ret = 0;

	if (dev) {
		seq_printf(s, "tx_throttle = %lu\n", dev->tx_throttle);
		seq_printf(s, "tx_pkts_rcvd=%u\n", dev->tx_pkts_rcvd);
		seq_printf(s, "rx_throttle = %lu\n", dev->rx_throttle);
		seq_printf(s, "skb_expand_cnt = %lu\n",
					dev->skb_expand_cnt);
	}
	return ret;
}

static int uether_open(struct inode *inode, struct file *file)
{
	return single_open(file, uether_stat_show, inode->i_private);
}

static ssize_t uether_stat_reset(struct file *file,
		const char __user *ubuf, size_t count, loff_t *ppos)
{
	struct seq_file *s = file->private_data;
	struct eth_dev *dev = s->private;
	unsigned long flags;

	spin_lock_irqsave(&dev->lock, flags);
	/* Reset tx_throttle */
	dev->tx_throttle = 0;
	dev->rx_throttle = 0;
	dev->skb_expand_cnt = 0;
	spin_unlock_irqrestore(&dev->lock, flags);
	return count;
}

static const struct file_operations uether_stats_ops = {
	.open = uether_open,
	.read = seq_read,
	.write = uether_stat_reset,
};

static void uether_debugfs_init(struct eth_dev *dev)
{
	struct dentry *uether_dent;
	struct dentry *uether_dfile;

	uether_dent = debugfs_create_dir("uether_rndis", NULL);
	if (IS_ERR(uether_dent))
		return;
	dev->uether_dent = uether_dent;

	uether_dfile = debugfs_create_file("status", 0644,
				uether_dent, dev, &uether_stats_ops);
	if (!uether_dfile || IS_ERR(uether_dfile))
		debugfs_remove(uether_dent);
	dev->uether_dfile = uether_dfile;
}

static void uether_debugfs_exit(struct eth_dev *dev)
{
	debugfs_remove(dev->uether_dfile);
	debugfs_remove(dev->uether_dent);
	dev->uether_dent = NULL;
	dev->uether_dfile = NULL;
}

static int __init gether_init(void)
{
	uether_wq  = create_singlethread_workqueue("uether");
	if (!uether_wq) {
		pr_err("%s: Unable to create workqueue: uether\n", __func__);
		return -ENOMEM;
	}
	return 0;
}
module_init(gether_init);

static void __exit gether_exit(void)
{
	destroy_workqueue(uether_wq);

}
module_exit(gether_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("David Brownell");
