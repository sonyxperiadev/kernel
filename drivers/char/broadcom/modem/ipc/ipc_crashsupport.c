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
#include <linux/syscalls.h>
#include <linux/interrupt.h>
#include <linux/broadcom/ipcinterface.h>
#include <linux/firmware.h>
#include <linux/slab.h>
#include "ipc_sharedmemory.h"
#include "ipc_crashsupport.h"
#include <mach/comms/platform_mconfig.h>

#include "ipc_debug.h"
#include "ipc_stubs.h"
#include "bcmlog.h"
#include "ipc_crashsupport.h"
#ifdef CONFIG_FB_BRCM_CP_CRASH_DUMP_IMAGE_SUPPORT
#include <video/kona_fb_image_dump.h>
#endif

#include <mach/ns_ioremap.h>

#ifdef CONFIG_KONA_SECURE_MEMC
#include <linux/broadcom/secure_memc_shared.h>
#endif


/* CP crash recovery action */
#define   RECOVERY_ACTION_NONE                      0
#define   RECOVERY_ACTION_SYSRESET                  1
#define   RECOVERY_ACTION_SYSRESET_USERCONFIRM      2
#define   RECOVERY_ACTION_TBD                       3

struct vm_struct *ipc_cpmap_area;

#define get_vaddr(area)	(ipc_cpmap_area->addr + area)

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
#ifndef CONFIG_BCM_AP_PANIC_ON_CPCRASH
static char assert_buf[ASSERT_BUF_SIZE];
#endif
static struct T_CRASH_SUMMARY *dumped_crash_summary_ptr = { 0 };

static int crashCount;

/* maximum number of times we'll try to re-establish comms with
 * CP during crash log dump */
#define MAX_CP_DUMP_RETRIES 5
#define TICKS_ONE_SECOND 1024

struct T_CP_IMGS {
	const char *img_name;	/* CP image filename */
	int ram_addr;		/* RAM address */
	int img_size;		/* CP image size.
				 * 0 means the size is calculated.
				 */
};

/* main image signature and offset */
#define MSP_SIGNATURE_VALUE      0xBABEFACE
#define MSP_SIGNATURE_OFFSET     0x20

#define CP_IMAGE_SIZE_OFFSET     0x24
#define LINKER_FLAG_OFFSET_VALUE 0x4b4e494c
#define LINKER_FLAG_OFFSET_ADDR  (RESERVED_HEADER + 0x48)

/* from msp/cboot/bootldr/loader/capri/loader.h: */
#define CP_RUN_ADDR                 CP_RO_RAM_ADDR
#define CP_IMG_SIZE                 0	/* image size is calculated */

#define CP_BOOT_RUN_ADDR            MODEM_DTCM_ADDRESS
#define CP_BOOT_IMAGE_SIZE          0x00008000	/* CP_BOOT_SIZE */

static struct T_CP_IMGS g_cp_imgs[] = {
		/* name,	addr,	size */
	{ "sysparm_ind.img", PARM_IND_RAM_ADDR, PARM_IND_SIZE },
	{ "sysparm_dep.img", PARM_DEP_RAM_ADDR, PARM_DEP_SIZE },
	{ "parm_spml_ind.img", PARM_SPML_IND_RAM_ADDR, PARM_SPML_IND_SIZE },
	{ "parm_spml_dep.img", PARM_SPML_DEP_RAM_ADDR, PARM_SPML_DEP_SIZE },
	{ "umts_cal.img", UMTS_CAL_RAM_ADDR, UMTS_CAL_SIZE },
	{ "cp_image.img", CP_RUN_ADDR, CP_IMG_SIZE },
	{ "dsp_pram.img", DSP_PRAM_RAM_ADDR, DSP_PRAM_SIZE },
	{ "dsp_dram.img", DSP_DRAM_RAM_ADDR, DSP_DRAM_SIZE },
	{ "cp_boot.img", CP_BOOT_RUN_ADDR, CP_BOOT_IMAGE_SIZE },
	{ NULL, 0, 0 }
};

#ifndef CONFIG_BCM_AP_PANIC_ON_CPCRASH
/* internal helper functions */
static void DUMP_CP_assert_log(void);
static void DUMP_CPMemoryByList(struct T_RAMDUMP_BLOCK *mem_dump);
#endif
static void GetStringFromPA(UInt32 inPhysAddr, char *inStrBuf,
		UInt32 inStrBufLen);
static void ReloadCP(void);
static int DownloadFirmware(uint32_t len, const uint8_t *p_data, uint32_t addr);
static int32_t LoadFirmware(struct device *p_device, const char *p_name,
			int addr, int expectedSize);
static UInt32 IsCommsImageValid(const UInt8 *ram_addr);
static UInt32 CheckCommsImageSignature(UInt8 *p);

#ifdef CONFIG_HAS_WAKELOCK
extern struct wake_lock ipc_wake_lock;
#endif

static struct timer_list cp_reset_timer;
/* set CP reset timeout to 5 seconds for now */
#define CP_RESET_TIMEOUT_MILLISEC	5000

/* wait for 500ms, 20ms at a time, for CP to be ready to reset */
#define WAIT_FOR_CP_RESET_READY_MILLISEC	20
#define WAIT_FOR_CP_RESET_READY_ITERATIONS	25

/* grab virtual memory in 1M chunks */
#define VIRTUAL_MEM_CHUNK_SIZE	(1024*1024)

/* flag indicating that all cp reset clients have acked */
static int cp_reset_clients_acked;

extern int RpcDbgDumpHistoryLogging(int type, int level);
extern int cpStart(int isReset);
extern struct device *ipcs_get_drvdata(void);
extern int ipcs_reinitialize_ipc(void);

#ifdef CONFIG_FB_BRCM_CP_CRASH_DUMP_IMAGE_SUPPORT
int crash_dump_ui_on;
EXPORT_SYMBOL(crash_dump_ui_on);
#endif

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

	virtAddr = plat_ioremap_ns((unsigned long __force)
			get_vaddr(IPC_CP_STRING_MAP_AREA),
			IPC_CP_STRING_MAP_AREA_SZ,
			(phys_addr_t)inPhysAddr);
	if (!virtAddr) {
		IPC_DEBUG(DBG_ERROR,
			"ioremap failed in GetStringFromPA\n");
		return;
	}

	strncpy(inStrBuf, (char *)virtAddr, inStrBufLen);

	/* pad NULL in the end of the string */
	inStrBuf[inStrBufLen - 1] = '\0';
	plat_iounmap_ns(get_vaddr(IPC_CP_STRING_MAP_AREA),
			free_size_ipc(IPC_CP_STRING_MAP_AREA_SZ));
}

static IPCAP_CPResetHandler_T sCPResetHandler;

/* registers client callback to be used for passing silent CP reset events */
int IPCAP_RegisterCPResetHandler(IPCAP_CPResetHandler_T inResetHandler)
{
	/* **FIXME** need to support multiple clients, or just RPC? */
	sCPResetHandler = inResetHandler;
	IPC_DEBUG(DBG_INFO, "cp reset handler registered\n");
	return 1;
}

void HandleCPResetDone(void)
{
	IPC_DEBUG(DBG_INFO, "notifying cp reset IPC_CPRESET_COMPLETE\n");
	/* kick off notification to upper layer clients */
	if (sCPResetHandler)
		sCPResetHandler(IPC_CPRESET_COMPLETE);
	IPC_DEBUG(DBG_INFO, "DONE notifying cp reset IPC_CPRESET_COMPLETE\n");
}

void ResetCP(void)
{
	void __iomem *cp_root_reset_base;
	void __iomem *cp_bmdm_reset_base;

	IPC_DEBUG(DBG_INFO, "resetting CP\n");

	/* reset CP - copy from cp_reset.cmm rxd from CP team */
	/*;CP reset, from AP
	 *
	 * D.S ZSD:0x35001F00 %LE %LONG 0xa5a501
	 * D.S ZSD:0x35001F08 %LE %LONG 0x3bd	;reset
	 * D.S ZSD:0x35001F08 %LE %LONG 0x3fd	;clear
	 */
	cp_root_reset_base = ioremap(ROOT_RST_BASE_ADDR,
				ROOT_RST_MGR_REG_PD_SOFT_RSTN_OFFSET + 4);
	if (!cp_root_reset_base) {
		IPC_DEBUG(DBG_ERROR,
			"failed to remap ROOT_RST_BASE_ADDR, crashing\n");
		BUG();
	}

	cp_bmdm_reset_base = ioremap(BMDM_RST_BASE_ADDR,
				BMDM_RST_MGR_REG_CP_RSTN_OFFSET + 4);
	if (!cp_bmdm_reset_base) {
		IPC_DEBUG(DBG_ERROR,
			"failed to remap BMDM_RST_BASE_ADDR, crashing\n");
		BUG();
	}

	writel(0xa5a501,
	       cp_root_reset_base + ROOT_RST_MGR_REG_WR_ACCESS_OFFSET);
	udelay(1);
	writel(0x3bd,
	       cp_root_reset_base + ROOT_RST_MGR_REG_PD_SOFT_RSTN_OFFSET);
	udelay(1);
	writel(0x3fd,
	       cp_root_reset_base + ROOT_RST_MGR_REG_PD_SOFT_RSTN_OFFSET);
	udelay(1);

	/* reset R4 - copy from cp_reset.cmm rxd from CP team */
	/*;R4 reset
	 *
	 * D.S ZSD:0x3a055f00 %LE %LONG 0xa5a501
	 * D.S ZSD:0x3a055f18 %LE %LONG 0x2 ;reset
	 * D.S ZSD:0x3a055f18 %LE %LONG 0x3  ;clear
	 */
	writel(0xa5a501,
	       cp_bmdm_reset_base + BMDM_RST_MGR_REG_WR_ACCESS_OFFSET);
	udelay(1);
	writel(0x2, cp_bmdm_reset_base + BMDM_RST_MGR_REG_CP_RSTN_OFFSET);
	udelay(1);
	writel(0x3, cp_bmdm_reset_base + BMDM_RST_MGR_REG_CP_RSTN_OFFSET);
	udelay(1);

	iounmap(cp_root_reset_base);
	iounmap(cp_bmdm_reset_base);

}

int HandleRestartCP(void *data)
{
#ifdef CONFIG_KONA_SECURE_MEMC
	u32 *memc_handle;
#endif
	IPC_DEBUG(DBG_INFO, "enter\n");

	IPC_DEBUG(DBG_INFO, "call local_irq_disable()\n");
	local_irq_disable();

	IPC_DEBUG(DBG_INFO, "resetting CP\n");
	ResetCP();

	/* reload CP */
	IPC_DEBUG(DBG_INFO, "reloading CP\n");
	ReloadCP();

	IPC_DEBUG(DBG_INFO, "rebooting CP\n");
	/* reboot CP; this will also wipe IPC shared memory */
	cpStart(1);

	IPC_DEBUG(DBG_INFO, "call local_irq_enable()\n");
	local_irq_enable();
	enable_irq(IRQ_IPC_C2A);

	IPC_DEBUG(DBG_INFO, "re-init IPC\n");
	/* reinitialize IPC, and wait for IPC sync with CP */
	if (ipcs_reinitialize_ipc()) {
		IPC_DEBUG(DBG_ERROR, "ipcs_reinitialize_ipc failed\n");
		/* CP didn't re-sync, so crash AP here */
		BUG();
	}

	/* give CP some time to boot; without this delay, we hang
	 * on the CAPI2_PhoneCtrlApi_ProcessPowerUpReq() from RIL
	 */
	msleep(2000);

	/* notify clients that we're back in business... */
	IPC_DEBUG(DBG_INFO, "notifying clients CP reset is complete\n");
	HandleCPResetDone();
	IPC_DEBUG(DBG_INFO, "notification done, exiting reset thread\n");

#ifdef CONFIG_KONA_SECURE_MEMC
	memc_handle = get_secure_memc_handle();

	if (!memc_handle)
		pr_err("Failed to get secure memc handle\n");
	else if (do_revoke_region_access(memc_handle, AP))
		pr_err("Failed to revoke access for AP\n");
#endif

	/* done with thread */
	do_exit(0);
}

/* callback from client indicating it is ready for CP reset */
void IPCAP_ReadyForReset(int inClientID)
{
	IPC_DEBUG(DBG_INFO, "ready for reset\n");

	/* get rid of the ack timeout timer */
	del_timer(&cp_reset_timer);

	cp_reset_clients_acked = 1;
}

void CPReset_Timer_Callback(unsigned long data)
{
	/* not all IPC/RPC clients ackd the reset in time, so crash AP */
	IPC_DEBUG(DBG_INFO, "cp reset timeout %ld jiffies\n", jiffies);
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
	/* **FIXME** not required, according to CP team; CP will only
	 * report IPC_CP_SILENT_RESET_READY when it is ready for restart;
	 * there will be no IPC_CP_SILENT_RESET_START that we must ack
	 * with  IPC_AP_ACK_CP_RESET_START
	 */
	/* SmLocalControl.SmControl->CrashCode = IPC_AP_ACK_CP_RESET_START; */

	/* exit low power mode:
	 * ipc_wake_lock acquired on IPC interrupt that triggered this
	 * crash handler, and is not released until CP reset is complete
	 */

	IPC_DEBUG(DBG_INFO, "disabling IRQ_IPC_C2A\n");

	/* disable CP to AP interrupt */
	disable_irq(IRQ_IPC_C2A);

	/* set timeout timer for ack from IPC/RPC clients; if timer fires
	 * before all have ack'd, we should crash AP
	*/
	setup_timer(&cp_reset_timer, CPReset_Timer_Callback, 0);
	ret = mod_timer(&cp_reset_timer, jiffies +
		msecs_to_jiffies(CP_RESET_TIMEOUT_MILLISEC));
	if (ret)
		IPC_DEBUG(DBG_ERROR, "ERROR starting CP reset timer %d\n", ret);
	else
		IPC_DEBUG(DBG_INFO, "CP reset timeout timer set for %d ms\n",
			CP_RESET_TIMEOUT_MILLISEC);

	IPC_DEBUG(DBG_INFO, "start notifying cp reset IPC_CPRESET_START\n");

	/* kick off notification to upper layer clients */
	if (sCPResetHandler)
		sCPResetHandler(IPC_CPRESET_START);

	IPC_DEBUG(DBG_INFO, "DONE notifying cp reset IPC_CPRESET_START\n");
}

/*************************************************
*   Loads the image into RAM.
*
*	@param	len		(in) The size of the image.
*	@param	p_data		(in) Pointer to the image data.
*	@param	addr		(in) The RAM address to load the image into.
*
*	@return Returns 0 on success, non-zero otherwise.
*
*****************************************************/
static int DownloadFirmware(uint32_t len, const uint8_t *p_data, uint32_t addr)
{
	void __iomem *virtAddr;
	int ret = 0;
	int count;
	int chunk_size;

	IPC_DEBUG(DBG_INFO, "Downloading %d bytes from %p to address %p\n",
		len, (void *)p_data, (void *)addr);

	chunk_size = (VIRTUAL_MEM_CHUNK_SIZE > len) ? len
					: VIRTUAL_MEM_CHUNK_SIZE;
	for (count = 0; count < len; count += chunk_size) {
		if (count + chunk_size > len) {
			/* chunk size is too large for last segment */
			chunk_size = len - count;
			IPC_DEBUG(DBG_INFO,
				"*** chunk_size: truncated to %d\n",
				chunk_size);
		}
		virtAddr = ioremap_nocache(addr + count, chunk_size);
		if (NULL == virtAddr) {
			IPC_DEBUG(DBG_ERROR,
				"*** ERROR: ioremap_nocache failed\n");
			ret = -1;
			break;
		} else {
			IPC_DEBUG(DBG_INFO, "[%d] copying to virtual addr %p\n",
				count, (void *)virtAddr);
			memcpy(virtAddr, p_data + count, chunk_size);
			iounmap(virtAddr);
		}
	}

	return ret;
}

/*************************************************
*   Loads the image with the given name into RAM.
*
*	@param	p_device	(in) The kernel device.
*	@param	p_name		(in) The image name. This image file must be located
*							in /vendor/firmware.
*	@param	addr		(in) The RAM address to load the image into.
*	@param	expectedSize (in) The expected size of the image file, or 0
*                             if the size is to be calculated.
*
*****************************************************/
static int32_t LoadFirmware(struct device *p_device, const char *p_name,
				int addr, int expectedSize)
{
	const struct firmware *fw;
	int32_t err;
	int imgSize;

	IPC_DEBUG(DBG_INFO, "calling request_firmware for %s, device=%p\n",
		p_name, p_device);

	/** call kernel to start firmware load **/
	/* request_firmware(const struct firmware **fw,
	*                  const char *name,
	*                  struct device *device);
	*/
	err = request_firmware(&fw, p_name, p_device);
	if (err) {
		IPC_DEBUG(DBG_ERROR, "firmware request failed (%d)\n", err);
		return err;
	}

	if (fw)
		IPC_DEBUG(DBG_INFO, "fw->size=%d\n", fw->size);
	else {
		/*Coverity Complaint: FORWARD_NULL */
		IPC_DEBUG(DBG_INFO, "fw = NULL!\n");
		return err;
	}

	imgSize = fw->size;
	if (expectedSize == 0) {
		UInt8 *ptr;

		/* This is the main CP image */
		if (IsCommsImageValid(fw->data))
			IPC_DEBUG(DBG_INFO, "verified CP image\n");
		else
			IPC_DEBUG(DBG_ERROR, "failed to verify main image\n");

		ptr = ((UInt8 *) fw->data) + CP_IMAGE_SIZE_OFFSET;

		imgSize = (ptr[3] << 24) |
			(ptr[2] << 16) | (ptr[1] << 8) | ptr[0];
		IPC_DEBUG(DBG_INFO, "calculated CP image size = 0x%x\n",
			imgSize);
	} else if (expectedSize != imgSize) {
		if (imgSize > expectedSize) {
			IPC_DEBUG(DBG_ERROR,
					"ERROR: fw->size > expected (0x%x > 0x%x)\n",
					fw->size, expectedSize);
			imgSize = expectedSize;
		} else
			IPC_DEBUG(DBG_ERROR,
				"ERROR: fw->size < expected (0x%x < 0x%x)\n",
				fw->size, expectedSize);
	}

	/** download to chip **/
	err = DownloadFirmware(imgSize, fw->data, addr);

	/* Verify CP image @ RAM addr */
	if (expectedSize == 0) {
		void __iomem *virtAddr;

		/* map only the header of the CP image
			that's needed for validation */
		int chunk_size = LINKER_FLAG_OFFSET_ADDR + 1;

		virtAddr = ioremap_nocache(addr, chunk_size);
		if (virtAddr) {
			/* This is the main CP image */
			if (IsCommsImageValid(virtAddr))
				IPC_DEBUG(DBG_INFO,
					"verified CP image @ %p\n",
					(void *)addr);
			else
				IPC_DEBUG(DBG_ERROR,
					"failed to verify main image @ %p\n",
					(void *)addr);
			iounmap(virtAddr);
		} else {
			IPC_DEBUG(DBG_ERROR,
				  "*** ERROR: ioremap_nocache FAILED for addr %p\n",
				(void *)addr);
		}
	}

	/** free kernel structure */
	release_firmware(fw);

	return err;
}

/*************************************************
*   Reloads the CP images.
*
*****************************************************/
static void ReloadCP(void)
{
	int ret;
	int index;

	for (index = 0; (g_cp_imgs[index].img_name != NULL); index++) {
		IPC_DEBUG(DBG_INFO, "LoadFirmware for %s @ %p, size %d\n",
			g_cp_imgs[index].img_name,
			(void *)g_cp_imgs[index].ram_addr,
			g_cp_imgs[index].img_size);
		ret = LoadFirmware(ipcs_get_drvdata(),
				g_cp_imgs[index].img_name,
				g_cp_imgs[index].ram_addr,
				g_cp_imgs[index].img_size);
		IPC_DEBUG(DBG_INFO, "LoadFirmware for %s returned %d\n",
			g_cp_imgs[index].img_name, ret);
	}
}

/*************************************************
*   Verifies the comms image signature.
*
*	@param	ram_addr (in) The address to verify.
*****************************************************/
static UInt32 IsCommsImageValid(const UInt8 *ram_addr)
{
	UInt32 msp_signature;
	static UInt32 cp_loaded;
	UInt32 linkerFlagOffset;

	msp_signature = CheckCommsImageSignature(
		(UInt8 *)ram_addr + RESERVED_HEADER);
	cp_loaded = (msp_signature == MSP_SIGNATURE_VALUE);

	if (!cp_loaded) {
		IPC_DEBUG(DBG_ERROR, "bad MSP Signature: =0x%x [addr=%p]\r\n",
			(unsigned int)msp_signature, ram_addr);
	}

	/* also check the linker flag offset:
		RO_BASE + RESERVED_HEADER + 0x48
	*/
	linkerFlagOffset =
		*((UInt32 *)((UInt8 *)ram_addr + LINKER_FLAG_OFFSET_ADDR));
	if (linkerFlagOffset != LINKER_FLAG_OFFSET_VALUE) {
		IPC_DEBUG(DBG_ERROR,
			"bad linker flag offset value 0x%x [addr=%p]\r\n",
			(unsigned int)linkerFlagOffset, ram_addr);
		cp_loaded = 0;
	}

	return cp_loaded;
}

/*************************************************
*   Verifies the comms image signature.
*
*	@param	p (in) The address to verify.
*****************************************************/
static UInt32 CheckCommsImageSignature(UInt8 *p)
{
	UInt8 *ptr;
	UInt32 msp_signature = 0;

	ptr = p + MSP_SIGNATURE_OFFSET;
	msp_signature = (UInt32)(*ptr++);
	msp_signature += ((UInt32)(*ptr++)) << 8;
	msp_signature += ((UInt32)(*ptr++)) << 16;
	msp_signature += ((UInt32)(*ptr)) << 24;

	return msp_signature;
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
	int cpReset = SmLocalControl.SmControl->CrashCode ==
	    IPC_CP_SILENT_RESET_READY;
	RpcDbgDumpHistoryLogging(0, 0);
#ifdef CONFIG_KONA_SECURE_MEMC
	u32 *memc_handle;

	/* this is where any type of cp crash gets handled.
	 * there could be a scenerio of silent-reboot of cp.
	 * or there could be abrupt crash of cp.
	 * either ap would be alive or ap will crash itself.
	 * we protect cp area from following masters.
	 * > ap
	 * > mm
	 * > fabric
	 * in the event of ap/cp crash we need to give an ap
	 * access back to the cp area for the dump.
	 * and in the case of silent cp reset, ap will
	 * be alive, in that case we grant an access to ap.
	 * so that ap can load cp images and can take dump
	 * if required.
	 * and when cp comes up successfully and sync up with ap,
	 * we revoke the access for ap somwhere in HandleRestartCP.
	 */

	memc_handle = get_secure_memc_handle();
	if (!memc_handle)
		pr_err("Failed to get secure memc handle\n");
	if (do_grant_region_access(memc_handle, AP))
		pr_err("Failed to grant access for AP\n");
#endif

#ifdef CONFIG_FB_BRCM_CP_CRASH_DUMP_IMAGE_SUPPORT
if (!crash_dump_ui_on && !cpReset) {
	if (ramdump_enable)
		kona_display_crash_image(CP_RAM_DUMP_START);
	else
		kona_display_crash_image(CP_CRASH_DUMP_START);
	crash_dump_ui_on = 1;
	}
#endif

#ifdef CONFIG_CDEBUGGER
	if (ramdump_enable
#ifdef CONFIG_APANIC_ON_MMC
		&& ap_triggered == 0
#endif
		&& !cpReset
		) {
		/* we kill AP when CP crashes */
		IPC_DEBUG(DBG_ERROR, "Crashing AP for Ramdump ...\n\n");
		abort();
	}
#endif
	if ((BCMLOG_OUTDEV_PANIC == BCMLOG_GetCpCrashLogDevice() ||
		BCMLOG_OUTDEV_NONE == BCMLOG_GetCpCrashLogDevice() ||
		BCMLOG_OUTDEV_STM == BCMLOG_GetCpCrashLogDevice()) &&
			!cpReset
#ifdef CONFIG_APANIC_ON_MMC
		&& !ap_triggered
#endif
		) {
		/* we kill AP when CP crashes */
		IPC_DEBUG(DBG_ERROR, "Crashing AP now ...\n\n");
		abort();
	}

	IPC_Dump();


#if defined(CONFIG_BRCM_CP_CRASH_DUMP_EMMC) \
	|| defined(CONFIG_BCM_AP_PANIC_ON_CPCRASH)
	while (SmLocalControl.SmControl->CrashDump == NULL)
		; /* No op */
#endif

	/* **NOTE** for now, continue doing simple dump out IPC_DEBUG so there
	 * is some indication of CP crash in console
	 * (in case user not running MTT) */
	Dump = (void *)SmLocalControl.SmControl->CrashDump;

	IPC_DEBUG(DBG_ERROR, "ioremap\n");

	DumpVAddr = plat_ioremap_ns((unsigned long __force)
			get_vaddr(IPC_CP_CRASH_SUMMARY_AREA),
			IPC_CP_CRASH_SUMMARY_AREA_SZ, (phys_addr_t)Dump);

	if (!DumpVAddr) {
		IPC_DEBUG(DBG_ERROR,
			"ioremap failed in ProcessCPCrashedDump\n");
		goto cleanUp;
	}
	IPC_DEBUG(DBG_ERROR, "Crash Summary Virtual Addr: 0x%08X\n",
		  (unsigned int)DumpVAddr);

	dumped_crash_summary_ptr = (struct T_CRASH_SUMMARY *)DumpVAddr;

	IPC_DEBUG(DBG_ERROR, "===== COMMS_PROCESSOR crash summary =====\r\n");

	if (dumped_crash_summary_ptr->link_signature) {
		GetStringFromPA((UInt32)
				dumped_crash_summary_ptr->link_signature,
				outString, 128);
		IPC_DEBUG(DBG_ERROR, "%s\r\n", outString);
	}

	if (dumped_crash_summary_ptr->project_version) {
		GetStringFromPA((UInt32)
				dumped_crash_summary_ptr->project_version,
				outString, 128);
		IPC_DEBUG(DBG_ERROR, "%s\r\n", outString);
	}

	if (dumped_crash_summary_ptr->DSP_version) {
		GetStringFromPA((UInt32)dumped_crash_summary_ptr->DSP_version,
				outString, 128);
		IPC_DEBUG(DBG_ERROR, "%s\r\n", outString);
	}

	if (dumped_crash_summary_ptr->FW_version) {
		GetStringFromPA((UInt32)dumped_crash_summary_ptr->FW_version,
				outString, 128);
		IPC_DEBUG(DBG_ERROR, "%s\r\n", outString);
	}

	if (dumped_crash_summary_ptr->decoder_version) {
		GetStringFromPA((UInt32)
				dumped_crash_summary_ptr->decoder_version,
				outString, 128);
		IPC_DEBUG(DBG_ERROR, "%s\r\n", outString);
	}

	GetStringFromPA((UInt32)dumped_crash_summary_ptr->reason, crashReason,
			40);

	GetStringFromPA((UInt32)dumped_crash_summary_ptr->file, crashFile, 40);

	GetStringFromPA((UInt32)dumped_crash_summary_ptr->thread, crashThread,
			40);

	IPC_DEBUG(DBG_ERROR, "%s f=%s l=%d v=%d/0x%x t=%s TS=%d\r\n",
		  crashReason,
		  crashFile,
		  dumped_crash_summary_ptr->line,
		  dumped_crash_summary_ptr->value,
		  dumped_crash_summary_ptr->value,
		  crashThread, dumped_crash_summary_ptr->time);

	/* notify clients about CP reset */
	if (cpReset
#ifdef CONFIG_BRCM_CP_CRASH_DUMP_EMMC
	    && !ap_triggered
#endif
	    )
		HandleCPResetStart();

#ifndef CONFIG_BCM_AP_PANIC_ON_CPCRASH
	/* done with "simple" dump, so now pull the full assert
	 * log from CP and dump out to MTT */
	DUMP_CP_assert_log();
#endif

cleanUp:

	if (DumpVAddr)
		plat_iounmap_ns(get_vaddr(IPC_CP_CRASH_SUMMARY_AREA),
				free_size_ipc(IPC_CP_CRASH_SUMMARY_AREA_SZ));

	/* crash dump is done, so trigger CP reset */
	if (cpReset
#ifdef CONFIG_BRCM_CP_CRASH_DUMP_EMMC
	    && !ap_triggered
#endif
	    ) {
		IPC_DEBUG(DBG_INFO, "waiting for clients to ack...\n");
		while (!cp_reset_clients_acked)
			msleep(300);
		cp_reset_clients_acked = 0;

		IPC_DEBUG(DBG_INFO, "starting cp_reset thread\n");
		kthread_run(HandleRestartCP, 0, "cp_reset");
	}
#ifdef CONFIG_HAS_WAKELOCK
	else
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

	if (IPC_CP_NOT_CRASHED != CpCrashReason &&
	    IPC_CP_MAX_CRASH_CODE > CpCrashReason && NULL != Dump) {
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

#ifndef CONFIG_BCM_AP_PANIC_ON_CPCRASH
/******************************************************************
*   Utility function to retrieve full crash log from CP via simple
*   handshake protocol.
*
*
********************************************************************/
void DUMP_CP_assert_log(void)
{
	UInt32 t1, i, size, retryCount;
	UInt8 *p;
	UInt32 packetCount = 0;
	void __iomem *AssertLogVAddr = NULL;
	struct file *sdDumpFile = NULL;
	int cpReset = SmLocalControl.SmControl->CrashCode ==
	    IPC_CP_SILENT_RESET_READY;

	/* put logging driver into crash dump mode; messages will be sent
	 * straight out to MTT via RNDIS (or dump file) instead of buffering
	 * in RING buffer (flood of crash dump info overloads ring buffer
	 * otherwise,and we lose a lot of crash dump info)
	 * NOTE: crash dump is put into SD by default; if SD file fails to open,
	 * then we'll try sending it out RNDIS */

	BCMLOG_StartCpCrashDump(sdDumpFile, cpReset);

	/* only grab RAM dump for CP reset case */
	if (!cpReset) {
		retryCount = 0;
		while (1) {
			t1 = TIMER_GetValue();

			/* signal to CP that we're ready to rx crash log... */
			SmLocalControl.SmControl->CrashCode =
			    IPC_AP_CLEAR_TO_SEND;

			/* wait for CP to "dump"; CrashCode field will be
			 * set to physical address of current assert buf */
			while (SmLocalControl.SmControl->CrashCode ==
			       IPC_AP_CLEAR_TO_SEND) {
				for (i = 0; i < 256; i++)
					;
				if (TIMER_GetValue() - t1 >
				    TICKS_ONE_SECOND * 20)
					break;
			}

			/* check for time out */
			if (SmLocalControl.SmControl->CrashCode ==
			    IPC_AP_CLEAR_TO_SEND) {
				if (retryCount < MAX_CP_DUMP_RETRIES) {
					retryCount++;
					IPC_DEBUG(
						DBG_TRACE,
						"timeout %d, trying again..\n",
						(int)retryCount);
					continue;
				} else {
					/* no response from CP,
					 * so get out of here
					 */
					IPC_DEBUG(
						DBG_ERROR,
						"Abort --- max retries %d reached\n",
						(int)retryCount);
					break;
				}
			}

			/* reset retry counter */
			retryCount = 0;

			/* get virtual address of CP assert buffer */
			AssertLogVAddr = plat_ioremap_ns((unsigned long __force)
				get_vaddr(IPC_CP_ASSERT_BUF_AREA),
				IPC_CP_ASSERT_BUF_AREA_SZ,
				(phys_addr_t)
				SmLocalControl.SmControl->CrashCode);

			if (!AssertLogVAddr) {
				IPC_DEBUG(DBG_ERROR,
				  "ioremap failed in DUMP_CP_assert_log\n");
				break;
			}
			p = (UInt8 *)AssertLogVAddr;

			/* number of bytes in assert buffer */
			size = (p[0] << 8) + p[1];

			/* size of 0 means CP is done dumping assert log */
			if (size == 0) {
				IPC_DEBUG(DBG_ERROR,
					  "assert log size 0, exiting, packetCount:0x%x\n",
					  (int)packetCount);
				plat_iounmap_ns(get_vaddr
					(IPC_CP_ASSERT_BUF_AREA),
				free_size_ipc(IPC_CP_ASSERT_BUF_AREA_SZ));
				AssertLogVAddr = NULL;
				break;
			}
			/* sanity check for too beaucoup... */
			if (size > ASSERT_BUF_SIZE - 2) {
				IPC_DEBUG(DBG_ERROR,
					  "Abort --- improper size [%08x]=%d\n",
					  SmLocalControl.SmControl->CrashCode,
					  (int)size);
				plat_iounmap_ns(get_vaddr
					(IPC_CP_ASSERT_BUF_AREA),
				free_size_ipc(IPC_CP_ASSERT_BUF_AREA_SZ));
				AssertLogVAddr = NULL;
				break;
			}
			/* send packet out to log
			 * (MTT via RNDIS or crash dump file)
			 */
			BCMLOG_HandleCpCrashDumpData((const char *)(p + 2),
						     size);

			packetCount++;
			plat_iounmap_ns(get_vaddr(IPC_CP_ASSERT_BUF_AREA),
				free_size_ipc(IPC_CP_ASSERT_BUF_AREA_SZ));
			AssertLogVAddr = NULL;

		}
	}

	RpcDbgDumpHistoryLogging(2, 1);
#ifdef CONFIG_CDEBUGGER
	if (!ramdump_enable) {
#endif
		IPC_DEBUG(DBG_ERROR, "Starting CP RAM dump - do not power down...\n");

		/* dump all CP memory to log */
		DUMP_CPMemoryByList(dumped_crash_summary_ptr->mem_dump);

		IPC_DEBUG(DBG_ERROR, "CP RAM dump complete\n");
#ifdef CONFIG_CDEBUGGER
	}
#endif
	/* resume normal logging activities... */
	BCMLOG_EndCpCrashDump();

	if (BCMLOG_OUTDEV_SDCARD == BCMLOG_GetCpCrashLogDevice())
		sys_sync();

	IPC_DEBUG(DBG_ERROR, "CP crash dump complete\n");

#ifdef CONFIG_FB_BRCM_CP_CRASH_DUMP_IMAGE_SUPPORT
	if (
#ifdef CONFIG_CDEBUGGER
	!ramdump_enable &&
#endif
	!cpReset)
	kona_display_crash_image(GENERIC_DUMP_END);
#endif

	if ((BCMLOG_OUTDEV_RNDIS == BCMLOG_GetCpCrashLogDevice() ||
		BCMLOG_OUTDEV_ACM == BCMLOG_GetCpCrashLogDevice()
		|| BCMLOG_OUTDEV_SDCARD == BCMLOG_GetCpCrashLogDevice()
	   ) && cp_crashed == 1 && !cpReset)
		abort();

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

	RamDumpBlockVAddr = plat_ioremap_ns((unsigned long __force)
		get_vaddr(IPC_CP_RAMDUMP_BLOCK_AREA),
		IPC_CP_RAMDUMP_BLOCK_AREA_SZ,
		(phys_addr_t)mem_dump);

	if (!RamDumpBlockVAddr) {
		IPC_DEBUG(DBG_ERROR, "failed to remap RAM dump block addr\n");
		return;
	}

	pBlockVAddr = (struct T_RAMDUMP_BLOCK *)RamDumpBlockVAddr;

	BCMLOG_LogCPCrashDumpString("===== COMMS PROCESSOR memory dump =====");

	i = 0;
	while (i < MAX_RAMDUMP_BLOCKS && pBlockVAddr[i].name[0] != '\0'
	       && pBlockVAddr[i].mem_size != 0) {
		if (pBlockVAddr[i].mem_start == SIM_DEBUG_DATA) {
			offset = (pBlockVAddr[i].name[4] << 24) +
				(pBlockVAddr[i].name[5] << 16) +
				(pBlockVAddr[i].name[6] << 8) +
				pBlockVAddr[i].name[7];
			snprintf(assert_buf,
				ASSERT_BUF_SIZE,
				"FLASH DUMP: %8s, start=0x%08x, size=0x%08x, image_start=0x%08x, offset_in_image=0x%08x",
				pBlockVAddr[i].name,
				pBlockVAddr[i].mem_start,
				pBlockVAddr[i].mem_size,
				0,	/* MSP_IMAGE_ADDR, */
				(int)offset);
		} else if (pBlockVAddr[i].mem_start == SIM_AP_DEBUG_DATA) {
			offset = (pBlockVAddr[i].name[4] << 24) +
				(pBlockVAddr[i].name[5] << 16) +
				(pBlockVAddr[i].name[6] << 8) +
				pBlockVAddr[i].name[7];
			snprintf(assert_buf,
				ASSERT_BUF_SIZE,
				"FLASH DUMP: %8s, start=0x%08x, size=0x%08x, image_start=0x%08x, offset_in_image=0x%08x",
				pBlockVAddr[i].name,
				pBlockVAddr[i].mem_start,
				pBlockVAddr[i].mem_size,
				0,	/* AP_IMAGE_ADDR, */
				(int)offset);
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
			offset = (pBlockVAddr[i].name[4] << 24) +
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
			offset = (pBlockVAddr[i].name[4] << 24) +
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

	plat_iounmap_ns(get_vaddr(IPC_CP_RAMDUMP_BLOCK_AREA),
		free_size_ipc(IPC_CP_RAMDUMP_BLOCK_AREA_SZ));

}
#endif

int __init ipc_crashsupport_init(void)
{
	ipc_cpmap_area = plat_get_vm_area(IPC_CPMAP_NUM_PAGES);

	if (!ipc_cpmap_area) {
		pr_err("Failed to allocate vm area\n");
		return -ENOMEM;
	}

	pr_info("ipc:vm area:start:0x%lx, size:%lx\n",
			(unsigned long __force)ipc_cpmap_area->addr,
			ipc_cpmap_area->size);

	return 0;
}
