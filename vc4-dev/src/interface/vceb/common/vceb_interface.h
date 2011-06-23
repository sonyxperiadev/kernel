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


#ifndef VCEB_INTERFACE_H_
#define VCEB_INTERFACE_H_

#include "vceb_types.h"
#include "vceb_host_interface.h"

/******************************************************************************
Global functions.
******************************************************************************/

//Routine to init the interface layer
extern int32_t vceb_interface_initialise( void );

//Routine to send a message
extern int32_t vceb_interface_transmit(   VCEB_HOST_INTERFACE_INSTANCE_T host_interface_instance,
                                          const uint32_t bulk_channel,
                                          const uint32_t data_size,
                                          const void * const data,
                                          const uint32_t terminate_transmission );

//Routine to receive a message
extern int32_t vceb_interface_receive( VCEB_HOST_INTERFACE_INSTANCE_T host_interface_instance,
                                       const uint32_t bulk_channel,
                                       const uint32_t data_size,
                                       void **data,
                                       const uint32_t copy_data_not_return_ptr );


extern int32_t vceb_interface_get_stat(   VCEB_HOST_INTERFACE_INSTANCE_T host_interface_instance,
                                          const char * const stat_name,
                                          char *buffer,
                                          const uint32_t buffer_len );

#endif // VCEB_INTERFACE_H_
