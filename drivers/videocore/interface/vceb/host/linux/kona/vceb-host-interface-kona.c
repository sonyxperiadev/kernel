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


/****************************************************************************
*
*  vceb-host-interface-mphi.c
*
*  PURPOSE:
*
*       This file implements the necessary access routines so that the vchiq
*       stack can talk to the videocore through the PIF interface.
*
*****************************************************************************/

/* ---- Include Files ---------------------------------------------------- */

#define USE_DMA     0

#include <linux/moduleparam.h>
#include <linux/platform_device.h>

#include <linux/gpio.h>
//#include <linux/broadcom/timer.h>
//#include <linux/broadcom/dbg-log.h>

#include <linux/dma-mapping.h>
#include <mach/sdma.h>

#include <linux/crc32.h>
#include <linux/time.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/clk.h>
#include <vceb_platform_data_kona.h>

#include "vceb_linux_wrapper.h"
#include "vceb_linux_host_interface.h"
#include "interface/vchiq_arm/vchiq_bi_ipc_shared_mem.h"

//#include <csp/chal_ccu_inline.h>
//#include <csp/chal_ccu_kps_inline.h>
#include <chal/chal_ipc.h>
//#include <csp/gpiomux.h>
//#include <csp/gpiomux_hana.h>
#include <mach/mpuHw.h>

/******************************************************************************
Private defs
******************************************************************************/

static  int  gDbgLogRwPerf              = 0;
static  int  gDbgLogBootTrace           = 1;
static  int  gDbgLogIntfTrace           = 0;
static  int  gDbgLogIntfTraceLow        = 0;
static  int  gDbgLogIntfTraceData       = 0;
static  int  gDbgLogIntfTraceDataSize   = 64;

module_param_named( rw_perf,                gDbgLogRwPerf,              bool,   0644 );
module_param_named( boot_trace,             gDbgLogBootTrace,           int,    0644 );
module_param_named( intf_trace,             gDbgLogIntfTrace,           int,    0644 );
module_param_named( intf_trace_low,         gDbgLogIntfTraceLow,        int,    0644 );
module_param_named( intf_trace_data,        gDbgLogIntfTraceData,       int,    0644 );
module_param_named( intf_trace_data_size,   gDbgLogIntfTraceDataSize,   int,    0644 );

struct opaque_vceb_host_interface_state_t
{
    VCEB_INSTANCE_T         vceb_inst;
    VCEB_LINUX_INSTANCE_T   linux_inst;
    int                     initialized;
    CHAL_IPC_HANDLE         ipcHandle;
};

#define VCBOOT_VC4_BOOT_ADDR        0xC0000200  // Address to start running videocore from (I think this should be 0x200)

static int vceb_is_videocore_running = 0;

/****************************************************************************
*
* vceb_kona_interface_initialize
*
*   Initializes the videocore interface. This is called from the probe routine
*   and when resuming a videocore.
*
***************************************************************************/

int32_t vceb_kona_interface_initialize( VCEB_HOST_INTERFACE_INSTANCE_T instance )
{
    int                         rc;
    VCEB_HOST_INTERFACE_STATE_T state = instance->state;
    uint32_t                    wakeup_register;

    /*
     * Enable Clocks
     */

    state->ipcHandle = chal_ipc_config( NULL );
    /* gpio pin mux is setup as part of the board configuration */
    /* we will have to request the non muxed gpio pins here and set it to the appropriate values */

    /*
     * Check the IPC Awake register to see ig the videocore is already running.
     * If it's running, then we assume that u-boot initialized it.
     */

    chal_ipc_query_wakeup_vc( state->ipcHandle, &wakeup_register );
    if (( wakeup_register & ~1 ) == 0 )
    {
        vceb_is_videocore_running = 0;
    }
    else
    {
        vceb_is_videocore_running = 1;
    }

    /*
     * FIXME All the hardcoded GPIO pin numbers should be moved to the
     * platform_data! Otherwise this code may not work when we move to another
     * platform that has a different GPIO muxing.
     */

#if defined(CONFIG_BCM_HDMI_DET) || defined(CONFIG_BCM_HDMI_DET_MODULE)
    /* leave the gpio alone so it can be claimed by the hot-plug detection
    ** driver.
    */
    printk( KERN_INFO "%s: skipping gpio mux for HDMI hotplug detect pin\n",
            __func__ );
#else
    /* request HDMI hot plug gpio */
 #define HDMI_HOT_PLUG    62
    if (( rc = gpio_request( HDMI_HOT_PLUG, "hdmi_hot_plug" )) != 0 )
    {
        printk( KERN_ERR "%s: gpio_request( %d, 'hdmi_hot_plug' ) failed: %d\n",
                __func__, HDMI_HOT_PLUG, rc );
        return -ENODEV;
    }
    //explicitly set the direction for GPIO pins that are muxed to the host
    gpio_direction_input( 62 );   //HDMI_HOT_DETECT
#endif

#define CAM1_PWR_EN  50
#define CAM1_RST_B   51
#define CAM2_GPIO2   168
#define CAM2_GPIO1   169
#define CAM2_RST_B   170
#define CAM2_GPIO0   174
    if (( rc = gpio_request( CAM1_PWR_EN, "cam1_pwr_en" )) != 0 )
    {
       printk( KERN_ERR "%s: gpio_request( %d, 'cam1_pwr_en' ) failed: %d\n",
               __func__, CAM1_PWR_EN, rc );
       return -ENODEV;
    }
    if (( rc = gpio_request( CAM1_RST_B, "cam1_rst_b" )) != 0 )
    {
       printk( KERN_ERR "%s: gpio_request( %d, 'cam1_rst_b' ) failed: %d\n",
               __func__, CAM1_RST_B, rc );
       return -ENODEV;
    }
    if (( rc = gpio_request( CAM2_RST_B, "cam2_rst_b" )) != 0 )
    {
       printk( KERN_ERR "%s: gpio_request( %d, 'cam2_rst_b' ) failed: %d\n",
               __func__, CAM2_RST_B, rc );
       return -ENODEV;
    }
    if (( rc = gpio_request( CAM2_GPIO2, "cam2_gpio2" )) != 0 )
    {
       printk( KERN_ERR "%s: gpio_request( %d, 'cam2_gpio2' ) failed: %d\n",
               __func__, CAM2_GPIO2, rc );
       return -ENODEV;
    }
    if (( rc = gpio_request( CAM2_GPIO1, "cam2_gpio1" )) != 0 )
    {
       printk( KERN_ERR "%s: gpio_request( %d, 'cam2_gpio1' ) failed: %d\n",
               __func__, CAM2_GPIO1, rc );
       return -ENODEV;
    }
    if (( rc = gpio_request( CAM2_GPIO0, "cam2_gpio0" )) != 0 )
    {
       printk( KERN_ERR "%s: gpio_request( %d, 'cam2_gpio0' ) failed: %d\n",
               __func__, CAM2_GPIO0, rc );
       return -ENODEV;
    }

    //explicitly set the direction for GPIO pins that are muxed to the host
    gpio_direction_output( 50, 1 );    //CAM1_PWR_EN/CAM1_WP
    gpio_direction_output( 51, 0 );    //CAM1_RST_B
    gpio_direction_output( 169, 0 );   //CAM2_GPIO1
    gpio_direction_output( 170, 0 );   //CAM2_RST_B

    {
        /*
         * Set the entire SRAM to be unsecure. The API only allows is to do 4K at a time
         * instead of 3 x 32 register writes.
         */

        printk( KERN_INFO "calling mpu Memory Region ACCESS_OPEN\n" );

        mpuHw_MEMORY_REGION_e       region;

        for ( region = mpuHw_MEMORY_REGION_0K_4K; region <= mpuHw_MEMORY_REGION_160K_164K; region ++ )
        {
           mpuHw_setSRAM_AccessMode( region, mpuHw_MEMORY_ACCESS_OPEN );
        }
    }
    return 0;
}

/****************************************************************************
*
* vceb_kona_interface_shutdown
*
*   Shuts down the videocore interface. This is called prior to suspending
*   a videocore, and before unloading the module.
*
***************************************************************************/

int32_t vceb_kona_interface_shutdown( VCEB_HOST_INTERFACE_INSTANCE_T instance )
{
    (void)instance;

    printk( KERN_ERR "%s: Not Implemented yet\n", __func__ );

    return 0;
}

/****************************************************************************
*
* vceb_kona_interface_control_run_pin
*
*   Manipulates the RUN pin connected to the videocore.
*
***************************************************************************/

int32_t vceb_kona_interface_control_run_pin( VCEB_HOST_INTERFACE_INSTANCE_T instance,
                                        const uint32_t run_pin_high )
{
    (void)instance;
    (void)run_pin_high;

    printk( KERN_ERR "%s: Not Implemented yet\n", __func__ );

    return -1;
}

/****************************************************************************
*
* vceb_kona_interface_download
*
*   Downloads firmware to the videocore.
*
***************************************************************************/

static int32_t vceb_kona_interface_download( VCEB_HOST_INTERFACE_INSTANCE_T instance,
                                             const void * const data,
                                             const uint32_t data_size )
{
    int32_t ret = -1;
    VCEB_HOST_INTERFACE_STATE_T state = instance->state;
    VCEB_PLATFORM_DATA_KONA_T  *platform_data = instance->host_param;
    struct resource *res;
    void *vc_mem;

    if ( vceb_is_videocore_running )
    {
        printk( KERN_INFO "VCEB: Internal Videocore already running. Ignoring download request...\n" );

        ret = 0;
        goto out;
    }

    printk( KERN_INFO "VCEB download address 0x%x data_size = %d\n",
          (unsigned int)platform_data->vcMemAddr,
          data_size );

    // Request an I/O memory region big enough for the videocore image
    res = request_mem_region( platform_data->vcMemAddr, data_size,
                              "vceb download" );
    if ( res == NULL )
    {
        printk( KERN_ERR "VCEB: Failed to request I/O memory region\n" );
        goto out;
    }

    // I/O remap the videocore memory
    vc_mem = ioremap_nocache( res->start, resource_size( res ));
    if ( vc_mem == NULL )
    {
        printk( KERN_ERR "VCEB: Failed to I/O remap\n" );
        goto rel_mem_region;
    }

    printk( KERN_INFO "VCEB: Downloading Videocore image (%u bytes) to 0x%p\n",
            data_size, vc_mem );


    // Copy the videocore image to the videocore memory
    memcpy_toio( vc_mem, data, data_size );

    chal_ipc_wakeup_vc( state->ipcHandle, VCBOOT_VC4_BOOT_ADDR );

    /* Wait for Videocore boot. */
    msleep( 2000 );

    printk( KERN_INFO "VCEB: Internal Videocore running...\n" );

    // Success!
    ret = 0;

    // Unmap the videocore memory
    iounmap( vc_mem );

    vceb_firmware_downloaded();

rel_mem_region:
    // Release the I/O memory region
    release_mem_region( res->start, resource_size( res ));

out:
    return ret;
}

/****************************************************************************
*
* vceb_kona_interface_download_status
*
*   Returns status information about the download.
*
***************************************************************************/

static int vceb_kona_interface_download_status( VCEB_HOST_INTERFACE_INSTANCE_T instance,
                                                char *buf,
                                                int count )
{
    VCEB_PLATFORM_DATA_KONA_T  *platform_data = instance->host_param;
    char *p = buf;

    uint32_t   clockDebug = *(volatile uint32_t *)( platform_data->vcSramAddr
                                                  - IPC_SHARED_MEM_CHANNEL_VC_OFFSET
                                                  + IPC_SHARED_MEM_CLOCK_DEBUG_OFFSET );

    p += sprintf( p, "Stability Test:\n" );

    if ((( clockDebug >> 16 ) & 0xffff ) != 0xBABE )
    {
       p += sprintf( p, "WARNING! Stability test did not run!\n" );
    }
    else
    {
       int first_pass = clockDebug & 0xff;

       if( first_pass != 100 )
       {
          int second_pass = (clockDebug >> 8) & 0xff;

          p += sprintf( p, "1st pass FAILED... %u/100\n", first_pass );

          if( second_pass != 100 )
          {
             p += sprintf( p, "2nd pass FAILED... %u/100\n", second_pass );
          }
          else
          {
             p += sprintf( p, "2nd pass PASSED... %u/100\n", second_pass );
          }
       }
       else
       {
          p += sprintf( p, "1st pass PASSED... %u/100\n", first_pass );
       }
    }

    return p - buf;
}

/****************************************************************************
*
* vceb_kona_interface_get_stat
*
*   Returns statistics information.
*
***************************************************************************/

int32_t vceb_kona_interface_get_stat( VCEB_HOST_INTERFACE_INSTANCE_T instance,
                                 const char * const stat_name,
                                 char *buffer,
                                 const uint32_t buffer_len )
{
    (void)instance;
    (void)stat_name;
    (void)buffer;
    (void)buffer_len;

    return 0;
}

/****************************************************************************
*
* vceb_kona_interface_funcs
*
*   Interface functions.
*
***************************************************************************/

static VCEB_HOST_INTERFACE_FUNCS_T vceb_kona_interface_funcs =
{
    "kona",
    vceb_kona_interface_initialize,
    vceb_kona_interface_shutdown,
    vceb_kona_interface_control_run_pin,
    vceb_kona_interface_download,
    vceb_kona_interface_download_status,
    NULL,   // readData
    NULL,   // writeData
    vceb_kona_interface_get_stat,
};

/****************************************************************************
*
* vceb_kona_interface_probe
*
*   This function will be called for each "vceb_kona" device which is
*   registered in the board definition.
*
***************************************************************************/

static int __devinit vceb_kona_interface_probe( struct platform_device *pdev )
{
    int     rc = -ENODEV;
    VCEB_PLATFORM_DATA_KONA_T *platform_data = pdev->dev.platform_data;
    const char *name = platform_data->create_params.instance_name;
    VCEB_HOST_INTERFACE_INSTANCE_T instance = NULL;

    printk( KERN_INFO "vceb_kona: Probing %s ...\n", name );

    /*
     * Since this is for the internal videocore, we assume that it's always
     * present.
     */

    if ( vceb_linux_host_interface_alloc( &platform_data->create_params,
                                          &vceb_kona_interface_funcs,
                                          &instance ) != 0 )
    {
        printk( KERN_ERR "%s: vceb_linux_host_interface_alloc failed\n", __func__ );
        rc = -ENOMEM;
        goto err;
    }

    if (( instance->state = kzalloc( sizeof( *instance->state ), GFP_KERNEL )) == NULL )
    {
        printk( KERN_ERR "%s: Unable to allocate %d bytes for kona_interface state data\n",
                __func__, sizeof( *instance->state ));
        rc = -ENOMEM;
        goto err;
    }

    snprintf( instance->descr, sizeof( instance->descr ),
              "KONA Internal VC" );

    if ( vceb_kona_interface_initialize( instance ) != 0 )
    {
        rc = -ENODEV;
        goto err;
    }

    if ( vceb_linux_create_instance( &platform_data->create_params,
                                     instance,
                                     &instance->state->vceb_inst,
                                     &instance->state->linux_inst ))
    {
        printk( KERN_ERR "%s: vceb_create_instance( '%s' ) failed\n",
                __func__, name );

        rc = -ENODEV;
        goto err;
    }
    platform_set_drvdata( pdev, instance );

    printk( KERN_INFO "vceb_kona: DDR  Memory: 0x%08x\n", platform_data->vcMemAddr );
    printk( KERN_INFO "vceb_kona: SRAM Memory: 0x%08x\n", platform_data->vcSramAddr );
    printk( KERN_INFO "vceb_kona: %s detected\n", name );
    printk( KERN_INFO "vceb_kona: bootFromKernel: %d\n", platform_data->bootFromKernel );
#if 0
    printk( KERN_INFO "vceb_kona:    LCD gpiomux: group: %d id: %d label: '%s'\n",
            platform_data->gpiomux_lcd_group,
            platform_data->gpiomux_lcd_id,
            platform_data->gpiomux_lcd_label );
    printk( KERN_INFO "vceb_kona:   JTAG gpiomux: group: %d id: %d label: '%s'\n",
            platform_data->gpiomux_jtag_group,
            platform_data->gpiomux_jtag_id,
            platform_data->gpiomux_jtag_label );
#endif

    {
        VCEB_INSTANCE_T vceb_inst;

        // The videocore is already running (probably loaded by uboot). Auto initialize.

        printk( KERN_INFO "vceb_hana: Videocore already running - autoinitializing\n" );

        vceb_inst = vceb_linux_get_vceb_instance( instance->state->linux_inst );
        vceb_initialise( vceb_inst, 1 );
    }

    if ( vceb_is_videocore_running )
    {
        vceb_firmware_downloaded();
    }

    return 0;

err:

    if ( instance != NULL )
    {
        if ( instance->state != NULL )
        {
            if ( instance->state->initialized )
            {
                vceb_kona_interface_shutdown( instance );
            }
            kfree( instance->state );
        }
        vceb_linux_host_interface_free( instance );
    }

    return rc;
}

/****************************************************************************
*
* vceb_kona_interface_remove
*
*   Register a "driver". We do this so that the probe routine will be called
*   when a corresponding architecture device is registered.
*
***************************************************************************/

static int vceb_kona_interface_remove( struct platform_device *pdev )
{
    VCEB_HOST_INTERFACE_INSTANCE_T instance;

    instance = platform_get_drvdata( pdev );
    if ( instance != NULL )
    {
        if ( instance->state->linux_inst != NULL )
        {
            vceb_linux_delete_instance( instance->state->linux_inst );
        }

        if ( instance->state != NULL )
        {
            if ( instance->state->initialized )
            {
                vceb_kona_interface_shutdown( instance );
            }
            kfree( instance->state );
        }
        vceb_linux_host_interface_free( instance );
    }

    platform_set_drvdata( pdev, NULL );
    return 0;
}

/****************************************************************************
*
* vceb_kona_interface_driver
*
*   Register a "driver". We do this so that the probe routine will be called
*   when a corresponding architecture device is registered.
*
***************************************************************************/

static struct platform_driver vceb_kona_interface_driver = {
    .probe          = vceb_kona_interface_probe,
    .remove         = vceb_kona_interface_remove,
    .driver = {
        .name	    = "vceb_kona",
    }
};

/****************************************************************************
*
* vceb_kona_interface_init
*
*   Creates the instance that is used to access the videocore(s). One
*   instance is created per videocore.
*
***************************************************************************/

static int __init vceb_kona_interface_init( void )
{
   int error;

    printk( KERN_INFO "vceb_kona_interface_init called\n" );
    error = platform_driver_register( &vceb_kona_interface_driver );
    printk( KERN_INFO "vceb_kona_interface_init driver register result = %d\n", error );

    return error;
}

/****************************************************************************
*
* vceb_kona_interface_exit
*
*   Called when the module is unloaded.
*
***************************************************************************/

static void __exit vceb_kona_interface_exit( void )
{
    platform_driver_unregister( &vceb_kona_interface_driver );
}

module_init( vceb_kona_interface_init );
module_exit( vceb_kona_interface_exit );

MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("VCEB Kona Interface Driver");
MODULE_LICENSE("GPL");

