/*
 * f_gg.c -- USB gadget gordons gate driver for ETS
 *
 * Copyright (C) 2013 Sony Mobile Communications AB.
 *
 * This software is distributed under the terms of the GNU General
 * Public License ("GPL") as published by the Free Software Foundation,
 * either version 2 of that License or (at your option) any later version.
 */

#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/err.h>

#include "u_serial.h"
#include "gadget_chips.h"

struct gg_gser_descs {
	struct usb_endpoint_descriptor	*in;
	struct usb_endpoint_descriptor	*out;
};

struct f_gg {
	struct gserial			port;
	u8				data_id;
	u8				port_num;

	struct gg_gser_descs		fs;
	struct gg_gser_descs		hs;
};

/*-------------------------------------------------------------------------*/
static inline struct f_gg *func_to_gg(struct usb_function *f)
{
	return container_of(f, struct f_gg, port.func);
}

static inline struct f_gg *port_to_gg(struct gserial *p)
{
	return container_of(p, struct f_gg, port);
}

/*-------------------------------------------------------------------------*/

/* interface descriptor: */

static struct usb_interface_descriptor gg_gser_interface_desc = {
	.bLength =		USB_DT_INTERFACE_SIZE,
	.bDescriptorType =	USB_DT_INTERFACE,
	/* .bInterfaceNumber = DYNAMIC */
	.bNumEndpoints =	2,
	.bInterfaceClass =	USB_CLASS_VENDOR_SPEC,
	.bInterfaceSubClass =	0,
	.bInterfaceProtocol =	0,
	/* .iInterface = DYNAMIC */
};

/* full speed support: */

static struct usb_endpoint_descriptor gg_gser_fs_in_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
};

static struct usb_endpoint_descriptor gg_gser_fs_out_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bEndpointAddress =	USB_DIR_OUT,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
};

static struct usb_descriptor_header *gg_gser_fs_function[] = {
	(struct usb_descriptor_header *) &gg_gser_interface_desc,
	(struct usb_descriptor_header *) &gg_gser_fs_in_desc,
	(struct usb_descriptor_header *) &gg_gser_fs_out_desc,
	NULL,
};

/* high speed support: */

static struct usb_endpoint_descriptor gg_gser_hs_in_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	cpu_to_le16(512),
};

static struct usb_endpoint_descriptor gg_gser_hs_out_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	cpu_to_le16(512),
};

static struct usb_descriptor_header *gg_gser_hs_function[] = {
	(struct usb_descriptor_header *) &gg_gser_interface_desc,
	(struct usb_descriptor_header *) &gg_gser_hs_in_desc,
	(struct usb_descriptor_header *) &gg_gser_hs_out_desc,
	NULL,
};

/* string descriptors: */

static struct usb_string gg_gser_string_defs[] = {
	[0].s = "Generic Serial",
	{  } /* end of list */
};

static struct usb_gadget_strings gg_gser_string_table = {
	.language =		0x0409,	/* en-us */
	.strings =		gg_gser_string_defs,
};

static struct usb_gadget_strings *gg_gser_strings[] = {
	&gg_gser_string_table,
	NULL,
};

/*-------------------------------------------------------------------------*/

static int gg_gser_set_alt(struct usb_function *f, unsigned intf, unsigned alt)
{
	struct f_gg		*gser = func_to_gg(f);
	struct usb_composite_dev *cdev = f->config->cdev;

	/* we know alt == 0, so this is an activation or a reset */

	if (gser->port.in->driver_data) {
		pr_info("reset gg ttyGS%d\n", gser->port_num);
		gserial_disconnect(&gser->port);
	}
	if (!gser->port.in->desc || !gser->port.out->desc) {
		pr_info("activate gg ttyGS%d\n", gser->port_num);
		if (config_ep_by_speed(cdev->gadget, f, gser->port.in) ||
		    config_ep_by_speed(cdev->gadget, f, gser->port.out)) {
			gser->port.in->desc = NULL;
			gser->port.out->desc = NULL;
			return -EINVAL;
		}
	}
	gserial_connect(&gser->port, gser->port_num);
	pr_info("gg_gser_set_alt: ttyGS%d\n", gser->port_num);
	return 0;
}

static void gg_gser_disable(struct usb_function *f)
{
	struct f_gg	*gser = func_to_gg(f);

	pr_info("gg ttyGS%d deactivated\n", gser->port_num);
	gserial_disconnect(&gser->port);
}

/*-------------------------------------------------------------------------*/

/* serial function driver setup/binding */

static int gg_gser_bind(struct usb_configuration *c, struct usb_function *f)
{
	struct usb_composite_dev *cdev = c->cdev;
	struct f_gg		*gser = func_to_gg(f);
	struct usb_string	*us;
	int			status;
	struct usb_ep		*ep;

	/* maybe allocate device-global string IDs, and patch descriptors */
	us = usb_gstrings_attach(cdev, gg_gser_strings,
			ARRAY_SIZE(gg_gser_string_defs));
	if (IS_ERR(us))
		return PTR_ERR(us);
	gg_gser_interface_desc.iInterface = us[0].id;

	/* allocate instance-specific interface IDs */
	status = usb_interface_id(c, f);
	if (status < 0)
		goto fail;
	gser->data_id = status;
	gg_gser_interface_desc.bInterfaceNumber = status;

	status = -ENODEV;

	/* allocate instance-specific endpoints */
	ep = usb_ep_autoconfig(cdev->gadget, &gg_gser_fs_in_desc);
	if (!ep)
		goto fail;
	gser->port.in = ep;
	ep->driver_data = cdev;	/* claim */

	ep = usb_ep_autoconfig(cdev->gadget, &gg_gser_fs_out_desc);
	if (!ep)
		goto fail;
	gser->port.out = ep;
	ep->driver_data = cdev;	/* claim */

	/* copy descriptors, and track endpoint copies */
	f->fs_descriptors = usb_copy_descriptors(gg_gser_fs_function);

	/*gser->fs.in = usb_find_endpoint(gg_gser_fs_function,
			f->descriptors, &gg_gser_fs_in_desc);
	gser->fs.out = usb_find_endpoint(gg_gser_fs_function,
			f->descriptors, &gg_gser_fs_out_desc);*/


	/* support all relevant hardware speeds... we expect that when
	 * hardware is dual speed, all bulk-capable endpoints work at
	 * both speeds
	 */
	if (gadget_is_dualspeed(c->cdev->gadget)) {
		gg_gser_hs_in_desc.bEndpointAddress =
				gg_gser_fs_in_desc.bEndpointAddress;
		gg_gser_hs_out_desc.bEndpointAddress =
				gg_gser_fs_out_desc.bEndpointAddress;

		/* copy descriptors, and track endpoint copies */
		f->hs_descriptors = usb_copy_descriptors(gg_gser_hs_function);

/*		gser->hs.in = usb_find_endpoint(gg_gser_hs_function,
				f->hs_descriptors, &gg_gser_hs_in_desc);
		gser->hs.out = usb_find_endpoint(gg_gser_hs_function,
				f->hs_descriptors, &gg_gser_hs_out_desc);*/
	}
	pr_info("gg ttyGS%d: %s speed IN/%s OUT/%s\n",
			gser->port_num,
			gadget_is_dualspeed(c->cdev->gadget) ? "dual" : "full",
			gser->port.in->name, gser->port.out->name);
	return 0;

fail:
	/* we might as well release our claims on endpoints */
	if (gser->port.out)
		gser->port.out->driver_data = NULL;
	if (gser->port.in)
		gser->port.in->driver_data = NULL;

	pr_err("can't bind, err %s %d\n", f->name, status);

	return status;
}

static void gg_gser_unbind(struct usb_configuration *c, struct usb_function *f)
{
	gg_gser_string_defs[0].id = 0;
	usb_free_all_descriptors(f);
}

static void gg_gser_free_func(struct usb_function *f)
{
	struct f_gg		*ggser = func_to_gg(f);

	kfree(ggser);
}

static struct usb_function *gg_gser_alloc_func(struct usb_function_instance *fi)
{
	struct f_serial_opts *opts;
	struct f_gg *gser;

	gser = kzalloc(sizeof(*gser), GFP_KERNEL);
	if (!gser)
		return ERR_PTR(-ENOMEM);

	gser->port.func.name = "gg";
	gser->port.func.strings = gg_gser_strings;
	/* descriptors are per-instance copies */
	gser->port.func.bind = gg_gser_bind;
	gser->port.func.set_alt = gg_gser_set_alt;
	gser->port.func.disable = gg_gser_disable;

	opts = container_of(fi, struct f_serial_opts, func_inst);
	gser->port_num = opts->port_num;
	gser->port.func.unbind = gg_gser_unbind;
	gser->port.func.free_func = gg_gser_free_func;

	return &gser->port.func;
}

static inline struct f_serial_opts *to_f_serial_opts(struct config_item *item)
{
	return container_of(to_config_group(item), struct f_serial_opts,
			func_inst.group);
}

CONFIGFS_ATTR_STRUCT(f_serial_opts);
static ssize_t f_gg_attr_show(struct config_item *item,
				 struct configfs_attribute *attr,
				 char *page)
{
	struct f_serial_opts *opts = to_f_serial_opts(item);
	struct f_serial_opts_attribute *f_serial_opts_attr =
		container_of(attr, struct f_serial_opts_attribute, attr);
	ssize_t ret = 0;

	if (f_serial_opts_attr->show)
		ret = f_serial_opts_attr->show(opts, page);
	return ret;
}

static void gg_gser_attr_release(struct config_item *item)
{
	struct f_serial_opts *opts = to_f_serial_opts(item);

	usb_put_function_instance(&opts->func_inst);
}

static struct configfs_item_operations gg_gser_item_ops = {
	.release                = gg_gser_attr_release,
	.show_attribute		= f_gg_attr_show,
};

static ssize_t f_gg_port_num_show(struct f_serial_opts *opts, char *page)
{
	return sprintf(page, "%u\n", opts->port_num);
}

static struct f_serial_opts_attribute f_gg_port_num =
	__CONFIGFS_ATTR_RO(port_num, f_gg_port_num_show);


static struct configfs_attribute *gg_gser_attrs[] = {
	&f_gg_port_num.attr,
	NULL,
};

static struct config_item_type gg_gser_func_type = {
	.ct_item_ops    = &gg_gser_item_ops,
	.ct_attrs	= gg_gser_attrs,
	.ct_owner       = THIS_MODULE,
};

static void gg_gser_free_instance(struct usb_function_instance *fi)
{
	struct f_serial_opts *opts;

	opts = container_of(fi, struct f_serial_opts, func_inst);
	gserial_free_line(opts->port_num);
	kfree(opts);
}

static struct usb_function_instance *gg_gser_alloc_instance(void)
{
	struct f_serial_opts *opts;
	int ret;

	opts = kzalloc(sizeof(*opts), GFP_KERNEL);
	if (!opts)
		return ERR_PTR(-ENOMEM);
	opts->func_inst.free_func_inst = gg_gser_free_instance;
	ret = gserial_alloc_line(&opts->port_num);
	if (ret) {
		kfree(opts);
		return ERR_PTR(ret);
	}
	config_group_init_type_name(&opts->func_inst.group, "",
			&gg_gser_func_type);
	return &opts->func_inst;
}
DECLARE_USB_FUNCTION_INIT(gg, gg_gser_alloc_instance, gg_gser_alloc_func);
MODULE_LICENSE("GPL");
