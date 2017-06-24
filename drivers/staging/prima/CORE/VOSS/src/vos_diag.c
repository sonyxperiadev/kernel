/*
 * Copyright (c) 2014-2016 The Linux Foundation. All rights reserved.
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

/*============================================================================
  FILE:         vos_diag.c

  OVERVIEW:     This source file contains definitions for vOS diag APIs

  DEPENDENCIES:
============================================================================*/

#include "vos_types.h"
#include "i_vos_diag_core_log.h"
#include "i_vos_diag_core_event.h"
#include "wlan_hdd_main.h"
#include "wlan_nlink_common.h"
#include "vos_sched.h"
#include "wlan_ptt_sock_svc.h"
#include "wlan_nlink_srv.h"


#define PTT_MSG_DIAG_CMDS_TYPE   0x5050

#define DIAG_TYPE_LOGS   1 
#define DIAG_TYPE_EVENTS 2

#define DIAG_SWAP16(A) ((((tANI_U16)(A) & 0xff00) >> 8) | (((tANI_U16)(A) & 0x00ff) << 8))



typedef struct event_report_s
{
    v_U32_t diag_type;
    v_U16_t event_id;
    v_U16_t length;
} event_report_t;


/**---------------------------------------------------------------------------
  
  \brief vos_log_set_code() - 

   This function sets the logging code in the given log record. 

  \param  - ptr - Pointer to the log header type.
              - code - log code.
  \return - None
  
  --------------------------------------------------------------------------*/

void vos_log_set_code (v_VOID_t *ptr, v_U16_t code)
{
    if (ptr)
    {
        /* All log packets are required to start with 'log_header_type'. */
        ((log_hdr_type *) ptr)->code = code;
    }

} 

/**---------------------------------------------------------------------------
  
  \brief vos_log_set_length() - 

   This function sets the length field in the given log record.

  \param  - ptr - Pointer to the log header type.
              - length - log length.
              
  \return - None
  
  --------------------------------------------------------------------------*/

void vos_log_set_length (v_VOID_t *ptr, v_U16_t length)
{
    if(ptr)
    {
        /* All log packets are required to start with 'log_header_type'. */
        ((log_hdr_type *) ptr)->len = (v_U16_t) length;
    }
} 

/**---------------------------------------------------------------------------
  
  \brief vos_log_submit() - 

   This function sends the log data to the ptt socket app only if it is registered with the driver.

  \param  - ptr - Pointer to the log header type.
              
  \return - None
  
  --------------------------------------------------------------------------*/

void vos_log_submit(v_VOID_t *plog_hdr_ptr)
{

    log_hdr_type *pHdr = (log_hdr_type*) plog_hdr_ptr;

    tAniHdr *wmsg = NULL;
    v_U8_t *pBuf;
    struct hdd_context_s *pHddCtx;
    v_CONTEXT_t pVosContext= NULL;
    v_U16_t data_len;
    v_U16_t total_len;


     /*Get the global context */
    pVosContext = vos_get_global_context(VOS_MODULE_ID_SYS, NULL);

     /*Get the Hdd Context */
    pHddCtx = ((VosContextType*)(pVosContext))->pHDDContext;

    if (WLAN_HDD_IS_LOAD_UNLOAD_IN_PROGRESS(pHddCtx))
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
                  "%s: Unloading/Loading in Progress. Ignore!!!", __func__);
        return;
    }

    if (nl_srv_is_initialized() != 0)
        return;

#ifdef WLAN_KD_READY_NOTIFIER
    /* NL is not ready yet, WLAN KO started first */
    if ((pHddCtx->kd_nl_init) && (!pHddCtx->ptt_pid))
    {
        nl_srv_nl_ready_indication();
    }
#endif /* WLAN_KD_READY_NOTIFIER */

   /* Send the log data to the ptt app only if it is registered with the wlan driver*/
    if(vos_is_multicast_logging())
    {
        data_len = pHdr->len;
    
        total_len = sizeof(tAniHdr)+sizeof(v_U32_t)+data_len;
    
        pBuf =  (v_U8_t*)vos_mem_malloc(total_len);
    
        if(!pBuf)
        {
            VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR, "vos_mem_malloc failed");
            return;
        }
        
        vos_mem_zero((v_VOID_t*)pBuf,total_len);
    
        wmsg = (tAniHdr*)pBuf;
        wmsg->type = PTT_MSG_DIAG_CMDS_TYPE;
        wmsg->length = total_len;
        wmsg->length = DIAG_SWAP16(wmsg->length);
        pBuf += sizeof(tAniHdr);
    
    
            /*  Diag Type events or log */
        *(v_U32_t*)pBuf = DIAG_TYPE_LOGS;
        pBuf += sizeof(v_U32_t);
    
    
        vos_mem_copy(pBuf, pHdr,data_len);

        if (ptt_sock_send_msg_to_app(wmsg, 0,
                  ANI_NL_MSG_PUMAC, INVALID_PID, MSG_DONTWAIT) < 0)
        {
            vos_mem_free((v_VOID_t *)wmsg);
            return;
        }

        vos_mem_free((v_VOID_t*)wmsg);
    }
    return;
}

/**
 * vos_log_wlock_diag() - This function is used to send wake lock diag events
 * @reason: Reason why the wakelock was taken or released
 * @wake_lock_name: Function in which the wakelock was taken or released
 * @timeout: Timeout value in case of timed wakelocks
 * @status: Status field indicating whether the wake lock was taken/released
 *
 * This function is used to send wake lock diag events to user space
 *
 * Return: None
 *
 */
void vos_log_wlock_diag(uint32_t reason, const char *wake_lock_name,
                              uint32_t timeout, uint32_t status)
{
     VosContextType *vos_context;
     WLAN_VOS_DIAG_EVENT_DEF(wlan_diag_event,
     struct vos_event_wlan_wake_lock);

    vos_context = vos_get_global_context(VOS_MODULE_ID_SYS, NULL);
    if (!vos_context) {
      VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
            "vos context is Invald");
       return;
    }
     if (nl_srv_is_initialized() != 0 ||
         vos_context->wakelock_log_level == WLAN_LOG_LEVEL_OFF)
          return;

     wlan_diag_event.status = status;
     wlan_diag_event.reason = reason;
     wlan_diag_event.timeout = timeout;
     wlan_diag_event.name_len = strlen(wake_lock_name);
     strlcpy(&wlan_diag_event.name[0],
             wake_lock_name,
             wlan_diag_event.name_len+1);

     WLAN_VOS_DIAG_EVENT_REPORT(&wlan_diag_event, EVENT_WLAN_WAKE_LOCK);
}


/**---------------------------------------------------------------------------
  
  \brief vos_event_report_payload() - 

   This function sends the event data to the ptt socket app only if it is registered with the driver.

  \param  - ptr - Pointer to the log header type.
              
  \return - None
  
  --------------------------------------------------------------------------*/

void vos_event_report_payload(v_U16_t event_Id, v_U16_t length, v_VOID_t *pPayload)
{ 


    tAniHdr *wmsg = NULL;
    v_U8_t *pBuf;
    struct hdd_context_s *pHddCtx;
    v_CONTEXT_t pVosContext= NULL;
    event_report_t *pEvent_report;
    v_U16_t total_len;

     /*Get the global context */
    pVosContext = vos_get_global_context(VOS_MODULE_ID_SYS, NULL);
    if (!pVosContext)
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                  "%s: vos context is NULL", __func__);
        return;
    }

     /*Get the Hdd Context */
    pHddCtx = ((VosContextType*)(pVosContext))->pHDDContext;
    if (!pHddCtx)
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                  "%s: hdd context is NULL", __func__);
        return;
    }

    if (nl_srv_is_initialized() != 0)
        return;

#ifdef WLAN_KD_READY_NOTIFIER
    /* NL is not ready yet, WLAN KO started first */
    if ((pHddCtx->kd_nl_init) && (!pHddCtx->ptt_pid))
    {
        nl_srv_nl_ready_indication();
    }
#endif /* WLAN_KD_READY_NOTIFIER */
    
    /* Send the log data to the ptt app only if it is registered with the wlan driver*/
    if(vos_is_multicast_logging())
    {
        total_len = sizeof(tAniHdr)+sizeof(event_report_t)+length;
        
        pBuf =  (v_U8_t*)vos_mem_malloc(total_len);
    
        if(!pBuf)
        {
            VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR, "vos_mem_malloc failed");
            return;
        }
        wmsg = (tAniHdr*)pBuf;
        wmsg->type = PTT_MSG_DIAG_CMDS_TYPE;
        wmsg->length = total_len;
        wmsg->length = DIAG_SWAP16(wmsg->length);
        pBuf += sizeof(tAniHdr);
    
        pEvent_report = (event_report_t*)pBuf;
        pEvent_report->diag_type = DIAG_TYPE_EVENTS;
        pEvent_report->event_id = event_Id;
        pEvent_report->length = length;
    
        pBuf += sizeof(event_report_t); 
    
        vos_mem_copy(pBuf, pPayload,length);
      
        if( ptt_sock_send_msg_to_app(wmsg, 0,
                     ANI_NL_MSG_PUMAC, INVALID_PID, MSG_DONTWAIT) < 0) {
            VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                       ("Ptt Socket error sending message to the app!!"));
            vos_mem_free((v_VOID_t*)wmsg);
            return;
        }
    
        vos_mem_free((v_VOID_t*)wmsg);
    }
  
    return;
    
}
