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
*  @file    mdmgpio.c
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
#include <linux/io.h>
#include <asm/system.h>


#define MDMGPIO_NUM 3
#define MDMGPIO_GPIO_OFFSET 112
#define MDMGPIO_TO_GPIO(x) MDMGPIO_GPIO_OFFSET+x
#define MDMGPIO_READ_ASYNC 100
#define MDMGPIO_GET_STATUS 101

#define PAD_CTRL 0x35004800
 
/* ---- Public Variables ------------------------------------------------- */
/* ---- Private Constants and Types -------------------------------------- */



/* ---- Private Variables ------------------------------------------------ */

static dev_t mdmgpio_devnum = 0;
static struct class *mdmgpio_class = NULL;
static struct cdev mdmgpio_cdev;
static int gReadAsync = 1;

struct mdmgpio_info
{
   int               changed;       /* Changed flag used by poll */
   int               status;
   int               id;
   int               flag;
   wait_queue_head_t waitq;         /* wait queue */
};

static struct mdmgpio_info gMdmgpio;

static spinlock_t mdmgpiolock = __SPIN_LOCK_UNLOCKED();

/* ---- Private Function Prototypes -------------------------------------- */
static int  mdmgpio_open( struct inode *inode, struct file *file );
static long  mdmgpio_ioctl( struct file *file, unsigned int cmd, unsigned long arg );
static ssize_t mdmgpio_read( struct file *file, char __user *buf, size_t count,loff_t *f_pos);
static ssize_t mdmgpio_write(struct file *, const char *, size_t, loff_t *);
static unsigned int mdmgpio_poll( struct file *file, struct poll_table_struct *poll_table );

/* File Operations (these are the device driver entry points) */
static struct file_operations mdmgpio_fops =
{
   .owner          = THIS_MODULE,
   .open           = mdmgpio_open,
   .unlocked_ioctl = mdmgpio_ioctl,
   .poll           = mdmgpio_poll, 
   .read           = mdmgpio_read,
   .write          = mdmgpio_write
};

/* ---- Functions -------------------------------------------------------- */

static irqreturn_t mdmgpio_irq( int irq, void *dev_id)
{
   struct mdmgpio_info *pMdmgpio = (struct mdmgpio_info *)dev_id;
   unsigned long flags;
   int status;

   //printk(KERN_ERR "irq=%d, id =%d\n", irq, pMdmgpio->id);

   spin_lock_irqsave( &mdmgpiolock, flags );
   status = gpio_get_value( MDMGPIO_TO_GPIO(pMdmgpio->id) );
   //printk(KERN_ERR "MDMGPIO :%d status: %d\n", pMdmgpio->id, status);
   if(status != pMdmgpio->status)
   {
       pMdmgpio->changed = 1;
       pMdmgpio->status = status;
       pMdmgpio->flag = 1;
       printk(KERN_ERR "wake up queue: 3\n");
       wake_up_interruptible( &(pMdmgpio->waitq) );

   }
   spin_unlock_irqrestore( &mdmgpiolock, flags );

   return IRQ_HANDLED;
}



static int mdmgpio_init_irq(int mdmgpio)
{
    int ret;
    int gpio;
    char name[64];

    //printk(KERN_ERR "mdmgpio_init_irq: mdmgpio =%d\n", mdmgpio);
    gpio = MDMGPIO_TO_GPIO(mdmgpio);
    printk(KERN_INFO "GPIO %d mapped to MDMGPIO %d\n", gpio, mdmgpio);
    sprintf(name, "MDMGPIO%d", mdmgpio);
    gpio_request(gpio, name);
    gpio_direction_input(gpio);
    if(gMdmgpio.id == mdmgpio)
    {
        printk(KERN_ERR"MDMGPIO%d irq already initialized\n", mdmgpio);
        return -1;
    }

    gMdmgpio.id = mdmgpio;
    
    sprintf(name, "mdmgpio%d_irq", mdmgpio);

    //printk(KERN_ERR "irq request for MDMGPIO:%d, irq:%d\n", mdmgpio, gpio_to_irq(gpio));
    init_waitqueue_head( &gMdmgpio.waitq );
    ret = request_irq( gpio_to_irq(gpio),
                       mdmgpio_irq, IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,
                       name, (void*)&gMdmgpio);

    if(ret)
    {
        printk(KERN_ERR "Unable to request irq=%d for gpio=%d\n",
                              gpio_to_irq(gpio), gpio );
        return -ENOTTY;
         
    }

    //enable_irq(gpio_to_irq(gpio));

    return 0;


}

/***************************************************************************/
/**
*  Driver open method
*
*  @remarks
*/
static int mdmgpio_open( struct inode *inode, struct file *file )
{
   return 0;
}


/***************************************************************************/
/**
*  Driver ioctl method
*
*  @remarks
*/
static long mdmgpio_ioctl( struct file *file, unsigned int cmd, unsigned long arg )
{
    int rc = (int)arg;
   
   switch ( cmd )
   {

      case MDMGPIO_READ_ASYNC:
         if(rc == 1)
             gReadAsync = 1;
         else if(rc == 0)
             gReadAsync = 0;
         break;

      case MDMGPIO_GET_STATUS:

          rc = gpio_get_value( MDMGPIO_TO_GPIO(MDMGPIO_NUM) );
          if ( copy_to_user( (unsigned long *)arg, &rc, sizeof(rc) ) != 0 )
          {
              return -EFAULT;
          }
          
          break;

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
static unsigned int mdmgpio_poll(
   struct file *file,
   struct poll_table_struct *poll_table
)
{

   poll_wait( file, &gMdmgpio.waitq, poll_table );

   if ( gMdmgpio.changed )
   {
      gMdmgpio.changed = 0;
      return POLLIN | POLLRDNORM;
   }

   
   return 0;
}

/***************************************************************************/
/**
*  Driver open method
*
*  @remarks
*/
static ssize_t mdmgpio_read( struct file *file, char __user *buf, size_t count,
      loff_t *f_pos)
{
   
   char status_buf[8] = {0};
   int status_len = 0;
   ssize_t retval = 0;
   int status;

   if(gReadAsync == 0)
   {
       printk(KERN_ERR "mdmgpio_read: sync\n");
       status = gpio_get_value( MDMGPIO_TO_GPIO(MDMGPIO_NUM) );
       if(buf != NULL && count >=4)
       {
           status_len = sprintf(status_buf, "%d\n", status);
           if ( copy_to_user( buf, status_buf, status_len ) != 0 )
           {
               return 0;
           }
       }

   }         
   else
   {
       printk(KERN_ERR "mdmgpio_read: async\n");
       wait_event_interruptible(gMdmgpio.waitq, gMdmgpio.flag !=0);
       gMdmgpio.flag = 0;
       //printk(KERN_ERR "mdmgpio_read: after wait\n");
       if(buf != NULL && count >=4)
       {
           status_len = sprintf(status_buf, "%d\n", gMdmgpio.status);
           if ( copy_to_user( buf, status_buf, status_len ) != 0 )
           {
               return 0;
           }
       }
   }

   printk(KERN_INFO "mdmgpio_read: read return. mdmgpio:%d, value: %d\n", MDMGPIO_NUM, gMdmgpio.status);
   retval = status_len;


   return retval;
}


/***************************************************************************/
/**
*  Driver write method
*
*  @remarks
*/
static ssize_t mdmgpio_write(struct file *file, const char *buf, size_t count, loff_t *f_pos)
{
    //int mdmgpio;

    if(buf != NULL)
    {
        printk(KERN_ERR "mdmgpio_write: %s\n", buf);

    }

    return count;

}


/***************************************************************************/
/**
*  Platform probe method
*/
static int __devinit mdmgpio_pltfm_probe(struct platform_device *pdev)
{
   
   //int ret;

   BUG_ON( pdev == NULL );

   
   return 0;


}

/***************************************************************************/
/**
*  Platform remove method
*/
static int __devexit mdmgpio_pltfm_remove( struct platform_device *pdev )
{
   
   platform_set_drvdata( pdev, NULL );

   return 0;
}

static struct platform_driver gPlatform_driver =
{
   .driver =
   {  
      .name    = "mdmgpio3",
      .owner   = THIS_MODULE,
   },
   .probe      = mdmgpio_pltfm_probe,
   .remove     = __devexit_p( mdmgpio_pltfm_remove ),
};

/***************************************************************************/
/**
*  Initialize mdmgpio 
*
*  @remarks
*/

static int __init mdmgpio_init( void )
{
   int rc;
   
   struct device *dev;
   void __iomem *vaddr;
   int i;

   printk(KERN_INFO "mdmgpio%d_init\n", MDMGPIO_NUM);


   /*hack to config pin mux. CP is hardcoding the pin Mux*/
   vaddr = ioremap(PAD_CTRL, 1024);
   /*write password*/
   writel(0xa5a501, vaddr + 0x7F0);
   /*unlock config register */
   writel(0, vaddr + 0x784);
   writel(0, vaddr + 0x788);
   for(i=0;i<9;i++)
   {
       writel(0x443, vaddr + 0xE8 + i*4);
   }


   /*set driver name */
   //rc = sprintf(gPlatform_driver.driver.name, "mdmgpio%d\n", MDMGPIO_NUM);
   
   if (( rc = alloc_chrdev_region( &mdmgpio_devnum, 0, 1, gPlatform_driver.driver.name )) < 0 )
   {
      printk(KERN_ERR "[%s]: alloc_chrdev_region failed (rc=%d)", __func__, rc );
      goto err;
   }

   cdev_init( &mdmgpio_cdev, &mdmgpio_fops );
   if (( rc = cdev_add( &mdmgpio_cdev, mdmgpio_devnum, 1 )) != 0 )
   {
      printk(KERN_ERR "[%s]: cdev_add failed (rc=%d)", __func__, rc );
      goto err_unregister;
   }

   mdmgpio_class = class_create( THIS_MODULE, gPlatform_driver.driver.name );
   if ( IS_ERR( mdmgpio_class ))
   {
      rc = PTR_ERR( mdmgpio_class );
      printk(KERN_ERR "[%s]: class_create failed (rc=%d)", __func__, rc );
      goto err_cdev_del;
   }

   dev = device_create( mdmgpio_class, NULL, mdmgpio_devnum, NULL,
                        gPlatform_driver.driver.name );
   if ( IS_ERR( dev ))
   {
      rc = PTR_ERR( dev );
      printk(KERN_ERR "[%s]: device_create failed (rc=%d)", __func__, rc );
      goto err_class_destroy;
   }

   //create_proc_read_entry( PROC_MDMGPIO, 0, NULL, mdmgpio_read_procmem, NULL);

   rc = platform_driver_register( &gPlatform_driver );
   if ( rc < 0 )
   {
      printk(KERN_ERR "%s: failed to register platform driver\n", __FUNCTION__ );
      goto err_device_destroy;
   }


   /*init irq handler */
   rc = mdmgpio_init_irq(MDMGPIO_NUM);
   if(rc != 0)
   {
       goto err_device_destroy;
   }

   spin_lock_init ( &mdmgpiolock );


   return 0;



err_device_destroy:
   device_destroy( mdmgpio_class, mdmgpio_devnum );
err_class_destroy:
   class_destroy( mdmgpio_class );
   mdmgpio_class = NULL;
err_cdev_del:
   cdev_del( &mdmgpio_cdev );
err_unregister:
   unregister_chrdev_region( mdmgpio_devnum, 1 );

err:

   return rc;
}

/***************************************************************************/
/**
*  Destructor for the mdmgpio detection driver
*
*  @remarks
*/
static void __exit mdmgpio_exit( void )
{
   platform_driver_unregister( &gPlatform_driver );
   //remove_proc_entry(PROC_MDMGPIO, NULL);

#ifdef CONFIG_SYSFS
   device_destroy( mdmgpio_class, mdmgpio_devnum );
   class_destroy( mdmgpio_class );
   cdev_del( &mdmgpio_cdev );
   unregister_chrdev_region( mdmgpio_devnum, 1 );
#endif
}

late_initcall( mdmgpio_init );
module_exit( mdmgpio_exit );
MODULE_AUTHOR( "Broadcom" );
MODULE_DESCRIPTION( "MDMGPIO driver." );
MODULE_LICENSE( "GPL" );
