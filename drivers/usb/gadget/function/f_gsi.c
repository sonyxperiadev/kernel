/*
 * Copyright (c) 2015-2018, Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "f_gsi.h"
#include "rndis.h"

struct usb_gsi_debugfs {
	struct dentry *debugfs_root;
};

static struct usb_gsi_debugfs debugfs;

static bool qti_packet_debug;
module_param(qti_packet_debug, bool, 0644);
MODULE_PARM_DESC(qti_packet_debug, "Print QTI Packet's Raw Data");

/* initial value, changed by "ifconfig usb0 hw ether xx:xx:xx:xx:xx:xx" */
static char *gsi_dev_addr;
module_param(gsi_dev_addr, charp, 0644);
MODULE_PARM_DESC(gsi_dev_addr, "QC Device Ethernet Address");

/* this address is invisible to ifconfig */
static char *gsi_host_addr;
module_param(gsi_host_addr, charp, 0644);
MODULE_PARM_DESC(gsi_host_addr, "QC Host Ethernet Address");

static struct workqueue_struct *ipa_usb_wq;
static struct f_gsi *__gsi[USB_PROT_MAX];
static void *ipc_log_ctxt;

#define NUM_LOG_PAGES 15
#define log_event_err(x, ...) do { \
	if (gsi) { \
		ipc_log_string(ipc_log_ctxt, "id%d:"x, gsi->prot_id, \
				##__VA_ARGS__); \
		pr_err("id%d:"x, gsi->prot_id, ##__VA_ARGS__); \
	} \
} while (0)

#define log_event_dbg(x, ...) do { \
	if (gsi) { \
		ipc_log_string(ipc_log_ctxt, "id%d:"x, gsi->prot_id, \
				##__VA_ARGS__); \
		pr_debug("id%d:"x, gsi->prot_id, ##__VA_ARGS__); \
	} \
} while (0)

#define log_event_info(x, ...) do { \
	if (gsi) { \
		ipc_log_string(ipc_log_ctxt, "id%d:"x, gsi->prot_id, \
		##__VA_ARGS__); \
		pr_info("id%d:"x, gsi->prot_id, ##__VA_ARGS__); \
	} \
} while (0)

/* Deregister misc device and free instance structures */
static void gsi_rndis_ipa_reset_trigger(struct gsi_data_port *d_port);
static void ipa_disconnect_handler(struct gsi_data_port *d_port);
static int gsi_ctrl_send_notification(struct f_gsi *gsi);
static int gsi_alloc_trb_buffer(struct f_gsi *gsi);
static void gsi_free_trb_buffer(struct f_gsi *gsi);
static struct gsi_ctrl_pkt *gsi_ctrl_pkt_alloc(unsigned int len, gfp_t flags);
static void gsi_ctrl_pkt_free(struct gsi_ctrl_pkt *pkt);

static inline bool is_ext_prot_ether(int prot_id)
{
	if (prot_id == USB_PROT_RMNET_ETHER ||
				prot_id == USB_PROT_DPL_ETHER)
		return true;

	return false;
}

static inline bool usb_gsi_remote_wakeup_allowed(struct usb_function *f)
{
	bool remote_wakeup_allowed;
	struct f_gsi *gsi = func_to_gsi(f);

	if (f->config->cdev->gadget->speed == USB_SPEED_SUPER)
		remote_wakeup_allowed = f->func_wakeup_allowed;
	else
		remote_wakeup_allowed = f->config->cdev->gadget->remote_wakeup;

	log_event_dbg("%s: remote_wakeup_allowed:%s", __func__,
			(remote_wakeup_allowed ? "true" : "false"));
	return remote_wakeup_allowed;
}

static void post_event(struct gsi_data_port *port, u8 event)
{
	unsigned long flags;
	struct f_gsi *gsi = d_port_to_gsi(port);

	spin_lock_irqsave(&port->evt_q.q_lock, flags);

	port->evt_q.tail++;
	/* Check for wraparound and make room */
	port->evt_q.tail = port->evt_q.tail % MAXQUEUELEN;

	/* Check for overflow */
	if (port->evt_q.tail == port->evt_q.head) {
		log_event_err("%s: event queue overflow error", __func__);
		spin_unlock_irqrestore(&port->evt_q.q_lock, flags);
		return;
	}
	/* Add event to queue */
	port->evt_q.event[port->evt_q.tail] = event;
	spin_unlock_irqrestore(&port->evt_q.q_lock, flags);
}

static void __maybe_unused post_event_to_evt_queue(struct gsi_data_port *port,
								u8 event)
{
	post_event(port, event);
	queue_work(port->ipa_usb_wq, &port->usb_ipa_w);
}

static u8 read_event(struct gsi_data_port *port)
{
	u8 event;
	unsigned long flags;
	struct f_gsi *gsi = d_port_to_gsi(port);

	spin_lock_irqsave(&port->evt_q.q_lock, flags);
	if (port->evt_q.head == port->evt_q.tail) {
		log_event_dbg("%s: event queue empty", __func__);
		spin_unlock_irqrestore(&port->evt_q.q_lock, flags);
		return EVT_NONE;
	}

	port->evt_q.head++;
	/* Check for wraparound and make room */
	port->evt_q.head = port->evt_q.head % MAXQUEUELEN;

	event = port->evt_q.event[port->evt_q.head];
	spin_unlock_irqrestore(&port->evt_q.q_lock, flags);

	return event;
}

static u8 peek_event(struct gsi_data_port *port)
{
	u8 event;
	unsigned long flags;
	u8 peek_index = 0;
	struct f_gsi *gsi = d_port_to_gsi(port);

	spin_lock_irqsave(&port->evt_q.q_lock, flags);
	if (port->evt_q.head == port->evt_q.tail) {
		log_event_dbg("%s: event queue empty", __func__);
		spin_unlock_irqrestore(&port->evt_q.q_lock, flags);
		return EVT_NONE;
	}

	peek_index = (port->evt_q.head + 1) % MAXQUEUELEN;
	event = port->evt_q.event[peek_index];
	spin_unlock_irqrestore(&port->evt_q.q_lock, flags);

	return event;
}

static void __maybe_unused reset_event_queue(struct gsi_data_port *port)
{
	unsigned long flags;

	spin_lock_irqsave(&port->evt_q.q_lock, flags);
	port->evt_q.head = port->evt_q.tail = MAXQUEUELEN - 1;
	memset(&port->evt_q.event[0], EVT_NONE, MAXQUEUELEN);
	spin_unlock_irqrestore(&port->evt_q.q_lock, flags);
}

static int gsi_wakeup_host(struct f_gsi *gsi)
{

	int ret;
	struct usb_gadget *gadget;
	struct usb_function *func;

	func = &gsi->function;
	gadget = gsi->function.config->cdev->gadget;

	log_event_dbg("Entering %s", __func__);

	if (!gadget) {
		log_event_err("FAILED: d_port->cdev->gadget == NULL");
		return -ENODEV;
	}

	/*
	 * In Super-Speed mode, remote wakeup is not allowed for suspended
	 * functions which have been disallowed by the host to issue Function
	 * Remote Wakeup.
	 * Note - We deviate here from the USB 3.0 spec and allow
	 * non-suspended functions to issue remote-wakeup even if they were not
	 * allowed to do so by the host. This is done in order to support non
	 * fully USB 3.0 compatible hosts.
	 */
	if ((gadget->speed == USB_SPEED_SUPER) && (func->func_is_suspended)) {
		log_event_dbg("%s: Calling usb_func_wakeup", __func__);
		ret = usb_func_wakeup(func);
	} else {
		log_event_dbg("%s: Calling usb_gadget_wakeup", __func__);
		ret = usb_gadget_wakeup(gadget);
	}

	if ((ret == -EBUSY) || (ret == -EAGAIN))
		log_event_dbg("RW delayed due to LPM exit.");
	else if (ret)
		log_event_err("wakeup failed. ret=%d.", ret);

	return ret;
}

static void gsi_rw_timer_func(unsigned long arg)
{
	struct f_gsi *gsi = (struct f_gsi *)arg;

	if (!atomic_read(&gsi->connected)) {
		log_event_dbg("%s: gsi not connected.. bail-out\n", __func__);
		gsi->debugfs_rw_timer_enable = 0;
		return;
	}

	log_event_dbg("%s: calling gsi_wakeup_host\n", __func__);
	gsi_wakeup_host(gsi);

	if (gsi->debugfs_rw_timer_enable) {
		log_event_dbg("%s: re-arm the timer\n", __func__);
		mod_timer(&gsi->gsi_rw_timer,
			jiffies + msecs_to_jiffies(gsi->gsi_rw_timer_interval));
	}
}

static struct f_gsi *get_connected_gsi(void)
{
	struct f_gsi *connected_gsi;
	bool gsi_connected = false;
	unsigned int i;

	for (i = 0; i < IPA_USB_MAX_TETH_PROT_SIZE; i++) {
		connected_gsi = __gsi[i];
		if (connected_gsi && atomic_read(&connected_gsi->connected)) {
			gsi_connected = true;
			break;
		}
	}

	if (!gsi_connected)
		connected_gsi = NULL;

	return connected_gsi;
}

#define DEFAULT_RW_TIMER_INTERVAL 500 /* in ms */
static ssize_t usb_gsi_rw_write(struct file *file,
			const char __user *ubuf, size_t count, loff_t *ppos)
{
	struct f_gsi *gsi;
	u8 input;
	int ret;

	gsi = get_connected_gsi();
	if (!gsi) {
		log_event_dbg("%s: gsi not connected\n", __func__);
		goto err;
	}

	if (ubuf == NULL) {
		log_event_dbg("%s: buffer is Null.\n", __func__);
		goto err;
	}

	ret = kstrtou8_from_user(ubuf, count, 0, &input);
	if (ret) {
		log_event_err("%s: Invalid value. err:%d\n", __func__, ret);
		goto err;
	}

	if (gsi->debugfs_rw_timer_enable == !!input) {
		if (!!input)
			log_event_dbg("%s: RW already enabled\n", __func__);
		else
			log_event_dbg("%s: RW already disabled\n", __func__);
		goto err;
	}

	gsi->debugfs_rw_timer_enable = !!input;

	if (gsi->debugfs_rw_timer_enable) {
		mod_timer(&gsi->gsi_rw_timer, jiffies +
			  msecs_to_jiffies(gsi->gsi_rw_timer_interval));
		log_event_dbg("%s: timer initialized\n", __func__);
	} else {
		del_timer_sync(&gsi->gsi_rw_timer);
		log_event_dbg("%s: timer deleted\n", __func__);
	}

err:
	return count;
}

static int usb_gsi_rw_show(struct seq_file *s, void *unused)
{

	struct f_gsi *gsi;

	gsi = get_connected_gsi();
	if (!gsi) {
		log_event_dbg("%s: gsi not connected\n", __func__);
		return 0;
	}

	seq_printf(s, "%d\n", gsi->debugfs_rw_timer_enable);

	return 0;
}

static int usb_gsi_rw_open(struct inode *inode, struct file *f)
{
	return single_open(f, usb_gsi_rw_show, inode->i_private);
}

static const struct file_operations fops_usb_gsi_rw = {
	.open = usb_gsi_rw_open,
	.read = seq_read,
	.write = usb_gsi_rw_write,
	.owner = THIS_MODULE,
	.llseek = seq_lseek,
	.release = seq_release,
};

static ssize_t usb_gsi_rw_timer_write(struct file *file,
			const char __user *ubuf, size_t count, loff_t *ppos)
{
	struct f_gsi *gsi;
	u16 timer_val;
	int ret;

	gsi = get_connected_gsi();
	if (!gsi) {
		log_event_dbg("%s: gsi not connected\n", __func__);
		goto err;
	}

	if (ubuf == NULL) {
		log_event_dbg("%s: buffer is NULL.\n", __func__);
		goto err;
	}

	ret = kstrtou16_from_user(ubuf, count, 0, &timer_val);
	if (ret) {
		log_event_err("%s: Invalid value. err:%d\n", __func__, ret);
		goto err;
	}

	if (timer_val <= 0 || timer_val >  10000) {
		log_event_err("%s: value must be > 0 and < 10000.\n", __func__);
		goto err;
	}

	gsi->gsi_rw_timer_interval = timer_val;
err:
	return count;
}

static int usb_gsi_rw_timer_show(struct seq_file *s, void *unused)
{
	struct f_gsi *gsi;

	gsi = get_connected_gsi();
	if (!gsi) {
		log_event_dbg("%s: gsi not connected\n", __func__);
		return 0;
	}

	seq_printf(s, "%ums\n", gsi->gsi_rw_timer_interval);

	return 0;
}

static int usb_gsi_rw_timer_open(struct inode *inode, struct file *f)
{
	return single_open(f, usb_gsi_rw_timer_show, inode->i_private);
}

static const struct file_operations fops_usb_gsi_rw_timer = {
	.open = usb_gsi_rw_timer_open,
	.read = seq_read,
	.write = usb_gsi_rw_timer_write,
	.owner = THIS_MODULE,
	.llseek = seq_lseek,
	.release = seq_release,
};

static int usb_gsi_debugfs_init(void)
{
	debugfs.debugfs_root = debugfs_create_dir("usb_gsi", NULL);
	if (!debugfs.debugfs_root)
		return -ENOMEM;

	debugfs_create_file("remote_wakeup_enable", 0600,
					debugfs.debugfs_root,
					__gsi, &fops_usb_gsi_rw);
	debugfs_create_file("remote_wakeup_interval", 0600,
					debugfs.debugfs_root,
					__gsi,
					&fops_usb_gsi_rw_timer);
	return 0;
}

static void usb_gsi_debugfs_exit(void)
{
	debugfs_remove_recursive(debugfs.debugfs_root);
}

/*
 * Callback for when when network interface is up
 * and userspace is ready to answer DHCP requests,  or remote wakeup
 */
int ipa_usb_notify_cb(enum ipa_usb_notify_event event,
	void *driver_data)
{
	struct f_gsi *gsi = driver_data;
	unsigned long flags;
	struct gsi_ctrl_pkt *cpkt_notify_connect, *cpkt_notify_speed;

	if (!gsi) {
		log_event_err("%s: invalid driver data", __func__);
		return -EINVAL;
	}

	spin_lock_irqsave(&gsi->d_port.lock, flags);

	switch (event) {
	case IPA_USB_DEVICE_READY:

		if (gsi->d_port.net_ready_trigger) {
			spin_unlock_irqrestore(&gsi->d_port.lock, flags);
			log_event_dbg("%s: Already triggered", __func__);
			return 1;
		}

		log_event_err("%s: Set net_ready_trigger", __func__);
		gsi->d_port.net_ready_trigger = true;

		if (gsi->prot_id == USB_PROT_ECM_IPA) {
			cpkt_notify_connect = gsi_ctrl_pkt_alloc(0, GFP_ATOMIC);
			if (IS_ERR(cpkt_notify_connect)) {
				spin_unlock_irqrestore(&gsi->d_port.lock,
								flags);
				log_event_dbg("%s: err cpkt_notify_connect\n",
								__func__);
				return -ENOMEM;
			}
			cpkt_notify_connect->type = GSI_CTRL_NOTIFY_CONNECT;

			cpkt_notify_speed = gsi_ctrl_pkt_alloc(0, GFP_ATOMIC);
			if (IS_ERR(cpkt_notify_speed)) {
				spin_unlock_irqrestore(&gsi->d_port.lock,
								flags);
				gsi_ctrl_pkt_free(cpkt_notify_connect);
				log_event_dbg("%s: err cpkt_notify_speed\n",
								__func__);
				return -ENOMEM;
			}
			cpkt_notify_speed->type = GSI_CTRL_NOTIFY_SPEED;
			spin_lock(&gsi->c_port.lock);
			list_add_tail(&cpkt_notify_connect->list,
					&gsi->c_port.cpkt_resp_q);
			list_add_tail(&cpkt_notify_speed->list,
					&gsi->c_port.cpkt_resp_q);
			spin_unlock(&gsi->c_port.lock);
			gsi_ctrl_send_notification(gsi);
		}

		/*
		 * Do not post EVT_CONNECTED for RNDIS.
		 * Data path for RNDIS is enabled on EVT_HOST_READY.
		 */
		if (gsi->prot_id != USB_PROT_RNDIS_IPA) {
			post_event(&gsi->d_port, EVT_CONNECTED);
			queue_work(gsi->d_port.ipa_usb_wq,
					&gsi->d_port.usb_ipa_w);
		}
		break;

	case IPA_USB_REMOTE_WAKEUP:
		gsi_wakeup_host(gsi);
		break;

	case IPA_USB_SUSPEND_COMPLETED:
		post_event(&gsi->d_port, EVT_IPA_SUSPEND);
		queue_work(gsi->d_port.ipa_usb_wq, &gsi->d_port.usb_ipa_w);
		break;
	}

	spin_unlock_irqrestore(&gsi->d_port.lock, flags);
	return 1;
}

static int ipa_connect_channels(struct gsi_data_port *d_port)
{
	int ret;
	struct f_gsi *gsi = d_port_to_gsi(d_port);
	struct ipa_usb_xdci_chan_params *in_params =
				&d_port->ipa_in_channel_params;
	struct ipa_usb_xdci_chan_params *out_params =
				&d_port->ipa_out_channel_params;
	struct ipa_usb_xdci_connect_params *conn_params =
				&d_port->ipa_conn_pms;
	struct usb_composite_dev *cdev = gsi->function.config->cdev;
	struct gsi_channel_info gsi_channel_info;
	struct ipa_req_chan_out_params ipa_in_channel_out_params;
	struct ipa_req_chan_out_params ipa_out_channel_out_params;

	log_event_dbg("%s: USB GSI IN OPS", __func__);
	usb_gsi_ep_op(d_port->in_ep, &d_port->in_request,
		GSI_EP_OP_PREPARE_TRBS);
	usb_gsi_ep_op(d_port->in_ep, &d_port->in_request,
			GSI_EP_OP_STARTXFER);
	d_port->in_xfer_rsc_index = usb_gsi_ep_op(d_port->in_ep, NULL,
			GSI_EP_OP_GET_XFER_IDX);

	memset(in_params, 0x0, sizeof(*in_params));
	gsi_channel_info.ch_req = &d_port->in_request;
	usb_gsi_ep_op(d_port->in_ep, (void *)&gsi_channel_info,
			GSI_EP_OP_GET_CH_INFO);

	log_event_dbg("%s: USB GSI IN OPS Completed", __func__);
	in_params->client =
		(gsi->prot_id != USB_PROT_DIAG_IPA) ? IPA_CLIENT_USB_CONS :
						IPA_CLIENT_USB_DPL_CONS;
	in_params->ipa_ep_cfg.mode.mode = IPA_BASIC;
	in_params->teth_prot = gsi->prot_id;
	in_params->gevntcount_low_addr =
		gsi_channel_info.gevntcount_low_addr;
	in_params->gevntcount_hi_addr =
		gsi_channel_info.gevntcount_hi_addr;
	in_params->dir = GSI_CHAN_DIR_FROM_GSI;
	in_params->xfer_ring_len = gsi_channel_info.xfer_ring_len;
	in_params->xfer_scratch.last_trb_addr_iova =
					gsi_channel_info.last_trb_addr;
	in_params->xfer_ring_base_addr_iova =
					gsi_channel_info.xfer_ring_base_addr;
	in_params->data_buff_base_len = d_port->in_request.buf_len *
					d_port->in_request.num_bufs;
	in_params->data_buff_base_addr_iova = d_port->in_request.dma;
	in_params->sgt_xfer_rings = &d_port->in_request.sgt_trb_xfer_ring;
	in_params->sgt_data_buff = &d_port->in_request.sgt_data_buff;
	log_event_dbg("%s(): IN: sgt_xfer_rings:%pK sgt_data_buff:%pK\n",
		__func__, in_params->sgt_xfer_rings, in_params->sgt_data_buff);
	in_params->xfer_scratch.const_buffer_size =
		gsi_channel_info.const_buffer_size;
	in_params->xfer_scratch.depcmd_low_addr =
		gsi_channel_info.depcmd_low_addr;
	in_params->xfer_scratch.depcmd_hi_addr =
		gsi_channel_info.depcmd_hi_addr;

	if (d_port->out_ep) {
		log_event_dbg("%s: USB GSI OUT OPS", __func__);
		usb_gsi_ep_op(d_port->out_ep, &d_port->out_request,
			GSI_EP_OP_PREPARE_TRBS);
		usb_gsi_ep_op(d_port->out_ep, &d_port->out_request,
				GSI_EP_OP_STARTXFER);
		d_port->out_xfer_rsc_index =
			usb_gsi_ep_op(d_port->out_ep,
				NULL, GSI_EP_OP_GET_XFER_IDX);
		memset(out_params, 0x0, sizeof(*out_params));
		gsi_channel_info.ch_req = &d_port->out_request;
		usb_gsi_ep_op(d_port->out_ep, (void *)&gsi_channel_info,
				GSI_EP_OP_GET_CH_INFO);
		log_event_dbg("%s: USB GSI OUT OPS Completed", __func__);
		out_params->client = IPA_CLIENT_USB_PROD;
		out_params->ipa_ep_cfg.mode.mode = IPA_BASIC;
		out_params->teth_prot = gsi->prot_id;
		out_params->gevntcount_low_addr =
			gsi_channel_info.gevntcount_low_addr;
		out_params->gevntcount_hi_addr =
			gsi_channel_info.gevntcount_hi_addr;
		out_params->dir = GSI_CHAN_DIR_TO_GSI;
		out_params->xfer_ring_len =
			gsi_channel_info.xfer_ring_len;
		out_params->xfer_ring_base_addr_iova =
			gsi_channel_info.xfer_ring_base_addr;
		out_params->data_buff_base_len = d_port->out_request.buf_len *
			d_port->out_request.num_bufs;
		out_params->data_buff_base_addr_iova =
			d_port->out_request.dma;
		out_params->sgt_xfer_rings =
			&d_port->out_request.sgt_trb_xfer_ring;
		out_params->sgt_data_buff = &d_port->out_request.sgt_data_buff;
		log_event_dbg("%s(): OUT: sgt_xfer_rings:%pK sgt_data_buff:%pK\n",
			__func__, out_params->sgt_xfer_rings,
			out_params->sgt_data_buff);

		out_params->xfer_scratch.last_trb_addr_iova =
			gsi_channel_info.last_trb_addr;
		out_params->xfer_scratch.const_buffer_size =
			gsi_channel_info.const_buffer_size;
		out_params->xfer_scratch.depcmd_low_addr =
			gsi_channel_info.depcmd_low_addr;
		out_params->xfer_scratch.depcmd_hi_addr =
			gsi_channel_info.depcmd_hi_addr;
	}

	/* Populate connection params */
	conn_params->max_pkt_size =
		(cdev->gadget->speed == USB_SPEED_SUPER) ?
		IPA_USB_SUPER_SPEED_1024B : IPA_USB_HIGH_SPEED_512B;
	conn_params->ipa_to_usb_xferrscidx =
			d_port->in_xfer_rsc_index;
	conn_params->usb_to_ipa_xferrscidx =
			d_port->out_xfer_rsc_index;
	conn_params->usb_to_ipa_xferrscidx_valid =
			(gsi->prot_id != USB_PROT_DIAG_IPA) ? true : false;
	conn_params->ipa_to_usb_xferrscidx_valid = true;
	conn_params->teth_prot = gsi->prot_id;
	conn_params->teth_prot_params.max_xfer_size_bytes_to_dev = 23700;
	conn_params->teth_prot_params.max_xfer_size_bytes_to_dev
				= d_port->out_aggr_size;
	conn_params->teth_prot_params.max_xfer_size_bytes_to_host
					= d_port->in_aggr_size;
	conn_params->teth_prot_params.max_packet_number_to_dev =
		DEFAULT_MAX_PKT_PER_XFER;
	conn_params->max_supported_bandwidth_mbps =
		(cdev->gadget->speed == USB_SPEED_SUPER) ? 3600 : 400;

	memset(&ipa_in_channel_out_params, 0x0,
				sizeof(ipa_in_channel_out_params));
	memset(&ipa_out_channel_out_params, 0x0,
				sizeof(ipa_out_channel_out_params));

	log_event_dbg("%s: Calling xdci_connect", __func__);
	ret = ipa_usb_xdci_connect(out_params, in_params,
					&ipa_out_channel_out_params,
					&ipa_in_channel_out_params,
					conn_params);
	if (ret) {
		log_event_err("%s: IPA connect failed %d", __func__, ret);
		return ret;
	}
	log_event_dbg("%s: xdci_connect done", __func__);

	log_event_dbg("%s: IN CH HDL %x", __func__,
			ipa_in_channel_out_params.clnt_hdl);
	log_event_dbg("%s: IN CH DBL addr %x", __func__,
			ipa_in_channel_out_params.db_reg_phs_addr_lsb);

	log_event_dbg("%s: OUT CH HDL %x", __func__,
			ipa_out_channel_out_params.clnt_hdl);
	log_event_dbg("%s: OUT CH DBL addr %x", __func__,
			ipa_out_channel_out_params.db_reg_phs_addr_lsb);

	d_port->in_channel_handle = ipa_in_channel_out_params.clnt_hdl;
	d_port->in_request.db_reg_phs_addr_lsb =
		ipa_in_channel_out_params.db_reg_phs_addr_lsb;
	d_port->in_request.db_reg_phs_addr_msb =
		ipa_in_channel_out_params.db_reg_phs_addr_msb;

	if (gsi->prot_id != USB_PROT_DIAG_IPA) {
		d_port->out_channel_handle =
			ipa_out_channel_out_params.clnt_hdl;
		d_port->out_request.db_reg_phs_addr_lsb =
			ipa_out_channel_out_params.db_reg_phs_addr_lsb;
		d_port->out_request.db_reg_phs_addr_msb =
			ipa_out_channel_out_params.db_reg_phs_addr_msb;
	}
	return ret;
}

static void ipa_data_path_enable(struct gsi_data_port *d_port)
{
	struct f_gsi *gsi = d_port_to_gsi(d_port);
	bool block_db = false;

	log_event_dbg("IN: db_reg_phs_addr_lsb = %x",
			gsi->d_port.in_request.db_reg_phs_addr_lsb);
	usb_gsi_ep_op(gsi->d_port.in_ep,
			&gsi->d_port.in_request,
			GSI_EP_OP_STORE_DBL_INFO);

	if (gsi->d_port.out_ep) {
		log_event_dbg("OUT: db_reg_phs_addr_lsb = %x",
				gsi->d_port.out_request.db_reg_phs_addr_lsb);
		usb_gsi_ep_op(gsi->d_port.out_ep,
				&gsi->d_port.out_request,
				GSI_EP_OP_STORE_DBL_INFO);

		usb_gsi_ep_op(gsi->d_port.out_ep, &gsi->d_port.out_request,
				GSI_EP_OP_ENABLE_GSI);
	}

	/* Unblock doorbell to GSI */
	usb_gsi_ep_op(d_port->in_ep, (void *)&block_db,
				GSI_EP_OP_SET_CLR_BLOCK_DBL);

	usb_gsi_ep_op(gsi->d_port.in_ep, &gsi->d_port.in_request,
						GSI_EP_OP_RING_DB);

	if (gsi->d_port.out_ep)
		usb_gsi_ep_op(gsi->d_port.out_ep, &gsi->d_port.out_request,
						GSI_EP_OP_RING_DB);
}

static void ipa_disconnect_handler(struct gsi_data_port *d_port)
{
	struct f_gsi *gsi = d_port_to_gsi(d_port);
	bool block_db = true;

	log_event_dbg("%s: EP Disable for data", __func__);

	if (gsi->d_port.in_ep) {
		/*
		 * Block doorbell to GSI to avoid USB wrapper from
		 * ringing doorbell in case IPA clocks are OFF.
		 */
		usb_gsi_ep_op(d_port->in_ep, (void *)&block_db,
				GSI_EP_OP_SET_CLR_BLOCK_DBL);
		usb_gsi_ep_op(gsi->d_port.in_ep,
				&gsi->d_port.in_request, GSI_EP_OP_DISABLE);
	}

	if (gsi->d_port.out_ep)
		usb_gsi_ep_op(gsi->d_port.out_ep,
				&gsi->d_port.out_request, GSI_EP_OP_DISABLE);

	gsi->d_port.net_ready_trigger = false;
}

static void ipa_disconnect_work_handler(struct gsi_data_port *d_port)
{
	int ret;
	struct f_gsi *gsi = d_port_to_gsi(d_port);

	log_event_dbg("%s: Calling xdci_disconnect", __func__);

	ret = ipa_usb_xdci_disconnect(gsi->d_port.out_channel_handle,
			gsi->d_port.in_channel_handle, gsi->prot_id);
	if (ret)
		log_event_err("%s: IPA disconnect failed %d",
				__func__, ret);

	log_event_dbg("%s: xdci_disconnect done", __func__);

	/* invalidate channel handles*/
	gsi->d_port.in_channel_handle = -EINVAL;
	gsi->d_port.out_channel_handle = -EINVAL;

	usb_gsi_ep_op(gsi->d_port.in_ep, &gsi->d_port.in_request,
							GSI_EP_OP_FREE_TRBS);

	if (gsi->d_port.out_ep)
		usb_gsi_ep_op(gsi->d_port.out_ep, &gsi->d_port.out_request,
							GSI_EP_OP_FREE_TRBS);

	/* free buffers allocated with each TRB */
	gsi_free_trb_buffer(gsi);
}

static int ipa_suspend_work_handler(struct gsi_data_port *d_port)
{
	int ret = 0;
	bool block_db, f_suspend;
	struct f_gsi *gsi = d_port_to_gsi(d_port);
	struct usb_function *f = &gsi->function;

	f_suspend = f->func_wakeup_allowed;
	log_event_dbg("%s: f_suspend:%d", __func__, f_suspend);

	if (!usb_gsi_ep_op(gsi->d_port.in_ep, (void *) &f_suspend,
				GSI_EP_OP_CHECK_FOR_SUSPEND)) {
		ret = -EFAULT;
		block_db = false;
		usb_gsi_ep_op(d_port->in_ep, (void *)&block_db,
			GSI_EP_OP_SET_CLR_BLOCK_DBL);
		goto done;
	}

	log_event_dbg("%s: Calling xdci_suspend", __func__);
	ret = ipa_usb_xdci_suspend(gsi->d_port.out_channel_handle,
				gsi->d_port.in_channel_handle, gsi->prot_id,
				usb_gsi_remote_wakeup_allowed(f));
	if (!ret) {
		d_port->sm_state = STATE_SUSPENDED;
		log_event_dbg("%s: STATE SUSPENDED", __func__);
		goto done;
	}

	if (ret == -EFAULT) {
		block_db = false;
		usb_gsi_ep_op(d_port->in_ep, (void *)&block_db,
					GSI_EP_OP_SET_CLR_BLOCK_DBL);
		gsi_wakeup_host(gsi);
	} else if (ret == -EINPROGRESS) {
		d_port->sm_state = STATE_SUSPEND_IN_PROGRESS;
	} else {
		log_event_err("%s: Error %d for %d", __func__, ret,
							gsi->prot_id);
	}
done:
	log_event_dbg("%s: xdci_suspend ret %d", __func__, ret);
	return ret;
}

static void ipa_resume_work_handler(struct gsi_data_port *d_port)
{
	bool block_db;
	struct f_gsi *gsi = d_port_to_gsi(d_port);
	int ret;

	log_event_dbg("%s: Calling xdci_resume", __func__);

	ret = ipa_usb_xdci_resume(gsi->d_port.out_channel_handle,
					gsi->d_port.in_channel_handle,
					gsi->prot_id);
	if (ret)
		log_event_dbg("%s: xdci_resume ret %d", __func__, ret);

	log_event_dbg("%s: xdci_resume done", __func__);

	block_db = false;
	usb_gsi_ep_op(d_port->in_ep, (void *)&block_db,
			GSI_EP_OP_SET_CLR_BLOCK_DBL);
}

static void ipa_work_handler(struct work_struct *w)
{
	struct gsi_data_port *d_port = container_of(w, struct gsi_data_port,
						  usb_ipa_w);
	u8 event;
	int ret = 0;
	struct usb_gadget *gadget = d_port->gadget;
	struct device *dev;
	struct device *gad_dev;
	struct f_gsi *gsi = d_port_to_gsi(d_port);
	bool block_db;

	event = read_event(d_port);

	log_event_dbg("%s: event = %x sm_state %x", __func__,
			event, d_port->sm_state);

	if (gadget) {
		dev = &gadget->dev;
		if (!dev || !dev->parent) {
			log_event_err("%s(): dev or dev->parent is NULL.\n",
					__func__);
			return;
		}
		gad_dev = dev->parent;
	} else {
		log_event_err("%s(): gadget is NULL.\n", __func__);
		return;
	}

	switch (d_port->sm_state) {
	case STATE_UNINITIALIZED:
		break;
	case STATE_INITIALIZED:
		if (event == EVT_CONNECT_IN_PROGRESS) {
			usb_gadget_autopm_get(d_port->gadget);
			log_event_dbg("%s: get = %d", __func__,
				atomic_read(&gad_dev->power.usage_count));
			/* allocate buffers used with each TRB */
			ret = gsi_alloc_trb_buffer(gsi);
			if (ret) {
				log_event_err("%s: gsi_alloc_trb_failed\n",
								__func__);
				break;
			}
			ipa_connect_channels(d_port);
			d_port->sm_state = STATE_CONNECT_IN_PROGRESS;
			log_event_dbg("%s: ST_INIT_EVT_CONN_IN_PROG",
					__func__);
		} else if (event == EVT_HOST_READY) {
			/*
			 * When in a composition such as RNDIS + ADB,
			 * RNDIS host sends a GEN_CURRENT_PACKET_FILTER msg
			 * to enable/disable flow control eg. during RNDIS
			 * adaptor disable/enable from device manager.
			 * In the case of the msg to disable flow control,
			 * connect IPA channels and enable data path.
			 * EVT_HOST_READY is posted to the state machine
			 * in the handler for this msg.
			 */
			usb_gadget_autopm_get(d_port->gadget);
			log_event_dbg("%s: get = %d", __func__,
				atomic_read(&gad_dev->power.usage_count));
			/* allocate buffers used with each TRB */
			ret = gsi_alloc_trb_buffer(gsi);
			if (ret) {
				log_event_err("%s: gsi_alloc_trb_failed\n",
								__func__);
				break;
			}

			ipa_connect_channels(d_port);
			ipa_data_path_enable(d_port);
			d_port->sm_state = STATE_CONNECTED;
			log_event_dbg("%s: ST_INIT_EVT_HOST_READY", __func__);
		}
		break;
	case STATE_CONNECT_IN_PROGRESS:
		if (event == EVT_HOST_READY) {
			ipa_data_path_enable(d_port);
			d_port->sm_state = STATE_CONNECTED;
			log_event_dbg("%s: ST_CON_IN_PROG_EVT_HOST_READY",
					 __func__);
		} else if (event == EVT_CONNECTED) {
			if (peek_event(d_port) == EVT_SUSPEND) {
				log_event_dbg("%s: ST_CON_IN_PROG_EVT_SUSPEND",
					 __func__);
				break;
			}
			ipa_data_path_enable(d_port);
			d_port->sm_state = STATE_CONNECTED;
			log_event_dbg("%s: ST_CON_IN_PROG_EVT_CON %d",
					__func__, __LINE__);
		} else if (event == EVT_SUSPEND) {
			if (peek_event(d_port) == EVT_DISCONNECTED) {
				read_event(d_port);
				ipa_disconnect_work_handler(d_port);
				d_port->sm_state = STATE_INITIALIZED;
				usb_gadget_autopm_put_async(d_port->gadget);
				log_event_dbg("%s: ST_CON_IN_PROG_EVT_SUS_DIS",
						__func__);
				log_event_dbg("%s: put_async1 = %d", __func__,
						atomic_read(
						&gad_dev->power.usage_count));
				break;
			}
			ret = ipa_suspend_work_handler(d_port);
			if (!ret) {
				usb_gadget_autopm_put_async(d_port->gadget);
				log_event_dbg("%s: ST_CON_IN_PROG_EVT_SUS",
						__func__);
				log_event_dbg("%s: put_async2 = %d", __func__,
						atomic_read(
						&gad_dev->power.usage_count));
			}
		} else if (event == EVT_DISCONNECTED) {
			ipa_disconnect_work_handler(d_port);
			d_port->sm_state = STATE_INITIALIZED;
			usb_gadget_autopm_put_async(d_port->gadget);
			log_event_dbg("%s: ST_CON_IN_PROG_EVT_DIS",
						__func__);
			log_event_dbg("%s: put_async3 = %d",
					__func__, atomic_read(
						&gad_dev->power.usage_count));
		}
		break;
	case STATE_CONNECTED:
		if (event == EVT_DISCONNECTED || event == EVT_HOST_NRDY) {
			if (peek_event(d_port) == EVT_HOST_READY) {
				read_event(d_port);
				log_event_dbg("%s: NO_OP NRDY_RDY", __func__);
				break;
			}

			if (event == EVT_HOST_NRDY) {
				log_event_dbg("%s: ST_CON_HOST_NRDY\n",
								__func__);
				block_db = true;
				/* stop USB ringing doorbell to GSI(OUT_EP) */
				usb_gsi_ep_op(d_port->in_ep, (void *)&block_db,
						GSI_EP_OP_SET_CLR_BLOCK_DBL);
				gsi_rndis_ipa_reset_trigger(d_port);
				usb_gsi_ep_op(d_port->in_ep, NULL,
						GSI_EP_OP_ENDXFER);
				usb_gsi_ep_op(d_port->out_ep, NULL,
						GSI_EP_OP_ENDXFER);
			}

			ipa_disconnect_work_handler(d_port);
			d_port->sm_state = STATE_INITIALIZED;
			usb_gadget_autopm_put_async(d_port->gadget);
			log_event_dbg("%s: ST_CON_EVT_DIS", __func__);
			log_event_dbg("%s: put_async4 = %d",
					__func__, atomic_read(
						&gad_dev->power.usage_count));
		} else if (event == EVT_SUSPEND) {
			if (peek_event(d_port) == EVT_DISCONNECTED) {
				read_event(d_port);
				ipa_disconnect_work_handler(d_port);
				d_port->sm_state = STATE_INITIALIZED;
				usb_gadget_autopm_put_async(d_port->gadget);
				log_event_dbg("%s: ST_CON_EVT_SUS_DIS",
						__func__);
				log_event_dbg("%s: put_async5 = %d",
						__func__, atomic_read(
						&gad_dev->power.usage_count));
				break;
			}
			ret = ipa_suspend_work_handler(d_port);
			if (!ret) {
				usb_gadget_autopm_put_async(d_port->gadget);
				log_event_dbg("%s: ST_CON_EVT_SUS",
						__func__);
				log_event_dbg("%s: put_async6 = %d",
						__func__, atomic_read(
						&gad_dev->power.usage_count));
			}
		} else if (event == EVT_CONNECTED) {
			d_port->sm_state = STATE_CONNECTED;
			log_event_dbg("%s: ST_CON_EVT_CON", __func__);
		}
		break;
	case STATE_DISCONNECTED:
		if (event == EVT_CONNECT_IN_PROGRESS) {
			ipa_connect_channels(d_port);
			d_port->sm_state = STATE_CONNECT_IN_PROGRESS;
			log_event_dbg("%s: ST_DIS_EVT_CON_IN_PROG", __func__);
		} else if (event == EVT_UNINITIALIZED) {
			d_port->sm_state = STATE_UNINITIALIZED;
			log_event_dbg("%s: ST_DIS_EVT_UNINIT", __func__);
		}
		break;
	case STATE_SUSPEND_IN_PROGRESS:
		if (event == EVT_IPA_SUSPEND) {
			d_port->sm_state = STATE_SUSPENDED;
			usb_gadget_autopm_put_async(d_port->gadget);
			log_event_dbg("%s: ST_SUS_IN_PROG_EVT_IPA_SUS",
					__func__);
			log_event_dbg("%s: put_async6 = %d",
						__func__, atomic_read(
						&gad_dev->power.usage_count));
		} else	if (event == EVT_RESUMED) {
			ipa_resume_work_handler(d_port);
			d_port->sm_state = STATE_CONNECTED;
			/*
			 * Increment usage count here to disallow gadget
			 * parent suspend. This counter will decrement
			 * after IPA disconnect is done in disconnect work
			 * (due to cable disconnect) or in suspended state.
			 */
			usb_gadget_autopm_get_noresume(d_port->gadget);
			log_event_dbg("%s: ST_SUS_IN_PROG_EVT_RES", __func__);
			log_event_dbg("%s: get_nores1 = %d", __func__,
					atomic_read(
						&gad_dev->power.usage_count));
		} else if (event == EVT_DISCONNECTED) {
			ipa_disconnect_work_handler(d_port);
			d_port->sm_state = STATE_INITIALIZED;
			usb_gadget_autopm_put_async(d_port->gadget);
			log_event_dbg("%s: ST_SUS_IN_PROG_EVT_DIS", __func__);
			log_event_dbg("%s: put_async7 = %d", __func__,
					atomic_read(
						&gad_dev->power.usage_count));
		}
		break;

	case STATE_SUSPENDED:
		if (event == EVT_RESUMED) {
			usb_gadget_autopm_get(d_port->gadget);
			log_event_dbg("%s: ST_SUS_EVT_RES", __func__);
			log_event_dbg("%s: get = %d", __func__,
				atomic_read(&gad_dev->power.usage_count));
			ipa_resume_work_handler(d_port);
			d_port->sm_state = STATE_CONNECTED;
		} else if (event == EVT_DISCONNECTED) {
			ipa_disconnect_work_handler(d_port);
			d_port->sm_state = STATE_INITIALIZED;
			log_event_dbg("%s: ST_SUS_EVT_DIS", __func__);
		}
		break;
	default:
		log_event_dbg("%s: Invalid state to SM", __func__);
	}

	if (peek_event(d_port) != EVT_NONE) {
		log_event_dbg("%s: New events to process", __func__);
		queue_work(d_port->ipa_usb_wq, &d_port->usb_ipa_w);
	}
}

static struct gsi_ctrl_pkt *gsi_ctrl_pkt_alloc(unsigned int len, gfp_t flags)
{
	struct gsi_ctrl_pkt *pkt;

	pkt = kzalloc(sizeof(struct gsi_ctrl_pkt), flags);
	if (!pkt)
		return ERR_PTR(-ENOMEM);

	pkt->buf = kmalloc(len, flags);
	if (!pkt->buf) {
		kfree(pkt);
		return ERR_PTR(-ENOMEM);
	}
	pkt->len = len;

	return pkt;
}

static void gsi_ctrl_pkt_free(struct gsi_ctrl_pkt *pkt)
{
	if (pkt) {
		kfree(pkt->buf);
		kfree(pkt);
	}
}

static void gsi_ctrl_clear_cpkt_queues(struct f_gsi *gsi, bool skip_req_q)
{
	struct gsi_ctrl_pkt *cpkt = NULL;
	struct list_head *act, *tmp;

	spin_lock(&gsi->c_port.lock);
	if (skip_req_q)
		goto clean_resp_q;

	list_for_each_safe(act, tmp, &gsi->c_port.cpkt_req_q) {
		cpkt = list_entry(act, struct gsi_ctrl_pkt, list);
		list_del(&cpkt->list);
		gsi_ctrl_pkt_free(cpkt);
	}
clean_resp_q:
	list_for_each_safe(act, tmp, &gsi->c_port.cpkt_resp_q) {
		cpkt = list_entry(act, struct gsi_ctrl_pkt, list);
		list_del(&cpkt->list);
		gsi_ctrl_pkt_free(cpkt);
	}
	spin_unlock(&gsi->c_port.lock);
}

static int gsi_ctrl_send_cpkt_tomodem(struct f_gsi *gsi, void *buf, size_t len)
{
	unsigned long flags;
	struct gsi_ctrl_port *c_port = &gsi->c_port;
	struct gsi_ctrl_pkt *cpkt;

	spin_lock_irqsave(&c_port->lock, flags);
	/* drop cpkt if port is not open */
	if (!gsi->c_port.is_open) {
		log_event_dbg("%s: ctrl device %s is not open",
			   __func__, gsi->c_port.name);
		c_port->cpkt_drop_cnt++;
		spin_unlock_irqrestore(&c_port->lock, flags);
		return -ENODEV;
	}

	cpkt = gsi_ctrl_pkt_alloc(len, GFP_ATOMIC);
	if (IS_ERR(cpkt)) {
		log_event_err("%s: Reset func pkt allocation failed", __func__);
		spin_unlock_irqrestore(&c_port->lock, flags);
		return -ENOMEM;
	}

	memcpy(cpkt->buf, buf, len);
	cpkt->len = len;

	list_add_tail(&cpkt->list, &c_port->cpkt_req_q);
	c_port->host_to_modem++;
	spin_unlock_irqrestore(&c_port->lock, flags);

	log_event_dbg("%s: Wake up read queue", __func__);
	wake_up(&c_port->read_wq);

	return 0;
}

static int gsi_ctrl_dev_open(struct inode *ip, struct file *fp)
{
	struct gsi_ctrl_port *c_port = container_of(fp->private_data,
						struct gsi_ctrl_port,
						ctrl_device);
	struct f_gsi *gsi;

	if (!c_port) {
		pr_err_ratelimited("%s: gsi ctrl port %pK", __func__, c_port);
		return -ENODEV;
	}

	gsi = c_port_to_gsi(c_port);

	log_event_dbg("%s: open ctrl dev %s", __func__, c_port->name);

	if (c_port->is_open) {
		log_event_err("%s: Already opened\n", __func__);
		return -EBUSY;
	}

	c_port->is_open = true;

	return 0;
}

static int gsi_ctrl_dev_release(struct inode *ip, struct file *fp)
{
	struct gsi_ctrl_port *c_port = container_of(fp->private_data,
						struct gsi_ctrl_port,
						ctrl_device);
	struct f_gsi *gsi;

	if (!c_port) {
		pr_err_ratelimited("%s: gsi ctrl port NULL", __func__);
		return -ENODEV;
	}

	gsi = c_port_to_gsi(c_port);

	log_event_dbg("close ctrl dev %s\n", c_port->name);

	c_port->is_open = false;

	return 0;
}

static ssize_t
gsi_ctrl_dev_read(struct file *fp, char __user *buf, size_t count, loff_t *pos)
{
	struct gsi_ctrl_port *c_port = container_of(fp->private_data,
						struct gsi_ctrl_port,
						ctrl_device);
	struct gsi_ctrl_pkt *cpkt = NULL;
	unsigned long flags;
	int ret = 0;
	struct f_gsi *gsi;

	if (!c_port) {
		pr_err_ratelimited("%s: gsi ctrl port NULL", __func__);
		return -ENODEV;
	}

	gsi = c_port_to_gsi(c_port);

	log_event_dbg("%s: Enter %zu", __func__, count);

	if (count > GSI_MAX_CTRL_PKT_SIZE) {
		log_event_err("Large buff size %zu, should be %d",
			count, GSI_MAX_CTRL_PKT_SIZE);
		return -EINVAL;
	}

	/* block until a new packet is available */
	spin_lock_irqsave(&c_port->lock, flags);
	while (list_empty(&c_port->cpkt_req_q)) {
		log_event_dbg("Requests list is empty. Wait.");
		spin_unlock_irqrestore(&c_port->lock, flags);
		ret = wait_event_interruptible(c_port->read_wq,
			!list_empty(&c_port->cpkt_req_q));
		if (ret < 0) {
			log_event_err("Waiting failed");
			return -ERESTARTSYS;
		}
		log_event_dbg("Received request packet");
		spin_lock_irqsave(&c_port->lock, flags);
	}

	cpkt = list_first_entry(&c_port->cpkt_req_q, struct gsi_ctrl_pkt,
							list);
	list_del(&cpkt->list);
	spin_unlock_irqrestore(&c_port->lock, flags);

	if (cpkt->len > count) {
		log_event_err("cpkt size large:%d > buf size:%zu",
				cpkt->len, count);
		gsi_ctrl_pkt_free(cpkt);
		return -ENOMEM;
	}

	log_event_dbg("%s: cpkt size:%d", __func__, cpkt->len);
	if (qti_packet_debug)
		print_hex_dump(KERN_DEBUG, "READ:", DUMP_PREFIX_OFFSET, 16, 1,
			cpkt->buf, min_t(int, 30, cpkt->len), false);

	ret = copy_to_user(buf, cpkt->buf, cpkt->len);
	if (ret) {
		log_event_err("copy_to_user failed: err %d", ret);
		ret = -EFAULT;
	} else {
		log_event_dbg("%s: copied %d bytes to user", __func__,
							cpkt->len);
		ret = cpkt->len;
		c_port->copied_to_modem++;
	}

	gsi_ctrl_pkt_free(cpkt);

	log_event_dbg("%s: Exit %zu", __func__, count);

	return ret;
}

static ssize_t gsi_ctrl_dev_write(struct file *fp, const char __user *buf,
		size_t count, loff_t *pos)
{
	int ret = 0;
	unsigned long flags;
	struct gsi_ctrl_pkt *cpkt;
	struct f_gsi *gsi;
	struct gsi_ctrl_port *c_port = container_of(fp->private_data,
						struct gsi_ctrl_port,
						ctrl_device);

	if (!c_port) {
		pr_err_ratelimited("%s: gsi ctrl port NULL", __func__);
		return -ENODEV;
	}

	if (!c_port->notify_req) {
		pr_err_ratelimited("%s: gsi ctrl port notify_req NULL",
				__func__);
		return -ENODEV;
	}

	if (!c_port->notify_req->buf) {
		pr_err_ratelimited("%s: gsi ctrl port notify_req->buf",
				__func__);
		return -ENODEV;
	}

	gsi = c_port_to_gsi(c_port);

	log_event_dbg("%s: Enter %zu", __func__, count);

	if (!count || count > GSI_MAX_CTRL_PKT_SIZE) {
		log_event_err("error: ctrl pkt length %zu", count);
		return -EINVAL;
	}

	if (!atomic_read(&gsi->connected)) {
		log_event_err("USB cable not connected\n");
		return -ECONNRESET;
	}

	if (gsi->function.func_is_suspended &&
			!gsi->function.func_wakeup_allowed) {
		c_port->cpkt_drop_cnt++;
		log_event_err("drop ctrl pkt of len %zu", count);
		return -ENOTSUPP;
	}

	cpkt = gsi_ctrl_pkt_alloc(count, GFP_KERNEL);
	if (IS_ERR(cpkt)) {
		log_event_err("failed to allocate ctrl pkt");
		return -ENOMEM;
	}

	ret = copy_from_user(cpkt->buf, buf, count);
	if (ret) {
		log_event_err("copy_from_user failed err:%d", ret);
		gsi_ctrl_pkt_free(cpkt);
		return ret;
	}
	cpkt->type = GSI_CTRL_NOTIFY_RESPONSE_AVAILABLE;
	c_port->copied_from_modem++;
	if (qti_packet_debug)
		print_hex_dump(KERN_DEBUG, "WRITE:", DUMP_PREFIX_OFFSET, 16, 1,
			cpkt->buf, min_t(int, 30, count), false);

	spin_lock_irqsave(&c_port->lock, flags);
	list_add_tail(&cpkt->list, &c_port->cpkt_resp_q);
	spin_unlock_irqrestore(&c_port->lock, flags);

	if (!gsi_ctrl_send_notification(gsi))
		c_port->modem_to_host++;

	log_event_dbg("Exit %zu", count);

	return ret ? ret : count;
}

static long gsi_ctrl_dev_ioctl(struct file *fp, unsigned int cmd,
		unsigned long arg)
{
	struct gsi_ctrl_port *c_port = container_of(fp->private_data,
						struct gsi_ctrl_port,
						ctrl_device);
	struct f_gsi *gsi;
	struct gsi_ctrl_pkt *cpkt;
	struct ep_info info;
	int val, ret = 0;
	unsigned long flags;

	if (!c_port) {
		pr_err_ratelimited("%s: gsi ctrl port NULL", __func__);
		return -ENODEV;
	}

	gsi = c_port_to_gsi(c_port);

	switch (cmd) {
	case QTI_CTRL_MODEM_OFFLINE:
		if (gsi->prot_id == USB_PROT_DIAG_IPA ||
				gsi->prot_id == USB_PROT_DPL_ETHER) {
			log_event_dbg("%s:Modem Offline not handled", __func__);
			goto exit_ioctl;
		}
		atomic_set(&c_port->ctrl_online, 0);
		gsi_ctrl_clear_cpkt_queues(gsi, true);
		cpkt = gsi_ctrl_pkt_alloc(0, GFP_KERNEL);
		if (IS_ERR(cpkt)) {
			log_event_err("%s: err allocating cpkt\n", __func__);
			return -ENOMEM;
		}
		cpkt->type = GSI_CTRL_NOTIFY_OFFLINE;
		spin_lock_irqsave(&c_port->lock, flags);
		list_add_tail(&cpkt->list, &c_port->cpkt_resp_q);
		spin_unlock_irqrestore(&c_port->lock, flags);
		gsi_ctrl_send_notification(gsi);
		break;
	case QTI_CTRL_MODEM_ONLINE:
		if (gsi->prot_id == USB_PROT_DIAG_IPA ||
				gsi->prot_id == USB_PROT_DPL_ETHER) {
			log_event_dbg("%s:Modem Online not handled", __func__);
			goto exit_ioctl;
		}

		atomic_set(&c_port->ctrl_online, 1);
		break;
	case QTI_CTRL_GET_LINE_STATE:
		val = atomic_read(&gsi->connected);
		if (gsi->prot_id == USB_PROT_RMNET_IPA ||
				gsi->prot_id == USB_PROT_RMNET_ETHER)
			val = gsi->rmnet_dtr_status;

		ret = copy_to_user((void __user *)arg, &val, sizeof(val));
		if (ret) {
			log_event_err("copy_to_user fail LINE_STATE");
			ret = -EFAULT;
		}
		log_event_dbg("%s: Sent line_state: %d for prot id:%d",
				__func__, val, gsi->prot_id);
		break;
	case QTI_CTRL_EP_LOOKUP:
	case GSI_MBIM_EP_LOOKUP:
		log_event_dbg("%s: EP_LOOKUP for prot id:%d", __func__,
							gsi->prot_id);
		if (!atomic_read(&gsi->connected)) {
			log_event_dbg("EP_LOOKUP failed: not connected");
			ret = -EAGAIN;
			break;
		}

		if ((gsi->prot_id == USB_PROT_DIAG_IPA ||
				gsi->prot_id == USB_PROT_DPL_ETHER) &&
				(gsi->d_port.in_channel_handle == -EINVAL)) {
			ret = -EAGAIN;
			break;
		}

		if (gsi->prot_id != USB_PROT_GPS_CTRL) {
			if (gsi->d_port.in_channel_handle == -EINVAL &&
				gsi->d_port.out_channel_handle == -EINVAL) {
				ret = -EAGAIN;
				break;
			}
			info.ph_ep_info.ep_type = GSI_MBIM_DATA_EP_TYPE_HSUSB;
			info.ph_ep_info.peripheral_iface_id = gsi->data_id;
		} else {
			info.ph_ep_info.ep_type = GSI_MBIM_DATA_EP_TYPE_HSUSB;
			info.ph_ep_info.peripheral_iface_id = gsi->ctrl_id;
		}

		log_event_dbg("%s: prot id :%d ep_type:%d intf:%d",
				__func__, gsi->prot_id, info.ph_ep_info.ep_type,
				info.ph_ep_info.peripheral_iface_id);
		if (gsi->prot_id != USB_PROT_GPS_CTRL) {
			info.ipa_ep_pair.cons_pipe_num =
			(gsi->prot_id == USB_PROT_DIAG_IPA ||
				gsi->prot_id == USB_PROT_DPL_ETHER) ? -1 :
					gsi->d_port.out_channel_handle;
			info.ipa_ep_pair.prod_pipe_num =
					gsi->d_port.in_channel_handle;


			log_event_dbg("%s: ipa_cons_idx:%d ipa_prod_idx:%d",
					__func__,
					info.ipa_ep_pair.cons_pipe_num,
					info.ipa_ep_pair.prod_pipe_num);
		}

		ret = copy_to_user((void __user *)arg, &info,
			sizeof(info));
		if (ret) {
			log_event_err("copy_to_user fail MBIM");
			ret = -EFAULT;
		}
		break;
	case GSI_MBIM_GET_NTB_SIZE:
		ret = copy_to_user((void __user *)arg,
			&gsi->d_port.ntb_info.ntb_input_size,
			sizeof(gsi->d_port.ntb_info.ntb_input_size));
		if (ret) {
			log_event_err("copy_to_user failNTB_SIZE");
			ret = -EFAULT;
		}
		log_event_dbg("Sent NTB size %d",
				gsi->d_port.ntb_info.ntb_input_size);
		break;
	case GSI_MBIM_GET_DATAGRAM_COUNT:
		ret = copy_to_user((void __user *)arg,
			&gsi->d_port.ntb_info.ntb_max_datagrams,
			sizeof(gsi->d_port.ntb_info.ntb_max_datagrams));
		if (ret) {
			log_event_err("copy_to_user fail DATAGRAM");
			ret = -EFAULT;
		}
		log_event_dbg("Sent NTB datagrams count %d",
			gsi->d_port.ntb_info.ntb_max_datagrams);
		break;
	default:
		log_event_err("wrong parameter");
		ret = -EINVAL;
	}

exit_ioctl:
	return ret;
}

static unsigned int gsi_ctrl_dev_poll(struct file *fp, poll_table *wait)
{
	struct gsi_ctrl_port *c_port = container_of(fp->private_data,
						struct gsi_ctrl_port,
						ctrl_device);
	unsigned long flags;
	unsigned int mask = 0;
	struct f_gsi *gsi;

	if (!c_port) {
		pr_err_ratelimited("%s: gsi ctrl port NULL", __func__);
		return -ENODEV;
	}

	gsi = c_port_to_gsi(c_port);

	poll_wait(fp, &c_port->read_wq, wait);

	spin_lock_irqsave(&c_port->lock, flags);
	if (!list_empty(&c_port->cpkt_req_q)) {
		mask |= POLLIN | POLLRDNORM;
		log_event_dbg("%s sets POLLIN for %s", __func__, c_port->name);
	}
	spin_unlock_irqrestore(&c_port->lock, flags);

	return mask;
}

/* file operations for rmnet/mbim/dpl devices */
static const struct file_operations gsi_ctrl_dev_fops = {
	.owner = THIS_MODULE,
	.open = gsi_ctrl_dev_open,
	.release = gsi_ctrl_dev_release,
	.read = gsi_ctrl_dev_read,
	.write = gsi_ctrl_dev_write,
	.unlocked_ioctl = gsi_ctrl_dev_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = gsi_ctrl_dev_ioctl,
#endif
	.poll = gsi_ctrl_dev_poll,
};

/* peak (theoretical) bulk transfer rate in bits-per-second */
static unsigned int gsi_xfer_bitrate(struct usb_gadget *g)
{
	if (gadget_is_superspeed(g) && g->speed == USB_SPEED_SUPER)
		return 13 * 1024 * 8 * 1000 * 8;
	else if (gadget_is_dualspeed(g) && g->speed == USB_SPEED_HIGH)
		return 13 * 512 * 8 * 1000 * 8;
	else
		return 19 * 64 * 1 * 1000 * 8;
}

static int gsi_function_ctrl_port_init(struct f_gsi *gsi)
{
	int ret;
	char *cdev_name = NULL;
	int sz = GSI_CTRL_NAME_LEN;

	INIT_LIST_HEAD(&gsi->c_port.cpkt_req_q);
	INIT_LIST_HEAD(&gsi->c_port.cpkt_resp_q);

	spin_lock_init(&gsi->c_port.lock);

	init_waitqueue_head(&gsi->c_port.read_wq);

	switch (gsi->prot_id) {
	case USB_PROT_RMNET_IPA:
		cdev_name = GSI_RMNET_CTRL_NAME;
		break;
	case USB_PROT_RMNET_ETHER:
		cdev_name = ETHER_RMNET_CTRL_NAME;
		break;
	case USB_PROT_MBIM_IPA:
		cdev_name = GSI_MBIM_CTRL_NAME;
		break;
	case USB_PROT_DIAG_IPA:
		cdev_name = GSI_DPL_CTRL_NAME;
		break;
	case USB_PROT_DPL_ETHER:
		cdev_name = ETHER_DPL_CTRL_NAME;
		break;
	case USB_PROT_GPS_CTRL:
		cdev_name = GSI_GPS_CTRL_NAME;
		break;
	default:
		break;
	}

	if (!cdev_name)
		return 0;
	else
		strlcat(gsi->c_port.name, cdev_name, sz);

	gsi->c_port.ctrl_device.name = gsi->c_port.name;
	gsi->c_port.ctrl_device.fops = &gsi_ctrl_dev_fops;
	gsi->c_port.ctrl_device.minor = MISC_DYNAMIC_MINOR;

	ret = misc_register(&gsi->c_port.ctrl_device);
	if (ret) {
		log_event_err("%s: misc register failed prot id %d",
				__func__, gsi->prot_id);
		return ret;
	}

	return 0;
}

static struct net_device *gsi_rndis_get_netdev(const char *netname)
{
	struct net_device *net_dev;

	net_dev = dev_get_by_name(&init_net, netname);
	if (!net_dev)
		return ERR_PTR(-EINVAL);

	/*
	 * Decrement net_dev refcount as it was incremented in
	 * dev_get_by_name().
	 */
	dev_put(net_dev);
	return net_dev;
}

static void gsi_rndis_open(struct f_gsi *gsi)
{
	struct usb_composite_dev *cdev = gsi->function.config->cdev;

	log_event_dbg("%s", __func__);

	rndis_set_param_medium(gsi->params, RNDIS_MEDIUM_802_3,
				gsi_xfer_bitrate(cdev->gadget) / 100);
	rndis_signal_connect(gsi->params);
}

static void gsi_rndis_ipa_reset_trigger(struct gsi_data_port *d_port)
{
	unsigned long flags;
	struct f_gsi *gsi = d_port_to_gsi(d_port);

	log_event_dbg("%s: setting net_ready_trigger\n", __func__);
	spin_lock_irqsave(&d_port->lock, flags);
	d_port->net_ready_trigger = false;
	spin_unlock_irqrestore(&d_port->lock, flags);
}

void gsi_rndis_flow_ctrl_enable(bool enable, struct rndis_params *param)
{
	struct f_gsi *gsi = param->v;
	struct gsi_data_port *d_port;

	if (!gsi) {
		pr_err("%s: gsi prot ctx is %pK", __func__, gsi);
		return;
	}

	d_port = &gsi->d_port;
	if (enable) {
		log_event_dbg("%s: posting HOST_NRDY\n", __func__);
		post_event(d_port, EVT_HOST_NRDY);
	} else {
		log_event_dbg("%s: posting HOST_READY\n", __func__);
		post_event(d_port, EVT_HOST_READY);
		/*
		 * If host supports flow control with RNDIS_MSG_INIT then
		 * mark the flag to true. This flag will be used further to
		 * enable the flow control on resume path.
		 */
		gsi->host_supports_flow_control = true;
	}

	queue_work(gsi->d_port.ipa_usb_wq, &gsi->d_port.usb_ipa_w);
}

static int queue_notification_request(struct f_gsi *gsi)
{
	int ret;
	unsigned long flags;

	ret = usb_func_ep_queue(&gsi->function, gsi->c_port.notify,
			   gsi->c_port.notify_req, GFP_ATOMIC);
	if (ret < 0) {
		spin_lock_irqsave(&gsi->c_port.lock, flags);
		gsi->c_port.notify_req_queued = false;
		spin_unlock_irqrestore(&gsi->c_port.lock, flags);
	}

	log_event_dbg("%s: ret:%d req_queued:%d",
		__func__, ret, gsi->c_port.notify_req_queued);

	return ret;
}

static int gsi_ctrl_send_notification(struct f_gsi *gsi)
{
	__le32 *data;
	struct usb_cdc_notification *event;
	struct usb_request *req = gsi->c_port.notify_req;
	struct usb_composite_dev *cdev = gsi->function.config->cdev;
	struct gsi_ctrl_pkt *cpkt;
	unsigned long flags;
	bool del_free_cpkt = false;

	if (!atomic_read(&gsi->connected)) {
		log_event_dbg("%s: cable disconnect", __func__);
		return -ENODEV;
	}

	spin_lock_irqsave(&gsi->c_port.lock, flags);
	if (list_empty(&gsi->c_port.cpkt_resp_q)) {
		spin_unlock_irqrestore(&gsi->c_port.lock, flags);
		log_event_dbg("%s: cpkt_resp_q is empty\n", __func__);
		return 0;
	}

	log_event_dbg("%s: notify_req_queued:%d\n",
		__func__, gsi->c_port.notify_req_queued);

	if (gsi->c_port.notify_req_queued) {
		spin_unlock_irqrestore(&gsi->c_port.lock, flags);
		log_event_dbg("%s: notify_req is already queued.\n", __func__);
		return 0;
	}

	cpkt = list_first_entry(&gsi->c_port.cpkt_resp_q,
			struct gsi_ctrl_pkt, list);
	log_event_dbg("%s: cpkt->type:%d\n", __func__, cpkt->type);

	event = req->buf;

	switch (cpkt->type) {
	case GSI_CTRL_NOTIFY_CONNECT:
		del_free_cpkt = true;
		event->bNotificationType = USB_CDC_NOTIFY_NETWORK_CONNECTION;
		event->wValue = cpu_to_le16(1);
		event->wLength = cpu_to_le16(0);
		break;
	case GSI_CTRL_NOTIFY_SPEED:
		del_free_cpkt = true;
		event->bNotificationType = USB_CDC_NOTIFY_SPEED_CHANGE;
		event->wValue = cpu_to_le16(0);
		event->wLength = cpu_to_le16(8);

		/* SPEED_CHANGE data is up/down speeds in bits/sec */
		data = req->buf + sizeof(*event);
		data[0] = cpu_to_le32(gsi_xfer_bitrate(cdev->gadget));
		data[1] = data[0];

		log_event_dbg("notify speed %d",
				gsi_xfer_bitrate(cdev->gadget));
		break;
	case GSI_CTRL_NOTIFY_OFFLINE:
		del_free_cpkt = true;
		event->bNotificationType = USB_CDC_NOTIFY_NETWORK_CONNECTION;
		event->wValue = cpu_to_le16(0);
		event->wLength = cpu_to_le16(0);
		break;
	case GSI_CTRL_NOTIFY_RESPONSE_AVAILABLE:
		event->bNotificationType = USB_CDC_NOTIFY_RESPONSE_AVAILABLE;
		event->wValue = cpu_to_le16(0);
		event->wLength = cpu_to_le16(0);

		if (gsi->prot_id == USB_PROT_RNDIS_IPA) {
			data = req->buf;
			data[0] = cpu_to_le32(1);
			data[1] = cpu_to_le32(0);
			/*
			 * we need to free dummy packet for RNDIS as sending
			 * notification about response available multiple time,
			 * RNDIS host driver doesn't like. All SEND/GET
			 * ENCAPSULATED response is one-to-one for RNDIS case
			 * and host expects to have below sequence:
			 * ep0: USB_CDC_SEND_ENCAPSULATED_COMMAND
			 * int_ep: device->host: RESPONSE_AVAILABLE
			 * ep0: USB_GET_SEND_ENCAPSULATED_COMMAND
			 * For RMNET case: host ignores multiple notification.
			 */
			del_free_cpkt = true;
		}
		break;
	default:
		spin_unlock_irqrestore(&gsi->c_port.lock, flags);
		log_event_err("%s:unknown notify state", __func__);
		WARN_ON(1);
		return -EINVAL;
	}

	/*
	 * Delete and free cpkt related to non NOTIFY_RESPONSE_AVAILABLE
	 * notification whereas NOTIFY_RESPONSE_AVAILABLE related cpkt is
	 * deleted from USB_CDC_GET_ENCAPSULATED_RESPONSE setup request
	 */
	if (del_free_cpkt) {
		list_del(&cpkt->list);
		gsi_ctrl_pkt_free(cpkt);
	}

	gsi->c_port.notify_req_queued = true;
	spin_unlock_irqrestore(&gsi->c_port.lock, flags);
	log_event_dbg("send Notify type %02x", event->bNotificationType);

	return queue_notification_request(gsi);
}

static void gsi_ctrl_notify_resp_complete(struct usb_ep *ep,
					struct usb_request *req)
{
	struct f_gsi *gsi = req->context;
	struct usb_cdc_notification *event = req->buf;
	int status = req->status;
	unsigned long flags;

	spin_lock_irqsave(&gsi->c_port.lock, flags);
	gsi->c_port.notify_req_queued = false;
	spin_unlock_irqrestore(&gsi->c_port.lock, flags);

	switch (status) {
	case -ECONNRESET:
	case -ESHUTDOWN:
		/* connection gone */
		log_event_dbg("ESHUTDOWN/ECONNRESET, connection gone");
		gsi_ctrl_clear_cpkt_queues(gsi, false);
		gsi_ctrl_send_cpkt_tomodem(gsi, NULL, 0);
		break;
	default:
		log_event_err("Unknown event %02x --> %d",
			event->bNotificationType, req->status);
		/* FALLTHROUGH */
	case 0:
		break;
	}
}

static void gsi_rndis_response_available(void *_rndis)
{
	struct f_gsi *gsi = _rndis;
	struct gsi_ctrl_pkt *cpkt;
	unsigned long flags;

	cpkt = gsi_ctrl_pkt_alloc(0, GFP_ATOMIC);
	if (IS_ERR(cpkt)) {
		log_event_err("%s: err allocating cpkt\n", __func__);
		return;
	}

	cpkt->type = GSI_CTRL_NOTIFY_RESPONSE_AVAILABLE;
	spin_lock_irqsave(&gsi->c_port.lock, flags);
	list_add_tail(&cpkt->list, &gsi->c_port.cpkt_resp_q);
	spin_unlock_irqrestore(&gsi->c_port.lock, flags);
	gsi_ctrl_send_notification(gsi);
}

static void gsi_rndis_command_complete(struct usb_ep *ep,
		struct usb_request *req)
{
	struct f_gsi *gsi = req->context;
	rndis_init_msg_type *buf;
	int status;

	status = rndis_msg_parser(gsi->params, (u8 *) req->buf);
	if (status < 0)
		log_event_err("RNDIS command error %d, %d/%d",
			status, req->actual, req->length);

	buf = (rndis_init_msg_type *)req->buf;
	if (buf->MessageType == RNDIS_MSG_INIT) {
		/* honor host dl aggr size */
		gsi->d_port.in_aggr_size = gsi->params->dl_max_xfer_size;
		log_event_dbg("RNDIS host dl_aggr_size:%d\n",
				gsi->params->dl_max_xfer_size);
	}
}

static void
gsi_ctrl_set_ntb_cmd_complete(struct usb_ep *ep, struct usb_request *req)
{
	/* now for SET_NTB_INPUT_SIZE only */
	unsigned int in_size = 0;
	struct f_gsi *gsi = req->context;
	struct gsi_ntb_info *ntb = NULL;

	log_event_dbg("dev:%pK", gsi);

	req->context = NULL;
	if (req->status || req->actual != req->length) {
		log_event_err("Bad control-OUT transfer");
		goto invalid;
	}

	if (req->length == 4) {
		in_size = get_unaligned_le32(req->buf);
		if (in_size < USB_CDC_NCM_NTB_MIN_IN_SIZE ||
		in_size > le32_to_cpu(mbim_gsi_ntb_parameters.dwNtbInMaxSize))
			goto invalid;
	} else if (req->length == 8) {
		ntb = (struct gsi_ntb_info *)req->buf;
		in_size = get_unaligned_le32(&(ntb->ntb_input_size));
		if (in_size < USB_CDC_NCM_NTB_MIN_IN_SIZE ||
		in_size > le32_to_cpu(mbim_gsi_ntb_parameters.dwNtbInMaxSize))
			goto invalid;

		gsi->d_port.ntb_info.ntb_max_datagrams =
			get_unaligned_le16(&(ntb->ntb_max_datagrams));
	} else {
		goto invalid;
	}

	log_event_dbg("Set NTB INPUT SIZE %d", in_size);

	gsi->d_port.ntb_info.ntb_input_size = in_size;
	return;

invalid:
	log_event_err("Illegal NTB INPUT SIZE %d from host", in_size);
	usb_ep_set_halt(ep);
}

static void gsi_ctrl_cmd_complete(struct usb_ep *ep, struct usb_request *req)
{
	struct f_gsi *gsi = req->context;

	gsi_ctrl_send_cpkt_tomodem(gsi, req->buf, req->actual);
}

static void gsi_ctrl_reset_cmd_complete(struct usb_ep *ep,
		struct usb_request *req)
{
	struct f_gsi *gsi = req->context;

	gsi_ctrl_send_cpkt_tomodem(gsi, req->buf, 0);
}

static void gsi_ctrl_send_response_complete(struct usb_ep *ep,
		struct usb_request *req)
{
	struct f_gsi *gsi = req->context;

	gsi_ctrl_send_notification(gsi);
}

static int
gsi_setup(struct usb_function *f, const struct usb_ctrlrequest *ctrl)
{
	struct f_gsi *gsi = func_to_gsi(f);
	struct usb_composite_dev *cdev = f->config->cdev;
	struct usb_request *req = cdev->req;
	int id, value = -EOPNOTSUPP;
	u16 w_index = le16_to_cpu(ctrl->wIndex);
	u16 w_value = le16_to_cpu(ctrl->wValue);
	u16 w_length = le16_to_cpu(ctrl->wLength);
	struct gsi_ctrl_pkt *cpkt;
	u8 *buf;
	u32 n;
	bool line_state;

	if (!atomic_read(&gsi->connected)) {
		log_event_dbg("usb cable is not connected");
		return -ENOTCONN;
	}

	/* rmnet and dpl does not have ctrl_id */
	if (gsi->ctrl_id == -ENODEV)
		id = gsi->data_id;
	else
		id = gsi->ctrl_id;

	/* composite driver infrastructure handles everything except
	 * CDC class messages; interface activation uses set_alt().
	 */
	switch ((ctrl->bRequestType << 8) | ctrl->bRequest) {
	case ((USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE) << 8)
		| USB_CDC_RESET_FUNCTION:

		log_event_dbg("USB_CDC_RESET_FUNCTION");
		value = 0;
		req->complete = gsi_ctrl_reset_cmd_complete;
		req->context = gsi;
		break;
	case ((USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE) << 8)
			| USB_CDC_SEND_ENCAPSULATED_COMMAND:
		log_event_dbg("USB_CDC_SEND_ENCAPSULATED_COMMAND");

		if (w_value || w_index != id)
			goto invalid;
		/* read the request; process it later */
		value = w_length;
		req->context = gsi;
		if (gsi->prot_id == USB_PROT_RNDIS_IPA)
			req->complete = gsi_rndis_command_complete;
		else
			req->complete = gsi_ctrl_cmd_complete;
		/* later, rndis_response_available() sends a notification */
		break;
	case ((USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE) << 8)
			| USB_CDC_GET_ENCAPSULATED_RESPONSE:
		log_event_dbg("USB_CDC_GET_ENCAPSULATED_RESPONSE");
		if (w_value || w_index != id)
			goto invalid;

		if (gsi->prot_id == USB_PROT_RNDIS_IPA) {
			/* return the result */
			buf = rndis_get_next_response(gsi->params, &n);
			if (buf) {
				memcpy(req->buf, buf, n);
				rndis_free_response(gsi->params, buf);
				value = n;
			}
			break;
		}

		spin_lock(&gsi->c_port.lock);
		if (list_empty(&gsi->c_port.cpkt_resp_q)) {
			log_event_dbg("ctrl resp queue empty");
			spin_unlock(&gsi->c_port.lock);
			break;
		}

		cpkt = list_first_entry(&gsi->c_port.cpkt_resp_q,
					struct gsi_ctrl_pkt, list);
		list_del(&cpkt->list);
		gsi->c_port.get_encap_cnt++;
		spin_unlock(&gsi->c_port.lock);

		value = min_t(unsigned int, w_length, cpkt->len);
		memcpy(req->buf, cpkt->buf, value);
		gsi_ctrl_pkt_free(cpkt);

		req->complete = gsi_ctrl_send_response_complete;
		req->context = gsi;
		log_event_dbg("copied encap_resp %d bytes",
			value);
		break;
	case ((USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE) << 8)
			| USB_CDC_REQ_SET_CONTROL_LINE_STATE:
		line_state = (w_value & GSI_CTRL_DTR ? true : false);
		if (gsi->prot_id == USB_PROT_RMNET_IPA ||
				gsi->prot_id == USB_PROT_RMNET_ETHER)
			gsi->rmnet_dtr_status = line_state;
		log_event_dbg("%s: USB_CDC_REQ_SET_CONTROL_LINE_STATE DTR:%d\n",
						__func__, line_state);
		gsi_ctrl_send_cpkt_tomodem(gsi, NULL, 0);
		value = 0;
		break;
	case ((USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE) << 8)
			| USB_CDC_SET_ETHERNET_PACKET_FILTER:
		/* see 6.2.30: no data, wIndex = interface,
		 * wValue = packet filter bitmap
		 */
		if (w_length != 0 || w_index != id)
			goto invalid;
		log_event_dbg("packet filter %02x", w_value);
		/* REVISIT locking of cdc_filter.  This assumes the UDC
		 * driver won't have a concurrent packet TX irq running on
		 * another CPU; or that if it does, this write is atomic...
		 */
		gsi->d_port.cdc_filter = w_value;
		value = 0;
		break;
	case ((USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE) << 8)
		| USB_CDC_GET_NTB_PARAMETERS:
		log_event_dbg("USB_CDC_GET_NTB_PARAMETERS");

		if (w_length == 0 || w_value != 0 || w_index != id)
			break;

		value = w_length > sizeof(mbim_gsi_ntb_parameters) ?
			sizeof(mbim_gsi_ntb_parameters) : w_length;
		memcpy(req->buf, &mbim_gsi_ntb_parameters, value);
		break;
	case ((USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE) << 8)
		| USB_CDC_GET_NTB_INPUT_SIZE:

		log_event_dbg("USB_CDC_GET_NTB_INPUT_SIZE");

		if (w_length < 4 || w_value != 0 || w_index != id)
			break;

		put_unaligned_le32(gsi->d_port.ntb_info.ntb_input_size,
				req->buf);
		value = 4;
		log_event_dbg("Reply to host INPUT SIZE %d",
			 gsi->d_port.ntb_info.ntb_input_size);
		break;
	case ((USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE) << 8)
		| USB_CDC_SET_NTB_INPUT_SIZE:
		log_event_dbg("USB_CDC_SET_NTB_INPUT_SIZE");

		if (w_length != 4 && w_length != 8) {
			log_event_err("wrong NTB length %d", w_length);
			break;
		}

		if (w_value != 0 || w_index != id)
			break;

		req->complete = gsi_ctrl_set_ntb_cmd_complete;
		req->length = w_length;
		req->context = gsi;

		value = req->length;
		break;
	default:
invalid:
		log_event_err("inval ctrl req%02x.%02x v%04x i%04x l%d",
			ctrl->bRequestType, ctrl->bRequest,
			w_value, w_index, w_length);
	}

	/* respond with data transfer or status phase? */
	if (value >= 0) {
		log_event_dbg("req%02x.%02x v%04x i%04x l%d",
			ctrl->bRequestType, ctrl->bRequest,
			w_value, w_index, w_length);
		req->zero = (value < w_length);
		req->length = value;
		value = usb_ep_queue(cdev->gadget->ep0, req, GFP_ATOMIC);
		if (value < 0)
			log_event_err("response on err %d", value);
	}

	/* device either stalls (value < 0) or reports success */
	return value;
}

/*
 * Because the data interface supports multiple altsettings,
 * function *MUST* implement a get_alt() method.
 */
static int gsi_get_alt(struct usb_function *f, unsigned int intf)
{
	struct f_gsi *gsi = func_to_gsi(f);

	/* RNDIS, RMNET and DPL only support alt 0*/
	if (intf == gsi->ctrl_id || gsi->prot_id == USB_PROT_RNDIS_IPA ||
			gsi->prot_id == USB_PROT_RMNET_IPA ||
			gsi->prot_id == USB_PROT_DIAG_IPA ||
			is_ext_prot_ether(gsi->prot_id))
		return 0;
	else if (intf == gsi->data_id)
		return gsi->data_interface_up;

	return -EINVAL;
}

static int gsi_alloc_trb_buffer(struct f_gsi *gsi)
{
	u32 len_in = 0, len_out = 0;
	int ret = 0;
	struct device *dev;

	log_event_dbg("allocate trb's buffer\n");

	dev = gsi->d_port.gadget->dev.parent;
	if (gsi->d_port.in_ep && !gsi->d_port.in_request.buf_base_addr) {
		log_event_dbg("IN: num_bufs:=%zu, buf_len=%zu\n",
			gsi->d_port.in_request.num_bufs,
			gsi->d_port.in_request.buf_len);

		len_in = gsi->d_port.in_request.buf_len *
				gsi->d_port.in_request.num_bufs;
		gsi->d_port.in_request.buf_base_addr =
			dma_zalloc_coherent(dev->parent,
			len_in, &gsi->d_port.in_request.dma, GFP_KERNEL);
		if (!gsi->d_port.in_request.buf_base_addr) {
			dev_err(&gsi->d_port.gadget->dev,
					"IN buf_base_addr allocate failed %s\n",
					gsi->function.name);
			ret = -ENOMEM;
			goto fail1;
		}

		dma_get_sgtable(dev->parent,
			&gsi->d_port.in_request.sgt_data_buff,
			gsi->d_port.in_request.buf_base_addr,
			gsi->d_port.in_request.dma, len_in);
	}

	if (gsi->d_port.out_ep && !gsi->d_port.out_request.buf_base_addr) {
		log_event_dbg("OUT: num_bufs:=%zu, buf_len=%zu\n",
			gsi->d_port.out_request.num_bufs,
			gsi->d_port.out_request.buf_len);

		len_out = gsi->d_port.out_request.buf_len *
				gsi->d_port.out_request.num_bufs;
		gsi->d_port.out_request.buf_base_addr =
			dma_zalloc_coherent(dev->parent,
			len_out, &gsi->d_port.out_request.dma, GFP_KERNEL);
		if (!gsi->d_port.out_request.buf_base_addr) {
			dev_err(&gsi->d_port.gadget->dev,
					"OUT buf_base_addr allocate failed %s\n",
					gsi->function.name);
			ret = -ENOMEM;
			goto fail;
		}

		dma_get_sgtable(dev->parent,
			&gsi->d_port.out_request.sgt_data_buff,
			gsi->d_port.out_request.buf_base_addr,
			gsi->d_port.out_request.dma, len_out);
	}

	log_event_dbg("finished allocating trb's buffer\n");
	return ret;

fail:
	if (len_in && gsi->d_port.in_request.buf_base_addr) {
		dma_free_coherent(dev->parent, len_in,
				gsi->d_port.in_request.buf_base_addr,
				gsi->d_port.in_request.dma);
		gsi->d_port.in_request.buf_base_addr = NULL;
	}
fail1:
	return ret;
}

static void gsi_free_trb_buffer(struct f_gsi *gsi)
{
	u32 len;

	log_event_dbg("freeing trb's buffer\n");

	if (gsi->d_port.out_ep &&
			gsi->d_port.out_request.buf_base_addr) {
		len = gsi->d_port.out_request.buf_len *
			gsi->d_port.out_request.num_bufs;
		dma_free_coherent(gsi->d_port.gadget->dev.parent->parent, len,
			gsi->d_port.out_request.buf_base_addr,
			gsi->d_port.out_request.dma);
		gsi->d_port.out_request.buf_base_addr = NULL;
		sg_free_table(&gsi->d_port.out_request.sgt_data_buff);
	}

	if (gsi->d_port.in_ep &&
			gsi->d_port.in_request.buf_base_addr) {
		len = gsi->d_port.in_request.buf_len *
			gsi->d_port.in_request.num_bufs;
		dma_free_coherent(gsi->d_port.gadget->dev.parent->parent, len,
			gsi->d_port.in_request.buf_base_addr,
			gsi->d_port.in_request.dma);
		gsi->d_port.in_request.buf_base_addr = NULL;
		sg_free_table(&gsi->d_port.in_request.sgt_data_buff);
	}
}

static int gsi_set_alt(struct usb_function *f, unsigned int intf,
						unsigned int alt)
{
	struct f_gsi	 *gsi = func_to_gsi(f);
	struct usb_composite_dev *cdev = f->config->cdev;
	struct net_device	*net;
	int ret;

	log_event_dbg("intf=%u, alt=%u", intf, alt);

	/* Control interface has only altsetting 0 */
	if (intf == gsi->ctrl_id || gsi->prot_id == USB_PROT_RMNET_IPA ||
				gsi->prot_id == USB_PROT_RMNET_ETHER) {
		if (alt != 0)
			goto fail;

		if (!gsi->c_port.notify)
			goto fail;

		if (gsi->c_port.notify->driver_data) {
			log_event_dbg("reset gsi control %d", intf);
			usb_ep_disable(gsi->c_port.notify);
		}

		ret = config_ep_by_speed(cdev->gadget, f,
					gsi->c_port.notify);
		if (ret) {
			gsi->c_port.notify->desc = NULL;
			log_event_err("Config-fail notify ep %s: err %d",
				gsi->c_port.notify->name, ret);
			goto fail;
		}

		ret = usb_ep_enable(gsi->c_port.notify);
		if (ret) {
			log_event_err("usb ep#%s enable failed, err#%d",
				gsi->c_port.notify->name, ret);
			goto fail;
		}
		gsi->c_port.notify->driver_data = gsi;
	}

	/* Data interface has two altsettings, 0 and 1 */
	if (intf == gsi->data_id) {
		gsi->d_port.net_ready_trigger = false;
		/* for rndis and rmnet alt is always 0 update alt accordingly */
		if (gsi->prot_id == USB_PROT_RNDIS_IPA ||
				gsi->prot_id == USB_PROT_RMNET_IPA ||
				gsi->prot_id == USB_PROT_DIAG_IPA ||
				is_ext_prot_ether(gsi->prot_id))
			alt = 1;

		if (alt > 1)
			goto notify_ep_disable;

		if (gsi->data_interface_up == alt)
			return 0;

		if (gsi->d_port.in_ep && gsi->d_port.in_ep->driver_data)
			gsi->d_port.ntb_info.ntb_input_size =
				MBIM_NTB_DEFAULT_IN_SIZE;
		if (alt == 1) {
			if (gsi->d_port.in_ep && !gsi->d_port.in_ep->desc
				&& config_ep_by_speed(cdev->gadget, f,
					gsi->d_port.in_ep)) {
				gsi->d_port.in_ep->desc = NULL;
				goto notify_ep_disable;
			}

			if (gsi->d_port.out_ep && !gsi->d_port.out_ep->desc
				&& config_ep_by_speed(cdev->gadget, f,
					gsi->d_port.out_ep)) {
				gsi->d_port.out_ep->desc = NULL;
				goto notify_ep_disable;
			}

			/* Configure EPs for GSI */
			if (gsi->d_port.in_ep &&
				gsi->prot_id <= USB_PROT_DIAG_IPA) {
				if (gsi->prot_id == USB_PROT_DIAG_IPA)
					gsi->d_port.in_ep->ep_intr_num = 3;
				else
					gsi->d_port.in_ep->ep_intr_num = 2;
				usb_gsi_ep_op(gsi->d_port.in_ep,
					&gsi->d_port.in_request,
						GSI_EP_OP_CONFIG);
			}

			if (gsi->d_port.out_ep &&
				gsi->prot_id <= USB_PROT_DIAG_IPA) {
				gsi->d_port.out_ep->ep_intr_num = 1;
				usb_gsi_ep_op(gsi->d_port.out_ep,
					&gsi->d_port.out_request,
						GSI_EP_OP_CONFIG);
			}

			gsi->d_port.gadget = cdev->gadget;

			if (is_ext_prot_ether(gsi->prot_id)) {
				net = gether_connect(&gsi->d_port.gether_port);
				if (IS_ERR(net)) {
					pr_err("%s:gether_connect err:%ld\n",
					__func__, PTR_ERR(net));
					goto notify_ep_disable;
				}
				gsi->d_port.gether_port.cdc_filter = 0;
			}

			if (gsi->prot_id == USB_PROT_RNDIS_IPA) {
				gsi_rndis_open(gsi);
				net = gsi_rndis_get_netdev("rndis0");
				if (IS_ERR(net))
					goto notify_ep_disable;

				log_event_dbg("RNDIS RX/TX early activation");
				gsi->d_port.cdc_filter = 0;
				rndis_set_param_dev(gsi->params, net,
						&gsi->d_port.cdc_filter);
			}

			if (gsi->prot_id == USB_PROT_ECM_IPA)
				gsi->d_port.cdc_filter = DEFAULT_FILTER;

			/*
			 * For RNDIS the event is posted from the flow control
			 * handler which is invoked when the host sends the
			 * GEN_CURRENT_PACKET_FILTER message.
			 */
			if (gsi->prot_id != USB_PROT_RNDIS_IPA)
				post_event(&gsi->d_port,
						EVT_CONNECT_IN_PROGRESS);
			queue_work(gsi->d_port.ipa_usb_wq,
					&gsi->d_port.usb_ipa_w);
		}

		if (alt == 0 && ((gsi->d_port.in_ep &&
				!gsi->d_port.in_ep->driver_data) ||
				(gsi->d_port.out_ep &&
				!gsi->d_port.out_ep->driver_data)))
			ipa_disconnect_handler(&gsi->d_port);

		gsi->data_interface_up = alt;
		log_event_dbg("DATA_INTERFACE id = %d, status = %d",
				gsi->data_id, gsi->data_interface_up);
	}

	atomic_set(&gsi->connected, 1);

	/* send 0 len pkt to qti to notify state change */
	if (gsi->prot_id == USB_PROT_DIAG_IPA ||
				gsi->prot_id == USB_PROT_DPL_ETHER)
		gsi_ctrl_send_cpkt_tomodem(gsi, NULL, 0);

	return 0;

notify_ep_disable:
	if (gsi->c_port.notify && gsi->c_port.notify->driver_data)
		usb_ep_disable(gsi->c_port.notify);
fail:
	return -EINVAL;
}

static void gsi_disable(struct usb_function *f)
{
	struct f_gsi *gsi = func_to_gsi(f);

	atomic_set(&gsi->connected, 0);

	del_timer(&gsi->gsi_rw_timer);
	gsi->debugfs_rw_timer_enable = 0;

	if (gsi->prot_id == USB_PROT_RNDIS_IPA)
		rndis_uninit(gsi->params);

	if (gsi->prot_id == USB_PROT_RMNET_IPA ||
				gsi->prot_id == USB_PROT_RMNET_ETHER)
		gsi->rmnet_dtr_status = false;

	/* Disable Control Path */
	if (gsi->c_port.notify &&
		gsi->c_port.notify->driver_data) {
		usb_ep_disable(gsi->c_port.notify);
		gsi->c_port.notify->driver_data = NULL;
	}

	gsi_ctrl_clear_cpkt_queues(gsi, false);
	/* send 0 len pkt to qti/qbi to notify state change */
	gsi_ctrl_send_cpkt_tomodem(gsi, NULL, 0);
	gsi->c_port.notify_req_queued = false;
	/* Disable Data Path  - only if it was initialized already (alt=1) */
	if (!gsi->data_interface_up) {
		log_event_dbg("%s: data intf is closed", __func__);
		return;
	}

	gsi->data_interface_up = false;

	gsi->host_supports_flow_control = false;

	log_event_dbg("%s deactivated", gsi->function.name);

	if (is_ext_prot_ether(gsi->prot_id)) {
		gether_disconnect(&gsi->d_port.gether_port);
		return;
	}

	ipa_disconnect_handler(&gsi->d_port);
	post_event(&gsi->d_port, EVT_DISCONNECTED);
	queue_work(gsi->d_port.ipa_usb_wq, &gsi->d_port.usb_ipa_w);
}

static void gsi_suspend(struct usb_function *f)
{
	bool block_db;
	struct f_gsi *gsi = func_to_gsi(f);

	/* Check if function is already suspended in gsi_func_suspend()
	 * Or func_suspend would have bailed out earlier if func_remote_wakeup
	 * wasn't enabled.
	 */
	if (f->func_is_suspended && (gsi->d_port.sm_state == STATE_SUSPENDED ||
			gsi->d_port.sm_state == STATE_SUSPEND_IN_PROGRESS)) {
		log_event_dbg("%s: func already suspended, return\n", __func__);
		return;
	}

	/*
	 * GPS doesn't use any data interface, hence bail out as there is no
	 * GSI specific handling needed.
	 */
	if (gsi->prot_id == USB_PROT_GPS_CTRL) {
		log_event_dbg("%s: suspend done\n", __func__);
		return;
	}

	block_db = true;
	usb_gsi_ep_op(gsi->d_port.in_ep, (void *)&block_db,
			GSI_EP_OP_SET_CLR_BLOCK_DBL);
	post_event(&gsi->d_port, EVT_SUSPEND);
	queue_work(gsi->d_port.ipa_usb_wq, &gsi->d_port.usb_ipa_w);
	log_event_dbg("gsi suspended");

	/*
	 * If host suspended bus without receiving notification request then
	 * initiate remote-wakeup. As driver won't be able to do it later since
	 * notification request is already queued.
	 */
	if (gsi->c_port.notify_req_queued && usb_gsi_remote_wakeup_allowed(f)) {
		mod_timer(&gsi->gsi_rw_timer, jiffies + msecs_to_jiffies(2000));
		log_event_dbg("%s: pending response, arm rw_timer\n", __func__);
	}
}

static void gsi_resume(struct usb_function *f)
{
	struct f_gsi *gsi = func_to_gsi(f);
	struct usb_composite_dev *cdev = f->config->cdev;

	log_event_dbg("%s", __func__);

	/*
	 * If the function is in USB3 Function Suspend state, resume is
	 * canceled. In this case resume is done by a Function Resume request.
	 */
	if ((cdev->gadget->speed == USB_SPEED_SUPER) &&
		f->func_is_suspended)
		return;

	/* Keep timer enabled if user enabled using debugfs */
	if (!gsi->debugfs_rw_timer_enable)
		del_timer(&gsi->gsi_rw_timer);

	if (gsi->c_port.notify && !gsi->c_port.notify->desc)
		config_ep_by_speed(cdev->gadget, f, gsi->c_port.notify);

	/* Check any pending cpkt, and queue immediately on resume */
	gsi_ctrl_send_notification(gsi);

	if (gsi->prot_id == USB_PROT_GPS_CTRL) {
		log_event_dbg("%s: resume done\n", __func__);
		return;
	}

	/*
	 * Linux host does not send RNDIS_MSG_INIT or non-zero
	 * RNDIS_MESSAGE_PACKET_FILTER after performing bus resume.
	 * Check whether host supports flow_control are not. If yes
	 * Trigger state machine explicitly on resume.
	 */
	if (gsi->prot_id == USB_PROT_RNDIS_IPA &&
			!usb_gsi_remote_wakeup_allowed(f) &&
			gsi->host_supports_flow_control)
		rndis_flow_control(gsi->params, false);

	post_event(&gsi->d_port, EVT_RESUMED);
	queue_work(gsi->d_port.ipa_usb_wq, &gsi->d_port.usb_ipa_w);

	log_event_dbg("%s: completed", __func__);
}

static int gsi_get_status(struct usb_function *f)
{
	unsigned int remote_wakeup_en_status = f->func_wakeup_allowed ? 1 : 0;
	struct f_gsi *gsi = func_to_gsi(f);

	/* Disable function remote wake-up for DPL interface */
	if (gsi->prot_id == USB_PROT_DIAG_IPA)
		return 0;

	return (remote_wakeup_en_status << FUNC_WAKEUP_ENABLE_SHIFT) |
		(1 << FUNC_WAKEUP_CAPABLE_SHIFT);
}

static int gsi_func_suspend(struct usb_function *f, u8 options)
{
	bool func_wakeup_allowed;
	struct f_gsi *gsi = func_to_gsi(f);

	log_event_dbg("func susp %u cmd for %s",
		options, f->name ? f->name : "");

	func_wakeup_allowed =
		((options & FUNC_SUSPEND_OPT_RW_EN_MASK) != 0);

	if (options & FUNC_SUSPEND_OPT_SUSP_MASK) {
		f->func_wakeup_allowed = func_wakeup_allowed;
		if (!f->func_is_suspended) {
			gsi_suspend(f);
			f->func_is_suspended = true;
		}
	} else {
		if (f->func_is_suspended) {
			f->func_is_suspended = false;
			gsi_resume(f);
		}
		f->func_wakeup_allowed = func_wakeup_allowed;
	}

	return 0;
}

static int gsi_update_function_bind_params(struct f_gsi *gsi,
	struct usb_composite_dev *cdev,
	struct gsi_function_bind_info *info)
{
	struct usb_ep *ep;
	struct usb_cdc_notification *event;
	struct usb_function *f = &gsi->function;
	int status;

	if (info->ctrl_str_idx >= 0 && info->ctrl_desc) {
		/* ctrl interface label */
		status = usb_string_id(cdev);
		if (status < 0)
			return status;
		info->string_defs[info->ctrl_str_idx].id = status;
		info->ctrl_desc->iInterface = status;
	}

	if (info->data_str_idx >= 0 && info->data_desc) {
		/* data interface label */
		status = usb_string_id(cdev);
		if (status < 0)
			return status;
		info->string_defs[info->data_str_idx].id = status;
		info->data_desc->iInterface = status;
	}

	if (info->iad_str_idx >= 0 && info->iad_desc) {
		/* IAD iFunction label */
		status = usb_string_id(cdev);
		if (status < 0)
			return status;
		info->string_defs[info->iad_str_idx].id = status;
		info->iad_desc->iFunction = status;
	}

	if (info->mac_str_idx >= 0 && info->cdc_eth_desc) {
		/* IAD iFunction label */
		status = usb_string_id(cdev);
		if (status < 0)
			return status;
		info->string_defs[info->mac_str_idx].id = status;
		info->cdc_eth_desc->iMACAddress = status;
	}

	if (info->ctrl_desc)
		info->ctrl_desc->bInterfaceNumber = gsi->ctrl_id;

	if (info->iad_desc)
		info->iad_desc->bFirstInterface = gsi->ctrl_id;

	if (info->union_desc) {
		info->union_desc->bMasterInterface0 = gsi->ctrl_id;
		info->union_desc->bSlaveInterface0 = gsi->data_id;
	}

	if (info->data_desc)
		info->data_desc->bInterfaceNumber = gsi->data_id;

	if (info->data_nop_desc)
		info->data_nop_desc->bInterfaceNumber = gsi->data_id;

	/* allocate instance-specific endpoints */
	if (info->fs_in_desc && gsi->prot_id <= USB_PROT_DIAG_IPA) {
		ep = usb_ep_autoconfig_by_name(cdev->gadget,
				info->fs_in_desc, info->in_epname);
		if (!ep)
			goto fail;
		gsi->d_port.in_ep = ep;
		msm_ep_config(gsi->d_port.in_ep, NULL);
		ep->driver_data = cdev;	/* claim */
	} else {
		if (info->fs_in_desc) {
			ep = usb_ep_autoconfig(cdev->gadget, info->fs_in_desc);
			if (!ep)
				goto fail;
			gsi->d_port.in_ep = ep;
			ep->driver_data = cdev; /* claim */
		}
	}

	if (info->fs_out_desc && gsi->prot_id <= USB_PROT_DIAG_IPA) {
		ep = usb_ep_autoconfig_by_name(cdev->gadget,
				info->fs_out_desc, info->out_epname);
		if (!ep)
			goto fail;
		gsi->d_port.out_ep = ep;
		msm_ep_config(gsi->d_port.out_ep, NULL);
		ep->driver_data = cdev;	/* claim */
	} else {
		if (info->fs_out_desc) {
			ep = usb_ep_autoconfig(cdev->gadget, info->fs_out_desc);
			if (!ep)
				goto fail;
			gsi->d_port.out_ep = ep;
			ep->driver_data = cdev; /* claim */
		}
	}

	if (info->fs_notify_desc) {
		ep = usb_ep_autoconfig(cdev->gadget, info->fs_notify_desc);
		if (!ep)
			goto fail;
		gsi->c_port.notify = ep;
		ep->driver_data = cdev;	/* claim */

		/* allocate notification request and buffer */
		gsi->c_port.notify_req = usb_ep_alloc_request(ep, GFP_KERNEL);
		if (!gsi->c_port.notify_req)
			goto fail;

		gsi->c_port.notify_req->buf =
			kmalloc(info->notify_buf_len, GFP_KERNEL);
		if (!gsi->c_port.notify_req->buf)
			goto fail;

		gsi->c_port.notify_req->length = info->notify_buf_len;
		gsi->c_port.notify_req->context = gsi;
		gsi->c_port.notify_req->complete =
				gsi_ctrl_notify_resp_complete;
		event = gsi->c_port.notify_req->buf;
		event->bmRequestType = USB_DIR_IN | USB_TYPE_CLASS
				| USB_RECIP_INTERFACE;

		if (gsi->ctrl_id == -ENODEV)
			event->wIndex = cpu_to_le16(gsi->data_id);
		else
			event->wIndex = cpu_to_le16(gsi->ctrl_id);

		event->wLength = cpu_to_le16(0);
	}

	gsi->d_port.in_request.buf_len = info->in_req_buf_len;
	gsi->d_port.in_request.num_bufs = info->in_req_num_buf;
	if (gsi->d_port.out_ep) {
		gsi->d_port.out_request.buf_len = info->out_req_buf_len;
		gsi->d_port.out_request.num_bufs = info->out_req_num_buf;
	}

	/* Initialize event queue */
	spin_lock_init(&gsi->d_port.evt_q.q_lock);
	gsi->d_port.evt_q.head = gsi->d_port.evt_q.tail = MAXQUEUELEN - 1;

	/* copy descriptors, and track endpoint copies */
	f->fs_descriptors = usb_copy_descriptors(info->fs_desc_hdr);
	if (!gsi->function.fs_descriptors)
		goto fail;

	/* support all relevant hardware speeds... we expect that when
	 * hardware is dual speed, all bulk-capable endpoints work at
	 * both speeds
	 */
	if (gadget_is_dualspeed(cdev->gadget)) {
		if (info->fs_in_desc)
			info->hs_in_desc->bEndpointAddress =
					info->fs_in_desc->bEndpointAddress;
		if (info->fs_out_desc)
			info->hs_out_desc->bEndpointAddress =
					info->fs_out_desc->bEndpointAddress;
		if (info->fs_notify_desc)
			info->hs_notify_desc->bEndpointAddress =
					info->fs_notify_desc->bEndpointAddress;

		/* copy descriptors, and track endpoint copies */
		f->hs_descriptors = usb_copy_descriptors(info->hs_desc_hdr);
		if (!f->hs_descriptors)
			goto fail;
	}

	if (gadget_is_superspeed(cdev->gadget)) {
		if (info->fs_in_desc)
			info->ss_in_desc->bEndpointAddress =
					info->fs_in_desc->bEndpointAddress;

		if (info->fs_out_desc)
			info->ss_out_desc->bEndpointAddress =
					info->fs_out_desc->bEndpointAddress;
		if (info->fs_notify_desc)
			info->ss_notify_desc->bEndpointAddress =
					info->fs_notify_desc->bEndpointAddress;

		/* copy descriptors, and track endpoint copies */
		f->ss_descriptors = usb_copy_descriptors(info->ss_desc_hdr);
		if (!f->ss_descriptors)
			goto fail;
	}

	return 0;

fail:
	if (gadget_is_superspeed(cdev->gadget) && f->ss_descriptors)
		usb_free_descriptors(f->ss_descriptors);
	if (gadget_is_dualspeed(cdev->gadget) && f->hs_descriptors)
		usb_free_descriptors(f->hs_descriptors);
	if (f->fs_descriptors)
		usb_free_descriptors(f->fs_descriptors);
	if (gsi->c_port.notify_req) {
		kfree(gsi->c_port.notify_req->buf);
		usb_ep_free_request(gsi->c_port.notify, gsi->c_port.notify_req);
	}
	/* we might as well release our claims on endpoints */
	if (gsi->c_port.notify)
		gsi->c_port.notify->driver_data = NULL;
	if (gsi->d_port.out_ep && gsi->d_port.out_ep->desc)
		gsi->d_port.out_ep->driver_data = NULL;
	if (gsi->d_port.in_ep && gsi->d_port.in_ep->desc)
		gsi->d_port.in_ep->driver_data = NULL;
	log_event_err("%s: bind failed for %s", __func__, f->name);
	return -ENOMEM;
}

static void ipa_ready_callback(void *user_data)
{
	struct f_gsi *gsi = user_data;

	log_event_info("%s: ipa is ready\n", __func__);

	gsi->d_port.ipa_ready = true;
	wake_up_interruptible(&gsi->d_port.wait_for_ipa_ready);
}

static void gsi_get_ether_addr(const char *str, u8 *dev_addr)
{
	if (str) {
		unsigned int i;

		for (i = 0; i < ETH_ALEN; i++) {
			unsigned char num;

			if ((*str == '.') || (*str == ':'))
				str++;
			num = hex_to_bin(*str++) << 4;
			num |= hex_to_bin(*str++);
			dev_addr[i] = num;
		}
		if (is_valid_ether_addr(dev_addr))
			return;
	}
	random_ether_addr(dev_addr);
}

static int gsi_bind(struct usb_configuration *c, struct usb_function *f)
{
	struct usb_composite_dev *cdev = c->cdev;
	struct gsi_function_bind_info info = {0};
	struct f_gsi *gsi = func_to_gsi(f);
	struct rndis_params *params;
	struct net_device *net;
	struct gsi_opts *opts;
	char *name = NULL;
	int status;
	__u8  class;
	__u8  subclass;
	__u8  proto;


	if (gsi->prot_id == USB_PROT_RMNET_IPA ||
		gsi->prot_id == USB_PROT_DIAG_IPA ||
		is_ext_prot_ether(gsi->prot_id))
		gsi->ctrl_id = -ENODEV;
	else {
		status = gsi->ctrl_id = usb_interface_id(c, f);
		if (status < 0)
			goto fail;
	}

	if (gsi->prot_id != USB_PROT_GPS_CTRL) {
		status = gsi->data_id = usb_interface_id(c, f);
		if (status < 0)
			goto fail;
	}

	switch (gsi->prot_id) {
	case USB_PROT_RNDIS_IPA:
		info.string_defs = rndis_gsi_string_defs;
		info.ctrl_desc = &rndis_gsi_control_intf;
		info.ctrl_str_idx = 0;
		info.data_desc = &rndis_gsi_data_intf;
		info.data_str_idx = 1;
		info.iad_desc = &rndis_gsi_iad_descriptor;
		info.iad_str_idx = 2;
		info.union_desc = &rndis_gsi_union_desc;
		info.fs_in_desc = &rndis_gsi_fs_in_desc;
		info.fs_out_desc = &rndis_gsi_fs_out_desc;
		info.fs_notify_desc = &rndis_gsi_fs_notify_desc;
		info.hs_in_desc = &rndis_gsi_hs_in_desc;
		info.hs_out_desc = &rndis_gsi_hs_out_desc;
		info.hs_notify_desc = &rndis_gsi_hs_notify_desc;
		info.ss_in_desc = &rndis_gsi_ss_in_desc;
		info.ss_out_desc = &rndis_gsi_ss_out_desc;
		info.ss_notify_desc = &rndis_gsi_ss_notify_desc;
		info.fs_desc_hdr = gsi_eth_fs_function;
		info.hs_desc_hdr = gsi_eth_hs_function;
		info.ss_desc_hdr = gsi_eth_ss_function;
		info.in_epname = "gsi-epin";
		info.out_epname = "gsi-epout";
		info.in_req_buf_len = GSI_IN_BUFF_SIZE;
		gsi->d_port.in_aggr_size = GSI_IN_RNDIS_AGGR_SIZE;
		info.in_req_num_buf = GSI_NUM_IN_RNDIS_BUFFERS;
		gsi->d_port.out_aggr_size = GSI_OUT_AGGR_SIZE;
		info.out_req_buf_len = GSI_OUT_AGGR_SIZE;
		info.out_req_num_buf = GSI_NUM_OUT_BUFFERS;
		info.notify_buf_len = sizeof(struct usb_cdc_notification);

		params = rndis_register(gsi_rndis_response_available, gsi,
				gsi_rndis_flow_ctrl_enable);
		if (IS_ERR(params))
			goto fail;

		gsi->params = params;

		rndis_set_param_medium(gsi->params, RNDIS_MEDIUM_802_3, 0);

		/* export host's Ethernet address in CDC format */
		gsi_get_ether_addr(gsi_dev_addr,
				   gsi->d_port.ipa_init_params.device_ethaddr);

		gsi_get_ether_addr(gsi_host_addr,
				   gsi->d_port.ipa_init_params.host_ethaddr);

		log_event_dbg("setting host_ethaddr=%pM, device_ethaddr = %pM",
				gsi->d_port.ipa_init_params.host_ethaddr,
				gsi->d_port.ipa_init_params.device_ethaddr);
		memcpy(gsi->ethaddr, &gsi->d_port.ipa_init_params.host_ethaddr,
				ETH_ALEN);
		rndis_set_host_mac(gsi->params, gsi->ethaddr);

		if (gsi->manufacturer && gsi->vendorID &&
			rndis_set_param_vendor(gsi->params, gsi->vendorID,
				gsi->manufacturer))
			goto dereg_rndis;

		log_event_dbg("%s: max_pkt_per_xfer : %d", __func__,
					DEFAULT_MAX_PKT_PER_XFER);
		rndis_set_max_pkt_xfer(gsi->params, DEFAULT_MAX_PKT_PER_XFER);

		/* In case of aggregated packets QC device will request
		 * aliment to 4 (2^2).
		 */
		log_event_dbg("%s: pkt_alignment_factor : %d", __func__,
					DEFAULT_PKT_ALIGNMENT_FACTOR);
		rndis_set_pkt_alignment_factor(gsi->params,
					DEFAULT_PKT_ALIGNMENT_FACTOR);

		/* Windows7/Windows10 automatically loads RNDIS drivers for
		 * class drivers which represents MISC_ACTIVE_SYNC,
		 * MISC_RNDIS_OVER_ETHERNET & WIRELESS_CONTROLLER_REMOTE_NDIS.
		 * All the codes listed below are from
		 * http://www.usb.org/developers/defined_class and its unknown
		 * why windows loads rndis class driver for some of them.
		 * Note that, Windows loads NDIS6 stack automatically for
		 * MISC_RNDIS_OVER_ETHERNET. Windows loads NDIS5 stack for
		 * MISC_ACTIVE_SYNC and WIRELESS_CONTROLLER_REMOTE_NDIS.
		 * For other class codes, NDIS stack can be selected using
		 * customized INF file but that defeats the purpose as its
		 * expected to load drivers automatically for known class
		 * drivers published by usbif.
		 * Linux rndis host driver supports MISC_ACTIVE_SYNC and
		 * WIRELESS_CONTROLLER_REMOTE_NDIS as of now.
		 * Default to rndis over ethernet which loads NDIS6 drivers
		 * for windows7/windows10 to avoid data stall issues
		 */
		if (gsi->rndis_id == RNDIS_ID_UNKNOWN)
			gsi->rndis_id = MISC_RNDIS_OVER_ETHERNET;

		switch (gsi->rndis_id) {
		default:
			/* fall throug */
		case WIRELESS_CONTROLLER_REMOTE_NDIS:
			class = USB_CLASS_WIRELESS_CONTROLLER;
			subclass = 0x01;
			proto = 0x03;
			break;
		case MISC_ACTIVE_SYNC:
			class = USB_CLASS_MISC;
			subclass = 0x01;
			proto = 0x01;
			break;
		case MISC_RNDIS_OVER_ETHERNET:
			class = USB_CLASS_MISC;
			subclass = 0x04;
			proto = 0x01;
			break;
		case MISC_RNDIS_OVER_WIFI:
			class = USB_CLASS_MISC;
			subclass = 0x04;
			proto = 0x02;
			break;
		case MISC_RNDIS_OVER_WIMAX:
			class = USB_CLASS_MISC;
			subclass = 0x04;
			proto = 0x03;
			break;
		case MISC_RNDIS_OVER_WWAN:
			class = USB_CLASS_MISC;
			subclass = 0x04;
			proto = 0x04;
			break;
		case MISC_RNDIS_FOR_IPV4:
			class = USB_CLASS_MISC;
			subclass = 0x04;
			proto = 0x05;
			break;
		case MISC_RNDIS_FOR_IPV6:
			class = USB_CLASS_MISC;
			subclass = 0x04;
			proto = 0x06;
			break;
		case MISC_RNDIS_FOR_GPRS:
			class = USB_CLASS_MISC;
			subclass = 0x04;
			proto = 0x07;
			break;
		}

		info.iad_desc->bFunctionClass = class;
		info.iad_desc->bFunctionSubClass = subclass;
		info.iad_desc->bFunctionProtocol = proto;
		info.ctrl_desc->bInterfaceClass = class;
		info.ctrl_desc->bInterfaceSubClass = subclass;
		info.ctrl_desc->bInterfaceProtocol = proto;

		break;
	case USB_PROT_MBIM_IPA:
		info.string_defs = mbim_gsi_string_defs;
		info.ctrl_desc = &mbim_gsi_control_intf;
		info.ctrl_str_idx = 0;
		info.data_desc = &mbim_gsi_data_intf;
		info.data_str_idx = 1;
		info.data_nop_desc = &mbim_gsi_data_nop_intf;
		info.iad_desc = &mbim_gsi_iad_desc;
		info.iad_str_idx = -1;
		info.union_desc = &mbim_gsi_union_desc;
		info.fs_in_desc = &mbim_gsi_fs_in_desc;
		info.fs_out_desc = &mbim_gsi_fs_out_desc;
		info.fs_notify_desc = &mbim_gsi_fs_notify_desc;
		info.hs_in_desc = &mbim_gsi_hs_in_desc;
		info.hs_out_desc = &mbim_gsi_hs_out_desc;
		info.hs_notify_desc = &mbim_gsi_hs_notify_desc;
		info.ss_in_desc = &mbim_gsi_ss_in_desc;
		info.ss_out_desc = &mbim_gsi_ss_out_desc;
		info.ss_notify_desc = &mbim_gsi_ss_notify_desc;
		info.fs_desc_hdr = mbim_gsi_fs_function;
		info.hs_desc_hdr = mbim_gsi_hs_function;
		info.ss_desc_hdr = mbim_gsi_ss_function;
		info.in_epname = "gsi-epin";
		info.out_epname = "gsi-epout";
		gsi->d_port.in_aggr_size = GSI_IN_MBIM_AGGR_SIZE;
		info.in_req_buf_len = GSI_IN_MBIM_AGGR_SIZE;
		info.in_req_num_buf = GSI_NUM_IN_BUFFERS;
		gsi->d_port.out_aggr_size = GSI_OUT_AGGR_SIZE;
		info.out_req_buf_len = GSI_OUT_MBIM_BUF_LEN;
		info.out_req_num_buf = GSI_NUM_OUT_BUFFERS;
		info.notify_buf_len = sizeof(struct usb_cdc_notification);
		mbim_gsi_desc.wMaxSegmentSize = cpu_to_le16(0x800);

		if (cdev->use_os_string) {
			f->os_desc_table = kzalloc(sizeof(*f->os_desc_table),
						GFP_KERNEL);
			if (!f->os_desc_table)
				return -ENOMEM;
			opts = container_of(f->fi, struct gsi_opts, func_inst);
			f->os_desc_n = 1;
			f->os_desc_table[0].os_desc = &opts->os_desc;
			f->os_desc_table[0].if_id = gsi->data_id;
		}
		break;
	case USB_PROT_RMNET_IPA:
	case USB_PROT_RMNET_ETHER:
		info.string_defs = rmnet_gsi_string_defs;
		info.data_desc = &rmnet_gsi_interface_desc;
		info.data_str_idx = 0;
		info.fs_in_desc = &rmnet_gsi_fs_in_desc;
		info.fs_out_desc = &rmnet_gsi_fs_out_desc;
		info.fs_notify_desc = &rmnet_gsi_fs_notify_desc;
		info.hs_in_desc = &rmnet_gsi_hs_in_desc;
		info.hs_out_desc = &rmnet_gsi_hs_out_desc;
		info.hs_notify_desc = &rmnet_gsi_hs_notify_desc;
		info.ss_in_desc = &rmnet_gsi_ss_in_desc;
		info.ss_out_desc = &rmnet_gsi_ss_out_desc;
		info.ss_notify_desc = &rmnet_gsi_ss_notify_desc;
		info.fs_desc_hdr = rmnet_gsi_fs_function;
		info.hs_desc_hdr = rmnet_gsi_hs_function;
		info.ss_desc_hdr = rmnet_gsi_ss_function;
		info.in_epname = "gsi-epin";
		info.out_epname = "gsi-epout";
		gsi->d_port.in_aggr_size = GSI_IN_RMNET_AGGR_SIZE;
		info.in_req_buf_len = GSI_IN_BUFF_SIZE;
		info.in_req_num_buf = GSI_NUM_IN_BUFFERS;
		gsi->d_port.out_aggr_size = GSI_OUT_AGGR_SIZE;
		info.out_req_buf_len = GSI_OUT_RMNET_BUF_LEN;
		info.out_req_num_buf = GSI_NUM_OUT_BUFFERS;
		info.notify_buf_len = sizeof(struct usb_cdc_notification);
		name = "usb_rmnet";
		break;
	case USB_PROT_ECM_IPA:
		info.string_defs = ecm_gsi_string_defs;
		info.ctrl_desc = &ecm_gsi_control_intf;
		info.ctrl_str_idx = 0;
		info.data_desc = &ecm_gsi_data_intf;
		info.data_str_idx = 2;
		info.data_nop_desc = &ecm_gsi_data_nop_intf;
		info.cdc_eth_desc = &ecm_gsi_desc;
		info.mac_str_idx = 1;
		info.union_desc = &ecm_gsi_union_desc;
		info.fs_in_desc = &ecm_gsi_fs_in_desc;
		info.fs_out_desc = &ecm_gsi_fs_out_desc;
		info.fs_notify_desc = &ecm_gsi_fs_notify_desc;
		info.hs_in_desc = &ecm_gsi_hs_in_desc;
		info.hs_out_desc = &ecm_gsi_hs_out_desc;
		info.hs_notify_desc = &ecm_gsi_hs_notify_desc;
		info.ss_in_desc = &ecm_gsi_ss_in_desc;
		info.ss_out_desc = &ecm_gsi_ss_out_desc;
		info.ss_notify_desc = &ecm_gsi_ss_notify_desc;
		info.fs_desc_hdr = ecm_gsi_fs_function;
		info.hs_desc_hdr = ecm_gsi_hs_function;
		info.ss_desc_hdr = ecm_gsi_ss_function;
		info.in_epname = "gsi-epin";
		info.out_epname = "gsi-epout";
		gsi->d_port.in_aggr_size = GSI_ECM_AGGR_SIZE;
		info.in_req_buf_len = GSI_IN_BUFF_SIZE;
		info.in_req_num_buf = GSI_NUM_IN_BUFFERS;
		gsi->d_port.out_aggr_size = GSI_ECM_AGGR_SIZE;
		info.out_req_buf_len = GSI_OUT_ECM_BUF_LEN;
		info.out_req_num_buf = GSI_NUM_OUT_BUFFERS;
		info.notify_buf_len = GSI_CTRL_NOTIFY_BUFF_LEN;

		/* export host's Ethernet address in CDC format */
		gsi_get_ether_addr(gsi_dev_addr,
				   gsi->d_port.ipa_init_params.device_ethaddr);

		gsi_get_ether_addr(gsi_host_addr,
				   gsi->d_port.ipa_init_params.host_ethaddr);

		log_event_dbg("setting host_ethaddr=%pM, device_ethaddr = %pM",
				gsi->d_port.ipa_init_params.host_ethaddr,
				gsi->d_port.ipa_init_params.device_ethaddr);

		snprintf(gsi->ethaddr, sizeof(gsi->ethaddr),
		"%02X%02X%02X%02X%02X%02X",
		gsi->d_port.ipa_init_params.host_ethaddr[0],
		gsi->d_port.ipa_init_params.host_ethaddr[1],
		gsi->d_port.ipa_init_params.host_ethaddr[2],
		gsi->d_port.ipa_init_params.host_ethaddr[3],
		gsi->d_port.ipa_init_params.host_ethaddr[4],
		gsi->d_port.ipa_init_params.host_ethaddr[5]);
		info.string_defs[1].s = gsi->ethaddr;
		break;
	case USB_PROT_DIAG_IPA:
	case USB_PROT_DPL_ETHER:
		info.string_defs = qdss_gsi_string_defs;
		info.data_desc = &qdss_gsi_data_intf_desc;
		info.data_str_idx = 0;
		info.fs_in_desc = &qdss_gsi_fs_data_desc;
		info.hs_in_desc = &qdss_gsi_hs_data_desc;
		info.ss_in_desc = &qdss_gsi_ss_data_desc;
		info.fs_desc_hdr = qdss_gsi_fs_data_only_desc;
		info.hs_desc_hdr = qdss_gsi_hs_data_only_desc;
		info.ss_desc_hdr = qdss_gsi_ss_data_only_desc;
		info.in_epname = "gsi-epin";
		info.out_epname = "";
		info.in_req_buf_len = 16384;
		info.in_req_num_buf = GSI_NUM_IN_BUFFERS;
		info.notify_buf_len = sizeof(struct usb_cdc_notification);
		name = "dpl_usb";
		break;
	case USB_PROT_GPS_CTRL:
		info.string_defs = gps_string_defs;
		info.ctrl_str_idx = 0;
		info.ctrl_desc = &gps_interface_desc;
		info.fs_notify_desc = &gps_fs_notify_desc;
		info.hs_notify_desc = &gps_hs_notify_desc;
		info.ss_notify_desc = &gps_ss_notify_desc;
		info.fs_desc_hdr = gps_fs_function;
		info.hs_desc_hdr = gps_hs_function;
		info.ss_desc_hdr = gps_ss_function;
		info.notify_buf_len = sizeof(struct usb_cdc_notification);
		break;
	default:
		log_event_err("%s: Invalid prot id %d", __func__,
							gsi->prot_id);
		return -EINVAL;
	}

	status = gsi_update_function_bind_params(gsi, cdev, &info);
	if (status)
		goto dereg_rndis;

	if (gsi->prot_id == USB_PROT_GPS_CTRL)
		goto skip_ipa_init;

	if (is_ext_prot_ether(gsi->prot_id)) {
		if (!name)
			return -EINVAL;

		gsi->d_port.gether_port.in_ep = gsi->d_port.in_ep;
		gsi->d_port.gether_port.out_ep = gsi->d_port.out_ep;
			net = gether_setup_name_default(name);
			if (IS_ERR(net)) {
				pr_err("%s: gether_setup failed\n", __func__);
				return PTR_ERR(net);
			}
			gsi->d_port.gether_port.ioport = netdev_priv(net);
			gether_set_gadget(net, c->cdev->gadget);
			status = gether_register_netdev(net);
			if (status < 0) {
				pr_err("%s: gether_register_netdev failed\n",
					__func__);
				free_netdev(net);
				return status;
			}
		goto skip_ipa_init;
	}

	status = ipa_register_ipa_ready_cb(ipa_ready_callback, gsi);
	if (!status) {
		log_event_info("%s: ipa is not ready", __func__);
		status = wait_event_interruptible_timeout(
			gsi->d_port.wait_for_ipa_ready, gsi->d_port.ipa_ready,
			msecs_to_jiffies(GSI_IPA_READY_TIMEOUT));
		if (!status) {
			log_event_err("%s: ipa ready timeout", __func__);
			status = -ETIMEDOUT;
			goto dereg_rndis;
		}
	}

	gsi->d_port.ipa_usb_notify_cb = ipa_usb_notify_cb;
	status = ipa_usb_init_teth_prot(gsi->prot_id,
		&gsi->d_port.ipa_init_params, gsi->d_port.ipa_usb_notify_cb,
		gsi);
	if (status) {
		log_event_err("%s: failed to init teth prot(%d) with err:%d",
					__func__, gsi->prot_id, status);
		goto dereg_rndis;
	}

	gsi->d_port.sm_state = STATE_INITIALIZED;

skip_ipa_init:
	DBG(cdev, "%s: %s speed IN/%s OUT/%s NOTIFY/%s\n",
			f->name,
			gadget_is_superspeed(c->cdev->gadget) ? "super" :
			gadget_is_dualspeed(c->cdev->gadget) ? "dual" : "full",
			(gsi->d_port.in_ep == NULL ? "NULL" :
					gsi->d_port.in_ep->name),
			(gsi->d_port.out_ep == NULL ? "NULL" :
					gsi->d_port.out_ep->name),
			(gsi->c_port.notify == NULL ? "NULL" :
					gsi->c_port.notify->name));
	return 0;

dereg_rndis:
	rndis_deregister(gsi->params);
	kfree(f->os_desc_table);
fail:
	return status;
}

static void gsi_unbind(struct usb_configuration *c, struct usb_function *f)
{
	struct f_gsi *gsi = func_to_gsi(f);

	if (is_ext_prot_ether(gsi->prot_id)) {
		gether_cleanup(gsi->d_port.gether_port.ioport);
		gsi->d_port.gether_port.ioport = NULL;
		goto skip_ipa_dinit;
	}

	/*
	 * Use drain_workqueue to accomplish below conditions:
	 * 1. Make sure that any running work completed
	 * 2. Make sure to wait until all pending work completed i.e. workqueue
	 * is not having any pending work.
	 * Above conditions are making sure that ipa_usb_deinit_teth_prot()
	 * with ipa driver shall not fail due to unexpected state.
	 */
	drain_workqueue(gsi->d_port.ipa_usb_wq);
	ipa_usb_deinit_teth_prot(gsi->prot_id);

skip_ipa_dinit:
	if (gsi->prot_id == USB_PROT_RNDIS_IPA) {
		gsi->d_port.sm_state = STATE_UNINITIALIZED;
		rndis_deregister(gsi->params);
	}

	if (gsi->prot_id == USB_PROT_MBIM_IPA) {
		kfree(f->os_desc_table);
		f->os_desc_table = NULL;
		f->os_desc_n = 0;
	}

	if (gadget_is_superspeed(c->cdev->gadget)) {
		usb_free_descriptors(f->ss_descriptors);
		f->ss_descriptors = NULL;
	}
	if (gadget_is_dualspeed(c->cdev->gadget)) {
		usb_free_descriptors(f->hs_descriptors);
		f->hs_descriptors = NULL;
	}
	usb_free_descriptors(f->fs_descriptors);
	f->fs_descriptors = NULL;

	if (gsi->c_port.notify) {
		kfree(gsi->c_port.notify_req->buf);
		usb_ep_free_request(gsi->c_port.notify, gsi->c_port.notify_req);
	}
}


static void gsi_free_func(struct usb_function *f)
{
	struct f_gsi *gsi = func_to_gsi(f);

	log_event_dbg("%s\n", __func__);
}

static int gsi_bind_config(struct f_gsi *gsi)
{
	int status = 0;

	log_event_dbg("%s: prot id %d", __func__, gsi->prot_id);

	switch (gsi->prot_id) {
	case USB_PROT_RNDIS_IPA:
		gsi->function.name = "rndis";
		gsi->function.strings = rndis_gsi_strings;
		break;
	case USB_PROT_ECM_IPA:
		gsi->function.name = "cdc_ethernet";
		gsi->function.strings = ecm_gsi_strings;
		break;
	case USB_PROT_RMNET_IPA:
	case USB_PROT_RMNET_ETHER:
		gsi->function.name = "rmnet";
		gsi->function.strings = rmnet_gsi_strings;
		break;
	case USB_PROT_MBIM_IPA:
		gsi->function.name = "mbim";
		gsi->function.strings = mbim_gsi_strings;
		break;
	case USB_PROT_DIAG_IPA:
	case USB_PROT_DPL_ETHER:
		gsi->function.name = "dpl";
		gsi->function.strings = qdss_gsi_strings;
		break;
	case USB_PROT_GPS_CTRL:
		gsi->function.name = "gps";
		gsi->function.strings = gps_strings;
		break;
	default:
		log_event_err("%s: invalid prot id %d", __func__, gsi->prot_id);
		return -EINVAL;
	}

	/* descriptors are per-instance copies */
	gsi->function.bind = gsi_bind;
	gsi->function.unbind = gsi_unbind;
	gsi->function.set_alt = gsi_set_alt;
	gsi->function.get_alt = gsi_get_alt;
	gsi->function.setup = gsi_setup;
	gsi->function.disable = gsi_disable;
	gsi->function.free_func = gsi_free_func;
	gsi->function.suspend = gsi_suspend;
	gsi->function.get_status = gsi_get_status;
	gsi->function.func_suspend = gsi_func_suspend;
	gsi->function.resume = gsi_resume;

	INIT_WORK(&gsi->d_port.usb_ipa_w, ipa_work_handler);

	return status;
}

static struct f_gsi *gsi_function_init(void)
{
	struct f_gsi *gsi;
	int ret = 0;

	gsi = kzalloc(sizeof(*gsi), GFP_KERNEL);
	if (!gsi) {
		ret = -ENOMEM;
		return ERR_PTR(ret);
	}

	spin_lock_init(&gsi->d_port.lock);

	init_waitqueue_head(&gsi->d_port.wait_for_ipa_ready);

	gsi->d_port.in_channel_handle = -EINVAL;
	gsi->d_port.out_channel_handle = -EINVAL;

	gsi->d_port.ipa_usb_wq = ipa_usb_wq;

	gsi->gsi_rw_timer_interval = DEFAULT_RW_TIMER_INTERVAL;
	setup_timer(&gsi->gsi_rw_timer, gsi_rw_timer_func, (unsigned long) gsi);

	return gsi;
}

static void gsi_opts_release(struct config_item *item)
{
	struct gsi_opts *opts = to_gsi_opts(item);
	struct f_gsi *gsi;

	gsi = opts->gsi;
	log_event_dbg("%s: releasing %s instance\n",
			__func__, gsi->function.name);
	usb_put_function_instance(&opts->func_inst);
}

static struct configfs_item_operations gsi_item_ops = {
	.release	= gsi_opts_release,
};

static ssize_t gsi_info_show(struct config_item *item, char *page)
{
	struct ipa_usb_xdci_chan_params *ipa_chnl_params;
	struct ipa_usb_xdci_connect_params *con_pms;
	struct f_gsi *gsi = to_gsi_opts(item)->gsi;
	int ret, j = 0;
	unsigned int len = 0;
	char *buf;

	buf = kzalloc(PAGE_SIZE, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	if (gsi && atomic_read(&gsi->connected)) {
		len += scnprintf(buf + len, PAGE_SIZE - len,
			"Info: Prot_id:%d\n", gsi->prot_id);
		ipa_chnl_params = &gsi->d_port.ipa_in_channel_params;
		con_pms = &gsi->d_port.ipa_conn_pms;
		len += scnprintf(buf + len, PAGE_SIZE - len, "%55s\n",
		"==================================================");
		len += scnprintf(buf + len, PAGE_SIZE - len,
		"%25s %10s\n", "Ctrl Name: ", gsi->c_port.name);
		len += scnprintf(buf + len, PAGE_SIZE - len,
		"%25s %10u\n", "Ctrl Online: ",
				gsi->c_port.ctrl_online.counter);
		len += scnprintf(buf + len, PAGE_SIZE - len,
		"%25s %10u\n", "Ctrl Open: ",
				gsi->c_port.is_open);
		len += scnprintf(buf + len, PAGE_SIZE - len,
		"%25s %10u\n", "Ctrl Host to Modem: ",
				gsi->c_port.host_to_modem);
		len += scnprintf(buf + len, PAGE_SIZE - len,
		"%25s %10u\n", "Ctrl Modem to Host: ",
				gsi->c_port.modem_to_host);
		len += scnprintf(buf + len, PAGE_SIZE - len,
		"%25s %10u\n", "Ctrl Cpd to Modem: ",
				gsi->c_port.copied_to_modem);
		len += scnprintf(buf + len, PAGE_SIZE - len,
		"%25s %10u\n", "Ctrl Cpd From Modem: ",
				gsi->c_port.copied_from_modem);
		len += scnprintf(buf + len, PAGE_SIZE - len,
		"%25s %10u\n", "Ctrl Pkt Drops: ",
				gsi->c_port.cpkt_drop_cnt);
		len += scnprintf(buf + len, PAGE_SIZE - len, "%25s\n",
		"==============");
		len += scnprintf(buf + len, PAGE_SIZE - len,
		"%25s %10u\n", "Protocol ID: ", gsi->prot_id);
		len += scnprintf(buf + len, PAGE_SIZE - len,
		"%25s %10u\n", "SM State: ", gsi->d_port.sm_state);
		len += scnprintf(buf + len, PAGE_SIZE - len,
		"%25s %10u\n", "IN XferRscIndex: ",
				gsi->d_port.in_xfer_rsc_index);
		len += scnprintf(buf + len, PAGE_SIZE - len,
		"%25s %10d\n", "IN Chnl Hdl: ",
				gsi->d_port.in_channel_handle);
		len += scnprintf(buf + len, PAGE_SIZE - len,
		"%25s %10x\n", "IN Chnl Dbl Addr: ",
				gsi->d_port.in_request.db_reg_phs_addr_lsb);
		len += scnprintf(buf + len, PAGE_SIZE - len,
		"%25s %10u\n", "IN TRB Ring Len: ",
				ipa_chnl_params->xfer_ring_len);
		len += scnprintf(buf + len, PAGE_SIZE - len,
		"%25s %10x\n", "IN TRB Base Addr: ", (unsigned int)
			ipa_chnl_params->xfer_ring_base_addr_iova);
		len += scnprintf(buf + len, PAGE_SIZE - len,
		"%25s %10x\n", "GEVENTCNTLO IN Addr: ",
			ipa_chnl_params->gevntcount_low_addr);
		len += scnprintf(buf + len, PAGE_SIZE - len,
		"%25s %10x\n", "DEPCMDLO IN Addr: ",
		ipa_chnl_params->xfer_scratch.depcmd_low_addr);
		len += scnprintf(buf + len, PAGE_SIZE - len,
		"%25s %10x\n", "IN LastTRB Addr Off: ",
		ipa_chnl_params->xfer_scratch.last_trb_addr_iova);
		len += scnprintf(buf + len, PAGE_SIZE - len,
		"%25s %10u\n", "IN Buffer Size: ",
		ipa_chnl_params->xfer_scratch.const_buffer_size);
		len += scnprintf(buf + len, PAGE_SIZE - len,
		"%25s %10u\n", "IN/DL Aggr Size: ",
		con_pms->teth_prot_params.max_xfer_size_bytes_to_host);

		ipa_chnl_params = &gsi->d_port.ipa_out_channel_params;
		len += scnprintf(buf + len, PAGE_SIZE - len, "%25s\n",
		"==============");
		len += scnprintf(buf + len, PAGE_SIZE - len,
		"%25s %10u\n", "OUT XferRscIndex: ",
			gsi->d_port.out_xfer_rsc_index);
		len += scnprintf(buf + len, PAGE_SIZE - len,
		"%25s %10d\n", "OUT Channel Hdl: ",
			gsi->d_port.out_channel_handle);
		len += scnprintf(buf + len, PAGE_SIZE - len,
		"%25s %10x\n", "OUT Channel Dbl Addr: ",
			gsi->d_port.out_request.db_reg_phs_addr_lsb);
		len += scnprintf(buf + len, PAGE_SIZE - len,
		"%25s %10u\n", "OUT TRB Ring Len: ",
			ipa_chnl_params->xfer_ring_len);
		len += scnprintf(buf + len, PAGE_SIZE - len,
		"%25s %10x\n", "OUT TRB Base Addr: ", (unsigned int)
			ipa_chnl_params->xfer_ring_base_addr_iova);
		len += scnprintf(buf + len, PAGE_SIZE - len,
		"%25s %10x\n", "GEVENTCNTLO OUT Addr: ",
			ipa_chnl_params->gevntcount_low_addr);
		len += scnprintf(buf + len, PAGE_SIZE - len,
		"%25s %10x\n", "DEPCMDLO OUT Addr: ",
			ipa_chnl_params->xfer_scratch.depcmd_low_addr);
		len += scnprintf(buf + len, PAGE_SIZE - len,
		"%25s %10x\n", "OUT LastTRB Addr Off: ",
		ipa_chnl_params->xfer_scratch.last_trb_addr_iova);
		len += scnprintf(buf + len, PAGE_SIZE - len,
		"%25s %10u\n", "OUT Buffer Size: ",
		ipa_chnl_params->xfer_scratch.const_buffer_size);
		len += scnprintf(buf + len, PAGE_SIZE - len,
		"%25s %10u\n", "OUT/UL Aggr Size: ",
		con_pms->teth_prot_params.max_xfer_size_bytes_to_dev);
		len += scnprintf(buf + len, PAGE_SIZE - len,
		"%25s %10u\n", "OUT/UL Packets to dev: ",
		con_pms->teth_prot_params.max_packet_number_to_dev);
		len += scnprintf(buf + len, PAGE_SIZE - len,
		"%25s %10u\n", "Net_ready_trigger:",
		gsi->d_port.net_ready_trigger);
		len += scnprintf(buf + len, PAGE_SIZE - len, "%25s\n",
		"USB Bus Events");
		for (j = 0; j < MAXQUEUELEN; j++)
			len += scnprintf(buf + len, PAGE_SIZE - len,
				"%d\t", gsi->d_port.evt_q.event[j]);
		len += scnprintf(buf + len, PAGE_SIZE - len, "\n");
		len += scnprintf(buf + len, PAGE_SIZE - len,
		"%25s %10u\n", "Eventq head: ",
				gsi->d_port.evt_q.head);
		len += scnprintf(buf + len, PAGE_SIZE - len,
		"%25s %10u\n", "Eventq tail: ",
				gsi->d_port.evt_q.tail);
	}

	if (len > PAGE_SIZE)
		len = PAGE_SIZE;

	ret = scnprintf(page, len, buf);

	kfree(buf);

	return ret;
}

CONFIGFS_ATTR_RO(gsi_, info);

static struct configfs_attribute *gsi_attrs[] = {
	&gsi_attr_info,
	NULL,
};

static struct config_item_type gsi_func_type = {
	.ct_item_ops	= &gsi_item_ops,
	.ct_attrs	= gsi_attrs,
	.ct_owner	= THIS_MODULE,
};

static ssize_t gsi_rndis_class_id_show(struct config_item *item, char *page)
{
	struct f_gsi *gsi = to_gsi_opts(item)->gsi;

	return snprintf(page, PAGE_SIZE, "%d\n", gsi->rndis_id);
}

static ssize_t gsi_rndis_class_id_store(struct config_item *item,
			const char *page, size_t len)
{
	struct f_gsi *gsi = to_gsi_opts(item)->gsi;
	u8 id;

	if (kstrtou8(page, 0, &id))
		return -EINVAL;

	if (id > RNDIS_ID_UNKNOWN && id < RNDIS_ID_MAX)
		gsi->rndis_id = id;
	else
		return -EINVAL;

	return len;
}
CONFIGFS_ATTR(gsi_, rndis_class_id);

static struct configfs_attribute *gsi_rndis_attrs[] = {
	&gsi_attr_info,
	&gsi_attr_rndis_class_id,
	NULL,
};

static struct config_item_type gsi_func_rndis_type = {
	.ct_item_ops	= &gsi_item_ops,
	.ct_attrs	= gsi_rndis_attrs,
	.ct_owner	= THIS_MODULE,
};

static int gsi_set_inst_name(struct usb_function_instance *fi,
	const char *name)
{
	int name_len, prot_id, ret = 0;
	struct gsi_opts *opts;
	struct f_gsi *gsi;
	struct usb_os_desc *descs[1];
	char *names[1];

	opts = container_of(fi, struct gsi_opts, func_inst);

	name_len = strlen(name) + 1;
	if (name_len > MAX_INST_NAME_LEN)
		return -ENAMETOOLONG;

	prot_id = name_to_prot_id(name);
	if (prot_id < 0) {
		pr_err("%s: failed to find prot id for %s instance\n",
						__func__, name);
		return -EINVAL;
	}

	if (prot_id == USB_PROT_RNDIS_IPA)
		config_group_init_type_name(&opts->func_inst.group, "",
					    &gsi_func_rndis_type);
	if (prot_id == IPA_USB_MBIM) {
		opts->os_desc.ext_compat_id = opts->ext_compat_id;
		INIT_LIST_HEAD(&opts->os_desc.ext_prop);
		descs[0] = &opts->os_desc;
		names[0] = "MBIM";
		opts->interf_group = usb_os_desc_prepare_interf_dir(
						&opts->func_inst.group, 1,
						descs, names, THIS_MODULE);
	}

	gsi = opts->gsi = __gsi[prot_id];
	opts->gsi->prot_id = prot_id;
	ret = gsi_function_ctrl_port_init(opts->gsi);
	if (ret)
		log_event_err("%s:ctrl port init failed for %s instance\n",
						__func__, name);
	return ret;
}

static void gsi_free_inst(struct usb_function_instance *f)
{
	struct gsi_opts *opts = container_of(f, struct gsi_opts, func_inst);

	if (opts) {
		if (opts->gsi && opts->gsi->c_port.ctrl_device.fops)
			misc_deregister(&opts->gsi->c_port.ctrl_device);
		kfree(opts->interf_group);
	}

	kfree(opts);
}

static struct usb_function_instance *gsi_alloc_inst(void)
{
	struct gsi_opts *opts;

	opts = kzalloc(sizeof(*opts), GFP_KERNEL);
	if (!opts)
		return ERR_PTR(-ENOMEM);

	opts->func_inst.set_inst_name = gsi_set_inst_name;
	opts->func_inst.free_func_inst = gsi_free_inst;
	config_group_init_type_name(&opts->func_inst.group, "",
					&gsi_func_type);

	return &opts->func_inst;
}

static struct usb_function *gsi_alloc(struct usb_function_instance *fi)
{
	struct gsi_opts *opts;
	int ret;

	opts = container_of(fi, struct gsi_opts, func_inst);

	ret = gsi_bind_config(opts->gsi);
	if (ret)
		return ERR_PTR(ret);

	return &opts->gsi->function;
}

DECLARE_USB_FUNCTION(gsi, gsi_alloc_inst, gsi_alloc);
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("GSI function driver");

static int fgsi_init(void)
{
	int i;

	ipa_usb_wq = alloc_workqueue("k_ipa_usb",
				WQ_UNBOUND | WQ_MEM_RECLAIM | WQ_FREEZABLE, 1);
	if (!ipa_usb_wq) {
		pr_err("%s(): Failed to create workqueue\n", __func__);
		return -ENOMEM;
	}

	for (i = 0; i < USB_PROT_MAX; i++) {
		__gsi[i] = gsi_function_init();
		if (IS_ERR(__gsi[i]))
			return PTR_ERR(__gsi[i]);
	}

	ipc_log_ctxt = ipc_log_context_create(NUM_LOG_PAGES, "usb_gsi", 0);
	if (!ipc_log_ctxt)
		pr_err("%s: Err allocating ipc_log_ctxt\n", __func__);

	usb_gsi_debugfs_init();
	return usb_function_register(&gsiusb_func);
}
module_init(fgsi_init);

static void __exit fgsi_exit(void)
{
	int i;

	if (ipa_usb_wq)
		destroy_workqueue(ipa_usb_wq);
	if (ipc_log_ctxt)
		ipc_log_context_destroy(ipc_log_ctxt);

	for (i = 0; i < USB_PROT_MAX; i++)
		kfree(__gsi[i]);

	usb_gsi_debugfs_exit();
	usb_function_unregister(&gsiusb_func);
}
module_exit(fgsi_exit);
