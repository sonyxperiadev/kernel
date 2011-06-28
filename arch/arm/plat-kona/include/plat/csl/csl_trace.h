/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2011  Broadcom Corporation                                                        */
/*                                                                                              */
/*     Unless you and Broadcom execute a separate written software license agreement governing  */
/*     use of this software, this software is licensed to you under the terms of the GNU        */
/*     General Public License version 2 (the GPL), available at                                 */
/*                                                                                              */
/*          http://www.broadcom.com/licenses/GPLv2.php                                          */
/*                                                                                              */
/*     with the following added to such license:                                                */
/*                                                                                              */
/*     As a special exception, the copyright holders of this software give you permission to    */
/*     link this software with independent modules, and to copy and distribute the resulting    */
/*     executable under terms of your choice, provided that you also meet, for each linked      */
/*     independent module, the terms and conditions of the license of that module.              */
/*     An independent module is a module which is not derived from this software.  The special  */
/*     exception does not apply to any modifications of the software.                           */
/*                                                                                              */
/*     Notwithstanding the above, under no circumstances may you combine this software in any   */
/*     way with any other Broadcom software provided under a license other than the GPL,        */
/*     without Broadcom's express prior written consent.                                        */
/*                                                                                              */
/************************************************************************************************/

/**
*
*   @file   csl_trace.h
*
*   @brief  APIs for trace/debug module
*
****************************************************************************/
#ifndef __CSL_TRACE_H__
#define __CSL_TRACE_H__

#ifdef __cplusplus
extern "C" {
#endif

#if defined (_HERA_)
#define CSL_TRACE_PERF_COUNT_SUPPORT
#undef CSL_TRACE_STM_SUPPORT
#undef CSL_TRACE_AXI_TRACE_SUPPORT
#undef CSL_TRACE_GIC_TRACE_SUPPORT
#undef CSL_TRACE_PWRMGR_TRACE_SUPPORT

#elif defined (_RHEA_)

#define CSL_TRACE_PERF_COUNT_SUPPORT
#define CSL_TRACE_STM_SUPPORT
#define CSL_TRACE_AXI_TRACE_SUPPORT
#define CSL_TRACE_GIC_TRACE_SUPPORT
#define CSL_TRACE_PWRMGR_TRACE_SUPPORT

#elif defined (_SAMOA_)

#undef CSL_TRACE_PERF_COUNT_SUPPORT
#define CSL_TRACE_STM_SUPPORT
#undef CSL_TRACE_AXI_TRACE_SUPPORT
#define CSL_TRACE_GIC_TRACE_SUPPORT
#define CSL_TRACE_PWRMGR_TRACE_SUPPORT

#else

#error platform not supported!

#endif

/* ATB ID definitions */
enum 
{
    ATB_ID_SW_STM_A9 = 1,
    ATB_ID_SW_STM_R4 = 2,
    ATB_ID_RESERVED1 = 3,
    ATB_ID_RESERVED2 = 4,
    ATB_ID_STM = 5,
    ATB_ID_AXI1_READ = 6,
    ATB_ID_AXI1_WRITE = 7,
    ATB_ID_AXI2_READ = 8,
    ATB_ID_AXI2_WRITE = 9,
    ATB_ID_AXI3_READ = 10,
    ATB_ID_AXI3_WRITE = 11,
    ATB_ID_AXI4_READ = 12,
    ATB_ID_AXI4_WRITE = 13,
    ATB_ID_AXI5_READ = 14,
    ATB_ID_AXI5_WRITE = 15,
    ATB_ID_AXI6_READ = 16,
    ATB_ID_AXI6_WRITE = 17,
    ATB_ID_AXI7_READ = 18,
    ATB_ID_AXI7_WRITE = 19,
    ATB_ID_AXI8_READ = 20,
    ATB_ID_AXI8_WRITE = 21,
    ATB_ID_AXI9_READ = 22,
    ATB_ID_AXI9_WRITE = 23,
    ATB_ID_AXI10_READ = 24,
    ATB_ID_AXI10_WRITE = 25,
    ATB_ID_AXI11_READ = 26,
    ATB_ID_AXI11_WRITE = 27,
    ATB_ID_AXI12_READ = 28,
    ATB_ID_AXI12_WRITE = 29,
    ATB_ID_AXI13_READ = 30,
    ATB_ID_AXI13_WRITE = 31,
    ATB_ID_AXI14_READ = 32,
    ATB_ID_AXI14_WRITE = 33,
    ATB_ID_AXI15_READ = 34,
    ATB_ID_AXI15_WRITE = 35,
    ATB_ID_AXI16_READ = 36,
    ATB_ID_AXI16_WRITE = 37,
    ATB_ID_AXI17_READ = 38,
    ATB_ID_AXI17_WRITE = 39,
    ATB_ID_AXI18_READ = 40,
    ATB_ID_AXI18_WRITE = 41,
    ATB_ID_AXI19_READ = 42,
    ATB_ID_AXI19_WRITE = 43,
    ATB_ID_GIC_A9 = 44,
    ATB_ID_GIC_R4 = 45,
    ATB_ID_POWER_MGR = 46,
    ATB_ID_MAX = 0x3F, /* max 6bit ATB_ID */
};

typedef enum
{
  CSL_TRACE_OK,
  CSL_TRACE_FAIL,
  CSL_TRACE_NO_INIT, /* CSL not initialized */
  CSL_TRACE_INIT, /* CSL already initialized */
  CSL_TRACE_INVALID_PARM, /* invalid parameter */
  CSL_TRACE_SATURATION, /* peformance counter saturation occured */
} CSL_TRACE_ERR_CODE_t;

typedef enum
{
  CSL_TRACE_AXITRACE1 = 1,
  CSL_TRACE_AXITRACE2,
  CSL_TRACE_AXITRACE3,
  CSL_TRACE_AXITRACE4,
  CSL_TRACE_AXITRACE5,
  CSL_TRACE_AXITRACE6,
  CSL_TRACE_AXITRACE7,
  CSL_TRACE_AXITRACE8,
  CSL_TRACE_AXITRACE9,
  CSL_TRACE_AXITRACE10,
  CSL_TRACE_AXITRACE11,
  CSL_TRACE_AXITRACE12,
  CSL_TRACE_AXITRACE13,
  CSL_TRACE_AXITRACE14,
  CSL_TRACE_AXITRACE15,
  CSL_TRACE_AXITRACE16,
  CSL_TRACE_AXITRACE17,
  CSL_TRACE_AXITRACE18,
  CSL_TRACE_AXITRACE19,
  CSL_TRACE_AXITRACE_ALL,
  CSL_TRACE_AXITRACE_TOTAL = CSL_TRACE_AXITRACE_ALL,
} CSL_TRACE_AXITraceID_t;

typedef enum
{
  WRITEADDR,
  READADDR,
  WRITEDATA,
  READDATA,
  WRITERESPONSE,
} CSL_TRACE_AXIChannel_t  ;

typedef enum
{
  PWRMGR_TRACE_MODE_A = 0, /* Outputs PM/CCU interface signals (voltage request, policy info) */
  PWRMGR_TRACE_MODE_B = 1, /* Outputs PM I2C interface signals and 16-bit PM debug bus */
} CSL_TRACE_pwr_mgr_mode_t  ;

typedef struct
{
   UInt32 Util[CSL_TRACE_AXITRACE_TOTAL];
} CSL_TRACE_Report_st_t;

#define CSL_TRACE_INVALID_PCT 0xFFFFFFFF

/**
*
*  @brief  Initialize CSL for trace/debug module
*
*  @param  None
*
*  @return CSL_TRACE_ERR_CODE_t CSL_TRACE_OK if successfull; error code otherwise
*
*****************************************************************************/
#ifdef UNDER_LINUX
    static int __init csl_trace_init(void);
#else
    CSL_TRACE_ERR_CODE_t csl_trace_init ( void );
#endif

#if defined(CSL_TRACE_PERF_COUNT_SUPPORT)
/**
*
*  @brief  Get recent active counter of specific AXITRACE.
*
*  @param  axitrace_id - index of the axi_trace module.
*  @param  channel - AXI bus channel type
*  @param  pActiveCount - pointer of a structure CSL_TRACE_Report_st_t where the active count will be stored.
*
*  @return CSL_TRACE_ERR_CODE_t CSL_TRACE_OK if successfull; error code otherwise
*
*****************************************************************************/
CSL_TRACE_ERR_CODE_t csl_trace_get_recent_active_count ( CSL_TRACE_AXITraceID_t axitrace_id, CSL_TRACE_AXIChannel_t  channel,  CSL_TRACE_Report_st_t *pActiveCount );

/**
*
*  @brief  Get recent busy counter of specific AXITRACE.
*
*  @param  axitrace_id - index of the axi_trace module.
*  @param  channel - AXI bus channel type
*  @param  pBusyCount - pointer of a structure CSL_TRACE_Report_st_t where the busy count will be stored.
*                  utilization percentage will be stored as the unit of basis point (0.01%)
*
*  @return CSL_TRACE_ERR_CODE_t CSL_TRACE_OK if successfull; error code otherwise
*
*****************************************************************************/
CSL_TRACE_ERR_CODE_t csl_trace_get_recent_busy_count ( CSL_TRACE_AXITraceID_t axitrace_id, CSL_TRACE_AXIChannel_t  channel,  CSL_TRACE_Report_st_t *pBusyCount );

/**
*
*  @brief  Start AXITRACE perf. Counting.
*
*  @param  period_ms - Profiling periodic time interval in ms
*  @param  all_cycle_enable - When set, BCYCLES counts all cycles regardless of any handshakes
*
*  @return CSL_TRACE_ERR_CODE_t CSL_TRACE_OK if successfull; error code otherwise
*
*****************************************************************************/
CSL_TRACE_ERR_CODE_t csl_trace_start_perfcounter ( UInt32 period_ms, Boolean all_cycle_enable );

/**
*
*  @brief  Stop AXITRACE perf. counting
*
*  @param  None
*
*  @return CSL_TRACE_ERR_CODE_t CSL_TRACE_OK if successfull; error code otherwise
*
*****************************************************************************/
CSL_TRACE_ERR_CODE_t csl_trace_stop_perfcounter ( void );
#endif // (CSL_TRACE_PERF_COUNT_SUPPORT)

#if defined (CSL_TRACE_AXI_TRACE_SUPPORT)
/**
*
*  @brief  This function enables or disables the AXI trace and configures the STM and funnel accordingly.
*
*  @param  on - 1 turns on the trace, 0 turns off the trace.
*  @param  axi_id - AXITRACE ID (1 ~ 19).
*  @param  time_stamp - This bit sets the bit 0 of the ATB ID
*
*  @return CSL_TRACE_ERR_CODE_t CSL_TRACE_OK if successfull; error code otherwise
*
*****************************************************************************/
CSL_TRACE_ERR_CODE_t csl_trace_set_axi_trace( Boolean on, CSL_TRACE_AXITraceID_t axi_id, Boolean time_stamp );
#endif //(CSL_TRACE_AXI_TRACE_SUPPORT)

#if defined (CSL_TRACE_GIC_TRACE_SUPPORT)
/**
*
*  @brief  This function enables or disables the GIC trace and configures the STM and funnel accordingly.
*
*  @param  on - 1 turns on the trace, 0 turns off the trace.
*  @param  time_stamp - This bit sets the bit 0 of the ATB ID
*
*  @return CSL_TRACE_ERR_CODE_t CSL_TRACE_OK if successfull; error code otherwise
*
*****************************************************************************/
CSL_TRACE_ERR_CODE_t csl_trace_set_gic_trace( Boolean on, Boolean time_stamp );
#endif // (CSL_TRACE_GIC_TRACE_SUPPORT)

#if defined (CSL_TRACE_PWRMGR_TRACE_SUPPORT)
/**
*
*  @brief  This function enables or disables the power manager trace and configures the STM and funnel accordingly. 
*
*  @param  on - 1 turns on the trace, 0 turns off the trace.
*  @param  mode - This bit sets the bit 0 of the ATB ID.
*                 If the bit 0 of the ATB  ID is zero, the output will be PM/CCU interface signals (voltage request, policy info).
*                 If 1, the output will be PM I2C interface signals and 16-bit PM debug bus.
*
*  @return CSL_TRACE_ERR_CODE_t CSL_TRACE_OK if successfull; error code otherwise
*
*****************************************************************************/
CSL_TRACE_ERR_CODE_t csl_trace_set_powermgr( Boolean on, CSL_TRACE_pwr_mgr_mode_t mode );
#endif // (CSL_TRACE_PWRMGR_TRACE_SUPPORT)

#if defined (CSL_TRACE_STM_SUPPORT)
/**
*
*  @brief  This function enables or disables the SW STM trace and configures the STM and funnel accordingly. 
*
*  @param  on - 1 turns on the trace, 0 turns off the trace.
*
*  @return CSL_TRACE_ERR_CODE_t CSL_TRACE_OK if successfull; error code otherwise
*
*****************************************************************************/
CSL_TRACE_ERR_CODE_t csl_trace_set_sw_stm( Boolean on );

#ifdef CNEON_COMMON
/**
*
*  @brief  This function writes data to the SW STM trace
*
*  @param  ch - stm logging channel
*
*  @param  ts - 1 include timestamp, 0 do not include timestamp
*
*  @param  len - length of data
*
*  @param  data - pointer to data to send
*
*  @return CSL_TRACE_ERR_CODE_t CSL_TRACE_OK if successfull; error code otherwise
*
*****************************************************************************/
CSL_TRACE_ERR_CODE_t csl_trace_stm_write(UInt8 ch, Boolean ts, UInt32 len, UInt8 *data);
#endif /* CNEON_COMMON */
#endif // (CSL_TRACE_STM_SUPPORT)

#ifdef __cplusplus
}
#endif

#endif // __CSL_TRACE_H__

