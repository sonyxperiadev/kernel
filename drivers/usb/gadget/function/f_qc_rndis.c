/*
 * f_qc_rndis.c -- RNDIS link function driver
 *
 * Copyright (C) 2003-2005,2008 David Brownell
 * Copyright (C) 2003-2004 Robert Schwebel, Benedikt Spranger
 * Copyright (C) 2008 Nokia Corporation
 * Copyright (C) 2009 Samsung Electronics
 *			Author: Michal Nazarewicz (mina86@mina86.com)
 * Copyright (c) 2012-2017, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* #define VERBOSE_DEBUG */

#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/etherdevice.h>

#include <linux/atomic.h>

#include "u_ether.h"
#include "rndis.h"
#include "u_data_ipa.h"
#include <linux/rndis_ipa.h>
#include "configfs.h"

unsigned int rndis_dl_max_xfer_size = 9216;
module_param(rndis_dl_max_xfer_size, uint, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(rndis_dl_max_xfer_size,
		"Max size of bus transfer to host");

/*
 * This function is an RNDIS Ethernet port -- a Microsoft protocol that's
 * been promoted instead of the standard CDC Ethernet.  The published RNDIS
 * spec is ambiguous, incomplete, and needlessly complex.  Variants such as
 * ActiveSync have even worse status in terms of specification.
 *
 * In short:  it's a protocol controlled by (and for) Microsoft, not for an
 * Open ecosystem or markets.  Linux supports it *only* because Microsoft
 * doesn't support the CDC Ethernet standard.
 *
 * The RNDIS data transfer model is complex, with multiple Ethernet packets
 * per USB message, and out of band data.  The control model is built around
 * what's essentially an "RNDIS RPC" protocol.  It's all wrapped in a CDC ACM
 * (modem, not Ethernet) veneer, with those ACM descriptors being entirely
 * useless (they're ignored).  RNDIS expects to be the only function in its
 * configuration, so it's no real help if you need composite devices; and
 * it expects to be the first configuration too.
 *
 * There is a single technical advantage of RNDIS over CDC Ethernet, if you
 * discount the fluff that its RPC can be made to deliver: it doesn't need
 * a NOP altsetting for the data interface.  That lets it work on some of the
 * "so smart it's stupid" hardware which takes over configuration changes
 * from the software, and adds restrictions like "no altsettings".
 *
 * Unfortunately MSFT's RNDIS drivers are buggy.  They hang or oops, and
 * have all sorts of contrary-to-specification oddities that can prevent
 * them from working sanely.  Since bugfixes (or accurate specs, letting
 * Linux work around those bugs) are unlikely to ever come from MSFT, you
 * may want to avoid using RNDIS on purely operational grounds.
 *
 * Omissions from the RNDIS 1.0 specification include:
 *
 *   - Power management ... references data that's scattered around lots
 *     of other documentation, which is incorrect/incomplete there too.
 *
 *   - There are various undocumented protocol requirements, like the need
 *     to send garbage in some control-OUT messages.
 *
 *   - MS-Windows drivers sometimes emit undocumented requests.
 *
 * This function is based on RNDIS link function driver and
 * contains MSM specific implementation.
 */

struct f_rndis_qc {
	struct usb_function		func;
	u8				ctrl_id, data_id;
	u8				ethaddr[ETH_ALEN];
	u32				vendorID;
	u8				ul_max_pkt_per_xfer;
	u8				pkt_alignment_factor;
	u32				max_pkt_size;
	const char			*manufacturer;
	struct rndis_params		*params;
	atomic_t			ioctl_excl;
	atomic_t			open_excl;

	struct usb_ep			*notify;
	struct usb_request		*notify_req;
	atomic_t			notify_count;
	struct gadget_ipa_port		bam_port;
	u8				port_num;
	u16				cdc_filter;
	bool				net_ready_trigger;
};

static struct ipa_usb_init_params rndis_ipa_params;
static spinlock_t rndis_lock;
static bool rndis_ipa_supported;
static void rndis_qc_open(struct f_rndis_qc *rndis);

static inline struct f_rndis_qc *func_to_rndis_qc(struct usb_function *f)
{
	return container_of(f, struct f_rndis_qc, func);
}

/* peak (theoretical) bulk transfer rate in bits-per-second */
static unsigned int rndis_qc_bitrate(struct usb_gadget *g)
{
	if (gadget_is_superspeed(g) && g->speed == USB_SPEED_SUPER)
		return 13 * 1024 * 8 * 1000 * 8;
	else if (gadget_is_dualspeed(g) && g->speed == USB_SPEED_HIGH)
		return 13 * 512 * 8 * 1000 * 8;
	else
		return 19 * 64 * 1 * 1000 * 8;
}

/*-------------------------------------------------------------------------*/

#define RNDIS_QC_LOG2_STATUS_INTERVAL_MSEC	5	/* 1 << 5 == 32 msec */
#define RNDIS_QC_STATUS_BYTECOUNT		8	/* 8 bytes data */

/* currently only one rndis instance is supported - port
 * index 0.
 */
#define RNDIS_QC_NO_PORTS				1
#define RNDIS_QC_ACTIVE_PORT				0

/* default max packets per tarnsfer value */
#define DEFAULT_MAX_PKT_PER_XFER			15

/* default pkt alignment factor */
#define DEFAULT_PKT_ALIGNMENT_FACTOR			4

#define RNDIS_QC_IOCTL_MAGIC		'i'
#define RNDIS_QC_GET_MAX_PKT_PER_XFER   _IOR(RNDIS_QC_IOCTL_MAGIC, 1, u8)
#define RNDIS_QC_GET_MAX_PKT_SIZE	_IOR(RNDIS_QC_IOCTL_MAGIC, 2, u32)


/* interface descriptor: */

/* interface descriptor: Supports "Wireless" RNDIS; auto-detected by Windows*/
static struct usb_interface_descriptor rndis_qc_control_intf = {
	.bLength =		sizeof(rndis_qc_control_intf),
	.bDescriptorType =	USB_DT_INTERFACE,

	/* .bInterfaceNumber = DYNAMIC */
	/* status endpoint is optional; this could be patched later */
	.bNumEndpoints =	1,
	.bInterfaceClass =	USB_CLASS_WIRELESS_CONTROLLER,
	.bInterfaceSubClass =   0x01,
	.bInterfaceProtocol =   0x03,
	/* .iInterface = DYNAMIC */
};

static struct usb_cdc_header_desc rndis_qc_header_desc = {
	.bLength =		sizeof(rndis_qc_header_desc),
	.bDescriptorType =	USB_DT_CS_INTERFACE,
	.bDescriptorSubType =	USB_CDC_HEADER_TYPE,

	.bcdCDC =		cpu_to_le16(0x0110),
};

static struct usb_cdc_call_mgmt_descriptor rndis_qc_call_mgmt_descriptor = {
	.bLength =		sizeof(rndis_qc_call_mgmt_descriptor),
	.bDescriptorType =	USB_DT_CS_INTERFACE,
	.bDescriptorSubType =	USB_CDC_CALL_MANAGEMENT_TYPE,

	.bmCapabilities =	0x00,
	.bDataInterface =	0x01,
};

static struct usb_cdc_acm_descriptor rndis_qc_acm_descriptor = {
	.bLength =		sizeof(rndis_qc_acm_descriptor),
	.bDescriptorType =	USB_DT_CS_INTERFACE,
	.bDescriptorSubType =	USB_CDC_ACM_TYPE,

	.bmCapabilities =	0x00,
};

static struct usb_cdc_union_desc rndis_qc_union_desc = {
	.bLength =		sizeof(rndis_qc_union_desc),
	.bDescriptorType =	USB_DT_CS_INTERFACE,
	.bDescriptorSubType =	USB_CDC_UNION_TYPE,
	/* .bMasterInterface0 =	DYNAMIC */
	/* .bSlaveInterface0 =	DYNAMIC */
};

/* the data interface has two bulk endpoints */

static struct usb_interface_descriptor rndis_qc_data_intf = {
	.bLength =		sizeof(rndis_qc_data_intf),
	.bDescriptorType =	USB_DT_INTERFACE,

	/* .bInterfaceNumber = DYNAMIC */
	.bNumEndpoints =	2,
	.bInterfaceClass =	USB_CLASS_CDC_DATA,
	.bInterfaceSubClass =	0,
	.bInterfaceProtocol =	0,
	/* .iInterface = DYNAMIC */
};


/*  Supports "Wireless" RNDIS; auto-detected by Windows */
static struct usb_interface_assoc_descriptor
rndis_qc_iad_descriptor = {
	.bLength =		sizeof(rndis_qc_iad_descriptor),
	.bDescriptorType =	USB_DT_INTERFACE_ASSOCIATION,
	.bFirstInterface =	0, /* XXX, hardcoded */
	.bInterfaceCount =	2, /* control + data */
	.bFunctionClass =	USB_CLASS_WIRELESS_CONTROLLER,
	.bFunctionSubClass =	0x01,
	.bFunctionProtocol =	0x03,
	/* .iFunction = DYNAMIC */
};

/* full speed support: */

static struct usb_endpoint_descriptor rndis_qc_fs_notify_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_INT,
	.wMaxPacketSize =	cpu_to_le16(RNDIS_QC_STATUS_BYTECOUNT),
	.bInterval =		1 << RNDIS_QC_LOG2_STATUS_INTERVAL_MSEC,
};

static struct usb_endpoint_descriptor rndis_qc_fs_in_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
};

static struct usb_endpoint_descriptor rndis_qc_fs_out_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	.bEndpointAddress =	USB_DIR_OUT,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
};

static struct usb_descriptor_header *eth_qc_fs_function[] = {
	(struct usb_descriptor_header *) &rndis_qc_iad_descriptor,
	/* control interface matches ACM, not Ethernet */
	(struct usb_descriptor_header *) &rndis_qc_control_intf,
	(struct usb_descriptor_header *) &rndis_qc_header_desc,
	(struct usb_descriptor_header *) &rndis_qc_call_mgmt_descriptor,
	(struct usb_descriptor_header *) &rndis_qc_acm_descriptor,
	(struct usb_descriptor_header *) &rndis_qc_union_desc,
	(struct usb_descriptor_header *) &rndis_qc_fs_notify_desc,
	/* data interface has no altsetting */
	(struct usb_descriptor_header *) &rndis_qc_data_intf,
	(struct usb_descriptor_header *) &rndis_qc_fs_in_desc,
	(struct usb_descriptor_header *) &rndis_qc_fs_out_desc,
	NULL,
};

/* high speed support: */

static struct usb_endpoint_descriptor rndis_qc_hs_notify_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_INT,
	.wMaxPacketSize =	cpu_to_le16(RNDIS_QC_STATUS_BYTECOUNT),
	.bInterval =		RNDIS_QC_LOG2_STATUS_INTERVAL_MSEC + 4,
};
static struct usb_endpoint_descriptor rndis_qc_hs_in_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	cpu_to_le16(512),
};

static struct usb_endpoint_descriptor rndis_qc_hs_out_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	.bEndpointAddress =	USB_DIR_OUT,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	cpu_to_le16(512),
};

static struct usb_descriptor_header *eth_qc_hs_function[] = {
	(struct usb_descriptor_header *) &rndis_qc_iad_descriptor,
	/* control interface matches ACM, not Ethernet */
	(struct usb_descriptor_header *) &rndis_qc_control_intf,
	(struct usb_descriptor_header *) &rndis_qc_header_desc,
	(struct usb_descriptor_header *) &rndis_qc_call_mgmt_descriptor,
	(struct usb_descriptor_header *) &rndis_qc_acm_descriptor,
	(struct usb_descriptor_header *) &rndis_qc_union_desc,
	(struct usb_descriptor_header *) &rndis_qc_hs_notify_desc,
	/* data interface has no altsetting */
	(struct usb_descriptor_header *) &rndis_qc_data_intf,
	(struct usb_descriptor_header *) &rndis_qc_hs_in_desc,
	(struct usb_descriptor_header *) &rndis_qc_hs_out_desc,
	NULL,
};

/* super speed support: */

static struct usb_endpoint_descriptor rndis_qc_ss_notify_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_INT,
	.wMaxPacketSize =	cpu_to_le16(RNDIS_QC_STATUS_BYTECOUNT),
	.bInterval =		RNDIS_QC_LOG2_STATUS_INTERVAL_MSEC + 4,
};

static struct usb_ss_ep_comp_descriptor ss_intr_comp_desc = {
	.bLength =		sizeof(ss_intr_comp_desc),
	.bDescriptorType =	USB_DT_SS_ENDPOINT_COMP,

	/* the following 3 values can be tweaked if necessary */
	/* .bMaxBurst =		0, */
	/* .bmAttributes =	0, */
	.wBytesPerInterval =	cpu_to_le16(RNDIS_QC_STATUS_BYTECOUNT),
};

static struct usb_ss_ep_comp_descriptor rndis_qc_ss_intr_comp_desc = {
	.bLength =		sizeof(ss_intr_comp_desc),
	.bDescriptorType =	USB_DT_SS_ENDPOINT_COMP,

	/* the following 3 values can be tweaked if necessary */
	/* .bMaxBurst =		0, */
	/* .bmAttributes =	0, */
	.wBytesPerInterval =	cpu_to_le16(RNDIS_QC_STATUS_BYTECOUNT),
};

static struct usb_ss_ep_comp_descriptor ss_bulk_comp_desc = {
	.bLength =		sizeof(ss_bulk_comp_desc),
	.bDescriptorType =	USB_DT_SS_ENDPOINT_COMP,

	/* the following 2 values can be tweaked if necessary */
	/* .bMaxBurst =		0, */
	/* .bmAttributes =	0, */
};

static struct usb_endpoint_descriptor rndis_qc_ss_in_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	cpu_to_le16(1024),
};

static struct usb_endpoint_descriptor rndis_qc_ss_out_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	.bEndpointAddress =	USB_DIR_OUT,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	cpu_to_le16(1024),
};

static struct usb_ss_ep_comp_descriptor rndis_qc_ss_bulk_comp_desc = {
	.bLength =		sizeof(ss_bulk_comp_desc),
	.bDescriptorType =	USB_DT_SS_ENDPOINT_COMP,

	/* the following 2 values can be tweaked if necessary */
	/* .bMaxBurst =		0, */
	/* .bmAttributes =	0, */
};

static struct usb_descriptor_header *eth_qc_ss_function[] = {
	(struct usb_descriptor_header *) &rndis_qc_iad_descriptor,

	/* control interface matches ACM, not Ethernet */
	(struct usb_descriptor_header *) &rndis_qc_control_intf,
	(struct usb_descriptor_header *) &rndis_qc_header_desc,
	(struct usb_descriptor_header *) &rndis_qc_call_mgmt_descriptor,
	(struct usb_descriptor_header *) &rndis_qc_acm_descriptor,
	(struct usb_descriptor_header *) &rndis_qc_union_desc,
	(struct usb_descriptor_header *) &rndis_qc_ss_notify_desc,
	(struct usb_descriptor_header *) &rndis_qc_ss_intr_comp_desc,

	/* data interface has no altsetting */
	(struct usb_descriptor_header *) &rndis_qc_data_intf,
	(struct usb_descriptor_header *) &rndis_qc_ss_in_desc,
	(struct usb_descriptor_header *) &rndis_qc_ss_bulk_comp_desc,
	(struct usb_descriptor_header *) &rndis_qc_ss_out_desc,
	(struct usb_descriptor_header *) &rndis_qc_ss_bulk_comp_desc,
	NULL,
};

/* string descriptors: */

static struct usb_string rndis_qc_string_defs[] = {
	[0].s = "RNDIS Communications Control",
	[1].s = "RNDIS Ethernet Data",
	[2].s = "RNDIS",
	{  } /* end of list */
};

static struct usb_gadget_strings rndis_qc_string_table = {
	.language =		0x0409,	/* en-us */
	.strings =		rndis_qc_string_defs,
};

static struct usb_gadget_strings *rndis_qc_strings[] = {
	&rndis_qc_string_table,
	NULL,
};

struct f_rndis_qc *_rndis_qc;

static inline int rndis_qc_lock(atomic_t *excl)
{
	if (atomic_inc_return(excl) == 1)
		return 0;

	atomic_dec(excl);
	return -EBUSY;
}

static inline void rndis_qc_unlock(atomic_t *excl)
{
	atomic_dec(excl);
}

/*-------------------------------------------------------------------------*/

static void rndis_qc_response_available(void *_rndis)
{
	struct f_rndis_qc			*rndis = _rndis;
	struct usb_request		*req = rndis->notify_req;
	__le32				*data = req->buf;
	int				status;

	if (atomic_inc_return(&rndis->notify_count) != 1)
		return;

	if (!rndis->notify->driver_data)
		return;

	/* Send RNDIS RESPONSE_AVAILABLE notification; a
	 * USB_CDC_NOTIFY_RESPONSE_AVAILABLE "should" work too
	 *
	 * This is the only notification defined by RNDIS.
	 */
	data[0] = cpu_to_le32(1);
	data[1] = cpu_to_le32(0);

	status = usb_ep_queue(rndis->notify, req, GFP_ATOMIC);
	if (status) {
		atomic_dec(&rndis->notify_count);
		pr_info("notify/0 --> %d\n", status);
	}
}

static void rndis_qc_response_complete(struct usb_ep *ep,
					struct usb_request *req)
{
	struct f_rndis_qc		*rndis;
	int				status = req->status;
	struct usb_composite_dev	*cdev;
	struct usb_ep *notify_ep;

	spin_lock(&rndis_lock);
	rndis = _rndis_qc;
	if (!rndis || !rndis->notify || !rndis->notify->driver_data) {
		spin_unlock(&rndis_lock);
		return;
	}

	if (!rndis->func.config || !rndis->func.config->cdev) {
		pr_err("%s(): cdev or config is NULL.\n", __func__);
		spin_unlock(&rndis_lock);
		return;
	}

	cdev = rndis->func.config->cdev;

	/* after TX:
	 *  - USB_CDC_GET_ENCAPSULATED_RESPONSE (ep0/control)
	 *  - RNDIS_RESPONSE_AVAILABLE (status/irq)
	 */
	switch (status) {
	case -ECONNRESET:
	case -ESHUTDOWN:
		/* connection gone */
		atomic_set(&rndis->notify_count, 0);
		goto out;
	default:
		pr_info("RNDIS %s response error %d, %d/%d\n",
			ep->name, status,
			req->actual, req->length);
		/* FALLTHROUGH */
	case 0:
		if (ep != rndis->notify)
			goto out;

		/* handle multiple pending RNDIS_RESPONSE_AVAILABLE
		 * notifications by resending until we're done
		 */
		if (atomic_dec_and_test(&rndis->notify_count))
			goto out;
		notify_ep = rndis->notify;
		spin_unlock(&rndis_lock);
		status = usb_ep_queue(notify_ep, req, GFP_ATOMIC);
		if (status) {
			spin_lock(&rndis_lock);
			if (!_rndis_qc)
				goto out;
			atomic_dec(&_rndis_qc->notify_count);
			DBG(cdev, "notify/1 --> %d\n", status);
			spin_unlock(&rndis_lock);
		}
	}

	return;

out:
	spin_unlock(&rndis_lock);
}

static void rndis_qc_command_complete(struct usb_ep *ep,
							struct usb_request *req)
{
	struct f_rndis_qc		*rndis;
	int				status;
	rndis_init_msg_type		*buf;
	u32		ul_max_xfer_size, dl_max_xfer_size;

	if (req->status != 0) {
		pr_err("%s: RNDIS command completion error %d\n",
				__func__, req->status);
		return;
	}

	spin_lock(&rndis_lock);
	rndis = _rndis_qc;
	if (!rndis || !rndis->notify || !rndis->notify->driver_data) {
		spin_unlock(&rndis_lock);
		return;
	}

	/* received RNDIS command from USB_CDC_SEND_ENCAPSULATED_COMMAND */
	status = rndis_msg_parser(rndis->params, (u8 *) req->buf);
	if (status < 0)
		pr_err("RNDIS command error %d, %d/%d\n",
			status, req->actual, req->length);

	buf = (rndis_init_msg_type *)req->buf;

	if (buf->MessageType == RNDIS_MSG_INIT) {
		ul_max_xfer_size = rndis_get_ul_max_xfer_size(rndis->params);
		ipa_data_set_ul_max_xfer_size(ul_max_xfer_size);
		/*
		 * For consistent data throughput from IPA, it is required to
		 * fine tune aggregation byte limit as 7KB. RNDIS IPA driver
		 * use provided this value to calculate aggregation byte limit
		 * and program IPA hardware for aggregation.
		 * Host provides 8KB or 16KB as Max Transfer size, hence select
		 * minimum out of host provided value and optimum transfer size
		 * to get 7KB as aggregation byte limit.
		 */
		if (rndis_dl_max_xfer_size)
			dl_max_xfer_size = min_t(u32, rndis_dl_max_xfer_size,
				rndis_get_dl_max_xfer_size(rndis->params));
		else
			dl_max_xfer_size =
				rndis_get_dl_max_xfer_size(rndis->params);
		ipa_data_set_dl_max_xfer_size(dl_max_xfer_size);
	}
	spin_unlock(&rndis_lock);
}

static int
rndis_qc_setup(struct usb_function *f, const struct usb_ctrlrequest *ctrl)
{
	struct f_rndis_qc		*rndis = func_to_rndis_qc(f);
	struct usb_composite_dev *cdev = f->config->cdev;
	struct usb_request	*req = cdev->req;
	int			value = -EOPNOTSUPP;
	u16			w_index = le16_to_cpu(ctrl->wIndex);
	u16			w_value = le16_to_cpu(ctrl->wValue);
	u16			w_length = le16_to_cpu(ctrl->wLength);

	/* composite driver infrastructure handles everything except
	 * CDC class messages; interface activation uses set_alt().
	 */
	pr_debug("%s: Enter\n", __func__);
	switch ((ctrl->bRequestType << 8) | ctrl->bRequest) {

	/* RNDIS uses the CDC command encapsulation mechanism to implement
	 * an RPC scheme, with much getting/setting of attributes by OID.
	 */
	case ((USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE) << 8)
			| USB_CDC_SEND_ENCAPSULATED_COMMAND:
		if (w_value || w_index != rndis->ctrl_id)
			goto invalid;
		/* read the request; process it later */
		value = w_length;
		req->complete = rndis_qc_command_complete;
		/* later, rndis_response_available() sends a notification */
		break;

	case ((USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE) << 8)
			| USB_CDC_GET_ENCAPSULATED_RESPONSE:
		if (w_value || w_index != rndis->ctrl_id)
			goto invalid;
		else {
			u8 *buf;
			u32 n;

			/* return the result */
			buf = rndis_get_next_response(rndis->params, &n);
			if (buf) {
				memcpy(req->buf, buf, n);
				req->complete = rndis_qc_response_complete;
				rndis_free_response(rndis->params, buf);
				value = n;
			}
			/* else stalls ... spec says to avoid that */
		}
		break;

	default:
invalid:
		VDBG(cdev, "invalid control req%02x.%02x v%04x i%04x l%d\n",
			ctrl->bRequestType, ctrl->bRequest,
			w_value, w_index, w_length);
	}

	/* respond with data transfer or status phase? */
	if (value >= 0) {
		DBG(cdev, "rndis req%02x.%02x v%04x i%04x l%d\n",
			ctrl->bRequestType, ctrl->bRequest,
			w_value, w_index, w_length);
		req->context = rndis;
		req->zero = (value < w_length);
		req->length = value;
		value = usb_ep_queue(cdev->gadget->ep0, req, GFP_ATOMIC);
		if (value < 0)
			pr_err("rndis response on err %d\n", value);
	}

	/* device either stalls (value < 0) or reports success */
	return value;
}

struct net_device *rndis_qc_get_net(const char *netname)
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

static int rndis_qc_set_alt(struct usb_function *f, unsigned intf, unsigned alt)
{
	struct f_rndis_qc	 *rndis = func_to_rndis_qc(f);
	struct f_rndis_qc_opts *opts;
	struct usb_composite_dev *cdev = f->config->cdev;
	u8 src_connection_idx;
	u8 dst_connection_idx;
	enum usb_ctrl usb_bam_type;
	int ret;

	/* we know alt == 0 */

	opts = container_of(f->fi, struct f_rndis_qc_opts, func_inst);
	if (intf == rndis->ctrl_id) {
		if (rndis->notify->driver_data) {
			VDBG(cdev, "reset rndis control %d\n", intf);
			usb_ep_disable(rndis->notify);
		}
		if (!rndis->notify->desc) {
			VDBG(cdev, "init rndis ctrl %d\n", intf);
			if (config_ep_by_speed(cdev->gadget, f, rndis->notify))
				goto fail;
		}
		usb_ep_enable(rndis->notify);
		rndis->notify->driver_data = rndis;

	} else if (intf == rndis->data_id) {
		struct net_device	*net;

		rndis->net_ready_trigger = false;
		if (rndis->bam_port.in->driver_data) {
			DBG(cdev, "reset rndis\n");
			/* bam_port is needed for disconnecting the BAM data
			 * path. Only after the BAM data path is disconnected,
			 * we can disconnect the port from the network layer.
			 */
			ipa_data_disconnect(&rndis->bam_port,
						USB_IPA_FUNC_RNDIS);
		}

		if (!rndis->bam_port.in->desc || !rndis->bam_port.out->desc) {
			DBG(cdev, "init rndis\n");
			if (config_ep_by_speed(cdev->gadget, f,
					       rndis->bam_port.in) ||
			    config_ep_by_speed(cdev->gadget, f,
					       rndis->bam_port.out)) {
				rndis->bam_port.in->desc = NULL;
				rndis->bam_port.out->desc = NULL;
				goto fail;
			}
		}

		/* RNDIS should be in the "RNDIS uninitialized" state,
		 * either never activated or after rndis_uninit().
		 *
		 * We don't want data to flow here until a nonzero packet
		 * filter is set, at which point it enters "RNDIS data
		 * initialized" state ... but we do want the endpoints
		 * to be activated.  It's a strange little state.
		 *
		 * REVISIT the RNDIS gadget code has done this wrong for a
		 * very long time.  We need another call to the link layer
		 * code -- gether_updown(...bool) maybe -- to do it right.
		 */
		rndis->cdc_filter = 0;

		rndis->bam_port.cdev = cdev;
		rndis->bam_port.func = &rndis->func;
		ipa_data_port_select(USB_IPA_FUNC_RNDIS);
		usb_bam_type = usb_bam_get_bam_type(cdev->gadget->name);

		src_connection_idx = usb_bam_get_connection_idx(usb_bam_type,
			IPA_P_BAM, USB_TO_PEER_PERIPHERAL, USB_BAM_DEVICE,
			rndis->port_num);
		dst_connection_idx = usb_bam_get_connection_idx(usb_bam_type,
			IPA_P_BAM, PEER_PERIPHERAL_TO_USB, USB_BAM_DEVICE,
			rndis->port_num);
		if (src_connection_idx < 0 || dst_connection_idx < 0) {
			pr_err("%s: usb_bam_get_connection_idx failed\n",
				__func__);
			return ret;
		}
		if (ipa_data_connect(&rndis->bam_port, USB_IPA_FUNC_RNDIS,
				src_connection_idx, dst_connection_idx))
			goto fail;

		DBG(cdev, "RNDIS RX/TX early activation ...\n");
		rndis_qc_open(rndis);
		net = rndis_qc_get_net("rndis0");
		if (IS_ERR(net))
			return PTR_ERR(net);
		opts->net = net;

		rndis_set_param_dev(rndis->params, net,
				&rndis->cdc_filter);
	} else
		goto fail;

	return 0;
fail:
	return -EINVAL;
}

static void rndis_qc_disable(struct usb_function *f)
{
	struct f_rndis_qc		*rndis = func_to_rndis_qc(f);
	struct usb_composite_dev *cdev = f->config->cdev;
	unsigned long flags;

	if (!rndis->notify->driver_data)
		return;

	DBG(cdev, "rndis deactivated\n");

	spin_lock_irqsave(&rndis_lock, flags);
	rndis_uninit(rndis->params);
	spin_unlock_irqrestore(&rndis_lock, flags);
	ipa_data_disconnect(&rndis->bam_port, USB_IPA_FUNC_RNDIS);

	msm_ep_unconfig(rndis->bam_port.out);
	msm_ep_unconfig(rndis->bam_port.in);
	usb_ep_disable(rndis->notify);
	rndis->notify->driver_data = NULL;
}

static void rndis_qc_suspend(struct usb_function *f)
{
	struct f_rndis_qc	*rndis = func_to_rndis_qc(f);
	bool remote_wakeup_allowed;

	if (f->config->cdev->gadget->speed == USB_SPEED_SUPER)
		remote_wakeup_allowed = f->func_wakeup_allowed;
	else
		remote_wakeup_allowed = f->config->cdev->gadget->remote_wakeup;

	pr_info("%s(): start rndis suspend: remote_wakeup_allowed:%d\n:",
					__func__, remote_wakeup_allowed);

	if (!remote_wakeup_allowed) {
		/* This is required as Linux host side RNDIS driver doesn't
		 * send RNDIS_MESSAGE_PACKET_FILTER before suspending USB bus.
		 * Hence we perform same operations explicitly here for Linux
		 * host case. In case of windows, this RNDIS state machine is
		 * already updated due to receiving of PACKET_FILTER.
		 */
		rndis_flow_control(rndis->params, true);
		pr_debug("%s(): Disconnecting\n", __func__);
	}

	ipa_data_suspend(&rndis->bam_port, USB_IPA_FUNC_RNDIS,
			remote_wakeup_allowed);
	pr_debug("rndis suspended\n");
}

static void rndis_qc_resume(struct usb_function *f)
{
	struct f_rndis_qc	*rndis = func_to_rndis_qc(f);
	bool remote_wakeup_allowed;

	pr_debug("%s: rndis resumed\n", __func__);

	/* Nothing to do if DATA interface wasn't initialized */
	if (!rndis->bam_port.cdev) {
		pr_debug("data interface was not up\n");
		return;
	}

	if (f->config->cdev->gadget->speed == USB_SPEED_SUPER)
		remote_wakeup_allowed = f->func_wakeup_allowed;
	else
		remote_wakeup_allowed = f->config->cdev->gadget->remote_wakeup;

	ipa_data_resume(&rndis->bam_port, USB_IPA_FUNC_RNDIS,
				remote_wakeup_allowed);

	if (!remote_wakeup_allowed) {
		rndis_qc_open(rndis);
		/*
		 * Linux Host doesn't sends RNDIS_MSG_INIT or non-zero value
		 * set with RNDIS_MESSAGE_PACKET_FILTER after performing bus
		 * resume. Hence trigger USB IPA transfer functionality
		 * explicitly here. For Windows host case is also being
		 * handle with RNDIS state machine.
		 */
		rndis_flow_control(rndis->params, false);
	}

	pr_debug("%s: RNDIS resume completed\n", __func__);
}

/*-------------------------------------------------------------------------*/

/*
 * This isn't quite the same mechanism as CDC Ethernet, since the
 * notification scheme passes less data, but the same set of link
 * states must be tested.  A key difference is that altsettings are
 * not used to tell whether the link should send packets or not.
 */

static void rndis_qc_open(struct f_rndis_qc *rndis)
{
	struct usb_composite_dev *cdev = rndis->func.config->cdev;

	DBG(cdev, "%s\n", __func__);

	rndis_set_param_medium(rndis->params, RNDIS_MEDIUM_802_3,
				rndis_qc_bitrate(cdev->gadget) / 100);
	rndis_signal_connect(rndis->params);
}

void ipa_data_flow_control_enable(bool enable, struct rndis_params *param)
{
	if (enable)
		ipa_data_stop_rndis_ipa(USB_IPA_FUNC_RNDIS);
	else
		ipa_data_start_rndis_ipa(USB_IPA_FUNC_RNDIS);
}

/*-------------------------------------------------------------------------*/

/* ethernet function driver setup/binding */

static int
rndis_qc_bind(struct usb_configuration *c, struct usb_function *f)
{
	struct usb_composite_dev *cdev = c->cdev;
	struct f_rndis_qc		*rndis = func_to_rndis_qc(f);
	struct rndis_params		*params;
	int			status;
	struct usb_ep		*ep;

	/* maybe allocate device-global string IDs */
	if (rndis_qc_string_defs[0].id == 0) {

		/* control interface label */
		status = usb_string_id(c->cdev);
		if (status < 0)
		return status;
		rndis_qc_string_defs[0].id = status;
		rndis_qc_control_intf.iInterface = status;

		/* data interface label */
		status = usb_string_id(c->cdev);
		if (status < 0)
			return status;
		rndis_qc_string_defs[1].id = status;
		rndis_qc_data_intf.iInterface = status;

		/* IAD iFunction label */
		status = usb_string_id(c->cdev);
		if (status < 0)
			return status;
		rndis_qc_string_defs[2].id = status;
		rndis_qc_iad_descriptor.iFunction = status;
	}

	/* allocate instance-specific interface IDs */
	status = usb_interface_id(c, f);
	if (status < 0)
		goto fail;
	rndis->ctrl_id = status;
	rndis_qc_iad_descriptor.bFirstInterface = status;

	rndis_qc_control_intf.bInterfaceNumber = status;
	rndis_qc_union_desc.bMasterInterface0 = status;

	status = usb_interface_id(c, f);
	if (status < 0)
		goto fail;
	rndis->data_id = status;

	rndis_qc_data_intf.bInterfaceNumber = status;
	rndis_qc_union_desc.bSlaveInterface0 = status;

	status = -ENODEV;

	/* allocate instance-specific endpoints */
	ep = usb_ep_autoconfig(cdev->gadget, &rndis_qc_fs_in_desc);
	if (!ep)
		goto fail;
	rndis->bam_port.in = ep;
	ep->driver_data = cdev;	/* claim */

	ep = usb_ep_autoconfig(cdev->gadget, &rndis_qc_fs_out_desc);
	if (!ep)
		goto fail;
	rndis->bam_port.out = ep;
	ep->driver_data = cdev;	/* claim */

	/* NOTE:  a status/notification endpoint is, strictly speaking,
	 * optional.  We don't treat it that way though!  It's simpler,
	 * and some newer profiles don't treat it as optional.
	 */
	ep = usb_ep_autoconfig(cdev->gadget, &rndis_qc_fs_notify_desc);
	if (!ep)
		goto fail;
	rndis->notify = ep;
	ep->driver_data = cdev;	/* claim */

	status = -ENOMEM;

	/* allocate notification request and buffer */
	rndis->notify_req = usb_ep_alloc_request(ep, GFP_KERNEL);
	if (!rndis->notify_req)
		goto fail;
	rndis->notify_req->buf = kmalloc(RNDIS_QC_STATUS_BYTECOUNT, GFP_KERNEL);
	if (!rndis->notify_req->buf)
		goto fail;
	rndis->notify_req->length = RNDIS_QC_STATUS_BYTECOUNT;
	rndis->notify_req->context = rndis;
	rndis->notify_req->complete = rndis_qc_response_complete;

	/* copy descriptors, and track endpoint copies */
	f->fs_descriptors = usb_copy_descriptors(eth_qc_fs_function);
	if (!f->fs_descriptors)
		goto fail;

	/* support all relevant hardware speeds... we expect that when
	 * hardware is dual speed, all bulk-capable endpoints work at
	 * both speeds
	 */
	if (gadget_is_dualspeed(c->cdev->gadget)) {
		rndis_qc_hs_in_desc.bEndpointAddress =
				rndis_qc_fs_in_desc.bEndpointAddress;
		rndis_qc_hs_out_desc.bEndpointAddress =
				rndis_qc_fs_out_desc.bEndpointAddress;
		rndis_qc_hs_notify_desc.bEndpointAddress =
				rndis_qc_fs_notify_desc.bEndpointAddress;

		/* copy descriptors, and track endpoint copies */
		f->hs_descriptors = usb_copy_descriptors(eth_qc_hs_function);

		if (!f->hs_descriptors)
			goto fail;
	}

	if (gadget_is_superspeed(c->cdev->gadget)) {
		rndis_qc_ss_in_desc.bEndpointAddress =
				rndis_qc_fs_in_desc.bEndpointAddress;
		rndis_qc_ss_out_desc.bEndpointAddress =
				rndis_qc_fs_out_desc.bEndpointAddress;
		rndis_qc_ss_notify_desc.bEndpointAddress =
				rndis_qc_fs_notify_desc.bEndpointAddress;

		/* copy descriptors, and track endpoint copies */
		f->ss_descriptors = usb_copy_descriptors(eth_qc_ss_function);
		if (!f->ss_descriptors)
			goto fail;
	}

	params = rndis_register(rndis_qc_response_available, rndis,
			ipa_data_flow_control_enable);
	if (params < 0)
		goto fail;
	rndis->params = params;

	rndis_set_param_medium(rndis->params, RNDIS_MEDIUM_802_3, 0);
	rndis_set_host_mac(rndis->params, rndis->ethaddr);

	if (rndis->manufacturer && rndis->vendorID &&
		rndis_set_param_vendor(rndis->params, rndis->vendorID,
			rndis->manufacturer))
		goto fail;

	pr_debug("%s(): max_pkt_per_xfer:%d\n", __func__,
				rndis->ul_max_pkt_per_xfer);
	rndis_set_max_pkt_xfer(rndis->params, rndis->ul_max_pkt_per_xfer);

	/* In case of aggregated packets QC device will request
	 * aliment to 4 (2^2).
	 */
	pr_debug("%s(): pkt_alignment_factor:%d\n", __func__,
				rndis->pkt_alignment_factor);
	rndis_set_pkt_alignment_factor(rndis->params,
				rndis->pkt_alignment_factor);

	/* NOTE:  all that is done without knowing or caring about
	 * the network link ... which is unavailable to this code
	 * until we're activated via set_alt().
	 */

	DBG(cdev, "RNDIS: %s speed IN/%s OUT/%s NOTIFY/%s\n",
			gadget_is_superspeed(c->cdev->gadget) ? "super" :
			gadget_is_dualspeed(c->cdev->gadget) ? "dual" : "full",
			rndis->bam_port.in->name, rndis->bam_port.out->name,
			rndis->notify->name);
	return 0;

fail:
	if (gadget_is_superspeed(c->cdev->gadget) && f->ss_descriptors)
		usb_free_descriptors(f->ss_descriptors);
	if (gadget_is_dualspeed(c->cdev->gadget) && f->hs_descriptors)
		usb_free_descriptors(f->hs_descriptors);
	if (f->fs_descriptors)
		usb_free_descriptors(f->fs_descriptors);

	if (rndis->notify_req) {
		kfree(rndis->notify_req->buf);
		usb_ep_free_request(rndis->notify, rndis->notify_req);
	}

	/* we might as well release our claims on endpoints */
	if (rndis->notify)
		rndis->notify->driver_data = NULL;
	if (rndis->bam_port.out->desc)
		rndis->bam_port.out->driver_data = NULL;
	if (rndis->bam_port.in->desc)
		rndis->bam_port.in->driver_data = NULL;

	pr_err("%s: can't bind, err %d\n", f->name, status);

	return status;
}

static void rndis_qc_free(struct usb_function *f)
{
	struct f_rndis_qc_opts *opts;

	opts = container_of(f->fi, struct f_rndis_qc_opts, func_inst);
	opts->refcnt--;
}

static void
rndis_qc_unbind(struct usb_configuration *c, struct usb_function *f)
{
	struct f_rndis_qc		*rndis = func_to_rndis_qc(f);

	pr_debug("rndis_qc_unbind: free\n");
	rndis_deregister(rndis->params);

	if (gadget_is_dualspeed(c->cdev->gadget))
		usb_free_descriptors(f->hs_descriptors);
	usb_free_descriptors(f->fs_descriptors);

	kfree(rndis->notify_req->buf);
	usb_ep_free_request(rndis->notify, rndis->notify_req);

	/*
	 * call flush_workqueue to make sure that any pending
	 * disconnect_work() from u_bam_data.c file is being
	 * flushed before calling this rndis_ipa_cleanup API
	 * as rndis ipa disconnect API is required to be
	 * called before this.
	 */
	ipa_data_flush_workqueue();
	rndis_ipa_cleanup(rndis_ipa_params.private);
	rndis_ipa_supported = false;

}

void rndis_ipa_reset_trigger(void)
{
	struct f_rndis_qc *rndis;

	rndis = _rndis_qc;
	if (!rndis) {
		pr_err("%s: No RNDIS instance", __func__);
		return;
	}

	rndis->net_ready_trigger = false;
}

/*
 * Callback let RNDIS_IPA trigger us when network interface is up
 * and userspace is ready to answer DHCP requests
 */
void rndis_net_ready_notify(void)
{
	struct f_rndis_qc *rndis;
	unsigned long flags;

	spin_lock_irqsave(&rndis_lock, flags);
	rndis = _rndis_qc;
	if (!rndis) {
		pr_err("%s: No RNDIS instance", __func__);
		spin_unlock_irqrestore(&rndis_lock, flags);
		return;
	}
	if (rndis->net_ready_trigger) {
		pr_err("%s: Already triggered", __func__);
		spin_unlock_irqrestore(&rndis_lock, flags);
		return;
	}

	pr_debug("%s: Set net_ready_trigger", __func__);
	rndis->net_ready_trigger = true;
	spin_unlock_irqrestore(&rndis_lock, flags);
	ipa_data_start_rx_tx(USB_IPA_FUNC_RNDIS);
}

/**
 * rndis_qc_bind_config - add RNDIS network link to a configuration
 * @c: the configuration to support the network link
 * @ethaddr: a buffer in which the ethernet address of the host side
 *	side of the link was recorded
 * Context: single threaded during gadget setup
 *
 * Returns zero on success, else negative errno.
 *
 * Caller must have called @gether_setup().  Caller is also responsible
 * for calling @gether_cleanup() before module unload.
 */

static struct
usb_function *rndis_qc_bind_config_vendor(struct usb_function_instance *fi,
				u32 vendorID, const char *manufacturer,
				u8 max_pkt_per_xfer, u8 pkt_alignment_factor)
{
	struct f_rndis_qc_opts *opts = container_of(fi,
				struct f_rndis_qc_opts, func_inst);
	struct f_rndis_qc	*rndis;
	int		status;

	/* allocate and initialize one new instance */
	status = -ENOMEM;

	opts = container_of(fi, struct f_rndis_qc_opts, func_inst);

	opts->refcnt++;
	rndis = opts->rndis;

	rndis->vendorID = opts->vendor_id;
	rndis->manufacturer = opts->manufacturer;
	/* export host's Ethernet address in CDC format */
	random_ether_addr(rndis_ipa_params.host_ethaddr);
	random_ether_addr(rndis_ipa_params.device_ethaddr);
	pr_debug("setting host_ethaddr=%pM, device_ethaddr=%pM\n",
		rndis_ipa_params.host_ethaddr,
		rndis_ipa_params.device_ethaddr);
	rndis_ipa_supported = true;
	ether_addr_copy(rndis->ethaddr, rndis_ipa_params.host_ethaddr);
	rndis_ipa_params.device_ready_notify = rndis_net_ready_notify;

	/* if max_pkt_per_xfer was not configured set to default value */
	rndis->ul_max_pkt_per_xfer =
			max_pkt_per_xfer ? max_pkt_per_xfer :
			DEFAULT_MAX_PKT_PER_XFER;
	ipa_data_set_ul_max_pkt_num(rndis->ul_max_pkt_per_xfer);

	/*
	 * Check no RNDIS aggregation, and alignment if not mentioned,
	 * use alignment factor as zero. If aggregated RNDIS data transfer,
	 * max packet per transfer would be default if it is not set
	 * explicitly, and same way use alignment factor as 2 by default.
	 * This would eliminate need of writing to sysfs if default RNDIS
	 * aggregation setting required. Writing to both sysfs entries,
	 * those values will always override default values.
	 */
	if ((rndis->pkt_alignment_factor == 0) &&
			(rndis->ul_max_pkt_per_xfer == 1))
		rndis->pkt_alignment_factor = 0;
	else
		rndis->pkt_alignment_factor = pkt_alignment_factor ?
				pkt_alignment_factor :
				DEFAULT_PKT_ALIGNMENT_FACTOR;

	/* RNDIS activates when the host changes this filter */
	rndis->cdc_filter = 0;

	rndis->func.name = "rndis";
	rndis->func.strings = rndis_qc_strings;
	/* descriptors are per-instance copies */
	rndis->func.bind = rndis_qc_bind;
	rndis->func.unbind = rndis_qc_unbind;
	rndis->func.set_alt = rndis_qc_set_alt;
	rndis->func.setup = rndis_qc_setup;
	rndis->func.disable = rndis_qc_disable;
	rndis->func.suspend = rndis_qc_suspend;
	rndis->func.resume = rndis_qc_resume;
	rndis->func.free_func = rndis_qc_free;

	status = rndis_ipa_init(&rndis_ipa_params);
	if (status) {
		pr_err("%s: failed to init rndis_ipa\n", __func__);
		goto fail;
	}

	_rndis_qc = rndis;

	return &rndis->func;
fail:
	kfree(rndis);
	_rndis_qc = NULL;
	return ERR_PTR(status);
}

static struct usb_function *qcrndis_alloc(struct usb_function_instance *fi)
{
	return rndis_qc_bind_config_vendor(fi, 0, NULL, 0, 0);
}

static int rndis_qc_open_dev(struct inode *ip, struct file *fp)
{
	int ret = 0;
	unsigned long flags;
	pr_info("Open rndis QC driver\n");

	spin_lock_irqsave(&rndis_lock, flags);
	if (!_rndis_qc) {
		pr_err("rndis_qc_dev not created yet\n");
		ret = -ENODEV;
		goto fail;
	}

	if (rndis_qc_lock(&_rndis_qc->open_excl)) {
		pr_err("Already opened\n");
		ret = -EBUSY;
		goto fail;
	}

	fp->private_data = _rndis_qc;
fail:
	spin_unlock_irqrestore(&rndis_lock, flags);

	if (!ret)
		pr_info("rndis QC file opened\n");

	return ret;
}

static int rndis_qc_release_dev(struct inode *ip, struct file *fp)
{
	unsigned long flags;
	pr_info("Close rndis QC file\n");

	spin_lock_irqsave(&rndis_lock, flags);

	if (!_rndis_qc) {
		pr_err("rndis_qc_dev not present\n");
		spin_unlock_irqrestore(&rndis_lock, flags);
		return -ENODEV;
	}
	rndis_qc_unlock(&_rndis_qc->open_excl);
	spin_unlock_irqrestore(&rndis_lock, flags);
	return 0;
}

static long rndis_qc_ioctl(struct file *fp, unsigned cmd, unsigned long arg)
{
	u8 qc_max_pkt_per_xfer = 0;
	u32 qc_max_pkt_size = 0;
	int ret = 0;
	unsigned long flags;

	spin_lock_irqsave(&rndis_lock, flags);
	if (!_rndis_qc) {
		pr_err("rndis_qc_dev not present\n");
		ret = -ENODEV;
		goto fail;
	}

	qc_max_pkt_per_xfer = _rndis_qc->ul_max_pkt_per_xfer;
	qc_max_pkt_size = _rndis_qc->max_pkt_size;

	if (rndis_qc_lock(&_rndis_qc->ioctl_excl)) {
		ret = -EBUSY;
		goto fail;
	}

	spin_unlock_irqrestore(&rndis_lock, flags);

	pr_info("Received command %d\n", cmd);

	switch (cmd) {
	case RNDIS_QC_GET_MAX_PKT_PER_XFER:
		ret = copy_to_user((void __user *)arg,
					&qc_max_pkt_per_xfer,
					sizeof(qc_max_pkt_per_xfer));
		if (ret) {
			pr_err("copying to user space failed\n");
			ret = -EFAULT;
		}
		pr_info("Sent UL max packets per xfer %d\n",
				qc_max_pkt_per_xfer);
		break;
	case RNDIS_QC_GET_MAX_PKT_SIZE:
		ret = copy_to_user((void __user *)arg,
					&qc_max_pkt_size,
					sizeof(qc_max_pkt_size));
		if (ret) {
			pr_err("copying to user space failed\n");
			ret = -EFAULT;
		}
		pr_debug("Sent max packet size %d\n",
				qc_max_pkt_size);
		break;
	default:
		pr_err("Unsupported IOCTL\n");
		ret = -EINVAL;
	}

	spin_lock_irqsave(&rndis_lock, flags);

	if (!_rndis_qc) {
		pr_err("rndis_qc_dev not present\n");
		ret = -ENODEV;
		goto fail;
	}

	rndis_qc_unlock(&_rndis_qc->ioctl_excl);

fail:
	spin_unlock_irqrestore(&rndis_lock, flags);
	return ret;
}

static const struct file_operations rndis_qc_fops = {
	.owner = THIS_MODULE,
	.open = rndis_qc_open_dev,
	.release = rndis_qc_release_dev,
	.unlocked_ioctl	= rndis_qc_ioctl,
};

static struct miscdevice rndis_qc_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "android_rndis_qc",
	.fops = &rndis_qc_fops,
};

static void qcrndis_free_inst(struct usb_function_instance *f)
{
	struct f_rndis_qc_opts	*opts = container_of(f,
				struct f_rndis_qc_opts, func_inst);
	unsigned long flags;

	misc_deregister(&rndis_qc_device);

	ipa_data_free(USB_IPA_FUNC_RNDIS);
	spin_lock_irqsave(&rndis_lock, flags);
	kfree(opts->rndis);
	_rndis_qc = NULL;
	kfree(opts);
	spin_unlock_irqrestore(&rndis_lock, flags);
}

static int qcrndis_set_inst_name(struct usb_function_instance *fi,
	const char *name)
{
	struct f_rndis_qc_opts	*opts = container_of(fi,
				struct f_rndis_qc_opts, func_inst);
	struct f_rndis_qc	*rndis;
	int name_len;
	int ret;

	name_len = strlen(name) + 1;
	if (name_len > MAX_INST_NAME_LEN)
		return -ENAMETOOLONG;

	pr_debug("initialize rndis QC instance\n");
	rndis = kzalloc(sizeof(*rndis), GFP_KERNEL);
	if (!rndis) {
		pr_err("%s: fail allocate and initialize new instance\n",
			   __func__);
		return -ENOMEM;
	}

	spin_lock_init(&rndis_lock);
	opts->rndis = rndis;
	ret = misc_register(&rndis_qc_device);
	if (ret)
		pr_err("rndis QC driver failed to register\n");

	ret = ipa_data_setup(USB_IPA_FUNC_RNDIS);
	if (ret) {
		pr_err("bam_data_setup failed err: %d\n", ret);
		kfree(rndis);
		return ret;
	}

	return 0;
}

static inline
struct f_rndis_qc_opts *to_f_qc_rndis_opts(struct config_item *item)
{
	return container_of(to_config_group(item), struct f_rndis_qc_opts,
				func_inst.group);
}

static void qcrndis_attr_release(struct config_item *item)
{
	struct f_rndis_qc_opts *opts = to_f_qc_rndis_opts(item);

	usb_put_function_instance(&opts->func_inst);
}

static struct configfs_item_operations qcrndis_item_ops = {
	.release        = qcrndis_attr_release,
};

static struct config_item_type qcrndis_func_type = {
	.ct_item_ops    = &qcrndis_item_ops,
	.ct_owner       = THIS_MODULE,
};

static struct usb_function_instance *qcrndis_alloc_inst(void)
{
	struct f_rndis_qc_opts *opts;

	opts = kzalloc(sizeof(*opts), GFP_KERNEL);
	if (!opts)
		return ERR_PTR(-ENOMEM);

	opts->func_inst.set_inst_name = qcrndis_set_inst_name;
	opts->func_inst.free_func_inst = qcrndis_free_inst;

	config_group_init_type_name(&opts->func_inst.group, "",
				&qcrndis_func_type);

	return &opts->func_inst;
}

void *rndis_qc_get_ipa_rx_cb(void)
{
	return rndis_ipa_params.ipa_rx_notify;
}

void *rndis_qc_get_ipa_tx_cb(void)
{
	return rndis_ipa_params.ipa_tx_notify;
}

void *rndis_qc_get_ipa_priv(void)
{
	return rndis_ipa_params.private;
}

bool rndis_qc_get_skip_ep_config(void)
{
	return rndis_ipa_params.skip_ep_cfg;
}

DECLARE_USB_FUNCTION_INIT(rndis_bam, qcrndis_alloc_inst, qcrndis_alloc);

static int __init usb_qcrndis_init(void)
{
	int ret;

	ret = usb_function_register(&rndis_bamusb_func);
	if (ret) {
		pr_err("%s: failed to register diag %d\n", __func__, ret);
		return ret;
	}
	return ret;
}

static void __exit usb_qcrndis_exit(void)
{
	usb_function_unregister(&rndis_bamusb_func);
}

module_init(usb_qcrndis_init);
module_exit(usb_qcrndis_exit);
MODULE_DESCRIPTION("USB RMNET Function Driver");
