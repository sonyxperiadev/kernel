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

#include <linux/errno.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
#include <linux/semaphore.h>
#include <linux/spinlock.h>

#include <asm/io.h>
#include <mach/irqs.h>
#include <mach/io_map.h>
#include <mach/rdb/brcm_rdb_hsotg_ctrl.h>

#include "kona_otg_cp.h"

#define DRIVER_NAME "kona-otg-cp"

/* set VBUS line monitoring workqueue work item to run every 100 ms */
#define VBUS_MONITOR_JIFFIES	(HZ / 10)

#define NUM_VBUS_IRQS           3

struct otg_drv {
	struct device *dev;
	struct semaphore cp_lock;
	spinlock_t spinlock;
	struct kona_otg_cp *cp;
	void __iomem *reg_base;
	struct workqueue_struct *workqueue;
	struct delayed_work work_vbus;
	int vbus_irq[NUM_VBUS_IRQS];
};

static struct otg_drv *gDrv = NULL;

#define KLOG_PREFIX			DRIVER_NAME

#define KLOG_ERROR(fmt, args...)	KLOG(KERN_ERR, fmt, ## args)
#define KLOG_WARN(fmt, args...)		KLOG(KERN_WARNING, fmt, ## args)
#define KLOG_NOTICE(fmt, args...)	KLOG(KERN_NOTICE, fmt, ## args)
#define KLOG_INFO(fmt, args...)		KLOG(KERN_INFO, fmt, ## args)

/* #define DEBUG */
/* #define DEBUG_VERBOSE */

#ifdef DEBUG
	/*
	 * Add any normal log messages (syslogd) to the kernel log as well. Makes things easier
	 * to interpret if the syslogd messages are interleaved in the knllog.
	 */
    	#define KDBG(fmt, args...)	/* KNLLOG(fmt, ## args) */
	#define KLOG(lvl, fmt, args...) \
					do \
					{ \
						/*KNLLOG(fmt, ## args);*/ \
						PRINTK(lvl, fmt, ## args); \
					} \
					while(0)
#else
    	#define KDBG(fmt...)		do{}while(0)
	#define KLOG			PRINTK
#endif

#define KDBG_ENTER()			KDBG_TRACE("enter\n")
#define KDBG_EXIT()			KDBG_TRACE("exit\n")

#ifdef DEBUG_VERBOSE
	#define KDBG_TRACE		KDBG
#else
	#define KDBG_TRACE(fmt...)	do{}while(0)
#endif

#define PRINTK(lvl, fmt, args...)	printk( lvl KLOG_PREFIX ": %s: " fmt, __FUNCTION__, ## args )

static irqreturn_t vbus_drive_isr(int irq, struct otg_drv *drv);
static irqreturn_t vbus_charge_isr(int irq, struct otg_drv *drv);
static irqreturn_t vbus_discharge_isr(int irq, struct otg_drv *drv);

static const char *vbus_irq_name[NUM_VBUS_IRQS] = {
	"OTG VBus Drive",
	"OTG VBus Charge",
	"OTG VBus Discharge",
};

static const irq_handler_t vbus_isr[NUM_VBUS_IRQS] = {
	(irq_handler_t)vbus_drive_isr,
	(irq_handler_t)vbus_charge_isr,
	(irq_handler_t)vbus_discharge_isr,
};

void otg_vbus_drive_work(struct work_struct *work)
{
	struct otg_drv *drv = gDrv;
	uint32_t status = 1;
	static unsigned cnt = 0;
	unsigned long val;
	void *gicdist_reg = (void *)KONA_GICDIST_VA;
	int tmp;
	uint32_t ulRegNum, ulBitNum, ulAddr;
	
	/* There is no Linux API to determine the interrupt line state, so
	 * have to use a CHAL API for this.
	 */
	tmp = BCM_INT_ID_USB_OTG_DRV_VBUS - BCM_INT_ID_PPI_MAX; 
        ulRegNum = tmp >> 5;
        ulBitNum = tmp & 0x1f;

        ulAddr = gicdist_reg + 0x0d04 + (ulRegNum * 4);
	val = readl(ulAddr);
        status = (val & (1 << ulBitNum)) ? 1 : 0;
	
	if (status) {
		/*
		 * UTMI+ VBUS signal still high. Re-schedule work.
		 * Only print debug periodically. Too noisy otherwise.
		 */
		if ((cnt & 0x3ff) == 0) {
			KDBG("BCM_INT_ID_USB_OTG_DRV_VBUS status=%d", status);
			cnt++;
		}
		queue_delayed_work(drv->workqueue,
				&drv->work_vbus,
				VBUS_MONITOR_JIFFIES);
	} else {
		/*
		 * UTMI+ VBUS signal has gone low. Request the external
		 * VBUS to be turned off, and re-enable the internal
		 * UTMI+ VBUS interrupt.
		 */
		cnt = 0;
		KDBG("BCM_INT_ID_USB_OTG_DRV_VBUS status=%d", status);
		drv->cp->ops->vbus_drive(drv->cp, false);
		enable_irq(drv->vbus_irq[0]);
	}
}

int kona_otg_cp_register(struct kona_otg_cp *cp)
{
	int i, rc;

	if (!cp)
		return -EINVAL;

	if (!gDrv)
		return -ENODEV;

	down(&gDrv->cp_lock);

	if (gDrv->cp) {
		dev_err(gDrv->dev, "already bound to %s\n",
				gDrv->cp->name);
		up(&gDrv->cp_lock);
		return -EBUSY;
	}

	gDrv->cp = cp;

	/*
	 * Create a strictly ordered work queue for monitoring USB OTG UTMI+
	 * VBUS line. Will only get an interrupt on a rising edge. Once
	 * this has occurred, have to poll monitor the line for a transition in
	 * the other direction. This is a nasty bug that needs to be fixed in
	 * the HW
	 */
	gDrv->workqueue = create_singlethread_workqueue(DRIVER_NAME);
	if (gDrv->workqueue == NULL) {
		dev_err(gDrv->dev, "unable to create workqueue\n");
		rc = -ENOEXEC;
		goto err_unbind_cp;
	}
	INIT_DELAYED_WORK(&gDrv->work_vbus, otg_vbus_drive_work);

	/* now request the VBus IRQ lines */
	for (i = 0; i < NUM_VBUS_IRQS; i++) {
		if (gDrv->vbus_irq[i] > 0) {
			rc = request_irq(gDrv->vbus_irq[i],
					vbus_isr[i],
					IRQF_TRIGGER_RISING,
					vbus_irq_name[i],
					gDrv);
			if (rc < 0)
				goto err_free_irq;
		}
	}

	up(&gDrv->cp_lock);
	return 0;

err_free_irq:
	for (i = 0; i < NUM_VBUS_IRQS; i++) {
		if (gDrv->vbus_irq[i] > 0)
			free_irq(gDrv->vbus_irq[i], gDrv);
	}

	destroy_workqueue(gDrv->workqueue);

err_unbind_cp:
	gDrv->cp = NULL;
	up(&gDrv->cp_lock);
	return rc;
}
EXPORT_SYMBOL(kona_otg_cp_register);

int kona_otg_cp_unregister(struct kona_otg_cp *cp)
{
	int i;

	if (!cp)
		return -EINVAL;

	if (!gDrv)
		return -ENODEV;

	down(&gDrv->cp_lock);

	if (!gDrv->cp) {
		dev_err(gDrv->dev, "not registered\n");
		up(&gDrv->cp_lock);
		return -EFAULT;
	}

	if (gDrv->cp != cp) {
		dev_err(gDrv->dev, "different cp registered\n");
		up(&gDrv->cp_lock);
		return -EINVAL;
	}
	
	for (i = 0; i < NUM_VBUS_IRQS; i++) {
		if (gDrv->vbus_irq[i] > 0)
			free_irq(gDrv->vbus_irq[i], gDrv);
	}
	destroy_workqueue(gDrv->workqueue);
	gDrv->cp = NULL;

	up(&gDrv->cp_lock);

	return 0;
}
EXPORT_SYMBOL(kona_otg_cp_unregister);

/*
 * To update the ID pin information. Can be called from any context
 */
int kona_otg_cp_id_change(const struct kona_otg_cp *cp, enum kona_otg_cp_id id)
{
	uint32_t tmp;
	unsigned long flags;

	if (!cp)
		return -EINVAL;

	if (!gDrv)
		return -ENODEV;

	if (!gDrv->cp) {
		dev_err(gDrv->dev, "not registered\n");
		return -EFAULT;
	}

	if (gDrv->cp != cp) {
		dev_err(gDrv->dev, "different cp registered\n");
		return -EINVAL;
	}

	spin_lock_irqsave(&gDrv->spinlock, flags);
	tmp = readl(gDrv->reg_base + HSOTG_CTRL_USBOTGCONTROL_OFFSET);

	switch (id) {
	case KONA_OTG_CP_ID_DEVICE_A:
		tmp &= ~(HSOTG_CTRL_USBOTGCONTROL_UTMIOTG_IDDIG_SW_MASK);
		break;

	case KONA_OTG_CP_ID_DEVICE_B:
	default:
		tmp |= (HSOTG_CTRL_USBOTGCONTROL_UTMIOTG_IDDIG_SW_MASK);
		break;
	}

	writel(tmp, gDrv->reg_base + HSOTG_CTRL_USBOTGCONTROL_OFFSET);
	spin_unlock_irqrestore(&gDrv->spinlock, flags);

	switch (id) {
	case KONA_OTG_CP_ID_DEVICE_A:
		KLOG_INFO("A-Device\n");
		break;

	case KONA_OTG_CP_ID_DEVICE_B:
	default:
		KLOG_INFO("B-Device\n");
		break;

	}
	KDBG("HSOTG_CTRL_USBOTGCONTROL reg write=0x%08x\n", tmp);

	return 0;
}
EXPORT_SYMBOL(kona_otg_cp_id_change);

/*
 * To report the VBus voltage information. Can be callde from any context
 */
int kona_otg_cp_volt_change(const struct kona_otg_cp *cp,
		enum kona_otg_cp_volt_state volt_state)
{
	uint32_t tmp;
	char *str;
	unsigned long flags;

	if (!cp)
		return -EINVAL;

	if (!gDrv)
		return -ENODEV;

	if (!gDrv->cp) {
		dev_err(gDrv->dev, "not registered\n");
		return -EFAULT;
	}

	if (gDrv->cp != cp) {
		dev_err(gDrv->dev, "different cp registered\n");
		return -EINVAL;
	}

#define STAT1 HSOTG_CTRL_USBOTGCONTROL_REG_OTGSTAT1_MASK
#define STAT2 HSOTG_CTRL_USBOTGCONTROL_REG_OTGSTAT2_MASK

	spin_lock_irqsave(&gDrv->spinlock, flags);
	tmp = readl(gDrv->reg_base + HSOTG_CTRL_USBOTGCONTROL_OFFSET);
	tmp &= ~(STAT1 | STAT2);
	tmp |= HSOTG_CTRL_USBOTGCONTROL_OTGSTAT_CTRL_MASK;

	switch (volt_state)
	{
	case KONA_OTG_CP_VOLT_VBUS_VALID:
		/* 4.5 <= VBUS */
		str = "VBUS valid: set STAT1 set STAT2\n";
		tmp |= (STAT1 | STAT2);
		break;

	case KONA_OTG_CP_VOLT_SESS_VALID:
		/* 1.5 <= VBUS < 4.5 */
		str = "session valid: clr STAT1 set STAT2\n";
		tmp |= (STAT2);
		break;

	case KONA_OTG_CP_VOLT_SESS_DETECT:
		/* 0.5 <= VBUS < 1.5 */
		str = "session detect: set STAT1 clr STAT2\n";
		tmp |= (STAT1);
		break;

	case KONA_OTG_CP_VOLT_SESS_END:
	default:
		/* VBUS < 0.5 */
		str = "session end: clr STAT1 clr STAT2\n";
		break;
	}

	writel(tmp, gDrv->reg_base + HSOTG_CTRL_USBOTGCONTROL_OFFSET);
	spin_unlock_irqrestore(&gDrv->spinlock, flags);

	KLOG_INFO("%s", str);
	KDBG("HSOTG_CTRL_USBOTGCONTROL reg write=0x%08x\n", tmp);

	return 0;
}
EXPORT_SYMBOL(kona_otg_cp_volt_change);

static irqreturn_t vbus_drive_isr(int irq, struct otg_drv *drv)
{
	if (drv->cp == NULL || drv->cp->ops->vbus_drive == NULL) {
		dev_err(gDrv->dev, "VBus drive control function not registered\n");
		disable_irq_nosync(irq);
		return IRQ_HANDLED;
	}
	
	drv->cp->ops->vbus_drive(drv->cp, true);
	disable_irq_nosync(drv->vbus_irq[0]);
	queue_delayed_work(drv->workqueue,
			&drv->work_vbus,
			VBUS_MONITOR_JIFFIES);
				
	return IRQ_HANDLED;
}

irqreturn_t vbus_charge_isr(int irq, struct otg_drv *otg)
{
	KLOG_ERROR("unexpected interrrupt\n");
	disable_irq_nosync(irq);
			
	return IRQ_HANDLED;
}

irqreturn_t vbus_discharge_isr(int irq, struct otg_drv *otg)
{
	KLOG_ERROR("unexpected interrrupt\n");
	disable_irq_nosync(irq);

	return IRQ_HANDLED;
}

static int __devinit otg_cp_probe(struct platform_device *pdev)
{
	int i, rc = -EFAULT;
	struct otg_drv *drv;
	struct resource *iomem;

	if (gDrv) {
		dev_err(&pdev->dev, "probe has been called\n");
		return -EBUSY;
	}

	/* allocate memory for our private data structure */
	drv = kzalloc(sizeof(*drv), GFP_KERNEL);
	if (!drv) {
		dev_err(&pdev->dev, "unable to allocate mem for private data\n");
		return -ENOMEM;
	}
	
	/* get register memory resource */
	iomem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!iomem) {
		dev_err(&pdev->dev, "no mem resource\n");
		rc = -ENODEV;
		goto err_free_drv;
	}

	drv->reg_base = ioremap(iomem->start, resource_size(iomem));
	if (!drv->reg_base) {
		dev_err(&pdev->dev, "ioremap of register space failed\n");
		rc = -ENOMEM;
		goto err_free_drv;
	}

	for (i = 0; i < NUM_VBUS_IRQS; i++) {
		drv->vbus_irq[i] = platform_get_irq(pdev, i);
		if (drv->vbus_irq[i] <= 0 && i == 0) {
			dev_err(&pdev->dev, "no irq resource\n");
			goto err_iounmap;
		}
	}

	drv->dev = &pdev->dev;
	platform_set_drvdata(pdev, drv);

	sema_init(&drv->cp_lock, 1);
	spin_lock_init(&drv->spinlock);
	gDrv = drv;
	
	KLOG_INFO("Kona OTG CP probe done\n");

	return 0;

err_iounmap:
	iounmap(drv->reg_base);

err_free_drv:
	kfree(drv);
	return rc;
}

static int otg_cp_remove(struct platform_device *pdev)
{
	struct otg_drv *drv = platform_get_drvdata(pdev);

	if (drv->cp)
		kona_otg_cp_unregister(drv->cp);

	platform_set_drvdata(pdev, NULL);
	iounmap(drv->reg_base);
	
	gDrv = NULL;
	kfree(drv);

	return 0;
}

static struct platform_driver otg_cp_driver = {
	.driver = {
		.name = "kona-otg-cp",
		.owner = THIS_MODULE,
	},
	.probe = otg_cp_probe,
	.remove = otg_cp_remove,
};

static int __init otg_cp_module_init(void)
{
	return platform_driver_register(&otg_cp_driver);
}

static void __exit otp_cp_module_exit(void)
{
	platform_driver_unregister(&otg_cp_driver);
}
subsys_initcall(otg_cp_module_init);
module_exit(otp_cp_module_exit);

MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("Kona USB OTG Charge Pump Driver");
MODULE_LICENSE("GPL");
