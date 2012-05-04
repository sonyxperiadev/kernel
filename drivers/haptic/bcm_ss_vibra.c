/*****************************************************************************
* Copyright 2012 Broadcom Corporation.  All rights reserved.
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

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>
#if defined(CONFIG_HAS_WAKELOCK)
#include <linux/wakelock.h>
#endif /*CONFIG_HAS_WAKELOCK*/

#include "../staging/android/timed_output.h"
#include "../staging/android/timed_gpio.h"

#define VIB_ON 1
#define VIB_OFF 0
#define MIN_TIME_MS 100

#if defined(CONFIG_HAS_WAKELOCK)
static struct wake_lock vib_wl;
#endif /*CONFIG_HAS_WAKELOCK*/

static struct timed_output_dev vibrator_timed_dev;
static struct timer_list	vibrate_timer;
static struct work_struct	vibrator_off_work;
static struct regulator* vib_regulator = NULL;
static int vib_voltage;
static int Is_vib_shortly;

static void vibrator_ctrl_regulator(int on_off)
{
	printk(KERN_NOTICE "Vibrator: %s\n",(on_off?"ON":"OFF"));
	
	if(on_off==VIB_ON)
	{
		if(!regulator_is_enabled(vib_regulator))
		{
			regulator_set_voltage(vib_regulator,vib_voltage,vib_voltage);
			regulator_enable(vib_regulator);
			printk(KERN_NOTICE "Vibrator: enable\n");
		}
	}
	else
	{
		if(regulator_is_enabled(vib_regulator))
		{
			regulator_disable(vib_regulator);
			printk(KERN_NOTICE "Vibrator: disable\n");
		}
	}
}
static void vibrator_off_worker(struct work_struct *work)
{
	vibrator_ctrl_regulator(VIB_OFF);

#if defined(CONFIG_HAS_WAKELOCK)
	wake_unlock(&vib_wl);
#endif /*CONFIG_HAS_WAKELOCK*/
}

static void on_vibrate_timer_expired(unsigned long x)
{
   printk(KERN_NOTICE "Vibrator: expired %ldms\n", x);
	Is_vib_shortly = false;
	schedule_work(&vibrator_off_work);
}

static void vibrator_enable_set_timeout(struct timed_output_dev *sdev,
	int timeout)
{
	int ret_mod_timer = 0;
	printk(KERN_NOTICE "Vibrator: Set duration: %dms\n", timeout);

	if( timeout == 0 )
	{
		if(Is_vib_shortly == false)
      {
			vibrator_ctrl_regulator(VIB_OFF);
			del_timer(&vibrate_timer);
		}
      
		return;
	}

#if defined(CONFIG_HAS_WAKELOCK)
	wake_lock(&vib_wl);
#endif /*CONFIG_HAS_WAKELOCK*/

	vibrator_ctrl_regulator(VIB_ON);
	if(timeout < MIN_TIME_MS)
   {
		Is_vib_shortly = true;
		timeout *= 2;
	}
   
	if(timeout == 5000 || timeout == 10000)
   {
		printk(KERN_NOTICE "Vibrator: timeout= %dms, skip off\n", timeout);
   }
	else
   {
		if(!timer_pending(&vibrate_timer))
      {
			ret_mod_timer = mod_timer(&vibrate_timer, jiffies + msecs_to_jiffies(timeout));
         if( ret_mod_timer )
         {
				printk(KERN_NOTICE "Vibrator: ret_mod_timer= %d\n", ret_mod_timer);
            vibrator_ctrl_regulator(VIB_OFF);
         }
      }
		else
		{
#if defined(CONFIG_HAS_WAKELOCK)
			wake_unlock(&vib_wl);
#endif /*CONFIG_HAS_WAKELOCK*/
		}
	}		
   
	return;
}

static int vibrator_get_remaining_time(struct timed_output_dev *sdev)
{
	int retTime = jiffies_to_msecs(jiffies - vibrate_timer.expires);
	printk(KERN_NOTICE "Vibrator: Current duration: %dms\n", retTime);
	return retTime;
}

static int vibrator_probe(struct platform_device *pdev)
{
	int ret = 0;

  	/* use LDO_HV4 regulator off PMU chip */
	vib_regulator = regulator_get(NULL, (const char *)(pdev->dev.platform_data));
   
	/* Setup timed_output obj */
	vibrator_timed_dev.name = "vibrator";
	vibrator_timed_dev.enable = vibrator_enable_set_timeout;
	vibrator_timed_dev.get_time = vibrator_get_remaining_time;
	vib_voltage = 3300000;
   
#if defined(CONFIG_HAS_WAKELOCK)
	wake_lock_init(&vib_wl, WAKE_LOCK_SUSPEND, __stringify(vib_wl));
#endif

	/* Vibrator dev register in /sys/class/timed_output/ */
	ret = timed_output_dev_register(&vibrator_timed_dev);
	if (ret < 0) {
		printk(KERN_ERR "Vibrator: timed_output dev registration failure\n");
		goto error;
	}

	init_timer(&vibrate_timer);
	vibrate_timer.function = on_vibrate_timer_expired;
	vibrate_timer.data = (unsigned long)NULL;
	INIT_WORK(&vibrator_off_work, vibrator_off_worker);
	Is_vib_shortly = false;

	return 0;
   
error:
   wake_lock_destroy(&vib_wl);
   return ret;
}

static int __devexit vibrator_remove(struct platform_device *pdev)
{
	timed_output_dev_unregister(&vibrator_timed_dev);
   if (vib_regulator) 
	{
         regulator_put(vib_regulator);
         vib_regulator = NULL;
   }
   
	return 0;
}

static int vibrator_suspend(struct platform_device *pdev, pm_message_t state)
{
	return 0;
}

static int vibrator_resume(struct platform_device *pdev)
{
	return 0;
}

static struct platform_driver vibrator_driver = {
	.probe		= vibrator_probe,
	.remove		= __devexit_p(vibrator_remove),
	.suspend		= vibrator_suspend,
	.resume		=  vibrator_resume,
	.driver		= {
		.name	= "vibrator",
		.owner	= THIS_MODULE,
	},
};

static int __init vibrator_init(void)
{
	return platform_driver_register(&vibrator_driver);
}

static void __exit vibrator_exit(void)
{
	platform_driver_unregister(&vibrator_driver);
}

module_init(vibrator_init);
module_exit(vibrator_exit);

MODULE_DESCRIPTION("Android Vibrator driver");
MODULE_LICENSE("GPL");
