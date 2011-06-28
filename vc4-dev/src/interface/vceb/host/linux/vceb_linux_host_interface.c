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

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/string.h>

#include "vceb_linux_host_interface.h"

/***************************************************************************** 
* 
*    Allocates a host interface instance, and populates it.
*  
*****************************************************************************/

int32_t vceb_linux_host_interface_alloc( const VCEB_HOST_INTERFACE_CREATE_PARAMS_T *create_params,
                                         const VCEB_HOST_INTERFACE_FUNCS_T *funcs,
                                         VCEB_HOST_INTERFACE_INSTANCE_T *instance )
{
    VCEB_HOST_INTERFACE_INSTANCE_T  newInst;

    newInst = kzalloc( sizeof( *newInst ), GFP_KERNEL );
    if ( newInst == NULL )
    {
        printk( KERN_ERR "%s: Unable to allocate %d bytes for host interface instance\n",
                __func__, sizeof( *newInst ));
        return -1;
    }

    newInst->name               = create_params->instance_name;
    newInst->videocore_param    = create_params->videocore_param;
    newInst->host_param         = create_params->host_param;

    newInst->funcs              = funcs;

    *instance = newInst;

    return 0;
}

EXPORT_SYMBOL( vceb_linux_host_interface_alloc );

/***************************************************************************** 
* 
*    Corresponding free routine
*  
*****************************************************************************/

void vceb_linux_host_interface_free( VCEB_HOST_INTERFACE_INSTANCE_T instance )
{
    kfree( instance );
}

EXPORT_SYMBOL( vceb_linux_host_interface_free );
