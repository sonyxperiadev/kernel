/*****************************************************************************
* Copyright 2008 - 2009 Broadcom Corporation.  All rights reserved.
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
/**
*
*  usb_drv.c
*
*  PURPOSE:
*
*     This file contains the USB driver routines.
*
*  NOTES:
*
*****************************************************************************/


/* ---- Include Files ---------------------------------------- */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>

#include <linux/usb.h>
#include <linux/proc_fs.h>                      /* For /proc/audio */
#include <linux/sysctl.h>
#include <linux/vmalloc.h>
#include <linux/broadcom/halaudio.h>
#include <linux/broadcom/halaudio_lib.h>
#include <linux/broadcom/amxr.h>

#include "halaudio_usbcore.h"

/* ---- Public Variables ------------------------------------- */
/* ---- Private Constants and Types -------------------------- */

#define USB_SAMP_WIDTH                  2       /* Number of bytes per sample                  */
#define USB_FRAME_PERIOD                5       /* in msec                                     */
#define USB_MAX_SAMPLING_FREQ           48000
#define USB_MAX_CHANNELS                2       /* Max support stereo channels                 */
#define USB_MAX_FRAME_SIZE              (USB_MAX_CHANNELS*(USB_MAX_SAMPLING_FREQ/1000)*USB_SAMP_WIDTH*USB_FRAME_PERIOD)

#define USB_DEFAULT_FREQ                16000
#define USB_DEFAULT_FRAME_SIZE_1MS_MONO ((USB_DEFAULT_FREQ/1000) * USB_SAMP_WIDTH)
#define USB_DEFAULT_FRAME_SIZE_MONO     (USB_DEFAULT_FRAME_SIZE_1MS_MONO * USB_FRAME_PERIOD)

/* Rounding added to calculate dB value from raw USB value */
#define USB_VOL_DB_VALUE(val)           ((val) > 0 ? ((val)+128)/256 : ((val)-128)/256)
#define USB_VOL_LIN_VALUE(db)           ((db)*256)

/* KNL Log debug
 */
#define USB_ENABLE_LOG                  0
#if USB_ENABLE_LOG
#include <linux/broadcom/knllog.h>
#define USB_LOG                         KNLLOG
#else
#define USB_LOG(c,d...)
#endif

/* CSX Data stucture */
struct usb_csx_data
{
   CSX_IO_POINT_FNCS    csx_ops;
   void                *priv;
};

/* Media buffers for USB
 */
struct usb_media
{
   /* bufIdx points to buffer used by USB interface.
    * The other buffer is used by HAL Audio interface.
    */
   int bufIdx;

   /* USB interface read/write index into a buffer.
    */
   int rdWtIdx;

   /* Pointer to double buffer.
    */
   uint8_t *pBuf[2];
};

/* USB state information
 */
struct usb_info
{
   int                      initialized;
   atomic_t                 enable;
   atomic_t                 prepared;

   /* Audio ingress/egress information
    */
   struct usb_media         igrMedia;
   struct usb_media         egrMedia;

   /* Audio Mixer facilities
    *
    * Create two ports for mono and stereo
    */
   AMXR_PORT_ID            mixer_port;

   /* Debug information
    */
   int                      loop_ig2eg;
   int                      loop_eg2ig;
   unsigned int             isrcount_igr;
   unsigned int             isrcount_egr;
   HALAUDIO_SINECTL         sinectl;

   /* Write state
    */
   HALAUDIO_WRITE           write;

   /* CSX data
    */
   struct usb_csx_data      csx_data[HALAUDIO_NUM_CSX_POINTS]; /* Array of CSX data structures */
};

/* Supported sampling rates
 */
static int gUsbSupportedSamplingRates[] =
{
   8000, 16000, /*22050,*/ 32000, /*40000, 44100,*/ 48000, 0 /* NULL terminated */
};

/* ---- Private Variables ------------------------------------ */

/* USB information
 */
static struct usb_info           gUsb;

/* Installed callback. Called when all ingress processing has completed */
static HALAUDIO_IF_FRAME_ELAPSED gUsbElapsedCb;
static void                     *gUsbUserData;

/* HAL Audio interface handle
 */
static HALAUDIO_IF_HDL           gInterfHandle;

/* ---- Private Function Prototypes -------------------------- */

static int  usbReadProc( char *buf, char **start, off_t offset, int count, int *eof, void *data );

static void usbEgressHandler( int16_t *pData, int bytes );
static void usbIngressHandler( int16_t *pData, int bytes );
static void usbIngressInfo( int freqHz, int numberChannels );
static void usbEgressInfo( int freqHz, int numberChannels );
static void usbDisconnect( void );

static __devinit int  usbInit( HALAUDIO_IF_FRAME_ELAPSED cb, void *data );
static int  usbExit( void );
static int  usbPrepare( void );
static int  usbEnable( void );
static int  usbDisable( void );
static int  usbSetFreq( int chno, int freqHz );
static int  usbGetFreq( int chno, int *freqHz );
static int  usbDigGainSet( int chno, int db, HALAUDIO_DIR dir );
static int  usbDigGainGet( int chno, HALAUDIO_GAIN_INFO *info, HALAUDIO_DIR dir );
static int  usbCodecInfo( int chno, HALAUDIO_CODEC_INFO *codec_info );
static int  usbCsxSet( int chno, HALAUDIO_CSX_POINT_ID point, const CSX_IO_POINT_FNCS *fncp, void *data );
static int  usbWrite( int chno, int bytes, const char *audiobuf, HALAUDIO_CODEC_IORW_CB usercb, void *userdata );
static int  usbWriteTerm( void );

static int  allocateMedia( struct usb_media *pM, int frameSize );
static void freeMedia( struct usb_media *pM );
static void createProcEntries( void );
static void cleanupProcEntries( void );

static int  usbMixerPortsRegister( void );
static int  usbMixerPortsDeregister( void );

/* For proc/sys
 */
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,31)
static int  proc_doSineGen( ctl_table *table, int write,
      void __user *buffer, size_t *lenp, loff_t *ppos );
#else
static int  proc_doSineGen( ctl_table *table, int write, struct file *filp,
      void __user *buffer, size_t *lenp, loff_t *ppos );
#endif

static struct ctl_table gSysCtlUsb[] =
{
   {
      .procname      = "sine_freq",
      .data          = &gUsb.sinectl.freq,
      .maxlen        = sizeof(int),
      .mode          = 0644,
      .proc_handler  = &proc_doSineGen,
   },
   {
      .procname      = "loopi2e",
      .data          = &gUsb.loop_ig2eg,
      .maxlen        = sizeof(int),
      .mode          = 0644,
      .proc_handler  = &proc_dointvec,
   },
   {
      .procname      = "loope2i",
      .data          = &gUsb.loop_eg2ig,
      .maxlen        = sizeof(int),
      .mode          = 0644,
      .proc_handler  = &proc_dointvec,
   },
   {}
};

static struct ctl_table gSysCtl[] =
{
    { 
      .procname   = "usb", 
      .mode       = 0555, 
      .child      = gSysCtlUsb, 
    },
    {}
};

static struct ctl_table_header *gSysCtlHeader;

/* HAL Audio USB interface operations
 */
static HALAUDIO_IF_OPS halaudio_usb_ops __initdata =
{
   .init                = usbInit,
   .exit                = usbExit,
   .prepare             = usbPrepare,
   .enable              = usbEnable,
   .disable             = usbDisable,
   .codec_ops           =
   {
      .setfreq          = usbSetFreq,
      .getfreq          = usbGetFreq,
      .setdig           = usbDigGainSet,
      .getdig           = usbDigGainGet,
/*      .read             = usbRead, */
      .write            = usbWrite,
      .info             = usbCodecInfo,
      .setcsx           = usbCsxSet,
   },
};

/* Callbacks from HAL Audio USB core
 */
static __devinitdata USB_CORE_CBS usbCoreCbs =
{
   .mediaIn    = usbIngressHandler,
   .mediaOut   = usbEgressHandler,
   .infoIn     = usbIngressInfo,
   .infoOut    = usbEgressInfo,
   .disconnect = usbDisconnect
};

/* ---- Functions -------------------------------------------- */

/***************************************************************************/
/**
*  USB driver Initialization
*
*  @return  
*     0        success
*     -ve      failure code
*/
static __devinit int usbInit(
   HALAUDIO_IF_FRAME_ELAPSED  cb,   /*<< (i) Callback to call when one frame elapses */
   void                      *data  /*<< (i) User data to pass to callback */
)
{
   int rc;
   int egrFrameSize = usbCoreGetEgressFrameSize1ms()*USB_FRAME_PERIOD;
   int igrFrameSize = usbCoreGetIngressFrameSize1ms()*USB_FRAME_PERIOD;

   /* Initialize globals
    */
   memset( &gUsb, 0, sizeof( gUsb ) );

   atomic_set( &gUsb.enable, 0 );
   atomic_set( &gUsb.prepared, 0 );

   /* Register to HAL Audio USB core
    */
   usbCoreRegisterCb( &usbCoreCbs );

   rc = usbMixerPortsRegister();
   if( rc )
   {
      printk( KERN_ERR "USB: Failed to register mixer ports rc=%i\n", rc );
      goto init_cleanup;
   }

   /* Allocate resources 
    */
   rc = allocateMedia( &gUsb.igrMedia, igrFrameSize );
   if( rc )
   {
      printk( KERN_ERR "USB: Failed to allocate ingress resources rc=%i\n", rc );
      goto init_cleanup;
   }
   rc = allocateMedia( &gUsb.egrMedia, egrFrameSize );
   if( rc )
   {
      printk( KERN_ERR "USB: Failed to allocate egress resources rc=%i\n", rc );
      goto init_cleanup;
   }

   /* Create proc entries
    */
   createProcEntries();

   /* Save HAL Audio specifics */
   gUsbElapsedCb = cb;
   gUsbUserData  = data;

   gUsb.initialized = 1;

   return 0;

init_cleanup:

   usbMixerPortsDeregister();
   usbCoreDeregisterCb();

   freeMedia( &gUsb.igrMedia );
   freeMedia( &gUsb.egrMedia );

   return rc;
}

/***************************************************************************/
/**
*  USB driver exit routine
*
*  @return  
*     0        success
*     -ve      failure code
*/
int usbExit( void )
{
   int rc = 0;

   /* Remove ports from mixer
    */
   rc = usbMixerPortsDeregister();
   if( rc )
   {
      printk( KERN_ERR "USB: Failed to de-register mixer ports\n" );
   }

   /* Cleanup write interface.
    */
   usbWriteTerm();

   /* Deregister from HAL Audio USB core
    */
   usbCoreDeregisterCb();

   /* Free media resources
    */
   freeMedia( &gUsb.igrMedia );
   freeMedia( &gUsb.egrMedia );

   /* Remove proc entries
    */
   cleanupProcEntries();

   return rc;
}

/***************************************************************************/
/**
*  Query codec channel information
*
*  @return 
*     0        Success
*     -ve      Error code
*
*  @remarks
*/
static int usbCodecInfo( int chno, HALAUDIO_CODEC_INFO *codec_info )
{
   int *freq;
   int igrChans     = usbCoreGetIngressChannels();
   int egrChans     = usbCoreGetEgressChannels();
   int igrConnected = usbCoreIngressIsConnected();
   int egrConnected = usbCoreEgressIsConnected();

   (void) chno;

   memset( codec_info, 0, sizeof(*codec_info) );

   sprintf( codec_info->name, "USB" );

   if( igrConnected || egrConnected )
   {
      freq = gUsbSupportedSamplingRates;
      while( *freq != 0 )
      {
         /* Make sure both directions support the sampling rate
          */
         if( (igrConnected && !usbCoreIngressIsSupported( *freq, igrChans )) ||
             (egrConnected && !usbCoreEgressIsSupported( *freq, egrChans )) )
         {
            freq++;
            continue;
         }

         /* Go ahead and add the frequency as one that is supported
          */
         codec_info->freqs.freq[codec_info->freqs.num] = *freq;
         codec_info->freqs.num++;
         freq++;
      }
   }

   codec_info->channels_tx  = igrChans;             /* # of channels for tx (mic), 1=mono, 2=stereo, etc. */
   codec_info->channels_rx  = egrChans;             /* # of channels for rx (spk), 1=mono, 2=stereo, etc. */
   codec_info->sample_width = USB_SAMP_WIDTH;       /* sample size width in bytes */
   codec_info->read_format  = HALAUDIO_FMT_S16_LE;  /* Read format */
   codec_info->write_format = HALAUDIO_FMT_S16_LE;  /* Write format */
   codec_info->mics         = igrConnected;         /* # of mic paths. 0 means no ADC */
   codec_info->spkrs        = egrConnected;         /* # of speaker paths. 0 means no DAC */
   codec_info->bulk_delay   = -1;                   /* Echo bulk delay in samples, -1 = not calibrated */

#if 0
   codec_info->equlen_tx  = 0;                    /* max # of tx equalizer coeffs, 0 = equ unsupported */
   codec_info->equlen_rx  = 0;                    /* max # of rx equalizer coeffs, 0 = equ unsupported */
   codec_info->parent_id  = 0;                    /* Parent interface id */
#endif

   return 0;
}

/***************************************************************************/
/**
*  Set up CSX points for capture and injection. Refer to CSX documentation
*  for more informatin.
* 
*  @return
*     0        Success
*     -ve      Error code
*/
static int usbCsxSet(
   int                        chno, /*<< (i) Codec channel number */
   HALAUDIO_CSX_POINT_ID      point,/*<< (i) Point ID to install the CSX point */
   const CSX_IO_POINT_FNCS   *fncp, /*<< (i) Ptr to CSX callbacks */
   void                      *data  /*<< (i) User data to pass back to callbacks */
)
{
   unsigned long flags;
   int err = 0;

   /* Only support single USB device
    */
   if( chno >= 1 )
   {
      return -EINVAL;
   }

   local_irq_save( flags );
   switch( point )
   {
      case HALAUDIO_CSX_POINT_ADC:
      {
         memcpy( &gUsb.csx_data[HALAUDIO_CSX_POINT_ADC].csx_ops,
                 fncp,
                 sizeof(gUsb.csx_data[HALAUDIO_CSX_POINT_ADC].csx_ops) );

         gUsb.csx_data[HALAUDIO_CSX_POINT_ADC].priv = data;
         break;
      }
      case HALAUDIO_CSX_POINT_DAC:
      {
         memcpy( &gUsb.csx_data[HALAUDIO_CSX_POINT_DAC].csx_ops,
                 fncp,
                 sizeof(gUsb.csx_data[HALAUDIO_CSX_POINT_DAC].csx_ops) );

         gUsb.csx_data[HALAUDIO_CSX_POINT_DAC].priv = data;
         break;
      }
      default:
      {
         err = -EINVAL;
      }
   }
   local_irq_restore( flags );

   return err;
}

/***************************************************************************/
/**
*  Enable USB
*
*  @return 
*     0     success
*     -1    failed to enable
*/
int usbEnable( void )
{
   int rc;

   if( !atomic_read( &gUsb.prepared ) )
   {
      return -EPERM;
   }

   if( atomic_read( &gUsb.enable ) )
   {
      /* Already enabled
       */
      return -EBUSY;
   }

   /* Enable HAL Audio USB core
    */
   rc = usbCoreEnable();
   if( rc )
   {
      printk( KERN_ERR "USB: Unable to enable USB core rc=%i\n", rc );
      return rc;
   }

   atomic_set( &gUsb.enable, 1 );

   return 0;
}

/***************************************************************************/
/**
*  Disable USB
*
*  @return 
*     0     success
*     -1    failed to enable
*/
int usbDisable( void )
{
   int rc;

   if( !atomic_read( &gUsb.prepared ) )
   {
      /* Nothing to disable, has not been prepared
       */
      return 0;
   }

   atomic_set( &gUsb.enable, 0 );

   halAudioWriteFlush( &gUsb.write ); 

   /* Disable HAL Audio USB core
    */
   rc = usbCoreDisable();
   if( rc )
   {
      printk( KERN_ERR "USB: Unable to disable USB core rc=%i\n", rc );
      return rc;
   }

   atomic_set( &gUsb.prepared, 0 );

   return 0;
}

/***************************************************************************/
/**
*  Prepare USB
*
*  @return 
*     0     success
*     -1    failed to enable
*/
static int usbPrepare( void )
{
   int rc;

   if( atomic_read( &gUsb.prepared ) )
   {
      /* Already enabled or prepared
       */
      return -EBUSY;
   }

   /* call USB Core prepare routine
    */
   rc = usbCorePrepare();
   if( rc )
   {
      printk( KERN_ERR "USB: Unable to prepare USB core rc=%i\n", rc );
      return rc;
   }

   atomic_set( &gUsb.prepared, 1 );

   return 0;
}

/***************************************************************************/
/**
*  USB egress handler
*
*  @return  none
*
*  @remarks This routine operates in an atomic operation
*/
static void usbEgressHandler( int16_t *pData, int bytes )
{
   struct usb_media *pM = &gUsb.egrMedia;

   int egrFrameSize = usbCoreGetEgressFrameSize1ms()*USB_FRAME_PERIOD;

   USB_LOG( "Egress handler isr=%u", gUsb.isrcount_egr );

   gUsb.isrcount_egr++;

   /* Copy over another 1ms of egress data to the USB core
    */
   memcpy( pData, pM->pBuf[ pM->bufIdx ] + pM->rdWtIdx, bytes );

   pM->rdWtIdx += bytes;
   if( pM->rdWtIdx >= egrFrameSize )
   {
      void *egressp;

      /* Update buffer indices to point to where data has been deposited by
       * the audio mixer.
       */
      pM->bufIdx  ^= 1;
      pM->rdWtIdx  = 0;

      egressp = (int16_t *)pM->pBuf[ pM->bufIdx ];

      /* Write request
       */
      halAudioWriteService( &gUsb.write, egressp, egrFrameSize );

      if( gUsb.sinectl.freq )
      {
         halAudioSine( egressp, &gUsb.sinectl, egrFrameSize/2, usbCoreGetEgressChannels() );
      }
      else if( gUsb.loop_ig2eg )
      {
         memcpy( egressp, gUsb.igrMedia.pBuf[ gUsb.igrMedia.bufIdx^1 ], egrFrameSize );
      }

      if ( gUsb.csx_data[HALAUDIO_CSX_POINT_DAC].csx_ops.csxCallback )
      {
         gUsb.csx_data[HALAUDIO_CSX_POINT_DAC].csx_ops.csxCallback( (char *)egressp, egrFrameSize, gUsb.csx_data[HALAUDIO_CSX_POINT_DAC].priv );
      }
   }

   USB_LOG( "end" );
}

/***************************************************************************/
/**
*  USB ingress handler
*
*  @return  none
*
*  @remarks This routine operates in an atomic operation
*/
static void usbIngressHandler(  int16_t *pData, int bytes )
{
   struct usb_media *pM = &gUsb.igrMedia;
   uint8_t          *bufp;
   int igrFrameSize = usbCoreGetIngressFrameSize1ms()*USB_FRAME_PERIOD;

   USB_LOG( "Ingress handler isr=%u", gUsb.isrcount_igr );

   gUsb.isrcount_igr++;

   bufp = pM->pBuf[ pM->bufIdx ] + pM->rdWtIdx;
   memcpy( bufp, pData, bytes );

   if( gUsb.csx_data[HALAUDIO_CSX_POINT_ADC].csx_ops.csxCallback )
   {
      gUsb.csx_data[HALAUDIO_CSX_POINT_ADC].csx_ops.csxCallback( bufp, bytes, gUsb.csx_data[HALAUDIO_CSX_POINT_ADC].priv );
   }

   pM->rdWtIdx += bytes;
   if( pM->rdWtIdx >= igrFrameSize )
   {
      void *ingressp = pM->pBuf[ pM->bufIdx ];

      /* Update buffer indices */
      pM->bufIdx  ^= 1;
      pM->rdWtIdx  = 0;

      if( gUsb.loop_eg2ig )
      {
         memcpy( ingressp, gUsb.egrMedia.pBuf[ gUsb.egrMedia.bufIdx^1 ], igrFrameSize );
      }

      /* Perform callback action. All ingress processing have completed. */
      if ( gUsbElapsedCb ) 
      {
         (*gUsbElapsedCb)( gUsbUserData );
      }
   }

   USB_LOG( "end" );
}

/***************************************************************************/
/**
*  Select sampling frequency for all channels
*
*  @return 
*     0        Success
*     -ENODEV  USB has not be initiialized
*     -EBUSY   USB is currently running
*     -EINVAL  Invalid channel number
*/
int usbSetFreq( 
   int chno,                        /**< (i) channel number 0-2 */
   int freqHz                       /**< (i) sampling frequency in Hz */
)
{
   int validhz, rc = 0;
   int igrChans     = usbCoreGetIngressChannels();
   int egrChans     = usbCoreGetEgressChannels();
   int igrConnected = usbCoreIngressIsConnected();
   int egrConnected = usbCoreEgressIsConnected();
   int *pFreq;

   (void)chno;

   USB_LOG( "New sampling frequency %u Hz requested\n", freqHz );

   if( !gUsb.initialized )
   {
      return -ENODEV;
   }

   if( !igrConnected && !egrConnected )
   {
      return -ENODEV;
   }

   if( atomic_read( &gUsb.enable ) || atomic_read( &gUsb.prepared ) )
   {
      /* Cannot change sampling frequency if currently running
       */
      return -EBUSY;
   }

   /* Check if the sampling rate is supported in both directions.
    */
   validhz = 0;
   pFreq = gUsbSupportedSamplingRates;
   while( *pFreq != 0 )
   {
      if ( freqHz == *pFreq )
      {
         /* Make sure both directions support the sampling rate
          */
         if( !((igrConnected && !usbCoreIngressIsSupported( *pFreq, igrChans )) ||
               (egrConnected && !usbCoreEgressIsSupported( *pFreq, egrChans ))) )
         {
            validhz = 1;
         }
         break;
      }
      pFreq++;
   }
   if( !validhz )
   {
      return -EINVAL;
   }

   /* Call USB core to set both ingress and egress frequencies
    */
   if( usbCoreGetIngressFreq() != freqHz )
   {
      rc = usbCoreSetIngressFreq( freqHz );
      if( rc )
      {
         printk( KERN_ERR "USB: Can not set ingress sampling frequency rc=%u\n", rc );
         return rc;
      }
   }
   
   if( usbCoreGetEgressFreq() != freqHz )
   {
      rc = usbCoreSetEgressFreq( freqHz );
      if( rc )
      {
         printk( KERN_ERR "USB: Can not set egress sampling frequency rc=%u\n", rc );
         return rc;
      }
   }

   /* Don't adjust mixer frequency here.  Wait for the HAL Audio USB core callbacks 
    * to change the frequency.
    */

   return 0;
}

/***************************************************************************/
/**
*  Retrieve sampling frequency information for a codec channel.
*
*  @return
*     0        Success
*     -ve      Error code
*/
static int usbGetFreq( 
   int   chno,                      /*<< (i) Codec channel number */
   int  *freqhz                     /*<< (o) Ptr to sampling frequency in Hz */
)
{
   (void) chno;

   /* Return ingress sampling frequency
    */
   *freqhz = usbCoreGetIngressFreq();

   return 0;
}

/***************************************************************************/
/**
*  Set hardware digital gains. Each channel consists of up to two
*  gains, one per direction (ingress and egress). 
*
*  @return 
*     0        Success
*     -ve      Error code
*
*  @remarks
*/
static int usbDigGainSet(
   int             chno,      /**< (i)    Channel index */
   int             db,        /**< (i)    dB value to set gain */
   HALAUDIO_DIR    dir        /**< (i)    Audio direction */
)
{
   int rc, usbVolume;

   (void) chno;

   /* Convert db to USB usable volume. */
   usbVolume = USB_VOL_LIN_VALUE( db );

   if( db >= 128 )
   {
      usbVolume = 0x7FFF;
   }
   
   if( db <= -128 )
   {
      usbVolume = 0x8000;
   }

   if( dir == HALAUDIO_DIR_ADC )
   {
      rc = usbCoreSetIngressVolume( usbVolume );
   }
   else
   {
      rc = usbCoreSetEgressVolume( usbVolume );
   }

   return rc;
}

/***************************************************************************/
/**
*  Retrieve digital gains for a codec channel. Valid gain range excludes
*  mute.
*
*  @return
*     0        Success
*     -ve      Error code
*/
static int usbDigGainGet( 
   int                  chno,       /*<< (i) codec channel number */
   HALAUDIO_GAIN_INFO  *info,       /*<< (o) Ptr to gain info structure */
   HALAUDIO_DIR         dir         /*<< (i) Direction path */
)
{
   int rc;
   struct usb_core_volume_info usbInfo;

   (void) chno;

   if( dir == HALAUDIO_DIR_ADC )
   {
      rc = usbCoreGetIngressVolume( &usbInfo );
   }
   else
   {
      rc = usbCoreGetEgressVolume( &usbInfo );
   }

   if( rc )
   {
      printk( KERN_ERR "USB: Failed to get volume\n" );
      return rc;
   }

   /* Round down
    */
   info->mindb             = USB_VOL_DB_VALUE( usbInfo.min );
   info->maxdb             = USB_VOL_DB_VALUE( usbInfo.max );
   if( usbInfo.cur == 0x8000 )
   {
      info->currentdb      = HALAUDIO_GAIN_MUTE;
   }
   else
   {
      info->currentdb      = USB_VOL_DB_VALUE( usbInfo.cur );
   }

   info->range_type        = HALAUDIO_RANGETYPE_FIXED_STEPSIZE;

   /* Round up
    */
   info->range.fixed_step  = USB_VOL_DB_VALUE( usbInfo.res );
   if( !info->range.fixed_step )
   {
      info->range.fixed_step = 1;
   }

   return 0;
}

/***************************************************************************/
/**
*  Callback from USB core to indicate change in sampling frequency
*  or number of channels.
*
*  @return     Nothing
*/
static void usbIngressInfo( int freqHz, int numberChannels )
{
   int rc, bytes;

   bytes = numberChannels * (freqHz/1000) * USB_FRAME_PERIOD * USB_SAMP_WIDTH; 

   /* Free and re-allocate memory for media data
    */
   freeMedia( &gUsb.igrMedia );
   allocateMedia( &gUsb.igrMedia, bytes );

   /* Adjust the Audio mixer
    */
   rc = amxrSetPortSrcFreq( gUsb.mixer_port, freqHz, bytes ); 
   if( rc )
   {
      printk( KERN_ERR "%s: Failed to set mixer sampling frequency rc=%i.\n", __FUNCTION__, rc );
   }
   rc = amxrSetPortSrcChannels( gUsb.mixer_port, numberChannels, bytes ); 
   if( rc )
   {
      printk( KERN_ERR "%s: Failed to set mixer channel config to %i rc=%i.\n", __FUNCTION__, numberChannels, rc );
   }
}

/***************************************************************************/
/**
*  Callback from USB core to indicate change in sampling frequency
*  or number of channels.
*
*  @return     Nothing
*/
static void usbEgressInfo( int freqHz, int numberChannels )
{
   int rc, bytes;

   bytes = numberChannels * (freqHz/1000) * USB_FRAME_PERIOD * USB_SAMP_WIDTH; 

   /* Free and re-allocate memory for media data
    */
   freeMedia( &gUsb.egrMedia );
   allocateMedia( &gUsb.egrMedia, bytes );

   /* Adjust the Audio mixer
    */
   rc = amxrSetPortDstFreq( gUsb.mixer_port, freqHz, bytes );
   if( rc )
   {
      printk( KERN_ERR "%s: Failed to set mixer freqhz=%i rc=%i.\n", __FUNCTION__, freqHz, rc );
   }
   rc = amxrSetPortDstChannels( gUsb.mixer_port, numberChannels, bytes ); 
   if( rc )
   {
      printk( KERN_ERR "%s: Failed to set mixer chans=%i rc=%i.\n", __FUNCTION__, numberChannels, rc );
   }

   halAudioSineConfig( &gUsb.sinectl, gUsb.sinectl.freq, freqHz );
}

/***************************************************************************/
/**
*  Callback from USB core indicating the USB device is removed.  Clean
*  up some buffers.
*
*  @return     Nothing
*/
static void usbDisconnect( void )
{
   struct usb_media *pM;
   int frameSize;

   /* Clean up all the buffers and indices */
   pM          = &gUsb.egrMedia;
   frameSize   = usbCoreGetEgressFrameSize1ms()*USB_FRAME_PERIOD;
   pM->bufIdx  = 0;
   pM->rdWtIdx = 0;
   memset( pM->pBuf[0], 0, frameSize );
   memset( pM->pBuf[1], 0, frameSize );

   pM          = &gUsb.igrMedia;
   frameSize   = usbCoreGetIngressFrameSize1ms()*USB_FRAME_PERIOD;
   pM->bufIdx  = 0;
   pM->rdWtIdx = 0;
   memset( pM->pBuf[0], 0, frameSize );
   memset( pM->pBuf[1], 0, frameSize );
}

/***************************************************************************/
/**
*  Write method used to directly write samples to a channel's egress buffers.
*
*  @return 
*     0        Success
*     -ve      Error code
*
*  @remarks
*/
static int usbWrite(
   int                     chno,       /**< (i) PCM channel index */
   int                     bytes,      /**< (i) Number of bytes to write */
   const char             *audiobuf,   /**< (i) Pointer to audio samples */
   HALAUDIO_CODEC_IORW_CB  usercb,     /**< (i) User callback to request for more data */
   void                   *userdata    /**< (i) User data */
)
{
   (void) chno;

   if( usbCoreEgressIsConnected() )
   {
      printk( KERN_ERR "USB: No egress path\n" );
      return -EPERM;
   }

   return halAudioWriteRequest( &gUsb.write, bytes, audiobuf, usercb, userdata );
}

/***************************************************************************/
/**
*  Cleanup USB write memory resources
*
*  @return  
*     0        success
*     -ve      failure code
*/
static int usbWriteTerm( void )
{
   return halAudioWriteFree( &gUsb.write );
}

/***************************************************************************/
/**
*  Allocate resources
*
*  @return  
*     0        success
*     -ve      failure code
*/
static int allocateMedia( struct usb_media *pM, int frameSize )
{
   pM->pBuf[0] = vmalloc( frameSize );
   if( !pM->pBuf[0] )
   {
      printk( KERN_ERR "USB: Failed to allocate memory for media\n" );
      return -ENOMEM;
   }

   memset( pM->pBuf[0], 0, frameSize );

   pM->pBuf[1] = vmalloc( frameSize );
   if( !pM->pBuf[1] )
   {
      printk( KERN_ERR "USB: Failed to allocate memory for media\n" );
      vfree( pM->pBuf[0] );
      return -ENOMEM;
   }

   memset( pM->pBuf[1], 0, frameSize );

   return 0;
}

/***************************************************************************/
/**
*  Free resources
*
*  @return     Nothing
*/
static void freeMedia( struct usb_media *pM )
{
   if( pM->pBuf[0] )
   {
      vfree( pM->pBuf[0] );
   }

   if( pM->pBuf[1] )
   {
      vfree( pM->pBuf[1] );
   }

   memset( pM, 0, sizeof( *pM ) );
}

/***************************************************************************/
/**
*  USB mixer callback for outgoing data (i.e. ingress)
*
*  @return
*     NULL     - non-matching frame size or non-existent buffer
*     ptr      - pointer to ingress buffer
*/
static int16_t *usbMixerCb_IngressGetPtr( 
   int   numBytes,            /**< (i) frame size in bytes */
   void *privdata             /**< (i) User data           */
)
{
   struct usb_media *pM = &gUsb.igrMedia;
   int igrFrameSize = usbCoreGetIngressFrameSize1ms()*USB_FRAME_PERIOD;

   (void) privdata;

   /* Only expect to be called asking for a full frame.
    */
   if( numBytes != igrFrameSize )
   {
      return NULL;
   }

   /* Perform clock drift compensation here.  Only compensate
    * for clock drift when we need to sync with Audio mixer.
    *
    * If the device is no longer connected, return a
    * pointer to the buffer anyway since it will contain
    * zeros due to the disconnect callback.
    */
   if( usbCoreIngressIsConnected() && pM->rdWtIdx != 0 )
   {
      if( pM->rdWtIdx > (igrFrameSize/2) )
      {
         /* Detected slow USB clock
          */
         USB_LOG( "USB [rdWtIdx=%u]: Ingress Get Ptr SLOW USB clock detected.", pM->rdWtIdx );

         /* Pad the buffer with zeros and release to
          * Audio mixer.  Have USB start writing to the next
          * buffer in the double buffer.
          */
         pM->bufIdx  ^= 1;
         pM->rdWtIdx  = 0;
      }
      else
      {
         /* Detected fast USB clock
          */
         USB_LOG( "USB [rdWtIdx=%u]: Ingress Get Ptr FAST USB clock detected.", pM->rdWtIdx );

         /* Delete excess samples
          */
         pM->rdWtIdx = 0;
      }

      memset( pM->pBuf[ pM->bufIdx ], 0, igrFrameSize );
   }

   return (int16_t*)pM->pBuf[ pM->bufIdx^1 ];
}

/***************************************************************************/
/**
*  USB mixer callback for incoming data (i.e. egress)
*
*  @return  
*     NULL     - non-matching frame size or non-existent buffer
*     ptr      - pointer to egress buffer
*/
static int16_t *usbMixerCb_EgressGetPtr(
   int   numBytes,            /**< (i) frame size in bytes */
   void *privdata             /**< (i) User data           */
)
{
   struct usb_media *pM = &gUsb.egrMedia;
   int egrFrameSize = usbCoreGetEgressFrameSize1ms()*USB_FRAME_PERIOD;

   (void) privdata;

   /* Only expect to be called asking for a full frame.
    */
   if( numBytes != egrFrameSize )
   {
      return NULL;
   }

   /* Perform clock drift compensation here.  Only compensate
    * for clock drift when we need to sync with Audio mixer.
    *
    * If the device is no longer connected, return a
    * pointer to the buffer anyway since it will contain
    * zeros due to the disconnect callback.
    */
   if( usbCoreEgressIsConnected() && pM->rdWtIdx != 0 )
   {
      if( pM->rdWtIdx > (egrFrameSize/2) )
      {
         /* Detected slow USB clock
          */
         USB_LOG( "USB [rdWtIdx=%u]: Egress Get Ptr SLOW USB clock detected.", pM->rdWtIdx );

         /* Pad the buffer with zeros and release to
          * Audio mixer.  Have USB start writing to the next
          * buffer in the double buffer.
          */
         pM->bufIdx  ^= 1;
         pM->rdWtIdx  = 0;
      }
      else
      {
         /* Detected fast USB clock
          */
         USB_LOG( "USB [rdWtIdx=%u]: Egress Get Ptr FAST USB clock detected.", pM->rdWtIdx );

         /* Delete excess samples
          */
         pM->rdWtIdx = 0;
      }

      memset( pM->pBuf[ pM->bufIdx ], 0, egrFrameSize );
   }

   return (int16_t*)pM->pBuf[ pM->bufIdx^1 ];
}

/***************************************************************************/
/**
*  USB mixer callback to flush the egress buffers when the last destination
*  connection is removed.
*
*  @return     None
*/
static void usbMixerCb_EgressFlush(
   void *privdata             /*<< (i) private data */
)
{
   struct usb_media *pM = &gUsb.egrMedia;
   int egrFrameSize = usbCoreGetEgressFrameSize1ms()*USB_FRAME_PERIOD;
   unsigned long flags;

   (void) privdata;

   /* Clear double buffer */
   local_irq_save( flags );
   memset( pM->pBuf[0], 0, egrFrameSize );
   memset( pM->pBuf[1], 0, egrFrameSize );
   local_irq_restore( flags );
}

/***************************************************************************/
/**
*  USB mixer port registration
*
*  @return  None
*/
static int usbMixerPortsRegister( void )
{
   AMXR_PORT_CB cb;
   AMXR_PORT_ID portid;
   char *mixerName = "halaudio.usb";
   int err;
   int ingress_bytes, egress_bytes;

   memset( &cb, 0, sizeof(cb) );

   cb.getsrc         = usbMixerCb_IngressGetPtr;
   cb.getdst         = usbMixerCb_EgressGetPtr;
   cb.dstcnxsremoved = usbMixerCb_EgressFlush;

   egress_bytes = ( usbCoreGetEgressChannels() * usbCoreGetEgressFreq() * USB_FRAME_PERIOD ) / 1000 * USB_SAMP_WIDTH;
   ingress_bytes = ( usbCoreGetIngressChannels() * usbCoreGetIngressFreq() * USB_FRAME_PERIOD ) / 1000 * USB_SAMP_WIDTH;

   err = amxrCreatePort( mixerName, &cb, NULL,
         usbCoreGetEgressFreq(), usbCoreGetEgressChannels(), egress_bytes,
         usbCoreGetIngressFreq(), usbCoreGetIngressChannels(), ingress_bytes,
         &portid );
   if( err )
   {
      usbMixerPortsDeregister();
      return err;
   }

   gUsb.mixer_port = portid;

   return 0;
}

/***************************************************************************/
/**
*  USB mixer port de-registration
*
*  @return  None
*/
static int usbMixerPortsDeregister( void )
{
   int rc;

   if( gUsb.mixer_port != NULL )
   {
      rc = amxrRemovePort( gUsb.mixer_port );
      if( rc )
      {
         printk( KERN_ERR "USB: failed to deregister mixer port\n" );
      }

      gUsb.mixer_port = NULL;    /* invalidate handle */
   }

   return 0;
}

/***************************************************************************/
/**
*  Create proc entries
*
*  @return     Notthing 
*/
static void createProcEntries( void )
{
   create_proc_read_entry( "usb", 0, NULL, usbReadProc, &gUsb );

   gSysCtlHeader = register_sysctl_table( gSysCtl );
}

/***************************************************************************/
/**
*  Cleanup proc entries
*
*  @return     Notthing 
*/
static void cleanupProcEntries( void )
{
   /* Release proc entries
    */
   remove_proc_entry( "usb", NULL );

   if( gSysCtlHeader )
   {
      unregister_sysctl_table( gSysCtlHeader );
   }
}

/***************************************************************************/
/**
*  Proc read callback function audio data (ingress part)
*
*  @return  None
*/
static int usbReadIngressProc( char *buf, void *data )
{
   int     len = 0;
   int16_t igbuf[USB_MAX_FRAME_SIZE/2];

   struct usb_info *pInfo = (struct usb_info*)data;

   unsigned long flags;

   int igrFrameSize = usbCoreGetIngressFrameSize1ms()*USB_FRAME_PERIOD;

   memset( igbuf, 0, sizeof(igbuf) );

   /* Cache data in critical section
    */
   local_irq_save( flags );
   if( usbCoreIngressIsConnected() )
      memcpy( igbuf, pInfo->igrMedia.pBuf[ pInfo->igrMedia.bufIdx^1 ], igrFrameSize );
   local_irq_restore( flags );

   len += sprintf( buf+len, "USB Interface @ %i ms\n\n", USB_FRAME_PERIOD );

   if( usbCoreIngressIsConnected() )
   {
      len += sprintf( buf+len, "Igr @ %i Hz, %i bytes\n",
            usbCoreGetIngressFreq(), igrFrameSize );
      halAudioPrintMemory( buf, &len, igbuf, igrFrameSize/sizeof(uint16_t), 1 /* HEX_FORMAT */, 10 /* line length */,
            2 /* word width */, 0 /* print_addr */, 0 /* addr */ );
   }

   /* Error report and other information
    */
   len += sprintf( buf+len, "Irqs: ingress=%u\n", gUsb.isrcount_igr );

   return len+1;
}


/***************************************************************************/
/**
*  Proc read callback function audio data (egress part)
*
*  @return  None
*/
static int usbReadEgressProc( char *buf, void *data )
{
   int     len = 0;
   int16_t egbuf[USB_MAX_FRAME_SIZE/2];

   struct usb_info *pInfo = (struct usb_info*)data;

   unsigned long flags;

   int egrFrameSize = usbCoreGetEgressFrameSize1ms()*USB_FRAME_PERIOD;

   memset( egbuf, 0, sizeof(egbuf) );

   /* Cache data in critical section
    */
   local_irq_save( flags );
   if( usbCoreEgressIsConnected() )
      memcpy( egbuf, pInfo->egrMedia.pBuf[ pInfo->egrMedia.bufIdx ], egrFrameSize );
   local_irq_restore( flags );

   len += sprintf( buf+len, "USB Interface @ %i ms\n\n", USB_FRAME_PERIOD );

   if( usbCoreEgressIsConnected() )
   {
      len += sprintf( buf+len, "Egr @ %i Hz, %i bytes \n",
            usbCoreGetEgressFreq(), egrFrameSize );
      halAudioPrintMemory( buf, &len, egbuf, egrFrameSize/sizeof(uint16_t), 1 /* HEX_FORMAT */, 10 /* line length */,
            2 /* word width */, 0 /* print_addr */, 0 /* addr */ );
   }

   /* Error report and other information
    */
   len += sprintf( buf+len, "Irqs: egress=%u\n", gUsb.isrcount_egr );

   return len+1;
}

/***************************************************************************/
/**
*  Proc read callback function audio data
*
*  @return  None
*/
static int usbReadProc( char *buf, char **start, off_t offset, int count, int *eof, void *data )
{
   int     len = 0;

   (void) start; (void) offset; (void) count;      /* avoid compiler warning */

   if( usbCoreIngressIsConnected() )
   {
	   len = usbReadIngressProc( buf, data );
   }

   if( usbCoreEgressIsConnected() )
   {
	   len += usbReadEgressProc( buf, data );
   }

   *eof = 1;
   return len;
}

/***************************************************************************/
/**
*  Sysctl callback to handle sine generation test
*
*  @return  0 success, otherwise failure
*/
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,31)
static int proc_doSineGen( ctl_table *table, int write,
      void __user *buffer, size_t *lenp, loff_t *ppos )
#else
static int proc_doSineGen( ctl_table *table, int write, struct file *filp,
      void __user *buffer, size_t *lenp, loff_t *ppos )
#endif
{
   int rc;

   /* Process integer operation 
    */
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,31)
   rc = proc_dointvec( table, write, buffer, lenp, ppos );
#else
   rc = proc_dointvec( table, write, filp, buffer, lenp, ppos );
#endif

   if( write ) 
   {
      /* Skip 0 hz 
       */
      if( gUsb.sinectl.freq )
      {
         halAudioSineConfig( &gUsb.sinectl, gUsb.sinectl.freq, usbCoreGetEgressFreq() );
      }
   }
   return rc;
}

/***************************************************************************/
/**
*  Driver initialization called when module loaded by kernel
*
*  @return  
*     0              Success
*     -ve            Error code
*/
static int __init usb_init( void )
{
   int err;

   printk( KERN_INFO "HAL Audio USB Driver: 1.0. Built %s %s\n", __DATE__, __TIME__ );

   /* NOTE: The USB interface is not synced with other HAL Audio interfaces.  Pass in 0 as the frame size
    *       to indicate this.
    */
   err = halAudioAddInterface( &halaudio_usb_ops, 1, "USB", 
         USB_FRAME_PERIOD * 1000, 0 /* synchronize */, &gInterfHandle );
   if( err )
   {
      printk( KERN_ERR "USB: failed to install the audio interface %d!\n", err );
      return err;
   }

   return 0;
}

/***************************************************************************/
/**
*  Driver destructor routine. Frees all resources
*/
static void __exit usb_exit( void )
{
   halAudioDelInterface( gInterfHandle );
}

module_init( usb_init );
module_exit( usb_exit );

MODULE_AUTHOR( "Broadcom" );
MODULE_DESCRIPTION( "HAL Audio USB Driver" );
MODULE_LICENSE( "GPL v2" );

