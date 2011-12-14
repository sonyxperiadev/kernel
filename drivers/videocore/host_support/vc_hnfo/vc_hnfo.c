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

#include <vc_hnfo.h>


#define DRIVER_NAME  "vc-hnfo"
#define PROC_WRITE_BUF_SIZE      128

// Uncomment to enable debug logging
//#define ENABLE_DBG

#if defined(ENABLE_DBG)
#define LOG_DBG( fmt, ... )  printk( KERN_INFO fmt "\n", ##__VA_ARGS__ )
#else
#define LOG_DBG( fmt, ... )
#endif
#define LOG_ERR( fmt, ... )  printk( KERN_ERR fmt "\n", ##__VA_ARGS__ )

// Device (/dev) related variables
static dev_t         vc_hnfo_devnum = 0;
static struct class *vc_hnfo_class = NULL;
static struct cdev   vc_hnfo_cdev;

static struct vc_hnfo_usr_info vc_hnfo_status;
// Proc entry
static struct proc_dir_entry *vc_hnfo_proc_entry;


/****************************************************************************
*
*   vc_hnfo_open
*
***************************************************************************/

static int vc_hnfo_open( struct inode *inode, struct file *file )
{
    (void)inode;
    (void)file;

    LOG_DBG( "[%s]: called file = 0x%p", __func__, file );

    return 0;
}

/****************************************************************************
*
*   vc_hnfo_release
*
***************************************************************************/

static int vc_hnfo_release( struct inode *inode, struct file *file )
{
    (void)inode;
    (void)file;

    LOG_DBG( "[%s]: called file = 0x%p", __func__, file );

    return 0;
}


/****************************************************************************
*
*   vc_hnfo_ioctl
*
***************************************************************************/

static long vc_hnfo_ioctl( struct file *file, unsigned int cmd, unsigned long arg )
{
    int rc = 0;

    (void)cmd;

    LOG_DBG( "[%s]: called file = 0x%p", __func__, file );

    switch ( cmd )
    {
        case VC_HNFO_IOC_INFO:
        {
            if ( copy_to_user( (void *)arg,
                               &vc_hnfo_status,
                               sizeof( vc_hnfo_status )) != 0 )
            {
               rc = -EFAULT;
            }
            break;
        }
        default:
        {
            return -ENOTTY;
        }
    }
    LOG_DBG( "[%s]: file = 0x%p returning %d", __func__, file, rc );

    return rc;
}

/****************************************************************************
*
*   File Operations for the driver.
*
***************************************************************************/

static const struct file_operations vc_hnfo_fops =
{
    .owner          = THIS_MODULE,
    .open           = vc_hnfo_open,
    .release        = vc_hnfo_release,
    .unlocked_ioctl = vc_hnfo_ioctl,
};

/****************************************************************************
*
*   vc_hnfo_proc_read
*
***************************************************************************/

static int vc_hnfo_proc_read( char *buf, char **start, off_t offset, int count, int *eof, void *data )
{
    char *p = buf;

    (void)start;
    (void)count;
    (void)data;

    if ( offset > 0 )
    {
       *eof = 1;
       return 0;
    }

    p += sprintf( p, "HDMI Default Setup:\n\n" );
    p += sprintf( p, "     \'power_on\'    : %s\n",
                  vc_hnfo_status.power_on ? "explicit" : "per-detection" );
    p += sprintf( p, "     \'dis_detect\'  : %s\n",
                  vc_hnfo_status.dis_detect ? "oui" : "non" );
    p += sprintf( p, "     \'resolution\'  : %d\n",
                  vc_hnfo_status.resolution );
    p += sprintf( p, "     \'dis_clone\'   : %s\n",
                  vc_hnfo_status.dis_clone ? "oui" : "non" );

    *eof = 1;
    return p - buf;
}

/****************************************************************************
*
*   vc_hnfo_proc_read
*
***************************************************************************/

static int vc_hnfo_proc_write( struct file *file, const char __user *buffer, unsigned long count, void *data )
{
   int ret;
   unsigned char kbuf[PROC_WRITE_BUF_SIZE+1];
   char name[PROC_WRITE_BUF_SIZE];
   unsigned int value;

   if ( count >= PROC_WRITE_BUF_SIZE )
   {
      count = PROC_WRITE_BUF_SIZE;
   }

   memset( kbuf, 0, PROC_WRITE_BUF_SIZE+1 );
   if ( copy_from_user( kbuf,
                        buffer,
                        count ) != 0 )
   {
      LOG_ERR( "[%s]: failed to copy-from-user",
               __func__ );

      ret = -EFAULT;
      goto out;
   }

   /* Return read value no matter what from there on.
   */
   kbuf[ count ] = '\0';
   ret = count;

   if( sscanf( kbuf, "%s %u", name, &value ) != 2 )
   {
      LOG_ERR( "[%s]: echo <name> <value> > /proc/%s",
               __func__,
               DRIVER_NAME );

      /* Failed to assign the proper value.
      */
      goto out;
   }

   /* Big case to assign the desired value.  Make this a lookup table
   ** eventually.
   */
   if ( strcmp( name, "power_on" ) == 0 )
   {
      vc_hnfo_status.power_on = value;
   }
   else if ( strcmp( name, "dis_detect" ) == 0 )
   {
      vc_hnfo_status.dis_detect = value;
   }
   else if ( strcmp( name, "dis_clone" ) == 0 )
   {
      vc_hnfo_status.dis_clone = value;
   }
   else if ( strcmp( name, "resolution" ) == 0 )
   {
      vc_hnfo_status.resolution = value;
   }
   else
   {
      LOG_ERR( "[%s]: paramerter '%s' is **unknown** to driver '%s'",
               __func__,
               name,
               DRIVER_NAME );
   }


out:
   return ret;
}

/****************************************************************************
*
*   vc_hnfo_init
*
***************************************************************************/

static int __init vc_hnfo_init( void )
{
    int rc;
    struct device *dev;

    LOG_DBG( "[%s]: called", __func__ );

    if (( rc = alloc_chrdev_region( &vc_hnfo_devnum, 0, 1, DRIVER_NAME )) < 0 )
    {
        LOG_ERR( "[%s]: alloc_chrdev_region failed (rc=%d)", __func__, rc );
        goto out_err;
    }

    cdev_init( &vc_hnfo_cdev, &vc_hnfo_fops );
    if (( rc = cdev_add( &vc_hnfo_cdev, vc_hnfo_devnum, 1 )) != 0 )
    {
        LOG_ERR( "[%s]: cdev_add failed (rc=%d)", __func__, rc );
        goto out_unregister;
    }

    vc_hnfo_class = class_create( THIS_MODULE, DRIVER_NAME );
    if ( IS_ERR( vc_hnfo_class ))
    {
        rc = PTR_ERR( vc_hnfo_class );
        LOG_ERR( "[%s]: class_create failed (rc=%d)", __func__, rc );
        goto out_cdev_del;
    }

    dev = device_create( vc_hnfo_class, NULL, vc_hnfo_devnum, NULL,
                         DRIVER_NAME );
    if ( IS_ERR( dev ))
    {
        rc = PTR_ERR( dev );
        LOG_ERR( "[%s]: device_create failed (rc=%d)", __func__, rc );
        goto out_class_destroy;
    }

    vc_hnfo_proc_entry = create_proc_entry( DRIVER_NAME, 0660, NULL );
    if ( vc_hnfo_proc_entry == NULL )
    {
        rc = -EFAULT;
        LOG_ERR( "[%s]: create_proc_entry failed", __func__ );
        goto out_device_destroy;
    }
    vc_hnfo_proc_entry->read_proc = vc_hnfo_proc_read;
    vc_hnfo_proc_entry->write_proc = vc_hnfo_proc_write;

    /* Default setting for configuration entries.
    */
    vc_hnfo_status.power_on   = 0;   // No explicit power on, power on detection.
    vc_hnfo_status.dis_detect = 0;   // Don't disable action on detection.
    vc_hnfo_status.resolution = 19;  // 'HDMI_CEA_720p50' - Highest supported resolution preferred.
                                     // Can't ask for more at this time (ie 'HDMI_CEA_1080p60'=16).
    vc_hnfo_status.dis_clone = 0;    // Don't disable GUI cloning.

    return 0;

out_device_destroy:
    device_destroy( vc_hnfo_class, vc_hnfo_devnum );

out_class_destroy:
    class_destroy( vc_hnfo_class );
    vc_hnfo_class = NULL;

out_cdev_del:
    cdev_del( &vc_hnfo_cdev );

out_unregister:
    unregister_chrdev_region( vc_hnfo_devnum, 1 );

out_err:
    return rc;
}

/****************************************************************************
*
*   vc_hnfo_exit
*
***************************************************************************/

static void __exit vc_hnfo_exit( void )
{
    LOG_DBG( "[%s]: called", __func__ );

    remove_proc_entry( vc_hnfo_proc_entry->name, NULL );
    device_destroy( vc_hnfo_class, vc_hnfo_devnum );
    class_destroy( vc_hnfo_class );
    cdev_del( &vc_hnfo_cdev );
    unregister_chrdev_region( vc_hnfo_devnum, 1 );
}

module_init( vc_hnfo_init );
module_exit( vc_hnfo_exit );
MODULE_LICENSE( "GPL" );
MODULE_AUTHOR( "Broadcom Corporation" );
