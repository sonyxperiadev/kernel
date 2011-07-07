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

//#include "interface/vchi/os/os.h"
#include "interface/vcos/vcos.h"
#include "vc_vchi_fb.h"

// ---- Private Constants and Types ------------------------------------------

// VCOS logging category for this service
#define VCOS_LOG_CATEGORY (&fb_log_category)

// Default VCOS logging level
#define LOG_LEVEL  VCOS_LOG_TRACE

// Logging macros (for remapping to other logging mechanisms, i.e., printf)
#define LOG_ERR( fmt, arg... )   vcos_log_error( fmt, ##arg )
#define LOG_WARN( fmt, arg... )  vcos_log_warn( fmt, ##arg )
#define LOG_INFO( fmt, arg... )  vcos_log_info( fmt, ##arg )
#define LOG_DBG( fmt, arg... )   vcos_log_trace( fmt, ##arg )

typedef struct opaque_vc_vchi_fb_handle_t
{
   uint32_t               num_connections;
   VCHI_SERVICE_HANDLE_T  vchi_handle[VCHI_MAX_NUM_CONNECTIONS];
   VCOS_EVENT_T           msg_avail_event;
   VCOS_MUTEX_T           vchi_mutex;

   uint8_t                msg_buf[VC_FB_MAX_MSG_LEN];
} FB_INSTANCE_T;

// ---- Private Variables ----------------------------------------------------

// VCOS logging category for this service
static VCOS_LOG_CAT_T fb_log_category;

// ---- Private Function Prototypes ------------------------------------------

// ---- Private Functions ----------------------------------------------------

static void fb_vchi_callback( void *param,
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

VC_VCHI_FB_HANDLE_T vc_vchi_fb_init( VCHI_INSTANCE_T vchi_instance,
                                     VCHI_CONNECTION_T **vchi_connections,
                                     uint32_t num_connections )
{
   uint32_t i;
   FB_INSTANCE_T *instance;
   VCOS_STATUS_T status;

   // Set up the VCOS logging
	vcos_log_register( "fb", &fb_log_category );
   vcos_log_set_level( VCOS_LOG_CATEGORY, LOG_LEVEL );

   LOG_DBG( "%s: start", __func__ );

   if ( num_connections > VCHI_MAX_NUM_CONNECTIONS )
   {
      LOG_ERR( "%s: unsupported number of connections %u (max=%u)",
               __func__, num_connections, VCHI_MAX_NUM_CONNECTIONS );

      vcos_assert( num_connections <= VCHI_MAX_NUM_CONNECTIONS );
      return NULL;
   }

   // Allocate memory for this instance
   instance = vcos_malloc( sizeof( *instance ), "fb_instance" );
   memset( instance, 0, sizeof( *instance ));

   instance->num_connections = num_connections;

   // Create the message available event
   status = vcos_event_create( &instance->msg_avail_event, "fb_msg_avail" );
   if ( status != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed to create event (status=%d)", __func__, status );

      goto err_free_mem;
   }

   // Create a lock for exclusive, serialized VCHI connection access
   status = vcos_mutex_create( &instance->vchi_mutex, "fb_vchi_mutex" );
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
         VC_FB_SERVER_NAME,          // 4cc service code
         vchi_connections[i],        // passed in fn pointers
         0,                          // rx fifo size (unused)
         0,                          // tx fifo size (unused)
         fb_vchi_callback,           // service callback
         &instance->msg_avail_event, // service callback parameter
         VCOS_FALSE,                 // unaligned bulk recieves
         VCOS_FALSE,                 // unaligned bulk transmits
         VCOS_FALSE                  // want crc check on bulk transfers
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

   return NULL;
}

int32_t vc_vchi_fb_stop( VC_VCHI_FB_HANDLE_T *handle )
{
   FB_INSTANCE_T *instance = *handle;
   uint32_t i;

   if ( handle == NULL )
   {
      LOG_ERR( "%s: invalid pointer to handle %p", __func__, handle );

      vcos_assert( handle != NULL );
      return -1;
   }

   if ( *handle == NULL )
   {
      LOG_ERR( "%s: invalid handle %p", __func__, *handle );

      vcos_assert( *handle != NULL );
      return -1;
   }

   vcos_mutex_lock( &instance->vchi_mutex );

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
	vcos_log_unregister( &fb_log_category );

   return 0;
}

int32_t vc_vchi_fb_get_scrn_info( VC_VCHI_FB_HANDLE_T handle,
                                  VC_FB_SCRN scrn,
                                  VC_FB_SCRN_INFO_T *info )
{
   int ret;
   FB_INSTANCE_T *instance = handle;
   int32_t success;
   uint32_t msg_len;
   VC_FB_MSG_HDR_T *msg_hdr;
   VC_FB_GET_SCRN_INFO_T *get_scrn_info;
   VCOS_STATUS_T status;

   if ( handle == NULL )
   {
      LOG_ERR( "%s: invalid handle %p", __func__, handle );

      vcos_assert( handle != NULL );
      return -1;
   }

   if ( scrn >= VC_FB_SCRN_MAX )
   {
      LOG_ERR( "%s: invalid screen %u", __func__, scrn );

      vcos_assert( scrn < VC_FB_SCRN_MAX );
      return -1;
   }

   if ( info == NULL )
   {
      LOG_ERR( "%s: invalid info pointer %p", __func__, info );

      vcos_assert( info != NULL );
      return -1;
   }

   vcos_mutex_lock( &instance->vchi_mutex );

   msg_len = sizeof( *msg_hdr ) + sizeof( *get_scrn_info );
   memset( instance->msg_buf, 0, msg_len );

   msg_hdr = (VC_FB_MSG_HDR_T *)instance->msg_buf;
   msg_hdr->type = VC_FB_MSG_TYPE_GET_SCRN_INFO;

   get_scrn_info = (VC_FB_GET_SCRN_INFO_T *)msg_hdr->body;
   get_scrn_info->scrn = scrn;

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
                               info, sizeof( *info ),
                               &msg_len, VCHI_FLAGS_NONE );
   if ( success != 0 )
   {
      LOG_ERR( "%s: failed to dequeue message (success=%d)",
               __func__, success );

      ret = -1;
      goto unlock;
   }
   else if ( msg_len != sizeof( *info ))
   {
      LOG_ERR( "%s: incorrect message length %u (expected=%u)",
               __func__, msg_len, sizeof( *info ));

      ret = -1;
      goto unlock;
   }

   ret = 0;

unlock:
   vcos_mutex_unlock( &instance->vchi_mutex );

   return ret;
}

int32_t vc_vchi_fb_alloc( VC_VCHI_FB_HANDLE_T handle,
                          VC_FB_ALLOC_T *alloc,
                          VC_FB_ALLOC_RESULT_T *alloc_result )
{
   FB_INSTANCE_T *instance = handle;
   int32_t success;
   uint32_t msg_len;
   VC_FB_MSG_HDR_T *msg_hdr;
   VCOS_STATUS_T status;

   if ( handle == NULL )
   {
      LOG_ERR( "%s: invalid handle %p", __func__, handle );

      vcos_assert( handle != NULL );
      return -1;
   }

   if ( alloc == NULL )
   {
      LOG_ERR( "%s: invalid alloc pointer %p", __func__, alloc );

      vcos_assert( alloc != NULL );
      return -1;
   }

   //TODO check individual alloc member values

   if ( alloc_result == NULL )
   {
      LOG_ERR( "%s: invalid alloc_result pointer 0x%p", __func__, alloc_result );

      vcos_assert( alloc_result != NULL );
      return -1;
   }

   vcos_mutex_lock( &instance->vchi_mutex );

   msg_len = sizeof( *msg_hdr ) + sizeof( *alloc );
   memset( instance->msg_buf, 0, msg_len );

   msg_hdr = (VC_FB_MSG_HDR_T *)instance->msg_buf;
   msg_hdr->type = VC_FB_MSG_TYPE_ALLOC;

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

   return -1;
}

int32_t vc_vchi_fb_free( VC_VCHI_FB_HANDLE_T handle,
                         uint32_t res_handle )
{
   int ret;
   FB_INSTANCE_T *instance = handle;
   int32_t success;
   uint32_t msg_len;
   VC_FB_MSG_HDR_T *msg_hdr;
   VC_FB_FREE_T *free;
   VC_FB_RESULT_T result;
   VCOS_STATUS_T status;

   if ( handle == NULL )
   {
      LOG_ERR( "%s: invalid handle %p", __func__, handle );

      vcos_assert( handle != NULL );
      return -1;
   }

   vcos_mutex_lock( &instance->vchi_mutex );

   msg_len = sizeof( *msg_hdr ) + sizeof( *free );
   memset( instance->msg_buf, 0, msg_len );

   msg_hdr = (VC_FB_MSG_HDR_T *)instance->msg_buf;
   msg_hdr->type = VC_FB_MSG_TYPE_FREE;

   free = (VC_FB_FREE_T *)msg_hdr->body;
   free->res_handle = res_handle;

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

   return ret;
}

int32_t vc_vchi_fb_pan( VC_VCHI_FB_HANDLE_T handle,
                        uint32_t res_handle,
                        uint32_t y_offset )
{
   int ret;
   FB_INSTANCE_T *instance = handle;
   int32_t success;
   uint32_t msg_len;
   VC_FB_MSG_HDR_T *msg_hdr;
   VC_FB_PAN_T *pan;
   VC_FB_RESULT_T result;
   VCOS_STATUS_T status;

   if ( handle == NULL )
   {
      LOG_ERR( "%s: invalid handle %p", __func__, handle );

      vcos_assert( handle != NULL );
      return -1;
   }

   vcos_mutex_lock( &instance->vchi_mutex );

   msg_len = sizeof( *msg_hdr ) + sizeof( *pan );
   memset( instance->msg_buf, 0, msg_len );

   msg_hdr = (VC_FB_MSG_HDR_T *)instance->msg_buf;
   msg_hdr->type = VC_FB_MSG_TYPE_PAN;

   pan = (VC_FB_PAN_T *)msg_hdr->body;
   pan->res_handle = res_handle;
   pan->y_offset = y_offset;

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

   return ret;
}

int32_t vc_vchi_fb_swap( VC_VCHI_FB_HANDLE_T handle,
                         uint32_t res_handle,
                         uint32_t active_frame )
{
   int ret;
   FB_INSTANCE_T *instance = handle;
   int32_t success;
   uint32_t msg_len;
   VC_FB_MSG_HDR_T *msg_hdr;
   VC_FB_SWAP_T *swap;
   VC_FB_RESULT_T result;
   VCOS_STATUS_T status;

   if ( handle == NULL )
   {
      LOG_ERR( "%s: invalid handle %p", __func__, handle );

      vcos_assert( handle != NULL );
      return -1;
   }

   vcos_mutex_lock( &instance->vchi_mutex );

   msg_len = sizeof( *msg_hdr ) + sizeof( *swap );
   memset( instance->msg_buf, 0, msg_len );

   msg_hdr = (VC_FB_MSG_HDR_T *)instance->msg_buf;
   msg_hdr->type = VC_FB_MSG_TYPE_SWAP;

   swap = (VC_FB_SWAP_T *)msg_hdr->body;
   swap->res_handle = res_handle;
   swap->active_frame = active_frame;

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

   return ret;
}
