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


#ifndef VCEB_H_
#define VCEB_H_

#include "vceb_types.h"
#include "vceb_common.h"
#include "common/vceb_host_interface.h"

/******************************************************************************
 VCEB Instance opaque type
 *****************************************************************************/

//Generic handle used to describe a bus interface
typedef struct opaque_vceb_instance_t *VCEB_INSTANCE_T;

//The largest instance name that we support
#define VCEB_MAX_INSTANCE_NAME   (32)

/******************************************************************************
 Initialise and exit functions
 *****************************************************************************/

// Routine to retrieve a previously created instance.
extern int32_t vceb_get_instance( const char *instance_name, VCEB_INSTANCE_T *instance );

extern const char *vceb_get_instance_name( VCEB_INSTANCE_T instance );

//Routine to create an instance of a bus interface
//Only needs to occur once during the lifecycle of a system
//The param can be used to pass in specific information about a bus interface
//  such as physical address, videocore 'instance number' or internal/external
extern int32_t vceb_create_instance( const VCEB_HOST_INTERFACE_CREATE_PARAMS_T *createParams,
                                     VCEB_HOST_INTERFACE_INSTANCE_T host_interface_instance,
                                     VCEB_INSTANCE_T *instance );

// Removes a previously created instance.
extern int32_t vceb_delete_instance( VCEB_INSTANCE_T instance );

//Routine to retrieve the first instance of all created instances.
VCEB_INSTANCE_T vceb_get_first_instance( void );

// Routine to retrieve the next instance in the list after the indicated instance.
VCEB_INSTANCE_T vceb_get_next_instance( VCEB_INSTANCE_T instance );

//Routine to initialise the vceb layer
//Must be called before any other functions will work.
//This layer loads the 2nd + 3rd stage loaders and sends a 'hello' message to VideoCore
extern int32_t vceb_initialise(  VCEB_INSTANCE_T instance,
                                 const uint32_t noreset );

//Routine to exit vceb and load the final VMCS-X image
//The image data must point to the unpacked (i.e. not an ELF file) copy of the videocore image
//This binary is built as part of the toolchain and can be included in most ARM environments using the string:
//    objcopy -v -O elf32-littlearm -I binary -B arm vmcs.bin vmcs.bin.o
extern int32_t vceb_exit_and_load_final_image(  VCEB_INSTANCE_T instance,
                                                const void * const image_data,
                                                const uint32_t image_size );

//Routine to attempt to resume VideoCore from a suspend to SDRAM state
//This function does not initialise vceb - it is just used to install the 2nd stage loader
// and bring vc to the point where it can resume from sdram.
extern int32_t vceb_resume_videocore( VCEB_INSTANCE_T instance );

// Routine which allows the run pin to be controlled for a particular VCEB instance.
extern int32_t vceb_control_run_pin(VCEB_INSTANCE_T instance, const uint32_t run_pin_high );

extern const char *vceb_get_videocore_command_line( VCEB_INSTANCE_T instance );

extern int32_t vceb_get_host_interface_instance( VCEB_INSTANCE_T instance, 
                                                 VCEB_HOST_INTERFACE_INSTANCE_T *host_interface_instance );

/******************************************************************************
 Console functions
 *****************************************************************************/

//Routine to display the console on the screen
//VCEB can be built using different displays for the console to be output on but this is hardcoded inside the VCEB binary
extern int32_t vceb_console_enable( VCEB_INSTANCE_T instance,
                                    const uint32_t enable );

//Routine to put a character on the console
//Even if the character is not visible, it will still be send
// and shown the next time the console is displayed
//The console is prebuilt with a defined buffer and history limits
extern int32_t vceb_console_putstring( VCEB_INSTANCE_T instance,
                                       const char *string );

//Routine to clear the consoles output
//Note - the console is still visible on the screen
extern int32_t vceb_console_clear( VCEB_INSTANCE_T instance );

/******************************************************************************
 Framebuffer overlay functionality
 *****************************************************************************/

//Routine to enable the framebuffer
//This lives on top of the console, which will be hidden unless this image contains alpha
extern int32_t vceb_framebuffer_overlay_enable( VCEB_INSTANCE_T instance,
                                                const uint32_t enable );

//Routine to set the contents of the framebuffer overlay
extern int32_t vceb_framebuffer_overlay_set( VCEB_INSTANCE_T instance,
                                             const void * const data,            //unaligned data
                                             const VCEB_RBG_FORMAT_T rbg_format, //rgb format
                                             const uint32_t transparent_colour,  //which colour (in terms of color space) is transparent
                                             const uint16_t width,
                                             const uint16_t height,
                                             const VCEB_ALIGN_T alignment );     //alignment of the image on the screen (centre, stretched etc..)

//Routine to clear the current contents of the framebuffer
//The framebuffer is cleared with alpha values
extern int32_t vceb_framebuffer_overlay_clear( VCEB_INSTANCE_T instance );

/******************************************************************************
 Backlight control
 *****************************************************************************/

//Routine to set brightness 
//range 0 to 255
extern int32_t vceb_set_backlight_brightness(VCEB_INSTANCE_T instance,
                                             const uint8_t brightness );

/******************************************************************************
 Diagnostics
 *****************************************************************************/

//Routine to run a diagnostic command
//Optional "per diagnostic" parameter list can be passed in if required
extern int32_t vceb_diag_run( VCEB_INSTANCE_T instance,
                              const char * const command_line,
                              char * const buffer_for_reply,
                              const uint32_t buffer_len );


/******************************************************************************
 Statistics
 *****************************************************************************/

//Routine to get a statistic from the driver
//Examples are framecount, bytes per sec etc...
extern int32_t vceb_get_stat( VCEB_INSTANCE_T instance,
                              const char * const stat_name,
                              char *buffer,
                              const uint32_t buffer_len );

#endif // VCEB_H_
