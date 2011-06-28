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


#include "vceb_types.h"
#include "vceb_common.h"
#include "vceb_interface.h"
#include "vceb_host_interface.h"
#include "vceb_os.h"

/******************************************************************************
Private defs
******************************************************************************/

#define VCEB_INTERFACE_DEBUG     0

/******************************************************************************
Static data
******************************************************************************/

static void *vceb_interface_semaphore = NULL;

/******************************************************************************
Global functions.
******************************************************************************/

/***********************************************************
 * Name: vceb_interface_initialise
 *
 * Arguments:  void
 *
 * Description: Routine to initialise the interface
 *
 * Returns: int ( < 0 is fail)
 *
 ***********************************************************/
int32_t vceb_interface_initialise( void )
{
   int32_t success = -1; //fail by default
   
   if( NULL == vceb_interface_semaphore )
   {
      vceb_interface_semaphore = vceb_semaphore_create( 1 );
   }

   //check for a valid semaphore
   if( NULL != vceb_interface_semaphore )
   {
      success = 0;
   }

   return success;
}

/***********************************************************
 * Name: vceb_interface_transmit
 *
 * Arguments:
 *       const uint32_t bulk_channel,
         const uint32_t data_size,
         const void * const data,
         const uint32_t terminate_transmission
 *
 * Description: Routine to transmit a message - it always rounds up the data to 16 bytes long
 *
 * Returns: int ( < 0 is fail)
 *
 ***********************************************************/
int32_t vceb_interface_transmit( VCEB_HOST_INTERFACE_INSTANCE_T host_interface_instance,
                                 const uint32_t bulk_channel,
                                 const uint32_t bytes_to_write,
                                 const void * const data,
                                 const uint32_t terminate_transmission )
{
   int32_t success = -1; //fail by default

   if( 0 == vceb_semaphore_obtain( vceb_interface_semaphore, 1 /*block*/ ) )
   {
      #if VCEB_INTERFACE_DEBUG
         vceb_printf( VCEB_PRINT_LEVEL_TRACE, "vceb_interface_transmit 1 %i\n", bytes_to_write );
      #endif

      //write the data to the specific channel
      success = vceb_host_interface_write(host_interface_instance,
                                          bulk_channel,
                                          bytes_to_write,
                                          data );

      #if VCEB_INTERFACE_DEBUG
         vceb_printf( VCEB_PRINT_LEVEL_TRACE, "vceb_interface_transmit 2 %i + (%i)\n", success, terminate_transmission );
      #endif

      //if we are flushing out the transaction, write a null buffer
      if( (success == 0) && terminate_transmission )
      {
         success = vceb_host_interface_write(host_interface_instance,
                                             bulk_channel,
                                             0,
                                             NULL );
      }

      vceb_semaphore_release( vceb_interface_semaphore );
   }
   else
   {
      #if VCEB_INTERFACE_DEBUG
         vceb_printf( VCEB_PRINT_LEVEL_ERROR, "vceb_interface_transmit FAIL\n" );
      #endif
   }

   return success;
}

/***********************************************************
 * Name: vceb_interface_receive
 *
 * Arguments:
 *       const uint32_t bulk_channel,
         const uint32_t data_size,
         void * data
 *
 * Description: Routine to receive a message
 *
 * Returns: int ( < 0 is fail)
 *
 ***********************************************************/
int32_t vceb_interface_receive(  VCEB_HOST_INTERFACE_INSTANCE_T host_interface_instance,
                                 const uint32_t bulk_channel,
                                 const uint32_t bytes_to_read,
                                 void **data,
                                 const uint32_t copy_data_not_return_ptr )
{
   int32_t success = -1; //fail by default
   static uint8_t temp_buffer[ 256 ];

   if( 0 == vceb_semaphore_obtain( vceb_interface_semaphore, 1 /*block*/ ) )
   {
      //if we want to get a ptr to the read data, fix up the buffer here
      if( !copy_data_not_return_ptr )
      {
         *data = temp_buffer;
      }

      //read data from the specific channel
      success = vceb_host_interface_read( host_interface_instance,
                                          bulk_channel,
                                          bytes_to_read,
                                          *data );

      vceb_semaphore_release( vceb_interface_semaphore );
   }
   else
   {
      #if VCEB_INTERFACE_DEBUG
         vceb_printf( VCEB_PRINT_LEVEL_ERROR, "vceb_interface_receive FAIL\n" );
      #endif
   }

   return success;
}

/* ************************************ The End ***************************************** */
