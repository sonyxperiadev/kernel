/*
 * Copyright (C) 2011 Bill Gatliff <bgat@billgatliff.com>
 * Copyright (C) 2011 Arun Murthy <arun.murth@stericsson.com>
 *
 * This program is free software; you may redistribute and/or modify
 * it under the terms of the GNU General Public License version 2, as
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
#ifndef __LINUX_PWM_H
#define __LINUX_PWM_H

#include <linux/device.h>

enum {
	PWM_FLAG_REQUESTED	= 0,
	PWM_FLAG_STOP		= 1,
	PWM_FLAG_RUNNING	= 2,
	PWM_FLAG_EXPORTED	= 3,
};

enum {
	PWM_CONFIG_DUTY_TICKS	= 0,
	PWM_CONFIG_PERIOD_TICKS	= 1,
	PWM_CONFIG_POLARITY	= 2,
	PWM_CONFIG_START	= 3,
	PWM_CONFIG_STOP		= 4,
};

struct pwm_config;
struct pwm_device;

struct pwm_device_ops {
	struct module *owner;

	int	(*request)		(struct pwm_device *p);
	void	(*release)		(struct pwm_device *p);
	int	(*config)		(struct pwm_device *p,
					 struct pwm_config *c);
	int	(*config_nosleep)	(struct pwm_device *p,
					 struct pwm_config *c);
	int	(*synchronize)		(struct pwm_device *p,
					 struct pwm_device *to_p);
	int	(*unsynchronize)	(struct pwm_device *p,
					 struct pwm_device *from_p);
};

/**
 * struct pwm_config - configuration data for a PWM device
 *
 * @config_mask: which fields are valid
 * @duty_ticks: requested duty cycle, in ticks
 * @period_ticks: requested period, in ticks
 * @polarity: active high (1), or active low (0)
 */
struct pwm_config {
	unsigned long	config_mask;
	unsigned long	duty_ticks;
	unsigned long	period_ticks;
	int		polarity;
};

/**
 * struct pwm_device - represents a PWM device
 *
 * @dev: device model reference
 * @ops: operations supported by the PWM device
 * @label: requestor of the PWM device, or NULL
 * @flags: PWM device state, see FLAG_*
 * @tick_hz: base tick rate of PWM device, in HZ
 * @polarity: active high (1), or active low (0)
 * @period_ticks: PWM device's current period, in ticks
 * @duty_ticks: duration of PWM device's active cycle, in ticks
 */
struct pwm_device {
	struct device	dev;
	const struct pwm_device_ops *ops;
	const char	*label;
	unsigned long	flags;
	unsigned long	tick_hz;
	int		polarity;
	unsigned long	period_ticks;
	unsigned long	duty_ticks;
};

void pwm_release(struct pwm_device *p);

static inline int pwm_is_requested(const struct pwm_device *p)
{
	return test_bit(PWM_FLAG_REQUESTED, &p->flags);
}

static inline int pwm_is_running(const struct pwm_device *p)
{
	return test_bit(PWM_FLAG_RUNNING, &p->flags);
}

static inline int pwm_is_exported(const struct pwm_device *p)
{
	return test_bit(PWM_FLAG_EXPORTED, &p->flags);
}

struct pwm_device *pwm_register(const struct pwm_device_ops *ops, struct device *parent,
				const char *fmt, ...);
void pwm_unregister(struct pwm_device *p);

void pwm_set_drvdata(struct pwm_device *p, void *data);
void *pwm_get_drvdata(const struct pwm_device *p);

int pwm_set(struct pwm_device *p, unsigned long period_ns,
	    unsigned long duty_ns, int polarity);

int pwm_set_period_ns(struct pwm_device *p, unsigned long period_ns);
unsigned long pwm_get_period_ns(struct pwm_device *p);

int pwm_set_duty_ns(struct pwm_device *p, unsigned long duty_ns);
unsigned long pwm_get_duty_ns(struct pwm_device *p);

int pwm_set_polarity(struct pwm_device *p, int polarity);

int pwm_start(struct pwm_device *p);
int pwm_stop(struct pwm_device *p);

int pwm_config_nosleep(struct pwm_device *p, struct pwm_config *c);

int pwm_synchronize(struct pwm_device *p, struct pwm_device *to_p);
int pwm_unsynchronize(struct pwm_device *p, struct pwm_device *from_p);

struct pwm_device *gpio_pwm_create(int gpio);
int gpio_pwm_destroy(struct pwm_device *p);

/*
 * pwm_request - request a PWM device
 */
struct pwm_device *pwm_request(int pwm_id, const char *label);

/*
 * pwm_free - free a PWM device
 */
void pwm_free(struct pwm_device *pwm);

/*
 * pwm_config - change a PWM device configuration
 */
int pwm_config(struct pwm_device *pwm, int duty_ns, int period_ns);

/*
 * pwm_enable - start a PWM output toggling
 */
int pwm_enable(struct pwm_device *pwm);

/*
 * pwm_disable - stop a PWM output toggling
 */
void pwm_disable(struct pwm_device *pwm);


#endif
