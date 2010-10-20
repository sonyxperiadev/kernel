/*
 * PWM API implementation
 *
 * Copyright (C) 2011 Bill Gatliff <bgat@billgatliff.com>
 * Copyright (C) 2011 Arun Murthy <arun.murthy@stericsson.com>
 *
 * This program is free software; you may redistribute and/or modify
 * it under the terms of the GNU General Public License version 2 as
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
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/pwm/pwm.h>

static const char *REQUEST_SYSFS = "sysfs";
static struct class pwm_class;

void pwm_set_drvdata(struct pwm_device *p, void *data)
{
	dev_set_drvdata(&p->dev, data);
}
EXPORT_SYMBOL(pwm_set_drvdata);

void *pwm_get_drvdata(const struct pwm_device *p)
{
	return dev_get_drvdata(&p->dev);
}
EXPORT_SYMBOL(pwm_get_drvdata);

static inline struct pwm_device *to_pwm_device(struct device *dev)
{
	return container_of(dev, struct pwm_device, dev);
}

static int pwm_match_name(struct device *dev, void *name)
{
	return !strcmp(name, dev_name(dev));
}

static int __pwm_request(struct pwm_device *p, const char *label)
{
	int ret;

	if (!try_module_get(p->ops->owner))
		return -ENODEV;

	ret = test_and_set_bit(PWM_FLAG_REQUESTED, &p->flags);
	if (ret) {
		ret = -EBUSY;
		goto err_flag_requested;
	}

	p->label = label;

	if (p->ops->request) {
		ret = p->ops->request(p);
		if (ret)
			goto err_request_ops;
			
	}

	return 0;

err_request_ops:
	clear_bit(PWM_FLAG_REQUESTED, &p->flags);

err_flag_requested:
	module_put(p->ops->owner);
	return ret;
}

/**
 * pwm_request - request a PWM device by name
 *
 * @name: name of PWM device
 * @label: label that identifies requestor
 *
 * The @name format is driver-specific, but is typically of the form
 * "<bus_id>:<chan>".  For example, "atmel_pwmc:1" identifies the
 * second ATMEL PWMC peripheral channel.
 *
 * Returns a pointer to the requested PWM device on success, -EINVAL
 * otherwise.
 */
struct pwm_device *pwm_request(const char *name, const char *label)
{
	struct device *d;
	struct pwm_device *p;
	int ret;

	d = class_find_device(&pwm_class, NULL, (char*)name, pwm_match_name);
	if (!d)
		return ERR_PTR(-EINVAL);

	p = to_pwm_device(d);
	ret = __pwm_request(p, label);
	if (ret) {
		put_device(d);
		return ERR_PTR(ret);
	}

	return p;
}
EXPORT_SYMBOL(pwm_request);

/**
 * pwm_release - releases a previously-requested PWM channel
 *
 * @p: PWM device to release
 */
void pwm_release(struct pwm_device *p)
{
	if (!test_and_clear_bit(PWM_FLAG_REQUESTED, &p->flags)) {
		WARN(1, "%s: releasing unrequested PWM device %s\n",
		     __func__, dev_name(&p->dev));
		return;
	}

	pwm_stop(p);
	pwm_unsynchronize(p, NULL);
	p->label = NULL;

	if (p->ops->release)
		p->ops->release(p);

	put_device(&p->dev);
	module_put(p->ops->owner);
}
EXPORT_SYMBOL(pwm_release);

static unsigned long pwm_ns_to_ticks(struct pwm_device *p, unsigned long nsecs)
{
	unsigned long long ticks;

	ticks = nsecs;
	ticks *= p->tick_hz;
	do_div(ticks, 1000000000);
	return ticks;
}

static unsigned long pwm_ticks_to_ns(struct pwm_device *p, unsigned long ticks)
{
	unsigned long long ns;

	if (!p->tick_hz)
		return 0;

	ns = ticks;
	ns *= 1000000000UL;
	do_div(ns, p->tick_hz);
	return ns;
}

/**
 * pwm_config_nosleep - configures a PWM device in an atomic context
 *
 * @p: PWM device to configure
 * @c: configuration to apply to the PWM device
 *
 * Returns whatever the PWM device driver's config_nosleep() returns,
 * or -ENOSYS if the PWM device driver does not have a
 * config_nosleep() method.
 */
int pwm_config_nosleep(struct pwm_device *p, struct pwm_config *c)
{
	if (!p->ops->config_nosleep)
		return -ENOSYS;

	return p->ops->config_nosleep(p, c);
}
EXPORT_SYMBOL(pwm_config_nosleep);

/**
 * pwm_config - configures a PWM device
 *
 * @p: PWM device to configure
 * @c: configuration to apply to the PWM device
 *
 * Performs some basic sanity checking of the parameters, and returns
 * -EINVAL if they are found to be invalid.  Otherwise, returns
 * whatever the PWM device's config() method returns.
 */
int pwm_config(struct pwm_device *p, struct pwm_config *c)
{
	int ret = 0;

	dev_dbg(&p->dev, "%s: config_mask %lu period_ticks %lu "
		"duty_ticks %lu polarity %d\n",
		__func__, c->config_mask, c->period_ticks,
		c->duty_ticks, c->polarity);

	switch (c->config_mask & (BIT(PWM_CONFIG_PERIOD_TICKS)
				  | BIT(PWM_CONFIG_DUTY_TICKS))) {
	case BIT(PWM_CONFIG_PERIOD_TICKS):
		if (p->duty_ticks > c->period_ticks)
			ret = -EINVAL;
		break;
	case BIT(PWM_CONFIG_DUTY_TICKS):
		if (p->period_ticks < c->duty_ticks)
			ret = -EINVAL;
		break;
	case BIT(PWM_CONFIG_DUTY_TICKS) | BIT(PWM_CONFIG_PERIOD_TICKS):
		if (c->duty_ticks > c->period_ticks)
			ret = -EINVAL;
		break;
	default:
		break;
	}

	if (ret)
		return ret;
	return p->ops->config(p, c);
}
EXPORT_SYMBOL(pwm_config);

/**
 * pwm_set - compatibility function to ease migration from older code
 * @p: the PWM device to configure
 * @period_ns: period of the desired PWM signal, in nanoseconds
 * @duty_ns: duration of active portion of desired PWM signal, in nanoseconds
 * @polarity: 1 if active period is high, zero otherwise
 */
int pwm_set(struct pwm_device *p, unsigned long period_ns,
	    unsigned long duty_ns, int polarity)
{
	struct pwm_config c = {
		.config_mask = (BIT(PWM_CONFIG_PERIOD_TICKS)
				| BIT(PWM_CONFIG_DUTY_TICKS)
				| BIT(PWM_CONFIG_POLARITY)),
		.period_ticks = pwm_ns_to_ticks(p, period_ns),
		.duty_ticks = pwm_ns_to_ticks(p, duty_ns),
		.polarity = polarity
	};

	return pwm_config(p, &c);
}
EXPORT_SYMBOL(pwm_set);

int pwm_set_period_ns(struct pwm_device *p, unsigned long period_ns)
{
	struct pwm_config c = {
		.config_mask = BIT(PWM_CONFIG_PERIOD_TICKS),
		.period_ticks = pwm_ns_to_ticks(p, period_ns),
	};

	return pwm_config(p, &c);
}
EXPORT_SYMBOL(pwm_set_period_ns);

unsigned long pwm_get_period_ns(struct pwm_device *p)
{
	return pwm_ticks_to_ns(p, p->period_ticks);
}
EXPORT_SYMBOL(pwm_get_period_ns);

int pwm_set_duty_ns(struct pwm_device *p, unsigned long duty_ns)
{
	struct pwm_config c = {
		.config_mask = BIT(PWM_CONFIG_DUTY_TICKS),
		.duty_ticks = pwm_ns_to_ticks(p, duty_ns),
	};
	return pwm_config(p, &c);
}
EXPORT_SYMBOL(pwm_set_duty_ns);

unsigned long pwm_get_duty_ns(struct pwm_device *p)
{
	return pwm_ticks_to_ns(p, p->duty_ticks);
}
EXPORT_SYMBOL(pwm_get_duty_ns);

int pwm_set_polarity(struct pwm_device *p, int polarity)
{
	struct pwm_config c = {
		.config_mask = BIT(PWM_CONFIG_POLARITY),
		.polarity = polarity,
	};
	return pwm_config(p, &c);
}
EXPORT_SYMBOL(pwm_set_polarity);

int pwm_start(struct pwm_device *p)
{
	struct pwm_config c = {
		.config_mask = BIT(PWM_CONFIG_START),
	};
	return pwm_config(p, &c);
}
EXPORT_SYMBOL(pwm_start);

int pwm_stop(struct pwm_device *p)
{
	struct pwm_config c = {
		.config_mask = BIT(PWM_CONFIG_STOP),
	};
	return pwm_config(p, &c);
}
EXPORT_SYMBOL(pwm_stop);

int pwm_synchronize(struct pwm_device *p, struct pwm_device *to_p)
{
	if (!p->ops->synchronize)
		return -ENOSYS;

	return p->ops->synchronize(p, to_p);
}
EXPORT_SYMBOL(pwm_synchronize);

int pwm_unsynchronize(struct pwm_device *p, struct pwm_device *from_p)
{
	if (!p->ops->unsynchronize)
		return -ENOSYS;

	return p->ops->unsynchronize(p, from_p);
}
EXPORT_SYMBOL(pwm_unsynchronize);

static ssize_t pwm_run_show(struct device *dev,
			    struct device_attribute *attr,
			    char *buf)
{
	struct pwm_device *p = to_pwm_device(dev);
	return sprintf(buf, "%d\n", pwm_is_running(p));
}

static ssize_t pwm_run_store(struct device *dev,
			     struct device_attribute *attr,
			     const char *buf, size_t len)
{
	struct pwm_device *p = to_pwm_device(dev);

	if (!pwm_is_exported(p))
		return -EPERM;

	if (sysfs_streq(buf, "1"))
		pwm_start(p);
	else if (sysfs_streq(buf, "0"))
		pwm_stop(p);
	else
		return -EINVAL;

	return len;
}

static ssize_t pwm_tick_hz_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	struct pwm_device *p = to_pwm_device(dev);
	return sprintf(buf, "%lu\n", p->tick_hz);
}

static ssize_t pwm_duty_ns_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	struct pwm_device *p = to_pwm_device(dev);
	return sprintf(buf, "%lu\n", pwm_get_duty_ns(p));
}

static ssize_t pwm_duty_ns_store(struct device *dev,
				 struct device_attribute *attr,
				 const char *buf, size_t len)
{
	unsigned long duty_ns;
	struct pwm_device *p = to_pwm_device(dev);
	int ret;

	if (!pwm_is_exported(p))
		return -EPERM;

	ret = strict_strtoul(buf, 10, &duty_ns);
	if (ret)
		return ret;
	pwm_set_duty_ns(p, duty_ns);
	return len;
}

static ssize_t pwm_period_ns_show(struct device *dev,
				  struct device_attribute *attr,
				  char *buf)
{
	struct pwm_device *p = to_pwm_device(dev);
	return sprintf(buf, "%lu\n", pwm_get_period_ns(p));
}

static ssize_t pwm_period_ns_store(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t len)
{
	unsigned long period_ns;
	struct pwm_device *p = to_pwm_device(dev);
	int ret;

	if (!pwm_is_exported(p))
		return -EPERM;

	ret = strict_strtoul(buf, 10, &period_ns);
	if (ret)
		return ret;

	pwm_set_period_ns(p, period_ns);
	return len;
}

static ssize_t pwm_polarity_show(struct device *dev,
				 struct device_attribute *attr,
				 char *buf)
{
	struct pwm_device *p = to_pwm_device(dev);
	return sprintf(buf, "%d\n", p->polarity ? 1 : 0);
}

static ssize_t pwm_polarity_store(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t len)
{
	unsigned long polarity;
	struct pwm_device *p = to_pwm_device(dev);
	int ret;

	if (!pwm_is_exported(p))
		return -EPERM;

	ret = strict_strtoul(buf, 10, &polarity);
	if (ret)
		return ret;

	pwm_set_polarity(p, polarity);
	return len;
}

static ssize_t pwm_export_show(struct device *dev,
			       struct device_attribute *attr,
			       char *buf)
{
	struct pwm_device *p = to_pwm_device(dev);

	if (pwm_is_requested(p))
		return sprintf(buf, "%s\n", p->label);
	return 0;
}

static ssize_t pwm_export_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t len)
{
	struct pwm_device *p = to_pwm_device(dev);
	int ret;

	get_device(dev);
	ret = __pwm_request(p, REQUEST_SYSFS);

	if (!ret)
		set_bit(PWM_FLAG_EXPORTED, &p->flags);
	else {
		put_device(dev);
		ret = -EBUSY;
	}

	return ret ? ret : len;
}

static ssize_t pwm_unexport_store(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t len)
{
	struct pwm_device *p = to_pwm_device(dev);

	if (!pwm_is_exported(p))
		return -EINVAL;

	pwm_release(p);
	clear_bit(PWM_FLAG_EXPORTED, &p->flags);
	return len;
}

static struct device_attribute pwm_dev_attrs[] = {
	__ATTR(export, S_IRUGO | S_IWUSR, pwm_export_show, pwm_export_store),
	__ATTR(unexport, S_IWUSR, NULL, pwm_unexport_store),
	__ATTR(polarity, S_IRUGO | S_IWUSR, pwm_polarity_show, pwm_polarity_store),
	__ATTR(period_ns, S_IRUGO | S_IWUSR, pwm_period_ns_show, pwm_period_ns_store),
	__ATTR(duty_ns, S_IRUGO | S_IWUSR, pwm_duty_ns_show, pwm_duty_ns_store),
	__ATTR(tick_hz, S_IRUGO, pwm_tick_hz_show, NULL),
	__ATTR(run, S_IRUGO | S_IWUSR, pwm_run_show, pwm_run_store),
	__ATTR_NULL,
};

static struct class pwm_class = {
	.name		= "pwm",
	.owner		= THIS_MODULE,
	.dev_attrs	= pwm_dev_attrs,
};

static void __pwm_release(struct device *dev)
{
	struct pwm_device *p = container_of(dev, struct pwm_device, dev);
	kfree(p);
}

/**
 * pwm_register - registers a PWM device
 *
 * @ops: PWM device operations
 * @parent: reference to parent device, if any
 * @fmt: printf-style format specifier for device name
 */
struct pwm_device *pwm_register(const struct pwm_device_ops *ops,
				struct device *parent, const char *fmt, ...)
{
	struct pwm_device *p;
	int ret;
	va_list vargs;

	if (!ops || !ops->config)
		return ERR_PTR(-EINVAL);

	p = kzalloc(sizeof(*p), GFP_KERNEL);
	if (!p)
		return ERR_PTR(-ENOMEM);

	p->ops = ops;

	p->dev.class = &pwm_class;
	p->dev.parent = parent;
	p->dev.release = __pwm_release;

	va_start(vargs, fmt);
	ret = kobject_set_name_vargs(&p->dev.kobj, fmt, vargs);

	ret = device_register(&p->dev);
	if (ret)
		goto err;

	return p;

err:
	put_device(&p->dev);
	return ERR_PTR(ret);
}
EXPORT_SYMBOL(pwm_register);

void pwm_unregister(struct pwm_device *p)
{
	device_unregister(&p->dev);
}
EXPORT_SYMBOL(pwm_unregister);

static int __init pwm_init(void)
{
	return class_register(&pwm_class);
}

static void __exit pwm_exit(void)
{
	class_unregister(&pwm_class);
}

postcore_initcall(pwm_init);
module_exit(pwm_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Bill Gatliff <bgat@billgatliff.com>");
MODULE_DESCRIPTION("Generic PWM device API implementation");
