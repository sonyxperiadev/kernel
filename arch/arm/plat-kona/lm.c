/*
 *  Copyright (C) 2003 Deep Blue Solutions Ltd, All Rights Reserved.
 *  Copyright (C) 2007 Broadcom Ltd, All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/slab.h>

#include <mach/lm.h>

#define to_lm_device(d) container_of(d, struct lm_device, dev)
#define to_lm_driver(d) container_of(d, struct lm_driver, drv)

static int
lm_match(struct device *dev, struct device_driver *drv)
{
    printk(KERN_INFO "lm_match()\n");
    return 1;
}

static int
lm_bus_probe(struct device *dev)
{
    struct lm_device *lmdev = to_lm_device(dev);
    struct lm_driver *lmdrv = to_lm_driver(dev->driver);

    printk(KERN_INFO "lm_bus_probe()\n");
    return lmdrv->probe(lmdev);
}

static int
lm_bus_remove(struct device *dev)
{
    struct lm_device *lmdev = to_lm_device(dev);
    struct lm_driver *lmdrv = to_lm_driver(dev->driver);

    printk(KERN_INFO "lm_bus_remove()\n");
    if (lmdrv->remove)
        lmdrv->remove(lmdev);
    return 0;
}

static struct bus_type lm_bustype =
{
    .name    = "logicmodule",
    .match   = lm_match,
    .probe   = lm_bus_probe,
    .remove  = lm_bus_remove,
    /* .suspend = lm_bus_suspend, */
    /* .resume  = lm_bus_resume, */
};

static int __init
lm_init(void)
{
    printk(KERN_INFO "lm_init()\n");
    return bus_register(&lm_bustype);
}

postcore_initcall(lm_init);

int
lm_driver_register(struct lm_driver *drv)
{
    printk(KERN_INFO "lm_driver_register(): name=%s\n", drv->drv.name);
    drv->drv.bus = &lm_bustype;
    return driver_register(&drv->drv);
}

void
lm_driver_unregister(struct lm_driver *drv)
{
    printk(KERN_INFO "lm_driver_unregister()\n");
    driver_unregister(&drv->drv);
}

static void
lm_device_release(struct device *dev)
{
    struct lm_device *lmdev = to_lm_device(dev);

    printk(KERN_INFO "lm_device_release()\n");
    release_resource(&lmdev->resource);
    kfree(lmdev);
}

int
lm_device_register(struct lm_device *lmdev)
{
    int ret;

    printk(KERN_INFO "lm_device_register(): id=%d irq=%d start=0x%08x end=0x%08x\n", lmdev->id, lmdev->irq, lmdev->resource.start, lmdev->resource.end);

    lmdev->dev.release = lm_device_release;
    lmdev->dev.bus = &lm_bustype;
    dev_set_name (&lmdev->dev, "lm%d", lmdev->id);
    lmdev->resource.name = dev_name (&lmdev->dev);

    ret = request_resource(&iomem_resource, &lmdev->resource);
    if (ret == 0) {
        ret = device_register(&lmdev->dev);
        if (ret)
        {
            printk(KERN_ERR "lm_device_register(): device_register() failed, rc=%d\n", ret );
            release_resource(&lmdev->resource);
        }
    }
    else
        printk(KERN_ERR "lm_device_register(): request_resource() failed, rc=%d\n", ret );

    return ret;
}

void
lm_device_unregister(struct lm_device *lmdev)
{
    printk(KERN_INFO "lm_device_unregister()\n");
    device_unregister(&lmdev->dev);
}


EXPORT_SYMBOL(lm_device_register);
EXPORT_SYMBOL(lm_device_unregister);

EXPORT_SYMBOL(lm_driver_register);
EXPORT_SYMBOL(lm_driver_unregister);
