/* kernel/arch/arm/mach-msm/sony_ssm.c
 *
 * Copyright (C) 2012-2013 Sony Mobile Communications AB.
 *
 * Author: Mattias Larsson <mattias7.larsson@sonymobile.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/suspend.h>
#include <linux/earlysuspend.h>

#define MODULE_NAME "sony_ssm"

enum ssm_state {
	SSM_SUSPEND_PREPARE = 0,
	SSM_LATE_RESUME = 1
};

struct ssm_data {
	struct device *dev;
	struct early_suspend early_suspend;
	struct notifier_block pm_notifier;
	struct mutex lock;
	bool enabled;
	bool notify_next_suspend_prepare;
	bool notify_late_resume;
};

static void ssm_notify(struct ssm_data *sd, enum ssm_state state)
{
	char event[8];
	char *envp[] = {event, NULL};

	snprintf(event, sizeof(event), "EVENT=%d", state);

	dev_dbg(sd->dev, "%s: Sending uevent EVENT=%d\n", __func__, state);

	kobject_uevent_env(&sd->dev->kobj, KOBJ_CHANGE, envp);
}

static void ssm_late_resume(struct early_suspend *h)
{
	struct ssm_data *sd = container_of(h, struct ssm_data, early_suspend);

	dev_dbg(sd->dev, "%s\n", __func__);

	mutex_lock(&sd->lock);
	if (sd->notify_late_resume)
		ssm_notify(sd, SSM_LATE_RESUME);
	mutex_unlock(&sd->lock);
}

static int ssm_pm_notifier(struct notifier_block *nb, unsigned long event,
		void *ignored)
{
	struct ssm_data *sd = container_of(nb, struct ssm_data, pm_notifier);

	dev_dbg(sd->dev, "%s: event=%lu\n", __func__, event);

	if (event == PM_SUSPEND_PREPARE) {
		mutex_lock(&sd->lock);
		if (sd->notify_next_suspend_prepare) {
			ssm_notify(sd, SSM_SUSPEND_PREPARE);
			sd->notify_next_suspend_prepare = false;
			mutex_unlock(&sd->lock);
			return NOTIFY_BAD;
		}
		mutex_unlock(&sd->lock);
	}

	return NOTIFY_DONE;
}

static int ssm_enable(struct ssm_data *sd)
{
	int rc;

	dev_dbg(sd->dev, "%s\n", __func__);

	mutex_lock(&sd->lock);

	if (sd->enabled) {
		dev_err(sd->dev, "%s: Already enabled!\n", __func__);
		rc = -EINVAL;
		goto exit;
	}

	rc = register_pm_notifier(&sd->pm_notifier);
	if (rc) {
		dev_err(sd->dev, "%s: Failed to register pm_notifier (%d)\n",
			__func__, rc);
		goto exit;
	}

	register_early_suspend(&sd->early_suspend);

	sd->notify_next_suspend_prepare = false;
	sd->notify_late_resume = false;
	sd->enabled = true;

exit:
	mutex_unlock(&sd->lock);
	return rc;
}

static void ssm_disable(struct ssm_data *sd)
{
	dev_dbg(sd->dev, "%s\n", __func__);

	mutex_lock(&sd->lock);
	if (sd->enabled) {
		unregister_early_suspend(&sd->early_suspend);
		unregister_pm_notifier(&sd->pm_notifier);
		sd->enabled = false;
	} else {
		dev_warn(sd->dev, "%s: Not enabled\n", __func__);
	}
	mutex_unlock(&sd->lock);
}

static ssize_t ssm_store_enable(struct device *pdev,
		struct device_attribute *attr, const char *pbuf, size_t count)
{
	int rc;
	u8 val;
	struct ssm_data *sd = dev_get_drvdata(pdev);

	dev_dbg(sd->dev, "%s: %s\n", __func__, pbuf);

	rc = kstrtou8(pbuf, 2, &val);
	if (!rc) {
		if (!!val)
			rc = ssm_enable(sd);
		else
			ssm_disable(sd);
	}

	return rc == 0 ? count : rc;
}

static ssize_t ssm_store_request_next_suspend_prepare_notification(
		struct device *pdev, struct device_attribute *attr,
		const char *pbuf, size_t count)
{
	int rc;
	u8 val;
	struct ssm_data *sd = dev_get_drvdata(pdev);

	dev_dbg(sd->dev, "%s: %s\n", __func__, pbuf);

	rc = kstrtou8(pbuf, 2, &val);
	if (!rc) {
		mutex_lock(&sd->lock);
		if (sd->enabled) {
			sd->notify_next_suspend_prepare = !!val;
		} else {
			rc = -EINVAL;
			dev_err(sd->dev, "%s: Notifications are not enabled\n",
				__func__);
		}
		mutex_unlock(&sd->lock);
	}

	return rc == 0 ? count : rc;
}

static ssize_t ssm_store_set_late_resume_notifications(struct device *pdev,
		struct device_attribute *attr, const char *pbuf, size_t count)
{
	int rc;
	u8 val;
	struct ssm_data *sd = dev_get_drvdata(pdev);

	dev_dbg(sd->dev, "%s: %s\n", __func__, pbuf);

	rc = kstrtou8(pbuf, 2, &val);
	if (!rc) {
		mutex_lock(&sd->lock);
		if (sd->enabled) {
			sd->notify_late_resume = !!val;
		} else {
			rc = -EINVAL;
			dev_err(sd->dev, "%s: Notifications are not enabled\n",
				__func__);
		}
		mutex_unlock(&sd->lock);
	}

	return rc == 0 ? count : rc;
}

static struct device_attribute ssm_attrs[] = {
	__ATTR(enable, 0600, NULL,
			ssm_store_enable),
	__ATTR(set_request_next_suspend_prepare_notification, 0600, NULL,
			ssm_store_request_next_suspend_prepare_notification),
	__ATTR(set_late_resume_notifications, 0600, NULL,
			ssm_store_set_late_resume_notifications),
};

static int ssm_create_attrs(struct device *dev)
{
	unsigned int i;
	int rc;
	for (i = 0; i < ARRAY_SIZE(ssm_attrs); i++) {
		rc = device_create_file(dev, &ssm_attrs[i]);
		if (rc)
			goto err;
	}
	return 0;
err:
	while (i--)
		device_remove_file(dev, &ssm_attrs[i]);
	return rc;
}

static void ssm_remove_attrs(struct device *dev)
{
	unsigned int i;
	for (i = 0; i < ARRAY_SIZE(ssm_attrs); i++)
		(void)device_remove_file(dev, &ssm_attrs[i]);
}

static int ssm_probe(struct platform_device *pdev)
{
	int rc;
	struct ssm_data *sd;

	dev_dbg(&pdev->dev, "%s\n", __func__);

	sd = kzalloc(sizeof(struct ssm_data), GFP_KERNEL);
	if (!sd) {
		dev_err(&pdev->dev, "%s: OOM for ssm_data\n", __func__);
		return -ENOMEM;
	}

	sd->early_suspend.level = EARLY_SUSPEND_LEVEL_STOP_DRAWING;
	sd->early_suspend.resume = ssm_late_resume;

	sd->dev = &pdev->dev;

	mutex_init(&sd->lock);

	sd->enabled = false;
	sd->notify_next_suspend_prepare = false;
	sd->notify_late_resume = false;

	sd->pm_notifier.notifier_call = ssm_pm_notifier;
	sd->pm_notifier.priority = 0;

	rc = ssm_create_attrs(&pdev->dev);
	if (rc) {
		dev_err(sd->dev, "%s: Failed to create attrs (%d)\n",
			__func__, rc);
		goto err_create_attrs;
	}

	platform_set_drvdata(pdev, sd);

	return 0;

err_create_attrs:
	kfree(sd);
	return rc;
}

static int ssm_remove(struct platform_device *pdev)
{
	struct ssm_data *sd = platform_get_drvdata(pdev);

	dev_dbg(sd->dev, "%s\n", __func__);

	ssm_remove_attrs(sd->dev);

	if (sd->enabled) {
		unregister_early_suspend(&sd->early_suspend);
		unregister_pm_notifier(&sd->pm_notifier);
	}

	kfree(sd);

	return 0;
}

static struct platform_device ssm_device = {
	.name = MODULE_NAME,
	.id = -1,
};

static struct platform_driver ssm_driver = {
	.probe = ssm_probe,
	.remove = ssm_remove,
	.driver = {
		.name = MODULE_NAME,
		.owner = THIS_MODULE,
	},
};

static int __init ssm_init(void)
{
	int rc;

	pr_debug("%s\n", __func__);

	rc = platform_driver_register(&ssm_driver);
	if (rc) {
		pr_err("%s: Failed to register driver (%d)\n", __func__, rc);
		return rc;
	}

	rc = platform_device_register(&ssm_device);
	if (rc) {
		platform_driver_unregister(&ssm_driver);
		pr_err("%s: Failed to register device (%d)\n", __func__, rc);
		return rc;
	}

	return 0;
}

static void __exit ssm_exit(void)
{
	pr_debug("%s\n", __func__);
	platform_device_unregister(&ssm_device);
	platform_driver_unregister(&ssm_driver);
}

late_initcall(ssm_init);
module_exit(ssm_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Power management notifications for Super Stamina Mode");
