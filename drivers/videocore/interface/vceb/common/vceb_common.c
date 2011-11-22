/*****************************************************************************
* Copyright 2008 - 2010 Broadcom Corporation.  All rights reserved.
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


#if defined( __KERNEL__ )
   #include <linux/memory.h>
#else
   #include <memory.h>
#endif

#include "vceb_types.h"
#include "vceb_common.h"
#include "vceb_interface.h"

/******************************************************************************
Private defs
******************************************************************************/

//TODO - add in endian support

static void VCHI_PACK_8( uint8_t **mem, uint8_t data8 )   { memcpy( *mem, &data8, 1); *mem += 1; }
static void VCHI_PACK_16( uint8_t **mem, uint16_t data16 ) { memcpy( *mem, &data16, 2); *mem += 2;  }
static void VCHI_PACK_32( uint8_t **mem, uint32_t data32 ) { memcpy( *mem, &data32, 4); *mem += 4;  }

static void VCHI_UNPACK_8( uint8_t **mem, uint8_t *data8 )    { memcpy( data8, *mem, 1); *mem += 1;  }
static void VCHI_UNPACK_16( uint8_t **mem, uint16_t *data16 )  { memcpy( data16, *mem, 2); *mem += 2;  }
static void VCHI_UNPACK_32( uint8_t **mem, uint32_t *data32 )  { memcpy( data32, *mem, 4); *mem += 4;  }

/******************************************************************************
Static data
******************************************************************************/

/******************************************************************************
Global functions.
******************************************************************************/

/***********************************************************
 * Name: vceb_message_send
 *
 * Arguments:
 *       void
 *
 * Description: Routine to clear the framebuffer
 *
 * Returns: int ( < 0 is fail)
 *
 ***********************************************************/
int32_t vceb_message_send( VCEB_HOST_INTERFACE_INSTANCE_T host_interface_instance,
                           const VCEB_MSG_T message,
                           const VCEB_FLAGS_T flags,
                           const uint32_t param_0,
                           const uint32_t param_1,
                           const uint32_t size_of_optional_data_in_bytes,
                           const void * const data,
                           const uint32_t terminate_transfer )
{
   int32_t success = -1; //fail by default
   uint8_t packed_message[ VCEB_MSG_HEADER_SIZE_IN_BYTES ];
   uint8_t *packed_message_ptr = (uint8_t *)&packed_message[ 0 ];
   uint8_t *crc_ptr = NULL;
   int data_rounded_up = (size_of_optional_data_in_bytes + 15) & ~15;

   //pack the message
   VCHI_PACK_8( &packed_message_ptr, (uint8_t)message );
   VCHI_PACK_8( &packed_message_ptr, (uint8_t)flags );

   //store away the crc pointer
   crc_ptr = packed_message_ptr;

   VCHI_PACK_16( &packed_message_ptr, 0 ); //todo - add in crc

   //message size
   VCHI_PACK_32( &packed_message_ptr, (uint32_t)(VCEB_MSG_HEADER_SIZE_IN_BYTES + size_of_optional_data_in_bytes) );   

   //params
   VCHI_PACK_32( &packed_message_ptr, (uint32_t)param_0 );
   VCHI_PACK_32( &packed_message_ptr, (uint32_t)param_1 );

   //send the bulk data first if required
   if (flags & VCEB_FLAGS_USES_BULK_CHANNEL)
   {
      success = vceb_interface_transmit(  host_interface_instance,
                                          1, //bulk channel
                                          data_rounded_up,
                                          data,
                                          1 ); //terminate now
   }

   //now transmit this message over the control channel
   success = vceb_interface_transmit(  host_interface_instance,
                                       0, //control channel
                                       VCEB_MSG_HEADER_SIZE_IN_BYTES,
                                       packed_message,
                                       ((flags & VCEB_FLAGS_USES_BULK_CHANNEL) || (0 == size_of_optional_data_in_bytes)) && terminate_transfer ); //terminate if bulk or no payload

   //any tail data to transmit?
   if( !(flags & VCEB_FLAGS_USES_BULK_CHANNEL) && (0 != size_of_optional_data_in_bytes) )
   {
      success = vceb_interface_transmit(  host_interface_instance,
                                          0, //control channel
                                          size_of_optional_data_in_bytes,
                                          data,
                                          terminate_transfer ); //terminate now
   }

   return success;
}

/***********************************************************
 * Name: vceb_message_check_receive
 *
 * Arguments:
 *       void
 *
 * Description: Routine to clear the framebuffer
 *
 * Returns: int ( < 0 is fail)
 *
 ***********************************************************/
int32_t vceb_message_receive( VCEB_HOST_INTERFACE_INSTANCE_T host_interface_instance,
                              VCEB_MSG_T *message,
                              VCEB_FLAGS_T *flags,
                              uint32_t *param_0,
                              uint32_t *param_1,
                              uint32_t *size_of_optional_data_in_bytes,
                              void ** data )
{
   int32_t success = -1; //fail by default
   static uint8_t packed_message[ VCEB_MSG_HEADER_SIZE_IN_BYTES ];
   uint8_t *packed_message_ptr = (uint8_t *)&packed_message[ 0 ];
   void *packed_message_ptr_void = packed_message_ptr;

   //fill in the message with stuff
   memset( packed_message, 0x5A, sizeof( packed_message ) );

   success = vceb_interface_receive(host_interface_instance,
                                    0, //control channel
                                    sizeof( packed_message ),
                                    &packed_message_ptr_void,
                                    1 /* copy data into this buffer */ );

   if( success == 16 )
   {
      uint16_t crc = 0;
      uint32_t message_size = 0;
      uint8_t msg8 = 0;
      uint8_t flags8 = 0;

      //unpack the message
      VCHI_UNPACK_8( &packed_message_ptr, &msg8 );
      VCHI_UNPACK_8( &packed_message_ptr, &flags8 );

      *message = (VCEB_MSG_T)msg8;
      *flags = (VCEB_FLAGS_T)flags8;

      VCHI_UNPACK_16( &packed_message_ptr, &crc ); //todo - add in crc

      //message size
      VCHI_UNPACK_32( &packed_message_ptr, &message_size );

      //params
      VCHI_UNPACK_32( &packed_message_ptr, param_0 );
      VCHI_UNPACK_32( &packed_message_ptr, param_1 );

      //any extra data to receive?
      if( message_size > VCEB_MSG_HEADER_SIZE_IN_BYTES )
      {
         *size_of_optional_data_in_bytes = vceb_interface_receive(host_interface_instance,
                                                                  (*flags & VCEB_FLAGS_USES_BULK_CHANNEL),
                                                                  message_size - VCEB_MSG_HEADER_SIZE_IN_BYTES,
                                                                  data,
                                                                  0 /* return pointer, not copy data */ );

         success += *size_of_optional_data_in_bytes;
      }
   }

   return success;
}

/***********************************************************
 * Name: vceb_message_check_receive
 *
 * Arguments:
 *       void
 *
 * Description: Routine to clear the framebuffer
 *
 * Returns: int ( < 0 is fail)
 *
 ***********************************************************/
int32_t vceb_message_check_receive( VCEB_HOST_INTERFACE_INSTANCE_T host_interface_instance,
                                    const VCEB_MSG_T message,
                                    const VCEB_FLAGS_T flags,
                                    const uint32_t param_0,
                                    const uint32_t param_1,
                                    uint32_t *size_of_optional_data_in_bytes,
                                    void * data )
{
   int32_t success = -1; //fail by default
   VCEB_MSG_T rcvd_message = 0;
   VCEB_FLAGS_T rcvd_flags = 0;
   uint32_t rcvd_param_0 = 0;
   uint32_t rcvd_param_1 = 0;

   //read the message from the control channel
   success = vceb_message_receive(  host_interface_instance,
                                    &rcvd_message, &rcvd_flags, &rcvd_param_0, &rcvd_param_1, size_of_optional_data_in_bytes, data );

   if( success == 0 )
   {
      //check the parameters
      if( (rcvd_message == message) && (rcvd_flags == flags) && (rcvd_param_0 == param_0) && (rcvd_param_1 == param_1) )
      {
         //todo - data in bytes received!
         success = 0;
      }
   }

   return success;
}

/* ************************************ The End ***************************************** */
