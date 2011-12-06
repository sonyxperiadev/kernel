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

#include <linux/videocore/vc_omx.h>


#define DRIVER_NAME  "vc-omx"
#define PROC_WRITE_BUF_SIZE      128

// Uncomment to enable debug logging
//#define ENABLE_DBG

#if defined(ENABLE_DBG)
#define LOG_DBG( fmt, ... )  printk( KERN_INFO fmt "\n", ##__VA_ARGS__ )
#else
#define LOG_DBG( fmt, ... )
#endif
#define LOG_ERR( fmt, ... )  printk( KERN_ERR fmt "\n", ##__VA_ARGS__ )

struct vc_omx_comp_supported
{
   int enabled;
   char name[OMX_PLUGIN__COMPONENT_NAME_LEN];
};

// Device (/dev) related variables
static dev_t         vc_omx_devnum = 0;
static struct class *vc_omx_class = NULL;
static struct cdev   vc_omx_cdev;
static struct vc_omx_comp_supported vc_omx_info[OMX_PLUGIN__COMPONENT_SUPPORTED__NUM + 1] =
{
   { 1, OMX_PLUGIN__MKNAME( OMX_PLUGIN__COMPONENT_SUPPORTED__VIDEO_DECODER, OMX_PLUGIN__COMPONENT_SUPPORTED__MPEG4 ) },
   { 1, OMX_PLUGIN__MKNAME( OMX_PLUGIN__COMPONENT_SUPPORTED__VIDEO_DECODER, OMX_PLUGIN__COMPONENT_SUPPORTED__H263 )  },
   { 1, OMX_PLUGIN__MKNAME( OMX_PLUGIN__COMPONENT_SUPPORTED__VIDEO_DECODER, OMX_PLUGIN__COMPONENT_SUPPORTED__AVC )   },
   { 1, OMX_PLUGIN__MKNAME( OMX_PLUGIN__COMPONENT_SUPPORTED__VIDEO_ENCODER, OMX_PLUGIN__COMPONENT_SUPPORTED__MPEG4 ) },
   { 1, OMX_PLUGIN__MKNAME( OMX_PLUGIN__COMPONENT_SUPPORTED__VIDEO_ENCODER, OMX_PLUGIN__COMPONENT_SUPPORTED__H263 )  },
   { 1, OMX_PLUGIN__MKNAME( OMX_PLUGIN__COMPONENT_SUPPORTED__VIDEO_ENCODER, OMX_PLUGIN__COMPONENT_SUPPORTED__AVC )   },
};
static struct vc_omx_comp_status vc_omx_status;
static struct vc_omx_enc_color vc_omx_color;
static struct vc_omx_and_ext vc_omx_andext;
// Proc entry
static struct proc_dir_entry *vc_omx_proc_entry;


/****************************************************************************
*
*   vc_omx_open
*
***************************************************************************/

static int vc_omx_open( struct inode *inode, struct file *file )
{
    (void)inode;
    (void)file;

    LOG_DBG( "[%s]: called file = 0x%p", __func__, file );

    return 0;
}

/****************************************************************************
*
*   vc_omx_release
*
***************************************************************************/

static int vc_omx_release( struct inode *inode, struct file *file )
{
    (void)inode;
    (void)file;

    LOG_DBG( "[%s]: called file = 0x%p", __func__, file );

    return 0;
}


/****************************************************************************
*
*   vc_omx_ioctl
*
***************************************************************************/

static long vc_omx_ioctl( struct file *file, unsigned int cmd, unsigned long arg )
{
    int rc = 0;
    int index;

    (void)cmd;

    LOG_DBG( "[%s]: called file = 0x%p", __func__, file );

    switch ( cmd )
    {
        case VC_OMX_IOC_COMP_STATUS:
        {
            if ( copy_from_user( &vc_omx_status,
                                 (void *)arg,
                                 sizeof( vc_omx_status )) != 0 )
            {
               rc = -EFAULT;
            }

            for ( index = 0 ; index < OMX_PLUGIN__COMPONENT_SUPPORTED__NUM ; index++ )
            {
               if ( strcmp( vc_omx_status.name,
                            vc_omx_info[index].name ) == 0 )
               {
                  vc_omx_status.enabled = vc_omx_info[index].enabled;
                  break;
               }
            }

            if ( copy_to_user( (void *)arg,
                               &vc_omx_status,
                               sizeof( vc_omx_status )) != 0 )
            {
               rc = -EFAULT;
            }
            break;
        }

        case VC_OMX_IOC_ENC_COLOR:
        {
            if ( copy_to_user( (void *)arg,
                               &vc_omx_color,
                               sizeof( vc_omx_color )) != 0 )
            {
               rc = -EFAULT;
            }
            break;
        }

        case VC_OMX_IOC_AND_EXT:
        {
            if ( copy_to_user( (void *)arg,
                               &vc_omx_andext,
                               sizeof( vc_omx_andext )) != 0 )
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

static const struct file_operations vc_omx_fops =
{
    .owner          = THIS_MODULE,
    .open           = vc_omx_open,
    .release        = vc_omx_release,
    .unlocked_ioctl = vc_omx_ioctl,
};

/****************************************************************************
*
*   vc_omx_proc_read
*
***************************************************************************/

static int vc_omx_proc_read( char *buf, char **start, off_t offset, int count, int *eof, void *data )
{
    char *p = buf;
    int index;

    (void)start;
    (void)count;
    (void)data;

    if ( offset > 0 )
    {
       *eof = 1;
       return 0;
    }

    p += sprintf( p, "Registered OMX Plugin Components:\n\n" );
    for ( index = 0 ; index < OMX_PLUGIN__COMPONENT_SUPPORTED__NUM ; index++ )
    {
       p += sprintf( p, "\t%s\t%s\n",
                     vc_omx_info[index].name,
                     vc_omx_info[index].enabled ? "Enabled" : "DISABLED" );
    }

    p += sprintf( p, "\nPreferred Color Format Selected: %d\n\n",
                  vc_omx_color.format );
    p += sprintf( p, "\t(supported format): %d\tYUV-420\n",
                  OMX_PLUGIN__ENCODING__YUV420 );
    p += sprintf( p, "\t(supported format): %d\tYUVUV-128\n",
                  OMX_PLUGIN__ENCODING__YUVUV128 );
    p += sprintf( p, "\t(supported format): %d\tOPAQUE\n",
                  OMX_PLUGIN__ENCODING__OPAQUE );

    p += sprintf( p, "\nAndroid OpenMAX Extension Support: %s\n",
                  (vc_omx_andext.andext == 0) ? "non" : "oui" );

    p += sprintf( p, "\nPrevent OpenMAX Component Creation: %s\n\n",
                  (vc_omx_andext.nocreate == 0) ? "non" : "oui" );

    *eof = 1;
    return p - buf;
}

/****************************************************************************
*
*   vc_omx_proc_read
*
***************************************************************************/

static int vc_omx_proc_write( struct file *file, const char __user *buffer, unsigned long count, void *data )
{
   int ret;
   unsigned char kbuf[PROC_WRITE_BUF_SIZE+1];
   char name[PROC_WRITE_BUF_SIZE];
   unsigned int value;
   int index;

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

   if ( strcmp( name, "color" ) == 0 )
   {
      if ( (value >= OMX_PLUGIN__ENCODING__YUV420) &&
           (value <= OMX_PLUGIN__ENCODING__OPAQUE) )
      {
         vc_omx_color.format = value;
         goto out;
      }
   }
   else if ( strcmp( name, "andext" ) == 0 )
   {
      if ( (value >= 0) &&
           (value <= 1) )
      {
         vc_omx_andext.andext = value;
         goto out;
      }
   }
   else if ( strcmp( name, "nocreate" ) == 0 )
   {
      if ( (value >= 0) &&
           (value <= 1) )
      {
         vc_omx_andext.nocreate = value;
         goto out;
      }
   }
   else
   {
      for ( index = 0 ; index < OMX_PLUGIN__COMPONENT_SUPPORTED__NUM ; index++ )
      {
         if ( strcmp( name,
                      vc_omx_info[index].name ) == 0 )
         {
            vc_omx_info[index].enabled = (value > 0 ? 1 : 0);
            goto out;
         }
      }
   }

   LOG_ERR( "[%s]: omx-component '%s' is **unknown** to driver '%s'",
            __func__,
            name,
            DRIVER_NAME );
   
out:
   return ret;
}

/****************************************************************************
*
*   vc_omx_init
*
***************************************************************************/

static int __init vc_omx_init( void )
{
    int rc;
    struct device *dev;

    LOG_DBG( "[%s]: called", __func__ );

    if (( rc = alloc_chrdev_region( &vc_omx_devnum, 0, 1, DRIVER_NAME )) < 0 )
    {
        LOG_ERR( "[%s]: alloc_chrdev_region failed (rc=%d)", __func__, rc );
        goto out_err;
    }

    cdev_init( &vc_omx_cdev, &vc_omx_fops );
    if (( rc = cdev_add( &vc_omx_cdev, vc_omx_devnum, 1 )) != 0 )
    {
        LOG_ERR( "[%s]: cdev_add failed (rc=%d)", __func__, rc );
        goto out_unregister;
    }

    vc_omx_class = class_create( THIS_MODULE, DRIVER_NAME );
    if ( IS_ERR( vc_omx_class ))
    {
        rc = PTR_ERR( vc_omx_class );
        LOG_ERR( "[%s]: class_create failed (rc=%d)", __func__, rc );
        goto out_cdev_del;
    }

    dev = device_create( vc_omx_class, NULL, vc_omx_devnum, NULL,
                         DRIVER_NAME );
    if ( IS_ERR( dev ))
    {
        rc = PTR_ERR( dev );
        LOG_ERR( "[%s]: device_create failed (rc=%d)", __func__, rc );
        goto out_class_destroy;
    }

    vc_omx_proc_entry = create_proc_entry( DRIVER_NAME, 0660, NULL );
    if ( vc_omx_proc_entry == NULL )
    {
        rc = -EFAULT;
        LOG_ERR( "[%s]: create_proc_entry failed", __func__ );
        goto out_device_destroy;
    }
    vc_omx_proc_entry->read_proc = vc_omx_proc_read;
    vc_omx_proc_entry->write_proc = vc_omx_proc_write;

    /* Init configuration data.
    */
    vc_omx_color.format = OMX_PLUGIN__ENCODING__YUVUV128;
    vc_omx_andext.andext = 1;
    vc_omx_andext.nocreate = 0;

    return 0;

out_device_destroy:
    device_destroy( vc_omx_class, vc_omx_devnum );

out_class_destroy:
    class_destroy( vc_omx_class );
    vc_omx_class = NULL;

out_cdev_del:
    cdev_del( &vc_omx_cdev );

out_unregister:
    unregister_chrdev_region( vc_omx_devnum, 1 );

out_err:
    return rc;
}

/****************************************************************************
*
*   vc_omx_exit
*
***************************************************************************/

static void __exit vc_omx_exit( void )
{
    LOG_DBG( "[%s]: called", __func__ );

    remove_proc_entry( vc_omx_proc_entry->name, NULL );
    device_destroy( vc_omx_class, vc_omx_devnum );
    class_destroy( vc_omx_class );
    cdev_del( &vc_omx_cdev );
    unregister_chrdev_region( vc_omx_devnum, 1 );
}

module_init( vc_omx_init );
module_exit( vc_omx_exit );
MODULE_LICENSE( "GPL" );
MODULE_AUTHOR( "Broadcom Corporation" );
