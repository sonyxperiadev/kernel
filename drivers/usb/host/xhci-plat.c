/*
 * xhci-plat.c - xHCI host controller driver platform Bus Glue.
 *
 * Copyright (C) 2012 Texas Instruments Incorporated - http://www.ti.com
 * Author: Sebastian Andrzej Siewior <bigeasy@linutronix.de>
 *
 * A lot of code borrowed from the Linux xHCI driver.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 */

#include <linux/platform_device.h>
#include <linux/pm_runtime.h>
#include <linux/module.h>
#include <linux/slab.h>

#include "xhci.h"

#define SYNOPSIS_DWC3_VENDOR	0x5533

static void xhci_plat_quirks(struct device *dev, struct xhci_hcd *xhci)
{
	struct xhci_plat_data *pdata = dev->platform_data;

	/*
	 * As of now platform drivers don't provide MSI support so we ensure
	 * here that the generic code does not try to make a pci_dev from our
	 * dev struct in order to setup MSI
	 */
	xhci->quirks |= XHCI_PLAT;

	if (!pdata)
		return;

	if (pdata->vendor == SYNOPSIS_DWC3_VENDOR && pdata->revision < 0x230A)
		xhci->quirks |= XHCI_PORTSC_DELAY;

	if (pdata->vendor == SYNOPSIS_DWC3_VENDOR && pdata->revision <= 0x250A)
		xhci->quirks |= XHCI_TR_DEQ_ERR_QUIRK;

	if (pdata->vendor == SYNOPSIS_DWC3_VENDOR && pdata->revision == 0x250A)
		xhci->quirks |= XHCI_RESET_DELAY;

	if (pdata->vendor == SYNOPSIS_DWC3_VENDOR && pdata->revision <= 0x250A)
		xhci->quirks |= XHCI_RESET_RS_ON_RESUME_QUIRK;
}

/* called during probe() after chip reset completes */
static int xhci_plat_setup(struct usb_hcd *hcd)
{
	return xhci_gen_setup(hcd, xhci_plat_quirks);
}

static const struct hc_driver xhci_plat_xhci_driver = {
	.description =		"xhci-hcd",
	.product_desc =		"xHCI Host Controller",
	.hcd_priv_size =	sizeof(struct xhci_hcd *),

	/*
	 * generic hardware linkage
	 */
	.irq =			xhci_irq,
	.flags =		HCD_MEMORY | HCD_USB3 | HCD_SHARED,

	/*
	 * basic lifecycle operations
	 */
	.reset =		xhci_plat_setup,
	.start =		xhci_run,
	.stop =			xhci_stop,
	.shutdown =		xhci_shutdown,

	/*
	 * managing i/o requests and associated device resources
	 */
	.urb_enqueue =		xhci_urb_enqueue,
	.urb_dequeue =		xhci_urb_dequeue,
	.alloc_dev =		xhci_alloc_dev,
	.free_dev =		xhci_free_dev,
	.alloc_streams =	xhci_alloc_streams,
	.free_streams =		xhci_free_streams,
	.add_endpoint =		xhci_add_endpoint,
	.drop_endpoint =	xhci_drop_endpoint,
	.endpoint_reset =	xhci_endpoint_reset,
	.check_bandwidth =	xhci_check_bandwidth,
	.reset_bandwidth =	xhci_reset_bandwidth,
	.address_device =	xhci_address_device,
	.update_hub_device =	xhci_update_hub_device,
	.reset_device =		xhci_discover_or_reset_device,

	/*
	 * scheduling support
	 */
	.get_frame_number =	xhci_get_frame,

	/* Root hub support */
	.hub_control =		xhci_hub_control,
	.hub_status_data =	xhci_hub_status_data,
	.bus_suspend =		xhci_bus_suspend,
	.bus_resume =		xhci_bus_resume,
};

static ssize_t config_imod_store(struct device *pdev,
		struct device_attribute *attr, const char *buff, size_t size)
{
	struct usb_hcd *hcd = dev_get_drvdata(pdev);
	struct xhci_hcd *xhci;
	u32 temp;
	u32 imod;
	unsigned long flags;

	if (sscanf(buff, "%u", &imod) != 1)
		return 0;

	imod &= ER_IRQ_INTERVAL_MASK;
	xhci = hcd_to_xhci(hcd);

	if (xhci->shared_hcd->state == HC_STATE_SUSPENDED
		&& hcd->state == HC_STATE_SUSPENDED)
		return -EACCES;

	spin_lock_irqsave(&xhci->lock, flags);
	temp = xhci_readl(xhci, &xhci->ir_set->irq_control);
	temp &= ~ER_IRQ_INTERVAL_MASK;
	temp |= imod;
	xhci_writel(xhci, temp, &xhci->ir_set->irq_control);
	spin_unlock_irqrestore(&xhci->lock, flags);

	return size;
}

static ssize_t config_imod_show(struct device *pdev,
		struct device_attribute *attr, char *buff)
{
	struct usb_hcd *hcd = dev_get_drvdata(pdev);
	struct xhci_hcd *xhci;
	u32 temp;
	unsigned long flags;

	xhci = hcd_to_xhci(hcd);

	if (xhci->shared_hcd->state == HC_STATE_SUSPENDED
		&& hcd->state == HC_STATE_SUSPENDED)
		return -EACCES;

	spin_lock_irqsave(&xhci->lock, flags);
	temp = xhci_readl(xhci, &xhci->ir_set->irq_control) &
			ER_IRQ_INTERVAL_MASK;
	spin_unlock_irqrestore(&xhci->lock, flags);

	return snprintf(buff, PAGE_SIZE, "%08u\n", temp);
}

static DEVICE_ATTR(config_imod, S_IRUGO | S_IWUSR,
		config_imod_show, config_imod_store);

static int xhci_plat_probe(struct platform_device *pdev)
{
	const struct hc_driver	*driver;
	struct xhci_hcd		*xhci;
	struct resource         *res;
	struct usb_hcd		*hcd;
	int			ret;
	int			irq;

	if (usb_disabled())
		return -ENODEV;

	driver = &xhci_plat_xhci_driver;

	irq = platform_get_irq(pdev, 0);
	if (irq < 0)
		return -ENODEV;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res)
		return -ENODEV;

	hcd = usb_create_hcd(driver, &pdev->dev, dev_name(&pdev->dev));
	if (!hcd)
		return -ENOMEM;

	hcd_to_bus(hcd)->skip_resume = true;
	hcd->rsrc_start = res->start;
	hcd->rsrc_len = resource_size(res);

	if (!request_mem_region(hcd->rsrc_start, hcd->rsrc_len,
				driver->description)) {
		dev_dbg(&pdev->dev, "controller already in use\n");
		ret = -EBUSY;
		goto put_hcd;
	}

	hcd->regs = ioremap_nocache(hcd->rsrc_start, hcd->rsrc_len);
	if (!hcd->regs) {
		dev_dbg(&pdev->dev, "error mapping memory\n");
		ret = -EFAULT;
		goto release_mem_region;
	}

	if (pdev->dev.parent)
		pm_runtime_resume(pdev->dev.parent);

	pm_runtime_use_autosuspend(&pdev->dev);
	pm_runtime_set_autosuspend_delay(&pdev->dev, 1000);
	pm_runtime_set_active(&pdev->dev);
	pm_runtime_enable(&pdev->dev);
	pm_runtime_get_sync(&pdev->dev);

	ret = usb_add_hcd(hcd, irq, IRQF_SHARED);
	if (ret)
		goto unmap_registers;

	/* USB 2.0 roothub is stored in the platform_device now. */
	hcd = dev_get_drvdata(&pdev->dev);
	xhci = hcd_to_xhci(hcd);
	xhci->shared_hcd = usb_create_shared_hcd(driver, &pdev->dev,
			dev_name(&pdev->dev), hcd);
	if (!xhci->shared_hcd) {
		ret = -ENOMEM;
		goto dealloc_usb2_hcd;
	}

	hcd_to_bus(xhci->shared_hcd)->skip_resume = true;
	/*
	 * Set the xHCI pointer before xhci_plat_setup() (aka hcd_driver.reset)
	 * is called by usb_add_hcd().
	 */
	*((struct xhci_hcd **) xhci->shared_hcd->hcd_priv) = xhci;

	ret = usb_add_hcd(xhci->shared_hcd, irq, IRQF_SHARED);
	if (ret)
		goto put_usb3_hcd;

	ret = device_create_file(&pdev->dev, &dev_attr_config_imod);
	if (ret)
		dev_err(&pdev->dev, "%s: unable to create imod sysfs entry\n",
					__func__);
	pm_runtime_put_autosuspend(&pdev->dev);

	return 0;

put_usb3_hcd:
	usb_put_hcd(xhci->shared_hcd);

dealloc_usb2_hcd:
	usb_remove_hcd(hcd);

unmap_registers:
	iounmap(hcd->regs);

release_mem_region:
	release_mem_region(hcd->rsrc_start, hcd->rsrc_len);

put_hcd:
	usb_put_hcd(hcd);

	return ret;
}

static int xhci_plat_remove(struct platform_device *dev)
{
	struct usb_hcd	*hcd = platform_get_drvdata(dev);
	struct xhci_hcd	*xhci = hcd_to_xhci(hcd);

	pm_runtime_disable(&dev->dev);

	device_remove_file(&dev->dev, &dev_attr_config_imod);
	xhci->xhc_state |= XHCI_STATE_REMOVING;

	usb_remove_hcd(xhci->shared_hcd);
	usb_put_hcd(xhci->shared_hcd);

	usb_remove_hcd(hcd);
	iounmap(hcd->regs);
	release_mem_region(hcd->rsrc_start, hcd->rsrc_len);
	usb_put_hcd(hcd);
	kfree(xhci);

	return 0;
}

#ifdef CONFIG_PM_RUNTIME
static int xhci_plat_runtime_idle(struct device *dev)
{
	if (pm_runtime_autosuspend_expiration(dev)) {
		pm_runtime_autosuspend(dev);
		return -EAGAIN;
	}

	return 0;
}

static int xhci_plat_runtime_suspend(struct device *dev)
{
	struct usb_hcd *hcd = dev_get_drvdata(dev);
	struct xhci_hcd *xhci = hcd_to_xhci(hcd);

	if (!xhci)
		return 0;

	dev_dbg(dev, "xhci-plat runtime suspend\n");

	return xhci_suspend(xhci);
}

static int xhci_plat_runtime_resume(struct device *dev)
{
	struct usb_hcd *hcd = dev_get_drvdata(dev);
	struct xhci_hcd *xhci = hcd_to_xhci(hcd);
	int ret;

	if (!xhci)
		return 0;

	dev_dbg(dev, "xhci-plat runtime resume\n");

	ret = xhci_resume(xhci, false);
	pm_runtime_mark_last_busy(dev);

	return ret;
}
#endif

static const struct dev_pm_ops xhci_plat_pm_ops = {
	SET_RUNTIME_PM_OPS(xhci_plat_runtime_suspend, xhci_plat_runtime_resume,
			   xhci_plat_runtime_idle)
};

static struct platform_driver usb_xhci_driver = {
	.probe	= xhci_plat_probe,
	.remove	= xhci_plat_remove,
	.driver	= {
		.name = "xhci-hcd",
		.pm = &xhci_plat_pm_ops,
	},
};
MODULE_ALIAS("platform:xhci-hcd");

int xhci_register_plat(void)
{
	return platform_driver_register(&usb_xhci_driver);
}

void xhci_unregister_plat(void)
{
	platform_driver_unregister(&usb_xhci_driver);
}
