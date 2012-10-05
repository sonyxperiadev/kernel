/*******************************************************************************
Copyright 2012 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement
governing use of this software, this software is licensed to you under the
terms of the GNU General Public License version 2, available at
http://www.gnu.org/copyleft/gpl.html (the "GPL").

Notwithstanding the above, under no circumstances may you combine this software
in any way with any other Broadcom software provided under a license other than
the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

#include <asm/io.h>
#include <linux/string.h> /* memset */
#include <linux/slab.h> /* kmalloc, kfree */
#include <linux/list.h>
#include <linux/completion.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/jiffies.h>
#include <linux/kthread.h>
#include <linux/err.h>

#include <mach/rdb/brcm_rdb_v3d.h>
#include <mach/rdb/brcm_rdb_pwrmgr.h>
#include <plat/scu.h>
#include <plat/clock.h>

#include "Session.h"
#include "Driver.h"
#include "Device.h"

#define IRQ_GRAPHICS BCM_INT_ID_RESERVED148
#define MEMORY_BIT V3D_INTCTL_INT_OUTOMEM_SHIFT
/*#define MEMORY_BIT V3D_INTCTL_INT_SPILLUSE_SHIFT*/
#define DUMMY_BYTES (8 << 10)
#define DELAY_SWITCHOFF_MS 100


/* ================================================================ */

static irqreturn_t InterruptHandler(int Irq, void *Context);


/* ================================================================ */

static inline uint32_t Read(const V3dDeviceType *Instance, unsigned int Offset)
{
	BUG_ON(Instance->On == 0);
	return ioread32(Instance->Register.Base + Offset);
}

static inline void Write(const V3dDeviceType *Instance, unsigned int Offset, uint32_t Value)
{
	BUG_ON(Instance->On == 0);
	iowrite32(Value, Instance->Register.Base + Offset);
}


/* ================================================================ */

static void SupplyBinningMemory(V3dDeviceType *Instance, V3dBinMemoryType *Memory)
{
#ifdef VERBOSE_DEBUG
	printk(KERN_ERR "%s: Supplying %08x (%1u/%1u)\n", __func__, Memory->Physical, Instance->OutOfMemory.Index.InUse, Instance->OutOfMemory.Index.Allocated);
#endif
	BUG_ON(Memory->Physical == 0);
	Write(Instance, V3D_BPOA_OFFSET, Memory->Physical);
	Write(Instance, V3D_BPOS_OFFSET, BIN_BLOCK_BYTES);

	/* The BPOS write doesn't take effect immediately */
	while ((Read(Instance, V3D_PCS_OFFSET) & V3D_PCS_BMOOM_MASK) != 0)
		;

	Write(Instance, V3D_INTCTL_OFFSET, 1 << MEMORY_BIT);
	Write(Instance, V3D_INTENA_OFFSET, 1 << MEMORY_BIT);
}

static void AllocateBinMemory(struct work_struct *Work)
{
	V3dDeviceType    *Instance = container_of(Work, V3dDeviceType, OutOfMemory.BinAllocation);
	V3dBinMemoryType *Memory;
	unsigned long     Flags;
	BUG_ON(Instance->OutOfMemory.Index.InUse != Instance->OutOfMemory.Index.Allocated);
	if (Instance->OutOfMemory.Index.Allocated == sizeof(Instance->OutOfMemory.Memory) / sizeof(Instance->OutOfMemory.Memory[0])) {
		printk(KERN_ERR "Out of overspill binning memory entries!\n");
		return;
	}

	/* We only change Allocated here */
	Memory = &Instance->OutOfMemory.Memory[Instance->OutOfMemory.Index.Allocated];
	Memory->Virtual = dma_alloc_coherent(Instance->Device, BIN_BLOCK_BYTES, &Memory->Physical, GFP_DMA);
	if (Memory->Virtual == NULL) {
		printk(KERN_ERR "Unable to allocate overspill binning memory!\n");
		/* TODO: Fail the job .. */
		return;
	}
	spin_lock_irqsave(&Instance->OutOfMemory.Lock, Flags);
	++Instance->OutOfMemory.Index.Allocated;
	++Instance->OutOfMemory.Index.InUse;
	spin_unlock_irqrestore(&Instance->OutOfMemory.Lock, Flags);
	SupplyBinningMemory(Instance, Memory);
}

static void ReleaseBinMemory(V3dDeviceType *Instance)
{
	Instance->OutOfMemory.Index.InUse = 0;
}

static void FreeBinMemory(V3dDeviceType *Instance)
{
	unsigned int i;
	for (i = 0 ; i < Instance->OutOfMemory.Index.Allocated ; ++i)
		dma_free_coherent(Instance->Device, BIN_BLOCK_BYTES, Instance->OutOfMemory.Memory[i].Virtual, Instance->OutOfMemory.Memory[i].Physical);
	Instance->OutOfMemory.Index.InUse = Instance->OutOfMemory.Index.Allocated = 0;
}


/* ================================================================ */

/* TODO: Removed reset value writes - we reset often */
static void InitialiseRegisters(V3dDeviceType *Instance)
{
	/* Disable L2 cache */
	Write(Instance, V3D_L2CACTL_OFFSET, 1 << V3D_L2CACTL_L2CDIS_SHIFT);

#if 0
	/* Reset control list executors */
	Write(Instance, V3D_CT0CS_OFFSET, 1 << V3D_CT0CS_CTRSTA_SHIFT);
	Write(Instance, V3D_CT1CS_OFFSET, 1 << V3D_CT1CS_CTRSTA_SHIFT);

	/* Clear flush and frame counts */
	Write(Instance, V3D_RFC_OFFSET, 1);
	Write(Instance, V3D_BFC_OFFSET, 1);

	/* Clear slice caches */
	Write(Instance, V3D_SLCACTL_OFFSET, 0x0f0f0f0f);
#endif

	/* Reset reserved VPM user reservation */
	Write(Instance, V3D_VPMBASE_OFFSET, Instance->Mode == V3dMode_Render ? 0U : (64 * 64) >> 8);

#if 0
	/* No limits for binning / rendering VPM allocations */
	Write(Instance, V3D_VPACNTL_OFFSET, 0);
#endif

	/* Set global debug enable */
	Write(Instance, V3D_DBCFG_OFFSET, 1);

	/* Supply over-spill binning memory */
	Write(Instance, V3D_BPOA_OFFSET, Instance->Dummy.Physical);
	Write(Instance, V3D_BPOS_OFFSET, DUMMY_BYTES);
	mb();

	/* The BPOS write doesn't take effect immediately */
	while ((Read(Instance, V3D_PCS_OFFSET) & V3D_PCS_BMOOM_MASK) != 0)
		;

	/* Set-up interrupts */
	Write(Instance, V3D_INTDIS_OFFSET, 0xf);
	Write(Instance, V3D_INTCTL_OFFSET, 0xf);
	Write(Instance, V3D_INTENA_OFFSET, 0
		| 1 << MEMORY_BIT
		/*| 1 << V3D_INTCTL_INT_FLDONE_SHIFT*/
		| 1 << V3D_INTCTL_INT_FRDONE_SHIFT);
	Write(Instance, V3D_DBQITC_OFFSET, 0xffff);
	Write(Instance, V3D_DBQITE_OFFSET, 0xffff);

	/* Clear user program counts */
	Write(Instance, V3D_SRQCS_OFFSET, (1 << V3D_SRQCS_QPURQCM_SHIFT) | (1 << V3D_SRQCS_QPURQCC_SHIFT));
	Instance->InProgress.LastCompleted = 0;
	mb();
}

static void Reset(V3dDeviceType *Instance)
{
	clk_reset(Instance->Clock);
	InitialiseRegisters(Instance);
}

static void EnableClock(V3dDeviceType *Instance)
{
	int Status;
	BUG_ON(IS_ERR_OR_NULL(Instance->Clock));
	Status = clk_enable(Instance->Clock);
	BUG_ON(Status != 0);
	pi_mgr_qos_request_update(&Instance->QosNode, 0);
	Status = pi_mgr_dfs_request_update(&Instance->DfsNode, PI_OPP_TURBO);
	BUG_ON(Status != 0);
	clk_reset(Instance->Clock);
}

static void DisableClock(V3dDeviceType *Instance)
{
	int Status;
	Status = pi_mgr_dfs_request_update(&Instance->DfsNode, PI_MGR_DFS_MIN_VALUE);
	BUG_ON(Status != 0);
	pi_mgr_qos_request_update(&Instance->QosNode, PI_MGR_QOS_DEFAULT_VALUE);
	clk_disable(Instance->Clock);
}

/* Power mutex must be held by the caller of PowerOn() and PowerOff() */
static void PowerOn(V3dDeviceType *Instance)
{
#ifdef VERBOSE_DEBUG
	printk(KERN_ERR "On\n");
#endif
	EnableClock(Instance);
	scu_standby(0);
	mb();
	Instance->On = 1;
	InitialiseRegisters(Instance);
}

static void PowerOff(V3dDeviceType *Instance)
{
#ifdef VERBOSE_DEBUG
	printk(KERN_ERR "Off\n");
#endif
	Instance->On = 0;
	DisableClock(Instance);
	scu_standby(1);
}


/* ================================================================ */

static void PostUserJob(V3dDeviceType *Instance, V3dDriver_JobType *Job)
{
	unsigned int Index = Instance->InProgress.Head++ & (V3D_USER_FIFO_LENGTH - 1);
#ifdef VERBOSE_DEBUG
	printk(KERN_ERR "%s: Issuing user job %p ID %08x (q %08x)\n", __func__, Job, Job->UserJob.job_id, Read(Instance, V3D_SRQCS_OFFSET));
#endif
	BUG_ON(Job == NULL);
	Instance->InProgress.User[Index] = Job;
	Reset(Instance);
	Job->Start = ktime_get();
	Job->State = V3DDRIVER_JOB_ACTIVE;
	Write(Instance, V3D_SRQUL_OFFSET, Job->UserJob.v3d_srqul[0]);
	Write(Instance, V3D_SRQUA_OFFSET, Job->UserJob.v3d_srqua[0]);
	Write(Instance, V3D_SRQPC_OFFSET, Job->UserJob.v3d_srqpc[0]);
}

static void PostBinRenderJob(V3dDeviceType *Instance, V3dDriver_JobType *Job)
{
	BUG_ON(Instance->InProgress.BinRender != NULL);
	BUG_ON(Job == NULL);
	Instance->InProgress.BinRender = Job;
#ifdef VERBOSE_DEBUG
	printk(KERN_ERR "%s: ID j %p:%08x %08x - %08x %08x - %08x\n", __func__,
		Job,
		Job->UserJob.job_id,
		Job->UserJob.v3d_ct0ca, Job->UserJob.v3d_ct0ea,
		Job->UserJob.v3d_ct1ca, Job->UserJob.v3d_ct1ea);
#endif
	Reset(Instance);
	Job->Start = ktime_get();
	Job->State = V3DDRIVER_JOB_ACTIVE;
	if ((Job->UserJob.job_type & V3D_JOB_BIN) != 0) {
		if (Job->UserJob.v3d_ct0ca != Job->UserJob.v3d_ct0ea) {
			BUG_ON(Job->UserJob.v3d_ct0ca == 0);
			BUG_ON(Job->UserJob.v3d_ct0ea == 0);
			Write(Instance, V3D_CT0CA_OFFSET, Job->UserJob.v3d_ct0ca);
			Write(Instance, V3D_CT0EA_OFFSET, Job->UserJob.v3d_ct0ea);
		} else
			Job->UserJob.job_type &= ~V3D_JOB_BIN;
	}
	if ((Job->UserJob.job_type & V3D_JOB_REND) != 0) {
		if (Job->UserJob.v3d_ct1ca != Job->UserJob.v3d_ct1ea) {
			BUG_ON(Job->UserJob.v3d_ct1ca == 0);
			BUG_ON(Job->UserJob.v3d_ct1ea == 0);
			Write(Instance, V3D_CT1CA_OFFSET, Job->UserJob.v3d_ct1ca);
			Write(Instance, V3D_CT1EA_OFFSET, Job->UserJob.v3d_ct1ea);
		} else {
#ifdef VERBOSE_DEBUG
			printk(KERN_ERR "%s: Job %p has no rendering\n", __func__, Job);
#endif
			Job->UserJob.job_type &= ~V3D_JOB_REND;
		}
	}
}

#ifdef WORKAROUND_GFXH16
static int Gfxh16Thread(void *Context)
{
	V3dDeviceType *Instance = (V3dDeviceType *) Context;
	while (msleep(60), kthread_should_stop() == 0) {
		int Status = mutex_lock(&Instance->Power);
		if (Status != 0)
			break;
		if (Instance->On != 0) {
			/* Clear caches - see GFXH-16 */
			Write(Instance, V3D_SLCACTL_OFFSET, 0x0f0f0f0f);
		}
		mutex_unlock(&Instance->Power);
	}
	return 0;
}
#endif

static void SetMode(V3dDeviceType *Instance, V3dModeType Mode)
{
	if (Mode != Instance->Mode)
		Instance->Mode = Mode;
}


/* ================================================================ */

static void SwitchOff(struct work_struct *Work)
{
	V3dDeviceType *Instance = container_of(Work, V3dDeviceType, SwitchOff.work);
	mutex_lock(&Instance->Power);
	if (Instance->Idle != 0) {
		BUG_ON(Instance->InProgress.BinRender != NULL);
		BUG_ON(Instance->InProgress.Head != Instance->InProgress.Tail);
		PowerOff(Instance);
		mutex_unlock(&Instance->Suspend);
	}
	mutex_unlock(&Instance->Power);
}


/* ================================================================ */

/* Power mutex must be held by the caller */
static V3dDriver_JobType *PostJob(V3dDeviceType * Instance)
{
	unsigned long      Flags;
	V3dDriver_JobType *Job;
	if (Instance->Idle == 0)
		return NULL;

	/* Check for any jobs */
	Flags = V3dDriver_JobLock(Instance->Driver.Instance);
	Job = V3dDriver_JobGet(Instance->Driver.Instance, V3D_JOB_USER | V3D_JOB_BIN_REND);
	V3dDriver_JobUnlock(Instance->Driver.Instance, Flags);
	if (Job == NULL)
		return NULL;

	/* Starting a job */

	/* Avoid a race with SwitchOff */
	if (Instance->Idle != 0)
		cancel_delayed_work(&Instance->SwitchOff);
	Instance->Idle = 0; /* Can't be switched-off from here on */

	/* Do we need to switch on? */
	if (Instance->On == 0) {
		mutex_lock(&Instance->Suspend); /* TODO: mutex_lock_interruptible */
		Instance->Mode = Job->UserJob.job_type == V3D_JOB_USER ? V3dMode_User : V3dMode_Render;
		PowerOn(Instance);
	}
	BUG_ON(Instance->On == 0);

	/* Post the job to the hardware */
	if (Job->UserJob.job_type == V3D_JOB_USER) {
		SetMode(Instance, V3dMode_User);
		PostUserJob(Instance, Job);
	} else {
		SetMode(Instance, V3dMode_Render);
		PostBinRenderJob(Instance, Job);
	}
	return Job;
}


/* ================================================================ */

void V3dDevice_Delete(V3dDeviceType *Instance)
{
	switch (Instance->Initialised) {
	case 7:
		mutex_lock(&Instance->Suspend);
		free_irq(IRQ_GRAPHICS, NULL);
		flush_delayed_work(&Instance->SwitchOff);
		FreeBinMemory(Instance);

	case 6:
		pi_mgr_qos_request_remove(&Instance->QosNode);

	case 5:
		pi_mgr_dfs_request_remove(&Instance->DfsNode);

	case 4:
#ifdef WORKAROUND_GFXH16
		kthread_stop(Instance->Gfxh16Thread);
#endif

	case 3:
#if 0
		kthread_stop(Instance->Thread);
		complete(&Instance->PostEvent);
#endif

	case 2:
		destroy_workqueue(Instance->OutOfMemory.WorkQueue);

	case 1:
		dma_free_coherent(
			Instance->Device,
			DUMMY_BYTES,
			Instance->Dummy.Virtual,
			Instance->Dummy.Physical);

	case 0:
		clk_put(Instance->Clock);
		kfree(Instance);
		break;
	}
}

static void ResetState(V3dDeviceType *Instance)
{
	Instance->InProgress.BinRender = NULL;
	Instance->InProgress.Head = Instance->InProgress.Tail = 0;
	Instance->InProgress.LastCompleted = 0;
}

V3dDeviceType *V3dDevice_Create(V3dDriverType *Driver, struct device *Device, uint32_t RegisterBase)
{
	V3dDeviceType *Instance = kmalloc(sizeof(V3dDeviceType), GFP_KERNEL);
	int            Status;
	if (Instance == NULL)
		return NULL;
	Instance->Initialised = 0;

	/* Unconditional initialisation */
	Instance->Mode = V3dMode_Undefined;
	Instance->Register.Base = RegisterBase;
	spin_lock_init(&Instance->OutOfMemory.Lock);
	Instance->OutOfMemory.Index.InUse     = 0;
	Instance->OutOfMemory.Index.Allocated = 0;
	INIT_WORK(&Instance->OutOfMemory.BinAllocation, AllocateBinMemory);
	memset(
		&Instance->OutOfMemory.Memory[0],
		0,
		sizeof(Instance->OutOfMemory.Memory));

	Instance->Driver.Instance = Driver;
	Instance->Device = Device;
	Instance->Idle = 1;
	mutex_init(&Instance->Suspend);
	mutex_init(&Instance->Power);
	Instance->On = 0;
	INIT_DELAYED_WORK(&Instance->SwitchOff, SwitchOff);

	ResetState(Instance);

	Instance->Clock = clk_get(NULL, "v3d_axi_clk");
	BUG_ON(Instance->Clock == NULL);

	/* Initialisation that could fail follows */
	Instance->Dummy.Virtual = dma_alloc_coherent(
		Instance->Device,
		DUMMY_BYTES,
		&Instance->Dummy.Physical,
		GFP_DMA);
	if (Instance->Dummy.Virtual == NULL)
		return printk(KERN_ERR "%s:%d fail\n", __func__, __LINE__), V3dDevice_Delete(Instance), NULL;
	++Instance->Initialised;

	Instance->OutOfMemory.WorkQueue = create_singlethread_workqueue("BinAlloc");
	if (Instance->OutOfMemory.WorkQueue == NULL)
		return printk(KERN_ERR "%s:%d fail\n", __func__, __LINE__), V3dDevice_Delete(Instance), NULL;
	++Instance->Initialised;

#if 0
	Instance->Thread = kthread_run(&Thread, Instance, "V3dDevice");
	if (IS_ERR(Instance->Thread))
		return printk(KERN_ERR "%s:%d fail\n", __func__, __LINE__), V3dDevice_Delete(Instance), NULL;
#endif
	++Instance->Initialised;

#ifdef WORKAROUND_GFXH16
	Instance->Gfxh16Thread = kthread_run(&Gfxh16Thread, Instance, "V3dDeviceGfxh16");
	if (IS_ERR(Instance->Gfxh16Thread))
		return printk(KERN_ERR "%s:%d fail\n", __func__, __LINE__), V3dDevice_Delete(Instance), NULL;
#endif
	++Instance->Initialised;

	memset(&Instance->QosNode, 0, sizeof(Instance->QosNode));
	memset(&Instance->DfsNode, 0, sizeof(Instance->DfsNode));
	Status = pi_mgr_qos_add_request(
		&Instance->QosNode,
		"v3d",
		PI_MGR_PI_ID_ARM_CORE,
		PI_MGR_QOS_DEFAULT_VALUE);
	if (Status != 0)
		return printk(KERN_ERR "%s:%d fail\n", __func__, __LINE__), V3dDevice_Delete(Instance), NULL;
	++Instance->Initialised;

	Status = pi_mgr_dfs_add_request(
		&Instance->DfsNode,
		"v3d",
		PI_MGR_PI_ID_MM,
		PI_MGR_DFS_MIN_VALUE);
	if (Status != 0)
		return printk(KERN_ERR "%s:%d fail\n", __func__, __LINE__), V3dDevice_Delete(Instance), NULL;
	++Instance->Initialised;

	Status = request_irq(IRQ_GRAPHICS, InterruptHandler, IRQF_TRIGGER_HIGH, "v3d", Instance);
	if (Status != 0)
		return printk(KERN_ERR "%s:%d fail\n", __func__, __LINE__), V3dDevice_Delete(Instance), NULL;
	++Instance->Initialised;

	return Instance;
}


/* ================================================================ */

void V3dDevice_JobPosted(V3dDeviceType *Instance)
{
	V3dDriver_JobType *Job;
	int                Status;
	if (Instance->Idle == 0)
		return; /* Job already on the queue, so no race */

	Status = mutex_lock_interruptible(&Instance->Power);
	if (Status != 0)
		return;
	Job = PostJob(Instance);
	mutex_unlock(&Instance->Power);
}


/* ================================================================ */

/* Returns Complete? */
static int HandleBinRenderInterrupt(V3dDeviceType *Instance, uint32_t Status)
{
	int CompleteBinRender = 0;

	/* Acknowledge the interrupts that we'll serviced */
	Write(Instance, V3D_INTCTL_OFFSET, Status);

	if ((Status & (1 << V3D_INTCTL_INT_FLDONE_SHIFT)) != 0) {
		/* Binning completed */
		if (Instance->InProgress.BinRender->UserJob.job_type == V3D_JOB_BIN)
			CompleteBinRender = 1;
	} else
		if ((Status & (1 << MEMORY_BIT)) != 0) {
			/* Need more overflow binning memory */
			V3dBinMemoryType *Memory = NULL;
			spin_lock(&Instance->OutOfMemory.Lock);
			if (Instance->OutOfMemory.Index.InUse != Instance->OutOfMemory.Index.Allocated)
				Memory = &Instance->OutOfMemory.Memory[Instance->OutOfMemory.Index.InUse++];
			spin_unlock(&Instance->OutOfMemory.Lock);
			if (Memory != NULL)
				SupplyBinningMemory(Instance, Memory);
			else {
				/* Need another allocation */
				Write(Instance, V3D_INTDIS_OFFSET, 1 << MEMORY_BIT);
				queue_work(Instance->OutOfMemory.WorkQueue, &Instance->OutOfMemory.BinAllocation);
			}
		}
	if ((Status & (1 << V3D_INTCTL_INT_FRDONE_SHIFT)) != 0)
		CompleteBinRender = 1; /* Render complete => job complete */

	if (CompleteBinRender != 0) {
		/* Job complete */
		ReleaseBinMemory(Instance);
		V3dSession_Complete(Instance->InProgress.BinRender);
		Instance->InProgress.BinRender = NULL;
	}
	return CompleteBinRender;
}

/* Queue as many user requests as possible */
#if 0
static void TopUpUserFifo(V3dDeviceType *Instance)
{
	unsigned long Flags = V3dDriver_JobLock(Instance->Driver.Instance);
	if (Instance->InProgress.Head - Instance->InProgress.Tail < V3D_USER_FIFO_LENGTH) {
		V3dDriver_JobType *Job = V3dDriver_JobGet(Instance->Driver.Instance, V3D_JOB_USER);
#ifdef VERBOSE_DEBUG
		printk(KERN_ERR "%s: Posting, entries %x (%x - %x)\n",
			__func__,
			Instance->InProgress.Head - Instance->InProgress.Tail,
			Instance->InProgress.Head,
			Instance->InProgress.Tail);
#endif
		if (Job != NULL)
			PostUserJob(Instance, Job);
	}
	V3dDriver_JobUnlock(Instance->Driver.Instance, Flags);
}
#endif

int HandleUserInterrupt(V3dDeviceType *Instance, uint32_t QpuStatus)
{
	/* User job(s) completed */
	V3dDriver_JobType *Job;
	unsigned int       Index;
	uint32_t           UserFifo = Read(Instance, V3D_SRQCS_OFFSET);
	int                Complete = 0;

	/* TODO: Correct for queueing multiple consequtive user jobs */
	unsigned int Completed = UserFifo >> V3D_SRQCS_QPURQCC_SHIFT
		& V3D_SRQCS_QPURQCC_MASK >> V3D_SRQCS_QPURQCC_SHIFT;
	BUG_ON((QpuStatus & 1 << V3D_SRQCS_QPURQERR_SHIFT) != 0);
#ifdef VERBOSE_DEBUG
	printk(KERN_ERR "%s: s %08x c %1x\n", __func__, UserFifo, Completed);
#endif

	/* Acknowledge the interrupts that we'll serviced */
	if (QpuStatus != 0)
		Write(Instance, V3D_DBQITC_OFFSET, QpuStatus);

	/* Complete user jobs */
	for (; Completed != (Instance->InProgress.LastCompleted & (V3D_SRQCS_QPURQCC_MASK >> V3D_SRQCS_QPURQCC_SHIFT)) ; ++Instance->InProgress.LastCompleted, Complete = 1) {
		Index = Instance->InProgress.Tail++ & (V3D_USER_FIFO_LENGTH - 1);
		Job   = Instance->InProgress.User[Index];
		BUG_ON(Job == NULL);
		V3dSession_Complete(Job);
	}
#ifdef VERBOSE_DEBUG
	printk(KERN_ERR "%s: Completed ..%4x\n", __func__, Completed);
#endif
	return Complete;
}

static uint32_t GetStatus(V3dDeviceType *Instance, uint32_t *Status, uint32_t *QpuStatus)
{
	uint32_t Enable = Read(Instance, V3D_INTENA_OFFSET);
	*Status    = Read(Instance, V3D_INTCTL_OFFSET) & Enable;
	*QpuStatus = Read(Instance, V3D_DBQITC_OFFSET);
	return *Status | *QpuStatus;
}

static irqreturn_t InterruptHandler(int Irq, void *Context)
{
	V3dDeviceType *Instance  = (V3dDeviceType *) Context;
	uint32_t       Status, QpuStatus;
	int            Complete = 0;
	BUG_ON(Instance->Idle != 0);
	while (GetStatus(Instance, &Status, &QpuStatus) != 0) {
		if (Status != 0)
			Complete |= HandleBinRenderInterrupt(Instance, Status);
		if (QpuStatus != 0)
			Complete |= HandleUserInterrupt(Instance, QpuStatus);
	}

	if (Complete != 0) {
		/* Job complete */
		V3dDriver_JobType *Job;
		unsigned long      Flags;
		/* Issue the next one */
		Flags = V3dDriver_JobLock(Instance->Driver.Instance);
		Job = V3dDriver_JobGet(Instance->Driver.Instance, V3D_JOB_USER | V3D_JOB_BIN_REND);
		if (Job != NULL) {
#ifdef VERBOSE_DEBUG
			printk(KERN_ERR "%s: Issuing job %p\n", __func__, Job);
#endif
			/* Post the job to the hardware */
			if (Job->UserJob.job_type == V3D_JOB_USER) {
				SetMode(Instance, V3dMode_User);
				PostUserJob(Instance, Job);
			} else {
				SetMode(Instance, V3dMode_Render);
				PostBinRenderJob(Instance, Job);
			}
		}
		V3dDriver_JobUnlock(Instance->Driver.Instance, Flags);
	}

	/* Update Idle - we only go from busy to idle here */
	if (Instance->InProgress.BinRender == NULL
		&& Instance->InProgress.Head == Instance->InProgress.Tail
		&& (Status & (1 << MEMORY_BIT)) == 0) {
		Instance->Idle = 1;
		schedule_delayed_work(&Instance->SwitchOff, msecs_to_jiffies(DELAY_SWITCHOFF_MS));
	}
	return IRQ_HANDLED;
}


/* ================================================================ */

void V3dDevice_Suspend(V3dDeviceType *Instance)
{
	mutex_lock(&Instance->Suspend);
}

void V3dDevice_Resume(V3dDeviceType *Instance)
{
	mutex_unlock(&Instance->Suspend);
}
