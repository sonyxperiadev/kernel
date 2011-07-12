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

// VCOS logging category for this service
#define VCOS_LOG_CATEGORY (&sm_log_category)

// Default VCOS logging level
#define LOG_LEVEL  VCOS_LOG_TRACE

// Logging macros (for remapping to other logging mechanisms, i.e., printf)
#define LOG_ERR( fmt, arg... )   vcos_log_error( fmt, ##arg )
#define LOG_WARN( fmt, arg... )  vcos_log_warn( fmt, ##arg )
#define LOG_INFO( fmt, arg... )  vcos_log_info( fmt, ##arg )
#define LOG_DBG( fmt, arg... )   vcos_log_trace( fmt, ##arg )

typedef struct opaque_vc_vchi_sm_handle_t
{
   uint32_t               num_connections;
   VCHI_SERVICE_HANDLE_T  vchi_handle[VCHI_MAX_NUM_CONNECTIONS];
   VCOS_EVENT_T           msg_avail_event;
   VCOS_MUTEX_T           vchi_mutex;

   uint8_t                msg_buf[VC_SM_MAX_MSG_LEN];
} SM_INSTANCE_T;

// ---- Private Variables ----------------------------------------------------

// VCOS logging category for this service
static VCOS_LOG_CAT_T sm_log_category;

// ---- Private Function Prototypes ------------------------------------------

// ---- Private Functions ----------------------------------------------------

static void sm_vchi_callback( void *param,
                              const VCHI_CALLBACK_REASON_T reason,
                              void *msg_handle )
{
   VCOS_EVENT_T *event = (VCOS_EVENT_T *)param;

   (void)msg_handle;

   if ( reason != VCHI_CALLBACK_MSG_AVAILABLE )
   {
      return;
   }

   vcos_event_signal( event );
}

VC_VCHI_SM_HANDLE_T vc_vchi_sm_init( VCHI_INSTANCE_T vchi_instance,
                                     VCHI_CONNECTION_T **vchi_connections,
                                     uint32_t num_connections )
{
   uint32_t i;
   SM_INSTANCE_T *instance;
   VCOS_STATUS_T status;

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

   // Create the message available event
   status = vcos_event_create( &instance->msg_avail_event, "sm_msg_avail" );
   if ( status != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed to create event (status=%d)", __func__, status );
      goto err_free_mem;
   }

   // Create a lock for exclusive, serialized VCHI connection access
   status = vcos_mutex_create( &instance->vchi_mutex, "sm_vchi_mutex" );
   if ( status != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed to create event (status=%d)", __func__, status );
      goto err_delete_event;
   }

   // Open the VCHI service connections
   for ( i = 0; i < num_connections; i++ )
   {
      SERVICE_CREATION_T params =
      {
         VC_SM_SERVER_NAME,          // 4cc service code
         vchi_connections[i],        // passed in fn pointers
         0,                          // rx fifo size (unused)
         0,                          // tx fifo size (unused)
         sm_vchi_callback,           // service callback
         &instance->msg_avail_event, // service callback parameter
         VCOS_FALSE,                 // unaligned bulk recieves
         VCOS_FALSE,                 // unaligned bulk transmits
         VCOS_FALSE,                 // want crc check on bulk transfers
      };

      status = vchi_service_open( vchi_instance, &params,
                                  &instance->vchi_handle[i] );
      if ( status != VCOS_SUCCESS )
      {
         LOG_ERR( "%s: failed to open VCHI service connection (status=%d)",
                  __func__, status );

         vcos_assert( status == VCOS_SUCCESS );
         goto err_close_services;
      }
   }

   LOG_DBG( "%s: success - instance %u", __func__, (unsigned)instance );
   return instance;

err_close_services:
   for ( i = 0; i < instance->num_connections; i++ )
   {
      vchi_service_close( instance->vchi_handle[i] );
   }

   vcos_mutex_delete( &instance->vchi_mutex );

err_delete_event:
   vcos_event_delete( &instance->msg_avail_event );

err_free_mem:
   vcos_free( instance );

err_null:
   LOG_DBG( "%s: FAILED", __func__ );
   return NULL;
}

int32_t vc_vchi_sm_stop( VC_VCHI_SM_HANDLE_T *handle )
{
   SM_INSTANCE_T *instance = *handle;
   uint32_t i;

   if ( handle == NULL )
   {
      LOG_ERR( "%s: invalid pointer to handle %p", __func__, handle );

      vcos_assert( handle != NULL );
      goto err_lock;
   }

   if ( *handle == NULL )
   {
      LOG_ERR( "%s: invalid handle %p", __func__, *handle );

      vcos_assert( *handle != NULL );
      goto err_lock;
   }

   if ( vcos_mutex_lock( &instance->vchi_mutex ) != VCOS_SUCCESS )
   {
      goto err_lock;
   }

   // Close all VCHI service connections
   for ( i = 0; i < instance->num_connections; i++ )
   {
      int32_t success;

      success = vchi_service_close( instance->vchi_handle[i] );
      vcos_assert( success == 0 );
   }

   vcos_mutex_unlock( &instance->vchi_mutex );
   vcos_mutex_delete( &instance->vchi_mutex );
   vcos_event_delete( &instance->msg_avail_event );

   vcos_free( instance );

   // NULLify the handle to prevent the user from using it
   *handle = NULL;

   // Unregister the log category so we can add it back next time
	vcos_log_unregister( &sm_log_category );

   return 0;

err_lock:
   return -1;
}

int32_t vc_vchi_sm_alloc( VC_VCHI_SM_HANDLE_T handle,
                          VC_SM_ALLOC_T *alloc,
                          VC_SM_ALLOC_RESULT_T *alloc_result )
{
   SM_INSTANCE_T *instance = handle;
   int32_t success;
   uint32_t msg_len;
   VC_SM_MSG_HDR_T *msg_hdr;
   VCOS_STATUS_T status;

   if ( handle == NULL )
   {
      LOG_ERR( "%s: invalid handle 0x%p", __func__, handle );

      vcos_assert( handle != NULL );
      goto err_lock;
   }

   if ( alloc == NULL )
   {
      LOG_ERR( "%s: invalid alloc pointer 0x%p", __func__, alloc );

      vcos_assert( alloc != NULL );
      goto err_lock;
   }

   if ( alloc_result == NULL )
   {
      LOG_ERR( "%s: invalid alloc_result pointer 0x%p", __func__, alloc_result );

      vcos_assert( alloc_result != NULL );
      goto err_lock;
   }

   if ( vcos_mutex_lock( &instance->vchi_mutex ) != VCOS_SUCCESS )
   {
      goto err_lock;
   }

   msg_len = sizeof( *msg_hdr ) + sizeof( *alloc );
   memset( instance->msg_buf, 0, msg_len );

   msg_hdr = (VC_SM_MSG_HDR_T *)instance->msg_buf;
   msg_hdr->type = VC_SM_MSG_TYPE_ALLOC;

   // Copy the user buffer into the message buffer
   memcpy( msg_hdr->body, alloc, sizeof( *alloc ));

   // Send the message to the videocore
   success = vchi_msg_queue( instance->vchi_handle[0],
                             instance->msg_buf, msg_len,
                             VCHI_FLAGS_BLOCK_UNTIL_QUEUED, NULL );
   if ( success != 0 )
   {
      LOG_ERR( "%s: failed to queue message (success=%d)",
               __func__, success );

      goto err_unlock;
   }

   // We are expecting a reply from the videocore
   status = vcos_event_wait( &instance->msg_avail_event );
   if ( status != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed on waiting for event (status=%d)",
               __func__, status );

      goto err_unlock;
   }

   success = vchi_msg_dequeue( instance->vchi_handle[0],
                               alloc_result, sizeof( *alloc_result ),
                               &msg_len, VCHI_FLAGS_NONE );
   if ( success != 0 )
   {
      LOG_ERR( "%s: failed to dequeue message (success=%d)",
               __func__, success );

      goto err_unlock;
   }
   else if ( msg_len != sizeof( *alloc_result ))
   {
      LOG_ERR( "%s: incorrect message length %u (expected=%u)",
               __func__, msg_len, sizeof( *alloc_result ));

      goto err_unlock;
   }

   vcos_mutex_unlock( &instance->vchi_mutex );
   return 0;

err_unlock:
   vcos_mutex_unlock( &instance->vchi_mutex );

err_lock:
   return -1;
}

int32_t vc_vchi_sm_free( VC_VCHI_SM_HANDLE_T handle,
                         VC_SM_FREE_T *free )
{
   int ret;
   SM_INSTANCE_T *instance = handle;
   int32_t success;
   uint32_t msg_len;
   VC_SM_MSG_HDR_T *msg_hdr;
   VC_SM_RESULT_T result;
   VCOS_STATUS_T status;

   if ( handle == NULL )
   {
      LOG_ERR( "%s: invalid handle 0x%p", __func__, handle );

      vcos_assert( handle != NULL );
      ret = -1;
      goto err_lock;
   }

   if ( free == NULL )
   {
      LOG_ERR( "%s: invalid free pointer 0x%p", __func__, free );

      vcos_assert( free != NULL );
      ret = -1;
      goto err_lock;
   }

   if ( vcos_mutex_lock( &instance->vchi_mutex ) != VCOS_SUCCESS )
   {
      ret = -1;
      goto err_lock;
   }

   msg_len = sizeof( *msg_hdr ) + sizeof( *free );
   memset( instance->msg_buf, 0, msg_len );

   msg_hdr = (VC_SM_MSG_HDR_T *)instance->msg_buf;
   msg_hdr->type = VC_SM_MSG_TYPE_FREE;

   // Copy the user buffer into the message buffer
   memcpy( msg_hdr->body, free, sizeof( *free ));

   // Send the message to the videocore
   success = vchi_msg_queue( instance->vchi_handle[0],
                             instance->msg_buf, msg_len,
                             VCHI_FLAGS_BLOCK_UNTIL_QUEUED, NULL );
   if ( success != 0 )
   {
      LOG_ERR( "%s: failed to queue message (success=%d)",
               __func__, success );

      ret = -1;
      goto unlock;
   }

   // We are expecting a reply from the videocore
   status = vcos_event_wait( &instance->msg_avail_event );
   if ( status != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed on waiting for event (status=%d)",
               __func__, status );

      ret = -1;
      goto unlock;
   }

   success = vchi_msg_dequeue( instance->vchi_handle[0],
                               &result, sizeof( result ),
                               &msg_len, VCHI_FLAGS_NONE );
   if ( success != 0 )
   {
      LOG_ERR( "%s: failed to dequeue message (success=%d)",
               __func__, success );

      ret = -1;
      goto unlock;
   }
   else if ( msg_len != sizeof( result ))
   {
      LOG_ERR( "%s: incorrect message length %u (expected=%u)",
               __func__, msg_len, sizeof( result ));

      ret = -1;
      goto unlock;
   }

   ret = result.success ? -1 : 0;

unlock:
   vcos_mutex_unlock( &instance->vchi_mutex );

err_lock:
   return ret;
}

int32_t vc_vchi_sm_lock( VC_VCHI_SM_HANDLE_T handle,
                         VC_SM_LOCK_UNLOCK_T *lock_unlock,
                         VC_SM_LOCK_RESULT_T *lock_result )
{
   int ret;
   SM_INSTANCE_T *instance = handle;
   int32_t success;
   uint32_t msg_len;
   VC_SM_MSG_HDR_T *msg_hdr;
   VCOS_STATUS_T status;

   if ( handle == NULL )
   {
      LOG_ERR( "%s: invalid handle 0x%p", __func__, handle );

      vcos_assert( handle != NULL );
      ret = -1;
      goto err_lock;
   }

   if ( lock_unlock == NULL )
   {
      LOG_ERR( "%s: invalid lock_unlock pointer 0x%p", __func__, lock_unlock );

      vcos_assert( lock_unlock != NULL );
      ret = -1;
      goto err_lock;
   }

   if ( lock_result == NULL )
   {
      LOG_ERR( "%s: invalid lock_result pointer 0x%p", __func__, lock_result );

      vcos_assert( lock_result != NULL );
      ret = -1;
      goto err_lock;
   }

   if ( vcos_mutex_lock( &instance->vchi_mutex ) != VCOS_SUCCESS )
   {
      goto err_lock;
   }

   msg_len = sizeof( *msg_hdr ) + sizeof( *lock_unlock );
   memset( instance->msg_buf, 0, msg_len );

   msg_hdr = (VC_SM_MSG_HDR_T *)instance->msg_buf;
   msg_hdr->type = VC_SM_MSG_TYPE_LOCK;

   // Copy the user buffer into the message buffer
   memcpy( msg_hdr->body, lock_unlock, sizeof( *lock_unlock ));

   // Send the message to the videocore
   success = vchi_msg_queue( instance->vchi_handle[0],
                             instance->msg_buf, msg_len,
                             VCHI_FLAGS_BLOCK_UNTIL_QUEUED, NULL );
   if ( success != 0 )
   {
      LOG_ERR( "%s: failed to queue message (success=%d)",
               __func__, success );

      ret = -1;
      goto err_unlock;
   }

   // We are expecting a reply from the videocore
   status = vcos_event_wait( &instance->msg_avail_event );
   if ( status != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed on waiting for event (status=%d)",
               __func__, status );

      ret = -1;
      goto err_unlock;
   }

   success = vchi_msg_dequeue( instance->vchi_handle[0],
                               lock_result, sizeof( *lock_result ),
                               &msg_len, VCHI_FLAGS_NONE );
   if ( success != 0 )
   {
      LOG_ERR( "%s: failed to dequeue message (success=%d)",
               __func__, success );

      ret = -1;
      goto err_unlock;
   }
   else if ( msg_len != sizeof( *lock_result ))
   {
      LOG_ERR( "%s: incorrect message length %u (expected=%u)",
               __func__, msg_len, sizeof( *lock_result ));

      ret = -1;
      goto err_unlock;
   }

   vcos_mutex_unlock( &instance->vchi_mutex );

   return 0;

err_unlock:
   vcos_mutex_unlock( &instance->vchi_mutex );

err_lock:
   return -1;
}

int32_t vc_vchi_sm_unlock( VC_VCHI_SM_HANDLE_T handle,
                           VC_SM_LOCK_UNLOCK_T *lock_unlock )
{
   int ret;
   SM_INSTANCE_T *instance = handle;
   int32_t success;
   uint32_t msg_len;
   VC_SM_MSG_HDR_T *msg_hdr;
   VC_SM_RESULT_T result;
   VCOS_STATUS_T status;

   if ( handle == NULL )
   {
      LOG_ERR( "%s: invalid handle 0x%p", __func__, handle );

      vcos_assert( handle != NULL );
      ret = -1;
      goto err_lock;
   }

   if ( lock_unlock == NULL )
   {
      LOG_ERR( "%s: invalid lock_unlock pointer 0x%p", __func__, lock_unlock );

      vcos_assert( lock_unlock != NULL );
      ret = -1;
      goto err_lock;
   }

   if ( vcos_mutex_lock( &instance->vchi_mutex ) != VCOS_SUCCESS )
   {
      ret = -1;
      goto err_lock;
   }

   msg_len = sizeof( *msg_hdr ) + sizeof( *lock_unlock );
   memset( instance->msg_buf, 0, msg_len );

   msg_hdr = (VC_SM_MSG_HDR_T *)instance->msg_buf;
   msg_hdr->type = VC_SM_MSG_TYPE_UNLOCK;

   // Copy the user buffer into the message buffer
   memcpy( msg_hdr->body, lock_unlock, sizeof( *lock_unlock ));

   // Send the message to the videocore
   success = vchi_msg_queue( instance->vchi_handle[0],
                             instance->msg_buf, msg_len,
                             VCHI_FLAGS_BLOCK_UNTIL_QUEUED, NULL );
   if ( success != 0 )
   {
      LOG_ERR( "%s: failed to queue message (success=%d)",
               __func__, success );

      ret = -1;
      goto unlock;
   }

   // We are expecting a reply from the videocore
   status = vcos_event_wait( &instance->msg_avail_event );
   if ( status != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed on waiting for event (status=%d)",
               __func__, status );

      ret = -1;
      goto unlock;
   }

   success = vchi_msg_dequeue( instance->vchi_handle[0],
                               &result, sizeof( result ),
                               &msg_len, VCHI_FLAGS_NONE );
   if ( success != 0 )
   {
      LOG_ERR( "%s: failed to dequeue message (success=%d)",
               __func__, success );

      ret = -1;
      goto unlock;
   }
   else if ( msg_len != sizeof( result ))
   {
      LOG_ERR( "%s: incorrect message length %u (expected=%u)",
               __func__, msg_len, sizeof( result ));

      ret = -1;
      goto unlock;
   }

   ret = result.success ? -1 : 0;

unlock:
   vcos_mutex_unlock( &instance->vchi_mutex );

err_lock:
   return ret;
}

int32_t vc_vchi_sm_resize( VC_VCHI_SM_HANDLE_T handle,
                           VC_SM_RESIZE_T *resize )
{
   int ret;
   SM_INSTANCE_T *instance = handle;
   int32_t success;
   uint32_t msg_len;
   VC_SM_MSG_HDR_T *msg_hdr;
   VC_SM_RESULT_T result;
   VCOS_STATUS_T status;

   if ( handle == NULL )
   {
      LOG_ERR( "%s: invalid handle 0x%p", __func__, handle );

      vcos_assert( handle != NULL );
      ret = -1;
      goto err_lock;
   }

   if ( resize == NULL )
   {
      LOG_ERR( "%s: invalid resize pointer 0x%p", __func__, resize );

      vcos_assert( resize != NULL );
      ret = -1;
      goto err_lock;
   }

   if ( vcos_mutex_lock( &instance->vchi_mutex ) != VCOS_SUCCESS )
   {
      ret = -1;
      goto err_lock;
   }

   msg_len = sizeof( *msg_hdr ) + sizeof( *resize );
   memset( instance->msg_buf, 0, msg_len );

   msg_hdr = (VC_SM_MSG_HDR_T *)instance->msg_buf;
   msg_hdr->type = VC_SM_MSG_TYPE_RESIZE;

   // Copy the user buffer into the message buffer
   memcpy( msg_hdr->body, resize, sizeof( *resize ));

   // Send the message to the videocore
   success = vchi_msg_queue( instance->vchi_handle[0],
                             instance->msg_buf, msg_len,
                             VCHI_FLAGS_BLOCK_UNTIL_QUEUED, NULL );
   if ( success != 0 )
   {
      LOG_ERR( "%s: failed to queue message (success=%d)",
               __func__, success );

      ret = -1;
      goto unlock;
   }

   // We are expecting a reply from the videocore
   status = vcos_event_wait( &instance->msg_avail_event );
   if ( status != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed on waiting for event (status=%d)",
               __func__, status );

      ret = -1;
      goto unlock;
   }

   success = vchi_msg_dequeue( instance->vchi_handle[0],
                               &result, sizeof( result ),
                               &msg_len, VCHI_FLAGS_NONE );
   if ( success != 0 )
   {
      LOG_ERR( "%s: failed to dequeue message (success=%d)",
               __func__, success );

      ret = -1;
      goto unlock;
   }
   else if ( msg_len != sizeof( result ))
   {
      LOG_ERR( "%s: incorrect message length %u (expected=%u)",
               __func__, msg_len, sizeof( result ));

      ret = -1;
      goto unlock;
   }

   ret = result.success ? -1 : 0;

unlock:
   vcos_mutex_unlock( &instance->vchi_mutex );

err_lock:
   return ret;
}

int32_t vc_vchi_sm_walk_alloc( VC_VCHI_SM_HANDLE_T handle )
{
   int ret;
   SM_INSTANCE_T *instance = handle;
   int32_t success;
   uint32_t msg_len;
   VC_SM_MSG_HDR_T *msg_hdr;
   VC_SM_RESULT_T result;
   VCOS_STATUS_T status;

   if ( handle == NULL )
   {
      LOG_ERR( "%s: invalid handle 0x%p", __func__, handle );

      vcos_assert( handle != NULL );
      ret = -1;
      goto err_lock;
   }

   if ( vcos_mutex_lock( &instance->vchi_mutex ) != VCOS_SUCCESS )
   {
      ret = -1;
      goto err_lock;
   }

   msg_len = sizeof( *msg_hdr );
   memset( instance->msg_buf, 0, msg_len );

   msg_hdr = (VC_SM_MSG_HDR_T *)instance->msg_buf;
   msg_hdr->type = VC_SM_MSG_TYPE_WALK_ALLOC;

   // Send the message to the videocore
   success = vchi_msg_queue( instance->vchi_handle[0],
                             instance->msg_buf, msg_len,
                             VCHI_FLAGS_BLOCK_UNTIL_QUEUED, NULL );
   if ( success != 0 )
   {
      LOG_ERR( "%s: failed to queue message (success=%d)",
               __func__, success );

      ret = -1;
      goto unlock;
   }

   // We are expecting a reply from the videocore
   status = vcos_event_wait( &instance->msg_avail_event );
   if ( status != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed on waiting for event (status=%d)",
               __func__, status );

      ret = -1;
      goto unlock;
   }

   success = vchi_msg_dequeue( instance->vchi_handle[0],
                               &result, sizeof( result ),
                               &msg_len, VCHI_FLAGS_NONE );
   if ( success != 0 )
   {
      LOG_ERR( "%s: failed to dequeue message (success=%d)",
               __func__, success );

      ret = -1;
      goto unlock;
   }
   else if ( msg_len != sizeof( result ))
   {
      LOG_ERR( "%s: incorrect message length %u (expected=%u)",
               __func__, msg_len, sizeof( result ));

      ret = -1;
      goto unlock;
   }

   ret = result.success ? -1 : 0;

unlock:
   vcos_mutex_unlock( &instance->vchi_mutex );

err_lock:
   return ret;
}

