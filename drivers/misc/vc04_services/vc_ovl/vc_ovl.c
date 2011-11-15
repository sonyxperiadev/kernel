/*****************************************************************************
* Copyright 2010 - 2011 Broadcom Corporation.  All rights reserved.
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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/list.h>

#include <linux/videocore/vc_ovl.h>


#define DRIVER_NAME  "vc-ovl"
#define PROC_WRITE_BUF_SIZE      128

// Uncomment to enable debug logging
//#define ENABLE_DBG

#if defined(ENABLE_DBG)
#define LOG_DBG( fmt, ... )  printk( KERN_INFO fmt "\n", ##__VA_ARGS__ )
#else
#define LOG_DBG( fmt, ... )
#endif
#define LOG_ERR( fmt, ... )  printk( KERN_ERR fmt "\n", ##__VA_ARGS__ )

typedef struct vc_ovl_data_int
{
   struct vc_ovl_data_int    *next;          // Linked list next pointer

   struct vc_ovl_info        ovl_info;
   struct vc_ovl_handle      ovl_handle;
   unsigned int              ovl_changed;
   unsigned int              ovl_pending;

} vc_ovl_data_int_t;


// Device (/dev) related variables
static dev_t                  vc_ovl_devnum = 0;
static struct class           *vc_ovl_class = NULL;
static struct cdev            vc_ovl_cdev;
static struct mutex           vc_ovl_lock;
static vc_ovl_data_int_t      *vc_ovl_list = NULL;
// Proc entry
static struct proc_dir_entry *vc_ovl_proc_entry;


static void vc_ovl_add( vc_ovl_data_int_t *ovl )
{
   mutex_lock( &vc_ovl_lock );

   if ( vc_ovl_list == NULL )
   {
      vc_ovl_list = ovl;
   }
   else
   {
      vc_ovl_data_int_t *ptr = vc_ovl_list;

      while ( ptr->next != NULL )
      {
         ptr = ptr->next;
      }

      ptr->next = ovl;
   }

   ovl->next = NULL;
   mutex_unlock( &vc_ovl_lock );
   return;
}

static void vc_ovl_rem( vc_ovl_data_int_t **ovl )
{
   vc_ovl_data_int_t *prev_ptr = NULL;
   vc_ovl_data_int_t *curr_ptr = vc_ovl_list;

   mutex_lock( &vc_ovl_lock );
   while ( curr_ptr != NULL )
   {
      if ( curr_ptr == *ovl )
      {
         if ( curr_ptr == vc_ovl_list )
         {
            vc_ovl_list = curr_ptr->next;
         }
         else
         {
            prev_ptr->next = curr_ptr->next;
         }

         kfree( *ovl );
         *ovl = NULL;

         mutex_unlock( &vc_ovl_lock );
         return;
      }
      else
      {
         prev_ptr = curr_ptr;
         curr_ptr = curr_ptr->next;
      }
   }

   // We should free the memory anyway to avoid a memory leak
   kfree( *ovl );
   *ovl = NULL;

   mutex_unlock( &vc_ovl_lock );
   return;
}

static vc_ovl_data_int_t *vc_ovl_get( int handle )
{
   vc_ovl_data_int_t *ovl;

   mutex_lock( &vc_ovl_lock );
   if ( vc_ovl_list == NULL )
   {
      mutex_unlock( &vc_ovl_lock );
      return NULL;
   }

   // Start at the head of the list
   ovl = vc_ovl_list;

   do
   {
      if ( ovl->ovl_handle.handle == handle )
      {
         mutex_unlock( &vc_ovl_lock );
         return ovl;
      }

      // Move on to next entry in list
      ovl = ovl->next;
   } while ( ovl != NULL );

   mutex_unlock( &vc_ovl_lock );
   return NULL;
}


/****************************************************************************
*
*   vc_ovl_open
*
***************************************************************************/

static int vc_ovl_open( struct inode *inode, struct file *file )
{
    (void)inode;
    (void)file;

    LOG_DBG( "[%s]: called file = 0x%p", __func__, file );

    return 0;
}

/****************************************************************************
*
*   vc_ovl_release
*
***************************************************************************/

static int vc_ovl_release( struct inode *inode, struct file *file )
{
    (void)inode;
    (void)file;

    LOG_DBG( "[%s]: called file = 0x%p", __func__, file );

    return 0;
}


/****************************************************************************
*
*   vc_ovl_ioctl
*
***************************************************************************/

static long vc_ovl_ioctl( struct file *file, unsigned int cmd, unsigned long arg )
{
   int rc = 0;
   struct vc_ovl_handle ovl_hdl;
   struct vc_ovl_data ovl_data;
   vc_ovl_data_int_t *ovl;

   LOG_DBG( "[%s]: called file = 0x%p", __func__, file );

   switch ( cmd )
   {
      case VC_OVL_IOC_OVL_NEW:
      {
         if ( copy_from_user( &ovl_hdl,
                              (void *)arg,
                              sizeof( struct vc_ovl_handle )) != 0 )
         {
            rc = -EFAULT;
            goto out;
         }

         ovl = vc_ovl_get ( ovl_hdl.handle );
         if ( ovl != NULL )
         {
            // Already there...  ignore.
            goto out;
         }

         ovl = kzalloc( sizeof( struct vc_ovl_data_int ), GFP_KERNEL );
         if ( ovl == NULL )
         {
            rc = -ENOMEM;
            goto out;
         }

         ovl->ovl_handle.handle = ovl_hdl.handle;
         memset ( &(ovl->ovl_info), 0, sizeof( struct vc_ovl_info ) );
         vc_ovl_add ( ovl );
         break;
      }

      case VC_OVL_IOC_OVL_DEL:
      {
         if ( copy_from_user( &ovl_hdl,
                              (void *)arg,
                              sizeof( struct vc_ovl_handle )) != 0 )
         {
            rc = -EFAULT;
            goto out;
         }

         ovl = vc_ovl_get ( ovl_hdl.handle );
         if ( ovl == NULL )
         {
            // Not found...  ignore.
            goto out;
         }

         vc_ovl_rem ( &ovl );
         break;
      }

      case VC_OVL_IOC_OVL_GET:
      {
         if ( copy_from_user( &ovl_data,
                              (void *)arg,
                              sizeof( struct vc_ovl_data )) != 0 )
         {
            rc = -EFAULT;
            goto out;
         }

         ovl = vc_ovl_get ( ovl_data.ovl_handle.handle );
         if ( ovl == NULL )
         {
            rc = -EINVAL;
            goto out;
         }

         memcpy ( &(ovl_data.ovl_info),
                  &(ovl->ovl_info),
                  sizeof( struct vc_ovl_info ) );

         if ( copy_to_user( (void *)arg,
                            &ovl_data,
                            sizeof( struct vc_ovl_data )) != 0 )
         {
            rc = -EFAULT;
            goto out;
         }

         break;
      }

      case VC_OVL_IOC_OVL_SET:
      {
         if ( copy_from_user( &ovl_data,
                              (void *)arg,
                              sizeof( struct vc_ovl_data )) != 0 )
         {
            rc = -EFAULT;
            goto out;
         }

         ovl = vc_ovl_get ( ovl_data.ovl_handle.handle );
         if ( ovl == NULL )
         {
            // Not found...  ignore.
            goto out;
         }
         
         if ( memcmp ( &(ovl->ovl_info),
                       &(ovl_data.ovl_info),
                       sizeof( struct vc_ovl_info ) ) != 0 )
         {
            memcpy ( &(ovl->ovl_info),
                     &(ovl_data.ovl_info),
                     sizeof( struct vc_ovl_info ) );

            ovl->ovl_pending = 1;
         }
         break;
      }

      case VC_OVL_IOC_OVL_CHG_SET:
      {
         if ( copy_from_user( &ovl_hdl,
                              (void *)arg,
                              sizeof( struct vc_ovl_handle )) != 0 )
         {
            rc = -EFAULT;
            goto out;
         }

         ovl = vc_ovl_get ( ovl_hdl.handle );
         if ( ovl == NULL )
         {
            // Not found...  ignore.
            goto out;
         }

         // Overlay characteristics have changed.
         if ( ovl->ovl_pending )
         {
            ovl->ovl_changed = 1;
            ovl->ovl_pending = 0;
         }
         break;
      }

      case VC_OVL_IOC_OVL_CHG_CHK:
      {
         if ( copy_from_user( &ovl_hdl,
                              (void *)arg,
                              sizeof( struct vc_ovl_handle )) != 0 )
         {
            rc = -EFAULT;
            goto out;
         }

         ovl = vc_ovl_get ( ovl_hdl.handle );
         if ( ovl == NULL )
         {
            rc = -EINVAL;
            goto out;
         }

         if ( ovl->ovl_changed )
         {
            rc = 0;
         }
         else
         {
            // No change...  ignore.
            rc = -EINVAL;
         }

         goto out;
         break;
      }

      case VC_OVL_IOC_OVL_CHG_CLR:
      {
         if ( copy_from_user( &ovl_hdl,
                              (void *)arg,
                              sizeof( struct vc_ovl_handle )) != 0 )
         {
            rc = -EFAULT;
            goto out;
         }

         ovl = vc_ovl_get ( ovl_hdl.handle );
         if ( ovl == NULL )
         {
            rc = -EINVAL;
            goto out;
         }

         // Overlay characteristics was changed and this change has been picked
         // up and dealt with, so reset the flag now.
         if ( ovl->ovl_changed )
         {
            ovl->ovl_pending = 0;
            ovl->ovl_changed = 0;
         }

         break;
      }
    
      default:
      {
         rc = -ENOTTY;
         goto out;
      }
   }

   LOG_DBG( "[%s]: file = 0x%p returning %d", __func__, file, rc );

out:
    return rc;
}

/****************************************************************************
*
*   File Operations for the driver.
*
***************************************************************************/

static const struct file_operations vc_ovl_fops =
{
    .owner          = THIS_MODULE,
    .open           = vc_ovl_open,
    .release        = vc_ovl_release,
    .unlocked_ioctl = vc_ovl_ioctl,
};

/****************************************************************************
*
*   vc_ovl_proc_read
*
***************************************************************************/

static int vc_ovl_proc_read( char *buf, char **start, off_t offset, int count, int *eof, void *data )
{
   char *p = buf;
   int ix = 0;
   vc_ovl_data_int_t *ovl = vc_ovl_list;

   (void)start;
   (void)count;
   (void)data;

   if ( offset > 0 )
   {
      *eof = 1;
      return 0;
   }

   mutex_lock ( &vc_ovl_lock );
   while ( ovl != NULL )
   {
      ix++;
      p += sprintf( p, "\tovl: %x\n",
                    ovl->ovl_handle.handle );
      p += sprintf( p, "\torig: (%dx%d, fmt:%d)\n",
                    ovl->ovl_info.in_width,
                    ovl->ovl_info.in_height,
                    ovl->ovl_info.format );
      p += sprintf( p, "\tpos: (x:%d, y:%d, %dx%d, r:%d, h:%d, v:%d, l:%d)\n",
                    ovl->ovl_info.xaxis,
                    ovl->ovl_info.yaxis,
                    ovl->ovl_info.width,
                    ovl->ovl_info.height,
                    ovl->ovl_info.rotation,
                    ovl->ovl_info.h_flip,
                    ovl->ovl_info.v_flip,
                    ovl->ovl_info.layer );
      p += sprintf( p, "\tstatus: c:%s, p:%s\n",
                    ovl->ovl_changed ? "yes" : "no",
                    ovl->ovl_pending ? "yes" : "no" );

      ovl = ovl->next;
   }
   p += sprintf( p, "\n%d overlay registered\n", ix );
   mutex_unlock ( &vc_ovl_lock );

   *eof = 1;
   return p - buf;
}


/****************************************************************************
*
*   vc_ovl_init
*
***************************************************************************/

static int __init vc_ovl_init( void )
{
    int rc;
    struct device *dev;

    LOG_DBG( "[%s]: called", __func__ );

    if (( rc = alloc_chrdev_region( &vc_ovl_devnum, 0, 1, DRIVER_NAME )) < 0 )
    {
        LOG_ERR( "[%s]: alloc_chrdev_region failed (rc=%d)", __func__, rc );
        goto out_err;
    }

    cdev_init( &vc_ovl_cdev, &vc_ovl_fops );
    if (( rc = cdev_add( &vc_ovl_cdev, vc_ovl_devnum, 1 )) != 0 )
    {
        LOG_ERR( "[%s]: cdev_add failed (rc=%d)", __func__, rc );
        goto out_unregister;
    }

    vc_ovl_class = class_create( THIS_MODULE, DRIVER_NAME );
    if ( IS_ERR( vc_ovl_class ))
    {
        rc = PTR_ERR( vc_ovl_class );
        LOG_ERR( "[%s]: class_create failed (rc=%d)", __func__, rc );
        goto out_cdev_del;
    }

    dev = device_create( vc_ovl_class, NULL, vc_ovl_devnum, NULL,
                         DRIVER_NAME );
    if ( IS_ERR( dev ))
    {
        rc = PTR_ERR( dev );
        LOG_ERR( "[%s]: device_create failed (rc=%d)", __func__, rc );
        goto out_class_destroy;
    }

    vc_ovl_proc_entry = create_proc_entry( DRIVER_NAME, 0660, NULL );
    if ( vc_ovl_proc_entry == NULL )
    {
        rc = -EFAULT;
        LOG_ERR( "[%s]: create_proc_entry failed", __func__ );
        goto out_device_destroy;
    }
    vc_ovl_proc_entry->read_proc = vc_ovl_proc_read;

    mutex_init( &vc_ovl_lock );

    return 0;

out_device_destroy:
    device_destroy( vc_ovl_class, vc_ovl_devnum );

out_class_destroy:
    class_destroy( vc_ovl_class );
    vc_ovl_class = NULL;

out_cdev_del:
    cdev_del( &vc_ovl_cdev );

out_unregister:
    unregister_chrdev_region( vc_ovl_devnum, 1 );

out_err:
    return rc;
}

/****************************************************************************
*
*   vc_ovl_exit
*
***************************************************************************/

static void __exit vc_ovl_exit( void )
{
    LOG_DBG( "[%s]: called", __func__ );

    remove_proc_entry( vc_ovl_proc_entry->name, NULL );
    device_destroy( vc_ovl_class, vc_ovl_devnum );
    class_destroy( vc_ovl_class );
    cdev_del( &vc_ovl_cdev );
    unregister_chrdev_region( vc_ovl_devnum, 1 );
    mutex_destroy( &vc_ovl_lock );
}

module_init( vc_ovl_init );
module_exit( vc_ovl_exit );
MODULE_LICENSE( "GPL" );
MODULE_AUTHOR( "Broadcom Corporation" );
