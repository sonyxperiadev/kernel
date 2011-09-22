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

#ifndef VCEB_LINUX_WRAPPER_H_
#define VCEB_LINUX_WRAPPER_H_

#include "vceb.h"
#include "vceb_host_interface.h"

typedef struct opaque_vceb_linux_instanct_t *VCEB_LINUX_INSTANCE_T;

int32_t vceb_linux_create_instance( const VCEB_HOST_INTERFACE_CREATE_PARAMS_T *createParams,
                                    VCEB_HOST_INTERFACE_INSTANCE_T host_interface_instance,
                                    VCEB_INSTANCE_T *vceb_inst_out,
                                    VCEB_LINUX_INSTANCE_T *linux_inst_out );

void vceb_linux_delete_instance( VCEB_LINUX_INSTANCE_T linux_inst );

#endif /* VCEB_LINUX_WRAPPER_H_ */

