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

/* ---- Include Files ---------------------------------------------------- */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/vchiq_platform_data_memdrv_hana.h>

#include "vchiq_core.h"
#include "vchiq_memdrv.h"

/****************************************************************************
*
* vchiq_memdrv_hana_interface_probe
*
*   This function will be called for each "vchiq_memdrv_hana" device which is
*   registered in the board definition.
*
***************************************************************************/

VCHIQ_STATUS_T vchiq_userdrv_create_instance( const VCHIQ_PLATFORM_DATA_T *platform_data );

static int __devinit vchiq_memdrv_hana_interface_probe( struct platform_device *pdev )
{
    VCHIQ_PLATFORM_DATA_MEMDRV_HANA_T *platform_data = pdev->dev.platform_data;
    const char *name = platform_data->memdrv.common.instance_name;

    printk( KERN_INFO "vchiq_memdrv_hana: Probing '%s' ...\n", name );
    printk( KERN_INFO "vchiq_memdrv_hana: Shared Memory: 0x%08x\n",
            (uint32_t)platform_data->memdrv.sharedMemVirt );

    platform_set_drvdata( pdev, NULL );

    /*
     * This needs to be moved up into vchiq_memdrv.c
    */

    if ( vchiq_userdrv_create_instance( &platform_data->memdrv.common) != VCHIQ_SUCCESS )
    {
        printk( KERN_ERR "vchiq_memdrv_hana: Failed to create vchiq instance for '%s'\n",
                name );

        return -ENOMEM;
    }

    return 0;
}

/****************************************************************************
*
* vchiq_memdrv_hana_interface_remove
*
*   Register a "driver". We do this so that the probe routine will be called
*   when a corresponding architecture device is registered.
*
***************************************************************************/

static int vchiq_memdrv_hana_interface_remove( struct platform_device *pdev )
{
    platform_set_drvdata( pdev, NULL );

    return 0;
}

/****************************************************************************
*
* vchiq_memdrv_hana_interface_driver
*
*   Register a "driver". We do this so that the probe routine will be called
*   when a corresponding architecture device is registered.
*
***************************************************************************/

static struct platform_driver vchiq_memdrv_hana_interface_driver = 
{
    .probe          = vchiq_memdrv_hana_interface_probe,
    .remove         = vchiq_memdrv_hana_interface_remove,
    .driver = {
        .name	    = "vchiq_memdrv_hana",
    }
};

/****************************************************************************
*
* vchiq_memdrv_hana_interface_init
*
*   Creates the instance that is used to access the videocore(s). One
*   instance is created per videocore.
*
***************************************************************************/

static int __init vchiq_memdrv_hana_interface_init( void )
{
    return platform_driver_register( &vchiq_memdrv_hana_interface_driver );
}

/****************************************************************************
*
* vchiq_memdrv_hana_interface_exit
* 
*   Called when the module is unloaded.
*
***************************************************************************/

static void __exit vchiq_memdrv_hana_interface_exit( void )
{
    platform_driver_unregister( &vchiq_memdrv_hana_interface_driver );
}

module_init( vchiq_memdrv_hana_interface_init );
module_exit( vchiq_memdrv_hana_interface_exit );

MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("VCHIQ Shared Memory Interface Driver");
MODULE_LICENSE("GPL");

