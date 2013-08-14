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
*   software in any way with any other Broadcom software provided
*   under a license
*   other than the GPL, without Broadcom's express prior written consent.
*
****************************************************************************/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/workqueue.h>
#include <linux/uaccess.h>
#include <linux/errno.h>
#include <linux/brcm_console.h>
#include <linux/types.h>
#include <linux/time.h>
#include <linux/rtc.h>
#include <trace/stm.h>
#include "linux/broadcom/mobcom_types.h"
#include <linux/vt_kern.h>
#include <linux/statfs.h>
#include <linux/namei.h>
#include "bcmlog.h"
#include "fifo.h"
#include "bcmmtt.h"
#include "output.h"
#include "config.h"

#define BCMLOG_LATEST_LOGS

#ifdef CONFIG_BRCM_NETCONSOLE

/*
 *	forward declarations
 */
static int netconsole_start_cb(void);
static int netconsole_stop_cb(void);

#ifdef BCMLOG_DEBUG_FLAG
unsigned int g_malloc_sig_buf;
static int nospace;
static int noheader;
static int splitbuffer;
static int writefail;
static int n100;
static unsigned int totalbyte;
#endif

/**
 *	local vars
 **/
/* flow control state for RNDIS, set/reset by flow control callbacks */
static char g_netconsole_on;

#endif
/* flow control state for ACM, set/reset by flow control callbacks */
static char g_acm_on;

/* 23MB reserved area for AP/CP crash dump */
#define MTT_SD_RESERVED (23 * 1024 * 1024)

/*output fifo */
static BCMLOG_Fifo_t g_fifo;

/* keep track of the last log file name */
char g_last_log_fname[BCMLOG_OUTPUT_MAX_LOG_PATHNAME];

/**
 *	frame counter
 **/
static unsigned char g_frame_counter;

/**
 *	ACM call backs
**/
static int acm_start_cb(void);
static int acm_stop_cb(void);

struct acm_callbacks {
	/** Start function for role change */
	int (*start) (void);
	/** Stop Function for role change */
	int (*stop) (void);
};

/* ACM flow control callbacks */
static struct acm_callbacks _acm_cb = {
	.start = acm_start_cb,
	.stop = acm_stop_cb
};

/**
 *	flow control callback for ACM (start flow), called when ACM
 *	available to transport data
 **/
static int acm_start_cb(void)
{
	g_acm_on = 1;
	pr_info("BCMLOG: acm logging started\n");
	return 0;
}

/**
 *	flow control callback for ACM (stop flow), called when ACM
 *	available to transport data
 **/
static int acm_stop_cb(void)
{
	g_acm_on = 0;
	pr_info("BCMLOG: acm logging stopped\n");
	return 0;
}

struct acm_logging_callbacks *get_acm_callback_func(void)
{
	return &_acm_cb;
}
EXPORT_SYMBOL(get_acm_callback_func);

struct WriteToLogDevParms_t {
	struct work_struct wq;
	int outdev;
	int prev_outdev;
	struct file *file;
	struct file *acm_file;
	u8 busy;
};

/* worker thread vars */
static struct WriteToLogDevParms_t g_devWrParms = {
	.file = 0,
	.busy = 0,
	.prev_outdev = BCMLOG_OUTDEV_NONE,
	.outdev = BCMLOG_OUTDEV_NONE,
};

#ifdef CONFIG_BRCM_NETCONSOLE
/* RNDIS flow control callbacks */
static struct brcm_netconsole_callbacks _cb = {
	.start = netconsole_start_cb,
	.stop = netconsole_stop_cb
};

/**
 *	flow control callback for RNDIS (start flow), called when RNDIS
 *	available to transport data
 **/
static int netconsole_start_cb(void)
{
	g_netconsole_on = 1;
	pr_info("BCMLOG: RNDIS netconsole started\n");
	return 0;
}

/**
 *	flow control callback for RNDIS (stop flow), called when RNDIS
 *	available to transport data
 **/
static int netconsole_stop_cb(void)
{
	g_netconsole_on = 0;
	pr_info("BCMLOG: RNDIS netconsole stopped\n");
	return 0;
}
#endif

/*
 *	Create a file name for logging, based on system time.
 */
static void GetLogFileName(char *buf, char *rootdev, int size)
{

	struct timespec ts;
	struct rtc_time tm;

	getnstimeofday(&ts);
	rtc_time_to_tm(ts.tv_sec, &tm);

	snprintf(buf, size,
		 "%slog-%d_%02d_%02d_%02d_%02d_%02d.bin",
		 rootdev,
		 tm.tm_year + 1900,
		 tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
}

#define MAX_FS_WRITE_SIZE 16384

/*
 *	Return available SD card size (bytes)
 *	If over 2G, return 2G
 */
int Get_SDCARD_Available(void)
{
	struct kstatfs sbuf;
	struct path path;
	int ret;
	u64 int_max = INT_MAX;

	ret = kern_path(BCMLOG_GetFileBase(), LOOKUP_DIRECTORY, &path);
	if (ret < 0)
		return ret;

	ret = vfs_statfs(&path, &sbuf);
	if (ret < 0)
		goto out;

	ret = (int)min(sbuf.f_bavail * sbuf.f_bsize, int_max);

out:
	path_put(&path);
	return ret;
}

char *Get_SDCARD_LastFile(void)
{
	return g_last_log_fname;
}

/*
 *	Write log to file system
 */
static void WriteToLogDev_SDCARD(void)
{
	mm_segment_t oldfs;
	char fname[BCMLOG_OUTPUT_MAX_LOG_PATHNAME];

	oldfs = get_fs();
	set_fs(KERNEL_DS);

	/*
	 *      Attempt to open log file, if not already open
	 */
	if (!g_devWrParms.file) {
		if ((Get_SDCARD_Available()) > MTT_SD_RESERVED) {
			GetLogFileName(fname, BCMLOG_GetFileBase(),
				sizeof(fname));

			g_devWrParms.file =
			    filp_open(fname, O_WRONLY | O_TRUNC | O_CREAT,
				      0666);

			if (IS_ERR(g_devWrParms.file)) {
				g_devWrParms.file = 0;
			} else {
				unsigned long irql;

				/* record the latest SD log file name */
				irql = AcquireOutputLock();
				memcpy(g_last_log_fname, fname,
					BCMLOG_OUTPUT_MAX_LOG_PATHNAME);
				ReleaseOutputLock(irql);
			}
		} else if ((Get_SDCARD_Available()) > 0) {
			g_devWrParms.outdev = BCMLOG_OUTDEV_NONE;
			BCMLOG_SetRunlogDevice(BCMLOG_OUTDEV_NONE);
		} else {
			/*
			 * SD card not exist or not mounted yet.
			 */
		}
	}

	/*
	 *      If log file open start logging to it
	 */
	if (g_devWrParms.file) {
		u32 nFifo;

		do {
			int nWrite;

			nFifo = BCMLOG_FifoGetNumContig(&g_fifo);

			if (nFifo > MAX_FS_WRITE_SIZE)
				nFifo = MAX_FS_WRITE_SIZE;

			if (nFifo > 0) {
				nWrite =
				    g_devWrParms.file->f_op->write(g_devWrParms.
						   file,
						   BCMLOG_FifoGetData
						   (&g_fifo),
						   nFifo,
						   &g_devWrParms.
						   file->f_pos);

				if (nWrite > 0)
					BCMLOG_FifoRemove(&g_fifo, nWrite);

				if ((nWrite < nFifo)
				    || ((Get_SDCARD_Available()) <
					MTT_SD_RESERVED)) {
					nFifo = 0;
					filp_close(g_devWrParms.file, NULL);
					g_devWrParms.file = 0;
					/*
					 * in the case of sdcard, redirect
					 * output to 'null' as the card is
					 * full or removed at this point
					 */
					g_devWrParms.outdev =
					    BCMLOG_OUTDEV_NONE;
					BCMLOG_SetRunlogDevice
					    (BCMLOG_OUTDEV_NONE);
				} else if (g_devWrParms.file->f_pos >=
					   (BCMLOG_GetSdFileMax())) {

					/*
					 * file size reached maximum.
					 * close and open new file.
					 */
					nFifo = 0;
					filp_close(g_devWrParms.file, NULL);
					g_devWrParms.file = 0;
				}
			}
		} while (nFifo > 0);
	}

	set_fs(oldfs);
}
#ifdef CONFIG_BCM_STM
static void WriteToLogDev_STM(void)
{
	u32 nFifo;

	int nWrite;

	nFifo = BCMLOG_FifoGetNumContig(&g_fifo);

	if (nFifo > MAX_FS_WRITE_SIZE)
		nFifo = MAX_FS_WRITE_SIZE;

	if (nFifo > 0) {
		if (nFifo < BCMLOG_FifoGetDataSize(&g_fifo)) {
			stm_trace_buffer_start(FUSE_LOG_CHANNEL);
			nWrite = stm_trace_buffer_data(FUSE_LOG_CHANNEL,
						       BCMLOG_FifoGetData
						       (&g_fifo), nFifo);
			BCMLOG_FifoRemove(&g_fifo, nWrite);
			nFifo = BCMLOG_FifoGetNumContig(&g_fifo);
			nWrite = stm_trace_buffer_data(FUSE_LOG_CHANNEL,
						       BCMLOG_FifoGetData
						       (&g_fifo), nFifo);
			BCMLOG_FifoRemove(&g_fifo, nWrite);

			stm_trace_buffer_end(FUSE_LOG_CHANNEL);
		} else {
			nWrite = stm_trace_buffer_onchannel(FUSE_LOG_CHANNEL,
							    BCMLOG_FifoGetData
							    (&g_fifo), nFifo);
			BCMLOG_FifoRemove(&g_fifo, nWrite);
		}
	}
}
#endif

#ifdef BCMLOG_DEBUG_FLAG
static unsigned int nTotalout;

static void verifyRefCount(unsigned char *pBytes, unsigned long nBytes)
{
	static u32 i;
	static unsigned char frame_counter;
	static unsigned char old_frame_counter;
	static unsigned char firstsync = 1;

	unsigned char expect_counter;

	while (i < nBytes) {
		if (pBytes[i] == MTTLOG_FrameSync0 &&
		    pBytes[i + 1] == MTTLOG_FrameSync1) {
			if (firstsync) {
				frame_counter = old_frame_counter =
				    pBytes[i + 2];
				firstsync = 0;
			} else {
				frame_counter = pBytes[i + 2];
				expect_counter = old_frame_counter + 1;
				if (frame_counter != expect_counter) {
					pr_info
			    ("BCMLOG: RefCount Skip!!!Time %d: %d, expect %d\n",
					     (pBytes[i + 4] << 24) |
					     (pBytes[i + 5] << 16) |
					     (pBytes[i + 6] << 8) |
					     (pBytes[i + 7]), frame_counter,
					     expect_counter);
				}

				old_frame_counter = frame_counter;
			}

			i += (15 + (((unsigned short)(pBytes[i + 10])) << 8) +
			      (unsigned short)(pBytes[i + 11]));
		} else
			i++;
	}

	if (i >= nBytes)
		i -= nBytes;
}
#endif

static void WriteToLogDev_RNDIS(void)
{
#ifdef CONFIG_BRCM_NETCONSOLE

	if (g_netconsole_on) {
		u32 nFifo;

		do {
			int nWrite;
			unsigned char *pData;

			nFifo = BCMLOG_FifoGetNumContig(&g_fifo);

			if (nFifo > MAX_FS_WRITE_SIZE)
				nFifo = MAX_FS_WRITE_SIZE;

			if (nFifo > 0) {
				pData = BCMLOG_FifoGetData(&g_fifo);
				nWrite = brcm_klogging(pData, nFifo);

				if (nWrite > 0) {
#ifdef BCMLOG_DEBUG_FLAG
					verifyRefCount(pData, nWrite);
					nTotalout += nWrite;
#endif
					BCMLOG_FifoRemove(&g_fifo, nWrite);
				}

				if (nWrite < nFifo)
					nFifo = 0;
			}
		} while (nFifo > 0);
	}
#endif
}

static void WriteToLogDev_UART(void)
{
	mm_segment_t oldfs;

	oldfs = get_fs();
	set_fs(KERNEL_DS);

	/*
	 *      Attempt to open log file, if not already open
	 */

	if (!g_devWrParms.file) {
		g_devWrParms.file = filp_open(BCMLOG_GetUartDev(),
					      O_WRONLY | O_TRUNC | O_CREAT,
					      0666);

		if (IS_ERR(g_devWrParms.file))
			g_devWrParms.file = 0;
	}

	/*
	 *      If log file open start logging to it
	 */
	if (g_devWrParms.file) {
		u32 nFifo;

		do {
			int nWrite;

			nFifo = BCMLOG_FifoGetNumContig(&g_fifo);

			if (nFifo > MAX_FS_WRITE_SIZE)
				nFifo = MAX_FS_WRITE_SIZE;

			if (nFifo > 0) {
				nWrite =
					g_devWrParms.file->f_op->write(
					g_devWrParms.
					file,
					BCMLOG_FifoGetData
					(&g_fifo),
					nFifo,
					&g_devWrParms.
					file->f_pos);

				if (nWrite > 0)
					BCMLOG_FifoRemove(&g_fifo, nWrite);

				if (nWrite < nFifo) {
					nFifo = 0;
					filp_close(g_devWrParms.file, NULL);
					g_devWrParms.file = 0;
				}
			}
		} while (nFifo > 0);
	}

	set_fs(oldfs);
}

void WriteToLogDev_ACM(void)
{
	mm_segment_t oldfs;

	oldfs = get_fs();
	set_fs(KERNEL_DS);

	/*
	 *      Attempt to open log file, if not already open
	 */
	if (!g_devWrParms.file) {
		struct tty_struct *tty = NULL;

		g_devWrParms.file = filp_open(BCMLOG_GetAcmDev(),
					      O_WRONLY | O_TRUNC | O_CREAT,
					      0666);

		if (IS_ERR(g_devWrParms.file)) {
			pr_info("WriteToLogDev_ACM can not open %s\n",
				BCMLOG_GetAcmDev());
			g_devWrParms.file = 0;
			set_fs(oldfs);
			return;
		}

		tty = ((struct tty_file_private *)
		       g_devWrParms.file->private_data)->tty;
		tty->termios->c_iflag |= IGNBRK | ISTRIP | IGNPAR;
		tty->termios->c_oflag = 0;
		tty->termios->c_lflag = 0;
		tty->termios->c_cc[VERASE] = 0;
		tty->termios->c_cc[VKILL] = 0;
		tty->termios->c_cc[VMIN] = 1;
		tty->termios->c_cc[VTIME] = 0;

		g_devWrParms.acm_file = g_devWrParms.file;
	}

	/*
	 *      If log file open start logging to it
	 */
	if (g_devWrParms.file) {
		if (g_acm_on) {
			u32 nFifo;

			do {
				int nWrite;

				nFifo = BCMLOG_FifoGetNumContig(&g_fifo);

				if (nFifo > MAX_FS_WRITE_SIZE)
					nFifo = MAX_FS_WRITE_SIZE;

				if (nFifo > 0) {
					nWrite =
					    g_devWrParms.file->f_op->
					    write(g_devWrParms.file,
						  BCMLOG_FifoGetData(&g_fifo),
						  nFifo,
						  &g_devWrParms.file->f_pos);

					if (nWrite > 0)
						BCMLOG_FifoRemove(&g_fifo,
								  nWrite);

					if (nWrite < (int)nFifo) {
						pr_info
				    ("ACM failed to write log error:%d\n",
						     nWrite);
						nFifo = 0;
						filp_close(g_devWrParms.file,
							   NULL);
						g_devWrParms.file = 0;
					}
				}
			} while (nFifo > 0);
		}
	}

	set_fs(oldfs);
}

static void WriteToLogDev(struct work_struct *work)
{
	unsigned long irql;

	if (g_devWrParms.prev_outdev != g_devWrParms.outdev) {
		if (g_devWrParms.file) {
			filp_close(g_devWrParms.file, NULL);
			g_devWrParms.file = 0;
		}
	}

	switch (g_devWrParms.outdev) {
	case BCMLOG_OUTDEV_SDCARD:
		WriteToLogDev_SDCARD();
		break;

	case BCMLOG_OUTDEV_RNDIS:
		irql = AcquireOutputLock();
		WriteToLogDev_RNDIS();
		ReleaseOutputLock(irql);
		break;

	case BCMLOG_OUTDEV_UART:
		WriteToLogDev_UART();
		break;

	case BCMLOG_OUTDEV_ACM:
		WriteToLogDev_ACM();
		break;

#ifdef CONFIG_BCM_STM
	case BCMLOG_OUTDEV_STM:
		irql = AcquireOutputLock();
		WriteToLogDev_STM();
		ReleaseOutputLock(irql);
		break;
#endif

	default:
		break;
	}

	g_devWrParms.busy = 0;

}

unsigned long BCMLOG_GetFreeSize(void)
{
	return BCMLOG_FifoGetFreeSize(&g_fifo);
}

/**
 *	Output bytes to host
 *	@param  pBytes				(in)	pointer to user buffer
 *	@param	nBytes				(in)	number of bytes
 **/
void BCMLOG_Output(unsigned char *pBytes, unsigned long nBytes,
		   unsigned int might_has_mtthead)
{
	unsigned int i = 0;
	unsigned int wrotebyte;

#if defined(BCMLOG_LATEST_LOGS)
	/*
	Before adding to FIFO , check if FIFO is full,
	remove logs from fifo ( early logs) and make space
	for latest logs in fifo so we can keep latest logs.
	When this option enabled, we would loose early logs
	and would get latest logs as much as possible
	*/
	if (BCMLOG_FifoGetFreeSize(&g_fifo) < nBytes && (nBytes > 0))
		BCMLOG_FifoRemove(&g_fifo, nBytes);
#endif

	if (nBytes <= BCMLOG_FifoGetFreeSize(&g_fifo) && (nBytes > 0)) {
		if (might_has_mtthead) {
			while (i + 12 < nBytes) {
				if (pBytes[i] == MTTLOG_FrameSync0 &&
				    pBytes[i + 1] == MTTLOG_FrameSync1) {
					pBytes[i + 2] = g_frame_counter++;
					pBytes[i + 12] = (unsigned char)
					    MTTLOG_Checksum16(&pBytes[i], 12);
					i += (15 + (((unsigned short)
						     (pBytes[i + 10])) << 8)
					      + (unsigned short)
					      (pBytes[i + 11]));
				} else {
#ifdef BCMLOG_DEBUG_FLAG
					noheader++;
#endif
					break;
				}
			}
		}

		wrotebyte = BCMLOG_FifoAdd(&g_fifo, pBytes, nBytes);

#ifdef BCMLOG_DEBUG_FLAG
		if (wrotebyte != nBytes)
			writefail++;
		totalbyte += wrotebyte;
#endif
	}
#ifdef BCMLOG_DEBUG_FLAG

	else if (g_netconsole_on)
		nospace++;

	n100++;
	if (n100 == 100) {
		n100 = 0;
		pr_info("nospc %d nohdr %d splt %d wrtfl %d nosigbuf %d\n",
			nospace, noheader, splitbuffer, writefail,
			g_malloc_sig_buf);
		pr_info("IN %d OUT %d dif %d inFIFO %d\n",
			totalbyte, nTotalout, totalbyte - nTotalout,
			BCMLOG_FifoGetDataSize(&g_fifo));
	}
#endif
	/*
	 *      If output worker thread is not busy then set up arguments and
	 *      schedule it; otherwise we'll retry on the next output to run
	 *      the thread.
	 */
	if (!g_devWrParms.busy) {
		g_devWrParms.busy = 1;
		g_devWrParms.prev_outdev = g_devWrParms.outdev;
		g_devWrParms.outdev = BCMLOG_GetRunlogDevice();
		schedule_work(&g_devWrParms.wq);
	}
}

/**
 *	Initialize output module
 **/
BCMLOG_Fifo_t *BCMLOG_OutputInit(unsigned char *buffer, int buffer_size)
{
	BCMLOG_FifoInit(&g_fifo, buffer, buffer_size);

	g_devWrParms.busy = 0;
	g_devWrParms.file = 0;

	INIT_WORK(&g_devWrParms.wq, WriteToLogDev);
#ifdef CONFIG_BRCM_NETCONSOLE

	/*
	 *      register flow control callback functions
	 */
	g_netconsole_on = brcm_netconsole_register_callbacks(&_cb);

#endif
	return &g_fifo;

}
