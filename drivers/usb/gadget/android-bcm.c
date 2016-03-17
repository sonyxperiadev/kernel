/*
 * Gadget Driver for Android
 *
 * Copyright (C) 2008 Google, Inc.
 *.Copyright (c) 2014, The Linux Foundation. All rights reserved.
 * Author: Mike Lockwood <lockwood@android.com>
 *         Benoit Goby <benoit@android.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/utsname.h>
#include <linux/platform_device.h>
#include <linux/pm_qos.h>
#include <linux/of.h>

#include <linux/usb/ch9.h>
#include <linux/usb/composite.h>
#include <linux/usb/gadget.h>
#include <linux/usb/android.h>

#include <linux/qcom/diag_dload.h>

#include "gadget_chips.h"

#include "f_fs.c"
#ifdef CONFIG_SND_PCM
#include "f_audio_source.c"
#endif
#include "f_mass_storage.c"
#define USB_ETH_RNDIS y
#include "f_diag.c"
#include "f_qdss.c"
#include "f_rmnet_smd.c"
#include "f_rmnet.c"
#include "f_gps.c"
#include "u_smd.c"
#include "u_bam.c"
#include "u_rmnet_ctrl_smd.c"
#include "u_ctrl_qti.c"
#include "u_ctrl_hsic.c"
#include "u_data_hsic.c"
#include "u_ctrl_hsuart.c"
#include "u_data_hsuart.c"
#include "f_ccid.c"
#include "f_mtp.c"
#include "f_accessory.c"
#include "f_rndis.c"
#include "rndis.c"
#include "f_qc_ecm.c"
#include "f_mbim.c"
#include "f_qc_rndis.c"
#include "u_data_ipa.c"
#include "u_bam_data.c"
#include "f_ecm.c"
#include "u_ether.c"
#include "u_qc_ether.c"
#ifdef CONFIG_TARGET_CORE
#endif
#ifdef CONFIG_SND_PCM
#include "u_uac1.c"
#include "f_uac1.c"
#endif
#include "f_ncm.c"
#include "f_charger.c"

MODULE_AUTHOR("Mike Lockwood");
MODULE_DESCRIPTION("Android Composite USB Driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");

static const char longname[] = "Gadget Android";

/* Default vendor and product IDs, overridden by userspace */
#define VENDOR_ID		0x18D1
#define PRODUCT_ID		0x0001

#define ANDROID_DEVICE_NODE_NAME_LENGTH 11

struct android_usb_function {
	char *name;
	void *config;

	struct device *dev;
	char *dev_name;
	struct device_attribute **attributes;

	/* for android_dev.enabled_functions */
	struct list_head enabled_list;

	/* Optional: initialization during gadget bind */
	int (*init)(struct android_usb_function *, struct usb_composite_dev *);
	/* Optional: cleanup during gadget unbind */
	void (*cleanup)(struct android_usb_function *);
	/* Optional: called when the function is added the list of
	 *		enabled functions */
	void (*enable)(struct android_usb_function *);
	/* Optional: called when it is removed */
	void (*disable)(struct android_usb_function *);

	int (*bind_config)(struct android_usb_function *,
			   struct usb_configuration *);

	/* Optional: called when the configuration is removed */
	void (*unbind_config)(struct android_usb_function *,
			      struct usb_configuration *);
	/* Optional: handle ctrl requests before the device is configured */
	int (*ctrlrequest)(struct android_usb_function *,
					struct usb_composite_dev *,
					const struct usb_ctrlrequest *);
};

struct android_usb_function_holder {

	struct android_usb_function *f;

	/* for android_conf.enabled_functions */
	struct list_head enabled_list;
};

/**
* struct android_dev - represents android USB gadget device
* @name: device name.
* @functions: an array of all the supported USB function
*    drivers that this gadget support but not necessarily
*    added to one of the gadget configurations.
* @cdev: The internal composite device. Android gadget device
*    is a composite device, such that it can support configurations
*    with more than one function driver.
* @dev: The kernel device that represents this android device.
* @enabled: True if the android gadget is enabled, means all
*    the configurations were set and all function drivers were
*    bind and ready for USB enumeration.
* @disable_depth: Number of times the device was disabled, after
*    symmetrical number of enables the device willl be enabled.
*    Used for controlling ADB userspace disable/enable requests.
* @mutex: Internal mutex for protecting device member fields.
* @pdata: Platform data fetched from the kernel device platfrom data.
* @connected: True if got connect notification from the gadget UDC.
*    False if got disconnect notification from the gadget UDC.
* @sw_connected: Equal to 'connected' only after the connect
*    notification was handled by the android gadget work function.
* @suspended: True if got suspend notification from the gadget UDC.
*    False if got resume notification from the gadget UDC.
* @sw_suspended: Equal to 'suspended' only after the susped
*    notification was handled by the android gadget work function.
* @pm_qos: An attribute string that can be set by user space in order to
*    determine pm_qos policy. Set to 'high' for always demand pm_qos
*    when USB bus is connected and resumed. Set to 'low' for disable
*    any setting of pm_qos by this driver. Default = 'high'.
* @work: workqueue used for handling notifications from the gadget UDC.
* @configs: List of configurations currently configured into the device.
*    The android gadget supports more than one configuration. The host
*    may choose one configuration from the suggested.
* @configs_num: Number of configurations currently configured and existing
*    in the configs list.
* @list_item: This driver supports more than one android gadget device (for
*    example in order to support multiple USB cores), therefore this is
*    a item in a linked list of android devices.
*/
struct android_dev {
	const char *name;
	struct android_usb_function **functions;
	struct list_head enabled_functions;
	struct usb_composite_dev *cdev;
	struct device *dev;

	void (*setup_complete)(struct usb_ep *ep,
				struct usb_request *req);

	bool enabled;
	int disable_depth;
	struct mutex mutex;
	struct android_usb_platform_data *pdata;

	bool connected;
	bool sw_connected;
	bool suspended;
	bool sw_suspended;
	char pm_qos[5];
	struct pm_qos_request pm_qos_req_dma;
	unsigned up_pm_qos_sample_sec;
	unsigned up_pm_qos_threshold;
	unsigned down_pm_qos_sample_sec;
	unsigned down_pm_qos_threshold;
	unsigned idle_pc_rpm_no_int_secs;
	struct delayed_work pm_qos_work;
	enum android_pm_qos_state curr_pm_qos_state;
	struct work_struct work;
	char ffs_aliases[256];

	/* A list of struct android_configuration */
	struct list_head configs;
	int configs_num;

	/* A list node inside the android_dev_list */
	struct list_head list_item;
};

struct android_configuration {
	struct usb_configuration usb_config;

	/* A list of the functions supported by this config */
	struct list_head enabled_functions;

	/* A list node inside the struct android_dev.configs list */
	struct list_head list_item;
};

struct dload_struct __iomem *diag_dload;
static struct class *android_class;
static struct android_dev *_android_dev;
static int android_bind_config(struct usb_configuration *c);
static void android_unbind_config(struct usb_configuration *c);
static void android_disconnect(struct usb_composite_dev *cdev);
static int usb_diag_update_pid_and_serial_num(uint32_t pid, const char *snum);

/* string IDs are assigned dynamically */
#define STRING_MANUFACTURER_IDX		0
#define STRING_PRODUCT_IDX		1
#define STRING_SERIAL_IDX		2

static char manufacturer_string[256];
static char product_string[256];
static char serial_string[256];

/* String Table */
static struct usb_string strings_dev[] = {
	[STRING_MANUFACTURER_IDX].s = manufacturer_string,
	[STRING_PRODUCT_IDX].s = product_string,
	[STRING_SERIAL_IDX].s = serial_string,
	{  }			/* end of list */
};

static struct usb_gadget_strings stringtab_dev = {
	.language	= 0x0409,	/* en-us */
	.strings	= strings_dev,
};

static struct usb_gadget_strings *dev_strings[] = {
	&stringtab_dev,
	NULL,
};

static struct usb_device_descriptor device_desc = {
	.bLength              = sizeof(device_desc),
	.bDescriptorType      = USB_DT_DEVICE,
	.bcdUSB               = __constant_cpu_to_le16(0x0200),
	.bDeviceClass         = USB_CLASS_PER_INTERFACE,
	.idVendor             = __constant_cpu_to_le16(VENDOR_ID),
	.idProduct            = __constant_cpu_to_le16(PRODUCT_ID),
	.bcdDevice            = __constant_cpu_to_le16(0xffff),
	.bNumConfigurations   = 1,
};

static struct usb_otg_descriptor otg_descriptor = {
	.bLength =		sizeof otg_descriptor,
	.bDescriptorType =	USB_DT_OTG,
	.bmAttributes =		USB_OTG_SRP | USB_OTG_HNP,
	.bcdOTG               = __constant_cpu_to_le16(0x0200),
};
/*
static const struct usb_descriptor_header *otg_desc[] = {
	(struct usb_descriptor_header *) &otg_descriptor,
	NULL,
};

*/
static const char *pm_qos_to_string(enum android_pm_qos_state state)
{
	switch (state) {
	case NO_USB_VOTE:	return "NO_USB_VOTE";
	case WFI:		return "WFI";
	case IDLE_PC:		return "IDLE_PC";
	case IDLE_PC_RPM:	return "IDLE_PC_RPM";
	default:		return "INVALID_STATE";
	}
}

static struct usb_configuration android_config_driver = {
	.label		= "android",
	.unbind		= android_unbind_config,
	.bConfigurationValue = 1,
	.bmAttributes	= USB_CONFIG_ATT_ONE | USB_CONFIG_ATT_SELFPOWER,
	.MaxPower	= 500, /* 500ma */
};

static void android_work(struct work_struct *data)
{
	struct android_dev *dev = container_of(data, struct android_dev, work);
	struct usb_composite_dev *cdev = dev->cdev;
	char *disconnected[2] = { "USB_STATE=DISCONNECTED", NULL };
	char *connected[2]    = { "USB_STATE=CONNECTED", NULL };
	char *configured[2]   = { "USB_STATE=CONFIGURED", NULL };
	char **uevent_envp = NULL;
	unsigned long flags;

	spin_lock_irqsave(&cdev->lock, flags);
	if (cdev->config)
		uevent_envp = configured;
	else if (dev->connected != dev->sw_connected)
		uevent_envp = dev->connected ? connected : disconnected;
	dev->sw_connected = dev->connected;
	spin_unlock_irqrestore(&cdev->lock, flags);

	if (uevent_envp) {
		kobject_uevent_env(&dev->dev->kobj, KOBJ_CHANGE, uevent_envp);
		pr_info("%s: sent uevent %s\n", __func__, uevent_envp[0]);
	} else {
		pr_info("%s: did not send uevent (%d %d %p)\n", __func__,
			 dev->connected, dev->sw_connected, cdev->config);
	}
}

static int android_enable(struct android_dev *dev)
{
	struct usb_composite_dev *cdev = dev->cdev;
	int err = 0;

	if (WARN_ON(!dev->disable_depth))
		return err;

	if (--dev->disable_depth == 0) {
		usb_add_config(cdev, &android_config_driver,
					android_bind_config);
		usb_gadget_connect(cdev->gadget);
	}

	return err;
}

static void android_disable(struct android_dev *dev)
{
	struct usb_composite_dev *cdev = dev->cdev;

	BUG_ON(!mutex_is_locked(&dev->mutex));

	if (dev->disable_depth++ == 0) {
		usb_gadget_disconnect(cdev->gadget);
		/* Cancel pending control requests */
		usb_ep_dequeue(cdev->gadget->ep0, cdev->req);
		usb_remove_config(cdev, &android_config_driver);
		/*
		 * Moving the disconnect uevent to after
		 * cdev->config is made NULL. Else, if the
		 * android_work is scheduled before cdev->config
		 * is made NULL we may miss out the disconnect uevent.
		 */
		android_disconnect(cdev);
	}
}

/*-------------------------------------------------------------------------*/
/* Supported functions initialization */

struct functionfs_config {
	bool opened;
	bool enabled;
	struct ffs_data *data;
	struct android_dev *dev;
};

static int ffs_function_init(struct android_usb_function *f,
			     struct usb_composite_dev *cdev)
{
	f->config = kzalloc(sizeof(struct functionfs_config), GFP_KERNEL);
	if (!f->config)
		return -ENOMEM;

	return functionfs_init();
}

static void ffs_function_cleanup(struct android_usb_function *f)
{
	functionfs_cleanup();
	kfree(f->config);
}

static void ffs_function_enable(struct android_usb_function *f)
{
	struct android_dev *dev = _android_dev;
	struct functionfs_config *config = f->config;

	config->enabled = true;

	/* Disable the gadget until the function is ready */
	if (!config->opened)
		android_disable(dev);
}

static void ffs_function_disable(struct android_usb_function *f)
{
	struct android_dev *dev = _android_dev;
	struct functionfs_config *config = f->config;

	config->enabled = false;

	/* Balance the disable that was called in closed_callback */
	if (!config->opened)
		android_enable(dev);
}

static int ffs_function_bind_config(struct android_usb_function *f,
				    struct usb_configuration *c)
{
	struct functionfs_config *config = f->config;
	return functionfs_bind_config(c->cdev, c, config->data);
}

static ssize_t
ffs_aliases_show(struct device *pdev, struct device_attribute *attr, char *buf)
{
	struct android_dev *dev = _android_dev;
	int ret;

	mutex_lock(&dev->mutex);
	ret = sprintf(buf, "%s\n", dev->ffs_aliases);
	mutex_unlock(&dev->mutex);

	return ret;
}

static ssize_t
ffs_aliases_store(struct device *pdev, struct device_attribute *attr,
					const char *buf, size_t size)
{
	struct android_dev *dev = _android_dev;
	char buff[256];

	mutex_lock(&dev->mutex);

	if (dev->enabled) {
		mutex_unlock(&dev->mutex);
		return -EBUSY;
	}

	strlcpy(buff, buf, sizeof(buff));
	strlcpy(dev->ffs_aliases, strim(buff), sizeof(dev->ffs_aliases));

	mutex_unlock(&dev->mutex);

	return size;
}

static DEVICE_ATTR(aliases, S_IRUGO | S_IWUSR, ffs_aliases_show,
					       ffs_aliases_store);
static struct device_attribute *ffs_function_attributes[] = {
	&dev_attr_aliases,
	NULL
};

static struct android_usb_function ffs_function = {
	.name		= "ffs",
	.init		= ffs_function_init,
	.enable		= ffs_function_enable,
	.disable	= ffs_function_disable,
	.cleanup	= ffs_function_cleanup,
	.bind_config	= ffs_function_bind_config,
	.attributes	= ffs_function_attributes,
};

static int functionfs_ready_callback(struct ffs_data *ffs)
{
	struct android_dev *dev = _android_dev;
	struct functionfs_config *config = ffs_function.config;
	int ret = 0;

	/* dev is null in case ADB is not in the composition */
	if (dev) {
		mutex_lock(&dev->mutex);
		ret = functionfs_bind(ffs, dev->cdev);
		if (ret) {
			mutex_unlock(&dev->mutex);
			return ret;
		}
	} else {
		/* android ffs_func requires daemon to start only after enable*/
		pr_debug("start adbd only in ADB composition\n");
		return -ENODEV;
	}

	config->data = ffs;
	config->opened = true;
	/* Save dev in case the adb function will get disabled */
	config->dev = dev;

	if (config->enabled)
		android_enable(dev);

	mutex_unlock(&dev->mutex);

	return 0;
}

static void functionfs_closed_callback(struct ffs_data *ffs)
{
	struct android_dev *dev = _android_dev;
	struct functionfs_config *config = ffs_function.config;

	/*
	 * In case new composition is without ADB or ADB got disabled by the
	 * time ffs_daemon was stopped then use saved one
	 */
	if (!dev)
		dev = config->dev;

	/* fatal-error: It should never happen */
	if (!dev)
		pr_err("adb_closed_callback: config->dev is NULL");

	if (dev)
		mutex_lock(&dev->mutex);

	if (config->enabled && dev)
		android_disable(dev);

	config->dev = NULL;

	config->opened = false;
	config->data = NULL;

	functionfs_unbind(ffs);

	if (dev)
		mutex_unlock(&dev->mutex);
}

static void *functionfs_acquire_dev_callback(const char *dev_name)
{
	return 0;
}

static void functionfs_release_dev_callback(struct ffs_data *ffs_data)
{
}

/* ACM */
static char acm_transports[32];	/*enabled ACM ports - "tty[,sdio]"*/
#define MAX_ACM_INSTANCES 4
struct acm_function_config {
	int instances;
	int instances_on;
	struct usb_function *f_acm[MAX_ACM_INSTANCES];
	struct usb_function_instance *f_acm_inst[MAX_ACM_INSTANCES];
};

static int
acm_function_init(struct android_usb_function *f,
		struct usb_composite_dev *cdev)
{
	struct acm_function_config *config;

	config = kzalloc(sizeof(struct acm_function_config), GFP_KERNEL);
	if (!config)
		return -ENOMEM;
	f->config = config;

	return 0;
}

static void acm_function_cleanup(struct android_usb_function *f)
{
	int i;
	struct acm_function_config *config = f->config;

	acm_port_cleanup();
	for (i = 0; i < config->instances_on; i++) {
		usb_put_function(config->f_acm[i]);
		usb_put_function_instance(config->f_acm_inst[i]);
	}
	kfree(f->config);
	f->config = NULL;
}

static int
acm_function_bind_config(struct android_usb_function *f,
		struct usb_configuration *c)
{
	char *name;
	char buf[32], *b;
	int err = -1, i;
	static int acm_initialized, ports;
	struct acm_function_config *config = f->config;

	if (acm_initialized)
		goto bind_config;

	acm_initialized = 1;
	strlcpy(buf, acm_transports, sizeof(buf));
	b = strim(buf);

	while (b) {
		name = strsep(&b, ",");

		if (name) {
			err = acm_init_port(ports, name);
			if (err) {
				pr_err("acm: Cannot open port '%s'", name);
				goto out;
			}
			ports++;
			if (ports >= MAX_ACM_INSTANCES) {
				pr_err("acm: max ports reached '%s'", name);
				goto out;
			}
		}
	}
	err = acm_port_setup(c);
	if (err) {
		pr_err("acm: Cannot setup transports");
		goto out;
	}

	for (i = 0; i < ports; i++) {
		config->f_acm_inst[i] = usb_get_function_instance("acm");
		if (IS_ERR(config->f_acm_inst[i])) {
			err = PTR_ERR(config->f_acm_inst[i]);
			goto err_usb_get_function_instance;
		}
		config->f_acm[i] = usb_get_function(config->f_acm_inst[i]);
		if (IS_ERR(config->f_acm[i])) {
			err = PTR_ERR(config->f_acm[i]);
			goto err_usb_get_function;
		}
	}
	config->instances_on = ports;

bind_config:
	for (i = 0; i < ports; i++) {
		err = usb_add_function(c, config->f_acm[i]);
		if (err) {
			pr_err("Could not bind acm%u config\n", i);
			goto err_usb_add_function;
		}
	}

	return 0;

err_usb_add_function:
	while (i-- > 0)
		usb_remove_function(c, config->f_acm[i]);

	config->instances_on = 0;
	return err;

err_usb_get_function_instance:
	while (i-- > 0) {
		usb_put_function(config->f_acm[i]);
err_usb_get_function:
		usb_put_function_instance(config->f_acm_inst[i]);
	}

out:
	config->instances_on = 0;
	return err;
}

static void acm_function_unbind_config(struct android_usb_function *f,
				       struct usb_configuration *c)
{
	struct acm_function_config *config = f->config;
	config->instances_on = 0;
}

static ssize_t acm_transports_store(
		struct device *device, struct device_attribute *attr,
		const char *buff, size_t size)
{
	strlcpy(acm_transports, buff, sizeof(acm_transports));

	return size;
}

static DEVICE_ATTR(acm_transports, S_IWUSR, NULL, acm_transports_store);
static struct device_attribute *acm_function_attributes[] = {
	&dev_attr_acm_transports,
	NULL
};

static struct android_usb_function acm_function = {
	.name		= "acm",
	.init		= acm_function_init,
	.cleanup	= acm_function_cleanup,
	.bind_config	= acm_function_bind_config,
	.unbind_config	= acm_function_unbind_config,
	.attributes	= acm_function_attributes,
};

/* RMNET_SMD */
static int rmnet_smd_function_bind_config(struct android_usb_function *f,
					  struct usb_configuration *c)
{
	return rmnet_smd_bind_config(c);
}

static struct android_usb_function rmnet_smd_function = {
	.name		= "rmnet_smd",
	.bind_config	= rmnet_smd_function_bind_config,
};

/*rmnet transport string format(per port):"ctrl0,data0,ctrl1,data1..." */
#define MAX_XPORT_STR_LEN 50
static char rmnet_transports[MAX_XPORT_STR_LEN];

/*rmnet transport name string - "rmnet_hsic[,rmnet_hsusb]" */
static char rmnet_xport_names[MAX_XPORT_STR_LEN];

/*qdss transport string format(per port):"bam [, hsic]" */
static char qdss_transports[MAX_XPORT_STR_LEN];

/*qdss transport name string - "qdss_bam [, qdss_hsic]" */
static char qdss_xport_names[MAX_XPORT_STR_LEN];

/*qdss debug interface setting 0: disable   1:enable */
static bool qdss_debug_intf;

static void rmnet_function_cleanup(struct android_usb_function *f)
{
	frmnet_cleanup();
}

static int rmnet_function_bind_config(struct android_usb_function *f,
					 struct usb_configuration *c)
{
	int i;
	int err = 0;
	char *ctrl_name;
	char *data_name;
	char *tname = NULL;
	char buf[MAX_XPORT_STR_LEN], *b;
	char xport_name_buf[MAX_XPORT_STR_LEN], *tb;
	static int rmnet_initialized, ports;

	if (!rmnet_initialized) {
		rmnet_initialized = 1;
		strlcpy(buf, rmnet_transports, sizeof(buf));
		b = strim(buf);

		strlcpy(xport_name_buf, rmnet_xport_names,
				sizeof(xport_name_buf));
		tb = strim(xport_name_buf);

		while (b) {
			ctrl_name = strsep(&b, ",");
			data_name = strsep(&b, ",");
			if (ctrl_name && data_name) {
				if (tb)
					tname = strsep(&tb, ",");
				err = frmnet_init_port(ctrl_name, data_name,
						tname);
				if (err) {
					pr_err("rmnet: Cannot open ctrl port:"
						"'%s' data port:'%s'\n",
						ctrl_name, data_name);
					goto out;
				}
				ports++;
			}
		}

		err = rmnet_gport_setup();
		if (err) {
			pr_err("rmnet: Cannot setup transports");
			goto out;
		}
	}

	for (i = 0; i < ports; i++) {
		err = frmnet_bind_config(c, i);
		if (err) {
			pr_err("Could not bind rmnet%u config\n", i);
			break;
		}
	}
out:
	return err;
}

static void rmnet_function_unbind_config(struct android_usb_function *f,
						struct usb_configuration *c)
{
	frmnet_unbind_config();
}

static ssize_t rmnet_transports_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%s\n", rmnet_transports);
}

static ssize_t rmnet_transports_store(
		struct device *device, struct device_attribute *attr,
		const char *buff, size_t size)
{
	strlcpy(rmnet_transports, buff, sizeof(rmnet_transports));

	return size;
}

static ssize_t rmnet_xport_names_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%s\n", rmnet_xport_names);
}

static ssize_t rmnet_xport_names_store(
		struct device *device, struct device_attribute *attr,
		const char *buff, size_t size)
{
	strlcpy(rmnet_xport_names, buff, sizeof(rmnet_xport_names));

	return size;
}

static struct device_attribute dev_attr_rmnet_transports =
					__ATTR(transports, S_IRUGO | S_IWUSR,
						rmnet_transports_show,
						rmnet_transports_store);

static struct device_attribute dev_attr_rmnet_xport_names =
				__ATTR(transport_names, S_IRUGO | S_IWUSR,
				rmnet_xport_names_show,
				rmnet_xport_names_store);

static struct device_attribute *rmnet_function_attributes[] = {
					&dev_attr_rmnet_transports,
					&dev_attr_rmnet_xport_names,
					NULL };

static struct android_usb_function rmnet_function = {
	.name		= "rmnet",
	.cleanup	= rmnet_function_cleanup,
	.bind_config	= rmnet_function_bind_config,
	.unbind_config	= rmnet_function_unbind_config,
	.attributes	= rmnet_function_attributes,
};

static void gps_function_cleanup(struct android_usb_function *f)
{
	gps_cleanup();
}

static int gps_function_bind_config(struct android_usb_function *f,
					 struct usb_configuration *c)
{
	int err;
	static int gps_initialized;

	if (!gps_initialized) {
		gps_initialized = 1;
		err = gps_init_port();
		if (err) {
			pr_err("gps: Cannot init gps port");
			return err;
		}
	}

	err = gps_gport_setup();
	if (err) {
		pr_err("gps: Cannot setup transports");
		return err;
	}
	err = gps_bind_config(c);
	if (err) {
		pr_err("Could not bind gps config\n");
		return err;
	}

	return 0;
}

static struct android_usb_function gps_function = {
	.name		= "gps",
	.cleanup	= gps_function_cleanup,
	.bind_config	= gps_function_bind_config,
};

/* ncm */
struct ncm_function_config {
	u8      ethaddr[ETH_ALEN];
	struct eth_dev *dev;
};
static int
ncm_function_init(struct android_usb_function *f, struct usb_composite_dev *c)
{
	f->config = kzalloc(sizeof(struct ncm_function_config), GFP_KERNEL);
	if (!f->config)
		return -ENOMEM;

	return 0;
}

static void ncm_function_cleanup(struct android_usb_function *f)
{
	kfree(f->config);
	f->config = NULL;
}

static int
ncm_function_bind_config(struct android_usb_function *f,
				struct usb_configuration *c)
{
	struct ncm_function_config *ncm = f->config;
	int ret;
	struct eth_dev *dev;

	if (!ncm) {
		pr_err("%s: ncm config is null\n", __func__);
		return -EINVAL;
	}

	pr_info("%s MAC: %02X:%02X:%02X:%02X:%02X:%02X\n", __func__,
		ncm->ethaddr[0], ncm->ethaddr[1], ncm->ethaddr[2],
		ncm->ethaddr[3], ncm->ethaddr[4], ncm->ethaddr[5]);

	dev = gether_setup_name(c->cdev->gadget, ncm->ethaddr, "ncm");
	if (IS_ERR(dev)) {
		ret = PTR_ERR(dev);
		pr_err("%s: gether setup failed err:%d\n", __func__, ret);
		return ret;
	}
	ncm->dev = dev;

	ret = ncm_bind_config(c, ncm->ethaddr, dev);
	if (ret) {
		pr_err("%s: ncm bind config failed err:%d", __func__, ret);
		gether_cleanup(dev);
		return ret;
	}

	return ret;
}

static void ncm_function_unbind_config(struct android_usb_function *f,
						struct usb_configuration *c)
{
	struct ncm_function_config *ncm = f->config;
	gether_cleanup(ncm->dev);
}

static ssize_t ncm_ethaddr_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct android_usb_function *f = dev_get_drvdata(dev);
	struct ncm_function_config *ncm = f->config;
	return snprintf(buf, PAGE_SIZE, "%02x:%02x:%02x:%02x:%02x:%02x\n",
		ncm->ethaddr[0], ncm->ethaddr[1], ncm->ethaddr[2],
		ncm->ethaddr[3], ncm->ethaddr[4], ncm->ethaddr[5]);
}

static ssize_t ncm_ethaddr_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct android_usb_function *f = dev_get_drvdata(dev);
	struct ncm_function_config *ncm = f->config;

	if (sscanf(buf, "%02x:%02x:%02x:%02x:%02x:%02x\n",
		    (int *)&ncm->ethaddr[0], (int *)&ncm->ethaddr[1],
		    (int *)&ncm->ethaddr[2], (int *)&ncm->ethaddr[3],
		    (int *)&ncm->ethaddr[4], (int *)&ncm->ethaddr[5]) == 6)
		return size;
	return -EINVAL;
}

static DEVICE_ATTR(ncm_ethaddr, S_IRUGO | S_IWUSR, ncm_ethaddr_show,
					       ncm_ethaddr_store);
static struct device_attribute *ncm_function_attributes[] = {
	&dev_attr_ncm_ethaddr,
	NULL
};

static struct android_usb_function ncm_function = {
	.name		= "ncm",
	.init		= ncm_function_init,
	.cleanup	= ncm_function_cleanup,
	.bind_config	= ncm_function_bind_config,
	.unbind_config	= ncm_function_unbind_config,
	.attributes	= ncm_function_attributes,
};
/* ecm transport string */
static char ecm_transports[MAX_XPORT_STR_LEN];

struct ecm_function_config {
	u8      ethaddr[ETH_ALEN];
	struct eth_dev *dev;
};

static int ecm_function_init(struct android_usb_function *f,
				struct usb_composite_dev *cdev)
{
	f->config = kzalloc(sizeof(struct ecm_function_config), GFP_KERNEL);
	if (!f->config)
		return -ENOMEM;
	return 0;
}

static int ecm_qc_function_init(struct android_usb_function *f,
				struct usb_composite_dev *cdev)
{
	f->config = kzalloc(sizeof(struct ecm_function_config), GFP_KERNEL);
	if (!f->config)
		return -ENOMEM;
	return ecm_qc_init();
}
static void ecm_function_cleanup(struct android_usb_function *f)
{
	kfree(f->config);
	f->config = NULL;
}

static int ecm_qc_function_bind_config(struct android_usb_function *f,
					struct usb_configuration *c)
{
	int ret;
	char *trans;
	struct ecm_function_config *ecm = f->config;

	if (!ecm) {
		pr_err("%s: ecm_pdata\n", __func__);
		return -EINVAL;
	}

	pr_info("%s MAC: %02X:%02X:%02X:%02X:%02X:%02X\n", __func__,
		ecm->ethaddr[0], ecm->ethaddr[1], ecm->ethaddr[2],
		ecm->ethaddr[3], ecm->ethaddr[4], ecm->ethaddr[5]);

	pr_debug("%s: ecm_transport is %s", __func__, ecm_transports);

	trans = strim(ecm_transports);
	if (strcmp("BAM2BAM_IPA", trans)) {
		ret = gether_qc_setup_name(c->cdev->gadget,
						ecm->ethaddr, "ecm");
		if (ret) {
			pr_err("%s: gether_setup failed\n", __func__);
			return ret;
		}
	}

	return ecm_qc_bind_config(c, ecm->ethaddr, trans);
}

static void ecm_qc_function_unbind_config(struct android_usb_function *f,
						struct usb_configuration *c)
{
	char *trans = strim(ecm_transports);

	if (strcmp("BAM2BAM_IPA", trans)) {
		bam_data_flush_workqueue();
		gether_qc_cleanup_name("ecm0");
	}
}

static ssize_t ecm_ethaddr_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct android_usb_function *f = dev_get_drvdata(dev);
	struct ecm_function_config *ecm = f->config;
	return snprintf(buf, PAGE_SIZE, "%02x:%02x:%02x:%02x:%02x:%02x\n",
		ecm->ethaddr[0], ecm->ethaddr[1], ecm->ethaddr[2],
		ecm->ethaddr[3], ecm->ethaddr[4], ecm->ethaddr[5]);
}

static ssize_t ecm_ethaddr_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct android_usb_function *f = dev_get_drvdata(dev);
	struct ecm_function_config *ecm = f->config;

	if (sscanf(buf, "%02x:%02x:%02x:%02x:%02x:%02x\n",
		    (int *)&ecm->ethaddr[0], (int *)&ecm->ethaddr[1],
		    (int *)&ecm->ethaddr[2], (int *)&ecm->ethaddr[3],
		    (int *)&ecm->ethaddr[4], (int *)&ecm->ethaddr[5]) == 6)
		return size;
	return -EINVAL;
}

static DEVICE_ATTR(ecm_ethaddr, S_IRUGO | S_IWUSR, ecm_ethaddr_show,
					       ecm_ethaddr_store);

static ssize_t ecm_transports_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%s\n", ecm_transports);
}

static ssize_t ecm_transports_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	strlcpy(ecm_transports, buf, sizeof(ecm_transports));
	return size;
}

static DEVICE_ATTR(ecm_transports, S_IRUGO | S_IWUSR, ecm_transports_show,
					       ecm_transports_store);

static struct device_attribute *ecm_function_attributes[] = {
	&dev_attr_ecm_transports,
	&dev_attr_ecm_ethaddr,
	NULL
};

static struct android_usb_function ecm_qc_function = {
	.name		= "ecm_qc",
	.init		= ecm_qc_function_init,
	.cleanup	= ecm_function_cleanup,
	.bind_config	= ecm_qc_function_bind_config,
	.unbind_config	= ecm_qc_function_unbind_config,
	.attributes	= ecm_function_attributes,
};

/* MBIM - used with BAM */
#define MAX_MBIM_INSTANCES 1

static int mbim_function_init(struct android_usb_function *f,
					 struct usb_composite_dev *cdev)
{
	return mbim_init(MAX_MBIM_INSTANCES);
}

static void mbim_function_cleanup(struct android_usb_function *f)
{
	fmbim_cleanup();
}


/* mbim transport string */
static char mbim_transports[MAX_XPORT_STR_LEN];

static int mbim_function_bind_config(struct android_usb_function *f,
					  struct usb_configuration *c)
{
	char *trans;

	pr_debug("%s: mbim transport is %s", __func__, mbim_transports);
	trans = strim(mbim_transports);
	return mbim_bind_config(c, 0, trans);
}

static void mbim_function_unbind_config(struct android_usb_function *f,
						struct usb_configuration *c)
{
	char *trans = strim(mbim_transports);

	if (strcmp("BAM2BAM_IPA", trans))
		bam_data_flush_workqueue();
}

static int mbim_function_ctrlrequest(struct android_usb_function *f,
					struct usb_composite_dev *cdev,
					const struct usb_ctrlrequest *c)
{
	return mbim_ctrlrequest(cdev, c);
}

static ssize_t mbim_transports_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%s\n", mbim_transports);
}

static ssize_t mbim_transports_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	strlcpy(mbim_transports, buf, sizeof(mbim_transports));
	return size;
}

static DEVICE_ATTR(mbim_transports, S_IRUGO | S_IWUSR, mbim_transports_show,
				mbim_transports_store);

static ssize_t wMTU_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", ext_mbb_desc.wMTU);
}

static ssize_t wMTU_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	int value;
	if (sscanf(buf, "%d", &value) == 1) {
		if (value < 0 || value > USHRT_MAX)
			pr_err("illegal MTU %d, enter unsigned 16 bits\n",
				value);
		else
			ext_mbb_desc.wMTU = cpu_to_le16(value);
		return size;
	}
	return -EINVAL;
}

static DEVICE_ATTR(wMTU, S_IRUGO | S_IWUSR, wMTU_show,
				wMTU_store);

static struct device_attribute *mbim_function_attributes[] = {
	&dev_attr_mbim_transports,
	&dev_attr_wMTU,
	NULL
};

static struct android_usb_function mbim_function = {
	.name		= "usb_mbim",
	.cleanup	= mbim_function_cleanup,
	.bind_config	= mbim_function_bind_config,
	.unbind_config	= mbim_function_unbind_config,
	.init		= mbim_function_init,
	.ctrlrequest	= mbim_function_ctrlrequest,
	.attributes		= mbim_function_attributes,
};

#ifdef CONFIG_SND_PCM
/* PERIPHERAL AUDIO */
static int audio_function_bind_config(struct android_usb_function *f,
					  struct usb_configuration *c)
{
	return audio_bind_config(c);
}

static struct android_usb_function audio_function = {
	.name		= "audio",
	.bind_config	= audio_function_bind_config,
};
#endif


/* DIAG */
static char diag_clients[32];	    /*enabled DIAG clients- "diag[,diag_mdm]" */
static ssize_t clients_store(
		struct device *device, struct device_attribute *attr,
		const char *buff, size_t size)
{
	strlcpy(diag_clients, buff, sizeof(diag_clients));

	return size;
}

static DEVICE_ATTR(clients, S_IWUSR, NULL, clients_store);
static struct device_attribute *diag_function_attributes[] =
					 { &dev_attr_clients, NULL };

static int diag_function_init(struct android_usb_function *f,
				 struct usb_composite_dev *cdev)
{
	return diag_setup();
}

static void diag_function_cleanup(struct android_usb_function *f)
{
	diag_cleanup();
}

static int diag_function_bind_config(struct android_usb_function *f,
					struct usb_configuration *c)
{
	char *name;
	char buf[32], *b;
	int once = 0, err = -1;
	int (*notify)(uint32_t, const char *);
	struct android_dev *dev = _android_dev;

	strlcpy(buf, diag_clients, sizeof(buf));
	b = strim(buf);

	while (b) {
		notify = NULL;
		name = strsep(&b, ",");
		/* Allow only first diag channel to update pid and serial no */
		if (!once++) {
			if (dev->pdata && dev->pdata->update_pid_and_serial_num)
				notify = dev->pdata->update_pid_and_serial_num;
			else
				notify = usb_diag_update_pid_and_serial_num;
		}

		if (name) {
			err = diag_function_add(c, name, notify);
			if (err)
				pr_err("diag: Cannot open channel '%s'", name);
		}
	}

	return err;
}

static struct android_usb_function diag_function = {
	.name		= "diag",
	.init		= diag_function_init,
	.cleanup	= diag_function_cleanup,
	.bind_config	= diag_function_bind_config,
	.attributes	= diag_function_attributes,
};

/* DEBUG */
static int qdss_function_init(struct android_usb_function *f,
	struct usb_composite_dev *cdev)
{
	return qdss_setup();
}

static void qdss_function_cleanup(struct android_usb_function *f)
{
	qdss_cleanup();
}

static int qdss_init_transports(int *portnum)
{
	char *ts_port;
	char *tname = NULL;
	char *ctrl_name = NULL;
	char buf[MAX_XPORT_STR_LEN], *type;
	char xport_name_buf[MAX_XPORT_STR_LEN], *tn;
	int err = 0;

	strlcpy(buf, qdss_transports, sizeof(buf));
	type = strim(buf);

	strlcpy(xport_name_buf, qdss_xport_names,
			sizeof(xport_name_buf));
	tn = strim(xport_name_buf);

	pr_debug("%s: qdss_debug_intf = %d\n",
		__func__, qdss_debug_intf);

	/*
	 * QDSS function driver is being used for QDSS and DPL
	 * functionality. ctrl_name (i.e. ctrl xport) is optional
	 * whereas data transport name is mandatory to provide
	 * while using QDSS/DPL as part of USB composition.
	 */
	while (type) {
		ctrl_name = strsep(&type, ",");
		ts_port = strsep(&type, ",");
		if (!ts_port) {
			pr_debug("%s:ctrl transport is not provided.\n",
								__func__);
			ts_port = ctrl_name;
			ctrl_name = NULL;
		}

		if (ts_port) {
			if (tn)
				tname = strsep(&tn, ",");

			err = qdss_init_port(
				ctrl_name,
				ts_port,
				tname,
				qdss_debug_intf);

			if (err) {
				pr_err("%s: Cannot open transport port:'%s'\n",
					__func__, ts_port);
				return err;
			}
			(*portnum)++;
		} else {
			pr_err("%s: No data transport name for QDSS port.\n",
								__func__);
			err = -ENODEV;
		}
	}
	return err;
}

static int qdss_function_bind_config(struct android_usb_function *f,
					struct usb_configuration *c)
{
	int i;
	int err = 0;
	static int qdss_initialized = 0, portsnum;

	if (!qdss_initialized) {
		qdss_initialized = 1;

		err = qdss_init_transports(&portsnum);
		if (err) {
			pr_err("qdss: Cannot init transports");
			goto out;
		}

		err = qdss_gport_setup();
		if (err) {
			pr_err("qdss: Cannot setup transports");
			goto out;
		}
	}

	pr_debug("%s: port number is %d\n", __func__, portsnum);

	for (i = 0; i < portsnum; i++) {
		err = qdss_bind_config(c, i);
		if (err) {
			pr_err("Could not bind qdss%u config\n", i);
			break;
		}
	}
out:
	return err;
}

static ssize_t qdss_transports_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%s\n", qdss_transports);
}

static ssize_t qdss_transports_store(
		struct device *device, struct device_attribute *attr,
		const char *buff, size_t size)
{
	strlcpy(qdss_transports, buff, sizeof(qdss_transports));

	return size;
}

static ssize_t qdss_xport_names_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%s\n", qdss_xport_names);
}

static ssize_t qdss_xport_names_store(
		struct device *device, struct device_attribute *attr,
		const char *buff, size_t size)
{
	strlcpy(qdss_xport_names, buff, sizeof(qdss_xport_names));
	return size;
}

static ssize_t qdss_debug_intf_store(
		struct device *device, struct device_attribute *attr,
		const char *buff, size_t size)
{
	strtobool(buff, &qdss_debug_intf);
	return size;
}

static ssize_t qdss_debug_intf_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", qdss_debug_intf);
}

static struct device_attribute dev_attr_qdss_transports =
					__ATTR(transports, S_IRUGO | S_IWUSR,
						qdss_transports_show,
						qdss_transports_store);

static struct device_attribute dev_attr_qdss_xport_names =
				__ATTR(transport_names, S_IRUGO | S_IWUSR,
				qdss_xport_names_show,
				qdss_xport_names_store);

/* 1(enable)/0(disable) the qdss debug interface */
static struct device_attribute dev_attr_qdss_debug_intf =
				__ATTR(debug_intf, S_IRUGO | S_IWUSR,
				qdss_debug_intf_show,
				qdss_debug_intf_store);

static struct device_attribute *qdss_function_attributes[] = {
					&dev_attr_qdss_transports,
					&dev_attr_qdss_xport_names,
					&dev_attr_qdss_debug_intf,
					NULL };

static struct android_usb_function qdss_function = {
	.name		= "qdss",
	.init		= qdss_function_init,
	.cleanup	= qdss_function_cleanup,
	.bind_config	= qdss_function_bind_config,
	.attributes	= qdss_function_attributes,
};

/* SERIAL */
#define MAX_SERIAL_INSTANCES 4
struct serial_function_config {
	int instances_on;
	struct usb_function *f_serial[MAX_SERIAL_INSTANCES];
	struct usb_function_instance *f_serial_inst[MAX_SERIAL_INSTANCES];
};

static char serial_transports[32];	/*enabled FSERIAL ports - "tty[,sdio]"*/
static ssize_t serial_transports_store(
		struct device *device, struct device_attribute *attr,
		const char *buff, size_t size)
{
	strlcpy(serial_transports, buff, sizeof(serial_transports));

	return size;
}

/*enabled FSERIAL transport names - "serial_hsic[,serial_hsusb]"*/
static char serial_xport_names[32];
static ssize_t serial_xport_names_store(
		struct device *device, struct device_attribute *attr,
		const char *buff, size_t size)
{
	strlcpy(serial_xport_names, buff, sizeof(serial_xport_names));

	return size;
}

static ssize_t serial_xport_names_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%s\n", serial_xport_names);
}
static ssize_t serial_modem_is_connected_show(
		struct device *device, struct device_attribute *attr,
		char *buff)
{
	unsigned int is_connected = gserial_is_connected();

	return snprintf(buff, PAGE_SIZE, "%u\n", is_connected);
}

static ssize_t dun_w_softap_enable_show(
		struct device *device, struct device_attribute *attr,
		char *buff)
{
	unsigned int dun_w_softap_enable = gserial_is_dun_w_softap_enabled();

	return snprintf(buff, PAGE_SIZE, "%u\n", dun_w_softap_enable);
}

static ssize_t dun_w_softap_enable_store(
		struct device *device, struct device_attribute *attr,
		const char *buff, size_t size)
{
	unsigned int dun_w_softap_enable;

	sscanf(buff, "%u", &dun_w_softap_enable);

	gserial_dun_w_softap_enable(dun_w_softap_enable);

	return size;
}

static ssize_t dun_w_softap_active_show(
		struct device *device, struct device_attribute *attr,
		char *buff)
{
	unsigned int dun_w_softap_active = gserial_is_dun_w_softap_active();

	return snprintf(buff, PAGE_SIZE, "%u\n", dun_w_softap_active);
}

static DEVICE_ATTR(is_connected_flag, S_IRUGO, serial_modem_is_connected_show,
		NULL);
static DEVICE_ATTR(dun_w_softap_enable, S_IRUGO | S_IWUSR,
	dun_w_softap_enable_show, dun_w_softap_enable_store);
static DEVICE_ATTR(dun_w_softap_active, S_IRUGO, dun_w_softap_active_show,
		NULL);


static DEVICE_ATTR(transports, S_IWUSR, NULL, serial_transports_store);
static struct device_attribute dev_attr_serial_xport_names =
				__ATTR(transport_names, S_IRUGO | S_IWUSR,
				serial_xport_names_show,
				serial_xport_names_store);

static struct device_attribute *serial_function_attributes[] = {
					&dev_attr_transports,
					&dev_attr_serial_xport_names,
					&dev_attr_is_connected_flag,
					&dev_attr_dun_w_softap_enable,
					&dev_attr_dun_w_softap_active,
					NULL };

static int serial_function_init(struct android_usb_function *f,
					struct usb_composite_dev *cdev)
{
	struct serial_function_config *config;

	config = kzalloc(sizeof(struct serial_function_config), GFP_KERNEL);
	if (!config)
		return -ENOMEM;

	f->config = config;

	return 0;
}

static void serial_function_cleanup(struct android_usb_function *f)
{
	int i;
	struct serial_function_config *config = f->config;

	gport_cleanup();
	for (i = 0; i < config->instances_on; i++) {
		usb_put_function(config->f_serial[i]);
		usb_put_function_instance(config->f_serial_inst[i]);
	}
	kfree(f->config);
	f->config = NULL;
}

static int serial_function_bind_config(struct android_usb_function *f,
					struct usb_configuration *c)
{
	char *name, *xport_name = NULL;
	char buf[32], *b, xport_name_buf[32], *tb;
	int err = -1, i;
	static int serial_initialized = 0, ports = 0;
	struct serial_function_config *config = f->config;

	if (serial_initialized)
		goto bind_config;

	serial_initialized = 1;
	strlcpy(buf, serial_transports, sizeof(buf));
	b = strim(buf);

	strlcpy(xport_name_buf, serial_xport_names, sizeof(xport_name_buf));
	tb = strim(xport_name_buf);

	while (b) {
		name = strsep(&b, ",");

		if (name) {
			if (tb)
				xport_name = strsep(&tb, ",");
			err = gserial_init_port(ports, name, xport_name);
			if (err) {
				pr_err("serial: Cannot open port '%s'", name);
				goto out;
			}
			ports++;
			if (ports >= MAX_SERIAL_INSTANCES) {
				pr_err("serial: max ports reached '%s'", name);
				goto out;
			}
		}
	}
	err = gport_setup(c);
	if (err) {
		pr_err("serial: Cannot setup transports");
		goto out;
	}

	for (i = 0; i < ports; i++) {
		config->f_serial_inst[i] = usb_get_function_instance("gser");
		if (IS_ERR(config->f_serial_inst[i])) {
			err = PTR_ERR(config->f_serial_inst[i]);
			goto err_gser_usb_get_function_instance;
		}
		config->f_serial[i] = usb_get_function(config->f_serial_inst[i]);
		if (IS_ERR(config->f_serial[i])) {
			err = PTR_ERR(config->f_serial[i]);
			goto err_gser_usb_get_function;
		}
	}
	config->instances_on = ports;

bind_config:
	for (i = 0; i < ports; i++) {
		err = usb_add_function(c, config->f_serial[i]);
		if (err) {
			pr_err("Could not bind gser%u config\n", i);
			goto err_gser_usb_add_function;
		}
	}
	return 0;

err_gser_usb_add_function:
	while (i-- > 0)
		usb_remove_function(c, config->f_serial[i]);

	return err;

err_gser_usb_get_function_instance:
	while (i-- > 0) {
		usb_put_function(config->f_serial[i]);
err_gser_usb_get_function:
		usb_put_function_instance(config->f_serial_inst[i]);
	}

out:
	return err;
}

static struct android_usb_function serial_function = {
	.name		= "serial",
	.init		= serial_function_init,
	.cleanup	= serial_function_cleanup,
	.bind_config	= serial_function_bind_config,
	.attributes	= serial_function_attributes,
};

/* CCID */
static int ccid_function_init(struct android_usb_function *f,
					struct usb_composite_dev *cdev)
{
	return ccid_setup();
}

static void ccid_function_cleanup(struct android_usb_function *f)
{
	ccid_cleanup();
}

static int ccid_function_bind_config(struct android_usb_function *f,
						struct usb_configuration *c)
{
	return ccid_bind_config(c);
}

static struct android_usb_function ccid_function = {
	.name		= "ccid",
	.init		= ccid_function_init,
	.cleanup	= ccid_function_cleanup,
	.bind_config	= ccid_function_bind_config,
};

/* Charger */
static int charger_function_bind_config(struct android_usb_function *f,
						struct usb_configuration *c)
{
	return charger_bind_config(c);
}

static struct android_usb_function charger_function = {
	.name		= "charging",
	.bind_config	= charger_function_bind_config,
};


static int
mtp_function_init(struct android_usb_function *f,
		struct usb_composite_dev *cdev)
{
	return mtp_setup();
}

static void mtp_function_cleanup(struct android_usb_function *f)
{
	mtp_cleanup();
}

static int
mtp_function_bind_config(struct android_usb_function *f,
		struct usb_configuration *c)
{
	return mtp_bind_config(c, false);
}

static int
ptp_function_init(struct android_usb_function *f,
		struct usb_composite_dev *cdev)
{
	/* nothing to do - initialization is handled by mtp_function_init */
	return 0;
}

static void ptp_function_cleanup(struct android_usb_function *f)
{
	/* nothing to do - cleanup is handled by mtp_function_cleanup */
}

static int
ptp_function_bind_config(struct android_usb_function *f,
		struct usb_configuration *c)
{
	return mtp_bind_config(c, true);
}

static int mtp_function_ctrlrequest(struct android_usb_function *f,
					struct usb_composite_dev *cdev,
					const struct usb_ctrlrequest *c)
{
	return mtp_ctrlrequest(cdev, c);
}

static int ptp_function_ctrlrequest(struct android_usb_function *f,
					struct usb_composite_dev *cdev,
					const struct usb_ctrlrequest *c)
{
	return mtp_ctrlrequest(cdev, c);
}


static struct android_usb_function mtp_function = {
	.name		= "mtp",
	.init		= mtp_function_init,
	.cleanup	= mtp_function_cleanup,
	.bind_config	= mtp_function_bind_config,
	.ctrlrequest	= mtp_function_ctrlrequest,
};

/* PTP function is same as MTP with slightly different interface descriptor */
static struct android_usb_function ptp_function = {
	.name		= "ptp",
	.init		= ptp_function_init,
	.cleanup	= ptp_function_cleanup,
	.bind_config	= ptp_function_bind_config,
	.ctrlrequest	= ptp_function_ctrlrequest,
};

/* rndis transport string */
static char rndis_transports[MAX_XPORT_STR_LEN];

struct rndis_function_config {
	u8      ethaddr[ETH_ALEN];
	u32     vendorID;
	u8      max_pkt_per_xfer;
	u8	pkt_alignment_factor;
	char	manufacturer[256];
	/* "Wireless" RNDIS; auto-detected by Windows */
	bool	wceis;
	struct eth_dev *dev;
};

static int
rndis_function_init(struct android_usb_function *f,
		struct usb_composite_dev *cdev)
{
	f->config = kzalloc(sizeof(struct rndis_function_config), GFP_KERNEL);
	if (!f->config)
		return -ENOMEM;
	return 0;
}

static void rndis_function_cleanup(struct android_usb_function *f)
{
	kfree(f->config);
	f->config = NULL;
}

static int rndis_qc_function_init(struct android_usb_function *f,
					struct usb_composite_dev *cdev)
{
	struct rndis_function_config *rndis;

	rndis = kzalloc(sizeof(struct rndis_function_config), GFP_KERNEL);
	if (!rndis)
		return -ENOMEM;

	rndis->wceis = true;

	f->config = rndis;
	return rndis_qc_init();
}

static void rndis_qc_function_cleanup(struct android_usb_function *f)
{
	rndis_qc_cleanup();
	kfree(f->config);
}

static int
rndis_function_bind_config(struct android_usb_function *f,
		struct usb_configuration *c)
{
	int ret;
	struct eth_dev *dev;
	struct rndis_function_config *rndis = f->config;

	if (!rndis) {
		pr_err("%s: rndis_pdata\n", __func__);
		return -1;
	}

	pr_info("%s MAC: %02X:%02X:%02X:%02X:%02X:%02X\n", __func__,
		rndis->ethaddr[0], rndis->ethaddr[1], rndis->ethaddr[2],
		rndis->ethaddr[3], rndis->ethaddr[4], rndis->ethaddr[5]);

	if (rndis->ethaddr[0])
		dev = gether_setup_name(c->cdev->gadget, NULL, "rndis");
	else
		dev = gether_setup_name(c->cdev->gadget, rndis->ethaddr,
								"rndis");
	if (IS_ERR(dev)) {
		ret = PTR_ERR(dev);
		pr_err("%s: gether_setup failed\n", __func__);
		return ret;
	}
	rndis->dev = dev;

	if (rndis->wceis) {
		/* "Wireless" RNDIS; auto-detected by Windows */
		rndis_iad_descriptor.bFunctionClass =
						USB_CLASS_WIRELESS_CONTROLLER;
		rndis_iad_descriptor.bFunctionSubClass = 0x01;
		rndis_iad_descriptor.bFunctionProtocol = 0x03;
		rndis_control_intf.bInterfaceClass =
						USB_CLASS_WIRELESS_CONTROLLER;
		rndis_control_intf.bInterfaceSubClass =	 0x01;
		rndis_control_intf.bInterfaceProtocol =	 0x03;
	}

	return rndis_bind_config_vendor(c, rndis->ethaddr, rndis->vendorID,
					   rndis->manufacturer, rndis->dev);
}

static int rndis_qc_function_bind_config(struct android_usb_function *f,
					struct usb_configuration *c)
{
	int ret;
	char *trans;
	struct rndis_function_config *rndis = f->config;

	if (!rndis) {
		pr_err("%s: rndis_pdata\n", __func__);
		return -EINVAL;
	}

	pr_info("%s MAC: %02X:%02X:%02X:%02X:%02X:%02X\n", __func__,
		rndis->ethaddr[0], rndis->ethaddr[1], rndis->ethaddr[2],
		rndis->ethaddr[3], rndis->ethaddr[4], rndis->ethaddr[5]);

	pr_debug("%s: rndis_transport is %s", __func__, rndis_transports);

	trans = strim(rndis_transports);
	if (strcmp("BAM2BAM_IPA", trans)) {
		ret = gether_qc_setup_name(c->cdev->gadget,
					rndis->ethaddr, "rndis");
		if (ret) {
			pr_err("%s: gether_setup failed\n", __func__);
			return ret;
		}
	}

	if (rndis->wceis) {
		/* "Wireless" RNDIS; auto-detected by Windows */
		rndis_qc_iad_descriptor.bFunctionClass =
						USB_CLASS_WIRELESS_CONTROLLER;
		rndis_qc_iad_descriptor.bFunctionSubClass = 0x01;
		rndis_qc_iad_descriptor.bFunctionProtocol = 0x03;
		rndis_qc_control_intf.bInterfaceClass =
						USB_CLASS_WIRELESS_CONTROLLER;
		rndis_qc_control_intf.bInterfaceSubClass =	 0x01;
		rndis_qc_control_intf.bInterfaceProtocol =	 0x03;
	}

	return rndis_qc_bind_config_vendor(c, rndis->ethaddr, rndis->vendorID,
			rndis->manufacturer, rndis->max_pkt_per_xfer,
			rndis->pkt_alignment_factor, trans);
}

static void rndis_function_unbind_config(struct android_usb_function *f,
						struct usb_configuration *c)
{
	struct rndis_function_config *rndis = f->config;
	gether_cleanup(rndis->dev);
}

static void rndis_qc_function_unbind_config(struct android_usb_function *f,
						struct usb_configuration *c)
{
	char *trans = strim(rndis_transports);

	if (strcmp("BAM2BAM_IPA", trans)) {
		bam_data_flush_workqueue();
		gether_qc_cleanup_name("rndis0");
	}
}

static ssize_t rndis_manufacturer_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct android_usb_function *f = dev_get_drvdata(dev);
	struct rndis_function_config *config = f->config;

	return snprintf(buf, PAGE_SIZE, "%s\n", config->manufacturer);
}

static ssize_t rndis_manufacturer_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct android_usb_function *f = dev_get_drvdata(dev);
	struct rndis_function_config *config = f->config;

	if (size >= sizeof(config->manufacturer))
		return -EINVAL;

	if (sscanf(buf, "%255s", config->manufacturer) == 1)
		return size;
	return -1;
}

static DEVICE_ATTR(manufacturer, S_IRUGO | S_IWUSR, rndis_manufacturer_show,
						    rndis_manufacturer_store);

static ssize_t rndis_wceis_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct android_usb_function *f = dev_get_drvdata(dev);
	struct rndis_function_config *config = f->config;

	return snprintf(buf, PAGE_SIZE, "%d\n", config->wceis);
}

static ssize_t rndis_wceis_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct android_usb_function *f = dev_get_drvdata(dev);
	struct rndis_function_config *config = f->config;
	int value;

	if (sscanf(buf, "%d", &value) == 1) {
		config->wceis = value;
		return size;
	}
	return -EINVAL;
}

static DEVICE_ATTR(wceis, S_IRUGO | S_IWUSR, rndis_wceis_show,
					     rndis_wceis_store);

static ssize_t rndis_ethaddr_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct android_usb_function *f = dev_get_drvdata(dev);
	struct rndis_function_config *rndis = f->config;

	return snprintf(buf, PAGE_SIZE, "%02x:%02x:%02x:%02x:%02x:%02x\n",
		rndis->ethaddr[0], rndis->ethaddr[1], rndis->ethaddr[2],
		rndis->ethaddr[3], rndis->ethaddr[4], rndis->ethaddr[5]);
}

static ssize_t rndis_ethaddr_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct android_usb_function *f = dev_get_drvdata(dev);
	struct rndis_function_config *rndis = f->config;

	if (sscanf(buf, "%02x:%02x:%02x:%02x:%02x:%02x\n",
		    (int *)&rndis->ethaddr[0], (int *)&rndis->ethaddr[1],
		    (int *)&rndis->ethaddr[2], (int *)&rndis->ethaddr[3],
		    (int *)&rndis->ethaddr[4], (int *)&rndis->ethaddr[5]) == 6)
		return size;
	return -EINVAL;
}

static DEVICE_ATTR(ethaddr, S_IRUGO | S_IWUSR, rndis_ethaddr_show,
					       rndis_ethaddr_store);

static ssize_t rndis_vendorID_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct android_usb_function *f = dev_get_drvdata(dev);
	struct rndis_function_config *config = f->config;

	return snprintf(buf, PAGE_SIZE, "%04x\n", config->vendorID);
}

static ssize_t rndis_vendorID_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct android_usb_function *f = dev_get_drvdata(dev);
	struct rndis_function_config *config = f->config;
	int value;

	if (sscanf(buf, "%04x", &value) == 1) {
		config->vendorID = value;
		return size;
	}
	return -EINVAL;
}

static DEVICE_ATTR(vendorID, S_IRUGO | S_IWUSR, rndis_vendorID_show,
						rndis_vendorID_store);

static ssize_t rndis_max_pkt_per_xfer_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct android_usb_function *f = dev_get_drvdata(dev);
	struct rndis_function_config *config = f->config;
	return snprintf(buf, PAGE_SIZE, "%d\n", config->max_pkt_per_xfer);
}

static ssize_t rndis_max_pkt_per_xfer_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct android_usb_function *f = dev_get_drvdata(dev);
	struct rndis_function_config *config = f->config;
	int value;

	if (sscanf(buf, "%d", &value) == 1) {
		config->max_pkt_per_xfer = value;
		return size;
	}
	return -EINVAL;
}

static DEVICE_ATTR(max_pkt_per_xfer, S_IRUGO | S_IWUSR,
				   rndis_max_pkt_per_xfer_show,
				   rndis_max_pkt_per_xfer_store);
static ssize_t rndis_transports_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%s\n", rndis_transports);
}

static ssize_t rndis_transports_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	strlcpy(rndis_transports, buf, sizeof(rndis_transports));
	return size;
}

static DEVICE_ATTR(rndis_transports, S_IRUGO | S_IWUSR, rndis_transports_show,
					       rndis_transports_store);

static ssize_t rndis_rx_trigger_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	int value;

	if (sscanf(buf, "%d", &value) == 1) {
		rndis_rx_trigger();
		return size;
	}
	return -EINVAL;
}

static DEVICE_ATTR(rx_trigger, S_IWUSR, NULL,
					     rndis_rx_trigger_store);

static ssize_t rndis_pkt_alignment_factor_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct android_usb_function *f = dev_get_drvdata(dev);
	struct rndis_function_config *config = f->config;

	return snprintf(buf, PAGE_SIZE, "%d\n", config->pkt_alignment_factor);
}

static ssize_t rndis_pkt_alignment_factor_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct android_usb_function *f = dev_get_drvdata(dev);
	struct rndis_function_config *config = f->config;
	int value;

	if (sscanf(buf, "%d", &value) == 1) {
		config->pkt_alignment_factor = value;
		return size;
	}

	return -EINVAL;
}

static DEVICE_ATTR(pkt_alignment_factor, S_IRUGO | S_IWUSR,
					rndis_pkt_alignment_factor_show,
					rndis_pkt_alignment_factor_store);

static struct device_attribute *rndis_function_attributes[] = {
	&dev_attr_manufacturer,
	&dev_attr_wceis,
	&dev_attr_ethaddr,
	&dev_attr_vendorID,
	&dev_attr_max_pkt_per_xfer,
	&dev_attr_rndis_transports,
	&dev_attr_rx_trigger,
	&dev_attr_pkt_alignment_factor,
	NULL
};

static struct android_usb_function rndis_function = {
	.name		= "rndis",
	.init		= rndis_function_init,
	.cleanup	= rndis_function_cleanup,
	.bind_config	= rndis_function_bind_config,
	.unbind_config	= rndis_function_unbind_config,
	.attributes	= rndis_function_attributes,
};

static struct android_usb_function rndis_qc_function = {
	.name		= "rndis_qc",
	.init		= rndis_qc_function_init,
	.cleanup	= rndis_qc_function_cleanup,
	.bind_config	= rndis_qc_function_bind_config,
	.unbind_config	= rndis_qc_function_unbind_config,
	.attributes	= rndis_function_attributes,
};

static int ecm_function_bind_config(struct android_usb_function *f,
					struct usb_configuration *c)
{
	int ret;
	struct eth_dev *dev;
	struct ecm_function_config *ecm = f->config;

	if (!ecm) {
		pr_err("%s: ecm_pdata\n", __func__);
		return -EINVAL;
	}

	pr_info("%s MAC: %02X:%02X:%02X:%02X:%02X:%02X\n", __func__,
		ecm->ethaddr[0], ecm->ethaddr[1], ecm->ethaddr[2],
		ecm->ethaddr[3], ecm->ethaddr[4], ecm->ethaddr[5]);

	dev = gether_setup_name(c->cdev->gadget, ecm->ethaddr, "ecm");
	if (IS_ERR(dev)) {
		ret = PTR_ERR(dev);
		pr_err("%s: gether_setup failed\n", __func__);
		return ret;
	}
	ecm->dev = dev;

	ret = ecm_bind_config(c, ecm->ethaddr, dev);
	if (ret) {
		pr_err("%s: ecm_bind_config failed\n", __func__);
		gether_cleanup(dev);
	}
	return ret;
}

static void ecm_function_unbind_config(struct android_usb_function *f,
						struct usb_configuration *c)
{
	struct ecm_function_config *ecm = f->config;
	gether_cleanup(ecm->dev);
}

static struct android_usb_function ecm_function = {
	.name		= "ecm",
	.init		= ecm_function_init,
	.cleanup	= ecm_function_cleanup,
	.bind_config	= ecm_function_bind_config,
	.unbind_config	= ecm_function_unbind_config,
	.attributes	= ecm_function_attributes,
};

#define MAX_LUN_STR_LEN 25
static char lun_info[MAX_LUN_STR_LEN] = {'\0'};
struct mass_storage_function_config {
	struct fsg_config fsg;
	struct fsg_common *common;
};

#define MAX_LUN_NAME 8
static int mass_storage_function_init(struct android_usb_function *f,
					struct usb_composite_dev *cdev)
{
	struct android_dev *dev = _android_dev;
	struct mass_storage_function_config *config;
	struct fsg_common *common;
	int err;
	int i, n;
	char name[FSG_MAX_LUNS][MAX_LUN_NAME];
	u8 uicc_nluns = dev->pdata ? dev->pdata->uicc_nluns : 0;

	config = kzalloc(sizeof(struct mass_storage_function_config),
							GFP_KERNEL);
	if (!config) {
		pr_err("Memory allocation failed.\n");
		return -ENOMEM;
	}

	config->fsg.nluns = 1;
	snprintf(name[0], MAX_LUN_NAME, "lun");
	config->fsg.luns[0].removable = 1;

	if (dev->pdata && dev->pdata->cdrom) {
		config->fsg.luns[config->fsg.nluns].cdrom = 1;
		config->fsg.luns[config->fsg.nluns].ro = 1;
		config->fsg.luns[config->fsg.nluns].removable = 0;
		snprintf(name[config->fsg.nluns], MAX_LUN_NAME, "rom");
		config->fsg.nluns++;
	}

	if (uicc_nluns > FSG_MAX_LUNS - config->fsg.nluns) {
		uicc_nluns = FSG_MAX_LUNS - config->fsg.nluns;
		pr_debug("limiting uicc luns to %d\n", uicc_nluns);
	}

	for (i = 0; i < uicc_nluns; i++) {
		n = config->fsg.nluns;
		snprintf(name[n], MAX_LUN_NAME, "uicc%d", i);
		config->fsg.luns[n].removable = 1;
		config->fsg.nluns++;
	}

	common = fsg_common_init(NULL, cdev, &config->fsg);
	if (IS_ERR(common)) {
		kfree(config);
		return PTR_ERR(common);
	}

	for (i = 0; i < config->fsg.nluns; i++) {
		err = sysfs_create_link(&f->dev->kobj,
					&common->luns[i].dev.kobj,
					name[i]);
		if (err)
			goto error;
	}

	config->common = common;
	f->config = config;
	return 0;
error:
	for (; i > 0; i--)
		sysfs_remove_link(&f->dev->kobj, name[i-1]);

	fsg_common_release(&common->ref);
	kfree(config);
	return err;
}

static int mass_storage_lun_init(struct android_usb_function *f,
						char *lun_info)
{
	struct mass_storage_function_config *config = f->config;
	bool inc_lun = true;
	int i = config->fsg.nluns, index, length;
	static int number_of_luns;

	length = strlen(lun_info);
	if (!length) {
		pr_err("LUN_INFO is null.\n");
		return -EINVAL;
	}

	index = i + number_of_luns;
	if (index >= FSG_MAX_LUNS) {
		pr_err("Number of LUNs exceed the limit.\n");
		return -EINVAL;
	}

	if (!strcmp(lun_info, "disk")) {
		config->fsg.luns[index].removable = 1;
	} else if (!strcmp(lun_info, "rom")) {
		config->fsg.luns[index].cdrom = 1;
		config->fsg.luns[index].removable = 0;
		config->fsg.luns[index].ro = 1;
	} else {
		pr_err("Invalid LUN info.\n");
		inc_lun = false;
		return -EINVAL;
	}

	if (inc_lun)
		number_of_luns++;

	pr_debug("number_of_luns:%d\n", number_of_luns);
	return number_of_luns;
}

static void mass_storage_function_cleanup(struct android_usb_function *f)
{
	kfree(f->config);
	f->config = NULL;
}

static void mass_storage_function_enable(struct android_usb_function *f)
{
	struct usb_composite_dev *cdev = _android_dev->cdev;
	struct mass_storage_function_config *config = f->config;
	struct fsg_common *common = config->common;
	char *lun_type;
	int i, err, prev_nluns;
	char buf[MAX_LUN_STR_LEN], *b;
	int number_of_luns = 0;
	char buf1[5];
	char *lun_name = buf1;
	static int msc_initialized;

	if (msc_initialized)
		return;

	prev_nluns = config->fsg.nluns;

	if (lun_info[0] != '\0') {
		strlcpy(buf, lun_info, sizeof(buf));
		b = strim(buf);

		while (b) {
			lun_type = strsep(&b, ",");
			if (lun_type)
				number_of_luns =
					mass_storage_lun_init(f, lun_type);
				if (number_of_luns <= 0)
					return;
		}
	} else {
		pr_debug("No extra msc lun required.\n");
		return;
	}

	err = fsg_add_lun(common, cdev, &config->fsg, number_of_luns);
	if (err) {
		pr_err("Failed adding LUN.\n");
		return;
	}

	pr_debug("fsg.nluns:%d\n", config->fsg.nluns);
	for (i = prev_nluns; i < config->fsg.nluns; i++) {
		snprintf(lun_name, sizeof(buf), "lun%d", (i-prev_nluns));
		pr_debug("sysfs: LUN name:%s\n", lun_name);
		err = sysfs_create_link(&f->dev->kobj,
			&common->luns[i].dev.kobj, lun_name);
		if (err)
			pr_err("sysfs file creation failed: lun%d err:%d\n",
							(i-prev_nluns), err);
	}

	msc_initialized = 1;
}

static int mass_storage_function_bind_config(struct android_usb_function *f,
						struct usb_configuration *c)
{
	struct mass_storage_function_config *config = f->config;
	int ret;

	ret = fsg_bind_config(c->cdev, c, config->common);
	if (ret)
		pr_err("fsg_bind_config failed. ret:%x\n", ret);

	return ret;
}

static ssize_t mass_storage_inquiry_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct android_usb_function *f = dev_get_drvdata(dev);
	struct mass_storage_function_config *config = f->config;
	return snprintf(buf, PAGE_SIZE, "%s\n", config->common->inquiry_string);
}

static ssize_t mass_storage_inquiry_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct android_usb_function *f = dev_get_drvdata(dev);
	struct mass_storage_function_config *config = f->config;
	if (size >= sizeof(config->common->inquiry_string))
		return -EINVAL;
	if (sscanf(buf, "%28s", config->common->inquiry_string) != 1)
		return -EINVAL;
	return size;
}

static DEVICE_ATTR(inquiry_string, S_IRUGO | S_IWUSR,
					mass_storage_inquiry_show,
					mass_storage_inquiry_store);

static ssize_t mass_storage_lun_info_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{

	return snprintf(buf, PAGE_SIZE, "%s\n", lun_info);
}

static ssize_t mass_storage_lun_info_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	strlcpy(lun_info, buf, sizeof(lun_info));
	return size;
}

static DEVICE_ATTR(luns, S_IRUGO | S_IWUSR,
				mass_storage_lun_info_show,
				mass_storage_lun_info_store);

static struct device_attribute *mass_storage_function_attributes[] = {
	&dev_attr_inquiry_string,
	&dev_attr_luns,
	NULL
};

static struct android_usb_function mass_storage_function = {
	.name		= "mass_storage",
	.init		= mass_storage_function_init,
	.cleanup	= mass_storage_function_cleanup,
	.bind_config	= mass_storage_function_bind_config,
	.attributes	= mass_storage_function_attributes,
	.enable		= mass_storage_function_enable,
};


static int accessory_function_init(struct android_usb_function *f,
					struct usb_composite_dev *cdev)
{
	return acc_setup();
}

static void accessory_function_cleanup(struct android_usb_function *f)
{
	acc_cleanup();
}

static int accessory_function_bind_config(struct android_usb_function *f,
						struct usb_configuration *c)
{
	return acc_bind_config(c);
}

static int accessory_function_ctrlrequest(struct android_usb_function *f,
						struct usb_composite_dev *cdev,
						const struct usb_ctrlrequest *c)
{
	return acc_ctrlrequest(cdev, c);
}

static struct android_usb_function accessory_function = {
	.name		= "accessory",
	.init		= accessory_function_init,
	.cleanup	= accessory_function_cleanup,
	.bind_config	= accessory_function_bind_config,
	.ctrlrequest	= accessory_function_ctrlrequest,
};

#ifdef CONFIG_SND_PCM
static int audio_source_function_init(struct android_usb_function *f,
			struct usb_composite_dev *cdev)
{
	struct audio_source_config *config;

	config = kzalloc(sizeof(struct audio_source_config), GFP_KERNEL);
	if (!config)
		return -ENOMEM;
	config->card = -1;
	config->device = -1;
	f->config = config;
	return 0;
}

static void audio_source_function_cleanup(struct android_usb_function *f)
{
	kfree(f->config);
}

static int audio_source_function_bind_config(struct android_usb_function *f,
						struct usb_configuration *c)
{
	struct audio_source_config *config = f->config;

	return audio_source_bind_config(c, config);
}

static void audio_source_function_unbind_config(struct android_usb_function *f,
						struct usb_configuration *c)
{
	struct audio_source_config *config = f->config;

	config->card = -1;
	config->device = -1;
}

static ssize_t audio_source_pcm_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct android_usb_function *f = dev_get_drvdata(dev);
	struct audio_source_config *config = f->config;

	/* print PCM card and device numbers */
	return sprintf(buf, "%d %d\n", config->card, config->device);
}

static DEVICE_ATTR(pcm, S_IRUGO, audio_source_pcm_show, NULL);

static struct device_attribute *audio_source_function_attributes[] = {
	&dev_attr_pcm,
	NULL
};

static struct android_usb_function audio_source_function = {
	.name		= "audio_source",
	.init		= audio_source_function_init,
	.cleanup	= audio_source_function_cleanup,
	.bind_config	= audio_source_function_bind_config,
	.unbind_config	= audio_source_function_unbind_config,
	.attributes	= audio_source_function_attributes,
};
#endif

static int android_uasp_connect_cb(bool connect)
{
	/*
	 * TODO
	 * We may have to disable gadget till UASP configfs nodes
	 * are configured which includes mapping LUN with the
	 * backing file. It is a fundamental difference between
	 * f_mass_storage and f_tcp. That means UASP can not be
	 * in default composition.
	 *
	 * For now, assume that UASP configfs nodes are configured
	 * before enabling android gadget. Or cable should be
	 * reconnected after mapping the LUN.
	 *
	 * Also consider making UASP to respond to Host requests when
	 * Lun is not mapped.
	 */
	pr_debug("UASP %s\n", connect ? "connect" : "disconnect");

	return 0;
}

static int uasp_function_init(struct android_usb_function *f,
					struct usb_composite_dev *cdev)
{
	return f_tcm_init(&android_uasp_connect_cb);
}

static void uasp_function_cleanup(struct android_usb_function *f)
{
	f_tcm_exit();
}

static int uasp_function_bind_config(struct android_usb_function *f,
						struct usb_configuration *c)
{
	return tcm_bind_config(c);
}

static struct android_usb_function uasp_function = {
	.name		= "uasp",
	.init		= uasp_function_init,
	.cleanup	= uasp_function_cleanup,
	.bind_config	= uasp_function_bind_config,
};

static struct android_usb_function *supported_functions[] = {
	&ffs_function,
	&mbim_function,
	&ecm_qc_function,
#ifdef CONFIG_SND_PCM
	&audio_function,
#endif
	&rmnet_smd_function,
	&rmnet_function,
	&gps_function,
	&diag_function,
	&qdss_function,
	&serial_function,
	&ccid_function,
	&acm_function,
	&mtp_function,
	&ptp_function,
	&rndis_function,
	&rndis_qc_function,
	&ecm_function,
	&ncm_function,
	&mass_storage_function,
	&accessory_function,
#ifdef CONFIG_SND_PCM
	&audio_source_function,
#endif
	&uasp_function,
	&charger_function,
	NULL
};

static void android_cleanup_functions(struct android_usb_function **functions)
{
	struct android_usb_function *f;
	struct device_attribute **attrs;
	struct device_attribute *attr;

	while (*functions) {
		f = *functions++;

		if (f->dev) {
			device_destroy(android_class, f->dev->devt);
			kfree(f->dev_name);
		} else
			continue;

		if (f->cleanup)
			f->cleanup(f);

		attrs = f->attributes;
		if (attrs) {
			while ((attr = *attrs++))
				device_remove_file(f->dev, attr);
		}
	}
}

static int android_init_functions(struct android_usb_function **functions,
				  struct usb_composite_dev *cdev)
{
	struct android_dev *dev = _android_dev;
	struct android_usb_function *f;
	struct device_attribute **attrs;
	struct device_attribute *attr;
	int err = 0;
	int index = 0;

	for (; (f = *functions++); index++) {
		f->dev_name = kasprintf(GFP_KERNEL, "f_%s", f->name);
		f->dev = device_create(android_class, dev->dev,
				MKDEV(0, index), f, f->dev_name);
		if (IS_ERR(f->dev)) {
			pr_err("%s: Failed to create dev %s", __func__,
							f->dev_name);
			err = PTR_ERR(f->dev);
			goto err_create;
		}

		if (f->init) {
			err = f->init(f, cdev);
			if (err) {
				pr_err("%s: Failed to init %s", __func__,
								f->name);
				goto err_out;
			}
		}

		attrs = f->attributes;
		if (attrs) {
			while ((attr = *attrs++) && !err)
				err = device_create_file(f->dev, attr);
		}
		if (err) {
			pr_err("%s: Failed to create function %s attributes",
					__func__, f->name);
			goto err_out;
		}
	}
	return 0;

err_out:
	device_destroy(android_class, f->dev->devt);
err_create:
	kfree(f->dev_name);
	return err;
}

static int
android_bind_enabled_functions(struct android_dev *dev,
			       struct usb_configuration *c)
{
	struct android_usb_function *f;
	int ret;

	list_for_each_entry(f, &dev->enabled_functions, enabled_list) {
		ret = f->bind_config(f, c);
		if (ret) {
			pr_err("%s: %s failed", __func__, f->name);
			return ret;
		}
	}
	return 0;
}

static void
android_unbind_enabled_functions(struct android_dev *dev,
			       struct usb_configuration *c)
{
	struct android_usb_function *f;

	list_for_each_entry(f, &dev->enabled_functions, enabled_list) {
		if (f->unbind_config)
			f->unbind_config(f, c);
	}
}

static int android_enable_function(struct android_dev *dev, char *name)
{
	struct android_usb_function **functions = dev->functions;
	struct android_usb_function *f;
	while ((f = *functions++)) {
		if (!strcmp(name, f->name)) {
			list_add_tail(&f->enabled_list,
						&dev->enabled_functions);
			return 0;
		}
	}
	return -EINVAL;
}

/*-------------------------------------------------------------------------*/
/* /sys/class/android_usb/android%d/ interface */

static ssize_t remote_wakeup_show(struct device *pdev,
		struct device_attribute *attr, char *buf)
{
	struct android_dev *dev = dev_get_drvdata(pdev);
	struct android_configuration *conf;

	/*
	 * Show the wakeup attribute of the first configuration,
	 * since all configurations have the same wakeup attribute
	 */
	if (dev->configs_num == 0)
		return 0;
	conf = list_entry(dev->configs.next,
			  struct android_configuration,
			  list_item);

	return snprintf(buf, PAGE_SIZE, "%d\n",
			!!(conf->usb_config.bmAttributes &
				USB_CONFIG_ATT_WAKEUP));
}

static ssize_t remote_wakeup_store(struct device *pdev,
		struct device_attribute *attr, const char *buff, size_t size)
{
	struct android_dev *dev = dev_get_drvdata(pdev);
	struct android_configuration *conf;
	int enable = 0;

	sscanf(buff, "%d", &enable);

	pr_debug("android_usb: %s remote wakeup\n",
			enable ? "enabling" : "disabling");

	list_for_each_entry(conf, &dev->configs, list_item)
		if (enable)
			conf->usb_config.bmAttributes |=
					USB_CONFIG_ATT_WAKEUP;
		else
			conf->usb_config.bmAttributes &=
					~USB_CONFIG_ATT_WAKEUP;

	return size;
}

static ssize_t
functions_show(struct device *pdev, struct device_attribute *attr, char *buf)
{
	struct android_dev *dev = dev_get_drvdata(pdev);
	struct android_usb_function *f;
	char *buff = buf;

	mutex_lock(&dev->mutex);

	list_for_each_entry(f, &dev->enabled_functions, enabled_list)
		buff += sprintf(buff, "%s,", f->name);

	mutex_unlock(&dev->mutex);

	if (buff != buf)
		*(buff-1) = '\n';
	return buff - buf;
}

static ssize_t
functions_store(struct device *pdev, struct device_attribute *attr,
			       const char *buff, size_t size)
{
	struct android_dev *dev = dev_get_drvdata(pdev);
	char *name;
	char buf[256], *b;
	char aliases[256], *a;
	int err;
	int is_ffs;
	int ffs_enabled = 0;

	mutex_lock(&dev->mutex);

	if (dev->enabled) {
		mutex_unlock(&dev->mutex);
		return -EBUSY;
	}

	INIT_LIST_HEAD(&dev->enabled_functions);

	strlcpy(buf, buff, sizeof(buf));
	b = strim(buf);

	while (b) {
		name = strsep(&b, ",");
		if (!name)
			continue;

		is_ffs = 0;
		strlcpy(aliases, dev->ffs_aliases, sizeof(aliases));
		a = aliases;

		while (a) {
			char *alias = strsep(&a, ",");
			if (alias && !strcmp(name, alias)) {
				is_ffs = 1;
				break;
			}
		}

		if (is_ffs) {
			if (ffs_enabled)
				continue;
			err = android_enable_function(dev, "ffs");
			if (err)
				pr_err("android_usb: Cannot enable ffs (%d)",
									err);
			else
				ffs_enabled = 1;
			continue;
		}

		err = android_enable_function(dev, name);
		if (err)
			pr_err("android_usb: Cannot enable '%s' (%d)",
							   name, err);
	}

	mutex_unlock(&dev->mutex);

	return size;
}

static ssize_t enable_show(struct device *pdev, struct device_attribute *attr,
			   char *buf)
{
	struct android_dev *dev = dev_get_drvdata(pdev);

	return snprintf(buf, PAGE_SIZE, "%d\n", dev->enabled);
}

static ssize_t enable_store(struct device *pdev, struct device_attribute *attr,
			    const char *buff, size_t size)
{
	struct android_dev *dev = dev_get_drvdata(pdev);
	struct usb_composite_dev *cdev = dev->cdev;
	struct android_usb_function *f;
	int enabled = 0;


	if (!cdev)
		return -ENODEV;

	mutex_lock(&dev->mutex);

	sscanf(buff, "%d", &enabled);
	if (enabled && !dev->enabled) {
		/*
		 * Update values in composite driver's copy of
		 * device descriptor.
		 */
		cdev->desc.idVendor = device_desc.idVendor;
		cdev->desc.idProduct = device_desc.idProduct;
		cdev->desc.bcdDevice = device_desc.bcdDevice;
		cdev->desc.bDeviceClass = device_desc.bDeviceClass;
		cdev->desc.bDeviceSubClass = device_desc.bDeviceSubClass;
		cdev->desc.bDeviceProtocol = device_desc.bDeviceProtocol;
		list_for_each_entry(f, &dev->enabled_functions, enabled_list) {
			if (f->enable)
				f->enable(f);
		}
		android_enable(dev);
		dev->enabled = true;
	} else if (!enabled && dev->enabled) {
		android_disable(dev);
		list_for_each_entry(f, &dev->enabled_functions, enabled_list) {
			if (f->disable)
				f->disable(f);
		}
		dev->enabled = false;
	} else {
		pr_err("android_usb: already %s\n",
				dev->enabled ? "enabled" : "disabled");
	}

	mutex_unlock(&dev->mutex);

	return size;
}

static ssize_t pm_qos_show(struct device *pdev,
			   struct device_attribute *attr, char *buf)
{
	struct android_dev *dev = dev_get_drvdata(pdev);

	return snprintf(buf, PAGE_SIZE, "%s\n", dev->pm_qos);
}

static ssize_t pm_qos_store(struct device *pdev,
			   struct device_attribute *attr,
			   const char *buff, size_t size)
{
	struct android_dev *dev = dev_get_drvdata(pdev);

	strlcpy(dev->pm_qos, buff, sizeof(dev->pm_qos));

	return size;
}

static ssize_t pm_qos_state_show(struct device *pdev,
			struct device_attribute *attr, char *buf)
{
	struct android_dev *dev = dev_get_drvdata(pdev);

	return snprintf(buf, PAGE_SIZE, "%s\n",
				pm_qos_to_string(dev->curr_pm_qos_state));
}

static ssize_t state_show(struct device *pdev, struct device_attribute *attr,
			   char *buf)
{
	struct android_dev *dev = dev_get_drvdata(pdev);
	struct usb_composite_dev *cdev = dev->cdev;
	char *state = "DISCONNECTED";
	unsigned long flags;

	if (!cdev)
		goto out;

	spin_lock_irqsave(&cdev->lock, flags);
	if (cdev->config)
		state = "CONFIGURED";
	else if (dev->connected)
		state = "CONNECTED";
	spin_unlock_irqrestore(&cdev->lock, flags);
out:
	return snprintf(buf, PAGE_SIZE, "%s\n", state);
}

#define ANDROID_DEV_ATTR(field, format_string)				\
static ssize_t								\
field ## _show(struct device *pdev, struct device_attribute *attr,	\
		char *buf)						\
{									\
	struct android_dev *dev = dev_get_drvdata(pdev);		\
									\
	return snprintf(buf, PAGE_SIZE,					\
			format_string, dev->field);			\
}									\
static ssize_t								\
field ## _store(struct device *pdev, struct device_attribute *attr,	\
		const char *buf, size_t size)				\
{									\
	unsigned value;							\
	struct android_dev *dev = dev_get_drvdata(pdev);		\
									\
	if (sscanf(buf, format_string, &value) == 1) {			\
		dev->field = value;					\
		return size;						\
	}								\
	return -EINVAL;							\
}									\
static DEVICE_ATTR(field, S_IRUGO | S_IWUSR, field ## _show, field ## _store);

#define DESCRIPTOR_ATTR(field, format_string)				\
static ssize_t								\
field ## _show(struct device *dev, struct device_attribute *attr,	\
		char *buf)						\
{									\
	return snprintf(buf, PAGE_SIZE,					\
			format_string, device_desc.field);		\
}									\
static ssize_t								\
field ## _store(struct device *dev, struct device_attribute *attr,	\
		const char *buf, size_t size)				\
{									\
	int value;							\
	if (sscanf(buf, format_string, &value) == 1) {			\
		device_desc.field = value;				\
		return size;						\
	}								\
	return -1;							\
}									\
static DEVICE_ATTR(field, S_IRUGO | S_IWUSR, field ## _show, field ## _store);

#define DESCRIPTOR_STRING_ATTR(field, buffer)				\
static ssize_t								\
field ## _show(struct device *dev, struct device_attribute *attr,	\
		char *buf)						\
{									\
	return snprintf(buf, PAGE_SIZE, "%s", buffer);			\
}									\
static ssize_t								\
field ## _store(struct device *dev, struct device_attribute *attr,	\
		const char *buf, size_t size)				\
{									\
	if (size >= sizeof(buffer))					\
		return -EINVAL;						\
	strlcpy(buffer, buf, sizeof(buffer));				\
	strim(buffer);							\
	return size;							\
}									\
static DEVICE_ATTR(field, S_IRUGO | S_IWUSR, field ## _show, field ## _store);


DESCRIPTOR_ATTR(idVendor, "%04x\n")
DESCRIPTOR_ATTR(idProduct, "%04x\n")
DESCRIPTOR_ATTR(bcdDevice, "%04x\n")
DESCRIPTOR_ATTR(bDeviceClass, "%d\n")
DESCRIPTOR_ATTR(bDeviceSubClass, "%d\n")
DESCRIPTOR_ATTR(bDeviceProtocol, "%d\n")
DESCRIPTOR_STRING_ATTR(iManufacturer, manufacturer_string)
DESCRIPTOR_STRING_ATTR(iProduct, product_string)
DESCRIPTOR_STRING_ATTR(iSerial, serial_string)

static DEVICE_ATTR(functions, S_IRUGO | S_IWUSR, functions_show,
						 functions_store);
static DEVICE_ATTR(enable, S_IRUGO | S_IWUSR, enable_show, enable_store);

static DEVICE_ATTR(pm_qos, S_IRUGO | S_IWUSR, pm_qos_show, pm_qos_store);
static DEVICE_ATTR(pm_qos_state, S_IRUGO, pm_qos_state_show, NULL);
ANDROID_DEV_ATTR(up_pm_qos_sample_sec, "%u\n");
ANDROID_DEV_ATTR(down_pm_qos_sample_sec, "%u\n");
ANDROID_DEV_ATTR(up_pm_qos_threshold, "%u\n");
ANDROID_DEV_ATTR(down_pm_qos_threshold, "%u\n");
ANDROID_DEV_ATTR(idle_pc_rpm_no_int_secs, "%u\n");

static DEVICE_ATTR(state, S_IRUGO, state_show, NULL);
static DEVICE_ATTR(remote_wakeup, S_IRUGO | S_IWUSR,
		remote_wakeup_show, remote_wakeup_store);

static struct device_attribute *android_usb_attributes[] = {
	&dev_attr_idVendor,
	&dev_attr_idProduct,
	&dev_attr_bcdDevice,
	&dev_attr_bDeviceClass,
	&dev_attr_bDeviceSubClass,
	&dev_attr_bDeviceProtocol,
	&dev_attr_iManufacturer,
	&dev_attr_iProduct,
	&dev_attr_iSerial,
	&dev_attr_functions,
	&dev_attr_enable,
	&dev_attr_pm_qos,
	&dev_attr_up_pm_qos_sample_sec,
	&dev_attr_down_pm_qos_sample_sec,
	&dev_attr_up_pm_qos_threshold,
	&dev_attr_down_pm_qos_threshold,
	&dev_attr_idle_pc_rpm_no_int_secs,
	&dev_attr_pm_qos_state,
	&dev_attr_state,
	&dev_attr_remote_wakeup,
	NULL
};

/*-------------------------------------------------------------------------*/
/* Composite driver */

static int android_bind_config(struct usb_configuration *c)
{
	struct android_dev *dev = _android_dev;
	int ret = 0;

	ret = android_bind_enabled_functions(dev, c);
	if (ret)
		return ret;

	return 0;
}

static void android_unbind_config(struct usb_configuration *c)
{
	struct android_dev *dev = _android_dev;

	if (c->cdev->gadget->streaming_enabled) {
		c->cdev->gadget->streaming_enabled = false;
		pr_debug("setting streaming_enabled to false.\n");
	}
	android_unbind_enabled_functions(dev, c);
}

static int android_bind(struct usb_composite_dev *cdev)
{
	struct android_dev *dev = _android_dev;
	struct usb_gadget	*gadget = cdev->gadget;
	int			id, ret;

	/*
	 * Start disconnected. Userspace will connect the gadget once
	 * it is done configuring the functions.
	 */
	usb_gadget_disconnect(gadget);

	ret = android_init_functions(dev->functions, cdev);
	if (ret)
		return ret;

	/* Allocate string descriptor numbers ... note that string
	 * contents can be overridden by the composite_dev glue.
	 */
	id = usb_string_id(cdev);
	if (id < 0)
		return id;
	strings_dev[STRING_MANUFACTURER_IDX].id = id;
	device_desc.iManufacturer = id;

	id = usb_string_id(cdev);
	if (id < 0)
		return id;
	strings_dev[STRING_PRODUCT_IDX].id = id;
	device_desc.iProduct = id;

	/* Default strings - should be updated by userspace */
	strlcpy(manufacturer_string, "Android",
		sizeof(manufacturer_string) - 1);
	strlcpy(product_string, "Android", sizeof(product_string) - 1);
	strlcpy(serial_string, "0123456789ABCDEF", sizeof(serial_string) - 1);

	id = usb_string_id(cdev);
	if (id < 0)
		return id;
	strings_dev[STRING_SERIAL_IDX].id = id;
	device_desc.iSerialNumber = id;

	usb_gadget_set_selfpowered(gadget);
	dev->cdev = cdev;

	return 0;
}

static int android_usb_unbind(struct usb_composite_dev *cdev)
{
	struct android_dev *dev = _android_dev;

	manufacturer_string[0] = '\0';
	product_string[0] = '\0';
	serial_string[0] = '0';
	cancel_work_sync(&dev->work);
	cancel_delayed_work_sync(&dev->pm_qos_work);
	android_cleanup_functions(dev->functions);
	return 0;
}

/* HACK: android needs to override setup for accessory to work */
static int (*composite_setup_func)(struct usb_gadget *gadget, const struct usb_ctrlrequest *c);
static void (*composite_suspend_func)(struct usb_gadget *gadget);
static void (*composite_resume_func)(struct usb_gadget *gadget);

static int
android_setup(struct usb_gadget *gadget, const struct usb_ctrlrequest *c)
{
	struct android_dev		*dev = _android_dev;
	struct usb_composite_dev	*cdev = get_gadget_data(gadget);
	struct usb_request		*req = cdev->req;
	struct android_usb_function	*f;
	int value = -EOPNOTSUPP;
	unsigned long flags;
	bool do_work = false;
	bool prev_configured = false;

	req->zero = 0;
	req->length = 0;
	gadget->ep0->driver_data = cdev;

	list_for_each_entry(f, &dev->enabled_functions, enabled_list) {
		if (f->ctrlrequest) {
			value = f->ctrlrequest(f, cdev, c);
			if (value >= 0)
				break;
		}
	}

	/*
	 * skip the  work when 2nd set config arrives
	 * with same value from the host.
	 */
	if (cdev->config)
		prev_configured = true;
	/* Special case the accessory function.
	 * It needs to handle control requests before it is enabled.
	 */
	if (value < 0)
		value = acc_ctrlrequest(cdev, c);

	if (value < 0)
		value = composite_setup_func(gadget, c);

	spin_lock_irqsave(&cdev->lock, flags);
	if (!dev->connected) {
		dev->connected = 1;
		do_work = true;
	} else if (c->bRequest == USB_REQ_SET_CONFIGURATION &&
						cdev->config) {
		if (!prev_configured)
			do_work = true;
	}
	spin_unlock_irqrestore(&cdev->lock, flags);
	if (do_work)
		schedule_work(&dev->work);
	return value;
}

static void android_disconnect(struct usb_composite_dev *cdev)
{
	struct android_dev *dev = _android_dev;

	/* accessory HID support can be active while the
	   accessory function is not actually enabled,
	   so we need to inform it when we are disconnected.
	 */
	acc_disconnect();

	dev->connected = 0;
	schedule_work(&dev->work);
}

static struct usb_composite_driver android_usb_driver = {
	.name		= "android_usb",
	.dev		= &device_desc,
	.strings	= dev_strings,
	.bind		= android_bind,
	.unbind		= android_usb_unbind,
	.disconnect	= android_disconnect,
	.max_speed	= USB_SPEED_SUPER
};

static void android_suspend(struct usb_gadget *gadget)
{
	struct usb_composite_dev *cdev = get_gadget_data(gadget);
	struct android_dev *dev = _android_dev;
	unsigned long flags;

	spin_lock_irqsave(&cdev->lock, flags);
	if (!dev->suspended) {
		dev->suspended = 1;
		schedule_work(&dev->work);
	}
	spin_unlock_irqrestore(&cdev->lock, flags);

	composite_suspend_func(gadget);
}

static void android_resume(struct usb_gadget *gadget)
{
	struct usb_composite_dev *cdev = get_gadget_data(gadget);
	struct android_dev *dev = _android_dev;
	unsigned long flags;

	spin_lock_irqsave(&cdev->lock, flags);
	if (dev->suspended) {
		dev->suspended = 0;
		schedule_work(&dev->work);
	}
	spin_unlock_irqrestore(&cdev->lock, flags);

	composite_resume_func(gadget);
}

static int android_create_device(struct android_dev *dev)
{
	struct device_attribute **attrs = android_usb_attributes;
	struct device_attribute *attr;
	int err;

	dev->dev = device_create(android_class, NULL,
					MKDEV(0, 0), NULL, "android0");
	if (IS_ERR(dev->dev))
		return PTR_ERR(dev->dev);

	dev_set_drvdata(dev->dev, dev);

	while ((attr = *attrs++)) {
		err = device_create_file(dev->dev, attr);
		if (err) {
			device_destroy(android_class, dev->dev->devt);
			return err;
		}
	}
	return 0;
}


static int usb_diag_update_pid_and_serial_num(u32 pid, const char *snum)
{
	struct dload_struct local_diag_dload = { 0 };
	int *src, *dst, i;

	if (!diag_dload) {
		pr_debug("%s: unable to update PID and serial_no\n", __func__);
		return -ENODEV;
	}

	pr_debug("%s: dload:%p pid:%x serial_num:%s\n",
				__func__, diag_dload, pid, snum);

	/* update pid */
	local_diag_dload.magic_struct.pid = PID_MAGIC_ID;
	local_diag_dload.pid = pid;

	/* update serial number */
	if (!snum) {
		local_diag_dload.magic_struct.serial_num = 0;
		memset(&local_diag_dload.serial_number, 0,
				SERIAL_NUMBER_LENGTH);
	} else {
		local_diag_dload.magic_struct.serial_num = SERIAL_NUM_MAGIC_ID;
		strlcpy((char *)&local_diag_dload.serial_number, snum,
				SERIAL_NUMBER_LENGTH);
	}

	/* Copy to shared struct (accesses need to be 32 bit aligned) */
	src = (int *)&local_diag_dload;
	dst = (int *)diag_dload;

	for (i = 0; i < sizeof(*diag_dload) / 4; i++)
		*dst++ = *src++;

	return 0;
}

static int __init init(void)
{
	struct android_dev *dev;
	int err;

	android_class = class_create(THIS_MODULE, "android_usb");
	if (IS_ERR(android_class))
		return PTR_ERR(android_class);

	dev = kzalloc(sizeof(*dev), GFP_KERNEL);
	if (!dev) {
		err = -ENOMEM;
		goto err_dev;
	}

	dev->disable_depth = 1;
	dev->functions = supported_functions;
	INIT_LIST_HEAD(&dev->enabled_functions);
	INIT_WORK(&dev->work, android_work);
	mutex_init(&dev->mutex);

	err = android_create_device(dev);
	if (err) {
		pr_err("%s: failed to create android device %d", __func__, err);
		goto err_create;
	}

	_android_dev = dev;

	err = usb_composite_probe(&android_usb_driver);
	if (err) {
		pr_err("%s: failed to probe driver %d", __func__, err);
		goto err_probe;
	}

	/* HACK: exchange composite's setup with ours */
	composite_setup_func = android_usb_driver.gadget_driver.setup;
	android_usb_driver.gadget_driver.setup = android_setup;
	composite_suspend_func = android_usb_driver.gadget_driver.suspend;
	android_usb_driver.gadget_driver.suspend = android_suspend;
	composite_resume_func = android_usb_driver.gadget_driver.resume;
	android_usb_driver.gadget_driver.resume = android_resume;

	return 0;

err_probe:
	device_destroy(android_class, dev->dev->devt);
err_create:
	kfree(dev);
err_dev:
	class_destroy(android_class);
	return err;
}
late_initcall(init);

static void __exit cleanup(void)
{
	usb_composite_unregister(&android_usb_driver);
	class_destroy(android_class);
	kfree(_android_dev);
	_android_dev = NULL;
}
module_exit(cleanup);
