/*****************************************************************************
*  Copyright 2011 Broadcom Corporation.  All rights reserved.
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2, available at
*  http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
*****************************************************************************/

/* ---- Include Files ---------------------------------------------------- */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/errno.h>
#include <linux/proc_fs.h>
#include <linux/sysctl.h>
#include <linux/device.h>
#include <linux/dmapool.h>
#include <linux/dma-mapping.h>
#include <linux/vmalloc.h>
#include <linux/platform_device.h>
#include <linux/semaphore.h>

#include <linux/broadcom/amxr_port.h>

/* ---- Private Constants and Types -------------------------------------- */

#define AAP_DMA_ALIGN_IN_BYTES         8
#define AAP_DMA_ALLOC_CROSS_RESTRICT   4096

#define AAP_MAX_SAMP_FREQ              48000
#define AAP_MAX_FRAME_PERIOD           5000
#define AAP_MAX_SAMP_WIDTH_BYTES       2
#define AAP_MAX_CHANNELS               2

#define AAP_DMA_BUFFER_SIZE_BYTES      (( AAP_MAX_SAMP_FREQ *        \
                                          AAP_MAX_FRAME_PERIOD *     \
                                          AAP_MAX_SAMP_WIDTH_BYTES * \
                                          AAP_MAX_CHANNELS) / 1000000)

#define AAP_MAX_DMA_BUF_SIZE_BYTES     (AAP_DMA_BUFFER_SIZE_BYTES * AAP_NUM_BUFFERS_PER_PIPE)

#define AAP_DEFAULT_FRAME_PERIOD       5000

#define AAP_NUM_BUFFERS_PER_PIPE       2

#define AAP_MAX_NUM_PIPES              32

/* STUB IN */
typedef uint32_t                       PIPE_MSG_ID_t;
typedef enum aap_msg_id
{
   AAP_CREATED = 0,
   AAP_DESTROYED,
   AAP_DEVICE_CHANGE,
   AAP_DEVICE_CHANGED,
   AAP_BUFFER_DONE,
   AAP_SYNC_EVENT,
   AAP_ERROR,
};

typedef enum aap_dir
{
   AAP_DIR_INGRESS = 0,
   AAP_DIR_EGRESS
};

/* DMA buffer structure */
struct aap_dma_data_buf
{
   void                *virt;       /* Virtual address pointer */
   dma_addr_t           phys;       /* Physical address */
};

struct aap_prop
{
   uint32_t                device_id;
   uint32_t                sampling_rate;
   uint32_t                num_ch;
   uint32_t                bits_per_sample;
   uint32_t                direction;
   uint32_t                frame_size;
   uint32_t                frame_size_reserve;
};

struct aap_stats
{
   uint32_t                transaction_count;
   uint32_t                num_errs;
};

struct aap_pipe_cfg
{
   atomic_t                active;

   struct aap_prop         aap_prop;
   struct aap_stats        aap_stats;

   AMXR_PORT_ID            amxr_port_id;
   atomic_t                active_idx;

   struct semaphore        pipe_lock;

   struct aap_dma_data_buf aap_buf[AAP_NUM_BUFFERS_PER_PIPE];
   struct aap_dma_data_buf aap_buf_reserve[AAP_NUM_BUFFERS_PER_PIPE];
};

struct aap_pipe_info
{
   struct aap_pipe_cfg  pipe_cfg[AAP_MAX_NUM_PIPES];
};

/* ---- Private Function Prototypes -------------------------------------- */

static int aapDmaInit( void );
static int aapCreatePipe(  PIPE_MSG_ID_t pipe_id, uint32_t device_id, uint32_t sampling_rate,
                           uint32_t num_ch, uint32_t bits_per_sample, uint32_t direction );

static int aapRemovePipe( PIPE_MSG_ID_t pipe_id );
static int aapPipeChangePending( PIPE_MSG_ID_t pipe_id, uint32_t device_id, uint32_t sampling_rate,
                                 uint32_t num_ch, uint32_t bits_per_sample );

static int aapPipeChangeComplete( PIPE_MSG_ID_t pipe_id, uint32_t device_id, uint32_t sampling_rate,
                                  uint32_t num_ch, uint32_t bits_per_sample );

static int aapBufferDone( PIPE_MSG_ID_t pipe_id, uint32_t buf_index );
static int aapSyncEvent( void );

static int aapError( PIPE_MSG_ID_t pipe_id, uint32_t err_code );

static void aapEgressFlush( void *privdata );
static int16_t *aapBufGetPtr( int numBytes, void *privdata );

/* ---- Private Variables ------------------------------------------------- */

/* DMA buffer pool */
static struct dma_pool        *gDmaPool;

static struct aap_pipe_info   gPipeInfo;

/* ---- Public Variables ------------------------------------------------- */
/* ---- Functions -------------------------------------------------------- */

static int aapCreatePipe(  PIPE_MSG_ID_t pipe_id, uint32_t device_id, uint32_t sampling_rate,
                           uint32_t num_ch, uint32_t bits_per_sample, uint32_t direction )
{
   int i;
   int rc = 0;
   struct aap_pipe_cfg *pipe_cfg;
   AMXR_PORT_CB amxr_cb;
   char pipe_name[32];

   pipe_cfg = &gPipeInfo.pipe_cfg[pipe_id];

   /* Check if pipe is active */
   if ( atomic_read(&pipe_cfg->active) )
   {
      printk( KERN_ERR "%s: [PIPE ID %u] Pipe already active\n", __FUNCTION__, pipe_id );
      return -EINVAL;
   }

   down( &pipe_cfg->pipe_lock );

   /* Copy over parameters */
   pipe_cfg->aap_prop.device_id = device_id;
   pipe_cfg->aap_prop.sampling_rate = sampling_rate;
   pipe_cfg->aap_prop.num_ch = num_ch;
   pipe_cfg->aap_prop.bits_per_sample = bits_per_sample;
   pipe_cfg->aap_prop.direction = direction;

   /* Calculate frame size */
   pipe_cfg->aap_prop.frame_size =                                      \
                           ((pipe_cfg->aap_prop.sampling_rate *         \
                           pipe_cfg->aap_prop.num_ch *                  \
                           (pipe_cfg->aap_prop.bits_per_sample / 8) *   \
                           AAP_DEFAULT_FRAME_PERIOD) / 1000000);

   if ( (pipe_cfg->aap_prop.frame_size * AAP_NUM_BUFFERS_PER_PIPE) > AAP_MAX_DMA_BUF_SIZE_BYTES )
   {
      rc = -ENOMEM;
      goto cleanup_exit;
   }

   pipe_cfg->aap_buf[0].virt = dma_pool_alloc( gDmaPool, GFP_KERNEL, &pipe_cfg->aap_buf[0].phys );

   if ( !pipe_cfg->aap_buf[0].virt )
   {
      up ( &pipe_cfg->pipe_lock );
      printk( KERN_ERR "%s: [PIPE ID %u] failed to allocate DMA buffers\n", __FUNCTION__, pipe_id );
      rc = -ENOMEM;
      goto cleanup_exit;
   }
   memset( pipe_cfg->aap_buf[0].virt, 0, AAP_MAX_DMA_BUF_SIZE_BYTES );

   for( i = 1; i < AAP_NUM_BUFFERS_PER_PIPE; i++ )
   {
      pipe_cfg->aap_buf[i].phys = pipe_cfg->aap_buf[0].phys + (pipe_cfg->aap_prop.frame_size * i);
      pipe_cfg->aap_buf[i].virt = pipe_cfg->aap_buf[0].virt + (pipe_cfg->aap_prop.frame_size * i);
   }

   /* Set AMXR port */
   if ( pipe_cfg->aap_prop.direction == AAP_DIR_EGRESS )
   {
      amxr_cb.dstcnxsremoved = aapEgressFlush;
      amxr_cb.getdst = aapBufGetPtr;
   }
   else
   {
      amxr_cb.getsrc = aapBufGetPtr;
   }

   sprintf( pipe_name, "aap.pipe%i", pipe_id );
   rc = amxrCreatePort( pipe_name, &amxr_cb, (void *)pipe_id,
                        ((pipe_cfg->aap_prop.direction == AAP_DIR_EGRESS) ? pipe_cfg->aap_prop.sampling_rate : 0),
                        ((pipe_cfg->aap_prop.direction == AAP_DIR_EGRESS) ? pipe_cfg->aap_prop.num_ch : 0),
                        ((pipe_cfg->aap_prop.direction == AAP_DIR_EGRESS) ? pipe_cfg->aap_prop.frame_size : 0),
                        ((pipe_cfg->aap_prop.direction == AAP_DIR_EGRESS) ? 0 : pipe_cfg->aap_prop.sampling_rate),
                        ((pipe_cfg->aap_prop.direction == AAP_DIR_EGRESS) ? 0 : pipe_cfg->aap_prop.num_ch),
                        ((pipe_cfg->aap_prop.direction == AAP_DIR_EGRESS) ? 0 : pipe_cfg->aap_prop.frame_size),
                        &pipe_cfg->amxr_port_id );

   up ( &pipe_cfg->pipe_lock );

   if( rc )
   {
      printk( KERN_ERR "%s: failed to create mixer port on pipe ID %i err=%i\n", __FUNCTION__, pipe_id, rc );
      return rc;
   }

   atomic_set( &pipe_cfg->active, 1);

#if 0
   /* Call to audp_set_buffer() here */
   audp_set_buffer( pipe_id, pipe_cfg->aap_buf[0].phys, (pipe_cfg->aap_prop.frame_size * AAP_NUM_BUFFERS_PER_PIPE) , AAP_NUM_BUFFERS_PER_PIPE );
#endif

   return rc;

cleanup_exit:

   aapRemovePipe( pipe_id );

   return rc;
}

static int aapRemovePipe( uint32_t pipe_id )
{
   int i;
   int rc = 0;
   struct aap_pipe_cfg *pipe_cfg;

   pipe_cfg = &gPipeInfo.pipe_cfg[pipe_id];

   down( &pipe_cfg->pipe_lock );

   /* Remove pipe from AMXR */
   rc = amxrRemovePort( pipe_cfg->amxr_port_id );

   if ( rc )
   {
      printk( KERN_ERR "AAP [pipe=%d]: failed to deregister mixer port err=%i\n", pipe_id, rc );
   }

   pipe_cfg->amxr_port_id = NULL; /* Invalidate handle */

   if ( pipe_cfg->aap_buf[0].virt )
   {
      dma_pool_free( gDmaPool, pipe_cfg->aap_buf[0].virt, pipe_cfg->aap_buf[0].phys );
   }

   if ( pipe_cfg->aap_buf[0].virt )
   {
      dma_pool_free( gDmaPool, pipe_cfg->aap_buf_reserve[0].virt, pipe_cfg->aap_buf_reserve[0].phys );
   }

   for ( i = 0; i < AAP_NUM_BUFFERS_PER_PIPE; i++ )
   {
      pipe_cfg->aap_buf[i].virt = NULL;
      pipe_cfg->aap_buf[i].phys = 0;
      pipe_cfg->aap_buf_reserve[i].virt = NULL;
      pipe_cfg->aap_buf_reserve[i].phys = 0;
   }

   atomic_set( &pipe_cfg->active, 0 );

   memset( &pipe_cfg->aap_prop, 0, sizeof(pipe_cfg->aap_prop) );
   memset( &pipe_cfg->aap_stats, 0, sizeof(pipe_cfg->aap_stats) );

   up( &pipe_cfg->pipe_lock );

   return rc;
}

static int aapPipeChangePending( PIPE_MSG_ID_t pipe_id, uint32_t device_id, uint32_t sampling_rate,
                                 uint32_t num_ch, uint32_t bits_per_sample )
{
   int i;
   int rc = 0;
   struct aap_pipe_cfg *pipe_cfg;
   uint32_t frame_size;

   pipe_cfg = &gPipeInfo.pipe_cfg[pipe_id];

   /* Check if pipe is active */
   if ( atomic_read( &pipe_cfg->active ) == 0 )
   {
      printk( KERN_ERR "%s: [PIPE ID %u] Pipe is no longer active\n", __FUNCTION__, pipe_id );
      return -EINVAL;
   }

   /* Calculate frame size */
   frame_size = ((sampling_rate * num_ch * (bits_per_sample / 8) * AAP_DEFAULT_FRAME_PERIOD) / 1000000);

   down( &pipe_cfg->pipe_lock );

   /* Check if we need to readjust buffers due to change in device */
   if ( frame_size != pipe_cfg->aap_prop.frame_size )
   {
      if ( (frame_size * AAP_NUM_BUFFERS_PER_PIPE) > AAP_MAX_DMA_BUF_SIZE_BYTES )
      {
         return -ENOMEM;
      }

      pipe_cfg->aap_buf_reserve[0].virt = dma_pool_alloc( gDmaPool, GFP_KERNEL, &pipe_cfg->aap_buf_reserve[0].phys );

      if ( !pipe_cfg->aap_buf_reserve[0].virt )
      {
         printk( KERN_ERR "%s: [PIPE ID %u] failed to allocate DMA buffers\n", __FUNCTION__, pipe_id );
         return -ENOMEM;
      }

      pipe_cfg->aap_prop.frame_size_reserve = frame_size;

      memset( pipe_cfg->aap_buf_reserve[0].virt, 0, AAP_MAX_DMA_BUF_SIZE_BYTES );

      for( i = 1; i < AAP_NUM_BUFFERS_PER_PIPE; i++ )
      {
         pipe_cfg->aap_buf_reserve[i].phys = pipe_cfg->aap_buf_reserve[0].phys + (pipe_cfg->aap_prop.frame_size_reserve * i);
         pipe_cfg->aap_buf_reserve[i].virt = pipe_cfg->aap_buf_reserve[0].virt + (pipe_cfg->aap_prop.frame_size_reserve * i);
      }

#if 0
      /* Call to set buffer with original settings */
      audp_set_buffer( pipe_id, pipe_cfg->aap_buf_reserve[0].phys, (pipe_cfg->aap_prop.frame_size_reserve * AAP_NUM_BUFFERS_PER_PIPE) , AAP_NUM_BUFFERS_PER_PIPE );
#endif
   }
   else
   {
#if 0
      /* Call to set buffer with original settings */
      audp_set_buffer( pipe_id, pipe_cfg->aap_buf[0].phys, (pipe_cfg->aap_prop.frame_size * AAP_NUM_BUFFERS_PER_PIPE) , AAP_NUM_BUFFERS_PER_PIPE );
#endif
   }

   up( &pipe_cfg->pipe_lock );

   return rc;
}

static int aapPipeChangeComplete( PIPE_MSG_ID_t pipe_id, uint32_t device_id, uint32_t sampling_rate,
                                  uint32_t num_ch, uint32_t bits_per_sample )
{
   int i;
   int rc = 0;
   struct aap_pipe_cfg *pipe_cfg;
   uint32_t frame_size;

   pipe_cfg = &gPipeInfo.pipe_cfg[pipe_id];

   /* Check if pipe is active */
   if ( atomic_read( &pipe_cfg->active ) == 0 )
   {
      printk( KERN_ERR "%s: [PIPE ID %u] Pipe is no longer active\n", __FUNCTION__, pipe_id );
      return -EINVAL;
   }

   /* Calculate frame size */
   frame_size = ((sampling_rate * num_ch * (bits_per_sample / 8) * AAP_DEFAULT_FRAME_PERIOD) / 1000000);

   down( &pipe_cfg->pipe_lock );

  /* Check if we need to readjust buffers due to change in device */
   if ( frame_size != pipe_cfg->aap_prop.frame_size )
   {
      /* Check for reserve buffer allocation */
      if ( pipe_cfg->aap_buf_reserve[0].virt && frame_size == pipe_cfg->aap_prop.frame_size_reserve  )
      {
         /* Deallocate current buffers */
         if ( pipe_cfg->aap_buf[0].virt )
         {
            dma_pool_free ( gDmaPool, pipe_cfg->aap_buf[0].virt, pipe_cfg->aap_buf[0].phys );
         }

         /* Switch over buffers */
         for ( i = 0; i < AAP_NUM_BUFFERS_PER_PIPE; i++ )
         {
            pipe_cfg->aap_buf[i].phys = pipe_cfg->aap_buf_reserve[i].phys;
            pipe_cfg->aap_buf[i].virt = pipe_cfg->aap_buf_reserve[i].virt;

            pipe_cfg->aap_buf_reserve[i].phys = 0;
            pipe_cfg->aap_buf_reserve[i].virt = 0;
         }

         pipe_cfg->aap_prop.frame_size = frame_size;
         pipe_cfg->aap_prop.frame_size_reserve = 0;
      }
      else
      {
         up( &pipe_cfg->pipe_lock );
         printk( KERN_ERR "%s: [PIPE ID %u] Pipe unable to switch to newly sized buffers\n", __FUNCTION__, pipe_id );
         return -ENOMEM;
      }
   }

   /* Copy over new properties */
   pipe_cfg->aap_prop.device_id = device_id;
   pipe_cfg->aap_prop.bits_per_sample = bits_per_sample;    /* AMXR can only work with 16 bit samples */

   if ( sampling_rate != pipe_cfg->aap_prop.sampling_rate )
   {
      pipe_cfg->aap_prop.sampling_rate = sampling_rate;
      if ( pipe_cfg->aap_prop.direction == AAP_DIR_EGRESS )
      {
         rc = amxrSetPortFreq( pipe_cfg->amxr_port_id, pipe_cfg->aap_prop.sampling_rate, pipe_cfg->aap_prop.frame_size, 0, 0 );
      }
      else
      {
         rc = amxrSetPortFreq( pipe_cfg->amxr_port_id, 0, 0, pipe_cfg->aap_prop.sampling_rate, pipe_cfg->aap_prop.frame_size );
      }
   }

   if ( num_ch != pipe_cfg->aap_prop.num_ch )
   {
      pipe_cfg->aap_prop.num_ch = num_ch;
      if ( pipe_cfg->aap_prop.direction == AAP_DIR_EGRESS )
      {
         rc = amxrSetPortDstChannels( pipe_cfg->amxr_port_id, pipe_cfg->aap_prop.num_ch, pipe_cfg->aap_prop.frame_size );
      }
      else
      {
         rc = amxrSetPortSrcChannels( pipe_cfg->amxr_port_id, pipe_cfg->aap_prop.num_ch, pipe_cfg->aap_prop.frame_size );
      }
   }

   up( &pipe_cfg->pipe_lock );

   return rc;
}

static int aapBufferDone( uint32_t pipe_id, uint32_t buf_index )
{
   struct aap_pipe_cfg *pipe_cfg;

   pipe_cfg = &gPipeInfo.pipe_cfg[pipe_id];

   /* Set the active index to the ready buffer index */
   atomic_set( &pipe_cfg->active_idx, buf_index );
   pipe_cfg->aap_stats.transaction_count++;

   return 0;
}

static int aapSyncEvent( void )
{
   amxrElapsedTime( AAP_DEFAULT_FRAME_PERIOD );

   return 0;
}

static int aapError( PIPE_MSG_ID_t pipe_id, uint32_t err_code )
{
   int rc = 0;
   struct aap_pipe_cfg *pipe_cfg;

   pipe_cfg = &gPipeInfo.pipe_cfg[pipe_id];

   pipe_cfg->aap_stats.num_errs++;

   printk( KERN_ERR "%s: [PIPE ID %u] Pipe is reporting error code %d\n", __FUNCTION__, pipe_id, err_code );

   return rc;
}

static void aapEgressFlush( void *privdata )
{
   int i;
   uint32_t pipe_id;
   struct aap_pipe_cfg *pipe_cfg;

   pipe_id = (uint32_t)privdata;

   pipe_cfg = &gPipeInfo.pipe_cfg[pipe_id];

   down( &pipe_cfg->pipe_lock );

   for ( i = 0; i < AAP_NUM_BUFFERS_PER_PIPE; i++ )
   {
      if ( pipe_cfg->aap_buf[i].virt )
      {
         memset( pipe_cfg->aap_buf[i].virt, 0, AAP_MAX_DMA_BUF_SIZE_BYTES );
      }
   }

   up( &pipe_cfg->pipe_lock );
}

static int16_t *aapBufGetPtr( int numBytes, void *privdata )
{
   int active_idx;
   int16_t *ptr;
   uint32_t pipe_id;
   struct aap_pipe_cfg *pipe_cfg;

   pipe_id = (uint32_t)privdata;
   ptr = NULL;

   pipe_cfg = &gPipeInfo.pipe_cfg[pipe_id];

   active_idx = atomic_read( &pipe_cfg->active_idx );

   if ( numBytes == pipe_cfg->aap_prop.frame_size )
   {
      ptr = pipe_cfg->aap_buf[active_idx].virt;
   }

   return ptr;
}

static int aapMsgHandler(  PIPE_MSG_ID_t msg_id, uint32_t pipe_id, uint32_t parm1,
                           uint32_t parm2, uint32_t parm3, uint32_t parm4, uint32_t parm5 )
{
   int rc = 0;

   switch ( msg_id )
   {
      case AAP_CREATED:
         rc = aapCreatePipe( pipe_id, parm1, parm2, parm3, parm4, parm5 );
         break;
      case AAP_DESTROYED:
         rc = aapRemovePipe( pipe_id );
         break;
      case AAP_DEVICE_CHANGE:
         rc = aapPipeChangePending( pipe_id, parm1, parm2, parm3, parm4 );
         break;
      case AAP_DEVICE_CHANGED:
         rc = aapPipeChangeComplete( pipe_id, parm1, parm2, parm3, parm4 );
         break;
      case AAP_BUFFER_DONE:
         rc = aapBufferDone( pipe_id, parm1 );
         break;
      case AAP_SYNC_EVENT:
         rc = aapSyncEvent();
         break;
      case AAP_ERROR:
         rc = aapError( pipe_id, parm1 );
         break;
      default:
         rc = -EINVAL;
         break;
   };

   return rc;
}

static int aapDmaInit( void )
{
   int rc = 0;

   gDmaPool = dma_pool_create("AMXR Audio Pipe DMA memory pool", NULL,
                              AAP_MAX_DMA_BUF_SIZE_BYTES, AAP_DMA_ALIGN_IN_BYTES,
                              AAP_DMA_ALLOC_CROSS_RESTRICT );

   if( gDmaPool == NULL )
   {
      printk( KERN_ERR "AAP: failed to allocate DMA buffer pool\n" );
      return -ENOMEM;
   }


   return rc;
}

static int aapDmaTerm( void )
{
   int i, j;
   struct aap_pipe_cfg *pipe_cfg;

   pipe_cfg = gPipeInfo.pipe_cfg;

   down( &pipe_cfg->pipe_lock );

   for ( i = 0; i < AAP_MAX_NUM_PIPES; i++, pipe_cfg++ )
   {
      if ( pipe_cfg->aap_buf[0].virt )
      {
         dma_pool_free ( gDmaPool, pipe_cfg->aap_buf[0].virt, pipe_cfg->aap_buf[0].phys );
      }

      if ( pipe_cfg->aap_buf_reserve[0].virt )
      {
         dma_pool_free ( gDmaPool, pipe_cfg->aap_buf_reserve[0].virt, pipe_cfg->aap_buf_reserve[0].phys );
      }

      for ( j = 0; j < AAP_NUM_BUFFERS_PER_PIPE; j++ )
      {
         pipe_cfg->aap_buf[i].virt = NULL;
         pipe_cfg->aap_buf[i].phys = 0;
         pipe_cfg->aap_buf_reserve[i].virt = NULL;
         pipe_cfg->aap_buf_reserve[i].phys = 0;
      }
   }
   up( &pipe_cfg->pipe_lock );

   dma_pool_destroy( gDmaPool );

   return 0;
}

static int __init aap_init( void )
{
   int err = 0;
   int i;
   struct aap_pipe_cfg *pipe_cfg;

   aapDmaInit();

   pipe_cfg = gPipeInfo.pipe_cfg;

   for ( i = 0; i < AAP_MAX_NUM_PIPES; i++, pipe_cfg++ )
   {
      sema_init( &pipe_cfg->pipe_lock, 1 );

      atomic_set( &pipe_cfg->active, 0);

      memset( &pipe_cfg->aap_prop, 0, sizeof(pipe_cfg->aap_prop) );
      memset( &pipe_cfg->aap_stats, 0, sizeof(pipe_cfg->aap_stats) );
   }

#if 0
   /* Register callback functions to audp here */
   audp_register( aapMsgHandler );
#endif

   return err;
}

static void __exit aap_exit( void )
{
   aapDmaTerm();

#if 0
   /* Register callback functions to audp here */
   audp_deregister();
#endif

}

module_init( aap_init );
module_exit( aap_exit );
MODULE_AUTHOR( "Broadcom" );
MODULE_DESCRIPTION( "AMXR Audio Pipe (AAP)" );
MODULE_LICENSE( "GPL" );
