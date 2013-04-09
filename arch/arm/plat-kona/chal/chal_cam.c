/***************************************************************************
*
* Copyright 2004 - 2009 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*
****************************************************************************/
//============================================================================
//!
//! \file   chal_cam.c
//! \brief  CAM cHAL layer
//! \note  
//!
//============================================================================
#include <plat/chal/chal_types.h>
#include <plat/chal/chal_common.h>
#include <plat/chal/chal_cam.h>
#include <mach/memory.h>
#include <mach/rdb/brcm_rdb_cam.h>
#include <mach/rdb/brcm_rdb_mm_cfg.h>
#include <mach/rdb/brcm_rdb_padctrlreg.h>
#include <mach/rdb/brcm_rdb_sysmap.h>
#include <mach/rdb/brcm_rdb_util.h>

#include <linux/err.h>

//===========================================================================
// local macro declarations
//
//#define ENABLE_DEBUG

#ifdef ENABLE_DEBUG
    #define DBG_OUT(x) x
#else
   #define DBG_OUT(x) 
#endif

//#define  DISPLAY_CHAL_CAM_STATUS    

#define MIN_FRAME_RATE  100                // Minimum Camera Video Frame Rate (10Fps)
#define CAM_BUFFER_PAD  0x00000080

typedef struct chal_cam_s {
    cBool   init;
    cUInt32  baseAddr;                       // base address of registers
    CHAL_CAM_CFG_CNTRL_st_t cfg_cntrl_st;
} chal_cam_t, *p_chal_cam_t;

//*****************************************************************************
// Local Variables
//*****************************************************************************
static chal_cam_t cam_dev[1] = 
{ 
    {0}
};

//*****************************************************************************
// Local Functions
//*****************************************************************************
//SETs REGISTER BIT FIELD; VALUE IS 0 BASED 
#define CAM_REG_FIELD_SET(r,f,d)        \
    ( ((BRCM_REGTYPE(r))(d) << BRCM_FIELDSHIFT(r,f) ) & BRCM_FIELDMASK(r,f) )
//GETs REGISTER BIT FIELD; VALUE IS 0 BASED 
#define CAM_REG_FIELD_GET(r,f,d)        \
    ( ((BRCM_REGTYPE(r))(d) & BRCM_FIELDMASK(r,f) ) >> BRCM_FIELDSHIFT(r,f) )

//GETs REGISTER BITs Defined WITH MASK
#define CAM_REG_WRITE_MASKED(b,r,m,d)   \
    ( BRCM_WRITE_REG(b,r,(BRCM_READ_REG(b,r) & (~m)) | d) )

//SETs REGISTER BITs Defined WITH MASK
#define CAM_REG_WRITE_MASKED(b,r,m,d)   \
    ( BRCM_WRITE_REG(b,r,(BRCM_READ_REG(b,r) & (~m)) | d) )

// ---- Private Function Prototypes -----------------------------------------
static void chalCamCopy( cUInt8 *wr_addr, cUInt8 *rd_addr, cUInt32 size );
// static CHAL_CAM_STATUS_CODES chal_cam_wait_value(CHAL_HANDLE handle, CHAL_CAM_INTF_t intf, cUInt32 timeout, cUInt32 *reg_addr, cUInt32 reg_mask, cUInt32 value);

// ---- Functions -----------------------------------------------------------


CHAL_CAM_STATUS_CODES chal_cam_register_display(CHAL_HANDLE handle, CHAL_CAM_PARAM_st_t* param)
{
    CHAL_CAM_STATUS_CODES chal_status = CHAL_OP_OK;
    chal_cam_t *pCamDevice = (chal_cam_t *)handle;
    cUInt32 ctl, sta, ana, pri, clk, clt;
    cUInt32 dat0, dat1, dlt;
    cUInt32 cmp0, cmp1, cap0, cap1;
    cUInt32 dbg0,dbg1,dbg2,dbg3;

    cUInt32 ictl, ista, ipipe;
    cUInt32 idi, ibsa, ibea;
    cUInt32 ibls, ibwp, ihwin, ihsta, ivwin, ivsta;
    cUInt32 dcs, dbsa, dbea, dbwp;

    cUInt32 idi1, ibsa1, ibea1; 
    cUInt32 icc, ics, idc, idpo, idca, idcd, ids;
    cUInt32 dbctl, dbsa1, dbea1;
    cUInt32 misc;

	if (pCamDevice == NULL) {
		printk(KERN_ERR "%s: pCamDevice = NULL. Serious error.\n",
						__func__);
		return CHAL_OP_FAILED;
	}

	if (pCamDevice->baseAddr == NULL) {
		printk(KERN_ERR "%s: pCamDevice->baseAddr = NULL. Serious error.\n",
						__func__);
		return CHAL_OP_FAILED;
	}

// Register Output
    // control
        ctl = BRCM_READ_REG ( pCamDevice->baseAddr, CAM_CTL );
    // status
        sta = BRCM_READ_REG ( pCamDevice->baseAddr, CAM_STA );
        ana = BRCM_READ_REG ( pCamDevice->baseAddr, CAM_ANA );
        pri = BRCM_READ_REG ( pCamDevice->baseAddr, CAM_PRI );
        clk = BRCM_READ_REG ( pCamDevice->baseAddr, CAM_CLK );
        clt = BRCM_READ_REG ( pCamDevice->baseAddr, CAM_CLT );
    // data status
        dat0 = BRCM_READ_REG ( pCamDevice->baseAddr, CAM_DAT0 );
        dat1 = BRCM_READ_REG ( pCamDevice->baseAddr, CAM_DAT1 );
        dlt = BRCM_READ_REG ( pCamDevice->baseAddr, CAM_DLT);
        cmp0 = BRCM_READ_REG ( pCamDevice->baseAddr, CAM_CMP0 );
        cmp1 = BRCM_READ_REG ( pCamDevice->baseAddr, CAM_CMP1 );
        cap0 = BRCM_READ_REG ( pCamDevice->baseAddr, CAM_CAP0 );
        cap1 = BRCM_READ_REG ( pCamDevice->baseAddr, CAM_CAP1 );
    // debug
        dbg0 = BRCM_READ_REG ( pCamDevice->baseAddr, CAM_DBG0 );
        dbg1 = BRCM_READ_REG ( pCamDevice->baseAddr, CAM_DBG1 );
        dbg2 = BRCM_READ_REG ( pCamDevice->baseAddr, CAM_DBG2 );
        dbg3 = BRCM_READ_REG ( pCamDevice->baseAddr, CAM_DBG3 );
    // image
        ictl = BRCM_READ_REG ( pCamDevice->baseAddr, CAM_ICTL );
        ista = BRCM_READ_REG ( pCamDevice->baseAddr, CAM_ISTA );
        idi = BRCM_READ_REG ( pCamDevice->baseAddr, CAM_IDI );
        ipipe = BRCM_READ_REG ( pCamDevice->baseAddr, CAM_IPIPE );
        ibsa = BRCM_READ_REG ( pCamDevice->baseAddr, CAM_IBSA );
        ibea = BRCM_READ_REG ( pCamDevice->baseAddr, CAM_IBEA );
        ibls = BRCM_READ_REG ( pCamDevice->baseAddr, CAM_IBLS );
    // window
        ibwp = BRCM_READ_REG ( pCamDevice->baseAddr, CAM_IBWP );
        ihwin = BRCM_READ_REG ( pCamDevice->baseAddr, CAM_IHWIN );
        ihsta = BRCM_READ_REG ( pCamDevice->baseAddr, CAM_IHSTA );
        ivwin = BRCM_READ_REG ( pCamDevice->baseAddr, CAM_IVWIN );
        ivsta = BRCM_READ_REG ( pCamDevice->baseAddr, CAM_IVSTA );

        dcs = BRCM_READ_REG ( pCamDevice->baseAddr, CAM_DCS );
        dbsa = BRCM_READ_REG ( pCamDevice->baseAddr, CAM_DBSA );
        dbea = BRCM_READ_REG ( pCamDevice->baseAddr, CAM_DBEA );
        dbwp = BRCM_READ_REG ( pCamDevice->baseAddr, CAM_DBWP );

    // image
        idi1 = BRCM_READ_REG ( pCamDevice->baseAddr, CAM_IDI1 );
        ibsa1 = BRCM_READ_REG ( pCamDevice->baseAddr, CAM_IBSA1 );
        ibea1 = BRCM_READ_REG ( pCamDevice->baseAddr, CAM_IBEA1 );

        icc = BRCM_READ_REG ( pCamDevice->baseAddr, CAM_ICC );
        ics = BRCM_READ_REG ( pCamDevice->baseAddr, CAM_ICS );
        idc = BRCM_READ_REG ( pCamDevice->baseAddr, CAM_IDC );
        idpo = BRCM_READ_REG ( pCamDevice->baseAddr, CAM_IDPO );
        idca = BRCM_READ_REG ( pCamDevice->baseAddr, CAM_IDCA );
        idcd = BRCM_READ_REG ( pCamDevice->baseAddr, CAM_IDCD );
        ids = BRCM_READ_REG ( pCamDevice->baseAddr, CAM_IDS );
    // data
        dbsa1 = BRCM_READ_REG ( pCamDevice->baseAddr, CAM_DBSA1 );
        dbea1 = BRCM_READ_REG ( pCamDevice->baseAddr, CAM_DBEA1 );
        dbctl = BRCM_READ_REG ( pCamDevice->baseAddr, CAM_DBCTL );
        misc = BRCM_READ_REG ( pCamDevice->baseAddr, CAM_MISC );

	printk(KERN_INFO "CTL=0x%x STA=0x%x ANA=0x%x PRI=0x%x "
		"CLK=0x%x CLT=0x%x DAT0=0x%x DAT1=0x%x DLT=0x%x\r\n",
		ctl, sta, ana, pri, clk, clt, dat0, dat1, dlt);
	printk(KERN_INFO "CMP0=0x%x CMP1=0x%x CAP0=0x%x CAP1=0x%x "
		"DBG0=0x%x DBG1=0x%x DBG2=0x%x DBG3=0x%x\r\n",
		cmp0, cmp1, cap0, cap1, dbg0, dbg1, dbg2, dbg3);
	printk(KERN_INFO "ICTL=0x%x ISTA=0x%x IDI=0x%x IPIPE=0x%x "
		"IBSA=0x%x IBEA=0x%x IBLS=0x%x\r\n",
		ictl, ista, idi, ipipe, ibsa, ibea, ibls);
	printk(KERN_INFO "IBSA1=0x%x IBEA1=0x%x IDI1=0x%x "
		"IBWP=0x%x IHWIN=0x%x IHSTA=0x%x IVWIN=0x%x IVSTA=0x%x\r\n",
		ibsa1, ibea1, idi1, ibwp, ihwin, ihsta, ivwin, ivsta);
	printk(KERN_INFO "DCS=0x%x DBSA=0x%x DBEA=0x%x DBWP=0x%x "
		"DBSA1=0x%x DBEA1=0x%x DBCTL=0x%x\r\n",
		dcs, dbsa, dbea, dbwp, dbsa1, dbea1, dbctl);
	printk(KERN_INFO "ICC=0x%x ICS=0x%x IDC=0x%x IDPO=0x%x IDCA=0x%x "
		"IDCD=0x%x IDS=0x%x MISC=0x%x\r\n\r\n",
		icc, ics, idc, idpo, idca, idcd, ids, misc);

	return chal_status;
}

//***************************************************************************
/**
*       chalCamCopy
*/
static void chalCamCopy( cUInt8 *wr_addr, cUInt8 *rd_addr, cUInt32 size )
{
    cUInt32 i;

    for (i=0; i<size; i++)
    {
        *wr_addr++ = *rd_addr++;
    }
} 


//==============================================================================
//! \brief 
//!         Initialize CHAL CAM for the passed CAM instance
//! \note 
//!         Parameters:   
//!             baseAddr, mapped address of this CAM instance
//!         Return:      
//!             CHAL handle for this CAM instance
//==============================================================================
CHAL_HANDLE chal_cam_init(cUInt32 baseAddr)
{
    chal_cam_t *pCamDevice=NULL;

    DBG_OUT( chal_dprintf( CDBG_INFO, "chal_cam_init\n") );

// Assign handle
    pCamDevice = &cam_dev[0];

// Check if init already
    if( pCamDevice->init != TRUE )
    {
        pCamDevice->baseAddr = baseAddr;
        pCamDevice->init = TRUE;
    }
    else
    {
        DBG_OUT( chal_dprintf(CDBG_INFO, "chal_cam_init: already initialized\n") );
    }         
    
	DBG_OUT(chal_dprintf(CDBG_INFO, "chal_cam_init: Register base=0x%x\n",
						pCamDevice->baseAddr));

    return (CHAL_HANDLE)pCamDevice;
}


//==============================================================================
//! \brief 
//!         De-Initialize CHAL CAM for the passed CAM instance
//! \note 
//!         Parameters:   
//!             handle  (in) this CAM instance
//!             cfg     (in) configuration parameters to be set
//!         Return:
//!             CHAL_CAM_STATUS_CODES
//==============================================================================
cVoid chal_cam_deinit(CHAL_HANDLE handle)
{
    chal_cam_t *pCamDevice = (chal_cam_t *)handle;

    if (pCamDevice == NULL)
    {
        DBG_OUT( chal_dprintf(CDBG_INFO, "chal_cam_deinit: already NULL handle\n") );
    }
    else
    {
        pCamDevice->baseAddr = 0;
        pCamDevice->init = FALSE;
    }
}

//==============================================================================
//! \brief 
//!         Configure CAM module Control Settings
//! \note 
//!         Parameters:   
//!             handle  (in) this CAM instance
//!             cfg     (in) configuration parameters to be set
//!         Return:
//!             CHAL_CAM_STATUS_CODES
//==============================================================================
CHAL_CAM_STATUS_CODES chal_cam_cfg_intf(CHAL_HANDLE handle, CHAL_CAM_CFG_INTF_st_t* cfg)
{
    chal_cam_t *pCamDevice = (chal_cam_t *)handle;
    CHAL_CAM_STATUS_CODES chal_status = CHAL_OP_OK;
    CHAL_CAM_PARAM_st_t chal_cam_param_st;
    cUInt32 cfg_base_addr;
    cUInt32 mm_switch_base;
    u32 valsw;
    DBG_OUT( chal_dprintf( CDBG_INFO, "chal_cam_cfg_intf\n") ); 


	printk(KERN_ERR "chal_cam_cfg_intf() base=0x%x\n",
					pCamDevice->baseAddr);
    // Set Camera CSIx Phy & Clock Registers
        cfg_base_addr = HW_IO_PHYS_TO_VIRT(MM_CFG_BASE_ADDR);

        // CSI0 has PHY selection.
            if (cfg->afe_port == CHAL_CAM_PORT_AFE_1) 
                BRCM_WRITE_REG(cfg_base_addr,MM_CFG_CSI1_LDO_CTL, 0x5A00000F);
            else 
                BRCM_WRITE_REG(cfg_base_addr,MM_CFG_CSI0_LDO_CTL, 0x5A00000F);

/* MM_CFG changes here for QoS*/
#ifndef __KERNEL__
mm_switch_base = MM_CFG_BASE_ADDR;
#else
mm_switch_base = HW_IO_PHYS_TO_VIRT(MM_CFG_BASE_ADDR);
valsw = readl((mm_switch_base + 0x444));
valsw = valsw | 1;
writel(valsw, (mm_switch_base + 0x444));
#endif
/* Enable CSI QOS in MM switch */

// Memories Enabled
    BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_CTL,MEN,1);

// Analog PHY Setup
    // Analog Power Up, Reset & wait for Phy to settle
        BRCM_WRITE_REG(pCamDevice->baseAddr,CAM_ANA,( (1 << CAM_ANA_AR_SHIFT) | (0 << CAM_ANA_DDL_SHIFT) | (0x07<<CAM_ANA_CTATADJ_SHIFT) | (0x07<<CAM_ANA_PTATADJ_SHIFT) ));
    // Disable Analog Reset
        BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_ANA,AR,0);

    // Set Cam Interface
        chal_status |= chal_cam_set_intf(handle, cfg);
    // Pipeline
        BRCM_WRITE_REG(pCamDevice->baseAddr,CAM_IPIPE,0x00000000);
    // Packet Compare
        BRCM_WRITE_REG(pCamDevice->baseAddr,CAM_CMP0,0x00000000);
        BRCM_WRITE_REG(pCamDevice->baseAddr,CAM_CMP1,0x00000000);
    // Reset Interface
        chal_cam_param_st.param = CHAL_CAM_RESET_RX;
        chal_status |= chal_cam_reset(handle, &chal_cam_param_st);
    
    return chal_status;
}    
    
//==============================================================================
//! \brief 
//!         Configure CAM module Control Settings
//! \note 
//!         Parameters:   
//!             handle  (in) this CAM instance
//!             cfg     (in) configuration parameters to be set
//!         Return:
//!             CHAL_CAM_STATUS_CODES
//==============================================================================
CHAL_CAM_STATUS_CODES chal_cam_cfg_cntrl(CHAL_HANDLE handle, CHAL_CAM_CFG_CNTRL_st_t* cfg)
{
    CHAL_CAM_STATUS_CODES chal_status = CHAL_OP_OK;
    chal_cam_t *pCamDevice = (chal_cam_t *)handle;
    cUInt32 reg;

    DBG_OUT( chal_dprintf( CDBG_INFO, "chal_cam_cfg_cntrl\n") ); 

// Update Interface CFG Control
    chalCamCopy((cUInt8 *)&pCamDevice->cfg_cntrl_st, (cUInt8 *)cfg, sizeof(CHAL_CAM_CFG_CNTRL_st_t) );
       
// Rx Packet Timeout
    BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_CTL,PFT,cfg->packet_timeout);

// Receiver Burst Length
    if ( (cfg->burst_length > 8) || (cfg->burst_length == 0) )
    {
        reg = 0;        // CHAL_CAM_BURST_LENGTH_16
    }
    else if (cfg->burst_length > 4)
    {
        reg = 1;        // CHAL_CAM_BURST_LENGTH_8
    }
    else if (cfg->burst_length > 2)
    {
        reg = 2;        // CHAL_CAM_BURST_LENGTH_4
    }
    else 
    {
        reg = 3;        // CHAL_CAM_BURST_LENGTH_2
    }
    BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_PRI,BL,reg);
// Receiver Burst Spacing
    if (cfg->burst_space <= 2)
    {
        reg = 0;        // CHAL_CAM_BURST_SPACE_2
    }
    else if (cfg->burst_space <= 4)
    {
        reg = 1;        // CHAL_CAM_BURST_SPACE_4
    }
    else if (cfg->burst_space <= 8)
    {
        reg = 2;        // CHAL_CAM_BURST_SPACE_8
    }
    else if (cfg->burst_space <= 16)
    {
        reg = 3;        // CHAL_CAM_BURST_SPACE_16
    }
    else if (cfg->burst_space <= 32)
    {
        reg = 4;        // CHAL_CAM_BURST_SPACE_32
    }
    else if (cfg->burst_space <= 64)
    {
        reg = 5;        // CHAL_CAM_BURST_SPACE_64
    }
    else if (cfg->burst_space <= 128)
    {
        reg = 6;        // CHAL_CAM_BURST_SPACE_128
    }
    else if (cfg->burst_space <= 256)
    {
        reg = 7;        // CHAL_CAM_BURST_SPACE_256
    }
    else if (cfg->burst_space <= 512)
    {
        reg = 8;        // CHAL_CAM_BURST_SPACE_512
    }
    else
    {
        reg = 9;        // CHAL_CAM_BURST_SPACE_1024
    }
    BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_PRI,BS,reg);
// Panic Priority
    BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_PRI,PP,cfg->panic_pr);
// Normal Priority
    BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_PRI,NP,cfg->norm_pr);
// Panic Threshold
    BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr, CAM_PRI, PT, cfg->panic_thr);
    BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr, CAM_PRI, PE, cfg->panic_enable);
#if 0
    printk("Camera: Urgent request enable set to %d\n", cfg->panic_enable);
#endif
    return chal_status;
}    


//==============================================================================
//! \brief 
//!         Configure CAM Channel pipeline
//! \note 
//!         Parameters:   
//!             handle  (in) this CAM instance
//!             cfg     (in) configuration parameters to be set
//!         Return:
//!             CHAL_CAM_STATUS_CODES
//==============================================================================
CHAL_CAM_STATUS_CODES chal_cam_cfg_pipeline(CHAL_HANDLE handle, CHAL_CAM_PIPELINE_CFG_st_t* cfg)
{
    CHAL_CAM_STATUS_CODES chal_status = CHAL_OP_OK;
    chal_cam_t *pCamDevice = (chal_cam_t *)handle;
    cUInt32 blk, dpcm, pack;
    
    DBG_OUT( chal_dprintf( CDBG_INFO, "chal_cam_cfg_pipeline\n") ); 

// Image Downsize Mode
    BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_IPIPE,IDM,0x00);
// Image Correction Mode
    BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_IPIPE,ICM,0x00);

// Encode Block
    if (cfg->enc_blk_lnth <= CHAL_CAM_ENC_BLK_LTH_0)
    {
        blk = 0;
    }
    else if (cfg->enc_blk_lnth <= CHAL_CAM_ENC_BLK_LTH_8)
    {
        blk = 1;
    }
    else if (cfg->enc_blk_lnth <= CHAL_CAM_ENC_BLK_LTH_16)
    {
        blk = 2;
    }
    else if (cfg->enc_blk_lnth <= CHAL_CAM_ENC_BLK_LTH_32)
    {
        blk = 3;
    }
    else if (cfg->enc_blk_lnth <= CHAL_CAM_ENC_BLK_LTH_64)
    {
        blk = 4;
    }
    else if (cfg->enc_blk_lnth <= CHAL_CAM_ENC_BLK_LTH_128)
    {
        blk = 5;
    }
    else if (cfg->enc_blk_lnth <= CHAL_CAM_ENC_BLK_LTH_256)
    {
        blk = 6;
    }
    else if (cfg->enc_blk_lnth <= CHAL_CAM_ENC_BLK_LTH_512)
    {
        blk = 7;
    }
    else 
    {
        blk = 0;
        chal_status |= CHAL_OP_INVALID;
        chal_dprintf( CDBG_ERRO, "ERROR:  chal_cam_cfg_pipeline:  Encoding Block Size %d not allowed\n", cfg->enc_blk_lnth);    
    }
    BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_IPIPE,DEBL,blk);

// Encode Processing
    switch (cfg->enc_proc)
    {
        case CHAL_CAM_10_8_DPCM_ENC:
            dpcm = 1;
            break;
        case CHAL_CAM_12_8_DPCM_ENC:
            dpcm = 2;
            break;
        case CHAL_CAM_14_8_DPCM_ENC:
            dpcm = 3;
            break;
        case CHAL_CAM_ENC_NONE:
        default:
            dpcm = 0;
            break;
    }
    BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_IPIPE,DEM,dpcm);

// Encode Packing
    switch (cfg->enc_pixel_pack)
    {
        case CHAL_CAM_PIXEL_8BIT:
            pack = 1;
            break;
        case CHAL_CAM_PIXEL_10BIT:
            pack = 2;
            break;
        case CHAL_CAM_PIXEL_12BIT:
            pack = 3;
            break;
        case CHAL_CAM_PIXEL_14BIT:
            pack = 4;
            break;
        case CHAL_CAM_PIXEL_16BIT:
            pack = 5;
            break;
        case CHAL_CAM_PIXEL_NONE:
        default:
            pack = 0;
            break;
    }
    BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_IPIPE,PPM,pack);

// Decode Processing
    switch (cfg->dec_proc)
    {
        case CHAL_CAM_8_10_DPCM_DEC:
            dpcm = 1;
            break;
        case CHAL_CAM_7_10_DPCM_DEC:
            dpcm = 2;
            break;
        case CHAL_CAM_6_10_DPCM_DEC:
            dpcm = 3;
            break;
        case CHAL_CAM_8_12_DPCM_DEC:
            dpcm = 4;
            break;
        case CHAL_CAM_7_12_DPCM_DEC:
            dpcm = 5;
            break;
        case CHAL_CAM_6_12_DPCM_DEC:
            dpcm = 6;
            break;
        case CHAL_CAM_10_14_DPCM_DEC:
            dpcm = 7;
            break;
        case CHAL_CAM_8_14_DPCM_DEC:
            dpcm = 8;
            break;
        case CHAL_CAM_12_16_DPCM_DEC:
            dpcm = 9;
            break;
        case CHAL_CAM_10_16_DPCM_DEC:
            dpcm = 10;
            break;
        case CHAL_CAM_8_16_DPCM_DEC:
            dpcm = 11;
            break;
        case CHAL_CAM_DEC_NONE:
        default:
            dpcm = 0;
            break;
    }
    BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_IPIPE,DDM,dpcm);

// Decode UnPacking
    switch (cfg->dec_pixel_unpack)
    {
        case CHAL_CAM_PIXEL_6BIT:
            pack = 1;
            break;
        case CHAL_CAM_PIXEL_7BIT:
            pack = 2;
            break;
        case CHAL_CAM_PIXEL_8BIT:
            pack = 3;
            break;
        case CHAL_CAM_PIXEL_10BIT:
            pack = 4;
            break;
        case CHAL_CAM_PIXEL_12BIT:
            pack = 5;
            break;
        case CHAL_CAM_PIXEL_14BIT:
            pack = 6;
            break;
        case CHAL_CAM_PIXEL_16BIT:
            pack = 7;
            break;
        case CHAL_CAM_PIXEL_NONE:
        default:
            pack = 0;
            break;
    }
    BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_IPIPE,PUM,pack);
#if 0
// Data Decode Processing
    switch (cfg->data_dec_proc)
    {
        case CHAL_CAM_FSP_DEC:
            reg = 1;
            break;
        case CHAL_CAM_DEC_NONE:
        default:
            reg = 0;
            break;
    }
    BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_DCS,FDE,reg);
#endif

    return chal_status;
}        

//==============================================================================
//! \brief 
//!         Configure CAM Channel x for Receive data
//! \note 
//!         Parameters:   
//!             handle  (in) this CAM instance
//!             cfg     (in) configuration parameters to be set
//!         Return:
//!             CHAL_CAM_STATUS_CODES
//==============================================================================

// add 16 bytes to buffer end addr if wrap-around not enabled to prevent UNDERRUN in image buffer
//#define CAM_BUFFER_UNDERRUN_FIX  0x00000010  

CHAL_CAM_STATUS_CODES chal_cam_cfg_buffer(CHAL_HANDLE handle, CHAL_CAM_BUFFER_CFG_st_t* cfg)
{
    CHAL_CAM_STATUS_CODES chal_status = CHAL_OP_OK;
    chal_cam_t *pCamDevice = (chal_cam_t *)handle;
    cUInt32 start_addr;

// We configure image0Buff and dataBuff parameters. image1Buff is ignored.
    if ( (cfg->buffers.image0Buff == NULL) && (cfg->buffers.data0Buff == NULL) )
    {
        chal_dprintf( CDBG_ERRO, "ERROR:  chal_cam_cfg_buffer:  Nothing to configure. Both image0Buff and dataBuff are NULL \n");   
        return CHAL_OP_INVALID;
    }

// Disable Double Buffering    
    BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_DBCTL,DB_EN,0);
    BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_DBCTL,BUF0_IE,0);
    BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_DBCTL,BUF1_IE,0);
    BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_MISC,DIS_DB_IE,1);

    if ( cfg->buffers.image0Buff != NULL )
    {
        if ( (cfg->buffers.image0Buff->start_addr & 0x0000000F) != 0 )
        {
            chal_dprintf( CDBG_ERRO, "ERROR:  chal_cam_cfg_buffer:  Image Start Addr *16 boundary = 0x%x \n", cfg->buffers.image0Buff->start_addr);   
            chal_status |= CHAL_OP_INVALID;
        }
        if ( ((cfg->buffers.image0Buff->start_addr + cfg->buffers.image0Buff->size) & 0x0000000F) != 0 )
        {
            chal_dprintf( CDBG_ERRO, "ERROR:  chal_cam_cfg_buffer: Image End Addr *16 boundary = 0x%x \n", (cfg->buffers.image0Buff->start_addr + cfg->buffers.image0Buff->size));   
            chal_status |= CHAL_OP_INVALID;
        }

        // Image Buffer OverRun Setting (If not set can lose up to 128 bytes before End Address, unless add 16 to end address)
            if (cfg->buffers.image0Buff->buf_wrap_enable == TRUE)
            {
            // Image Buffer Does not UNDERRUN or OVERRUN with wrap enabled
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_ICTL,IBOB,1);
            }
            else
            {
            // Image Buffer UNDERRUNS with wrap disabled
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_ICTL,IBOB,0);
            }
        // Image Start Addr
            start_addr = cfg->buffers.image0Buff->start_addr;
            BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_IBSA,IBSA,start_addr);
        // Image End Addr
            BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_IBEA,IBEA,(start_addr + cfg->buffers.image0Buff->size));
        // Image line stride
            if (cfg->buffers.image0Buff->line_stride & 0x0000000F)
            {
                chal_dprintf( CDBG_ERRO, "ERROR:  chal_cam_cfg_buffer:  Image Line Stride *16 boundary ERROR\n");   
                chal_status |= CHAL_OP_INVALID;
            }
            BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_IBLS,IBLS,cfg->buffers.image0Buff->line_stride);
        // Load Addr Pointers
            BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_ICTL,LIP,1);
    }
 
    if ( cfg->buffers.data0Buff != NULL )
    {
        if ( (cfg->buffers.data0Buff->start_addr & 0x0000000F) != 0 )
        {
            chal_dprintf( CDBG_ERRO, "ERROR:  chal_cam_cfg_buffer:  Data Start Addr *16 boundary = 0x%x \n", cfg->buffers.data0Buff->start_addr);   
            chal_status |= CHAL_OP_INVALID;
        }
        if ( ((cfg->buffers.data0Buff->start_addr + cfg->buffers.data0Buff->size) & 0x0000000F) != 0 )
        {
            chal_dprintf( CDBG_ERRO, "ERROR:  chal_cam_cfg_buffer:  Data End Addr *16 boundary = 0x%x \n", (cfg->buffers.data0Buff->start_addr + cfg->buffers.data0Buff->size - 1)); 
            chal_status |= CHAL_OP_INVALID;
        }
     // Data Buffer OVERRUNS with wrap enabled
        if (cfg->buffers.data0Buff->buf_wrap_enable == TRUE)
        {
        // Data Buffer OVERRUNS with wrap disabled
            BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_DCS,DBOB,1);
        }
        else
        {
        // Data Buffer UNDERRUNS with wrap disabled
            BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_DCS,DBOB,0);
        }
     // Data Start Addr
        start_addr = cfg->buffers.data0Buff->start_addr;
        BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_DBSA,DBSA,start_addr);
     // Data End Addr
        BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_DBEA,DBEA,(start_addr + cfg->buffers.data0Buff->size));
    // Load Addr Pointers
        BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_DCS,LDP,1);
    }
    return chal_status;
}    


//==============================================================================
//! \brief 
//!         Configure CAM Channel x for Receive data
//! \note 
//!         Parameters:   
//!             handle  (in) this CAM instance
//!             cfg     (in) configuration parameters to be set
//!         Return:
//!             CHAL_CAM_STATUS_CODES
//==============================================================================

CHAL_CAM_STATUS_CODES chal_cam_cfg_dbl_buffer(CHAL_HANDLE handle, CHAL_CAM_BUFFER_CFG_st_t* cfg)
{
    CHAL_CAM_STATUS_CODES chal_status = CHAL_OP_OK;
    chal_cam_t *pCamDevice = (chal_cam_t *)handle;
    cUInt32 start_addr;

// We configure image0Buff, image1Buff and data0Buff, data1Buff parameters. 
    if ( (cfg->buffers.image0Buff == NULL) || (cfg->buffers.image1Buff == NULL) || (cfg->buffers.data0Buff == NULL) || (cfg->buffers.data1Buff == NULL))
    {
        chal_dprintf( CDBG_ERRO, "ERROR:  chal_cam_cfg_dbl_buffer:  Buffer is NULL \n");   
        return CHAL_OP_INVALID;
    }

    if ( (cfg->buffers.image0Buff->start_addr & 0x0000000F) || (cfg->buffers.image1Buff->start_addr & 0x0000000F) || (cfg->buffers.data0Buff->start_addr & 0x0000000F) || (cfg->buffers.data1Buff->start_addr & 0x0000000F) )
    {
        chal_dprintf( CDBG_ERRO, "ERROR:  chal_cam_cfg_dbl_buffer:  Start Addr is not *16 boundary \n");   
        chal_status |= CHAL_OP_INVALID;
    }

    if ( ((cfg->buffers.image0Buff->start_addr + cfg->buffers.image0Buff->size) & 0x0000000F)
            || ((cfg->buffers.image1Buff->start_addr + cfg->buffers.image0Buff->size) & 0x0000000F)
            || ((cfg->buffers.data0Buff->start_addr + cfg->buffers.data0Buff->size) & 0x0000000F)
            || ((cfg->buffers.data1Buff->start_addr + cfg->buffers.data1Buff->size) & 0x0000000F) )
    {
        chal_dprintf( CDBG_ERRO, "ERROR:  chal_cam_cfg_dbl_buffer: End Addr is not *16 boundary \n");   
        chal_status |= CHAL_OP_INVALID;
    }

// Enable Double Buffering    
    BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_DBCTL,DB_EN,1);

// Image Buffer OverRun Setting (If not set can lose up to 128 bytes before End Address, unless add 16 to end address)
    if (cfg->buffers.image0Buff->buf_wrap_enable == TRUE)
    {
    // Image Buffer Does not UNDERRUN or OVERRUN with wrap enabled
        BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_ICTL,IBOB,1);
    }
    else
    {
    // Image Buffer UNDERRUNS with wrap disabled
        BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_ICTL,IBOB,0);
    }
// Image0 Start Addr
    start_addr = cfg->buffers.image0Buff->start_addr;
    BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_IBSA,IBSA,start_addr);
// Image0 End Addr
    BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_IBEA,IBEA,(start_addr + cfg->buffers.image0Buff->size));
// Image1 Start Addr
    start_addr = cfg->buffers.image1Buff->start_addr;
    BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_IBSA1,IBSA1,start_addr);
// Image1 End Addr
    BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_IBEA1,IBEA1,(start_addr + cfg->buffers.image1Buff->size));
// Image line stride
    if (cfg->buffers.image0Buff->line_stride & 0x0000000F)
    {
        chal_dprintf( CDBG_ERRO, "ERROR:  chal_cam_cfg_dbl_buffer:  Image Line Stride *16 boundary ERROR\n");   
        chal_status |= CHAL_OP_INVALID;
    }
    BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_IBLS,IBLS,cfg->buffers.image0Buff->line_stride);
// Load Addr Pointers
    BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_ICTL,LIP,1);


// Data Buffer OverRun Setting (If not set can lose up to 128 bytes before End Address, unless add 16 to end address)
    if (cfg->buffers.data0Buff->buf_wrap_enable == TRUE)
    {
     // Data Buffer OVERRUNS with wrap enabled
        BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_DCS,DBOB,1);
    }
    else
    {
    // Data Buffer UNDERRUNS with wrap disabled
        BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_DCS,DBOB,0);
    }
// Data0 Start Addr
    start_addr = cfg->buffers.data0Buff->start_addr;
    BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_DBSA,DBSA,start_addr);
// Data0 End Addr
    BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_DBEA,DBEA,(start_addr + cfg->buffers.data0Buff->size));
// Data1 Start Addr
    start_addr = cfg->buffers.data1Buff->start_addr;
    BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_DBSA1,DBSA1,start_addr);
// Data1 End Addr
    BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_DBEA1,DBEA1,(start_addr + cfg->buffers.data1Buff->size ));
// Load Addr Pointers
    BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_DCS,LDP,1);

// Enable Double Buffering    
    BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_DBCTL,BUF0_IE,1);
    BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_DBCTL,BUF1_IE,1);
    BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_DBCTL,DB_EN,1);
    BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_MISC,DIS_DB_IE,0);
    return chal_status;
}    

//==============================================================================
//! \brief 
//!         Configure CAM Channel x for Receive data
//! \note 
//!         Parameters:   
//!             handle  (in) this CAM instance
//!             cfg     (in) configuration parameters to be set
//!         Return:
//!             CHAL_CAM_STATUS_CODES
//==============================================================================
CHAL_CAM_STATUS_CODES chal_cam_cfg_data(CHAL_HANDLE handle, CHAL_CAM_DATA_CFG_st_t* cfg)
{
    CHAL_CAM_STATUS_CODES chal_status = CHAL_OP_OK;
    cUInt32 reg;
    chal_cam_t *pCamDevice = (chal_cam_t *)handle;

// Get Data Control Register
    reg = BRCM_READ_REG(pCamDevice->baseAddr,CAM_DCS);
    reg &= ~(CAM_DCS_DIE_MASK | CAM_DCS_DIM_MASK | CAM_DCS_EDL_MASK | CAM_DCS_FDE_MASK);
    // Update Data Line Count
        reg |= (cfg->line_count << CAM_DCS_EDL_SHIFT);
    // Data Interrupt Setting
        if (cfg->intr_enable & CHAL_CAM_INT_DATA_END)
        {
            reg |= (1 << CAM_DCS_DIM_SHIFT);
            reg |= (1 << CAM_DCS_DIE_SHIFT);
        }
        else if (cfg->intr_enable & CHAL_CAM_INT_DATA_FRAME_END)
        {
            reg |= (1 << CAM_DCS_DIE_SHIFT);
        }
    // Update FSP Decode 
        if (cfg->fsp_decode_enable)
        {
            reg |= (1 << CAM_DCS_FDE_SHIFT);
        }
    BRCM_WRITE_REG(pCamDevice->baseAddr,CAM_DCS,reg);
    return chal_status;
}    

//==============================================================================
//! \brief 
//!         Get CAM x Configuration for Receive Data
//! \note 
//!         Parameters:   
//!             handle  (in) this CAM instance
//!             cfg     (out) configuration parameters to be set
//!         Return:
//!             CHAL_CAM_STATUS_CODES
//==============================================================================
CHAL_CAM_STATUS_CODES chal_cam_get_buffer_cfg(CHAL_HANDLE handle, CHAL_CAM_BUFFER_CFG_st_t* cfg)
{
    chal_cam_t *pCamDevice = (chal_cam_t *)handle;
    CHAL_CAM_STATUS_CODES chal_status = CHAL_OP_OK;
    
    DBG_OUT( chal_dprintf( CDBG_INFO, "chal_cam_get_buffer_cfg\n") );  

    if (cfg->buffers.image0Buff != NULL)
    {
        DBG_OUT( chal_dprintf( CDBG_INFO, "chal_cam_get_cfg_data:  CHAL_CAM_IMAGE\n") );
    // Image Start Addr
        cfg->buffers.image0Buff->start_addr = BRCM_READ_REG(pCamDevice->baseAddr,CAM_IBSA);
    // Image End Addr
        cfg->buffers.image0Buff->size = BRCM_READ_REG(pCamDevice->baseAddr,CAM_IBEA) - cfg->buffers.image0Buff->start_addr;
    // Image line stride
        cfg->buffers.image0Buff->line_stride = BRCM_READ_REG(pCamDevice->baseAddr,CAM_IBLS);
    // Image buffer wrap enable
        cfg->buffers.image0Buff->buf_wrap_enable = BRCM_READ_REG_FIELD(pCamDevice->baseAddr,CAM_ICTL,IBOB);
    // Image write pointer
        cfg->write_ptr = BRCM_READ_REG(pCamDevice->baseAddr,CAM_IBWP);
    // Image bytes/line
        cfg->bytes_per_line = BRCM_READ_REG_FIELD(pCamDevice->baseAddr,CAM_IHSTA,PPL);
    // Image lines/frame
        cfg->lines_per_frame = BRCM_READ_REG_FIELD(pCamDevice->baseAddr,CAM_IVSTA,LPF);
    }

    if (cfg->buffers.data0Buff != NULL)
    {
        DBG_OUT( chal_dprintf( CDBG_INFO, "chal_cam_get_cfg_data:  CHAL_CAM_DATA\n") );
    // Data Start Addr
        cfg->buffers.data0Buff->start_addr = BRCM_READ_REG(pCamDevice->baseAddr,CAM_DBSA);
    // Data End Addr
        cfg->buffers.data0Buff->size = BRCM_READ_REG(pCamDevice->baseAddr,CAM_DBEA) - cfg->buffers.data0Buff->start_addr;
    // line stride
        cfg->buffers.data0Buff->line_stride = 0;
    // Data buffer wrap enable
        cfg->buffers.data0Buff->buf_wrap_enable = BRCM_READ_REG_FIELD(pCamDevice->baseAddr,CAM_DCS,DBOB);
    // Data write pointer
        cfg->write_ptr = BRCM_READ_REG(pCamDevice->baseAddr,CAM_DBWP);
    }
    return chal_status;
}    
//==============================================================================
//! \brief 
//!         Configure CAM Channel x for Receive
//! \note 
//!         Parameters:   
//!             handle  (in) this CAM instance
//!             cfg     (in) configuration parameters to be set
//!         Return:
//!             CHAL_CAM_STATUS_CODES
//==============================================================================
CHAL_CAM_STATUS_CODES chal_cam_cfg_frame(CHAL_HANDLE handle, CHAL_CAM_FRAME_CFG_st_t* cfg)
{
    CHAL_CAM_STATUS_CODES chal_status = CHAL_OP_OK;
    chal_cam_t *pCamDevice = (chal_cam_t *)handle;
    cUInt32 reg;
    
    DBG_OUT( chal_dprintf( CDBG_INFO, "chal_cam_cfg_chan\n") ); 

// Get Image Control Register
    reg = BRCM_READ_REG(pCamDevice->baseAddr,CAM_ICTL);
    reg &= ~(CAM_ICTL_FSIE_MASK | CAM_ICTL_FEIE_MASK | CAM_ICTL_LCIE_MASK | CAM_ICTL_FCM_MASK);
    // Interrupt Setting
        if (cfg->interrupts & CHAL_CAM_INT_FRAME_START)
        {
            reg |= (1 << CAM_ICTL_FSIE_SHIFT);
        }
        if (cfg->interrupts & CHAL_CAM_INT_FRAME_END)
        {
            reg |= (1 << CAM_ICTL_FEIE_SHIFT);
        }
        if (cfg->interrupts & CHAL_CAM_INT_LINE_COUNT)
        {
            reg |= (cfg->line_count << CAM_ICTL_LCIE_SHIFT);
        }
      // Frame Capture Setting
        if (cfg->mode == CHAL_CAM_SHOT_MODE_SINGLE)
        {
            reg |= (1 << CAM_ICTL_FCM_SHIFT);
        }
// Update Image Control Register
    BRCM_WRITE_REG(pCamDevice->baseAddr,CAM_ICTL,reg);
    
    return chal_status;
}    



//==============================================================================
//! \brief 
//!         Configure CAM Image Id's for Receive
//! \note 
//!         Parameters:   
//!             handle  (in) this CAM instance
//!             cfg     (in) configuration parameters to be set
//!         Return:
//!             CHAL_CAM_STATUS_CODES
//==============================================================================
CHAL_CAM_STATUS_CODES chal_cam_cfg_image_id(CHAL_HANDLE handle, CHAL_CAM_IMAGE_ID_st_t* cfg)
{
    CHAL_CAM_STATUS_CODES chal_status = CHAL_OP_OK;
    chal_cam_t *pCamDevice = (chal_cam_t *)handle;
    cUInt32 reg;

// CSI Interface
    if (cfg->intf == CHAL_CAM_INTF_CSI)
    {
    // Set Image Data Identifier Register ID0-ID3
        reg = (cfg->image_data_id_0 << CAM_IDI_IDI0_SHIFT) | (cfg->image_data_id_1 << CAM_IDI_IDI1_SHIFT) |(cfg->image_data_id_2 << CAM_IDI_IDI2_SHIFT) | (cfg->image_data_id_3 << CAM_IDI_IDI3_SHIFT);
    // Update Image Data Identifier Register
        BRCM_WRITE_REG(pCamDevice->baseAddr,CAM_IDI,reg);
    // Set Image Data Identifier Register ID4-ID7
        reg = (cfg->image_data_id_4 << CAM_IDI1_IDI4_SHIFT) | (cfg->image_data_id_5 << CAM_IDI1_IDI5_SHIFT) |(cfg->image_data_id_6 << CAM_IDI1_IDI6_SHIFT) | (cfg->image_data_id_7 << CAM_IDI1_IDI7_SHIFT);
    // Update Image Data Identifier Register
        BRCM_WRITE_REG(pCamDevice->baseAddr,CAM_IDI1,reg);
    }
// CCP Interface
    else 
    {
    // Set Image Data Identifier Register
        reg = ( (0x80 | cfg->image_data_id_0) << CAM_IDI_IDI0_SHIFT);
    // Update Image Data Identifier Register
        BRCM_WRITE_REG(pCamDevice->baseAddr,CAM_IDI,reg);
        BRCM_WRITE_REG(pCamDevice->baseAddr,CAM_IDI1,0x00000000);
    }
    return chal_status;
}    
    
//==============================================================================
//! \brief 
//!         Configure CAM Channel x windowing
//! \note 
//!         Parameters:   
//!             handle  (in) this CAM instance
//!             cfg     (in) configuration parameters to be set
//!         Return:
//!             CHAL_CAM_STATUS_CODES
//==============================================================================
CHAL_CAM_STATUS_CODES chal_cam_cfg_window(CHAL_HANDLE handle, CHAL_CAM_WINDOW_CFG_st_t* cfg)
{
    CHAL_CAM_STATUS_CODES chal_status = CHAL_OP_OK;
    chal_cam_t *pCamDevice = (chal_cam_t *)handle;
    cUInt32 reg;
    
    DBG_OUT( chal_dprintf( CDBG_INFO, "chal_cam_cfg_window\n") );   

// Windowing
    if (cfg->enable)
    {    
    // Set Horizontal Window
        reg = (cfg->h_start_sample << CAM_IHWIN_HWSP_SHIFT);
        reg |= (cfg->h_end_sample << CAM_IHWIN_HWEP_SHIFT);
        BRCM_WRITE_REG(pCamDevice->baseAddr,CAM_IHWIN,reg);
    // Set Vertical Window
        reg = (cfg->v_start_sample << CAM_IVWIN_VWSL_SHIFT);
        reg |= (cfg->v_end_sample << CAM_IVWIN_VWEL_SHIFT);
        BRCM_WRITE_REG(pCamDevice->baseAddr,CAM_IVWIN,reg);
    }
    else
    {
        reg = 0;
        BRCM_WRITE_REG(pCamDevice->baseAddr,CAM_IHWIN,reg);
        BRCM_WRITE_REG(pCamDevice->baseAddr,CAM_IVWIN,reg);
    }
    
    return chal_status;
}    

//==============================================================================
//! \brief 
//!          Configure CAM module AFE Controller
//! \note 
//!         Parameters:   
//!             handle  (in) this CAM instance
//!             cfg     (in) configuration parameters to be set
//!         Return:
//!             CHAL_CAM_STATUS_CODES
//==============================================================================
CHAL_CAM_STATUS_CODES chal_cam_cfg_afe_cntrl(CHAL_HANDLE handle, CHAL_CAM_AFE_CFG_st_t* cfg)
{
    CHAL_CAM_STATUS_CODES chal_status = CHAL_OP_OK;
    chal_cam_t *pCamDevice = (chal_cam_t *)handle;
    cUInt32 reg;

    reg = BRCM_READ_REG(pCamDevice->baseAddr,CAM_ANA);
    reg &= ~(CAM_ANA_PTATADJ_MASK | CAM_ANA_CTATADJ_MASK | CAM_ANA_DDL_MASK);

    // Analog Bandgap Bias Control
        reg |= (cfg->bandgap_bias << CAM_ANA_PTATADJ_SHIFT);
        reg |= (cfg->bandgap_bias << CAM_ANA_CTATADJ_SHIFT);
        if (cfg->data_lane_disable == 1)
        {
            reg |= (1 << CAM_ANA_DDL_SHIFT);
        }
    BRCM_WRITE_REG(pCamDevice->baseAddr,CAM_ANA,reg);
    return chal_status;
}    

//==============================================================================
//! \brief 
//!         CAM module Set Analog Power
//! \note 
//!         Parameters:   
//!             handle          (in) this CAM instance
//!             param.intf      (in) interface select       (unused)
//!             param.chan      (in) channel select         (unused)
//!             param.lane      (in) lane select            (unused)
//!             param.select    (in) cfg select             
//!             param.virt_chan (in) virtual channel select (unused)
//!             param.param     (in) 0=OFF  1=ON
//!         Return:
//!             CHAL_CAM_STATUS_CODES
//==============================================================================
CHAL_CAM_STATUS_CODES chal_cam_set_analog_pwr(CHAL_HANDLE handle, CHAL_CAM_PARAM_st_t* param)
{
    CHAL_CAM_STATUS_CODES chal_status = CHAL_OP_OK;
    chal_cam_t *pCamDevice = (chal_cam_t *)handle;
    
    DBG_OUT( chal_dprintf( CDBG_INFO, "chal_cam_set_analog_pwr = %d\n", param->param) );    


// Set Analog Power
    if (param->param == FALSE)
    {
    // if Power OFF, Check if channels enabled        
        if ( (BRCM_READ_REG_FIELD(pCamDevice->baseAddr,CAM_CLK,CLE) == 1) || (BRCM_READ_REG_FIELD(pCamDevice->baseAddr,CAM_DAT0,DLEN) == 1) || (BRCM_READ_REG_FIELD(pCamDevice->baseAddr,CAM_DAT1,DLEN) == 1))
        {
            DBG_OUT( chal_dprintf( CDBG_INFO, "chal_cam_set_analog_pwr: OFF: Channels Enabled\n") );    
            chal_status |= CHAL_OP_WRONG_ORDER;
        }
    // Reset Analog Receiver
        BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_ANA,AR,1);
    // Analog Power Down
        BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_ANA,APD,1);
    }
    else
    {
    // Analog Power Up
        BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_ANA,APD,0);
    // Reset Analog Receiver
        BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_ANA,AR,1);
        BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_ANA,AR,0);
    }    
    
    return chal_status;
}    

//==============================================================================
//! \brief 
//!         CAM module Set Bandgap Power
//! \note 
//!         Parameters:   
//!             handle          (in) this CAM instance
//!             param.intf      (in) interface select       (unused)
//!             param.chan      (in) channel select         (unused)
//!             param.lane      (in) lane select            (unused)
//!             param.select    (in) cfg select             (unused)
//!             param.virt_chan (in) virtual channel select (unused)
//!             param.param     (in) 0=OFF  1=ON
//==============================================================================
CHAL_CAM_STATUS_CODES chal_cam_set_bandgap_pwr(CHAL_HANDLE handle, CHAL_CAM_PARAM_st_t* param)
{
    CHAL_CAM_STATUS_CODES chal_status = CHAL_OP_OK;
    chal_cam_t *pCamDevice = (chal_cam_t *)handle;
    
    DBG_OUT( chal_dprintf( CDBG_INFO, "chal_cam_set_bandgap_pwr = %d\n", param->param) );   

// Set Analog Power
    if (param->param == FALSE)
    {
    // if Power OFF, Check if channels enabled        
        if ( (BRCM_READ_REG_FIELD(pCamDevice->baseAddr,CAM_CLK,CLE) == 1) || (BRCM_READ_REG_FIELD(pCamDevice->baseAddr,CAM_DAT0,DLEN) == 1) || (BRCM_READ_REG_FIELD(pCamDevice->baseAddr,CAM_DAT1,DLEN) == 1))
        {
            DBG_OUT( chal_dprintf( CDBG_INFO, "chal_cam_set_bandgap_pwr: OFF: Channels Enabled\n") );   
            chal_status |= CHAL_OP_WRONG_ORDER;
        }
    // Bandgap Power Down
        BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_ANA,BPD,1);
    }
    else
    {
    // Bandgap Power Up
        BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_ANA,BPD,0);
    }    
    
    return chal_status;
}    

//==============================================================================
//! \brief 
//!         CAM module Set Lane Enable
//! \note 
//!         Parameters:   
//!             handle          (in) this CAM instance
//!             param.intf      (in) interface select       
//!             param.chan      (in) channel select         (unused)
//!             param.lane      (in) lane select            
//!             param.select    (in) cfg select             (unused)
//!             param.virt_chan (in) virtual channel select (unused)
//!             param.param     (in) if (CHAL_CAM_LANE_SELECT_t == 1) Lane = ON, else Lane = OFF
//!         Return:
//!             CHAL_CAM_STATUS_CODES
//==============================================================================
CHAL_CAM_STATUS_CODES chal_cam_lane_enable(CHAL_HANDLE handle, CHAL_CAM_PARAM_st_t* param)
{
    CHAL_CAM_STATUS_CODES chal_status = CHAL_OP_OK;
    chal_cam_t *pCamDevice = (chal_cam_t *)handle;
    cUInt32 reg;
    
    DBG_OUT( chal_dprintf( CDBG_INFO, "chal_cam_lane_enable: lane=0x%x enable=0x%x\n", param->lane, param->param) );    
    
// CSI2 Interface
    if (param->intf == CHAL_CAM_INTF_CSI)
    {
    // Lane 0
        if (param->lane & CHAL_CAM_DATA_LANE_0)
        {
        // Clear Data Lane 0 bits for CSI mode
            reg = BRCM_READ_REG(pCamDevice->baseAddr,CAM_DAT0);
            reg &= ~(CAM_DAT0_DLEN_MASK | CAM_DAT0_DLPDN_MASK | CAM_DAT0_DLLPEN_MASK | CAM_DAT0_DLHSEN_MASK | CAM_DAT0_DLTREN_MASK);
            if (param->param & CHAL_CAM_DATA_LANE_0)
            {
            // Enable Data Lane 0
                reg |= ( (1 << CAM_DAT0_DLEN_SHIFT)|(0 << CAM_DAT0_DLPDN_SHIFT)|(1 << CAM_DAT0_DLLPEN_SHIFT)|(0 << CAM_DAT0_DLHSEN_SHIFT)|(0 << CAM_DAT0_DLTREN_SHIFT)|(1 << CAM_DAT0_DLSEN_SHIFT)|(1 << CAM_DAT0_DLFON_SHIFT)|(1 << CAM_DAT0_DLSTEN_SHIFT)  );
            }
            else
            {
            // Disable Data Lane 0
                reg |= (1 << CAM_DAT0_DLPDN_SHIFT);
            }
            BRCM_WRITE_REG(pCamDevice->baseAddr,CAM_DAT0,reg);
        }
    // Lane 1
        if (param->lane & CHAL_CAM_DATA_LANE_1)
        {
        // Clear Data Lane 1 bits for CSI mode
            reg = BRCM_READ_REG(pCamDevice->baseAddr,CAM_DAT1);
            reg &= ~(CAM_DAT1_DLEN_MASK | CAM_DAT1_DLPDN_MASK | CAM_DAT1_DLLPEN_MASK | CAM_DAT1_DLHSEN_MASK | CAM_DAT1_DLTREN_MASK);
            if (param->param & CHAL_CAM_DATA_LANE_1)
            {
            // Enable Data Lane 1
                reg |= ( (1 << CAM_DAT1_DLEN_SHIFT)|(0 << CAM_DAT1_DLPDN_SHIFT)|(1 << CAM_DAT1_DLLPEN_SHIFT)|(0 << CAM_DAT1_DLHSEN_SHIFT)|(0 << CAM_DAT1_DLTREN_SHIFT)|(1 << CAM_DAT1_DLSEN_SHIFT)|(1 << CAM_DAT1_DLFON_SHIFT)|(1 << CAM_DAT1_DLSTEN_SHIFT)  );
            }
            else
            {
            // Disable Data Lane 1
                reg |= (1 << CAM_DAT1_DLPDN_SHIFT);
            }
            BRCM_WRITE_REG(pCamDevice->baseAddr,CAM_DAT1,reg);
        }
    // Clock
        if (param->lane & CHAL_CAM_CLK_LANE)
        {
        // Clear Clock Lane bits for CSI mode
            reg = BRCM_READ_REG(pCamDevice->baseAddr,CAM_CLK);
            reg &= ~(CAM_CLK_CLE_MASK | CAM_CLK_CLPD_MASK | CAM_CLK_CLLPE_MASK | CAM_CLK_CLHSE_MASK | CAM_CLK_CLTRE_MASK);
            if (param->param & CHAL_CAM_CLK_LANE)
            {
            // Enable Clock Lane
                reg |= ( (1 << CAM_CLK_CLE_SHIFT)|(0 << CAM_CLK_CLPD_SHIFT)|(1 << CAM_CLK_CLLPE_SHIFT)|(1 << CAM_CLK_CLHSE_SHIFT)|(1 << CAM_CLK_CLTRE_SHIFT) | (1 << CAM_CLK_CLSTE_SHIFT) );
            }
            else
            {
            // Disable Clock Lane
                reg |= (1 << CAM_CLK_CLPD_SHIFT);
            }
            BRCM_WRITE_REG(pCamDevice->baseAddr,CAM_CLK,reg);
        }
    }
    else
// CCP2 Interface
    {
    // Lane 0
        if (param->lane & CHAL_CAM_DATA_LANE_0)
        {
        // Clear Data Lane 0 bits for CCP mode
            reg = BRCM_READ_REG(pCamDevice->baseAddr,CAM_DAT0);
            reg &= ~(CAM_DAT0_DLEN_MASK | CAM_DAT0_DLPDN_MASK | CAM_DAT0_DLLPEN_MASK | CAM_DAT0_DLHSEN_MASK | CAM_DAT0_DLTREN_MASK);
            if (param->param & CHAL_CAM_DATA_LANE_0)
            {
            // Enable Data Lane 0
                reg |= ( (1 << CAM_DAT0_DLEN_SHIFT) | (1 << CAM_DAT0_DLHSEN_SHIFT) | (1 << CAM_DAT0_DLTREN_SHIFT) );
            }
            else
            {
            // Disable Data Lane 0
                reg |= (1 << CAM_DAT0_DLPDN_SHIFT);
            }
            BRCM_WRITE_REG(pCamDevice->baseAddr,CAM_DAT0,reg);
        }
    // Lane 1
        if (param->lane & CHAL_CAM_DATA_LANE_1)
        {
         // Clear Data Lane 1 bits for CCP mode
            reg = BRCM_READ_REG(pCamDevice->baseAddr,CAM_DAT1);
            reg &= ~(CAM_DAT1_DLEN_MASK | CAM_DAT1_DLPDN_MASK | CAM_DAT1_DLTREN_MASK | CAM_DAT1_DLHSEN_MASK | CAM_DAT1_DLLPEN_MASK);
            if (param->param & CHAL_CAM_DATA_LANE_1)
            {
            // Enable Data Lane 1
                reg |= ( (1 << CAM_DAT1_DLEN_SHIFT) | (1 << CAM_DAT1_DLTREN_SHIFT) | (1 << CAM_DAT1_DLHSEN_SHIFT) );
            }
            else
            {
            // Disable Data Lane 1
                reg |= (1 << CAM_DAT1_DLPDN_SHIFT);
            }
            BRCM_WRITE_REG(pCamDevice->baseAddr,CAM_DAT1,reg);
        }
    // Clock
        if (param->lane & CHAL_CAM_CLK_LANE)
        {
        // Clear Clock Lane bits for CCP mode
            reg = BRCM_READ_REG(pCamDevice->baseAddr,CAM_CLK);
            reg &= ~(CAM_CLK_CLE_MASK | CAM_CLK_CLPD_MASK | CAM_CLK_CLTRE_MASK | CAM_CLK_CLHSE_MASK);
            if (param->param & CHAL_CAM_CLK_LANE)
            {
            // Enable Clock Lane
                reg |= ( (1 << CAM_CLK_CLE_SHIFT) | (1 << CAM_CLK_CLHSE_SHIFT) | (1 << CAM_CLK_CLTRE_SHIFT) );
            }
            else
            {
            // Disable Clock Lane
                reg |= (1 << CAM_CLK_CLPD_SHIFT);
            }
            BRCM_WRITE_REG(pCamDevice->baseAddr,CAM_CLK,reg);
        }
    }   
    return chal_status;
}    

//==============================================================================
//! \brief 
//!         CAM module Set Lane Control
//! \note 
//!         Parameters:   
//!             handle          (in) this CAM instance
//!             param.intf      (in) interface select
//!             param.chan      (in) unused
//!             param.lane      (in) lane select
//!             param.param     (in) if (CHAL_CAM_LANE_SELECT_t == 1) Lane = ON, else Lane = OFF
//!         Return:
//!             CHAL_CAM_STATUS_CODES
//==============================================================================
CHAL_CAM_STATUS_CODES chal_cam_set_lane_cntrl(CHAL_HANDLE handle, CHAL_CAM_LANE_CNTRL_st_t* cfg)
{
    CHAL_CAM_STATUS_CODES chal_status = CHAL_OP_OK;
    chal_cam_t *pCamDevice = (chal_cam_t *)handle;
    
    DBG_OUT( chal_dprintf( CDBG_INFO, "chal_cam_set_lane_cntrl: lane=0x%x enable=0x%x\n", cfg->lane, cfg->param) ); 

    
    // Analog Bias
        if (cfg->cntrl_state & CHAL_CAM_LANE_ANALOG_BIAS)
        {
        // Lane 0  Analog Bias
            if (cfg->lane & CHAL_CAM_DATA_LANE_0)
            {
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_DAT0,DLACN,cfg->param);
            }
        // Lane 1 Analog Bias
            if (cfg->lane & CHAL_CAM_DATA_LANE_1)
            {
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_DAT1,DLACN,cfg->param);
            }
        // Clock Analog Bias
            if (cfg->lane & CHAL_CAM_CLK_LANE)
            {
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_CLK,CLAC,cfg->param);
            }
        }
    // Power Down
        else if (cfg->cntrl_state & CHAL_CAM_LANE_PD)
        {
        // Lane 0 Power Down
            if (cfg->lane & CHAL_CAM_DATA_LANE_0)
            {
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_DAT0,DLPDN,cfg->param);
            }
        // Lane 1 Power Down
            if (cfg->lane & CHAL_CAM_DATA_LANE_1)
            {
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_DAT1,DLPDN,cfg->param);
            }
        // Clock Power Down
            if (cfg->lane & CHAL_CAM_CLK_LANE)
            {
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_CLK,CLPD,cfg->param);
            }
        }
    // Low Power Enable
        else if (cfg->cntrl_state & CHAL_CAM_LANE_LPEN)
        {
        // Lane 0 Low Power
            if (cfg->lane & CHAL_CAM_DATA_LANE_0)
            {
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_DAT0,DLLPEN,cfg->param);
            }
        // Lane 1 Low Power
            if (cfg->lane & CHAL_CAM_DATA_LANE_1)
            {
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_DAT1,DLLPEN,cfg->param);
            }
        // Clock Low Power
            if (cfg->lane & CHAL_CAM_CLK_LANE)
            {
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_CLK,CLLPE,cfg->param);
            }
        }
    // High Speed Enable
        else if (cfg->cntrl_state & CHAL_CAM_LANE_HS)
        {
        // Lane 0 High Speed
            if (cfg->lane & CHAL_CAM_DATA_LANE_0)
            {
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_DAT0,DLHSEN,cfg->param);
            }
        // Lane 1 High Speed
            if (cfg->lane & CHAL_CAM_DATA_LANE_1)
            {
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_DAT1,DLHSEN,cfg->param);
            }
        // Clock High Speed
            if (cfg->lane & CHAL_CAM_CLK_LANE)
            {
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_CLK,CLHSE,cfg->param);
            }
        }
    // Force Termination Enable
        else if (cfg->cntrl_state & CHAL_CAM_LANE_TERM_EN)
        {
        // Lane 0 Termination
            if (cfg->lane & CHAL_CAM_DATA_LANE_0)
            {
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_DAT0,DLTREN,cfg->param);
            }
        // Lane 1 Termination
            if (cfg->lane & CHAL_CAM_DATA_LANE_1)
            {
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_DAT1,DLTREN,cfg->param);
            }
        // Clock Termination
            if (cfg->lane & CHAL_CAM_CLK_LANE)
            {
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_CLK,CLTRE,cfg->param);
            }
        }
    // Exact Sync Matching Enable
        else if (cfg->cntrl_state & CHAL_CAM_LANE_SYNC_MATCHING)
        {
        // Lane 0 Sync Matching
            if (cfg->lane & CHAL_CAM_DATA_LANE_0)
            {
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_DAT0,DLSMN,cfg->param);
            }
        // Lane 1 Sync Matching
            if (cfg->lane & CHAL_CAM_DATA_LANE_1)
            {
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_DAT1,DLSMN,cfg->param);
            }
        }
    // High Speed Data Reception Delay Time
        else if (cfg->cntrl_state & CHAL_CAM_LANE_HS_RX_TIME)
        {
        // Lane 0/1 HS Rx
            if ( (cfg->lane & CHAL_CAM_DATA_LANE_0) || (cfg->lane & CHAL_CAM_DATA_LANE_1) )
            {
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_DLT,DLT3,cfg->param);
            }
        }
    // High Speed Settle Delay Time
        else if (cfg->cntrl_state & CHAL_CAM_LANE_HS_SETTLE_TIME)
        {
        // Lane 0/1 HS Settle
            if ( (cfg->lane & CHAL_CAM_DATA_LANE_0) || (cfg->lane & CHAL_CAM_DATA_LANE_1) )
            {
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_DLT,DLT2,cfg->param);
            }
            if (cfg->lane & CHAL_CAM_CLK_LANE)
            {
				BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,
						CAM_CLT, CLT2, cfg->param);
            }
        }
    // High Speed Settle Delay Time
        else if (cfg->cntrl_state & CHAL_CAM_LANE_HS_TERM_TIME)
        {
        // Lane 0/1 HS Termination
            if ( (cfg->lane & CHAL_CAM_DATA_LANE_0) || (cfg->lane & CHAL_CAM_DATA_LANE_1) )
            {
				BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,
						CAM_DLT, DLT1, cfg->param);
            }
            if (cfg->lane & CHAL_CAM_CLK_LANE)
            {
				BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,
						CAM_CLT, CLT1, cfg->param);
            }
        }

    return chal_status;
}    


//==============================================================================
//! \brief 
//!         Set CAM module Input Intf
//! \note 
//!         Parameters:   
//!             handle  (in) this CAM instance
//!             cfg     (in) configuration parameters to be set
//!         Return:
//!             CHAL_CAM_STATUS_CODES
//==============================================================================
CHAL_CAM_STATUS_CODES chal_cam_set_intf(CHAL_HANDLE handle, CHAL_CAM_CFG_INTF_st_t* cfg)
{
    CHAL_CAM_STATUS_CODES chal_status = CHAL_OP_OK;
    chal_cam_t *pCamDevice = (chal_cam_t *)handle;
    cUInt32 reg;

    DBG_OUT( chal_dprintf( CDBG_INFO, "chal_cam_set_intf\n") ); 

// Set Cam Interface
        if (cfg->intf == CHAL_CAM_INTF_CCP)
        {
        // Cam CCP Mode
            reg = BRCM_READ_REG(pCamDevice->baseAddr,CAM_CTL);
                reg &= ~(CAM_CTL_CPM_MASK | CAM_CTL_DCM_MASK | CAM_CTL_PFT_MASK);
                reg |= CAM_REG_FIELD_SET(CAM_CTL,CPM,1);
                reg |= CAM_REG_FIELD_SET(CAM_CTL,PFT,5);
            // Clk Mode
                if (cfg->clk_mode == CHAL_CAM_DATA_CLOCK)
                {
                    reg |= CAM_REG_FIELD_SET(CAM_CTL,DCM,1);
                }
            BRCM_WRITE_REG(pCamDevice->baseAddr,CAM_CTL,reg);
        // Cam CCP Image ID
            BRCM_WRITE_REG(pCamDevice->baseAddr,CAM_IDI,0x00000080);
        // Lane Timing
            BRCM_WRITE_REG(pCamDevice->baseAddr,CAM_CLT,( (6<<CAM_CLT_CLT1_SHIFT)|(6<<CAM_CLT_CLT2_SHIFT) ));
            BRCM_WRITE_REG(pCamDevice->baseAddr,CAM_DLT,( (6<<CAM_DLT_DLT1_SHIFT)|(6<<CAM_DLT_DLT2_SHIFT)|(0<<CAM_DLT_DLT3_SHIFT) ));
        }
        else
        {
        // CSI Mode Setup
            BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_CTL,CPM,0);
            BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_CTL,PFT,5);
        // Lane Timing
		BRCM_WRITE_REG(pCamDevice->baseAddr, CAM_CLT,
		((0x0<<CAM_CLT_CLT1_SHIFT)|(0x05<<CAM_CLT_CLT2_SHIFT)));
		BRCM_WRITE_REG(pCamDevice->baseAddr, CAM_DLT,
		((0x8<<CAM_DLT_DLT1_SHIFT)|(0<<CAM_DLT_DLT2_SHIFT)
		|(0<<CAM_DLT_DLT3_SHIFT)));
        }
    #ifndef __KERNEL__
        reg = MM_CFG_BASE_ADDR;
    #else
        reg = HW_IO_PHYS_TO_VIRT(MM_CFG_BASE_ADDR);
    #endif
        
    // Select Camera Phy AFE 
        if (cfg->afe_port == CHAL_CAM_PORT_AFE_1) 
        {
        // AFE 1 Select:  CSI0 has PHY selection.
            BRCM_WRITE_REG_FIELD(reg, MM_CFG_CSI0_PHY_CTRL, CSI_PHY_SEL, 1);
            BRCM_WRITE_REG(reg, MM_CFG_CSI1_LDO_CTL, 0x5A00000F);
        }
        else
        {
        // AFE 0 Select:  CSI0 has PHY selection.
            BRCM_WRITE_REG_FIELD(reg, MM_CFG_CSI0_PHY_CTRL, CSI_PHY_SEL, 0);
            BRCM_WRITE_REG(reg, MM_CFG_CSI0_LDO_CTL, 0x5A00000F);
        }

    // Cam Output Engine Timeout
        BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_CTL,OET,0xFF);
                    
    return chal_status;
}    

//==============================================================================
//! \brief 
//!         CAM module Configure Short Packet Capture
//! \note 
//!         Parameters:   
//!             handle      (in) this CAM instance
//!             cfg         (in) configuration parameters to be set
//!         Return:
//!             CHAL_CAM_STATUS_CODES
//==============================================================================
CHAL_CAM_STATUS_CODES chal_cam_cfg_short_pkt(CHAL_HANDLE handle, CHAL_CAM_PKT_CFG_st_t* cfg)
{
    CHAL_CAM_STATUS_CODES chal_status = CHAL_OP_OK;
    chal_cam_t *pCamDevice = (chal_cam_t *)handle;
    cUInt32 reg = 0;

    DBG_OUT( chal_dprintf( CDBG_INFO, "chal_cam_cfg_short_pkt: instance=%d \n",cfg->instance ) );   

    if (cfg->instance & CHAL_CAM_PKT_CMP_0)
    {
        reg = CAM_REG_FIELD_SET(CAM_CMP0,PCEN,cfg->cmp_enable);
        reg |= CAM_REG_FIELD_SET(CAM_CMP0,GIN,cfg->intr_enable);
        reg |= CAM_REG_FIELD_SET(CAM_CMP0,CPHN,cfg->capture_enable);
        reg |= CAM_REG_FIELD_SET(CAM_CMP0,PCVCN,cfg->data_id);
        BRCM_WRITE_REG(pCamDevice->baseAddr,CAM_CMP0,reg);
    }          

    if (cfg->instance & CHAL_CAM_PKT_CMP_1)
    {
        reg = CAM_REG_FIELD_SET(CAM_CMP1,PCEN,cfg->cmp_enable);
        reg |= CAM_REG_FIELD_SET(CAM_CMP1,GIN,cfg->intr_enable);
        reg |= CAM_REG_FIELD_SET(CAM_CMP1,CPHN,cfg->capture_enable);
        reg |= CAM_REG_FIELD_SET(CAM_CMP1,PCVCN,cfg->data_id);
        BRCM_WRITE_REG(pCamDevice->baseAddr,CAM_CMP1,reg);
    }
    return chal_status;
}    

//==============================================================================
//! \brief 
//!         CAM module Get Short Packet
//! \note 
//!         Parameters:   
//!             handle      (in) this CAM instance
//!             short_pkt   (out) packet struct to fill in
//!         Return:
//!             CHAL_CAM_STATUS_CODES
//==============================================================================
CHAL_CAM_STATUS_CODES chal_cam_get_short_pkt(CHAL_HANDLE handle, CHAL_CAM_PKT_st_t* short_pkt)
{
    CHAL_CAM_STATUS_CODES chal_status = CHAL_OP_OK;
    chal_cam_t *pCamDevice = (chal_cam_t *)handle;
    cUInt32 reg0, reg1;
//    cUInt32 data_size;

    DBG_OUT( chal_dprintf( CDBG_INFO, "chal_cam_get_short_pkt:\n") );   

    reg0 = BRCM_READ_REG(pCamDevice->baseAddr,CAM_CAP0);
    reg1 = BRCM_READ_REG(pCamDevice->baseAddr,CAM_CAP1);
    if (reg0 & CAM_CAP0_CPHV_MASK)
    {
        *short_pkt->data = 0;
        short_pkt->data_count = CAM_REG_FIELD_GET(CAM_CAP0,CWCN,reg0);
#if 0
        data_size = CAM_REG_FIELD_GET(CAM_CAP0,CDTN,reg0); 
        switch (data_size)
        {
            case xxxx:  
                short_pkt->data_size = yyyy;   
                break;
            default:
                break;
       {
#endif                
        short_pkt->virt_chan = CAM_REG_FIELD_GET(CAM_CAP0,CVCN,reg0);   
        short_pkt->ecc = CAM_REG_FIELD_GET(CAM_CAP0,CECCN,reg0);   
        BRCM_WRITE_REG(pCamDevice->baseAddr,CAM_CAP0,reg0);
                
    }
    else if (reg1 & CAM_CAP1_CPHV_MASK)
    {
        *short_pkt->data = 0;
        short_pkt->data_count = CAM_REG_FIELD_GET(CAM_CAP1,CWCN,reg1);   
#if 0
        data_size = CAM_REG_FIELD_GET(CAM_CAP1,CDTN,reg1); 
        switch (data_size)
        {
            case xxxx:  
                short_pkt->data_size = yyyy;   
                break;
            default:
                break;
       {
#endif                
        short_pkt->virt_chan = CAM_REG_FIELD_GET(CAM_CAP1,CVCN,reg1);   
        short_pkt->ecc = CAM_REG_FIELD_GET(CAM_CAP1,CECCN,reg1);   
        BRCM_WRITE_REG(pCamDevice->baseAddr,CAM_CAP1,reg1);
    }
    else
    {
        chal_status = CHAL_OP_FAILED;
        chal_dprintf( CDBG_INFO, "chal_cam_get_short_pkt: FAILED: No Packet Available\n");  
    }
    return chal_status;
}    

//==============================================================================
//! \brief 
//!         CAM module Get Receiver status
//! \note 
//!         Parameters:   
//!             handle          (in) this CAM instance
//!             param.intf      (in) interface select       (unused)
//!             param.chan      (in) channel select         (unused)
//!             param.lane      (in) lane select            (unused)
//!             param.select    (in) cfg select             (unused)
//!             param.virt_chan (in) virtual channel select (unused)
//!             param.param     (in) OR'd Interrupt and Error Status's for receiver
//!         Return:
//!             CHAL_CAM_STATUS_CODES
//==============================================================================
cUInt32 chal_cam_get_rx_status(CHAL_HANDLE handle, CHAL_CAM_PARAM_st_t* param)
{
    chal_cam_t *pCamDevice = (chal_cam_t *)handle;
    cUInt32 reg;
    
// Get Status Register
    reg = BRCM_READ_REG(pCamDevice->baseAddr,CAM_STA);
// Clear status          
    BRCM_WRITE_REG(pCamDevice->baseAddr,CAM_STA,reg);
    param->param = 0;
    
    param->param = chalCamRawRxStatus( handle, reg );
     
// CRC Error
    if ( reg & CAM_STA_CRCE_MASK)
    {
    // Hera CCP2 Interface always gives CRC ERROR
        param->param |= CHAL_CAM_RX_CRC_ERROR;
    }
// Output Busy
    if ( reg & CAM_STA_OES_MASK)
    {
        param->param |= CHAL_CAM_RX_BUSY;
    }
    
// HS Clock Present
    if ( reg & CAM_STA_CS_MASK)
    {
        param->param |= CHAL_CAM_RX_CLK_PRESENT;
    }
    return reg;
}    

/***********************************************************
 * Name: chalCamRawIntrStatus
 * 
 * Arguments: 
 *       UInt32 rx status register - raw Rx Status 
 *
 * Description: Routine used to get Receiver status
 *
 * Returns: chal rx status
 *
 ***********************************************************/
UInt32 chalCamRawIntrStatus( CHAL_HANDLE handle, UInt32 reg )
{
    UInt32 status = 0;
    (void)handle;
    
        if (reg & CAM_ISTA_FSI_MASK)
        {
            status |= CHAL_CAM_INT_FRAME_START;
        }
        if (reg & CAM_ISTA_FEI_MASK)
        {
            status |= CHAL_CAM_INT_FRAME_END;
        }
         if (reg & CAM_ISTA_LCI_MASK)
        {
            status |= CHAL_CAM_INT_LINE_COUNT;
        }
    return status;
}    

/***********************************************************
 * Name: chalCamRawRxStatus
 * 
 * Arguments: 
 *       UInt32 rx status register - raw Rx Status 
 *
 * Description: Routine used to get Receiver status
 *
 * Returns: chal rx status
 *
 ***********************************************************/
UInt32 chalCamRawRxStatus( CHAL_HANDLE handle, UInt32 reg )
{
    UInt32 rx_status = 0;
    (void)handle;
    
 // Packet 1
    if (reg & CAM_STA_PI1_MASK)
    {
        rx_status |= CHAL_CAM_RX_INT_PKT_1;
    }
// Packet 0 
    if (reg & CAM_STA_PI0_MASK)
    {
        rx_status |= CHAL_CAM_RX_INT_PKT_0;
    }
// Interrupt Pending
    if (reg & CAM_STA_IS_MASK)
    {
        rx_status |= CHAL_CAM_RX_INT_CHAN_0;
    }
// Packet Error
    //*********************************************************************************    
    // On Hera Data will be lost every Frame due to DBOB not being set in DCS register (Data Underflows)
    //*********************************************************************************    
        if ( reg & (CAM_STA_DL_MASK | CAM_STA_PLE_MASK | CAM_STA_SBE_MASK | CAM_STA_HOE_MASK) )
        {
            rx_status |= CHAL_CAM_RX_PKT_ERROR;
        }
// Fifo Error
    if ( reg & (CAM_STA_OFO_MASK | CAM_STA_IFO_MASK | CAM_STA_BFO_MASK) )
    {
        rx_status |= CHAL_CAM_RX_FIFO_ERROR;
    }
// Panic Error
    if (reg & CAM_STA_PS_MASK)
    {
        rx_status |= CHAL_CAM_RX_PANIC;
    }
// Parity Error
    if ( reg & CAM_STA_PBE_MASK)
    {
        rx_status |= CHAL_CAM_RX_PARITY_ERROR;
    }
// CRC Error
    if ( reg & CAM_STA_CRCE_MASK)
    {
        rx_status |= CHAL_CAM_RX_CRC_ERROR;
    }
// Output Busy
    if ( reg & CAM_STA_OES_MASK)
    {
        rx_status |= CHAL_CAM_RX_BUSY;
    }
    
// HS Clock Present
    if ( reg & CAM_STA_CS_MASK)
    {
        rx_status |= CHAL_CAM_RX_CLK_PRESENT;
    }

// Buffer Status 
    if ( reg & CAM_STA_BUF0_RDY_MASK)
    {
        rx_status |= CHAL_CAM_RX_BUF0_RDY;
    }
    if ( reg & CAM_STA_BUF0_NO_MASK)
    {
        rx_status |= CHAL_CAM_RX_BUF0_NO;
    }
    if ( reg & CAM_STA_BUF1_RDY_MASK)
    {
        rx_status |= CHAL_CAM_RX_BUF1_RDY;
    }
    if ( reg & CAM_STA_BUF1_NO_MASK)
    {
        rx_status |= CHAL_CAM_RX_BUF1_NO;
    }

    return rx_status;
}    

//==============================================================================
//! \brief 
//!         CAM module Get Receiver Lane status
//! \note 
//!         Parameters:   
//!             handle          (in) this CAM instance
//!             param.intf      (in) interface select
//!             param.chan      (in) unused
//!             param.lane      (in) lane select: CLK, Lane 1-2
//!             param.cntrl_state (in) unused
//!             param.param     (in) OR'd Lane Status's for receiver
//!         Return:
//!             CHAL_CAM_STATUS_CODES
//==============================================================================
CHAL_CAM_STATUS_CODES chal_cam_get_lane_status(CHAL_HANDLE handle, CHAL_CAM_PARAM_st_t* param)
{
    chal_cam_t *pCamDevice = (chal_cam_t *)handle;
    cUInt32 reg;
    CHAL_CAM_STATUS_CODES chal_status = CHAL_OP_OK;
    
    DBG_OUT( chal_dprintf( CDBG_INFO, "chal_cam_get_lane_status:  0x%x\n", param->lane) );  

// Get Status Register       
    param->param = 0;
    if (param->lane == CHAL_CAM_CLK_LANE)
    {
    // Check Clock error bits   
        reg = BRCM_READ_REG(pCamDevice->baseAddr,CAM_CLK);
    // Clear status          
        BRCM_WRITE_REG(pCamDevice->baseAddr,CAM_CLK,reg);
        DBG_OUT( chal_dprintf( CDBG_INFO, "chal_cam_get_lane_status:  CAM_CLK=0x%x\n", reg) );  

    // Enabled
        if (reg & CAM_CLK_CLE_MASK)
        {
            param->param |= CHAL_CAM_LANE_EN;
        }
    // Power Down
        if (reg & CAM_CLK_CLPD_MASK)
        {
            param->param |= CHAL_CAM_LANE_PD;
        }
    // Lane state error bits    
        if ( (reg & CAM_CLK_CLSTE_MASK) || ( (CAM_REG_FIELD_GET(CAM_CLK,CLS,reg)) == 13) )
        {
            param->param |= CHAL_CAM_LANE_STATE_ERROR;
        }
    // LP
        if ( (CAM_REG_FIELD_GET(CAM_CLK,CLS,reg)) == 4 )
        {
            param->param |= CHAL_CAM_LANE_ULP;
        }
    // HS
        if ( (CAM_REG_FIELD_GET(CAM_CLK,CLS,reg)) == 6 )
        {
            param->param |= CHAL_CAM_LANE_HS;
        }

    }
    else if (param->lane == CHAL_CAM_DATA_LANE_0)
    {
        reg = BRCM_READ_REG(pCamDevice->baseAddr,CAM_DAT0);
    // Clear status          
        BRCM_WRITE_REG(pCamDevice->baseAddr,CAM_DAT0,reg);
        DBG_OUT( chal_dprintf( CDBG_INFO, "chal_cam_get_lane_status:  CAM_DAT0=0x%x\n", reg) ); 

    // Enabled
        if (reg & CAM_DAT0_DLEN_MASK)
        {
            param->param |= CHAL_CAM_LANE_EN;
        }
    // Power Down
        if (reg & CAM_DAT0_DLPDN_MASK)
        {
            param->param |= CHAL_CAM_LANE_PD;
        }
    // Fifo error
        if (reg & CAM_DAT0_DLFON_MASK)
        {
            param->param |= CHAL_CAM_LANE_FF_ERROR;
        }
    // Lane state error bits    
        if ( (reg & CAM_DAT0_DLSTEN_MASK) || ( (CAM_REG_FIELD_GET(CAM_DAT0,DLSN,reg)) == 10) )
        {
            param->param |= CHAL_CAM_LANE_STATE_ERROR;
        }
    // Sync error
        if (reg & CAM_DAT0_DLSEN_MASK)
        {
            param->param |= CHAL_CAM_LANE_SYNC_MATCHING;
        }
    // LP
        if ( (CAM_REG_FIELD_GET(CAM_DAT0,DLSN,reg)) == 15)
        {
            param->param |= CHAL_CAM_LANE_ULP;
        }
    // HS
        if ( (CAM_REG_FIELD_GET(CAM_DAT0,DLSN,reg)) == 6)
        {
            param->param |= CHAL_CAM_LANE_HS;
        }
    }
    else if (param->lane == CHAL_CAM_DATA_LANE_1)
    {
    // Check Data error bits    
        reg = BRCM_READ_REG(pCamDevice->baseAddr,CAM_DAT1);
    // Clear status          
        BRCM_WRITE_REG(pCamDevice->baseAddr,CAM_DAT1,reg);
        DBG_OUT( chal_dprintf( CDBG_INFO, "chal_cam_get_lane_status:  CAM_DAT1=0x%x\n", reg) ); 

    // Enabled
        if (reg & CAM_DAT1_DLEN_MASK)
        {
            param->param |= CHAL_CAM_LANE_EN;
        }
    // Power Down
        if (reg & CAM_DAT1_DLPDN_MASK)
        {
            param->param |= CHAL_CAM_LANE_PD;
        }
    // Fifo error
        if (reg & CAM_DAT1_DLFON_MASK)
        {
            param->param |= CHAL_CAM_LANE_FF_ERROR;
        }
    // Lane state error bits    
        if ( (reg & CAM_DAT1_DLSTEN_MASK) || ( (CAM_REG_FIELD_GET(CAM_DAT1,DLSN,reg)) == 10) )
        {
            param->param |= CHAL_CAM_LANE_STATE_ERROR;
        }
    // Sync error
        if (reg & CAM_DAT1_DLSEN_MASK)
        {
            param->param |= CHAL_CAM_LANE_SYNC_MATCHING;
        }
    // LP
        if ( (CAM_REG_FIELD_GET(CAM_DAT1,DLSN,reg)) == 15)
        {
            param->param |= CHAL_CAM_LANE_ULP;
        }
    // HS
        if ( (CAM_REG_FIELD_GET(CAM_DAT1,DLSN,reg)) == 6)
        {
            param->param |= CHAL_CAM_LANE_HS;
        }
    }
    else
    {
        chal_status |= CHAL_OP_INVALID_PARAMETER;
    }
    return chal_status;
}    

//==============================================================================
//! \brief 
//!         CAM module Get Receiver Channel status
//! \note 
//!         Parameters:   
//!             handle          (in) this CAM instance
//!             param.intf      (in) interface select       (unused)
//!             param.chan      (in) channel select         (unused)
//!             param.lane      (in) lane select            (unused)
//!             param.select    (in) cfg select             (unused)
//!             param.virt_chan (in) virtual channel select (unused)
//!             param.param     (in) OR'd channel interrupt status, interrupts then cleared
//!         Return:
//!             CHAL_CAM_STATUS_CODES
//==============================================================================
cUInt32 chal_cam_get_chan_status(CHAL_HANDLE handle, CHAL_CAM_PARAM_st_t* param)
{
    chal_cam_t *pCamDevice = (chal_cam_t *)handle;
    cUInt32 reg;
    
        param->param = 0;
    // Check Data Status bits   
        reg = BRCM_READ_REG(pCamDevice->baseAddr,CAM_DCS);
    // Clear status          
        BRCM_WRITE_REG(pCamDevice->baseAddr,CAM_DCS,reg);
        //DBG_OUT( chal_dprintf( CDBG_INFO, "chal_cam_get_chan_status:  CAM_DCS=0x%x\n", reg) );    
            if (reg & CAM_DCS_DI_MASK)
            {
                if (reg & CAM_DCS_DIM_MASK)
                {
                    param->param |= CHAL_CAM_INT_DATA_END;
                }
                else
                {
                    param->param |= CHAL_CAM_INT_DATA_FRAME_END;
                }
            }
    // Check Packet Status bits 
        reg = BRCM_READ_REG(pCamDevice->baseAddr,CAM_CAP0);
            if (reg & CAM_CAP0_CPHV_MASK)
            {
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_CAP0,CPHV,1);
                param->param |= CHAL_CAM_INT_PKT;
                DBG_OUT( chal_dprintf( CDBG_INFO, "chal_cam_get_chan_status:  CAM_CAP0=0x%x\n", reg) ); 
            }
        reg = BRCM_READ_REG(pCamDevice->baseAddr,CAM_CAP1);
            if (reg & CAM_CAP1_CPHV_MASK)
            {
                BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_CAP1,CPHV,1);
                param->param |= CHAL_CAM_INT_PKT;
                DBG_OUT( chal_dprintf( CDBG_INFO, "chal_cam_get_chan_status:  CAM_CAP1=0x%x\n", reg) ); 
            }
    // Check Image Status bits  
        reg = BRCM_READ_REG(pCamDevice->baseAddr,CAM_ISTA);
    // Clear status          
        BRCM_WRITE_REG(pCamDevice->baseAddr,CAM_ISTA,reg);
        //DBG_OUT( chal_dprintf( CDBG_INFO, "chal_cam_get_chan_status:  CAM_ISTA=0x%x\n", reg) );   
    // Update Intr status
        param->param |= chalCamRawIntrStatus( handle, reg );

    return reg;
}    

//==============================================================================
//! \brief 
//!         CAM module Clear Channel Status
//! \note 
//!       NOT USED, status is cleared when read
//!         Parameters:   
//!             handle          (in) this CAM instance
//!             param.intf      (in) interface select       (unused)
//!             param.chan      (in) channel select         (unused)
//!             param.lane      (in) lane select            (unused)
//!             param.select    (in) cfg select             (unused)
//!             param.virt_chan (in) virtual channel select (unused)
//!             param.param     (in) OR'd channel interrupts to clear
//!         Return:
//!             CHAL_CAM_STATUS_CODES
//==============================================================================
CHAL_CAM_STATUS_CODES chal_cam_clear_chan_status(CHAL_HANDLE handle, CHAL_CAM_PARAM_st_t* param)
{
    chal_cam_t *pCamDevice = (chal_cam_t *)handle;
    CHAL_CAM_STATUS_CODES chal_status = CHAL_OP_OK;
    cUInt32 reg = 0;

    DBG_OUT( chal_dprintf( CDBG_INFO, "chal_cam_clear_chan_status:  \n") ); 
// Image status bits
    if (param->param & CHAL_CAM_INT_FRAME_START)
    {
        reg |= CAM_ISTA_FSI_MASK;
    }
    if (param->param & CHAL_CAM_INT_FRAME_END)
    {
        reg |= CAM_ISTA_FEI_MASK;
    }
    if (param->param & CHAL_CAM_INT_LINE_COUNT)
    {
        reg |= CAM_ISTA_LCI_MASK;
    }
    BRCM_WRITE_REG(pCamDevice->baseAddr,CAM_ISTA,reg);
// Data status bits
    reg = BRCM_READ_REG(pCamDevice->baseAddr,CAM_DCS);
    if ( (param->param & CHAL_CAM_INT_DATA_END) || (param->param & CHAL_CAM_INT_DATA_FRAME_END) )
    {
        reg |= CAM_DCS_DI_MASK;
        BRCM_WRITE_REG(pCamDevice->baseAddr,CAM_DCS,reg);
    }
// Packet status bits
    reg = BRCM_READ_REG(pCamDevice->baseAddr,CAM_DCS);
    if (param->param & CHAL_CAM_INT_PKT)
    {
        reg = BRCM_READ_REG(pCamDevice->baseAddr,CAM_CAP0);
        reg |= CAM_CAP0_CPHV_MASK;
        BRCM_WRITE_REG(pCamDevice->baseAddr,CAM_CAP0,reg);

        reg = BRCM_READ_REG(pCamDevice->baseAddr,CAM_CAP1);
        reg |= CAM_CAP1_CPHV_MASK;
        BRCM_WRITE_REG(pCamDevice->baseAddr,CAM_CAP1,reg);
    }
    return chal_status;
}    

//==============================================================================
//! \brief 
//!         CAM module Receiver Reset
//! \note 
//!         Parameters:   
//!             handle          (in) this CAM instance
//!             param.intf      (in) interface select       (unused)
//!             param.chan      (in) channel select         (unused)
//!             param.lane      (in) lane select            (unused)
//!             param.select    (in) cfg select             (unused)
//!             param.virt_chan (in) virtual channel select (unused)
//!             param.param     (in) 0=OFF  1=ON
//!         Return:
//!             CHAL_CAM_STATUS_CODES
//==============================================================================
CHAL_CAM_STATUS_CODES chal_cam_reset(CHAL_HANDLE handle, CHAL_CAM_PARAM_st_t* param)
{
    CHAL_CAM_STATUS_CODES chal_status = CHAL_OP_OK;
    chal_cam_t *pCamDevice = (chal_cam_t *)handle;

    DBG_OUT( chal_dprintf( CDBG_INFO, "chal_cam_reset:  param=0x%x\n", param->param) );
    
// Software Reset
    if (param->param & CHAL_CAM_RESET_RX)
    {
        BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_CTL,CPR,1);
        BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_CTL,CPR,0);
    }
// Analog Reset
    if (param->param & CHAL_CAM_RESET_ARST)
    {
        BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_ANA,AR,1);
        BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_ANA,AR,0);
    }
// Image Pointers
    if (param->param & CHAL_CAM_RESET_IMAGE)
    {
        BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_ICTL,LIP,1);
    }
// Data Pointers
    if (param->param & CHAL_CAM_RESET_DATA)
    {
        BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_DCS,LDP,1);
    }
    return chal_status;
}    


//==============================================================================
//! \brief 
//!         CAM module Enable Frame Trigger
//! \note 
//!             handle          (in) this CAM instance
//!             param.intf      (in) interface select       (unused)
//!             param.chan      (in) channel select         (unused)
//!             param.lane      (in) lane select            (unused)
//!             param.select    (in) cfg select             (unused)
//!             param.virt_chan (in) virtual channel select (unused)
//!             param.param     (in) 0=OFF  1=ON
//!         Return:
//!             CHAL_CAM_STATUS_CODES
//==============================================================================
CHAL_CAM_STATUS_CODES chal_cam_channel_trigger(CHAL_HANDLE handle, CHAL_CAM_PARAM_st_t* param)
{
    CHAL_CAM_STATUS_CODES chal_status = CHAL_OP_OK;
    chal_cam_t *pCamDevice = (chal_cam_t *)handle;
    
    DBG_OUT( chal_dprintf( CDBG_INFO, "chal_cam_channel_trigger: \n") );    
    BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_ICTL,FCM,1);
    BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_ICTL,TFC,1);
    return chal_status;
}


//==============================================================================
//! \brief 
//!         CAM module Receiver Start
//! \note 
//!             handle          (in) this CAM instance
//!             param.intf      (in) interface select       (unused)
//!             param.chan      (in) channel select         (unused)
//!             param.lane      (in) lane select            (unused)
//!             param.select    (in) cfg select             (unused)
//!             param.virt_chan (in) virtual channel select (unused)
//!             param.param     (in) 0=OFF  1=ON
//!         Return:
//!             CHAL_CAM_STATUS_CODES
//==============================================================================
CHAL_CAM_STATUS_CODES chal_cam_rx_start(CHAL_HANDLE handle, CHAL_CAM_PARAM_st_t* param)
{
    CHAL_CAM_STATUS_CODES chal_status = CHAL_OP_OK;
    chal_cam_t *pCamDevice = (chal_cam_t *)handle;
    CHAL_CAM_PARAM_st_t chal_cam_param_st;
    cUInt32 dbctl;
    
    DBG_OUT( chal_dprintf( CDBG_INFO, "chal_cam_rx_start:  chan=0x%x lane=0x%x\n", param->chan, param->lane) );

// Rhea A0, Need to disable DDR PLL PWRDN mode to prevent data errors when capturing camera data
    #ifdef CSR_DDR_PLL_REG_UNSET_FLAG
        RegCsrDdrPllPwrdnBit = BRCM_READ_REG_FIELD(MEMC0_OPEN_BASE_ADDR,CSR_HW_FREQ_CHANGE_CNTRL,DDR_PLL_PWRDN_ENABLE);
        BRCM_WRITE_REG_FIELD(MEMC0_OPEN_BASE_ADDR,CSR_HW_FREQ_CHANGE_CNTRL,DDR_PLL_PWRDN_ENABLE,0);
    #endif    
    

// Disable Data Lane
    BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_ANA,DDL,1);
// Power Up Analog
    BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_ANA,APD,0);
    BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_ANA,BPD,0);
// Enable Clock Lane
    BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_CLK,CLPD,0);
    BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_CLK,CLE,1);

// Analog in Reset
    BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_ANA,AR,1);
// Enable Camera Interface
    BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_CTL,CPE,1);
// Reset Camera Interface, saving/restoring registers that get cleared
    dbctl = BRCM_READ_REG(pCamDevice->baseAddr,CAM_DBCTL);
    chal_cam_param_st.param = CHAL_CAM_RESET_RX;
    chal_status |= chal_cam_reset(handle, &chal_cam_param_st);
    BRCM_WRITE_REG(pCamDevice->baseAddr,CAM_DBCTL,dbctl);
// Update CFG Control registers    
    chal_status |= chal_cam_cfg_cntrl(handle, &pCamDevice->cfg_cntrl_st);
// Load Image Addr Pointers
    BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_ICTL,LIP,1);
// Load Data Addr Pointers
    BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_DCS,LDP,1);
// Analog out of Reset
    BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_ANA,AR,0);
// Enable Output Engine
    BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_CTL,SOE,0);
// Enable Data Lane 
    BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_ANA,DDL,0);

    return chal_status;
}    

//==============================================================================
//! \brief 
//!         CAM module Receiver Stop
//! \note 
//!         Parameters:   
//!             handle          (in) this CAM instance
//!             param.intf      (in) interface select       (unused)
//!             param.chan      (in) channel select         (unused)
//!             param.lane      (in) lane select            (unused)
//!             param.select    (in) cfg select             (unused)
//!             param.virt_chan (in) virtual channel select (unused)
//!             param.param     (in) 0=OFF  1=ON
//!         Return:
//!             CHAL_CAM_STATUS_CODES
//==============================================================================
CHAL_CAM_STATUS_CODES chal_cam_rx_stop(CHAL_HANDLE handle, CHAL_CAM_PARAM_st_t* param)
{
    CHAL_CAM_STATUS_CODES chal_status = CHAL_OP_OK;
    chal_cam_t *pCamDevice = (chal_cam_t *)handle;
    CHAL_CAM_PARAM_st_t chal_cam_param_st;
    cUInt32 timeout;
    
    DBG_OUT( chal_dprintf( CDBG_INFO, "chal_cam_rx_stop:   \n") );

    timeout = MIN_FRAME_RATE;
// Disable Data Lanes Analog 
    BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_ANA,DDL,1);
    
// Shutdown Output Engine
    BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_CTL,SOE,1);
// Wait for Output Engine idle
    while ( (timeout != 0) && (BRCM_READ_REG_FIELD(pCamDevice->baseAddr,CAM_STA,OES) == 1) )
    {
        CHAL_DELAY_MS(1);        
        timeout--;
    } 
// Re-enable Output Engine
    BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_CTL,SOE,0);
    if (timeout == 0)
    {
        chal_dprintf(CDBG_ERRO, "ERROR: chal_cam_rx_stop(): CAM_STA_OEB != 0 \r\n");
    }
// Reset Camera Interface
    chal_cam_param_st.param = CHAL_CAM_RESET_RX;
    chal_status |= chal_cam_reset(handle, &chal_cam_param_st);
// Disable Camera Interface
    BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_CTL,CPE,0);
// Analog in Reset
    chal_cam_param_st.param = CHAL_CAM_RESET_ARST;
    chal_status |= chal_cam_reset(handle, &chal_cam_param_st);
// Disable Clock Lanes
    BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_CLK,CLE,0);
    BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_CLK,CLPD,1);
// Power Down Analog
    BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_ANA,APD,1);
    BRCM_WRITE_REG_FIELD(pCamDevice->baseAddr,CAM_ANA,BPD,1);
    
// Rhea A0, Need to disable DDR PLL PWRDN mode to prevent data errors when capturing camera data
    #ifdef CSR_DDR_PLL_REG_UNSET_FLAG
        if (RegCsrDdrPllPwrdnBit != CSR_DDR_PLL_REG_UNSET_FLAG)
        {
            BRCM_WRITE_REG_FIELD(MEMC0_OPEN_BASE_ADDR,CSR_HW_FREQ_CHANGE_CNTRL,DDR_PLL_PWRDN_ENABLE,RegCsrDdrPllPwrdnBit);
            RegCsrDdrPllPwrdnBit = CSR_DDR_PLL_REG_UNSET_FLAG;
        }
    #endif    
    
    
    return chal_status;
}    
    

//==============================================================================
//! \brief 
//!         CAM module Bytes Written
//! \note 
//!         Parameters:   
//!             handle          (in) this CAM instance
//!             param.intf      (in) interface select       (unused)
//!             param.chan      (in) channel select         (unused)
//!             param.lane      (in) lane select            (unused)
//!             param.select    (in) cfg select             (unused)
//!             param.virt_chan (in) virtual channel select (unused)
//!             param.param     (out) bytes written
//!         Return:
//!             CHAL_CAM_STATUS_CODES
//==============================================================================
CHAL_CAM_STATUS_CODES chal_cam_rx_bytes_written(CHAL_HANDLE handle, CHAL_CAM_PARAM_st_t* param)
{
    chal_cam_t *pCamDevice = (chal_cam_t *)handle;
    cUInt32 pixels, lines;
    CHAL_CAM_STATUS_CODES chal_status = CHAL_OP_OK;

// # pixels/line    
    pixels = BRCM_READ_REG_FIELD(pCamDevice->baseAddr,CAM_IHSTA,PPL);
    DBG_OUT( chal_dprintf( CDBG_INFO, "chal_cam_rx_bytes_written: pixels/line=%d \n", pixels) );
    lines = BRCM_READ_REG_FIELD(pCamDevice->baseAddr,CAM_IVSTA,LPF);
    DBG_OUT( chal_dprintf( CDBG_INFO, "chal_cam_rx_bytes_written: lines=%d \n", lines) );
    param->param = pixels * lines;
    return chal_status;
}    


/*****************************************************************************
 * NAME:    chal_cam_clock
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
CHAL_CAM_STATUS_CODES chal_cam_clock(CHAL_HANDLE handle, cUInt32 clk_select, cUInt32 divider, cBool enable)
{
	cUInt32 base_addr;

	DBG_OUT(chal_dprintf(CDBG_INFO, "%s():\n", __func__));

	if (enable == 1) {
		/* Enable Clock Select 1*/
		if (clk_select == 1) {
			/* Select GPIO32 to be DCLK2 (bits 10:8 = 0x300 => DCLK2
			 * bits 2:0 = 3 => 8 mAmps strength*/
			base_addr = HW_IO_PHYS_TO_VIRT(PAD_CTRL_BASE_ADDR);
			BRCM_WRITE_REG_FIELD(base_addr, PADCTRLREG_GPIO32,
				PINSEL_GPIO32, 3);
		} else {
			/* Select DCLK1 (bits 10:8 = 0x000 => DCLK1 ,
			 * bits 2:0 = 3 => 8 mAmps strength*/
			base_addr = HW_IO_PHYS_TO_VIRT(PAD_CTRL_BASE_ADDR);
			BRCM_WRITE_REG_FIELD(base_addr , PADCTRLREG_DCLK1,
				PINSEL_DCLK1, 0);
		}
	}

	return CHAL_OP_OK;
}            
