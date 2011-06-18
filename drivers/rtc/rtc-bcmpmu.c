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

struct bcmpmu_rtc {
	struct bcmpmu *bcmpmu;
	wait_queue_head_t wait;
	struct mutex lock;
};
static struct bcmpmu_rtc *bcmpmu_rtc;

static int bcmpmu_rtc_ioctl(struct inode *inode, struct file *file,
			unsigned int cmd, unsigned long arg);
static int bcmpmu_rtc_open(struct inode *inode, struct file *file);
static int bcmpmu_rtc_release(struct inode *inode, struct file *file);

static const struct file_operations bcmpmu_rtc_fops = {
	.owner		= THIS_MODULE,
	.open		= bcmpmu_rtc_open,
	.ioctl		= bcmpmu_rtc_ioctl,
	.release	= bcmpmu_rtc_release,
};

static struct miscdevice bcmpmu_rtc_device = {
	MISC_DYNAMIC_MINOR, "bcmpmu_rtc", &bcmpmu_rtc_fops
};


static int bcmpmu_rtc_ioctl(struct inode *inode, struct file *file,
			unsigned int cmd, unsigned long arg)
{
	struct bcmpmu_rtc *data = file->private_data;
	int ret;
	return 0;
}

static int bcmpmu_rtc_open(struct inode *inode, struct file *file)
{
	file->private_data = bcmpmu_rtc;
	return 0;
}

static int bcmpmu_rtc_release(struct inode *inode, struct file *file)
{
	file->private_data = NULL; 
	return 0;
}



static void bcmpmu_rtc_isr(enum bcmpmu_irq irq, void *data)
{
	struct bcmpmu_rtc *prtc = data;
	int ret;
			
	if (irq == PMU_IRQ_RTM_DATA_RDY) {
	}
	else if ((irq == PMU_IRQ_RTM_IN_CON_MEAS) ||
		(irq == PMU_IRQ_RTM_UPPER) ||
		(irq == PMU_IRQ_RTM_IGNORE) ||
		(irq == PMU_IRQ_RTM_OVERRIDDEN)) {
	}
	else {
	}
}


static int __devinit bcmpmu_rtc_probe(struct platform_device *pdev)
{
	int ret;

	struct bcmpmu *bcmpmu = pdev->dev.platform_data;
	struct bcmpmu_rtc *prtc;
	
	printk("bcmpmu_rtc: rtc_probe called \n") ;

	prtc = kzalloc(sizeof(struct bcmpmu_rtc), GFP_KERNEL);
	if (prtc == NULL) {
		printk("bcmpmu_rtc: failed to alloc mem.\n") ;
		return -ENOMEM;
	}
	init_waitqueue_head(&prtc->wait);
	mutex_init(&prtc->lock);
	prtc->bcmpmu = bcmpmu;
	bcmpmu->rtcinfo = (void *)prtc;
	bcmpmu_rtc = prtc;
	
	misc_register(&bcmpmu_rtc_device);

	bcmpmu->register_irq(bcmpmu, PMU_IRQ_RTC_ALARM, bcmpmu_rtc_isr, prtc);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_RTC_SEC, bcmpmu_rtc_isr, prtc);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_RTC_MIN, bcmpmu_rtc_isr, prtc);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_RTCADJ, bcmpmu_rtc_isr, prtc);

	return 0;

err:
	return ret;
}

static int __devexit bcmpmu_rtc_remove(struct platform_device *pdev)
{
	struct bcmpmu *bcmpmu = pdev->dev.platform_data;

	misc_deregister(&bcmpmu_rtc_device);

	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_RTC_ALARM);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_RTC_SEC);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_RTC_MIN);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_RTCADJ);

	return 0;
}

static struct platform_driver bcmpmu_rtc_driver = {
	.driver = {
		.name = "bcmpmu_rtc",
	},
	.probe = bcmpmu_rtc_probe,
	.remove = __devexit_p(bcmpmu_rtc_remove),
};

static int __init bcmpmu_rtc_init(void)
{
	return platform_driver_register(&bcmpmu_rtc_driver);
}
module_init(bcmpmu_rtc_init);

static int __exit bcmpmu_rtc_exit(void)
{
	platform_driver_unregister(&bcmpmu_rtc_driver);
	return 0;
}
module_exit(bcmpmu_rtc_exit);

MODULE_DESCRIPTION("BCM PMIC rtcery driver");
MODULE_LICENSE("GPL");
