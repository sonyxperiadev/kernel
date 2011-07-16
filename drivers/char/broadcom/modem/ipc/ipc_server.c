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
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
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
#ifdef CONFIG_BRCM_FUSE_IPC_CIB
#include <linux/broadcom/ipcinterface.h>
#else
#include <linux/broadcom/IPCInterface.h>
#endif
#include <linux/broadcom/ipc_server_ifc.h>
#include <linux/broadcom/ipc_server_ioctl.h>
#include <linux/broadcom/bcm_fuse_memmap.h>

#include "lnx_support.h"
#include "ipc_server_ccb.h"
#include "ipc_debug.h"
#include "bcmlog.h"

#include <linux/broadcom/bcm_fuse_memmap.h>
#include <linux/broadcom/platform_mconfig.h>

#include <mach/io_map.h>
// for BINTC register offsets
#include <mach/rdb/brcm_rdb_bintc.h>
#include <mach/irqs.h>

#ifndef CONFIG_BCM_MODEM_HEADER_SIZE
#define CONFIG_BCM_MODEM_HEADER_SIZE 0
#endif
// definitions for Rhea/BI BModem IRQ's
// extracted from chip_irq.h for Rhea
#define NUM_KONAIRQs          224
#define LAST_KONAIRQ          (NUM_KONAIRQs-1)
#define FIRST_BMIRQ           (LAST_KONAIRQ+1)
#define NUM_BMIRQs            56
#define IRQ_TO_BMIRQ(irq)         ((irq)-FIRST_BMIRQ)

#define IPC_MAJOR (204)

typedef struct 
{
  dev_t devnum;
  int ipc_state;
  struct semaphore ipc_sem;
  struct cdev cdev;               
  struct work_struct cp_crash_dump_wq;
  struct work_struct intr_work;
  struct workqueue_struct *intr_workqueue;
  struct workqueue_struct *crash_dump_workqueue;
  void __iomem *apcp_shmem;
}ipcs_info_t;

// flag used to track occurence of early CP interrupt; 
// CP interrupt before IPC configured indicates early CP
// crash, so we'll do crash dump as soon as IPC is set up
static int sEarlyCPInterrupt = 0;

static ipcs_info_t g_ipc_info = { 0 };
#ifdef CONFIG_HAS_WAKELOCK
struct wake_lock ipc_wake_lock;
#endif
static IPC_PlatformSpecificPowerSavingInfo_T ipc_ps;

extern int IpcCPCrashCheck(void);
extern void ProcessCPCrashedDump(struct work_struct *work);

#if defined(CONFIG_BCM215X_PM) && defined(CONFIG_ARCH_BCM2153)
extern void pm_ipc_power_saving_init(IPC_PlatformSpecificPowerSavingInfo_T* ipc_ps);
#endif


typedef struct
{
    wait_queue_head_t evt_wait;
    int evt;
}IPC_Evt_t;

static void * EventCreate (void)
{
    IPC_Evt_t* ipcEvt;

    ipcEvt = kmalloc(sizeof(IPC_Evt_t), GFP_KERNEL);
    if (!ipcEvt)
        return (void*)IPC_ERROR;
    
    init_waitqueue_head(&(ipcEvt->evt_wait));
    ipcEvt->evt = 0;
    
	return ipcEvt;
}


static IPC_ReturnCode_T EventSet (void * Event)
{
    IPC_Evt_t* ipcEvt = (IPC_Evt_t*)Event;
    
    // **FIXME** need to protect access to this?
    ipcEvt->evt = 1;
    wake_up(&(ipcEvt->evt_wait));
    return IPC_OK;   
}

static IPC_ReturnCode_T EventClear (void * Event)
{

    IPC_Evt_t* ipcEvt = (IPC_Evt_t*)Event;
    
    // **FIXME** need to protect access to this?
    ipcEvt->evt = 0;
    //wakeup(&(ipcEvt->evt_wait));
    return IPC_OK;   
}

static IPC_ReturnCode_T EventWait (void * Event, IPC_U32 MilliSeconds)
{
    IPC_Evt_t* ipcEvt = (IPC_Evt_t*)Event;
	IPC_ReturnCode_T  rtnCode = IPC_OK;

	if(MilliSeconds == IPC_WAIT_FOREVER)
	{
		wait_event( (ipcEvt->evt_wait), (ipcEvt->evt == 1) );
	}
	else
	{
	    int timeout = 0;
	    // timeout in "jiffies" (apparently 10ms/jiffie in android?)
		timeout = wait_event_timeout( (ipcEvt->evt_wait), (ipcEvt->evt == 1), MilliSeconds/10 );
		// returns 0 if we timed out, > 0 otherwise
		if ( timeout == 0 )
		{
		    rtnCode = IPC_TIMEOUT;
		}
	}
	
	return rtnCode;
}

static int ipcs_open(struct inode *inode, struct file *file)
{
  ipcs_info_t *info;

  info = container_of(inode->i_cdev, ipcs_info_t, cdev);
  file->private_data = info;

  return 0;
}

static int ipcs_release(struct inode *inode, struct file *file)
{
  return(0);
}

ssize_t ipcs_read(struct file *filep, char __user *buf, size_t size, loff_t *off)
{
  ssize_t rc = 0;

  //rc = ipc_server_read(filep, buf, size, off);
      
  return(rc);
}

ssize_t ipcs_write(struct file *filep, const char __user *buf, size_t size, loff_t *off)
{
  return -EPERM;
}

static int ipcs_ioctl(struct inode *inode, struct file *filep, unsigned int cmd, unsigned long arg)
{
  int rc = 0;

   //rc = ipc_server_ioctl(cmd, arg);

  return(rc);
}


static struct file_operations ipc_ops = 
{
  .owner = THIS_MODULE,
  .open  = ipcs_open,
  .read  = ipcs_read,
  .write = ipcs_write,
  .ioctl = ipcs_ioctl,
  .mmap  = NULL,
  .release = ipcs_release,
};

/** 
   @fn void ipcs_ipc_initialised(void);
*/
void ipcs_ipc_initialised(void)
{
   printk( KERN_ALERT "IPC Initialization completed\n");
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
  printk( KERN_ALERT  ">> %s\n", __FUNCTION__);
  printk( KERN_ALERT  "<< %s\n", __FUNCTION__);

   return;
}

extern void abort(void);

void ipcs_intr_workqueue_process(struct work_struct *work)
{
    static int first = 1;
    if (first)
    {
        first = 0;
        set_user_nice(current, -16);
    }

   if(IpcCPCrashCheck())
   {
		switch(BCMLOG_GetCpCrashLogDevice()) {
#if 0 // MTD (flash/panic partition) not supported
		case BCMLOG_CPCRASH_MTD:
			/* we kill AP when CP crashes */
			printk( KERN_ALERT	"Crashing AP now...\n\n");
			abort();  
			break;
#endif // 0
		case BCMLOG_OUTDEV_RNDIS:
			//Using RNDIS causes work queue event/0 lock up so it needs its own thread
			g_ipc_info.crash_dump_workqueue = create_singlethread_workqueue("dump-wq");
			if (!g_ipc_info.crash_dump_workqueue)
			{
			  IPC_DEBUG(DBG_ERROR,"[ipc]: cannot create cp crash dump workqueue\n");
			}
			queue_work(g_ipc_info.crash_dump_workqueue, &g_ipc_info.cp_crash_dump_wq);
			break;
		default:
			schedule_work(&g_ipc_info.cp_crash_dump_wq);
		}

		IPC_ProcessEvents();
   }
   else
   {
       IPC_ProcessEvents();  
#ifdef CONFIG_HAS_WAKELOCK 
       wake_unlock(&ipc_wake_lock);
#endif
   }
}

static irqreturn_t ipcs_interrupt(int irq, void *dev_id)
{
   //printk( KERN_ALERT  "[ipc]: ipcs_interrupt %x %x \n", BINTC_ISWIR1_CLR_OFFSET, BINTC_ISWIR0_CLR_OFFSET);
   if((&g_ipc_info.intr_work)->func )
   {
#ifdef CONFIG_HAS_WAKELOCK
       wake_lock(&ipc_wake_lock);
#endif
       queue_work(g_ipc_info.intr_workqueue, &g_ipc_info.intr_work);
   }

   else
   {
        // if we're interrupted before IPC is setup, that 
        // means CP has had an early crash....
        printk( KERN_ALERT  "[ipc]: abnormal CP interrupt\n");
        sEarlyCPInterrupt = 1;
   }

   {
        // on Rhea/BI, we need to "manually" clear the CP->AP softint here
        void __iomem* base = (void __iomem *)( KONA_BINTC_BASE_ADDR);
        int birq = IRQ_TO_BMIRQ(IRQ_IPC_C2A_BINTC); //55;
        if ( birq >= 32 )
            writel(1 << (birq-32), base + BINTC_ISWIR1_CLR_OFFSET/*0x34*/);
        else
            writel(1 << (birq), base + BINTC_ISWIR0_CLR_OFFSET /*0x24*/);
    }

  return IRQ_HANDLED;
}


/**
   @fn int ipc_ipc_init(void *smbase, unsigned int size);
*/
int __init ipc_ipc_init(void *smbase, unsigned int size)
{
  IPC_ControlInfo_T ipc_control;

  memset(&ipc_control, 0, sizeof(IPC_ControlInfo_T));

  ipc_control.RaiseEventFptr  = bcm_raise_cp_int;
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

   return(0);
}

extern int IPC_IsCpIpcInit (void* pSmBase, IPC_CPU_ID_T Cpu);

void WaitForCpIpc (void* pSmBase)
{
	int k = 0, ret = 0;

    printk( KERN_ALERT  "ipcs_init Waiting for CP IPC to init ....\n");
	while ( (ret = IPC_IsCpIpcInit(pSmBase,IPC_AP_CPU )) == 0)
	{
//		for (i=0; i<2048; i++);	// do not fight for accessing shared memory
		set_current_state(TASK_INTERRUPTIBLE);
		schedule_timeout (100);
		k++;
	}
    printk( KERN_ALERT  "ipcs_init CP IPC initialized ret=%d\n", ret);
}



/**
   static int ipcs_init(void *smbase, unsigned int size)
 */
static int __init ipcs_init(void *smbase, unsigned int size)
{
  int rc = 0;

  //Wait for CP to initialize
  WaitForCpIpc(smbase);

   //Initialize OS specific callbacks with the IPC lib
  rc = ipc_ipc_init(smbase, size);
  if (rc) 
  {
    IPC_DEBUG(DBG_ERROR, "ipc_ipc_init() failed, ret[%d]\n", rc);
      return(rc);
  }

  //Register Endpoints 
  rc = ipcs_ccb_init();
  if (rc) 
  {
    IPC_DEBUG(DBG_ERROR, "ipcs_ccb_init() failed, ret[%d]\n", rc);
      return(rc);
  }

  //Let CP know that we are done registering endpoints
  IPC_Configured();

  //Wait for IPC initialized
  printk( KERN_ALERT  "IPC_Configured() invoked\n");

  return(0);
}

void Comms_Start(void)
{
    void __iomem *apcp_shmem;
    void __iomem *cp_boot_base;

    apcp_shmem = ioremap_nocache(IPC_BASE, IPC_SIZE);
    if (!apcp_shmem) {
        printk(KERN_ERR "%s: ioremap shmem failed\n", __func__);
        return;
    }
    /* clear first (9) 32-bit words in shared memory */
    memset(apcp_shmem, 0, IPC_SIZE);
    iounmap(apcp_shmem);

    cp_boot_base = ioremap(MODEM_DTCM_ADDRESS+CONFIG_BCM_MODEM_HEADER_SIZE, CP_BOOT_BASE_SIZE);
    if (!cp_boot_base) {
        printk(KERN_ERR "%s: ioremap error\n", __func__);
        return;
    }

    /* Start the CP, Code taken from Nucleus BSP */
    *(unsigned int *)(cp_boot_base+INIT_ADDRESS_OFFSET) = *(unsigned int *)(cp_boot_base+MAIN_ADDRESS_OFFSET);

    iounmap(cp_boot_base);
    printk(KERN_ALERT "%s: modem (R4 COMMS) started....\n", __func__);
}

static int __init ipcs_module_init(void)
{
  int rc;
  
  printk( KERN_ALERT "[ipc]: ipcs_module_init start..\n");
  
  Comms_Start();

  init_MUTEX_LOCKED(&g_ipc_info.ipc_sem);

  g_ipc_info.ipc_state = 0;

  g_ipc_info.devnum = MKDEV(IPC_MAJOR, 0);
  
  rc = register_chrdev_region(g_ipc_info.devnum, 1, "bcm_fuse_ipc");
  if (rc < 0) 
  {
    IPC_DEBUG(DBG_ERROR,"Error registering the IPC device\n");
    goto out;
  }

  cdev_init(&g_ipc_info.cdev, &ipc_ops);
  
  g_ipc_info.cdev.owner = THIS_MODULE;

  rc = cdev_add(&g_ipc_info.cdev, g_ipc_info.devnum, 1);
  if (rc) 
  {
    IPC_DEBUG(DBG_ERROR,"[ipc]: cdev_add errpr\n");
    goto out_unregister;
  }

  printk( KERN_ALERT  "[ipc]: create_workqueue\n");
 
  INIT_WORK(&g_ipc_info.cp_crash_dump_wq, ProcessCPCrashedDump);
  INIT_WORK(&g_ipc_info.intr_work, ipcs_intr_workqueue_process);

  g_ipc_info.intr_workqueue = create_singlethread_workqueue("ipc-wq");
  if (!g_ipc_info.intr_workqueue)
  {
    IPC_DEBUG(DBG_ERROR,"[ipc]: cannot create workqueue\n");
    goto out_unregister;
  } 


  printk( KERN_ALERT  "[ipc]: request_irq\n");
  rc = request_irq(IRQ_IPC_C2A, ipcs_interrupt, IRQF_NO_SUSPEND, "ipc-intr", &g_ipc_info);
  if (rc) 
  {
    IPC_DEBUG(DBG_ERROR,"[ipc]: request_irq error\n");
    goto out_del;
  }
  
  printk( KERN_ALERT  "[ipc]: IRQ Clear and Enable\n");
  
  /**
     Make sure this is not cache'd because CP has to know about any changes
     we write to this memory immediately.
   */
  printk( KERN_ALERT  "[ipc]: ioremap_nocache IPC_BASE\n");
  g_ipc_info.apcp_shmem = ioremap_nocache(IPC_BASE, IPC_SIZE);
  if (!g_ipc_info.apcp_shmem) 
  {
    rc = -ENOMEM;
    IPC_DEBUG(DBG_ERROR,"[ipc]: Could not map shmem\n");
    goto out_del;
  }
#ifdef CONFIG_HAS_WAKELOCK
  wake_lock_init(&ipc_wake_lock, WAKE_LOCK_SUSPEND, "ipc_wake_lock");
#endif

  printk( KERN_ALERT  "[ipc]: ipcs_init\n");
  if (ipcs_init((void *)g_ipc_info.apcp_shmem, IPC_SIZE))
  {
    rc = -1;
    IPC_DEBUG(DBG_ERROR,"[ipc]: ipcs_init() failed\n");
    goto out_del;
  }
  
  printk( KERN_ALERT "[ipc]: ipcs_module_init ok\n");
  
  if ( sEarlyCPInterrupt )
  {
    printk( KERN_ALERT "[ipc]: early CP interrupt - doing crash dump...\n");
#ifdef CONFIG_HAS_WAKELOCK
    wake_lock(&ipc_wake_lock);
#endif
    schedule_work(&g_ipc_info.cp_crash_dump_wq);
  }

  return 0;

out_del:
  cdev_del(&g_ipc_info.cdev);
out_unregister:
  unregister_chrdev_region(g_ipc_info.devnum, 1);
out:
  IPC_DEBUG(DBG_ERROR,"IPC Driver Failed to initialise!\n");
  return rc;
}

static void __exit ipcs_module_exit(void)
{
  flush_workqueue(g_ipc_info.intr_workqueue);
  destroy_workqueue(g_ipc_info.intr_workqueue);

  iounmap(g_ipc_info.apcp_shmem);

  free_irq(IRQ_IPC_C2A, &g_ipc_info);

  cdev_del(&g_ipc_info.cdev);

  unregister_chrdev_region(g_ipc_info.devnum, 1);
#ifdef CONFIG_HAS_WAKELOCK
  wake_lock_destroy(&ipc_wake_lock);
#endif

  return;
}

module_init(ipcs_module_init);
module_exit(ipcs_module_exit);
