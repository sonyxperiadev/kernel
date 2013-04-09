/* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*******************************************************************************/
//
// File: csl_cam.c
//
//       HERA, RHEA CAM CSL implementation.
//

//#define ENABLE_DEBUG
//#define ENABLE_DEBUG_REGISTER_DISPLAY

#include <linux/string.h>
#include <linux/workqueue.h>
#include <linux/interrupt.h>
#include <mach/irqs.h>
#include <mach/memory.h>
#include <linux/kernel.h>
#include <mach/io_map.h>
#include <mach/rdb/brcm_rdb_sysmap.h>

#include <linux/broadcom/mobcom_types.h>
#include <linux/broadcom/msconsts.h>
#include <plat/osabstract/ostypes.h>
#include <plat/osabstract/ostask.h>
#include <plat/osabstract/ossemaphore.h>
#include <plat/osabstract/osinterrupt.h>
#include <plat/osdal_os.h>
#include <plat/clock.h>

#include <plat/chal/chal_common.h>
#include <plat/chal/chal_cam.h>
#include <plat/csl/csl_cam.h>

#define IRQ_Clear(irq_id)	do {} while (0)
#define IRQ_Disable(irq_id) 	do {} while (0)	//disable_irq_nosync(irq_id)
#define IRQ_Enable(irq_id)	do {} while (0)	//enable_irq(irq_id)

#undef PMUX_INCLUDED

#ifdef ENABLE_DEBUG
#define DBG_OUT(x) x
#else
#define DBG_OUT(x)
#endif
#define CSLCAM_DBG_ID 10
#ifdef ENABLE_DEBUG
#define CSLCAM_DBG(id, fmt, args...)	do { if (id == CSLCAM_DBG_ID) printk(KERN_ERR fmt, ##args); } while (0)
#else
#define CSLCAM_DBG(id, fmt, args...)
#endif

#ifdef ENABLE_DEBUG_REGISTER_DISPLAY
#define DBG_OUT_REGISTER_DISPLAY(x) x
#else
#define DBG_OUT_REGISTER_DISPLAY(x)
#endif

/******************************************************************************
  Local typedefs 
 *****************************************************************************/
typedef struct {
	Boolean active;
	CSL_CAM_CAPTURE_MODE_T capture_mode;
	CSL_CAM_FRAME_st_t frame_cfg;
	CSL_CAM_PIPELINE_st_t pipeline_cfg;
	CSL_CAM_BUFFER_st_t image_buffer_0;
	CSL_CAM_BUFFER_st_t image_buffer_1;
	CSL_CAM_BUFFER_st_t data_buffer_0;
	CSL_CAM_BUFFER_st_t data_buffer_1;
	CSL_CAM_WINDOW_st_t window_cfg;

// LISR CallBack   
	cslCamCB_t lisr_callback;
	void *lisr_cb_userdata;

// Task Frame End CallBack   
	cslCamCB_t task_callback;
	void *task_cb_userdata;

// Frame Buffers
	UInt32 doubleBufferEn;
	UInt32 bufferIndex;
	pCSL_CAM_BUFFER_st currentBuffer;
	pCSL_CAM_BUFFER_st nextBuffer;
	pCSL_CAM_BUFFER_st currentDataBuffer;

// Frame Status
	UInt32 rx_status;
	UInt32 intr_status;
	UInt32 raw_rx_status;
	UInt32 raw_intr_status;
	UInt32 dropped_frames;

// Frame Time
	UInt32 LC_time;
	UInt32 FE_time;
	UInt32 FS_time;
} CSL_CAM_STATE_st_t;

typedef struct {
	Boolean open;
	CSL_CAM_PORT_CHAN_T port_chan_sel;
	CSL_CAM_STATE_st_t state;
	CSL_CAM_DEVICE_MODE_T dev_mode;
} CAM_HANDLE_T, *CAM_HANDLE;

typedef struct {
	Boolean init;
	UInt32 use_count;
	UInt32 instance;
	UInt32 frame_task_running;
	CSL_CAM_HANDLE currHandle;
	CSL_CAM_INTF_CFG_st_t intf_cfg;
	CSL_CAM_PACKET_st_t packet_cfg;
	Boolean int_registered;
	UInt32 int_id;
	CHAL_HANDLE chalCamH;
	CAM_HANDLE_T camH[CAM_MAX_HANDLES];
} CSL_CAM_DRV_st;

/******************************************************************************
 Static functions
 *****************************************************************************/
static Int32 csl_cam_set_buffer_params(CSL_CAM_HANDLE cslCamH,
				       CSL_CAM_BUFFER_PTR_st_t buffers);
static UInt32 cslCamChalIntrStatus(UInt32 chal_val);
static UInt32 cslCamChalRxStatus(UInt32 chal_val);

/******************************************************************************
 Static data
 *****************************************************************************/

// cam driver
static CSL_CAM_DRV_st cslCamDrv;

// handles
static void *sClkHandle = NULL;	// clock handle

/******************************************************************************
 Local Functions
 *****************************************************************************/
static CHAL_CAM_INTF_t cslCamChalIntf(CSL_CAM_INTF_T csl_val);
static CHAL_CAM_PORT_AFE_t cslCamChalPortAFE(CSL_CAM_PORT_AFE_T csl_val);
static CHAL_CAM_CHAN_t cslCamChalPortChan(CSL_CAM_PORT_CHAN_T csl_val);

static Int32 cslCamClock(UInt32 clk_select, UInt32 freq, Boolean enable);
static void cslCamReset(void);


/******************************************************************************
 Static  Functions
 *****************************************************************************/
//==============================================================================
//! \brief 
//!         CAM module Convert chal status CHAL_CAM_STATUS_CODES to csl status CSL_CAM_RES_T
//! \note 
//!         Parameters:   
//!             status          (in) CHAL_CAM_STATUS_CODES chal_status
//!         Return:
//!             CSL_CAM_RES_T csl_result
//==============================================================================
static CSL_CAM_RES_T cslCamChalResult(CHAL_CAM_STATUS_CODES chal_status)
{
	CSL_CAM_RES_T csl_result = CSL_CAM_OK;

	if (chal_status != CHAL_OP_OK) {
		csl_result |= CSL_CAM_CHAL_ERR;
		if (chal_status & CHAL_OP_INVALID) {
			csl_result |= CSL_CAM_OP_INVALID;
		}
		if (chal_status & CHAL_OP_INVALID_PARAMETER) {
			csl_result |= CSL_CAM_BAD_PARAM;
		}
		if (chal_status & CHAL_OP_DRIVER_NOT_REGISTERED) {
			csl_result |= CSL_CAM_BAD_HANDLE;
		}
		if (chal_status & CHAL_OP_WRONG_ORDER) {
			csl_result |= CSL_CAM_BAD_STATE;
		}
	}
	return csl_result;
}

//==============================================================================
//! \brief 
//!         CAM module Convert csl to chal CHAL_CAM_INTF_t
//! \note 
//!         Parameters:   
//!             handle          (in) CSL_CAM_CHAN_SEL_t chan
//!         Return:
//!             CHAL_CAM_SUB_CHAN_t channel value
//==============================================================================
static CHAL_CAM_INTF_t cslCamChalIntf(CSL_CAM_INTF_T csl_val)
{
	CHAL_CAM_INTF_t chal_val;

	switch (csl_val) {
	default:
	case CSL_CAM_INTF_CCP:
		chal_val = CHAL_CAM_INTF_CCP;
		break;
	case CSL_CAM_INTF_CSI:
		chal_val = CHAL_CAM_INTF_CSI;
		break;
	case CSL_CAM_INTF_CPI:
		chal_val = CHAL_CAM_INTF_CPI;
		break;
	}
	return chal_val;
}

//==============================================================================
//! \brief 
//!         CAM module Convert csl to chal CHAL_CAM_PORT_AFE_t
//! \note 
//!         Parameters:   
//!             handle          (in) CSL_CAM_CHAN_SEL_t chan
//!         Return:
//!             CHAL_CAM_SUB_CHAN_t channel value
//==============================================================================
static CHAL_CAM_PORT_AFE_t cslCamChalPortAFE(CSL_CAM_PORT_AFE_T csl_val)
{
	CHAL_CAM_PORT_AFE_t chal_val;

	switch (csl_val) {
	default:
	case CSL_CAM_PORT_AFE_0:
		chal_val = CHAL_CAM_PORT_AFE_0;
		break;
	case CSL_CAM_PORT_AFE_1:
		chal_val = CHAL_CAM_PORT_AFE_1;
		break;
	}
	return chal_val;
}

//==============================================================================
//! \brief 
//!         CAM module Convert csl to chal lane select
//! \note 
//!         Parameters:   
//!             csl_val          (in) CSL_CAM_PORT_CHAN_T 
//!         Return:
//!             CHAL_CAM_CHAN_t channel value
//==============================================================================
static CHAL_CAM_CHAN_t cslCamChalPortChan(CSL_CAM_PORT_CHAN_T csl_val)
{
	CHAL_CAM_CHAN_t chal_val = (CHAL_CAM_CHAN_t) 0;

	if (csl_val & CSL_CAM_PORT_CHAN_0) {
		chal_val |= CHAL_CAM_CHAN_0;
	}
	if (csl_val & CSL_CAM_PORT_CHAN_1) {
		chal_val |= CHAL_CAM_CHAN_1;
	}
	return chal_val;
}

//==============================================================================
//! \brief 
//!         CAM module Convert chal to csl interrupt status
//! \note 
//!         Parameters:   
//!             chal_val          (in) CHAL_CAM_INTERRUPT_t 
//!         Return:
//!             CSL_CAM_INTERRUPT_t status
//==============================================================================
static UInt32 cslCamChalIntrStatus(UInt32 chal_val)
{
	UInt32 status = 0;

// Check status bits    
	if (chal_val & CHAL_CAM_INT_FRAME_START) {
		status |= CSL_CAM_INT_FRAME_START;
	}
	if (chal_val & CHAL_CAM_INT_FRAME_END) {
		status |= CSL_CAM_INT_FRAME_END;
	}
	if (chal_val & CHAL_CAM_INT_LINE_START) {
		status |= CSL_CAM_INT_LINE_START;
	}
	if (chal_val & CHAL_CAM_INT_LINE_END) {
		status |= CSL_CAM_INT_LINE_END;
	}
	if (chal_val & CHAL_CAM_INT_LINE_COUNT) {
		status |= CSL_CAM_INT_LINE_COUNT;
	}
	if ((chal_val & CHAL_CAM_INT_DATA_END)
	    || (chal_val & CHAL_CAM_INT_DATA_FRAME_END)) {
		status |= CSL_CAM_INT_DATA;
	}
	if (chal_val & CHAL_CAM_INT_PKT) {
		status |= CSL_CAM_INT_PACKET;
	}
	if (chal_val & CHAL_CAM_INT_FRAME_ERROR) {
		status |= CSL_CAM_INT_FRAME_ERROR;
	}
	return status;
}

//==============================================================================
//! \brief 
//!         CAM module Convert chal to csl rx status
//! \note 
//!         Parameters:   
//!             chal_val          (in) CHAL_CAM_RX_t 
//!         Return:
//!             CSL_CAM_RX_STATUS_t status
//==============================================================================
static UInt32 cslCamChalRxStatus(UInt32 chal_val)
{
	UInt32 status = 0;

// Check status bits    
	if (chal_val & CHAL_CAM_RX_INT_CHAN_0) {
		status |= CSL_CAM_RX_INT;
	}
	if (chal_val & CHAL_CAM_RX_INT_PKT_0) {
		status |= CSL_CAM_RX_INT_PKT;
	}
	if (chal_val & CHAL_CAM_RX_INT_PKT_1) {
		status |= CSL_CAM_RX_INT_PKT;
	}
	if (chal_val & CHAL_CAM_RX_PKT_ERROR) {
		status |= (CSL_CAM_RX_PKT_ERROR | CSL_CAM_RX_ERROR);
	}
	if (chal_val & CHAL_CAM_RX_PANIC) {
		status |= (CSL_CAM_RX_PANIC | CSL_CAM_RX_ERROR);
	}
	if (chal_val & CHAL_CAM_RX_BUSY) {
		status |= CSL_CAM_RX_BUSY;
	}
	if (chal_val & CHAL_CAM_RX_CRC_ERROR) {
		status |= CSL_CAM_RX_CRC_ERROR;
//        status |= CSL_CAM_RX_ERROR;
	}
	if (chal_val & CHAL_CAM_RX_FIFO_ERROR) {
		status |= (CSL_CAM_RX_FIFO_ERROR | CSL_CAM_RX_ERROR);
	}
	if (chal_val & CHAL_CAM_RX_PARITY_ERROR) {
		status |= (CSL_CAM_RX_PARITY_ERROR | CSL_CAM_RX_ERROR);
	}
	if (chal_val & CHAL_CAM_RX_BIT_ERROR) {
		status |= (CSL_CAM_RX_BIT_ERROR | CSL_CAM_RX_ERROR);
	}
	if (chal_val & CHAL_CAM_RX_CLK_PRESENT) {
		status |= CSL_CAM_RX_CLK_PRESENT;
	}
	if (chal_val & CHAL_CAM_RX_SYNCD) {
		status |= CSL_CAM_RX_SYNCD;
	}
	if (chal_val & CHAL_CAM_RX_BUF0_RDY) {
		status |= CSL_CAM_RX_BUF0_RDY;
	}
	if (chal_val & CHAL_CAM_RX_BUF0_NO) {
		status |= CSL_CAM_RX_BUF0_NO;
	}
	if (chal_val & CHAL_CAM_RX_BUF1_RDY) {
		status |= CSL_CAM_RX_BUF1_RDY;
	}
	if (chal_val & CHAL_CAM_RX_BUF1_NO) {
		status |= CSL_CAM_RX_BUF1_NO;
	}
	return status;
}

//==============================================================================
//! \brief 
//!         CAM module Convert csl to chal lane select
//! \note 
//!         Parameters:   
//!             csl_val          (in) CSL_CAM_PORT_CHAN_T 
//!         Return:
//!             CHAL_CAM_CHAN_t channel value
//==============================================================================
static CHAL_CAM_LANE_SELECT_t cslCamChalLane(CSL_CAM_SELECT_t csl_val)
{
	CHAL_CAM_LANE_SELECT_t chal_val = (CHAL_CAM_LANE_SELECT_t) 0;

	if (csl_val & CSL_CAM_DATA_LANE_0) {
		chal_val |= CHAL_CAM_DATA_LANE_0;
	}
	if (csl_val & CSL_CAM_DATA_LANE_1) {
		chal_val |= CHAL_CAM_DATA_LANE_1;
	}
	if (csl_val & CSL_CAM_CLOCK) {
		chal_val |= CHAL_CAM_CLK_LANE;
	}
	return chal_val;
}

//==============================================================================
//! \brief 
//!         CAM module Convert csl to chal lane select
//! \note 
//!         Parameters:   
//!             csl_val          (in) CSL_CAM_PORT_CHAN_T 
//!         Return:
//!             CHAL_CAM_CHAN_t channel value
//==============================================================================
static CHAL_CAM_LANE_STATE_t cslCamChalLane_ctrl(CSL_CAM_LANE_CONTROL_t csl_val)
{
	CHAL_CAM_LANE_STATE_t chal_val = CHAL_CAM_LANE_NOCHANGE;

	if (csl_val & CSL_CAM_LANE_EN) {
		chal_val |= CHAL_CAM_LANE_EN;
	}
	if (csl_val & CSL_CAM_LANE_PD) {
		chal_val |= CHAL_CAM_LANE_PD;
	}
	if (csl_val & CSL_CAM_LANE_LPEN) {
		chal_val |= CHAL_CAM_LANE_LPEN;
	}
	if (csl_val & CSL_CAM_LANE_HS) {
		chal_val |= CHAL_CAM_LANE_HS;
	}
	if (csl_val & CSL_CAM_LANE_TERM_EN) {
		chal_val |= CHAL_CAM_LANE_TERM_EN;
	}
	if (csl_val & CSL_CAM_LANE_SYNC_MATCHING) {
		chal_val |= CHAL_CAM_LANE_SYNC_MATCHING;
	}
	if (csl_val & CSL_CAM_LANE_ANALOG_BIAS) {
		chal_val |= CHAL_CAM_LANE_ANALOG_BIAS;
	}
	if (csl_val & CSL_CAM_LANE_HS_RX_TIME) {
		chal_val |= CHAL_CAM_LANE_HS_RX_TIME;
	}
	if (csl_val & CSL_CAM_LANE_HS_SETTLE_TIME) {
		chal_val |= CHAL_CAM_LANE_HS_SETTLE_TIME;
	}
	if (csl_val & CSL_CAM_LANE_HS_TERM_TIME) {
		chal_val |= CHAL_CAM_LANE_HS_TERM_TIME;
	}
	return chal_val;
}

//==============================================================================
//! \brief 
//!         CAM module Convert csl to chal lane status
//! \note 
//!         Parameters:   
//!             csl_val          (in) CSL_CAM_PORT_CHAN_T 
//!         Return:
//!             CSL_CAM_LANE_CONTROL_t 
//==============================================================================
static CSL_CAM_LANE_STATUS_t cslCamChalLaneStatus(CHAL_CAM_LANE_STATE_t
						  chal_val)
{
	CSL_CAM_LANE_STATUS_t csl_val = (CSL_CAM_LANE_STATUS_t)0;

	if (chal_val & CHAL_CAM_LANE_EN) {
		csl_val |= CSL_CAM_LANE_STATUS_EN;
	}
	if (chal_val & CHAL_CAM_LANE_PD) {
		csl_val |= CSL_CAM_LANE_STATUS_PD;
	}
	if (chal_val & CHAL_CAM_LANE_STATE_ERROR) {
		csl_val |= CSL_CAM_LANE_STATUS_ERROR;
	}
	if (chal_val & CHAL_CAM_LANE_ULP) {
		csl_val |= CSL_CAM_LANE_STATUS_ULP;
	}
	if (chal_val & CHAL_CAM_LANE_HS) {
		csl_val |= CSL_CAM_LANE_STATUS_HS;
	}
	if (chal_val & CHAL_CAM_LANE_FF_ERROR) {
		csl_val |= CSL_CAM_LANE_STATUS_FF_ERROR;
	}
	if (chal_val & CHAL_CAM_LANE_SYNC_MATCHING) {
		csl_val |= CSL_CAM_LANE_STATUS_SYNC_ERROR;
	}
	return csl_val;
}

//==============================================================================
//! \brief 
//!         CAM module Convert csl to chal pixel size
//! \note 
//!         Parameters:   
//!             csl_val          (in) CSL_CAM_PIXEL_SIZE_T 
//!         Return:
//!             CHAL_CAM_PIXEL_SIZE_t channel value
//==============================================================================
static CHAL_CAM_PIXEL_SIZE_t cslCamChalPixelSize(CSL_CAM_PIXEL_SIZE_T csl_val)
{
	CHAL_CAM_PIXEL_SIZE_t chal_val;

	switch (csl_val) {
	case CSL_CAM_PIXEL_6BIT:
		chal_val = CHAL_CAM_PIXEL_6BIT;
		break;
	case CSL_CAM_PIXEL_7BIT:
		chal_val = CHAL_CAM_PIXEL_7BIT;
		break;
	case CSL_CAM_PIXEL_8BIT:
		chal_val = CHAL_CAM_PIXEL_8BIT;
		break;
	case CSL_CAM_PIXEL_10BIT:
		chal_val = CHAL_CAM_PIXEL_10BIT;
		break;
	case CSL_CAM_PIXEL_12BIT:
		chal_val = CHAL_CAM_PIXEL_12BIT;
		break;
	case CSL_CAM_PIXEL_14BIT:
		chal_val = CHAL_CAM_PIXEL_14BIT;
		break;
	case CSL_CAM_PIXEL_16BIT:
		chal_val = CHAL_CAM_PIXEL_16BIT;
		break;
	default:
		chal_val = CHAL_CAM_PIXEL_NONE;
		break;
	}
	return chal_val;
}


/*****************************************************************************
 * NAME:    cslCamReset
 *
 * Description: this function Reset Unicam Block
 *
 ******************************************************************************/
static void cslCamReset(void)
{
	struct clk *unicam_clk;

	/* Reset UNICAM interface */
	unicam_clk = clk_get(NULL, "csi0_axi_clk");
	if (!unicam_clk)
		return;
	/* Should clear and set CSI0_SOFT_RSTN_MASK */
	clk_reset(unicam_clk);
}


/*****************************************************************************
 * NAME:    cslCamClock
 *
 * Description: this function will select the Primary or the Secondary Camera
 *
 * Parameters:  clkSel =0(DIG0) =1(DIG1)
 *              enable = 1 or disable = 0 clock
 *
 * Returns: int == 0 is success, all other values are failures
 *
 * Notes: Max clock rate = 26Mhz
 *
 ******************************************************************************/
static Int32 cslCamClock(UInt32 clk_select, UInt32 freq, Boolean enable)
{
	Int32 success = CSL_CAM_OK;	//pass by default
	UInt32 clock_freq = 13000000;

#if defined(PMUX_INCLUDED)
	PinMuxConfig_t pinMuxCfg;
#endif

	switch (freq) {
	case CSL_CAM_CLK_48MHz:
	case CSL_CAM_CLK_24MHz:
	case CSL_CAM_CLK_12MHz:
	default:
		success |= CSL_CAM_BAD_CLK;
		clock_freq = 0;
		break;
	case CSL_CAM_CLK_13MHz:
		clock_freq = 1;
		break;
	case CSL_CAM_CLK_Disabled:
	case CSL_CAM_CLK_26MHz:
		clock_freq = 0;
		break;
	}

	if (chal_cam_clock
	    (cslCamDrv.chalCamH, (cUInt32)clk_select, (cUInt32)clock_freq,
	     (cBool)enable) != CHAL_OP_OK) {
		DBG_OUT(CSLCAM_DBG
			(CSLCAM_DBG_ID,
			 "[cslCamClock][Info] : chal_cam_clock():  ERROR!\n"));
		success |= CSL_CAM_BAD_CLK;
	}
	return success;
}

/******************************************************************************
 Global Functions
 *****************************************************************************/

 /***********************************************************
 * Name: csl_cam_init
 * 
 * Arguments: 
 *       void
 *
 * Description: Routine used to initialise the csi2/ccp2/cpi driver & camera interface pins
 *
 * Returns: int == 0 is success, all other values are failures
 *
 ***********************************************************/
Int32 csl_cam_init(void)
{
	Int32 success = CSL_CAM_OK;	//pass by default
	UInt32 instance;

// Set interface pointer
	if (cslCamDrv.init != 1) {
		printk("csl_cam_init() clear handle\n");
		memset(&cslCamDrv, 0, sizeof(CSL_CAM_DRV_st));
		cslCamDrv.currHandle = &cslCamDrv.camH[0];

		// Assign Port Channels for each handle        
		for (instance = 0; instance < CAM_MAX_HANDLES; instance++) {
			cslCamDrv.camH[instance].port_chan_sel =
			    (CSL_CAM_PORT_CHAN_T) (1 << instance);
		}

		// Init CHAL
		printk("csl_cam_init() chal_cam_init()\n");
		cslCamDrv.chalCamH =
		    (CHAL_HANDLE)
		    chal_cam_init(HW_IO_PHYS_TO_VIRT(MM_CSI0_BASE_ADDR));
		if (cslCamDrv.chalCamH == NULL) {
			DBG_OUT(CSLCAM_DBG
				(CSLCAM_DBG_ID,
				 "[csl_cam_init][Error] : chal_cam_init() FAILED \n"));
			success |= CSL_CAM_BAD_HANDLE;
		} else {
			cslCamDrv.init = 1;
			DBG_OUT(CSLCAM_DBG
				(CSLCAM_DBG_ID, "[csl_cam_init]: Ok \n"));
		}
	}
	return success;
}

 /***********************************************************
 * Name: csl_cam_exit
 * 
 * Arguments: 
 *       void
 *
 * Description: Routine used to exit the driver
 *
 * Returns: int == 0 is success, all other values are failures
 *
 ***********************************************************/
Int32 csl_cam_exit(void)
{
	Int32 success = CSL_CAM_OK;	//pass by default

	if (cslCamDrv.init) {
		chal_cam_deinit(cslCamDrv.chalCamH);
		cslCamDrv.init = 0;
	}
	DBG_OUT(CSLCAM_DBG(CSLCAM_DBG_ID, "[csl_cam_exit][Info] : exit:\n"));
	return success;
}

 /***********************************************************
 * Name: csl_cam_info
 * 
 * Arguments: 
 *       char **driver_name,
         UInt32 *version_major,
         UInt32 *version_minor
         DRIVER_FLAGS_T *flags
 *
 * Description: Routine used to get the driver info
 *
 * Returns: int == 0 is success, all other values are failures
 *
 ***********************************************************/
Int32 csl_cam_info(const char **driver_name,
		   UInt32 *version_major, UInt32 *version_minor)
{
	Int32 success = CSL_CAM_BAD_PARAM;	//fail by default

	//return the driver name   
	if ((NULL != driver_name) && (NULL != version_major)
	    && (NULL != version_minor)) {
		*driver_name = "csl_cam";
		*version_major = 0;
		*version_minor = 1;
		success = CSL_CAM_OK;
	}

	return success;
}

 /***********************************************************
 * Name: csl_cam_open
 * 
 * Arguments: 
 *       const void *params - optional parameters, defined per driver
         DRIVER_HANDLE_T *handle - returned handle to the device
 *
 * Description: Routine used to open a driver and get a handle 
 *
 * Returns: int == 0 is success, all other values are failures
 *
 ***********************************************************/
Int32 csl_cam_open(pCSL_CAM_INTF_CFG_st intfCfg, CSL_CAM_HANDLE *cslCamH)
{

	Int32 success = CSL_CAM_OK;
	UInt32 instance = 0;
	CAM_HANDLE camH = NULL;

	CHAL_CAM_STATUS_CODES chal_status = CHAL_OP_OK;
	CHAL_CAM_CFG_INTF_st_t chal_cam_intf_st;
	CHAL_CAM_CFG_CNTRL_st_t chal_cam_cfg_cntrl_st;
	CHAL_CAM_PARAM_st_t chal_cam_param_st;

	DBG_OUT(CSLCAM_DBG(CSLCAM_DBG_ID, "[csl_cam_open][Info] : Start! \n"));

// need to guard against NULL params being passed
	*cslCamH = NULL;

	if (intfCfg) {
		if (cslCamDrv.init != 1) {
			DBG_OUT(CSLCAM_DBG
				(CSLCAM_DBG_ID,
				 "[csl_cam_open][Error] : Not Initiailized \n"));
			return (CSL_CAM_NOT_INIT);
		}
		// Check for maximum channels already opened
		if (cslCamDrv.use_count == CAM_MAX_HANDLES) {
			DBG_OUT(CSLCAM_DBG
				(CSLCAM_DBG_ID,
				 "[csl_cam_open][Error] : MAX handles allocated! \n"));
			return (CSL_CAM_INST_COUNT);
		}
		// If Channel opened, check Interface & AFE port is same
		if (cslCamDrv.use_count
		    && (cslCamDrv.intf_cfg.intf != intfCfg->intf)
		    && (cslCamDrv.intf_cfg.afe_port != intfCfg->afe_port)) {
			DBG_OUT(CSLCAM_DBG
				(CSLCAM_DBG_ID,
				 "[csl_cam_open][Error] : Channel opened, Interface must be same! \n"));
			return (CSL_CAM_IS_OPEN);
		}
		// Get available channel
		instance = 0;
		while ((instance < CAM_MAX_HANDLES) && (camH == NULL)) {
			if (cslCamDrv.camH[instance].open == 0) {
				camH = &cslCamDrv.camH[instance];
				cslCamDrv.camH[instance].open = 1;
				DBG_OUT(CSLCAM_DBG
					(CSLCAM_DBG_ID,
					 "[csl_cam][Info] : csl_cam_open(): Instance=0x%x \n",
					 instance));
			}
			instance++;
		}
		// Verify channel available
		if (camH == NULL) {
			DBG_OUT(CSLCAM_DBG
				(CSLCAM_DBG_ID,
				 "[csl_cam_open][Error] : No Channels available! \n"));
			return (CSL_CAM_NOT_OPEN);
		}
		// Clear state for channel selected
		memset(&camH->state, 0, sizeof(CSL_CAM_STATE_st_t));

		// If Initial Channel opening on Interface
		cslCamDrv.use_count++;
		if (cslCamDrv.use_count == 1) {
			// Acquire Clock Handle if not Open 

			// Update Interface Info
			memcpy(&cslCamDrv.intf_cfg, intfCfg,
			       sizeof(CSL_CAM_INTF_CFG_st_t));

			DBG_OUT(CSLCAM_DBG
				(CSLCAM_DBG_ID,
				 "[csl_cam][Info] : csl_cam_open(): intf=0x%x afe=0x%x chan=0x%x input=0x%x time_out=0x%x \n",
				 cslCamDrv.intf_cfg.intf,
				 cslCamDrv.intf_cfg.afe_port,
				 camH->port_chan_sel,
				 cslCamDrv.intf_cfg.input_mode,
				 cslCamDrv.intf_cfg.frame_time_out));

			/* Reset Unicam Interface */
			cslCamReset();

			// Set Interface
			chal_cam_intf_st.intf =
			    cslCamChalIntf(cslCamDrv.intf_cfg.intf);
			chal_cam_intf_st.afe_port =
			    cslCamChalPortAFE(cslCamDrv.intf_cfg.afe_port);
			switch (cslCamDrv.intf_cfg.input_mode) {
			case CSL_CAM_INPUT_MODE_DATA_CLOCK:
				chal_cam_intf_st.clk_mode = CHAL_CAM_DATA_CLOCK;
				chal_cam_intf_st.input_mode =
				    (CHAL_CAM_INPUT_MODE_t) 0;
				break;
			case CSL_CAM_INPUT_MODE_DATA_STROBE:
				chal_cam_intf_st.clk_mode =
				    CHAL_CAM_DATA_STROBE;
				chal_cam_intf_st.input_mode =
				    (CHAL_CAM_INPUT_MODE_t) 0;
				break;
			case CSL_CAM_INPUT_SINGLE_LANE:
				chal_cam_intf_st.clk_mode =
				    (CHAL_CAM_CLOCK_MODE_t) 0;
				chal_cam_intf_st.input_mode =
				    CHAL_CAM_INPUT_SINGLE_LANE;
				break;
			case CSL_CAM_INPUT_DUAL_LANE:
				chal_cam_intf_st.clk_mode =
				    (CHAL_CAM_CLOCK_MODE_t) 0;
				chal_cam_intf_st.input_mode =
				    CHAL_CAM_INPUT_DUAL_LANE;
				break;
			default:
				chal_cam_intf_st.clk_mode =
				    (CHAL_CAM_CLOCK_MODE_t) 0;
				chal_cam_intf_st.input_mode =
				    (CHAL_CAM_INPUT_MODE_t) 0;
				break;
			}

			// Set CPI Configuration Pointer
			chal_cam_intf_st.p_cpi_cfg_st =
			    cslCamDrv.intf_cfg.p_cpi_intf_st;
			chal_status |=
			    chal_cam_cfg_intf(cslCamDrv.chalCamH,
					      &chal_cam_intf_st);
			// Power Up Analog
			chal_cam_param_st.intf =
			    cslCamChalIntf(cslCamDrv.intf_cfg.intf);
			chal_cam_param_st.param = TRUE;
			chal_cam_param_st.select = CHAL_CAM_BANDGAP;
			chal_status |=
			    chal_cam_set_bandgap_pwr(cslCamDrv.chalCamH,
						     &chal_cam_param_st);
			chal_cam_param_st.select = CHAL_CAM_ANALOG;
			chal_status |=
			    chal_cam_set_analog_pwr(cslCamDrv.chalCamH,
						    &chal_cam_param_st);
			// Set default Controller settings                        
			chal_cam_cfg_cntrl_st.packet_timeout = 15;
			if (cslCamDrv.intf_cfg.input_mode ==
			    CHAL_CAM_INPUT_DUAL_LANE) {
				chal_cam_cfg_cntrl_st.packet_timeout = 15;
			}
			chal_cam_cfg_cntrl_st.line_start_suppress = FALSE;
			chal_cam_cfg_cntrl_st.burst_length =
			    (CHAL_CAM_BURST_LENGTH_t) 0;
			chal_cam_cfg_cntrl_st.burst_space =
			    (CHAL_CAM_BURST_SPACE_t) 0;

			chal_cam_cfg_cntrl_st.panic_pr = 15;
			chal_cam_cfg_cntrl_st.norm_pr = 0;
			chal_cam_cfg_cntrl_st.panic_thr = 2;
			chal_cam_cfg_cntrl_st.panic_enable = TRUE;
			chal_status |=
			    chal_cam_cfg_cntrl(cslCamDrv.chalCamH,
					       &chal_cam_cfg_cntrl_st);
			/* Register the CSI2/CCP2 interrupt handler */
#if 0
			if (cslCamDrv.int_registered == 0) {
				cslCamDrv.int_id = BCM_INT_ID_CSI;

				// Clear any irq
				IRQ_Disable((InterruptId_t) cslCamDrv.int_id);
				// Register Isr 
				{
					int ret =
					    request_irq(cslCamDrv.int_id,
							cslCamFrameLisr,
							IRQF_DISABLED,
							"rhea csl_cam interrupt",
							(void *)NULL);
					if (ret < 0) {
						printk(KERN_ERR
						       "Unable to register Interrupt for Rhea CSL_CAM interface\n");
						success |= CSL_CAM_ERR;
					}
				}
				IRQ_Clear((InterruptId_t) cslCamDrv.int_id);
				IRQ_Enable((InterruptId_t) cslCamDrv.int_id);
				cslCamDrv.int_registered = TRUE;
			}
#endif
			// Cfg Lane Enable 
			chal_cam_param_st.intf =
			    cslCamChalIntf(cslCamDrv.intf_cfg.intf);
			chal_cam_param_st.chan =
			    cslCamChalPortChan(camH->port_chan_sel);
			chal_cam_param_st.lane =
			    (CHAL_CAM_LANE_SELECT_t) (CHAL_CAM_DATA_LANE_0 |
						      CHAL_CAM_DATA_LANE_1 |
						      CHAL_CAM_CLK_LANE);
			chal_cam_param_st.param =
			    (CHAL_CAM_LANE_SELECT_t) (CHAL_CAM_DATA_LANE_0 |
						      CHAL_CAM_CLK_LANE);
			if (cslCamDrv.intf_cfg.input_mode ==
			    CSL_CAM_INPUT_DUAL_LANE) {
				chal_cam_param_st.param |= CHAL_CAM_DATA_LANE_1;
			}
			chal_status |=
			    chal_cam_lane_enable(cslCamDrv.chalCamH,
						 &chal_cam_param_st);
		}		// if (cslCamDrv.use_count == 1)

		if (chal_status == CHAL_OP_OK) {
			cslCamDrv.currHandle = camH;
			// assign handle
			*cslCamH = (CSL_CAM_HANDLE)camH;
		} else {
			DBG_OUT(CSLCAM_DBG
				(CSLCAM_DBG_ID,
				 "[csl_cam_open][Error] :  chal_cam_xxx: FAILED \n"));
		}
	} else {
		DBG_OUT(CSLCAM_DBG
			(CSLCAM_DBG_ID,
			 "[csl_cam_open][Error] :  params: FAILED \n"));
	}

// Return Clock Handle
	if (sClkHandle != NULL) {
		sClkHandle = NULL;
	}
	if (camH != NULL) {
		DBG_OUT_REGISTER_DISPLAY(csl_cam_register_display
					 ((CSL_CAM_HANDLE)camH));
	}
	success |= cslCamChalResult(chal_status);
	return success;
}

 /***********************************************************
 * Name: csl_cam_close
 * 
 * Arguments: 
 *       const DRIVER_HANDLE_T handle - handle to close
 *
 * Description: Routine used to close a handle to the driver
 *
 * Returns: int == 0 is success, all other values are failures
 *
 ***********************************************************/
Int32 csl_cam_close(CSL_CAM_HANDLE cslCamH)
{
	Int32 success = CSL_CAM_OK;	//pass by default
	CAM_HANDLE camH = (CAM_HANDLE) cslCamH;
	CHAL_CAM_PARAM_st_t chal_cam_param_st;

// Decrement # handles for interface in use
	if (cslCamDrv.use_count != 0) {
		cslCamDrv.use_count--;
	}
// Make sure channel is stopped
	success |= csl_cam_rx_stop(cslCamH);
// If all handles closed on this interface, close everything
	if (cslCamDrv.use_count == 0) {
		if ((success |= csl_cam_reset(cslCamH, CSL_CAM_RESET_SWR))) {
			DBG_OUT(CSLCAM_DBG
				(CSLCAM_DBG_ID,
				 "[csl_cam_close][Error] :  csl_cam_reset(): CSL_CAM_RESET_SWR: FAILED \n"));
		}
		if ((success |= csl_cam_reset(cslCamH, CSL_CAM_RESET_ARST))) {
			DBG_OUT(CSLCAM_DBG
				(CSLCAM_DBG_ID,
				 "[csl_cam_close][Error] :  csl_cam_reset(): CSL_CAM_RESET_ARST: FAILED \n"));
		}
		// Disable all Lanes  
		chal_cam_param_st.intf =
		    cslCamChalIntf(cslCamDrv.intf_cfg.intf);
		chal_cam_param_st.chan =
		    cslCamChalPortChan(camH->port_chan_sel);
		chal_cam_param_st.lane =
		    (CHAL_CAM_LANE_SELECT_t) (CHAL_CAM_DATA_LANE_0 |
					      CHAL_CAM_DATA_LANE_1 |
					      CHAL_CAM_CLK_LANE);
		chal_cam_param_st.param = (CHAL_CAM_LANE_SELECT_t) 0x0;
		if (chal_cam_lane_enable(cslCamDrv.chalCamH, &chal_cam_param_st)
		    != CHAL_OP_OK) {
			DBG_OUT(CSLCAM_DBG
				(CSLCAM_DBG_ID,
				 "[csl_cam_close][Error] : ERROR!\n"));
			success |= CSL_CAM_BAD_PARAM;
		}
		// Power Down Analog
		chal_cam_param_st.intf =
		    cslCamChalIntf(cslCamDrv.intf_cfg.intf);
		chal_cam_param_st.param = FALSE;
		chal_cam_param_st.select = CHAL_CAM_BANDGAP;
		if (chal_cam_set_analog_pwr
		    (cslCamDrv.chalCamH, &chal_cam_param_st) != CHAL_OP_OK) {
			DBG_OUT(CSLCAM_DBG
				(CSLCAM_DBG_ID,
				 "[csl_cam_close][Error] :  chal_cam_set_analog_pwr(): CHAL_CAM_ANALOG: FAILED \n"));
			success |= CSL_CAM_ERR;
		}
		chal_cam_param_st.select = CHAL_CAM_ANALOG;
		if (chal_cam_set_analog_pwr
		    (cslCamDrv.chalCamH, &chal_cam_param_st) != CHAL_OP_OK) {
			DBG_OUT(CSLCAM_DBG
				(CSLCAM_DBG_ID,
				"[csl_cam_close][Error] :  "
				"chal_cam_set_analog_pwr(): "
				"CHAL_CAM_ANALOG: FAILED\n"));
			success |= CSL_CAM_ERR;
		}

		/* Reset Unicam Interface */
		cslCamReset();
	}
// close handle
	camH->open = 0;
	memset(&camH->state, 0, sizeof(CSL_CAM_STATE_st_t));
	return success;
}

 /***********************************************************
 * Name: csl_cam_get_handle
 * 
 * Arguments: 
 *       DRIVER_HANDLE_T *handle - return current handle to the device
 *
 * Description: Routine used to get current handle opened by a driver
 *
 * Returns: int == 0 is success, all other values are failures
 *
 ***********************************************************/
Int32 csl_cam_get_handle(CSL_CAM_HANDLE *cslCamH)
{

	Int32 success = CSL_CAM_OK;	//fail by default

	*cslCamH = NULL;

	if (cslCamDrv.currHandle != NULL) {
		// assign handle
		*cslCamH = cslCamDrv.currHandle;
	} else {
		success |= CSL_CAM_NOT_OPEN;
		DBG_OUT(CSLCAM_DBG
			(CSLCAM_DBG_ID,
			 "[csl_cam_get_handle][Error] :  No Handle available \n"));
	}
	return success;
}

/***********************************************************
 * Name: csl_cam_set_intf_control
 * 
 * Arguments: 
 *       CSL_CAM_HANDLE cslCamH - device handle 
 *       pCSL_CAM_INTF_CNTRL_st intfCtrl - Interface configuration structure
 *
 * Description: Routine used to set Interface Control values
 *
 * Returns: int == 0 is success, all other values are failures
 *
 ***********************************************************/
Int32 csl_cam_set_intf_control(CSL_CAM_HANDLE cslCamH,
			       pCSL_CAM_INTF_CNTRL_st intfCtrl)
{
	Int32 success = CSL_CAM_OK;	//fail by default
//    CAM_HANDLE                  camH = (CAM_HANDLE)cslCamH;
	CHAL_CAM_STATUS_CODES chal_status = CHAL_OP_OK;
	CHAL_CAM_CFG_CNTRL_st_t chal_cam_cfg_cntrl_st;

// Interface
	chal_cam_cfg_cntrl_st.intf = cslCamChalIntf(cslCamDrv.intf_cfg.intf);
// Packet timeout
	chal_cam_cfg_cntrl_st.packet_timeout = (UInt32)intfCtrl->packet_timeout;
// Line Start
	chal_cam_cfg_cntrl_st.line_start_suppress =
	    intfCtrl->line_start_suppress;
// Burst
	chal_cam_cfg_cntrl_st.burst_length =
	    (CHAL_CAM_BURST_LENGTH_t) intfCtrl->burst_length;
	chal_cam_cfg_cntrl_st.burst_space =
	    (CHAL_CAM_BURST_SPACE_t) intfCtrl->burst_space;
// Panic
	chal_cam_cfg_cntrl_st.norm_pr = (UInt32)intfCtrl->norm_pr;
	chal_cam_cfg_cntrl_st.panic_pr = (UInt32)intfCtrl->panic_pr;
	chal_cam_cfg_cntrl_st.panic_thr = (UInt32)intfCtrl->panic_thr;
	chal_cam_cfg_cntrl_st.panic_enable = intfCtrl->panic_enable;
	chal_status |=
	    chal_cam_cfg_cntrl(cslCamDrv.chalCamH, &chal_cam_cfg_cntrl_st);
	if (chal_status != CHAL_OP_OK) {
		DBG_OUT(CSLCAM_DBG
			(CSLCAM_DBG_ID,
			 "[csl_cam_set_intf_control][Info] : ERROR!\n"));
	}
	DBG_OUT_REGISTER_DISPLAY(csl_cam_register_display(cslCamH));
	success |= cslCamChalResult(chal_status);
	return success;
}

/***********************************************************
 * Name: csl_cam_set_input_mode
 * 
 * Arguments: 
 *       CSL_CAM_HANDLE cslCamH - device handle 
 *
 *       pCSL_CAM_INTF_CFG_st intfCfg - intf input cfg
 *
 * Description: Routine used to set Interface Input mode
 *
 * Returns: int == 0 is success, all other values are failures
 *
 ***********************************************************/
Int32 csl_cam_set_input_mode(CSL_CAM_HANDLE cslCamH,
			     pCSL_CAM_INPUT_st inputMode)
{

	Int32 success = CSL_CAM_OK;	//pass by default
	CAM_HANDLE camH = (CAM_HANDLE) cslCamH;
	CHAL_CAM_STATUS_CODES chal_status = CHAL_OP_OK;
	CHAL_CAM_CFG_INTF_st_t chal_cam_intf_st;
	CHAL_CAM_PARAM_st_t chal_cam_param_st;

	DBG_OUT(CSLCAM_DBG
		(CSLCAM_DBG_ID,
		 "[csl_cam_set_input_mode][Info] : mode = 0x%x\n",
		 inputMode->input_mode));

// Update Frame Time-out
	if (inputMode->frame_time_out != 0) {
		cslCamDrv.intf_cfg.frame_time_out = inputMode->frame_time_out;
	}
// Interface
	chal_cam_intf_st.intf = cslCamChalIntf(cslCamDrv.intf_cfg.intf);
	chal_cam_intf_st.afe_port =
	    cslCamChalPortAFE(cslCamDrv.intf_cfg.afe_port);

// Set Interface
	switch (inputMode->input_mode) {
	case CSL_CAM_INPUT_MODE_DATA_CLOCK:
		chal_cam_intf_st.clk_mode = CHAL_CAM_DATA_CLOCK;
		chal_cam_intf_st.input_mode = (CHAL_CAM_INPUT_MODE_t) 0;
		break;
	case CSL_CAM_INPUT_MODE_DATA_STROBE:
		chal_cam_intf_st.clk_mode = CHAL_CAM_DATA_STROBE;
		chal_cam_intf_st.input_mode = (CHAL_CAM_INPUT_MODE_t) 0;
		break;
	case CSL_CAM_INPUT_SINGLE_LANE:
		chal_cam_intf_st.clk_mode = (CHAL_CAM_CLOCK_MODE_t) 0;
		chal_cam_intf_st.input_mode = CHAL_CAM_INPUT_SINGLE_LANE;
		break;
	case CSL_CAM_INPUT_DUAL_LANE:
		chal_cam_intf_st.clk_mode = (CHAL_CAM_CLOCK_MODE_t) 0;
		chal_cam_intf_st.input_mode = CHAL_CAM_INPUT_DUAL_LANE;
		break;
	default:
		chal_cam_intf_st.clk_mode = (CHAL_CAM_CLOCK_MODE_t) 0;
		chal_cam_intf_st.input_mode = (CHAL_CAM_INPUT_MODE_t) 0;
		break;
	}
// Set CPI Configuration Pointer
	chal_cam_intf_st.p_cpi_cfg_st = inputMode->p_cpi_intf_st;
	chal_status |= chal_cam_set_intf(cslCamDrv.chalCamH, &chal_cam_intf_st);

// Cfg Lane Enables 
	chal_cam_param_st.intf = cslCamChalIntf(cslCamDrv.intf_cfg.intf);
	chal_cam_param_st.chan = cslCamChalPortChan(camH->port_chan_sel);
	chal_cam_param_st.lane =
	    (CHAL_CAM_LANE_SELECT_t) (CHAL_CAM_DATA_LANE_0 |
				      CHAL_CAM_DATA_LANE_1 | CHAL_CAM_CLK_LANE);
	chal_cam_param_st.param =
	    (CHAL_CAM_LANE_SELECT_t) (CHAL_CAM_DATA_LANE_0 | CHAL_CAM_CLK_LANE);
	if (inputMode->input_mode == CSL_CAM_INPUT_DUAL_LANE) {
		chal_cam_param_st.param |= CHAL_CAM_DATA_LANE_1;
	}
	chal_status |=
	    chal_cam_lane_enable(cslCamDrv.chalCamH, &chal_cam_param_st);

	if (chal_status != CHAL_OP_OK) {
		DBG_OUT(CSLCAM_DBG
			(CSLCAM_DBG_ID,
			 "[csl_cam_set_input_mode][Info] : ERROR!\n"));
	}
	DBG_OUT_REGISTER_DISPLAY(csl_cam_register_display(cslCamH));
	success |= cslCamChalResult(chal_status);
	return success;
}

/***********************************************************
 * Name: csl_cam_set_input_addr
 * 
 * Arguments: 
 *       CSL_CAM_HANDLE cslCamH - device handle 
 *
 *       pCSL_CAM_BUFFER_st imageAddr_0 - image_0 buffer select
 *       pCSL_CAM_BUFFER_st imageAddr_1 - image_1 buffer select
 *       pCSL_CAM_BUFFER_st dataAddr - data buffer select
 *
 * Description: Routine used to set Interface Addresses Before Rx starts
 *
 * Returns: int == 0 is success, all other values are failures
 *
 ***********************************************************/
Int32 csl_cam_set_input_addr(CSL_CAM_HANDLE cslCamH,
			     pCSL_CAM_BUFFER_st imageAddr_0,
			     pCSL_CAM_BUFFER_st imageAddr_1,
			     pCSL_CAM_BUFFER_st dataAddr)
{
	Int32 success = CSL_CAM_OK;	//pass by default
	CAM_HANDLE camH = (CAM_HANDLE) cslCamH;
	CSL_CAM_BUFFER_PTR_st_t buffers;

	buffers.image0Buff = NULL;
	buffers.image1Buff = NULL;
	buffers.data0Buff = NULL;
	buffers.data1Buff = NULL;
	camH->state.doubleBufferEn = FALSE;

// Set Image buffer 0
	if (imageAddr_0 == NULL) {
		DBG_OUT(CSLCAM_DBG
			(CSLCAM_DBG_ID,
			 "[csl_cam_set_input_addr][Error] : ERROR! imageAddr_0 is NULL. No image buffers \n"));
		success |= CSL_CAM_BAD_PARAM;
	} else if ((imageAddr_1 == NULL) || (camH->state.active == 0)
		   ||
		   (memcmp
		    (&camH->state.image_buffer_0, imageAddr_0,
		     sizeof(CSL_CAM_BUFFER_st_t)) != 0)) {
		memcpy((void *)&camH->state.image_buffer_0,
		       (const void *)imageAddr_0, sizeof(CSL_CAM_BUFFER_st_t));
		buffers.image0Buff = imageAddr_0;
		camH->state.bufferIndex = 0;	// set current buffer index.
		DBG_OUT(CSLCAM_DBG
			(CSLCAM_DBG_ID,
			 "[csl_cam_set_input_addr][Info] : Image0 -> addr=0x%x, size=%d, line_stride=%d, buffer_wrap_en=%d \n",
			 imageAddr_0->start_addr, imageAddr_0->size,
			 imageAddr_0->line_stride,
			 imageAddr_0->buffer_wrap_en));
	}
// Set Image buffer 1
	if (imageAddr_1 == NULL) {
		DBG_OUT(CSLCAM_DBG
			(CSLCAM_DBG_ID,
			 "[csl_cam_set_input_addr][Info] : Image1 -> NULL \n"));
		memset((void *)&camH->state.image_buffer_1, 0,
		       sizeof(CSL_CAM_BUFFER_st_t));
		camH->state.bufferIndex = 0;	// set current buffer index.
	} else if ((camH->state.active == 0)
		   ||
		   (memcmp
		    ((void *)&camH->state.image_buffer_1, (void *)imageAddr_1,
		     sizeof(CSL_CAM_BUFFER_st_t)) != 0)) {
		memcpy((void *)&camH->state.image_buffer_1,
		       (const void *)imageAddr_1, sizeof(CSL_CAM_BUFFER_st_t));
		buffers.image1Buff = imageAddr_1;
		DBG_OUT(CSLCAM_DBG
			(CSLCAM_DBG_ID,
			 "[csl_cam_set_input_addr][Info] : Image1 -> addr=0x%x, size=%d, line_stride=%d, buffer_wrap_en=%d \n",
			 imageAddr_1->start_addr, imageAddr_1->size,
			 imageAddr_1->line_stride,
			 imageAddr_1->buffer_wrap_en));
	}
// Set Data Addr
	if (dataAddr == NULL) {
		memset((void *)&camH->state.data_buffer_0, 0,
		       sizeof(CSL_CAM_BUFFER_st_t));
	} else if ((camH->state.active == 0)
		   ||
		   (memcmp
		    ((void *)&camH->state.data_buffer_0, (void *)dataAddr,
		     sizeof(CSL_CAM_BUFFER_st_t)) != 0)) {
		memcpy((void *)&camH->state.data_buffer_0,
		       (const void *)dataAddr, sizeof(CSL_CAM_BUFFER_st_t));
		buffers.data0Buff = dataAddr;
	}

	DBG_OUT(CSLCAM_DBG
		(CSLCAM_DBG_ID,
		 "[csl_cam_set_input_addr][Info] : Image0 addr=0x%x, Image1 addr=0x%x, dataAddr=0x%x \n",
		 camH->state.image_buffer_0.start_addr,
		 camH->state.image_buffer_1.start_addr,
		 camH->state.data_buffer_0.start_addr));

	success |= csl_cam_set_buffer_params(cslCamH, buffers);
	if (success) {
		DBG_OUT(CSLCAM_DBG
			(CSLCAM_DBG_ID,
			 "[csl_cam_set_input_addr][Info] : ERROR!\n"));
	}
	DBG_OUT_REGISTER_DISPLAY(csl_cam_register_display(cslCamH));
	return success;
}

/***********************************************************
 * Name: csl_cam_set_dbl_buf
 * 
 * Arguments: 
 *       CSL_CAM_HANDLE cslCamH - device handle 
 *
 *       pCSL_CAM_BUFFER_st imageAddr_0 - image_0 buffer select
 *       pCSL_CAM_BUFFER_st imageAddr_1 - image_1 buffer select
 *       pCSL_CAM_BUFFER_st dataAddr_0 - data_0 buffer select
 *       pCSL_CAM_BUFFER_st dataAddr_1 - data_1 buffer select
 *
 * Description: Routine used to set Interface Addresses Before Rx starts
 *
 * Returns: int == 0 is success, all other values are failures
 *
 ***********************************************************/
Int32 csl_cam_set_dbl_buf(CSL_CAM_HANDLE cslCamH,
			  CSL_CAM_BUFFER_PTR_st_t cslBuffers)
{
	Int32 success = CSL_CAM_OK;	//pass by default
	CAM_HANDLE camH = (CAM_HANDLE) cslCamH;

	if ((cslBuffers.image0Buff == NULL) || (cslBuffers.image1Buff == NULL)
	    || (cslBuffers.data0Buff == NULL)
	    || (cslBuffers.data1Buff == NULL)) {
//       CSLCAM_DBG(CSLCAM_DBG_ID,  "[csl_cam_set_dbl_buf][Error] : ERROR! Buffer is NULL \n");    
		success |= CSL_CAM_BAD_PARAM;
		return success;
	}
// Set Image buffer 0
	DBG_OUT(CSLCAM_DBG
		(CSLCAM_DBG_ID,
		 "[csl_cam_set_dbl_buf][Info] : Image0 -> addr=0x%x, size=%d, line_stride=%d, buffer_wrap_en=%d \n",
		 cslBuffers.image0Buff->start_addr, cslBuffers.image0Buff->size,
		 cslBuffers.image0Buff->line_stride,
		 cslBuffers.image0Buff->buffer_wrap_en));
	memcpy(&camH->state.image_buffer_0, cslBuffers.image0Buff,
	       sizeof(CSL_CAM_BUFFER_st_t));
	camH->state.bufferIndex = 0;	// set current buffer index.
// Set Image buffer 1
	memcpy(&camH->state.image_buffer_1, cslBuffers.image1Buff,
	       sizeof(CSL_CAM_BUFFER_st_t));
	DBG_OUT(CSLCAM_DBG
		(CSLCAM_DBG_ID,
		 "[csl_cam_set_dbl_buf][Info] : Image1 -> addr=0x%x, size=%d, line_stride=%d, buffer_wrap_en=%d \n",
		 cslBuffers.image1Buff->start_addr, cslBuffers.image1Buff->size,
		 cslBuffers.image1Buff->line_stride,
		 cslBuffers.image1Buff->buffer_wrap_en));

// Set Data buffer 0
	memcpy(&camH->state.data_buffer_0, cslBuffers.data0Buff,
	       sizeof(CSL_CAM_BUFFER_st_t));
	DBG_OUT(CSLCAM_DBG
		(CSLCAM_DBG_ID,
		 "[csl_cam_set_dbl_buf][Info] : Data0 -> addr=0x%x, size=%d, line_stride=%d, buffer_wrap_en=%d \n",
		 cslBuffers.data0Buff->start_addr, cslBuffers.data0Buff->size,
		 cslBuffers.data0Buff->line_stride,
		 cslBuffers.data0Buff->buffer_wrap_en));
// Set Data buffer 1
	memcpy(&camH->state.data_buffer_1, cslBuffers.data1Buff,
	       sizeof(CSL_CAM_BUFFER_st_t));
	DBG_OUT(CSLCAM_DBG
		(CSLCAM_DBG_ID,
		 "[csl_cam_set_dbl_buf][Info] : Data1 -> addr=0x%x, size=%d, line_stride=%d, buffer_wrap_en=%d \n",
		 cslBuffers.data1Buff->start_addr, cslBuffers.data1Buff->size,
		 cslBuffers.data1Buff->line_stride,
		 cslBuffers.data1Buff->buffer_wrap_en));

	DBG_OUT(CSLCAM_DBG
		(CSLCAM_DBG_ID,
		 "[csl_cam_set_dbl_buf][Info] : Image0=0x%x, Image1=0x%x, data0=0x%x data1=0x%x \n",
		 camH->state.image_buffer_0.start_addr,
		 camH->state.image_buffer_1.start_addr,
		 camH->state.data_buffer_0.start_addr,
		 camH->state.data_buffer_1.start_addr));

	camH->state.doubleBufferEn = TRUE;
	success |= csl_cam_set_buffer_params(cslCamH, cslBuffers);

	if (success) {
//        CSLCAM_DBG(CSLCAM_DBG_ID,  "[csl_cam_set_dbl_buf][Info] : ERROR!\n");
	}
	DBG_OUT_REGISTER_DISPLAY(csl_cam_register_display(cslCamH));
	return success;
}

/***********************************************************
 * Name: csl_cam_set_buffer_params
 * 
 * Arguments: 
 *       CSL_CAM_HANDLE cslCamH - device handle 
 *
 *       CAM_SELECT_t buffer_select - buffer select
 *       pCSL_CAM_BUFFER_st bufParams - buffer configuration structure
 *
 * Description: Routine used to set Buffer Interface values every Frame
 *
 * Returns: int == 0 is success, all other values are failures
 *
 ***********************************************************/
static Int32 csl_cam_set_buffer_params(CSL_CAM_HANDLE cslCamH,
				       CSL_CAM_BUFFER_PTR_st_t cslBuffers)
{
	Int32 success = CSL_CAM_OK;
	CAM_HANDLE camH = (CAM_HANDLE) cslCamH;
	CHAL_CAM_STATUS_CODES chal_status = CHAL_OP_OK;
	CHAL_CAM_BUFFER_CFG_st_t chal_cam_buffer_cfg_st;
	CHAL_CAM_BUFFER_st_t image0, image1, data0, data1;

// Interface
	chal_cam_buffer_cfg_st.intf = cslCamChalIntf(cslCamDrv.intf_cfg.intf);
// Channel
	chal_cam_buffer_cfg_st.chan = cslCamChalPortChan(camH->port_chan_sel);

	chal_cam_buffer_cfg_st.buffers.image0Buff = NULL;
	chal_cam_buffer_cfg_st.buffers.image1Buff = NULL;
	chal_cam_buffer_cfg_st.buffers.data0Buff = NULL;
	chal_cam_buffer_cfg_st.buffers.data1Buff = NULL;

// Set Image buffer
	if (cslBuffers.image0Buff != NULL) {
		image0.start_addr = cslBuffers.image0Buff->start_addr;
		image0.size = cslBuffers.image0Buff->size;
		image0.line_stride = cslBuffers.image0Buff->line_stride;
		image0.buf_wrap_enable = cslBuffers.image0Buff->buffer_wrap_en;
		// ACP Compatible Memory Configuration  
		if (cslBuffers.image0Buff->
		    mem_type & CSL_CAM_MEM_TYPE_ACP_COMPATIBLE) {
			image0.start_addr &= ~(CSL_MEM_ACP_BITS);
			image0.start_addr |= (CSL_CAM_MEM_ACP_TYPE);
		}
		chal_cam_buffer_cfg_st.buffers.image0Buff = &image0;

		if (cslBuffers.image1Buff != NULL) {
			image1.start_addr = cslBuffers.image1Buff->start_addr;
			image1.size = cslBuffers.image1Buff->size;
			image1.line_stride = cslBuffers.image1Buff->line_stride;
			image1.buf_wrap_enable =
			    cslBuffers.image1Buff->buffer_wrap_en;
			// ACP Compatible Memory Configuration  
			if (cslBuffers.image1Buff->
			    mem_type & CSL_CAM_MEM_TYPE_ACP_COMPATIBLE) {
				image1.start_addr &= ~(CSL_MEM_ACP_BITS);
				image1.start_addr |= (CSL_CAM_MEM_ACP_TYPE);
			}
			chal_cam_buffer_cfg_st.buffers.image1Buff = &image1;
		} else {
			chal_cam_buffer_cfg_st.buffers.image1Buff = NULL;
		}
	} else {
//       CSLCAM_DBG(CSLCAM_DBG_ID,  "[csl_cam_set_buffer_params][Error] : ERROR! imageAddr_0 is NULL. No image buffers \n");    
	}

// Set Data Buffer
	if (cslBuffers.data0Buff != NULL) {
		data0.start_addr = cslBuffers.data0Buff->start_addr;
		data0.size = cslBuffers.data0Buff->size;
		data0.line_stride = cslBuffers.data0Buff->line_stride;
		data0.buf_wrap_enable = cslBuffers.data0Buff->buffer_wrap_en;
		// ACP Compatible Memory Configuration  
		if (cslBuffers.data0Buff->
		    mem_type & CSL_CAM_MEM_TYPE_ACP_COMPATIBLE) {
			data0.start_addr &= ~(CSL_MEM_ACP_BITS);
			data0.start_addr |= (CSL_CAM_MEM_ACP_TYPE);
		}
		chal_cam_buffer_cfg_st.buffers.data0Buff = &data0;

		if (cslBuffers.data1Buff != NULL) {
			data1.start_addr = cslBuffers.data1Buff->start_addr;
			data1.size = cslBuffers.data1Buff->size;
			data1.line_stride = cslBuffers.data1Buff->line_stride;
			data1.buf_wrap_enable =
			    cslBuffers.data1Buff->buffer_wrap_en;
			// ACP Compatible Memory Configuration  
			if (cslBuffers.data1Buff->
			    mem_type & CSL_CAM_MEM_TYPE_ACP_COMPATIBLE) {
				data1.start_addr &= ~(CSL_MEM_ACP_BITS);
				data1.start_addr |= (CSL_CAM_MEM_ACP_TYPE);
			}
			chal_cam_buffer_cfg_st.buffers.data1Buff = &data1;
		}
	}
// Update Buffers base on buffering flag    
	if (camH->state.doubleBufferEn) {
		DBG_OUT(CSLCAM_DBG
			(CSLCAM_DBG_ID,
			 "[csl_cam_set_buffer_params][Info] :  Dbl Buffer Enabled \n"));
		chal_status |=
		    chal_cam_cfg_dbl_buffer(cslCamDrv.chalCamH,
					    &chal_cam_buffer_cfg_st);
	} else {
		DBG_OUT(CSLCAM_DBG
			(CSLCAM_DBG_ID,
			 "[csl_cam_set_buffer_params][Info] :  Dbl Buffer Disabled \n"));
		chal_status |=
		    chal_cam_cfg_buffer(cslCamDrv.chalCamH,
					&chal_cam_buffer_cfg_st);
	}

	DBG_OUT_REGISTER_DISPLAY(csl_cam_register_display(cslCamH));
	success |= cslCamChalResult(chal_status);
	return success;
}

/***********************************************************
 * Name: csl_cam_set_frame_control
 * 
 * Arguments: 
 *       CSL_CAM_HANDLE cslCamH - device handle 
 *
 *       pCSL_CAM_FRAME_st frameCtrl - frame interrupt configuration
 *
 * Description: Routine used to set Frame Input Control values
 *
 * Returns: int == 0 is success, all other values are failures
 *
 ***********************************************************/
Int32 csl_cam_set_frame_control(CSL_CAM_HANDLE cslCamH,
				pCSL_CAM_FRAME_st frameCtrl)
{
	Int32 success = CSL_CAM_OK;	//pass by default
	CAM_HANDLE camH = (CAM_HANDLE) cslCamH;
	CHAL_CAM_STATUS_CODES chal_status = CHAL_OP_OK;
	CHAL_CAM_FRAME_CFG_st_t chal_cam_frame_cfg_st;

// Update handle
	memcpy(&camH->state.frame_cfg, frameCtrl, sizeof(CSL_CAM_FRAME_st_t));
// Interface
	chal_cam_frame_cfg_st.intf = cslCamChalIntf(cslCamDrv.intf_cfg.intf);
// Channel
	chal_cam_frame_cfg_st.chan = cslCamChalPortChan(camH->port_chan_sel);
// Interrupts
	chal_cam_frame_cfg_st.interrupts = (CHAL_CAM_INTERRUPT_t) 0;
	if (frameCtrl->int_enable & CSL_CAM_INT_FRAME_START) {
		chal_cam_frame_cfg_st.interrupts |= CHAL_CAM_INT_FRAME_START;
	}
	if (frameCtrl->int_enable & CSL_CAM_INT_FRAME_END) {
		chal_cam_frame_cfg_st.interrupts |= CHAL_CAM_INT_FRAME_END;
	}
	if ((frameCtrl->int_enable & CSL_CAM_INT_LINE_START)
	    || (frameCtrl->int_enable & CSL_CAM_INT_LINE_END)) {
		success |= CSL_CAM_BAD_PARAM;
		DBG_OUT(CSLCAM_DBG
			(CSLCAM_DBG_ID,
			 "[csl_cam_set_frame_control][Error] :  LINE_START/LINE_END not allowed!\n"));
	}

	if (frameCtrl->int_enable & CSL_CAM_INT_LINE_COUNT) {
		chal_cam_frame_cfg_st.interrupts |= CHAL_CAM_INT_LINE_COUNT;
	}
// Line Count
	DBG_OUT(CSLCAM_DBG
		(CSLCAM_DBG_ID,
		 "[csl_cam_set_frame_control][Info] :  intr=0x%x line_count=%d\n",
		 chal_cam_frame_cfg_st.interrupts, frameCtrl->int_line_count));
	chal_cam_frame_cfg_st.line_count = (cUInt32)frameCtrl->int_line_count;
// Capture mode
	camH->state.capture_mode = frameCtrl->capture_mode;
	if (frameCtrl->capture_mode == CSL_CAM_CAPTURE_MODE_TRIGGER) {
		chal_cam_frame_cfg_st.mode = CHAL_CAM_SHOT_MODE_SINGLE;
	} else {
		chal_cam_frame_cfg_st.mode = CHAL_CAM_SHOT_MODE_CONTINOUS;
	}
	chal_status |=
	    chal_cam_cfg_frame(cslCamDrv.chalCamH, &chal_cam_frame_cfg_st);
	if (chal_status != CHAL_OP_OK) {
		DBG_OUT(CSLCAM_DBG
			(CSLCAM_DBG_ID,
			 "[csl_cam_set_frame_control][Info] : ERROR!\n"));
	}
	DBG_OUT_REGISTER_DISPLAY(csl_cam_register_display(cslCamH));
	success |= cslCamChalResult(chal_status);
	return success;
}

/***********************************************************
 * Name: csl_cam_set_image_type_control
 * 
 * Arguments: 
 *       CSL_CAM_HANDLE cslCamH - device handle 
 *
 *       pCSL_CAM_IMAGE_st imageCtrl - image channel id's (virtual channels)
 *
 * Description: Routine used to set Image Capture Control values
 *
 * Returns: int == 0 is success, all other values are failures
 *
 ***********************************************************/
Int32 csl_cam_set_image_type_control(CSL_CAM_HANDLE cslCamH,
				     pCSL_CAM_IMAGE_ID_st imageCtrl)
{
	Int32 success = CSL_CAM_OK;	//pass by default
	CAM_HANDLE camH = (CAM_HANDLE) cslCamH;
	CHAL_CAM_STATUS_CODES chal_status = CHAL_OP_OK;
	CHAL_CAM_IMAGE_ID_st_t chal_cam_image_id_st;

// Interface
	chal_cam_image_id_st.intf = cslCamChalIntf(cslCamDrv.intf_cfg.intf);
// Channel
	chal_cam_image_id_st.chan = cslCamChalPortChan(camH->port_chan_sel);
// Image Channel Id's
	chal_cam_image_id_st.image_data_id_0 =
	    (cUInt8)imageCtrl->image_data_id0;
	chal_cam_image_id_st.image_data_id_1 =
	    (cUInt8)imageCtrl->image_data_id1;
	chal_cam_image_id_st.image_data_id_2 =
	    (cUInt8)imageCtrl->image_data_id2;
	chal_cam_image_id_st.image_data_id_3 =
	    (cUInt8)imageCtrl->image_data_id3;
	chal_cam_image_id_st.image_data_id_4 =
	    (cUInt8)imageCtrl->image_data_id4;
	chal_cam_image_id_st.image_data_id_5 =
	    (cUInt8)imageCtrl->image_data_id5;
	chal_cam_image_id_st.image_data_id_6 =
	    (cUInt8)imageCtrl->image_data_id6;
	chal_cam_image_id_st.image_data_id_7 =
	    (cUInt8)imageCtrl->image_data_id7;
	chal_status |=
	    chal_cam_cfg_image_id(cslCamDrv.chalCamH, &chal_cam_image_id_st);
	if (chal_status != CHAL_OP_OK) {
		DBG_OUT(CSLCAM_DBG
			(CSLCAM_DBG_ID,
			 "[csl_cam_set_image_type_control][Info] : ERROR!\n"));
	}
	DBG_OUT_REGISTER_DISPLAY(csl_cam_register_display(cslCamH));
	success |= cslCamChalResult(chal_status);
	return success;
}

/***********************************************************
 * Name: csl_cam_set_data_type_control
 * 
 * Arguments: 
 *       CSL_CAM_HANDLE cslCamH - device handle 
 *
 *       pCSL_CAM_DATA_st dataCtrl - data channel cfg
 *
 * Description: Routine used to set Data Capture Control values
 *
 * Returns: int == 0 is success, all other values are failures
 *
 ***********************************************************/
Int32 csl_cam_set_data_type_control(CSL_CAM_HANDLE cslCamH,
				    pCSL_CAM_DATA_st dataCtrl)
{
	Int32 success = CSL_CAM_OK;	//pass by default
	CAM_HANDLE camH = (CAM_HANDLE) cslCamH;
	CHAL_CAM_STATUS_CODES chal_status = CHAL_OP_OK;
	CHAL_CAM_DATA_CFG_st_t chal_cam_data_cfg_st;

// Interface
	chal_cam_data_cfg_st.intf = cslCamChalIntf(cslCamDrv.intf_cfg.intf);
// Channel
	chal_cam_data_cfg_st.chan = cslCamChalPortChan(camH->port_chan_sel);
// Line Count
	chal_cam_data_cfg_st.line_count = (cUInt32)dataCtrl->line_count;
// Data Type
	chal_cam_data_cfg_st.data_id = dataCtrl->data_id;
// Interrupt Enable    
	chal_cam_data_cfg_st.intr_enable = (CHAL_CAM_INTERRUPT_t) 0;
	if ((dataCtrl->int_enable & CSL_CAM_INT_DATA)
	    && (dataCtrl->int_enable & CSL_CAM_INT_FRAME_END)) {
		chal_cam_data_cfg_st.intr_enable |= CHAL_CAM_INT_DATA_FRAME_END;
	} else if (dataCtrl->int_enable & CSL_CAM_INT_DATA) {
		chal_cam_data_cfg_st.intr_enable |= CHAL_CAM_INT_DATA_END;
	}
// FSP Decoding
	chal_cam_data_cfg_st.fsp_decode_enable = dataCtrl->fsp_decode_enable;
	chal_status |=
	    chal_cam_cfg_data(cslCamDrv.chalCamH, &chal_cam_data_cfg_st);
	if (chal_status != CHAL_OP_OK) {
		DBG_OUT(CSLCAM_DBG
			(CSLCAM_DBG_ID,
			 "[csl_cam_set_data_type_control][Info] : ERROR!\n"));
	}
	DBG_OUT_REGISTER_DISPLAY(csl_cam_register_display(cslCamH));
	success |= cslCamChalResult(chal_status);
	return success;
}

/***********************************************************
 * Name: csl_cam_set_packet_control
 * 
 * Arguments: 
 *       CSL_CAM_HANDLE cslCamH - device handle 
 *
 *       pCSL_CAM_PACKET_st packetCtrl - packet input cfg
 *
 * Description: Routine used to set Data Capture Control values
 *
 * Returns: int == 0 is success, all other values are failures
 *
 ***********************************************************/
Int32 csl_cam_set_packet_control(CSL_CAM_HANDLE cslCamH,
				 pCSL_CAM_PACKET_st packetCtrl)
{
	Int32 success = CSL_CAM_OK;	// pass by default
	UInt32 i;
	CAM_HANDLE camH = (CAM_HANDLE) cslCamH;
	CHAL_CAM_STATUS_CODES chal_status = CHAL_OP_OK;
	CHAL_CAM_PKT_CFG_st_t chal_cam_pkt_cfg_st;

// Interface
	chal_cam_pkt_cfg_st.intf = cslCamChalIntf(cslCamDrv.intf_cfg.intf);
// Channel
	chal_cam_pkt_cfg_st.chan = cslCamChalPortChan(camH->port_chan_sel);

// Config packet captures
	for (i = 0; i < MAX_PACKET_CAPTURE; i++) {
		chal_cam_pkt_cfg_st.instance =
		    (CHAL_CAM_PKT_CMP_SELECT_t) (1 << i);
		chal_cam_pkt_cfg_st.cmp_enable =
		    packetCtrl->packet_capture[i].packet_compare_enable;
		chal_cam_pkt_cfg_st.capture_enable =
		    packetCtrl->packet_capture[i].capture_header_enable;
		if (packetCtrl->packet_capture[i].int_enable ==
		    CSL_CAM_INT_PACKET) {
			chal_cam_pkt_cfg_st.intr_enable = TRUE;
		} else {
			chal_cam_pkt_cfg_st.intr_enable = FALSE;
		}
		chal_cam_pkt_cfg_st.data_id =
		    (cUInt8)packetCtrl->packet_capture[i].data_id;
		chal_status |=
		    chal_cam_cfg_short_pkt(cslCamDrv.chalCamH,
					   &chal_cam_pkt_cfg_st);
	}
	if (chal_status != CHAL_OP_OK) {
		DBG_OUT(CSLCAM_DBG
			(CSLCAM_DBG_ID,
			 "[csl_cam_set_packet_control][Info] : ERROR!\n"));
	}
	DBG_OUT_REGISTER_DISPLAY(csl_cam_register_display(cslCamH));
	success |= cslCamChalResult(chal_status);
	return success;
}

/***********************************************************
 * Name: csl_cam_set_lane_control
 * 
 * Arguments: 
 *       CSL_CAM_HANDLE cslCamH - device handle 
 *
 *       pCSL_CAM_LANE_CONTROL_st laneCtrl - lane control input cfg
 *
 * Description: Routine used to set Lane Control values
 *
 * Returns: int == 0 is success, all other values are failures
 *
 ***********************************************************/
Int32 csl_cam_set_lane_control(CSL_CAM_HANDLE cslCamH,
			       pCSL_CAM_LANE_CONTROL_st laneCtrl)
{
	Int32 success = CSL_CAM_OK;	//pass by default
	CAM_HANDLE camH = (CAM_HANDLE) cslCamH;
	CHAL_CAM_STATUS_CODES chal_status = CHAL_OP_OK;
	CHAL_CAM_LANE_CNTRL_st_t chal_cam_lane_cntrl_st;

// Interface
	chal_cam_lane_cntrl_st.intf = cslCamChalIntf(cslCamDrv.intf_cfg.intf);
// Channel
	chal_cam_lane_cntrl_st.chan = cslCamChalPortChan(camH->port_chan_sel);
// Lane
	chal_cam_lane_cntrl_st.lane = cslCamChalLane(laneCtrl->lane_select);
// Lane Control
	chal_cam_lane_cntrl_st.cntrl_state =
	    cslCamChalLane_ctrl(laneCtrl->lane_control);
// Parameter
	chal_cam_lane_cntrl_st.param = (cUInt32)laneCtrl->param;

	chal_status |=
	    chal_cam_set_lane_cntrl(cslCamDrv.chalCamH,
				    &chal_cam_lane_cntrl_st);
	if (chal_status != CHAL_OP_OK) {
		DBG_OUT(CSLCAM_DBG
			(CSLCAM_DBG_ID,
			 "[csl_cam_set_lane_control][Info] : ERROR!\n"));
	}
	DBG_OUT_REGISTER_DISPLAY(csl_cam_register_display(cslCamH));
	success |= cslCamChalResult(chal_status);
	return success;
}

/***********************************************************
 * Name: csl_cam_set_pipeline_control
 * 
 * Arguments: 
 *       CSL_CAM_HANDLE cslCamH - device handle 
 *
 *       pCSL_CAM_PIPELINE_st pipelineCtrl - pipeling control cfg
 *
 * Description: Routine used to set Pipeline Control values
 *
 * Returns: int == 0 is success, all other values are failures
 *
 ***********************************************************/
Int32 csl_cam_set_pipeline_control(CSL_CAM_HANDLE cslCamH,
				   pCSL_CAM_PIPELINE_st pipelineCtrl)
{
	Int32 success = CSL_CAM_OK;	// pass by default
	CAM_HANDLE camH = (CAM_HANDLE) cslCamH;
	CHAL_CAM_STATUS_CODES chal_status = CHAL_OP_OK;
	CHAL_CAM_PIPELINE_CFG_st_t chal_cam_pipeline_st;

// Update handle
	memcpy(&camH->state.pipeline_cfg, pipelineCtrl,
	       sizeof(CSL_CAM_PIPELINE_st_t));
// Interface
	chal_cam_pipeline_st.intf = cslCamChalIntf(cslCamDrv.intf_cfg.intf);
// Channel
	chal_cam_pipeline_st.chan = cslCamChalPortChan(camH->port_chan_sel);

// Encode DPCM
	switch (pipelineCtrl->encode) {
	case CSL_CAM_10_8_DPCM_ENC:
		chal_cam_pipeline_st.enc_proc = CHAL_CAM_10_8_DPCM_ENC;
		break;
	case CSL_CAM_12_8_DPCM_ENC:
		chal_cam_pipeline_st.enc_proc = CHAL_CAM_12_8_DPCM_ENC;
		break;
	case CSL_CAM_14_8_DPCM_ENC:
		chal_cam_pipeline_st.enc_proc = CHAL_CAM_14_8_DPCM_ENC;
		break;
	default:
		chal_cam_pipeline_st.enc_proc = CHAL_CAM_ENC_NONE;
		break;
	}
//  Encode enhanced predictor
	chal_cam_pipeline_st.enc_predictor = FALSE;
	if (pipelineCtrl->enc_adv_predictor == TRUE) {
		chal_cam_pipeline_st.enc_predictor = TRUE;
	}
// Encode Pixel packing
	chal_cam_pipeline_st.enc_pixel_pack =
	    cslCamChalPixelSize(pipelineCtrl->pack);
// Encode Block Size
	chal_cam_pipeline_st.enc_blk_lnth =
	    (CHAL_CAM_ENC_BLK_LTH_t) pipelineCtrl->encode_blk_size;

// Decode DPCM
	switch (pipelineCtrl->decode) {
	case CSL_CAM_6_10_DPCM_DEC:
		chal_cam_pipeline_st.dec_proc = CHAL_CAM_6_10_DPCM_DEC;
		break;
	case CSL_CAM_6_12_DPCM_DEC:
		chal_cam_pipeline_st.dec_proc = CHAL_CAM_6_12_DPCM_DEC;
		break;
	case CSL_CAM_7_10_DPCM_DEC:
		chal_cam_pipeline_st.dec_proc = CHAL_CAM_7_10_DPCM_DEC;
		break;
	case CSL_CAM_7_12_DPCM_DEC:
		chal_cam_pipeline_st.dec_proc = CHAL_CAM_7_12_DPCM_DEC;
		break;
	case CSL_CAM_8_10_DPCM_DEC:
		chal_cam_pipeline_st.dec_proc = CHAL_CAM_8_10_DPCM_DEC;
		break;
	case CSL_CAM_8_12_DPCM_DEC:
		chal_cam_pipeline_st.dec_proc = CHAL_CAM_8_12_DPCM_DEC;
		break;
	case CSL_CAM_8_14_DPCM_DEC:
		chal_cam_pipeline_st.dec_proc = CHAL_CAM_8_14_DPCM_DEC;
		break;
	case CSL_CAM_8_16_DPCM_DEC:
		chal_cam_pipeline_st.dec_proc = CHAL_CAM_8_16_DPCM_DEC;
		break;
	case CSL_CAM_10_14_DPCM_DEC:
		chal_cam_pipeline_st.dec_proc = CHAL_CAM_10_14_DPCM_DEC;
		break;
	case CSL_CAM_10_16_DPCM_DEC:
		chal_cam_pipeline_st.dec_proc = CHAL_CAM_10_16_DPCM_DEC;
		break;
	case CSL_CAM_12_16_DPCM_DEC:
		chal_cam_pipeline_st.dec_proc = CHAL_CAM_12_16_DPCM_DEC;
		break;
	case CSL_CAM_FSP_DEC:
		chal_cam_pipeline_st.dec_proc = CHAL_CAM_FSP_DEC;
		break;
	default:
		chal_cam_pipeline_st.dec_proc = CHAL_CAM_DEC_NONE;
		break;
	}
//  Decode enhanced predictor
	chal_cam_pipeline_st.dec_predictor = FALSE;
	if (pipelineCtrl->dec_adv_predictor == TRUE) {
		chal_cam_pipeline_st.dec_predictor = TRUE;
	}
// Decode Pixel unpacking
	chal_cam_pipeline_st.dec_pixel_unpack =
	    cslCamChalPixelSize(pipelineCtrl->unpack);

	chal_status |=
	    chal_cam_cfg_pipeline(cslCamDrv.chalCamH, &chal_cam_pipeline_st);
	if (chal_status != CHAL_OP_OK) {
		DBG_OUT(CSLCAM_DBG
			(CSLCAM_DBG_ID,
			 "[csl_cam_set_pipeline_control][Info] : ERROR!\n"));
	}
	DBG_OUT_REGISTER_DISPLAY(csl_cam_register_display(cslCamH));
	success |= cslCamChalResult(chal_status);
	return success;
}

/***********************************************************
 * Name: csl_cam_set_pipeline_control
 * 
 * Arguments: 
 *       CSL_CAM_HANDLE cslCamH - device handle 
 *
 *       pCSL_CAM_WINDOW_st imageWindow - window control cfg
 *
 * Description: Routine used to set Window Control values
 *
 * Returns: int == 0 is success, all other values are failures
 *
 ***********************************************************/
Int32 csl_cam_set_image_window(CSL_CAM_HANDLE cslCamH,
			       pCSL_CAM_WINDOW_st imageWindow)
{
	Int32 success = CSL_CAM_OK;	// pass by default
	CAM_HANDLE camH = (CAM_HANDLE) cslCamH;
	CHAL_CAM_STATUS_CODES chal_status = CHAL_OP_OK;
	CHAL_CAM_WINDOW_CFG_st_t chal_cam_window_st;

// Update handle
	memcpy(&camH->state.window_cfg, imageWindow,
	       sizeof(CSL_CAM_WINDOW_st_t));
// Interface
	chal_cam_window_st.intf = cslCamChalIntf(cslCamDrv.intf_cfg.intf);
// Channel
	chal_cam_window_st.chan = cslCamChalPortChan(camH->port_chan_sel);

	chal_cam_window_st.enable = imageWindow->enable;
	chal_cam_window_st.field_gating = FALSE;
	chal_cam_window_st.h_start_sample = imageWindow->horizontal_start_byte;
	chal_cam_window_st.h_end_sample =
	    imageWindow->horizontal_start_byte +
	    imageWindow->horizontal_size_bytes;
	chal_cam_window_st.v_start_sample = imageWindow->vertical_start_line;
	chal_cam_window_st.v_end_sample =
	    imageWindow->vertical_start_line + imageWindow->vertical_size_lines;
	chal_status |=
	    chal_cam_cfg_window(cslCamDrv.chalCamH, &chal_cam_window_st);
	if (chal_status != CHAL_OP_OK) {
		DBG_OUT(CSLCAM_DBG
			(CSLCAM_DBG_ID,
			 "[csl_cam_set_image_window][Info] : ERROR!\n"));
	}
	DBG_OUT_REGISTER_DISPLAY(csl_cam_register_display(cslCamH));
	success |= cslCamChalResult(chal_status);
	return success;
}

/***********************************************************
 * Name: csl_cam_get_buffer_status
 * 
 * Arguments: 
 *       CSL_CAM_HANDLE cslCamH - device handle 
 *
 *       CSL_CAM_SELECT_t status_select - buffer select
 *       pCSL_CAM_BUFFER_STATUS_st bufStatus - buffer status struct
 *
 * Description: Routine used to set Window Control values
 *
 * Returns: int == 0 is success, all other values are failures
 *
 ***********************************************************/
Int32 csl_cam_get_buffer_status(CSL_CAM_HANDLE cslCamH,
				CSL_CAM_SELECT_t status_select,
				pCSL_CAM_BUFFER_STATUS_st bufStatus)
{
	Int32 success = CSL_CAM_OK;	// pass by default
	CAM_HANDLE camH = (CAM_HANDLE) cslCamH;
	CHAL_CAM_STATUS_CODES chal_status = CHAL_OP_OK;
	CHAL_CAM_BUFFER_CFG_st_t chal_cam_buffer_st;
	CHAL_CAM_BUFFER_st_t bufferInfo;

// Interface
	chal_cam_buffer_st.intf = cslCamChalIntf(cslCamDrv.intf_cfg.intf);
// Channel
	chal_cam_buffer_st.chan = cslCamChalPortChan(camH->port_chan_sel);

	chal_cam_buffer_st.buffers.image0Buff = NULL;
	chal_cam_buffer_st.buffers.data0Buff = NULL;
	chal_cam_buffer_st.buffers.image1Buff = NULL;
	chal_cam_buffer_st.buffers.data1Buff = NULL;

	if (status_select == CSL_CAM_DATA)
		chal_cam_buffer_st.buffers.data0Buff = &bufferInfo;
	else
		chal_cam_buffer_st.buffers.image0Buff = &bufferInfo;

	chal_status |=
	    chal_cam_get_buffer_cfg(cslCamDrv.chalCamH, &chal_cam_buffer_st);

	bufStatus->buffer_st.start_addr = bufferInfo.start_addr;
	bufStatus->buffer_st.size = bufferInfo.size;
	bufStatus->buffer_st.line_stride = bufferInfo.line_stride;
	bufStatus->buffer_st.buffer_wrap_en = bufferInfo.buf_wrap_enable;
	bufStatus->write_ptr = chal_cam_buffer_st.write_ptr;
	bufStatus->bytes_per_line = chal_cam_buffer_st.bytes_per_line;
	bufStatus->lines_per_frame = chal_cam_buffer_st.lines_per_frame;

	if (chal_status != CHAL_OP_OK) {
		DBG_OUT(CSLCAM_DBG
			(CSLCAM_DBG_ID,
			 "[csl_cam_get_buffer_status][Info] : ERROR!\n"));
	}
	success |= cslCamChalResult(chal_status);
	return success;
}

/***********************************************************
 * Name: csl_cam_get_intr_status
 * 
 * Arguments: 
 *       CSL_CAM_HANDLE cslCamH - device handle 
 *
 *       CSL_CAM_INTERRUPT_t *intStatus - OR'd interrupt status
 *
 * Description: Routine used to get Channel Interrupt Status
 *
 * Returns: int == 0 is success, all other values are failures
 *
 ***********************************************************/
UInt32 csl_cam_get_intr_status(CSL_CAM_HANDLE cslCamH,
			       CSL_CAM_INTERRUPT_t *intStatus)
{
	CAM_HANDLE camH = (CAM_HANDLE) cslCamH;
	CHAL_CAM_PARAM_st_t chal_cam_param_st;
	UInt32 raw_status = 0;

// Interface
	chal_cam_param_st.intf = cslCamChalIntf(cslCamDrv.intf_cfg.intf);
// Channel
	chal_cam_param_st.chan = cslCamChalPortChan(camH->port_chan_sel);
// Channel
	chal_cam_param_st.param = 0;
// Channel Intr Status    
	raw_status =
	    chal_cam_get_chan_status(cslCamDrv.chalCamH, &chal_cam_param_st);

// Check status bits    
	*intStatus =
	    (CSL_CAM_INTERRUPT_t)(cslCamChalIntrStatus
				  (chal_cam_param_st.param));
	return raw_status;
}

/***********************************************************
 * Name: csl_cam_get_rx_status
 * 
 * Arguments: 
 *       CSL_CAM_HANDLE cslCamH - device handle 
 *
 *       CSL_CAM_RX_STATUS_t *rxStatus - receiver status
 *
 * Description: Routine used to get Receiver status
 *
 * Returns: cHAL raw status
 *
 ***********************************************************/
UInt32 csl_cam_get_rx_status(CSL_CAM_HANDLE cslCamH,
			     CSL_CAM_RX_STATUS_t *rxStatus)
{
	CAM_HANDLE camH = (CAM_HANDLE) cslCamH;
	CHAL_CAM_PARAM_st_t chal_cam_param_st;
	UInt32 raw_status = 0;

// Interface
	chal_cam_param_st.intf = cslCamChalIntf(cslCamDrv.intf_cfg.intf);
// Channel
	chal_cam_param_st.chan = cslCamChalPortChan(camH->port_chan_sel);
// Receiver Intr Status    
	chal_cam_param_st.param = 0;
	raw_status =
	    chal_cam_get_rx_status(cslCamDrv.chalCamH, &chal_cam_param_st);

// Csl status bits from Chal
	*rxStatus =
	    (CSL_CAM_RX_STATUS_t)cslCamChalRxStatus(chal_cam_param_st.param);
	return raw_status;
}

/***********************************************************
 * Name: csl_cam_get_lane_status
 * 
 * Arguments: 
 *       CSL_CAM_HANDLE cslCamH - device handle 
 *       CSL_CAM_SELECT_t lane - lane select
 *       CSL_CAM_LANE_STATUS_t *laneStatus - lane status
 *
 * Description: Routine used to get Lane status
 *
 * Returns: cHAL raw status
 *
 ***********************************************************/
Int32 csl_cam_get_lane_status(CSL_CAM_HANDLE cslCamH, CSL_CAM_SELECT_t lane,
			      CSL_CAM_LANE_STATUS_t *laneStatus)
{
	Int32 success = CSL_CAM_OK;	// pass by default
	CAM_HANDLE camH = (CAM_HANDLE) cslCamH;
	CHAL_CAM_STATUS_CODES chal_status = CHAL_OP_OK;
	CHAL_CAM_PARAM_st_t chal_cam_param_st;

// Interface
	chal_cam_param_st.intf = cslCamChalIntf(cslCamDrv.intf_cfg.intf);
// Channel
	chal_cam_param_st.chan = cslCamChalPortChan(camH->port_chan_sel);
// Lane
	chal_cam_param_st.lane = cslCamChalLane(lane);
// Lane Status    
	chal_cam_param_st.param = 0;
	chal_status |=
	    chal_cam_get_lane_status(cslCamDrv.chalCamH, &chal_cam_param_st);
	*laneStatus =
	    cslCamChalLaneStatus((CHAL_CAM_LANE_STATE_t) chal_cam_param_st.
				 param);
	if (chal_status != CHAL_OP_OK) {
		DBG_OUT(CSLCAM_DBG
			(CSLCAM_DBG_ID,
			 "[csl_cam_get_lane_status][Info] : ERROR!\n"));
	}
	DBG_OUT_REGISTER_DISPLAY(csl_cam_register_display(cslCamH));
	success |= cslCamChalResult(chal_status);
	return success;
}

/***********************************************************
 * Name: csl_cam_set_clk
 * 
 * Arguments: 
 *       pCSL_CAM_CLOCK_OUT_st clockSet - clock parameter structure 
 *       CSL_CAM_SELECT_t lane - lane select
 *       CSL_CAM_LANE_STATUS_t *laneStatus - lane status
 *
 * Description: Routine used to set clock for camera device
 *
 * Returns: int == 0 is success, all other values are failures
 *
 ***********************************************************/
Int32 csl_cam_set_clk(pCSL_CAM_CLOCK_OUT_st clockSet)
{
	Int32 success = CSL_CAM_OK;	// pass by default

// Clock to Camera Sensor
	success |=
	    cslCamClock(clockSet->clock_select, clockSet->speed,
			clockSet->enable);

	return success;
}

/***********************************************************
 * Name: csl_cam_trigger_capture
 * 
 * Arguments: 
 *       CSL_CAM_HANDLE cslCamH - device handle 
 *
 * Description: Routine used to Trigger capture mode
 *
 * Returns: int == 0 is success, all other values are failures
 *
 ***********************************************************/
Int32 csl_cam_trigger_capture(CSL_CAM_HANDLE cslCamH)
{
	Int32 success = CSL_CAM_OK;	// pass by default
	CAM_HANDLE camH = (CAM_HANDLE) cslCamH;
	CHAL_CAM_STATUS_CODES chal_status = CHAL_OP_OK;
	CHAL_CAM_PARAM_st_t chal_cam_param_st;

//*************************************************
//  TEST CODE ONLY
//*************************************************
	UInt32 frame_time;

	frame_time = 0;

	camH->state.FS_time = frame_time;
	camH->state.FE_time = frame_time;
	camH->state.LC_time = frame_time;
//*************************************************
//*************************************************

// Interface
	chal_cam_param_st.intf = cslCamChalIntf(cslCamDrv.intf_cfg.intf);
// Channel
	chal_cam_param_st.chan = cslCamChalPortChan(camH->port_chan_sel);

	chal_cam_param_st.param = 0x00000000;
	chal_status |=
	    chal_cam_channel_trigger(cslCamDrv.chalCamH, &chal_cam_param_st);
	if (chal_status != CHAL_OP_OK) {
		DBG_OUT(CSLCAM_DBG
			(CSLCAM_DBG_ID,
			 "[csl_cam_trigger_capture][Info] : ERROR!\n"));
	} else {
		// Update Capture mode
		camH->state.capture_mode = CSL_CAM_CAPTURE_MODE_TRIGGER;
	}
	DBG_OUT_REGISTER_DISPLAY(csl_cam_register_display(cslCamH));
	success |= cslCamChalResult(chal_status);
	return success;
}

/***********************************************************
 * Name: csl_cam_rx_start
 * 
 * Arguments: 
 *       CSL_CAM_HANDLE cslCamH - device handle 
 *
 * Description: Routine used to Start Receiver for capture mode
 *
 * Returns: int == 0 is success, all other values are failures
 *
 ***********************************************************/
Int32 csl_cam_rx_start(CSL_CAM_HANDLE cslCamH)
{
	Int32 success = CSL_CAM_OK;	// pass by default
	CAM_HANDLE camH = (CAM_HANDLE) cslCamH;
	CHAL_CAM_STATUS_CODES chal_status = CHAL_OP_OK;
	CHAL_CAM_PARAM_st_t chal_cam_param_st;

// Interface
	chal_cam_param_st.intf = cslCamChalIntf(cslCamDrv.intf_cfg.intf);
// Channel
	chal_cam_param_st.chan = cslCamChalPortChan(camH->port_chan_sel);

// Verify Receiver was opened
	if (camH->open) {
		chal_status |=
		    chal_cam_rx_start(cslCamDrv.chalCamH, &chal_cam_param_st);
		camH->state.active = 1;
	}
	if (chal_status != CHAL_OP_OK) {
		DBG_OUT(CSLCAM_DBG
			(CSLCAM_DBG_ID, "[csl_cam_rx_start][Info] : ERROR!\n"));
	}
	DBG_OUT_REGISTER_DISPLAY(csl_cam_register_display(cslCamH));

	camH->state.FS_time = 0;
	camH->state.FE_time = 0;
	camH->state.LC_time = 0;

	success |= cslCamChalResult(chal_status);
	return success;
}

/***********************************************************
 * Name: csl_cam_rx_stop
 * 
 * Arguments: 
 *       CSL_CAM_HANDLE cslCamH - device handle 
 *
 * Description: Routine used to Stop Receiver, disable capture mode
 *
 * Returns: int == 0 is success, all other values are failures
 *
 ***********************************************************/
Int32 csl_cam_rx_stop(CSL_CAM_HANDLE cslCamH)
{
	Int32 success = CSL_CAM_OK;	// pass by default
	CAM_HANDLE camH = (CAM_HANDLE) cslCamH;
	CHAL_CAM_STATUS_CODES chal_status = CHAL_OP_OK;
	CHAL_CAM_PARAM_st_t chal_cam_param_st;

// Interface
	chal_cam_param_st.intf = cslCamChalIntf(cslCamDrv.intf_cfg.intf);
// Channel
	chal_cam_param_st.chan = cslCamChalPortChan(camH->port_chan_sel);

// Verify Receiver is active
	if (camH->state.active) {
		// Set Receiver to Inactive
		camH->state.active = 0;
		chal_cam_param_st.param = 0x00000000;
		chal_status |=
		    chal_cam_rx_stop(cslCamDrv.chalCamH, &chal_cam_param_st);

		if ((success |= csl_cam_reset(cslCamH, CSL_CAM_RESET_SWR))) {
			DBG_OUT(CSLCAM_DBG
				(CSLCAM_DBG_ID,
				 "[csl_cam_rx_stop][Error] :  csl_cam_reset(): CSL_CAM_RESET_SWR: FAILED \n"));
		}
		if ((success |= csl_cam_reset(cslCamH, CSL_CAM_RESET_ARST))) {
			DBG_OUT(CSLCAM_DBG
				(CSLCAM_DBG_ID,
				 "[csl_cam_rx_stop][Error] :  csl_cam_reset(): CSL_CAM_RESET_ARST: FAILED \n"));
		}
	}
	if (chal_status != CHAL_OP_OK) {
		DBG_OUT(CSLCAM_DBG
			(CSLCAM_DBG_ID, "[csl_cam_rx_stop][Info] : ERROR!\n"));
	}
	DBG_OUT_REGISTER_DISPLAY(csl_cam_register_display(cslCamH));
	success |= cslCamChalResult(chal_status);
	return success;
}

/***********************************************************
 * Name: csl_cam_reset
 * 
 * Arguments: 
 *       CSL_CAM_HANDLE cslCamH - device handle 
 *       CSL_CAM_RESET_t mode - reset mode
 *
 * Description: Routine used to Reset Receiver
 *
 * Returns: int == 0 is success, all other values are failures
 *
 ***********************************************************/
Int32 csl_cam_reset(CSL_CAM_HANDLE cslCamH, CSL_CAM_RESET_t mode)
{
	Int32 success = CSL_CAM_OK;	// pass by default
	CAM_HANDLE camH = (CAM_HANDLE) cslCamH;
	CHAL_CAM_STATUS_CODES chal_status = CHAL_OP_OK;
	CHAL_CAM_PARAM_st_t chal_cam_param_st;

// Interface
	chal_cam_param_st.intf = cslCamChalIntf(cslCamDrv.intf_cfg.intf);
// Channel
	chal_cam_param_st.chan = cslCamChalPortChan(camH->port_chan_sel);
	chal_cam_param_st.param = 0x00000000;
// Set reset requested
	if (mode & CSL_CAM_RESET_SWR) {
		chal_cam_param_st.param |= CHAL_CAM_RESET_RX;
	}
	if (mode & CSL_CAM_RESET_ARST) {
		chal_cam_param_st.param |= CHAL_CAM_RESET_ARST;
	}
// Reset 
	chal_status |= chal_cam_reset(cslCamDrv.chalCamH, &chal_cam_param_st);
	if (chal_status != CHAL_OP_OK) {
		DBG_OUT(CSLCAM_DBG
			(CSLCAM_DBG_ID, "[csl_cam_reset][Info] : ERROR!\n"));
	}
	DBG_OUT_REGISTER_DISPLAY(csl_cam_register_display(cslCamH));

	success |= cslCamChalResult(chal_status);
	return success;
}

/***********************************************************
 * Name: csl_cam_register_callback
 * 
 * Arguments: 
 *      CSL_CAM_HANDLE cslCamH - device handle 
 *      CSL_CAM_CB_EVENT_t event - which callback to register/unregister for
 *      callback - callback function to the applcation, NULL unregisters callback
 * Description: Routine used to Set Callback function
 *
 * Returns: int == 0 is success, all other values are failures
 *
 ***********************************************************/
Int32 csl_cam_register_event_callback(CSL_CAM_HANDLE cslCamH,
				      CSL_CAM_CB_EVENT_t event,
				      cslCamCB_t callback, void *userdata)
{
	Int32 success = CSL_CAM_OK;	// pass by default
	CAM_HANDLE camH = (CAM_HANDLE) cslCamH;

// CallBack Event
	switch (event) {
	case CSL_CAM_LISR:
		camH->state.lisr_callback = callback;
		camH->state.lisr_cb_userdata = userdata;
		break;
	case CSL_CAM_TASK:
		camH->state.task_callback = callback;
		camH->state.task_cb_userdata = userdata;
		break;
	default:
		success |= CSL_CAM_BAD_PARAM;
		DBG_OUT(CSLCAM_DBG
			(CSLCAM_DBG_ID,
			 "[csl_cam_register_event_callback][Info] : ERROR! No Selection\n"));
		break;
	}
	return success;
}

/***********************************************************
 * Name: csl_cam_register_display
 * 
 * Arguments: 
 *      CSL_CAM_HANDLE cslCamH - device handle 
 * Description: Debug Routine used to display camera interface registers
 *
 * Returns: int == 0 is success, all other values are failures
 *
 ***********************************************************/
Int32 csl_cam_register_display(CSL_CAM_HANDLE cslCamH)
{
	Int32 success = CSL_CAM_OK;	// pass by default
	CAM_HANDLE camH = (CAM_HANDLE) cslCamH;
	CHAL_CAM_STATUS_CODES chal_status = CHAL_OP_OK;
	CHAL_CAM_PARAM_st_t chal_cam_param_st;

// Interface
	chal_cam_param_st.intf = cslCamChalIntf(cslCamDrv.intf_cfg.intf);
// Channel
	chal_cam_param_st.chan = cslCamChalPortChan(camH->port_chan_sel);
	chal_cam_param_st.param = 0x00000000;
// Display Registers for Interface/Channel chosen
	chal_status |=
	    chal_cam_register_display(cslCamDrv.chalCamH, &chal_cam_param_st);

	success |= cslCamChalResult(chal_status);
	return success;
}

/********************************** End of file ******************************************/
