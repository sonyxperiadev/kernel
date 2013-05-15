/*
 * Copyright(C) 2012 NTT DATA MSE CORPORATION. All right reserved.
 * Copyright (C) 2012-2013 Sony Mobile Communications AB.
 *
 * Gadget Driver for DTF
 *
 * This code borrows from f_adb.c, which is
 * Copyright (C) 2008 Google, Inc.
 * Author: Mike Lockwood <lockwood@android.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/poll.h>
#include <linux/delay.h>
#include <linux/wait.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/kthread.h>
#include <linux/freezer.h>

#include <linux/types.h>
#include <linux/file.h>
#include <linux/device.h>
#include <linux/miscdevice.h>

#include <linux/usb.h>
#include <linux/usb_usual.h>
#include <linux/usb/ch9.h>
#include <linux/usb/cdc.h>
#include <linux/usb/f_dtf.h>
#include <linux/usb/f_dtf_if.h>

/* PipeGroup 1 */
/* Interface Association Descriptor */

/* Standard interface Descriptor (communication) */
static struct usb_interface_descriptor vPg1_intf_comm_desc = {
	.bLength                = USB_DT_INTERFACE_SIZE,
	.bDescriptorType        = USB_DT_INTERFACE,
	.bInterfaceNumber       = 0,
	.bAlternateSetting      = 0,
	.bNumEndpoints          = 1,
	.bInterfaceClass        = USB_CLASS_COMM,
	.bInterfaceSubClass     = 0x88,
	.bInterfaceProtocol     = 1,
	.iInterface             = 0,
};

/* Header Functional Descriptor */
static struct usb_cdc_header_desc vPg1_cdc_header = {
	.bLength            = sizeof vPg1_cdc_header,
	.bDescriptorType    = USB_DT_CS_INTERFACE,
	.bDescriptorSubType = USB_CDC_HEADER_TYPE,
	.bcdCDC             = __constant_cpu_to_le16(0x0110),
};

/* Control Management Functional Descriptor */
static struct usb_cdc_call_mgmt_descriptor vPg1_call_mng = {
	.bLength            =  sizeof vPg1_call_mng,
	.bDescriptorType    =  USB_DT_CS_INTERFACE,
	.bDescriptorSubType =  USB_CDC_CALL_MANAGEMENT_TYPE,
	.bmCapabilities     =  (USB_CDC_CALL_MGMT_CAP_CALL_MGMT
		| USB_CDC_CALL_MGMT_CAP_DATA_INTF),
	.bDataInterface     =  0x01,
};

/* Abstract Controll Magagement  Descriptor */
static struct usb_cdc_acm_descriptor vPg1_acm_desc = {
	.bLength            = sizeof vPg1_acm_desc,
	.bDescriptorType    = USB_DT_CS_INTERFACE,
	.bDescriptorSubType = USB_CDC_ACM_TYPE,
	.bmCapabilities     = (USB_CDC_CAP_LINE|USB_CDC_CAP_BRK),
};

/* Union Function Descriptor */
static struct usb_cdc_union_desc vPg1_union_desc = {
	.bLength            = sizeof vPg1_union_desc,
	.bDescriptorType    = USB_DT_CS_INTERFACE,
	.bDescriptorSubType = USB_CDC_UNION_TYPE,
	.bMasterInterface0  = 0,
	.bSlaveInterface0   = 1,
};

/* Mobile Abstract Control Medel Specific Function Descriptor */
static struct sUsb_model_spcfc_func_desc vPg1_model_desc = {
	.bLength            = 5,
	.bDescriptorType    = (USB_TYPE_VENDOR | USB_DT_INTERFACE),
	.bDescriptorSubType = 0x11,
	.bType              = 0x02,
	.bMode              = {0xC0},
	.mMode_num           = 1,
};

/* Standard endpoint Descriptor (interrupt) (full speed) */
static struct usb_endpoint_descriptor vPg1_epintr_desc = {
	.bLength            = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType    = USB_DT_ENDPOINT,
	.bEndpointAddress   = USB_DIR_IN | 0x01,
	.bmAttributes       = USB_ENDPOINT_XFER_INT,
	.wMaxPacketSize     = __constant_cpu_to_le16(16),
	.bInterval          = 0x10,
};

/* Standard endpoint Descriptor (interrupt) (high speed) */
static struct usb_endpoint_descriptor vPg1_epintr_desc_hs = {
	.bLength            = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType    = USB_DT_ENDPOINT,
	.bEndpointAddress   = USB_DIR_IN | 0x01,
	.bmAttributes       = USB_ENDPOINT_XFER_INT,
	.wMaxPacketSize     = __constant_cpu_to_le16(16),
	.bInterval          = 0x08,
};

/* Standard interface Descriptor */
static struct usb_interface_descriptor vPg1_intf_bulk_desc = {
	.bLength                = USB_DT_INTERFACE_SIZE,
	.bDescriptorType        = USB_DT_INTERFACE,
	.bInterfaceNumber       = 1,
	.bAlternateSetting      = 0,
	.bNumEndpoints          = 2,
	.bInterfaceClass        = USB_CLASS_CDC_DATA,
	.bInterfaceSubClass     = 0,
	.bInterfaceProtocol     = 0,
	.iInterface             = 0,
};

/* Standard endpoint Descriptor (bulk) (high speed) */
static struct usb_endpoint_descriptor vPg1_epin_desc_hs = {
	.bLength            = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType    = USB_DT_ENDPOINT,
	.bEndpointAddress   = USB_DIR_IN | 0x02,
	.bmAttributes       = USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize     = __constant_cpu_to_le16(DTF_MAX_PACKET_SIZE),
	.bInterval          = 0x00,
};

/* Standard endpoint Descriptor (bulk) (high speed) */
static struct usb_endpoint_descriptor vPg1_epout_desc_hs = {
	.bLength            = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType    = USB_DT_ENDPOINT,
	.bEndpointAddress   = USB_DIR_OUT | 0x03,
	.bmAttributes       = USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize     = __constant_cpu_to_le16(DTF_MAX_PACKET_SIZE),
	.bInterval          = 0x00,
};

/* Standard endpoint Descriptor (bulk) (full speed) */
static struct usb_endpoint_descriptor vPg1_epin_desc = {
	.bLength            = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType    = USB_DT_ENDPOINT,
	.bEndpointAddress   = USB_DIR_IN | 0x02,
	.bmAttributes       = USB_ENDPOINT_XFER_BULK,
	.bInterval          = 0x00,
};

/* Standard endpoint Descriptor (bulk) (full speed) */
static struct usb_endpoint_descriptor vPg1_epout_desc = {
	.bLength            = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType    = USB_DT_ENDPOINT,
	.bEndpointAddress   = USB_DIR_OUT | 0x03,
	.bmAttributes       = USB_ENDPOINT_XFER_BULK,
	.bInterval          = 0x00,
};

static struct usb_descriptor_header *vFs_dtf_descs[] = {
	/* PipeGroup 1 */
	(struct usb_descriptor_header *) &vPg1_intf_comm_desc,
	(struct usb_descriptor_header *) &vPg1_cdc_header,
	(struct usb_descriptor_header *) &vPg1_call_mng,
	(struct usb_descriptor_header *) &vPg1_acm_desc,
	(struct usb_descriptor_header *) &vPg1_union_desc,
	(struct usb_descriptor_header *) &vPg1_model_desc,
	(struct usb_descriptor_header *) &vPg1_epintr_desc,
	(struct usb_descriptor_header *) &vPg1_intf_bulk_desc,
	(struct usb_descriptor_header *) &vPg1_epin_desc,
	(struct usb_descriptor_header *) &vPg1_epout_desc,

	NULL,
};

static struct usb_descriptor_header *vHs_dtf_descs[] = {
	/* PipeGroup 1 */
	(struct usb_descriptor_header *) &vPg1_intf_comm_desc,
	(struct usb_descriptor_header *) &vPg1_cdc_header,
	(struct usb_descriptor_header *) &vPg1_call_mng,
	(struct usb_descriptor_header *) &vPg1_acm_desc,
	(struct usb_descriptor_header *) &vPg1_union_desc,
	(struct usb_descriptor_header *) &vPg1_model_desc,
	(struct usb_descriptor_header *) &vPg1_epintr_desc_hs,
	(struct usb_descriptor_header *) &vPg1_intf_bulk_desc,
	(struct usb_descriptor_header *) &vPg1_epin_desc_hs,
	(struct usb_descriptor_header *) &vPg1_epout_desc_hs,

	NULL,
};

static void dtf_complete_intr(struct usb_ep *ep, struct usb_request *req);
static void dtf_complete_in(struct usb_ep *ep, struct usb_request *req);
static void dtf_complete_out(struct usb_ep *ep,
					struct usb_request *req);
static int dtf_function_bind(struct usb_configuration *c,
					struct usb_function *f);
static void dtf_function_unbind(struct usb_configuration *c,
					struct usb_function *f);
static int dtf_function_setup(struct usb_function *f,
					const struct usb_ctrlrequest *ctrl);
static int dtf_function_set_alt(struct usb_function *f,
					unsigned intf, unsigned alt);
static void dtf_function_disable(struct usb_function *f);
static void dtf_function_suspend(struct usb_function *f);
static void dtf_function_resume(struct usb_function *f);
static void dtf_request_free(struct usb_request *req, struct usb_ep *ep);
static inline struct dtf_dev *func_to_dtf(struct usb_function *f);
static struct usb_request *dtf_request_new(struct usb_ep *ep, int buffer_size);

static ssize_t dtf_if_read(struct file *pfile, char *pbuf,
			 size_t count, loff_t *ppos);
static ssize_t dtf_if_write(struct file *pfile, const char *pbuf,
					size_t count, loff_t *ppos);
static unsigned int dtf_if_poll(struct file *pfile,
				 poll_table *pwait);
static long dtf_if_ioctl(struct file *pfile, unsigned int cmd,
					unsigned long arg);
static int dtf_if_open(struct inode *pinode, struct file *pfile);
static int dtf_if_close(struct inode *pinode, struct file *pfile);

static int dtf_if_init(void);
static void dtf_if_out_setup(const struct dtf_if_ctrlrequest *);
static void dtf_if_out_set_alt(int);
static void dtf_if_out_disable(int);
static void dtf_if_out_suspend(void);
static void dtf_if_out_resume(void);
static void dtf_if_out_complete_in(int);
static void dtf_if_out_complete_out(int, int, char *);
static void dtf_if_out_complete_intr(int);
static void dtf_if_out_ctrl_complete(int, int, char *);

static void dtf_if_init_read_data(void);
static void dtf_if_add_read_data(struct dtf_if_read_data *read_data);
static struct dtf_if_read_data *dtf_if_get_read_data(void);
static int dtf_if_get_read_data_num(void);

static int dtf_if_in_intr_in(unsigned size, const char *data);
static int dtf_if_in_bulk_in(unsigned size, const char *data);
static void dtf_if_in_set_halt_intr_in(void);
static void dtf_if_in_set_halt_bulk_in(void);
static void dtf_if_in_set_halt_out(void);
static void dtf_if_in_clear_halt_intr_in(void);
static void dtf_if_in_clear_halt_bulk_in(void);
static void dtf_if_in_clear_halt_out(void);
static void dtf_if_in_ctrl_in(int length, const char *data);
static void dtf_if_in_ctrl_out(int length);

#define DTF_IF_READ_DATA_MAX	5

static const char dtf_shortname[] = "android_dtf";

struct dtf_dev {
	struct usb_function function;
	struct usb_composite_dev *cdev;
	spinlock_t lock;
	struct sDtf_pg pg;
	int    mCtrl_ep_enbl;
	int    mData_ep_enbl;
	atomic_t dtf_if_active;
	int dtf_if_readable_num;
	int dtf_if_readable_head;
	int dtf_if_readable_tail;
	struct dtf_if_read_data _dtf_if_read_data[DTF_IF_READ_DATA_MAX];
};

/* dtf file_operation */
static const struct file_operations dtf_fops = {
	.read =				dtf_if_read,
	.write =			dtf_if_write,
	.poll =				dtf_if_poll,
	.unlocked_ioctl =		dtf_if_ioctl,
	.open =				dtf_if_open,
	.release =			dtf_if_close
};

static struct miscdevice dtf_device = {
	.minor = D_DTF_MINOR_NUMBER,
	.name = dtf_shortname,
	.fops = &dtf_fops,
};

static struct dtf_dev *_dtf_dev;
spinlock_t		lock_read_data;
wait_queue_head_t	poll_wait_read;

static int dtf_if_open(struct inode *pinode, struct file *pfile)
{
	atomic_set(&_dtf_dev->dtf_if_active, 1);
	return 0;
}

static int dtf_if_close(struct inode *pinode, struct file *pfile)
{
	atomic_set(&_dtf_dev->dtf_if_active, 0);
	return 0;
}

static ssize_t dtf_if_read(struct file *pfile, char *pbuf,
					size_t count, loff_t *ppos)
{
	struct dtf_if_read_data *read_data;
	unsigned long	flags;

	if (count < sizeof(struct dtf_if_read_data)) {
		printk(KERN_ERR "[DTF] dtf_if_read count OVER\n");
		return -EFAULT;
	}
	spin_lock_irqsave(&lock_read_data, flags);
	read_data = dtf_if_get_read_data();
	if (read_data == NULL) {
		printk(KERN_ERR "[DTF] dtf_if_read read_data NULL\n");
		spin_unlock_irqrestore(&lock_read_data, flags);
		return -EFAULT;
	}
	if (copy_to_user((void *)pbuf, read_data,
			sizeof(struct dtf_if_read_data))) {
		spin_unlock_irqrestore(&lock_read_data, flags);
		return -EFAULT;
	}
	spin_unlock_irqrestore(&lock_read_data, flags);

	return 0;
}
static ssize_t dtf_if_write(struct file *pfile, const char *pbuf,
					size_t count, loff_t *ppos)
{
	char bulk_in_buff[DTF_MAX_PACKET_SIZE];

	if (count > DTF_MAX_PACKET_SIZE)
		return -EIO;

	if (copy_from_user(bulk_in_buff, pbuf, count))
		return -EFAULT;

	dtf_if_in_bulk_in((unsigned)count, (const char *)bulk_in_buff);
	return 0;
}
static unsigned int dtf_if_poll(struct file *pfile, poll_table *pwait)
{
	unsigned int mask = 0;

	poll_wait(pfile, &poll_wait_read, pwait);

	if (dtf_if_get_read_data_num() > 0)
		mask |= (POLLIN | POLLRDNORM);

	return mask;
}
static long dtf_if_ioctl(struct file *pfile, unsigned int cmd,
					unsigned long arg)
{
	struct dtf_if_write_data write_data;

	switch (cmd) {
	case DTF_IF_EVENT_INTR_IN:
		if (copy_from_user(&write_data, (void *)arg,
						 sizeof(write_data)))
			return -EFAULT;

		if (write_data.size < 0 || write_data.size > 16)
			return -EINVAL;

		dtf_if_in_intr_in((unsigned)write_data.size,
			(const char *)write_data.data);
		break;

	case DTF_IF_EVENT_SET_HALT_INTR_IN:
		dtf_if_in_set_halt_intr_in();
		break;

	case DTF_IF_EVENT_SET_HALT_BULK_IN:
		dtf_if_in_set_halt_bulk_in();
		break;

	case DTF_IF_EVENT_SET_HALT_OUT:
		dtf_if_in_set_halt_out();
		break;

	case DTF_IF_EVENT_CLEAR_HALT_INTR_IN:
		dtf_if_in_clear_halt_intr_in();
		break;

	case DTF_IF_EVENT_CLEAR_HALT_BULK_IN:
		dtf_if_in_clear_halt_bulk_in();
		break;

	case DTF_IF_EVENT_CLEAR_HALT_OUT:
		dtf_if_in_clear_halt_out();
		break;

	case DTF_IF_EVENT_CTRL_IN:
		if (copy_from_user(&write_data, (void *)arg,
						 sizeof(write_data)))
			return -EFAULT;

		if (write_data.size < 0 || write_data.size > 16)
			return -EINVAL;

		dtf_if_in_ctrl_in(write_data.size,
				(const char *)write_data.data);

		break;

	case DTF_IF_EVENT_CTRL_OUT:
		if (copy_from_user(&write_data,
				(void *)arg, sizeof(write_data)))
			return -EFAULT;

		dtf_if_in_ctrl_out(write_data.size);

		break;

	default:
		break;
	}

	return 0;
}


static int dtf_allocate_endpoints(struct usb_configuration *c,
					struct usb_function *f)
{
	struct usb_composite_dev *cdev = c->cdev;
	struct dtf_dev *dev = func_to_dtf(f);
	struct usb_request *req;
	struct usb_ep *ep;

	/* allocate endpoints: PipeGroup1 intrrupt */
	ep = usb_ep_autoconfig(cdev->gadget, &vPg1_epintr_desc);
	if (!ep)
		return -ENODEV;

	ep->driver_data = dev;
	dev->pg.ep_intr = ep;

	/* allocate endpoints: PipeGroup1 bulk(in) */
	ep = usb_ep_autoconfig(cdev->gadget, &vPg1_epin_desc);
	if (!ep)
		return -ENODEV;

	ep->driver_data = dev;
	dev->pg.ep_in = ep;

	/* allocate endpoints: PipeGroup1 bulk(out) */
	ep = usb_ep_autoconfig(cdev->gadget, &vPg1_epout_desc);
	if (!ep)
		return -ENODEV;

	ep->driver_data = dev;
	dev->pg.ep_out = ep;

	/* support high speed hardware */
	if (gadget_is_dualspeed(cdev->gadget)) {
		vPg1_epintr_desc_hs.bEndpointAddress
			= vPg1_epintr_desc.bEndpointAddress;
		vPg1_epin_desc_hs.bEndpointAddress
			= vPg1_epin_desc.bEndpointAddress;
		vPg1_epout_desc_hs.bEndpointAddress
			= vPg1_epout_desc.bEndpointAddress;
	}

	/* allocate request for endpoints */
	req = dtf_request_new(dev->pg.ep_intr, 16);
	if (!req)
		return -ENODEV;

	req->complete = dtf_complete_intr;
	dev->pg.mReq_intr = req;

	req = dtf_request_new(dev->pg.ep_in, DTF_MAX_PACKET_SIZE);
	if (!req) {
		dtf_request_free(dev->pg.mReq_intr, dev->pg.ep_intr);
		return -ENODEV;
	}

	req->complete = dtf_complete_in;
	dev->pg.mReq_in = req;

	req = dtf_request_new(dev->pg.ep_out, DTF_MAX_PACKET_SIZE);
	if (!req) {
		dtf_request_free(dev->pg.mReq_intr, dev->pg.ep_intr);
		dtf_request_free(dev->pg.mReq_in, dev->pg.ep_in);
		return -ENODEV;
	}

	req->complete = dtf_complete_out;
	dev->pg.mReq_out = req;

	return 0;
}

static int dtf_allocate_interface_ids(struct usb_configuration *c,
					struct usb_function *f)
{
	int id;
	struct dtf_dev *dev = func_to_dtf(f);

	/* Allocate Interface ID: PipeGroup1 communication interface */
	id = usb_interface_id(c, f);
	if (id < 0)
		return id;

	dev->pg.mCtrl_id = id;
	id = 0;     /* fixed interface number */
	vPg1_intf_comm_desc.bInterfaceNumber = id;
	vPg1_union_desc.bMasterInterface0 = id;

	/* Allocate Interface ID: PipeGroup1 bulk interface */
	id = usb_interface_id(c, f);
	if (id < 0)
		return id;

	dev->pg.mData_id = id;
	id = 1;     /* fixed interface number */
	vPg1_intf_bulk_desc.bInterfaceNumber = id;
	vPg1_union_desc.bSlaveInterface0 = id;
	vPg1_call_mng.bDataInterface = id;
	return 0;
}

static int dtf_bind_config(struct usb_configuration *c)
{
	struct dtf_dev  *dev = _dtf_dev;
	int ret;

	dev->cdev = c->cdev;
	dev->function.name = "dtf";
	dev->function.descriptors = vFs_dtf_descs;
	dev->function.hs_descriptors = vHs_dtf_descs;
	dev->function.bind = dtf_function_bind;
	dev->function.unbind = dtf_function_unbind;
	dev->function.setup = dtf_function_setup;
	dev->function.set_alt = dtf_function_set_alt;
	dev->function.disable = dtf_function_disable;
	dev->function.suspend = dtf_function_suspend;
	dev->function.resume = dtf_function_resume;

	dev->mCtrl_ep_enbl = 0;
	dev->mData_ep_enbl = 0;

	ret = usb_add_function(c, &dev->function);

	return ret;
}

static int dtf_setup(void)
{
	struct dtf_dev  *dev;
	int ret;

	dev = kzalloc(sizeof(*dev), GFP_KERNEL);
	if (!dev)
		return -ENOMEM;

	spin_lock_init(&dev->lock);

	_dtf_dev = dev;

	ret = dtf_if_init();
	if (ret)
		goto err;

	return 0;

err:
	kfree(dev);
	return ret;
}

static void dtf_complete_in(struct usb_ep *ep, struct usb_request *req)
{
	dtf_if_out_complete_in(req->status);
}

static void dtf_complete_intr(struct usb_ep *ep, struct usb_request *req)
{
	dtf_if_out_complete_intr(req->status);
}

static void dtf_complete_out(struct usb_ep *ep, struct usb_request *req)
{
	struct dtf_dev *dev = _dtf_dev;

	dtf_if_out_complete_out(req->status, (int)req->actual,
					(char *)req->buf);

	if (dev->mData_ep_enbl == 1) {
		dev->pg.mReq_out->length = DTF_MAX_PACKET_SIZE;
		if (usb_ep_queue(dev->pg.ep_out, dev->pg.mReq_out, GFP_ATOMIC))
			printk(KERN_ERR "[DTF] %s:usb_ep_queue error.\n",
				__func__);
	}
}

static int dtf_function_bind(struct usb_configuration *c,
					struct usb_function *f)
{
	struct usb_composite_dev *cdev = c->cdev;
	struct dtf_dev *dev = func_to_dtf(f);
	int ret;

	dev->cdev = cdev;

	/* allocate Interface IDs */
	ret = dtf_allocate_interface_ids(c, f);
	if (ret < 0)
		return ret;

	/* allocate endpoints */
	ret = dtf_allocate_endpoints(c, f);
	if (ret < 0)
		return ret;
	return 0;
}

static void dtf_function_disable(struct usb_function *f)
{
	struct dtf_dev *dev = func_to_dtf(f);
	int speed_check;

	if (dev->mCtrl_ep_enbl == 1) {
		dev->mCtrl_ep_enbl = 0;
		usb_ep_disable(dev->pg.ep_intr);
	}

	if (dev->mData_ep_enbl == 1) {
		dev->mData_ep_enbl = 0;
		usb_ep_dequeue(dev->pg.ep_out, dev->pg.mReq_out);
		usb_ep_disable(dev->pg.ep_in);
		usb_ep_disable(dev->pg.ep_out);
	}

	speed_check = (_dtf_dev->cdev->gadget->speed
				== USB_SPEED_HIGH) ? (1) : (0);
	dtf_if_out_disable(speed_check);

}

static void dtf_function_resume(struct usb_function *f)
{
	dtf_if_out_resume();
}

static int dtf_function_set_alt(struct usb_function *f,
					unsigned intf, unsigned alt)
{
	struct dtf_dev *dev = func_to_dtf(f);
	struct usb_composite_dev *cdev = f->config->cdev;
	int ret;
	int set_alt_end = 0;
	int speed_check = 0;

	if (dev->pg.mCtrl_id == intf) {
		ret = config_ep_by_speed(cdev->gadget, f, dev->pg.ep_intr);
		if (ret)
			return ret;
		ret = usb_ep_enable(dev->pg.ep_intr);
		if (ret)
			return ret;
		dev->mCtrl_ep_enbl = 1;
	} else if (dev->pg.mData_id == intf) {
		ret = config_ep_by_speed(cdev->gadget, f, dev->pg.ep_in);
		if (ret)
			return ret;
		ret = usb_ep_enable(dev->pg.ep_in);
		if (ret)
			return ret;
		ret = config_ep_by_speed(cdev->gadget, f, dev->pg.ep_out);
		if (ret)
			return ret;
		ret = usb_ep_enable(dev->pg.ep_out);
		if (ret) {
			usb_ep_disable(dev->pg.ep_in);
			return ret;
		}

		dev->pg.mReq_out->length = DTF_MAX_PACKET_SIZE;
		ret = usb_ep_queue(dev->pg.ep_out, dev->pg.mReq_out,
						GFP_ATOMIC);
		if (ret) {
			usb_ep_disable(dev->pg.ep_in);
			usb_ep_disable(dev->pg.ep_out);
			return ret;
		}
		dev->mData_ep_enbl = 1;

	} else {
	}
	set_alt_end = (
		(dev->mCtrl_ep_enbl) & (dev->mData_ep_enbl));
	speed_check = (dev->cdev->gadget->speed == USB_SPEED_HIGH) ? (1) : (0);

	if (set_alt_end == 1)
		dtf_if_out_set_alt(speed_check);

	return 0;
}

static int dtf_function_setup(struct usb_function *f,
					const struct usb_ctrlrequest *ctrl)
{
	struct dtf_if_ctrlrequest ctrlrequest;

	ctrlrequest.bRequestType = ctrl->bRequestType;
	ctrlrequest.bRequest = ctrl->bRequest;
	ctrlrequest.wValue = ctrl->wValue;
	ctrlrequest.wIndex = ctrl->wIndex;
	ctrlrequest.wLength = ctrl->wLength;

	dtf_if_out_setup(&ctrlrequest);

	return 0;
}

static void dtf_function_suspend(struct usb_function *f)
{
	dtf_if_out_suspend();
}

static void dtf_function_unbind(struct usb_configuration *c,
					struct usb_function *f)
{
	struct dtf_dev *dev = func_to_dtf(f);
	unsigned long	flags;

	spin_lock_irqsave(&dev->lock, flags);
	dtf_request_free(dev->pg.mReq_intr, dev->pg.ep_intr);
	dtf_request_free(dev->pg.mReq_in, dev->pg.ep_in);
	dtf_request_free(dev->pg.mReq_out, dev->pg.ep_out);
	spin_unlock_irqrestore(&dev->lock, flags);
}

static void dtf_cleanup(void)
{
	kfree(_dtf_dev);
	_dtf_dev = NULL;
}

static void dtf_request_free(struct usb_request *req, struct usb_ep *ep)
{
	if (req) {
		kfree(req->buf);
		usb_ep_free_request(ep, req);
	}
}

static struct usb_request *dtf_request_new(struct usb_ep *ep, int buffer_size)
{
	struct usb_request *req;

	req = usb_ep_alloc_request(ep, GFP_KERNEL);
	if (!req)
		return NULL;

	/* now allocate buffers for the requests */
	req->buf = kmalloc(buffer_size, GFP_KERNEL);
	if (!req->buf) {
		usb_ep_free_request(ep, req);
		return NULL;
	}

	return req;
}

static inline struct dtf_dev *func_to_dtf(struct usb_function *f)
{
	return container_of(f, struct dtf_dev, function);
}

/* DTF IF */
int dtf_if_in_intr_in(unsigned size, const char *data)
{
	struct usb_ep   *ep;
	struct usb_request *req = NULL;
	int ret;

	ep = _dtf_dev->pg.ep_intr;
	req = _dtf_dev->pg.mReq_intr;

	req->length = size;
	memcpy(req->buf, data, size);

	ret = usb_ep_queue(ep, req, GFP_KERNEL);

	return ret;
}

int dtf_if_in_bulk_in(unsigned size, const char *data)
{
	struct usb_ep   *ep;
	struct usb_request *req = NULL;
	int ret;

	ep = _dtf_dev->pg.ep_in;
	req = _dtf_dev->pg.mReq_in;

	req->length = size;
	memcpy(req->buf, data, size);

	if ((_dtf_dev->cdev->gadget->speed == USB_SPEED_FULL)
		&& (size % ep->maxpacket == 0)) {
		/* set zero flag to send ZeroLengthPacket(ZLP) */
		printk(KERN_INFO "packet size is equal to maxpacket and USB1.1 .\n");
		req->zero = 1;
	} else {
		/* NOT send ZeroLengthPacket(ZLP) */
		req->zero = 0;
	}
	ret = usb_ep_queue(ep, req, GFP_KERNEL);

	return ret;
}

void dtf_if_in_set_halt_intr_in(void)
{
	usb_ep_set_halt(_dtf_dev->pg.ep_intr);
}

void dtf_if_in_set_halt_bulk_in(void)
{
	usb_ep_set_halt(_dtf_dev->pg.ep_in);
}

void dtf_if_in_set_halt_out(void)
{
	usb_ep_set_halt(_dtf_dev->pg.ep_out);
}

void dtf_if_in_clear_halt_intr_in(void)
{
	usb_ep_clear_halt(_dtf_dev->pg.ep_intr);
}

void dtf_if_in_clear_halt_bulk_in(void)
{
	usb_ep_clear_halt(_dtf_dev->pg.ep_in);
}

void dtf_if_in_clear_halt_out(void)
{
	usb_ep_clear_halt(_dtf_dev->pg.ep_out);
}

void dtf_if_in_ctrl_in(int length, const char *data)
{
	struct dtf_dev  *dev = _dtf_dev;
	struct usb_composite_dev *cdev = dev->cdev;
	struct usb_request  *req = cdev->req;
	int ret;

	req->zero = 0;
	req->length = length;
	if (length > 0)
		memcpy(req->buf, data, length);

	ret = usb_ep_queue(cdev->gadget->ep0, req, GFP_KERNEL);
}

static void dtf_ctrl_complete(struct usb_ep *ep, struct usb_request *req)
{
	dtf_if_out_ctrl_complete(req->length, req->actual, (char *)req->buf);
}


void dtf_if_in_ctrl_out(int length)
{
	struct dtf_dev  *dev = _dtf_dev;
	struct usb_composite_dev *cdev = dev->cdev;
	struct usb_request  *req = cdev->req;
	int ret;

	cdev->gadget->ep0->driver_data = dev;
	req->complete = dtf_ctrl_complete;

	req->zero = 0;
	req->length = length;
	ret = usb_ep_queue(cdev->gadget->ep0, req, GFP_KERNEL);
}

int dtf_if_init(void)
{
	int ret;

	init_waitqueue_head(&poll_wait_read);

	dtf_if_init_read_data();

	ret = misc_register(&dtf_device);
	return ret;
}

void dtf_if_out_setup(const struct dtf_if_ctrlrequest *ctrlrequest)
{
	struct dtf_if_read_data read_data;

	if (!atomic_read(&_dtf_dev->dtf_if_active))
		return ;

	/* Event ID set */
	read_data.event_id = DTF_IF_EVENT_SETUP;
	/* Event Data set */
	read_data.ctrl.bRequestType = ctrlrequest->bRequestType;
	read_data.ctrl.bRequest = ctrlrequest->bRequest;
	read_data.ctrl.wValue = ctrlrequest->wValue;
	read_data.ctrl.wIndex = ctrlrequest->wIndex;
	read_data.ctrl.wLength = ctrlrequest->wLength;

	dtf_if_add_read_data(&read_data);

	if (waitqueue_active(&poll_wait_read))
		wake_up_interruptible(&poll_wait_read);
}

void dtf_if_out_set_alt(int speed_check)
{
	struct dtf_if_read_data read_data;

	if (!atomic_read(&_dtf_dev->dtf_if_active))
		return ;

	/* Event ID set */
	read_data.event_id = DTF_IF_EVENT_SET_ALT;
	/* Event Data set */
	read_data.speed_check = speed_check;
	dtf_if_add_read_data(&read_data);

	if (waitqueue_active(&poll_wait_read))
		wake_up_interruptible(&poll_wait_read);
}

void dtf_if_out_disable(int speed_check)
{
	struct dtf_if_read_data read_data;

	if (!atomic_read(&_dtf_dev->dtf_if_active))
		return ;

	/* Event ID set */
	read_data.event_id = DTF_IF_EVENT_DISABLE;
	/* Event Data set */
	read_data.speed_check = speed_check;
	dtf_if_add_read_data(&read_data);

	if (waitqueue_active(&poll_wait_read))
		wake_up_interruptible(&poll_wait_read);
}

void dtf_if_out_suspend(void)
{
	struct dtf_if_read_data read_data;

	if (!atomic_read(&_dtf_dev->dtf_if_active))
		return ;

	/* Event ID set */
	read_data.event_id = DTF_IF_EVENT_SUSPEND;

	dtf_if_add_read_data(&read_data);

	if (waitqueue_active(&poll_wait_read))
		wake_up_interruptible(&poll_wait_read);
}

void dtf_if_out_resume(void)
{
	struct dtf_if_read_data read_data;

	if (!atomic_read(&_dtf_dev->dtf_if_active))
		return ;

	/* Event ID set */
	read_data.event_id = DTF_IF_EVENT_RESUME;
	dtf_if_add_read_data(&read_data);

	if (waitqueue_active(&poll_wait_read))
		wake_up_interruptible(&poll_wait_read);
}

void dtf_if_out_complete_in(int status)
{
	struct dtf_if_read_data read_data;

	if (!atomic_read(&_dtf_dev->dtf_if_active))
		return ;

	/* Event ID set */
	read_data.event_id = DTF_IF_EVENT_COMPLETE_IN;

	/* Event Data set */
	read_data.status = status;
	dtf_if_add_read_data(&read_data);

	if (waitqueue_active(&poll_wait_read))
		wake_up_interruptible(&poll_wait_read);
}

void dtf_if_out_complete_out(int status, int actual, char *buf)
{
	struct dtf_if_read_data read_data;

	if (!atomic_read(&_dtf_dev->dtf_if_active))
		return ;
	if (actual > DTF_IF_READ_DATA_SIZE) {
		printk(KERN_ERR
		    "[DTF] dtf_if_out_complete_outDTF_IF_READ_DATA_SIZE OVER\n");
		return ;
	}
	/* Event ID set */
	read_data.event_id = DTF_IF_EVENT_COMPLETE_OUT;

	/* Event Data set */
	read_data.status = status;
	read_data.actual = actual;
	memcpy(read_data.data, buf, actual);

	dtf_if_add_read_data(&read_data);

	if (waitqueue_active(&poll_wait_read))
		wake_up_interruptible(&poll_wait_read);
}

void dtf_if_out_complete_intr(int status)
{
	struct dtf_if_read_data read_data;

	if (!atomic_read(&_dtf_dev->dtf_if_active))
		return ;

	/* Event ID set */
	read_data.event_id = DTF_IF_EVENT_COMPLETE_INTR;

	/* Event Data set */
	read_data.status = status;
	dtf_if_add_read_data(&read_data);

	if (waitqueue_active(&poll_wait_read))
		wake_up_interruptible(&poll_wait_read);
}

void dtf_if_out_ctrl_complete(int length, int actual, char *buf)
{
	struct dtf_if_read_data read_data;

	if (!atomic_read(&_dtf_dev->dtf_if_active))
		return ;

	/* Event ID set */
	read_data.event_id = DTF_IF_EVENT_CTRL_COMPLETE;

	/* Event Data set */
	read_data.length = length;
	read_data.actual = actual;
	memcpy(read_data.data, buf, actual);

	dtf_if_add_read_data(&read_data);

	if (waitqueue_active(&poll_wait_read))
		wake_up_interruptible(&poll_wait_read);
}

static void dtf_if_init_read_data(void)
{
	spin_lock_init(&lock_read_data);
	_dtf_dev->dtf_if_readable_num = 0;
	_dtf_dev->dtf_if_readable_head = 0;
	_dtf_dev->dtf_if_readable_tail = 0;
}

static void dtf_if_add_read_data(struct dtf_if_read_data *read_data)
{
	unsigned long	flags;

	spin_lock_irqsave(&lock_read_data, flags);

	if (_dtf_dev->dtf_if_readable_num >= DTF_IF_READ_DATA_MAX) {
		printk(KERN_ERR
		    "[DTF] dtf_if_add_read_data DTF_IF_READ_DATA_MAX OVER\n");
		spin_unlock_irqrestore(&lock_read_data, flags);
		return;
	}

	_dtf_dev->dtf_if_readable_num++;
	memcpy(&_dtf_dev->_dtf_if_read_data[_dtf_dev->dtf_if_readable_tail],
		read_data , sizeof(struct dtf_if_read_data));
	_dtf_dev->dtf_if_readable_tail++;
	if (_dtf_dev->dtf_if_readable_tail >= DTF_IF_READ_DATA_MAX)
		_dtf_dev->dtf_if_readable_tail = 0;

	spin_unlock_irqrestore(&lock_read_data, flags);
}

static struct dtf_if_read_data *dtf_if_get_read_data(void)
{
	struct dtf_if_read_data *read_data;

	if (!(_dtf_dev->dtf_if_readable_num))
		return NULL;
	read_data = &_dtf_dev->
			_dtf_if_read_data[_dtf_dev->dtf_if_readable_head];
	_dtf_dev->dtf_if_readable_head++;
	if (_dtf_dev->dtf_if_readable_head >= DTF_IF_READ_DATA_MAX)
		_dtf_dev->dtf_if_readable_head = 0;

	_dtf_dev->dtf_if_readable_num--;

	return read_data;
}

static int dtf_if_get_read_data_num(void)
{
	return _dtf_dev->dtf_if_readable_num;
}
