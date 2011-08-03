/*****************************************************************************
* Copyright 2003 - 2009 Broadcom Corporation.  All rights reserved.
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
*  @file    halaudio_drv.c
*
*  @brief   HAL Audio driver
*
****************************************************************************/

/* ---- Include Files ---------------------------------------------------- */
#include <linux/types.h>               /* For stdint types: uint8_t, etc. */
#include <linux/module.h>              /* For EXPORT_SYMBOL and module defns */
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/platform_device.h>     /* For Linux platform bus */
#include <linux/proc_fs.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/list.h>                /* For linked lists */
#include <linux/sched.h>               /* For schedule_timeout */
#include <linux/semaphore.h>           /* For semaphores */
#include <linux/string.h>
#include <linux/sysctl.h>              /* Sysctl */
#include <linux/vmalloc.h>             /* For memory alloc */

#include <linux/broadcom/halaudio.h>     /* HALAUDIO API */
#include <linux/broadcom/halaudio_cfg.h> /* HALAUDIO board configurations */
#include <linux/broadcom/halaudio_ioctl.h>
#ifdef CONFIG_BCM_BSC
#include <linux/broadcom/bsc.h>        /* Board Specific Configurations */
#endif

#include <asm/atomic.h>                /* Atomic operators */
#include <asm/io.h>
#include <asm/uaccess.h>               /* User access routines */

#include <linux/version.h>

/* ---- Private Constants and Types -------------------------------------- */

/* Arbitrary maximum number of interfaces and codecs to protect
 * the procfs callback when scanning for existing interfaces
 * and codec channes
 */
#define HALAUDIO_MAX_INTERFACES     100
#define HALAUDIO_MAX_CODECS         100

#define HALAUDIO_NAME               "halaudio"

/* Procfs file name */
#define HALAUDIO_PROC_NAME          HALAUDIO_NAME

/**
*  Union of all the different IOCTL parameter structures to determine
*  max stack variable size
*/
union halaudio_ioctl_params
{
   struct halaudio_ioctl_getgaininfo         getgain;
   struct halaudio_ioctl_setgain             setgain;
   struct halaudio_ioctl_altergain           altergain;
   struct halaudio_ioctl_rw_parms            rw_parms;
   struct halaudio_ioctl_setfreq             setfreq;
   struct halaudio_ioctl_getfreq             getfreq;
   struct halaudio_ioctl_setequparms         setequ;
   struct halaudio_ioctl_getequparms         getequ;
   struct halaudio_ioctl_getinterfaceinfo    ifinfo;
   struct halaudio_ioctl_getcodecinfo        codecinfo;
   struct halaudio_ioctl_enableinterface     enableif;
   struct halaudio_ioctl_query_interface_id  interfid;
   struct halaudio_ioctl_query_codec_id      codecid;
};

/**
* HAL Audio R/W config
*/
typedef struct halaudio_rw_cfg
{
   HALAUDIO_CODEC       cid;              /* Codec id */
   HALAUDIO_FMT         format;           /* Data format */
}
HALAUDIO_RW_CFG;

/**
*  HAL Audio client information
*/
struct halaudio_client
{
   struct list_head     lnode;            /* List node */
   HALAUDIO_HDL         handle;           /* Client handle */
   HALAUDIO_RW_CFG      read;             /* Read config */
   HALAUDIO_RW_CFG      write;            /* Write config */
   int                  powerlevel;       /* Cached power level request */
   int                  superuser;        /* Cached superuser request */
};

/**
*  Global info structure
*/
struct halaudio_info
{
   struct list_head     client_list;      /* Client list */
   struct semaphore     client_lock;      /* Protect client list */
   int                  num_clients;      /* Number of clients */
};

/**
*  Verbose prints are enabled via /proc/sys/halaudio/dbgprint.
*
*  Usage:
*  Set to 1 to get basic informational traces. Set to 2 to get basic plus
*  verbose prints.
*/
#define HALAUDIO_TRACE( fmt, args... ) do { if ( gDebugPrintLevel ) printk( KERN_INFO "%s: " fmt, __FUNCTION__ , ## args ); } while (0)
#define HALAUDIO_DEBUG( fmt, args... ) do { if ( gDebugPrintLevel >= 2 ) printk( KERN_INFO "%s: " fmt, __FUNCTION__ , ## args ); } while (0)

/* ---- Private Function Prototypes -------------------------------------- */
#ifdef CONFIG_PM
static int     pmSuspend( struct platform_device *pdev, pm_message_t state );
static int     pmResume( struct platform_device *pdev );
#endif

static void    halaudio_debug_init( void );
static void    halaudio_debug_exit( void );
static int     halaudio_read_proc( char *buf, char **start, off_t offset, int count, int *eof, void *data );

static int     halaudio_open( struct inode *inode, struct file *file );
static int     halaudio_release( struct inode *inode, struct file *file );
static ssize_t halaudio_read( struct file *file, char __user *buffer, size_t count, loff_t *ppos );
static ssize_t halaudio_write( struct file *file, const char __user *buffer, size_t count, loff_t *ppos );
static long     halaudio_ioctl(struct file *file, unsigned int cmd, unsigned long arg );

static int     halaudio_probe( struct platform_device *pdev );
static int     halaudio_remove( struct platform_device *pdev );

/* ---- Private Variables ------------------------------------------------- */
static int gDriverMajor;
#ifdef CONFIG_SYSFS
static struct class * halaudio_class;
static struct device * halaudio_dev;
#endif

/* Debug print flag to gate verbose prints.  */
static int gDebugPrintLevel;

/* Debug print flag to gate verbose prints.  */
static int gDefaultWriteCodec;

/* The amount of delay in msec before going to deep sleep. Default 
 * is 800 msec to allow audio buffers to clear.
 */
int gHalAudioSleepDelayMsec = 800;
EXPORT_SYMBOL( gHalAudioSleepDelayMsec );

/* File Operations (these are the device driver entry points) */
static struct file_operations gfops =
{
   .owner      = THIS_MODULE,
   .open       = halaudio_open,
   .release    = halaudio_release,
   .read       = halaudio_read,
   .write      = halaudio_write,
   .unlocked_ioctl = halaudio_ioctl,
};

/*  SysCtl data structures */
static struct ctl_table gSysCtlChild[] =
{
   {
      .procname      = "dbgprint",
      .data          = &gDebugPrintLevel,
      .maxlen        = sizeof( int ),
      .mode          = 0644,
      .proc_handler  = &proc_dointvec,
   },
   {
      .procname      = "sleepdelay_msec",
      .data          = &gHalAudioSleepDelayMsec,
      .maxlen        = sizeof( int ),
      .mode          = 0644,
      .proc_handler  = &proc_dointvec,
   },
   {
      .procname      = "default_write_codec",
      .data          = &gDefaultWriteCodec,
      .maxlen        = sizeof( int ),
      .mode          = 0644,
      .proc_handler  = &proc_dointvec,
   },
   {}
};
static struct ctl_table gSysCtl[] =
{
   {
      .procname      = "halaudio",
      .mode          = 0555,
      .child         = gSysCtlChild
   },
   {}
};
static struct ctl_table_header  *gSysCtlHeader;

/* Installed proprietary API methods */
static HALAUDIO_API_FUNCS        gApi;

/* power management callbacks */
static HALAUDIO_PM_OPS           gPMOps;

/* Global information structure */
static struct halaudio_info      gInfo;

/* Global containing platform specific extensions operations. */
static HALAUDIO_EXTENSIONS_OPS gExtensionsOps;

/* G.711 a-law and u-law expansion lookup tables. */
static int16_t alawExpandTable[] =
{
   0xea80, 0xeb80, 0xe880, 0xe980, 0xee80, 0xef80, 0xec80, 0xed80,
   0xe280, 0xe380, 0xe080, 0xe180, 0xe680, 0xe780, 0xe480, 0xe580,
   0xf540, 0xf5c0, 0xf440, 0xf4c0, 0xf740, 0xf7c0, 0xf640, 0xf6c0,
   0xf140, 0xf1c0, 0xf040, 0xf0c0, 0xf340, 0xf3c0, 0xf240, 0xf2c0,
   0xaa00, 0xae00, 0xa200, 0xa600, 0xba00, 0xbe00, 0xb200, 0xb600,
   0x8a00, 0x8e00, 0x8200, 0x8600, 0x9a00, 0x9e00, 0x9200, 0x9600,
   0xd500, 0xd700, 0xd100, 0xd300, 0xdd00, 0xdf00, 0xd900, 0xdb00,
   0xc500, 0xc700, 0xc100, 0xc300, 0xcd00, 0xcf00, 0xc900, 0xcb00,
   0xfea8, 0xfeb8, 0xfe88, 0xfe98, 0xfee8, 0xfef8, 0xfec8, 0xfed8,
   0xfe28, 0xfe38, 0xfe08, 0xfe18, 0xfe68, 0xfe78, 0xfe48, 0xfe58,
   0xffa8, 0xffb8, 0xff88, 0xff98, 0xffe8, 0xfff8, 0xffc8, 0xffd8,
   0xff28, 0xff38, 0xff08, 0xff18, 0xff68, 0xff78, 0xff48, 0xff58,
   0xfaa0, 0xfae0, 0xfa20, 0xfa60, 0xfba0, 0xfbe0, 0xfb20, 0xfb60,
   0xf8a0, 0xf8e0, 0xf820, 0xf860, 0xf9a0, 0xf9e0, 0xf920, 0xf960,
   0xfd50, 0xfd70, 0xfd10, 0xfd30, 0xfdd0, 0xfdf0, 0xfd90, 0xfdb0,
   0xfc50, 0xfc70, 0xfc10, 0xfc30, 0xfcd0, 0xfcf0, 0xfc90, 0xfcb0,
   0x1580, 0x1480, 0x1780, 0x1680, 0x1180, 0x1080, 0x1380, 0x1280,
   0x1d80, 0x1c80, 0x1f80, 0x1e80, 0x1980, 0x1880, 0x1b80, 0x1a80,
   0x0ac0, 0x0a40, 0x0bc0, 0x0b40, 0x08c0, 0x0840, 0x09c0, 0x0940,
   0x0ec0, 0x0e40, 0x0fc0, 0x0f40, 0x0cc0, 0x0c40, 0x0dc0, 0x0d40,
   0x5600, 0x5200, 0x5e00, 0x5a00, 0x4600, 0x4200, 0x4e00, 0x4a00,
   0x7600, 0x7200, 0x7e00, 0x7a00, 0x6600, 0x6200, 0x6e00, 0x6a00,
   0x2b00, 0x2900, 0x2f00, 0x2d00, 0x2300, 0x2100, 0x2700, 0x2500,
   0x3b00, 0x3900, 0x3f00, 0x3d00, 0x3300, 0x3100, 0x3700, 0x3500,
   0x0158, 0x0148, 0x0178, 0x0168, 0x0118, 0x0108, 0x0138, 0x0128,
   0x01d8, 0x01c8, 0x01f8, 0x01e8, 0x0198, 0x0188, 0x01b8, 0x01a8,
   0x0058, 0x0048, 0x0078, 0x0068, 0x0018, 0x0008, 0x0038, 0x0028,
   0x00d8, 0x00c8, 0x00f8, 0x00e8, 0x0098, 0x0088, 0x00b8, 0x00a8,
   0x0560, 0x0520, 0x05e0, 0x05a0, 0x0460, 0x0420, 0x04e0, 0x04a0,
   0x0760, 0x0720, 0x07e0, 0x07a0, 0x0660, 0x0620, 0x06e0, 0x06a0,
   0x02b0, 0x0290, 0x02f0, 0x02d0, 0x0230, 0x0210, 0x0270, 0x0250,
   0x03b0, 0x0390, 0x03f0, 0x03d0, 0x0330, 0x0310, 0x0370, 0x0350,
};

static int16_t ulawExpandTable[] =
{
   0x8284, 0x8684, 0x8a84, 0x8e84, 0x9284, 0x9684, 0x9a84, 0x9e84,
   0xa284, 0xa684, 0xaa84, 0xae84, 0xb284, 0xb684, 0xba84, 0xbe84,
   0xc184, 0xc384, 0xc584, 0xc784, 0xc984, 0xcb84, 0xcd84, 0xcf84,
   0xd184, 0xd384, 0xd584, 0xd784, 0xd984, 0xdb84, 0xdd84, 0xdf84,
   0xe104, 0xe204, 0xe304, 0xe404, 0xe504, 0xe604, 0xe704, 0xe804,
   0xe904, 0xea04, 0xeb04, 0xec04, 0xed04, 0xee04, 0xef04, 0xf004,
   0xf0c4, 0xf144, 0xf1c4, 0xf244, 0xf2c4, 0xf344, 0xf3c4, 0xf444,
   0xf4c4, 0xf544, 0xf5c4, 0xf644, 0xf6c4, 0xf744, 0xf7c4, 0xf844,
   0xf8a4, 0xf8e4, 0xf924, 0xf964, 0xf9a4, 0xf9e4, 0xfa24, 0xfa64,
   0xfaa4, 0xfae4, 0xfb24, 0xfb64, 0xfba4, 0xfbe4, 0xfc24, 0xfc64,
   0xfc94, 0xfcb4, 0xfcd4, 0xfcf4, 0xfd14, 0xfd34, 0xfd54, 0xfd74,
   0xfd94, 0xfdb4, 0xfdd4, 0xfdf4, 0xfe14, 0xfe34, 0xfe54, 0xfe74,
   0xfe8c, 0xfe9c, 0xfeac, 0xfebc, 0xfecc, 0xfedc, 0xfeec, 0xfefc,
   0xff0c, 0xff1c, 0xff2c, 0xff3c, 0xff4c, 0xff5c, 0xff6c, 0xff7c,
   0xff88, 0xff90, 0xff98, 0xffa0, 0xffa8, 0xffb0, 0xffb8, 0xffc0,
   0xffc8, 0xffd0, 0xffd8, 0xffe0, 0xffe8, 0xfff0, 0xfff8, 0x0000,
   0x7d7c, 0x797c, 0x757c, 0x717c, 0x6d7c, 0x697c, 0x657c, 0x617c,
   0x5d7c, 0x597c, 0x557c, 0x517c, 0x4d7c, 0x497c, 0x457c, 0x417c,
   0x3e7c, 0x3c7c, 0x3a7c, 0x387c, 0x367c, 0x347c, 0x327c, 0x307c,
   0x2e7c, 0x2c7c, 0x2a7c, 0x287c, 0x267c, 0x247c, 0x227c, 0x207c,
   0x1efc, 0x1dfc, 0x1cfc, 0x1bfc, 0x1afc, 0x19fc, 0x18fc, 0x17fc,
   0x16fc, 0x15fc, 0x14fc, 0x13fc, 0x12fc, 0x11fc, 0x10fc, 0x0ffc,
   0x0f3c, 0x0ebc, 0x0e3c, 0x0dbc, 0x0d3c, 0x0cbc, 0x0c3c, 0x0bbc,
   0x0b3c, 0x0abc, 0x0a3c, 0x09bc, 0x093c, 0x08bc, 0x083c, 0x07bc,
   0x075c, 0x071c, 0x06dc, 0x069c, 0x065c, 0x061c, 0x05dc, 0x059c,
   0x055c, 0x051c, 0x04dc, 0x049c, 0x045c, 0x041c, 0x03dc, 0x039c,
   0x036c, 0x034c, 0x032c, 0x030c, 0x02ec, 0x02cc, 0x02ac, 0x028c,
   0x026c, 0x024c, 0x022c, 0x020c, 0x01ec, 0x01cc, 0x01ac, 0x018c,
   0x0174, 0x0164, 0x0154, 0x0144, 0x0134, 0x0124, 0x0114, 0x0104,
   0x00f4, 0x00e4, 0x00d4, 0x00c4, 0x00b4, 0x00a4, 0x0094, 0x0084,
   0x0078, 0x0070, 0x0068, 0x0060, 0x0058, 0x0050, 0x0048, 0x0040,
   0x0038, 0x0030, 0x0028, 0x0020, 0x0018, 0x0010, 0x0008, 0x0000,
};

/* ---- Public Variables ------------------------------------------------- */

/* ---- Functions -------------------------------------------------------- */

/***************************************************************************/
/**
*  Map client handle to client state memory
*
*  @return  Pointer to client state memory
*/
static inline struct halaudio_client *halaudio_get_clientp(
   HALAUDIO_HDL client_hdl          /**< (i) client handle */
)
{
   struct halaudio_client *clientp, *foundp;

   foundp = NULL;

   down( &gInfo.client_lock );
   list_for_each_entry( clientp, &gInfo.client_list, lnode )
   {
      if ( clientp->handle == client_hdl )
      {
         foundp = clientp;
         break;
      }
   }
   up( &gInfo.client_lock );

   return foundp;
}

#ifdef CONFIG_PM
/***************************************************************************/
/**
*  Power Management Suspend.  Call the suspend op in the HAL audio
*  API for power management.
*
*  @return
*     0     - Success
*     -ve   - error code
*/
static int pmSuspend(
   struct platform_device *pdev,    /**< (in) pointer to platform device */
   pm_message_t            state    /**< (in) power management message */
)
{
   if ( gPMOps.suspend )
   {
      return gPMOps.suspend();
   }
   return 0;
}

/***************************************************************************/
/**
*  Power Management Resume.  Call the resume op  in the HAL audio
*  API for power management.
*
*  @return
*     0     - Success
*     -ve   - error code
*/
static int pmResume(
   struct platform_device *pdev     /**< (in) pointer to platform device */
)
{
   if ( gPMOps.resume )
   {
      return gPMOps.resume();
   }
   return 0;
}
#endif

/***************************************************************************/
/**
*  Allocate client handle to make use of HAL Audio
*
*  @return
*     valid pointer - success
*     NULL          - error, failed to open driver handle
*/
HALAUDIO_HDL halAudioAllocateClient( void )
{
   if ( gApi.allocateClient )
   {
      HALAUDIO_HDL            client_hdl;
      struct halaudio_client *clientp;

      clientp = kmalloc( sizeof(*clientp), GFP_KERNEL );
      if ( clientp == NULL )
      {
         return NULL;
      }
      memset( clientp, 0, sizeof(*clientp) );

      client_hdl = gApi.allocateClient();
      if ( client_hdl == NULL )
      {
         kfree( clientp );
         return NULL;
      }

      clientp->handle = client_hdl;
      
      /* Assign a configurable default write codec for this client.
      */
      clientp->write.cid = gDefaultWriteCodec;

      down( &gInfo.client_lock );
      list_add_tail( &clientp->lnode, &gInfo.client_list );
      gInfo.num_clients++;
      up( &gInfo.client_lock );

      HALAUDIO_TRACE( "client=0x%lx\n", (unsigned long)client_hdl );

      return client_hdl;
   }
   return NULL;
}
EXPORT_SYMBOL( halAudioAllocateClient );

/***************************************************************************/
/**
*  Free client handle
*
*  @return
*     0        Success
*     -ve      Failure code
*/
int halAudioFreeClient(
   HALAUDIO_HDL client_hdl          /**< (i) Client handle */
)
{
   if ( gApi.freeClient )
   {
      struct halaudio_client *clientp;

      clientp = halaudio_get_clientp( client_hdl );
      if ( clientp == NULL )
      {
         return -EINVAL;
      }

      down( &gInfo.client_lock );
      gInfo.num_clients--;
      list_del( &clientp->lnode );
      up( &gInfo.client_lock );

      kfree( clientp );

      HALAUDIO_TRACE( "client=0x%lx\n", (unsigned long)client_hdl );

      return gApi.freeClient( client_hdl );
   }
   return -EPERM;
}
EXPORT_SYMBOL( halAudioFreeClient );

/***************************************************************************/
/**
*  Otbain audio interface ID by name string
*
*  @return
*     0        Success
*     -ve      Failure code
*/
int halAudioQueryInterfaceByName(
   HALAUDIO_HDL client_hdl,         /**< (i) Client handle */
   const char  *name,               /**< (i) Name of interface */
   HALAUDIO_IF *id                  /**< (i) Ptr to store found interface ID */
)
{
   HALAUDIO_DEBUG( "client=0x%lx name='%s'\n", (unsigned long)client_hdl, name );
   if ( gApi.queryInterfaceByName )
   {
      return gApi.queryInterfaceByName( client_hdl, name, id );
   }
   return -EPERM;
}
EXPORT_SYMBOL( halAudioQueryInterfaceByName );

/***************************************************************************/
/**
*  Otbain audio codec channel ID by name string
*
*  @return
*     0        Success
*     -ve      Failure code
*/
int halAudioQueryCodecByName(
   HALAUDIO_HDL    client_hdl,      /**< (i) Client handle */
   const char     *name,            /**< (i) Name of codec channel */
   HALAUDIO_CODEC *id               /**< (i) Ptr to store found codec ID */
)
{
   HALAUDIO_DEBUG( "client=0x%lx name='%s'\n", (unsigned long)client_hdl, name );
   if ( gApi.queryCodecByName )
   {
      return gApi.queryCodecByName( client_hdl, name, id );
   }
   return -EPERM;
}
EXPORT_SYMBOL( halAudioQueryCodecByName );

/***************************************************************************/
/**
*  Acquire lock for an audio codec channel. Locking a codec will prevent
*  other clients from modifying any of its hardware parameters, such as
*  gains, equalizer parameters, and sampling frequency. Locking a channel
*  is not mandatory to make use of it. A user can make use of a channel
*  that is in an unlocked state; however, other clients are equally free
*  to modify the codec's hardware properties.
*
*  @return
*     0        Success
*     -EBUSY   Codec is already locked by another client, try again later.
*     -ENODEV  Non-existent codec or client
*
*  @remarks
*     Client can safely lock the same codec multiple times.
*/
int halAudioLockCodec(
   HALAUDIO_HDL    client_hdl,      /**< (i) Client handle */
   HALAUDIO_CODEC  cid              /**< (i) Codec to lock */
)
{
   if ( gApi.lockCodec )
   {
      return gApi.lockCodec( client_hdl, cid );
   }
   return -EPERM;
}
EXPORT_SYMBOL( halAudioLockCodec );

/***************************************************************************/
/**
*  Release lock for an audio codec channel.
*
*  @return
*     0        Success
*     -EINVAL  Non-existent codec, client does not hold the lock, or codec
*              is already unlocked.
*     -ENODEV  Non-existing codec
*/
int halAudioUnlockCodec(
   HALAUDIO_HDL    client_hdl,      /**< (i) Client handle */
   HALAUDIO_CODEC  cid              /**< (i) Codec ID to unlock */
)
{
   if ( gApi.unlockCodec )
   {
      return gApi.unlockCodec( client_hdl, cid );
   }
   return -EPERM;
}
EXPORT_SYMBOL( halAudioUnlockCodec );

/***************************************************************************/
/**
*  To query gain information for a particular analog or digital gain block.
*
*  @return
*     0        Success
*     -EINVAL  Invalid parameters
*     -EPERM   Parent interface is disabled
*     -ve      Failure code
*/
int halAudioGetGainInfo(
   HALAUDIO_HDL        client_hdl,  /**< (i) Client handle */
   HALAUDIO_BLOCK      block,       /**< (i) Block id for digital or analog gain */
   HALAUDIO_GAIN_INFO *info         /**< (o) Pointer to returned gain information */
)
{
   HALAUDIO_DEBUG( "client=0x%lx block=0x%x\n", (unsigned long)client_hdl, block );

   if ( gApi.getGainInfo )
   {
      return gApi.getGainInfo( client_hdl, block, info );
   }
   return -EPERM;
}
EXPORT_SYMBOL( halAudioGetGainInfo );


/***************************************************************************/
/**
*  To set gain for a particular analog or digital gain block.
*
*  @return
*     0        Success
*     -EBUSY   Codec is locked by another user
*     -ENODEV  Non-existing codec
*     -EPERM   Parent interface is disabled
*     -ve      Failure code
*/
int halAudioSetGain(
   HALAUDIO_HDL    client_hdl,      /**< (i) Client handle */
   HALAUDIO_BLOCK  block,           /**< (i) Block id for digital or analog gain */
   int             db               /**< (i) Gain in dB to set */
)
{
   HALAUDIO_TRACE( "client=0x%lx block=0x%x\n", (unsigned long)client_hdl, block );

   if ( gApi.setGain )
   {
      int err;
      err = gApi.setGain( client_hdl, block, db );
      if ( !err )
      {
         /* Platform specific extension */
         if ( gExtensionsOps.setgain )
         {
            err = gExtensionsOps.setgain( block, db );
         }
      }
      return err;
   }

   return -EPERM;
}
EXPORT_SYMBOL( halAudioSetGain );

/***************************************************************************/
/**
*  To adjust gain for a particular analog or digital gain block for any
*  number of unitless notches. The routine abstracts the knowledge of the
*  precise gain settings and ranges for the user.
*
*  @return
*     1        Gain level either reach the max or min setting
*     0        Success
*     -EINVAL  Invalid gain block
*     -ENODEV  Non-existing codec
*     -EPERM   Parent interface is disabled
*     -EBUSY   Codec is locked by another user
*/
int halAudioAlterGain(
   HALAUDIO_HDL    client_hdl,      /**< (i) Client handle */
   HALAUDIO_BLOCK  block,           /**< (i) Block id for digital or analog gain */
   int             delta            /**< (i) Number of notches to adjust gain by in unitless notches */
)
{
   HALAUDIO_GAIN_INFO   info;
   int                  db;
   int                  rc, err;

   HALAUDIO_TRACE( "client=0x%lx block=0x%x delta=%i\n", (unsigned long)client_hdl, block, delta );

   err = halAudioGetGainInfo( client_hdl, block, &info );
   if ( err )
   {
      return err;
   }

   rc = 0;
   db = info.currentdb;

   if ( info.range_type == HALAUDIO_RANGETYPE_FIXED_STEPSIZE )
   {
      if ( db < info.mindb )
      {
         /* Must be muted, adjust value to represent mute as a single step */
         db = info.mindb - info.range.fixed_step;
      }

      /* Calculate the next gain setting. */
      db += info.range.fixed_step * delta;

      if ( db >= info.maxdb )
      {
         db = info.maxdb;
         rc = 1;     /* volume min/max limited */
      }
      if ( db < info.mindb )
      {
         db = HALAUDIO_GAIN_MUTE;
         rc = 1;     /* volume min/max limited */
      }
   }
   else
   {
      int i;

      /* Find where we are in the list. Mute is not in the list and
       * is handled specially.
       */
      if ( db < info.range.list.db[0] )
      {
         /* Must be muted, adjust index to represent mute as a single step */
         i = -1;
      }
      else
      {
         for ( i = 0; ( i < (info.range.list.num - 1) && db > info.range.list.db[i] ); i++ );
      }

      /* Adjust for direction of change */
      i += delta;

      if ( i >= info.range.list.num )
      {
         i = info.range.list.num - 1;
         rc = 1;     /* volume min/max limited */
      }

      /* Get the new db value */
      if ( i < 0 )
      {
         db = HALAUDIO_GAIN_MUTE;
         rc = 1;     /* volume min/max limited */
      }
      else
      {
         db = info.range.list.db[i];
      }
   }

   err = halAudioSetGain( client_hdl, block, db );
   if ( err )
   {
      rc = err;
   }

   return rc;
}
EXPORT_SYMBOL( halAudioAlterGain );

/***************************************************************************/
/**
*  Set power level. The actual power level takes into consideration requests
*  from all clients with a bias for power to be enabled. For example, deep
*  sleep only occurs if there is consensus with all clients. In contrast,
*  active power level is engaged even if only one client needs power.
*
*  @return
*     0        Success
*     -EINVAL  Invalid power level
*/
int halAudioSetPower(
   HALAUDIO_HDL         client_hdl, /**< (i) Client handle */
   HALAUDIO_POWER_LEVEL level       /**< (i) Power level request */
)
{
   HALAUDIO_TRACE( "client=0x%lx level=%i\n", (unsigned long)client_hdl, level );

   if ( gApi.setPower )
   {
      int err;
      err = gApi.setPower( client_hdl, level );
      if ( !err )
      {
         struct halaudio_client *clientp = halaudio_get_clientp( client_hdl );
         if ( clientp )
         {
            clientp->powerlevel = level;
         }
         else
         {
            err = -EINVAL;
         }
      }
      return err;
   }

   return -EPERM;
}
EXPORT_SYMBOL( halAudioSetPower );

/***************************************************************************/
/**
*  Query actual power level.
*
*  @return
*     0        Success
*     -ve      Error code
*/
int halAudioGetPower(
   HALAUDIO_HDL         client_hdl, /**< (i) Client handle */
   HALAUDIO_POWER_LEVEL *level      /**< (o) Pointer to store power level */
)
{
   HALAUDIO_DEBUG( "client=0x%lx\n", (unsigned long)client_hdl );

   if ( gApi.getPower )
   {
      return gApi.getPower( client_hdl, level );
   }

   return -EPERM;
}
EXPORT_SYMBOL( halAudioGetPower );

/***************************************************************************/
/**
*  Write audio samples directly to an audio channel's hardware buffers.
*
*  @return
*     >= 0     Success, returns the number of bytes written
*     -EBUSY   Codec is already locked by other client, try again later.
*     -EPERM   Parent interface is disabled
*     -ve      Error code
*/
int halAudioWrite(
   HALAUDIO_HDL   client_hdl,       /**< (i) Client handle */
   HALAUDIO_CODEC cid,              /**< (i) Codec channel to write to */
   HALAUDIO_FMT   format,           /**< (i) Format of samples */
   const uint8_t *audio,            /**< (i) Ptr to data buffer to playback */
   int            bytes             /**< (i) Number of bytes contained in buffer */
)
{
   HALAUDIO_DEBUG( "cid=%u bytes=%d fmt=%i\n", cid, bytes, format );

   if ( gApi.write )
   {
      return gApi.write( client_hdl, cid, format, audio, bytes );
   }

   return -EPERM;
}
EXPORT_SYMBOL( halAudioWrite );

/***************************************************************************/
/**
*  Read audio samples directly from an audio channel's hardware buffers.
*
*  @return
*     >= 0     Number of bytes read
*     -EBUSY   Codec is already locked by other client, try again later.
*     -EPERM   Parent interface is disabled
*     -ve      Error code
*/
int halAudioRead(
   HALAUDIO_HDL   client_hdl,       /**< (i) Client handle */
   HALAUDIO_CODEC cid,              /**< (i) Codec channel to write to */
   HALAUDIO_FMT   format,           /**< (i) Format of samples */
   uint8_t       *audio,            /**< (o) Ptr to data buffer to record to */
   int            bytes             /**< (i) Max number of bytes to read */
)
{
   HALAUDIO_DEBUG( "cid=%u bytes=%d\n", cid, bytes );

   if ( gApi.read )
   {
      return gApi.read( client_hdl, cid, format, audio, bytes );
   }

   return -EPERM;
}
EXPORT_SYMBOL( halAudioRead );

/***************************************************************************/
/**
*  Enable and disable super user privileges. A super user is permitted
*  to modify any codec channel property regardless if the codec is in
*  a locked or unlocked state.
*
*  @return
*     0        Success
*     -1       Super user privileges not permitted
*/
int halAudioSetSuperUser(
   HALAUDIO_HDL   client_hdl,       /**< (i) Client handle */
   int            enable            /**< (i) 1 to enable super user, 0 to disable */
)
{
   HALAUDIO_TRACE( "client=0x%lx enable=%i\n", (unsigned long)client_hdl, enable );

   if ( gApi.setSuperUser )
   {
      int err;
      err = gApi.setSuperUser( client_hdl, enable );
      if ( !err )
      {
         struct halaudio_client *clientp = halaudio_get_clientp( client_hdl );
         if ( clientp )
         {
            clientp->superuser = enable;
         }
         else
         {
            err = -EINVAL;
         }
      }
      return err;
   }

   return -EPERM;
}
EXPORT_SYMBOL( halAudioSetSuperUser );

/***************************************************************************/
/**
*  Set sampling frequency for an audio codec channel
*
*  @return
*     0        Success
*     -EBUSY   Codec is already locked by other client, try again later.
*     -EINVAL  Invalid sampling frequency to set, or invalid codec
*     -EPERM   Parent interface is disabled
*
*  @remarks
*     Changing the sampling frequency while a codec is running may produce
*     an audible glitch. To minimize audible glitches, it is recommended
*     that the the analog gains be lowered prior to switching the sampling
*     frequency.
*/
int halAudioSetFreq(
   HALAUDIO_HDL   client_hdl,       /**< (i) Client handle */
   HALAUDIO_CODEC cid,              /**< (i) Codec channel to write to */
   int            freqhz            /**< (i) Sampling frequency in Hz to set */
)
{
   HALAUDIO_TRACE( "cid=%u freqhz=%d\n", cid, freqhz );

   if ( gApi.setFreq )
   {
      return gApi.setFreq( client_hdl, cid, freqhz );
   }

   return -EPERM;
}
EXPORT_SYMBOL( halAudioSetFreq );

/***************************************************************************/
/**
*  Query sampling frequency for an audio codec channel
*
*  @return
*     0        Success
*     -EINVAL  Invalid sampling frequency to set, or invalid codec
*     -EPERM   Parent interface is disabled
*/
int halAudioGetFreq(
   HALAUDIO_HDL   client_hdl,       /**< (i) Client handle */
   HALAUDIO_CODEC cid,              /**< (i) Codec channel to write to */
   int           *freqhz            /**< (o) Ptr to store sampling freq */
)
{
   HALAUDIO_DEBUG( "cid=%u\n", cid );

   if ( gApi.getFreq )
   {
      return gApi.getFreq( client_hdl, cid, freqhz );
   }

   return -EPERM;
}
EXPORT_SYMBOL( halAudioGetFreq );

/***************************************************************************/
/**
*  Set equalizer parameters for a particular audio codec channel
*
*  @return
*     0        Success
*     -EBUSY   Codec is already locked by other client, try again later.
*     -EINVAL  Invalid parameters
*     -EPERM   Parent interface is disabled
*/
int halAudioSetEquParms(
   HALAUDIO_HDL         client_hdl, /**< (i) Client handle */
   HALAUDIO_CODEC       cid,        /**< (i) Codec to write to */
   HALAUDIO_DIR         dir,        /**< (i) Select equalizer direction */
   const HALAUDIO_EQU  *equ         /**< (i) Pointer to equalizer parameters */
)
{
   HALAUDIO_TRACE( "cid=%u dir=%u len=%d\n", cid, dir, equ->len );

   if ( gApi.setEquParms )
   {
      return gApi.setEquParms( client_hdl, cid, dir, equ );
   }

   return -EPERM;
}
EXPORT_SYMBOL( halAudioSetEquParms );

/***************************************************************************/
/**
*  Query equalizer parameters for a particular audio codec channel
*
*  @return
*     0        Success
*     -EINVAL  Invalid parameters
*     -EPERM   Parent interface is disabled
*/
int halAudioGetEquParms(
   HALAUDIO_HDL         client_hdl, /**< (i) Client handle */
   HALAUDIO_CODEC       cid,        /**< (i) Codec channel to write to */
   HALAUDIO_DIR         dir,        /**< (i) Select equalizer direction */
   HALAUDIO_EQU        *equ         /**< (o) Ptr to store equalizer parameters */
)
{
   HALAUDIO_DEBUG( "cid=%u dir=%u\n", cid, dir );

   if ( gApi.getEquParms )
   {
      return gApi.getEquParms( client_hdl, cid, dir, equ );
   }

   return -EPERM;
}
EXPORT_SYMBOL( halAudioGetEquParms );

/***************************************************************************/
/**
*  Query high-level hardware information. To get detailed sub-component
*  information call halAudioGetInterfaceInfo, halAudioGetCodecInfo and
*  halAudioGetGainInfo.
*
*  @return
*     0        Success
*     -EINVAL  Invalid parameters
*/
int halAudioGetHardwareInfo(
   HALAUDIO_HDL       client_hdl,   /**< (i) Client handle */
   HALAUDIO_HW_INFO  *info          /**< (o) Ptr to store hardware info */
)
{
   HALAUDIO_DEBUG( "client=0x%lx\n", (unsigned long)client_hdl );

   if ( gApi.getHardwareInfo )
   {
      return gApi.getHardwareInfo( client_hdl, info );
   }

   return -EPERM;
}
EXPORT_SYMBOL( halAudioGetHardwareInfo );

/***************************************************************************/
/**
*  Query audio interface information.
*
*  @return
*     0        Success
*     -EINVAL  Invalid parameters
*/
int halAudioGetInterfaceInfo(
   HALAUDIO_HDL         client_hdl, /**< (i) Client handle */
   HALAUDIO_IF          id,         /**< (i) Interface id */
   HALAUDIO_IF_INFO    *info        /**< (o) Ptr to store interface info */
)
{
   HALAUDIO_DEBUG( "client=0x%lx interface=%u\n", (unsigned long)client_hdl, id );

   if ( gApi.getInterfaceInfo )
   {
      return gApi.getInterfaceInfo( client_hdl, id, info );
   }

   return -EPERM;
}
EXPORT_SYMBOL( halAudioGetInterfaceInfo );

/***************************************************************************/
/**
*  Query audio codec information.
*
*  @return
*     0        Success
*     -EINVAL  Invalid parameters
*     -EPERM   Parent interface is disabled
*/
int halAudioGetCodecInfo(
   HALAUDIO_HDL         client_hdl, /**< (i) Client handle */
   HALAUDIO_CODEC       cid,        /**< (i) Codec channel to write to */
   HALAUDIO_CODEC_INFO *info        /**< (o) Ptr to store codec info */
)
{
   HALAUDIO_DEBUG( "client=0x%lx cid=%u\n", (unsigned long)client_hdl, cid );

   if ( gApi.getCodecInfo )
   {
      return gApi.getCodecInfo( client_hdl, cid, info );
   }

   return -EPERM;
}
EXPORT_SYMBOL( halAudioGetCodecInfo );

/***************************************************************************/
/**
*  Enable an audio interface. By default, audio interfaces are all
*  enabled at start up. However, in some cases, it is necessary or
*  desirable to disable an interface and re-enable at a later time.
*  For example, if interface is slaved to an external clock and that
*  clock needs to power down, it is necessary to disable the interface
*  first to ensure that synchronization between interfaces does not
*  get out of phase.
*
*  @return
*     0        Success
*     -EINVAL  Invalid parameters
*/
int halAudioEnableInterface(
   HALAUDIO_HDL         client_hdl, /**< (i) Client handle */
   HALAUDIO_IF          id,         /**< (i) Interface number */
   int                  enable      /**< (i) 1 to enable, 0 to disable */
)
{
   HALAUDIO_DEBUG( "client=0x%lx id=%u enable=%i\n", (unsigned long)client_hdl, id, enable );

   if ( gApi.enableInterface )
   {
      return gApi.enableInterface( client_hdl, id, enable );
   }

   return -EPERM;
}
EXPORT_SYMBOL( halAudioEnableInterface );

/***************************************************************************/
/**
*  Setup proprietary HAL Audio framework API functions
*
*  @return    none
*/
void halAudioSetApiFuncs(
   const HALAUDIO_API_FUNCS *funcsp /**< (i) Ptr to API functions.
                                             If NULL, clears registration */
)
{
   unsigned long flags;

   local_irq_save( flags );
   if ( funcsp )
   {
      memcpy( &gApi, funcsp, sizeof(gApi) );
   }
   else
   {
      /* Clear callbacks */
      memset( &gApi, 0, sizeof(gApi) );
   }
   local_irq_restore( flags );
}
EXPORT_SYMBOL( halAudioSetApiFuncs );

/***************************************************************************/
/**
*  Register power management operations.
*
*  @return
*     0     - Success
*     -ve   - error code
*/
int halAudioSetPowerManageOps(
   HALAUDIO_PM_OPS *pm_ops            /**< (in) Power management callbacks
                                                If NULL, clears registration */
)
{
   unsigned long flags;

   local_irq_save( flags );
   if ( pm_ops )
   {
      memcpy( &gPMOps, pm_ops, sizeof( gPMOps ));
   }
   else
   {
      memset( &gPMOps, 0, sizeof(gPMOps) );
   }
   local_irq_restore( flags );

   return 0;
}
EXPORT_SYMBOL( halAudioSetPowerManageOps );

/***************************************************************************/
/**
*  Set platform extension operations. This routine is called to setup platform
*  specific extensions.
*
*  @return Nothing
*/
void halAudioSetExtensionsOps(
   const HALAUDIO_EXTENSIONS_OPS *extensions_ops  /**< (in) Extension ops */
)
{
   unsigned long flags;

   local_irq_save( flags );
   if ( extensions_ops )
   {
      memcpy( &gExtensionsOps, extensions_ops, sizeof( gExtensionsOps ));
   }
   else
   {
      memset( &gExtensionsOps, 0, sizeof(gExtensionsOps) );
   }
   local_irq_restore( flags );
}
EXPORT_SYMBOL( halAudioSetExtensionsOps );

/***************************************************************************/
/**
*  Add an HAL Audio Interface
*
*  @return
*     0        success
*     -ve      Error code such as -EFAULT, -ENOMEM, -EINVAL
*
*  @remarks
*     Interfaces by default will be synchronized by HAL Audio based on
*     their interrupt periodicity. However, some interfaces may choose
*     not to synchronize with other interfaces because of timing
*     limitations or other reasons. To opt out of synchronization, set
*     the frame period to 0.
*/
int halAudioAddInterface(
   HALAUDIO_IF_OPS  *ops,           /**< (i) Interface operations */
   unsigned int      codecs,        /**< (i) Total number of audio codec channels */
   const char       *name,          /**< (i) Name string */
   int               frame_usec,    /**< (i) Interrupt period in usec. 0 means not synchronized */
   int               sync,          /*<< (i) Requests to synchronize with other interfaces */
   HALAUDIO_IF_HDL  *hdlp           /**< (o) Ptr to store handle */
)
{
   if ( gApi.addInterface )
   {
      return gApi.addInterface( ops, codecs, name, frame_usec, sync, hdlp );
   }
   return -EPERM;
}
EXPORT_SYMBOL( halAudioAddInterface );

/***************************************************************************/
/**
*  Delete HAL Audio Interface. All associated audio codec channels are
*  cleaned up as part of deleting the interface.
*
*  @return
*     0        Success
*     -1       Failed to delete interface, may have orphaned audio codec
*              channel resources
*     -ve      Other error codes
*/
int halAudioDelInterface(
   HALAUDIO_IF_HDL handle           /**< (i) interface to delete */
)
{
   if ( gApi.delInterface )
   {
      return gApi.delInterface( handle );
   }
   return -EPERM;
}
EXPORT_SYMBOL( halAudioDelInterface );

/***************************************************************************/
/**
*  Changes an interface's synchronization behavior. Synchronization refers
*  to timing with other interfaces. All synchronized interfaces are expected
*  to start at the same time and execute at similar sampling frequencies.
*  Unsynchronized interfaces are free to execute at any sampling 
*  frequencies; however, the their clocks do not contribute to the 
*  HAL Audio wall clock.
*
*  @return
*     0        on success
*     -1       Failed, may have orphan codec channels
*     -ve      Other error codes
*
*  @remarks
*/
int halAudioSetSyncFlag(
   HALAUDIO_IF_HDL   handle,        /*<< (i) interface to delete */
   int               sync           /*<< (i) 1 requests to sync with other interfaces */
)
{
   if ( gApi.setSyncFlag )
   {
      return gApi.setSyncFlag( handle, sync );
   }
   return -EPERM;
}
EXPORT_SYMBOL( halAudioSetSyncFlag );

/***************************************************************************/
/**
*  Set CSX IO callbacks for debug infrastructure to support audio inject and
*  capture points.
*
*  @return
*     0        Success
*     -EAGAIN  Failed to acquire resource
*     -ve      Other error codes
*/
int halAudioSetCsxIoPoints(
   HALAUDIO_CODEC           cid,    /**< (i) Codec to install CSX callbacks */
   HALAUDIO_CSX_POINT_ID    point,  /**< (i) Point ID for CSX callbacks */
   const CSX_IO_POINT_FNCS *fncp,   /**< (i) List of callbacks */
   void                    *data    /**< (i) User data */
)
{
   if ( gApi.setCsxIoPoints )
   {
      return gApi.setCsxIoPoints( cid, point, fncp, data );
   }
   return -EPERM;
}
EXPORT_SYMBOL( halAudioSetCsxIoPoints );

/***************************************************************************/
/**
*  Set CSX frame sync callback for debug infrastructure to support alignment
*  of audio inject and capture operations.
*
*  @return
*     0        Success
*     -ve      Other error codes
*/
int halAudioSetCsxFrameSync(
   CSX_FRAME_SYNC_FP fncp,    /**< (i) Frame sync callback */
   void              *data    /**< (i) User data */
)
{
   if ( gApi.setCsxFrameSync )
   {
      return gApi.setCsxFrameSync( fncp, data );
   }
   return -EPERM;
}
EXPORT_SYMBOL( halAudioSetCsxFrameSync );

/***************************************************************************/
/**
*  Proc read callback function
*
*  @return  Number of characters to print
*/
static int halaudio_read_proc( char *buf, char **start, off_t offset, int count, int *eof, void *data )
{
   int                           len, err;
   struct halaudio_client       *clientp;
   HALAUDIO_HW_INFO              hwinfo;

   len = 0;

   err = halAudioGetHardwareInfo( NULL, &hwinfo );
   if ( !err )
   {
      HALAUDIO_IF_INFO     ifinfo;
      HALAUDIO_CODEC_INFO  codecinfo;
      int                  i, id;

      /* Dump interface info */
      len += sprintf( buf+len, "Interfaces: total=%i\n", hwinfo.interfaces );
      for ( i = 0, id = 0; i < hwinfo.interfaces && id < HALAUDIO_MAX_INTERFACES; i++ )
      {
         /* Scan ID */
         do
         {
            err = halAudioGetInterfaceInfo( NULL, id, &ifinfo );
            if ( !err )
            {
               len += sprintf( buf+len, " %s: id=%i codecs=%i frame_period=%i enabled=%i sync=%i\n",
                     ifinfo.name, id, ifinfo.codecs, ifinfo.frame_period_us, ifinfo.enabled, ifinfo.sync );
            }
            id++;
         }
         while ( err );
      }

      /* Dump codec info */
      len += sprintf( buf+len, "Codecs: total=%i\n", hwinfo.codecs );
      for ( i = 0, id = 0; i < hwinfo.codecs && id < HALAUDIO_MAX_CODECS; i++ )
      {
         /* Scan ID */
         do
         {
            err = halAudioGetCodecInfo( NULL, id, &codecinfo );
            if ( !err )
            {
               /* Get parent interface info */
               halAudioGetInterfaceInfo( NULL, codecinfo.parent_id, &ifinfo );

               len += sprintf( buf+len, " %s: cid=%i locked=0x%lx\n",
                     codecinfo.name, id, codecinfo.locked );
            }
            id++;
         }
         while ( err );
      }
   }

   len += sprintf( buf+len, "Clients: total=%i\n", gInfo.num_clients );
   down( &gInfo.client_lock );
   list_for_each_entry( clientp, &gInfo.client_list, lnode )
   {
      len += sprintf( buf+len, " 0x%lx: superuser=%i powerlevel=%i rcid=%i rfmt=%i wcid=%i wfmt=%i\n",
            (unsigned long)clientp->handle, clientp->superuser,
            clientp->powerlevel, clientp->read.cid, clientp->read.format,
            clientp->write.cid, clientp->write.format );
   }
   up( &gInfo.client_lock );

   *eof = 1;
   return len+1;
}

/***************************************************************************/
/**
*  Initialize debug interfaces.
*
*  @return  none
*
*  @remarks
*/
static void halaudio_debug_init( void )
{
   create_proc_read_entry( HALAUDIO_PROC_NAME, 0, NULL, halaudio_read_proc, NULL );
   gSysCtlHeader = register_sysctl_table( gSysCtl );
}

/***************************************************************************/
/**
*  Cleanup debug interfaces.
*
*  @return  none
*
*  @remarks
*/
static void halaudio_debug_exit( void )
{
   remove_proc_entry( HALAUDIO_PROC_NAME, NULL );

   if ( gSysCtlHeader )
   {
      unregister_sysctl_table( gSysCtlHeader );
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
static int halaudio_open(
   struct inode *inode,             /**< (io) Pointer to inode info */
   struct file  *file               /**< (io) File structure pointer */
)
{
   HALAUDIO_HDL client_hdl;

   client_hdl = halAudioAllocateClient();
   if ( client_hdl == NULL )
   {
      return -ENOMEM;
   }

   file->private_data = halaudio_get_clientp( client_hdl );

   HALAUDIO_DEBUG( "client=0x%lx\n", (unsigned long)client_hdl );

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
static int halaudio_release(
   struct inode *inode,             /**< (io) Pointer to inode info */
   struct file  *file               /**< (io) File structure pointer */
)
{
   struct halaudio_client *clientp = file->private_data;
   HALAUDIO_HDL            client_hdl;

   clientp     = file->private_data;
   client_hdl  = clientp->handle;

   HALAUDIO_DEBUG( "client=0x%lx\n", (unsigned long)clientp->handle );

   return halAudioFreeClient( client_hdl );
}

/***************************************************************************/
/**
*  Driver read method
*
*  @return
*     >= 0           Number of bytes read
*     -ve            Error code
*/
static ssize_t halaudio_read(
   struct file *file,
   char __user *buffer,
   size_t count,
   loff_t *ppos
)
{
   uint8_t                *kbuf;
   int                     bytes_read;
   HALAUDIO_HDL            client_hdl;
   struct halaudio_client *clientp;
   int                     rc;

   kbuf = vmalloc( count );
   if ( !kbuf )
   {
      return -ENOMEM;
   }

   clientp     = file->private_data;
   client_hdl  = clientp->handle;

   bytes_read  = halAudioRead( client_hdl, clientp->read.cid,
#if defined (__LITTLE_ENDIAN)
         HALAUDIO_FMT_S16_LE,
#else
         HALAUDIO_FMT_S16_BE,
#endif
         kbuf, (int)count );

   if ( bytes_read > 0 )
   {
      if (( rc = copy_to_user( buffer, kbuf, bytes_read )) != 0 )
      {
         vfree( kbuf );
         return -EIO;
      }
   }

   vfree( kbuf );

   return bytes_read;
}

/***************************************************************************/
/**
*  Helper routine to decompress G.711 encoded data.
*/
static void uncompress(
   int16_t *outbuf,                 /**< (o) Ptr to 16-bit uncompressed output */
   const uint8_t *inbuf,            /**< (i) Ptr to 8-bit compressed input */
   const int16_t *table,            /**< (i) Ptr to table lookup */
   int count                        /**< (i) Number of samples to operate on */
)
{
   int i;

   /* Uncompress from back to front in the case output buffer
    * is the same as input buffer to save memory
    */
   for ( i = count - 1; i >= 0; i-- )
   {
      unsigned int offset = inbuf[i];
      outbuf[i] = table[offset];
   }
}

/***************************************************************************/
/**
*  Driver write method. Some format conversions are added to
*  this method to support legacy functionality.
*
*  @return
*     >= 0           Number of bytes write
*     -ve            Error code
*/
static ssize_t halaudio_write( struct file *file, const char __user *buffer, size_t count, loff_t *ppos )
{
   uint8_t                *kbuf;
   HALAUDIO_HDL            client_hdl;
   struct halaudio_client *clientp;
   HALAUDIO_FMT           format;
   int                    bytes_written;
   int                    bufsize;
   int                    rc;
#if defined ( __LITTLE_ENDIAN )
   int                    be_flag = 0;
#elif defined ( __BIG_ENDIAN )
   int                    be_flag = 1;
#else
#error No endian defined.
#endif

   clientp     = file->private_data;
   client_hdl  = clientp->handle;

   format      = clientp->write.format;
   bufsize     = count;

   if ( format == HALAUDIO_FMT_ULAW || format == HALAUDIO_FMT_ALAW )
   {
      /* Uncompressed size is double the size*/
      bufsize = bufsize * 2;
   }

   kbuf = vmalloc( bufsize );
   if ( !kbuf )
   {
      return -ENOMEM;
   }

   if (( rc = copy_from_user( kbuf, buffer, count )) < 0 )
   {
      printk( KERN_ERR "%s: failed to copy data from user buffer\n", __FUNCTION__ );
      vfree( kbuf );
      return -EIO;
   }

   if ( format == HALAUDIO_FMT_ULAW )
   {
      uncompress( (int16_t *)(void *)kbuf, kbuf, ulawExpandTable, count );
   }
   else if ( format == HALAUDIO_FMT_ALAW )
   {
      uncompress( (short *)(void *)kbuf, kbuf, alawExpandTable, count );
   }
   else if (( be_flag && format != HALAUDIO_FMT_S16_BE )
   ||       ( be_flag == 0 && format != HALAUDIO_FMT_S16_LE ))
   {
      int i;
      uint8_t *tmpbuf = kbuf;

      if ( count & 1 )
      {
         /* must be even number of bytes for 16-bit data */
         vfree( kbuf );
         return -EINVAL;
      }

      /* Byte swap 16-bit data when format inconsistent with machine */
      for ( i = 0; i < count; i += 2 )
      {
         uint8_t swap = *tmpbuf;
         tmpbuf[0] = tmpbuf[1];
         tmpbuf++;
         *tmpbuf = swap;
         tmpbuf++;
      }
   }

   bytes_written = halAudioWrite( client_hdl, clientp->write.cid,
#if defined (__LITTLE_ENDIAN)
         HALAUDIO_FMT_S16_LE,
#else
         HALAUDIO_FMT_S16_BE,
#endif
         kbuf, bufsize );

   if ( bytes_written == bufsize )
   {
      /* Successful write, return original uncompressed buffer size. */
      bytes_written = count;
   }

   vfree( kbuf );

   return bytes_written;
}

/***************************************************************************/
/**
*  Driver ioctl method to support user library API.
*
*  @return
*     >= 0           Number of bytes write
*     -ve            Error code
*/
static long halaudio_ioctl(
   struct file   *file,             /**< (io) File structure pointer */
   unsigned int   cmd,              /**< (i)  IOCTL command */
   unsigned long  arg               /**< (i)  User argument */
)
{
   HALAUDIO_HDL                        client_hdl;
   struct halaudio_client             *clientp;
   int                                 rc;
   union halaudio_ioctl_params         parm;
   unsigned int                        cmdnr, size;

   cmdnr    = _IOC_NR( cmd );
   size     = _IOC_SIZE( cmd );

   HALAUDIO_DEBUG( "cmdnr=%i\n", cmdnr );

   if ( size > sizeof(parm) )
   {
      return -ENOMEM;
   }

   if ( size )
   {
      if (( rc = copy_from_user( &parm, (void *)arg, size )) != 0 )
      {
         printk( KERN_ERR "%s: (%d) FAILED copy_from_user status=%d\n", __FUNCTION__, cmdnr, rc );
         return rc;
      }
   }

   clientp     = file->private_data;
   client_hdl  = clientp->handle;
   rc          = -EPERM;

   switch ( cmdnr )
   {
      case HALAUDIO_CMD_LOCK_CODEC:
      {
         rc = halAudioLockCodec( client_hdl, (HALAUDIO_CODEC)arg );
      }
      break;

      case HALAUDIO_CMD_UNLOCK_CODEC:
      {
         rc = halAudioUnlockCodec( client_hdl, (HALAUDIO_CODEC)arg );
      }
      break;

      case HALAUDIO_CMD_GET_GAIN_INFO:
      {
         HALAUDIO_GAIN_INFO info;
         rc = halAudioGetGainInfo( client_hdl, parm.getgain.block, &info );
         if ( rc == 0 )
         {
            rc = copy_to_user( parm.getgain.info, &info, sizeof(info) );
         }
      }
      break;

      case HALAUDIO_CMD_SET_GAIN:
      {
         rc = halAudioSetGain( client_hdl, parm.setgain.block, parm.setgain.db );
      }
      break;

      case HALAUDIO_CMD_ALTER_GAIN:
      {
         rc = halAudioAlterGain( client_hdl, parm.altergain.block, parm.altergain.delta );
      }
      break;

      case HALAUDIO_CMD_SET_POWER:
      {
         rc = halAudioSetPower( client_hdl, (HALAUDIO_POWER_LEVEL)arg );
      }
      break;

      case HALAUDIO_CMD_GET_POWER:
      {
         HALAUDIO_POWER_LEVEL level;
         rc = halAudioGetPower( client_hdl, &level );
         if ( rc == 0 )
         {
            rc = copy_to_user( (void *)arg, &level, sizeof(level) );
         }
      }
      break;

      case HALAUDIO_CMD_READ_PARMS:
      {
         clientp->read.cid    = parm.rw_parms.cid;
         clientp->read.format = parm.rw_parms.format;
         rc = 0;
      }
      break;

      case HALAUDIO_CMD_WRITE_PARMS:
      {
         clientp->write.cid      = parm.rw_parms.cid;
         clientp->write.format   = parm.rw_parms.format;
         rc = 0;
      }
      break;

      case HALAUDIO_CMD_SET_SUPERUSER:
      {
         rc = halAudioSetSuperUser( client_hdl, (int)arg );
      }
      break;

      case HALAUDIO_CMD_SET_FREQ:
      {
         rc = halAudioSetFreq( client_hdl, parm.setfreq.cid, parm.setfreq.freqhz );
      }
      break;

      case HALAUDIO_CMD_GET_FREQ:
      {
         int freqhz;
         rc = halAudioGetFreq( client_hdl, parm.getfreq.cid, &freqhz );
         if ( rc == 0 )
         {
            rc = copy_to_user( parm.getfreq.freqhz, &freqhz, sizeof(freqhz) );
         }
      }
      break;

      case HALAUDIO_CMD_SET_EQU_PARMS:
      {
         HALAUDIO_EQU equ;
         if (( rc = copy_from_user( &equ, parm.setequ.equ, sizeof(equ) )) != 0 )
         {
            return rc;
         }
         rc = halAudioSetEquParms( client_hdl, parm.setequ.cid, parm.setequ.dir, &equ );
      }
      break;

      case HALAUDIO_CMD_GET_EQU_PARMS:
      {
         HALAUDIO_EQU equ;
         rc = halAudioGetEquParms( client_hdl, parm.getequ.cid, parm.getequ.dir, &equ );
         if ( rc == 0 )
         {
            rc = copy_to_user( parm.getequ.equ, &equ, sizeof(equ) );
         }
      }
      break;

      case HALAUDIO_CMD_GET_HARDWARE_INFO:
      {
         HALAUDIO_HW_INFO info;
         rc = halAudioGetHardwareInfo( client_hdl, &info );
         if ( rc == 0 )
         {
            rc = copy_to_user( (void *)arg, &info, sizeof(info) );
         }
      }
      break;

      case HALAUDIO_CMD_GET_INTERFACE_INFO:
      {
         HALAUDIO_IF_INFO info;
         rc = halAudioGetInterfaceInfo( client_hdl, parm.ifinfo.interface, &info );
         if ( rc == 0 )
         {
            rc = copy_to_user( parm.ifinfo.info, &info, sizeof(info) );
         }
      }
      break;

      case HALAUDIO_CMD_GET_CODEC_INFO:
      {
         HALAUDIO_CODEC_INFO info;
         rc = halAudioGetCodecInfo( client_hdl, parm.codecinfo.cid, &info );
         if ( rc == 0 )
         {
            rc = copy_to_user( parm.codecinfo.info, &info, sizeof(info) );
         }
      }
      break;

      case HALAUDIO_CMD_ENABLE_INTERFACE:
      {
         rc = halAudioEnableInterface( client_hdl, parm.enableif.interface, parm.enableif.enable );
      }
      break;

      case HALAUDIO_CMD_QUERY_INTERFACE_ID_BY_NAME:
      {
         HALAUDIO_IF id;
         rc = halAudioQueryInterfaceByName( client_hdl, parm.interfid.name, &id );
         if ( rc == 0 )
         {
            rc = copy_to_user( parm.interfid.id, &id, sizeof(id) );
         }
      }
      break;

      case HALAUDIO_CMD_QUERY_CODEC_ID_BY_NAME:
      {
         HALAUDIO_CODEC id;
         rc = halAudioQueryCodecByName( client_hdl, parm.codecid.name, &id );
         if ( rc == 0 )
         {
            rc = copy_to_user( parm.codecid.id, &id, sizeof(id) );
         }
      }
      break;

      default:
      {
         rc = -EINVAL;
      }
   }

   return rc;
}

/***************************************************************************/
/**
*  Platform support constructor
*/
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36)
static int __init halaudio_probe( struct platform_device *pdev )
#else
static int halaudio_probe( struct platform_device *pdev )
#endif
{
#ifdef CONFIG_BCM_BSC
#define MAX_NAMES_BUFFER_LEN 256
   int                i;
   const char        *name = "halaudio_devices";
   char               names_buffer[MAX_NAMES_BUFFER_LEN];
   int                device_name_len = 0;
   int                names_len = 0;
#endif
   int                err;
   HALAUDIO_CFG      *cfg;
   HALAUDIO_DEV_CFG  *dev;

   cfg = pdev->dev.platform_data;
   if ( cfg == NULL || cfg->numdev == 0 )
   {
      printk( KERN_ERR "%s: failed to obtain platform data\n", __FUNCTION__ );
      return -ENODEV;
   }

   dev = cfg->devlist;
#ifdef CONFIG_BCM_BSC
   /* store the Hal Audio device configurations into the BSC driver */
   for ( i = 0; i < cfg->numdev; i++ )
   {
      err = bsc_register( dev[i].name, &dev[i], sizeof( dev[i] ));
      if ( err )
      {
         printk( KERN_ERR "%s: failed to register %s data into the BSC driver\n", __FUNCTION__, dev[i].name );
         return -EFAULT;
      }
      /* Copy device name into devices list */
      device_name_len = strlen(dev[i].name);
      if ( device_name_len > (MAX_NAMES_BUFFER_LEN - names_len) )
      {
         printk( KERN_ERR "%s: failed to fix %s into %s BSC entry\n", __FUNCTION__, dev[i].name, name );
         return -EFAULT;
      }
      memcpy( names_buffer + names_len, dev[i].name, device_name_len);
      names_len += device_name_len;
      /* Add space separator between names. */
      names_buffer[ names_len ] = ' ';
      names_len += 1;
   }
   /* Store list of device names */
   err = bsc_register( name, names_buffer, names_len);
   if ( err )
   {
      printk( KERN_ERR "%s: failed to register %s data into the BSC driver\n", __FUNCTION__, name );
      return -EFAULT;
   }
#endif

   gDriverMajor = register_chrdev( 0, HALAUDIO_NAME, &gfops );
   if ( gDriverMajor < 0 )
   {
      printk( KERN_ERR "HALAUDIO: Failed to register character device major\n" );
      err = -EFAULT;
      goto error_cleanup;
   }

#ifdef CONFIG_SYSFS
   halaudio_class = class_create(THIS_MODULE,"bcm-halaudio");
   if(IS_ERR(halaudio_class)){
	   printk(KERN_ERR "HALAUDIO: Class create failed\n");
	   err = -EFAULT;
	   goto err_unregister_chrdev;
   }

   halaudio_dev = device_create(halaudio_class, NULL, MKDEV(gDriverMajor,0),NULL,"halaudio");
   if(IS_ERR(halaudio_dev)){
	   printk(KERN_ERR "HALAUDIO: Device create failed\n");
	   err = -EFAULT;
	   goto err_class_destroy;
   }
#endif

   printk( KERN_INFO "HAL Audio Driver: 1.0\n" );

return 0;

#ifdef CONFIG_SYSFS
err_class_destroy:
   class_destroy(halaudio_class);
err_unregister_chrdev:
   unregister_chrdev(gDriverMajor, HALAUDIO_NAME);
#endif

error_cleanup:
   halaudio_remove( pdev );
   return err;
}

/***************************************************************************/
/**
*  Platform support destructor
*/
static int halaudio_remove( struct platform_device *pdev )
{
   struct halaudio_client *clientp, *tmp;
#ifdef CONFIG_BCM_BSC
   int                     i;
   HALAUDIO_CFG           *cfg;
   HALAUDIO_DEV_CFG       *dev;
#endif

#ifdef CONFIG_SYSFS
   device_destroy(halaudio_class,MKDEV(gDriverMajor,0));
   class_destroy(halaudio_class);
#endif

   unregister_chrdev( gDriverMajor, "halaudio" );

   /* Free any leftover clients */
   list_for_each_entry_safe( clientp, tmp, &gInfo.client_list, lnode )
   {
      halAudioFreeClient( clientp->handle );
   }

#ifdef CONFIG_BCM_BSC
   cfg = pdev->dev.platform_data;

   /* store the Hal Audio device configurations into the BSC driver */
   dev = cfg->devlist;
   for ( i = 0; i < cfg->numdev; i++ )
   {
      bsc_unregister( dev[i].name );
   }
#endif

   return 0;
}

/* Platform bus driver */
static struct platform_driver halaudio_driver =
{
   .driver     =
   {
      .name    = "bcm-halaudio",
      .owner   = THIS_MODULE,
   },
   .probe = halaudio_probe,
   .remove = halaudio_remove,
#ifdef CONFIG_PM
   .suspend    = pmSuspend,
   .resume     = pmResume,
#endif
};

static int __init halaudio_init( void )
{
   INIT_LIST_HEAD( &gInfo.client_list );
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,37)
   init_MUTEX( &gInfo.client_lock );
#else
   sema_init( &gInfo.client_lock , 1 );
#endif
   halaudio_debug_init();

   return platform_driver_register( &halaudio_driver );
}

static void __exit halaudio_exit( void )
{
   halaudio_debug_exit();

   platform_driver_unregister( &halaudio_driver );
}

module_init( halaudio_init );
module_exit( halaudio_exit );
MODULE_AUTHOR( "Broadcom" );
MODULE_DESCRIPTION( "HAL Audio driver" );
MODULE_LICENSE( "GPL" );
