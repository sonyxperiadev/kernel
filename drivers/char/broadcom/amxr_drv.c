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
*  @file    amxr_drv.c
*
*  @brief   This file implements the Audio Mixer driver.
*
*****************************************************************************/

/* ---- Include Files ---------------------------------------------------- */

#include <linux/types.h>                     /* For stdint types: uint8_t, etc. */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/proc_fs.h>                   /* For /proc/audio */
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
#include <linux/broadcom/amxr.h>             /* Audio mixer API */
#include <linux/broadcom/amxr_ioctl.h>       /* Audio mixer user API */
#include <asm/io.h>
#include <asm/uaccess.h>                     /* User access routines */
#include <asm/atomic.h>                      /* Atomic operations */

/* ---- Public Variables ------------------------------------------------- */

/* ---- Private Constants and Types -------------------------------------- */

/* Client list */
struct amxr_client_list
{
   struct semaphore           mutex;         /* Protect client list */
   int                        total;         /* Total number of clients */
};

/* AMXR port information */
struct amxr_port_node
{
   struct list_head           lnode;         /* List node */
   AMXR_PORT_ID               id;            /* Port ID */
   char                       name[32];      /* Name string */
};

/**
*  Mixer port head structure
*/
struct amxr_port_head
{
   struct semaphore           mutex;         /* Protect list */
   struct list_head           list;          /* List head */
   int                        total;         /* Total number of ports */
};

/**
*  Union of all the different IOCTL parameter structures to determine
*  max stack variable size
*/
union amxr_ioctl_params
{
   struct amxr_ioctl_queryport             getportid;
   struct amxr_ioctl_queryportinfo         getportinfo;
   struct amxr_ioctl_setcnxloss            setcnxloss;
   struct amxr_ioctl_getcnxloss            getcnxloss;
   struct amxr_ioctl_connect               connect;
   struct amxr_ioctl_disconnect            disconnect;
   struct amxr_ioctl_get_cnxlist           getcnxlist;
};

/* Procfs file name */
#define AMXR_PROC_NAME            "amxr"

/**
*  Verbose prints are enabled via /proc/sys/amxr/dbgprint.
*
*  Usage:
*
*  Set to 1 to get basic informational traces. Set to 2 to get
*  all basic traces plus additional verbose prints.
*/
#define AMXR_TRACE( fmt, args... )  do { if ( gDbgPrintLevel ) printk( KERN_INFO "%s: " fmt, __FUNCTION__ , ## args ); } while (0)
#define AMXR_DEBUG( fmt, args... )  do { if ( gDbgPrintLevel >= 2 ) printk( KERN_INFO "%s: " fmt, __FUNCTION__ , ## args ); } while (0)

/* ---- Private Variables ------------------------------------------------ */

static int gDriverMajor;
static struct class *amxr_class;
static struct device *amxr_dev;

/* Driver start up banner string */
static char banner[] __initdata = KERN_INFO "Audio Mixer Driver: 1.0\n";

/* Clients list */
static struct amxr_client_list    gClients;

/* Mixer ports list */
static struct amxr_port_head      gPorts;

/* Debug print level */
static int                        gDbgPrintLevel;

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
      .procname      = "amxr",
      .mode          = 0555,
      .child         = gSysCtlChild
   },
   {}
};
static struct ctl_table_header  *gSysCtlHeader;

/* Installed proprietary API methods */
static AMXR_API_FUNCS            gApi;

/* Memory pointer and other control variables for figuring out cnx lists */
#define AMXR_CNXS_REALLOC_INC    (10 * sizeof(AMXR_PORT_CNXINFO))
static AMXR_CNXS                *gCnxListp;
static int                       gCnxListSize;
static struct semaphore          gCnxListLock;

/* ---- Private Function Prototypes -------------------------------------- */
static void    amxr_debug_init( void );
static void    amxr_debug_exit( void );
static int     amxr_open( struct inode *inode, struct file *file );
static int     amxr_release( struct inode *inode, struct file *file );
static long    amxr_ioctl(struct file *file, unsigned int cmd, unsigned long arg );

/* ---- Functions -------------------------------------------------------- */

/**
*  File Operations (these are the device driver entry points)
*/
static struct file_operations gfops =
{
   .owner      = THIS_MODULE,
   .open       = amxr_open,
   .release    = amxr_release,
   .unlocked_ioctl = amxr_ioctl,
};

/***************************************************************************/
/**
*  Allocates a client handle to allow the client to make use of the
*  audio mixer resources.
*
*  @return
*     valid ptr   On success, valid client handle
*     NULL        Out of memory, -ENOMEM
*/
AMXR_HDL amxrAllocateClient( void )
{
   AMXR_HDL handle;

   down( &gClients.mutex );
   handle = gClients.total++;
   up( &gClients.mutex );

   return handle;
}
EXPORT_SYMBOL( amxrAllocateClient );

/***************************************************************************/
/**
*  Frees client handle and performs related cleanup. Connections
*  created by this clients are not deleted.
*
*  @return
*     0           On success
*     -ve         On general failure
*/
int amxrFreeClient(
   AMXR_HDL hdl                   /*<< (i) client handle */
)
{
   /* Free client */
   down( &gClients.mutex );
   if ( gClients.total > 0 )
   {
      gClients.total--;
   }
   up( &gClients.mutex );

   return 0;
}
EXPORT_SYMBOL( amxrFreeClient );

/***************************************************************************/
/**
*  Queries for port ID by name string.
*
*  @return
*     0        Found matching port
*     -EINVAL  Port not found
*     -ve      Other errors
*/
int amxrQueryPortByName(
   AMXR_HDL        hdl,             /*<< (i) Mixer client handle */
   const char     *name,            /*<< (i) Name of port */
   AMXR_PORT_ID   *id               /*<< (o) Ptr to store found port ID */
)
{
   struct amxr_port_node  *portp;
   int                     err;

   err = -EINVAL;

   down( &gPorts.mutex );
   list_for_each_entry( portp, &gPorts.list, lnode )
   {
      if ( strncmp( name, portp->name, sizeof(portp->name) ) == 0 )
      {
         /* Match found */
         *id = portp->id;
         err = 0;
         break;
      }
   }
   up( &gPorts.mutex );

   return err;
}
EXPORT_SYMBOL( amxrQueryPortByName );

/***************************************************************************/
/**
*  Queries for name string by Port ID
*
*  @return
*     0        Found matching port
*     -EINVAL  Port not found
*     -ve      Other errors
*
*  @remarks
*     Intended to be used by kernel drivers for verbose printing.
*/
int amxrQueryNameById(
   AMXR_PORT_ID    id,              /*<< (i) Port ID */
   const char    **name             /*<< (o) Name of port */
)
{
   struct amxr_port_node  *portp;
   int                     err;

   err = -EINVAL;

   down( &gPorts.mutex );
   list_for_each_entry( portp, &gPorts.list, lnode )
   {
      if ( id == portp->id )
      {
         /* Match found */
         *name = portp->name;
         err   = 0;
         break;
      }
   }
   up( &gPorts.mutex );

   return err;
}
EXPORT_SYMBOL( amxrQueryNameById );

/***************************************************************************/
/**
*  Obtain audio mixer port information
*
*  @return
*     0     On success
*     -ve   Failure code
*/
int amxrGetPortInfo(
   AMXR_HDL        hdl,           /*<< (i) Mixer client handle */
   AMXR_PORT_ID    port,          /*<< (i) Port id */
   AMXR_PORT_INFO *info           /*<< (o) Ptr to port info structure */
)
{
   if ( gApi.getPortInfo )
   {
      return gApi.getPortInfo( hdl, port, info );
   }
   return -EPERM;
}
EXPORT_SYMBOL( amxrGetPortInfo );

/***************************************************************************/
/**
*  Obtain general audio mixer information, such as the number of
*  registered users.
*
*  @return
*     0     On success
*     -ve   Failure code
*/
int amxrGetInfo(
   AMXR_HDL     hdl,              /*<< (i) Mixer client handle */
   AMXR_INFO   *info              /*<< (o) Ptr to info structure */
)
{
   int err = 0;

   memset( info, 0, sizeof(*info) );

   if ( gApi.getInfo )
   {
      err = gApi.getInfo( hdl, info );
   }

   if ( !err )
   {
      info->clients = gClients.total;

      /* Error check */
      if ( info->ports != gPorts.total )
      {
         printk( KERN_ERR "%s: inconsistent number of ports between driver (%i) and core (%i).\n",
               __FUNCTION__, gPorts.total, info->ports );
      }
   }

   return err;
}
EXPORT_SYMBOL( amxrGetInfo );

/***************************************************************************/
/**
*  Set connection loss to attenuate samples between source and destination
*  ports. Loss range is from 0 db to mute in 1db increments.
*
*  @return
*     0           On success
*     -EINVAL     No such connection found
*     -ve         Other errors
*/
int amxrSetCnxLoss(
   AMXR_HDL           hdl,        /*<< (i) client handle */
   AMXR_PORT_ID       src_port,   /*<< (i) source port id */
   AMXR_PORT_ID       dst_port,   /*<< (i) destination port id */
   AMXR_CONNECT_DESC  desc,       /*<< (i) Connection descriptor */
   unsigned int       db          /*<< (i) Loss amount in dB */
)
{
   if ( gApi.setCnxLoss )
   {
      return gApi.setCnxLoss( hdl, src_port, dst_port, desc, db );
   }
   return -EINVAL;
}
EXPORT_SYMBOL( amxrSetCnxLoss );

/***************************************************************************/
/**
*  Read connection loss amount in dB.
*
*  @return
*     0           On success
*     -EINVAL     No such connection found
*     -ve         Other errors
*/
int amxrGetCnxLoss(
   AMXR_HDL           hdl,        /*<< (i) client handle */
   AMXR_PORT_ID       src_port,   /*<< (i) source port id */
   AMXR_PORT_ID       dst_port,   /*<< (i) destination port id */
   AMXR_CONNECT_DESC  desc,       /*<< (i) Connection descriptor */
   unsigned int      *db          /*<< (o) Pointer to store attenuation amount */
)
{
   if ( gApi.getCnxLoss )
   {
      return gApi.getCnxLoss( hdl, src_port, dst_port, desc, db );
   }
   return -EINVAL;
}
EXPORT_SYMBOL( amxrGetCnxLoss );

/***************************************************************************/
/**
*  Make a simplex connection from source to destination ports.
*
*  @return
*     0           On success or connection already exists
*     -ve         On general failure
*/
int amxrConnect(
   AMXR_HDL           hdl,        /*<< (i) client handle */
   AMXR_PORT_ID       src_port,   /*<< (i) source port id */
   AMXR_PORT_ID       dst_port,   /*<< (i) destination port id */
   AMXR_CONNECT_DESC  desc        /*<< (i) Connection descriptor */
)
{
   if ( gApi.connect )
   {
      return gApi.connect( hdl, src_port, dst_port, desc );
   }
   return -EPERM;
}
EXPORT_SYMBOL( amxrConnect );

/***************************************************************************/
/**
*  Remove simplex connections between source and destination ports.
*
*  @return
*     0           On success
*     -ve         On general failure
*/
int amxrDisconnect(
   AMXR_HDL           hdl,        /*<< (i) Mixer client handle */
   AMXR_PORT_ID       src_port,   /*<< (i) source port id */
   AMXR_PORT_ID       dst_port    /*<< (i) destination port id */
)
{
   if ( gApi.disconnect )
   {
      return gApi.disconnect( hdl, src_port, dst_port );
   }
   return -EPERM;
}
EXPORT_SYMBOL( amxrDisconnect );

/***************************************************************************/
/**
*  Query the connection list by source port. All destination ports connected
*  to the specified source port will be returned.
*
*  @return
*     0           On success
*     -ve         On general failure
*/
int amxrGetCnxListBySrc(
   AMXR_HDL             hdl,        /*<< (i) Mixer client handle */
   AMXR_PORT_ID         src_port,   /*<< (i) Source port */
   AMXR_CNXS           *cnxlist,    /*<< (o) Ptr to store cnx list */
   int                  maxlen      /*<< (i) Max length in bytes */
)
{
   if ( gApi.getCnxListBySrc )
   {
      return gApi.getCnxListBySrc( hdl, src_port, cnxlist, maxlen );
   }
   return -EPERM;
}
EXPORT_SYMBOL( amxrGetCnxListBySrc );

/***************************************************************************/
/**
*  Query the connection list by destination port. All source ports connected
*  to the specified destination port will be returned.
*
*  @return
*     0           On success
*     -ve         On general failure
*/
int amxrGetCnxListByDst(
   AMXR_HDL             hdl,        /*<< (i) Mixer client handle */
   AMXR_PORT_ID         dst_port,   /*<< (i) Dst port */
   AMXR_CNXS           *cnxlist,    /*<< (o) Ptr to store cnx list */
   int                  maxlen      /*<< (i) Max length in bytes */
)
{
   if ( gApi.getCnxListByDst )
   {
      return gApi.getCnxListByDst( hdl, dst_port, cnxlist, maxlen );
   }
   return -EPERM;
}
EXPORT_SYMBOL( amxrGetCnxListByDst );

/***************************************************************************/
/**
*  Create a mixer port.
*
*  @return
*     0        On success
*     -ve      Error code
*/
int amxrCreatePort(
   const char       *name,          /*<< (i) Name string */
   AMXR_PORT_CB     *cb,            /*<< (i) Callbacks */
   void             *privdata,      /*<< (i) Private data passed back to callbacks */
   int               dst_hz,        /*<< (i) Current destination sampling frequency in Hz */
   int               dst_chans,     /*<< (i) Number of channels, i.e. mono = 1 */
   int               dst_bytes,     /*<< (i) Destination period size in bytes */
   int               src_hz,        /*<< (i) Current source sampling frequency in Hz */
   int               src_chans,     /*<< (i) Number of channels, i.e. mono = 1 */
   int               src_bytes,     /*<< (i) Source period size in bytes */
   AMXR_PORT_ID     *portidp        /*<< (o) Ptr to port ID */
)
{
   int err = -EPERM;
   if ( gApi.createPort )
   {
      struct amxr_port_node *portp;

      portp = vmalloc( sizeof(*portp) );
      if ( !portp )
      {
         return -ENOMEM;
      }
      memset( portp, 0, sizeof(*portp) );
      strncpy( portp->name, name, sizeof(portp->name) - 1 );

      err = gApi.createPort( name, cb, privdata, dst_hz, dst_chans, dst_bytes,
            src_hz, src_chans, src_bytes, portidp );
      if ( !err )
      {
         /* Keep record of ports */
         portp->id = *portidp;

         down( &gPorts.mutex );
         list_add_tail( &portp->lnode, &gPorts.list );
         gPorts.total++;
         up( &gPorts.mutex);
      }
      else
      {
         vfree( portp );
      }
   }
   return err;
}
EXPORT_SYMBOL( amxrCreatePort );

/***************************************************************************/
/**
*  Remove an existing mixer port. All connections to this port will also
*  be removed.
*
*  @return
*     0           On success
*     -ve         On general failure
*
*  @remarks
*     Typically ports are not removed unless the entire user is removed.
*     Although it is conceivable that the API supports dynamic creation and
*     deletion of ports, it is not practical for most applications since
*     applications will have to constantly query what ports still exists
*     and keep track of valid port IDs.
*/
int amxrRemovePort(
   AMXR_PORT_ID port              /*<< (i) Port to remove */
)
{
   int err = -EPERM;
   if ( gApi.removePort )
   {
      struct amxr_port_node  *portp, *tmpportp;

      err = gApi.removePort( port );

      down( &gPorts.mutex );
      list_for_each_entry_safe( portp, tmpportp, &gPorts.list, lnode )
      {
         if ( portp->id == port )
         {
            list_del( &portp->lnode );
            vfree( portp );
            gPorts.total--;
            break;
         }
      }
      up( &gPorts.mutex );
   }
   return err;
}
EXPORT_SYMBOL( amxrRemovePort );

/***************************************************************************/
/**
*  Set destination port sampling frequency. Connections using the port
*  are also updated accordingly.
*
*  @return
*     0           On success
*     -ve         On general failure
*/
int amxrSetPortDstFreq(
   AMXR_PORT_ID   portid,           /*<< (i) Destination port id */
   int            dst_hz,           /*<< (i) Destination sampling frequency in Hz */
   int            dst_bytes         /*<< (i) Destination period size in bytes */
)
{
   if ( gApi.setPortDstFreq )
   {
      return gApi.setPortDstFreq( portid, dst_hz, dst_bytes );
   }
   return -EPERM;
}
EXPORT_SYMBOL( amxrSetPortDstFreq );

/***************************************************************************/
/**
*  Set source port sampling frequency and expected frame size.
*  Connections using the port are also updated accordingly.
*
*  @return
*     0           On success
*     -ve         On general failure
*/
int amxrSetPortSrcFreq(
   AMXR_PORT_ID   portid,           /*<< (i) Source port id */
   int            src_hz,           /*<< (i) Source sampling frequency in Hz */
   int            src_bytes         /*<< (i) Source period size in bytes */
)
{
   if ( gApi.setPortSrcFreq )
   {
      return gApi.setPortSrcFreq( portid, src_hz, src_bytes );
   }
   return -EPERM;
}
EXPORT_SYMBOL( amxrSetPortSrcFreq );

/***************************************************************************/
/**
*  Set the number of channels supported by the destination port.
*
*  @return
*     0           On success
*     -ve         On general failure
*
*  @remarks    Not all existing connections with this source port
*              will be maintained after changing the number of channels.
*/
int amxrSetPortDstChannels(
   AMXR_PORT_ID   portid,           /*<< (i) Destination port id */
   int            dst_chans,        /*<< (i) Number of channels: 1 for mono, 2 for stereo, etc */
   int            dst_bytes         /*<< (i) Destination period size in bytes */
)
{
   if ( gApi.setPortDstChannels )
   {
      return gApi.setPortDstChannels( portid, dst_chans, dst_bytes );
   }
   return -EPERM;
}
EXPORT_SYMBOL( amxrSetPortDstChannels );

/***************************************************************************/
/**
*  Set the number of channels supported by the destination port.
*
*  @return
*     0           On success
*     -ve         On general failure
*
*  @remarks    Not all existing connections with this source port
*              will be maintained after changing the number of channels.
*/
int amxrSetPortSrcChannels(
   AMXR_PORT_ID   portid,           /*<< (i) Source port id */
   int            src_chans,        /*<< (i) Number of src channels: 1 for mono, 2 for stereo, etc. */
   int            src_bytes         /*<< (i) Source period size in bytes */
)
{
   if ( gApi.setPortSrcChannels )
   {
      return gApi.setPortSrcChannels( portid, src_chans, src_bytes );
   }
   return -EPERM;
}
EXPORT_SYMBOL( amxrSetPortSrcChannels );

/***************************************************************************/
/**
*  Setup proprietary Audio Mixer implementation
*
*  @return    none
*/
void amxrSetApiFuncs(
   const AMXR_API_FUNCS *funcsp     /*<< (i) Ptr to API functions.
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
EXPORT_SYMBOL( amxrSetApiFuncs );

/***************************************************************************/
/**
*  This routine services all connections to and from a particular port
*  that are at "irregular" sampling frequencies, which are defined as not 
*  a multiple of 8.
*  
*  @remarks
*     Typically, this routine is called by HAL Audio, which is clocked 
*     by hardware.
*
*     This routine runs in an ATOMIC context!
*/
void amxrServiceUnsyncPort( 
   AMXR_PORT_ID id                  /*<< (i) Port ID of port to service */
)
{
   if ( gApi.serviceUnsyncPort )
   {
      return gApi.serviceUnsyncPort( id );
   }
}
EXPORT_SYMBOL( amxrServiceUnsyncPort );

/***************************************************************************/
/**
*  Driver open routine
*
*  @return
*     0        Success
*     -ENOMEM  Insufficient memory
*/
static int amxr_open(
   struct inode *inode,             /*<< (io) Pointer to inode info */
   struct file  *file               /*<< (io) File structure pointer */
)
{
   AMXR_HDL  client_hdl;

   client_hdl = amxrAllocateClient();
   if ( client_hdl < 0 )
   {
      return -ENOMEM;
   }
   file->private_data = (void *)client_hdl;

   AMXR_TRACE( "hdl=0x%lx\n", (unsigned long)client_hdl );

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
static int amxr_release(
   struct inode *inode,             /*<< (io) Pointer to inode info */
   struct file  *file               /*<< (io) File structure pointer */
)
{
   AMXR_TRACE( "hdl=0x%lx\n", (unsigned long)file->private_data );

   return amxrFreeClient( (AMXR_HDL)file->private_data );
}

/***************************************************************************/
/**
*  Driver ioctl method to support user library API.
*
*  @return
*     >= 0           Number of bytes write
*     -ve            Error code
*/
static long amxr_ioctl(
   struct file   *file,             /*<< (io) File structure pointer */
   unsigned int   cmd,              /*<< (i)  IOCTL command */
   unsigned long  arg               /*<< (i)  User argument */
)
{
   int                        rc;
   union amxr_ioctl_params    parm;
   unsigned int               cmdnr, size;
   AMXR_HDL                   client_hdl;

   cmdnr    = _IOC_NR( cmd );
   size     = _IOC_SIZE( cmd );

   AMXR_DEBUG( "cmdnr=%i size=%i\n", cmdnr, size );

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

   client_hdl  = (AMXR_HDL)file->private_data;
   rc          = -EPERM;

   switch ( cmdnr )
   {
      case AMXR_CMD_QUERY_PORT:
      {
         AMXR_PORT_ID id;
         rc = amxrQueryPortByName( client_hdl, parm.getportid.name, &id );
         if ( rc == 0 )
         {
            rc = copy_to_user( parm.getportid.id, &id, sizeof(id) );
         }
      }
      break;

      case AMXR_CMD_GET_PORT_INFO:
      {
         AMXR_PORT_INFO info;
         rc = amxrGetPortInfo( client_hdl, parm.getportinfo.port, &info );
         if ( rc == 0 )
         {
            rc = copy_to_user( parm.getportinfo.info, &info, sizeof(info) );
         }
      }
      break;

      case AMXR_CMD_GET_INFO:
      {
         AMXR_INFO info;
         rc = amxrGetInfo( client_hdl, &info );
         if ( rc == 0 )
         {
            rc = copy_to_user( (void *)arg, &info, sizeof(info) );
         }
      }
      break;

      case AMXR_CMD_SET_CNX_LOSS:
      {
         rc = amxrSetCnxLoss( client_hdl, parm.setcnxloss.src_port,
               parm.setcnxloss.dst_port, parm.setcnxloss.desc, parm.setcnxloss.db );
      }
      break;

      case AMXR_CMD_GET_CNX_LOSS:
      {
         int db;
         rc = amxrGetCnxLoss( client_hdl, parm.getcnxloss.src_port,
               parm.getcnxloss.dst_port, parm.getcnxloss.desc, &db );
         if ( rc == 0 )
         {
            rc = copy_to_user( parm.getcnxloss.db, &db, sizeof(db) );
         }
      }
      break;

      case AMXR_CMD_CONNECT:
      {
         rc = amxrConnect( client_hdl, parm.connect.src_port,
               parm.connect.dst_port, parm.connect.desc );
      }
      break;

      case AMXR_CMD_DISCONNECT:
      {
         rc = amxrDisconnect( client_hdl, parm.disconnect.src_port, parm.disconnect.dst_port );
      }
      break;

      case AMXR_CMD_GET_CNXLIST_BY_SRC:
      case AMXR_CMD_GET_CNXLIST_BY_DST:
      {
         void *memp;
         int (*getcnxlistfp)( AMXR_HDL, AMXR_PORT_ID, AMXR_CNXS *, int );

         memp = vmalloc( parm.getcnxlist.maxlen );
         if ( memp == NULL )
         {
            return -ENOMEM;
         }

         getcnxlistfp = ( cmdnr == AMXR_CMD_GET_CNXLIST_BY_SRC ) ?
            amxrGetCnxListBySrc : amxrGetCnxListByDst;

         rc = getcnxlistfp( client_hdl, parm.getcnxlist.port, memp, parm.getcnxlist.maxlen );
         if ( rc == 0 )
         {
            rc = copy_to_user( parm.getcnxlist.cnxlist, memp, parm.getcnxlist.maxlen );
         }

         vfree( memp );
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
*  Driver initialization called when module loaded by kernel
*
*  @return
*     0              Success
*     -ve            Error code
*/
static int __init amxr_init( void )
{
   int err;

   printk( banner );

   memset( &gClients, 0, sizeof(gClients) );
   memset( &gPorts, 0, sizeof(gPorts) );
   INIT_LIST_HEAD( &gPorts.list );
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,37)
   init_MUTEX( &gClients.mutex );
#else
   sema_init( &gClients.mutex , 1 );
#endif
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,37)
   init_MUTEX( &gPorts.mutex );
#else
   sema_init( &gPorts.mutex , 1 );
#endif

   gDbgPrintLevel = 0;
   amxr_debug_init();

   gDriverMajor = register_chrdev( 0, "amxr", &gfops );
   if ( gDriverMajor < 0 )
   {
      printk( KERN_ERR "AMXR: Failed to register character device major\n" );
      err = -EFAULT;
      goto failed_cleanup;
   }

	amxr_class = class_create( THIS_MODULE, "bcmring-amxr" );
	if ( IS_ERR( amxr_class ))
	{
		printk( KERN_ERR "AMXR: Class create failed\n" );
		err = -EFAULT;
		goto err_unregister_chrdev;
	}
	
	amxr_dev = device_create( amxr_class, NULL, MKDEV( gDriverMajor, 0 ), NULL, "amxr" );
	if ( IS_ERR( amxr_dev ))
	{
		printk( KERN_ERR "AMXR: Device create failed\n" );
		err = -EFAULT;
		goto err_class_destroy;
	}	
	return 0;
	
err_class_destroy: 
	class_destroy( amxr_class ); 
err_unregister_chrdev: 
	unregister_chrdev( gDriverMajor, "amxr" );
failed_cleanup:
	printk( KERN_ERR "failed_cleanup\n" );

	return err;
}

/***************************************************************************/
/**
*  Driver destructor routine. Frees all resources
*/
static void __exit amxr_exit( void )
{
   if ( gClients.total )
   {
      printk( KERN_ERR "%s: orphaned %i clients\n", __FUNCTION__, gClients.total );
   }

   amxr_debug_exit();

   device_destroy( amxr_class, MKDEV( gDriverMajor, 0 ));
	class_destroy( amxr_class );
   unregister_chrdev( gDriverMajor, "amxr" );
}

/***************************************************************************/
/**
*  Proc read callback function
*
*  @return  Number of characters to print
*/
static int amxr_read_proc( char *buf, char **start, off_t offset, int count, int *eof, void *data )
{
   int                     i, len, err;
   struct amxr_port_node  *portp;
   AMXR_PORT_INFO          portinfo;

   len = 0;

   len += sprintf( buf+len, "Ports: total=%i\n", gPorts.total );
   list_for_each_entry( portp, &gPorts.list, lnode )
   {
      err = amxrGetPortInfo( 0, portp->id, &portinfo );
      if ( err )
      {
         break;
      }
      len += sprintf( buf+len, " %s: dst(cnxs=%i hz=%i ch=%i fs=%i) src(cnxs=%i hz=%i ch=%i fs=%i)\n",
            portinfo.name, portinfo.dst_cnxs, portinfo.dst_hz, portinfo.dst_chans,
            portinfo.dst_bytes, portinfo.src_cnxs, portinfo.src_hz, portinfo.src_chans,
            portinfo.src_bytes );
   }

   len += sprintf( buf+len, "Connections (src->dst):\n" );
   if ( gCnxListSize )
   {
      down( &gCnxListLock );
      list_for_each_entry( portp, &gPorts.list, lnode )
      {
         AMXR_PORT_CNXINFO *cnxinfop;

         err = amxrGetCnxListBySrc( 0, portp->id, gCnxListp, gCnxListSize );
         if ( err )
         {
            if ( err == -ENOMEM )
            {
               void *memp;
               int   size;

               /* Re-allocate memory */

               size = gCnxListSize + AMXR_CNXS_REALLOC_INC;
               memp = vmalloc( gCnxListSize );
               if ( memp )
               {
                  vfree( gCnxListp );
                  gCnxListSize   = size;
                  gCnxListp      = memp;
               }
            }
            continue;
         }

         if ( gCnxListp->cnxs )
         {
            len += sprintf( buf+len, " %s:\n", portp->name );

            cnxinfop = gCnxListp->list;
            for ( i = 0; i < gCnxListp->cnxs; i++, cnxinfop++ )
            {
               const char *dstportname;
               err = amxrQueryNameById( cnxinfop->port, &dstportname );
               if ( err )
               {
                  dstportname = "UNKNOWN";
               }
               len += sprintf( buf+len, " -> %s (lossdb=%i dst_idx=%i cnx_type=%i src_idx=%i decim=%i inter=%i %s)\n",
                     dstportname, cnxinfop->lossdb,
                     AMXR_CONNECT_GET_DST_IDX( cnxinfop->desc ),
                     AMXR_CONNECT_GET_TYPE( cnxinfop->desc ),
                     AMXR_CONNECT_GET_SRC_IDX( cnxinfop->desc ),
                     cnxinfop->decim, cnxinfop->inter,
                     cnxinfop->active ? "active" : "X" );
            }
         }
      }
      up( &gCnxListLock );
   }

   len += sprintf( buf+len, "Clients: total=%i\n", gClients.total );

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
static void amxr_debug_init( void )
{
   gCnxListSize = sizeof(*gCnxListp) + AMXR_CNXS_REALLOC_INC;
   gCnxListp = vmalloc( gCnxListSize );
   if ( gCnxListp == NULL )
   {
      gCnxListSize = 0;
      printk( KERN_ERR "%s: failed to allocate scratch memory\n", __FUNCTION__ );
   }

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,37)
   init_MUTEX( &gCnxListLock );
#else
   sema_init( &gCnxListLock , 1 );
#endif
   create_proc_read_entry( AMXR_PROC_NAME, 0, NULL, amxr_read_proc, NULL );

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
static void amxr_debug_exit( void )
{
   remove_proc_entry( AMXR_PROC_NAME, NULL );

   if ( gSysCtlHeader )
   {
      unregister_sysctl_table( gSysCtlHeader );
   }

   if ( gCnxListp )
   {
      vfree( gCnxListp );
      gCnxListp      = NULL;
      gCnxListSize   = 0;
   }
}

module_init( amxr_init );
module_exit( amxr_exit );
MODULE_AUTHOR( "Broadcom" );
MODULE_DESCRIPTION( "Audio Mixer Driver" );
MODULE_LICENSE( "GPL" );
