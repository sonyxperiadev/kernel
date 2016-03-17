/****************************************************************************
*
*     Copyright (c) 2009 Broadcom Corporation
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
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/io.h>
#include <linux/workqueue.h>
#include <linux/console.h>
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
#include <linux/slab.h>
#include <linux/poll.h>
#include <linux/broadcom/bcm_major.h>
#include <linux/time.h>
#include <linux/rtc.h>
#include <linux/hardirq.h>
#include <linux/vmalloc.h>
#include <linux/bootmem.h>
#ifdef CONFIG_BRCM_CP_CRASH_DUMP_EMMC
#include <linux/mmc-poll/mmc_poll_stack.h>
#include <linux/mmc-poll/mmc_poll.h>
#endif
/*#include <linux/broadcom/ipc_server_ifc.h>
*/
#include <linux/jiffies.h>
#ifdef CONFIG_HAS_WAKELOCK
#include <linux/wakelock.h>
#endif
#include <linux/delay.h>

#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/vmalloc.h>
#include <linux/mtd/mtd.h>
#include <linux/broadcom/ipcinterface.h>
#include <mach/ns_ioremap.h>

#include "fifo.h"
#include "bcmmtt.h"
#include "bcmlog.h"
#include "output.h"
#include "config.h"

/**
 *	Console message logging levels -- can be or'ed together
 **/
/* output info messages to console */
#define BCMLOG_CONSOLE_MSG_INFO		1
/* output error messages to console */
#define BCMLOG_CONSOLE_MSG_ERROR	2
/* output debug messages to console */
#define BCMLOG_CONSOLE_MSG_DEBUG	4

/**
 *	Console message logging macro
 **/
#define BCMLOG_PRINTF(level, fmt, args...)
/**
 *	if ((level) & g_module.console_msg_lvl)
 *		printk( "%s:: " fmt, __FUNCTION__, ##args )
 **/

/**
 *  Definitions used for packaging up CP crash dump memory blocks
 **/
#define	LE_COMPRESS_FLAG		0x5C
#define	LE_UNCOMPRESS_FLAG		0xFF
#define	COMPRESS_TOKEN			0xC5
#define	P_log_general			250
#define MEMORY_DUMP             ((P_log_general<<16)|1)

#define	WORDS_PER_SIGNAL	0x400
/* file that CP crash dump log will be written to */
#define CP_CRASH_DUMP_BASE_FILE_NAME    "cp_crash_dump_"
#define CP_CRASH_DUMP_FILE_EXT          ".bin"
#define CP_CRASH_DUMP_MAX_LEN           100
#define CP_DUMP_LEN			512
#define CP_CRASH_DUMP_SDCARD_SIZE	(20*1024*1024)
#define KPANIC_CP_DUMP_OFFSET		0x100000

#define BCMLOG_OUTPUT_FIFO_MAX_BYTES  (64 * 1024 * 1024)

struct panic_header {
	u32 magic;
#define PANIC_MAGIC 0xdeadf00d

	u32 version;
#define PHDR_VERSION   0x01

	u32 console_offset;
	u32 console_length;

	u32 threads_offset;
	u32 threads_length;
};

struct cpanic_data {
	int dev_num;
	int mmc_poll_dev_num;
	char dev_path[256];
	struct mmc *mmc;
	struct panic_header curr;
	void *bounce;
	struct proc_dir_entry *apanic_trigger;
	struct proc_dir_entry *apanic_console;
	struct proc_dir_entry *apanic_threads;
};

static void BCMLOG_klogging_crashdump(const char *buf, int size)
{
#ifdef CONFIG_BRCM_NETCONSOLE
	brcm_klogging((char *)buf, size) ;
#endif
	mdelay(10);
}

struct vm_struct *bcmlog_cpmap_area;
#define get_vaddr_ipc(area) (ipc_cpmap_area->addr + area)
#define get_vaddr(area) (bcmlog_cpmap_area->addr + area)

#ifdef CONFIG_BRCM_CP_CRASH_DUMP_EMMC
static unsigned long offs;
#endif

static unsigned int compress_memcpy(char *dest, char *src,
				    unsigned short nbytes);
static unsigned short Checksum16(unsigned char *data, unsigned long len);
static void BCMLOG_OutputLinkList(unsigned long ListSize,
				  struct BCMLOG_LogLinkList_t *LinkList);
static unsigned char *sMemDumpSignalBuf;
mm_segment_t sCrashDumpFS;

static unsigned char *cp_buf;

/**
 *  Definitions used for redirect printk using cbks
 **/
static BrcmRedirectPrintkCbk sPrintkCbk;
static atomic_t redirectEnabled = ATOMIC_INIT(0);

/**
 *  Definitions used for logging binary signals
 **/
#define HWTRC_B_SIGNAL_HEADER_SIZE  (2 + 12 * 2 + 2)
#define	LOG_COMPRESS_THRESHOLD		64

enum LogSignalCompress_t {
	LOGSIGNAL_NOCOMPRESS = 0,
	LOGSIGNAL_COMPRESS = 1
};

/*
 * prototypes for LogSignal_Internal and LogString_Internal
 *
 * Note:
 *	The Output() function handles sending messages (signals and strings) to
 *	MTT. If Output() is unable to synchronousely send a message to MTT
 *	(for example, RNDIS is not ready) the message will be placed in a FIFO
 *	for later handling.
 *
 *	To avoid unnecessary copying, signals that already have MTT overhead
 *	(e.g., timestamp, checksum) may be copied directly to the FIFO. This is
 *	the reason for the inMemFree argument to LogSignal_Internal();
 *
 *	Unlike signals, strings are always copied to a new MTT message buffer,
 *	so there is never a reason to 'keep' the original string.
 *	For this reason no inMemFree argument is passed to LogString_Internal()
 */

static void LogSignal_Internal(unsigned int inSigCode,
			       void *inSigBuf,
			       unsigned int inSigBufSize,
			       unsigned short inState,
			       unsigned short inSender,
			       enum LogSignalCompress_t inCompress);

static void LogString_Internal(const char *inLogString,
			       unsigned short inSender);

/**
 *	symbol to be defined for module debugging only
 **/

/**
 *	#define BCMLOG_CONSOLE_MSG_LVL BCMLOG_CONSOLE_MSG_ERROR |
 *		BCMLOG_CONSOLE_MSG_INFO |
 *		BCMLOG_CONSOLE_MSG_DEBUG
 **/

/**
 *	set console logging level
 **/
#ifndef BCMLOG_CONSOLE_MSG_LVL
#define BCMLOG_CONSOLE_MSG_LVL BCMLOG_CONSOLE_MSG_ERROR
#endif

/**
 *	module status
 **/
struct BCMLOG_Module_t {
	/* console message level */
	int console_msg_lvl;
	/* driver class  */
	struct class *logdrv_class;
	/* locks output stream to ensure sequential message output */
	spinlock_t output_lock;
	/* 1==dumping CP crash log, 0 otherwise */
	int dumping_cp_crash_log;
	/* log buffer */
	void *buffer;
	/* size of log buffer in bytes */
	int buffer_size;
	/* fifo handle */
	BCMLOG_Fifo_t *fifo;
};

/**
 *	user file status (for open, close and ioctl calls)
 **/
struct BCMLOG_File_t {
	struct file *file;	/* user file handle */
};

/**
 * pointer to file descriptor for file used to dump CP crash data
 **/
static struct file *sDumpFile;

/*
 *	forward declarations used in 'struct file_operations'
 */
static int BCMLOG_Open(struct inode *inode, struct file *filp);
static long BCMLOG_Ioctl(struct file *filp, unsigned int cmd,
			 unsigned long arg);
static int BCMLOG_Release(struct inode *inode, struct file *filp);

/*
 *	module status
 */
static struct BCMLOG_Module_t g_module = {
	.console_msg_lvl = 0,
	.logdrv_class = NULL,
	.dumping_cp_crash_log = 0,
	.buffer = NULL,
	.buffer_size = KMALLOC_MAX_SIZE,
	.fifo = NULL,
};

/*
 *	file ops
 */
static const struct file_operations g_file_operations = {
	.owner = THIS_MODULE,
	.open = BCMLOG_Open,
	.read = NULL,
	.write = NULL,
	.unlocked_ioctl = BCMLOG_Ioctl,
	.poll = NULL,
	.mmap = NULL,
	.release = BCMLOG_Release,
};

/**
 *	Check for CP crash dump in progres
 *	@return	non-zero if CP crash dump in progress, else zero
 **/
int CpCrashDumpInProgress(void)
{
	return (g_module.dumping_cp_crash_log) ? 1 : 0;
}

/**
 *	Acquire lock on output stream
 *	@return irql value to be passed to ReleaseOutputLock()
 **/
unsigned long AcquireOutputLock(void)
{
	unsigned long irql;
	spin_lock_irqsave(&g_module.output_lock, irql);
	return irql;
}

/**
 *	Release lock on output stream
 *	@param irql (in) value returned by AcquireOutputLock()
 **/
void ReleaseOutputLock(unsigned long irql)
{
	spin_unlock_irqrestore(&g_module.output_lock, irql);
}

/**
 *	Get the current log buffer size
 *	@return	size of buffer in bytes
 **/
int BCMLOG_GetBufferSize(void)
{
	return g_module.buffer_size;
}

/**
 *	Get the current log buffer size.  All stored logs will be lost.
 *	@return	size of buffer in bytes
 **/
void BCMLOG_SetBufferSize(int buffer_size)
{
	unsigned long lock;

	/* check range */
	if ((buffer_size <= 0) ||
			(buffer_size > BCMLOG_OUTPUT_FIFO_MAX_BYTES)) {
		BCMLOG_PRINTF(BCMLOG_CONSOLE_MSG_ERROR,
			"BCM Log Invalid buffer size:%x\n", buffer_size);
		return;
	}
	if (buffer_size == g_module.buffer_size) {
		BCMLOG_PRINTF(BCMLOG_CONSOLE_MSG_ERROR,
			"BCM Log no change in buffer size:%x\n", buffer_size);
		return;
	}

	lock = AcquireOutputLock();
	/* stop current logging and create new buffer */
	if (g_module.buffer_size <= KMALLOC_MAX_SIZE)
		kfree(g_module.buffer);
	else
		vfree(g_module.buffer);

	if (buffer_size <= KMALLOC_MAX_SIZE)
		g_module.buffer = kmalloc(buffer_size, GFP_KERNEL);
	else
		g_module.buffer = vmalloc(buffer_size);

	/* Failed allocation will default to default size */
	if (!g_module.buffer) {
		BCMLOG_PRINTF(BCMLOG_CONSOLE_MSG_ERROR,
			"BCM Log Failed to allocate new log buffer of size:%x\n",
			buffer_size);
		if (g_module.buffer_size > KMALLOC_MAX_SIZE) {
			g_module.buffer = kmalloc(
				KMALLOC_MAX_SIZE,
				GFP_KERNEL);
			g_module.buffer_size = KMALLOC_MAX_SIZE;
		}

		if (!g_module.buffer) {
			BCMLOG_PRINTF(BCMLOG_CONSOLE_MSG_ERROR,
				"BCM Log Failed to allocate default log buffer of size:%x\n",
				KMALLOC_MAX_SIZE);
		}
	} else
		g_module.buffer_size = buffer_size;

	BCMLOG_FifoInit(g_module.fifo, g_module.buffer, g_module.buffer_size);

	ReleaseOutputLock(lock);
	return ;
}

/**
 *	Called by Linux I/O system to handle open() call.
 *	@param	(in)	not used
 *	@param	(io)	file pointer
 *	@return	int		0 if success, -1 if error
 *	@note
 *		API is defined by struct file_operations 'open' member.
 **/
static int BCMLOG_Open(struct inode *inode, struct file *filp)
{
	struct BCMLOG_File_t *priv = kmalloc(sizeof(*priv), GFP_KERNEL);

	if (!priv) {
		BCMLOG_PRINTF(BCMLOG_CONSOLE_MSG_ERROR, "ENOMEM\n");
		return -ENOMEM;
	}

	priv->file = filp;
	filp->private_data = priv;

	return 0;
}

/**
 *	Called by Linux I/O system to handle ioctl() call.
 *	@param	(in)	not used
 *	@param	(in)	not used
 *	@param	(in)	ioctl command (see note)
 *	@param	(in)	ioctl argument (see note)
 *	@return	int		0 if success, -1 if error
 *	@note
 *		API is defined by struct file_operations 'ioctl' member.
 *
 *		cmd is one following:
 *
 *			BCMLOG_IOC_LOGSTR	- log a string
 *			BCMLOG_IOC_ENABLE	- get/set local logging enable
 *
 *		arg depends on cmd:
 *
 *			BCMLOG_IOC_LOGSTR
 *			- arg is a pointer to type BCMLOG_IoctlLogStr_t,
 *			  which specifies the string buffer and length
 *
 *			BCMLOG_IOC_ENABLE
 *			- arg is a pointer to type BCMLOG_IoctlEnable_t
 *			  which controls get/set of enable/disable status
 *			  for a particular log ID
 **/
static long BCMLOG_Ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int rc;

	switch (cmd) {
		/**
		 *	log a null-terminated string from user space
		 **/
	case BCMLOG_IOC_LOGSTR:
		{
			struct BCMLOG_IoctlLogStr_t *lcl =
			    (struct BCMLOG_IoctlLogStr_t *)arg;

			if (!CpCrashDumpInProgress()) {
				char *kbuf_str = 0;

				if (!lcl || !lcl->str || !lcl->size) {
					rc = -1;
					break;
				}

				if (!BCMLOG_LogIdIsEnabled(lcl->sender)) {
					rc = -1;
					break;
				}

				kbuf_str = kmalloc(lcl->size + 1, GFP_KERNEL);

				if (!kbuf_str) {
					BCMLOG_PRINTF(BCMLOG_CONSOLE_MSG_ERROR,
						      "allocation error\n");
					rc = -1;
					break;
				}

				if (copy_from_user
				    (kbuf_str, lcl->str, lcl->size)) {
					BCMLOG_PRINTF(BCMLOG_CONSOLE_MSG_ERROR,
						      "bad userspace pointer\n");
					rc = -1;
					kfree(kbuf_str);
					break;
				}
				/* this ensures the string is null terminated */
				kbuf_str[lcl->size] = 0;

				LogString_Internal(kbuf_str, lcl->sender);

				kfree(kbuf_str);
			}

			rc = 0;

			break;
		}

	case BCMLOG_IOC_ENABLE:
		{
			struct BCMLOG_IoctlEnable_t *lcl =
			    (struct BCMLOG_IoctlEnable_t *)arg;

			if (!lcl || lcl->id >= BCMLOG_MAX_LOG_ID) {
				rc = -1;
				break;
			}

			if (lcl->set)
				BCMLOG_EnableLogId(lcl->id, lcl->enable);

			else
				lcl->enable = BCMLOG_LogIdIsEnabled(lcl->id);

			rc = 0;
			break;

		}

	case BCMLOG_IOC_LOGSIGNAL:
		{
			/* request to log binary signal from user space */
			struct BCMLOG_IoctlLogSignal_t *lcl =
			    (struct BCMLOG_IoctlLogSignal_t *)arg;

			if (!CpCrashDumpInProgress()) {

				/* sanity check... */
				if (!lcl) {
					rc = -1;
					break;
				}

				if (!BCMLOG_LogIdIsEnabled(lcl->sender)) {
					rc = -1;
					break;
				}

				if (lcl->sigPtr && (lcl->sigBufSize > 0)) {
					/**
					 * allocate buffer for kernel space
					 * signal
					 **/
					void *kernelSigBuf =
					    kmalloc(lcl->sigBufSize,
						    GFP_KERNEL);

					if (!kernelSigBuf) {
						BCMLOG_PRINTF
						    (BCMLOG_CONSOLE_MSG_ERROR,
						     "allocation error\n");
						return -1;
					}
					/**
					 * copy the signal from user space to
					 * kernel space
					 **/
					if (copy_from_user(kernelSigBuf,
							   lcl->sigPtr,
							   lcl->sigBufSize)) {
						rc = -1;
						kfree(kernelSigBuf);
						break;
					}

					/* internal api for signal logging */
					LogSignal_Internal(lcl->sigCode,
							   kernelSigBuf,
							   lcl->sigBufSize,
							   lcl->state,
							   lcl->sender,
							   LOGSIGNAL_COMPRESS);

					kfree(kernelSigBuf);
				}
			}

			rc = 0;

			break;
		}

	default:
		rc = -1;
		break;
	}

	return rc;
}

/**
 *	Called by Linux I/O system to handle release() call.
 *	@param	(in)	not used
 *	@param	(in)	not used
 *	@return	int		0 if success, -1 if error
 *	@note
 *		API is defined by struct file_operations 'release' member.
 **/
static int BCMLOG_Release(struct inode *inode, struct file *filp)
{
	struct BCMLOG_File_t *priv = filp->private_data;

	kfree(priv);

	return 0;
}

int __init bcmlog_crashsupport_init(void)
{
	bcmlog_cpmap_area = plat_get_vm_area(BCMLOG_IOREMAP_NUM_PAGES);

	if (!bcmlog_cpmap_area) {
		pr_err("bcmlog_cpmap_area:Failed to allocate vm area\n");
		return -ENOMEM;
	}

	pr_info("bcmlog:vm area:start:0x%lx, size:%lx\n",
			(unsigned long __force)bcmlog_cpmap_area->addr,
			bcmlog_cpmap_area->size);
	return 0;
}

static int __init setup_mtt_logbuffer(char *str)
{
	if (str)
		get_option(&str, &g_module.buffer_size);
	if ((g_module.buffer_size <= 0)
		|| (g_module.buffer_size > BCMLOG_OUTPUT_FIFO_MAX_BYTES))
		g_module.buffer_size = KMALLOC_MAX_SIZE;
	pr_info("Allocating mtt log buffer size : %d Bytes\n",\
	g_module.buffer_size);
	g_module.buffer = alloc_bootmem_pages(g_module.buffer_size);
	if (!g_module.buffer) {
		pr_err("Failed to allocate mtt log buffer\n");
		return -1;
	}	else {
		pr_info("[allocated mtt log buffer] base:0x%x,size:%d,bytes\n",\
		(unsigned int)g_module.buffer, g_module.buffer_size);
	}
	return 0;
}

__setup("mtt_log_buffer=", setup_mtt_logbuffer);

/**
 *	Called by Linux I/O system to initialize module.
 *	@return	int		0 if success, -1 if error
 *	@note
 *		API is defined by module_init macro
 **/
static int __init BCMLOG_ModuleInit(void)
{
	struct device *drvdata;
	int ret = 0;

	cp_buf = NULL;
	sMemDumpSignalBuf = NULL;
	sDumpFile = NULL;
	g_module.console_msg_lvl = BCMLOG_CONSOLE_MSG_LVL;
	g_module.logdrv_class = NULL;

	pr_info("[%s]g_module.buffer=0x%x, g_module.buffer_size=%d\n",\
	__func__, (unsigned int)g_module.buffer, g_module.buffer_size);

	if (!g_module.buffer) {
		if (g_module.buffer_size <= 0 ||
			g_module.buffer_size > KMALLOC_MAX_SIZE)
			g_module.buffer_size = KMALLOC_MAX_SIZE;
		pr_info("[%s] mtt log buffer size %d\n", __func__,
			g_module.buffer_size);
		g_module.buffer = kmalloc(g_module.buffer_size, GFP_KERNEL);
	}
	if (!g_module.buffer) {
		BCMLOG_PRINTF(BCMLOG_CONSOLE_MSG_ERROR,
			"BCM Log Failed to allocate buffer\n");
		return -1;
	}

	if (bcmlog_crashsupport_init())
		return -1;

	g_module.fifo = BCMLOG_OutputInit((unsigned char *)g_module.buffer,
		g_module.buffer_size);

	spin_lock_init(&g_module.output_lock);

	ret = register_chrdev(BCM_LOG_MAJOR, BCMLOG_MODULE_NAME,
			      &g_file_operations);

	if (ret < 0) {
		BCMLOG_PRINTF(BCMLOG_CONSOLE_MSG_ERROR,
			      "register_chrdev failed\n");
		return -1;
	}

	g_module.logdrv_class = class_create(THIS_MODULE, BCMLOG_MODULE_NAME);

	if (IS_ERR(g_module.logdrv_class)) {
		BCMLOG_PRINTF(BCMLOG_CONSOLE_MSG_ERROR,
			      "class_create failed\n");
		unregister_chrdev(BCM_LOG_MAJOR, BCMLOG_MODULE_NAME);
		return -1;
	}

	drvdata =
	    device_create(g_module.logdrv_class, NULL, MKDEV(BCM_LOG_MAJOR, 0),
			  NULL, BCMLOG_MODULE_NAME);

	if (IS_ERR(drvdata)) {
		BCMLOG_PRINTF(BCMLOG_CONSOLE_MSG_ERROR,
			      "device_create failed\n");
		unregister_chrdev(BCM_LOG_MAJOR, BCMLOG_MODULE_NAME);
		return -1;
	}

	BCMLOG_InitConfig(drvdata);

/* #ifdef CONFIG_BRCM_UNIFIED_LOGGING */
	{
		bcmlog_log_ulogging_id = BCMLOG_LOG_ULOGGING_ID;
		BCMLOG_EnableLogId(bcmlog_log_ulogging_id, 1);

		bcmlog_mtt_on = 1;

		ret = brcm_retrive_early_printk();
		if (ret < 0)
			printk(KERN_INFO
			       "\n Printk->Mtt: Couldn't get early printk\n");
	}
/* #endif */

	BrcmLogString = BCMLOG_LogString;
	return ret;
}

/**
 *	Called by Linux I/O system to exit module.
 *	@return	int		0 if success, -1 if error
 *	@note
 *		API is defined by module_exit macro
 **/
static void __exit BCMLOG_ModuleExit(void)
{
}

/**
 *	Log null terminated string.
 *
 *	@param	inLogString		(in)	null terminated string to be logged
 *	@param	inSender		(in)	ID of sending task
 *
**/
static void LogString_Internal(const char *inLogString, unsigned short inSender)
{
	int logStrSize;
	int mttFrameSize;
	char *kbuf_mtt = 0;
	unsigned long irql;

	/* include the NULL termination... */
	logStrSize = strlen(inLogString) + 1;

	/* mtt adds framing bytes; get total size */
	mttFrameSize = BCMMTT_GetRequiredFrameLength(logStrSize);

	if (mttFrameSize <= 0) {
		BCMLOG_PRINTF(BCMLOG_CONSOLE_MSG_ERROR,
			      "invalid string length\n");
		return;
	}

	kbuf_mtt = kmalloc(mttFrameSize, GFP_ATOMIC | __GFP_NOWARN);

	if (!kbuf_mtt) {
		BCMLOG_PRINTF(BCMLOG_CONSOLE_MSG_ERROR, "allocation error\n");
		return;
	}

	mttFrameSize = BCMMTT_FrameString(kbuf_mtt, inLogString, mttFrameSize);

	irql = AcquireOutputLock();
	/**
	 * we are using a copy of string to be logged,
	 * always free the buffer when done
	 **/

	BCMLOG_Output(kbuf_mtt, mttFrameSize, 1);

	ReleaseOutputLock(irql);

	kfree(kbuf_mtt);
}

/**
 *	Log null terminated string.
 *
 *	@param	inLogString	(in)	null terminated string to be logged
 *	@param	inSender	(in)	ID of sending task
 *
**/
void BCMLOG_LogString(const char *inLogString, unsigned short inSender)
{
	/**
	 * ignore logging requests if we're doing a CP
	 * crash dump
	 **/

	char temp_LogString[BCMLOG_MAX_ASCII_STRING_LENGTH];
	int StringLength, j = 0, k = 0;

	/* Redirect the printk string if reqested by client */
	if (inSender == BCMLOG_LOG_ULOGGING_ID &&
	    atomic_read(&redirectEnabled) == 1) {
		unsigned long irql;

		irql = AcquireOutputLock();
		if (sPrintkCbk)
			sPrintkCbk(inLogString);
		ReleaseOutputLock(irql);
	}

	if (!CpCrashDumpInProgress() && BCMLOG_LogIdIsEnabled(inSender)) {

		StringLength = strlen(inLogString);
		memset(temp_LogString, 0, BCMLOG_MAX_ASCII_STRING_LENGTH);

		if (StringLength >= BCMLOG_MAX_ASCII_STRING_LENGTH) {
			for (j = 0; j < StringLength; j++) {
				if (inLogString[j] == '\n') {
					LogString_Internal(temp_LogString,
							   inSender);
					k = 0;
					memset(temp_LogString, 0,
					       BCMLOG_MAX_ASCII_STRING_LENGTH);
				}
				temp_LogString[k++] = inLogString[j];
				if (k == BCMLOG_MAX_ASCII_STRING_LENGTH - 1) {
					temp_LogString[k] = 0;
					LogString_Internal(temp_LogString,
							   inSender);
					k = 0;
					memset(temp_LogString, 0,
					       BCMLOG_MAX_ASCII_STRING_LENGTH);
				}
			}

			if (k != 0) {
				temp_LogString[k] = 0;
				LogString_Internal(temp_LogString, inSender);
			}
		} else {
			LogString_Internal(inLogString, inSender);
		}
	}
}

/**
	Log a formatted list of arguments
	@param		inSender (in) ID of the source module
	@param		*fmt (in) a format string as printf()
	...		(in) a list of arguments as printf()
	@note		if the Broadcom Unified Logging option is not
			enabled then printk is called
**/
/* #ifdef CONFIG_BRCM_UNIFIED_LOGGING */

void BCMLOG_Printf(unsigned short inSender, char *fmt, ...)
{
	va_list ap;
	char *tmpBuf;

	if (!CpCrashDumpInProgress() && BCMLOG_LogIdIsEnabled(inSender)) {
		va_start(ap, fmt);
		tmpBuf = kvasprintf(GFP_ATOMIC, fmt, ap);
		va_end(ap);

		if (tmpBuf) {
			LogString_Internal(tmpBuf, inSender);
			kfree(tmpBuf);
		}
	}
}

/* #endif */

/**
 *	Log binary signal.
 *
 *	@param	inSigCode	(in)	signal code
 *	@param	inSigBuf	(in)	pointer to signal buffer
 *	@param	inSigBufSize	(in)	size of signal buffer in bytes
 *	@param	inState		(in)	receiving task's state information
 *	@param	inSender	(in)	ID of sending task
 *
**/
void BCMLOG_LogSignal(unsigned int inSigCode,
		      void *inSigBuf,
		      unsigned int inSigBufSize,
		      unsigned short inState, unsigned short inSender)
{
	/* ignore logging requests if we're doing a CP crash dump */
	if (!CpCrashDumpInProgress() && BCMLOG_LogIdIsEnabled(inSender)) {
		LogSignal_Internal(inSigCode,
				   inSigBuf,
				   inSigBufSize,
				   inState, inSender, LOGSIGNAL_COMPRESS);
	}
}

/**
 *	Log binary signal (internal api)
 *
 *	@param	inSigCode	(in)	signal code
 *	@param	inSigBuf	(in)	pointer to signal buffer
 *	@param	inSigBufSize	(in)	size of signal buffer in bytes
 *	@param	inState		(in)	receiving task's state information (optional)
 *	@param	inSender	(in)	ID of sending task
 *	@param	inCompress	(in)	if 0 send signal uncompressed, otherwise try to compress signal
 *
**/
static void LogSignal_Internal(unsigned int inSigCode,
			       void *inSigBuf,
			       unsigned int inSigBufSize,
			       unsigned short inState,
			       unsigned short inSender,
			       enum LogSignalCompress_t inCompress)
{
	/**
	 * NOTE: based on code in LOG_SignalToLoggingPort()
	 * from msp/debug/src/log.c in 2157 SDB
	 **/
	unsigned char frame_head[HWTRC_B_SIGNAL_HEADER_SIZE * 2];
	char frame_head_size = 0;
	unsigned char mtt_payload[8];
	char mtt_payload_size = 0;
	unsigned char frame_end[4];
	char frame_end_size = 0;
	unsigned char *compressedBuffer = NULL;
	unsigned long compress_size;
	unsigned char *bufToSend = NULL;
	unsigned long bufToSendSize = 0;
	unsigned short chksum;

	if (inSigBufSize == 0) {
		/* signal with no data */

		/* MTT header */
		frame_head_size = BCMMTT_MakeMTTSignalHeader(6, frame_head);

		/* MTT payload */
		mtt_payload_size = 6;
		mtt_payload[0] = inSigCode >> 24;
		mtt_payload[1] = (inSigCode >> 16) & 0xFF;
		mtt_payload[2] = (inSigCode >> 8) & 0xFF;
		mtt_payload[3] = inSigCode & 0xFF;
		mtt_payload[4] = (unsigned char)inState;
		mtt_payload[5] = (unsigned char)inSender;

		/* MTT frame end */
		frame_end_size = 2;
		chksum = Checksum16(mtt_payload, 6);
		frame_end[0] = chksum >> 8;
		frame_end[1] = chksum & 0xFF;
	} else if ((inSigBufSize <= LOG_COMPRESS_THRESHOLD)
		   || (LOGSIGNAL_NOCOMPRESS == inCompress)) {
		/* signal with data to be sent uncompressed */

		/* MTT header */
		frame_head_size =
		    BCMMTT_MakeMTTSignalHeader(inSigBufSize + 7, frame_head);

		/* MTT payload */
		mtt_payload_size = 7;
		mtt_payload[0] = inSigCode >> 24;
		mtt_payload[1] = (inSigCode >> 16) & 0xFF;
		mtt_payload[2] = (inSigCode >> 8) & 0xFF;
		mtt_payload[3] = inSigCode & 0xFF;
		mtt_payload[4] = (unsigned char)inState;
		mtt_payload[5] = (unsigned char)inSender;
#ifdef __BIG_ENDIAN
		/* flag: uncompressed, big endian */
		mtt_payload[6] = BE_UNCOMPRESS_FLAG;
#else
		/* flag: uncompressed, little endian */
		mtt_payload[6] = LE_UNCOMPRESS_FLAG;
#endif

		/* MTT frame end */
		frame_end_size = 2;
		chksum = Checksum16(mtt_payload, 7);
		chksum += Checksum16(inSigBuf, inSigBufSize);
		frame_end[0] = chksum >> 8;
		frame_end[1] = chksum & 0xFF;

		bufToSend = inSigBuf;
		bufToSendSize = inSigBufSize;
	} else {
		/* signal with data that is to be sent compressed */
		compressedBuffer = kmalloc((inSigBufSize * 2), GFP_ATOMIC);

		if (compressedBuffer == NULL) {
#ifdef BCMLOG_DEBUG_FLAG
			g_malloc_sig_buf++;
#endif
			BCMLOG_PRINTF(BCMLOG_CONSOLE_MSG_ERROR,
				      "allocation error\n");
			return;
		}

		compress_size =
		    compress_memcpy((char *)compressedBuffer, (char *)inSigBuf,
				    inSigBufSize);

		/* MTT header */
		frame_head_size =
		    BCMMTT_MakeMTTSignalHeader(compress_size + 7, frame_head);

		/* MTT payload */
		mtt_payload_size = 7;
		mtt_payload[0] = inSigCode >> 24;
		mtt_payload[1] = (inSigCode >> 16) & 0xFF;
		mtt_payload[2] = (inSigCode >> 8) & 0xFF;
		mtt_payload[3] = inSigCode & 0xFF;
		mtt_payload[4] = (unsigned char)inState;
		mtt_payload[5] = (unsigned char)inSender;
#ifdef __BIG_ENDIAN
		/* flag: compressed, big endian */
		mtt_payload[6] = BE_COMPRESS_FLAG;
#else
		/* flag: compressed, little endian */
		mtt_payload[6] = LE_COMPRESS_FLAG;
#endif

		/* MTT frame end */
		frame_end_size = 2;
		chksum = Checksum16(mtt_payload, 7);
		chksum += Checksum16(compressedBuffer, compress_size);
		frame_end[0] = chksum >> 8;
		frame_end[1] = chksum & 0xFF;

		bufToSend = compressedBuffer;
		bufToSendSize = compress_size;
	}

	if (CpCrashDumpInProgress()) {
		/* crash log, so send directly to SDCARD/MTT */

		/* write out frame header... */
		BCMLOG_HandleCpCrashDumpData(frame_head, frame_head_size);
		/* then payload... */
		BCMLOG_HandleCpCrashDumpData(mtt_payload, mtt_payload_size);
		/* then signal if applicable... */
		if (bufToSend && (bufToSendSize > 0))
			BCMLOG_HandleCpCrashDumpData(bufToSend, bufToSendSize);
		/* then frame end */
		BCMLOG_HandleCpCrashDumpData(frame_end, frame_end_size);
	} else {
		unsigned long totallen, availlen;

		/* regular logging -- multiple blocks */
		/* must be output contiguously */
		/* so keep the 'output lock' until all are processedd */
		unsigned long irql = AcquireOutputLock();

		totallen =
		    frame_head_size + mtt_payload_size + frame_end_size +
		    bufToSendSize;
		availlen = BCMLOG_GetFreeSize();
		if (availlen > totallen) {
			/**
			 * write out frame header...
			 * (located on stack; do not free when done)
			 **/
			BCMLOG_Output(frame_head, frame_head_size, 1);
			/**
			 * then payload...
			 * (located on stack; do not free when done)
			 **/
			BCMLOG_Output(mtt_payload, mtt_payload_size, 0);

			/* then signal if applicable... */
			if (bufToSend && (bufToSendSize > 0))
				BCMLOG_Output(bufToSend, bufToSendSize, 0);
			/**
			 * then frame end
			 * (located on stack; do not free when done)
			 **/
			BCMLOG_Output(frame_end, frame_end_size, 0);
		}
#ifdef BCMLOG_DEBUG_FLAG
		else
			pr_info("Warning: want %d, have %d", totallen,
				availlen);
#endif

		ReleaseOutputLock(irql);
	}

	kfree(compressedBuffer);

	return;
}

/**
 *	Handle a CP log message from IPC server.
 *
 *	@param	buf		(in)	message buffer
 *	@param	size	(in)	message length
 *	@note	does not free the IPC message buffer
 **/
void BCMLOG_HandleCpLogMsg(unsigned char *buf, int size)
{
	if (!CpCrashDumpInProgress()) {
		unsigned long irql = AcquireOutputLock();

		BCMLOG_Output(buf, size, 1);

		ReleaseOutputLock(irql);
	}
}

#ifdef CONFIG_BRCM_CP_CRASH_DUMP_EMMC
void BCMLOG_WriteEMMC(const char *buf, int size)
{
	struct cpanic_data *ctx = (struct cpanic_data *)&drv_ctx;
	int ret = 0;
	static int tot_size;
	size_t wlen = 0;
	int num_pages, written, fill_size;
	int wr_sz;
	unsigned long partition_end = get_apanic_end_address();

	wr_sz = ctx->mmc->write_bl_len;
	written = wlen = fill_size = 0;

	/* final write? */
	if (buf == NULL)
		goto final;

	/* we accumulate anything less than page size */
	if ((tot_size + size) < wr_sz) {
		memcpy(cp_buf + tot_size, buf, size);
		tot_size += size;
		return;
	}

	fill_size = ctx->mmc->write_bl_len - tot_size;
	memcpy(cp_buf + tot_size, buf, fill_size);

final:
	if (offs >= partition_end) {
		pr_err("ERROR %s: Write across the partition boundary\n",
			__func__);
		return;
	}
	/* write locally stored data first */
	ret =
	    ctx->mmc->block_dev.block_write(ctx->mmc_poll_dev_num, offs, 1,
					    cp_buf);
	if (ret <= 0) {
		printk(KERN_CRIT
		       "%s: Error writing data to flash at line %d, offs:%lx ret:%d!!\n",
		       __func__, __LINE__, offs, ret);
		return;
	}

	if (buf == NULL)
		return;

	memset(cp_buf, 0, wr_sz);
	offs += ret;
	buf += fill_size;
	written += fill_size;

	/* write the passed data now */
	num_pages = (size - fill_size) / wr_sz;
	while (num_pages) {
		memcpy(cp_buf, buf, wr_sz);

		if (offs >= partition_end) {
			pr_err("ERROR %s: Write across the partition boundary\n"
			, __func__);
			return;
		}

		ret =
		    ctx->mmc->block_dev.block_write(ctx->mmc_poll_dev_num, offs,
						    1, cp_buf);
		if (ret <= 0) {
			printk(KERN_CRIT
			       "%s: Error writing data line:%d, offs:%lx ret:%d!!\n",
			       __func__, __LINE__, offs, ret);
			return;
		}

		buf += wr_sz;
		offs += ret;
		written += wr_sz;
		memset(cp_buf, 0, wr_sz);
		num_pages--;
	}

	if (size - written) {
		memcpy(cp_buf, buf, (size - written));
		tot_size = size - written;
	} else
		tot_size = 0;
}

static void start_emmc_crashlog(void)
{
	unsigned long blk;

	blk = get_apanic_start_address();
	if (blk == 0) {
		printk("apanic: Invalid block number \r\n");
		return;
	}

	offs = (KPANIC_CP_DUMP_OFFSET / 0x200) + blk;
	cp_buf = kmalloc(CP_DUMP_LEN, GFP_ATOMIC);
	if (!cp_buf) {
		printk(KERN_ERR "%s: kmalloc failed!!\n", __func__);
		return;
	}
	memset(cp_buf, 0, CP_DUMP_LEN);
}
#endif

static int start_sdcard_crashlog(struct file *inDumpFile)
{
	int ret_status;
	struct timespec ts;
	struct rtc_time tm;
	char assertFileName[CP_CRASH_DUMP_MAX_LEN];
	/* need to tell kernel that pointers from within the */
	/* kernel address space are valid (needed to do */
	/* file ops from kernel) */
	sCrashDumpFS = get_fs();
	set_fs(KERNEL_DS);

	/* get current time */
	getnstimeofday(&ts);
	rtc_time_to_tm(ts.tv_sec, &tm);
	snprintf(assertFileName, CP_CRASH_DUMP_MAX_LEN,
		 "%s%s%d_%02d_%02d_%02d_%02d_%02d%s",
		 BCMLOG_GetFileBase(),
		 CP_CRASH_DUMP_BASE_FILE_NAME,
		 tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
		 tm.tm_hour, tm.tm_min, tm.tm_sec, CP_CRASH_DUMP_FILE_EXT);

	sDumpFile =
	    filp_open(assertFileName,
		      O_WRONLY | O_TRUNC | O_LARGEFILE | O_CREAT, 666);
	if (IS_ERR(sDumpFile)) {
		BCMLOG_PRINTF(BCMLOG_CONSOLE_MSG_ERROR,
			      "failed to open sdDumpFile %s\n", assertFileName);
		sDumpFile = NULL;
		ret_status = 0;
		} else {
		BCMLOG_PRINTF(BCMLOG_CONSOLE_MSG_ERROR,
			      "sdDumpFile %s opened OK\n", assertFileName);
		ret_status = 1;
		}
	return ret_status;
}

/**
 *	Prepare to handle CP crash dump. During CP crash
 *  dump, all other logging requests are ignored.
 *
 *	@param	inDumpFile (in)	ptr to already open dump file (may be NULL)
 *  @note inDumpFile must be opened by caller before calling
 *        BCMLOG_StartCpCrashDump() and closed after calling
 *        BCMLOG_EndCpCrashDump()
 *
 **/
void BCMLOG_StartCpCrashDump(struct file *inDumpFile, int cpresetStatus)
{
	int dump_port;
	/* note: don't need wakelock here as CP crash */
	/* dump is done under IPC wakelock */
	g_module.dumping_cp_crash_log = 1;
	sDumpFile = inDumpFile;

	if ((BCMLOG_OUTDEV_SDCARD == BCMLOG_GetCpCrashLogDevice())
#ifdef CONFIG_BRCM_CP_CRASH_DUMP_EMMC
	    && ap_triggered
#endif
	    ) {
		dump_port = BCMLOG_OUTDEV_PANIC;
		BCMLOG_SetCpCrashLogDevice(dump_port);
	}

	pr_info("BCMLOG cp crash port %d", BCMLOG_GetCpCrashLogDevice());

	switch (BCMLOG_GetCpCrashLogDevice()) {
	case BCMLOG_OUTDEV_SDCARD:
		if ((Get_SDCARD_Available() < CP_CRASH_DUMP_SDCARD_SIZE) ||
				!start_sdcard_crashlog(inDumpFile)) {
#ifdef CONFIG_BCM_MODEM
			if (!cpresetStatus) {
				plat_iounmap_ns(get_vaddr_ipc
				(IPC_CP_CRASH_SUMMARY_AREA),
				free_size_ipc
				(IPC_CP_CRASH_SUMMARY_AREA_SZ));
				/*sdcard not present or failed,
				   save the dump to flash */
				BCMLOG_SetCpCrashLogDevice(BCMLOG_OUTDEV_PANIC);
				abort();
			}
#endif
		}
		break;
	case BCMLOG_OUTDEV_PANIC:
#ifdef CONFIG_BRCM_CP_CRASH_DUMP_EMMC
		start_emmc_crashlog();
#endif
		break;
	case BCMLOG_OUTDEV_NONE:
		BCMLOG_PRINTF(BCMLOG_CONSOLE_MSG_ERROR,
			      "CP Crash Log no output selected\n");
		break;
	}

	/* buffer for packaging up CP RAM dump blocks for crash log */
	sMemDumpSignalBuf = kmalloc((WORDS_PER_SIGNAL << 3), GFP_ATOMIC);
	if (sMemDumpSignalBuf == NULL) {
		pr_err("sMemDumpSignalBuf kmalloc failed\n");
		return;
	}
}

/**
 *	Done with CP crash dump. Normal logging is resumed.
 **/
void BCMLOG_EndCpCrashDump(void)
{
	g_module.dumping_cp_crash_log = 0;

	kfree(sMemDumpSignalBuf);

	switch (BCMLOG_GetCpCrashLogDevice()) {
	case BCMLOG_OUTDEV_SDCARD:
		if (sDumpFile)
			filp_close(sDumpFile, NULL);
		set_fs(sCrashDumpFS);
		sDumpFile = NULL;
		break;
	case BCMLOG_OUTDEV_ACM:
		WriteToLogDev_ACM();
		break;
	case BCMLOG_OUTDEV_PANIC:
#ifdef CONFIG_BRCM_CP_CRASH_DUMP_EMMC
		BCMLOG_WriteEMMC(NULL, 0);
#endif
		kfree(cp_buf);
		break;
	case BCMLOG_OUTDEV_NONE:
		break;
	}

}

/**
 *	Handle CP crash dump data from IPC server.
 *
 *	@param	buf		(in)	message buffer
 *	@param	size	(in)	message length
 *	@note	does not free the IPC message buffer
 **/
void BCMLOG_HandleCpCrashDumpData(const char *buf, int size)
{
	unsigned long irql;
	switch (BCMLOG_GetCpCrashLogDevice()) {
	case BCMLOG_OUTDEV_SDCARD:
		if (sDumpFile && sDumpFile->f_op && sDumpFile->f_op->write) {
			sDumpFile->f_op->write(sDumpFile, buf, size,
					       &sDumpFile->f_pos);
		}
		break;
	case BCMLOG_OUTDEV_PANIC:
#ifdef CONFIG_BRCM_CP_CRASH_DUMP_EMMC
		BCMLOG_WriteEMMC(buf, size);
#endif
		break;
	case BCMLOG_OUTDEV_RNDIS:
		BCMLOG_klogging_crashdump(buf, size);
		break;
	case BCMLOG_OUTDEV_ACM:
	case BCMLOG_OUTDEV_STM:
		irql = AcquireOutputLock();
		BCMLOG_Output((unsigned char *)buf, size, 0);
		ReleaseOutputLock(irql);
		break;
	case BCMLOG_OUTDEV_NONE:
		break;
	}
}

/**
 *	Log null terminated string during crash dump.
 *
 *	@param	inLogString		(in)	null terminated string to be logged
 *
**/
void BCMLOG_LogCPCrashDumpString(const char *inLogString)
{
	int logStrSize;
	int mttFrameSize;
	unsigned long irql;

	/* include the NULL termination... */
	logStrSize = strlen(inLogString) + 1;

	/* mtt adds framing bytes; get total size */
	mttFrameSize = BCMMTT_GetRequiredFrameLength(logStrSize);

	if (mttFrameSize > 0) {
		char *kbuf_mtt = kmalloc(mttFrameSize, GFP_ATOMIC);

		if (!kbuf_mtt) {
			BCMLOG_PRINTF(BCMLOG_CONSOLE_MSG_ERROR,
				      "allocation error\n");
			return;
		}

		mttFrameSize =
		    BCMMTT_FrameString(kbuf_mtt, inLogString, mttFrameSize);

		switch (BCMLOG_GetCpCrashLogDevice()) {
		case BCMLOG_OUTDEV_SDCARD:
			if (sDumpFile && sDumpFile->f_op
			    && sDumpFile->f_op->write) {
				sDumpFile->f_op->write(sDumpFile, kbuf_mtt,
						       mttFrameSize,
						       &sDumpFile->f_pos);
			}
			break;
		case BCMLOG_OUTDEV_PANIC:
#ifdef CONFIG_BRCM_CP_CRASH_DUMP_EMMC
			BCMLOG_WriteEMMC(kbuf_mtt, mttFrameSize);
#endif
			break;
		case BCMLOG_OUTDEV_RNDIS:
			BCMLOG_klogging_crashdump(kbuf_mtt, mttFrameSize);
			break;
		case BCMLOG_OUTDEV_STM:
		case BCMLOG_OUTDEV_ACM:
			irql = AcquireOutputLock();
			BCMLOG_Output(kbuf_mtt, mttFrameSize, 0);
			ReleaseOutputLock(irql);
			break;
		case BCMLOG_OUTDEV_NONE:
			break;
		}
		kfree(kbuf_mtt);
	}
}

/**
 *	Handle memory dump data from CP crash.
 *
 *	@param	inPhysAddr	(in)	physical address of memory to dump
 *	@param	size	    (in)	size in bytes of buf
 *
 **/
void BCMLOG_HandleCpCrashMemDumpData(const char *inPhysAddr, int size)
{
	unsigned long p, sz, csz, n, progress;
	unsigned char *pHbuf;
	unsigned char *pLength;
	unsigned char *pChksum;
	unsigned short chksum;
	char tmpStr[255];
	unsigned long currPhysical = (unsigned long)inPhysAddr;

	/* make sure we were able to allocate our buffer... */
	if (NULL == sMemDumpSignalBuf) {
		BCMLOG_PRINTF(BCMLOG_CONSOLE_MSG_ERROR,
			      "BCMLOG_HandleCpCrashMemDumpData: failed to allocate mem dump signal buffer\n");
		snprintf(tmpStr, 255,
			 "*** %s: failed to allocate mem dump signal buffer ***",
			 __func__);
		BCMLOG_LogCPCrashDumpString(tmpStr);
		return;
	}

	BCMLOG_LogCPCrashDumpString("** BCMLOG_HandlCpCrashMemDumpData **\n");

	/**
	 * NOTE: this is mostly copied from DUMP_CompressedMemory()and
	 * DUMP_Signal() from CIB dump.c
	 **/
	n = 0;
	for (progress = 0; progress < size; currPhysical +=
	     (WORDS_PER_SIGNAL << 2), progress += (WORDS_PER_SIGNAL << 2)) {
		sz = size - progress;
		if (sz > (WORDS_PER_SIGNAL << 2))
			sz = WORDS_PER_SIGNAL << 2;

		p = (unsigned long) plat_ioremap_ns((unsigned long __force)
				get_vaddr(BCMLOG_IOREMAP_AREA),
				BCMLOG_IOREMAP_AREA_SZ,
				(phys_addr_t)currPhysical);

		if (!p) {
			BCMLOG_PRINTF(BCMLOG_CONSOLE_MSG_ERROR,
					"BCMLOG: remap failed 0x%lx, 0x%lx\n",
					p, currPhysical);
			snprintf(tmpStr, 255,
					"%s remap failed 0x%lx, 0x%lx",
					__func__, p, currPhysical);

			BCMLOG_LogCPCrashDumpString(tmpStr);
			return;
		}
		/**
		 * **FIXME** MAG doesn't appear to be needed under Android...
		 * address p may point to unreadable address; memcpy here for
		 * less detour to DataAbort exception
		 * Hui Luo, 1/3/08
		 * memcpy(b, (unsigned char*)p, sz);
		 **/

		pHbuf = sMemDumpSignalBuf;
		/* flag: compressed, little endian */
		*pHbuf++ = LE_COMPRESS_FLAG;
		*pHbuf++ = 0xC7;
		*pHbuf++ = 0xE5;
		*pHbuf++ = 0xBB;
		*pHbuf++ = 0xAA;
		*pHbuf++ = 0xC2;
		*pHbuf++ = 0xDE;
		*pHbuf++ = 0xEA;
		*pHbuf++ = 0xCD;

		pChksum = pHbuf;
		pHbuf += 2;
		pLength = pHbuf;
		pHbuf += 2;

		*pHbuf++ = currPhysical >> 24;
		*pHbuf++ = (currPhysical << 8) >> 24;
		*pHbuf++ = (currPhysical << 16) >> 24;
		*pHbuf++ = (currPhysical << 24) >> 24;

		*pHbuf++ = sz >> 24;
		*pHbuf++ = (sz << 8) >> 24;
		*pHbuf++ = (sz << 16) >> 24;
		*pHbuf++ = (sz << 24) >> 24;

		csz = compress_memcpy((char *)pHbuf, (char *)p, sz);

		*pLength = (csz + 8) >> 8;
		*(pLength + 1) = (csz + 8) & 0xFF;

		chksum = Checksum16(pLength, csz + 10);
		*pChksum = chksum >> 8;
		*(pChksum + 1) = chksum & 0xFF;

		/**
		* use internal api so we don't get blocked by crash dump flag
		**/
		LogSignal_Internal(MEMORY_DUMP,
				sMemDumpSignalBuf,
				csz + 21, 0, 250,
				LOGSIGNAL_NOCOMPRESS);

		n++;
		if (n >= 32) {
			n = 0;
			snprintf(tmpStr, 255,
				 "CP memory dump done %ld of %d bytes. Do not stop logging",
				 progress, size);
			BCMLOG_LogCPCrashDumpString(tmpStr);
		}
		/**
		 * A small sleep to let slower drivers like ACM time to dump
		 **/
		if (BCMLOG_GetCpCrashLogDevice() == BCMLOG_OUTDEV_ACM) {
			set_current_state(TASK_INTERRUPTIBLE);
			schedule_timeout(1);
		}

		plat_iounmap_ns(get_vaddr(BCMLOG_IOREMAP_AREA),
				free_size_bcmlog(BCMLOG_IOREMAP_AREA_SZ));
	}
}

/**
 *	Function to log a binary signal described by a link list
 *	@param	sig_code (in) binary signal code, the high 16 bits are
 *		logging ID/receiver
 *	@param	*link_list (in) starting address of a link list
 *	@param	list_size (in) size of the link list, in number of list items
 *	@param	state (in) receiving task's state information (optional)
 *	@param	sender (in) ID of sending task
 *	@note
 *	This function is provided to assist BCMLOG_LogSignal() in case the
 *	binary signal is assembled from different memories, where a link list
 *	can avoid unnecessary memory.
 **/
void BCMLOG_LogLinkList(unsigned int sig_code,
			struct BCMLOG_LogLinkList_t *link_list,
			unsigned int list_size,
			unsigned short state, unsigned short sender)
{
	struct BCMLOG_LogLinkList_t LinkList[12];
	unsigned char frame_head[HWTRC_B_SIGNAL_HEADER_SIZE * 2];
	unsigned char mtt_payload[8];
	unsigned char frame_end[4];
	unsigned char *compressedBuffer = NULL;
	unsigned long compress_size;
	unsigned short chksum;
	unsigned long ptr_size;
	unsigned char *ptr;
	unsigned long i;

	if ((BCMLOG_LogIdIsEnabled(sender)) && (list_size <= 8)) {

		ptr_size = 0;
		for (i = 0; i < list_size; i++)
			ptr_size += link_list[i].size;

		if (ptr_size == 0) {
			LinkList[0].byte_array = frame_head;
			LinkList[0].size =
			    BCMMTT_MakeMTTSignalHeader(6, frame_head);
			LinkList[1].byte_array = mtt_payload;
			LinkList[1].size = 6;
			mtt_payload[0] = sig_code >> 24;
			mtt_payload[1] = (sig_code >> 16) & 0xFF;
			mtt_payload[2] = (sig_code >> 8) & 0xFF;
			mtt_payload[3] = sig_code & 0xFF;
			mtt_payload[4] = (unsigned char)state;
			mtt_payload[5] = (unsigned char)sender;
			LinkList[2].byte_array = frame_end;
			LinkList[2].size = 2;
			chksum = Checksum16(mtt_payload, 6);
			frame_end[0] = chksum >> 8;
			frame_end[1] = chksum & 0xFF;
			BCMLOG_OutputLinkList(3, LinkList);
		} else if (ptr_size <= LOG_COMPRESS_THRESHOLD) {
			LinkList[0].byte_array = frame_head;
			LinkList[0].size =
			    BCMMTT_MakeMTTSignalHeader(ptr_size + 7,
						       frame_head);
			LinkList[1].byte_array = mtt_payload;
			LinkList[1].size = 7;
			mtt_payload[0] = sig_code >> 24;
			mtt_payload[1] = (sig_code >> 16) & 0xFF;
			mtt_payload[2] = (sig_code >> 8) & 0xFF;
			mtt_payload[3] = sig_code & 0xFF;
			mtt_payload[4] = (unsigned char)state;
			mtt_payload[5] = (unsigned char)sender;
#ifdef __BIG_ENDIAN
			/* flag: uncompressed, big endian */
			mtt_payload[6] = BE_UNCOMPRESS_FLAG;
#else
			/* flag: uncompressed, little endian */
			mtt_payload[6] = LE_UNCOMPRESS_FLAG;
#endif
			for (i = 0; i < list_size; i++)
				LinkList[i + 2] = link_list[i];
			LinkList[i + 2].byte_array = frame_end;
			LinkList[i + 2].size = 2;
			chksum = Checksum16(mtt_payload, 7);
			for (i = 0; i < list_size; i++)
				chksum +=
				    Checksum16(link_list[i].byte_array,
					       link_list[i].size);
			frame_end[0] = chksum >> 8;
			frame_end[1] = chksum & 0xFF;
			BCMLOG_OutputLinkList(list_size + 3, LinkList);
		} else {
			compressedBuffer = kmalloc((ptr_size * 2), GFP_ATOMIC);

			if (compressedBuffer == NULL) {
#ifdef BCMLOG_DEBUG_FLAG
				g_malloc_sig_buf++;
#endif
				BCMLOG_PRINTF(BCMLOG_CONSOLE_MSG_ERROR,
					      "allocation error\n");
				return;
			}

			ptr = compressedBuffer;
			for (i = 0; i < list_size; i++) {
				ptr +=
				    compress_memcpy((char *)ptr,
						    (char *)
						    link_list[i].byte_array,
						    link_list[i].size);
			}
			compress_size =
			    (unsigned long)ptr -
			    (unsigned long)compressedBuffer;

			LinkList[0].byte_array = frame_head;
			LinkList[0].size =
			    BCMMTT_MakeMTTSignalHeader(compress_size + 7,
						       frame_head);
			LinkList[1].byte_array = mtt_payload;
			LinkList[1].size = 7;
			mtt_payload[0] = sig_code >> 24;
			mtt_payload[1] = (sig_code >> 16) & 0xFF;
			mtt_payload[2] = (sig_code >> 8) & 0xFF;
			mtt_payload[3] = sig_code & 0xFF;
			mtt_payload[4] = (unsigned char)state;
			mtt_payload[5] = (unsigned char)sender;
#ifdef __BIG_ENDIAN
			/* flag: compressed, big endian */
			mtt_payload[6] = BE_COMPRESS_FLAG;
#else
			/* flag: compressed, little endian */
			mtt_payload[6] = LE_COMPRESS_FLAG;
#endif
			LinkList[2].byte_array = compressedBuffer;
			LinkList[2].size = compress_size;
			LinkList[3].byte_array = frame_end;
			LinkList[3].size = 2;
			chksum = Checksum16(mtt_payload, 7);
			chksum += Checksum16(compressedBuffer, compress_size);
			frame_end[0] = chksum >> 8;
			frame_end[1] = chksum & 0xFF;
			BCMLOG_OutputLinkList(4, LinkList);

			kfree(compressedBuffer);
		}
	}
	return;
}

/**
 *
 * Function Name:       compress_memcpy
 *
 * Description:         Copy a block of data with compression if needed.
 * Notes:       Return the size of compressed data block
 * Notes2:  Blatantly copied from CIB dump.c
 **/

static unsigned int compress_memcpy(char *dest,
				    char *src, unsigned short nbytes)
{
	register unsigned long n;
	register unsigned char *p, *q, *r, *sn, *rn, c;

	p = (unsigned char *)src;
	q = (unsigned char *)dest;
	sn = (unsigned char *)((unsigned long)src + (unsigned long)nbytes);

	while (p < sn) {
		if (sn < p + 128)
			rn = sn;
		else
			rn = p + 128;
		c = *p;
		for (r = p + 1; r < rn && *r == c; r++)
			;
		n = r - p;
		if (n == 1) {
			*q++ = c;
			if (c == COMPRESS_TOKEN)
				*q++ = 1;
		} else if (n == 2) {
			*q++ = c;
			if (c == COMPRESS_TOKEN)
				*q++ = 2;
			else
				*q++ = c;
		} else if (n == 3) {
			if (c == 0) {
				*q++ = COMPRESS_TOKEN;
				*q++ = 0x83;
			} else if (c == COMPRESS_TOKEN) {
				*q++ = COMPRESS_TOKEN;
				*q++ = 3;
			} else {
				*q++ = c;
				*q++ = c;
				*q++ = c;
			}
		} else {
			*q++ = COMPRESS_TOKEN;
			if (c == 0) {
				*q++ = (n | 0x80);
			} else {
				*q++ = (n & 0x7F);
				*q++ = c;
			}
		}
		p += n;
	}
	return (unsigned int)q - (unsigned int)dest;
}

static unsigned short Checksum16(unsigned char *data, unsigned long len)
{
	unsigned short csum = 0;
	unsigned long i;

	for (i = 0; i < len; i++)
		csum = csum + (unsigned short)(data[i]);

	return csum;
}

static void BCMLOG_OutputLinkList(unsigned long ListSize,
				  struct BCMLOG_LogLinkList_t *LinkList)
{
	unsigned long i, totallen, availlen, irql;

	totallen = 0;
	for (i = 0; i < ListSize; i++)
		totallen += LinkList[i].size;

	irql = AcquireOutputLock();

	availlen = BCMLOG_GetFreeSize();
	if (availlen > totallen) {
		for (i = 0; i < ListSize; i++) {
			if (i == 0) {
				BCMLOG_Output(LinkList[i].byte_array,
					      LinkList[i].size, 1);
			} else {
				BCMLOG_Output(LinkList[i].byte_array,
					      LinkList[i].size, 0);
			}
		}
	}
#ifdef BCMLOG_DEBUG_FLAG
	else
		pr_info("Warning: want %d, have %d", totallen, availlen);
#endif

	ReleaseOutputLock(irql);
}

/**
 *
 * Function Name:       BCMLOG_RegisterPrintkRedirectCbk
 *
 * Description:         Register callback to redirect printk data to
 *			temp buffer using registered callback
 * Notes:		BrcmRedirectPrintkCbk should ONLY copy the data to
 *			temp buffer and return.
 **/
void BCMLOG_RegisterPrintkRedirectCbk(int enable, BrcmRedirectPrintkCbk cb)
{
	unsigned long irql;
	irql = AcquireOutputLock();
	sPrintkCbk = cb;
	atomic_set(&redirectEnabled, enable);
	ReleaseOutputLock(irql);
}


/**
 *	export module init and export functions
 **/
subsys_initcall(BCMLOG_ModuleInit);
module_exit(BCMLOG_ModuleExit);

module_param_named(mtt_log_buffer, g_module.buffer_size, int, 0444);
MODULE_PARM_DESC(mtt_log_buffer, "Size of MTT Log Buffer in bytes");

