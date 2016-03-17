/****************************************************************************
*
*	Copyright (c) 2011 Broadcom Corporation
*
*   Unless you and Broadcom execute a separate written software license
*   agreement governing use of this software, this software is licensed to you
*   under the terms of the GNU General Public License version 2, available
*    at http://www.gnu.org/licenses/old-licenses/gpl-2.0.html (the "GPL").
*
*   Notwithstanding the above, under no circumstances may you combine this
*   software in any way with any other Broadcom software provided under a
*   license other than the GPL, without Broadcom's express prior written
*   consent.
*
****************************************************************************/


#include <linux/init.h>
#include <linux/module.h>
#include <linux/input.h>
#include <linux/miscdevice.h>
#include <linux/poll.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/kfifo.h>
#include <linux/list.h>
#include <linux/spinlock_types.h>
#include <linux/workqueue.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/version.h>

#include <linux/brvsens_driver.h>

#define DRIVER_VERSION       1
#define DRIVER_NAME          "brvsens"
#define DEFAULT_DELAY_MSEC    100

#define ACTIVATE_SENSOR(sw, flag)				\
	if (sw->activateCbk)					\
		sw->activateCbk(sw->context, flag);

static int dbg;
module_param_named(dbg, dbg, int, S_IRUGO | S_IWUSR | S_IWGRP);

/* work struct definition */
struct sensor_work {
	struct work_struct wq;

	char     name[20];      /* queue name */
	u32      handle;	/* sensor id -- follows
				   Android enumeration */
	atomic_t pollrate;      /* pollrate */
	atomic_t exit;          /* exit flag */

	void        *context;        /* driver context */
	PFNACTIVATE activateCbk;     /* activate function */
	PFNREAD     readCbk;         /* read function */
};

/* Work Object Entry --- Sensor List Node */
struct brvnode {
	struct list_head  list;		/* embedded link list (Kernel way!) */

	struct workqueue_struct	 *pwq;	/* dynamically allocated workqueue */
	struct sensor_work       *psw;	/* dynamically allocated
					   work function */
};


/* Main Data Structure Definition */
struct brvsens_driver {
	dev_t            devnum;	/* brvsens dev registration */
	struct class    *class;
	struct cdev      cdev;

	spinlock_t       lock;		/* multiple readers (workqueues)
					   access guard */
	struct brvnode   dnodes;  /* list of sensor "nodes" -- 1 per driver
				     registration */

    /* Shared Event Buffer: kfifo interface has changed between 35 and 36    */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36)
	DECLARE_KFIFO(ebuff, BRVSENS_EVENT_NUM * sizeof(struct brvsens_event));
#else
	DECLARE_KFIFO(ebuff, struct brvsens_event, BRVSENS_EVENT_NUM);
						/* more civilized! */
#endif

	wait_queue_head_t esig;	 /* buffer data event; signaled on each write */
};

/* Global Instance of BRVSENS Driver */
static struct brvsens_driver brvsens_data;

/*
   Workqueue function
   ====================

   --- Keeps polling the sensor (rate can be modified dynamically -- see ioctl)
   until Sensor is disabled / node deactivated

   --- Each read sticks extracted event into spinlocked shared kfifo buffer
   then sets
*/
static void sensor_poll(struct work_struct *work)
{
    /* cast to our work structure */
	struct sensor_work *sw = (struct sensor_work *)work;
	struct brvsens_event evt;
	int result = 0;

	if (dbg)
		printk("++++ Workqueue [%s] starting ++++\n",
				sw->name);
	evt.type = sw->handle;

	/* poll in the loop while not told to quit */
	while (atomic_read(&(sw->exit)) == 0) {
		/* always first read from the sensor, then sleep  */
		/* (so that apps that just activated the queue don't have */
		/*  to wait for the first measurement!) */
		result = sw->readCbk(sw->context, &(evt.data[0]));

		/* stick data to fifo event buffer  */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36)
		kfifo_in_locked(&(brvsens_data.ebuff), &evt,
			sizeof(struct brvsens_event), &(brvsens_data.lock));
#else
		kfifo_in_spinlocked(&(brvsens_data.ebuff), &evt,
			1, &(brvsens_data.lock));  /* more civilized */
#endif

		/* wake up main process, asleep in fops::read */
		wake_up_interruptible(&(brvsens_data.esig));

		/* pause */
		msleep_interruptible(atomic_read(&(sw->pollrate)));

	}

	if (dbg)
		printk("+++ Workqueue [%s] terminating +++\n",
			 sw->name);

	return;
}


/* Utility: Search for node by handle -- Unique ID */
static struct brvnode *find_node(unsigned int handle)
{
	struct list_head *pos  = 0;
	struct list_head *q    = 0;
	struct brvnode   *tmp  = 0;

	list_for_each_safe(pos, q, &(brvsens_data.dnodes.list))
	{
		/* get this node (Do NOT compare on Names,
		   as same driver might be exposing more than 1 reading
		   (i.e. bmp18x-core = pressure + temperature).
		   Handles are unique */
		tmp = list_entry(pos, struct brvnode, list);
		if (tmp->psw->handle == handle)
			return tmp;
	}

	return 0;  /* not found */
}

/* Utility: Search for first active node with same name, but different handle
   Helps us identify sensor registrations where single driver supports
   multiple Sensors */
static struct brvnode *find_active_node(struct brvnode* pn)
{
	struct list_head *pos  = 0;
	struct list_head *q    = 0;
	struct brvnode   *tmp  = 0;

	list_for_each_safe(pos, q, &(brvsens_data.dnodes.list))
	{
		tmp = list_entry(pos, struct brvnode, list);
		if ((strcmp(pn->psw->name, tmp->psw->name) == 0) &&
		    (tmp->psw->handle != pn->psw->handle) && (tmp->pwq != 0))
			return tmp;
	}

	return 0;  /* not found */
}

/* Helper called as part of destructor (exit) to stop all workqueues (if any)
   and deallocate/deregister all nodes */
static void free_nodes(void)
{
	struct list_head *pos  = 0;
	struct list_head *q    = 0;
	struct brvnode   *tmp  = 0;

	list_for_each_safe(pos, q, &(brvsens_data.dnodes.list))
	{
		/* get this node */
		tmp = list_entry(pos, struct brvnode, list);

		/* signal exit */
		atomic_set(&(tmp->psw->exit), 1);

		msleep_interruptible(DEFAULT_DELAY_MSEC);

		/* deallocate resources */
		destroy_workqueue(tmp->pwq);
		ACTIVATE_SENSOR(tmp->psw, 0);

		/* deallocate work */
		kfree(tmp->psw);

		/* remove the node itself */
		list_del(pos); kfree(tmp);
	}
}

/* Helper to deactivate existing node. We terminate workqueue, but leave
   work structure for next time around. This is always initiated when last
   sensor reference in User Land has been cleared, as part of
   user app termination

   Actual work definition is part of Driver registration. The only time
   we clear that is when all nodes are freed -- on exit */
static int deactivate_node(struct brvnode *pn)
{
	/* check workqueue */
	if (pn->pwq == 0) {
		if (dbg)
			printk("++++ %s: Node [%s], Handle [0x%x]"
				"not Active! ++++\n", __func__, pn->psw->name,
				pn->psw->handle);
	} else {
		atomic_set(&(pn->psw->exit), 1);

		/* pause */
		msleep_interruptible(DEFAULT_DELAY_MSEC);

		if (dbg)
			printk("++++ %s: Node [%s], Handle [0x%x] deactivated ++++\n",
				__func__, pn->psw->name, pn->psw->handle);

		/* remove workqueue */
		flush_workqueue(pn->pwq);
		destroy_workqueue(pn->pwq);
		pn->pwq = 0;
	}

    /* deactivate sensor ONLY if there is no other node with same name
       - as single sensor device can support different readings
       (i.e. bmp18-x --> Pressure + Temperature) */
	if (find_active_node(pn) == 0) {
		if (dbg)
			printk("++++ %s: Deactivating Sensor [%s] ++++\n",
			__func__, pn->psw->name);
		ACTIVATE_SENSOR(pn->psw, 0);
	} else {
		if (dbg)
			printk("++++ %s: Not deactivating Sensor [%s]"
			" - still in use! ++++\n", __func__, pn->psw->name);
	}

	return 0;
}

/*
  Helper to activate driver node -- start work
*/
static int activate_node(struct brvnode *pn)
{
	if (pn->pwq) {
		if (dbg)
			printk("++++ %s: Node [%s], Handle [0x%x] already Active! ++++\n",
				__func__, pn->psw->name, pn->psw->handle);

		return 0;
	}

	pn->pwq = create_workqueue(pn->psw->name);

	if (pn->pwq == 0) {
		printk(KERN_ERR "%s: -- memory allocation error!", __func__);
		return -ENOMEM;
	}

    /* ensure sensor is active */
	if (find_active_node(pn) == 0)
		ACTIVATE_SENSOR(pn->psw, 1);

    /* queue new work */
	atomic_set(&(pn->psw->exit), 0);   /* must because
				 it might have been reset last time! */
	INIT_WORK((struct work_struct *)pn->psw, sensor_poll);
	queue_work(pn->pwq, (struct work_struct *)pn->psw);

	if (dbg)
		printk("++++ %s: Sensor [%s] Activated."
			" Handle [0x%x], Delay [%u] ms ++++\n",
			__func__, pn->psw->name, pn->psw->handle,
			atomic_read(&(pn->psw->pollrate)));

	return 0;   /* all ok */
}

/*
  fops::read

  1. Wait Queue block -- signal that data is available in event buffer;

  2. Copy from kfifo buffer to user space and adjusts read pointer
  We don't worry about concurrency as this is one reader,
  multiple writers scenario

  Note: This is directly invoked from "pollEvents" libsensors API
*/
static ssize_t brvsens_read(struct file *file, char *buf,
				size_t count, loff_t *ptr)
{
	u32 iRead = 0;
	unsigned int res = 0;
	/* wait for event that data is available */
	wait_event_interruptible((brvsens_data.esig),
			(kfifo_len(&(brvsens_data.ebuff)) > 0));

	/* transfer to user land */
	res = kfifo_to_user(&(brvsens_data.ebuff), buf, count, &iRead);
	if (res < 0) {
		printk(KERN_ERR "[%s]: failed to copy data"
			" to user\n", __func__);
		return -EFAULT;
	}
	return iRead;
}

/*
  ioctl device interface: Supported commands:

  -- Activate: Activates / Deactivates a sensor
  -- SetDelay: Sets constant rate by which sensor data is reported
*/
static long brvsens_ioctl
(
	struct file   *file,
	unsigned int  cmd,
	unsigned long arg
)
{
	int err = 0;
	struct brvsens_cmd command;

	/* check cmd */
	if (_IOC_TYPE(cmd) != BRVSENS_DRIVER_IOC_MAGIC)	{
		printk(KERN_ERR "[%s]::cmd magic type error\n", __func__);
		return -EINVAL;
	}

	/* cmd mapping */
	switch (cmd) {
	/* activate node: Must be registered */
	case BRVSENS_DRIVER_IOC_ACTIVATE:
	{
		struct brvnode *pn = 0;

		/* extract command */
		if (copy_from_user(&command,
			(struct brvsens_cmd *)arg,
			sizeof(struct brvsens_cmd)) != 0) {
			printk(KERN_ERR "[%s]::copy_from_user error\n",
					__func__);
			return -EFAULT;
		}

		if (dbg) {
			printk("++++ %s:BRVSENS_DRIVER_IOC_ACTIVATE Handle: [0x%x], Delay: [%d] msec, Activate: [%d] ++++\n",
				__func__, command.handle,
				command.delay, command.enable);
		}

		/* Driver must register sensor in order to be activated */
		pn = find_node(command.handle);
		if (pn == 0) {
			printk(KERN_ERR "[%s]::Sensor Node [0x%x] not Registered!",
				__func__, command.handle);

			err = -ENODEV;
		} else {
			err = command.enable ? activate_node(pn)
				 : deactivate_node(pn);
		}
	}
	break;

	/* Set polling rate -- node must be registered */
	case BRVSENS_DRIVER_IOC_SET_DELAY:
	{
		struct brvnode *pn = 0;

		/* extract command */
		if (copy_from_user(&command, (struct brvsens_cmd *)arg,
					sizeof(struct brvsens_cmd)) != 0) {
			printk(KERN_ERR "[%s]::copy_from_user error\n",
					__func__);
			return -EFAULT;
		}

		if (dbg) {
			printk("++++ %s:BRVSENS_DRIVER_IOC_SET_DELAY Handle: [0x%x], Delay: [%d] msec, Activate: [%d] ++++\n",
				__func__, command.handle,
				command.delay, command.enable);
		}

		/* driver must register sensor  */
		pn = find_node(command.handle);
		if (pn == 0) {
			printk(KERN_ERR "[%s]::Sensor Node [0x%x] not Registered!",
				__func__, command.handle);

			err = -ENODEV;
		} else {
			atomic_set(&(pn->psw->pollrate), command.delay);
			if (dbg)
				printk("++++ %s: Polling Rate for node [0x%x (%s)] set to [%d] msec ++++\n",
					__func__, pn->psw->handle,
					pn->psw->name, command.delay);
		}

	}
	break;

    /* Board configuration. Return bitmask of
	registered sensors (brvsens nodes) */
	case BRVSENS_DRIVER_IOC_CONFIG:
	{
		struct list_head *pos  = 0;
		struct list_head *q    = 0;
		struct brvnode	 *tmp  = 0;
		unsigned int iMask = 0;

		/* build bitmask */
		list_for_each_safe(pos, q, &(brvsens_data.dnodes.list))
		{
			tmp = list_entry(pos, struct brvnode, list);
			iMask |= tmp->psw->handle;
		}

		/* transfer to user buffer */
		if (copy_to_user((unsigned int *)arg, &iMask,
				sizeof(unsigned int)) != 0) {
			printk(KERN_ERR "[%s]::BRVSENS_DRIVER_IOC_CONFIG"
				" -- copy_to_user error\n", __func__);
			err = -EFAULT;
		}
	}
	break;

	default:
		printk(KERN_ERR "[%s]::Cmd number error", __func__);
		err = -EBADRQC;
	}


	if (dbg)
		printk("++++ %s::END. Result: [%d] ++++\n",
				__func__, err);
	return err;
}

/* FOPS on /dev/brvsens. Mapping

   --- brvsens_read:   poll events
   --- brvsens_ioctl:  enable/disable + setDelay

*/
static const struct file_operations brvsens_fops = {
	.owner            = THIS_MODULE,
	.read             = brvsens_read,
	.unlocked_ioctl   = brvsens_ioctl,
};


/* Constructor

   Allocate and register brvsens device. Note:
   -- Driver Registration performs allocation of brvsens nodes
   -- Node Activation/Deactivation is part of ioctl::activate
*/
static int __init brvsens_init(void)
{
	int rc = 0;
	struct  device *dev = 0;

	if (dbg)
		printk("++++ %s::BEGIN ++++\n", __func__);

	/* allocate driver region */
	rc = alloc_chrdev_region(&(brvsens_data.devnum), 0, 1, DRIVER_NAME);
	if (rc < 0) {
		printk(KERN_ERR "%s: alloc_chrdev_region failed: %d\n",
				__func__, rc);
		goto out_err;
	}

    /* add driver fops interface */
	cdev_init(&(brvsens_data.cdev), &brvsens_fops);
	rc = cdev_add(&(brvsens_data.cdev), brvsens_data.devnum, 1);
	if (rc != 0) {
		printk(KERN_ERR "%s: cdev_add failed: %d\n", __func__, rc);
		goto out_unregister;
	}

    /* create driver class */
	brvsens_data.class = class_create(THIS_MODULE, DRIVER_NAME);
	if (IS_ERR(brvsens_data.class)) {
		rc = PTR_ERR(brvsens_data.class);
		printk(KERN_ERR "%s: class_create failed: %d\n", __func__, rc);

		goto out_cdev_del;
	}

    /* finally create brvsens device */
	dev = device_create(brvsens_data.class, NULL,
			brvsens_data.devnum, NULL, DRIVER_NAME);
	if (IS_ERR(dev)) {
		rc = PTR_ERR(dev);
		printk(KERN_ERR "%s: device_create failed: %d\n", __func__, rc);

		goto out_class_destroy;
	}

	/* initialize kfifo event buffer */
	INIT_KFIFO(brvsens_data.ebuff);

    /* initialize spinlock */
	spin_lock_init(&(brvsens_data.lock));

    /* initialize link list of sensor nodes */
	INIT_LIST_HEAD(&(brvsens_data.dnodes.list));

	/* init wait queue */
	init_waitqueue_head(&(brvsens_data.esig));

	if (dbg)
		printk("++++ %s::END ++++\n", __func__);
	return 0;

out_class_destroy:
	class_destroy(brvsens_data.class);
	brvsens_data.class = NULL;

out_cdev_del:
	cdev_del(&(brvsens_data.cdev));

out_unregister:
	unregister_chrdev_region(brvsens_data.devnum, 1);

out_err:
	return rc;
}

/* Destructor:

   Deregister IRQ handlers
   Stop Workqueues
   Deallocate Sensor Node List
   Deallocate device itself
*/
static void __exit brvsens_exit(void)
{
	if (dbg)
		printk("++++ %s::BEGIN ++++\n", __func__);

    /* free list nodes; this will also stop all workqueues */
	free_nodes();

    /* deallocate /dev/brvsens device */
	device_destroy(brvsens_data.class,  brvsens_data.devnum);
	class_destroy(brvsens_data.class);
	cdev_del(&(brvsens_data.cdev));
	unregister_chrdev_region(brvsens_data.devnum, 1);

	if (dbg)
		printk("++++ %s::END ++++\n", __func__);
}



/* Sensor Registration: Export Where specific drivers
			register with BRVSENS device */
int brvsens_register(
	u32          handle,   /* Sensor UID, must be provided and unique */
	const char   *name,     /* Name: Must be provided, duplicates OK
				(i.e. different sensors from same driver) */
	void         *context,  /* Callback context (i.e. struct i2c_device*),
				driver specific. Null ok */
	PFNACTIVATE  actCbk,   /* Activation callback. Null ok  */
	PFNREAD      readCbk)   /* Read Callback. Must be provided */
{
	struct brvnode           *pn  = 0;
	struct sensor_work       *psw = 0;

    /* input validation -- handle, name and readCallback must be provided */
	if ((handle == 0) || (!name) || (readCbk == 0)) {
		printk(KERN_ERR "%s: -- Invalid Registration Data\n", __func__);
		return -EFAULT;
	}

    /* input validation -- handle must be unique */
	if (find_node(handle) != 0) {
		if (dbg)
			printk("++++ %s: Sensor [0x%x]"
				" already registered! ++++\n",
				__func__, handle);
		return -EFAULT;
	}

	if (dbg)
		printk("++++ %s: handle [0x%x],"
				" name [%s] ++++\n", __func__, handle, name);

    /* allocate node and work structure */
	psw = kmalloc(sizeof(struct sensor_work), GFP_KERNEL);
	if (psw)
		pn = kmalloc(sizeof(struct brvnode), GFP_KERNEL);

	if (!pn) {
		kfree(psw);
		printk(KERN_ERR "%s: -- memory allocation error!", __func__);

		return -ENOMEM;
	}

    /* configure work structure */
	psw->handle = handle;
	atomic_set(&(psw->pollrate), DEFAULT_DELAY_MSEC);
	atomic_set(&(psw->exit), 0);

	strcpy(psw->name, name);
	psw->context = context;
	psw->activateCbk = actCbk;
	psw->readCbk = readCbk;

	/* configure new node and add it to the list */
	pn->pwq = 0;
	pn->psw = psw;

	list_add_tail(&(pn->list), &(brvsens_data.dnodes.list));

	if (dbg)
		printk("++++ %s: Sensor [%s] Registered. Handle [0x%x], Delay [%u] ms ++++\n",
			__func__, psw->name, pn->psw->handle,
			atomic_read(&(psw->pollrate)));

	return 0;   /* all ok */
}
/* just like DllExport */
EXPORT_SYMBOL(brvsens_register);

/* driver de-registration. Initially for completeness only; has no use
    for Android which takes one-time snapshot at SensorService initialization &
    does not query again (omni-potent Google frequently sucks) */
int brvsens_deregister(u32 handle)
{
	struct list_head *pos  = 0;
	struct list_head *q    = 0;
	struct brvnode   *tmp  = 0;

	if (dbg)
		printk("++++ %s: handle [0x%x] ++++\n",
			__func__, handle);

	list_for_each_safe(pos, q, &(brvsens_data.dnodes.list))
	{
		/* get this node */
		tmp = list_entry(pos, struct brvnode, list);

		if (tmp->psw->handle == handle) {
			/* deactivate node first
			(This will clear the workqueue) */
			deactivate_node(tmp);

			/* deallocate work structure */
			kfree(tmp->psw);

			/* remove the node itself */
			list_del(pos); kfree(tmp);

			if (dbg)
				printk("++++ %s: Sensor Driver [0x%x] successfully deregistered! ++++\n",
					__func__, handle);

			return 0;
		}
	}

    /* if here, handle is not found so log an error */
	printk(KERN_ERR "%s: Sensor Driver [0x%x] has not "
					"been registered ++++\n",
					__func__, handle);

	return -EFAULT;
}
/* just like DllExport */
EXPORT_SYMBOL(brvsens_deregister);

module_init(brvsens_init);
module_exit(brvsens_exit);

MODULE_DESCRIPTION("Broadcom Virtual Sensor Driver");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Broadcom Corporation");
MODULE_VERSION("1:0.0");
