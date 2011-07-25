/*****************************************************************************
* Copyright 2008 - 2010 Broadcom Corporation.  All rights reserved.
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
*  @file    halaudio_core.c
*
*  @brief   This file implements the HAL Audio core framework.
*
*****************************************************************************/

/* ---- Include Files ---------------------------------------------------- */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/semaphore.h>                 /* Semaphores */
#include <linux/proc_fs.h>
#include <linux/sched.h>

#include <linux/broadcom/knllog.h>           /* For debugging */
#include <linux/broadcom/halaudio.h>         /* HAL Audio API */
#include <linux/broadcom/amxr.h>             /* Audio mixer API */
#include <linux/broadcom/llist.h>            /* Link list definitions */

/* ---- Public Variables ------------------------------------------------- */

/* ---- Private Constants and Types -------------------------------------- */

/**
*  The default frame period defines the frequency of audio interrupts
*  expected in usec. All registered audio interfaces are expected to
*  run at this rate to maintain synchronicity.
*/
#define HALAUDIO_DEFAULT_FRAME_PERIOD          5000

/**
*  Read/write state structure used to implement non-blocking read/write
*/
struct halaudio_rw_state
{
   struct semaphore           sem;              /* Semaphore used for blocking IO */
   int                        bytes;            /* Number of bytes returned */
   HALAUDIO_CODEC             cid;              /* Codec id */
   HALAUDIO_FMT               format;           /* Data format */
};

/**
* Interface enabled states. These states are used to determine whether
* an interface is to be synchronized with other interfaces.
* An interface that is in the pending enable state is transitioning from the
* disabled state. It will be enabled at the completion of a roll-call. This
* delay mechanism allows an interface to synchronize start times with
* other interfaces.
*/
#define HALAUDIO_INTERFACE_DISABLED          0  /* Interface disabled */
#define HALAUDIO_INTERFACE_ENABLED           1  /* Interface enabled */
#define HALAUDIO_INTERFACE_PENDING_ENABLE    2  /* Interface will soon be enabled */

/**
*  Audio interface list node
*/
struct halaudio_if_node
{
   LLIST_NODE                 lnode;            /* List node - must be first */
   struct semaphore           mutex;            /* Protect interface items */
   char                       name[32];         /* Interface name */
   HALAUDIO_IF                id;               /* Interface id */
   int                        codecs;           /* Number of codec channels in interface */
   int                        frame_period_us;  /* Interface frame period */
   int                        sync_with_others; /* Flag to indicate this interface with synchronize with others */
   int                        rollcall;         /* Roll call counter used for synchronization */
   int                        sync_err;         /* Synchronization error count */
   int                        running;          /* Current running status. */
   int                        enabled;          /* Interface enabled state */
   struct semaphore           resync;           /* Blocking semaphore used for synchronization */
   struct halaudio_if_ops     ops;              /* Interface operations */
};

/**
*  Audio codec list node
*/
struct halaudio_codec_node
{
   LLIST_NODE                 lnode;            /* List node - must be first */
   struct semaphore           mutex;            /* Protect codec items */
   char                       name[32];         /* Codec channel name */
   struct halaudio_codec_ops  ops;              /* Codec operations (copied from parent, which allows child to override inheritance) */
   struct halaudio_if_node    *parent;          /* Pointer to parent audio interface */
   unsigned int               chanidx;          /* Channel index within interface */
   HALAUDIO_CODEC             cid;              /* Corresponding HAL Audio codec id */
   unsigned long              locked_by_client; /* Holds client handle that locked this codec */
};

/**
*  Audio codec list head
*/
struct halaudio_codec_head
{
   struct semaphore           mutex;            /* Protect codec list */
   LLIST_NODE                 list;             /* Head of audio codec list */
   int                        total;            /* Total number of codec channels supported */
   unsigned int               codectag;         /* Codec tag used to map channels to HAL Audio codec ID's.
                                                   Starts at 0 and monotonically increases regardless
                                                   codec channels are ever removed. */
};

/**
*  Audio interface list head
*/
struct halaudio_if_head
{
   struct semaphore           mutex;            /* Protect interface list */
   LLIST_NODE                 list;             /* Head of interface list. Master interface at end of list */
   int                        total;            /* Total number of audio interfaces added */

   /* FIXME: still have to implement the functionality to make use of the back
    * interface for generating the wall clock
    */
   struct halaudio_if_node   *unsync_backup_ifp;/* Backup interface used to generate wall clock when no synchronize interface exists */
};

/**
*  Client state list node
*/
struct halaudio_client_node
{
   LLIST_NODE                 lnode;            /* List node - must be first */
   HALAUDIO_POWER_LEVEL       powerlevel;       /* Requested power level */
   int                        superuser;        /* Super user setting. */
   struct halaudio_rw_state   read;             /* Read state */
   struct halaudio_rw_state   write;            /* Write state */
};

/**
*  Client state list head
*/
struct halaudio_client_head
{
   struct semaphore           mutex;            /* Protect client list and list items */
   LLIST_NODE                 list;             /* Head of client list */
   int                        total;            /* Total number of clients */
};

/**
*  Structure used to manage the power level.
*/
struct halaudio_power
{
   HALAUDIO_POWER_LEVEL       level;            /* Power level */
   struct semaphore           mutex;            /* Mutex protection */
};

struct halaudio_csx_framesync
{
   CSX_FRAME_SYNC_FP frame_sync_fp;
   void     *data;
};

/* knllog traces used for debugging */
#define HALAUDIO_LOG_ENABLED        0
#if HALAUDIO_LOG_ENABLED
#define HALAUDIO_LOG                KNLLOG
#else
#define HALAUDIO_LOG(c...)
#endif

#define HALAUDIO_ENABLE_PRINTK      1
#if HALAUDIO_ENABLE_PRINTK
#define HALAUDIO_PRINTK( fmt, args... ) KNLLOG( KERN_INFO, fmt, ## args )
#else
#define HALAUDIO_PRINTK(c...)
#endif

/* ---- Private Variables ------------------------------------------------ */

/* The number of active super users */
static int                          gSuperUsers;

/* Global audio power level control */
static struct halaudio_power        gPower;

/* Interface list head */
static struct halaudio_if_head      gInterfaces;

/* Codec list head */
static struct halaudio_codec_head   gCodecs;

/* Client list head */
static struct halaudio_client_head  gClients;

/* Resync interface enable and disable */
static struct halaudio_if_node     *gResyncEnable;
static struct halaudio_if_node     *gResyncDisable;

/* CSX frame sync callback */
static struct halaudio_csx_framesync gCsxFrameSync;

/* ---- Private Function Prototypes -------------------------------------- */
static HALAUDIO_HDL  halAudioCoreAllocateClient( void );
static int           halAudioCoreFreeClient( HALAUDIO_HDL client_hdl );
static int           halAudioCoreQueryInterfaceByName( HALAUDIO_HDL client_hdl, const char *name, HALAUDIO_IF *id );
static int           halAudioCoreQueryCodecByName( HALAUDIO_HDL client_hdl, const char *name, HALAUDIO_CODEC *id );
static int           halAudioCoreLockCodec( HALAUDIO_HDL client_hdl, HALAUDIO_CODEC cid );
static int           halAudioCoreUnlockCodec( HALAUDIO_HDL client_hdl, HALAUDIO_CODEC cid );
static int           halAudioCoreGetGainInfo( HALAUDIO_HDL client_hdl, HALAUDIO_BLOCK block, HALAUDIO_GAIN_INFO *info );
static int           halAudioCoreSetGain( HALAUDIO_HDL client_hdl, HALAUDIO_BLOCK block, int db );
static int           halAudioCoreSetPower( HALAUDIO_HDL client_hdl, HALAUDIO_POWER_LEVEL level );
static int           halAudioCoreGetPower( HALAUDIO_HDL client_hdl, HALAUDIO_POWER_LEVEL *level );
static int           halAudioCoreWrite( HALAUDIO_HDL client_hdl, HALAUDIO_CODEC cid, HALAUDIO_FMT format, const uint8_t *audio, int bytes );
static int           halAudioCoreAddInterface( HALAUDIO_IF_OPS *ops, unsigned int codecs, const char *name, int frame_usec, int sync, HALAUDIO_IF_HDL *hdlp );
static int           halAudioCoreDelInterface( HALAUDIO_IF_HDL handle );
static int           halAudioCoreSetSyncFlag( HALAUDIO_IF_HDL handle, int sync );
static int           halAudioCoreSetCsxIoPoints( HALAUDIO_CODEC cid, HALAUDIO_CSX_POINT_ID point, const CSX_IO_POINT_FNCS *fncp, void *data );
static int           halAudioCoreSetCsxFrameSync( CSX_FRAME_SYNC_FP fncp, void *data );

static int     halaudio_add_codec( HALAUDIO_IF_HDL ifhdl );
static void    halaudio_frame_elapsed( void *data );
static int     halaudio_del_interface_unsafe( HALAUDIO_IF_HDL handle );
static int     halaudio_del_codec_unsafe( struct halaudio_codec_node *codec, struct halaudio_if_node *interfp );
static int     halaudio_unlock_codec_unsafe( HALAUDIO_HDL client_hdl, struct halaudio_codec_node *codecp );
static void    halaudio_io_cb( int bytes, void *data );
static int     halaudio_find_others_synced( const struct halaudio_if_node *interfp );
static int     halaudio_resync_interface( struct halaudio_if_node *interfp );
static int     halaudio_disable_if_while_others_running( struct halaudio_if_node *interfp );
static int     halaudio_enable_interface( struct halaudio_if_node *interfp, int enable );
static int     halaudio_enable_interface_unsafe( struct halaudio_if_node *interfp, int enable );

static int     halaudio_pm_suspend( void );
static int     halaudio_pm_resume( void );
static int     halaudio_set_power_unsafe( HALAUDIO_POWER_LEVEL final_level, HALAUDIO_POWER_LEVEL prev_level );
static int     halaudio_find_backup_interface_unsafe( struct halaudio_if_node **ifp );


/* ---- Functions -------------------------------------------------------- */

/***************************************************************************/
/**
*  Map a codec ID to a codec channel.
*
*  @return
*     valid pointer - success
*     NULL          - error
*/
static inline struct halaudio_codec_node *cidToCodec(
   HALAUDIO_CODEC cid               /*<< (i) Codec ID */
)
{
   struct halaudio_codec_node *codecp, *foundp;

   foundp = NULL;

   down( &gCodecs.mutex );

   llist_foreach_item( codecp, &gCodecs.list, lnode )
   {
      if ( cid == codecp->cid )
      {
         foundp = codecp;
         break;
      }
   }
   up( &gCodecs.mutex );

   return foundp;
}

/***************************************************************************/
/**
*  Maps a block handle to a codec channel
*
*  @return
*     valid pointer - success
*     NULL          - error
*/
static inline struct halaudio_codec_node *blockToCodec(
   HALAUDIO_BLOCK block             /*<< (i) Block ID */
)
{
   return cidToCodec( HALAUDIO_BLOCK_GET_CODEC( block ));
}

/***************************************************************************/
/**
*  Maps an interface ID to an interface node
*
*  @return
*     valid pointer - success
*     NULL          - error
*/
static inline struct halaudio_if_node *getInterface(
   HALAUDIO_IF id                   /*<< (i) Interface ID */
)
{
   struct halaudio_if_node *interfp, *foundp;

   foundp = NULL;

   down( &gInterfaces.mutex );

   llist_foreach_item( interfp, &gInterfaces.list, lnode )
   {
      if ( id == interfp->id )
      {
         foundp = interfp;
         break;
      }
   }
   up( &gInterfaces.mutex );

   return foundp;
}

/***************************************************************************/
/**
*  Map client handle to client state memory
*
*  @return  Pointer to client state memory
*/
static inline struct halaudio_client_node *halaudio_get_client_state(
   HALAUDIO_HDL client_hdl          /*<< (i) client handle */
)
{
   return client_hdl;
}

/***************************************************************************/
/**
*  Map client state memory to client handle
*
*  @return  Pointer to client state memory
*/
static inline HALAUDIO_HDL halaudio_get_client_hdl(
   struct halaudio_client_node *client /*<< (i) client handle */
)
{
   return (HALAUDIO_HDL)client;
}

/***************************************************************************/
/**
*  Check whether client has control of a codec channel.
*
*  A client has control of the channel when she has previously locked
*  the channel, she is a superuser, or this codec channel is unlocked.
*  When a channel is unlocked. Anyone is free to modify an unlocked
*  channel's properties or lock it.
*
*  @return
*     0        Does not have control
*     1        Has control
*/
static inline int halaudio_has_control(
   HALAUDIO_HDL                client_hdl,   /*<< (i) Client handle */
   struct halaudio_codec_node *codecp        /*<< (i) Ptr to codec channel */
)
{
   struct halaudio_client_node *clientp;
   unsigned long                lock_owner;

   clientp     = halaudio_get_client_state( client_hdl );
   lock_owner  = codecp->locked_by_client;

   return ( clientp->superuser || lock_owner == (long)client_hdl || !lock_owner );
}

/***************************************************************************/
/**
*  Helper to cleanup client state
*
*  @return  Nothing
*/
static void halaudio_cleanup_clientp(
   struct halaudio_client_node *clientp      /**< (i) client to cleanup */
)
{
   kfree( clientp );
}

/***************************************************************************/
/**
*  Allocate client handle to make use of HAL Audio
*
*  @return
*     valid pointer - success
*     NULL          - error, out of memory
*/
HALAUDIO_HDL halAudioCoreAllocateClient( void )
{
   struct halaudio_client_node *clientp;

   clientp = kmalloc( sizeof(*clientp), GFP_KERNEL );
   if ( !clientp )
   {
      return NULL;
   }

   memset( clientp, 0, sizeof(*clientp) );

   clientp->powerlevel   = HALAUDIO_POWER_DEEP_SLEEP;
   clientp->superuser    = 0;
   clientp->write.format = HALAUDIO_FMT_S16_LE;  /* Default PC PCM format */

   sema_init( &clientp->read.sem, 0 );
   sema_init( &clientp->write.sem, 0 );

   down( &gClients.mutex );

   llist_add_tail( &clientp->lnode, &gClients.list );
   gClients.total++;

   up( &gClients.mutex );

   return clientp;
}

/***************************************************************************/
/**
*  Free client handle
*
*  @return
*     0        Success
*     -ve      Failure code
*/
int halAudioCoreFreeClient(
   HALAUDIO_HDL client_hdl          /*<< (i) Client handle */
)
{
   struct halaudio_client_node  *clientp;
   struct halaudio_codec_node   *codecp;

   clientp = halaudio_get_client_state( client_hdl );

   /* If we were a super user, decrement the super user counter. */
   if ( clientp->superuser )
   {
      clientp->superuser = 0;
      gSuperUsers--;
   }

   /* Scan all codecs and unlock ones owned by this client. */
   down( &gCodecs.mutex );

   llist_foreach_item( codecp, &gCodecs.list, lnode )
   {
      halaudio_unlock_codec_unsafe( client_hdl, codecp );
   }

   up( &gCodecs.mutex );

   /* Clear client's power request in case we were influencing
    * the power setting
    */
   halAudioCoreSetPower( client_hdl, HALAUDIO_POWER_DEEP_SLEEP );

   /* Free client */
   down( &gClients.mutex );

   llist_del( &clientp->lnode, &gClients.list );
   halaudio_cleanup_clientp( clientp );
   gClients.total--;

   up( &gClients.mutex );

   return 0;
}

/***************************************************************************/
/**
*  Otbain audio interface ID by name string
*
*  @return
*     0        Success
*     -ve      Failure code
*/
int halAudioCoreQueryInterfaceByName(
   HALAUDIO_HDL client_hdl,         /*<< (i) Client handle */
   const char  *name,               /*<< (i) Name of interface */
   HALAUDIO_IF *id                  /*<< (i) Ptr to store found interface ID */
)
{
   struct halaudio_if_node *interfp;
   int                      err;

   down( &gInterfaces.mutex );

   err = -EINVAL;
   llist_foreach_item( interfp, &gInterfaces.list, lnode )
   {
      if ( strncmp( name, interfp->name, sizeof(interfp->name) ) == 0 )
      {
         /* Match found */
         *id = interfp->id;
         err = 0;
         break;
      }
   }

   up( &gInterfaces.mutex );

   return err;
}

/***************************************************************************/
/**
*  Otbain audio codec channel ID by name string
*
*  @return
*     0        Success
*     -ve      Failure code
*/
int halAudioCoreQueryCodecByName(
   HALAUDIO_HDL    client_hdl,      /*<< (i) Client handle */
   const char     *name,            /*<< (i) Name of codec channel */
   HALAUDIO_CODEC *id               /*<< (i) Ptr to store found codec ID */
)
{
   struct halaudio_codec_node *codecp;
   int                         err;

   down( &gCodecs.mutex );

   err = -EINVAL;
   llist_foreach_item( codecp, &gCodecs.list, lnode )
   {
      if ( strncmp( name, codecp->name, sizeof(codecp->name) ) == 0 )
      {
         /* Match found */
         *id = codecp->cid;
         err = 0;
         break;
      }
   }

   up( &gCodecs.mutex );

   return err;
}

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
int halAudioCoreLockCodec(
   HALAUDIO_HDL    client_hdl,      /*<< (i) Client handle */
   HALAUDIO_CODEC  cid              /*<< (i) Codec to lock */
)
{
   struct halaudio_codec_node *codecp;
   HALAUDIO_HDL               tmphdl;
   int                        err;

   codecp = cidToCodec( cid );
   if ( codecp == NULL )
   {
      return -EINVAL;
   }

   down( &codecp->mutex );

   err = 0;

   /* Client can call lock safely multiple times on the same codec. */
   tmphdl = (HALAUDIO_HDL)codecp->locked_by_client;
   if ( tmphdl && tmphdl != client_hdl )
   {
      /* Codec already locked by another client */
      err = -EBUSY;
      goto backout;
   }

   codecp->locked_by_client = (unsigned long)client_hdl;

backout:
   up( &codecp->mutex );
   return err;
}

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
int halAudioCoreUnlockCodec(
   HALAUDIO_HDL    client_hdl,      /*<< (i) Client handle */
   HALAUDIO_CODEC  cid              /*<< (i) Codec ID to unlock */
)
{
   struct halaudio_codec_node *codecp;
   int                         err;

   codecp = cidToCodec( cid );
   if ( codecp == NULL )
   {
      return -EINVAL;
   }

   down( &codecp->mutex );

   err = halaudio_unlock_codec_unsafe( client_hdl, codecp );

   up( &codecp->mutex );

   return err;
}

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
int halAudioCoreGetGainInfo(
   HALAUDIO_HDL        client_hdl,  /*<< (i) Client handle */
   HALAUDIO_BLOCK      block,       /*<< (i) Block id for digital or analog gain */
   HALAUDIO_GAIN_INFO *info         /*<< (o) Pointer to returned gain information */
)
{
   HALAUDIO_CODEC_OPS         *ops;
   HALAUDIO_DIR                dir;
   HALAUDIO_HWSEL              sel;
   struct halaudio_codec_node *codecp;
   int                         ch, err;

   codecp = blockToCodec( block );
   if ( codecp == NULL )
   {
      return -EINVAL;
   }

   down( &codecp->mutex );

   /* Can only proceed when parent interface is enabled */
   if ( !codecp->parent->enabled )
   {
      err = -EPERM;
      goto backout;
   }

   ops = &codecp->ops;
   dir = HALAUDIO_BLOCK_GET_DIR( block );
   sel = HALAUDIO_BLOCK_GET_HWSEL( block );
   ch  = codecp->chanidx;
   err = -EPERM;

   if ( HALAUDIO_BLOCK_IS_DIGITAL( block ))
   {
      if ( ops->getdig )
      {
         err = ops->getdig( ch, info, dir );
      }
   }
   else if ( HALAUDIO_BLOCK_IS_SIDETONE( block ))
   {
      if ( ops->getsidetone )
      {
         err = ops->getsidetone( ch, info );
      }
   }
   else  /* Must be an analog block */
   {
      if ( ops->getana )
      {
         err = ops->getana( ch, info, dir, sel );
      }
   }

backout:
   up( &codecp->mutex );
   return err;
}

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
int halAudioCoreSetGain(
   HALAUDIO_HDL    client_hdl,      /*<< (i) Client handle */
   HALAUDIO_BLOCK  block,           /*<< (i) Block id for digital or analog gain */
   int             db               /*<< (i) Gain in dB to set */
)
{
   HALAUDIO_CODEC_OPS           *ops;
   HALAUDIO_DIR                  dir;
   HALAUDIO_HWSEL                sel;
   int                           ch, err;
   struct halaudio_codec_node   *codecp;

   codecp = blockToCodec( block );
   if ( codecp == NULL )
   {
      return -EINVAL;
   }

   down( &codecp->mutex );

   if ( halaudio_has_control( client_hdl, codecp ) == 0 )
   {
      err = -EBUSY;
      goto backout;
   }

   /* Can only proceed when parent interface is enabled */
   if ( !codecp->parent->enabled )
   {
      err = -EPERM;
      goto backout;
   }

   ops = &codecp->ops;
   dir = HALAUDIO_BLOCK_GET_DIR( block );
   sel = HALAUDIO_BLOCK_GET_HWSEL( block );
   ch  = codecp->chanidx;
   err = -EPERM;

   if ( HALAUDIO_BLOCK_IS_DIGITAL( block ))
   {
      if ( ops->setdig )
      {
         err = ops->setdig( ch, db, dir );
      }
   }
   else if ( HALAUDIO_BLOCK_IS_SIDETONE( block ))
   {
      if ( ops->setsidetone )
      {
         err = ops->setsidetone( ch, db );
      }
   }
   else  /* Must be an analog block */
   {
      if ( ops->setana )
      {
         err = ops->setana( ch, db, dir, sel );
      }
   }

backout:
   up( &codecp->mutex );
   return err;
}

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
int halAudioCoreSetPower(
   HALAUDIO_HDL         client_hdl, /*<< (i) Client handle */
   HALAUDIO_POWER_LEVEL level       /*<< (i) Power level request */
)
{
   struct halaudio_client_node  *clientp, *tmpclientp;
   HALAUDIO_POWER_LEVEL          final_level, prev_level;
   int rc;

   down( &gPower.mutex );

   clientp = halaudio_get_client_state( client_hdl );

   clientp->powerlevel  = level;
   final_level          = HALAUDIO_POWER_DEEP_SLEEP;

   if ( clientp->superuser )
   {
      /* Superusers can override consensus power level */
      final_level = level;
   }
   else
   {
      /* Look over all clients' power settings to determine what we should
       * do for the final setting.
       */
      down( &gClients.mutex );

      llist_foreach_item( tmpclientp, &gClients.list, lnode )
      {
         if ( tmpclientp->powerlevel > final_level )
         {
            final_level = tmpclientp->powerlevel;
         }
      }

      up( &gClients.mutex );
   }

   rc          = 0;
   prev_level  = gPower.level;

   /* There is a change in the power level */
   if ( final_level != prev_level )
   {
      rc = halaudio_set_power_unsafe( final_level, prev_level );
      if ( rc == 0 )
      {
         /* Update power level */
         gPower.level = final_level;
      }
   }

   up( &gPower.mutex );
   return rc;
}

/***************************************************************************/
/**
*  Query actual power level.
*
*  @return
*     0        Success
*     -ve      Error code
*/
int halAudioCoreGetPower(
   HALAUDIO_HDL         client_hdl, /*<< (i) Client handle */
   HALAUDIO_POWER_LEVEL *level      /**< (o) Pointer to store power level */
)
{
   down( &gPower.mutex );

   *level = gPower.level;

   up( &gPower.mutex );

   return 0;
}

/***************************************************************************/
/**
*  Write audio samples directly to an audio channel's hardware buffers.
*
*  @return
*     0        Success
*     -EBUSY   Codec is already locked by other client, try again later.
*     -EPERM   Parent interface is disabled
*     -ve      Error code
*/
int halAudioCoreWrite(
   HALAUDIO_HDL   client_hdl,       /*<< (i) Client handle */
   HALAUDIO_CODEC cid,              /*<< (i) Codec channel to write to */
   HALAUDIO_FMT   format,           /*<< (i) Format of samples */
   const uint8_t *audio,            /*<< (i) Ptr to data buffer to playback */
   int            bytes             /*<< (i) Number of bytes contained in buffer */
)
{
   struct halaudio_codec_node   *codecp;
   HALAUDIO_FMT                  expected_fmt;

   codecp = cidToCodec( cid );
   if ( codecp == NULL )
   {
      return -EINVAL;
   }

   /* Only the controlling client can operate on this codec channel */
   if ( halaudio_has_control( client_hdl, codecp ) == 0 )
   {
      return -EBUSY;
   }

   /* Can only proceed when parent interface is enabled */
   if ( !codecp->parent->enabled )
   {
      return -EPERM;
   }

#if defined ( __LITTLE_ENDIAN )
   expected_fmt = HALAUDIO_FMT_S16_LE;
#else
   expected_fmt = HALAUDIO_FMT_S16_BE;
#endif

   /* Sanity check */
   if ( format != expected_fmt )
   {
      return -EINVAL;
   }

   if ( codecp->ops.write )
   {
      struct halaudio_client_node  *clientp;
      int                           rc;

      clientp = halaudio_get_client_state( client_hdl );

      rc = codecp->ops.write( codecp->chanidx, bytes, audio,
            halaudio_io_cb, &clientp->write );
      if ( rc < 0 )
      {
         /* There is a problem with writing */
         return rc;
      }

      /* The write call is non-blocking so we need to use our semaphore to block
       * until the write operation is done.
       */
      down( &clientp->write.sem );

      return clientp->write.bytes;
   }

   return -EINVAL;
}

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
int halAudioCoreRead(
   HALAUDIO_HDL   client_hdl,       /*<< (i) Client handle */
   HALAUDIO_CODEC cid,              /*<< (i) Codec channel to write to */
   HALAUDIO_FMT   format,           /*<< (i) Format of samples */
   uint8_t       *audio,            /*<< (o) Ptr to data buffer to record to */
   int            bytes             /*<< (i) Max number of bytes to read */
)
{
   struct halaudio_codec_node   *codecp;
   HALAUDIO_FMT                  expected_fmt;

   codecp = cidToCodec( cid );
   if ( codecp == NULL )
   {
      return -EINVAL;
   }

   /* Only the controlling client can operate on this codec channel */
   if ( halaudio_has_control( client_hdl, codecp ) == 0 )
   {
      return -EBUSY;
   }

   /* Can only proceed when parent interface is enabled */
   if ( !codecp->parent->enabled )
   {
      return -EPERM;
   }

#if defined (__LITTLE_ENDIAN)
   expected_fmt = HALAUDIO_FMT_S16_LE;
#else
   expected_fmt = HALAUDIO_FMT_S16_BE;
#endif

   /* Sanity check */
   if ( format != expected_fmt )
   {
      return -EINVAL;
   }

   if ( codecp->ops.read )
   {
      struct halaudio_client_node  *clientp;
      int                           rc;

      clientp = halaudio_get_client_state( client_hdl );

      rc = codecp->ops.read( codecp->chanidx, bytes, audio,
            halaudio_io_cb, &clientp->read );
      if ( rc < 0 )
      {
         /* There is a problem with reading */
         return rc;
      }

      /* The read call is non-blocking so we need to use our semaphore to block
       * until the read operation is done.
       */
      down( &clientp->read.sem );

      return clientp->read.bytes;
   }

   return -EINVAL;
}

/***************************************************************************/
/**
*  Enable and disable super user privileges.
*
*  @return
*     0        Success
*     -1       Super user privileges not permitted
*/
int halAudioCoreSetSuperUser(
   HALAUDIO_HDL   client_hdl,       /*<< (i) Client handle */
   int            enable            /*<< (i) 1 to enable super user, 0 to disable */
)
{
   struct halaudio_client_node *client;

   client = halaudio_get_client_state( client_hdl );

   if ( !enable && client->superuser )
   {
      client->superuser = 0;
      gSuperUsers--;
   }
   else if ( enable && client->superuser == 0 )
   {
      client->superuser = 1;
      gSuperUsers++;
   }

   return 0;
}

/***************************************************************************/
/**
*  Set sampling frequency for an audio codec channel.
*
*  @return
*     0        Success
*     -EBUSY   Codec is already locked by other client, try again later.
*     -EINVAL  Invalid sampling frequency to set, or invalid codec
*     -EPERM   Parent interface is disabled
*
*  @remarks
*     Changing the sampling frequency while a codec is running requires
*     the codec be disabled temporarily while the hardware parameters
*     are reconfigured. As expected, the affected codec will produce audible
*     glitches. It is recommended that the the analog gains be turned down
*     prior to switching the sampling frequency. Re-enabling the codec
*     involves resynchronizing with the other codec channels.
*/
int halAudioCoreSetFreq(
   HALAUDIO_HDL   client_hdl,       /*<< (i) Client handle */
   HALAUDIO_CODEC cid,              /*<< (i) Codec channel to write to */
   int            newfreq           /*<< (i) Sampling frequency in Hz to set */
)
{
   struct halaudio_codec_node *codecp;
   int                         err;
   int                         currfreq;
   HALAUDIO_CODEC_OPS         *ops;

   codecp = cidToCodec( cid );
   if ( codecp == NULL )
   {
      return -EINVAL;
   }

   down( &codecp->mutex );

   /* Only the controlling client can operate on this codec channel */
   if ( halaudio_has_control( client_hdl, codecp ) == 0 )
   {
      err = -EBUSY;
      goto backout;
   }

   /* Can only proceed when parent interface is enabled */
   if ( !codecp->parent->enabled )
   {
      err = -EPERM;
      goto backout;
   }

   ops      = &codecp->ops;
   err      = -EPERM;
   currfreq = 0;

   /* More error checks */
   if ( !ops->getfreq || !ops->setfreq )
   {
      err = -EPERM;
      goto backout;
   }

   /* Get current sampling frequency */
   err = ops->getfreq( codecp->chanidx, &currfreq );
   if ( err )
   {
      goto backout;
   }

   if ( currfreq != newfreq )
   {
      struct halaudio_if_node *interfp;

      interfp = getInterface( codecp->parent->id );
      if ( interfp == NULL )
      {
         err = -EINVAL;
         goto backout;
      }

      /* Grab control of individual interface and power mutexes */
      down( &interfp->mutex );
      down( &gPower.mutex );

      /* Disable interface */
      err = halaudio_enable_interface_unsafe( interfp, 0 /* disable */ );
      if ( err )
      {
         goto release_interf_and_power;
      }

      /* Configure new frequency */
      err = ops->setfreq( codecp->chanidx, newfreq );

      /* Re-enable interface, even if previous setfreq failed */
      err |= halaudio_enable_interface_unsafe( interfp, 1 /* enable */ );

release_interf_and_power:
      up( &gPower.mutex );
      up( &interfp->mutex );
   }

backout:
   up( &codecp->mutex );
   return err;
}

/***************************************************************************/
/**
*  Query sampling frequency for an audio codec channel
*
*  @return
*     0        Success
*     -EINVAL  Invalid sampling frequency to set, or invalid codec
*     -EPERM   Parent interface is disabled
*/
int halAudioCoreGetFreq(
   HALAUDIO_HDL   client_hdl,       /*<< (i) Client handle */
   HALAUDIO_CODEC cid,              /*<< (i) Codec channel to write to */
   int           *freqhz            /*<< (o) Ptr to store sampling freq */
)
{
   struct halaudio_codec_node *codecp;
   int                         err;

   codecp = cidToCodec( cid );
   if ( codecp == NULL )
   {
      return -EINVAL;
   }

   down( &codecp->mutex );

   /* Can only proceed when parent interface is enabled */
   if ( !codecp->parent->enabled )
   {
      err = -EPERM;
      goto backout;
   }

   err = -EPERM;
   if ( codecp->ops.getfreq )
   {
      err = codecp->ops.getfreq( codecp->chanidx, freqhz );
   }

backout:
   up( &codecp->mutex );
   return err;
}

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
int halAudioCoreSetEquParms(
   HALAUDIO_HDL         client_hdl, /*<< (i) Client handle */
   HALAUDIO_CODEC       cid,        /*<< (i) Codec to write to */
   HALAUDIO_DIR         dir,        /*<< (i) Select equalizer direction */
   const HALAUDIO_EQU  *equ         /*<< (i) Pointer to equalizer parameters */
)
{
   struct halaudio_codec_node *codecp;
   int                         err;

   codecp = cidToCodec( cid );
   if ( codecp == NULL )
   {
      return -EINVAL;
   }

   down( &codecp->mutex );

   /* Can only proceed when parent interface is enabled */
   if ( !codecp->parent->enabled )
   {
      err = -EPERM;
      goto backout;
   }

   /* Only the controlling client can operate on this codec channel */
   if ( halaudio_has_control( client_hdl, codecp ) == 0 )
   {
      err = -EBUSY;
      goto backout;
   }

   err = -EPERM;
   if ( codecp->ops.setequ )
   {
      err = codecp->ops.setequ( codecp->chanidx, dir, equ );
   }

backout:
   up( &codecp->mutex );
   return err;
}

/***************************************************************************/
/**
*  Query equalizer parameters for a particular audio codec channel
*
*  @return
*     0        Success
*     -EINVAL  Invalid parameters
*     -EPERM   Parent interface is disabled
*/
int halAudioCoreGetEquParms(
   HALAUDIO_HDL         client_hdl, /*<< (i) Client handle */
   HALAUDIO_CODEC       cid,        /*<< (i) Codec channel to write to */
   HALAUDIO_DIR         dir,        /*<< (i) Select equalizer direction */
   HALAUDIO_EQU        *equ         /*<< (o) Ptr to store equalizer parameters */
)
{
   struct halaudio_codec_node *codecp;
   int                         err;

   codecp = cidToCodec( cid );
   if ( codecp == NULL )
   {
      return -EINVAL;
   }

   down( &codecp->mutex );

   /* Can only proceed when parent interface is enabled */
   if ( !codecp->parent->enabled )
   {
      err = -EPERM;
      goto backout;
   }

   err = -EPERM;
   if ( codecp->ops.getequ )
   {
      err = codecp->ops.getequ( codecp->chanidx, dir, equ );
   }

backout:
   up( &codecp->mutex );
   return err;
}

/***************************************************************************/
/**
*  Query high-level hardware information. To get detailed sub-component
*  information call halAudioCoreGetInterfaceInfo, halAudioCoreGetCodecInfo and
*  halAudioCoreGetGainInfo.
*
*  @return
*     0        Success
*     -EINVAL  Invalid parameters
*/
int halAudioCoreGetHardwareInfo(
   HALAUDIO_HDL       client_hdl,   /*<< (i) Client handle */
   HALAUDIO_HW_INFO  *info          /*<< (o) Ptr to store hardware info */
)
{
   memset( info, 0, sizeof(*info) );
   info->interfaces      = gInterfaces.total;
   info->codecs          = gCodecs.total;
   info->frame_period_us = HALAUDIO_DEFAULT_FRAME_PERIOD;
   return 0;
}

/***************************************************************************/
/**
*  Query audio interface information.
*
*  @return
*     0        Success
*     -EINVAL  Invalid parameters
*/
int halAudioCoreGetInterfaceInfo(
   HALAUDIO_HDL         client_hdl, /*<< (i) Client handle */
   HALAUDIO_IF          id,         /*<< (i) Interface id */
   HALAUDIO_IF_INFO    *info        /*<< (o) Ptr to store interface info */
)
{
   struct halaudio_if_node *interfp;

   interfp = getInterface( id );
   if ( interfp == NULL )
   {
      return -EINVAL;
   }

   memset( info, 0, sizeof(*info) );
   strncpy( info->name, interfp->name, sizeof(info->name) );
   info->codecs            = interfp->codecs;
   info->frame_period_us   = interfp->frame_period_us;
   info->enabled           = interfp->enabled;
   info->sync              = interfp->sync_with_others;

   return 0;
}

/***************************************************************************/
/**
*  Query audio codec information.
*
*  @return
*     0        Success
*     -EINVAL  Invalid parameters
*     -EPERM   Parent interface is disabled
*/
int halAudioCoreGetCodecInfo(
   HALAUDIO_HDL         client_hdl, /*<< (i) Client handle */
   HALAUDIO_CODEC       cid,        /*<< (i) Codec channel to write to */
   HALAUDIO_CODEC_INFO *info        /*<< (o) Ptr to store codec info */
)
{
   struct halaudio_codec_node *codecp;
   int                         err;

   codecp = cidToCodec( cid );
   if ( codecp == NULL )
   {
      return -EINVAL;
   }

#if 0
   if ( down( &codecp->mutex ))
   {
      return -ERESTARTSYS;
   }
#endif

   err = -EPERM;
   if ( codecp->ops.info )
   {
      err = codecp->ops.info( codecp->chanidx, info );
      if ( err == 0 )
      {
         /* Add parent interface information */
         info->parent_id = codecp->parent->id;
         info->locked    = codecp->locked_by_client;
      }
   }

#if 0
   up( &codecp->mutex );
#endif

   return err;
}

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
int halAudioCoreEnableInterface(
   HALAUDIO_HDL         client_hdl, /*<< (i) Client handle */
   HALAUDIO_IF          id,         /*<< (i) Interface number */
   int                  enable      /*<< (i) 1 to enable, 0 to disable */
)
{
   struct halaudio_if_node *interfp;

   interfp = getInterface( id );
   if ( interfp == NULL )
   {
      return -EINVAL;
   }

   return halaudio_enable_interface( interfp, enable );
}

/***************************************************************************/
/**
*  Add an HAL Audio Interface
*
*  @return
*     handle > 0     On success
*     NULL           On general failure
*
*  @remarks
*     Interfaces by default will be synchronized by HAL Audio based on
*     their interrupt periodicity. However, some interfaces may choose
*     not to synchronize with other interfaces because of timing
*     limitations or other reasons. To opt out of synchronization, set
*     the frame period to 0.
*/
int halAudioCoreAddInterface(
   HALAUDIO_IF_OPS  *ops,           /*<< (i) Interface operations */
   unsigned int      codecs,        /*<< (i) Total number of audio codec channels */
   const char       *name,          /*<< (i) Name string */
   int               frame_usec,    /*<< (i) Interrupt period in usec */
   int               sync,          /*<< (i) Requests to synchronize with other interfaces */
   HALAUDIO_IF_HDL  *hdlp           /*<< (o) Ptr to store handle */
)
{
   struct halaudio_if_node    *interfp, *tmpinterfp;
   LLIST_NODE                 *prevnodep;
   int                         i, err;
   HALAUDIO_IF                 id;
   char                        strname[40];

   /* New interface must conform with HAL Audio core timing. If
    * an interface cannot be synchronized with other interfaces, the frame
    * size should be set to 0.  In the future, interfaces operating
    * at multiple HAL Audio core frame periods may be supported.
    */
   if ( sync && frame_usec != HALAUDIO_DEFAULT_FRAME_PERIOD  )
   {
      HALAUDIO_PRINTK( "Error cannot sychronize an interface with a frame period of %i usec. Expect %i usec.",
            frame_usec, HALAUDIO_DEFAULT_FRAME_PERIOD );
      return -EINVAL;
   }

   /* Check that mandatory operations are provided */
   if ( !ops->prepare || !ops->enable || !ops->disable )
   {
      return -EPERM;
   }

   interfp = kmalloc( sizeof(*interfp), GFP_KERNEL );
   if ( !interfp )
   {
      return -ENOMEM;
   }

   if ( codecs == 0 )
   {
      return -EINVAL;
   }

   /* Initialize interface parameters */
   memset( interfp, 0, sizeof(*interfp) );

   strncpy( strname, name, sizeof(interfp->name) );
   sema_init( &interfp->mutex, 1 );

   strncpy( strname, name, sizeof(interfp->name) );
   strncat( strname, " resync", sizeof(interfp->name) - strlen( strname ) - 1 );
   sema_init( &interfp->resync, 0 );

   strncpy( interfp->name, name, sizeof(interfp->name) );
   interfp->frame_period_us   = frame_usec;
   interfp->sync_with_others  = sync;
   memcpy( &interfp->ops, ops, sizeof(interfp->ops) );
   interfp->enabled = HALAUDIO_INTERFACE_DISABLED;

   /* Add codec channels */
   for ( i = 0; i < codecs; i++ )
   {
      err = halaudio_add_codec( interfp );
      if ( err )
      {
         goto exit_cleanup_memory;
      }
   }

   /* Initialize interface. */
   if ( ops->init )
   {
      err = ops->init( halaudio_frame_elapsed, interfp );
      if ( err )
      {
         goto exit_cleanup_memory;
      }
   }

   /* Add interface */
   down( &gInterfaces.mutex );

   /* Map interface id to what is available. The interface list
    * is sorted by interface id. Start with 0 and continue
    * incrementing until an unused id is found. The intent is
    * to reuse relinquished ids when interfaces are removed
    * dynamically.
    */
   id          = 0;
   prevnodep   = &gInterfaces.list;
   llist_foreach_item( tmpinterfp, &gInterfaces.list, lnode )
   {
      if ( tmpinterfp->id == id )
      {
         prevnodep = &tmpinterfp->lnode;
         id++;
         continue;
      }
      break;
   }
   interfp->id = id;

   /* Insert interface in sorted order */
   llist_add( &interfp->lnode, prevnodep );

   gInterfaces.total++;

   up( &gInterfaces.mutex );

   /* Enable interface */
   err = halaudio_enable_interface( interfp, 1 /* enable */ );
   if ( err )
   {
      goto exit_cleanup_memory;
   }

   /* Success, return interface pointer as handle */
   *hdlp = interfp;

   return 0;

exit_cleanup_memory:

   halAudioCoreDelInterface( interfp );
   return err;
}

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
*
*  @remarks    This routine is very similar to the driver exit routine.
*/
int halAudioCoreDelInterface(
   HALAUDIO_IF_HDL handle           /*<< (i) interface to delete */
)
{
   int err;

   /* Disable interface */
   err = halaudio_enable_interface( handle, 0 /* disable */ );
   if ( err )
   {
      return err;
   }

   /* Free interface */
   down( &gInterfaces.mutex );

   err = halaudio_del_interface_unsafe( handle );

   up( &gInterfaces.mutex );

   return err;
}

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
int halAudioCoreSetCsxIoPoints(
   HALAUDIO_CODEC           cid,    /*<< (i) Codec to install CSX callbacks */
   HALAUDIO_CSX_POINT_ID    point,  /*<< (i) Point ID for CSX callbacks */
   const CSX_IO_POINT_FNCS *fncp,   /*<< (i) List of callbacks */
   void                    *data    /*<< (i) User data */
)
{
   struct halaudio_codec_node *codecp;
   int                         err;

   codecp = cidToCodec( cid );
   if ( codecp == NULL )
   {
      return -EINVAL;
   }

   down( &codecp->mutex );

   /* Can only proceed when parent interface is enabled */
   if ( !codecp->parent->enabled )
   {
      err = -EPERM;
      goto backout;
   }

   err = -EPERM;
   if ( codecp->ops.setcsx )
   {
      err = codecp->ops.setcsx( codecp->chanidx, point, fncp, data );
   }

backout:
   up( &codecp->mutex );
   return err;
}

/***************************************************************************/
/**
*  Set CSX frame sync callback for debug infrastructure to support alignment
*  of audio inject and capture operations.
*
*  @return
*     0        Success
*     -ve      Other error codes
*/
int halAudioCoreSetCsxFrameSync(
   CSX_FRAME_SYNC_FP fncp,    /**< (i) Frame sync callback */
   void              *data    /**< (i) User data */
)
{
   if ( fncp != NULL )
   {
      gCsxFrameSync.frame_sync_fp = fncp;
      gCsxFrameSync.data = data;
   }
   else
   {
      gCsxFrameSync.frame_sync_fp = NULL;
      gCsxFrameSync.data = NULL;
   }

   return 0;
}

/***************************************************************************/
/**
*  Changes an interface's synchronization behavior. Synchronization refers
*  to timing with other interfaces. All synchronized interfaces are expected
*  to start at the same time and execute at similar sampling frequencies.
*  Unsynchronized interfaces are free to execute at any sampling frequencies
*  however the their clocks do not contribute to the HAL Audio wall clock.
*
*  @return
*     0        on success
*     -1       Failed, may have orphan codec channels
*     -ve      Other error codes
*
*  @remarks
*/
int halAudioCoreSetSyncFlag(
   HALAUDIO_IF_HDL   handle,        /*<< (i) interface to delete */
   int               sync           /*<< (i) 1 requests to sync with other interfaces */
)
{
   struct halaudio_if_node *interfp = handle;

   if ( sync && interfp->frame_period_us != HALAUDIO_DEFAULT_FRAME_PERIOD )
   {
      HALAUDIO_PRINTK( "Error: cannot sychronize an interface with a frame period of %i usec. Expect %i usec.",
            interfp->frame_period_us, HALAUDIO_DEFAULT_FRAME_PERIOD );
      return -EPERM;
   }

   interfp->sync_with_others = sync;

   return 0;
}

/***************************************************************************/
/**
*  Helper routine to install backup unsynchronized interface for generating
*  the wall when there no synchronized interfaces or they are all disabled.
*
*  @return
*     0        on success
*     -1       Failed, may have orphan codec channels
*     -ve      Other error codes
*
*  @remarks
*     This routine is not thread safe. It is the responsibility of
*     the caller to protect the interface list.
*/
static int halaudio_find_backup_interface_unsafe(
   struct halaudio_if_node **ifpp
)
{
   struct halaudio_if_node *tmpifp;

   /* Find replacement backup */
   llist_foreach_item( tmpifp, &gInterfaces.list, lnode )
   {
      if ( tmpifp->sync_with_others == 0 
      &&   tmpifp->frame_period_us == HALAUDIO_DEFAULT_FRAME_PERIOD 
      &&   tmpifp->enabled )
      {
         *ifpp = tmpifp;
         return 0;
      }
   }

   /* No suitable backup found */
   *ifpp = NULL;
   return -1;
}

/***************************************************************************/
/**
*  Helper routine to delete an audio interface, but does not protect the
*  interface list.
*
*  @return
*     0        on success
*     -1       Failed, may have orphan codec channels
*     -ve      Other error codes
*
*  @remarks
*     This routine is not thread safe. It is the responsibility of
*     the caller to protect the interface list.
*
*     This routine operates on the codec list and it will protect the
*     codec list.
*/
static int halaudio_del_interface_unsafe(
   HALAUDIO_IF_HDL handle           /*<< (i) interface to delete */
)
{
   struct halaudio_if_node    *interfp;
   struct halaudio_if_ops     *ops;
   struct halaudio_codec_node *codecp, *tmpcodecp;
   int                         num_codecs;
   int                         error;

   interfp  = handle;
   ops      = &interfp->ops;
   error    = 0;

   /* Call exit() to terminate interface */
   if ( ops->exit )
   {
      error = ops->exit();
      if ( error )
      {
         HALAUDIO_PRINTK( "failed executing %s interface exit err=%i",
               interfp->name, error );
      }
   }

   /* Cleanup attached codec resources  */
   num_codecs = 0;
   down( &gCodecs.mutex );

   llist_foreach_item_safe( codecp, tmpcodecp, &gCodecs.list, lnode )
   {
      if ( halaudio_del_codec_unsafe( codecp, interfp ) == 0 )
      {
         num_codecs++;
      }
   }

   up( &gCodecs.mutex );

   if ( interfp->codecs != num_codecs )
   {
      HALAUDIO_PRINTK( "error may have orphan codec channels for %s interface. Cleaned %i but registered %i",
            interfp->name, num_codecs, interfp->codecs );
   }

   if ( llist_del( &interfp->lnode, &gInterfaces.list ))
   {
      gInterfaces.total--;
   }

   halaudio_find_backup_interface_unsafe( &gInterfaces.unsync_backup_ifp );

   kfree( interfp );

   return error;
}

/***************************************************************************/
/**
*  Add codec channel to an audio interface
*
*  @return
*     handle > 0     on success
*     NULL           on general failure
*     -EFAULT        failed to initialize interface
*/
static int halaudio_add_codec(
   HALAUDIO_IF_HDL ifhdl            /**< (i) interface handle */
)
{
   struct halaudio_if_node    *interfp;
   struct halaudio_codec_node *codecp, *tmpcodecp;
   LLIST_NODE                 *prevnodep;
   unsigned int                chanidx;
   HALAUDIO_CODEC              cid;
   int                         err;

   interfp = ifhdl;
   if ( !interfp )
   {
      return -EINVAL;
   }

   codecp = kmalloc( sizeof(*codecp), GFP_KERNEL );
   if ( !codecp )
   {
      return -ENOMEM;
   }

   memset( codecp, 0, sizeof(*codecp) );

   /* Make a copy of the codec ops */
   memcpy( &codecp->ops, &interfp->ops.codec_ops, sizeof(codecp->ops) );

   codecp->parent = interfp;
   chanidx        = interfp->codecs++;

   down( &gCodecs.mutex );

   /* Map codec id to what is available. The codec list
    * is sorted by codec id. Start with HALAUDIO_CODEC0 and
    * continue to increment until an unused id is found. The
    * intent is to reuse relinquished codec ids when interfaces
    * are removed dynamically.
    */
   cid         = HALAUDIO_CODEC0;
   prevnodep   = &gCodecs.list;
   llist_foreach_item( tmpcodecp, &gCodecs.list, lnode )
   {
      if ( tmpcodecp->cid == cid )
      {
         prevnodep = &tmpcodecp->lnode;
         cid++;
         continue;
      }
      break;
   }

   codecp->cid     = cid;
   codecp->chanidx = chanidx;

   /* Get name of codec channel */
   err = -EINVAL;
   if ( codecp->ops.info )
   {
      HALAUDIO_CODEC_INFO  info;
      err = codecp->ops.info( chanidx, &info );
      if ( !err )
      {
         strncpy( codecp->name, info.name, sizeof(codecp->name) );
      }
   }
   if ( err )
   {
      sprintf( codecp->name, "%s CH%i", interfp->name, cid );
   }

   sema_init( &codecp->mutex, 1 );

   /* Insert codec in sorted order */
   llist_add( &codecp->lnode, prevnodep );

   gCodecs.total++;

   up( &gCodecs.mutex );
   return err;
}

/***************************************************************************/
/**
*  Remove codec channel
*
*  @return
*     0           Success
*     -EINVAL     Failed to remove codec channel
*
*  @remark
*     This routine is not thread safe. It is the responsibility of the
*     caller to protect the codec list.
*/
static int halaudio_del_codec_unsafe(
   struct halaudio_codec_node *codecp,  /**< (i) codec channel to remove */
   struct halaudio_if_node    *interfp  /**< (i) Parent interface, if NULL no check provided */
)
{
   if ( codecp->parent == interfp || interfp == NULL )
   {
      if ( llist_del( &codecp->lnode, &gCodecs.list ))
      {
         gCodecs.total--;
      }
      kfree( codecp );
      return 0;
   }

   return -EINVAL;
}

/***************************************************************************/
/**
*  Release lock for an audio codec channel. Unprotected version.
*
*  @return
*     0           Success
*     -EINVAL     Failed to remove codec channel
*
*  @remark
*     This routine is not thread safe.  It is the responsibility of the
*     caller to protect the codec list.
*/
static int halaudio_unlock_codec_unsafe(
   HALAUDIO_HDL                client_hdl,   /*<< (i) Client handle */
   struct halaudio_codec_node *codecp        /**< (i) codec channel to remove */
)
{
   /* Can only unlock a channel that it owns. Super users cannot unlock
    * channels on a client's behalf.
    */
   if ( codecp->locked_by_client != (unsigned long)client_hdl )
   {
      return -EINVAL;
   }

   /* Unlock codec */
   codecp->locked_by_client = 0;

   return 0;
}

/***************************************************************************/
/**
*  This callback is used to synchronize the audio interfaces. It is
*  registered with each audio interface that requests synchronization
*  and is expected to be called once per frame tick by each interface
*  regardless of the number of codec channels the interface has.
*
*  @return  Nothing.
*
*  @remark  This routine executes in an atomic context; i.e. within an ISR.
*/
static void halaudio_frame_elapsed( void *data )
{
   struct halaudio_if_node *interfp = data;

   HALAUDIO_LOG( "Called by %s", interfp->name );

   if ( !interfp->running || !interfp->sync_with_others 
   ||   interfp->enabled == HALAUDIO_INTERFACE_DISABLED )
   {
      /* Interface has been powered down, is unsynchronized or is
       * disabled. Ignore interrupts from it.
       */
      return;
   }

   interfp->rollcall++;
   if ( interfp->rollcall > 1 )
   {
      /* Looks like either one of the interfaces died or this one is running
       * fast. Set error condition to recover from this error later
       */
      KNLLOG( "Looks like %s is triggering too fast", interfp->name );
   }

   /* See which 'enabled' interfaces have not reported in yet. Interfaces that
    * are transitioning to be enabled are not counted. Interfaces that did
    * not request to be synchronized are also not counted.
    */
   llist_foreach_item( interfp, &gInterfaces.list, lnode )
   {
      if ( interfp->enabled  == HALAUDIO_INTERFACE_ENABLED
      &&   interfp->sync_with_others )
      {
         if ( interfp->rollcall == 0 )
         {
            /* Found an interface that has not reported in yet. Quit now and continue to wait */
            return;
         }
      }
   }

   /* All enabled and synchronized interfaces have reported in */
   HALAUDIO_LOG( "syncd" );

   /* Service requesets to enable pending interfaces */
   /* FIXME: should consider enabling interface at first interrupt after sync'd frame */
   if ( gResyncEnable )
   {
      interfp = gResyncEnable;
      HALAUDIO_LOG( "resync if=%i", interfp->id );

      interfp->ops.enable();
      interfp->enabled = HALAUDIO_INTERFACE_ENABLED;
      interfp->running = 1;
      up( &interfp->resync );

      gResyncEnable = NULL;
   }

   /* Signal CSX beginning of frame if function registered */
   if ( gCsxFrameSync.frame_sync_fp != NULL )
   {
      gCsxFrameSync.frame_sync_fp( gCsxFrameSync.data );
   }

   /* Indicate the passing of time to the amxr */
   amxrElapsedTime( HALAUDIO_DEFAULT_FRAME_PERIOD );

   /* Service interface disable requests */
   if ( gResyncDisable )
   {
      interfp = gResyncDisable;
      HALAUDIO_LOG( "disable if=%i", interfp->id );

      interfp->enabled = HALAUDIO_INTERFACE_DISABLED;
      interfp->running = 0;
      up( &interfp->resync );
      gResyncDisable = NULL;
   }

   /* Update rollcall counts */
   llist_foreach_item( interfp, &gInterfaces.list, lnode )
   {
      if ( interfp->rollcall )
      {
         interfp->rollcall--;
      }
   }
}

/***************************************************************************/
/**
*  Find at least one other interface that is enabled and
*  synchronized.
*
*  @return
*     1     If at least one interface is enabled
*     0     No interfaces are enabled
*     -ve   error
*/
static int halaudio_find_others_synced(
   const struct halaudio_if_node *interfp    /*<< Ptr to interface node to not match */
)
{
   struct halaudio_if_node *tmpifp;
   int                      found;

   down( &gInterfaces.mutex );

   found = 0;
   llist_foreach_item( tmpifp, &gInterfaces.list, lnode )
   {
      if ( tmpifp != interfp
      &&   tmpifp->enabled == HALAUDIO_INTERFACE_ENABLED
      &&   tmpifp->sync_with_others )
      {
         found = 1;
         break;
      }
   }

   up( &gInterfaces.mutex );

   return found;
}

/***************************************************************************/
/**
*  Re-sychronize interface with other running interfaces. It is the
*  responsibility of the caller to ensure that other interfaces are
*  actually running.
*
*  @return
*     0     Success
*     -ve   Error code
*/
static int halaudio_resync_interface(
   struct halaudio_if_node *interfp /*<< (i) Ptr to interface node to resync */
)
{
   unsigned long flags;

   local_irq_save( flags );

   gResyncEnable = interfp;

   local_irq_restore( flags );

   /* Wait for synchronization to complete */
   down( &interfp->resync );

   return 0;
}

/***************************************************************************/
/**
*  Sychronize the disable of an interface when other sychronized interfaces
*  are running. Synchronization is necessary because a disable is not
*  permitted while a roll-call is occurring.
*
*  It is the responsibility of the caller to ensure that other interfaces
*  are actually running.
*
*  @return
*     0     Success
*     -ve   Error code
*/
static int halaudio_disable_if_while_others_running(
   struct halaudio_if_node *interfp /*<< (i) Ptr to interface node to disable */
)
{
   unsigned long flags;

   local_irq_save( flags );

   gResyncDisable = interfp;

   local_irq_restore( flags );

   /* Wait for synchronization to complete */
   down( &interfp->resync );

   return 0;
}

/***************************************************************************/
/**
*  Enable or disable an audio interface low level routine that does not
*  hold any mutex (either interface or power mutex)
*
*  @return
*     0        Success
*     -EINVAL  Invalid parameters
*
*  @remarks
*     See halAudioCoreEnableInterface for a more elaborate description.
*/
static int halaudio_enable_interface_unsafe(
   struct halaudio_if_node *interfp,   /*<< (io) Ptr to interface structure */
   int                      enable     /*<< (i)  1 to enable, 0 to disable */
)
{
   int err, enabled_state;

   err            = 0;
   enabled_state  = interfp->enabled;

   /* Only change interface status if request is different
    * from current status
    */
   if (( enable && enabled_state == HALAUDIO_INTERFACE_DISABLED )
   ||  ( !enable && enabled_state != HALAUDIO_INTERFACE_DISABLED ))
   {
      HALAUDIO_IF_OPS  *ops;
      unsigned long flags;

      ops = &interfp->ops;

      /* Enabling and disable an audio interface involves different steps
       * depending on the global audio power level
       *
       * a) Deep-sleep
       *    Enabling and disabling an audio interface just requires
       *    setting the state. No further work needs to be done.
       *
       * b) Not deep-sleep
       *    While the audio power is active, other synchronized interfaces
       *    may be running. Enabling and disabling are only permitted in
       *    certain windows of time. In particular, both are permitted
       *    after roll-call is complete.
       */
      if ( gPower.level != HALAUDIO_POWER_DEEP_SLEEP )
      {
         int others_found = halaudio_find_others_synced( interfp );

         if ( others_found < 0 )
         {
            return others_found;    /* error occurred */
         }

         if ( enable )
         {
            /* Prepare interface */
            err = ops->prepare();
            if ( err )
            {
               HALAUDIO_PRINTK( "failed to prepare %s interface err=%i", interfp->name, err );
               return err;
            }

            interfp->rollcall = 0;

            if ( others_found )
            {
               /* Even interfaces that don't require to be synchronized
                * are started in a synchronized method if possible.
                */

               interfp->enabled = HALAUDIO_INTERFACE_PENDING_ENABLE;

               /* Resync interface with other interfaces */
               HALAUDIO_LOG( "enable resync" );
               err = halaudio_resync_interface( interfp );
               HALAUDIO_LOG( "done enable resync err=%i", err );
               if ( err )
               {
                  return err;
               }
            }
            else
            {
               local_irq_save( flags );

               /* No other interfaces are running, just enable this one */
               interfp->enabled = HALAUDIO_INTERFACE_ENABLED;
               err = ops->enable();
               if ( err )
               {
                  HALAUDIO_PRINTK( "failed to enable %s interface err=%i", interfp->name, err );
                  local_irq_restore( flags );
                  return err;
               }
               interfp->running = 1;

               local_irq_save( flags );
            }
         }
         else  /* disable */
         {
            if ( others_found )
            {
               /* Synchronize disable */
               HALAUDIO_LOG( "disable sync" );
               err = halaudio_disable_if_while_others_running( interfp );
               HALAUDIO_LOG( "done disable sync err=%i", err );
               if ( err )
               {
                  return err;
               }
            }

            local_irq_save( flags );
            interfp->enabled = HALAUDIO_INTERFACE_DISABLED;
            interfp->running = 0;
            local_irq_restore( flags );

            /* Disable all codecs in interface */
            err = ops->disable();
            if ( err )
            {
               HALAUDIO_PRINTK( "failed to disable %s interface err=%i", interfp->name, err );
               return err;
            }
         }
      }
      else
      {
         /* Nothing more to do since global audio power is in deep-sleep */
         interfp->enabled = enable ? 
            HALAUDIO_INTERFACE_ENABLED : HALAUDIO_INTERFACE_DISABLED;
      }
   }

   return 0;
}

/***************************************************************************/
/**
*  Enable or disable an audio interface low level routine.
*
*  @return
*     0        Success
*     -EINVAL  Invalid parameters
*
*  @remarks
*     See halAudioCoreEnableInterface for a more elaborate description.
*
*     This routine grabs interface and power mutexes.
*/
static int halaudio_enable_interface(
   struct halaudio_if_node *interfp,   /*<< (io) Ptr to interface structure */
   int                      enable     /*<< (i)  1 to enable, 0 to disable */
)
{
   int err;

   down( &interfp->mutex );

   /* Grab audio power control */
   down( &gPower.mutex );

   err = halaudio_enable_interface_unsafe( interfp, enable );

   up( &gPower.mutex );

   /* Find unsynchronized clock backup */
   down( &gInterfaces.mutex );
   halaudio_find_backup_interface_unsafe( &gInterfaces.unsync_backup_ifp );
   up( &gInterfaces.mutex );

   up( &interfp->mutex );
   return err;
}

/***************************************************************************/
/**
*  Read/write IO callback used by low-level driver to indicate that
*  operation has completed.
*/
static void halaudio_io_cb(
   int   bytes,                     /*<< Number of bytes transacted */
   void *data                       /*<< User data */
)
{
   struct halaudio_rw_state *state = data;
   state->bytes = bytes;
   up( &state->sem );
}

/***************************************************************************/
/**
*  Helper function for setting power level.  This function is used in
*  halAudioCoreSetPower and in power management suspend/resume.  It is expected
*  that the user of this function handles the mutex for protecting
*  this critical section.
*
*  @return
*     0              Success
*     -ve            Error code
*/

static int halaudio_set_power_unsafe( HALAUDIO_POWER_LEVEL final_level, HALAUDIO_POWER_LEVEL prev_level )
{
   unsigned long           flags;
   struct halaudio_if_node *interfp;
   int                     rc = 0;

   /* Protect against changes to the interface list */
   down( &gInterfaces.mutex );

   if ( final_level == HALAUDIO_POWER_DEEP_SLEEP )
   {
      if ( prev_level == HALAUDIO_POWER_DEEP_SLEEP )
      {
         HALAUDIO_PRINTK( "already in deep sleep" );
         goto cleanup_and_exit;
      }

      /* Delay 800 ms before cutting power to allow time for audio
       * buffers to flush
       */
      set_current_state( TASK_INTERRUPTIBLE );
      schedule_timeout( gHalAudioSleepDelayMsec );

      /* Disable interfaces */
      local_irq_save( flags );
      llist_foreach_item( interfp, &gInterfaces.list, lnode )
      {
         interfp->running  = 0;
      }
      local_irq_restore( flags );
      llist_foreach_item( interfp, &gInterfaces.list, lnode )
      {
         HALAUDIO_IF_OPS *ops;
         ops   = &interfp->ops;
         rc    = ops->disable();
         if ( rc )
         {
            HALAUDIO_PRINTK( "failed to disable %s interface err=%i", interfp->name, rc );
            goto cleanup_and_exit;
         }
      }
   }
   else  /* Activate power */
   {
      /* Wake-up audio driver if previously sleeping */
      if ( prev_level == HALAUDIO_POWER_DEEP_SLEEP )
      {
         /* Prepare interfaces and clear reference counters */
         llist_foreach_item( interfp, &gInterfaces.list, lnode )
         {
            HALAUDIO_IF_OPS *ops;

            if ( !interfp->enabled )
            {
               /* Skip over disabled interface */
               continue;
            }

            interfp->rollcall = 0;
            ops               = &interfp->ops;

            rc = ops->prepare();
            if ( rc )
            {
               HALAUDIO_PRINTK( "failed to prepare %s interface err=%i", interfp->name, rc );
               goto cleanup_and_exit;
            }
         }

         /* Create critical section to synchronize all audio channel start times */
         local_irq_save( flags );
         llist_foreach_item( interfp, &gInterfaces.list, lnode )
         {
            HALAUDIO_IF_OPS *ops;

            if ( !interfp->enabled )
            {
               /* Skip over disabled interface */
               continue;
            }

            ops   = &interfp->ops;
            rc    = ops->enable();
            if ( rc )
            {
               HALAUDIO_PRINTK( "failed to enable %s interface err=%i", interfp->name, rc );
               local_irq_restore( flags );
               goto cleanup_and_exit;
            }
            interfp->running = 1;
         }
         local_irq_restore( flags );
      }
   }

   /* Enable/disable power to analog interfaces */
   llist_foreach_item( interfp, &gInterfaces.list, lnode )
   {
      HALAUDIO_IF_OPS  *ops;
      int               powerdn;

      ops      = &interfp->ops;
      powerdn  = (final_level != HALAUDIO_POWER_FULL_POWER);

      if ( !interfp->enabled )
      {
         /* Interface disabled, so power down its analog blocks */
         powerdn = 1;
      }

      if ( ops->ana_powerdn )
      {
         rc = ops->ana_powerdn( powerdn );
         if ( rc )
         {
            HALAUDIO_PRINTK( "failed to affect the %s analog interface powerdn=%i err=%i",
                 interfp->name, powerdn, rc );
            goto cleanup_and_exit;
         }
      }
   }

cleanup_and_exit:

   up( &gInterfaces.mutex );

   return rc;
}

/***************************************************************************/
/**
*  Power Management Suspend. Set power level to deep sleep and disable clocks.
*  Power mutex is not released until resume is called.
*
*  @return
*     0              Success
*     -ve            Error code
*/
static int halaudio_pm_suspend( void )
{
   struct halaudio_if_node *interfp;
   int                      err;

   down( &gPower.mutex );

   err = halaudio_set_power_unsafe( HALAUDIO_POWER_DEEP_SLEEP, gPower.level );
   if ( err )
   {
      /* power down failed, so unlock mutex and fail to suspend */
      HALAUDIO_PRINTK( "failed to set power level to deep sleep" );
      goto cleanup_and_exit;
   }

   /* Proceed to shutdown clocks for enabled interfaces */
   down( &gInterfaces.mutex );

   llist_foreach_item( interfp, &gInterfaces.list, lnode )
   {
      if ( interfp->enabled )
      {
         HALAUDIO_IF_OPS *ops;

         ops = &interfp->ops;
         if ( ops->pm_shutdown )
         {
            err = ops->pm_shutdown();
            if ( err )
            {
               HALAUDIO_PRINTK( "failed to suspend %s interface (err=%i). "
                     "Proceeding anyway.", interfp->name, err );
            }
         }
      }
   }
   up( &gInterfaces.mutex );

   /* Keep hold of power mutex until resume */

   return 0;

cleanup_and_exit:
   up( &gPower.mutex );
   return err;
}

/***************************************************************************/
/**
*  Power Management Resume.  Re-enable clocks and restore power level.
*
*  @return
*     0              Success
*     -ve            Error code
*/
static int halaudio_pm_resume( void )
{
   struct halaudio_if_node *interfp;
   int                      rc, err;

   rc = 0;

   /* re-enable clocks */
   down( &gInterfaces.mutex );
   llist_foreach_item( interfp, &gInterfaces.list, lnode )
   {
      if ( interfp->enabled )
      {
         HALAUDIO_IF_OPS *ops;

         ops = &interfp->ops;
         if ( ops->pm_resume )
         {
            err = ops->pm_resume();
            if ( err )
            {
               HALAUDIO_PRINTK( "failed to resume %s interface err=%i",
                     interfp->name, err );
               rc = err;
            }
         }
      }
   }
   up( &gInterfaces.mutex );

   /* restore power level */
   err = halaudio_set_power_unsafe( gPower.level, HALAUDIO_POWER_DEEP_SLEEP );
   if ( err )
   {
      HALAUDIO_PRINTK( "failed to restore power level err=%i", err );
      rc = err;
   }
   up( &gPower.mutex );

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
static int __init halaudio_init( void )
{
   HALAUDIO_API_FUNCS   apifuncs;
   HALAUDIO_PM_OPS      pm_ops;

   /* Initialize interface and codec list heads and mutexes */
   memset( &gInterfaces, 0, sizeof(gInterfaces) );
   memset( &gCodecs, 0, sizeof(gCodecs) );
   memset( &gCsxFrameSync, 0, sizeof(gCsxFrameSync) );
   llist_head_init( &gInterfaces.list );
   llist_head_init( &gCodecs.list );
   sema_init( &gInterfaces.mutex, 1 );
   sema_init( &gCodecs.mutex, 1 );

   /* Initialize client list */
   memset( &gClients, 0, sizeof(gClients) );
   llist_head_init( &gClients.list );
   sema_init( &gClients.mutex, 1 );

   /* Initialize power structure */
   memset( &gPower, 0, sizeof(gPower) );
   sema_init( &gPower.mutex, 1 );
   gPower.level = HALAUDIO_POWER_DEEP_SLEEP;

   /* Register API routines */
   memset( &apifuncs, 0, sizeof(apifuncs) );
   apifuncs.allocateClient    = halAudioCoreAllocateClient;
   apifuncs.freeClient        = halAudioCoreFreeClient;
   apifuncs.queryInterfaceByName = halAudioCoreQueryInterfaceByName;
   apifuncs.queryCodecByName  = halAudioCoreQueryCodecByName;
   apifuncs.lockCodec         = halAudioCoreLockCodec;
   apifuncs.unlockCodec       = halAudioCoreUnlockCodec;
   apifuncs.getGainInfo       = halAudioCoreGetGainInfo;
   apifuncs.setGain           = halAudioCoreSetGain;
   apifuncs.setPower          = halAudioCoreSetPower;
   apifuncs.getPower          = halAudioCoreGetPower;
   apifuncs.write             = halAudioCoreWrite;
   apifuncs.read              = halAudioCoreRead;
   apifuncs.setSuperUser      = halAudioCoreSetSuperUser;
   apifuncs.setFreq           = halAudioCoreSetFreq;
   apifuncs.getFreq           = halAudioCoreGetFreq;
   apifuncs.setEquParms       = halAudioCoreSetEquParms;
   apifuncs.getEquParms       = halAudioCoreGetEquParms;
   apifuncs.getHardwareInfo   = halAudioCoreGetHardwareInfo;
   apifuncs.getInterfaceInfo  = halAudioCoreGetInterfaceInfo;
   apifuncs.getCodecInfo      = halAudioCoreGetCodecInfo;
   apifuncs.enableInterface   = halAudioCoreEnableInterface;
   apifuncs.addInterface      = halAudioCoreAddInterface;
   apifuncs.delInterface      = halAudioCoreDelInterface;
   apifuncs.setSyncFlag       = halAudioCoreSetSyncFlag;
   apifuncs.setCsxIoPoints    = halAudioCoreSetCsxIoPoints;
   apifuncs.setCsxFrameSync   = halAudioCoreSetCsxFrameSync;
   halAudioSetApiFuncs( &apifuncs );

   /* Register power management routines */
   memset( &pm_ops, 0, sizeof(pm_ops) );
   pm_ops.suspend          = halaudio_pm_suspend;
   pm_ops.resume           = halaudio_pm_resume;
   halAudioSetPowerManageOps( &pm_ops );

   return 0;
}

/***************************************************************************/
/**
*  Driver destructor routine. Frees all resources
*/
static void __exit halaudio_exit( void )
{
   struct halaudio_if_node      *interfp, *tmpifp;
   struct halaudio_codec_node   *codecp, *tmpcodec;
   struct halaudio_client_node  *client, *tmpclient;
   int                           error;

   halAudioSetPowerManageOps( NULL );
   halAudioSetApiFuncs( NULL );

   /* Free all clients */
   llist_foreach_item_safe( client, tmpclient, &gClients.list, lnode )
   {
      error = halAudioCoreFreeClient( halaudio_get_client_hdl( client ));
      if ( error != 0 )
      {
         HALAUDIO_PRINTK( "failed to free client rc=%i", error );
      }
   }

   /* Delete remaining interfaces */
   llist_foreach_item_safe( interfp, tmpifp, &gInterfaces.list, lnode )
   {
      error = halaudio_del_interface_unsafe( interfp );
      if ( error != 0 )
      {
         HALAUDIO_PRINTK( "failed to delete interface rc=%i", error );
      }
   }

   /* Delete remaining codecs. There shouldn't be any codecs left at this
    * point. This is just a safe guard.
    */
   llist_foreach_item_safe( codecp, tmpcodec, &gCodecs.list, lnode )
   {
      error = halaudio_del_codec_unsafe( codecp, NULL );
      if ( error != 0 )
      {
         HALAUDIO_PRINTK( "failed to delete codec rc=%i", error );
      }
   }

   /* Additional sanity checks */
   if ( gCodecs.total )
   {
      HALAUDIO_PRINTK( "orphaned %i codec channels", gCodecs.total );
   }
   if ( gInterfaces.total )
   {
      HALAUDIO_PRINTK( "orphaned %i interfaces", gInterfaces.total );
   }
}

module_init( halaudio_init );
module_exit( halaudio_exit );
MODULE_AUTHOR( "Broadcom" );
MODULE_DESCRIPTION( "HALAUDIO Driver" );
MODULE_LICENSE( "GPL" );
