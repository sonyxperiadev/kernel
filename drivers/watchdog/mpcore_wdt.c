/*
 *	Watchdog driver for the mpcore watchdog timer
 *
 *	(c) Copyright 2004 ARM Limited
 *
 *	Based on the SoftDog driver:
 *	(c) Copyright 1996 Alan Cox <alan@lxorguk.ukuu.org.uk>,
 *						All Rights Reserved.
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 *
 *	Neither Alan Cox nor CymruNet Ltd. admit liability nor provide
 *	warranty for any of this software. This material is provided
 *	"AS-IS" and at no charge.
 *
 *	(c) Copyright 1995    Alan Cox <alan@lxorguk.ukuu.org.uk>
 *
 */


/*
 * Frameworks:
 *
 *    - SMP:          Fully supported.    Locking is in place where necessary.
 *    - GPIO:         Fully supported.    No GPIOs are used.
 *    - MMU:          Fully supported.    Platform model with ioremap used.
 *    - Dynamic /dev: Fully supported.    Registers itself as /dev/watchdog with dynamic device numbers.
 *    - Suspend:      Fully supported.    Suspend/resume disable/enable watchdog and clock.
 *    - Clocks:       Fully supported.    Uses new 'arm_periph' clock, fixed at core clock / 2.
 *    - Power:        Not done.
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/types.h>
#include <linux/miscdevice.h>
#include <linux/watchdog.h>
#include <linux/fs.h>
#include <linux/reboot.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/workqueue.h>
#include <linux/cpumask.h>
#include <linux/clk.h>

#include <asm/smp_twd.h>

struct mpcore_wdt {
        unsigned long  timer_alive;
        struct device *dev;
        void __iomem  *base;
        int	       irq;
        char           expect_close;
        struct cdev    cdev;
        dev_t          number;
        struct class  *class;
        struct clk    *clock;
};

static struct platform_device *mpcore_wdt_dev;

static struct workqueue_struct *wdt_wq;

typedef struct {
        struct work_struct work;
        struct mpcore_wdt *wdt;
} wdt_work_t;

/* There is a watchdog in each core with different state. */
DEFINE_PER_CPU(unsigned int,      perturb)      = { 0 };
DEFINE_PER_CPU(wdt_work_t,        work);

#define TIMER_MARGIN	60
static int mpcore_margin = TIMER_MARGIN;
module_param(mpcore_margin, int, 0);
MODULE_PARM_DESC(mpcore_margin,
	"MPcore timer margin in seconds. (0 < mpcore_margin < 65536, default="
				__MODULE_STRING(TIMER_MARGIN) ")");

static int nowayout = WATCHDOG_NOWAYOUT;
module_param(nowayout, int, 0);
MODULE_PARM_DESC(nowayout,
	"Watchdog cannot be stopped once started (default="
				__MODULE_STRING(WATCHDOG_NOWAYOUT) ")");

#define ONLY_TESTING	0
static int mpcore_noboot = ONLY_TESTING;
module_param(mpcore_noboot, int, 0);
MODULE_PARM_DESC(mpcore_noboot, "MPcore watchdog action, "
	"set to 1 to ignore reboots, 0 to reboot (default="
					__MODULE_STRING(ONLY_TESTING) ")");

/*
 *	This is the interrupt handler.  Note that we only use this
 *	in testing mode, so don't actually do a reboot here.
 */
static irqreturn_t mpcore_wdt_fire(int irq, void *arg)
{
	struct mpcore_wdt *wdt = arg;

	/* Check it really was our interrupt */
	if (readl(wdt->base + TWD_WDOG_INTSTAT)) {
		dev_printk(KERN_CRIT, wdt->dev,
					"Triggered on CPU %d - Reboot ignored.\n", smp_processor_id());
		/* Clear the interrupt on the watchdog */
		writel(1, wdt->base + TWD_WDOG_INTSTAT);
		return IRQ_HANDLED;
	}
	return IRQ_NONE;
}

/*
 *	mpcore_wdt_keepalive - reload the timer
 *
 *	Note that the spec says a DIFFERENT value must be written to the reload
 *	register each time.  The "perturb" variable deals with this by adding 1
 *	to the count every other time the function is called.
 */
static void mpcore_wdt_keepalive_worker(struct work_struct *work)
{
        unsigned long count;
        wdt_work_t *wdt_work = (wdt_work_t *)work;
        struct mpcore_wdt *wdt = wdt_work->wdt;

        /* Assume prescale is set to 256 */
        count = (clk_get_rate(wdt->clock) / 256) * mpcore_margin;

        /* Reload the counter */
        writel(count + per_cpu(perturb, smp_processor_id()), wdt->base + TWD_WDOG_LOAD);
        per_cpu(perturb, smp_processor_id()) = per_cpu(perturb, smp_processor_id()) ? 0 : 1;
}

static void mpcore_wdt_stop_worker(struct work_struct *work)
{
        wdt_work_t *wdt_work = (wdt_work_t *)work;
        struct mpcore_wdt *wdt = wdt_work->wdt;

        dev_printk(KERN_INFO, wdt->dev, "disabling watchdog on CPU %d.\n",smp_processor_id());

        writel(0x12345678, wdt->base + TWD_WDOG_DISABLE);
        writel(0x87654321, wdt->base + TWD_WDOG_DISABLE);
        writel(0x0, wdt->base + TWD_WDOG_CONTROL);
}

static void mpcore_wdt_start_worker(struct work_struct *work)
{
        wdt_work_t *wdt_work = (wdt_work_t *)work;
        struct mpcore_wdt *wdt = wdt_work->wdt;
        u32 mode;

        dev_printk(KERN_INFO, wdt->dev, "enabling watchdog on CPU %d.\n",smp_processor_id());

        /* This loads the count register but does NOT start the count yet */
        mpcore_wdt_keepalive_worker(work);

        /* Setup watchdog - prescale=256, enable=1 */
        mode = (255 << 8) | TWD_WDOG_CONTROL_ENABLE;

        if (mpcore_noboot) {
                /* timer mode, send interrupt */
                mode |=	TWD_WDOG_CONTROL_TIMER_MODE
                     |  TWD_WDOG_CONTROL_IT_ENABLE;
        } else {
            /* watchdog mode */
                mode |=	TWD_WDOG_CONTROL_WATCHDOG_MODE;
        }
        writel(mode, wdt->base + TWD_WDOG_CONTROL);
}

static void mpcore_wdt_dispatcher(void (*func)(struct work_struct *work), struct mpcore_wdt *wdt)
{
        int i;

        /* Make sure the work structs aren't still being used. */
        flush_workqueue(wdt_wq);

        /* Loop over each active CPU queuing watchdog commands on each one */
        for_each_online_cpu(i) {
                INIT_WORK( (struct work_struct *) &per_cpu(work, i).work, func);
                per_cpu(work, i).wdt = wdt;
                queue_work_on(i, wdt_wq, &per_cpu(work, i).work);
        }
}

static void mpcore_wdt_start(struct mpcore_wdt *wdt)
{
	mpcore_wdt_dispatcher(mpcore_wdt_start_worker, wdt);
}

static void mpcore_wdt_stop(struct mpcore_wdt *wdt)
{
	mpcore_wdt_dispatcher(mpcore_wdt_stop_worker, wdt);
}

static void mpcore_wdt_keepalive(struct mpcore_wdt *wdt)
{
	/* Do nothing if watchdog isn't on. IOCTL might have turned it off */
	/* and we shouldn't just reenable it becuase we got pinged.        */
	if (test_bit(1, &wdt->timer_alive)) {
		mpcore_wdt_dispatcher(mpcore_wdt_keepalive_worker, wdt);
	}
}

static int mpcore_wdt_set_heartbeat(int t)
{
	if (t < 0x0001 || t > 0xFFFF)
		return -EINVAL;

	mpcore_margin = t;
	return 0;
}

/*
 *	/dev/watchdog handling
 */
static int mpcore_wdt_open(struct inode *inode, struct file *file)
{
	struct mpcore_wdt *wdt = platform_get_drvdata(mpcore_wdt_dev);

	if (test_and_set_bit(0, &wdt->timer_alive))
		return -EBUSY;

        /* Track watchdog on separately from dev open. */
        set_bit(1, &wdt->timer_alive);

        if (nowayout)
		__module_get(THIS_MODULE);

	file->private_data = wdt;

	/*
	 *	Activate timer
	 */
	mpcore_wdt_start(wdt);

	return nonseekable_open(inode, file);
}

static int mpcore_wdt_release(struct inode *inode, struct file *file)
{
	struct mpcore_wdt *wdt = file->private_data;

	/*
	 *	Shut off the timer.
	 *	Lock it in if it's a module and we set nowayout
	 */
	if (wdt->expect_close == 42) {
		mpcore_wdt_stop(wdt);
		clear_bit(1, &wdt->timer_alive);
	}
        else {
		dev_printk(KERN_CRIT, wdt->dev,
				"unexpected close, not stopping watchdog!\n");
		mpcore_wdt_keepalive(wdt);
	}
	clear_bit(0, &wdt->timer_alive);
	wdt->expect_close = 0;
	return 0;
}

static ssize_t mpcore_wdt_write(struct file *file, const char *data,
						size_t len, loff_t *ppos)
{
	struct mpcore_wdt *wdt = file->private_data;

	/*
	 *	Refresh the timer.
	 */
	if (len) {
		if (!nowayout) {
			size_t i;

			/* In case it was set long ago */
			wdt->expect_close = 0;

			for (i = 0; i != len; i++) {
				char c;

				if (get_user(c, data + i))
					return -EFAULT;
				if (c == 'V')
					wdt->expect_close = 42;
			}
		}
		mpcore_wdt_keepalive(wdt);
	}
	return len;
}

static const struct watchdog_info ident = {
	.options		= WDIOF_SETTIMEOUT |
				  WDIOF_KEEPALIVEPING |
				  WDIOF_MAGICCLOSE,
	.identity		= "MPcore Watchdog",
};

static long mpcore_wdt_ioctl(struct file *file, unsigned int cmd,
							unsigned long arg)
{
	struct mpcore_wdt *wdt = file->private_data;
	int ret;
	union {
		struct watchdog_info ident;
		int i;
	} uarg;

	if (_IOC_DIR(cmd) && _IOC_SIZE(cmd) > sizeof(uarg))
		return -ENOTTY;

	if (_IOC_DIR(cmd) & _IOC_WRITE) {
		ret = copy_from_user(&uarg, (void __user *)arg, _IOC_SIZE(cmd));
		if (ret)
			return -EFAULT;
	}

	switch (cmd) {
	case WDIOC_GETSUPPORT:
		uarg.ident = ident;
		ret = 0;
		break;

	case WDIOC_GETSTATUS:
	case WDIOC_GETBOOTSTATUS:
		uarg.i = 0;
		ret = 0;
		break;

	case WDIOC_SETOPTIONS:
		ret = -EINVAL;
		if (uarg.i & WDIOS_DISABLECARD) {
			clear_bit(1, &wdt->timer_alive);
			mpcore_wdt_stop(wdt);
			ret = 0;
		}
		if (uarg.i & WDIOS_ENABLECARD) {
			set_bit(1, &wdt->timer_alive);
			mpcore_wdt_start(wdt);
			ret = 0;
		}
		break;

	case WDIOC_KEEPALIVE:
		mpcore_wdt_keepalive(wdt);
		ret = 0;
		break;

	case WDIOC_SETTIMEOUT:
		ret = mpcore_wdt_set_heartbeat(uarg.i);
		if (ret)
			break;

		mpcore_wdt_keepalive(wdt);
		/* Fall */
	case WDIOC_GETTIMEOUT:
		uarg.i = mpcore_margin;
		ret = 0;
		break;

	default:
		return -ENOTTY;
	}

	if (ret == 0 && _IOC_DIR(cmd) & _IOC_READ) {
		ret = copy_to_user((void __user *)arg, &uarg, _IOC_SIZE(cmd));
		if (ret)
			ret = -EFAULT;
	}
	return ret;
}

/*
 *	System shutdown handler.  Turn off the watchdog if we're
 *	restarting or halting the system.
 */
static void mpcore_wdt_shutdown(struct platform_device *dev)
{
	struct mpcore_wdt *wdt = platform_get_drvdata(dev);

	if (system_state == SYSTEM_RESTART || system_state == SYSTEM_HALT)
		mpcore_wdt_stop(wdt);
}

/*
 *	Kernel Interfaces
 */
static const struct file_operations mpcore_wdt_fops = {
	.owner		= THIS_MODULE,
	.llseek		= no_llseek,
	.write		= mpcore_wdt_write,
	.unlocked_ioctl	= mpcore_wdt_ioctl,
	.open		= mpcore_wdt_open,
	.release	= mpcore_wdt_release,
};

static int __devinit mpcore_wdt_probe(struct platform_device *dev)
{
	struct mpcore_wdt *wdt;
	struct resource *res;
	int ret;

	/* We only accept one device, and it must have an id of -1 */
	if (dev->id != -1)
		return -ENODEV;

	res = platform_get_resource(dev, IORESOURCE_MEM, 0);
	if (!res) {
		ret = -ENODEV;
		goto err_out;
	}

	wdt = kzalloc(sizeof(struct mpcore_wdt), GFP_KERNEL);
	if (!wdt) {
		ret = -ENOMEM;
		goto err_out;
	}

	wdt->dev = &dev->dev;

        wdt->clock = clk_get(wdt->dev, wdt->dev->platform_data);
        if (wdt->clock < 0)
        {
		ret = -ENXIO;
		goto err_free;
        }

        wdt->irq = platform_get_irq(dev, 0);
	if (wdt->irq < 0) {
		ret = -ENXIO;
		goto err_clock;
	}
	wdt->base = ioremap(res->start, resource_size(res));
	if (!wdt->base) {
		ret = -ENOMEM;
		goto err_clock;
	}

 	ret = alloc_chrdev_region(&wdt->number, 0, 1, "mpcore_wdt");
	if (ret < 0) {
		dev_printk(KERN_ERR, wdt->dev,
			"cannot register with dynamic device number (err=%d)\n", -ret);
		goto err_misc;
	}
	dev_printk(KERN_INFO, wdt->dev, "using device number %d, %d", MAJOR(wdt->number), MINOR(wdt->number));

        cdev_init(&wdt->cdev, &mpcore_wdt_fops);

	ret = request_irq(wdt->irq, mpcore_wdt_fire, IRQF_DISABLED,
							"mpcore_wdt", wdt);
	if (ret) {
		dev_printk(KERN_ERR, wdt->dev,
			"cannot register IRQ%d for watchdog\n", wdt->irq);
		goto err_irq;
	}

	mpcore_wdt_stop(wdt);
	platform_set_drvdata(dev, wdt);
	mpcore_wdt_dev = dev;

        ret = cdev_add(&wdt->cdev, wdt->number, 1);
	if (ret < 0) {
		dev_printk(KERN_ERR, wdt->dev, "To add device\n");
		goto err_cdev_add;
	}

        /* create /dev/watchdog
         * we use udev to make the file
         */
        wdt->class = class_create(THIS_MODULE,"watchdog");
        (void) device_create(wdt->class, wdt->dev, wdt->number,NULL,"watchdog");

        clk_enable(wdt->clock);

        return 0;

err_cdev_add:
	free_irq(wdt->irq, wdt);
err_irq:
        unregister_chrdev_region (wdt->number, 1);
err_misc:
	iounmap(wdt->base);
err_clock:
        clk_put(wdt->clock);
err_free:
	kfree(wdt);
err_out:
	return ret;
}

static int __devexit mpcore_wdt_remove(struct platform_device *dev)
{
	struct mpcore_wdt *wdt = platform_get_drvdata(dev);

        clk_disable(wdt->clock);

        device_destroy(wdt->class,wdt->number);
        class_unregister(wdt->class);
        class_destroy(wdt->class);

        cdev_del(&wdt->cdev);

	platform_set_drvdata(dev, NULL);

        unregister_chrdev_region (wdt->number, 1);

 	mpcore_wdt_dev = NULL;

        clk_put(wdt->clock);
	free_irq(wdt->irq, wdt);
	iounmap(wdt->base);
	kfree(wdt);
	return 0;
}

#ifdef CONFIG_PM
static int mpcore_wdt_suspend(struct platform_device *dev, pm_message_t msg)
{
        struct mpcore_wdt *wdt = platform_get_drvdata(dev);
        mpcore_wdt_stop(wdt);		/* Turn the WDT off */
        clk_disable(wdt->clock);
        return 0;
}

static int mpcore_wdt_resume(struct platform_device *dev)
{
        struct mpcore_wdt *wdt = platform_get_drvdata(dev);
        clk_enable(wdt->clock);
        /* re-activate timer */
        if (test_bit(1, &wdt->timer_alive)) {
                mpcore_wdt_start(wdt);
        }
        return 0;
}
#else
#define mpcore_wdt_suspend    NULL
#define mpcore_wdt_resume     NULL
#endif

/* work with hotplug and coldplug */
MODULE_ALIAS("platform:mpcore_wdt");

static struct platform_driver mpcore_wdt_driver = {
	.remove		= __devexit_p(mpcore_wdt_remove),
        .suspend        = mpcore_wdt_suspend,
        .resume         = mpcore_wdt_resume,
	.shutdown	= mpcore_wdt_shutdown,
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= "mpcore_wdt",
	},
};

static char banner[] __initdata = KERN_INFO "MPcore Watchdog Timer: 0.1. "
		"mpcore_noboot=%d mpcore_margin=%d sec (nowayout= %d)\n";

static int __init mpcore_wdt_init(void)
{
        wdt_wq = create_workqueue("watchdog_queue");

        printk(banner, mpcore_noboot, mpcore_margin, nowayout);

        /*
         * Check that the margin value is within it's range;
         * if not reset to the default
         */
        if (mpcore_wdt_set_heartbeat(mpcore_margin)) {
            mpcore_wdt_set_heartbeat(TIMER_MARGIN);
                printk(KERN_INFO "mpcore_margin value must be 0 < mpcore_margin < 65536, using %d\n",
                       TIMER_MARGIN);
        }

        return platform_driver_probe(&mpcore_wdt_driver, mpcore_wdt_probe);
}

static void __exit mpcore_wdt_exit(void)
{
        flush_workqueue(wdt_wq);
        destroy_workqueue(wdt_wq);
        platform_driver_unregister(&mpcore_wdt_driver);
}

module_init(mpcore_wdt_init);
module_exit(mpcore_wdt_exit);

MODULE_AUTHOR("ARM Limited");
MODULE_DESCRIPTION("MPcore Watchdog Device Driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS_MISCDEV(WATCHDOG_MINOR);
