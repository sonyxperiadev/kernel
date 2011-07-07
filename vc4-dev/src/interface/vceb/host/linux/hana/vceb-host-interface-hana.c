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
#include <linux/vceb_platform_data_hana.h>

//#include <linux/videocore/vc_boot_mode.h>

#include "vceb_linux_wrapper.h"
#include "vceb_linux_host_interface.h"
#include "vcinclude/bigisland_chip/ipc_shared_mem.h"

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

/****************************************************************************
*
* vceb_hana_interface_initialize
*
*   Initializes the videocore interface. This is called from the probe routine
*   and when resuming a videocore.
*
***************************************************************************/

int32_t vceb_hana_interface_initialize( VCEB_HOST_INTERFACE_INSTANCE_T instance )
{
    int                         rc;
    //gpiomux_rc_e                gpiomux_rc;
    VCEB_PLATFORM_DATA_HANA_T  *platform_data = instance->host_param;
    VCEB_HOST_INTERFACE_STATE_T state = instance->state;
#if 0
    DBG_LOG( BootTrace, "called, vc_boot_mode = '%s'", vc_boot_mode );

    if ( !platform_data->bootFromKernel )
    {
        if ( strcmp( vc_boot_mode, "none" ) == 0 )
        {
            /*
             * This means that the bootloader didn't initialize the videocore, or that
             * it requested that the videocore not be initialized. In this case we fail
             * so that the bootup will proceed.
             */
    
            printk( KERN_ERR "%s: CFG_GLOBAL_VIDEO_BOOT_FROM_KERNEL == 0, vc_boot_mode = '%s'\n", __func__, vc_boot_mode );
            printk( KERN_ERR "%s: Not initializing videocore\n", __func__ );
            return -1;
        }
    }

    if ( state->initialized )
    {
        /* The first videocore may be initialized via vceb_fb_probe early on
        * in the kernel boot.
        */

        DBG_LOG( BootTrace, "already initialized - leaving early" );
        return 0;
    }
#endif
    /*
     * Enable Clocks
     */

    state->ipcHandle = chal_ipc_config( NULL );
    /* gpio pin mux is setup as part of the board configuration */
    /* we will have to request the non muxed gpio pins here and set it to the appropriate values */

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

    /*
     * This code doesn't really belong here, but it's good enough for now.
     */

 #define LCD_BL_POWER_ENABLE 141
 #define LCD_BL_ENABLE        69
 #define LCD_BL_PWM          145
 #define LCD_RESET           146
 #define LCD_POWER_ENABLE    142

    if (( rc = gpio_request( LCD_BL_POWER_ENABLE, "bl-pwr-en" )) != 0 )
    {
        printk( KERN_ERR "%s: gpio_request( %d, 'bl-pwr-en' ) failed: %d\n", 
                __func__, LCD_BL_POWER_ENABLE, rc );
        return -ENODEV;
    }
    if (( rc = gpio_request( LCD_BL_ENABLE, "bl-en" )) != 0 )
    {
        printk( KERN_ERR "%s: gpio_request( %d, 'bl-en' ) failed: %d\n", 
                __func__, LCD_BL_ENABLE, rc );
        return -ENODEV;
    }
    if (( rc = gpio_request( LCD_BL_PWM, "bl-pwm" )) != 0 )
    {
        printk( KERN_ERR "%s: gpio_request( %d, 'bl-pwm' ) failed: %d\n", 
                __func__, LCD_BL_PWM, rc );
        return -ENODEV;
    }
    if (( rc = gpio_request( LCD_RESET, "lcd-reset" )) != 0 )
    {
        printk( KERN_ERR "%s: gpio_request( %d, 'lcd-reset' ) failed: %d\n", 
                __func__, LCD_RESET, rc );
        return -ENODEV;
    }
    if (( rc = gpio_request( LCD_POWER_ENABLE, "lcd-pwr-en" )) != 0 )
    {
        printk( KERN_ERR "%s: gpio_request( %d, 'lcd-pwr-en' ) failed: %d\n", 
                __func__, LCD_POWER_ENABLE, rc );
        return -ENODEV;
    }

    gpio_direction_output( LCD_RESET,           0 );
    gpio_direction_output( LCD_POWER_ENABLE,    0 );
    gpio_direction_output( LCD_BL_PWM,          0 );
    gpio_direction_output( LCD_BL_ENABLE,       0 );
    gpio_direction_output( LCD_BL_POWER_ENABLE, 0 );

    gpio_set_value( LCD_POWER_ENABLE,    1 );
    gpio_set_value( LCD_RESET,           1 );
    gpio_set_value( LCD_BL_POWER_ENABLE, 1 );
    gpio_set_value( LCD_BL_ENABLE,       1 );
    gpio_set_value( LCD_BL_PWM,          1 );

    return 0;
}

/****************************************************************************
*
* vceb_hana_interface_shutdown
* 
*   Shuts down the videocore interface. This is called prior to suspending
*   a videocore, and before unloading the module.
*
***************************************************************************/

int32_t vceb_hana_interface_shutdown( VCEB_HOST_INTERFACE_INSTANCE_T instance )
{
    (void)instance;

    printk( KERN_ERR "%s: Not Implemented yet\n", __func__ );

    return 0;
}

/****************************************************************************
*
* vceb_hana_interface_control_run_pin
* 
*   Manipulates the RUN pin connected to the videocore.
*
***************************************************************************/

int32_t vceb_hana_interface_control_run_pin( VCEB_HOST_INTERFACE_INSTANCE_T instance,
                                        const uint32_t run_pin_high )
{
    (void)instance;
    (void)run_pin_high;

    printk( KERN_ERR "%s: Not Implemented yet\n", __func__ );

    return -1;
}

/****************************************************************************
*
* vceb_hana_interface_download
*
*   Downloads firmware to the videocore.
*
***************************************************************************/

int32_t vceb_hana_interface_download( VCEB_HOST_INTERFACE_INSTANCE_T instance,
                                 const void * const data,
                                 const uint32_t data_size )
{
    VCEB_HOST_INTERFACE_STATE_T state = instance->state;
    VCEB_PLATFORM_DATA_HANA_T  *platform_data = instance->host_param;

    printk( KERN_INFO "VCEB download address 0x%x data_size = %d\n", 
          (unsigned int)platform_data->vcMemAddr,
          data_size );
    memcpy( (void *)platform_data->vcMemAddr, data, data_size );
    chal_ipc_wakeup_vc( state->ipcHandle, VCBOOT_VC4_BOOT_ADDR );

    /* Wait for Videocore boot. */
    msleep( 2000 );

    printk( KERN_INFO "VCEB: Internal Videocore running...\n" );

    return 0;
}

/****************************************************************************
*
* vceb_hana_interface_download_status
*
*   Returns status information about the download.
*
***************************************************************************/

static int vceb_hana_interface_download_status( VCEB_HOST_INTERFACE_INSTANCE_T instance, 
                                                char *buf,
                                                int count )
{
    VCEB_PLATFORM_DATA_HANA_T  *platform_data = instance->host_param;
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
* vceb_hana_interface_get_stat
*
*   Returns statistics information.
*
***************************************************************************/

int32_t vceb_hana_interface_get_stat( VCEB_HOST_INTERFACE_INSTANCE_T instance,
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
* vceb_hana_interface_funcs
*
*   Interface functions.
*
***************************************************************************/

static VCEB_HOST_INTERFACE_FUNCS_T vceb_hana_interface_funcs =
{
    "hana",
    vceb_hana_interface_initialize,
    vceb_hana_interface_shutdown,
    vceb_hana_interface_control_run_pin,
    vceb_hana_interface_download,
    vceb_hana_interface_download_status,
    NULL,   // readData
    NULL,   // writeData
    vceb_hana_interface_get_stat,
};

/****************************************************************************
*
* vceb_hana_interface_probe
*
*   This function will be called for each "vceb_hana" device which is
*   registered in the board definition.
*
***************************************************************************/

static int __devinit vceb_hana_interface_probe( struct platform_device *pdev )
{
    int     rc = -ENODEV;
    VCEB_PLATFORM_DATA_HANA_T *platform_data = pdev->dev.platform_data;
    const char *name = platform_data->create_params.instance_name;
    VCEB_HOST_INTERFACE_INSTANCE_T instance = NULL;

    printk( KERN_INFO "vceb_hana: Probing %s ...\n", name );

    /*
     * Since this is for the internal videocore, we assume that it's always 
     * present. 
     */

    if ( vceb_linux_host_interface_alloc( &platform_data->create_params,
                                          &vceb_hana_interface_funcs,
                                          &instance ) != 0 )
    {
        printk( KERN_ERR "%s: vceb_linux_host_interface_alloc failed\n", __func__ );
        rc = -ENOMEM;
        goto err;
    }

    if (( instance->state = kzalloc( sizeof( *instance->state ), GFP_KERNEL )) == NULL )
    {
        printk( KERN_ERR "%s: Unable to allocate %d bytes for hana_interface state data\n",
                __func__, sizeof( *instance->state ));
        rc = -ENOMEM;
        goto err;
    }

    snprintf( instance->descr, sizeof( instance->descr ),
              "HANA Internal VC" );

    if ( vceb_hana_interface_initialize( instance ) != 0 )
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

    printk( KERN_INFO "vceb_hana: DDR  Memory: 0x%08x\n", platform_data->vcMemAddr );
    printk( KERN_INFO "vceb_hana: SRAM Memory: 0x%08x\n", platform_data->vcSramAddr );
    printk( KERN_INFO "vceb_hana: %s detected\n", name );
    printk( KERN_INFO "vceb_hana: bootFromKernel: %d\n", platform_data->bootFromKernel );
#if 0
    printk( KERN_INFO "vceb_hana:    LCD gpiomux: group: %d id: %d label: '%s'\n", 
            platform_data->gpiomux_lcd_group,
            platform_data->gpiomux_lcd_id,
            platform_data->gpiomux_lcd_label );
    printk( KERN_INFO "vceb_hana:   JTAG gpiomux: group: %d id: %d label: '%s'\n", 
            platform_data->gpiomux_jtag_group,
            platform_data->gpiomux_jtag_id,
            platform_data->gpiomux_jtag_label );
#endif

    return 0;

err:

    if ( instance != NULL )
    {
        if ( instance->state != NULL )
        {
            if ( instance->state->initialized )
            {
                vceb_hana_interface_shutdown( instance );
            }
            kfree( instance->state );
        }
        vceb_linux_host_interface_free( instance );
    }

    return rc;
}

/****************************************************************************
*
* vceb_hana_interface_remove
*
*   Register a "driver". We do this so that the probe routine will be called
*   when a corresponding architecture device is registered.
*
***************************************************************************/

static int vceb_hana_interface_remove( struct platform_device *pdev )
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
                vceb_hana_interface_shutdown( instance );
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
* vceb_hana_interface_driver
*
*   Register a "driver". We do this so that the probe routine will be called
*   when a corresponding architecture device is registered.
*
***************************************************************************/

static struct platform_driver vceb_hana_interface_driver = {
    .probe          = vceb_hana_interface_probe,
    .remove         = vceb_hana_interface_remove,
    .driver = {
        .name	    = "vceb_hana",
    }
};

/****************************************************************************
*
* vceb_hana_interface_init
*
*   Creates the instance that is used to access the videocore(s). One
*   instance is created per videocore.
*
***************************************************************************/

static int __init vceb_hana_interface_init( void )
{
   int error;
#if 0
    if ( vc_boot_mode_skip() )
    {
        return -ENODEV;
    }
#endif
    printk( KERN_INFO "vceb_hana_interface_init called\n" );
    error = platform_driver_register( &vceb_hana_interface_driver );
    printk( KERN_INFO "vceb_hana_interface_init driver register result = %d\n", error );

    return error;
}

/****************************************************************************
*
* vceb_hana_interface_exit
* 
*   Called when the module is unloaded.
*
***************************************************************************/

static void __exit vceb_hana_interface_exit( void )
{
#if 0
    if ( vc_boot_mode_skip() )
    {
        return;
    }
#endif
    platform_driver_unregister( &vceb_hana_interface_driver );
}

module_init( vceb_hana_interface_init );
module_exit( vceb_hana_interface_exit );

MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("VCEB Hana Interface Driver");
MODULE_LICENSE("GPL");

