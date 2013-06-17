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
#include <linux/reboot.h>	 /* For kernel_power_off() */

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
#include <linux/dma-mapping.h>

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

#ifdef CONFIG_BRCM_EMMC_RPMB_SUPPORT
/* For eMMC RPMB access */
#include <linux/emmc_rpmb.h>
#endif

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
 * module device
 */
static struct device *drvdata;


#define PAGE_SIZE_UP(addr)	(((addr)+((PAGE_SIZE)-1))&(~((PAGE_SIZE)-1)))
#define IS_PAGE_ALIGNED(addr)	(!((addr) & (~PAGE_MASK)))

/**
 *  private data for each session
 */
struct _SEC_PrivData_t {
	struct file *mUserfile;
	/* Lock for the allocation list and ioctl/mmap */
	struct mutex lock;
	dma_addr_t handle;
	void *dma_addr;
	unsigned long size;
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
static int sec_mmap(struct file *filp, struct vm_area_struct *vma);

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
static long handle_get_imei_ioc(struct file *filp, unsigned int cmd,
				 unsigned long param);
static long handle_set_lock_state_ioc(struct file *filp, unsigned int cmd,
				unsigned long param);
static Boolean read_imei(UInt8 *imeiStr1, UInt8 *imeiStr2);

static int handle_sec_mem_req(struct file *filp, unsigned int cmd,
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
	.mmap = sec_mmap,
	.release = sec_release,
};

/* ============================File operations============================= */

static int sec_open(struct inode *inode, struct file *file)
{
	int ret = 0;
	SEC_PrivData_t *priv;

	pr_info("sec_open begin file=%x\n", (int)file);

	priv = kzalloc(sizeof(SEC_PrivData_t), GFP_KERNEL);

	if (!priv) {
		pr_info("rpcipc_open mem allocation fail file=%x\n", (int)file);
		ret = -ENOMEM;
	} else {
		priv->mUserfile = file;
		file->private_data = priv;
		mutex_init(&priv->lock);
	}

	return ret;

}

static int sec_release(struct inode *inode, struct file *file)
{
	SEC_PrivData_t *priv = file->private_data;

	BUG_ON(!priv);

	pr_info("sec_release begin file=%x\n", (int)file);

	if (!priv->size || !priv->dma_addr || !priv->handle)
		goto out_free;

	dma_free_coherent(drvdata, priv->size, priv->dma_addr, priv->handle);

out_free:
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
			pr_info("SEC_SIMLOCK_IS_LOCK_ON_IOC\n");
			retVal = handle_is_lock_on_ioc(filp, cmd, arg);
			break;
		}
	case SEC_SIMLOCK_SET_LOCK_IOC:
		{
			pr_info("SEC_SIMLOCK_SET_LOCK_IOC\n");
			retVal = handle_set_lock_ioc(filp, cmd, arg);
			break;
		}
	case SEC_SIMLOCK_UNLOCK_SIM_IOC:
		{
			pr_info("SEC_SIMLOCK_UNLOCK_SIM_IOC\n");
			retVal = handle_unlock_sim_ioc(filp, cmd, arg);
			break;
		}
	case SEC_SIMLOCK_GET_LOCK_STATE_IOC:
		{
			pr_info("SEC_SIMLOCK_GET_LOCK_STATE_IOC\n");
			retVal = handle_get_lock_state_ioc(filp, cmd, arg);
			break;
		}
	case SEC_SIMLOCK_GET_REMAIN_ATTMPT_IOC:
		{
			pr_info("SEC_SIMLOCK_GET_REMAIN_ATTMPT_IOC\n");
			retVal =
			    handle_get_remain_attempt_info_ioc(filp, cmd, arg);
			break;
		}
	case SEC_GET_IMEI_IOC:
		{
			pr_info("SEC_GET_IMEI_IOC\n");
			retVal = handle_get_imei_ioc(filp, cmd, arg);
			break;
		}
	case SEC_SIMLOCK_SET_LOCK_STATE_IOC:
		{
			pr_info("SEC_SIMLOCK_SET_LOCK_STATE_IOC\n");
			retVal = handle_set_lock_state_ioc(filp, cmd, arg);
			break;
		}

	case SEC_MEM_ALLOC:
			retVal = handle_sec_mem_req(filp, cmd, arg);
			break;
	default:
		retVal = -ENOIOCTLCMD;
		pr_err("sec_ioctl ERROR unhandled cmd=0x%x\n", cmd);
		break;
	}

	return retVal;
}


static int sec_mmap(struct file *filp, struct vm_area_struct *vma)
{
	int ret = 0;
	SEC_PrivData_t *priv = (SEC_PrivData_t *)filp->private_data;
	unsigned long vma_size;

	BUG_ON(!priv);

	if (!priv->size || !priv->dma_addr || !priv->handle) {
		pr_err("sec_mmap - no memory to be mmaped for this file"\
				"size(%lu), dma_addr(%p), handle(%u)\n",
				priv->size, priv->dma_addr, priv->handle);
		ret = -EINVAL;
		goto out;
	}

	if (vma->vm_pgoff) {
		pr_err("sec_mmap - don't support mmaping from an offset\n");
		ret = -EINVAL;
		goto out;
	}

	vma_size = vma->vm_end - vma->vm_start;
	if (!IS_PAGE_ALIGNED(vma_size) || vma_size != priv->size) {
		pr_err("sec_mmap - vma_size (%lu) is either not page aligned"\
				"or not equal to allocation size (%lu)\n",
				vma_size, priv->size);
		ret = -EINVAL;
		goto out;
	}

	/* always create dma_coherent (uncached) mappings */
	vma->vm_page_prot = pgprot_dmacoherent(vma->vm_page_prot);

	ret = io_remap_pfn_range(vma, vma->vm_start,
			virt_to_phys(priv->dma_addr) >> PAGE_SHIFT,
			vma_size, vma->vm_page_prot);
out:
	return ret;
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

	/* Coverity [TAINTED_SCALAR] */
	if (copy_from_user
	(&ioc_param, (sec_simlock_set_lock_t *) param,
	sizeof(sec_simlock_set_lock_t)) != 0) {
		pr_err("handle_set_lock_ioc - copy_from_user() had error\n");
		return -EFAULT;
	}

	if (ioc_param.sim_id >= DUAL_SIM_SIZE)	{
		pr_err("handle_set_lock_ioc - SIMid=%d invalid.\n",
		    ioc_param.sim_id);
		return -EFAULT;
	}

	/* try setting lock */
	ioc_param.set_lock_status = SIMLockSetLock(ioc_param.sim_id,
					(UInt8)ioc_param.action,
					ioc_param.full_lock_on ? TRUE
					: FALSE, ioc_param.lock_type,
					ioc_param.key);

	if (ioc_param.set_lock_status != SEC_SIMLOCK_FAILURE) {
		ioc_param.remain_attempt =
		    (int)SIMLockGetRemainAttempt(ioc_param.sim_id);

		/* Coverity [TAINTED_SCALAR] */
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
		sys_lock_status.network_lock = sec_lock_state.network_lock;
		sys_lock_status.network_subset_lock =
		    sec_lock_state.network_subset_lock;
		sys_lock_status.service_provider_lock =
		    sec_lock_state.service_provider_lock;
		sys_lock_status.corporate_lock = sec_lock_state.corporate_lock;
		sys_lock_status.phone_lock = sec_lock_state.phone_lock;

		SIMLOCKApi_SetStatusEx(ioc_param.sim_id, &sys_lock_status);
	}

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

	/* Coverity [TAINTED_SCALAR] */
	if (copy_from_user
		(&ioc_param, (sec_simlock_unlock_t *) param,
		sizeof(sec_simlock_unlock_t)) != 0) {
		pr_err("handle_unlock_sim_ioc - copy_from_user() had error\n");
		return -EFAULT;
	}

	if (ioc_param.sim_id >= DUAL_SIM_SIZE)	{
		pr_err("handle_unlock_sim_ioc - SIMid=%d invalid.\n",
		    ioc_param.sim_id);
		return -EFAULT;
	}

	/* try to unlock here, and set ioc_param result fields appropriately */
	ioc_param.unlock_status = SIMLockUnlockSIM(ioc_param.sim_id,
							ioc_param.lock_type,
							ioc_param.password);

	if (ioc_param.unlock_status != SEC_SIMLOCK_FAILURE) {
		ioc_param.remain_attempt =
		    (int)SIMLockGetRemainAttempt(ioc_param.sim_id);
		/* Coverity [TAINTED_SCALAR] */
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
		sys_lock_status.network_lock = sec_lock_state.network_lock;
		sys_lock_status.network_subset_lock =
		    sec_lock_state.network_subset_lock;
		sys_lock_status.service_provider_lock =
		    sec_lock_state.service_provider_lock;
		sys_lock_status.corporate_lock = sec_lock_state.corporate_lock;
		sys_lock_status.phone_lock = sec_lock_state.phone_lock;

		SIMLOCKApi_SetStatusEx(ioc_param.sim_id, &sys_lock_status);
	}

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

	/* Coverity [TAINTED_SCALAR] */
	if (copy_from_user
	    (&ioc_param, (sec_simlock_state_t *)param,
	     sizeof(sec_simlock_state_t)) != 0) {
		pr_err("handle_get_lock_state_ioc - copy_from_user() had error\n");
		return -EFAULT;
	}

	if (ioc_param.sim_id >= DUAL_SIM_SIZE)	{
		pr_err("handle_get_lock_state_ioc - SIMid=%d invalid.\n",
		    ioc_param.sim_id);
		return -EFAULT;
	}

	/* retrieve current lock state */
	/* Coverity [TAINTED_SCALAR]   */
	SIMLockGetSIMLockState(ioc_param.sim_id, &ioc_param);

	if (copy_to_user
	    ((sec_simlock_state_t *)param, &ioc_param,
	     sizeof(sec_simlock_state_t)) != 0) {
		pr_err("handle_get_lock_state_ioc - copy_to_user() had error\n");
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
		pr_err("handle_get_remain_attempt_info_ioc: copy_from_user error\n");
		return -EFAULT;
	}

	if (ioc_param.sim_id >= DUAL_SIM_SIZE)	{
		pr_err("handle_get_remain_attempt_info_ioc - SIMid=%d invalid.\n",
		    ioc_param.sim_id);
		return -EFAULT;
	}

	/* retrieve remaining pin attempts */
	ioc_param.remain_attempt =
	    (int)SIMLockGetRemainAttempt(ioc_param.sim_id);

	if (copy_to_user((sec_simlock_remain_t *) param,
			 &ioc_param, sizeof(sec_simlock_remain_t)) != 0) {
		pr_err("handle_get_remain_attempt_info_ioc: copy_to_user error\n");
		return -EFAULT;
	}

	return 0;
}

static long handle_get_imei_ioc(struct file *filp, unsigned int cmd,
						   unsigned long param)
{
	sec_get_imei_data_t ioc_param;

	if (copy_from_user(&ioc_param, (sec_get_imei_data_t *) param,
			   sizeof(sec_get_imei_data_t)) != 0) {
		pr_err("handle_get_imei_ioc: copy_from_user error\n");
		return -EFAULT;
	}
	memset(&ioc_param, 0x0, sizeof(sec_get_imei_data_t));

	if (!read_imei(ioc_param.imei1_string, ioc_param.imei2_string))	{
#ifdef CONFIG_BRCM_SIM_SECURE_ENABLE
		kernel_power_off();
#endif
		return -EFAULT;
	}
#ifdef CONFIG_BRCM_SIM_SECURE_ENABLE
	if (strlen(ioc_param.imei1_string) == 0) {
		pr_err("%s: IMEI_1 is empty or not programmed !!!", __func__);
		kernel_power_off();
		return -EFAULT;
	}

	if (FALSE == SetImeiData(SEC_SimLock_SIM_DUAL_FIRST,
		 (UInt8 *)ioc_param.imei1_string)) {
		pr_err("SetImei IMEI 1:%s Failed!!!", ioc_param.imei1_string);
		kernel_power_off();
		return -EFAULT;
	}
#endif

	if (copy_to_user((sec_get_imei_data_t *) param,
			 &ioc_param, sizeof(sec_get_imei_data_t)) != 0) {
		pr_err("handle_get_imei_ioc: copy_to_user error\n");
		return -EFAULT;
	}

	return 0;
}

static long handle_set_lock_state_ioc(struct file *filp, unsigned int cmd,
				unsigned long param)
{
#ifdef CONFIG_BCM_SET_CP_LOCK_STATE_SUPPORT
/* To support customer implement SIMLOCK on user space*/
	sec_simlock_state_t ioc_param = { 0 };
	SYS_SIMLOCK_STATE_t sys_lock_status = { 0 };

	if (copy_from_user
		(&ioc_param, (sec_simlock_state_t *) param,
		 sizeof(sec_simlock_state_t)) != 0) {
		pr_err("handle_set_lock_state_ioc - copy_from_user() had error\n");
		return -EFAULT;
	}

	/* setting lock status*/

	sys_lock_status.network_lock_enabled =
				(UInt8)ioc_param.network_lock_enabled;
	sys_lock_status.network_subset_lock_enabled =
				(UInt8)ioc_param.network_subset_lock_enabled;
	sys_lock_status.service_provider_lock_enabled =
				(UInt8)ioc_param.service_provider_lock_enabled;
	sys_lock_status.corporate_lock_enabled =
				(UInt8)ioc_param.corporate_lock_enabled;
	sys_lock_status.phone_lock_enabled =
				(UInt8)ioc_param.phone_lock_enabled;
	sys_lock_status.network_lock =
				ioc_param.network_lock;
	sys_lock_status.network_subset_lock =
				ioc_param.network_subset_lock;
	sys_lock_status.service_provider_lock =
				ioc_param.service_provider_lock;
	sys_lock_status.corporate_lock = ioc_param.corporate_lock;
	sys_lock_status.phone_lock = ioc_param.phone_lock;

	SIMLOCKApi_SetStatusEx(ioc_param.sim_id, &sys_lock_status);

	return 0;
#else
	pr_err("SET_LOCK_STATE_IOC is not allowed.\n");
	return -EFAULT;
#endif
}

static int handle_sec_mem_req(struct file *filp, unsigned int cmd,
						 unsigned long param)
{
	int ret = 0;
	unsigned long kparam;
	SEC_PrivData_t *priv = (SEC_PrivData_t *)filp->private_data;

	BUG_ON(!priv);

	mutex_lock(&priv->lock);

	switch (cmd) {
	case SEC_MEM_ALLOC:
		/*
		 * check if allocation is already done for this file descriptor
		 */
		if (priv->dma_addr || priv->handle || priv->size) {
			pr_err("handle_sec_mem_Req - Allocation Exists\n");
			ret = -EEXIST;
			goto out_unlock;
		}

		if (copy_from_user(&kparam,
			(unsigned long *)param, sizeof(unsigned long)) != 0) {
			pr_err("handle_sec_mem_Req - copy_from_user() had error\n");
			ret = -EFAULT;
			goto out_unlock;
		}

		/* check if the size is valid */
		if (!kparam) {
			pr_err("handle_sec_mem_Req - invalid parameter passed\n");
			ret = -EINVAL;
			goto out_unlock;
		}

		/* Make sure size is page aligned */
		kparam = PAGE_SIZE_UP(kparam);

		priv->dma_addr = dma_alloc_coherent(drvdata, kparam,
						&priv->handle, GFP_KERNEL);
		if (priv->dma_addr == NULL) {
			pr_err("handle_sec_mem_Req - dma allocation failed for size(%lukB)\n",
					kparam/SZ_1K);
			ret = -ENOMEM;
			goto out_unlock;
		}

		priv->size = kparam;
		break;
	default:
		ret = -ENOIOCTLCMD;
	}

out_unlock:
	mutex_unlock(&priv->lock);
	return ret;
}

/***************************************************************************/
/**
 *	Function Name: read_imei()
 *
 *	Description:  Get IMEI information
 *
 *	Return:	IMEI string
 **/

Boolean read_imei(UInt8 *imeiStr1, UInt8 *imeiStr2)
{
#ifdef CONFIG_BRCM_EMMC_RPMB_SUPPORT
	char imeiMacStr1[IMEI_MAC_DIGITS+1] = {0};
	char imeiMacStr2[IMEI_MAC_DIGITS+1] = {0};

	if ((imeiStr1 == NULL) || (imeiStr2 == NULL)) {
		pr_err("ReadIMEIHexData: imeiStr buffer is NULL\n");
		goto ERROR;
	}

	if (read_imei1(imeiStr1, imeiMacStr1, IMEI_DIGITS,
		IMEI_MAC_DIGITS) < 0) {
		pr_err("ReadIMEIHexData: read_imei 1 is fail\n");
	}

	if (read_imei2(imeiStr2, imeiMacStr2, IMEI_DIGITS,
		IMEI_MAC_DIGITS) < 0) {
		pr_err("ReadIMEIHexData: read_imei 2 is fail\n");
	}

	pr_info("%s: IMEI1:%s IMEI 2:%s !!!", __func__, imeiStr1, imeiStr2);

	return TRUE;

ERROR:
	return FALSE;

#else
	pr_info("handle_get_imei_ioc: EMMC RPMB is not support\n");
	return FALSE;
#endif
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
			   int is_test_sim, sec_simlock_state_t *sim_lock_state)
{
	int result = 0;
	pr_err("%s:SIM%d; is_test_sim = %d\n", __func__, simID, is_test_sim);
	if (!sim_data || !sim_lock_state) {
		pr_err("%s: invalid sim_data or sim_lock_state ptrs, exit\n",
								__func__);
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
		sim_lock_state->network_subset_lock = SEC_SIMLOCK_SECURITY_OPEN;
		sim_lock_state->service_provider_lock =
					SEC_SIMLOCK_SECURITY_OPEN;
		sim_lock_state->corporate_lock = SEC_SIMLOCK_SECURITY_OPEN;
		sim_lock_state->phone_lock = SEC_SIMLOCK_SECURITY_OPEN;

#ifdef CONFIG_BRCM_SIM_SECURE_ENABLE
		pr_info("%s: SIM SECURE enabled\n", __func__);
		/* update lock state */
		SIMLockCheckAllLocks(simID,
			sim_data->imsi_string,
			(0 != sim_data->gid1_len ? sim_data->gid1 : NULL),
			(0 != sim_data->gid2_len ? sim_data->gid2 : NULL));
		SIMLockUpdateSIMLockState(simID);

		/* retrieve current SIM lock state for this SIM */
		SIMLockGetSIMLockState(simID, sim_lock_state);
#else
		pr_info("%s: SIM SECURE not enabled\n", __func__);
#endif
		pr_info("%s enabled: %d, %d, %d, %d, %d\n",
		  __func__,
		  sim_lock_state->network_lock_enabled,
		  sim_lock_state->network_subset_lock_enabled,
		  sim_lock_state->service_provider_lock_enabled,
		  sim_lock_state->corporate_lock_enabled,
		  sim_lock_state->phone_lock_enabled);

		pr_info("%s status: %d, %d, %d, %d, %d\n",
		  __func__,
		  sim_lock_state->network_lock,
		  sim_lock_state->network_subset_lock,
		  sim_lock_state->service_provider_lock,
		  sim_lock_state->corporate_lock,
		  sim_lock_state->phone_lock);
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
		class_destroy(sModule.mDriverClass);
		return PTR_ERR(drvdata);
	}

	/* Need to set this if we are going to dma allocations */
	drvdata->coherent_dma_mask = ((u64)~0);

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
