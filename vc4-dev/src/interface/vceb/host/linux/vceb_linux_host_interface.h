/*****************************************************************************
* Copyright 2001 - 2010 Broadcom Corporation.  All rights reserved.
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

#ifndef VCEB_LINUX_HOST_INTERFACE_H_
#define VCEB_LINUX_HOST_INTERFACE_H_

#include "common/vceb_host_interface.h"

// Helper function which allocates and populates the host interface instance
int32_t vceb_linux_host_interface_alloc( const VCEB_HOST_INTERFACE_CREATE_PARAMS_T *createParams,
                                         const VCEB_HOST_INTERFACE_FUNCS_T *funcs,
                                         VCEB_HOST_INTERFACE_INSTANCE_T *instance );

// Matching free function.
void vceb_linux_host_interface_free( VCEB_HOST_INTERFACE_INSTANCE_T instance );

#endif /* VCEB_LINUX_HOST_INTERFACE_H_ */

