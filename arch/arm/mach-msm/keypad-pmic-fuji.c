/* Copyright (c) 2011, Sony Ericsson Mobile Communications AB.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 */

#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/input.h>
#include <linux/mfd/pmic8058.h>
#include <linux/slab.h>
#include <linux/hrtimer.h>
#include <linux/wakelock.h>
#include <linux/mfd/pm8xxx/core.h>
#include <mach/board-msm8660.h>

#include "keypad-pmic-fuji.h"
#include "gpiomux-semc_fuji.h"

#define	SSBI_REG_ADDR_IRQ_RT_STATUS 0x1c3

enum {
	DEBOUNCE_STAT_WAIT,
	DEBOUNCE_STAT_DONE,
};

struct kp_key {
	int code;
	int state;
	int irq;
	int gpio;
	int id;
	int wake;
	ktime_t debounce_time;
	int debounce_stat;
};

struct kp_data {
	int num_keys;
	struct kp_key *keys;
	struct device *dev;
	struct input_dev *input;
	struct pm8058_chip *pm_chip;
	struct pm_gpio *pm_gpio_config;
	struct hrtimer timer;
	struct wake_lock wake_lock;
};

static int kp_pm_gpio_config(struct kp_data *dt,
		struct pm8058_chip *pm_chip, int gpio)
{
	int rc;

	rc = pm8xxx_gpio_config(PM8058_GPIO_PM_TO_SYS(gpio - 1),
				dt->pm_gpio_config);
	if (rc) {
		pr_err("%s: pm8058_gpio_config() failed, rc = %d.\n",
				__func__, rc);
		return rc;
	}

	return 0;
}

static int kp_get_state(struct kp_data *dt, struct kp_key *key)
{
	u8 data;
	if (pm8xxx_read_buf(dt->dev->parent, SSBI_REG_ADDR_IRQ_RT_STATUS,
			    &data, 1))
		return 0;
	return !(data & (1 << key->id));
}

static irqreturn_t kp_irq(int irq, void *dev_id)
{
	int i;
	struct device *dev = dev_id;
	struct kp_data *dt;
	dt = dev_get_drvdata(dev);

	for (i = 0; i < dt->num_keys; ++i) {
		if (dt->keys[i].debounce_time.tv64) {
			dt->keys[i].debounce_stat = DEBOUNCE_STAT_WAIT;
			disable_irq_nosync(dt->keys[i].irq);
			wake_lock(&dt->wake_lock);
			hrtimer_start(&dt->timer, dt->keys[i].debounce_time,
					HRTIMER_MODE_REL);
		} else {
			if (dt->keys[i].irq == irq) {
				dt->keys[i].state = kp_get_state(dt,
							&dt->keys[i]);
				input_report_key(dt->input, dt->keys[i].code,
						dt->keys[i].state);
				break;
			}
		}
	}
	input_sync(dt->input);
	return IRQ_HANDLED;
}

static enum hrtimer_restart kp_debounce_timer_func(struct hrtimer *timer)
{
	int i;
	int flag = 0;
	struct kp_data *dt =
		container_of(timer, struct kp_data, timer);

	for (i = 0 ; i < dt->num_keys ; i++) {
		if (dt->keys[i].debounce_stat == DEBOUNCE_STAT_WAIT) {
			dt->keys[i].debounce_stat = DEBOUNCE_STAT_DONE;
			enable_irq(dt->keys[i].irq);
			dt->keys[i].state = kp_get_state(dt,
							&dt->keys[i]);
			input_report_key(dt->input, dt->keys[i].code,
						dt->keys[i].state);
			flag = 1;
		}
	}

	if (flag == 1) {
		wake_unlock(&dt->wake_lock);
		input_sync(dt->input);
	}

	return HRTIMER_NORESTART;
}

static int kp_device_open(struct input_dev *dev)
{
	int i;
	struct kp_data *dt = input_get_drvdata(dev);

	for (i = 0; i < dt->num_keys; ++i) {
		if (!dt->keys[i].wake)
			enable_irq(dt->keys[i].irq);
	}

	return 0;
}

static void kp_device_close(struct input_dev *dev)
{
	int i;
	struct kp_data *dt = input_get_drvdata(dev);

	for (i = 0; i < dt->num_keys; ++i) {
		if (!dt->keys[i].wake)
			disable_irq(dt->keys[i].irq);
	}
}

static int __devinit kp_probe(struct platform_device *pdev)
{
	struct keypad_pmic_fuji_platform_data *pdata = pdev->dev.platform_data;
	struct kp_data *dt;
	struct pm8058_chip *pm_chip = dev_get_drvdata(pdev->dev.parent);
	int rc, i;

	if (pm_chip == NULL) {
		dev_err(&pdev->dev, "no parent pm8058\n");
		return -EINVAL;
	}
	if (pdata == NULL) {
		dev_err(&pdev->dev, "no pdata\n");
		return -EINVAL;
	}

	dt = kzalloc(sizeof(struct kp_data), GFP_KERNEL);
	if (dt == NULL)
		return -ENOMEM;

	dt->pm_chip = pm_chip;
	dt->num_keys = pdata->keymap_size;
	dt->pm_gpio_config = pdata->pm_gpio_config;
	dt->keys = kzalloc(dt->num_keys * sizeof(struct kp_key), GFP_KERNEL);
	if (dt->keys == NULL) {
		rc = -ENOMEM;
		goto err_key_alloc_failed;
	}

	platform_set_drvdata(pdev, dt);
	dt->dev	= &pdev->dev;

	dt->input = input_allocate_device();
	if (dt->input == NULL) {
		dev_err(&pdev->dev, "unable to allocate input device\n");
		rc = -ENOMEM;
		goto err_input_alloc_failed;
	}

	wake_lock_init(&dt->wake_lock, WAKE_LOCK_SUSPEND, KP_NAME);

	for (i = 0; i < dt->num_keys; ++i) {
		dt->keys[i].irq  = pdata->keymap[i].irq;
		dt->keys[i].gpio = pdata->keymap[i].gpio;
		dt->keys[i].code = pdata->keymap[i].code;
		dt->keys[i].wake = pdata->keymap[i].wake;
		dt->keys[i].debounce_time = pdata->keymap[i].debounce_time;
		dt->keys[i].debounce_stat = DEBOUNCE_STAT_DONE;
		/* our irq status will be a bitmask of the block which
		 * contains a certain gpio. since a block is only eight bits
		 * we need to find the correct bit in the block which
		 * reflects the requested gpio */
		dt->keys[i].id   = (pdata->keymap[i].gpio - 1) % 8;

		set_bit(dt->keys[i].code, dt->input->keybit);
		rc = kp_pm_gpio_config(dt, pm_chip, dt->keys[i].gpio);
		if (rc)
			goto err_bad_gpio_config;
	}

	dt->input->name = pdata->input_name;
	dt->input->phys = KP_DEVICE;
	dt->input->dev.parent	= pdev->dev.parent;
	dt->input->open		= kp_device_open;
	dt->input->close	= kp_device_close;
	dt->input->id.bustype	= BUS_HOST;
	dt->input->id.version	= 0x0001;
	dt->input->id.product	= 0x0001;
	dt->input->id.vendor	= 0x0001;
	set_bit(EV_KEY, dt->input->evbit);
	input_set_drvdata(dt->input, dt);

	rc = input_register_device(dt->input);
	if (rc < 0) {
		dev_err(&pdev->dev, "unable to register keypad input device\n");
		input_free_device(dt->input);
		goto err_input_register_failed;
	}
	hrtimer_init(&dt->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	dt->timer.function = kp_debounce_timer_func;

	for (i = 0; i < dt->num_keys; ++i) {
		rc = pm8xxx_read_irq_stat(dt->dev->parent, dt->keys[i].irq);
		if (rc < 0) {
			dev_err(&dt->input->dev, "unable to get irq status\n");
			/* non-fatal */
		} else {
			dt->keys[i].state = !rc;
			input_report_key(dt->input, dt->keys[i].code, !rc);
		}
		rc = request_threaded_irq(dt->keys[i].irq, NULL, kp_irq,
				IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING |
				IRQF_DISABLED, KP_NAME, &dt->input->dev);
		if (rc < 0) {
			dev_err(&dt->input->dev, "unable to request irq\n");
			goto err_request_irq;
		}
		if (dt->keys[i].wake)
			enable_irq_wake(dt->keys[i].irq);
	}

	return 0;

 err_request_irq:
	for (--i; i >= 0; --i)
		free_irq(dt->keys[i].irq, &dt->input->dev);
	input_unregister_device(dt->input);
 err_input_register_failed:
 err_bad_gpio_config:
 err_input_alloc_failed:
	kfree(dt->keys);
 err_key_alloc_failed:
	kfree(dt);
	return rc;
}


static int __devexit kp_remove(struct platform_device *pdev)
{
	struct kp_data *dt = platform_get_drvdata(pdev);
	int i;

	for (i = 0; i < dt->num_keys; ++i)
		free_irq(dt->keys[i].irq, &dt->input->dev);

	wake_lock_destroy(&dt->wake_lock);

	input_unregister_device(dt->input);
	platform_set_drvdata(pdev, NULL);
	kfree(dt->keys);
	kfree(dt);

	return 0;
}

static struct platform_driver kp_driver = {
	.probe		= kp_probe,
	.remove		= __devexit_p(kp_remove),
	.driver		= {
		.name = KP_NAME,
		.owner = THIS_MODULE,
	},
};

static int __init kp_init(void)
{
	return platform_driver_register(&kp_driver);
}

static void __exit kp_exit(void)
{
	platform_driver_unregister(&kp_driver);
}

module_init(kp_init);
module_exit(kp_exit);

MODULE_DESCRIPTION("pmic8058 GPIO keypad driver");
MODULE_VERSION("1.0");
