/* this definition is enabled if use alarm timer. */
/*
 * Copyright (C) 2018 Sony Mobile Communications Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation.
 */
/*#define USE_ALARM*/
/* this definition is enabled if use LCD linkage. */
#define USE_LCDSTATE

#include <linux/module.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/of_irq.h>
#include <linux/power_supply.h>
#ifdef USE_LCDSTATE
#include <linux/drm_notify.h>
#else
#ifdef CONFIG_PM_SLEEP
#include <linux/suspend.h>
#endif
#endif
#include <linux/drm_notify.h>
#include <linux/pm_wakeup.h>
#include <linux/spinlock.h>
#include <linux/alarmtimer.h>

#define WDET_CONNECT_CHECK_DELAY	300	/*ms*/
#define WDET_NEGOTIATION_CHECK_DELAY	2000	/*ms*/

#define WDET_WATER_CHECK_SHORT_TIMER	7	/*sec*/
#define WDET_WATER_CHECK_LONG_TIMER	0	/*sec*/

#define WDET_WAKELOCK_TIMEOUT		1000	/*ms*/

#define WDET_CABLE_TIMER_COUNT		7	/*times*/

enum wdet_det_state {
	WDET_NONE,
	WDET_OR,
	WDET_AND,
};

enum wdet_polling_state {
	WDET_POLLING_START,
	WDET_POLLING_STOP,
	WDET_POLLING_OR_DETECTED,
};

enum wdet_timer_kind {
	WDET_TIMER_SHORT,
	WDET_TIMER_LONG,
	WDET_TIMER_STOP,
};

struct wdet {
	struct device		*dev;
	struct device		*device;
	struct class		*class;
	struct power_supply	*usb_psy;
	enum wdet_polling_state	polling_state;
	enum wdet_polling_state	notify;

	spinlock_t		lock;

#ifdef	USE_ALARM
	struct alarm		*alarmtimer;
#else
	struct delayed_work	alarm_work;
#endif
	int			dvdt_enable_gpio;
	int			dvdt_wrt_det_or_gpio;
	int			dvdt_wrt_det_and_gpio;
	int			dvdt_wrt_det_irq;
	int			cable_timer;
	enum wdet_det_state	dvdt_det_state;

	bool			connected;

	struct workqueue_struct *dvdt_wq;
	struct work_struct	check_water_work;
	struct delayed_work	check_connect_work;
	struct delayed_work	check_negotiation_work;
	struct wakeup_source	check_lock;

	/* for debug use */
	bool			dvdt_polling_unstop;
	bool			dvdt_enable_force;

	struct mutex		wdet_mutex;

#ifdef USE_LCDSTATE
	bool			isblanked;
	struct notifier_block	drm_notify;
#else
#ifdef CONFIG_PM_SLEEP
	struct notifier_block	pm_notify;
#endif
#endif
	struct notifier_block	psy_nb;
};

void wdet_check_water(struct wdet *wdet);

static ssize_t wdet_polling_unstop_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct wdet *wdet = dev_get_drvdata(dev);

	return snprintf(buf, 8, "%d", wdet->dvdt_polling_unstop);
}

static ssize_t wdet_polling_unstop_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct wdet *wdet = dev_get_drvdata(dev);
	int	ret;
	bool	enable;
	union power_supply_propval val = {0};

	ret = strtobool(buf, &enable);
	if (!ret) {
		wdet->dvdt_polling_unstop = enable;
		if (enable) {
			val.intval = POWER_SUPPLY_TYPEC_PR_DUAL;
			ret = power_supply_set_property(wdet->usb_psy,
				POWER_SUPPLY_PROP_TYPEC_POWER_ROLE_FOR_WDET,
				&val);
			if (ret)
				dev_err(wdet->dev,
					"%s:set powerrole fail(%d)\n",
								__func__, ret);
			else
				dev_dbg(wdet->dev,
					"%s: powerrole changes to DUAL\n",
								__func__);
		}
	}

	return count;
}
static DEVICE_ATTR_RW(wdet_polling_unstop);

static ssize_t wdet_enable_force_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct wdet *wdet = dev_get_drvdata(dev);

	return snprintf(buf, 8, "%d", wdet->dvdt_enable_force);
}

static ssize_t wdet_enable_force_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct wdet *wdet = dev_get_drvdata(dev);
	int	ret;
	bool	enable;

	ret = strtobool(buf, &enable);
	if (!ret)
		wdet->dvdt_enable_force = enable;

	return count;
}
static DEVICE_ATTR_RW(wdet_enable_force);

static ssize_t wdet_wrt_det_or_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct wdet *wdet = dev_get_drvdata(dev);

	if (gpio_get_value(wdet->dvdt_wrt_det_or_gpio))
		return snprintf(buf, 8, "1");
	else
		return snprintf(buf, 8, "0");
}
static DEVICE_ATTR_RO(wdet_wrt_det_or);

static ssize_t wdet_wrt_det_and_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct wdet *wdet = dev_get_drvdata(dev);

	if (gpio_get_value(wdet->dvdt_wrt_det_and_gpio))
		return snprintf(buf, 8, "1");
	else
		return snprintf(buf, 8, "0");
}
static DEVICE_ATTR_RO(wdet_wrt_det_and);

static ssize_t wdet_wrt_det_state_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct wdet *wdet = dev_get_drvdata(dev);

	switch (wdet->dvdt_det_state) {
	case WDET_OR:
		return snprintf(buf, 8, "OR");
	case WDET_AND:
		return snprintf(buf, 8, "AND");
	case WDET_NONE:
	default:
		return snprintf(buf, 8, "NONE");
	}
}
static DEVICE_ATTR_RO(wdet_wrt_det_state);

static ssize_t wdet_enable_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct wdet *wdet = dev_get_drvdata(dev);

	if (gpio_get_value(wdet->dvdt_enable_gpio))
		return snprintf(buf, 8, "1");
	else
		return snprintf(buf, 8, "0");
}

static ssize_t wdet_enable_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct wdet *wdet = dev_get_drvdata(dev);
	int	ret;
	bool	enable;
	union power_supply_propval val = {0};

	ret = strtobool(buf, &enable);
	if (!ret) {
		if (gpio_is_valid(wdet->dvdt_enable_gpio))
			gpio_direction_output(wdet->dvdt_enable_gpio, enable);

		if (enable) {
			val.intval = POWER_SUPPLY_TYPEC_PR_DUAL;
			ret = power_supply_set_property(wdet->usb_psy,
				POWER_SUPPLY_PROP_TYPEC_POWER_ROLE_FOR_WDET,
				&val);
			if (ret)
				dev_err(wdet->dev,
					"%s:set powerrole fail(%d)\n",
								__func__, ret);
			else
				dev_dbg(wdet->dev,
					"%s: powerrole changes to DUAL\n",
								__func__);
		}
	}

	return count;
}
static DEVICE_ATTR_RW(wdet_enable);

static ssize_t wdet_polling_restart_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct wdet *wdet = dev_get_drvdata(dev);

	wdet_check_water(wdet);
	return count;
}
static DEVICE_ATTR_WO(wdet_polling_restart);

static ssize_t wdet_wrt_det_cable_timer_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct wdet *wdet = dev_get_drvdata(dev);

	return snprintf(buf, 8, "%d", wdet->cable_timer);
}
static DEVICE_ATTR_RO(wdet_wrt_det_cable_timer);

static struct attribute *wdet_attrs[] = {
	&dev_attr_wdet_polling_unstop.attr,
	&dev_attr_wdet_enable_force.attr,
	&dev_attr_wdet_wrt_det_or.attr,
	&dev_attr_wdet_wrt_det_and.attr,
	&dev_attr_wdet_wrt_det_state.attr,
	&dev_attr_wdet_enable.attr,
	&dev_attr_wdet_polling_restart.attr,
	&dev_attr_wdet_wrt_det_cable_timer.attr,
	NULL,
};
ATTRIBUTE_GROUPS(wdet);

static void wdet_rtctimer_start(struct wdet *wdet, int sec)
{
	if (!sec)
		return;

#ifdef USE_ALARM
	ktime_t next_alarm;

	next_alarm = ktime_set(sec, 0);
	alarm_start_relative(wdet->alarmtimer, next_alarm);
#else
	queue_delayed_work(wdet->dvdt_wq, &wdet->alarm_work,
			msecs_to_jiffies(sec*1000));
#endif
}

static void wdet_rtctimer_stop(struct wdet *wdet)
{
#ifdef USE_ALARM
	alarm_cancel(wdet->alarmtimer);
#else
	cancel_delayed_work(&wdet->alarm_work);
#endif

}

static void wdet_set_timer(struct wdet *wdet, enum wdet_timer_kind timer_kind)
{
	switch (timer_kind) {
	case WDET_TIMER_SHORT:
		if (!wdet->isblanked)
			wdet_rtctimer_start(wdet,
					WDET_WATER_CHECK_SHORT_TIMER);
		break;
	case WDET_TIMER_LONG:
		if (!wdet->isblanked)
			wdet_rtctimer_start(wdet,
					WDET_WATER_CHECK_LONG_TIMER);
		break;
	case WDET_TIMER_STOP:
		wdet_rtctimer_stop(wdet);
		break;
	}
}

static int wdet_send_polling_uevent(struct wdet *wdet,
					enum wdet_polling_state notify)
{
	int ret;
	char udev_event[32];
	char *envp[] = {udev_event, NULL};

	wdet->notify = notify;
	if (notify == WDET_POLLING_START)
		snprintf(udev_event, sizeof(udev_event),
				"EVENT=USB_POLLING_STARTED");
	else if (notify == WDET_POLLING_OR_DETECTED)
		snprintf(udev_event, sizeof(udev_event),
				"EVENT=USB_WATER_OR_DETECTED");
	else
		snprintf(udev_event, sizeof(udev_event),
				"EVENT=USB_POLLING_STOPPED");
	ret = kobject_uevent_env(&wdet->device->kobj, KOBJ_CHANGE, envp);
	if (ret)
		dev_err(wdet->dev, "%s:uevent send error\n", __func__);
	return ret;
}

static void wdet_check_water_work(struct work_struct *w)
{
	struct wdet *wdet = container_of(w, struct wdet, check_water_work);
	union power_supply_propval val = {0};
	unsigned long flags;
	int ret, or, and;

	dev_info(wdet->dev, "%s:\n", __func__);

	mutex_lock(&wdet->wdet_mutex);

	/* check connecting */
	if (wdet->polling_state == WDET_POLLING_STOP) {
		dev_dbg(wdet->dev, "%s: USB is connected\n", __func__);
		goto exit_work;
	}

	if (wdet->isblanked == true)
		goto exit_work;

	disable_irq_nosync(wdet->dvdt_wrt_det_irq);

	val.intval = POWER_SUPPLY_TYPEC_PR_DUAL;
	if (power_supply_set_property(wdet->usb_psy,
			POWER_SUPPLY_PROP_TYPEC_POWER_ROLE_FOR_WDET, &val)) {
		dev_err(wdet->dev, "%s:set powerrole fail\n", __func__);
		goto exit_work;
	}
	dev_dbg(wdet->dev, "%s: powerrole changes to DUAL\n", __func__);

	ret = gpio_direction_output(wdet->dvdt_enable_gpio, 1);
	if (ret)
		dev_err(wdet->dev, "%s: cannot change gpio direction (%d)\n",
				__func__, ret);
	dev_dbg(wdet->dev, "%s: enable dvdt interrupt\n", __func__);

	msleep(100);

	or = gpio_get_value(wdet->dvdt_wrt_det_or_gpio);
	and = gpio_get_value(wdet->dvdt_wrt_det_and_gpio);
	if (or < 0 || and < 0) {
		/* will be changed to DRY, if gpio read error */
		dev_err(wdet->dev,
			"%s: can't get wrt_det_and_gpio.(or=%d and=%d)\n",
							__func__, or, and);
		wdet->cable_timer = 0;
		wdet_send_polling_uevent(wdet, WDET_POLLING_START);
		spin_lock_irqsave(&wdet->lock, flags);
		wdet->dvdt_det_state = WDET_NONE;
		spin_unlock_irqrestore(&wdet->lock, flags);

		dev_info(wdet->dev, "%s: water pin state = DRY\n",
				__func__);
		enable_irq(wdet->dvdt_wrt_det_irq);
		goto exit_work;
	}

	if (wdet->cable_timer == 0) {
		if (or) {
			dev_dbg(wdet->dev, "%s: water is detecting yet\n",
					__func__);
		} else {
			wdet_send_polling_uevent(wdet, WDET_POLLING_START);
			spin_lock_irqsave(&wdet->lock, flags);
			wdet->dvdt_det_state = WDET_NONE;
			spin_unlock_irqrestore(&wdet->lock, flags);

			dev_info(wdet->dev, "%s: water pin state = DRY\n",
					__func__);
		}

	} else if (wdet->cable_timer < WDET_CABLE_TIMER_COUNT) {
		if (!or) {
			/* Process as dry if dry state or read error */
			__pm_wakeup_event(&wdet->check_lock,
							WDET_WAKELOCK_TIMEOUT);
			queue_delayed_work(wdet->dvdt_wq,
				&wdet->check_connect_work,
				msecs_to_jiffies(WDET_CONNECT_CHECK_DELAY));
			ret = gpio_direction_output(wdet->dvdt_enable_gpio, 0);
			if (ret)
				dev_err(wdet->dev,
				"%s: cannot change gpio direction (%d)\n",
								__func__, ret);

		} else if (and) {
			/* do check changing to AND */
			spin_lock_irqsave(&wdet->lock, flags);
			wdet->dvdt_det_state = WDET_AND;
			spin_unlock_irqrestore(&wdet->lock, flags);
			dev_info(wdet->dev, "%s: detect AND in cable check\n",
								__func__);
		}

		dev_dbg(wdet->dev, "%s: cable_timer=%d\n", __func__,
							wdet->cable_timer);
	}
	enable_irq(wdet->dvdt_wrt_det_irq);

exit_work:
	mutex_unlock(&wdet->wdet_mutex);
	__pm_relax(&wdet->check_lock);
}

static void wdet_check_connect_work(struct work_struct *w)
{
	struct wdet *wdet = container_of(w, struct wdet,
						check_connect_work.work);
	union power_supply_propval val = {0};
	int ret;

	dev_dbg(wdet->dev, "%s: polling_state:%d det:%d\n", __func__,
			wdet->polling_state,
			wdet->dvdt_det_state);

	mutex_lock(&wdet->wdet_mutex);

	/* check connecting */
	if (wdet->polling_state == WDET_POLLING_STOP) {
		dev_dbg(wdet->dev, "%s: USB is connected\n", __func__);
		mutex_unlock(&wdet->wdet_mutex);
		return;
	}

	if (wdet->dvdt_det_state == WDET_NONE) {
		ret = gpio_direction_output(wdet->dvdt_enable_gpio, 1);
		if (ret)
			dev_err(wdet->dev,
				"%s: cannot change gpio direction (%d)\n",
								__func__, ret);
		dev_info(wdet->dev, "%s: already dried\n", __func__);
		mutex_unlock(&wdet->wdet_mutex);
		return;
	}

	/* USB is not connected, it is detected water. */
	if (!wdet->dvdt_polling_unstop) {
		val.intval = POWER_SUPPLY_TYPEC_PR_SINK;
		if (power_supply_set_property(wdet->usb_psy,
				POWER_SUPPLY_PROP_TYPEC_POWER_ROLE_FOR_WDET,
				&val)) {
			dev_err(wdet->dev, "%s:set powerrole fail\n",
					__func__);
			mutex_unlock(&wdet->wdet_mutex);
			return;
		}
		dev_dbg(wdet->dev, "%s: powerrole changes to SINK\n",
								__func__);
	}

	if (wdet->dvdt_det_state == WDET_OR) {
		wdet_send_polling_uevent(wdet, WDET_POLLING_OR_DETECTED);
		wdet->cable_timer++;
		if (wdet->cable_timer < WDET_CABLE_TIMER_COUNT)
			wdet_set_timer(wdet, WDET_TIMER_SHORT);
	} else if (wdet->dvdt_det_state == WDET_AND) {
		wdet_send_polling_uevent(wdet, WDET_POLLING_STOP);
		wdet_set_timer(wdet, WDET_TIMER_LONG);
	}

	msleep(50);
	ret = gpio_direction_output(wdet->dvdt_enable_gpio, 1);
	if (ret)
		dev_err(wdet->dev, "%s: cannot change gpio direction (%d)\n",
								__func__, ret);
	dev_dbg(wdet->dev, "%s: enable dvdt_enable\n", __func__);
	mutex_unlock(&wdet->wdet_mutex);

	dev_info(wdet->dev, "%s: wdet_state=%d, cable_timer=%d\n",
			__func__, wdet->dvdt_det_state, wdet->cable_timer);
	if (!power_supply_get_property(wdet->usb_psy,
			POWER_SUPPLY_PROP_TYPEC_POWER_ROLE_FOR_WDET, &val)) {
		dev_dbg(wdet->dev, "%s: powerrole change results:%d\n",
							__func__, val.intval);
	}
}

static void wdet_check_negotiation_work(struct work_struct *w)
{
	struct wdet *wdet = container_of(w, struct wdet,
						check_negotiation_work.work);
	int ret;

	dev_info(wdet->dev, "%s: polling_state=%d", __func__,
			wdet->polling_state);
	mutex_lock(&wdet->wdet_mutex);
	if (wdet->polling_state == WDET_POLLING_START) {
		wdet_send_polling_uevent(wdet, WDET_POLLING_START);
		ret = gpio_direction_output(wdet->dvdt_enable_gpio, 1);
		if (ret)
			dev_err(wdet->dev,
				"%s: cannot change gpio direction (%d)\n",
								__func__, ret);
		dev_dbg(wdet->dev, "%s: enable dvdt_enable\n", __func__);
		dev_info(wdet->dev, "%s: Polling start\n", __func__);
	} else if (!wdet->dvdt_enable_force) {
		dev_dbg(wdet->dev, "%s: USB is connected\n", __func__);
		mutex_unlock(&wdet->wdet_mutex);
		return;
	}
	mutex_unlock(&wdet->wdet_mutex);
	wdet_check_water(wdet);
}

#ifdef USE_ALARM
static enum alarmtimer_restart alarm_interrupt(struct alarm *alarm, ktime_t now)
{
	struct wdet *wdet = (struct wdet *)alarm->data;

	dev_dbg(wdet->dev, "%s:\n", __func__);
	__pm_wakeup_event(&wdet->check_lock, WDET_WAKELOCK_TIMEOUT);
	queue_work(wdet->dvdt_wq, &wdet->check_water_work);
	return ALARMTIMER_NORESTART;
}
#else
static void wdet_alarm_work(struct work_struct *w)
{
	struct wdet *wdet = container_of(w, struct wdet, alarm_work.work);

	__pm_wakeup_event(&wdet->check_lock, WDET_WAKELOCK_TIMEOUT);
	queue_work(wdet->dvdt_wq, &wdet->check_water_work);
}
#endif

static irqreturn_t wdet_dvdt_wrt_det_irq(int irq, void *data)
{
	struct wdet *wdet = data;
	unsigned long flags;
	int ret;

	dev_dbg(wdet->dev, "%s: dvdt interrupt happen state=%d\n", __func__,
							wdet->dvdt_det_state);

	__pm_wakeup_event(&wdet->check_lock, WDET_WAKELOCK_TIMEOUT);

	cancel_work(&wdet->check_water_work);
	cancel_delayed_work(&wdet->check_connect_work);

	ret = gpio_get_value(wdet->dvdt_wrt_det_and_gpio);
	if (ret < 0) {
		dev_err(wdet->dev,
			"%s: can't get wrt_det_and_gpio.(ret = %d)\n",
							__func__, ret);
		return IRQ_HANDLED;
	}
	dev_info(wdet->dev, "%s: water pin state = %s\n",
						__func__, ret ? "AND" : "OR");
	spin_lock_irqsave(&wdet->lock, flags);
	if (ret)
		wdet->dvdt_det_state = WDET_AND;
	else if (wdet->dvdt_det_state == WDET_AND)
		wdet->dvdt_det_state = WDET_AND;
	else
		wdet->dvdt_det_state = WDET_OR;
	spin_unlock_irqrestore(&wdet->lock, flags);

	/* for dvdt disable */
	if (!wdet->dvdt_enable_force) {
		ret = gpio_direction_output(wdet->dvdt_enable_gpio, 0);
		if (ret)
			dev_err(wdet->dev,
				"%s: cannot change gpio direction (%d)\n",
								__func__, ret);
		dev_dbg(wdet->dev, "%s: disable dvdt_enable\n", __func__);
	}

	queue_delayed_work(wdet->dvdt_wq, &wdet->check_connect_work,
			msecs_to_jiffies(WDET_CONNECT_CHECK_DELAY));
	return IRQ_HANDLED;
}

void wdet_check_water(struct wdet *wdet)
{
	dev_info(wdet->dev, "%s: check water for polling restart\n", __func__);
	__pm_wakeup_event(&wdet->check_lock, WDET_WAKELOCK_TIMEOUT);
	wdet_set_timer(wdet, WDET_TIMER_STOP);
	wdet->cable_timer = 0;
	queue_work(wdet->dvdt_wq, &wdet->check_water_work);
}
EXPORT_SYMBOL(wdet_check_water);

int wdet_polling_set(struct wdet *wdet, enum wdet_polling_state state)
{
	unsigned long flags;
	int ret = 0;

	if (!wdet)
		return -EINVAL;

	if (!gpio_is_valid(wdet->dvdt_enable_gpio)) {
		dev_err(wdet->dev, "%s: dvdt_enable_gpio is invalid\n",
								__func__);
		return -EINVAL;
	}

	dev_info(wdet->dev, "%s: state %d -> %d\n", __func__,
					wdet->polling_state, state);
	spin_lock_irqsave(&wdet->lock, flags);
	if (wdet->polling_state == state)
		goto notchange;
	wdet->polling_state = state;
	wdet->dvdt_det_state = WDET_NONE;
	spin_unlock_irqrestore(&wdet->lock, flags);

	wdet_set_timer(wdet, WDET_TIMER_STOP);

	/* in negotiation mode */
	if (work_busy(&wdet->check_negotiation_work.work))
		goto busy;

	mutex_lock(&wdet->wdet_mutex);
	if (state == WDET_POLLING_START) {
		wdet_send_polling_uevent(wdet, WDET_POLLING_START);
		ret = gpio_direction_output(wdet->dvdt_enable_gpio, 1);
		dev_dbg(wdet->dev, "%s: enable dvdt_enable\n", __func__);
		dev_info(wdet->dev, "%s: Polling start\n", __func__);
	} else if (!wdet->dvdt_enable_force) {
		wdet->cable_timer = 0;
		ret = gpio_direction_output(wdet->dvdt_enable_gpio, 0);
		dev_dbg(wdet->dev, "%s: disable dvdt_enable\n", __func__);
		dev_info(wdet->dev, "%s: Polling stop\n", __func__);
	}
	if (ret)
		dev_err(wdet->dev, "%s: cannot change gpio direction (%d)\n",
								__func__, ret);
	mutex_unlock(&wdet->wdet_mutex);
	return 0;
notchange:
	spin_unlock_irqrestore(&wdet->lock, flags);
busy:
	return 0;
}
EXPORT_SYMBOL(wdet_polling_set);

int wdet_invalid_during_negotiation(struct wdet *wdet)
{
	int ret;

	if (!wdet)
		return -EINVAL;

	dev_info(wdet->dev, "%s:\n", __func__);

	/* for dvdt disable */
	if (!wdet->dvdt_enable_force) {
		ret = gpio_direction_output(wdet->dvdt_enable_gpio, 0);
		if (ret)
			dev_err(wdet->dev,
				"%s: cannot change gpio direction (%d)\n",
								__func__, ret);
		dev_dbg(wdet->dev, "%s: disable dvdt_enable\n", __func__);
	}
	cancel_delayed_work(&wdet->check_negotiation_work);
	queue_delayed_work(wdet->dvdt_wq, &wdet->check_negotiation_work,
			msecs_to_jiffies(WDET_NEGOTIATION_CHECK_DELAY));
	return 0;
}
EXPORT_SYMBOL(wdet_invalid_during_negotiation);

#ifdef USE_LCDSTATE
static int wdet_drm_notify(struct notifier_block *notify_block,
				unsigned long event, void *data)
{
	struct wdet *wdet = container_of(notify_block, struct wdet, drm_notify);
	struct drm_ext_event *ev = (struct drm_ext_event *)data;
	int *blank = (int *)ev->data;

	switch (*blank) {
	/* LCD OFF */
	case DRM_BLANK_POWERDOWN:
		if (event == DRM_EXT_EVENT_AFTER_BLANK) {
			mutex_lock(&wdet->wdet_mutex);
			wdet->isblanked = true;
			wdet_set_timer(wdet, WDET_TIMER_STOP);
			cancel_work(&wdet->check_water_work);
			mutex_unlock(&wdet->wdet_mutex);
		}
		dev_dbg(wdet->dev, "%s: wdet blank (%ld)\n", __func__, event);
		break;
	/* LCD ON */
	case DRM_BLANK_UNBLANK:
		if (event == DRM_EXT_EVENT_BEFORE_BLANK) {
			mutex_lock(&wdet->wdet_mutex);
			wdet->isblanked = false;
			wdet->cable_timer = 0;
			if (wdet->dvdt_det_state != WDET_NONE)
				queue_work(wdet->dvdt_wq,
						&wdet->check_water_work);
			mutex_unlock(&wdet->wdet_mutex);
		}
		dev_dbg(wdet->dev, "%s: wdet unblank (%ld)\n", __func__, event);
		break;
	}
	return 0;
}
#else
#ifdef CONFIG_PM_SLEEP
static int wdet_pm_notify(struct notifier_block *notify_block,
					unsigned long mode, void *unused)
{
	struct wdet *wdet = container_of(notify_block, struct wdet, pm_notify);

	dev_dbg(wdet->dev, "%s: may_wakeup=%d\n", __func__,
					device_may_wakeup(wdet->dev));
	switch (mode) {
	/* suspend */
	case PM_HIBERNATION_PREPARE:
	case PM_SUSPEND_PREPARE:
		cancel_work(&wdet->check_water_work);
		wdet_set_timer(wdet, WDET_TIMER_STOP);
		dev_dbg(wdet->dev, "%s: wdet suspend\n", __func__);
		break;
	/* resume */
	case PM_POST_HIBERNATION:
	case PM_POST_SUSPEND:
		wdet->cable_timer = 0;
		if (wdet->dvdt_det_state != WDET_NONE)
			queue_work(wdet->dvdt_wq, &wdet->check_water_work);
		dev_dbg(wdet->dev, "%s: wdet resume\n", __func__);
		break;
	}
	return NOTIFY_DONE;
}
#endif
#endif

static int wdet_psy_changed(struct notifier_block *notify_block,
				unsigned long event, void *data)
{
	struct wdet *wdet = container_of(notify_block, struct wdet, psy_nb);
	union power_supply_propval val;
	int connected, ret;

	if (data != wdet->usb_psy || event != PSY_EVENT_PROP_CHANGED)
		return 0;

	ret = power_supply_get_property(wdet->usb_psy,
			POWER_SUPPLY_PROP_TYPEC_MODE, &val);
	if (ret) {
		dev_err(wdet->dev, "Unable to read USB TYPEC_MODE: %d\n", ret);
		return ret;
	}

	switch (val.intval) {
	/* Acting as source */
	case POWER_SUPPLY_TYPEC_SINK:
	case POWER_SUPPLY_TYPEC_SINK_POWERED_CABLE:
	case POWER_SUPPLY_TYPEC_SINK_DEBUG_ACCESSORY:
	case POWER_SUPPLY_TYPEC_SINK_AUDIO_ADAPTER:
	case POWER_SUPPLY_TYPEC_POWERED_CABLE_ONLY:
		/* fall through */
	/* Acting as sink */
	case POWER_SUPPLY_TYPEC_SOURCE_DEFAULT:
	case POWER_SUPPLY_TYPEC_SOURCE_MEDIUM:
	case POWER_SUPPLY_TYPEC_SOURCE_HIGH:
	/* non compliant */
	case POWER_SUPPLY_TYPEC_NON_COMPLIANT:
		connected = true;
		break;
	case POWER_SUPPLY_TYPEC_NONE:
	default:
		connected = false;
		break;
	};

	if (wdet->connected == connected)
		return 0;
	dev_info(wdet->dev, "Connection status is changed to %d\n", connected);
	wdet->connected = connected;

	if (wdet->connected) {
		wdet_polling_set(wdet, WDET_POLLING_STOP);
	} else {
		wdet_invalid_during_negotiation(wdet);
		wdet_polling_set(wdet, WDET_POLLING_START);
	}
	return 0;
}

static int wdet_probe(struct platform_device *pdev)
{
	int ret;
	struct wdet *wdet;
	struct power_supply *usb_psy;

	usb_psy = power_supply_get_by_name("usb");
	if (!usb_psy) {
		pr_err("%s: Could not get USB power_supply, deferring probe\n",
				__func__);
		return -EPROBE_DEFER;
	}

	wdet = devm_kzalloc(&pdev->dev, sizeof(*wdet), GFP_KERNEL);
	if (!wdet)
		return -ENOMEM;

	platform_set_drvdata(pdev, wdet);
	wdet->dev = &pdev->dev;


	wdet->class = class_create(THIS_MODULE, "water_detection");
	if (IS_ERR(wdet->class)) {
		ret = PTR_ERR(wdet->class);
		devm_kfree(&pdev->dev, wdet);
		return -ENODEV;
	}
	wdet->class->dev_groups = wdet_groups;

	wdet->device = device_create(wdet->class, NULL, MKDEV(0, 0),
							NULL, "wdet");
	if (IS_ERR(wdet->device)) {
		pr_err("%s: Could not create device\n", __func__);
		ret = -ENODEV;
		goto del_class;
	}
	dev_set_drvdata(wdet->device, wdet);

	wdet->usb_psy = usb_psy;
	spin_lock_init(&wdet->lock);
	wakeup_source_init(&wdet->check_lock, "wdet_wakelock");
	mutex_init(&wdet->wdet_mutex);

	wdet->dvdt_wq = alloc_ordered_workqueue("dvdt_wq", 0);
	if (!wdet->dvdt_wq) {
		pr_err("%s: Unable to create workqueue dvdt_wq\n", __func__);
		ret = -ENOMEM;
		goto del_dev;
	}
	INIT_WORK(&wdet->check_water_work, wdet_check_water_work);
	INIT_DELAYED_WORK(&wdet->check_connect_work, wdet_check_connect_work);
	INIT_DELAYED_WORK(&wdet->check_negotiation_work,
						wdet_check_negotiation_work);

	wdet->dvdt_enable_force = false;
	wdet->dvdt_wrt_det_and_gpio = of_get_named_gpio(
			pdev->dev.of_node,
			"dvdt_wrt_det_and", 0);
	if (!gpio_is_valid(wdet->dvdt_wrt_det_and_gpio)) {
		dev_err(wdet->dev, "dvdt_wrt_det_irq is not found\n");
		ret = -EINVAL;
		goto del_dvdt;
	}
	wdet->dvdt_wrt_det_or_gpio = of_get_named_gpio(
			pdev->dev.of_node,
			"dvdt_wrt_det_or", 0);
	if (!gpio_is_valid(wdet->dvdt_wrt_det_or_gpio)) {
		dev_err(wdet->dev, "dvdt_wrt_det_irq is not found\n");
		ret = -EINVAL;
		goto del_dvdt;
	}

	wdet->dvdt_wrt_det_irq = gpio_to_irq(wdet->dvdt_wrt_det_or_gpio);
	if (!gpio_is_valid(wdet->dvdt_wrt_det_irq)) {
		dev_err(wdet->dev,
				"not change gpio to irq: dvdt_wrt_det_irq\n");
		ret = -EINVAL;
		goto del_dvdt;
	}

	ret = devm_request_irq(wdet->dev, wdet->dvdt_wrt_det_irq,
			wdet_dvdt_wrt_det_irq,
			IRQF_TRIGGER_RISING | IRQF_ONESHOT,
			"dvdt_wrt_det", wdet);
	if (ret) {
		dev_err(wdet->dev, "irqreq dvdt_wrt_det_irq failed: %d\n",
				ret);
		goto del_dvdt;
	}
	enable_irq_wake(wdet->dvdt_wrt_det_irq);

	wdet->dvdt_enable_gpio = of_get_named_gpio(
			pdev->dev.of_node,
			"dvdt_enable_gpio", 0);
	if (!gpio_is_valid(wdet->dvdt_enable_gpio)) {
		dev_err(wdet->dev, "dvdt_enable_gpio is not found\n");
		devm_free_irq(wdet->dev, wdet->dvdt_wrt_det_irq, wdet);
		ret = -EINVAL;
		goto del_dvdt;
	}
	ret = gpio_direction_output(wdet->dvdt_enable_gpio, 1);
	if (ret)
		dev_err(wdet->dev, "%s: cannot change gpio direction (%d)\n",
								__func__, ret);

	wdet->psy_nb.notifier_call = wdet_psy_changed;
	ret = power_supply_reg_notifier(&wdet->psy_nb);
	if (ret)
		goto del_dvdt;

#ifdef	USE_ALARM
	/* initial real time clock timer */
	wdet->alarmtimer = kzalloc(sizeof(struct alarm), GFP_KERNEL);
	if (!wdet->alarmtimer)
		goto del_dvdt;
	wdet->alarmtimer->data = wdet;
	alarm_init(wdet->alarmtimer, ALARM_REALTIME, alarm_interrupt);
#else
	INIT_DELAYED_WORK(&wdet->alarm_work, wdet_alarm_work);
#endif

#ifdef USE_LCDSTATE
	wdet->drm_notify.notifier_call = wdet_drm_notify;
	ret = drm_register_client(&wdet->drm_notify);
#else
#ifdef CONFIG_PM_SLEEP
	wdet->pm_notify.notifier_call = wdet_pm_notify;
	ret = register_pm_notifier(&wdet->pm_notify);
#endif
#endif

	return 0;

del_dvdt:
	flush_workqueue(wdet->dvdt_wq);
	destroy_workqueue(wdet->dvdt_wq);
del_dev:
	mutex_destroy(&wdet->wdet_mutex);
	wakeup_source_trash(&wdet->check_lock);
del_class:
	class_destroy(wdet->class);
	devm_kfree(&pdev->dev, wdet);
	return ret;
}

static int wdet_remove(struct platform_device *pdev)
{
	struct wdet *wdet = platform_get_drvdata(pdev);

#ifdef USE_ALARM
	kfree(wdet->alarmtimer);
#endif
#ifdef USE_LCDSTATE
	drm_unregister_client(&wdet->drm_notify);
#else
#ifdef CONFIG_PM_SLEEP
	unregister_pm_notifier(&wdet->pm_notify);
#endif
#endif
	disable_irq(wdet->dvdt_wrt_det_irq);
	wdet_set_timer(wdet, WDET_TIMER_STOP);
	flush_workqueue(wdet->dvdt_wq);
	destroy_workqueue(wdet->dvdt_wq);
	mutex_destroy(&wdet->wdet_mutex);
	wakeup_source_trash(&wdet->check_lock);
	class_destroy(wdet->class);
	devm_kfree(&pdev->dev, wdet);
	return 0;
}

static const struct of_device_id wdet_match_table[] = {
	{
		.compatible	 = "somc,water-detection",
	},
	{ },
};

static struct platform_driver wdet_driver = {
	.probe		= wdet_probe,
	.remove		= wdet_remove,
	.driver		= {
		.name	= "water_detection",
		.of_match_table	= wdet_match_table,
	},
};

module_platform_driver(wdet_driver);

MODULE_DESCRIPTION("USB water detection");
MODULE_LICENSE("GPL v2");
