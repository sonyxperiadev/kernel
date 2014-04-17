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
#include <linux/brcm_console.h>

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

#ifdef CONFIG_BRCM_NETCONSOLE

#define USB_EVENTQUEUE_SIZE 4

struct usb_event_queue_t {
	uint32_t		in_index;
	uint32_t		out_index;
	uint32_t		is_empty;
	spinlock_t		usbevent_lock;
	uint32_t		event[USB_EVENTQUEUE_SIZE];
};

static uint32_t queue_usbevent(struct usb_event_queue_t *queue, uint32_t event);
static uint32_t dequeue_usbevent(struct usb_event_queue_t *queue, uint32_t *pevent);
static uint32_t usb_eventqueue_init(struct usb_event_queue_t *queue);

static uint32_t usb_eventqueue_init(struct usb_event_queue_t *queue)
{
	spin_lock_init(&queue->usbevent_lock);
	queue->in_index = 0;
	queue->out_index = 0;
	queue->is_empty = 1;
	return 1;
}

static uint32_t is_queue_empty(struct usb_event_queue_t *queue)
{
	if ((queue->in_index == queue->out_index) && (queue->is_empty))
		return 1;
	else
		return 0;
}
static uint32_t is_queue_full(struct usb_event_queue_t *queue)
{
	if ((queue->in_index == queue->out_index) && (!queue->is_empty))
		return 1;
	else
		return 0;
}

static uint32_t queue_usbevent(struct usb_event_queue_t *queue, uint32_t event)
{
	unsigned long flags;
	spin_lock_irqsave(&queue->usbevent_lock, flags);
	if (is_queue_full(queue)) {
		spin_unlock_irqrestore(&queue->usbevent_lock, flags);
		return 0;
	}
	queue->event[queue->in_index++] = event;
	if (queue->in_index >= USB_EVENTQUEUE_SIZE) {
		queue->in_index = 0;
	}
	queue->is_empty = 0;
	spin_unlock_irqrestore(&queue->usbevent_lock, flags);
	return 1;
}

static uint32_t dequeue_usbevent(struct usb_event_queue_t *queue, uint32_t *pevent)
{
	unsigned long flags;
	spin_lock_irqsave(&queue->usbevent_lock, flags);
	if (is_queue_empty(queue)) {
		spin_unlock_irqrestore(&queue->usbevent_lock, flags);
		return 0;
	}
	*pevent = queue->event[queue->out_index++];
	if (queue->out_index >= USB_EVENTQUEUE_SIZE) {
		queue->out_index = 0;
	}
	if (queue->out_index == queue->in_index) {
		queue->is_empty = 1;
	}
	spin_unlock_irqrestore(&queue->usbevent_lock, flags);
	return 1;
}
#endif

struct eth_dev {
	/* lock is held while accessing port_usb
	 */
	spinlock_t		lock;
	struct gether		*port_usb;

	struct net_device	*net;
	struct usb_gadget	*gadget;

	spinlock_t		req_lock
#ifdef CONFIG_USB_ETH_SKB_ALLOC_OPTIMIZATION
	, req_rx_lock
#endif
	;	/* guard {rx,tx}_reqs */
	struct list_head	tx_reqs, rx_reqs;
	atomic_t		tx_qlen;

	struct sk_buff_head	rx_frames;

	unsigned		header_len;
	struct sk_buff		*(*wrap)(struct gether *, struct sk_buff *skb);
	int			(*unwrap)(struct gether *,
						struct sk_buff *skb,
						struct sk_buff_head *list);

	struct work_struct	work;
#ifdef CONFIG_BRCM_NETCONSOLE
	struct usb_event_queue_t usbevent_queue;
#endif

#ifdef CONFIG_USB_ETH_SKB_ALLOC_OPTIMIZATION
	struct workqueue_struct *rx_workqueue;
	unsigned char			*skb_data;
#endif
	unsigned long		todo;
#define	WORK_RX_MEMORY		0
#define	WORK_BRCM_NETCONSOLE_ON		1
#define	WORK_BRCM_NETCONSOLE_OFF	2
#ifdef CONFIG_USB_ETH_SKB_ALLOC_OPTIMIZATION
#define	WORK_ALLOC_RX_SKB			4
#endif
	bool			zlp;
	u8			host_mac[ETH_ALEN];
};

#ifdef CONFIG_BRCM_NETCONSOLE

static DEFINE_MUTEX(cleanup_netpoll_mutex);

void cleanup_netpoll_lock(void)
{
	mutex_lock(&cleanup_netpoll_mutex);
}

void cleanup_netpoll_unlock(void)
{
	if (mutex_is_locked(&cleanup_netpoll_mutex))
		mutex_unlock(&cleanup_netpoll_mutex);
}
#endif

#ifdef CONFIG_USB_ETH_SKB_ALLOC_OPTIMIZATION
#define	MAX_RX_SKB_SIZE			2048
#define	UETH_RX_SKB_THRESHOLD	192
#define	UETH_RX_SKB_THRESH_NCM	48
static struct sk_buff_head skb_rx_pool;
static int	rx_skb_q_no;
static bool is_rx_prealloc_mode;
static unsigned short max_skb_buf_sz, max_skb_buf_no;

/**
 * static void prealloc_rx_skbs(void) - the function is to alloc skb
 * if pre-allocated memory was able to reach the max_skb_buf_no.
 *
 */
static void alloc_rx_skb(void)
{
	struct sk_buff *skb;
	unsigned long flags;

	pr_debug("alloc_rx_skbs");
	spin_lock_irqsave(&skb_rx_pool.lock, flags);
	if (rx_skb_q_no < max_skb_buf_no) {
		skb = alloc_skb(max_skb_buf_sz, GFP_ATOMIC);
		if (!skb) {
			spin_unlock_irqrestore(&skb_rx_pool.lock, flags);
			return;
		} else {
			rx_skb_q_no++;
			pr_debug("TX_SKB NO:%d\n", skb_rx_pool.qlen);
			skb->signature = SKB_UETH_RX_THRESHOLD_SIG;
			__skb_queue_tail(&skb_rx_pool, skb);
		}
	}
	spin_unlock_irqrestore(&skb_rx_pool.lock, flags);
}


/**
 * static void refill_rx_skbs(struct sk_buff *skb) - put the skb buf back to queue to reuse
 *
 */
static void refill_rx_skbs(struct sk_buff *skb)
{
	unsigned long flags;

	spin_lock_irqsave(&skb_rx_pool.lock, flags);
	if (skb) {
		skb->signature = SKB_UETH_RX_THRESHOLD_SIG;
		__skb_queue_tail(&skb_rx_pool, skb);
		/* pr_info("@%d", skb_rx_pool.qlen); */
	} else {
		pr_warn("Can not refill the skb buffer....\n");
	}
	/* pr_info("@%d",skb_pool.qlen); */
	spin_unlock_irqrestore(&skb_rx_pool.lock, flags);
}

static void defer_kevent(struct eth_dev *dev, int flag);

/**
 * void ueth_recycle_rx_skbs_data - recycle the skb buffer with
 * the new skb header.
 *
 */
void ueth_recycle_rx_skb_data(unsigned char *skb_data, gfp_t gfp_flags, void  *dev_handle)
{
	struct sk_buff *skb;
	struct eth_dev *dev = dev_handle;

	skb = alloc_skb_uether_rx(max_skb_buf_sz, skb_data, gfp_flags);
	if (!skb) {
		pr_warn("retry: alloc ueth rx skb\n");
		dev->skb_data = skb_data;
		defer_kevent(dev, WORK_ALLOC_RX_SKB);
	} else
		refill_rx_skbs(skb);
}

/**
 * void ueth_rx_skb_queue_purge(struct sk_buff *skb) -
 *	Delete all buffers on an &sk_buff list. Each buffer is removed from
 *	the list and one reference dropped. This function takes the list
 *	lock and is atomic with respect to other list locking functions.
 *
 */
static void ueth_rx_skb_queue_purge(struct sk_buff_head *list)
{
	struct sk_buff *skb;
	while ((skb = skb_dequeue(list)) != NULL) {
		skb->signature = 0;
		kfree_skb(skb);
	}
}

/**
 * static void prealloc_rx_skbs(void) - reserve the skb bufs with
 * allocated memory for rx_submit.
 *
 */
static void prealloc_rx_skbs(void)
{
	struct sk_buff *skb;
	unsigned long flags;

	pr_info("prealloc_rx_skbs");
	rx_skb_q_no = 0;
	is_rx_prealloc_mode =  true;

	skb_queue_head_init(&skb_rx_pool);
	spin_lock_irqsave(&skb_rx_pool.lock, flags);
	while (skb_rx_pool.qlen < max_skb_buf_no) {
		skb = alloc_skb(max_skb_buf_sz, GFP_ATOMIC);
		if (!skb) {
			rx_skb_q_no = skb_rx_pool.qlen;
			spin_unlock_irqrestore(&skb_rx_pool.lock, flags);
			return;
		} else {
			rx_skb_q_no++;
			pr_debug("TX_SKB NO:%d\n", skb_rx_pool.qlen);
			skb->signature = SKB_UETH_RX_THRESHOLD_SIG;
			__skb_queue_tail(&skb_rx_pool, skb);
		}
	}
	spin_unlock_irqrestore(&skb_rx_pool.lock, flags);
}
#endif

/*-------------------------------------------------------------------------*/

#define RX_EXTRA	20	/* bytes guarding against rx overflows */

#define DEFAULT_QLEN	2	/* double buffering by default */

static unsigned qmult = 5;
module_param(qmult, uint, S_IRUGO|S_IWUSR);
MODULE_PARM_DESC(qmult, "queue length multiplier at high/super speed");

/* for dual-speed hardware, use deeper queues at high/super speed */
static inline int qlen(struct usb_gadget *gadget)
{
	if (gadget_is_dualspeed(gadget) && (gadget->speed == USB_SPEED_HIGH ||
					    gadget->speed == USB_SPEED_SUPER))
		return qmult * DEFAULT_QLEN;
	else
		return DEFAULT_QLEN;
}

/*-------------------------------------------------------------------------*/

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
	struct eth_dev	*dev = netdev_priv(net);
	unsigned long	flags;
	int		status = 0;

	/* don't change MTU on "live" link (peer won't know) */
	spin_lock_irqsave(&dev->lock, flags);
	if (dev->port_usb)
		status = -EBUSY;
	else if (new_mtu <= ETH_HLEN || new_mtu > ETH_FRAME_LEN)
		status = -ERANGE;
	else
		net->mtu = new_mtu;
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
#ifdef CONFIG_USB_ETH_SKB_ALLOC_OPTIMIZATION
	int testwkbit;
	unsigned long	flags;

	spin_lock_irqsave(&dev->req_rx_lock, flags);
	testwkbit = test_and_set_bit(flag, &dev->todo);
	spin_unlock_irqrestore(&dev->req_rx_lock, flags);

	if (testwkbit)
		return;
#else
	if (test_and_set_bit(flag, &dev->todo))
		return;
#endif

#ifdef CONFIG_BRCM_NETCONSOLE
	if ((flag == WORK_BRCM_NETCONSOLE_ON) || (flag == WORK_BRCM_NETCONSOLE_OFF)) {
		if (!queue_usbevent(&dev->usbevent_queue, flag)) {
			ERROR(dev, "event queue full, kevent %d may have been dropped\n", flag);
			return;
		}
	}
#endif

#ifdef CONFIG_USB_ETH_SKB_ALLOC_OPTIMIZATION
	if (dev->rx_workqueue) {
		if (!queue_work(dev->rx_workqueue, &dev->work))
			ERROR(dev, "kevent %d may have been dropped\n", flag);
		else
			DBG(dev, "kevent %d scheduled\n", flag);
		return;
	}
#endif
	if (!schedule_work(&dev->work))
		ERROR(dev, "kevent %d may have been dropped\n", flag);
	else
		DBG(dev, "kevent %d scheduled\n", flag);
}

static void rx_complete(struct usb_ep *ep, struct usb_request *req);

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
	spin_unlock_irqrestore(&dev->lock, flags);

	if (!out)
		return -ENOTCONN;


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

		if (dev->port_usb->is_fixed)
			size = max_t(size_t, size,
				dev->port_usb->fixed_out_len);

#ifdef CONFIG_USB_ETH_SKB_ALLOC_OPTIMIZATION
	if (is_rx_prealloc_mode) {
		if (size > max_skb_buf_sz) {
			pr_info("skb data size = %d\n", size);
			skb = alloc_skb(size + NET_IP_ALIGN, gfp_flags);
		} else
			skb = skb_dequeue(&skb_rx_pool);

		if (!skb) {
			alloc_rx_skb();
			pr_debug("No RX SKB...\n");
			goto enomem;
		}
	} else {
#endif
				skb = alloc_skb(size + NET_IP_ALIGN, gfp_flags);
			if (skb == NULL) {
				DBG(dev, "no rx skb\n");
			goto enomem;
		}
#ifdef CONFIG_USB_ETH_SKB_ALLOC_OPTIMIZATION
	}
#endif

	/* Some platforms perform better when IP packets are aligned,
	 * but on at least one, checksumming fails otherwise.  Note:
	 * RNDIS headers involve variable numbers of LE32 values.
	 */
	skb_reserve(skb, NET_IP_ALIGN);

	req->buf = skb->data;
	req->length = size;
	req->complete = rx_complete;
	req->context = skb;
	req->dma = 0;

#ifdef CONFIG_USB_ETH_SKB_ALLOC_OPTIMIZATION
	/* Need lock since it could be prempted by rx_complete
	when rx_submit is executed from schedule queue. */
	spin_lock_irqsave(&dev->req_rx_lock, flags);
#endif
	retval = usb_ep_queue(out, req, gfp_flags);
#ifdef CONFIG_USB_ETH_SKB_ALLOC_OPTIMIZATION
	spin_unlock_irqrestore(&dev->req_rx_lock, flags);
#endif
	if (retval == -ENOMEM)
enomem:
		defer_kevent(dev, WORK_RX_MEMORY);
	if (retval) {
		DBG(dev, "rx submit --> %d\n", retval);
		if (skb)
			dev_kfree_skb_any(skb);
		spin_lock_irqsave(&dev->req_lock, flags);
		list_add(&req->list, &dev->rx_reqs);
		spin_unlock_irqrestore(&dev->req_lock, flags);
	}
	return retval;
}

static void rx_complete(struct usb_ep *ep, struct usb_request *req)
{
	struct sk_buff	*skb = req->context, *skb2;
	struct eth_dev	*dev = ep->driver_data;
	int		status = req->status;

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
			} else {
				dev_kfree_skb_any(skb);
				status = -ENOTCONN;
			}
			spin_unlock_irqrestore(&dev->lock, flags);
		} else {
			skb_queue_tail(&dev->rx_frames, skb);
		}
		skb = NULL;

		skb2 = skb_dequeue(&dev->rx_frames);
		while (skb2) {
			if (status < 0
					|| ETH_HLEN > skb2->len
					|| skb2->len > VLAN_ETH_FRAME_LEN) {
				dev->net->stats.rx_errors++;
				dev->net->stats.rx_length_errors++;
				DBG(dev, "rx length %d\n", skb2->len);
				dev_kfree_skb_any(skb2);
				goto next_frame;
			}
			skb2->protocol = eth_type_trans(skb2, dev->net);
			dev->net->stats.rx_packets++;
			dev->net->stats.rx_bytes += skb2->len;

			/* no buffer copies needed, unless hardware can't
			 * use skb buffers.
			 */
			status = netif_rx(skb2);
next_frame:
			skb2 = skb_dequeue(&dev->rx_frames);
		}
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
		dev->net->stats.rx_errors++;
		DBG(dev, "rx status %d\n", status);
		break;
	}

	if (skb)
		dev_kfree_skb_any(skb);

	if (!netif_running(dev->net)) {
clean:
		spin_lock(&dev->req_lock);
		list_add(&req->list, &dev->rx_reqs);
		spin_unlock(&dev->req_lock);
		req = NULL;
	}
	if (req)
		rx_submit(dev, req, GFP_ATOMIC);
}

static int prealloc(struct list_head *list, struct usb_ep *ep, unsigned n)
{
	unsigned		i;
	struct usb_request	*req;

	if (!n)
		return -ENOMEM;

	/* queue/recycle up to N requests */
	i = n;
	list_for_each_entry(req, list, list) {
		if (i-- == 0)
			goto extra;
	}
	while (i--) {
		req = usb_ep_alloc_request(ep, GFP_ATOMIC);
		if (!req)
			return list_empty(list) ? -ENOMEM : 0;
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
	int	status;

	spin_lock(&dev->req_lock);
	status = prealloc(&dev->tx_reqs, link->in_ep, n);
	if (status < 0)
		goto fail;
	status = prealloc(&dev->rx_reqs, link->out_ep, n);
	if (status < 0)
		goto fail;
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

	/* fill unused rxq slots with some skb */
	spin_lock_irqsave(&dev->req_lock, flags);
	while (!list_empty(&dev->rx_reqs)) {
		req = container_of(dev->rx_reqs.next,
				struct usb_request, list);
		list_del_init(&req->list);
		spin_unlock_irqrestore(&dev->req_lock, flags);

		if (rx_submit(dev, req, gfp_flags) < 0) {
			defer_kevent(dev, WORK_RX_MEMORY);
			return;
		}

		spin_lock_irqsave(&dev->req_lock, flags);
	}
	spin_unlock_irqrestore(&dev->req_lock, flags);
}
#ifdef CONFIG_BRCM_NETCONSOLE
static void usb_test_work(struct work_struct *work)
{
	struct eth_dev	*dev = container_of(work, struct eth_dev, work);
	uint32_t event;

	while (dequeue_usbevent(&dev->usbevent_queue, &event)) {
		switch (event) {
		case WORK_BRCM_NETCONSOLE_ON:
			cleanup_netpoll_lock();
			brcm_current_netcon_status(USB_RNDIS_ON);
			cleanup_netpoll_unlock();
			break;

		case WORK_BRCM_NETCONSOLE_OFF:
			cleanup_netpoll_lock();
			brcm_current_netcon_status(USB_RNDIS_OFF);
			cleanup_netpoll_unlock();
			break;

		default:
			break;
		}
	}
	return;
}
#endif

static void eth_work(struct work_struct *work)
{
	struct eth_dev	*dev = container_of(work, struct eth_dev, work);

#ifdef CONFIG_USB_ETH_SKB_ALLOC_OPTIMIZATION
	int			testwkbit;
	unsigned long	flags;

	spin_lock_irqsave(&dev->req_rx_lock, flags);
	testwkbit = test_and_clear_bit(WORK_RX_MEMORY, &dev->todo);
	spin_unlock_irqrestore(&dev->req_rx_lock, flags);

	if (testwkbit) {
		if (netif_running(dev->net))
			rx_fill(dev, GFP_KERNEL);
	}
#ifdef CONFIG_BRCM_NETCONSOLE
	else {
		usb_test_work(work);
	}

	if (test_and_clear_bit(WORK_ALLOC_RX_SKB, &dev->todo))
		ueth_recycle_rx_skb_data(dev->skb_data, GFP_KERNEL, dev);
#endif

	if (dev->todo)
		DBG(dev, "work done, flags = 0x%lx\n", dev->todo);
	return;

#else /* CONFIG_USB_ETH_SKB_ALLOC_OPTIMIZATION */

	if (test_and_clear_bit(WORK_RX_MEMORY, &dev->todo)) {
		if (netif_running(dev->net))
			rx_fill(dev, GFP_KERNEL);
	}
#ifdef CONFIG_BRCM_NETCONSOLE
	else {
		usb_test_work(work);
	}
#endif

	if (dev->todo)
		DBG(dev, "work done, flags = 0x%lx\n", dev->todo);

#endif /* CONFIG_USB_ETH_SKB_ALLOC_OPTIMIZATION */

}


#ifdef CONFIG_NET_POLL_CONTROLLER
static void eth_poll_controller(struct net_device *dev)
{
	return; /* do not thing.... */
}
#endif

static void tx_complete(struct usb_ep *ep, struct usb_request *req)
{
	struct sk_buff	*skb = req->context;
	struct eth_dev	*dev = ep->driver_data;

	switch (req->status) {
	default:
		dev->net->stats.tx_errors++;
		VDBG(dev, "tx err %d\n", req->status);
		/* FALLTHROUGH */
	case -ECONNRESET:		/* unlink */
	case -ESHUTDOWN:		/* disconnect etc */
		break;
	case 0:
		dev->net->stats.tx_bytes += skb->len;
	}
	dev->net->stats.tx_packets++;

	spin_lock(&dev->req_lock);
	list_add(&req->list, &dev->tx_reqs);
	spin_unlock(&dev->req_lock);
	dev_kfree_skb_any(skb);

	atomic_dec(&dev->tx_qlen);
	if (netif_carrier_ok(dev->net))
		netif_wake_queue(dev->net);
}

static inline int is_promisc(u16 cdc_filter)
{
	return cdc_filter & USB_CDC_PACKET_TYPE_PROMISCUOUS;
}

static netdev_tx_t eth_start_xmit(struct sk_buff *skb,
					struct net_device *net)
{
	struct eth_dev		*dev = netdev_priv(net);
	int			length = skb->len;
	int			retval;
	struct usb_request	*req = NULL;
	unsigned long		flags;
	struct usb_ep		*in;
	u16			cdc_filter;

	spin_lock_irqsave(&dev->lock, flags);
	if (dev->port_usb) {
		in = dev->port_usb->in_ep;
		cdc_filter = dev->port_usb->cdc_filter;
	} else {
		in = NULL;
		cdc_filter = 0;
	}
	spin_unlock_irqrestore(&dev->lock, flags);

	if (!in) {
		dev_kfree_skb_any(skb);
		return NETDEV_TX_OK;
	}

	/* apply outgoing CDC or RNDIS filters */
	if (!is_promisc(cdc_filter)) {
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
				dev_kfree_skb_any(skb);
				return NETDEV_TX_OK;
			}
		}
		/* ignores USB_CDC_PACKET_TYPE_DIRECTED */
	}

	spin_lock_irqsave(&dev->req_lock, flags);
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
	if (list_empty(&dev->tx_reqs))
		netif_stop_queue(net);
	spin_unlock_irqrestore(&dev->req_lock, flags);

	/* no buffer copies needed, unless the network stack did it
	 * or the hardware can't use skb buffers.
	 * or there's not enough space for extra headers we need
	 */
	if (skb->signature != SKB_NETPOLL_SIGNATURE) {
		if (dev->wrap) {
			unsigned long	flags;

			spin_lock_irqsave(&dev->lock, flags);
			if (dev->port_usb)
				skb = dev->wrap(dev->port_usb, skb);
			spin_unlock_irqrestore(&dev->lock, flags);
			if (!skb)
				goto drop;
#ifdef CONFIG_USB_ETH_SKB_ALLOC_OPTIMIZATION
			/* The following is to eliminate to malloc for
				the unaligned memory*/
			spin_lock_irqsave(&dev->lock, flags);
			if (skb_headroom(skb) >
				((unsigned long)skb->data & 3)) {
				u8 *data = skb->data;
				size_t len = skb_headlen(skb);
				skb->data -= ((unsigned long)skb->data & 3);
				memmove(skb->data, data, len);
				skb_set_tail_pointer(skb, len);
				pr_debug("mem_mv");
			}
			spin_unlock_irqrestore(&dev->lock, flags);
#endif
		}
	}
	length = skb->len;

	req->buf = skb->data;
	req->context = skb;
	req->complete = tx_complete;

	/* NCM requires no zlp if transfer is dwNtbInMaxSize */
	if (dev->port_usb->is_fixed &&
	    length == dev->port_usb->fixed_in_len &&
	    (length % in->maxpacket) == 0)
		req->zero = 0;
	else
		req->zero = 1;

	/* use zlp framing on tx for strict CDC-Ether conformance,
	 * though any robust network rx path ignores extra padding.
	 * and some hardware doesn't like to write zlps.
	 */
	if (req->zero && !dev->zlp && (length % in->maxpacket) == 0)
		length++;

	req->length = length;

	/* throttle high/super speed IRQ rate back slightly */
	if (gadget_is_dualspeed(dev->gadget))
		req->no_interrupt = (dev->gadget->speed == USB_SPEED_HIGH ||
				     dev->gadget->speed == USB_SPEED_SUPER)
			? ((atomic_read(&dev->tx_qlen) % qmult) != 0)
			: 0;

	req->dma = 0;

#ifdef CONFIG_USB_ETH_SKB_ALLOC_OPTIMIZATION
	spin_lock_irqsave(&dev->req_lock, flags);
#endif
	retval = usb_ep_queue(in, req, GFP_ATOMIC);
#ifdef CONFIG_USB_ETH_SKB_ALLOC_OPTIMIZATION
	spin_unlock_irqrestore(&dev->req_lock, flags);
#endif
	switch (retval) {
	default:
		DBG(dev, "tx queue err %d\n", retval);
		break;
	case 0:
		net->trans_start = jiffies;
		atomic_inc(&dev->tx_qlen);
	}

	if (retval) {
		dev_kfree_skb_any(skb);
drop:
		dev->net->stats.tx_dropped++;
		spin_lock_irqsave(&dev->req_lock, flags);
		if (list_empty(&dev->tx_reqs))
			netif_start_queue(net);
		list_add(&req->list, &dev->tx_reqs);
		spin_unlock_irqrestore(&dev->req_lock, flags);
	}
	return NETDEV_TX_OK;
}

/*-------------------------------------------------------------------------*/

static void eth_start(struct eth_dev *dev, gfp_t gfp_flags)
{
	DBG(dev, "%s\n", __func__);

	/* fill the rx queue */
	rx_fill(dev, gfp_flags);

	/* and open the tx floodgates */
	atomic_set(&dev->tx_qlen, 0);
	netif_wake_queue(dev->net);

#ifdef CONFIG_BRCM_NETCONSOLE
	defer_kevent(dev, WORK_BRCM_NETCONSOLE_ON);
#endif
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
		in = link->in_ep->desc;
		out = link->out_ep->desc;
		usb_ep_disable(link->in_ep);
		usb_ep_disable(link->out_ep);
		if (netif_carrier_ok(net)) {
			DBG(dev, "host still using in/out endpoints\n");
			link->in_ep->desc = in;
			link->out_ep->desc = out;
			usb_ep_enable(link->in_ep);
			usb_ep_enable(link->out_ep);
		}
	}
	spin_unlock_irqrestore(&dev->lock, flags);

	return 0;
}

/*-------------------------------------------------------------------------*/

/* initial value, changed by "ifconfig usb0 hw ether xx:xx:xx:xx:xx:xx" */
static char *dev_addr;
module_param(dev_addr, charp, S_IRUGO);
MODULE_PARM_DESC(dev_addr, "Device Ethernet Address");

/* this address is invisible to ifconfig */
static char *host_addr;
module_param(host_addr, charp, S_IRUGO);
MODULE_PARM_DESC(host_addr, "Host Ethernet Address");

#if 0 /* See gether_setup */
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
			dev_addr[i] = num;
		}
		if (is_valid_ether_addr(dev_addr))
			return 0;
	}
	eth_random_addr(dev_addr);
	return 1;
}
#endif

static const struct net_device_ops eth_netdev_ops = {
	.ndo_open		= eth_open,
	.ndo_stop		= eth_stop,
	.ndo_start_xmit		= eth_start_xmit,
	.ndo_change_mtu		= ueth_change_mtu,
	.ndo_set_mac_address	= eth_mac_addr,
	.ndo_validate_addr	= eth_validate_addr,
#ifdef CONFIG_NET_POLL_CONTROLLER
	.ndo_poll_controller = eth_poll_controller,
#endif
};

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
 * Returns negative errno, or zero on success
 */
struct eth_dev *gether_setup_name(struct usb_gadget *g, u8 ethaddr[ETH_ALEN],
		const char *netname)
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
	INIT_LIST_HEAD(&dev->tx_reqs);
	INIT_LIST_HEAD(&dev->rx_reqs);
	skb_queue_head_init(&dev->rx_frames);

#ifdef CONFIG_USB_ETH_SKB_ALLOC_OPTIMIZATION
	spin_lock_init(&dev->req_rx_lock);
	dev->rx_workqueue = create_workqueue("rx_ether_queue");
	if (dev->rx_workqueue == NULL)
		pr_err("rx_ether_queue creation fail once\n");
#endif

#ifdef CONFIG_BRCM_NETCONSOLE
	usb_eventqueue_init(&dev->usbevent_queue);
#endif
	/* network device setup */
	dev->net = net;
	snprintf(net->name, sizeof(net->name), "%s%%d", netname);

#if 0 /* We use the fixed host MAC address for USB logging. */
	if (get_ether_addr(dev_addr, net->dev_addr))
		dev_warn(&g->dev,
			"using random %s ethernet address\n", "self");
	if (get_ether_addr(host_addr, dev->host_mac))
		dev_warn(&g->dev,
			"using random %s ethernet address\n", "host");
#else
	dev_warn(&g->dev,
			"using fixed %s ethernet address\n", "self");
	net->dev_addr[0] = 0x22;
	net->dev_addr[1] = 0x33;
	net->dev_addr[2] = 0x44;
	net->dev_addr[3] = 0x55;
	net->dev_addr[4] = 0x66;
	net->dev_addr[5] = 0x77;
	dev_warn(&g->dev,
			"using fixed %s ethernet address\n", "host");
	dev->host_mac[0] = 0xaa;
	dev->host_mac[1] = 0xbb;
	dev->host_mac[2] = 0xcc;
	dev->host_mac[3] = 0xdd;
	dev->host_mac[4] = 0xee;
	dev->host_mac[5] = 0xff;
#endif
	if (ethaddr)
		memcpy(ethaddr, dev->host_mac, ETH_ALEN);

	net->netdev_ops = &eth_netdev_ops;

	SET_ETHTOOL_OPS(net, &ops);

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

		/* two kinds of host-initiated state changes:
		 *  - iff DATA transfer is active, carrier is "on"
		 *  - tx queueing enabled if open *and* carrier is "on"
		 */
		netif_carrier_off(net);
	}

	return dev;
}

/**
 * gether_cleanup - remove Ethernet-over-USB device
 * Context: may sleep
 *
 * This is called to free all resources allocated by @gether_setup().
 */
void gether_cleanup(struct eth_dev *dev)
{
	pr_info("%s\n", __func__);

	if (!dev)
		return;

	unregister_netdev(dev->net);
	flush_work(&dev->work);
#ifdef CONFIG_USB_ETH_SKB_ALLOC_OPTIMIZATION
	flush_workqueue(dev->rx_workqueue);
	destroy_workqueue(dev->rx_workqueue);
#endif
	free_netdev(dev->net);
}

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

	pr_info("%s\n", __func__);

	if (!dev)
		return ERR_PTR(-EINVAL);

	link->in_ep->driver_data = dev;
	result = usb_ep_enable(link->in_ep);
	if (result != 0) {
		DBG(dev, "enable %s --> %d\n",
			link->in_ep->name, result);
		goto fail0;
	}

	link->out_ep->driver_data = dev;
	result = usb_ep_enable(link->out_ep);
	if (result != 0) {
		DBG(dev, "enable %s --> %d\n",
			link->out_ep->name, result);
		goto fail1;
	}

	if (result == 0)
		result = alloc_requests(dev, link, qlen(dev->gadget));

	if (result == 0) {
#ifdef CONFIG_USB_ETH_SKB_ALLOC_OPTIMIZATION
		if (link->is_fixed) { /* NCM */
			max_skb_buf_no = UETH_RX_SKB_THRESH_NCM;
			max_skb_buf_sz = link->fixed_out_len;
		} else {
			max_skb_buf_no = UETH_RX_SKB_THRESHOLD;
			max_skb_buf_sz = MAX_RX_SKB_SIZE;
		}
		pr_info("USB Ether: It is in SKB prealloc mode!\n");
		prealloc_rx_skbs();
#endif
		dev->zlp = link->is_zlp_ok;
		DBG(dev, "qlen %d\n", qlen(dev->gadget));

		dev->header_len = link->header_len;
		dev->unwrap = link->unwrap;
		dev->wrap = link->wrap;

		spin_lock(&dev->lock);
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
		(void) usb_ep_disable(link->out_ep);
fail1:
		(void) usb_ep_disable(link->in_ep);
	}
fail0:
	/* caller is responsible for cleanup on error */
	if (result < 0)
		return ERR_PTR(result);
	return dev->net;
}

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

	WARN_ON(!dev);
	if (!dev)
		return;

	DBG(dev, "%s\n", __func__);
	pr_info("%s\n", __func__);

#ifdef CONFIG_BRCM_NETCONSOLE
	defer_kevent(dev, WORK_BRCM_NETCONSOLE_OFF);
#endif

	netif_stop_queue(dev->net);
	netif_carrier_off(dev->net);

	/* disable endpoints, forcing (synchronous) completion
	 * of all pending i/o.  then free the request objects
	 * and forget about the endpoints.
	 */
	usb_ep_disable(link->in_ep);
	spin_lock(&dev->req_lock);
	while (!list_empty(&dev->tx_reqs)) {
		req = container_of(dev->tx_reqs.next,
					struct usb_request, list);
		list_del(&req->list);

		spin_unlock(&dev->req_lock);
		usb_ep_free_request(link->in_ep, req);
		spin_lock(&dev->req_lock);
	}
	spin_unlock(&dev->req_lock);
	link->in_ep->driver_data = NULL;
	link->in_ep->desc = NULL;

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
	link->out_ep->driver_data = NULL;
	link->out_ep->desc = NULL;

	/* finish forgetting about this USB link episode */
	dev->header_len = 0;
	dev->unwrap = NULL;
	dev->wrap = NULL;

	spin_lock(&dev->lock);
	dev->port_usb = NULL;
	spin_unlock(&dev->lock);
#ifdef CONFIG_USB_ETH_SKB_ALLOC_OPTIMIZATION
	ueth_rx_skb_queue_purge(&skb_rx_pool);
#endif
}
