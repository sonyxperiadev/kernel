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
#include <linux/slab.h>

#include "vchiq_core.h"
#include "vchiq_if.h"
#include "vchiq_memdrv.h"
#include "interface/vcos/vcos_logging.h"

/* ---- Public Variables ------------------------------------------------- */

extern VCOS_LOG_CAT_T vchiq_core_log_category;
#define  VCOS_LOG_CATEGORY (&vchiq_core_log_category)

/* ---- Private Constants and Types -------------------------------------- */

struct vchiq_instance_struct {

    int             connected;
    VCHIQ_STATE_T  *state;
};

/* ---- Private Variables ------------------------------------------------ */

static VCHIQ_STATE_T    g_state;

/* ---- Private Function Prototypes -------------------------------------- */

/* ---- Functions  ------------------------------------------------------- */

/****************************************************************************
*
*   vchiq_initialise
*
***************************************************************************/

VCHIQ_STATUS_T vchiq_initialise( VCHIQ_INSTANCE_T *instanceOut )
{
    VCHIQ_INSTANCE_T    instance;

    if ( g_state.remote == NULL )
    {
        printk( "%s: g_state.remote = 0x%p\n", __func__, g_state.remote );
    }

    if ( !g_state.remote->initialised )
    {
        printk( KERN_ERR "%s: videocore not initialized\n", __func__ );
        return VCHIQ_ERROR;
    }

    instance = kzalloc( sizeof(*instance), GFP_KERNEL );
    if( !instance )
    {
        printk( KERN_ERR "%s: error allocating vchiq instance\n", __func__ );
        return VCHIQ_ERROR;
    }

    instance->connected = 0;
    instance->state = &g_state;

    *instanceOut = instance;

    vcos_log_trace( "%s: returning instance %p", __func__, instance );

    return VCHIQ_SUCCESS;
}

/****************************************************************************
*
*   vchiq_is_connected
*
***************************************************************************/

int vchiq_is_connected(VCHIQ_INSTANCE_T instance)
{
    return instance->connected;
}

/****************************************************************************
*
*   vchiq_connect
*
***************************************************************************/

VCHIQ_STATUS_T vchiq_connect(VCHIQ_INSTANCE_T instance)
{
    VCHIQ_STATUS_T  status;
    VCHIQ_STATE_T  *state = instance->state;

    vcos_log_trace( "%s(%p) called", __func__, instance );

    if (vcos_mutex_lock(&state->mutex) != VCOS_SUCCESS) {
        vcos_log_trace( "%s: call to vcos_mutex_lock failed", __func__ );
        return VCHIQ_RETRY;
    }
    status = vchiq_connect_internal(state, instance);
    vcos_mutex_unlock(&state->mutex);

    if (status == VCHIQ_SUCCESS)
        instance->connected = 1;

    vcos_log_trace( "%s: returning %d", __func__, status );
    return status;
}

/****************************************************************************
*
*   vchiq_add_service
*
***************************************************************************/

VCHIQ_STATUS_T vchiq_add_service(
    VCHIQ_INSTANCE_T        instance,
    int                     fourcc,
    VCHIQ_CALLBACK_T        callback,
    void                   *userdata,
    VCHIQ_SERVICE_HANDLE_T *pservice)
{
    VCHIQ_SERVICE_T *service;
    int srvstate;

    srvstate = vchiq_is_connected( instance )
             ? VCHIQ_SRVSTATE_LISTENING
             : VCHIQ_SRVSTATE_HIDDEN;

    vcos_mutex_lock(&instance->state->mutex);

    service = vchiq_add_service_internal(instance->state,
                                         fourcc,
                                         callback,
                                         userdata,
                                         srvstate,
                                         instance);

    vcos_mutex_unlock(&instance->state->mutex);

    if ( service == NULL )
    {
        *pservice = NULL;
        return VCHIQ_ERROR;
    }

    *pservice = &service->base;
    return VCHIQ_SUCCESS;
}
EXPORT_SYMBOL( vchiq_add_service );

/****************************************************************************
*
*   vchiq_open_service
*
***************************************************************************/

VCHIQ_STATUS_T vchiq_open_service(
    VCHIQ_INSTANCE_T        instance,
    int                     fourcc,
    VCHIQ_CALLBACK_T        callback,
    void                   *userdata,
    VCHIQ_SERVICE_HANDLE_T *pservice)
{
    VCHIQ_STATUS_T   status;
    VCHIQ_SERVICE_T *service;

    *pservice = NULL;

    if ( !vchiq_is_connected(instance ))
    {
        return VCHIQ_ERROR;
    }

    vcos_mutex_lock(&instance->state->mutex);

    service = vchiq_add_service_internal(instance->state,
                                         fourcc,
                                         callback,
                                         userdata,
                                         VCHIQ_SRVSTATE_OPENING,
                                         instance);

    vcos_mutex_unlock(&instance->state->mutex);

    if ( service == NULL )
    {
        return VCHIQ_ERROR;
    }

    status = vchiq_open_service_internal(service);
    if ( status != VCHIQ_SUCCESS )
    {
        vchiq_remove_service(&service->base);
        return status;
    }

    *pservice = &service->base;
    return VCHIQ_SUCCESS;
}
EXPORT_SYMBOL( vchiq_open_service );


/****************************************************************************
*
*   vchiq_terminate_service
*
***************************************************************************/

void vchiq_terminate_service(VCHIQ_SERVICE_HANDLE_T service)
{
    vchiq_terminate_service_internal((VCHIQ_SERVICE_T *)service);
}

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 *%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 *%%
 *%%    Stuff from this point down really belongs in vchiq_memdrv_hana.c
 *%%
 *%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 *%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

#include <linux/dma-mapping.h>
#include <mach/sdma.h>
#include <mach/dma_mmap.h>
#include <mach/aram_layout.h>
#include <asm/gpio.h> // for GPIO

#include <chal/chal_ipc.h>
#include <mach/irqs.h>
#include <chal/chal_icd.h>

#include "vcinclude/bigisland_chip/ipc_shared_mem.h"

static CHAL_IPC_HANDLE  ipcHandle;

static DMA_MMAP_CFG_T    gVchiqDmaMmap;
static SDMA_Handle_t     dmaHndl;
static struct completion gDmaDone;

#if ( BCMHANA_ARAM_VC_OFFSET != IPC_SHARED_MEM_VC_OFFSET )
#error BAMHANA ARAM VC OFFSET MUST be the same as IPC SHARED MEM VC OFFSET
#endif

#define IPC_SHARED_MEM_VC_BASE_VIRT     (KONA_INT_SRAM_BASE + IPC_SHARED_MEM_VC_OFFSET)
#define IPC_SHARED_MEM_VC_BASE_PHYS     (INT_SRAM_BASE + IPC_SHARED_MEM_VC_OFFSET)

#define IPC_SHARED_CHANNEL_VC_VIRT    (IPC_SHARED_MEM_VC_BASE_VIRT + IPC_SHARED_MEM_CHANNEL_VC_OFFSET)
#define IPC_SHARED_CHANNEL_VC_PHYS    (IPC_SHARED_MEM_VC_BASE_PHYS + IPC_SHARED_MEM_CHANNEL_VC_OFFSET)
#define IPC_SHARED_CHANNEL_ARM_VIRT   (IPC_SHARED_MEM_VC_BASE_VIRT + IPC_SHARED_MEM_CHANNEL_ARM_OFFSET)
#define IPC_SHARED_CHANNEL_ARM_PHYS   (IPC_SHARED_MEM_VC_BASE_PHYS + IPC_SHARED_MEM_CHANNEL_ARM_OFFSET)

/*
 * On Big-Island there are restrictions about what we can DMA. Specifically, we
 * require that the data be 8-byte aligned, and be a multiple of 4 in length.
 *
 * If we're asked to deal with data which doesn't fit this criteria, then we
 * allocate a temporary buffer and copy the data into the temporary buffer.
 *
 * Normally, the memhandle is used to store the thread id of the process issuing
 * the request. We use the special value MEM_HANDLE_ALLOCATED_TEMP_BUF to indicate
 * that we allocated a temporary buffer which we need to free later.
 */

#define MEM_HANDLE_ALLOCATED_TEMP_BUF    1

/****************************************************************************
*
*   vchiq_memdrv_get_info
*
***************************************************************************/

void vchiq_get_shared_mem_info( VCHIQ_SHARED_MEM_INFO_T *info )
{
    info->armSharedMemPhys = IPC_SHARED_CHANNEL_ARM_PHYS;
    info->armSharedMemVirt = (void *)IPC_SHARED_CHANNEL_ARM_VIRT;
    info->armSharedMemSize = IPC_SHARED_MEM_CHANNEL_ARM_SIZE;

    info->vcSharedMemPhys = IPC_SHARED_CHANNEL_VC_PHYS;
    info->vcSharedMemVirt = (void *)IPC_SHARED_CHANNEL_VC_VIRT;
    info->vcSharedMemSize = IPC_SHARED_MEM_CHANNEL_VC_SIZE;
}


/****************************************************************************
*
*   Function which translates a client pointer into a task_struct *
*
***************************************************************************/

static inline struct task_struct *ClientTask( VCHI_MEM_HANDLE_T client )
{
    if ( client == MEM_HANDLE_ALLOCATED_TEMP_BUF )
    {
        client = 0;
    }
    return (struct task_struct *)client;
}

/****************************************************************************
*
*   Function which determines the pid of a client.
*
***************************************************************************/

static inline int ClientPid( VCHI_MEM_HANDLE_T client )
{
    if (( client == VCHI_MEM_HANDLE_INVALID ) || ( client == MEM_HANDLE_ALLOCATED_TEMP_BUF ))
    {
        return -1;
    }
    return ClientTask(client)->pid;
}

/****************************************************************************
*
*   Function which determines the task name of a client.
*
***************************************************************************/

static inline const char *ClientName( VCHI_MEM_HANDLE_T client )
{
    if (( client == VCHI_MEM_HANDLE_INVALID ) || ( client == MEM_HANDLE_ALLOCATED_TEMP_BUF ))
    {
        return "???";
    }
    return ClientTask(client)->comm;
}

/****************************************************************************
*
*   Function which translates a dma direction into a printable string.
*
***************************************************************************/

static inline const char *dma_data_direction_as_str( enum dma_data_direction dir )
{
    switch ( dir )
    {
        case DMA_BIDIRECTIONAL:     return "BIDIRECTIONAL";
        case DMA_TO_DEVICE:         return "TO_DEVICE";
        case DMA_FROM_DEVICE:       return "FROM_DEVICE";
        case DMA_NONE:              return "NONE";
    }
    return "???";
}

/****************************************************************************
*
*   dma_device_handler
*
***************************************************************************/

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

/****************************************************************************
*
*   vchiq_prepare_bulk
*
***************************************************************************/

VCHIQ_STATUS_T vchiq_prepare_bulk(VCHIQ_BULK_T *bulk, unsigned int dstport,
                                  VCHI_MEM_HANDLE_T memhandle, void *offset,
                                  int size, void *userdata,
                                  VCHIQ_BULK_MODE_T mode, int dir)
{
    bulk->dstport = dstport;
    bulk->handle = memhandle;
    bulk->size = size;
    //bulk->actual = 0;
    bulk->userdata = userdata;
    (void)mode; //bulk->mode = mode;
    bulk->data = offset;

    (void)dir;

    return VCHIQ_SUCCESS;
}

/****************************************************************************
*
*   vchiq_complete_bulk
*
***************************************************************************/

void vchiq_complete_bulk(VCHIQ_BULK_T *bulk)
{
    (void)bulk;
}

/****************************************************************************
*
*   ipc_dma
*
***************************************************************************/

static int ipc_dma( void *vcaddr, void *armaddr, int len, VCHI_MEM_HANDLE_T client, enum dma_data_direction dir )
{
   int rc;
   dma_addr_t   vcPhysOffset;
   dma_addr_t   vcPhysAddr;
   DMA_Device_t dmaDev;

#if 0
   printk( "(Bulk) dir=%s vcaddr=0x%x armaddr=0x%x len=%u\n",
         (dir == DMA_TO_DEVICE) ? "Tx" : "Rx", (unsigned int)vcaddr,
         (unsigned int)armaddr, len );
#endif

   /* Convert the videocore pointer to a videocore physical offset */
   vcPhysOffset = (dma_addr_t)(((unsigned long)vcaddr) & 0x3FFFFFFFuL);

   /* Convert the videocore physical address into an ARM physical address */
   vcPhysAddr = VC_EMI + vcPhysOffset;

   if ((( (unsigned long)vcaddr  & 7uL ) == 0 )
   &&  (( (unsigned long)armaddr & 7uL ) == 0 )
   &&  (( len & 3 ) == 0 ))
   {
       init_completion( &gDmaDone );

       dmaDev = DMA_DEVICE_MEM_TO_MEM;
       if (( len & 7 ) != 0 )
       {
           /* If the length isn't a multiple of 8, then we need to use 32-bit transactions */
           dmaDev = DMA_DEVICE_MEM_TO_MEM_32;
       }
   }
   else
   {
#if 1
       if (( (unsigned long)vcaddr & 7uL ) != 0 )
       {
           printk( KERN_ERR "%s: vcaddr 0x%p isn't a multiple of 8\n", __func__, vcaddr );
       }
       if (( (unsigned long)armaddr & 7uL ) != 0 )
       {
           printk( KERN_ERR "%s: armaddr 0x%p isn't a multiple of 8\n", __func__, armaddr );
       }
       if (( len & 3 ) != 0 )
       {
           printk( KERN_ERR "%s: len %d isn't a multiple of 4\n", __func__, len );
       }
#endif

       dmaDev = DMA_DEVICE_NONE;
   }

   if ( dmaDev != DMA_DEVICE_NONE )
   {
       dmaHndl = sdma_request_channel( dmaDev );
       if( dmaHndl < 0 )
       {
          printk( "%s: sdma_request_channel failed\n", __func__ );
          vcos_assert( 0 );
       }

       rc = sdma_set_device_handler( dmaDev, dma_device_handler, &gDmaDone );
       if( rc < 0 )
       {
          printk( "%s: sdma_set_device_handler failed\n", __func__ );
          vcos_assert( 0 );
       }

       INIT_COMPLETION( gDmaDone );   /* Mark as incomplete */
   }

   /* Double check the memory is supported by dma_mmap */
   rc = dma_mmap_dma_is_supported( armaddr );
   if( !rc )
   {
      printk( "%s: Buffer not supported buf=0x%lx\n", __func__, (unsigned long)armaddr );
      vcos_assert(0);
   }

   /* Set user task if necessary */
   dma_mmap_set_user_task( &gVchiqDmaMmap, ClientTask(client));

   /* Map memory */
   rc = dma_mmap_map( &gVchiqDmaMmap, armaddr, len, dir );
   if( rc < 0 )
   {
      printk( KERN_ERR "%s: dma_mmap_map FAILED buf=0x%lx len=0x%lx pid=%d name='%s'\n",
            __func__, (unsigned long)armaddr, (unsigned long)len, ClientPid(client), ClientName(client));
      vcos_assert(0);
   }

   if ( dmaDev == DMA_DEVICE_NONE )
   {
       dma_mmap_memcpy( &gVchiqDmaMmap, (void *)HW_IO_PHYS_TO_VIRT( vcPhysAddr ));
   }
   else
   {
       rc = sdma_map_create_descriptor_ring( dmaHndl, &gVchiqDmaMmap, vcPhysAddr, DMA_UPDATE_MODE_INC );
       if( rc < 0 )
       {
          printk( "%s: sdma_map_create_descriptor_ring FAILED rc=%u\n",
                __func__, rc );
          printk( KERN_ERR "%s: vcaddr=0x%p armaddr=0x%p len=%d dir=%s client=%d-%s\n",
                  __func__, vcaddr, armaddr, len, dma_data_direction_as_str( dir ),
                  ClientPid(client), ClientName(client));
       }
       if ((rc = sdma_start_transfer( dmaHndl )) != 0 )
       {
          printk( KERN_ERR "%s: DMA failed %d\n", __func__, rc );
          vcos_assert( 0 );
       }

       wait_for_completion( &gDmaDone );

       if ( dmaHndl >= 0 )
       {
          sdma_free_channel( dmaHndl );
          dmaHndl = SDMA_INVALID_HANDLE;
       }
   }


   dma_mmap_unmap( &gVchiqDmaMmap, (dir == DMA_FROM_DEVICE) ? DMA_MMAP_DIRTIED : DMA_MMAP_CLEAN );

   return rc;
}

void vchiq_copy_bulk_from_host(void *dst, const void *src, int size, VCHI_MEM_HANDLE_T handle)
{
   /* dst is VC addr and src is ARM addr */
   ipc_dma( dst, (void *)src, size, handle, DMA_TO_DEVICE );
}

void vchiq_copy_bulk_to_host(void *dst, const void *src, int size, VCHI_MEM_HANDLE_T handle)
{
   /* dst is ARM addr and src is VC addr */
   ipc_dma( (void *)src, dst, size, handle, DMA_FROM_DEVICE );
}

int vchiq_copy_from_user(void *dst, const void *src, int size)
{
    if ( (uint32_t)src < TASK_SIZE)
    {
        return copy_from_user(dst, src, size);
    }
    else
    {
        memcpy( dst, src, size );
        return 0;
    }
}

void vchiq_ring_doorbell(void)
{
   /* trigger vc interrupt */
   chal_ipc_int_vcset( ipcHandle, IPC_INTERRUPT_SOURCE_0 ); //FIXMEs
}

/*
 * Due to the limitations at the RTL level, there are some GPIO pins that
 * cannot be muxed to the videocore. As a workaround, we are using a combination
 * of the IPC doorbells and shared memory to communicate between the host and
 * videocore to control the pins that are muxed to the host.
 *
 * For the time being, we are piggybacking off of the sharedmem driver because
 * it is the fastest way. The long term solution would see the code that deals
 * with the GPIO moved to a separate service/driver.
 *
 * Operation details:
 *    - syncing:
 *         Before the videocore can request GPIO operations from the host, the
 *         videocore needs to make sure the host is up first. When the videocore
 *         boots up, it rings an IPC doorbell. If the host is already up at
 *         that moment, it will ring the same IPC doorbell back. Upon receiving
 *         the doorbell, the videocore will know the host is up and ready.
 *         Before this time, the videocore will return failure on any host GPIO
 *         requests.
 *
 *         To cover the scenario where the videocore comes up before the host,
 *         the host will automatically ring the IPC doorbell to notify the
 *         videocore.
 *
 *    - setting/clearing:
 *         1. Videocore writes GPIO pin to be set/clear into the shared memory
 *            at GPIO_MAILBOX_WRITE. If performing a set, the value is also
 *            bitwise OR'd with GPIO_MAILBOX_WRITE_SET.
 *         2. Videocore rings the IPC doorbell and waits for a reply.
 *         3. Host answers the doorbell and sets/clears the GPIO pin.
 *         4. Host rings the same IPC doorbell to notify the videocore.
 *         5. Videocore gets the doorbell and returns to the user.
 *
 *    - reading:
 *         1. Videocore writes GPIO pin to be read into the shared memory at
 *            GPIO_MAILBOX_READ.
 *         2. Videocore rings the IPC doorbell and waits for a reply.
 *         3. Host answers the doorbell and reads the value of the GPIO and
 *            stores it back into the shared memory at GPIO_MAILBOX_READ.
 *         4. Host rings the same IPC doorbell to notify the videocore.
 *         5. Videocore gets the doorbell and reads out the value of the GPIO
 *            from the shared memory and returns it to the user.
 */
static void service_gpio( uint32_t irq_status )
{
#define GPIO_MAILBOX_WRITE            (*(volatile uint32_t *)(KONA_INT_SRAM_BASE + IPC_SHARED_MEM_CHANNEL_ARM_OFFSET + IPC_SHARED_MEM_GPIO_WRITE_OFFSET))
#define GPIO_MAILBOX_READ             (*(volatile uint32_t *)(KONA_INT_SRAM_BASE + IPC_SHARED_MEM_CHANNEL_ARM_OFFSET + IPC_SHARED_MEM_GPIO_READ_OFFSET))

#define GPIO_MAILBOX_WRITE_SET        (0x80000000)
#define GPIO_MAILBOX_WRITE_PIN_MASK   (0x7FFFFFFF)

   if ( irq_status & ( IPC_INTERRUPT_STATUS_ENABLED << IPC_INTERRUPT_SOURCE_2 ))
   {
      uint32_t reg = GPIO_MAILBOX_WRITE;

      if ( reg & GPIO_MAILBOX_WRITE_SET )
      {
//         printk( KERN_ERR "%s: setting GPIO %d\n", __func__,
//                 reg & GPIO_MAILBOX_WRITE_PIN_MASK );

         /* GPIO clear */
         gpio_set_value( reg & GPIO_MAILBOX_WRITE_PIN_MASK , 1 );
      }
      else
      {
//         printk( KERN_ERR "%s: clearing GPIO %d\n", __func__,
//                 reg & GPIO_MAILBOX_WRITE_PIN_MASK );

         /* GPIO set */
         gpio_set_value( reg & GPIO_MAILBOX_WRITE_PIN_MASK , 0 );
      }

      /* Notify videocore that GPIO has been cleared */
      chal_ipc_int_vcset( ipcHandle, IPC_INTERRUPT_SOURCE_2 );

      irq_status &= ~( IPC_INTERRUPT_STATUS_ENABLED << IPC_INTERRUPT_SOURCE_2 );
   }

   if ( irq_status & ( IPC_INTERRUPT_STATUS_ENABLED << IPC_INTERRUPT_SOURCE_3 ))
   {
//      printk( KERN_ERR "%s: reading GPIO %d\n", __func__, GPIO_MAILBOX_READ);

      /* GPIO set */
      GPIO_MAILBOX_READ = gpio_get_value( GPIO_MAILBOX_READ );

      /* Notify videocore that GPIO has been set */
      chal_ipc_int_vcset( ipcHandle, IPC_INTERRUPT_SOURCE_3 );

      irq_status &= ~( IPC_INTERRUPT_STATUS_ENABLED << IPC_INTERRUPT_SOURCE_3 );
   }

   if ( irq_status & ( IPC_INTERRUPT_STATUS_ENABLED << IPC_INTERRUPT_SOURCE_4 ))
   {
//      printk( KERN_ERR "%s: videocore is ready\n", __func__ );

      /* Reply back to the videocore to tell them we are ready */
      chal_ipc_int_vcset( ipcHandle, IPC_INTERRUPT_SOURCE_4 );

      irq_status &= ~( IPC_INTERRUPT_STATUS_ENABLED << IPC_INTERRUPT_SOURCE_4 );
   }
}

static irqreturn_t vchiq_doorbell_irq(int irq, void *dev_id)
{
   IPC_INTERRUPT_SOURCE source;
   uint32_t             status;
   VCHIQ_STATE_T       *state = dev_id;

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

   if ( status & ( IPC_INTERRUPT_STATUS_ENABLED << IPC_INTERRUPT_SOURCE_0 ))
   {
      /* signal the stack that there is something to pick up */
      remote_event_pollall(state);
   }
   else if (( status & ( IPC_INTERRUPT_STATUS_ENABLED << IPC_INTERRUPT_SOURCE_2 )) ||
            ( status & ( IPC_INTERRUPT_STATUS_ENABLED << IPC_INTERRUPT_SOURCE_3 )) ||
            ( status & ( IPC_INTERRUPT_STATUS_ENABLED << IPC_INTERRUPT_SOURCE_4 )))
   {
      /* this is a GPIO request */
      service_gpio( status );
   }

   return IRQ_HANDLED;
}

/****************************************************************************
*
*   vchiq_memdrv_initialise
*
***************************************************************************/

VCHIQ_STATUS_T vchiq_memdrv_initialise(void)
{
	int err;
	int i;

    printk( "%s: IPC_SHARED_CHANNEL_ARM_VIRT = 0x%lx\n", __func__, IPC_SHARED_CHANNEL_ARM_VIRT );
    printk( "%s: IPC_SHARED_CHANNEL_VC_VIRT  = 0x%lx\n", __func__, IPC_SHARED_CHANNEL_VC_VIRT );
    printk( "%s: sizeof( VCHIQ_CHANNEL_T )   = %d\n", __func__, sizeof( VCHIQ_CHANNEL_T ));
    printk( "%s: VCHIQ_MAX_SERVICES          = %d\n", __func__, VCHIQ_MAX_SERVICES );

    vcos_assert( IPC_SHARED_CHANNEL_VC_VIRT + sizeof( VCHIQ_CHANNEL_T ) <= IPC_SHARED_CHANNEL_ARM_VIRT );

    /* initialize the channel memory, note the vc04 has already started by now
       so the channel memory is expected to be cleared */
	vchiq_init_state( &g_state, (VCHIQ_CHANNEL_T *)IPC_SHARED_CHANNEL_ARM_VIRT,
                      (VCHIQ_CHANNEL_T *)IPC_SHARED_CHANNEL_VC_VIRT );

    ipcHandle = chal_ipc_config( NULL );
    chal_icd_set_security (0, BCM_INT_ID_IPC_OPEN, eINT_STATE_SECURE );
    for ( i = 0; i < IPC_INTERRUPT_SOURCE_MAX; i++ )
    {
        chal_ipc_int_secmode( ipcHandle, i, IPC_INTERRUPT_MODE_OPEN );
    }

    /* clear all interrupts */
    for (i = 0; i < IPC_INTERRUPT_SOURCE_MAX; i++)
    {
        chal_ipc_int_clr(ipcHandle, i);
    }

	//setup_irq(IRQ_ARM_DOORBELL_0, &vchiq_doorbell_irqaction);
    if (( err = request_irq( BCM_INT_ID_IPC_OPEN, vchiq_doorbell_irq,
                            IRQF_DISABLED, "IPC driver", &g_state )) != 0 )
    {
        printk( KERN_ERR "%s: failed to register irq=%d err=%d\n", __func__,
                BCM_INT_ID_IPC_OPEN, err );
        goto out_err;
    }
   else
   {
      /* Tell the videocore we are ready for servicing GPIO requests */
      chal_ipc_int_vcset( ipcHandle, IPC_INTERRUPT_SOURCE_4 );
   }

    /* initialize dma_mmap for use */
    dma_mmap_init_map( &gVchiqDmaMmap );

	return VCHIQ_SUCCESS;

out_err:
	return VCHIQ_ERROR;
}

