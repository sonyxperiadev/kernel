/*
 * Platform data for Android USB
 *
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
#ifndef	__LINUX_USB_BRCM_H
#define	__LINUX_USB_BRCM_H

#include <linux/usb/composite.h>
#include <linux/if_ether.h>

struct brcm_usb_function {
	struct list_head	list;
	char			*name;
	int 			(*bind_config)(struct usb_configuration *c);
};

struct brcm_usb_product {
	/* Default product ID. */
	__u16 product_id;

	/* List of function names associated with this product.
	 * This is used to compute the USB product ID dynamically
	 * based on which functions are enabled.
	 */
	int num_functions;
	char **functions;
};

struct brcm_usb_platform_data {
	/* USB device descriptor fields */
	__u16 vendor_id;

	/* Default product ID. */
	__u16 product_id;

	__u16 version;

	char *product_name;
	char *manufacturer_name;
	char *serial_number;

	/* List of available USB products.
	 * This is used to compute the USB product ID dynamically
	 * based on which functions are enabled.
	 * if num_products is zero or no match can be found,
	 * we use the default product ID
	 */
	int num_products;
	struct brcm_usb_product *products;

	/* List of all supported USB functions.
	 * This list is used to define the order in which
	 * the functions appear in the configuration's list of USB interfaces.
	 * This is necessary to avoid depending upon the order in which
	 * the individual function drivers are initialized.
	 */
	int num_functions;
	char **functions;
};

/* Platform data for "usb_mass_storage" driver. */
struct usb_mass_storage_g_data {
	/* Contains values for the SC_INQUIRY SCSI command. */
	char *vendor;
	char *product;
	int release;

	/* number of LUNS */
	int nluns;
};

/* Platform data for USB ethernet driver. */
struct usb_ether_g_data {
	u8	ethaddr[ETH_ALEN];
	u32	vendorID;
	const char *vendorDescr;
};

extern void brcm_register_function(struct brcm_usb_function *f);

extern void brcm_enable_function(struct usb_function *f, int enable);

/* get the actual interface index under current configuration
 * interface index from set up command doesn't include disabled function
 * for example, if interface 0 and 1  are disabled and interface 2 is enabled.
 * set up command from host specifies interface
 */
extern u8 composite_actual_intf (struct usb_composite_dev *cdev, u8 intf);

#endif	/* __LINUX_USB_BRCM_H */
