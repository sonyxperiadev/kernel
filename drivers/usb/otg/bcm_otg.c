/*****************************************************************************
*  Copyright 2001 - 2008 Broadcom Corporation.  All rights reserved.
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
#include <linux/usb/otg.h>
#include <linux/err.h>
#include <linux/notifier.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/mfd/bcm590xx/core.h>

#include <asm/io.h>
#include <mach/io_map.h>
#include <mach/rdb/brcm_rdb_hsotg_ctrl.h>

struct bcm_otg_data {
	struct device *dev;
	struct bcm590xx *bcm590xx;
	struct otg_transceiver xceiver;
	bool host;
	struct clk *otg_clk;
	void *hsotg_ctrl_base;
};

#define xceiver_to_data(x) container_of((x), struct bcm_otg_data, xceiver);

#define OTGCTRL1_VBUS_ON 0xDC
#define OTGCTRL1_VBUS_OFF 0xD8

static int bcm_otg_control_vbus(struct otg_transceiver *otg, bool enabled) ;

static void bcm_otg_phy_set_vbus_stat(struct bcm_otg_data *otg_data,
				      bool on)
{
	unsigned long reg;
	void *hsotg_ctrl_base = otg_data->hsotg_ctrl_base;

	reg = readl(hsotg_ctrl_base + HSOTG_CTRL_USBOTGCONTROL_OFFSET);

	if (on) {
		reg |= (HSOTG_CTRL_USBOTGCONTROL_REG_OTGSTAT2_MASK |
			HSOTG_CTRL_USBOTGCONTROL_REG_OTGSTAT1_MASK);
	} else {
		reg &= ~(HSOTG_CTRL_USBOTGCONTROL_REG_OTGSTAT2_MASK |
			 HSOTG_CTRL_USBOTGCONTROL_REG_OTGSTAT1_MASK);
	}

	writel(reg, hsotg_ctrl_base + HSOTG_CTRL_USBOTGCONTROL_OFFSET);
}

static void bcm_otg_phy_set_id_stat(struct bcm_otg_data *otg_data,
				    bool floating)
{
	unsigned long reg;
	void *hsotg_ctrl_base = otg_data->hsotg_ctrl_base;

	reg = readl(hsotg_ctrl_base + HSOTG_CTRL_USBOTGCONTROL_OFFSET);

	if (floating) {
		reg |= HSOTG_CTRL_USBOTGCONTROL_UTMIOTG_IDDIG_SW_MASK;
	} else {
		reg &= ~HSOTG_CTRL_USBOTGCONTROL_UTMIOTG_IDDIG_SW_MASK;
	}

	writel(reg, hsotg_ctrl_base + HSOTG_CTRL_USBOTGCONTROL_OFFSET);
}

static void bcm_otg_set_vbus(struct bcm_otg_data *otg_data,
			     bool on)
{
	int stat;

	if (on) {
		dev_info(otg_data->dev, "Turning on VBUS\n");
		stat =
		    bcm590xx_reg_write(otg_data->bcm590xx,
				       BCM59055_REG_OTGCTRL1,
				       OTGCTRL1_VBUS_ON);
	} else {
		dev_info(otg_data->dev, "Turning off VBUS\n");
		stat =
		    bcm590xx_reg_write(otg_data->bcm590xx,
				       BCM59055_REG_OTGCTRL1,
				       OTGCTRL1_VBUS_OFF);
	}

	if (stat < 0) {
		dev_warn(otg_data->dev, "Failed to set VBUS\n");
	}
}

static ssize_t bcm_otg_host_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	struct bcm_otg_data *otg_data = dev_get_drvdata(dev);

	return sprintf(buf, "%s\n", otg_data->host ? "1" : "0");
}

static ssize_t bcm_otg_host_store(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t count)
{
	ssize_t result = 0;
	unsigned int val;
	struct bcm_otg_data *otg_data = dev_get_drvdata(dev);

	result = sscanf(buf, "%u\n", &val);
	if (result != 1) {
		result = -EINVAL;
	} else if (val == 0) {
		dev_info(otg_data->dev, "Switching to Peripheral\n");
		otg_data->host = false;
		bcm_otg_phy_set_id_stat(otg_data, true);
		msleep(100);
		bcm_otg_set_vbus(otg_data, false);
		bcm_otg_phy_set_vbus_stat(otg_data, false);
		msleep(500);
		bcm_otg_phy_set_vbus_stat(otg_data, true);
	} else {
		dev_info(otg_data->dev, "Switching to Host\n");
		otg_data->host = true;
		bcm_otg_phy_set_vbus_stat(otg_data, false);
		msleep(100);
		bcm_otg_phy_set_id_stat(otg_data, false);
		msleep(500);
		bcm_otg_set_vbus(otg_data, true);
		bcm_otg_phy_set_vbus_stat(otg_data, true);
	}

	return result < 0 ? result : count;
}

static DEVICE_ATTR(host, S_IRUGO | S_IWUSR, bcm_otg_host_show,
		   bcm_otg_host_store);

static int bcm_otg_control_vbus(struct otg_transceiver *otg, bool enabled)
{
	struct bcm_otg_data *otg_data = dev_get_drvdata(otg->dev);

	if (NULL == otg_data)
		return -EINVAL;

	bcm_otg_set_vbus(otg_data, enabled);
	bcm_otg_phy_set_vbus_stat(otg_data, enabled);
	return 0;
}

static int __devinit bcm_otg_probe(struct platform_device *pdev)
{
	int error = 0;
	struct bcm_otg_data *otg_data;
	struct bcm590xx *bcm590xx = dev_get_drvdata(pdev->dev.parent);

	dev_info(&pdev->dev, "Probing started...\n");

	otg_data = kzalloc(sizeof(*otg_data), GFP_KERNEL);
	if (!otg_data) {
		dev_warn(&pdev->dev, "Memory allocation failed\n");
		return -ENOMEM;
	}

	otg_data->dev = &pdev->dev;
	otg_data->bcm590xx = bcm590xx;
	otg_data->xceiver.dev = otg_data->dev;
	otg_data->xceiver.label = "bcm_otg";
	otg_data->host = false;

	otg_data->otg_clk = clk_get(NULL, "usb_otg_clk");
	if (!otg_data->otg_clk) {
		dev_warn(&pdev->dev, "Clock allocation failed\n");
		kfree(otg_data);
		return -EIO;
	}

	otg_data->hsotg_ctrl_base = ioremap(HSOTG_CTRL_BASE_ADDR, SZ_4K);
	if (!otg_data->hsotg_ctrl_base) {
		dev_warn(&pdev->dev, "IO remap failed\n");
		clk_put(otg_data->otg_clk);
		kfree(otg_data);
		return -ENOMEM;
	}

	otg_data->xceiver.set_vbus = bcm_otg_control_vbus;
	otg_set_transceiver(&otg_data->xceiver);

	platform_set_drvdata(pdev, otg_data);

	error = device_create_file(&pdev->dev, &dev_attr_host);

	if (error)
	{
		dev_warn(&pdev->dev, "Failed to create HOST file\n");
		goto Error_bcm_otg_probe;
	}

	dev_info(&pdev->dev, "Probing successful\n");
	return 0;

Error_bcm_otg_probe:
	iounmap(otg_data->hsotg_ctrl_base);
	clk_put(otg_data->otg_clk);
	kfree(otg_data);
	return error;
}

static int __exit bcm_otg_remove(struct platform_device *pdev)
{
	struct bcm_otg_data *otg_data = platform_get_drvdata(pdev);

	device_remove_file(otg_data->dev, &dev_attr_host);

	iounmap(otg_data->hsotg_ctrl_base);
	clk_put(otg_data->otg_clk);
	kfree(otg_data);

	return 0;
}

static struct platform_driver bcm_otg_driver = {
	.probe = bcm_otg_probe,
	.remove = __exit_p(bcm_otg_remove),
	.driver = {
		   .name = "bcm_otg",
		   .owner = THIS_MODULE,
		   },
};

static int __init bcm_otg_init(void)
{
	pr_info("Broadcom USB OTG Transceiver Driver\n");

	return platform_driver_register(&bcm_otg_driver);
}
subsys_initcall(bcm_otg_init);

static void __exit bcm_otg_exit(void)
{
	platform_driver_unregister(&bcm_otg_driver);
}
module_exit(bcm_otg_exit);

MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("USB OTG transceiver driver");
MODULE_LICENSE("GPL");
