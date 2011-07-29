/*****************************************************************************
*  Copyright 2001 - 2011 Broadcom Corporation.  All rights reserved.
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2, available at
*  http://www.gnu.org/licenses/old-license/gpl-2.0.html (the "GPL").
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
*****************************************************************************/

#include <linux/module.h>
#include <linux/init.h>
#include <linux/stat.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <asm/io.h>
#include <mach/io_map.h>
#include <mach/rdb/brcm_rdb_hsotg_ctrl.h>
#include "bcm_hsotgctrl.h"

struct bcm_hsotgctrl_drv_data {
	struct device *dev;
	struct clk *otg_clk;
	void *hsotg_ctrl_base;
};

static struct bcm_hsotgctrl_drv_data *local_hsotgctrl_handle = NULL;

/**
 * Do OTG init. Currently only for testing dataline pulsing when Vbus is off
 */
static ssize_t do_hsotgctrlinit(struct device *dev,
			 struct device_attribute *attr,
			 const char *buf, size_t count)
{
	bcm_hsotgctrl_phy_set_vbus_stat(false);
	bcm_hsotgctrl_phy_set_non_driving(true);
	bcm_hsotgctrl_phy_set_non_driving(false);

	return count;
	
}
static DEVICE_ATTR(hsotgctrlinit, S_IWUSR, NULL, do_hsotgctrlinit);

static ssize_t dump_hsotgctrl(struct device *dev, 
	struct device_attribute *attr,
	char *buf)
{
	struct bcm_hsotgctrl_drv_data *hsotgctrl_drvdata = dev_get_drvdata(dev);
	void __iomem *hsotg_ctrl_base = hsotgctrl_drvdata->hsotg_ctrl_base;

	printk("\nusbotgcontrol: 0x%x", readl(hsotg_ctrl_base + HSOTG_CTRL_USBOTGCONTROL_OFFSET));
	printk("\nphy_cfg: 0x%x", readl(hsotg_ctrl_base + HSOTG_CTRL_PHY_CFG_OFFSET));
	printk("\nphy_p1ctl: 0x%x", readl(hsotg_ctrl_base + HSOTG_CTRL_PHY_P1CTL_OFFSET));
	printk("\nbc11_status: 0x%x", readl(hsotg_ctrl_base + HSOTG_CTRL_BC11_STATUS_OFFSET));
	printk("\nbc11_cfg: 0x%x", readl(hsotg_ctrl_base + HSOTG_CTRL_BC11_CFG_OFFSET));
	printk("\ntp_in: 0x%x", readl(hsotg_ctrl_base + HSOTG_CTRL_TP_IN_OFFSET));
	printk("\ntp_out: 0x%x", readl(hsotg_ctrl_base + HSOTG_CTRL_TP_OUT_OFFSET));
	printk("\nphy_ctrl: 0x%x", readl(hsotg_ctrl_base + HSOTG_CTRL_PHY_CTRL_OFFSET));
	printk("\nusbreg: 0x%x", readl(hsotg_ctrl_base + HSOTG_CTRL_USBREG_OFFSET));
	printk("\nusbproben: 0x%x", readl(hsotg_ctrl_base + HSOTG_CTRL_USBPROBEN_OFFSET));

	return sprintf(buf, "hsotgctrl register dump\n");
}
static DEVICE_ATTR(hsotgctrldump, S_IRUSR, dump_hsotgctrl, NULL);

static int __devinit bcm_hsotgctrl_probe(struct platform_device *pdev)
{
	int error = 0;
	struct resource *resource;
	struct bcm_hsotgctrl_drv_data *hsotgctrl_drvdata;

	resource = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (NULL == resource)
		return -EIO;

	hsotgctrl_drvdata = kzalloc(sizeof(*hsotgctrl_drvdata), GFP_KERNEL);
	if (!hsotgctrl_drvdata) {
		dev_warn(&pdev->dev, "Memory allocation failed\n");
		return -ENOMEM;
	}

	local_hsotgctrl_handle = hsotgctrl_drvdata;

	hsotgctrl_drvdata->dev = &pdev->dev;
	hsotgctrl_drvdata->otg_clk = clk_get(NULL, "usb_otg_clk");

	if (!hsotgctrl_drvdata->otg_clk) {
		dev_warn(&pdev->dev, "Clock allocation failed\n");
		kfree(hsotgctrl_drvdata);
		return -EIO;
	}

	hsotgctrl_drvdata->hsotg_ctrl_base = ioremap(resource->start, SZ_4K);
	if (!hsotgctrl_drvdata->hsotg_ctrl_base) {
		dev_warn(&pdev->dev, "IO remap failed\n");
		clk_put(hsotgctrl_drvdata->otg_clk);
		kfree(hsotgctrl_drvdata);
		return -ENOMEM;
	}

	platform_set_drvdata(pdev, hsotgctrl_drvdata);

	error = device_create_file(&pdev->dev, &dev_attr_hsotgctrldump);

	if (error)
	{
		dev_warn(&pdev->dev, "Failed to create HOST file\n");
		goto Error_bcm_hsotgctrl_probe;
	}

	error = device_create_file(&pdev->dev, &dev_attr_hsotgctrlinit);

	if (error)
	{
		dev_warn(&pdev->dev, "Failed to create phy_shutdown file\n");
		goto Error_bcm_hsotgctrl_probe;
	}

	return 0;

Error_bcm_hsotgctrl_probe:
	iounmap(hsotgctrl_drvdata->hsotg_ctrl_base);
	clk_put(hsotgctrl_drvdata->otg_clk);
	kfree(hsotgctrl_drvdata);
	return error;
}

static int bcm_hsotgctrl_remove(struct platform_device *pdev)
{
	struct bcm_hsotgctrl_drv_data *hsotgctrl_drvdata = platform_get_drvdata(pdev);

	device_remove_file(&pdev->dev, &dev_attr_hsotgctrldump);
	device_remove_file(&pdev->dev, &dev_attr_hsotgctrlinit);

	iounmap(hsotgctrl_drvdata->hsotg_ctrl_base);
	clk_put(hsotgctrl_drvdata->otg_clk);
	local_hsotgctrl_handle = NULL;
	kfree(hsotgctrl_drvdata);

	return 0;
}

static struct platform_driver bcm_hsotgctrl_driver = {
	.driver = {
		   .name = "bcm_hsotgctrl",
	},
	.probe = bcm_hsotgctrl_probe,
	.remove = bcm_hsotgctrl_remove,
};

static int __init bcm_hsotgctrl_init(void)
{
	pr_info("Broadcom USB HSOTGCTRL Driver\n");

	return platform_driver_register(&bcm_hsotgctrl_driver);
}
module_init(bcm_hsotgctrl_init);

static void __exit bcm_hsotgctrl_exit(void)
{
	platform_driver_unregister(&bcm_hsotgctrl_driver);
}
module_exit(bcm_hsotgctrl_exit);

int bcm_hsotgctrl_phy_set_vbus_stat(bool on)
{
	unsigned long val;
	void *hsotg_ctrl_base;

	if (NULL != local_hsotgctrl_handle)
		hsotg_ctrl_base = local_hsotgctrl_handle->hsotg_ctrl_base;
	else
		return -ENODEV;

	val = readl(hsotg_ctrl_base + HSOTG_CTRL_USBOTGCONTROL_OFFSET);

	if (on) {
		val |= (HSOTG_CTRL_USBOTGCONTROL_REG_OTGSTAT2_MASK |
			HSOTG_CTRL_USBOTGCONTROL_REG_OTGSTAT1_MASK);
	} else {
		val &= ~(HSOTG_CTRL_USBOTGCONTROL_REG_OTGSTAT2_MASK |
			 HSOTG_CTRL_USBOTGCONTROL_REG_OTGSTAT1_MASK);
	}

	writel(val, hsotg_ctrl_base + HSOTG_CTRL_USBOTGCONTROL_OFFSET);

	return 0;
}
EXPORT_SYMBOL_GPL(bcm_hsotgctrl_phy_set_vbus_stat);

int bcm_hsotgctrl_phy_set_non_driving(bool on)
{
	unsigned long val;
	void *hsotg_ctrl_base;

	if (NULL != local_hsotgctrl_handle)
		hsotg_ctrl_base = local_hsotgctrl_handle->hsotg_ctrl_base;
	else
		return -ENODEV;

	/* set Phy to driving mode */
	val = readl(hsotg_ctrl_base + HSOTG_CTRL_PHY_P1CTL_OFFSET);

	if (on)
		val |= HSOTG_CTRL_PHY_P1CTL_NON_DRIVING_MASK;
	else
		val &= ~HSOTG_CTRL_PHY_P1CTL_NON_DRIVING_MASK;

	writel(val, hsotg_ctrl_base + HSOTG_CTRL_PHY_P1CTL_OFFSET);
	return 0;
}
EXPORT_SYMBOL_GPL(bcm_hsotgctrl_phy_set_non_driving);

MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("USB HSOTGCTRL driver");
MODULE_LICENSE("GPL");
