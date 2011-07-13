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
#include <linux/version.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/platform_device.h>
#include <linux/proc_fs.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/vmalloc.h>
#include <linux/semaphore.h>

#include <linux/broadcom/amxr.h>
#include <linux/broadcom/aaa_ioctl.h>
#include <linux/broadcom/bsc.h>
#include <linux/broadcom/halaudio.h>
#include <linux/broadcom/halaudio_cfg.h>
#include <linux/broadcom/halaudio_lib.h>
#include <linux/broadcom/bcm_major.h>

#include <asm/uaccess.h>

/* ---- Private Constants and Types -------------------------------------- */

// #define AAA_RUNS_AT_44_1_KHz

#if defined(AAA_RUNS_AT_44_1_KHz)
#define AAA_PORT_SRC_HZ          44100
#define AAA_PORT_SRC_FRAME       882
#else
#define AAA_PORT_SRC_HZ          48000
#define AAA_PORT_SRC_FRAME       (240 * 2)
#endif /* defined(AAA_RUNS_AT_44_1_KHz) */
#define AAA_PORT_SRC_CHAN        2
#define AAA_PORT_DST_HZ          16000
#define AAA_PORT_DST_FRAME       (80 * 2)
#define AAA_PORT_DST_CHAN        1
#define AAA_DEVICE_NAME          "aaa"
#define AAA_AUDIO_DIG_GAIN       0
#define AAA_AUDIO_ANA_GAIN       30
#define AAA_PORT_NAME            "aaa.grade-beef"
#define AAA_NUM_READ_BUF         8 

/**
*  Union of all the different IOCTL parameter structures to determine
*  max stack variable size
*/
union aaa_ioctl_params
{
   struct aaa_ioctl_setwrtcodec              setwrtcodec;
   struct aaa_ioctl_setrdcodec               setrdcodec;
};

typedef struct aaa_read_buf
{
   int8_t   buf[AAA_NUM_READ_BUF][AAA_PORT_DST_FRAME];
                        /* Circular read buffer */
   int      wrtidx;     /* Current write buffer index */
   atomic_t rdidx;      /* Current read buffer index */
   int      reset;      /* Set when the read index should be reset on next read */
   int      pending;    /* Set when there is a pending read (aaa_read blocked) */
   int      flush;      /* Set when the read buffer is flushed and aaa_read
                        ** should return immediately.
                        */
} AAA_READ_BUF;

/**
* Debug traces
*/
#define AAA_ENABLE_KNLLOG        0
#if AAA_ENABLE_KNLLOG
#include <linux/broadcom/knllog.h>
#define AAA_KNLLOG               KNLLOG
#else
#define AAA_KNLLOG(...)
#endif

/* ---- Private Function Prototypes -------------------------------------- */

#if defined(AAA_RUNS_AT_44_1_KHz)
static int     aaa_441KHz_setup( void );
static int     aaa_441KHz_gain_reset( void );
#endif /* defined(AAA_RUNS_AT_44_1_KHz) */

static int     aaa_open( struct inode *inode, struct file *file );
static int     aaa_release( struct inode *inode, struct file *file );
static ssize_t aaa_read( struct file *file, char __user *buffer, size_t count, loff_t *ppos );
static ssize_t aaa_write( struct file *file, const char __user *buffer, size_t count, loff_t *ppos );
static long    aaa_ioctl(struct file *file, unsigned int cmd, unsigned long arg );

static int     aaa_probe( struct platform_device *pdev );
static int     aaa_remove( struct platform_device *pdev );

static int16_t *aaa_amxr_getsrc(
      int   bytes,                     /**< (i) size of the buffer in bytes */
      void *privdata                   /**< (i) user supplied data */
   );
static void    aaa_amxr_srcdone(
      int   bytes,                     /**< (i) size of the buffer in bytes */
      void *privdata                   /**< (i) user supplied data */
   );

static int16_t *aaa_amxr_getdst(
      int   bytes,                     /**< (i) size of the buffer in bytes */
      void *privdata                   /**< (i) user supplied data */
   );

static void    aaa_amxr_dstdone(
      int   bytes,                     /**< (i) size of the buffer in bytes */
      void *privdata                   /**< (i) user supplied data */
   );

static void    aaa_amxr_dstcnxsremoved(
      void *privdata                   /**< (i) user supplied data */
   );

/* ---- Private Variables ------------------------------------------------- */

static AMXR_HDL           g_amxr_client;
static HALAUDIO_HDL       g_halaudio_client = NULL;
static AMXR_PORT_ID       g_aaa_port;
static HALAUDIO_WRITE     g_aaa_write_state;
static uint8_t            *g_aaa_wrap_buf;
static HALAUDIO_DEV_CFG   g_hal_device;
static HALAUDIO_DEV_CFG   g_hal_wrt_device;
static HALAUDIO_DEV_CFG   g_hal_rd_device;
static AMXR_PORT_INFO     g_amxr_port_info;
static AAA_READ_BUF       g_aaa_read_buf;
struct semaphore          g_rd_lock;

static AMXR_PORT_CB  g_amxr_callback =
{
   .getsrc         = aaa_amxr_getsrc,
   .srcdone        = aaa_amxr_srcdone,
   .getdst         = aaa_amxr_getdst,
   .dstdone        = aaa_amxr_dstdone,
   .dstcnxsremoved = aaa_amxr_dstcnxsremoved,
};


#ifdef CONFIG_SYSFS
static struct class  * aaa_class;
static struct device * aaa_dev;
#endif

/* File Operations (these are the device driver entry points) */
static struct file_operations gfops =
{
   .owner          = THIS_MODULE,
   .open           = aaa_open,
   .release        = aaa_release,
   .read           = aaa_read,
   .write          = aaa_write,
   .unlocked_ioctl = aaa_ioctl,
};


/* ---- Public Variables ------------------------------------------------- */

/* ---- Functions -------------------------------------------------------- */

#if defined(AAA_RUNS_AT_44_1_KHz)
/***************************************************************************/
/**
*  Driver gains resets for 44.1 KHz audio.
*
*  @return
*     0        Success
*     <= 0     Error
*/
static int aaa_441KHz_gain_reset( void )
{
   /* 'handsfree-spkr' (right) */
   halAudioSetGain ( g_halaudio_client,
                     HALAUDIO_BLOCK_DIG_ID( HALAUDIO_CODEC4 + 1, HALAUDIO_DIR_DAC ),
                     HALAUDIO_GAIN_MUTE );

   /* 'handsfree-spkr' (left) */
   halAudioSetGain ( g_halaudio_client,
                     HALAUDIO_BLOCK_DIG_ID( HALAUDIO_CODEC5 + 1, HALAUDIO_DIR_DAC ),
                     HALAUDIO_GAIN_MUTE );

   /* 'headset-spkr' */
   halAudioSetGain ( g_halaudio_client,
                     HALAUDIO_BLOCK_DIG_ID( HALAUDIO_CODEC6 + 1, HALAUDIO_DIR_DAC ),
                     HALAUDIO_GAIN_MUTE );

   return 0;
}


/***************************************************************************/
/**
*  Driver setup for 44.1 KHz audio.
*
*  @return
*     0        Success
*     <= 0     Error
*/
static int aaa_441KHz_setup( void )
{
   AMXR_PORT_ID portid;

   /* Native 44.1 KHz audio support.
   **
   ** At the present time, we always setup 44.1 KHz audio channel,
   ** that is because Android likes to do so and we can also support
   ** it, however there are some caveat on how this should work.
   **
   ** All audio goes through the audioh6 channel, therefore aaa.grade-beef
   ** is always connected to that port.  Other potential user (such as
   ** audioh5) need to be configured for 44.1 KHz as well.
   */
   halAudioSetFreq ( g_halaudio_client,
                     HALAUDIO_CODEC6,     /* Maps to 'headset-spkr' */
                     AAA_PORT_SRC_HZ );

   halAudioSetFreq ( g_halaudio_client,
                     HALAUDIO_CODEC5,     /* Maps to 'handsfree-spkr' (left) */
                     AAA_PORT_SRC_HZ );

   halAudioSetFreq ( g_halaudio_client,
                     HALAUDIO_CODEC4,     /* Maps to 'handsfree-spkr' (right) */
                     AAA_PORT_SRC_HZ );

   bsc_query( "headset-spkr",
              &g_hal_device,
              sizeof ( g_hal_device ) );

   amxrGetPortInfoByName( g_amxr_client,
                          g_hal_device.info.mport_name,
                          &g_amxr_port_info );

   amxrQueryPortByName( g_amxr_client,
                        g_amxr_port_info.name,
                        &portid );

   amxrConnect( g_amxr_client,
                g_aaa_port,
                portid,
                AMXR_CONNECT_STEREO2STEREO );

   return 0;
}
#endif /* defined(AAA_RUNS_AT_44_1_KHz) */


/***************************************************************************/
/**
*  Driver callback to obtain source data pointer 
*
*  @return
*     <>0      Source data pointer 
*     NULL     No data to read
*/
static int16_t *aaa_amxr_getsrc(
      int   bytes,                     /**< (i) size of the buffer in bytes */
      void *privdata                   /**< (i) user supplied data */
   )
{
   int16_t *ptr = NULL;
   HALAUDIO_WRITE *writestp = privdata;

   if ( writestp && writestp->request )
   {
      HALAUDIO_RW_BUF *curbuf, *otherbuf;
      int len;

      curbuf = &writestp->buf[writestp->curr_bufidx];
      otherbuf = &writestp->buf[writestp->curr_bufidx ^ 1];

      if ( curbuf->remain_bytes < bytes )
      {
         /* frame wrap */
         len = bytes - curbuf->remain_bytes;

         memset( g_aaa_wrap_buf + curbuf->remain_bytes, 0, len );
         memcpy( g_aaa_wrap_buf, curbuf->posp, curbuf->remain_bytes );
         if ( otherbuf->posp && otherbuf->remain_bytes > 0 )
         {
            memcpy( g_aaa_wrap_buf + curbuf->remain_bytes, otherbuf->posp,
                  otherbuf->remain_bytes < len ? otherbuf->remain_bytes : len );
         }
         ptr = (int16_t *)g_aaa_wrap_buf;
      }
      else
      {
         ptr = (int16_t *)curbuf->posp;
      }
   }

   AAA_KNLLOG( "out: request=%i bytes=%i bufidx=%i remain=%i ptr=0x%x", 
         writestp->request, bytes, writestp->curr_bufidx,
         writestp->buf[writestp->curr_bufidx].remain_bytes, ptr );

   return ptr;
}


/***************************************************************************/
/**
*  Driver callback to indicate source data pointer done
*/
static void    aaa_amxr_srcdone(
      int   bytes,                     /**< (i) size of the buffer in bytes */
      void *privdata                   /**< (i) user supplied data */
   )
{
   HALAUDIO_WRITE *writestp = privdata;

   AAA_KNLLOG( "out: request=%i bytes=%i bufidx=%i remain=%i", 
         writestp->request, bytes, writestp->curr_bufidx,
         writestp->buf[writestp->curr_bufidx].remain_bytes );

   if ( writestp && writestp->request )
   {
      HALAUDIO_RW_BUF  *curbuf, *otherbuf, *tmpbuf;
      int               len;

      curbuf         = &writestp->buf[writestp->curr_bufidx];
      otherbuf       = &writestp->buf[writestp->curr_bufidx ^ 1];

      len = bytes;
      if ( curbuf->remain_bytes < bytes )
      {
         len = bytes - curbuf->remain_bytes;
         curbuf->remain_bytes = 0;

         AAA_KNLLOG( "Swap buffers" );
         tmpbuf   = curbuf;      /* Swap double buffers */
         curbuf   = otherbuf;
         otherbuf = tmpbuf; 
         writestp->curr_bufidx ^= 1;
      }

      curbuf->posp         += len;
      curbuf->remain_bytes -= len;
      if ( curbuf->remain_bytes < 0 )
      {
         curbuf->remain_bytes = 0;
      }

      /* Low on samples, ask for more */
      if ( writestp->usercb && otherbuf->remain_bytes == 0 )
      {
         AAA_KNLLOG( "CB triggered" );
         writestp->usercb( 0, writestp->userdata );
         writestp->usercb = NULL; /* Clear callback to limit calling to once */
      }

      /* End condition */
      if ( curbuf->remain_bytes <= 0 && --writestp->flush_count == 0 )
      {
         AAA_KNLLOG( "Reset request" );
         writestp->request = 0;
      }
   }
}


/***************************************************************************/
/**
*  Driver callback to obtain destination data pointer 
*
*  @return
*     <>0      Destination data pointer 
*     NULL     No data to read
*/
static int16_t *aaa_amxr_getdst(
      int   bytes,                     /**< (i) size of the buffer in bytes */
      void *privdata                   /**< (i) user supplied data */
   )
{
   return ( int16_t * ) g_aaa_read_buf.buf[g_aaa_read_buf.wrtidx];
}


/***************************************************************************/
/**
*  Driver callback to indicate destination data pointer done
*/
static void    aaa_amxr_dstdone(
      int   bytes,                     /**< (i) size of the buffer in bytes */
      void *privdata                   /**< (i) user supplied data */
   )
{
   AAA_READ_BUF   *bufp = &g_aaa_read_buf;
   int             len = bytes;

   do
   {
      // AAA_KNLLOG( "in: bytes=%i/%i wrtidx=%i", len, bytes, bufp->wrtidx );

      /* Increment write buffer index now that the write is done.
      */
      bufp->wrtidx++;
      bufp->wrtidx %= AAA_NUM_READ_BUF;

      /* Read and write indexes wrap around - Client has not been reading.  Set the
      ** reset flag so that the read index resets on next read.
      */
      if ( !bufp->reset && bufp->wrtidx == atomic_read( &bufp->rdidx ) )
      {
         bufp->reset = 1;
      }

      len -= AAA_PORT_DST_FRAME;
   } while ( len > 0 );

   /* If we have a pending read, unblock now that we have new data.
   */
   if ( bufp->pending )
   {
      bufp->pending = 0;
      up( &g_rd_lock );
   }
}


/***************************************************************************/
/**
*  Driver callback to indicate when the last destination connection to the
*  port is removed.
*/
static void    aaa_amxr_dstcnxsremoved(
      void *privdata                   /**< (i) user supplied data */
   )
{
   AAA_READ_BUF   *bufp = &g_aaa_read_buf;

   AAA_KNLLOG( "Flushing read buffer: wrtidx=%i\n", bufp->wrtidx );

   /* Flush the read buffer.
   */
   bufp->reset = 0;
   bufp->wrtidx = 0;
   atomic_set( &bufp->rdidx, 0 );

   /* We have a pending read, purge the current read.
   */
   if ( bufp->pending )
   {
      bufp->pending = 0;
      bufp->flush = 1;
      up( &g_rd_lock );
   }
}


/***************************************************************************/
/**
*  Driver open routine
*
*  @return
*     0        Success
*     -ENOMEM  Insufficient memory
*/
static int aaa_open(
   struct inode *inode,             /**< (io) Pointer to inode info */
   struct file  *file               /**< (io) File structure pointer */
)
{
   /*
   ** Android is very sensitive to timing of this function call, which will
   ** dictate whether or not AudioFlinger service gets restarted properly. To
   ** keep timing more or less consistent between AudioFlinger service
   ** restarts (and hence repeated calls to this function), we should free
   ** the halaudio client handle and reallocate it. Once the problem in
   ** Android is addressed, we can remove this inefficiency.
   */
   int retval;
   if ( g_halaudio_client != NULL )
   {
      retval = halAudioFreeClient( g_halaudio_client );
      if (retval != 0)
      {
         /* Failed to free halaudio client */
         return retval;
      }

      g_halaudio_client = NULL;
   }

   if ( g_halaudio_client == NULL )
   {
      g_halaudio_client = halAudioAllocateClient();
      if ( g_halaudio_client == NULL )
      {
         return -ENOMEM;
      }

      /* We are going to be using this driver for audio in Android,
      ** we set the full power.
      */
      halAudioSetPower ( g_halaudio_client,
                         HALAUDIO_POWER_FULL_POWER );

#if defined(AAA_RUNS_AT_44_1_KHz)
      /* Setup the connection as needed.
      */
      aaa_441KHz_setup();
#endif /* defined(AAA_RUNS_AT_44_1_KHz) */
   }
   return 0;
}

/***************************************************************************/
/**
*  Driver release routine
*
*  @return
*     0        Success
*     -ve      Error code
*/
static int aaa_release(
   struct inode *inode,             /**< (io) Pointer to inode info */
   struct file  *file               /**< (io) File structure pointer */
)
{
   if ( g_halaudio_client != NULL )
   {
      halAudioSetPower ( g_halaudio_client,
                         HALAUDIO_POWER_DEEP_SLEEP );

      halAudioFreeClient( g_halaudio_client );
      g_halaudio_client = NULL;
   }

   return 0;
}

/***************************************************************************/
/**
*  Driver read method
*
*  @return
*     >= 0           Number of bytes read
*     -ve            Error code
*/
static ssize_t aaa_read(
   struct file *file,
   char __user *buffer,
   size_t count,
   loff_t *ppos
)
{
   int            rc, i, numframes, new_rdidx;
   AAA_READ_BUF  *bufp = &g_aaa_read_buf;

   AAA_KNLLOG( "in: count=%i", count );

   /* It is assumed that the client is setup to always read multiples of
   ** AAA_PORT_DST_FRAME.  If not, return an error immediately.
   */
   if ( ( count % AAA_PORT_DST_FRAME ) != 0 )
   {
      printk( KERN_ERR "%s: read count (%i) not multiples of AAA read frame (%i)\n",
              __FUNCTION__, count, AAA_PORT_DST_FRAME );

      return -EIO;
   }

   /* Sanity check */
   if ( count == 0 )
   {
      return 0;
   }

   /* Number of frames to read */
   numframes = count / AAA_PORT_DST_FRAME;

   if ( bufp->reset )
   {
      /* This is a fresh read.  Restart the read index to an older frame.
      */
      bufp->reset = 0;
      atomic_set( &bufp->rdidx, ( bufp->wrtidx + numframes ) % AAA_NUM_READ_BUF );
      AAA_KNLLOG( "in: read index reset" );
   }

   for ( i = 0; i < numframes; i++ )
   {
      if ( bufp->wrtidx == atomic_read( &bufp->rdidx ) )
      {
         /* Client is reading faster than what is available.  Block until data is
         ** available.
         */

         /* Indicates that there is a pending read.  Note it is important that the
         ** semaphore is initialized before the pending flag is set as g_rd_lock may
         ** be accessed as soon as the pending flag is set.
         */
         sema_init( &g_rd_lock, 0 );
         bufp->pending = 1;

         /* Block now */
         rc = down_interruptible( &g_rd_lock );
         if ( rc )
         {
              printk( KERN_ERR "%s: down_interruptible failed on g_rd_lock\n",
              __FUNCTION__);

            return rc;
         }

         /* Read buffer is flushed - return immediately.
         */
         if ( bufp->flush )
         {
            bufp->flush = 0;
            return ( i * AAA_PORT_DST_FRAME );
         }
      }

      /* Copy a frame to the user supplied buffer */
      if (copy_to_user( &buffer[ i * AAA_PORT_DST_FRAME ],
                    bufp->buf[ atomic_read( &bufp->rdidx ) ],
                    AAA_PORT_DST_FRAME ) != 0)
      {
            printk(KERN_ERR "AAA: copy_to_user failed\n");
            return -EFAULT;
      }

      AAA_KNLLOG( "in: copied %i, rdidx=%i",
                  i,
                  atomic_read( &bufp->rdidx ) );

      /* Update read index */
      new_rdidx = ( atomic_read( &bufp->rdidx ) + 1 ) % AAA_NUM_READ_BUF;
      atomic_set( &bufp->rdidx, new_rdidx );
   }

   return count;
}


/***************************************************************************/
/**
*  Callback to indicate when write is complete and more samples are needed
*/
static void aaa_write_done( int bytes,
                            void *userdata )
{
   struct semaphore *semp = userdata;
   (void)bytes;
   up( semp );
}


/***************************************************************************/
/**
*  Driver write method.
*
*  @return
*     >= 0           Number of bytes write
*     -ve            Error code
*/
static ssize_t aaa_write( struct file *file,
                          const char __user *buffer,
                          size_t count,
                          loff_t *ppos )
{
   int                    rc;
   char                  *kbuf;
   struct semaphore       wrt_lock;

   kbuf = vmalloc( count );
   if ( !kbuf )
   {
      return -ENOMEM;
   }

   sema_init( &wrt_lock, 0 );

   if (( rc = copy_from_user( kbuf, buffer, count )) < 0 )
   {
      count = -EIO;
      goto backout;
   }

   AAA_KNLLOG( "write count=%i", count );

   rc = halAudioWriteRequest( &g_aaa_write_state,
                              count,
                              kbuf,
                              aaa_write_done,
                              &wrt_lock );
   if ( rc )
   {
      count = rc;
      goto backout;
   }

   /* Wait until we are kicked out of there...
    */
   rc = down_interruptible( &wrt_lock );
   if ( rc )
   {
      count = rc;
      printk( KERN_ERR "%s: down_interruptible failed on wrt_lock\n", __FUNCTION__);
      goto backout;
   }

   AAA_KNLLOG( "done" );

backout:
   vfree( kbuf );
   return count;
}

/***************************************************************************/
/**
*  Driver ioctl method to support user library API.
*
*  @return
*     >= 0           Number of bytes write
*     -ve            Error code
*/
static long aaa_ioctl(
   struct file   *file,             /**< (io) File structure pointer */
   unsigned int   cmd,              /**< (i)  IOCTL command */
   unsigned long  arg               /**< (i)  User argument */
)
{
   int                                 rc;
   union aaa_ioctl_params              parm;
   unsigned int                        cmdnr, size;
   HALAUDIO_CODEC                      codec;
   AMXR_PORT_ID                        portid;

   cmdnr    = _IOC_NR( cmd );
   size     = _IOC_SIZE( cmd );

   if ( size > sizeof(parm) )
   {
      return -ENOMEM;
   }

   if ( size )
   {
      if (( rc = copy_from_user( &parm, (void *)arg, size )) != 0 )
      {
         return rc;
      }
   }

   rc       = -EPERM;

   switch ( cmdnr )
   {                 
      case AAA_CMD_SET_WRT_CODEC:
      {
         if ( strcmp( g_hal_wrt_device.name, parm.setwrtcodec.name ) == 0 )
         {
            /* Do nothing if the device has already been connected.
            */
            rc = 0;
            break;
         }

         memset ( &g_hal_device,
                  0,
                  sizeof (g_hal_device) );

         if ( bsc_query( parm.setwrtcodec.name,
                         &g_hal_device,
                         sizeof ( g_hal_device ) ) == 0 )
         {
#if defined(AAA_RUNS_AT_44_1_KHz)
            /* Reset gains on all codecs we use.
            */
            aaa_441KHz_gain_reset();
#else
            /* Remove existing connection.
            */
            if ( g_hal_wrt_device.name[0] != 0 )
            {
               amxrDisconnectAllByName ( g_amxr_client,
                                         AAA_PORT_NAME,
                                         g_hal_wrt_device.info.mport_name );

               /* If speaker is currently used, disconnect the right speaker which
               ** goes through "audioh4".
               */
               if ( strcmp( g_hal_wrt_device.name, "handsfree-spkr" ) == 0 )
               {
                  amxrDisconnectAllByName ( g_amxr_client,
                                            AAA_PORT_NAME,
                                            "halaudio.audioh4" );
               }
            }
#endif /* defined(AAA_RUNS_AT_44_1_KHz) */

            memcpy( &g_hal_wrt_device,
                    &g_hal_device,
                    sizeof(g_hal_wrt_device) );

#if !defined(AAA_RUNS_AT_44_1_KHz)
            /* Connect the ports together.
            */
            amxrGetPortInfoByName( g_amxr_client,
                                   g_hal_device.info.mport_name,
                                   &g_amxr_port_info );

            amxrQueryPortByName( g_amxr_client,
                                 g_amxr_port_info.name,
                                 &portid );

            if ( g_amxr_port_info.dst_chans == 2 )
            {
               /* Setup the stereo channels */
               amxrConnect( g_amxr_client,
                            g_aaa_port,
                            portid,
                            AMXR_CONNECT_STEREO2STEREO );
            }
            else
            {
               /* Setup the left channel */
               amxrConnect( g_amxr_client,
                            g_aaa_port,
                            portid,
                            AMXR_CONNECT_STEREOSPLITL );

               /* Setup the right speaker (handsfree) channel which goes through
               ** "audioh4".
               */
               if ( strcmp( g_hal_device.name, "handsfree-spkr" ) == 0 )
               {
                  amxrQueryPortByName( g_amxr_client,
                                       "halaudio.audioh4",
                                       &portid );

                  amxrConnect( g_amxr_client,
                               g_aaa_port,
                               portid,
                               AMXR_CONNECT_STEREOSPLITR );
               }
            }
#endif /* !defined(AAA_RUNS_AT_44_1_KHz) */

            /* Setup gain on codec of interest for audio output.
            */
            halAudioQueryCodecByName ( g_halaudio_client, 
                                       g_hal_device.info.codec_name,
                                       &codec );

            halAudioSetGain ( g_halaudio_client,
                              HALAUDIO_BLOCK_DIG_ID( codec, HALAUDIO_DIR_DAC ),
                              AAA_AUDIO_DIG_GAIN );

            /* Setup gain on codec of the right speaker (handsfree) channel for
            ** audio output.
            */
            if ( strcmp( g_hal_device.name, "handsfree-spkr" ) == 0 )
            {
               /* Maps to 'handsfree-spkr' (right) */
               halAudioQueryCodecByName ( g_halaudio_client,
                                          "AUDIOH CH4",
                                          &codec );

               halAudioSetGain ( g_halaudio_client,
                                 HALAUDIO_BLOCK_DIG_ID( codec,
                                                        HALAUDIO_DIR_DAC ),
                                 AAA_AUDIO_DIG_GAIN );
            }

            rc = 0;
         }
      }
      break;

      case AAA_CMD_SET_RD_CODEC:
      {
         if ( strcmp( g_hal_rd_device.name, parm.setrdcodec.name ) == 0 )
         {
            /* Do nothing if the device has already been connected.
            */
            rc = 0;
            break;
         }

         memset ( &g_hal_device,
                  0,
                  sizeof (g_hal_device) );

         if ( bsc_query( parm.setrdcodec.name,
                         &g_hal_device,
                         sizeof ( g_hal_device ) ) == 0 )
         {
            /* Remove existing connection.
            */
            if ( g_hal_rd_device.name[0] != 0 )
            {
               amxrDisconnectAllByName ( g_amxr_client,
                                         AAA_PORT_NAME,
                                         g_hal_rd_device.info.mport_name );
            }

            memcpy( &g_hal_rd_device,
                    &g_hal_device,
                    sizeof(g_hal_rd_device) );

            /* Connect the ports together.
            */
            amxrGetPortInfoByName( g_amxr_client,
                                   g_hal_device.info.mport_name,
                                   &g_amxr_port_info );

            amxrQueryPortByName( g_amxr_client,
                                 g_amxr_port_info.name,
                                 &portid );

            amxrConnect( g_amxr_client,
                         portid,
                         g_aaa_port,
                         AMXR_CONNECT_MONO2MONO ); 

            /* Setup gain on codec of interest for audio input.
            */
            halAudioQueryCodecByName ( g_halaudio_client,
                                       g_hal_device.info.codec_name,
                                       &codec );

            /* Digital gain */
            halAudioSetGain ( g_halaudio_client,
                              HALAUDIO_BLOCK_DIG_ID( codec, HALAUDIO_DIR_ADC ),
                              AAA_AUDIO_DIG_GAIN );

            /* Analog gain */
            halAudioSetGain ( g_halaudio_client,
                              HALAUDIO_BLOCK_ANA_ID( codec,
                                                     g_hal_device.info.mic_hwsel,
                                                     HALAUDIO_DIR_ADC ),
                              AAA_AUDIO_ANA_GAIN );

            rc = 0;
         }
      }
      break;

      default:
      {
         rc = -EINVAL;
      }
      break;
   }

   return rc;
}

/***************************************************************************/
/**
*  Platform support constructor
*/
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36)
static int __init aaa_probe( struct platform_device *pdev )
#else
static int aaa_probe( struct platform_device *pdev )
#endif
{
   int err;

   memset( &g_aaa_write_state, 0, sizeof( g_aaa_write_state ));

   g_aaa_wrap_buf = vmalloc( AAA_PORT_SRC_FRAME * AAA_PORT_SRC_CHAN );
   if ( g_aaa_wrap_buf == NULL )
   {
      return -ENOMEM;
   }
   // printk( KERN_ERR "%s: setting up.\n", __FUNCTION__ );

   err = register_chrdev( BCM_AAA_MAJOR, AAA_DEVICE_NAME, &gfops );   
   if ( err )
   {
      printk( KERN_ERR "%s: failed to register character device major=%d\n",
              __FUNCTION__, BCM_AAA_MAJOR );
      goto error_cleanup;
   }

#ifdef CONFIG_SYSFS
   // printk( KERN_ERR "%s: creating class.\n", __FUNCTION__ );
   aaa_class = class_create( THIS_MODULE, "bcm-aaa" );
   if(IS_ERR( aaa_class ))
   {
      printk(KERN_ERR "AAA: Class create failed\n");
      err = -EFAULT;
      goto err_unregister_chrdev;
   }

   // printk( KERN_ERR "%s: creating device: %s.\n", __FUNCTION__, AAA_DEVICE_NAME );
   aaa_dev = device_create( aaa_class,
                            NULL,
                            MKDEV( BCM_AAA_MAJOR, 0 ),
                            NULL,
                            AAA_DEVICE_NAME );
   if(IS_ERR(aaa_dev))
   {
      printk(KERN_ERR "AAA: Device create failed\n");
      err = -EFAULT;
      goto err_class_destroy;
   }
#endif

   /* Create AMXR client.
   */
   // printk( KERN_ERR "%s: allocating amxr client.\n", __FUNCTION__ );
   err = g_amxr_client = amxrAllocateClient();
   if( g_amxr_client < 0 )
   {
      printk(KERN_ERR "%s: failed to allocate AAA Amxr client. rc=%i\n",
             __FUNCTION__, err);
      goto err_dev_destroy;
   }

   memset ( &g_hal_wrt_device, 0, sizeof(g_hal_wrt_device) );
   memset ( &g_hal_rd_device, 0, sizeof(g_hal_rd_device) );
   memset ( &g_aaa_read_buf, 0, sizeof(g_aaa_read_buf) );

   /* Create AAA port.
   */
   // printk( KERN_ERR "%s: allocating amxr port.\n", __FUNCTION__ );
   err = amxrCreatePort( AAA_PORT_NAME,
                         &g_amxr_callback,
                         &g_aaa_write_state,
                         AAA_PORT_DST_HZ,
                         AAA_PORT_DST_CHAN,
                         AAA_PORT_DST_CHAN * AAA_PORT_DST_FRAME,
                         AAA_PORT_SRC_HZ,
                         AAA_PORT_SRC_CHAN,
                         AAA_PORT_SRC_CHAN * AAA_PORT_SRC_FRAME,
                         &g_aaa_port );
   if( err )
   {
      printk(KERN_ERR "%s: failed to create AAA port. rc=%i\n",
             __FUNCTION__, err );
      goto err_amxr_client;
   }

   /* All is well...
   */
   printk( KERN_INFO "Android Amxr Audio - \'AAA\' grade beef driver...\n" );
   return 0;

err_amxr_client:
   amxrFreeClient( g_amxr_client );
err_dev_destroy:
#ifdef CONFIG_SYSFS
   device_destroy( aaa_class, MKDEV( BCM_AAA_MAJOR, 0 ) );
err_class_destroy:
   class_destroy( aaa_class );
err_unregister_chrdev:
   unregister_chrdev( BCM_AAA_MAJOR, AAA_DEVICE_NAME );
#endif
error_cleanup:
   aaa_remove( pdev );
   return err;
}

/***************************************************************************/
/**
*  Platform support destructor
*/
static int aaa_remove( struct platform_device *pdev )
{
   halAudioWriteFree( &g_aaa_write_state );
   vfree( &g_aaa_wrap_buf );
   g_aaa_wrap_buf = NULL;

#ifdef CONFIG_SYSFS
   device_destroy( aaa_class, MKDEV( BCM_AAA_MAJOR, 0 ) );
   class_destroy( aaa_class );
#endif

   unregister_chrdev( BCM_AAA_MAJOR, "aaa" );

   /* Terminate 'aaa' port existence.
   */
   if ( g_hal_wrt_device.name[0] != 0 )
   {
      amxrDisconnectAllByName ( g_amxr_client,
                                AAA_PORT_NAME,
                                g_hal_wrt_device.info.mport_name );

      /* If speaker is currently used, disconnect the right speaker which
      ** goes through "audioh4".
      */
      if ( strcmp( g_hal_wrt_device.name, "handsfree-spkr" ) == 0 )
      {
         amxrDisconnectAllByName ( g_amxr_client,
                                   AAA_PORT_NAME,
                                   "halaudio.audioh4" );
      }

      memset ( &g_hal_wrt_device, 0, sizeof(g_hal_wrt_device) );
   }
   if ( g_hal_rd_device.name[0] != 0 )
   {
      amxrDisconnectAllByName ( g_amxr_client,
                                AAA_PORT_NAME,
                                g_hal_rd_device.info.mport_name );
      memset ( &g_hal_rd_device, 0, sizeof(g_hal_rd_device) );
   }
   amxrRemovePort( g_aaa_port );
   amxrFreeClient( g_amxr_client );

   return 0;
}

/* Platform driver */
static struct platform_driver aaa_driver =
{
   .driver     =
   {
      .name    = "bcm-aaa",
      .owner   = THIS_MODULE,
   },
   .probe  = aaa_probe,
   .remove = aaa_remove,
};

static int __init aaa_init( void )
{
   // printk( KERN_ERR "%s: called...\n", __FUNCTION__ );

   return platform_driver_register( &aaa_driver );
}

static void __exit aaa_exit( void )
{
   // printk( KERN_ERR "%s: called...\n", __FUNCTION__ );

   platform_driver_unregister( &aaa_driver );
}

module_init( aaa_init );
module_exit( aaa_exit );
MODULE_AUTHOR( "Broadcom" );
MODULE_DESCRIPTION( "Android Amxr Audio - \'AAA\' best grade beef!" );
MODULE_LICENSE( "GPL" );
