/*****************************************************************************
*  Copyright 2003 - 2007 Broadcom Corporation.  All rights reserved.
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2, available at
*  http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
*****************************************************************************/

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/fcntl.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/wakelock.h>
#include <linux/semaphore.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/jiffies.h>
#ifdef CONFIG_BRCM_FUSE_IPC_CIB
#include <linux/broadcom/ipcinterface.h>
#else
#include <linux/broadcom/IPCInterface.h>
#endif
#include <linux/broadcom/ipc_server_ifc.h>
#include <linux/broadcom/ipc_server_ioctl.h>
#include <plat/kona_reset_reason.h>

#include "lnx_support.h"
#include "ipc_server_ccb.h"
#include "ipc_debug.h"
#include "bcmlog.h"
#include "ipc_server.h"
#include "ipcinterface.h"

#include <mach/comms/platform_mconfig.h>
#include <mach/memory.h>

#include <mach/io_map.h>
/* for BINTC register offsets */
#include <mach/rdb/brcm_rdb_bintc.h>




#include <mach/irqs.h>

/* definitions for Rhea/BI BModem IRQ's
 * extracted from chip_irq.h for Rhea */
#define NUM_KONAIRQs          224
#define LAST_KONAIRQ          (NUM_KONAIRQs-1)
#define FIRST_BMIRQ           (LAST_KONAIRQ+1)
#define NUM_BMIRQs            56
#define IRQ_TO_BMIRQ(irq)         ((irq)-FIRST_BMIRQ)

#define IPC_MAJOR (204)
#define BCM_KERNEL_IPC_NAME "bcm_fuse_ipc"

struct ipcs_info_t {
	int ipc_state;
	struct tasklet_struct intr_tasklet;
	struct work_struct cp_crash_dump_wq;
	struct workqueue_struct *crash_dump_workqueue;
	void __iomem *apcp_shmem;
	struct class *mDriverClass;	/* driver class */
	struct device *drvdata;
};

static struct ipcs_info_t g_ipc_info = { 0 };
static Boolean cp_running = 0;	//FALSE;
static RAW_NOTIFIER_HEAD(cp_state_notifier_list);
static DEFINE_SPINLOCK(cp_state_notifier_lock);

extern void ipc_set_interrupt_mask(void);
struct wake_lock ipc_wake_lock;
static IPC_PlatformSpecificPowerSavingInfo_T ipc_ps;

extern int IpcCPCrashCheck(void);
extern void ProcessCPCrashedDump(struct work_struct *work);

#if defined(CONFIG_BCM215X_PM) && defined(CONFIG_ARCH_BCM2153)
extern void pm_ipc_power_saving_init(
	IPC_PlatformSpecificPowerSavingInfo_T *ipc_ps);
#endif


struct IPC_Evt_t {
	wait_queue_head_t evt_wait;
	int evt;
};

static void *EventCreate(void)
{
	struct IPC_Evt_t *ipcEvt;

	ipcEvt = kmalloc(sizeof(struct IPC_Evt_t), GFP_ATOMIC);
	if (!ipcEvt)
	{
		IPC_DEBUG(DBG_ERROR, "IPC event create fail\n");
		return NULL;
	}

	init_waitqueue_head(&(ipcEvt->evt_wait));
	ipcEvt->evt = 0;

	return ipcEvt;
}

static IPC_ReturnCode_T EventSet(void *Event)
{
	struct IPC_Evt_t *ipcEvt = (struct IPC_Evt_t *)Event;

	/* **FIXME** need to protect access to this? */
	ipcEvt->evt = 1;
	wake_up_interruptible(&(ipcEvt->evt_wait));
	return IPC_OK;
}

static IPC_ReturnCode_T EventClear(void *Event)
{
	struct IPC_Evt_t *ipcEvt = (struct IPC_Evt_t *)Event;

	/* **FIXME** need to protect access to this? */
	ipcEvt->evt = 0;
	/* wakeup(&(ipcEvt->evt_wait)); */
	return IPC_OK;
}

static IPC_ReturnCode_T EventWait(void *Event, IPC_U32 MilliSeconds)
{
	struct IPC_Evt_t *ipcEvt = (struct IPC_Evt_t *)Event;
	int ret;

	if (MilliSeconds == IPC_WAIT_FOREVER)
	{
		ret = wait_event_interruptible((ipcEvt->evt_wait), (ipcEvt->evt == 1));
		return (ret == 0) ? IPC_OK : IPC_ERROR;
	}
	else
	{
		ret = wait_event_interruptible_timeout((ipcEvt->evt_wait), (ipcEvt->evt == 1),
				     msecs_to_jiffies(MilliSeconds));		
		return (ret == 0) ? IPC_TIMEOUT : ((ret == -ERESTARTSYS) ? IPC_ERROR : IPC_OK);
	}
}

static IPC_ReturnCode_T EventDelete(void *Event)
{
	struct IPC_Evt_t *ipcEvt = (struct IPC_Evt_t *)Event;
	if (ipcEvt) {
		IPC_DEBUG(DBG_TRACE, "EventDelete: %p\n", ipcEvt);
		kfree(ipcEvt);
	}
	return IPC_OK;
}

/**
   @fn Boolean is_CP_running(void);
*/
Boolean is_CP_running(void)
{
	return cp_running;
}

/**
   @fn void ipcs_ipc_initialised(void);
*/
void ipcs_ipc_initialised(void)
{
	IPC_DEBUG(DBG_TRACE, "IPC Initialization completed\n");
	g_ipc_info.ipc_state = 1;

	return;
}

/**
   @fn void ipcs_get_ipc_state(int *state);
 */
void ipcs_get_ipc_state(int *state)
{
	if (state != NULL)
		*state = g_ipc_info.ipc_state;

	return;
}

/**
   @fn void ipcs_ipc_reset(void);
*/
void ipcs_ipc_reset(void)
{
	IPC_DEBUG(DBG_TRACE, "reset ...\n");

	return;
}

int cp_crashed;

void ipcs_intr_tasklet_handler(unsigned long data)
{
	cp_crashed = 0;

	if (IpcCPCrashCheck()) {
		cp_crashed = 1;

		spin_lock_bh(&cp_state_notifier_lock);
		raw_notifier_call_chain(&cp_state_notifier_list, IPC_CPSTATE_CRASHED, NULL);
		spin_unlock_bh(&cp_state_notifier_lock);
		/* schedule the work on the decidated CP crash dump work queue */
		queue_work(g_ipc_info.crash_dump_workqueue,
			   &g_ipc_info.cp_crash_dump_wq);
		IPC_ProcessEvents();
	} else {
		IPC_ProcessEvents();
		wake_unlock(&ipc_wake_lock);
	}
}


static irqreturn_t ipcs_interrupt(int irq, void *dev_id)
{
	void __iomem *base = (void __iomem *)(KONA_BINTC_BASE_ADDR);
	int birq = IRQ_TO_BMIRQ(IRQ_IPC_C2A_BINTC);	//55;

	/* Clear the interrupt */
	if (birq >= 32)
		writel(1 << (birq - 32),
		       base + BINTC_ISWIR1_CLR_OFFSET /*0x34 */ );
	else
		writel(1 << (birq), base + BINTC_ISWIR0_CLR_OFFSET /*0x24 */ );

	IPC_UpdateIrqStats();

	wake_lock(&ipc_wake_lock);
	tasklet_schedule(&g_ipc_info.intr_tasklet);

	return IRQ_HANDLED;
}

/**
   @fn int ipc_ipc_init(void *smbase, unsigned int size);
*/
int ipc_ipc_init(void *smbase, unsigned int size)
{
	IPC_ControlInfo_T ipc_control;

	memset(&ipc_control, 0, sizeof(IPC_ControlInfo_T));

	ipc_control.RaiseEventFptr = bcm_raise_cp_int;
	ipc_control.LockFunctions.CreateLock = bcm_create_lock;
	ipc_control.LockFunctions.AcquireLock = bcm_aquire_lock;
	ipc_control.LockFunctions.ReleaseLock = bcm_release_lock;
	ipc_control.LockFunctions.DeleteLock = bcm_delete_lock;
	ipc_control.PhyToOSAddrFPtr = bcm_map_phys_to_virt;
	ipc_control.OSToPhyAddrFPtr = bcm_map_virt_to_phys;
	ipc_control.EventFunctions.Create = EventCreate;
	ipc_control.EventFunctions.Set = EventSet;
	ipc_control.EventFunctions.Clear = EventClear;
	ipc_control.EventFunctions.Wait = EventWait;
	ipc_control.EventFunctions.Delete = EventDelete;
	ipc_control.PowerSavingStruct = &ipc_ps;

#if defined(CONFIG_BCM215X_PM) && defined(CONFIG_ARCH_BCM2153)
	pm_ipc_power_saving_init(&ipc_ps);
#endif

	IPC_Initialise(smbase, size, IPC_AP_CPU, &ipc_control,
		       ipcs_ipc_initialised, ipcs_ipc_reset, NULL);

	return 0;
}

void WaitForCpIpc(void *pSmBase)
{
	int k = 0, ret = 0;
	void __iomem *cp_boot_base;
	u32 reg_val;

	cp_running = 0;


	IPC_DEBUG(DBG_WARN, "Waiting for CP IPC to init 0x%x\n",
		  (unsigned int)pSmBase);

	/* Debug info to show is_ap_only_boot() status */
	if (is_ap_only_boot())
		IPC_DEBUG(DBG_WARN, "AP ONLY BOOT\n");
	else
		IPC_DEBUG(DBG_WARN, "NORMAL BOOT\n");

	if (!is_ap_only_boot()) { /* Check for AP_BOOT or NORMAL_BOOT */
	    ret = IPC_IsCpIpcInit(pSmBase, IPC_AP_CPU);
		IPC_DEBUG(DBG_WARN, "back from IPC_IsCpIpcInit\n");
	    while (ret == 0) {
		/* Wait up to 2s for CP to init */
		if (k++ > 200)
			break;
		else
			msleep(10);
		ret = IPC_IsCpIpcInit(pSmBase, IPC_AP_CPU);
		}
	}

	if (ret == 1) {
		IPC_DEBUG(DBG_WARN, "CP IPC initialized\n");
		spin_lock_bh(&cp_state_notifier_lock);
		cp_running = 1;	/* TRUE; */


			


		raw_notifier_call_chain(&cp_state_notifier_list, IPC_CPSTATE_RUNNING, NULL);
		spin_unlock_bh(&cp_state_notifier_lock);
	} else if (ret == 0) {
		IPC_DEBUG(DBG_ERROR,
			  "********************************************************************\n");
		IPC_DEBUG(DBG_ERROR,
			  "*                                                                  *\n");
		IPC_DEBUG(DBG_ERROR,
			  "*       CP IPC NOT INITIALIZED - SYSTEM BOOTS WITH AP ONLY!!!      *\n");
		IPC_DEBUG(DBG_ERROR,
			  "*                                                                  *\n");
		IPC_DEBUG(DBG_ERROR,
			  "********************************************************************\n");
		cp_boot_base = ioremap_nocache(MODEM_ITCM_ADDRESS, 0x20);
		if (!cp_boot_base) {
			IPC_DEBUG(DBG_ERROR,
				  "ITCM Addr=0x%x, length=0x%x",
				  MODEM_ITCM_ADDRESS, 0x20);
			IPC_DEBUG(DBG_ERROR, "ioremap cp_boot_base error\n");
			return;
		}
		reg_val = readl(cp_boot_base);
		IPC_DEBUG(DBG_ERROR, "reset vector value is 0x%x\n", reg_val);

		reg_val = readl(cp_boot_base + 0x20);
		IPC_DEBUG(DBG_ERROR, "CP Boot flag 0x%x\n", reg_val);

		iounmap(cp_boot_base);

		/* SKIP reset is_ap_only_boot() non zero */
		if (!is_ap_only_boot())
			BUG_ON(ret == 0);
	} else if (ret == -1) {
		IPC_DEBUG(DBG_ERROR,
			  "********************************************************************\n");
		IPC_DEBUG(DBG_ERROR,
			  "*                                                                  *\n");
		IPC_DEBUG(DBG_ERROR,
			  "*                             CP CRASHED !!!                       *\n");
		IPC_DEBUG(DBG_ERROR,
			  "*                                                                  *\n");
		IPC_DEBUG(DBG_ERROR,
			  "********************************************************************\n");
		BUG_ON(ret);
	} else if (ret == -2) {
		IPC_DEBUG(DBG_ERROR,
			  "********************************************************************\n");
		IPC_DEBUG(DBG_ERROR,
			  "*                                                                  *\n");
		IPC_DEBUG(DBG_ERROR,
			  "*                     AP/CP IPC VERSION NOT MATCH !!!               *\n");
		IPC_DEBUG(DBG_ERROR,
			  "*                                                                  *\n");
		IPC_DEBUG(DBG_ERROR,
			  "********************************************************************\n");
		/* BUG_ON(ret); */
	}
}

void ipcs_cp_notifier_register(struct notifier_block *nb)
{
	/* lock serializes against cp_running value changing */
	spin_lock_bh(&cp_state_notifier_lock);
	if(nb != NULL)
		raw_notifier_chain_register(&cp_state_notifier_list, nb);
	if (cp_running && nb != NULL)
		nb->notifier_call(nb, IPC_CPSTATE_RUNNING, NULL);
	spin_unlock_bh(&cp_state_notifier_lock);
}

void ipcs_cp_notifier_unregister(struct notifier_block *nb)
{
	spin_lock_bh(&cp_state_notifier_lock);
	raw_notifier_chain_unregister(&cp_state_notifier_list, nb);
	spin_unlock_bh(&cp_state_notifier_lock);
}

/**
   static int ipcs_init(void *smbase, unsigned int size)
 */
static int ipcs_init(void *smbase, unsigned int size, int isReset)
{
	int rc = 0;

	IPC_DEBUG(DBG_TRACE, "WaitForCpIpc\n");

	/* Wait for CP to initialize */
	WaitForCpIpc(smbase);
	IPC_DEBUG(DBG_TRACE, "WaitForCpIpc done\n");

	IPC_DEBUG(DBG_TRACE, "Calling ipc_set_interrupt_mask()\n");
	ipc_set_interrupt_mask();
	IPC_DEBUG(DBG_TRACE, "Done ipc_set_interrupt_mask()\n");


	/* Initialize OS specific callbacks with the IPC lib */
	rc = ipc_ipc_init(smbase, size);
	if (rc) {
		IPC_DEBUG(DBG_ERROR, "ipc_ipc_init() failed, ret[%d]\n", rc);
		return rc;
	}
	IPC_DEBUG(DBG_TRACE, "ipc_ipc_init done\n");

	/* Register Endpoints */
	rc = ipcs_ccb_init(isReset);
	if (rc) {
		IPC_DEBUG(DBG_ERROR, "ipcs_ccb_init() failed, ret[%d]\n", rc);
		return rc;
	}

	IPC_DEBUG(DBG_TRACE, "ipcs_ccb_init done\n");
	/* Let CP know that we are done registering endpoints */
	IPC_Configured();

	/* Wait for IPC initialized */
	IPC_DEBUG(DBG_TRACE, "IPC_Configured() invoked\n");

	return 0;
}

int ipcs_reinitialize_ipc(void)
{
	IPC_DEBUG(DBG_INFO, "calling ipcs_init\n");
	return ipcs_init((void *)g_ipc_info.apcp_shmem, IPC_SIZE, 1);
}

static int CP_Boot(void)
{
	int started = 0;
	void __iomem *cp_boot_itcm;
	void __iomem *cp_bmodem_r4cfg;
	unsigned int r4init;
	unsigned int jump_instruction = 0xEA000000;

#define BMODEM_SYSCFG_R4_CFG0  0x3a004000
#define CP_SYSCFG_BASE_SIZE    0x8
#define CP_ITCM_BASE_SIZE      0x1000     /* 1 4k page */

	IPC_DEBUG(DBG_TRACE, "enter\n");
	cp_bmodem_r4cfg = ioremap(BMODEM_SYSCFG_R4_CFG0, CP_SYSCFG_BASE_SIZE);
	if (!cp_bmodem_r4cfg) {
		IPC_DEBUG(DBG_ERROR,
		"BMODEM_SYSCFG_R4_CFG0=0x%x, CP_SYSCFG_BASE_SIZE=0x%x\n",
			BMODEM_SYSCFG_R4_CFG0, CP_SYSCFG_BASE_SIZE);
		IPC_DEBUG(DBG_ERROR, "ioremap cp_bmodem_r4cfg failed\n");
		return started;
	}

	r4init = *(unsigned int *)(cp_bmodem_r4cfg);

	/* check if the CP is already booted, and if not, then boot it */
	if ((0x5 != (r4init & 0x5))) {
		IPC_DEBUG(DBG_TRACE, "boot (R4 COMMS) - init code 0x%x ...\n",
			  r4init);

		/* Set the CP jump to address.  CP must jump to DTCM offset
		 * 0x400
		 */
		cp_boot_itcm = ioremap(MODEM_ITCM_ADDRESS, CP_ITCM_BASE_SIZE);
		if (!cp_boot_itcm) {
			IPC_DEBUG(DBG_ERROR,
			"MODEM_ITCM_ADDRESS=0x%x, CP_ITCM_BASE_SIZE=0x%x\n",
			MODEM_ITCM_ADDRESS, CP_ITCM_BASE_SIZE);
			IPC_DEBUG(DBG_ERROR, "ioremap cp_boot_itcm failed\n");
			return 0;
		}
		/* generate instruction for reset vector that jumps to start of
		 * cp_boot.img at 0x20400
		 */
		jump_instruction |=
		    (0x00FFFFFFUL & (((0x20000 + RESERVED_HEADER)
				      / 4) - 2));
		IPC_DEBUG(DBG_TRACE,
			  "cp_boot_itcm 0x%x jump_instruction 0x%x\n",
			  (unsigned int)cp_boot_itcm, jump_instruction);
		/* write jump instruction to cp reset vector */
		*(unsigned int *)(cp_boot_itcm) = jump_instruction;

		iounmap(cp_boot_itcm);

		/* start CP - should jump to 0x20400 and spin there */
		*(unsigned int *)(cp_bmodem_r4cfg) = 0x5;
	} else {
		IPC_DEBUG(DBG_TRACE,
			"(R4 COMMS) already started - init code 0x%x ...\n",
			r4init);
	}

	iounmap(cp_bmodem_r4cfg);
	IPC_DEBUG(DBG_TRACE, "exit\n");

	return started;
}


int cpStart(int isReset)
{
	void __iomem *apcp_shmem;
	void __iomem *cp_boot_base;
	u32 reg_val;

	IPC_DEBUG(DBG_TRACE, "enter\n");

#ifdef CONFIG_BCM_MODEM_DEFER_CP_START
	CP_Boot();
#else
	if (isReset) {
		IPC_DEBUG(DBG_INFO, "call CP_Boot\n");
		CP_Boot();
	}
#endif

	apcp_shmem = ioremap_nocache(IPC_BASE, IPC_SIZE);
	if (!apcp_shmem) {
		IPC_DEBUG(DBG_ERROR, "IPC_BASE=0x%x, IPC_SIZE=0x%x\n",
			  IPC_BASE, IPC_SIZE);
		IPC_DEBUG(DBG_ERROR, "ioremap shmem failed\n");
		return -1;
	}
	/* clear first (9) 32-bit words in shared memory */
	memset(apcp_shmem, 0, IPC_SIZE);
	iounmap(apcp_shmem);
	IPC_DEBUG(DBG_TRACE, "cleared sh mem\n");

	cp_boot_base = ioremap_nocache(MODEM_DTCM_ADDRESS,
				       INIT_ADDRESS_OFFSET + RESERVED_HEADER);

	if (!cp_boot_base) {
		IPC_DEBUG(DBG_ERROR,
			  "DTCM Addr=0x%x, length=0x%x",
			  MODEM_DTCM_ADDRESS,
			  INIT_ADDRESS_OFFSET + RESERVED_HEADER);
		IPC_DEBUG(DBG_ERROR, "ioremap cp_boot_base error\n");
		return -1;
	}

	/* Start the CP:
	 *	- read main address
	 *	- write to init address
	 */
	reg_val = readl(cp_boot_base+MAIN_ADDRESS_OFFSET+RESERVED_HEADER);
	IPC_DEBUG(DBG_TRACE, "main addr 0x%x\n", reg_val);
	writel(reg_val, cp_boot_base+INIT_ADDRESS_OFFSET+RESERVED_HEADER);

	iounmap(cp_boot_base);
	IPC_DEBUG(DBG_TRACE, "modem (R4 COMMS) started ...\n");
	return 0;
}


#ifndef CONFIG_MACH_BCM_FPGA
static int __init Comms_Start(void)
{
	return cpStart(0);   /* Normal Comms_Start */
}
/* The FPGA block does not have the comms module and will hang
	 * on attempt to access it
	 */
arch_initcall(Comms_Start);
#endif

struct device *ipcs_get_drvdata(void)
{
	return g_ipc_info.drvdata;
}
static int ipcs_read_proc(struct file *file, char __user *buf, size_t count,
				loff_t *pos)
{
	int len = IPC_DumpStatus(buf, count);
	if (len > count)
		len = count;
	if (len < 0)
		len = 0;
	return len;
}

static const struct file_operations ipc_proc_fops = {
	.read	=	ipcs_read_proc,
};

static int __init ipcs_module_init(void)
{
	int rc = -1;
	struct proc_dir_entry *dir;

	if (ipc_crashsupport_init())
		goto out;

	dir = proc_create_data("driver/bcmipc", 0, NULL, &ipc_proc_fops,
			NULL);
	if (dir == NULL) {
		IPC_DEBUG(DBG_ERROR,
			  "ipcs_module_init: can't create /proc/driver/bcmipc\n");
		//return -1;
	}

	IPC_DEBUG(DBG_TRACE, "start ...\n");

	g_ipc_info.ipc_state = 0;

	g_ipc_info.mDriverClass = class_create(THIS_MODULE,
				BCM_KERNEL_IPC_NAME);
	if (IS_ERR(g_ipc_info.mDriverClass)) {
		IPC_DEBUG(DBG_ERROR, "driver class_create failed\n");
		goto out;
	}

	g_ipc_info.drvdata = device_create(g_ipc_info.mDriverClass, NULL,
				MKDEV(IPC_MAJOR, 0), NULL, BCM_KERNEL_IPC_NAME);
	if (IS_ERR(g_ipc_info.drvdata)) {
		IPC_DEBUG(DBG_ERROR, "device_create drvdata failed\n");
		goto out;
	}

	IPC_DEBUG(DBG_TRACE, "Allocate CP crash dump workqueue\n");
	g_ipc_info.crash_dump_workqueue = alloc_workqueue("dump-wq",
							  WQ_FREEZABLE |
							  WQ_MEM_RECLAIM, 0);

	if (!g_ipc_info.crash_dump_workqueue) {
		IPC_DEBUG(DBG_ERROR,
			  "Cannot allocate CP crash dump workqueue\n");
		goto out;
	}

	INIT_WORK(&g_ipc_info.cp_crash_dump_wq, ProcessCPCrashedDump);

	tasklet_init(&g_ipc_info.intr_tasklet, ipcs_intr_tasklet_handler, 0);

	/**
	 Make sure this is not cache'd because CP has to know about any changes
	 we write to this memory immediately.
	*/
	IPC_DEBUG(DBG_TRACE, "ioremap_nocache IPC_BASE\n");
	g_ipc_info.apcp_shmem = ioremap_nocache(IPC_BASE, IPC_SIZE);
	if (!g_ipc_info.apcp_shmem) {
		rc = -ENOMEM;
		IPC_DEBUG(DBG_ERROR, "Could not map shmem\n");
		goto out_shared_mem_fail;
	}

	IPC_DEBUG(DBG_TRACE, "ipcs_init\n");
	if (ipcs_init((void *)g_ipc_info.apcp_shmem, IPC_SIZE, 0)) {
		rc = -1;
		IPC_DEBUG(DBG_ERROR, "ipcs_init() failed\n");
		goto out_ipc_init_fail;
	}

	IPC_DEBUG(DBG_TRACE, "ok\n");

	wake_lock_init(&ipc_wake_lock, WAKE_LOCK_SUSPEND, "ipc_wake_lock");

	IPC_DEBUG(DBG_TRACE, "request_irq\n");
	rc = request_irq(IRQ_IPC_C2A, ipcs_interrupt, IRQF_NO_SUSPEND,
			 "ipc-intr", &g_ipc_info);

	if (rc) {
		IPC_DEBUG(DBG_ERROR, "request_irq error\n");
		goto out_irq_req_fail;
	}

	IPC_DEBUG(DBG_TRACE, "IRQ Clear and Enable\n");

	return 0;

out_irq_req_fail:

	wake_lock_destroy(&ipc_wake_lock);
out_ipc_init_fail:
	iounmap(g_ipc_info.apcp_shmem);

out_shared_mem_fail:
	flush_workqueue(g_ipc_info.crash_dump_workqueue);
	destroy_workqueue(g_ipc_info.crash_dump_workqueue);

out:
	IPC_DEBUG(DBG_ERROR, "IPC Driver Failed to initialise!\n");
	return rc;
}

static void __exit ipcs_module_exit(void)
{
	tasklet_kill(&g_ipc_info.intr_tasklet);
	flush_workqueue(g_ipc_info.crash_dump_workqueue);
	destroy_workqueue(g_ipc_info.crash_dump_workqueue);

	iounmap(g_ipc_info.apcp_shmem);

	free_irq(IRQ_IPC_C2A, &g_ipc_info);

	wake_lock_destroy(&ipc_wake_lock);
	device_destroy(g_ipc_info.mDriverClass, MKDEV(IPC_MAJOR, 0));
	class_destroy(g_ipc_info.mDriverClass);

	return;
}






late_initcall(ipcs_module_init);
module_exit(ipcs_module_exit);
