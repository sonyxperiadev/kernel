/*****************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
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
*  @file    vcpdrv_user.c
*
*  @brief   This file implements the infrastructure for user space
*           VCP devices
*
*****************************************************************************/
/* ---- Include Files ---------------------------------------------------- */

#include <linux/types.h>                     /* For stdint types: uint8_t, etc. */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/proc_fs.h>                   /* For proc_fs files */
#include <linux/init.h>
#include <linux/string.h>
#include <linux/vmalloc.h>                   /* For memory alloc */
#include <linux/sysctl.h>
#include <linux/semaphore.h>                 /* For down_interruptible, up, etc. */
#include <linux/list.h>                      /* Linked list */
#include <linux/version.h>

#include <linux/broadcom/knllog.h>           /* For debugging */
#include <linux/broadcom/vcp_device.h>       /* VCP Device API */
#include <linux/broadcom/vcp_msg.h>          /* VCP msg API */

#include <asm/uaccess.h>                     /* User access routines */
#include "vcppriv.h"

/* ---- Public Variables ------------------------------------------------- */
/* ---- Private Constants and Types -------------------------------------- */

#define PROC_NAME             "vcp_userdevs"

/* Generic list structure */
struct vcpdev_list
{
   struct list_head           list;          /* List head */
   struct semaphore           mutex;         /* Protect list */
   struct semaphore           avail;         /* Counting semaphore */
   volatile unsigned int      total;         /* Total elements in list */
};

/* Video device node - used for encoder and decoder devices */
struct vcpuser_device_node
{
   struct list_head           lnode;         /* List node */ \
   char                       name[32];      /* Name of device */ \
   VCP_DIR                    dir;           /* Encoder or decoder */ \
   void                      *user_devdata;  /* User's private device data */ \
   VCP_DEV_HDL                devhdl;        /* Kernel side device handle */ \
   struct vcpdev_list         msglist;       /* Message list */ \
   struct semaphore           msglock;       /* Lock to serialize msg processing */ \
   struct vcpdev_list         resplist;      /* Response list */
   union
   {
      VCP_DEC_DEVICE_OPS         dec;        /* Device operations */
      VCP_ENC_DEVICE_OPS         enc;        /* Device operations */
   }ops;

   struct vcpdev_list         framelist;     /* List of frames */

};

/* Video decoder or encoder pipeline node */
struct vcpuser_pipe_node
{
   struct list_head           lnode;         /* List node */
   VCP_DIR                    dir;           /* Encoder or decoder */
   struct vcpuser_device_node *nodep;        /* Parent device node */

   /* Host Callbacks */
   union{
      VCP_HOST_ENC_CBKS      enc;
      VCP_HOST_DEC_CBKS      dec;
   }cbks;
   void                      *privdata;      /* Host private data */
   VCP_PIPE_HDL               user_pipehdl;  /* User domain pipeline handle */

   /* State */
   int                        active;        /* 1 if active, otherwise 0 */

   /* Statistics */
   unsigned int               processedPkts; /* Number of processed pkts */
   unsigned int               droppedPkts;   /* Number of intentially dropped pkts */
};

/* This structure is an overlay to make use of the VCP frame's scratch area.
 *
 * Warning: This structure definition is shared with the user syslib. The
 *          variables in the public section are shared between kernel and
 *          user domains.
 */
struct framebuf_priv
{
   /* PUBLIC information */
   VCP_PIPE_HDL               pipehdl;       /* Pipeline handle */
   void                      *devdata;       /* Private device data */

   /* Private information */
   struct list_head           lnode;         /* List node */
};

/* Message node structure */
struct vcpuser_msg_node
{
   struct list_head           lnode;         /* List node */
   VCPMSG_ALL                 payload;
};

/* Response node structure */
struct vcpuser_resp_node
{
   struct list_head           lnode;         /* List node */
   VCPMSG_RESP                data;          /* Response data */
};


#define VCP_LOG_SUPPORT             0
#define VCP_VERBOSE_LOG_SUPPORT     0

#if VCP_LOG_SUPPORT
#define VCP_LOG                     KNLLOG
#else
#define VCP_LOG(s,args...)
#endif
#if VCP_VERBOSE_LOG_SUPPORT
#define VCP_VERBOSE_LOG             KNLLOG
#else
#define VCP_VERBOSE_LOG(s,args...)
#endif

/* ---- Private Variables ------------------------------------------------ */

/* Lists of registered devices */
static struct vcpdev_list           gDecDeviceList;
static struct vcpdev_list           gEncDeviceList;

/* Lists of open pipelines */
static struct vcpdev_list           gDecPipeList;
static struct vcpdev_list           gEncPipeList;

/* ---- Private Function Prototypes -------------------------------------- */
static void init_list( struct vcpdev_list *listp );
static int vcpdev_start( VCP_PIPE_HDL hdl, void *devdata );
static int vcpdev_stop( VCP_PIPE_HDL hdl, void *devdata );
static int vcpdev_enc_open( VCP_HOST_ENC_CBKS *cbksp, void *privdata, VCP_PIPE_HDL *hdlp, void *devdata );
static int vcpdev_enc_close( VCP_PIPE_HDL hdl, void *devdata );
static int vcpdev_enc_setparm( VCP_PIPE_HDL hdl, VCP_PARM_INDEX index, const void *datap, void *devdata );
static int vcpdev_enc_getstats( VCP_PIPE_HDL hdl, int reset, VCP_ENCODER_STATS *stats, void *devdata );
static int vcpdev_enc_getcap( VCP_PIPE_HDL hdl, VCP_CAPABILITIES *cap, void *devdata );
static int vcpdev_dec_open( VCP_HOST_DEC_CBKS *cbksp, void *privdata, VCP_PIPE_HDL *hdlp, void *devdata );
static int vcpdev_dec_close( VCP_PIPE_HDL hdl, void *devdata );
static int vcpdev_dec_setparm( VCP_PIPE_HDL hdl, VCP_PARM_INDEX index, const void *datap, void *devdata );
static int vcpdev_dec_getstats( VCP_PIPE_HDL hdl, int reset, VCP_DECODER_STATS *stats, void *devdata );
static int vcpdev_dec_getcap( VCP_PIPE_HDL hdl, VCP_CAPABILITIES *cap, void *devdata );
static int vcpdev_decode( VCP_PIPE_HDL hdl, VCP_FRAMEBUF *framebufp, void *devdata );

/* ---- Functions -------------------------------------------------------- */

/* Encoder device methods */
static VCP_ENC_DEVICE_OPS encOps =
{
   .open       = vcpdev_enc_open,
   .close      = vcpdev_enc_close,
   .start      = vcpdev_start,
   .stop       = vcpdev_stop,
   .getencstats = vcpdev_enc_getstats,
   .setparm    = vcpdev_enc_setparm,
   .getcap     = vcpdev_enc_getcap,
};

/* Decoder device methods */
static VCP_DEC_DEVICE_OPS decOps =
{
   .open       = vcpdev_dec_open,
   .close      = vcpdev_dec_close,
   .start      = vcpdev_start,
   .stop       = vcpdev_stop,
   .setparm    = vcpdev_dec_setparm,
   .getdecstats = vcpdev_dec_getstats,
   .getcap     = vcpdev_dec_getcap,
   .decode     = vcpdev_decode,
};

/***************************************************************************/
/**
*  Send a message and wait for a response
*
*  @return  0 on success, otherwise -ve err
*
*  @remarks
*
*  Once msgp is queued, the caller of this routine cannot access its
*  context lest memory corruption may occur. The memory may be dequeued
*  by another thread and then freed.
*/
static int sendmsg(
   struct vcpuser_msg_node *msgp,/**< (i) Ptr to msg  */
   struct vcpdev_list *msglistp, /**< (i) Ptr to msg list */
   VCPMSG_RESP *resp,            /**< (o) Ptr to return response */
   struct vcpdev_list *resplistp,/**< (i) Ptr to response list */
   struct semaphore *lockp       /**< (i) Ptr to lock mutex */    // FIXME: may not need a lock
)
{
   struct vcpuser_resp_node *respnodep;
   int err;

   VCP_VERBOSE_LOG( "About to take msg lock: resplistp=0x%lx msgid=%i.\n",
         (unsigned long)resplistp, msgp->payload.base.msgid );

   /* FIXME: Serialize message by holding mutex until response is received */
   down( lockp );

   /* Queue message */
   down( &msglistp->mutex );
   list_add_tail( &msgp->lnode, &msglistp->list );
   msglistp->total++;
   up( &msglistp->mutex );

   up( &msglistp->avail );

   VCP_VERBOSE_LOG( "Sent message: resplistp=0x%lx msgid=%i. Waiting for response...\n",
         (unsigned long)resplistp, msgp->payload.base.msgid );

   /* Wait for response */
   err = down_interruptible( &resplistp->avail );
   if ( err < 0 )
   {
      VCP_DEBUG( "Interrupted waiting for response!\n" );
      err = -EBUSY;
      goto backout;
   }

   /* Dequeue response */
   down( &resplistp->mutex );
   respnodep = list_entry( resplistp->list.next, struct vcpuser_resp_node, lnode );
   list_del( &respnodep->lnode );
   resplistp->total--;
   up( &resplistp->mutex );

   memcpy( resp, &respnodep->data, sizeof(*resp) );
   kfree( respnodep );

   VCP_VERBOSE_LOG( "Got response! msgid=%i rc=%i\n", resp->msgid, resp->rc );

backout:
   up( lockp );

   return err;
}

/***************************************************************************/
/**
*  Open a decoder device channel
*
*  @return  0 on success, otherwise -ve error
*/
static int vcpdev_dec_open(
   VCP_HOST_DEC_CBKS *cbksp,     /**< (i) Ptr to host callbacks */
   void *privdata,               /**< (i) Private data for callbacks */
   VCP_PIPE_HDL *hdlp,           /**< (o) Ptr to handle if success */
   void *devdata                 /**< (i) Private device data */
)
{
   struct vcpuser_device_node *nodep = devdata;
   struct vcpuser_pipe_node *pipep;
   struct vcpuser_msg_node *msgp=NULL;
   VCPMSG_OPEN *datap;
   VCPMSG_RESP resp;
   int err;

   pipep = kmalloc( sizeof(*pipep), GFP_KERNEL );
   if ( pipep == NULL )
   {
      err = -ENOMEM;
      goto backout;
   }
   memset( pipep, 0, sizeof(*pipep) );

   pipep->dir = VCP_DIR_DECODER;
   pipep->nodep = nodep;

   /* Save callback information */
   memcpy( &pipep->cbks.dec, cbksp, sizeof(pipep->cbks.dec) );
   pipep->privdata = privdata;

   /* Allocate and queue message for user device */
   msgp = kmalloc( sizeof(*msgp), GFP_KERNEL );
   if ( msgp == NULL )
   {
      err = -ENOMEM;
      goto backout;
   }
   memset( msgp, 0, sizeof(*msgp) );
   datap = &msgp->payload.open;

   datap->msgid = VCPMSG_OPEN_DEC_ID;
   datap->devdata = nodep->user_devdata;
   datap->knlpipe = pipep;

   err = sendmsg( msgp, &nodep->msglist, &resp, &nodep->resplist, &nodep->msglock );
   if ( err )
   {
      printk( "%s: Failed sending msg or getting response from %s\n",
            __func__, nodep->name );
      goto backout;
   }
   /* Override message pointer as it is queued and freed by sendmsg */
   msgp = NULL;

   if ( resp.msgid != VCPMSG_OPEN_DEC_ID )
   {
      printk( "%s: Mismatched msgid! Got respid=%i!\n",
            __func__, resp.msgid );
      err = -EINVAL;
      goto backout;
   }

   if ( resp.rc == 0 )
   {
      down( &gDecPipeList.mutex );
      list_add_tail( &pipep->lnode, &gDecPipeList.list );
      gDecPipeList.total++;
      up( &gDecPipeList.mutex );

      pipep->user_pipehdl = resp.data.hdl;
      *hdlp = pipep;  /* Return handle */
   }
   else
   {
      err = resp.rc;
      goto backout;
   }

   return resp.rc;

backout:
   if ( pipep )
   {
      kfree( pipep );
   }
   if ( msgp )
   {
      kfree( msgp );
   }
   return err;
}

/***************************************************************************/
/**
*  Simple proxy message
*
*  @return  0 on success, otherwise -ve error
*/
static int vcpdev_simple_msg(
   unsigned int msgid,           /**< (i) Message ID */
   VCP_PIPE_HDL hdl,             /**< (i) Pipeline handle */
   void *devdata                 /**< (i) Private device data */
)
{
   struct vcpuser_device_node *nodep = devdata;
   struct vcpuser_pipe_node *pipep = hdl;
   struct vcpuser_msg_node *msgp;
   VCPMSG_BASE *datap;
   VCPMSG_RESP resp;
   int err;

   /* Allocate and queue message for user device */
   msgp = kmalloc( sizeof(*msgp), GFP_KERNEL );
   if ( msgp == NULL )
   {
      return -ENOMEM;
   }
   memset( msgp, 0, sizeof(*msgp) );
   datap = &msgp->payload.base;

   datap->msgid = msgid;
   datap->hdl = pipep->user_pipehdl;
   datap->devdata = nodep->user_devdata;

   err = sendmsg( msgp, &nodep->msglist, &resp, &nodep->resplist, &nodep->msglock );
   if ( err )
   {
      printk( "%s: Failed sending msgid=%i to user device '%s' err=%i\n",
            __func__, msgid, nodep->name, err );
      return err;
   }

   if ( resp.msgid != msgid )
   {
      printk( "%s: Mismatched msgid=%i and respid=%i!\n",
            __func__, msgid, resp.msgid );
      return -EINVAL;
   }

   return resp.rc;
}

/***************************************************************************/
/**
*  Close decoder device channel
*
*  @return  0 on success, otherwise -ve error
*/
static int vcpdev_dec_close(
   VCP_PIPE_HDL hdl,             /**< (i) Handle */
   void *devdata                 /**< (i) Private device data */
)
{
   struct vcpuser_pipe_node *pipep = hdl;
   int err;

   VCP_LOG( "hdl=0x%lx devdata=0x%lx", (unsigned long)hdl, (unsigned long) devdata );

   err = vcpdev_simple_msg( VCPMSG_CLOSE_ID, hdl, devdata );

   down( &gDecPipeList.mutex );
   list_del( &pipep->lnode );
   gDecPipeList.total--;
   kfree( pipep );
   up( &gDecPipeList.mutex );

   return err;
}

/***************************************************************************/
/**
*  Start a decoder or encoder device
*
*  @return  0 on success, otherwise -ve error
*/
static int vcpdev_start(
   VCP_PIPE_HDL hdl,             /**< (i) Pipeline handle */
   void *devdata                 /**< (i) Private device data */
)
{
   struct vcpuser_pipe_node *pipep = hdl;
   int err;

   if ( pipep->active )
   {
      return 0;
   }
   VCP_LOG( "%s hdl=0x%lx devdata=0x%lx\n", (pipep->dir==VCP_DIR_DECODER)?"decoder":"encoder",
           (unsigned long)hdl, (unsigned long) devdata );
   err = vcpdev_simple_msg( VCPMSG_START_ID, hdl, devdata );
   if ( err )
   {
      return err;
   }

   pipep->active = 1;

   return 0;
}

/***************************************************************************/
/**
*  Stop a decoder or encoder device
*
*  @return  0 on success, otherwise -ve error
*/
static int vcpdev_stop(
   VCP_PIPE_HDL hdl,             /**< (i) Pipeline handle */
   void *devdata                 /**< (i) Private device data */
)
{
   struct vcpuser_pipe_node *pipep = hdl;

   if ( !pipep->active )
   {
      return 0;
   }
   VCP_LOG( "%s hdl=0x%lx devdata=0x%lx\n", (pipep->dir==VCP_DIR_DECODER)?"decoder":"encoder",
           (unsigned long)hdl, (unsigned long) devdata );

   pipep->active = 0;

   return vcpdev_simple_msg( VCPMSG_STOP_ID, hdl, devdata );
}

/***************************************************************************/
/**
*  Set decoder device parameters
*
*  @return  0 on success, otherwise -ve error
*/
static int vcpdev_dec_setparm(
   VCP_PIPE_HDL hdl,             /**< (i) Pipeline handle */
   VCP_PARM_INDEX index,         /**< (i) Parameter index */
   const void *parmp,            /**< (i) Ptr to command related data */
   void *devdata                 /**< (i) Private device data */
)
{
   struct vcpuser_device_node *nodep = devdata;
   struct vcpuser_pipe_node *pipep = hdl;
   struct vcpuser_msg_node *msgp;
   VCPMSG_SETPARM *datap;
   VCPMSG_RESP resp;
   int err, parmsize = 0;

   VCP_LOG( "hdl=0x%lx devdata=0x%lx, index=%d\n", (unsigned long)hdl, (unsigned long) devdata, index );
   switch ( index )
   {
      case VCP_PARM_DEC_CONFIG:
         parmsize = sizeof(VCP_DECODER_PARM);
         break;
      case VCP_PARM_DEC_DISP:
         parmsize = sizeof(VCP_DISPLAY_PARM);
         break;
      case VCP_PARM_DISP_MASK:
         parmsize = sizeof(VCP_DISPLAY_MASK_PARM);
         break;
      default:
         printk( "%s: unknown parameter index=%d\n", __func__, index );
         return -EINVAL;
   }

   /* Allocate and queue message for user device */
   msgp = kmalloc( sizeof(*msgp), GFP_KERNEL );
   if ( msgp == NULL )
   {
      return -ENOMEM;
   }
   memset( msgp, 0, sizeof(*msgp) );
   datap = &msgp->payload.setparm;

   datap->msgid = VCPMSG_SETPARM_ID;
   datap->index = index;
   datap->hdl = pipep->user_pipehdl;
   datap->devdata = nodep->user_devdata;
   memcpy( &datap->data, parmp, parmsize );

   err = sendmsg( msgp, &nodep->msglist, &resp, &nodep->resplist, &nodep->msglock );
   if ( err )
   {
      printk( "%s: Failed sending msgid=%i to user device '%s'\n",
            __func__, datap->msgid, nodep->name );
      return err;
   }

   if ( resp.msgid != VCPMSG_SETPARM_ID )
   {
      printk( "%s: Mismatched msgid! Got respid=%i!\n",
            __func__, resp.msgid );
      return -EINVAL;
   }

   return resp.rc;
}

/***************************************************************************/
/**
*  Get decoder device statistics
*
*  @return  0 on success, otherwise -ve error
*/
static int vcpdev_dec_getstats(
   VCP_PIPE_HDL hdl,             /**< (i) Pipeline handle */
   int reset,                    /**< (i) 0=>do not reset stats after reading them, otherwise reset them */
   VCP_DECODER_STATS *statsp,    /**< (o) Ptr to location to store statistics */
   void *devdata                 /**< (i) Private device data */
)
{
   struct vcpuser_device_node *nodep = devdata;
   struct vcpuser_pipe_node *pipep = hdl;
   struct vcpuser_msg_node *msgp;
   VCPMSG_GETDECSTATS *datap;
   VCPMSG_RESP resp;
   int err;

   VCP_LOG( "hdl=0x%lx devdata=0x%lx\n", (unsigned long)hdl, (unsigned long) devdata );
   /* Allocate and queue message for user device */
   msgp = kmalloc( sizeof(*msgp), GFP_KERNEL );
   if ( msgp == NULL )
   {
      return -ENOMEM;
   }
   memset( msgp, 0, sizeof(*msgp) );
   datap = &msgp->payload.getdecstats;

   datap->msgid = VCPMSG_GETDECSTATS_ID;
   datap->reset = reset;
   datap->hdl = pipep->user_pipehdl;
   datap->devdata = nodep->user_devdata;

   err = sendmsg( msgp, &nodep->msglist, &resp, &nodep->resplist, &nodep->msglock );
   if ( err )
   {
      printk( "%s: Failed sending msgid=%i to user device '%s'\n",
            __func__, datap->msgid, nodep->name );
      return err;
   }

   if ( resp.msgid != VCPMSG_GETDECSTATS_ID )
   {
      printk( "%s: Mismatched msgid! Got respid=%i!\n",
            __func__, resp.msgid );
      return -EINVAL;
   }

   if ( resp.rc == 0 )
   {
      /* Copy stats */
      memcpy( statsp, &resp.data.decstats, sizeof(*statsp) );
   }

   return resp.rc;
}

/***************************************************************************/
/**
*  Get decoder device capabilities
*
*  @return  0 on success, otherwise -ve error
*/
static int vcpdev_dec_getcap(
   VCP_PIPE_HDL hdl,             /**< (i) Pipeline handle */
   VCP_CAPABILITIES *cap,        /**< (o) Ptr to location to store capabilities */
   void *devdata                 /**< (i) Private device data */
)
{
   struct vcpuser_device_node *nodep = devdata;
   struct vcpuser_pipe_node *pipep = hdl;
   struct vcpuser_msg_node *msgp;
   VCPMSG_GETCAP *datap;
   VCPMSG_RESP resp;
   int err;

   VCP_LOG("hdl=0x%lx devdata=0x%lx\n", (unsigned long)hdl, (unsigned long) devdata );
   /* Allocate and queue message for user device */
   msgp = kmalloc( sizeof(*msgp), GFP_KERNEL );
   if ( msgp == NULL )
   {
      return -ENOMEM;
   }
   memset( msgp, 0, sizeof(*msgp) );
   datap = &msgp->payload.getcap;

   datap->msgid = VCPMSG_GETCAP_ID;
   datap->hdl = pipep->user_pipehdl;
   datap->devdata = nodep->user_devdata;

   err = sendmsg( msgp, &nodep->msglist, &resp, &nodep->resplist, &nodep->msglock );
   if ( err )
   {
      printk( "%s: Failed sending msgid=%i to user device '%s'\n",
            __func__, datap->msgid, nodep->name );
      return err;
   }

   if ( resp.msgid != VCPMSG_GETCAP_ID )
   {
      printk( "%s: Mismatched msgid! Got respid=%i!\n",
            __func__, resp.msgid );
      return -EINVAL;
   }

   if ( resp.rc == 0 )
   {
      /* Copy capabilities */
      memcpy( cap, &resp.data.cap, sizeof(*cap) );
   }

   return resp.rc;
}
/***************************************************************************/
/**
*  Decode frame
*
*  @return  0 on success, otherwise -ve error
*
*  @remark  This routine should return as soon as possible
*/
static int vcpdev_decode(
   VCP_PIPE_HDL hdl,             /**< (i) Pipeline handle */
   VCP_FRAMEBUF *framebufp,      /**< (i) Ptr to frame buffer to decode */
   void *devdata                 /**< (i) Private device data */
)
{
   struct vcpuser_device_node *nodep = devdata;
   struct vcpuser_pipe_node *pipep = hdl;
   struct vcpdev_list *listp;
   struct framebuf_priv *fbnodep;

   if ( !pipep->active )
   {
      printk( "%s: Decoder is inactive. Discarding packet\n", __func__ );
      pipep->cbks.dec.free( framebufp, pipep->privdata );
      return -EPERM;
   }

   VCP_VERBOSE_LOG( "data_len=%u flags=0x%x seq_num=%i timestamp=%i\n",
         framebufp->hdr.data_len, framebufp->hdr.flags,
         framebufp->hdr.seq_num, framebufp->hdr.timestamp );

   /* Tag packet with pipeline handle for later use */
   fbnodep = (void *)framebufp;
   fbnodep->pipehdl = hdl;

   listp = &nodep->framelist;
   down( &listp->mutex );
   list_add_tail( &fbnodep->lnode, &listp->list );
   listp->total++;
   up( &listp->mutex );

   up( &listp->avail );

   return 0;
}

/***************************************************************************/
/**
*  Open encoder pipeline
*
*  @return  0 on success, otherwise -ve error
*/
static int vcpdev_enc_open(
   VCP_HOST_ENC_CBKS *cbksp,     /**< (i) Ptr to host callbacks */
   void *privdata,               /**< (i) Private data for callbacks */
   VCP_PIPE_HDL *hdlp,           /**< (o) Ptr to handle if success */
   void *devdata                 /**< (i) Private device data */
)
{
   struct vcpuser_device_node *nodep = devdata;
   struct vcpuser_pipe_node *pipep;
   struct vcpuser_msg_node *msgp = NULL;
   VCPMSG_OPEN *datap;
   VCPMSG_RESP resp;
   int err;

   pipep = kmalloc( sizeof(*pipep), GFP_KERNEL );
   if ( pipep == NULL )
   {
      err = -ENOMEM;
      goto backout;
   }
   memset( pipep, 0, sizeof(*pipep) );

   pipep->nodep = nodep;

   /* Save callback information */
   memcpy( &pipep->cbks.enc, cbksp, sizeof(pipep->cbks.enc) );
   pipep->privdata = privdata;
   pipep->dir = VCP_DIR_ENCODER;

   /* Allocate and queue message for user device */
   msgp = kmalloc( sizeof(*msgp), GFP_KERNEL );
   if ( msgp == NULL )
   {
      err = -ENOMEM;
      goto backout;
   }
   memset( msgp, 0, sizeof(*msgp) );
   datap = &msgp->payload.open;

   datap->msgid = VCPMSG_OPEN_ENC_ID;
   datap->devdata = nodep->user_devdata;
   datap->knlpipe = pipep;

   err = sendmsg( msgp, &nodep->msglist, &resp, &nodep->resplist, &nodep->msglock );
   if ( err )
   {
      printk( "%s: Failed sending msg or getting response from %s\n",
            __func__, nodep->name );
      goto backout;
   }
   /* Override message pointer as it is queued and freed by sendmsg */
   msgp = NULL;

   if ( resp.msgid != VCPMSG_OPEN_ENC_ID )
   {
      printk( "%s: Mismatched msgid! Got respid=%i!\n",
            __func__, resp.msgid );
      err = -EINVAL;
      goto backout;
   }

   if ( resp.rc == 0 )
   {
      down( &gEncPipeList.mutex );
      list_add_tail( &pipep->lnode, &gEncPipeList.list );
      gEncPipeList.total++;
      up( &gEncPipeList.mutex );

      pipep->user_pipehdl = resp.data.hdl;
      *hdlp = pipep;  /* Return handle */
      VCP_LOG("hdl=0x%lx devdata=0x%lx\n", (unsigned long)*hdlp, (unsigned long) devdata );
   }
   else
   {
      err = resp.rc;
      goto backout;
   }

   return resp.rc;

backout:
   if ( pipep )
   {
      kfree( pipep );
   }
   if ( msgp )
   {
      kfree( msgp );
   }
   return err;
}

/***************************************************************************/
/**
*  Close encoder pipeline
*
*  @return  0 on success, otherwise -ve error
*/
static int vcpdev_enc_close(
   VCP_PIPE_HDL hdl,             /**< (i) Pipeline handle */
   void *devdata                 /**< (i) Private device data */
)
{
   struct vcpuser_pipe_node *pipep = hdl;
   int err;

   VCP_LOG("hdl=0x%lx devdata=0x%lx\n", (unsigned long)hdl, (unsigned long) devdata );

   err = vcpdev_simple_msg( VCPMSG_CLOSE_ID, hdl, devdata );

   down( &gEncPipeList.mutex );
   list_del( &pipep->lnode );
   gEncPipeList.total--;
   kfree( pipep );
   up( &gEncPipeList.mutex );

   return err;
}

/***************************************************************************/
/**
*  Set encoder pipeline parameters
*
*  @return  0 on success, otherwise -ve error
*/
static int vcpdev_enc_setparm(
   VCP_PIPE_HDL hdl,             /**< (i) Pipeline handle */
   VCP_PARM_INDEX index,         /**< (i) Parameter index */
   const void *parmp,            /**< (i) Ptr to command related data */
   void *devdata                 /**< (i) Private device data */
)
{
   struct vcpuser_device_node *nodep = devdata;
   struct vcpuser_pipe_node *pipep = hdl;
   struct vcpuser_msg_node *msgp;
   VCPMSG_SETPARM *datap;
   VCPMSG_RESP resp;
   int err, parmsize = 0;

   VCP_LOG("hdl=0x%lx devdata=0x%lx, index=%d\n", (unsigned long)hdl, (unsigned long) devdata, index );

   switch ( index )
   {
      case VCP_PARM_ENC_CONFIG:
         parmsize = sizeof(VCP_ENCODER_PARM);
         break;
      case VCP_PARM_ENC_FREEZE:
         parmsize = sizeof(int);
         break;
      case VCP_PARM_ENC_FIR:
         parmsize = 0;
         break;
      case VCP_PARM_ENC_DISP:
         parmsize = sizeof(VCP_DISPLAY_PARM);
         break;
      case VCP_PARM_ENC_PRIVACY_MODE:
         parmsize = sizeof(int);
         break;
      case VCP_PARM_ENC_PAN_ZOOM:
         parmsize = sizeof(VCP_PANZOOM_PARM);
         break;
      case VCP_PARM_DISP_MASK:
         parmsize = sizeof(VCP_DISPLAY_MASK_PARM);
         break;
      default:
         printk( "%s: unknown parameter index=%d\n",
               __func__, index );
         return -EINVAL;
   }

   /* Allocate and queue message for user device */
   msgp = kmalloc( sizeof(*msgp), GFP_KERNEL );
   if ( msgp == NULL )
   {
      return -ENOMEM;
   }
   memset( msgp, 0, sizeof(*msgp) );
   datap = &msgp->payload.setparm;

   datap->msgid = VCPMSG_SETPARM_ID;
   datap->index = index;
   datap->hdl = pipep->user_pipehdl;
   datap->devdata = nodep->user_devdata;
   memcpy( &datap->data, parmp, parmsize );

   err = sendmsg( msgp, &nodep->msglist, &resp, &nodep->resplist, &nodep->msglock );
   if ( err )
   {
      printk( "%s: Failed sending msgid=%i to user device '%s'\n",
            __func__, datap->msgid, nodep->name );
      return err;
   }

   if ( resp.msgid != VCPMSG_SETPARM_ID )
   {
      printk( "%s: Mismatched msgid! Got respid=%i!\n",
            __func__, resp.msgid );
      return -EINVAL;
   }

   return resp.rc;
}
#if 0
/***************************************************************************/
/**
*  Get encoder pipeline parameters
*
*  @return  0 on success, otherwise -ve error
*/
static int vcpdev_enc_getparm(
   VCP_PIPE_HDL hdl,             /**< (i) Pipeline handle */
   VCP_PARM_INDEX index,         /**< (i) Parameter index */
   void *datap,                  /**< (o) Ptr to config data */
   void *devdata                 /**< (i) Private device data */
)
{
   return 0;
}
#endif
/***************************************************************************/
/**
*  Get encoder device capabilities
*
*  @return  0 on success, otherwise -ve error
*/
static int vcpdev_enc_getcap(
   VCP_PIPE_HDL hdl,             /**< (i) Pipeline handle */
   VCP_CAPABILITIES *cap,        /**< (o) Ptr to location to store capabilities */
   void *devdata                 /**< (i) Private device data */
)
{
   struct vcpuser_device_node *nodep = devdata;
   struct vcpuser_pipe_node *pipep = hdl;
   struct vcpuser_msg_node *msgp;
   VCPMSG_GETCAP *datap;
   VCPMSG_RESP resp;
   int err;

   VCP_LOG("hdl=0x%lx devdata=0x%lx\n", (unsigned long)hdl, (unsigned long) devdata );
   /* Allocate and queue message for user device */
   msgp = kmalloc( sizeof(*msgp), GFP_KERNEL );
   if ( msgp == NULL )
   {
      return -ENOMEM;
   }
   memset( msgp, 0, sizeof(*msgp) );
   datap = &msgp->payload.getcap;

   datap->msgid = VCPMSG_GETCAP_ID;
   datap->hdl = pipep->user_pipehdl;
   datap->devdata = nodep->user_devdata;

   err = sendmsg( msgp, &nodep->msglist, &resp, &nodep->resplist, &nodep->msglock );
   if ( err )
   {
      printk( "%s: Failed sending msgid=%i to user device '%s'\n",
            __func__, datap->msgid, nodep->name );
      return err;
   }

   if ( resp.msgid != VCPMSG_GETCAP_ID )
   {
      printk( "%s: Mismatched msgid! Got respid=%i!\n",
            __func__, resp.msgid );
      return -EINVAL;
   }

   if ( resp.rc == 0 )
   {
      /* Copy capabilities */
      memcpy( cap, &resp.data.cap, sizeof(*cap) );
   }

   return resp.rc;
}

/***************************************************************************/
/**
*  Get encoder device statistics
*
*  @return  0 on success, otherwise -ve error
*/
static int vcpdev_enc_getstats(
   VCP_PIPE_HDL hdl,             /**< (i) Pipeline handle */
   int reset,                    /**< (i) 0=>do not reset stats after reading them, otherwise reset them */
   VCP_ENCODER_STATS *statsp,    /**< (o) Ptr to location to store statistics */
   void *devdata                 /**< (i) Private device data */
)
{
   struct vcpuser_device_node *nodep = devdata;
   struct vcpuser_pipe_node *pipep = hdl;
   struct vcpuser_msg_node *msgp;
   VCPMSG_GETENCSTATS *datap;
   VCPMSG_RESP resp;
   int err;

   VCP_LOG("hdl=0x%lx devdata=0x%lx\n", (unsigned long)hdl, (unsigned long) devdata );
   /* Allocate and queue message for user device */
   msgp = kmalloc( sizeof(*msgp), GFP_KERNEL );
   if ( msgp == NULL )
   {
      return -ENOMEM;
   }
   memset( msgp, 0, sizeof(*msgp) );
   datap = &msgp->payload.getencstats;

   datap->msgid = VCPMSG_GETENCSTATS_ID;
   datap->reset = reset;
   datap->hdl = pipep->user_pipehdl;
   datap->devdata = nodep->user_devdata;

   err = sendmsg( msgp, &nodep->msglist, &resp, &nodep->resplist, &nodep->msglock );
   if ( err )
   {
      printk( "%s: Failed sending msgid=%i to user device '%s'\n",
            __func__, datap->msgid, nodep->name );
      return err;
   }

   if ( resp.msgid != VCPMSG_GETENCSTATS_ID )
   {
      printk( "%s: Mismatched msgid! Got respid=%i!\n",
            __func__, resp.msgid );
      return -EINVAL;
   }

   if ( resp.rc == 0 )
   {
      /* Copy stats */
      memcpy( statsp, &resp.data.encstats, sizeof(*statsp) );
   }

   return resp.rc;
}

/***************************************************************************/
/**
*  Proc read callback function
*
*  @return  Number of characters to print
*/
static int read_proc( char *buf, char **start, off_t offset, int count,
   int *eof, void *data )
{
   struct vcpuser_device_node *nodep;
   struct vcpuser_pipe_node *pipep;
   int len = 0;

   len += sprintf( buf+len, "User-domain decoder devices: total=%i\n", gDecDeviceList.total );
   list_for_each_entry( nodep, &gDecDeviceList.list, lnode  )
   {
      len += sprintf( buf+len, " %s: user_devdata=0x%lx msgs=%u resps=%u frames=%u \n",
            nodep->name,
            (unsigned long)nodep->user_devdata, nodep->msglist.total,
            nodep->resplist.total, nodep->framelist.total );
   }

   len += sprintf( buf+len, "User-domain decoder pipelines: total=%i\n", gDecPipeList.total );
   list_for_each_entry( pipep, &gDecPipeList.list, lnode  )
   {
      len += sprintf( buf+len, " %s: active=%i user_pipehdl=0x%lx processedPkts=%i droppedPkts=%i\n",
            pipep->nodep->name, pipep->active, (unsigned long)pipep->user_pipehdl,
            pipep->processedPkts, pipep->droppedPkts );
   }

   len += sprintf( buf+len, "User-domain encoder devices: total=%i\n", gEncDeviceList.total );
   list_for_each_entry( nodep, &gEncDeviceList.list, lnode  )
   {
      len += sprintf( buf+len, " %s: user_devdata=0x%lx msgs=%u resps=%u \n",
            nodep->name,
            (unsigned long)nodep->user_devdata, nodep->msglist.total, nodep->resplist.total );
   }

   len += sprintf( buf+len, "User-domain encoder pipelines: total=%i\n", gEncPipeList.total );
   list_for_each_entry( pipep, &gEncPipeList.list, lnode  )
   {
      len += sprintf( buf+len, " %s: active=%i user_pipehdl=0x%lx processedPkts=%i droppedPkts=%i\n",
            pipep->nodep->name, pipep->active, (unsigned long)pipep->user_pipehdl,
            pipep->processedPkts, pipep->droppedPkts );
   }

   *eof = 1;
   return len+1 /* NULL character */;
}

/***************************************************************************/
/**
*  Initialize debug interfaces.
*
*  @return  none
*
*  @remarks
*/
static void debug_init( void )
{
   create_proc_read_entry( PROC_NAME, 0, NULL, read_proc, NULL );
}

/***************************************************************************/
/**
*  Cleanup debug interfaces.
*
*  @return  none
*
*  @remarks
*/
static void debug_exit( void )
{
   remove_proc_entry( PROC_NAME, NULL );
}

/***************************************************************************/
/**
*  Helper routine to initialize lists
*
*  @return  Nothing
*/
static void init_list(
   struct vcpdev_list *listp              /**< (io) Ptr to list to initialize */
)
{
   memset( listp, 0, sizeof(*listp) );
   INIT_LIST_HEAD( &listp->list );
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,37)
   init_MUTEX( &listp->mutex );
#else
   sema_init( &listp->mutex , 1 );
#endif
   sema_init( &listp->avail, 0 );
}

/***************************************************************************/
/**
*  Register a user proxy VCP decoder device.
*
*  @return  0  on success, -ve on error
*/
int vcpuser_register_dec_device(
   const char *devname,             /**< (i) Device name */
   VCP_DEC_DEVICE_OPS *ops,         /**< (i) Device callbacks */
   void *user_devdata,              /**< (i) User private device data */
   VCP_DEV_HDL *hdlp                /**< (o) Return handle to user proxy device */
)
{
   struct vcpuser_device_node *nodep;
   int err;

   nodep = kmalloc( sizeof(*nodep), GFP_KERNEL );
   if ( nodep == NULL )
   {
      return -ENOMEM;
   }
   memset( nodep, 0, sizeof(*nodep) );
   init_list( &nodep->msglist );
   init_list( &nodep->resplist );
   init_list( &nodep->framelist );
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,37)
   init_MUTEX( &nodep->msglock );
#else
   sema_init( &nodep->msglock , 1 );
#endif
   strncpy( nodep->name, devname, sizeof(nodep->name) );
   nodep->dir = VCP_DIR_DECODER;
   nodep->user_devdata = user_devdata;
   memcpy( &nodep->ops.dec, ops, sizeof(nodep->ops.dec) );

   /* Register proxy for the user-level device */
   err = vcp_register_dec_device( devname,
         &decOps, nodep /* devdata for kernel use */, &nodep->devhdl );
   if ( err )
   {
      kfree( nodep );
      return err;
   }

   down( &gDecDeviceList.mutex );
   list_add_tail( &nodep->lnode, &gDecDeviceList.list );
   gDecDeviceList.total++;
   up( &gDecDeviceList.mutex );

   *hdlp = nodep;

   return 0;
}

/***************************************************************************/
/**
*  Register a user proxy VCP encoder device.
*
*  @return  0  on success, -ve on error
*/
int vcpuser_register_enc_device(
   const char *devname,             /**< (i) Device name */
   VCP_ENC_DEVICE_OPS *ops,         /**< (i) Device callbacks */
   void *user_devdata,              /**< (i) User private device data */
   VCP_DEV_HDL *hdlp                /**< (o) Return handle to user proxy device */
)
{
   struct vcpuser_device_node *nodep;
   int err;

   nodep = kmalloc( sizeof(*nodep), GFP_KERNEL );
   if ( nodep == NULL )
   {
      return -ENOMEM;
   }
   memset( nodep, 0, sizeof(*nodep) );
   init_list( &nodep->msglist );
   init_list( &nodep->resplist );
   init_list( &nodep->framelist );
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,37)
   init_MUTEX( &nodep->msglock );
#else
   sema_init( &nodep->msglock , 1 );
#endif
   strncpy( nodep->name, devname, sizeof(nodep->name) );
   nodep->dir = VCP_DIR_ENCODER;
   nodep->user_devdata = user_devdata;
   memcpy( &nodep->ops.enc, ops, sizeof(nodep->ops.enc) );

   /* Register proxy for the user-level device */
   err = vcp_register_enc_device( devname,
         &encOps, nodep /* devdata for kernel use */, &nodep->devhdl );
   if ( err )
   {
      kfree( nodep );
      return err;
   }

   down( &gEncDeviceList.mutex );
   list_add_tail( &nodep->lnode, &gEncDeviceList.list );
   gEncDeviceList.total++;
   up( &gEncDeviceList.mutex );

   *hdlp = nodep;

   return 0;
}
/***************************************************************************/
/**
*  De-register a user-level video encoder or decoder device.
*
*  @return  0  on success, -ve on error
*/
int vcpuser_deregister_device(
   VCP_DEV_HDL hdl                  /**< (i) handle */
)
{
   struct vcpuser_device_node *nodep = hdl;
   int err;
   struct vcpdev_list         *listp;

   if ( nodep->dir == VCP_DIR_DECODER )
   {
      listp = &gDecDeviceList;
   }
   else
   {
      listp = &gEncDeviceList;
   }

   VCP_LOG("%s hdl=0x%lx \n", (nodep->dir == VCP_DIR_DECODER)?"decoder":"encoder", (unsigned long)hdl );

   err = vcp_deregister_device( nodep->devhdl );
   if ( err )
   {
      printk( "%s: failed to deregister proxy device, err=%i\n", __func__, err );
   }

   /* Dummy signals to unblock user threads and sync */
   up( &nodep->framelist.avail );
   up( &nodep->msglist.avail );

   down( &listp->mutex );
   list_del( &nodep->lnode );
   listp->total--;
   kfree( nodep );
   up( &listp->mutex );

   return err;
}

/***************************************************************************/
/**
*  Helper routine to dequeue decode frame
*
*  @return  0  on success, -ve on error
*
*/
static int dequeue_decframe(
   struct vcpdev_list *listp,          /* Ptr to decode frame list */
   struct framebuf_priv **fbnodepp     /* Ptr to return ptr to decode frame,
                                          can be NULL to discard frame */
)
{
   struct framebuf_priv *fbnodep;

   if ( list_empty( &listp->list ))
   {
      /* Empty frame. Probably signaling quit */
      return -EPERM;
   }

   /* Dequeue frame */
   down( &listp->mutex );
   fbnodep = list_entry( listp->list.next, struct framebuf_priv, lnode );
   list_del( &fbnodep->lnode );
   listp->total--;
   up( &listp->mutex );

   if ( fbnodepp )
   {
      *fbnodepp = fbnodep;
   }

   return 0;
}

/***************************************************************************/
/**
*  Dequeue a decoder frame. Blocks until a frame is available.
*
*  @return  0  on success, -ve on error
*
*  @remarks
*
*  This routine is expected to be called indirectly by the user
*  thread. The output frame buffer pointer is a user space
*  pointer where the frame data is returned.
*
*  There is a single queue for decoder frames per device.
*  Each decode frame is tagged with device and pipeline information
*  for routing.
*/
int vcpuser_get_decframe(
   VCP_DEV_HDL hdl,                 /**< (i) device handle */
   int max_pktsize,                 /**< (i) Max size to return */
   VCP_FRAMEBUF __user *userfbp,    /**< (o) User ptr to return frame buffer data */
   int __user *size_neededp         /**< (o) If insufficient memory, ptr to indicate memory needed */
)
{
   struct vcpuser_device_node *nodep = hdl;
   struct vcpdev_list *listp;
   struct framebuf_priv *fbnodep;
   struct vcpuser_pipe_node *pipep;
   VCP_FRAMEBUF *fbp;
   int err;
   int pktlen;

   listp = &nodep->framelist;

   /* Wait for available frame */
   err = down_interruptible( &listp->avail );
   if ( err < 0 )
   {
      return -ERESTARTSYS;
   }

   err = dequeue_decframe( listp, &fbnodep );
   if ( err )
   {
      VCP_LOG( "Error dequeuing decframe, probably signaling to quit!\n" );
      return -EPERM;
   }

   fbp = (void *)fbnodep;
   pktlen = fbp->hdr.data_len + sizeof(fbp->hdr);

   VCP_VERBOSE_LOG( "got frame: pktlen=%i", pktlen );

   if ( pktlen > max_pktsize )
   {
      /* Insufficient memory to hold payload, discard packet */
      VCP_LOG( "Insufficient memory! Realloc: pktlen=%i max_pktsize=%i fbnodep=0x%lx\n",
            pktlen, max_pktsize, (unsigned long)fbnodep );
      err = -ENOMEM;
      if ( size_neededp )
      {
         /* Report size needed */
         VCP_LOG( "size_neededp=0x%x pktlen=%i", (unsigned int)size_neededp, pktlen );
         copy_to_user( size_neededp, &pktlen, sizeof(pktlen) );
      }

      /* Re-enqueue packet to head of list to let user try again */
      down( &listp->mutex );
      list_add( &fbnodep->lnode, &listp->list );
      listp->total++;
      up( &listp->avail );
      up( &listp->mutex );
      return -ENOMEM;
   }

   /* Retrieve pipeline handle and Setup info for transfer to user domain */
   pipep = fbnodep->pipehdl;
   fbnodep->pipehdl = pipep->user_pipehdl;
   fbnodep->devdata = nodep->user_devdata;

   /* Copy frame to user pointer */
   err = copy_to_user( userfbp, fbp, pktlen );

   /* Free dequeued frame */
   pipep->processedPkts++;
   pipep->cbks.dec.free( fbp, pipep->privdata );

   return err;
}

/***************************************************************************/
/**
*  Discard next decoder frame. This is a blocking call.
*
*  @return  0  on success, -ve on error
*
*  @remarks
*
*  This routine is expected to be called indirectly by the user
*  thread.
*
*  There is a single queue for decoder frames per device.
*/
int vcpuser_discard_decframe(
   VCP_DEV_HDL hdl                  /**< (i) device handle */
)
{
   struct vcpuser_device_node *nodep = hdl;
   struct vcpuser_pipe_node *pipep;
   struct vcpdev_list *listp;
   struct framebuf_priv *fbnodep;
   int err;

   listp = &nodep->framelist;

   /* Wait for available frame */
   err = down_interruptible( &listp->avail );
   if ( err < 0 )
   {
      return -ERESTARTSYS;
   }

   /* Dequeue frame */
   err = dequeue_decframe( listp, &fbnodep );
   if ( err )
   {
      VCP_LOG( "failed to dequeue! err=%i", err );
      return err;
   }

   /* Discard frame */
   pipep = fbnodep->pipehdl;
   pipep->droppedPkts++;
   pipep->cbks.dec.free( (void *)fbnodep, pipep->privdata );
   VCP_LOG( "Discarded fbnodep=0x%lx", (unsigned long)fbnodep );

   return err;
}

/***************************************************************************/
/**
*  Dequeue a user message. Blocks until a message is available.
*
*  @return  0  on success, -ve on error
*
*  @remarks
*
*  This routine is expected to be called indirectly by the user
*  thread. The output frame buffer pointer is a user space
*  pointer where the frame data is returned.
*
*  There is a single queue for control messages frames per device. Message
*  contain device and pipeline information for routine.
*/
int vcpuser_get_ctrlmsg(
   VCP_DEV_HDL hdl,                 /**< (i) device handle */
   VCPMSG_ALL __user *userdatap     /**< (o) User ptr to return message */
)
{
   struct vcpuser_device_node *nodep = hdl;
   struct vcpdev_list *listp;
   struct vcpuser_msg_node *msgp;
   int err;

   listp = &nodep->msglist;

   /* Wait for available frame */
   err = down_interruptible( &listp->avail );
   if ( err < 0 )
   {
      return -ERESTARTSYS;
   }

   if ( list_empty( &listp->list ))
   {
      /* Empty frame. Probably signaling quit */
      VCP_LOG( "probably quitting!" );
      return -EPERM;
   }

   /* Dequeue frame */
   down( &listp->mutex );
   msgp = list_entry( listp->list.next, struct vcpuser_msg_node, lnode );
   list_del( &msgp->lnode );
   listp->total--;
   up( &listp->mutex );

   err = copy_to_user( userdatap, &msgp->payload, sizeof(*userdatap) );

   VCP_VERBOSE_LOG( "User got msg: msgid=%i", msgp->payload.base.msgid );

   /* Free packet */
   kfree( msgp );

   return err;
}

/***************************************************************************/
/**
*  Queue user response.
*
*  @return  0  on success, -ve on error
*
*  @remarks This routine is expected to be called indirectly by the user
*           thread.
*/
int vcpuser_put_response(
   VCP_DEV_HDL hdl,                 /**< (i) device handle */
   VCPMSG_RESP __user *userdatap    /**< (i) User ptr to return message */
)
{
   struct vcpuser_device_node *nodep = hdl;
   struct vcpuser_resp_node *respnodep;
   struct vcpdev_list *resplistp;
   int err;

   respnodep = kmalloc( sizeof(*respnodep), GFP_KERNEL );
   if ( respnodep == NULL )
   {
      return -ENOMEM;
   }
   memset( respnodep, 0, sizeof(*respnodep) );

   err = copy_from_user( &respnodep->data, userdatap, sizeof(respnodep->data) );
   if ( err )
   {
      /* Failed to copy data */
      VCP_DEBUG( "Failed to copy user data: userdatap=0x%lx\n", (unsigned long)userdatap );
      kfree( respnodep );
      return err;
   }

   resplistp = &nodep->resplist;

   /* Intercept and discard NULL messages used to sync cleanup */
   if ( respnodep->data.msgid == VCPMSG_NULL_ID )
   {
      VCP_DEBUG( "Discarding null message\n" );
      kfree( respnodep );
   }
   else
   {
      down( &resplistp->mutex );
      list_add_tail( &respnodep->lnode, &resplistp->list );
      resplistp->total++;
      up( &resplistp->mutex );
   }

   up( &resplistp->avail );

   VCP_VERBOSE_LOG( "Response queued: resplistp=0x%lx msgid=%i rc=%i\n",
         (unsigned long)resplistp, respnodep->data.msgid, respnodep->data.rc );

   return 0;
}

/***************************************************************************/
/**
*  Handle decoder events from user proxy device
*
*  @return  0  on success, -ve on error
*
*  @remarks This routine is expected to be called indirectly by the user
*           thread.
*/
int vcpuser_dec_eventcb(
   VCP_PIPE_HDL hdl,                /**< (i) handle */
   VCP_EVENT_CODE eventCode,        /**< (i) event code */
   void __user *userinfo,           /**< (i) User ptr to event info */
   void *devdata                    /**< (i) Private device data */
)
{
   struct vcpuser_pipe_node *pipep = hdl;
   (void)devdata;

   if ( pipep->cbks.dec.dec_event )
   {
      void *infop = NULL;
      int err, parmsize;
      union
      {
         VCP_ERR_RETCODE retcode;
         VCP_ERR_DEC_CORRUPTMB corruptmb;
         VCP_DEC_BFRAME_RECV bframe;
         VCP_DEC_STREAM_INFO streaminfo;
      }
      parm;

      switch ( eventCode )
      {
         case VCP_EVENT_DEC_UNSUPPORTED:
         case VCP_EVENT_DEC_OUT_OF_MEM:
         case VCP_EVENT_DEC_STRM_PROCESS_ERR:
         case VCP_EVENT_DEC_QUEUE_ERR:
         case VCP_EVENT_DEC_PIPELINE_FLUSH_ERR:
            parmsize = sizeof(parm.retcode);
            break;
         case VCP_EVENT_DEC_MB_CORRUPTION:
            parmsize = sizeof(parm.corruptmb);
            break;
         case VCP_EVENT_DEC_B_FRAME_RECEIVED:
            parmsize = sizeof(parm.bframe);
            break;
         case VCP_EVENT_DEC_STREAM_INFO:
            parmsize = sizeof(parm.streaminfo);
            break;
         default:
            /* For any other event, just assume max parameter size */
            parmsize = sizeof(parm);
      }

      if ( parmsize && userinfo )
      {
         err = copy_from_user( &parm, userinfo, parmsize );
         if ( err )
         {
            VCP_DEBUG( "Failed to copy user data: userinfo=0x%lx\n", (unsigned long)userinfo );
            return err;
         }
         infop = &parm;
      }

      pipep->cbks.dec.dec_event( eventCode, infop, pipep->privdata );
   }

   return 0;
}

/***************************************************************************/
/**
*  Handle encoder events from user proxy device
*
*  @return  0  on success, -ve on error
*
*  @remarks This routine is expected to be called indirectly by the user
*           thread.
*/
int vcpuser_enc_eventcb(
   VCP_PIPE_HDL hdl,                /**< (i) handle */
   VCP_EVENT_CODE eventCode,        /**< (i) event code */
   void __user *userinfo,           /**< (i) User ptr to event info */
   void *devdata                    /**< (i) Private device data */
)
{
   struct vcpuser_pipe_node *pipep = hdl;
   (void)devdata;

   if ( pipep->cbks.enc.enc_event )
   {
      void *infop = NULL;
      int err, parmsize;

      union
      {
         VCP_ERR_RETCODE retcode;
#if 0 /* enable this once VCP_ENC_STREAM_INFO is supported */
         VCP_ENC_STREAM_INFO streaminfo;
#endif
      }
      parm;

      switch ( eventCode )
      {
         case VCP_EVENT_ENC_UNSUPPORTED:
            parmsize = sizeof(parm.retcode);
            break;
         case VCP_EVENT_ENC_STREAM_INFO:
#if 0 /* enable this once VCP_ENC_STREAM_INFO is supported */
            parmsize = sizeof(parm.streaminfo);
#else
            parmsize = 0;
#endif
            break;
         default:
            /* For any other event, just assume max parameter size */
            parmsize = sizeof(parm);
      }

      if ( parmsize && userinfo )
      {
         err = copy_from_user( &parm, userinfo, parmsize );
         if ( err )
         {
            VCP_DEBUG( "Failed to copy user data: userinfo=0x%lx\n", (unsigned long)userinfo );
            return err;
         }
         infop = &parm;
      }
      pipep->cbks.enc.enc_event( eventCode, infop, pipep->privdata );
   }

   return 0;
}

/***************************************************************************/
/**
*  Handle encoder frames from user proxy device
*
*  @return  0  on success, -ve on error
*
*  @remarks This routine is expected to be called indirectly by the user
*           thread.
*/
int vcpuser_enc_frame(
   VCP_PIPE_HDL hdl,                /**< (i) handle */
   int bytesize,                    /**< (i) frame size in bytes */
   void __user *userdatap,          /**< (i) User ptr to frame */
   void *devdata                    /**< (i) Private device data */
)
{
   VCP_FRAMEBUF *frame_kernel;
   int err;
   struct vcpuser_pipe_node *pipep = hdl;
   (void)devdata;

   if ( pipep->cbks.enc.enqueue && pipep->cbks.enc.alloc )
   {
      frame_kernel = pipep->cbks.enc.alloc( bytesize, pipep->privdata );

      if( frame_kernel == NULL )
      {
         pipep->droppedPkts++;
         VCP_LOG("alloc failed\n");
         return -ENOMEM;
      }

      err = copy_from_user( frame_kernel, userdatap, bytesize );
      if ( err )
      {
         /* copy_from_user failed.  We consider this a dropped packet.
          * Enqueue the packet with data_len=0.  The VCP host must
          * deallocate the memory it provided in the previous
          * pipep->cbks.enc.alloc call. */
         frame_kernel->hdr.data_len = 0;

         pipep->cbks.enc.enqueue( frame_kernel, pipep->privdata );

         pipep->droppedPkts++;
         VCP_LOG( "Failed to copy user data=0x%lx bytesize=%d\n", (unsigned long)userdatap, bytesize );
      }
      else
      {
         err = pipep->cbks.enc.enqueue( frame_kernel, pipep->privdata );
         if ( err )
         {
            /* This error condition should never occur.  If it does we assume
             * the enqueue function will free the buffer */

            pipep->droppedPkts++;
            VCP_LOG( "Failed to enqueue.  err=%d\n", err );
         }
         else
         {
            pipep->processedPkts++;
         }
      }
   }
   else
   {
      VCP_LOG("enqueue or alloc callback is undefined\n");
      err = -EPERM;
   }
   return err;
}
/***************************************************************************/
/**
*  User proxy layer constructor
*
*  @return  0  on success, -ve on error
*/
int vcpuser_constructor( void )
{
   VCP_FRAMEBUF_HDR *hdrp;

   /* Sanity check structures */
   if ( sizeof(struct framebuf_priv) > sizeof(hdrp->scratch) )
   {
      printk( "%s: Fatal error: sizeof framebuf_priv (%d) greater than VCPFB scratch mem (%d)\n",
            __func__, sizeof(struct framebuf_priv), sizeof(hdrp->scratch) );
      return -EPERM;
   }

   init_list( &gDecDeviceList );
   init_list( &gDecPipeList );
   init_list( &gEncDeviceList );
   init_list( &gEncPipeList );

   debug_init();

   return 0;
}

/***************************************************************************/
/**
*  User proxy layer destructor
*
*  @return  0  on success, -ve on error
*/
int vcpuser_destructor( void )
{
   struct vcpuser_device_node *nodep, *tmpnodep;

   debug_exit();

   list_for_each_entry_safe( nodep, tmpnodep, &gDecDeviceList.list, lnode )
   {
      vcpuser_deregister_device( nodep );
   }
   list_for_each_entry_safe( nodep, tmpnodep, &gEncDeviceList.list, lnode )
   {
      vcpuser_deregister_device( nodep );
   }

   return 0;
}
