/*******************************************************************************
* Copyright 2010-2012 Broadcom Corporation.  All rights reserved.
*
* @file	drivers/xxx/bcmbt_lpm.c
*
* This driver handles Broadcom LPM via tty line displine (N_BRCM_HCI)
* It allows to asssert or de-assert BT_WAKE via ioct()
* HOST_WAKE pin is handle in isr calling the platform host allow sleep/dis-
* allow sleep functions (pi_mgr, wakelock)
*
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

/*#define DEBUG */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <mach/gpio.h>
#include <linux/ioctl.h>
#include <linux/broadcom/bcmbt_lpm.h>
#ifdef CONFIG_BCM_BT_GPS_SELFTEST
#include <linux/broadcom/bcmbt_gps.h>
#endif
#include <linux/platform_device.h>
#include <linux/sched.h>

#include <linux/serial_core.h>
#include <linux/tty.h>

#include <linux/uaccess.h>
#ifdef CONFIG_HAS_WAKELOCK
#include <linux/wakelock.h>
#endif
#include <linux/clk.h>

#include <linux/io.h>
#include <asm/irq.h>

#ifdef CONFIG_KONA_PI_MGR
#include <mach/pi_mgr.h>
#include <plat/pi_mgr.h>
#endif


#define BTLPM_ENABLE_CLOCK 1
#define BTLPM_DISABLE_CLOCK 0

#define BTLPM_LDISC_FREE_RESOURCES

static struct bcm_bt_lpm_data bcm_bt_lpm_data = {
	0,
};

static struct pi_mgr_qos_node btqos_node;

struct bcmbt_ldisc_data {
	int	(*open)(struct tty_struct *);
	void	(*close)(struct tty_struct *);
};

static struct tty_ldisc_ops bcmbt_ldisc_ops;
static struct bcmbt_ldisc_data bcmbt_ldisc_saved;

static int bcm_assert_bt_wake(void);
static int bcm_deassert_bt_wake(void);
static int bcm_init_bt_wake(struct bcm_bt_lpm_platform_data *gpio_data);
static int bcm_init_hostwake(struct bcm_bt_lpm_platform_data *gpio_data);
static void clean_bt_wake(struct bcm_bt_lpm_platform_data *pdata, bool b_tty);
static void clean_host_wake(struct bcm_bt_lpm_platform_data *pdata);

static int bcm_assert_bt_wake(void)
{
	if (unlikely((bcm_bt_lpm_data.state == DISABLE_LPM) ||
			(bcm_bt_lpm_data.gpio_bt_wake == -1)))
		return -EFAULT;

	gpio_set_value(bcm_bt_lpm_data.gpio_bt_wake, BT_WAKE_ASSERT);

#ifdef CONFIG_HAS_WAKELOCK
	wake_lock(&bcm_bt_lpm_data.bt_wake_lock);
#endif

	pr_debug("bt_wake assert: gpio: %d, %d\n", bcm_bt_lpm_data.gpio_bt_wake,
		 BT_WAKE_ASSERT);

	return 0;
}

static int bcm_deassert_bt_wake(void)
{
	if (unlikely((bcm_bt_lpm_data.state == DISABLE_LPM) ||
			(bcm_bt_lpm_data.gpio_bt_wake == -1)))
		return -EFAULT;

	gpio_set_value(bcm_bt_lpm_data.gpio_bt_wake, BT_WAKE_DEASSERT);

	/* release lock: enable deep sleep */
#ifdef CONFIG_HAS_WAKELOCK
	wake_unlock(&bcm_bt_lpm_data.bt_wake_lock);
#endif

	pr_debug("bt_wake DEassert: gpio: %d, %d\n",
		 bcm_bt_lpm_data.gpio_bt_wake, BT_WAKE_DEASSERT);
	return 0;
}

static int bcm_get_bt_wake_state(unsigned long arg)
{
	void __user *uarg = (void __user *)arg;
	unsigned long tmp;

	if (unlikely((bcm_bt_lpm_data.state == DISABLE_LPM) ||
			(bcm_bt_lpm_data.gpio_bt_wake == -1)))
		return -EFAULT;

	tmp = gpio_get_value(bcm_bt_lpm_data.gpio_bt_wake);

	pr_info("bcm_get_bt_wake_state(bt_wake:%d)\n",
		bcm_bt_lpm_data.gpio_bt_wake);

	if (copy_to_user(uarg, &tmp, sizeof(*uarg)))
		return -EFAULT;
	return 0;
}

static int bcmbt_init_peripheral_clock(void)
{
	int qos_btlpm = pi_mgr_qos_add_request(&btqos_node, "bcmbt_qos_node",
					       PI_MGR_PI_ID_ARM_SUB_SYSTEM,
					       PI_MGR_QOS_DEFAULT_VALUE);
	if (qos_btlpm < 0)
		return -1;

	return 0;
}

static int bcmbt_release_peripheral_clock(void)
{
	pi_mgr_qos_request_update(&btqos_node, PI_MGR_QOS_DEFAULT_VALUE);
	pi_mgr_qos_request_remove(&btqos_node);
	return 0;
}

static int bcmbt_tty_ioctl(struct tty_struct *tty, struct file *file,
			   unsigned int cmd, unsigned long arg)
{
	int rc = -1;

	switch (cmd) {
	case TIO_ASSERT_BT_WAKE:
		rc = bcm_assert_bt_wake();
		break;

	case TIO_DEASSERT_BT_WAKE:
		rc = bcm_deassert_bt_wake();
		break;

	case TIO_GET_BT_WAKE_STATE:
		rc = bcm_get_bt_wake_state(arg);
		break;

	case TIO_SET_LPM_MODE:
		pr_warn("bcmbt_tty_ioctl(TIO_SET_LPM_MODE):not implemented\n");
		break;

#ifdef CONFIG_BCM_BT_GPS_SELFTEST
	case TIO_GPS_SETLFTEST_CMD: {
		struct bcmbt_gps_selftest_cmd test_cmd;
		if (copy_from_user(&test_cmd, (const void __user *)arg,
				sizeof(struct bcmbt_gps_selftest_cmd))) {
			pr_err("bcmbt_tty_ioctl(): Failed getting user data");
			return -EFAULT;
		}
		rc = bcmbt_gps_selftest(&test_cmd);
	}
		break;
#endif
	default:
		return n_tty_ioctl_helper(tty, file, cmd, arg);

	}
	return rc;
}


static int bcmbt_tty_open(struct tty_struct *tty)
{
#ifdef BTLPM_LDISC_FREE_RESOURCES
	struct bcm_bt_lpm_platform_data gpio_data;

	gpio_data.gpio_bt_wake = bcm_bt_lpm_data.gpio_bt_wake;
	gpio_data.gpio_host_wake = bcm_bt_lpm_data.gpio_host_wake;
	/* do not init tty ldisc as we are called from ldisc */
	bcm_init_bt_wake(&gpio_data);
#endif
	pr_debug("bcmbt_tty_open()::open(): x%p", bcmbt_ldisc_saved.open);
#ifdef CONFIG_BCM_BT_GPS_SELFTEST
	__bcmbt_gps_selftest_init();
#endif
	if (bcmbt_ldisc_saved.open)
		return bcmbt_ldisc_saved.open(tty);
	return 0;
}

static void bcmbt_tty_close(struct tty_struct *tty)
{
#ifdef BTLPM_LDISC_FREE_RESOURCES
	struct bcm_bt_lpm_platform_data gpio_data;
	gpio_data.gpio_bt_wake = bcm_bt_lpm_data.gpio_bt_wake;
	gpio_data.gpio_host_wake = bcm_bt_lpm_data.gpio_host_wake;
	/* do not init tty ldisc as we are called from ldisc */
	clean_bt_wake(&gpio_data, false);
	clean_host_wake(&gpio_data);
#endif
	pr_debug("bcmbt_tty_close()::close(): x%p", bcmbt_ldisc_saved.close);
#ifdef CONFIG_BCM_BT_GPS_SELFTEST
	/* make sure resources for GPS selftest are freed on ldisc closure! */
	__bcmbt_gps_selftest_exit();
#endif
	if (bcmbt_ldisc_saved.close)
		bcmbt_ldisc_saved.close(tty);
}

static int bcmbt_tty_init(void)
{
	int err;

	/* Inherit the N_TTY's ops */
	n_tty_inherit_ops(&bcmbt_ldisc_ops);

	bcmbt_ldisc_ops.owner = THIS_MODULE;
	bcmbt_ldisc_ops.name = "bcmbt_tty";
	bcmbt_ldisc_ops.ioctl = bcmbt_tty_ioctl;
	bcmbt_ldisc_saved.open = bcmbt_ldisc_ops.open;
	bcmbt_ldisc_saved.close = bcmbt_ldisc_ops.close;
	bcmbt_ldisc_ops.open = bcmbt_tty_open;
	bcmbt_ldisc_ops.close = bcmbt_tty_close;

	err = tty_register_ldisc(N_BRCM_HCI, &bcmbt_ldisc_ops);
	if (err)
		pr_err("can't register N_BRCM_HCI line discipline\n");
	else
		pr_info("N_BRCM_HCI line discipline registered\n");

	return err;
}

static void bcmbt_tty_cleanup(void)
{
	int err;

	err = tty_unregister_ldisc(N_BRCM_HCI);
	if (err)
		pr_err("can't unregister N_BRCM_HCI line discipline\n");
	else
		pr_info("N_BRCM_HCI line discipline removed\n");
}

static void bcmbt_peripheral_clocks(int enable)
{

	if (enable) {
		pi_mgr_qos_request_update(&btqos_node, 0);
#ifdef CONFIG_HAS_WAKELOCK
		wake_lock(&bcm_bt_lpm_data.host_wake_lock);
#endif

	} else {
		pi_mgr_qos_request_update(&btqos_node,
					  PI_MGR_QOS_DEFAULT_VALUE);
		/* release lock: enable deep sleep */
#ifdef CONFIG_HAS_WAKELOCK
		wake_unlock(&bcm_bt_lpm_data.host_wake_lock);
#endif

	}

}

static irqreturn_t host_wake_isr(int irq, void *dev)
{
	unsigned int host_wake;

	if (unlikely(bcm_bt_lpm_data.state == DISABLE_LPM))
		return IRQ_HANDLED;

	host_wake = gpio_get_value(bcm_bt_lpm_data.gpio_host_wake);

	if (HOST_WAKE_ASSERT == host_wake) {
		/* hold lock: disable deep sleep */
		bcmbt_peripheral_clocks(BTLPM_ENABLE_CLOCK);
		pr_debug("host_wake assert, hold lock at %lld\n",
			 ktime_to_ns(ktime_get()));
	} else {
		/* release lock: enable deep sleep */
		bcmbt_peripheral_clocks(BTLPM_DISABLE_CLOCK);
		pr_debug("host_wake DEassert, release lock  at %lld\n",
			 ktime_to_ns(ktime_get()));
	}

	return IRQ_HANDLED;
}

static int bcm_init_hostwake(struct bcm_bt_lpm_platform_data *gpio_data)
{
	unsigned int irq;
	int ret;
	if (bcm_bt_lpm_data.host_wake_installed) {
		pr_info("host wake irq is already installed\n");
		return 0;
	} else
		bcm_bt_lpm_data.host_wake_installed = 1;

	bcmbt_init_peripheral_clock();

#ifdef CONFIG_HAS_WAKELOCK
	wake_lock_init(&bcm_bt_lpm_data.host_wake_lock, WAKE_LOCK_SUSPEND,
		       "HOSTWAKE");
#endif
	irq = gpio_to_irq(gpio_data->gpio_host_wake);

	pr_info("host wake irq=%d\n", irq);

	ret = request_irq(irq, host_wake_isr,
			(IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING |
			IRQF_NO_SUSPEND), "bt_host_wake",
			(void *)&bcm_bt_lpm_data);

	pr_info("request_irq returned value=%u\n", ret);

	return ret;
}

static int bcm_init_bt_wake(struct bcm_bt_lpm_platform_data *gpio_data)
{
	if (0 != bcm_bt_lpm_data.bt_wake_installed) {
		pr_info("bcm_init_bt_wake( gpio_bt_wake: %d )::already "
			"installed", gpio_data->gpio_bt_wake);
		return 0;
	}
	pr_info("bcm_init_bt_wake( gpio_bt_wake: %d )",
			gpio_data->gpio_bt_wake);

	bcm_bt_lpm_data.gpio_bt_wake = gpio_data->gpio_bt_wake;
	bcm_bt_lpm_data.gpio_host_wake = gpio_data->gpio_host_wake;

#ifdef CONFIG_HAS_WAKELOCK
	wake_lock_init(&bcm_bt_lpm_data.bt_wake_lock, WAKE_LOCK_SUSPEND,
			"BTWAKE");
#endif
	bcm_bt_lpm_data.bt_wake_installed = 1;

	if (bcm_init_hostwake(gpio_data))
		pr_info("host_wake_isr installation failed\n");
	return 0;
}

/* WARNING b_tty==true force unregister of ldisc, only use it if module
 * is unregistered! */
static void clean_bt_wake(struct bcm_bt_lpm_platform_data *pdata, bool b_tty)
{
	if (unlikely((pdata->gpio_bt_wake == -1) ||
			(bcm_bt_lpm_data.bt_wake_installed == 0))) {
		if (b_tty)
			goto tty_only;
		return;
	}
#ifdef CONFIG_HAS_WAKELOCK
	wake_lock_destroy(&bcm_bt_lpm_data.bt_wake_lock);
#endif
	gpio_free((unsigned)pdata->gpio_bt_wake);
tty_only:
	bcm_bt_lpm_data.bt_wake_installed = 0;
	if (b_tty)
		bcmbt_tty_cleanup();
}

static void clean_host_wake(struct bcm_bt_lpm_platform_data *pdata)
{
	unsigned int irq;
	if (unlikely((pdata->gpio_host_wake == -1) ||
			(bcm_bt_lpm_data.host_wake_installed == 0)))
		return;

	bcm_bt_lpm_data.host_wake_installed = 0;
#ifdef CONFIG_HAS_WAKELOCK
	wake_lock_destroy(&bcm_bt_lpm_data.host_wake_lock);
#endif
	irq = gpio_to_irq(pdata->gpio_host_wake);
	/* make sure to use the same dev_id as in request_irq */
	free_irq(irq, (void *)&bcm_bt_lpm_data);
	pr_debug("freed host wake irq=%d\n", irq);
	gpio_free((unsigned)pdata->gpio_host_wake);
	bcmbt_release_peripheral_clock();
}

static int bcmbt_lpm_probe(struct platform_device *pdev)
{
	struct bcm_bt_lpm_platform_data *pdata;
	int rc;

	pdata = pdev->dev.platform_data;
	bcm_bt_lpm_data.polarity = HOST_WAKE_ASSERT;
	bcm_bt_lpm_data.state = ENABLE_LPM_TYPE_OOB_USER;
	bcm_bt_lpm_data.timeout = DEFAULT_TO;
#ifdef BTLPM_LDISC_FREE_RESOURCES
	/* store platform gpio assignement for ldisc use at open */
	bcm_bt_lpm_data.gpio_bt_wake = pdata->gpio_bt_wake;
	bcm_bt_lpm_data.gpio_host_wake = pdata->gpio_host_wake;
#else
	bcm_init_bt_wake(pdata);
#endif
	/* register the tty line discipline driver */
	rc = bcmbt_tty_init();
	if (rc) {
		pr_err("%s: bcmbt_tty_init failed\n", __func__);
		return -1;
	}
	return 0;
}

static int bcmbt_lpm_remove(struct platform_device *pdev)
{
	struct bcm_bt_lpm_platform_data *pdata = pdev->dev.platform_data;

	if (pdata) {
		clean_bt_wake(pdata, true);
		clean_host_wake(pdata);
	}
	pr_info("bcmbt_lpm_remove() unloading bcmbt-lpm, bt_wake: %d, host_"
			"wake: %d\n", bcm_bt_lpm_data.bt_wake_installed,
			bcm_bt_lpm_data.host_wake_installed);
	return 0;
}

static struct platform_driver bcmbt_lpm_platform_driver = {
	.probe = bcmbt_lpm_probe,
	.remove = bcmbt_lpm_remove,
	.driver = {
		   .name = "bcmbt-lpm",
		   .owner = THIS_MODULE,
		   },
};

static int __init bcmbt_lpm_init(void)
{
	return platform_driver_register(&bcmbt_lpm_platform_driver);
}

static void __exit bcmbt_lpm_exit(void)
{
	platform_driver_unregister(&bcmbt_lpm_platform_driver);
}

module_init(bcmbt_lpm_init);
module_exit(bcmbt_lpm_exit);

MODULE_DESCRIPTION("bcmbt-lpm");
MODULE_AUTHOR("Broadcom");
MODULE_LICENSE("GPL");
MODULE_ALIAS_LDISC(N_BRCM_HCI);
