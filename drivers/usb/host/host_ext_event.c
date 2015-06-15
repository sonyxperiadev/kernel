 /* drivers/usb/host/host_ext_event.c
 *
 * USB host event handling function
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */
/*
 * Copyright (C) 2013 Sony Mobile Communications Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/usb/host_ext_event.h>
#include <linux/cdev.h>
#include <linux/semaphore.h>
#include <linux/slab.h>

#define MAX_NAME_SIZE 32
#define MAX_MODULE_NAME_SIZE 32
#define MAX_EVENT_STRING_SIZE 35

struct host_ext_event_drv {
	struct class *class;
	struct device *dev;
	char name[MAX_NAME_SIZE + 1];
	int event;
};

static DEFINE_SEMAPHORE(sem);

static struct host_ext_event_drv *host_ext_event;

static const char *event_string(enum usb_host_ext_event event)
{
	switch (event) {
	case USB_HOST_EXT_EVENT_NONE:
		return "USB_HOST_NONE";
	case USB_HOST_EXT_EVENT_VBUS_DROP:
		return "USB_HOST_VBUS_DROP";
	case USB_HOST_EXT_EVENT_INSUFFICIENT_POWER:
		return "USB_HOST_INSUFFICIENT_POWER";
	default:
		return "UNDEFINED";
	}
}

int host_send_uevent(enum usb_host_ext_event event)
{
	struct host_ext_event_drv *dev = host_ext_event;

	char udev_event[MAX_EVENT_STRING_SIZE];
	char module[MAX_MODULE_NAME_SIZE];
	char *envp[] = {module, udev_event, NULL};
	int ret;

	if (dev == NULL)
		return -ENODEV;

	ret = down_interruptible(&sem);
	if (ret < 0)
		return ret;

	pr_info("%s: sending %s event\n", dev->name, event_string(event));

	snprintf(udev_event, MAX_EVENT_STRING_SIZE, "EVENT=%s",
			event_string(event));
	snprintf(module, MAX_MODULE_NAME_SIZE, "MODULE=%s", dev->name);
	ret = kobject_uevent_env(&dev->dev->kobj, KOBJ_CHANGE, envp);

	dev->event = event;

	up(&sem);

	if (ret < 0)
		pr_info("uevent sending failed with ret = %d\n", ret);

	return ret;
}

static int usb_host_ext_uevent(struct device *dev, struct kobj_uevent_env *env)
{
	int ret;

	struct host_ext_event_drv *udev = host_ext_event;

	if (udev == NULL)
		return -ENODEV;

	ret = add_uevent_var(env, "%d", udev->event);
	if (ret) {
		dev_err(dev, "failed to add usb host ext uevent\n");
		return ret;
	}

	return 0;
}

static int __init host_ext_event_driver_register(void)
{
	struct host_ext_event_drv *dev;
	int ret;

	if (host_ext_event)
		return -EBUSY;

	dev = kzalloc(sizeof(struct host_ext_event_drv), GFP_KERNEL);
	if (!dev)
		return -ENOMEM;

	host_ext_event = dev;

	dev->class = class_create(THIS_MODULE, "usb_host_ext_event");
	if (IS_ERR(dev->class)) {
		pr_info("%s :failed to create class", __func__);
		ret = PTR_ERR(dev->class);
		goto class_create_fail;
	}

	dev->class->dev_uevent = usb_host_ext_uevent;

	dev->event = USB_HOST_EXT_EVENT_NONE;

	strlcpy(dev->name, "usb_host_ext_event", MAX_NAME_SIZE);

	dev->dev = device_create(dev->class, NULL, MKDEV(0, 0), NULL,
					dev->name);
	if (IS_ERR(dev->dev)) {
		pr_info("%s :failed to create device", __func__);
		ret = PTR_ERR(dev->dev);
		goto device_create_fail;
	}

	pr_info("usb_host_ext_event has been registered!");

	return 0;

device_create_fail:
	class_destroy(dev->class);
class_create_fail:
	kfree(dev);
	host_ext_event = NULL;
	return ret;
}

static void __exit host_ext_event_driver_unregister(void)
{
	struct host_ext_event_drv *dev = host_ext_event;

	down(&sem);

	device_destroy(dev->class, MKDEV(0, 0));
	class_destroy(dev->class);
	kfree(dev);
	host_ext_event = NULL;

	up(&sem);
}

module_init(host_ext_event_driver_register);
module_exit(host_ext_event_driver_unregister);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Extra event notifier of USB host");
