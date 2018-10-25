/*
 * Copyright (c) 2015-2018, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details
 */

#ifndef _F_GSI_H
#define _F_GSI_H

#include <linux/poll.h>
#include <linux/miscdevice.h>
#include <linux/ipa.h>
#include <uapi/linux/usb/cdc.h>
#include <linux/usb/ch9.h>
#include <linux/usb/composite.h>
#include <linux/usb/gadget.h>
#include <linux/usb/usb_ctrl_qti.h>
#include <linux/etherdevice.h>
#include <linux/debugfs.h>
#include <linux/ipa_usb.h>
#include <linux/ipc_logging.h>
#include <linux/timer.h>

#include "u_ether.h"
#include "configfs.h"

#define GSI_RMNET_CTRL_NAME "rmnet_ctrl"
#define GSI_MBIM_CTRL_NAME "android_mbim"
#define GSI_DPL_CTRL_NAME "dpl_ctrl"
#define ETHER_RMNET_CTRL_NAME "rmnet_ctrl0"
#define ETHER_DPL_CTRL_NAME "dpl_ctrl0"
#define GSI_GPS_CTRL_NAME "gps"

#define GSI_CTRL_NAME_LEN (sizeof(GSI_MBIM_CTRL_NAME)+2)
#define GSI_MAX_CTRL_PKT_SIZE 4096
#define GSI_CTRL_DTR (1 << 0)

#define GSI_NUM_IN_RNDIS_BUFFERS 50
#define GSI_NUM_IN_BUFFERS 15
#define GSI_IN_BUFF_SIZE 2048
#define GSI_NUM_OUT_BUFFERS 14
#define GSI_OUT_AGGR_SIZE 24576

#define GSI_IN_RNDIS_AGGR_SIZE 16384
#define GSI_IN_MBIM_AGGR_SIZE 16384
#define GSI_IN_RMNET_AGGR_SIZE 16384
#define GSI_ECM_AGGR_SIZE 2048

#define GSI_OUT_MBIM_BUF_LEN 16384
#define GSI_OUT_RMNET_BUF_LEN 16384
#define GSI_OUT_ECM_BUF_LEN 2048

#define GSI_IPA_READY_TIMEOUT 5000

#define ETH_ADDR_STR_LEN 14

/* mbin and ecm */
#define GSI_CTRL_NOTIFY_BUFF_LEN 16

/* default max packets per tarnsfer value */
#define DEFAULT_MAX_PKT_PER_XFER 15

/* default pkt alignment factor */
#define DEFAULT_PKT_ALIGNMENT_FACTOR 4

#define GSI_MBIM_IOCTL_MAGIC 'o'
#define GSI_MBIM_GET_NTB_SIZE  _IOR(GSI_MBIM_IOCTL_MAGIC, 2, u32)
#define GSI_MBIM_GET_DATAGRAM_COUNT  _IOR(GSI_MBIM_IOCTL_MAGIC, 3, u16)
#define GSI_MBIM_EP_LOOKUP _IOR(GSI_MBIM_IOCTL_MAGIC, 4, struct ep_info)
#define GSI_MBIM_DATA_EP_TYPE_HSUSB 0x2
/* ID for Microsoft OS String */
#define GSI_MBIM_OS_STRING_ID 0xEE

#define EVT_NONE			0
#define EVT_UNINITIALIZED		1
#define EVT_INITIALIZED			2
#define EVT_CONNECT_IN_PROGRESS		3
#define EVT_CONNECTED			4
#define EVT_HOST_NRDY			5
#define EVT_HOST_READY			6
#define EVT_DISCONNECTED		7
#define	EVT_SUSPEND			8
#define	EVT_IPA_SUSPEND			9
#define	EVT_RESUMED			10

enum connection_state {
	STATE_UNINITIALIZED,
	STATE_INITIALIZED,
	STATE_CONNECT_IN_PROGRESS,
	STATE_CONNECTED,
	STATE_DISCONNECTED,
	STATE_SUSPEND_IN_PROGRESS,
	STATE_SUSPENDED
};

enum gsi_ctrl_notify_state {
	GSI_CTRL_NOTIFY_NONE,
	GSI_CTRL_NOTIFY_CONNECT,
	GSI_CTRL_NOTIFY_SPEED,
	GSI_CTRL_NOTIFY_OFFLINE,
	GSI_CTRL_NOTIFY_RESPONSE_AVAILABLE,
};

enum rndis_class_id {
	RNDIS_ID_UNKNOWN,
	WIRELESS_CONTROLLER_REMOTE_NDIS,
	MISC_ACTIVE_SYNC,
	MISC_RNDIS_OVER_ETHERNET,
	MISC_RNDIS_OVER_WIFI,
	MISC_RNDIS_OVER_WIMAX,
	MISC_RNDIS_OVER_WWAN,
	MISC_RNDIS_FOR_IPV4,
	MISC_RNDIS_FOR_IPV6,
	MISC_RNDIS_FOR_GPRS,
	RNDIS_ID_MAX,
};

enum usb_prot_id {
	/* accelerated: redefined from ipa_usb.h, do not change order */
	USB_PROT_RNDIS_IPA,
	USB_PROT_ECM_IPA,
	USB_PROT_RMNET_IPA,
	USB_PROT_MBIM_IPA,
	USB_PROT_DIAG_IPA,

	/* non-accelerated */
	USB_PROT_RMNET_ETHER,
	USB_PROT_DPL_ETHER,
	USB_PROT_GPS_CTRL,

	USB_PROT_MAX,
};

#define MAXQUEUELEN 128
struct event_queue {
	u8 event[MAXQUEUELEN];
	u8 head, tail;
	spinlock_t q_lock;
};

struct gsi_ntb_info {
	u32	ntb_input_size;
	u16	ntb_max_datagrams;
	u16	reserved;
};

struct gsi_ctrl_pkt {
	void				*buf;
	int				len;
	enum gsi_ctrl_notify_state	type;
	struct list_head		list;
};

struct gsi_function_bind_info {
	struct usb_string *string_defs;
	int ctrl_str_idx;
	int data_str_idx;
	int iad_str_idx;
	int mac_str_idx;
	struct usb_interface_descriptor *ctrl_desc;
	struct usb_interface_descriptor *data_desc;
	struct usb_interface_assoc_descriptor *iad_desc;
	struct usb_cdc_ether_desc *cdc_eth_desc;
	struct usb_cdc_union_desc *union_desc;
	struct usb_interface_descriptor *data_nop_desc;
	struct usb_endpoint_descriptor *fs_in_desc;
	struct usb_endpoint_descriptor *fs_out_desc;
	struct usb_endpoint_descriptor *fs_notify_desc;
	struct usb_endpoint_descriptor *hs_in_desc;
	struct usb_endpoint_descriptor *hs_out_desc;
	struct usb_endpoint_descriptor *hs_notify_desc;
	struct usb_endpoint_descriptor *ss_in_desc;
	struct usb_endpoint_descriptor *ss_out_desc;
	struct usb_endpoint_descriptor *ss_notify_desc;

	struct usb_descriptor_header **fs_desc_hdr;
	struct usb_descriptor_header **hs_desc_hdr;
	struct usb_descriptor_header **ss_desc_hdr;
	const char *in_epname;
	const char *out_epname;

	u32 in_req_buf_len;
	u32 in_req_num_buf;
	u32 out_req_buf_len;
	u32 out_req_num_buf;
	u32 notify_buf_len;
};

struct gsi_ctrl_port {
	char name[GSI_CTRL_NAME_LEN];
	struct miscdevice ctrl_device;

	struct usb_ep *notify;
	struct usb_request *notify_req;
	bool notify_req_queued;

	atomic_t ctrl_online;

	bool is_open;

	wait_queue_head_t read_wq;

	struct list_head cpkt_req_q;
	struct list_head cpkt_resp_q;
	unsigned long cpkts_len;

	spinlock_t lock;

	int ipa_cons_clnt_hdl;
	int ipa_prod_clnt_hdl;

	unsigned int host_to_modem;
	unsigned int copied_to_modem;
	unsigned int copied_from_modem;
	unsigned int modem_to_host;
	unsigned int cpkt_drop_cnt;
	unsigned int get_encap_cnt;
};

struct gsi_data_port {
	struct usb_ep *in_ep;
	struct usb_ep *out_ep;
	struct usb_gsi_request in_request;
	struct usb_gsi_request out_request;
	struct usb_gadget *gadget;
	int (*ipa_usb_notify_cb)(enum ipa_usb_notify_event, void *driver_data);
	struct ipa_usb_teth_params ipa_init_params;
	int in_channel_handle;
	int out_channel_handle;
	u32 in_xfer_rsc_index;
	u32 out_xfer_rsc_index;
	u16 in_last_trb_addr;
	u16 cdc_filter;
	u32 in_aggr_size;
	u32 out_aggr_size;

	bool ipa_ready;
	bool net_ready_trigger;
	struct gsi_ntb_info ntb_info;

	spinlock_t lock;

	struct work_struct usb_ipa_w;
	struct workqueue_struct *ipa_usb_wq;
	enum connection_state sm_state;
	struct event_queue evt_q;
	wait_queue_head_t wait_for_ipa_ready;
	struct gether gether_port;

	/* Track these for debugfs */
	struct ipa_usb_xdci_chan_params ipa_in_channel_params;
	struct ipa_usb_xdci_chan_params ipa_out_channel_params;
	struct ipa_usb_xdci_connect_params ipa_conn_pms;
};

struct f_gsi {
	struct usb_function function;
	enum usb_prot_id prot_id;
	int ctrl_id;
	int data_id;
	u32 vendorID;
	u8 ethaddr[ETH_ADDR_STR_LEN];
	const char *manufacturer;
	struct rndis_params *params;
	atomic_t connected;
	bool data_interface_up;
	enum rndis_class_id rndis_id;

	const struct usb_endpoint_descriptor *in_ep_desc_backup;
	const struct usb_endpoint_descriptor *out_ep_desc_backup;

	struct gsi_data_port d_port;
	struct gsi_ctrl_port c_port;
	bool rmnet_dtr_status;

	/* To test remote wakeup using debugfs */
	struct timer_list gsi_rw_timer;
	u8 debugfs_rw_timer_enable;
	u16 gsi_rw_timer_interval;

	bool host_supports_flow_control;
};

static inline struct f_gsi *func_to_gsi(struct usb_function *f)
{
	return container_of(f, struct f_gsi, function);
}

static inline struct f_gsi *d_port_to_gsi(struct gsi_data_port *d)
{
	return container_of(d, struct f_gsi, d_port);
}

static inline struct f_gsi *c_port_to_gsi(struct gsi_ctrl_port *d)
{
	return container_of(d, struct f_gsi, c_port);
}

/* for configfs support */
#define MAX_INST_NAME_LEN	40

struct gsi_opts {
	struct usb_function_instance func_inst;
	struct f_gsi *gsi;

	/* os desc support */
	struct config_group *interf_group;
	char ext_compat_id[16];
	struct usb_os_desc os_desc;
};

static inline struct gsi_opts *to_gsi_opts(struct config_item *item)
{
	return container_of(to_config_group(item), struct gsi_opts,
			    func_inst.group);
}

static int name_to_prot_id(const char *name)
{
	if (!name)
		goto error;

	if (!strncasecmp(name, "rndis", MAX_INST_NAME_LEN))
		return USB_PROT_RNDIS_IPA;
	if (!strncasecmp(name, "ecm", MAX_INST_NAME_LEN))
		return USB_PROT_ECM_IPA;
	if (!strncasecmp(name, "rmnet", MAX_INST_NAME_LEN))
		return USB_PROT_RMNET_IPA;
	if (!strncasecmp(name, "mbim", MAX_INST_NAME_LEN))
		return USB_PROT_MBIM_IPA;
	if (!strncasecmp(name, "dpl", MAX_INST_NAME_LEN))
		return USB_PROT_DIAG_IPA;
	if (!strncasecmp(name, "rmnet.ether", MAX_INST_NAME_LEN))
		return USB_PROT_RMNET_ETHER;
	if (!strncasecmp(name, "dpl.ether", MAX_INST_NAME_LEN))
		return USB_PROT_DPL_ETHER;
	if (!strncasecmp(name, "gps", MAX_INST_NAME_LEN))
		return USB_PROT_GPS_CTRL;

error:
	return -EINVAL;
}

/* device descriptors */

#define LOG2_STATUS_INTERVAL_MSEC 5
#define MAX_NOTIFY_SIZE sizeof(struct usb_cdc_notification)
#define GPS_MAX_NOTIFY_SIZE 64

/* rmnet device descriptors */

static struct usb_interface_descriptor rmnet_gsi_interface_desc = {
	.bLength =		USB_DT_INTERFACE_SIZE,
	.bDescriptorType =	USB_DT_INTERFACE,
	.bNumEndpoints =	3,
	.bInterfaceClass =	USB_CLASS_VENDOR_SPEC,
	.bInterfaceSubClass =	USB_CLASS_VENDOR_SPEC,
	.bInterfaceProtocol =	USB_CLASS_VENDOR_SPEC,
	/* .iInterface = DYNAMIC */
};

/* Full speed support */
static struct usb_endpoint_descriptor rmnet_gsi_fs_notify_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_INT,
	.wMaxPacketSize =	cpu_to_le16(MAX_NOTIFY_SIZE),
	.bInterval =		1 << LOG2_STATUS_INTERVAL_MSEC,
};

static struct usb_endpoint_descriptor rmnet_gsi_fs_in_desc  = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize   =	cpu_to_le16(64),
};

static struct usb_endpoint_descriptor rmnet_gsi_fs_out_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bEndpointAddress =	USB_DIR_OUT,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize   =	cpu_to_le16(64),
};

static struct usb_descriptor_header *rmnet_gsi_fs_function[] = {
	(struct usb_descriptor_header *) &rmnet_gsi_interface_desc,
	(struct usb_descriptor_header *) &rmnet_gsi_fs_notify_desc,
	(struct usb_descriptor_header *) &rmnet_gsi_fs_in_desc,
	(struct usb_descriptor_header *) &rmnet_gsi_fs_out_desc,
	NULL,
};

/* High speed support */
static struct usb_endpoint_descriptor rmnet_gsi_hs_notify_desc  = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_INT,
	.wMaxPacketSize =	cpu_to_le16(MAX_NOTIFY_SIZE),
	.bInterval =		LOG2_STATUS_INTERVAL_MSEC + 4,
};

static struct usb_endpoint_descriptor rmnet_gsi_hs_in_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	cpu_to_le16(512),
};

static struct usb_endpoint_descriptor rmnet_gsi_hs_out_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bEndpointAddress =	USB_DIR_OUT,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	cpu_to_le16(512),
};

static struct usb_descriptor_header *rmnet_gsi_hs_function[] = {
	(struct usb_descriptor_header *) &rmnet_gsi_interface_desc,
	(struct usb_descriptor_header *) &rmnet_gsi_hs_notify_desc,
	(struct usb_descriptor_header *) &rmnet_gsi_hs_in_desc,
	(struct usb_descriptor_header *) &rmnet_gsi_hs_out_desc,
	NULL,
};

/* Super speed support */
static struct usb_endpoint_descriptor rmnet_gsi_ss_notify_desc  = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_INT,
	.wMaxPacketSize =	cpu_to_le16(MAX_NOTIFY_SIZE),
	.bInterval =		LOG2_STATUS_INTERVAL_MSEC + 4,
};

static struct usb_ss_ep_comp_descriptor rmnet_gsi_ss_notify_comp_desc = {
	.bLength =		sizeof(rmnet_gsi_ss_notify_comp_desc),
	.bDescriptorType =	USB_DT_SS_ENDPOINT_COMP,

	/* the following 3 values can be tweaked if necessary */
	/* .bMaxBurst =		0, */
	/* .bmAttributes =	0, */
	.wBytesPerInterval =	cpu_to_le16(MAX_NOTIFY_SIZE),
};

static struct usb_endpoint_descriptor rmnet_gsi_ss_in_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	cpu_to_le16(1024),
};

static struct usb_ss_ep_comp_descriptor rmnet_gsi_ss_in_comp_desc = {
	.bLength =		sizeof(rmnet_gsi_ss_in_comp_desc),
	.bDescriptorType =	USB_DT_SS_ENDPOINT_COMP,

	/* the following 2 values can be tweaked if necessary */
	.bMaxBurst =		2,
	/* .bmAttributes =	0, */
};

static struct usb_endpoint_descriptor rmnet_gsi_ss_out_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bEndpointAddress =	USB_DIR_OUT,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	cpu_to_le16(1024),
};

static struct usb_ss_ep_comp_descriptor rmnet_gsi_ss_out_comp_desc = {
	.bLength =		sizeof(rmnet_gsi_ss_out_comp_desc),
	.bDescriptorType =	USB_DT_SS_ENDPOINT_COMP,

	/* the following 2 values can be tweaked if necessary */
	.bMaxBurst =		2,
	/* .bmAttributes =	0, */
};

static struct usb_descriptor_header *rmnet_gsi_ss_function[] = {
	(struct usb_descriptor_header *) &rmnet_gsi_interface_desc,
	(struct usb_descriptor_header *) &rmnet_gsi_ss_notify_desc,
	(struct usb_descriptor_header *) &rmnet_gsi_ss_notify_comp_desc,
	(struct usb_descriptor_header *) &rmnet_gsi_ss_in_desc,
	(struct usb_descriptor_header *) &rmnet_gsi_ss_in_comp_desc,
	(struct usb_descriptor_header *) &rmnet_gsi_ss_out_desc,
	(struct usb_descriptor_header *) &rmnet_gsi_ss_out_comp_desc,
	NULL,
};

/* String descriptors */
static struct usb_string rmnet_gsi_string_defs[] = {
	[0].s = "RmNet",
	{  } /* end of list */
};

static struct usb_gadget_strings rmnet_gsi_string_table = {
	.language =		0x0409,	/* en-us */
	.strings =		rmnet_gsi_string_defs,
};

static struct usb_gadget_strings *rmnet_gsi_strings[] = {
	&rmnet_gsi_string_table,
	NULL,
};

/* rndis device descriptors */

/* interface descriptor: Supports "Wireless" RNDIS; auto-detected by Windows*/
static struct usb_interface_descriptor rndis_gsi_control_intf = {
	.bLength =		sizeof(rndis_gsi_control_intf),
	.bDescriptorType =	USB_DT_INTERFACE,

	/* .bInterfaceNumber = DYNAMIC */
	/* status endpoint is optional; this could be patched later */
	.bNumEndpoints =	1,
	.bInterfaceClass =	USB_CLASS_WIRELESS_CONTROLLER,
	.bInterfaceSubClass =   0x01,
	.bInterfaceProtocol =   0x03,
	/* .iInterface = DYNAMIC */
};

static struct usb_cdc_header_desc rndis_gsi_header_desc = {
	.bLength =		sizeof(rndis_gsi_header_desc),
	.bDescriptorType =	USB_DT_CS_INTERFACE,
	.bDescriptorSubType =	USB_CDC_HEADER_TYPE,

	.bcdCDC =		cpu_to_le16(0x0110),
};

static struct usb_cdc_call_mgmt_descriptor rndis_gsi_call_mgmt_descriptor = {
	.bLength =		sizeof(rndis_gsi_call_mgmt_descriptor),
	.bDescriptorType =	USB_DT_CS_INTERFACE,
	.bDescriptorSubType =	USB_CDC_CALL_MANAGEMENT_TYPE,

	.bmCapabilities =	0x00,
	.bDataInterface =	0x01,
};

static struct usb_cdc_acm_descriptor rndis_gsi_acm_descriptor = {
	.bLength =		sizeof(rndis_gsi_acm_descriptor),
	.bDescriptorType =	USB_DT_CS_INTERFACE,
	.bDescriptorSubType =	USB_CDC_ACM_TYPE,

	.bmCapabilities =	0x00,
};

static struct usb_cdc_union_desc rndis_gsi_union_desc = {
	.bLength =		sizeof(rndis_gsi_union_desc),
	.bDescriptorType =	USB_DT_CS_INTERFACE,
	.bDescriptorSubType =	USB_CDC_UNION_TYPE,
	/* .bMasterInterface0 =	DYNAMIC */
	/* .bSlaveInterface0 =	DYNAMIC */
};

/* the data interface has two bulk endpoints */

static struct usb_interface_descriptor rndis_gsi_data_intf = {
	.bLength =		sizeof(rndis_gsi_data_intf),
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
rndis_gsi_iad_descriptor = {
	.bLength =		sizeof(rndis_gsi_iad_descriptor),
	.bDescriptorType =	USB_DT_INTERFACE_ASSOCIATION,
	.bFirstInterface =	0, /* XXX, hardcoded */
	.bInterfaceCount =	2, /* control + data */
	.bFunctionClass =	USB_CLASS_WIRELESS_CONTROLLER,
	.bFunctionSubClass =	0x01,
	.bFunctionProtocol =	0x03,
	/* .iFunction = DYNAMIC */
};

/* full speed support: */
static struct usb_endpoint_descriptor rndis_gsi_fs_notify_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_INT,
	.wMaxPacketSize =	cpu_to_le16(MAX_NOTIFY_SIZE),
	.bInterval =		1 << LOG2_STATUS_INTERVAL_MSEC,
};

static struct usb_endpoint_descriptor rndis_gsi_fs_in_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.wMaxPacketSize =	cpu_to_le16(64),
	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
};

static struct usb_endpoint_descriptor rndis_gsi_fs_out_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.wMaxPacketSize =	cpu_to_le16(64),
	.bEndpointAddress =	USB_DIR_OUT,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
};

static struct usb_descriptor_header *gsi_eth_fs_function[] = {
	(struct usb_descriptor_header *) &rndis_gsi_iad_descriptor,
	/* control interface matches ACM, not Ethernet */
	(struct usb_descriptor_header *) &rndis_gsi_control_intf,
	(struct usb_descriptor_header *) &rndis_gsi_header_desc,
	(struct usb_descriptor_header *) &rndis_gsi_call_mgmt_descriptor,
	(struct usb_descriptor_header *) &rndis_gsi_acm_descriptor,
	(struct usb_descriptor_header *) &rndis_gsi_union_desc,
	(struct usb_descriptor_header *) &rndis_gsi_fs_notify_desc,
	/* data interface has no altsetting */
	(struct usb_descriptor_header *) &rndis_gsi_data_intf,
	(struct usb_descriptor_header *) &rndis_gsi_fs_in_desc,
	(struct usb_descriptor_header *) &rndis_gsi_fs_out_desc,
	NULL,
};

/* high speed support: */
static struct usb_endpoint_descriptor rndis_gsi_hs_notify_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_INT,
	.wMaxPacketSize =	cpu_to_le16(MAX_NOTIFY_SIZE),
	.bInterval =		LOG2_STATUS_INTERVAL_MSEC + 4,
};
static struct usb_endpoint_descriptor rndis_gsi_hs_in_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	cpu_to_le16(512),
};

static struct usb_endpoint_descriptor rndis_gsi_hs_out_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	.bEndpointAddress =	USB_DIR_OUT,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	cpu_to_le16(512),
};

static struct usb_descriptor_header *gsi_eth_hs_function[] = {
	(struct usb_descriptor_header *) &rndis_gsi_iad_descriptor,
	/* control interface matches ACM, not Ethernet */
	(struct usb_descriptor_header *) &rndis_gsi_control_intf,
	(struct usb_descriptor_header *) &rndis_gsi_header_desc,
	(struct usb_descriptor_header *) &rndis_gsi_call_mgmt_descriptor,
	(struct usb_descriptor_header *) &rndis_gsi_acm_descriptor,
	(struct usb_descriptor_header *) &rndis_gsi_union_desc,
	(struct usb_descriptor_header *) &rndis_gsi_hs_notify_desc,
	/* data interface has no altsetting */
	(struct usb_descriptor_header *) &rndis_gsi_data_intf,
	(struct usb_descriptor_header *) &rndis_gsi_hs_in_desc,
	(struct usb_descriptor_header *) &rndis_gsi_hs_out_desc,
	NULL,
};

/* super speed support: */
static struct usb_endpoint_descriptor rndis_gsi_ss_notify_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_INT,
	.wMaxPacketSize =	cpu_to_le16(MAX_NOTIFY_SIZE),
	.bInterval =		LOG2_STATUS_INTERVAL_MSEC + 4,
};

static struct usb_ss_ep_comp_descriptor rndis_gsi_ss_intr_comp_desc = {
	.bLength =		sizeof(rndis_gsi_ss_intr_comp_desc),
	.bDescriptorType =	USB_DT_SS_ENDPOINT_COMP,

	/* the following 3 values can be tweaked if necessary */
	/* .bMaxBurst =		0, */
	/* .bmAttributes =	0, */
	.wBytesPerInterval =	cpu_to_le16(MAX_NOTIFY_SIZE),
};

static struct usb_endpoint_descriptor rndis_gsi_ss_in_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	cpu_to_le16(1024),
};

static struct usb_endpoint_descriptor rndis_gsi_ss_out_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	.bEndpointAddress =	USB_DIR_OUT,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	cpu_to_le16(1024),
};

static struct usb_ss_ep_comp_descriptor rndis_gsi_ss_bulk_comp_desc = {
	.bLength =		sizeof(rndis_gsi_ss_bulk_comp_desc),
	.bDescriptorType =	USB_DT_SS_ENDPOINT_COMP,

	/* the following 2 values can be tweaked if necessary */
	.bMaxBurst =		2,
	/* .bmAttributes =	0, */
};

static struct usb_descriptor_header *gsi_eth_ss_function[] = {
	(struct usb_descriptor_header *) &rndis_gsi_iad_descriptor,

	/* control interface matches ACM, not Ethernet */
	(struct usb_descriptor_header *) &rndis_gsi_control_intf,
	(struct usb_descriptor_header *) &rndis_gsi_header_desc,
	(struct usb_descriptor_header *) &rndis_gsi_call_mgmt_descriptor,
	(struct usb_descriptor_header *) &rndis_gsi_acm_descriptor,
	(struct usb_descriptor_header *) &rndis_gsi_union_desc,
	(struct usb_descriptor_header *) &rndis_gsi_ss_notify_desc,
	(struct usb_descriptor_header *) &rndis_gsi_ss_intr_comp_desc,

	/* data interface has no altsetting */
	(struct usb_descriptor_header *) &rndis_gsi_data_intf,
	(struct usb_descriptor_header *) &rndis_gsi_ss_in_desc,
	(struct usb_descriptor_header *) &rndis_gsi_ss_bulk_comp_desc,
	(struct usb_descriptor_header *) &rndis_gsi_ss_out_desc,
	(struct usb_descriptor_header *) &rndis_gsi_ss_bulk_comp_desc,
	NULL,
};

/* string descriptors: */
static struct usb_string rndis_gsi_string_defs[] = {
	[0].s = "RNDIS Communications Control",
	[1].s = "RNDIS Ethernet Data",
	[2].s = "RNDIS",
	{  } /* end of list */
};

static struct usb_gadget_strings rndis_gsi_string_table = {
	.language =		0x0409,	/* en-us */
	.strings =		rndis_gsi_string_defs,
};

static struct usb_gadget_strings *rndis_gsi_strings[] = {
	&rndis_gsi_string_table,
	NULL,
};

/* mbim device descriptors */
#define MBIM_NTB_DEFAULT_IN_SIZE	(0x4000)

static struct usb_cdc_ncm_ntb_parameters mbim_gsi_ntb_parameters = {
	.wLength = sizeof(mbim_gsi_ntb_parameters),
	.bmNtbFormatsSupported = cpu_to_le16(USB_CDC_NCM_NTB16_SUPPORTED),
	.dwNtbInMaxSize = cpu_to_le32(MBIM_NTB_DEFAULT_IN_SIZE),
	.wNdpInDivisor = cpu_to_le16(4),
	.wNdpInPayloadRemainder = cpu_to_le16(0),
	.wNdpInAlignment = cpu_to_le16(4),

	.dwNtbOutMaxSize = cpu_to_le32(0x4000),
	.wNdpOutDivisor = cpu_to_le16(4),
	.wNdpOutPayloadRemainder = cpu_to_le16(0),
	.wNdpOutAlignment = cpu_to_le16(4),
	.wNtbOutMaxDatagrams = 16,
};

/*
 * Use wMaxPacketSize big enough to fit CDC_NOTIFY_SPEED_CHANGE in one
 * packet, to simplify cancellation;
 */
#define NCM_STATUS_BYTECOUNT		16	/* 8 byte header + data */

static struct usb_interface_assoc_descriptor mbim_gsi_iad_desc = {
	.bLength =		sizeof(mbim_gsi_iad_desc),
	.bDescriptorType =	USB_DT_INTERFACE_ASSOCIATION,

	/* .bFirstInterface =	DYNAMIC, */
	.bInterfaceCount =	2,	/* control + data */
	.bFunctionClass =	2,
	.bFunctionSubClass =	0x0e,
	.bFunctionProtocol =	0,
	/* .iFunction =		DYNAMIC */
};

/* interface descriptor: */
static struct usb_interface_descriptor mbim_gsi_control_intf = {
	.bLength =		sizeof(mbim_gsi_control_intf),
	.bDescriptorType =	USB_DT_INTERFACE,

	/* .bInterfaceNumber = DYNAMIC */
	.bNumEndpoints =	1,
	.bInterfaceClass =	0x02,
	.bInterfaceSubClass =	0x0e,
	.bInterfaceProtocol =	0,
	/* .iInterface = DYNAMIC */
};

static struct usb_cdc_header_desc mbim_gsi_header_desc = {
	.bLength =		sizeof(mbim_gsi_header_desc),
	.bDescriptorType =	USB_DT_CS_INTERFACE,
	.bDescriptorSubType =	USB_CDC_HEADER_TYPE,

	.bcdCDC =		cpu_to_le16(0x0110),
};

static struct usb_cdc_union_desc mbim_gsi_union_desc = {
	.bLength =		sizeof(mbim_gsi_union_desc),
	.bDescriptorType =	USB_DT_CS_INTERFACE,
	.bDescriptorSubType =	USB_CDC_UNION_TYPE,
	/* .bMasterInterface0 =	DYNAMIC */
	/* .bSlaveInterface0 =	DYNAMIC */
};

static struct usb_cdc_mbim_desc mbim_gsi_desc = {
	.bLength =		sizeof(mbim_gsi_desc),
	.bDescriptorType =	USB_DT_CS_INTERFACE,
	.bDescriptorSubType =	USB_CDC_MBIM_TYPE,

	.bcdMBIMVersion =	cpu_to_le16(0x0100),

	.wMaxControlMessage =	cpu_to_le16(0x1000),
	.bNumberFilters =	0x20,
	.bMaxFilterSize =	0x80,
	.wMaxSegmentSize =	cpu_to_le16(0xfe0),
	.bmNetworkCapabilities = 0x20,
};

static struct usb_cdc_mbim_extended_desc mbim_gsi_ext_mbb_desc = {
	.bLength =	sizeof(mbim_gsi_ext_mbb_desc),
	.bDescriptorType =	USB_DT_CS_INTERFACE,
	.bDescriptorSubType =	USB_CDC_MBIM_EXTENDED_TYPE,

	.bcdMBIMExtendedVersion =		cpu_to_le16(0x0100),
	.bMaxOutstandingCommandMessages =	64,
	.wMTU =					cpu_to_le16(1500),
};

/* the default data interface has no endpoints ... */
static struct usb_interface_descriptor mbim_gsi_data_nop_intf = {
	.bLength =		sizeof(mbim_gsi_data_nop_intf),
	.bDescriptorType =	USB_DT_INTERFACE,

	/* .bInterfaceNumber = DYNAMIC */
	.bAlternateSetting =	0,
	.bNumEndpoints =	0,
	.bInterfaceClass =	0x0a,
	.bInterfaceSubClass =	0,
	.bInterfaceProtocol =	0x02,
	/* .iInterface = DYNAMIC */
};

/* ... but the "real" data interface has two bulk endpoints */
static struct usb_interface_descriptor mbim_gsi_data_intf = {
	.bLength =		sizeof(mbim_gsi_data_intf),
	.bDescriptorType =	USB_DT_INTERFACE,

	/* .bInterfaceNumber = DYNAMIC */
	.bAlternateSetting =	1,
	.bNumEndpoints =	2,
	.bInterfaceClass =	0x0a,
	.bInterfaceSubClass =	0,
	.bInterfaceProtocol =	0x02,
	/* .iInterface = DYNAMIC */
};

/* full speed support: */

static struct usb_endpoint_descriptor mbim_gsi_fs_notify_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_INT,
	.wMaxPacketSize =	4*cpu_to_le16(NCM_STATUS_BYTECOUNT),
	.bInterval =		1 << LOG2_STATUS_INTERVAL_MSEC,
};

static struct usb_endpoint_descriptor mbim_gsi_fs_in_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	4*cpu_to_le16(NCM_STATUS_BYTECOUNT),
};

static struct usb_endpoint_descriptor mbim_gsi_fs_out_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	.bEndpointAddress =	USB_DIR_OUT,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	4*cpu_to_le16(NCM_STATUS_BYTECOUNT),
};

static struct usb_descriptor_header *mbim_gsi_fs_function[] = {
	(struct usb_descriptor_header *) &mbim_gsi_iad_desc,
	/* MBIM control descriptors */
	(struct usb_descriptor_header *) &mbim_gsi_control_intf,
	(struct usb_descriptor_header *) &mbim_gsi_header_desc,
	(struct usb_descriptor_header *) &mbim_gsi_union_desc,
	(struct usb_descriptor_header *) &mbim_gsi_desc,
	(struct usb_descriptor_header *) &mbim_gsi_ext_mbb_desc,
	(struct usb_descriptor_header *) &mbim_gsi_fs_notify_desc,
	/* data interface, altsettings 0 and 1 */
	(struct usb_descriptor_header *) &mbim_gsi_data_nop_intf,
	(struct usb_descriptor_header *) &mbim_gsi_data_intf,
	(struct usb_descriptor_header *) &mbim_gsi_fs_in_desc,
	(struct usb_descriptor_header *) &mbim_gsi_fs_out_desc,
	NULL,
};

/* high speed support: */
static struct usb_endpoint_descriptor mbim_gsi_hs_notify_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_INT,
	.wMaxPacketSize =	4*cpu_to_le16(NCM_STATUS_BYTECOUNT),
	.bInterval =		LOG2_STATUS_INTERVAL_MSEC + 4,
};
static struct usb_endpoint_descriptor mbim_gsi_hs_in_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	cpu_to_le16(512),
};

static struct usb_endpoint_descriptor mbim_gsi_hs_out_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	.bEndpointAddress =	USB_DIR_OUT,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	cpu_to_le16(512),
};

static struct usb_descriptor_header *mbim_gsi_hs_function[] = {
	(struct usb_descriptor_header *) &mbim_gsi_iad_desc,
	/* MBIM control descriptors */
	(struct usb_descriptor_header *) &mbim_gsi_control_intf,
	(struct usb_descriptor_header *) &mbim_gsi_header_desc,
	(struct usb_descriptor_header *) &mbim_gsi_union_desc,
	(struct usb_descriptor_header *) &mbim_gsi_desc,
	(struct usb_descriptor_header *) &mbim_gsi_ext_mbb_desc,
	(struct usb_descriptor_header *) &mbim_gsi_hs_notify_desc,
	/* data interface, altsettings 0 and 1 */
	(struct usb_descriptor_header *) &mbim_gsi_data_nop_intf,
	(struct usb_descriptor_header *) &mbim_gsi_data_intf,
	(struct usb_descriptor_header *) &mbim_gsi_hs_in_desc,
	(struct usb_descriptor_header *) &mbim_gsi_hs_out_desc,
	NULL,
};

/* Super Speed Support */
static struct usb_endpoint_descriptor mbim_gsi_ss_notify_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_INT,
	.wMaxPacketSize =	4*cpu_to_le16(NCM_STATUS_BYTECOUNT),
	.bInterval =		LOG2_STATUS_INTERVAL_MSEC + 4,
};

static struct usb_ss_ep_comp_descriptor mbim_gsi_ss_notify_comp_desc = {
	.bLength =		sizeof(mbim_gsi_ss_notify_comp_desc),
	.bDescriptorType =	USB_DT_SS_ENDPOINT_COMP,

	/* the following 3 values can be tweaked if necessary */
	/* .bMaxBurst =         0, */
	/* .bmAttributes =      0, */
	.wBytesPerInterval =	4*cpu_to_le16(NCM_STATUS_BYTECOUNT),
};

static struct usb_endpoint_descriptor mbim_gsi_ss_in_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	cpu_to_le16(1024),
};

static struct usb_ss_ep_comp_descriptor mbim_gsi_ss_in_comp_desc = {
	.bLength =              sizeof(mbim_gsi_ss_in_comp_desc),
	.bDescriptorType =      USB_DT_SS_ENDPOINT_COMP,

	/* the following 2 values can be tweaked if necessary */
	.bMaxBurst =         2,
	/* .bmAttributes =      0, */
};

static struct usb_endpoint_descriptor mbim_gsi_ss_out_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bEndpointAddress =	USB_DIR_OUT,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	cpu_to_le16(1024),
};

static struct usb_ss_ep_comp_descriptor mbim_gsi_ss_out_comp_desc = {
	.bLength =		sizeof(mbim_gsi_ss_out_comp_desc),
	.bDescriptorType =	USB_DT_SS_ENDPOINT_COMP,

	/* the following 2 values can be tweaked if necessary */
	.bMaxBurst =         2,
	/* .bmAttributes =      0, */
};

static struct usb_descriptor_header *mbim_gsi_ss_function[] = {
	(struct usb_descriptor_header *) &mbim_gsi_iad_desc,
	/* MBIM control descriptors */
	(struct usb_descriptor_header *) &mbim_gsi_control_intf,
	(struct usb_descriptor_header *) &mbim_gsi_header_desc,
	(struct usb_descriptor_header *) &mbim_gsi_union_desc,
	(struct usb_descriptor_header *) &mbim_gsi_desc,
	(struct usb_descriptor_header *) &mbim_gsi_ext_mbb_desc,
	(struct usb_descriptor_header *) &mbim_gsi_ss_notify_desc,
	(struct usb_descriptor_header *) &mbim_gsi_ss_notify_comp_desc,
	/* data interface, altsettings 0 and 1 */
	(struct usb_descriptor_header *) &mbim_gsi_data_nop_intf,
	(struct usb_descriptor_header *) &mbim_gsi_data_intf,
	(struct usb_descriptor_header *) &mbim_gsi_ss_in_desc,
	(struct usb_descriptor_header *) &mbim_gsi_ss_in_comp_desc,
	(struct usb_descriptor_header *) &mbim_gsi_ss_out_desc,
	(struct usb_descriptor_header *) &mbim_gsi_ss_out_comp_desc,
	NULL,
};

/* string descriptors: */
static struct usb_string mbim_gsi_string_defs[] = {
	[0].s = "MBIM Control",
	[1].s = "MBIM Data",
	{  } /* end of list */
};

static struct usb_gadget_strings mbim_gsi_string_table = {
	.language =		0x0409,	/* en-us */
	.strings =		mbim_gsi_string_defs,
};

static struct usb_gadget_strings *mbim_gsi_strings[] = {
	&mbim_gsi_string_table,
	NULL,
};

/* ecm device descriptors */
#define ECM_QC_LOG2_STATUS_INTERVAL_MSEC	5
#define ECM_QC_STATUS_BYTECOUNT			16 /* 8 byte header + data */

/* interface descriptor: */
static struct usb_interface_descriptor ecm_gsi_control_intf = {
	.bLength =		sizeof(ecm_gsi_control_intf),
	.bDescriptorType =	USB_DT_INTERFACE,

	/* .bInterfaceNumber = DYNAMIC */
	/* status endpoint is optional; this could be patched later */
	.bNumEndpoints =	1,
	.bInterfaceClass =	USB_CLASS_COMM,
	.bInterfaceSubClass =	USB_CDC_SUBCLASS_ETHERNET,
	.bInterfaceProtocol =	USB_CDC_PROTO_NONE,
	/* .iInterface = DYNAMIC */
};

static struct usb_cdc_header_desc ecm_gsi_header_desc = {
	.bLength =		sizeof(ecm_gsi_header_desc),
	.bDescriptorType =	USB_DT_CS_INTERFACE,
	.bDescriptorSubType =	USB_CDC_HEADER_TYPE,

	.bcdCDC =		cpu_to_le16(0x0110),
};

static struct usb_cdc_union_desc ecm_gsi_union_desc = {
	.bLength =		sizeof(ecm_gsi_union_desc),
	.bDescriptorType =	USB_DT_CS_INTERFACE,
	.bDescriptorSubType =	USB_CDC_UNION_TYPE,
	/* .bMasterInterface0 =	DYNAMIC */
	/* .bSlaveInterface0 =	DYNAMIC */
};

static struct usb_cdc_ether_desc ecm_gsi_desc = {
	.bLength =		sizeof(ecm_gsi_desc),
	.bDescriptorType =	USB_DT_CS_INTERFACE,
	.bDescriptorSubType =	USB_CDC_ETHERNET_TYPE,

	/* this descriptor actually adds value, surprise! */
	/* .iMACAddress = DYNAMIC */
	.bmEthernetStatistics =	cpu_to_le32(0), /* no statistics */
	.wMaxSegmentSize =	cpu_to_le16(ETH_FRAME_LEN),
	.wNumberMCFilters =	cpu_to_le16(0),
	.bNumberPowerFilters =	0,
};

/* the default data interface has no endpoints ... */

static struct usb_interface_descriptor ecm_gsi_data_nop_intf = {
	.bLength =		sizeof(ecm_gsi_data_nop_intf),
	.bDescriptorType =	USB_DT_INTERFACE,

	.bInterfaceNumber =	1,
	.bAlternateSetting =	0,
	.bNumEndpoints =	0,
	.bInterfaceClass =	USB_CLASS_CDC_DATA,
	.bInterfaceSubClass =	0,
	.bInterfaceProtocol =	0,
	/* .iInterface = DYNAMIC */
};

/* ... but the "real" data interface has two bulk endpoints */

static struct usb_interface_descriptor ecm_gsi_data_intf = {
	.bLength =		sizeof(ecm_gsi_data_intf),
	.bDescriptorType =	USB_DT_INTERFACE,

	.bInterfaceNumber =	1,
	.bAlternateSetting =	1,
	.bNumEndpoints =	2,
	.bInterfaceClass =	USB_CLASS_CDC_DATA,
	.bInterfaceSubClass =	0,
	.bInterfaceProtocol =	0,
	/* .iInterface = DYNAMIC */
};

/* full speed support: */
static struct usb_endpoint_descriptor ecm_gsi_fs_notify_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_INT,
	.wMaxPacketSize =	cpu_to_le16(ECM_QC_STATUS_BYTECOUNT),
	.bInterval =		1 << LOG2_STATUS_INTERVAL_MSEC,
};

static struct usb_endpoint_descriptor ecm_gsi_fs_in_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	cpu_to_le16(ECM_QC_STATUS_BYTECOUNT),
};

static struct usb_endpoint_descriptor ecm_gsi_fs_out_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	.bEndpointAddress =	USB_DIR_OUT,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	cpu_to_le16(ECM_QC_STATUS_BYTECOUNT),
};

static struct usb_descriptor_header *ecm_gsi_fs_function[] = {
	/* CDC ECM control descriptors */
	(struct usb_descriptor_header *) &ecm_gsi_control_intf,
	(struct usb_descriptor_header *) &ecm_gsi_header_desc,
	(struct usb_descriptor_header *) &ecm_gsi_union_desc,
	(struct usb_descriptor_header *) &ecm_gsi_desc,
	/* NOTE: status endpoint might need to be removed */
	(struct usb_descriptor_header *) &ecm_gsi_fs_notify_desc,
	/* data interface, altsettings 0 and 1 */
	(struct usb_descriptor_header *) &ecm_gsi_data_nop_intf,
	(struct usb_descriptor_header *) &ecm_gsi_data_intf,
	(struct usb_descriptor_header *) &ecm_gsi_fs_in_desc,
	(struct usb_descriptor_header *) &ecm_gsi_fs_out_desc,
	NULL,
};

/* high speed support: */
static struct usb_endpoint_descriptor ecm_gsi_hs_notify_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_INT,
	.wMaxPacketSize =	cpu_to_le16(ECM_QC_STATUS_BYTECOUNT),
	.bInterval =		LOG2_STATUS_INTERVAL_MSEC + 4,
};
static struct usb_endpoint_descriptor ecm_gsi_hs_in_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	cpu_to_le16(512),
};

static struct usb_endpoint_descriptor ecm_gsi_hs_out_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	.bEndpointAddress =	USB_DIR_OUT,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	cpu_to_le16(512),
};

static struct usb_descriptor_header *ecm_gsi_hs_function[] = {
	/* CDC ECM control descriptors */
	(struct usb_descriptor_header *) &ecm_gsi_control_intf,
	(struct usb_descriptor_header *) &ecm_gsi_header_desc,
	(struct usb_descriptor_header *) &ecm_gsi_union_desc,
	(struct usb_descriptor_header *) &ecm_gsi_desc,
	/* NOTE: status endpoint might need to be removed */
	(struct usb_descriptor_header *) &ecm_gsi_hs_notify_desc,
	/* data interface, altsettings 0 and 1 */
	(struct usb_descriptor_header *) &ecm_gsi_data_nop_intf,
	(struct usb_descriptor_header *) &ecm_gsi_data_intf,
	(struct usb_descriptor_header *) &ecm_gsi_hs_in_desc,
	(struct usb_descriptor_header *) &ecm_gsi_hs_out_desc,
	NULL,
};

static struct usb_endpoint_descriptor ecm_gsi_ss_notify_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_INT,
	.wMaxPacketSize =	cpu_to_le16(ECM_QC_STATUS_BYTECOUNT),
	.bInterval =		ECM_QC_LOG2_STATUS_INTERVAL_MSEC + 4,
};

static struct usb_ss_ep_comp_descriptor ecm_gsi_ss_notify_comp_desc = {
	.bLength =		sizeof(ecm_gsi_ss_notify_comp_desc),
	.bDescriptorType =	USB_DT_SS_ENDPOINT_COMP,

	/* the following 3 values can be tweaked if necessary */
	/* .bMaxBurst =         0, */
	/* .bmAttributes =      0, */
	.wBytesPerInterval =	cpu_to_le16(ECM_QC_STATUS_BYTECOUNT),
};

static struct usb_endpoint_descriptor ecm_gsi_ss_in_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	cpu_to_le16(1024),
};

static struct usb_ss_ep_comp_descriptor ecm_gsi_ss_in_comp_desc = {
	.bLength =		sizeof(ecm_gsi_ss_in_comp_desc),
	.bDescriptorType =	USB_DT_SS_ENDPOINT_COMP,

	/* the following 2 values can be tweaked if necessary */
	.bMaxBurst =         2,
	/* .bmAttributes =      0, */
};

static struct usb_endpoint_descriptor ecm_gsi_ss_out_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bEndpointAddress =	USB_DIR_OUT,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	cpu_to_le16(1024),
};

static struct usb_ss_ep_comp_descriptor ecm_gsi_ss_out_comp_desc = {
	.bLength =		sizeof(ecm_gsi_ss_out_comp_desc),
	.bDescriptorType =	USB_DT_SS_ENDPOINT_COMP,

	/* the following 2 values can be tweaked if necessary */
	.bMaxBurst =         2,
	/* .bmAttributes =      0, */
};

static struct usb_descriptor_header *ecm_gsi_ss_function[] = {
	/* CDC ECM control descriptors */
	(struct usb_descriptor_header *) &ecm_gsi_control_intf,
	(struct usb_descriptor_header *) &ecm_gsi_header_desc,
	(struct usb_descriptor_header *) &ecm_gsi_union_desc,
	(struct usb_descriptor_header *) &ecm_gsi_desc,
	/* NOTE: status endpoint might need to be removed */
	(struct usb_descriptor_header *) &ecm_gsi_ss_notify_desc,
	(struct usb_descriptor_header *) &ecm_gsi_ss_notify_comp_desc,
	/* data interface, altsettings 0 and 1 */
	(struct usb_descriptor_header *) &ecm_gsi_data_nop_intf,
	(struct usb_descriptor_header *) &ecm_gsi_data_intf,
	(struct usb_descriptor_header *) &ecm_gsi_ss_in_desc,
	(struct usb_descriptor_header *) &ecm_gsi_ss_in_comp_desc,
	(struct usb_descriptor_header *) &ecm_gsi_ss_out_desc,
	(struct usb_descriptor_header *) &ecm_gsi_ss_out_comp_desc,
	NULL,
};

/* string descriptors: */
static struct usb_string ecm_gsi_string_defs[] = {
	[0].s = "CDC Ethernet Control Model (ECM)",
	[1].s = NULL /* DYNAMIC */,
	[2].s = "CDC Ethernet Data",
	{  } /* end of list */
};

static struct usb_gadget_strings ecm_gsi_string_table = {
	.language =		0x0409,	/* en-us */
	.strings =		ecm_gsi_string_defs,
};

static struct usb_gadget_strings *ecm_gsi_strings[] = {
	&ecm_gsi_string_table,
	NULL,
};

/* qdss device descriptor */

static struct usb_interface_descriptor qdss_gsi_data_intf_desc = {
	.bLength            =	sizeof(qdss_gsi_data_intf_desc),
	.bDescriptorType    =	USB_DT_INTERFACE,
	.bAlternateSetting  =   0,
	.bNumEndpoints      =	1,
	.bInterfaceClass    =	0xff,
	.bInterfaceSubClass =	0xff,
	.bInterfaceProtocol =	0xff,
};

static struct usb_endpoint_descriptor qdss_gsi_fs_data_desc = {
	.bLength              =	 USB_DT_ENDPOINT_SIZE,
	.bDescriptorType      =	 USB_DT_ENDPOINT,
	.bEndpointAddress     =	 USB_DIR_IN,
	.bmAttributes         =	 USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize       =	 cpu_to_le16(64),
};

static struct usb_endpoint_descriptor qdss_gsi_hs_data_desc = {
	.bLength              =	 USB_DT_ENDPOINT_SIZE,
	.bDescriptorType      =	 USB_DT_ENDPOINT,
	.bEndpointAddress     =	 USB_DIR_IN,
	.bmAttributes         =	 USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize       =	 cpu_to_le16(512),
};

static struct usb_endpoint_descriptor qdss_gsi_ss_data_desc = {
	.bLength              =	 USB_DT_ENDPOINT_SIZE,
	.bDescriptorType      =	 USB_DT_ENDPOINT,
	.bEndpointAddress     =	 USB_DIR_IN,
	.bmAttributes         =  USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize       =	 cpu_to_le16(1024),
};

static struct usb_ss_ep_comp_descriptor qdss_gsi_data_ep_comp_desc = {
	.bLength              =	 sizeof(qdss_gsi_data_ep_comp_desc),
	.bDescriptorType      =	 USB_DT_SS_ENDPOINT_COMP,
	.bMaxBurst            =	 1,
	.bmAttributes         =	 0,
	.wBytesPerInterval    =	 0,
};

static struct usb_descriptor_header *qdss_gsi_fs_data_only_desc[] = {
	(struct usb_descriptor_header *) &qdss_gsi_data_intf_desc,
	(struct usb_descriptor_header *) &qdss_gsi_fs_data_desc,
	NULL,
};

static struct usb_descriptor_header *qdss_gsi_hs_data_only_desc[] = {
	(struct usb_descriptor_header *) &qdss_gsi_data_intf_desc,
	(struct usb_descriptor_header *) &qdss_gsi_hs_data_desc,
	NULL,
};

static struct usb_descriptor_header *qdss_gsi_ss_data_only_desc[] = {
	(struct usb_descriptor_header *) &qdss_gsi_data_intf_desc,
	(struct usb_descriptor_header *) &qdss_gsi_ss_data_desc,
	(struct usb_descriptor_header *) &qdss_gsi_data_ep_comp_desc,
	NULL,
};

/* string descriptors: */
static struct usb_string qdss_gsi_string_defs[] = {
	[0].s = "DPL Data",
	{}, /* end of list */
};

static struct usb_gadget_strings qdss_gsi_string_table = {
	.language =		0x0409,
	.strings =		qdss_gsi_string_defs,
};

static struct usb_gadget_strings *qdss_gsi_strings[] = {
	&qdss_gsi_string_table,
	NULL,
};

/* gps device descriptor */
static struct usb_interface_descriptor gps_interface_desc = {
	.bLength =		USB_DT_INTERFACE_SIZE,
	.bDescriptorType =	USB_DT_INTERFACE,
	.bNumEndpoints =	1,
	.bInterfaceClass =	USB_CLASS_VENDOR_SPEC,
	.bInterfaceSubClass =	USB_CLASS_VENDOR_SPEC,
	.bInterfaceProtocol =	USB_CLASS_VENDOR_SPEC,
	/* .iInterface = DYNAMIC */
};

/* Full speed support */
static struct usb_endpoint_descriptor gps_fs_notify_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_INT,
	.wMaxPacketSize =	cpu_to_le16(GPS_MAX_NOTIFY_SIZE),
	.bInterval =		1 << LOG2_STATUS_INTERVAL_MSEC,
};

static struct usb_descriptor_header *gps_fs_function[] = {
	(struct usb_descriptor_header *) &gps_interface_desc,
	(struct usb_descriptor_header *) &gps_fs_notify_desc,
	NULL,
};

/* High speed support */
static struct usb_endpoint_descriptor gps_hs_notify_desc  = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_INT,
	.wMaxPacketSize =	cpu_to_le16(GPS_MAX_NOTIFY_SIZE),
	.bInterval =		LOG2_STATUS_INTERVAL_MSEC + 4,
};

static struct usb_descriptor_header *gps_hs_function[] = {
	(struct usb_descriptor_header *) &gps_interface_desc,
	(struct usb_descriptor_header *) &gps_hs_notify_desc,
	NULL,
};

/* Super speed support */
static struct usb_endpoint_descriptor gps_ss_notify_desc  = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_INT,
	.wMaxPacketSize =	cpu_to_le16(GPS_MAX_NOTIFY_SIZE),
	.bInterval =		LOG2_STATUS_INTERVAL_MSEC + 4,
};

static struct usb_ss_ep_comp_descriptor gps_ss_notify_comp_desc = {
	.bLength =		sizeof(gps_ss_notify_comp_desc),
	.bDescriptorType =	USB_DT_SS_ENDPOINT_COMP,

	/* the following 3 values can be tweaked if necessary */
	/* .bMaxBurst =		0, */
	/* .bmAttributes =	0, */
	.wBytesPerInterval =	cpu_to_le16(GPS_MAX_NOTIFY_SIZE),
};

static struct usb_descriptor_header *gps_ss_function[] = {
	(struct usb_descriptor_header *) &gps_interface_desc,
	(struct usb_descriptor_header *) &gps_ss_notify_desc,
	(struct usb_descriptor_header *) &gps_ss_notify_comp_desc,
	NULL,
};

/* String descriptors */

static struct usb_string gps_string_defs[] = {
	[0].s = "GPS",
	{  } /* end of list */
};

static struct usb_gadget_strings gps_string_table = {
	.language =		0x0409,	/* en-us */
	.strings =		gps_string_defs,
};

static struct usb_gadget_strings *gps_strings[] = {
	&gps_string_table,
	NULL,
};
#endif
