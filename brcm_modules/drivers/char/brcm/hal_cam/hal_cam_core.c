/*******************************************************************************************
 * Copyright 2010 Broadcom Corporation.  All rights reserved.
 *
 * Unless you and Broadcom execute a separate written software license agreement
 * governing use of this software, this software is licensed to you under the
 * terms of the GNU General Public License version 2, available at
 * http://www.gnu.org/copyleft/gpl.html (the "GPL").
 *
 * Notwithstanding the above, under no circumstances may you combine this software
 * in any way with any other Broadcom software provided under a license other than
 * the GPL, without Broadcom's express prior written consent.
 * *******************************************************************************************/

/*
 * *
 * *****************************************************************************
 * *
 * *  camera.c
 * *
 * *  PURPOSE:
 * *
 * *     This implements the driver for the Sanyo IGT99268 CCD camera.
 * *
 * *  NOTES:
 * *
 * *****************************************************************************/

/* ---- Include Files ---------------------------------------------------- */

#include <linux/version.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/poll.h>
#include <linux/sysctl.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/gpio.h>

#include <mach/reg_camera.h>
#include <mach/reg_lcd.h>

#include <mach/reg_clkpwr.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/atomic.h>

#include <linux/semaphore.h>
#include <linux/broadcom/types.h>
#include <linux/broadcom/bcm_major.h>
#include <linux/broadcom/hw_cfg.h>
#include <linux/broadcom/hal_camera.h>
#include <linux/broadcom/lcd.h>
#include <linux/broadcom/bcm_sysctl.h>
#include <plat/dma.h>
#include <linux/dma-mapping.h>
#ifdef CONFIG_HAS_WAKELOCK
#include <linux/wakelock.h>
#endif
#include <linux/clk.h>
#include <mach/clkmgr.h>
#include <linux/regulator/consumer.h>

#include "hal_cam.h"
#include "hal_cam_drv.h"
#include "cam_cntrl_2153.h"
#include "camdrv_dev.h"

/* FIXME: Clear VSYNC interrupt explicitly until handler properly */
#define CLEAR_VSYNC_INTR()   writel(1 << IRQ_VSYNC, HW_IRQ_BASE + 0x0008)
#define CAM_BOOT_TIME_MEMORY_SIZE (1024*1024*2)
#define CAM_SEC_OFFSET (1024*1024*3/2)
#define CAM_NUM_VFVIDEO 4
#define XFR_SIZE_MAX (4095*4)
#define XFR_SIZE (4095)
#define BYTES_PER_LLI   16/* number of bytes per Linked List Item (LLI)*/
#define INTS_PER_LLI            (BYTES_PER_LLI/4)
#define MAX_QUEUE_SIZE 4
#define MAX_QUEUE_SIZE_MASK (MAX_QUEUE_SIZE - 1)
#define SWAP(val)   cpu_to_le32(val)
#define SKIP_STILL_FRAMES 3
#define IF_NAME             "cami2c"
#define I2C_DRIVERID_CAM    0xf000
#define DMA_FROM_CAM_CONFIG                      \
    (REG_DMA_CHAN_CFG_TC_INT_ENABLE             \
    | REG_DMA_CHAN_CFG_ERROR_INT_ENABLE          \
    | REG_DMA_CHAN_CFG_FLOW_CTL_PRF_TO_MEM_DMA   \
    | (REG_DMA_PERIPHERAL_CAMERA << REG_DMA_CHAN_CFG_SRC_PERIPHERAL_SHIFT) \
    | REG_DMA_CHAN_CFG_ENABLE)

#define DMA_FROM_CAM_CONTROL                \
    (REG_DMA_CHAN_CTL_DEST_INCR            \
    | REG_DMA_CHAN_CTL_SRC_INCR             \
    | REG_DMA_CHAN_CTL_DEST_WIDTH_32        \
    | REG_DMA_CHAN_CTL_SRC_WIDTH_32         \
    | REG_DMA_CHAN_CTL_DEST_BURST_SIZE_32   \
    | REG_DMA_CHAN_CTL_SRC_BURST_SIZE_32    \
    )

#define CHECK_RES(x) {if ((x) != 0) \
			printk(KERN_ERR"Calling x error\n"); \
		     }

#define REFCAPTIME 80000000
struct clk *cam_clk;
/* Camera driver generic data. This contains camera variables that are common
 * across all instances */
enum camera_state {
	CAM_OFF,
	CAM_INIT,
	CAM_PAUSE,
	CAM_ON,
	CAM_STOPPING,
};

enum capture_mode {
	CAM_NONE,
	CAM_STILL,
	CAM_STREAM,
};

struct CAM_DATA{
	int vsyncIrqs;
	int dmaCam;
	int level;
	int blocked;

};

struct CAM_DATA gCamState;

static struct ctl_table gSysCtlCam[] = {
	{
	 .ctl_name = BCM_SYSCTL_CAM_LEVEL,
	 .procname = "level",
	 .data = &gCamState.level,
	 .maxlen = sizeof(int),
	 .mode = 0644,
	 .proc_handler = &proc_dointvec},
	{
	 .ctl_name = BCM_SYSCTL_CAM_VSYNCIRQS,
	 .procname = "vsyncIrqs",
	 .data = &gCamState.vsyncIrqs,
	 .maxlen = sizeof(int),
	 .mode = 0644,
	 .proc_handler = &proc_dointvec},
	{
	 .ctl_name = BCM_SYSCTL_CAM_DMA_CAM,
	 .procname = "dmaCam",
	 .data = &gCamState.dmaCam,
	 .maxlen = sizeof(int),
	 .mode = 0644,
	 .proc_handler = &proc_dointvec},
	{
	 .ctl_name = BCM_SYSCTL_CAM_BLOCKED,
	 .procname = "blocked",
	 .data = &gCamState.blocked,
	 .maxlen = sizeof(int),
	 .mode = 0644,
	 .proc_handler = &proc_dointvec},
	{}
};

static struct ctl_table gSysCtl[] = {
	{
	 .ctl_name = CTL_BCM_CAM,
	 .procname = "camera",
	 .mode = 0555,
	 .child = gSysCtlCam},
	{}
};

struct cam_dma_buf_t {
	size_t sizeInBytes;
	void *virt;
	dma_addr_t phy;
};
struct cam_i2c_info {
	struct i2c_client *client;
};

struct buf_q {
	CAM_BufData data[MAX_QUEUE_SIZE];
	unsigned int ReadIndex;
	unsigned int WriteIndex;
	spinlock_t lock;
	struct semaphore Sem;
	bool isActive;
	bool isWaitQueue;
};

struct camera_sensor_t {
	CamIntfConfig_st_t *sensor_intf;
	struct sens_methods *sens_m;
	CAM_Parm_t main;
	/* Either Still or VF/Video but not both at the same time.*/
	CAM_Parm_t th;
	/* Represents thumbnail only valid with still cap*/
	enum capture_mode mode;
	enum camera_state state;
	struct cam_dma_buf_t camb[CAM_NUM_VFVIDEO];
	struct cam_dma_buf_t cam_ll[CAM_NUM_VFVIDEO];
	atomic_t captured;
	int sCaptureFrameCountdown;
	int dma_chan;
	int still_ready;
	short *framebuf;
	int gProcessFrameRunning;
	int sCaptureJpegSize;
	spinlock_t c_lock;
	struct buf_q rd_Q;
	struct buf_q wr_Q;
	CAM_BufData gCurrData;
	struct cam_i2c_info *cam_i2c_datap;
	ktime_t prev;
};

struct cam_generic_t {
	struct class *cam_class;
	struct regulator *cam_regulator;
#ifdef CONFIG_HAS_WAKELOCK
	struct wake_lock camera_wake_lock;
#endif
	struct ctl_table_header *gSysCtlHeader;
	CamSensorSelect_t curr;
	struct cam_dma_buf_t cam_buf;
	struct cam_dma_buf_t cam_buf_main;
	struct semaphore cam_sem;
	struct camera_sensor_t sens[2];
};

static struct cam_generic_t *cam_g;
DECLARE_WAIT_QUEUE_HEAD(gDataReadyQ);
static char banner[] __initdata =
    KERN_INFO "Camera Driver: 1.00 (built on " __DATE__ " " __TIME__ ")\n";
/* Module static functions prototype begin*/
static int camera_enable(CamSensorSelect_t sensor);
static int camera_disable(CamSensorSelect_t sensor);
static int process_frame(CamSensorSelect_t sensor);
static int cam_power_down(CamSensorSelect_t sensor);
static int cam_power_up(CamSensorSelect_t sensor);
static int cam_open(struct inode *inode, struct file *file);
static int cam_release(struct inode *inode, struct file *file);
static int cam_ioctl(struct inode *inode, struct file *file, unsigned int cmd,
		     unsigned long arg);

static void push_queue(struct buf_q *queue, CAM_BufData * buf);
static void deinit_queue(struct buf_q *queue);
static void init_queue(struct buf_q *queue);
static bool pull_queue(struct buf_q *queue, CAM_BufData * buf);
static void wakeup_push_queue(struct buf_q *queue, CAM_BufData * buf);
static bool wait_pull_queue(struct buf_q *queue, CAM_BufData * buf);

extern struct sens_methods *CAMDRV_primary_get(void);
/* Module static functions prototype end*/
static int cam_ioctl(struct inode *inode, struct file *file, unsigned int cmd,
		     unsigned long arg)
{
	int sensor = 0;
	int ret;
	int rc = 0;
	switch (cmd) {
	case CAM_IOCTL_ENABLE:
		pr_debug(KERN_INFO "Cam Enable called %d\n", (int)arg);
		if (arg)
			camera_enable(sensor);
		else
			camera_disable(sensor);
		break;
	case CAM_IOCTL_SET_PARAMS:
		{
			CAM_Parm_t parm;
			ret =
			    copy_from_user(&parm, (CAM_Parm_t *) arg,
					   sizeof(parm));
			if (ret != 0)
				return ret;
			cam_g->sens[sensor].main = parm;
	/* Old code calls CAM_Enable over here .... Not needed for this */
			break;
		}
	case CAM_IOCTL_SET_FPS:
		cam_g->sens[sensor].main.fps = (CamRates_t) arg;
		if (cam_g->sens[sensor].sens_m->
			DRV_SetFrameRate((CamRates_t) arg,
					   sensor) != HAL_CAM_SUCCESS) {
			rc = -EFAULT;
			pr_debug
				("CAM_IOCTL_SET_FPS: CAMDRV_SetFrameRate(): ERROR: \r\n");
		}
		break;



		
		/* Set it with the next enable */
		break;
	case CAM_IOCTL_SET_THUMBNAIL_PARAMS:
		copy_from_user(&cam_g->sens[sensor].th, (void *)arg,
			       sizeof(CAM_Parm_t));
		break;
	case CAM_IOCTL_MEM_REGISTER:
		{
			CAM_BufData p;
			copy_from_user(&p, (CAM_BufData *) arg,
				       sizeof(CAM_BufData));
			cam_g->cam_buf.phy = (dma_addr_t) p.busAddress;
			break;
		}
	case CAM_IOCTL_MEM_BUFFERS:
		{
			CAM_BufData buf;
			copy_from_user(&buf, (CAM_BufData *) arg,
				       sizeof(CAM_BufData));
			if (buf.busAddress ==
			    (void *)cam_g->sens[sensor].camb[buf.id].phy)
				push_queue(&cam_g->sens[sensor].wr_Q, &buf);
			else
				ret = -EFAULT;
			break;
		}
	case CAM_IOCTL_GET_FRAME:
		{
			CAM_BufData buf;
			/* For VF and Video */
			wait_pull_queue(&cam_g->sens[sensor].rd_Q, &buf);
			copy_to_user((void *)arg, &buf, sizeof(buf));
			break;
		}
	case CAM_IOCTL_GET_FRAME1:
		{
			/* Block for still capture */
			CAM_Frame1_t frame;
			int copy = 0;
			if (copy_from_user
			    (&frame, (CAM_Frame1_t *) arg,
			     sizeof(CAM_Frame1_t)) != 0)
				ret = -EFAULT;
			else {
				if (frame.len)
					copy = 1;
				process_frame(sensor);
				if (cam_g->sens[sensor].sCaptureJpegSize)
					frame.len = (cam_g->sens[sensor].sCaptureJpegSize + 1) >> 1;
				if (copy) {
					/*copy_to_user();*/
				}
				copy_to_user((CAM_Frame1_t *) arg, &frame,
					     sizeof(frame));
			}
			break;
		}
	case CAM_IOCTL_GET_JPEG:
		{
			CAM_Frame1_t frame;
			int length;
			short *fbuf;
			if (0 !=
			    (copy_from_user
			     (&frame, (void *)arg, sizeof(CAM_Frame1_t)))) {
				return -EINVAL;
			}
			length = (cam_g->sens[sensor].sCaptureJpegSize + 1) >> 1;
			if (!frame.buffer || (frame.len < length)) {
				printk(KERN_ERR "Error 0x%x length %d\n",
				       (u32) frame.buffer, frame.len);
				return -EINVAL;
			}
			frame.len = length;
			fbuf =
			    cam_g->sens[sensor].sens_m->DRV_GetJpeg(cam_g->
								    sens
								    [sensor].
								    framebuf);
			if (copy_to_user
			    (frame.buffer, fbuf,
			     frame.len * sizeof(unsigned short)) != 0)
				frame.len = 0;
			if (copy_to_user
			    ((CAM_Frame1_t *) arg, &frame, sizeof(frame)) != 0)
				return -EFAULT;
			break;
		}
	case CAM_IOCTL_GET_THUMBNAIL:
		{
			CAM_Frame1_t frame;
			char *thumbnail;
			int length;
			struct camera_sensor_t *c = &cam_g->sens[sensor];
			if (copy_from_user
			    (&frame, (CAM_Frame1_t *) arg,
			     sizeof(frame)) != 0) {
				return -EFAULT;
			}
			length = c->th.size_window.end_pixel * c->th.size_window.end_line;
			if (!frame.buffer || (frame.len < length)) {
				return -EFAULT;
			}
			frame.len = length;
			thumbnail =
			    cam_g->sens[sensor].sens_m->DRV_GetThumbnail(c->
									 framebuf,
									 c->
									 sCaptureJpegSize);
			if (copy_to_user
			    (frame.buffer, thumbnail,
			     frame.len * sizeof(unsigned short)) != 0)
				frame.len = 0;

			if (copy_to_user
			    ((CAM_Frame1_t *) arg, &frame, sizeof(frame)) != 0)
				return -EFAULT;
			break;
		}
	case CAM_IOCTL_SET_DIGITAL_EFFECT:
		{
			CAM_Parm_t parm;
			if (copy_from_user
			    (&parm, (CAM_Parm_t *) arg, sizeof(parm)) != 0) {
				rc = -EFAULT;
				break;
			}
			if (cam_g->sens[sensor].sens_m->
			    DRV_SetDigitalEffect(parm.coloreffects,
						 sensor) != HAL_CAM_SUCCESS)
				rc = -EFAULT;
			break;
		}
	case CAM_IOCTL_SET_SCENE_MODE:
		{
			CAM_Parm_t parm;
			if (copy_from_user
			    (&parm, (CAM_Parm_t *) arg, sizeof(parm)) != 0) {
				rc = -EFAULT;
				break;
			}
			if (cam_g->sens[sensor].sens_m->
			    DRV_SetSceneMode(parm.scenemode,
					     sensor) != HAL_CAM_SUCCESS) {
				rc = -EFAULT;
				pr_debug
				    ("CAM_IOCTL_SET_SCENE_MODE: CAMDRV_SetSceneMode(): ERROR: \r\n");
			}
			break;
		}
	case CAM_IOCTL_SET_WB:
		{
			CAM_Parm_t parm;
			if (copy_from_user
			    (&parm, (CAM_Parm_t *) arg, sizeof(parm)) != 0) {
				rc = -EFAULT;
				break;
			}
			if (cam_g->sens[sensor].sens_m->
			    DRV_SetWBMode(parm.wbmode,
					  sensor) != HAL_CAM_SUCCESS) {
				rc = -EFAULT;
				pr_debug
				    ("CAM_IOCTL_SET_WB: CAMDRV_SetWBMode(): ERROR: \r\n");
			}
			break;
		}
	case CAM_IOCTL_SET_ANTI_BANDING:
		{
			CAM_Parm_t parm;
			if (copy_from_user
			    (&parm, (CAM_Parm_t *) arg, sizeof(parm)) != 0) {
				rc = -EFAULT;
				break;
			}
			if (cam_g->sens[sensor].sens_m->
			    DRV_SetAntiBanding(parm.antibanding,
					       sensor) != HAL_CAM_SUCCESS) {
				rc = -EFAULT;
				pr_debug
				    ("CAM_IOCTL_SET_ANTI_BANDING: CAMDRV_SetAntiBanding(): ERROR: \r\n");
			}
			break;
		}
	case CAM_IOCTL_SET_FLASH_MODE:
		{
			CAM_Parm_t parm;
			if (copy_from_user
			    (&parm, (CAM_Parm_t *) arg, sizeof(parm)) != 0) {
				rc = -EFAULT;
				break;
			}
			if (cam_g->sens[sensor].sens_m->
			    DRV_SetFlashMode(parm.focus,
					     sensor) != HAL_CAM_SUCCESS) {
				rc = -EFAULT;
				pr_debug
				    ("CAM_IOCTL_SET_FLASH_MODE: CAMDRV_SetFlashMode(): ERROR: \r\n");
			}
			break;
		}
	case CAM_IOCTL_SET_FOCUS_MODE:
		{
			CAM_Parm_t parm;
			if (copy_from_user
			    (&parm, (CAM_Parm_t *) arg, sizeof(parm)) != 0) {
				rc = -EFAULT;
				break;
			}
			if (cam_g->sens[sensor].sens_m->
			    DRV_SetFocusMode(parm.focus,
					     sensor) != HAL_CAM_SUCCESS) {
				rc = -EFAULT;
				pr_debug
				    ("CAM_IOCTL_SET_FOCUS_MODE: CAMDRV_SetFocusMode(): ERROR: \r\n");
			}
			break;
		}
	case CAM_IOCTL_ENABLE_AUTOFOCUS:
		{
			if (cam_g->sens[sensor].sens_m->
			    DRV_TurnOnAF(sensor) != HAL_CAM_SUCCESS) {
				rc = -EFAULT;
				pr_debug
				    ("CAM_IOCTL_ENABLE_AUTOFOCUS: CAMDRV_TurnOnAF(): ERROR: \r\n");
			}
			break;
		}
	case CAM_IOCTL_DISABLE_AUTOFOCUS:
		{
			if (cam_g->sens[sensor].sens_m->
			    DRV_TurnOffAF(sensor) != HAL_CAM_SUCCESS) {
				rc = -EFAULT;
				pr_debug
				    ("CAM_IOCTL_DISABLE_AUTOFOCUS: CAMDRV_TurnOffAF(): ERROR: \r\n");
			}
			break;
		}
	case CAM_IOCTL_SET_JPEG_QUALITY:
		{
			CAM_Parm_t parm;
			if (copy_from_user
			    (&parm, (CAM_Parm_t *) arg, sizeof(parm)) != 0) {
				rc = -EFAULT;
				break;
			}
			if (cam_g->sens[sensor].sens_m->
			    DRV_SetJpegQuality(parm.quality,
					       sensor) != HAL_CAM_SUCCESS) {
				rc = -EFAULT;
				pr_debug
				    ("CAM_IOCTL_SET_JPEG_QUALITY: CAMDRV_SetJpegQuality(): ERROR: \r\n");
			}
			break;
		}
	case CAM_IOCTL_SET_ZOOM:
		{
			CAM_Parm_t parm;
			if (copy_from_user
			    (&parm, (CAM_Parm_t *) arg, sizeof(parm)) != 0) {
				rc = -EFAULT;
				break;
			}
			if (cam_g->sens[sensor].sens_m->
			    DRV_SetZoom(parm.zoom,
					       sensor) != HAL_CAM_SUCCESS) {
				rc = -EFAULT;
				pr_debug
				    ("CAM_IOCTL_SET_ZOOM: CAMDRV_SetZoom(): ERROR: \r\n");
			}
			break;
		}

	default:
		break;
	}
	return rc;
}

static int process_frame(CamSensorSelect_t sensor)
{
	struct camera_sensor_t *c = &cam_g->sens[sensor];

	/*  processFrame is not a kernel thread anymore
	 *  Just a regular blocking function invoked from process context
	 *  Since process context can sleep, we don't see any need for a separate
	 *  kernel thread to carry-out JPEG processing */

	/* While(1) and break is required as the STV sensor operates that way */
	while (1) {
	/* Block waiting to be signalled by either VSYNC ISR or camera disable */
		if (0 == wait_event_interruptible(gDataReadyQ, c->still_ready)) {
			c->still_ready = 0;
/* A Valid wake-up by VSYNC ISR has gProcessFrameRunning = 1.
 * The disable method wakes up with gProcessFrameRunning = 0 which means exit */
			if (!c->gProcessFrameRunning)
				break;
			atomic_set(&c->captured, 1);
			c->sCaptureJpegSize =
			    c->sens_m->DRV_GetJpegSize(sensor, c->camb[0].virt);
#if defined(CONFIG_BCM_CAM_STV0986)
			if (c->sCaptureFrameCountdown >= -15) {
				if (Cam_MarkerSearch(c->camb[0].virt)) {
					c->sCaptureJpegSize = 0;
					atomic_set(&c->captured, 0);
					continue;
				}
			}
#endif
			c->framebuf = (short *)c->camb[0].virt;
			atomic_set(&c->captured, 0);
			break;
		}
	}
	return 0;
}

#if defined(CONFIG_BCM_CAM_STV0986)
static int Cam_MarkerSearch(void *buf)
{
	char *ptr = (char *)buf;

	/* check SOI ( 0xFF 0xD8 ) */
	if ((*ptr != 0xFF) || (*(ptr + 1) != 0xD8))
		return -1;

	/* check EOI ( 0xFF 0xD9 ) */
	if ((*(ptr + sCaptureJpegSize - 2) != 0xFF)
	    || (*(ptr + sCaptureJpegSize - 1) != 0xD9))
		return -1;

	return 0;
}
#endif

static struct file_operations cam_fops = {
owner:  THIS_MODULE,
open :	cam_open,
release : cam_release,
ioctl :	cam_ioctl,
};

/* Queue handling functions begin */
static int64_t systemTime(void)
{
	struct timespec t;
	t.tv_sec = t.tv_nsec = 0;
	ktime_get_ts(&t);
	return (int64_t) (t.tv_sec) * 1000000000LL + t.tv_nsec;
}

static void init_queue(struct buf_q *queue)
{
	if (queue) {
		if (queue->isActive)
			return;
		queue->ReadIndex = 0;
		queue->WriteIndex = 0;
		sema_init(&queue->Sem, 0);
		spin_lock_init(&queue->lock);
		queue->isActive = true;
		queue->isWaitQueue = false;
	} else {
	}
	/*
	   g_CurrData.id =-1;
	   g_CurrData.busAddress =NULL;
	   g_CurrData.timestamp=0;
	 */
}

static void deinit_queue(struct buf_q *queue)
{
	unsigned long stat;
	if (queue) {
		if (!queue->isActive)
			return;

		spin_lock_irqsave(&queue->lock, stat);
		queue->ReadIndex = 0;
		queue->WriteIndex = 0;
		sema_init(&queue->Sem, 1);
		spin_unlock_irqrestore(&queue->lock, stat);
		queue->isActive = false;
		queue->isWaitQueue = false;
	}
}

static void push_queue(struct buf_q *queue, CAM_BufData * buf)
{
	unsigned int NextIndex = 0;
	unsigned long stat;
	if (!queue)
		return;
	if (!queue->isActive)
		return;
	if (queue->isWaitQueue)
		return;

	spin_lock_irqsave(&queue->lock, stat);

	NextIndex = (queue->WriteIndex + 1) & MAX_QUEUE_SIZE_MASK;

	if (queue->ReadIndex == NextIndex) {
		pr_debug(KERN_INFO
			 "Push datat fail!!: Camera  Data Queue Full!\n");
	} else {
	/*pr_debug("PushQueue Index %d  Address 0x%x\n",buf->id ,buf->busAddress); */
		queue->data[queue->WriteIndex].busAddress = buf->busAddress;
		queue->data[queue->WriteIndex].id = buf->id;
		queue->data[queue->WriteIndex].timestamp = buf->timestamp;
		queue->WriteIndex = NextIndex;
	}
	spin_unlock_irqrestore(&queue->lock, stat);
}

static bool pull_queue(struct buf_q *queue, CAM_BufData * buf)
{
	unsigned long stat;
	if (!queue)
		return false;
	if (!queue->isActive)
		return false;
	if (queue->isWaitQueue)
		return false;
	spin_lock_irqsave(&queue->lock, stat);
	if (queue->ReadIndex == queue->WriteIndex) {
		buf->busAddress = NULL;
		buf->id = -1;
	} else {
		buf->busAddress = queue->data[queue->ReadIndex].busAddress;
		buf->id = queue->data[queue->ReadIndex].id;
		buf->timestamp = queue->data[queue->ReadIndex].timestamp;
		queue->ReadIndex = (queue->ReadIndex + 1) & MAX_QUEUE_SIZE_MASK;
	}
	spin_unlock_irqrestore(&queue->lock, stat);
	return true;
}

static void wakeup_push_queue(struct buf_q *queue, CAM_BufData * buf)
{
	unsigned int NextIndex;
	unsigned long stat;
	if (!queue)
		return;
	if (!queue->isActive)
		return;
	if (!buf)
		return;
	if (!buf->busAddress)
		return;

	spin_lock_irqsave(&queue->lock, stat);

	NextIndex = (queue->WriteIndex + 1) & MAX_QUEUE_SIZE_MASK;

	if (queue->ReadIndex == NextIndex) {
	/* printk( "Wake Push datat failed!!l Camera  Data Queue Full!\n"); */
	} else {
		queue->data[queue->WriteIndex].busAddress = buf->busAddress;
		queue->data[queue->WriteIndex].id = buf->id;
		queue->data[queue->WriteIndex].timestamp = systemTime();
		queue->WriteIndex = NextIndex;
	}

	queue->isWaitQueue = true;
	spin_unlock_irqrestore(&queue->lock, stat);
	up(&queue->Sem);

}

static bool wait_pull_queue(struct buf_q *queue, CAM_BufData * buf)
{
	unsigned long stat;
	if (!queue)
		return false;
	if (!queue->isActive)
		return false;

	if (down_interruptible(&queue->Sem)) {
		return false;
	}

	spin_lock_irqsave(&queue->lock, stat);
	if (queue->ReadIndex == queue->WriteIndex) {
		buf->busAddress = NULL;
		buf->id = -1;
		pr_debug
		    ("Wait WaitPullQueue failed !! Camera  Data Queue Empty!\n");
	} else {
		buf->busAddress = queue->data[queue->ReadIndex].busAddress;
		buf->id = queue->data[queue->ReadIndex].id;
		buf->timestamp = queue->data[queue->ReadIndex].timestamp;
		queue->ReadIndex = (queue->ReadIndex + 1) & MAX_QUEUE_SIZE_MASK;
	}
	queue->isWaitQueue = true;
	spin_unlock_irqrestore(&queue->lock, stat);
	return true;
}

/* Queue handling functions end*/

static int cam_i2c_probe(struct i2c_client *client,
			 const struct i2c_device_id *dev_id)
{
	int s = 0; /* For primary sensor */
	cam_g->sens[s].cam_i2c_datap =
	    kmalloc(sizeof(struct cam_i2c_info), GFP_KERNEL);
	memset(cam_g->sens[s].cam_i2c_datap, 0, sizeof(struct cam_i2c_info));
	i2c_set_clientdata(client, cam_g->sens[s].cam_i2c_datap);
	cam_g->sens[s].cam_i2c_datap->client = client;
	return 0;
}

static int cam_i2c_remove(struct i2c_client *client)
{
	int s = 0;
	kfree(cam_g->sens[s].cam_i2c_datap);
	cam_g->sens[s].cam_i2c_datap = NULL;
	return 0;
}

static int cam_i2c_command(struct i2c_client *device, unsigned int cmd,
			   void *arg)
{
	return 0;
}

struct i2c_device_id cam_i2c_id_table[] = {
	{"cami2c", 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, cam_i2c_id_table);

static HAL_CAM_Result_en_t cam_sensor_cntrl_seq(CamSensorIntfCntrl_st_t *seq,
						UInt32 length)
{
	HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
	UInt32 i, err;

	pr_debug("%s(): \r\n", __FUNCTION__);
	pr_debug("Sequence Length=%d: \r\n",
		 (int)length / sizeof(CamSensorIntfCntrl_st_t));

	for (i = 0; i < length / sizeof(CamSensorIntfCntrl_st_t); i++) {
		switch (seq[i].cntrl_sel) {
		case GPIO_CNTRL:
			err = gpio_request(seq[i].value, "hal_cam_gpio_cntrl");
			if (err) {
				printk(KERN_ERR, "gpio_request failed for %d ", seq[i].value);
				return HAL_CAM_ERROR_ACTION_NOT_SUPPORTED;
			}
			if (seq[i].cmd == GPIO_SetHigh) {
				pr_debug("GPIO_CNTRL(%d), GPIO_SetHigh\n",
					 (int)seq[i].value);
				if (gpio_is_valid(seq[i].value))
					gpio_direction_output(seq[i].value, 1);
			} else {
				pr_debug("GPIO_CNTRL(%d), GPIO_SetLow\n",
					 (int)seq[i].value);
				if (gpio_is_valid(seq[i].value))
					gpio_direction_output(seq[i].value, 0);
			}
			gpio_free(seq[i].value);
			break;

		case MCLK_CNTRL:
			if (seq[i].cmd == CLK_TurnOn) {
				pr_debug("MCLK_CNTRL - Freq=%d\n",
					 (int)seq[i].value);
				CAM_CNTRL_SetClkFreq((CamClkSel_t) seq[i].
						     value);
			} else {
				pr_debug("MCLK_CNTRL %d - NoClk\n",
					 (int)seq[i].value);
				CAM_CNTRL_SetClkFreq(CamDrv_NO_CLK);
			}
			break;

		case PAUSE:
			if (seq[i].value != 0) {
				pr_debug("PAUSE - %d ms\n", (int)seq[i].value);
				mdelay(seq[i].value);
			}
			break;

		default:
			pr_debug("CNTRL - Not Supported\n");
			result = HAL_CAM_ERROR_ACTION_NOT_SUPPORTED;
			break;
		}
	}
	return result;
}

int cam_get_bpp_from_fmt(CamDataFmt_t fmt)
{
	/* For now */
	return 2;
}

static HAL_CAM_Result_en_t cam_sensor_intf_seqsel(CamSensorSelect_t nSensor,
						  CamSensorSeqSel_t nSeqSel)
{
	HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
	UInt32 len = 0;
	CamSensorIntfCntrl_st_t *power_seq = NULL;
	struct camera_sensor_t *c = &cam_g->sens[nSensor];
	power_seq = c->sens_m->DRV_GetIntfSeqSel(nSensor, nSeqSel, &len);
		/* get power-up/power-down sequence */
	if ((len != 0) && (power_seq != NULL)) {
		cam_sensor_cntrl_seq(power_seq, len);
	} else {
		pr_debug("%s(): No Sequence\r\n", __FUNCTION__);
	}
	return result;
}

static HAL_CAM_Result_en_t cam_config_cmi(CamSensorSeqSel_t mode,
					  CamIntfConfig_st_t *pintf)
{
	if (pintf == NULL) {
		printk(KERN_ERR
		       "No interface config specified .... returning\n");
		return -1;
	}
	/* Reset Camera Controller to default states */
	CAM_CNTRL_ResetController();
#if defined(LITTLE_ENDIAN_SWITCH)
	/* Byte Swap Input data from Camera Controller */
	CAM_CNTRL_SetByteSwap(TRUE);
#endif
	if ((mode == SensorPwrDn) || (mode == SensorInitPwrDn)) {
		CAM_CNTRL_IOCR_Cfg(mode);
		return 0;
	}
	if ((mode == SensorPwrUp) || (mode == SensorInitPwrUp)) {
		CAM_CNTRL_IOCR_Cfg(SensorPwrUp);
		if (pintf->sensor_config_ccir656->ext_sync_enable == TRUE) {
			CAM_CNTRL_SetSYNC(TRUE);
		}
		if (pintf->sensor_config_ccir656->vsync_polarity ==
		    SyncActiveHigh) {
			CAM_CNTRL_SetVsyncInv(TRUE);
		}
		if (pintf->sensor_config_ccir656->hsync_polarity ==
		    SyncActiveLow) {
			CAM_CNTRL_SetHsyncInv(TRUE);
		}
		if (pintf->sensor_config_ccir656->hsync_control == TRUE) {
			CAM_CNTRL_SetHsyncControl(TRUE);
		}
		if (pintf->sensor_config_ccir656->data_clock_sample ==
		    SyncFallingEdge) {
			CAM_CNTRL_SetClockInv(TRUE);
		}
		CAM_CNTRL_SetColorConversion(pintf->sensor_config_ycbcr->cc_red,
					     pintf->sensor_config_ycbcr->
					     cc_green,
					     pintf->sensor_config_ycbcr->
					     cc_blue);
		if (pintf->sensor_config_ycbcr->yuv_full_range == TRUE) {
			CAM_CNTRL_SetYuvFullRange(TRUE);
		}
		switch (pintf->sensor_config_ycbcr->sensor_yc_seq) {
		case SensorYCSeq_YCrYCb:
			CAM_CNTRL_SetSwapUV(TRUE);
			break;
		case SensorYCSeq_CbYCrY:
			CAM_CNTRL_SetSwapIY(TRUE);
			break;
		case SensorYCSeq_CrYCbY:
			CAM_CNTRL_SetSwapUV(TRUE);
			CAM_CNTRL_SetSwapIY(TRUE);
			break;
		case SensorYCSeq_YCbYCr:
		default:
			CAM_CNTRL_SetSwapUV(FALSE);
			CAM_CNTRL_SetSwapIY(FALSE);
			break;

		}
		CAM_CNTRL_SetWindow(0,
				    pintf->sensor_config_caps->output_st.
				    max_height, 0,
				    pintf->sensor_config_caps->output_st.
				    max_width);

	}
	return 0;
}

static HAL_CAM_Result_en_t cam_flash_enable(CamSensorSelect_t nSensor,
					    Boolean en)
{
	CamSensorSeqSel_t flash;
	HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
	if (en > 0)
		flash = SensorFlashEnable;
	else
		flash = SensorFlashDisable;
	result =  cam_sensor_intf_seqsel(nSensor, flash);
	if (result != HAL_CAM_SUCCESS)
		printk(KERN_ERR "Error with flash handling %d\n", result);
	return result;
}

static int cam_power_up(CamSensorSelect_t sensor)
{
	int rc = -1;
	struct camera_sensor_t *c;
	if (cam_g->cam_regulator)
		rc = regulator_enable(cam_g->cam_regulator);
	c = &cam_g->sens[sensor];
	c->sensor_intf = c->sens_m->DRV_GetIntfConfig(sensor);
	if (!c->sensor_intf) {
		printk(KERN_ERR "Unable to get sensor interface config \n");
		rc = -EFAULT;
	}
	/* Config CMI controller over here */
	if (cam_flash_enable(sensor, FALSE) != HAL_CAM_SUCCESS) {
		printk(KERN_ERR "Unable to Control Flash\n");
		rc = -EFAULT;
	}
	if (cam_config_cmi(SensorInitPwrUp, c->sensor_intf) != HAL_CAM_SUCCESS) {
		rc = -EFAULT;
	}
	if (cam_sensor_intf_seqsel(sensor, SensorInitPwrUp) != 0) {
		printk(KERN_ERR "Unable to Set power seq at Open\n");
		rc = -EFAULT;
	}
	return 0;
}

static int cam_power_down(CamSensorSelect_t sensor)
{
	int rc = -1;
	struct camera_sensor_t *c;
	c = &cam_g->sens[sensor];
	c->sensor_intf = c->sens_m->DRV_GetIntfConfig(sensor);
	if (c->sensor_intf == NULL) {
		printk(KERN_ERR "Cam power down unable to get intf config\n");
		return -1;
	}
	if (c->sens_m->DRV_SetCamSleep(sensor) != HAL_CAM_SUCCESS) {
		printk(KERN_ERR "Cam power down unable to get intf config\n");
		return -1;
	}
	if (cam_sensor_intf_seqsel(sensor, SensorPwrDn) != 0) {
		printk(KERN_ERR "Unable to Set power seq at power down\n");
		rc = -EFAULT;
	}
	if (cam_config_cmi(SensorInitPwrDn, c->sensor_intf) != HAL_CAM_SUCCESS) {
		printk(KERN_ERR "Unable to power down CMI controller\n");
		rc = -EFAULT;
	}
	if (cam_g->cam_regulator)
		regulator_disable(cam_g->cam_regulator);
	return 0;

}

static int cam_config_dma_buffers(int x, int y, CamDataFmt_t fmt,
				  CamSensorSelect_t sensor)
{
	int split, i = 0, j = 0;
	int cam_off;
	int size;
	int num;
	int *ptr;
	int tx;
	if (cam_g->sens[sensor].mode == CAM_STILL)
		split = 1;
	else
		split = CAM_NUM_VFVIDEO;
	if (sensor == CamSensorPrimary)
		cam_off = 0;
	else	/* Not required if user specifies mem buffers for VF/Video */
		cam_off = CAM_SEC_OFFSET;
	size = x * y * cam_get_bpp_from_fmt(fmt);
	num = x * cam_get_bpp_from_fmt(fmt);
	tx = num >> 2;

	/* No flip for now
	 The transfer size is kept to max 4095 * 32 bits
	 To reduce the number of LLI required */
	if (split == 1)
		cam_g->cam_buf = cam_g->cam_buf_main;
	/* Once you move to still mode from VF, need to use the
		original buffer that we allocated at boot time.
		On the next return to VF, MEM REGISTER needs to be
		called so that things are restored. */

	for (i = 0; i < split; i++) {
		cam_g->sens[sensor].camb[i].sizeInBytes = size;
	/* cam_buf can be overridden by user specified physical address */
		cam_g->sens[sensor].camb[i].virt =
		    cam_g->cam_buf.virt + cam_off + i * size;
		cam_g->sens[sensor].camb[i].phy =
		    cam_g->cam_buf.phy + cam_off + i * size;
		cam_g->sens[sensor].cam_ll[i].sizeInBytes =
		    (y + 1) * BYTES_PER_LLI;
		cam_g->sens[sensor].cam_ll[i].virt =
		    dma_alloc_coherent(0,
				       (cam_g->sens[sensor].cam_ll[i].
					sizeInBytes),
				       &cam_g->sens[sensor].cam_ll[i].phy,
				       GFP_KERNEL | GFP_DMA);
		if (!cam_g->sens[sensor].cam_ll[i].virt)
			pr_debug(KERN_ERR
				 "Failed to alloc memory for LLI for main buffer %d\n",
				 i);
		else {
			pr_debug(KERN_ERR
				 "Allocated 0x%x virt 0x%x phys %d size\n",
				 (unsigned int)cam_g->sens[sensor].cam_ll[i].
				 virt,
				 (unsigned int)cam_g->sens[sensor].cam_ll[i].
				 phy,
				 cam_g->sens[sensor].cam_ll[i].sizeInBytes);
		}
		ptr = cam_g->sens[sensor].cam_ll[i].virt;
		ptr[0] = REG_CAM_DATR_PADDR;
		/* Source addr is always the camera CMI FIFO memory address */
		ptr[1] = cam_g->sens[sensor].camb[i].phy;
		ptr[2] = cam_g->sens[sensor].cam_ll[i].phy + BYTES_PER_LLI;
		ptr[3] = DMA_FROM_CAM_CONTROL | tx;
		ptr += INTS_PER_LLI;
		/* Now construct the remaining LLI descriptors */
		for (j = 1; j < (y - 1); j++) {
			ptr[0] = SWAP(REG_CAM_DATR_PADDR);
			ptr[1] =
			    SWAP(cam_g->sens[sensor].camb[i].phy + (num * j));
			ptr[2] =
			    SWAP(cam_g->sens[sensor].cam_ll[i].phy +
				 (BYTES_PER_LLI * (j + 1)));
			ptr[3] = SWAP(DMA_FROM_CAM_CONTROL | tx);
			ptr += INTS_PER_LLI;
		}
		ptr[0] = SWAP(REG_CAM_DATR_PADDR);
		ptr[1] = SWAP(cam_g->sens[sensor].camb[i].phy + (num * j));
		ptr[2] = 0;
		ptr[3] =
		    SWAP(DMA_FROM_CAM_CONTROL | tx |
			 REG_DMA_CHAN_CTL_TC_INT_ENABLE);
	}
	return 0;
}

static void camStartDmaTransfer(int index, CamSensorSelect_t sensor)
{
	int *ptr;
	dma_init_chan(cam_g->sens[sensor].dma_chan);
	ptr = cam_g->sens[sensor].cam_ll[index].virt;
	dma_setup_chan(cam_g->sens[sensor].dma_chan, ptr[0], ptr[1], ptr[2],
		       ptr[3], DMA_FROM_CAM_CONFIG);
}

static void camStopDmaTransfer(CamSensorSelect_t sensor)
{
	dma_disable_chan(cam_g->sens[sensor].dma_chan);
}

static irqreturn_t vsync_isr(int irq, void *arg)
{
	CamSensorSelect_t s;
	ktime_t p,diff;
	struct camera_sensor_t *c;
	s = *(CamSensorSelect_t *) arg;
	c = &cam_g->sens[s];
	/* *c would have the sensor ID now */
	CLEAR_VSYNC_INTR();
	if (c->mode <= CAM_NONE)
		return IRQ_HANDLED;
	if ((c->state == CAM_STOPPING) || (c->state == CAM_OFF))
		return IRQ_HANDLED;
	CAM_CNTRL_DisableRst();
	if(c->mode == CAM_STILL){
		if(c->state == CAM_PAUSE)
			return IRQ_HANDLED;
		p = ktime_get();
		if(c->prev.tv.sec == 0)
			c->prev = p;
		diff = ktime_sub(p,c->prev);
		if(diff.tv.nsec > REFCAPTIME){
			c->sCaptureFrameCountdown = 0;
		} else {
			c->prev = p;
			c->sCaptureFrameCountdown--;
			if(c->sCaptureFrameCountdown <= 0)
				printk(KERN_ERR"Retries and no good JPEG :-( \n");
		}
	}
	#if 0
	if (c->state != CAM_PAUSE)
		c->sCaptureFrameCountdown--;
	#endif
	if (c->mode == CAM_STILL) {
		if ((c->state != CAM_PAUSE) && (c->sCaptureFrameCountdown <= 0)) {
			c->state = CAM_PAUSE;
			camStopDmaTransfer(s);
			c->still_ready = 1;
			/* allow process context to process next frame */
			wake_up_interruptible(&gDataReadyQ);
			return IRQ_HANDLED;
		}
		if ((c->state == CAM_PAUSE) && !c->sCaptureJpegSize
		    && (atomic_read(&c->captured) == 0))
			c->state = CAM_STILL;
		if (c->state != CAM_PAUSE) {
			camStartDmaTransfer(0, s);
		}
		CAM_CNTRL_Enable();
		/* STill cpature */
	} else if (c->mode == CAM_STREAM) {
		wakeup_push_queue(&c->rd_Q, &c->gCurrData);
		pull_queue(&c->wr_Q, &c->gCurrData);
		if (c->gCurrData.busAddress) {
			camStartDmaTransfer(c->gCurrData.id, s);
		} else {
		}
		CAM_CNTRL_Enable();
	}

	return IRQ_HANDLED;
}

static irqreturn_t dma_isr(void *unused)
{
	return IRQ_HANDLED;
}

/* Invoke these functions directly from IOCTL based on arg */
int camera_enable(CamSensorSelect_t sensor)
{
	struct camera_sensor_t *c = &cam_g->sens[sensor];
	/* SetParm IOCTL would have populated the CAM_PARM_t structure */
	HAL_CAM_ResolutionSize_st_t sensor_size;
	int sdfh = 128, sdfv = 128;
	/*c->sens_m->DRV_GetResolution(c->main.size_window.size
	*,c->main.mode,sensor,&sensor_size);
	c->main.size_window.size = sensor_size.size; */
	sensor_size.resX = c->main.size_window.end_pixel;
	sensor_size.resY = c->main.size_window.end_line;
	if (c->main.format == CamDataFmtJPEG) {
		sensor_size.resX =
		    c->sensor_intf->sensor_config_jpeg->
		    jpeg_packet_size_bytes >> 1;
		sensor_size.resY =
		    c->sensor_intf->sensor_config_jpeg->jpeg_max_packets;
	} else if (c->main.format == CamDataFmtYUV) {
		sensor_size.resX = (sensor_size.resX * 3) >> 1;
		/* YUV420 1.5 bytes per pixel case */
	}
	/* We dont support padding in the initial version
	as we have been using only JPEG capture mostly.*/
	CAM_CNTRL_SetWindow(0, sensor_size.resY, 0, sensor_size.resX);
	CAM_CNTRL_SetSDF(sdfh, sdfv);
	/* Config DMA with dma_x and dma_y */
	CAM_CNTRL_SetDataFmt(c->main.format);
	/* Config DMA over here
	Sensor driver settings */
	if ((c->main.mode == CamStillnThumb) || (c->main.mode == CamStill)) {
		CHECK_RES(c->sens_m->
			  DRV_CfgStillnThumbCapture(c->main.size_window.size,
						    c->main.format,
						    c->th.size_window.size,
						    c->th.format, sensor));
		c->mode = CAM_STILL;
		c->prev.tv.sec = 0;
		c->prev.tv.nsec = 0;
		cam_config_dma_buffers(sensor_size.resX, sensor_size.resY,
				       c->main.format, sensor);
		c->gProcessFrameRunning = 1;
		c->state = CAM_INIT;
		c->sCaptureJpegSize = 0;
	} else {
		/* Video and/or VF mode */
		cam_config_dma_buffers(sensor_size.resX, sensor_size.resY,
				       c->main.format, sensor);
		CHECK_RES(c->sens_m->
			  DRV_SetVideoCaptureMode(c->main.size_window.size,
						  c->main.format, sensor));
		CHECK_RES(c->sens_m->DRV_SetFrameRate(c->main.fps, sensor));
		CHECK_RES(c->sens_m->DRV_EnableVideoCapture(sensor));
		c->mode = CAM_STREAM;
		c->state = CAM_INIT;
	}
	c->dma_chan = 0;
	if(dma_request_chan(c->dma_chan,"camera") != 0) {
		if (0 != dma_request_avail_chan(&c->dma_chan, "camera")) {
			printk(KERN_ERR "Failed to alloc DMA channel\n");
			return -EBUSY;
		}
	}
	if (dma_request_irq(cam_g->sens[cam_g->curr].dma_chan, dma_isr, 0) != 0) {
		printk(KERN_ERR "Failed to set DMA ISR\n");
	}
	c->sCaptureFrameCountdown = 10;
#ifdef CONFIG_HAS_WAKELOCK
	wake_lock(&cam_g->camera_wake_lock);
#endif
	c->rd_Q.isActive = 0;
	c->wr_Q.isActive = 0;
	init_queue(&c->wr_Q);
	init_queue(&c->rd_Q);
	c->state = CAM_ON;
	CLEAR_VSYNC_INTR();
	enable_irq(IRQ_VSYNC);
	return 0;
}

int camera_disable(CamSensorSelect_t sensor)
{
	struct camera_sensor_t *c = &cam_g->sens[sensor];
/* SetParm IOCTL would have populated the CAM_PARM_t structure */
	unsigned long stat;
	int split;
	int i;
	int rc = 0;
	spin_lock_irqsave(&c->c_lock, stat);
	c->mode = CAM_NONE;
	c->state = CAM_STOPPING;
	spin_unlock_irqrestore(&c->c_lock, stat);
	disable_irq(IRQ_VSYNC);
	CAM_CNTRL_DisableRst();
	c->gProcessFrameRunning = 0;
	wake_up_interruptible(&gDataReadyQ);
	dma_init_chan(c->dma_chan);
	dma_free_irq(c->dma_chan);
	dma_free_chan(c->dma_chan);
	if ((c->main.mode == CamStillnThumb) || (c->main.mode == CamStill)) {
		pr_debug(KERN_INFO "Disabling capture\n");
		c->sens_m->DRV_DisableCapture(sensor);
	} else {
		pr_debug(KERN_INFO "Disabling stream\n");
		c->sens_m->DRV_DisablePreview(sensor);
	}
	if ((c->main.mode == CamStill) || (c->main.mode == CamStillnThumb))
		split = 1;
	else
		split = 4;
	for (i = 0; i < split; i++) {
		dma_free_coherent(NULL, c->cam_ll[i].sizeInBytes,
				  c->cam_ll[i].virt, c->cam_ll[i].phy);
	}
#ifdef CONFIG_HAS_WAKELOCK
	wake_unlock(&cam_g->camera_wake_lock);
#endif
	deinit_queue(&c->wr_Q);
	deinit_queue(&c->rd_Q);
	c->state = CAM_OFF;
	return rc;
}

static int cam_open(struct inode *inode, struct file *file)
{
	int rc;
	struct camera_sensor_t *c;
	/* Choose between primary and sec camera */
	down(&cam_g->cam_sem);
	if (1) {
		cam_g->curr = CamSensorPrimary;
		cam_g->sens[CamSensorPrimary].sens_m = CAMDRV_primary_get();
	} else {
		cam_g->curr = CamSensorSecondary;
	}
	c = &cam_g->sens[cam_g->curr];
	spin_lock_init(&c->c_lock);
	c->sensor_intf = c->sens_m->DRV_GetIntfConfig(cam_g->curr);
	cam_power_up(cam_g->curr);
	c->mode = -1;
	if (c->sens_m->DRV_Wakeup(cam_g->curr)) {
		printk(KERN_ERR "Failed to init the sensor\n");
		rc = -EFAULT;
		goto oerr;
	}
	if ((c->sensor_intf->sensor_config_ccir656->vsync_irq_trigger ==
	     SyncRisingEdge)
	    || (c->sensor_intf->sensor_config_ccir656->vsync_irq_trigger ==
		SyncActiveHigh)) {
		set_irq_type(IRQ_VSYNC, IRQ_TYPE_EDGE_RISING);
	} else {
		set_irq_type(IRQ_VSYNC, IRQ_TYPE_EDGE_FALLING);
	}
	CLEAR_VSYNC_INTR();
	rc = request_irq(IRQ_VSYNC, vsync_isr, IRQF_DISABLED, "vsync",
			 &cam_g->curr);
	if (rc != 0) {
		printk(KERN_ERR "Failed to subscribe to VSYNC IRQ %d\n", rc);
		rc = -EFAULT;
		goto oerr;
	}
	/* c->sensor_intf would be valid after successful return of power up */
	/* Init the working buffer queue */
	c->gCurrData.id = -1;
	c->gCurrData.busAddress = NULL;
	c->gCurrData.timestamp = 0;
	cam_g->sens[cam_g->curr].state = CAM_OFF;
	c->mode = CAM_NONE;
	up(&cam_g->cam_sem);
	disable_irq(IRQ_VSYNC);
	return 0;
oerr:
	clk_disable(cam_clk);
	up(&cam_g->cam_sem);
	return rc;
}

static int cam_release(struct inode *inode, struct file *file)
{
	struct camera_sensor_t *c;
	c = &cam_g->sens[cam_g->curr];
	cam_power_down(cam_g->curr);
	if (c->state != CAM_OFF) {
		camera_disable(cam_g->curr);
	}
	free_irq(IRQ_VSYNC, &cam_g->curr);
	pr_debug(KERN_INFO "IRQ VSYNC freed\n");
#ifdef CONFIG_HAS_WAKELOCK
	wake_unlock(&cam_g->camera_wake_lock);
#endif
	return 0;
}

/* CAM I2C functions begin */
HAL_CAM_Result_en_t CAM_WriteI2c(UInt16 camRegID, UInt16 DataCnt, UInt8 *Data)
{
	HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
	struct i2c_adapter *adap;
	int s = cam_g->curr;

	if (!DataCnt || (DataCnt > MAX_I2C_DATA_COUNT)) {
		result = HAL_CAM_ERROR_INTERNAL_ERROR;
		pr_debug("%s() - DataCnt = %d out of range\n", __FUNCTION__,
			 DataCnt);
		goto done;
	}

	if ((cam_g->sens[s].cam_i2c_datap != NULL)
	    && (adap = cam_g->sens[s].cam_i2c_datap->client->adapter)) {
		int ret;
		unsigned char msgbuf[sizeof(camRegID) + MAX_I2C_DATA_COUNT];
		struct i2c_msg msg = {
			cam_g->sens[s].cam_i2c_datap->client->addr,
			cam_g->sens[s].cam_i2c_datap->client->flags,
			sizeof(camRegID) + DataCnt, msgbuf };

		msgbuf[0] = (u8) ((camRegID & 0xFF00) >> 8);
		msgbuf[1] = (u8) (camRegID & 0x00FF);
		memcpy(&msgbuf[sizeof(camRegID)], Data, DataCnt);

		ret = i2c_transfer(adap, &msg, 1);
		if (ret != 1)
			result = HAL_CAM_ERROR_INTERNAL_ERROR;

	} else {
		pr_debug("%s() - Camera I2C adapter null\n", __FUNCTION__);
		result = HAL_CAM_ERROR_INTERNAL_ERROR;
	}
done:
	return result;
}

HAL_CAM_Result_en_t CAM_ReadI2c(UInt16 camRegID, UInt16 DataCnt, UInt8 *Data)
{
	HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
	struct i2c_adapter *adap;
	int s = cam_g->curr;

	if (!DataCnt) {
		result = HAL_CAM_ERROR_INTERNAL_ERROR;
		pr_debug("%s() - DataCnt = %d out of range\n", __FUNCTION__,
			 DataCnt);
		goto done;
	}

	if ((cam_g->sens[s].cam_i2c_datap)
	    && (adap = cam_g->sens[s].cam_i2c_datap->client->adapter)) {
		int ret;
		unsigned char msgbuf0[sizeof(camRegID)];
		struct i2c_msg msg[2] = {
			{cam_g->sens[s].cam_i2c_datap->client->addr,
			 cam_g->sens[s].cam_i2c_datap->client->flags,
			 sizeof(camRegID), msgbuf0}
			,
			{cam_g->sens[s].cam_i2c_datap->client->addr,
			 cam_g->sens[s].cam_i2c_datap->client->flags | I2C_M_RD,
			 DataCnt, Data}
		};

		msgbuf0[0] = (camRegID & 0xFF00) >> 8;
		msgbuf0[1] = (camRegID & 0x00FF);
		ret = i2c_transfer(adap, msg, 2);
		if (ret != 2)
			result |= HAL_CAM_ERROR_INTERNAL_ERROR;

	} else {
		pr_debug("%s() - Camera I2C adapter null\n", __FUNCTION__);
		result = HAL_CAM_ERROR_INTERNAL_ERROR;
	}
done:
	return result;
}

/* CAM I2C functions end */
struct i2c_driver i2c_driver_cam = {
	.driver = {
		   .name = IF_NAME,
		   },
	.id_table = cam_i2c_id_table,
	.probe = cam_i2c_probe,
	.remove = cam_i2c_remove,
	.command = cam_i2c_command,
};

static int __init cam_init(void)
{
	int rc;
	int sensor = 0;
	struct camera_sensor_t *c;
	printk(KERN_INFO "%s\n", banner);
	cam_g =
	(struct cam_generic_t *)kmalloc(sizeof(struct cam_generic_t), GFP_KERNEL);
	if (!cam_g) {
		printk(KERN_ERR "No memory for camera driver\n");
		return -ENOMEM;
	}
	rc = register_chrdev(BCM_CAM_MAJOR, "camera", &cam_fops);
	if (rc < 0) {
		printk(KERN_ERR "Camera: register_chrdev failed for major %d\n",
		       BCM_CAM_MAJOR);
		return rc;
	}
	cam_g->cam_class = class_create(THIS_MODULE, "camera");
	if (IS_ERR(cam_g->cam_class)) {
		unregister_chrdev(BCM_CAM_MAJOR, "camera");
		rc = PTR_ERR(cam_g->cam_class);
		goto err;
	}
	device_create(cam_g->cam_class, NULL, MKDEV(BCM_CAM_MAJOR, 0), NULL,
		      "camera");
	cam_g->cam_regulator = regulator_get(NULL, "cam_vdd");
	if (!cam_g->cam_regulator || IS_ERR(cam_g->cam_regulator)) {
		printk(KERN_ERR "No Regulator available\n");
		rc = -EFAULT;
		goto err2;
	}
#ifdef CONFIG_HAS_WAKELOCK
	wake_lock_init(&cam_g->camera_wake_lock, WAKE_LOCK_SUSPEND, "camera");
#endif
	cam_g->gSysCtlHeader = register_sysctl_table(gSysCtl);
	cam_clk = clk_get(NULL, BCM_CLK_CAMERA_STR_ID);
	if (!cam_clk)
		goto err3;
	cam_g->sens[0].sens_m = CAMDRV_primary_get();
	cam_g->curr = 0;
	cam_power_up(cam_g->curr);
	/* Alloc separately for primary and secondary cameras. For now only primary
	 cam_buf is the operating buffer
	 cam_buf_main always holds the addresses of the allocated boot time buffer
	 Stub 1 */
	c = &cam_g->sens[0];
	c->sensor_intf = c->sens_m->DRV_GetIntfConfig(0);
	if (!c->sensor_intf) {
		printk(KERN_ERR "Unable to get sensor interface config \n");
		rc = -EFAULT;
	}
	/* Config CMI controller over here */
	if (cam_config_cmi(SensorInitPwrUp, c->sensor_intf) != HAL_CAM_SUCCESS) {
		rc = -EFAULT;
	}
	if (cam_sensor_intf_seqsel(sensor, SensorInitPwrUp) != 0) {
		printk(KERN_ERR "Unable to Set power seq at Open\n");
		rc = -EFAULT;
	}
	cam_g->sens[0].cam_i2c_datap = NULL;
	/* For now only for primary camera */
	rc = i2c_add_driver(&i2c_driver_cam);
	sema_init(&cam_g->cam_sem, 1);
	mdelay(2);
	if (cam_sensor_intf_seqsel(sensor, SensorPwrDn) != 0) {
		printk(KERN_ERR "Unable to Set power seq at Open\n");
		rc = -EFAULT;
	}
	if (cam_config_cmi(SensorPwrDn, c->sensor_intf) != HAL_CAM_SUCCESS) {
		rc = -EFAULT;
	}

	cam_g->cam_buf_main.virt =
	    dma_alloc_coherent(NULL, CAM_BOOT_TIME_MEMORY_SIZE,
			       &cam_g->cam_buf_main.phy, GFP_DMA | GFP_KERNEL);
	if ((cam_g->cam_buf_main.virt == NULL)
	    || (cam_g->cam_buf_main.phy == (dma_addr_t) 0)) {
		printk(KERN_ERR "No DMA buffer memory for cam driver\n");
		rc = -ENOMEM;
		goto err3;
	}
#if defined(CONFIG_BCM_CAM_MT9T111)
	CAMDRV_StoreBaseAddress(cam_g->cam_buf_main.virt);
	cam_g->cam_buf_main.virt += 1024;
	cam_g->cam_buf_main.phy += 1024;
#endif
	cam_g->cam_buf = cam_g->cam_buf_main;
	cam_power_down(cam_g->curr);
	return rc;
#ifdef CONFIG_HAS_WAKELOCK
err3:	wake_lock_destroy(&cam_g->camera_wake_lock);
#else
err3:
#endif
err2:	device_destroy(cam_g->cam_class, MKDEV(BCM_CAM_MAJOR, 0));
	class_destroy(cam_g->cam_class);
err:	unregister_chrdev(BCM_CAM_MAJOR, "camera");
	return rc;
}

static void __exit cam_exit(void)
{
	if (cam_g->gSysCtlHeader != NULL) {
		unregister_sysctl_table(cam_g->gSysCtlHeader);
	}
	if (cam_g->cam_regulator)
		regulator_put(cam_g->cam_regulator);
#ifdef CONFIG_HAS_WAKELOCK
	wake_lock_destroy(&cam_g->camera_wake_lock);
#endif
	device_destroy(cam_g->cam_class, MKDEV(BCM_CAM_MAJOR, 0));
	class_destroy(cam_g->cam_class);
	unregister_chrdev(BCM_CAM_MAJOR, "camera");
	kfree(cam_g);
}

module_init(cam_init);
module_exit(cam_exit);
MODULE_AUTHOR("Broadcom");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Camera Driver");

