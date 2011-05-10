/*****************************************************************************
*
*    (c) 2001-2010 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or
* its licensors, and may only be used, duplicated, modified or distributed
* pursuant to the terms and conditions of a separate, written license
* agreement executed between you and Broadcom (an "Authorized License").
* Except as set forth in an Authorized License, Broadcom grants no license
* (express or implied), right to use, or waiver of any kind with respect to
* the Software, and Broadcom expressly reserves all rights in and to the
* Software and all intellectual property rights therein.
* IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS
* SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE
* ALL USE OF THE SOFTWARE.  
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
*    IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS
*    FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS,
*    QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU
*    ASSUME THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
*
*****************************************************************************/
/**
*
*  @file   csl_trace.c
*
*  @brief  This file contains APIs for trace/debug module
*
****************************************************************************/
#define UNDER_LINUX // Only supports Rhea
#define CNEON_COMMON
#define FUSE_APPS_PROCESSOR

//#include <stdio.h>
#include <linux/string.h>
#include <linux/init.h>
//#include <stdarg.h>
#include "plat/chal/chal_types.h"
#include "plat/mobcom_types.h"
//#include "xassert.h"
//#include "ostimer.h"
#include <linux/timer.h>
#include "plat/osabstract/ostypes.h"
#include "plat/chal/chal_trace.h"
#include "plat/csl/csl_trace.h"
//#include "sysmap_types.h"
//#include "csl_sysmap.h"
#include "mach/rdb/brcm_rdb_sysmap_a9.h"
// Lots of RDBs to setup the pads/funnels
#include "mach/rdb/brcm_rdb_util.h"
#include "mach/rdb/brcm_rdb_swstm.h"
#include "mach/rdb/brcm_rdb_atb_stm.h"
#include "mach/rdb/brcm_rdb_cstf.h"
#include "mach/rdb/brcm_rdb_atbfilter.h"
#include "mach/rdb/brcm_rdb_padctrlreg.h"
#include "mach/rdb/brcm_rdb_chipreg.h"
#include <mach/hardware.h>

#ifdef UNDER_LINUX
#define xassert(e,v) ((e) ? (void)0 : printk("Assertion failed! %s, line=%d\n", __FILE__, __LINE__))
#endif

//******************************************************************************
// Constants Definition
//******************************************************************************

//******************************************************************************
// Local Definition
//******************************************************************************

/* STM and SWSTM on A9 will be initialized at boot loader. */
/* In case SW need the initialization again, define STM_INIT_FROM_CSL */
#ifdef CNEON_COMMON
#define STM_INIT_FROM_CSL
#else
#undef  STM_INIT_FROM_CSL
#endif /* CNEON_COMMON */

#define ATB_ID_ODD(x) ((x<<1)|0x1) // 6bit ATB ID + 1bit (1)
#define ATB_ID_EVEN(x) (x<<1) // 6bit ATB ID + 1bit (0)
#define INVALID_FUNNEL CHAL_TRACE_MAX_FUNNEL
#define LOCK_ACCESS_CODE 0xC5ACCE55

static CHAL_TRACE_DEV_t TraceDev_baseaddr;
static CHAL_HANDLE csl_trace_handle = NULL;
#if defined(CSL_TRACE_PERF_COUNT_SUPPORT)
#ifndef UNDER_LINUX
static Timer_t csl_trace_timer = NULL;
#else
static struct timer_list trace_timer;
static Timer_t csl_trace_timer = NULL;
#endif
static UInt32 RecentCount[CHAL_TRACE_MAX_AXITRACE][CHAL_TRACE_AXITRACE_MAX_COUNT];
static Boolean csl_trace_count_saturated[CHAL_TRACE_MAX_AXITRACE];
#endif // (CSL_TRACE_PERF_COUNT_SUPPORT)

#if defined(CSL_TRACE_AXI_TRACE_SUPPORT)
typedef struct
{
   UInt8 atb_id_write;
   UInt8 atb_id_read;
   CHAL_TRACE_FUNNEL_t funnel_1;
   UInt8 port_1;
   CHAL_TRACE_FUNNEL_t funnel_2;
   UInt8 port_2;
} CSL_TRACE_axi_config_st_t;

static const CSL_TRACE_axi_config_st_t axi_config[CSL_TRACE_AXITRACE_TOTAL] =
{
#if defined (_RHEA_)
   {0, 0, INVALID_FUNNEL, 0, INVALID_FUNNEL, 0},
   {ATB_ID_AXI1_WRITE, ATB_ID_AXI1_READ, CHAL_TRACE_FUNNEL, 2, CHAL_TRACE_FIN_FUNNEL, 1},
   {ATB_ID_AXI2_WRITE, ATB_ID_AXI2_READ, CHAL_TRACE_FAB_FUNNEL1, 0, CHAL_TRACE_FIN_FUNNEL, 2},
   {ATB_ID_AXI3_WRITE, ATB_ID_AXI3_READ, CHAL_TRACE_FAB_FUNNEL1, 1, CHAL_TRACE_FIN_FUNNEL, 2},
   {ATB_ID_AXI4_WRITE, ATB_ID_AXI4_READ, CHAL_TRACE_FUNNEL, 3, CHAL_TRACE_FIN_FUNNEL, 1},
   {0, 0, INVALID_FUNNEL, 0, INVALID_FUNNEL, 0},
   {0, 0, INVALID_FUNNEL, 0, INVALID_FUNNEL, 0},
   {0, 0, INVALID_FUNNEL, 0, INVALID_FUNNEL, 0},
   {0, 0, INVALID_FUNNEL, 0, INVALID_FUNNEL, 0},
   {0, 0, INVALID_FUNNEL, 0, INVALID_FUNNEL, 0},
   {0, 0, INVALID_FUNNEL, 0, INVALID_FUNNEL, 0},
   {ATB_ID_AXI11_WRITE, ATB_ID_AXI11_READ, CHAL_TRACE_HUB_FUNNEL, 1, CHAL_TRACE_FIN_FUNNEL, 0},
   {ATB_ID_AXI12_WRITE, ATB_ID_AXI12_READ, CHAL_TRACE_HUB_FUNNEL, 2, CHAL_TRACE_FIN_FUNNEL, 0},
   {0, 0, INVALID_FUNNEL, 0, INVALID_FUNNEL, 0},
   {0, 0, INVALID_FUNNEL, 0, INVALID_FUNNEL, 0},
   {0, 0, INVALID_FUNNEL, 0, INVALID_FUNNEL, 0},
   {ATB_ID_AXI16_WRITE, ATB_ID_AXI16_READ, CHAL_TRACE_FUNNEL, 5, CHAL_TRACE_FIN_FUNNEL, 1},
   {ATB_ID_AXI17_WRITE, ATB_ID_AXI17_READ, CHAL_TRACE_FIN_FUNNEL, 3, INVALID_FUNNEL, 0},
   {ATB_ID_AXI18_WRITE, ATB_ID_AXI18_READ, CHAL_TRACE_COMMS_FUNNEL, 2, CHAL_TRACE_FIN_FUNNEL, 4},
   {ATB_ID_AXI19_WRITE, ATB_ID_AXI19_READ, CHAL_TRACE_HUB_FUNNEL, 0, CHAL_TRACE_FIN_FUNNEL, 0},
#else
#error platform not supported!
#endif //defined(_RHEA_)
};
#endif // (CSL_TRACE_AXI_TRACE_SUPPORT)

//******************************************************************************
// Function Definition
//******************************************************************************
#if defined(CSL_TRACE_PERF_COUNT_SUPPORT)
static CHAL_TRACE_AXITRACE_t csl_trace_get_chal_axi_type( CSL_TRACE_AXITraceID_t axitrace_id );
static CHAL_TRACE_AXITRACE_COUNT_t csl_trace_get_chal_active_count_type( CSL_TRACE_AXIChannel_t  channel );
static CHAL_TRACE_AXITRACE_COUNT_t csl_trace_get_chal_busy_count_type( CSL_TRACE_AXIChannel_t  channel );
static void csl_trace_store_count ( void );
#ifndef UNDER_LINUX
static void csl_trace_handle_timer( TimerID_t id );
#else
static void csl_trace_handle_timer( unsigned long id );
#endif
#endif // CSL_TRACE_PERF_COUNT_SUPPORT

#if defined (CSL_TRACE_STM_SUPPORT) || defined (CSL_TRACE_AXI_TRACE_SUPPORT) || defined (CSL_TRACE_GIC_TRACE_SUPPORT) || defined (CSL_TRACE_PWRMGR_TRACE_SUPPORT)
static Boolean csl_trace_funnel_set_enable ( CHAL_TRACE_FUNNEL_t funnel_type, cUInt8 port_n, cBool enable );
#endif // (CSL_TRACE_STM_SUPPORT) ||  (CSL_TRACE_AXI_TRACE_SUPPORT) ||  (CSL_TRACE_GIC_TRACE_SUPPORT) ||  (CSL_TRACE_PWRMGR_TRACE_SUPPORT)

#if defined (CSL_TRACE_STM_SUPPORT)
static void csl_trace_set_stm_sw( UInt8 bit_mask, Boolean set );
static void csl_trace_set_stm_en( UInt8 bit_mask, Boolean set );
#endif // (CSL_TRACE_STM_SUPPORT)


#ifdef UNDER_LINUX
//******************************************************************************
// 
//  Function Name: csl_tracepad_init
//
//  Description:   Initialize pads for trace/debug module (not needed for RTOS as done in  loader)
//
//******************************************************************************
static void csl_tracepad_init(void)
{
    // All register config values taken from T32 script
    
    // clear pti_clk_is_idle
    BRCM_WRITE_REG(KONA_CHIPREG_VA, CHIPREG_PERIPH_SPARE_CONTROL1, 0x2);

    // Config ATB Filter rm id's for STM
    BRCM_WRITE_REG(KONA_ATBFILTER_VA, ATBFILTER_ATB_FILTER, 0x203);
    // Config Funnels
    BRCM_WRITE_REG(KONA_FUNNEL_VA, CSTF_FUNNEL_CONTROL, 0xe40);
    BRCM_WRITE_REG(KONA_FIN_FUNNEL_VA, CSTF_FUNNEL_CONTROL, 0xe02);
    // Config STM
    BRCM_WRITE_REG(KONA_STM_VA, ATB_STM_CONFIG, 0x102);
    BRCM_WRITE_REG(KONA_SWSTM_VA, SWSTM_R_CONFIG, 0x82);
    BRCM_WRITE_REG(KONA_SWSTM_ST_VA, SWSTM_R_CONFIG, 0x82);

    // Not tracepad setup, but RXD clock setup
	*(volatile UInt32*)(KONA_SLV_CLK_VA +0x000) = 0xA5A501; // WR_ACCESS
	*(volatile UInt32*)(KONA_SLV_CLK_VA +0x01C) |= 0x50000; // UARTB3_POLICY3_MASK
	*(volatile UInt32*)(KONA_SLV_CLK_VA +0x034) = 0x01;		// LVM_EN
	*(volatile UInt32*)(KONA_SLV_CLK_VA +0x00C) = 0x05;		// POLICY_CTL
	*(volatile UInt32*)(KONA_SLV_CLK_VA +0x408) = 0x0F;		// UARTB3

}
#endif

//******************************************************************************
// 
//  Function Name: csl_trace_init
//
//  Description:   Initialize CSL for trace/debug module
//
//******************************************************************************
#ifdef UNDER_LINUX
static int __init csl_trace_init(void)
#else
CSL_TRACE_ERR_CODE_t csl_trace_init ( void )
#endif
{
    CSL_TRACE_ERR_CODE_t status = CSL_TRACE_FAIL;

    if (csl_trace_handle)
    {
        status = CSL_TRACE_INIT;
    }
    else
    {
        memset (&TraceDev_baseaddr, 0, sizeof (CHAL_TRACE_DEV_t));
#if defined(CSL_TRACE_PERF_COUNT_SUPPORT)
        memset (RecentCount, 0, sizeof(RecentCount));
        memset (csl_trace_count_saturated, 0, sizeof(csl_trace_count_saturated));
#endif // CSL_TRACE_PERF_COUNT_SUPPORT

#if defined (_HERA_)

        TraceDev_baseaddr.APB2ATB_base = csl_sysmap_get_base_addr(APB2ATB);
        TraceDev_baseaddr.ATB2PTI_base = csl_sysmap_get_base_addr(ATB2PTI);
        TraceDev_baseaddr.ATB2OCP_base = csl_sysmap_get_base_addr(ATB2OCP);
        TraceDev_baseaddr.WGM_OCP2ATB_base = csl_sysmap_get_base_addr(A9_STMBUS_OCP2ATB);
        TraceDev_baseaddr.HUBOCP2ATB_base = csl_sysmap_get_base_addr(HUBOCP2ATB);

        TraceDev_baseaddr.GICTR_base = csl_sysmap_get_base_addr(GICTR);

        TraceDev_baseaddr.PWRMGR_base = csl_sysmap_get_base_addr(PWRMGR);

        TraceDev_baseaddr.FUNNEL_base[CHAL_TRACE_HUB_FUNNEL] = csl_sysmap_get_base_addr(HUB_FUNNEL);
        TraceDev_baseaddr.FUNNEL_base[CHAL_TRACE_FUNNEL] = csl_sysmap_get_base_addr(FUNNEL);
        TraceDev_baseaddr.FUNNEL_base[CHAL_TRACE_FIN_FUNNEL] = csl_sysmap_get_base_addr(FIN_FUNNEL);
        TraceDev_baseaddr.FUNNEL_base[CHAL_TRACE_FAB_FUNNEL1] = csl_sysmap_get_base_addr(FAB_FUNNEL1);
        TraceDev_baseaddr.FUNNEL_base[CHAL_TRACE_MM_FUNNEL] = csl_sysmap_get_base_addr(MM_FUNNEL);
        TraceDev_baseaddr.FUNNEL_base[CHAL_TRACE_COMMS_FUNNEL] = csl_sysmap_get_base_addr(A9_ATB_FUNNEL);

        TraceDev_baseaddr.AXITRACE_base[CHAL_TRACE_AXITRACE1] = csl_sysmap_get_base_addr(AXITRACE1);
        TraceDev_baseaddr.AXITRACE_base[CHAL_TRACE_AXITRACE2] = csl_sysmap_get_base_addr(AXITRACE2);
        TraceDev_baseaddr.AXITRACE_base[CHAL_TRACE_AXITRACE3] = csl_sysmap_get_base_addr(AXITRACE3);
        TraceDev_baseaddr.AXITRACE_base[CHAL_TRACE_AXITRACE4] = csl_sysmap_get_base_addr(AXITRACE4);
        TraceDev_baseaddr.AXITRACE_base[CHAL_TRACE_AXITRACE5] = csl_sysmap_get_base_addr(AXITRACE5);
        TraceDev_baseaddr.AXITRACE_base[CHAL_TRACE_AXITRACE6] = csl_sysmap_get_base_addr(AXITRACE6);
        TraceDev_baseaddr.AXITRACE_base[CHAL_TRACE_AXITRACE7] = csl_sysmap_get_base_addr(AXITRACE7);
        TraceDev_baseaddr.AXITRACE_base[CHAL_TRACE_AXITRACE9] = csl_sysmap_get_base_addr(AXITRACE9);
        TraceDev_baseaddr.AXITRACE_base[CHAL_TRACE_AXITRACE10] = csl_sysmap_get_base_addr(AXITRACE10);
        TraceDev_baseaddr.AXITRACE_base[CHAL_TRACE_AXITRACE11] = csl_sysmap_get_base_addr(AXITRACE11);
        TraceDev_baseaddr.AXITRACE_base[CHAL_TRACE_AXITRACE12] = csl_sysmap_get_base_addr(AXITRACE12);
        TraceDev_baseaddr.AXITRACE_base[CHAL_TRACE_AXITRACE14] = csl_sysmap_get_base_addr(AXITRACE14);
        TraceDev_baseaddr.AXITRACE_base[CHAL_TRACE_AXITRACE16] = csl_sysmap_get_base_addr(AXITRACE16);
        TraceDev_baseaddr.AXITRACE_base[CHAL_TRACE_AXITRACE17] = csl_sysmap_get_base_addr(AXITRACE17);

        TraceDev_baseaddr.CTI_base[CHAL_TRACE_HUB_CTI] = csl_sysmap_get_base_addr(HUB_CTI);
        TraceDev_baseaddr.CTI_base[CHAL_TRACE_MM_CTI] = csl_sysmap_get_base_addr(MM_CTI);
        TraceDev_baseaddr.CTI_base[CHAL_TRACE_FAB_CTI] = csl_sysmap_get_base_addr(FAB_CTI);
        TraceDev_baseaddr.CTI_base[CHAL_TRACE_A9CTI0] = csl_sysmap_get_base_addr(A9CTI0);
        TraceDev_baseaddr.CTI_base[CHAL_TRACE_R4CS_CTI] = csl_sysmap_get_base_addr(R4CS_CTI);
        TraceDev_baseaddr.CTI_base[CHAL_TRACE_R4MPSS_CTI] = csl_sysmap_get_base_addr(R4MPSS_CTI);
        TraceDev_baseaddr.CTI_base[CHAL_TRACE_R4_CTI] = csl_sysmap_get_base_addr(R4_CTI);

        TraceDev_baseaddr.ETB_base = csl_sysmap_get_base_addr(ETB);

        TraceDev_baseaddr.ETB2AXI_base = csl_sysmap_get_base_addr(ETB2AXI);

        TraceDev_baseaddr.GLOBPERF_base = csl_sysmap_get_base_addr(GLOBPERF);

#elif defined(_RHEA_)

#ifndef UNDER_LINUX

#ifdef CNEON_COMMON
        TraceDev_baseaddr.CHIPREGS_base = csl_sysmap_get_base_addr(CHIPREGS);
#endif /* CNEON_COMMON */
        TraceDev_baseaddr.PWRMGR_base = csl_sysmap_get_base_addr(PWRMGR);
        TraceDev_baseaddr.FUNNEL_base[CHAL_TRACE_HUB_FUNNEL] = csl_sysmap_get_base_addr(HUB_FUNNEL);
        TraceDev_baseaddr.FUNNEL_base[CHAL_TRACE_FUNNEL] = csl_sysmap_get_base_addr(FUNNEL);
        TraceDev_baseaddr.FUNNEL_base[CHAL_TRACE_FIN_FUNNEL] = csl_sysmap_get_base_addr(FIN_FUNNEL);
        TraceDev_baseaddr.FUNNEL_base[CHAL_TRACE_FAB_FUNNEL1] = csl_sysmap_get_base_addr(FAB_FUNNEL1);
        TraceDev_baseaddr.FUNNEL_base[CHAL_TRACE_COMMS_FUNNEL] = csl_sysmap_get_base_addr(BMODEM_FUNNEL);

        TraceDev_baseaddr.AXITRACE_base[CHAL_TRACE_AXITRACE1] = csl_sysmap_get_base_addr(AXITRACE1);
        TraceDev_baseaddr.AXITRACE_base[CHAL_TRACE_AXITRACE2] = csl_sysmap_get_base_addr(AXITRACE2);
        TraceDev_baseaddr.AXITRACE_base[CHAL_TRACE_AXITRACE3] = csl_sysmap_get_base_addr(AXITRACE3);
        TraceDev_baseaddr.AXITRACE_base[CHAL_TRACE_AXITRACE4] = csl_sysmap_get_base_addr(AXITRACE4);
        TraceDev_baseaddr.AXITRACE_base[CHAL_TRACE_AXITRACE11] = csl_sysmap_get_base_addr(AXITRACE11);
        TraceDev_baseaddr.AXITRACE_base[CHAL_TRACE_AXITRACE12] = csl_sysmap_get_base_addr(AXITRACE12);
        TraceDev_baseaddr.AXITRACE_base[CHAL_TRACE_AXITRACE16] = csl_sysmap_get_base_addr(AXITRACE16);
        TraceDev_baseaddr.AXITRACE_base[CHAL_TRACE_AXITRACE17] = csl_sysmap_get_base_addr(AXITRACE17);
        TraceDev_baseaddr.AXITRACE_base[CHAL_TRACE_AXITRACE18] = csl_sysmap_get_base_addr(AXITP18);
        TraceDev_baseaddr.AXITRACE_base[CHAL_TRACE_AXITRACE19] = csl_sysmap_get_base_addr(AXITRACE19);

        TraceDev_baseaddr.CTI_base[CHAL_TRACE_HUB_CTI] = csl_sysmap_get_base_addr(HUB_CTI);

        TraceDev_baseaddr.CTI_base[CHAL_TRACE_MM_CTI] = csl_sysmap_get_base_addr(MM_CTI);
        TraceDev_baseaddr.CTI_base[CHAL_TRACE_FAB_CTI] = csl_sysmap_get_base_addr(FAB_CTI);

        TraceDev_baseaddr.CTI_base[CHAL_TRACE_A9CTI0] = csl_sysmap_get_base_addr(A9CTI0);

        TraceDev_baseaddr.CTI_base[CHAL_TRACE_R4_CTI] = csl_sysmap_get_base_addr(BMODEM_CTI);

        TraceDev_baseaddr.ETB_base = csl_sysmap_get_base_addr(ETB);
        TraceDev_baseaddr.ETB2AXI_base = csl_sysmap_get_base_addr(ETB2AXI);

        TraceDev_baseaddr.GLOBPERF_base = csl_sysmap_get_base_addr(GLOBPERF);

        TraceDev_baseaddr.ATB_STM_base = csl_sysmap_get_base_addr(STM);
#if defined(FUSE_APPS_PROCESSOR)
        TraceDev_baseaddr.SW_STM_base[CHAL_TRACE_SWSTM] = csl_sysmap_get_base_addr(SWSTM);
        TraceDev_baseaddr.SW_STM_base[CHAL_TRACE_SWSTM_ST] = csl_sysmap_get_base_addr(SWSTM_ST);
        TraceDev_baseaddr.GICTR_base = csl_sysmap_get_base_addr(GICTR);

#else
        TraceDev_baseaddr.SW_STM_base[CHAL_TRACE_SWSTM] = csl_sysmap_get_base_addr(BMODEM_SWSTM);
        TraceDev_baseaddr.SW_STM_base[CHAL_TRACE_SWSTM_ST] = csl_sysmap_get_base_addr(BMODEM_SWSTM_TS);
        TraceDev_baseaddr.GICTR_base = csl_sysmap_get_base_addr(BMODEM_GICTR);
#endif

#else //UNDER_LINUX

#ifdef CNEON_COMMON
        TraceDev_baseaddr.CHIPREGS_base  = HW_IO_PHYS_TO_VIRT(CHIPREGS_BASE_ADDR);
#endif /* CNEON_COMMON */
        TraceDev_baseaddr.PWRMGR_base  = HW_IO_PHYS_TO_VIRT(PWRMGR_BASE_ADDR);
        TraceDev_baseaddr.FUNNEL_base[CHAL_TRACE_HUB_FUNNEL]  = HW_IO_PHYS_TO_VIRT(HUB_FUNNEL_BASE_ADDR);
        TraceDev_baseaddr.FUNNEL_base[CHAL_TRACE_FUNNEL]  = HW_IO_PHYS_TO_VIRT(FUNNEL_BASE_ADDR);
        TraceDev_baseaddr.FUNNEL_base[CHAL_TRACE_FIN_FUNNEL]  = HW_IO_PHYS_TO_VIRT(FIN_FUNNEL_BASE_ADDR);
        TraceDev_baseaddr.FUNNEL_base[CHAL_TRACE_FAB_FUNNEL1]  = HW_IO_PHYS_TO_VIRT(FAB_FUNNEL1_BASE_ADDR);
        TraceDev_baseaddr.FUNNEL_base[CHAL_TRACE_COMMS_FUNNEL]  = HW_IO_PHYS_TO_VIRT(BMODEM_FUNNEL_BASE_ADDR);

        TraceDev_baseaddr.AXITRACE_base[CHAL_TRACE_AXITRACE1]  = HW_IO_PHYS_TO_VIRT(AXITRACE1_BASE_ADDR);
        TraceDev_baseaddr.AXITRACE_base[CHAL_TRACE_AXITRACE2]  = HW_IO_PHYS_TO_VIRT(AXITRACE2_BASE_ADDR);
        TraceDev_baseaddr.AXITRACE_base[CHAL_TRACE_AXITRACE3]  = HW_IO_PHYS_TO_VIRT(AXITRACE3_BASE_ADDR);
        TraceDev_baseaddr.AXITRACE_base[CHAL_TRACE_AXITRACE4]  = HW_IO_PHYS_TO_VIRT(AXITRACE4_BASE_ADDR);
        TraceDev_baseaddr.AXITRACE_base[CHAL_TRACE_AXITRACE11]  = HW_IO_PHYS_TO_VIRT(AXITRACE11_BASE_ADDR);
        TraceDev_baseaddr.AXITRACE_base[CHAL_TRACE_AXITRACE12]  = HW_IO_PHYS_TO_VIRT(AXITRACE12_BASE_ADDR);
        TraceDev_baseaddr.AXITRACE_base[CHAL_TRACE_AXITRACE16]  = HW_IO_PHYS_TO_VIRT(AXITRACE16_BASE_ADDR);
        TraceDev_baseaddr.AXITRACE_base[CHAL_TRACE_AXITRACE17]  = HW_IO_PHYS_TO_VIRT(AXITRACE17_BASE_ADDR);
        TraceDev_baseaddr.AXITRACE_base[CHAL_TRACE_AXITRACE18]  = HW_IO_PHYS_TO_VIRT(AXITP18_BASE_ADDR);
        TraceDev_baseaddr.AXITRACE_base[CHAL_TRACE_AXITRACE19]  = HW_IO_PHYS_TO_VIRT(AXITRACE19_BASE_ADDR);

        TraceDev_baseaddr.CTI_base[CHAL_TRACE_HUB_CTI]  = HW_IO_PHYS_TO_VIRT(HUB_CTI_BASE_ADDR);

        TraceDev_baseaddr.CTI_base[CHAL_TRACE_MM_CTI]  = HW_IO_PHYS_TO_VIRT(MM_CTI_BASE_ADDR);
        TraceDev_baseaddr.CTI_base[CHAL_TRACE_FAB_CTI]  = HW_IO_PHYS_TO_VIRT(FAB_CTI_BASE_ADDR);

        TraceDev_baseaddr.CTI_base[CHAL_TRACE_A9CTI0]  = HW_IO_PHYS_TO_VIRT(A9CTI0_BASE_ADDR);

        TraceDev_baseaddr.CTI_base[CHAL_TRACE_R4_CTI]  = HW_IO_PHYS_TO_VIRT(BMODEM_CTI_BASE_ADDR);

        TraceDev_baseaddr.ETB_base  = HW_IO_PHYS_TO_VIRT(ETB_BASE_ADDR);
        TraceDev_baseaddr.ETB2AXI_base  = HW_IO_PHYS_TO_VIRT(ETB2AXI_BASE_ADDR);

        TraceDev_baseaddr.GLOBPERF_base  = HW_IO_PHYS_TO_VIRT(GLOBPERF_BASE_ADDR);

        TraceDev_baseaddr.ATB_STM_base  = HW_IO_PHYS_TO_VIRT(STM_BASE_ADDR);
        TraceDev_baseaddr.SW_STM_base[CHAL_TRACE_SWSTM]  = HW_IO_PHYS_TO_VIRT(SWSTM_BASE_ADDR);
        TraceDev_baseaddr.SW_STM_base[CHAL_TRACE_SWSTM_ST]  = HW_IO_PHYS_TO_VIRT(SWSTM_ST_BASE_ADDR);
        TraceDev_baseaddr.GICTR_base  = HW_IO_PHYS_TO_VIRT(GICTR_BASE_ADDR);

#endif //UNDER_LINUX

#elif defined(_SAMOA_)

        TraceDev_baseaddr.PWRMGR_base = csl_sysmap_get_base_addr(PWRMGR);
        TraceDev_baseaddr.FUNNEL_base[CHAL_TRACE_HUB_FUNNEL] = csl_sysmap_get_base_addr(HUB_FUNNEL);
        TraceDev_baseaddr.FUNNEL_base[CHAL_TRACE_FUNNEL] = csl_sysmap_get_base_addr(FUNNEL);

        TraceDev_baseaddr.FUNNEL_base[CHAL_TRACE_COMMS_FUNNEL] = csl_sysmap_get_base_addr(BMODEM_FUNNEL);

        TraceDev_baseaddr.AXITRACE_base[CHAL_TRACE_AXITRACE18] = csl_sysmap_get_base_addr(AXITP18);

        TraceDev_baseaddr.CTI_base[CHAL_TRACE_HUB_CTI] = csl_sysmap_get_base_addr(HUB_CTI);

        TraceDev_baseaddr.CTI_base[CHAL_TRACE_A9CTI0] = csl_sysmap_get_base_addr(A9CTI0);

        TraceDev_baseaddr.CTI_base[CHAL_TRACE_R4_CTI] = csl_sysmap_get_base_addr(BMODEM_CTI);

        TraceDev_baseaddr.ETB_base = csl_sysmap_get_base_addr(ETB);
        TraceDev_baseaddr.ETB2AXI_base = csl_sysmap_get_base_addr(ETB2AXI);

        TraceDev_baseaddr.ATB_STM_base = csl_sysmap_get_base_addr(STM);

#if defined(FUSE_APPS_PROCESSOR)
        TraceDev_baseaddr.SW_STM_base[CHAL_TRACE_SWSTM] = csl_sysmap_get_base_addr(SWSTM);
        TraceDev_baseaddr.SW_STM_base[CHAL_TRACE_SWSTM_ST] = csl_sysmap_get_base_addr(SWSTM_ST);
        TraceDev_baseaddr.GICTR_base = csl_sysmap_get_base_addr(GICTR);
#else
        TraceDev_baseaddr.SW_STM_base[CHAL_TRACE_SWSTM] = csl_sysmap_get_base_addr(BMODEM_SWSTM);
        TraceDev_baseaddr.SW_STM_base[CHAL_TRACE_SWSTM_ST] = csl_sysmap_get_base_addr(BMODEM_SWSTM_TS);
        TraceDev_baseaddr.GICTR_base = csl_sysmap_get_base_addr(BMODEM_GICTR);
#endif

#else

#error platform not supported!

#endif
#ifdef UNDER_LINUX
        csl_tracepad_init();
#endif
        chal_trace_init (&TraceDev_baseaddr);
        csl_trace_handle = &TraceDev_baseaddr;
    
        if (csl_trace_handle)
        {
            status = CSL_TRACE_OK;
        }
#if defined (CSL_TRACE_STM_SUPPORT)
#if defined(FUSE_APPS_PROCESSOR)
#if defined (STM_INIT_FROM_CSL)
        /* STM config */
        /* 4 bits wide PTI output, always break, ATBID 0x0B */
        chal_trace_atb_stm_set_config(csl_trace_handle, 0, 0, 1, ATB_ID_ODD(ATB_ID_STM));
        /* Turn on the A9 SWSTM */
        csl_trace_set_sw_stm (1);

#endif //(STM_INIT_FROM_CSL)
#else // defined(FUSE_APPS_PROCESSOR)
        /* Turn on the R4 SWSTM */
        csl_trace_set_sw_stm (1);
#endif // defined(FUSE_APPS_PROCESSOR)
#endif // (CSL_TRACE_STM_SUPPORT)
    }

#ifdef UNDER_LINUX
    pr_info("CSL Trace Init complete\n");
#endif
    return status;
}

#if defined(CSL_TRACE_PERF_COUNT_SUPPORT)
//******************************************************************************
// 
//  Function Name: csl_trace_get_recent_active_count
//
//  Description:   Get recent active counter of specific AXITRACE.
//
//******************************************************************************
CSL_TRACE_ERR_CODE_t csl_trace_get_recent_active_count ( CSL_TRACE_AXITraceID_t axitrace_id, CSL_TRACE_AXIChannel_t  channel,  CSL_TRACE_Report_st_t *pActiveCount )
{
    CSL_TRACE_ERR_CODE_t status = CSL_TRACE_OK;
    CHAL_TRACE_AXITRACE_t chal_axi_type;
    CHAL_TRACE_AXITRACE_COUNT_t chal_axi_count_type;
    CSL_TRACE_AXITraceID_t count;
    CSL_TRACE_AXITraceID_t count_from;
    CSL_TRACE_AXITraceID_t count_to;

    if (csl_trace_handle == NULL)
    {
        status = CSL_TRACE_NO_INIT;
    }
    else
    {
        if (axitrace_id == CSL_TRACE_AXITRACE_ALL)
        {
            count_from = CSL_TRACE_AXITRACE1;
            count_to = CSL_TRACE_AXITRACE19;

        }
        else if ((axitrace_id < CSL_TRACE_AXITRACE_ALL) && (axitrace_id >= CSL_TRACE_AXITRACE1))
        {
            count_from = axitrace_id;
            count_to = axitrace_id;
        }
        else
        {
            /* invalid axitrace_id */
            status = CSL_TRACE_INVALID_PARM;
        }

        if (status != CSL_TRACE_INVALID_PARM)
        {
            chal_axi_count_type = csl_trace_get_chal_active_count_type(channel);

            for (count = count_from; count <= count_to; count++)
            {
                chal_axi_type = csl_trace_get_chal_axi_type(count);

                pActiveCount->Util[count] = RecentCount[chal_axi_type][chal_axi_count_type];;
                /* if saturation occured on any channel, return saturation status */
                /* Need to reduce Profiling periodic timer interval */
                if (csl_trace_count_saturated[chal_axi_type])
                {
                    status = CSL_TRACE_SATURATION;
                }
            }

        }
    }
    return status;
}

//******************************************************************************
// 
//  Function Name: csl_trace_get_recent_busy_count
//
//  Description:   Get recent busy counter of specific AXITRACE.
//
//******************************************************************************
CSL_TRACE_ERR_CODE_t csl_trace_get_recent_busy_count ( CSL_TRACE_AXITraceID_t axitrace_id, CSL_TRACE_AXIChannel_t  channel,  CSL_TRACE_Report_st_t *pBusyCount )
{
    CSL_TRACE_ERR_CODE_t status = CSL_TRACE_OK;
    CHAL_TRACE_AXITRACE_t chal_axi_type;
    CHAL_TRACE_AXITRACE_COUNT_t chal_axi_count_type;
    CSL_TRACE_AXITraceID_t count;
    CSL_TRACE_AXITraceID_t count_from;
    CSL_TRACE_AXITraceID_t count_to;

    if (csl_trace_handle == NULL)
    {
        status = CSL_TRACE_NO_INIT;
    }
    else
    {
        if (axitrace_id == CSL_TRACE_AXITRACE_ALL)
        {
            count_from = CSL_TRACE_AXITRACE1;
            count_to = CSL_TRACE_AXITRACE19;

        }
        else if ((axitrace_id < CSL_TRACE_AXITRACE_ALL) && (axitrace_id >= CSL_TRACE_AXITRACE1))
        {
            count_from = axitrace_id;
            count_to = axitrace_id;
        }
        else
        {
            /* invalid axitrace_id */
            status = CSL_TRACE_INVALID_PARM;
        }

        if (status != CSL_TRACE_INVALID_PARM)
        {
            chal_axi_count_type = csl_trace_get_chal_busy_count_type(channel);

            for (count = count_from; count <= count_to; count++)
            {
                chal_axi_type = csl_trace_get_chal_axi_type(count);
                pBusyCount->Util[count] = RecentCount[chal_axi_type][chal_axi_count_type];
                /* if saturation occured on any channel, return saturation status */
                /* Need to reduce Profiling periodic timer interval */
                if (csl_trace_count_saturated[chal_axi_type])
                {
                    status = CSL_TRACE_SATURATION;
                }
            }
        }
    }
    return status;
}

//******************************************************************************
// 
//  Function Name: csl_trace_start_perfcounter
//
//  Description:   Start AXITRACE perf. Counting. 
//                 Perf counting runs all the time after this function is called and until csl_trace_stop_perfcounter is called.  
//                 During the runs, user might call csl_trace_get_recent_utilpct to get the instantaneous utilization percentage.
//                 If the period_ms is zero, do not run the timer. Uppder layer (e.g. xscript) will run the timer.
//
//******************************************************************************
CSL_TRACE_ERR_CODE_t csl_trace_start_perfcounter ( UInt32 period_ms, Boolean all_cycle_enable )
{
    UInt8 count;
    CHAL_TRACE_ATM_CONF_t atm_config;
    CSL_TRACE_ERR_CODE_t status = CSL_TRACE_FAIL;

    if (csl_trace_handle == NULL)
    {
        status = CSL_TRACE_NO_INIT;
    }
    else
    {

        if (csl_trace_timer)
        {
            /* performance counter already started */
            status = CSL_TRACE_FAIL;
        }
        else
        {
            if (period_ms)
            {
#ifndef UNDER_LINUX
                /* Profiling periodic time interval */
                csl_trace_timer = OSTIMER_Create(
                        (TimerEntry_t)csl_trace_handle_timer,
                        0,
                        TICKS_IN_MILLISECONDS(period_ms),
                        TICKS_IN_MILLISECONDS(period_ms)
                        );
#else
                init_timer (&trace_timer);
                trace_timer.expires = TICKS_IN_MILLISECONDS(period_ms);
                trace_timer.data = 0;
                trace_timer.function = csl_trace_handle_timer;
                csl_trace_timer = &trace_timer;
#endif
            }
    
            memset (RecentCount, 0, sizeof(RecentCount));
            memset (csl_trace_count_saturated, 0, sizeof(csl_trace_count_saturated));

            for (count = CHAL_TRACE_AXITRACE0; count < CHAL_TRACE_MAX_AXITRACE; count++)
            {
                memset (&atm_config, 0, sizeof (CHAL_TRACE_ATM_CONF_t));
    
                chal_trace_axi_get_atm_config (csl_trace_handle, (CHAL_TRACE_AXITRACE_t) count, &atm_config);
#if defined(_RHEA_)
                atm_config.ALL_CYCLES_EN = all_cycle_enable;
#endif //_RHEA_
                atm_config.BUSY_EN = 1;
                atm_config.SAT_EN = 1;
                atm_config.CTRL_SRC = 0;
        
                chal_trace_axi_set_atm_config (csl_trace_handle, (CHAL_TRACE_AXITRACE_t) count, &atm_config);
            }

            chal_trace_globperf_set_cmd (csl_trace_handle, CHAL_TRACE_GLB_START_RESUME);

            status = CSL_TRACE_OK;
        
            if (csl_trace_timer)
            {
#ifndef UNDER_LINUX
                OSTIMER_Start( csl_trace_timer );
#else
                add_timer(&trace_timer);
#endif
            }
        }
    }

    return status;
}

//******************************************************************************
// 
//  Function Name: csl_trace_stop_perfcounter
//
//  Description:   Stop AXITRACE perf. counting
//
//******************************************************************************
CSL_TRACE_ERR_CODE_t csl_trace_stop_perfcounter ( void )
{
    UInt8 count;
    CHAL_TRACE_ATM_CONF_t atm_config;
    CSL_TRACE_ERR_CODE_t status = CSL_TRACE_FAIL;

    if (csl_trace_handle == NULL)
    {
        status = CSL_TRACE_NO_INIT;
    }
    else
    {
        if (csl_trace_timer)
        {
#ifndef UNDER_LINUX
            OSTIMER_Stop( csl_trace_timer );
#else
            del_timer(&trace_timer);
#endif
        }

        chal_trace_globperf_set_cmd (csl_trace_handle, CHAL_TRACE_GLB_STOP);    

        csl_trace_store_count ();

        for (count = CHAL_TRACE_AXITRACE0; count < CHAL_TRACE_MAX_AXITRACE; count++)
        {        
            memset (&atm_config, 0, sizeof (CHAL_TRACE_ATM_CONF_t));

            chal_trace_axi_get_atm_config (csl_trace_handle, (CHAL_TRACE_AXITRACE_t) count, &atm_config);
#if defined(_RHEA_)
            atm_config.ALL_CYCLES_EN = 0;
#endif //_RHEA_
            atm_config.BUSY_EN = 0;
            atm_config.SAT_EN = 0;
            atm_config.CTRL_SRC = 0;

            chal_trace_axi_set_atm_config (csl_trace_handle, (CHAL_TRACE_AXITRACE_t) count, &atm_config);
        }

        chal_trace_globperf_set_cmd (csl_trace_handle, CHAL_TRACE_GLB_CLEAR);

        status = CSL_TRACE_OK;

        if (csl_trace_timer)
        {
#ifndef UNDER_LINUX
            OSTIMER_Destroy( csl_trace_timer );
#endif
            csl_trace_timer = NULL;
        }
    }

    return status;
}

//******************************************************************************
// 
//  Function Name: csl_trace_get_chal_axi_type
//
//  Description:   This function returns the CHAL axi type from CSL axitrace_id
//                 in case when the definitions in CSL and cHAL does not match.
//
//******************************************************************************
static CHAL_TRACE_AXITRACE_t csl_trace_get_chal_axi_type(CSL_TRACE_AXITraceID_t axitrace_id)
{
    CHAL_TRACE_AXITRACE_t chal_axi_type = CHAL_TRACE_AXITRACE1;

    switch (axitrace_id)
    {
        case CSL_TRACE_AXITRACE1:
            chal_axi_type = CHAL_TRACE_AXITRACE1;
            break;

        case CSL_TRACE_AXITRACE2:
            chal_axi_type = CHAL_TRACE_AXITRACE2;
            break;

        case CSL_TRACE_AXITRACE3:
            chal_axi_type = CHAL_TRACE_AXITRACE3;
            break;

        case CSL_TRACE_AXITRACE4:
            chal_axi_type = CHAL_TRACE_AXITRACE4;
            break;

        case CSL_TRACE_AXITRACE5:
            chal_axi_type = CHAL_TRACE_AXITRACE5;
            break;

        case CSL_TRACE_AXITRACE6:
            chal_axi_type = CHAL_TRACE_AXITRACE6;
            break;

        case CSL_TRACE_AXITRACE7:
            chal_axi_type = CHAL_TRACE_AXITRACE7;
            break;

        case CSL_TRACE_AXITRACE8:
            chal_axi_type = CHAL_TRACE_AXITRACE8;
            break;

        case CSL_TRACE_AXITRACE9:
            chal_axi_type = CHAL_TRACE_AXITRACE9;
            break;

        case CSL_TRACE_AXITRACE10:
            chal_axi_type = CHAL_TRACE_AXITRACE10;
            break;

        case CSL_TRACE_AXITRACE11:
            chal_axi_type = CHAL_TRACE_AXITRACE11;
            break;
        case CSL_TRACE_AXITRACE12:
            chal_axi_type = CHAL_TRACE_AXITRACE12;
            break;

        case CSL_TRACE_AXITRACE13:
            chal_axi_type = CHAL_TRACE_AXITRACE13;
            break;

        case CSL_TRACE_AXITRACE14:
            chal_axi_type = CHAL_TRACE_AXITRACE14;
            break;

        case CSL_TRACE_AXITRACE15:
            chal_axi_type = CHAL_TRACE_AXITRACE15;
            break;

        case CSL_TRACE_AXITRACE16:
            chal_axi_type = CHAL_TRACE_AXITRACE16;
            break;

        case CSL_TRACE_AXITRACE17:
            chal_axi_type = CHAL_TRACE_AXITRACE17;
            break;

        case CSL_TRACE_AXITRACE18:
            chal_axi_type = CHAL_TRACE_AXITRACE18;
            break;

        case CSL_TRACE_AXITRACE19:
            chal_axi_type = CHAL_TRACE_AXITRACE19;
            break;

        default:
            /* invalid axitrace_id */
            xassert(FALSE, axitrace_id);
            break;

    }

    return chal_axi_type;
}

//******************************************************************************
// 
//  Function Name: csl_trace_get_chal_axi_type
//
//  Description:   This function returns the CHAL AXI performance active counter type 
//                 from AXI bus channel type
//
//******************************************************************************
static CHAL_TRACE_AXITRACE_COUNT_t csl_trace_get_chal_active_count_type(CSL_TRACE_AXIChannel_t  channel)
{
    CHAL_TRACE_AXITRACE_COUNT_t chal_axi_count_type = WRITE_ADDRESS_CYCLES;

    switch (channel)
    {
        case WRITEADDR:
            chal_axi_count_type = WRITE_ADDRESS_CYCLES;
            break;

        case READADDR:
            chal_axi_count_type = READ_ADDRESS_CYCLES;
            break;

        case WRITEDATA:
            chal_axi_count_type = WRITE_DATA_CYCLES;
            break;

        case READDATA:
            chal_axi_count_type = READ_DATA_CYCLES;
            break;

        case WRITERESPONSE:
            chal_axi_count_type = WRITE_RESPONSE_CYCLES;
            break;

        default:
            /* invalid channel */
            xassert(FALSE, channel);
            break;
    }

    return chal_axi_count_type;
}

//******************************************************************************
// 
//  Function Name: csl_trace_get_chal_axi_type
//
//  Description:   This function returns the CHAL AXI performance busy counter type 
//                 from AXI bus channel type
//
//******************************************************************************
static CHAL_TRACE_AXITRACE_COUNT_t csl_trace_get_chal_busy_count_type(CSL_TRACE_AXIChannel_t  channel)
{
    CHAL_TRACE_AXITRACE_COUNT_t chal_axi_count_type = WRITE_ADDRESS_BUSY_CYCLES;

    switch (channel)
    {
        case WRITEADDR:
            chal_axi_count_type = WRITE_ADDRESS_BUSY_CYCLES;
            break;

        case READADDR:
            chal_axi_count_type = READ_ADDRESS_BUSY_CYCLES;
            break;

        case WRITEDATA:
            chal_axi_count_type = WRITE_DATA_BUSY_CYCLES;
            break;

        case READDATA:
            chal_axi_count_type = READ_DATA_BUSY_CYCLES;
            break;

        case WRITERESPONSE:
            chal_axi_count_type = WRITE_RESPONSE_BUSY_CYCLES;
            break;

        default:
            /* invalid channel */
            xassert(FALSE, channel);
            break;
    }

    return chal_axi_count_type;
}

//******************************************************************************
// 
//  Function Name: csl_trace_handle_timer
//
//  Description: store the snapshot of the performance counters on the profiling timer expiration
//
//******************************************************************************
#ifndef UNDER_LINUX
static void csl_trace_handle_timer( TimerID_t id )
#else
static void csl_trace_handle_timer( unsigned long id )
#endif
{
    if (csl_trace_handle != NULL)
    {
        chal_trace_globperf_set_cmd (csl_trace_handle, CHAL_TRACE_GLB_STOP);    

        csl_trace_store_count ();

        chal_trace_globperf_set_cmd (csl_trace_handle, CHAL_TRACE_GLB_CLEAR);

        chal_trace_globperf_set_cmd (csl_trace_handle, CHAL_TRACE_GLB_START_RESUME);
    }
#ifdef UNDER_LINUX
    //reschedule the timer
    add_timer(&trace_timer);
#endif
}

//******************************************************************************
// 
//  Function Name: csl_trace_store_count
//
//  Description:   store the snapshot of the performance counters
//
//******************************************************************************
static void csl_trace_store_count ( void )
{

    CHAL_TRACE_AXITRACE_t chal_axi_type;
    CHAL_TRACE_AXITRACE_COUNT_t cycle;
    CSL_TRACE_AXITraceID_t count;

    for (count = CSL_TRACE_AXITRACE1; count < CSL_TRACE_AXITRACE_TOTAL; count++)
    {
        chal_axi_type = csl_trace_get_chal_axi_type(count);

        for (cycle = WRITE_ADDRESS_CYCLES; cycle <= WRITE_RESPONSE_BUSY_CYCLES; cycle++)
        {
            RecentCount[chal_axi_type][cycle] = chal_trace_axi_get_count(csl_trace_handle, chal_axi_type, cycle);
        }
        csl_trace_count_saturated[chal_axi_type] = chal_trace_axi_get_status_sat (csl_trace_handle, chal_axi_type);
    }
}
#endif // (CSL_TRACE_PERF_COUNT_SUPPORT)

#if defined (CSL_TRACE_AXI_TRACE_SUPPORT)
CSL_TRACE_ERR_CODE_t csl_trace_set_axi_trace( Boolean on, CSL_TRACE_AXITraceID_t axi_id, Boolean time_stamp )
{
    CHAL_TRACE_AXITRACE_t chal_axi_type;
    CHAL_TRACE_AXI_FILTER_CONF_t filter_config; 
    CHAL_TRACE_ATM_CONF_t atm_config;
    CSL_TRACE_ERR_CODE_t status = CSL_TRACE_OK;

    if (csl_trace_handle == NULL)
    {
        status = CSL_TRACE_NO_INIT;
    }
    else
    {
        filter_config.TRIGGER_EN = 0;
        filter_config.FILTER_SEC = 1;
        filter_config.FILTER_OPEN = 1;
        // Set the filter0 ID the same as axi_id for now
        filter_config.FILTER_ID = axi_id;
        filter_config.ID_MASK = axi_id;
        filter_config.FILTER_LEN = 0;
        filter_config.LEN_MODE = 3;
        filter_config.FILTER_WRITE = 1;
        filter_config.FILTER_READ = 1;
    
        chal_axi_type = csl_trace_get_chal_axi_type(axi_id);
    
        /* set TRACE_EN */
        chal_trace_axi_get_atm_config(csl_trace_handle, chal_axi_type, &atm_config);
        if (on)
        {
            atm_config.TRACE_EN = 1;
        }
        else
        {
            atm_config.TRACE_EN = 0;
        }
        chal_trace_axi_set_atm_config(csl_trace_handle, chal_axi_type, &atm_config);
    
        /* set filter0 */
        chal_trace_axi_set_filter (csl_trace_handle, chal_axi_type, 0, &filter_config, 0x0, 0xFFFFFF);
    
        /* set output ATB ID */
        if (axi_config[axi_id].atb_id_write != 0)
        {
            if (on)
            {
                if (time_stamp)
                {
                    chal_trace_axi_set_atm_outid (csl_trace_handle, chal_axi_type, ATB_ID_ODD(axi_config[axi_id].atb_id_write), ATB_ID_ODD(axi_config[axi_id].atb_id_read));
                }
                else
                {
                    chal_trace_axi_set_atm_outid (csl_trace_handle, chal_axi_type, ATB_ID_EVEN(axi_config[axi_id].atb_id_write), ATB_ID_EVEN(axi_config[axi_id].atb_id_read));
                }
            
                if (axi_config[axi_id].funnel_1 != INVALID_FUNNEL)
                {
                    csl_trace_funnel_set_enable(axi_config[axi_id].funnel_1, axi_config[axi_id].port_1, 1);
                }
            
                if (axi_config[axi_id].funnel_2 != INVALID_FUNNEL)
                {
                    csl_trace_funnel_set_enable(axi_config[axi_id].funnel_2, axi_config[axi_id].port_2, 1);
                }
    
                /* Set Enable */
                csl_trace_set_stm_en (axi_config[axi_id].atb_id_write, 1);
                csl_trace_set_stm_en (axi_config[axi_id].atb_id_read, 1);
        
                /* Set HW */
                csl_trace_set_stm_sw(axi_config[axi_id].atb_id_write, 0);
                csl_trace_set_stm_sw(axi_config[axi_id].atb_id_read, 0);
            }
            else
            {
                if (axi_config[axi_id].funnel_1 != INVALID_FUNNEL)
                {
                    csl_trace_funnel_set_enable(axi_config[axi_id].funnel_1, axi_config[axi_id].port_1, 0);
                }
    
                /* Set Disable */
                csl_trace_set_stm_en (axi_config[axi_id].atb_id_write, 0);
                csl_trace_set_stm_en (axi_config[axi_id].atb_id_read, 0);
            }
        }
    }

    return status;
}
#endif //(CSL_TRACE_AXI_TRACE_SUPPORT)

#if defined (CSL_TRACE_GIC_TRACE_SUPPORT)
CSL_TRACE_ERR_CODE_t csl_trace_set_gic_trace( Boolean on, Boolean time_stamp )
{
    CHAL_TRACE_GIC_CONF_t gic_config;
    CSL_TRACE_ERR_CODE_t status = CSL_TRACE_OK;

    if (csl_trace_handle == NULL)
    {
        status = CSL_TRACE_NO_INIT;
    }
    else
    {
        chal_trace_gic_get_config (csl_trace_handle, &gic_config);
        if (on)
        {
            gic_config.TRACE_EN = 1;
        }
        else
        {
            gic_config.TRACE_EN = 0;
        }
        chal_trace_gic_set_config (csl_trace_handle, &gic_config);
    
        if (on)
        {
#if defined(FUSE_APPS_PROCESSOR)
            if (time_stamp)
            {
                chal_trace_gic_set_outid (csl_trace_handle, ATB_ID_ODD(ATB_ID_GIC_A9));
            }
            else
            {
                chal_trace_gic_set_outid (csl_trace_handle, ATB_ID_EVEN(ATB_ID_GIC_A9));
            }
        
            /* Enable ARM Funnel port4 */
            csl_trace_funnel_set_enable(CHAL_TRACE_FUNNEL, 4, 1);
        
#if defined (_RHEA_)
            /* Enable FINAL Funnel port 1 */
            csl_trace_funnel_set_enable(CHAL_TRACE_FIN_FUNNEL, 1, 1);
#elif defined (_SAMOA_)
            /* Enable HUB Funnel port 1 */
            csl_trace_funnel_set_enable(CHAL_TRACE_HUB_FUNNEL, 1, 1);
#endif
    
            /* Set Enable */
            csl_trace_set_stm_en (ATB_ID_GIC_A9, 1);
    
            /* Set HW */
            csl_trace_set_stm_sw(ATB_ID_GIC_A9, 0);
#else
            if (time_stamp)
            {
                chal_trace_gic_set_outid (csl_trace_handle, ATB_ID_ODD(ATB_ID_GIC_R4));
            }
            else
            {
                chal_trace_gic_set_outid (csl_trace_handle, ATB_ID_EVEN(ATB_ID_GIC_R4));
            }
    
            /* Enable BMODEM Funnel port5 */
            csl_trace_funnel_set_enable(CHAL_TRACE_COMMS_FUNNEL, 5, 1);
            
#if defined (_RHEA_)
            /* Enable FINAL Funnel port 4 */
            csl_trace_funnel_set_enable(CHAL_TRACE_FIN_FUNNEL, 4, 1);
#elif defined (_SAMOA_)
            /* Enable HUB Funnel port 4 */
            csl_trace_funnel_set_enable(CHAL_TRACE_HUB_FUNNEL, 4, 1);
#endif

            /* Set Enable for R4 GIC */
            csl_trace_set_stm_en (ATB_ID_GIC_R4, 1);
    
            /* Set HW for R4GIC */
            csl_trace_set_stm_sw(ATB_ID_GIC_R4, 0);
#endif
        }
        else
        {
#if defined(FUSE_APPS_PROCESSOR)
            /* Disable ARM Funnel port4 */
            csl_trace_funnel_set_enable(CHAL_TRACE_FUNNEL, 4, 0);
    
            /* Set Disable */
            csl_trace_set_stm_en (ATB_ID_GIC_A9, 0);
#else
            /* Disable BMODEM Funnel port5 */
            csl_trace_funnel_set_enable(CHAL_TRACE_COMMS_FUNNEL, 5, 0);

            /* Set Disable */
            csl_trace_set_stm_en (ATB_ID_GIC_R4, 0);
#endif
        }
    }

    return status;
}
#endif // (CSL_TRACE_GIC_TRACE_SUPPORT)

#if defined (CSL_TRACE_PWRMGR_TRACE_SUPPORT)
CSL_TRACE_ERR_CODE_t csl_trace_set_powermgr( Boolean on, CSL_TRACE_pwr_mgr_mode_t mode )
{
    CSL_TRACE_ERR_CODE_t status = CSL_TRACE_OK;

    if (csl_trace_handle == NULL)
    {
        status = CSL_TRACE_NO_INIT;
    }
    else
    {
        if (on)
        {
            /* Set PM ATB trace ID */
            /* Bit 24 of ATB_ID selects Mode A or Mode B (0 = Mode A, 1 = Mode B) */
            if (mode == PWRMGR_TRACE_MODE_A)
            {
                chal_trace_pwrmgr_set_atb_id (csl_trace_handle, ATB_ID_EVEN(ATB_ID_POWER_MGR));
            }
            else
            {
                chal_trace_pwrmgr_set_atb_id (csl_trace_handle, ATB_ID_ODD(ATB_ID_POWER_MGR));
            }
#if defined (_RHEA_)
            /* Enable HUB Funnel port6 */
            csl_trace_funnel_set_enable(CHAL_TRACE_HUB_FUNNEL, 6, 1);
        
            /* Enable FINAL Funnel port 0 */
            csl_trace_funnel_set_enable(CHAL_TRACE_FIN_FUNNEL, 0, 1);
#elif defined (_SAMOA_)
            /* Enable HUB Funnel port 0 */
            csl_trace_funnel_set_enable(CHAL_TRACE_HUB_FUNNEL, 0, 1);
#endif
    
            /* Set Enable */
            csl_trace_set_stm_en (ATB_ID_POWER_MGR, 1);
    
            /* Set HW */
            csl_trace_set_stm_sw(ATB_ID_POWER_MGR, 0);
        }
        else
        {
            /* if PM ATB trace ID set to zero then trace is disabled 
*/
            chal_trace_pwrmgr_set_atb_id (csl_trace_handle, 0);
    
#if defined (_RHEA_)
            /* Disable HUB Funnel port6 */
            csl_trace_funnel_set_enable(CHAL_TRACE_HUB_FUNNEL, 6, 0);
#elif defined (_SAMOA_)
            /* Disable HUB Funnel port 0 */
            csl_trace_funnel_set_enable(CHAL_TRACE_HUB_FUNNEL, 0, 0);
#endif
    
            /* Set Disable */
            csl_trace_set_stm_en (ATB_ID_POWER_MGR, 0);
        }
    }

    return status;
}
#endif // (CSL_TRACE_PWRMGR_TRACE_SUPPORT)

#if defined (CSL_TRACE_STM_SUPPORT) || defined (CSL_TRACE_AXI_TRACE_SUPPORT) || defined (CSL_TRACE_GIC_TRACE_SUPPORT) || defined (CSL_TRACE_PWRMGR_TRACE_SUPPORT)
//******************************************************************************
// 
//  Function Name: csl_trace_funnel_set_enable
//
//  Description:   Function enable/disable funnel port.
//
//  NOTE: If the write access is locked, unlock it first.
//
//******************************************************************************
static Boolean csl_trace_funnel_set_enable (CHAL_TRACE_FUNNEL_t funnel_type, cUInt8 port_n, cBool enable)
{
    Boolean status;

    if (chal_trace_funnel_locked(csl_trace_handle, funnel_type))
    {
        chal_trace_funnel_set_lock_access(csl_trace_handle, funnel_type, LOCK_ACCESS_CODE);
    }
    
    status = chal_trace_funnel_set_enable(csl_trace_handle, funnel_type, port_n, enable);
    return status;
}
#endif // (CSL_TRACE_STM_SUPPORT) ||  (CSL_TRACE_AXI_TRACE_SUPPORT) ||  (CSL_TRACE_GIC_TRACE_SUPPORT) ||  (CSL_TRACE_PWRMGR_TRACE_SUPPORT)

#if defined (CSL_TRACE_STM_SUPPORT)
CSL_TRACE_ERR_CODE_t csl_trace_set_sw_stm( Boolean on )
{
    CSL_TRACE_ERR_CODE_t status = CSL_TRACE_OK;

    if (csl_trace_handle == NULL)
    {
        status = CSL_TRACE_NO_INIT;
    }
    else
    {
        if (on)
        {
#if defined(FUSE_APPS_PROCESSOR)
            /* SW STM Config */
            chal_trace_sw_stm_set_config (csl_trace_handle, CHAL_TRACE_SWSTM, 1, ATB_ID_SW_STM_A9);
        
            /* Enable ARM Funnel port6 */
            csl_trace_funnel_set_enable(CHAL_TRACE_FUNNEL, 6, 1);
        
#if defined (_RHEA_)
            /* Enable FINAL Funnel port 1 */
            csl_trace_funnel_set_enable(CHAL_TRACE_FIN_FUNNEL, 1, 1);
#elif defined (_SAMOA_)
            /* Enable HUB Funnel port 1 */
            csl_trace_funnel_set_enable(CHAL_TRACE_HUB_FUNNEL, 1, 1);
#endif
    
            /* Set Enable for SWSTM */
            csl_trace_set_stm_en (ATB_ID_SW_STM_A9, 1);
    
            /* Set SW for SWSTM */
            csl_trace_set_stm_sw(ATB_ID_SW_STM_A9, 1);
#else
            /* SW STM Config */
            chal_trace_sw_stm_set_config (csl_trace_handle, CHAL_TRACE_SWSTM, 1, ATB_ID_SW_STM_R4);
         
            /* Enable BMODEM Funnel port3 */
            csl_trace_funnel_set_enable(CHAL_TRACE_COMMS_FUNNEL, 3, 1);
        
#if defined (_RHEA_)
            /* Enable FINAL Funnel port 4 */
            csl_trace_funnel_set_enable(CHAL_TRACE_FIN_FUNNEL, 4, 1);
#elif defined (_SAMOA_)
            /* Enable HUB Funnel port 4 */
            csl_trace_funnel_set_enable(CHAL_TRACE_HUB_FUNNEL, 4, 1);
#endif
    
            /* Set Enable for R4 SWSTM */
            csl_trace_set_stm_en (ATB_ID_SW_STM_R4, 1);
    
            /* Set SW for R4 SWSTM */
            csl_trace_set_stm_sw(ATB_ID_SW_STM_R4, 1);
#endif
        }
        else
        {
#if defined(FUSE_APPS_PROCESSOR)
            /* Disable ARM Funnel port6 */
            csl_trace_funnel_set_enable(CHAL_TRACE_FUNNEL, 6, 0);
    
            /* Set Disable for SWSTM */
            csl_trace_set_stm_en (ATB_ID_SW_STM_A9, 0);
#else
            /* Disable BMODEM Funnel port3 */
            csl_trace_funnel_set_enable(CHAL_TRACE_COMMS_FUNNEL, 3, 0);

            /* Set Disable for SWSTM */
            csl_trace_set_stm_en (ATB_ID_SW_STM_R4, 0);
#endif
        }
    }

    return status;
}

#ifdef CNEON_COMMON
CSL_TRACE_ERR_CODE_t
csl_trace_stm_write(UInt8 ch, Boolean ts, UInt32 len, UInt8 *data)
{
    CHAL_TRACE_SWSTM_t st;

    if (csl_trace_handle == NULL)
    {
        return CSL_TRACE_NO_INIT;
    }

    if(!data)
        return CSL_TRACE_INVALID_PARM;

    if (ts)
        st = CHAL_TRACE_SWSTM_ST;
    else
        st = CHAL_TRACE_SWSTM;

    /* Align start before writing 4 bytes at a time */
    while ((len > 0) && (((UInt32)data) & (sizeof (UInt32)-1)))
    {
        chal_trace_sw_stm_write (csl_trace_handle, st, ch, 1, *data);
        data++;
        len--;
    }

    /* Write 4 bytes at a time while we can */
    while (len >= 4)
    {
        UInt32 tmp = 0;
        tmp = ((((UInt32)data[0]) << 24) |
               (((UInt32)data[1]) << 16) |
               (((UInt32)data[2]) <<  8) |
               (((UInt32)data[3]) <<  0));
        chal_trace_sw_stm_write (csl_trace_handle, st, ch, 4, tmp);
        data += 4;
        len  -= 4;
    }

    /* Finish remaining bytes */
    while (len > 0)
    {
        chal_trace_sw_stm_write (csl_trace_handle, st, ch, 1, *data);
        data++;
        len--;
    }

    return CSL_TRACE_OK;
}
#endif /* CNEON_COMMON */


static void csl_trace_set_stm_en( UInt8 bit_mask, Boolean set )
{
    UInt32 reg_high, reg_low;

    reg_high = chal_trace_atb_stm_get_en (csl_trace_handle, 1);
    reg_low = chal_trace_atb_stm_get_en (csl_trace_handle, 0);

    if (bit_mask <32)
    {
        if (set)
        {
            reg_low |= (1<<bit_mask);
        }
        else
        {
            reg_low &= ~(1<<bit_mask);
        }
    }
    else if (bit_mask <64)
    {
        if (set)
        {
            reg_high |= (1<<(bit_mask-32));
        }
        else
        {
            reg_high &= ~(1<<(bit_mask-32));
        }
    }

    chal_trace_atb_stm_set_en (csl_trace_handle, reg_low, reg_high);
}

static void csl_trace_set_stm_sw( UInt8 bit_mask, Boolean set )
{
    UInt32 reg_high, reg_low;

    reg_high = chal_trace_atb_stm_get_sw (csl_trace_handle, 1);
    reg_low = chal_trace_atb_stm_get_sw (csl_trace_handle, 0);

    if (bit_mask <32)
    {
        if (set)
        {
            reg_low |= (1<<bit_mask);
        }
        else
        {
            reg_low &= ~(1<<bit_mask);
        }
    }
    else if (bit_mask <64)
    {
        if (set)
        {
            reg_high |= (1<<(bit_mask-32));
        }
        else
        {
            reg_high &= ~(1<<(bit_mask-32));
        }
    }

    chal_trace_atb_stm_set_sw (csl_trace_handle, reg_low, reg_high);
}

#endif // (CSL_TRACE_STM_SUPPORT)

#ifdef UNDER_LINUX
/*
 * Trace send bytes and STM macros from Sebastian
 */

#define L_STM_BEGIN(trace_type, channel)                                    \
{                                                                           \
    Int8 data = trace_type;                                                 \
    csl_trace_stm_write (channel, FALSE, 1, &data);                         \
}


#define L_STM_WRITE(channel, datalen, data)                                 \
                 csl_trace_stm_write (channel, FALSE, datalen, data)


#define L_STM_WRITE_TS(channel, datalen, data)                              \
                 csl_trace_stm_write (channel, TRUE, datalen, data)

#define  L_STM_TERMINATE(channel)                                           \
       {                                                                    \
           UInt8 data = 0;                                                  \
           csl_trace_stm_write (channel, TRUE, 1, &data);                   \
       }

void XTRACE_SEND_BYTES(UInt16 length, void *data_ptr)
{
    UInt8 trace_type = 0x72; //make it configurable
    UInt8 channel = 1;
    unsigned long flags;
    char data[256]; // length must be < 256-19 or this will overrun

    //setup OST header (magic numbers copied from test script)
    data[0] = 0x00;
    data[1] = 0x00;
    data[2] = 0x0c;
    data[3] = 0x01;
    data[4] = 0x00;
    data[5] = 0x1f;
    data[6] = 0xa5;
    data[7] = 0xc3;
    data[8] = 0xbb;
    data[9] = 0x01;
    data[10] = 0x00;
    data[11] = 0x11;
    data[12] = 0x66;
    data[13] = 0xb3;
    data[14] = 0x80;
    data[15] = 0x02;
    data[16] = 0x00;    // Length of
    data[17] = 0x13;    // payload
    data[18] = 0xe3;    //  CRC of payload

    //copy in data_ptr
    strncpy(&data[19], &((char*)data_ptr)[0], length);

    local_irq_save(flags);
    L_STM_BEGIN(trace_type, channel);
    L_STM_WRITE(channel, (length + 19), data);
    L_STM_TERMINATE(channel);
    local_irq_restore(flags);
}

/*
 * Small test function to send data to BMTT via FIDO
 */

static int __init csl_trace_test(void)
{
    char string[64] = "Broadcom Trace Test\n";

    XTRACE_SEND_BYTES(20, &string[0]);

    pr_info("Test Trace String %s\n", &string[0]);

    return 0;
}

arch_initcall(csl_trace_init);

late_initcall(csl_trace_test);

#endif
