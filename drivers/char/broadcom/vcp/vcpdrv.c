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
*  @file    vcpdrv.c
*
*  @brief   This file implements the Video Conference Pipeline (VCP)
*           kernel driver.
*
*****************************************************************************/

/* ---- Include Files ---------------------------------------------------- */

#include <linux/types.h>                     /* For stdint types: uint8_t, etc. */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/proc_fs.h>                   /* For proc_fs files */
#include <linux/signal.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/string.h>
#include <linux/vmalloc.h>                   /* For memory alloc */
#include <linux/sysctl.h>
#include <linux/version.h>

#include <linux/semaphore.h>                 /* For down_interruptible, up, etc. */
#include <linux/list.h>                      /* Linked list */

#include <linux/broadcom/knllog.h>           /* For debugging */
#include <linux/broadcom/vcp.h>              /* VCP API */
#include <linux/broadcom/vcp_device.h>       /* VCP Device API */
#include <linux/broadcom/vcp_ioctl.h>        /* IOCTL API */
#include <asm/io.h>
#include <asm/uaccess.h>                     /* User access routines */
#include <asm/atomic.h>                      /* Atomic operations */

#include "vcppriv.h"

/* ---- Public Variables ------------------------------------------------- */
/* ---- Private Constants and Types -------------------------------------- */

/* Generic list structure */
struct vcp_list
{
   struct list_head           list;          /* List head */
   struct semaphore           mutex;         /* Protect list */
   int                        total;
};

#define BASE_CLASS_DEVICE_NODE \
   struct list_head           lnode;         /* List node */ \
   char                       name[32];      /* Name of device */ \
   VCP_DIR                    dir;           /* Encoder or decoder */ \
   void                      *devdata        /* Private device data */

/* Base class video device node */
struct vcp_base_device_node
{
   BASE_CLASS_DEVICE_NODE;                   /* Must be first. DO NOT CHANGE! */
};

/* Video encoder device node, derived  */
struct vcp_enc_device_node
{
   BASE_CLASS_DEVICE_NODE;                   /* Must be first. DO NOT CHANGE! */
   VCP_ENC_DEVICE_OPS         ops;           /* Device operations */
};

/* Video decoder device node */
struct vcp_dec_device_node
{
   BASE_CLASS_DEVICE_NODE;                   /* Must be first. DO NOT CHANGE! */
   VCP_DEC_DEVICE_OPS         ops;           /* Device operations */
};

#define BASE_CLASS_USER_NODE \
   struct list_head           lnode;         /* List node */ \
   VCP_DIR                    dir            /* Encoder or decoder */

/* Base class user node */
struct vcp_base_user_node
{
   BASE_CLASS_USER_NODE;                     /* Must be first. DO NOT CHANGE! */
};

/* Encoder user node */
struct vcp_enc_user_node
{
   BASE_CLASS_USER_NODE;                     /* Must be first. DO NOT CHANGE! */
   VCP_HOST_ENC_CBKS          callbacks;     /* User callbacks */
   void                       *privdata;     /* User private data */
   struct vcp_enc_device_node *devnodep;     /* Associated device */
   VCP_PIPE_HDL               pipehdl;       /* Pipeline handle */
};

/* Decoder user node */
struct vcp_dec_user_node
{
   BASE_CLASS_USER_NODE;                     /* Must be first. DO NOT CHANGE! */
   VCP_HOST_DEC_CBKS          callbacks;     /* User callbacks */
   void                       *privdata;     /* User private data */
   struct vcp_dec_device_node *devnodep;     /* Associated device */
   VCP_PIPE_HDL               pipehdl;       /* Pipeline handle */
};

/**
*  Union of all the different IOCTL parameter structures to determine
*  max stack variable size
*/
union vcp_ioctl_params
{
   struct vcpioctl_register_dec_data      register_dec;
   struct vcpioctl_register_enc_data      register_enc;
   struct vcpioctl_get_decframe_data      get_decframe;
   struct vcpioctl_get_ctrlmsg_data       get_ctrlmsg;
   struct vcpioctl_put_response_data      put_response;
   struct vcpioctl_dec_eventcb            dec_event;
   struct vcpioctl_enc_eventcb            enc_event;
   struct vcpioctl_encframe_data          enc_frame;
};

/* Procfs file name */
#define VCP_PROC_NAME               "vcp"

/* ---- Private Variables ------------------------------------------------ */

static int gDriverMajor;
static struct class *vcp_class;
static struct device *vcp_dev;

/* Driver start up banner string */
static char banner[] __initdata = KERN_INFO "VCP Driver: 1.0\n";

/* Lists */
static struct vcp_list              gUserList;
static struct vcp_list              gEncDeviceList;
static struct vcp_list              gDecDeviceList;

/* Debug print level */
int gDbgPrintLevel;

/* Number of user devices */
static int                          gOpenCount;

/**
*  SysCtl data structures
*/
static struct ctl_table gSysCtlChild[] =
{
   {
      .procname      = "dbgprint",
      .data          = &gDbgPrintLevel,
      .maxlen        = sizeof( int ),
      .mode          = 0644,
      .proc_handler  = &proc_dointvec,
   },
   {}
};
static struct ctl_table gSysCtl[] =
{
   {
      .procname      = "vcp",
      .mode          = 0555,
      .child         = gSysCtlChild
   },
   {}
};
static struct ctl_table_header  *gSysCtlHeader;

/* ---- Private Function Prototypes -------------------------------------- */
static void    debug_init( void );
static void    debug_exit( void );

static int     vcp_open( struct inode *inode, struct file *file );
static int     vcp_release( struct inode *inode, struct file *file );
static long    vcp_ioctl( struct file *file, unsigned int cmd, unsigned long arg );

/* ---- Functions -------------------------------------------------------- */

/**
*  File Operations (these are the device driver entry points)
*/
static struct file_operations gfops =
{
   .owner          = THIS_MODULE,
   .open           = vcp_open,
   .release        = vcp_release,
   .unlocked_ioctl = vcp_ioctl,
};

/***************************************************************************/
/**
*  Helper routine to search for a device. The default device returns
*  the first one matching the direction/type in the list
*
*  @return  1 if found, 0 if not found
*/
static int search_device(
   const char *devname,                /**< (i) Device name */
   VCP_DIR dir,                        /**< (i) Encoder or decoder */
   struct vcp_base_device_node **devpp /**< (i) Ptr to device node pointer */
)
{
   struct vcp_base_device_node *devnodep;
   struct vcp_list *listp;
   int usedefault, found;

   usedefault = strcmp( devname, "default" ) == 0;
   found = 0;
   listp = ( dir == VCP_DIR_ENCODER ) ?  &gEncDeviceList : &gDecDeviceList;

   list_for_each_entry( devnodep, &listp->list, lnode )
   {
      if ( usedefault )
      {
         /* Return first matching device as the default device */
         found = 1;
         break;
      }
      else if ( strncmp( devname, devnodep->name, sizeof(devnodep->name) ) == 0 )
      {
         found = 1;
         break;
      }
   }
   if ( !found )
   {
      return 0;
   }

   *devpp = devnodep;
   return 1;
}

/***************************************************************************/
/**
*  Open a VCP encoder channel on a particular video hardware device.
*  The "default" device is the first one in the list of installed devices.
*
*  @return  0 on success, otherwise -ve error. If success, a VCP
*           handle is returned in handlep pointer.
*/
int vcp_open_encoder(
   const char *devname,             /**< (i) Device name */
   VCP_HOST_ENC_CBKS *cbksp,        /**< (i) Ptr to host callbacks */
   void *privdata,                  /**< (i) Priv data returned in callbacks */
   VCP_HDL *handlep                 /**< (o) If success, valid handle */
)
{
   struct vcp_enc_user_node *userp;
   struct vcp_enc_device_node *devnodep;
   VCP_PIPE_HDL pipehdl;
   int err;

   if ( search_device( devname, VCP_DIR_ENCODER, (struct vcp_base_device_node **)&devnodep ) == 0 )
   {
      /* Device not found */
      return -EINVAL;
   }

   /* Open device */
   err = devnodep->ops.open( cbksp, privdata, &pipehdl, devnodep->devdata );
   if ( err )
   {
      return err;
   }

   /* Create node for user */
   userp = kmalloc( sizeof(*userp), GFP_KERNEL );
   if ( userp == NULL )
   {
      return -ENOMEM;
   }
   memset( userp, 0, sizeof(*userp) );
   memcpy( &userp->callbacks, cbksp, sizeof(userp->callbacks) );
   userp->privdata = privdata;
   userp->devnodep = devnodep;
   userp->pipehdl = pipehdl;
   userp->dir = VCP_DIR_ENCODER;

   down( &gUserList.mutex );
   list_add_tail( &userp->lnode, &gUserList.list );
   gUserList.total++;
   up( &gUserList.mutex );

   *handlep = userp;
   return 0;
}
EXPORT_SYMBOL( vcp_open_encoder );

/***************************************************************************/
/**
*  Open a VCP decoder channel on a particular video hardware device.
*  The "default" device is the first one in the list of installed devices.
*
*  @return  0 on success, otherwise -ve error. If success, a VCP
*           handle is returned in handlep pointer.
*/
int vcp_open_decoder(
   const char *devname,             /**< (i) Device name */
   VCP_HOST_DEC_CBKS *cbksp,        /**< (i) Host callbacks */
   void *privdata,                  /**< (i) Priv data returned in callbacks */
   VCP_HDL *handlep                 /**< (o) If success, valid handle */
)
{
   struct vcp_dec_user_node *userp;
   struct vcp_dec_device_node *devnodep;
   VCP_PIPE_HDL pipehdl;
   int err;

   if ( search_device( devname, VCP_DIR_DECODER, (struct vcp_base_device_node **)&devnodep ) == 0 )
   {
      /* Device not found */
      return -EINVAL;
   }

   /* Open device */
   err = devnodep->ops.open( cbksp, privdata, &pipehdl, devnodep->devdata );
   if ( err )
   {
      return err;
   }

   /* Create user node */
   userp = kmalloc( sizeof(*userp), GFP_KERNEL );
   if ( userp == NULL )
   {
      return -ENOMEM;
   }
   memset( userp, 0, sizeof(*userp) );
   memcpy( &userp->callbacks, cbksp, sizeof(userp->callbacks) );
   userp->privdata = privdata;
   userp->devnodep = devnodep;
   userp->pipehdl = pipehdl;
   userp->dir = VCP_DIR_DECODER;

   down( &gUserList.mutex );
   list_add_tail( &userp->lnode, &gUserList.list );
   gUserList.total++;
   up( &gUserList.mutex );

   *handlep = userp;
   return 0;
}
EXPORT_SYMBOL( vcp_open_decoder );

/***************************************************************************/
/**
*  Helper routine to check whether the handle is valid
*  channel.
*
*  @return  1 if handle is valid, otherwise 0.
*/
static int is_valid_handle(
   VCP_HDL handle                   /**< (i) Handle */
)
{
   struct vcp_base_user_node *nodep;
   int found = 0;

   down( &gUserList.mutex );
   list_for_each_entry( nodep, &gUserList.list, lnode )
   {
      if ( nodep == handle )
      {
         found = 1;
         break;
      }
   }
   up( &gUserList.mutex );

   return found;
}

/***************************************************************************/
/**
*  Helper routine to check whether the handle corresponds to an encoder
*  channel.
*
*  @return  1 if handle is for the encoder, otherwise 0.
*/
static inline int is_enc_handle(
   VCP_HDL handle                   /**< (i) Handle */
)
{
   struct vcp_base_user_node *basenodep = handle;
   return ( basenodep->dir == VCP_DIR_ENCODER );
}

/***************************************************************************/
/**
*  Close a VCP channel, either encoder or decoder
*
*  @return  0 on success, otherwise -ve error. If success,
*           handle is returned in handlep pointer.
*/
int vcp_close(
   VCP_HDL   handle                 /**< (i) Handle */
)
{
   struct vcp_base_user_node *baseuserp = handle;
   int err;

   if ( !is_valid_handle( handle ))
   {
      return -EINVAL;
   }

   if ( is_enc_handle( handle ))
   {
      struct vcp_enc_user_node *encuserp = handle;
      struct vcp_enc_device_node *devnodep = encuserp->devnodep;

      /* close device */
      err = devnodep->ops.close( encuserp->pipehdl, devnodep->devdata );
      if ( err )
      {
         printk( "%s: failed to close encoder device (%s), err=%i\n",
               __func__, encuserp->devnodep->name, err );
      }
   }
   else
   {
      struct vcp_dec_user_node *decuserp = handle;
      struct vcp_dec_device_node *devnodep = decuserp->devnodep;

      /* close device */
      err = devnodep->ops.close( decuserp->pipehdl, devnodep->devdata );
      if ( err )
      {
         printk( "%s: failed to close decoder device (%s), err=%i\n",
               __func__, decuserp->devnodep->name, err );
      }
   }

   /* Cleanup */
   down( &gUserList.mutex );
   list_del( &baseuserp->lnode );
   gUserList.total--;
   up( &gUserList.mutex );

   kfree( baseuserp );

   return err;
}
EXPORT_SYMBOL( vcp_close );

/***************************************************************************/
/**
*  Deliver video data to be decoded by VCP. The memory is held by VCP and
*  later freed by calling the free_decframe callback.
*
*  @return  0 on success, otherwise -ve error
*
*  @remark  This routine is non-blocking.
*/
int vcp_decode_frame(
   VCP_HDL handle,                  /**< (i) Handle to VCP channel */
   VCP_FRAMEBUF *framebufp          /**< (i) Ptr to frame buffer to decode */
)
{
   struct vcp_dec_user_node *decuserp = handle;
   struct vcp_dec_device_node *devnodep = decuserp->devnodep;
   int err;

#if 0
   /* Skip check to save cycles */
   if ( !is_valid_handle( handle ))
   {
      return -EINVAL;
   }
#endif

   err = decuserp->devnodep->ops.decode( decuserp->pipehdl, framebufp,
         devnodep->devdata );
   if ( err )
   {
      printk( "%s: failed to decode frame with device (%s), err=%i\n",
            __func__, decuserp->devnodep->name, err );
      return err;
   }

   return 0;
}
EXPORT_SYMBOL( vcp_decode_frame );

/***************************************************************************/
/**
*  Start the video pipeline
*
*  @return  0 on success, otherwise -ve error
*/
int vcp_start(
   VCP_HDL handle                   /**< (i) Handle to VCP channel */
)
{
   int err;

   if ( !is_valid_handle( handle ))
   {
      return -EINVAL;
   }

   if ( is_enc_handle( handle ))
   {
      struct vcp_enc_user_node *encuserp = handle;
      struct vcp_enc_device_node *devnodep = encuserp->devnodep;

      /* Start device */
      err = devnodep->ops.start( encuserp->pipehdl, devnodep->devdata );
      if ( err )
      {
         printk( "%s: failed to start encoder device (%s), err=%i\n",
               __func__, encuserp->devnodep->name, err );
      }
   }
   else
   {
      struct vcp_dec_user_node *decuserp = handle;
      struct vcp_dec_device_node *devnodep = decuserp->devnodep;

      /* Start device */
      err = devnodep->ops.start( decuserp->pipehdl, devnodep->devdata );
      if ( err )
      {
         printk( "%s: failed to start decoder device (%s), err=%i\n",
               __func__, decuserp->devnodep->name, err );
      }
   }

   return err;
}
EXPORT_SYMBOL( vcp_start );

/***************************************************************************/
/**
*  Stop the video pipeline
*
*  @return  0 on success, otherwise -ve error
*/
int vcp_stop(
   VCP_HDL handle                   /**< (i) Handle to VCP channel */
)
{
   int err;

   if ( !is_valid_handle( handle ))
   {
      return -EINVAL;
   }

   if ( is_enc_handle( handle ))
   {
      struct vcp_enc_user_node *encuserp = handle;
      struct vcp_enc_device_node *devnodep = encuserp->devnodep;

      /* Stop device */
      err = devnodep->ops.stop( encuserp->pipehdl, devnodep->devdata );
      if ( err )
      {
         printk( "%s: failed to stop encoder device (%s), err=%i\n",
               __func__, encuserp->devnodep->name, err );
      }
   }
   else
   {
      struct vcp_dec_user_node *decuserp = handle;
      struct vcp_dec_device_node *devnodep = decuserp->devnodep;

      /* Stop device */
      err = devnodep->ops.stop( decuserp->pipehdl, devnodep->devdata );
      if ( err )
      {
         printk( "%s: failed to stop decoder device (%s), err=%i\n",
               __func__, decuserp->devnodep->name, err );
      }
   }

   return err;
}
EXPORT_SYMBOL( vcp_stop );

/***************************************************************************/
/**
*  Set video pipeline parameters
*
*  @return  0 on success, otherwise -ve error
*/
int vcp_set_parameter(
   VCP_HDL handle,                  /**< (i) Handle to VCP channel */
   VCP_PARM_INDEX index,            /**< (i) Parameter index */
   const void *datap                /**< (i) Ptr to command related data */
)
{
   int err = -EPERM;

   if ( !is_valid_handle( handle ))
   {
      return -EINVAL;
   }

   if ( is_enc_handle( handle ))
   {
      struct vcp_enc_user_node *encuserp = handle;
      struct vcp_enc_device_node *devnodep = encuserp->devnodep;

      if ( devnodep->ops.setparm )
      {
         err = devnodep->ops.setparm( encuserp->pipehdl, index, datap, devnodep->devdata );
      }
   }
   else
   {
      struct vcp_dec_user_node *decuserp = handle;
      struct vcp_dec_device_node *devnodep = decuserp->devnodep;

      if ( devnodep->ops.setparm )
      {
         err = devnodep->ops.setparm( decuserp->pipehdl, index, datap, devnodep->devdata );
      }
   }

   return err;
}
EXPORT_SYMBOL( vcp_set_parameter );

/***************************************************************************/
/**
*  Get video pipeline parameters
*
*  @return  0 on success, otherwise -ve error
*/
int vcp_get_parameter(
   VCP_HDL handle,                  /**< (i) Handle to VCP channel */
   VCP_PARM_INDEX index,            /**< (i) Parameter index */
   void *datap                      /**< (o) Ptr to config data */
)
{
   int err = -EPERM;

   if ( !is_valid_handle( handle ))
   {
      return -EINVAL;
   }

   if ( is_enc_handle( handle ))
   {
      struct vcp_enc_user_node *encuserp = handle;
      struct vcp_enc_device_node *devnodep = encuserp->devnodep;

      if ( devnodep->ops.getparm )
      {
         err = devnodep->ops.getparm( encuserp->pipehdl, index, datap, devnodep->devdata );
      }
   }
   else
   {
      struct vcp_dec_user_node *decuserp = handle;
      struct vcp_dec_device_node *devnodep = decuserp->devnodep;

      if ( devnodep->ops.getparm )
      {
         err = devnodep->ops.getparm( decuserp->pipehdl, index, datap, devnodep->devdata );
      }
   }

   return err;
}
EXPORT_SYMBOL( vcp_get_parameter );

/***************************************************************************/
/**
*  Query video pipeline capabilities
*
*  @return  0 on success, otherwise -ve error
*/
int vcp_get_capabilities(
   VCP_HDL handle,                  /**< (i) Handle to VCP channel */
   VCP_CAPABILITIES *cap            /**< (o) Ptr to capabilities data */
)
{
   int err = -EPERM;

   if ( !is_valid_handle( handle ))
   {
      return -EINVAL;
   }

   if ( is_enc_handle( handle ))
   {
      struct vcp_enc_user_node *encuserp = handle;
      struct vcp_enc_device_node *devnodep = encuserp->devnodep;

      if ( devnodep->ops.getcap )
      {
         err = devnodep->ops.getcap( encuserp->pipehdl, cap, devnodep->devdata );
      }
   }
   else
   {
      struct vcp_dec_user_node *decuserp = handle;
      struct vcp_dec_device_node *devnodep = decuserp->devnodep;

      if ( devnodep->ops.getcap )
      {
         err = devnodep->ops.getcap( decuserp->pipehdl, cap, devnodep->devdata );
      }
   }

   return err;
}
EXPORT_SYMBOL( vcp_get_capabilities );

/***************************************************************************/
/**
*  Query for installed video devices by numerical index. 0 would be the
*  first device, 1 would be the second device, and so on.
*
*  @return  0  on success
*           -1 Non-existent device
*/
int vcp_query_devname(
   char *devname,                   /**< (o) Ptr to store device name */
   int namelen,                     /**< (i) Max string length to output */
   VCP_DIR dir,                     /**< (i) Encoder or decoder device */
   int index                        /**< (i) 0-indexed number */
)
{
   struct vcp_base_device_node *devnodep;
   struct vcp_list *listp;
   int retval, count;

   retval = -1;
   count = 0;
   listp = ( dir == VCP_DIR_ENCODER ) ?  &gEncDeviceList : &gDecDeviceList;

   down( &listp->mutex );
   list_for_each_entry( devnodep, &listp->list, lnode )
   {
      if ( index == count )
      {
         /* Match found */
         if ( namelen > sizeof(devnodep->name) )
         {
            namelen = sizeof(devnodep->name);
         }
         strncpy( devname, devnodep->name, namelen );
         retval = 0;
         break;
      }
      count++;
   }
   up( &listp->mutex );

   return retval;
}
EXPORT_SYMBOL( vcp_query_devname );

/***************************************************************************/
/**
*  Query video encoder statistics
*
*  @return  0 on success, otherwise -ve error
*/
int vcp_get_encoder_stats(
   VCP_HDL handle,                  /**< (i) Handle to VCP channel */
   int  reset,                      /**< (i) 0=>do not reset stats after reading them, otherwise reset them */
   VCP_ENCODER_STATS *stats         /**< (o) Ptr to statistics */
)
{
   int err = -EPERM;

   if ( !is_valid_handle( handle ))
   {
      return -EINVAL;
   }

   if ( is_enc_handle( handle ))
   {
      struct vcp_enc_user_node *encuserp = handle;
      struct vcp_enc_device_node *devnodep = encuserp->devnodep;

      if ( devnodep->ops.getencstats )
      {
         err = devnodep->ops.getencstats( encuserp->pipehdl, reset, stats, devnodep->devdata );
      }
   }
   else
   {
      return -EINVAL;
   }

   return err;
}
EXPORT_SYMBOL( vcp_get_encoder_stats);

/***************************************************************************/
/**
*  Query video decoder statistics
*
*  @return  0 on success, otherwise -ve error
*/
int vcp_get_decoder_stats(
   VCP_HDL handle,                  /**< (i) Handle to VCP channel */
   int  reset,                      /**< (i) 0=>do not reset stats after reading them, otherwise reset them */
   VCP_DECODER_STATS *stats         /**< (o) Ptr to statistics */
)
{
   int err = -EPERM;

   if ( !is_valid_handle( handle ))
   {
      return -EINVAL;
   }

   if ( !is_enc_handle( handle ))
   {
      struct vcp_dec_user_node *decuserp = handle;
      struct vcp_dec_device_node *devnodep = decuserp->devnodep;

      if ( devnodep->ops.getdecstats )
      {
         err = devnodep->ops.getdecstats( decuserp->pipehdl, reset, stats, devnodep->devdata );
      }
   }
   else
   {
      return -EINVAL;
   }

   return err;
}
EXPORT_SYMBOL( vcp_get_decoder_stats);

/***************************************************************************/
/**
*  set the conference parameter
*
*  @return  0  on success, -ve on error
*/
int vcp_set_conference_param(
   VCP_HDL handle,                  /**< (i) Handle to VCP channel */
   VCP_CONFERENCE_PARM_INDEX index, /**< (i) Parameter index */
   const void *datap                /**< (i) Ptr to command related data */
)
{
   int err = -EPERM;
   if( !is_valid_handle( handle ) )
   {
      return -EINVAL;
   }
   if ( is_enc_handle( handle ))
   {
      struct vcp_enc_user_node *encuserp = handle;
      struct vcp_enc_device_node *devnodep = encuserp->devnodep;

      if ( devnodep->ops.setconference )
      {
         err = devnodep->ops.setconference( encuserp->pipehdl, index, datap, devnodep->devdata );
      }
   }
   else
   {
      struct vcp_dec_user_node *decuserp = handle;
      struct vcp_dec_device_node *devnodep = decuserp->devnodep;

      if ( devnodep->ops.setconference )
      {
         err = devnodep->ops.setconference( decuserp->pipehdl, index, datap, devnodep->devdata );
      }
   }

   return err;
}
EXPORT_SYMBOL( vcp_set_conference_param );

/***************************************************************************/
/**
*  get the conference parameter
*
*  @return  0  on success, -ve on error
*/
int vcp_get_conference_param(
   VCP_HDL handle,                  /**< (i) Handle to VCP channel */
   VCP_CONFERENCE_PARM_INDEX index, /**< (i) Parameter index */
   const void *datap                /**< (i) Ptr to command related data */
)
{
   int err = -EPERM;
   if( !is_valid_handle( handle ) )
   {
      return -EINVAL;
   }
   if ( is_enc_handle( handle ))
   {
      struct vcp_enc_user_node *encuserp = handle;
      struct vcp_enc_device_node *devnodep = encuserp->devnodep;

      if ( devnodep->ops.getconference )
      {
         err = devnodep->ops.getconference( encuserp->pipehdl, index, datap, devnodep->devdata );
      }
   }
   else
   {
      struct vcp_dec_user_node *decuserp = handle;
      struct vcp_dec_device_node *devnodep = decuserp->devnodep;

      if ( devnodep->ops.getconference )
      {
         err = devnodep->ops.getconference( decuserp->pipehdl, index, datap, devnodep->devdata );
      }
   }

   return err;
}
EXPORT_SYMBOL( vcp_get_conference_param );

/***************************************************************************/
/**
*  add a decoder channel as part of a (3 way) video conference
*
*  @return  0  on success, -ve on error
*/
int vcp_add_conference(
   VCP_HDL dec_hdl,                  /**< (i) Handle to VCP dec channel */
   VCP_HDL enc_hdl                   /**< (i) Handle to VCP enc channel */
)
{
   int err = -EPERM;
   printk( "vcp_add_conference 0x%x 0x%x\n", (unsigned int)dec_hdl, (unsigned int)enc_hdl);
   if( !is_valid_handle( enc_hdl ) || !is_valid_handle( dec_hdl ) )
   {
      printk("invalid handle!\n");
      return -EINVAL;
   }
   /* must be valid encoder and decoder handle */
   if( is_enc_handle( enc_hdl ) && !is_enc_handle( dec_hdl ) )
   {
      struct vcp_enc_user_node *encuserp = enc_hdl;
      struct vcp_dec_user_node *decuserp = dec_hdl;

      struct vcp_enc_device_node *encdevnodep = encuserp->devnodep;
      struct vcp_dec_device_node *decdevnodep = decuserp->devnodep;

      if( (encdevnodep->ops.setconference) && (encdevnodep->ops.getconference) &&
          (decdevnodep->ops.setconference) )
      {
         VCP_ENCODE_CONFERENCE enc_conf;
         VCP_ENCODE_GET_MIXER get_mixer;
         VCP_DECODE_CONFERENCE dec_conf;

         /* add the decoder as part of the conference call */
         enc_conf.dec_hdl = dec_hdl;
         err = encdevnodep->ops.setconference( encuserp->pipehdl,
               VCP_ENC_ADD_CONFERENCE, &enc_conf, encdevnodep->devdata );

         if( err == 0 )
         {
            /* retrieve the details on the mixer component needed by the decoder */
            get_mixer.dec_hdl = dec_hdl;
            err = encdevnodep->ops.getconference( encuserp->pipehdl,
                  VCP_ENC_GET_MIX_HDL, &get_mixer, encdevnodep->devdata );
         }
         if( err == 0 )
         {
            printk("calling decoder set conference\n");
            /* enable 3 party conference in the decoder */
            dec_conf.enable = 1;
            dec_conf.video_mixer = get_mixer.video_mixer;
            dec_conf.port = get_mixer.port;
            err = decdevnodep->ops.setconference( decuserp->pipehdl,
                  VCP_DEC_ENABLE_CONFERENCE, &dec_conf, decdevnodep->devdata );
         }
      }
   }
   else
   {
      printk("failure\n");
   }

   return err;   
}
EXPORT_SYMBOL( vcp_add_conference );

/***************************************************************************/
/**
*  remove a decoder channel from part of a (3 way) video conference
*
*  @return  0  on success, -ve on error
*/
int vcp_del_conference(
   VCP_HDL dec_hdl,                  /**< (i) Handle to VCP dec channel */
   VCP_HDL enc_hdl                   /**< (i) Handle to VCP enc channel */
)
{
   int err = -EPERM;
   printk( "vcp_del_conference 0x%x 0x%x\n", (unsigned int)dec_hdl, (unsigned int)enc_hdl);
   if( !is_valid_handle( enc_hdl ) || !is_valid_handle( dec_hdl ) )
   {
      printk("invalid handle!\n");
      return -EINVAL;
   }
   /* must be valid encoder and decoder handle */
   if( is_enc_handle( enc_hdl ) && !is_enc_handle( dec_hdl ) )
   {
      struct vcp_enc_user_node *encuserp = enc_hdl;
      struct vcp_dec_user_node *decuserp = dec_hdl;

      struct vcp_enc_device_node *encdevnodep = encuserp->devnodep;
      struct vcp_dec_device_node *decdevnodep = decuserp->devnodep;

      if( (encdevnodep->ops.setconference) &&(decdevnodep->ops.setconference) )
      {
         VCP_ENCODE_CONFERENCE enc_conf;
         VCP_DECODE_CONFERENCE dec_conf;

         /* add the decoder as part of the conference call */
         enc_conf.dec_hdl = dec_hdl;
         err = encdevnodep->ops.setconference( encuserp->pipehdl,
               VCP_ENC_DEL_CONFERENCE, &enc_conf, encdevnodep->devdata );

         if( err == 0 )
         {
            /* disable 3 party conference in the decoder */
            dec_conf.enable = 0;
            err = decdevnodep->ops.setconference( decuserp->pipehdl,
                  VCP_DEC_ENABLE_CONFERENCE, &dec_conf, decdevnodep->devdata );
         }
      }
   }
   else
   {
      printk("failure\n");
   }

   return err;   
}
EXPORT_SYMBOL( vcp_del_conference );
/***************************************************************************/
/**
*  Register a video encoder device.
*
*  @return  0        on success, otherwise -ve on error
*           EINVAL   Invalid parmeters, e.g. missing ops
*           EPERM    Cannot register, device already exists
*           ENOMEM   Insufficient memory
*/
int vcp_register_enc_device(
   const char *devname,             /**< (i) Device name */
   const VCP_ENC_DEVICE_OPS *ops,   /**< (i) Device callbacks */
   void *devdata,                   /**< (i) Private device data */
   VCP_DEV_HDL *hdlp                /**< (o) Ptr to output handle */
)
{
   struct vcp_enc_device_node *encnodep;
   struct vcp_base_device_node *tmp;

   if ( ops->start == NULL || ops->stop == NULL
   ||   ops->close == NULL || ops->setparm == NULL )
   {
      return -EINVAL;
   }

   if ( search_device( devname, VCP_DIR_ENCODER, &tmp ))
   {
      /* Cannot register, device already exists */
      return -EPERM;
   }

   VCP_DEBUG( "name=%s\n", devname );

   encnodep = kmalloc( sizeof(*encnodep), GFP_KERNEL );
   if ( encnodep == NULL )
   {
      return -ENOMEM;
   }
   memset( encnodep, 0, sizeof(*encnodep) );
   strncpy( encnodep->name, devname, sizeof(encnodep->name) );
   encnodep->dir = VCP_DIR_ENCODER;
   encnodep->devdata = devdata;
   memcpy( &encnodep->ops, ops, sizeof(encnodep->ops) );

   down( &gEncDeviceList.mutex );
   list_add_tail( &encnodep->lnode, &gEncDeviceList.list );
   gEncDeviceList.total++;
   up( &gEncDeviceList.mutex );

   *hdlp = encnodep;

   return 0;
}
EXPORT_SYMBOL( vcp_register_enc_device );

/***************************************************************************/
/**
*  Register a video decoder device.
*
*  @return  0  on success, -ve on error
*/
int vcp_register_dec_device(
   const char *devname,             /**< (i) Device name */
   const VCP_DEC_DEVICE_OPS *ops,   /**< (i) Device callbacks */
   void *devdata,                   /**< (i) Private device data */
   VCP_DEV_HDL *hdlp                /**< (o) Ptr to output handle */
)
{
   struct vcp_dec_device_node *decnodep;
   struct vcp_base_device_node *tmp;

   if ( ops->start == NULL || ops->stop == NULL || ops->close == NULL
   ||   ops->decode == NULL || ops->setparm == NULL )
   {
      return -EINVAL;
   }

   if ( search_device( devname, VCP_DIR_DECODER, &tmp ))
   {
      /* Cannot register, device already exists */
      return -EPERM;
   }

   VCP_DEBUG( "name=%s\n", devname );

   decnodep = kmalloc( sizeof(*decnodep), GFP_KERNEL );
   if ( decnodep == NULL )
   {
      return -ENOMEM;
   }
   memset( decnodep, 0, sizeof(*decnodep) );
   strncpy( decnodep->name, devname, sizeof(decnodep->name) );
   decnodep->dir = VCP_DIR_DECODER;
   decnodep->devdata = devdata;
   memcpy( &decnodep->ops, ops, sizeof(decnodep->ops) );

   down( &gDecDeviceList.mutex );
   list_add_tail( &decnodep->lnode, &gDecDeviceList.list );
   gDecDeviceList.total++;
   up( &gDecDeviceList.mutex );

   *hdlp = decnodep;

   return 0;
}
EXPORT_SYMBOL( vcp_register_dec_device );

/***************************************************************************/
/**
*  Deregister a video decoder or video encoder device
*
*  @return  0  on success, -ve on error
*/
int vcp_deregister_device(
   VCP_DEV_HDL hdl                  /**< (i) Handle */
)
{
   struct vcp_base_device_node *nodep = hdl;
   struct vcp_list *listp;
   struct vcp_base_user_node *baseuserp, *tmpbaseuserp;
   struct vcp_dec_user_node *decuserp;
   struct vcp_enc_user_node *encuserp;

   VCP_DEBUG( "name=%s\n", nodep->name );

   /* FIXME: Cleanup user handles to this device */
   list_for_each_entry_safe( baseuserp, tmpbaseuserp, &gUserList.list, lnode )
   {
      struct vcp_base_device_node *tstnodep;

      if ( baseuserp->dir == VCP_DIR_ENCODER )
      {
         encuserp = (void *)baseuserp;
         tstnodep = (void *)encuserp->devnodep;
      }
      else
      {
         decuserp = (void *)baseuserp;
         tstnodep = (void *)decuserp->devnodep;
      }

      if ( tstnodep == nodep )
      {
         // FIXME: what are the repercussions? Memory crashes in some APIs?
         /* Close user */
         vcp_close( baseuserp );
      }
   }

   listp = ( nodep->dir == VCP_DIR_ENCODER ) ?  &gEncDeviceList : &gDecDeviceList;
   down( &listp->mutex );
   list_del( &nodep->lnode );
   listp->total--;
   up( &listp->mutex );

   kfree( nodep );

   return 0;
}
EXPORT_SYMBOL( vcp_deregister_device );

/***************************************************************************/
/**
*  Driver open routine used specifically to register a user domain
*  video device.
*
*  @return
*     0        Success
*     -ENOMEM  Insufficient memory
*
*  @remarks
*
*  In the future, it is conceivable that users of VCP may be a mix of
*  kernel and user space daemons. In this future, the open routine will
*  just allocate a generic handle and the differentiation between
*  registering a device and creating a user handle (for user control)
*  will be determined by specific IOCTL commands. For now, the open
*  method is only ever used (by the user applications) to register
*  a device.
*/
static int vcp_open(
   struct inode *inode,             /*<< (io) Pointer to inode info */
   struct file  *file               /*<< (io) File structure pointer */
)
{
   gOpenCount++;
   file->private_data = NULL;
   VCP_DEBUG( "open=%i\n", gOpenCount );
   return 0;
}

/***************************************************************************/
/**
*  Driver release routine
*
*  @return
*     0        Success
*     -ve      Error code
*
*  @remarks
*
*  See the driver open routine for more information.
*/
static int vcp_release(
   struct inode *inode,             /*<< (io) Pointer to inode info */
   struct file  *file               /*<< (io) File structure pointer */
)
{
   gOpenCount--;
   if ( file->private_data )
   {
      /* Cleanup */
      vcpuser_deregister_device( file->private_data );
      file->private_data = NULL;
   }
   VCP_DEBUG( "released\n" );
   return 0;
}

/***************************************************************************/
/**
*  Driver ioctl method to support the user library API.
*
*  @return
*     >= 0           Number of bytes write
*     -ve            Error code
*/
static long vcp_ioctl(
   struct file   *file,             /*<< (io) File structure pointer */
   unsigned int   cmd,              /*<< (i)  IOCTL command */
   unsigned long  arg               /*<< (i)  User argument */
)
{
   int rc;
   union vcp_ioctl_params parm;
   unsigned int cmdnr, size;

   cmdnr    = _IOC_NR( cmd );
   size     = _IOC_SIZE( cmd );

   VCP_DEBUG( "cmdnr=%i size=%i\n", cmdnr, size );

   if ( size > sizeof(parm) )
   {
      return -ENOMEM;
   }

   if ( size )
   {
      if (( rc = copy_from_user( &parm, (void *)arg, size )) != 0 )
      {
         printk( "%s: (%d) FAILED copy_from_user status=%d\n", __FUNCTION__, cmdnr, rc );
         return rc;
      }
   }

   rc          = -EPERM;

   switch ( cmdnr )
   {
      case VCPIOCTL_CMD_REGISTER_DEC:
      {
         if ( file->private_data )
         {
            /* Already registered, must de-register first */
            return -EBUSY;
         }
         rc = vcpuser_register_dec_device( parm.register_dec.devname,
               &parm.register_dec.ops, parm.register_dec.devdata, &file->private_data );
         VCP_DEBUG( "register_dec_device: rc=%i devname=%s pipehdl=0x%lx\n", rc,
               parm.register_dec.devname, (unsigned long)file->private_data );
      }
      break;

      case VCPIOCTL_CMD_REGISTER_ENC:
      {
         if ( file->private_data )
         {
            /* Already registered, must de-register first */
            return -EBUSY;
         }
         rc = vcpuser_register_enc_device( parm.register_enc.devname,
               &parm.register_enc.ops, parm.register_enc.devdata, &file->private_data );
         VCP_DEBUG( "register_enc_device: rc=%i devname=%s pipehdl=0x%lx\n", rc,
               parm.register_enc.devname, (unsigned long)file->private_data );
      }
      break;

      case VCPIOCTL_CMD_DEREGISTER:
      {
         rc = vcpuser_deregister_device( file->private_data );
         file->private_data = NULL;
         VCP_DEBUG( "deregister_dec_device: rc=%i\n", rc );
      }
      break;

      case VCPIOCTL_CMD_GET_DECFRAME:
      {
         rc = vcpuser_get_decframe( file->private_data,
               parm.get_decframe.max_pktsize,
               parm.get_decframe.userdatap,
               parm.get_decframe.size_needed );
         VCP_DEBUG( "got decframe: rc=%i\n", rc );
      }
      break;

      case VCPIOCTL_CMD_GET_CTRLMSG:
      {
         rc = vcpuser_get_ctrlmsg( file->private_data,
               parm.get_ctrlmsg.userdatap );
         VCP_DEBUG( "got ctrlmsg: rc=%i\n", rc );
      }
      break;

      case VCPIOCTL_CMD_PUT_RESPONSE:
      {
         rc = vcpuser_put_response( file->private_data,
               parm.put_response.userdatap );
         VCP_DEBUG( "put response: rc=%i\n", rc );
      }
      break;

      case VCPIOCTL_CMD_DISCARD_DECFRAME:
      {
         rc = vcpuser_discard_decframe( file->private_data );
      }
      break;

      case VCPIOCTL_CMD_DEC_EVENTCB:
      {
         rc = vcpuser_dec_eventcb( parm.dec_event.pipehdl,
               parm.dec_event.eventCode, parm.dec_event.userinfo, file->private_data );
         VCP_DEBUG( "dec event %i: rc=%i\n", parm.dec_event.eventCode, rc );
      }
      break;

      case VCPIOCTL_CMD_ENC_EVENTCB:
      {
         rc = vcpuser_enc_eventcb( parm.enc_event.pipehdl,
               parm.enc_event.eventCode, parm.enc_event.userinfo, file->private_data );
         VCP_DEBUG( "enc event %i: rc=%i\n", parm.enc_event.eventCode, rc );
      }
      break;

      case VCPIOCTL_CMD_ENC_FRAME:
      {
         rc = vcpuser_enc_frame( parm.enc_frame.pipehdl,
               parm.enc_frame.bytesize, parm.enc_frame.userdatap, file->private_data );
         if( rc )
         {
            printk("%s: VCPIOCTL_CMD_ENC_FRAME failed rc=%d\n", __func__, rc );
         }
         VCP_DEBUG( "enc frame %i: rc=%i\n", parm.enc_frame.bytesize, rc );
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
*  Helper routine to initialize lists
*
*  @return  Nothing
*/
static void init_list(
   struct vcp_list *listp              /**< (io) Ptr to list to initialize */
)
{
   memset( listp, 0, sizeof(*listp) );
   INIT_LIST_HEAD( &listp->list );
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,37)
   init_MUTEX( &listp->mutex );
#else
   sema_init( &listp->mutex , 1 );
#endif
}

/***************************************************************************/
/**
*  Driver initialization called when module loaded by kernel
*
*  @return
*     0              Success
*     -ve            Error code
*/
static int __init vcp_init( void )
{
   int err;

   printk( banner );

   init_list( &gEncDeviceList );
   init_list( &gDecDeviceList );
   init_list( &gUserList );

   gDbgPrintLevel = 0;
   debug_init();

   err = vcpuser_constructor();
   if ( err )
   {
      printk( "%s: failed to init user proxy layer\n", __FUNCTION__ );
      return err;
   }

   gDriverMajor = register_chrdev( 0, "vcp", &gfops );
   if ( gDriverMajor < 0 )
   {
      printk( "%s: failed to register character device major=%d\n", __FUNCTION__, gDriverMajor );
      goto failed_cleanup;
   }

	vcp_class = class_create( THIS_MODULE, "bcmring-vcp" );
	if ( IS_ERR( vcp_class ))
	{
		printk( "VCP: Class create failed\n" );
		err = -EFAULT;
		goto err_unregister_chrdev;
	}

	vcp_dev = device_create( vcp_class, NULL, MKDEV( gDriverMajor, 0 ), NULL, "vcp");
	if ( IS_ERR( vcp_dev ))
	{
		printk( "VCP: Device create failed\n" );
		err = -EFAULT;
		goto err_class_destroy;
	}

	return 0;

err_class_destroy:
	class_destroy( vcp_class );
err_unregister_chrdev:
	unregister_chrdev( gDriverMajor, "vcp" );
failed_cleanup:
	printk( "failed_cleanup\n" );

	return err;
}

/***************************************************************************/
/**
*  Driver destructor routine. Frees all resources
*/
static void __exit vcp_exit( void )
{
   debug_exit();
   vcpuser_destructor();

	device_destroy( vcp_class, MKDEV( gDriverMajor, 0 ));
	class_destroy( vcp_class );
   unregister_chrdev( gDriverMajor, "vcp" );
}

/***************************************************************************/
/**
*  Proc read callback function
*
*  @return  Number of characters to print
*/
static int vcp_read_proc( char *buf, char **start, off_t offset, int count,
   int *eof, void *data )
{
   struct vcp_base_device_node *devnodep;
   struct vcp_base_user_node *userp;
   int len = 0;

   len += sprintf( buf+len, "Encoder Devices: total=%i\n", gEncDeviceList.total );
   list_for_each_entry( devnodep, &gEncDeviceList.list, lnode )
   {
      len += sprintf( buf+len, " %s:enc\n", devnodep->name );
   }

   len += sprintf( buf+len, "Decoder Devices: total=%i\n", gDecDeviceList.total );
   list_for_each_entry( devnodep, &gDecDeviceList.list, lnode )
   {
      len += sprintf( buf+len, " %s:dec\n", devnodep->name );
   }

   len += sprintf( buf+len, "Users: total=%i\n", gUserList.total );
   list_for_each_entry( userp, &gUserList.list, lnode )
   {
      if ( userp->dir == VCP_DIR_ENCODER )
      {
         struct vcp_enc_user_node *encuserp = (void *)userp;
         len += sprintf( buf+len, " %s:enc pipehdl=0x%lx\n",
               encuserp->devnodep->name,
               (unsigned long)encuserp->pipehdl );
      }
      else
      {
         struct vcp_dec_user_node *decuserp = (void *)userp;
         len += sprintf( buf+len, " %s:dec pipehdl=0x%lx\n",
               decuserp->devnodep->name,
               (unsigned long)decuserp->pipehdl );
      }
   }

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
static void debug_init( void )
{
   create_proc_read_entry( VCP_PROC_NAME, 0, NULL, vcp_read_proc, NULL );

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
static void debug_exit( void )
{
   remove_proc_entry( VCP_PROC_NAME, NULL );

   if ( gSysCtlHeader )
   {
      unregister_sysctl_table( gSysCtlHeader );
   }
}

module_init( vcp_init );
module_exit( vcp_exit );
MODULE_AUTHOR( "Broadcom" );
MODULE_DESCRIPTION( "Video Conference Pipeline Framework Driver" );
MODULE_LICENSE( "GPL v2" );
