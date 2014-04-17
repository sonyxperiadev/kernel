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

/**
 *
 *  @file   bcmlog.h
 *
 *  @brief  Interface to the Broadcom logging driver.
 *
 *  @note   Use code will access driver through an API layer and typically should not
 *          require use of these symbols.
 *
 **/

#ifndef __BCMLOG_H__
#define __BCMLOG_H__

#include <linux/file.h>

#ifdef __cplusplus
extern "C" {
#endif				/* __cplusplus */

/**
 *	Log IDs are in the range 0 to 8191, partitioned as follows:
 *
 *		   0 - 1023 :	reserved to Broadcom
 *		1024 - 32767:	available for custom applications
 *
 *	The following Log IDs are reserved to Broadcom for Android-specific
 *      purposes:
 *
 *		1000 - 1023 :	reserved to Broadcom (Android-specific)
 *
 *	See also logapi.h
 *
 **/

/* maximum LOG ID */
#define BCMLOG_MAX_LOG_ID		32767
/* Android unified logging (e.g., printk logging) */
#define BCMLOG_LOG_ULOGGING_ID		1000
/* Android KRIL logging for basic information */
#define BCMLOG_ANDROID_KRIL_BASIC	1001
/* Android KRIL logging for detail information */
#define BCMLOG_ANDROID_KRIL_DETAIL	1002
/* RPC kernel driver logging for basic information */
#define BCMLOG_RPC_KERNEL_BASIC		1003
/* RPC kernel driver logging for detailed information */
#define BCMLOG_RPC_KERNEL_DETAIL	1004

/* MTT limit ASCII size to be 256 */
#define BCMLOG_MAX_ASCII_STRING_LENGTH	256

/**
 *	miscellaneous definitions
 **/
/* public module name */
#define BCMLOG_MODULE_NAME		"bcm_log"

/**
 *
 *  ioctl commands
 *
 **/
/* log a string */
#define BCMLOG_IOC_LOGSTR	102
/* get or set enable/disable local message logging based on ID */
#define BCMLOG_IOC_ENABLE	103
/* log a signal */
#define BCMLOG_IOC_LOGSIGNAL	104

#define BCMLOG_IOREMAP_GUARD		(SZ_4K)
#define BCMLOG_IOREMAP_AREA_SZ		(SZ_8K)
#define BCMLOG_IOREMAP_AREA		(0)
#define BCMLOG_IOREMAP_NUM_PAGES	((BCMLOG_IOREMAP_AREA_SZ +	\
			BCMLOG_IOREMAP_GUARD) >> PAGE_SHIFT)

#define free_size_bcmlog(size) (size + BCMLOG_IOREMAP_GUARD)

/**
 *  for ioctl cmd BCMLOG_IOC_LOGSTR, a variable of this type
 *	is passed as the 'arg' to ioctl()
 **/
	struct BCMLOG_IoctlLogStr_t {
		char *str;	/* pointer to string buffer */
		int size;	/* size of buffer including terminating null */
		unsigned short sender;	/* ID of sending task */
	};

/**
 *  for ioctl cmd BCMLOG_IOC_ENABLE a variable of this type
 *	is passed as the 'arg' to ioctl()
 **/
	struct BCMLOG_IoctlEnable_t {
		/*log ID */
		unsigned short id;
		/**
		 *  if set != 0 set (write) the enable/disable status;
		 *  if set == 0 return (read) the enable/disable status
		 **/
		unsigned char set;
		/**
		 *  if set != 0 ID logging is enabled based on this member
		 *  if set == 0 ID logging this member contains
		 *  current enabled/disabled status on return from ioctl call
		 **/
		unsigned char enable;
	};

/**
 *  for ioctl cmd BCMLOG_IOC_LOGSIGNAL a variable of this type
 *	is passed as the 'arg' to ioctl()
 **/
	struct BCMLOG_IoctlLogSignal_t {
		/*binary signal code */
		unsigned int sigCode;
		/*pointer to signal buffer */
		void *sigPtr;
		/* size of signal buffer in bytes */
		unsigned int sigBufSize;
		/* receiving task's state information */
		unsigned short state;
		/* ID of sending task */
		unsigned short sender;
	};

/**
 *  for BCMLOG_LogLinkList
 **/
	struct BCMLOG_LogLinkList_t {
		unsigned char *byte_array;
		unsigned int size;
	};

/**
 *	Log null terminated string.
 *
 *	@param	inLogString		(in)	null terminated string to be logged
 *	@param	inSender		(in)	ID of sending task
 *
 **/
	void BCMLOG_LogString(const char *inLogString, unsigned short inSender);

/**
 *	Log a formatted list of arguments
 *	@param	inSender (in) ID of the source module
 *	@param	*fmt (in) a format string as printf()
 *	...	(in) a list of arguments as printf()
 *	@note	if the Broadcom Unified Logging option is not
 *		enabled then printk is called
 **/

/*
#ifdef CONFIG_BRCM_UNIFIED_LOGGING
 */
	void BCMLOG_Printf(unsigned short inSender, char *fmt, ...);
/*
#else
#define BCMLOG_Printf( inSender, ... ) printk( __VA_ARGS__ )
#endif
 */

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
			      unsigned short inState, unsigned short inSender);

/**
 *	Handle a CP log message from IPC server.
 *
 *	@param	buf		(in)	message buffer
 *	@param	size	(in)	message length
 *	@note	does not free the IPC message buffer
 **/
	void BCMLOG_HandleCpLogMsg(unsigned char *buf, int size);

/**
 *	Prepare to handle CP crash dump. During CP crash
 *  dump, all other logging requests are ignored.
 **/
	void BCMLOG_StartCpCrashDump(struct file *inDumpFile,
				     int cpresetStatus);

/**
 *	Done with CP crash dump. Normal logging is resumed.
 **/
	void BCMLOG_EndCpCrashDump(void);

/**
 *	Handle CP crash dump data from IPC server.
 *
 *	@param	buf		(in)	message buffer
 *	@param	size	(in)	message length
 *	@note	does not free the IPC message buffer
 **/
	void BCMLOG_HandleCpCrashDumpData(const char *buf, int size);

/**
 *	Handle memory dump data from CP crash.
 *
 *	@param	address	(in)	virtual address of memory to dump
 *	@param	size	(in)	size in bytes of buf
 *
 **/
	void BCMLOG_HandleCpCrashMemDumpData(const char *address, int size);

/**
 *	Log null terminated string during crash dump.
 *
 *	@param	inLogString		(in)	null terminated string to be logged
 *
 **/
	void BCMLOG_LogCPCrashDumpString(const char *inLogString);

/**
 *	Check for CP crash dump in progres
 *	@return	non-zero if CP crash dump in progress, else zero
 **/
	int CpCrashDumpInProgress(void);

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
				unsigned short state, unsigned short sender);

/**
 *	Logging output devices.
 **/
#define BCMLOG_OUTDEV_NONE		0	/* undefined or disabled */
#define BCMLOG_OUTDEV_PANIC		1	/* panic partition / MTD */
#define BCMLOG_OUTDEV_SDCARD	2	/* sdcard */
#define BCMLOG_OUTDEV_RNDIS		3	/* MTT/RNDIS */
#define BCMLOG_OUTDEV_UART		4	/* UART */
#define BCMLOG_OUTDEV_ACM		5	/* ACM/OBEX */
#define BCMLOG_OUTDEV_STM		6	/* STM */
#define BCMLOG_OUTDEV_CUSTOM	7	/* custom handler */
#define BCMLOG_OUTDEV_MAX		8

	extern struct apanic_data drv_ctx;
#ifdef CONFIG_BRCM_CP_CRASH_DUMP_EMMC
	extern unsigned long get_apanic_start_address(void);
	extern unsigned long get_apanic_end_address(void);
#endif

/* help functions for STM crash dump */
#ifdef CONFIG_BCM_STM
	extern int stm_trace_buffer_onchannel(int channel, const void *data,
					      size_t length);
	extern void stm_trace_buffer_end(int channel);
	extern int stm_trace_buffer_data(int channel, const void *data_ptr,
					 size_t length);
	extern void stm_trace_buffer_start(int channel);
#endif

/**
 *	Get runtime log device
 **/
	int BCMLOG_GetRunlogDevice(void);
	int BCMLOG_GetCpCrashLogDevice(void);
	int BCMLOG_GetApCrashLogDevice(void);
	void ReleaseOutputLock(unsigned long irql);
	unsigned long AcquireOutputLock(void);
/**
 *	Set runtime log device
 **/
	void BCMLOG_SetRunlogDevice(int run_log_dev);

/**
 *	Get runtime SD log file size in bytes
 **/
	int BCMLOG_GetSdFileMax(void);

	int BCMLOG_GetBufferSize(void);
	void BCMLOG_SetBufferSize(int buffer_size);


	extern void (*BrcmLogString) (const char *inLogString,
				      unsigned short inSender);
#ifdef CONFIG_BRCM_CP_CRASH_DUMP_EMMC
	extern int ap_triggered;
#endif
	extern struct vm_struct *ipc_cpmap_area;
	extern int cp_crashed;
	extern int brcm_klogging(char *data, int length);
	extern int brcm_retrive_early_printk(void);
	extern int bcmlog_mtt_on;
	extern unsigned ramdump_enable;
	extern unsigned short bcmlog_log_ulogging_id;
	extern unsigned long BCMLOG_GetFreeSize(void);
#ifdef BCMLOG_DEBUG_FLAG
	extern unsigned int g_malloc_sig_buf;
#endif
	extern void abort(void);
/**
 *	printk redirect callback registration
 **/	typedef int (*BrcmRedirectPrintkCbk)(const char *str);
	void BCMLOG_RegisterPrintkRedirectCbk(int enable,
			BrcmRedirectPrintkCbk cb);

#ifdef __cplusplus
}
#endif				/* __cplusplus */
#endif				/* __BCMLOG_H__ */
