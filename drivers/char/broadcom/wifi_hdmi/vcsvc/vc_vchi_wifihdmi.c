/*****************************************************************************
* Copyright 2011 Broadcom Corporation.  All rights reserved.
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

// ---- Include Files --------------------------------------------------------
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/cdev.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/pagemap.h>
#include <linux/slab.h>
#include <linux/ioctl.h>
#include <linux/semaphore.h>
#include <linux/proc_fs.h>
#include <linux/gpio.h>

#include "vc_vchi_wifihdmi.h"
#include <vc_sm_defs.h>
#include <vc_sm_knl.h>
#include <linux/broadcom/hdmi.h>
#include <linux/broadcom/whdmi.h>
#include <asm/memory.h>

// ---- Private Constants and Types ------------------------------------------

#define CLIENT_EVENT_MASK   0x1
#define SERVER_EVENT_MASK   0x2
#define NOTIFY_EVENT_MASK   0x4

#define SMEM_POOL_SIZE      1600
#define SMEM_POOL_DEPTH     100

// VCOS logging category for this service
#define VCOS_LOG_CATEGORY (&wifihdmi_log_category)

// Default VCOS logging level
// #define LOG_LEVEL  VCOS_LOG_TRACE
#define LOG_LEVEL  VCOS_LOG_INFO

// Logging macros (for remapping to other logging mechanisms, i.e., printf)
#define LOG_ERR( fmt, arg... )   vcos_log_error( fmt, ##arg )
#define LOG_WARN( fmt, arg... )  vcos_log_warn( fmt, ##arg )
#define LOG_INFO( fmt, arg... )  vcos_log_info( fmt, ##arg )
#define LOG_DBG( fmt, arg... )   vcos_log_trace( fmt, ##arg )

typedef struct wifihdmi_cmd_rsp_blk_t
{
   struct wifihdmi_cmd_rsp_blk_t    *next;       // Linked list next pointer

   VCOS_EVENT_T                     cmd_resp;    // To be signaled when the response is there
   uint32_t                         trans_id;

   uint8_t                          command_msg_buf[VC_WIFIHDMI_MAX_MSG_LEN];
   uint32_t                         command_len;

   uint8_t                          reply_wait;
   uint8_t                          reply_msg_buf[VC_WIFIHDMI_MAX_RSP_LEN];

} WIFIHDMI_CMD_RSP_BLK_T;

typedef struct wifihdmi_snd_blk_t
{
   struct wifihdmi_snd_blk_t    *next;       // Linked list next pointer

   uint32_t                     handle;
   uint32_t                     size;
   uint32_t                     socket;
   uint32_t                     address;
   uint32_t                     port;

} WIFIHDMI_SND_BLK_T;

typedef struct wifihdmi_data_pump_blk_t
{
   struct wifihdmi_data_pump_blk_t    *next;       // Linked list next pointer

   uint32_t                           handle;
   uint32_t                           size;

} WIFIHDMI_DATA_PUMP_BLK_T;

typedef struct wifihdmi_ctrl_blk_t
{
   struct wifihdmi_ctrl_blk_t   *next;       // Linked list next pointer

   VC_WIFIHDMI_MSG_TYPE         action;
   uint32_t                     handle;
   uint16_t                     port;
   uint16_t                     sendonly;

} WIFIHDMI_CTRL_BLK_T;

typedef struct opaque_vc_vchi_wifihdmi_handle_t
{
   uint32_t               num_connections;
   VCHI_SERVICE_HANDLE_T  vchi_server[VCHI_MAX_NUM_CONNECTIONS];
   VCHI_SERVICE_HANDLE_T  vchi_srv_fast[VCHI_MAX_NUM_CONNECTIONS];
   VCHI_SERVICE_HANDLE_T  vchi_notifier_ctrl[VCHI_MAX_NUM_CONNECTIONS];
   VCHI_SERVICE_HANDLE_T  vchi_notifier_data[VCHI_MAX_NUM_CONNECTIONS];

   VCOS_THREAD_T          io_thread;
   uint32_t               io_event_mask;
   VCOS_MUTEX_T           io_lock;
   VCOS_EVENT_T           io_event;

   VCOS_THREAD_T          iof_thread;
   uint32_t               iof_event_mask;
   VCOS_MUTEX_T           iof_lock;
   VCOS_EVENT_T           iof_event;

   VCOS_THREAD_T          dt_thread;
   uint32_t               dt_event_mask;
   VCOS_MUTEX_T           dt_lock;
   VCOS_EVENT_T           dt_event;

   VCOS_THREAD_T          snd_thread;
   VCOS_EVENT_T           snd_event;

   VCOS_THREAD_T          ctrl_thread;
   VCOS_EVENT_T           ctrl_event;

   VCOS_SEMAPHORE_T       vchi_sema;
   VCOS_SEMAPHORE_T       vchi_fast_sema;

   uint32_t               trans_id;
   uint32_t               trans_fast_id;

   WIFIHDMI_CMD_RSP_BLK_T *cmd_list;
   VCOS_MUTEX_T           cmd_lock;

   WIFIHDMI_CMD_RSP_BLK_T *cmd_fast_list;
   VCOS_MUTEX_T           cmd_fast_lock;

   WIFIHDMI_CMD_RSP_BLK_T *rsp_list;
   VCOS_MUTEX_T           rsp_lock;

   WIFIHDMI_CMD_RSP_BLK_T *rsp_fast_list;
   VCOS_MUTEX_T           rsp_fast_lock;

   WIFIHDMI_SND_BLK_T     *snd_list;
   VCOS_MUTEX_T           snd_lock;

   WIFIHDMI_DATA_PUMP_BLK_T *data_list;
   VCOS_MUTEX_T           data_lock;

   WIFIHDMI_CTRL_BLK_T    *ctrl_list;
   VCOS_MUTEX_T           ctrl_lock;

   uint32_t               data_in_handle;
   uint32_t               pool_init;

   uint32_t               gpio_toggle;
   uint32_t               gpio_toggle_cnt;

} WIFIHDMI_INSTANCE_T;

// ---- Private Variables ----------------------------------------------------

// VCOS logging category for this service
static VCOS_LOG_CAT_T wifihdmi_log_category;

// ---- Private Function Prototypes ------------------------------------------

// ---- Private Functions ----------------------------------------------------

static void vc_vchi_add_data( WIFIHDMI_INSTANCE_T *instance,
                              WIFIHDMI_DATA_PUMP_BLK_T *data )
{
   vcos_mutex_lock ( &(instance->data_lock) );

   if ( instance->data_list == NULL )
   {
      instance->data_list = data;
   }
   else
   {
      WIFIHDMI_DATA_PUMP_BLK_T *ptr = instance->data_list;

      while ( ptr->next != NULL )
      {
         ptr = ptr->next;
      }

      ptr->next = data;
   }

   data->next = NULL;

   vcos_mutex_unlock ( &(instance->data_lock) );
}

static void vc_vchi_add_snd( WIFIHDMI_INSTANCE_T *instance,
                             WIFIHDMI_SND_BLK_T *snd )
{
   vcos_mutex_lock ( &(instance->snd_lock) );

   if ( instance->snd_list == NULL )
   {
      instance->snd_list = snd;
   }
   else
   {
      WIFIHDMI_SND_BLK_T *ptr = instance->snd_list;

      while ( ptr->next != NULL )
      {
         ptr = ptr->next;
      }

      ptr->next = snd;
   }

   snd->next = NULL;

   vcos_mutex_unlock ( &(instance->snd_lock) );
}

static void vc_vchi_add_ctrl( WIFIHDMI_INSTANCE_T *instance,
                              WIFIHDMI_CTRL_BLK_T *ctrl )
{
   vcos_mutex_lock ( &(instance->ctrl_lock) );

   if ( instance->ctrl_list == NULL )
   {
      instance->ctrl_list = ctrl;
   }
   else
   {
      WIFIHDMI_CTRL_BLK_T *ptr = instance->ctrl_list;

      while ( ptr->next != NULL )
      {
         ptr = ptr->next;
      }

      ptr->next = ctrl;
   }

   ctrl->next = NULL;

   vcos_mutex_unlock ( &(instance->ctrl_lock) );
}

static WIFIHDMI_SND_BLK_T *vc_vchi_deq_snd( WIFIHDMI_INSTANCE_T *instance )
{
   WIFIHDMI_SND_BLK_T *prev_ptr = NULL;

   vcos_mutex_lock( &(instance->snd_lock) );
   
   if ( instance->snd_list != NULL )
   {
      prev_ptr = instance->snd_list;
      instance->snd_list = instance->snd_list->next;
      prev_ptr->next = NULL;
   }

   vcos_mutex_unlock( &(instance->snd_lock) );
   return prev_ptr;
}

static WIFIHDMI_CTRL_BLK_T *vc_vchi_deq_ctrl( WIFIHDMI_INSTANCE_T *instance )
{
   WIFIHDMI_CTRL_BLK_T *prev_ptr = NULL;

   vcos_mutex_lock( &(instance->ctrl_lock) );
   
   if ( instance->ctrl_list != NULL )
   {
      prev_ptr = instance->ctrl_list;
      instance->ctrl_list = instance->ctrl_list->next;
      prev_ptr->next = NULL;
   }

   vcos_mutex_unlock( &(instance->ctrl_lock) );
   return prev_ptr;
}

static void vc_vchi_add_cmd( WIFIHDMI_INSTANCE_T *instance,
                             WIFIHDMI_CMD_RSP_BLK_T *cmd )
{
   vcos_mutex_lock ( &(instance->cmd_lock) );

   if ( instance->cmd_list == NULL )
   {
      instance->cmd_list = cmd;
   }
   else
   {
      WIFIHDMI_CMD_RSP_BLK_T *ptr = instance->cmd_list;

      while ( ptr->next != NULL )
      {
         ptr = ptr->next;
      }

      ptr->next = cmd;
   }

   cmd->next = NULL;

   vcos_mutex_unlock ( &(instance->cmd_lock) );
}

static void vc_vchi_add_fast_cmd( WIFIHDMI_INSTANCE_T *instance,
                                  WIFIHDMI_CMD_RSP_BLK_T *cmd )
{
   vcos_mutex_lock ( &(instance->cmd_fast_lock) );

   if ( instance->cmd_fast_list == NULL )
   {
      instance->cmd_fast_list = cmd;
   }
   else
   {
      WIFIHDMI_CMD_RSP_BLK_T *ptr = instance->cmd_fast_list;

      while ( ptr->next != NULL )
      {
         ptr = ptr->next;
      }

      ptr->next = cmd;
   }

   cmd->next = NULL;

   vcos_mutex_unlock ( &(instance->cmd_fast_lock) );
}

static void vc_vchi_add_rsp( WIFIHDMI_INSTANCE_T *instance,
                             WIFIHDMI_CMD_RSP_BLK_T *rsp )
{
   vcos_mutex_lock ( &(instance->rsp_lock) );

   if ( instance->rsp_list == NULL )
   {
      instance->rsp_list = rsp;
   }
   else
   {
      WIFIHDMI_CMD_RSP_BLK_T *ptr = instance->rsp_list;

      while ( ptr->next != NULL )
      {
         ptr = ptr->next;
      }

      ptr->next = rsp;
   }

   rsp->next = NULL;

   vcos_mutex_unlock ( &(instance->rsp_lock) );
}

static void vc_vchi_add_fast_rsp( WIFIHDMI_INSTANCE_T *instance,
                                  WIFIHDMI_CMD_RSP_BLK_T *rsp )
{
   vcos_mutex_lock ( &(instance->rsp_fast_lock) );

   if ( instance->rsp_fast_list == NULL )
   {
      instance->rsp_fast_list = rsp;
   }
   else
   {
      WIFIHDMI_CMD_RSP_BLK_T *ptr = instance->rsp_fast_list;

      while ( ptr->next != NULL )
      {
         ptr = ptr->next;
      }

      ptr->next = rsp;
   }

   rsp->next = NULL;

   vcos_mutex_unlock ( &(instance->rsp_fast_lock) );
}

static void vc_vchi_rem_cmd( WIFIHDMI_INSTANCE_T *instance,
                             WIFIHDMI_CMD_RSP_BLK_T **cmd )
{
   WIFIHDMI_CMD_RSP_BLK_T *prev_ptr = NULL;
   WIFIHDMI_CMD_RSP_BLK_T *curr_ptr = NULL;

   vcos_mutex_lock ( &(instance->cmd_lock) );

   curr_ptr = instance->cmd_list;
   while ( curr_ptr != NULL )
   {
      if ( curr_ptr == *cmd )
      {
         if ( curr_ptr == instance->cmd_list )
         {
            instance->cmd_list = curr_ptr->next;
         }
         else
         {
            prev_ptr->next = curr_ptr->next;
         }

         vcos_event_delete( &((*cmd)->cmd_resp) );
         vcos_free( *cmd );
         *cmd = NULL;

         vcos_mutex_unlock ( &(instance->cmd_lock) );
         return;
      }
      else
      {
         prev_ptr = curr_ptr;
         curr_ptr = curr_ptr->next;
      }
   }

   /* This could actually be a valid situation is the command was unlinked
   ** (ie failed or is a 'no response' type of command).
   */
   vcos_event_delete( &((*cmd)->cmd_resp) );
   vcos_free( *cmd );
   *cmd = NULL;

   vcos_mutex_unlock ( &(instance->cmd_lock) );
}

static void vc_vchi_rem_fast_cmd( WIFIHDMI_INSTANCE_T *instance,
                                  WIFIHDMI_CMD_RSP_BLK_T **cmd )
{
   WIFIHDMI_CMD_RSP_BLK_T *prev_ptr = NULL;
   WIFIHDMI_CMD_RSP_BLK_T *curr_ptr = NULL;

   vcos_mutex_lock ( &(instance->cmd_fast_lock) );

   curr_ptr = instance->cmd_fast_list;
   while ( curr_ptr != NULL )
   {
      if ( curr_ptr == *cmd )
      {
         if ( curr_ptr == instance->cmd_fast_list )
         {
            instance->cmd_fast_list = curr_ptr->next;
         }
         else
         {
            prev_ptr->next = curr_ptr->next;
         }

         vcos_event_delete( &((*cmd)->cmd_resp) );
         vcos_free( *cmd );
         *cmd = NULL;

         vcos_mutex_unlock ( &(instance->cmd_fast_lock) );
         return;
      }
      else
      {
         prev_ptr = curr_ptr;
         curr_ptr = curr_ptr->next;
      }
   }

   /* This could actually be a valid situation is the command was unlinked
   ** (ie failed or is a 'no response' type of command).
   */
   vcos_event_delete( &((*cmd)->cmd_resp) );
   vcos_free( *cmd );
   *cmd = NULL;

   vcos_mutex_unlock ( &(instance->cmd_fast_lock) );
}

static void vc_vchi_unlink_cmd( WIFIHDMI_INSTANCE_T *instance,
                                WIFIHDMI_CMD_RSP_BLK_T *cmd )
{
   WIFIHDMI_CMD_RSP_BLK_T *prev_ptr = NULL;
   WIFIHDMI_CMD_RSP_BLK_T *curr_ptr = NULL;

   vcos_mutex_lock ( &(instance->cmd_lock) );

   curr_ptr = instance->cmd_list;
   while ( curr_ptr != NULL )
   {
      if ( curr_ptr == cmd )
      {
         if ( curr_ptr == instance->cmd_list )
         {
            instance->cmd_list = curr_ptr->next;
         }
         else
         {
            prev_ptr->next = curr_ptr->next;
         }

         vcos_mutex_unlock ( &(instance->cmd_lock) );
         return;
      }
      else
      {
         prev_ptr = curr_ptr;
         curr_ptr = curr_ptr->next;
      }
   }

   vcos_mutex_unlock ( &(instance->cmd_lock) );
}

static void vc_vchi_unlink_fast_cmd( WIFIHDMI_INSTANCE_T *instance,
                                     WIFIHDMI_CMD_RSP_BLK_T *cmd )
{
   WIFIHDMI_CMD_RSP_BLK_T *prev_ptr = NULL;
   WIFIHDMI_CMD_RSP_BLK_T *curr_ptr = NULL;

   vcos_mutex_lock ( &(instance->cmd_fast_lock) );

   curr_ptr = instance->cmd_fast_list;
   while ( curr_ptr != NULL )
   {
      if ( curr_ptr == cmd )
      {
         if ( curr_ptr == instance->cmd_fast_list )
         {
            instance->cmd_fast_list = curr_ptr->next;
         }
         else
         {
            prev_ptr->next = curr_ptr->next;
         }

         vcos_mutex_unlock ( &(instance->cmd_fast_lock) );
         return;
      }
      else
      {
         prev_ptr = curr_ptr;
         curr_ptr = curr_ptr->next;
      }
   }

   vcos_mutex_unlock ( &(instance->cmd_fast_lock) );
}

static void vc_vchi_rem_rsp( WIFIHDMI_INSTANCE_T *instance,
                             WIFIHDMI_CMD_RSP_BLK_T **rsp )
{
   WIFIHDMI_CMD_RSP_BLK_T *prev_ptr = NULL;
   WIFIHDMI_CMD_RSP_BLK_T *curr_ptr = NULL;

   vcos_mutex_lock ( &(instance->rsp_lock) );

   curr_ptr = instance->rsp_list;
   while ( curr_ptr != NULL )
   {
      if ( curr_ptr == *rsp )
      {
         if ( curr_ptr == instance->rsp_list )
         {
            instance->rsp_list = curr_ptr->next;
         }
         else
         {
            prev_ptr->next = curr_ptr->next;
         }

         vcos_event_delete( &((*rsp)->cmd_resp) );
         vcos_free( *rsp );
         *rsp = NULL;

         vcos_mutex_unlock ( &(instance->rsp_lock) );
         return;
      }
      else
      {
         prev_ptr = curr_ptr;
         curr_ptr = curr_ptr->next;
      }
   }

   /* We should free the memory anyway to avoid a memory leak.
   */
   vcos_event_delete( &((*rsp)->cmd_resp) );
   vcos_free( *rsp );
   *rsp = NULL;

   vcos_mutex_unlock ( &(instance->rsp_lock) );
}

static void vc_vchi_rem_fast_rsp( WIFIHDMI_INSTANCE_T *instance,
                                  WIFIHDMI_CMD_RSP_BLK_T **rsp )
{
   WIFIHDMI_CMD_RSP_BLK_T *prev_ptr = NULL;
   WIFIHDMI_CMD_RSP_BLK_T *curr_ptr = NULL;

   vcos_mutex_lock ( &(instance->rsp_fast_lock) );

   curr_ptr = instance->rsp_fast_list;
   while ( curr_ptr != NULL )
   {
      if ( curr_ptr == *rsp )
      {
         if ( curr_ptr == instance->rsp_fast_list )
         {
            instance->rsp_fast_list = curr_ptr->next;
         }
         else
         {
            prev_ptr->next = curr_ptr->next;
         }

         vcos_event_delete( &((*rsp)->cmd_resp) );
         vcos_free( *rsp );
         *rsp = NULL;

         vcos_mutex_unlock ( &(instance->rsp_fast_lock) );
         return;
      }
      else
      {
         prev_ptr = curr_ptr;
         curr_ptr = curr_ptr->next;
      }
   }

   /* We should free the memory anyway to avoid a memory leak.
   */
   vcos_event_delete( &((*rsp)->cmd_resp) );
   vcos_free( *rsp );
   *rsp = NULL;

   vcos_mutex_unlock ( &(instance->rsp_fast_lock) );
}

static void vc_vchi_rem_data( WIFIHDMI_INSTANCE_T *instance,
                              WIFIHDMI_DATA_PUMP_BLK_T **data )
{
   WIFIHDMI_DATA_PUMP_BLK_T *prev_ptr = NULL;
   WIFIHDMI_DATA_PUMP_BLK_T *curr_ptr = NULL;

   vcos_mutex_lock ( &(instance->data_lock) );

   curr_ptr = instance->data_list;
   while ( curr_ptr != NULL )
   {
      if ( curr_ptr == *data )
      {
         if ( curr_ptr == instance->data_list )
         {
            instance->data_list = curr_ptr->next;
         }
         else
         {
            prev_ptr->next = curr_ptr->next;
         }

         vc_sm_free ( (*data)->handle );
         vcos_free( *data );
         *data = NULL;

         vcos_mutex_unlock ( &(instance->data_lock) );
         return;
      }
      else
      {
         prev_ptr = curr_ptr;
         curr_ptr = curr_ptr->next;
      }
   }

   /* We should free the memory anyway to avoid a memory leak.
   */
   vc_sm_free ( (*data)->handle );
   vcos_free( *data );
   *data = NULL;

   vcos_mutex_unlock ( &(instance->data_lock) );
}

static WIFIHDMI_CMD_RSP_BLK_T * vc_vchi_rsp_from_tid( WIFIHDMI_INSTANCE_T *instance,
                                                      uint32_t trans_id )
{
   WIFIHDMI_CMD_RSP_BLK_T *blk = NULL;

   if ( instance == NULL )
   {
      goto out;
   }

   if ( trans_id == 0 )
   {
      goto out;
   }

   vcos_mutex_lock ( &(instance->rsp_lock) );

   /* Lookup the resource.
   **
   ** Linear search for now, could be improved.
   */
   blk = instance->rsp_list;
   while ( 1 )
   {
      if ( blk == NULL )
      {
         break;
      }

      if ( blk->trans_id == trans_id )
      {
         vcos_mutex_unlock ( &(instance->rsp_lock) );
         return blk;
      }

      blk = blk->next;
   }

   vcos_mutex_unlock ( &(instance->rsp_lock) );

out:
   return NULL;
}

static WIFIHDMI_CMD_RSP_BLK_T * vc_vchi_rsp_fast_from_tid( WIFIHDMI_INSTANCE_T *instance,
                                                           uint32_t trans_id )
{
   WIFIHDMI_CMD_RSP_BLK_T *blk = NULL;

   if ( instance == NULL )
   {
      goto out;
   }

   if ( trans_id == 0 )
   {
      goto out;
   }

   vcos_mutex_lock ( &(instance->rsp_fast_lock) );

   /* Lookup the resource.
   **
   ** Linear search for now, could be improved.
   */
   blk = instance->rsp_fast_list;
   while ( 1 )
   {
      if ( blk == NULL )
      {
         break;
      }

      if ( blk->trans_id == trans_id )
      {
         vcos_mutex_unlock ( &(instance->rsp_fast_lock) );
         return blk;
      }

      blk = blk->next;
   }

   vcos_mutex_unlock ( &(instance->rsp_fast_lock) );

out:
   return NULL;
}

static WIFIHDMI_CMD_RSP_BLK_T * vc_vchi_cmd_top( WIFIHDMI_INSTANCE_T *instance )
{
   WIFIHDMI_CMD_RSP_BLK_T *blk = NULL;

   if ( instance == NULL )
   {
      goto out;
   }

   vcos_mutex_lock ( &(instance->cmd_lock) );
   blk = instance->cmd_list;
   vcos_mutex_unlock ( &(instance->cmd_lock) );

   return blk;

out:
   return NULL;
}

static WIFIHDMI_CMD_RSP_BLK_T * vc_vchi_cmd_fast_top( WIFIHDMI_INSTANCE_T *instance )
{
   WIFIHDMI_CMD_RSP_BLK_T *blk = NULL;

   if ( instance == NULL )
   {
      goto out;
   }

   vcos_mutex_lock ( &(instance->cmd_fast_lock) );
   blk = instance->cmd_fast_list;
   vcos_mutex_unlock ( &(instance->cmd_fast_lock) );

   return blk;

out:
   return NULL;
}

static void vc_vchi_wifihdmi_socket_callback( WHDMI_EVENT event,
                                              WHDMI_EVENT_PARAM *param,
                                              void *arg )
{
   WIFIHDMI_INSTANCE_T *instance = (WIFIHDMI_INSTANCE_T *)arg;
   uint32_t trans_id;
   VCOS_STATUS_T status;

   switch ( event )
   {
      case WHDMI_EVENT_SOCKET_INCOMING:
      {
         VC_WIFIHDMI_SKT_RES_T skt_res;
         VC_WIFIHDMI_SKT_T skt;
         WHDMI_EVENT_SOCKET_INCOMING_PARAM *ptr = (WHDMI_EVENT_SOCKET_INCOMING_PARAM *) param;

         memset ( &skt_res, 0, sizeof(skt_res) );
         memset ( &skt, 0, sizeof(skt) );

         skt.handle  = (uint32_t) ptr->parent_km_socket_handle;
         skt.address = ptr->client_addr;
         skt.port    = ptr->client_port;

         ptr->km_socket_handle_returned = 0;

         LOG_INFO( "%s: incoming connection %x from %x:%u",
                   __func__, skt.handle, skt.address, skt.port );
         
         if ( vc_vchi_wifihdmi_skt_in( instance,
                                       &skt,
                                       &skt_res,
                                       &trans_id ) == VCOS_SUCCESS )
         {
            if ( skt_res.success == 0 )
            {
               ptr->km_socket_handle_returned = skt_res.handle; 

               LOG_INFO( "%s: accepted socket %x, parent %x",
                         __func__, skt_res.handle, skt.handle );

            }
            else
            {
               LOG_ERR( "%s: failed to assign incoming handle %x, ret %d",
                        __func__, skt.handle, skt_res.success );
            }
         }
         else
         {
            LOG_ERR( "%s: failed to register incoming connection %x",
                     __func__, skt.handle );
         }
      }
      break;

      case WHDMI_EVENT_SOCKET_DISCONNECTED:
      {
         VC_WIFIHDMI_RESULT_T result;
         VC_WIFIHDMI_SKT_T skt;
         WHDMI_EVENT_SOCKET_DISCONNECTED_PARAM *ptr = (WHDMI_EVENT_SOCKET_DISCONNECTED_PARAM *) param;

         memset ( &skt, 0, sizeof(skt) );
         memset ( &result, 0, sizeof(result) );

         skt.handle  = (uint32_t) ptr->km_socket_handle;

         LOG_INFO( "%s: disconnected connection %x",
                   __func__, skt.handle );

         vc_vchi_wifihdmi_skt_dsc( instance,
                                   &skt,
                                   &result,
                                   &trans_id ); 
      }
      break;

      case WHDMI_EVENT_SOCKET_DATA_AVAIL:
      {
         long unsigned int data_ptr = 0;
         VC_WIFIHDMI_RESULT_T result;
         VC_WIFIHDMI_SKT_DATA_T skt_data;
         WHDMI_EVENT_SOCKET_DATA_AVAIL_PARAM *ptr = (WHDMI_EVENT_SOCKET_DATA_AVAIL_PARAM *) param;

         memset ( &skt_data, 0, sizeof(skt_data) );
         memset ( &result, 0, sizeof(result) );

         skt_data.handle    = (uint32_t) ptr->km_socket_handle;
         skt_data.data_len  = ptr->data_len;

         //LOG_DBG( "%s: data on connection %x, %u bytes",
         //         __func__, skt_data.handle, skt_data.data_len );

         if ( skt_data.data_len < VC_WIFIHDMI_MAX_DATA_LEN )
         {
            data_ptr = 0;
            if ( vc_sm_lock( (int) instance->data_in_handle,
                             VC_SM_LOCK_NON_CACHED,
                             &data_ptr ) == 0 )
            {
               if ( data_ptr == 0 )
               {
                  LOG_ERR( "%s: rx %d bytes from skt handle %x, failed forwarding, no mapping...",
                           __func__, skt_data.data_len, skt_data.handle );

                  vc_sm_unlock( (int) instance->data_in_handle,
                                0 );
               }
               else if ( (int)ptr->data < TASK_SIZE ) /* data from user-space. */
               {
                  if ( copy_from_user( (void *) data_ptr,
                                       (void __user *) ptr->data,
                                       ptr->data_len ) == 0 )
                  {
                     vc_sm_unlock( (int) instance->data_in_handle,
                                   0 );

                     skt_data.data_handle =
                           vc_sm_int_handle( instance->data_in_handle );

                     status = vc_vchi_wifihdmi_skt_data( instance,
                                                         &skt_data,
                                                         &result,
                                                         &trans_id );
                     if ( status != VCOS_SUCCESS )
                     {
                        /* What to do...  we are not able to pass the data to the
                        ** actual consumer, should we retry, abandon? ... will depend
                        ** on consequence(s), right now do nothing as it is always more
                        ** relaxing to do so...
                        */
                     }
                  }
                  else
                  {
                     vc_sm_unlock( (int) instance->data_in_handle,
                                   0 );

                     LOG_ERR( "%s: rx %d bytes from skt handle %x, failed copy...",
                              __func__, skt_data.data_len, skt_data.handle );
                  }
               }
               else /* data from within kernel. */
               {
                  memcpy ( (void *) data_ptr,
                           ptr->data,
                           ptr->data_len );

                  vc_sm_unlock( (int) instance->data_in_handle,
                                0 );

                  skt_data.data_handle =
                        vc_sm_int_handle( instance->data_in_handle );

                  status = vc_vchi_wifihdmi_skt_data( instance,
                                                      &skt_data,
                                                      &result,
                                                      &trans_id );
                  if ( status != VCOS_SUCCESS )
                  {
                     /* What to do...  we are not able to pass the data to the
                     ** actual consumer, should we retry, abandon? ... will depend
                     ** on consequence(s), right now do nothing as it is always more
                     ** relaxing to do so...
                     */
                  }
               }
            }
         }
         else
         {
            LOG_ERR( "%s: rx %d bytes from skt handle %x, max %d bytes, dropping...",
                     __func__, skt_data.data_len, skt_data.handle, VC_WIFIHDMI_MAX_DATA_LEN );
         }
      }
      break;

      case WHDMI_EVENT_SOCKET_CLOSED:
      {
         VC_WIFIHDMI_RESULT_T result;
         VC_WIFIHDMI_SKT_T skt;
         WHDMI_EVENT_SOCKET_CLOSED_PARAM *ptr = (WHDMI_EVENT_SOCKET_CLOSED_PARAM *) param;

         memset ( &skt, 0, sizeof(skt) );
         memset ( &result, 0, sizeof(result) );

         skt.handle  = (uint32_t) ptr->km_socket_handle;

         LOG_INFO( "%s: socket closed %x",
                   __func__, skt.handle );

         vc_vchi_wifihdmi_skt_end( instance,
                                   &skt,
                                   &result,
                                   &trans_id ); 
      }
      break;

      case WHDMI_EVENT_START_SERVICE:
      {
         VC_WIFIHDMI_RESULT_T result;
         VC_WIFIHDMI_MODE_T mode;

         if ( !instance->pool_init )
         {
            vc_vchi_wifihdmi_tx_pool( instance,
                                      SMEM_POOL_DEPTH,
                                      SMEM_POOL_SIZE );
            
            instance->pool_init = 1;
         }

         memset ( &mode, 0, sizeof(mode) );
         memset ( &result, 0, sizeof(result) );

         mode.wifihdmi = 1;

         hdmi_set_wifi_hdmi ( 1 );

         vc_vchi_wifihdmi_start( instance,
                                 &mode, 
                                 &result,
                                 &trans_id );
      }
      break;

      case WHDMI_EVENT_STOP_SERVICE:
      {
         VC_WIFIHDMI_RESULT_T result;
         VC_WIFIHDMI_MODE_T mode;

         memset ( &mode, 0, sizeof(mode) );
         memset ( &result, 0, sizeof(result) );

         mode.wifihdmi = 1;

         hdmi_set_wifi_hdmi ( 0 );

         vc_vchi_wifihdmi_stop( instance,
                                &mode, 
                                &result,
                                &trans_id );
      }
      break;

      case WHDMI_EVENT_AUDIO_STREAM_STATUS:
      {
         VC_WIFIHDMI_STR_STA_RES_T result;
         VC_WIFIHDMI_STREAM_T stream;
         WHDMI_EVENT_AUDIO_STREAM_STATUS_PARAM *ptr = (WHDMI_EVENT_AUDIO_STREAM_STATUS_PARAM *) param;

         vc_vchi_wifihdmi_audio_status( instance,
                                        &stream,
                                        &result,
                                        &trans_id );

         ptr->enabled = (int)result.enabled;
      }
      break;

      default:
      break;
   }
}

static void *vc_vchi_wifihdmi_videocore_ctrl( void *arg )
{
   WIFIHDMI_INSTANCE_T *instance = (WIFIHDMI_INSTANCE_T *)arg;
   WIFIHDMI_CTRL_BLK_T *ctrlblk;
   int rc = 0;

   while ( 1 )
   {
      ctrlblk = vc_vchi_deq_ctrl( instance );

      if ( ctrlblk == NULL )
      {
         vcos_event_wait( &instance->ctrl_event );
      }
      else
      {
         switch ( ctrlblk->action )
         {
            case VC_WIFIHDMI_MSG_TYPE_SKT_OPEN:
            {
               if ( ctrlblk->sendonly )
               {
                  rc = whdmi_create_udp_tx_socket( (int) ctrlblk->handle,
                                                   (unsigned short) ctrlblk->port,
                                                   0 );
               }
               else
               {
                  rc = whdmi_create_udp_socket( (int) ctrlblk->handle,
                                                (unsigned short) ctrlblk->port,
                                                0 );
               }

               LOG_INFO( "%s: open-socket %p, handle %x, port %d - returns %d",
                         __func__, ctrlblk, ctrlblk->handle, ctrlblk->port, rc );

               if ( rc != 0 )
               {
                  /* Handle socket operation failure appropriately here.
                  */
               }
            }
            break;

            case VC_WIFIHDMI_MSG_TYPE_SKT_CLOSE:
            {
               rc = whdmi_close_socket ( (int) ctrlblk->handle );

               LOG_INFO( "%s: close-socket %p, handle %x - returns %d",
                         __func__, ctrlblk, ctrlblk->handle, rc );
            }
            break;

            case VC_WIFIHDMI_MSG_TYPE_SKT_LISTEN:
            {
               rc = whdmi_create_tcp_listening_socket ( (int) ctrlblk->handle,
                                                        (unsigned short) ctrlblk->port,
                                                        0 );

               LOG_INFO( "%s: listen-socket %p, handle %x, port %d - returns %d",
                         __func__, ctrlblk, ctrlblk->handle, ctrlblk->port, rc );
            }
            break;

            default:
            break;
         }

         vcos_free ( ctrlblk );
         ctrlblk = NULL;
      }
   }

   return NULL;
}

static void *vc_vchi_wifihdmi_videocore_snd( void *arg )
{
   WIFIHDMI_INSTANCE_T *instance = (WIFIHDMI_INSTANCE_T *)arg;
   WIFIHDMI_SND_BLK_T *sndblk;
   VC_WIFIHDMI_REC_T rec;
   VC_WIFIHDMI_RESULT_T result;
   uint32_t trans_id;
   long unsigned int data_ptr;
   int rc;

   while ( 1 )
   {
      sndblk = vc_vchi_deq_snd( instance );

      if ( sndblk == NULL )
      {
         vcos_event_wait( &instance->snd_event );
      }
      else
      {
         rc = 0;
         data_ptr = 0;

         if ( vc_sm_lock( sndblk->handle,
                          VC_SM_LOCK_NON_CACHED,
                          &data_ptr ) == 0 )
         {
            if ( data_ptr == 0 )
            {
               LOG_ERR( "%s: sender-block %p, handle %x, no valid mapping...",
                        __func__,
                        sndblk,
                        sndblk->handle );
            }
            else
            {
               if ( (sndblk->address == 0) &&
                    (sndblk->port == 0) )
               {
                  LOG_DBG( "%s: control de-queue %p, handle %x, size %d, @ 0x%lx",
                           __func__,
                           sndblk,
                           sndblk->handle,
                           sndblk->size,
                           data_ptr );

                  rc = whdmi_tcp_send( sndblk->socket,
                                       sndblk->size,
                                       (uint8_t *) data_ptr );

                  LOG_DBG( "%s: control de-queue %p - returns %d",
                           __func__,
                           sndblk,
                           rc );
               }
               else
               {
                  LOG_DBG( "%s: data-pump de-queue %p, handle %x, size %d, @ 0x%lx, to %x:%d",
                           __func__,
                           sndblk,
                           sndblk->handle,
                           sndblk->size,
                           data_ptr,
                           sndblk->address,
                           sndblk->port );

                  rc = whdmi_udp_send_to( sndblk->socket,
                                          sndblk->address,
                                          sndblk->port,
                                          sndblk->size,
                                          (uint8_t *) data_ptr );

                  LOG_DBG( "%s: data-pump de-queue %p - returns %d",
                           __func__,
                           sndblk,
                           rc );
               }
            }

            vc_sm_unlock( sndblk->handle,
                          0 );
         }

         memset ( &rec, 0, sizeof(rec) );
         memset ( &result, 0, sizeof(result) );

         rec.res_handle = sndblk->handle;

         vcos_free ( sndblk );
         sndblk = NULL;

         vc_vchi_wifihdmi_rec( instance,
                               &rec,
                               &result,
                               &trans_id );

      }
   }

   return NULL;
}

static void *vc_vchi_wifihdmi_videocore_io( void *arg )
{
   uint32_t event_mask;
   WIFIHDMI_INSTANCE_T *instance = (WIFIHDMI_INSTANCE_T *)arg;
   int32_t success;
   uint32_t msg_len;
   VC_WIFIHDMI_RESULT_T *result;
   uint8_t reply_msg_buf[VC_WIFIHDMI_MAX_RSP_LEN];
   uint8_t ntfy_msg_buf[VC_WIFIHDMI_MAX_MSG_LEN];
   WIFIHDMI_CMD_RSP_BLK_T *waiter = NULL;
   VCOS_STATUS_T status;
   VC_WIFIHDMI_MSG_HDR_T *msg_hdr;


   while ( 1 )
   {
      event_mask = 0;

      status = vcos_event_wait( &instance->io_event );
      if ( status == VCOS_SUCCESS )
      {
         if ( vcos_mutex_lock ( &instance->io_lock ) == VCOS_SUCCESS )
         {
            event_mask = instance->io_event_mask;
            instance->io_event_mask = 0;
            vcos_mutex_unlock ( &instance->io_lock );
         }
         else
         {
            LOG_ERR( "%s: failed on io-lock",
                     __func__ );
         }
      }

      vchi_service_use( instance->vchi_server[0] );

      if ( event_mask & CLIENT_EVENT_MASK )
      {
         waiter = vc_vchi_cmd_top( instance );
         while ( waiter != NULL )
         {
            vc_vchi_unlink_cmd( instance,
                                waiter );

            success = vchi_msg_queue( instance->vchi_server[0],
                                      waiter->command_msg_buf,
                                      waiter->command_len,
                                      VCHI_FLAGS_BLOCK_UNTIL_QUEUED,
                                      NULL );
            if ( success != 0 )
            {
               LOG_ERR( "%s: failed to queue message (success=%d)",
                        __func__, success );
               vcos_event_signal ( &waiter->cmd_resp );
            }
            else
            {
               if ( waiter->reply_wait )
               {
                  vc_vchi_add_rsp( instance,
                                   waiter );
               }
               else
               {
                  vcos_event_signal ( &waiter->cmd_resp );
               }
            }

            waiter = vc_vchi_cmd_top( instance );
         }
      }

      if ( event_mask & SERVER_EVENT_MASK )
      {
         success = vchi_msg_dequeue( instance->vchi_server[0],
                                     reply_msg_buf,
                                     sizeof( reply_msg_buf ),
                                     &msg_len,
                                     VCHI_FLAGS_NONE );

         if ( success != 0 )
         {
            LOG_ERR( "%s: failed to dequeue message (success=%d)",
                     __func__,
                     success );
         }
         else
         {
            result = (VC_WIFIHDMI_RESULT_T *) reply_msg_buf;
            waiter = vc_vchi_rsp_from_tid ( instance,
                                            result->trans_id );

            if ( waiter == NULL )
            {
               LOG_DBG( "%s: received response for transaction %u, throw away...",
                        __func__,
                        result->trans_id );
            }
            else
            {
               memcpy ( waiter->reply_msg_buf,
                        reply_msg_buf,
                        sizeof ( reply_msg_buf ) );

               vcos_event_signal ( &waiter->cmd_resp );
            }
         }
      }

      vchi_service_release( instance->vchi_server[0] );
      vchi_service_use( instance->vchi_notifier_ctrl[0] );

      if ( event_mask & NOTIFY_EVENT_MASK )
      {
         success = vchi_msg_dequeue( instance->vchi_notifier_ctrl[0],
                                     ntfy_msg_buf,
                                     sizeof( ntfy_msg_buf ),
                                     &msg_len,
                                     VCHI_FLAGS_NONE );
         while ( success == 0 )
         {
            msg_hdr = (VC_WIFIHDMI_MSG_HDR_T *)ntfy_msg_buf;
            if ( msg_len < sizeof( VC_WIFIHDMI_MSG_HDR_T ))
            {
               vcos_assert( msg_len >= sizeof( VC_WIFIHDMI_MSG_HDR_T ));
            }
            if ( msg_hdr->type >= VC_WIFIHDMI_MSG_TYPE_MAX )
            {
               vcos_assert( msg_hdr->type < VC_WIFIHDMI_MSG_TYPE_MAX );
            }

            switch ( msg_hdr->type )
            {
               case VC_WIFIHDMI_MSG_TYPE_SKT_OPEN:
               case VC_WIFIHDMI_MSG_TYPE_SKT_CLOSE:
               case VC_WIFIHDMI_MSG_TYPE_SKT_LISTEN:
                  if ( msg_len ==
                         ( sizeof( VC_WIFIHDMI_MSG_HDR_T ) + sizeof( VC_WIFIHDMI_SKT_ACTION_T )) )
                  {
                     VC_WIFIHDMI_SKT_ACTION_T *sktaction = (VC_WIFIHDMI_SKT_ACTION_T *)msg_hdr->body;
                     WIFIHDMI_CTRL_BLK_T *ctrlblk = vcos_malloc( sizeof( *ctrlblk ), "" );

                     if ( ctrlblk != NULL )
                     {
                        ctrlblk->action   = msg_hdr->type;
                        ctrlblk->handle   = sktaction->socket_handle;
                        ctrlblk->port     = sktaction->socket_port;
                        ctrlblk->sendonly = sktaction->socket_send_only;

                        vc_vchi_add_ctrl( instance,
                                          ctrlblk );
                        vcos_event_signal ( &instance->ctrl_event );
                     }
                  }
               break;

               /****** DEBUG CODE ******/
               case VC_WIFIHDMI_MSG_TYPE_FRAME_TOGGLE:
               {
                  /* To help measure the frame rate and latency seen on the wifi/hdmi link
                  ** we toggle the led located at gpio 0 (red) (1160 big-island tablet).
                  */
                  instance->gpio_toggle = (instance->gpio_toggle == 0) ? 1 : 0;
                  
                  /* gpio_direction_output( 0, 1 ); */
                  gpio_set_value ( 0, instance->gpio_toggle );

                  instance->gpio_toggle_cnt++;
               }
               break;
               /****** DEBUG CODE ******/

               default:
               break;
            }

            success = vchi_msg_dequeue( instance->vchi_notifier_ctrl[0],
                                        ntfy_msg_buf,
                                        sizeof( ntfy_msg_buf ),
                                        &msg_len,
                                        VCHI_FLAGS_NONE );
         }
      }

      vchi_service_release( instance->vchi_notifier_ctrl[0] );
   }

   return NULL;
}

static void *vc_vchi_wifihdmi_videocore_iof( void *arg )
{
   uint32_t event_mask;
   WIFIHDMI_INSTANCE_T *instance = (WIFIHDMI_INSTANCE_T *)arg;
   int32_t success;
   uint32_t msg_len;
   VC_WIFIHDMI_RESULT_T *result;
   uint8_t reply_msg_buf[VC_WIFIHDMI_MAX_RSP_LEN];
   WIFIHDMI_CMD_RSP_BLK_T *waiter = NULL;
   VCOS_STATUS_T status;


   while ( 1 )
   {
      event_mask = 0;

      status = vcos_event_wait( &instance->iof_event );
      if ( status == VCOS_SUCCESS )
      {
         if ( vcos_mutex_lock ( &instance->iof_lock ) == VCOS_SUCCESS )
         {
            event_mask = instance->iof_event_mask;
            instance->iof_event_mask = 0;
            vcos_mutex_unlock ( &instance->iof_lock );
         }
         else
         {
            LOG_ERR( "%s: failed on io-lock",
                     __func__ );
         }
      }
      vchi_service_use( instance->vchi_srv_fast[0] );

      if ( event_mask & CLIENT_EVENT_MASK )
      {
         waiter = vc_vchi_cmd_fast_top( instance );
         while ( waiter != NULL )
         {
            vc_vchi_unlink_fast_cmd( instance,
                                     waiter );

            success = vchi_msg_queue( instance->vchi_srv_fast[0],
                                      waiter->command_msg_buf,
                                      waiter->command_len,
                                      VCHI_FLAGS_BLOCK_UNTIL_QUEUED,
                                      NULL );
            if ( success != 0 )
            {
               LOG_ERR( "%s: failed to queue message (success=%d)",
                        __func__, success );
               vcos_event_signal ( &waiter->cmd_resp );
            }
            else
            {
               if ( waiter->reply_wait )
               {
                  vc_vchi_add_fast_rsp( instance,
                                        waiter );
               }
               else
               {
                  vcos_event_signal ( &waiter->cmd_resp );
               }
            }

            waiter = vc_vchi_cmd_fast_top( instance );
         }
      }

      if ( event_mask & SERVER_EVENT_MASK )
      {
         success = vchi_msg_dequeue( instance->vchi_srv_fast[0],
                                     reply_msg_buf,
                                     sizeof( reply_msg_buf ),
                                     &msg_len,
                                     VCHI_FLAGS_NONE );

         if ( success != 0 )
         {
            LOG_ERR( "%s: failed to dequeue message (success=%d)",
                     __func__,
                     success );
         }
         else
         {
            result = (VC_WIFIHDMI_RESULT_T *) reply_msg_buf;
            waiter = vc_vchi_rsp_fast_from_tid ( instance,
                                                 result->trans_id );

            if ( waiter == NULL )
            {
               LOG_DBG( "%s: received response for transaction %u, throw away...",
                        __func__,
                        result->trans_id );
            }
            else
            {
               memcpy ( waiter->reply_msg_buf,
                        reply_msg_buf,
                        sizeof ( reply_msg_buf ) );

               vcos_event_signal ( &waiter->cmd_resp );
            }
         }
      }

      vchi_service_release( instance->vchi_srv_fast[0] );
   }

   return NULL;
}

static void *vc_vchi_wifihdmi_videocore_dt( void *arg )
{
   uint32_t event_mask;
   WIFIHDMI_INSTANCE_T *instance = (WIFIHDMI_INSTANCE_T *)arg;
   int32_t success;
   uint32_t msg_len;
   uint8_t ntfy_msg_buf[VC_WIFIHDMI_MAX_MSG_LEN];
   VCOS_STATUS_T status;
   VC_WIFIHDMI_MSG_HDR_T *msg_hdr;


   while ( 1 )
   {
      event_mask = 0;

      status = vcos_event_wait( &instance->dt_event );
      if ( status == VCOS_SUCCESS )
      {
         if ( vcos_mutex_lock ( &instance->dt_lock ) == VCOS_SUCCESS )
         {
            event_mask = instance->dt_event_mask;
            instance->dt_event_mask = 0;
            vcos_mutex_unlock ( &instance->dt_lock );
         }
         else
         {
            LOG_ERR( "%s: failed on dt-lock",
                     __func__ );
         }
      }

      vchi_service_use( instance->vchi_notifier_data[0] );

      if ( event_mask & NOTIFY_EVENT_MASK )
      {
         success = vchi_msg_dequeue( instance->vchi_notifier_data[0],
                                     ntfy_msg_buf,
                                     sizeof( ntfy_msg_buf ),
                                     &msg_len,
                                     VCHI_FLAGS_NONE );
         while ( success == 0 )
         {
            msg_hdr = (VC_WIFIHDMI_MSG_HDR_T *)ntfy_msg_buf;
            if ( msg_len < sizeof( VC_WIFIHDMI_MSG_HDR_T ))
            {
               vcos_assert( msg_len >= sizeof( VC_WIFIHDMI_MSG_HDR_T ));
            }
            if ( msg_hdr->type >= VC_WIFIHDMI_MSG_TYPE_MAX )
            {
               vcos_assert( msg_hdr->type < VC_WIFIHDMI_MSG_TYPE_MAX );
            }

            switch ( msg_hdr->type )
            {
               case VC_WIFIHDMI_MSG_TYPE_TX_DATA:
                  if ( msg_len ==
                         ( sizeof( VC_WIFIHDMI_MSG_HDR_T ) + sizeof( VC_WIFIHDMI_TX_DATA_T )) )
                  {
                     VC_WIFIHDMI_TX_DATA_T *txdata = (VC_WIFIHDMI_TX_DATA_T *)msg_hdr->body;
                     WIFIHDMI_SND_BLK_T *sndblk = vcos_malloc( sizeof( *sndblk ), "" );

                     if ( sndblk != NULL )
                     {
                        sndblk->handle  = txdata->res_handle;
                        sndblk->size    = txdata->res_size;
                        sndblk->socket  = txdata->res_socket;
                        sndblk->address = txdata->res_rem_addr;
                        sndblk->port    = txdata->res_rem_port;

                        //LOG_DBG( "%s: data-pump queue %p, handle %x, size %d",
                        //         __func__, sndblk, sndblk->handle, sndblk->size );

                        vc_vchi_add_snd( instance,
                                         sndblk );
                        vcos_event_signal ( &instance->snd_event );
                     }
                  }
               break;

               default:
               break;
            }

            success = vchi_msg_dequeue( instance->vchi_notifier_data[0],
                                        ntfy_msg_buf,
                                        sizeof( ntfy_msg_buf ),
                                        &msg_len,
                                        VCHI_FLAGS_NONE );
         }
      }

      vchi_service_release( instance->vchi_notifier_data[0] );
   }

   return NULL;
}

static void vc_wifihdmi_server_callback( void *param,
                                         const VCHI_CALLBACK_REASON_T reason,
                                         void *msg_handle )
{
   WIFIHDMI_INSTANCE_T *instance = (WIFIHDMI_INSTANCE_T *)param;

   (void)msg_handle;

   switch( reason )
   {
      /* Message is available, de-queue it, process the response according to the
      ** waiter.
      */
      case VCHI_CALLBACK_MSG_AVAILABLE:
         if ( vcos_mutex_lock ( &instance->io_lock ) == VCOS_SUCCESS )
         {
            instance->io_event_mask |= SERVER_EVENT_MASK;
            vcos_mutex_unlock ( &instance->io_lock );
            vcos_event_signal( &instance->io_event );
         }
         else
         {
            LOG_ERR( "%s: failed on io-lock",
                     __func__ );
         }
      break;

      case VCHI_CALLBACK_SERVICE_CLOSED:
         LOG_INFO( "%s: service CLOSED!!",
                   __func__ );
      default:
      break;
   }
}

static void vc_wifihdmi_srv_fast_cb( void *param,
                                     const VCHI_CALLBACK_REASON_T reason,
                                     void *msg_handle )
{
   WIFIHDMI_INSTANCE_T *instance = (WIFIHDMI_INSTANCE_T *)param;

   (void)msg_handle;

   switch( reason )
   {
      /* Message is available, de-queue it, process the response according to the
      ** waiter.
      */
      case VCHI_CALLBACK_MSG_AVAILABLE:
         if ( vcos_mutex_lock ( &instance->iof_lock ) == VCOS_SUCCESS )
         {
            instance->iof_event_mask |= SERVER_EVENT_MASK;
            vcos_mutex_unlock ( &instance->iof_lock );
            vcos_event_signal( &instance->iof_event );
         }
         else
         {
            LOG_ERR( "%s: failed on io-lock",
                     __func__ );
         }
      break;

      case VCHI_CALLBACK_SERVICE_CLOSED:
         LOG_INFO( "%s: service CLOSED!!",
                   __func__ );
      default:
      break;
   }
}

static void vc_wifihdmi_notifier_ctrl_cb( void *param,
                                          const VCHI_CALLBACK_REASON_T reason,
                                          void *msg_handle )
{
   WIFIHDMI_INSTANCE_T *instance = (WIFIHDMI_INSTANCE_T *)param;

   (void)msg_handle;

   switch( reason )
   {
      /* Notification message is available from the service.
      */
      case VCHI_CALLBACK_MSG_AVAILABLE:
         if ( vcos_mutex_lock ( &instance->io_lock ) == VCOS_SUCCESS )
         {
            instance->io_event_mask |= NOTIFY_EVENT_MASK;
            vcos_mutex_unlock ( &instance->io_lock );
            vcos_event_signal( &instance->io_event );
         }
         else
         {
            LOG_ERR( "%s: failed on io-lock",
                     __func__ );
         }
      break;

      case VCHI_CALLBACK_SERVICE_CLOSED:
         LOG_INFO( "%s: service CLOSED!!",
                   __func__ );
      default:
      break;
   }
}

static void vc_wifihdmi_notifier_data_cb( void *param,
                                          const VCHI_CALLBACK_REASON_T reason,
                                          void *msg_handle )
{
   WIFIHDMI_INSTANCE_T *instance = (WIFIHDMI_INSTANCE_T *)param;

   (void)msg_handle;

   switch( reason )
   {
      /* Notification message is available from the service.
      */
      case VCHI_CALLBACK_MSG_AVAILABLE:
         if ( vcos_mutex_lock ( &instance->dt_lock ) == VCOS_SUCCESS )
         {
            instance->dt_event_mask |= NOTIFY_EVENT_MASK;
            vcos_mutex_unlock ( &instance->dt_lock );
            vcos_event_signal( &instance->dt_event );
         }
         else
         {
            LOG_ERR( "%s: failed on io-lock",
                     __func__ );
         }
      break;

      case VCHI_CALLBACK_SERVICE_CLOSED:
         LOG_INFO( "%s: service CLOSED!!",
                   __func__ );
      default:
      break;
   }
}

VC_VCHI_WIFIHDMI_HANDLE_T vc_vchi_wifihdmi_init( VCHI_INSTANCE_T vchi_instance,
                                                 VCHI_CONNECTION_T **vchi_connections,
                                                 uint32_t num_connections )
{
   uint32_t i;
   WIFIHDMI_INSTANCE_T *instance;
   VCOS_STATUS_T status;
   VCOS_THREAD_ATTR_T attrs;
   VC_SM_ALLOC_T alloc;
   int handle;

   // Set up the VCOS logging
   vcos_log_set_level( VCOS_LOG_CATEGORY, LOG_LEVEL );
   vcos_log_register( "wifihdmi", VCOS_LOG_CATEGORY );

   LOG_DBG( "%s: start", __func__ );

   if ( num_connections > VCHI_MAX_NUM_CONNECTIONS )
   {
      LOG_ERR( "%s: unsupported number of connections %u (max=%u)",
               __func__, num_connections, VCHI_MAX_NUM_CONNECTIONS );

      vcos_assert( num_connections <= VCHI_MAX_NUM_CONNECTIONS );
      goto err_null;
   }

   // Allocate memory for this instance
   instance = vcos_malloc( sizeof( *instance ), "wifihdmi_instance" );
   memset( instance, 0, sizeof( *instance ));

   instance->num_connections = num_connections;

   // Create a lock for exclusive, serialized VCHI connection access
   status = vcos_semaphore_create( &instance->vchi_sema, "wifihdmi_vchi_sem", 1 );
   if ( status != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed to create vchi-sema (status=%d)", __func__, status );
      goto err_free_mem;
   }

   // Create a lock for exclusive, serialized VCHI connection access (fast)
   status = vcos_semaphore_create( &instance->vchi_fast_sema, "wifihdmi_vchif_sem", 1 );
   if ( status != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed to create vchi-fast-sema (status=%d)", __func__, status );
      goto err_delete_vchi_sema;
   }

   // Lock for command processing
   status = vcos_mutex_create( &instance->cmd_lock, "wifihdmi_cmd_lock" );
   if ( status != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed to create cmd-lock (status=%d)", __func__, status );
      goto err_delete_vchi_fast_sema;
   }

   // Lock for command processing (fast)
   status = vcos_mutex_create( &instance->cmd_fast_lock, "wifihdmi_cmd_fast_lock" );
   if ( status != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed to create cmd-fast-lock (status=%d)", __func__, status );
      goto err_delete_cmd_lock;
   }

   // Lock for response processing
   status = vcos_mutex_create( &instance->rsp_lock, "wifihdmi_rsp_lock" );
   if ( status != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed to create rsp-lock (status=%d)", __func__, status );
      goto err_delete_cmd_fast_lock;
   }
   
   // Lock for response processing (fast)
   status = vcos_mutex_create( &instance->rsp_fast_lock, "wifihdmi_rsp_fast_lock" );
   if ( status != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed to create rsp-fast-lock (status=%d)", __func__, status );
      goto err_delete_rsp_lock;
   }

   // Lock for io events processing
   status = vcos_mutex_create( &instance->io_lock, "sm_io_lock" );
   if ( status != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed to create io-lock (status=%d)", __func__, status );
      goto err_delete_rsp_fast_lock;
   }

   // Event for io events processing
   status = vcos_event_create( &instance->io_event, "" );
   if ( status != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed to create io-event (status=%d)", __func__, status );
      goto err_delete_io_lock;
   }

   // Lock for send processing
   status = vcos_mutex_create( &instance->snd_lock, "wifihdmi_snd_lock" );
   if ( status != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed to create snd-lock (status=%d)", __func__, status );
      goto err_delete_io_event;
   }

   // Event for send events processing
   status = vcos_event_create( &instance->snd_event, "" );
   if ( status != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed to create snd-event (status=%d)", __func__, status );
      goto err_delete_snd_lock;
   }

   // Lock for control processing
   status = vcos_mutex_create( &instance->ctrl_lock, "wifihdmi_ctrl_lock" );
   if ( status != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed to create ctrl-lock (status=%d)", __func__, status );
      goto err_delete_snd_event;
   }

   // Event for control events processing
   status = vcos_event_create( &instance->ctrl_event, "" );
   if ( status != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed to create ctrl-event (status=%d)", __func__, status );
      goto err_delete_ctrl_lock;
   }

   // Lock for data events processing
   status = vcos_mutex_create( &instance->dt_lock, "sm_data_lock" );
   if ( status != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed to create io-lock (status=%d)", __func__, status );
      goto err_delete_ctrl_event;
   }

   // Event for data events processing
   status = vcos_event_create( &instance->dt_event, "" );
   if ( status != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed to create io-event (status=%d)", __func__, status );
      goto err_delete_dt_lock;
   }

   // Lock for io-fast events processing
   status = vcos_mutex_create( &instance->iof_lock, "sm_iof_lock" );
   if ( status != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed to create iof-lock (status=%d)", __func__, status );
      goto err_delete_dt_event;
   }

   // Event for io-fast events processing
   status = vcos_event_create( &instance->iof_event, "" );
   if ( status != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed to create iof-event (status=%d)", __func__, status );
      goto err_delete_iof_lock;
   }

   // Open the VCHI service connections
   for ( i = 0; i < num_connections; i++ )
   {
      SERVICE_CREATION_T server =
      {
         VC_WIFIHDMI_SERVER_NAME,     // 4cc service code
         vchi_connections[i],         // passed in fn pointers
         0,                           // rx fifo size
         0,                           // tx fifo size
         vc_wifihdmi_server_callback, // service callback
         instance,                    // service callback parameter
         VCOS_FALSE,                  // unaligned bulk recieves
         VCOS_FALSE,                  // unaligned bulk transmits
         VCOS_FALSE,                  // want crc check on bulk transfers
      };

      SERVICE_CREATION_T server_fast =
      {
         VC_WIFIHDMI_SERVER_FAST_NAME,// 4cc service code
         vchi_connections[i],         // passed in fn pointers
         0,                           // rx fifo size
         0,                           // tx fifo size
         vc_wifihdmi_srv_fast_cb,     // service callback
         instance,                    // service callback parameter
         VCOS_FALSE,                  // unaligned bulk recieves
         VCOS_FALSE,                  // unaligned bulk transmits
         VCOS_FALSE,                  // want crc check on bulk transfers
      };

      SERVICE_CREATION_T notifier_ctrl =
      {
         VC_WIFIHDMI_NOTIFY_CTRL_NAME,  // 4cc service code
         vchi_connections[i],           // passed in fn pointers
         0,                             // rx fifo size
         0,                             // tx fifo size
         vc_wifihdmi_notifier_ctrl_cb,  // service callback
         instance,                      // service callback parameter
         VCOS_FALSE,                    // unaligned bulk recieves
         VCOS_FALSE,                    // unaligned bulk transmits
         VCOS_FALSE,                    // want crc check on bulk transfers
      };

      SERVICE_CREATION_T notifier_data =
      {
         VC_WIFIHDMI_NOTIFY_DATA_NAME,  // 4cc service code
         vchi_connections[i],           // passed in fn pointers
         0,                             // rx fifo size
         0,                             // tx fifo size
         vc_wifihdmi_notifier_data_cb,  // service callback
         instance,                      // service callback parameter
         VCOS_FALSE,                    // unaligned bulk recieves
         VCOS_FALSE,                    // unaligned bulk transmits
         VCOS_FALSE,                    // want crc check on bulk transfers
      };

      status = vchi_service_open( vchi_instance,
                                  &server,
                                  &instance->vchi_server[i] );
      if ( status != VCOS_SUCCESS )
      {
         LOG_ERR( "%s: failed to open VCHI service server connection (status=%d)",
                  __func__, status );

         vcos_assert( status == VCOS_SUCCESS );
         goto err_close_services;
      }

      status = vchi_service_open( vchi_instance,
                                  &server_fast,
                                  &instance->vchi_srv_fast[i] );
      if ( status != VCOS_SUCCESS )
      {
         LOG_ERR( "%s: failed to open VCHI service server connection (status=%d)",
                  __func__, status );

         vcos_assert( status == VCOS_SUCCESS );
         goto err_close_services;
      }

      status = vchi_service_open( vchi_instance,
                                  &notifier_ctrl,
                                  &instance->vchi_notifier_ctrl[i] );
      if ( status != VCOS_SUCCESS )
      {
         LOG_ERR( "%s: failed to open VCHI service notifier-ctrl connection (status=%d)",
                  __func__, status );

         vcos_assert( status == VCOS_SUCCESS );
         goto err_close_services;
      }

      status = vchi_service_open( vchi_instance,
                                  &notifier_data,
                                  &instance->vchi_notifier_data[i] );
      if ( status != VCOS_SUCCESS )
      {
         LOG_ERR( "%s: failed to open VCHI service notifier-data connection (status=%d)",
                  __func__, status );

         vcos_assert( status == VCOS_SUCCESS );
         goto err_close_services;
      }

      vchi_service_release( instance->vchi_server[i] );
      vchi_service_release( instance->vchi_srv_fast[i] );
      vchi_service_release( instance->vchi_notifier_ctrl[i] );
      vchi_service_release( instance->vchi_notifier_data[i] );
   }

   // Create a thread to process the server command/responses
   vcos_thread_attr_init( &attrs );
   vcos_thread_attr_setpriority( &attrs,
                                 VCOS_THREAD_PRI_NORMAL );
   status = vcos_thread_create( &instance->io_thread,
                                "WH-SRV-IO",
                                &attrs,
                                vc_vchi_wifihdmi_videocore_io,
                                instance );
   if ( status != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed to create videocore io thread (status=%d)",
               __func__, status );

      vcos_assert( status == VCOS_SUCCESS );
      goto err_close_services;
   }

   // Create a thread to process the server (fast) command/responses
   vcos_thread_attr_init( &attrs );
   vcos_thread_attr_setpriority( &attrs,
                                 VCOS_THREAD_PRI_HIGHEST );
   status = vcos_thread_create( &instance->iof_thread,
                                "WH-SRV-FIO",
                                &attrs,
                                vc_vchi_wifihdmi_videocore_iof,
                                instance );
   if ( status != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed to create videocore iof thread (status=%d)",
               __func__, status );

      vcos_assert( status == VCOS_SUCCESS );
      goto err_close_services;
   }

   // Create a thread to process the server data notification
   vcos_thread_attr_init( &attrs );
   vcos_thread_attr_setpriority( &attrs,
                                 VCOS_THREAD_PRI_HIGHEST );
   status = vcos_thread_create( &instance->dt_thread,
                                "WH-DT-NOT",
                                &attrs,
                                vc_vchi_wifihdmi_videocore_dt,
                                instance );
   if ( status != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed to create videocore io thread (status=%d)",
               __func__, status );

      vcos_assert( status == VCOS_SUCCESS );
      goto err_close_services;
   }

   // Create a thread to process the data pump notifications
   vcos_thread_attr_init( &attrs );
   vcos_thread_attr_setpriority( &attrs,
                                 VCOS_THREAD_PRI_HIGHEST );
   status = vcos_thread_create( &instance->snd_thread,
                                "WH-DT-SND",
                                &attrs,
                                vc_vchi_wifihdmi_videocore_snd,
                                instance );
   if ( status != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed to create videocore snd thread (status=%d)",
               __func__, status );

      vcos_assert( status == VCOS_SUCCESS );
      goto err_close_services;
   }

   // Create a thread to process the socket control notifications
   vcos_thread_attr_init( &attrs );
   vcos_thread_attr_setpriority( &attrs,
                                 VCOS_THREAD_PRI_NORMAL );
   status = vcos_thread_create( &instance->ctrl_thread,
                                "WH-SKT-CTL",
                                &attrs,
                                vc_vchi_wifihdmi_videocore_ctrl,
                                instance );
   if ( status != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed to create videocore ctrl thread (status=%d)",
               __func__, status );

      vcos_assert( status == VCOS_SUCCESS );
      goto err_close_services;
   }

   // Create the shared memory region for incoming data passing.
   memset ( &alloc, 0, sizeof(alloc) );

   alloc.type       = VC_SM_ALLOC_NON_CACHED;
   alloc.base_unit  = VC_WIFIHDMI_MAX_DATA_LEN;
   alloc.num_unit   = 1;
   alloc.alignement = 4096;
   alloc.allocator  = 0;

   if ( vc_sm_alloc( &alloc, &handle ) != 0 )
   {
      vcos_assert( 0 );
      goto err_close_services;
   }
   instance->data_in_handle = (uint32_t) handle;

   // Lock for data-pump block reference
   status = vcos_mutex_create( &instance->data_lock, "wifihdmi_data_lock" );
   if ( status != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed to create data-lock (status=%d)", __func__, status );
      goto err_close_services;
   }

   // Hook up the callback for socket activity.
   whdmi_set_callback( (WHDMI_CALLBACK) &vc_vchi_wifihdmi_socket_callback,
                       (void*)instance );

   LOG_DBG( "%s: success - instance 0x%x, in-sm-hdl 0x%x",
            __func__,
            (unsigned)instance,
            (unsigned) handle );
   return instance;

err_close_services:
   for ( i = 0; i < instance->num_connections; i++ )
   {
      if ( instance->vchi_server[i] != NULL )
      {
         vchi_service_use( instance->vchi_server[i] );
         vchi_service_close( instance->vchi_server[i] );
      }

      if ( instance->vchi_srv_fast[i] != NULL )
      {
         vchi_service_use( instance->vchi_srv_fast[i] );
         vchi_service_close( instance->vchi_srv_fast[i] );
      }

      if ( instance->vchi_notifier_ctrl[i] != NULL )
      {
         vchi_service_use( instance->vchi_notifier_ctrl[i] );
         vchi_service_close( instance->vchi_notifier_ctrl[i] );
      }

      if ( instance->vchi_notifier_data[i] != NULL )
      {
         vchi_service_use( instance->vchi_notifier_data[i] );
         vchi_service_close( instance->vchi_notifier_data[i] );
      }
   }
   vcos_event_delete( &instance->iof_event );
err_delete_iof_lock:
   vcos_mutex_delete( &instance->iof_lock );
err_delete_dt_event:
   vcos_event_delete( &instance->dt_event );
err_delete_dt_lock:
   vcos_mutex_delete( &instance->data_lock );
err_delete_ctrl_event:
   vcos_event_delete( &instance->snd_event );
err_delete_ctrl_lock:
   vcos_mutex_delete( &instance->ctrl_lock );
err_delete_snd_event:
   vcos_event_delete( &instance->snd_event );
err_delete_snd_lock:
   vcos_mutex_delete( &instance->snd_lock );
err_delete_io_event:
   vcos_event_delete( &instance->io_event );
err_delete_io_lock:
   vcos_mutex_delete( &instance->io_lock );
err_delete_rsp_fast_lock:
   vcos_mutex_delete( &instance->rsp_fast_lock );
err_delete_rsp_lock:
   vcos_mutex_delete( &instance->rsp_lock );
err_delete_cmd_fast_lock:
   vcos_mutex_delete( &instance->cmd_fast_lock );
err_delete_cmd_lock:
   vcos_mutex_delete( &instance->cmd_lock );
err_delete_vchi_fast_sema:
   vcos_semaphore_delete( &instance->vchi_fast_sema );
err_delete_vchi_sema:
   vcos_semaphore_delete( &instance->vchi_sema );
err_free_mem:
   vcos_free( instance );
err_null:
   LOG_ERR( "%s: FAILED", __func__ );
   return NULL;
}
EXPORT_SYMBOL_GPL(vc_vchi_wifihdmi_init);

VCOS_STATUS_T vc_vchi_wifihdmi_end( VC_VCHI_WIFIHDMI_HANDLE_T *handle )
{
   WIFIHDMI_INSTANCE_T *instance = *handle;
   VCOS_STATUS_T status;
   uint32_t i;
   WIFIHDMI_DATA_PUMP_BLK_T *blk_data = NULL;

   if ( handle == NULL )
   {
      LOG_ERR( "%s: invalid pointer to handle %p", __func__, handle );

      vcos_assert( handle != NULL );
      goto lock;
   }

   if ( *handle == NULL )
   {
      LOG_ERR( "%s: invalid handle %p", __func__, *handle );

      vcos_assert( *handle != NULL );
      goto lock;
   }

   if ( (status = vcos_semaphore_wait( &instance->vchi_sema )) != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed to wait on vchi-sema (status=%d)", __func__, status );
      goto lock;
   }

   // Close all VCHI service connections
   for ( i = 0; i < instance->num_connections; i++ )
   {
      int32_t success;

      if ( instance->vchi_server[i] )
      {
         vchi_service_use( instance->vchi_server[i] );
         success = vchi_service_close( instance->vchi_server[i] );
         vcos_assert( success == 0 );
      }

      if ( instance->vchi_srv_fast[i] )
      {
         vchi_service_use( instance->vchi_srv_fast[i] );
         success = vchi_service_close( instance->vchi_srv_fast[i] );
         vcos_assert( success == 0 );
      }

      if ( instance->vchi_notifier_ctrl[i] )
      {
         vchi_service_use( instance->vchi_notifier_ctrl[i] );
         success = vchi_service_close( instance->vchi_notifier_ctrl[i] );
         vcos_assert( success == 0 );
      }

      if ( instance->vchi_notifier_data[i] )
      {
         vchi_service_use( instance->vchi_notifier_data[i] );
         success = vchi_service_close( instance->vchi_notifier_data[i] );
         vcos_assert( success == 0 );
      }
   }

   blk_data = instance->data_list;
   while ( blk_data != NULL )
   {
      vc_vchi_rem_data ( instance,
                         &blk_data );
      blk_data = instance->data_list;
   };

   vcos_semaphore_post( &instance->vchi_sema );
   vcos_semaphore_delete( &instance->vchi_sema );

   vcos_semaphore_post( &instance->vchi_fast_sema );
   vcos_semaphore_delete( &instance->vchi_fast_sema );

   vcos_event_delete( &instance->io_event );
   vcos_event_delete( &instance->iof_event );
   vcos_event_delete( &instance->snd_event );
   vcos_event_delete( &instance->ctrl_event );
   vcos_event_delete( &instance->dt_event );

   vcos_mutex_delete( &instance->cmd_lock );
   vcos_mutex_delete( &instance->cmd_fast_lock );
   vcos_mutex_delete( &instance->rsp_lock );
   vcos_mutex_delete( &instance->rsp_fast_lock );
   vcos_mutex_delete( &instance->io_lock );
   vcos_mutex_delete( &instance->iof_lock );
   vcos_mutex_delete( &instance->dt_lock );
   vcos_mutex_delete( &instance->snd_lock );
   vcos_mutex_delete( &instance->ctrl_lock );
   vcos_mutex_delete( &instance->data_lock );
   vcos_free( instance );

   // NULLify the handle to prevent the user from using it
   *handle = NULL;

   // Unregister the log category so we can add it back next time
   vcos_log_unregister( &wifihdmi_log_category );

   return VCOS_SUCCESS;

lock:
   return VCOS_EINVAL;
}
EXPORT_SYMBOL_GPL(vc_vchi_wifihdmi_end);

VCOS_STATUS_T vc_vchi_wifihdmi_set( VC_VCHI_WIFIHDMI_HANDLE_T handle,
                                    VC_WIFIHDMI_SET_T *set,
                                    VC_WIFIHDMI_RESULT_T *result,
                                    uint32_t *cur_trans_id )
{
   VCOS_STATUS_T final = VCOS_EINVAL;
   WIFIHDMI_INSTANCE_T *instance = handle;
   VC_WIFIHDMI_MSG_HDR_T *msg_hdr;
   VCOS_STATUS_T status;
   WIFIHDMI_CMD_RSP_BLK_T *cmd_blk = NULL;
   WIFIHDMI_CMD_RSP_BLK_T *rsp_blk = NULL;

   if ( handle == NULL )
   {
      LOG_ERR( "%s: invalid handle", __func__ );

      vcos_assert( handle != NULL );
      goto lock;
   }

   if ( (set == NULL) || (result == NULL) )
   {
      LOG_ERR( "%s: invalid input pointer", __func__ );

      vcos_assert( set != NULL );
      vcos_assert( result != NULL );
      goto lock;
   }

   cmd_blk = vcos_malloc( sizeof( *cmd_blk ), "" );
   if ( cmd_blk == NULL )
   {
      LOG_ERR( "[%s]: failed to allocate global tracking resource",
               __func__ );
      goto lock;
   }
   else
   {
      status = vcos_event_create( &cmd_blk->cmd_resp, "" );
      if ( status != VCOS_SUCCESS )
      {
         LOG_ERR( "%s: failed to create cmd-resp-evt (status=%d)", __func__, status );
         goto lock;
      }
   }

   if ( (status = vcos_semaphore_wait( &instance->vchi_sema )) != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed to wait on vchi-sema (status=%d)", __func__, status );
      goto lock;
   }

   cmd_blk->command_len = sizeof( *msg_hdr ) + sizeof( *set );
   memset( cmd_blk->command_msg_buf,
           0,
           cmd_blk->command_len );

   msg_hdr           = (VC_WIFIHDMI_MSG_HDR_T *)cmd_blk->command_msg_buf;
   msg_hdr->type     = VC_WIFIHDMI_MSG_TYPE_SET;
   msg_hdr->trans_id = ++instance->trans_id;
   memcpy( msg_hdr->body, set, sizeof( *set ));

   cmd_blk->reply_wait = 1;
   cmd_blk->trans_id   = msg_hdr->trans_id;
   if ( cur_trans_id != NULL ) { *cur_trans_id = msg_hdr->trans_id; }

   vc_vchi_add_cmd( instance,
                    cmd_blk );

   if ( vcos_mutex_lock ( &instance->io_lock ) == VCOS_SUCCESS )
   {
      instance->io_event_mask |= CLIENT_EVENT_MASK;
      vcos_mutex_unlock ( &instance->io_lock );
      vcos_event_signal( &instance->io_event );
   }
   else
   {
      LOG_ERR( "%s: failed on io-lock",
               __func__ );
      final = VCOS_EINVAL;
      goto unlock;
   }

   status = vcos_event_wait( &cmd_blk->cmd_resp );
   if ( status == VCOS_EAGAIN )
   {
      final = VCOS_EINTR;
      goto unlock;
   }
   else if ( status != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed on waiting for event (status=%d)",
               __func__, status );
      goto unlock;
   }

   rsp_blk = vc_vchi_rsp_from_tid( instance,
                                   cmd_blk->trans_id );
   if ( rsp_blk )
   {
      memcpy ( result,
               rsp_blk->reply_msg_buf,
               sizeof( *result ) );
      cmd_blk = NULL;
      final = VCOS_SUCCESS;
   }


unlock:
   vcos_semaphore_post( &instance->vchi_sema );
lock:
   if ( cmd_blk != NULL )
   {
      vc_vchi_rem_cmd( instance,
                       &cmd_blk );
   }
   if ( rsp_blk != NULL )
   {
      vc_vchi_rem_rsp( instance,
                       &rsp_blk );
   }
   return final;
}

VCOS_STATUS_T vc_vchi_wifihdmi_unset( VC_VCHI_WIFIHDMI_HANDLE_T handle,
                                      VC_WIFIHDMI_SET_T *set,
                                      VC_WIFIHDMI_RESULT_T *result,
                                      uint32_t *cur_trans_id )
{
   VCOS_STATUS_T final = VCOS_EINVAL;
   WIFIHDMI_INSTANCE_T *instance = handle;
   VC_WIFIHDMI_MSG_HDR_T *msg_hdr;
   VCOS_STATUS_T status;
   WIFIHDMI_CMD_RSP_BLK_T *cmd_blk = NULL;
   WIFIHDMI_CMD_RSP_BLK_T *rsp_blk = NULL;

   if ( handle == NULL )
   {
      LOG_ERR( "%s: invalid handle", __func__ );

      vcos_assert( handle != NULL );
      goto lock;
   }

   if ( (set == NULL) || (result == NULL) )
   {
      LOG_ERR( "%s: invalid input pointer", __func__ );

      vcos_assert( set != NULL );
      vcos_assert( result != NULL );
      goto lock;
   }

   cmd_blk = vcos_malloc( sizeof( *cmd_blk ), "" );
   if ( cmd_blk == NULL )
   {
      LOG_ERR( "[%s]: failed to allocate global tracking resource",
               __func__ );
      goto lock;
   }
   else
   {
      status = vcos_event_create( &cmd_blk->cmd_resp, "" );
      if ( status != VCOS_SUCCESS )
      {
         LOG_ERR( "%s: failed to create cmd-resp-evt (status=%d)", __func__, status );
         goto lock;
      }
   }

   if ( (status = vcos_semaphore_wait( &instance->vchi_sema )) != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed to wait on vchi-sema (status=%d)", __func__, status );
      goto lock;
   }

   cmd_blk->command_len = sizeof( *msg_hdr ) + sizeof( *set );
   memset( cmd_blk->command_msg_buf,
           0,
           cmd_blk->command_len );

   msg_hdr           = (VC_WIFIHDMI_MSG_HDR_T *)cmd_blk->command_msg_buf;
   msg_hdr->type     = VC_WIFIHDMI_MSG_TYPE_UNSET;
   msg_hdr->trans_id = ++instance->trans_id;
   memcpy( msg_hdr->body, set, sizeof( *set ));

   cmd_blk->reply_wait = 1;
   cmd_blk->trans_id   = msg_hdr->trans_id;
   if ( cur_trans_id != NULL ) { *cur_trans_id = msg_hdr->trans_id; }

   vc_vchi_add_cmd( instance,
                    cmd_blk );

   if ( vcos_mutex_lock ( &instance->io_lock ) == VCOS_SUCCESS )
   {
      instance->io_event_mask |= CLIENT_EVENT_MASK;
      vcos_mutex_unlock ( &instance->io_lock );
      vcos_event_signal( &instance->io_event );
   }
   else
   {
      LOG_ERR( "%s: failed on io-lock",
               __func__ );
      final = VCOS_EINVAL;
      goto unlock;
   }

   status = vcos_event_wait( &cmd_blk->cmd_resp );
   if ( status == VCOS_EAGAIN )
   {
      final = VCOS_EINTR;
      goto unlock;
   }
   else if ( status != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed on waiting for event (status=%d)",
               __func__, status );
      goto unlock;
   }

   rsp_blk = vc_vchi_rsp_from_tid( instance,
                                   cmd_blk->trans_id );
   if ( rsp_blk )
   {
      memcpy ( result,
               rsp_blk->reply_msg_buf,
               sizeof( *result ) );
      cmd_blk = NULL;
      final = VCOS_SUCCESS;
   }


unlock:
   vcos_semaphore_post( &instance->vchi_sema );
lock:
   if ( cmd_blk != NULL )
   {
      vc_vchi_rem_cmd( instance,
                       &cmd_blk );
   }
   if ( rsp_blk != NULL )
   {
      vc_vchi_rem_rsp( instance,
                       &rsp_blk );
   }
   return final;
}

VCOS_STATUS_T vc_vchi_wifihdmi_rec( VC_VCHI_WIFIHDMI_HANDLE_T handle,
                                    VC_WIFIHDMI_REC_T *rec,
                                    VC_WIFIHDMI_RESULT_T *result,
                                    uint32_t *cur_trans_id )
{
   VCOS_STATUS_T final = VCOS_EINVAL;
   WIFIHDMI_INSTANCE_T *instance = handle;
   VC_WIFIHDMI_MSG_HDR_T *msg_hdr;
   VCOS_STATUS_T status;
   WIFIHDMI_CMD_RSP_BLK_T *cmd_blk = NULL;
   WIFIHDMI_CMD_RSP_BLK_T *rsp_blk = NULL;

   if ( handle == NULL )
   {
      LOG_ERR( "%s: invalid handle", __func__ );

      vcos_assert( handle != NULL );
      goto lock;
   }

   if ( (rec == NULL) || (result == NULL) )
   {
      LOG_ERR( "%s: invalid input pointer", __func__ );

      vcos_assert( rec != NULL );
      vcos_assert( result != NULL );
      goto lock;
   }

   cmd_blk = vcos_malloc( sizeof( *cmd_blk ), "" );
   if ( cmd_blk == NULL )
   {
      LOG_ERR( "[%s]: failed to allocate global tracking resource",
               __func__ );
      goto lock;
   }
   else
   {
      status = vcos_event_create( &cmd_blk->cmd_resp, "" );
      if ( status != VCOS_SUCCESS )
      {
         LOG_ERR( "%s: failed to create cmd-resp-evt (status=%d)", __func__, status );
         goto lock;
      }
   }

   if ( (status = vcos_semaphore_wait( &instance->vchi_fast_sema )) != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed to wait on vchi-fast-sema (status=%d)", __func__, status );
      goto lock;
   }

   cmd_blk->command_len = sizeof( *msg_hdr ) + sizeof( *rec );
   memset( cmd_blk->command_msg_buf,
           0,
           cmd_blk->command_len );

   msg_hdr           = (VC_WIFIHDMI_MSG_HDR_T *)cmd_blk->command_msg_buf;
   msg_hdr->type     = VC_WIFIHDMI_MSG_TYPE_REC;
   msg_hdr->trans_id = ++instance->trans_fast_id;
   memcpy( msg_hdr->body, rec, sizeof( *rec ));

   cmd_blk->reply_wait = 1;
   cmd_blk->trans_id   = msg_hdr->trans_id;
   if ( cur_trans_id != NULL ) { *cur_trans_id = msg_hdr->trans_id; }

   vc_vchi_add_fast_cmd( instance,
                         cmd_blk );

   if ( vcos_mutex_lock ( &instance->iof_lock ) == VCOS_SUCCESS )
   {
      instance->iof_event_mask |= CLIENT_EVENT_MASK;
      vcos_mutex_unlock ( &instance->iof_lock );
      vcos_event_signal( &instance->iof_event );
   }
   else
   {
      LOG_ERR( "%s: failed on io-lock",
               __func__ );
      final = VCOS_EINVAL;
      goto unlock;
   }

   status = vcos_event_wait( &cmd_blk->cmd_resp );
   if ( status == VCOS_EAGAIN )
   {
      final = VCOS_EINTR;
      goto unlock;
   }
   else if ( status != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed on waiting for event (status=%d)",
               __func__, status );
      goto unlock;
   }

   rsp_blk = vc_vchi_rsp_fast_from_tid( instance,
                                        cmd_blk->trans_id );
   if ( rsp_blk )
   {
      memcpy ( result,
               rsp_blk->reply_msg_buf,
               sizeof( *result ) );
      cmd_blk = NULL;
      final = VCOS_SUCCESS;
   }


unlock:
   vcos_semaphore_post( &instance->vchi_fast_sema );
lock:
   if ( cmd_blk != NULL )
   {
      vc_vchi_rem_fast_cmd( instance,
                            &cmd_blk );
   }
   if ( rsp_blk != NULL )
   {
      vc_vchi_rem_fast_rsp( instance,
                            &rsp_blk );
   }
   return final;
}

VCOS_STATUS_T vc_vchi_wifihdmi_start( VC_VCHI_WIFIHDMI_HANDLE_T handle,
                                      VC_WIFIHDMI_MODE_T *mode, 
                                      VC_WIFIHDMI_RESULT_T *result,
                                      uint32_t *trans_id )
{
   VCOS_STATUS_T final = VCOS_EINVAL;
   WIFIHDMI_INSTANCE_T *instance = handle;
   VC_WIFIHDMI_MSG_HDR_T *msg_hdr;
   VCOS_STATUS_T status;
   WIFIHDMI_CMD_RSP_BLK_T *cmd_blk = NULL;
   WIFIHDMI_CMD_RSP_BLK_T *rsp_blk = NULL;

   if ( handle == NULL )
   {
      LOG_ERR( "%s: invalid handle", __func__ );

      vcos_assert( handle != NULL );
      goto lock;
   }

   if ( (mode == NULL) || (result == NULL) )
   {
      LOG_ERR( "%s: invalid input pointer", __func__ );

      vcos_assert( mode != NULL );
      vcos_assert( result != NULL );
      goto lock;
   }

   cmd_blk = vcos_malloc( sizeof( *cmd_blk ), "" );
   if ( cmd_blk == NULL )
   {
      LOG_ERR( "[%s]: failed to allocate global tracking resource",
               __func__ );
      goto lock;
   }
   else
   {
      status = vcos_event_create( &cmd_blk->cmd_resp, "" );
      if ( status != VCOS_SUCCESS )
      {
         LOG_ERR( "%s: failed to create cmd-resp-evt (status=%d)", __func__, status );
         goto lock;
      }
   }

   if ( (status = vcos_semaphore_wait( &instance->vchi_sema )) != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed to wait on vchi-sema (status=%d)", __func__, status );
      goto lock;
   }

   cmd_blk->command_len = sizeof( *msg_hdr ) + sizeof( *mode );
   memset( cmd_blk->command_msg_buf,
           0,
           cmd_blk->command_len );

   msg_hdr           = (VC_WIFIHDMI_MSG_HDR_T *)cmd_blk->command_msg_buf;
   msg_hdr->type     = VC_WIFIHDMI_MSG_TYPE_START;
   msg_hdr->trans_id = ++instance->trans_id;
   memcpy( msg_hdr->body, mode, sizeof( *mode ));

   cmd_blk->reply_wait = 1;
   cmd_blk->trans_id   = msg_hdr->trans_id;
   if ( trans_id != NULL ) { *trans_id = msg_hdr->trans_id; }

   vc_vchi_add_cmd( instance,
                    cmd_blk );

   if ( vcos_mutex_lock ( &instance->io_lock ) == VCOS_SUCCESS )
   {
      instance->io_event_mask |= CLIENT_EVENT_MASK;
      vcos_mutex_unlock ( &instance->io_lock );
      vcos_event_signal( &instance->io_event );
   }
   else
   {
      LOG_ERR( "%s: failed on io-lock",
               __func__ );
      final = VCOS_EINVAL;
      goto unlock;
   }

   status = vcos_event_wait( &cmd_blk->cmd_resp );
   if ( status == VCOS_EAGAIN )
   {
      final = VCOS_EINTR;
      goto unlock;
   }
   else if ( status != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed on waiting for event (status=%d)",
               __func__, status );
      goto unlock;
   }

   rsp_blk = vc_vchi_rsp_from_tid( instance,
                                   cmd_blk->trans_id );
   if ( rsp_blk )
   {
      memcpy ( result,
               rsp_blk->reply_msg_buf,
               sizeof( *result ) );
      cmd_blk = NULL;
      final = VCOS_SUCCESS;
   }


unlock:
   vcos_semaphore_post( &instance->vchi_sema );
lock:
   if ( cmd_blk != NULL )
   {
      vc_vchi_rem_cmd( instance,
                       &cmd_blk );
   }
   if ( rsp_blk != NULL )
   {
      vc_vchi_rem_rsp( instance,
                       &rsp_blk );
   }
   return final;
}
EXPORT_SYMBOL_GPL(vc_vchi_wifihdmi_start);

VCOS_STATUS_T vc_vchi_wifihdmi_stop( VC_VCHI_WIFIHDMI_HANDLE_T handle,
                                     VC_WIFIHDMI_MODE_T *mode, 
                                     VC_WIFIHDMI_RESULT_T *result,
                                     uint32_t *trans_id )
{
   VCOS_STATUS_T final = VCOS_EINVAL;
   WIFIHDMI_INSTANCE_T *instance = handle;
   VC_WIFIHDMI_MSG_HDR_T *msg_hdr;
   VCOS_STATUS_T status;
   WIFIHDMI_CMD_RSP_BLK_T *cmd_blk = NULL;
   WIFIHDMI_CMD_RSP_BLK_T *rsp_blk = NULL;

   if ( handle == NULL )
   {
      LOG_ERR( "%s: invalid handle", __func__ );

      vcos_assert( handle != NULL );
      goto lock;
   }

   if ( (mode == NULL) || (result == NULL) )
   {
      LOG_ERR( "%s: invalid input pointer", __func__ );

      vcos_assert( mode != NULL );
      vcos_assert( result != NULL );
      goto lock;
   }

   cmd_blk = vcos_malloc( sizeof( *cmd_blk ), "" );
   if ( cmd_blk == NULL )
   {
      LOG_ERR( "[%s]: failed to allocate global tracking resource",
               __func__ );
      goto lock;
   }
   else
   {
      status = vcos_event_create( &cmd_blk->cmd_resp, "" );
      if ( status != VCOS_SUCCESS )
      {
         LOG_ERR( "%s: failed to create cmd-resp-evt (status=%d)", __func__, status );
         goto lock;
      }
   }

   if ( (status = vcos_semaphore_wait( &instance->vchi_sema )) != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed to wait on vchi-sema (status=%d)", __func__, status );
      goto lock;
   }

   cmd_blk->command_len = sizeof( *msg_hdr ) + sizeof( *mode );
   memset( cmd_blk->command_msg_buf,
           0,
           cmd_blk->command_len );

   msg_hdr           = (VC_WIFIHDMI_MSG_HDR_T *)cmd_blk->command_msg_buf;
   msg_hdr->type     = VC_WIFIHDMI_MSG_TYPE_STOP;
   msg_hdr->trans_id = ++instance->trans_id;
   memcpy( msg_hdr->body, mode, sizeof( *mode ));

   cmd_blk->reply_wait = 1;
   cmd_blk->trans_id   = msg_hdr->trans_id;
   if ( trans_id != NULL ) { *trans_id = msg_hdr->trans_id; }

   vc_vchi_add_cmd( instance,
                    cmd_blk );

   if ( vcos_mutex_lock ( &instance->io_lock ) == VCOS_SUCCESS )
   {
      instance->io_event_mask |= CLIENT_EVENT_MASK;
      vcos_mutex_unlock ( &instance->io_lock );
      vcos_event_signal( &instance->io_event );
   }
   else
   {
      LOG_ERR( "%s: failed on io-lock",
               __func__ );
      final = VCOS_EINVAL;
      goto unlock;
   }

   status = vcos_event_wait( &cmd_blk->cmd_resp );
   if ( status == VCOS_EAGAIN )
   {
      final = VCOS_EINTR;
      goto unlock;
   }
   else if ( status != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed on waiting for event (status=%d)",
               __func__, status );
      goto unlock;
   }

   rsp_blk = vc_vchi_rsp_from_tid( instance,
                                   cmd_blk->trans_id );
   if ( rsp_blk )
   {
      memcpy ( result,
               rsp_blk->reply_msg_buf,
               sizeof( *result ) );
      cmd_blk = NULL;
      final = VCOS_SUCCESS;
   }


unlock:
   vcos_semaphore_post( &instance->vchi_sema );
lock:
   if ( cmd_blk != NULL )
   {
      vc_vchi_rem_cmd( instance,
                       &cmd_blk );
   }
   if ( rsp_blk != NULL )
   {
      vc_vchi_rem_rsp( instance,
                       &rsp_blk );
   }
   return final;
}
EXPORT_SYMBOL_GPL(vc_vchi_wifihdmi_stop);

VCOS_STATUS_T vc_vchi_wifihdmi_skt_in( VC_VCHI_WIFIHDMI_HANDLE_T handle,
                                       VC_WIFIHDMI_SKT_T *skt,
                                       VC_WIFIHDMI_SKT_RES_T *skt_res,
                                       uint32_t *trans_id )
{
   VCOS_STATUS_T final = VCOS_EINVAL;
   WIFIHDMI_INSTANCE_T *instance = handle;
   VC_WIFIHDMI_MSG_HDR_T *msg_hdr;
   VCOS_STATUS_T status;
   WIFIHDMI_CMD_RSP_BLK_T *cmd_blk = NULL;
   WIFIHDMI_CMD_RSP_BLK_T *rsp_blk = NULL;

   if ( handle == NULL )
   {
      LOG_ERR( "%s: invalid handle", __func__ );

      vcos_assert( handle != NULL );
      goto lock;
   }

   if ( (skt == NULL) || (skt_res == NULL) )
   {
      LOG_ERR( "%s: invalid input pointer", __func__ );

      vcos_assert( skt != NULL );
      vcos_assert( skt_res != NULL );
      goto lock;
   }

   cmd_blk = vcos_malloc( sizeof( *cmd_blk ), "" );
   if ( cmd_blk == NULL )
   {
      LOG_ERR( "[%s]: failed to allocate global tracking resource",
               __func__ );
      goto lock;
   }
   else
   {
      status = vcos_event_create( &cmd_blk->cmd_resp, "" );
      if ( status != VCOS_SUCCESS )
      {
         LOG_ERR( "%s: failed to create cmd-resp-evt (status=%d)", __func__, status );
         goto lock;
      }
   }

   if ( (status = vcos_semaphore_wait( &instance->vchi_sema )) != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed to wait on vchi-sema (status=%d)", __func__, status );
      goto lock;
   }

   cmd_blk->command_len = sizeof( *msg_hdr ) + sizeof( *skt );
   memset( cmd_blk->command_msg_buf,
           0,
           cmd_blk->command_len );

   msg_hdr           = (VC_WIFIHDMI_MSG_HDR_T *)cmd_blk->command_msg_buf;
   msg_hdr->type     = VC_WIFIHDMI_MSG_TYPE_SKT_IN;
   msg_hdr->trans_id = ++instance->trans_id;
   memcpy( msg_hdr->body, skt, sizeof( *skt ));

   cmd_blk->reply_wait = 1;
   cmd_blk->trans_id   = msg_hdr->trans_id;
   if ( trans_id != NULL ) { *trans_id = msg_hdr->trans_id; }

   vc_vchi_add_cmd( instance,
                    cmd_blk );

   if ( vcos_mutex_lock ( &instance->io_lock ) == VCOS_SUCCESS )
   {
      instance->io_event_mask |= CLIENT_EVENT_MASK;
      vcos_mutex_unlock ( &instance->io_lock );
      vcos_event_signal( &instance->io_event );
   }
   else
   {
      LOG_ERR( "%s: failed on io-lock",
               __func__ );
      final = VCOS_EINVAL;
      goto unlock;
   }

   status = vcos_event_wait( &cmd_blk->cmd_resp );
   if ( status == VCOS_EAGAIN )
   {
      final = VCOS_EINTR;
      goto unlock;
   }
   else if ( status != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed on waiting for event (status=%d)",
               __func__, status );
      goto unlock;
   }

   rsp_blk = vc_vchi_rsp_from_tid( instance,
                                   cmd_blk->trans_id );
   if ( rsp_blk )
   {
      memcpy ( skt_res,
               rsp_blk->reply_msg_buf,
               sizeof( *skt_res ) );
      cmd_blk = NULL;
      final = VCOS_SUCCESS;
   }


unlock:
   vcos_semaphore_post( &instance->vchi_sema );
lock:
   if ( cmd_blk != NULL )
   {
      vc_vchi_rem_cmd( instance,
                       &cmd_blk );
   }
   if ( rsp_blk != NULL )
   {
      vc_vchi_rem_rsp( instance,
                       &rsp_blk );
   }
   return final;
}

VCOS_STATUS_T vc_vchi_wifihdmi_skt_dsc( VC_VCHI_WIFIHDMI_HANDLE_T handle,
                                        VC_WIFIHDMI_SKT_T *skt,
                                        VC_WIFIHDMI_RESULT_T *result,
                                        uint32_t *trans_id )
{
   VCOS_STATUS_T final = VCOS_EINVAL;
   WIFIHDMI_INSTANCE_T *instance = handle;
   VC_WIFIHDMI_MSG_HDR_T *msg_hdr;
   VCOS_STATUS_T status;
   WIFIHDMI_CMD_RSP_BLK_T *cmd_blk = NULL;
   WIFIHDMI_CMD_RSP_BLK_T *rsp_blk = NULL;

   if ( handle == NULL )
   {
      LOG_ERR( "%s: invalid handle", __func__ );

      vcos_assert( handle != NULL );
      goto lock;
   }

   if ( (skt == NULL) || (result == NULL) )
   {
      LOG_ERR( "%s: invalid input pointer", __func__ );

      vcos_assert( skt != NULL );
      vcos_assert( result != NULL );
      goto lock;
   }

   cmd_blk = vcos_malloc( sizeof( *cmd_blk ), "" );
   if ( cmd_blk == NULL )
   {
      LOG_ERR( "[%s]: failed to allocate global tracking resource",
               __func__ );
      goto lock;
   }
   else
   {
      status = vcos_event_create( &cmd_blk->cmd_resp, "" );
      if ( status != VCOS_SUCCESS )
      {
         LOG_ERR( "%s: failed to create cmd-resp-evt (status=%d)", __func__, status );
         goto lock;
      }
   }

   if ( (status = vcos_semaphore_wait( &instance->vchi_sema )) != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed to wait on vchi-sema (status=%d)", __func__, status );
      goto lock;
   }

   cmd_blk->command_len = sizeof( *msg_hdr ) + sizeof( *skt );
   memset( cmd_blk->command_msg_buf,
           0,
           cmd_blk->command_len );

   msg_hdr           = (VC_WIFIHDMI_MSG_HDR_T *)cmd_blk->command_msg_buf;
   msg_hdr->type     = VC_WIFIHDMI_MSG_TYPE_SKT_DSC;
   msg_hdr->trans_id = ++instance->trans_id;
   memcpy( msg_hdr->body, skt, sizeof( *skt ));

   cmd_blk->reply_wait = 1;
   cmd_blk->trans_id   = msg_hdr->trans_id;
   if ( trans_id != NULL ) { *trans_id = msg_hdr->trans_id; }

   vc_vchi_add_cmd( instance,
                    cmd_blk );

   if ( vcos_mutex_lock ( &instance->io_lock ) == VCOS_SUCCESS )
   {
      instance->io_event_mask |= CLIENT_EVENT_MASK;
      vcos_mutex_unlock ( &instance->io_lock );
      vcos_event_signal( &instance->io_event );
   }
   else
   {
      LOG_ERR( "%s: failed on io-lock",
               __func__ );
      final = VCOS_EINVAL;
      goto unlock;
   }

   status = vcos_event_wait( &cmd_blk->cmd_resp );
   if ( status == VCOS_EAGAIN )
   {
      final = VCOS_EINTR;
      goto unlock;
   }
   else if ( status != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed on waiting for event (status=%d)",
               __func__, status );
      goto unlock;
   }

   rsp_blk = vc_vchi_rsp_from_tid( instance,
                                   cmd_blk->trans_id );
   if ( rsp_blk )
   {
      memcpy ( result,
               rsp_blk->reply_msg_buf,
               sizeof( *result ) );
      cmd_blk = NULL;
      final = VCOS_SUCCESS;
   }


unlock:
   vcos_semaphore_post( &instance->vchi_sema );
lock:
   if ( cmd_blk != NULL )
   {
      vc_vchi_rem_cmd( instance,
                       &cmd_blk );
   }
   if ( rsp_blk != NULL )
   {
      vc_vchi_rem_rsp( instance,
                       &rsp_blk );
   }
   return final;
}

VCOS_STATUS_T vc_vchi_wifihdmi_skt_end( VC_VCHI_WIFIHDMI_HANDLE_T handle,
                                        VC_WIFIHDMI_SKT_T *skt,
                                        VC_WIFIHDMI_RESULT_T *result,
                                        uint32_t *trans_id )
{
   VCOS_STATUS_T final = VCOS_EINVAL;
   WIFIHDMI_INSTANCE_T *instance = handle;
   VC_WIFIHDMI_MSG_HDR_T *msg_hdr;
   VCOS_STATUS_T status;
   WIFIHDMI_CMD_RSP_BLK_T *cmd_blk = NULL;
   WIFIHDMI_CMD_RSP_BLK_T *rsp_blk = NULL;

   if ( handle == NULL )
   {
      LOG_ERR( "%s: invalid handle", __func__ );

      vcos_assert( handle != NULL );
      goto lock;
   }

   if ( (skt == NULL) || (result == NULL) )
   {
      LOG_ERR( "%s: invalid input pointer", __func__ );

      vcos_assert( skt != NULL );
      vcos_assert( result != NULL );
      goto lock;
   }

   cmd_blk = vcos_malloc( sizeof( *cmd_blk ), "" );
   if ( cmd_blk == NULL )
   {
      LOG_ERR( "[%s]: failed to allocate global tracking resource",
               __func__ );
      goto lock;
   }
   else
   {
      status = vcos_event_create( &cmd_blk->cmd_resp, "" );
      if ( status != VCOS_SUCCESS )
      {
         LOG_ERR( "%s: failed to create cmd-resp-evt (status=%d)", __func__, status );
         goto lock;
      }
   }

   if ( (status = vcos_semaphore_wait( &instance->vchi_sema )) != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed to wait on vchi-sema (status=%d)", __func__, status );
      goto lock;
   }

   cmd_blk->command_len = sizeof( *msg_hdr ) + sizeof( *skt );
   memset( cmd_blk->command_msg_buf,
           0,
           cmd_blk->command_len );

   msg_hdr           = (VC_WIFIHDMI_MSG_HDR_T *)cmd_blk->command_msg_buf;
   msg_hdr->type     = VC_WIFIHDMI_MSG_TYPE_SKT_END;
   msg_hdr->trans_id = ++instance->trans_id;
   memcpy( msg_hdr->body, skt, sizeof( *skt ));

   cmd_blk->reply_wait = 1;
   cmd_blk->trans_id   = msg_hdr->trans_id;
   if ( trans_id != NULL ) { *trans_id = msg_hdr->trans_id; }

   vc_vchi_add_cmd( instance,
                    cmd_blk );

   if ( vcos_mutex_lock ( &instance->io_lock ) == VCOS_SUCCESS )
   {
      instance->io_event_mask |= CLIENT_EVENT_MASK;
      vcos_mutex_unlock ( &instance->io_lock );
      vcos_event_signal( &instance->io_event );
   }
   else
   {
      LOG_ERR( "%s: failed on io-lock",
               __func__ );
      final = VCOS_EINVAL;
      goto unlock;
   }

   status = vcos_event_wait( &cmd_blk->cmd_resp );
   if ( status == VCOS_EAGAIN )
   {
      final = VCOS_EINTR;
      goto unlock;
   }
   else if ( status != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed on waiting for event (status=%d)",
               __func__, status );
      goto unlock;
   }

   rsp_blk = vc_vchi_rsp_from_tid( instance,
                                   cmd_blk->trans_id );
   if ( rsp_blk )
   {
      memcpy ( result,
               rsp_blk->reply_msg_buf,
               sizeof( *result ) );
      cmd_blk = NULL;
      final = VCOS_SUCCESS;
   }


unlock:
   vcos_semaphore_post( &instance->vchi_sema );
lock:
   if ( cmd_blk != NULL )
   {
      vc_vchi_rem_cmd( instance,
                       &cmd_blk );
   }
   if ( rsp_blk != NULL )
   {
      vc_vchi_rem_rsp( instance,
                       &rsp_blk );
   }
   return final;
}

VCOS_STATUS_T vc_vchi_wifihdmi_skt_data( VC_VCHI_WIFIHDMI_HANDLE_T handle,
                                         VC_WIFIHDMI_SKT_DATA_T *skt_data,
                                         VC_WIFIHDMI_RESULT_T *result,
                                         uint32_t *trans_id )
{
   VCOS_STATUS_T final = VCOS_EINVAL;
   WIFIHDMI_INSTANCE_T *instance = handle;
   VC_WIFIHDMI_MSG_HDR_T *msg_hdr;
   VCOS_STATUS_T status;
   WIFIHDMI_CMD_RSP_BLK_T *cmd_blk = NULL;
   WIFIHDMI_CMD_RSP_BLK_T *rsp_blk = NULL;

   if ( handle == NULL )
   {
      LOG_ERR( "%s: invalid handle", __func__ );

      vcos_assert( handle != NULL );
      goto lock;
   }

   if ( (skt_data == NULL) || (result == NULL) )
   {
      LOG_ERR( "%s: invalid input pointer", __func__ );

      vcos_assert( skt_data != NULL );
      vcos_assert( result != NULL );
      goto lock;
   }

   cmd_blk = vcos_malloc( sizeof( *cmd_blk ), "" );
   if ( cmd_blk == NULL )
   {
      LOG_ERR( "[%s]: failed to allocate global tracking resource",
               __func__ );
      goto lock;
   }
   else
   {
      status = vcos_event_create( &cmd_blk->cmd_resp, "" );
      if ( status != VCOS_SUCCESS )
      {
         LOG_ERR( "%s: failed to create cmd-resp-evt (status=%d)", __func__, status );
         goto lock;
      }
   }

   if ( (status = vcos_semaphore_wait( &instance->vchi_sema )) != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed to wait on vchi-sema (status=%d)", __func__, status );
      goto lock;
   }

   cmd_blk->command_len = sizeof( *msg_hdr ) + sizeof( *skt_data );
   memset( cmd_blk->command_msg_buf,
           0,
           cmd_blk->command_len );

   msg_hdr           = (VC_WIFIHDMI_MSG_HDR_T *)cmd_blk->command_msg_buf;
   msg_hdr->type     = VC_WIFIHDMI_MSG_TYPE_SKT_DATA;
   msg_hdr->trans_id = ++instance->trans_id;
   memcpy( msg_hdr->body, skt_data, sizeof( *skt_data ));

   cmd_blk->reply_wait = 1;
   cmd_blk->trans_id   = msg_hdr->trans_id;
   if ( trans_id != NULL ) { *trans_id = msg_hdr->trans_id; }

   vc_vchi_add_cmd( instance,
                    cmd_blk );

   if ( vcos_mutex_lock ( &instance->io_lock ) == VCOS_SUCCESS )
   {
      instance->io_event_mask |= CLIENT_EVENT_MASK;
      vcos_mutex_unlock ( &instance->io_lock );
      vcos_event_signal( &instance->io_event );
   }
   else
   {
      LOG_ERR( "%s: failed on io-lock",
               __func__ );
      final = VCOS_EINVAL;
      goto unlock;
   }

   status = vcos_event_wait( &cmd_blk->cmd_resp );
   if ( status == VCOS_EAGAIN )
   {
      final = VCOS_EINTR;
      goto unlock;
   }
   else if ( status != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed on waiting for event (status=%d)",
               __func__, status );
      goto unlock;
   }

   rsp_blk = vc_vchi_rsp_from_tid( instance,
                                   cmd_blk->trans_id );
   if ( rsp_blk )
   {
      memcpy ( result,
               rsp_blk->reply_msg_buf,
               sizeof( *result ) );
      cmd_blk = NULL;
      final = VCOS_SUCCESS;
   }


unlock:
   vcos_semaphore_post( &instance->vchi_sema );
lock:
   if ( cmd_blk != NULL )
   {
      vc_vchi_rem_cmd( instance,
                       &cmd_blk );
   }
   if ( rsp_blk != NULL )
   {
      vc_vchi_rem_rsp( instance,
                       &rsp_blk );
   }
   return final;
}

VCOS_STATUS_T vc_vchi_wifihdmi_audio_status( VC_VCHI_WIFIHDMI_HANDLE_T handle,
                                             VC_WIFIHDMI_STREAM_T *stream,
                                             VC_WIFIHDMI_STR_STA_RES_T *result,
                                             uint32_t *trans_id )
{
   VCOS_STATUS_T final = VCOS_EINVAL;
   WIFIHDMI_INSTANCE_T *instance = handle;
   VC_WIFIHDMI_MSG_HDR_T *msg_hdr;
   VCOS_STATUS_T status;
   WIFIHDMI_CMD_RSP_BLK_T *cmd_blk = NULL;
   WIFIHDMI_CMD_RSP_BLK_T *rsp_blk = NULL;

   if ( handle == NULL )
   {
      LOG_ERR( "%s: invalid handle", __func__ );

      vcos_assert( handle != NULL );
      goto lock;
   }

   if ( (stream == NULL) || (result == NULL) )
   {
      LOG_ERR( "%s: invalid input pointer", __func__ );

      vcos_assert( stream != NULL );
      vcos_assert( result != NULL );
      goto lock;
   }

   cmd_blk = vcos_malloc( sizeof( *cmd_blk ), "" );
   if ( cmd_blk == NULL )
   {
      LOG_ERR( "[%s]: failed to allocate global tracking resource",
               __func__ );
      goto lock;
   }
   else
   {
      status = vcos_event_create( &cmd_blk->cmd_resp, "" );
      if ( status != VCOS_SUCCESS )
      {
         LOG_ERR( "%s: failed to create cmd-resp-evt (status=%d)", __func__, status );
         goto lock;
      }
   }

   if ( (status = vcos_semaphore_wait( &instance->vchi_sema )) != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed to wait on vchi-sema (status=%d)", __func__, status );
      goto lock;
   }

   cmd_blk->command_len = sizeof( *msg_hdr ) + sizeof( *stream );
   memset( cmd_blk->command_msg_buf,
           0,
           cmd_blk->command_len );

   msg_hdr           = (VC_WIFIHDMI_MSG_HDR_T *)cmd_blk->command_msg_buf;
   msg_hdr->type     = VC_WIFIHDMI_MSG_TYPE_AUDIO_STREAM_STAT;
   msg_hdr->trans_id = ++instance->trans_id;
   memcpy( msg_hdr->body, stream, sizeof( *stream ));

   cmd_blk->reply_wait = 1;
   cmd_blk->trans_id   = msg_hdr->trans_id;
   if ( trans_id != NULL ) { *trans_id = msg_hdr->trans_id; }

   vc_vchi_add_cmd( instance,
                    cmd_blk );

   if ( vcos_mutex_lock ( &instance->io_lock ) == VCOS_SUCCESS )
   {
      instance->io_event_mask |= CLIENT_EVENT_MASK;
      vcos_mutex_unlock ( &instance->io_lock );
      vcos_event_signal( &instance->io_event );
   }
   else
   {
      LOG_ERR( "%s: failed on io-lock",
               __func__ );
      final = VCOS_EINVAL;
      goto unlock;
   }

   status = vcos_event_wait( &cmd_blk->cmd_resp );
   if ( status == VCOS_EAGAIN )
   {
      final = VCOS_EINTR;
      goto unlock;
   }
   else if ( status != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed on waiting for event (status=%d)",
               __func__, status );
      goto unlock;
   }

   rsp_blk = vc_vchi_rsp_from_tid( instance,
                                   cmd_blk->trans_id );
   if ( rsp_blk )
   {
      memcpy ( result,
               rsp_blk->reply_msg_buf,
               sizeof( *result ) );
      cmd_blk = NULL;
      final = VCOS_SUCCESS;
   }


unlock:
   vcos_semaphore_post( &instance->vchi_sema );
lock:
   if ( cmd_blk != NULL )
   {
      vc_vchi_rem_cmd( instance,
                       &cmd_blk );
   }
   if ( rsp_blk != NULL )
   {
      vc_vchi_rem_rsp( instance,
                       &rsp_blk );
   }
   return final;
}

VCOS_STATUS_T vc_vchi_wifihdmi_stats( VC_VCHI_WIFIHDMI_HANDLE_T handle,
                                      VC_WIFIHDMI_MODE_T *mode,
                                      VC_WIFIHDMI_STATS_T *stats,
                                      uint32_t *trans_id )
{
   VCOS_STATUS_T final = VCOS_EINVAL;
   WIFIHDMI_INSTANCE_T *instance = handle;
   VC_WIFIHDMI_MSG_HDR_T *msg_hdr;
   VCOS_STATUS_T status;
   WIFIHDMI_CMD_RSP_BLK_T *cmd_blk = NULL;
   WIFIHDMI_CMD_RSP_BLK_T *rsp_blk = NULL;

   if ( handle == NULL )
   {
      LOG_ERR( "%s: invalid handle", __func__ );

      vcos_assert( handle != NULL );
      goto lock;
   }

   if ( (mode == NULL) || (stats == NULL) )
   {
      LOG_ERR( "%s: invalid input pointer", __func__ );

      vcos_assert( mode != NULL );
      vcos_assert( stats != NULL );
      goto lock;
   }

   cmd_blk = vcos_malloc( sizeof( *cmd_blk ), "" );
   if ( cmd_blk == NULL )
   {
      LOG_ERR( "[%s]: failed to allocate global tracking resource",
               __func__ );
      goto lock;
   }
   else
   {
      status = vcos_event_create( &cmd_blk->cmd_resp, "" );
      if ( status != VCOS_SUCCESS )
      {
         LOG_ERR( "%s: failed to create cmd-resp-evt (status=%d)", __func__, status );
         goto lock;
      }
   }

   if ( (status = vcos_semaphore_wait( &instance->vchi_sema )) != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed to wait on vchi-sema (status=%d)", __func__, status );
      goto lock;
   }

   cmd_blk->command_len = sizeof( *msg_hdr ) + sizeof( *mode );
   memset( cmd_blk->command_msg_buf,
           0,
           cmd_blk->command_len );

   msg_hdr           = (VC_WIFIHDMI_MSG_HDR_T *)cmd_blk->command_msg_buf;
   msg_hdr->type     = VC_WIFIHDMI_MSG_TYPE_STATS;
   msg_hdr->trans_id = ++instance->trans_id;
   memcpy( msg_hdr->body, mode, sizeof( *mode ));

   cmd_blk->reply_wait = 1;
   cmd_blk->trans_id   = msg_hdr->trans_id;
   if ( trans_id != NULL ) { *trans_id = msg_hdr->trans_id; }

   vc_vchi_add_cmd( instance,
                    cmd_blk );

   if ( vcos_mutex_lock ( &instance->io_lock ) == VCOS_SUCCESS )
   {
      instance->io_event_mask |= CLIENT_EVENT_MASK;
      vcos_mutex_unlock ( &instance->io_lock );
      vcos_event_signal( &instance->io_event );
   }
   else
   {
      LOG_ERR( "%s: failed on io-lock",
               __func__ );
      final = VCOS_EINVAL;
      goto unlock;
   }

   status = vcos_event_wait( &cmd_blk->cmd_resp );
   if ( status == VCOS_EAGAIN )
   {
      final = VCOS_EINTR;
      goto unlock;
   }
   else if ( status != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed on waiting for event (status=%d)",
               __func__, status );
      goto unlock;
   }

   rsp_blk = vc_vchi_rsp_from_tid( instance,
                                   cmd_blk->trans_id );
   if ( rsp_blk )
   {
      memcpy ( stats,
               rsp_blk->reply_msg_buf,
               sizeof( *stats ) );
      cmd_blk = NULL;
      final = VCOS_SUCCESS;
   }


unlock:
   vcos_semaphore_post( &instance->vchi_sema );
lock:
   if ( cmd_blk != NULL )
   {
      vc_vchi_rem_cmd( instance,
                       &cmd_blk );
   }
   if ( rsp_blk != NULL )
   {
      vc_vchi_rem_rsp( instance,
                       &rsp_blk );
   }
   return final;
}
EXPORT_SYMBOL_GPL(vc_vchi_wifihdmi_stats);

VCOS_STATUS_T vc_vchi_wifihdmi_tx_pool( VC_VCHI_WIFIHDMI_HANDLE_T handle,
                                        uint32_t pool_size, 
                                        uint32_t unit_size )
{
   WIFIHDMI_INSTANCE_T *instance = handle;
   VC_SM_ALLOC_T alloc;
   VC_WIFIHDMI_RESULT_T result;
   VC_WIFIHDMI_SET_T set;
   uint32_t ix;
   int sm_handle;
   uint32_t trans_id;

   if ( handle == NULL )
   {
      LOG_ERR( "%s: invalid handle %p", __func__, handle );

      vcos_assert( handle != NULL );
      return VCOS_EINVAL;
   }

   memset ( &alloc, 0, sizeof(alloc) );

   alloc.type       = VC_SM_ALLOC_NON_CACHED;
   alloc.base_unit  = unit_size;
   alloc.num_unit   = 1;
   alloc.alignement = 4096;
   alloc.allocator  = 0;

   for ( ix = 0 ; ix < pool_size ; ix++ )
   {
      if ( vc_sm_alloc( &alloc, &sm_handle ) == 0 )
      {
         WIFIHDMI_DATA_PUMP_BLK_T *blk = vcos_malloc( sizeof( *blk ), "" );

         memset ( &set, 0, sizeof(set) );
         memset ( &result, 0, sizeof(result) );

         set.res_handle   = sm_handle;
         set.res_inthdl   = vc_sm_int_handle( sm_handle );
         set.res_size     = alloc.base_unit;

         if ( blk &&
              vc_vchi_wifihdmi_set( instance,
                                    &set,
                                    &result,
                                    &trans_id ) == VCOS_SUCCESS )
         {
            if ( result.success == 0 )
            {
               blk->handle  = set.res_handle;
               blk->size    = set.res_size;

               //LOG_DBG( "%s: data-pump creating %p, handle %x, size %d",
               //         __func__, blk, blk->handle, blk->size );

               vc_vchi_add_data( instance,
                                 blk );
            }
            else
            {
               LOG_ERR( "%s: data-pump failed to associate handle %x",
                        __func__, blk->handle );

               vc_sm_free ( sm_handle );
               vcos_free ( blk );
            }
         }
         else
         {
            LOG_ERR( "%s: data-pump failed to allocate handle %x",
                     __func__, sm_handle );

            vc_sm_free ( sm_handle );
         }
      }
   }

   return VCOS_SUCCESS;
}
EXPORT_SYMBOL_GPL(vc_vchi_wifihdmi_tx_pool);
