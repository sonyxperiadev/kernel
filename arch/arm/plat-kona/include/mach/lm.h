/*****************************************************************************
* Copyright 2006 - 2008 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/

#ifndef _LM_DEVICE_H_
#define _LM_DEVICE_H_

#include <linux/device.h>
#include <linux/ioport.h>

struct lm_device {
	struct device dev;
	struct resource resource;
	unsigned int irq;
	unsigned int id;
};

struct lm_driver {
	struct device_driver drv;
	int (*probe) (struct lm_device *);
	void (*remove) (struct lm_device *);
	int (*suspend) (struct lm_device *, pm_message_t);
	int (*resume) (struct lm_device *);
};

int lm_driver_register(struct lm_driver *drv);
void lm_driver_unregister(struct lm_driver *drv);

int lm_device_register(struct lm_device *dev);
void lm_device_unregister(struct lm_device *dev);

#define lm_get_drvdata(lm)	dev_get_drvdata(&(lm)->dev)
#define lm_set_drvdata(lm,d)	dev_set_drvdata(&(lm)->dev, d)

#endif /* _LM_DEVICE_H_ */
