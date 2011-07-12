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

struct bcmpmu_adc {
	struct bcmpmu *bcmpmu;
	wait_queue_head_t wait;
	struct mutex lock;
	void (*adc_rtm_cb)(struct bcmpmu *, struct bcmpmu_adc_req *);
	struct bcmpmu_adc_req last_rtm_req;
	const struct bcmpmu_adc_map *adcmap;
};
static struct bcmpmu_adc *bcmpmu_adc;

static int adc_ioctl(struct inode *inode, struct file *file,
			unsigned int cmd, unsigned long arg);
static int adc_open(struct inode *inode, struct file *file);
static int adc_release(struct inode *inode, struct file *file);

static const struct file_operations adc_fops = {
	.owner		= THIS_MODULE,
	.open		= adc_open,
	.ioctl		= adc_ioctl,
	.release	= adc_release,
};

static struct miscdevice bcmpmu_adc_device = {
	MISC_DYNAMIC_MINOR, "bcmpmu_adc", &adc_fops
};


static int adc_ioctl(struct inode *inode, struct file *file,
			unsigned int cmd, unsigned long arg)
{
	struct bcmpmu_adc *data = file->private_data;
	int ret;

	if (data == NULL) {
		printk("bcmpmu adc_ioctl, no platform data.\n");
		return -EINVAL;
	}

	switch (cmd) {
	case PMU_ADC_REQ_RTM:
		break;
	case PMU_ADC_READ_RTM:
		break;
	case PMU_ADC_READ_HK:
		break;
	default:
		return -ENOIOCTLCMD;
	}
	return 0;
}

static int adc_open(struct inode *inode, struct file *file)
{
	file->private_data = bcmpmu_adc;
	return 0;
}

static int adc_release(struct inode *inode, struct file *file)
{
	file->private_data = NULL; 
	return 0;
}

static int read_adc_result(struct bcmpmu_adc *padc, struct bcmpmu_adc_req *req)
{
	int ret;
	unsigned int val;
	unsigned int val1;
	struct bcmpmu_adc_map adcmap;
	if (req->sig >= PMU_ADC_MAX) return -EINVAL;
	adcmap = padc->adcmap[req->sig];

	if ((adcmap.addr0 == 0) && (adcmap.addr1 == 0)) {
		printk("bcmpmu, adc_isr, sig map failed\n");
		req->data = -EINVAL;
		ret = -EINVAL;
	}
	else {
		ret = padc->bcmpmu->read_dev_drct(padc->bcmpmu,
			adcmap.addr0, &val, adcmap.dmask);
		if (ret != 0) {
			printk("bcmpmu: read_adc_result, map adc addr failed\n");
			req->data = -EINVAL;
		}
		if (adcmap.addr0 != adcmap.addr1) {
			ret = padc->bcmpmu->read_dev_drct(padc->bcmpmu,
				adcmap.addr1, &val1, adcmap.dmask);
			if (ret != 0) {
				printk("bcmpmu: read_adc_result map adc addr1 failed\n");
				req->data = -EINVAL;
			}
			val = (val & 0xFF) | ((val1 << 8) & 0xFF00);
		}
		req->data = val & adcmap.dmask;
	}
	return ret;
}

static int convert_adc_result(struct bcmpmu_adc *adc, struct bcmpmu_adc_req *req)
{
	return 0;
}

static void adc_isr(enum bcmpmu_irq irq, void *data)
{
	struct bcmpmu_adc *padc = data;
	int ret;
			
	if (irq == PMU_IRQ_RTM_DATA_RDY) {
		ret = read_adc_result(padc, &padc->last_rtm_req);
		if (ret < 0) {
			printk("bcmpmu, adc_isr, read rtm addr failed\n");
			padc->last_rtm_req.data = -EINVAL;
		}
	}
	else if ((irq == PMU_IRQ_RTM_IN_CON_MEAS) ||
		(irq == PMU_IRQ_RTM_UPPER) ||
		(irq == PMU_IRQ_RTM_IGNORE) ||
		(irq == PMU_IRQ_RTM_OVERRIDDEN)) {
		padc->last_rtm_req.data = -EINVAL;
	}
	else {
		printk("bcmpmu, adc_isr, interrupt unsupported, irq = %d\n", irq);
		return;
	}
	if (padc->adc_rtm_cb != NULL)
		padc->adc_rtm_cb(padc->bcmpmu, &padc->last_rtm_req);
}

static int bcmpmu_adc_request(struct bcmpmu *bcmpmu, struct bcmpmu_adc_req *req)
{
	struct bcmpmu_adc *padc = bcmpmu->adcinfo;
	int ret = -EINVAL;
	if (req->req == PMU_ADC_REQ_HK) {
		if (req->rslt == PMU_ADC_RSLT_RAW)
			ret = read_adc_result(padc, req);
		else if (req->rslt == PMU_ADC_RSLT_PHY) {
			ret = read_adc_result(padc, req);
			if (ret == 0)
				ret = convert_adc_result(padc, req);
		}
	}
	else if (req->req == PMU_ADC_REQ_RTM) {
		padc->last_rtm_req = *req;
		padc->adc_rtm_cb = req->adc_rtm_cb;
		/*todo: start rtm adc here*/
	}
	return ret;
}


static int __devinit adc_probe(struct platform_device *pdev)
{
	int ret;

	struct bcmpmu *bcmpmu = pdev->dev.platform_data;
	struct bcmpmu_adc *padc;
	
	printk("bcmpmu_adc: adc_probe called \n") ;

	padc = kzalloc(sizeof(struct bcmpmu_adc), GFP_KERNEL);
	if (padc == NULL) {
		printk("bcmpmu_adc: failed to alloc mem.\n") ;
		return -ENOMEM;
	}
	init_waitqueue_head(&padc->wait);
	mutex_init(&padc->lock);
	padc->bcmpmu = bcmpmu;
	padc->adcmap = bcmpmu_get_adcmap();
	padc->bcmpmu->adc_req = bcmpmu_adc_request;
	bcmpmu->adcinfo = (void *)padc;
	bcmpmu_adc = padc;
	
	misc_register(&bcmpmu_adc_device);
	
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_RTM_DATA_RDY, adc_isr, padc);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_RTM_IN_CON_MEAS, adc_isr, padc);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_RTM_UPPER, adc_isr, padc);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_RTM_IGNORE, adc_isr, padc);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_RTM_OVERRIDDEN, adc_isr, padc);

	return 0;

err:
	return ret;
}

static int __devexit adc_remove(struct platform_device *pdev)
{
	struct bcmpmu *bcmpmu = pdev->dev.platform_data;

	misc_deregister(&bcmpmu_adc_device);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_RTM_DATA_RDY);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_RTM_IN_CON_MEAS);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_RTM_UPPER);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_RTM_IGNORE);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_RTM_OVERRIDDEN);
	return 0;
}

static struct platform_driver bcmpmu_adc_driver = {
	.driver = {
		.name = "bcmpmu_adc",
	},
	.probe = adc_probe,
	.remove = __devexit_p(adc_remove),
};

static int __init adc_init(void)
{
	return platform_driver_register(&bcmpmu_adc_driver);
}
module_init(adc_init);

static int __exit adc_exit(void)
{
	platform_driver_unregister(&bcmpmu_adc_driver);
	return 0;
}
module_exit(adc_exit);

MODULE_DESCRIPTION("BCM PMIC adc driver");
MODULE_LICENSE("GPL");
