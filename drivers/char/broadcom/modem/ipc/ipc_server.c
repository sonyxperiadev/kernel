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
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/fcntl.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/slab.h>
#ifdef CONFIG_HAS_WAKELOCK
#include <linux/wakelock.h>
#endif
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

#include "lnx_support.h"
#include "ipc_server_ccb.h"
#include "ipc_debug.h"
#include "bcmlog.h"
#include "ipc_server.h"

#include <mach/comms/platform_mconfig.h>

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

struct ipcs_info_t {
	int ipc_state;
	struct tasklet_struct intr_tasklet;
	struct work_struct cp_crash_dump_wq;
	struct workqueue_struct *crash_dump_workqueue;
	void __iomem *apcp_shmem;
};

static struct ipcs_info_t g_ipc_info = { 0 };
static Boolean cp_running = 0;	//FALSE;

#ifdef CONFIG_HAS_WAKELOCK
struct wake_lock ipc_wake_lock;
#endif
static IPC_PlatformSpecificPowerSavingInfo_T ipc_ps;

struct IPC_Evt_t {
	wait_queue_head_t evt_wait;
	int evt;
};

static void *EventCreate(void)
{
	struct IPC_Evt_t *ipcEvt;

	ipcEvt = kmalloc(sizeof(struct IPC_Evt_t), GFP_KERNEL);
	if (!ipcEvt)
		return (void *)IPC_ERROR;

	init_waitqueue_head(&(ipcEvt->evt_wait));
	ipcEvt->evt = 0;

	return ipcEvt;
}

static IPC_ReturnCode_T EventSet(void *Event)
{
	struct IPC_Evt_t *ipcEvt = (struct IPC_Evt_t *)Event;

	/* **FIXME** need to protect access to this? */
	ipcEvt->evt = 1;
	wake_up(&(ipcEvt->evt_wait));
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

	if (MilliSeconds == IPC_WAIT_FOREVER)
		wait_event((ipcEvt->evt_wait), (ipcEvt->evt == 1));
	else if (!wait_event_timeout((ipcEvt->evt_wait), (ipcEvt->evt == 1),
				     msecs_to_jiffies(MilliSeconds)))
		return IPC_TIMEOUT;

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

		/* schedule the work on the decidated CP crash dump work queue */
		queue_work(g_ipc_info.crash_dump_workqueue,
			   &g_ipc_info.cp_crash_dump_wq);
		IPC_ProcessEvents();
	} else {
		IPC_ProcessEvents();
#ifdef CONFIG_HAS_WAKELOCK
		wake_unlock(&ipc_wake_lock);
#endif
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

#ifdef CONFIG_HAS_WAKELOCK
	wake_lock(&ipc_wake_lock);
#endif
	tasklet_schedule(&g_ipc_info.intr_tasklet);

	return IRQ_HANDLED;
}

/**
   @fn int ipc_ipc_init(void *smbase, unsigned int size);
*/
int __init ipc_ipc_init(void *smbase, unsigned int size)
{
	IPC_ControlInfo_T ipc_control;

	memset(&ipc_control, 0, sizeof(IPC_ControlInfo_T));

	ipc_control.RaiseEventFptr = bcm_raise_cp_int;
	ipc_control.EnableReEntrancyFPtr = bcm_enable_reentrancy;
	ipc_control.DisableReEntrancyFPtr = bcm_disable_reentrancy;
	ipc_control.PhyToOSAddrFPtr = bcm_map_phys_to_virt;
	ipc_control.OSToPhyAddrFPtr = bcm_map_virt_to_phys;
	ipc_control.EventFunctions.Create = EventCreate;
	ipc_control.EventFunctions.Set = EventSet;
	ipc_control.EventFunctions.Clear = EventClear;
	ipc_control.EventFunctions.Wait = EventWait;
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
	cp_running = 0;

	IPC_DEBUG(DBG_WARN, "Waiting for CP IPC to init ...\n");

	ret = IPC_IsCpIpcInit(pSmBase, IPC_AP_CPU);
	while (ret == 0) {
		/* Wait up to 2s for CP to init */
		if (k++ > 200)
			break;
		else
			msleep(10);
		ret = IPC_IsCpIpcInit(pSmBase, IPC_AP_CPU);
	}

	if (ret == 1) {
		IPC_DEBUG(DBG_WARN, "CP IPC initialized\n");
		cp_running = 1;	/* TRUE; */
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
		//BUG_ON(ret);
	}
}

/**
   static int ipcs_init(void *smbase, unsigned int size)
 */
static int __init ipcs_init(void *smbase, unsigned int size)
{
	int rc = 0;

	/* Wait for CP to initialize */
	WaitForCpIpc(smbase);

	/* Initialize OS specific callbacks with the IPC lib */
	rc = ipc_ipc_init(smbase, size);
	if (rc) {
		IPC_DEBUG(DBG_ERROR, "ipc_ipc_init() failed, ret[%d]\n", rc);
		return rc;
	}
	/* Register Endpoints  */
	rc = ipcs_ccb_init();
	if (rc) {
		IPC_DEBUG(DBG_ERROR, "ipcs_ccb_init() failed, ret[%d]\n", rc);
		return rc;
	}
	/* Let CP know that we are done registering endpoints */
	IPC_Configured();

	/* Wait for IPC initialized */
	IPC_DEBUG(DBG_TRACE, "IPC_Configured() invoked\n");

	return 0;
}

#ifdef CONFIG_BCM_MODEM_DEFER_CP_START
static int CP_Boot(void)
{
	int started = 0;
	void __iomem *cp_boot_itcm;
	void __iomem *cp_bmodem_r4cfg;
	unsigned int r4init;
	unsigned int jump_instruction = 0xEA000000;

#define BMODEM_SYSCFG_R4_CFG0  0x3a004000
#define CP_SYSCFG_BASE_SIZE    0x8
#define CP_ITCM_BASE_SIZE      0x1000	/* 1 4k page */

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

		/* Set the CP jump to address.
		   CP must jump to DTCM offset 0x400 */
		cp_boot_itcm = ioremap(MODEM_ITCM_ADDRESS, CP_ITCM_BASE_SIZE);
		if (!cp_boot_itcm) {
			IPC_DEBUG(DBG_ERROR,
				  "MODEM_ITCM_ADDRESS=0x%x, CP_ITCM_BASE_SIZE=0x%x\n",
				  MODEM_ITCM_ADDRESS, CP_ITCM_BASE_SIZE);
			IPC_DEBUG(DBG_ERROR, "ioremap cp_boot_itcm failed\n");
			return 0;
		}
		jump_instruction |=
		    (0x00FFFFFFUL & (((0x10000 + RESERVED_HEADER) / 4) - 2));
		*(unsigned int *)(cp_boot_itcm) = jump_instruction;

		iounmap(cp_boot_itcm);

		/* boot CP */
		*(unsigned int *)(cp_bmodem_r4cfg) = 0x5;
	} else {
		IPC_DEBUG(DBG_TRACE,
			  "(R4 COMMS) already started - init code 0x%x ...\n",
			  r4init);
	}

	iounmap(cp_bmodem_r4cfg);

	return started;
}
#endif

void Comms_Start(void)
{
	void __iomem *apcp_shmem;
	void __iomem *cp_boot_base;
	u32 reg_val;

#ifdef CONFIG_BCM_MODEM_DEFER_CP_START
	CP_Boot();
#endif

	apcp_shmem = ioremap_nocache(IPC_BASE, IPC_SIZE);
	if (!apcp_shmem) {
		IPC_DEBUG(DBG_ERROR, "IPC_BASE=0x%x, IPC_SIZE=0x%x\n",
			  IPC_BASE, IPC_SIZE);
		IPC_DEBUG(DBG_ERROR, "ioremap shmem failed\n");
		return;
	}
	/* clear first (9) 32-bit words in shared memory */
	memset(apcp_shmem, 0, IPC_SIZE);
	iounmap(apcp_shmem);

	cp_boot_base = ioremap_nocache(MODEM_DTCM_ADDRESS,
				       INIT_ADDRESS_OFFSET + RESERVED_HEADER);

	if (!cp_boot_base) {
		IPC_DEBUG(DBG_ERROR,
			  "DTCM Addr=0x%x, length=0x%x",
			  MODEM_DTCM_ADDRESS,
			  INIT_ADDRESS_OFFSET + RESERVED_HEADER);
		IPC_DEBUG(DBG_ERROR, "ioremap cp_boot_base error\n");
		return;
	}

	/* Start the CP */
	reg_val = readl(cp_boot_base + MAIN_ADDRESS_OFFSET + RESERVED_HEADER);
	writel(reg_val, cp_boot_base + INIT_ADDRESS_OFFSET + RESERVED_HEADER);

	iounmap(cp_boot_base);
	IPC_DEBUG(DBG_TRACE, "modem (R4 COMMS) started ...\n");
}

static int ipcs_read_proc(char *page, char **start, off_t off, int count,
			  int *eof, void *data)
{
	int len = IPC_DumpStatus(page);
	if (len <= off + count)
		*eof = 1;
	*start = page + off;
	len -= off;
	if (len > count)
		len = count;
	if (len < 0)
		len = 0;
	return len;
}

static int __init ipcs_module_init(void)
{
	int rc = -1;
	struct proc_dir_entry *dir;

	dir =
	    create_proc_read_entry("driver/bcmipc", 0, NULL, ipcs_read_proc,
				   NULL);
	if (dir == NULL) {
		IPC_DEBUG(DBG_ERROR,
			  "ipcs_module_init: can't create /proc/driver/bcmipc\n");
		//return -1;
	}

	IPC_DEBUG(DBG_TRACE, "start ...\n");

	Comms_Start();

	g_ipc_info.ipc_state = 0;

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
		goto out;
	}

	IPC_DEBUG(DBG_TRACE, "ipcs_init\n");
	if (ipcs_init((void *)g_ipc_info.apcp_shmem, IPC_SIZE)) {
		rc = -1;
		IPC_DEBUG(DBG_ERROR, "ipcs_init() failed\n");
		goto out;
	}

	IPC_DEBUG(DBG_TRACE, "ok\n");

#ifdef CONFIG_HAS_WAKELOCK
	wake_lock_init(&ipc_wake_lock, WAKE_LOCK_SUSPEND, "ipc_wake_lock");
#endif

	IPC_DEBUG(DBG_TRACE, "request_irq\n");
	rc = request_irq(IRQ_IPC_C2A, ipcs_interrupt, IRQF_NO_SUSPEND,
			 "ipc-intr", &g_ipc_info);

	if (rc) {
		IPC_DEBUG(DBG_ERROR, "request_irq error\n");
		goto out;
	}

	IPC_DEBUG(DBG_TRACE, "IRQ Clear and Enable\n");

	return 0;

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

#ifdef CONFIG_HAS_WAKELOCK
	wake_lock_destroy(&ipc_wake_lock);
#endif

	return;
}

late_initcall(ipcs_module_init);
