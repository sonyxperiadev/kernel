/*
 * f_gg.c -- USB gadget gordons gate driver for ETS
 *
 * Copyright (C) 2010-2012 by Sony Ericsson Mobile Communications AB
 * Copyright (C) 2012 Sony Mobile Communications AB.
 *
 * This software is distributed under the terms of the GNU General
 * Public License ("GPL") as published by the Free Software Foundation,
 * either version 2 of that License or (at your option) any later version.
 */

#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/usb/android.h>
#include "u_serial.h"

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
static inline struct f_gg *gg_func_to_gser(struct usb_function *f)
{
	return container_of(f, struct f_gg, port.func);
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
	.wMaxPacketSize =	__constant_cpu_to_le16(512),
};

static struct usb_endpoint_descriptor gg_gser_hs_out_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	__constant_cpu_to_le16(512),
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
	struct f_gg		*gser = gg_func_to_gser(f);
	struct usb_composite_dev *cdev = f->config->cdev;

	/* we know alt == 0, so this is an activation or a reset */

	if (gser->port.in->driver_data) {
		DBG(cdev, "reset generic ttyGS%d\n", gser->port_num);
		gserial_disconnect(&gser->port);
	} else
		DBG(cdev, "activate generic ttyGS%d\n", gser->port_num);

	gser->port.in_desc = ep_choose(cdev->gadget,
				gser->hs.in, gser->fs.in);
	gser->port.out_desc = ep_choose(cdev->gadget,
				gser->hs.out, gser->fs.out);

	gserial_connect(&gser->port, gser->port_num);
	printk(KERN_INFO "GG: gg_gser_set_alt: ttyGS%d\n", gser->port_num);
	return 0;
}

static void gg_gser_disable(struct usb_function *f)
{
	struct f_gg	*gser = gg_func_to_gser(f);

	DBG(f->config->cdev, "generic ttyGS%d deactivated\n", gser->port_num);
	gserial_disconnect(&gser->port);
}

/*-------------------------------------------------------------------------*/

/* serial function driver setup/binding */

static int gg_gser_bind(struct usb_configuration *c, struct usb_function *f)
{
	struct usb_composite_dev *cdev = c->cdev;
	struct f_gg		*gser = gg_func_to_gser(f);
	int			status;
	struct usb_ep		*ep;

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
	f->descriptors = usb_copy_descriptors(gg_gser_fs_function);

	gser->fs.in = usb_find_endpoint(gg_gser_fs_function,
			f->descriptors, &gg_gser_fs_in_desc);
	gser->fs.out = usb_find_endpoint(gg_gser_fs_function,
			f->descriptors, &gg_gser_fs_out_desc);


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

		gser->hs.in = usb_find_endpoint(gg_gser_hs_function,
				f->hs_descriptors, &gg_gser_hs_in_desc);
		gser->hs.out = usb_find_endpoint(gg_gser_hs_function,
				f->hs_descriptors, &gg_gser_hs_out_desc);
	}

	DBG(cdev, "generic ttyGS%d: %s speed IN/%s OUT/%s\n",
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

	printk(KERN_ERR "can't bind, err %s %d\n", f->name, status);

	return status;
}

static void gg_gser_unbind(struct usb_configuration *c, struct usb_function *f)
{
	if (gadget_is_dualspeed(c->cdev->gadget))
		usb_free_descriptors(f->hs_descriptors);
	usb_free_descriptors(f->descriptors);
	kfree(gg_func_to_gser(f));
}

static int gordonsg_bind_config(struct usb_configuration *c, u8 port_num)
{
	struct f_gg	*gser;
	int		status;

	/* maybe allocate device-global string ID */
	if (gg_gser_string_defs[0].id == 0) {
		status = usb_string_id(c->cdev);
		if (status < 0)
			return status;

		gg_gser_string_defs[0].id = status;
	}

	/* allocate and initialize one new instance */
	gser = kzalloc(sizeof *gser, GFP_KERNEL);
	if (!gser)
		return -ENOMEM;

	gser->port_num = port_num;

	gser->port.func.name = "gg";
	gser->port.func.strings = gg_gser_strings;
	gser->port.func.bind = gg_gser_bind;
	gser->port.func.unbind = gg_gser_unbind;
	gser->port.func.set_alt = gg_gser_set_alt;
	gser->port.func.disable = gg_gser_disable;

	status = usb_add_function(c, &gser->port.func);
	if (status)
		kfree(gser);

	return status;
}

static int gg_bind_config(struct usb_configuration *c)
{
	int status = 0;

	/* See if composite driver can allocate
	 * serial ports. But for now allocate
	 * two ports one for acm and another for gg.
	 */
	status = gserial_setup(c->cdev->gadget, 2);
	if (status)
		return status;

	status = gordonsg_bind_config(c, 1);
	return status;
}
