/*
 * Copyright (c) 2010-2011 Broadcom Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/pagemap.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <asm/pgtable.h>
#include <asm/io.h>
#include <asm/uaccess.h>

#include <linux/vchiq_platform_data.h>

#include "vchiq_arm.h"
#include "vchiq_bi.h"

#include "vchiq_memdrv.h"

#include "interface/vceb/host/vceb.h"

#include <linux/dma-mapping.h>
#include <mach/sdma.h>
#include <mach/dma_mmap.h>
#include <mach/aram_layout.h>
#include <asm/gpio.h>

#ifdef CONFIG_ARCH_KONA

#include <chal/chal_ipc.h>
#include <mach/irqs.h>
#include <chal/chal_icd.h>
#include <mach/io_map.h>
#define IPC_SHARED_MEM_BASE       KONA_INT_SRAM_BASE

#else

#include <csp/chal_ipc.h>
#include <csp/chal_intid.h>
#include <csp/chal_icd.h>
#define IPC_SHARED_MEM_BASE       MM_IO_BASE_SRAM

#endif

#include <linux/videocore/vc_mem.h>

#define VCOS_LOG_CATEGORY (&vchiq_arm_log_category)

#define IPC_SHARED_MEM_SLOTS_VIRT IPC_SHARED_MEM_SLOTS

#if ( BCMHANA_ARAM_VC_OFFSET != IPC_SHARED_MEM_VC_OFFSET )
#error BAMHANA ARAM VC OFFSET MUST be the same as IPC SHARED MEM VC OFFSET
#endif

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

typedef struct
{
    unsigned int                 instNum;

    const char                  *instance_name;
    const VCHIQ_PLATFORM_DATA_T *platform_data;

    VCOS_CFG_ENTRY_T             instance_cfg_dir;
    VCOS_CFG_ENTRY_T             vchiq_control_cfg_entry;
} VCHIQ_KERNEL_STATE_T;


#define VCHIQ_NUM_VIDEOCORES 1

static VCHIQ_KERNEL_STATE_T    *vchiq_kernel_state[ VCHIQ_NUM_VIDEOCORES ];
static unsigned int             vchiq_num_instances = 0;

/* Global 'vc' proc entry used by all instances */
VCOS_CFG_ENTRY_T         vc_cfg_dir;

static CHAL_IPC_HANDLE   ipcHandle;

static DMA_MMAP_CFG_T    gVchiqDmaMmap;
static SDMA_Handle_t     dmaHndl;
static struct completion gDmaDone;
static VCOS_MUTEX_T      g_dma_mutex;

static VCHIQ_STATE_T    *g_vchiq_state;

static int
ipc_dma( void *vcaddr, void *armaddr, int len, DMA_MMAP_PAGELIST_T *pagelist,
   enum dma_data_direction dir );

static irqreturn_t
vchiq_doorbell_irq(int irq, void *dev_id);

int __init
vchiq_platform_vcos_init(void)
{
   return 0;
}

int __init
vchiq_platform_init(VCHIQ_STATE_T *state)
{
   g_vchiq_state = state;

   vcos_mutex_create(&g_dma_mutex, "dma_mutex");

   return 0;
}

void __exit
vchiq_platform_exit(VCHIQ_STATE_T *state)
{
   vcos_assert(state == g_vchiq_state);
   g_vchiq_state = NULL;
   free_irq(BCM_INT_ID_IPC_OPEN, state);
   vcos_mutex_delete(&g_dma_mutex);
}

void
remote_event_signal(REMOTE_EVENT_T *event)
{
	event->fired = 1;

	/* The test on the next line also ensures the write on the previous line
		has completed */

	if (event->armed) {
		/* trigger vc interrupt */
		dsb();         /* data barrier operation */

      chal_ipc_int_vcset( ipcHandle, IPC_INTERRUPT_SOURCE_0 );
	}
}

int
vchiq_copy_from_user(void *dst, const void *src, int size)
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

VCHIQ_STATUS_T
vchiq_prepare_bulk_data(VCHIQ_BULK_T *bulk,
	VCHI_MEM_HANDLE_T memhandle, void *offset, int size, int dir)
{
	vcos_assert(memhandle == VCHI_MEM_HANDLE_INVALID);

	/* Check the memory is supported by dma_mmap */
	if (!dma_mmap_dma_is_supported(offset))
	{
		vcos_log_error( "%s: buffer at %lx not supported", __func__,
			(unsigned long)offset );
		return VCHIQ_ERROR;
	}

        if (dma_mmap_mem_type(offset) == DMA_MMAP_TYPE_USER)
	{
		DMA_MMAP_PAGELIST_T *pagelist;
		int ret;
		ret = dma_mmap_create_pagelist(
			(char __user *)offset,
			size,
			(dir == VCHIQ_BULK_RECEIVE)
			? DMA_FROM_DEVICE
			: DMA_TO_DEVICE,
			current,
			&pagelist);
		if (ret <= 0)
			return VCHIQ_ERROR;
		bulk->data = offset;
		bulk->handle = (VCHI_MEM_HANDLE_T)pagelist;
	}
	else
	{
		bulk->data = offset;
		bulk->handle = 0;
	}
   
	return VCHIQ_SUCCESS;
}

void
vchiq_complete_bulk(VCHIQ_BULK_T *bulk)
{
	if (bulk->handle != 0)
		dma_mmap_free_pagelist((DMA_MMAP_PAGELIST_T *)bulk->handle);
}

void
vchiq_transfer_bulk(VCHIQ_BULK_T *bulk)
{
	if ((bulk->size == bulk->remote_size) &&
		(ipc_dma( bulk->remote_data, bulk->data, bulk->size, (DMA_MMAP_PAGELIST_T *)bulk->handle,
		(bulk->dir == VCHIQ_BULK_TRANSMIT) ? DMA_TO_DEVICE : DMA_FROM_DEVICE) == 0))
	{
		bulk->actual = bulk->size;
	}
	else
	{
		bulk->actual = VCHIQ_BULK_ACTUAL_ABORTED;
	}
}

void
vchiq_dump_platform_state(void *dump_context)
{
        char buf[80];
        int len;
        len = vcos_snprintf(buf, sizeof(buf),
                "  Platform: BI (ARM master)");
        vchiq_dump(dump_context, buf, len + 1);
}

/****************************************************************************
*
*   vchiq_control_cfg_parse
*
***************************************************************************/

static void vchiq_control_cfg_parse( VCOS_CFG_BUF_T buf, void *data )
{
   VCHIQ_KERNEL_STATE_T    *kernState = data;
   char                    *command;

   command = vcos_cfg_buf_get_str( buf );

   if ( strncmp( "connect", command, strlen( "connect" )) == 0 )
   {
      if ( vchiq_memdrv_initialise() != VCHIQ_SUCCESS )
      {
         vcos_log_error( "%s: failed to initialize vchiq for '%s'",
            __func__, kernState->instance_name );
      }
      else
      {
         vcos_log_warn( "%s: initialized vchiq for '%s'", __func__,
            kernState->instance_name );
      }
   }
   //TODO support "disconnect", "resume", "suspend"
   else
   {
      vcos_log_error( "%s: unknown command '%s'", __func__, command );
   }
}

/****************************************************************************
*
* vchiq_userdrv_create_instance
*
*   The lower level drivers (vchiq_memdrv or vchiq_busdrv) will call this
*   function for each videcoreo that exists. We then register a character
*   driver which is what userspace uses to talk to us.
*
***************************************************************************/

VCHIQ_STATUS_T vchiq_userdrv_create_instance( const VCHIQ_PLATFORM_DATA_T *platform_data )
{
   VCEB_INSTANCE_T       vceb_instance;
   VCHIQ_KERNEL_STATE_T   *kernState;

   vcos_log_warn( "%s: vchiq_num_instances = %d, VCHIQ_NUM_VIDEOCORES = %d",
      __func__, vchiq_num_instances, VCHIQ_NUM_VIDEOCORES );

   if ( vchiq_num_instances >= VCHIQ_NUM_VIDEOCORES )
   {
      vcos_log_error( "%s: already created %d instances", __func__,
         VCHIQ_NUM_VIDEOCORES );

      return VCHIQ_ERROR;
   }

   if ( vceb_get_instance( platform_data->instance_name, &vceb_instance ) != 0 )
   {
      /* No instance registered with vceb, which means the videocore is not
         present */
      return VCHIQ_ERROR;
   }

   /* Allocate some memory */
   kernState = kmalloc( sizeof( *kernState ), GFP_KERNEL );
   if ( kernState == NULL )
   {
      vcos_log_error( "%s: failed to allocate memory", __func__ );

      return VCHIQ_ERROR;
   }

   memset( kernState, 0, sizeof( *kernState ));

   vchiq_kernel_state[ vchiq_num_instances ] = kernState;

   /* Do some bookkeeping */
   kernState->instNum = vchiq_num_instances++;
   kernState->instance_name = platform_data->instance_name;
   kernState->platform_data = platform_data;

   /* Create proc entries */
   if ( !vcos_cfg_is_entry_created( vc_cfg_dir ))
   {
      if ( vcos_cfg_mkdir( &vc_cfg_dir, NULL, "vc" ) != VCOS_SUCCESS )
      {
         vcos_log_error( "%s: failed to create proc entry", __func__ );

         return VCHIQ_ERROR;
      }
   }

   if ( vcos_cfg_mkdir( &kernState->instance_cfg_dir, &vc_cfg_dir,
      kernState->instance_name ) != VCOS_SUCCESS )
   {
      vcos_log_error( "%s: failed to create proc entry", __func__ );

      return VCHIQ_ERROR;
   }

   if ( vcos_cfg_create_entry( &kernState->vchiq_control_cfg_entry,
      &kernState->instance_cfg_dir,
      "vchiq_control",
      NULL,
      vchiq_control_cfg_parse,
      kernState ) != VCOS_SUCCESS )
   {
      vcos_log_error( "%s: failed to create proc entry", __func__ );

      return VCHIQ_ERROR;
   }

   vcos_log_warn( "%s: successfully initialized '%s' videocore",
      __func__, kernState->instance_name );

   return VCHIQ_SUCCESS;
}

EXPORT_SYMBOL( vchiq_userdrv_create_instance );

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
    vcos_unused(dev);
    vcos_unused(userData);

    struct completion *dmaDone = userData;

    if ( reason & DMA_HANDLER_REASON_TRANSFER_COMPLETE )
    {
        complete( dmaDone );
    }

    if ( reason != DMA_HANDLER_REASON_TRANSFER_COMPLETE )
    {
        vcos_log_error( "%s: called with reason = 0x%x", __func__, reason );
    }
}


/****************************************************************************
*
*   ipc_dma
*
***************************************************************************/

static int
ipc_dma( void *vcaddr, void *armaddr, int len, DMA_MMAP_PAGELIST_T *pagelist, enum dma_data_direction dir )
{
   int rc;
   dma_addr_t vcAddrOffset;
   dma_addr_t vcPhysAddr;
   DMA_Device_t dmaDev;

   vcos_mutex_lock(&g_dma_mutex);

   vcos_log_trace( "(Bulk) dir=%s vcaddr=0x%x armaddr=0x%x len=%u",
         (dir == DMA_TO_DEVICE) ? "Tx" : "Rx", (unsigned int)vcaddr,
         (unsigned int)armaddr, len );

   /* Convert the videocore pointer to a videocore address offset */
   vcAddrOffset = (dma_addr_t)(((unsigned long)vcaddr) & 0x3FFFFFFFuL);

   /* Convert the videocore physical address into an ARM physical address */
   vcPhysAddr = mm_vc_mem_phys_addr + vcAddrOffset;

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
           vcos_log_warn( "%s: vcaddr 0x%p isn't a multiple of 8", __func__, vcaddr );
       }
       if (( (unsigned long)armaddr & 7uL ) != 0 )
       {
           vcos_log_warn( "%s: armaddr 0x%p isn't a multiple of 8", __func__, armaddr );
       }
       if (( len & 3 ) != 0 )
       {
           vcos_log_warn( "%s: len %d isn't a multiple of 4", __func__, len );
       }
#endif

       dmaDev = DMA_DEVICE_NONE;
   }

   if ( dmaDev == DMA_DEVICE_NONE )
   {
      dmaHndl = SDMA_INVALID_HANDLE;
   }
   else
   {
      dmaHndl = sdma_request_channel( dmaDev );
      if( dmaHndl < 0 )
      {
         vcos_log_error( "%s: sdma_request_channel failed", __func__ );
         rc = -1;
         goto failed_sdma_request_channel;
      }

      rc = sdma_set_device_handler( dmaDev, dma_device_handler, &gDmaDone );
      if( rc < 0 )
      {
         vcos_log_error( "%s: sdma_set_device_handler failed", __func__ );
         goto failed_sdma_set_device_handler;
      }

      INIT_COMPLETION( gDmaDone );   /* Mark as incomplete */
   }

   /* Double check the memory is supported by dma_mmap */
   rc = dma_mmap_dma_is_supported( armaddr );
   if( !rc )
   {
      vcos_log_error( "%s: Buffer not supported buf=0x%lx", __func__, (unsigned long)armaddr );
      goto failed_dma_mmap_dma_is_supported;
   }

   /* Set the pagelist (for user buffers) */
   if (pagelist)
      dma_mmap_set_pagelist( &gVchiqDmaMmap, pagelist);

   /* Map memory */
   rc = dma_mmap_map( &gVchiqDmaMmap, armaddr, len, dir );
   if( rc < 0 )
   {
      vcos_log_error( "%s: dma_mmap_map FAILED buf=0x%lx len=0x%lx",
            __func__, (unsigned long)armaddr, (unsigned long)len);
      goto failed_dma_mmap_map;
   }

   if ( dmaDev == DMA_DEVICE_NONE )
   {
      /* Calculate the kernel virtual address using the address offset */
      void *vcVirtAddr = (void *)(mm_vc_mem_virt_addr + vcAddrOffset);

      dma_mmap_memcpy( &gVchiqDmaMmap, vcVirtAddr);
   }
   else
   {
      rc = sdma_map_create_descriptor_ring( dmaHndl, &gVchiqDmaMmap,
         vcPhysAddr, DMA_UPDATE_MODE_INC );
      if( rc < 0 )
      {
         vcos_log_error( "%s: sdma_map_create_descriptor_ring FAILED rc=%u",
            __func__, rc );
         vcos_log_error( "%s: vcaddr=0x%p armaddr=0x%p len=%d dir=%s",
            __func__, vcaddr, armaddr, len,
            dma_data_direction_as_str( dir ));
         goto failed_sdma_map_create_descriptor_ring;
      }
      if ((rc = sdma_start_transfer( dmaHndl )) != 0 )
      {
         vcos_log_error( "%s: DMA failed %d", __func__, rc );
         goto failed_sdma_start_transfer;
      }

      wait_for_completion( &gDmaDone );
   }

   rc = 0;

failed_sdma_start_transfer:
failed_sdma_map_create_descriptor_ring:
   dma_mmap_unmap( &gVchiqDmaMmap, (dir == DMA_FROM_DEVICE) ? DMA_MMAP_DIRTIED : DMA_MMAP_CLEAN );
failed_dma_mmap_map:
failed_dma_mmap_dma_is_supported:
failed_sdma_set_device_handler:
   if ( dmaHndl != SDMA_INVALID_HANDLE )
   {
      sdma_free_channel( dmaHndl );
      dmaHndl = SDMA_INVALID_HANDLE;
   }
failed_sdma_request_channel:
   vcos_mutex_unlock(&g_dma_mutex);

   return rc;
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
static void
service_gpio( uint32_t irq_status )
{
#define GPIO_MAILBOX_WRITE            (*(volatile uint32_t *)(IPC_SHARED_MEM_BASE + IPC_SHARED_MEM_CHANNEL_ARM_OFFSET + IPC_SHARED_MEM_GPIO_WRITE_OFFSET))
#define GPIO_MAILBOX_READ             (*(volatile uint32_t *)(IPC_SHARED_MEM_BASE + IPC_SHARED_MEM_CHANNEL_ARM_OFFSET + IPC_SHARED_MEM_GPIO_READ_OFFSET))

#define GPIO_MAILBOX_WRITE_SET        (0x80000000)
#define GPIO_MAILBOX_WRITE_PIN_MASK   (0x7FFFFFFF)

   if ( irq_status & ( IPC_INTERRUPT_STATUS_ENABLED << IPC_INTERRUPT_SOURCE_2 ))
   {
      uint32_t reg = GPIO_MAILBOX_WRITE;

      if ( reg & GPIO_MAILBOX_WRITE_SET )
      {
         /* GPIO set */
         gpio_set_value( reg & GPIO_MAILBOX_WRITE_PIN_MASK , 1 );
      }
      else
      {
         /* GPIO clear */
         gpio_set_value( reg & GPIO_MAILBOX_WRITE_PIN_MASK , 0 );
      }

      /* Notify videocore that GPIO has been changed */
      chal_ipc_int_vcset( ipcHandle, IPC_INTERRUPT_SOURCE_2 );

      irq_status &= ~( IPC_INTERRUPT_STATUS_ENABLED << IPC_INTERRUPT_SOURCE_2 );
   }

   if ( irq_status & ( IPC_INTERRUPT_STATUS_ENABLED << IPC_INTERRUPT_SOURCE_3 ))
   {
      /* GPIO set */
      GPIO_MAILBOX_READ = gpio_get_value( GPIO_MAILBOX_READ );

      /* Notify videocore that GPIO has been set */
      chal_ipc_int_vcset( ipcHandle, IPC_INTERRUPT_SOURCE_3 );

      irq_status &= ~( IPC_INTERRUPT_STATUS_ENABLED << IPC_INTERRUPT_SOURCE_3 );
   }

   if ( irq_status & ( IPC_INTERRUPT_STATUS_ENABLED << IPC_INTERRUPT_SOURCE_4 ))
   {
      /* Reply back to the videocore to tell them we are ready */
      chal_ipc_int_vcset( ipcHandle, IPC_INTERRUPT_SOURCE_4 );

      irq_status &= ~( IPC_INTERRUPT_STATUS_ENABLED << IPC_INTERRUPT_SOURCE_4 );
   }
}

/*
 * Local functions
 */

static irqreturn_t
vchiq_doorbell_irq(int irq, void *dev_id)
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
	VCHIQ_SLOT_ZERO_T *vchiq_slot_zero;
	VCHIQ_STATE_T *state;
	VCHIQ_STATUS_T status;
	int err = 0;
	int i;

   vcos_log_warn( "%s: IPC_SHARED_MEM_SLOTS_VIRT = 0x%lx", __func__, IPC_SHARED_MEM_SLOTS_VIRT );
   vcos_log_warn( "%s: IPC_SHARED_MEM_SLOTS_SIZE = 0x%x", __func__, IPC_SHARED_MEM_SLOTS_SIZE );
   vcos_log_warn( "%s: VCHIQ_MAX_SERVICES        = %d", __func__, VCHIQ_MAX_SERVICES );

   vchiq_slot_zero = (VCHIQ_SLOT_ZERO_T *)IPC_SHARED_MEM_SLOTS_VIRT;
   state = g_vchiq_state;

   /* Initialize the local state. Note that vc04 has already started by now
      so the slot memory is expected to be initialised. */
	status = vchiq_init_state(state, vchiq_slot_zero, 1/*master*/);

   if (status != VCHIQ_SUCCESS)
      goto failed_init_state;

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
      IRQF_DISABLED, "IPC driver", state )) != 0 )
   {
      vcos_log_error( "%s: failed to register irq=%d err=%d", __func__,
         BCM_INT_ID_IPC_OPEN, err );
      goto failed_request_irq;
   }
   else
   {
      /* Tell the videocore we are ready for servicing GPIO requests */
      chal_ipc_int_vcset( ipcHandle, IPC_INTERRUPT_SOURCE_4 );
   }

   /* initialize dma_mmap for use */
   dma_mmap_init_map( &gVchiqDmaMmap );

	return 0;

failed_request_irq:
failed_init_state:
	return VCHIQ_ERROR;
}
