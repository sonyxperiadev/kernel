/*
 * Bluetooth Broadcomm and low power control via GPIO
 *
 *  Copyright (C) 2011 Google, Inc.
 *  Copyright (C) 2013 Sony Mobile Communications AB.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this progrsoinlockam; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <asm/mach-types.h>
#include <linux/gpio.h>
#include <linux/hrtimer.h>
#include <linux/init.h>
#include <linux/irq.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of_gpio.h>
#include <linux/platform_device.h>
#include <linux/proc_fs.h>
#include <linux/regulator/consumer.h>
#include <linux/rfkill.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/termios.h>
#include <linux/uaccess.h>
#include <linux/wakelock.h>
#include <linux/bcm4339_bt_lpm.h>
#include <mach/gpiomux.h>
#include <mach/msm_serial_hs.h>
#include <net/bluetooth/bluetooth.h>

#define D_BCM_BLUETOOTH_CONFIG_MATCH_TABLE   "bcm,bcm4339"
#define PROC_DIR   "bluetooth/wakeup"

enum gpio_id {
	BT_REG_ON_PIN = 0,
	BT_HOST_WAKE_PIN,
	BT_DEV_WAKE_PIN,
};

static char const * const gpio_rsrcs[] = {
	"bt-reg-on-gpio",
	"bt-host-wake-gpio",
	"bt-dev-wake-gpio",
};

struct bcm4339_data {
	struct device *dev;
	struct platform_device *pdev;
	unsigned int gpios[ARRAY_SIZE(gpio_rsrcs)];
	bool is_enable;
};

static enum gpio_id req_ids[] = {
	BT_REG_ON_PIN,
	BT_HOST_WAKE_PIN,
	BT_DEV_WAKE_PIN,
};

static struct bcm4339_data *bcm4339_my_data;
static struct rfkill *bt_rfkill;
static bool bt_enabled;
static struct regulator *bt_batfet;
static char bt_wake_request[2];
static bool bt_sleep;
struct proc_dir_entry *bluetooth_dir, *wakeup_dir;

struct bcm_bt_lpm {
	int wake;
	int host_wake;

	struct hrtimer enter_lpm_timer;
	ktime_t enter_lpm_delay;

	struct uart_port *uport;

	struct wake_lock wake_lock;
	char wake_lock_name[16];

	struct mutex mutex;
	struct work_struct enter_lpm_work;
} bt_lpm;

static void bcm_bt_lpm_exit_lpm(void);

static int bcm4339_bt_rfkill_set_power(void *data, bool blocked)
{
	int regOnGpio;

	BT_DBG("Bluetooth device set power\n");

	regOnGpio = gpio_get_value(bcm4339_my_data->gpios[BT_REG_ON_PIN]);
	if (!bt_batfet) {
		bt_batfet = regulator_get(NULL, "batfet");
		if (IS_ERR_OR_NULL(bt_batfet)) {
			pr_debug("unable to get batfet reg. rc=%d\n",
				PTR_RET(bt_batfet));
			bt_batfet = NULL;
		}
	}

	/* rfkill_ops callback. Turn transmitter on when blocked is false */
	if (!blocked) {
		if (regOnGpio) {
			BT_DBG("Bluetooth device is already power on:%d\n",
				regOnGpio);
			return 0;
		}
		if (bt_batfet)
			regulator_enable(bt_batfet);
		gpio_set_value(bcm4339_my_data->gpios[BT_DEV_WAKE_PIN], 1);
		gpio_set_value(bcm4339_my_data->gpios[BT_REG_ON_PIN], 1);
		gpio_request(bcm4339_my_data->gpios[BT_HOST_WAKE_PIN],
			"BT_HOST_WAKE"); /* NO_PULL */
	} else {
		if (!regOnGpio) {
			BT_DBG("Bluetooth device is already power off:%d\n",
				regOnGpio);
			return 0;
		}
		gpio_free(bcm4339_my_data->gpios[BT_HOST_WAKE_PIN]); /* PULL_DOWN */
		gpio_set_value(bcm4339_my_data->gpios[BT_REG_ON_PIN], 0);
		if (bt_batfet)
			regulator_disable(bt_batfet);
	}
	bt_enabled = !blocked;

	return 0;
}

static const struct rfkill_ops bcm4339_bt_rfkill_ops = {
	.set_block = bcm4339_bt_rfkill_set_power,
};

static enum hrtimer_restart enter_lpm(struct hrtimer *timer)
{
	schedule_work(&bt_lpm.enter_lpm_work);
	return HRTIMER_NORESTART;
}

static void enter_lpm_work(struct work_struct *data)
{
	BT_DBG("Bluetooth device sleep\n");

	mutex_lock(&bt_lpm.mutex);
	bt_lpm.wake = 0;
	wake_unlock(&bt_lpm.wake_lock);
	bt_sleep = 1;
	gpio_set_value(bcm4339_my_data->gpios[BT_DEV_WAKE_PIN], 0);
	bt_wake_request[0] = '0';
	if (!bt_lpm.uport) {
		BT_DBG("Bluetooth device sleep uport is null\n");
	} else {
		BT_DBG("Bluetooth device sleep request HS UART clock off\n");
		msm_hs_set_mctrl(bt_lpm.uport, 0);
		msm_hs_request_clock_off(bt_lpm.uport);
	}
	mutex_unlock(&bt_lpm.mutex);
}

static void bcm_bt_lpm_reset_timer(void)
{
	mutex_lock(&bt_lpm.mutex);

	hrtimer_try_to_cancel(&bt_lpm.enter_lpm_timer);

	hrtimer_start(&bt_lpm.enter_lpm_timer, bt_lpm.enter_lpm_delay,
		HRTIMER_MODE_REL);
	mutex_unlock(&bt_lpm.mutex);
}

static void bcm_bt_lpm_exit_lpm(void)
{
	mutex_lock(&bt_lpm.mutex);

	hrtimer_try_to_cancel(&bt_lpm.enter_lpm_timer);

	bt_lpm.wake = 1;
	gpio_set_value(bcm4339_my_data->gpios[BT_DEV_WAKE_PIN], 1);
	if (bt_sleep && bt_lpm.uport) {
		BT_DBG("Bluetooth exit lpm request HS UART clock on\n");
		msm_hs_request_clock_on(bt_lpm.uport);
		msm_hs_set_mctrl(bt_lpm.uport, TIOCM_RTS);
		bt_sleep = 0;
	}

	hrtimer_start(&bt_lpm.enter_lpm_timer, bt_lpm.enter_lpm_delay,
		HRTIMER_MODE_REL);
	mutex_unlock(&bt_lpm.mutex);
}

void bcm_bt_lpm_exit_lpm_locked(struct uart_port *uport)
{
	bt_lpm.uport = uport;
}
EXPORT_SYMBOL(bcm_bt_lpm_exit_lpm_locked);

static irqreturn_t host_wake_isr(int irq, void *dev)
{
	int host_wake;

	mutex_lock(&bt_lpm.mutex);

	host_wake = gpio_get_value(bcm4339_my_data->gpios[BT_HOST_WAKE_PIN]);
	irq_set_irq_type(irq, host_wake ? IRQF_TRIGGER_LOW : IRQF_TRIGGER_HIGH);

	BT_DBG("Bluetooth Host wake up host_wake:%d\n", host_wake);

	if (!bt_lpm.uport) {
		bt_lpm.host_wake = host_wake;
		goto unlock_mutex;
	}

	if (host_wake == bt_lpm.host_wake) {
		BT_DBG("Bluetooth Host wake already handled\n");
		goto unlock_mutex;
	}

	bt_lpm.host_wake = host_wake;

	if (!host_wake) {
		BT_DBG("Bluetooth Host wake ignore irq of trigger low\n");
		goto unlock_mutex;
	}

	hrtimer_try_to_cancel(&bt_lpm.enter_lpm_timer);

	if (bt_sleep) {
		BT_DBG("Bluetooth Host wake up request HS UART clock on\n");
		msm_hs_request_clock_on(bt_lpm.uport);
		msm_hs_set_mctrl(bt_lpm.uport, TIOCM_RTS);
		bt_sleep = 0;
	}

	wake_lock(&bt_lpm.wake_lock);

unlock_mutex:
	mutex_unlock(&bt_lpm.mutex);
	return IRQ_HANDLED;
}

static int bcm_bt_lpm_init(struct platform_device *pdev)
{
	int irq;
	int ret;

	dev_dbg(&pdev->dev, "bcm_bt_lpm_init\n");

	hrtimer_init(&bt_lpm.enter_lpm_timer, CLOCK_MONOTONIC,
		HRTIMER_MODE_REL);
	bt_lpm.enter_lpm_delay = ktime_set(10, 0);  /* 10 sec */
	bt_lpm.enter_lpm_timer.function = enter_lpm;

	bt_lpm.host_wake = 0;

	bt_sleep = 0;

	mutex_init(&bt_lpm.mutex);

	irq = gpio_to_irq(bcm4339_my_data->gpios[BT_HOST_WAKE_PIN]);

	ret = request_threaded_irq(irq, NULL, host_wake_isr,
		IRQF_TRIGGER_HIGH | IRQF_ONESHOT, "bt host_wake", NULL);

	if (ret)
		return ret;

	ret = irq_set_irq_wake(irq, 1);
	if (ret)
		return ret;

	snprintf(bt_lpm.wake_lock_name, sizeof(bt_lpm.wake_lock_name),
			"BTLowPower");
	wake_lock_init(&bt_lpm.wake_lock, WAKE_LOCK_SUSPEND,
			 bt_lpm.wake_lock_name);

	INIT_WORK(&bt_lpm.enter_lpm_work, enter_lpm_work);

	return 0;
}

static void bcm4339_bluetooth_free_gpio(void)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(req_ids); i++) {
		if (req_ids[i] != BT_HOST_WAKE_PIN)
			gpio_free(bcm4339_my_data->gpios[req_ids[i]]);
	}
};

static int bcm4339_bluetooth_dev_init(struct platform_device *pdev,
				struct bcm4339_data *my_data) {
	int i, ret, gpio;
	unsigned int flags;
	struct device_node *of_node = pdev->dev.of_node;

	my_data->is_enable = false;

	for (i = 0; i < ARRAY_SIZE(gpio_rsrcs); i++) {
		gpio = of_get_gpio_flags(of_node, i, &flags);
		if (!gpio_is_valid(gpio)) {
			dev_err(&pdev->dev, "%s: invalid gpio #%s: %d\n",
				__func__, gpio_rsrcs[i], gpio);
			ret = -EPROBE_DEFER;
			goto error_gpio;
		}
		my_data->gpios[i] = gpio;
	}
	for (i = 0; i < ARRAY_SIZE(req_ids); i++) {
		if (req_ids[i] != BT_HOST_WAKE_PIN) {
			ret = gpio_request(my_data->gpios[req_ids[i]],
					gpio_rsrcs[req_ids[i]]);
			if (ret) {
				dev_err(&pdev->dev, "%s: request err %s: %d\n",
					__func__, gpio_rsrcs[req_ids[i]], ret);
				goto error_gpio_request;
			}
		}
	}

	return 0;

error_gpio_request:
	for (; i >= 0; --i) {
		if (req_ids[i] != BT_HOST_WAKE_PIN)
			gpio_free(my_data->gpios[req_ids[i]]);
	}
error_gpio:
	return ret;

}

static int bcm4339_bluetooth_probe(struct platform_device *pdev)
{
	int rc, ret;

	struct device_node *of_node = pdev->dev.of_node;
	dev_dbg(&pdev->dev, "bcm4339 bluetooth driver being loaded\n");

	if (!of_node) {
		dev_err(&pdev->dev, "%s: of_node is null\n", __func__);
		ret = -EPERM;
		goto error_of_node;
	}

	bcm4339_my_data = kzalloc(sizeof(*bcm4339_my_data), GFP_KERNEL);
	if (!bcm4339_my_data) {
		dev_err(&pdev->dev, "%s: no memory\n", __func__);
		ret = -ENOMEM;
		goto error_alloc_bcm4339_my_data;
	}
	bcm4339_my_data->pdev = pdev;

	ret = bcm4339_bluetooth_dev_init(pdev, bcm4339_my_data);
	if (ret) {
		dev_err(&pdev->dev, "%s: dev init failed\n", __func__);
		goto error_dev_init;
	}

	dev_dbg(&pdev->dev, "bcm4339_bluetooth_probe\n");

	bt_rfkill = rfkill_alloc("bcm4339 Bluetooth", &pdev->dev,
				RFKILL_TYPE_BLUETOOTH, &bcm4339_bt_rfkill_ops,
				NULL);

	if (unlikely(!bt_rfkill)) {
		ret = -ENOMEM;
		dev_err(&pdev->dev, "bcm4339_bluetooth_probe rfkill_alloc fail\n");
		goto  error_free_gpio;
	}

	rfkill_set_states(bt_rfkill, true, false);
	rc = rfkill_register(bt_rfkill);

	if (unlikely(rc)) {
		rfkill_destroy(bt_rfkill);
		ret = -ENOMEM;
		dev_err(&pdev->dev, "bcm4339_bluetooth_probe rfkill_register fail\n");
		goto  error_free_gpio;
	}

	ret = bcm_bt_lpm_init(pdev);
	if (ret) {
		rfkill_unregister(bt_rfkill);
		rfkill_destroy(bt_rfkill);
		dev_err(&pdev->dev, "bcm4339_bluetooth_probe bcm_bt_lpm_init fail\n");
		ret = -1;
		goto  error_free_gpio;
	}
	return 0;

error_free_gpio:
	bcm4339_bluetooth_free_gpio();
error_dev_init:
	kzfree(bcm4339_my_data);
error_alloc_bcm4339_my_data:
error_of_node:
	return ret;
}

static int bcm4339_read_proc_proto(char *page, char **start, off_t offset,
					int count, int *eof, void *data)
{
	unsigned long outbyte = 0;

	BT_DBG("Bluetooth read proc request\n");

	if (offset > 0) {
		*eof = 1;
		return 0;
	}
	outbyte = snprintf(page, 2, "%s", bt_wake_request);
	BT_DBG("proc_read len = %lu\n", outbyte);
	*eof = 1;
	return outbyte;
}

static int bcm4339_write_proc_proto(struct file *file, const char *buffer,
					unsigned long count, void *data)
{
	char proto;

	if (count < 1) {
		BT_ERR("Bluetooth write proc count error %d\n", (int)count);
		return -EINVAL;
	}

	if (copy_from_user(&proto, buffer, 1)) {
		BT_ERR("Bluetooth write proc could not get data\n");
		return -EFAULT;
	}

	if (proto == '1')
		bcm_bt_lpm_exit_lpm();
	else if (proto == '2')
		bcm_bt_lpm_reset_timer();
	else
		BT_DBG("Bluetooth write proc invalid write data %c\n", proto);

	bt_wake_request[0] = proto;

	/* claim that we wrote everything */
	return count;
}

static void bcm4339_proc_exit(void)
{
	remove_proc_entry("proto", wakeup_dir);
	remove_proc_entry("wakeup", bluetooth_dir);
	remove_proc_entry("bluetooth", 0);
}

static int bcm4339_proc_init(void)
{
	struct proc_dir_entry *ent;
	int retval;

	bt_wake_request[0] = '0';

	bluetooth_dir = proc_mkdir("bluetooth", NULL);
	if (bluetooth_dir == NULL) {
		BT_ERR("Unable to create /proc/bluetooth director\ny");
		return -ENOMEM;
	}

	wakeup_dir = proc_mkdir("wakeup", bluetooth_dir);
	if (wakeup_dir == NULL) {
		BT_ERR("Unable to create /proc/%s directory\n", PROC_DIR);
		return -ENOMEM;
	}

	/* read/write proc entries */
	ent = create_proc_entry("proto", 0, wakeup_dir);
	if (ent == NULL) {
		BT_ERR("Unable to create /proc/%s/proto entry\n", PROC_DIR);
		retval = -ENOMEM;
		goto fail;
	}
	ent->read_proc =  bcm4339_read_proc_proto;
	ent->write_proc =  bcm4339_write_proc_proto;

	return 0;
fail:
	bcm4339_proc_exit();
	return retval;
}

static int bcm4339_bluetooth_remove(struct platform_device *pdev)
{
	dev_dbg(&pdev->dev, "bcm4339_bluetooth_remove\n");
	rfkill_unregister(bt_rfkill);
	rfkill_destroy(bt_rfkill);

	wake_lock_destroy(&bt_lpm.wake_lock);
	kzfree(bcm4339_my_data);

	return 0;
}

int bcm4339_bluetooth_suspend(struct platform_device *pdev, pm_message_t state)
{
	int host_wake;

	dev_dbg(&pdev->dev, "bcm4339_bluetooth_suspend\n");
	host_wake = gpio_get_value(bcm4339_my_data->gpios[BT_HOST_WAKE_PIN]);

	if (host_wake)
		return -EBUSY;

	return 0;
}

int bcm4339_bluetooth_resume(struct platform_device *pdev)
{
	dev_dbg(&pdev->dev, "bcm4339_bluetooth_resume\n");
	return 0;
}


static struct of_device_id bcm4339_match_table[] = {
	{.compatible = D_BCM_BLUETOOTH_CONFIG_MATCH_TABLE },
	{}
};


static struct platform_driver bcm4339_bluetooth_platform_driver = {
	.probe = bcm4339_bluetooth_probe,
	.remove = bcm4339_bluetooth_remove,
	.suspend = bcm4339_bluetooth_suspend,
	.resume = bcm4339_bluetooth_resume,
	.driver = {
		   .name = "bcm4339_bluetooth",
		   .owner = THIS_MODULE,
		   .of_match_table = bcm4339_match_table,
		   },
};

static int __init bcm4339_bluetooth_init(void)
{
	int ret;
	bt_enabled = false;
	ret = bcm4339_proc_init();
	if (ret < 0) {
		BT_ERR("bcm4339_proc_init() failed\n");
		return ret;
	}
	return platform_driver_register(&bcm4339_bluetooth_platform_driver);
}

static void __exit bcm4339_bluetooth_exit(void)
{
	bcm4339_proc_exit();
	platform_driver_unregister(&bcm4339_bluetooth_platform_driver);
}


module_init(bcm4339_bluetooth_init);
module_exit(bcm4339_bluetooth_exit);

MODULE_ALIAS("platform:bcm4339");
MODULE_DESCRIPTION("bcm4339_bluetooth");
MODULE_AUTHOR("Jaikumar Ganesh <jaikumar@google.com>");
MODULE_LICENSE("GPL v2");
