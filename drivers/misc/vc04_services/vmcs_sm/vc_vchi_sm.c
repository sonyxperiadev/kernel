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

#include "vc_vchi_sm.h"

// ---- Private Constants and Types ------------------------------------------

#define CLIENT_EVENT_MASK   0x1
#define SERVER_EVENT_MASK   0x2

// VCOS logging category for this service
#define VCOS_LOG_CATEGORY (&sm_log_category)

// Default VCOS logging level
#define LOG_LEVEL  VCOS_LOG_TRACE

// Logging macros (for remapping to other logging mechanisms, i.e., printf)
#define LOG_ERR( fmt, arg... )   vcos_log_error( fmt, ##arg )
#define LOG_WARN( fmt, arg... )  vcos_log_warn( fmt, ##arg )
#define LOG_INFO( fmt, arg... )  vcos_log_info( fmt, ##arg )
#define LOG_DBG( fmt, arg... )   vcos_log_trace( fmt, ##arg )

typedef struct sm_cmd_rsp_blk_t
{
   struct sm_cmd_rsp_blk_t          *next;       // Linked list next pointer

   VCOS_EVENT_T                     cmd_resp;    // To be signaled when the response is there
   uint32_t                         trans_id;

   uint8_t                          command_msg_buf[VC_SM_MAX_MSG_LEN];
   uint32_t                         command_len;

   uint8_t                          reply_wait;
   uint8_t                          reply_msg_buf[VC_SM_MAX_RSP_LEN];

} SM_CMD_RSP_BLK_T;

typedef struct opaque_vc_vchi_sm_handle_t
{
   uint32_t               num_connections;
   VCHI_SERVICE_HANDLE_T  vchi_handle[VCHI_MAX_NUM_CONNECTIONS];
   VCOS_THREAD_T          io_thread;
   uint32_t               io_event_mask;
   VCOS_MUTEX_T           io_lock;
   VCOS_EVENT_T           io_event;

   VCOS_SEMAPHORE_T       vchi_sema;   // Use semaphore of count 1 to simulate mutex, but we need semaphore
                                       // because of interrupt handling threading.

   uint32_t               trans_id;

   SM_CMD_RSP_BLK_T       *cmd_list;
   VCOS_MUTEX_T           cmd_lock;

   SM_CMD_RSP_BLK_T       *rsp_list;
   VCOS_MUTEX_T           rsp_lock;

} SM_INSTANCE_T;

// ---- Private Variables ----------------------------------------------------

// VCOS logging category for this service
static VCOS_LOG_CAT_T sm_log_category;

// ---- Private Function Prototypes ------------------------------------------

// ---- Private Functions ----------------------------------------------------
static void vc_vchi_add_cmd( SM_INSTANCE_T *instance,
                             SM_CMD_RSP_BLK_T *cmd )
{
   mutex_lock ( &(instance->cmd_lock) );

   if ( instance->cmd_list == NULL )
   {
      instance->cmd_list = cmd;
   }
   else
   {
      SM_CMD_RSP_BLK_T *ptr = instance->cmd_list;

      while ( ptr->next != NULL )
      {
         ptr = ptr->next;
      }

      ptr->next = cmd;
   }

   cmd->next = NULL;

   mutex_unlock ( &(instance->cmd_lock) );
}

static void vc_vchi_add_rsp( SM_INSTANCE_T *instance,
                             SM_CMD_RSP_BLK_T *rsp )
{
   mutex_lock ( &(instance->rsp_lock) );

   if ( instance->rsp_list == NULL )
   {
      instance->rsp_list = rsp;
   }
   else
   {
      SM_CMD_RSP_BLK_T *ptr = instance->rsp_list;

      while ( ptr->next != NULL )
      {
         ptr = ptr->next;
      }

      ptr->next = rsp;
   }

   rsp->next = NULL;

   mutex_unlock ( &(instance->rsp_lock) );
}

static void vc_vchi_rem_cmd( SM_INSTANCE_T *instance,
                             SM_CMD_RSP_BLK_T **cmd )
{
   SM_CMD_RSP_BLK_T *prev_ptr = NULL;
   SM_CMD_RSP_BLK_T *curr_ptr = NULL;

   mutex_lock ( &(instance->cmd_lock) );

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
         kfree( *cmd );
         *cmd = NULL;

         mutex_unlock ( &(instance->cmd_lock) );
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
   kfree( *cmd );
   *cmd = NULL;

   mutex_unlock ( &(instance->cmd_lock) );
}

static void vc_vchi_unlink_cmd( SM_INSTANCE_T *instance,
                                SM_CMD_RSP_BLK_T *cmd )
{
   SM_CMD_RSP_BLK_T *prev_ptr = NULL;
   SM_CMD_RSP_BLK_T *curr_ptr = NULL;

   mutex_lock ( &(instance->cmd_lock) );

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

         mutex_unlock ( &(instance->cmd_lock) );
         return;
      }
      else
      {
         prev_ptr = curr_ptr;
         curr_ptr = curr_ptr->next;
      }
   }

   mutex_unlock ( &(instance->cmd_lock) );
}

static void vc_vchi_rem_rsp( SM_INSTANCE_T *instance,
                             SM_CMD_RSP_BLK_T **rsp )
{
   SM_CMD_RSP_BLK_T *prev_ptr = NULL;
   SM_CMD_RSP_BLK_T *curr_ptr = NULL;

   mutex_lock ( &(instance->rsp_lock) );

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
         kfree( *rsp );
         *rsp = NULL;

         mutex_unlock ( &(instance->rsp_lock) );
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
   kfree( *rsp );
   *rsp = NULL;

   mutex_unlock ( &(instance->rsp_lock) );
}

static SM_CMD_RSP_BLK_T * vc_vchi_rsp_from_tid( SM_INSTANCE_T *instance,
                                                uint32_t trans_id )
{
   SM_CMD_RSP_BLK_T *blk = NULL;

   if ( instance == NULL )
   {
      goto out;
   }

   if ( trans_id == 0 )
   {
      goto out;
   }

   mutex_lock ( &(instance->rsp_lock) );

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
         mutex_unlock ( &(instance->rsp_lock) );
         return blk;
      }

      blk = blk->next;
   }

   mutex_unlock ( &(instance->rsp_lock) );

out:
   return NULL;
}

static SM_CMD_RSP_BLK_T * vc_vchi_cmd_top( SM_INSTANCE_T *instance )
{
   SM_CMD_RSP_BLK_T *blk = NULL;

   if ( instance == NULL )
   {
      goto out;
   }

   mutex_lock ( &(instance->cmd_lock) );
   blk = instance->cmd_list;
   mutex_unlock ( &(instance->cmd_lock) );

   return blk;

out:
   return NULL;
}

static void *vc_vchi_sm_videocore_io( void *arg )
{
   uint32_t event_mask;
   SM_INSTANCE_T *instance = (SM_INSTANCE_T *)arg;
   int32_t success;
   uint32_t msg_len;
   VC_SM_RESULT_T *result;
   uint8_t reply_msg_buf[VC_SM_MAX_RSP_LEN];
   SM_CMD_RSP_BLK_T *waiter = NULL;
   VCOS_STATUS_T status;


   while ( 1 )
   {
      event_mask = 0;

      vchi_service_release(instance->vchi_handle[0]);
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
      vchi_service_use(instance->vchi_handle[0]);

      if ( event_mask & CLIENT_EVENT_MASK )
      {
         waiter = vc_vchi_cmd_top( instance );
         while ( waiter != NULL )
         {
            vc_vchi_unlink_cmd( instance,
                                waiter );

            success = vchi_msg_queue( instance->vchi_handle[0],
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
         success = vchi_msg_dequeue( instance->vchi_handle[0],
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
            result = (VC_SM_RESULT_T *) reply_msg_buf;
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
   }

   return NULL;
}

static void vc_sm_vchi_callback( void *param,
                                 const VCHI_CALLBACK_REASON_T reason,
                                 void *msg_handle )
{
   SM_INSTANCE_T *instance = (SM_INSTANCE_T *)param;

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

VC_VCHI_SM_HANDLE_T vc_vchi_sm_init( VCHI_INSTANCE_T vchi_instance,
                                     VCHI_CONNECTION_T **vchi_connections,
                                     uint32_t num_connections )
{
   uint32_t i;
   SM_INSTANCE_T *instance;
   VCOS_STATUS_T status;
   VCOS_THREAD_ATTR_T attrs;

   // Set up the VCOS logging
	vcos_log_register( "smem", &sm_log_category );
   vcos_log_set_level( VCOS_LOG_CATEGORY, LOG_LEVEL );

   LOG_DBG( "%s: start", __func__ );

   if ( num_connections > VCHI_MAX_NUM_CONNECTIONS )
   {
      LOG_ERR( "%s: unsupported number of connections %u (max=%u)",
               __func__, num_connections, VCHI_MAX_NUM_CONNECTIONS );

      vcos_assert( num_connections <= VCHI_MAX_NUM_CONNECTIONS );
      goto err_null;
   }

   // Allocate memory for this instance
   instance = vcos_malloc( sizeof( *instance ), "sm_instance" );
   memset( instance, 0, sizeof( *instance ));

   instance->num_connections = num_connections;

   // Create a lock for exclusive, serialized VCHI connection access
   status = vcos_semaphore_create( &instance->vchi_sema, "sm_vchi_sem", 1 );
   if ( status != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed to create vchi-sema (status=%d)", __func__, status );
      goto err_free_mem;
   }

   // Lock for command/response processing
   status = vcos_mutex_create( &instance->cmd_lock, "sm_cmd_lock" );
   if ( status != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed to create cmd-lock (status=%d)", __func__, status );
      goto err_delete_vchi_sema;
   }

   // Lock for command/response processing
   status = vcos_mutex_create( &instance->rsp_lock, "sm_rsp_lock" );
   if ( status != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed to create rsp-lock (status=%d)", __func__, status );
      goto err_delete_cmd_lock;
   }

   // Lock for io events processing
   status = vcos_mutex_create( &instance->io_lock, "sm_io_lock" );
   if ( status != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed to create io-lock (status=%d)", __func__, status );
      goto err_delete_rsp_lock;
   }

   // Event for io events processing
   status = vcos_event_create( &instance->io_event, "" );
   if ( status != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed to create io-event (status=%d)", __func__, status );
      goto err_delete_io_lock;
   }

   // Open the VCHI service connections
   for ( i = 0; i < num_connections; i++ )
   {
      SERVICE_CREATION_T params =
      {
         VC_SM_SERVER_NAME,          // 4cc service code
         vchi_connections[i],        // passed in fn pointers
         0,                          // rx fifo size
         0,                          // tx fifo size
         vc_sm_vchi_callback,        // service callback
         instance,                   // service callback parameter
         VCOS_FALSE,                 // unaligned bulk recieves
         VCOS_FALSE,                 // unaligned bulk transmits
         VCOS_FALSE,                 // want crc check on bulk transfers
      };

      status = vchi_service_open( vchi_instance,
                                  &params,
                                  &instance->vchi_handle[i] );
      if ( status != VCOS_SUCCESS )
      {
         LOG_ERR( "%s: failed to open VCHI service connection (status=%d)",
                  __func__, status );

         vcos_assert( status == VCOS_SUCCESS );
         goto err_close_services;
      }
   }

   // Create the thread which takes care of all io to/from videoocore.
   vcos_thread_attr_init( &attrs );
   vcos_thread_attr_setstacksize( &attrs, 2048 );
   vcos_thread_attr_settimeslice( &attrs, 1 );

   // Create a thread to process the incoming log messages
   status = vcos_thread_create( &instance->io_thread,
                                "Shared Memory IO",
                                &attrs,
                                vc_vchi_sm_videocore_io,
                                instance );
   if ( status != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed to create videocore io thread (status=%d)",
               __func__, status );

      vcos_assert( status == VCOS_SUCCESS );
      goto err_close_services;
   }

   LOG_DBG( "%s: success - instance 0x%x", __func__, (unsigned)instance );
   return instance;

err_close_services:
   for ( i = 0; i < instance->num_connections; i++ )
   {
      if ( instance->vchi_handle[i] != NULL )
      {
         vchi_service_close( instance->vchi_handle[i] );
      }
   }
   vcos_event_delete( &instance->io_event );
err_delete_io_lock:
   vcos_mutex_delete( &instance->io_lock );
err_delete_rsp_lock:
   vcos_mutex_delete( &instance->rsp_lock );
err_delete_cmd_lock:
   vcos_mutex_delete( &instance->cmd_lock );
err_delete_vchi_sema:
   vcos_semaphore_delete( &instance->vchi_sema );
err_free_mem:
   vcos_free( instance );
err_null:
   LOG_DBG( "%s: FAILED", __func__ );
   return NULL;
}

VCOS_STATUS_T vc_vchi_sm_stop( VC_VCHI_SM_HANDLE_T *handle )
{
   SM_INSTANCE_T *instance = *handle;
   VCOS_STATUS_T status;
   uint32_t i;

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
      vchi_service_use(instance->vchi_handle[i]);

      success = vchi_service_close( instance->vchi_handle[i] );
      vcos_assert( success == 0 );
   }

   vcos_semaphore_post( &instance->vchi_sema );
   vcos_semaphore_delete( &instance->vchi_sema );

   vcos_event_delete( &instance->io_event );

   vcos_mutex_delete( &instance->cmd_lock );
   vcos_mutex_delete( &instance->rsp_lock );
   vcos_mutex_delete( &instance->io_lock );
   vcos_free( instance );

   // NULLify the handle to prevent the user from using it
   *handle = NULL;

   // Unregister the log category so we can add it back next time
   vcos_log_unregister( &sm_log_category );

   return VCOS_SUCCESS;

lock:
   return VCOS_EINVAL;
}

VCOS_STATUS_T vc_vchi_sm_alloc( VC_VCHI_SM_HANDLE_T handle,
                                VC_SM_ALLOC_T *alloc,
                                VC_SM_ALLOC_RESULT_T *alloc_result,
                                uint32_t *cur_trans_id )
{
   VCOS_STATUS_T final = VCOS_EINVAL;
   SM_INSTANCE_T *instance = handle;
   VC_SM_MSG_HDR_T *msg_hdr;
   VCOS_STATUS_T status;
   SM_CMD_RSP_BLK_T *cmd_blk = NULL;
   SM_CMD_RSP_BLK_T *rsp_blk = NULL;

   if ( handle == NULL )
   {
      LOG_ERR( "%s: invalid handle", __func__ );

      vcos_assert( handle != NULL );
      goto lock;
   }

   if ( alloc == NULL )
   {
      LOG_ERR( "%s: invalid alloc pointer", __func__ );

      vcos_assert( alloc != NULL );
      goto lock;
   }

   if ( alloc_result == NULL )
   {
      LOG_ERR( "%s: invalid alloc_result pointer", __func__ );

      vcos_assert( alloc_result != NULL );
      goto lock;
   }

   cmd_blk = kzalloc( sizeof( *cmd_blk ), GFP_KERNEL );
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

   cmd_blk->command_len = sizeof( *msg_hdr ) + sizeof( *alloc );
   memset( cmd_blk->command_msg_buf,
           0,
           cmd_blk->command_len );

   msg_hdr           = (VC_SM_MSG_HDR_T *)cmd_blk->command_msg_buf;
   msg_hdr->type     = VC_SM_MSG_TYPE_ALLOC;
   msg_hdr->trans_id = ++instance->trans_id;
   memcpy( msg_hdr->body, alloc, sizeof( *alloc ));

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
   if ( status == VCOS_EINTR )
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
      memcpy ( alloc_result,
               rsp_blk->reply_msg_buf,
               sizeof( *alloc_result ) );
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

VCOS_STATUS_T vc_vchi_sm_free( VC_VCHI_SM_HANDLE_T handle,
                               VC_SM_FREE_T *free,
                               uint32_t *cur_trans_id )
{
   VCOS_STATUS_T final = VCOS_EINVAL;
   SM_INSTANCE_T *instance = handle;
   VC_SM_MSG_HDR_T *msg_hdr;
   VC_SM_RESULT_T *result;
   VCOS_STATUS_T status;
   SM_CMD_RSP_BLK_T *cmd_blk = NULL;
   SM_CMD_RSP_BLK_T *rsp_blk = NULL;

   if ( handle == NULL )
   {
      LOG_ERR( "%s: invalid handle", __func__ );

      vcos_assert( handle != NULL );
      goto lock;
   }

   if ( free == NULL )
   {
      LOG_ERR( "%s: invalid free pointer", __func__ );

      vcos_assert( free != NULL );
      goto lock;
   }

   cmd_blk = kzalloc( sizeof( *cmd_blk ), GFP_KERNEL );
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

   cmd_blk->command_len = sizeof( *msg_hdr ) + sizeof( *free );
   memset( cmd_blk->command_msg_buf,
           0,
           cmd_blk->command_len );

   msg_hdr           = (VC_SM_MSG_HDR_T *)cmd_blk->command_msg_buf;
   msg_hdr->type     = VC_SM_MSG_TYPE_FREE;
   msg_hdr->trans_id = ++instance->trans_id;
   memcpy( msg_hdr->body, free, sizeof( *free ));

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
   if ( status == VCOS_EINTR )
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
      result = (VC_SM_RESULT_T *) rsp_blk->reply_msg_buf;
      cmd_blk = NULL;
      final = (result->success == 0) ? VCOS_SUCCESS : VCOS_ENXIO;
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

VCOS_STATUS_T vc_vchi_sm_lock( VC_VCHI_SM_HANDLE_T handle,
                               VC_SM_LOCK_UNLOCK_T *lock_unlock,
                               VC_SM_LOCK_RESULT_T *lock_result,
                               uint32_t *cur_trans_id )
{
   VCOS_STATUS_T final = VCOS_EINVAL;
   SM_INSTANCE_T *instance = handle;
   VC_SM_MSG_HDR_T *msg_hdr;
   VCOS_STATUS_T status;
   SM_CMD_RSP_BLK_T *cmd_blk = NULL;
   SM_CMD_RSP_BLK_T *rsp_blk = NULL;

   if ( handle == NULL )
   {
      LOG_ERR( "%s: invalid handle", __func__ );

      vcos_assert( handle != NULL );
      goto lock;
   }

   if ( lock_unlock == NULL )
   {
      LOG_ERR( "%s: invalid lock_unlock pointer", __func__ );

      vcos_assert( lock_unlock != NULL );
      goto lock;
   }

   if ( lock_result == NULL )
   {
      LOG_ERR( "%s: invalid lock_result pointer", __func__ );

      vcos_assert( lock_result != NULL );
      goto lock;
   }

   cmd_blk = kzalloc( sizeof( *cmd_blk ), GFP_KERNEL );
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

   cmd_blk->command_len = sizeof( *msg_hdr ) + sizeof( *lock_unlock );
   memset( cmd_blk->command_msg_buf,
           0,
           cmd_blk->command_len );

   msg_hdr           = (VC_SM_MSG_HDR_T *)cmd_blk->command_msg_buf;
   msg_hdr->type     = VC_SM_MSG_TYPE_LOCK;
   msg_hdr->trans_id = ++instance->trans_id;
   memcpy( msg_hdr->body, lock_unlock, sizeof( *lock_unlock ));

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
   if ( status == VCOS_EINTR )
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
      memcpy ( lock_result,
               rsp_blk->reply_msg_buf,
               sizeof ( *lock_result ) );
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

VCOS_STATUS_T vc_vchi_sm_unlock( VC_VCHI_SM_HANDLE_T handle,
                                 VC_SM_LOCK_UNLOCK_T *lock_unlock,
                                 uint32_t *cur_trans_id )
{
   VCOS_STATUS_T final = VCOS_EINVAL;
   SM_INSTANCE_T *instance = handle;
   VC_SM_MSG_HDR_T *msg_hdr;
   VC_SM_RESULT_T *result;
   VCOS_STATUS_T status;
   SM_CMD_RSP_BLK_T *cmd_blk = NULL;
   SM_CMD_RSP_BLK_T *rsp_blk = NULL;

   if ( handle == NULL )
   {
      LOG_ERR( "%s: invalid handle", __func__ );

      vcos_assert( handle != NULL );
      goto lock;
   }

   if ( lock_unlock == NULL )
   {
      LOG_ERR( "%s: invalid lock_unlock pointer", __func__ );

      vcos_assert( lock_unlock != NULL );
      goto lock;
   }

   cmd_blk = kzalloc( sizeof( *cmd_blk ), GFP_KERNEL );
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

   cmd_blk->command_len = sizeof( *msg_hdr ) + sizeof( *lock_unlock );
   memset( cmd_blk->command_msg_buf,
           0,
           cmd_blk->command_len );

   msg_hdr           = (VC_SM_MSG_HDR_T *)cmd_blk->command_msg_buf;
   msg_hdr->type     = VC_SM_MSG_TYPE_UNLOCK;
   msg_hdr->trans_id = ++instance->trans_id;
   memcpy( msg_hdr->body, lock_unlock, sizeof( *lock_unlock ));

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
   if ( status == VCOS_EINTR )
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
      result = (VC_SM_RESULT_T *) rsp_blk->reply_msg_buf;
      cmd_blk = NULL;
      final = (result->success == 0) ? VCOS_SUCCESS : VCOS_ENXIO;
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

VCOS_STATUS_T vc_vchi_sm_resize( VC_VCHI_SM_HANDLE_T handle,
                                 VC_SM_RESIZE_T *resize,
                                 uint32_t *cur_trans_id )
{
   VCOS_STATUS_T final = VCOS_EINVAL;
   SM_INSTANCE_T *instance = handle;
   VC_SM_MSG_HDR_T *msg_hdr;
   VC_SM_RESULT_T *result;
   VCOS_STATUS_T status;
   SM_CMD_RSP_BLK_T *cmd_blk = NULL;
   SM_CMD_RSP_BLK_T *rsp_blk = NULL;

   if ( handle == NULL )
   {
      LOG_ERR( "%s: invalid handle", __func__ );

      vcos_assert( handle != NULL );
      goto lock;
   }

   if ( resize == NULL )
   {
      LOG_ERR( "%s: invalid resize pointer", __func__ );

      vcos_assert( resize != NULL );
      goto lock;
   }

   cmd_blk = kzalloc( sizeof( *cmd_blk ), GFP_KERNEL );
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

   cmd_blk->command_len = sizeof( *msg_hdr ) + sizeof( *resize );
   memset( cmd_blk->command_msg_buf,
           0,
           cmd_blk->command_len );

   msg_hdr           = (VC_SM_MSG_HDR_T *)cmd_blk->command_msg_buf;
   msg_hdr->type     = VC_SM_MSG_TYPE_RESIZE;
   msg_hdr->trans_id = ++instance->trans_id;
   memcpy( msg_hdr->body, resize, sizeof( *resize ));

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
      result = (VC_SM_RESULT_T *) rsp_blk->reply_msg_buf;
      cmd_blk = NULL;
      final = (result->success == 0) ? VCOS_SUCCESS : VCOS_ENXIO;
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

VCOS_STATUS_T vc_vchi_sm_walk_alloc( VC_VCHI_SM_HANDLE_T handle )
{
   VCOS_STATUS_T final = VCOS_SUCCESS;
   SM_INSTANCE_T *instance = handle;
   VC_SM_MSG_HDR_T *msg_hdr;
   VCOS_STATUS_T status;
   SM_CMD_RSP_BLK_T *cmd_blk = NULL;

   if ( handle == NULL )
   {
      LOG_ERR( "%s: invalid handle", __func__ );

      vcos_assert( handle != NULL );
      goto lock;
   }

   cmd_blk = kzalloc( sizeof( *cmd_blk ), GFP_KERNEL );
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

   cmd_blk->command_len = sizeof( *msg_hdr );
   memset( cmd_blk->command_msg_buf,
           0,
           cmd_blk->command_len );

   msg_hdr           = (VC_SM_MSG_HDR_T *)cmd_blk->command_msg_buf;
   msg_hdr->type     = VC_SM_MSG_TYPE_WALK_ALLOC;
   msg_hdr->trans_id = ++instance->trans_id;

   cmd_blk->reply_wait = 0;
   cmd_blk->trans_id   = msg_hdr->trans_id;

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
   if ( status == VCOS_EINTR )
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

   final = VCOS_SUCCESS;


unlock:
   vcos_semaphore_post( &instance->vchi_sema );
lock:
   if ( cmd_blk != NULL )
   {
      vc_vchi_rem_cmd( instance,
                       &cmd_blk );
   }
   return final;
}

VCOS_STATUS_T vc_vchi_sm_clean_up( VC_VCHI_SM_HANDLE_T handle,
                                   VC_SM_ACTION_CLEAN_T *action_clean )
{
   VCOS_STATUS_T final = VCOS_EINVAL;
   SM_INSTANCE_T *instance = handle;
   VC_SM_MSG_HDR_T *msg_hdr;
   VCOS_STATUS_T status;
   SM_CMD_RSP_BLK_T *cmd_blk = NULL;

   if ( handle == NULL )
   {
      // Go directly to the unlock stage...
      goto unlock;
   }

   cmd_blk = kzalloc( sizeof( *cmd_blk ), GFP_KERNEL );
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

   cmd_blk->command_len = sizeof( *msg_hdr ) + sizeof( *action_clean );
   memset( cmd_blk->command_msg_buf,
           0,
           cmd_blk->command_len );

   msg_hdr           = (VC_SM_MSG_HDR_T *)cmd_blk->command_msg_buf;
   msg_hdr->type     = VC_SM_MSG_TYPE_ACTION_CLEAN;
   msg_hdr->trans_id = ++instance->trans_id;
   memcpy( msg_hdr->body, action_clean, sizeof( *action_clean ));

   cmd_blk->reply_wait = 0;
   cmd_blk->trans_id   = msg_hdr->trans_id;

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
   if ( status == VCOS_EINTR )
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

   final = VCOS_SUCCESS;


unlock:
   vcos_semaphore_post( &instance->vchi_sema );
lock:
   if ( cmd_blk != NULL )
   {
      vc_vchi_rem_cmd( instance,
                       &cmd_blk );
   }
   return final;
}

/* Ask the shared memory service to free up all memory associated with a given
** allocator identifier (used for emergency cleanup).  Free attempt is one shot
** and immediate.
*/
VCOS_STATUS_T vc_vchi_sm_free_all( VC_VCHI_SM_HANDLE_T handle,
                                   VC_SM_FREE_ALL_T *free_all )
{
   VCOS_STATUS_T final = VCOS_EINVAL;
   SM_INSTANCE_T *instance = handle;
   VC_SM_MSG_HDR_T *msg_hdr;
   VCOS_STATUS_T status;
   SM_CMD_RSP_BLK_T *cmd_blk = NULL;

   if ( handle == NULL )
   {
      LOG_ERR( "%s: invalid handle", __func__ );

      vcos_assert( handle != NULL );
      goto lock;
   }

   if ( free_all == NULL )
   {
      LOG_ERR( "%s: invalid free_all pointer", __func__ );

      vcos_assert( free_all != NULL );
      goto lock;
   }

   cmd_blk = kzalloc( sizeof( *cmd_blk ), GFP_KERNEL );
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

   cmd_blk->command_len = sizeof( *msg_hdr ) + sizeof( *free_all );
   memset( cmd_blk->command_msg_buf,
           0,
           cmd_blk->command_len );

   msg_hdr           = (VC_SM_MSG_HDR_T *)cmd_blk->command_msg_buf;
   msg_hdr->type     = VC_SM_MSG_TYPE_FREE_ALL;
   msg_hdr->trans_id = ++instance->trans_id;
   memcpy( msg_hdr->body, free_all, sizeof( *free_all ));

   cmd_blk->reply_wait = 0;
   cmd_blk->trans_id   = msg_hdr->trans_id;

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
   if ( status == VCOS_EINTR )
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

   final = VCOS_SUCCESS;

unlock:
   vcos_semaphore_post( &instance->vchi_sema );
lock:
   if ( cmd_blk != NULL )
   {
      vc_vchi_rem_cmd( instance,
                       &cmd_blk );
   }
   return final;
}

/* Ask the shared memory service to free up all memory associated with a given
** allocator identifier (used for emergency cleanup).  Free attempt is progressive
** until all resources are confirmed cleaned.
*/
VCOS_STATUS_T vc_vchi_sm_free_post_mortem( VC_VCHI_SM_HANDLE_T handle,
                                           VC_SM_FREE_ALL_T *free_all )
{
   VCOS_STATUS_T final = VCOS_EINVAL;
   SM_INSTANCE_T *instance = handle;
   VC_SM_MSG_HDR_T *msg_hdr;
   VCOS_STATUS_T status;
   SM_CMD_RSP_BLK_T *cmd_blk = NULL;

   if ( handle == NULL )
   {
      LOG_ERR( "%s: invalid handle", __func__ );

      vcos_assert( handle != NULL );
      goto lock;
   }

   if ( free_all == NULL )
   {
      LOG_ERR( "%s: invalid free_all pointer", __func__ );

      vcos_assert( free_all != NULL );
      goto lock;
   }

   cmd_blk = kzalloc( sizeof( *cmd_blk ), GFP_KERNEL );
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

   cmd_blk->command_len = sizeof( *msg_hdr ) + sizeof( *free_all );
   memset( cmd_blk->command_msg_buf,
           0,
           cmd_blk->command_len );

   msg_hdr           = (VC_SM_MSG_HDR_T *)cmd_blk->command_msg_buf;
   msg_hdr->type     = VC_SM_MSG_TYPE_FREE_POST_MORTEM;
   msg_hdr->trans_id = ++instance->trans_id;
   memcpy( msg_hdr->body, free_all, sizeof( *free_all ));

   cmd_blk->reply_wait = 0;
   cmd_blk->trans_id   = msg_hdr->trans_id;

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
   if ( status == VCOS_EINTR )
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

   final = VCOS_SUCCESS;

unlock:
   vcos_semaphore_post( &instance->vchi_sema );
lock:
   if ( cmd_blk != NULL )
   {
      vc_vchi_rem_cmd( instance,
                       &cmd_blk );
   }
   return final;
}

