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
   #include <linux/string.h>
   #include <linux/kernel.h>
#else
   #include <string.h>  /* for strlen, strcmp */
#endif

#include "vceb_types.h"
#include "vceb_common.h"
#include "common/vceb_host_interface.h"

//2nd stage code defs
#include "common/vceb_2nd_stage_code.h"
#include "common/vceb_app_code.h"

//vceb app defs
#include "../common/vceb_interface.h"

#include "vceb_os.h"
#include "vceb.h"

/******************************************************************************
Debug control
******************************************************************************/

#define VCEB_DEBUG      1

/******************************************************************************
Private defs
******************************************************************************/

#define HOSTPORT_COMMS_HEADER                      0xAFE45251
#define HOSTPORT_COMMS_FOOTER                      0xF001BC4E

typedef enum
{
   HOSTPORT_MESSAGE_ID_MIN,

   HOSTPORT_MESSAGE_ID_READY_FOR_NEXT_IMAGE,
   HOSTPORT_MESSAGE_ID_HOST_HAS_SENT_NEXT_IMAGE,
   HOSTPORT_MESSAGE_ID_NEXT_IMAGE_VERIFIED, // Only now sent on failure

   HOSTPORT_MESSAGE_ID_MAX

} HOSTPORT_MESSAGE_ID_T;


typedef struct
{
  uint32_t header;
  uint32_t id_info;
  uint32_t signature[5];
  uint32_t footer;

} VCEB_BOOT_MESSAGE_T;

//NOTE - this currently only supports integers
// Could be made to support anything using some printf format
// strings and pointers however
typedef struct
{
   const char * const name;
   uint32_t *value;

} VCEB_STAT_T;

//Structure to store the vceb instance state in
struct opaque_vceb_instance_t
{
   //is this vceb instance already initialised? stops multiple users trashing the bus
   uint32_t vceb_initialised;

   uint32_t vceb_app_loaded;
   
   //the instance name / key
   char name[VCEB_MAX_INSTANCE_NAME];

   uint32_t vceb_frame_count;

   VCEB_HOST_INTERFACE_CREATE_PARAMS_T createParams;
   
   //the host interface instance
   VCEB_HOST_INTERFACE_INSTANCE_T host_interface_instance;

   //ptr to the next instance
   struct opaque_vceb_instance_t *next;

};

/******************************************************************************
Static Data
******************************************************************************/

int vceb_enable_dma = 0;

static VCEB_INSTANCE_T vceb_instance_head = NULL;

/******************************************************************************
Static funcs
******************************************************************************/

static int32_t vceb_wait_ready_for_next_stage( VCEB_HOST_INTERFACE_INSTANCE_T host_interface_instance );

static int32_t vceb_wait_ready_for_suspend_ack( VCEB_HOST_INTERFACE_INSTANCE_T host_interface_instance );

static int32_t vceb_load_next_stage_loader(  VCEB_HOST_INTERFACE_INSTANCE_T host_interface_instance,
                                             const void * const data,
                                             const uint32_t data_size,
                                             const uint32_t attemp_to_exit_suspend );

/******************************************************************************
Global functions.
******************************************************************************/


/***********************************************************
 * Name: vceb_get_instance
 *
 * Arguments:
 *       const char *instance_name  - Name of instance to retrieve
 *       VCEB_INSTANCE_T *instance  - Place to store resulting instance
 *
 * Description:   Retrieves a previously created instance.
 *
 * Returns: int ( < 0 is fail)
 *
 ***********************************************************/

int32_t vceb_get_instance( const char *instance_name, VCEB_INSTANCE_T *instance )
{
    VCEB_INSTANCE_T scan;

    scan = vceb_instance_head;
    while (( scan != NULL ) && ( strcmp( scan->name, instance_name ) != 0 ))
    {
        scan = scan->next;
    }
    *instance = scan;
    return scan == NULL ? -1 : 0;
}

/***********************************************************
 * Name: vceb_get_instance_name
 *
 * Arguments:
 *       VCEB_INSTANCE_T *instance  - Place to store resulting instance
 *
 * Description:   Retrieves the name of the vceb instance
 *
 ***********************************************************/

const char *vceb_get_instance_name( VCEB_INSTANCE_T instance )
{
    return instance->name;
}

/***********************************************************
 * Name: vceb_create_instance
 *
 * Arguments:
 *       const char *param,
         VCEB_INSTANCE_T *instanc
 *
 * Description:   Routine to initialise the vceb layer
 *                Must be called before any other functions will work.
 *                This layer loads the 2nd + 3rd stage loaders and sends a 'hello' message to VideoCore
 *
 * Returns: int ( < 0 is fail)
 *
 ***********************************************************/
int32_t vceb_create_instance( const VCEB_HOST_INTERFACE_CREATE_PARAMS_T *createParams,
                              VCEB_HOST_INTERFACE_INSTANCE_T host_interface_instance,
                              VCEB_INSTANCE_T *instance )
{
   VCEB_INSTANCE_T state;

   if (( instance == NULL )
   ||  ( createParams == NULL )
   ||  ( createParams->instance_name == NULL ))
   {
        #if VCEB_DEBUG
         vceb_printf( VCEB_PRINT_LEVEL_ERROR, "vceb_create_instance - bad input\n" );
        #endif   
       return -1;
   }
   *instance = NULL;

   #if VCEB_DEBUG
      vceb_printf( VCEB_PRINT_LEVEL_TRACE, "vceb_create_instance( %s, %s, 0x%p, xxx )\n", 
                   createParams->instance_name, 
                   createParams->videocore_param,
                   createParams->host_param );
   #endif
   
   if ( vceb_get_instance( createParams->instance_name, instance ) == 0 )
   {
      #if VCEB_DEBUG
      vceb_printf( VCEB_PRINT_LEVEL_ERROR, "vceb_create_instance - instance '%s' already exists\n", 
                   createParams->instance_name );
      #endif
      return -1;
   }

   state = vceb_calloc( sizeof( *state ) );
   if( NULL != state )
   {
      state->createParams = *createParams;
      state->host_interface_instance = host_interface_instance;

      //copy in the instance name
      //ignore the return value - assume that a trucated result is ok
      vceb_strlcpy( state->name, createParams->instance_name, sizeof( state->name ) );
   
      *instance = state;
   
      //insert this into the list
      if (vceb_instance_head == NULL)
      {
         vceb_instance_head = state;
      }
      else
      {
         vceb_instance_head->next = state;
      }
   }

   return 0;
}

/***********************************************************
 * Name: vceb_delete_instance
 *
 * Arguments:
 *       VCEB_INSTANCE_T    instance                - vceb instance
 *
 * Description:   Deletes an instance of a videocore which
 *                is discovered to not actually exist.
 *
 * Returns:  First instance in list.
 *
 ***********************************************************/
int32_t vceb_delete_instance( VCEB_INSTANCE_T instance )
{
    VCEB_INSTANCE_T prev;
    VCEB_INSTANCE_T scan;

    // Scan through the list and find the particular instance

    prev = NULL;
    scan = vceb_instance_head;
    while (( scan != NULL ) && ( scan != instance ))
    {
        prev = scan;
        scan = scan->next;
    }
    if ( scan != instance )
    {
        // Instance not found

        return -1;
    }

    // Unlink the instance from the list.

    if ( prev == NULL )
    {
        vceb_instance_head = scan->next;
    }
    else
    {
        prev->next = scan->next;
    }
    scan->next = NULL;

    // Free any resources

    vceb_free( instance );
    return 0;
}

/***********************************************************
 * Name: vceb_get_first_instance
 *
 * Arguments:
 *       VCEB_INSTANCE_T    instance                - vceb instance
 *
 * Description:   Returns a pointer to the first instance
 *                in the list of created instances.
 *
 * Returns:  First instance in list.
 *
 ***********************************************************/
VCEB_INSTANCE_T vceb_get_first_instance( void )
{
    return vceb_instance_head;
}

/***********************************************************
 * Name: vceb_get_next_instance
 *
 * Arguments:
 *       VCEB_INSTANCE_T    instance                - vceb instance
 *
 * Description:   Returns the next instance in the list 
 *                of created instances.
 *
 * Returns:  Next instance in list.
 *
 ***********************************************************/
VCEB_INSTANCE_T vceb_get_next_instance( VCEB_INSTANCE_T instance )
{
    return instance->next;
}

/***********************************************************
 * Name: vceb_get_videocore_command_line
 *
 * Arguments:
 *       VCEB_INSTANCE_T    instance    - vceb instance
 *
 * Returns: Returns the videocore command line parameters
 *          passed into vceb_create_instance.
 *
 ***********************************************************/

const char *vceb_get_videocore_command_line( VCEB_INSTANCE_T instance )
{
    return instance->createParams.videocore_param;
}

/***********************************************************
 * Name: vceb_get_host_interface_driver_name
 *
 * Arguments:
 *       VCEB_INSTANCE_T    instance    - vceb instance
 *
 * Returns: Returns the name of the host interface drriver
 *          associated with the indicated vceb instance.
 *
 ***********************************************************/

int32_t vceb_get_host_interface_instance( VCEB_INSTANCE_T instance, 
                                          VCEB_HOST_INTERFACE_INSTANCE_T *host_interface_instance )
{
    *host_interface_instance = instance->host_interface_instance;
    return 0;
}

/***********************************************************
 * Name: vceb_initialise
 *
 * Arguments:
 *       const uint32_t noreset - flag to say 'dont reset the board'
 *
 * Description:   Routine to initialise the vceb layer
 *                Must be called before any other functions will work.
 *                This layer loads the 2nd + 3rd stage loaders and sends a 'hello' message to VideoCore
 *
 * Returns: int ( < 0 is fail)
 *
 ***********************************************************/
int32_t vceb_initialise(VCEB_INSTANCE_T instance,
                        const uint32_t noreset )
{
   int32_t success = -1; //fail by default

   #if VCEB_DEBUG
      vceb_printf( VCEB_PRINT_LEVEL_TRACE, "vceb_initialise( 0x%08lX, %i ) 1\n", (long)instance, noreset );
   #endif
   
   if( NULL != instance )
   {
      //Init the interface layer - glue logic to wrap the transmission of messages
      // so the VC code and the host call the same message primitives
      success = vceb_interface_initialise();
      if (success != 0)
      {
         vceb_printf( VCEB_PRINT_LEVEL_ERROR, "vceb_interface_initialise failed\n" );
         return -1;
      }

      if (( 0 == noreset ) && ( instance->host_interface_instance->funcs->download == NULL ))
      {
         //init the 2nd stage loader code
         vceb_2nd_stage_code_init( instance->name );

         #if VCEB_DEBUG
            vceb_printf( VCEB_PRINT_LEVEL_TRACE, "vceb_initialise 3\n" );
         #endif

         //load the 2nd stage loader
         success = vceb_load_next_stage_loader( instance->host_interface_instance,
                                                vceb_2nd_stage_code_get_ptr( instance->name ),
                                                vceb_2nd_stage_code_get_size( instance->name ),
                                                0 /*dont' attempt to exit suspend*/ );
         if (success != 0)
         {
            vceb_printf( VCEB_PRINT_LEVEL_ERROR, "vceb_load_next_stage_loader failed to load 2nd stage loader\n" );
            return -1;
         }

         #if VCEB_DEBUG
            vceb_printf( VCEB_PRINT_LEVEL_TRACE, "vceb_initialise 4\n" );
         #endif

         if ( vceb_app_code_get_size( instance->name ) > 0 )
         {
             //init the 3rd stage / vceb app code
             vceb_app_code_init( instance->name );

             //load in the VCEB stage - this means the interface is ready to go
             success = vceb_load_next_stage_loader( instance->host_interface_instance,
                                                    vceb_app_code_get_ptr( instance->name ),
                                                    vceb_app_code_get_size( instance->name ),
                                                    0 /*dont' attempt to exit suspend*/ );
             if (success != 0)
             {
                vceb_printf( VCEB_PRINT_LEVEL_ERROR, "vceb_load_next_stage_loader failed to load VCEB\n" );
                return -2;
             }

#if 0
             // We don't support vceb.bin right now, so we use it for loading 
             // test code which doesn't respond with the "ready for next stage message"

             //wait for VCEB on VideoCore to initialise
             success = vceb_wait_ready_for_next_stage( instance->host_interface_instance );
             if ( success == -3 )
             {
                 // When we load the 2nd stage bootloader it reponds initially with
                 // an "Image Verified" message and then follows that with a "Ready for Next Stage"
                 success = vceb_wait_ready_for_next_stage( instance->host_interface_instance );
             }
             if (success != 0)
             {
                vceb_printf( VCEB_PRINT_LEVEL_ERROR, "vceb_wait_ready_for_next_stage failed to get VCEB initialised message\n" );
                return -3;
             }
             instance->vceb_app_loaded = 1;
#endif
         }
      }

      //enable dma
      vceb_enable_dma = 1;

      if( success == 0 )
      {
         //mark as initialised
         instance->vceb_initialised = 1;
      }
   }

   return success;
}

/***********************************************************
 * Name: vceb_exit_and_load_final_image
 *
 * Arguments:
 *       const void * const image_data,
         const uint32_t image_size
 *
 * Description:  Routine to exit vceb and load the final VMCS-X image
 *
 * Returns: int ( < 0 is fail)
 *
 ***********************************************************/
int32_t vceb_exit_and_load_final_image(VCEB_INSTANCE_T instance,
                                       const void * const image_data,
                                       const uint32_t image_size )
{
   int32_t success = -1; //fail by default

   #if VCEB_DEBUG
   vceb_printf( VCEB_PRINT_LEVEL_ERROR, "%s: instance = 0x%p\n",
                __func__, instance );
   #endif

   if( NULL != instance )
   {
      #if VCEB_DEBUG
      vceb_printf( VCEB_PRINT_LEVEL_ERROR, "%s: vceb_initialised = %d\n",
                   __func__, instance->vceb_initialised );
      #endif
      if( instance->vceb_initialised )
      {
         if ( instance->host_interface_instance->funcs->download != NULL )
         {
             success = instance->host_interface_instance->funcs->download( instance->host_interface_instance, 
                                                                           image_data,
                                                                           image_size );
             
         }
         else
         {
             if ( instance->vceb_app_loaded )
             {
                 uint32_t count = 0;
                 const char dummy_text[] = "0123456789ABCDEF";
        
                 #if VCEB_DEBUG
                 vceb_printf( VCEB_PRINT_LEVEL_ERROR, "Putting VCEB into download mode\n" );
                 #endif
    
                 //disable DMA
                 //TODO - fix this!
                 vceb_enable_dma = 0;
        
                 //send a message to say 'go into download mode;
                 success = vceb_message_send(  instance->host_interface_instance,
                                               VCEB_MSG_GOTO_DOWNLOAD,
                                               VCEB_FLAGS_NONE,
                                               0,
                                               0,
                                               0, //no additional data
                                               NULL,
                                               1 /*terminate*/ );
        
                 //get the reply, indicating that we can now empty the MPHI queues
                 success = vceb_wait_ready_for_next_stage( instance->host_interface_instance );
                 if (success != 0)
                    return success;
    
                 //NOTE! There is a nasty bit of logic here. The host assumes that VC has 16 dma entries in the control + bulk channels
                 // and it sends down 16 terminating writes. The logic below just plucks off entries until they are all gone (as interrupts are disable, no more will get added)
                 // However, if an interrupt was pending, these 16 descriptors could get out of step leaving data from the host in the fifo.
    
                 //force terminate all the descriptors now
                 for( count = 0; count < 16; count++ )
                 {
                    vceb_interface_transmit(   instance->host_interface_instance,
                                               0, //control channel
                                               strlen( dummy_text ),
                                               dummy_text,
                                               1 ); //terminate
    
                    vceb_interface_transmit(   instance->host_interface_instance,
                                               1, //bulk
                                               strlen( dummy_text ),
                                               dummy_text,
                                               1 ); //terminate
                 }
                 instance->vceb_app_loaded = 0;
             }

             //load the final vmcs image
             success = vceb_load_next_stage_loader( instance->host_interface_instance, 
                                                    image_data,
                                                    image_size,
                                                    0 /*dont' attemp to exit suspend*/ );
             if ( !instance->vceb_app_loaded )
             {
                 // The 2ndstage loader sends a "Image Verified" message after we download.
                 // Wait for that.

                 #if VCEB_DEBUG
                 vceb_printf( VCEB_PRINT_LEVEL_ERROR, "final image loaded - waiting for response\n" );
                 #endif
                 success = vceb_wait_ready_for_next_stage( instance->host_interface_instance );
                 if (success != -3)
                 {
                    // We didn't get the image verified message from the 2ndstage loader

                    vceb_printf( VCEB_PRINT_LEVEL_ERROR, "%s: failed to get image verified message\n", __func__ );
                    return -1;
                 }
                 success = 0;
             }
         }

         //mark vceb as no longer initialised
         instance->vceb_initialised = 0;
         #if VCEB_DEBUG
         vceb_printf( VCEB_PRINT_LEVEL_ERROR, "Releasing vceb_initialised\n" );
         #endif

         //finally, shutdown the bus driver
         success += vceb_host_interface_shutdown( instance->host_interface_instance );
      }
   }

   return success;
}


/***********************************************************
 * Name: vceb_resume_videocore
 *
 * Arguments:
 *       void
 *
 * Description:  Routine to resume VC from suspend
 *
 * Returns: int ( < 0 is fail)
 *
 ***********************************************************/
int32_t vceb_resume_videocore( VCEB_INSTANCE_T instance )
{
   int32_t success = -1; //fail by default

   if( NULL != instance )
   {
      //if VCEB is running, we are fairly sure that VMCS-X will not
      //have just suspended
      if( !instance->vceb_initialised )
      {
         vceb_enable_dma = 0;

         //Init the interface layer - glue logic to wrap the transmission of messages
         // so the VC code and the host call the same message primitives
         success = vceb_interface_initialise();
         if (success != 0)
         {
            vceb_printf( VCEB_PRINT_LEVEL_ERROR, "vceb_interface_initialise failed\n" );
            return -1;
         }

         //Initialise the host hardware
         //this means that the message layer can be used as well
         success = vceb_host_interface_initialise( instance->host_interface_instance );
         if (success != 0)
         {
            vceb_printf( VCEB_PRINT_LEVEL_ERROR, "vceb_host_interface_initialise failed\n" );
            return -1;
         }

         //bring VC out of reset
         vceb_printf( VCEB_PRINT_LEVEL_ERROR, "VCEB: Powering up videocore %s\n", 
                      instance->name ? instance->name : "" );
         success += vceb_host_interface_control_run_pin( instance->host_interface_instance, 0 );
         vceb_delay( 25000 );
         success += vceb_host_interface_control_run_pin( instance->host_interface_instance, 1 );
         if (success != 0)
         {
            vceb_printf( VCEB_PRINT_LEVEL_ERROR, "vceb_host_interface_control_run_pin failed\n" );
            return -1;
         }

         #if VCEB_DEBUG
            vceb_printf( VCEB_PRINT_LEVEL_TRACE, "vceb_resume_videocore 2\n" );
         #endif

         //init the 2nd stage loader code
         vceb_2nd_stage_code_init( instance->name );
         
         #if VCEB_DEBUG
            vceb_printf( VCEB_PRINT_LEVEL_TRACE, "vceb_resume_videocore 3\n" );
         #endif

         //load the 2nd stage loader
         //when sending the 2nd stage loader, flag that we want the 2nd stage to attempt to exit VC from suspend mode      
         success = vceb_load_next_stage_loader( instance->host_interface_instance, 
                                                vceb_2nd_stage_code_get_ptr( instance->name ),
                                                vceb_2nd_stage_code_get_size( instance->name ),
                                                1 /*attemp to exit suspend*/ );
         if (success != 0)
         {
            vceb_printf( VCEB_PRINT_LEVEL_ERROR, "vceb_load_next_stage_loader failed to load 2nd stage loader\n" );
            return -1;
         }

         #if VCEB_DEBUG
            vceb_printf( VCEB_PRINT_LEVEL_TRACE, "vceb_resume_videocore 4\n" );
         #endif

         //wait for the suspend ack      
         success = vceb_wait_ready_for_suspend_ack( instance->host_interface_instance );
         if (success == 0)
         {
            //mark vceb as no longer initialised
            instance->vceb_initialised = 0;
            vceb_host_interface_shutdown( instance->host_interface_instance );
         }      
         //at this point, videocore should be running VMCS-X...
      }
   }

   return success;
}

/***********************************************************
 * Name: vceb_console_enable
 *
 * Arguments:
 *       const uint32_t enable
 *
 * Description: Routine to display the console on the screen
 *
 * Returns: int ( < 0 is fail)
 *
 ***********************************************************/
int32_t vceb_console_enable(  VCEB_INSTANCE_T instance,
                              const uint32_t enable )
{
   int32_t success = -1; //fail by default

   if( NULL != instance )
   {
      if( instance->vceb_initialised )
      {
         //package up a message and send it to the host
         success = vceb_message_send(  instance->host_interface_instance,
                                       VCEB_MSG_CONSOLE_ENABLE,
                                       VCEB_FLAGS_NONE,
                                       enable, //param 0 is the enable param,
                                       0, //no param 1,
                                       0, //no additional data
                                       NULL,
                                       1 /* don't terminate */ );

         #ifdef VCEB_EXPECT_RESPONSE
            if( success == 0 )
            {
               uint32_t expected_param_0 = 0;

               //wait and check that the reply is valid
               success = vceb_vceb_message_check_receive(instance->host_interface_instance,
                                                         VCEB_MSG_CONSOLE_ENABLE,
                                                         VCEB_FLAGS_REPLY,
                                                         &expected_param_0,
                                                         NULL,
                                                         NULL,
                                                         NULL );
            }
         #endif
      }
   }

   return success;
}


/***********************************************************
 * Name: vceb_console_put_char
 *
 * Arguments:
 *       const char c
 *
 * Description: Routine to put a character on the console
 *
 * Returns: int ( < 0 is fail)
 *
 ***********************************************************/
int32_t vceb_console_putstring(  VCEB_INSTANCE_T instance,
                                 const char *string )
{
   int32_t success = -1; //fail by default
   uint32_t param_0 = 0;
   uint32_t param_1 = 0;

   if( NULL != instance )
   {
      if( instance->vceb_initialised )
      {
         param_0 =   (string[0] << 24)
                  |  (string[1] << 16)
                  |  (string[2] << 8)
                  |  (string[3] << 0);

         param_1 =   (string[4] << 24)
                  |  (string[5] << 16)
                  |  (string[6] << 8)
                  |  (string[7] << 0);

         //package up a message and send it to the host
         success = vceb_message_send(  instance->host_interface_instance,
                                       VCEB_MSG_CONSOLE_PUTCHAR,
                                       VCEB_FLAGS_NONE,
                                       param_0,
                                       param_1,
                                       0, //no additional data
                                       NULL,
                                       0 /* don't terminate */ );

         #ifdef VCEB_EXPECT_RESPONSE
            if( success == 0 )
            {
               uint32_t expected_param_0 = 0;

               //wait and check that the reply is valid
               success = vceb_vceb_message_check_receive(instance->host_interface_instance,
                                                         VCEB_MSG_CONSOLE_PUTCHAR,
                                                         VCEB_FLAGS_REPLY,
                                                         &expected_param_0,
                                                         NULL,
                                                         NULL,
                                                         NULL );
            }
         #endif
      }
   }

   return success;
}


/***********************************************************
 * Name: vceb_console_clear
 *
 * Arguments:
 *       void
 *
 * Description: Routine to clear the console
 *
 * Returns: int ( < 0 is fail)
 *
 ***********************************************************/
int32_t vceb_console_clear( VCEB_INSTANCE_T instance )
{
   int32_t success = -1; //fail by default

   if( NULL != instance )
   {
      if( instance->vceb_initialised )
      {
         //package up a message and send it to the host
         success = vceb_message_send(  instance->host_interface_instance,
                                       VCEB_MSG_CONSOLE_CLEAR,
                                       VCEB_FLAGS_NONE,
                                       0, //no param 0
                                       0, //no param 1
                                       0, //no additional data
                                       NULL,
                                       0 /* don't terminate */ );

         #ifdef VCEB_EXPECT_RESPONSE
            if( success == 0 )
            {
               uint32_t expected_param_0 = 0;

               //wait and check that the reply is valid
               success = vceb_vceb_message_check_receive(instance->host_interface_instance,
                                                         VCEB_MSG_CONSOLE_CLEAR,
                                                         VCEB_FLAGS_REPLY,
                                                         &expected_param_0,
                                                         NULL,
                                                         NULL,
                                                         NULL );
            }
         #endif
      }
   }

   return success;
}


/***********************************************************
 * Name: vceb_framebuffer_overlay_enable
 *
 * Arguments:
 *       const uint32_t enable
 *
 * Description: Routine to enable the framebuffer
 *
 * Returns: int ( < 0 is fail)
 *
 ***********************************************************/
int32_t vceb_framebuffer_overlay_enable(  VCEB_INSTANCE_T instance,
                                          const uint32_t enable )
{
   int32_t success = -1; //fail by default

   if( NULL != instance )
   {
      if( instance->vceb_initialised )
      {
         //package up a message and send it to the host
         success = vceb_message_send(  instance->host_interface_instance,
                                       VCEB_MSG_FB_OVERLAY_ENABLE,
                                       VCEB_FLAGS_NONE,
                                       enable, //param 0 is enable
                                       0, //no param 1
                                       0, //no additional data
                                       NULL,
                                       1 /* terminate */ );

         #ifdef VCEB_EXPECT_RESPONSE
            if( success == 0 )
            {
               uint32_t expected_param_0 = 0;

               //wait and check that the reply is valid
               success = vceb_vceb_message_check_receive(instance->host_interface_instance,
                                                         VCEB_MSG_FB_OVERLAY_ENABLE,
                                                         VCEB_FLAGS_REPLY,
                                                         &expected_param_0,
                                                         NULL,
                                                         NULL,
                                                         NULL );
            }
         #endif
      }
   }

   return success;
}

/***********************************************************
 * Name: vceb_framebuffer_overlay_set
 *
 * Arguments:
 *       const void * const data,
         const VCEB_RBG_FORMAT_T rbg_format,
         const uint32_t transparent_colour,
         const uint32_t width,
         const uint32_t height,
         const VCEB_ALIGN_T alignment
 *
 * Description: Routine to set the contents of the framebuffer overlay
 *
 * Returns: int ( < 0 is fail)
 *
 ***********************************************************/
int32_t vceb_framebuffer_overlay_set(  VCEB_INSTANCE_T instance,
                                       const void * const data,
                                       const VCEB_RBG_FORMAT_T rbg_format,
                                       const uint32_t transparent_colour,
                                       const uint16_t width,
                                       const uint16_t height,
                                       const VCEB_ALIGN_T alignment )
{
   int32_t success = -1; //fail by default
   uint32_t size_of_framebuf_in_bytes  = 0;

   if( NULL != instance )
   {
      if( instance->vceb_initialised )
      {
         //default to success
         success = 0;
      
         switch( rbg_format )
         {
            case VCEB_RBG_FORMAT_RGB565:
               size_of_framebuf_in_bytes = width * height * 2;
            break;

            case VCEB_RBG_FORMAT_RGB888:
               size_of_framebuf_in_bytes = width * height * 3;
            break;

            case VCEB_RBG_FORMAT_RGBA8888:
            case VCEB_RBG_FORMAT_BGRA8888:
               size_of_framebuf_in_bytes = width * height * 4;
            break;

            default:
               //bad input format
               success = -2;
            break;
         }
         
         if( success == 0 )
         {  
            //inc the number of frames sent
            instance->vceb_frame_count++;

            //package up a message and send it to the host
            success = vceb_message_send(  instance->host_interface_instance,
                                          VCEB_MSG_FB_OVERLAY_SET,
                                          VCEB_FLAGS_USES_BULK_CHANNEL, //use the bulk channel to send the framebuffer data down
                                          (width << 16) | (0xFFFF & height), //param 0
                                          (rbg_format << 28) | (alignment << 24) | (0xFFFFFF & transparent_colour), //param 1 is packed with non documented bit fields...
                                          size_of_framebuf_in_bytes, //size of the framebuffer data
                                          data, //framebuffer data
                                          1 /* terminate */ );

            #ifdef VCEB_EXPECT_RESPONSE
               if( success == 0 )
               {
                  uint32_t expected_param_0 = 0;

                  //wait and check that the reply is valid
                  success = vceb_vceb_message_check_receive(instance->host_interface_instance,
                                                            VCEB_MSG_FB_OVERLAY_SET,
                                                            VCEB_FLAGS_REPLY,
                                                            &expected_param_0,
                                                            NULL,
                                                            NULL,
                                                            NULL );
               }
            #endif
         }
      }
   }

   return success;
}

/***********************************************************
 * Name: vceb_framebuffer_overlay_clear
 *
 * Arguments:
 *       void
 *
 * Description: Routine to clear the framebuffer
 *
 * Returns: int ( < 0 is fail)
 *
 ***********************************************************/
int32_t vceb_framebuffer_overlay_clear( VCEB_INSTANCE_T instance )
{
   int32_t success = -1; //fail by default

   if( NULL != instance )
   {
      if( instance->vceb_initialised )
      {
         //package up a message and send it to the host
         success = vceb_message_send(  instance->host_interface_instance,
                                       VCEB_MSG_FB_OVERLAY_CLEAR,
                                       VCEB_FLAGS_NONE,
                                       0, //no param 0
                                       0, //no param 1
                                       0, //no additional data
                                       NULL,
                                       1 /* terminate */ );

         #ifdef VCEB_EXPECT_RESPONSE
            if( success == 0 )
            {
               uint32_t expected_param_0 = 0;

               //wait and check that the reply is valid
               success = vceb_vceb_message_check_receive(instance->host_interface_instance,
                                                         VCEB_MSG_FB_OVERLAY_CLEAR,
                                                         VCEB_FLAGS_REPLY,
                                                         &expected_param_0,
                                                         NULL,
                                                         NULL,
                                                         NULL );
            }
         #endif
      }
   }

   return success;
}


/***********************************************************
 * Name: vceb_set_backlight_brightness
 *
 * Arguments:
 *       void
 *
 * Description: Routine to set the brightness
 *
 * Returns: int ( < 0 is fail)
 *
 ***********************************************************/
int32_t vceb_set_backlight_brightness( VCEB_INSTANCE_T instance,
                                       const uint8_t brightness )
{
   int32_t success = -1; //fail by default

   if( NULL != instance )
   {
      if( instance->vceb_initialised )
      {
         //package up a message and send it to the host
         success = vceb_message_send(  instance->host_interface_instance,
                                       VCEB_MSG_SET_LCD_BRIGHTNESS,
                                       VCEB_FLAGS_NONE,
                                       brightness, //no param 0
                                       0, //no param 1
                                       0, //no additional data
                                       NULL,
                                       1 /* terminate */ );

         #ifdef VCEB_EXPECT_RESPONSE
            if( success == 0 )
            {
               uint32_t expected_param_0 = 0;

               //wait and check that the reply is valid
               success = vceb_vceb_message_check_receive(instance->host_interface_instance,
                                                         VCEB_MSG_SET_LCD_BRIGHTNESS,
                                                         VCEB_FLAGS_REPLY,
                                                         &expected_param_0,
                                                         NULL,
                                                         NULL,
                                                         NULL );
            }
         #endif
      }
   }

   return success;
}

/***********************************************************
 * Name: vceb_get_stat
 *
 * Arguments:
 *       void
 *
 * Description: Routine to clear the framebuffer
 *
 * Returns: int ( < 0 is fail)
 *
 ***********************************************************/
int32_t vceb_get_stat(  VCEB_INSTANCE_T instance,
                        const char * const stat_name,
                        char *buffer,
                        const uint32_t buffer_len )
{
   int32_t success = -1;
   uint32_t dump = 0;
   uint32_t buffer_space_left = buffer_len;

   if( NULL != instance )
   {
      if( instance->vceb_initialised )
      {      
         if( ( NULL != stat_name ) && (buffer != NULL) )
         {
            uint32_t count = 0;
            VCEB_STAT_T stats[] =
            {
               { "frame_count", &instance->vceb_frame_count },
            };

            //do we want to dump all the stats out?
            dump = !strcmp( stat_name, "dump" );

            //loop over all the stats
            for( count = 0; count < (sizeof( stats ) / sizeof( VCEB_STAT_T ) ) && buffer_space_left; count++ )
            {
               if( dump || (0 == strcmp( stat_name, stats[ count ].name ) ) )
               {
                  //print the stat and its value
                  int bytes_written = snprintf( buffer, buffer_space_left, "%s = %i\n", stats[ count ].name, *stats[ count ].value );

                  if( bytes_written >= 0 )
                  {
                     buffer += bytes_written;
                     buffer_space_left -= bytes_written;

                     //any data written is good by me
                     success = 0;
                  }
               }
            }

            //proccess any interface specific stats
            if( 0 == vceb_host_interface_get_stat( instance->host_interface_instance, stat_name, buffer, buffer_space_left ) )
            {
               success = 0;
            }
         }
      }
   }

   return success;
}


/******************************************************************************
Static funcs
******************************************************************************/

/***********************************************************
 * Name: vceb_wait_ready_for_next_stage
 *
 * Arguments:
 *       void
 *
 * Description: Waits for a "ready for next stage" message
 *              from VideoCore, indicating that the current
 *              stage has completed its initialisation and
 *              is ready to accept commands / the next stage
 *
 * Returns: int ( < 0 is fail)
 *
 ***********************************************************/
static int32_t vceb_wait_ready_for_next_stage( VCEB_HOST_INTERFACE_INSTANCE_T host_interface_instance )
{
   VCEB_BOOT_MESSAGE_T Message;
   void              * Opaque = (void *) &Message;

   int Bytes = vceb_interface_receive( host_interface_instance, 0 /* Bulk? */, sizeof( Message ), &Opaque, 1 /* Copy data? */ );
   if (Bytes < 0)
   {
      return -1; // Communication issue
   }

   if (Message.header != HOSTPORT_COMMS_HEADER || Message.footer != HOSTPORT_COMMS_FOOTER)
   {
      return -2; // Malformed message
   }

   //here we are just checking for a failure - we don't expect this message normally
   if ((Message.id_info >> 24) == HOSTPORT_MESSAGE_ID_NEXT_IMAGE_VERIFIED)
   {
          int32_t status = (Message.id_info & 0xffffff) == 1 ? -3  // Next image loaded from another source
                                                             : -4; // Failed verification
          return status;
   }

   if ((Message.id_info >> 24) != HOSTPORT_MESSAGE_ID_READY_FOR_NEXT_IMAGE)
   {
      return -5; // Invalid message
   }
   return 0;
}


/***********************************************************
 * Name: vceb_wait_ready_for_suspend_ack
 *
 * Arguments:
 *       void
 *
 * Description: Waits for a "suspend ack" message
 *              from VideoCore, indicating that the 2nd
 *              stage has found a valid suspend image or not
 *
 * Returns: int ( < 0 is fail)
 *
 ***********************************************************/
static int32_t vceb_wait_ready_for_suspend_ack( VCEB_HOST_INTERFACE_INSTANCE_T host_interface_instance )
{
   VCEB_BOOT_MESSAGE_T Message;
   void              * Opaque = (void *) &Message;
   int Bytes = vceb_interface_receive( host_interface_instance, 0 /* Bulk? */, sizeof( Message ), &Opaque, 1 /* Copy data? */ );
   if (Bytes < 0)
      return -1; // Communication issue

   if (Message.header != HOSTPORT_COMMS_HEADER || Message.footer != HOSTPORT_COMMS_FOOTER)
      return -2; // Malformed message

   #if VCEB_DEBUG
      vceb_printf( VCEB_PRINT_LEVEL_TRACE, "vceb_wait_ready_for_suspend_ack msg = %i, %i\n", Message.id_info >> 24, Message.id_info & 0xFFFFFF );
   #endif

   if ((Message.id_info >> 24) != HOSTPORT_MESSAGE_ID_NEXT_IMAGE_VERIFIED)
      return -3; // Invalid message
      
   return (Message.id_info ? 0 : -5 );
}


/***********************************************************
 * Name: vceb_load_next_stage_loader
 *
 * Arguments:
 *       void
 *
 * Description: Routine to load an image into videocore
 *
 * Returns: int ( < 0 is fail)
 *
 ***********************************************************/
static int32_t vceb_load_next_stage_loader(  VCEB_HOST_INTERFACE_INSTANCE_T host_interface_instance,
                                             const void * const data,
                                             const uint32_t data_size,
                                             const uint32_t attemp_to_exit_suspend )
{
   if (data == NULL || data_size == 0)
      return -1;

   // Wait for VideoCore to be ready for the next stage
   int32_t status = vceb_wait_ready_for_next_stage(host_interface_instance);
   if ( status == -3 )
   {
       // When we load the 2nd stage bootloader it reponds initially with
       // an "Image Verified" message and then follows that with a "Ready for Next Stage"
       status = vceb_wait_ready_for_next_stage(host_interface_instance);
   }
   if (status != 0)
   {
#if VCEB_DEBUG
      vceb_printf(VCEB_PRINT_LEVEL_ERROR, "error: boot failed (%d), invalid message from 2727\n", (int) status);
#endif
      return -2;
   }
   if (data == NULL || data_size == 0)
      return -1;

   // Write the next stage on the bulk channel
   uint32_t roundedup_data_size = (data_size + 15) & ~15; // VideoCore's MPHI only supports 16 byte aligned transfers

   // Termination is not required, as the next message's control write will terminate this message
   vceb_interface_transmit( host_interface_instance, 1 /* Bulk? */, roundedup_data_size, data, 0 /* Terminate? */ );

   // Inform VideoCore that the next stage has been written to the bulk channel
   VCEB_BOOT_MESSAGE_T Message;
   Message.header = HOSTPORT_COMMS_HEADER;
   Message.id_info = (HOSTPORT_MESSAGE_ID_HOST_HAS_SENT_NEXT_IMAGE << 24) | (data_size & 0xffffff);
   //TODO - we currently don't support secure boot so we are abusing the signature storage for the suspend flag
   Message.signature[0] = Message.signature[1] = Message.signature[2] = Message.signature[3] = Message.signature[4] = (attemp_to_exit_suspend ? 0xa5a5a5a5 : 0);
   Message.footer = HOSTPORT_COMMS_FOOTER;
   vceb_interface_transmit( host_interface_instance, 0 /* Bulk? */, sizeof(Message), &Message, 1 /* Terminate? */ );

   return 0;
}

/***********************************************************
 * Name: vceb_control_run_pin
 *
 * Arguments:
 *       VCEB_INSTANCE_T    instance         - vceb instance
 *       run_pin_high       0 to set the run pin low, 1 to set it high
 *
 * Description: Controls the run pin for a given vceb instance.
 *
 * Returns: int ( < 0 is fail)
 *
 ***********************************************************/
int32_t vceb_control_run_pin(VCEB_INSTANCE_T instance, const uint32_t run_pin_high )
{
    return vceb_host_interface_control_run_pin( instance->host_interface_instance, run_pin_high );
}

/* ************************************ The End ***************************************** */
