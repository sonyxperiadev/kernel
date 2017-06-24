/*
 * Copyright (c) 2012-2016 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
 *
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */

#if !defined( __VOS_API_H )
#define __VOS_API_H

/**=========================================================================
  
  \file  vos_Api.h
  
  \brief virtual Operating System Services (vOSS) API
               
   Header file that inludes all the vOSS API definitions.
  
  
  ========================================================================*/
 /*=========================================================================== 

                       EDIT HISTORY FOR FILE 
   
   
  This section contains comments describing changes made to the module. 
  Notice that changes are listed in reverse chronological order. 
   
   
  $Header:$ $DateTime: $ $Author: $ 
   
   
  when        who    what, where, why 
  --------    ---    --------------------------------------------------------
  06/23/08    hba     Added vos_preOpen()
  05/18/08    lac     Created module. 
===========================================================================*/

/*--------------------------------------------------------------------------
  Include Files
  ------------------------------------------------------------------------*/
// one stop shopping.  This brings in the entire vOSS API.
#include <vos_types.h>
#include <vos_status.h>
#include <vos_memory.h>
#include <vos_list.h>
#include <vos_getBin.h>
#include <vos_trace.h>
#include <vos_event.h>
#include <vos_lock.h>
#include <vos_nvitem.h>
#include <vos_mq.h>
#include <vos_packet.h>
#include <vos_threads.h>
#include <vos_timer.h>
#include <vos_pack_align.h>

/**
 * enum userspace_log_level - Log level at userspace
 * @LOG_LEVEL_NO_COLLECTION: verbose_level 0 corresponds to no collection
 * @LOG_LEVEL_NORMAL_COLLECT: verbose_level 1 correspond to normal log level,
 * with minimal user impact. this is the default value
 * @LOG_LEVEL_ISSUE_REPRO: verbose_level 2 are enabled when user is lazily
 * trying to reproduce a problem, wifi performances and power can be impacted
 * but device should not otherwise be significantly impacted
 * @LOG_LEVEL_ACTIVE: verbose_level 3+ are used when trying to
 * actively debug a problem
 *
 * Various log levels defined in the userspace for logging applications
 */
enum userspace_log_level {
    LOG_LEVEL_NO_COLLECTION,
    LOG_LEVEL_NORMAL_COLLECT,
    LOG_LEVEL_ISSUE_REPRO,
    LOG_LEVEL_ACTIVE,
};

/**
 * enum wifi_driver_log_level - Log level defined in the driver for logging
 * @WLAN_LOG_LEVEL_OFF: No logging
 * @WLAN_LOG_LEVEL_NORMAL: Default logging
 * @WLAN_LOG_LEVEL_REPRO: Normal debug level
 * @WLAN_LOG_LEVEL_ACTIVE: Active debug level
 *
 * Log levels defined for logging by the wifi driver
 */
enum wifi_driver_log_level {
    WLAN_LOG_LEVEL_OFF,
    WLAN_LOG_LEVEL_NORMAL,
    WLAN_LOG_LEVEL_REPRO,
    WLAN_LOG_LEVEL_ACTIVE,
};

/**
 * enum wifi_logging_ring_id - Ring id of logging entities
 * @RING_ID_WAKELOCK:         Power events ring id
 * @RING_ID_CONNECTIVITY:     Connectivity event ring id
 * @RING_ID_PER_PACKET_STATS: Per packet statistic ring id
 *
 * This enum has the ring id values of logging rings
 */
enum wifi_logging_ring_id {
    RING_ID_WAKELOCK,
    RING_ID_CONNECTIVITY,
    RING_ID_PER_PACKET_STATS,
};

/* 15 Min */
#define WLAN_POWER_COLLAPSE_FAIL_THRESHOLD  (1000 * 60 * 15)
/**
 * enum log_event_type - Type of event initiating bug report
 * @WLAN_LOG_TYPE_NON_FATAL: Non fatal event
 * @WLAN_LOG_TYPE_FATAL: Fatal event
 *
 * Enum indicating the type of event that is initiating the bug report
 */
enum log_event_type {
	WLAN_LOG_TYPE_NON_FATAL,
	WLAN_LOG_TYPE_FATAL,
};

/**
 * enum log_event_indicator - Module triggering bug report
 * @WLAN_LOG_INDICATOR_UNUSED: Unused
 * @WLAN_LOG_INDICATOR_FRAMEWORK: Framework triggers bug report
 * @WLAN_LOG_INDICATOR_HOST_DRIVER: Host driver triggers bug report
 * @WLAN_LOG_INDICATOR_FIRMWARE: FW initiates bug report
 * @WLAN_LOG_INDICATOR_IOCTL: Bug report is initiated by IOCTL
 * @WLAN_LOG_INDICATOR_HOST_ONLY: Host initiated and only Host
 * logs are needed
 *
 * Enum indicating the module that triggered the bug report
 */
enum log_event_indicator {
	WLAN_LOG_INDICATOR_UNUSED,
	WLAN_LOG_INDICATOR_FRAMEWORK,
	WLAN_LOG_INDICATOR_HOST_DRIVER,
	WLAN_LOG_INDICATOR_FIRMWARE,
	WLAN_LOG_INDICATOR_IOCTL,
	WLAN_LOG_INDICATOR_HOST_ONLY,
};

/**
 * enum log_event_host_reason_code - Reason code for bug report
 * @WLAN_LOG_REASON_CODE_UNUSED: Unused
 * @WLAN_LOG_REASON_ROAM_FAIL: Driver initiated roam has failed
 * @WLAN_LOG_REASON_THREAD_STUCK: Monitor Health of host threads and report
 * fatal event if some thread is stuck
 * @WLAN_LOG_REASON_DATA_STALL: Unable to send/receive data due to low resource
 * scenario for a prolonged period
 * @WLAN_LOG_REASON_SME_COMMAND_STUCK: SME command is stuck in SME active queue
 * @WLAN_LOG_REASON_QUEUE_FULL: Defer queue becomes full for a prolonged period
 * @WLAN_LOG_REASON_POWER_COLLAPSE_FAIL: Unable to allow apps power collapse
 * for a prolonged period
 * @WLAN_LOG_REASON_MALLOC_FAIL: Memory allocation Fails
 * @WLAN_LOG_REASON_VOS_MSG_UNDER_RUN: VOS Core runs out of message wrapper
 * @WLAN_LOG_REASON_IOCTL: Initiated by IOCTL
 * @WLAN_LOG_REASON_CODE_FRAMEWORK: Initiated by framework
 * @WLAN_LOG_REASON_DEL_BSS_STA_FAIL: DEL BSS/STA rsp is failure
 * @WLAN_LOG_REASON_ADD_BSS_STA_FAIL: ADD BSS/STA rsp is failure
 * @WLAN_LOG_REASON_ENTER_IMPS_BMPS_FAIL: Enter IMPS/BMPS rsp failure
 * @WLAN_LOG_REASON_EXIT_IMPS_BMPS_FAIL: Exit IMPS/BMPS rsp failure
 * @WLAN_LOG_REASON_HDD_TIME_OUT: Wait for event Timeout in HDD layer
 * @WLAN_LOG_REASON_MGMT_FRAME_TIMEOUT:Management frame timedout
 * @WLAN_LOG_REASON_SME_OUT_OF_CMD_BUFL sme out of cmd buffer
 * @WLAN_LOG_REASON_SCAN_NOT_ALLOWED: scan not allowed due to connection states
 * This enum contains the different reason codes for bug report
 */
enum log_event_host_reason_code {
	WLAN_LOG_REASON_CODE_UNUSED,
	WLAN_LOG_REASON_ROAM_FAIL,
	WLAN_LOG_REASON_THREAD_STUCK,
	WLAN_LOG_REASON_DATA_STALL,
	WLAN_LOG_REASON_SME_COMMAND_STUCK,
	WLAN_LOG_REASON_QUEUE_FULL,
	WLAN_LOG_REASON_POWER_COLLAPSE_FAIL,
	WLAN_LOG_REASON_MALLOC_FAIL,
	WLAN_LOG_REASON_VOS_MSG_UNDER_RUN,
	WLAN_LOG_REASON_IOCTL,
	WLAN_LOG_REASON_CODE_FRAMEWORK,
	WLAN_LOG_REASON_DEL_BSS_STA_FAIL,
	WLAN_LOG_REASON_ADD_BSS_STA_FAIL,
	WLAN_LOG_REASON_ENTER_IMPS_BMPS_FAIL,
	WLAN_LOG_REASON_EXIT_IMPS_BMPS_FAIL,
	WLAN_LOG_REASON_HDD_TIME_OUT,
	WLAN_LOG_REASON_MGMT_FRAME_TIMEOUT,
	WLAN_LOG_REASON_SME_OUT_OF_CMD_BUF,
	WLAN_LOG_REASON_SCAN_NOT_ALLOWED,
};

/**
 * vos_wdi_trace_event_type: Trace type for WDI Write/Read
 * VOS_WDI_READ: Log the WDI read event
 * VOS_WDI_WRITE: Log the WDI write event
 */
typedef enum
{
   VOS_WDI_READ,
   VOS_WDI_WRITE,
} vos_wdi_trace_event_type;

/*------------------------------------------------------------------------- 
  Function declarations and documenation
  ------------------------------------------------------------------------*/

/**--------------------------------------------------------------------------
  
  \brief vos_preOpen() - PreOpen the vOSS Module  
    
  The \a vos_preOpen() function allocates the Vos Context, but do not      
  initialize all the members. This overal initialization will happen
  at vos_Open().
  The reason why we need vos_preOpen() is to get a minimum context 
  where to store BAL and SAL relative data, which happens before
  vos_Open() is called.
  
  \param  pVosContext: A pointer to where to store the VOS Context 
 
  
  \return VOS_STATUS_SUCCESS - Scheduler was successfully initialized and 
          is ready to be used.
              
          VOS_STATUS_E_FAILURE - Failure to initialize the scheduler/   
          
  \sa vos_open()
  
---------------------------------------------------------------------------*/
VOS_STATUS vos_preOpen ( v_CONTEXT_t *pVosContext );

VOS_STATUS vos_preClose( v_CONTEXT_t *pVosContext );


VOS_STATUS vos_preStart( v_CONTEXT_t vosContext );


VOS_STATUS vos_open( v_CONTEXT_t *pVosContext, void *devHandle );

VOS_STATUS vos_mon_start( v_CONTEXT_t vosContext );

VOS_STATUS vos_mon_stop( v_CONTEXT_t vosContext );

VOS_STATUS vos_start( v_CONTEXT_t vosContext ); 

VOS_STATUS vos_stop( v_CONTEXT_t vosContext );

VOS_STATUS vos_close( v_CONTEXT_t vosContext );

/* vos shutdown will not close control transport and will not handshake with Riva */
VOS_STATUS vos_shutdown( v_CONTEXT_t vosContext );

/* the wda interface to shutdown */
VOS_STATUS vos_wda_shutdown( v_CONTEXT_t vosContext );

/**---------------------------------------------------------------------------
  
  \brief vos_get_context() - get context data area
  
  Each module in the system has a context / data area that is allocated
  and maanged by voss.  This API allows any user to get a pointer to its 
  allocated context data area from the VOSS global context.  

  \param vosContext - the VOSS Global Context.  
  
  \param moduleId - the module ID, who's context data are is being retrived.
                      
  \return - pointer to the context data area.
  
          - NULL if the context data is not allocated for the module ID
            specified 
              
  --------------------------------------------------------------------------*/
v_VOID_t *vos_get_context( VOS_MODULE_ID moduleId, 
                           v_CONTEXT_t vosContext );


/**---------------------------------------------------------------------------
  
  \brief vos_get_global_context() - get VOSS global Context
  
  This API allows any user to get the VOS Global Context pointer from a
  module context data area.  
  
  \param moduleContext - the input module context pointer
  
  \param moduleId - the module ID who's context pointer is input in 
         moduleContext.
                      
  \return - pointer to the VOSS global context
  
          - NULL if the function is unable to retreive the VOSS context. 
              
  --------------------------------------------------------------------------*/
v_CONTEXT_t vos_get_global_context( VOS_MODULE_ID moduleId, 
                                    v_VOID_t *moduleContext );

v_U8_t vos_is_logp_in_progress(VOS_MODULE_ID moduleId, v_VOID_t *moduleContext);
void vos_set_logp_in_progress(VOS_MODULE_ID moduleId, v_U8_t value);

v_U8_t vos_is_load_unload_in_progress(VOS_MODULE_ID moduleId, v_VOID_t *moduleContext);
void vos_set_load_unload_in_progress(VOS_MODULE_ID moduleId, v_U8_t value);

v_U8_t vos_is_reinit_in_progress(VOS_MODULE_ID moduleId, v_VOID_t *moduleContext);
void vos_set_reinit_in_progress(VOS_MODULE_ID moduleId, v_U8_t value);
VOS_STATUS vos_logger_pkt_serialize(vos_pkt_t *pPacket, uint32 pkt_type);
bool vos_is_log_report_in_progress(void);
void vos_reset_log_report_in_progress(void);
int vos_set_log_completion(uint32 is_fatal, uint32 indicator, uint32 reason_code);
void vos_get_log_and_reset_completion(uint32 *is_fatal,
           uint32 *indicator, uint32 *reason_code, bool reset);
v_BOOL_t vos_isFatalEventEnabled(void);
VOS_STATUS vos_fatal_event_logs_req( uint32_t is_fatal, uint32_t indicator,
                                 uint32_t reason_code, bool wait_required,
                                 bool dump_vos_trace);
VOS_STATUS vos_process_done_indication(v_U8_t type, v_U32_t reason_code);
void vos_flush_host_logs_for_fatal(void);

void vos_send_fatal_event_done(void);


/**---------------------------------------------------------------------------
  
  \brief vos_alloc_context() - allocate a context within the VOSS global Context
  
  This API allows any user to allocate a user context area within the 
  VOS Global Context.  
  
  \param pVosContext - pointer to the global Vos context
  
  \param moduleId - the module ID who's context area is being allocated.
  
  \param ppModuleContext - pointer to location where the pointer to the 
                           allocated context is returned.  Note this 
                           output pointer is valid only if the API
                           returns VOS_STATUS_SUCCESS
  
  \param size - the size of the context area to be allocated.
                      
  \return - VOS_STATUS_SUCCESS - the context for the module ID has been 
            allocated successfully.  The pointer to the context area
            can be found in *ppModuleContext.  
            \note This function returns VOS_STATUS_SUCCESS if the 
            module context was already allocated and the size 
            allocated matches the size on this call.

            VOS_STATUS_E_INVAL - the moduleId is not a valid or does 
            not identify a module that can have a context allocated.

            VOS_STATUS_E_EXISTS - vos could allocate the requested context 
            because a context for this module ID already exists and it is
            a *different* size that specified on this call.
            
            VOS_STATUS_E_NOMEM - vos could not allocate memory for the 
            requested context area.  
              
  \sa vos_get_context(), vos_free_context()
  
  --------------------------------------------------------------------------*/
VOS_STATUS vos_alloc_context( v_VOID_t *pVosContext, VOS_MODULE_ID moduleID, 
                              v_VOID_t **ppModuleContext, v_SIZE_t size );


/**---------------------------------------------------------------------------
  
  \brief vos_free_context() - free an allocated a context within the 
                               VOSS global Context
  
  This API allows a user to free the user context area within the 
  VOS Global Context.  
  
  \param pVosContext - pointer to the global Vos context
  
  \param moduleId - the module ID who's context area is being free
  
  \param pModuleContext - pointer to module context area to be free'd.
                      
  \return - VOS_STATUS_SUCCESS - the context for the module ID has been 
            free'd.  The pointer to the context area is not longer 
            available.
            
            VOS_STATUS_E_FAULT - pVosContext or pModuleContext are not 
            valid pointers.
                                 
            VOS_STATUS_E_INVAL - the moduleId is not a valid or does 
            not identify a module that can have a context free'd.
            
            VOS_STATUS_E_EXISTS - vos could not free the requested 
            context area because a context for this module ID does not
            exist in the global vos context.
              
  \sa vos_get_context()              
  
  --------------------------------------------------------------------------*/
VOS_STATUS vos_free_context( v_VOID_t *pVosContext, VOS_MODULE_ID moduleID,
                             v_VOID_t *pModuleContext );
v_BOOL_t vos_is_apps_power_collapse_allowed(void* pHddCtx);

/**
  @brief vos_wlanShutdown() - This API will shutdown WLAN driver

  This function is called when Riva subsystem crashes.  There are two
  methods (or operations) in WLAN driver to handle Riva crash,
    1. shutdown: Called when Riva goes down, this will shutdown WLAN
                 driver without handshaking with Riva.
    2. re-init:  Next API

  @param
       NONE
  @return
       VOS_STATUS_SUCCESS   - Operation completed successfully.
       VOS_STATUS_E_FAILURE - Operation failed.

*/
VOS_STATUS vos_wlanShutdown(void);

/**
  @brief vos_wlanReInit() - This API will re-init WLAN driver

  This function is called when Riva subsystem reboots.  There are two
  methods (or operations) in WLAN driver to handle Riva crash,
    1. shutdown: Previous API
    2. re-init:  Called when Riva comes back after the crash. This will
                 re-initialize WLAN driver. In some cases re-open may be
                 referred instead of re-init.
  @param
       NONE
  @return
       VOS_STATUS_SUCCESS   - Operation completed successfully.
       VOS_STATUS_E_FAILURE - Operation failed.

*/
VOS_STATUS vos_wlanReInit(void);

/**
  @brief vos_wlanRestart() - This API will reload WLAN driver.

  This function is called if driver detects any fatal state which 
  can be recovered by a WLAN module reload ( Android framwork initiated ).
  Note that this API will not initiate any RIVA subsystem restart.

  @param
       NONE
  @return
       VOS_STATUS_SUCCESS   - Operation completed successfully.
       VOS_STATUS_E_FAILURE - Operation failed.

*/
VOS_STATUS vos_wlanRestart(void);

/**
  @brief vos_fwDumpReq()

  This function is called to issue dump commands to Firmware

  @param
       cmd     -  Command No. to execute
       arg1    -  argument 1 to cmd
       arg2    -  argument 2 to cmd
       arg3    -  argument 3 to cmd
       arg4    -  argument 4 to cmd
       async   -  asynchronous event. Don't wait for completion.
  @return
       NONE
*/
v_VOID_t vos_fwDumpReq(tANI_U32 cmd, tANI_U32 arg1, tANI_U32 arg2,
                        tANI_U32 arg3, tANI_U32 arg4, tANI_U8 async);

v_VOID_t vos_flush_work(struct work_struct *work);
v_VOID_t vos_flush_delayed_work(struct delayed_work *dwork);

v_VOID_t vos_init_work(struct work_struct *work , void *callbackptr);
v_VOID_t vos_init_delayed_work(struct delayed_work *dwork , void *callbackptr);

v_U64_t vos_get_monotonic_boottime(void);

VOS_STATUS vos_randomize_n_bytes(void *mac_addr, tANI_U32 n);

v_BOOL_t vos_is_wlan_in_badState(VOS_MODULE_ID moduleId,
                                 v_VOID_t *moduleContext);
v_VOID_t  vos_set_roam_delay_stats_enabled(v_U8_t value);
v_U8_t    vos_get_roam_delay_stats_enabled(v_VOID_t);
v_U32_t   vos_get_dxeReplenishRXTimerVal(void);
v_BOOL_t  vos_get_dxeSSREnable(void);

v_U8_t vos_is_fw_logging_enabled(void);
v_U8_t vos_is_fw_ev_logging_enabled(void);

v_U8_t vos_is_fw_logging_supported(void);
void vos_set_multicast_logging(uint8_t value);
v_U8_t vos_is_multicast_logging(void);
void vos_set_ring_log_level(v_U32_t ring_id, v_U32_t log_level);
v_U8_t vos_get_ring_log_level(v_U32_t ring_id);
void get_rate_and_MCS(per_packet_stats *stats, uint32 rateindex);

v_BOOL_t vos_isUnloadInProgress(void);
v_BOOL_t vos_isLoadUnloadInProgress(void);

bool vos_get_rx_wow_dump(void);
void vos_set_rx_wow_dump(bool value);

void vos_probe_threads(void);
void vos_per_pkt_stats_to_user(void *perPktStat);
void vos_updatePktStatsInfo(void * pktStat);
bool vos_is_wlan_logging_enabled(void);

v_BOOL_t vos_is_probe_rsp_offload_enabled(void);
void vos_smd_dump_stats(void);
void vos_log_wdi_event(uint16 msg, vos_wdi_trace_event_type event);
void vos_dump_wdi_events(void);
#endif // if !defined __VOS_NVITEM_H
