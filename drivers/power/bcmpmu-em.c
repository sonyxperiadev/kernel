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
#include <linux/poll.h>

#include <linux/mfd/bcmpmu.h>

struct bcmpmu_em {
	struct bcmpmu *bcmpmu;
	wait_queue_head_t wait;
	struct mutex lock;
};
static struct bcmpmu_em *bcmpmu_em;

static unsigned int em_poll(struct file *file, poll_table *wait);
static int em_ioctl(struct inode *inode, struct file *file,
			unsigned int cmd, unsigned long arg);
static int em_open(struct inode *inode, struct file *file);
static int em_release(struct inode *inode, struct file *file);

static const struct file_operations em_fops = {
	.owner		= THIS_MODULE,
	.open		= em_open,
	.ioctl		= em_ioctl,
	.release	= em_release,
	.poll		= em_poll,
};

static struct miscdevice bcmpmu_em_device = {
	MISC_DYNAMIC_MINOR, "bcmpmu_em", &em_fops
};

static int em_ioctl(struct inode *inode, struct file *file,
			unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	struct bcmpmu_em *data = file->private_data;
	struct bcmpmu_adc_req adc;
	unsigned long env_status;

	if (data == NULL) {
		printk("bcmpmu em_ioctl, no platform data.\n");
		return -EINVAL;
	}
	switch (cmd) {
	case PMU_EM_ADC_REQ:
		if (copy_from_user((void *)&adc, (void *)arg,
				sizeof(struct bcmpmu_adc_req)))
			return -EFAULT;
		ret = data->bcmpmu->adc_req(data->bcmpmu, &adc);
		if (ret)
			return ret;
		if (copy_to_user((void *)arg, (void *)&adc,
			sizeof(struct bcmpmu_adc_req)))
			return -EFAULT;
	
		break;
	case PMU_EM_ADC_LOAD_CAL:
		/* TBD */
		break;
	case PMU_EM_ENV_STATUS:
		data->bcmpmu->update_env_status(data->bcmpmu, &env_status);
		if (copy_to_user((void *)arg, (void *)&env_status,
			sizeof(unsigned long)))
			return -EFAULT;
		break;
	/* APIs TBD
	case PMU_EM_SET_CHARGE_CURRENT:
	case PMU_EM_SET_CHARGE_CURRENT_TRICKLE:
	case PMU_EM_SET_CHARGE_VOLTAGE:
	case PMU_EM_SET_EOC:
	case PMU_EM_SET_WDG:
	APIs TBD */
	
	default:
		return -ENOIOCTLCMD;
	}
	return ret;
}

static int em_open(struct inode *inode, struct file *file)
{
	file->private_data = bcmpmu_em;
	return 0;
}

static int em_release(struct inode *inode, struct file *file)
{
	file->private_data = NULL; 
	return 0;
}

static unsigned int em_poll(struct file *file, poll_table *wait)
{
	/* To handle inerrupts and other events */
}

static void em_isr(enum bcmpmu_irq irq, void *data)
{
	struct bcmpmu_batt *pem = data;
	int ret;
}

static int __devinit bcmpmu_em_probe(struct platform_device *pdev)
{
	int ret;

	struct bcmpmu *bcmpmu = pdev->dev.platform_data;
	struct bcmpmu_em *pem;
	
	printk(KERN_INFO "%s: called.\n", __func__);

	pem = kzalloc(sizeof(struct bcmpmu_em), GFP_KERNEL);
	if (pem == NULL) {
		printk(KERN_ERR "%s: failed to alloc mem.\n", __func__);
		return -ENOMEM;
	}
	init_waitqueue_head(&pem->wait);
	mutex_init(&pem->lock);
	pem->bcmpmu = bcmpmu;
	bcmpmu_em = pem;
	
	misc_register(&bcmpmu_em_device);
/*
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_BATINS, em_isr, pem);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_BATRM, em_isr, pem);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_GBAT_PLUG_IN, em_isr, pem);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_SMPL_INT, em_isr, pem);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_MBTEMPLOW, em_isr, pem);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_MBTEMPHIGH, em_isr, pem);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_MBOV, em_isr, pem);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_MBTEMPLOW, em_isr, pem);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_MBTEMPLOW, em_isr, pem);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_MBOV_DIS, em_isr, pem);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_MBWV_R_10S_WAIT, em_isr, pem);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_BBLOW, em_isr, pem);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_LOWBAT, em_isr, pem);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_VERYLOWBAT, em_isr, pem);
*/	
	return 0;

err:
	return ret;
}

static int __devexit bcmpmu_em_remove(struct platform_device *pdev)
{
	struct bcmpmu *bcmpmu = pdev->dev.platform_data;

	misc_deregister(&bcmpmu_em_device);
/*
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_BATINS);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_BATRM);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_GBAT_PLUG_IN);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_SMPL_INT);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_MBTEMPLOW);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_MBTEMPHIGH);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_MBOV);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_MBTEMPLOW);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_MBTEMPLOW);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_MBOV_DIS);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_MBWV_R_10S_WAIT);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_BBLOW);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_LOWBAT);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_VERYLOWBAT);
*/
	kfree(bcmpmu_em);

	return 0;
}

static struct platform_driver bcmpmu_em_driver = {
	.driver = {
		.name = "bcmpmu_em",
	},
	.probe = bcmpmu_em_probe,
	.remove = __devexit_p(bcmpmu_em_remove),
};

static int __init bcmpmu_em_init(void)
{
	return platform_driver_register(&bcmpmu_em_driver);
}
module_init(bcmpmu_em_init);

static int __exit bcmpmu_em_exit(void)
{
	platform_driver_unregister(&bcmpmu_em_driver);
	return 0;
}
module_exit(bcmpmu_em_exit);

MODULE_DESCRIPTION("BCM PMIC Battery Charging and Gauging");
MODULE_LICENSE("GPL");
