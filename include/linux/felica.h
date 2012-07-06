/* kernel/drivers/misc/felica.h
 *
 * Copyright (C) 2010-2011 Sony Ericsson Mobile Communications AB.
 *
 * Author: Hiroaki Kuriyama <Hiroaki.Kuriyama@sonyericsson.com>
 * Author: Satomi Watanabe <Satomi.Watanabe@sonyericsson.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef _SEMC_FELICA_H
#define _SEMC_FELICA_H

#include <linux/device.h>
#include <linux/miscdevice.h>
#include <linux/switch.h>

#define FELICA_DEV_NAME "semc_felica"

/* FeliCa Device structures */
struct felica_dev {
	struct list_head	node;
	struct device	*dev;
	struct miscdevice	device_cen;
	struct miscdevice	device_pon;
	struct miscdevice	device_rfs;
	struct miscdevice	device_rws;
	struct felica_cen_pfdata	*flcen;
	struct felica_pon_pfdata	*flpon;
	struct felica_rfs_pfdata	*flrfs;
	struct felica_int_pfdata	*flint;
	struct switch_dev	swdev;
	unsigned int st_usbcon;
	unsigned int st_airplane;
	unsigned int ta_rwusb;
};

/* FeliCa Platform Data structures */
struct felica_cen_pfdata {
	int	(*cen_init)(struct felica_dev *dev);
	int	(*cen_read)(u8 *, struct felica_dev *dev);
	int	(*cen_write)(u8, struct felica_dev *dev);
};

struct felica_pon_pfdata {
	int	(*pon_init)(struct felica_dev *dev);
	void	(*pon_write)(int, struct felica_dev *dev);
	void	(*pon_release)(struct felica_dev *dev);
	int	(*tvdd_on)(struct felica_dev *dev);
	void	(*tvdd_off)(struct felica_dev *dev);
};

struct felica_rfs_pfdata {
	int	(*rfs_init)(struct felica_dev *dev);
	int	(*rfs_read)(struct felica_dev *dev);
	void	(*rfs_release)(struct felica_dev *dev);
};

struct felica_int_pfdata {
	int	(*int_init)(struct felica_dev *dev);
	int	(*int_read)(struct felica_dev *dev);
	void	(*int_release)(struct felica_dev *dev);
	unsigned int irq_int;
};

struct felica_platform_data {
	struct felica_cen_pfdata	cen_pfdata;
	struct felica_pon_pfdata	pon_pfdata;
	struct felica_rfs_pfdata	rfs_pfdata;
	struct felica_int_pfdata	int_pfdata;
	int (*gpio_init)(struct felica_dev *dev);
	int (*reg_release)(struct felica_dev *dev);
};

#endif
