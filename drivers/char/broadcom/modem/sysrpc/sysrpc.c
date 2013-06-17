#include <linux/kobject.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/init.h>

#include "mobcom_types.h"
#include "resultcode.h"
#include "taskmsgs.h"
#include <linux/broadcom/ipcproperties.h>
#include "rpc_global.h"
#include "rpc_ipc.h"
#include "xdr_porting_layer.h"
#include "xdr.h"
#include "rpc_api.h"
#include "sys_common_rpc.h"
#include "sys_gen_rpc.h"

#define	SYSRPC_TRACE(format)	printk(KERN_DEBUG format)


struct sysrpc_obj {
	struct kobject kobj;
	int sobj;
	int temp;
};

#define container_of_sysrpc(x) container_of(x, struct sysrpc_obj, kobj)

struct sysrpc_attribute {
	struct attribute attr;
	ssize_t (*show)(struct sysrpc_obj *sobj,
	struct sysrpc_attribute *attr,
	char *buf);
	ssize_t (*store)(struct sysrpc_obj *sobj,
	struct sysrpc_attribute *attr,
	const char *buf,
	size_t count);
};
#define container_of_sattr(x) container_of(x, struct sysrpc_attribute, attr)

static ssize_t sysrpc_attr_show(struct kobject *kobj,
				struct attribute *attr,
				char *buf)
{
	struct sysrpc_attribute *attribute;
	struct sysrpc_obj *sobj;

	_DBG_(SYSRPC_TRACE("SYSRPC: sysrpc_attr_show()\n"));
	attribute = container_of_sattr(attr);
	sobj = container_of_sysrpc(kobj);

	if (!attribute->show)
		return -EIO;

	return attribute->show(sobj, attribute, buf);
}

static ssize_t sysrpc_attr_store(struct kobject *kobj,
				struct attribute *attr,
				const char *buf, size_t len)
{
	struct sysrpc_attribute *attribute;
	struct sysrpc_obj *sobj;

	_DBG_(SYSRPC_TRACE("SYSRPC: ssysrpc_attr_store()\n"));
	attribute = container_of_sattr(attr);
	sobj = container_of_sysrpc(kobj);

	if (!attribute->store)
		return -EIO;

	return attribute->store(sobj, attribute, buf, len);
}

static const struct sysfs_ops sysrpc_sysfs_ops = {
	.show = sysrpc_attr_show,
	.store = sysrpc_attr_store,
};

static void sysrpc_release(struct kobject *kobj)
{
	struct sysrpc_obj *sobj;

	_DBG_(SYSRPC_TRACE("SYSRPC: sysrpc_release()\n"));
	sobj = container_of_sysrpc(kobj);
	kfree(sobj);
}


static ssize_t sysrpc_show(struct sysrpc_obj *sysrpc_obj,
			struct sysrpc_attribute *attr,
			char *buf)
{
	int var;

	_DBG_(SYSRPC_TRACE("SYSRPC: sysrpc_show()\n"));
	if (strcmp(attr->attr.name, "lps") == 0)
		var = sysrpc_obj->sobj;
	else
		var = sysrpc_obj->temp;

	return sprintf(buf, "%d\n", var);
}

static ssize_t sysrpc_store(struct sysrpc_obj *sysrpc_obj,
			struct sysrpc_attribute *attr,
			const char *buf, size_t count)
{
	int var;
	char varStr[5];
	static bool firstTime = TRUE;

	_DBG_(SYSRPC_TRACE("SYSRPC: sysrpc_store()\n"));
	sscanf(buf, "%du", &var);
	if (strcmp(attr->attr.name, "lps") == 0) {
		sysrpc_obj->sobj = var;
		if (firstTime) {
			firstTime = FALSE;
			SYS_AT_MTEST_Handler(0, SYS_GetClientId(), "1", "1",
			"coolcard", "0", "0", "0", "0", "0", "0", 10);
		}
		/* Invoke MTEST=25,4,var */
		snprintf(varStr, 5, "%du", var);
		SYS_AT_MTEST_Handler(0, SYS_GetClientId(), "25", "4", varStr,
			"0", "0", "0", "0", "0", "0", 10);
	} else
		sysrpc_obj->temp = var;
	return count;
}

static struct sysrpc_attribute sysrpc_attribute =
	__ATTR(lps,
		(S_IRUSR|S_IRGRP|S_IROTH|S_IWUSR),
		sysrpc_show, sysrpc_store);

static struct attribute *sysrpc_default_attrs[] = {
	&sysrpc_attribute.attr,
	NULL,
};

static struct kobj_type sysrpc_ktype = {
	.sysfs_ops = &sysrpc_sysfs_ops,
	.release = sysrpc_release,
	.default_attrs = sysrpc_default_attrs,
};

static struct kset *sysrpc_kset;
static struct sysrpc_obj *sysrpc_obj;


static struct sysrpc_obj *create_sysrpc_obj(const char *name)
{
	struct sysrpc_obj *sobj;
	int retval;

	_DBG_(SYSRPC_TRACE("SYSRPC: create_sysrpc_obj()\n"));
	sobj = kzalloc(sizeof(*sobj), GFP_KERNEL);
	if (!sobj)
		return NULL;

	sobj->kobj.kset = sysrpc_kset;

	retval = kobject_init_and_add(&sobj->kobj, &sysrpc_ktype,
			NULL, "%s", name);
	if (retval) {
		kobject_put(&sobj->kobj);
		return NULL;
	}

	kobject_uevent(&sobj->kobj, KOBJ_ADD);

	return sobj;
}

static void destroy_sysrpc_obj(struct sysrpc_obj *sobj)
{
	_DBG_(SYSRPC_TRACE("SYSRPC: destroy_sysrpc_obj()\n"));
	kobject_put(&sobj->kobj);
}

static int __init sysrpc_init(void)
{
	_DBG_(SYSRPC_TRACE("SYSRPC: sysrpc_init()\n"));
	sysrpc_kset = kset_create_and_add("sysrpc", NULL, kernel_kobj);
	if (!sysrpc_kset)
		return -ENOMEM;

	sysrpc_obj = create_sysrpc_obj("mtest");
	if (!sysrpc_obj)
		goto sysrpc_error;

	return 0;

sysrpc_error:
	_DBG_(SYSRPC_TRACE("SYSRPC: sysrpc_init() - ERROR\n"));
	return -EINVAL;
}

static void __exit sysrpc_exit(void)
{
	printk(KERN_DEBUG "SYSRPC: sysrpc_exit()\n");
	destroy_sysrpc_obj(sysrpc_obj);
	kset_unregister(sysrpc_kset);
}

module_init(sysrpc_init);
module_exit(sysrpc_exit);
