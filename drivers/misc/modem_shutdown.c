#include <linux/version.h>
#include <linux/sched.h>
#include <linux/signal.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/timer.h>
#include <linux/io.h>
#include <asm/irq.h>
#include <linux/delay.h>
#include <linux/wakelock.h>
#include <linux/kmod.h>
#include <linux/workqueue.h>
static struct timer_list my_timer;
struct wake_lock modem_power_wake_lock;
static struct delayed_work work_modem_init;

static void kmod_work_handler(struct work_struct *work);

MODULE_LICENSE("GPL");


static void
kmod_work_handler(struct work_struct *work)
{
	char *path = "/system/bin/capi2test";
	char *argv[] = {"/system/bin/capi2test", "2", NULL };
	char *envp[] = {"HOME=/", "PATH=/sbin:/usr/sbin:/bin:/usr/bin", NULL };
	int ret = -1;

	wake_lock(&modem_power_wake_lock);
	printk(KERN_ERR"%s-> CAPI2TEST SCRIPT LOAD\n", __func__);
	ret = call_usermodehelper(path, argv, envp, 0);

	if (ret != 0)
		printk(KERN_ERR"%s-> CAPI2Test Failed to set AirPlane Mode RETURN(%i)\n", __func__, ret);
	else
		printk(KERN_ERR"%s-> CAPI2Test Success AirPlane Mode RETURN(%i)\n", __func__, ret);

	udelay(1000);
	wake_unlock(&modem_power_wake_lock);
	wake_lock_destroy(&modem_power_wake_lock);

	del_timer(&my_timer);
}



void my_timer_callback(unsigned long data)
{
	printk(KERN_ERR"%s-> QUEUE Modem Shutdown Work\n", __func__);
	schedule_delayed_work(&work_modem_init,
			msecs_to_jiffies(4000));
}

static int __init modem_shutdown_init(void)
{
	int status = 0;

	printk(KERN_ERR"%s Modem Shutdown Timer Set START\n", __func__);

	setup_timer(&my_timer, my_timer_callback, 0);
	/* setup timer interval to 60000 msecs */
	wake_lock_init(&modem_power_wake_lock,
		WAKE_LOCK_SUSPEND, "modem_wakelock");

	INIT_DELAYED_WORK(&work_modem_init, kmod_work_handler);
	mod_timer(&my_timer, jiffies + msecs_to_jiffies(20000));
	printk(KERN_ERR"%s  Modem Shutdown Timer Set DONE\n", __func__);
	return status;
}


static void __exit modem_shutdown_exit(void)
{
	printk(KERN_ERR"%s Modem Shutdown Module unloaded!\n", __func__);
}


module_init(modem_shutdown_init);

module_exit(modem_shutdown_exit);
