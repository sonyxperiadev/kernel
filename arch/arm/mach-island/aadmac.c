/*****************************************************************************
* Copyright 2004 - 2011 Broadcom Corporation.  All rights reserved.
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

/****************************************************************************/
/**
*   @file   aadmac.c
*
*   @brief  Implements the AADMA interface.
*/
/****************************************************************************/

/* ---- Include Files ---------------------------------------------------- */

#include <linux/module.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/irqreturn.h>
#include <linux/proc_fs.h>
#include <linux/hugetlb.h>
#include <linux/version.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/pfn.h>

#ifdef CONFIG_BCM_KNLLOG_IRQ
#include <linux/broadcom/knllog.h>
#endif

#include <asm/atomic.h>

#include <mach/aadma.h>
#include <mach/rdb/brcm_rdb_sysmap.h>

#include <chal/chal_caph.h>
#include <chal/chal_caph_cfifo.h>
#include <chal/chal_caph_dma.h>
#include <chal/chal_caph_intc.h>
#include <chal/chal_audio.h>

/* ---- Public Variables ------------------------------------------------- */

/* ---- Private Constants and Types -------------------------------------- */

#define CFIFO_PHYS_BASE_ADDR_START        ( CFIFO_BASE_ADDR )
#define AADMAC_PHYS_BASE_ADDR_START       ( AADMAC_BASE_ADDR )
#define AHINTC_PHYS_BASE_ADDR_START       ( AHINTC_BASE_ADDR )

#define CFIFO_REGISTER_LENGTH             0X600  /* Arbitrarily longer than actual */
#define AADMAC_REGISTER_LENGTH            0x200  /* Arbitrarily longer than actual */
#define AHINTC_REGISTER_LENGTH            0x200  /* Arbitrarily longer than actual */

#define MAKE_HANDLE( controllerIdx, channelIdx )    (( (controllerIdx) << 4 ) | (channelIdx) )

#define CONTROLLER_FROM_HANDLE( handle )    (( (handle) >> 4 ) & 0x0f )
#define CHANNEL_FROM_HANDLE( handle )       (  (handle) & 0x0f )

#define AADMA_DEF_BURST_SIZE           0x4
#define AADMA_DEF_MONO_IGR_FIFO_SIZE   32
#define AADMA_DEF_MONO_EGR_FIFO_SIZE   512
#define AADMA_DEF_STEREO_IGR_FIFO_SIZE 32
#define AADMA_DEF_STEREO_EGR_FIFO_SIZE 1024
#define AADMA_DEF_IGR_THRES1           (32)
#define AADMA_DEF_EGR_THRES1           4
#define AADMA_DEF_THRES2               0

#define CFIFO_SEG_SIZE_BYTES        32
#define CFIFO_SLOT_SIZE_BYTES       256
#define CFIFO_TOTAL_BUF_SIZE        8192
#define CFIFO_NUM_SEG_PER_SLOT      (CFIFO_SLOT_SIZE_BYTES / CFIFO_SEG_SIZE_BYTES)
#define CFIFO_NUM_SLOT_IN_BUF       (CFIFO_TOTAL_BUF_SIZE / CFIFO_SLOT_SIZE_BYTES)

typedef struct
{
   int seg_allocated;
   int seg_alloc_ch_id;

} AADMA_cfifo_buf_seg_t;


typedef struct
{
   unsigned int num_seg_allocated;
   AADMA_cfifo_buf_seg_t seg[CFIFO_NUM_SEG_PER_SLOT];

} AADMA_cfifo_buf_slot_t;

/* ---- Private Variables ------------------------------------------------ */

static AADMA_Global_t            gAADMA;
static struct proc_dir_entry    *gDmaDir;
static AADMA_ChalHandle_t        gChalHandle;

static spinlock_t                gHwAADmaLock;  /* acquired when starting DMA channel */
static spinlock_t                gAADmaDevLock; /* acquired when setting device handler */

static AADMA_cfifo_buf_slot_t    gCfifoBuffer[CFIFO_NUM_SLOT_IN_BUF];

#define DEVICE_AUDIOH_VINR_CFIFO_TO_MEM \
{ \
   .flags = AADMA_DEVICE_FLAG_SECURE, \
   .name = "audioh vinr to mem", \
   .aadma_trans_size = AADMA_DEF_BURST_SIZE,     \
   .cfifo_dir = CAPH_CFIFO_OUT, \
   .cfifo_size = AADMA_DEF_MONO_IGR_FIFO_SIZE, \
   .cfifo_thres1 = AADMA_DEF_IGR_THRES1, \
   .cfifo_thres2 = AADMA_DEF_THRES2, \
   .dev_fifo_addr = AUDIOH_BASE_ADDR + CHAL_AUDIO_VINR_FIFO_OFFSET, \
}

#define DEVICE_AUDIOH_VINL_CFIFO_TO_MEM \
{ \
   .flags = AADMA_DEVICE_FLAG_SECURE, \
   .name = "audioh vinl to mem", \
   .aadma_trans_size = AADMA_DEF_BURST_SIZE,     \
   .cfifo_dir = CAPH_CFIFO_OUT, \
   .cfifo_size = AADMA_DEF_MONO_IGR_FIFO_SIZE, \
   .cfifo_thres1 = AADMA_DEF_IGR_THRES1, \
   .cfifo_thres2 = AADMA_DEF_THRES2, \
   .dev_fifo_addr = AUDIOH_BASE_ADDR + CHAL_AUDIO_VINL_FIFO_OFFSET, \
}

#define DEVICE_AUDIOH_NVINR_CFIFO_TO_MEM \
{ \
   .flags = AADMA_DEVICE_FLAG_SECURE, \
   .name = "audioh nvinr to mem", \
   .aadma_trans_size = AADMA_DEF_BURST_SIZE,     \
   .cfifo_dir = CAPH_CFIFO_OUT, \
   .cfifo_size = AADMA_DEF_MONO_IGR_FIFO_SIZE, \
   .cfifo_thres1 = AADMA_DEF_IGR_THRES1, \
   .cfifo_thres2 = AADMA_DEF_THRES2, \
   .dev_fifo_addr = AUDIOH_BASE_ADDR + CHAL_AUDIO_NVINR_FIFO_OFFSET, \
}

#define DEVICE_AUDIOH_NVINL_CFIFO_TO_MEM \
{ \
   .flags = AADMA_DEVICE_FLAG_SECURE, \
   .name = "audioh nvinl to mem", \
   .aadma_trans_size = AADMA_DEF_BURST_SIZE,     \
   .cfifo_dir = CAPH_CFIFO_OUT, \
   .cfifo_size = AADMA_DEF_MONO_IGR_FIFO_SIZE, \
   .cfifo_thres1 = AADMA_DEF_IGR_THRES1, \
   .cfifo_thres2 = AADMA_DEF_THRES2, \
   .dev_fifo_addr = AUDIOH_BASE_ADDR + CHAL_AUDIO_NVINL_FIFO_OFFSET, \
}

#define DEVICE_AUDIOH_MEM_TO_CFIFO_TO_EARPIECE \
{ \
   .flags = AADMA_DEVICE_FLAG_SECURE, \
   .name = "audioh mem to earpiece", \
   .aadma_trans_size = AADMA_DEF_BURST_SIZE,     \
   .cfifo_dir = CAPH_CFIFO_IN, \
   .cfifo_size = AADMA_DEF_MONO_EGR_FIFO_SIZE, \
   .cfifo_thres1 = AADMA_DEF_EGR_THRES1, \
   .cfifo_thres2 = AADMA_DEF_THRES2, \
   .dev_fifo_addr = AUDIOH_BASE_ADDR + CHAL_AUDIO_EARPIECE_FIFO_OFFSET, \
}

#define DEVICE_AUDIOH_MEM_TO_CFIFO_TO_HEADSET \
{ \
   .flags = AADMA_DEVICE_FLAG_SECURE, \
   .name = "audioh mem to headset", \
   .aadma_trans_size = AADMA_DEF_BURST_SIZE,     \
   .cfifo_dir = CAPH_CFIFO_IN, \
   .cfifo_size = AADMA_DEF_STEREO_EGR_FIFO_SIZE, \
   .cfifo_thres1 = AADMA_DEF_EGR_THRES1, \
   .cfifo_thres2 = AADMA_DEF_THRES2, \
   .dev_fifo_addr = AUDIOH_BASE_ADDR + CHAL_AUDIO_HEADSET_FIFO_OFFSET, \
}

#define DEVICE_AUDIOH_MEM_TO_CFIFO_TO_HANDSFREE \
{ \
   .flags = AADMA_DEVICE_FLAG_SECURE, \
   .name = "audioh mem to handsfree", \
   .aadma_trans_size = AADMA_DEF_BURST_SIZE,     \
   .cfifo_dir = CAPH_CFIFO_IN, \
   .cfifo_size = AADMA_DEF_MONO_EGR_FIFO_SIZE, \
   .cfifo_thres1 = AADMA_DEF_EGR_THRES1, \
   .cfifo_thres2 = AADMA_DEF_THRES2, \
   .dev_fifo_addr = AUDIOH_BASE_ADDR + CHAL_AUDIO_IHF_FIFO_OFFSET, \
}

#define DEVICE_AUDIOH_MEM_TO_CFIFO_TO_VIBRA \
{ \
   .flags = AADMA_DEVICE_FLAG_SECURE, \
   .name = "audioh mem to vibra", \
   .aadma_trans_size = AADMA_DEF_BURST_SIZE,     \
   .cfifo_dir = CAPH_CFIFO_IN, \
   .cfifo_size = AADMA_DEF_MONO_EGR_FIFO_SIZE, \
   .cfifo_thres1 = AADMA_DEF_EGR_THRES1, \
   .cfifo_thres2 = AADMA_DEF_THRES2, \
   .dev_fifo_addr = AUDIOH_BASE_ADDR + CHAL_AUDIO_VIBRA_FIFO_OFFSET, \
}


AADMA_DeviceAttribute_t AADMA_gDeviceAttribute[ AADMA_NUM_DEVICE_ENTRIES ] =
{
    [AADMA_DEVICE_AUDIOH_VINR_CFIFO_TO_MEM] = DEVICE_AUDIOH_VINR_CFIFO_TO_MEM,
    [AADMA_DEVICE_AUDIOH_VINL_CFIFO_TO_MEM] = DEVICE_AUDIOH_VINL_CFIFO_TO_MEM,
    [AADMA_DEVICE_AUDIOH_NVINR_CFIFO_TO_MEM] = DEVICE_AUDIOH_NVINR_CFIFO_TO_MEM,
    [AADMA_DEVICE_AUDIOH_NVINL_CFIFO_TO_MEM] = DEVICE_AUDIOH_NVINL_CFIFO_TO_MEM,
    [AADMA_DEVICE_AUDIOH_MEM_TO_CFIFO_TO_EARPIECE] = DEVICE_AUDIOH_MEM_TO_CFIFO_TO_EARPIECE,
    [AADMA_DEVICE_AUDIOH_MEM_TO_CFIFO_TO_HEADSET] = DEVICE_AUDIOH_MEM_TO_CFIFO_TO_HEADSET,
    [AADMA_DEVICE_AUDIOH_MEM_TO_CFIFO_TO_HANDSFREE] = DEVICE_AUDIOH_MEM_TO_CFIFO_TO_HANDSFREE,
    [AADMA_DEVICE_AUDIOH_MEM_TO_CFIFO_TO_VIBRA] = DEVICE_AUDIOH_MEM_TO_CFIFO_TO_VIBRA,
};

EXPORT_SYMBOL( AADMA_gDeviceAttribute );

/* ---- Private Function Prototypes -------------------------------------- */
/* ---- Functions  ------------------------------------------------------- */


/****************************************************************************/
/**
*   Displays information for /proc/dma/channels
*/
/****************************************************************************/

static int aadma_proc_read_channels( char *buf, char **start, off_t offset, int count, int *eof, void *data)
{
   int             channelIdx;
   int             limit = count - 200;
   int             len = 0;
   AADMA_Channel_t  *channel;

   down( &gAADMA.lock );

   for ( channelIdx = 0; channelIdx < AADMA_NUM_CHANNELS; channelIdx++ )
   {
      if ( len >= limit )
      {
         break;
      }

      channel = &gAADMA.channel[ channelIdx ];

      len += sprintf( buf + len, "%d ", channelIdx );

      len += sprintf( buf + len, "Fifo: %d  ", AADMA_gDeviceAttribute[ channel->devType ].cfifo_size  );

      if (( channel->flags & AADMA_CHANNEL_FLAG_IN_USE ) != 0 )
      {
         len += sprintf( buf + len, "InUse by %s", AADMA_gDeviceAttribute[ channel->devType ].name );
#if ( AADMA_DEBUG_TRACK_RESERVATION )
         len += sprintf( buf + len, " (%s:%d)", channel->fileName, channel->lineNum );
#endif
      }
      else
      {
         len += sprintf( buf + len, "Avail " );
      }

      len += sprintf( buf + len, "\n" );
   }
   up( &gAADMA.lock );
   *eof = 1;

   return len;
}

/****************************************************************************/
/**
*   Displays information for /proc/dma/devices
*/
/****************************************************************************/

static int aadma_proc_read_devices( char *buf, char **start, off_t offset, int count, int *eof, void *data)
{
   int     limit = count - 200;
   int     len = 0;
   int     devIdx;

   down( &gAADMA.lock );

   for ( devIdx = 0; devIdx < AADMA_NUM_DEVICE_ENTRIES; devIdx++ )
   {
      AADMA_DeviceAttribute_t   *devAttr = &AADMA_gDeviceAttribute[ devIdx ];

      if ( devAttr->name == NULL )
      {
         continue;
      }

      if ( len >= limit )
      {
         break;
      }

      len += sprintf( buf + len, "%-12s ", devAttr->name );

      len += sprintf( buf + len, " " );

      len += sprintf( buf + len, "Xfer #: %Lu Ticks: %Lu Bytes: %Lu\n",
            devAttr->numTransfers,
            devAttr->transferTicks,
            devAttr->transferBytes );
   }

   up( &gAADMA.lock );
   *eof = 1;

   return len;
}

/****************************************************************************/
/**
*   Determines if a AADMA_Device_t is "valid".
*
*   @return
*       TRUE        - dma device is valid
*       FALSE       - dma device isn't valid
*/
/****************************************************************************/

static inline int IsDeviceValid( AADMA_Device_t device )
{
   return ( device >= 0 ) && ( device < (AADMA_Device_t)AADMA_NUM_DEVICE_ENTRIES );
}

/****************************************************************************/
/**
*   Translates a DMA handle into a pointer to a channel.
*
*   @return
*       non-NULL    - pointer to AADMA_Channel_t
*       NULL        - DMA Handle was invalid
*/
/****************************************************************************/

static inline AADMA_Channel_t *HandleToChannel( AADMA_Handle_t handle )
{
    int channelIdx = CHANNEL_FROM_HANDLE( handle );
    if ( channelIdx > AADMA_NUM_CHANNELS )
    {
        return NULL;
    }
    return &gAADMA.channel[ channelIdx ];
}

/****************************************************************************/
/**
*   Interrupt handler which is called to process DMA interrupts.
*/
/****************************************************************************/

static irqreturn_t aadma_interrupt_handler( int irq, void *dev_id )
{
   AADMA_Channel_t *channel;
   AADMA_DeviceAttribute_t *devAttr;
   AADMA_ChalHandle_t *chal_hdl;
   AADMA_Status_t aadma_status;
   volatile uint32_t status;
   CAPH_DMA_CHANNEL_e dma_ch;
   CAPH_DMA_CHNL_FIFO_STATUS_e dma_status;
   int i, j;
   int found;

   chal_hdl = (AADMA_ChalHandle_t *)dev_id;

   status = chal_caph_intc_read_dma_intr( chal_hdl->chalAhintcHandle, CAPH_ARM );

   for ( i = 0; i < AADMA_NUM_CHANNELS; i++ )
   {
      dma_ch = (CAPH_DMA_CHANNEL_e)(1UL << i );
      if ( status & dma_ch )
      {
         found = 0;

         /* Obtain DMA status */
         dma_status = chal_caph_dma_read_ddrfifo_status( chal_hdl->chalAadmacHandle, dma_ch );

         /* Always clear status */
         chal_caph_dma_clr_ddrfifo_status( chal_hdl->chalAadmacHandle, dma_ch, dma_status );

         for( j = 0; j < AADMA_NUM_CHANNELS; j++ )
         {
            channel = &gAADMA.channel[j];
            if( dma_ch == channel->caph_dma_ch )
            {
               found = 1;
               break;
            }
         }

         if( found )
         {
            /* Get channel attribute */
            devAttr = &AADMA_gDeviceAttribute[channel->devType];

            devAttr->numTransfers++;
            devAttr->transferBytes += devAttr->numBytes;
            devAttr->transferTicks += (timer_get_tick_count() - devAttr->transferStartTime);

            aadma_status.dma_status = dma_status;

            /* Call installed handler */
            if ( devAttr->devHandler )
            {
               devAttr->devHandler( channel->devType, &aadma_status, devAttr->userData );
            }
         }
      }
   }

   /* Clear interrupt status */
   chal_caph_intc_clr_dma_intr( chal_hdl->chalAhintcHandle, status, CAPH_ARM );

   return IRQ_HANDLED;
}

static int aadma_request_cfifo_mem_alloc( uint32_t *start_addr, uint16_t *req_bytes, int fifo_ch )
{
   int i, j;
   AADMA_cfifo_buf_slot_t *cfifo_buf_end_p = gCfifoBuffer;
   AADMA_cfifo_buf_slot_t *cfifo_buf_start_p = gCfifoBuffer;
   uint16_t bytes = *req_bytes;
   uint16_t num_slots = 0;
   int num_contiguous_slots = 0;
   int done = 0;

   if( bytes < CFIFO_SEG_SIZE_BYTES )
   {
      printk( KERN_ERR "%s Invalid size %d bytes requested\n", __FUNCTION__, bytes);
      /* Unable to allocate less than size of segment */
      goto error;
   }

   /* Special case where sizes less than 2 slots will default to a single segment */
   if( bytes < (2 * CFIFO_SLOT_SIZE_BYTES) )
   {
      num_slots = 0;
      bytes = CFIFO_SEG_SIZE_BYTES;
   }
   else
   {
      /* Allowed values are (2 * slot size) + (n * slot size),
       * where n must be positive or zero.  Will always round down
       * to nearest acceptable value.
       */
      num_slots = (bytes / CFIFO_SLOT_SIZE_BYTES);
      bytes = (num_slots * CFIFO_SLOT_SIZE_BYTES);
   }

   /* Search for available space in buffer pool */
   for( i = 0; i < CFIFO_NUM_SLOT_IN_BUF; i++, cfifo_buf_end_p++ )
   {
      /* If a slot is requested and no segments are allocated,
       * entire slot is available */
      if( num_slots > 0 )
      {
         if( cfifo_buf_end_p->num_seg_allocated == 0 )
         {
            if( num_contiguous_slots == 0 )
            {
               cfifo_buf_start_p = cfifo_buf_end_p;
            }

            /* Track the amount of contiguous slots until we reach the desired goal */
            num_contiguous_slots++;
         }
         else
         {
            /* Found a slot with allocated segments so reset contiguous tracker
             * of unallocated slots */
            num_contiguous_slots = 0;
         }

         if( num_contiguous_slots >= num_slots )
         {
            /* Found enough available slots to allocate */
            while( cfifo_buf_start_p <= cfifo_buf_end_p )
            {
               /* Mark all segments as allocated */
               for( j = 0; j < CFIFO_NUM_SEG_PER_SLOT; j++ )
               {
                  cfifo_buf_start_p->seg[j].seg_allocated = 1;
                  cfifo_buf_start_p->seg[j].seg_alloc_ch_id = fifo_ch;
                  cfifo_buf_start_p->num_seg_allocated++;
               }
               cfifo_buf_start_p++;
            }
            *start_addr = ((i + 1) - num_slots) * CFIFO_SLOT_SIZE_BYTES;
            *req_bytes = bytes;
            done = 1;
            break;
         }
      }
      else
      {
         /* Looking for a segment to allocate */
         if( cfifo_buf_end_p->num_seg_allocated < CFIFO_NUM_SEG_PER_SLOT )
         {
            /* Still available segments to be allocated */
            for( j = 0; j < CFIFO_NUM_SEG_PER_SLOT; j++ )
            {
               if( cfifo_buf_start_p->seg[j].seg_allocated == 0 )
               {
                  cfifo_buf_start_p->seg[j].seg_allocated = 1;
                  cfifo_buf_start_p->seg[j].seg_alloc_ch_id = fifo_ch;
                  cfifo_buf_end_p->num_seg_allocated++;
                  break;
               }
            }
            *start_addr = (i * CFIFO_SLOT_SIZE_BYTES) + (j * CFIFO_SEG_SIZE_BYTES);
            *req_bytes = bytes;
            done = 1;
            break;
         }
      }
   }

   if( !done )
   {
error:
      *req_bytes = 0;
      *start_addr = 0;
      return -1;
   }

   return 0;
}

static int aadma_free_cfifo_mem_alloc( int fifo_ch )
{
   int i, j;
   AADMA_cfifo_buf_slot_t *cfifo_buf_p = gCfifoBuffer;

   for( i = 0; i < CFIFO_NUM_SLOT_IN_BUF; i++, cfifo_buf_p++ )
   {
      for( j = 0; j < CFIFO_NUM_SEG_PER_SLOT; j++ )
      {
         if( cfifo_buf_p->num_seg_allocated > 0 )
         {
            if( cfifo_buf_p->seg[j].seg_allocated && cfifo_buf_p->seg[j].seg_alloc_ch_id == fifo_ch )
            {
               cfifo_buf_p->seg[j].seg_allocated = 0;
               cfifo_buf_p->seg[j].seg_alloc_ch_id = 0;
               cfifo_buf_p->num_seg_allocated--;
            }
         }
      }
   }
   return 0;
}

static int aadma_find_channel( AADMA_Channel_t *channel )
{
   int i;
   for( i = 0; i < AADMA_NUM_CHANNELS; i++, channel++ )
   {
      if( !(channel->flags & AADMA_CHANNEL_FLAG_IN_USE) )
      {
         break;
      }
   }

   if( i == AADMA_NUM_CHANNELS )
   {
      /* Unable to find available AADMA channel */
      return -1;
   }

   return i;
}

static int aadma_reset_intc( AADMA_ChalHandle_t *chal_handle )
{
   /* Should be broken out to seperate AHINTC section */
   chal_caph_intc_reset( chal_handle->chalAhintcHandle );
   chal_caph_intc_set_cfifo_int_detect_level( chal_handle->chalAhintcHandle, 0xFFFF, 0x0000 );
   return 0;
}

static int aadma_ioremap_init( AADMA_ChalHandle_t *chal_handle )
{
   void __iomem *cfifo_virt_addr = 0;
   void __iomem *aadmac_virt_addr = 0;
   void __iomem *ahintc_virt_addr = 0;

   struct resource *cfifo_ioarea;
   struct resource *aadmac_ioarea;
   struct resource *ahintc_ioarea;

   cfifo_ioarea = request_mem_region( CFIFO_PHYS_BASE_ADDR_START, CFIFO_REGISTER_LENGTH, "CFIFO Component Mem Region" );
   aadmac_ioarea = request_mem_region( AADMAC_PHYS_BASE_ADDR_START, AADMAC_REGISTER_LENGTH, "AADMAC Component Mem Region" );
   ahintc_ioarea = request_mem_region( AHINTC_PHYS_BASE_ADDR_START, AHINTC_REGISTER_LENGTH, "AHINTC Component Mem Region" );

   if( !cfifo_ioarea    ||
       !aadmac_ioarea   ||
       !ahintc_ioarea   )
   {
      return -EBUSY;
   }

   cfifo_virt_addr = ioremap( CFIFO_PHYS_BASE_ADDR_START, CFIFO_REGISTER_LENGTH );
   aadmac_virt_addr = ioremap( AADMAC_PHYS_BASE_ADDR_START, AADMAC_REGISTER_LENGTH );
   ahintc_virt_addr = ioremap( AHINTC_PHYS_BASE_ADDR_START, AHINTC_REGISTER_LENGTH );

   if( !cfifo_virt_addr || !aadmac_virt_addr || !ahintc_virt_addr )
   {
      return -ENOMEM;
   }

   chal_handle->chalAadmacHandle = chal_caph_dma_init((uint32_t)aadmac_virt_addr);
   chal_handle->chalCfifoHandle = chal_caph_cfifo_init((uint32_t)cfifo_virt_addr);
   chal_handle->chalAhintcHandle = chal_caph_intc_init((uint32_t)ahintc_virt_addr);

   return 0;
}

static int aadma_ioremap_exit( AADMA_ChalHandle_t *chal_handle )
{
   chal_caph_dma_deinit( chal_handle->chalAadmacHandle );
   chal_caph_cfifo_deinit( chal_handle->chalCfifoHandle );
   chal_caph_intc_deinit( chal_handle->chalAhintcHandle );

   release_mem_region( CFIFO_PHYS_BASE_ADDR_START, CFIFO_REGISTER_LENGTH );
   release_mem_region( AADMAC_PHYS_BASE_ADDR_START, AADMAC_REGISTER_LENGTH );
   release_mem_region( AHINTC_PHYS_BASE_ADDR_START, AHINTC_REGISTER_LENGTH );

   return 0;
}

/****************************************************************************/
/**
*   Obtains the physical address to the CFIFO channel FIFO
*/
/****************************************************************************/
uint32_t aadma_get_cfifo_address
(
   AADMA_Handle_t handle    /* DMA handle. */
)
{
   AADMA_ChalHandle_t         *chal_handle;
   AADMA_Channel_t            *channel;
   uint32_t                   cfifo_addr;

   if (( channel = HandleToChannel( handle )) == NULL )
   {
      return 0;
   }

   chal_handle = &gChalHandle;

   cfifo_addr = (uint16_t)(CFIFO_PHYS_BASE_ADDR_START +
         chal_caph_cfifo_get_fifo_addr_offset( chal_handle->chalCfifoHandle, channel->caph_cfifo_ch ));

   return cfifo_addr;
}

EXPORT_SYMBOL(aadma_get_cfifo_address);

/****************************************************************************/
/**
*   Initialize memory space and intialize the clock.
*   @return
*       >= 0    - Initialization was successfull.
*
*       -EBUSY  - Device is currently being used.
*       -ENOMEM - Device has invalid memory region.
*/
/****************************************************************************/

int aadma_init( void )
{
   int             rc = 0;

   printk( "%s: initializing AADMA\n", __func__ );

   if( (rc = aadma_ioremap_init( &gChalHandle )) != 0 )
   {
      printk( KERN_ERR "Failed to perform i/o remap\n" );
      return rc;
   }

   /* Enable CAPH clock */
   aadma_reset_intc( &gChalHandle );

   return rc;
}

EXPORT_SYMBOL(aadma_init);

/****************************************************************************/
/**
*   Sets up all of the data structures associated with the DMA.
*   @return
*       >= 0    - Setup was successful.
*
*       -EBUSY  - Device is currently being used.
*       -ENODEV - Device handed in is invalid.
*/
/****************************************************************************/

int aadma_setup( void )
{
   int             rc = 0;
   int             channelIdx;
   AADMA_Channel_t  *channel;

   printk( "%s: Setting up AADMA\n", __func__ );

   memset( &gAADMA, 0, sizeof( gAADMA ));
   memset( &gCfifoBuffer, 0, sizeof( gCfifoBuffer ));

   sema_init( &gAADMA.lock, 1 );
   spin_lock_init( &gHwAADmaLock );
   spin_lock_init( &gAADmaDevLock );

   for ( channelIdx = 0; channelIdx < AADMA_NUM_CHANNELS; channelIdx++ )
   {
      channel = &gAADMA.channel[ channelIdx ];

      channel->flags = 0;
      channel->devType = AADMA_DEVICE_NONE;

#if ( AADMA_DEBUG_TRACK_RESERVATION )
      channel->fileName = "";
      channel->lineNum = 0;
#endif
      channel->caph_dma_ch = CAPH_DMA_CH_VOID;
      channel->caph_cfifo_ch = CAPH_CFIFO_VOID;
   }

   if (( rc = request_irq( BCM_INT_ID_CAPH, aadma_interrupt_handler,
               IRQF_DISABLED, "AADMA Handler", &gChalHandle )) != 0 )
   {
      printk( KERN_ERR "request_irq for AADMA failed\n" );
      return rc;
   }

   /* Create /proc/dma/channels and /proc/dma/devices */
   gDmaDir = create_proc_entry( "aadma", S_IFDIR | S_IRUGO | S_IXUGO, NULL );

   if ( gDmaDir == NULL )
   {
      printk( KERN_ERR "Unable to create /proc/aadma\n" );
   }
   else
   {
      create_proc_read_entry( "channels", 0, gDmaDir, aadma_proc_read_channels, NULL );
      create_proc_read_entry( "devices",  0, gDmaDir, aadma_proc_read_devices,  NULL );
   }

   return rc;
}

EXPORT_SYMBOL(aadma_setup);

/****************************************************************************/
/**
*   Exits the AADMA module.
*
*   @return
*       0       - Success
*       < 0     - Error
*/
/****************************************************************************/
void aadma_exit( void )
{
   remove_proc_entry( "channels", gDmaDir );
   remove_proc_entry( "devices", gDmaDir );
   remove_proc_entry( "aadma", NULL );

   free_irq( BCM_INT_ID_CAPH, &gChalHandle );
   aadma_ioremap_exit( &gChalHandle );
}

EXPORT_SYMBOL(aadma_exit);

/****************************************************************************/
/**
*   Reserves a channel for use with @a dev. If the device is setup to use
*   a shared channel, then this function will block until a free channel
*   becomes available.
*
*   @return
*       >= 0    - A valid DMA Handle.
*       -EBUSY  - Device is currently being used.
*       -ENODEV - Device handed in is invalid.
*/
/****************************************************************************/

#if ( AADMA_DEBUG_TRACK_RESERVATION )
AADMA_Handle_t aadma_request_channel_dbg
(
   AADMA_Device_t    dev,
   const char        *fileName,
   int               lineNum
)
#else
AADMA_Handle_t aadma_request_channel
(
   AADMA_Device_t    dev
)
#endif
{
   AADMA_Handle_t           handle;
   AADMA_DeviceAttribute_t  *devAttr;
   AADMA_Channel_t          *channel;
   AADMA_ChalHandle_t       *chal_handle;
   int                      channelIdx;
   uint32_t                 startAddr = 0;

   down( &gAADMA.lock );

   /* Get device attribute */
   devAttr = &AADMA_gDeviceAttribute[ dev ];

   if (( dev < 0 ) || ( dev >= AADMA_NUM_DEVICE_ENTRIES ) || devAttr->name == NULL )
   {
      handle = -ENODEV;
      goto out;
   }

#if ( AADMA_DEBUG_TRACK_RESERVATION )
   {
      char *s;

      if (( s = strrchr( fileName, '/' )) != NULL )
      {
         fileName = s+1;
      }
   }
#endif

   if (( devAttr->flags & AADMA_DEVICE_FLAG_IN_USE ) != 0 )
   {
      /* This device has already been requested and not been freed */

      printk( KERN_ERR "%s: device %s is already requested\n", __func__, devAttr->name );
      handle = -EBUSY;
      goto out;
   }

   chal_handle = &gChalHandle;

   /* Obtain a free DMA channel */
   handle = AADMA_INVALID_HANDLE;
   while ( handle == AADMA_INVALID_HANDLE )
   {
      if ( (channelIdx = aadma_find_channel( gAADMA.channel )) >= 0 )
      {
         channel = &gAADMA.channel[ channelIdx ];

         channel->caph_dma_ch = chal_caph_dma_alloc_given_channel( chal_handle->chalAadmacHandle, CAPH_DMA_CH_VOID );
         if( (CAPH_DMA_CHANNEL_e)channel->caph_dma_ch == CAPH_DMA_CH_VOID )
         {
            printk( KERN_ERR "Failed to request CAPH AADMA channel\n" );
            goto error;
         }

         channel->caph_cfifo_ch = chal_caph_cfifo_alloc_given_channel( chal_handle->chalCfifoHandle, CAPH_CFIFO_VOID );
         if( (CAPH_CFIFO_e)channel->caph_dma_ch == CAPH_CFIFO_VOID )
         {
            printk( KERN_ERR "Failed to request CAPH CFIFO channel\n" );
            goto error;
         }

         channel->flags |= AADMA_CHANNEL_FLAG_IN_USE;
         channel->devType = dev;
         devAttr->flags |= AADMA_DEVICE_FLAG_IN_USE;

#if ( AADMA_DEBUG_TRACK_RESERVATION )
         channel->fileName = fileName;
         channel->lineNum = lineNum;
#endif

         handle = MAKE_HANDLE( 0, channelIdx );

         if ( aadma_request_cfifo_mem_alloc( &startAddr, &devAttr->cfifo_size, channel->caph_cfifo_ch ) )
         {
            /* Failed to reserve enough data in CFIFO buffers */
            printk( KERN_ERR "%s Failed to reserve enough data in CFIFO buffers\n", __FUNCTION__ );
            goto error;
         }

         /* Configure DMA burst size */
         chal_caph_dma_set_tsize( chal_handle->chalAadmacHandle, channel->caph_dma_ch, devAttr->aadma_trans_size );

         /* Link CFIFO with DMA channel */
         chal_caph_dma_set_cfifo( chal_handle->chalAadmacHandle, channel->caph_dma_ch, channel->caph_cfifo_ch );

         /* Set CFIFO address */
         chal_caph_cfifo_set_address( chal_handle->chalCfifoHandle, channel->caph_cfifo_ch, startAddr );

         /* Configure DMA direction */
         chal_caph_dma_set_direction( chal_handle->chalAadmacHandle, channel->caph_dma_ch, devAttr->cfifo_dir );

         /* Configure CFIFO direction */
         chal_caph_cfifo_set_direction( chal_handle->chalCfifoHandle, channel->caph_cfifo_ch, devAttr->cfifo_dir );

         /* Set CFIFO size */
         chal_caph_cfifo_set_size( chal_handle->chalCfifoHandle, channel->caph_cfifo_ch, devAttr->cfifo_size );

         /* Set CFIFO threshold */
         chal_caph_cfifo_set_fifo_thres( chal_handle->chalCfifoHandle, channel->caph_cfifo_ch, devAttr->cfifo_thres1, devAttr->cfifo_thres2 );

         /* Clear DMA channel FIFO */
         chal_caph_dma_clr_channel_fifo( chal_handle->chalAadmacHandle, channel->caph_dma_ch );

         /* Enable DMA interrupt for ARM */
         chal_caph_intc_enable_dma_intr( chal_handle->chalAhintcHandle, channel->caph_dma_ch, CAPH_ARM );
         chal_caph_intc_enable_dma_int_err( chal_handle->chalAhintcHandle, channel->caph_dma_ch, CAPH_ARM );

         /* Clear CFIFO */
         chal_caph_cfifo_clr_fifo( chal_handle->chalCfifoHandle, channel->caph_cfifo_ch );

         /* Enable CFIFOF error interrupt */
         chal_caph_intc_enable_cfifo_int_err( chal_handle->chalAhintcHandle, (uint16_t)channel->caph_cfifo_ch, CAPH_ARM );

         goto out;
      }
   }

error:
   if( handle != AADMA_INVALID_HANDLE )
   {
      handle = AADMA_INVALID_HANDLE;
      chal_caph_dma_free_channel( chal_handle->chalAadmacHandle, channel->caph_dma_ch );
      chal_caph_cfifo_free_channel( chal_handle->chalCfifoHandle, channel->caph_cfifo_ch );
      channel->flags &= ~AADMA_CHANNEL_FLAG_IN_USE;
      devAttr->flags &= ~AADMA_DEVICE_FLAG_IN_USE;
   }

out:
   up( &gAADMA.lock );

   return handle;

}

/* Create both _dbg and non _dbg functions for modules. */

#if ( AADMA_DEBUG_TRACK_RESERVATION )
#undef aadma_request_channel
AADMA_Handle_t aadma_request_channel
(
   AADMA_Device_t dev
)
{
   return aadma_request_channel_dbg( dev, __FILE__, __LINE__ );
}

EXPORT_SYMBOL( aadma_request_channel_dbg );
#endif
EXPORT_SYMBOL( aadma_request_channel );

/****************************************************************************/
/**
*   Frees a previously allocated DMA Handle.
*/
/****************************************************************************/

int aadma_free_channel
(
   AADMA_Handle_t handle    /* DMA handle. */
)
{
   int                     rc = 0;
   AADMA_Channel_t          *channel;
   AADMA_DeviceAttribute_t  *devAttr;
   AADMA_ChalHandle_t       *chal_handle;

   down( &gAADMA.lock );

   if (( channel = HandleToChannel( handle )) == NULL )
   {
      rc = -EINVAL;
      goto out;
   }

   chal_handle = &gChalHandle;

   devAttr = &AADMA_gDeviceAttribute[ channel->devType ];

   channel->flags &= ~AADMA_CHANNEL_FLAG_IN_USE;
   devAttr->flags &= ~AADMA_DEVICE_FLAG_IN_USE;

   aadma_free_cfifo_mem_alloc( channel->caph_cfifo_ch );

   channel->caph_dma_ch = CAPH_DMA_CH_VOID;
   channel->caph_cfifo_ch = CAPH_CFIFO_VOID;

out:
   up( &gAADMA.lock );

   return rc;
}

EXPORT_SYMBOL( aadma_free_channel );

/****************************************************************************/
/**
*   Enables AADMA operation.
*
*   @return
*        0      - DMA Handle was released successfully.
*       -EINVAL - Invalid DMA handle
*/
/****************************************************************************/
int aadma_enable
(
    AADMA_Handle_t    handle            /* AADMA handle. */
)
{
   AADMA_Channel_t      *channel;
   AADMA_ChalHandle_t   *chal_handle;

   if (( channel = HandleToChannel( handle )) == NULL )
   {
      return -EINVAL;
   }

   chal_handle = &gChalHandle;

   /*Enable CFIFO */
   chal_caph_cfifo_enable( chal_handle->chalCfifoHandle, channel->caph_cfifo_ch );

   /* Enable DMA */
   chal_caph_dma_enable( chal_handle->chalAadmacHandle, channel->caph_dma_ch );

   return 0;
}

EXPORT_SYMBOL( aadma_enable );

/****************************************************************************/
/**
*   Sets the physical address to the CFIFO channel FIFO
*/
/****************************************************************************/
int aadma_set_device_fifo_address
(
   AADMA_Device_t    dev,
   uint32_t          dev_fifo_addr
)
{
   AADMA_DeviceAttribute_t  *devAttr;

   devAttr = &AADMA_gDeviceAttribute[ dev ];

   down( &gAADMA.lock );

   if (( devAttr->flags & AADMA_DEVICE_FLAG_IN_USE ))
   {
      up( &gAADMA.lock );
      printk( KERN_ERR "%s Unable to set FIFO address as device currently in use\n", __FUNCTION__ );
      return -EBUSY;
   }

   if( !dev_fifo_addr )
   {
      up( &gAADMA.lock );
      printk( KERN_ERR "%s Invalid device FIFO address\n", __FUNCTION__ );
      return -EINVAL;
   }

   devAttr->dev_fifo_addr = dev_fifo_addr;

   up( &gAADMA.lock );

   return 0;
}

EXPORT_SYMBOL( aadma_set_device_fifo_address );

/****************************************************************************/
/**
*   Sets the size to the CFIFO channel FIFO
*/
/****************************************************************************/
int aadma_set_device_fifo_size
(
   AADMA_Device_t    dev,
   uint16_t          fifo_size
)
{
   AADMA_DeviceAttribute_t  *devAttr;

   devAttr = &AADMA_gDeviceAttribute[ dev ];

   down( &gAADMA.lock );

   if (( devAttr->flags & AADMA_DEVICE_FLAG_IN_USE ))
   {
      up( &gAADMA.lock );
      printk( KERN_ERR "%s Unable to set FIFO size as device currently in use\n", __FUNCTION__ );
      return -EBUSY;
   }

   if( !fifo_size )
   {
      up( &gAADMA.lock );
      printk( KERN_ERR "%s Invalid device FIFO address\n", __FUNCTION__ );
      return -EINVAL;
   }

   devAttr->cfifo_size = fifo_size;

   up( &gAADMA.lock );

   return 0;
}

EXPORT_SYMBOL( aadma_set_device_fifo_size );

/****************************************************************************/
/**
*   Sets the threshold values to the CFIFO channel FIFO
*/
/****************************************************************************/
int aadma_set_device_thres
(
   AADMA_Device_t    dev,
   uint16_t          thres1,
   uint16_t          thres2
)
{
   AADMA_DeviceAttribute_t  *devAttr;

   devAttr = &AADMA_gDeviceAttribute[ dev ];

   down( &gAADMA.lock );

   if (( devAttr->flags & AADMA_DEVICE_FLAG_IN_USE ))
   {
      up( &gAADMA.lock );
      printk( KERN_ERR "%s Unable to set FIFO threshold as device currently in use\n", __FUNCTION__ );
      return -EBUSY;
   }

   devAttr->cfifo_thres1 = thres1;
   devAttr->cfifo_thres2 = thres2;

   up( &gAADMA.lock );

   return 0;
}

EXPORT_SYMBOL( aadma_set_device_thres );


/****************************************************************************/
/**
*   Allocates buffers for the descriptors. This is normally done automatically
*   but needs to be done explicitly when initiating a dma from interrupt
*   context.
*
*   @return
*       0       Descriptors were allocated successfully
*       -EINVAL Invalid device type for this kind of transfer
*               (i.e. the device is _MEM_TO_DEV and not _DEV_TO_MEM)
*       -ENOMEM Memory exhausted
*/
/****************************************************************************/

int aadma_alloc_descriptors
(
   AADMA_Handle_t   handle,            /* DMA Handle */
   dma_addr_t       data_addr,         /* Place to read/write data to/from device */
   size_t           numBytes           /* Number of bytes to transfer to the device */
)
{
   AADMA_Channel_t         *channel;
   AADMA_DeviceAttribute_t *devAttr;
   AADMA_ChalHandle_t      *chal_handle;
   unsigned long           flags;

   if (( channel = HandleToChannel( handle )) == NULL )
   {
      return -ENODEV;
   }

   devAttr = &AADMA_gDeviceAttribute[ channel->devType ];

   chal_handle = &gChalHandle;

   spin_lock_irqsave( &gHwAADmaLock, flags );

   channel->ring_buffer = 0;

   /* Set DDR address.  Note, will only transfer half the bytes indicated due to expected double buffer in chal API. */
   chal_caph_dma_set_buffer( chal_handle->chalAadmacHandle, channel->caph_dma_ch, data_addr, (numBytes*2) );

   devAttr->numBytes = numBytes;

   spin_unlock_irqrestore( &gHwAADmaLock, flags );

   return 0;
}
EXPORT_SYMBOL( aadma_alloc_descriptors );

/****************************************************************************/
/**
*   Allocates and sets up descriptors for a double buffered circular buffer
*   for destination data.
*
*   This is primarily intended to be used for things like the ingress samples
*   from a microphone.
*
*   @return
*       > 0     Number of descriptors actually allocated.
*       -EINVAL Invalid device type for this kind of transfer
*               (i.e. the device is _MEM_TO_DEV and not _DEV_TO_MEM)
*       -ENOMEM Memory exhausted
*/
/****************************************************************************/

int aadma_alloc_ring_descriptors
(
    AADMA_Handle_t         handle,         /* DMA Handle */
    dma_addr_t             data_addr,      /* Place to read/write data to/from device */
    size_t                 numBytes        /* Number of bytes in each destination buffer */
)
{
   AADMA_Channel_t         *channel;
   AADMA_DeviceAttribute_t *devAttr;
   AADMA_ChalHandle_t      *chal_handle;
   unsigned long           flags;

   if (( channel = HandleToChannel( handle )) == NULL )
   {
      return -ENODEV;
   }

   devAttr = &AADMA_gDeviceAttribute[ channel->devType ];

   chal_handle = &gChalHandle;

   spin_lock_irqsave( &gHwAADmaLock, flags );

   channel->ring_buffer = 1;

   /* Set DDR address.  Note, will only transfer half the bytes indicated due to expected double buffer in chal API. */
   chal_caph_dma_set_buffer( chal_handle->chalAadmacHandle, channel->caph_dma_ch, data_addr, numBytes);

   devAttr->numBytes = (numBytes / 2);

   spin_unlock_irqrestore( &gHwAADmaLock, flags );

   return 0;
}
EXPORT_SYMBOL( aadma_alloc_ring_descriptors );

/****************************************************************************/
/**
*   Initiates a DMA, allocating the descriptors as required.
*
*   @return
*       0       Transfer was started successfully
*       -EINVAL Invalid device type for this kind of transfer
*               (i.e. the device is _DEV_TO_MEM and not _MEM_TO_DEV)
*/
/****************************************************************************/

int aadma_start_transfer
(
   AADMA_Handle_t           handle         /* DMA Handle */
)
{
   AADMA_Channel_t          *channel;
   AADMA_DeviceAttribute_t  *devAttr;
   AADMA_ChalHandle_t       *chal_handle;
   unsigned long            flags;

   if (( channel = HandleToChannel( handle )) == NULL )
   {
      return -ENODEV;
   }

   devAttr = &AADMA_gDeviceAttribute[ channel->devType ];

   chal_handle = &gChalHandle;

   spin_lock_irqsave( &gHwAADmaLock, flags );

   /* And kick off the transfer */
   devAttr->transferStartTime = timer_get_tick_count();

#ifdef CONFIG_BCM_KNLLOG_IRQ
   if ( gKnllogIrqSchedEnable & KNLLOG_DMA )
   {
      KNLLOG( "tstart [%s hdl=%u devType=%u bytes=%u]\n",
            devAttr->name,
            handle,
            channel->devType,
            devAttr->numBytes );
   }
#endif

   spin_unlock_irqrestore( &gHwAADmaLock, flags );

   if( channel->ring_buffer )
   {
      /* Enable High and Low buffer */
      chal_caph_dma_set_ddrfifo_status( chal_handle->chalAadmacHandle, channel->caph_dma_ch, CAPH_READY_HIGHLOW );
   }
   else
   {
      /* Enable Low buffer */
      chal_caph_dma_set_ddrfifo_status( chal_handle->chalAadmacHandle, channel->caph_dma_ch, CAPH_READY_LOW );
   }

   return 0;
}
EXPORT_SYMBOL( aadma_start_transfer );

/****************************************************************************/
/**
*   Initiates a DMA, allocating the descriptors as required.
*
*   @return
*       0       Transfer was started successfully
*       -EINVAL Invalid device type for this kind of transfer
*               (i.e. the device is _DEV_TO_MEM and not _MEM_TO_DEV)
*/
/****************************************************************************/

int aadma_continue_transfer
(
   AADMA_Handle_t    handle,        /* DMA Handle */
   int               dma_status     /* DMA status bits */
)
{
   AADMA_Channel_t      *channel;
   AADMA_ChalHandle_t   *chal_handle;
   unsigned long        flags;

   if (( channel = HandleToChannel( handle )) == NULL )
   {
      return -ENODEV;
   }

   chal_handle = &gChalHandle;

   if( channel->ring_buffer )
   {
      spin_lock_irqsave( &gHwAADmaLock, flags );

      /* Enable High and Low buffer */
      chal_caph_dma_set_ddrfifo_status( chal_handle->chalAadmacHandle, channel->caph_dma_ch, dma_status );

      spin_unlock_irqrestore( &gHwAADmaLock, flags );
   }
   else
   {
      return -EINVAL;
   }
   return 0;
}
EXPORT_SYMBOL(aadma_continue_transfer);

/****************************************************************************/
/**
*   Stops a previously started DMA transfer.
*
*   @return
*       0       Transfer was stopped successfully
*       -ENODEV Invalid handle
*/
/****************************************************************************/

int aadma_stop_transfer( AADMA_Handle_t handle )
{
   AADMA_Channel_t            *channel;
   AADMA_DeviceAttribute_t    *devAttr;
   AADMA_ChalHandle_t         *chal_handle;
   unsigned long              flags;

   if (( channel = HandleToChannel( handle )) == NULL )
   {
      return -ENODEV;
   }

   devAttr = &AADMA_gDeviceAttribute[ channel->devType ];

   chal_handle = &gChalHandle;

   down( &gAADMA.lock );

   spin_lock_irqsave( &gHwAADmaLock, flags );

   chal_caph_dma_set_ddrfifo_status( chal_handle->chalAadmacHandle, channel->caph_dma_ch, CAPH_READY_NONE);
   chal_caph_dma_clr_channel_fifo( chal_handle->chalAadmacHandle, channel->caph_cfifo_ch );
   chal_caph_dma_disable( chal_handle->chalAadmacHandle, channel->caph_cfifo_ch );

   chal_caph_intc_clr_dma_intr( chal_handle->chalAhintcHandle, (uint16_t)channel->caph_dma_ch, CAPH_ARM );
   chal_caph_intc_clr_dma_int_err( chal_handle->chalAhintcHandle, (uint16_t)channel->caph_dma_ch, CAPH_ARM );

   chal_caph_intc_disable_dma_intr( chal_handle->chalAhintcHandle, (uint16_t)channel->caph_dma_ch, CAPH_ARM );
   chal_caph_intc_disable_dma_int_err( chal_handle->chalAhintcHandle, (uint16_t)channel->caph_dma_ch, CAPH_ARM );

   chal_caph_cfifo_int_disable( chal_handle->chalCfifoHandle, channel->caph_cfifo_ch );
   chal_caph_cfifo_int_err_disable( chal_handle->chalCfifoHandle, channel->caph_cfifo_ch );

   chal_caph_cfifo_disable( chal_handle->chalCfifoHandle, channel->caph_cfifo_ch );
   chal_caph_cfifo_clr_fifo( chal_handle->chalCfifoHandle, channel->caph_cfifo_ch );

   chal_caph_intc_disable_cfifo_intr( chal_handle->chalAhintcHandle, channel->caph_cfifo_ch, CAPH_ARM );
   chal_caph_intc_disable_cfifo_int_err( chal_handle->chalAhintcHandle, channel->caph_cfifo_ch, CAPH_ARM );

   chal_caph_dma_free_channel( chal_handle->chalAadmacHandle, channel->caph_dma_ch );
   chal_caph_cfifo_free_channel( chal_handle->chalCfifoHandle, channel->caph_cfifo_ch );

   spin_unlock_irqrestore( &gHwAADmaLock, flags );

   up( &gAADMA.lock );

   return 0;
}
EXPORT_SYMBOL( aadma_stop_transfer );

/****************************************************************************/
/**
*   Initiates a DMA, allocating the descriptors as required.
*
*   @return
*       0       Transfer was started successfully
*       -EINVAL Invalid device type for this kind of transfer
*               (i.e. the device is _DEV_TO_MEM and not _MEM_TO_DEV)
*/
/****************************************************************************/

int aadma_transfer
(
    AADMA_Handle_t          handle,         /* DMA Handle */
    dma_addr_t              data_addr,      /* Place to read/write data to/from device */
    size_t                  numBytes        /* Number of bytes to transfer to the device */
)
{
   AADMA_Channel_t         *channel;
   int                     rc = 0;

   if (( channel = HandleToChannel( handle )) == NULL )
   {
      return -ENODEV;
   }

   if (( rc = aadma_alloc_descriptors( handle, data_addr, numBytes )) != 0 )
   {
      return rc;
   }

   /* And kick off the transfer */

   return aadma_start_transfer( handle );
}
EXPORT_SYMBOL( aadma_transfer );

/****************************************************************************/
/**
*   Set the callback function which will be called when a transfer completes.
*   If a NULL callback function is set, then no callback will occur.
*
*   @note   @a devHandler will be called from IRQ context.
*
*   @return
*       0       - Success
*       -ENODEV - Device handed in is invalid.
*/
/****************************************************************************/

int aadma_set_device_handler
(
    AADMA_Device_t          dev,           /* Device to set the callback for. */
    AADMA_DeviceHandler_t   devHandler,    /* Function to call when the DMA completes */
    void                    *userData      /* Pointer which will be passed to devHandler. */
)
{
    AADMA_DeviceAttribute_t   *devAttr;
    unsigned long            flags;

    if ( !IsDeviceValid( dev ))
    {
        return -ENODEV;
    }
    devAttr = &AADMA_gDeviceAttribute[ dev ];

    spin_lock_irqsave( &gAADmaDevLock, flags );

    devAttr->devHandler = devHandler;
    devAttr->userData   = userData;

    spin_unlock_irqrestore( &gAADmaDevLock, flags );

    return 0;
}
EXPORT_SYMBOL( aadma_set_device_handler );

#ifdef CONFIG_MACH_BCMHANA_CHIP_TEST
/****************************************************************************/
/**
*   Aquire/release driver lock for testing/debugging purposes
*   @return
*       lock status
*/
/****************************************************************************/

int aadma_lock( int aquire )
{
   static unsigned long flags;

   if ( aquire )
   {
      down( &gAADMA.lock );
      spin_lock_irqsave( &gHwAADmaLock, flags );
   }
   else
   {
      spin_unlock_irqrestore( &gHwAADmaLock, flags );
      up( &gAADMA.lock );
   }

   return 0;
}
EXPORT_SYMBOL( aadma_lock );
#endif /* CONFIG_MACH_BCMHANA_CHIP_TEST */
