/*
 * cyttsp4_devtree.c
 * Cypress TrueTouch(TM) Standard Product V4 Device Tree Support Driver.
 * For use with Cypress Txx4xx parts.
 * Supported parts include:
 * TMA4XX
 * TMA1036
 *
 * Copyright (C) 2013 Cypress Semiconductor
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2, and only version 2, as published by the
 * Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Contact Cypress Semiconductor at www.cypress.com <ttdrivers@cypress.com>
 *
 */

#include <linux/device.h>
#include <linux/err.h>
#include <linux/of_device.h>
#include <linux/slab.h>

/* cyttsp */
#include <linux/cyttsp4_bus.h>
#include <linux/cyttsp4_core.h>
#include <linux/cyttsp4_btn.h>
#include <linux/cyttsp4_mt.h>
#include <linux/cyttsp4_proximity.h>
#include <linux/cyttsp4_platform.h>

#include "cyttsp4_regs.h"
#include "cyttsp4_devtree.h"

#define ENABLE_VIRTUAL_KEYS

#define MAX_NAME_LENGTH		64

enum cyttsp4_device_type {
	DEVICE_MT,
	DEVICE_BTN,
	DEVICE_PROXIMITY,
	DEVICE_TYPE_MAX,
};

struct cyttsp4_device_pdata_func {
	void *(*create_and_get_pdata)(struct device_node *);
	void (*free_pdata)(void *);
};

#ifdef ENABLE_VIRTUAL_KEYS
static struct kobject *board_properties_kobj;

struct cyttsp4_virtual_keys {
	struct kobj_attribute kobj_attr;
	u16 *data;
	int size;
};
#endif

struct cyttsp4_extended_mt_platform_data {
	struct cyttsp4_mt_platform_data pdata;
#ifdef ENABLE_VIRTUAL_KEYS
	struct cyttsp4_virtual_keys vkeys;
#endif
};

static inline int get_inp_dev_name(struct device_node *dev_node,
		const char **inp_dev_name)
{
	return of_property_read_string(dev_node, "cy,inp_dev_name",
			inp_dev_name);
}

static u16 *create_and_get_u16_array(struct device_node *dev_node,
		const char *name, int *size)
{
	const __be32 *values;
	u16 *val_array;
	int len;
	int sz;
	int rc;
	int i;

	values = of_get_property(dev_node, name, &len);
	if (values == NULL)
		return NULL;

	sz = len / sizeof(u32);
//	pr_debug("%s: %s size:%d\n", __func__, name, sz);

	val_array = kzalloc(sz * sizeof(u16), GFP_KERNEL);
	if (val_array == NULL) {
		rc = -ENOMEM;
		goto fail;
	}

	for (i = 0; i < sz; i++)
		val_array[i] = (u16)be32_to_cpup(values++);

	*size = sz;

	return val_array;

fail:
	return ERR_PTR(rc);
}

static struct touch_framework *create_and_get_touch_framework(
		struct device_node *dev_node)
{
	struct touch_framework *frmwrk;
	u16 *abs;
	int size;
	int rc;

	abs = create_and_get_u16_array(dev_node, "cy,abs", &size);
	if (IS_ERR_OR_NULL(abs))
		return (void *)abs;

	/* Check for valid abs size */
	if (size % CY_NUM_ABS_SET) {
		rc = -EINVAL;
		goto fail_free_abs;
	}

	frmwrk = kzalloc(sizeof(*frmwrk), GFP_KERNEL);
	if (frmwrk == NULL) {
		rc = -ENOMEM;
		goto fail_free_abs;
	}

	frmwrk->abs = abs;
	frmwrk->size = size;

	return frmwrk;

fail_free_abs:
	kfree(abs);

	return ERR_PTR(rc);
}

static void free_touch_framework(struct touch_framework *frmwrk)
{
	kfree(frmwrk->abs);
	kfree(frmwrk);
}

#ifdef ENABLE_VIRTUAL_KEYS
#define VIRTUAL_KEY_ELEMENT_SIZE	5
static ssize_t virtual_keys_show(struct kobject *kobj,
		struct kobj_attribute *attr, char *buf)
{
	struct cyttsp4_virtual_keys *vkeys = container_of(attr,
		struct cyttsp4_virtual_keys, kobj_attr);
	u16 *data = vkeys->data;
	int size = vkeys->size;
	int index;
	int i;

	index = 0;
	for (i = 0; i < size; i += VIRTUAL_KEY_ELEMENT_SIZE)
		index += scnprintf(buf + index, CY_MAX_PRBUF_SIZE - index,
			"0x01:%d:%d:%d:%d:%d\n",
			data[i], data[i+1], data[i+2], data[i+3], data[i+4]);

	return index;
}

static int setup_virtual_keys(struct device_node *dev_node,
		const char *inp_dev_name, struct cyttsp4_virtual_keys *vkeys)
{
	char *name;
	u16 *data;
	int size;
	int rc;

	data = create_and_get_u16_array(dev_node, "cy,virtual_keys", &size);
	if (data == NULL)
		return 0;
	else if (IS_ERR(data)) {
		rc = PTR_ERR(data);
		goto fail;
	}

	/* Check for valid virtual keys size */
	if (size % VIRTUAL_KEY_ELEMENT_SIZE) {
		rc = -EINVAL;
		goto fail_free_data;
	}

	name = kzalloc(MAX_NAME_LENGTH, GFP_KERNEL);
	if (name == NULL) {
		rc = -ENOMEM;
		goto fail_free_data;
	}

	snprintf(name, MAX_NAME_LENGTH, "virtualkeys.%s", inp_dev_name);

	vkeys->data = data;
	vkeys->size = size;

	/* TODO: Instantiate in board file and export it */
	if (board_properties_kobj == NULL)
		board_properties_kobj =
			kobject_create_and_add("board_properties", NULL);
	if (board_properties_kobj == NULL) {
		pr_err("%s: Cannot get board_properties kobject!\n", __func__);
		rc = -EINVAL;
		goto fail_free_name;
	}

	/* Initialize dynamic SysFs attribute */
	sysfs_attr_init(&vkeys->kobj_attr.attr);
	vkeys->kobj_attr.attr.name = name;
	vkeys->kobj_attr.attr.mode = S_IRUGO;
	vkeys->kobj_attr.show = virtual_keys_show;

	rc = sysfs_create_file(board_properties_kobj, &vkeys->kobj_attr.attr);
	if (rc)
		goto fail_del_kobj;

	return 0;

fail_del_kobj:
	kobject_del(board_properties_kobj);
fail_free_name:
	kfree(name);
	vkeys->kobj_attr.attr.name = NULL;
fail_free_data:
	kfree(data);
	vkeys->data = NULL;
fail:
	return rc;
}

static void free_virtual_keys(struct cyttsp4_virtual_keys *vkeys)
{
	if (board_properties_kobj)
		sysfs_remove_file(board_properties_kobj,
			&vkeys->kobj_attr.attr);

	kfree(vkeys->data);
	kfree(vkeys->kobj_attr.attr.name);
}
#endif

static void *create_and_get_mt_pdata(struct device_node *dev_node)
{
	struct cyttsp4_extended_mt_platform_data *ext_pdata;
	struct cyttsp4_mt_platform_data *pdata;
	u32 value;
	int rc;

	ext_pdata = kzalloc(sizeof(*ext_pdata), GFP_KERNEL);
	if (ext_pdata == NULL) {
		rc = -ENOMEM;
		goto fail;
	}

	pdata = &ext_pdata->pdata;

	rc = get_inp_dev_name(dev_node, &pdata->inp_dev_name);
	if (rc)
		goto fail_free_pdata;

	/* Optional fields */
	rc = of_property_read_u32(dev_node, "cy,flags", &value);
	if (!rc)
		pdata->flags = value;

	rc = of_property_read_u32(dev_node, "cy,vkeys_x", &value);
	if (!rc)
		pdata->vkeys_x = value;

	rc = of_property_read_u32(dev_node, "cy,vkeys_y", &value);
	if (!rc)
		pdata->vkeys_y = value;

	/* Required fields */
	pdata->frmwrk = create_and_get_touch_framework(dev_node);
	if (pdata->frmwrk == NULL) {
		rc = -EINVAL;
		goto fail_free_pdata;
	} else if (IS_ERR(pdata->frmwrk)) {
		rc = PTR_ERR(pdata->frmwrk);
		goto fail_free_pdata;
	}
#ifdef ENABLE_VIRTUAL_KEYS
	rc = setup_virtual_keys(dev_node, pdata->inp_dev_name,
			&ext_pdata->vkeys);
	if (rc) {
		pr_err("%s: Cannot setup virtual keys!\n", __func__);
		goto fail_free_pdata;
	}
#endif
	return pdata;

fail_free_pdata:
	kfree(ext_pdata);
fail:
	return ERR_PTR(rc);
}

static void free_mt_pdata(void *pdata)
{
	struct cyttsp4_mt_platform_data *mt_pdata =
		(struct cyttsp4_mt_platform_data *)pdata;
	struct cyttsp4_extended_mt_platform_data *ext_mt_pdata =
		container_of(mt_pdata,
			struct cyttsp4_extended_mt_platform_data, pdata);

	free_touch_framework(mt_pdata->frmwrk);
#ifdef ENABLE_VIRTUAL_KEYS
	free_virtual_keys(&ext_mt_pdata->vkeys);
#endif
	kfree(ext_mt_pdata);
}

static void *create_and_get_btn_pdata(struct device_node *dev_node)
{
	struct cyttsp4_btn_platform_data *pdata;
	int rc;

	pdata = kzalloc(sizeof(*pdata), GFP_KERNEL);
	if (pdata == NULL) {
		rc = -ENOMEM;
		goto fail;
	}

	rc = get_inp_dev_name(dev_node, &pdata->inp_dev_name);
	if (rc)
		goto fail_free_pdata;

	return pdata;

fail_free_pdata:
	kfree(pdata);
fail:
	return ERR_PTR(rc);
}

static void free_btn_pdata(void *pdata)
{
	struct cyttsp4_btn_platform_data *btn_pdata =
		(struct cyttsp4_btn_platform_data *)pdata;

	kfree(btn_pdata);
}

static void *create_and_get_proximity_pdata(struct device_node *dev_node)
{
	struct cyttsp4_proximity_platform_data *pdata;
	int rc;

	pdata = kzalloc(sizeof(*pdata), GFP_KERNEL);
	if (pdata == NULL) {
		rc = -ENOMEM;
		goto fail;
	}

	rc = get_inp_dev_name(dev_node, &pdata->inp_dev_name);
	if (rc)
		goto fail_free_pdata;

	pdata->frmwrk = create_and_get_touch_framework(dev_node);
	if (pdata->frmwrk == NULL) {
		rc = -EINVAL;
		goto fail_free_pdata;
	} else if (IS_ERR(pdata->frmwrk)) {
		rc = PTR_ERR(pdata->frmwrk);
		goto fail_free_pdata;
	}

	return pdata;

fail_free_pdata:
	kfree(pdata);
fail:
	return ERR_PTR(rc);
}

static void free_proximity_pdata(void *pdata)
{
	struct cyttsp4_proximity_platform_data *proximity_pdata =
		(struct cyttsp4_proximity_platform_data *)pdata;

	free_touch_framework(proximity_pdata->frmwrk);

	kfree(proximity_pdata);
}

static struct cyttsp4_device_pdata_func device_pdata_funcs[DEVICE_TYPE_MAX] = {
	[DEVICE_MT] = {
		.create_and_get_pdata = create_and_get_mt_pdata,
		.free_pdata = free_mt_pdata,
	},
	[DEVICE_BTN] = {
		.create_and_get_pdata = create_and_get_btn_pdata,
		.free_pdata = free_btn_pdata,
	},
	[DEVICE_PROXIMITY] = {
		.create_and_get_pdata = create_and_get_proximity_pdata,
		.free_pdata = free_proximity_pdata,
	},
};

static const char *device_names[DEVICE_TYPE_MAX] = {
	[DEVICE_MT] = "cy,mt",
	[DEVICE_BTN] = "cy,btn",
	[DEVICE_PROXIMITY] = "cy,proximity",
};

static int get_device_type(struct device_node *dev_node,
		enum cyttsp4_device_type *type)
{
	const char *name;
	enum cyttsp4_device_type t;
	int rc;

	rc = of_property_read_string(dev_node, "name", &name);
	if (rc)
		return rc;

	for (t = 0; t < DEVICE_TYPE_MAX; t++)
		if (!strncmp(name, device_names[t], MAX_NAME_LENGTH)) {
			*type = t;
			return 0;
		}

	return -EINVAL;
}

static inline void *create_and_get_device_pdata(struct device_node *dev_node,
		enum cyttsp4_device_type type)
{
	return device_pdata_funcs[type].create_and_get_pdata(dev_node);
}

static inline void free_device_pdata(void *pdata,
		enum cyttsp4_device_type type)
{
	device_pdata_funcs[type].free_pdata(pdata);
}

static int register_device(struct device_node *dev_node,
		const char *core_id)
{
	struct cyttsp4_device_info info = {0};
	enum cyttsp4_device_type type;
	int rc;

	info.core_id = core_id;

	rc = of_property_read_string(dev_node, "cy,name", &info.name);
	if (rc) {
		pr_err("%s: OF error rc=%d\n", __func__, rc);
		goto fail;
	} else
		pr_debug("%s: OF cy,name: %s\n", __func__, info.name);

	rc = get_device_type(dev_node, &type);
	if (rc)
		goto fail;

	info.platform_data = create_and_get_device_pdata(dev_node, type);
	if (IS_ERR(info.platform_data)) {
		rc = PTR_ERR(info.platform_data);
		goto fail;
	}

	rc = cyttsp4_register_device(&info);
	if (rc)
		goto fail_free;

	return 0;

fail_free:
	free_device_pdata(info.platform_data, type);
fail:
	return rc;
}

static struct touch_settings *create_and_get_touch_setting(
		struct device_node *core_node, const char *name)
{
	struct touch_settings *setting;
	char *tag_name;
	u32 tag_value;
	u16 *data;
	int size;
	int rc;

	data = create_and_get_u16_array(core_node, name, &size);
	if (IS_ERR_OR_NULL(data))
		return (void *)data;

//	pr_debug("%s: Touch setting:'%s' size:%d\n", __func__, name, size);

	setting = kzalloc(sizeof(*setting), GFP_KERNEL);
	if (setting == NULL) {
		rc = -ENOMEM;
		goto fail_free_data;
	}

	setting->data = (u8 *)data;
	setting->size = size;

	tag_name = kzalloc(MAX_NAME_LENGTH, GFP_KERNEL);
	if (tag_name == NULL) {
		rc = -ENOMEM;
		goto fail_free_setting;
	}

	snprintf(tag_name, MAX_NAME_LENGTH, "%s-tag", name);

	rc = of_property_read_u32(core_node, tag_name, &tag_value);
	if (!rc)
		setting->tag = tag_value;

	kfree(tag_name);

	return setting;

fail_free_setting:
	kfree(setting);
fail_free_data:
	kfree(data);

	return ERR_PTR(rc);
}

static void free_touch_setting(struct touch_settings *setting)
{
	if (setting) {
		kfree(setting->data);
		kfree(setting);
	}
}

static char *touch_setting_names[CY_IC_GRPNUM_NUM] = {
	NULL,			/* CY_IC_GRPNUM_RESERVED */
	"cy,cmd_regs",		/* CY_IC_GRPNUM_CMD_REGS */
	"cy,tch_rep",		/* CY_IC_GRPNUM_TCH_REP */
	"cy,data_rec",		/* CY_IC_GRPNUM_DATA_REC */
	"cy,test_rec",		/* CY_IC_GRPNUM_TEST_REC */
	"cy,pcfg_rec",		/* CY_IC_GRPNUM_PCFG_REC */
	"cy,tch_parm_val",	/* CY_IC_GRPNUM_TCH_PARM_VAL */
	"cy,tch_parm_size",	/* CY_IC_GRPNUM_TCH_PARM_SIZE */
	NULL,			/* CY_IC_GRPNUM_RESERVED1 */
	NULL,			/* CY_IC_GRPNUM_RESERVED2 */
	"cy,opcfg_rec",		/* CY_IC_GRPNUM_OPCFG_REC */
	"cy,ddata_rec",		/* CY_IC_GRPNUM_DDATA_REC */
	"cy,mdata_rec",		/* CY_IC_GRPNUM_MDATA_REC */
	"cy,test_regs",		/* CY_IC_GRPNUM_TEST_REGS */
	"cy,btn_keys",		/* CY_IC_GRPNUM_BTN_KEYS */
	NULL,			/* CY_IC_GRPNUM_TTHE_REGS */
};

static struct cyttsp4_core_platform_data *create_and_get_core_pdata(
		struct device_node *core_node)
{
	struct cyttsp4_core_platform_data *pdata;
	u32 value;
	int rc;
	int i;

	pdata = kzalloc(sizeof(*pdata), GFP_KERNEL);
	if (pdata == NULL) {
		rc = -ENOMEM;
		goto fail;
	}

	/* Required fields */
	rc = of_property_read_u32(core_node, "cy,irq_gpio", &value);
	if (rc)
		goto fail_free;
	pdata->irq_gpio = value;

	/* Optional fields */
	/* rst_gpio is optional since a platform may use
	 * power cycling instead of using the XRES pin
	 */
	rc = of_property_read_u32(core_node, "cy,rst_gpio", &value);
	if (!rc)
		pdata->rst_gpio = value;

	rc = of_property_read_u32(core_node, "cy,level_irq_udelay", &value);
	if (!rc)
		pdata->level_irq_udelay = value;

	rc = of_property_read_u32(core_node, "cy,max_xfer_len", &value);
	if (!rc)
		pdata->max_xfer_len = value;

	rc = of_property_read_u32(core_node, "cy,flags", &value);
	if (!rc)
		pdata->flags = value;

	rc = of_property_read_u32(core_node, "cy,easy_wakeup_gesture", &value);
	if (!rc)
		pdata->easy_wakeup_gesture = (u8)value;

	for (i = 0; (unsigned int)i < ARRAY_SIZE(touch_setting_names); i++) {
		if (touch_setting_names[i] == NULL)
			continue;

		pdata->sett[i] = create_and_get_touch_setting(core_node,
				touch_setting_names[i]);
		if (IS_ERR(pdata->sett[i])) {
			rc = PTR_ERR(pdata->sett[i]);
			goto fail_free_sett;
		}
#if 0
		else if (pdata->sett[i] == NULL)
			pr_debug("%s: No data for setting '%s'\n", __func__,
				touch_setting_names[i]);
#endif
	}
#if 0
	pr_debug("%s: irq_gpio:%d rst_gpio:%d level_irq_udelay:%d\n"
		"max_xfer_len:%d flags:%d easy_wakeup_gesture:%d\n", __func__,
		pdata->irq_gpio, pdata->rst_gpio, pdata->level_irq_udelay,
		pdata->max_xfer_len, pdata->flags, pdata->easy_wakeup_gesture);
#endif
	pdata->xres = cyttsp4_xres;
	pdata->init = cyttsp4_init;
	pdata->power = cyttsp4_power;
#ifdef CYTTSP4_DETECT_HW
	pdata->detect = cyttsp4_detect;
#endif
	pdata->irq_stat = cyttsp4_irq_stat;

#ifdef CONFIG_TOUCHSCREEN_CYPRESS_CYTTSP4_PLATFORM_YUKON
	pdata->loader_pdata = &_cyttsp4_loader_platform_data;
#else
        pdata->loader_pdata = &_cyttsp4_loader_platform_data_vy58;
#endif

	return pdata;

fail_free_sett:
	for (i--; i >= 0; i--)
		free_touch_setting(pdata->sett[i]);
fail_free:
	kfree(pdata);
fail:
	return ERR_PTR(rc);
}

static void free_core_pdata(struct cyttsp4_core_platform_data *pdata)
{
	int i;

	for (i = 0; i < CY_TOUCH_SETTINGS_MAX; i++)
		free_touch_setting(pdata->sett[i]);

	kfree(pdata);
}

static int register_core_device(struct device_node *core_node,
		const char *adap_id, const char **core_id)
{
	struct cyttsp4_core_info info = {0};
	int rc;

	rc = of_property_read_string(core_node, "cy,name", &info.name);
	if (rc) {
		pr_err("%s: OF error rc=%d\n", __func__, rc);
		goto fail;
	} else
		pr_debug("%s: OF cy,name: %s\n", __func__, info.name);

	rc = of_property_read_string(core_node, "cy,id", &info.id);
	if (rc) {
		pr_err("%s: OF error rc=%d\n", __func__, rc);
		goto fail;
	} else
		pr_debug("%s: OF cy,id: %s\n", __func__, info.id);

	info.platform_data = create_and_get_core_pdata(core_node);
	if (IS_ERR(info.platform_data)) {
		rc = PTR_ERR(info.platform_data);
		goto fail;
	}

	info.adap_id = adap_id;

	rc = cyttsp4_register_core_device(&info);
	if (rc)
		goto fail_free;

	*core_id = info.id;

	return 0;

fail_free:
	free_core_pdata(info.platform_data);
fail:
	return rc;
}

int cyttsp4_devtree_register_devices(struct device *adap_dev)
{
	struct device_node *core_node, *dev_node;
	const char *adap_id;
	int count = 0;
	int rc;

	if (!adap_dev->of_node)
		return 0;

	rc = of_property_read_string(adap_dev->of_node, "cy,adapter_id",
			&adap_id);
	if (rc)
		return rc;

	/* There should be only one core node */
	for_each_child_of_node(adap_dev->of_node, core_node) {
		const char *core_id = NULL;
		const char *name;

		rc = of_property_read_string(core_node, "name", &name);
		if (!rc)
			pr_debug("%s: name:%s\n", __func__, name);

		rc = register_core_device(core_node, adap_id, &core_id);
		if (rc)
			break;
		/* Increment reference count */
		of_node_get(core_node);

		for_each_child_of_node(core_node, dev_node) {
			count++;
			rc = register_device(dev_node, core_id);
			if (rc)
				break;
			/* Increment reference count */
			of_node_get(dev_node);
		}
	}

	pr_debug("%s: %d child node(s) found\n", __func__, count);

	return rc;
}
EXPORT_SYMBOL_GPL(cyttsp4_devtree_register_devices);

static int __init cyttsp4_devtree_init(void)
{
	return 0;
}
module_init(cyttsp4_devtree_init);

static void __exit cyttsp4_devtree_exit(void)
{
}
module_exit(cyttsp4_devtree_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Cypress Semiconductor");
