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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/bug.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>


#include <linux/mfd/bcmpmu.h>

struct bcmpmu_accy {
	struct bcmpmu *bcmpmu;
	wait_queue_head_t wait;
	struct mutex lock;
};

static void bcmpmu_accy_isr(enum bcmpmu_irq irq, void *data)
{
	struct bcmpmu_chrgr *paccy = data;
	int ret;
}

static int __devinit accy_probe(struct platform_device *pdev)
{
	int ret;

	int i;
	struct bcmpmu *bcmpmu = pdev->dev.platform_data;
	struct bcmpmu_accy *paccy;
	
	printk("bcmpmu_accy: accy_probe called \n") ;

	paccy = kzalloc(sizeof(struct bcmpmu_accy), GFP_KERNEL);
	if (paccy == NULL) {
		printk(KERN_INFO "%s: failed to alloc mem.\n", __func__);
		return -ENOMEM;
	}

	paccy->bcmpmu = bcmpmu;

	bcmpmu->register_irq(bcmpmu, PMU_IRQ_EOC, bcmpmu_accy_isr, paccy);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_USBOV_DIS, bcmpmu_accy_isr, paccy);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_USBINS, bcmpmu_accy_isr, paccy);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_USBRM, bcmpmu_accy_isr, paccy);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_CHGDET_LATCH, bcmpmu_accy_isr, paccy);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_CHGDET_TO, bcmpmu_accy_isr, paccy);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_VBUS_1V5_R, bcmpmu_accy_isr, paccy);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_VBUS_4V5_R, bcmpmu_accy_isr, paccy);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_VBUS_1V5_F, bcmpmu_accy_isr, paccy);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_VBUS_4V5_F, bcmpmu_accy_isr, paccy);

	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_EOC);
	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_USBOV_DIS);
	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_USBINS);
	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_USBRM);
	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_CHGDET_LATCH);
	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_CHGDET_TO);
	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_VBUS_1V5_R);
	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_VBUS_4V5_R);
	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_VBUS_1V5_F);
	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_VBUS_4V5_F);

	return 0;

err:
	return ret;
}

static int __devexit accy_remove(struct platform_device *pdev)
{
	struct bcmpmu *bcmpmu = pdev->dev.platform_data;

	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_EOC);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_USBOV_DIS);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_USBINS);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_USBRM);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_CHGDET_LATCH);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_CHGDET_TO);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_VBUS_1V5_R);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_VBUS_4V5_R);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_VBUS_1V5_F);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_VBUS_4V5_F);
	return 0;
}

static struct platform_driver bcmpmu_accy_driver = {
	.driver = {
		.name = "bcmpmu_accy",
	},
	.probe = accy_probe,
	.remove = __devexit_p(accy_remove),
};

static int __init accy_init(void)
{
	return platform_driver_register(&bcmpmu_accy_driver);
}
module_init(accy_init);

static int __exit accy_exit(void)
{
	platform_driver_unregister(&bcmpmu_accy_driver);
	return 0;
}
module_exit(accy_exit);

MODULE_DESCRIPTION("BCM PMIC accy driver");
MODULE_LICENSE("GPL");
