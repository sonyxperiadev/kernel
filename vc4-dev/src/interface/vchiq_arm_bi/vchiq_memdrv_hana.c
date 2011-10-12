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

#define IGNORE_CHAL_UTILS 1

/* ---- Include Files ---------------------------------------------------- */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/proc_fs.h>
#include <linux/platform_device.h>

//#include <linux/broadcom/timer.h>
//#include <linux/broadcom/dbg-log.h>
#include <chal/chal_ipc.h>
#include <mach/irqs.h>
#include <chal/chal_icd.h>
#include <mach/io_map.h>
#include <asm/gpio.h>

#include <linux/vchiq_platform_data_memdrv_hana.h>

#include <mach/sdma.h>
#include <mach/dma_mmap.h>
//#include <linux/broadcom/knllog.h>

//#include <linux/videocore/vc_boot_mode.h>

#include "vchiq_core.h"

/* ---- Private Stuff ---------------------------------------------------- */

#define IPC_SHARED_MEM_BUFFER_A 0xe8060000
#define IPC_SHARED_MEM_BUFFER_B 0xe8064000

#define SHARED_MAILBOX(num)        (*(volatile uint32_t*)(0xe8066100+num*sizeof(uint32_t)))

#if 0
struct opaque_vchiq_memdrv_driver_data_t
{
    VCHIQ_PLATFORM_DATA_HANA_T *platform_data;
#if 0
    VCHIQ_MEMDRV_INSTANCE_T     memdrv_instance;
#endif
    
    CHAL_IPC_HANDLE             ipcHandle;
    DMA_MMAP_CFG_T              dma_mmap;    
};
#endif

#if 0
struct BusDriver_OpaqueHandle_s
{
    CHAL_IPC_HANDLE     ipcHandle;
    uint32_t            enabled;
    uint32_t            suspended;

    uint32_t                            irqEnabled;
//    BusDriver_InterruptHandlerType      irqHandler;
    void                               *irqContext;
    int                                 irqSuspendDetectCounter;

    const VCHIQ_PLATFORM_DATA_HANA_T   *platform_data;

#if 0

    int                 rwPerf;
    int                 dmaDisabled;
    int                 dmaMinSize;
    int                 dmaDumpInfo;
#endif
    int                 intfTrace;
    int                 intfTraceLow;
    int                 intfTraceData;
    int                 intfTraceDataSize;

    const char         *lockHolderStr;

    uint32_t            writeCount;

    uint32_t            readCount;

    uint32_t            lastVcStatus;
    uint8_t            *dataPtr;
    uint32_t            bytesRemaining;
    uint32_t            bytesThisTime;

    int                         vectorNumElements;
    int                         vectorIdx;
//    const BusDriver_VectorType *vector;

    struct proc_dir_entry *busDriver_proc_dir;

    struct proc_dir_entry *intfTrace_proc_entry;
    struct proc_dir_entry *intfTraceLow_proc_entry;
    struct proc_dir_entry *intfTraceData_proc_entry;
    struct proc_dir_entry *intfTraceDataSize_proc_entry;
    struct proc_dir_entry *status_proc_entry;

#if 0
    struct proc_dir_entry *rwPerf_proc_entry;
    struct proc_dir_entry *dmaDisabled_proc_entry;
    struct proc_dir_entry *dmaMinSize_proc_entry;
    struct proc_dir_entry *dmaDumpInfo_proc_entry;
#endif
#define NUM_COMPLETION_ENTRIES 128
    OS_EVENT_T         *completionEvent;
    volatile uint32_t   completion[ NUM_COMPLETION_ENTRIES ];
    volatile uint32_t   completionHead;
    volatile uint32_t   completionTail;
};
#endif

#if 0
//XXX TODO need to move these into per instance variables!!
static DMA_MMAP_CFG_T    gVchiqDmaMmap;
static SDMA_Handle_t     dmaHndl;
static struct completion gDmaDone;
static CHAL_IPC_HANDLE  ipcHandle;

static struct proc_dir_entry   *gIrqTraceProcEntry = NULL;
#endif

/*
 * We may have multiple instances of the bus driver, so we play a game 
 * in order to allow the DBG_LOG stuff to work. This assumes that every 
 * function will have a variable called instance which points to the 
 * instance data.
 */

#define gDbgLogIntfTrace        instance->intfTrace
#define gDbgLogIntfTraceLow     instance->intfTraceLow
#define gDbgLogIntfTraceData    instance->intfTraceData

static  int gDbgLogIrqTrace             = 0;
static  int gDefaultIntfTrace           = 0;
static  int gDefaultIntfTraceLow        = 0;
static  int gDefaultIntfTraceData       = 0;
static  int gDefaultIntfTraceDataSize   = 64;

module_param_named( irq_trace,              gDbgLogIrqTrace,            int, 0644 );

module_param_named( intf_trace,             gDefaultIntfTrace,          int, 0644 );
module_param_named( intf_trace_low,         gDefaultIntfTraceLow,       int, 0644 );
module_param_named( intf_trace_data,        gDefaultIntfTraceData,      int, 0644 );
module_param_named( intf_trace_data_size,   gDefaultIntfTraceDataSize,  int, 0644 );

/****************************************************************************
*
*   Called to display a proc entry that's an integer
*
***************************************************************************/

#if 0
static int proc_status_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
#if 0
   uint32_t mailboxValue_0, mailboxValue_1, mailboxValue_2, mailboxValue_3;
   BusDriver_Instance_t        *instance = data;
   char       *p = page;

   p += sprintf( p, "Driver: Hana\n" );

   if ( instance == NULL )
   {
       p += sprintf( p, "Instance is NULL\n" );
   }
   else
   {
       p += sprintf( p, "readCount: %d writeCount: %d\n", instance->readCount, instance->writeCount );

       mailboxValue_0 = SHARED_MAILBOX(IPC_MAILBOX_ID_0);
       mailboxValue_1 = SHARED_MAILBOX(IPC_MAILBOX_ID_1);
       mailboxValue_2 = SHARED_MAILBOX(IPC_MAILBOX_ID_2);
       mailboxValue_3 = SHARED_MAILBOX(IPC_MAILBOX_ID_3);

       p += sprintf( p, "mailboxes 0x%x 0x%x 0x%x 0x%x\n", (uint32_t)mailboxValue_0, (uint32_t)mailboxValue_1, (uint32_t)mailboxValue_2, (uint32_t)mailboxValue_3 );

       p += sprintf( p, "\n" );
       p += sprintf( p, "Lock Holder: %s\n", instance->lockHolderStr );
   }

   p += sprintf( p, "\nL2 Cache: %s\n",
#if defined( CONFIG_CACHE_L2X0 )
                 "enabled"
#else
                 "disabled"
#endif
               );

   return p - page;
#else //TODO
    return 0;
#endif
}
#endif

/****************************************************************************
*
*   Called to create the status proc entry
*
***************************************************************************/

#if 0
static struct proc_dir_entry *proc_create_status_entry( struct proc_dir_entry *parent, 
                                                        const char *name, 
                                                        BusDriver_Instance_t *instance )
{
    struct proc_dir_entry   *de = NULL;

    if (( de = create_proc_entry( name, 0444, parent )) == NULL )
    {
        remove_proc_entry( name, parent );
        printk( KERN_ERR "%s: Unable to create proc entry for '%s'\n", __func__, name );
        return NULL;
    }

    de->read_proc = proc_status_read;
    de->write_proc = NULL;
    de->data = instance;

    return de;
}
#endif

/****************************************************************************
*
*   Called to display a proc entry that's an integer
*
***************************************************************************/

#if 0
static int proc_int_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
    char *p = page;
    int len;

    if ( data == NULL )
    {
        printk( KERN_ERR "%s: data == NULL\n", __func__ );
        p += sprintf( p, "(null)\n" );
    }
    else
    {
        p += sprintf( p, "%d\n", *(int *)data );
    }

    len = (p - page) - off;
    if (len < 0)
        len = 0;

    *eof = (len <= count) ? 1 : 0;
    *start = page + off;

    return len;
}
#endif

/****************************************************************************
*
*   Called to write a proc entry that's an integer
*
***************************************************************************/

#if 0
static int proc_int_write( struct file *file, const char __user *buffer,
				           unsigned long count, void *data )
{
    char    intStr[ 100 ];

    if ( count > sizeof( intStr ))
    {
        count = sizeof( intStr );
    }
    if ( copy_from_user( intStr, buffer, count ))
    {
        return -EFAULT;
    }

    if ( data == NULL )
    {
        printk( KERN_ERR "%s: data == NULL\n", __func__ );
    }
    else
    {
        *(int *)data = simple_strtoul( intStr, NULL, 0 ); 
    }

    return count;
}
#endif

/****************************************************************************
*
*   Called to create a proc entry that's an integer
*
***************************************************************************/

#if 0
static struct proc_dir_entry *proc_create_int_entry( struct proc_dir_entry *parent, 
                                                     const char *name, 
                                                     int *data )
{
    struct proc_dir_entry   *de = NULL;

    if (( de = create_proc_entry( name, 0644, parent )) == NULL )
    {
        remove_proc_entry( name, parent );
        printk( KERN_ERR "%s: Unable to create proc entry for '%s'\n", __func__, name );
        return NULL;
    }

    de->read_proc = proc_int_read;
    de->write_proc = proc_int_write;
    de->data = data;

    return de;
}
#endif

/****************************************************************************
*
*   vchiq_memdrv_hana_doorbell_irq
* 
*   ISR which is called when the videocore rings the doorbell on its side.
*
***************************************************************************/

#if 0
static irqreturn_t vchiq_memdrv_hana_doorbell_irq( int irq, void *dev_id )
{
   IPC_INTERRUPT_SOURCE source;
   uint32_t             status;

   /* get the interrupt status value */
   chal_ipc_get_int_status( ipcHandle, &status );

   /* clear all the interrupts first */
   for ( source = IPC_INTERRUPT_SOURCE_0; source < IPC_INTERRUPT_SOURCE_MAX; source++ )
   {
      if ( status & ( IPC_INTERRUPT_STATUS_ENABLED << source ))
      {
         chal_ipc_int_clr ( ipcHandle, source );
      }
   }
   //FIXME!!!!!!!! does not work with GPIO doorbells!!

   /* signal the stack that there is something to pick up */
//   remote_event_pollall(&g_state);

   return IRQ_HANDLED;
}
#endif

/****************************************************************************
*
*   dma_device_handler
*
***************************************************************************/

#if 0
static void dma_device_handler( DMA_Device_t dev, int reason, void *userData )
{
    (void)dev;
    (void)userData;

    struct completion *dmaDone = userData;
    
    if ( reason & DMA_HANDLER_REASON_TRANSFER_COMPLETE )
    {
        complete( dmaDone );
    }
    
    if ( reason != DMA_HANDLER_REASON_TRANSFER_COMPLETE )
    {
        printk( KERN_ERR "%s: called with reason = 0x%x\n", __func__, reason );
    }
}
#endif

/****************************************************************************
*
*   ipc_dma
*
***************************************************************************/

#if 0
static int ipc_dma( void *vcaddr, void *armaddr, int len, void *client, enum dma_data_direction dir )
{
   int rc;
   dma_addr_t vcptr;

#if 0
   printk( "(Bulk) dir=%s vcaddr=0x%x armaddr=0x%x %x len=%u\n",
         (dir == DMA_TO_DEVICE) ? "Tx" : "Rx", (unsigned int)vcaddr,
         (int)virt_to_phys(armaddr),
         (unsigned int)armaddr, len );
#endif

   /* Convert to physical address */
   vcptr = (dma_addr_t)(((unsigned int)vcaddr) & 0x7FFFFFFF);

   init_completion( &gDmaDone );

   dmaHndl = sdma_request_channel( DMA_DEVICE_MEM_TO_MEM );
   if( dmaHndl < 0 )
   {
      printk( "%s: sdma_request_channel failed\n", __func__ );
      vcos_assert( 0 );
   }

   rc = sdma_set_device_handler( DMA_DEVICE_MEM_TO_MEM, dma_device_handler, &gDmaDone );
   if( rc < 0 )
   {
      printk( "%s: sdma_set_device_handler failed\n", __func__ );
      vcos_assert( 0 );
   }

   INIT_COMPLETION( gDmaDone );   /* Mark as incomplete */

   /* Double check the memory is supported by dma_mmap */
   rc = dma_mmap_dma_is_supported( armaddr );
   if( !rc )
   {
      printk( "%s: Buffer not supported buf=0x%lx\n", __func__, (unsigned long)armaddr );
      vcos_assert(0);
   }

   /* Set user task if necessary */
   dma_mmap_set_user_task( &gVchiqDmaMmap, client );

   /* Map memory */
   rc = dma_mmap_map( &gVchiqDmaMmap, armaddr, len, dir );
   if( rc < 0 )
   {
      printk( "%s: dma_mmap_map FAILED buf=0x%lx\n",
            __func__, (unsigned long)armaddr );
      vcos_assert(0);
   }

   rc = sdma_map_create_descriptor_ring( dmaHndl, &gVchiqDmaMmap, vcptr, DMA_UPDATE_MODE_INC );
   if( rc < 0 )
   {
      printk( "%s: sdma_map_create_descriptor_ring FAILED rc=%u\n",
            __func__, rc );
      vcos_assert(0);
   }
   if ((rc = sdma_start_transfer( dmaHndl )) != 0 )
   {
      printk( KERN_ERR "%s: DMA failed %d\n", __func__, rc );
      vcos_assert( 0 );
   }

   wait_for_completion( &gDmaDone );

   if( dmaHndl >= 0 )
   {
      sdma_free_channel( dmaHndl );
      dmaHndl = SDMA_INVALID_HANDLE;
   }

   dma_mmap_unmap( &gVchiqDmaMmap, (dir == DMA_FROM_DEVICE) ? DMA_MMAP_DIRTIED : DMA_MMAP_CLEAN );

   return rc;
}
#endif

/****************************************************************************
*
*   print_channel
*
***************************************************************************/
#if 0
static int print_channel( VCHIQ_CHANNEL_T *pC, char *Buffer, uint32_t Bytes )
{
   uint32_t Offset;
   int i, j;

   if( pC == NULL )
   {
      Offset = Trace_SNPrintf(
            Buffer,
            Bytes,
            "  Not opened.\n" );
      return Offset;
   }
 
   Offset = Trace_SNPrintf(
         Buffer,
         Bytes,
         "    initialised=%u\n", pC->initialised );
   Offset += Trace_SNPrintf(
         Buffer + Offset,
         Bytes - Offset,
         "\n    CTRL Messages\n" );
   Offset += Trace_SNPrintf(
         Buffer + Offset,
         Bytes - Offset,
         "      remove=%u process=%u insert=%u\n", pC->ctrl.remove, pC->ctrl.process, pC->ctrl.insert );
   Offset += Trace_SNPrintf(
         Buffer + Offset,
         Bytes - Offset,
         "\n    BULK Messages\n" );
   Offset += Trace_SNPrintf(
         Buffer + Offset,
         Bytes - Offset,
         "      remove=%u process=%u insert=%u\n", pC->bulk.remove, pC->bulk.process, pC->bulk.insert );

   for( i = 0; i < VCHIQ_NUM_CURRENT_BULKS; i++ )
   {
      Offset += Trace_SNPrintf(
            Buffer + Offset,
            Bytes - Offset,
            "      bulks[%u]: dstport=%u data=0x%x size=%u userdata=0x%x\n", i, pC->bulk.bulks[i].dstport, (unsigned int)pC->bulk.bulks[i].data, pC->bulk.bulks[i].size, (unsigned int)pC->bulk.bulks[i].userdata );
   }

   Offset += Trace_SNPrintf(
         Buffer + Offset,
         Bytes - Offset,
         "\n    Services\n" );

   for( i = 0; i < VCHIQ_MAX_SERVICES; i++ )
   {
      VCHIQ_SERVICE_T * Service = &pC->services[i];
      if (Service->fourcc != VCHIQ_FOURCC_INVALID)
      {
         const char * FourCC = (const char *) &Service->fourcc;
         Offset += Trace_SNPrintf(
               Buffer + Offset,
               Bytes - Offset,
               "    %c%c%c%c:",
               FourCC[3], FourCC[2], FourCC[1], FourCC[0] ); // Hacky, but someone didn't understand little endian ..
         Offset += Trace_SNPrintf(
               Buffer + Offset,
               Bytes - Offset,
               "      remove=%u process=%u insert=%u\n", Service->remove, Service->process, Service->insert );

         for( j = 0; j < VCHIQ_NUM_SERVICE_BULKS; j++ )
         {
            Offset += Trace_SNPrintf(
                  Buffer + Offset,
                  Bytes - Offset,
                  "      bulks[%u]: dstport=%u data=0x%x size=%u userdata=0x%x\n", i, Service->bulks[i].dstport, (unsigned int)Service->bulks[i].data, Service->bulks[i].size, (unsigned int)Service->bulks[i].userdata );
         }
      }
   }

   return Offset;
}
#endif

/****************************************************************************
*
*   Vchiq_ChannelRead
*
***************************************************************************/

#if 0
uint32_t Vchiq_ChannelRead( char* Buffer, uint32_t Bytes )
{
   uint32_t Offset = Trace_SNPrintf( Buffer, Bytes, "VCHI-Q Channels\n" );
   /* ARM->VC Channel */
   Offset += Trace_SNPrintf(
         Buffer + Offset,
         Bytes - Offset,
         "  ARM->VC Channel (local)\n" );
   Offset += print_channel( g_state.local, Buffer + Offset, Bytes - Offset );

   /* VC->ARM Channel */
   Offset += Trace_SNPrintf(
         Buffer + Offset,
         Bytes - Offset,
         "  VC->ARM Channel (remote)\n" );
   Offset += print_channel( g_state.remote, Buffer + Offset, Bytes - Offset );

   return Offset;
}
#endif

/****************************************************************************
*
*   vchiq_copy_bulk_from_host
*
***************************************************************************/

#if 0
static void vchiq_memdrv_hana_copy_bulk_from_host( VCHIQ_MEMDRV_DRIVER_DATA_T driverData, void *dst, const void *src, size_t numBytes )
{
    void *armAddr = (void *)src;
    void *vcAddr = dst;

    ipc_dma( vcAddr, armAddr, numBytes, NULL, DMA_TO_DEVICE );
}
#endif

/****************************************************************************
*
*   vchiq_copy_bulk_to_host
*
***************************************************************************/

#if 0
static void vchiq_memdrv_hana_copy_bulk_to_host( VCHIQ_MEMDRV_DRIVER_DATA_T driverData, void *dst, const void *src, size_t numBytes )
{
    void *vcAddr = (void *)src;
    void *armAddr = dst;

    ipc_dma( vcAddr, armAddr, numBytes, NULL, DMA_FROM_DEVICE );
}
#endif

/****************************************************************************
*
*   vchiq_copy_from_user
*
***************************************************************************/

#if 0
static int vchiq_memdrv_hana_copy_from_user( VCHIQ_MEMDRV_DRIVER_DATA_T driverData, void *dst, const void *src, size_t numBytes )
{
    /* we are running service in kernel mode, so just do a memcpy */
    memcpy( dst, src, numBytes );
    return VCOS_SUCCESS;
}
#endif

/****************************************************************************
*
*   vchiq_ring_doorbell
*
***************************************************************************/

#if 0
static void vchiq_memdrv_hana_ring_doorbell( VCHIQ_MEMDRV_DRIVER_DATA_T driverData )
{
    /* trigger vc interrupt */
    chal_ipc_int_vcset( ipcHandle, IPC_INTERRUPT_SOURCE_0 );
}
#endif

/****************************************************************************
*
* vchiq_memdrv_driver
*
*   This structure describes the platform specific functions which are used
*   by the rest of the vchiq stack.
*
***************************************************************************/

#if 0
static VCHIQ_MEMDRV_DRIVER_T vchiq_memdrv_hana_driver =
{
    .ring_doorbell          = vchiq_memdrv_hana_ring_doorbell,
    .copy_from_user         = vchiq_memdrv_hana_copy_from_user,
    .copy_bulk_from_host    = vchiq_memdrv_hana_copy_bulk_from_host,
    .copy_bulk_to_host      = vchiq_memdrv_hana_copy_bulk_to_host,
}
#endif

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
#if 0
    int rc;
    int idx;
#endif

    VCHIQ_PLATFORM_DATA_MEMDRV_HANA_T *platform_data = pdev->dev.platform_data;
    const char *name = platform_data->memdrv.common.instance_name;
//    VCHIQ_KERNEL_INSTANCE_T vchiq_instance;
//    VCHIQ_MEMDRV_DRIVER_DATA_T driverData;

    printk( KERN_INFO "vchiq_memdrv_hana: Probing '%s' ...\n", name );
    printk( KERN_INFO "vchiq_memdrv_hana: Shared Memory: 0x%08x\n",
            (uint32_t)platform_data->memdrv.sharedMemVirt );

    platform_set_drvdata( pdev, NULL );

    /*
     * Initialize the mailboxes to zero. The videocore also does this, but it may or 
     * may not have gotten around to doing this before we run. We set them to zero 
     * to ensure that we don't read false data from a previous boot.
     */

    SHARED_MAILBOX(IPC_MAILBOX_ID_0) = 0;
    SHARED_MAILBOX(IPC_MAILBOX_ID_1) = 0;
    SHARED_MAILBOX(IPC_MAILBOX_ID_2) = 0;
    SHARED_MAILBOX(IPC_MAILBOX_ID_3) = 0;

#if 0
    if (( driverData = kzalloc( sizeof( driverData ), GFP_KERNEL )) == NULL )
    {
        printk( KERN_ERR "%s: Unable to allocate %d bytes for hana instance data\n",
                __func__, sizeof( driverData ));
        rc = -ENOMEM;
        goto err;
    }
    driverData->platform_data = platform_data;

    driverData->ipcHandle = chal_ipc_config( NULL );

    chal_icd_set_security (0, platform_data->ipcIrq, eINT_STATE_SECURE );
    for ( idx = 0; idx < IPC_INTERRUPT_SOURCE_MAX; idx++ )
    {
        chal_ipc_int_secmode( driverData->ipcHandle, idx, IPC_INTERRUPT_MODE_OPEN );
    }

    dma_mmap_init_map( &driverData->dma_mmap );

#if 0
    if (( rc = vchiq_memdrv_create_instance( &platform_data->memdrv,
                                             &vchiq_memdrv_hana_driver,
                                             driverData,
                                             &driverData->memdrv_instance )) != 0 )
    {
        printk( KERN_ERR "vchiq_memdrv_hana: '%s' not detected\n", name );
        goto err;
    }
#endif

    platform_set_drvdata( pdev, driverData );

    /*
     * Clear any interrupt sources.
     */

    for ( idx = 0; idx < IPC_INTERRUPT_SOURCE_MAX; idx++ )
    {
        chal_ipc_int_clr( driverData->ipcHandle, idx );
    }

    /*
     * Now that everything has been setup and initialized, register an IRQ handler.
     */

    if (( rc = request_irq( platform_data->ipcIrq, vchiq_doorbell_irq, IRQF_DISABLED, "IPC driver", driverData )) != 0 )
    {
        printk( KERN_ERR "%s: failed to register irq=%d rc=%d\n", __func__, platform_data->ipcIrq, rc );
        goto err;
    }

    return 0;

err:
    
    if ( driverData != NULL )
    {
#if 0
        vchiq_memdrv_free_instance( driverData->memdrv_instance );
#endif
        kfree( driverData );
    }
    return rc;
#else //TODO

    /*
     * This needs to be moved up into vchiq_memdrv.c
    */

    if ( vchiq_userdrv_create_instance( &platform_data->memdrv.common) != VCHIQ_SUCCESS )
    {
        printk( KERN_ERR "vchiq_memdrv_hana: Failed to create vchiq instance for '%s'\n",
                name );

        return -ENOMEM;
    }
//    platform_set_drvdata( pdev, vchiq_instance );

    return 0;
#endif
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
#if 0
    VCHIQ_PLATFORM_DATA_MEMDRV_HANA_T  *platform_data = pdev->dev.platform_data;
    VCHIQ_MEMDRV_DRIVER_DATA_T  driverData;

    driverData = platform_get_drvdata( pdev );
    if ( driverData != NULL )
    {
        vchiq_memdrv_free_instance( driverData->memdrv_instance );
        kfree( driverData );
    }
    free_irq( platform_data->ipcIrq );
#endif
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
#if 0
    if ( vc_boot_mode_skip() )
    {
        return -ENODEV;
    }
#endif

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
#if 0
    if ( vc_boot_mode_skip() )
    {
        return;
    }
#endif
    
    platform_driver_unregister( &vchiq_memdrv_hana_interface_driver );
}

module_init( vchiq_memdrv_hana_interface_init );
module_exit( vchiq_memdrv_hana_interface_exit );

MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("VCHIQ Shared Memory Interface Driver");
MODULE_LICENSE("GPL");

