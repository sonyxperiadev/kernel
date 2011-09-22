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
#include <linux/usb.h>
#include <linux/usb/gadget.h>
#include <linux/usb/hcd.h>
#include <linux/err.h>
#include <linux/notifier.h>
#include <linux/slab.h>
#include <linux/clk.h>
#ifdef CONFIG_MFD_BCMPMU
#include <linux/mfd/bcmpmu.h>
#else
#include <linux/mfd/bcm590xx/core.h>
#endif

#include <asm/io.h>
#include <mach/io_map.h>
#include "bcm_hsotgctrl.h"
#include "bcm_otg_adp.h"
#include "bcmpmu_otg_xceiv.h"

#define OTGCTRL1_VBUS_ON 0xDC
#define OTGCTRL1_VBUS_OFF 0xD8

#define HOST_TO_PERIPHERAL_DELAY_MS 1000
#define PERIPHERAL_TO_HOST_DELAY_MS 100

static int bcm_otg_set_vbus(struct otg_transceiver *otg, bool enabled)
{
	struct bcm_otg_data *otg_data = dev_get_drvdata(otg->dev);
	int stat;

	/* The order of these operations has temporarily been
	 * swapped due to overcurrent issue caused by slow I2C
	 * operations. I2C operations take >200ms to complete */
	bcm_hsotgctrl_phy_set_vbus_stat(enabled);
	
	if (enabled) {
		dev_info(otg_data->dev, "Turning on VBUS\n");
		otg_data->vbus_enabled = true;
#ifdef CONFIG_MFD_BCMPMU
		stat =
		    otg_data->bcmpmu->usb_set(otg_data->bcmpmu,
					      BCMPMU_USB_CTRL_VBUS_ON_OFF,
					      1);
#else
		stat =
		    bcm590xx_reg_write(otg_data->bcm590xx,
				       BCM59055_REG_OTGCTRL1,
				       OTGCTRL1_VBUS_ON);
#endif
	} else {
		dev_info(otg_data->dev, "Turning off VBUS\n");
		otg_data->vbus_enabled = false;
#ifdef CONFIG_MFD_BCMPMU
		stat =
		    otg_data->bcmpmu->usb_set(otg_data->bcmpmu,
					      BCMPMU_USB_CTRL_VBUS_ON_OFF,
					      0);
#else
		stat =
		    bcm590xx_reg_write(otg_data->bcm590xx,
				       BCM59055_REG_OTGCTRL1,
				       OTGCTRL1_VBUS_OFF);
#endif
	}

	if (stat < 0)
		dev_warn(otg_data->dev, "Failed to set VBUS\n");
		
	return stat;
}

static int bcm_otg_set_peripheral(struct otg_transceiver *otg,
				  struct usb_gadget *gadget)
{
	struct bcm_otg_data *otg_data = dev_get_drvdata(otg->dev);

	dev_info(otg_data->dev, "Setting Peripheral\n");
	otg->gadget = gadget;

	return 0;
}

static int bcm_otg_set_host(struct otg_transceiver *otg,
			    struct usb_bus *host)
{
	struct bcm_otg_data *otg_data = dev_get_drvdata(otg->dev);

	dev_info(otg_data->dev, "Setting Host\n");
	otg->host = host;

	return 0;
}

static ssize_t bcm_otg_wake_store(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t count)
{
	struct usb_gadget *gadget;
	ssize_t result = 0;
	unsigned int val;
	struct bcm_otg_data *otg_data = dev_get_drvdata(dev);
	int error;

	gadget = otg_data->otg_xceiver.xceiver.gadget;

	result = sscanf(buf, "%u\n", &val);
	if (result != 1) {
		result = -EINVAL;
	} else if (val == 0) {
		dev_warn(otg_data->dev, "Illegal value\n");
	} else {
		dev_info(otg_data->dev, "Waking up host\n");
		error = usb_gadget_wakeup(gadget);
		if (error)
			dev_err(otg_data->dev,
				"Failed to issue wakeup\n");
	}

	return result < 0 ? result : count;
}
static DEVICE_ATTR(wake, S_IWUSR, NULL, bcm_otg_wake_store);

static ssize_t bcm_otg_vbus_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	struct bcm_otg_data *otg_data = dev_get_drvdata(dev);

	return sprintf(buf, "%s\n", otg_data->vbus_enabled ? "1" : "0");
}

static ssize_t bcm_otg_vbus_store(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t count)
{
	struct usb_hcd *hcd;
	ssize_t result = 0;
	unsigned int val;
	struct bcm_otg_data *otg_data = dev_get_drvdata(dev);
	int error;

	hcd = bus_to_hcd(otg_data->otg_xceiver.xceiver.host);

	result = sscanf(buf, "%u\n", &val);
	if (result != 1) {
		result = -EINVAL;
	} else if (val == 0) {
		dev_info(otg_data->dev, "Clearing PORT_POWER feature\n");
		error = hcd->driver->hub_control(hcd, ClearPortFeature,
						 USB_PORT_FEAT_POWER, 1, NULL,
						 0);
		if (error)
			dev_err(otg_data->dev,
				"Failed to clear PORT_POWER feature\n");
	} else {
		dev_info(otg_data->dev, "Setting PORT_POWER feature\n");
		error = hcd->driver->hub_control(hcd, SetPortFeature,
						 USB_PORT_FEAT_POWER, 1, NULL,
						 0);
		if (error)
			dev_err(otg_data->dev,
				"Failed to set PORT_POWER feature\n");
	}

	return result < 0 ? result : count;
}

static DEVICE_ATTR(vbus, S_IRUGO | S_IWUSR, bcm_otg_vbus_show,
		   bcm_otg_vbus_store);

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
		bcm_hsotgctrl_phy_set_id_stat(true);
		msleep(HOST_TO_PERIPHERAL_DELAY_MS);
		bcm_hsotgctrl_phy_set_vbus_stat(true);
	} else {
		dev_info(otg_data->dev, "Switching to Host\n");
		otg_data->host = true;
		bcm_hsotgctrl_phy_set_vbus_stat(false);
		msleep(PERIPHERAL_TO_HOST_DELAY_MS);
		bcm_hsotgctrl_phy_set_id_stat(false);
	}

	return result < 0 ? result : count;
}

static DEVICE_ATTR(host, S_IRUGO | S_IWUSR, bcm_otg_host_show,
		   bcm_otg_host_store);

static void bcm_otg_vbus_invalid_handler(struct work_struct *work)
{
	struct bcm_otg_data *otg_data = container_of(work, struct bcm_otg_data, bcm_otg_vbus_invalid_work);
	dev_info(otg_data->dev, "Vbus invalid\n");
}

static void bcm_otg_vbus_valid_handler(struct work_struct *work)
{
	struct bcm_otg_data *otg_data = container_of(work, struct bcm_otg_data, bcm_otg_vbus_valid_work);
	dev_info(otg_data->dev, "Vbus valid\n");
}

static void bcm_otg_vbus_a_invalid_handler(struct work_struct *work)
{
	struct bcm_otg_data *otg_data = container_of(work, struct bcm_otg_data, bcm_otg_vbus_a_invalid_work);
	dev_info(otg_data->dev, "A session invalid\n");
}

static void bcm_otg_vbus_a_valid_handler(struct work_struct *work)
{
	struct bcm_otg_data *otg_data = container_of(work, struct bcm_otg_data, bcm_otg_vbus_a_valid_work);
	dev_info(otg_data->dev, "A session valid\n");
}

static void bcm_otg_adp_cprb_done_handler(struct work_struct *work)
{
	struct bcm_otg_data *otg_data = container_of(work, struct bcm_otg_data, bcm_otg_adp_cprb_done_work);
	dev_info(otg_data->dev, "ADP calibration probe done\n");
}

static void bcm_otg_adp_change_handler(struct work_struct *work)
{
	struct bcm_otg_data *otg_data = container_of(work, struct bcm_otg_data, bcm_otg_adp_change_work);
	dev_info(otg_data->dev, "ADP change detected\n");
}

static int __devinit bcm_otg_probe(struct platform_device *pdev)
{
	int error = 0;
	struct bcm_otg_data *otg_data;
#ifdef CONFIG_MFD_BCMPMU
	struct bcmpmu *bcmpmu = pdev->dev.platform_data;
#else
	struct bcm590xx *bcm590xx = dev_get_drvdata(pdev->dev.parent);
#endif

	dev_info(&pdev->dev, "Probing started...\n");

	otg_data = kzalloc(sizeof(*otg_data), GFP_KERNEL);
	if (!otg_data) {
		dev_warn(&pdev->dev, "Memory allocation failed\n");
		return -ENOMEM;
	}

	otg_data->dev = &pdev->dev;
#ifdef CONFIG_MFD_BCMPMU
	otg_data->bcmpmu = bcmpmu;
#else
	otg_data->bcm590xx = bcm590xx;
#endif
	otg_data->otg_xceiver.xceiver.dev = otg_data->dev;
	otg_data->otg_xceiver.xceiver.label = "bcm_otg";
	otg_data->host = false;
	otg_data->vbus_enabled = false;

	/* Create a work queue for OTG work items */
	otg_data->bcm_otg_work_queue = create_workqueue("bcm_otg_events");
	if (otg_data->bcm_otg_work_queue == NULL) {
		dev_warn(&pdev->dev, "BCM OTG events work queue creation failed\n");
		return -ENOMEM;
	}

	/* Create one work item per deferrable function */
	INIT_WORK(&otg_data->bcm_otg_vbus_invalid_work, bcm_otg_vbus_invalid_handler);
	INIT_WORK(&otg_data->bcm_otg_vbus_valid_work, bcm_otg_vbus_valid_handler);
	INIT_WORK(&otg_data->bcm_otg_vbus_a_invalid_work, bcm_otg_vbus_a_invalid_handler);
	INIT_WORK(&otg_data->bcm_otg_vbus_a_valid_work, bcm_otg_vbus_a_valid_handler);
	INIT_WORK(&otg_data->bcm_otg_adp_cprb_done_work, bcm_otg_adp_cprb_done_handler);
	INIT_WORK(&otg_data->bcm_otg_adp_change_work, bcm_otg_adp_change_handler);

	otg_data->otg_clk = clk_get(NULL, "usb_otg_clk");
	if (!otg_data->otg_clk) {
		dev_warn(&pdev->dev, "Clock allocation failed\n");
		kfree(otg_data);
		return -EIO;
	}

	otg_data->otg_xceiver.xceiver.set_vbus = bcm_otg_set_vbus;
	otg_data->otg_xceiver.xceiver.set_peripheral = bcm_otg_set_peripheral;
	otg_data->otg_xceiver.xceiver.set_host = bcm_otg_set_host;
	otg_data->otg_xceiver.do_adp_calibration_probe = bcm_otg_do_adp_calibration_probe;
	otg_data->otg_xceiver.do_adp_probe = bcm_otg_do_adp_probe;
	otg_data->otg_xceiver.do_adp_sense = bcm_otg_do_adp_sense;
	otg_data->otg_xceiver.do_adp_sense_then_probe = bcm_otg_do_adp_sense_then_probe;
	otg_set_transceiver(&otg_data->otg_xceiver.xceiver);

	platform_set_drvdata(pdev, otg_data);

	error = device_create_file(&pdev->dev, &dev_attr_host);
	if (error) {
		dev_warn(&pdev->dev, "Failed to create HOST file\n");
		goto error_attr_host;;
	}

	error = device_create_file(&pdev->dev, &dev_attr_vbus);
	if (error) {
		dev_warn(&pdev->dev, "Failed to create VBUS file\n");
		goto error_attr_vbus;
	}

	error = device_create_file(&pdev->dev, &dev_attr_wake);
	if (error) {
		dev_warn(&pdev->dev, "Failed to create WAKE file\n");
		goto error_attr_wake;
	}

	dev_info(&pdev->dev, "Probing successful\n");
	return 0;

error_attr_wake:
	device_remove_file(otg_data->dev, &dev_attr_vbus);

error_attr_vbus:
	device_remove_file(otg_data->dev, &dev_attr_host);

error_attr_host:
	destroy_workqueue(otg_data->bcm_otg_work_queue);
	clk_put(otg_data->otg_clk);
	kfree(otg_data);
	return error;
}

static int __exit bcm_otg_remove(struct platform_device *pdev)
{
	struct bcm_otg_data *otg_data = platform_get_drvdata(pdev);

	device_remove_file(otg_data->dev, &dev_attr_wake);
	device_remove_file(otg_data->dev, &dev_attr_vbus);
	device_remove_file(otg_data->dev, &dev_attr_host);

	destroy_workqueue(otg_data->bcm_otg_work_queue);
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
