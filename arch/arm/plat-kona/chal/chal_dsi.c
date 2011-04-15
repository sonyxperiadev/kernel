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
/**
*  @file   chal_dsi.c
*
*  @brief  HERA DSI cHAL
*
*  @note
*
****************************************************************************/

#include <linux/string.h>


#include "plat/chal/chal_common.h"
#include "plat/chal/chal_dsi.h"
#include "mach/rdb/brcm_rdb_sysmap_a9.h"
#include "mach/rdb/brcm_rdb_dsi1.h"

#include "mach/rdb/brcm_rdb_util.h"

typedef struct
{
    cBool     init;
    cUInt32   baseAddr;
    cInt32    dlCount;
    cBool     clkContinuous;
}CHAL_DSI_HANDLE_st, *CHAL_DSI_HANDLE;


// NO OF DATA LINES SUPPORTED
#define DSI_DL_COUNT                1
// NO OF CONTROLLERs
#define DSI_DEV_COUNT               2

//*****************************************************************************
// Local Variables
//*****************************************************************************
static CHAL_DSI_HANDLE_st dsi_dev [ DSI_DEV_COUNT ];       

//*****************************************************************************
// Local Functions
//*****************************************************************************
//SETs REGISTER BIT FIELD; VALUE IS 0 BASED 
#define DSI_REG_FIELD_SET(r,f,d)        \
    ( ((BRCM_REGTYPE(r))(d) << BRCM_FIELDSHIFT(r,f) ) & BRCM_FIELDMASK(r,f) )

//SETs REGISTER BITs Defined WITH MASK
#define DSI_REG_WRITE_MASKED(b,r,m,d)   \
    ( BRCM_WRITE_REG(b,r,(BRCM_READ_REG(b,r) & (~m)) | d) )


// DSI COMMAND TYPE
#define CMND_CTRL_CMND_PKT          0               
#define CMND_CTRL_CMND_PKTwBTA      1               
#define CMND_CTRL_TRIG              2               
#define CMND_CTRL_BTA               3               
                   
// DSI PACKET SOURCE                                       
#define DSI_PKT_SRC_CMND_FIFO       0               
#define DSI_PKT_SRC_DE0             1               
#define DSI_PKT_SRC_DE1             2               
#define DSI_PKT_SRC_INV             3                

#define DSI_DT_VC_MASK              0x000000C0      
#define DSI_DT_VC_MASK_SHIFT        6               
#define DSI_DT_MASK                 0x0000003F      

// D-PHY Timing Record
typedef struct
{
    char*   name;           
    UInt32  timeBase;         
    UInt32  mode;
    UInt32  time_lpx;       // esc_clk LP counters are speced using this member
    UInt32  time_min1_ns;
    UInt32  time_min1_ui;
    UInt32  time_min2_ns;
    UInt32  time_min2_ui;
    UInt32  time_max_ns;
    UInt32  time_max_ui;
    UInt32  counter_min;
    UInt32  counter_max;
    UInt32  counter_step;
    UInt32  counter_offs;
    UInt32  counter;        // calculated value of the register
    float   period;         // dbg
} DSI_COUNTER;

//--- Counter Mode Flags
// record has MAX value set
#define DSI_C_HAS_MAX        1  
// record MIN value is MAX of 2 values 
#define DSI_C_MIN_MAX_OF_2   2  

//--- Counter timeBase Flags
// ESC2LPDT entry - must be first record
#define DSI_C_TIME_ESC2LPDT  0   
// counts in HS Bit Clk
#define DSI_C_TIME_HS        1   
// counts in ESC CLKs
#define DSI_C_TIME_ESC       2   

// DSI Core Timing Registers
typedef enum
{
    DSI_C_ESC2LP_RATIO = 0,
    DSI_C_HS_INIT,    
    DSI_C_HS_WAKEUP,  
    DSI_C_LP_WAKEUP,  
    DSI_C_HS_CLK_PRE,
    DSI_C_HS_CLK_PREPARE,
    DSI_C_HS_CLK_ZERO,
    DSI_C_HS_CLK_POST,
    DSI_C_HS_CLK_TRAIL,
    DSI_C_HS_LPX,     
    DSI_C_HS_PRE,     
    DSI_C_HS_ZERO,       
    DSI_C_HS_TRAIL,     
    DSI_C_HS_EXIT,       
    DSI_C_LPX,               
    DSI_C_LP_TA_GO,     
    DSI_C_LP_TA_SURE, 
    DSI_C_LP_TA_GET,
    DSI_C_MAX,   
} DSI_TIMING_C;


//--------------------------------------------------------------
// D-PHY 0.92 Timing
//   o changed T-INIT to 1ms(from1us) to fix LM2550 
//     start-up issues
//--------------------------------------------------------------
DSI_COUNTER  dsi_dphy_0_92[] =
{                         
    // LP Data Symbol Rate Calc - MUST BE FIRST RECORD
    { "ESC2LP_RATIO" , DSI_C_TIME_ESC2LPDT, 0,
          0,  0,0,0,0,  0,0,  0,0x0000003F,  1,1,  0 },

    // min = 100[us] + 0[UI]         
    { "HS_INIT"      , DSI_C_TIME_HS,  0,
          0,  1000000,0,0,0,  0,0,  0,0x00FFFFFF,  0,0,  0 },      

    // min = 1[ms] + 0[UI]   
    { "HS_WAKEUP"    , DSI_C_TIME_HS,  0,
          0,  1000000,0,0,0,  0,0,  0,0x00FFFFFF,  0,0,  0 },      

    // min = 1[ms] + 0[UI]   
    { "LP_WAKEUP"    , DSI_C_TIME_ESC, 0,
          0,  1000000,0,0,0,  0,0,  0,0x00FFFFFF,  1,1, 0 },      
       
    // min = 0[ns] + 8[UI]
    { "HS_CLK_PRE"   , DSI_C_TIME_HS,  0,
          0,  0,8,0,0,  0,0,  0,0x000001FF,  0,0,  0 },      

    // min = 38[ns] + 0[UI]   max= 95[ns] + 0[UI]   
    { "HS_CLK_PREPARE"   , DSI_C_TIME_HS,  DSI_C_HAS_MAX,
          0,  38,0,0,0,  95,0,  0,0x000001FF,  0,0,  0 },      

    // min = 262[ns] + 0[UI]        
    { "HS_CLK_ZERO"  , DSI_C_TIME_HS,  0,
          0,  262,0,0,0,  0,0,  0,0x000001FF,  0,0,  0 },      

    // min =  60[ns] + 52[UI]       
    { "HS_CLK_POST"  , DSI_C_TIME_HS,  0,
          0,  60,52,0,0,  0,0,  0,0x000001FF,  0,0,  0 },      

    // min =  60[ns] + 0[UI]        
    { "HS_CLK_TRAIL" , DSI_C_TIME_HS,  0,
          0,  60,0,0,0,  0,0,  0,0x000001FF,  0,0,  0 },      

    // min =  50[ns] + 0[UI]        
    { "HS_LPX"       , DSI_C_TIME_HS,  0,
          0,  50,0,0,0,  0,0,  0,0x000001FF,  0,0,  0 },      

    // min = 40[ns] + 4[UI]      max= 85[ns] + 6[UI]        
    { "HS_PRE"       , DSI_C_TIME_HS,  DSI_C_HAS_MAX,
          0,  40,4,0,0,  85,6,  0,0x000001FF,  0,0,  0 },      

    // min = 105[ns] + 6[UI]        
    { "HS_ZERO"      , DSI_C_TIME_HS,  0,
          0,  105,6,0,0, 0,0,  0,0x000001FF,  0,0, 0 },      

    // min = max(0[ns]+32[UI],60[ns]+16[UI])  n=4     
    { "HS_TRAIL"     , DSI_C_TIME_HS,  DSI_C_MIN_MAX_OF_2,
          0,  0,32,60,16, 0,0, 0,0x000001FF,  0,0,  0 },      

    // min = 100[ns] + 0[UI]         
    { "HS_EXIT"      , DSI_C_TIME_HS,  0,
          0,  100,0,0,0,  0,0,  0,0x000001FF,  0,0,  0 },      

    // min = 50[ns] + 0[UI]       
    // LP esc counters are speced in LP LPX units. LP_LPX is calculated by 
    // chal_dsi_set_timing and equals LP data clock 
    { "LPX"          , DSI_C_TIME_ESC, 0,
          1,  0,0,0,0,  0,0, 0,0x000000FF,  1,1,  0 },      

    // min = 4*[Tlpx]  max = 4[Tlpx], set to 4         
    { "LP-TA-GO"     , DSI_C_TIME_ESC, 0,
          4,  0,0,0,0,  0,0, 0,0x000000FF,  1,1,  0 },      

    // min = 1*[Tlpx]  max = 2[Tlpx], set to 2         
    { "LP-TA-SURE"   , DSI_C_TIME_ESC, 0,
          2,  0,0,0,0,  0,0,  0,0x000000FF,  1,1,  0 },      

    // min = 5*[Tlpx]  max = 5[Tlpx], set to 5         
    { "LP-TA-GET"    , DSI_C_TIME_ESC, 0,
          5,  0,0,0,0,  0,0,  0,0x000000FF,  1,1,  0 },      
};


//*****************************************************************************
//
// Function Name: chal_dsi_init
//
// Description:   Initialize DSI Controller  and software interface
//
//*****************************************************************************
CHAL_HANDLE chal_dsi_init ( cUInt32 baseAddr, pCHAL_DSI_INIT dsiInit )
{
    CHAL_DSI_HANDLE pDev = NULL;
    cUInt32         i;
         
    chal_dprintf (CDBG_INFO, "chal_dsi_init\n");

    if ( dsiInit->dlCount > DSI_DL_COUNT )
    {
        chal_dprintf (CDBG_ERRO, "ERROR: chal_dsi_init: DataLine Count\n");
        return (CHAL_HANDLE) NULL;
    }
    
    for ( i=0; i < DSI_DEV_COUNT; i++ )
    {
        if( dsi_dev[i].init )
        {
            if (dsi_dev[i].baseAddr == baseAddr)
            { 
                pDev = (CHAL_DSI_HANDLE) &dsi_dev[i];
                break;
            }  
        }        
        else
        {
            pDev = (CHAL_DSI_HANDLE) &dsi_dev[i];
            pDev->init          = TRUE;
            pDev->baseAddr      = baseAddr;
            pDev->dlCount       = dsiInit->dlCount;
            pDev->clkContinuous = dsiInit->clkContinuous;
            break;
        }
    }
    
    if( pDev == NULL )
    {
        chal_dprintf (CDBG_ERRO, "ERROR: chal_dsi_init: failed ...\n");
    }    
    
    return (CHAL_HANDLE) pDev;
}


//****************************************************************************
//
//  Function Name:  chal_dsi_phy_state
//                  
//  Description:    PHY State Control
//                  Controls Clock & Data Line States 
//
//  CLK lane cannot be directed to STOP state. It is normally in stop state 
//  if cont clock is not enabled.
//  CLK lane will not transition to ULPS if cont clk is enabled.
//  When ULPS is requested and cont clock was enabled it will be disabled
//  at same time as ULPS is activated, and re-enabled upon ULPS exit
//                  
//****************************************************************************
cVoid  chal_dsi_phy_state ( 
    CHAL_HANDLE             handle, 
    CHAL_DSI_PHY_STATE_t    state 
    )
{
    CHAL_DSI_HANDLE pDev   = (CHAL_DSI_HANDLE) handle;
    cUInt32         regMask = 0;
    cUInt32         regVal  = 0;
    
    regMask =  DSI_REG_FIELD_SET( DSI1_PHYC, FORCE_TXSTOP_0, 1 )
             | DSI_REG_FIELD_SET( DSI1_PHYC, TXULPSCLK     , 1 )    
             | DSI_REG_FIELD_SET( DSI1_PHYC, TXULPSESC_0   , 1 )
             | DSI_REG_FIELD_SET( DSI1_PHYC, TX_HSCLK_CONT , 1 );

    switch ( state )
    {
    case PHY_TXSTOP:
        regVal =  DSI_REG_FIELD_SET( DSI1_PHYC, FORCE_TXSTOP_0, 1 );
        break;
    case PHY_ULPS:
        regVal =  DSI_REG_FIELD_SET( DSI1_PHYC, TXULPSCLK  , 1 )
                | DSI_REG_FIELD_SET( DSI1_PHYC, TXULPSESC_0, 1 );
        break;
    case PHY_CORE:
    default:
        if ( pDev->clkContinuous )
            regVal  = DSI_REG_FIELD_SET( DSI1_PHYC, TX_HSCLK_CONT , 1 );
        else
            regVal  = 0;
        break;
    }

    DSI_REG_WRITE_MASKED( pDev->baseAddr, DSI1_PHYC, regMask, regVal );
}


//****************************************************************************
//
//  Function Name:  chal_dsi_set_afe_off
//                  
//  Description:    Power Down PHY AFE (Analog Front End)
//                  
//****************************************************************************
cVoid  chal_dsi_phy_afe_off ( CHAL_HANDLE handle )
{
    CHAL_DSI_HANDLE pDev   = (CHAL_DSI_HANDLE) handle;
    cUInt32         afeVal  = 0;
    cUInt32         afeMask = 0;


    afeMask =  DSI1_PHY_AFEC0_PD_MASK
             | DSI1_PHY_AFEC0_PD_BG_MASK;

    afeVal =   DSI_REG_FIELD_SET( DSI1_PHY_AFEC0, PD       , 1 )  // Pwr Down AFE
             | DSI_REG_FIELD_SET( DSI1_PHY_AFEC0, PD_BG    , 1 ); // Pwr Down BG

    DSI_REG_WRITE_MASKED ( pDev->baseAddr, DSI1_PHY_AFEC0, afeMask, afeVal );
}

//****************************************************************************
//
//  Function Name:  chal_dsi_phy_afe_on
//                  
//  Description:    Configure & Enable PHY-AFE  (Analog Front End)
//                  
//****************************************************************************
cVoid chal_dsi_phy_afe_on ( CHAL_HANDLE handle, pCHAL_DSI_AFE_CFG afeCfg )
{
    CHAL_DSI_HANDLE pDev   = (CHAL_DSI_HANDLE) handle;
    cUInt32         afeVal  = 0;
    cUInt32         afeMask = 0;
    //cUInt32         i;

    afeMask =  DSI1_PHY_AFEC0_CTATADJ_MASK
             | DSI1_PHY_AFEC0_PTATADJ_MASK
             | DSI1_PHY_AFEC0_DDRCLK_EN_MASK
             | DSI1_PHY_AFEC0_PD_BG_MASK
             | DSI1_PHY_AFEC0_PD_MASK
             | DSI1_PHY_AFEC0_RESET_MASK 
             | DSI1_PHY_AFEC0_DDR2CLK_EN_MASK 
             | DSI1_PHY_AFEC0_IDR_DLANE0_MASK                    
             | DSI1_PHY_AFEC0_IDR_CLANE_MASK;                         

    afeVal =   DSI_REG_FIELD_SET( DSI1_PHY_AFEC0, CTATADJ   , afeCfg->afeCtaAdj )
             | DSI_REG_FIELD_SET( DSI1_PHY_AFEC0, PTATADJ   , afeCfg->afePtaAdj )
             | DSI_REG_FIELD_SET( DSI1_PHY_AFEC0, IDR_CLANE , afeCfg->afeClkIdr )
             | DSI_REG_FIELD_SET( DSI1_PHY_AFEC0, IDR_DLANE0, afeCfg->afeDlIdr  );

//    if( !afeCfg->afeBandGapOn ) 
//        afeVal |= DSI_REG_FIELD_SET( DSI1_PHY_AFEC0, PD_BG, 1 );
// 
//    if( afeCfg->afeDs2xClkEna )
//        afeVal |= DSI_REG_FIELD_SET( DSI1_PHY_AFEC0, DDRCLK_EN, 1 );

    rmb();
    afeVal |= DSI_REG_FIELD_SET( DSI1_PHY_AFEC0, PD_BG     , 0 );
    afeVal |= DSI_REG_FIELD_SET( DSI1_PHY_AFEC0, PD        , 0 );
    // for now, enable all clock outputs
    afeVal |= DSI_REG_FIELD_SET( DSI1_PHY_AFEC0, DDRCLK_EN , 1 );
    afeVal |= DSI_REG_FIELD_SET( DSI1_PHY_AFEC0, DDR2CLK_EN, 1 );

    afeVal |= DSI_REG_FIELD_SET( DSI1_PHY_AFEC0, RESET     , 1 );

    // PWR-UP & Reset
    wmb();
    DSI_REG_WRITE_MASKED ( pDev->baseAddr, DSI1_PHY_AFEC0, afeMask, afeVal );
    
    //for ( i=0; i<100; i++ ) {}
    CHAL_DELAY_MS(2);
    
    // remove reset
    wmb();
    BRCM_WRITE_REG_FIELD ( pDev->baseAddr, DSI1_PHY_AFEC0, RESET, 0 );
}

//*****************************************************************************
//
// Function Name:  chalDsiTimingDivAndRoundUp
// 
// Description:    DSI Timing Counters - Divide & RoundUp Utility
//                 Checks for Counter Value Overflow
//*****************************************************************************
static Boolean chalDsiTimingDivAndRoundUp ( 
    DSI_COUNTER *       pDsiC,
    cUInt32             i,         // DSI counter index
    float               dividend, 
    float               divisor 
    )
{
	float	 counter_f;
	cUInt32 counter;
	
	counter_f	= dividend / divisor;
	counter	= (UInt32)counter_f;
	
	if( counter_f != (float)counter ) 
		 counter++;
	 
	if ( (counter % pDsiC[i].counter_step) != 0 )
		 counter += pDsiC[i].counter_step;
	
	counter  = counter & (~(pDsiC[i].counter_step - 1));
	
	counter -= pDsiC[i].counter_offs;
	
	pDsiC[i].counter = counter;
	
	if ( counter > pDsiC[i].counter_max ) 
	{
		 chal_dprintf ( CDBG_ERRO, "[cHAL DSI] chalDsiTimingDivAndRoundUp: "
			 "%s counter value overflow\n\r",  pDsiC[i].name );
		 return ( FALSE );
	} 
	else
	{
		 return ( TRUE );
	}
}


//*****************************************************************************
//
// Function Name:  chal_dsi_set_timing
// 
// Description:    Calculate Values Of DSI Timing Counters
//
//                 <in> escClk_MHz      ESC CLK after divider
//                 <in> hsBitRate_Mbps  HS Bit Rate ( eq to DSI PLL/dsiPllDiv ) 
//                 <in> lpBitRate_Mbps  LP Bit Rate, Max 10 Mbps 
//                   
//*****************************************************************************
cBool chal_dsi_set_timing ( 
    CHAL_HANDLE         handle,
    cUInt32             DPHY_SpecRev,
    CHAL_DSI_CLK_SEL_t  coreClkSel,
    float               escClk_MHz,              
    float               hsBitRate_Mbps,  
    float               lpBitRate_Mbps  
    )
{
    Boolean res = FALSE;
    
    float           time_min;
    float           time_min1;
    float           time_min2;
    float           time_max;
    float           period;
    float           ui_ns;
    float           escClk_ns;
    float           lp_clk_mhz;
    cUInt32         i;
    DSI_COUNTER *   pDsiC;
    CHAL_DSI_HANDLE pDev;
    cUInt32         counter_offs;
    cUInt32         counter_step;
    cUInt32         lp_lpx_ns;
    
    pDev        = (CHAL_DSI_HANDLE) handle;
    ui_ns       = 1 / hsBitRate_Mbps  * 1000;
    escClk_ns   = 1 / escClk_MHz * 1000;

    switch ( DPHY_SpecRev )
    {
        case 1:
            pDsiC = &dsi_dphy_0_92[0];
            break;
        default:
            return ( FALSE );
    }

    // figure step & offset for HS counters
    if ( coreClkSel == CHAL_DSI_BIT_CLK_DIV_BY_8 )
    {
        counter_offs = 8;
        counter_step = 8;
    } 
    else if ( coreClkSel == CHAL_DSI_BIT_CLK_DIV_BY_4 )
    {
        counter_offs = 4;
        counter_step = 4;
    }
    else
    {
        counter_offs = 2;
        counter_step = 2;
    }
    
    // init offset & step for HS counters     
    for( i=1; i < DSI_C_MAX; i++ )
    {
        // Period_units [ns]
        if ( pDsiC[i].timeBase & DSI_C_TIME_HS )
        {
            pDsiC[i].counter_offs = counter_offs;
            pDsiC[i].counter_step = counter_step;
        }
    }
    
    
    // LP clk (LP Symbol Data Rate) = esc_clk / esc2lp_ratio
    // calculate esc2lp_ratio
    if( ! chalDsiTimingDivAndRoundUp( pDsiC, DSI_C_ESC2LP_RATIO, 
        (float)escClk_MHz, (float)lpBitRate_Mbps * 2) )
    {
        return ( FALSE );
    }            
    
    // actual lp clock 
    lp_clk_mhz =   escClk_MHz 
                / (   pDsiC [ DSI_C_ESC2LP_RATIO ].counter 
                    + pDsiC [ DSI_C_ESC2LP_RATIO ].counter_offs ); 
    
    // lp_esc_clk == lp_data_clock
    lp_lpx_ns = (UInt32)( 1000 / lp_clk_mhz );
    // set LP LPX to be equal to LP bit rate
    
    // set time_min_ns for LP esc_clk counters
    pDsiC[DSI_C_LPX].time_min1_ns        = 
        pDsiC[DSI_C_LPX].time_lpx * lp_lpx_ns;
    pDsiC[DSI_C_LP_TA_GO].time_min1_ns   = 
        pDsiC[DSI_C_LP_TA_GO].time_lpx * lp_lpx_ns;
    pDsiC[DSI_C_LP_TA_SURE].time_min1_ns = 
        pDsiC[DSI_C_LP_TA_SURE].time_lpx * lp_lpx_ns;
    pDsiC[DSI_C_LP_TA_GET].time_min1_ns  = 
        pDsiC[DSI_C_LP_TA_GET].time_lpx * lp_lpx_ns;
    
    
    // start from 1, skip [0]=esc2lp_ratio   
    for( i=1; i < DSI_C_MAX; i++ )
    {
        // Period_min1 [ns]
        time_min1 =  (float)pDsiC[i].time_min1_ns 
                   + (float)pDsiC[i].time_min1_ui * ui_ns;

        // Period_min2 [ns]
        if( pDsiC[i].mode & DSI_C_MIN_MAX_OF_2 )
            time_min2 = (float)pDsiC[i].time_min2_ns 
                      + (float)pDsiC[i].time_min2_ui * ui_ns;
        else
            time_min2 = 0;
        
        // Period_min [ns] = max(min1, min2)       
        if( time_min1 >= time_min2 )
            time_min = time_min1;
        else
            time_min = time_min2;    

        // Period_max [ns]
        if( pDsiC[i].mode & DSI_C_HAS_MAX )
            time_max = (float)pDsiC[i].time_max_ns 
                     + (float)pDsiC[i].time_max_ui * ui_ns;
        else
            time_max = 0;
        
        // Period_units [ns]
        if ( pDsiC[i].timeBase & DSI_C_TIME_HS )
            period = ui_ns;
        else if ( pDsiC[i].timeBase & DSI_C_TIME_ESC )
            period = escClk_ns;
        else 
            period = 0;
        
        
        pDsiC[i].period = period;
        
        if( period != 0 )
        {
            res = chalDsiTimingDivAndRoundUp ( pDsiC, i, time_min, period );
            if( !res )
                return ( res );
                
            if( pDsiC[i].mode & DSI_C_HAS_MAX )
            {
                if( ((float)pDsiC[i].counter * period ) > time_max )
                {
                    chal_dprintf ( CDBG_ERRO, "[cHAL DSI] chal_dsi_set_timing: "
                        "%s violates MAX D-PHY Spec allowed value\n\r", 
                        pDsiC[i].name );
                    return ( FALSE );
                }
            }
        }              
    }    


    for( i=0; i < DSI_C_MAX; i++ )
    {
        if ( pDsiC[i].timeBase == DSI_C_TIME_ESC2LPDT )
        {
            chal_dprintf ( CDBG_ERRO, "[cHAL DSI] chal_dsi_set_timing: "
                "%14s %7d => LP clk %5.2f[Mhz]\n\r", 
                pDsiC[i].name, pDsiC[i].counter,
                escClk_MHz / (pDsiC[i].counter + pDsiC[i].counter_offs));
        } 
        else
        {
            chal_dprintf ( CDBG_ERRO, "[cHAL DSI] chal_dsi_set_timing: "
                "%14s %7d => %10.2f[ns]\n\r", 
                pDsiC[i].name, 
                pDsiC[i].counter, 
                ((float)pDsiC[i].counter + pDsiC[i].counter_offs) 
                * pDsiC[i].period );
        }
    }

    chal_dprintf (CDBG_ERRO, "\r\n[cHAL DSI] chal_dsi_set_timing: "
        "HS_DATA_RATE %6.2f[Mbps]\r\n",  hsBitRate_Mbps );

    chal_dprintf (CDBG_ERRO, "[cHAL DSI] chal_dsi_set_timing: "
        "LP_DATA_RATE %6.2f[Mbps]\n\r", lp_clk_mhz / 2 );

    // set ESC 2 LPDT ratio
    BRCM_WRITE_REG_FIELD ( pDev->baseAddr, DSI1_PHYC     , 
        ESC_CLK_LPDT, pDsiC [ DSI_C_ESC2LP_RATIO ].counter );

    // HS_DLT5  INIT  
    BRCM_WRITE_REG_FIELD ( pDev->baseAddr, DSI1_HS_DLT5  , 
        HS_INIT   , pDsiC [ DSI_C_HS_INIT ].counter );
    
    // HS_CLT2  ULPS WakeUp    
    BRCM_WRITE_REG_FIELD ( pDev->baseAddr, DSI1_HS_CLT2  , 
        HS_WUP    , pDsiC [ DSI_C_HS_WAKEUP ].counter);
    // LP_DLT7  ULPS WakeUp 
    BRCM_WRITE_REG_FIELD ( pDev->baseAddr, DSI1_LP_DLT7  , 
        LP_WUP    , pDsiC [ DSI_C_LP_WAKEUP ].counter );


    // HS CLK - HS_CLT0 reg
    BRCM_WRITE_REG_FIELD ( pDev->baseAddr, DSI1_HS_CLT0 , 
        HS_CZERO  , pDsiC [ DSI_C_HS_CLK_ZERO ].counter );
    BRCM_WRITE_REG_FIELD ( pDev->baseAddr, DSI1_HS_CLT0  , 
        HS_CPRE   , pDsiC [ DSI_C_HS_CLK_PRE ].counter );
    BRCM_WRITE_REG_FIELD ( pDev->baseAddr, DSI1_HS_CLT0  , 
        HS_CPREP  , pDsiC [ DSI_C_HS_CLK_PREPARE ].counter );
    
    // HS CLK - HS_CLT1 reg
    BRCM_WRITE_REG_FIELD ( pDev->baseAddr, DSI1_HS_CLT1, 
        HS_CTRAIL , pDsiC [ DSI_C_HS_CLK_TRAIL ].counter );
    BRCM_WRITE_REG_FIELD ( pDev->baseAddr, DSI1_HS_CLT1 , 
        HS_CPOST  , pDsiC [ DSI_C_HS_CLK_POST ].counter );

    // HS DATA HS_DLT3 REG
    BRCM_WRITE_REG_FIELD ( pDev->baseAddr, DSI1_HS_DLT3  , 
        HS_EXIT   , pDsiC [ DSI_C_HS_EXIT ].counter  );
    BRCM_WRITE_REG_FIELD ( pDev->baseAddr, DSI1_HS_DLT3  , 
        HS_ZERO   , pDsiC [ DSI_C_HS_ZERO ].counter );
    BRCM_WRITE_REG_FIELD ( pDev->baseAddr, DSI1_HS_DLT3  , 
        HS_PRE    , pDsiC [ DSI_C_HS_PRE ].counter );
    
    // HS DATA HS_DLT4 REG
    // !!! HS_ANLAT, new in HERA, for now init to 0 (DEF)   
    BRCM_WRITE_REG_FIELD ( pDev->baseAddr, DSI1_HS_DLT4 , 
        HS_ANLAT  , 0  );
    BRCM_WRITE_REG_FIELD ( pDev->baseAddr, DSI1_HS_DLT4 , 
        HS_TRAIL  , pDsiC [ DSI_C_HS_TRAIL ].counter );
    BRCM_WRITE_REG_FIELD ( pDev->baseAddr, DSI1_HS_DLT4 , 
        HS_LPX    , pDsiC [ DSI_C_HS_LPX ].counter );
    
    // LP_DLT6 REG
    BRCM_WRITE_REG_FIELD ( pDev->baseAddr, DSI1_LP_DLT6    , 
        TA_GET    , pDsiC [ DSI_C_LP_TA_GET ].counter );
    BRCM_WRITE_REG_FIELD ( pDev->baseAddr, DSI1_LP_DLT6    , 
        TA_SURE   , pDsiC [ DSI_C_LP_TA_SURE ].counter );
    BRCM_WRITE_REG_FIELD ( pDev->baseAddr, DSI1_LP_DLT6    , 
        TA_GO     , pDsiC [ DSI_C_LP_TA_GO ].counter);
    BRCM_WRITE_REG_FIELD ( pDev->baseAddr, DSI1_LP_DLT6    , 
        LPX       , pDsiC [ DSI_C_LPX ].counter );
        

    return ( TRUE );
}


//****************************************************************************
//
//  Function Name:  chal_dsi_off
//                  
//  Description:    Disable DSI core 
//                  
//****************************************************************************
cVoid  chal_dsi_off ( CHAL_HANDLE handle )
{
    CHAL_DSI_HANDLE pDev   = (CHAL_DSI_HANDLE) handle;

    //DSI_PHYC_PHY_CLANE_EN   = 0;
    //DSI_PHYC_PHY_DLANE0_EN  = 0;
    BRCM_WRITE_REG ( pDev->baseAddr, DSI1_PHYC, 0 );
    
    // DSI_CTRL_DSI_EN = 0
    BRCM_WRITE_REG_FIELD ( pDev->baseAddr, DSI1_CTRL, DSI_EN, 0 );
}


//****************************************************************************
//
//  Function Name:  chal_dsi_te_mode
//                  
//  Description:    Set TE SYNC mode
//                  
//****************************************************************************
cVoid chal_dsi_te_mode ( CHAL_HANDLE handle, CHAL_DSI_TE_MODE_t teMode )
{
    CHAL_DSI_HANDLE pDev   = (CHAL_DSI_HANDLE) handle;
    
    BRCM_WRITE_REG_FIELD ( pDev->baseAddr, DSI1_CTRL, TE_TRIGC, teMode );
}

//****************************************************************************
//
//  Function Name:  chal_dsi_on
//                  
//  Description:    
//                  
//****************************************************************************
cVoid chal_dsi_on ( CHAL_HANDLE handle, pCHAL_DSI_MODE dsiMode )
{
    
    CHAL_DSI_HANDLE pDev = (CHAL_DSI_HANDLE) handle;
    cUInt32         mask = 0;
    cUInt32         ctrl = 0;
    
    // DSI-CTRL  Configure Mode & enable the core  
    mask =   DSI_REG_FIELD_SET( DSI1_CTRL, DISP_CRCC      , 1 )
           | DSI_REG_FIELD_SET( DSI1_CTRL, DISP_ECCC      , 1 )
           | DSI_REG_FIELD_SET( DSI1_CTRL, RX_LPDT_EOT_EN , 1 )
           | DSI_REG_FIELD_SET( DSI1_CTRL, LPDT_EOT_EN    , 1 )
           | DSI_REG_FIELD_SET( DSI1_CTRL, HSDT_EOT_EN    , 1 )
           | DSI_REG_FIELD_SET( DSI1_CTRL, DSI_EN         , 1 )
           | DSI1_CTRL_HS_CLKC_MASK;

    ctrl = dsiMode->clkSel << DSI1_CTRL_HS_CLKC_SHIFT;

    rmb();
    if( dsiMode->enaRxCrc )
        ctrl |= DSI_REG_FIELD_SET( DSI1_CTRL, DISP_CRCC, 1 );
    if( dsiMode->enaRxEcc )
        ctrl |= DSI_REG_FIELD_SET( DSI1_CTRL, DISP_ECCC, 1 );
    
    if( dsiMode->enaHsTxEotPkt )
        ctrl |= DSI_REG_FIELD_SET( DSI1_CTRL, HSDT_EOT_EN, 1 );
    
    if( dsiMode->enaLpTxEotPkt )
        ctrl |= DSI_REG_FIELD_SET( DSI1_CTRL, LPDT_EOT_EN, 1 );
    
    if( dsiMode->enaLpRxEotPkt )
        ctrl |= DSI_REG_FIELD_SET( DSI1_CTRL, RX_LPDT_EOT_EN, 1 );

    ctrl |= DSI_REG_FIELD_SET( DSI1_CTRL, DSI_EN, 1 );
    
    wmb();
    DSI_REG_WRITE_MASKED ( pDev->baseAddr, DSI1_CTRL, mask, ctrl );
       
    // PHY-C  Configure & Enable D-PHY Interface
    mask = 0;
    ctrl = 0;

    mask =   DSI1_PHYC_TX_HSCLK_CONT_MASK 
           | DSI1_PHYC_PHY_CLANE_EN_MASK  
           | DSI1_PHYC_PHY_DLANE0_EN_MASK; 

    rmb();
    if ( dsiMode->enaContClock )
        ctrl |= DSI_REG_FIELD_SET( DSI1_PHYC, TX_HSCLK_CONT, 1 );
    
    ctrl |= DSI_REG_FIELD_SET( DSI1_PHYC, PHY_CLANE_EN , 1 );
    ctrl |= DSI_REG_FIELD_SET( DSI1_PHYC, PHY_DLANE0_EN, 1 );

    wmb();
    DSI_REG_WRITE_MASKED ( pDev->baseAddr, DSI1_PHYC, mask, ctrl );
}



//****************************************************************************
//
//  Function Name:  chal_dsi_ena_int
//                  
//  Description:    Enable(b'1b) | Disable(b'0') DSI Interrupts
//                  
//****************************************************************************
cVoid  chal_dsi_ena_int ( CHAL_HANDLE handle, cUInt32 intMask )
{
    CHAL_DSI_HANDLE pDev   = (CHAL_DSI_HANDLE) handle;
    
    BRCM_WRITE_REG ( pDev->baseAddr, DSI1_INT_EN, intMask );
}

//****************************************************************************
//
//  Function Name:  chal_dsi_get_int
//                  
//  Description:    Get Pending Interrupts
//                  
//****************************************************************************
cUInt32  chal_dsi_get_int ( CHAL_HANDLE handle )
{
    CHAL_DSI_HANDLE pDev   = (CHAL_DSI_HANDLE) handle;

    return ( BRCM_READ_REG ( pDev->baseAddr, DSI1_INT_STAT ) );
}

//****************************************************************************
//
//  Function Name:  chal_dsi_clr_int
//                  
//  Description:    Clear Pending Interrupt
//                  
//****************************************************************************
cVoid  chal_dsi_clr_int ( CHAL_HANDLE handle, cUInt32 intMask )
{
    CHAL_DSI_HANDLE pDev = (CHAL_DSI_HANDLE) handle;
    
    BRCM_WRITE_REG ( pDev->baseAddr, DSI1_INT_STAT, intMask );
}


//****************************************************************************
//
//  Function Name:  chal_dsi_clr_fifo
//                  
//  Description:    Clear Sleleceted DSI FIFOs
//                  
//****************************************************************************
cVoid  chal_dsi_clr_fifo ( CHAL_HANDLE handle, cUInt32  fifoMask )
{
    CHAL_DSI_HANDLE pDev = (CHAL_DSI_HANDLE) handle;
    
    DSI_REG_WRITE_MASKED ( pDev->baseAddr, DSI1_CTRL, fifoMask, fifoMask );
}


//****************************************************************************
//
//  Function Name:  chal_dsi_get_status
//                  
//  Description:    Int Status is collection of Int event (set until cleared)
//                  and status (real time state) flags
//                  
//****************************************************************************
cUInt32 chal_dsi_get_status ( CHAL_HANDLE handle )
{
    CHAL_DSI_HANDLE pDev   = (CHAL_DSI_HANDLE) handle;
    
    return ( BRCM_READ_REG ( pDev->baseAddr, DSI1_STAT ) );
}

//****************************************************************************
//
//  Function Name:  chal_dsi_clr_status
//                  
//  Description:    Int Status is collection of Int event (set until cleared)
//                  and status (real time state) flags
//                  1 - clears event bits
//****************************************************************************
cVoid chal_dsi_clr_status ( CHAL_HANDLE handle, cUInt32 statMask )
{
    CHAL_DSI_HANDLE pDev = (CHAL_DSI_HANDLE) handle;
    
    BRCM_WRITE_REG ( pDev->baseAddr, DSI1_STAT, statMask );
}

 

//****************************************************************************
//
//  Function Name:  chal_dsi_send_bta
//                  
//  Description:    Turn Bus Around
//                  
//****************************************************************************
cVoid chal_dsi_send_bta (
    CHAL_HANDLE handle, 
    cUInt8      txEng    
    )   
{
    CHAL_DSI_HANDLE pDev   = (CHAL_DSI_HANDLE) handle;
    cUInt32         pktc   = 0;
    
    pktc |= DSI_REG_FIELD_SET( DSI1_TXPKT1_C, CMD_CTRL   , CMND_CTRL_BTA );
    pktc |= DSI_REG_FIELD_SET( DSI1_TXPKT1_C, CMD_REPEAT , 1 );
    pktc |= DSI_REG_FIELD_SET( DSI1_TXPKT1_C, CMD_MODE   , 1 );  // LowPower
    pktc |= DSI_REG_FIELD_SET( DSI1_TXPKT1_C, CMD_TX_TIME, 
                CHAL_DSI_CMND_WHEN_BEST_EFFORT );
    pktc |= DSI_REG_FIELD_SET( DSI1_TXPKT1_C, DISPLAY_NO , 
                DSI_PKT_SRC_CMND_FIFO );     
    pktc |= DSI_REG_FIELD_SET( DSI1_TXPKT1_C, CMD_EN     , 1 );

    if ( txEng )
        BRCM_WRITE_REG ( pDev->baseAddr, DSI1_TXPKT2_C, pktc );
    else
        BRCM_WRITE_REG ( pDev->baseAddr, DSI1_TXPKT1_C, pktc );
        
}


//****************************************************************************
//
//  Function Name:  chal_dsi_send_trig
//                  
//  Description:    Send TRIGGER Message
//                  
//****************************************************************************
cVoid chal_dsi_send_trig ( CHAL_HANDLE handle, cUInt8 txEng, cUInt8 trig )   
{
    CHAL_DSI_HANDLE pDev   = (CHAL_DSI_HANDLE) handle;
    cUInt32         pktc   = 0;
    
    pktc |= DSI_REG_FIELD_SET( DSI1_TXPKT1_C, CMD_CTRL   , CMND_CTRL_TRIG );
    pktc |= DSI_REG_FIELD_SET( DSI1_TXPKT1_C, CMD_REPEAT , 1 );
    pktc |= DSI_REG_FIELD_SET( DSI1_TXPKT1_C, CMD_MODE   , 1 );  // LowPower
    
    pktc |= DSI_REG_FIELD_SET( DSI1_TXPKT1_C, CMD_TX_TIME, 
                CHAL_DSI_CMND_WHEN_BEST_EFFORT );
    pktc |= DSI_REG_FIELD_SET( DSI1_TXPKT1_C, DISPLAY_NO , 
                DSI_PKT_SRC_CMND_FIFO );     
    pktc |= DSI_REG_FIELD_SET( DSI1_TXPKT1_C, CMD_EN     , 1 );
    pktc |= DSI_REG_FIELD_SET( DSI1_TXPKT1_C, TRIG_CMD   , trig );

    if ( txEng )
        BRCM_WRITE_REG ( pDev->baseAddr, DSI1_TXPKT2_C, pktc );
    else
        BRCM_WRITE_REG ( pDev->baseAddr, DSI1_TXPKT1_C, pktc );
        
}

//****************************************************************************
//
//  Function Name:  chal_dsi_send_cmnd
//                  
//  Description:    Send   SHORT CMND - up to 2 Byte Parms - LSB first on wire
//                       | LONG  CMND - up to 8 Byte Parms - LSB first on wire
//                  
//****************************************************************************
CHAL_DSI_RES_t chal_dsi_send_cmnd (
    CHAL_HANDLE     handle, 
    cUInt8          txEng,   
    pCHAL_DSI_CMND  cmnd )   
{
    CHAL_DSI_HANDLE pDev   = (CHAL_DSI_HANDLE) handle;
    cUInt32      i;
    cUInt32      pktc   = 0;
    cUInt32      pkth   = 0;
    cUInt32      dsi_DT = 0;

    if( !cmnd->isLong && cmnd->msgLen > 2 )
        return ( CHAL_DSI_MSG_SIZE );
        
    if( cmnd->isLong && cmnd->msgLen > 8 )
        return ( CHAL_DSI_MSG_SIZE );

    if( cmnd->isLP   ) 
        pktc |= DSI_REG_FIELD_SET( DSI1_TXPKT1_C, CMD_MODE , 1 );
    if( cmnd->isLong ) 
        pktc |= DSI_REG_FIELD_SET( DSI1_TXPKT1_C, CMD_TYPE , 1 );
    
    if( cmnd->endWithBta ) 
        pktc |= DSI_REG_FIELD_SET( DSI1_TXPKT1_C, CMD_CTRL, 
                    CMND_CTRL_CMND_PKTwBTA );
    else    
        pktc |= DSI_REG_FIELD_SET( DSI1_TXPKT1_C, CMD_CTRL, 
                    CMND_CTRL_CMND_PKT );
    
    pktc |= DSI_REG_FIELD_SET( DSI1_TXPKT1_C, CMD_REPEAT , 1 );
    pktc |= DSI_REG_FIELD_SET( DSI1_TXPKT1_C, CMD_TX_TIME, cmnd->vmWhen );
    pktc |= DSI_REG_FIELD_SET( DSI1_TXPKT1_C, DISPLAY_NO , DSI_PKT_SRC_CMND_FIFO );     
    pktc |= DSI_REG_FIELD_SET( DSI1_TXPKT1_C, CMD_EN , 1 );

    dsi_DT = ((cmnd->vc & 0x00000003) << 6) | cmnd->dsiCmnd;

    if ( cmnd->isLong ) 
    {
        // for long, parm byteCount && byteCount from CMND DATA FIFO are same, 
        pkth =   DSI_REG_FIELD_SET ( DSI1_TXPKT1_H, WC_CDFIFO,  cmnd->msgLen )
               | DSI_REG_FIELD_SET ( DSI1_TXPKT1_H, WC_PARAM, cmnd->msgLen );
    } 
    else
    {
        // SHORT MSG IS FIXED IN SIZE (always 2  parms)
        pkth  = cmnd->msg[0];
        pkth |= (cmnd->msg[1] << 8); 
            
        pkth = DSI_REG_FIELD_SET ( DSI1_TXPKT1_H, WC_PARAM, pkth );
    }
  
    pkth |= dsi_DT;

    if ( txEng )
        BRCM_WRITE_REG ( pDev->baseAddr, DSI1_TXPKT2_H, pkth );
    else
        BRCM_WRITE_REG ( pDev->baseAddr, DSI1_TXPKT1_H, pkth );
    
    // if LONG PKT, fill CMND DATA FIFO 
    if( cmnd->isLong ) 
    {
        for( i=0; i < cmnd->msgLen; i++ )
        {
            //BRCM_WRITE_REG ( pDev->baseAddr, DSI_CMD_DATA_FIFO, cmnd->msg[i] );
            BRCM_WRITE_REG ( pDev->baseAddr, DSI1_TXPKT_CMD_FIFO, cmnd->msg[i] );
        }   
    }   

    if ( txEng )
        BRCM_WRITE_REG ( pDev->baseAddr, DSI1_TXPKT2_C, pktc );
    else
        BRCM_WRITE_REG ( pDev->baseAddr, DSI1_TXPKT1_C, pktc );
        
    return ( CHAL_DSI_OK );
}


//****************************************************************************
//
//  Function Name:  chal_dsi_read_reply
//                  
//  Description:    
//                  
//****************************************************************************

CHAL_DSI_RES_t chal_dsi_read_reply ( 
    CHAL_HANDLE         handle, 
    cUInt32             event,    // DSI core event flags
    pCHAL_DSI_REPLY     reply 
    )
{
    #define	DSI_DT_P2H_SH_ACK_ERR_RPT	(0x02)    
    #define	DSI_DT_P2H_SH_GEN_RD_1B	    (0x11)    
    #define	DSI_DT_P2H_DCS_SH_RD_1B	    (0x21)    

    CHAL_DSI_HANDLE pDev     = (CHAL_DSI_HANDLE) handle;
    cUInt32         rxStat;
    cUInt32         pkth;
    cUInt32         size;
    cUInt32         i;
    cUInt32         replyDT; 
    
    reply->type = 0;
    
    if (    (event & DSI1_STAT_RX1_PKT_MASK) 
         || (event & DSI1_STAT_PHY_RXTRIG_MASK) )
    {
        pkth = BRCM_READ_REG ( pDev->baseAddr, DSI1_RXPKT1_H );
    
        if( event & DSI1_STAT_PHY_RXTRIG_MASK )
        {
            reply->type    = RX_TYPE_TRIG;
            reply->trigger = pkth & DSI1_RXPKT1_H_DT_LP_CMD_MASK;
        } 
        else
        {
            rxStat  = 0;
            replyDT = pkth & DSI1_RXPKT1_H_DT_LP_CMD_MASK;
            
            // either DATA REPLY or ERROR STAT
            if( (replyDT & 0x3F) == DSI_DT_P2H_SH_ACK_ERR_RPT )
            {
                // ERROR STAT
                reply->type  |= RX_TYPE_ERR_REPLY;
                reply->errReportDt = replyDT;
        
                if( pkth & DSI1_RXPKT1_H_DET_ERR_MASK    ) 
                    rxStat |= ERR_RX_MULTI_BIT;     
                if( pkth & DSI1_RXPKT1_H_ECC_ERR_MASK    ) 
                    rxStat |= ERR_RX_ECC;           
                if( pkth & DSI1_RXPKT1_H_COR_ERR_MASK    ) 
                    rxStat |= ERR_RX_CORRECTABLE;   
                if( pkth & DSI1_RXPKT1_H_INCOMP_PKT_MASK ) 
                    rxStat |= ERR_RX_PKT_INCOMPLETE;
        
                reply->errReportRxStat = rxStat;
 
                if( (rxStat == 0) || (rxStat == ERR_RX_CORRECTABLE) )
                    reply->errReportRxStat |= ERR_RX_OK;
                                    
                                               
                // ALWAYS SHORT - NO CRC
                reply->errReport = (pkth & DSI1_RXPKT1_H_WC_PARAM_MASK) >> 
                    DSI1_RXPKT1_H_WC_PARAM_SHIFT;
            }
            else
            {
                // DATA REPLY
                reply->type    = RX_TYPE_READ_REPLY;
                //reply->readReplyDt = pkth & DSI_RX1_PKTH_DT_LP_CMD_MASK;
                reply->readReplyDt = replyDT;
                
                if( pkth & DSI1_RXPKT1_H_CRC_ERR_MASK    ) 
                    rxStat |= ERR_RX_CRC;           
                if( pkth & DSI1_RXPKT1_H_DET_ERR_MASK    ) 
                    rxStat |= ERR_RX_MULTI_BIT;     
                if( pkth & DSI1_RXPKT1_H_ECC_ERR_MASK    ) 
                    rxStat |= ERR_RX_ECC;           
                if( pkth & DSI1_RXPKT1_H_COR_ERR_MASK    ) 
                    rxStat |= ERR_RX_CORRECTABLE;   
                if( pkth & DSI1_RXPKT1_H_INCOMP_PKT_MASK ) 
                    rxStat |= ERR_RX_PKT_INCOMPLETE;

                reply->readReplyRxStat = rxStat;
                if( (rxStat == 0) || (rxStat == ERR_RX_CORRECTABLE))
                    reply->readReplyRxStat |= ERR_RX_OK;
            
                if ( pkth & DSI1_RXPKT1_H_PKT_TYPE_MASK )
                { 
                    // LONG, MAX 8 bytes of parms
                    size = (pkth & DSI1_RXPKT1_H_WC_PARAM_MASK) >> 8;
                    for ( i=0; i < size; i++ )
                    {   
                        reply->pReadReply[i] =      
                            //BRCM_READ_REG ( pDev->baseAddr, DSI_CMD_DATA_FIFO );
                            BRCM_READ_REG ( pDev->baseAddr, DSI1_RXPKT_FIFO );
                            
                    }    
                    reply->readReplySize = size; 
                }
                else
                { 
                    // SHORT  PKT - MAX 2 BYTEs, NO CRC
                    reply->pReadReply[0] = 
                        (pkth & DSI1_RXPKT1_H_WC_PARAM_MASK) >> 8;
                    reply->pReadReply[1] = 
                        (pkth & DSI1_RXPKT1_H_WC_PARAM_MASK) >> 16;
                    if(  ((reply->readReplyDt & 0x3F)==DSI_DT_P2H_SH_GEN_RD_1B)
                       ||((reply->readReplyDt & 0x3F)==DSI_DT_P2H_DCS_SH_RD_1B))
                    {
                        reply->readReplySize = 1;
                    }    
                    else
                    {
                        reply->readReplySize = 2;
                    }    
                }
            }        
        }
    }

    if ( event & DSI1_STAT_RX2_PKT_MASK )
    {
        // we can only get ERROR STAT here
        reply->type  |= RX_TYPE_ERR_REPLY;
    
        pkth = BRCM_READ_REG ( pDev->baseAddr, DSI1_RXPKT2_H );
        
        rxStat = 0;
        
        if( pkth & DSI1_RXPKT2_H_DET_ERR_MASK    ) 
            rxStat |= ERR_RX_MULTI_BIT;     
        if( pkth & DSI1_RXPKT2_H_ECC_ERR_MASK    ) 
            rxStat |= ERR_RX_ECC;           
        if( pkth & DSI1_RXPKT2_H_COR_ERR_MASK    ) 
            rxStat |= ERR_RX_CORRECTABLE;   
        if( pkth & DSI1_RXPKT2_H_INCOMP_PKT_MASK ) 
            rxStat |= ERR_RX_PKT_INCOMPLETE;
        
        reply->errReportRxStat = rxStat;
 
        if( (rxStat == 0) || (rxStat == ERR_RX_CORRECTABLE) )
            reply->errReportRxStat |= ERR_RX_OK;
                                               
        reply->errReportDt = pkth & DSI1_RXPKT2_H_DT_MASK;
        // ALWAYS SHORT - NO CRC
        reply->errReport = (pkth & DSI1_RXPKT2_H_PARAM_MASK) >> 
            DSI1_RXPKT2_H_PARAM_SHIFT;
    }
    
    if ( reply->type == 0 )
        return (CHAL_DSI_RX_NO_PKT);
    else
        return (CHAL_DSI_OK);
}    


//****************************************************************************
//
//  Function Name:  chal_dsi_de1_get_dma_address
//                  
//  Description:    
//                  
//****************************************************************************
UInt32 chal_dsi_de1_get_dma_address ( CHAL_HANDLE handle )
{
    CHAL_DSI_HANDLE dsiH = (CHAL_DSI_HANDLE) handle;
    
    return ( BRCM_REGADDR ( dsiH->baseAddr, DSI1_TXPKT_PIXD_FIFO ));
}

//****************************************************************************
//
//  Function Name:  chal_dsi_de1_set_dma_thresh
//                  
//  Description:    
//                  
//****************************************************************************
cVoid chal_dsi_de1_set_dma_thresh ( CHAL_HANDLE handle, cUInt32 thresh )
{
    CHAL_DSI_HANDLE pDev = (CHAL_DSI_HANDLE) handle;
    
    BRCM_WRITE_REG_FIELD ( pDev->baseAddr, DSI1_DISP1_CTRL, DMA_THRESH, thresh );
}

//****************************************************************************
//
//  Function Name:  chal_dsi_de1_set_wc
//                  
//  Description:    
//                  
//                  
//****************************************************************************
cVoid chal_dsi_de1_set_wc ( CHAL_HANDLE handle, cUInt32 wc )   
{
    // NA to HERA
}

//****************************************************************************
//
//  Function Name:  chal_dsi_cmnd_start
//                  
//  Description:    ENABLE | DISABLE  Command Interface
//                  
//****************************************************************************
cVoid chal_dsi_cmnd_start ( 
    CHAL_HANDLE handle, 
    cUInt8      txEng,   
    cBool       start )   
{
    CHAL_DSI_HANDLE pDev = (CHAL_DSI_HANDLE) handle;
    
    if ( start )
        if ( txEng )
            BRCM_WRITE_REG_FIELD ( pDev->baseAddr, DSI1_TXPKT2_C, CMD_EN, 1 );
        else
            BRCM_WRITE_REG_FIELD ( pDev->baseAddr, DSI1_TXPKT1_C, CMD_EN, 1 );
    else    
        if ( txEng )
            BRCM_WRITE_REG_FIELD ( pDev->baseAddr, DSI1_TXPKT2_C, CMD_EN, 0 );
        else
            BRCM_WRITE_REG_FIELD ( pDev->baseAddr, DSI1_TXPKT1_C, CMD_EN, 0 );
}


//****************************************************************************
//
//  Function Name:  chal_dsi_de1_set_cm
//                  
//  Description:    
//                  
//****************************************************************************
cVoid chal_dsi_de1_set_cm ( CHAL_HANDLE handle, CHAL_DSI_DE1_COL_MOD_t cm )
{
    CHAL_DSI_HANDLE pDev = (CHAL_DSI_HANDLE) handle;
    
    BRCM_WRITE_REG_FIELD ( pDev->baseAddr, DSI1_DISP1_CTRL, PFORMAT, cm );
}

//****************************************************************************
//
//  Function Name:  chal_dsi_de1_enable
//                  
//  Description:    Ena | Dis Color Engine 1
//                  
//****************************************************************************
cVoid chal_dsi_de1_enable ( CHAL_HANDLE handle, cBool ena )
{
    CHAL_DSI_HANDLE pDev = (CHAL_DSI_HANDLE) handle;
    
    if ( ena )
        BRCM_WRITE_REG_FIELD ( pDev->baseAddr, DSI1_DISP1_CTRL, EN, 1 );
    else    
        BRCM_WRITE_REG_FIELD ( pDev->baseAddr, DSI1_DISP1_CTRL, EN, 0 );
}

//****************************************************************************
//
//  Function Name:  chal_dsi_de1_send
//                  
//  Description:    Display Engine 1 Send
//                  Sends Packet(s) using Display Engine 1     
//                  (pixels from PIXEL FIFO)
//****************************************************************************
CHAL_DSI_RES_t chal_dsi_de1_send ( 
    CHAL_HANDLE      handle, 
    cUInt8           txEng,  
    pCHAL_DSI_CM_CFG cmCfg )
{
    CHAL_DSI_HANDLE pDev         = (CHAL_DSI_HANDLE) handle;
    cUInt32         pktc         = 0;
    cUInt32         pkth         = 0;
    cUInt32         dsi_DT       = 0;
    cUInt32         dcsCmndCount = 0;
   
    if ( cmCfg->dcsCmnd != 0 )
        dcsCmndCount = 1;    
    
    //================================   
    // PKTH    
    //================================   
    dsi_DT = ((cmCfg->vc & 0x00000003) << 6) | cmCfg->dsiCmnd;
    pkth  = dsi_DT;    
    pkth |=  DSI_REG_FIELD_SET( DSI1_TXPKT1_H, WC_PARAM, 
                cmCfg->pktSizeBytes+dcsCmndCount )
           | DSI_REG_FIELD_SET( DSI1_TXPKT1_H, WC_CDFIFO , dcsCmndCount );

    //================================   
    // PKTC 
    //================================   
    if( cmCfg->isLP ) 
        pktc |= DSI_REG_FIELD_SET( DSI1_TXPKT1_C, CMD_MODE  , 1 );

    if( cmCfg->isTE ) 
        pktc |= DSI_REG_FIELD_SET( DSI1_TXPKT1_C, CMD_TE_EN , 1 );

    // always long
    pktc |= DSI_REG_FIELD_SET( DSI1_TXPKT1_C, CMD_TYPE , 1 );

    pktc |= DSI_REG_FIELD_SET( DSI1_TXPKT1_C, CMD_CTRL   , CMND_CTRL_CMND_PKT );
    pktc |= DSI_REG_FIELD_SET( DSI1_TXPKT1_C, CMD_REPEAT , cmCfg->pktCount );
    pktc |= DSI_REG_FIELD_SET( DSI1_TXPKT1_C, CMD_TX_TIME, cmCfg->vmWhen );
    pktc |= DSI_REG_FIELD_SET( DSI1_TXPKT1_C, DISPLAY_NO , DSI_PKT_SRC_DE1 );
     
    if( cmCfg->start ) 
    {
        pktc |= DSI_REG_FIELD_SET( DSI1_TXPKT1_C, CMD_EN , 1 );
    }
     
    //================================   
    // DCS Cmnd to CMND FIFO
    //================================   
    if ( dcsCmndCount == 1 )
        //BRCM_WRITE_REG ( pDev->baseAddr, DSI_CMD_DATA_FIFO, cmCfg->dcsCmnd );
        BRCM_WRITE_REG ( pDev->baseAddr, DSI1_TXPKT_CMD_FIFO, cmCfg->dcsCmnd );
    
    //================================   
    // Execute 
    //================================   
    if ( txEng )
    {
        BRCM_WRITE_REG ( pDev->baseAddr, DSI1_TXPKT2_H, pkth );
        BRCM_WRITE_REG ( pDev->baseAddr, DSI1_TXPKT2_C, pktc );
    }
    else
    {
        BRCM_WRITE_REG ( pDev->baseAddr, DSI1_TXPKT1_H, pkth );
        BRCM_WRITE_REG ( pDev->baseAddr, DSI1_TXPKT1_C, pktc );
    }

    return ( CHAL_DSI_OK );
}

            
    
