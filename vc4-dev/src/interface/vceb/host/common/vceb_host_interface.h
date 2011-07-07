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


#ifndef VCEB_HOST_INTERFACE_H_
#define VCEB_HOST_INTERFACE_H_

#include "vceb_types.h"

/******************************************************************************
 Host interface opaque type
 *****************************************************************************/

//Generic handle used to describe a bus interface
typedef struct opaque_vceb_host_interface_instance_t *VCEB_HOST_INTERFACE_INSTANCE_T;
typedef struct opaque_vceb_host_interface_state_t    *VCEB_HOST_INTERFACE_STATE_T;

//Generic handle used to describe platform specific parameters
typedef struct opaque_vceb_host_param_t *VCEB_HOST_PARAM_T;

typedef struct vceb_host_interface_create_params_t
{
    /*
     * The name must be unique and is used to create /proc/vceb entries.
     */
    const char                             *instance_name;

    /*
     * The videocore_parm is a command line which is passed down to the videocore 
     * as part of the firmware download. 
     */
    const char                             *videocore_param;

    /*
     * The host_param are instance specific parameters.
     */
    void                                    *host_param;

} VCEB_HOST_INTERFACE_CREATE_PARAMS_T;

//Routine to initialise the hardware interface
//This actually sets up the i/o and holds any locks to the hardware it needs
typedef int32_t (*VCEB_HOST_INTERFACE_INITIALISE_T)( VCEB_HOST_INTERFACE_INSTANCE_T instance );

//Routine to shutdown the hardware interface and free all resources
typedef int32_t (*VCEB_HOST_INTERFACE_SHUTDOWN_T)( VCEB_HOST_INTERFACE_INSTANCE_T instance );

//Routine to set the state of the run pin - this brings VC in and out of reset
typedef int32_t (*VCEB_HOST_INTERFACE_CONTROL_RUN_PIN_T)( VCEB_HOST_INTERFACE_INSTANCE_T instance,
                                                          const uint32_t run_pin_high );

/******************************************************************************
 Read / Write Functions
 *****************************************************************************/

typedef int32_t (*VCEB_HOST_INTERFACE_DOWNLOAD_T)( VCEB_HOST_INTERFACE_INSTANCE_T instance,
                                                   const void * const data,
                                                   const uint32_t data_size );

typedef int (*VCEB_HOST_INTERFACE_DOWNLOAD_STATUS_T)( VCEB_HOST_INTERFACE_INSTANCE_T instance, 
                                                      char *buf,
                                                      int count );
                                                   
// The read and write routines are only needed if the download routine needs them.
typedef int32_t (*VCEB_HOST_INTERFACE_READ_T)(  VCEB_HOST_INTERFACE_INSTANCE_T instance,
                                                const uint32_t address,
                                                const uint32_t bytes_to_read,
                                                void *buffer );

typedef int32_t (*VCEB_HOST_INTERFACE_WRITE_T)( VCEB_HOST_INTERFACE_INSTANCE_T instance,
                                                const uint32_t address,
                                                const uint32_t bytes_to_write,
                                                const void * const buffer );

//Routine to query the stats from the specific interface
typedef int32_t (*VCEB_HOST_INTERFACE_GET_STAT_T)( VCEB_HOST_INTERFACE_INSTANCE_T instance,
                                                   const char * const stat_name,
                                                   char *buffer,
                                                   const uint32_t buffer_len );

typedef struct
{
    const char                             *driver_name;
    VCEB_HOST_INTERFACE_INITIALISE_T        initialise;
    VCEB_HOST_INTERFACE_SHUTDOWN_T          shutdown;
    VCEB_HOST_INTERFACE_CONTROL_RUN_PIN_T   controlRunPin;
    VCEB_HOST_INTERFACE_DOWNLOAD_T          download;
    VCEB_HOST_INTERFACE_DOWNLOAD_STATUS_T   downloadStatus;

    VCEB_HOST_INTERFACE_READ_T              readData;
    VCEB_HOST_INTERFACE_WRITE_T             writeData;
    VCEB_HOST_INTERFACE_GET_STAT_T          getStat;

} VCEB_HOST_INTERFACE_FUNCS_T;

struct opaque_vceb_host_interface_instance_t
{
    const VCEB_HOST_INTERFACE_FUNCS_T      *funcs;

    const char                             *name;
    const char                             *videocore_param;
    void                                   *host_param;

    // The following description is filled in by the create instance function
    // and is used for informational purposes (i.e. shows up in the /proc/vceb 
    // directory).
    char                                    descr[ 32 ];

    // The state data structure contains interface specific data
    VCEB_HOST_INTERFACE_STATE_T             state;
};

/******************************************************************************
 Initialise + Reset Control function
 *****************************************************************************/

//Routine to initialise the hardware interface
//This actually sets up the i/o and holds any locks to the hardware it needs
static inline int32_t vceb_host_interface_initialise( VCEB_HOST_INTERFACE_INSTANCE_T instance )
{
    return instance->funcs->initialise( instance );
}

//Routine to shutdown the hardware interface and free all resources
static inline int32_t vceb_host_interface_shutdown( VCEB_HOST_INTERFACE_INSTANCE_T instance )
{
    return instance->funcs->shutdown( instance );
}

//Routine to set the state of the run pin - this brings VC in and out of reset
static inline int32_t vceb_host_interface_control_run_pin( VCEB_HOST_INTERFACE_INSTANCE_T instance,
                                                           const uint32_t run_pin_high )
{
    return instance->funcs->controlRunPin( instance, run_pin_high );
}

/******************************************************************************
 Read / Write Functions
 *****************************************************************************/

//Routine to read data from the interface
//The address param controls the single address pin that goes to VC from the host
//Note! bytes_to_read will always physically read data aligned up to a 2 byte boundary
// although the data returned will be masked off correctly
static inline int32_t vceb_host_interface_read( VCEB_HOST_INTERFACE_INSTANCE_T instance,
                                        const uint32_t address,
                                        const uint32_t bytes_to_read,
                                        void *buffer )
{
    return instance->funcs->readData( instance, address, bytes_to_read, buffer );
}

//Routine to write data to the interface
//The address param controls the single address pin that goes to VC from the host
//Note! vceb_host_interface_write will always physically write data aligned up to a 2 byte boundary
static inline int32_t vceb_host_interface_write( VCEB_HOST_INTERFACE_INSTANCE_T instance,
                                                 const uint32_t address,
                                                 const uint32_t bytes_to_write,
                                                 const void * const buffer )
{
    return instance->funcs->writeData( instance, address, bytes_to_write, buffer );
}

//Routine to query the stats from the specific interface
static inline int32_t vceb_host_interface_get_stat( VCEB_HOST_INTERFACE_INSTANCE_T instance,
                                                    const char * const stat_name,
                                                    char *buffer,
                                                    const uint32_t buffer_len )
{
    return instance->funcs->getStat( instance, stat_name, buffer, buffer_len );
}


#endif // VCEB_HOST_INTERFACE_H_
