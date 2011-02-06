/*******************************************************************************************
Copyright 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement 
governing use of this software, this software is licensed to you under the 
terms of the GNU General Public License version 2, available at 
http://www.gnu.org/copyleft/gpl.html (the "GPL"). 

Notwithstanding the above, under no circumstances may you combine this software 
in any way with any other Broadcom software provided under a license other than 
the GPL, without Broadcom's express prior written consent.
*******************************************************************************************/

/*
*
*****************************************************************************
*
*  camera.c
*
*  PURPOSE:
*
*     This implements the driver for the Sanyo IGT99268 CCD camera.
*
*  NOTES:
*
*****************************************************************************/

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
#include <linux/broadcom/PowerManager.h>
#include <plat/dma.h>
#include <linux/dma-mapping.h>
#include <linux/wakelock.h>
#include <linux/clk.h>
#include <mach/clkmgr.h>
#include <linux/regulator/consumer.h>

#include "hal_cam.h"
#include "hal_cam_drv.h"
#include "cam_cntrl_2153.h"
#include "camdrv_dev.h"

/* FIXME: Clear VSYNC interrupt explicitly until handler properly */
#define CLEAR_VSYNC_INTR()   writel(1 << IRQ_VSYNC, HW_IRQ_BASE + 0x0008)

/* ---- Public Variables ------------------------------------------------- */

typedef struct {
	int vsyncIrqs;
	int dmaCam;
	int level;
	int blocked;

} CAM_DATA;

static CAM_DATA gCamState;
static CAM_Config_t gCamConfig;

atomic_t captured;
/* ---- Private Constants and Types -------------------------------------- */
#define FLIP_IMAGE  		0	/* 1 = write image bottom to top */

/* swap from big endian to little endian for 32 bits */
#define SWAP(val)   cpu_to_le32(val)
#define CAMERA_WIDTH		CamWindowSize_QVGA_W	/* width of camera data in pixels */
#define CAMERA_HEIGHT   	CamWindowSize_QVGA_H	/* height of camera data in pixels */
#define FRAME_WIDTH 		CAMERA_WIDTH	/* frame width in pixels */
#define FRAME_HEIGHT		CAMERA_HEIGHT	/* frame height in pixels */

/* DMA LLI Defines */
#define DESIRED_DMA_CHANNEL 0	/* desired DMA channel: 0 is highest priority */
#define BYTES_PER_LLI   	16	/* number of bytes per Linked List Item (LLI) */
#define INTS_PER_LLI		(BYTES_PER_LLI/4)	/* number of ints per LLI */
#define DMA_LLI_BUFFERS 	4	/* number of DMA LLI Buffers */
#define MAX_QUEUE_SIZE   4	/*define the maximum queue buffer we support for queue operation */
#define MAX_QUEUE_SIZE_MASK  (MAX_QUEUE_SIZE-1)	/*define queue size mask */
#define CAM_BOOT_TIME_MEMORY_SIZE (1024 * 1024 * 3 / 2)
/*#define CAM_BOOT_TIME_MEMORY_SIZE (1024) */

/**  HAL CAM driver DMA Modes
*/
typedef enum {
	DMADRV_STILLS_MODE,	/*< Stills capture mode*/
	DMADRV_MOVIE_MODE	/*< Movie capture mode*/
} DMADRV_LLITABLE_MODE_t;

/* I2C */
#define IF_NAME             "cami2c"

#define I2C_DRIVERID_CAM    0xf000

/* definitions for command sequence processing */
#define CAM_CMD_END         0
#define CAM_CMD_SET_REG     1
#define CAM_CMD_WAIT_MSEC   2
#define CAM_END_SEQUENCE            { CAM_CMD_END }
#define CAM_SET_REG( reg, val )     { CAM_CMD_SET_REG, (reg), (val) }
#define CAM_WAIT_MSEC(msec)         { CAM_CMD_WAIT_MSEC, 0, (msec) }

/* DMA settings */
#define DMA_FROM_CAM_CONTROL                \
    ( REG_DMA_CHAN_CTL_DEST_INCR            \
    | REG_DMA_CHAN_CTL_SRC_INCR             \
    | REG_DMA_CHAN_CTL_DEST_WIDTH_32        \
    | REG_DMA_CHAN_CTL_SRC_WIDTH_32         \
    | REG_DMA_CHAN_CTL_DEST_BURST_SIZE_32   \
    | REG_DMA_CHAN_CTL_SRC_BURST_SIZE_32    \
    )

#define DMA_FROM_CAM_CONFIG                      \
    ( REG_DMA_CHAN_CFG_TC_INT_ENABLE             \
    | REG_DMA_CHAN_CFG_ERROR_INT_ENABLE          \
    | REG_DMA_CHAN_CFG_FLOW_CTL_PRF_TO_MEM_DMA   \
    | (REG_DMA_PERIPHERAL_CAMERA << REG_DMA_CHAN_CFG_SRC_PERIPHERAL_SHIFT) \
    | REG_DMA_CHAN_CFG_ENABLE)

/* Debug logging */
#ifdef DEBUG
#undef DEBUG
#endif
/*#define DEBUG 1*/

#define DBG_ERROR	0x01
#define DBG_INFO	0x02
#define DBG_TRACE	0x04
#define DBG_TRACE2	0x08
#define DBG_DATA	0x10
#define DBG_DATA2	0x20

#define DBG_DEFAULT_LEVEL	(DBG_ERROR)

#if defined(DEBUG)
#define CAM_DEBUG(lev,x) {if (lev & gCamState.level) printk x;}
#else
#define CAM_DEBUG(lev,x)
#endif

/* ---- Private Variables ------------------------------------------------ */

static char banner[] __initdata =
    KERN_INFO "Camera Driver: 1.00 (built on " __DATE__ " " __TIME__ ")\n";
static struct class *cam_class;

/* sysctl */
static struct ctl_table_header *gSysCtlHeader;

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

static CAM_Parm_t stCamParm = {
	CamRate_15,		/* CamRates_t    fps;        frames per second*/
	CamDataFmtYCbCr,	/* CamDataFmt_t format;     image format */
	{
	 CamImageSize_QCIF,	/* CamImageSize_t   size;       < (in) image size to window & scale */
	 0,			/* start_pixel;                 ///< (in) start pixel (x start)*/
	 176,			/* end_pixel;                   ///< (in) stop pixel  (x stop)*/
	 0,			/* start_line;                  ///< (in) start line  (y start)*/
	 144,			/* end_line;                    ///< (in) end line    (y stop)*/
	 CamHorizontalNoSDF,	/* horizontal_SDF;              ///< (in) horizontal scale factor*/
	 CamVerticalNoSDF	/* vertical_SDF;                ///< (in) vertical scale factor*/
	 },			/* CAM_window_scale_st_t   size_window;     // image resolution/windowing*/

	CamPreview		/* CamCaptureMode_t mode;   // image capture mode */
};

static CAM_Parm_t stThumbnailParm = {
	CamRate_15,		/* CamRates_t    fps;        // frames per second*/
	CamDataFmtYCbCr,	/* CamDataFmt_t format;     // image format*/
	{
	 CamImageSize_QCIF,	/* CamImageSize_t   size;       ///< (in) image size to window & scale*/
	 0,			/* start_pixel;                 ///< (in) start pixel (x start)*/
	 176,			/* end_pixel;                   ///< (in) stop pixel  (x stop)*/
	 0,			/* start_line;                  ///< (in) start line  (y start)*/
	 144,			/* end_line;                    ///< (in) end line    (y stop)*/
	 CamHorizontalNoSDF,	/* horizontal_SDF;              ///< (in) horizontal scale factor*/
	 CamVerticalNoSDF	/* vertical_SDF;                ///< (in) vertical scale factor*/
	 },			/* CAM_window_scale_st_t   size_window;     // image resolution/windowing*/

	CamPreview		/* CamCaptureMode_t mode;   // image capture mode*/
};

DECLARE_WAIT_QUEUE_HEAD(gDataReadyQ);
struct semaphore gCamDrvSem;
struct semaphore gLock;
volatile int gProcessFrameRunning;
static struct wake_lock camera_wake_lock;

struct cam_i2c_info {
	struct i2c_client *client;
};

static struct cam_i2c_info *cam_i2c_datap;

struct i2c_device_id cam_i2c_id_table[] = {
	{"cami2c", 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, cam_i2c_id_table);

/****************************************************************************************/
/*       Hal Cam Variables
****************************************************************************************/
typedef enum {
	CAM_STATE_IDLE,		/* idle state */
	CAM_STATE_ON,		/* powered on state */
	CAM_STATE_OFF,		/* powered off state */
/* Sensor Streaming Modes */
	CAM_STATE_PAUSE,	/* pause state */
	CAM_STATE_STILL_CAPTURE_INIT,	/* stills image capture init frame */
	CAM_STATE_STILL_CAPTURE,	/* stills image capture, stop image capture, call-back */
	CAM_STATE_VIDEO_CAPTURE_INIT,	/* video capture init frame */
	CAM_STATE_VIDEO_CAPTURE,	/* camera video record state -> call-back */
	CAM_STATE_STILLnTHUMB_CAPTURE_THUMB_INIT,	/* still & thumb capture, thumb capture state init frame */
	CAM_STATE_STILLnTHUMB_CAPTURE_THUMB,	/* still & thumb capture, thumb capture state */
	CAM_STATE_STILLnTHUMB_CAPTURE_STILL_INIT,	/* still & thumb capture, still capture state init frame */
	CAM_STATE_STILLnTHUMB_CAPTURE_STILL,	/* still & thumb capture, still capture state */
	CAM_STATE_ZOOM_PREVIEW	/* camera controller zoom operation  (NOT RECOMMENDED) */
#ifdef  PIN_CHECK_READY
	    , CAM_STATE_PIN_TEST	/* state to check pin status */
#endif
} CameraState_t;

/* Camera State Variables */
static CameraState_t sCamState = CAM_STATE_OFF;
static CameraState_t sCamState_Save;
static Boolean sCamInitialized = FALSE;
static CamSensorSelect_t sCurrentSensor = CamSensorPrimary;	/* Unused */
static Boolean sNewCamWindowStatus = FALSE;	/* New Camera Window status */
/* Dma Variables */
static DMADRV_LLITABLE_MODE_t sCamDmaMode = DMADRV_MOVIE_MODE;
static Boolean sCamDmaRepeat = FALSE;	/* DMA Repeat Mode (Link-List) */
/*static UInt32                                 sCamDmaLinesComplete;                                           // # of DMA lines captured */

/* Camera Capture Variables */
/*static CamCaptureConfig_st_t    stCamCaptureConfig; */
static CamImageSize_t sCaptureSize; /* capture resolution in call-back*/
static CamImageSize_t sSensorOutputSize; /* current sensor output resolution*/
static CamDataFmt_t sCaptureFormat; /* capture format*/
static CamWindowConfig_st_t stCamWindowConfig; /* Current Camera Window configuration*/
static CamCaptureMode_t sCaptureMode; /* capture mode*/
static CamWindowConfig_st_t stNewCamWindowConfig; /* New Camera Window configuration*/
static CamWindowConfig_st_t stCamWindowConfig; /* Current Camera Window configuration*/
static UInt32 sZoomFactor = SDF_MAX_SCALE;
static Int32 sCaptureFrameCountdown;

static CamWindowSize_t sCamOutputWidth; /* image output width resolution*/
static CamWindowSize_t sCamOutputHeight; /* image output height resolution*/
static CamWindowSize_t sCamWidth; /* Dma width byte count*/
static CamWindowSize_t sCamHeight; /* Dma height (lines)*/
static UInt32 sCamReturnWidth; /* Actual width size in bytes returned in call-back*/
static UInt32 sCamReturnHeight; /* Actual height size in bytes returned in call-back*/
/* static UInt32                                 sCamImageByteCnt;                       // Actual byte count of image returned in call-back*/
static UInt32 sCaptureJpegSize; /* Actual byte count of JPEG image returned in call-back*/

/* Sensor Interface Configuration:  CCIR-656, I2C, IOCR, Sensor Configuration*/
static CamIntfConfig_st_t *pSensorIntfConfig = NULL;

/* I2C Variables*/
static UInt32 sCamI2cAccessMode;	/* I2C Bus Access*/
static UInt32 sCamI2cSubAdrOp;	/* I2C sub-address size*/
static UInt32 sCamI2cDeviceId;	/* I2C Device ID*/
static UInt32 sCamI2cClockSpeed;	/* I2C clock speed*/
static UInt32 sCamI2cPageReg;	/* I2C device page register addr, Some drivers use this variable*/
static UInt32 sCamI2cMaxPage;	/* I2C device max page, Some drivers use this variable*/

/****************************************************************************************/
/****************************************************************************************/

/* command structure for command sequences */
typedef struct {
	u8 cmd;
	u8 reg;
	u8 val;

} cam_cmd_t;

typedef struct {
	size_t sizeInBytes;
	void *virtPtr;
	dma_addr_t physPtr;

} CAM_DmaBuffer_t;

struct BufQueue {
	CAM_BufData data[MAX_QUEUE_SIZE];
	unsigned int ReadIndex;
	unsigned int WriteIndex;
	spinlock_t lock;
	struct semaphore Sem;
	bool isActive;
	bool isWaitQueue;
};
static short *framebuf = NULL;
static CAM_DmaBuffer_t gDmaBuffer[DMA_LLI_BUFFERS];
static CAM_DmaBuffer_t gCamLinkedList[DMA_LLI_BUFFERS];
static CAM_DmaBuffer_t gCamBootBuffer;
static CAM_DmaBuffer_t gCamBootBuffer2;
static int cam_bootmem_offset;
static int gDataReady = 0;
struct clk *cam_clk = NULL;
static struct BufQueue g_WriteBufQueue;
static struct BufQueue g_ReadBufQueue;
static CAM_BufData g_CurrData;
static struct regulator *cam_regulator = NULL;

/* ---- Private Function Prototypes -------------------------------------- */

static irqreturn_t dma_isr(void *unused);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19)
static irqreturn_t vsync_isr(int irq, void *dev_id, struct pt_regs *regs);
#else
static irqreturn_t vsync_isr(int irq, void *dev_id);
#endif
/*static int camSendCmdSequence( cam_cmd_t *cmdSeq );*/
static void camStartDmaTransfer(int index);
static void camStopDmaTransfer(void);
static HAL_CAM_Result_en_t camDmaCreateList(int line_bytes, int lines,
					    DMADRV_LLITABLE_MODE_t mode);
static HAL_CAM_Result_en_t CAM_UpdateParm(CAM_Parm_t *parmp);
static inline void checkCameraID(CamSensorSelect_t sensor);
static int cam_i2c_command(struct i2c_client *device, unsigned int cmd,
			   void *arg);
static void writeFrame(void *bufferp, void *arg);
static int processFrame(void *unused);
static int cam_pm_update(PM_CompPowerLevel compPowerLevel,
			 PM_PowerLevel sysPowerLevel);

/****************************************************************************************/
/*       Hal Cam Function Prototypes */
/****************************************************************************************/
/* Camera Sensor Configuration -----------------------------------------*/
static HAL_CAM_Result_en_t CAM_PowerUp(CamSensorSelect_t sensor);
static HAL_CAM_Result_en_t CAM_PowerDown(CamSensorSelect_t sensor);
static HAL_CAM_Result_en_t CAM_Standby(CamSensorSelect_t sensor);
static HAL_CAM_Result_en_t CAM_Enable(CamSensorSelect_t sensor);
static HAL_CAM_Result_en_t CAM_Disable(CamSensorSelect_t sensor);
static HAL_CAM_Result_en_t Cam_SensorIntfSeqSel(CamSensorSelect_t nSensor,
						CamSensorSeqSel_t nSeqSel);
static HAL_CAM_Result_en_t Cam_SensorCntrlSeq(CamSensorIntfCntrl_st_t *seq,
					      UInt32 length);
static HAL_CAM_Result_en_t camSetUpCapture(CamSensorSelect_t Sensor);

/* Camera Baseband Configuration -----------------------------------------*/
static HAL_CAM_Result_en_t Cam_InitVsyncIrq(CamIntfConfig_st_t *
					    pSyncIntfConfig);
static HAL_CAM_Result_en_t Cam_CfgImageCapture(CamImageSize_t resolution,
					       CamCaptureMode_t mode,
					       CamSensorSelect_t sensor);
static HAL_CAM_Result_en_t Cam_CfgWindowScale(CamWindowConfig_st_t *
					      pcam_window_config);
static HAL_CAM_Result_en_t Cam_CfgDma(CamWindowConfig_st_t *pcam_window_config,
				      DMADRV_LLITABLE_MODE_t mode,
				      Boolean repeat);
static HAL_CAM_Result_en_t Cam_ConfigCamSensorZoom(Boolean forPreview,
						   CamSensorSelect_t nSensor);

/* Camera Controller Function Prototypes -----------------------------------------*/
static HAL_CAM_Result_en_t Cam_CfgCmiController(CamSensorSeqSel_t mode,
						CamIntfConfig_st_t *
						pCmiIntfConfig);
/* I2C Configuration -----------------------------------------*/
static HAL_CAM_Result_en_t Cam_CfgSensorI2c(CamIntfConfig_st_t *
					    pI2cIntfConfig);
#if defined(CONFIG_BCM_CAM_STV0986)
static int Cam_MarkerSearch(void *buf);
#endif

/****************************************************************************************/
/****************************************************************************************/

static int cam_i2c_probe(struct i2c_client *client,
			 const struct i2c_device_id *dev_id)
{
	pr_debug("cam_i2c_attach_adapter start\n");
	cam_i2c_datap = kmalloc(sizeof(*cam_i2c_datap), GFP_KERNEL);
	memset(cam_i2c_datap, 0, sizeof(*cam_i2c_datap));

	pr_debug("cam_i2c_attach_adapter successful\n");
	i2c_set_clientdata(client, cam_i2c_datap);
	cam_i2c_datap->client = client;

	return 0;
}

static int cam_i2c_remove(struct i2c_client *client)
{
	kfree(cam_i2c_datap);
	cam_i2c_datap = NULL;
	return 0;
}

struct i2c_driver i2c_driver_cam = {
	.driver = {
		   .name = IF_NAME,
		   },
	.id_table = cam_i2c_id_table,
	.probe = cam_i2c_probe,
	.remove = cam_i2c_remove,
	.command = cam_i2c_command,
};

static PM_Comp_Ops_t cam_pm_ops = {
      update_power_level: &cam_pm_update,
};

/* ---- Functions -------------------------------------------------------- */
static int64_t systemTime(void)
{
	struct timespec t;
	t.tv_sec = t.tv_nsec = 0;
	ktime_get_ts(&t);
	return (int64_t) (t.tv_sec) * 1000000000LL + t.tv_nsec;
}

/*initial stage for queue*/
static void InitQueue(struct BufQueue *queue)
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
	}
	g_CurrData.id = -1;
	g_CurrData.busAddress = NULL;
	g_CurrData.timestamp = 0;
}

/*de-initial stage for queue*/
static void deinitQueue(struct BufQueue *queue)
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

/*enqueue buffer*/
static void PushQueue(struct BufQueue *queue, CAM_BufData * buf)
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
		pr_debug("Push datat fail!!: Camera  Data Queue Full!\n");
	} else {
		/*pr_debug("PushQueue Index %d  Address 0x%x\n",buf->id ,buf->busAddress);*/
		queue->data[queue->WriteIndex].busAddress = buf->busAddress;
		queue->data[queue->WriteIndex].id = buf->id;
		queue->data[queue->WriteIndex].timestamp = buf->timestamp;
		queue->WriteIndex = NextIndex;
	}
	spin_unlock_irqrestore(&queue->lock, stat);
}

/*dequeue buffer*/
static bool PullQueue(struct BufQueue *queue, CAM_BufData * buf)
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
		pr_debug("Pull Queue failed!! Camera  Data Queue  Emptyl!\n");
		buf->busAddress = NULL;
		buf->id = -1;
	} else {
		buf->busAddress = queue->data[queue->ReadIndex].busAddress;
		buf->id = queue->data[queue->ReadIndex].id;
		buf->timestamp = queue->data[queue->ReadIndex].timestamp;
		queue->ReadIndex = (queue->ReadIndex + 1) & MAX_QUEUE_SIZE_MASK;
		/*pr_debug(" PullQueue Index %d  Address 0x%x\n",buf->id ,buf->busAddress);*/
	}
	spin_unlock_irqrestore(&queue->lock, stat);
	return true;
}

/*push buffer to queue and wakeup WaitPullQueue*/
static void WakeupPushQueue(struct BufQueue *queue, CAM_BufData * buf)
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
		pr_debug
		    ("Wake Push datat failed!!l Camera  Data Queue Full!\n");
	} else {
		/*pr_debug(" WakeupPushQueue Index %d  Address 0x%x\n",buf->id ,buf->busAddress);*/
		queue->data[queue->WriteIndex].busAddress = buf->busAddress;
		queue->data[queue->WriteIndex].id = buf->id;
		queue->data[queue->WriteIndex].timestamp = systemTime();
		queue->WriteIndex = NextIndex;
	}

	queue->isWaitQueue = true;
	spin_unlock_irqrestore(&queue->lock, stat);
	up(&queue->Sem);

}

/*pull buffer from queue if have any available data buffer*/
static bool WaitPullQueue(struct BufQueue *queue, CAM_BufData * buf)
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
		/*pr_debug(" WaitPullQueue Index %d  Address 0x%x\n",buf->id ,buf->busAddress);*/
		buf->busAddress = queue->data[queue->ReadIndex].busAddress;
		buf->id = queue->data[queue->ReadIndex].id;
		buf->timestamp = queue->data[queue->ReadIndex].timestamp;
		queue->ReadIndex = (queue->ReadIndex + 1) & MAX_QUEUE_SIZE_MASK;
	}
	queue->isWaitQueue = true;
	spin_unlock_irqrestore(&queue->lock, stat);
	return true;
}

/****************************************************************************
*
*  dma_isr
*
*  This isr is triggered when a camera to memory transfer has completed.
*
***************************************************************************/
static irqreturn_t dma_isr(void *unused)
{

	gCamState.dmaCam++;
	pr_debug("%s(): gCamState.dmaCam=%d \r\n", __FUNCTION__,
		 gCamState.dmaCam);
	pr_debug("Camera - dma_isr()\n");
	return IRQ_HANDLED;
}

/****************************************************************************
*
*  processFrame
*
*  A blocking function that processes data after each
*  frame is captured by running a callback function.
*
***************************************************************************/
static int processFrame(void *unused)
{
	void *ptr;

	/*         processFrame is not a kernel thread anymore
	 *         Just a regular blocking function invoked from process context
	 *         Since process context can sleep, we don't see any need for a separate
	 *         kernel thread to carry-out JPEG processing */

	/* While(1) and break is required as the STV sensor operates that way */
	while (1) {
		/* Block waiting to be signalled by either VSYNC ISR or camera disable */
		if (0 == wait_event_interruptible(gDataReadyQ, gDataReady)) {
			gDataReady = 0;
			/* A Valid wake-up by VSYNC ISR has gProcessFrameRunning = 1.
			 * The disable method wakes up with gProcessFrameRunning = 0 which means exit */
			if (!gProcessFrameRunning)
				break;
			atomic_set(&captured, 1);

			if ((sCamDmaMode == DMADRV_STILLS_MODE)
			    && (sCamState == CAM_STATE_PAUSE)
			    && !sCaptureJpegSize) {
				sCaptureJpegSize =
				    CAMDRV_GetJpegSize(sCurrentSensor,
						       gDmaBuffer[0].virtPtr);
#if defined(CONFIG_BCM_CAM_STV0986)
				if (sCaptureFrameCountdown >= -15) {
					if (Cam_MarkerSearch
					    (gDmaBuffer[0].virtPtr)) {
						sCaptureJpegSize = 0;
						atomic_set(&captured, 0);
						continue;
					}
				}
#endif
			}
			/* call the callback function and pass data to it */
			if (gCamConfig.frameFunc != 0) {
				/* get a pointer to the DMA buffer */
				if (sCamDmaMode == DMADRV_STILLS_MODE)
					ptr = gDmaBuffer[0].virtPtr;

				(*gCamConfig.frameFunc) (ptr,
							 gCamConfig.
							 frameFuncArg);
				/*pr_debug("CAM - processFrame done\n");*/

				gCamConfig.frameFunc = NULL;
				gCamConfig.frameFuncArg = NULL;
				atomic_set(&captured, 0);
				/* Enable returning to the process context/ioctl*/
				break;
			}
		} else {
			/* TODO check what type of signal interrupted us */
			pr_debug("CAM - processFrame Unknown\n");
			break;
		}
	}
	return 0;
}

/****************************************************************************
*
*  writeFrame
*
*  A callback to write frame to user buffer
*
*  bufferp: pointer to frame data from the camera (input)
*  arg: pointer to callback argument, user argument of frame
*
***************************************************************************/
static void writeFrame(void *bufferp, void *arg)
{
	CAM_Frame1_t *frame;
	int height, width, length;
	UInt16 *ptr;

	frame = (CAM_Frame1_t *) arg;

	if (sCamDmaMode != DMADRV_STILLS_MODE) {
		width = sCamWidth >> 1;
		height = sCamHeight;

		if (frame->len > (height * width))
			frame->len = height * width;
	} else {
		ptr = (UInt16 *) bufferp;
		length = (sCaptureJpegSize + 1) >> 1;	/* length in u16 */
		if (frame->len) {
			frame->len = (frame->len >= length) ? length : 0;
		} else {
			frame->len = length;
		}
	}

	/* get a pointer to the DMA buffer */
	framebuf = (short *)bufferp;

}

/****************************************************************************
*
*  vsync_isr
*
*  This ISR triggers on vertical sync and begins the DMA transfer
*  from camera to memory.
*
***************************************************************************/
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19)
static irqreturn_t vsync_isr(int irq, void *dev_id, struct pt_regs *regs)
#else
static irqreturn_t vsync_isr(int irq, void *dev_id)
#endif
{
	(void)irq;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19)
	(void)regs;
#endif

	gCamState.vsyncIrqs++;

	/* Clear the interrupt source */
	CLEAR_VSYNC_INTR();
	CAM_CNTRL_DisableRst();

	if (sCamState != CAM_STATE_PAUSE)
		sCaptureFrameCountdown--;

	if ((sCaptureMode == CamCaptureStill)
	    || (sCaptureMode == CamCaptureStillnThumb)) {
		if (sCamState == CAM_STATE_STILL_CAPTURE_INIT) {
			sCamState = CAM_STATE_STILL_CAPTURE;
			sCaptureJpegSize = 0;
		} else if ((sCamState == CAM_STATE_STILL_CAPTURE)
			   && (sCaptureFrameCountdown <= 0)) {
			sCamState = CAM_STATE_PAUSE;
			/* sCamDmaMissed = Cam_CheckDmaChan((Int32)gCamConfig.dmaChannel, (UInt32)gDmaBuffer[gDblBufInd].physPtr); */
		} else if ((sCamState == CAM_STATE_PAUSE) && !sCaptureJpegSize
			   && (atomic_read(&captured) == 0)) {
			sCamState = CAM_STATE_STILL_CAPTURE;
		}
	} else {
		sCamState = CAM_STATE_VIDEO_CAPTURE;
	}
	sCamState_Save = sCamState;

/* start the next DMA transfer */
	if (sCamState != CAM_STATE_PAUSE) {
		if (sCamDmaMode != DMADRV_STILLS_MODE) {
			/*push the image data we got to read queue */
			WakeupPushQueue(&g_ReadBufQueue, &g_CurrData);
			/*pull available buffer for DMA transfer from write queue*/
			PullQueue(&g_WriteBufQueue, &g_CurrData);

			/*if any buffer avalible for DMA transfer,we start DMA transfer operation.*/
			/*Otherwise, we skip this frame*/
			if (g_CurrData.busAddress) {
				camStartDmaTransfer(g_CurrData.id);
			} else {
				pr_debug("Camera - Drop frame\n");
			}
		} else {
			camStartDmaTransfer(0);
		}
		CAM_CNTRL_Enable();
	} else {
		camStopDmaTransfer();
	}

	if (((sCamState == CAM_STATE_VIDEO_CAPTURE)
	     || (sCamState == CAM_STATE_PAUSE))
	    && (sCaptureFrameCountdown <= 0)) {
		gDataReady = 1;
		/* allow kernel thread to process next frame */
		wake_up_interruptible(&gDataReadyQ);
	}

	return IRQ_HANDLED;
}

/****************************************************************************
*
*  camStartDmaTransfer
*
*  Start the dma transfers.
*
***************************************************************************/
static void camStartDmaTransfer(int index)
{
	int *ptr;

/*    pr_debug( "Camera - camStartDmaTransfer()\n" );*/

	dma_init_chan(gCamConfig.dmaChannel);
	/* pass first linked list entry to the dma controller to start transfer */
	ptr = gCamLinkedList[index].virtPtr;
	dma_setup_chan(gCamConfig.dmaChannel, ptr[0], ptr[1], ptr[2], ptr[3],
		       DMA_FROM_CAM_CONFIG);
}

/****************************************************************************
*
*  camStopDmaTransfer
*
*  Start the dma transfers.
*
***************************************************************************/
static void camStopDmaTransfer(void)
{
/*    pr_debug( "Camera - camStopDmaTransfer()\n" );*/
	dma_disable_chan(gCamConfig.dmaChannel);
}

/****************************************************************************
*
*  camDmaCreateList
*
*  Create the linked lists of DMA transfers, each line requires a DMA transfer
*
***************************************************************************/
static HAL_CAM_Result_en_t camDmaCreateList(int line_bytes, int lines,
					    DMADRV_LLITABLE_MODE_t mode)
{
	int i;
	int *ptr;
	int buffer, buffer_cnt;
	int transfer_size;

	buffer_cnt = (mode == DMADRV_STILLS_MODE) ? 1 : DMA_LLI_BUFFERS;

	transfer_size = (line_bytes >> 2) & REG_DMA_CHAN_CTL_TRANSFER_SIZE_MASK;

	/* create camera to memory linked list */
	for (buffer = 0; buffer < buffer_cnt; buffer++) {
		gCamLinkedList[buffer].sizeInBytes =
		    (lines + 1) * BYTES_PER_LLI;
		gCamLinkedList[buffer].virtPtr =
		    dma_alloc_coherent(NULL, gCamLinkedList[buffer].sizeInBytes,
				       &gCamLinkedList[buffer].physPtr,
				       GFP_KERNEL);

		if (gCamLinkedList[buffer].virtPtr == NULL) {
			pr_debug
			    ("Camera - Failed to allocate memory ( 0x%08x ) for DMA LL buffer %d\n",
			     gCamLinkedList[buffer].sizeInBytes, buffer);
			return HAL_CAM_ERROR_INTERNAL_ERROR;
		}

		ptr = gCamLinkedList[buffer].virtPtr;
		pr_debug
		    ("Camera - camDmaCreateList(): LLptr=0x%x Destptr=0x%x\n",
		     gCamLinkedList[buffer].physPtr,
		     gDmaBuffer[buffer].physPtr);

#if FLIP_IMAGE
		/* There are a few tricky things here.
		 * 1. The camera is upside down, so start at the bottom of the picture and go up.
		 * 2. Using the Vsync interrupt, the first line DMA'ed will be the last line
		 *    from the last frame.  So set up the descriptors to over-write the first
		 *    line that is received.
		 * 3. The LLI data must be stored little endian when DMA is configured for big
		 *    endian.  The first table entry is kept as big endian since it is read
		 *    directly into the control registers.
		 */
		ptr[0] = REG_CAM_DATR_PADDR;
		ptr[1] = gDmaBuffer[buffer].physPtr + line_bytes * (lines - 1);
		ptr[2] = gCamLinkedList[buffer].physPtr + BYTES_PER_LLI;
		ptr[3] = DMA_FROM_CAM_CONTROL | transfer_size;

		ptr += INTS_PER_LLI;

		for (i = 1; i < lines - 1; i++) {
			int j;

			j = lines - 1 - i;
			ptr[0] = SWAP(REG_CAM_DATR_PADDR);
			ptr[1] =
			    SWAP(gDmaBuffer[buffer].physPtr + line_bytes * j);
			ptr[2] =
			    SWAP(gCamLinkedList[buffer].physPtr +
				 BYTES_PER_LLI * (i + 1));
			ptr[3] = SWAP(DMA_FROM_CAM_CONTROL | transfer_size);
			ptr += INTS_PER_LLI;
		}

		/* create the last LLI */
		ptr[0] = SWAP(REG_CAM_DATR_PADDR);
		ptr[1] = SWAP(gDmaBuffer[buffer].physPtr);
		ptr[2] = 0;
		ptr[3] =
		    SWAP(DMA_FROM_CAM_CONTROL | transfer_size |
			 REG_DMA_CHAN_CTL_TC_INT_ENABLE);
#else
		/* Using the Vsync interrupt, the first line DMA'ed will be the last line
		 * from the last frame.  So set up the descriptors to over-write the first
		 * line that is received.
		 * The LLI data must be stored little endian when DMA is configured for big
		 * endian.  The first table entry is kept as big endian since it is read
		 * directly into the control registers.
		 */
		ptr[0] = REG_CAM_DATR_PADDR;
		ptr[1] = gDmaBuffer[buffer].physPtr;
		ptr[2] = gCamLinkedList[buffer].physPtr + BYTES_PER_LLI;
		ptr[3] = DMA_FROM_CAM_CONTROL | transfer_size;
		ptr += INTS_PER_LLI;

		/* create all but the last LLI */
		for (i = 1; i < lines - 1; i++) {
			ptr[0] = SWAP(REG_CAM_DATR_PADDR);
			ptr[1] =
			    SWAP(gDmaBuffer[buffer].physPtr + (line_bytes * i));
			ptr[2] =
			    SWAP(gCamLinkedList[buffer].physPtr +
				 (BYTES_PER_LLI * (i + 1)));
			ptr[3] = SWAP(DMA_FROM_CAM_CONTROL | transfer_size);
			ptr += INTS_PER_LLI;
		}

		/* create the last LLI */
		ptr[0] = SWAP(REG_CAM_DATR_PADDR);
		ptr[1] = SWAP(gDmaBuffer[buffer].physPtr + line_bytes * i);
		ptr[2] = 0;
		ptr[3] =
		    SWAP(DMA_FROM_CAM_CONTROL | transfer_size |
			 REG_DMA_CHAN_CTL_TC_INT_ENABLE);
#endif
	}

	pr_debug("Camera - camDmaCreateList(): SrcPtr=0x%x Cntrl=0x%x\n",
		 REG_CAM_DATR_PADDR, (DMA_FROM_CAM_CONTROL | transfer_size));

	return 0;
}

/****************************************************************************
*
*  cam_open
*
***************************************************************************/
static int cam_open(struct inode *inode, struct file *file)
{
	HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
	int rc = 0;

	pr_debug("Camera - %s called\n", __FUNCTION__);

/* Set Selected Camera Sensor*/
	sCurrentSensor = CamSensorPrimary;	/* Select main sensor */
/* Get Selected Camera Sensor Interface Configuration */
	pSensorIntfConfig = CAMDRV_GetIntfConfig(sCurrentSensor);
	if (pSensorIntfConfig == NULL) {
		pr_debug("%s(): CAMDRV_GetIntfConfig(): ERROR \r\n",
			 __FUNCTION__);
		result |= HAL_CAM_ERROR_INTERNAL_ERROR;
	}
/* Power-Up Selected Camera Sensor */
	if (CAM_PowerUp(sCurrentSensor) != HAL_CAM_SUCCESS) {
		result |= HAL_CAM_ERROR_INTERNAL_ERROR;
	}
/* Wake Up Camera Device */
	if (CAM_Standby(sCurrentSensor) != HAL_CAM_SUCCESS) {
		result |= HAL_CAM_ERROR_INTERNAL_ERROR;
		pr_debug("%s(): CAM_Standby(): ERROR: \r\n", __FUNCTION__);
	}
/* Initialize Vsync and Hsync Interface to Camera Sensor */
	if (Cam_InitVsyncIrq(pSensorIntfConfig) != HAL_CAM_SUCCESS) {
		result |= HAL_CAM_ERROR_INTERNAL_ERROR;
	}
/* Check Return Code */
	if (result != HAL_CAM_SUCCESS) {
		rc = -EFAULT;
		pr_debug("%s(): ERROR \r\n", __FUNCTION__);
	}

	if (rc >= 0)
		wake_lock(&camera_wake_lock);
	else
		clk_disable(cam_clk);

	return rc;
}				/* cam_open */

/****************************************************************************
*
*  cam_read
*
***************************************************************************/

static ssize_t cam_read(struct file *file, char *buffer, size_t count,
			loff_t *ppos)
{
	pr_debug("cam_read called, major = %d, minor = %d\n",
		 MAJOR(file->f_dentry->d_inode->i_rdev),
		 MINOR(file->f_dentry->d_inode->i_rdev));

	return -EINVAL;

}				/* cam_read */

/****************************************************************************
*
*  cam_release
*
***************************************************************************/
static int cam_release(struct inode *inode, struct file *file)
{
	HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
	int rc = 0;

	pr_debug("Camera - %s called\n", __FUNCTION__);
	pr_debug("%s(): Start \r\n", __FUNCTION__);

	if ((sCamState != CAM_STATE_IDLE) && (sCamState != CAM_STATE_OFF))
		camEnable(0);

/*this function puts camera in sleep mode. */
	if (CAMDRV_SetCamSleep(sCurrentSensor) != HAL_CAM_SUCCESS) {
		result |= HAL_CAM_ERROR_INTERNAL_ERROR;
		pr_debug("%s(): CAMDRV_SetCamSleep(): ERROR: \r\n",
			 __FUNCTION__);
	} else {
		pr_debug("%s(): CAMDRV_SetCamSleep() OK \r\n", __FUNCTION__);
	}
	sCamState = CAM_STATE_ON;

/* Power-Down Selected Camera Sensor */
	if (CAM_PowerDown(sCurrentSensor) != HAL_CAM_SUCCESS) {
		result |= HAL_CAM_ERROR_INTERNAL_ERROR;
	}
/* Release Vsync IRQ */
	free_irq(IRQ_VSYNC, NULL);

	if (result == HAL_CAM_SUCCESS) {
		pr_debug("%s(): End  OK\r\n", __FUNCTION__);
	} else {
		rc = -EFAULT;
		pr_debug("%s(): ERROR \r\n", __FUNCTION__);
	}

	wake_unlock(&camera_wake_lock);

	return rc;

}				/* cam_release */

/****************************************************************************
*
*  swap_byte
*
***************************************************************************/

static void swap_byte(u8 *start, int cnt)
{
	u8 val, *end = start + cnt - 1;

	while (start < end) {
		val = *start;
		*start++ = *end;
		*end-- = val;
	}
	return;
}

/****************************************************************************
*
*  cam_ioctl
*
***************************************************************************/

static int cam_ioctl(struct inode *inode, struct file *file, unsigned int cmd,
		     unsigned long arg)
{
	int rc = 0;

	down_interruptible(&gCamDrvSem);
	/*pr_debug("cam_ioctl: type: '%c' cmd: 0x%x\n", _IOC_TYPE( cmd ), _IOC_NR( cmd );*/

	switch (cmd) {
		/* enable or disable the camera */
	case CAM_IOCTL_ENABLE:
		rc = camEnable((int)arg);
		break;

		/* Get camera parameters */
	case CAM_IOCTL_GET_PARAMS:
		{
			CAM_Parm_t parm;

			if (copy_from_user
			    (&parm, (CAM_Parm_t *) arg, sizeof(parm)) != 0) {
				rc = -EFAULT;
				break;
			}

			camGetParm(&parm);

			if (copy_to_user
			    ((CAM_Parm_t *) arg, &parm, sizeof(parm)) != 0)
				rc = -EFAULT;

			break;
		}

		/* Set camera parameters */
	case CAM_IOCTL_SET_PARAMS:
		{
			CAM_Parm_t parm;

			if (copy_from_user
			    (&parm, (CAM_Parm_t *) arg, sizeof(parm)) != 0) {
				rc = -EFAULT;
				break;
			}

			rc = camSetParm(&parm);
			break;
		}

		/* set the frame rate */
	case CAM_IOCTL_SET_FPS:
		{
			stCamParm.fps = (CamRates_t) arg;
			/* Set New Frame Rate if Streaming */
			if (gCamConfig.enable) {
				if (CAMDRV_SetFrameRate
				    (stCamParm.fps,
				     sCurrentSensor) != HAL_CAM_SUCCESS) {
					rc = -EFAULT;
					pr_debug
					    ("CAM_IOCTL_SET_FPS: CAMDRV_SetFrameRate(): ERROR: \r\n");
				}
			}
			break;
		}

	case CAM_IOCTL_SET_THUMBNAIL_PARAMS:
		if (copy_from_user
		    (&stThumbnailParm, (CAM_Parm_t *) arg,
		     sizeof(stThumbnailParm)) != 0)
			rc = -EFAULT;

		break;

	case CAM_IOCTL_MEM_REGISTER:
		{
			CAM_BufData phydata;
			if (copy_from_user
			    (&phydata, (CAM_BufData *) arg,
			     sizeof(CAM_BufData)) != 0) {
				rc = -EFAULT;
				break;
			}
			cam_bootmem_offset = 0;
			gCamBootBuffer.physPtr =
			    (dma_addr_t) phydata.busAddress;
		}
		break;

	case CAM_IOCTL_MEM_BUFFERS:
		{
			CAM_BufData buf;
			if (copy_from_user
			    ((void *)&buf, (const void *)arg,
			     sizeof(CAM_BufData)) != 0) {
				rc = -EFAULT;
			} else {
				if (gDmaBuffer[buf.id].physPtr ==
				    (dma_addr_t) buf.busAddress) {
					PushQueue(&g_WriteBufQueue, &buf);
				} else
					rc = -EFAULT;
			}
			break;
		}
		/* get single frame */
	case CAM_IOCTL_GET_FRAME:
		{
			CAM_BufData buf;
			WaitPullQueue(&g_ReadBufQueue, &buf);
			if (copy_to_user
			    ((void *)arg, (const void *)&buf,
			     sizeof(CAM_BufData)) != 0)
				rc = -EFAULT;
			break;
		}
#if 0
	case CAM_IOCTL_GET_JPEG:
		{
			CAM_Frame_t frame;
			int length;

			if (gCamConfig.frameFunc) {
				rc = -EBUSY;
				break;
			}

			if (!gCamConfig.enable || !gCamConfig.enableInts) {
				rc = -EFAULT;
				break;
			}

			if (down_timeout(&gLock, msecs_to_jiffies(5))) {
				rc = -EINTR;
				break;
			}

			if (wait_event_interruptible(gDataReadyQ, gDataReady)) {
				rc = -EINTR;
				goto done_get_frame;
			}

			gDataReady = 0;
			sCaptureJpegSize = CAMDRV_GetJpegSize(sCurrentSensor);

			length = (sCaptureJpegSize + 1) >> 1;	/* length in u16 */
			frame.thumbnailOffset = (length << 1) + 2;
			frame.thumbnailSize =
			    stThumbnailParm.size_window.end_pixel *
			    stThumbnailParm.size_window.end_line;
			frame.len = (sCaptureJpegSize + 1) >> 1;

			if (copy_to_user
			    ((CAM_Frame_t *) arg, &frame, sizeof(frame)) != 0)
				rc = -EFAULT;

		      done_get_frame:
			up(&gLock);

			break;
		}
#else
		/* get single frame */
	case CAM_IOCTL_GET_FRAME1:
		{
			CAM_Frame1_t frame;
			Boolean copy;

			if (gCamConfig.frameFunc) {
				rc = -EBUSY;
				break;
			}

			if (!gCamConfig.enable || !gCamConfig.enableInts) {
				rc = -EFAULT;
				break;
			}

			if (copy_from_user
			    (&frame, (CAM_Frame1_t *) arg,
			     sizeof(frame)) != 0) {
				rc = -EFAULT;
				break;
			}
			/* ioctl behavior to help still capture with thumbnail
			 input                output
			 frame.len    frame.buffer
			 0        X       frame.len = JPEG size in u16, no copy_to_user
			 Non-Zero     NULL        ioctl failure
			 < JPEG size  Non-Zero    frame.len = 0, no copy_to_user
			 >= JPEG size Non-Zero    frame.len = JPEG size, copy_to_user*/
			if (frame.len)
				copy = TRUE;
			else
				copy = FALSE;

			if (down_timeout(&gLock, msecs_to_jiffies(5))) {
				rc = -EINTR;
				break;
			}

			gCamConfig.frameFunc = writeFrame;
			gCamConfig.frameFuncArg = &frame;
			/* Invoke processFrame as a regular function
			 * Blocks on data ready indication by the VSYNC-ISR */
			processFrame(NULL);

			if (!framebuf) {
				rc = -EFAULT;
				goto done_get_frame;
			}

			if (!frame.buffer && copy) {
				frame.len = 0;
				copy = FALSE;
			}

			if (copy) {
				if (copy_to_user
				    (frame.buffer, framebuf,
				     frame.len * sizeof(unsigned short)) != 0)
					frame.len = 0;
			}

			if (copy_to_user
			    ((CAM_Frame1_t *) arg, &frame, sizeof(frame)) != 0)
				rc = -EFAULT;

		      done_get_frame:
			up(&gLock);

			break;
		}

	case CAM_IOCTL_GET_JPEG:
		{
			CAM_Frame1_t frame;
			int length;
			short *fbuf;
			pr_debug("CAM_IOCTL_GET_JPEG: sCaptureJpegSize=%d\r\n",
				 sCaptureJpegSize);

			if (down_timeout(&gLock, msecs_to_jiffies(5))) {
				rc = -EINTR;
				break;
			}

			if (copy_from_user
			    (&frame, (CAM_Frame1_t *) arg,
			     sizeof(frame)) != 0) {
				rc = -EFAULT;
				goto done_get_jpeg;
			}

			length = (sCaptureJpegSize + 1) >> 1;	/* length in u16 */

			if (!frame.buffer || (frame.len < length)) {
				rc = -EPERM;
				goto done_get_jpeg;
			}

			frame.len = length;

			if (!framebuf) {
				rc = -EFAULT;
				goto done_get_jpeg;
			}
			fbuf = CAMDRV_GetJpeg(framebuf);

			if (copy_to_user
			    (frame.buffer, fbuf,
			     frame.len * sizeof(unsigned short)) != 0)
				frame.len = 0;

			if (copy_to_user
			    ((CAM_Frame1_t *) arg, &frame, sizeof(frame)) != 0)
				rc = -EFAULT;

		      done_get_jpeg:
			up(&gLock);

			break;
		}

	case CAM_IOCTL_GET_THUMBNAIL:
		{
			CAM_Frame1_t frame;
			char *thumbnail;
			int length;

			if (down_timeout(&gLock, msecs_to_jiffies(5))) {
				rc = -EINTR;
				break;
			}

			if (copy_from_user
			    (&frame, (CAM_Frame1_t *) arg,
			     sizeof(frame)) != 0) {
				rc = -EFAULT;
				goto done_get_thumbnail;
			}

			length =
			    stThumbnailParm.size_window.end_pixel *
			    stThumbnailParm.size_window.end_line;
			if (!frame.buffer || (frame.len < length)) {
				rc = -EPERM;
				goto done_get_thumbnail;
			}

			frame.len = length;

			if (!framebuf) {
				rc = -EFAULT;
				goto done_get_thumbnail;
			}

			thumbnail =
			    CAMDRV_GetThumbnail(framebuf, sCaptureJpegSize);

			if (copy_to_user
			    (frame.buffer, thumbnail,
			     frame.len * sizeof(unsigned short)) != 0)
				frame.len = 0;

			if (copy_to_user
			    ((CAM_Frame1_t *) arg, &frame, sizeof(frame)) != 0)
				rc = -EFAULT;

		      done_get_thumbnail:
			up(&gLock);

			break;
		}
#endif
	case CAM_IOCTL_SET_DIGITAL_EFFECT:
		{
			CAM_Parm_t parm;

			if (copy_from_user
			    (&parm, (CAM_Parm_t *) arg, sizeof(parm)) != 0) {
				rc = -EFAULT;
				break;
			}

			if (CAMDRV_SetDigitalEffect
			    (parm.coloreffects,
			     sCurrentSensor) != HAL_CAM_SUCCESS)
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
			if (CAMDRV_SetSceneMode(parm.scenemode, sCurrentSensor)
			    != HAL_CAM_SUCCESS) {
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

			if (CAMDRV_SetWBMode(parm.wbmode, sCurrentSensor) !=
			    HAL_CAM_SUCCESS) {
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
			if (CAMDRV_SetAntiBanding
			    (parm.antibanding,
			     sCurrentSensor) != HAL_CAM_SUCCESS) {
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
			if (CAMDRV_SetFlashMode(parm.focus, sCurrentSensor) !=
			    HAL_CAM_SUCCESS) {
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

			if (CAMDRV_SetFocusMode(parm.focus, sCurrentSensor) !=
			    HAL_CAM_SUCCESS) {
				rc = -EFAULT;
				pr_debug
				    ("CAM_IOCTL_SET_FOCUS_MODE: CAMDRV_SetFocusMode(): ERROR: \r\n");
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

			if (CAMDRV_SetJpegQuality(parm.quality, sCurrentSensor)
			    != HAL_CAM_SUCCESS) {
				rc = -EFAULT;
				pr_debug
				    ("CAM_IOCTL_SET_JPEG_QUALITY: CAMDRV_SetJpegQuality(): ERROR: \r\n");
			}
			break;
		}


	case CAM_IOCTL_GET_RESOLUTION_AVAILABLE:
		{
			CAM_Parm_t parm;
			HAL_CAM_ResolutionSize_st_t sensor_size;

			if (copy_from_user
			    (&parm, (CAM_Parm_t *) arg, sizeof(parm)) != 0)
				rc = -EFAULT;

			CAMDRV_GetResolutionAvailable(parm.size_window.
						      end_pixel,
						      parm.size_window.end_line,
						      parm.mode, sCurrentSensor,
						      &sensor_size);

			parm.size_window.end_pixel = sensor_size.resX;
			parm.size_window.end_line = sensor_size.resY;
			parm.size_window.size = sensor_size.size;

			if (copy_to_user
			    ((CAM_Parm_t *) arg, &parm, sizeof(parm)) != 0)
				rc = -EFAULT;
			break;
		}

		/* read camera register through I2C */
	case CAM_IOCTL_READ_REG:
		{
			HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
			CAM_Reg_t reg;

			if (copy_from_user(&reg, (CAM_Reg_t *) arg, sizeof(reg))
			    != 0) {
				rc = -EFAULT;
				break;
			}

			if (!reg.cnt || (reg.cnt > MAX_I2C_REGISTER_SIZE)) {
				rc = -EFAULT;
				break;
			}

			result =
			    CAM_ReadI2c(reg.reg, reg.cnt,
					(unsigned char *)&reg.val);

			if (result != HAL_CAM_SUCCESS) {
				pr_debug
				    ("CAM_IOCTL_READ_REG: - I2C Read Error: reg=0x%x  val=0x%x\n",
				     reg.reg, reg.val);
				rc = -EFAULT;
				break;
			}
			/* system in little endian, while I2C data in big endian */
			swap_byte((u8 *) &reg.val, reg.cnt);

			if (copy_to_user((CAM_Reg_t *) arg, &reg, sizeof(reg))
			    != 0)
				rc = -EFAULT;

			break;
		}

		/* write camera register through I2C */
	case CAM_IOCTL_WRITE_REG:
		{
			HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
			CAM_Reg_t reg;

			if (copy_from_user(&reg, (CAM_Reg_t *) arg, sizeof(reg))
			    != 0) {
				rc = -EFAULT;
				break;
			}

			if (!reg.cnt || (reg.cnt > MAX_I2C_REGISTER_SIZE)) {
				rc = -EFAULT;
				break;
			}
			/* system in little endian, while I2C data in big endian */
			swap_byte((u8 *) &reg.val, reg.cnt);

			result =
			    CAM_WriteI2c(reg.reg, reg.cnt, (u8 *) &reg.val);

			if (result != HAL_CAM_SUCCESS)
				pr_debug
				    ("CAM_IOCTL_WRITE_REG: - I2C Write Error: reg=0x%x  val=0x%x\n",
				     reg.reg, reg.val);

			break;
		}

	default:
		pr_debug("%s() - Unrecognized ioctl: '0x%x'\n", __FUNCTION__,
			 cmd);
		rc = -ENOTTY;
		break;
	}

	up(&gCamDrvSem);
	return rc;

}				/* cam_ioctl */

/****************************************************************************
*
*  cam_write
*
***************************************************************************/

static ssize_t cam_write(struct file *file, const char *buffer, size_t count,
			 loff_t *ppos)
{
	return -EINVAL;

}				/* cam_write */

/****************************************************************************
*
*   File Operations (these are the device driver entry points)
*
***************************************************************************/

struct file_operations cam_fops = {
      owner:THIS_MODULE,
      open : cam_open,
      release : cam_release,
      ioctl : cam_ioctl,
      read : cam_read,
      write : cam_write,
};

static inline void checkCameraID(CamSensorSelect_t sensor)
{
	u16 devID = CAMDRV_GetDeviceID(sensor);

	/* system in little endian, while I2C data in big endian */
	swap_byte((u8 *) &devID, 2);

	pr_info("[CAM] DeviceID = 0x%04x\n", devID);
}

/****************************************************************************
*
*  cam_init
*
*     Called to perform module initialization when the module is loaded
*
***************************************************************************/

static int __init cam_init(void)
{
	int rc;
	HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;

	pr_info("%s", banner);
	pr_debug("Camera - %s called\n", __FUNCTION__);

	gCamState.level = DBG_DEFAULT_LEVEL;
	HAL_CAM_Set_TraceLevel(HAL_CAM_TRACE_ERROR);
	rc = register_chrdev(BCM_CAM_MAJOR, "camera", &cam_fops);
	if (rc < 0) {
		pr_warning("Camera: register_chrdev failed for major %d\n",
			   BCM_CAM_MAJOR);
		return rc;
	}

	cam_class = class_create(THIS_MODULE, "camera");
	if (IS_ERR(cam_class)) {
		unregister_chrdev(BCM_CAM_MAJOR, "camera");
		return PTR_ERR(cam_class);
	}

	device_create(cam_class, NULL, MKDEV(BCM_CAM_MAJOR, 0), NULL, "camera");

	cam_clk = clk_get(NULL, BCM_CLK_CAMERA_STR_ID);
	if (!cam_clk)
		goto err;

	cam_regulator = regulator_get(NULL, "cam_vdd");
	if (cam_regulator && !IS_ERR(cam_regulator)
	    && !regulator_is_enabled(cam_regulator))
		regulator_enable(cam_regulator);

	wake_lock_init(&camera_wake_lock, WAKE_LOCK_SUSPEND, "camera");

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,21)
	/* register sysctl table */
	gSysCtlHeader = register_sysctl_table(gSysCtl, 0);
	if (gSysCtlHeader != NULL) {
		gSysCtlHeader->ctl_table->child->de->owner = THIS_MODULE;
	}
#else
	gSysCtlHeader = register_sysctl_table(gSysCtl);
#endif

	/* initialize semaphore */
	sema_init(&gCamDrvSem, 1);
	sema_init(&gLock, 0);
	gProcessFrameRunning = 0;

	/* Register our device with the Power Manager */
	if (pm_register_component(PM_COMP_CAMERA, &cam_pm_ops) < 0) {
		pr_debug("Camera - failed to register with power manager\n");
		pr_debug("%s(): pm_register_component(): ERROR \r\n",
			 __FUNCTION__);
		result |= HAL_CAM_ERROR_INTERNAL_ERROR;
	}
/* Set Selected Camera Sensor */
	sCurrentSensor = CamSensorPrimary;	/* Select main sensor */

/* Get Selected Camera Sensor Interface Configuration */
	pSensorIntfConfig = CAMDRV_GetIntfConfig(sCurrentSensor);
	if (pSensorIntfConfig == NULL) {
		pr_debug("%s(): CAMDRV_GetIntfConfig(): ERROR \r\n",
			 __FUNCTION__);
		result |= HAL_CAM_ERROR_INTERNAL_ERROR;
	}
/* Initialize I2C */
	if (Cam_CfgSensorI2c(pSensorIntfConfig) != HAL_CAM_SUCCESS) {
		result |= HAL_CAM_ERROR_INTERNAL_ERROR;
		pr_debug("%s(): Cam_CfgSensorI2c(): ERROR \r\n", __FUNCTION__);
	}
/* Configure Camera Controller for Initialization */
	if (Cam_CfgCmiController(SensorInitPwrUp, pSensorIntfConfig) !=
	    HAL_CAM_SUCCESS) {
		result |= HAL_CAM_ERROR_INTERNAL_ERROR;
		pr_debug("%s(): Cam_CfgCmiController(): ERROR \r\n",
			 __FUNCTION__);
	}
/* Init Power-up Sequence for Sensor */
	if (Cam_SensorIntfSeqSel(sCurrentSensor, SensorInitPwrUp) !=
	    HAL_CAM_SUCCESS) {
		result |= HAL_CAM_ERROR_INTERNAL_ERROR;
		pr_debug("%s(): Cam_SensorIntfSeqSel(): ERROR \r\n",
			 __FUNCTION__);
	}
/* Supplemental Initialization of Camera Sensor (if needed) */
	if (CAMDRV_Supp_Init(sCurrentSensor) != HAL_CAM_SUCCESS) {
		result |= HAL_CAM_ERROR_INTERNAL_ERROR;
		pr_debug("%s(): CAMDRV_Supp_Init(): ERROR \r\n", __FUNCTION__);
	}

	pr_debug("Camera i2_add_driver - %s called\n", __FUNCTION__);
/* normal_i2c[0] = sCamI2cDeviceId; */
	cam_i2c_datap = NULL;
	rc = i2c_add_driver(&i2c_driver_cam);
/*    if ((rc != 0) || (cam_i2c_datap == NULL)) */
	if (rc != 0) {
		pr_debug("Camera - Failed to initialize I2C\n");
		goto err;
	}
/* Check Camera Sensor */
	checkCameraID(sCurrentSensor);

/* Power Down Camera Sensor */
	result |= Cam_SensorIntfSeqSel(sCurrentSensor, SensorPwrDn);
/* Power Down Camera IF Controller */
	result |= Cam_CfgCmiController(SensorPwrDn, pSensorIntfConfig);
/* Camera Current State */
	sCamState = CAM_STATE_OFF;
/* Camera Initialized */
	if (result == HAL_CAM_SUCCESS) {
		sCamInitialized = TRUE;
		pr_debug("%s(): End OK \r\n", __FUNCTION__);
	} else {
		rc = -EFAULT;
		pr_debug("%s(): Failed \r\n", __FUNCTION__);
	}

	pr_debug("CLK_CAMCK_ENABLE=0x%x :%s() \r\n",
		 __REG32(HW_IO_PHYS_TO_VIRT(0x8140004)), __FUNCTION__);
	pr_debug("CLK_CAMCK_MODE=0x%x :%s()\r\n",
		 __REG32(HW_IO_PHYS_TO_VIRT(0x8140008)), __FUNCTION__);
	pr_debug("IOCR3=0x%x :%s()\r\n", __REG32(HW_IO_PHYS_TO_VIRT(0x888001C)),
		 __FUNCTION__);
	pr_debug("SYSCFG_ANACR10=0x%x :%s()\r\n",
		 __REG32(HW_IO_PHYS_TO_VIRT(0x088800a8)), __FUNCTION__);

	/* Allocate memory for the Camera.
	 allocate DMA buffers
	 Presently allocate 1024 * 800 (800KB) of memory. This can be used to share between
	 VF (2 buffers of 153K (320x240x2bpp)
	 DMA Linked list. (2 buffers of size 5136 bytes
	 Still Capture. (1 buffer of size 512 * 1536 = 786KB)
	 Also, this memory can be shared with the Hantro*/
	gCamBootBuffer2.sizeInBytes = CAM_BOOT_TIME_MEMORY_SIZE;
	gCamBootBuffer2.virtPtr = dma_alloc_coherent(NULL,
						     gCamBootBuffer2.
						     sizeInBytes,
						     &gCamBootBuffer2.physPtr,
						     GFP_KERNEL);
	if (gCamBootBuffer2.virtPtr == NULL) {
		pr_debug("Camera - Failed to allocate boot time memory");
		rc = -EFAULT;
	} else {
		pr_debug("Camera - Boot time memory of 0x%x bytes allocated\n",
			 gCamBootBuffer2.sizeInBytes);
	}

#if defined(CONFIG_BCM_CAM_MT9T111)
	/* We store the base address to be used by MT9T111 driver for JPEG header computation.*/
	CAMDRV_StoreBaseAddress(gCamBootBuffer2.virtPtr);
	/* We are reserving 1k for JPEG header in case of MT9T111 sensor.*/
	gCamBootBuffer2.virtPtr += 1024;
	gCamBootBuffer2.physPtr += 1024;
#endif

	cam_bootmem_offset = 0;
	if (cam_regulator && !IS_ERR(cam_regulator)
	    && regulator_is_enabled(cam_regulator))
		regulator_disable(cam_regulator);
	return rc;

      err:
	pr_debug("Camera - Failed to initialize I2C\n");
	device_destroy(cam_class, MKDEV(BCM_CAM_MAJOR, 0));
	class_destroy(cam_class);
	unregister_chrdev(BCM_CAM_MAJOR, "camera");
	return rc;
}				/* cam_init */

/****************************************************************************
*
*  cam_exit
*
*       Called to perform module cleanup when the module is unloaded.
*
***************************************************************************/

static void __exit cam_exit(void)
{
	HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;

	pr_debug("Camera - %s called\n", __FUNCTION__);

	if (sCamState != CAM_STATE_OFF) {
		/* Power-Down Selected Camera Sensor */
		if (CAM_PowerDown(sCurrentSensor) != HAL_CAM_SUCCESS) {
			result |= HAL_CAM_ERROR_INTERNAL_ERROR;
		}
	}

	if (gSysCtlHeader != NULL) {
		unregister_sysctl_table(gSysCtlHeader);
	}

	if (cam_regulator) {
		regulator_put(cam_regulator);
		cam_regulator = NULL;
	}

	wake_lock_destroy(&camera_wake_lock);
	device_destroy(cam_class, MKDEV(BCM_CAM_MAJOR, 0));
	class_destroy(cam_class);
	unregister_chrdev(BCM_CAM_MAJOR, "camera");
}				/* cam_exit */

/****************************************************************************
*
*  camEnableInts
*
***************************************************************************/
int camEnableInts(int level)
{
	if (level) {
		CLEAR_VSYNC_INTR();
		gDataReady = 0;
		enable_irq(IRQ_VSYNC);
	} else {
		disable_irq(IRQ_VSYNC);
	}
	gCamConfig.enableInts = level;
	return 0;
}

/****************************************************************************
*
*  cam_pm_update
*
*     Called by power manager to update component power level
*
***************************************************************************/
static int cam_pm_update(PM_CompPowerLevel compPowerLevel,
			 PM_PowerLevel sysPowerLevel)
{
	int rc = 0;

	switch (compPowerLevel) {
	case PM_COMP_PWR_OFF:
		if (CAM_PowerDown(sCurrentSensor) != HAL_CAM_SUCCESS) {
			rc = -EFAULT;
			pr_debug("%s(): CAM_PowerOff(): ERROR: \r\n",
				 __FUNCTION__);
		}
		break;
	case PM_COMP_PWR_STANDBY:
		/* Wake Up Camera Device */
		if (CAM_Standby(sCurrentSensor) != HAL_CAM_SUCCESS) {
			rc = -EFAULT;
			pr_debug("%s(): CAM_Standby(): ERROR: \r\n",
				 __FUNCTION__);
		}
		break;
	case PM_COMP_PWR_ON:
		if (CAM_PowerUp(sCurrentSensor) != HAL_CAM_SUCCESS) {
			rc = -EFAULT;
			pr_debug("%s(): CAM_PowerOn(): ERROR: \r\n",
				 __FUNCTION__);
		}
		break;
	}
	return rc;
}

/****************************************************************************
*
*  camEnable
*
***************************************************************************/
int camEnable(int level)
{
	HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
	int rc = 0;
	atomic_set(&captured, 0);
	if (level) {
		pr_debug("%s(): TRUE \r\n", __FUNCTION__);
		if (CAM_Enable(sCurrentSensor) != HAL_CAM_SUCCESS) {
			result |= HAL_CAM_ERROR_INTERNAL_ERROR;
			pr_debug("%s(): CAM_Enable(): ERROR \r\n",
				 __FUNCTION__);
		}
	} else {
		pr_debug("%s: FALSE \r\n", __FUNCTION__);
		if (CAM_Disable(sCurrentSensor) != HAL_CAM_SUCCESS) {
			result |= HAL_CAM_ERROR_INTERNAL_ERROR;
			pr_debug("%s(): Cam_Disable():  ERROR: \r\n",
				 __FUNCTION__);
		}
	}

	gCamConfig.enable = level;
	if (result != HAL_CAM_SUCCESS) {
		rc = -EFAULT;
	}
	return rc;
}

/****************************************************************************
*
*  camSetUpCapture
*
***************************************************************************/
static HAL_CAM_Result_en_t camSetUpCapture(CamSensorSelect_t Sensor)
{
	HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;

/* Set up for Image Capture */
	sCaptureFormat = stCamParm.format;
	sCaptureSize = stCamParm.size_window.size;
	memcpy(&stNewCamWindowConfig, &stCamParm.size_window,
	       sizeof stNewCamWindowConfig);

	pr_debug("%s(): stCamParm.mode=%d \r\n", __FUNCTION__, stCamParm.mode);
	switch (stCamParm.mode) {
	case CamPreview:
	case CamVideo:
		sCaptureMode = CamCaptureVideo;
		break;
	case CamVideonViewFinder:
		sCaptureMode = CamCaptureVideonViewFinder;
		break;
	case CamStill:
		sCaptureMode = CamCaptureStill;
		break;
	case CamStillnThumb:
		sCaptureMode = CamCaptureStillnThumb;
		break;
	default:
		sCaptureMode = CamCaptureVideo;
		result |= HAL_CAM_ERROR_ACTION_NOT_SUPPORTED;
		break;
	}

	sSensorOutputSize = sCaptureSize;

/* Set Frames to Discard based on Capture mode */
	if ((sCaptureMode == CamCaptureVideo)
	    || (sCaptureMode == CamCaptureVideonViewFinder)) {
		sCaptureFrameCountdown = pSensorIntfConfig->sensor_config_caps->output_st.pre_frame_video;	/* Throw out 1st x frames in video capture */
	} else {
		sCaptureFrameCountdown = pSensorIntfConfig->sensor_config_caps->output_st.pre_frame_still;	/* Throw out 1st x frames in stills image capture */
	}

/* Set Capture size & mode */
	result |= Cam_CfgImageCapture(sCaptureSize, sCaptureMode, Sensor);
	if (result != HAL_CAM_SUCCESS) {
		pr_debug("%s():  Cam_CfgImageCapture():  ERROR: \r\n",
			 __FUNCTION__);
	}
/* Setup camera controller for format conversion (YCbCr in) */
	result |= CAM_CNTRL_SetDataFmt(sCaptureFormat);	/* Camera Controller conversion */
	return result;
}

/****************************************************************************
*
*  camGetParm
*
*  Get private camera parameters.
*
*  parmp: copy camera parameter structure (output)
*
***************************************************************************/
void camGetParm(CAM_Parm_t *parmp)
{
	memcpy(parmp, &stCamParm, sizeof(stCamParm));
}

/****************************************************************************
*
*  camSetParm
*
*  Set public camera parameters.
*
*  parmp: pointer to camera parameter structure (input)
*
***************************************************************************/
int camSetParm(CAM_Parm_t *parmp)
{
	int rc = 0;

	if (CAM_UpdateParm(parmp) != HAL_CAM_SUCCESS) {
		rc = -EFAULT;
		pr_debug("%s(): ERROR: \r\n", __FUNCTION__);
	}
	return rc;
}

/****************************************************************************
*
*  CAM_UpdateParm
*
***************************************************************************/
static HAL_CAM_Result_en_t CAM_UpdateParm(CAM_Parm_t *parmp)
{
	HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;

	memcpy(&stCamParm, parmp, sizeof(stCamParm));

/* Restart Streaming if Camera Enabled */
	if (gCamConfig.enable) {
		if (CAM_Disable(sCurrentSensor) != HAL_CAM_SUCCESS) {
			result |= HAL_CAM_ERROR_INTERNAL_ERROR;
			pr_debug("%s(): Cam_Disable():  ERROR: \r\n",
				 __FUNCTION__);
		}
		if (CAM_Enable(sCurrentSensor) != HAL_CAM_SUCCESS) {
			result |= HAL_CAM_ERROR_INTERNAL_ERROR;
			pr_debug("%s(): CAM_Enable(): ERROR \r\n",
				 __FUNCTION__);
		}
	}
	return result;
}

/****************************************************************************
*
*  cam_i2c_command
*
***************************************************************************/
static int cam_i2c_command(struct i2c_client *device, unsigned int cmd,
			   void *arg)
{
	return 0;
}

/*!
 Function Name:  CAM_PowerUp

 Description:  Power-Up camera controller and sensor

 \param      none

 \return     HAL_CAM_Result_en_t

 \note       Static:
*/
static HAL_CAM_Result_en_t CAM_PowerUp(CamSensorSelect_t sensor)
{
	HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
	CamIntfConfig_st_t *psensor_intf_config;
	int Count;
	pr_debug("Camera - %s called, sCamState = %d\n", __FUNCTION__,
		 sCamState);

	if (cam_regulator && !IS_ERR(cam_regulator)
	    && !regulator_is_enabled(cam_regulator))
		regulator_enable(cam_regulator);
/* Initialize control structure */
	gCamState.vsyncIrqs = 0;
	gCamState.dmaCam = 0;
	gCamState.blocked = 0;
	gCamConfig.frameFunc = NULL;
	gCamConfig.frameFuncArg = NULL;

	for (Count = 0; Count < DMA_LLI_BUFFERS; Count++) {
		gDmaBuffer[Count].virtPtr = NULL;
		gCamLinkedList[Count].virtPtr = NULL;
	}

/* Get Selected Camera Sensor Interface Configuration */
	psensor_intf_config = CAMDRV_GetIntfConfig(sensor);
	if (psensor_intf_config == NULL) {
		pr_debug("%s(): CAMDRV_GetIntfConfig(): ERROR \r\n",
			 __FUNCTION__);
		result |= HAL_CAM_ERROR_INTERNAL_ERROR;
	}
/* Disable Flash */
	if (CAM_SetFlashEnable(FALSE) != HAL_CAM_SUCCESS) {
		pr_debug("%s(): CAM_SetFlashEnable(): ERROR \r\n",
			 __FUNCTION__);
		result |= HAL_CAM_ERROR_INTERNAL_ERROR;
	}
/* Configure Camera Controller for Initialization */
	if (Cam_CfgCmiController(SensorInitPwrUp, psensor_intf_config) !=
	    HAL_CAM_SUCCESS) {
		pr_debug("%s(): Cam_CfgCmiController(): ERROR \r\n",
			 __FUNCTION__);
		result |= HAL_CAM_ERROR_INTERNAL_ERROR;
	}
/* Init Power-up Sequence for Sensor (if needed) */
	if (Cam_SensorIntfSeqSel(sensor, SensorInitPwrUp) != HAL_CAM_SUCCESS) {
		result |= HAL_CAM_ERROR_INTERNAL_ERROR;
		pr_debug("%s(): Cam_SensorIntfSeqSel(): ERROR \r\n",
			 __FUNCTION__);
	}
/* Supplemental Initialization of Camera Sensor (if needed) */
	if (CAMDRV_Supp_Init(sensor) != HAL_CAM_SUCCESS) {
		result |= HAL_CAM_ERROR_INTERNAL_ERROR;
		pr_debug("%s(): CAMDRV_Supp_Init(): ERROR \r\n", __FUNCTION__);
	}
/* Camera Current State */
	sCamState = CAM_STATE_ON;
	return result;
}

/*
! Function Name:  CAM_PowerDown
!
! Description:  Power-Down camera controller and sensor
!
! \param      none
!
! \return     HAL_CAM_Result_en_t
!
! \note       Static:
*/
static HAL_CAM_Result_en_t CAM_PowerDown(CamSensorSelect_t sensor)
{
	HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
	CamIntfConfig_st_t *psensor_intf_config;

	pr_debug("Camera - %s called, sCamState = %d\n", __FUNCTION__,
		 sCamState);
	pr_debug("%s(): Start \r\n", __FUNCTION__);

/* Get Selected Camera Sensor Interface Configuration */
	psensor_intf_config = CAMDRV_GetIntfConfig(sensor);
	if (psensor_intf_config == NULL) {
		pr_debug("%s(): CAMDRV_GetIntfConfig(): ERROR: \r\n",
			 __FUNCTION__);
		result |= HAL_CAM_ERROR_INTERNAL_ERROR;
	}
/* Test for proper state */
	if ((sCamState != CAM_STATE_OFF) && (sCamState != CAM_STATE_ON)) {
		if (sCamState != CAM_STATE_ON) {
			result |= HAL_CAM_ERROR_INTERNAL_ERROR;
			pr_debug("%s(): ERROR:  Camera not in ON state \r\n",
				 __FUNCTION__);
		}
		/* this function puts camera in sleep mode. */
		if (CAMDRV_SetCamSleep(sensor) != HAL_CAM_SUCCESS) {
			result |= HAL_CAM_ERROR_INTERNAL_ERROR;
			pr_debug("%s(): CAMDRV_SetCamSleep(): ERROR: \r\n",
				 __FUNCTION__);
		} else {
			pr_debug("%s(): CAMDRV_SetCamSleep() OK \r\n",
				 __FUNCTION__);
		}
	}
/* Power Down Camera Sensor */
	result |= Cam_SensorIntfSeqSel(sensor, SensorPwrDn);
/* Power Down Camera IF Controller */
	result |= Cam_CfgCmiController(SensorPwrDn, psensor_intf_config);
	sCamState = CAM_STATE_OFF;
	if (cam_regulator && !IS_ERR(cam_regulator)
	    && regulator_is_enabled(cam_regulator))
		regulator_disable(cam_regulator);
	return result;
}				/* CAM_PowerDown */

/*
! Function Name:  CAM_Standby
!
! Description:  CAM_Standby state for sensor
!
! \param      none
!
! \return     HAL_CAM_Result_en_t
!
! \note       Static:
*/
static HAL_CAM_Result_en_t CAM_Standby(CamSensorSelect_t sensor)
{
	HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
	pr_debug("Camera - %s called, sCamState = %d\n", __FUNCTION__,
		 sCamState);

/* Test for proper state */
	if (sCamState != CAM_STATE_ON) {
		result |= HAL_CAM_ERROR_INTERNAL_ERROR;
		pr_debug("%s(): ERROR:  Camera not in ON state \r\n",
			 __FUNCTION__);
	}
/* Wake Up Camera Device */
	if (CAMDRV_Wakeup(sensor) != HAL_CAM_SUCCESS) {
		result |= HAL_CAM_ERROR_INTERNAL_ERROR;
		pr_debug("%s(): CAMDRV_Wakeup(): ERROR: \r\n", __FUNCTION__);
	}
	sCamState = CAM_STATE_IDLE;
	sCamState_Save = sCamState;
	pr_debug("%s(): End \r\n", __FUNCTION__);
	return result;
}

/*
! Function Name:  CAM_Enable
!
! Description:  Enables camera controller and sensor, puts in Streaming state
!
! \param      none
!
! \return     HAL_CAM_Result_en_t
!
! \note       Static:
*/
static HAL_CAM_Result_en_t CAM_Enable(CamSensorSelect_t sensor)
{
	HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
	int Count;

	pr_debug("Camera - %s called, sCamState = %d\n", __FUNCTION__,
		 sCamState);

	if (sCamState == CAM_STATE_IDLE) {

		/* Initialize control structure */
		gCamState.vsyncIrqs = 0;
		gCamState.dmaCam = 0;
		gCamState.blocked = 0;
		gCamConfig.frameFunc = NULL;
		gCamConfig.frameFuncArg = NULL;

		for (Count = 0; Count < DMA_LLI_BUFFERS; Count++) {
			gDmaBuffer[Count].virtPtr = NULL;
			gCamLinkedList[Count].virtPtr = NULL;
		}

		/* Set up for Camera Capture */
		if (camSetUpCapture(sCurrentSensor) != HAL_CAM_SUCCESS) {
			result |= HAL_CAM_ERROR_INTERNAL_ERROR;
			pr_debug("%s(): camSetUpCapture(): ERROR \r\n",
				 __FUNCTION__);
		}

		if (sCaptureMode == CamCaptureStillnThumb) {
			if (CAMDRV_CfgStillnThumbCapture
			    (sCaptureSize, sCaptureFormat,
			     stThumbnailParm.size_window.size,
			     stThumbnailParm.format,
			     sCurrentSensor) != HAL_CAM_SUCCESS) {
				result |= HAL_CAM_ERROR_INTERNAL_ERROR;
				pr_debug
				    ("%s(): CAMDRV_CfgStillnThumbCapture():  ERROR: \r\n",
				     __FUNCTION__);
			}
			sCamState = CAM_STATE_STILL_CAPTURE_INIT;

		} else {

			/* Configure Video Streaming mode */
			if (CAMDRV_SetVideoCaptureMode
			    (sSensorOutputSize, sCaptureFormat,
			     sCurrentSensor) != HAL_CAM_SUCCESS) {
				result |= HAL_CAM_ERROR_INTERNAL_ERROR;
				pr_debug
				    ("%s(): CAMDRV_SetVideoCaptureMode():  ERROR: \r\n",
				     __FUNCTION__);
			} else {
				/* Set Frame Rate */
				if (CAMDRV_SetFrameRate
				    (stCamParm.fps,
				     sCurrentSensor) != HAL_CAM_SUCCESS) {
					result |= HAL_CAM_ERROR_INTERNAL_ERROR;
					pr_debug
					    ("%s(): CAMDRV_SetFrameRate(): ERROR: \r\n",
					     __FUNCTION__);
				}
				/* Enable Camera Video streaming Mode */
				if (CAMDRV_EnableVideoCapture(sCurrentSensor) !=
				    HAL_CAM_SUCCESS) {
					result |= HAL_CAM_ERROR_INTERNAL_ERROR;
					pr_debug
					    ("%s(): CAMDRV_EnableVideoCapture(): ERROR: \r\n",
					     __FUNCTION__);
				} else {
					pr_debug
					    ("%s(): CAMDRV_EnableVideoCapture():  OK \r\n",
					     __FUNCTION__);

					/* Set Camera Sensor Zoom if Capable */
					if (Cam_ConfigCamSensorZoom
					    (TRUE,
					     sCurrentSensor) !=
					    HAL_CAM_SUCCESS) {
						result |=
						    HAL_CAM_ERROR_INTERNAL_ERROR;
						pr_debug
						    ("%s(): Cam_ConfigCamSensorZoom(): ERROR: \r\n",
						     __FUNCTION__);
					}
				}	/* if (result == HAL_CAM_SUCCESS) CAMDRV_EnableVideoCapture() */
			}	/* else:  if (result == HAL_CAM_SUCCESS) result = CAMDRV_SetVideoCaptureMode(); */
			sCamState = CAM_STATE_VIDEO_CAPTURE_INIT;
		}

		/* Set State for Capture */
		sCamState_Save = sCamState;
		pr_debug
		    ("%s(): image size=0x%x image format=0x%x sCamOutputWidth=%d sCamOutputHeight=%d sCamWidth=%d sCamHeigth=%d \r\n",
		     __FUNCTION__, sCaptureSize, sCaptureFormat,
		     sCamOutputWidth, sCamOutputHeight, sCamWidth, sCamHeight);
		InitQueue(&g_WriteBufQueue);
		InitQueue(&g_ReadBufQueue);

		gDataReady = 0;
		gCamConfig.frameFunc = NULL;

		/* Enable processFrame */
		if (sCamDmaMode == DMADRV_STILLS_MODE) {
			gProcessFrameRunning = 1;
			/* This keeps the processFrame from a premature exit */
		}
		up(&gLock);

		/* Enable Interrupts */
		camEnableInts(1);
	} else {
		result |= HAL_CAM_ERROR_INTERNAL_ERROR;
		pr_debug("%s(): ERROR: NOT IN IDLE STATE state=%d \r\n",
			 __FUNCTION__, sCamState);
	}
	return result;
}

/*
! Function Name:  CAM_Disable
!
! Description:  Halts camera controller and sensor, puts in Idle state
!
! \param      none
!
! \return     HAL_CAM_Result_en_t
!
! \note       Static:
*/
static HAL_CAM_Result_en_t CAM_Disable(CamSensorSelect_t sensor)
{
	HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
	int i;

	pr_debug("Camera - %s called, sCamState = %d\n", __FUNCTION__,
		 sCamState);

	if ((sCamState != CAM_STATE_IDLE) && (sCamState != CAM_STATE_OFF)) {
		if (!down_interruptible(&gLock))
			gCamConfig.frameFunc = NULL;

		/* Disable Interrupts */
		camEnableInts(0);

		/* Allow kernel thread to exit */
		if (sCamDmaMode == DMADRV_STILLS_MODE)
			gProcessFrameRunning = 0;

		deinitQueue(&g_WriteBufQueue);
		deinitQueue(&g_ReadBufQueue);

		gDataReady = 1;
		/* Wake up any blocking function with gProcessFrameRunning = 0 */
		wake_up_interruptible(&gDataReadyQ);

		/* Disable cam ctrl */
		CAM_CNTRL_DisableRst();

		/* Disable DMA */
		dma_init_chan(gCamConfig.dmaChannel);

		pr_debug("%s():  Stop DMA: \r\n", __FUNCTION__);
		if (dma_free_irq(gCamConfig.dmaChannel) != 0) {
			pr_debug("Camera - Failed to get DMA free IRQ \n");
			result |= HAL_CAM_ERROR_INTERNAL_ERROR;
		}

		dma_free_chan(gCamConfig.dmaChannel);

		for (i = 0; i < DMA_LLI_BUFFERS; i++) {
			if (gCamLinkedList[i].virtPtr != NULL) {
				dma_free_coherent(NULL,
						  gCamLinkedList[i].sizeInBytes,
						  gCamLinkedList[i].virtPtr,
						  gCamLinkedList[i].physPtr);
				gCamLinkedList[i].virtPtr = NULL;
				gCamLinkedList[i].physPtr = 0;
			}
		}
		for (i = 0; i < DMA_LLI_BUFFERS; i++) {
			if (gDmaBuffer[i].virtPtr != NULL) {
				gDmaBuffer[i].virtPtr = NULL;
				gDmaBuffer[i].physPtr = 0;
			}
		}
		framebuf = NULL;

		/* Halt Sensor Output */
		if ((sCaptureMode == CamCaptureStill)
		    || (sCaptureMode == CamCaptureStillnThumb)) {
			if (CAMDRV_DisableCapture(sensor) != HAL_CAM_SUCCESS) {
				result |= HAL_CAM_ERROR_INTERNAL_ERROR;
				pr_debug
				    ("%s():  CAMDRV_DisableCapture(): ERROR \r\n",
				     __FUNCTION__);
			}
		} else {
			if (CAMDRV_DisablePreview(sensor) != HAL_CAM_SUCCESS) {
				result |= HAL_CAM_ERROR_INTERNAL_ERROR;
				pr_debug
				    ("%s():  CAMDRV_DisablePreview(): ERROR \r\n",
				     __FUNCTION__);
			}
		}
		/* State to Idle mode */
		sCamState = CAM_STATE_IDLE;
		/* Initialize control structure */
		gCamState.vsyncIrqs = 0;
		gCamState.dmaCam = 0;
		gCamState.blocked = 0;
		gCamConfig.frameFunc = NULL;
		gCamConfig.frameFuncArg = NULL;
	} else {
		result |= HAL_CAM_ERROR_INTERNAL_ERROR;
		pr_debug("%s(): ERROR: NOT IN CORRECT STATE state=%d \r\n",
			 __FUNCTION__, sCamState);
	}
	return result;
}

/*
! Function Name:  Cam_CfgCmiController
!
! Description:  Set up Camera CMI interface.
!
! \param      CamIntfConfig_st_t *pCmiIntfConfig
!
! \return     none
!
! \note       Static:
*/
static HAL_CAM_Result_en_t Cam_CfgCmiController(CamSensorSeqSel_t mode,
						CamIntfConfig_st_t *
						pCmiIntfConfig)
{
	HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;

/* Reset Camera Controller to default states */
	CAM_CNTRL_ResetController();

#if defined( LITTLE_ENDIAN_SWITCH )
/* Byte Swap Input data from Camera Controller */
	CAM_CNTRL_SetByteSwap(TRUE);
#endif

/* Verify valid config pointer */
	if (pCmiIntfConfig == NULL) {
		pr_debug("%s(): ERROR pCmiIntfConfig is NULL \r\n",
			 __FUNCTION__);
		return HAL_CAM_ERROR_INTERNAL_ERROR;
	}

	if ((mode == SensorPwrDn) || (mode == SensorInitPwrDn)) {
		/* Set IOCR registers */
		CAM_CNTRL_IOCR_Cfg(mode);
	} else {
		/* Set IOCR registers */
		CAM_CNTRL_IOCR_Cfg(SensorPwrUp);

		/* Set the Vsync/Hsync for external,embedded */
		if (pCmiIntfConfig->sensor_config_ccir656->ext_sync_enable ==
		    TRUE) {
			CAM_CNTRL_SetSYNC(TRUE);
		}
		/* Modify Vsync level if not default IRQ_ACTIVE_LOW */
		if (pCmiIntfConfig->sensor_config_ccir656->vsync_polarity ==
		    SyncActiveHigh) {
			CAM_CNTRL_SetVsyncInv(TRUE);
		}
		/* Modify Hsync level if not default IRQ_ACTIVE_HIGH */
		/* Set the Hsync level in which to recieve sensor data */
		if (pCmiIntfConfig->sensor_config_ccir656->hsync_polarity ==
		    SyncActiveLow) {
			CAM_CNTRL_SetHsyncInv(TRUE);
		}
		/* Set the Hsync Control if not defautl FALSE */
		if (pCmiIntfConfig->sensor_config_ccir656->hsync_control ==
		    TRUE) {
			CAM_CNTRL_SetHsyncControl(TRUE);
		}
		/* Set the Clock edge to sample data on if not default IRQ_RISING_EDGE */
		if (pCmiIntfConfig->sensor_config_ccir656->data_clock_sample ==
		    SyncFallingEdge) {
			CAM_CNTRL_SetClockInv(TRUE);
		}
		/* Set Color Conversion registers for RGB565 conversion (if needed) */
		CAM_CNTRL_SetColorConversion(pCmiIntfConfig->
					     sensor_config_ycbcr->cc_red,
					     pCmiIntfConfig->
					     sensor_config_ycbcr->cc_green,
					     pCmiIntfConfig->
					     sensor_config_ycbcr->cc_blue);

		/* Set YUV Full Range if not default FALSE */
		if (pCmiIntfConfig->sensor_config_ycbcr->yuv_full_range == TRUE) {
			CAM_CNTRL_SetYuvFullRange(TRUE);
		}
		/* Configure YCbCr Input Format */
		switch (pCmiIntfConfig->sensor_config_ycbcr->sensor_yc_seq) {
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

		pr_debug("%s(): sensor_yc_seq=%d \r\n", __FUNCTION__,
			 pCmiIntfConfig->sensor_config_ycbcr->sensor_yc_seq);

		/* Set Controller Window to maximum image size */
		CAM_CNTRL_SetWindow(0,
				    pCmiIntfConfig->sensor_config_caps->
				    output_st.max_height, 0,
				    pCmiIntfConfig->sensor_config_caps->
				    output_st.max_width);
	}
	return result;
}

/*
! Function Name:  Cam_SensorIntfSeqSel
!
! Description:  Power-Up Camera Sensor
!
! \return     HAL_CAM_Result_en_t
!
*/
static HAL_CAM_Result_en_t Cam_SensorIntfSeqSel(CamSensorSelect_t nSensor,
						CamSensorSeqSel_t nSeqSel)
{
	HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
	UInt32 len = 0;
	CamSensorIntfCntrl_st_t *power_seq = NULL;

	power_seq = CAMDRV_GetIntfSeqSel(nSensor, nSeqSel, &len);	/* get power-up/power-down sequence */
	if ((len != 0) && (power_seq != NULL)) {
		Cam_SensorCntrlSeq(power_seq, len);
	} else {
		pr_debug("%s(): No Sequence\r\n", __FUNCTION__);
	}
	return result;
}

/*
! Function Name:  Cam_SensorCntrlSeq
!
! Description:  Power-Up Camera Sensor
!
! \return     HAL_CAM_Result_en_t
!
*/
static HAL_CAM_Result_en_t Cam_SensorCntrlSeq(CamSensorIntfCntrl_st_t *seq,
					      UInt32 length)
{
	HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
	UInt32 i;

	pr_debug("%s(): \r\n", __FUNCTION__);
	pr_debug("Sequence Length=%ld: \r\n",
		 length / sizeof(CamSensorIntfCntrl_st_t));

	for (i = 0; i < length / sizeof(CamSensorIntfCntrl_st_t); i++) {
		switch (seq[i].cntrl_sel) {
		case GPIO_CNTRL:
			/*gpio_direction_output( seq[i].value, 0 ); */
			if (seq[i].cmd == GPIO_SetHigh) {
				pr_debug("GPIO_CNTRL(%d), GPIO_SetHigh\n",
					 seq[i].value);
				if (gpio_is_valid(seq[i].value))
					gpio_direction_output(seq[i].value, 1);
			} else {
				pr_debug("GPIO_CNTRL(%d), GPIO_SetLow\n",
					 seq[i].value);
				if (gpio_is_valid(seq[i].value))
					gpio_direction_output(seq[i].value, 0);
			}
			break;

		case MCLK_CNTRL:
			if (seq[i].cmd == CLK_TurnOn) {
				pr_debug("MCLK_CNTRL - Freq=%d\n",
					 seq[i].value);
				CAM_CNTRL_SetClkFreq((CamClkSel_t) seq[i].value);	/* Enable camera clock output from BCM baseband chip */
			} else {
				pr_debug("MCLK_CNTRL - NoClk\n", seq[i].value);
				CAM_CNTRL_SetClkFreq(CamDrv_NO_CLK);	/* Disable camera clock output from BCM baseband chip */
			}
			break;

		case PAUSE:
			if (seq[i].value != 0) {
				pr_debug("PAUSE - %d ms\n", seq[i].value);
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

/*
! Function Name:  Cam_CfgSensorI2c
!
! Description:  Set up Camera I2C interface.
!
! \param      CamIntfConfig_st_t *pI2cIntfConfig
!
! \return     HAL_CAM_Result_en_t
!
! \note       Static:
*/
static HAL_CAM_Result_en_t Cam_CfgSensorI2c(CamIntfConfig_st_t *pI2cIntfConfig)
{
	HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;

	pr_debug("%s(): Start\r\n", __FUNCTION__);
	if (pI2cIntfConfig == NULL) {
		pr_debug
		    ("Cam_SetupModule(): Cam_CfgSensorI2c(): ERROR:  pI2cIntfConfig == NULL\r\n");
		return HAL_CAM_ERROR_INTERNAL_ERROR;
	}

	sCamI2cDeviceId = pI2cIntfConfig->sensor_config_i2c->i2c_device_id;	/* I2C Device ID */
	sCamI2cSubAdrOp = pI2cIntfConfig->sensor_config_i2c->i2c_sub_adr_op;	/* I2C sub-address size */
	sCamI2cClockSpeed = pI2cIntfConfig->sensor_config_i2c->i2c_clock_speed;	/* I2C clock speed */
	sCamI2cPageReg = pI2cIntfConfig->sensor_config_i2c->i2c_page_reg;	/* I2C device page register addr */
	sCamI2cMaxPage = pI2cIntfConfig->sensor_config_i2c->i2c_max_page;	/* I2C device max page */
	sCamI2cAccessMode = pI2cIntfConfig->sensor_config_i2c->i2c_access_mode;	/* I2C port */
	/* I2C_ChangeSpeed( (I2C_BUS_ID_t)sCamI2cAccessMode, (I2C_SPEED_t)sCamI2cClockSpeed, (UInt16)sCamI2cDeviceId ); */
	return result;
}

/*
! Function Name:  CAM_SetFlashEnable
!
! Description:  Configure & Set GPIO's to Enable/Disable Camera Flash
!
! \param      enable    Boolean:  TRUE=enable flash   FALSE=disable flash
!
! \return     HAL_CAM_Result_en_t
!
! \note       Global:
*/
HAL_CAM_Result_en_t CAM_SetFlashEnable(Boolean enable	/*< (in) True for enable */
    )
{
	HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;

/* -----------Flash Enable/Disable Sequence for Sensor (if needed) */
	if (enable) {
		result |=
		    Cam_SensorIntfSeqSel(sCurrentSensor, SensorFlashEnable);
	} else {
		result |=
		    Cam_SensorIntfSeqSel(sCurrentSensor, SensorFlashDisable);
	}
	pr_debug("%s(): return result=%d \r\n", __FUNCTION__, result);
	return result;
}

/*
! Function Name:  Cam_InitVsyncIrq
!
! Description:  Set up Camera Vsync low-level ISR.
!                 Set up level and trigger edge.
!
! \param      CamIntfConfig_st_t *pSyncIntfConfig
!
! \return     none
!
! \note       Static:
*/
static HAL_CAM_Result_en_t Cam_InitVsyncIrq(CamIntfConfig_st_t *
					    pSyncIntfConfig)
{
	HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
	int rc;

/* Set the Vsync edge to trigger on */
	if ((pSyncIntfConfig->sensor_config_ccir656->vsync_irq_trigger ==
	     SyncRisingEdge)
	    || (pSyncIntfConfig->sensor_config_ccir656->vsync_irq_trigger ==
		SyncActiveHigh)) {
		set_irq_type(IRQ_VSYNC, IRQ_TYPE_EDGE_RISING);
	} else {
		set_irq_type(IRQ_VSYNC, IRQ_TYPE_EDGE_FALLING);
	}
/* Clear any pending interrupts and register vsync interrupt handler. */
	CLEAR_VSYNC_INTR();
	rc = request_irq(IRQ_VSYNC, vsync_isr,
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 19)
			 SA_INTERRUPT,
#else
			 IRQF_DISABLED,
#endif
			 "vsync", NULL);
	if (rc != 0) {
		pr_debug("Camera - Failed to register vsync ISR\n");
		result |= HAL_CAM_ERROR_INTERNAL_ERROR;
	}
	disable_irq(IRQ_VSYNC);

	pr_debug("%s(): return result =%d \r\n", __FUNCTION__, result);
	return result;
}

/*
! Function Name:  Cam_CfgImageCapture
!
! Description:  Set up Camera Controller and DMA for Still Image Capture.
!
! \param      CamImageSize_t resolution
! \param      CamCaptureMode_t mode
! \param      CamSensorSelect_t sensor
!
! \return     Result_t
!
! \note       Static:  No Camera Controller Zoom for Still capture
*/
static HAL_CAM_Result_en_t Cam_CfgImageCapture(CamImageSize_t resolution,
					       CamCaptureMode_t mode,
					       CamSensorSelect_t sensor)
{
	HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
	HAL_CAM_ResolutionSize_st_t sensor_size;
	UInt32 start_line, end_line, start_pixel, end_pixel;
	UInt32 cntl_out_width, cntl_out_height;

/* Set start and end for pixels and lines */
	result = CAMDRV_GetResolution(resolution, mode, sensor, &sensor_size);
	start_pixel = 0;
	end_pixel = sensor_size.resX;
	start_line = 0;
	end_line = sensor_size.resY;

/* Set width and height of output image (No Zoom on Stills Image Capture) */
	cntl_out_width = end_pixel - start_pixel;
	cntl_out_height = end_line - start_line;

	stNewCamWindowConfig.image_size = sensor_size.size;
	stNewCamWindowConfig.res_X = sensor_size.resX;
	stNewCamWindowConfig.res_Y = sensor_size.resY;
	stNewCamWindowConfig.sdfH = CamHorizontalNoSDF;
	stNewCamWindowConfig.sdfV = CamVerticalNoSDF;
	stNewCamWindowConfig.window_X =
	    (cntl_out_width * stNewCamWindowConfig.sdfH) >> 7;
	stNewCamWindowConfig.window_Y =
	    (cntl_out_height * stNewCamWindowConfig.sdfV) >> 7;
	sNewCamWindowStatus = TRUE;

/* Adjust for formats other than YCbCr */
	if (sCaptureFormat == CamDataFmtJPEG) {
		end_pixel = (pSensorIntfConfig->sensor_config_jpeg->jpeg_packet_size_bytes >> 1);	/* Words/line */
		end_line = pSensorIntfConfig->sensor_config_jpeg->jpeg_max_packets;	/* lines */
	}
	if (sCaptureFormat == CamDataFmtYUV) {
		end_pixel = (((end_pixel >> 1) * 3) >> 1);	/* Words/line */
	}
	cntl_out_width = end_pixel - start_pixel;
	cntl_out_height = end_line - start_line;

/* Update Camera Window Configuration Structure */
	stNewCamWindowConfig.win_start_X = start_pixel;
	stNewCamWindowConfig.win_end_X = end_pixel;
	stNewCamWindowConfig.win_start_Y = start_line;
	stNewCamWindowConfig.win_end_Y = end_line;
	stNewCamWindowConfig.pad_Y = 0;
	stNewCamWindowConfig.pad_X = 0;
	stNewCamWindowConfig.dma_X = ((cntl_out_width * stNewCamWindowConfig.sdfH) >> 7) * 2;	/* # of bytes for Dma horizontal */
	stNewCamWindowConfig.dma_Y = (cntl_out_height * stNewCamWindowConfig.sdfV) >> 7;	/* # of lines for Dma vertical */
	sNewCamWindowStatus = TRUE;

/* Set Controller Window, Scaling, and DMA */
	result |= Cam_CfgWindowScale(&stNewCamWindowConfig);

	pr_debug
	    ("%s(): Controller Width=%d Height=%d format=0x%x resolution=0x%x returnval=%d\r\n",
	     __FUNCTION__, stCamWindowConfig.window_X,
	     stCamWindowConfig.window_Y, sCaptureFormat, resolution, result);
	return result;
}

/*
! Function Name:  Cam_CfgWindowScale
!
! Description:  Set up Camera Controller Window and DMA for Post Processor Zooming.
!
! \param      CamWindowConfig_st_t *      pcam_window_config
!
! \return     Result_t
!
! \note       Static:  Camera Controller windowing for PP zooming for Preview, Video, and Still capture
*/
static HAL_CAM_Result_en_t Cam_CfgWindowScale(CamWindowConfig_st_t *
					      pcam_window_config)
{
	HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
	UInt32 start_x, end_x, start_y, end_y, x_size, y_size;

	pr_debug("%s(): \r\n", __FUNCTION__);
/* Set Camera Controller Window and Scale Down according to Zoom Factor */
	start_x = pcam_window_config->win_start_X;
	end_x = pcam_window_config->win_end_X;
	start_y = pcam_window_config->win_start_Y;
	end_y = pcam_window_config->win_end_Y;

/* Get actual width and height of output image */
	if ((sCaptureFormat != CamDataFmtJPEG)
	    && (sCaptureFormat != CamDataFmtYUV)) {
		x_size = pcam_window_config->res_X;
		y_size = pcam_window_config->res_Y;
		pcam_window_config->dma_X = pcam_window_config->window_X * 2;	/* # of bytes for Dma horizontal */
		pcam_window_config->dma_Y = pcam_window_config->window_Y;	/* # of lines for Dma vertical */
		pcam_window_config->pad_Y = 0;
		pcam_window_config->pad_X = 0;

		/* Set Padding value if needed (Requested size > than scaled size) */
		if (pcam_window_config->window_X > x_size) {
			pcam_window_config->pad_X = pcam_window_config->window_X - x_size;	/* # of pixels to pad for output size */
			pcam_window_config->dma_X = x_size * 2;	/* # of bytes for Dma horizontal */
			end_x = end_x - pcam_window_config->pad_X;	/* update end_x (Fix Me!! should use scaled value) */
		}
		if (pcam_window_config->window_Y > y_size) {
			pcam_window_config->pad_Y = pcam_window_config->window_Y - y_size;	/* # of pixels to pad for output size */
			pcam_window_config->dma_Y = y_size;	/* # of bytes for Dma vertical */
			end_y = end_y - pcam_window_config->pad_Y;	/* update end_y (Fix Me!! should use scaled value) */
		}
	}

	if (pcam_window_config->pad_X != 0) {
		pr_debug("%s(): ERROR:  pad_X != 0 \r\n", __FUNCTION__);
		result |= HAL_CAM_ERROR_INTERNAL_ERROR;
	}
/* Set CMI Controller Windowing & Scaling */
	CAM_CNTRL_SetWindow(start_y, end_y, start_x, end_x);
	CAM_CNTRL_SetSDF(pcam_window_config->sdfH, pcam_window_config->sdfV);

	sCamOutputWidth = pcam_window_config->window_X;	/* Image resolution width */
	sCamOutputHeight = pcam_window_config->window_Y;	/* Image resolution height */
	sCamWidth = (CamWindowSize_t) pcam_window_config->dma_X;
	sCamHeight = (CamWindowSize_t) pcam_window_config->dma_Y;
	sCamReturnWidth =
	    pcam_window_config->dma_X + (pcam_window_config->pad_X * 2);
	sCamReturnHeight =
	    pcam_window_config->dma_Y + pcam_window_config->pad_Y;
	sNewCamWindowStatus = FALSE;

/* Update new Settings for CMI Windowing & Scaling in stCamWindowConfig */
	memcpy(&stCamWindowConfig, pcam_window_config,
	       sizeof stCamWindowConfig);
	pr_debug
	    ("%s(): output_res_width=%d  output_res_height=%d  dma_x=%d  dma_y=%d  sensor resolution=0x%x  zoom=%d\r\n",
	     __FUNCTION__, sCamOutputWidth, sCamOutputHeight, sCamWidth,
	     sCamHeight, stCamWindowConfig.image_size, sZoomFactor);
	pr_debug
	    ("%s(): controller window width=%d  controller window height=%d  sdf_X%d  sdf_Y=%d\r\n",
	     __FUNCTION__, (end_x - start_x), (end_y - start_y),
	     stCamWindowConfig.sdfH, stCamWindowConfig.sdfV);
	pr_debug
	    ("%s(): start_x=%d  end_x=%d  pad_x=%d  start_y=%d  end_y=%d  pad_y=%d\r\n",
	     __FUNCTION__, start_x, end_x, stCamWindowConfig.pad_X, start_y,
	     end_y, stCamWindowConfig.pad_Y);
#ifdef USE_DMA_LINK_LIST
	sCamDmaRepeat = TRUE;
#else
	sCamDmaRepeat = FALSE;
#endif

/* Set DMA to match CMI Controller Windowing & Scaling */
	if ((sCaptureMode == CamCaptureStill)
	    || (sCaptureMode == CamCaptureStillnThumb)) {
		sCamDmaMode = DMADRV_STILLS_MODE;
	} else {
		sCamDmaMode = DMADRV_MOVIE_MODE;
	}
	result |= Cam_CfgDma(pcam_window_config, sCamDmaMode, sCamDmaRepeat);
	pr_debug("%s(): return result =%d \r\n", __FUNCTION__, result);
	return result;
}

/*
! Function Name:  Cam_ConfigCamSensorZoom
!
! Description:  Set Camera Sensor Zooming if enabled:
!
! \param      forPreview    TRUE = ViewFinder/Video mode   FALSE = Stills Capture mode
!
! \return     Result_t
!
! \note       Static:  Recommended!!  Use camera sensor zooming if possible.
*/
static HAL_CAM_Result_en_t Cam_ConfigCamSensorZoom(Boolean forPreview,
						   CamSensorSelect_t nSensor)
{
	HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;

/* --------Check for Camera Sensor Zooming Capability */
	if ((pSensorIntfConfig->sensor_config_caps->digital_zoom_st.capable ==
	     TRUE) && (sZoomFactor != CamZoom_PP)) {
		result = CAMDRV_SetZoom(sZoomFactor, nSensor);
		result |= HAL_CAM_SUCCESS;
		if (result) {
			pr_debug("%s(): ERROR:  sZoomFactor=%d\r\n",
				 __FUNCTION__, sZoomFactor);
		} else {
			pr_debug("%s(): sZoomFactor=%d\r\n", __FUNCTION__,
				 sZoomFactor);
		}
	}
	return result;
}

/*
! Function Name:  Cam_CfgDma
!
! Description:  Set up Camera Dma LLI.
!
! \param      CamWindowConfig_st_t *      pcam_window_config
!
! \return     Result_t
!
! \note       Static:  Camera Dma LLI for PP zooming for Preview, Video, and Still capture
*/
static HAL_CAM_Result_en_t Cam_CfgDma(CamWindowConfig_st_t *pcam_window_config,
				      DMADRV_LLITABLE_MODE_t mode,
				      Boolean repeat)
{
	HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
	int buffer, buffer_cnt;

	buffer_cnt = (mode == DMADRV_STILLS_MODE) ? 1 : DMA_LLI_BUFFERS;

/* allocate DMA buffers */
	if (mode == DMADRV_STILLS_MODE) {
		gDmaBuffer[0].sizeInBytes =
		    pcam_window_config->dma_X * pcam_window_config->dma_Y;
		gDmaBuffer[0].physPtr = gCamBootBuffer2.physPtr;
		gDmaBuffer[0].virtPtr = gCamBootBuffer2.virtPtr;

		pr_debug("\n gDmaBuffer: offset: %d virtPtr = 0x%x\n",
			 buffer, gDmaBuffer[0].virtPtr);
		pr_debug("\n gDmaBuffer: offset: %d physPtr = 0x%x\n",
			 buffer, gDmaBuffer[0].physPtr);
		pr_debug("\n Allocating 0x%x bytes from offset:0x%x\n",
			 gDmaBuffer[0].sizeInBytes, cam_bootmem_offset);
		cam_bootmem_offset += gDmaBuffer[0].sizeInBytes;

		if (gDmaBuffer[0].physPtr == (dma_addr_t) NULL) {
			pr_debug
			    ("Camera - Failed to allocate memory ( 0x%08x ) for DMA buffer %d\n",
			     gDmaBuffer[0].sizeInBytes, buffer);
			result |= HAL_CAM_ERROR_INTERNAL_ERROR;
		}
	} else {
		for (buffer = 0; buffer < DMA_LLI_BUFFERS; buffer++) {
			gDmaBuffer[buffer].sizeInBytes =
			    pcam_window_config->dma_X *
			    pcam_window_config->dma_Y;
			gDmaBuffer[buffer].physPtr =
			    gCamBootBuffer.physPtr + cam_bootmem_offset;

			pr_debug("\n gDmaBuffer: offset: %d virtPtr = 0x%x\n",
				 buffer, gDmaBuffer[buffer].virtPtr);
			pr_debug("\n gDmaBuffer: offset: %d physPtr = 0x%x\n",
				 buffer, gDmaBuffer[buffer].physPtr);
			pr_debug("\n Allocating 0x%x bytes from offset:0x%x\n",
				 gDmaBuffer[buffer].sizeInBytes,
				 cam_bootmem_offset);
			cam_bootmem_offset += gDmaBuffer[buffer].sizeInBytes;

			if (gDmaBuffer[buffer].physPtr == (dma_addr_t) NULL) {
				pr_debug
				    ("Camera - Failed to allocate memory ( 0x%08x ) for DMA buffer %d\n",
				     gDmaBuffer[buffer].sizeInBytes, buffer);
				result |= HAL_CAM_ERROR_INTERNAL_ERROR;
			}
		}
	}
	if (result != HAL_CAM_SUCCESS) {
		pr_debug("Camera - Failed to allocate DMA buffer\n");
	}
/* set up the DMA linked list */
	result |=
	    camDmaCreateList(pcam_window_config->dma_X,
			     pcam_window_config->dma_Y, mode);
	if (result != HAL_CAM_SUCCESS) {
		pr_debug("Camera - Failed to create DMA linked list\n");
	}
/* request DMA channel and DMA irq */
	gCamConfig.dmaChannel = DESIRED_DMA_CHANNEL;
	if (dma_request_chan(gCamConfig.dmaChannel, "camera") != 0) {
		/* couldn't get desired DMA channel, take whatever is available */
		pr_debug("Camera - Failed to get desired DMA channel\n");
		if (dma_request_avail_chan(&gCamConfig.dmaChannel, "camera") !=
		    0) {
			pr_debug("Camera - Failed to get DMA channel\n");
			result |= HAL_CAM_ERROR_INTERNAL_ERROR;
		}
	}

	if (dma_request_irq(gCamConfig.dmaChannel, dma_isr, 0) != 0) {
		pr_debug("Camera - Failed to get dma irq\n");
		result |= HAL_CAM_ERROR_INTERNAL_ERROR;
	} else {
		dma_disable_irq(gCamConfig.dmaChannel);
	}
	pr_debug
	    ("%s(): channel=%d  mode=%d  size=0x%x dma_X=%d  dma_Y=%d  ptr0=0x%x ptr1=0x%x\r\n",
	     __FUNCTION__, gCamConfig.dmaChannel, mode,
	     gDmaBuffer[0].sizeInBytes, pcam_window_config->dma_X,
	     pcam_window_config->dma_Y, gDmaBuffer[0].physPtr,
	     gDmaBuffer[1].physPtr);
	return result;
}

#if defined(CONFIG_BCM_CAM_STV0986)
/*
! Function Name:  Cam_MarkerSearch
!
! Description:  Search JPEG markers for still image capture
!
! \param      void *  framebuffer
!
! \return     int     0: success
!
! \note       Static:
*/
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
/*
! Function Name:  CAM_WriteI2c
!
! Description:  I2C write to Camera device
!
! \param      camRegID  I2C device Sub-Addr
! \param      DataCnt   I2C data write count
! \param      Data      I2C data pointer
!
! \return     Result_t
!
! \note       Semaphore protected write, waits for callback to release semaphore
!               before returning with status
*/
HAL_CAM_Result_en_t CAM_WriteI2c(UInt16 camRegID, UInt16 DataCnt, UInt8 *Data)
{
	HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
	struct i2c_adapter *adap;

	if (!DataCnt || (DataCnt > MAX_I2C_DATA_COUNT)) {
		result = HAL_CAM_ERROR_INTERNAL_ERROR;
		pr_debug("%s() - DataCnt = %d out of range\n", __FUNCTION__,
			 DataCnt);
		goto done;
	}

	if ((cam_i2c_datap) && (adap = cam_i2c_datap->client->adapter)) {
		int ret;
		unsigned char msgbuf[sizeof(camRegID) + MAX_I2C_DATA_COUNT];
		struct i2c_msg msg = {
		     cam_i2c_datap->client->addr, cam_i2c_datap->client->flags,
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

/*
! Function Name:  CAM_ReadI2c
!
! Description:  I2C write to Camera device
!
! \param      camRegID    I2C device Sub-Addr
! \param      DataCnt     I2C data read count
! \param      Data        I2C data pointer
!
! \return     Result_t
!
! \note       Semaphore protected read, waits for callback to release semaphore
!               before returning with status
*/
HAL_CAM_Result_en_t CAM_ReadI2c(UInt16 camRegID, UInt16 DataCnt, UInt8 *Data)
{
	HAL_CAM_Result_en_t result = HAL_CAM_SUCCESS;
	struct i2c_adapter *adap;

	if (!DataCnt) {
		result = HAL_CAM_ERROR_INTERNAL_ERROR;
		pr_debug("%s() - DataCnt = %d out of range\n", __FUNCTION__,
			 DataCnt);
		goto done;
	}

	if ((cam_i2c_datap) && (adap = cam_i2c_datap->client->adapter)) {
		int ret;
		unsigned char msgbuf0[sizeof(camRegID)];
		struct i2c_msg msg[2] = {
			{cam_i2c_datap->client->addr,
			 cam_i2c_datap->client->flags, sizeof(camRegID),
			 msgbuf0}
			,
			{cam_i2c_datap->client->addr,
			 cam_i2c_datap->client->flags | I2C_M_RD, DataCnt, Data}
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

/****************************************************************************/

module_init(cam_init);
module_exit(cam_exit);

MODULE_AUTHOR("Broadcom");
MODULE_LICENSE("Proprietary");
MODULE_DESCRIPTION("Camera Driver");
