/*
 * Emulates a PWM device using an hrtimer and GPIO pin
 *
 * Copyright (C) 2011 Bill Gatliff <bgat@billgatliff.com>
 *
 * This program is free software; you may redistribute and/or modify
 * it under the terms of the GNU General Public License Version 2, as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/hrtimer.h>
#include <linux/err.h>
#include <linux/workqueue.h>
#include <linux/gpio.h>
#include <linux/slab.h>
#include <linux/completion.h>
#include <linux/configfs.h>
#include <linux/pwm/pwm.h>

#define DRIVER_NAME KBUILD_MODNAME

struct gpio_pwm {
	struct pwm_device *pwm;
	struct pwm_device_ops ops;
	struct hrtimer t;
	struct work_struct work;
	spinlock_t lock;
	struct completion complete;
	int gpio;
	int callback;
	unsigned long polarity : 1;
	unsigned long active : 1;
};

static void gpio_pwm_work(struct work_struct *work)
{
	struct gpio_pwm *gp = container_of(work, struct gpio_pwm, work);

	gpio_direction_output(gp->gpio, !(!!gp->polarity ^ !!gp->active));
}

static enum hrtimer_restart gpio_pwm_timeout(struct hrtimer *t)
{
	struct gpio_pwm *gp = container_of(t, struct gpio_pwm, t);
	struct pwm_device *p = gp->pwm;

	if (unlikely(p->duty_ticks == 0))
		gp->active = 0;
	else if (unlikely(p->duty_ticks == p->period_ticks))
		gp->active = 1;
	else
		gp->active ^= 1;

	if (gpio_cansleep(gp->gpio))
		schedule_work(&gp->work);
	else
		gpio_pwm_work(&gp->work);

	if (unlikely(!gp->active && test_bit(PWM_FLAG_STOP, &p->flags))) {
		clear_bit(PWM_FLAG_STOP, &p->flags);
		complete_all(&gp->complete);
		goto done;
	}

	if (gp->active)
		hrtimer_forward_now(&gp->t, ktime_set(0, p->duty_ticks));
	else
		hrtimer_forward_now(&gp->t, ktime_set(0, p->period_ticks
						      - p->duty_ticks));

done:
	return HRTIMER_RESTART;
}

static void gpio_pwm_start(struct pwm_device *p)
{
	struct gpio_pwm *gp = pwm_get_drvdata(p);

	gp->active = 0;
	hrtimer_start(&gp->t, ktime_set(0, p->period_ticks - p->duty_ticks),
		      HRTIMER_MODE_REL);
	set_bit(PWM_FLAG_RUNNING, &p->flags);
}

static int gpio_pwm_config_nosleep(struct pwm_device *p, struct pwm_config *c)
{
	struct gpio_pwm *gp = pwm_get_drvdata(p);
	int ret = 0;
	unsigned long flags;

	spin_lock_irqsave(&gp->lock, flags);

	switch (c->config_mask) {

	case BIT(PWM_CONFIG_DUTY_TICKS):
		p->duty_ticks = c->duty_ticks;
		break;

	case BIT(PWM_CONFIG_START):
		if (!hrtimer_active(&gp->t)) {
			gpio_pwm_start(p);
		}
		break;
	default:
		ret = -EINVAL;
		break;
	}

	spin_unlock_irqrestore(&gp->lock, flags);
	return ret;
}

static int gpio_pwm_stop_sync(struct pwm_device *p)
{
	struct gpio_pwm *gp = pwm_get_drvdata(p);
	int ret;
	int was_on = hrtimer_active(&gp->t);

	if (was_on) {
		do {
			init_completion(&gp->complete);
			set_bit(PWM_FLAG_STOP, &p->flags);
			ret = wait_for_completion_interruptible(&gp->complete);
			if (ret)
				return ret;
		} while (test_bit(PWM_FLAG_STOP, &p->flags));
	}

	clear_bit(PWM_FLAG_RUNNING, &p->flags);

	return was_on;
}

static int gpio_pwm_config(struct pwm_device *p, struct pwm_config *c)
{
	struct gpio_pwm *gp = pwm_get_drvdata(p);
	int was_on = 0;

	if (!gpio_pwm_config_nosleep(p, c))
		return 0;

	might_sleep();

	was_on = gpio_pwm_stop_sync(p);
	if (was_on < 0)
		return was_on;
	
	if (test_bit(PWM_CONFIG_PERIOD_TICKS, &c->config_mask))
		p->period_ticks = c->period_ticks;
	if (test_bit(PWM_CONFIG_DUTY_TICKS, &c->config_mask))
		p->duty_ticks = c->duty_ticks;
	if (test_bit(PWM_CONFIG_POLARITY, &c->config_mask))
		gp->polarity = !!c->polarity;

	if (test_bit(PWM_CONFIG_START, &c->config_mask)
	    || (was_on && !test_bit(PWM_CONFIG_STOP, &c->config_mask)))
		gpio_pwm_start(p);

	return 0;
}

static int gpio_pwm_request(struct pwm_device *p)
{
	p->tick_hz = 1000000000UL;
	return 0;
}

static const struct pwm_device_ops gpio_pwm_device_ops = {
	.owner		= THIS_MODULE,
	.config		= gpio_pwm_config,
	.config_nosleep	= gpio_pwm_config_nosleep,
	.request	= gpio_pwm_request,
};

struct pwm_device *gpio_pwm_create(int gpio)
{
	struct gpio_pwm *gp;
	int ret = 0;

	if (!gpio_is_valid(gpio))
		return ERR_PTR(-EINVAL);

	if (gpio_request(gpio, DRIVER_NAME))
		return ERR_PTR(-EBUSY);

	gp = kzalloc(sizeof(*gp), GFP_KERNEL);
	if (!gp)
		goto err_alloc;

	gp->gpio = gpio;
	INIT_WORK(&gp->work, gpio_pwm_work);
	init_completion(&gp->complete);
	hrtimer_init(&gp->t, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	gp->t.function = gpio_pwm_timeout;

	gp->pwm = pwm_register(&gpio_pwm_device_ops, NULL, "%s:%d", DRIVER_NAME, gpio);
	if (IS_ERR_OR_NULL(gp->pwm))
		goto err_pwm_register;

	pwm_set_drvdata(gp->pwm, gp);

	return gp->pwm;

err_pwm_register:
	kfree(gp);
err_alloc:
	gpio_free(gpio);
	return ERR_PTR(ret);
}
EXPORT_SYMBOL(gpio_pwm_create);

int gpio_pwm_destroy(struct pwm_device *p)
{
	struct gpio_pwm *gp = pwm_get_drvdata(p);

	if (pwm_is_requested(gp->pwm)) {
		if (pwm_is_running(gp->pwm))
			pwm_stop(gp->pwm);
		pwm_release(gp->pwm);
	}
	hrtimer_cancel(&gp->t);
	cancel_work_sync(&gp->work);

	pwm_unregister(gp->pwm);
	gpio_free(gp->gpio);
	kfree(gp);

	return 0;
}
EXPORT_SYMBOL(gpio_pwm_destroy);

#ifdef CONFIG_CONFIGFS_FS
struct gpio_pwm_target {
	struct config_item item;
	struct pwm_device *p;
};

static struct config_item_type gpio_pwm_item_type = {
	.ct_owner = THIS_MODULE,
};

static struct config_item *make_gpio_pwm_target(struct config_group *group,
						const char *name)
{
	struct gpio_pwm_target *t;
	unsigned long gpio;
	int ret;

	t = kzalloc(sizeof(*t), GFP_KERNEL);
	if (!t)
		return ERR_PTR(-ENOMEM);

	ret = strict_strtoul(name, 10, &gpio);
	if (ret || !gpio_is_valid(gpio)) {
		ret = -EINVAL;
		goto err_invalid_gpio;
	}

	config_item_init_type_name(&t->item, name, &gpio_pwm_item_type);

	t->p = gpio_pwm_create(gpio);
	if (IS_ERR_OR_NULL(t->p))
		goto err_gpio_pwm_create;

	return &t->item;

err_gpio_pwm_create:
err_invalid_gpio:
	kfree(t);
	return ERR_PTR(ret);
}

static void drop_gpio_pwm_target(struct config_group *group,
				 struct config_item *item)
{
	struct gpio_pwm_target *t =
		container_of(item, struct gpio_pwm_target, item);

	gpio_pwm_destroy(t->p);
	config_item_put(&t->item);
	kfree(t);
}

static struct configfs_group_operations gpio_pwm_subsys_group_ops = {
	.make_item = make_gpio_pwm_target,
	.drop_item = drop_gpio_pwm_target,
};

static struct config_item_type gpio_pwm_subsys_type = {
	.ct_group_ops = &gpio_pwm_subsys_group_ops,
	.ct_owner = THIS_MODULE,
};

static struct configfs_subsystem gpio_pwm_subsys = {
	.su_group = {
		.cg_item = {
			.ci_name = DRIVER_NAME,
			.ci_type = &gpio_pwm_subsys_type,
		},
	},
};

static int __init gpio_pwm_init(void)
{
	config_group_init(&gpio_pwm_subsys.su_group);
	mutex_init(&gpio_pwm_subsys.su_mutex);
	return configfs_register_subsystem(&gpio_pwm_subsys);
}
module_init(gpio_pwm_init);

static void __exit gpio_pwm_exit(void)
{
	configfs_unregister_subsystem(&gpio_pwm_subsys);
}
module_exit(gpio_pwm_exit);
#endif

MODULE_AUTHOR("Bill Gatliff <bgat@billgatliff.com>");
MODULE_DESCRIPTION("PWM channel emulator using GPIO and a high-resolution timer");
MODULE_LICENSE("GPL");
