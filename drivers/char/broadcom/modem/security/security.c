/******************************************************************************
Copyright 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement
governing use of this software, this software is licensed to you under the
terms of the GNU General Public License version 2, available at
http://www.gnu.org/copyleft/gpl.html (the "GPL").

Notwithstanding the above, under no circumstances may you combine this software
in any way with any other Broadcom software provided under a license other than
the GPL, without Broadcom's express prior written consent
******************************************************************************/

/***************************************************************************/
/**
*
*   @file   security.c
*
*   @brief  This driver is used to provide support for Broadcom security
*           functions such as SIM lock.
*
*
****************************************************************************/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/device.h>

#include <linux/fs.h>
#include <linux/fcntl.h>
#include <linux/list.h>
#include <linux/ioctl.h>
#include <linux/types.h>
#include <linux/unistd.h>
#include <linux/errno.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/jiffies.h>
#include <asm/pgtable.h>
#include <linux/io.h>
#include <linux/proc_fs.h>

#include <linux/broadcom/bcm_major.h>
#include "bcmlog.h"
#include <linux/broadcom/bcm_security.h>
#include "mobcom_types.h"
#include "resultcode.h"
#include "taskmsgs.h"
#include "simlockfun.h"

/*For RPC*/
#include <linux/broadcom/ipcproperties.h>
#include "rpc_global.h"
#include "rpc_ipc.h"
#include "xdr_porting_layer.h"
#include "xdr.h"
#include "rpc_api.h"
#include "sys_common_rpc.h"


MODULE_LICENSE("GPL");

/**
 *  module data
 */
struct _SEC_Module_t {
	struct class *mDriverClass;
};
#define SEC_Module_t struct _SEC_Module_t

/**
 *  module status
 */
static SEC_Module_t sModule = { 0 };

/**
 *  private data for each session
 */
struct _SEC_PrivData_t {
	struct file *mUserfile;
	/* **FIXME** MAG - anything else needed? */
};
#define SEC_PrivData_t struct _SEC_PrivData_t

/* local function protos */

/* forward declarations used in 'struct file_operations' */

static int sec_open(struct inode *inode, struct file *file);
static int sec_release(struct inode *inode, struct file *file);
static ssize_t sec_read(struct file *filep, char __user *buf, size_t len,
		 loff_t *off);
static ssize_t sec_write(struct file *filep, const char __user *buf, size_t len,
		  loff_t *off);
static long sec_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
static unsigned int sec_poll(struct file *filp, poll_table * wait);

/*****************************************************************/
static long handle_is_lock_on_ioc(struct file *filp, unsigned int cmd,
				  unsigned long param);
static long handle_set_lock_ioc(struct file *filp, unsigned int cmd,
				unsigned long param);
static long handle_unlock_sim_ioc(struct file *filp, unsigned int cmd,
				  unsigned long param);
static long handle_get_lock_state_ioc(struct file *filp, unsigned int cmd,
				      unsigned long param);
static long handle_get_remain_attempt_info_ioc(struct file *filp,
					       unsigned int cmd,
					       unsigned long param);

/*****************************************************************/

/**
 *  file ops
 */
static const struct file_operations sec_ops = {
	.owner = THIS_MODULE,
	.open = sec_open,
	.read = sec_read,
	.write = sec_write,
	.unlocked_ioctl = sec_ioctl,
	.poll = sec_poll,
	.release = sec_release,
};

/* ============================File operations============================= */

static int sec_open(struct inode *inode, struct file *file)
{
	int ret = 0;
	SEC_PrivData_t *priv;

	pr_info("sec_open begin file=%x\n", (int)file);

	priv = kmalloc(sizeof(SEC_PrivData_t), GFP_KERNEL);

	if (!priv) {
		pr_info("rpcipc_open mem allocation fail file=%x\n", (int)file);
		ret = -ENOMEM;
	} else {
		priv->mUserfile = file;
		file->private_data = priv;
	}

	return ret;

}

static int sec_release(struct inode *inode, struct file *file)
{
	SEC_PrivData_t *priv = file->private_data;

	BUG_ON(!priv);

	pr_info("sec_release begin file=%x\n", (int)file);

	kfree(priv);

	return 0;
}

static ssize_t sec_read(struct file *filep, char __user *buf, size_t len,
		 loff_t *off)
{
	return -EFAULT;
}

static ssize_t sec_write(struct file *filep, const char __user *buf,
		  size_t len, loff_t *off)
{
	return -EFAULT;
}

static unsigned int sec_poll(struct file *filp, poll_table *wait)
{
	return -EFAULT;
}

static long sec_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	long retVal = 0;

	if (_IOC_TYPE(cmd) != BCM_SEC_IOC_MAGIC
	    || _IOC_NR(cmd) >= SEC_SIMLOCK_IOC_MAXNR) {
		pr_err("sec_ioctl ERROR cmd=0x%x\n", cmd);
		return -ENOIOCTLCMD;
	}

	switch (cmd) {
	case SEC_SIMLOCK_IS_LOCK_ON_IOC:
		{
			retVal = handle_is_lock_on_ioc(filp, cmd, arg);
			break;
		}
	case SEC_SIMLOCK_SET_LOCK_IOC:
		{
			retVal = handle_set_lock_ioc(filp, cmd, arg);
			break;
		}
	case SEC_SIMLOCK_UNLOCK_SIM_IOC:
		{
			retVal = handle_unlock_sim_ioc(filp, cmd, arg);
			break;
		}
	case SEC_SIMLOCK_GET_LOCK_STATE_IOC:
		{
			retVal = handle_get_lock_state_ioc(filp, cmd, arg);
			break;
		}
	case SEC_SIMLOCK_GET_REMAIN_ATTMPT_IOC:
		{
			retVal =
			    handle_get_remain_attempt_info_ioc(filp, cmd, arg);
			break;
		}
	default:
		retVal = -ENOIOCTLCMD;
		pr_err("sec_ioctl ERROR unhandled cmd=0x%x\n", cmd);
		break;
	}

	return retVal;
}

static long handle_is_lock_on_ioc(struct file *filp, unsigned int cmd,
				  unsigned long param)
{
	Boolean bLocked = FALSE, bFullLock = FALSE;
	sec_simlock_islockon_t ioc_param = { 0 };

	if (copy_from_user
	    (&ioc_param, (sec_simlock_islockon_t *) param,
	     sizeof(sec_simlock_islockon_t)) != 0) {
		pr_err("handle_is_lock_on_ioc - copy_from_user() had error\n");
		return -EFAULT;
	}

	/* **FIXME** need a SIM ID param for SIMLockIsLockOn */
	bLocked = SIMLockIsLockOn(ioc_param.lock_type, &bFullLock);
	ioc_param.lock_enabled = bLocked ? 1 : 0;
	ioc_param.full_lock_enabled = bFullLock ? 1 : 0;

	if (copy_to_user
	    ((sec_simlock_islockon_t *) param, &ioc_param,
	     sizeof(sec_simlock_islockon_t)) != 0) {
		pr_err("handle_is_lock_on_ioc - copy_to_user() had error\n");
		return -EFAULT;
	}

	return 0;
}

static long handle_set_lock_ioc(struct file *filp, unsigned int cmd,
				unsigned long param)
{
	sec_simlock_set_lock_t ioc_param = { 0 };
	sec_simlock_state_t sec_lock_state = { 0 };
	SYS_SIMLOCK_STATE_t sys_lock_status = { 0 };

	if (copy_from_user
	    (&ioc_param, (sec_simlock_set_lock_t *) param,
	     sizeof(sec_simlock_set_lock_t)) != 0) {
		pr_err("handle_set_lock_ioc - copy_from_user() had error\n");
		return -EFAULT;
	}

	/* try setting lock */
	ioc_param.set_lock_status = SIMLockSetLock(ioc_param.sim_id,
						   (UInt8) ioc_param.action,
						   ioc_param.
						   full_lock_on ? TRUE : FALSE,
						   ioc_param.lock_type,
						   ioc_param.key);
	ioc_param.remain_attempt = (int)SIMLockGetRemainAttempt(ioc_param.
						   sim_id);

	SIMLockGetSIMLockState(ioc_param.sim_id, &sec_lock_state);

	sys_lock_status.network_lock_enabled =
				sec_lock_state.network_lock_enabled;
	sys_lock_status.network_subset_lock_enabled =
				sec_lock_state.network_subset_lock_enabled;
	sys_lock_status.service_provider_lock_enabled =
				sec_lock_state.service_provider_lock_enabled;
	sys_lock_status.corporate_lock_enabled =
				sec_lock_state.corporate_lock_enabled;
	sys_lock_status.phone_lock_enabled =
				sec_lock_state.phone_lock_enabled;
	sys_lock_status.network_lock =
				sec_lock_state.network_lock;
	sys_lock_status.network_subset_lock =
				sec_lock_state.network_subset_lock;
	sys_lock_status.service_provider_lock =
				sec_lock_state.service_provider_lock;
	sys_lock_status.corporate_lock = sec_lock_state.corporate_lock;
	sys_lock_status.phone_lock = sec_lock_state.phone_lock;

	SIMLOCKApi_SetStatusEx(ioc_param.sim_id, &sys_lock_status);

	if (copy_to_user
	    ((sec_simlock_set_lock_t *) param, &ioc_param,
	     sizeof(sec_simlock_set_lock_t)) != 0) {
		pr_err("handle_set_lock_ioc - copy_to_user() had error\n");
		return -EFAULT;
	}

	return 0;
}

static long handle_unlock_sim_ioc(struct file *filp, unsigned int cmd,
				  unsigned long param)
{
	sec_simlock_unlock_t ioc_param = { 0 };
	sec_simlock_state_t sec_lock_state = { 0 };
	SYS_SIMLOCK_STATE_t sys_lock_status = { 0 };

	if (copy_from_user
	    (&ioc_param, (sec_simlock_unlock_t *) param,
	     sizeof(sec_simlock_unlock_t)) != 0) {
		pr_err("handle_unlock_sim_ioc - copy_from_user() had error\n");
		return -EFAULT;
	}

	/* try to unlock here, and set ioc_param result fields appropriately */
	ioc_param.unlock_status = SIMLockUnlockSIM(ioc_param.sim_id,
						   ioc_param.lock_type,
						   ioc_param.password);

	ioc_param.remain_attempt = (int)SIMLockGetRemainAttempt(ioc_param.
						   sim_id);
	SIMLockGetSIMLockState(ioc_param.sim_id, &sec_lock_state);

	sys_lock_status.network_lock_enabled =
				sec_lock_state.network_lock_enabled;
	sys_lock_status.network_subset_lock_enabled =
				sec_lock_state.network_subset_lock_enabled;
	sys_lock_status.service_provider_lock_enabled =
				sec_lock_state.service_provider_lock_enabled;
	sys_lock_status.corporate_lock_enabled =
				sec_lock_state.corporate_lock_enabled;
	sys_lock_status.phone_lock_enabled =
				sec_lock_state.phone_lock_enabled;
	sys_lock_status.network_lock =
				sec_lock_state.network_lock;
	sys_lock_status.network_subset_lock =
				sec_lock_state.network_subset_lock;
	sys_lock_status.service_provider_lock =
				sec_lock_state.service_provider_lock;
	sys_lock_status.corporate_lock = sec_lock_state.corporate_lock;
	sys_lock_status.phone_lock = sec_lock_state.phone_lock;

	SIMLOCKApi_SetStatusEx(ioc_param.sim_id, &sys_lock_status);

	if (copy_to_user
	    ((sec_simlock_unlock_t *) param, &ioc_param,
	     sizeof(sec_simlock_unlock_t)) != 0) {
		pr_err("handle_unlock_sim_ioc - copy_to_user() had error\n");
		return -EFAULT;
	}

	return 0;
}

static long handle_get_lock_state_ioc(struct file *filp, unsigned int cmd,
				      unsigned long param)
{
	sec_simlock_state_t ioc_param = { 0 };

	if (copy_from_user
	    (&ioc_param, (sec_simlock_state_t *) param,
	     sizeof(sec_simlock_state_t)) != 0) {
		pr_err
		("handle_get_lock_state_ioc - copy_from_user() had error\n");
		return -EFAULT;
	}

	/* retrieve current lock state */
	SIMLockGetSIMLockState(ioc_param.sim_id, &ioc_param);

	if (copy_to_user
	    ((sec_simlock_state_t *) param, &ioc_param,
	     sizeof(sec_simlock_state_t)) != 0) {
		pr_err
		("handle_get_lock_state_ioc - copy_to_user() had error\n");
		return -EFAULT;
	}

	return 0;
}

static long handle_get_remain_attempt_info_ioc(struct file *filp,
					       unsigned int cmd,
					       unsigned long param)
{
	sec_simlock_remain_t ioc_param = { 0 };

	if (copy_from_user(&ioc_param, (sec_simlock_remain_t *) param,
			   sizeof(sec_simlock_remain_t)) != 0) {
		pr_err
		("handle_get_remain_attempt_info_ioc: copy_from_user error\n");
		return -EFAULT;
	}

	/* retrieve remaining pin attempts */
	ioc_param.remain_attempt =
	    (int)SIMLockGetRemainAttempt(ioc_param.sim_id);

	if (copy_to_user((sec_simlock_remain_t *) param,
			 &ioc_param, sizeof(sec_simlock_remain_t)) != 0) {
		pr_err
		 ("handle_get_remain_attempt_info_ioc: copy_to_user error\n");
		return -EFAULT;
	}

	return 0;
}

/***************************************************************************/
/**
 *  Called by CP via sysrpc driver to get current SIM lock state
 *
 *  sim_data (in) - sim data for SIM being queried
 *  simID (in) - sim ID for SIM being queried
 *  is_test_sim (in) - 1 if test sim, 0 otherwise
 *  sim_lock_state (out) -  sim lock state for SIM queried (if success)
 *  return - 0 if success, -1 if error
 */
int sec_simlock_get_status(sec_simlock_sim_data_t *sim_data,
			   SEC_SimLock_SimNumber_t simID,
			   int is_test_sim,
			   sec_simlock_state_t *sim_lock_state)
{
	int result = 0;
	pr_err("%s:SIM%d; IMSI=%s; is_test_sim = %d\n", __func__,
						simID, sim_data->imsi_string,
						is_test_sim);
	if (!sim_data || !sim_lock_state) {
		result = -1;
	} else {
		/* cache current SIM data */
		SIMLockSetSimData(sim_data, simID);

		/* initialize the result data */
		sim_lock_state->network_lock_enabled = 0;
		sim_lock_state->network_subset_lock_enabled = 0;
		sim_lock_state->service_provider_lock_enabled = 0;
		sim_lock_state->corporate_lock_enabled = 0;
		sim_lock_state->phone_lock_enabled = 0;
		sim_lock_state->network_lock = SEC_SIMLOCK_SECURITY_OPEN;
		sim_lock_state->network_subset_lock =
					SEC_SIMLOCK_SECURITY_OPEN;
		sim_lock_state->service_provider_lock_enabled =
					SEC_SIMLOCK_SECURITY_OPEN;
		sim_lock_state->corporate_lock_enabled =
					SEC_SIMLOCK_SECURITY_OPEN;
		sim_lock_state->phone_lock_enabled =
					SEC_SIMLOCK_SECURITY_OPEN;

#ifdef CONFIG_BRCM_SIM_SECURE_ENABLE
		/* update lock state */
		SIMLockCheckAllLocks(simID,
			sim_data->imsi_string,
			(0 != sim_data->gid1_len ? sim_data->gid1 : NULL),
			(0 != sim_data->gid2_len ? sim_data->gid2 : NULL));
		SIMLockUpdateSIMLockState(simID);

		/* retrieve current SIM lock state for this SIM */
		SIMLockGetSIMLockState(simID, sim_lock_state);
#endif
	}

	return result;
}

static int major;
/***************************************************************************/
/**
 *  Called by Linux I/O system to initialize module
 *  @return int     0 if success, -1 if error
 *  @note
 *      API is defined by module_init macro
 */
static int __init bcm_sec_ModuleInit(void)
{
	struct device *drvdata;
	pr_info("enter bcm_sec_ModuleInit()\n");

	major = register_chrdev(0, BCM_SEC_NAME, &sec_ops);
	if (major < 0) {
		pr_err("register_chrdev failed\n");
		return major;
	}

	sModule.mDriverClass = class_create(THIS_MODULE, BCM_SEC_NAME);
	if (IS_ERR(sModule.mDriverClass)) {
		pr_err("driver class_create failed\n");
		unregister_chrdev(major, BCM_SEC_NAME);
		return PTR_ERR(sModule.mDriverClass);
	}

	drvdata =
	    device_create(sModule.mDriverClass, NULL, MKDEV(major, 0), NULL,
			  BCM_SEC_NAME);
	if (IS_ERR(drvdata)) {
		pr_err("device_create_drvdata failed\n");
		unregister_chrdev(major, BCM_SEC_NAME);
		return PTR_ERR(drvdata);
	}

	pr_info("exit bcm_sec_ModuleInit()\n");
	return 0;
}

/***************************************************************************/
/**
 *  Called by Linux I/O system to exit module.
 *  @return int     0 if success, -1 if error
 *  @note
 *      API is defined by module_exit macro
 **/
static void __exit bcm_sec_ModuleExit(void)
{
	pr_info("bcm_sec_ModuleExit()\n");

	device_destroy(sModule.mDriverClass, MKDEV(major, 0));
	class_destroy(sModule.mDriverClass);
	unregister_chrdev(major, BCM_SEC_NAME);

}

/**
 *  export module init and export functions
 **/
module_init(bcm_sec_ModuleInit);
module_exit(bcm_sec_ModuleExit);
