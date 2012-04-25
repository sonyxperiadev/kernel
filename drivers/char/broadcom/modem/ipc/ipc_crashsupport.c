/****************************************************************************
*
*   Copyright (c) 2009 Broadcom Corporation
*
*   Unless you and Broadcom execute a separate written software license
*   agreement governing use of this software, this software is licensed to you
*   under the terms of the GNU General Public License version 2, available
*    at http://www.gnu.org/licenses/old-licenses/gpl-2.0.html (the "GPL").
*
*   Notwithstanding the above, under no circumstances may you combine this
*   software in any way with any other Broadcom software provided under a
*   license other than the GPL, without Broadcom's express prior
*   written consent.
*
****************************************************************************/

#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/cciss_ioctl.h>
#include <linux/string.h>
#include <linux/serial_reg.h>
#include <linux/workqueue.h>
#include <linux/kthread.h>
#include <mach/rdb/brcm_rdb_sysmap.h>
#include <mach/rdb/brcm_rdb_bmdm_rst_mgr_reg.h>
#include <mach/rdb/brcm_rdb_root_rst_mgr_reg.h>
#ifdef CONFIG_HAS_WAKELOCK
#include <linux/wakelock.h>
#endif
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/uaccess.h>
#include <linux/interrupt.h>
#include <linux/broadcom/ipcinterface.h>
#include <linux/syscalls.h>
#include "ipc_sharedmemory.h"

#include "ipc_debug.h"
#include "ipc_stubs.h"
#include "bcmlog.h"
#include "ipc_crashsupport.h"
#ifdef CONFIG_FB_BRCM_CP_CRASH_DUMP_IMAGE_SUPPORT
#include <video/kona_fb_image_dump.h>
#endif

/* CP crash recovery action */
#define   RECOVERY_ACTION_NONE                      0
#define   RECOVERY_ACTION_SYSRESET                  1
#define   RECOVERY_ACTION_SYSRESET_USERCONFIRM      2
#define   RECOVERY_ACTION_TBD                       3

/* following structs must match definition for CP from dump.h
 * **FIXME** MAG - add dump.h to headers imported from CP when doing
 * header cleanup...
 */
struct T_RAMDUMP_BLOCK {
	unsigned int mem_start;
	unsigned int mem_size;
	/* 0xFFFFFFFF means stand-alone ramdump block */
	unsigned int buffer_in_main;
	char name[8];		/* one of names must be "MAIN" */
};

struct T_CRASH_SUMMARY {
	char *reason;
	char *file;
	unsigned int line;
	unsigned int value;
	unsigned char *ptr;
	unsigned int size;
	char *link_signature;
	char *DSP_version;
	char *FW_version;
	char *project_version;
	char *decoder_version;
	char *thread;
	unsigned int time;
	unsigned int save_reg_area;
	char *func_trace;
	struct T_RAMDUMP_BLOCK *mem_dump;
};

/* also from dump.h*/

#define	SIM_DEBUG_DATA		0x18000000
#define	SIM_AP_DEBUG_DATA	0x19000000
#define	ASSERT_BUF_SIZE	    512
#define MAX_RAMDUMP_BLOCKS  16
static char assert_buf[ASSERT_BUF_SIZE];
static int crashCount;
static struct T_CRASH_SUMMARY *dumped_crash_summary_ptr = { 0 };

/* maximum number of times we'll try to re-establish comms with
 * CP during crash log dump */
#define MAX_CP_DUMP_RETRIES 5
#define TICKS_ONE_SECOND 1024

/* internal helper functions */
static void DUMP_CP_assert_log(void);
static void DUMP_CPMemoryByList(struct T_RAMDUMP_BLOCK *mem_dump);

static void GetStringFromPA(UInt32 inPhysAddr, char *inStrBuf,
			    UInt32 inStrBufLen);

#ifdef CONFIG_HAS_WAKELOCK
extern struct wake_lock ipc_wake_lock;
#endif

static struct timer_list cp_reset_timer;
/* set CP reset timeout to 2 seconds for now */
#define CP_RESET_TIMEOUT_MILLISEC	2000

/* wait for 500ms, 20ms at a time, for CP to be ready to reset */
#define WAIT_FOR_CP_RESET_READY_MILLISEC	20
#define WAIT_FOR_CP_RESET_READY_ITERATIONS	25

extern int RpcDbgDumpHistoryLogging(int type, int level);
extern void Comms_Start(int isReset);
extern int ipcs_reinitialize_ipc(void);

/*********************************************************************
*
*   Retrieve string from physical address
*
*	@param inPhysAddr   (in) Physical address of string.
*	@param inStrBuf	    (in) Pointer to buffer to copy string into.
*	@param inStrBufLen	(in) Size of inStrBuf in bytes.
*	@return	Null terminated string from physical address is
*                       copied in to buffer pointed to by inStrBuf.
*
***********************************************************************/
void GetStringFromPA(UInt32 inPhysAddr, char *inStrBuf, UInt32 inStrBufLen)
{
	void __iomem *virtAddr;

	virtAddr = ioremap_nocache(inPhysAddr, inStrBufLen);

	strncpy(inStrBuf, (char *)virtAddr, inStrBufLen);

	/* pad NULL in the end of the string */
	inStrBuf[inStrBufLen - 1] = '\0', iounmap(virtAddr);
}

static IPCAP_CPResetHandler_T sCPResetHandler = NULL;

/* registers client callback to be used for passing silent CP reset events */
int IPCAP_RegisterCPResetHandler(IPCAP_CPResetHandler_T inResetHandler)
{
	/* **FIXME** need to support multiple clients, or just RPC? */
	sCPResetHandler = inResetHandler;
	IPC_DEBUG(DBG_ERROR, "cp reset handler registered\n");
	return 1;
}

void HandleCPResetDone(void)
{
	IPC_DEBUG(DBG_ERROR, "notifying cp reset IPC_CPRESET_COMPLETE\n");
	/* kick off notification to upper layer clients */
	if (sCPResetHandler)
		sCPResetHandler(IPC_CPRESET_COMPLETE);
	IPC_DEBUG(DBG_ERROR, "DONE notifying cp reset IPC_CPRESET_COMPLETE\n");
}

int HandleRestartCP(void *data)
{
	int k = 0;
	void __iomem *cp_root_reset_base;
	void __iomem *cp_bmdm_reset_base;

	IPC_DEBUG(DBG_ERROR, "enter\n");

	/* verify that CP is ready to be reset */
	while (IPC_CP_SILENT_RESET_READY !=
		SmLocalControl.SmControl->CrashCode) {
		/* not yet ready, so we'll wait up to 500ms */
		if (k++ > WAIT_FOR_CP_RESET_READY_ITERATIONS)
			break;
		else
			msleep(WAIT_FOR_CP_RESET_READY_MILLISEC);
	}

	if (IPC_CP_SILENT_RESET_READY != SmLocalControl.SmControl->CrashCode) {
		/* CP not responding as ready, so we crash here */
		IPC_DEBUG(DBG_ERROR, "CP not ready for reset, crashing\n");
		BUG();
	}

	/* reload CP */
	/* **FIXME** add call to Lori's code here */

	IPC_DEBUG(DBG_ERROR, "resetting CP\n");

	/* reset CP - copy from cp_reset.cmm rxd from CP team */
	/*;CP reset, from AP
	 *
	 * D.S ZSD:0x35001F00 %LE %LONG 0xa5a501
	 * D.S ZSD:0x35001F08 %LE %LONG 0x3bd	;reset
	 * D.S ZSD:0x35001F08 %LE %LONG 0x3fd	;clear
	 */
	cp_root_reset_base = ioremap(ROOT_RST_BASE_ADDR,
				ROOT_RST_MGR_REG_PD_SOFT_RSTN_OFFSET+4);
	if (!cp_root_reset_base) {
		IPC_DEBUG(DBG_ERROR,
			"failed to remap ROOT_RST_BASE_ADDR, crashing\n");
		BUG();
	}
	writel(0xa5a501, cp_root_reset_base+ROOT_RST_MGR_REG_WR_ACCESS_OFFSET);
	writel(0x3bd, cp_root_reset_base+ROOT_RST_MGR_REG_PD_SOFT_RSTN_OFFSET);
	writel(0x3fd, cp_root_reset_base+ROOT_RST_MGR_REG_PD_SOFT_RSTN_OFFSET);

	/* reset R4 - copy from cp_reset.cmm rxd from CP team */
	/*;R4 reset
	*
	* D.S ZSD:0x3a055f00 %LE %LONG 0xa5a501
	* D.S ZSD:0x3a055f18 %LE %LONG 0x2 ;reset
	* D.S ZSD:0x3a055f18 %LE %LONG 0x3  ;clear
	*/
	cp_bmdm_reset_base = ioremap(BMDM_RST_BASE_ADDR,
				BMDM_RST_MGR_REG_CP_RSTN_OFFSET+4);
	if (!cp_bmdm_reset_base) {
		IPC_DEBUG(DBG_ERROR,
			"failed to remap BMDM_RST_BASE_ADDR, crashing\n");
		BUG();
	}
	writel(0xa5a501, cp_bmdm_reset_base+BMDM_RST_MGR_REG_WR_ACCESS_OFFSET);
	writel(0x2, cp_bmdm_reset_base+BMDM_RST_MGR_REG_CP_RSTN_OFFSET);
	writel(0x3, cp_bmdm_reset_base+BMDM_RST_MGR_REG_CP_RSTN_OFFSET);

	IPC_DEBUG(DBG_ERROR, "rebooting CP\n");
	/* reboot CP; this will also wipe IPC shared memory */
	Comms_Start(1);

	IPC_DEBUG(DBG_ERROR, "re-init IPC\n");
	/* reinitialize IPC, and wait for IPC sync with CP */
	if (ipcs_reinitialize_ipc()) {
		IPC_DEBUG(DBG_ERROR, "ipcs_reinitialize_ipc failed\n");
		/* CP didn't re-sync, so crash AP here */
		BUG();
	}

	IPC_DEBUG(DBG_ERROR, "reenable IRQ_IPC_C2A\n");
	/* re-enable CP to AP IRQ */
	enable_irq(IRQ_IPC_C2A);

	/* notify clients that we're back in business...*/
	IPC_DEBUG(DBG_ERROR, "notifying clients CP reset is complete\n");
	HandleCPResetDone();
	IPC_DEBUG(DBG_ERROR, "notification done, exiting reset thread\n");

	iounmap(cp_root_reset_base);
	iounmap(cp_bmdm_reset_base);

	/* done with thread */
	do_exit(0);
}

/* callback from client indicating it is ready for CP reset */
void IPCAP_ReadyForReset( int inClientID )
{
	IPC_DEBUG(DBG_ERROR, "ready for reset\n");
	
	/* get rid of the ack timeout timer */
	del_timer(&cp_reset_timer);

	IPC_DEBUG(DBG_ERROR, "starting cp_reset thread\n");

	/* kick off CP restart thread */
	kthread_run(HandleRestartCP, 0, "cp_reset");
}

void CPReset_Timer_Callback(unsigned long data)
{
	/* not all IPC/RPC clients ackd the reset in time, so crash AP */
	IPC_DEBUG(DBG_ERROR, "cp reset timeout %ld jiffies\n", jiffies);
	BUG();
}

void HandleCPResetStart(void)
{
	int ret;

	/* per Silent CP Reset doc, need to:
		- ack the silent cp reset notification from CP
		- exit low power mode
		- disable AP interrupts except CAPI HW int
			- is this right? all interrupts? Check with Derek
		- do reset notification to clients 
		- audio/dsp interface reset (assume this is done in audio
		  driver during reset notification process)
	    
	    Should also start timer here, and if reset notification process
	    isn't complete in X seconds, 
	*/

	/* ACK start of silent reset to CP */
	SmLocalControl.SmControl->CrashCode = IPC_AP_ACK_CP_RESET_START;

	/* exit low power mode:
	 * ipc_wake_lock acquired on IPC interrupt that triggered this
	 * crash handler, and is not released until CP reset is complete
	 */

	IPC_DEBUG(DBG_ERROR, "disabling IRQ_IPC_C2A\n");

	/* disable CP to AP interrupt */
	disable_irq(IRQ_IPC_C2A);

	/* set timeout timer for ack from IPC/RPC clients; if timer fires
	 * before all have ack'd, we should crash AP
	*/
	setup_timer(&cp_reset_timer, CPReset_Timer_Callback, 0);
	ret = mod_timer(&cp_reset_timer, jiffies +
		msecs_to_jiffies(CP_RESET_TIMEOUT_MILLISEC));
	if (ret)
		IPC_DEBUG(DBG_ERROR, "ERROR starting CP reset timer %d\n",
					ret);
	else
		IPC_DEBUG(DBG_ERROR, "CP reset timeout timer set for %d ms\n",
				CP_RESET_TIMEOUT_MILLISEC);

	IPC_DEBUG(DBG_ERROR, "start notifying cp reset IPC_CPRESET_START\n");

	/* kick off notification to upper layer clients */
	if (sCPResetHandler)
		sCPResetHandler(IPC_CPRESET_START);

	IPC_DEBUG(DBG_ERROR, "DONE notifying cp reset IPC_CPRESET_START\n");
}

/*************************************************
*
*   Worker thread to dump CP crash log information.
*
*
*****************************************************/
void ProcessCPCrashedDump(struct work_struct *work)
{
	char crashReason[40] = { 0 };
	char crashFile[40] = { 0 };
	char crashThread[40] = { 0 };
	char outString[512] = { 0 };
	IPC_U32 *Dump;
	void __iomem *DumpVAddr;

#ifdef CONFIG_FB_BRCM_CP_CRASH_DUMP_IMAGE_SUPPORT
	rhea_display_crash_image(CP_CRASH_DUMP_START);
#endif

#ifdef CONFIG_BCM_AP_PANIC_ON_CPCRASH
	if (BCMLOG_OUTDEV_SDCARD == BCMLOG_GetCpCrashLogDevice()
#ifdef CONFIG_CDEBUGGER
		&& ramdump_enable == 1
#endif
#ifdef CONFIG_APANIC_ON_MMC
		&& ap_triggered == 0
#endif
		) {
		/* we kill AP when CP crashes */
		IPC_DEBUG(DBG_ERROR, "Crashing AP for Ramdump ...\n\n");
		abort();
	}
	if ((BCMLOG_OUTDEV_NONE == BCMLOG_GetCpCrashLogDevice() ||
		BCMLOG_OUTDEV_PANIC == BCMLOG_GetCpCrashLogDevice() ||
		BCMLOG_OUTDEV_STM == BCMLOG_GetCpCrashLogDevice() ||
		BCMLOG_OUTDEV_RNDIS == BCMLOG_GetCpCrashLogDevice())
#ifdef CONFIG_APANIC_ON_MMC
		&& ap_triggered == 0
#endif
	    ) {
		/* we kill AP when CP crashes */
		IPC_DEBUG(DBG_ERROR, "Crashing AP now ...\n\n");
		abort();
	}
#endif

	IPC_Dump();
	RpcDbgDumpHistoryLogging(0, 0);

#if defined(CONFIG_BRCM_CP_CRASH_DUMP) \
	|| defined(CONFIG_BRCM_CP_CRASH_DUMP_EMMC) \
	|| defined(CONFIG_BCM_AP_PANIC_ON_CPCRASH)
	while (SmLocalControl.SmControl->CrashDump == NULL)
		; /* No op */
#endif

	/* **NOTE** for now, continue doing simple dump out IPC_DEBUG so there
	 * is some indication of CP crash in console
	 * (in case user not running MTT) */
	Dump = (void *)SmLocalControl.SmControl->CrashDump;

	IPC_DEBUG(DBG_ERROR, "ioremap_nocache\n");
	DumpVAddr = ioremap_nocache((UInt32) Dump,
				    sizeof(struct T_CRASH_SUMMARY));
	if (NULL == DumpVAddr) {
		IPC_DEBUG(DBG_ERROR, "VirtualAlloc failed\n");
		goto cleanUp;
	}

	IPC_DEBUG(DBG_ERROR, "Crash Summary Virtual Addr: 0x%08X\n",
		  (unsigned int)DumpVAddr);

	dumped_crash_summary_ptr = (struct T_CRASH_SUMMARY *)DumpVAddr;

	IPC_DEBUG(DBG_ERROR, "===== COMMS_PROCESSOR crash summary =====\r\n");

	if (dumped_crash_summary_ptr->link_signature) {
		GetStringFromPA((UInt32) dumped_crash_summary_ptr->
				link_signature, outString, 128);
		IPC_DEBUG(DBG_ERROR, "%s\r\n", outString);
	}

	if (dumped_crash_summary_ptr->project_version) {
		GetStringFromPA((UInt32) dumped_crash_summary_ptr->
				project_version, outString, 128);
		IPC_DEBUG(DBG_ERROR, "%s\r\n", outString);
	}

	if (dumped_crash_summary_ptr->DSP_version) {
		GetStringFromPA((UInt32) dumped_crash_summary_ptr->DSP_version,
				outString, 128);
		IPC_DEBUG(DBG_ERROR, "%s\r\n", outString);
	}

	if (dumped_crash_summary_ptr->FW_version) {
		GetStringFromPA((UInt32) dumped_crash_summary_ptr->FW_version,
				outString, 128);
		IPC_DEBUG(DBG_ERROR, "%s\r\n", outString);
	}

	if (dumped_crash_summary_ptr->decoder_version) {
		GetStringFromPA((UInt32) dumped_crash_summary_ptr->
				decoder_version, outString, 128);
		IPC_DEBUG(DBG_ERROR, "%s\r\n", outString);
	}

	GetStringFromPA((UInt32) dumped_crash_summary_ptr->reason, crashReason,
			40);

	GetStringFromPA((UInt32) dumped_crash_summary_ptr->file, crashFile, 40);

	GetStringFromPA((UInt32) dumped_crash_summary_ptr->thread, crashThread,
			40);

	IPC_DEBUG(DBG_ERROR, "%s f=%s l=%d v=%d/0x%x t=%s TS=%d\r\n",
		  crashReason,
		  crashFile,
		  dumped_crash_summary_ptr->line,
		  dumped_crash_summary_ptr->value,
		  dumped_crash_summary_ptr->value,
		  crashThread, dumped_crash_summary_ptr->time);

	/* done with "simple" dump, so now pull the full assert
	 * log from CP and dump out to MTT */
	DUMP_CP_assert_log();

cleanUp:

	if (NULL != DumpVAddr)
		iounmap(DumpVAddr);

#ifdef CONFIG_HAS_WAKELOCK
	wake_unlock(&ipc_wake_lock);
#endif

#ifdef CONFIG_BCM_AP_PANIC_ON_CPCRASH

#ifdef CONFIG_SEC_DEBUG
	cp_abort();
#endif /* CONFIG_SEC_DEBUG */

#endif /* CONFIG_AP_PANIC_ON_CPCRASH */

}

/****************************************************************
*   Called from the IPC interrupt service thread in ipc_server.c
*   to check if CP has crashed.
*
*	@return int		0 if CP not crashed, 1 otherwise
*
*****************************************************************/
int IpcCPCrashCheck(void)
{
	IPC_CrashCode_T CpCrashReason;
	IPC_U32 *Dump;
	crashCount = 0;

	if (!SmLocalControl.ConfiguredReported) {
		IPC_DEBUG(DBG_TRACE, "IPC Not Initialised\n");
		return 0;
	}
	/* Get the crash reason and crash dump location */
	CpCrashReason = SmLocalControl.SmControl->CrashCode;
	Dump = (void *)SmLocalControl.SmControl->CrashDump;

	if (IPC_CP_NOT_CRASHED != CpCrashReason
	    && IPC_CP_MAX_CRASH_CODE > CpCrashReason && NULL != Dump) {
		crashCount++;
		IPC_DEBUG(DBG_ERROR,
			  "CP Crashed!! CP Ticks[%ld] reason:%d count:%d Dump:0x%X\n",
			  TIMER_GetValue(), CpCrashReason, crashCount,
			  (unsigned int)Dump);

		if (crashCount > 1)
			return 0;

		return 1;
	}

	return 0;
}

/******************************************************************
*   Utility function to retrieve full crash log from CP via simple
*   handshake protocol.
*
*
********************************************************************/
void DUMP_CP_assert_log(void)
{
	UInt32 t0, t1, i, size, retryCount;
	UInt8 *p;
	UInt32 packetCount = 0;
	void __iomem *AssertLogVAddr = NULL;
	struct file *sdDumpFile = NULL;

	/* put logging driver into crash dump mode; messages will be sent
	 * straight out to MTT via RNDIS (or dump file) instead of buffering
	 * in RING buffer (flood of crash dump info overloads ring buffer
	 * otherwise,and we lose a lot of crash dump info)
	 * NOTE: crash dump is put into SD by default; if SD file fails to open,
	 * then we'll try sending it out RNDIS */

	BCMLOG_StartCpCrashDump(sdDumpFile);

	retryCount = 0;
	t0 = TIMER_GetValue();
	while (1) {
		t1 = TIMER_GetValue();

		/* signal to CP that we're ready to receive crash log... */
		SmLocalControl.SmControl->CrashCode = IPC_AP_CLEAR_TO_SEND;

		/* wait for CP to "dump"; CrashCode field will be
		 * set to physical address of current assert buf */
		while (SmLocalControl.SmControl->CrashCode ==
		       IPC_AP_CLEAR_TO_SEND) {
			for (i = 0; i < 256; i++)
				;
			if (TIMER_GetValue() - t1 > TICKS_ONE_SECOND * 20)
				break;
		}

		/* check for time out */
		if (SmLocalControl.SmControl->CrashCode ==
			IPC_AP_CLEAR_TO_SEND) {
			if (retryCount < MAX_CP_DUMP_RETRIES) {
				retryCount++;
				IPC_DEBUG(DBG_TRACE,
					  "timeout %d, trying again...\n",
					  (int)retryCount);
				continue;
			} else {
				/* no response from CP, so get out of here */
				IPC_DEBUG(DBG_ERROR,
					  "Abort --- max retries %d reached\n",
					  (int)retryCount);
				break;
			}
		}
		/* reset retry counter */
		retryCount = 0;

		/* get virtual address of CP assert buffer */
		AssertLogVAddr = ioremap_nocache((UInt32)
						 (SmLocalControl.SmControl->
						  CrashCode), ASSERT_BUF_SIZE);
		if (NULL == AssertLogVAddr) {
			IPC_DEBUG(DBG_ERROR,
				  "ioremap_nocache failed in DUMP_CP_assert_log\n");
			break;
		}

		p = (UInt8 *) AssertLogVAddr;

		/* number of bytes in assert buffer */
		size = (p[0] << 8) + p[1];

		/* size of 0 means CP is done dumping assert log */
		if (size == 0) {
			IPC_DEBUG(DBG_ERROR,
				  "assert log size 0, exiting, packetCount:0x%x\n",
				  (int)packetCount);
			iounmap(AssertLogVAddr);
			AssertLogVAddr = NULL;
			break;
		}
		/* sanity check for too beaucoup... */
		if (size > ASSERT_BUF_SIZE - 2) {
			IPC_DEBUG(DBG_ERROR,
				  "Abort --- improper size [%08x]=%d\n",
				  SmLocalControl.SmControl->CrashCode,
				  (int)size);
			iounmap(AssertLogVAddr);
			AssertLogVAddr = NULL;
			break;
		}
		/* send packet out to log (MTT via RNDIS or crash dump file) */
		BCMLOG_HandleCpCrashDumpData((const char *)(p + 2), size);

		packetCount++;
		iounmap(AssertLogVAddr);
		AssertLogVAddr = NULL;

#if 0
		/* **FIXME** this is Nucleus timeout code - do we want
		 * something similar for Android? Maybe if we get to the
		 * point of restarting CP with restarting AP
		 */
		if (TIMER_GetValue() - t0 > TICKS_ONE_SECOND * 10 * 60) {
			IPC_DEBUG(DBG_ERROR,
				  "Abort --- CP assertion log too long\n");
			break;
		}
#endif
	}

	RpcDbgDumpHistoryLogging(2, 1);

	IPC_DEBUG(DBG_ERROR, "Starting CP RAM dump - do not power down...\n");

	/* dump all CP memory to log */
	DUMP_CPMemoryByList(dumped_crash_summary_ptr->mem_dump);

	IPC_DEBUG(DBG_ERROR, "CP RAM dump complete\n");
	/* resume normal logging activities... */
	BCMLOG_EndCpCrashDump();

#ifdef CONFIG_FB_BRCM_CP_CRASH_DUMP_IMAGE_SUPPORT
	rhea_display_crash_image(CP_CRASH_DUMP_END);
#endif

	if (BCMLOG_OUTDEV_SDCARD == BCMLOG_GetCpCrashLogDevice())
		sys_sync();

	IPC_DEBUG(DBG_ERROR, "CP crash dump complete\n");

#ifdef CONFIG_BCM_AP_PANIC_ON_CPCRASH
		if ((BCMLOG_OUTDEV_SDCARD == BCMLOG_GetCpCrashLogDevice())
			&& cp_crashed == 1)
			abort();
#endif

}

/****************************************************************
**
*   Utility function to retrieve full CP RAM dump log for crash log
*
*
*******************************************************************/
void DUMP_CPMemoryByList(struct T_RAMDUMP_BLOCK *mem_dump)
{
	UInt32 i, offset;
	void __iomem *RamDumpBlockVAddr = NULL;
	struct T_RAMDUMP_BLOCK *pBlockVAddr = NULL;

	RamDumpBlockVAddr =
	    ioremap_nocache((UInt32) (mem_dump),
			    (MAX_RAMDUMP_BLOCKS *
			     sizeof(struct T_RAMDUMP_BLOCK)));
	if (NULL == RamDumpBlockVAddr) {
		IPC_DEBUG(DBG_ERROR, "failed to remap RAM dump block addr\n");
		return;
	}

	pBlockVAddr = (struct T_RAMDUMP_BLOCK *)RamDumpBlockVAddr;

	BCMLOG_LogCPCrashDumpString("===== COMMS PROCESSOR memory dump =====");

	i = 0;
	while (i < MAX_RAMDUMP_BLOCKS && pBlockVAddr[i].name[0] != '\0'
	       && pBlockVAddr[i].mem_size != 0) {
		if (pBlockVAddr[i].mem_start == SIM_DEBUG_DATA) {
			offset =
			    (pBlockVAddr[i].name[4] << 24) +
			    (pBlockVAddr[i].name[5] << 16) +
			    (pBlockVAddr[i].name[6] << 8) +
			    pBlockVAddr[i].name[7];
			snprintf(assert_buf, ASSERT_BUF_SIZE,
				 "FLASH DUMP: %8s, start=0x%08x, size=0x%08x, image_start=0x%08x, offset_in_image=0x%08x",
				 pBlockVAddr[i].name, pBlockVAddr[i].mem_start,
				 pBlockVAddr[i].mem_size, 0, (int)offset);
		} else if (pBlockVAddr[i].mem_start == SIM_AP_DEBUG_DATA) {
			offset =
			    (pBlockVAddr[i].name[4] << 24) +
			    (pBlockVAddr[i].name[5] << 16) +
			    (pBlockVAddr[i].name[6] << 8) +
			    pBlockVAddr[i].name[7];
			snprintf(assert_buf, ASSERT_BUF_SIZE,
				 "FLASH DUMP: %8s, start=0x%08x, size=0x%08x, image_start=0x%08x, offset_in_image=0x%08x",
				 pBlockVAddr[i].name, pBlockVAddr[i].mem_start,
				 pBlockVAddr[i].mem_size, 0, (int)offset);
		} else {
			snprintf(assert_buf, ASSERT_BUF_SIZE,
				 "RAM   DUMP: %8s, start=0x%08x, size=0x%08x, buffer_in_main=0x%08x",
				 pBlockVAddr[i].name,
				 pBlockVAddr[i].mem_start,
				 pBlockVAddr[i].mem_size,
				 pBlockVAddr[i].buffer_in_main);
		}
		BCMLOG_LogCPCrashDumpString(assert_buf);
		i++;
	}
	i = 0;
	while (i < MAX_RAMDUMP_BLOCKS && pBlockVAddr[i].name[0] != '\0'
	       && pBlockVAddr[i].mem_size != 0) {
		if (pBlockVAddr[i].mem_start == SIM_DEBUG_DATA) {
			offset =
			    (pBlockVAddr[i].name[4] << 24) +
			    (pBlockVAddr[i].name[5] << 16) +
			    (pBlockVAddr[i].name[6] << 8) +
			    pBlockVAddr[i].name[7];
			BCMLOG_LogCPCrashDumpString(pBlockVAddr[i].name);
			snprintf(assert_buf, ASSERT_BUF_SIZE,
				 "FLASH DUMP Begin: 0x%08x, 0x%08x",
				 pBlockVAddr[i].mem_start,
				 pBlockVAddr[i].mem_size);
			BCMLOG_LogCPCrashDumpString(assert_buf);
			/* **FIXME** MAG - flash dump not supported yet... */
			/* DUMP_CompressedFlash(cpu, pBlockVAddr[i].mem_start,
			   pBlockVAddr[i].mem_size, MSP_IMAGE_ADDR, offset); */
			BCMLOG_LogCPCrashDumpString
			    ("*** FLASH DUMP NOT SUPPORTED YET ***");
			snprintf(assert_buf, ASSERT_BUF_SIZE,
				 "FLASH DUMP End: 0x%08x, 0x%08x",
				 pBlockVAddr[i].mem_start,
				 pBlockVAddr[i].mem_size);
			BCMLOG_LogCPCrashDumpString(assert_buf);
		} else if (pBlockVAddr[i].mem_start == SIM_AP_DEBUG_DATA) {
			offset =
			    (pBlockVAddr[i].name[4] << 24) +
			    (pBlockVAddr[i].name[5] << 16) +
			    (pBlockVAddr[i].name[6] << 8) +
			    pBlockVAddr[i].name[7];
			BCMLOG_LogCPCrashDumpString(pBlockVAddr[i].name);
			snprintf(assert_buf, ASSERT_BUF_SIZE,
				 "FLASH DUMP Begin: 0x%08x, 0x%08x",
				 pBlockVAddr[i].mem_start,
				 pBlockVAddr[i].mem_size);
			BCMLOG_LogCPCrashDumpString(assert_buf);
			/* **FIXME** MAG - flash dump not supported yet... */
			BCMLOG_LogCPCrashDumpString
			    ("*** FLASH DUMP NOT SUPPORTED YET ***");
			/* DUMP_CompressedFlash(cpu, pBlockVAddr[i].mem_start,
			   pBlockVAddr[i].mem_size, AP_IMAGE_ADDR, offset); */
			snprintf(assert_buf, ASSERT_BUF_SIZE,
				 "FLASH DUMP End: 0x%08x, 0x%08x",
				 pBlockVAddr[i].mem_start,
				 pBlockVAddr[i].mem_size);
			BCMLOG_LogCPCrashDumpString(assert_buf);
		} else if (pBlockVAddr[i].buffer_in_main == 0xFFFFFFFF) {
			BCMLOG_LogCPCrashDumpString(pBlockVAddr[i].name);
			snprintf(assert_buf, ASSERT_BUF_SIZE,
				 "RAM DUMP Begin: 0x%08x, 0x%08x",
				 pBlockVAddr[i].mem_start,
				 pBlockVAddr[i].mem_size);
			BCMLOG_LogCPCrashDumpString(assert_buf);

			/* BCMLOG_HandleCpCrashMemDumpData takes
			   physical address... */
			BCMLOG_HandleCpCrashMemDumpData((const char *)
							pBlockVAddr[i].
							mem_start,
							pBlockVAddr[i].
							mem_size);

			snprintf(assert_buf, ASSERT_BUF_SIZE,
				 "RAM DUMP End: 0x%08x, 0x%08x",
				 pBlockVAddr[i].mem_start,
				 pBlockVAddr[i].mem_size);
			BCMLOG_LogCPCrashDumpString(assert_buf);
		}
		i++;
	}

	iounmap(RamDumpBlockVAddr);

}
