/*****************************************************************************
* Copyright 2011 Broadcom Corporation.  All rights reserved.
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
*  @file    hdmi.c
*
*  @brief   Implements simple GPIO-based HDMI hotplug detection interface
*
****************************************************************************/

/* ---- Include Files ---------------------------------------------------- */
#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/spinlock.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/poll.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/gpio.h>
#include <linux/proc_fs.h>
#include <linux/platform_device.h>
#include <asm/gpio.h>
#ifdef CONFIG_BCM_HDMI_DET_SWITCH
#include <linux/switch.h>
#endif

#include <linux/broadcom/hdmi.h>
#include <linux/broadcom/hdmi_cfg.h>

#define HDMI_DEBUG
#define HDMI_TRACE

#ifdef HDMI_DEBUG
   #define LOG_DEBUG(X) (X)
#else
   #define LOG_DEBUG(X)
#endif

#ifdef HDMI_TRACE
   #define LOG_TRACE(X) (X)
#else
   #define LOG_TRACE(X)
#endif

#define DRIVER_NAME "hdmi-detect"

#define HDMI_CONNECTED_STR "CONNECTED"
#define HDMI_UNPLUGGED_STR "UNPLUGGED"

#define PROC_HDMI_HPD "hdmi-hpd"
#define PROC_WRITE_BUF_SIZE 128
static struct proc_dir_entry *hdmi_proc_entry;

/* ---- Public Variables ------------------------------------------------- */
/* ---- Private Constants and Types -------------------------------------- */

/* hdmi channel info structure */
struct hdmi_info
{
   int               changed;       /* Changed flag used by poll */
   unsigned long     changed_at;    /* When the HPD was last updated */
   unsigned long     irq_at;        /* Last HPD interrupt */
   hdmi_state     state;            /* Current state */
   wait_queue_head_t waitq;         /* wait queue */
   struct hdmi_hw_cfg hw_cfg;       /* board hw configuration */
   struct timer_list hpd_timer;     /* timer list for delayed fn execution */
};

/* ---- Private Variables ------------------------------------------------ */

#ifdef CONFIG_SYSFS
static dev_t hdmi_devnum = 0;
static struct class *hdmi_class = NULL;
#ifdef CONFIG_BCM_HDMI_DET_SWITCH
static struct switch_dev hdmi_hpd_switch;
#endif
static struct cdev hdmi_cdev;
#endif

/* hdmi channel information */
static struct hdmi_info gHDMI[1] = {{0}};
static spinlock_t detlock = __SPIN_LOCK_UNLOCKED();

/* ---- Private Function Prototypes -------------------------------------- */
static int  hdmi_open( struct inode *inode, struct file *file );
static long  hdmi_ioctl( struct file *file, unsigned int cmd, unsigned long arg );
static ssize_t hdmi_read( struct file *file, char __user *buf, size_t count,
      loff_t *f_pos);
static void hdmi_det_timer_fn(unsigned long data);
static unsigned int hdmi_poll( struct file *file, struct poll_table_struct *poll_table );

/* File Operations (these are the device driver entry points) */
struct file_operations hdmi_fops =
{
   .owner   = THIS_MODULE,
   .open    = hdmi_open,
   .unlocked_ioctl = hdmi_ioctl,
   .poll    = hdmi_poll,
   .read    = hdmi_read,
};

/* ---- Functions -------------------------------------------------------- */

/** Read the HPD GPIO, update driver state and schedule work queue fn
 * to update the Android HPD switch.
 * TODO HPD switch is not the same as HDMI switch because Android expects
 * HDMI switch to be set once audio is usable.
 * Perhaps this driver could accept an IOCTL to configure the HDMI switch
 * once tvserver has done it's work
 */
static void check_hdmi_det_gpio( struct hdmi_info *ch )
{
   LOG_TRACE(printk("check_hdmi_det_gpio %p\n", ch));
   if ( ch->hw_cfg.gpio_hdmi_det >= 0 )
   {
      int gpio_val;
      unsigned long flags;
      hdmi_state new_state;

      spin_lock_irqsave( &detlock, flags );
      gpio_val = gpio_get_value( ch->hw_cfg.gpio_hdmi_det );

      new_state = (! gpio_val ) ? HDMI_CONNECTED : HDMI_UNPLUGGED;

      if (ch->state != new_state)
      {
         ch->changed_at = jiffies;
         ch->changed = 1;
         ch->state = new_state;
      }
      else
      {
         ch->changed = 0;
      }
      wake_up_interruptible( &ch->waitq );
      spin_unlock_irqrestore( &detlock, flags );
      LOG_DEBUG(printk("check_hdmi_det_gpio %p state %d gpio val %d\n",
               ch, ch->state, gpio_val));
   }
   else
   {
      ch->state = HDMI_STATE_INIT;
   }
}

/***************************************************************************/
/**
*  Driver open method
*
*  @remarks
*/
static int hdmi_open( struct inode *inode, struct file *file )
{
   struct hdmi_info *ch = gHDMI;

   LOG_TRACE(printk("hdmi open ch%p\n", ch));

   if ( ch->hw_cfg.gpio_hdmi_det < 0 )
   {
      /* Missing configuration */
      return -EPERM;
   }
   file->private_data = ch;

   return 0;
}

/***************************************************************************/
/**
*  Driver ioctl method
*
*  @remarks
*/
static long hdmi_ioctl( struct file *file, unsigned int cmd, unsigned long arg )
{
   struct hdmi_info *ch = file->private_data;

   switch ( cmd )
   {
      case HDMI_IOCTL_GET_STATE:
         if ( copy_to_user( (unsigned long *)arg, &ch->state, sizeof(ch->state) ) != 0 )
         {
            return -EFAULT;
         }
         break;

#ifdef CONFIG_BCM_HDMI_DET_SWITCH
      case HDMI_IOCTL_SET_SWITCH:
         {
            hdmi_state switch_state;

            if ( copy_from_user( &switch_state, (void *)arg, sizeof(switch_state) ) != 0 )
            {
               return -EFAULT;
            }

            switch_set_state( &hdmi_hpd_switch,
                              switch_state );
         }
         break;
#endif

      default:
         return -ENOTTY;
   }
   return 0;
}

/***************************************************************************/
/**
*  Driver poll method to support system select call
*
*  @remarks
*/
static unsigned int hdmi_poll(
   struct file *file,
   struct poll_table_struct *poll_table
)
{
   struct hdmi_info *ch = file->private_data;

   LOG_TRACE(printk("hdmi_poll:\n"));
   poll_wait( file, &ch->waitq, poll_table );

   if ( ch->changed )
   {
      LOG_TRACE(printk("hdmi_poll: changed\n"));
      ch->changed = 0;
      return POLLIN | POLLRDNORM;
   }

   return 0;
}

/** Driver read method. Contains 1 if connected, otherwise 0 */
static ssize_t hdmi_read( struct file *file, char __user *buf,
      size_t count, loff_t *f_pos)
{
   struct hdmi_info *ch = file->private_data;
   char status_buf[2] = {0};
   int status_len = 0;
   ssize_t retval = 0;


   sprintf(status_buf, "%d\n", (ch->state == HDMI_CONNECTED ? 1 : 0));
   status_len = strlen(status_buf);

   LOG_TRACE(printk("hdmi_read count %d f_pos %d status_len %d\n",
            count, (int) *f_pos, status_len));

   if (*f_pos >= status_len)
      goto exit;

   if (*f_pos + count > status_len)
      count = status_len - *f_pos;

   if (count > 0)
   {
      if (copy_to_user(buf, status_buf, count))
      {
         retval = -EFAULT;
         goto exit;
      }
   }
   *f_pos += count;
   retval = count;

exit:
   LOG_TRACE(printk("hdmi_read: returning %d fpos %d\n", retval, (int) *f_pos));
   return retval;
}

/** Called 100ms after HPD interrupt to debounce signal. This invokes
 * the function that checks the GPIO value. */
static void hdmi_det_timer_fn(unsigned long data)
{
   void* dev_id = (void *) data;
   check_hdmi_det_gpio(dev_id);
}

/***************************************************************************/
/**
*  hdmi detection ISR
*/
static irqreturn_t hdmi_det_irq( int irq, void *dev_id )
{
   struct hdmi_info* ch = dev_id;
   LOG_TRACE(printk("hdmi_det_irq: %d %p\n", irq, dev_id));

   /* Cancel exiting timer */
   del_timer(&ch->hpd_timer);

   ch->hpd_timer.data = (unsigned long) ch;
   ch->hpd_timer.function = hdmi_det_timer_fn;
   ch->hpd_timer.expires = jiffies + (HZ / 2);

   LOG_DEBUG(printk("hdmi_det_irq irq: %d HZ now %lu expires %lu\n", HZ, jiffies,
            ch->hpd_timer.expires));
   add_timer(&ch->hpd_timer);
   ch->irq_at = jiffies;
   return IRQ_HANDLED;
}

/**
 * Displays the connection status and the time in jiffies when it was updated.
 */
static int hdmi_proc_read( char *buf, char **start, off_t offset, int count,
      int *eof, void *data )
{
   struct hdmi_info* ch = gHDMI;
   sprintf(buf, "%s: %lu\n",
         ( ch->state ? HDMI_CONNECTED_STR : HDMI_UNPLUGGED_STR ),
         ch->changed_at);
   *eof = 1;
   return strlen(buf);
}

/**
 * hdmi_proc_write
 * Allows the HDMI connection status to be set from user-space. This is for
 * testing.
 */
static int hdmi_proc_write( struct file *file, const char __user *buffer,
      unsigned long count, void *data )
{
   int ret = -EFAULT;
   unsigned char kbuf[PROC_WRITE_BUF_SIZE + 1];
   int state_changed = 0;
   hdmi_state new_state = HDMI_CONNECTED;
   struct hdmi_info* ch = gHDMI;

   if ( count >= PROC_WRITE_BUF_SIZE )
   {
      count = PROC_WRITE_BUF_SIZE;
   }

   if ( copy_from_user( kbuf, buffer, count ) != 0 )
   {
      printk(KERN_ERR "%s: failed to copy %ld bytes from user", __func__, count);
      ret = -EFAULT;
      goto out;
   }
   kbuf[count] = '\0';

   if ( strcmp( kbuf, HDMI_CONNECTED_STR ) == 0 )
   {
      state_changed = 1;
      new_state = HDMI_CONNECTED;
   }
   else if ( strcmp( kbuf, HDMI_UNPLUGGED_STR ) == 0 )
   {
      state_changed = 1;
      new_state = HDMI_UNPLUGGED;
   }
   else
   {
      printk(KERN_ERR "[%s]: parameter '%s' is **unknown** to driver '%s'." \
            "Valid parameters: %s,%s",
            __func__, kbuf, DRIVER_NAME,
            HDMI_CONNECTED_STR, HDMI_UNPLUGGED_STR);
      goto out;
   }

   if ( state_changed )
   {
      ch->changed_at = jiffies;
      ch->changed = 1;
      if ( ch->state != new_state )
      {
         /* If the fake hotplug has changed the state then we want
          * to wake up any processes waiting on this.
          */
         ch->state = new_state;
         wake_up_interruptible( &ch->waitq );
      }
   }
   ret = count;
out:
   return ret;
}

/***************************************************************************/
/**
*  Platform probe method
*/
static int __devinit hdmi_pltfm_probe(struct platform_device *pdev)
{
   struct hdmi_info *ch;
   int ret;

   BUG_ON( pdev == NULL );

   if ( pdev->dev.platform_data == NULL )
   {
      dev_err( &pdev->dev, "platform_data missing\n" );
      return -EFAULT;
   }

   ch = gHDMI;
   memcpy( &ch->hw_cfg, (void *)pdev->dev.platform_data, sizeof(ch->hw_cfg) );

   LOG_DEBUG(printk("hdmi_platfm_probe %d", ch->hw_cfg.gpio_hdmi_det));
   if ( ch->hw_cfg.gpio_hdmi_det >= 0 )
   {
      ret = gpio_request( ch->hw_cfg.gpio_hdmi_det, "hdmi det");
      if ( ret < 0 )
      {
         dev_err( &pdev->dev, "Unable to request GPIO pin %d, reason %d\n",
                  ch->hw_cfg.gpio_hdmi_det, ret );
         return ret;
      }

      LOG_DEBUG(printk("hdmi_platfm_probe request_irq %d",
               gpio_to_irq( ch->hw_cfg.gpio_hdmi_det)));
      ret = request_irq( gpio_to_irq( ch->hw_cfg.gpio_hdmi_det ),
            hdmi_det_irq, IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,
            "hdmi_det", ch );
      if ( ret )
      {
         dev_err( &pdev->dev, "Unable to request hdmi detect irq=%d for gpio=%d\n",
               gpio_to_irq( ch->hw_cfg.gpio_hdmi_det ), ch->hw_cfg.gpio_hdmi_det );
         goto err_free_cd_gpio;
      }
   }

   return 0;

err_free_cd_gpio:
   gpio_free( ch->hw_cfg.gpio_hdmi_det );
   return ret;
}

/***************************************************************************/
/**
*  Platform remove method
*/
static int __devexit hdmi_pltfm_remove( struct platform_device *pdev )
{
   struct hdmi_info *ch;
   ch = gHDMI;

   if ( ch->hw_cfg.gpio_hdmi_det >= 0 )
   {
      gpio_free( ch->hw_cfg.gpio_hdmi_det );
   }
   platform_set_drvdata( pdev, NULL );

   return 0;
}

static struct platform_driver gPlatform_driver =
{
   .driver =
   {
      .name    = DRIVER_NAME,
      .owner   = THIS_MODULE,
   },
   .probe      = hdmi_pltfm_probe,
   .remove     = __devexit_p( hdmi_pltfm_remove ),
};

/***************************************************************************/
/**
*  Initialize APM hdmi detection
*
*  @remarks
*/
static int __init hdmi_init( void )
{
   int rc;
   struct hdmi_info *ch = gHDMI;
   struct device *dev;

   init_waitqueue_head( &ch->waitq );
   init_timer(&ch->hpd_timer);

#ifdef CONFIG_SYSFS
   if (( rc = alloc_chrdev_region( &hdmi_devnum, 0, 1, gPlatform_driver.driver.name )) < 0 )
   {
      printk(KERN_ERR "[%s]: alloc_chrdev_region failed (rc=%d)", __func__, rc );
      goto err;
   }

   cdev_init( &hdmi_cdev, &hdmi_fops );
   if (( rc = cdev_add( &hdmi_cdev, hdmi_devnum, 1 )) != 0 )
   {
      printk(KERN_ERR "[%s]: cdev_add failed (rc=%d)", __func__, rc );
      goto err_unregister;
   }

   hdmi_class = class_create( THIS_MODULE, gPlatform_driver.driver.name );
   if ( IS_ERR( hdmi_class ))
   {
      rc = PTR_ERR( hdmi_class );
      printk(KERN_ERR "[%s]: class_create failed (rc=%d)", __func__, rc );
      goto err_cdev_del;
   }

   dev = device_create( hdmi_class, NULL, hdmi_devnum, NULL,
                        "hdmi0" );
   if ( IS_ERR( dev ))
   {
      rc = PTR_ERR( dev );
      printk(KERN_ERR "[%s]: device_create failed (rc=%d)", __func__, rc );
      goto err_class_destroy;
   }

   hdmi_proc_entry = create_proc_entry( PROC_HDMI_HPD, 0660, NULL);
   if ( ! hdmi_proc_entry )
   {
      rc = -EFAULT;
      printk(KERN_ERR "[%s]: create_proc_entry failed", __func__);
      goto err_device_destroy;
   }
   hdmi_proc_entry->read_proc = hdmi_proc_read;
   hdmi_proc_entry->write_proc = hdmi_proc_write;
#endif

   rc = platform_driver_register( &gPlatform_driver );
   if ( rc < 0 )
   {
      printk(KERN_ERR "[%s]: failed to register platform driver\n", __func__ );
      goto err_device_destroy;
   }

#ifdef CONFIG_BCM_HDMI_DET_SWITCH
   /* Create the hdmi switch */
   hdmi_hpd_switch.name = "hdmi";

   rc = switch_dev_register( &hdmi_hpd_switch );
   if (rc < 0) {
      printk(KERN_ERR "HDMI: Device switch create failed\n");
      rc = -EFAULT;
      goto err_device_destroy;
   }

   /* Assign initial state. */
   switch_set_state( &hdmi_hpd_switch,
                     0 );
#endif

   if ( ch->hw_cfg.gpio_hdmi_det < 0 )
   {
      /* Missing configuration */
      rc = -EPERM;
      printk(KERN_ERR "HDMI: Detection GPIO not available\n");
      goto err_no_gpio;

   }
   spin_lock_init ( &detlock );
   check_hdmi_det_gpio( ch );

   return 0;

err_no_gpio:
   platform_driver_unregister( &gPlatform_driver );
   remove_proc_entry(PROC_HDMI_HPD, NULL);
#ifdef CONFIG_BCM_HDMI_DET_SWITCH
   switch_dev_unregister(&hdmi_hpd_switch);
#endif
#ifdef CONFIG_SYSFS
err_device_destroy:
   device_destroy( hdmi_class, hdmi_devnum );
err_class_destroy:
   class_destroy( hdmi_class );
   hdmi_class = NULL;
err_cdev_del:
   cdev_del( &hdmi_cdev );
err_unregister:
   unregister_chrdev_region( hdmi_devnum, 1 );
#endif
err:

   return rc;
}

/***************************************************************************/
/**
*  Destructor for the hdmi detection driver
*
*  @remarks
*/
static void __exit hdmi_exit( void )
{
   platform_driver_unregister( &gPlatform_driver );
   remove_proc_entry(PROC_HDMI_HPD, NULL);
#ifdef CONFIG_BCM_HDMI_DET_SWITCH
   switch_dev_unregister(&hdmi_hpd_switch);
#endif
#ifdef CONFIG_SYSFS
   device_destroy( hdmi_class, hdmi_devnum );
   class_destroy( hdmi_class );
   cdev_del( &hdmi_cdev );
   unregister_chrdev_region( hdmi_devnum, 1 );
#endif
}

module_init( hdmi_init );
module_exit( hdmi_exit );
MODULE_AUTHOR( "Broadcom" );
MODULE_DESCRIPTION( "HDMI Hotplug Detection driver." );
MODULE_LICENSE( "GPL" );
