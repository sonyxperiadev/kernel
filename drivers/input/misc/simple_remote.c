/* kernel/drivers/input/misc/simple_remote.c
 *
 * Copyright (C) 2011 Sony Ericsson Mobile Communications AB.
 *
 * Authors: Takashi Shiina <takashi.shiina@sonyericsson.com>
 *          Tadashi Kubo <tadashi.kubo@sonyericsson.com>
 *          Joachim Holst <joachim.holst@sonyericsson.com>
 *          Torbjorn Eklund <torbjorn.eklund@sonyericsson.com>
 *          Atsushi Iyogi <Atsushi.XA.Iyogi@sonyericsson.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/workqueue.h>
#include <linux/err.h>
#include <linux/switch.h>
#include <linux/input.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/mutex.h>
#include <linux/kernel.h>
#include <asm/atomic.h>
#include <linux/ctype.h>
#include <linux/timer.h>
#include <linux/kernel.h>
#include <linux/wakelock.h>

#include <linux/simple_remote.h>

#define SIMPLE_REMOTE_APPKEY BTN_3

#define DETECTION_DELAY 200
#define DETECTION_CYCLES 20
#define MAX_NODEV_CYCLES 10
#define MIN_NUM_SUPPORTED_HS_DET 3
#define MIN_NUM_HEADPHONE_DET 5
#define DEV_UNKNOWN_ADC_VAL 2000

#define DETECT_WORK_DELAY				\
	(jiffies + msecs_to_jiffies(DETECTION_DELAY))

static irqreturn_t simple_remote_button_irq_handler(int irq, void *dev_id);
static irqreturn_t simple_remote_detect_irq_handler(int irq, void *dev_id);

enum dev_state {
	NO_DEVICE = 0,
	DEVICE_HEADSET,
	DEVICE_HEADPHONE,
	DEVICE_UNSUPPORTED = 0xFE00,
	DEVICE_UNKNOWN = 0xFF00,
};

static int simple_remote_acc_min[] = {
	0,    /* DEVICE_HEADPHONE */
	200,  /* DEVICE_HEADSET */
};

static int simple_remote_acc_max[] = {
	200,  /* DEVICE_HEADPHONE */
	1599, /* DEVICE_HEADSET */
};

static int simple_remote_btn_min[] = {
	0,   /* BTN_0 */
	100, /* BTN_1 */
	280, /* BTN_2 */
	500, /* BTN_3 */
};

static int simple_remote_btn_max[] = {
	99,   /* BTN_0 */
	199,  /* BTN_1 */
	399,  /* BTN_2 */
	699,  /* BTN_3 */
};

static int simple_remote_btn_trig_level[] = {
	460, /* Default trig level */
};

static int simple_remote_trig_period_freq[] = {
	512, /* Default trig period frequency*/
};

static int simple_remote_trig_period_time[] = {
	16, /* Default trig period time*/
};

static int simple_remote_trig_hysteresis_freq[] = {
	512, /* Default trig hysteresis freq*/
};

static int simple_remote_trig_hysteresis_time[] = {
	16, /* Default trig hysteresis time*/
};


static ssize_t simple_remote_attrs_show_property(struct device *dev,
					  struct device_attribute *attr,
					  char *buf);

static ssize_t simple_remote_attrs_store_property(struct device *dev,
					   struct device_attribute *attr,
					   const char *buf,
					   size_t count);

#define SIMPLE_REMOTE_ATTR(_name) \
{\
	.attr = { .name = _name,\
			.mode = (S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH),\
			},\
	.show = simple_remote_attrs_show_property,\
	.store = simple_remote_attrs_store_property,\
}

#define SIMPLE_REMOTE_ACC_MIN_VALS_NAME "accessory_min_vals"
#define SIMPLE_REMOTE_ACC_MAX_VALS_NAME "accessory_max_vals"
#define SIMPLE_REMOTE_BTN_MIN_VALS_NAME "button_min_vals"
#define SIMPLE_REMOTE_BTN_MAX_VALS_NAME "button_max_vals"
#define SIMPLE_REMOTE_TRIG_LVL_NAME "btn_trig_level"
#define SIMPLE_REMOTE_TRIG_PRD_F_NAME "btn_trig_period_freq"
#define SIMPLE_REMOTE_TRIG_PRD_T_NAME "btn_trig_period_time"
#define SIMPLE_REMOTE_TRIG_HST_F_NAME "btn_trig_hyst_freq"
#define SIMPLE_REMOTE_TRIG_HST_T_NAME "btn_trig_hyst_time"
#define MAX_ATTRS_NAME_LEN 32

static struct device_attribute simple_remote_attrs[] = {
	SIMPLE_REMOTE_ATTR(SIMPLE_REMOTE_ACC_MIN_VALS_NAME),
	SIMPLE_REMOTE_ATTR(SIMPLE_REMOTE_ACC_MAX_VALS_NAME),
	SIMPLE_REMOTE_ATTR(SIMPLE_REMOTE_BTN_MIN_VALS_NAME),
	SIMPLE_REMOTE_ATTR(SIMPLE_REMOTE_BTN_MAX_VALS_NAME),
	SIMPLE_REMOTE_ATTR(SIMPLE_REMOTE_TRIG_LVL_NAME),
	SIMPLE_REMOTE_ATTR(SIMPLE_REMOTE_TRIG_PRD_F_NAME),
	SIMPLE_REMOTE_ATTR(SIMPLE_REMOTE_TRIG_PRD_T_NAME),
	SIMPLE_REMOTE_ATTR(SIMPLE_REMOTE_TRIG_HST_F_NAME),
	SIMPLE_REMOTE_ATTR(SIMPLE_REMOTE_TRIG_HST_T_NAME),
};

struct simple_remote_driver {
	struct switch_dev swdev;
	struct input_dev *indev;
	struct input_dev *indev_appkey;

	atomic_t detection_in_progress;
	atomic_t detect_cycle;

	u8 pressed_button;
	u8 nodetect_cycles;
	u8 num_supported_hs_det;
	u8 num_headphone_det;

	enum dev_state current_accessory_state;

	struct work_struct plug_det_work;
	struct work_struct btn_det_work;

	struct mutex simple_remote_mutex;

	struct timer_list plug_det_timer;

	enum dev_state new_accessory_state;

	int client_counter;
	struct device *dev;

	struct simple_remote_pf_interface *interface;

	struct wake_lock lock;
};


static int simple_remote_attrs_set_data_buffer(char *buf, const int *array,
					       int array_len)
{

	int j;
	int buf_pos = 0;

	for (j = 0; j < array_len; j++) {
		buf_pos += snprintf(&buf[buf_pos], PAGE_SIZE - buf_pos,
				"%d ", array[j]);

		if (buf_pos >= PAGE_SIZE) {
			pr_err("*** %s - Error! len (%d) > PAGE_SIZE\n",
			       __func__, buf_pos);
			return -E2BIG;
		}
	}

	buf[buf_pos - 1] = '\n';

	return buf_pos;
}


static ssize_t simple_remote_attrs_show_property(struct device *dev,
						 struct device_attribute *attr,
						 char *buf) {

	int retval = -EIO;
	int val = -1;

	struct simple_remote_driver *jack = dev_get_drvdata(dev);

	mutex_lock(&jack->simple_remote_mutex);

	if (!strncmp(SIMPLE_REMOTE_ACC_MIN_VALS_NAME, attr->attr.name,
		     MAX_ATTRS_NAME_LEN)) {
		retval = simple_remote_attrs_set_data_buffer(
			buf, simple_remote_acc_min,
			ARRAY_SIZE(simple_remote_acc_min));
		goto done;
	}

	if (!strncmp(SIMPLE_REMOTE_ACC_MAX_VALS_NAME, attr->attr.name,
		     MAX_ATTRS_NAME_LEN)) {
		retval = simple_remote_attrs_set_data_buffer(
			buf, simple_remote_acc_max,
			ARRAY_SIZE(simple_remote_acc_max));
		goto done;
	}

	if (!strncmp(SIMPLE_REMOTE_BTN_MIN_VALS_NAME, attr->attr.name,
		     MAX_ATTRS_NAME_LEN)) {
		retval = simple_remote_attrs_set_data_buffer(
			buf, simple_remote_btn_min,
			ARRAY_SIZE(simple_remote_btn_min));
		goto done;
	}

	if (!strncmp(SIMPLE_REMOTE_BTN_MAX_VALS_NAME, attr->attr.name,
		     MAX_ATTRS_NAME_LEN)) {
		retval = simple_remote_attrs_set_data_buffer(
			buf, simple_remote_btn_max,
			ARRAY_SIZE(simple_remote_btn_max));
		goto done;
	}

	if (!strncmp(SIMPLE_REMOTE_TRIG_LVL_NAME, attr->attr.name,
		     MAX_ATTRS_NAME_LEN))
		retval = jack->interface->get_trig_level(&val);
	else if (!strncmp(SIMPLE_REMOTE_TRIG_PRD_F_NAME, attr->attr.name,
			  MAX_ATTRS_NAME_LEN))
		retval = jack->interface->get_period_freq(&val);
	else if (!strncmp(SIMPLE_REMOTE_TRIG_PRD_T_NAME, attr->attr.name,
			  MAX_ATTRS_NAME_LEN))
		retval = jack->interface->get_period_time(&val);
	else if (!strncmp(SIMPLE_REMOTE_TRIG_HST_F_NAME, attr->attr.name,
			  MAX_ATTRS_NAME_LEN))
		retval = jack->interface->get_hysteresis_freq(&val);
	else if (!strncmp(SIMPLE_REMOTE_TRIG_HST_T_NAME, attr->attr.name,
			  MAX_ATTRS_NAME_LEN))
		retval = jack->interface->get_hysteresis_time(&val);

	if (!retval)
		retval = scnprintf(buf, PAGE_SIZE, "%d\n", val);

done:
	mutex_unlock(&jack->simple_remote_mutex);

	return retval;
}

static int simple_remote_attrs_update_data(const char *buf, size_t count,
					    int *array, int array_len)
{
	char tmp_buf[10];
	int i, stepper;
	int value_count = 0;
	long conversion = 0;
	char *q = (char *)buf;

	do {
		i = strcspn(q, " \t,\r\n");
		for (stepper = 0; stepper < i && stepper < sizeof(tmp_buf) - 1;
		     stepper++) {
			/* First char could be - sign.
			   Store it anyway */
			if (0 == stepper)
				tmp_buf[stepper] = q[stepper];
			else if (isdigit(q[stepper]))
				tmp_buf[stepper] = q[stepper];
			else
				return -EINVAL;
		}

		tmp_buf[stepper] = '\0';

		if (strict_strtol(tmp_buf, 10, &conversion))
			return -EINVAL;

		/* Making sure that we get values that are in range */
		if (conversion <= SHRT_MAX && conversion >= SHRT_MIN) {
			array[value_count] = (short)conversion;
		} else {
			pr_err("%s - Value out of range. Aborting"
				" change!\n", __func__);
			return -EINVAL;
		}

		value_count++;
		q += i+1;
		count -= (i + 1);
	} while (count > 0 && value_count < array_len);

	return 0;
}

static ssize_t simple_remote_attrs_store_property(struct device *dev,
						  struct device_attribute *attr,
						  const char *buf, size_t count)
{
	long val;
	int ret;

	struct simple_remote_driver *jack = dev_get_drvdata(dev);

	mutex_lock(&jack->simple_remote_mutex);

	if (!strncmp(SIMPLE_REMOTE_ACC_MIN_VALS_NAME, attr->attr.name,
		     MAX_ATTRS_NAME_LEN)) {
		ret = simple_remote_attrs_update_data(
			buf, count, simple_remote_acc_min,
			ARRAY_SIZE(simple_remote_acc_min));
	} else if (!strncmp(SIMPLE_REMOTE_ACC_MAX_VALS_NAME, attr->attr.name,
			    MAX_ATTRS_NAME_LEN)) {
		ret = simple_remote_attrs_update_data(
			buf, count, simple_remote_acc_max,
			ARRAY_SIZE(simple_remote_acc_max));
	} else if (!strncmp(SIMPLE_REMOTE_BTN_MIN_VALS_NAME, attr->attr.name,
			    MAX_ATTRS_NAME_LEN)) {
		ret = simple_remote_attrs_update_data(
			buf, count, simple_remote_btn_min,
			ARRAY_SIZE(simple_remote_btn_min));
	} else if (!strncmp(SIMPLE_REMOTE_BTN_MAX_VALS_NAME, attr->attr.name,
			    MAX_ATTRS_NAME_LEN)) {
		ret = simple_remote_attrs_update_data(
			buf, count, simple_remote_btn_max,
			ARRAY_SIZE(simple_remote_btn_max));
	} else {
		ret = strict_strtol(buf, 10, &val);

		if (ret) {
			dev_err(dev, "Error when parsing string: %s\n", buf);
			goto done;
		}

		if (!strncmp(SIMPLE_REMOTE_TRIG_LVL_NAME, attr->attr.name,
			     MAX_ATTRS_NAME_LEN))
			ret = jack->interface->set_trig_level(val);
		else if (!strncmp(SIMPLE_REMOTE_TRIG_PRD_F_NAME,
				  attr->attr.name, MAX_ATTRS_NAME_LEN))
			ret = jack->interface->set_period_freq(val);
		else if (!strncmp(SIMPLE_REMOTE_TRIG_PRD_T_NAME,
				  attr->attr.name, MAX_ATTRS_NAME_LEN))
			ret = jack->interface->set_period_time(val);
		else if (!strncmp(SIMPLE_REMOTE_TRIG_HST_F_NAME,
				  attr->attr.name, MAX_ATTRS_NAME_LEN))
			ret = jack->interface->set_hysteresis_freq(val);
		else if (!strncmp(SIMPLE_REMOTE_TRIG_HST_T_NAME,
				  attr->attr.name, MAX_ATTRS_NAME_LEN))
			ret = jack->interface->set_hysteresis_time(val);
	}

done:
	mutex_unlock(&jack->simple_remote_mutex);

	if (!ret)
		ret = count;

	return ret;
}

static int create_sysfs_interfaces(struct device *dev)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(simple_remote_attrs); i++)
		if (device_create_file(dev, simple_remote_attrs + i))
			goto error;

	return 0;
error:
	for (i = i - 1; i >= 0; i--)
		device_remove_file(dev, simple_remote_attrs + i);

	dev_err(dev, "Unable to create sysfs interfaces\n");

	return -EIO;
}


static void remove_sysfs_interfaces(struct device *dev)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(simple_remote_attrs); i++)
		device_remove_file(dev, simple_remote_attrs + i);
}


static u8 simple_remote_attrs_parse_button_value(
	struct simple_remote_driver *jack, unsigned int adc_value)
{
	int i;
	u8 button = BTN_9 - BTN_0;

	mutex_lock(&jack->simple_remote_mutex);

	for (i = 0; i < ARRAY_SIZE(simple_remote_btn_max); i++) {
		if (adc_value <= simple_remote_btn_max[i] &&
			adc_value >= simple_remote_btn_min[i]) {
			button = i;
			break;
		}
	}

	mutex_unlock(&jack->simple_remote_mutex);

	return button;
}


static enum dev_state simple_remote_attrs_parse_accessory_type(
	struct simple_remote_driver *jack, unsigned int adc_value,
	u8 gpio_state)
{
	if (gpio_state) {
		jack->nodetect_cycles++;
		dev_dbg(jack->dev, "NO_DEVICE\n");
		return NO_DEVICE;
	}

	jack->nodetect_cycles = 0;

	if (adc_value <= simple_remote_acc_max[0]
		   && adc_value >= simple_remote_acc_min[0]) {
		dev_dbg(jack->dev, "DEVICE_HEADPHONE\n");
		return DEVICE_HEADPHONE;
	} else if (adc_value <= simple_remote_acc_max[1]
		   && adc_value >= simple_remote_acc_min[1]) {
		dev_dbg(jack->dev, "DEVICE_HEADSET");
		return DEVICE_HEADSET;
	}

	return DEVICE_UNKNOWN;
}


static void report_button_id(struct simple_remote_driver *jack,
			     u8 pressed)
{
	unsigned int pressed_button;

	dev_dbg(jack->dev, "Reporting button event. Button = 0x%x, "
		"event = %d\n",	jack->pressed_button, pressed);

	pressed_button = BTN_0 + jack->pressed_button;

	/* Report all button presses to both input devices. The
	   input device will filter the button press, and only
	   let through registered buttons. */
	input_report_key(jack->indev, pressed_button, pressed);
	input_sync(jack->indev);

	input_report_key(jack->indev_appkey, pressed_button, pressed);
	input_sync(jack->indev_appkey);
}


static ssize_t simple_remote_print_name(struct switch_dev *sdev, char *buf)
{
	struct simple_remote_driver *jack =
		container_of(sdev, struct simple_remote_driver, swdev);

	switch (switch_get_state(&jack->swdev)) {
	case DEVICE_UNKNOWN:
		dev_info(jack->dev, "Device Unknown\n");
		return snprintf(buf, PAGE_SIZE, "Device Unknown\n");
	case DEVICE_UNSUPPORTED:
		dev_info(jack->dev, "Device unsupported\n");
		return snprintf(buf, PAGE_SIZE, "Unsupported\n");
	case NO_DEVICE:
		dev_info(jack->dev, "No Device\n");
		return snprintf(buf, PAGE_SIZE, "No Device\n");
	case DEVICE_HEADSET:
		dev_info(jack->dev, "Headset\n");
		return snprintf(buf, PAGE_SIZE, "Headset\n");
	case DEVICE_HEADPHONE:
		dev_info(jack->dev, "Headphone\n");
		return snprintf(buf, PAGE_SIZE, "Headphone\n");
	}
	return -EINVAL;
}


static void simple_remote_report_accessory_type(
	struct simple_remote_driver *jack)
{

	dev_vdbg(jack->dev, "current_accessory_state = %d, new_accessory_state "
		 "= %d\n", jack->current_accessory_state,
		 jack->new_accessory_state);


	if (jack->current_accessory_state != jack->new_accessory_state) {
		if (DEVICE_HEADSET == jack->current_accessory_state) {
			jack->interface->enable_mic_bias(0);
			report_button_id(jack, 0);
			jack->interface->unregister_hssd_button_interrupt(jack);
		}

		if (jack->current_accessory_state == DEVICE_HEADPHONE ||
		    jack->current_accessory_state == DEVICE_HEADSET)
			switch_set_state(&jack->swdev, NO_DEVICE);

		dev_dbg(jack->dev, "Accessory : ");
		switch (jack->new_accessory_state) {
		case DEVICE_UNKNOWN:
			dev_dbg(jack->dev, "DEVICE_UNKNOWN\n");
			break;
		case DEVICE_UNSUPPORTED:
			dev_err(jack->dev, "UNSUPPORTED\n");
			break;
		case DEVICE_HEADPHONE:
			dev_dbg(jack->dev, "DEVICE_HEADPHONE\n");
			break;
		case NO_DEVICE:
			dev_dbg(jack->dev, "NO_DEVICE\n");
			break;
		case DEVICE_HEADSET:
			dev_dbg(jack->dev, "DEVICE_HEADSET\n");
			msleep(100);
			jack->interface->enable_mic_bias(1);
			jack->interface->register_hssd_button_interrupt(
				simple_remote_button_irq_handler, jack);
			break;
		default:
			break;
		}
		switch_set_state(&jack->swdev, jack->new_accessory_state);
		wake_lock_timeout(&jack->lock, HZ / 2);
		jack->current_accessory_state = jack->new_accessory_state;
	}
}


static int simple_remote_open(struct input_dev *dev)
{
	int err = 0;

	struct simple_remote_driver *jack = input_get_drvdata(dev);

	mutex_lock(&jack->simple_remote_mutex);
	if (!jack->client_counter) {
		dev_dbg(jack->dev, "Starting interrupt handling.");

		err = jack->interface->register_plug_detect_interrupt
			(&simple_remote_detect_irq_handler, jack);

		if (!err) {
			/* This will force an initial detection to
			   make sure that we always have a correct
			   headset state and detect headsets inserted
			   before the phone has been booted.*/
			mod_timer(&jack->plug_det_timer, DETECT_WORK_DELAY);
		} else if (err == -EALREADY) {
			dev_info(jack->dev, "Plug detect interrupt "
				"is already registered\n");
			goto exit;
		} else {
			dev_err(jack->dev, "Failed to "
				"register plug detect interrupt\n");
			goto exit;
		}
	}

	jack->client_counter++;

exit:
	mutex_unlock(&jack->simple_remote_mutex);

	return err;
}


static void simple_remote_close(struct input_dev *dev)
{
	struct simple_remote_driver *jack = input_get_drvdata(dev);

	mutex_lock(&jack->simple_remote_mutex);

	if (1 == jack->client_counter) {
		dev_dbg(jack->dev, "Stopping button interrupt handling.");
		jack->interface->unregister_hssd_button_interrupt(jack);
		jack->interface->enable_mic_bias(0);
		jack->interface->unregister_plug_detect_interrupt(jack);
	}

	if (jack->client_counter > 0)
		jack->client_counter--;

	mutex_unlock(&jack->simple_remote_mutex);
}

/*
 * Plug detection is performed here and this function is only called after
 * the IRQ has been silent DETECTION_DELAY time (ms). This is done in order to
 * avoid gitter in the IRQ line. The functionlity should be quite clear
 * if the plug_detect interrupt handler is analyzed. Besides being quite fast,
 * this detection scheme will make sure that we will not have false detections.
 * If a HEADPHONE or UNKNOWN accessory is detected, we will continue to to call
 * the detection functionality for DETECTION_DELAY * DETECTION_CYCLES because
 * some accessories may generate a different accessory type than the actually
 * are in some circumstances.
 */
static void simple_remote_plug_det_work(struct work_struct *work)
{
	u8 getgpiovalue;
	unsigned int adc_value = DEV_UNKNOWN_ADC_VAL;
	unsigned int alt_adc_val = DEV_UNKNOWN_ADC_VAL;
	enum dev_state state;

	struct simple_remote_driver *jack =
		container_of(work, struct simple_remote_driver, plug_det_work);

	dev_vdbg(jack->dev, "%s - Called\n", __func__);

	jack->interface->get_current_plug_status(&getgpiovalue);

	if (!getgpiovalue) {
		jack->interface->enable_mic_bias(1);
		jack->interface->read_hs_adc(&adc_value);

		dev_dbg(jack->dev, "%s - adc_value = %d\n", __func__,
			adc_value);
		jack->interface->enable_mic_bias(0);
	}

	jack->new_accessory_state =
		simple_remote_attrs_parse_accessory_type(
			jack, adc_value, getgpiovalue);

	/* performing unsupported headset detection */
	if (!getgpiovalue && jack->new_accessory_state == DEVICE_HEADSET) {
		dev_dbg(jack->dev,
			"%s - Headset detected. Checking for unsupported\n",
			__func__);
		jack->interface->enable_mic_bias(1);
		if (0 > jack->interface->enable_alternate_adc_mode(1))
			dev_warn(jack->dev,
				 "%s - Alternate ADC mode did not engage "
				 "correctly. Unsupported headset may not be"
				 "correctly detected!\n", __func__);
		jack->interface->read_hs_adc(&alt_adc_val);
		dev_dbg(jack->dev, "%s - alt_adc_val = %d\n", __func__,
			alt_adc_val);
		jack->interface->enable_mic_bias(0);
		jack->interface->enable_alternate_adc_mode(0);
		state = simple_remote_attrs_parse_accessory_type(
			jack, alt_adc_val, getgpiovalue);
		if (DEVICE_HEADPHONE == state) {
			dev_info(jack->dev,
				 "%s - unsupported headset detected", __func__);
			if (jack->interface->enable_alternate_headset_mode(1)) {
				jack->new_accessory_state = DEVICE_UNSUPPORTED;
				jack->num_supported_hs_det = 0;
			}
		} else {
			jack->num_supported_hs_det++;
		}
	}

	if (jack->new_accessory_state == DEVICE_HEADPHONE)
		jack->num_headphone_det++;
	else
		jack->num_headphone_det = 0;

	/*
	 * Avoid the conflict between audio path changing and alternate
	 * ADC reading.
	 * Accessory state report to the upper layer is deferred until supported
	 * headset detection cycle finishes when the accessory state is
	 * DEVICE_HEADSET.
	 */
	if (!(jack->new_accessory_state == DEVICE_HEADPHONE ||
	    jack->new_accessory_state == DEVICE_HEADSET) ||
	    jack->num_headphone_det >= MIN_NUM_HEADPHONE_DET ||
	    jack->num_supported_hs_det >= MIN_NUM_SUPPORTED_HS_DET)
		simple_remote_report_accessory_type(jack);

	dev_vdbg(jack->dev, "%s - used detection cycles = %d\n", __func__,
		 atomic_read(&jack->detect_cycle));

	/* Restart detection cycle if required */
	if (DETECTION_CYCLES > atomic_read(&jack->detect_cycle) &&
	    jack->num_supported_hs_det < MIN_NUM_SUPPORTED_HS_DET &&
	    MAX_NODEV_CYCLES > jack->nodetect_cycles) {
		dev_vdbg(jack->dev, "%s - Restarting delayed work\n", __func__);
		mod_timer(&jack->plug_det_timer, DETECT_WORK_DELAY);
		atomic_inc(&jack->detect_cycle);
	} else {
		jack->nodetect_cycles = 0;
		atomic_set(&jack->detection_in_progress, 0);
		jack->num_supported_hs_det = 0;
		jack->num_headphone_det = 0;
	}
}


void simple_remote_plug_detect_tmr_func(unsigned long func_data)
{
	struct simple_remote_driver *jack =
		(struct simple_remote_driver *)func_data;

	dev_vdbg(jack->dev, "called\n");

	schedule_work(&jack->plug_det_work);
}


static irqreturn_t simple_remote_detect_irq_handler(int irq, void *dev_id)
{
	struct simple_remote_driver *jack = dev_id;

	dev_vdbg(jack->dev, "%s - Called\n", __func__);

	atomic_set(&jack->detection_in_progress, 1);
	atomic_set(&jack->detect_cycle, 0);
	mod_timer(&jack->plug_det_timer, DETECT_WORK_DELAY);

	return IRQ_HANDLED;
}


static void simple_remote_btn_det_work(struct work_struct *work)
{
	unsigned int adc_value;
	u8 button;

	struct simple_remote_driver *jack =
		container_of(work, struct simple_remote_driver, btn_det_work);

	/* Avoid sending button events if detection is in progress
	 * This may happen if plug is removed while detection work
	 * is in progress. */
	if (atomic_read(&jack->detection_in_progress) > 0) {
		dev_info(jack->dev, "%s - Detection in progress. Ignoring"
			 " button event\n", __func__);
		return;
	}

	atomic_set(&jack->detection_in_progress, 1);

	jack->interface->read_hs_adc(&adc_value);
	dev_dbg(jack->dev, "%s - ADC VAlue = %u\n", __func__, adc_value);

	button = simple_remote_attrs_parse_button_value(jack, adc_value);

	if (button < ARRAY_SIZE(simple_remote_btn_max))
		jack->pressed_button = button;

	report_button_id(jack, button < ARRAY_SIZE(simple_remote_btn_max));

	atomic_set(&jack->detection_in_progress, 0);

}


static irqreturn_t simple_remote_button_irq_handler(int irq, void *data)
{

	struct simple_remote_driver *jack = data;

	dev_dbg(jack->dev, "Received a Button interrupt\n");

	schedule_work(&jack->btn_det_work);

	return IRQ_HANDLED;
}

static int initialize_hardware(struct simple_remote_driver *jack)
{
	int err = jack->interface->set_period_freq(
		simple_remote_trig_period_freq[0]);

	err |= jack->interface->set_period_time(
		simple_remote_trig_period_time[0]);

	err |= jack->interface->set_hysteresis_freq(
		simple_remote_trig_hysteresis_freq[0]);

	err |= jack->interface->set_hysteresis_time(
		simple_remote_trig_hysteresis_time[0]);

	err |= jack->interface->set_trig_level(
		simple_remote_btn_trig_level[0]);

	return err;
}

static int simple_remote_probe(struct platform_device *pdev)
{
	int ret = 0;
	int size;
	void *v;
	struct simple_remote_driver *jack;

	dev_info(&pdev->dev, "**** Registering (headset) driver\n");

	jack = kzalloc(sizeof(struct simple_remote_driver), GFP_KERNEL);

	if (!jack)
		return -ENOMEM;

	mutex_init(&jack->simple_remote_mutex);

	if (!pdev->dev.platform_data)
		goto err_switch_dev_register;


	size = sizeof(*jack->interface) / sizeof(void *);
	v = (void *)pdev->dev.platform_data;

	for (; size > 0; size--)
		if (v++ == NULL)
			goto err_switch_dev_register;

	jack->interface = pdev->dev.platform_data;
	jack->dev = &pdev->dev;

	jack->plug_det_timer.function = simple_remote_plug_detect_tmr_func;
	jack->plug_det_timer.data = (unsigned long)jack;

	init_timer(&jack->plug_det_timer);

	/* Set flags */
	jack->current_accessory_state = NO_DEVICE;
	jack->pressed_button = 0xFF;

	/* Initialize work queue for Simple Remote Driver */
	INIT_WORK(&jack->plug_det_work,
			  simple_remote_plug_det_work);
	INIT_WORK(&jack->btn_det_work,
		  simple_remote_btn_det_work);

	/* device name */
	jack->swdev.name = "h2w";

	/* print function name */
	jack->swdev.print_name = simple_remote_print_name;

	ret = switch_dev_register(&jack->swdev);
	if (ret < 0) {
		dev_err(jack->dev, "switch_dev_register failed\n");
		goto err_switch_dev_register;
	}

	ret = create_sysfs_interfaces(&pdev->dev);
	if (ret) {
		dev_err(jack->dev,
			"create_sysfs_interfaces for input failed\n");
		goto err_switch_dev_register;
	}

	if (initialize_hardware(jack))
		dev_err(jack->dev, "Failed to set default values in HW "
			"components.");

	/* Create input device for normal key events. */
	jack->indev = input_allocate_device();
	if (!jack->indev) {
		ret = -ENOMEM;
		dev_err(jack->dev, "Failed to allocate input device\n");
		goto err_allocate_input_dev;
	}

	jack->indev->name = SIMPLE_REMOTE_NAME;
	jack->indev->evbit[0] = BIT_MASK(EV_KEY);
	jack->indev->keybit[BIT_WORD(KEY_MEDIA)] |= BIT_MASK(KEY_MEDIA);
	jack->indev->keybit[BIT_WORD(BTN_MISC)] |= BIT_MASK(BTN_0) |
		BIT_MASK(BTN_1) | BIT_MASK(BTN_2);

	jack->indev->open = simple_remote_open;
	jack->indev->close = simple_remote_close;

	input_set_drvdata(jack->indev, jack);
	platform_set_drvdata(pdev, jack);

	ret = input_register_device(jack->indev);
	if (ret) {
		dev_err(jack->dev, "input_register_device for input device "
			"failed\n");
		input_free_device(jack->indev);
		goto err_register_input_dev;
	}

	/* Create input device for application key events. */
	jack->indev_appkey = input_allocate_device();
	if (!jack->indev_appkey) {
		ret = -ENOMEM;
		dev_err(jack->dev, "Failed to allocate application key input "
			"device\n");
		goto err_allocate_input_appkey_dev;
	}

	jack->indev_appkey->name = SIMPLE_REMOTE_APPKEY_NAME;
	jack->indev_appkey->evbit[0] = BIT_MASK(EV_KEY);
	jack->indev_appkey->keybit[BIT_WORD(BTN_MISC)] |=
		BIT_MASK(SIMPLE_REMOTE_APPKEY);
	jack->indev_appkey->open = simple_remote_open;
	jack->indev_appkey->close = simple_remote_close;

	input_set_drvdata(jack->indev_appkey, jack);

	ret = input_register_device(jack->indev_appkey);
	if (ret) {
		dev_err(jack->dev, "input_register_device for application key "
				"input device failed\n");
		goto err_register_input_appkey_dev;
	}
	wake_lock_init(&jack->lock, WAKE_LOCK_SUSPEND, "simple remote");

	dev_info(jack->dev, "***** Successfully registered\n");

	return ret;

err_register_input_appkey_dev:
	input_free_device(jack->indev_appkey);
err_allocate_input_appkey_dev:
	input_unregister_device(jack->indev);
err_register_input_dev:
err_allocate_input_dev:
err_switch_dev_register:
	dev_err(&pdev->dev, "***** Failed to initialize\n");
	kzfree(jack);

	return ret;
}


static int simple_remote_remove(struct platform_device *pdev)
{
	struct simple_remote_driver *jack = platform_get_drvdata(pdev);

	if (switch_get_state(&jack->swdev)) {
		jack->interface->unregister_hssd_button_interrupt(jack);
		jack->interface->enable_mic_bias(0);
		switch_set_state(&jack->swdev, NO_DEVICE);
	}
	jack->interface->unregister_plug_detect_interrupt(jack);
	remove_sysfs_interfaces(&pdev->dev);
	input_unregister_device(jack->indev_appkey);
	input_unregister_device(jack->indev);
	switch_dev_unregister(&jack->swdev);
	wake_lock_destroy(&jack->lock);
	kzfree(jack);

	return 0;
}

int simlple_remote_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct simple_remote_driver *jack = dev_get_drvdata(&pdev->dev);

	dev_dbg(jack->dev, "%s - Suspend state = 0x%X\n", __func__,
	       state.event);

	if (atomic_read(&jack->detection_in_progress)) {
		dev_dbg(jack->dev, "%s - Plug detection in progress. "
			"Not allowing suspend\n", __func__);
		return -EAGAIN;
	}

	dev_dbg(jack->dev, "%s - No plug or button detection in progress. "
		"Allowing suspend\n", __func__);

	return 0;
}

static struct platform_driver simple_remote = {
	.probe		= simple_remote_probe,
	.remove		= simple_remote_remove,
	.suspend        = simlple_remote_suspend,
	.driver		= {
		.name		= SIMPLE_REMOTE_NAME,
		.owner		= THIS_MODULE,
	},
};

static int __init simple_remote_init(void)
{
	return platform_driver_register(&simple_remote);
}

static void __exit simple_remote_exit(void)
{
	platform_driver_unregister(&simple_remote);
}

module_init(simple_remote_init);
module_exit(simple_remote_exit);

MODULE_AUTHOR("Joachim Holst");
MODULE_DESCRIPTION("3.5mm audio jack driver");
MODULE_LICENSE("GPL");
