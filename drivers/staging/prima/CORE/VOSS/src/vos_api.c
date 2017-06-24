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

/**=========================================================================

  \file  vos_api.c

  \brief Stub file for all virtual Operating System Services (vOSS) APIs
  
  ========================================================================*/
 /*=========================================================================== 

                       EDIT HISTORY FOR FILE 
   
   
  This section contains comments describing changes made to the module. 
  Notice that changes are listed in reverse chronological order. 
   
   
  $Header:$ $DateTime: $ $Author: $ 
   
   
  when        who    what, where, why 
  --------    ---    --------------------------------------------------------
  03/29/09    kanand     Created module. 
===========================================================================*/

/*--------------------------------------------------------------------------
  Include Files
  ------------------------------------------------------------------------*/
#include <vos_mq.h>
#include "vos_sched.h"
#include <vos_api.h>
#include "sirTypes.h"
#include "sirApi.h"
#include "sirMacProtDef.h"
#include "sme_Api.h"
#include "macInitApi.h"
#include "wlan_qct_sys.h"
#include "wlan_qct_tl.h"
#include "wlan_hdd_misc.h"
#include "i_vos_packet.h"
#include "vos_nvitem.h"
#include "wlan_qct_wda.h"
#include "wlan_hdd_main.h"
#include <linux/vmalloc.h>
#include "wlan_hdd_cfg80211.h"
#include "vos_diag_core_log.h"

#include <linux/wcnss_wlan.h>

#include "sapApi.h"
#include "vos_trace.h"
#include "vos_utils.h"
#include <wlan_logging_sock_svc.h>

#ifdef WLAN_BTAMP_FEATURE
#include "bapApi.h"
#include "bapInternal.h"
#include "bap_hdd_main.h"
#endif //WLAN_BTAMP_FEATURE
#include "wlan_qct_wdi_cts.h"

/*---------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 * ------------------------------------------------------------------------*/
/* Amount of time to wait for WDA to perform an asynchronous activity.
   This value should be larger than the timeout used by WDI to wait for
   a response from WCNSS since in the event that WCNSS is not responding,
   WDI should handle that timeout */
#define VOS_WDA_TIMEOUT 15000

/* Approximate amount of time to wait for WDA to stop WDI */
#define VOS_WDA_STOP_TIMEOUT WDA_STOP_TIMEOUT 

/* Approximate amount of time to wait for WDA to issue a DUMP req */
#define VOS_WDA_RESP_TIMEOUT WDA_STOP_TIMEOUT

/* Trace index for WDI Read/Write */
#define VOS_TRACE_INDEX_MAX 256

/*---------------------------------------------------------------------------
 * Data definitions
 * ------------------------------------------------------------------------*/
static VosContextType  gVosContext;
static pVosContextType gpVosContext;
static v_U8_t vos_multicast_logging;

struct vos_wdi_trace
{
   vos_wdi_trace_event_type event;
   uint16        message;
   uint64        time;
};

static struct vos_wdi_trace gvos_wdi_msg_trace[VOS_TRACE_INDEX_MAX];
uint16 gvos_wdi_msg_trace_index = 0;

/*---------------------------------------------------------------------------
 * Forward declaration
 * ------------------------------------------------------------------------*/
v_VOID_t vos_sys_probe_thread_cback ( v_VOID_t *pUserData );

v_VOID_t vos_core_return_msg(v_PVOID_t pVContext, pVosMsgWrapper pMsgWrapper);

v_VOID_t vos_fetch_tl_cfg_parms ( WLANTL_ConfigInfoType *pTLConfig, 
    hdd_config_t * pConfig );


/*---------------------------------------------------------------------------
  
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
          
  \sa vos_Open()
  
---------------------------------------------------------------------------*/
VOS_STATUS vos_preOpen ( v_CONTEXT_t *pVosContext )
{
   if ( pVosContext == NULL)
      return VOS_STATUS_E_FAILURE;

   /* Allocate the VOS Context */
   *pVosContext = NULL;
   gpVosContext = &gVosContext;

   if (NULL == gpVosContext)
   {
     /* Critical Error ...Cannot proceed further */
      VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                 "%s: Failed to allocate VOS Context", __func__);
      VOS_ASSERT(0);
      return VOS_STATUS_E_RESOURCES;
   }

   vos_mem_zero(gpVosContext, sizeof(VosContextType));

   *pVosContext = gpVosContext;

   /* Initialize the spinlock */
   vos_trace_spin_lock_init();
   /* it is the right time to initialize MTRACE structures */
   #if defined(TRACE_RECORD)
       vosTraceInit();
   #endif
   vos_register_debugcb_init();

   return VOS_STATUS_SUCCESS;

} /* vos_preOpen()*/

  
/*---------------------------------------------------------------------------
  
  \brief vos_preClose() - PreClose the vOSS Module  
    
  The \a vos_preClose() function frees the Vos Context.
  
  \param  pVosContext: A pointer to where the VOS Context was stored 
 
  
  \return VOS_STATUS_SUCCESS - Always successful
                  
          
  \sa vos_preClose()
  \sa vos_close()
---------------------------------------------------------------------------*/
VOS_STATUS vos_preClose( v_CONTEXT_t *pVosContext )
{

   VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_INFO,
                "%s: De-allocating the VOS Context", __func__);

   if (( pVosContext == NULL) || (*pVosContext == NULL)) 
   {
      VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                "%s: vOS Context is Null", __func__);
      return VOS_STATUS_E_FAILURE;
   }

   if (gpVosContext != *pVosContext)
   {
      VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                "%s: Context mismatch", __func__);
      return VOS_STATUS_E_FAILURE;
   }

   *pVosContext = gpVosContext = NULL;

   return VOS_STATUS_SUCCESS;

} /* vos_preClose()*/

/*---------------------------------------------------------------------------
  
  \brief vos_open() - Open the vOSS Module  
    
  The \a vos_open() function opens the vOSS Scheduler
  Upon successful initialization:
  
     - All VOS submodules should have been initialized
     
     - The VOS scheduler should have opened
     
     - All the WLAN SW components should have been opened. This includes
       SYS, MAC, SME, WDA and TL.
      
  
  \param  devHandle: pointer to the OS specific device handle
 
  
  \return VOS_STATUS_SUCCESS - Scheduler was successfully initialized and 
          is ready to be used.
  
          VOS_STATUS_E_RESOURCES - System resources (other than memory) 
          are unavailable to initilize the scheduler

          
          VOS_STATUS_E_FAILURE - Failure to initialize the scheduler/   
          
  \sa vos_preOpen()
  
---------------------------------------------------------------------------*/
VOS_STATUS vos_open( v_CONTEXT_t *pVosContext, void *devHandle )

{
   VOS_STATUS vStatus      = VOS_STATUS_SUCCESS;
   int iter                = 0;
   tSirRetStatus sirStatus = eSIR_SUCCESS;
   tMacOpenParameters macOpenParms;
   WLANTL_ConfigInfoType TLConfig;

   VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_INFO_HIGH,
               "%s: Opening VOSS", __func__);

   if (NULL == gpVosContext)
   {
      VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                    "%s: Trying to open VOSS without a PreOpen", __func__);
      VOS_ASSERT(0);
      return VOS_STATUS_E_FAILURE;
   }

   /* Initialize the timer module */
   vos_timer_module_init();

   /* Initialize the probe event */
   if (vos_event_init(&gpVosContext->ProbeEvent) != VOS_STATUS_SUCCESS)
   {
      VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                    "%s: Unable to init probeEvent", __func__);
      VOS_ASSERT(0);
      return VOS_STATUS_E_FAILURE;
   }
   if (vos_event_init( &(gpVosContext->wdaCompleteEvent) ) != VOS_STATUS_SUCCESS )
   {
      VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                  "%s: Unable to init wdaCompleteEvent", __func__);
      VOS_ASSERT(0);
    
      goto err_probe_event;
   }
   if (vos_event_init( &(gpVosContext->fwLogsComplete) ) != VOS_STATUS_SUCCESS )
   {
      VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                  "%s: Unable to init fwLogsComplete", __func__);
      VOS_ASSERT(0);

      goto err_wda_complete_event;
   }

   /* Initialize the free message queue */
   vStatus = vos_mq_init(&gpVosContext->freeVosMq);
   if (! VOS_IS_STATUS_SUCCESS(vStatus))
   {

      /* Critical Error ...  Cannot proceed further */
      VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "%s: Failed to initialize VOS free message queue", __func__);
      VOS_ASSERT(0);
      goto err_fw_logs_complete_event;
   }

   for (iter = 0; iter < VOS_CORE_MAX_MESSAGES; iter++)
   {
      (gpVosContext->aMsgWrappers[iter]).pVosMsg = 
         &(gpVosContext->aMsgBuffers[iter]); 
      INIT_LIST_HEAD(&gpVosContext->aMsgWrappers[iter].msgNode);
      vos_mq_put(&gpVosContext->freeVosMq, &(gpVosContext->aMsgWrappers[iter]));
   }

   /* Now Open the VOS Scheduler */
   vStatus= vos_sched_open(gpVosContext, &gpVosContext->vosSched,
                           sizeof(VosSchedContext));

   if (!VOS_IS_STATUS_SUCCESS(vStatus))
   {
      /* Critical Error ...  Cannot proceed further */
      VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "%s: Failed to open VOS Scheduler", __func__);
      VOS_ASSERT(0);
      goto err_msg_queue;
   }

   /*
   ** Need to open WDA first because it calls WDI_Init, which calls wpalOpen
   ** The reason that is needed becasue vos_packet_open need to use PAL APIs
   */

   /*Open the WDA module */
   vos_mem_set(&macOpenParms, sizeof(macOpenParms), 0);
   /* UMA is supported in hardware for performing the
   ** frame translation 802.11 <-> 802.3
   */
   macOpenParms.frameTransRequired = 1;
   macOpenParms.driverType         = eDRIVER_TYPE_PRODUCTION;
   vStatus = WDA_open( gpVosContext, devHandle, &macOpenParms );

   if (!VOS_IS_STATUS_SUCCESS(vStatus))
   {
      /* Critical Error ...  Cannot proceed further */
      VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "%s: Failed to open WDA module", __func__);
      VOS_ASSERT(0);
      goto err_sched_close;
   }

   /* Initialize here the VOS Packet sub module */
   vStatus = vos_packet_open( gpVosContext, &gpVosContext->vosPacket,
                              sizeof( vos_pkt_context_t ) );

   if ( !VOS_IS_STATUS_SUCCESS( vStatus ) )
   {
      /* Critical Error ...  Cannot proceed further */
      VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "%s: Failed to open VOS Packet Module", __func__);
      VOS_ASSERT(0);
      goto err_wda_close;
   }

   /* Open the SYS module */
   vStatus = sysOpen(gpVosContext);

   if (!VOS_IS_STATUS_SUCCESS(vStatus))
   {
      /* Critical Error ...  Cannot proceed further */
      VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "%s: Failed to open SYS module", __func__);
      VOS_ASSERT(0);
      goto err_packet_close;
   }

#ifndef CONFIG_ENABLE_LINUX_REG
   /* initialize the NV module */
   vStatus = vos_nv_open();
   if (!VOS_IS_STATUS_SUCCESS(vStatus))
   {
     // NV module cannot be initialized
     VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "%s: Failed to initialize the NV module", __func__);
     goto err_sys_close;
   }
#endif

   /* If we arrive here, both threads dispacthing messages correctly */
   
   /* Now proceed to open the MAC */

   /* UMA is supported in hardware for performing the
      frame translation 802.11 <-> 802.3 */
   macOpenParms.frameTransRequired = 1;
   sirStatus = macOpen(&(gpVosContext->pMACContext), gpVosContext->pHDDContext,
                         &macOpenParms);
   
   if (eSIR_SUCCESS != sirStatus)
   {
     /* Critical Error ...  Cannot proceed further */
     VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
               "%s: Failed to open MAC", __func__);
     VOS_ASSERT(0);
     goto err_nv_close;
   }

   /* Now proceed to open the SME */
   vStatus = sme_Open(gpVosContext->pMACContext);
   if (!VOS_IS_STATUS_SUCCESS(vStatus))
   {
     /* Critical Error ...  Cannot proceed further */
     VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
               "%s: Failed to open SME", __func__);
     VOS_ASSERT(0);
     goto err_mac_close;
   }

   /* Now proceed to open TL. Read TL config first */
   vos_fetch_tl_cfg_parms ( &TLConfig, 
       ((hdd_context_t*)(gpVosContext->pHDDContext))->cfg_ini);

   vStatus = WLANTL_Open(gpVosContext, &TLConfig);
   if (!VOS_IS_STATUS_SUCCESS(vStatus))
   {
     /* Critical Error ...  Cannot proceed further */
     VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
               "%s: Failed to open TL", __func__);
     VOS_ASSERT(0);
     goto err_sme_close;
   }

   if (gpVosContext->roamDelayStatsEnabled &&
       !vos_roam_delay_stats_init())
   {
       VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                 "%s: Could not init roamDelayStats", __func__);
   }

   VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_INFO_HIGH,
               "%s: VOSS successfully Opened", __func__);

   *pVosContext = gpVosContext;

   return VOS_STATUS_SUCCESS;


err_sme_close:
   sme_Close(gpVosContext->pMACContext);

err_mac_close:
   macClose(gpVosContext->pMACContext);

err_nv_close:

#ifndef CONFIG_ENABLE_LINUX_REG
   vos_nv_close();

err_sys_close:
#endif

   sysClose(gpVosContext);

err_packet_close:
   vos_packet_close( gpVosContext );

err_wda_close:
   WDA_close(gpVosContext);

err_sched_close:
   vos_sched_close(gpVosContext);


err_msg_queue:
   vos_mq_deinit(&gpVosContext->freeVosMq);

err_fw_logs_complete_event:
    vos_event_destroy( &gpVosContext->fwLogsComplete);

err_wda_complete_event:
   vos_event_destroy( &gpVosContext->wdaCompleteEvent );

err_probe_event:
   vos_event_destroy(&gpVosContext->ProbeEvent);

   return VOS_STATUS_E_FAILURE;

} /* vos_open() */

/*---------------------------------------------------------------------------

  \brief vos_preStart() -

  The \a vos_preStart() function to download CFG.
  including:
      - ccmStart

      - WDA: triggers the CFG download


  \param  pVosContext: The VOS context


  \return VOS_STATUS_SUCCESS - Scheduler was successfully initialized and
          is ready to be used.

          VOS_STATUS_E_RESOURCES - System resources (other than memory)
          are unavailable to initilize the scheduler


          VOS_STATUS_E_FAILURE - Failure to initialize the scheduler/

  \sa vos_start

---------------------------------------------------------------------------*/
VOS_STATUS vos_preStart( v_CONTEXT_t vosContext )
{
   VOS_STATUS vStatus          = VOS_STATUS_SUCCESS;
   pVosContextType pVosContext = (pVosContextType)vosContext;
   
   VOS_TRACE(VOS_MODULE_ID_SYS, VOS_TRACE_LEVEL_INFO,
             "vos prestart");

   if (gpVosContext != pVosContext)
   {
      VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                "%s: Context mismatch", __func__);
      VOS_ASSERT(0);
      return VOS_STATUS_E_INVAL;
   }

   if (pVosContext->pMACContext == NULL)
   {
       VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
            "%s: MAC NULL context", __func__);
       VOS_ASSERT(0);
       return VOS_STATUS_E_INVAL;
   }

   if (pVosContext->pWDAContext == NULL)
   {
       VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
          "%s: WDA NULL context", __func__);
       VOS_ASSERT(0);
       return VOS_STATUS_E_INVAL;
   }

   /* call macPreStart */
   vStatus = macPreStart(gpVosContext->pMACContext);
   if ( !VOS_IS_STATUS_SUCCESS(vStatus) )
   {
      VOS_TRACE(VOS_MODULE_ID_SYS, VOS_TRACE_LEVEL_FATAL,
             "Failed at macPreStart ");
      return VOS_STATUS_E_FAILURE;
   }

   /* call ccmStart */
   ccmStart(gpVosContext->pMACContext);

   /* Reset wda wait event */
   vos_event_reset(&gpVosContext->wdaCompleteEvent);   
    

   /*call WDA pre start*/
   vStatus = WDA_preStart(gpVosContext);
   if (!VOS_IS_STATUS_SUCCESS(vStatus))
   {
      VOS_TRACE(VOS_MODULE_ID_SYS, VOS_TRACE_LEVEL_FATAL,
             "Failed to WDA prestart");
      macStop(gpVosContext->pMACContext, HAL_STOP_TYPE_SYS_DEEP_SLEEP);
      ccmStop(gpVosContext->pMACContext);
      VOS_ASSERT(0);
      return VOS_STATUS_E_FAILURE;
   }

   /* Need to update time out of complete */
   vStatus = vos_wait_single_event( &gpVosContext->wdaCompleteEvent,
                                    VOS_WDA_TIMEOUT );
   if ( vStatus != VOS_STATUS_SUCCESS )
   {
      if ( vStatus == VOS_STATUS_E_TIMEOUT )
      {
         VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
          "%s: Timeout occurred before WDA complete", __func__);
      }
      else
      {
         VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
           "%s: WDA_preStart reporting other error", __func__);
      }
      VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
           "%s: Test MC thread by posting a probe message to SYS", __func__);
      wlan_sys_probe();

      macStop(gpVosContext->pMACContext, HAL_STOP_TYPE_SYS_DEEP_SLEEP);
      ccmStop(gpVosContext->pMACContext);
      VOS_ASSERT( 0 );
      return VOS_STATUS_E_FAILURE;
   }

   return VOS_STATUS_SUCCESS;
}

VOS_STATUS vos_mon_start( v_CONTEXT_t vosContext )
{
  VOS_STATUS vStatus          = VOS_STATUS_SUCCESS;
  pVosContextType pVosContext = (pVosContextType)vosContext;

  if (pVosContext == NULL)
   {
       VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
          "%s: mismatch in context",__func__);
       return VOS_STATUS_E_FAILURE;
   }

  if (( pVosContext->pWDAContext == NULL) || ( pVosContext->pTLContext == NULL))
  {
     if (pVosContext->pWDAContext == NULL)
        VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
            "%s: WDA NULL context", __func__);
     else
        VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
            "%s: TL NULL context", __func__);

     return VOS_STATUS_E_FAILURE;
  }

   /* Reset wda wait event */
   vos_event_reset(&pVosContext->wdaCompleteEvent);

   /*call WDA pre start*/
   vStatus = WDA_preStart(pVosContext);
   if (!VOS_IS_STATUS_SUCCESS(vStatus))
   {
      VOS_TRACE(VOS_MODULE_ID_SYS, VOS_TRACE_LEVEL_ERROR,
             "Failed to WDA prestart ");
      VOS_ASSERT(0);
      return VOS_STATUS_E_FAILURE;
   }

   /* Need to update time out of complete */
   vStatus = vos_wait_single_event( &pVosContext->wdaCompleteEvent, 1000);
   if ( vStatus != VOS_STATUS_SUCCESS )
   {
      if ( vStatus == VOS_STATUS_E_TIMEOUT )
      {
         VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
          "%s: Timeout occurred before WDA complete",__func__);
      }
      else
      {
         VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
           "%s: WDA_preStart reporting  other error",__func__);
      }
      VOS_ASSERT( 0 );
      return VOS_STATUS_E_FAILURE;
   }

    vStatus = WDA_NVDownload_Start(pVosContext);

    if ( vStatus != VOS_STATUS_SUCCESS )
    {
       VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                   "%s: Failed to start NV Download",__func__);
       return VOS_STATUS_E_FAILURE;
    }

    vStatus = vos_wait_single_event(&(pVosContext->wdaCompleteEvent), 1000 * 30);

    if ( vStatus != VOS_STATUS_SUCCESS )
    {
       if ( vStatus == VOS_STATUS_E_TIMEOUT )
       {
          VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                     "%s: Timeout occurred before WDA_NVDownload_Start complete",__func__);
       }
       else
       {
         VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                    "%s: WDA_NVDownload_Start reporting  other error",__func__);
       }
       VOS_ASSERT(0);
       return VOS_STATUS_E_FAILURE;
    }

    vStatus = WDA_start(pVosContext);
    if (vStatus != VOS_STATUS_SUCCESS)
    {
       VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                 "%s: Failed to start WDA",__func__);
       return VOS_STATUS_E_FAILURE;
    }

  /** START TL */
  vStatus = WLANTL_Start(pVosContext);
  if (!VOS_IS_STATUS_SUCCESS(vStatus))
  {
    VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
               "%s: Failed to start TL", __func__);
    goto err_wda_stop;
  }

  return VOS_STATUS_SUCCESS;

err_wda_stop:
   vos_event_reset(&(pVosContext->wdaCompleteEvent));
   WDA_stop(pVosContext, HAL_STOP_TYPE_RF_KILL);
   vStatus = vos_wait_single_event(&(pVosContext->wdaCompleteEvent), 1000);
   if(vStatus != VOS_STATUS_SUCCESS)
   {
      if(vStatus == VOS_STATUS_E_TIMEOUT)
      {
         VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                   "%s: Timeout occurred before WDA_stop complete",__func__);

      }
      else
      {
        VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                  "%s: WDA_stop reporting  other error",__func__);
      }
      VOS_ASSERT(0);
   }
  return VOS_STATUS_E_FAILURE;
}

VOS_STATUS vos_mon_stop( v_CONTEXT_t vosContext )
{
  VOS_STATUS vosStatus;

  vos_event_reset( &(gpVosContext->wdaCompleteEvent) );

  VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
         "%s: HAL_STOP is requested", __func__);

  vosStatus = WDA_stop( vosContext, HAL_STOP_TYPE_RF_KILL );

  if (!VOS_IS_STATUS_SUCCESS(vosStatus))
  {
     VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
         "%s: Failed to stop WDA", __func__);
     VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
     WDA_setNeedShutdown(vosContext);
  }
  else
  {
    vosStatus = vos_wait_single_event( &(gpVosContext->wdaCompleteEvent),
                                       VOS_WDA_STOP_TIMEOUT );

    if ( vosStatus != VOS_STATUS_SUCCESS )
    {
       if ( vosStatus == VOS_STATUS_E_TIMEOUT )
       {
          VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
           "%s: Timeout occurred before WDA complete", __func__);
       }
       else
       {
          VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
           "%s: WDA_stop reporting other error", __func__ );
       }
       WDA_setNeedShutdown(vosContext);
    }
  }

  vosStatus = WLANTL_Stop( vosContext );
  if (!VOS_IS_STATUS_SUCCESS(vosStatus))
  {
     VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
         "%s: Failed to stop TL", __func__);
     VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
  }

  return VOS_STATUS_SUCCESS;
}

/*---------------------------------------------------------------------------
  
  \brief vos_start() - Start the Libra SW Modules 
    
  The \a vos_start() function starts all the components of the Libra SW
  including:
      - SAL/BAL, which in turn starts SSC
      
      - the MAC (HAL and PE)
      
      - SME
      
      - TL
      
      - SYS: triggers the CFG download
  
  
  \param  pVosContext: The VOS context
 
  
  \return VOS_STATUS_SUCCESS - Scheduler was successfully initialized and 
          is ready to be used.
  
          VOS_STATUS_E_RESOURCES - System resources (other than memory) 
          are unavailable to initilize the scheduler

          
          VOS_STATUS_E_FAILURE - Failure to initialize the scheduler/   
          
  \sa vos_preStart()
  \sa vos_open()
  
---------------------------------------------------------------------------*/
VOS_STATUS vos_start( v_CONTEXT_t vosContext )
{
  VOS_STATUS vStatus          = VOS_STATUS_SUCCESS;
  tSirRetStatus sirStatus     = eSIR_SUCCESS;
  pVosContextType pVosContext = (pVosContextType)vosContext;
  tHalMacStartParameters halStartParams;
  hdd_context_t *pHddCtx = NULL;

  VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_INFO,
            "%s: Starting Libra SW", __func__);

  /* We support only one instance for now ...*/
  if (gpVosContext != pVosContext)
  {
     VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
           "%s: mismatch in context", __func__);
     return VOS_STATUS_E_FAILURE;
  }

  if (( pVosContext->pWDAContext == NULL) || ( pVosContext->pMACContext == NULL)
     || ( pVosContext->pTLContext == NULL))
  {
     if (pVosContext->pWDAContext == NULL)
        VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, 
            "%s: WDA NULL context", __func__);
     else if (pVosContext->pMACContext == NULL)
        VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, 
            "%s: MAC NULL context", __func__);
     else
        VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, 
            "%s: TL NULL context", __func__);
     
     return VOS_STATUS_E_FAILURE;
  }

  /* WDA_Start will be called after NV image download because the 
    NV image data has to be updated at HAL before HAL_Start gets executed*/

  /* Start the NV Image Download */

  vos_event_reset( &(gpVosContext->wdaCompleteEvent) );

  vStatus = WDA_NVDownload_Start(pVosContext);

  if ( vStatus != VOS_STATUS_SUCCESS )
  {
     VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                 "%s: Failed to start NV Download", __func__);
     return VOS_STATUS_E_FAILURE;
  }

  vStatus = vos_wait_single_event( &(gpVosContext->wdaCompleteEvent),
                                   VOS_WDA_TIMEOUT );

  if ( vStatus != VOS_STATUS_SUCCESS )
  {
     if ( vStatus == VOS_STATUS_E_TIMEOUT )
     {
        VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
         "%s: Timeout occurred before WDA_NVDownload_start complete", __func__);
     }
     else
     {
        VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
         "%s: WDA_NVDownload_start reporting other error", __func__);
     }
     VOS_ASSERT(0);
     vos_event_reset( &(gpVosContext->wdaCompleteEvent) );
     if (vos_is_logp_in_progress(VOS_MODULE_ID_VOSS, NULL))
     {
       if (isSsrPanicOnFailure())
           VOS_BUG(0);
     }
     WDA_setNeedShutdown(vosContext);
     return VOS_STATUS_E_FAILURE;
  }

  VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_INFO,
            "%s: WDA_NVDownload_start correctly started", __func__);

  /* Start the WDA */
  vStatus = WDA_start(pVosContext);
  if ( vStatus != VOS_STATUS_SUCCESS )
  {
     VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                 "%s: Failed to start WDA - WDA_shutdown needed %d ",
                   __func__, vStatus);
     if ( vStatus == VOS_STATUS_E_TIMEOUT )
     {
         WDA_setNeedShutdown(vosContext);
         vos_smd_dump_stats();
         vos_dump_wdi_events();
         VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
               "%s: Test MC thread by posting a probe message to SYS",
                __func__);
         wlan_sys_probe();
     }
     VOS_ASSERT(0);
     return VOS_STATUS_E_FAILURE;
  }
  VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_INFO,
            "%s: WDA correctly started", __func__);

  pHddCtx = (hdd_context_t *)vos_get_context(VOS_MODULE_ID_HDD, pVosContext );
  if (!pHddCtx)
  {
     VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
               "%s: HDD context is null", __func__);
     goto err_wda_stop;
  }

  pHddCtx->wifi_turn_on_time_since_boot = vos_get_monotonic_boottime();

  /* Start the MAC */
  vos_mem_zero((v_PVOID_t)&halStartParams, sizeof(tHalMacStartParameters));

  /* Start the MAC */
  sirStatus = macStart(pVosContext->pMACContext,(v_PVOID_t)&halStartParams);

  if (eSIR_SUCCESS != sirStatus)
  {
    VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
              "%s: Failed to start MAC", __func__);
    goto err_wda_stop;
  }
   
  VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_INFO,
            "%s: MAC correctly started", __func__);

  /* START SME */
  vStatus = sme_Start(pVosContext->pMACContext);

  if (!VOS_IS_STATUS_SUCCESS(vStatus))
  {
    VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
               "%s: Failed to start SME", __func__);
    goto err_mac_stop;
  }

  VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_INFO,
            "%s: SME correctly started", __func__);

  /** START TL */
  vStatus = WLANTL_Start(pVosContext);
  if (!VOS_IS_STATUS_SUCCESS(vStatus))
  {
    VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
               "%s: Failed to start TL", __func__);
    goto err_sme_stop;
  }

  VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_INFO,
            "TL correctly started");
  VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_INFO,
            "%s: VOSS Start is successful!!", __func__);

  return VOS_STATUS_SUCCESS;


err_sme_stop:
  sme_Stop(pVosContext->pMACContext, HAL_STOP_TYPE_SYS_RESET);
    
err_mac_stop:
  macStop( pVosContext->pMACContext, HAL_STOP_TYPE_SYS_RESET );

err_wda_stop:   
  vos_event_reset( &(gpVosContext->wdaCompleteEvent) );
  vStatus = WDA_stop( pVosContext, HAL_STOP_TYPE_RF_KILL);
  if (!VOS_IS_STATUS_SUCCESS(vStatus))
  {
     VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
         "%s: Failed to stop WDA", __func__);
     VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vStatus ) );
     WDA_setNeedShutdown(vosContext);
  }
  else
  {
    vStatus = vos_wait_single_event( &(gpVosContext->wdaCompleteEvent),
                                     VOS_WDA_TIMEOUT );
    if( vStatus != VOS_STATUS_SUCCESS )
    {
       if( vStatus == VOS_STATUS_E_TIMEOUT )
       {
          VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
           "%s: Timeout occurred before WDA_stop complete", __func__);

       }
       else
       {
          VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
           "%s: WDA_stop reporting other error", __func__);
       }
       VOS_ASSERT( 0 );
       WDA_setNeedShutdown(vosContext);
    }
  }

  return VOS_STATUS_E_FAILURE;
   
} /* vos_start() */


/* vos_stop function */
VOS_STATUS vos_stop( v_CONTEXT_t vosContext )
{
  VOS_STATUS vosStatus;

  /* WDA_Stop is called before the SYS so that the processing of Riva 
  pending responces will not be handled during uninitialization of WLAN driver */
  vos_event_reset( &(gpVosContext->wdaCompleteEvent) );

  vosStatus = WDA_stop( vosContext, HAL_STOP_TYPE_RF_KILL );

  if (!VOS_IS_STATUS_SUCCESS(vosStatus))
  {
     VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
         "%s: Failed to stop WDA", __func__);
     VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
     WDA_setNeedShutdown(vosContext);
  }
  else
  {
    if(wcnss_device_is_shutdown())
    {
       vosStatus = VOS_STATUS_E_TIMEOUT;
       VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
        "%s: Wait for WDA_Stop complete event not needed due to SSR",
         __func__);
    }
    else
    {
       vosStatus = vos_wait_single_event( &(gpVosContext->wdaCompleteEvent),
                                       VOS_WDA_STOP_TIMEOUT );
    }

    if ( vosStatus != VOS_STATUS_SUCCESS )
    {
       if ( vosStatus == VOS_STATUS_E_TIMEOUT )
       {
          VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
           "%s: Timeout occurred before WDA complete", __func__);
       }
       else
       {
          VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
           "%s: WDA_stop reporting other error", __func__ );
       }
       VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
           "%s: Test MC thread by posting a probe message to SYS", __func__);
       wlan_sys_probe();
       WDA_setNeedShutdown(vosContext);
    }
  }

  /* SYS STOP will stop SME and MAC */
  vosStatus = sysStop( vosContext);
  if (!VOS_IS_STATUS_SUCCESS(vosStatus))
  {
     VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
         "%s: Failed to stop SYS", __func__);
     VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
  }

  vosStatus = WLANTL_Stop( vosContext );
  if (!VOS_IS_STATUS_SUCCESS(vosStatus))
  {
     VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
         "%s: Failed to stop TL", __func__);
     VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
  }


  return VOS_STATUS_SUCCESS;
}


/* vos_close function */
VOS_STATUS vos_close( v_CONTEXT_t vosContext )
{
  VOS_STATUS vosStatus;

#ifdef WLAN_BTAMP_FEATURE
  vosStatus = WLANBAP_Close(vosContext);
  if (!VOS_IS_STATUS_SUCCESS(vosStatus))
  {
     VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
         "%s: Failed to close BAP", __func__);
     VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
  }
#endif // WLAN_BTAMP_FEATURE


  vosStatus = WLANTL_Close(vosContext);
  if (!VOS_IS_STATUS_SUCCESS(vosStatus))
  {
     VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
         "%s: Failed to close TL", __func__);
     VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
  }
   
  vosStatus = sme_Close( ((pVosContextType)vosContext)->pMACContext);
  if (!VOS_IS_STATUS_SUCCESS(vosStatus))
  {
     VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
         "%s: Failed to close SME", __func__);
     VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
  }

  vosStatus = macClose( ((pVosContextType)vosContext)->pMACContext);
  if (!VOS_IS_STATUS_SUCCESS(vosStatus))
  {
     VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
         "%s: Failed to close MAC", __func__);
     VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
  }

  ((pVosContextType)vosContext)->pMACContext = NULL;

#ifndef CONFIG_ENABLE_LINUX_REG
  vosStatus = vos_nv_close();
  if (!VOS_IS_STATUS_SUCCESS(vosStatus))
  {
     VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
         "%s: Failed to close NV", __func__);
     VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
  }
#endif

  vosStatus = sysClose( vosContext );
  if (!VOS_IS_STATUS_SUCCESS(vosStatus))
  {
     VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
         "%s: Failed to close SYS", __func__);
     VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
  }

  if ( TRUE == WDA_needShutdown(vosContext ))
  {
     /* if WDA stop failed, call WDA shutdown to cleanup WDA/WDI */
     vosStatus = WDA_shutdown( vosContext, VOS_TRUE );
     if (VOS_IS_STATUS_SUCCESS( vosStatus ) )
     {
        hdd_set_ssr_required( HDD_SSR_REQUIRED );
     }
     else
     {
        VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                               "%s: Failed to shutdown WDA", __func__ );
        VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
     }
  } 
  else 
  {
     vosStatus = WDA_close( vosContext );
     if (!VOS_IS_STATUS_SUCCESS(vosStatus))
     {
        VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
            "%s: Failed to close WDA", __func__);
        VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
     }
  }
  
  /* Let DXE return packets in WDA_close and then free them here */
  vosStatus = vos_packet_close( vosContext );
  if (!VOS_IS_STATUS_SUCCESS(vosStatus))
  {
     VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
         "%s: Failed to close VOSS Packet", __func__);
     VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
  }


  vos_mq_deinit(&((pVosContextType)vosContext)->freeVosMq);

  vosStatus = vos_event_destroy(&gpVosContext->fwLogsComplete);
  if (!VOS_IS_STATUS_SUCCESS(vosStatus))
  {
     VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
         "%s: failed to destroy fwLogsComplete", __func__);
     VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
  }

  vosStatus = vos_event_destroy(&gpVosContext->wdaCompleteEvent);
  if (!VOS_IS_STATUS_SUCCESS(vosStatus))
  {
     VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
         "%s: failed to destroy wdaCompleteEvent", __func__);
     VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
  }


  vosStatus = vos_event_destroy(&gpVosContext->ProbeEvent);
  if (!VOS_IS_STATUS_SUCCESS(vosStatus))
  {
     VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
         "%s: failed to destroy ProbeEvent", __func__);
     VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
  }

  if (gpVosContext->roamDelayStatsEnabled &&
      !vos_roam_delay_stats_deinit())
  {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                "%s: Could not deinit roamDelayStats", __func__);
  }

  vos_wdthread_flush_timer_work();

  return VOS_STATUS_SUCCESS;
}
                  

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
v_VOID_t* vos_get_context( VOS_MODULE_ID moduleId, 
                           v_CONTEXT_t pVosContext )
{
  v_PVOID_t pModContext = NULL;

  if (pVosContext == NULL)
  {
    VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
        "%s: vos context pointer is null", __func__);
    return NULL;
  }

  if (gpVosContext != pVosContext)
  {
    VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, 
        "%s: pVosContext != gpVosContext", __func__);
    return NULL;
  }

  switch(moduleId)
  {
    case VOS_MODULE_ID_TL:  
    {
      pModContext = gpVosContext->pTLContext;
      break;
    }

#ifdef WLAN_BTAMP_FEATURE
    case VOS_MODULE_ID_BAP:
    {
        pModContext = gpVosContext->pBAPContext;
        break;
    }    
#endif //WLAN_BTAMP_FEATURE

    case VOS_MODULE_ID_SAP:
    {
      pModContext = gpVosContext->pSAPContext;
      break;
    }

    case VOS_MODULE_ID_HDD_SOFTAP:
    {
      pModContext = gpVosContext->pHDDSoftAPContext;
      break;
    }

    case VOS_MODULE_ID_HDD:
    {
      pModContext = gpVosContext->pHDDContext;
      break;
    }

    case VOS_MODULE_ID_SME:
    case VOS_MODULE_ID_PE:
    case VOS_MODULE_ID_PMC:
    {
      /* 
      ** In all these cases, we just return the MAC Context
      */
      pModContext = gpVosContext->pMACContext;
      break;
    }

    case VOS_MODULE_ID_WDA:
    {
      /* For WDA module */
      pModContext = gpVosContext->pWDAContext;
      break;
    }

    case VOS_MODULE_ID_VOSS:
    {
      /* For SYS this is VOS itself*/
      pModContext = gpVosContext;
      break;
    }

    default:
    {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,"%s: Module ID %i "
          "does not have its context maintained by VOSS", __func__, moduleId);
      VOS_ASSERT(0);
      return NULL;
    }
  }

  if (pModContext == NULL )
  {
    VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,"%s: Module ID %i "
          "context is Null", __func__, moduleId);
  }

  return pModContext;

} /* vos_get_context()*/


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
                                    v_VOID_t *moduleContext )
{
  if (gpVosContext == NULL)
  {
    VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, 
        "%s: global voss context is NULL", __func__);
  }

  return gpVosContext;

} /* vos_get_global_context() */


v_U8_t vos_is_logp_in_progress(VOS_MODULE_ID moduleId, v_VOID_t *moduleContext)
{
  if (gpVosContext == NULL)
  {
    VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, 
        "%s: global voss context is NULL", __func__);
    return 1;
  }

   return gpVosContext->isLogpInProgress;
}

void vos_set_logp_in_progress(VOS_MODULE_ID moduleId, v_U8_t value)
{
  if (gpVosContext == NULL)
  {
    VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, 
        "%s: global voss context is NULL", __func__);
    return;
  }

   gpVosContext->isLogpInProgress = value;
}

v_U8_t vos_is_load_unload_in_progress(VOS_MODULE_ID moduleId, v_VOID_t *moduleContext)
{
  if (gpVosContext == NULL)
  {
    VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, 
        "%s: global voss context is NULL", __func__);
    return 0; 
  }

   return gpVosContext->isLoadUnloadInProgress;
}

void vos_set_load_unload_in_progress(VOS_MODULE_ID moduleId, v_U8_t value)
{
  if (gpVosContext == NULL)
  {
    VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, 
        "%s: global voss context is NULL", __func__);
    return;
  }

   gpVosContext->isLoadUnloadInProgress = value;
}

v_U8_t vos_is_reinit_in_progress(VOS_MODULE_ID moduleId, v_VOID_t *moduleContext)
{
  if (gpVosContext == NULL)
  {
    VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
        "%s: global voss context is NULL", __func__);
    return 1;
  }

   return gpVosContext->isReInitInProgress;
}

void vos_set_reinit_in_progress(VOS_MODULE_ID moduleId, v_U8_t value)
{
  if (gpVosContext == NULL)
  {
    VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
        "%s: global voss context is NULL", __func__);
    return;
  }

   gpVosContext->isReInitInProgress = value;
}


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
                              v_VOID_t **ppModuleContext, v_SIZE_t size )
{
  v_VOID_t ** pGpModContext = NULL;

  if ( pVosContext == NULL) {
    VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, 
        "%s: vos context is null", __func__);
    return VOS_STATUS_E_FAILURE;
  }

  if (( gpVosContext != pVosContext) || ( ppModuleContext == NULL)) {
    VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, 
        "%s: context mismatch or null param passed", __func__);
    return VOS_STATUS_E_FAILURE;
  }

  switch(moduleID)
  {
    case VOS_MODULE_ID_TL:  
    {
      pGpModContext = &(gpVosContext->pTLContext); 
      break;
    }

#ifdef WLAN_BTAMP_FEATURE
    case VOS_MODULE_ID_BAP:
    {
        pGpModContext = &(gpVosContext->pBAPContext);
        break;
    }    
#endif //WLAN_BTAMP_FEATURE

    case VOS_MODULE_ID_SAP:
    {
      pGpModContext = &(gpVosContext->pSAPContext);
      break;
    }

    case VOS_MODULE_ID_WDA:
    {
      pGpModContext = &(gpVosContext->pWDAContext);
      break;
    }
    case VOS_MODULE_ID_SME:
    case VOS_MODULE_ID_PE:
    case VOS_MODULE_ID_PMC:
    case VOS_MODULE_ID_HDD:
    case VOS_MODULE_ID_HDD_SOFTAP:
    default:
    {     
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, "%s: Module ID %i "
          "does not have its context allocated by VOSS", __func__, moduleID);
      VOS_ASSERT(0);
      return VOS_STATUS_E_INVAL;
    }
  }

  if ( NULL != *pGpModContext)
  {
    /*
    ** Context has already been allocated!
    ** Prevent double allocation
    */
    VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
               "%s: Module ID %i context has already been allocated",
                __func__, moduleID);
    return VOS_STATUS_E_EXISTS;
  }
  
  /*
  ** Dynamically allocate the context for module
  */
  
  *ppModuleContext = kmalloc(size, GFP_KERNEL);

  
  if ( *ppModuleContext == NULL)
  {
    VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,"%s: Failed to "
        "allocate Context for module ID %i", __func__, moduleID);
    VOS_ASSERT(0);
    return VOS_STATUS_E_NOMEM;
  }
  
  if (moduleID==VOS_MODULE_ID_TL)
  {
     vos_mem_zero(*ppModuleContext, size);
  }

  *pGpModContext = *ppModuleContext;

  return VOS_STATUS_SUCCESS;

} /* vos_alloc_context() */


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
                             v_VOID_t *pModuleContext )
{
  v_VOID_t ** pGpModContext = NULL;

  if (( pVosContext == NULL) || ( gpVosContext != pVosContext) ||
      ( pModuleContext == NULL))
  {
    VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
        "%s: Null params or context mismatch", __func__);
    return VOS_STATUS_E_FAILURE;
  }
  

  switch(moduleID)
  {
    case VOS_MODULE_ID_TL:  
    {
      pGpModContext = &(gpVosContext->pTLContext); 
      break;
    }

#ifdef WLAN_BTAMP_FEATURE
    case VOS_MODULE_ID_BAP:
    {
        pGpModContext = &(gpVosContext->pBAPContext);
        break;
    }
#endif //WLAN_BTAMP_FEATURE
 
    case VOS_MODULE_ID_SAP:
    {
      pGpModContext = &(gpVosContext->pSAPContext); 
      break;
    }

    case VOS_MODULE_ID_WDA:
    {
      pGpModContext = &(gpVosContext->pWDAContext);
      break;
    }
    case VOS_MODULE_ID_HDD:
    case VOS_MODULE_ID_SME:
    case VOS_MODULE_ID_PE:
    case VOS_MODULE_ID_PMC:
    case VOS_MODULE_ID_HDD_SOFTAP:
    default:
    {     
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, "%s: Module ID %i "
          "does not have its context allocated by VOSS", __func__, moduleID);
      VOS_ASSERT(0);
      return VOS_STATUS_E_INVAL;
    }
  }

  if ( NULL == *pGpModContext)
  {
    /*
    ** Context has not been allocated or freed already!
    */
    VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,"%s: Module ID %i "
        "context has not been allocated or freed already", __func__,moduleID);
    return VOS_STATUS_E_FAILURE;
  }
  
  if (*pGpModContext != pModuleContext)
  {
    VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, 
        "%s: pGpModContext != pModuleContext", __func__);
    return VOS_STATUS_E_FAILURE;
  } 
  
  if(pModuleContext != NULL)
      kfree(pModuleContext);

  *pGpModContext = NULL;

  return VOS_STATUS_SUCCESS;

} /* vos_free_context() */


bool vos_is_log_report_in_progress(void)
{
    return wlan_is_log_report_in_progress();
}

void vos_reset_log_report_in_progress(void)
{
    return wlan_reset_log_report_in_progress();
}




int vos_set_log_completion(uint32 is_fatal,
                            uint32 indicator,
                            uint32 reason_code)
{
    return wlan_set_log_completion(is_fatal,
                                   indicator,reason_code);
}

void vos_get_log_and_reset_completion(uint32 *is_fatal,
                             uint32 *indicator,
                             uint32 *reason_code,
                             bool reset)
{
    wlan_get_log_and_reset_completion(is_fatal, indicator, reason_code, reset);
}



void vos_send_fatal_event_done(void)
{
    /*Complete the fwLogsComplete Event*/
    VosContextType *vos_context;
    uint32_t is_fatal, indicator, reason_code;

    vos_context = vos_get_global_context(VOS_MODULE_ID_SYS, NULL);
    if (!vos_context) {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
            "%s: vos context is Invalid", __func__);
        return;
    }
    VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_INFO,
         "%s: vos_event_set for fwLogsComplete", __func__);
    if (vos_event_set(&vos_context->fwLogsComplete)!= VOS_STATUS_SUCCESS)
    {
        VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
         "%s: vos_event_set failed for fwLogsComplete", __func__);
        return;
    }
    /*The below API will reset is_report_in_progress flag*/
    vos_get_log_and_reset_completion(&is_fatal, &indicator,
                                         &reason_code, true);
    VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_INFO,
         "is_fatal : %d, indicator: %d, reason_code=%d",
         is_fatal, indicator, reason_code);
    wlan_report_log_completion(is_fatal, indicator, reason_code);

    /* Do ssr after reporting fatal event to recover from
     * below conditions
     */
    if ((WLAN_LOG_INDICATOR_HOST_DRIVER == indicator) &&
        (WLAN_LOG_REASON_SME_COMMAND_STUCK == reason_code ||
         WLAN_LOG_REASON_SME_OUT_OF_CMD_BUF == reason_code ||
         WLAN_LOG_REASON_SCAN_NOT_ALLOWED == reason_code))
    {
         VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
           "Do SSR for reason_code=%d", reason_code);
         vos_wlanRestart();
    }
}

/**
 * vos_isFatalEventEnabled()
 *
 * Return TRUE if Fatal event is enabled is in progress.
 *
 */
v_BOOL_t vos_isFatalEventEnabled(void)
{
    hdd_context_t *pHddCtx = NULL;
    v_CONTEXT_t pVosContext = vos_get_global_context(VOS_MODULE_ID_SYS, NULL);

    if(!pVosContext)
    {
       hddLog(VOS_TRACE_LEVEL_FATAL,"%s: Global VOS context is Null", __func__);
       return FALSE;
    }

    pHddCtx = (hdd_context_t *)vos_get_context(VOS_MODULE_ID_HDD, pVosContext );
    if(!pHddCtx) {
       VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "%s: HDD context is Null", __func__);
       return FALSE;
    }

    return pHddCtx->cfg_ini->enableFatalEvent;
}

/**---------------------------------------------------------------------------

  \brief __vos_fatal_event_logs_req() - used to send flush command to FW

  This API is wrapper to SME flush API.

  \param is_fatal - fatal or non fatal event
         indicator - Tyoe of indicator framework/Host/FW
         reason_code - reason code for flush logs

  \return VOS_STATUS_SUCCESS - if command is sent successfully.
          VOS_STATUS_E_FAILURE - if command is not sent successfully.
  --------------------------------------------------------------------------*/
VOS_STATUS __vos_fatal_event_logs_req( uint32_t is_fatal,
                        uint32_t indicator,
                        uint32_t reason_code,
                        bool wait_required,
                        bool dump_vos_trace)
{
    VOS_STATUS vosStatus;
    eHalStatus status;
    VosContextType *vos_context;
    hdd_context_t *pHddCtx = NULL;

    vos_context = vos_get_global_context(VOS_MODULE_ID_SYS, NULL);
    if (!vos_context)
    {
        VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
            "%s: vos context is Invalid", __func__);
        return VOS_STATUS_E_FAILURE;
    }
    pHddCtx = (hdd_context_t *)vos_get_context(VOS_MODULE_ID_HDD, vos_context );
    if(!pHddCtx) {
       VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "%s: HDD context is Null", __func__);
       return VOS_STATUS_E_FAILURE;
    }

    if(!pHddCtx->cfg_ini->wlanLoggingEnable)
    {
       VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_INFO,
            "%s: Wlan logging not enabled", __func__);
        return VOS_STATUS_E_FAILURE;
    }

    if(!pHddCtx->cfg_ini->enableFatalEvent)
    {
       VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_INFO,
            "%s: Fatal event not enabled", __func__);
        return VOS_STATUS_E_FAILURE;
    }

    if (pHddCtx->isLoadUnloadInProgress ||
        vos_context->isLogpInProgress)
    {
       VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
            "%s: un/Load/SSR in progress", __func__);
        return VOS_STATUS_E_FAILURE;
    }

    if (vos_is_log_report_in_progress() == true)
    {
        VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
        "%s: Fatal Event Req already in progress - dropping! type:%d, indicator=%d reason_code=%d",
        __func__, is_fatal, indicator, reason_code);
        return VOS_STATUS_E_FAILURE;
    }

    vosStatus = vos_set_log_completion(is_fatal, indicator, reason_code);
    if (VOS_STATUS_SUCCESS != vosStatus) {
        VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
        "%s: Failed to set log trigger params for fatalEvent", __func__);
        return VOS_STATUS_E_FAILURE;
    }
    VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
        "%s: Triggering fatal Event: type:%d, indicator=%d reason_code=%d",
        __func__, is_fatal, indicator, reason_code);

    status = vos_event_reset(&gpVosContext->fwLogsComplete);
    if(!HAL_STATUS_SUCCESS(status))
    {
        VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                    FL("fwLogsComplete reset failed:%d"),status);
        return VOS_STATUS_E_FAILURE;
    }
    status = sme_fatal_event_logs_req(vos_context->pMACContext,
                                      is_fatal, indicator,
                                      reason_code, dump_vos_trace);

    if (HAL_STATUS_SUCCESS(status) && (wait_required == TRUE))
    {

        /* Need to update time out of complete */
        vosStatus = vos_wait_single_event(&gpVosContext->fwLogsComplete,
                                    WAIT_TIME_FW_LOGS);
        if ( vosStatus != VOS_STATUS_SUCCESS )
        {
            if ( vosStatus == VOS_STATUS_E_TIMEOUT )
            {
                VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                 "%s: Timeout occurred before fwLogsComplete", __func__);
            }
            else
            {
                VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                     "%s: fwLogsComplete reporting other error", __func__);
            }
            /*Done indication is not received.So reset the bug report in progress*/
            vos_reset_log_report_in_progress();
            return VOS_STATUS_E_FAILURE;
        }
    }
    if (HAL_STATUS_SUCCESS( status ))
        return VOS_STATUS_SUCCESS;
    else
        return VOS_STATUS_E_FAILURE;
}

VOS_STATUS vos_fatal_event_logs_req( uint32_t is_fatal,
                        uint32_t indicator,
                        uint32_t reason_code,
                        bool wait_required,
                        bool dump_vos_trace)
{
    VOS_STATUS status;

    vos_ssr_protect(__func__);
    status = __vos_fatal_event_logs_req(is_fatal, indicator, reason_code,
                                        wait_required, dump_vos_trace);
    vos_ssr_unprotect(__func__);

    return status;
}

/**---------------------------------------------------------------------------

  \brief vos_process_done_indication() - Process the done indication for fatal event,
   FW memory dump

  This API processes the done indication and wakeup the logger thread accordingly.

  \param type - Type for which done indication is received.


  \return VOS_STATUS_SUCCESS - the pkt has been successfully queued.
          VOS_STATUS_E_FAILURE - the pkt queue handler has reported
          a failure.
  --------------------------------------------------------------------------*/

VOS_STATUS vos_process_done_indication(v_U8_t type, v_U32_t reason_code)
{
    wlan_process_done_indication(type, reason_code);
    return VOS_STATUS_SUCCESS;
}

/**
 * vos_flush_host_logs_for_fatal() -flush host logs and send
 * fatal event to upper layer.
 */
void vos_flush_host_logs_for_fatal(void)
{
   wlan_flush_host_logs_for_fatal();
   return;
}


/**---------------------------------------------------------------------------

  \brief vos_logger_pkt_serialize() - queue a logging vos pkt

  This API allows single vos pkt to be queued and later sent to userspace by
  logger thread.

  \param pPacket - a pointer to a vos pkt to be queued
         pkt_type - type of pkt to be queued

  \return VOS_STATUS_SUCCESS - the pkt has been successfully queued.
          VOS_STATUS_E_FAILURE - the pkt queue handler has reported
          a failure.
  --------------------------------------------------------------------------*/
VOS_STATUS vos_logger_pkt_serialize( vos_pkt_t *pPacket, uint32 pkt_type)
{
#ifdef WLAN_LOGGING_SOCK_SVC_ENABLE
      return wlan_queue_logpkt_for_app(pPacket, pkt_type);
#else
      return vos_pkt_return_packet(pPacket);
#endif
}

void vos_per_pkt_stats_to_user(void *perPktStat)
{
#ifdef WLAN_LOGGING_SOCK_SVC_ENABLE
     wlan_pkt_stats_to_user(perPktStat);
#else
     return;
#endif



}

void vos_updatePktStatsInfo(void * pktStat)
{
#ifdef WLAN_LOGGING_SOCK_SVC_ENABLE
     wlan_fillTxStruct(pktStat);
#else
     return;
#endif

}


/**---------------------------------------------------------------------------
  
  \brief vos_mq_post_message() - post a message to a message queue

  This API allows messages to be posted to a specific message queue.  Messages
  can be posted to the following message queues:
  
  <ul>
    <li> SME
    <li> PE
    <li> HAL
    <li> TL
  </ul> 
  
  \param msgQueueId - identifies the message queue upon which the message
         will be posted.
         
  \param message - a pointer to a message buffer.  Memory for this message 
         buffer is allocated by the caller and free'd by the vOSS after the
         message is posted to the message queue.  If the consumer of the 
         message needs anything in this message, it needs to copy the contents
         before returning from the message queue handler.
  
  \return VOS_STATUS_SUCCESS - the message has been successfully posted
          to the message queue.
          
          VOS_STATUS_E_INVAL - The value specified by msgQueueId does not 
          refer to a valid Message Queue Id.
          
          VOS_STATUS_E_FAULT  - message is an invalid pointer.     
          
          VOS_STATUS_E_FAILURE - the message queue handler has reported
          an unknown failure.

  \sa
  
  --------------------------------------------------------------------------*/
VOS_STATUS vos_mq_post_message( VOS_MQ_ID msgQueueId, vos_msg_t *pMsg )
{
  pVosMqType      pTargetMq   = NULL;
  pVosMsgWrapper  pMsgWrapper = NULL;

  if ((gpVosContext == NULL) || (pMsg == NULL))
  {
    VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
        "%s: Null params or global vos context is null", __func__);
    VOS_ASSERT(0);
    return VOS_STATUS_E_FAILURE;
  }

  switch (msgQueueId)
  {
    /// Message Queue ID for messages bound for SME
    case  VOS_MQ_ID_SME: 
    {
       pTargetMq = &(gpVosContext->vosSched.smeMcMq);
       break;
    }

    /// Message Queue ID for messages bound for PE
    case VOS_MQ_ID_PE:  
    {
       pTargetMq = &(gpVosContext->vosSched.peMcMq);
       break;
    }

    /// Message Queue ID for messages bound for WDA
    case VOS_MQ_ID_WDA: 
    {
       pTargetMq = &(gpVosContext->vosSched.wdaMcMq);
       break;
    }

    /// Message Queue ID for messages bound for WDI
    case VOS_MQ_ID_WDI:
    {
       pTargetMq = &(gpVosContext->vosSched.wdiMcMq);
       break;
    }

    /// Message Queue ID for messages bound for TL
    case VOS_MQ_ID_TL: 
    {
       pTargetMq = &(gpVosContext->vosSched.tlMcMq);
       break;
    }

    /// Message Queue ID for messages bound for the SYS module
    case VOS_MQ_ID_SYS:
    {
       pTargetMq = &(gpVosContext->vosSched.sysMcMq);
       break;
    }

    default:

    VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
              ("%s: Trying to queue msg into unknown MC Msg queue ID %d"),
              __func__, msgQueueId);

    return VOS_STATUS_E_FAILURE;
  }

  VOS_ASSERT(NULL !=pTargetMq);
  if (pTargetMq == NULL)
  {
     VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, 
         "%s: pTargetMq == NULL", __func__);
     return VOS_STATUS_E_FAILURE;
  } 

  /*
  ** Try and get a free Msg wrapper
  */
  pMsgWrapper = vos_mq_get(&gpVosContext->freeVosMq);

  if (NULL == pMsgWrapper)
  {
    VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
              "%s: VOS Core run out of message wrapper", __func__);
    if (!gpVosContext->vosWrapperFullReported)
    {
      gpVosContext->vosWrapperFullReported = 1;
      vos_fatal_event_logs_req(WLAN_LOG_TYPE_FATAL,
                      WLAN_LOG_INDICATOR_HOST_ONLY,
                      WLAN_LOG_REASON_VOS_MSG_UNDER_RUN,
                      FALSE, TRUE);
    }
    return VOS_STATUS_E_RESOURCES;
  }
  
  /*
  ** Copy the message now
  */
  vos_mem_copy( (v_VOID_t*)pMsgWrapper->pVosMsg, 
                (v_VOID_t*)pMsg, sizeof(vos_msg_t));

  vos_mq_put(pTargetMq, pMsgWrapper);

  set_bit(MC_POST_EVENT, &gpVosContext->vosSched.mcEventFlag);
  wake_up_interruptible(&gpVosContext->vosSched.mcWaitQueue);

  return VOS_STATUS_SUCCESS;

} /* vos_mq_post_message()*/


/**--------------------------------------------------------------------------
  \brief vos_mq_post_message_high_pri() - posts a high priority message to
           a message queue

  This API allows messages to be posted to the head of a specific message
  queue. Messages  can be posted to the following message queues:

  <ul>
    <li> SME
    <li> PE
    <li> HAL
    <li> TL
  </ul>

  \param msgQueueId - identifies the message queue upon which the message
         will be posted.

  \param message - a pointer to a message buffer.  Memory for this message
         buffer is allocated by the caller and free'd by the vOSS after the
         message is posted to the message queue.  If the consumer of the
         message needs anything in this message, it needs to copy the contents
         before returning from the message queue handler.

  \return VOS_STATUS_SUCCESS - the message has been successfully posted
          to the message queue.

          VOS_STATUS_E_INVAL - The value specified by msgQueueId does not
          refer to a valid Message Queue Id.

          VOS_STATUS_E_FAULT  - message is an invalid pointer.

          VOS_STATUS_E_FAILURE - the message queue handler has reported
          an unknown failure.
  --------------------------------------------------------------------------*/
VOS_STATUS vos_mq_post_message_high_pri(VOS_MQ_ID msgQueueId, vos_msg_t *pMsg)
{
  pVosMqType      pTargetMq   = NULL;
  pVosMsgWrapper  pMsgWrapper = NULL;

  if ((gpVosContext == NULL) || (pMsg == NULL))
  {
    VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
        "%s: Null params or global vos context is null", __func__);
    VOS_ASSERT(0);
    return VOS_STATUS_E_FAILURE;
  }

  switch (msgQueueId)
  {
    /// Message Queue ID for messages bound for SME
    case  VOS_MQ_ID_SME:
    {
       pTargetMq = &(gpVosContext->vosSched.smeMcMq);
       break;
    }

    /// Message Queue ID for messages bound for PE
    case VOS_MQ_ID_PE:
    {
       pTargetMq = &(gpVosContext->vosSched.peMcMq);
       break;
    }

    /// Message Queue ID for messages bound for WDA
    case VOS_MQ_ID_WDA:
    {
       pTargetMq = &(gpVosContext->vosSched.wdaMcMq);
       break;
    }

    /// Message Queue ID for messages bound for WDI
    case VOS_MQ_ID_WDI:
    {
       pTargetMq = &(gpVosContext->vosSched.wdiMcMq);
       break;
    }

    /// Message Queue ID for messages bound for TL
    case VOS_MQ_ID_TL:
    {
       pTargetMq = &(gpVosContext->vosSched.tlMcMq);
       break;
    }

    /// Message Queue ID for messages bound for the SYS module
    case VOS_MQ_ID_SYS:
    {
       pTargetMq = &(gpVosContext->vosSched.sysMcMq);
       break;
    }

    default:

    VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
              ("%s: Trying to queue msg into unknown MC Msg queue ID %d"),
              __func__, msgQueueId);

    return VOS_STATUS_E_FAILURE;
  }

  VOS_ASSERT(NULL !=pTargetMq);
  if (pTargetMq == NULL)
  {
     VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
         "%s: pTargetMq == NULL", __func__);
     return VOS_STATUS_E_FAILURE;
  }

  /*
  ** Try and get a free Msg wrapper
  */
  pMsgWrapper = vos_mq_get(&gpVosContext->freeVosMq);

  if (NULL == pMsgWrapper)
  {
    VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
              "%s: VOS Core run out of message wrapper", __func__);
    if (!gpVosContext->vosWrapperFullReported)
    {
      gpVosContext->vosWrapperFullReported = 1;
      vos_fatal_event_logs_req(WLAN_LOG_TYPE_FATAL,
                      WLAN_LOG_INDICATOR_HOST_ONLY,
                      WLAN_LOG_REASON_VOS_MSG_UNDER_RUN,
                      FALSE, TRUE);
    }
    return VOS_STATUS_E_RESOURCES;
  }

  /*
  ** Copy the message now
  */
  vos_mem_copy( (v_VOID_t*)pMsgWrapper->pVosMsg,
                (v_VOID_t*)pMsg, sizeof(vos_msg_t));

  vos_mq_put_front(pTargetMq, pMsgWrapper);

  set_bit(MC_POST_EVENT, &gpVosContext->vosSched.mcEventFlag);
  wake_up_interruptible(&gpVosContext->vosSched.mcWaitQueue);

  return VOS_STATUS_SUCCESS;

} /* vos_mq_post_message_high_pri()*/



/**---------------------------------------------------------------------------
  
  \brief vos_tx_mq_serialize() - serialize a message to the Tx execution flow

  This API allows messages to be posted to a specific message queue in the 
  Tx excution flow.  Messages for the Tx execution flow can be posted only 
  to the following queue.
  
  <ul>
    <li> TL
    <li> SSC/WDI
  </ul>
  
  \param msgQueueId - identifies the message queue upon which the message
         will be posted.
         
  \param message - a pointer to a message buffer.  Body memory for this message 
         buffer is allocated by the caller and free'd by the vOSS after the
         message is dispacthed to the appropriate component.  If the consumer 
         of the message needs to keep anything in the body, it needs to copy 
         the contents before returning from the message handler.
  
  \return VOS_STATUS_SUCCESS - the message has been successfully posted
          to the message queue.
          
          VOS_STATUS_E_INVAL - The value specified by msgQueueId does not 
          refer to a valid Message Queue Id.
          
          VOS_STATUS_E_FAULT  - message is an invalid pointer.     
          
          VOS_STATUS_E_FAILURE - the message queue handler has reported
          an unknown failure.

  \sa
  
  --------------------------------------------------------------------------*/
VOS_STATUS vos_tx_mq_serialize( VOS_MQ_ID msgQueueId, vos_msg_t *pMsg )
{
  pVosMqType      pTargetMq   = NULL;
  pVosMsgWrapper  pMsgWrapper = NULL;

  if ((gpVosContext == NULL) || (pMsg == NULL))
  {
    VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
        "%s: Null params or global vos context is null", __func__);
    VOS_ASSERT(0);
    return VOS_STATUS_E_FAILURE;
  }

  switch (msgQueueId)
  {
    /// Message Queue ID for messages bound for SME
    case  VOS_MQ_ID_TL: 
    {
       pTargetMq = &(gpVosContext->vosSched.tlTxMq);
       break;
    }

    /// Message Queue ID for messages bound for SSC
    case VOS_MQ_ID_WDI:  
    {
       pTargetMq = &(gpVosContext->vosSched.wdiTxMq);
       break;
    }
    
    /// Message Queue ID for messages bound for the SYS module
    case VOS_MQ_ID_SYS:
    {
       pTargetMq = &(gpVosContext->vosSched.sysTxMq);
       break;
    }

    default:

    VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
              "%s: Trying to queue msg into unknown Tx Msg queue ID %d",
               __func__, msgQueueId);

    return VOS_STATUS_E_FAILURE;
  }

  if (pTargetMq == NULL)
  {
     VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, 
         "%s: pTargetMq == NULL", __func__);
     return VOS_STATUS_E_FAILURE;
  } 
    

  /*
  ** Try and get a free Msg wrapper
  */
  pMsgWrapper = vos_mq_get(&gpVosContext->freeVosMq);

  if (NULL == pMsgWrapper)
  {
    VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
              "%s: VOS Core run out of message wrapper", __func__);
    if (!gpVosContext->vosWrapperFullReported)
    {
      gpVosContext->vosWrapperFullReported = 1;
      vos_fatal_event_logs_req(WLAN_LOG_TYPE_FATAL,
                      WLAN_LOG_INDICATOR_HOST_ONLY,
                      WLAN_LOG_REASON_VOS_MSG_UNDER_RUN,
                      FALSE, TRUE);
    }

    return VOS_STATUS_E_RESOURCES;
  }

  /*
  ** Copy the message now
  */
  vos_mem_copy( (v_VOID_t*)pMsgWrapper->pVosMsg, 
                (v_VOID_t*)pMsg, sizeof(vos_msg_t));

  vos_mq_put(pTargetMq, pMsgWrapper);

  set_bit(TX_POST_EVENT, &gpVosContext->vosSched.txEventFlag);
  wake_up_interruptible(&gpVosContext->vosSched.txWaitQueue);

  return VOS_STATUS_SUCCESS;

} /* vos_tx_mq_serialize()*/

/**---------------------------------------------------------------------------

  \brief vos_rx_mq_serialize() - serialize a message to the Rx execution flow

  This API allows messages to be posted to a specific message queue in the
  Tx excution flow.  Messages for the Rx execution flow can be posted only
  to the following queue.

  <ul>
    <li> TL
    <li> WDI
  </ul>

  \param msgQueueId - identifies the message queue upon which the message
         will be posted.

  \param message - a pointer to a message buffer.  Body memory for this message
         buffer is allocated by the caller and free'd by the vOSS after the
         message is dispacthed to the appropriate component.  If the consumer
         of the message needs to keep anything in the body, it needs to copy
         the contents before returning from the message handler.

  \return VOS_STATUS_SUCCESS - the message has been successfully posted
          to the message queue.

          VOS_STATUS_E_INVAL - The value specified by msgQueueId does not
          refer to a valid Message Queue Id.

          VOS_STATUS_E_FAULT  - message is an invalid pointer.

          VOS_STATUS_E_FAILURE - the message queue handler has reported
          an unknown failure.

  \sa

  --------------------------------------------------------------------------*/

VOS_STATUS vos_rx_mq_serialize( VOS_MQ_ID msgQueueId, vos_msg_t *pMsg )
{
  pVosMqType      pTargetMq   = NULL;
  pVosMsgWrapper  pMsgWrapper = NULL;
  if ((gpVosContext == NULL) || (pMsg == NULL))
  {
    VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
        "%s: Null params or global vos context is null", __func__);
    VOS_ASSERT(0);
    return VOS_STATUS_E_FAILURE;
  }

  switch (msgQueueId)
  {

    case VOS_MQ_ID_SYS:
    {
       pTargetMq = &(gpVosContext->vosSched.sysRxMq);
       break;
    }

    /// Message Queue ID for messages bound for WDI
    case VOS_MQ_ID_WDI:
    {
       pTargetMq = &(gpVosContext->vosSched.wdiRxMq);
       break;
    }
    case VOS_MQ_ID_TL:
    {
       pTargetMq = &(gpVosContext->vosSched.tlRxMq);
       break;
    }

    default:

    VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
              "%s: Trying to queue msg into unknown Rx Msg queue ID %d",
               __func__, msgQueueId);

    return VOS_STATUS_E_FAILURE;
  }

  if (pTargetMq == NULL)
  {
     VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
         "%s: pTargetMq == NULL", __func__);
     return VOS_STATUS_E_FAILURE;
  }


  /*
  ** Try and get a free Msg wrapper
  */
  pMsgWrapper = vos_mq_get(&gpVosContext->freeVosMq);

  if (NULL == pMsgWrapper)
  {
    VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
              "%s: VOS Core run out of message wrapper", __func__);
    if (!gpVosContext->vosWrapperFullReported)
    {
      gpVosContext->vosWrapperFullReported = 1;
      vos_fatal_event_logs_req(WLAN_LOG_TYPE_FATAL,
                      WLAN_LOG_INDICATOR_HOST_ONLY,
                      WLAN_LOG_REASON_VOS_MSG_UNDER_RUN,
                      FALSE, TRUE);
    }

    return VOS_STATUS_E_RESOURCES;
  }

  /*
  ** Copy the message now
  */
  vos_mem_copy( (v_VOID_t*)pMsgWrapper->pVosMsg,
                (v_VOID_t*)pMsg, sizeof(vos_msg_t));

  vos_mq_put(pTargetMq, pMsgWrapper);

  set_bit(RX_POST_EVENT, &gpVosContext->vosSched.rxEventFlag);
  wake_up_interruptible(&gpVosContext->vosSched.rxWaitQueue);

  return VOS_STATUS_SUCCESS;

} /* vos_rx_mq_serialize()*/

v_VOID_t 
vos_sys_probe_thread_cback 
( 
  v_VOID_t *pUserData 
)
{
  if (gpVosContext != pUserData)
  {
     VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, 
         "%s: gpVosContext != pUserData", __func__);
     return;
  } 

  if (vos_event_set(&gpVosContext->ProbeEvent)!= VOS_STATUS_SUCCESS)
  {
     VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, 
         "%s: vos_event_set failed", __func__);
     return;
  }

} /* vos_sys_probe_thread_cback() */

v_VOID_t vos_WDAComplete_cback
(
  v_VOID_t *pUserData
)
{

  if (gpVosContext != pUserData)
  {
     VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
         "%s: gpVosContext != pUserData", __func__);
     return;
  }

  if (vos_event_set(&gpVosContext->wdaCompleteEvent)!= VOS_STATUS_SUCCESS)
  {
     VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
         "%s: vos_event_set failed", __func__);
     return;
  }

} /* vos_WDAComplete_cback() */

v_VOID_t vos_core_return_msg
(
  v_PVOID_t      pVContext, 
  pVosMsgWrapper pMsgWrapper
)
{
  pVosContextType pVosContext = (pVosContextType) pVContext;
  
  VOS_ASSERT( gpVosContext == pVosContext);

  if (gpVosContext != pVosContext)
  {
     VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, 
         "%s: gpVosContext != pVosContext", __func__);
     return;
  } 

  VOS_ASSERT( NULL !=pMsgWrapper );

  if (pMsgWrapper == NULL)
  {
     VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, 
         "%s: pMsgWrapper == NULL in function", __func__);
     return;
  } 
  
  /*
  ** Return the message on the free message queue
  */
  INIT_LIST_HEAD(&pMsgWrapper->msgNode);
  vos_mq_put(&pVosContext->freeVosMq, pMsgWrapper);

} /* vos_core_return_msg() */


/**
  @brief vos_fetch_tl_cfg_parms() - this function will attempt to read the
  TL config params from the registry
   
  @param pAdapter : [inout] pointer to TL config block

  @return 
  None

*/
v_VOID_t 
vos_fetch_tl_cfg_parms 
( 
  WLANTL_ConfigInfoType *pTLConfig,
  hdd_config_t * pConfig
)
{
  if (pTLConfig == NULL)
  {
   VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, "%s NULL ptr passed in!", __func__);
   return;
  }

  pTLConfig->ucAcWeights[0] = pConfig->WfqBkWeight;
  pTLConfig->ucAcWeights[1] = pConfig->WfqBeWeight;
  pTLConfig->ucAcWeights[2] = pConfig->WfqViWeight;
  pTLConfig->ucAcWeights[3] = pConfig->WfqVoWeight;
  pTLConfig->ucAcWeights[4] = pConfig->WfqVoWeight;
  pTLConfig->ucReorderAgingTime[0] = pConfig->BkReorderAgingTime;/*WLANTL_AC_BK*/
  pTLConfig->ucReorderAgingTime[1] = pConfig->BeReorderAgingTime;/*WLANTL_AC_BE*/
  pTLConfig->ucReorderAgingTime[2] = pConfig->ViReorderAgingTime;/*WLANTL_AC_VI*/
  pTLConfig->ucReorderAgingTime[3] = pConfig->VoReorderAgingTime;/*WLANTL_AC_VO*/
  pTLConfig->uDelayedTriggerFrmInt = pConfig->DelayedTriggerFrmInt;
  pTLConfig->uMinFramesProcThres = pConfig->MinFramesProcThres;

}

v_BOOL_t vos_is_apps_power_collapse_allowed(void* pHddCtx)
{
  return hdd_is_apps_power_collapse_allowed((hdd_context_t*) pHddCtx);
}

/*---------------------------------------------------------------------------

  \brief vos_shutdown() - shutdown VOS

     - All VOS submodules are closed.

     - All the WLAN SW components should have been opened. This includes
       SYS, MAC, SME and TL.


  \param  vosContext: Global vos context


  \return VOS_STATUS_SUCCESS - Operation successfull & vos is shutdown

          VOS_STATUS_E_FAILURE - Failure to close

---------------------------------------------------------------------------*/
VOS_STATUS vos_shutdown(v_CONTEXT_t vosContext)
{
  VOS_STATUS vosStatus;

#ifdef WLAN_BTAMP_FEATURE
  vosStatus = WLANBAP_Close(vosContext);
  if (!VOS_IS_STATUS_SUCCESS(vosStatus))
  {
     VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
         "%s: Failed to close BAP", __func__);
     VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
  }
#endif // WLAN_BTAMP_FEATURE

  vosStatus = WLANTL_Close(vosContext);
  if (!VOS_IS_STATUS_SUCCESS(vosStatus))
  {
     VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
         "%s: Failed to close TL", __func__);
     VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
  }

  vosStatus = sme_Close( ((pVosContextType)vosContext)->pMACContext);
  if (!VOS_IS_STATUS_SUCCESS(vosStatus))
  {
     VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
         "%s: Failed to close SME", __func__);
     VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
  }

  vosStatus = macClose( ((pVosContextType)vosContext)->pMACContext);
  if (!VOS_IS_STATUS_SUCCESS(vosStatus))
  {
     VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
         "%s: Failed to close MAC", __func__);
     VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
  }

  ((pVosContextType)vosContext)->pMACContext = NULL;

  vosStatus = sysClose( vosContext );
  if (!VOS_IS_STATUS_SUCCESS(vosStatus))
  {
     VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
         "%s: Failed to close SYS", __func__);
     VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
  }

 /* Let DXE return packets in WDA_close and then free them here */
  vosStatus = vos_packet_close( vosContext );
  if (!VOS_IS_STATUS_SUCCESS(vosStatus))
  {
     VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
         "%s: Failed to close VOSS Packet", __func__);
     VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
  }

  vos_mq_deinit(&((pVosContextType)vosContext)->freeVosMq);

  vosStatus = vos_event_destroy(&gpVosContext->fwLogsComplete);
  if (!VOS_IS_STATUS_SUCCESS(vosStatus))
  {
     VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
         "%s: failed to destroy fwLogsComplete", __func__);
     VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
  }


  vosStatus = vos_event_destroy(&gpVosContext->wdaCompleteEvent);
  if (!VOS_IS_STATUS_SUCCESS(vosStatus))
  {
     VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
         "%s: failed to destroy wdaCompleteEvent", __func__);
     VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
  }

  vosStatus = vos_event_destroy(&gpVosContext->ProbeEvent);
  if (!VOS_IS_STATUS_SUCCESS(vosStatus))
  {
     VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
         "%s: failed to destroy ProbeEvent", __func__);
     VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
  }

  return VOS_STATUS_SUCCESS;
}

/*---------------------------------------------------------------------------

  \brief vos_wda_shutdown() - VOS interface to wda shutdown

     - WDA/WDI shutdown

  \param  vosContext: Global vos context


  \return VOS_STATUS_SUCCESS - Operation successfull

          VOS_STATUS_E_FAILURE - Failure to close

---------------------------------------------------------------------------*/
VOS_STATUS vos_wda_shutdown(v_CONTEXT_t vosContext)
{
  VOS_STATUS vosStatus;
  vosStatus = WDA_shutdown(vosContext, VOS_FALSE);

  if (!VOS_IS_STATUS_SUCCESS(vosStatus))
  {
     VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
         "%s: failed to shutdown WDA", __func__);
     VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
  }
  return vosStatus;
}
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
VOS_STATUS vos_wlanShutdown(void)
{
   VOS_STATUS vstatus;
   vstatus = vos_watchdog_wlan_shutdown();
   return vstatus;
}
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
VOS_STATUS vos_wlanReInit(void)
{
   VOS_STATUS vstatus;
   vstatus = vos_watchdog_wlan_re_init();
   return vstatus;
}
/**
  @brief vos_wlanRestart() - This API will reload WLAN driver.

  This function is called if driver detects any fatal state which 
  can be recovered by a WLAN module reload ( Android framwork initiated ).
  Note that this API will not initiate any RIVA subsystem restart.

  The function wlan_hdd_restart_driver protects against re-entrant calls.

  @param
       NONE
  @return
       VOS_STATUS_SUCCESS   - Operation completed successfully.
       VOS_STATUS_E_FAILURE - Operation failed.
       VOS_STATUS_E_EMPTY   - No configured interface
       VOS_STATUS_E_ALREADY - Request already in progress


*/
VOS_STATUS vos_wlanRestart(void)
{
   VOS_STATUS vstatus;
   hdd_context_t *pHddCtx = NULL;
   v_CONTEXT_t pVosContext        = NULL;

   /* Check whether driver load unload is in progress */
   if(vos_is_load_unload_in_progress( VOS_MODULE_ID_VOSS, NULL)) 
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, 
               "%s: Driver load/unload is in progress, retry later.", __func__);
      return VOS_STATUS_E_AGAIN;
   }

   /* Get the Global VOSS Context */
   pVosContext = vos_get_global_context(VOS_MODULE_ID_VOSS, NULL);
   if(!pVosContext) {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, 
               "%s: Global VOS context is Null", __func__);
      return VOS_STATUS_E_FAILURE;
   }
    
   /* Get the HDD context */
   pHddCtx = (hdd_context_t *)vos_get_context(VOS_MODULE_ID_HDD, pVosContext );
   if(!pHddCtx) {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, 
               "%s: HDD context is Null", __func__);
      return VOS_STATUS_E_FAILURE;
   }

   /* Reload the driver */
   vstatus = wlan_hdd_restart_driver(pHddCtx);
   return vstatus;
}


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
                        tANI_U32 arg3, tANI_U32 arg4, tANI_U8 async)
{
   WDA_HALDumpCmdReq(NULL, cmd, arg1, arg2, arg3, arg4, NULL, async);
}

v_U64_t vos_get_monotonic_boottime(void)
{
    struct timespec ts;
    wcnss_get_monotonic_boottime(&ts);
    return (((v_U64_t)ts.tv_sec * 1000000) + (ts.tv_nsec / 1000));
}

/**---------------------------------------------------------------------------

  \brief vos_randomize_n_bytes() - HDD Random Mac Addr Generator

  This generates the random mac address for WLAN interface

  \param  - mac_addr - pointer to Mac address

  \return -  0 for success, < 0 for failure

  --------------------------------------------------------------------------*/

VOS_STATUS  vos_randomize_n_bytes(void *start_addr, tANI_U32 n)
{

    if (start_addr == NULL )
        return VOS_STATUS_E_FAILURE;

    get_random_bytes( start_addr, n);

    return eHAL_STATUS_SUCCESS;
}

/**---------------------------------------------------------------------------

  \brief vos_is_wlan_in_badState() - get isFatalError flag from WD Ctx

  \param  - VOS_MODULE_ID   - module id
          - moduleContext   - module context

  \return -  isFatalError value if WDCtx is valid otherwise true

  --------------------------------------------------------------------------*/
v_BOOL_t vos_is_wlan_in_badState(VOS_MODULE_ID moduleId,
                                 v_VOID_t *moduleContext)
{
    struct _VosWatchdogContext *pVosWDCtx = get_vos_watchdog_ctxt();

    if (pVosWDCtx == NULL){
        VOS_TRACE(moduleId, VOS_TRACE_LEVEL_ERROR,
                "%s: global wd context is null", __func__);

        return TRUE;
    }
    return pVosWDCtx->isFatalError;
}

/**---------------------------------------------------------------------------

  \brief vos_is_fw_logging_enabled() -

  API to check if firmware is configured to send logs using DXE channel

  \param  -  None

  \return -  0: firmware logging is not enabled (it may or may not
                be supported)
             1: firmware logging is enabled

  --------------------------------------------------------------------------*/
v_U8_t vos_is_fw_logging_enabled(void)
{
   return hdd_is_fw_logging_enabled();
}

/**---------------------------------------------------------------------------

  \brief vos_is_fw_ev_logging_enabled() -

  API to check if firmware is configured to send live logs using DXE channel

  \param  -  None

  \return -  0: firmware logging is not enabled (it may or may not
                be supported)
             1: firmware logging is enabled

  --------------------------------------------------------------------------*/
v_U8_t vos_is_fw_ev_logging_enabled(void)
{
   return hdd_is_fw_ev_logging_enabled();
}

/**---------------------------------------------------------------------------

  \brief vos_is_fw_logging_supported() -

  API to check if firmware supports to send logs using DXE channel

  \param  -  None

  \return -  0: firmware logging is not supported
             1: firmware logging is supported

  --------------------------------------------------------------------------*/
v_U8_t vos_is_fw_logging_supported(void)
{
   return IS_FRAME_LOGGING_SUPPORTED_BY_FW;
}
/**---------------------------------------------------------------------------

  \brief vos_set_roam_delay_stats_enabled() -

  API to set value of roamDelayStatsEnabled in vos context

  \param  -  value to be updated

  \return -  NONE

  --------------------------------------------------------------------------*/

v_VOID_t  vos_set_roam_delay_stats_enabled(v_U8_t value)
{
    gpVosContext->roamDelayStatsEnabled = value;
}


/**---------------------------------------------------------------------------

  \brief vos_get_roam_delay_stats_enabled() -

  API to get value of roamDelayStatsEnabled from vos context

  \param  -  NONE

  \return -  value of roamDelayStatsEnabled

  --------------------------------------------------------------------------*/

v_U8_t  vos_get_roam_delay_stats_enabled(v_VOID_t)
{
    return gpVosContext->roamDelayStatsEnabled;
}

v_U32_t vos_get_dxeReplenishRXTimerVal(void)
{
    hdd_context_t *pHddCtx = NULL;
    v_CONTEXT_t pVosContext = NULL;

    /* Get the Global VOSS Context */
    pVosContext = vos_get_global_context(VOS_MODULE_ID_SYS, NULL);
    if(!pVosContext) {
       hddLog(VOS_TRACE_LEVEL_FATAL, "%s: Global VOS context is Null", __func__);
       return 0;
    }

    /* Get the HDD context */
    pHddCtx = (hdd_context_t *)vos_get_context(VOS_MODULE_ID_HDD, pVosContext );
    if(!pHddCtx) {
       hddLog(VOS_TRACE_LEVEL_FATAL, "%s: HDD context is Null", __func__);
       return 0;
     }

   return pHddCtx->cfg_ini->dxeReplenishRXTimerVal;
}

v_BOOL_t vos_get_dxeSSREnable(void)
{
    hdd_context_t *pHddCtx = NULL;
    v_CONTEXT_t pVosContext = NULL;

    /* Get the Global VOSS Context */
    pVosContext = vos_get_global_context(VOS_MODULE_ID_SYS, NULL);
    if(!pVosContext) {
       hddLog(VOS_TRACE_LEVEL_FATAL, "%s: Global VOS context is Null", __func__);
       return FALSE;
    }

    /* Get the HDD context */
    pHddCtx = (hdd_context_t *)vos_get_context(VOS_MODULE_ID_HDD, pVosContext );
    if(!pHddCtx) {
       hddLog(VOS_TRACE_LEVEL_FATAL, "%s: HDD context is Null", __func__);
       return FALSE;
     }

   return pHddCtx->cfg_ini->dxeSSREnable;
}

v_VOID_t vos_flush_work(struct work_struct *work)
{
#if defined (WLAN_OPEN_SOURCE)
   cancel_work_sync(work);
#else
   wcnss_flush_work(work);
#endif
}

v_VOID_t vos_flush_delayed_work(struct delayed_work *dwork)
{
#if defined (WLAN_OPEN_SOURCE)
   cancel_delayed_work_sync(dwork);
#else
   wcnss_flush_delayed_work(dwork);
#endif
}

v_VOID_t vos_init_work(struct work_struct *work , void *callbackptr)
{
#if defined (WLAN_OPEN_SOURCE)
   INIT_WORK(work,callbackptr);
#else
   wcnss_init_work(work, callbackptr);
#endif
}

v_VOID_t vos_init_delayed_work(struct delayed_work *dwork , void *callbackptr)
{
#if defined (WLAN_OPEN_SOURCE)
   INIT_DELAYED_WORK(dwork,callbackptr);
#else
   wcnss_init_delayed_work(dwork, callbackptr);
#endif
}

/**
 * vos_set_multicast_logging() - Set mutlicast logging value
 * @value: Value of multicast logging
 *
 * Set the multicast logging value which will indicate
 * whether to multicast host and fw messages even
 * without any registration by userspace entity
 *
 * Return: None
 */
void vos_set_multicast_logging(uint8_t value)
{
   vos_multicast_logging = value;
}

/**
 * vos_is_multicast_logging() - Get multicast logging value
 *
 * Get the multicast logging value which will indicate
 * whether to multicast host and fw messages even
 * without any registration by userspace entity
 *
 * Return: 0 - Multicast logging disabled, 1 - Multicast logging enabled
 */
v_U8_t vos_is_multicast_logging(void)
{
   return vos_multicast_logging;
}

/**
 * vos_isLoadUnloadInProgress()
 *
 * Return TRUE if load/unload is in progress.
 *
 */
v_BOOL_t vos_isLoadUnloadInProgress(void)
{
    hdd_context_t *pHddCtx = NULL;
    v_CONTEXT_t pVosContext = vos_get_global_context(VOS_MODULE_ID_SYS, NULL);

    if(!pVosContext)
    {
       hddLog(VOS_TRACE_LEVEL_FATAL,"%s: Global VOS context is Null", __func__);
       return FALSE;
    }

    pHddCtx = (hdd_context_t *)vos_get_context(VOS_MODULE_ID_HDD, pVosContext );
    if(!pHddCtx) {
       VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "%s: HDD context is Null", __func__);
       return FALSE;
    }

    return ( 0 != pHddCtx->isLoadUnloadInProgress);
}

/**
 * vos_isUnloadInProgress()
 *
 * Return TRUE if unload is in progress.
 *
 */
v_BOOL_t vos_isUnloadInProgress(void)
{
    hdd_context_t *pHddCtx = NULL;
    v_CONTEXT_t pVosContext = vos_get_global_context(VOS_MODULE_ID_SYS, NULL);

    if(!pVosContext)
    {
       hddLog(VOS_TRACE_LEVEL_FATAL,"%s: Global VOS context is Null", __func__);
       return FALSE;
    }

    pHddCtx = (hdd_context_t *)vos_get_context(VOS_MODULE_ID_HDD, pVosContext );
    if(!pHddCtx) {
       VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "%s: HDD context is Null", __func__);
       return FALSE;
    }

    return (WLAN_HDD_UNLOAD_IN_PROGRESS == pHddCtx->isLoadUnloadInProgress);
}

/**
 *vos_get_rx_wow_dump()
 *
 * Return true/flase to dump RX packet
 *
 */
bool vos_get_rx_wow_dump(void)
{
    hdd_context_t *pHddCtx = NULL;
    v_CONTEXT_t pVosContext = vos_get_global_context(VOS_MODULE_ID_SYS, NULL);

    if(!pVosContext)
    {
       hddLog(VOS_TRACE_LEVEL_FATAL,"%s: Global VOS context is Null", __func__);
       return FALSE;
    }

    pHddCtx = (hdd_context_t *)vos_get_context(VOS_MODULE_ID_HDD, pVosContext );
    if(!pHddCtx) {
       VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "%s: HDD context is Null", __func__);
       return FALSE;
    }

    return pHddCtx->rx_wow_dump;
}

/**
 *vos_set_rx_wow_dump() - Set RX wow pkt dump
 *@value: Value of RX wow pkt dump
 *
 * Return none.
 *
 */
void vos_set_rx_wow_dump(bool value)
{
    hdd_context_t *pHddCtx = NULL;
    v_CONTEXT_t pVosContext = vos_get_global_context(VOS_MODULE_ID_SYS, NULL);

    if(!pVosContext)
    {
       hddLog(VOS_TRACE_LEVEL_FATAL,"%s: Global VOS context is Null", __func__);
       return;
    }

    pHddCtx = (hdd_context_t *)vos_get_context(VOS_MODULE_ID_HDD, pVosContext );
    if(!pHddCtx) {
       VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                "%s: HDD context is Null", __func__);
       return;
    }

    pHddCtx->rx_wow_dump = value;
}

/**
 * vos_probe_threads() - VOS API to post messages
 * to all the threads to detect if they are active or not
 *
 * Return none.
 *
 */
void vos_probe_threads(void)
{
    vos_msg_t msg;

    msg.callback = vos_wd_reset_thread_stuck_count;
    /* Post Message to MC Thread */
    sysBuildMessageHeader(SYS_MSG_ID_MC_THR_PROBE, &msg);
    if (VOS_STATUS_SUCCESS != vos_mq_post_message(VOS_MQ_ID_SYS, &msg)) {
         VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
          FL("Unable to post SYS_MSG_ID_MC_THR_PROBE message to MC thread"));
    }

    /* Post Message to Tx Thread */
    sysBuildMessageHeader(SYS_MSG_ID_TX_THR_PROBE, &msg);
    if (VOS_STATUS_SUCCESS != vos_tx_mq_serialize(VOS_MQ_ID_SYS, &msg)) {
        VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
          FL("Unable to post SYS_MSG_ID_TX_THR_PROBE message to TX thread"));
    }

    /* Post Message to Rx Thread */
    sysBuildMessageHeader(SYS_MSG_ID_RX_THR_PROBE, &msg);
    if (VOS_STATUS_SUCCESS != vos_rx_mq_serialize(VOS_MQ_ID_SYS, &msg)) {
        VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
          FL("Unable to post SYS_MSG_ID_RX_THR_PROBE message to RX thread"));
    }
}

/**
 * vos_set_ring_log_level() - Convert HLOS values to driver log levels
 * @ring_id: ring_id
 * @log_levelvalue: Log level specificed
 *
 * This function sets the log level of a particular ring
 *
 * Return: None
 */
 void vos_set_ring_log_level(v_U32_t ring_id, v_U32_t log_level)
{
    VosContextType *vos_context;
    v_U32_t log_val;

    vos_context = vos_get_global_context(VOS_MODULE_ID_SYS, NULL);
    if (!vos_context) {
      VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
              "%s: vos context is Invald", __func__);
      return;
    }

    switch (log_level) {
    case LOG_LEVEL_NO_COLLECTION:
        log_val = WLAN_LOG_LEVEL_OFF;
        break;
    case LOG_LEVEL_NORMAL_COLLECT:
        log_val = WLAN_LOG_LEVEL_NORMAL;
        break;
    case LOG_LEVEL_ISSUE_REPRO:
        log_val = WLAN_LOG_LEVEL_REPRO;
        break;
    case LOG_LEVEL_ACTIVE:
    default:
        log_val = WLAN_LOG_LEVEL_ACTIVE;
        break;
    }

    if (ring_id == RING_ID_WAKELOCK) {
        vos_context->wakelock_log_level = log_val;
        return;
    } else if (ring_id == RING_ID_CONNECTIVITY) {
        vos_context->connectivity_log_level = log_val;
        return;
    } else if (ring_id == RING_ID_PER_PACKET_STATS) {
        vos_context->packet_stats_log_level = log_val;
        if (WLAN_LOG_LEVEL_ACTIVE != log_val)
            wlan_disable_and_flush_pkt_stats();

        return;
    }
}
/**
 * vos_get_ring_log_level() - Get the a ring id's log level
 * @ring_id: Ring id
 *
 * Fetch and return the log level corresponding to a ring id
 *
 * Return: Log level corresponding to the ring ID
 */
v_U8_t vos_get_ring_log_level(v_U32_t ring_id)
{
    VosContextType *vos_context;

     vos_context = vos_get_global_context(VOS_MODULE_ID_SYS, NULL);
     if (!vos_context) {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
            "%s: vos context is Invald", __func__);
        return WLAN_LOG_LEVEL_OFF;
    }

    if (ring_id == RING_ID_WAKELOCK)
        return vos_context->wakelock_log_level;
    else if (ring_id == RING_ID_CONNECTIVITY)
        return vos_context->connectivity_log_level;
    else if (ring_id == RING_ID_PER_PACKET_STATS)
        return vos_context->packet_stats_log_level;

    return WLAN_LOG_LEVEL_OFF;
}

/* elements are rate, preamable, bw, short_gi */
rateidx_to_rate_bw_preamble_sgi   rateidx_to_rate_bw_preamble_sgi_table[] =
{
/*11B CCK Long preamble (0-3)*/
{ 10, PREAMBLE_CCK, S_BW20, 0},{ 20, PREAMBLE_CCK, S_BW20, 0},
{ 55, PREAMBLE_CCK, S_BW20, 0},{ 110, PREAMBLE_CCK, S_BW20, 0},
/*11B CCK Short preamble (4-7)*/
{ 10, PREAMBLE_CCK, S_BW20, 0},{ 20, PREAMBLE_CCK, S_BW20, 0},
{ 55, PREAMBLE_CCK, S_BW20, 0},{ 110, PREAMBLE_CCK, S_BW20, 0},
/*11G/A (8-15)*/
{ 60, PREAMBLE_OFDM, S_BW20, 0},{ 90, PREAMBLE_OFDM, S_BW20, 0},
{ 120, PREAMBLE_OFDM, S_BW20, 0},{ 180, PREAMBLE_OFDM, S_BW20, 0},
{ 240, PREAMBLE_OFDM, S_BW20, 0},{ 360, PREAMBLE_OFDM, S_BW20, 0},
{ 480, PREAMBLE_OFDM, S_BW20, 0},{ 540, PREAMBLE_OFDM, S_BW20, 0},
/*HT20 LGI MCS 0-7 (16-23)*/
{ 65, PREAMBLE_HT, S_BW20, 0},{ 130, PREAMBLE_HT, S_BW20, 0},
{ 195, PREAMBLE_HT, S_BW20, 0},{ 260, PREAMBLE_HT, S_BW20, 0},
{ 390, PREAMBLE_HT, S_BW20, 0},{ 520, PREAMBLE_HT, S_BW20, 0},
{ 585, PREAMBLE_HT, S_BW20, 0},{ 650, PREAMBLE_HT, S_BW20, 0},
/*HT20 SGI MCS 0-7 (24-31)*/
{ 72, PREAMBLE_HT, S_BW20, 1},{ 144, PREAMBLE_HT, S_BW20, 1},
{ 217, PREAMBLE_HT, S_BW20, 1},{ 289, PREAMBLE_HT, S_BW20, 1},
{ 433, PREAMBLE_HT, S_BW20, 1},{ 578, PREAMBLE_HT, S_BW20, 1},
{ 650, PREAMBLE_HT, S_BW20, 1},{ 722, PREAMBLE_HT, S_BW20, 1},
/*HT20 Greenfield MCS 0-7 rates (32-39)*/
{ 65, PREAMBLE_HT, S_BW20, 0},{ 130, PREAMBLE_HT, S_BW20, 0},
{ 195, PREAMBLE_HT, S_BW20, 0},{ 260, PREAMBLE_HT, S_BW20, 0},
{ 390, PREAMBLE_HT, S_BW20, 0},{ 520, PREAMBLE_HT, S_BW20, 0},
{ 585, PREAMBLE_HT, S_BW20, 0},{ 650, PREAMBLE_HT, S_BW20, 0},
/*HT40 LGI MCS 0-7 (40-47)*/
{ 135, PREAMBLE_HT, S_BW40, 0},{ 270, PREAMBLE_HT, S_BW40, 0},
{ 405, PREAMBLE_HT, S_BW40, 0},{ 540, PREAMBLE_HT, S_BW40, 0},
{ 810, PREAMBLE_HT, S_BW40, 0},{ 1080, PREAMBLE_HT, S_BW40, 0},
{ 1215, PREAMBLE_HT, S_BW40, 0},{ 1350, PREAMBLE_HT, S_BW40, 0},
/*HT40 SGI MCS 0-7 (48-55)*/
{ 150, PREAMBLE_HT, S_BW40, 1},{ 300, PREAMBLE_HT, S_BW40, 1},
{ 450, PREAMBLE_HT, S_BW40, 1},{ 600, PREAMBLE_HT, S_BW40, 1},
{ 900, PREAMBLE_HT, S_BW40, 1},{ 1200, PREAMBLE_HT, S_BW40, 1},
{ 1350, PREAMBLE_HT, S_BW40, 1},{ 1500, PREAMBLE_HT, S_BW40, 1},
/*HT40 Greenfield MCS 0-7 rates (56-63) 64-65 are dummy*/
{ 135, PREAMBLE_HT, S_BW40, 0},{ 270, PREAMBLE_HT, S_BW40, 0},
{ 405, PREAMBLE_HT, S_BW40, 0},{ 540, PREAMBLE_HT, S_BW40, 0},
{ 810, PREAMBLE_HT, S_BW40, 0},{ 1080, PREAMBLE_HT, S_BW40, 0},
{ 1215, PREAMBLE_HT, S_BW40, 0},{ 1350, PREAMBLE_HT, S_BW40, 0},
/*64-65 are dummy*/
{ 1350, PREAMBLE_HT, S_BW40, 0},{ 1350, PREAMBLE_HT, S_BW40, 0},
/*VHT20 LGI  MCS 0-9 rates (66-75)*/
{ 65, PREAMBLE_VHT, S_BW20, 0},{ 130, PREAMBLE_VHT, S_BW20, 0},
{ 195, PREAMBLE_VHT, S_BW20, 0},{ 260, PREAMBLE_VHT, S_BW20, 0},
{ 390, PREAMBLE_VHT, S_BW20, 0},{ 520, PREAMBLE_VHT, S_BW20, 0},
{ 585, PREAMBLE_VHT, S_BW20, 0},{ 650, PREAMBLE_VHT, S_BW20, 0},
{ 780, PREAMBLE_VHT, S_BW20, 0},{ 865, PREAMBLE_VHT, S_BW20, 0},
/*76-77 are dummy*/
{ 865, PREAMBLE_VHT, S_BW20, 0},{ 865, PREAMBLE_VHT, S_BW20, 0},
/*VHT20 SGI MCS 0-9 rates (78-87)*/
{ 72, PREAMBLE_VHT, S_BW20, 1},{ 144, PREAMBLE_VHT, S_BW20, 1},
{ 217, PREAMBLE_VHT, S_BW20, 1},{ 289, PREAMBLE_VHT, S_BW20, 1},
{ 433, PREAMBLE_VHT, S_BW20, 1},{ 578, PREAMBLE_VHT, S_BW20, 1},
{ 650, PREAMBLE_VHT, S_BW20, 1},{ 722, PREAMBLE_VHT, S_BW20, 1},
{ 867, PREAMBLE_VHT, S_BW20, 1},{ 961, PREAMBLE_VHT, S_BW20, 1},
/*88-89 are dummy*/
{ 961, PREAMBLE_VHT, S_BW20, 1},{ 961, PREAMBLE_VHT, S_BW20, 1},
/*VHT40 LGI MCS 0-9 rates (90-101) 98,101 is Dummy*/
{ 135, PREAMBLE_VHT, S_BW40, 0},{ 270, PREAMBLE_VHT, S_BW40, 0},
{ 405, PREAMBLE_VHT, S_BW40, 0},{ 540, PREAMBLE_VHT, S_BW40, 0},
{ 810, PREAMBLE_VHT, S_BW40, 0},{ 1080, PREAMBLE_VHT, S_BW40, 0},
{ 1215, PREAMBLE_VHT, S_BW40, 0},{ 1350, PREAMBLE_VHT, S_BW40, 0},
{ 1350, PREAMBLE_VHT, S_BW40, 0},{ 1620, PREAMBLE_VHT, S_BW40, 0},
{ 1800, PREAMBLE_VHT, S_BW40, 0},{ 1800, PREAMBLE_VHT, S_BW40, 0},
/*VHT40 SGI MCS 0-9 rates (102-112) 110, 113 is Dummy*/
{ 150, PREAMBLE_VHT, S_BW40, 1},{ 300, PREAMBLE_VHT, S_BW40, 1},
{ 450, PREAMBLE_VHT, S_BW40, 1},{ 600, PREAMBLE_VHT, S_BW40, 1},
{ 900, PREAMBLE_VHT, S_BW40, 1},{ 1200, PREAMBLE_VHT, S_BW40, 1},
{ 1350, PREAMBLE_VHT, S_BW40, 1},{ 1500, PREAMBLE_VHT, S_BW40, 1},
{ 1500, PREAMBLE_VHT, S_BW40, 1},{ 1800, PREAMBLE_VHT, S_BW40, 1},
{ 2000, PREAMBLE_VHT, S_BW40, 1},{ 2000, PREAMBLE_VHT, S_BW40, 1},
/*VHT80 LGI MCS 0-9 rates (114-125) 122, 125 is Dummy*/
{ 293, PREAMBLE_VHT, S_BW80, 0},{ 585, PREAMBLE_VHT, S_BW80, 0},
{ 878, PREAMBLE_VHT, S_BW80, 0},{ 1170, PREAMBLE_VHT, S_BW80, 0},
{ 1755, PREAMBLE_VHT, S_BW80, 0},{ 2340, PREAMBLE_VHT, S_BW80, 0},
{ 2633, PREAMBLE_VHT, S_BW80, 0},{ 2925, PREAMBLE_VHT, S_BW80, 0},
{ 2925, PREAMBLE_VHT, S_BW80, 0},{ 3510, PREAMBLE_VHT, S_BW80, 0},
{ 3900, PREAMBLE_VHT, S_BW80, 0},{ 3900, PREAMBLE_VHT, S_BW80, 0},
/*VHT80 SGI MCS 0-9 rates (126-136) 134 is Dummy*/
{ 325, PREAMBLE_VHT, S_BW80, 1},{ 650, PREAMBLE_VHT, S_BW80, 1},
{ 975, PREAMBLE_VHT, S_BW80, 1},{ 1300, PREAMBLE_VHT, S_BW80, 1},
{ 1950, PREAMBLE_VHT, S_BW80, 1},{ 2600, PREAMBLE_VHT, S_BW80, 1},
{ 2925, PREAMBLE_VHT, S_BW80, 1},{ 3250, PREAMBLE_VHT, S_BW80, 1},
{ 3250, PREAMBLE_VHT, S_BW80, 1},{ 3900, PREAMBLE_VHT, S_BW80, 1},
{ 4333, PREAMBLE_VHT, S_BW80, 1},
};

void get_rate_and_MCS(per_packet_stats *stats, uint32 rateindex)
{
    rateidx_to_rate_bw_preamble_sgi *ratetbl;

    if (STATS_MAX_RATE_INDEX < rateindex)
        rateindex = STATS_MAX_RATE_INDEX;
    ratetbl= &rateidx_to_rate_bw_preamble_sgi_table[rateindex];
    stats->last_transmit_rate = ratetbl->rate/5;
    stats->MCS.nss = 0;
    if (0 <= rateindex && rateindex <= 7)
        stats->MCS.rate = 7 - rateindex;
    else if (8 <= rateindex && rateindex <= 15)
    {
        switch(rateindex)
        {
            case 8:stats->MCS.rate = 3; break;
            case 9:stats->MCS.rate = 7; break;
            case 10:stats->MCS.rate = 2; break;
            case 11:stats->MCS.rate = 6; break;
            case 12:stats->MCS.rate = 1; break;
            case 13:stats->MCS.rate = 5; break;
            case 14:stats->MCS.rate = 0; break;
            case 15:stats->MCS.rate = 4; break;
        }
    }
    else if(16 <= rateindex && rateindex <= 23)
        stats->MCS.rate = rateindex - 16;
    else if(24 <= rateindex  && rateindex <= 31)
        stats->MCS.rate =  rateindex - 24;
    else if(32 <= rateindex  && rateindex <= 39)
        stats->MCS.rate = rateindex - 32;
    else if(40 <= rateindex && rateindex <= 47)
        stats->MCS.rate = rateindex - 40;
    else if(48 <= rateindex && rateindex <= 55)
        stats->MCS.rate = rateindex - 48;
    else if(56 <= rateindex && rateindex <= 63)
        stats->MCS.rate = rateindex - 56;
    else if(66 <= rateindex && rateindex <= 75)
        stats->MCS.rate = rateindex - 66;
    else if(78 <= rateindex && rateindex <= 87)
        stats->MCS.rate = rateindex - 78;
    else if(90 <= rateindex && rateindex <= 100)
        stats->MCS.rate = rateindex - 90;
    else if(78 <= rateindex && rateindex <= 87)
        stats->MCS.rate = rateindex - 78;
    else if(90 <= rateindex && rateindex <= 97)
        stats->MCS.rate = rateindex - 90;
    else if(99 <= rateindex && rateindex <= 100)
        stats->MCS.rate = rateindex - 91;
    else if(102 <= rateindex && rateindex <= 109)
        stats->MCS.rate = rateindex - 102;
    else if(111 <= rateindex && rateindex <= 112)
        stats->MCS.rate = rateindex - 103;
    else if(114 <= rateindex && rateindex <= 121)
        stats->MCS.rate = rateindex - 114;
    else if(123 <= rateindex && rateindex <= 124)
        stats->MCS.rate = rateindex - 115;
    else if(126 <= rateindex && rateindex <= 133)
        stats->MCS.rate = rateindex - 126;
    else if(135 <= rateindex && rateindex <= 136)
        stats->MCS.rate = rateindex - 127;
    else /*Invalid rate index mark it 0*/
        stats->MCS.rate = 0;
    stats->MCS.preamble = ratetbl->preamble;
    stats->MCS.bw = ratetbl->bw;
    stats->MCS.short_gi = ratetbl->short_gi;
}

bool vos_isPktStatsEnabled(void)
{
    bool value;
    value = wlan_isPktStatsEnabled();
    return (value);
}

bool vos_is_wlan_logging_enabled(void)
{
    v_CONTEXT_t vos_ctx = vos_get_global_context(VOS_MODULE_ID_SYS, NULL);
    hdd_context_t *hdd_ctx;

    if(!vos_ctx)
    {
       hddLog(VOS_TRACE_LEVEL_FATAL,"%s: Global VOS context is Null", __func__);
       return false;
    }

    hdd_ctx = vos_get_context(VOS_MODULE_ID_HDD, vos_ctx);

    if(!hdd_ctx)
    {
       hddLog(VOS_TRACE_LEVEL_FATAL,"%s: HDD context is Null", __func__);
       return false;
    }

    if (!hdd_ctx->cfg_ini->wlanLoggingEnable)
    {
       hddLog(VOS_TRACE_LEVEL_FATAL,"%s: Logging framework not enabled!", __func__);
       return false;
    }

    return true;
}

/**---------------------------------------------------------------------------

  \brief vos_is_probe_rsp_offload_enabled -

  API to check if probe response offload feature is enabled from ini

  \param  -  None

  \return -  0: probe response offload is disabled
             1: probe response offload is enabled

  --------------------------------------------------------------------------*/
v_BOOL_t vos_is_probe_rsp_offload_enabled(void)
{
	hdd_context_t *pHddCtx = NULL;
	v_CONTEXT_t pVosContext = NULL;

	/* Get the Global VOSS Context */
	pVosContext = vos_get_global_context(VOS_MODULE_ID_SYS, NULL);
	if (!pVosContext) {
		hddLog(VOS_TRACE_LEVEL_FATAL,
		       "%s: Global VOS context is Null", __func__);
		return FALSE;
	}

	/* Get the HDD context */
	pHddCtx = (hdd_context_t *)vos_get_context(VOS_MODULE_ID_HDD,
						   pVosContext);
	if (!pHddCtx) {
		hddLog(VOS_TRACE_LEVEL_FATAL,
		       "%s: HDD context is Null", __func__);
		return FALSE;
	}

	return pHddCtx->cfg_ini->sap_probe_resp_offload;
}

void vos_smd_dump_stats(void)
{
  WCTS_Dump_Smd_status();
}

void vos_log_wdi_event(uint16 msg, vos_wdi_trace_event_type event)
{

   if (gvos_wdi_msg_trace_index >= VOS_TRACE_INDEX_MAX)
   {
          gvos_wdi_msg_trace_index = 0;
   }

   gvos_wdi_msg_trace[gvos_wdi_msg_trace_index].event = event;
   gvos_wdi_msg_trace[gvos_wdi_msg_trace_index].time =
                                 vos_get_monotonic_boottime();
   gvos_wdi_msg_trace[gvos_wdi_msg_trace_index].message =  msg;
   gvos_wdi_msg_trace_index++;

   return;
}

void vos_dump_wdi_events(void)
{
   int i;

   for(i = 0; i < VOS_TRACE_INDEX_MAX; i++) {
            VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
            "%s:event:%d time:%lld msg:%d ",__func__,
            gvos_wdi_msg_trace[i].event,
            gvos_wdi_msg_trace[i].time,
            gvos_wdi_msg_trace[i].message);
  }
}
