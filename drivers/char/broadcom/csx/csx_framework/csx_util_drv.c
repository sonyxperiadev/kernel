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
*  @file    csx_util_drv.c
*
*  @brief   CSX-UTIL device driver
*
****************************************************************************/

/* ---- Include Files ---------------------------------------------------- */
#include <linux/types.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/platform_device.h>
#include <linux/proc_fs.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/string.h>
#include <linux/vmalloc.h>

#include <linux/broadcom/bcm_major.h>
#include <linux/broadcom/csx_framework.h>
#include <linux/broadcom/csx_util_ioctl.h>

#include <asm/uaccess.h>

/* ---- Private Constants and Types -------------------------------------- */

/**
*  Union of all the different IOCTL parameter structures to determine
*  max stack variable size
*/
typedef union csx_util_ioctl_parms
{
   CSX_UTIL_IOCTL_ADD_POINT_PARMS      add_point_parms;
   CSX_UTIL_IOCTL_REMOVE_POINT_PARMS   remove_point_parms;
   CSX_UTIL_IOCTL_GET_NUM_POINTS_PARMS get_num_points_parms;
   CSX_UTIL_IOCTL_QUERY_ALL_PARMS      query_all_parms;
   CSX_UTIL_IOCTL_SYNC_ENABLE_PARMS    sync_enable_parms;
   CSX_UTIL_IOCTL_SYNC_DISABLE_PARMS   sync_disable_parms;

} CSX_UTIL_IOCTL_PARMS;

/* ---- Private Function Prototypes -------------------------------------- */
static int csx_util_drv_open( struct inode *inode, struct file *file );
static int csx_util_drv_release( struct inode *inode, struct file *file );
static long csx_util_drv_unlocked_ioctl( struct file *file, unsigned int cmd, unsigned long arg );
static void csx_util_drv_cleanup( void );

/* ---- Private Variables ------------------------------------------------- */
static char banner[] = KERN_INFO "CSX UTIL Driver: 1.00 (built on "__DATE__" "__TIME__")\n";
static int csx_util_chr_dev_registered = 0;
#if CONFIG_SYSFS
static struct class * csx_util_class = NULL;
static struct device * csx_util_dev[CSX_IO_UTIL_NUM_MAX] = {NULL};
#endif

/* File Operations (these are the device driver entry points) */
static struct file_operations gfops =
{
   .owner            = THIS_MODULE,
   .open             = csx_util_drv_open,
   .release          = csx_util_drv_release,
   .unlocked_ioctl   = csx_util_drv_unlocked_ioctl,
};

static int gDrvOpenStatus[CSX_IO_UTIL_NUM_MAX] = {0};   /* Open status flag */
static CSX_UTIL_CTRL_FNCS gUtilCtrlFncp[CSX_IO_UTIL_NUM_MAX] = {{NULL},{NULL}};

/* ---- Public Variables ------------------------------------------------- */
/* ---- Functions -------------------------------------------------------- */

/***************************************************************************/
/**
*  Driver open routine.
*
*  @return
*     0     - Success
*     -ve   - error code
*/
static int csx_util_drv_open( struct inode *inode, struct file *file )
{
   int csx_util_id = iminor(inode);

   if ( !gUtilCtrlFncp[csx_util_id].csx_util_get_name )
   {
      printk( KERN_ERR "%s: CSX utility %d not registered\n", __FUNCTION__, csx_util_id );
      return -ENODEV;
   }

   if ( !gDrvOpenStatus[csx_util_id] )
   {
      printk( KERN_INFO "%s: CSX utility %s opened\n", __FUNCTION__,
                        gUtilCtrlFncp[csx_util_id].csx_util_get_name() );
      gDrvOpenStatus[csx_util_id] = 1;
   }
   else
   {
      /* Only allow single instance access */
      return -EBUSY;
   }
   return 0;
}

/***************************************************************************/
/**
*  Driver release routine
*
*  @return
*     0     - Success
*     -ve   - error code
*/
static int csx_util_drv_release( struct inode *inode, struct file *file )
{
   int csx_util_id = iminor(inode);

   gDrvOpenStatus[csx_util_id] = 0;
   return 0;
}

/***************************************************************************/
/**
*  Driver I/O Control routine
*
*  @return
*     0     - Success
*     -ve   - error code
*/
static long csx_util_drv_unlocked_ioctl(
   struct file   *file,             /**< (io) File structure pointer */
   unsigned int   cmd,              /**< (i)  IOCTL command */
   unsigned long  arg               /**< (i)  User argument */
)
{
   int csx_util_id = iminor(file->f_path.dentry->d_inode);
   int rc;
   CSX_UTIL_IOCTL_PARMS parms;
   unsigned int cmdnr;
   unsigned int size;

   cmdnr    = _IOC_NR( cmd );
   size     = _IOC_SIZE( cmd );

   if ( !gUtilCtrlFncp[csx_util_id].csx_util_get_name )
   {
      printk( KERN_ERR "%s: (%d) CSX utility %d not registered\n", __FUNCTION__, cmdnr, csx_util_id );
      return -ENODEV;
   }

   if ( size > sizeof(parms) )
   {
      return -ENOMEM;
   }

   if ( size )
   {
      if (( rc = copy_from_user( &parms, (void *)arg, size )) != 0 )
      {
         printk( KERN_ERR "%s: (%d) FAILED copy_from_user status=%d\n", __FUNCTION__, cmdnr, rc );
         return rc;
      }
   }

   switch ( cmd )
   {
      case CSX_UTIL_IOCTL_ADD_POINT:
      {
         CSX_IO_POINT_INFO csx_info;
         CSX_UTIL_INFO *csx_util_info;
         unsigned int util_info_size = gUtilCtrlFncp[csx_util_id].csx_util_size_of_info_struct();
         CSX_IO_HANDLE handle;

         if (( rc = copy_from_user( &csx_info, parms.add_point_parms.csx_info, sizeof(CSX_IO_POINT_INFO) )))
         {
            printk( KERN_ERR "%s: (%d) FAILED copy_from_user status=%d\n", __FUNCTION__, cmdnr, rc );
            return rc;
         }

         /* Allocate enough memory to hold number of points specified */
         if ( (csx_util_info = vmalloc( util_info_size )) == NULL )
         {
            printk( KERN_ERR "%s: (%d) FAILED to allocate memory\n", __FUNCTION__, cmdnr );
            goto csx_util_add_point_cleanup;
         }

         if (( rc = copy_from_user( csx_util_info, parms.add_point_parms.csx_util_info, util_info_size )))
         {
            printk( KERN_ERR "%s: (%d) FAILED copy_from_user status=%d\n", __FUNCTION__, cmdnr, rc );
            goto csx_util_add_point_cleanup;
         }

         rc = gUtilCtrlFncp[csx_util_id].csx_util_add_point( &csx_info, csx_util_info, &handle );

         if ( rc || handle == CSX_HANDLE_INVALID )
         {
            printk( KERN_ERR "%s: (%d) FAILED csx_util_add_point status=%d\n", __FUNCTION__, cmdnr, rc );
            rc = -EINVAL;
            goto csx_util_add_point_cleanup;
         }

         if (( rc = copy_to_user( parms.add_point_parms.csx_handle, &handle, sizeof(CSX_IO_HANDLE) )))
         {
            printk( KERN_ERR "%s: (%d) FAILED copy_to_user status=%d\n", __FUNCTION__, cmdnr, rc );
            goto csx_util_add_point_cleanup;
         }

csx_util_add_point_cleanup:
         if ( csx_util_info )
         {
            vfree( csx_util_info );
         }
         break;
      }
      case CSX_UTIL_IOCTL_REMOVE_POINT:
      {
         rc = gUtilCtrlFncp[csx_util_id].csx_util_remove_point( parms.remove_point_parms.csx_handle );
         break;
      }
      case CSX_UTIL_IOCTL_GET_NUM_POINTS:
      {
         unsigned int num_points = gUtilCtrlFncp[csx_util_id].csx_util_get_num_active_points();

         if (( rc = copy_to_user( parms.get_num_points_parms.csx_num_points, &num_points, sizeof(unsigned int) )))
         {
            printk( KERN_ERR "%s: (%d) FAILED copy_to_user status=%d\n", __FUNCTION__, cmdnr, rc );
            return rc;
         }
         break;
      }
      case CSX_UTIL_IOCTL_QUERY_ALL:
      {
         unsigned int num_points;
         CSX_UTIL_POINT *csx_util_point;
         unsigned int util_point_size = gUtilCtrlFncp[csx_util_id].csx_util_size_of_point_struct();

         if (( rc = copy_from_user( &num_points, parms.query_all_parms.num_points, sizeof(unsigned int) )))
         {
            printk( KERN_ERR "%s: (%d) FAILED copy_from_user status=%d\n", __FUNCTION__, cmdnr, rc );
            return rc;
         }

         if ( num_points > gUtilCtrlFncp[csx_util_id].csx_util_get_max_points() )
         {
            /* Cannot exceed maximum number of points allowable */
            num_points = gUtilCtrlFncp[csx_util_id].csx_util_get_max_points();
         }

         /* Allocate enough memory to hold number of points specified */
         if ( (csx_util_point = vmalloc( num_points * util_point_size )) == NULL )
         {
            printk( KERN_ERR "%s: (%d) FAILED to allocate memory\n", __FUNCTION__, cmdnr );
            goto csx_util_query_cleanup;
         }

         if (( rc = gUtilCtrlFncp[csx_util_id].csx_util_query_all( csx_util_point, &num_points )))
         {
            printk( KERN_ERR "%s: (%d) FAILED csx_util_query_all status=%d\n", __FUNCTION__, cmdnr, rc );
            goto csx_util_query_cleanup;
         }

         if (( rc = copy_to_user( parms.query_all_parms.csx_util_point, csx_util_point, (num_points * util_point_size ))))
         {
            printk( KERN_ERR "%s: (%d) FAILED copy_to_user status=%d\n", __FUNCTION__, cmdnr, rc );
            goto csx_util_query_cleanup;
         }

         if (( rc = copy_to_user( parms.query_all_parms.num_points, &num_points, sizeof(unsigned int) )))
         {
            printk( KERN_ERR "%s: (%d) FAILED copy_to_user status=%d\n", __FUNCTION__, cmdnr, rc );
            goto csx_util_query_cleanup;
         }

csx_util_query_cleanup:
         if ( csx_util_point )
         {
            vfree( csx_util_point );
         }
         break;
      }
      case CSX_UTIL_IOCTL_SYNC_ENABLE:
      {
         rc = gUtilCtrlFncp[csx_util_id].csx_util_sync_enable( parms.sync_enable_parms.csx_io_module );
         break;
      }
      case CSX_UTIL_IOCTL_SYNC_DISABLE:
      {
         rc = gUtilCtrlFncp[csx_util_id].csx_util_sync_disable( parms.sync_disable_parms.csx_io_module );
         break;
      }
      default:
      {
         rc = -EINVAL;
      }
   }

   return rc;
}

/***************************************************************************/
/**
*  Registers character device for CSX utility
*
*  @return
*     0     - Success
*     -ve   - error code
*/
int csx_register_util_drv( CSX_IO_UTIL csx_util_id, CSX_UTIL_CTRL_FNCS *csx_util_ctrl_fncp )
{
   int err;

   /* Check if function pointers valid */
   if ( csx_util_ctrl_fncp == NULL )
      return -EINVAL;

   if ( csx_util_ctrl_fncp->csx_util_get_name == NULL )
      return -EINVAL;

   /* Check if utility already registered */
   if ( gUtilCtrlFncp[csx_util_id].csx_util_get_name )
   {
      printk( KERN_ERR "%s: %s utility is already registered.\n", __FUNCTION__, gUtilCtrlFncp[csx_util_id].csx_util_get_name() );
      return -EBUSY;
   }

   /* Register single character device with known major number */
   if ( !csx_util_chr_dev_registered )
   {
      printk( banner );

      err = register_chrdev( BCM_CSX_UTIL_MAJOR, "csx_util", &gfops );

      if ( err )
      {
         printk( KERN_ERR "%s: failed to register character device major=%d\n", __FUNCTION__, BCM_CSX_UTIL_MAJOR );
         goto error_cleanup;
      }
      csx_util_chr_dev_registered = 1;
   }

#if CONFIG_SYSFS
   /* Register device class */
   if ( csx_util_class == NULL )
   {
      csx_util_class = class_create(THIS_MODULE,"bcmring-csx-util");
      if ( IS_ERR(csx_util_class))
      {
         printk(KERN_ERR "%s: Class create failed\n", __FUNCTION__);
         err = -EFAULT;
         csx_util_class = NULL;
         goto error_cleanup;
      }
   }
#endif

#if CONFIG_SYSFS
   /* Register per minor number sysfs device */
   csx_util_dev[csx_util_id] = device_create( csx_util_class,
                                              NULL,
                                              MKDEV(BCM_CSX_UTIL_MAJOR,csx_util_id),
                                              NULL,
                                              csx_util_ctrl_fncp->csx_util_get_name());
   if ( IS_ERR(csx_util_dev[csx_util_id]) )
   {
      printk(KERN_ERR "%s: Device %s create failed\n", __FUNCTION__,csx_util_ctrl_fncp->csx_util_get_name());
      err = -EFAULT;
      goto error_cleanup;
   }
#endif

   /* Copy over function pointers */
   memcpy( &gUtilCtrlFncp[csx_util_id], csx_util_ctrl_fncp, sizeof(CSX_UTIL_CTRL_FNCS) );

   return 0;

error_cleanup:

   csx_util_drv_cleanup();
   return err;
}

/***************************************************************************/
/**
*  Unregisters character device for CSX utility
*
*  @return
*     None
*/
int csx_deregister_util_drv( CSX_IO_UTIL csx_util_id )
{
   /* Check if utility is actually registered */
   if ( !gUtilCtrlFncp[csx_util_id].csx_util_get_name )
   {
      printk( KERN_ERR "%s: utility %d is not registered.\n", __FUNCTION__, csx_util_id );
      return -ENODEV;
   }

#if CONFIG_SYSFS
   if ( csx_util_dev[csx_util_id] && !IS_ERR(csx_util_dev[csx_util_id]) )
   {
      device_destroy(csx_util_class,MKDEV(BCM_CSX_UTIL_MAJOR,csx_util_id));
      csx_util_dev[csx_util_id] = NULL;
   }
#endif

   /* Clear registration */
   memset (&gUtilCtrlFncp[csx_util_id], 0, sizeof(gUtilCtrlFncp[0]));

   /* Clean up common interface */

   csx_util_drv_cleanup();

   return 0;
}


/***************************************************************************/
/**
*  Clean up common driver interface
*
*  @return
*     None
*/
static void csx_util_drv_cleanup( void )
{
   int i;

   /* Proceed with cleanup if no utilities registered */
   for (i = 0; i < CSX_IO_UTIL_NUM_MAX; i++)
   {
      if (gUtilCtrlFncp[i].csx_util_get_name)
         return;
   }

#if CONFIG_SYSFS
   if ( csx_util_class && !IS_ERR(csx_util_class))
   {
      class_destroy(csx_util_class);
      csx_util_class = NULL;
   }
#endif

   if ( csx_util_chr_dev_registered )
   {
      unregister_chrdev( BCM_CSX_UTIL_MAJOR, "csx_util" );
      csx_util_chr_dev_registered = 0;
   }
}

