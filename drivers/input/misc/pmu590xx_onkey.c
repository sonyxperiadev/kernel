/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2010  Broadcom Corporation                                                        */
/*                                                                                              */
/*     Unless you and Broadcom execute a separate written software license agreement governing  */
/*     use of this software, this software is licensed to you under the terms of the GNU        */
/*     General Public License version 2 (the GPL), available at                                 */
/*                                                                                              */
/*          http://www.broadcom.com/licenses/GPLv2.php                                          */
/*                                                                                              */
/*     with the following added to such license:                                                */
/*                                                                                              */
/*     As a special exception, the copyright holders of this software give you permission to    */
/*     link this software with independent modules, and to copy and distribute the resulting    */
/*     executable under terms of your choice, provided that you also meet, for each linked      */
/*     independent module, the terms and conditions of the license of that module.              */
/*     An independent module is a module which is not derived from this software.  The special  */
/*     exception does not apply to any modifications of the software.                           */
/*                                                                                              */
/*     Notwithstanding the above, under no circumstances may you combine this software in any   */
/*     way with any other Broadcom software provided under a license other than the GPL,        */
/*     without Broadcom's express prior written consent.                                        */
/*                                                                                              */
/************************************************************************************************/


#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/wait.h>
#include <linux/input.h>
#include <linux/mfd/bcm590xx/bcm59055_A0.h>
#include <linux/mfd/bcm590xx/core.h>


struct pmu590xx_onkey_info {
	struct input_dev	*idev;
	struct bcm590xx		*chip;
};

static void pmu590xx_onkey_handler(int irq, void *data)
{
	struct pmu590xx_onkey_info *info = data;
	int val = 0;

	switch(irq) {
		case BCM59055_IRQID_INT1_POK_PRESSED:
			val = 1;
			break;

		case BCM59055_IRQID_INT1_POK_RELEASED:
			val = 0;
			break;

		default:
			dev_err(info->chip->dev, "Invalid IRQ %d\n", irq);
			return;
			break;
	}		

	input_report_key(info->idev, KEY_POWER, val);
	input_sync(info->idev);
}

static int __devinit pmu590xx_onkey_probe(struct platform_device *pdev)
{
	struct bcm590xx *chip = dev_get_drvdata(pdev->dev.parent);
	struct pmu590xx_onkey_info *info;
	int error;

	info = kzalloc(sizeof(struct pmu590xx_onkey_info), GFP_KERNEL);
	if (!info) {
		dev_err(chip->dev, "Failed to allocate memory (%d bytes)\n", sizeof(struct pmu590xx_onkey_info));
		return -ENOMEM;
	}
	
	info->idev = input_allocate_device();
	if (!info->idev) {
		dev_err(chip->dev, "Failed to allocate input dev\n");
		error = -ENOMEM;
		goto out_input;
	}

	info->idev->name = "pmu590xx_on";
	info->idev->phys = "pmu590xx_on/input0";
	info->idev->dev.parent = &pdev->dev;
	info->idev->evbit[0] = BIT_MASK(EV_KEY);
	info->idev->keybit[BIT_WORD(KEY_POWER)] = BIT_MASK(KEY_POWER);
	info->chip = chip;
	
	/* Request PRESSED and RELEASED interrupts. 
	 */ 
    error = bcm590xx_request_irq(chip, BCM59055_IRQID_INT1_POK_PRESSED, true, pmu590xx_onkey_handler, info);
	if (error < 0) {
		dev_err(chip->dev, "Failed to request PMU590XX IRQ %d: %d\n", 
			BCM59055_IRQID_INT1_POK_PRESSED, error);
		goto out_irq_pressed;
	}

	error = bcm590xx_request_irq(chip, BCM59055_IRQID_INT1_POK_RELEASED, true, pmu590xx_onkey_handler, info);
	if (error < 0) {
		dev_err(chip->dev, "Failed to request PMU590XX IRQ %d: %d\n", 
			BCM59055_IRQID_INT1_POK_RELEASED, error);
		goto out_irq_released;
	}

	error = input_register_device(info->idev);
	if (error) {
		dev_err(chip->dev, "Can't register input device: %d\n", error);
		goto out;
	}

	platform_set_drvdata(pdev, info);

	return 0;

out:
	bcm590xx_free_irq(chip, BCM59055_IRQID_INT1_POK_RELEASED);
out_irq_released:
	bcm590xx_free_irq(chip, BCM59055_IRQID_INT1_POK_PRESSED);
out_irq_pressed:
	input_free_device(info->idev);
out_input:
	kfree(info);
	return error;
}

static int __devexit pmu590xx_onkey_remove(struct platform_device *pdev)
{
	struct pmu590xx_onkey_info *info = platform_get_drvdata(pdev);

	bcm590xx_free_irq(info->chip, BCM59055_IRQID_INT1_POK_PRESSED);
	bcm590xx_free_irq(info->chip, BCM59055_IRQID_INT1_POK_RELEASED);
	input_unregister_device(info->idev);
	kfree(info);

	platform_set_drvdata(pdev, NULL);

	return 0;
}

static struct platform_driver pmu590xx_onkey_driver = {
	.driver		= {
		.name	= "pmu590xx-onkey",
		.owner	= THIS_MODULE,
	},
	.probe		= pmu590xx_onkey_probe,
	.remove		= __devexit_p(pmu590xx_onkey_remove),
};

static int __init pmu590xx_onkey_init(void)
{
	return platform_driver_register(&pmu590xx_onkey_driver);
}

static void __exit pmu590xx_onkey_exit(void)
{
	platform_driver_unregister(&pmu590xx_onkey_driver);
}

/*
 *	module specification
 */

module_init(pmu590xx_onkey_init);
module_exit(pmu590xx_onkey_exit);

MODULE_AUTHOR("Randy Pan <rpan@broadcom.com>");
MODULE_DESCRIPTION("PMU590XX Power-On Key device driver");
MODULE_LICENSE("GPL");
