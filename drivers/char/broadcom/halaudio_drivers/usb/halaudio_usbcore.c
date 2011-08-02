/*****************************************************************************
* Copyright 2006 - 2009 Broadcom Corporation.  All rights reserved.
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

/* ---- Include Files ---------------------------------------- */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>

#include <linux/usb.h>
#include <linux/proc_fs.h>                   /* For /proc/audio */
#include <linux/sysctl.h>
#include <linux/vmalloc.h>
#include "halaudio_usbcore.h"

/* ---- Public Variables ------------------------------------- */

/* ---- Private Constants and Types -------------------------- */

#define USB_CORE_SAMP_WIDTH                  2         /* Number of bytes per sample                  */
#define USB_CORE_MAX_CHANNELS                2
#define USB_CORE_ISO_FRAMES_PER_URB          1
#define USB_CORE_NUM_URBS                    2

#define USB_CORE_DEFAULT_FREQ                16000
#define USB_CORE_DEFAULT_CHANNELS            1         /* 1 channel for mono or 2 channels for stereo */
#define USB_CORE_DEFAULT_FRAME_SIZE_1MS_MONO ((USB_CORE_DEFAULT_FREQ/1000) * USB_CORE_SAMP_WIDTH)
#define USB_CORE_DEFAULT_FRAME_SIZE_1MS      (USB_CORE_DEFAULT_CHANNELS * USB_CORE_DEFAULT_FRAME_SIZE_1MS_MONO)

/* Calculate dB value from raw value with rouding */
#define USB_CORE_DB_VALUE(val)               ((val) > 0 ? ((val)+128)/256 : ((val)-128)/256)

/* KNL Log debug
 */
#define USB_CORE_ENABLE_LOG                  0
#if USB_CORE_ENABLE_LOG
#include <linux/broadcom/knllog.h>
#define USB_CORE_LOG                         KNLLOG
#else
#define USB_CORE_LOG(c,d...)
#endif

#define USB_CORE_PROFILING_ENABLED             0
#if USB_CORE_PROFILING_ENABLED && defined(CONFIG_BCM_KNLLOG_SUPPORT)
#include <linux/broadcom/knllog.h>
#define USB_CORE_PROFILING(c,args...)          \
   if ( gKnllogIrqSchedEnable && KNLLOG_PROFILING ) { \
      KNLLOG(c, ## args); \
   }
#else
#define USB_CORE_PROFILING(c,args...)
#endif

/* Feature accessable in the feature unit
 */
enum {
	USB_FEATURE_NONE = 0,
	USB_FEATURE_MUTE,
	USB_FEATURE_VOLUME,
	USB_FEATURE_BASS,
	USB_FEATURE_MID,
	USB_FEATURE_TREBLE,
	USB_FEATURE_GEQ,
	USB_FEATURE_AGC,
	USB_FEATURE_DELAY,
	USB_FEATURE_BASSBOOST,
	USB_FEATURE_LOUDNESS,
   USB_FEATURE_MAX
};

/* Length in bytes of the argument to each feature unit
 */
static int fuLength[] =
{
   0,  /* USB_FEATURE_NONE       */
   1,  /* USB_FEATURE_MUTE       */
   2,  /* USB_FEATURE_VOLUME     */
   1,  /* USB_FEATURE_BASS       */
   1,  /* USB_FEATURE_MID        */
   1,  /* USB_FEATURE_TREBLE     */
   1,  /* USB_FEATURE_GEQ        */
   1,  /* USB_FEATURE_AGC        */
   2,  /* USB_FEATURE_DELAY      */
   1,  /* USB_FEATURE_BASSBOOST  */
   1   /* USB_FEATURE_LOUDNESS   */
};

/* Feature unit strings
 */
static char *fuStrings[] =
{
   "NONE",
   "MUTE",
   "VOLUME",
   "BASS",
   "MID",
   "TREBLE",
   "GEQ",
   "AGC",
   "DELAY",
   "BASSBOOST",
   "LOUDNESS"
};

/* Used to keep track of usable alternate settings
 */
struct usb_core_alts
{
   struct usb_host_interface *pAlts;
   struct formatTypeIDesc    *pFormatDesc;

   uint16_t wMaxPacketSize;
   uint8_t  bEndpointAddress;
};

/* Feature Unit specifics
 */
struct usb_core_fu
{
   uint8_t id;
   uint8_t channels[ USB_FEATURE_MAX ];
   int     numChannels;

   struct usb_core_volume_info  volInfo;
};

/* USB Core interface information
 */
struct usb_core_intf
{
   unsigned int sampFreq;        /* Sampling frequency in Hz           */
   unsigned int frameSize1ms;    /* Frame size in bytes for 1 ms frame */
   unsigned int numberChannels;  /* 1 for mono and 2 for stereo        */

   int number;                   /* Interface Number                   */
   int numAlts;                  /* Number of Alternates available     */
   int activeAlt;                /* Current active alternate setting   */

   /* At least 2 URBs are required to maintain an ISO
    * connection.
    */
   struct urb             *pUrb[USB_CORE_NUM_URBS];

   /* An interface may have several alternate settings.
    * The following tracks the ones USB Core can make use
    * of.
    */
   struct usb_core_alts   *pAlt;

   /* Feature Unit specifics
    */
   struct usb_core_fu      fu;
};

/* USB Core state information
 */
struct usb_core_info
{
   int                      initialized;
   int                      disconnecting;
   atomic_t                 enable;
   atomic_t                 prepared;
   int                      numInterf;
   int                      claimedInterfaces;

   /* Pointer to USB device information
    */
   struct usb_device       *pDev;

   /* Audio in/out information
    */
   struct usb_core_intf     inIntf;
   struct usb_core_intf     outIntf;

   /* Debug information
    */
   int                      loop_ig2eg;

   /* Mutex to protect I/O access.
    */
   struct semaphore         mutex;
};

/* Supported sampling rates
 */
static int gUsbCoreSupportedSamplingRates[] =
{
   8000, 16000, /*22050,*/ 32000, /*40000, 44100,*/ 48000, 0 /* NULL terminated */
};

/* ---- Private Variables ------------------------------------ */

/* USB Core information
 */
static struct usb_core_info gUsbCore;

/* Callbacks
 */
static USB_CORE_CBS gCbs;

/* ---- Private Function Prototypes -------------------------- */

static int  usbCoreInfoReadProc( char *buf, char **start, off_t offset, int count, int *eof, void *data );
static int  usbCoreInterfReadProc( char *buf, char **start, off_t offset, int count, int *eof, void *data );
static int  usbCoreFuReadProc( char *buf, char **start, off_t offset, int count, int *eof, void *data );

static int  usbCoreProbe( struct usb_interface *intf, const struct usb_device_id *id );
static void usbCoreDisconnect( struct usb_interface *intf );
static void usbCoreEgressHandler( struct urb *urb );
static void usbCoreIngressHandler( struct urb *urb );

static int  processAudioStream( struct usb_device *pDev, int intNum );
static int  isSupported( struct usb_core_intf *pIf, int freqHz, int numChans );
static int  isConnected( struct usb_core_intf *pIf );
static int  initInterface( struct usb_core_intf *pIf, int freqHz, int numChans );
static int  initUrbs( struct usb_core_intf *pIf );
static int  startUrbs( struct usb_core_intf *pIf );
static void disableUrbs( struct usb_core_intf *pIf );
static void freeUrbs( struct usb_core_intf *pIf );
static int  setFreq( struct usb_core_alts *pAlts, int freqHz );
static int  findFeatureUnits( int8_t *pDescs, int8_t *pEndDesc );
static int8_t *findDesc( int8_t *pDescs, int8_t desc, int8_t *pEndDesc );
static int8_t *findUnit( int8_t *pDescs, int8_t unit, int8_t *pEndDesc );
static int  setFeature( uint8_t feature, uint8_t request, uint8_t unit, uint8_t channel, int val );
static int  getFeature( uint8_t feature, uint8_t request, uint8_t unit, uint8_t channel );
static int  getVolume( struct usb_core_intf *pIf );

static void cleanupResources( void );
static void createProcEntries( void );
static void cleanupProcEntries( void );

/* For proc/sys
 */
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,31)
static int  proc_doModify( ctl_table *table, int write,
      void __user *buffer, size_t *lenp, loff_t *ppos );
#else
static int  proc_doModify( ctl_table *table, int write, struct file *filp,
      void __user *buffer, size_t *lenp, loff_t *ppos );
#endif

static struct ctl_table gSysCtlUsbCore[] =
{
   {
      .procname      = "igr_chans",
      .data          = &gUsbCore.inIntf.numberChannels,
      .maxlen        = sizeof(int),
      .mode          = 0644,
      .proc_handler  = &proc_doModify,
      .extra1        = &gUsbCore.inIntf,
      .extra2        = &gCbs.infoIn,
   },
   {
      .procname      = "egr_chans",
      .data          = &gUsbCore.outIntf.numberChannels,
      .maxlen        = sizeof(int),
      .mode          = 0644,
      .proc_handler  = &proc_doModify,
      .extra1        = &gUsbCore.outIntf,
      .extra2        = &gCbs.infoOut,
   },
   {
      .procname      = "igr_freq",
      .data          = &gUsbCore.inIntf.sampFreq,
      .maxlen        = sizeof(int),
      .mode          = 0644,
      .proc_handler  = &proc_doModify,
      .extra1        = &gUsbCore.inIntf,
      .extra2        = &gCbs.infoIn,
   },
   {
      .procname      = "egr_freq",
      .data          = &gUsbCore.outIntf.sampFreq,
      .maxlen        = sizeof(int),
      .mode          = 0644,
      .proc_handler  = &proc_doModify,
      .extra1        = &gUsbCore.outIntf,
      .extra2        = &gCbs.infoOut,
   },
   {}
};

static struct ctl_table gSysCtl[] =
{
    { 
      .procname   = "usbcore", 
      .mode       = 0555, 
      .child      = gSysCtlUsbCore, 
    },
    {}
};

static struct ctl_table_header *gSysCtlHeader;
static struct proc_dir_entry *gProcDir;

/* USB Core driver specifics
 */
static struct usb_device_id usbCoreDeviceIds[] =
{
   {
      .match_flags         = (USB_DEVICE_ID_MATCH_INT_CLASS | USB_DEVICE_ID_MATCH_INT_SUBCLASS),
      .bInterfaceClass     = USB_CLASS_AUDIO,
      .bInterfaceSubClass  = USB_SUBCLASS_AUDIO_CONTROL
   },
   {}
};

/* USB Core driver callbacks
 */
static struct usb_driver usbCoreDriver =
{
   .name       = "HAL Audio USB",
   .probe      = usbCoreProbe,
   .disconnect = usbCoreDisconnect,
   .id_table   = usbCoreDeviceIds,
};

/* ---- Functions -------------------------------------------- */

/* For endianness swap
 */
#define swap16(val) ((uint16_t)((((uint16_t)(val) & (uint16_t)0x00ffU) << 8) | (((uint16_t)(val) & (uint16_t)0xff00U) >> 8)))

/***************************************************************************/
/**
*  Register callbacks
*
*  @return     Nothing 
*/
void usbCoreRegisterCb( USB_CORE_CBS *pCbs )
{
   memcpy( &gCbs, pCbs, sizeof( gCbs ) );
}

/***************************************************************************/
/**
*  Deregister callbacks
*
*  @return     Nothing 
*/
void usbCoreDeregisterCb( void )
{
   memset( &gCbs, 0, sizeof( gCbs ) );
}

/***************************************************************************/
/**
*  USB Core driver probe routine
*
*  @return  
*     0        success
*     -ve      failure code
*/
static int usbCoreProbe( struct usb_interface *intf, const struct usb_device_id *id )
{
   struct usb_host_interface *pAlts;
   struct csAcIntHeaderDesc  *pCsAcIntHeaderDesc;

   int sampFreq, numberChannels;
   int i, rc;

   (void) id;

   USB_CORE_LOG( "Probe called" );

   /* Check if we are already working with a USB device.
    */
   if( gUsbCore.pDev )
   {
      printk( KERN_INFO "USB Core: Did not claim USB device since one already exists.\n" );
      return -ENODEV;
   }

   /* This probe function should only be called for interfaces of class USB_CLASS_AUDIO
    * and sub-class USB_SUBCLASS_AUDIO_CONTROL.  The AudioControl interface is automatically
    * claimed.  Next, look for and claim the AudioStreaming interfaces.
    */
   gUsbCore.claimedInterfaces++;

   /* Find Class-Specific AC Interface Descriptor to determine number of AS interfaces.
    */
   pAlts = &intf->altsetting[0];

   if( !pAlts->extralen )
   {
      printk( KERN_ERR "USB Core: No AC interface descriptors found.\n" );
      return -ENODEV;
   }

   pCsAcIntHeaderDesc = (struct csAcIntHeaderDesc*)pAlts->extra;

   if( pCsAcIntHeaderDesc->bDescriptorType != USB_DT_CS_INTERFACE &&
       pCsAcIntHeaderDesc->bDescriptorSubtype != HEADER )
   {
      printk( KERN_ERR "USB Core: HEADER AC interface descriptor found.\n" );
      return -ENODEV;
   }

   if( down_interruptible( &gUsbCore.mutex ) )
   {
      printk( KERN_ERR "USB Core: Probe cannot obtain mutex.\n" );
      return -ENODEV;
   }

   /* Find the USB device and save it.
    */
   gUsbCore.pDev = interface_to_usbdev( intf );

   gUsbCore.numInterf = pCsAcIntHeaderDesc->bInCollection + 1;

   /* Process the Audio Streams that are available.
    */
   for( i = 0; i < pCsAcIntHeaderDesc->bInCollection; i++ )
   {
      rc = processAudioStream( gUsbCore.pDev, *(pCsAcIntHeaderDesc->baInterfaceNr+i) );
      if( rc )
      {
         printk( KERN_ERR "USB Core [Interf=%u]: Failed to processAudioStream rc=%i.\n", *(pCsAcIntHeaderDesc->baInterfaceNr+i), rc );
         goto probe_cleanup;
      }
   }

   /* Initialize the IN and OUT interfaces. Must do the IN interface first 
    * otherwise the USB Core device may fail setting the sampling frequency
    * of the OUT interface.
    *
    * Try to bring up the interfaces with the same number of channels and sampling
    * frequency from a previous probe.
    */
   sampFreq       = gUsbCore.inIntf.sampFreq;
   numberChannels = gUsbCore.inIntf.numberChannels;
   rc = initInterface( &gUsbCore.inIntf, sampFreq, numberChannels );
   if( rc )
   {
      /* Don't error because we cannot initialize the interface.  We can possibly continue with some functionality or
       * to allow a peak into USB device info by the user.
       */
      printk( KERN_INFO "USB Core: Failed to initialize IN interface\n" );
   }

   /* Report changes to callbacks.
    */
   if( gCbs.infoIn && (sampFreq != gUsbCore.inIntf.sampFreq || numberChannels != gUsbCore.inIntf.numberChannels) )
   {
      gCbs.infoIn( gUsbCore.inIntf.sampFreq, gUsbCore.inIntf.numberChannels );
   }

   sampFreq       = gUsbCore.outIntf.sampFreq;
   numberChannels = gUsbCore.outIntf.numberChannels;
   rc = initInterface( &gUsbCore.outIntf, sampFreq, numberChannels );
   if( rc )
   {
      /* Don't error because we cannot initialize the interface.  We can possibly continue with some functionality or
       * to allow a peak into USB device info by the user.
       */
      printk( KERN_INFO "USB Core: Failed to initialize OUT interface\n" );
   }

   /* Report changes to callbacks.
    */
   if( gCbs.infoOut && (sampFreq != gUsbCore.outIntf.sampFreq || numberChannels != gUsbCore.outIntf.numberChannels) )
   {
      gCbs.infoOut( gUsbCore.outIntf.sampFreq, gUsbCore.outIntf.numberChannels );
   }

   /* Build information to access the feature unit.
    */
   rc = findFeatureUnits( pAlts->extra, pAlts->extra + pAlts->extralen );
   if( rc )
   {
      /* Possibly no feature unit
       */
      printk( KERN_INFO "USB Core: Failed to find feature units\n" );
   }

   /* Create proc entries
    */
   createProcEntries();

   if( atomic_read( &gUsbCore.enable ) )
   {
      struct usb_core_intf *pIf;

      pIf = &gUsbCore.inIntf;
      if( pIf->number != -1 )
      {
         /* Choose correct alternate setting.
          */
         usb_set_interface( gUsbCore.pDev, pIf->number, pIf->pAlt[pIf->activeAlt].pAlts->desc.bAlternateSetting );

         rc = startUrbs( pIf );
         if( rc )
         {
            /* Don't error because we cannot start the URBs.  We can possibly continue with some functionality or
             * to allow a peak into USB device info by the user.
             */
            printk( KERN_INFO "USB Core: Failed to start URBs in IN interface\n" );
         }
      }

      pIf = &gUsbCore.outIntf;
      if( pIf->number != -1 )
      {
         /* Choose correct alternate setting.
          */
         usb_set_interface( gUsbCore.pDev, pIf->number, pIf->pAlt[pIf->activeAlt].pAlts->desc.bAlternateSetting );

         rc = startUrbs( pIf );
         if( rc )
         {
            /* Don't error because we cannot start the URBs.  We can possibly continue with some functionality or
             * to allow a peak into USB device info by the user.
             */
            printk( KERN_INFO "USB Core: Failed to start URBS in OUT interface\n" );
         }
      }
   }

   up( &gUsbCore.mutex );

   return 0;

probe_cleanup:

   up( &gUsbCore.mutex );

   cleanupResources();

   /* Release claim on interfaces that may have been claimed
    */
   if( gUsbCore.inIntf.number != -1 )
   {
      usb_driver_release_interface( &usbCoreDriver, usb_ifnum_to_if( gUsbCore.pDev, gUsbCore.inIntf.number ) );
   }
   if( gUsbCore.outIntf.number != -1 )
   {
      usb_driver_release_interface( &usbCoreDriver, usb_ifnum_to_if( gUsbCore.pDev, gUsbCore.outIntf.number ) );
   }

   return -ENODEV;
}

/***************************************************************************/
/**
*  USB Core driver disconnect routine
*
*  @return  
*     0        success
*     -ve      failure code
*/
static void usbCoreDisconnect( struct usb_interface *intf )
{
   (void) intf;

   USB_CORE_LOG( "Disconnect called" );

   /* Provide disconnect information through cbk.
    */
   if( gCbs.disconnect )
   {
      gCbs.disconnect();
   }

   /* Disconnect is called for all the interfaces that
    * is claimed by the driver.
    */
   if( gUsbCore.pDev )
   {
      gUsbCore.disconnecting = 1;

      gUsbCore.claimedInterfaces--;
      if( gUsbCore.claimedInterfaces == 0 )
      {
         /* Clean up all resources
          */
         cleanupProcEntries();
         cleanupResources();
      }
   }
}

/***************************************************************************/
/**
*  Prepare routine
*
*  @return 
*     0     success
*     -1    failed to enable
*/
int usbCorePrepare( void )
{
   struct usb_core_intf *pIf;

   if( atomic_read( &gUsbCore.prepared ) )
   {
      /* Already enabled or prepared
       */
      return -EBUSY;
   }

   pIf = &gUsbCore.inIntf;
   if( pIf->number != -1 )
   {
      /* Choose correct alternate setting.
       */
      usb_set_interface( gUsbCore.pDev, pIf->number, pIf->pAlt[pIf->activeAlt].pAlts->desc.bAlternateSetting );
   }

   pIf = &gUsbCore.outIntf;
   if( pIf->number != -1 )
   {
      /* Choose correct alternate setting.
       */
      usb_set_interface( gUsbCore.pDev, pIf->number, pIf->pAlt[pIf->activeAlt].pAlts->desc.bAlternateSetting );
   }

   atomic_set( &gUsbCore.prepared, 1 );

   return 0;
}

/***************************************************************************/
/**
*  Enable USB Core
*
*  @return 
*     0     success
*     -1    failed to enable
*/
int usbCoreEnable( void )
{
   int rc;

   if ( !atomic_read( &gUsbCore.prepared ))
   {
      return -EPERM;
   }

   if( atomic_read( &gUsbCore.enable ) )
   {
      return -EBUSY;
   }

   rc = startUrbs( &gUsbCore.inIntf );
   if( rc )
   {
      printk( KERN_ERR "USB Core: Failed to start URBs in IN interface\n" );
      return rc;
   }

   rc = startUrbs( &gUsbCore.outIntf );
   if( rc )
   {
      printk( KERN_ERR "USB Core: Failed to start URBs in OUT interface\n" );
      return rc;
   }

   atomic_set( &gUsbCore.enable, 1 );

   return 0;
}

/***************************************************************************/
/**
*  Disable USB Core
*
*  @return 
*     0     success
*     -1    failed to enable
*/
int usbCoreDisable( void )
{
   if( !atomic_read( &gUsbCore.prepared ) )
   {
      return -EPERM;
   }
   
   atomic_set( &gUsbCore.enable, 0 );

   disableUrbs( &gUsbCore.inIntf );
   disableUrbs( &gUsbCore.outIntf );

   atomic_set( &gUsbCore.prepared, 0 );

   return 0;
}

/***************************************************************************/
/**
*  USB Core egress completion handler
*
*  @return  none
*/
static void usbCoreEgressHandler( struct urb *urb )
{
   struct usb_core_intf *pI = (struct usb_core_intf*)urb->context;

   int rc;

   if( !atomic_read( &gUsbCore.enable ) || gUsbCore.disconnecting )
   {
      return;
   }

   USB_CORE_PROFILING( "in  [usb]");
   if( urb->status )
   {
      USB_CORE_LOG( "USB Core: Egress URB error status=%i", urb->status );
   }

   /* Call egress callback if it exists, otherwise play out zeros
    */
   if( gCbs.mediaOut )
   {
      gCbs.mediaOut( urb->transfer_buffer, pI->frameSize1ms );
   }
   else
   {
      memset( urb->transfer_buffer, 0, pI->frameSize1ms );
   }

#if defined( __BIG_ENDIAN )
   {
      int j;
      int16_t *pBuf = urb->transfer_buffer;

      for( j = 0; j < pI->frameSize1ms/2; j++ )
      {
         pBuf[j] = swap16( pBuf[j] );
      }
   }
#endif

   /* Re-submit the URB
    */
   rc = usb_submit_urb( urb, GFP_ATOMIC );
   if( rc )
   {
      USB_CORE_LOG( "USB Core: Egress URB submit failure rc=%i", rc );
   }
   USB_CORE_PROFILING( "out [usb]");
}

/***************************************************************************/
/**
*  USB Core ingress handler
*
*  @return  none
*/
static void usbCoreIngressHandler( struct urb *urb )
{
   struct usb_core_intf *pI = (struct usb_core_intf*)urb->context;

   int rc = 0;

   if( !atomic_read( &gUsbCore.enable ) || gUsbCore.disconnecting )
   {
      return;
   }

   USB_CORE_PROFILING( "in  [usb]");
   if( urb->status )
   {
      USB_CORE_LOG( "USB Core: Ingress URB error status=%i", urb->status );
   }

#if defined( __BIG_ENDIAN )
   {
      int j;
      int16_t *pBuf = urb->transfer_buffer;

      for( j = 0; j < pI->frameSize1ms/2; j++ )
      {
         pBuf[j] = swap16( pBuf[j] );
      }
   }
#endif

   /* Provide samples to ingress callback if it exists.
    */
   if( gCbs.mediaIn )
   {
      gCbs.mediaIn( urb->transfer_buffer, pI->frameSize1ms );
   }
     
   /* Re-submit the URB.
    */
   rc = usb_submit_urb( urb, GFP_ATOMIC );
   if( rc )
   {
      USB_CORE_LOG( "USB Core: Ingress URB submit error rc=%i", rc );
   }
   USB_CORE_PROFILING( "out [usb]");
}

/***************************************************************************/
/**
*  Set ingress sampling frequency
*
*  @return 
*     0        Success
*     -ENODEV  PCM has not be initiialized
*     -EBUSY   PCM is currently running
*     -EINVAL  Invalid channel number
*/
int usbCoreSetIngressFreq( 
   int freqHz                       /**< (i) sampling frequency in Hz */
)
{
   int rc = 0;

   USB_CORE_LOG( "New sampling frequency %u Hz requested\n", freqHz );

   if( !gUsbCore.initialized )
   {
      return -ENODEV;
   }

   if( !gUsbCore.pDev )
   {
      return -ENODEV;
   }

   if( atomic_read( &gUsbCore.enable ) || atomic_read( &gUsbCore.prepared ) )
   {
      /* Cannot change sampling frequency if currently running
       */
      return -EBUSY;
   }

   if( down_interruptible( &gUsbCore.mutex ) )
   {
      printk( KERN_ERR "USB Core [Interf=%u]: Could not obtain mutex\n", gUsbCore.inIntf.number );
      return -ENODEV;
   }

   /* If the sampling frequency is supported, go ahead and change it for both channels
    */
   if( isSupported( &gUsbCore.inIntf, freqHz, gUsbCore.inIntf.numberChannels )  >= 0 )
   {
      rc = initInterface( &gUsbCore.inIntf, freqHz, gUsbCore.inIntf.numberChannels );
      if( rc )
      {
         printk( KERN_ERR "USB Core [Interf=%u]: Could not set sampling frequency of USB_DIR_IN rc=%i\n", gUsbCore.inIntf.number, rc );
         goto setfreq_cleanup;
      }

      /* Report changes to callbacks.
       */
      if( gCbs.infoIn )
      {
         gCbs.infoIn( gUsbCore.inIntf.sampFreq, gUsbCore.inIntf.numberChannels );
      }
   }

setfreq_cleanup:

   up( &gUsbCore.mutex );

   return rc;
}

/***************************************************************************/
/**
*  Set egress sampling frequency
*
*  @return 
*     0        Success
*     -ENODEV  PCM has not be initiialized
*     -EBUSY   PCM is currently running
*     -EINVAL  Invalid channel number
*/
int usbCoreSetEgressFreq( 
   int freqHz                       /**< (i) sampling frequency in Hz */
)
{
   int rc = 0;

   USB_CORE_LOG( "New sampling frequency %u Hz requested\n", freqHz );

   if( !gUsbCore.initialized )
   {
      return -ENODEV;
   }

   if( !gUsbCore.pDev )
   {
      return -ENODEV;
   }

   if( atomic_read( &gUsbCore.enable ) || atomic_read( &gUsbCore.prepared ) )
   {
      /* Cannot change sampling frequency if currently running
       */
      return -EBUSY;
   }

   if( down_interruptible( &gUsbCore.mutex ) )
   {
      printk( KERN_ERR "USB Core [Interf=%u]: Could not obtain mutex\n", gUsbCore.outIntf.number );
      return -ENODEV;
   }

   /* If the sampling frequency is supported, go ahead and change it for both channels
    */
   if( isSupported( &gUsbCore.outIntf, freqHz, gUsbCore.outIntf.numberChannels )  >= 0 )
   {
      rc = initInterface( &gUsbCore.outIntf, freqHz, gUsbCore.outIntf.numberChannels );
      if( rc )
      {
         printk( KERN_ERR "USB Core [Interf=%u]: Could not set sampling frequency of USB_DIR_OUT rc=%i\n", gUsbCore.outIntf.number, rc );
         goto setfreq_cleanup;
      }

      /* Report changes to callbacks.
       */
      if( gCbs.infoOut )
      {
         gCbs.infoOut( gUsbCore.outIntf.sampFreq, gUsbCore.outIntf.numberChannels );
      }
   }

setfreq_cleanup:

   up( &gUsbCore.mutex );

   return rc;
}

/***************************************************************************/
/**
*  Retrieve ingress sampling frequency
*
*  @return     sampling frequency
*/
int usbCoreGetIngressFreq( void )
{
   /* Return ingress sampling frequency
    */
   return gUsbCore.inIntf.sampFreq;
}

/***************************************************************************/
/**
*  Retrieve egress sampling frequency
*
*  @return     sampling frequency
*/
int usbCoreGetEgressFreq( void )
{
   /* Return ingress sampling frequency
    */
   return gUsbCore.outIntf.sampFreq;
}

/***************************************************************************/
/**
*  Retrieve ingress number of channels
*
*  @return     number of channels
*/
int usbCoreGetIngressChannels( void )
{
   /* Return ingress sampling frequency
    */
   return gUsbCore.inIntf.numberChannels;
}

/***************************************************************************/
/**
*  Retrieve egress number of channels
*
*  @return     number of channels
*/
int usbCoreGetEgressChannels( void )
{
   /* Return egress sampling frequency
    */
   return gUsbCore.outIntf.numberChannels;
}

/***************************************************************************/
/**
*  Retrieve ingress frame size
*
*  @return     frame size
*/
int usbCoreGetIngressFrameSize1ms( void )
{
   /* Return frame size
    */
   return gUsbCore.inIntf.frameSize1ms;
}

/***************************************************************************/
/**
*  Retrieve egress frame size
*
*  @return     frame size
*/
int usbCoreGetEgressFrameSize1ms( void )
{
   /* Return frame size
    */
   return gUsbCore.outIntf.frameSize1ms;
}

/***************************************************************************/
/**
*  Check if an interface is connected
*
*  @return
*     1 - Connected
*     0 - No connection
*/
int isConnected( struct usb_core_intf *pIf )
{
   /* Return frame size
    */
   return (pIf->number != - 1);
}

/***************************************************************************/
/**
*  Check if ingress is connected
*
*  @return
*     1 - Connected
*     0 - No connection
*/
int usbCoreIngressIsConnected( void )
{
   /* Return frame size
    */
   return isConnected( &gUsbCore.inIntf );
}

/***************************************************************************/
/**
*  Check if ingress is connected
*
*  @return
*     1 - Connected
*     0 - No connection
*/
int usbCoreEgressIsConnected( void )
{
   /* Return frame size
    */
   return isConnected( &gUsbCore.outIntf );
}

/***************************************************************************/
/**
*  Set volume
*
*  @return
*     0        success
*     -ve      failure code
*/
int setVolume( struct usb_core_intf *pIf, int vol )
{
   struct usb_core_fu *pF = &pIf->fu;

   int i, rc;
   int feature = USB_FEATURE_VOLUME;

   if( !isConnected( pIf ) )
   {
      return -ENODEV;
   }

   if( vol == pF->volInfo.cur )
   {
      return 0;
   }

   if( vol == 0x8000 )
   {
      /* Mute instead
       */
      feature         = USB_FEATURE_MUTE;
      pF->volInfo.cur = vol;
      vol             = 1;
   }
   else
   {
      /* Ensure the value is valid */
      if( vol < pF->volInfo.min )
      {
         vol = pF->volInfo.min;
      }
      else if( vol > pF->volInfo.max )
      {
         vol = pF->volInfo.max;
      }

      if( pF->volInfo.cur == 0x8000 )
      {
         /* Un-mute first
          */
         for( i = 0; i < 8; i++ )
         {
            if( pF->channels[USB_FEATURE_MUTE] & (1<<i) )
            {
               rc = setFeature( USB_FEATURE_MUTE, SET_CUR, pF->id, i, 0 ); 
               if( rc < 0 )
               {
                  printk( KERN_ERR "USB Core: Could not un-mute ch=%u rc=%i\n", i, rc );
                  return -EIO;
               }
            }
         }
      }

      pF->volInfo.cur = vol;
   }

   /* Must set all the channels
    */
   for( i = 0; i < pF->numChannels; i++ )
   {
      if( pF->channels[feature] & (1<<i) )
      {
         rc = setFeature( feature, SET_CUR, pF->id, i, vol ); 
         if( rc < 0 )
         {
            printk( KERN_ERR "USB Core: Could not set ingress volume ch=%u rc=%i\n", i, rc );
            return -EIO;
         }
      }
   }

   return 0;
}

/***************************************************************************/
/**
*  Set ingress volume
*
*  @return
*     0        success
*     -ve      failure code
*/
int usbCoreSetIngressVolume( int vol )
{
   return setVolume( &gUsbCore.inIntf, vol );
}

/***************************************************************************/
/**
*  Set egress volume
*
*  @return
*     0        success
*     -ve      failure code
*/
int usbCoreSetEgressVolume( int vol )
{
   return setVolume( &gUsbCore.outIntf, vol );
}

/***************************************************************************/
/**
*  Get volume
*
*  @return
*     0        success
*     -ve      failure code
*/
static int getVolume( struct usb_core_intf *pIf )
{
   struct usb_core_fu *pF = &pIf->fu;

   int i, minchn = 0;

   if( !isConnected( pIf ) )
   {
      return -ENODEV;
   }

   /* Look for 1 of the channels and get the volume.
    * Assume all other channels have the same setting.
    */
   for( i = 0; i < pF->numChannels; i++ )
   {
      if( pF->channels[USB_FEATURE_VOLUME] & (1<<i) )
      {
         minchn = i;
         break;
      }
   }

   if( i == pF->numChannels )
   {
      /* Could not find any volume feature */
      return -ENODEV;
   }

   pIf->fu.volInfo.cur = getFeature( USB_FEATURE_VOLUME, GET_CUR, pF->id, minchn );
   pIf->fu.volInfo.min = getFeature( USB_FEATURE_VOLUME, GET_MIN, pF->id, minchn );
   pIf->fu.volInfo.max = getFeature( USB_FEATURE_VOLUME, GET_MAX, pF->id, minchn );
   pIf->fu.volInfo.res = getFeature( USB_FEATURE_VOLUME, GET_RES, pF->id, minchn );

   return 0;
}

/***************************************************************************/
/**
*  Get ingress volume
*
*  @return
*     0        success
*     -ve      failure code
*/
int usbCoreGetIngressVolume( struct usb_core_volume_info *pVolInfo )
{
   memcpy( pVolInfo, &gUsbCore.inIntf.fu.volInfo, sizeof( *pVolInfo ) );

   return 0;
}

/***************************************************************************/
/**
*  Get egress volume
*
*  @return
*     0        success
*     -ve      failure code
*/
int usbCoreGetEgressVolume( struct usb_core_volume_info *pVolInfo )
{
   memcpy( pVolInfo, &gUsbCore.outIntf.fu.volInfo, sizeof( *pVolInfo ) );

   return 0;
}

/***************************************************************************/
/**
*  Set feature unit value
*
*  @return
*     0        success
*     -ve      failure code
*/
static int setFeature( uint8_t feature, uint8_t request, uint8_t unit, uint8_t channel, int val )
{
   struct usb_device *pDev = gUsbCore.pDev;

   int     rc;
   int     len = fuLength[ feature ];
   int8_t *pBuf = NULL;

   pBuf = kmalloc( len, GFP_KERNEL );
   if( !pBuf )
   {
      printk( KERN_ERR "USB Core [feature=%u][ch=%u]: Could not allocate mem for setFeature.\n", feature, channel );
      return -ENOMEM;
   }

   pBuf[0] = val & 0xff;
   if( len > 1 )
   {
      pBuf[1] = (val>>8) & 0xff;
   }

   rc = usb_control_msg( 
         pDev, 
         (PIPE_CONTROL<<30) | (0<<15) | (pDev->devnum<<8), 
         request, 
         USB_RECIP_INTERFACE | USB_TYPE_CLASS | USB_DIR_OUT,
         (uint16_t)feature<<8 | channel,
         (uint16_t)unit<<8 | 0,
         pBuf, 
         len, 
         100 );

   kfree( pBuf );

   return rc;
} 

/***************************************************************************/
/**
*  Get feature unit value
*
*  @return     Feature unit value
*/
static int getFeature( uint8_t feature, uint8_t request, uint8_t unit, uint8_t channel )
{
   struct usb_device *pDev = gUsbCore.pDev;

   int      rc;
   int      retval;
   int      len = fuLength[ feature ];
   int8_t  *pBuf = NULL;

   pBuf = kmalloc( len, GFP_KERNEL );
   if( !pBuf )
   {
      printk( KERN_ERR "USB Core [feature=%u][ch=%u]: Could not allocate mem for getFeature.\n", feature, channel );
      return -ENOMEM;
   }

   rc = usb_control_msg(
            pDev,
            (PIPE_CONTROL<<30) | (0<<15) | (pDev->devnum<<8) | USB_DIR_IN,
            request,
            USB_RECIP_INTERFACE | USB_TYPE_CLASS | USB_DIR_IN,
            (uint16_t)feature<<8 | channel,
            (uint16_t)unit<<8 | 0,
            pBuf,
            len, 
            100 );
   if( rc < 0 )
   {
      printk( KERN_ERR "USB Core [feature=%u][ch=%u]: Could not get egress feature rc=%i\n", feature, channel, rc );
   }

   retval = pBuf[0];
   if ( len > 1 )
   {
      /* Assume 16-bit max length */
      retval &= 0xff;
      retval |= pBuf[1] << 8;
   }

   /*printk( "%s: pBuf=0x%x 0x%x len=%i retval=0x%x", __FUNCTION__, pBuf[0] & 0xff, pBuf[1] & 0xff, len, retval ); */

   kfree( pBuf ); 

   return retval;
}

/***************************************************************************/
/**
*  Scan through Audio Stream descriptors to see if USB Core can make
*  use of it.
*
*  @return  
*     0        success
*     -ve      failure code
*/
static int processAudioStream( struct usb_device *pDev, int intNum )
{
   struct csAsIntDesc     *pCsAsIntDesc;
   struct formatTypeIDesc *pFormatTypeIDesc;
   struct usb_core_intf   *pIf;
   struct usb_interface   *pIntf;

   int j, rc;

   /* Find the interface based on interface number.
    */
   pIntf = usb_ifnum_to_if( pDev, intNum );

   /* Do several checks to ensure we can use the AS interface.
    */

   /* Make sure it is not already claimed.
    */
   if( usb_interface_claimed( pIntf ) )
   {
      printk( KERN_INFO "USB Core: [Interf=%u] Interface already claimed.\n", intNum );
      return -ENODEV;
   }

   /* Scan through the alternate settings to find ones we can use.
    */
   for( j = 0; j < pIntf->num_altsetting; j++ )
   {
      struct usb_host_endpoint        *pEpt;
      struct usb_host_interface       *pAlts;
      struct usb_interface_descriptor *pAltsDesc;
      uint16_t  format;
      int      *pNewBuf;

      pAlts     = &pIntf->altsetting[j];
      pAltsDesc = &pAlts->desc;

      /* Ignore interfaces that are not supported including MIDI.
       */
      if( pAltsDesc->bInterfaceClass != USB_CLASS_AUDIO ||
          pAltsDesc->bInterfaceSubClass != USB_SUBCLASS_AUDIO_STREAMING )
      {
         USB_CORE_LOG( "USB Core [Interf=%u][alt=%u]: InterfaceClass=%u InterfaceSubClass=%u not supported.\n", intNum, j, pAltsDesc->bInterfaceClass, pAltsDesc->bInterfaceSubClass );
         continue;
      }

      /* Must have 1 endpoint.
       */
      if( pAltsDesc->bNumEndpoints != 1 )
      {
         USB_CORE_LOG( "USB Core [Interf=%u][alt=%u]: %u endpoints. Expect 1.\n", intNum, j, pAltsDesc->bNumEndpoints );
         continue;
      }

      /* Now look at AS descriptors
       */
      if( !pAlts->extralen )
      {
         USB_CORE_LOG( "USB Core [Interf=%u][alt=%u]: No AS interface descriptors found.\n", intNum, j );
         continue;
      }

      pCsAsIntDesc = (struct csAsIntDesc*)pAlts->extra;

      if( pCsAsIntDesc->bDescriptorType != USB_DT_CS_INTERFACE ||
          pCsAsIntDesc->bDescriptorSubtype != AS_GENERAL )
      {
         USB_CORE_LOG( "USB Core [Interf=%u][alt=%u]: Could not find AS interface descriptor AS_GENERAL.\n", intNum, j );
         continue;
      }

      /* Check the format.  Only support 16-bit PCM for now.
       */
      format = (pCsAsIntDesc->wFormatTag[1]<<8) | pCsAsIntDesc->wFormatTag[0];
      if( format != 0x1 /* PCM */ )
      {
         USB_CORE_LOG( "USB Core [Interf=%u][alt=%u]: Unsupported format=0x%04x. Only PCM is supported.\n", intNum, j, format );
         continue;
      }

      pFormatTypeIDesc = (struct formatTypeIDesc*)(pAlts->extra + pCsAsIntDesc->bLength);

      if( pFormatTypeIDesc->bDescriptorType != USB_DT_CS_INTERFACE ||
          pFormatTypeIDesc->bDescriptorSubtype != FORMAT_TYPE )
      {
         USB_CORE_LOG( "USB Core [Interf=%u][alt=%u]: Could not find AS interface descriptor FORMAT_TYPE.\n", intNum, j );
         continue;
      }

      if( pFormatTypeIDesc->bFormatType != 0x1 /* FORMAT_TYPE_I */ ||
          pFormatTypeIDesc->bSubframeSize != 2 ||
          pFormatTypeIDesc->bNrChannels > 2 ||
          pFormatTypeIDesc->bNrChannels < 1 )
      {
         USB_CORE_LOG( "USB Core [Interf=%u][alt=%u]: Unsupported bFormatType=%u, bSubframeSize=%u, or bNrChannels=%u.\n", intNum, j, pFormatTypeIDesc->bFormatType, pFormatTypeIDesc->bSubframeSize, pFormatTypeIDesc->bNrChannels );
         continue;
      }

      /* Grab the endpoint.
       */
      pEpt = &pAlts->endpoint[0];

      /* Check the interface is an ISOC interface.
       */
      if( !usb_endpoint_xfer_isoc( &pEpt->desc ) )
      {
         USB_CORE_LOG( "USB Core [Interf=%u][alt=%u]: Not ISOC.\n", intNum, j );
         continue;
      }

      /* Okay, since we passed all the checks, we can save this endpoint as one that can be used 
       * for the particular interface.
       */
      if( usb_endpoint_dir_in( &pEpt->desc ) )
      {
         pIf = &gUsbCore.inIntf;
      }
      else
      {
         pIf = &gUsbCore.outIntf;
      }

      if( pIf->number == -1 )
      {
         pIf->number = intNum;

         /* Reset the interface to alternate zero and claim it.
          */
         usb_set_interface( pDev, intNum, 0 );

         rc = usb_driver_claim_interface( &usbCoreDriver, pIntf, (void *)-1L );
         if( rc )
         {
            printk( KERN_ERR "USB Core [Interf=%u]: Could not claim interface\n", intNum );
            return -ENODEV;
         } 

         gUsbCore.claimedInterfaces++;
      }

      if( pIf->number != intNum )
      {
         printk( KERN_ERR "USB Core [Interf=%u][alt=%u]: Found more than one IN/OUT interface.\n", intNum, j );
         return -ENODEV;
      }

      /* Allocate memory to store alternate settings.
       */
      pNewBuf = vmalloc( (pIf->numAlts+1) * sizeof( *pIf->pAlt ) );
      if( !pNewBuf )
      {
         printk( KERN_ERR "USB Core: [Interf=%u] Cannot allocate memory to save alternate settings.\n", intNum );
         return -ENODEV;
      }

      if( pIf->pAlt )
      {
         memcpy( pNewBuf, pIf->pAlt, pIf->numAlts * sizeof( *pIf->pAlt ) );

         vfree( pIf->pAlt );
      }

      pIf->pAlt = (struct usb_core_alts*)pNewBuf;

      pIf->pAlt[ pIf->numAlts ].pAlts            = pAlts;
      pIf->pAlt[ pIf->numAlts ].pFormatDesc      = pFormatTypeIDesc;
      pIf->pAlt[ pIf->numAlts ].bEndpointAddress = pEpt->desc.bEndpointAddress;
      pIf->pAlt[ pIf->numAlts ].wMaxPacketSize   = pEpt->desc.wMaxPacketSize;

      pIf->numAlts++;

      USB_CORE_LOG( "USB Core [Interf=%u][alt=%u]: Added.\n", intNum, j ); 
   }

   return 0;
}

/***************************************************************************/
/**
*  Intialize an interface
*
*  @return  
*     0        success
*     -ve      failure code
*/
static int initInterface( struct usb_core_intf *pIf, int freqHz, int numChans )
{
   int i, rc;
   int altChoice;
   struct usb_core_alts *pAlts;

   if( pIf->number == -1 )
   {
      /* No interface available 
       */
      return 0;
   }

   /* Free URBs if they are allocated.
    */
   freeUrbs( pIf );
   
   altChoice = isSupported( pIf, freqHz, numChans );

   /* If we fail to find an alternate setting that supports requested
    * sampling frequency, choose another that is supported.
    */
   if( altChoice < 0 )
   {
      int *testFreq = gUsbCoreSupportedSamplingRates;
      
      /* Scan supported frequencies but keep number of 
       * channels constant.
       */
      while( *testFreq != 0 )
      {
         altChoice = isSupported( pIf, *testFreq, numChans );
         if( altChoice >= 0 )
         {
            freqHz = *testFreq;
            break;
         }

         testFreq++;
      }

      /* If we still fail, scan through different number of channels
       * as well.
       */
      if( altChoice < 0 )
      {
         testFreq = gUsbCoreSupportedSamplingRates;

         for( i = 1; i <= USB_CORE_MAX_CHANNELS; i++ )
         {
            if( i == numChans )
            {
               /* Don't bother scanning number of channels already done.
                */
               continue;
            }

            while( *testFreq != 0 )
            {
               altChoice = isSupported( pIf, *testFreq, i );
               if( altChoice >= 0 )
               {
                  freqHz   = *testFreq;
                  numChans = i;
                  break;
               }

               testFreq++;
            }

            if( altChoice >= 0  )
            {
               break;
            }
         }
      }
   }

   if( altChoice < 0 )
   {
      printk( KERN_ERR "USB Core [Interf=%u] Failed to find usable alternate setting.\n", pIf->number );
      return -EINVAL;
   }

   USB_CORE_LOG( "USB Core [Interf=%u][activeAlt=%u] Selected an alternate setting.\n", pIf->number, altChoice );

   pIf->numberChannels = numChans;
   pIf->sampFreq       = freqHz;
   pIf->frameSize1ms   = numChans * (freqHz/1000) * USB_CORE_SAMP_WIDTH;

   /* Save the alternate setting found.
    */
   pIf->activeAlt = altChoice;

   pAlts = &pIf->pAlt[altChoice];

   /* Initialize sampling frequency
    */
   rc = setFreq( pAlts, freqHz );
   if( rc )
   {
      printk( KERN_ERR "USB Core [Interf=%u][activeAlt=%u] Failed to set sampling frequency rc=%i.\n", pIf->number, pIf->activeAlt, rc );
      return rc;
   }

   /* Initialize URBs for interface.
    */
   rc = initUrbs( pIf );
   if( rc )
   {
      printk( KERN_ERR "USB Core [Interf=%u][activeAlt=%u] Failed to initialize URB rc=%i.\n", pIf->number, pIf->activeAlt, rc );
      return rc;
   }

   return 0;
}

/***************************************************************************/
/**
*  Set the sampling frequency of a particular interface.
*
*  @return  
*     0        success
*     -ve      failure code
*/
static int setFreq( struct usb_core_alts *pAlts, int freqHz )
{
   int     rate;
   int     rc = 0;
   int     len = 3;
   int8_t *pBuf = NULL;
   struct usb_device *pDev = gUsbCore.pDev;

   pBuf = kmalloc( len, GFP_KERNEL );
   if( !pBuf )
   {
      printk( KERN_ERR "USB Core: Could not allocate mem for setFreq.\n" );
      return -ENOMEM;
   }

   pBuf[0] = freqHz;
   pBuf[1] = freqHz >> 8;
   pBuf[2] = freqHz >> 16;

   rc = usb_control_msg( 
         pDev, 
         (PIPE_CONTROL<<30) | (0<<15) | (pDev->devnum<<8), 
         SET_CUR, 
         USB_TYPE_CLASS | USB_RECIP_ENDPOINT | USB_DIR_OUT,
         SAMPLING_FREQ_CONTROL << 8, 
         pAlts->bEndpointAddress, 
         pBuf, 
         len, 
         1000 );
   if( rc < 0 )
   {
      printk( KERN_ERR "USB Core [ept=0x%02x]: Could not set sampling frequency rc=%i\n", pAlts->bEndpointAddress, rc );
      kfree( pBuf );
      return -EIO;
   }

   /* Get the sampling frequence 
    */
   rc = usb_control_msg( 
         pDev, 
         (PIPE_CONTROL<<30) | (0<<15) | (pDev->devnum<<8) | USB_DIR_IN, 
         GET_CUR,
         USB_TYPE_CLASS | USB_RECIP_ENDPOINT | USB_DIR_IN,
         SAMPLING_FREQ_CONTROL << 8, 
         pAlts->bEndpointAddress, 
         pBuf, 
         len, 
         1000 );
   if( rc < 0 )
   {
      printk( KERN_ERR "USB Core [ept=0x%02x]: Could not get sampling frequency rc=%i\n", pAlts->bEndpointAddress, rc );
      kfree( pBuf );
      return -EIO;
   }

   rate = pBuf[0] | pBuf[1]<<8 | pBuf[2]<<16;

   /* Check the set sampling frequency is the same as that retrieved
    */
   if( rate == freqHz )
   {
      USB_CORE_LOG( "USB Core [ept=0x%02x]: Successfully set sampling frequency to %u Hz\n", pAlts->bEndpointAddress, freqHz );
   }
   else
   {
      /* Some devices may not retrieve the correct rate.  Hence, do not error out here and hope that the 
       * rate was set properly.
       */
      USB_CORE_LOG( "USB Core [ept=0x%02x]: Retrieved frequency %u Hz is not the same as set frequency %u Hz\n", pAlts->bEndpointAddress, rate, freqHz );
   }

   kfree( pBuf );

   return 0;
}

/***************************************************************************/
/**
*  Check if an interface is supported.
*
*  @return  
*     0        success
*     -ve      failure code
*/
static int isSupported( struct usb_core_intf *pIf, int freqHz, int numChans ) 
{
   int i, j;

   /* Find an alternate that can be used based on current settings.
    */
   for( i = 0; i < pIf->numAlts; i++ )
   {
      struct usb_core_alts   *pAlts    = &pIf->pAlt[i];
      struct formatTypeIDesc *pFmtDesc = pAlts->pFormatDesc;

      int frameSize1ms;

      /* Check for mono/stereo properties.
       */
      if( pFmtDesc->bNrChannels != numChans )
      {
         USB_CORE_LOG( "USB Core [Interf=%u][alt=%u][numch=%u needed=%u] Alternate not chosen due to channel descepancy.\n", 
               pIf->number, i, pFmtDesc->bNrChannels, numChans );
         continue;
      }

      /* Check the supported sampling frequencies.
       */
      if( pFmtDesc->bSamFreqType == 0 )
      {
         uint32_t lower, upper;

         /* Continuous sampling frequency.
          */
         lower = pFmtDesc->tSamFreq[2]<<16 | pFmtDesc->tSamFreq[1]<<8 | pFmtDesc->tSamFreq[0];
         upper = pFmtDesc->tSamFreq[5]<<16 | pFmtDesc->tSamFreq[4]<<8 | pFmtDesc->tSamFreq[3];

         if( freqHz < lower ||
             freqHz > upper )
         {
            USB_CORE_LOG( "USB Core [Interf=%u][alt=%u][lower=%u upper=%u needed=%u] Alternate not chosen due to sampling frequency descepancy.\n", 
                  pIf->number, i, lower, upper, freqHz );
            continue;
         }
      }
      else
      {
         /* Descrete sampling frequencies.
          */
         for( j = 0; j < pFmtDesc->bSamFreqType; j++ )
         {
            uint32_t freq = pFmtDesc->tSamFreq[3*j+2]<<16 | pFmtDesc->tSamFreq[3*j+1]<<8 | pFmtDesc->tSamFreq[3*j];

            USB_CORE_LOG( "USB Core [Interf=%u][alt=%u][supported freq=%u].\n", pIf->number, i, freq );

            if( freq == freqHz )
            {
               break;
            }
         }

         /* If a descrete sampling frequency is not found, go to the next
          * alternate setting.
          */
         if( j == pFmtDesc->bSamFreqType )
         {
            USB_CORE_LOG( "USB Core [Interf=%u][alt=%u][freq needed=%u] Alternate not chosen due to sampling frequency descepancy.\n", 
                  pIf->number, i, freqHz );
            continue;
         }
      }

      /* Check the max packet size supported makes sense based on 1 ms polling
       * interval.
       */
      frameSize1ms = numChans * (freqHz/1000) * USB_CORE_SAMP_WIDTH;

      if( pAlts->wMaxPacketSize < frameSize1ms )
      {
         USB_CORE_LOG( "USB Core [Interf=%u][alt=%u][wMaxPacketSize=%u needed=%u] Alternate not chosen due to packet size descepancy.\n", 
               pIf->number, i, pAlts->wMaxPacketSize, frameSize1ms );
         continue;
      }

      /* Found an alternate setting that works
       */
      return i;
   }

   /* Did not find an alternate that worked
    */
   return -1;
}

/***************************************************************************/
/**
*  Query whether the sampling rate and mono/stereo configuration is supported.
*
*  @return  
*     0        success
*     -ve      failure code
*/
int usbCoreIngressIsSupported( int freqHz, int numChans )
{
   if( !usbCoreIngressIsConnected() )
   {
      return 0;
   }

   return (isSupported( &gUsbCore.inIntf, freqHz, numChans ) != -1);
}

/***************************************************************************/
/**
*  Query whether the sampling rate and mono/stereo configuration is supported.
*
*  @return  
*     0        success
*     -ve      failure code
*/
int usbCoreEgressIsSupported( int freqHz, int numChans )
{
   if( !usbCoreEgressIsConnected() )
   {
      return 0;
   }

   return (isSupported( &gUsbCore.outIntf, freqHz, numChans ) != -1);
}

/***************************************************************************/
/**
*  Intialize URBs for an interface.
*
*  @return  
*     0        success
*     -ve      failure code
*/
static int initUrbs( struct usb_core_intf *pIf )
{
   struct urb           *pUrb;
   struct usb_core_alts *pAlts = &pIf->pAlt[ pIf->activeAlt ];

   int i, j;

   /* Create URBs
    */
   for( i = 0; i < USB_CORE_NUM_URBS; i++ )
   {
      pIf->pUrb[i] = pUrb = usb_alloc_urb( 1, GFP_KERNEL );
      if( !pUrb )
      {
         printk( KERN_ERR "USB Core: Failed to allocate URBs\n" );
         return -ENOMEM;
      }

      pUrb->dev                    = gUsbCore.pDev;
      pUrb->context                = pIf;
      pUrb->transfer_flags         = URB_ISO_ASAP | URB_NO_TRANSFER_DMA_MAP;
      pUrb->transfer_buffer_length = USB_CORE_ISO_FRAMES_PER_URB * pIf->frameSize1ms;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,34)
      pUrb->transfer_buffer        = usb_buffer_alloc( gUsbCore.pDev, pUrb->transfer_buffer_length, GFP_KERNEL, &pUrb->transfer_dma );
#else
      pUrb->transfer_buffer        = usb_alloc_coherent( gUsbCore.pDev, pUrb->transfer_buffer_length, GFP_KERNEL, &pUrb->transfer_dma );
#endif
      pUrb->number_of_packets      = USB_CORE_ISO_FRAMES_PER_URB;
      pUrb->interval               = 1;

      if( usb_endpoint_dir_out( &pAlts->pAlts->endpoint[0].desc ) )
      {
         pUrb->pipe     = (PIPE_ISOCHRONOUS<<30) | ((pAlts->pAlts->endpoint[0].desc.bEndpointAddress&0xf)<<15) | (gUsbCore.pDev->devnum<<8);
         pUrb->complete = usbCoreEgressHandler;
      }
      else
      {
         pUrb->pipe     = (PIPE_ISOCHRONOUS<<30) | ((pAlts->pAlts->endpoint[0].desc.bEndpointAddress&0xf)<<15) | (gUsbCore.pDev->devnum<<8) | USB_DIR_IN;
         pUrb->complete = usbCoreIngressHandler;
      }

      for( j=0; j < USB_CORE_ISO_FRAMES_PER_URB; j++ )
      {
         pUrb->iso_frame_desc[j].offset = pIf->frameSize1ms*j;
         pUrb->iso_frame_desc[j].length = pIf->frameSize1ms;
      }
   }

   return 0;
}

/***************************************************************************/
/**
*  Submit URBs
*
*  @return  
*     0        success
*     -ve      failure code
*/
static int startUrbs( struct usb_core_intf *pIf )
{
   int i, rc = 0;

   if( pIf->number != -1 )
   {
      /* Start the URBs
       */
      for( i = 0; i < USB_CORE_NUM_URBS; i++ )
      {
         rc = usb_submit_urb( pIf->pUrb[i], GFP_ATOMIC );
         if( rc )
         {
            printk( KERN_ERR "USB Core: Failed to submit URB %u\n", i );
            return rc;
         }
      }
   }

   return 0;
}

/***************************************************************************/
/**
*  Disable URBs
*
*  @return     Nothing
*/
static void disableUrbs( struct usb_core_intf *pIf )
{
   int i;

   /* Disable the URBs
    */
   if( pIf->number != -1 )
   {
      for( i = 0; i < USB_CORE_NUM_URBS; i++ )
      {
         usb_kill_urb( pIf->pUrb[i] );
      }

      /* Set interface to alternate zero.
       */
      usb_set_interface( gUsbCore.pDev, pIf->number, 0 );
   }
}

/***************************************************************************/
/**
*  Free URBs
*
*  @return     Nothing
*/
static void freeUrbs( struct usb_core_intf *pIf )
{
   int i;

   for( i = 0; i < USB_CORE_NUM_URBS; i++ )
   {
      struct urb *pUrb = pIf->pUrb[i];

      if( pUrb != NULL )
      {
         usb_kill_urb( pUrb );

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,34)
         usb_buffer_free( gUsbCore.pDev, pUrb->transfer_buffer_length, pUrb->transfer_buffer, pUrb->transfer_dma );
#else
         usb_free_coherent( gUsbCore.pDev, pUrb->transfer_buffer_length, pUrb->transfer_buffer, pUrb->transfer_dma );
#endif

         usb_free_urb( pUrb );

         pIf->pUrb[i] = NULL;
      }
   }
}

/***************************************************************************/
/**
*  Find feature units to access volume
*
*  @return  
*     0        success
*     -ve      failure code
*/
static int findFeatureUnits( int8_t *pDescs, int8_t *pEndDesc )
{
   struct usb_core_intf *pI;

   int     found=0;
   int8_t *pO;

   /* Find all the output terminals and determine the 
    * feature unit connected
    */
   pO = findDesc( pDescs, OUTPUT_TERMINAL, pEndDesc );
   while( pO != NULL )
   {
      int8_t *p;
         
      p = findUnit( pDescs, pO[7], pEndDesc );
      if( !p )
      {
         printk( KERN_ERR "USB Core: Failed to find feature unit\n" );
         return -ENODEV;
      }

      while( p != NULL )
      {
         switch( p[2] )
         {
            case FEATURE_UNIT:
            {
               found = 1;
            }
            break;
            case OUTPUT_TERMINAL:
            case INPUT_TERMINAL:
            {
               printk( KERN_ERR "USB Core: Failed to find feature unit. IT/OT found.\n" );
               return -ENODEV;
            }
            break;
            case MIXER_UNIT:
            case SELECTOR_UNIT:
            {
               /* Assume only 1 connection to the unit
                */
               if( p[4] != 1 )
               {
                  printk( KERN_ERR "USB Core: Failed to find feature unit. Mixer/Selector found with more than 1 input pin.\n" );
                  return -ENODEV;
               }
               p = findUnit( pDescs, p[5], pEndDesc );
            }
            break;
            case EXTENSION_UNIT:
            case PROCESSING_UNIT:
            {
               /* Assume only 1 connection to the unit
                */
               if( p[6] != 1 )
               {
                  printk( KERN_ERR "USB Core: Failed to find feature unit. Extension/Processing found with more than 1 input pin.\n" );
                  return -ENODEV;
               }
               p = findUnit( pDescs, p[7], pEndDesc );
            }
            break;
            default:
            {
               printk( KERN_ERR "USB Core: Failed to find feature unit. Unknown descriptor %u.\n", p[2] );
               return -ENODEV;
            }
         }
         if( found )
         {
            break;
         }
      }

      if( !found )
      {
         printk( KERN_ERR "USB Core: Failed to find a feature unit connected to OT\n" );
         return -ENODEV;
      }

      /* Feature Unit found
       */
      if( ((uint16_t)pO[5]<<8 | pO[4]) == 0x0101 )
      {
         /* The FU is connected to a USB streaming Output Terminal.  This should be a 
          * microphone stream
          */
         USB_CORE_LOG( "USB Core: Found FU for IN with id=%u", p[3] );
         pI = &gUsbCore.inIntf;
      }
      else
      {
         /* Likely a speaker
          */
         USB_CORE_LOG(  "USB Core: Found FU for OUT with id=%u", p[3] );
         pI = & gUsbCore.outIntf;
      }

      /* Save the Unit ID
       */
      pI->fu.id = p[3];

      /* Make sure the channels are cleared
       */
      memset( pI->fu.channels, 0, sizeof(pI->fu.channels) );
      
      /* Create feature unit properties.
       */
      {
         int i, j;
         int numChans = (p[0]-7)/p[5];

         if( numChans > 8 )
         {
            printk( KERN_ERR "USB Core: Cannot support so many channels in feature unit=%u\n", numChans );
            return -ENODEV;
         }

         pI->fu.numChannels = numChans;

         for( i = 0; i < numChans; i++ )
         {
            unsigned int features = p[6+(p[5]*i)];

            for( j = 1; j < USB_FEATURE_MAX; j++ )
            {
               if( features & (1<<(j-1)) )
               {
                  pI->fu.channels[j] |= (1<<i);
               }
            }
         }
      }

      /* Update local volume parameters */
      getVolume( pI );

      pO = findDesc( pO + pO[0], OUTPUT_TERMINAL, pEndDesc );
   }

   return 0;
}

/***************************************************************************/
/**
*  Find descriptor
*
*  @return  
*      Pointer to descriptor
*/
static int8_t *findDesc( int8_t *pDescs, int8_t desc, int8_t *pEndDesc )
{
   int8_t *p = pDescs;
   while( p[2] != desc )
   {
      p += p[0];
      if( p >= pEndDesc )
         return NULL;
   }

   return p;
}

/***************************************************************************/
/**
*  Find descriptor based on unit ID
*
*  @return  
*      Pointer to descriptor
*/
static int8_t *findUnit( int8_t *pDescs, int8_t unit, int8_t *pEndDesc  )
{
   int8_t *p = pDescs;
   while( p[3] != unit )
   {
      p += p[0];
      if( p >= pEndDesc )
         return NULL;
   }

   return p;
}

/***************************************************************************/
/**
*  Clean up resources
*
*  @return  
*     0        success
*     -ve      failure code
*/
static void cleanupResources( void )
{
   if( gUsbCore.pDev )
   {
      /* Free allocated buffers and free URBs
       */
      freeUrbs( &gUsbCore.inIntf );
      freeUrbs( &gUsbCore.outIntf );

      if( gUsbCore.inIntf.pAlt )
      {
         vfree( gUsbCore.inIntf.pAlt );
         gUsbCore.inIntf.pAlt = NULL;
      }
      if( gUsbCore.outIntf.pAlt )
      {
         vfree( gUsbCore.outIntf.pAlt );
         gUsbCore.outIntf.pAlt = NULL;
      }

      /* Clear globals 
       */
      gUsbCore.pDev      = NULL;
      gUsbCore.numInterf = 0;

      gUsbCore.inIntf.number      = -1;
      gUsbCore.inIntf.numAlts     = 0;
      gUsbCore.inIntf.activeAlt   = -1;
      gUsbCore.outIntf.number     = -1;
      gUsbCore.outIntf.numAlts    = 0;
      gUsbCore.outIntf.activeAlt  = -1;

      /* Clean out feature units
       */
      memset( &gUsbCore.inIntf.fu, 0, sizeof(gUsbCore.inIntf.fu) );
      memset( &gUsbCore.outIntf.fu, 0, sizeof(gUsbCore.inIntf.fu) );

      gUsbCore.disconnecting      = 0;
   }
}

/***************************************************************************/
/**
*  Create proc entries
*
*  @return     Notthing 
*/
static void createProcEntries( void )
{
   int i;

   /* create directory */
   gProcDir = proc_mkdir( "usbcore", NULL );
   if( gProcDir == NULL ) {
      printk( "KERN_ERR" "USB Core: Failed to create directory for proc entries\n" );
      return;
   }
   
   create_proc_read_entry( "dev", 0, gProcDir, usbCoreInfoReadProc, gUsbCore.pDev );
   create_proc_read_entry( "fu", 0, gProcDir, usbCoreFuReadProc, gUsbCore.pDev );

   for( i = 0; i < gUsbCore.numInterf; i++ )
   {
      char procName[20];

      sprintf( procName, "intf%u", i );

      create_proc_read_entry( procName, 0, gProcDir, usbCoreInterfReadProc, gUsbCore.pDev->actconfig->interface[i] );
   }

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
   int i;

   /* Release proc entries
    */
   remove_proc_entry( "dev", gProcDir );
   remove_proc_entry( "fu", gProcDir );

   for( i = 0; i < gUsbCore.numInterf; i++ )
   {
      char procName[20];

      sprintf( procName, "intf%u", i );

      remove_proc_entry( procName, gProcDir );
   }

   remove_proc_entry( "usbcore", NULL );

   if( gSysCtlHeader )
   {
      unregister_sysctl_table( gSysCtlHeader );
   }
}

/***************************************************************************/
/**
*  Proc read callback function for USB device Info 
*
*  @return  None
*/
static char *usbSpeed[] = { "USB_SPEED_UNKNOWN", "USB_SPEED_LOW", "USB_SPEED_FULL", "USB_SPEED_HIGH", "USB_SPEED_VARIABLE" };

static int usbCoreInfoReadProc( char *buf, char **start, off_t offset, int count, int *eof, void *data )
{
   int len = 0;
   struct usb_device      *pDev = (struct usb_device*)data;
   struct usb_host_config *pCfg = pDev->actconfig;
   struct usb_core_intf  *pIf;

   (void) start; (void) offset; (void) count;

   len += sprintf( buf+len, "USB Core Device Info\n" );

   len += sprintf( buf+len, "           Product : %s\n", pDev->product );
   len += sprintf( buf+len, "      Manufacturer : %s\n", pDev->manufacturer );
   len += sprintf( buf+len, "            Serial : %s\n", pDev->serial );
   len += sprintf( buf+len, "             Speed : %s\n", usbSpeed[pDev->speed] );
   len += sprintf( buf+len, " Number of Interfs : %u\n\n", pCfg->desc.bNumInterfaces );

   len += sprintf( buf+len, "USB_DIR_IN (Ingress)\n" );
   pIf = &gUsbCore.inIntf;
   len += sprintf( buf+len, "   Interface Number     : %i\n", pIf->number );
   len += sprintf( buf+len, "   Number of Alternates : %u\n", pIf->numAlts );
   len += sprintf( buf+len, "   Active Alternate     : %i\n", pIf->activeAlt );
   len += sprintf( buf+len, "   Number of Channels   : %u\n", pIf->numberChannels );
   len += sprintf( buf+len, "   Sample frequency     : %u\n", pIf->sampFreq );
   len += sprintf( buf+len, "   Frame Size 1ms       : %u\n\n", pIf->frameSize1ms );

   len += sprintf( buf+len, "USB_DIR_OUT (Egress)\n" );
   pIf = &gUsbCore.outIntf;
   len += sprintf( buf+len, "   Interface Number     : %i\n", pIf->number );
   len += sprintf( buf+len, "   Number of Alternates : %u\n", pIf->numAlts );
   len += sprintf( buf+len, "   Active Alternate     : %i\n", pIf->activeAlt );
   len += sprintf( buf+len, "   Number of Channels   : %u\n", pIf->numberChannels );
   len += sprintf( buf+len, "   Sample frequency     : %u\n", pIf->sampFreq );
   len += sprintf( buf+len, "   Frame Size 1ms       : %u\n", pIf->frameSize1ms );

   *eof = 1;
   return len+1;
}

/***************************************************************************/
/**
*  Proc read callback function for USB feature unit info 
*
*  @return  None
*/
static int usbCoreFuReadProc( char *buf, char **start, off_t offset, int count, int *eof, void *data )
{
   int len = 0;
   int i, j, r;
   struct usb_core_fu *pF;

   (void) start; (void) offset; (void) count;

   len += sprintf( buf+len, "USB Core Feature Unit Info\n" );

   for( r = 0; r < 2; r++ )
   {
      if( r == 0 )
      {
         len += sprintf( buf+len, "\nIngress Features\n" );
         pF = &gUsbCore.inIntf.fu;
      }
      else
      {
         len += sprintf( buf+len, "\nEgress Features\n" );
         pF = &gUsbCore.outIntf.fu;
      }

      for( i = 1; i < USB_FEATURE_MAX; i++ )
      {
         int exists = 0;
         int cur, min, max, res;

         for( j = 0; j < pF->numChannels; j++ )
         {
            if( pF->channels[i] & (1<<j) )
            {
               if( !exists )
               {
                  len += sprintf( buf+len, "   %s:\n", fuStrings[i] );
                  exists = 1;
               }
               switch( i )
               {
                  case USB_FEATURE_MUTE:
                  case USB_FEATURE_AGC:
                  case USB_FEATURE_BASSBOOST:
                  case USB_FEATURE_LOUDNESS:
                  {
                     cur = getFeature( i, GET_CUR, pF->id, j );
                     len += sprintf( buf+len, "      ch=%u: cur=%i\n", j, cur );
                  }
                  break;
                  case USB_FEATURE_VOLUME:
                  {
                     cur = getFeature( i, GET_CUR, pF->id, j );
                     min = getFeature( i, GET_MIN, pF->id, j );
                     max = getFeature( i, GET_MAX, pF->id, j );
                     res = getFeature( i, GET_RES, pF->id, j );
                     len += sprintf( buf+len, "      ch=%u: cur=%i(%i db) min=%i(%i db) max=%i(%i db) res=%i(%i db)\n",
                           j, cur, USB_CORE_DB_VALUE( cur ), min, USB_CORE_DB_VALUE( min ), 
                           max, USB_CORE_DB_VALUE( max ), res, USB_CORE_DB_VALUE( res ));
                  }
                  break;
                  default:
                  {
                     cur = getFeature( i, GET_CUR, pF->id, j );
                     min = getFeature( i, GET_MIN, pF->id, j );
                     max = getFeature( i, GET_MAX, pF->id, j );
                     res = getFeature( i, GET_RES, pF->id, j );
                     len += sprintf( buf+len, "      ch=%u: cur=%i min=%i max=%i res=%i\n", j, cur, min, max, res );
                  }
                  break;
               }
            }
         }
      }
   }

   len += sprintf( buf+len, "\nCached Volume Info\n" );
   len += sprintf( buf+len, "   Ingress: cur=%i min=%i max=%i res=%i\n",
         gUsbCore.inIntf.fu.volInfo.cur, gUsbCore.inIntf.fu.volInfo.min,
         gUsbCore.inIntf.fu.volInfo.max, gUsbCore.inIntf.fu.volInfo.res );
   len += sprintf( buf+len, "   Egress:  cur=%i min=%i max=%i res=%i\n",
         gUsbCore.outIntf.fu.volInfo.cur, gUsbCore.outIntf.fu.volInfo.min,
         gUsbCore.outIntf.fu.volInfo.max, gUsbCore.outIntf.fu.volInfo.res );

   *eof = 1;
   return len+1;
}

/***************************************************************************/
/**
*  Proc read callback function for interface data
*
*  @return  None
*/
static char *csAcIntDesc[] = { "UNKNOWN", "HEADER", "INPUT_TERMINAL", "OUTPUT_TERMINAL", "MIXER_UNIT", "SELECTOR_UNIT", "FEATURE_UNIT", "PROCESSING_UNIT", "EXTENSION_UNIT" };
static char *csAsIntDesc[] = { "UNKNOWN", "AS_GENERAL", "FORMAT_TYPE", "FORMAT_SPECIFIC" };
static char *intClass[]    = { "UNKNOWN", "USB_CLASS_AUDIO" };
static char *intSubClass[] = { "UNKNOWN", "USB_SUBCLASS_AUDIO_CONTROL", "USB_SUBCLASS_AUDIO_STREAMING", "USB_SUBCLASS_MIDI_STREAMING" };
static char *csAsEptDesc[] = { "UNKNOWN", "EP_GENERAL" };

static int usbCoreInterfReadProc( char *buf, char **start, off_t offset, int count, int *eof, void *data )
{
   int j, s, r, active;
   int len = 0;

   struct usb_interface      *pIntf = (struct usb_interface*)data;
   struct usb_host_interface *pAlts = pIntf->altsetting;

   (void) start; (void) offset; (void) count;

   len += sprintf( buf+len, "\nUSB Audio Device Info for Interface %u\n", pIntf->cur_altsetting->desc.bInterfaceNumber );

   active = ((int)pIntf->cur_altsetting - (int)pIntf->altsetting) / sizeof( *pIntf->altsetting );

   for( j = 0; j < pIntf->num_altsetting; j++ )
   {
      len += sprintf( buf+len, "\nAlt(%u)", j );
      if( j == active )
      {
         len += sprintf( buf+len, " (active)" );
      }

      len += sprintf( buf+len, ": bNumEndpoints=%u", pAlts->desc.bNumEndpoints );

      if( pAlts->desc.bInterfaceClass < sizeof( intClass ) )
      {
         len += sprintf( buf+len, " bInterfaceClass=%s", intClass[pAlts->desc.bInterfaceClass] );
      }
      else
      {
         len += sprintf( buf+len, " bInterfaceClass=UNKNOWN" );
         break;
      }

      if( pAlts->desc.bInterfaceSubClass < sizeof( intSubClass ) )
      {
         len += sprintf( buf+len, " bInterfaceSubClass=%s", intSubClass[pAlts->desc.bInterfaceSubClass] );
      }
      else
      {
         len += sprintf( buf+len, " bInterfaceSubClass=UNKNOWN" );
         break;
      }

      len += sprintf( buf+len, "\n" );

      /* Parse extra descriptors based on subclass 
       */
      switch( pAlts->desc.bInterfaceSubClass )
      {
         case USB_SUBCLASS_AUDIO_CONTROL:
         {
            uint8_t *p = (uint8_t*)pAlts->extra;

            while( p < (uint8_t*)pAlts->extra + pAlts->extralen )
            {
               len += sprintf( buf+len, "   %s:", csAcIntDesc[ p[2] ] );

               switch( p[2] )
               {
                  case HEADER:
                  {
                     len += sprintf( buf+len, " bInCollection=%u", p[7] );
                     len += sprintf( buf+len, " baInterfaceNr" );
                     for( r = 0; r < p[7]; r++ )
                     {
                        len += sprintf( buf+len, "(%u)=%u ", r, p[8+r] );
                     }
                  }
                  break;
                  case INPUT_TERMINAL:
                  {
                     len += sprintf( buf+len, " bTerminalID=%u", p[3] );
                     len += sprintf( buf+len, " bTerminalType=0x%04x", (uint16_t)p[5]<<8 | p[4] );
                     len += sprintf( buf+len, " bAssocTerminal=%u", p[6] );
                     len += sprintf( buf+len, " bNrChannels=%u", p[7] );
                     len += sprintf( buf+len, " wChannelConfig=0x%04x", (uint16_t)p[9]<<8 | p[8] );
                  }
                  break;
                  case OUTPUT_TERMINAL:
                  {
                     len += sprintf( buf+len, " bTerminalID=%u", p[3] );
                     len += sprintf( buf+len, " bTerminalType=0x%04x", (uint16_t)p[5]<<8 | p[4] );
                     len += sprintf( buf+len, " bAssocTerminal=%u", p[6] );
                     len += sprintf( buf+len, " bSourceID=%u", p[7] );
                  }
                  break;
                  case FEATURE_UNIT:
                  {
                     len += sprintf( buf+len, " bUnitID=%u", p[3] );
                     len += sprintf( buf+len, " bSourceID=%u", p[4] );
                     len += sprintf( buf+len, " bControlSize=%u", p[5] );
                     len += sprintf( buf+len, " bmaControls" );
                     for( s = 0; s < p[0]-7; s += p[5] )
                     {
                        len += sprintf( buf+len, "(%u)=0x", s/p[5] );
                        for( r = p[5]; r > 0; r-- )
                        {
                           len += sprintf( buf+len, "%02x", p[s+6+r-1] );
                        }
                        len += sprintf( buf+len, " " );
                     }
                  }
                  break;
                  case SELECTOR_UNIT:
                  {
                     len += sprintf( buf+len, " bUnitID=%u", p[3] );
                     len += sprintf( buf+len, " bNrInPins=%u", p[4] );
                     len += sprintf( buf+len, " baSourceID" );
                     for( s = 0; s < p[4]; s++ )
                     {
                        len += sprintf( buf+len, "(%u)=%u ", s, p[5+s] );
                     }
                  }
                  break;
                  case MIXER_UNIT:
                  {
                     len += sprintf( buf+len, " bUnitID=%u", p[3] );
                     len += sprintf( buf+len, " bNrInPins=%u", p[4] );
                     len += sprintf( buf+len, " baSourceID" );
                     for( s = 0; s < p[4]; s++ )
                     {
                        len += sprintf( buf+len, "(%u)=%u ", s, p[5+s] );
                     }
                     len += sprintf( buf+len, " bNrChannels=%u", p[5+p[4]] );
                     len += sprintf( buf+len, " wChannelConfig=0x%x", (uint16_t)p[7+p[4]]<<8 | p[6+p[4]] );
                     /*len += sprintf( buf+len, " iChannelNames=%u", p[8+p[4]] ); */
                     len += sprintf( buf+len, " bmControls=0x" );
                     for( s = p[0]-2; s >= p[4]+9; s-- )
                     {
                        len += sprintf( buf+len, "%02x", p[s] );
                     }
                  }
                  break;
                  default:
                  {
                     len += sprintf( buf+len, " Unsupported" );
                  }
               }

               p += p[0];
               len += sprintf( buf+len, "\n" );
            }
         }
         break;

         case USB_SUBCLASS_AUDIO_STREAMING:
         {
            uint8_t *p = (uint8_t*)pAlts->extra;

            while( p < (uint8_t*)pAlts->extra + pAlts->extralen )
            {
               len += sprintf( buf+len, "   %s:", csAsIntDesc[ p[2] ] );
               switch( p[2] )
               {
                  case AS_GENERAL:
                  {
                     len += sprintf( buf+len, " bTerminalLink=%u", p[3] );
                     len += sprintf( buf+len, " bDelay=%u", p[4] );
                     len += sprintf( buf+len, " wFormatTag=0x%04x", (uint16_t)p[6]<<8 | p[5] );
                  }
                  break;
                  case FORMAT_TYPE:
                  {
                     len += sprintf( buf+len, " bFormatType=%u", p[3] );

                     /* Only look at FORMAT_TYPE_I 
                      */
                     if( p[3] == 1 )
                     {
                        len += sprintf( buf+len, " bNrChannels=%u", p[4] );
                        len += sprintf( buf+len, " bSubFrameSize=%u", p[5] );
                        len += sprintf( buf+len, " bBitResolution=%u\n", p[6] );
                        len += sprintf( buf+len, "                bSamFreqType=%u", p[7] );
                        if( p[7] == 0 )
                        {
                           len += sprintf( buf+len, " tLowerSamFreq=%u", p[10]<<16 | p[9]<<8 | p[8] );
                           len += sprintf( buf+len, " tUpperSamFreq=%u", p[13]<<16 | p[12]<<8 | p[11] );
                        }
                        else
                        {
                           len += sprintf( buf+len, " tSamFreq=" );
                           for( r = 0; r < p[7] ; r++ )
                           {
                              len += sprintf( buf+len, "%u ", p[3*r+2+8]<<16 | p[3*r+1+8]<<8 | p[3*r+8] );
                           }
                        }
                     }
                  }
                  break;
                  default:
                  {
                     len += sprintf( buf+len, " Unsupported AS descriptor" );
                  }
               }

               p += p[0];
               len += sprintf( buf+len, "\n" );
            }
         }
         break;

         default:
         {
            len += sprintf( buf+len, "   Unsupported Sub-class=%u\n", pAlts->desc.bInterfaceSubClass );
         }
      }

      if( pAlts->desc.bNumEndpoints )
      {
         len += sprintf( buf+len, "   Ept:" );
         {
            struct usb_host_endpoint *pEpt = pAlts->endpoint;
            uint8_t *p = (uint8_t*)pEpt->extra;

            len += sprintf( buf+len, " bEndpointAddress=0x%02x", pEpt->desc.bEndpointAddress );
            len += sprintf( buf+len, " bmAttributes=0x%02x", pEpt->desc.bmAttributes );
            len += sprintf( buf+len, " wMaxPacketSize=%u", pEpt->desc.wMaxPacketSize );
            len += sprintf( buf+len, " bInterval=%u\n", pEpt->desc.bInterval );

            while( p < (uint8_t*)pEpt->extra + pEpt->extralen )
            {
               len += sprintf( buf+len, "      %s:", csAsEptDesc[ p[2] ] );

               switch( p[2] )
               {
                  case EP_GENERAL:
                  {
                     len += sprintf( buf+len, " bmAttributes=0x%02x", p[3] );
                     len += sprintf( buf+len, " bLockDelayUnits=%u", p[4] );
                     len += sprintf( buf+len, " bLockDelay=%u", p[5] );
                  }
                  break;
                  default:
                  {
                     len += sprintf( buf+len, " Unsupported AS Endpoint Descriptor" );
                  }
               }

               p += p[0];
               len += sprintf( buf+len, "\n" );
            }
         }
      }

      pAlts++;
   }

   *eof = 1;
   return len+1;
}

/***************************************************************************/
/**
*  Sysctl callback to handle sine generation test
*
*  @return  0 success, otherwise failure
*/
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,31)
static int proc_doModify( ctl_table *table, int write,
      void __user *buffer, size_t *lenp, loff_t *ppos )
#else
static int proc_doModify( ctl_table *table, int write, struct file *filp,
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
      struct usb_core_intf *pI = table->extra1;
      USB_CORE_INFO_CB cb = *(USB_CORE_INFO_CB*)table->extra2;

      initInterface( pI, pI->sampFreq, pI->numberChannels );

      if( cb )
      {
         cb( pI->sampFreq, pI->numberChannels );
      }

      if( atomic_read( &gUsbCore.enable ) )
      {
         rc = startUrbs( pI );
         if( rc )
         {
            printk( KERN_ERR "USB Core: Failed to start URBs\n" );
            return rc;
         }
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
static int __init halaudio_usbcore_init( void )
{
   int rc;

   printk( KERN_INFO "HAL Audio USB Core Driver: 1.0. Built %s %s\n", __DATE__, __TIME__ );

   /* Initialize globals
    */
   memset( &gUsbCore, 0, sizeof( gUsbCore ) );

   gUsbCore.inIntf.number     = -1;
   gUsbCore.inIntf.activeAlt  = -1;
   gUsbCore.outIntf.number    = -1;
   gUsbCore.outIntf.activeAlt = -1;

   /* Initialize defaults
    */
   gUsbCore.inIntf.numberChannels = USB_CORE_DEFAULT_CHANNELS;
   gUsbCore.inIntf.sampFreq       = USB_CORE_DEFAULT_FREQ;
   gUsbCore.inIntf.frameSize1ms   = USB_CORE_DEFAULT_FRAME_SIZE_1MS;

   gUsbCore.outIntf.numberChannels = USB_CORE_DEFAULT_CHANNELS;
   gUsbCore.outIntf.sampFreq       = USB_CORE_DEFAULT_FREQ;
   gUsbCore.outIntf.frameSize1ms   = USB_CORE_DEFAULT_FRAME_SIZE_1MS;

   /* Initalize mutex
    */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,37)
   init_MUTEX( &gUsbCore.mutex ); /* unlocked */
#else
   sema_init( &gUsbCore.mutex , 1 );
#endif
   atomic_set( &gUsbCore.enable, 0 );
   atomic_set( &gUsbCore.prepared, 0 );

   /* Register this USB device driver to USB core.
    */
   rc = usb_register( &usbCoreDriver );
   if( rc )
   {
      printk( KERN_ERR "USB Core: Failed to register USB device driver rc=%i\n", rc );
      return rc;
   }

   gUsbCore.initialized = 1;

   return 0;
}

/***************************************************************************/
/**
*  Driver destructor routine. Frees all resources
*/
static void __exit halaudio_usbcore_exit( void )
{
   /* De-register this USB device driver from USB core.
    */
   usb_deregister( &usbCoreDriver );
}

module_init( halaudio_usbcore_init );
module_exit( halaudio_usbcore_exit );

EXPORT_SYMBOL( usbCorePrepare );
EXPORT_SYMBOL( usbCoreEnable );
EXPORT_SYMBOL( usbCoreDisable );
EXPORT_SYMBOL( usbCoreSetIngressFreq );
EXPORT_SYMBOL( usbCoreGetIngressFreq );
EXPORT_SYMBOL( usbCoreSetEgressFreq );
EXPORT_SYMBOL( usbCoreGetEgressFreq );
EXPORT_SYMBOL( usbCoreGetIngressChannels );
EXPORT_SYMBOL( usbCoreGetEgressChannels );
EXPORT_SYMBOL( usbCoreGetIngressFrameSize1ms );
EXPORT_SYMBOL( usbCoreGetEgressFrameSize1ms );
EXPORT_SYMBOL( usbCoreIngressIsConnected );
EXPORT_SYMBOL( usbCoreEgressIsConnected );
EXPORT_SYMBOL( usbCoreRegisterCb );
EXPORT_SYMBOL( usbCoreDeregisterCb );
EXPORT_SYMBOL( usbCoreSetIngressVolume );
EXPORT_SYMBOL( usbCoreGetIngressVolume );
EXPORT_SYMBOL( usbCoreSetEgressVolume );
EXPORT_SYMBOL( usbCoreGetEgressVolume );
EXPORT_SYMBOL( usbCoreIngressIsSupported );
EXPORT_SYMBOL( usbCoreEgressIsSupported );

MODULE_AUTHOR( "Broadcom" );
MODULE_DESCRIPTION( "HAL Audio USB Core Low Level Driver" );
MODULE_VERSION( "1.0" );
MODULE_LICENSE( "GPL" );
