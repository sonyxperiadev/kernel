/*****************************************************************************
* Copyright 2006 - 2009 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/

#include <linux/types.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/semaphore.h>
#include <linux/device.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>

#include <linux/broadcom/bsc.h>

#include <asm/uaccess.h>
#include <linux/version.h>

#define MAX_PROC_BUF_SIZE     256
#define PROC_PARENT_DIR       "bsc"
#define PROC_ENTRY_REGISTER   "register"
#define PROC_ENTRY_UNREGISTER "unregister"
#define PROC_ENTRY_QUERY      "query"
#define PROC_ENTRY_LIST       "list"

struct proc_dir {
	struct proc_dir_entry *parent_dir;
};

typedef struct {
	BSC_PARAM_T param;
	struct list_head list;
} BSC_ENTRY_T;

static struct proc_dir gProc;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 37)
static DECLARE_MUTEX(gBscLock);
#else
static DEFINE_SEMAPHORE(gBscLock);
#endif
static LIST_HEAD(gBscList);

static int gDriverMajor;
static struct class *bsc_class;
static struct device *bsc_dev;

/*
 * Routine that is used to register BSC data
 */
int bsc_register(const char *name, void *data, int len)
{
	int rc;
	struct list_head *list = &gBscList;
	BSC_ENTRY_T *entry = NULL;

	if (name == NULL || data == NULL || len <= 0) {
		rc = -EINVAL;
		goto err_quit;
	}

	rc = down_interruptible(&gBscLock);
	if (rc)
		goto err_quit;

	/* go through list to make sure it's not previously registered */
	list_for_each_entry(entry, list, list) {
		if (strcmp(entry->param.name, name) == 0) {
			/* do not allow re-register */
			rc = -EBUSY;
			goto err_unlock;
		}
	}

	/* allocate memory for entry */
	entry = kcalloc(1, sizeof(*entry), GFP_KERNEL);
	if (entry == NULL) {
		rc = -ENOMEM;
		goto err_unlock;
	}

	/* allocate memory for data */
	entry->param.data = kcalloc(1, len, GFP_KERNEL);
	if (entry->param.data == NULL) {
		rc = -ENOMEM;
		goto err_free_entry;
	}

	/* copy and save all required information and data */
	strncpy(entry->param.name, name, BSC_NAME_LEN);
	entry->param.name[BSC_NAME_LEN-1] = '\0';
	entry->param.len = len;
	memcpy(entry->param.data, data, len);

	list_add(&entry->list, list);

	up(&gBscLock);
	return 0;

      err_free_entry:
	kfree(entry);

      err_unlock:
	up(&gBscLock);

      err_quit:
	return rc;
}

EXPORT_SYMBOL(bsc_register);

/*
 * Routine that is used to unregister the BSC data
 */
int bsc_unregister(const char *name)
{
	int rc;
	struct list_head *list = &gBscList;
	BSC_ENTRY_T *entry = NULL;
	BSC_ENTRY_T *n;

	if (name == NULL) {
		return -EINVAL;
	}

	rc = down_interruptible(&gBscLock);
	if (rc)
		return rc;

	/* go through list to find entry */
	list_for_each_entry_safe(entry, n, list, list) {
		if (strcmp(entry->param.name, name) == 0) {
			/* found matching entry */
			list_del(&entry->list);
			kfree(entry->param.data);
			kfree(entry);
			up(&gBscLock);
			return 0;
		}
	}

	/* no entry can be found */
	up(&gBscLock);
	return -EFAULT;
}

EXPORT_SYMBOL(bsc_unregister);

/*
 * Routine that is used to query the BSC data. Note the memory that 'data'
 *points to should be at least 'len' bytes
 */
int bsc_query(const char *name, void *data, int len)
{
	int rc;
	struct list_head *list = &gBscList;
	BSC_ENTRY_T *entry = NULL;

	if (name == NULL || data == NULL || len <= 0) {
		return -EINVAL;
	}

	rc = down_interruptible(&gBscLock);
	if (rc)
		return rc;

	/* go through list to find entry */
	list_for_each_entry(entry, list, list) {
		if (strcmp(entry->param.name, name) == 0) {
			/* found matching entry */
			if (len < entry->param.len) {
				/* make sure length is large enough */
				up(&gBscLock);
				return -EINVAL;
			}
			/* everything's good, now copy the data */
			memcpy(data, entry->param.data, entry->param.len);
			up(&gBscLock);
			return 0;
		}
	}

	/* no entry can be found */
	up(&gBscLock);
	return -EFAULT;
}

EXPORT_SYMBOL(bsc_query);

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36)
static int bsc_ioctl(struct inode *inode,
#else
static long bsc_ioctl(
#endif
			     struct file *filp,
			     unsigned int cmd, unsigned long arg)
{
	int rc;
	BSC_PARAM_T param;
	BSC_PARAM_T *usr_param = NULL;

	switch (cmd) {
	case BSC_IOCTL_REGISTER:
		{
			usr_param = (BSC_PARAM_T *) arg;
			/* copy BSC parameters from user space */
			if (copy_from_user(&param.name,
				(void *) &(usr_param->name),
				BSC_NAME_LEN) != 0) {
				printk(KERN_ERR
				       "BSC: copy_from_user *name* failed for "
				       "ioctl=BSC_IOCTL_REGISTER\n");
				return -EFAULT;
			}

			param.len = usr_param->len;

			/* validate the data length */
			if (param.len <= 0)
				return -EINVAL;

			/* allocate temporary memory in kernel to store user data */
			param.data = kcalloc(1, param.len, GFP_KERNEL);
			if (param.data == NULL)
				return -ENOMEM;

			/* copy user data from user space into kernel */
			if (copy_from_user
			    (param.data, (void *)usr_param->data,
			     param.len) != 0) {
				printk(KERN_ERR
				       "BSC: copy_from_user failed for "
				       "ioctl=BSC_IOCTL_REGISTER during copying user data\n");
				kfree(param.data);
				return -EFAULT;
			}

			rc = bsc_register(param.name, param.data, param.len);
			if (rc != 0) {
				printk(KERN_ERR "BSC: bsc_register failed\n");
				kfree(param.data);
				return rc;
			}

			kfree(param.data);
			return 0;
		}

	case BSC_IOCTL_UNREGISTER:
		{
			char name[BSC_NAME_LEN];

			/* copy the name string from user space */
			if (strncpy_from_user(name, (void *)arg, BSC_NAME_LEN) <
			    0) {
				printk(KERN_ERR
				       "BSC: strncpy_from_user failed for "
				       "ioctl=BSC_IOCTL_UNREGISTER\n");
				return -EFAULT;
			}

			rc = bsc_unregister(name);
			if (rc != 0) {
				printk(KERN_ERR "BSC: bsc_unregister failed\n");
				return rc;
			}

			return 0;
		}

	case BSC_IOCTL_QUERY:
		{
			usr_param = (BSC_PARAM_T *) arg;
			/* copy BSC parameters from user space */
			if (copy_from_user(&param.name,
				(void *)&(usr_param->name),
				BSC_NAME_LEN) != 0) {
				printk(KERN_ERR
				       "BSC: copy_from_user failed for "
				       "ioctl=BSC_IOCTL_QUERY\n");
				return -EFAULT;
			}

			param.len = usr_param->len;
			/* validate the data length */
			if (param.len <= 0)
				return -EINVAL;

			/* allocate temporary memory in kernel to store user data */
			param.data = kcalloc(1, param.len, GFP_KERNEL);
			if (param.data == NULL)
				return -ENOMEM;

			rc = bsc_query(param.name, param.data, param.len);
			if (rc != 0) {
				kfree(param.data);
				return rc;
			}

			/* copy user data from kernel into user space */
			if (copy_to_user((void *)usr_param->data,
				(void *)param.data, param.len) != 0) {
				printk(KERN_ERR "BSC: copy_to_user failed for "
				       "ioctl=BSC_IOCTL_QUERY during copying user data\n");
				kfree(param.data);
				return -EFAULT;
			}

			kfree(param.data);
			return 0;
		}

	default:
		printk(KERN_WARNING "BSC: Unknown IOCTL cmd=0x%x\n", cmd);
		return -EINVAL;
	}

	return 0;
}

struct file_operations bsc_fops = {
      owner:THIS_MODULE,
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36)
      ioctl : bsc_ioctl,
#else
      unlocked_ioctl : bsc_ioctl,
#endif
};

static int
proc_register_write(struct file *file, const char __user *buffer,
		    size_t count, loff_t *data)
{
	int rc;
	BSC_PARAM_T param;
	/* Coverity OverRun error fix. Added #1 byte
	 * more for the kbuf array */
	unsigned char kbuf[MAX_PROC_BUF_SIZE + 1];
	unsigned char *tmp = kbuf;

	if (count > MAX_PROC_BUF_SIZE)
		count = MAX_PROC_BUF_SIZE;

	rc = copy_from_user(kbuf, buffer, count);
	if (rc) {
		printk(KERN_ERR "copy_from_user failed status=%d", rc);
		return -EFAULT;
	}

	tmp[count] = '\0';
	if (sscanf(kbuf, "%s", param.name) != 1) {
		printk(KERN_ERR "echo <name> <len> > /proc/%s/%s\n",
		       PROC_PARENT_DIR, PROC_ENTRY_REGISTER);
		return count;
	}

	tmp = tmp + strlen(param.name);

	if (kstrtol(tmp, 10, (long *)&param.len) < 0)
		return -EINVAL;

	if (param.len <= 0) {
		printk(KERN_INFO "Data length needs to be greater than zero\n");
		return count;
	}

	/* allocate whatever garbage since this is just for testing */
	param.data = kmalloc(param.len, GFP_KERNEL);
	if (param.data == NULL) {
		printk(KERN_ERR
		       "Unable to allocate memory for data of size %d bytes\n",
		       param.len);
		return count;
	}
	rc = bsc_register(param.name, param.data, param.len);
	if (rc != 0) {
		printk(KERN_ERR "Failed calling bsc_register\n");
	}

	kfree(param.data);
	return count;
}

static int
proc_unregister_write(struct file *file, const char __user *buffer,
		      size_t count, loff_t *data)
{
	int rc;
	char name[BSC_NAME_LEN];
	unsigned char kbuf[MAX_PROC_BUF_SIZE];

	if (count > MAX_PROC_BUF_SIZE)
		count = MAX_PROC_BUF_SIZE;

	rc = copy_from_user(kbuf, buffer, count);
	if (rc) {
		printk(KERN_ERR "copy_from_user failed status=%d", rc);
		return -EFAULT;
	}

	if (sscanf(kbuf, "%s", name) != 1) {
		printk(KERN_ERR "echo <name> > /proc/%s/%s\n",
		       PROC_PARENT_DIR, PROC_ENTRY_UNREGISTER);
		return count;
	}

	rc = bsc_unregister(name);
	if (rc != 0) {
		printk(KERN_ERR "Failed calling bsc_unregister\n");
	}

	return count;
}

static int
proc_query_write(struct file *file, const char __user *buffer,
		 size_t count, loff_t *data)
{
	int rc;
	BSC_PARAM_T param;
	/* Coverity OverRun error fix. Added #1 byte
	 * more for the kbuf array */
	unsigned char kbuf[MAX_PROC_BUF_SIZE + 1];
	unsigned char *tmp = kbuf;

	if (count > MAX_PROC_BUF_SIZE)
		count = MAX_PROC_BUF_SIZE;

	rc = copy_from_user(kbuf, buffer, count);
	if (rc) {
		printk(KERN_ERR "copy_from_user failed status=%d", rc);
		return -EFAULT;
	}

	if (sscanf(kbuf, "%s", param.name) != 1) {
		printk(KERN_ERR "echo <name> <len> > /proc/%s/%s\n",
		       PROC_PARENT_DIR, PROC_ENTRY_QUERY);
		return count;
	}
	tmp[count] = '\0';
	tmp = tmp + strlen(param.name);

	if (kstrtol(tmp, 10, (long *)&param.len) < 0)
		return -EINVAL;

	if (param.len <= 0) {
		printk(KERN_INFO "Data length needs to be greater than zero\n");
		return count;
	}

	param.data = kmalloc(param.len, GFP_KERNEL);
	if (param.data == NULL) {
		printk(KERN_ERR
		       "Unable to allocate memory for data of size %d bytes\n",
		       param.len);
		return count;
	}
	rc = bsc_query(param.name, param.data, param.len);
	if (rc != 0) {
		printk(KERN_ERR "Failed calling bsc_query\n");
	} else {
		printk(KERN_INFO "Query of [%s] successful\n", param.name);
	}

	kfree(param.data);
	return count;
}

static int
proc_list_read(struct file *file, char __user *buffer,
		 size_t count, loff_t *data)
{
	int rc;
	unsigned int len = 0;
	BSC_ENTRY_T *entry = NULL;
	struct list_head *list = &gBscList;

	len +=
	    sprintf(buffer + len,
		    "List of current Board Specific Configurations:\n");

	rc = down_interruptible(&gBscLock);
	if (rc)
		return len;

	/* go through list to find entry */
	list_for_each_entry(entry, list, list) {
		len += sprintf(buffer + len, "[%s] data length = %d bytes\n",
			       entry->param.name, entry->param.len);
	}

	up(&gBscLock);
	return len;
}

static const struct file_operations proc_register_fops = {
	.write =	proc_register_write,
};

static const struct file_operations proc_unregister_fops = {
	.write	=	proc_unregister_write,
};

static const struct file_operations proc_query_fops = {
	.write	=	proc_query_write,
};

static const struct file_operations proc_list_fops = {
	.read 	=	proc_list_read,
};
/*
 * Initialize the proc entries
 */
static int proc_init(void)
{
	int rc;
	struct proc_dir_entry *proc_register;
	struct proc_dir_entry *proc_unregister;
	struct proc_dir_entry *proc_query;
	struct proc_dir_entry *proc_list;

	gProc.parent_dir = proc_mkdir(PROC_PARENT_DIR, NULL);

	proc_register =
	    proc_create_data(PROC_ENTRY_REGISTER, 0644, gProc.parent_dir,
				&proc_register_fops, NULL);
	if (proc_register == NULL) {
		rc = -ENOMEM;
		goto proc_exit;
	}
	proc_unregister =
	    proc_create_data(PROC_ENTRY_UNREGISTER, 0644, gProc.parent_dir,
				&proc_unregister_fops, NULL);
	if (proc_unregister == NULL) {
		rc = -ENOMEM;
		goto proc_del_reg;
	}

	proc_query =
	    proc_create_data(PROC_ENTRY_QUERY, 0644, gProc.parent_dir,
				&proc_query_fops, NULL);
	if (proc_query == NULL) {
		rc = -ENOMEM;
		goto proc_del_unreg;
	}

	proc_list = proc_create_data(PROC_ENTRY_LIST, 0644, gProc.parent_dir,
					&proc_list_fops, NULL);
	if (proc_list == NULL) {
		rc = -ENOMEM;
		goto proc_del_query;
	}
	return 0;

      proc_del_query:
	remove_proc_entry(PROC_ENTRY_QUERY, gProc.parent_dir);

      proc_del_unreg:
	remove_proc_entry(PROC_ENTRY_UNREGISTER, gProc.parent_dir);

      proc_del_reg:
	remove_proc_entry(PROC_ENTRY_REGISTER, gProc.parent_dir);

      proc_exit:
	remove_proc_entry(PROC_PARENT_DIR, NULL);

	return rc;
}

/*
 * Terminate and remove the proc entries
 */
static void proc_term(void)
{
	remove_proc_entry(PROC_ENTRY_LIST, gProc.parent_dir);
	remove_proc_entry(PROC_ENTRY_QUERY, gProc.parent_dir);
	remove_proc_entry(PROC_ENTRY_REGISTER, gProc.parent_dir);
	remove_proc_entry(PROC_PARENT_DIR, NULL);
}

static int __init bsc_init(void)
{
	int rc;

	gDriverMajor = register_chrdev(0, "bsc", &bsc_fops);
	if (gDriverMajor < 0) {
		printk(KERN_WARNING "BSC: register_chrdev failed for major\n");
		rc = -EFAULT;
		goto err_exit;
	}

	bsc_class = class_create(THIS_MODULE, "bcm-bsc");
	if (IS_ERR(bsc_class)) {
		printk(KERN_ERR "BSC: Class creation failed\n");
		rc = -EFAULT;
		goto err_drv_unreg;
	}

	bsc_dev = device_create(bsc_class, NULL, MKDEV(gDriverMajor, 0), NULL,
				"bsc");
	if (IS_ERR(bsc_dev)) {
		printk(KERN_ERR "BSC: Device creation failed\n");
		rc = -EFAULT;
		goto err_class_destroy;
	}

	rc = proc_init();
	if (rc < 0) {
		printk(KERN_WARNING "BSC: proc_init failed err=%d\n", rc);
		goto err_device_destroy;
	}

	printk(KERN_INFO "BSC: Driver initialized\n");

	return 0;

      err_device_destroy:
	device_destroy(bsc_class, MKDEV(gDriverMajor, 0));

      err_class_destroy:
	class_destroy(bsc_class);

      err_drv_unreg:
	unregister_chrdev(gDriverMajor, "bsc");

      err_exit:
	return rc;
}

static void __exit bsc_exit(void)
{
	struct list_head *list = &gBscList;
	BSC_ENTRY_T *entry = NULL;
	BSC_ENTRY_T *n;

	proc_term();

	device_destroy(bsc_class, MKDEV(gDriverMajor, 0));
	class_destroy(bsc_class);

	unregister_chrdev(gDriverMajor, "bsc");

	down(&gBscLock);
	/* go through list */
	list_for_each_entry_safe(entry, n, list, list) {
		list_del(&entry->list);
		kfree(entry->param.data);
		kfree(entry);
	}
	up(&gBscLock);
}

module_init(bsc_init);
module_exit(bsc_exit);

MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("Board Specific Configuration (BSC) Driver");
MODULE_LICENSE("GPL");
