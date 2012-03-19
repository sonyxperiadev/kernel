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

#ifdef CONFIG_BRCM_NETCONSOLE

/*
 *	forward declarations
 */
static int netconsole_start_cb(void);
static int netconsole_stop_cb(void);

/**
 *	local vars
 **/
/* flow control state for RNDIS, set/reset by flow control callbacks */
static char g_netconsole_on;

#endif
/* flow control state for ACM, set/reset by flow control callbacks */
static char g_acm_on = 1;

#define BCMLOG_OUTPUT_FIFO_MAX_BYTES 65536
static BCMLOG_Fifo_t g_fifo;	/* output fifo */
/* 23MB reserved area for AP/CP crash dump */
#define MTT_SD_RESERVED (23 * 1024 * 1024)
/**
 *	frame counter
 **/
static unsigned char g_frame_counter;

struct acm_callbacks {
	/** Start function for role change */
	int (*start) (void);
	/** Stop Function for role change */
	int (*stop) (void);
};

struct WriteToLogDevParms_t {
	struct workqueue_struct *wq;
	struct work_struct work;
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
#define MAX_LOG_PATHNAME     64

/*
 *	Return available SD card size (bytes)
 *	If over 2G, return 2G
 */
static int Get_SDCARD_Available(void)
{
	struct kstatfs sbuf;
	struct path path;
	int ret;
	u64 int_max = INT_MAX;

	ret = kern_path("/sdcard/", LOOKUP_DIRECTORY, &path);
	if (ret < 0)
		goto out;

	ret = vfs_statfs(&path, &sbuf);
	if (ret < 0)
		goto out;

	ret = (int)min(sbuf.f_bavail * sbuf.f_bsize, int_max);

out:
	return ret;
}

/*
 *	Write log to file system
 */
static void WriteToLogDev_SDCARD(void)
{
	mm_segment_t oldfs;
	char fname[MAX_LOG_PATHNAME];

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

			if (IS_ERR(g_devWrParms.file))
				g_devWrParms.file = 0;
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
				nWrite = g_devWrParms.file->f_op->write
				    (g_devWrParms.file,
				     BCMLOG_FifoGetData(&g_fifo),
				     nFifo, &g_devWrParms.file->f_pos);

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

static void WriteToLogDev_STM(void)
{
	u32 nFifo;

	int nWrite;

	nFifo = BCMLOG_FifoGetNumContig(&g_fifo);

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

static void WriteToLogDev_CUSTOM(void)
{
	u32 nFifo;

	int nWrite;

	nFifo = BCMLOG_FifoGetNumContig(&g_fifo);

	if (nFifo > 0) {
		if (nFifo < BCMLOG_FifoGetDataSize(&g_fifo)) {
			nWrite =
			    BCMLOG_CallHandler(BCMLOG_CUSTOM_RUN_LOG,
					       BCMLOG_FifoGetData(&g_fifo),
					       nFifo, BCMLOG_CUSTOM_START);
			BCMLOG_FifoRemove(&g_fifo, nWrite);
			nFifo = BCMLOG_FifoGetNumContig(&g_fifo);

			while ((nFifo < BCMLOG_FifoGetDataSize(&g_fifo))
			       && (nWrite)) {
				nWrite =
				    BCMLOG_CallHandler(BCMLOG_CUSTOM_RUN_LOG,
						       BCMLOG_FifoGetData
						       (&g_fifo), nFifo,
						       BCMLOG_CUSTOM_DATA);
				BCMLOG_FifoRemove(&g_fifo, nWrite);
				nFifo = BCMLOG_FifoGetNumContig(&g_fifo);
			}

			nFifo = BCMLOG_FifoGetNumContig(&g_fifo);
			nWrite =
			    BCMLOG_CallHandler(BCMLOG_CUSTOM_RUN_LOG,
					       BCMLOG_FifoGetData(&g_fifo),
					       nFifo, BCMLOG_CUSTOM_END);
			BCMLOG_FifoRemove(&g_fifo, nWrite);
		} else {
			nWrite =
			    BCMLOG_CallHandler(BCMLOG_CUSTOM_RUN_LOG,
					       BCMLOG_FifoGetData(&g_fifo),
					       nFifo, BCMLOG_CUSTOM_COMPLETE);
			BCMLOG_FifoRemove(&g_fifo, nWrite);
		}
	}
}

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

				if (nWrite > 0)
					BCMLOG_FifoRemove(&g_fifo, nWrite);

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
		g_devWrParms.file =
		    filp_open(BCMLOG_GetUartDev(), O_WRONLY |
				O_TRUNC | O_CREAT, 0666);

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
				nWrite = g_devWrParms.file->f_op->write
				    (g_devWrParms.file,
				     BCMLOG_FifoGetData(&g_fifo),
				     nFifo, &g_devWrParms.file->f_pos);

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

static void WriteToLogDev_ACM(void)
{
	mm_segment_t oldfs;

	oldfs = get_fs();
	set_fs(KERNEL_DS);

	/*
	 *      Attempt to open log file, if not already open
	 */
	if (!g_devWrParms.file) {
		struct tty_struct *tty = NULL;

		g_devWrParms.file =
		    filp_open(BCMLOG_GetAcmDev(), O_WRONLY | O_TRUNC | O_CREAT,
			      0666);

		if (IS_ERR(g_devWrParms.file)) {
			pr_info("can not open %s\n", BCMLOG_GetAcmDev());
			g_devWrParms.file = 0;
			return;
		}

		tty =
		    ((struct tty_file_private *)g_devWrParms.file->
		     private_data)->tty;
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

					if (nWrite < nFifo) {
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
		if (g_devWrParms.file
		    && g_devWrParms.file != g_devWrParms.acm_file) {
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

	case BCMLOG_OUTDEV_STM:
		irql = AcquireOutputLock();
		WriteToLogDev_STM();
		ReleaseOutputLock(irql);
		break;

	case BCMLOG_OUTDEV_CUSTOM:
		irql = AcquireOutputLock();
		WriteToLogDev_CUSTOM();
		ReleaseOutputLock(irql);
		break;

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

	/*
	 *      If log loss happened and output buffer available,
	 *      put the log loss message to the buffer.
	 */
	if ((BCMLOG_IsLogError()) && (might_has_mtthead)) {
		unsigned int msg_size;
		char *log_loss_buf;

		msg_size = BCMMTT_GetRequiredFrameLength
		    ((((unsigned short)(pBytes[i + 10])) << 8) +
		     (unsigned short)(pBytes[i + 11]));
		msg_size = (((msg_size) > (nBytes)) ? (msg_size) : (nBytes));

		if (msg_size <= BCMLOG_FifoGetFreeSize(&g_fifo)) {
			log_loss_buf =
			    kmalloc(BCMMTT_GetRequiredFrameLength
				    (BCMLOG_LOGLOSS_SIZE), GFP_ATOMIC);
			if (log_loss_buf) {
				msg_size =
				    BCMLOG_BuildLogLossMessage(log_loss_buf);
				if (msg_size) {
					log_loss_buf[2] = g_frame_counter++;
					log_loss_buf[12] = (unsigned char)
					    MTTLOG_Checksum16(log_loss_buf, 12);
					wrotebyte =
					    BCMLOG_FifoAdd(&g_fifo,
							   log_loss_buf,
							   msg_size);
					if (wrotebyte == msg_size)
						BCMLOG_ClearLogError();
				}
				kfree(log_loss_buf);
			}
		}
	}

	/*
	 *      Add to FIFO.
	 *      If unable to add (FIFO full) then discard the message.
	 *      Update log loss counter.
	 */
	if (nBytes <= BCMLOG_FifoGetFreeSize(&g_fifo) && (nBytes > 0)) {
		if (might_has_mtthead) {
			while (i + 12 < nBytes) {
				if (pBytes[i] == MTTLOG_FrameSync0
				    && pBytes[i + 1] == MTTLOG_FrameSync1) {
					pBytes[i + 2] = g_frame_counter++;
					pBytes[i + 12] = (unsigned char)
					    MTTLOG_Checksum16(&pBytes[i], 12);
					i += (15 +
					      (((unsigned
						 short)(pBytes[i + 10])) << 8) +
					      (unsigned short)(pBytes[i + 11]));
				} else {
					BCMLOG_RecordLogError
					    (INITLOG_CONFLICT_ONCE);
					break;
				}
			}
		}

		wrotebyte = BCMLOG_FifoAdd(&g_fifo, pBytes, nBytes);

		if (wrotebyte != nBytes)
			BCMLOG_RecordLogError(INITLOG_CONFLICT_ONCE);
	} else {
		if (might_has_mtthead) {
			while (i + 12 < nBytes) {
				if (pBytes[i] == MTTLOG_FrameSync0
				    && pBytes[i + 1] == MTTLOG_FrameSync1) {
					BCMLOG_RecordLogError(SIOBUF_FULL_ONCE);
					i += (15 +
					      (((unsigned
						 short)(pBytes[i + 10])) << 8) +
					      (unsigned short)(pBytes[i + 11]));
				} else {
					BCMLOG_RecordLogError
					    (INITLOG_CONFLICT_ONCE);
					break;
				}
			}
		}
	}
	/*
	 *      If output worker thread is not busy then set up arguments and
	 *      schedule it; otherwise we'll retry on the next output to run
	 *      the thread.
	 */
	if (!g_devWrParms.busy) {
		g_devWrParms.busy = 1;
		g_devWrParms.prev_outdev = g_devWrParms.outdev;
		g_devWrParms.outdev = BCMLOG_GetRunlogDevice();
		queue_work(g_devWrParms.wq, &g_devWrParms.work);
	}
}

/**
 *	Initialize output module
 **/
int BCMLOG_OutputInit(void)
{
	unsigned char *fifobuf;

	fifobuf = kmalloc(BCMLOG_OUTPUT_FIFO_MAX_BYTES, GFP_KERNEL);

	if (!fifobuf)
		return -1;

	BCMLOG_FifoInit(&g_fifo, fifobuf, BCMLOG_OUTPUT_FIFO_MAX_BYTES);

	g_devWrParms.busy = 0;
	g_devWrParms.file = 0;

	INIT_WORK(&g_devWrParms.work, WriteToLogDev);
	g_devWrParms.wq = alloc_workqueue("fuse_log_work", WQ_FREEZABLE, 1);
	if (!g_devWrParms.wq) {
		printk(KERN_ERR "Failed to init fuse log workqueue\n");
		return -ENOMEM;
	}
#ifdef CONFIG_BRCM_NETCONSOLE

	/*
	 *      register flow control callback functions
	 */
	g_netconsole_on = brcm_netconsole_register_callbacks(&_cb);

#endif
	return 0;

}

void BCMLOG_OutputExit(void)
{
	destroy_workqueue(g_devWrParms.wq);
}
