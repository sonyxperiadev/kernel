/*****************************************************************************
* Copyright 2006 - 2010 Broadcom Corporation.  All rights reserved.
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

#include <linux/platform_device.h>
#include <linux/version.h>

#include <mach/irqs.h>
#include "bcm_usbh.h"

#define BCM_USBEHCI_MODULE_DESCRIPTION    "Broadcom USB EHCI driver"
#define BCM_USBEHCI_MODULE_VERSION        "1.0.0"

#define BCM_USBEHCI_IRQF_FLAGS            (IRQF_DISABLED | IRQF_SHARED)
#define BCM_USBEHCI_NAME                  "bcm-ehci"

#define EHCI_INFO(pdev, fmt, args...) dev_info(&pdev->dev, fmt, ## args)
#define EHCI_ERR(pdev, fmt, args...) dev_err(&pdev->dev, fmt, ## args)

struct usb_cfg
{
	void *virt_reg_base;
	struct usb_hcd *hcd;
};

extern int usb_disabled(void);
static int hcd_init(struct usb_hcd *hcd);

static const struct hc_driver ehci_hcd_driver =
{
	.description = hcd_name,
	.product_desc = BCM_USBEHCI_NAME,
	.hcd_priv_size = sizeof(struct ehci_hcd),
   
	/*
 	 * generic hardware linkage
	 */
	.irq = ehci_irq,
	.flags = HCD_USB2 | HCD_MEMORY | HCD_LOCAL_MEM,

	/*
	 * basic lifecycle operations
	 */
	.reset = hcd_init,
	.start = ehci_run,
	.stop = ehci_stop,
	.shutdown = ehci_shutdown,

	/*
	 * managing i/o requests and associated device resources
	 */
	.urb_enqueue = ehci_urb_enqueue,
	.urb_dequeue = ehci_urb_dequeue,
	.endpoint_disable = ehci_endpoint_disable,
	.endpoint_reset = ehci_endpoint_reset,

	/*
	 * scheduling support
	 */ 
	.get_frame_number = ehci_get_frame,

	/*
	 * root hub support
	 */
	.hub_status_data = ehci_hub_status_data,
	.hub_control = ehci_hub_control,
#ifdef CONFIG_PM
	.bus_suspend = ehci_bus_suspend,
	.bus_resume = ehci_bus_resume,
#endif
	.relinquish_port =  ehci_relinquish_port,
	.port_handed_over = ehci_port_handed_over,
	.clear_tt_buffer_complete = ehci_clear_tt_buffer_complete,
};

int hcd_init(struct usb_hcd *hcd)
{
	struct ehci_hcd *ehci;
	int err;

	if (!hcd) {
		printk(KERN_ERR "invalid hcd pointer in %s\n", __FUNCTION__);
		return -EINVAL;
	}

	ehci = hcd_to_ehci(hcd);

	if ((err = ehci_halt(ehci)) < 0) {
		printk(KERN_ERR "busnum %d: ehci_halt() failed, err=%d\n", hcd->self.busnum, err);
		return err;
	}
	
	if ((err = ehci_init(hcd)) < 0) {
		printk(KERN_ERR "busnum %d: ehci_init() failed, err=%d\n", hcd->self.busnum, err);
		return err;
	}

	/*
	 * Not sure why this is not set by ehci_init(). Convention seems to be
	 * to do it here for reasons unknown. This is a "packed release number".
	 */
	ehci->sbrn = 0x20;

	if ((err = ehci_reset(ehci)) < 0) {
		printk(KERN_ERR "busnum %d: ehci_reset() failed, err=%d\n", hcd->self.busnum, err);
		return err;
	}

	return 0;
}

int bcm_ehci_probe(struct platform_device *pdev)
{
	struct usb_cfg *usb;
	struct usb_hcd *hcd;
	struct ehci_hcd *ehci;
	struct resource *iomem, *ioarea;
	int ret, irq;
   
	if (usb_disabled())
		return -ENODEV;

	iomem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!iomem) {
		EHCI_ERR(pdev, "no mem resource\n");
		ret = -ENOMEM;
		goto err_exit;
	}

	/* get the irq info */
	irq = platform_get_irq(pdev, 0);
	if (!irq) {
		EHCI_ERR(pdev, "no irq resource\n");
		ret = -ENODEV;
		goto err_exit;
	}
	
	ioarea = request_mem_region(iomem->start, resource_size(iomem), pdev->name);
	if (!ioarea) {
		EHCI_ERR(pdev, "memory region already claimed\n");
		ret = -EBUSY;
		goto err_exit;
	}
	
	usb = kzalloc(sizeof(*usb), GFP_KERNEL);
	if (!usb) {
		EHCI_ERR(pdev, "unable to allocate memory for private data\n");
		ret = -ENOMEM;
		goto err_free_iomem;
	}

	usb->virt_reg_base = ioremap(iomem->start, resource_size(iomem));
	if (!usb->virt_reg_base) {
		EHCI_ERR(pdev, "ioremap failed\n");
		ret = -ENOMEM;
		goto err_free_private_mem;
	}

	/* enable clock and PHY */
	ret = bcm_usbh_init(pdev->id);
	if (ret < 0) {
		EHCI_ERR(pdev, "clock and PHY initialization failed\n");
		goto err_io_unmap;
	}

	hcd = usb_create_hcd(&ehci_hcd_driver, &pdev->dev, (char *)pdev->name);
	if (!hcd) {
		EHCI_ERR(pdev, "usb_create_hcd failed\n");
		ret = -ENOMEM;
		goto err_usb_term;
	}

	/* struct ehci_regs def'd in Linux ehci.h which is included by Linux ehci-hcd.c */
	usb->hcd = hcd;
	hcd->rsrc_start = (unsigned int)usb->virt_reg_base;
	hcd->rsrc_len = sizeof(struct ehci_regs);
	hcd->regs = usb->virt_reg_base;

	ehci = hcd_to_ehci(hcd);
	ehci->caps = hcd->regs;
	ehci->regs = hcd->regs + HC_LENGTH(ehci , readl(&ehci->caps->hc_capbase));
	/* cache this readonly data; minimize chip reads */
	ehci->hcs_params = ehci_readl(ehci, &ehci->caps->hcs_params);

	ret = usb_add_hcd(hcd, irq, BCM_USBEHCI_IRQF_FLAGS);
	if (ret) {
		EHCI_ERR(pdev, "usb_add_hcd failed\n");
		goto err_remove_hcd;
	}

	platform_set_drvdata(pdev, usb);

	EHCI_INFO(pdev, "probe done\n");
	return 0;
   
err_remove_hcd:
	usb_remove_hcd(hcd);
	usb_put_hcd(hcd);

err_usb_term:
	bcm_usbh_term(pdev->id);

err_io_unmap:
	iounmap(usb->virt_reg_base);

err_free_private_mem:
	kfree(usb);

err_free_iomem:
	release_mem_region(iomem->start, resource_size(iomem));

err_exit:
	EHCI_ERR(pdev, "probe failed: %d\n", ret);
	return ret;
}

int bcm_ehci_remove(struct platform_device *pdev)
{
	struct usb_cfg *usb = platform_get_drvdata(pdev);
	struct usb_hcd *hcd = usb->hcd;
	struct resource *iomem;

	usb_remove_hcd(hcd);
	usb_put_hcd(hcd);

	bcm_usbh_term(pdev->id);
	iounmap(usb->virt_reg_base);
	kfree(usb);

	iomem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	release_mem_region(iomem->start, resource_size(iomem));

	platform_set_drvdata(pdev, NULL);

	return 0;
}

static void bcm_ehci_shutdown(struct platform_device *pdev)
{
	struct usb_cfg *usb = platform_get_drvdata(pdev);
	struct usb_hcd *hcd = usb->hcd;
   
	if (hcd->driver->shutdown)
		hcd->driver->shutdown(hcd);
}

#ifdef CONFIG_PM
static int bcm_ehci_suspend(struct platform_device *pdev, pm_message_t message)
{
	struct usb_cfg *usb = platform_get_drvdata(pdev);
	struct usb_hcd *hcd = usb->hcd;
	struct ehci_hcd *ehci = hcd_to_ehci(hcd);
	unsigned long flags;
	int rc = 0;

	if (time_before(jiffies, ehci->next_statechange))
		msleep(10);

	/* Root hub was already suspended. Disable irq emission and
	 * mark HW unaccessible, bail out if RH has been resumed. Use
	 * the spinlock to properly synchronize with possible pending
	 * RH suspend or resume activity.
	 *
	 * This is still racy as hcd->state is manipulated outside of
	 * any locks =P But that will be a different fix.
	 */
	spin_lock_irqsave (&ehci->lock, flags);
	if (hcd->state != HC_STATE_SUSPENDED) {
		rc = -EINVAL;
		goto bail;
	}
	ehci_writel(ehci, 0, &ehci->regs->intr_enable);
	(void)ehci_readl(ehci, &ehci->regs->intr_enable);
   
	/* make sure snapshot being resumed re-enumerates everything */
	if (message.event == PM_EVENT_PRETHAW) {
		ehci_halt(ehci);
		ehci_reset(ehci);
	}

	clear_bit(HCD_FLAG_HW_ACCESSIBLE, &hcd->flags);
	rc = bcm_usbh_suspend(pdev->id);

bail:
	spin_unlock_irqrestore (&ehci->lock, flags);
   
	return rc;
}

static int bcm_ehci_resume(struct platform_device *pdev)
{
	struct usb_cfg *usb = platform_get_drvdata(pdev);
	struct usb_hcd *hcd = usb->hcd;
	struct ehci_hcd *ehci = hcd_to_ehci(hcd);
	int rc;
   
	if (time_before(jiffies, ehci->next_statechange))
		msleep(100);

	rc = bcm_usbh_resume(pdev->id);
   
	/* Mark hardware accessible again as we are out of D3 state by now */
	set_bit(HCD_FLAG_HW_ACCESSIBLE, &hcd->flags);
   
	/* If CF is still set, we maintained PCI Vaux power.
	 * Just undo the effect of ehci_pci_suspend().
	 */
	if (ehci_readl(ehci, &ehci->regs->configured_flag) == FLAG_CF) {
		int mask = INTR_MASK;

		if (!hcd->self.root_hub->do_remote_wakeup)
			mask &= ~STS_PCD;
		ehci_writel(ehci, mask, &ehci->regs->intr_enable);
		ehci_readl(ehci, &ehci->regs->intr_enable);
		return 0;
	}
   
	ehci_dbg(ehci, "lost power, restarting\n");
	usb_root_hub_lost_power(hcd->self.root_hub);
   
	/*
	 * Else reset, to cope with power loss or flush-to-storage
	 * style "resume" having let BIOS kick in during reboot.
	 */
	(void)ehci_halt(ehci);
	(void)ehci_reset(ehci);
   
	/* emptying the schedule aborts any urbs */
	spin_lock_irq(&ehci->lock);
	if (ehci->reclaim)
		end_unlink_async(ehci);
	ehci_work(ehci);
	spin_unlock_irq(&ehci->lock);

	ehci_writel(ehci, ehci->command, &ehci->regs->command);
	ehci_writel(ehci, FLAG_CF, &ehci->regs->configured_flag);
	ehci_readl(ehci, &ehci->regs->command); /* unblock posted writes */

	/* here we "know" root ports should always stay powered */
	ehci_port_power(ehci, 1);

	hcd->state = HC_STATE_SUSPENDED;
   
	return 0;
}

#else
#define bcm_ehci_suspend    NULL
#define bcm_ehci_resume     NULL
#endif /* CONFIG_PM */

static struct platform_driver ehci_bcm_driver =
{
	.probe = bcm_ehci_probe,
	.remove = bcm_ehci_remove,
	.shutdown = bcm_ehci_shutdown,
	.suspend = bcm_ehci_suspend,
	.resume = bcm_ehci_resume,
	.driver = {
		.name = BCM_USBEHCI_NAME,
		.owner = THIS_MODULE,
	},
};

MODULE_DESCRIPTION(BCM_USBEHCI_MODULE_DESCRIPTION);
MODULE_LICENSE("GPL");
MODULE_VERSION(BCM_USBEHCI_MODULE_VERSION);
