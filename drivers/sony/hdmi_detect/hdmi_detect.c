/*
 * Copyright 2021 Sony Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation.
 */

#include <linux/file.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/device.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <media/v4l2-dev.h>
#include <media/v4l2-device.h>
#include <media/v4l2-ioctl.h>

#define DRIVER_NAME "hdmi_detect"
#define VIDEO_DEVICE_NUMBER 63

struct hdmi_detect_data {
	struct platform_device *pdev;
	int gpio;
	unsigned long irq_flags;
	struct v4l2_device v4l2_dev;
	struct video_device vdev;
	bool registered;
	int samplerate;
};

static int hdmi_detect_v4l2_open(struct file *file) {
	return 0;
}

static int hdmi_detect_v4l2_close(struct file *file) {
	return 0;
}

static int hdmi_detect_v4l2_querycap(struct file *file, void *fh, struct v4l2_capability *cap) {
	memset(cap, 0, sizeof(struct v4l2_capability));
	strlcpy(cap->driver, DRIVER_NAME, sizeof(cap->driver));
	cap->version = 1;
	cap->device_caps = V4L2_CAP_VIDEO_CAPTURE;
	cap->capabilities = V4L2_CAP_DEVICE_CAPS;
	return 0;
}

static const struct v4l2_ioctl_ops hdmi_detect_v4l2_ioctl_ops = {
	.vidioc_querycap = hdmi_detect_v4l2_querycap,
	.vidioc_s_fmt_vid_cap_mplane = NULL,
	.vidioc_s_fmt_vid_out_mplane = NULL,
	.vidioc_g_fmt_vid_cap_mplane = NULL,
	.vidioc_g_fmt_vid_out_mplane = NULL,
	.vidioc_reqbufs = NULL,
	.vidioc_qbuf = NULL,
	.vidioc_dqbuf = NULL,
	.vidioc_streamon = NULL,
	.vidioc_streamoff = NULL,
	.vidioc_s_ctrl = NULL,
	.vidioc_g_ctrl = NULL,
	.vidioc_queryctrl = NULL,
	.vidioc_subscribe_event = NULL,
	.vidioc_unsubscribe_event = NULL,
	.vidioc_decoder_cmd = NULL,
	.vidioc_encoder_cmd = NULL,
	.vidioc_enum_framesizes = NULL,
	.vidioc_default = NULL,
};

static const struct v4l2_file_operations hdmi_detect_v4l2_file_ops = {
	.owner = THIS_MODULE,
	.open = hdmi_detect_v4l2_open,
	.release = hdmi_detect_v4l2_close,
	.unlocked_ioctl = video_ioctl2,
	.compat_ioctl32 = NULL,
	.poll = NULL,
};

static int hdmi_detect_insert_video_device(struct hdmi_detect_data *data)
{
	int rc = 0;

	if (data->registered)
		return 0;
	rc = v4l2_device_register(&data->pdev->dev, &data->v4l2_dev);
	if (rc) {
		dev_err(&data->pdev->dev, "%s: v4l2_device_register error %d\n", __func__, rc);
		return rc;
	}
	memset(&data->vdev, 0, sizeof(struct video_device));
	data->vdev.v4l2_dev = &data->v4l2_dev;
	data->vdev.vfl_dir = VFL_DIR_M2M;
	data->vdev.release = video_device_release_empty;
	data->vdev.fops = &hdmi_detect_v4l2_file_ops;
	data->vdev.ioctl_ops = &hdmi_detect_v4l2_ioctl_ops;
	video_set_drvdata(&data->vdev, data);
	rc = video_register_device(&data->vdev, VFL_TYPE_VIDEO, VIDEO_DEVICE_NUMBER);
	if (rc) {
		dev_err(&data->pdev->dev, "%s: video_register_device error %d\n", __func__, rc);
		return rc;
	}
	data->registered = true;
	dev_info(&data->pdev->dev, "%s: /dev/video%d is inserted\n", __func__, VIDEO_DEVICE_NUMBER);
	return rc;
}

static void hdmi_detect_delete_video_device(struct hdmi_detect_data *data)
{
	if (!data->registered)
		return;
	v4l2_device_unregister(&data->v4l2_dev);
	video_unregister_device(&data->vdev);
	data->registered = false;
	dev_info(&data->pdev->dev, "%s: /dev/video%d is deleted\n", __func__, VIDEO_DEVICE_NUMBER);
	return;
}

static bool hdmi_detect_is_cable_connected(struct hdmi_detect_data *data)
{
	int value;
	if (!data)
		return false;

	value = gpio_get_value_cansleep(data->gpio);
	if (value == 0) {
		// LO (connected)
		return true;
	} else if (value == 1) {
		// HI (disconnected)
		return false;
	}

	dev_err(&data->pdev->dev, "%s: invalid value %d\n", __func__, value);
	return false;
}

static void hdmi_detect_update_video_device(struct hdmi_detect_data *data)
{
	if (hdmi_detect_is_cable_connected(data)) {
		dev_info(&data->pdev->dev, "%s: cable connected\n", __func__);
		hdmi_detect_insert_video_device(data);
	} else {
		dev_info(&data->pdev->dev, "%s: cable disconnected\n", __func__);
		hdmi_detect_delete_video_device(data);
	}
}

static ssize_t state_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct hdmi_detect_data *data = dev_get_drvdata(dev);
	bool connected = false;
	if (!data || !buf)
		return -EINVAL;

	connected = hdmi_detect_is_cable_connected(data);
	return sprintf(buf, connected ? "connected\n" : "disconnected\n");
}

static ssize_t state_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t len)
{
	struct hdmi_detect_data *data = dev_get_drvdata(dev);
	if (!data)
		return 0;

	if (!buf || len < 1)
		return -EINVAL;

	if (!strncmp(buf, "disconnect", len < 10 ? len : 10)) {
		dev_info(dev, "force cable disconnection\n");
		hdmi_detect_delete_video_device(data);
		sysfs_notify(&dev->kobj, NULL, "state");
		return len;
	}

	if (!strncmp(buf, "connect", len < 7 ? len : 7)) {
		if (hdmi_detect_is_cable_connected(data)) {
			hdmi_detect_insert_video_device(data);
			sysfs_notify(&dev->kobj, NULL, "state");
			return len;
		}
	}

	return -EINVAL;
}

static ssize_t samplerate_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct hdmi_detect_data *data = dev_get_drvdata(dev);
	if (!data || !buf)
		return -EINVAL;

	if (data->registered && data->samplerate > 0)
		return sprintf(buf, "%d\n", data->samplerate);
	return sprintf(buf, "disabled\n");
}

static ssize_t samplerate_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t len)
{
	struct hdmi_detect_data *data = dev_get_drvdata(dev);
	int newrate = 0;
	if (!data)
		return 0;

	if (!buf || len < 1)
		return -EINVAL;

	if (!strncmp(buf, "disabled", len < 8 ? len : 8)) {
		data->samplerate = 0;
	} else if (!kstrtoint(buf, 10, &newrate)) {
		data->samplerate = newrate;
	} else {
		return -EINVAL;
	}

	dev_info(dev, "%s: set samplerate to %d\n", __func__, data->samplerate);
	sysfs_notify(&dev->kobj, NULL, "samplerate");
	return len;
}

static DEVICE_ATTR_RW(state);

static DEVICE_ATTR_RW(samplerate);

static struct attribute *sysfs_attrs[] = {
	&dev_attr_state.attr,
	&dev_attr_samplerate.attr,
	NULL,
};

static const struct attribute_group sysfs_attr_group = {
	.attrs = sysfs_attrs,
};

static irqreturn_t hdmi_detect_irq_handler(int irq, void *irq_data)
{
	struct hdmi_detect_data *data = irq_data;

	dev_info(&data->pdev->dev, "%s: IRQ happened\n", __func__);
	hdmi_detect_update_video_device(data);
	return IRQ_HANDLED;
}

static int hdmi_detect_probe(struct platform_device *pdev)
{
	int rc = 0;
	struct hdmi_detect_data *data = NULL;
	struct device_node *node = pdev->dev.of_node;
	uint32_t node_data[2];
	int irq = 0;

	data = kzalloc(sizeof(struct hdmi_detect_data), GFP_KERNEL);
	if (!data) {
		dev_err(&pdev->dev, "%s: kzalloc error\n", __func__);
		return -ENOMEM;
	}
	rc = of_property_read_u32_array(node, "interrupts", node_data, 2);
	if (rc) {
		dev_err(&pdev->dev, "%s: failed to read interrupts property: %d\n", __func__, rc);
		goto fail_gpio;
	}
	data->gpio = node_data[0];
	data->irq_flags = node_data[1];
	rc = gpio_request_one(data->gpio, GPIOF_DIR_IN, DRIVER_NAME);
	if (rc) {
		dev_err(&pdev->dev, "%s: failed to request gpio %d: %d\n", __func__, data->gpio, rc);
		goto fail_gpio;
	}
	irq = gpio_to_irq(data->gpio);
	dev_info(&pdev->dev, "%s: using gpio %d (irq %d) flags %d\n", __func__, data->gpio, irq, data->irq_flags);
	rc = devm_request_threaded_irq(&pdev->dev, irq, NULL,
		hdmi_detect_irq_handler, data->irq_flags | IRQF_ONESHOT, DRIVER_NAME, data);
	if (rc) {
		dev_err(&pdev->dev, "%s: devm_threaded_irq error %d\n", __func__, rc);
		goto fail_irq;
	}
	rc = v4l2_device_register(&pdev->dev, &data->v4l2_dev);
	if (rc) {
		dev_err(&pdev->dev, "%s: v4l2_device_register error %d\n", __func__, rc);
		goto fail_v4l2;
	}
	rc = sysfs_create_group(&pdev->dev.kobj, &sysfs_attr_group);
	if (rc) {
		dev_err(&pdev->dev, "%s: sysfs_create_group failed: %d\n", __func__, rc);
		goto fail_v4l2;
	}
	dev_set_drvdata(&pdev->dev, data);
	data->pdev = pdev;
	hdmi_detect_update_video_device(data);
	dev_info(&pdev->dev, "%s: probe success\n", __func__);
	return 0;

fail_v4l2:
	devm_free_irq(&pdev->dev, irq, data);
fail_irq:
	gpio_free(data->gpio);
fail_gpio:
	kfree(data);
	return rc;
}

static int hdmi_detect_remove(struct platform_device *pdev)
{
	struct hdmi_detect_data *data = NULL;

	data = dev_get_drvdata(&pdev->dev);
	if (data && data->pdev) {
		int irq = gpio_to_irq(data->gpio);
		sysfs_remove_group(&pdev->dev.kobj, &sysfs_attr_group);
		hdmi_detect_delete_video_device(data);
		v4l2_device_unregister(&data->v4l2_dev);
		devm_free_irq(&pdev->dev, irq, data);
		gpio_free(data->gpio);
		kfree(data);
	}
	return 0;
}

static const struct of_device_id hdmi_detect_dt_match[] = {
	{ .compatible = "somc,hdmi_detect" },
};

static struct platform_driver hdmi_detect_driver = {
	.probe = hdmi_detect_probe,
	.remove = hdmi_detect_remove,
	.driver = {
		.name = DRIVER_NAME,
		.owner = THIS_MODULE,
		.of_match_table = hdmi_detect_dt_match,
		.pm = NULL,
	},
};

static int __init hdmi_detect_init(void)
{
	return platform_driver_register(&hdmi_detect_driver);
}

static void __exit hdmi_detect_exit(void)
{
	platform_driver_unregister(&hdmi_detect_driver);
}

module_init(hdmi_detect_init);
module_exit(hdmi_detect_exit);

MODULE_DESCRIPTION("SONY camera hdmi detection driver");
MODULE_LICENSE("GPL v2");
