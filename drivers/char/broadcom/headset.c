/*****************************************************************************
* Copyright 2011 Broadcom Corporation.  All rights reserved.
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
/**
*
*  @file    headset.c
*
*  @brief   Implements simple GPIO-based headset detection interface
*
****************************************************************************/
/* ---- Include Files ---------------------------------------------------- */
#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/spinlock.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/poll.h>
#include <linux/device.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <asm/gpio.h>
#if defined(CONFIG_SWITCH)
#include <linux/switch.h>
#endif

#include <linux/broadcom/headset.h>
#include <linux/broadcom/headset_cfg.h>

#include <linux/broadcom/knllog.h>	/* for debugging */

/* ---- Public Variables ------------------------------------------------- */
/* ---- Private Constants and Types -------------------------------------- */

#define HEADSET_DEFAULT_SCHED_DELAY          30

/* Headset channel info structure */
struct headset_info {
	atomic_t avail;		/* Only allow single user because of select() */
	int changed;		/* Changed flag used by poll */
	int hs_spkr_debounce;	/* Speaker debounce time in microseconds */
	int hs_mic_debounce;	/* Mic debounce time in microseconds */
	headset_state state;	/* Speaker/mic current state */
	wait_queue_head_t waitq;	/* wait queue */
	struct headset_hw_cfg hw_cfg;	/* board hw configuration */
};

/* ---- Private Variables ------------------------------------------------ */

static int gDriverMajor;
#if CONFIG_SYSFS
static struct class *headset_class;
static struct device *headset_dev;
#if defined(CONFIG_SWITCH)
static struct switch_dev headset_switch;
#endif
#endif

/* Headset channel information */
static struct headset_info gHeadset[1] = {
	{
	 .avail = ATOMIC_INIT(1),
	 },
};

static spinlock_t detlock = __SPIN_LOCK_UNLOCKED(detlock);

/* ---- Private Function Prototypes -------------------------------------- */
static int headset_open(struct inode *inode, struct file *file);
static int headset_release(struct inode *inode, struct file *file);
static long headset_ioctl(struct file *file, unsigned int cmd,
			  unsigned long arg);

static unsigned int headset_poll(struct file *file,
				 struct poll_table_struct *poll_table);

#if defined(CONFIG_SWITCH)
static void setsw(struct work_struct *work);
DECLARE_DELAYED_WORK(setsw_work, setsw);
#endif

/* File Operations (these are the device driver entry points) */
struct file_operations headset_fops = {
	.owner = THIS_MODULE,
	.open = headset_open,
	.release = headset_release,
	.unlocked_ioctl = headset_ioctl,
	.poll = headset_poll,
};

/* ---- Functions -------------------------------------------------------- */

/***************************************************************************/
/**
*  Helper routine to check headset detection gpio
*/
static void check_headset_det_gpio(struct headset_info *ch)
{
	int spkr_gpio_val = 0;
	int mic_gpio_val = 0;

	if (ch->hw_cfg.gpio_headset_det >= 0) {
		spkr_gpio_val = gpio_get_value(ch->hw_cfg.gpio_headset_det);
		if (ch->hw_cfg.gpio_headset_active_low) {
			spkr_gpio_val = !spkr_gpio_val;
		}
	} else {
		ch->state = HEADSET_STATE_INIT;
		return;
	}

	if (ch->hw_cfg.gpio_mic_det >= 0) {
		mic_gpio_val = gpio_get_value(ch->hw_cfg.gpio_mic_det);
		if (ch->hw_cfg.gpio_mic_active_low) {
			mic_gpio_val = !mic_gpio_val;
		}
	}

	if (spkr_gpio_val) {
		if (mic_gpio_val) {
			ch->state = HEADSET_TOGGLE_SPKR_MIC;
		} else {
			ch->state = HEADSET_TOGGLE_SPKR_ONLY;
		}
	} else {
		ch->state = HEADSET_UNPLUGGED;
	}
	ch->changed = 1;

	wake_up_interruptible(&ch->waitq);
}

/***************************************************************************/
/**
*  Helper function to schedule switch state changes for Android
*/
#if defined(CONFIG_SWITCH)
static void setsw(struct work_struct *work)
{
	struct headset_info *ch = gHeadset;
	headset_state start_state;
	unsigned long flags;

	spin_lock_irqsave(&detlock, flags);

	start_state = ch->state;

	check_headset_det_gpio(ch);

	if (start_state == HEADSET_TOGGLE_SPKR_ONLY
	    || start_state == HEADSET_TOGGLE_SPKR_MIC) {
		KNLLOG("Start_state = %d\n", start_state);
		if (start_state != ch->state) {
			switch (ch->state) {
			case HEADSET_TOGGLE_SPKR_MIC:
			case HEADSET_TOGGLE_SPKR_ONLY:
				KNLLOG("Setting state to unplugged\n");
				switch_set_state(&headset_switch,
						 HEADSET_UNPLUGGED);
				break;
			default:
				/* Do nothing */
				break;
			}
		}
	}

	switch_set_state(&headset_switch, ch->state);
	KNLLOG("State set to %d\n", ch->state);

	spin_unlock_irqrestore(&detlock, flags);

}
#endif

#if defined(CONFIG_SWITCH)
static ssize_t spkr_debounce_show(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	struct headset_info *ch = gHeadset;

	if (ch->hw_cfg.gpio_headset_det < 0) {
		return -EPERM;
	}

	return snprintf(buf, PAGE_SIZE, "%d\n", ch->hs_spkr_debounce);
}

static ssize_t mic_debounce_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	struct headset_info *ch = gHeadset;

	if (ch->hw_cfg.gpio_mic_det < 0) {
		return -EPERM;
	}

	return snprintf(buf, PAGE_SIZE, "%d\n", ch->hs_mic_debounce);
}

static ssize_t spkr_debounce_store(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t count)
{
	int ret;
	long int debounce;
	struct headset_info *ch = gHeadset;

	if (ch->hw_cfg.gpio_headset_det < 0) {
		return -EPERM;
	}

	ret = kstrtol(buf, 0, &debounce);
	if (ret == 0) {
		if ((ret =
		     gpio_set_debounce(ch->hw_cfg.gpio_headset_det,
				       debounce)) == 0) {
			ch->hs_spkr_debounce = debounce;
		} else {
			return ret;
		}
		return count;
	}
	return -EINVAL;
}

static ssize_t mic_debounce_store(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t count)
{
	int ret;
	long int debounce;
	struct headset_info *ch = gHeadset;

	if (ch->hw_cfg.gpio_mic_det < 0) {
		return -EPERM;
	}

	ret = kstrtol(buf, 0, &debounce);
	if (ret == 0) {
		if ((ret =
		     gpio_set_debounce(ch->hw_cfg.gpio_mic_det,
				       debounce)) == 0) {
			ch->hs_mic_debounce = debounce;
		} else {
			return ret;
		}
		return count;
	}
	return -EINVAL;
}

static DEVICE_ATTR(spkr_debounce, S_IRUGO | S_IWUSR, spkr_debounce_show,
		   spkr_debounce_store);
static DEVICE_ATTR(mic_debounce, S_IRUGO | S_IWUSR, mic_debounce_show,
		   mic_debounce_store);

#endif
/***************************************************************************/
/**
*  Driver open method
*
*  @remarks
*/
static int headset_open(struct inode *inode, struct file *file)
{
	struct headset_info *ch;
	unsigned long flags;

	ch = gHeadset;

	if (ch->hw_cfg.gpio_headset_det < 0) {
		/* Missing configuration */
		return -EPERM;
	}

	/* Allow only 1 user because select() only works for single user */
	if (atomic_dec_and_test(&ch->avail) == 0) {
		atomic_inc(&ch->avail);
		return -EBUSY;
	}

	file->private_data = ch;

	spin_lock_irqsave(&detlock, flags);

	check_headset_det_gpio(ch);
#if defined(CONFIG_SWITCH)
	schedule_delayed_work(&setsw_work, 0);
#endif

	spin_unlock_irqrestore(&detlock, flags);

	return 0;
}

/***************************************************************************/
/**
*  Driver release method
*
*  @remarks
*/
static int headset_release(struct inode *inode, struct file *file)
{
	struct headset_info *ch = file->private_data;

	atomic_inc(&ch->avail);

	return 0;
}

/***************************************************************************/
/**
*  Driver ioctl method
*
*  @remarks
*/
static long headset_ioctl(struct file *file, unsigned int cmd,
			  unsigned long arg)
{
	struct headset_info *ch = file->private_data;
	int rc;
	unsigned debounce;

	switch (cmd) {
	case HEADSET_IOCTL_GET_STATE:
		if (copy_to_user
		    ((unsigned long *)arg, &ch->state,
		     sizeof(ch->state)) != 0) {
			return -EFAULT;
		}
		break;

	case HEADSET_IOCTL_GET_DEBOUNCE:
		if (copy_to_user
		    ((unsigned long *)arg, &ch->hs_spkr_debounce,
		     sizeof(ch->hs_spkr_debounce)) != 0) {
			return -EFAULT;
		}
		break;

	case HEADSET_IOCTL_GET_MIC_DEBOUNCE:
		if (copy_to_user
		    ((unsigned long *)arg, &ch->hs_mic_debounce,
		     sizeof(ch->hs_mic_debounce)) != 0) {
			return -EFAULT;
		}
		break;

	case HEADSET_IOCTL_SET_DEBOUNCE:
		if (ch->hw_cfg.gpio_headset_det < 0) {
			return -EINVAL;
		}

		if (copy_to_user
		    ((unsigned long *)arg, &debounce, sizeof(debounce)) != 0) {
			return -EFAULT;
		}

		if ((rc =
		     gpio_set_debounce(ch->hw_cfg.gpio_headset_det,
				       debounce))) {
			return rc;
		}

		ch->hs_spkr_debounce = debounce;
		break;

	case HEADSET_IOCTL_SET_MIC_DEBOUNCE:
		if (ch->hw_cfg.gpio_mic_det < 0) {
			return -EINVAL;
		}

		if (copy_to_user
		    ((unsigned long *)arg, &debounce, sizeof(debounce)) != 0) {
			return -EFAULT;
		}

		if ((rc = gpio_set_debounce(ch->hw_cfg.gpio_mic_det, debounce))) {
			return rc;
		}

		ch->hs_mic_debounce = debounce;
		break;

	default:
		return -ENOTTY;
	}
	return 0;
}

/***************************************************************************/
/**
*  Driver poll method to support system select call
*
*  @remarks
*/
static unsigned int headset_poll(struct file *file,
				 struct poll_table_struct *poll_table)
{
	struct headset_info *ch = file->private_data;

	poll_wait(file, &ch->waitq, poll_table);

	if (ch->changed) {
		ch->changed = 0;
		return POLLIN | POLLRDNORM;
	}

	return 0;
}

/***************************************************************************/
/**
*  Headset detection ISR
*/
static irqreturn_t headset_det_irq(int irq, void *dev_id)
{
	unsigned long flags;

	spin_lock_irqsave(&detlock, flags);

#if defined(CONFIG_SWITCH)
	schedule_delayed_work(&setsw_work, HEADSET_DEFAULT_SCHED_DELAY);
#endif

	spin_unlock_irqrestore(&detlock, flags);

	return IRQ_HANDLED;
}

/***************************************************************************/
/**
*  Platform probe method
*/
static int headset_pltfm_probe(struct platform_device *pdev)
{
	struct headset_info *ch;
	int ret;
	unsigned long flags;

	BUG_ON(pdev == NULL);

	if (pdev->dev.platform_data == NULL) {
		dev_err(&pdev->dev, "platform_data missing\n");
		return -EFAULT;
	}

	ch = gHeadset;
	memcpy(&ch->hw_cfg, (void *)pdev->dev.platform_data,
	       sizeof(ch->hw_cfg));

	if (ch->hw_cfg.gpio_headset_det >= 0) {
		ret =
		    gpio_request(ch->hw_cfg.gpio_headset_det,
				 "headset spkr det");
		if (ret < 0) {
			dev_err(&pdev->dev, "Unable to request GPIO pin %d\n",
				ch->hw_cfg.gpio_headset_det);
			return ret;
		}

		ret = request_irq(gpio_to_irq(ch->hw_cfg.gpio_headset_det),
				  headset_det_irq,
				  IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,
				  "headset_spkr_det", ch);
		if (ret) {
			dev_err(&pdev->dev,
				"Unable to request headset detect irq=%d for gpio=%d\n",
				gpio_to_irq(ch->hw_cfg.gpio_headset_det),
				ch->hw_cfg.gpio_headset_det);
			goto err_free_spkr_det_gpio;
		}
	} else {
		dev_err(&pdev->dev,
			"Missing headset speaker gpio configuration\n");
		return -EPERM;
	}

	if (ch->hw_cfg.gpio_mic_det >= 0) {
		ret = gpio_request(ch->hw_cfg.gpio_mic_det, "headset mic det");
		if (ret < 0) {
			dev_err(&pdev->dev, "Unable to request GPIO pin %d\n",
				ch->hw_cfg.gpio_mic_det);
			goto err_irq_spkr_free;
		}

		ret = request_irq(gpio_to_irq(ch->hw_cfg.gpio_mic_det),
				  headset_det_irq,
				  IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,
				  "headset_mic_det", ch);
		if (ret) {
			dev_err(&pdev->dev,
				"Unable to request headset detect irq=%d for gpio=%d\n",
				gpio_to_irq(ch->hw_cfg.gpio_mic_det),
				ch->hw_cfg.gpio_mic_det);
			goto err_free_mic_det_gpio;
		}
	}
#if defined(CONFIG_SWITCH)
	/* Create a headset switch */
	headset_switch.name = "h2w";

	ret = switch_dev_register(&headset_switch);
	if (ret < 0) {
		printk(KERN_ERR "HEADSET: Device switch create failed\n");
		ret = -EFAULT;
		goto err_irq_free;
	}

	/* Add the debounce sysfs entry for h2w */
	ret = device_create_file(headset_switch.dev, &dev_attr_spkr_debounce);
	if (ret < 0)
		goto err_swdev_destroy;

	ret = device_create_file(headset_switch.dev, &dev_attr_mic_debounce);
	if (ret < 0)
		goto err_swdev_destroy;
#endif

	/* set the default debounce */
	ch->hs_spkr_debounce = HEADSET_DEBOUNCE_DEFAULT;
	ch->hs_mic_debounce = HEADSET_DEBOUNCE_DEFAULT;

	if (ch->hw_cfg.gpio_headset_det >= 0) {
		ret =
		    gpio_set_debounce(ch->hw_cfg.gpio_headset_det,
				      ch->hs_spkr_debounce);
		if (ret < 0) {
			ret = -EPERM;
			printk(KERN_ERR
			       "HEADSET: Hardware GPIO debounce not supported and software debounce not implemented\n");
			goto probe_no_gpio;
		}
	}

	if (ch->hw_cfg.gpio_mic_det >= 0) {
		ret =
		    gpio_set_debounce(ch->hw_cfg.gpio_mic_det,
				      ch->hs_mic_debounce);
		if (ret < 0) {
			ret = -EPERM;
			printk(KERN_ERR
			       "HEADSET: Hardware GPIO debounce not supported and software debounce not implemented\n");
			goto probe_no_gpio;
		}
	}

	spin_lock_irqsave(&detlock, flags);

	check_headset_det_gpio(ch);
#if defined(CONFIG_SWITCH)
	schedule_delayed_work(&setsw_work, 0);
#endif

	spin_unlock_irqrestore(&detlock, flags);

	return 0;

      probe_no_gpio:
#if defined(CONFIG_SWITCH)
	device_remove_file(headset_switch.dev, &dev_attr_spkr_debounce);
	device_remove_file(headset_switch.dev, &dev_attr_mic_debounce);
      err_swdev_destroy:
	switch_dev_unregister(&headset_switch);
      err_irq_free:
#endif
	if (ch->hw_cfg.gpio_mic_det >= 0) {
		free_irq(gpio_to_irq(ch->hw_cfg.gpio_mic_det), &pdev->dev);
	}
      err_free_mic_det_gpio:
	if (ch->hw_cfg.gpio_mic_det >= 0) {
		gpio_free(ch->hw_cfg.gpio_mic_det);
	}
      err_irq_spkr_free:
	if (ch->hw_cfg.gpio_headset_det >= 0) {
		free_irq(gpio_to_irq(ch->hw_cfg.gpio_headset_det), &pdev->dev);
	}
      err_free_spkr_det_gpio:
	if (ch->hw_cfg.gpio_headset_det >= 0) {
		gpio_free(ch->hw_cfg.gpio_headset_det);
	}
	return ret;
}

/***************************************************************************/
/**
*  Platform remove method
*/
static int headset_pltfm_remove(struct platform_device *pdev)
{
	struct headset_info *ch;
	ch = gHeadset;

#if defined(CONFIG_SWITCH)
	device_remove_file(headset_switch.dev, &dev_attr_spkr_debounce);
	device_remove_file(headset_switch.dev, &dev_attr_mic_debounce);
	switch_dev_unregister(&headset_switch);
#endif
	if (ch->hw_cfg.gpio_headset_det >= 0) {
		free_irq(gpio_to_irq(ch->hw_cfg.gpio_headset_det), &pdev->dev);
		gpio_free(ch->hw_cfg.gpio_headset_det);
	}

	if (ch->hw_cfg.gpio_mic_det >= 0) {
		free_irq(gpio_to_irq(ch->hw_cfg.gpio_mic_det), &pdev->dev);
		gpio_free(ch->hw_cfg.gpio_mic_det);
	}

	platform_set_drvdata(pdev, NULL);

	return 0;
}

static struct platform_driver gPlatform_driver = {
	.driver = {
		   .name = "bcmisland-headset-det",
		   .owner = THIS_MODULE,
		   },
	.probe = headset_pltfm_probe,
	.remove = headset_pltfm_remove,
};

/***************************************************************************/
/**
*  Initialize APM headset detection
*
*  @remarks
*/
static int __init headset_init(void)
{
	int rc;
	struct headset_info *ch;

	init_waitqueue_head(&gHeadset[0].waitq);

	rc = platform_driver_register(&gPlatform_driver);
	if (rc < 0) {
		printk("%s: failed to register platform driver\n",
		       __FUNCTION__);
		return rc;
	}

	gDriverMajor = register_chrdev(0, "headset", &headset_fops);
	if (gDriverMajor < 0) {
		printk("HEADSET: Failed to register device major\n");
		rc = -EFAULT;
		goto err_unregister_platform;
	}
#if CONFIG_SYSFS
	headset_class = class_create(THIS_MODULE, "bcmisland-headset-det");
	if (IS_ERR(headset_class)) {
		printk("HEADSET: Class create failed\n");
		rc = -EFAULT;
		goto err_unregister_chrdev;
	}

	headset_dev = device_create(headset_class, NULL, MKDEV(gDriverMajor, 0),
				    NULL, "headset0");
	if (IS_ERR(headset_dev)) {
		printk(KERN_ERR "HEADSET: Device create failed\n");
		rc = -EFAULT;
		goto err_class_destroy;
	}
#endif
	ch = gHeadset;
	if (ch->hw_cfg.gpio_headset_det < 0) {
		/* Missing configuration */
		rc = -EPERM;
		printk(KERN_ERR "HEADSET: Detection GPIO not avilable\n");
		goto init_no_gpio;

	}

	return 0;

      init_no_gpio:
#if CONFIG_SYSFS
	device_destroy(headset_class, MKDEV(gDriverMajor, 0));

      err_class_destroy:
	class_destroy(headset_class);

      err_unregister_chrdev:
	unregister_chrdev(gDriverMajor, "headset");
#endif

      err_unregister_platform:
	platform_driver_unregister(&gPlatform_driver);

	return rc;
}

/***************************************************************************/
/**
*  Destructor for the headset detection driver
*
*  @remarks
*/
static void __exit headset_exit(void)
{
#if CONFIG_SYSFS
	device_destroy(headset_class, MKDEV(gDriverMajor, 0));
	class_destroy(headset_class);
#endif

	unregister_chrdev(gDriverMajor, "headset");

	platform_driver_unregister(&gPlatform_driver);
}

module_init(headset_init);
module_exit(headset_exit);
