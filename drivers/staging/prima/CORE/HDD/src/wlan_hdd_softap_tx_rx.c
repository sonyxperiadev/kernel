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

/**===========================================================================
  
  \file  wlan_hdd_softap_tx_rx.c
  
  \brief Linux HDD Tx/RX APIs
  
  ==========================================================================*/

/*--------------------------------------------------------------------------- 
  Include files
  -------------------------------------------------------------------------*/ 
#include <linux/semaphore.h>
#include <wlan_hdd_tx_rx.h>
#include <wlan_hdd_softap_tx_rx.h>
#include <wlan_hdd_dp_utils.h>
#include <wlan_qct_tl.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/etherdevice.h>
//#include <vos_list.h>
#include <vos_types.h>
#include <vos_sched.h>
#include <aniGlobal.h>
#include <halTypes.h>
#include <net/ieee80211_radiotap.h>
#include <linux/ratelimit.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0))
#include <soc/qcom/subsystem_restart.h>
#else
#include <mach/subsystem_restart.h>
#endif
#include "sapInternal.h"
#include  "wlan_hdd_trace.h"
#include "vos_diag_core_event.h"
/*--------------------------------------------------------------------------- 
  Preprocessor definitions and constants
  -------------------------------------------------------------------------*/ 

/*--------------------------------------------------------------------------- 
  Type declarations
  -------------------------------------------------------------------------*/ 

/*--------------------------------------------------------------------------- 
  Function definitions and documenation
  -------------------------------------------------------------------------*/ 
#if 0
static void hdd_softap_dump_sk_buff(struct sk_buff * skb)
{
  VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_ERROR,"%s: head = %p", __func__, skb->head);
  //VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_ERROR,"%s: data = %p", __func__, skb->data);
  VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_ERROR,"%s: tail = %p", __func__, skb->tail);
  VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_ERROR,"%s: end = %p", __func__, skb->end);
  VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_ERROR,"%s: len = %d", __func__, skb->len);
  VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_ERROR,"%s: data_len = %d", __func__, skb->data_len);
  VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_ERROR,"%s: mac_len = %d", __func__, skb->mac_len);

  VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_ERROR,"0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x",
     skb->data[0], skb->data[1], skb->data[2], skb->data[3], skb->data[4], 
     skb->data[5], skb->data[6], skb->data[7]); 
  VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_ERROR,"0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x",
     skb->data[8], skb->data[9], skb->data[10], skb->data[11], skb->data[12],
     skb->data[13], skb->data[14], skb->data[15]); 
}
#endif

extern void hdd_set_wlan_suspend_mode(bool suspend);

#define HDD_SAP_TX_TIMEOUT_RATELIMIT_INTERVAL 20*HZ
#define HDD_SAP_TX_TIMEOUT_RATELIMIT_BURST    1
#define HDD_SAP_TX_STALL_SSR_THRESHOLD        5
#define HDD_SAP_TX_STALL_RECOVERY_THRESHOLD HDD_SAP_TX_STALL_SSR_THRESHOLD - 2
#define HDD_SAP_TX_STALL_FATAL_EVENT_THRESHOLD    2


static DEFINE_RATELIMIT_STATE(hdd_softap_tx_timeout_rs,                 \
                              HDD_SAP_TX_TIMEOUT_RATELIMIT_INTERVAL,    \
                              HDD_SAP_TX_TIMEOUT_RATELIMIT_BURST);

/**============================================================================
  @brief hdd_softap_traffic_monitor_timeout_handler() -
         SAP/P2P GO traffin monitor timeout handler function
         If no traffic during programmed time, trigger suspand mode

  @param pUsrData : [in] pointer to hdd context
  @return         : NONE
  ===========================================================================*/
void hdd_softap_traffic_monitor_timeout_handler( void *pUsrData )
{
   hdd_context_t *pHddCtx = (hdd_context_t *)pUsrData;
   v_TIME_t       currentTS;

   ENTER();
   if (0 != (wlan_hdd_validate_context(pHddCtx)))
   {
       return;
   }

   currentTS = vos_timer_get_system_time();
   if (pHddCtx->cfg_ini->trafficIdleTimeout <
       (currentTS - pHddCtx->traffic_monitor.lastFrameTs))
   {
      VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_INFO,
          "%s: No Data Activity calling Wlan Suspend", __func__ );
      hdd_set_wlan_suspend_mode(1);
      atomic_set(&pHddCtx->traffic_monitor.isActiveMode, 0);
   }
   else
   {
      vos_timer_start(&pHddCtx->traffic_monitor.trafficTimer,
                      pHddCtx->cfg_ini->trafficIdleTimeout);
   }

   EXIT();
   return;
}

VOS_STATUS hdd_start_trafficMonitor( hdd_adapter_t *pAdapter, bool re_init)
{

    hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
    VOS_STATUS status = VOS_STATUS_SUCCESS;

    ENTER();

    if (!re_init) {
        status = wlan_hdd_validate_context(pHddCtx);
        if (-ENODEV == status) {
            return status;
        }
    }

    if ((pHddCtx->cfg_ini->enableTrafficMonitor) &&
        (!pHddCtx->traffic_monitor.isInitialized))
    {
        atomic_set(&pHddCtx->traffic_monitor.isActiveMode, 1);
        vos_timer_init(&pHddCtx->traffic_monitor.trafficTimer,
                      VOS_TIMER_TYPE_SW,
                      hdd_softap_traffic_monitor_timeout_handler,
                      pHddCtx);
        vos_lock_init(&pHddCtx->traffic_monitor.trafficLock);
        pHddCtx->traffic_monitor.isInitialized = 1;
        pHddCtx->traffic_monitor.lastFrameTs   = 0;
        /* Start traffic monitor timer here
         * If no AP assoc, immediatly go into suspend */
        VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_INFO,
                  "%s  Start Traffic Monitor Timer", __func__);
        vos_timer_start(&pHddCtx->traffic_monitor.trafficTimer,
                      pHddCtx->cfg_ini->trafficIdleTimeout);
    }
    else
    {
        VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_INFO,
                  "%s  Traffic Monitor is not Enable in ini file", __func__);
    }

    EXIT();
    return status;
}

VOS_STATUS hdd_stop_trafficMonitor( hdd_adapter_t *pAdapter, bool re_init)
{
    hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
    VOS_STATUS status = VOS_STATUS_SUCCESS;

    ENTER();

    if (!re_init){
        status = wlan_hdd_validate_context(pHddCtx);
        if (-ENODEV == status) {
            return status;
        }
    }

    if (pHddCtx->traffic_monitor.isInitialized)
    {
        if (VOS_TIMER_STATE_STOPPED !=
            vos_timer_getCurrentState(&pHddCtx->traffic_monitor.trafficTimer))
        {
            VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_INFO,
                      "%s  Stop Traffic Monitor Timer", __func__);
            vos_timer_stop(&pHddCtx->traffic_monitor.trafficTimer);
        }
        VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_INFO,
                  "%s  Destroy Traffic Monitor Timer", __func__);
        vos_timer_destroy(&pHddCtx->traffic_monitor.trafficTimer);
        vos_lock_destroy(&pHddCtx->traffic_monitor.trafficLock);
        pHddCtx->traffic_monitor.isInitialized = 0;
    }
    EXIT();
    return VOS_STATUS_SUCCESS;
}

/**============================================================================
  @brief hdd_softap_flush_tx_queues() - Utility function to flush the TX queues

  @param pAdapter : [in] pointer to adapter context  
  @return         : VOS_STATUS_E_FAILURE if any errors encountered 
                  : VOS_STATUS_SUCCESS otherwise
  ===========================================================================*/
static VOS_STATUS hdd_softap_flush_tx_queues( hdd_adapter_t *pAdapter )
{
   VOS_STATUS status = VOS_STATUS_SUCCESS;
   v_SINT_t i = -1;
   v_U8_t STAId = 0;
   hdd_list_node_t *anchor = NULL;
   skb_list_node_t *pktNode = NULL;
   struct sk_buff *skb = NULL;

   v_CONTEXT_t pVosContext = ( WLAN_HDD_GET_CTX(pAdapter))->pvosContext;
   ptSapContext pSapCtx = NULL;
   pSapCtx = VOS_GET_SAP_CB(pVosContext);
   if(pSapCtx == NULL){
       VOS_TRACE(VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_ERROR,
            FL("psapCtx is NULL"));
       return VOS_STATUS_E_FAULT;
   }
   spin_lock_bh( &pSapCtx->staInfo_lock );
   for (STAId = 0; STAId < WLAN_MAX_STA_COUNT; STAId++)
   {
      if (FALSE == pSapCtx->aStaInfo[STAId].isUsed)
      {
         continue;
      }

      for (i = 0; i < NUM_TX_QUEUES; i ++)
      {
         spin_lock_bh(&pSapCtx->aStaInfo[STAId].wmm_tx_queue[i].lock);
         while (true) 
         {
            status = hdd_list_remove_front ( &pSapCtx->aStaInfo[STAId].wmm_tx_queue[i], &anchor);

            if (VOS_STATUS_E_EMPTY != status)
            {
               //If success then we got a valid packet from some AC
               pktNode = list_entry(anchor, skb_list_node_t, anchor);
               skb = pktNode->skb;
               ++pAdapter->stats.tx_dropped;
               ++pAdapter->hdd_stats.hddTxRxStats.txFlushed;
               ++pAdapter->hdd_stats.hddTxRxStats.txFlushedAC[i];
               kfree_skb(skb);
               continue;
            }

            //current list is empty
            break;
         }
         pSapCtx->aStaInfo[STAId].txSuspended[i] = VOS_FALSE;
         spin_unlock_bh(&pSapCtx->aStaInfo[STAId].wmm_tx_queue[i].lock);
      }
      pSapCtx->aStaInfo[STAId].vosLowResource = VOS_FALSE;
   }

   spin_unlock_bh( &pSapCtx->staInfo_lock );

   return status;
}

/**
 * hdd_softap_get_connected_sta() -  provide number of connected STA
 * @pHostapdAdapter: pAdapter for SAP
 *
 * This function is invoked for SAP mode to get connected STA.
 *
 * Return:  Total number of connected STA to SAP.
 */
v_U8_t hdd_softap_get_connected_sta(hdd_adapter_t *pHostapdAdapter)
{
    v_U8_t i, sta_ct = 0;
    v_CONTEXT_t pVosContext = NULL;
    ptSapContext pSapCtx = NULL;

    pVosContext = (WLAN_HDD_GET_CTX(pHostapdAdapter))->pvosContext;
    pSapCtx = VOS_GET_SAP_CB(pVosContext);
    if (pSapCtx == NULL) {
        VOS_TRACE(VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_INFO,
                 FL("psapCtx is NULL"));
        goto error;
    }

    spin_lock_bh(&pSapCtx->staInfo_lock);
    // get stations associated with SAP
    for (i = 0; i < WLAN_MAX_STA_COUNT; i++) {
        if (pSapCtx->aStaInfo[i].isUsed &&
                  (!vos_is_macaddr_broadcast(&pSapCtx->aStaInfo[i].macAddrSTA)))
               sta_ct++;
    }
    spin_unlock_bh( &pSapCtx->staInfo_lock );

error:
    return sta_ct;
}

/**============================================================================
  @brief __hdd_softap_hard_start_xmit() - Function registered with the Linux OS
  for transmitting packets. There are 2 versions of this function. One that
  uses locked queue and other that uses lockless queues. Both have been
  retained to do some performance testing

  @param skb      : [in]  pointer to OS packet (sk_buff)
  @param dev      : [in] pointer to Libra network device
  
  @return         : NET_XMIT_DROP if packets are dropped
                  : NET_XMIT_SUCCESS if packet is enqueued succesfully
  ===========================================================================*/
int __hdd_softap_hard_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
   VOS_STATUS status;
   WLANTL_ACEnumType ac = WLANTL_AC_BE;
   sme_QosWmmUpType up = SME_QOS_WMM_UP_BE;
   skb_list_node_t *pktNode = NULL;
   v_SIZE_t pktListSize = 0;
   v_BOOL_t txSuspended = VOS_FALSE;
   hdd_adapter_t *pAdapter = (hdd_adapter_t *)netdev_priv(dev);
   hdd_ap_ctx_t *pHddApCtx = WLAN_HDD_GET_AP_CTX_PTR(pAdapter);   
   hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
   vos_list_node_t *anchor = NULL;
   v_U8_t STAId = WLAN_MAX_STA_COUNT;
   //Extract the destination address from ethernet frame
   v_MACADDR_t *pDestMacAddress = (v_MACADDR_t*)skb->data;
   int os_status = NETDEV_TX_OK; 
   struct sk_buff *skb1;

   v_CONTEXT_t pVosContext = ( WLAN_HDD_GET_CTX(pAdapter))->pvosContext;
   ptSapContext pSapCtx = NULL;
   pSapCtx = VOS_GET_SAP_CB(pVosContext);
   if(pSapCtx == NULL){
       VOS_TRACE(VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_ERROR,
             FL("psapCtx is NULL"));
       ++pAdapter->stats.tx_dropped;
       ++pAdapter->hdd_stats.hddTxRxStats.txXmitDropped;
       kfree_skb(skb);
       return os_status;
   }

   if (pHddCtx == NULL)
   {
      VOS_TRACE(VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_ERROR,
            FL("pHddCtx is NULL"));
      goto xmit_done;
   }

   pDestMacAddress = (v_MACADDR_t*)skb->data;
   
   ++pAdapter->hdd_stats.hddTxRxStats.txXmitCalled;

   VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_INFO,
              "%s: enter", __func__);

   spin_lock_bh( &pSapCtx->staInfo_lock );
   if (vos_is_macaddr_broadcast( pDestMacAddress ) || vos_is_macaddr_group(pDestMacAddress))
   {
      //The BC/MC station ID is assigned during BSS starting phase. SAP will return the station 
      //ID used for BC/MC traffic. The station id is registered to TL as well.
      STAId = pHddApCtx->uBCStaId;
      
      /* Setting priority for broadcast packets which doesn't go to select_queue function */
      skb->priority = SME_QOS_WMM_UP_BE;
      skb->queue_mapping = HDD_LINUX_AC_BE;

      VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_INFO_LOW,
              "%s: BC/MC packet", __func__);
   }
   else
   {
      STAId = hdd_sta_id_find_from_mac_addr(pAdapter, pDestMacAddress);
      if (STAId == HDD_WLAN_INVALID_STA_ID || STAId >= WLAN_MAX_STA_COUNT)
      {
         VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_WARN,
                    "%s: Failed to find right station", __func__);
         ++pAdapter->stats.tx_dropped;
         ++pAdapter->hdd_stats.hddTxRxStats.txXmitDropped;
         kfree_skb(skb);
         goto xmit_done;
      }
      else if (FALSE == pSapCtx->aStaInfo[STAId].isUsed )
      {
         VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_WARN,
                    "%s: STA %d is unregistered", __func__, STAId);
         ++pAdapter->stats.tx_dropped;
         ++pAdapter->hdd_stats.hddTxRxStats.txXmitDropped;
         kfree_skb(skb);
         goto xmit_done;
      }

      if ( (WLANTL_STA_CONNECTED != pSapCtx->aStaInfo[STAId].tlSTAState) &&
         (WLANTL_STA_AUTHENTICATED != pSapCtx->aStaInfo[STAId].tlSTAState) )
      {
         VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_WARN,
                    "%s: Station not connected yet", __func__);
         ++pAdapter->stats.tx_dropped;
         ++pAdapter->hdd_stats.hddTxRxStats.txXmitDropped;
         kfree_skb(skb);
         goto xmit_done;
      }
      else if(WLANTL_STA_CONNECTED == pSapCtx->aStaInfo[STAId].tlSTAState)
      {
        if(ntohs(skb->protocol) != HDD_ETHERTYPE_802_1_X)
        {
            VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_WARN,
                       "%s: NON-EAPOL packet in non-Authenticated state", __func__);
            ++pAdapter->stats.tx_dropped;
            ++pAdapter->hdd_stats.hddTxRxStats.txXmitDropped;
            kfree_skb(skb);
            goto xmit_done;
        }
      }
   }

   //Get TL AC corresponding to Qdisc queue index/AC.
   ac = hdd_QdiscAcToTlAC[skb->queue_mapping];
   //user priority from IP header, which is already extracted and set from 
   //select_queue call back function
   up = skb->priority;
   ++pAdapter->hdd_stats.hddTxRxStats.txXmitClassifiedAC[ac];

   VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_INFO,
              "%s: Classified as ac %d up %d", __func__, ac, up);

   if (( NULL != pHddCtx ) &&
         (pHddCtx->cfg_ini->gEnableDebugLog & VOS_PKT_PROTO_TYPE_DHCP))
   {
       hdd_dump_dhcp_pkt(skb, TX_PATH);
   }

   // If the memory differentiation mode is enabled, the memory limit of each queue will be 
   // checked. Over-limit packets will be dropped.
    spin_lock_bh(&pSapCtx->aStaInfo[STAId].wmm_tx_queue[ac].lock);
    hdd_list_size(&pSapCtx->aStaInfo[STAId].wmm_tx_queue[ac], &pktListSize);
    if(pktListSize >= pAdapter->aTxQueueLimit[ac])
    {
       VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_WARN,
            "%s: station %d ac %d queue over limit %d", __func__, STAId, ac, pktListSize);
       ++pAdapter->hdd_stats.hddTxRxStats.txXmitBackPressured;
       ++pAdapter->hdd_stats.hddTxRxStats.txXmitBackPressuredAC[ac];
       pSapCtx->aStaInfo[STAId].txSuspended[ac] = VOS_TRUE;
       netif_stop_subqueue(dev, skb_get_queue_mapping(skb));
       txSuspended = VOS_TRUE;
       MTRACE(vos_trace(VOS_MODULE_ID_HDD, TRACE_CODE_HDD_STOP_NETDEV,
                        pAdapter->sessionId, ac));
    }

    /* If 3/4th of the max queue size is used then enable the flag.
     * This flag indicates to place the DHCP packets in VOICE AC queue.*/
   if (WLANTL_AC_BE == ac)
   {
      if (pSapCtx->aStaInfo[STAId].wmm_tx_queue[ac].count >= HDD_TX_QUEUE_LOW_WATER_MARK)
      {
          if (!(pSapCtx->aStaInfo[STAId].wmm_tx_queue[ac].ratelimit_count % 0x40))
              VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_WARN,
                     "%s: TX queue for Best Effort AC is 3/4th full", __func__);
          pSapCtx->aStaInfo[STAId].wmm_tx_queue[ac].ratelimit_count++;
          pSapCtx->aStaInfo[STAId].vosLowResource = VOS_TRUE;
      }
      else
      {
          pSapCtx->aStaInfo[STAId].vosLowResource = VOS_FALSE;
      }
   }
   spin_unlock_bh(&pSapCtx->aStaInfo[STAId].wmm_tx_queue[ac].lock);

   if (VOS_TRUE == txSuspended)
   {
       VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_WARN, 
                  "%s: TX queue full for AC=%d Disable OS TX queue", 
                  __func__, ac );
      os_status = NETDEV_TX_BUSY;   
      goto xmit_done;
   }

   //Use the skb->cb field to hold the list node information
   pktNode = (skb_list_node_t *)&skb->cb;

   //Stick the OS packet inside this node.
   pktNode->skb = skb;

   //Stick the User Priority inside this node 
   pktNode->userPriority = up;

   INIT_LIST_HEAD(&pktNode->anchor);

   spin_lock_bh(&pSapCtx->aStaInfo[STAId].wmm_tx_queue[ac].lock);
   status = hdd_list_insert_back_size(&pSapCtx->aStaInfo[STAId].wmm_tx_queue[ac], &pktNode->anchor, &pktListSize );
   spin_unlock_bh(&pSapCtx->aStaInfo[STAId].wmm_tx_queue[ac].lock);
   if ( !VOS_IS_STATUS_SUCCESS( status ) )
   {
      VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_WARN,
                 "%s:Insert Tx queue failed. Pkt dropped", __func__);
      ++pAdapter->hdd_stats.hddTxRxStats.txXmitDropped;
      ++pAdapter->hdd_stats.hddTxRxStats.txXmitDroppedAC[ac];
      ++pAdapter->stats.tx_dropped;
      kfree_skb(skb);
      goto xmit_done;
   }

   ++pAdapter->hdd_stats.hddTxRxStats.txXmitQueued;
   ++pAdapter->hdd_stats.hddTxRxStats.txXmitQueuedAC[ac];
   ++pAdapter->hdd_stats.hddTxRxStats.pkt_tx_count;

   if (1 == pktListSize)
   {
      //Let TL know we have a packet to send for this AC
      status = WLANTL_STAPktPending( (WLAN_HDD_GET_CTX(pAdapter))->pvosContext, STAId, ac );      

      if ( !VOS_IS_STATUS_SUCCESS( status ) )
      {
         VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_WARN,
                    "%s: Failed to signal TL for AC=%d STAId =%d",
                      __func__, ac, STAId );

         //Remove the packet from queue. It must be at the back of the queue, as TX thread cannot preempt us in the middle
         //as we are in a soft irq context. Also it must be the same packet that we just allocated.
         spin_lock_bh(&pSapCtx->aStaInfo[STAId].wmm_tx_queue[ac].lock);
         status = hdd_list_remove_back( &pSapCtx->aStaInfo[STAId].wmm_tx_queue[ac], &anchor);
         spin_unlock_bh(&pSapCtx->aStaInfo[STAId].wmm_tx_queue[ac].lock);
         /* Free the skb only if we are able to remove it from the list.
          * If we are not able to retrieve it from the list it means that
          * the skb was pulled by TX Thread and is use so we should not free
          * it here
          */
         if (VOS_IS_STATUS_SUCCESS(status))
         {
            pktNode = list_entry(anchor, skb_list_node_t, anchor);
            skb1 = pktNode->skb;
            kfree_skb(skb1);
         }
         ++pAdapter->stats.tx_dropped;
         ++pAdapter->hdd_stats.hddTxRxStats.txXmitDropped;
         ++pAdapter->hdd_stats.hddTxRxStats.txXmitDroppedAC[ac];
         goto xmit_done;
      }
   }
   dev->trans_start = jiffies;

   VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_INFO_LOW,
              "%s: exit", __func__);

xmit_done:
   spin_unlock_bh( &pSapCtx->staInfo_lock );
   return os_status;
}

int hdd_softap_hard_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	int ret;
	vos_ssr_protect(__func__);
	ret = __hdd_softap_hard_start_xmit(skb, dev);
	vos_ssr_unprotect(__func__);
	return ret;
}

/**============================================================================
  @brief hdd_softap_sta_2_sta_xmit This function for Transmitting the frames when the traffic is between two stations.

  @param skb      : [in] pointer to packet (sk_buff)
  @param dev      : [in] pointer to Libra network device
  @param STAId    : [in] Station Id of Destination Station
  @param up       : [in] User Priority 
  
  @return         : NET_XMIT_DROP if packets are dropped
                  : NET_XMIT_SUCCESS if packet is enqueued succesfully
  ===========================================================================*/
VOS_STATUS hdd_softap_sta_2_sta_xmit(struct sk_buff *skb, 
                                      struct net_device *dev,
                                      v_U8_t STAId, 
                                      v_U8_t up)
{
   VOS_STATUS status = VOS_STATUS_SUCCESS; 
   skb_list_node_t *pktNode = NULL;
   v_SIZE_t pktListSize = 0;
   hdd_adapter_t *pAdapter = (hdd_adapter_t *)netdev_priv(dev);
   v_U8_t ac;
   vos_list_node_t *anchor = NULL;
   struct sk_buff *skb1;

   v_CONTEXT_t pVosContext = ( WLAN_HDD_GET_CTX(pAdapter))->pvosContext;
   ptSapContext pSapCtx = NULL;
   pSapCtx = VOS_GET_SAP_CB(pVosContext);
   if(pSapCtx == NULL){
       VOS_TRACE(VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_ERROR,
             FL("psapCtx is NULL"));
       kfree_skb(skb);
       status = VOS_STATUS_E_FAILURE;
       return status;
   }
   ++pAdapter->hdd_stats.hddTxRxStats.txXmitCalled;

   VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_INFO,
              "%s: enter", __func__);

   spin_lock_bh( &pSapCtx->staInfo_lock );
   if ( FALSE == pSapCtx->aStaInfo[STAId].isUsed )
   {
      VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_WARN,
                 "%s: STA %d is unregistered", __func__, STAId );
      kfree_skb(skb);
      status = VOS_STATUS_E_FAILURE;
      goto xmit_end;
   }

   /* If the QoS is not enabled on the receiving station, then send it with BE priority */
   if ( !pSapCtx->aStaInfo[STAId].isQosEnabled )
       up = SME_QOS_WMM_UP_BE;

   ac = hddWmmUpToAcMap[up];
   ++pAdapter->hdd_stats.hddTxRxStats.txXmitClassifiedAC[ac];
   VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_INFO,
              "%s: Classified as ac %d up %d", __func__, ac, up);

   skb->queue_mapping = hddLinuxUpToAcMap[up];

   //Use the skb->cb field to hold the list node information
   pktNode = (skb_list_node_t *)&skb->cb;

   //Stick the OS packet inside this node.
   pktNode->skb = skb;

   //Stick the User Priority inside this node 
   pktNode->userPriority = up;

   INIT_LIST_HEAD(&pktNode->anchor);

   spin_lock_bh(&pSapCtx->aStaInfo[STAId].wmm_tx_queue[ac].lock);
   hdd_list_size(&pSapCtx->aStaInfo[STAId].wmm_tx_queue[ac], &pktListSize);
   if(pSapCtx->aStaInfo[STAId].txSuspended[ac] ||
       pktListSize >= pAdapter->aTxQueueLimit[ac])
   {
       VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_WARN,
            "%s: station %d ac %d queue over limit %d", __func__, STAId, ac, pktListSize);
       /* TODO:Rx Flowchart should be trigerred here to SUPEND SSC on RX side.
        * SUSPEND should be done based on Threshold. RESUME would be 
        * triggered in fetch cbk after recovery.
        */
       kfree_skb(skb);
       spin_unlock_bh(&pSapCtx->aStaInfo[STAId].wmm_tx_queue[ac].lock);
       status = VOS_STATUS_E_FAILURE;
       goto xmit_end;
   }
   status = hdd_list_insert_back_size(&pSapCtx->aStaInfo[STAId].wmm_tx_queue[ac], &pktNode->anchor, &pktListSize );
   spin_unlock_bh(&pSapCtx->aStaInfo[STAId].wmm_tx_queue[ac].lock);

   if ( !VOS_IS_STATUS_SUCCESS( status ) )
   {
      VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_WARN,
                 "%s:Insert Tx queue failed. Pkt dropped", __func__);
      ++pAdapter->hdd_stats.hddTxRxStats.txXmitDropped;
      ++pAdapter->hdd_stats.hddTxRxStats.txXmitDroppedAC[ac];
      ++pAdapter->stats.tx_dropped;
      kfree_skb(skb);
      status = VOS_STATUS_E_FAILURE;
      goto xmit_end;
   }

   ++pAdapter->hdd_stats.hddTxRxStats.txXmitQueued;
   ++pAdapter->hdd_stats.hddTxRxStats.txXmitQueuedAC[ac];

   if (1 == pktListSize)
   {
      //Let TL know we have a packet to send for this AC
      //VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_ERROR,"%s:Indicating Packet to TL", __func__);
      status = WLANTL_STAPktPending( (WLAN_HDD_GET_CTX(pAdapter))->pvosContext, STAId, ac );

      if ( !VOS_IS_STATUS_SUCCESS( status ) )
      {
         VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_WARN,
                    "%s: Failed to signal TL for AC=%d STAId =%d",
                    __func__, ac, STAId );

         //Remove the packet from queue. It must be at the back of the queue, as TX thread cannot preempt us in the middle
         //as we are in a soft irq context. Also it must be the same packet that we just allocated.
         spin_lock_bh(&pSapCtx->aStaInfo[STAId].wmm_tx_queue[ac].lock);
         status = hdd_list_remove_back( &pSapCtx->aStaInfo[STAId].wmm_tx_queue[ac], &anchor);
         spin_unlock_bh(&pSapCtx->aStaInfo[STAId].wmm_tx_queue[ac].lock);
         /* Free the skb only if we are able to remove it from the list.
          * If we are not able to retrieve it from the list it means that
          * the skb was pulled by TX Thread and is use so we should not free
          * it here
          */
         if (VOS_IS_STATUS_SUCCESS(status))
         {
            pktNode = list_entry(anchor, skb_list_node_t, anchor);
            skb1 = pktNode->skb;
            kfree_skb(skb1);
         }
         ++pAdapter->stats.tx_dropped;
         ++pAdapter->hdd_stats.hddTxRxStats.txXmitDropped;
         ++pAdapter->hdd_stats.hddTxRxStats.txXmitDroppedAC[ac];
         status = VOS_STATUS_E_FAILURE;
         goto xmit_end;
      }
   }

   VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_INFO_LOW, "%s: exit", __func__);

xmit_end:
   spin_unlock_bh( &pSapCtx->staInfo_lock );
   return status;
}

/**============================================================================
  @brief __hdd_softap_tx_timeout() - Function called by OS if there is any
  timeout during transmission. Since HDD simply enqueues packet
  and returns control to OS right away, this would never be invoked

  @param dev : [in] pointer to Libra network device
  @return    : None
  ===========================================================================*/
void __hdd_softap_tx_timeout(struct net_device *dev)
{
   hdd_adapter_t *pAdapter =  WLAN_HDD_GET_PRIV_PTR(dev);
   struct netdev_queue *txq;
   int i = 0;
   int status = 0;
   hdd_context_t *pHddCtx;

   VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_ERROR,
      "%s: Transmission timeout occurred", __func__);

   if ( NULL == pAdapter )
   {
      VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_ERROR,
              FL("pAdapter is NULL"));
      VOS_ASSERT(0);
      return;
   }

   pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
   status = wlan_hdd_validate_context(pHddCtx);
   if (status != 0)
   {
       return;
   }

   ++pAdapter->hdd_stats.hddTxRxStats.txTimeoutCount;

   for (i = 0; i < NUM_TX_QUEUES; i++)
   {
      txq = netdev_get_tx_queue(dev, i);
      VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_ERROR,
                "Queue%d status: %d", i, netif_tx_queue_stopped(txq));
   }

   VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_ERROR,
             "carrier state: %d", netif_carrier_ok(dev));

   ++pAdapter->hdd_stats.hddTxRxStats.continuousTxTimeoutCount;

   if (pAdapter->hdd_stats.hddTxRxStats.continuousTxTimeoutCount ==
          HDD_SAP_TX_STALL_RECOVERY_THRESHOLD)
   {
      VOS_TRACE(VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_ERROR,
                "%s: Request firmware for recovery",__func__);
      WLANTL_TLDebugMessage(WLANTL_DEBUG_FW_CLEANUP);
   }
   if (pAdapter->hdd_stats.hddTxRxStats.continuousTxTimeoutCount >
          HDD_SAP_TX_STALL_SSR_THRESHOLD)
   {
      // Driver could not recover, issue SSR
      VOS_TRACE(VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_ERROR,
                "%s: Cannot recover from Data stall Issue SSR",
                __func__);
      WLANTL_FatalError();
      // reset count after issuing the SSR
      pAdapter->hdd_stats.hddTxRxStats.continuousTxTimeoutCount = 0;
      return;
   }

   /* If Tx stalled for a long time then *hdd_tx_timeout* is called
    * every 5sec. The TL debug spits out a lot of information on the
    * serial console, if it is called every time *hdd_tx_timeout* is
    * called then we may get a watchdog bite on the Application
    * processor, so ratelimit the TL debug logs.
    */
   if (__ratelimit(&hdd_softap_tx_timeout_rs))
   {
      hdd_wmm_tx_snapshot(pAdapter);
      WLANTL_TLDebugMessage(WLANTL_DEBUG_TX_SNAPSHOT);
   }
   /* Call fatal event if data stall is for
    * HDD_TX_STALL_FATAL_EVENT_THRESHOLD times
    */
   if (HDD_SAP_TX_STALL_FATAL_EVENT_THRESHOLD ==
       pAdapter->hdd_stats.hddTxRxStats.continuousTxTimeoutCount)
      vos_fatal_event_logs_req(WLAN_LOG_TYPE_FATAL,
                   WLAN_LOG_INDICATOR_HOST_DRIVER,
                   WLAN_LOG_REASON_DATA_STALL,
                   FALSE, TRUE);
} 

void hdd_softap_tx_timeout(struct net_device *dev)
{
   vos_ssr_protect(__func__);
   __hdd_softap_tx_timeout(dev);
   vos_ssr_unprotect(__func__);
   return;
}

/**============================================================================
  @brief __hdd_softap_stats() - Function registered with the Linux OS for
  device TX/RX statistic

  @param dev      : [in] pointer to Libra network device
  
  @return         : pointer to net_device_stats structure
  ===========================================================================*/
struct net_device_stats* __hdd_softap_stats(struct net_device *dev)
{
   hdd_adapter_t* priv = netdev_priv(dev);
   return &priv->stats;
}

struct net_device_stats* hdd_softap_stats(struct net_device *dev)
{
   struct net_device_stats *priv_stats;
   vos_ssr_protect(__func__);
   priv_stats = __hdd_softap_stats(dev);
   vos_ssr_unprotect(__func__);

   return priv_stats;
}

/**============================================================================
  @brief hdd_softap_init_tx_rx() - Init function to initialize Tx/RX
  modules in HDD

  @param pAdapter : [in] pointer to adapter context  
  @return         : VOS_STATUS_E_FAILURE if any errors encountered 
                  : VOS_STATUS_SUCCESS otherwise
  ===========================================================================*/
VOS_STATUS hdd_softap_init_tx_rx(hdd_adapter_t *pAdapter, bool re_init)
{
   VOS_STATUS status = VOS_STATUS_SUCCESS;
   v_SINT_t i = -1;
   v_SIZE_t size = 0;

   v_U8_t STAId = 0;
   v_CONTEXT_t pVosContext = ( WLAN_HDD_GET_CTX(pAdapter))->pvosContext;
   ptSapContext pSapCtx = NULL;

   v_U8_t pACWeights[] = {
                           HDD_SOFTAP_BK_WEIGHT_DEFAULT, 
                           HDD_SOFTAP_BE_WEIGHT_DEFAULT, 
                           HDD_SOFTAP_VI_WEIGHT_DEFAULT, 
                           HDD_SOFTAP_VO_WEIGHT_DEFAULT
                         };
   pSapCtx = VOS_GET_SAP_CB(pVosContext);
   if(pSapCtx == NULL){
       VOS_TRACE(VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_ERROR,
             FL("psapCtx is NULL"));
       return VOS_STATUS_E_FAULT;
   }


   pAdapter->isVosOutOfResource = VOS_FALSE;
   pAdapter->isVosLowResource = VOS_FALSE;

   vos_mem_zero(&pAdapter->stats, sizeof(struct net_device_stats));

   while (++i != NUM_TX_QUEUES) 
      hdd_list_init( &pAdapter->wmm_tx_queue[i], HDD_TX_QUEUE_MAX_LEN);

   /* Initial HDD buffer control / flow control fields*/
   vos_pkt_get_available_buffer_pool (VOS_PKT_TYPE_TX_802_3_DATA, &size);

   pAdapter->aTxQueueLimit[WLANTL_AC_BK] = HDD_SOFTAP_TX_BK_QUEUE_MAX_LEN;
   pAdapter->aTxQueueLimit[WLANTL_AC_BE] = HDD_SOFTAP_TX_BE_QUEUE_MAX_LEN;
   pAdapter->aTxQueueLimit[WLANTL_AC_VI] = HDD_SOFTAP_TX_VI_QUEUE_MAX_LEN;
   pAdapter->aTxQueueLimit[WLANTL_AC_VO] = HDD_SOFTAP_TX_VO_QUEUE_MAX_LEN;

   for (STAId = 0; STAId < WLAN_MAX_STA_COUNT; STAId++)
   {
      vos_mem_zero(&pSapCtx->aStaInfo[STAId], sizeof(hdd_station_info_t));
      for (i = 0; i < NUM_TX_QUEUES; i ++)
      {
         hdd_list_init(&pSapCtx->aStaInfo[STAId].wmm_tx_queue[i], HDD_TX_QUEUE_MAX_LEN);
      }
   }

   /* Update the AC weights suitable for SoftAP mode of operation */
   WLANTL_SetACWeights((WLAN_HDD_GET_CTX(pAdapter))->pvosContext, pACWeights);

   if (VOS_STATUS_SUCCESS != hdd_start_trafficMonitor(pAdapter, re_init))
   {
       VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_ERROR,
          "%s: failed to start Traffic Monito timer ", __func__ );
       return VOS_STATUS_E_INVAL;
   }
   return status;
}

/**============================================================================
  @brief hdd_softap_deinit_tx_rx() - Deinit function to clean up Tx/RX
  modules in HDD

  @param pAdapter : [in] pointer to adapter context  
  @return         : VOS_STATUS_E_FAILURE if any errors encountered 
                  : VOS_STATUS_SUCCESS otherwise
  ===========================================================================*/
VOS_STATUS hdd_softap_deinit_tx_rx( hdd_adapter_t *pAdapter, bool re_init)
{
   VOS_STATUS status = VOS_STATUS_SUCCESS;

   if (VOS_STATUS_SUCCESS != hdd_stop_trafficMonitor(pAdapter, re_init))
   {
       VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_ERROR,
                 "%s: Fail to Stop Traffic Monito timer", __func__ );
       return VOS_STATUS_E_INVAL;
   }

   status = hdd_softap_flush_tx_queues(pAdapter);

   return status;
}

/**============================================================================
  @brief hdd_softap_flush_tx_queues_sta() - Utility function to flush the TX queues of a station

  @param pAdapter : [in] pointer to adapter context
  @param STAId    : [in] Station ID to deinit 
  @return         : VOS_STATUS_E_FAILURE if any errors encountered 
                  : VOS_STATUS_SUCCESS otherwise
  ===========================================================================*/
static VOS_STATUS hdd_softap_flush_tx_queues_sta( hdd_adapter_t *pAdapter, v_U8_t STAId )
{
   v_U8_t i = -1;

   hdd_list_node_t *anchor = NULL;

   skb_list_node_t *pktNode = NULL;
   struct sk_buff *skb = NULL;

   v_CONTEXT_t pVosContext = ( WLAN_HDD_GET_CTX(pAdapter))->pvosContext;
   ptSapContext pSapCtx = NULL;
   pSapCtx = VOS_GET_SAP_CB(pVosContext);
   if(pSapCtx == NULL){
       VOS_TRACE(VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_ERROR,
            FL("psapCtx is NULL"));
       return VOS_STATUS_E_FAULT;
   }
   if (FALSE == pSapCtx->aStaInfo[STAId].isUsed)
   {
      return VOS_STATUS_SUCCESS;
   }

   for (i = 0; i < NUM_TX_QUEUES; i ++)
   {
      spin_lock_bh(&pSapCtx->aStaInfo[STAId].wmm_tx_queue[i].lock);
      while (true) 
      {
         if (VOS_STATUS_E_EMPTY !=
              hdd_list_remove_front(&pSapCtx->aStaInfo[STAId].wmm_tx_queue[i],
                                    &anchor))
         {
            //If success then we got a valid packet from some AC
            pktNode = list_entry(anchor, skb_list_node_t, anchor);
            skb = pktNode->skb;
            ++pAdapter->stats.tx_dropped;
            ++pAdapter->hdd_stats.hddTxRxStats.txFlushed;
            ++pAdapter->hdd_stats.hddTxRxStats.txFlushedAC[i];
            kfree_skb(skb);
            continue;
         }

         //current list is empty
         break;
      }
      spin_unlock_bh(&pSapCtx->aStaInfo[STAId].wmm_tx_queue[i].lock);
   }

   return VOS_STATUS_SUCCESS;
}

/**============================================================================
  @brief hdd_softap_init_tx_rx_sta() - Init function to initialize a station in Tx/RX
  modules in HDD

  @param pAdapter : [in] pointer to adapter context
  @param STAId    : [in] Station ID to deinit
  @param pmacAddrSTA  : [in] pointer to the MAC address of the station  
  @return         : VOS_STATUS_E_FAILURE if any errors encountered 
                  : VOS_STATUS_SUCCESS otherwise
  ===========================================================================*/
VOS_STATUS hdd_softap_init_tx_rx_sta( hdd_adapter_t *pAdapter, v_U8_t STAId, v_MACADDR_t *pmacAddrSTA)
{
   v_U8_t i = 0;
   VOS_STATUS status;
   v_CONTEXT_t pVosContext = ( WLAN_HDD_GET_CTX(pAdapter))->pvosContext;
   ptSapContext pSapCtx = NULL;

   pSapCtx = VOS_GET_SAP_CB(pVosContext);
   if(pSapCtx == NULL){
       VOS_TRACE(VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_ERROR,
            FL("psapCtx is NULL"));
       return VOS_STATUS_E_FAULT;
   }

   spin_lock_bh( &pSapCtx->staInfo_lock );
   if (pSapCtx->aStaInfo[STAId].isUsed)
   {
      VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_ERROR,
                 "%s: Reinit station %d", __func__, STAId );
      spin_unlock_bh( &pSapCtx->staInfo_lock );
      return VOS_STATUS_E_FAILURE;
   }

   vos_mem_zero(&pSapCtx->aStaInfo[STAId], sizeof(hdd_station_info_t));
   for (i = 0; i < NUM_TX_QUEUES; i ++)
   {
      hdd_list_init(&pSapCtx->aStaInfo[STAId].wmm_tx_queue[i], HDD_TX_QUEUE_MAX_LEN);
   }

   pSapCtx->aStaInfo[STAId].isUsed = TRUE;
   pSapCtx->aStaInfo[STAId].isDeauthInProgress = FALSE;
   vos_copy_macaddr( &pSapCtx->aStaInfo[STAId].macAddrSTA, pmacAddrSTA);

   spin_unlock_bh( &pSapCtx->staInfo_lock );

   status = hdd_sta_id_hash_add_entry(pAdapter, STAId, pmacAddrSTA);
   if (status != VOS_STATUS_SUCCESS) {
       VOS_TRACE(VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_ERROR,
                 FL("Not able to add staid hash %d"), STAId);
       return VOS_STATUS_E_FAILURE;
   }

   VOS_TRACE(VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_INFO,
             FL("New station added sta_id %d mac:"
             MAC_ADDRESS_STR), STAId,
             MAC_ADDR_ARRAY(pmacAddrSTA->bytes));

   return VOS_STATUS_SUCCESS;
}

/**============================================================================
  @brief hdd_softap_deinit_tx_rx_sta() - Deinit function to clean up a statioin in Tx/RX
  modules in HDD

  @param pAdapter : [in] pointer to adapter context
  @param STAId    : [in] Station ID to deinit 
  @return         : VOS_STATUS_E_FAILURE if any errors encountered 
                  : VOS_STATUS_SUCCESS otherwise
  ===========================================================================*/
VOS_STATUS hdd_softap_deinit_tx_rx_sta ( hdd_adapter_t *pAdapter, v_U8_t STAId )
{
   VOS_STATUS status = VOS_STATUS_SUCCESS;
   v_U8_t ac;
   /**Track whether OS TX queue has been disabled.*/
   v_BOOL_t txSuspended[NUM_TX_QUEUES];
   v_U8_t tlAC;
   hdd_hostapd_state_t *pHostapdState;
   v_U8_t i;

   v_CONTEXT_t pVosContext = ( WLAN_HDD_GET_CTX(pAdapter))->pvosContext;
   ptSapContext pSapCtx = NULL;
   pSapCtx = VOS_GET_SAP_CB(pVosContext);
   if(pSapCtx == NULL){
       VOS_TRACE(VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_ERROR,
             FL("psapCtx is NULL"));
       return VOS_STATUS_E_FAULT;
   }
   pHostapdState = WLAN_HDD_GET_HOSTAP_STATE_PTR(pAdapter);

   spin_lock_bh( &pSapCtx->staInfo_lock );
   if (FALSE == pSapCtx->aStaInfo[STAId].isUsed)
   {
      VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_ERROR,
                 "%s: Deinit station not inited %d", __func__, STAId );
      spin_unlock_bh( &pSapCtx->staInfo_lock );
      return VOS_STATUS_E_FAILURE;
   }

   status = hdd_softap_flush_tx_queues_sta(pAdapter, STAId);

   pSapCtx->aStaInfo[STAId].isUsed = FALSE;
   pSapCtx->aStaInfo[STAId].isDeauthInProgress = FALSE;

   status = hdd_sta_id_hash_remove_entry(pAdapter,
                STAId, &pSapCtx->aStaInfo[STAId].macAddrSTA);
   if (status != VOS_STATUS_SUCCESS) {
       spin_unlock_bh( &pSapCtx->staInfo_lock );
       VOS_TRACE(VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_ERROR,
                 FL("Not able to remove staid hash %d"), STAId);
       return VOS_STATUS_E_FAILURE;
   }

   VOS_TRACE(VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_INFO,
             FL("station removed sta_id %d mac:"
             MAC_ADDRESS_STR), STAId,
             MAC_ADDR_ARRAY(pSapCtx->aStaInfo[STAId].macAddrSTA.bytes));

   /* if this STA had any of its WMM TX queues suspended, then the
      associated queue on the network interface was disabled.  check
      to see if that is the case, in which case we need to re-enable
      the interface queue.  but we only do this if the BSS is running
      since, if the BSS is stopped, all of the interfaces have been
      stopped and should not be re-enabled */

   if (BSS_START == pHostapdState->bssState)
   {
      for (ac = HDD_LINUX_AC_VO; ac <= HDD_LINUX_AC_BK; ac++)
      {
         tlAC = hdd_QdiscAcToTlAC[ac];
         txSuspended[ac] = pSapCtx->aStaInfo[STAId].txSuspended[tlAC];
      }
   }
   vos_mem_zero(&pSapCtx->aStaInfo[STAId], sizeof(hdd_station_info_t));

   /* re-init spin lock, since netdev can still open adapter until
    * driver gets unloaded
    */
   for (i = 0; i < NUM_TX_QUEUES; i ++)
   {
      hdd_list_init(&pSapCtx->aStaInfo[STAId].wmm_tx_queue[i],
                    HDD_TX_QUEUE_MAX_LEN);
   }

   if (BSS_START == pHostapdState->bssState)
   {
      for (ac = HDD_LINUX_AC_VO; ac <= HDD_LINUX_AC_BK; ac++)
      {
         if (txSuspended[ac])
         {
            VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_INFO,
                       "%s: TX queue re-enabled", __func__);
            netif_wake_subqueue(pAdapter->dev, ac);
         }
      }
   }

   spin_unlock_bh( &pSapCtx->staInfo_lock );
   return status;
}

/**============================================================================
  @brief hdd_softap_disconnect_tx_rx() - Disconnect function to clean up Tx/RX
  modules in HDD

  @param pAdapter : [in] pointer to adapter context  
  @return         : VOS_STATUS_E_FAILURE if any errors encountered 
                  : VOS_STATUS_SUCCESS otherwise
  ===========================================================================*/
VOS_STATUS hdd_softap_disconnect_tx_rx( hdd_adapter_t *pAdapter )
{
   return hdd_softap_flush_tx_queues(pAdapter);
}

/**============================================================================
  @brief hdd_softap_tx_complete_cbk() - Callback function invoked by TL
  to indicate that a packet has been transmitted across the bus
  succesfully. OS packet resources can be released after this cbk.

  @param vosContext   : [in] pointer to VOS context   
  @param pVosPacket   : [in] pointer to VOS packet (containing skb) 
  @param vosStatusIn  : [in] status of the transmission 

  @return             : VOS_STATUS_E_FAILURE if any errors encountered 
                      : VOS_STATUS_SUCCESS otherwise
  ===========================================================================*/
VOS_STATUS hdd_softap_tx_complete_cbk( v_VOID_t *vosContext, 
                                vos_pkt_t *pVosPacket, 
                                VOS_STATUS vosStatusIn )
{
   VOS_STATUS status = VOS_STATUS_SUCCESS;
   hdd_adapter_t *pAdapter = NULL;   
   void* pOsPkt = NULL;
   
   if( ( NULL == vosContext ) || ( NULL == pVosPacket )  )
   {
      VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_ERROR,
                 "%s: Null params being passed", __func__);
      return VOS_STATUS_E_FAILURE; 
   }

   //Return the skb to the OS
   status = vos_pkt_get_os_packet( pVosPacket, &pOsPkt, VOS_TRUE );
   if ((!VOS_IS_STATUS_SUCCESS(status)) || (!pOsPkt))
   {
      //This is bad but still try to free the VOSS resources if we can
      VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_ERROR,
                 "%s: Failure extracting skb from vos pkt", __func__);
      vos_pkt_return_packet( pVosPacket );
      return VOS_STATUS_E_FAILURE;
   }

   //Get the Adapter context.
   pAdapter = (hdd_adapter_t *)netdev_priv(((struct sk_buff *)pOsPkt)->dev);
   if((pAdapter == NULL) || (WLAN_HDD_ADAPTER_MAGIC != pAdapter->magic))
   {
      VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_ERROR,
                 "%s: HDD adapter context is invalid", __func__);
   }
   else
   {
      ++pAdapter->hdd_stats.hddTxRxStats.txCompleted;
   }

   kfree_skb((struct sk_buff *)pOsPkt); 

   //Return the VOS packet resources.
   status = vos_pkt_return_packet( pVosPacket );
   if(!VOS_IS_STATUS_SUCCESS( status ))
   {
      VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_ERROR,
                 "%s: Could not return VOS packet to the pool", __func__);
   }

   return status;
}


/**============================================================================
  @brief hdd_softap_tx_fetch_packet_cbk() - Callback function invoked by TL to 
  fetch a packet for transmission.

  @param vosContext   : [in] pointer to VOS context  
  @param staId        : [in] Station for which TL is requesting a pkt
  @param ac           : [in] access category requested by TL
  @param pVosPacket   : [out] pointer to VOS packet packet pointer
  @param pPktMetaInfo : [out] pointer to meta info for the pkt 
  
  @return             : VOS_STATUS_E_EMPTY if no packets to transmit
                      : VOS_STATUS_E_FAILURE if any errors encountered 
                      : VOS_STATUS_SUCCESS otherwise
  ===========================================================================*/
VOS_STATUS hdd_softap_tx_fetch_packet_cbk( v_VOID_t *vosContext,
                                    v_U8_t *pStaId,
                                    WLANTL_ACEnumType  ac,
                                    vos_pkt_t **ppVosPacket,
                                    WLANTL_MetaInfoType *pPktMetaInfo )
{
   VOS_STATUS status = VOS_STATUS_E_FAILURE;
   hdd_adapter_t *pAdapter = NULL;
   hdd_list_node_t *anchor = NULL;
   skb_list_node_t *pktNode = NULL;
   struct sk_buff *skb = NULL;
   vos_pkt_t *pVosPacket = NULL;
   v_MACADDR_t* pDestMacAddress = NULL;
   v_TIME_t timestamp;
   v_SIZE_t size = 0;
   v_U8_t STAId = WLAN_MAX_STA_COUNT;   
   hdd_context_t *pHddCtx = NULL;
   v_U8_t proto_type = 0;
   v_CONTEXT_t pVosContext = NULL;
   ptSapContext pSapCtx = NULL;
   //Sanity check on inputs
   if ( ( NULL == vosContext ) || 
        ( NULL == pStaId ) || 
        ( NULL == ppVosPacket ) ||
        ( NULL == pPktMetaInfo ) )
   {
      VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_ERROR,
                 "%s: Null Params being passed", __func__);
      return VOS_STATUS_E_FAILURE;
   }
    
   //Get the HDD context.
   pHddCtx = (hdd_context_t *)vos_get_context( VOS_MODULE_ID_HDD, vosContext );
   if ( NULL == pHddCtx )
   {
      VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_ERROR,
                 "%s: HDD adapter context is Null", __func__);
      return VOS_STATUS_E_FAILURE;
   }

   STAId = *pStaId;
   if (STAId >= WLAN_MAX_STA_COUNT)
   {
      VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_ERROR,
                 "%s: Invalid STAId %d passed by TL", __func__, STAId);
      return VOS_STATUS_E_FAILURE;
   }

   pAdapter = pHddCtx->sta_to_adapter[STAId];
   if ((NULL == pAdapter) || (WLAN_HDD_ADAPTER_MAGIC != pAdapter->magic))
   {
      VOS_ASSERT(0);
      return VOS_STATUS_E_FAILURE;
   }
   pVosContext = ( WLAN_HDD_GET_CTX(pAdapter))->pvosContext;
   pSapCtx = VOS_GET_SAP_CB(pVosContext);
   if(pSapCtx == NULL){
       VOS_TRACE(VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_ERROR,
             FL("psapCtx is NULL"));
       return VOS_STATUS_E_FAULT;
   }

   if (FALSE == pSapCtx->aStaInfo[STAId].isUsed )
   {
      VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_ERROR,
                 "%s: Unregistered STAId %d passed by TL", __func__, STAId);
      return VOS_STATUS_E_FAILURE;
   }

   /* Monitor traffic */
   if ( pHddCtx->cfg_ini->enableTrafficMonitor )
   {
      pHddCtx->traffic_monitor.lastFrameTs = vos_timer_get_system_time();
      if ( !atomic_read(&pHddCtx->traffic_monitor.isActiveMode) )
      {
         vos_lock_acquire(&pHddCtx->traffic_monitor.trafficLock);
         /* It was IDLE mode,
          * this is new state, then switch mode from suspend to resume */
         if ( !atomic_read(&pHddCtx->traffic_monitor.isActiveMode) )
         {
            hdd_set_wlan_suspend_mode(0);
            vos_timer_start(&pHddCtx->traffic_monitor.trafficTimer,
                            pHddCtx->cfg_ini->trafficIdleTimeout);
            atomic_set(&pHddCtx->traffic_monitor.isActiveMode, 1);
         }
         vos_lock_release(&pHddCtx->traffic_monitor.trafficLock);
      }
   }

   ++pAdapter->hdd_stats.hddTxRxStats.txFetched;

   *ppVosPacket = NULL;

   //Make sure the AC being asked for is sane
   if( ac > WLANTL_MAX_AC || ac < 0)
   {
      VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_ERROR,
                 "%s: Invalid AC %d passed by TL", __func__, ac);
      return VOS_STATUS_E_FAILURE;
   }

   ++pAdapter->hdd_stats.hddTxRxStats.txFetchedAC[ac];

   VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_INFO,
              "%s: AC %d passed by TL", __func__, ac);

   //Get the vos packet. I don't want to dequeue and enqueue again if we are out of VOS resources 
   //This simplifies the locking and unlocking of Tx queue
   status = vos_pkt_wrap_data_packet( &pVosPacket, 
                                      VOS_PKT_TYPE_TX_802_3_DATA, 
                                      NULL, //OS Pkt is not being passed
                                      hdd_softap_tx_low_resource_cbk, 
                                      pAdapter );

   if (status == VOS_STATUS_E_ALREADY || status == VOS_STATUS_E_RESOURCES)
   {
      //Remember VOS is in a low resource situation
      pAdapter->isVosOutOfResource = VOS_TRUE;
      ++pAdapter->hdd_stats.hddTxRxStats.txFetchLowResources;
      VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_WARN,
                 "%s: VOSS in Low Resource scenario", __func__);
      //TL needs to handle this case. VOS_STATUS_E_EMPTY is returned when the queue is empty.
      return VOS_STATUS_E_FAILURE;
   }

   /* Only fetch this station and this AC. Return VOS_STATUS_E_EMPTY if nothing there. Do not get next AC
      as the other branch does.
   */
   spin_lock_bh( &pSapCtx->staInfo_lock );
   spin_lock_bh(&pSapCtx->aStaInfo[STAId].wmm_tx_queue[ac].lock);
   hdd_list_size(&pSapCtx->aStaInfo[STAId].wmm_tx_queue[ac], &size);

   if (0 == size)
   {
      spin_unlock_bh(&pSapCtx->aStaInfo[STAId].wmm_tx_queue[ac].lock);
      spin_unlock_bh( &pSapCtx->staInfo_lock );
      vos_pkt_return_packet(pVosPacket);
      return VOS_STATUS_E_EMPTY;
   }

   status = hdd_list_remove_front( &pSapCtx->aStaInfo[STAId].wmm_tx_queue[ac], &anchor );
   spin_unlock_bh(&pSapCtx->aStaInfo[STAId].wmm_tx_queue[ac].lock);
   spin_unlock_bh( &pSapCtx->staInfo_lock );

   VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_INFO,
                       "%s: AC %d has packets pending", __func__, ac);

   if(VOS_STATUS_SUCCESS == status)
   {
      //If success then we got a valid packet from some AC
      pktNode = list_entry(anchor, skb_list_node_t, anchor);
      skb = pktNode->skb;
   }
   else
   {
      ++pAdapter->hdd_stats.hddTxRxStats.txFetchDequeueError;
      VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_ERROR,
                 "%s: Error in de-queuing skb from Tx queue status = %d",
                 __func__, status );
      vos_pkt_return_packet(pVosPacket);
      return VOS_STATUS_E_FAILURE;
   }

   //Attach skb to VOS packet.
   status = vos_pkt_set_os_packet( pVosPacket, skb );
   if (status != VOS_STATUS_SUCCESS)
   {
      VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_ERROR,
                 "%s: Error attaching skb", __func__);
      vos_pkt_return_packet(pVosPacket);
      ++pAdapter->stats.tx_dropped;
      ++pAdapter->hdd_stats.hddTxRxStats.txFetchDequeueError;
      kfree_skb(skb);
      return VOS_STATUS_E_FAILURE;
   }

   //Just being paranoid. To be removed later
   if(pVosPacket == NULL)
   {
      VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_ERROR,
                 "%s: VOS packet returned by VOSS is NULL", __func__);
      ++pAdapter->stats.tx_dropped;
      ++pAdapter->hdd_stats.hddTxRxStats.txFetchDequeueError;
      kfree_skb(skb);
      return VOS_STATUS_E_FAILURE;
   }

   //Return VOS packet to TL;
   *ppVosPacket = pVosPacket;

   //Fill out the meta information needed by TL
   //FIXME This timestamp is really the time stamp of wrap_data_packet
   vos_pkt_get_timestamp( pVosPacket, &timestamp );
   pPktMetaInfo->usTimeStamp = (v_U16_t)timestamp;
   if ( 1 < size )
   {
       pPktMetaInfo->bMorePackets = 1; //HDD has more packets to send
   }
   else
   {
       pPktMetaInfo->bMorePackets = 0;
   }

   pPktMetaInfo->ucIsEapol = 0;

   if(pSapCtx->aStaInfo[STAId].tlSTAState != WLANTL_STA_AUTHENTICATED)
   {
      if (TRUE == hdd_IsEAPOLPacket( pVosPacket ))
      {
         pPktMetaInfo->ucIsEapol = 1;
         wlan_hdd_log_eapol(skb,
                            WIFI_EVENT_DRIVER_EAPOL_FRAME_TRANSMIT_REQUESTED);
      }
   }

   if ((NULL != pHddCtx) &&
       (pHddCtx->cfg_ini->gEnableDebugLog))
   {
      proto_type = vos_pkt_get_proto_type(skb,
                                          pHddCtx->cfg_ini->gEnableDebugLog);
      if (VOS_PKT_PROTO_TYPE_EAPOL & proto_type)
      {
         VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
                   "SAP TX EAPOL");
      }
      else if (VOS_PKT_PROTO_TYPE_DHCP & proto_type)
      {
         VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
                   "SAP TX DHCP");
      }
      else if (VOS_PKT_PROTO_TYPE_ARP & proto_type)
      {
         VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
                   "SAP TX ARP");
      }
   }
//xg: @@@@: temporarily disble these. will revisit later
   {
      pPktMetaInfo->ac = ac;
      pPktMetaInfo->ucUP = pktNode->userPriority;
      pPktMetaInfo->ucTID = pPktMetaInfo->ucUP;
   }

   pPktMetaInfo->ucType = 0;          //FIXME Don't know what this is
   //Extract the destination address from ethernet frame
   pDestMacAddress = (v_MACADDR_t*)skb->data;

   // we need 802.3 to 802.11 frame translation
   // (note that Bcast/Mcast will be translated in SW, unicast in HW)
   pPktMetaInfo->ucDisableFrmXtl = 0;
   pPktMetaInfo->ucBcast = vos_is_macaddr_broadcast( pDestMacAddress ) ? 1 : 0;
   pPktMetaInfo->ucMcast = vos_is_macaddr_group( pDestMacAddress ) ? 1 : 0;

   if ( (pSapCtx->aStaInfo[STAId].txSuspended[ac]) &&
        (size <= ((pAdapter->aTxQueueLimit[ac]*3)/4) ))
   {
      VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_INFO,
                 "%s: TX queue re-enabled", __func__);
      pSapCtx->aStaInfo[STAId].txSuspended[ac] = VOS_FALSE;
      netif_wake_subqueue(pAdapter->dev, skb_get_queue_mapping(skb));
      MTRACE(vos_trace(VOS_MODULE_ID_HDD, TRACE_CODE_HDD_WAKE_NETDEV,
                       pAdapter->sessionId, ac));
   }    

   // We're giving the packet to TL so consider it transmitted from
   // a statistics perspective.  We account for it here instead of
   // when the packet is returned for two reasons.  First, TL will
   // manipulate the skb to the point where the len field is not
   // accurate, leading to inaccurate byte counts if we account for
   // it later.  Second, TL does not provide any feedback as to
   // whether or not the packet was successfully sent over the air,
   // so the packet counts will be the same regardless of where we
   // account for them
   pAdapter->stats.tx_bytes += skb->len;
   ++pAdapter->stats.tx_packets;
   ++pAdapter->hdd_stats.hddTxRxStats.txFetchDequeued;
   ++pAdapter->hdd_stats.hddTxRxStats.txFetchDequeuedAC[ac];
   pAdapter->hdd_stats.hddTxRxStats.continuousTxTimeoutCount = 0;

   VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_INFO,
              "%s: Valid VOS PKT returned to TL", __func__);

   return status;
}


/**============================================================================
  @brief hdd_softap_tx_low_resource_cbk() - Callback function invoked in the 
  case where VOS packets are not available at the time of the call to get 
  packets. This callback function is invoked by VOS when packets are 
  available.

  @param pVosPacket : [in]  pointer to VOS packet 
  @param userData   : [in]  opaque user data that was passed initially 
  
  @return           : VOS_STATUS_E_FAILURE if any errors encountered, 
                    : VOS_STATUS_SUCCESS otherwise
  =============================================================================*/
VOS_STATUS hdd_softap_tx_low_resource_cbk( vos_pkt_t *pVosPacket, 
                                    v_VOID_t *userData )
{
   VOS_STATUS status;
   v_SINT_t i = 0;
   v_SIZE_t size = 0;
   hdd_adapter_t* pAdapter = (hdd_adapter_t *)userData;
   v_U8_t STAId = WLAN_MAX_STA_COUNT;
   v_CONTEXT_t pVosContext = NULL;
   ptSapContext pSapCtx = NULL;

   if (pAdapter == NULL || WLAN_HDD_ADAPTER_MAGIC != pAdapter->magic)
   {
      VOS_TRACE(VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_ERROR,
                 FL("Invalid adapter %p"), pAdapter);
      return VOS_STATUS_E_FAILURE;
   }
   pVosContext = (WLAN_HDD_GET_CTX(pAdapter))->pvosContext;
   pSapCtx = VOS_GET_SAP_CB(pVosContext);
   if(pSapCtx == NULL){
       VOS_TRACE(VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_ERROR,
             FL("psapCtx is NULL"));
       return VOS_STATUS_E_FAULT;
   }
   //Return the packet to VOS. We just needed to know that VOS is out of low resource
   //situation. Here we will only signal TL that there is a pending data for a STA. 
   //VOS packet will be requested (if needed) when TL comes back to fetch data.
   vos_pkt_return_packet( pVosPacket );

   pAdapter->isVosOutOfResource = VOS_FALSE;
   
   // Indicate to TL that there is pending data if a queue is non empty.
   // This Code wasnt included in earlier version which resulted in
   // Traffic stalling
   for (STAId = 0; STAId < WLAN_MAX_STA_COUNT; STAId++)
   {
      if ((pSapCtx->aStaInfo[STAId].tlSTAState == WLANTL_STA_AUTHENTICATED) ||
           (pSapCtx->aStaInfo[STAId].tlSTAState == WLANTL_STA_CONNECTED))
      {
         for( i=NUM_TX_QUEUES-1; i>=0; --i )
         {
            size = 0;
            hdd_list_size(&pSapCtx->aStaInfo[STAId].wmm_tx_queue[i], &size);
            if ( size > 0 )
            {
               status = WLANTL_STAPktPending( (WLAN_HDD_GET_CTX(pAdapter))->pvosContext,
                                        STAId,
                                        (WLANTL_ACEnumType)i );
               if( !VOS_IS_STATUS_SUCCESS( status ) )
               {
                  VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_ERROR,
                             "%s: Failure in indicating pkt to TL for ac=%d", __func__,i);
               }
            }
         }
      }
   } 
   return VOS_STATUS_SUCCESS;
}


/**============================================================================
  @brief hdd_softap_rx_packet_cbk() - Receive callback registered with TL.
  TL will call this to notify the HDD when one or more packets were
  received for a registered STA.

  @param vosContext      : [in] pointer to VOS context  
  @param pVosPacketChain : [in] pointer to VOS packet chain
  @param staId           : [in] Station Id (Adress 1 Index)
  @param pRxMetaInfo     : [in] pointer to meta info for the received pkt(s).

  @return                : VOS_STATUS_E_FAILURE if any errors encountered, 
                         : VOS_STATUS_SUCCESS otherwise
  ===========================================================================*/
VOS_STATUS hdd_softap_rx_packet_cbk( v_VOID_t *vosContext, 
                              vos_pkt_t *pVosPacketChain,
                              v_U8_t staId,
                              WLANTL_RxMetaInfoType* pRxMetaInfo )
{
   hdd_adapter_t *pAdapter = NULL;
   VOS_STATUS status = VOS_STATUS_E_FAILURE;
   int rxstat;
   struct sk_buff *skb = NULL;
   vos_pkt_t* pVosPacket;
   vos_pkt_t* pNextVosPacket;   
   hdd_context_t *pHddCtx = NULL;   
   v_U8_t proto_type;

   //Sanity check on inputs
   if ( ( NULL == vosContext ) || 
        ( NULL == pVosPacketChain ) ||
        ( NULL == pRxMetaInfo ) )
   {
      VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_ERROR,
                 "%s: Null params being passed", __func__);
      return VOS_STATUS_E_FAILURE;
   }
   
   pHddCtx = (hdd_context_t *)vos_get_context( VOS_MODULE_ID_HDD, vosContext );
   if ( NULL == pHddCtx )
   {
      VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_ERROR,
                 "%s: HDD adapter context is Null", __func__);
      return VOS_STATUS_E_FAILURE;
   }

   pAdapter = pHddCtx->sta_to_adapter[staId];
   if( (NULL == pAdapter) || (WLAN_HDD_ADAPTER_MAGIC != pAdapter->magic) )
   {
      VOS_TRACE(VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_ERROR,
          "%s: invalid adapter or adapter has invalid magic",__func__);
      VOS_ASSERT(0);
      return VOS_STATUS_E_FAILURE;
   }

   /* Monitor traffic */
   if ( pHddCtx->cfg_ini->enableTrafficMonitor )
   {
      pHddCtx->traffic_monitor.lastFrameTs = vos_timer_get_system_time();
      if ( !atomic_read(&pHddCtx->traffic_monitor.isActiveMode) )
      {
         vos_lock_acquire(&pHddCtx->traffic_monitor.trafficLock);
         /* It was IDLE mode,
          * this is new state, then switch mode from suspend to resume */
         if ( !atomic_read(&pHddCtx->traffic_monitor.isActiveMode) )
         {
            hdd_set_wlan_suspend_mode(0);
            vos_timer_start(&pHddCtx->traffic_monitor.trafficTimer,
                            pHddCtx->cfg_ini->trafficIdleTimeout);
            atomic_set(&pHddCtx->traffic_monitor.isActiveMode, 1);
         }
         vos_lock_release(&pHddCtx->traffic_monitor.trafficLock);
      }
   }

   ++pAdapter->hdd_stats.hddTxRxStats.rxChains;

   // walk the chain until all are processed
   pVosPacket = pVosPacketChain;
   do
   {
      // get the pointer to the next packet in the chain
      // (but don't unlink the packet since we free the entire chain later)
      status = vos_pkt_walk_packet_chain( pVosPacket, &pNextVosPacket, VOS_FALSE);

      // both "success" and "empty" are acceptable results
      if (!((status == VOS_STATUS_SUCCESS) || (status == VOS_STATUS_E_EMPTY)))
      {
         ++pAdapter->hdd_stats.hddTxRxStats.rxDropped;
         VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_ERROR,
                    "%s: Failure walking packet chain", __func__);
         return VOS_STATUS_E_FAILURE;
      }

      // Extract the OS packet (skb).
      status = vos_pkt_get_os_packet( pVosPacket, (v_VOID_t **)&skb, VOS_FALSE );
      if(!VOS_IS_STATUS_SUCCESS( status ))
      {
         ++pAdapter->hdd_stats.hddTxRxStats.rxDropped;
         VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_ERROR,
                    "%s: Failure extracting skb from vos pkt", __func__);
         return VOS_STATUS_E_FAILURE;
      }

      if (TRUE == hdd_IsEAPOLPacket(pVosPacket))
          wlan_hdd_log_eapol(skb, WIFI_EVENT_DRIVER_EAPOL_FRAME_RECEIVED);

      pVosPacket->pSkb = NULL;
      //hdd_softap_dump_sk_buff(skb);

      skb->dev = pAdapter->dev;
      
      if(skb->dev == NULL) {
  
          VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_FATAL,
                     "ERROR!!Invalid netdevice");
          return VOS_STATUS_E_FAILURE;
      }
      ++pAdapter->hdd_stats.hddTxRxStats.rxPackets;
      ++pAdapter->stats.rx_packets;
      pAdapter->stats.rx_bytes += skb->len;

      if (pHddCtx->cfg_ini->gEnableDebugLog)
      {
         proto_type = vos_pkt_get_proto_type(skb,
                                             pHddCtx->cfg_ini->gEnableDebugLog);
         if (VOS_PKT_PROTO_TYPE_EAPOL & proto_type)
         {
            VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
                      "SAP RX EAPOL");
         }
         else if (VOS_PKT_PROTO_TYPE_DHCP & proto_type)
         {
            VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
                      "SAP RX DHCP");
         }
         else if (VOS_PKT_PROTO_TYPE_ARP & proto_type)
         {
            VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
                      "SAP RX ARP");
         }
      }

      if (pHddCtx->rx_wow_dump) {
         if (!(VOS_PKT_PROTO_TYPE_ARP & proto_type) &&
             !(VOS_PKT_PROTO_TYPE_EAPOL & proto_type))
            hdd_log_ip_addr(skb);
            pHddCtx->rx_wow_dump = false;
      }

      if (WLAN_RX_BCMC_STA_ID == pRxMetaInfo->ucDesSTAId)
      {
         //MC/BC packets. Duplicate a copy of packet
         struct sk_buff *pSkbCopy;
         hdd_ap_ctx_t *pHddApCtx;

         pHddApCtx = WLAN_HDD_GET_AP_CTX_PTR(pAdapter);
         if (!(pHddApCtx->apDisableIntraBssFwd))
         {
             pSkbCopy = skb_copy(skb, GFP_ATOMIC);
             if (pSkbCopy)
             {
               hdd_softap_sta_2_sta_xmit(pSkbCopy, pSkbCopy->dev,
                          pHddApCtx->uBCStaId, (pRxMetaInfo->ucUP));
             }
         }
         else
         {
             VOS_TRACE(VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_ERROR,
                      "%s: skb allocation fails", __func__);
         }
 

      } //(WLAN_RX_BCMC_STA_ID == staId)

      if ((WLAN_RX_BCMC_STA_ID == pRxMetaInfo->ucDesSTAId) || 
          (WLAN_RX_SAP_SELF_STA_ID == pRxMetaInfo->ucDesSTAId))
      {
         VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_INFO_LOW,
                          "%s: send one packet to kernel", __func__);
         if ((NULL != pHddCtx) &&
             (pHddCtx->cfg_ini->gEnableDebugLog & VOS_PKT_PROTO_TYPE_DHCP))
         {
             hdd_dump_dhcp_pkt(skb, RX_PATH);
         }

         skb->protocol = eth_type_trans(skb, skb->dev);
         skb->ip_summed = CHECKSUM_NONE;
#ifdef WLAN_FEATURE_HOLD_RX_WAKELOCK
       vos_wake_lock_timeout_release(&pHddCtx->rx_wake_lock,
                    HDD_WAKE_LOCK_DURATION,
                    WIFI_POWER_EVENT_WAKELOCK_HOLD_RX);

#endif
         rxstat = netif_rx_ni(skb);
         if (NET_RX_SUCCESS == rxstat)
         {
            ++pAdapter->hdd_stats.hddTxRxStats.rxDelivered;
            ++pAdapter->hdd_stats.hddTxRxStats.pkt_rx_count;
         }
         else
         {
            ++pAdapter->hdd_stats.hddTxRxStats.rxRefused;
         }
      }
      else if ((WLAN_HDD_GET_AP_CTX_PTR(pAdapter))->apDisableIntraBssFwd)
      {
        kfree_skb(skb);   
      } 
      else
      {
         //loopback traffic
        status = hdd_softap_sta_2_sta_xmit(skb, skb->dev,
                 pRxMetaInfo->ucDesSTAId, (pRxMetaInfo->ucUP));
      }

      // now process the next packet in the chain
      pVosPacket = pNextVosPacket;

   } while (pVosPacket);

   //Return the entire VOS packet chain to the resource pool
   status = vos_pkt_return_packet( pVosPacketChain );
   if(!VOS_IS_STATUS_SUCCESS( status ))
   {
      VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_ERROR,
                 "%s: Failure returning vos pkt", __func__);
   }
   
   pAdapter->dev->last_rx = jiffies;

   return status;   
}

VOS_STATUS hdd_softap_DeregisterSTA( hdd_adapter_t *pAdapter, tANI_U8 staId )
{
    VOS_STATUS vosStatus = VOS_STATUS_SUCCESS;
    hdd_context_t *pHddCtx;
    if (NULL == pAdapter)
    {
        VOS_TRACE(VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_ERROR,
                    "%s: pAdapter is NULL", __func__);
        return VOS_STATUS_E_INVAL;
    }

    if (WLAN_HDD_ADAPTER_MAGIC != pAdapter->magic)
    {
        VOS_TRACE(VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_ERROR,
                    "%s: Invalid pAdapter magic", __func__);
        return VOS_STATUS_E_INVAL;
    }

    pHddCtx = (hdd_context_t*)(pAdapter->pHddCtx);
    //Clear station in TL and then update HDD data structures. This helps
    //to block RX frames from other station to this station.
    vosStatus = WLANTL_ClearSTAClient( pHddCtx->pvosContext, staId );
    if ( !VOS_IS_STATUS_SUCCESS( vosStatus ) )
    {
        VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_ERROR,
                    "WLANTL_ClearSTAClient() failed to for staID %d.  "
                    "Status= %d [0x%08X]",
                    staId, vosStatus, vosStatus );
    }

    vosStatus = hdd_softap_deinit_tx_rx_sta ( pAdapter, staId );
    if( VOS_STATUS_E_FAILURE == vosStatus )
    {
        VOS_TRACE ( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_ERROR,
                    "hdd_softap_deinit_tx_rx_sta() failed for staID %d. "
                    "Status = %d [0x%08X]",
                    staId, vosStatus, vosStatus );
        return( vosStatus );
    }
    
    pHddCtx->sta_to_adapter[staId] = NULL;

    return( vosStatus );
}

VOS_STATUS hdd_softap_RegisterSTA( hdd_adapter_t *pAdapter,
                                       v_BOOL_t fAuthRequired,
                                       v_BOOL_t fPrivacyBit,
                                       v_U8_t staId,
                                       v_U8_t ucastSig,
                                       v_U8_t bcastSig,
                                       v_MACADDR_t *pPeerMacAddress,
                                       v_BOOL_t fWmmEnabled )
{
   VOS_STATUS vosStatus = VOS_STATUS_E_FAILURE;
   WLAN_STADescType staDesc = {0};   
   hdd_context_t *pHddCtx = pAdapter->pHddCtx;
   hdd_adapter_t *pmonAdapter = NULL;

   v_CONTEXT_t pVosContext = ( WLAN_HDD_GET_CTX(pAdapter))->pvosContext;
   ptSapContext pSapCtx = NULL;
   pSapCtx = VOS_GET_SAP_CB(pVosContext);
   if(pSapCtx == NULL){
       VOS_TRACE(VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_ERROR,
             FL("psapCtx is NULL"));
       return VOS_STATUS_E_FAULT;
   }
   //eCsrEncryptionType connectedCipherAlgo;
   //v_BOOL_t  fConnected;
   
   /*
    * Clean up old entry if it is not cleaned up properly
   */
   if ( pSapCtx->aStaInfo[staId].isUsed )
   {
      VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_INFO,
                 "clean up old entry for STA %d", staId);
      hdd_softap_DeregisterSTA( pAdapter, staId );
   }

   // Get the Station ID from the one saved during the assocation.
   
   staDesc.ucSTAId = staId;
   

   /*Save the pAdapter Pointer for this staId*/
   pHddCtx->sta_to_adapter[staId] = pAdapter;

   staDesc.wSTAType = WLAN_STA_SOFTAP;

   vos_mem_copy( staDesc.vSTAMACAddress.bytes, pPeerMacAddress->bytes,sizeof(pPeerMacAddress->bytes) );
   vos_mem_copy( staDesc.vBSSIDforIBSS.bytes, &pAdapter->macAddressCurrent,6 );
   vos_copy_macaddr( &staDesc.vSelfMACAddress, &pAdapter->macAddressCurrent );

   VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_INFO,
              "register station");
   VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_INFO,
              "station mac " MAC_ADDRESS_STR,
              MAC_ADDR_ARRAY(staDesc.vSTAMACAddress.bytes));
   VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_INFO,
              "BSSIDforIBSS " MAC_ADDRESS_STR,
              MAC_ADDR_ARRAY(staDesc.vBSSIDforIBSS.bytes));
   VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_INFO,
              "SOFTAP SELFMAC " MAC_ADDRESS_STR,
              MAC_ADDR_ARRAY(staDesc.vSelfMACAddress.bytes));

   vosStatus = hdd_softap_init_tx_rx_sta(pAdapter, staId, &staDesc.vSTAMACAddress);

   staDesc.ucQosEnabled = fWmmEnabled;
   VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_INFO,
              "HDD SOFTAP register TL QoS_enabled=%d",
              staDesc.ucQosEnabled );

   staDesc.ucProtectedFrame = (v_U8_t)fPrivacyBit ;


   // For PRIMA UMA frame translation is not enable yet.
   staDesc.ucSwFrameTXXlation = 1;
   staDesc.ucSwFrameRXXlation = 1;
   staDesc.ucAddRmvLLC = 1;

   // Initialize signatures and state
   staDesc.ucUcastSig  = ucastSig;
   staDesc.ucBcastSig  = bcastSig;
   staDesc.ucInitState = fAuthRequired ?
      WLANTL_STA_CONNECTED : WLANTL_STA_AUTHENTICATED;

   staDesc.ucIsReplayCheckValid = VOS_FALSE;

   // Register the Station with TL...      
   vosStatus = WLANTL_RegisterSTAClient( (WLAN_HDD_GET_CTX(pAdapter))->pvosContext,
                                         hdd_softap_rx_packet_cbk,
                                         hdd_softap_tx_complete_cbk,
                                         hdd_softap_tx_fetch_packet_cbk, &staDesc, 0 );
   
   if ( !VOS_IS_STATUS_SUCCESS( vosStatus ) )
   {
      VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_ERROR,
                 "SOFTAP WLANTL_RegisterSTAClient() failed to register.  Status= %d [0x%08X]",
                 vosStatus, vosStatus );
      return vosStatus;      
   }

   //Timer value should be in milliseconds
   if ( pHddCtx->cfg_ini->dynSplitscan &&
      ( VOS_TIMER_STATE_RUNNING !=
                      vos_timer_getCurrentState(&pHddCtx->tx_rx_trafficTmr)))
   {
        vos_timer_start(&pHddCtx->tx_rx_trafficTmr,
                        pHddCtx->cfg_ini->trafficMntrTmrForSplitScan);
   }

   // if ( WPA ), tell TL to go to 'connected' and after keys come to the driver, 
   // then go to 'authenticated'.  For all other authentication types (those that do 
   // not require upper layer authentication) we can put TL directly into 'authenticated'
   // state.
   
   //VOS_ASSERT( fConnected );
   pSapCtx->aStaInfo[staId].ucSTAId = staId;
   pSapCtx->aStaInfo[staId].isQosEnabled = fWmmEnabled;
   if ( !fAuthRequired )
   {
      VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_INFO,
                 "open/shared auth StaId= %d.  Changing TL state to AUTHENTICATED at Join time",
                  pSapCtx->aStaInfo[staId].ucSTAId );
   
      // Connections that do not need Upper layer auth, transition TL directly
      // to 'Authenticated' state.      
      vosStatus = WLANTL_ChangeSTAState( (WLAN_HDD_GET_CTX(pAdapter))->pvosContext, staDesc.ucSTAId, 
                                         WLANTL_STA_AUTHENTICATED );
  
      pSapCtx->aStaInfo[staId].tlSTAState = WLANTL_STA_AUTHENTICATED;
      pAdapter->sessionCtx.ap.uIsAuthenticated = VOS_TRUE;
   }                                            
   else
   {

      VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_INFO,
                 "ULA auth StaId= %d.  Changing TL state to CONNECTED at Join time", pSapCtx->aStaInfo[staId].ucSTAId );
   
      vosStatus = WLANTL_ChangeSTAState( (WLAN_HDD_GET_CTX(pAdapter))->pvosContext, staDesc.ucSTAId, 
                                         WLANTL_STA_CONNECTED );
      pSapCtx->aStaInfo[staId].tlSTAState = WLANTL_STA_CONNECTED;

      pAdapter->sessionCtx.ap.uIsAuthenticated = VOS_FALSE;

   }      
   pmonAdapter= hdd_get_mon_adapter( pAdapter->pHddCtx);
   if(pmonAdapter) 
   {
       VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_INFO_HIGH,
                  "Turn on Monitor the carrier");
       netif_carrier_on(pmonAdapter->dev);
           //Enable Tx queue
       hddLog(VOS_TRACE_LEVEL_INFO, FL("Enabling queues"));
       netif_tx_start_all_queues(pmonAdapter->dev);
    }
   netif_carrier_on(pAdapter->dev);
   //Enable Tx queue
   hddLog(VOS_TRACE_LEVEL_INFO, FL("Enabling queues"));
   netif_tx_start_all_queues(pAdapter->dev);

   return( vosStatus );
}

VOS_STATUS hdd_softap_Register_BC_STA( hdd_adapter_t *pAdapter, v_BOOL_t fPrivacyBit)
{
   VOS_STATUS vosStatus = VOS_STATUS_E_FAILURE;
   hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
   v_MACADDR_t broadcastMacAddr = VOS_MAC_ADDR_BROADCAST_INITIALIZER;
   
   
   pHddCtx->sta_to_adapter[WLAN_RX_BCMC_STA_ID] = pAdapter;
   pHddCtx->sta_to_adapter[WLAN_RX_SAP_SELF_STA_ID] = pAdapter;
   vosStatus = hdd_softap_RegisterSTA( pAdapter, VOS_FALSE, fPrivacyBit, (WLAN_HDD_GET_AP_CTX_PTR(pAdapter))->uBCStaId, 0, 1, &broadcastMacAddr,0);

   return vosStatus;
}

VOS_STATUS hdd_softap_Deregister_BC_STA( hdd_adapter_t *pAdapter)
{
   return hdd_softap_DeregisterSTA( pAdapter, (WLAN_HDD_GET_AP_CTX_PTR(pAdapter))->uBCStaId);
}

VOS_STATUS hdd_softap_stop_bss( hdd_adapter_t *pAdapter)
{
    VOS_STATUS vosStatus = VOS_STATUS_E_FAILURE;
    v_U8_t staId = 0;
    hdd_context_t *pHddCtx;

    v_CONTEXT_t pVosContext = ( WLAN_HDD_GET_CTX(pAdapter))->pvosContext;
    ptSapContext pSapCtx = NULL;
    pSapCtx = VOS_GET_SAP_CB(pVosContext);
    if(pSapCtx == NULL){
        VOS_TRACE(VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_ERROR,
             FL("psapCtx is NULL"));
        return VOS_STATUS_E_FAULT;
    }

    pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
    /*bss deregister is not allowed during wlan driver loading or unloading*/
    if (WLAN_HDD_IS_LOAD_UNLOAD_IN_PROGRESS(pHddCtx))
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                   "%s:Loading_unloading in Progress. Ignore!!!",__func__);
        return VOS_STATUS_E_PERM;
    }

    vosStatus = hdd_softap_Deregister_BC_STA( pAdapter);

    if (!VOS_IS_STATUS_SUCCESS(vosStatus))
    {
        VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_ERROR,
                   "%s: Failed to deregister BC sta Id %d", __func__, (WLAN_HDD_GET_AP_CTX_PTR(pAdapter))->uBCStaId);
    }

    for (staId = 0; staId < WLAN_MAX_STA_COUNT; staId++)
    {
        if (pSapCtx->aStaInfo[staId].isUsed)// This excludes BC sta as it is already deregistered
        {
            vosStatus = hdd_softap_DeregisterSTA( pAdapter, staId);
            if (!VOS_IS_STATUS_SUCCESS(vosStatus))
            {
                VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_ERROR,
                       "%s: Failed to deregister sta Id %d", __func__, staId);
            }
       }
    }

    return vosStatus;
}

VOS_STATUS hdd_softap_change_STA_state( hdd_adapter_t *pAdapter, v_MACADDR_t *pDestMacAddress, WLANTL_STAStateType state)
{
    v_U8_t ucSTAId = WLAN_MAX_STA_COUNT;
    VOS_STATUS vosStatus = eHAL_STATUS_SUCCESS;
    v_CONTEXT_t pVosContext = (WLAN_HDD_GET_CTX(pAdapter))->pvosContext;

    ptSapContext pSapCtx = NULL;
    pSapCtx = VOS_GET_SAP_CB(pVosContext);
    if(pSapCtx == NULL){
        VOS_TRACE(VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_ERROR,
             FL("psapCtx is NULL"));
        return VOS_STATUS_E_FAULT;
    }
    VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_INFO,
               "%s: enter", __func__);

    if (VOS_STATUS_SUCCESS != hdd_softap_GetStaId(pAdapter, pDestMacAddress, &ucSTAId))
    {
        VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_ERROR,
                    "%s: Failed to find right station", __func__);
        return VOS_STATUS_E_FAILURE;
    }

    if (FALSE == vos_is_macaddr_equal(&pSapCtx->aStaInfo[ucSTAId].macAddrSTA, pDestMacAddress))
    {
         VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_ERROR,
                    "%s: Station MAC address does not matching", __func__);
         return VOS_STATUS_E_FAILURE;
    }

    vosStatus = WLANTL_ChangeSTAState( pVosContext, ucSTAId, state );
    VOS_TRACE( VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_INFO,
                   "%s: change station to state %d succeed", __func__, state);

    if (VOS_STATUS_SUCCESS == vosStatus)
    {
       pSapCtx->aStaInfo[ucSTAId].tlSTAState = WLANTL_STA_AUTHENTICATED;
    }

    VOS_TRACE(VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_INFO,
                                     "%s exit",__func__);

    return vosStatus;
}


VOS_STATUS hdd_softap_GetStaId(hdd_adapter_t *pAdapter, v_MACADDR_t *pMacAddress, v_U8_t *staId)
{
    v_U8_t i;

    v_CONTEXT_t pVosContext = ( WLAN_HDD_GET_CTX(pAdapter))->pvosContext;
    ptSapContext pSapCtx = NULL;
    pSapCtx = VOS_GET_SAP_CB(pVosContext);
    if(pSapCtx == NULL){
        VOS_TRACE(VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_ERROR,
             FL("psapCtx is NULL"));
        return VOS_STATUS_E_FAULT;
    }
    for (i = 0; i < WLAN_MAX_STA_COUNT; i++)
    {
        if (vos_mem_compare(&pSapCtx->aStaInfo[i].macAddrSTA, pMacAddress, sizeof(v_MACADDR_t)) &&
            pSapCtx->aStaInfo[i].isUsed)
        {
            *staId = i;
            return VOS_STATUS_SUCCESS;
        }
    }

    return VOS_STATUS_E_FAILURE;
}

VOS_STATUS hdd_softap_GetConnectedStaId(hdd_adapter_t *pAdapter, v_U8_t *staId)
{
    v_U8_t i;

    v_CONTEXT_t pVosContext = ( WLAN_HDD_GET_CTX(pAdapter))->pvosContext;
    ptSapContext pSapCtx = NULL;
    pSapCtx = VOS_GET_SAP_CB(pVosContext);
    if(pSapCtx == NULL){
        VOS_TRACE(VOS_MODULE_ID_HDD_SAP_DATA, VOS_TRACE_LEVEL_ERROR,
             FL("psapCtx is NULL"));
        return VOS_STATUS_E_FAULT;
    }
    for (i = 0; i < WLAN_MAX_STA_COUNT; i++)
    {
        if (pSapCtx->aStaInfo[i].isUsed &&
            (!vos_is_macaddr_broadcast(&pSapCtx->aStaInfo[i].macAddrSTA)))
        {
            *staId = i;
            return VOS_STATUS_SUCCESS;
        }
    }

    return VOS_STATUS_E_FAILURE;
}
