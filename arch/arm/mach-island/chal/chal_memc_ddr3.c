/*****************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/

/*
 * ===========================================================================
 *  inlcude header file declarations
 */

#include <plat/chal/chal_common.h>
//#include <plat/chal/chal_reg.h>
#include <mach/chipregHw_inline.h>
#include <chal/chal_memc.h>
#include <chal/chal_memc_ddr3.h>
#include <chal/bcmregmem.h>
#include <linux/string.h>
#include <linux/math64.h>

#include <mach/rdb/brcm_rdb_csr.h>
#include <mach/rdb/brcm_rdb_sec.h>
#include <mach/rdb/brcm_rdb_ddr3ctl.h>
#include <mach/rdb/brcm_rdb_ddr40_phy_addr_ctl.h>

/*
 * ===========================================================================
 *  global variable declarations
 * 
 */

/*
 * ===========================================================================
 *  extern variable declarations
 * 
 */

/*
 * ===========================================================================
 *  static function prototype declarations
 * 
 */
static BCM_ERR_CODE ddr3_clock_configuration(CHAL_MEMC_HANDLE handle);
static BCM_ERR_CODE ddr3_strap_config(CHAL_MEMC_HANDLE handle);
static BCM_ERR_CODE ddr3_auto_calibrate(CHAL_MEMC_HANDLE handle);
static BCM_ERR_CODE ddr3_mode_reg_write (CHAL_MEMC_HANDLE handle, MEMC_CS_CONNECTION cs, uint32_t addr, uint32_t val);
static BCM_ERR_CODE ddr3_config_mode_registers(CHAL_MEMC_HANDLE handle);
static BCM_ERR_CODE ddr3_config_dram(CHAL_MEMC_HANDLE handle);

/*
 * ===========================================================================
 *  local macro declarations
 * 
 */
#if 0
    #define MEMC_DBG_OUT(fmt...)    BCM_DBG_OUT((fmt))
#elif 0 && defined(CONFIG_CSP_MIN_BUILD) 
    #include <csp/stdio.h>
    #define MEMC_DBG_OUT(fmt...)	printf(fmt)
#else
    #define MEMC_DBG_OUT(fmt...)    do{}while(0)
#endif

#define CHAL_DELAY_MICROS(cnt) { udelay(cnt); }

#define DEBUG_REG_WRITE   0

#if DEBUG_REG_WRITE
#undef  CHAL_REG_WRITE32
#define CHAL_REG_WRITE32(addr, val) {\
    BCM_DBG_OUT(("Addr 0x%08x, Val 0x%08x\n", addr, val)); \
    (*((volatile unsigned int *)(addr)) = (val)); \
}

#undef CHAL_REG_CLRBIT32
#define CHAL_REG_CLRBIT32(addr, bits)  (CHAL_REG_WRITE32(addr, (CHAL_REG_READ32(addr) & (~(bits)))))

#undef CHAL_REG_SETBIT32
#define CHAL_REG_SETBIT32(addr, bits)  (CHAL_REG_WRITE32(addr, (CHAL_REG_READ32(addr) | (bits))))
#endif
#define BCM_DBG_ERR(...)

/* On the FPGA, there exists no PLL, DDR clock is configured for 13.5 MHz*/
#define DDR3_CLOCK_SPEED_FPGA  13500000

/*
 * ===========================================================================
 *  static variables declarations
 * 
 */
typedef struct JEDEC_INFO {
  uint32_t mhz;  // command bus speed in mhz
  uint32_t cl;   // read CAS lat
  uint32_t cwl;  // write CAS lat
  uint32_t wr;   // write recovery (min row active time)
  char *name;
} JEDEC_INFO_T;

JEDEC_INFO_T jedec_info[] = {
  /* MHz     CL     CWL     WR */
  {  266  ,    3  ,  0  ,   4 , "DDR2-533B"     },  /* 0   DDR2-533B  */
  {  266  ,    4  ,  0  ,   4 , "DDR2-533C"     },  /* 1   DDR2-533C  */
  {  333  ,    4  ,  0  ,   5 , "DDR2-667C"     },  /* 2   DDR2-667C  */
  {  333  ,    5  ,  0  ,   5 , "DDR2-667D"     },  /* 3   DDR2-667D  */
  {  400  ,    4  ,  0  ,   6 , "DDR2-800C"     },  /* 4   DDR2-800C  */
  {  400  ,    5  ,  0  ,   6 , "DDR2-800D"     },  /* 5   DDR2-800D  */
  {  400  ,    6  ,  0  ,   6 , "DDR2-800E"     },  /* 6   DDR2-800E  */
  {  533  ,    6  ,  0  ,   8 , "DDR2-1066E"    },  /* 7   DDR2-1066E */
  {  533  ,    7  ,  0  ,   8 , "DDR2-1066F"    },  /* 8   DDR2-1066F */
  {  400  ,    5  ,  5  ,   6 , "DDR3-800D"     },  /* 9   DDR3-800D  */
  {  400  ,    6  ,  5  ,   6 , "DDR3-800E"     },  /* 10  DDR3-800E  */
  {  533  ,    6  ,  6  ,   8 , "DDR3-1066E"    },  /* 11  DDR3-1066E */
  {  533  ,    7  ,  6  ,   8 , "DDR3-1066F"    },  /* 12  DDR3-1066F */
  {  533  ,    8  ,  6  ,   8 , "DDR3-1066G"    },  /* 13  DDR3-1066G */
  {  667  ,    7  ,  7  ,  10 , "DDR3-1333F"    },  /* 14  DDR3-1333F */
  {  667  ,    8  ,  7  ,  10 , "DDR3-1333G"    },  /* 15  DDR3-1333G */
  {  667  ,    9  ,  7  ,  10 , "DDR3-1333H"    },  /* 16  DDR3-1333H */
  {  667  ,   10  ,  7  ,  10 , "DDR3-1333J"    },  /* 17  DDR3-1333J */
  {  800  ,    8  ,  8  ,  12 , "DDR3-1600G"    },  /* 18  DDR3-1600G */
  {  800  ,    9  ,  8  ,  12 , "DDR3-1600H"    },  /* 19  DDR3-1600H */
  {  800  ,   10  ,  8  ,  12 , "DDR3-1600J"    },  /* 20  DDR3-1600J */
  {  800  ,   11  ,  8  ,  12 , "DDR3-1600K"    },  /* 21  DDR3-1600K */
  {  933  ,   10  ,  9  ,  14 , "DDR3-1866J"    },  /* 22  DDR3-1866J */
  {  933  ,   11  ,  9  ,  14 , "DDR3-1866K"    },  /* 23  DDR3-1866K */
  {  933  ,   12  ,  9  ,  14 , "DDR3-1866L"    },  /* 24  DDR3-1866L */
  {  933  ,   13  ,  9  ,  14 , "DDR3-1866M"    },  /* 25  DDR3-1866M */
  { 1066  ,   11  , 10  ,  16 , "DDR3-2133K"    },  /* 26  DDR3-2133K */
  { 1066  ,   12  , 10  ,  16 , "DDR3-2133L"    },  /* 27  DDR3-2133L */
  { 1066  ,   13  , 10  ,  16 , "DDR3-2133M"    },  /* 28  DDR3-2133M */
  { 1066  ,   14  , 10  ,  16 , "DDR3-2133N"    },  /* 29  DDR3-2133N */
};


/*
 * ===========================================================================
 * 
 *   Function Name: chal_memc_ddr3_init
 * 
 *   Description:
 *       Initialize MEMC controller for DDR3
 * 
 * ===========================================================================
 */
MEMC_EXPORT BCM_ERR_CODE chal_memc_ddr3_init (
    void* generic_hdl
)
{
    uint32_t wrvalue = 0x0;
    uint32_t base_addr = 0x0;

    CHAL_MEMC_HANDLE handle = (CHAL_MEMC_HANDLE)generic_hdl;

    if (!handle) {
        BCM_DBG_ERR (("invalid argument: handle==0\n"));
        return BCM_ERROR;
    }

    base_addr = handle->memc_open_reg_base;

    if ( ddr3_clock_configuration( handle ) == BCM_ERROR )
    {
        BCM_DBG_ERR (("DDR3 Clock configuration failed\n"));
        return BCM_ERROR;
    }

    /* Configure the strap settings */
    if ( ddr3_strap_config( handle ) == BCM_ERROR )
    {
       BCM_DBG_ERR (("DDR3 Strap configuration failed\n"));
       return BCM_ERROR;
    }

    /* Timing 0 : Based on 400MHz */
    wrvalue = ( 0xe << CSR_DRAM_TIMING0_TRAS_SHIFT |
                0x5 << CSR_DRAM_TIMING0_TWR_SHIFT |
                0x3 << CSR_DRAM_TIMING0_TRTP_SHIFT |
                0x5 << CSR_DRAM_TIMING0_TRP_PB_SHIFT |
                0x5 << CSR_DRAM_TIMING0_TRP_AB_SHIFT |
                0x3 << CSR_DRAM_TIMING0_TRRD_SHIFT |
                0x5 << CSR_DRAM_TIMING0_TRCD_SHIFT );
    CHAL_REG_WRITE32( base_addr + CSR_DRAM_TIMING0_OFFSET, wrvalue);

    /* Timing 1 : Based on 400MHz */
    wrvalue = ( 0x1 << CSR_DRAM_TIMING1_TDQSCK_SHIFT |
                0x1C<< CSR_DRAM_TIMING1_TXSR_SHIFT |
                0x4 << CSR_DRAM_TIMING1_TCKESR_SHIFT |
                0x3 << CSR_DRAM_TIMING1_TXP_SHIFT |
                0xF << CSR_DRAM_TIMING1_TFAW_SHIFT |
                0x3 << CSR_DRAM_TIMING1_TWTR_SHIFT );
    CHAL_REG_WRITE32( base_addr + CSR_DRAM_TIMING1_OFFSET, wrvalue);

    if ( ddr3_auto_calibrate( handle ) == BCM_ERROR )
    {
       /* Auto-init failed */
       BCM_DBG_ERR (("Auto-init failed\n"));
       return BCM_ERROR;
    }

    if (handle->operation_mode == MEMC_OP_MODE_ASIC)
    {
       ddr3_auto_power_down_enable( handle, CHAL_MEMC_DDR3_POWER_DOWN_MODE_PRECHARGE );
    }

    /* software reset*/
    CHAL_REG_WRITE32( base_addr + CSR_SOFT_RESET_OFFSET, 0);

    /* configure the cl, cwl and wr in the DDR3 mode registers */
    if ( ddr3_config_mode_registers( handle ) == BCM_ERROR )
    {
       BCM_DBG_ERR (("DDR3 Mode Register configuration failed\n"));
       return BCM_ERROR;
    }

    /* Configure the DRAM start and end address, dram density, auto calculates memory size */
    if ( ddr3_config_dram( handle ) == BCM_ERROR )
    {
       BCM_DBG_ERR (("DDR3 DRAM configuration failed\n"));
       return BCM_ERROR;
    }

    /** @todo The tRFC and tREFI really need to be set based on the clock speed. */
    if (handle->operation_mode == MEMC_OP_MODE_ASIC)
    {
       /* Values obtained from simulation test program */
       handle->refresh_ctrl = (0x04<<CSR_REFRESH_CNTRL_PENDING_REFRESH_PRIORITY_COUNT_SHIFT |
                               0x1d<<CSR_REFRESH_CNTRL_TRFC_SHIFT |
                               0xcd<<CSR_REFRESH_CNTRL_REFRESH_PERIOD_SHIFT ) ; /* ru_div(7800/38.46)) */
    }
    else
    {
       /* Values obtained from simulation test program */
       handle->refresh_ctrl = (0x2b<<CSR_REFRESH_CNTRL_TRFC_SHIFT |
                               0x65<<CSR_REFRESH_CNTRL_REFRESH_PERIOD_SHIFT ) ; /* ru_div(7800/76.92)) */
    }

    wrvalue = handle->refresh_ctrl;
    CHAL_REG_WRITE32( base_addr + CSR_REFRESH_CNTRL_OFFSET, wrvalue);

    /* ZQ Calibration timing */
    wrvalue = ( 0xff<<CSR_DRAM_ZQ_CALIBRATION_TIMING_TZQCL_SHIFT |
                0x40<<CSR_DRAM_ZQ_CALIBRATION_TIMING_TZQCS_SHIFT );
    CHAL_REG_WRITE32( base_addr + CSR_DRAM_ZQ_CALIBRATION_TIMING_OFFSET, wrvalue);

    CHAL_DELAY_MICROS (1); /* wait 1 us*/

    /* init done */
    wrvalue = CSR_SW_INIT_DONE_DONE_MASK;
    CHAL_REG_WRITE32( base_addr + CSR_SW_INIT_DONE_OFFSET, wrvalue); /* 0x1*/

    return(BCM_SUCCESS);
}

/*
 * ===========================================================================
 *
 *   Function Name: chal_memc_ddr3_get_clock_speed
 *
 *   Description:
 *       Get the current DDR3 clock speed in hz
 *
 * ===========================================================================
 */
MEMC_EXPORT BCM_ERR_CODE chal_memc_ddr3_get_clock_speed (
    void* generic_hdl,
    uint32_t *clock_speed_hz
)
{
    CHAL_MEMC_HANDLE handle = (CHAL_MEMC_HANDLE)generic_hdl;
    uint32_t  ddr3_ctl_base_addr;
    uint32_t  rdvalue;
    MEMC_MDIV_T  mdiv;
    uint32_t  mdiv_val;
    uint32_t  pdiv;
    uint32_t  pdiv_val;
    uint32_t  ndiv_int;
    uint32_t  ndiv_frac;
    uint32_t  xtal_freq;

    ddr3_ctl_base_addr = handle->memc_open_ddr3_ctl_base;

    if (handle->operation_mode == MEMC_OP_MODE_ASIC)
    {
       switch ( chipregHw_getStrapXtalType() )
       {
          case chipregHw_STRAP_XTAL_TYPE_13MHZ :
          {
             xtal_freq = 13*1000*1000;
             break;
          }
          case chipregHw_STRAP_XTAL_TYPE_26MHZ:
          {
            xtal_freq = 26*1000*1000;
            break;
          }
          case chipregHw_STRAP_XTAL_TYPE_19p2MHZ:
          {
            xtal_freq = 19200000;
            break;
          }
          case chipregHw_STRAP_XTAL_TYPE_38p4MHZ:
          {
            xtal_freq = 38400000;
            break;
          }
          default:
          {
             BCM_DBG_ERR (("Unsupported XTAL strap\n" ));
             *clock_speed_hz = 0;
             return (BCM_ERROR);
          }
       }

       rdvalue = CHAL_REG_READ32( ddr3_ctl_base_addr + DDR3CTL_PLL_CTL6_OFFSET );
       ndiv_int = (rdvalue & DDR3CTL_PLL_CTL6_PLL_NDIV_INT_MASK) >> DDR3CTL_PLL_CTL6_PLL_NDIV_INT_SHIFT;
       ndiv_frac = (rdvalue & DDR3CTL_PLL_CTL6_PLL_NDIV_FRAC_MASK) >> DDR3CTL_PLL_CTL6_PLL_NDIV_FRAC_SHIFT;

       rdvalue = CHAL_REG_READ32( ddr3_ctl_base_addr + DDR3CTL_PLL_CTL7_OFFSET );
       pdiv = (rdvalue & DDR3CTL_PLL_CTL7_PLL_PDIV_MASK) >> DDR3CTL_PLL_CTL7_PLL_PDIV_SHIFT;
       pdiv_val = ((pdiv == 0) ? 8 : pdiv);

       rdvalue = CHAL_REG_READ32( ddr3_ctl_base_addr + DDR3CTL_PLL_CTL2_OFFSET );
       mdiv = (rdvalue & DDR3CTL_PLL_CTL2_PLL_CH0_MDIV_MASK) >> DDR3CTL_PLL_CTL2_PLL_CH0_MDIV_SHIFT;
       mdiv_val = ((mdiv == 0) ? 256 : mdiv);

       /* Store the configured DDR clock speed */
       handle->mem_device_ddr3.dev_info.ddr_speed_hz = ((xtal_freq*ndiv_int)/pdiv_val + ((uint32_t)((xtal_freq*(uint64_t)ndiv_frac)>>20))/pdiv_val) / mdiv_val ;
    }
    else
    {
       /* On the FPGA, the DDR3 clock is hardcoded */
       handle->mem_device_ddr3.dev_info.ddr_speed_hz = DDR3_CLOCK_SPEED_FPGA;
    }

    *clock_speed_hz = handle->mem_device_ddr3.dev_info.ddr_speed_hz;

    return(BCM_SUCCESS);
}

/*
 * ===========================================================================
 *
 *   Function Name: chal_memc_ddr3_get_dev_info
 *
 *   Description:
 *       Get the current DDR3 device configuration
 *
 * ===========================================================================
 */
MEMC_EXPORT BCM_ERR_CODE chal_memc_ddr3_get_dev_info (
    void* generic_hdl
)
{
    CHAL_MEMC_HANDLE handle = (CHAL_MEMC_HANDLE)generic_hdl;
    uint32_t rdvalue = 0x0;
    uint32_t ddr40_phy_base_addr = 0x0;

    CHAL_MEMC_DDR3_DEV_INFO_T* dev_infop = &(handle->mem_device_ddr3.dev_info);
    CHAL_MEMC_DDR3_CONFIG_T* dev_configp = &(handle->mem_device_ddr3.dev_config);

    ddr40_phy_base_addr = handle->memc_open_ddr40_phy_addr_ctl_base;

    /* Read Strap 1 Status */
    rdvalue = CHAL_REG_READ32( ddr40_phy_base_addr + DDR40_PHY_ADDR_CTL_STRAP_STATUS_OFFSET );

    if ( rdvalue & DDR40_PHY_ADDR_CTL_STRAP_STATUS_STRAPS_VALID_MASK )
    {
       dev_configp->ad_width   = (rdvalue & DDR40_PHY_ADDR_CTL_STRAP_STATUS_AD_WIDTH_MASK) >> DDR40_PHY_ADDR_CTL_STRAP_STATUS_AD_WIDTH_SHIFT;
       dev_configp->rank       = (rdvalue & DDR40_PHY_ADDR_CTL_STRAP_STATUS_DUAL_RANK_MASK) >> DDR40_PHY_ADDR_CTL_STRAP_STATUS_DUAL_RANK_SHIFT;
       dev_configp->bus_width  = (rdvalue & (DDR40_PHY_ADDR_CTL_STRAP_STATUS_BUS8_MASK | DDR40_PHY_ADDR_CTL_STRAP_STATUS_BUS16_MASK)) >> DDR40_PHY_ADDR_CTL_STRAP_STATUS_BUS8_SHIFT;
       dev_configp->chip_width = (rdvalue & DDR40_PHY_ADDR_CTL_STRAP_STATUS_CHIP_WIDTH_MASK) >> DDR40_PHY_ADDR_CTL_STRAP_STATUS_CHIP_WIDTH_SHIFT;
       dev_configp->vddq       = (rdvalue & DDR40_PHY_ADDR_CTL_STRAP_STATUS_VDDQ_MASK) >> DDR40_PHY_ADDR_CTL_STRAP_STATUS_VDDQ_SHIFT;
       dev_configp->chip_size  = (rdvalue & DDR40_PHY_ADDR_CTL_STRAP_STATUS_CHIP_SIZE_MASK) >> DDR40_PHY_ADDR_CTL_STRAP_STATUS_CHIP_SIZE_SHIFT;
       dev_configp->jedec_type = (rdvalue & DDR40_PHY_ADDR_CTL_STRAP_STATUS_JEDEC_TYPE_MASK) >> DDR40_PHY_ADDR_CTL_STRAP_STATUS_JEDEC_TYPE_SHIFT;

       /* Populate device info structure  */
       dev_infop->device_speed_mhz =  (rdvalue & DDR40_PHY_ADDR_CTL_STRAP_STATUS_MHZ_MASK) >> DDR40_PHY_ADDR_CTL_STRAP_STATUS_MHZ_SHIFT;
       dev_infop->strap_source = (( rdvalue & ( DDR40_PHY_ADDR_CTL_STRAP_STATUS_FROM_MEMC_MASK |
                                                DDR40_PHY_ADDR_CTL_STRAP_STATUS_FROM_REG_MASK ) )
                                    >> DDR40_PHY_ADDR_CTL_STRAP_STATUS_FROM_MEMC_SHIFT );
    }
    else
    {
       /* Strap Status not valid */
       return BCM_ERROR;
    }

    /* Read Strap 2 Status */
    rdvalue = CHAL_REG_READ32( ddr40_phy_base_addr + DDR40_PHY_ADDR_CTL_STRAP_STATUS2_OFFSET );

    /* Populate device info structure  */
    dev_infop->wr  = (rdvalue & DDR40_PHY_ADDR_CTL_STRAP_STATUS2_WR_MASK)  >> DDR40_PHY_ADDR_CTL_STRAP_STATUS2_WR_SHIFT;
    dev_infop->cwl = (rdvalue & DDR40_PHY_ADDR_CTL_STRAP_STATUS2_CWL_MASK) >> DDR40_PHY_ADDR_CTL_STRAP_STATUS2_CWL_SHIFT;
    dev_infop->cl  = (rdvalue & DDR40_PHY_ADDR_CTL_STRAP_STATUS2_CL_MASK)  >> DDR40_PHY_ADDR_CTL_STRAP_STATUS2_CL_SHIFT;
    dev_infop->al  = (rdvalue & DDR40_PHY_ADDR_CTL_STRAP_STATUS2_AL_MASK)  >> DDR40_PHY_ADDR_CTL_STRAP_STATUS2_AL_SHIFT;
    strcpy (dev_infop->jedec_name, jedec_info[dev_configp->jedec_type].name);

    handle->total_bytes = CHAL_REG_READ32( handle->memc_secure_reg_base + SEC_DRAM_END_ADDR_OFFSET) -
                          CHAL_REG_READ32( handle->memc_secure_reg_base + SEC_DRAM_START_ADDR_OFFSET) + 1;

    return(BCM_SUCCESS);

}

/*
 * ===========================================================================
 *
 *   Function Name: chal_memc_ddr3_read_mode_register
 *
 *   Description:
 *       Read DDR3 mode register
 *
 * ===========================================================================
 */
MEMC_EXPORT uint32_t chal_memc_ddr3_read_mode_register(
    void* generic_hdl,
    uint32_t cs,
    uint32_t reg_addr
)
{
    CHAL_MEMC_HANDLE handle = (CHAL_MEMC_HANDLE)generic_hdl;
    uint32_t rdvalue = 0x0;
    (void)cs;

    if ( reg_addr > DDR3_MODE_REG_3 )
    {
       return 0;
    }

    rdvalue = CHAL_REG_READ32( handle->memc_open_ddr40_phy_addr_ctl_base + (DDR40_PHY_ADDR_CTL_MODE_REG0_OFFSET + (reg_addr * sizeof(uint32_t))) );

    if ( !(rdvalue & DDR40_PHY_ADDR_CTL_MODE_REG0_VALID_MASK) )
    {
       return 0;
    }

    return ((rdvalue & DDR40_PHY_ADDR_CTL_MODE_REG0_AD_MASK) >> DDR40_PHY_ADDR_CTL_MODE_REG0_AD_SHIFT);

}


/*
 * ===========================================================================
 *
 *   Function Name: chal_memc_ddr3_write_mode_register
 *
 *   Description:
 *       Write DDR3 mode register
 *
 * ===========================================================================
 */
MEMC_EXPORT _Bool chal_memc_ddr3_write_mode_register(
    void* generic_hdl,
    uint32_t cs,
    uint32_t reg_addr,
    uint32_t value
)
{
    if( ddr3_mode_reg_write ( (CHAL_MEMC_HANDLE)generic_hdl, (MEMC_CS_CONNECTION)cs, reg_addr, value) == BCM_SUCCESS )
    {
       return TRUE;
    }
    else
    {
       return FALSE;
    }
}


/*
 * ===========================================================================
 *   Description:
 *       Enter / exit DDR3 device self refresh mode
 * ===========================================================================
 */
/** @todo These routines probably need to be defined as inline functions for
 *  them to useable in a real application. It is likely that they will have
 *  to be executed from SRAM or similar. However, the handle stuff really
 *  needs to be simplified in order to do this. It appears that all the
 *  handle really needs to contain is a single register base address.
 *  Worst case is several base addresses as is done currently. The
 *  rest of the info is only needed during initialization, and could
 *  be retrieved from registers or the handle could reference (pointer)
 *  a configuration structure. Using SRAM and the current handle, will
 *  need to make a copy of the handle in SRAM space, and then invoke
 *  these routines using the SRAM handle.
 */
MEMC_EXPORT BCM_ERR_CODE chal_memc_ddr3_self_refresh_enter(void* generic_hdl)
{

    #if !defined(resetHw_DDR3_SELF_REFRESH_MODE_AVAIL) || !resetHw_DDR3_SELF_REFRESH_MODE_AVAIL

    (void) generic_hdl;

    return (BCM_ERROR);

    #else

    CHAL_MEMC_HANDLE handle = (CHAL_MEMC_HANDLE)generic_hdl;
    uint32_t data;


    /* See the DDR40 PHY spec for further details on the following. */

    /* Wait until OK to perform PHY standby operations. */
    do
    {
        data = CHAL_REG_READ32(handle->memc_open_ddr40_phy_addr_ctl_base + DDR40_PHY_ADDR_CTL_STANDBY_CONTROL_OFFSET);
    }
    while ( !(data & DDR40_PHY_ADDR_CTL_STANDBY_CONTROL_STANDBY_READY_MASK) );

    /* Put the PHY into the ARMED state with CKE low and RST_N high.
     * We are going to use the PHY standby pin to exit the STANDBY
     * standby state instead of VDDO. We also have already configured
     * the device, so don't need to re-issue the Mode Register Set (MRS)
     * when we exit.
     */
    switch (handle->mem_device_ddr3.dev_config.vddq)
    {
        case CHAL_MEMC_DDR3_VDDQ_1P35V:
            data = CHAL_MEMC_DDR3_LDO_1P35V << DDR40_PHY_ADDR_CTL_STANDBY_CONTROL_LDO_VOLTS_SHIFT;
            break;
        case CHAL_MEMC_DDR3_VDDQ_1P50V:
            data = CHAL_MEMC_DDR3_LDO_1P50V << DDR40_PHY_ADDR_CTL_STANDBY_CONTROL_LDO_VOLTS_SHIFT;
            break;
        case CHAL_MEMC_DDR3_VDDQ_1P80V:
            data = CHAL_MEMC_DDR3_LDO_1P80V << DDR40_PHY_ADDR_CTL_STANDBY_CONTROL_LDO_VOLTS_SHIFT;
            break;
        default:
            return (BCM_ERROR);
    }
    data |= DDR40_PHY_ADDR_CTL_STANDBY_CONTROL_RST_N_MASK |
            DDR40_PHY_ADDR_CTL_STANDBY_CONTROL_SKIP_MRS_MASK |
            DDR40_PHY_ADDR_CTL_STANDBY_CONTROL_STANDBY_EXIT_PIN_EN_MASK;

    CHAL_REG_WRITE32(handle->memc_open_ddr40_phy_addr_ctl_base + DDR40_PHY_ADDR_CTL_STANDBY_CONTROL_OFFSET,
                        data | 0x5 );

    do
    {
        data = CHAL_REG_READ32(handle->memc_open_ddr40_phy_addr_ctl_base + DDR40_PHY_ADDR_CTL_STANDBY_CONTROL_OFFSET);
    }
    while ( !(data & DDR40_PHY_ADDR_CTL_STANDBY_CONTROL_ARMED_MASK) );

    /* Instruct the MEMC to enter self-refresh mode. The MEMC will send a
     * signal to the PHY to go into its standby mode. Wait for the PHY to
     * enter standby before returning.
     */
    resetHw_ddr3SelfRefresh(resetHw_DDR3_SELF_REFRESH_MODE_ENTER);
    do
    {
        data = CHAL_REG_READ32(handle->memc_open_ddr40_phy_addr_ctl_base + DDR40_PHY_ADDR_CTL_STANDBY_CONTROL_OFFSET);
    }
    while ( !(data & DDR40_PHY_ADDR_CTL_STANDBY_CONTROL_STANDBY_ACTIVE_MASK) );

    return (BCM_SUCCESS);

    #endif /* !defined(resetHw_DDR3_SELF_REFRESH_MODE_AVAIL) || !resetHw_DDR3_SELF_REFRESH_MODE_AVAIL */
}

MEMC_EXPORT BCM_ERR_CODE chal_memc_ddr3_self_refresh_exit(void* generic_hdl)
{

    #if !defined(resetHw_DDR3_SELF_REFRESH_MODE_AVAIL) || !resetHw_DDR3_SELF_REFRESH_MODE_AVAIL

    (void) generic_hdl;

    return (BCM_ERROR);

    #else

    CHAL_MEMC_HANDLE handle = (CHAL_MEMC_HANDLE)generic_hdl;
    uint32_t data;


    /* See the DDR40 PHY spec for further details on the following. */

    /* Instruct the MEMC to exit self-refresh mode. The MEMC will send a
     * signal to the PHY to leave its standby mode. Wait for the PHY to
     * exit standby and then become active before returning.
     */
    resetHw_ddr3SelfRefresh(resetHw_DDR3_SELF_REFRESH_MODE_EXIT);
    do
    {
        data = CHAL_REG_READ32(handle->memc_open_ddr40_phy_addr_ctl_base + DDR40_PHY_ADDR_CTL_STANDBY_CONTROL_OFFSET);
    }
    while ( (data & DDR40_PHY_ADDR_CTL_STANDBY_CONTROL_STANDBY_ACTIVE_MASK) );
    do
    {
        data = CHAL_REG_READ32(handle->memc_open_ddr40_phy_addr_ctl_base + DDR40_PHY_ADDR_CTL_STANDBY_CONTROL_OFFSET);
    }
    while ( !(data & DDR40_PHY_ADDR_CTL_STANDBY_CONTROL_STANDBY_READY_MASK) );

    return (BCM_SUCCESS);

    #endif /* !defined(resetHw_DDR3_SELF_REFRESH_MODE_AVAIL) || !resetHw_DDR3_SELF_REFRESH_MODE_AVAIL */
}

/* ---- Private Functions ------------------------------------------------ */

/*
 * ===========================================================================
 *
 *   Function Name: ddr3_clock_configuration
 *
 *   Description:
 *       Configures the MEMC output frequency to DDR
 *
 * ===========================================================================
 */
static BCM_ERR_CODE ddr3_clock_configuration(CHAL_MEMC_HANDLE handle)
{
    uint32_t ddr3_ctl_base_addr;
    uint32_t ddr40_phy_base_addr;
    uint32_t memc_base_addr;
    uint32_t pll_fcw;
    uint32_t xtal_freq;
    uint32_t tmp;


    memc_base_addr = handle->memc_open_reg_base;
    ddr3_ctl_base_addr = handle->memc_open_ddr3_ctl_base;
    ddr40_phy_base_addr = handle->memc_open_ddr40_phy_addr_ctl_base;

    /* Disable HW Frequency Change procedure since it is illegal for a DDR3 device */
    CHAL_REG_CLRBIT32( memc_base_addr + CSR_HW_FREQ_CHANGE_CNTRL_OFFSET,
                       CSR_HW_FREQ_CHANGE_CNTRL_HW_FREQ_CHANGE_EN_MASK | CSR_HW_FREQ_CHANGE_CNTRL_HW_AUTO_PWR_TRANSITION_MASK );

    if (handle->operation_mode != MEMC_OP_MODE_ASIC)
    {
        /* On the FPGA, the DDR3 clock is hardcoded */
        handle->mem_device_ddr3.dev_info.ddr_speed_hz = DDR3_CLOCK_SPEED_FPGA;
        return(BCM_SUCCESS);
    }

    /* Enable the post-divider bypass for all channels */
    CHAL_REG_CLRBIT32( ddr3_ctl_base_addr + DDR3CTL_PLL_CTL1_OFFSET, DDR3CTL_PLL_CTL1_PLL_BYPCLK_EN_MASK );

    /* Disable HW Frequency Change procedure since it is illegal for a DDR3 device */
    CHAL_REG_CLRBIT32( memc_base_addr + CSR_HW_FREQ_CHANGE_CNTRL_OFFSET,
                         ( CSR_HW_FREQ_CHANGE_CNTRL_HW_FREQ_CHANGE_EN_MASK |
                           CSR_HW_FREQ_CHANGE_CNTRL_HW_AUTO_PWR_TRANSITION_MASK ) );

    if (handle->clock_Mhz > jedec_info[handle->mem_device_ddr3.dev_config.jedec_type].mhz)
    {
       BCM_DBG_ERR (("Clock speed %u MHz > %s max of %u MHz\n",
                        handle->clock_Mhz,
                        jedec_info[handle->mem_device_ddr3.dev_config.jedec_type].name,
                        jedec_info[handle->mem_device_ddr3.dev_config.jedec_type].mhz
                   ));
       return(BCM_ERROR);
    }

    switch ( chipregHw_getStrapXtalType () )
    {
       case chipregHw_STRAP_XTAL_TYPE_13MHZ :
       {
          xtal_freq = 13*1000*1000;
          break;
       }
       case chipregHw_STRAP_XTAL_TYPE_26MHZ:
       {
         xtal_freq = 26*1000*1000;
         break;
       }
       case chipregHw_STRAP_XTAL_TYPE_19p2MHZ:
       {
         xtal_freq = 19200000;
         break;
       }
       case chipregHw_STRAP_XTAL_TYPE_38p4MHZ:
       {
         xtal_freq = 38400000;
         break;
       }
       default:
       {
          BCM_DBG_ERR (("Unknown XTAL strap\n" ));
          return(BCM_ERROR);
       }
    }

    /*
     * Get configuration parameters for the DDR3 PLL, based upon the XTAL (Fxtal).
     *
     * Fvco = (Fxtal / PDIV) * (NDIV_INT + (NDIV_FRAC/2^20))
     * Fddr3 = Fvco / MDIV;
     * Fddr3 * MDIV = (Fxtal / PDIV) * (NDIV_INT + (NDIV_FRAC/2^20))
     *
     * We are going to fix the PDIV at 1, which gives us the following.
     *
     * Fddr3 * MDIV = Fxtal * (NDIV_INT + (NDIV_FRAC/2^20))
     *              = Fxtal * ((NDIV_INT*2*20 + NDIV_FRAC) / 2^20)
     *              = Fxtal * ((NDIV_INT<<20) + NDIV_FRAC) / 2^20)
     *
     * Frequency Code Word (FCW) = (NDIV_INT<<20) + NDIV_FRAC
     *                           = (Fddr3 * MDIV * 2^20) / Fxtal
     *
     * Fddr3 (actual) = (FCW * Fxtal) / (MDIV * 2^20)
     *
     * We are going to fix the MDIV at 4 which should be adequate for our purposes.
     */
    #define MDIV            4
    #define PDIV            1
    #define NDIV_FRAC_BITS  20
    #define NDIV_FRAC_MASK  ((1 << NDIV_FRAC_BITS) - 1)

    /* Configure PDIV */
    CHAL_REG_WRITE32( ddr3_ctl_base_addr + DDR3CTL_PLL_CTL7_OFFSET, PDIV << DDR3CTL_PLL_CTL7_PLL_PDIV_SHIFT );

    /* Calculate the frequency code word (FCW) and then store it. Unfortunately the register has
     * integer and fractional parts in reverse order. Extract the integer part first, and then
     * add the fractional part. A 64-bit quantity should be enough for these calculations. The
     * worst case is DDR3-2133 or 1,066,666,667 Hz, which * 4 * 2^20 is 0x000F_e502_aac0_0000.
     */
    pll_fcw = (uint32_t)div_u64( (uint64_t)handle->mem_device_ddr3.dev_config.clock_hz * MDIV * (1<<NDIV_FRAC_BITS), xtal_freq );

    tmp  = ((pll_fcw & ~NDIV_FRAC_MASK) >> NDIV_FRAC_BITS) << DDR3CTL_PLL_CTL6_PLL_NDIV_INT_SHIFT;
    tmp |=  (pll_fcw &  NDIV_FRAC_MASK)                    << DDR3CTL_PLL_CTL6_PLL_NDIV_FRAC_SHIFT;
    CHAL_REG_WRITE32( ddr3_ctl_base_addr + DDR3CTL_PLL_CTL6_OFFSET, tmp );

    /* Now calculate the actual clock speed being generated by the FCW. This should never be
     * greater than the requested frequency due to truncation that may occur with the FCW
     * calculations.
     */
    tmp = (uint32_t)div_u64( ((uint64_t)pll_fcw * xtal_freq), (MDIV * (1<<NDIV_FRAC_BITS)) );
    handle->mem_device_ddr3.dev_info.ddr_speed_hz = tmp;

    /* PLL Global Reset configured to normal operation */
    CHAL_REG_SETBIT32( ddr3_ctl_base_addr + DDR3CTL_PLL_CTL5_OFFSET, 1 << DDR3CTL_PLL_CTL5_PLL_RESETB_SHIFT );

    /* Wait until PLL lock enters phase tracking mode */
    do
    {
        tmp = CHAL_REG_READ32( ddr3_ctl_base_addr + DDR3CTL_PLL_STATUS_OFFSET );
    }
    while ( ((tmp & DDR3CTL_PLL_STATUS_PLL_LOCK_MASK) >> DDR3CTL_PLL_STATUS_PLL_LOCK_SHIFT) != 1 );

    /* Configure Channel 0 MDIV */
    CHAL_REG_SETBIT32( ddr3_ctl_base_addr + DDR3CTL_PLL_CTL2_OFFSET, MDIV << DDR3CTL_PLL_CTL2_PLL_CH0_MDIV_SHIFT );

    /* Toggle Load Enable for post divider on Channel 0 */
    CHAL_REG_SETBIT32( ddr3_ctl_base_addr + DDR3CTL_PLL_CTL5_OFFSET, (1 << 0) << DDR3CTL_PLL_CTL5_PLL_CH_LOAD_EN_SHIFT );
    CHAL_REG_CLRBIT32( ddr3_ctl_base_addr + DDR3CTL_PLL_CTL5_OFFSET, (1 << 0) << DDR3CTL_PLL_CTL5_PLL_CH_LOAD_EN_SHIFT );

    /* PLL Post divider reset configured to normal operation */
    CHAL_REG_SETBIT32( ddr3_ctl_base_addr + DDR3CTL_PLL_CTL5_OFFSET, 1 << DDR3CTL_PLL_CTL5_PLL_POST_RESETB_SHIFT);

    /* Take PHY PLL out of reset */
    CHAL_REG_CLRBIT32( ddr40_phy_base_addr + DDR40_PHY_ADDR_CTL_PLL_CONFIG_OFFSET, 1 << DDR40_PHY_ADDR_CTL_PLL_CONFIG_RESET_SHIFT);

    /* Set the clock speed input to the PHY */
    tmp  = CHAL_REG_READ32( ddr3_ctl_base_addr + DDR3CTL_PHY_CONFIG_OFFSET );
    tmp &= ~DDR3CTL_PHY_CONFIG_MC2IOB_PHY_DDR_MHZ_MASK;
    tmp |= ((handle->mem_device_ddr3.dev_info.ddr_speed_hz / 1000000) << DDR3CTL_PHY_CONFIG_MC2IOB_PHY_DDR_MHZ_SHIFT);
    CHAL_REG_WRITE32( ddr3_ctl_base_addr + DDR3CTL_PHY_CONFIG_OFFSET, tmp );

    return(BCM_SUCCESS);
}

/*
 * ===========================================================================
 *
 *   Function Name: ddr3_strap_config
 *
 *   Description:
 *       Configures the strap settings
 *
 * ===========================================================================
 */
static BCM_ERR_CODE ddr3_strap_config(CHAL_MEMC_HANDLE handle)
{
    uint32_t wrvalue = 0x0;
    uint32_t rdvalue = 0x0;
    uint32_t ddr40_phy_base_addr = 0x0;
    JEDEC_INFO_T *jedec_p;
    CHAL_MEMC_DDR3_CONFIG_T* dev_configp = &(handle->mem_device_ddr3.dev_config);
    CHAL_MEMC_DDR3_DEV_INFO_T* dev_infop = &(handle->mem_device_ddr3.dev_info);

    ddr40_phy_base_addr = handle->memc_open_ddr40_phy_addr_ctl_base;

    if ( dev_configp->jedec_type >= CHAL_MEMC_DDR3_JEDEC_TYPE_MAX )
    {
       BCM_DBG_ERR (("Invalid JEDEC type %d\n", dev_configp->jedec_type ));
       return(BCM_ERROR);
    }

    if ( dev_configp->strap_override != TRUE )
    {
       BCM_DBG_ERR (("Strap override must be enabled\n" ));
       return(BCM_ERROR);
    }

    jedec_p = &jedec_info[dev_configp->jedec_type];

    if (chipregHw_getChipIdNum() == 0x11160)
    {
    	/* Workaround for JIRA HWBIGISLAND-891. PHY auto-init is setting PLL divider
	 * configuration register incorrectly. Configure for 533 MHz to avoid issue.
	 */
    	wrvalue = (jedec_p->mhz < 500 ? 533 : jedec_p->mhz) << DDR40_PHY_ADDR_CTL_STRAP_CONTROL_MHZ_SHIFT;
    }
    else
    {
    	wrvalue = jedec_p->mhz << DDR40_PHY_ADDR_CTL_STRAP_CONTROL_MHZ_SHIFT;
    }

    wrvalue |= ((dev_configp->ad_width)  <<DDR40_PHY_ADDR_CTL_STRAP_CONTROL_AD_WIDTH_SHIFT   |
                (dev_configp->rank)      <<DDR40_PHY_ADDR_CTL_STRAP_CONTROL_DUAL_RANK_SHIFT  |
                (dev_configp->bus_width) <<DDR40_PHY_ADDR_CTL_STRAP_CONTROL_BUS8_SHIFT       |
                (dev_configp->chip_width)<<DDR40_PHY_ADDR_CTL_STRAP_CONTROL_CHIP_WIDTH_SHIFT |
                (dev_configp->vddq)      <<DDR40_PHY_ADDR_CTL_STRAP_CONTROL_VDDQ_SHIFT       |
                (dev_configp->chip_size) <<DDR40_PHY_ADDR_CTL_STRAP_CONTROL_CHIP_SIZE_SHIFT  |
                (dev_configp->jedec_type)<<DDR40_PHY_ADDR_CTL_STRAP_CONTROL_JEDEC_TYPE_SHIFT |
                (dev_configp->strap_override)<<DDR40_PHY_ADDR_CTL_STRAP_CONTROL_STRAPS_VALID_SHIFT );

    CHAL_REG_WRITE32( ddr40_phy_base_addr + DDR40_PHY_ADDR_CTL_STRAP_CONTROL_OFFSET, wrvalue );

    wrvalue = CHAL_MEMC_DDR3_JEDEC_TYPE_IS_DDR3(dev_configp->jedec_type) ? DDR40_PHY_ADDR_CTL_STRAP_CONTROL2_DDR3_MASK : 0;

    dev_infop->al = 0;
    wrvalue |= (dev_infop->al << DDR40_PHY_ADDR_CTL_STRAP_CONTROL2_AL_SHIFT);

    dev_infop->cl = jedec_p->cl;
    wrvalue |= (dev_infop->cl << DDR40_PHY_ADDR_CTL_STRAP_CONTROL2_CL_SHIFT);

    dev_infop->cwl = jedec_p->cwl;
    wrvalue |= (dev_infop->cwl << DDR40_PHY_ADDR_CTL_STRAP_CONTROL2_CWL_SHIFT);

    dev_infop->wr = jedec_p->wr;
    wrvalue |= (dev_infop->wr << DDR40_PHY_ADDR_CTL_STRAP_CONTROL2_WR_SHIFT);

    CHAL_REG_WRITE32( ddr40_phy_base_addr + DDR40_PHY_ADDR_CTL_STRAP_CONTROL2_OFFSET, wrvalue );

    /* Populate remainder of device info structure  */
    dev_infop->device_speed_mhz =  jedec_p->mhz;
    strcpy (dev_infop->jedec_name, jedec_p->name);

    rdvalue = BCM_REG_READ32 (handle, handle->memc_open_ddr40_phy_addr_ctl_base + DDR40_PHY_ADDR_CTL_STRAP_STATUS_OFFSET);
    if ( rdvalue & DDR40_PHY_ADDR_CTL_STRAP_STATUS_STRAPS_VALID_MASK )
    {
       dev_infop->strap_source = (( rdvalue & ( DDR40_PHY_ADDR_CTL_STRAP_STATUS_FROM_MEMC_MASK |
                                                DDR40_PHY_ADDR_CTL_STRAP_STATUS_FROM_REG_MASK ) )
                                    >> DDR40_PHY_ADDR_CTL_STRAP_STATUS_FROM_MEMC_SHIFT );
    }

    return(BCM_SUCCESS);
}

/*
 * ===========================================================================
 *
 *   Function Name: ddr3_auto_calibrate
 *
 *   Description:
 *       Perform auto-calibration
 *
 * ===========================================================================
 */
static BCM_ERR_CODE ddr3_auto_calibrate(CHAL_MEMC_HANDLE handle)
{
    uint32_t wrvalue = 0x0;
    uint32_t rdvalue = 0x0;
    uint32_t done = 0x0;
    uint32_t ddr40_phy_base_addr = 0x0;

    ddr40_phy_base_addr = handle->memc_open_ddr40_phy_addr_ctl_base;

    /* Auto-Calibrate */
    wrvalue = ( 0x1<<DDR40_PHY_ADDR_CTL_VDL_CALIBRATE_USE_STRAPS_SHIFT |
                0x1<<DDR40_PHY_ADDR_CTL_VDL_CALIBRATE_AUTO_INIT_SHIFT );
    CHAL_REG_WRITE32( ddr40_phy_base_addr + DDR40_PHY_ADDR_CTL_VDL_CALIBRATE_OFFSET, wrvalue );


    done = 0;
    /* polling done bit */
    do {
        rdvalue = CHAL_REG_READ32 ( ddr40_phy_base_addr + DDR40_PHY_ADDR_CTL_VDL_CALIB_STATUS_OFFSET);
        done = rdvalue & DDR40_PHY_ADDR_CTL_VDL_CALIB_STATUS_AUTO_INIT_DONE_MASK;
    } while ( !done );

    /* Verify PHY PLL has locked */
    rdvalue = 0x0;
    do {
      rdvalue = CHAL_REG_READ32( ddr40_phy_base_addr + DDR40_PHY_ADDR_CTL_PLL_STATUS_OFFSET );
    } while ((rdvalue & DDR40_PHY_ADDR_CTL_PLL_STATUS_LOCK_MASK >> DDR40_PHY_ADDR_CTL_PLL_STATUS_LOCK_SHIFT) != 1);

    rdvalue = CHAL_REG_READ32 ( ddr40_phy_base_addr + DDR40_PHY_ADDR_CTL_VDL_CALIB_STATUS_OFFSET);
    if ( rdvalue & DDR40_PHY_ADDR_CTL_VDL_CALIB_STATUS_AUTO_INIT_FAIL_MASK )
    {
       /* Auto-init failed */
       BCM_DBG_ERR (("VDL auto-calibration failed\n" ));
       return(BCM_ERROR);
    }
    else
    {
       /* Auto-init passed */
       return(BCM_SUCCESS);
    }
}


/*
 * ===========================================================================
 *
 *   Function Name: ddr3_auto_power_down_disable / enable
 *
 *   Description:
 *       Disable / enable the auto-power down feature
 *
 * ===========================================================================
 */
BCM_ERR_CODE ddr3_auto_power_down_disable(void* generic_hdl)
{
    CHAL_MEMC_HANDLE handle = (CHAL_MEMC_HANDLE)generic_hdl;
    uint32_t base_addr;

    base_addr = handle->memc_open_reg_base;

    CHAL_REG_CLRBIT32( base_addr + CSR_DDR_SW_POWER_DOWN_CONTROL_OFFSET,
                       CSR_DDR_SW_POWER_DOWN_CONTROL_POWER_DOWN_ENABLE_MASK );

    return(BCM_SUCCESS);
}

BCM_ERR_CODE ddr3_auto_power_down_enable(void* generic_hdl, CHAL_MEMC_DDR3_POWER_DOWN_MODE mode)
{
    CHAL_MEMC_HANDLE handle = (CHAL_MEMC_HANDLE)generic_hdl;
    uint32_t base_addr;

    base_addr = handle->memc_open_reg_base;

    /* Make sure the auto power down is disabled and the mode cleared
     * before re-enabling with the new mode.
     */
    CHAL_REG_CLRBIT32( base_addr + CSR_DDR_SW_POWER_DOWN_CONTROL_OFFSET,
                       CSR_DDR_SW_POWER_DOWN_CONTROL_POWER_DOWN_ENABLE_MASK | 
                       CSR_DDR_SW_POWER_DOWN_CONTROL_POWER_DOWN_MODE_MASK );
    
    CHAL_REG_SETBIT32( base_addr + CSR_DDR_SW_POWER_DOWN_CONTROL_OFFSET,
                       CSR_DDR_SW_POWER_DOWN_CONTROL_POWER_DOWN_ENABLE_MASK | mode );

    return(BCM_SUCCESS);
}

/*
 * ===========================================================================
 *
 *   Function Name: ddr3_mode_reg_write
 *
 *   Description:
 *       Write a single DDR3 Mode Register
 *
 * ===========================================================================
 */
static BCM_ERR_CODE ddr3_mode_reg_write (
    CHAL_MEMC_HANDLE handle,
    MEMC_CS_CONNECTION cs,
    uint32_t addr,
    uint32_t val
)
{
    uint32_t wrvalue=0x00;
    uint32_t base_addr = 0x00;
    uint32_t cs_bits = 0x00;

    if ( !handle ) {
        BCM_DBG_ERR (("invalid argument: handle=0 \n" ));
        return (BCM_ERROR);
    }

    if (cs == MEMC_CS_0)
    {
        cs_bits = 0x2;            /* CS0 */
    }
    else if (cs == MEMC_CS_1)
    {
        cs_bits = 0x1;            /* CS1 */
    }
    else if ( cs == MEMC_CS_BOTH )
    {
        cs_bits = 0x0;            /* both CS0 and CS1 */
    }
    else
    {
        return (BCM_ERROR);
    }

    base_addr = handle->memc_open_reg_base;
    wrvalue = ((cs_bits<<CSR_DRAM_INIT_CONTROL_CS_BITS_SHIFT) & CSR_DRAM_INIT_CONTROL_CS_BITS_MASK)
        | (MEMCX_OPEN_DRAM_INIT_CONTROL_MRW)
        /* The upper 8 bits of the val are placed in DDR3 extention field */
        | ((val<<(CSR_DRAM_INIT_CONTROL_MRW_DATA_DDR3_EXTN_SHIFT-8)) & CSR_DRAM_INIT_CONTROL_MRW_DATA_DDR3_EXTN_MASK)
        /* The bottom 8 bits of the val are placed in standard data field */
        | ((val<<CSR_DRAM_INIT_CONTROL_MRW_DATA_SHIFT) & CSR_DRAM_INIT_CONTROL_MRW_DATA_MASK)
        | ((addr<<CSR_DRAM_INIT_CONTROL_MR_ADDR_SHIFT) & CSR_DRAM_INIT_CONTROL_MR_ADDR_MASK);

    CHAL_REG_WRITE32( base_addr + CSR_DRAM_INIT_CONTROL_OFFSET, wrvalue);

    CHAL_DELAY_MICROS (1); /* wait 1 us?*/

    return BCM_SUCCESS;
}

/*
 * ===========================================================================
 *
 *  Function Name: ddr3_config_mode_registers
 *
 *  Description:
 *      Configure the mode registers with provided CL, CWL and WR configuration
 *
 * ===========================================================================
 */
static BCM_ERR_CODE ddr3_config_mode_registers(CHAL_MEMC_HANDLE handle)
{
    uint32_t value;

    /* The PHY auto-init will have already configured the device mode
     * registers for CL, CWL, WR, and possibly others. We just need to
     * make some adjustments for some settings that cannot be handled
     * by the PHY strap settings.
     */
    
    /* Configure Mode Register 0 */
    value = chal_memc_ddr3_read_mode_register(handle, MEMC_CS_0, DDR3_MODE_REG_0);
    MEMC_DBG_OUT(" mr0:   read = 0x%04x\n", value);
    value &= ~DDR3_MR0_BL_MASK;   
    MEMC_DBG_OUT(" mr0: modify = 0x%04x\n", value);
    value |= (DDR3_MR0_BL_BC4_BL8_ONFLY | DDR3_MR0_PDD_FAST_EXIT);  
    MEMC_DBG_OUT(" mr0:  write = 0x%04x\n", value);
    chal_memc_ddr3_write_mode_register(handle, MEMC_CS_BOTH, DDR3_MODE_REG_0, value);

#ifdef CSR_DDR_SW_POWER_DOWN_CONTROL_DDR3_DLL_PPD_MASK    
    /* The MEMC OPEN DLL PDD has to match the MR0 setting. Could be a
     * better location / method to do this, but this will have to do
     * for now.
     */
    if (value & DDR3_MR0_PPD_MASK)
    {
        CHAL_REG_SETBIT32( handle->memc_open_reg_base + CSR_DDR_SW_POWER_DOWN_CONTROL_OFFSET,
                            CSR_DDR_SW_POWER_DOWN_CONTROL_DDR3_DLL_PPD_MASK );
    }
    else
    {
        CHAL_REG_CLRBIT32( handle->memc_open_reg_base + CSR_DDR_SW_POWER_DOWN_CONTROL_OFFSET,
                            CSR_DDR_SW_POWER_DOWN_CONTROL_DDR3_DLL_PPD_MASK );
    }
#endif

    /* Configure Mode Register 1 */
    value = chal_memc_ddr3_read_mode_register(handle, MEMC_CS_0, DDR3_MODE_REG_1);
    MEMC_DBG_OUT(" mr1:   read = 0x%04x\n", value);
    value &= ~DDR3_MR1_RTT_NOM_MASK;   
    MEMC_DBG_OUT(" mr1: modify = 0x%04x\n", value);
    value |= handle->mem_device_ddr3.dev_config.odt_sdram_mr1;  
    MEMC_DBG_OUT(" mr1:  write = 0x%04x\n", value);
    chal_memc_ddr3_write_mode_register(handle, MEMC_CS_BOTH, DDR3_MODE_REG_1, value);

    /* Configure Mode Register 2 */
    value = chal_memc_ddr3_read_mode_register(handle, MEMC_CS_0, DDR3_MODE_REG_2);
    MEMC_DBG_OUT(" mr2:   read = 0x%04x\n", value);
    value &= ~DDR3_MR2_RTT_WR_MASK;   
    MEMC_DBG_OUT(" mr2: modify = 0x%04x\n", value);
    value |= handle->mem_device_ddr3.dev_config.odt_sdram_mr2;  
    MEMC_DBG_OUT(" mr2:  write = 0x%04x\n", value);
    chal_memc_ddr3_write_mode_register(handle, MEMC_CS_BOTH, DDR3_MODE_REG_2, value);

    return BCM_SUCCESS;
}

/*
 * ===========================================================================
 *
 *  Function Name: ddr3_config_dram
 *
 *  Description:
 *      Set the DRAM start and end address
 *      Auto-calculates the total size of memory
 *      Set the DRAM configu register
 *
 * ===========================================================================
 */
static BCM_ERR_CODE ddr3_config_dram(CHAL_MEMC_HANDLE handle)
{
    uint32_t mem_size_bytes = 0x00;
    uint32_t wrvalue = 0x0;
    uint32_t rdvalue = 0x0;
    uint32_t bus_width;
    uint32_t bus_width_val;
    uint32_t chip_width;
    uint32_t chip_width_val;
    uint32_t rank;
    uint32_t rank_val;
    uint32_t chip_size;
    uint32_t chip_size_bytes;
    uint32_t dram_config_size;
    uint32_t dram_device_width;

    rdvalue = BCM_REG_READ32 (handle, handle->memc_open_ddr40_phy_addr_ctl_base + DDR40_PHY_ADDR_CTL_STRAP_STATUS_OFFSET);

    if ( rdvalue & DDR40_PHY_ADDR_CTL_STRAP_STATUS_STRAPS_VALID_MASK )
    {
       bus_width = (rdvalue & (DDR40_PHY_ADDR_CTL_STRAP_STATUS_BUS8_MASK | DDR40_PHY_ADDR_CTL_STRAP_STATUS_BUS16_MASK))
                        >> DDR40_PHY_ADDR_CTL_STRAP_STATUS_BUS8_SHIFT;
       switch ( bus_width )
       {
          case CHAL_MEMC_DDR3_BUS_WIDTH_32:
          {
             bus_width_val = 32;
             dram_device_width = DRAM_CONFIG_WIDTH_32;
             break;
          }
          case CHAL_MEMC_DDR3_BUS_WIDTH_16:
          {
             bus_width_val = 16;
             dram_device_width = DRAM_CONFIG_WIDTH_16;
             break;
          }
          case CHAL_MEMC_DDR3_BUS_WIDTH_8:
          {
             bus_width_val = 8;
             /* The DRAM_CONFIG register (generally for LPDDR2 config) does not contain 8bit width.  Use 32 bit width as a place holder */
             dram_device_width = DRAM_CONFIG_WIDTH_32;
             break;
          }
          default:
          {
             BCM_DBG_ERR (("Invalid DDR3 bus width\n" ));
             return BCM_ERROR ;
          }
       }

       chip_width = (rdvalue & DDR40_PHY_ADDR_CTL_STRAP_STATUS_CHIP_WIDTH_MASK) >> DDR40_PHY_ADDR_CTL_STRAP_STATUS_CHIP_WIDTH_SHIFT;
       switch ( chip_width )
       {
          case CHAL_MEMC_DDR3_CHIP_WIDTH_8:
          {
             chip_width_val = 8;
             break;
          }
          case CHAL_MEMC_DDR3_CHIP_WIDTH_16:
          {
             chip_width_val = 16;
             break;
          }
          default:
          {
             BCM_DBG_ERR (("Invalid DDR3 chip width\n" ));
             return BCM_ERROR ;
          }
       }

       rank = (rdvalue & DDR40_PHY_ADDR_CTL_STRAP_STATUS_DUAL_RANK_MASK) >> DDR40_PHY_ADDR_CTL_STRAP_STATUS_DUAL_RANK_SHIFT;
       switch ( rank )
       {
          case CHAL_MEMC_DDR3_RANK_SINGLE:
          {
             rank_val = 1;
             break;
          }
          case CHAL_MEMC_DDR3_RANK_DUAL:
          {
             rank_val = 2;
             break;
          }
          default:
          {
             BCM_DBG_ERR (("Invalid DDR3 rank\n" ));
             return BCM_ERROR ;
          }
       }

       chip_size = (rdvalue & DDR40_PHY_ADDR_CTL_STRAP_STATUS_CHIP_SIZE_MASK) >> DDR40_PHY_ADDR_CTL_STRAP_STATUS_CHIP_SIZE_SHIFT;
       switch ( chip_size )
       {
          case CHAL_MEMC_DDR3_CHIP_SIZE_1Gb:
          {
             /* 128 MB */
             chip_size_bytes = 128*1024*1024;
             dram_config_size = DRAM_CONFIG_1_GB;
             break;
          }
          case CHAL_MEMC_DDR3_CHIP_SIZE_2Gb:
          {
             /* 256 MB */
             chip_size_bytes = 256*1024*1024;
             dram_config_size = DRAM_CONFIG_2_GB;
             break;
          }
          case CHAL_MEMC_DDR3_CHIP_SIZE_4Gb:
          {
             /* 512 MB */
             chip_size_bytes = 512*1024*1024;
             dram_config_size = DRAM_CONFIG_4_GB;
             break;
          }
          case CHAL_MEMC_DDR3_CHIP_SIZE_8Gb:
          {
             /* 1024 MB */
             chip_size_bytes = 1024*1024*1024;
             dram_config_size = DRAM_CONFIG_8_GB;
             break;
          }
          default:
          {
             BCM_DBG_ERR (("Invalid DDR3 chip size" ));
             return BCM_ERROR ;
          }
       }

       /* Calculate the total memory size */
       mem_size_bytes = chip_size_bytes * ( bus_width_val / chip_width_val ) * rank_val;

       /* set the DRAM start and end address */
       CHAL_REG_WRITE32( handle->memc_secure_reg_base + SEC_DRAM_START_ADDR_OFFSET, handle->memc_memory_base_dram);
       CHAL_REG_WRITE32( handle->memc_secure_reg_base + SEC_DRAM_END_ADDR_OFFSET, handle->memc_memory_base_dram+mem_size_bytes-1);

       /* store the total memory size */
       handle->total_bytes = mem_size_bytes;

       wrvalue = ( 0x0<<SEC_DRAM_CONFIG_DDR_IF_WIDTH_SHIFT |         /* LPDDR2 config - leave as 0 */
                   0x0<<SEC_DRAM_CONFIG_PASR_SHIFT |                 /* PASR inactive - default */
                   dram_device_width |
                   dram_config_size |
                   0x0<<SEC_DRAM_CONFIG_MEM_TYPE_SHIFT );            /* LPDDR2 config - leave as 0 */
       CHAL_REG_WRITE32( handle->memc_secure_reg_base + SEC_DRAM_CONFIG_OFFSET, wrvalue);

       return BCM_SUCCESS;
    }
    else
    {
       BCM_DBG_ERR (("Straps are not valid.  Skipping DRAM config\n" ));
       return BCM_ERROR;
    }
}

