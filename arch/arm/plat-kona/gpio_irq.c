/*****************************************************************************
* Copyright 2004 - 2008 Broadcom Corporation.  All rights reserved.
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




/*
*
*****************************************************************************
*
*  gpio_irq.c
*
*  PURPOSE:
*
*     This implements the gpio driver interrupt rising/falling edge driver.
*
*  NOTES:
*
*****************************************************************************/

/* ---- Include Files ---------------------------------------------------- */

#include <linux/version.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/kernel.h>

#include <linux/init.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/sysctl.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>

#include <asm/mach/irq.h>
#include <mach/kona_gpio_inline.h>

/* ---- Public Variables ------------------------------------------------- */

/* ---- Private Constants and Types -------------------------------------- */

/* Debug logging */
#ifdef DEBUG
#undef DEBUG
#endif
#define DEBUG 1

#define DBG_ERROR	0x01
#define DBG_INFO	0x02
#define DBG_TRACE	0x04
#define DBG_TRACE2	0x08
#define DBG_DATA	0x10
#define DBG_DATA2	0x20

#define DBG_DEFAULT_LEVEL	(DBG_ERROR | DBG_INFO)

#if DEBUG
#	define GPIO_DEBUG(level,x) {if (level & gLevel) printk x;}
#else
#	define GPIO_DEBUG(level,x)
#endif


static char banner[] __initdata = KERN_INFO "GPIO Control Driver: 1.00 (built on "__DATE__" "__TIME__")\n";
static int gLevel = DBG_DEFAULT_LEVEL;

/* ---- Private Variables ------------------------------------------------ */

/* ---- Functions -------------------------------------------------------- */

/****************************************************************************
*
*  gpio_irq_ack
*
*     Called by the interrupt handler to acknowledge (i.e. clear)
*     the interrupt.
*
***************************************************************************/

static void gpio_irq_ack( unsigned irq )
{
   /*
    * Since this function is ONLY called with interrupts disabled, we don't 
    * need to disable irqs around the following 
    */
	kona_gpio_clear_int(irq_to_gpio(irq));
}

/****************************************************************************
*
*  gpio_irq_mask
*
*     Called to mask (i.e. disable) an interrupt.
*
***************************************************************************/

static void gpio_irq_mask( unsigned irq )
{
   /*
    * Since this function is ONLY called with interrupts disabled, we don't 
    * need to disable irqs around the following 
    */
	kona_gpio_set_irqmask(irq_to_gpio(irq), 1);
}

/****************************************************************************
*
*  gpio_irq_unmask
*
*     Called to unmask (i.e. enable) an interrupt.
*
***************************************************************************/

static void gpio_irq_unmask( unsigned irq )
{
   /*
    * Since this function is ONLY called with interrupts disabled, we don't 
    * need to disable irqs around the following 
    */

	kona_gpio_set_irqmask(irq_to_gpio(irq), 0);
}

/****************************************************************************
*
*  gpio_set_irq_type
*
*     Sets the trigger type of the GPIO irq.
*
***************************************************************************/
static void gpio_set_irq_trigger_type(unsigned gpio, unsigned type)
{
	uint32_t mode;
	
	mode = kona_gpio_get_mode(gpio);
	 
	mode &= GPIO_GPCTR0_IOTR_MASK;
	switch (type)
	{
		case IRQ_TYPE_EDGE_RISING:
			mode |= (KONA_GPIO_INT_RISING << 1);
			break;
		case IRQ_TYPE_EDGE_FALLING:
			mode |= (KONA_GPIO_INT_FALLING << 1);
			break;
		case IRQ_TYPE_EDGE_BOTH:
			mode |= (KONA_GPIO_INT_BOTH << 1);
			break;
		default:
			printk(KERN_ERR "%s Bad interrupt trigger type %d\n", __func__, type);
			return;
	}

	kona_gpio_set_mode(gpio, mode);
}
/****************************************************************************
*
*  gpio_irq_type
*
*     Sets the type of the GPIO irq.
*
***************************************************************************/

static int gpio_irq_set_type( unsigned irq, unsigned type )
{
   int   gpio;
	uint32_t mode;

   gpio = irq_to_gpio( irq );

   /*
    * Since this function is ONLY called with interrupts disabled, we don't 
    * need to disable irqs around the following 
    */

   if ( type == IRQ_TYPE_PROBE )
   {
      /* Don't mess GPIOs which already have interrupt handlers registered. */

      if ( kona_gpio_get_irqmask( gpio ) == 0 )
      {
         return 0;
      }

      type = IRQ_TYPE_EDGE_BOTH;
   }

   printk( KERN_INFO "IRQ%d (gpio%d): ", irq, gpio );

	/* Disable Ints */
	kona_gpio_set_irqmask(gpio, 1);

	/* Set direction as input */
	mode = kona_gpio_get_mode(gpio);
   mode &= ~GPIO_GPCTR0_IOTR_MASK;
	mode |= KONA_GPIO_DIR_INPUT;
   kona_gpio_set_mode(gpio, mode);

	/* Clear ints */
	kona_gpio_clear_int(gpio);

	/* Enable ints */
	kona_gpio_set_irqmask(gpio, 0);

   if ( type & IRQ_TYPE_EDGE_RISING )
   {
      if ( type & IRQ_TYPE_EDGE_FALLING )
      {
         printk( "both edges\n" );
			gpio_set_irq_trigger_type(gpio, IRQ_TYPE_EDGE_BOTH); 
      }
      else
      {
         printk( "rising edges\n" );
			gpio_set_irq_trigger_type(gpio, IRQ_TYPE_EDGE_RISING); 
      }
   }
   else if ( type & IRQ_TYPE_EDGE_FALLING )
   {
      printk( "falling edges\n" );
      gpio_set_irq_trigger_type(gpio, IRQ_TYPE_EDGE_FALLING );
   }
   else if ( type & IRQ_TYPE_LEVEL_HIGH )
   {
		printk(KERN_ERR "%s: high level NOT SUPPORTED\n", __func__ );
		return -EINVAL; /* Not supported on this platform */
   }
   else if ( type & IRQ_TYPE_LEVEL_LOW )
   {
		printk(KERN_ERR "%s: low level NOT SUPPORTED\n", __func__ );
		return -EINVAL; /* Not supported on this platform */
   }
   else
   {
      printk( "no edges\n" );
      printk( KERN_ERR "%s: Failed to set type for IRQ%d (gpio%d): ", __func__, irq, gpio );
		kona_gpio_set_irqmask(gpio, 1); /* Disable interrupt */
      return -EINVAL;
   }

   return 0;

} /* gpio_irq_set_type */

/****************************************************************************
*
*  gpio_isr_handler
*
*     Figures out which GPIO caused the interrupt and calls the register
*     handler to deal with it.
*
***************************************************************************/

static void gpio_isr_handler_common(unsigned int gpio, struct irq_desc *desc, unsigned int irq )
{
   int loop;
   struct irq_desc *gpio_desc;

   /* temporarily ack/mask (level sensitive) parent IRQ */
   desc->chip->ack(irq);

   do
   {
		unsigned int mask = kona_gpio_get_irqstatus_register(gpio);
      loop = 0;
      if ( mask )
      {
			unsigned int irq;
			
         /* Clear the interrupts */	
			kona_gpio_clear_int_register(gpio);

         irq = gpio_to_irq( gpio );
         gpio_desc = irq_desc + irq;
         do
         {
            if ( mask & 1 )
            {
               gpio_desc->handle_irq( irq, gpio_desc );
            }
            irq++;
            gpio_desc++;
            mask >>= 1;

         } while ( mask );

         loop = 1;
      }

   } while ( loop );

   desc->chip->unmask(irq);
} /* gpio_isr_handler */

static void gpio_isr_handler0( unsigned int irq, struct irq_desc *desc )
{
   gpio_isr_handler_common(0, desc, irq);
}
static void gpio_isr_handler1( unsigned int irq, struct irq_desc *desc )
{
	gpio_isr_handler_common(32, desc, irq);
}
static void gpio_isr_handler2( unsigned int irq, struct irq_desc *desc )
{
	gpio_isr_handler_common(64, desc, irq);
}
static void gpio_isr_handler3( unsigned int irq, struct irq_desc *desc )
{
	gpio_isr_handler_common(96, desc, irq);
}
static void gpio_isr_handler4( unsigned int irq, struct irq_desc *desc )
{
	gpio_isr_handler_common(128, desc, irq);
}
static void gpio_isr_handler5( unsigned int irq, struct irq_desc *desc )
{
	gpio_isr_handler_common(160, desc, irq);
}

/****************************************************************************
*
*  gpio_chip data structure.
*
***************************************************************************/

static struct irq_chip gpio_chip =
{
   .typename = "GPIO-IRQ",
   .ack     = gpio_irq_ack,
   .mask    = gpio_irq_mask,
   .unmask  = gpio_irq_unmask,
   .set_type    = gpio_irq_set_type,
   .disable = gpio_irq_mask,
};

/****************************************************************************
*
*  gpio_cleanup
*
*       Called to perform module cleanup when the module is unloaded.
*
***************************************************************************/
static void gpio_cleanup( void )
{
	GPIO_DEBUG(DBG_TRACE,("gpio_cleanup()\n"));

}

/****************************************************************************
*
*  gpio_init
*
*     Called to perform module initialization when the module is loaded
*
***************************************************************************/

static int __init gpio_init( void )
{
	/* Initialize debug level */
	gLevel = DBG_DEFAULT_LEVEL;

	GPIO_DEBUG(DBG_INFO,( "gpio_init called\n" ));

	printk( banner );

   kona_gpio_init();
   {
      int   irq;

      for ( irq = gpio_to_irq( 0 ); irq <= gpio_to_irq( NUM_GPIO_IRQS-1 ); irq++ )
      {
         set_irq_chip( irq, &gpio_chip );
         set_irq_handler( irq, handle_simple_irq );
         set_irq_flags( irq, IRQF_VALID | IRQF_PROBE );
      }
      set_irq_chained_handler( BCM_INT_ID_GPIO1, gpio_isr_handler0 );
      set_irq_chained_handler( BCM_INT_ID_GPIO2, gpio_isr_handler1 );
      set_irq_chained_handler( BCM_INT_ID_GPIO3, gpio_isr_handler2 );
      set_irq_chained_handler( BCM_INT_ID_GPIO4, gpio_isr_handler3 );
      set_irq_chained_handler( BCM_INT_ID_GPIO5, gpio_isr_handler4 );
      set_irq_chained_handler( BCM_INT_ID_GPIO6, gpio_isr_handler5 );
   }

   return 0;
} /* gpio_init */

/****************************************************************************
*
*  gpio_exit
*
*       Called to perform module cleanup when the module is unloaded.
*
***************************************************************************/

static void __exit gpio_exit( void )
{
	GPIO_DEBUG(DBG_INFO,( "gpio_exit called\n" ));

	gpio_cleanup();
} /* gpio_exit */

/* Changed from module_init to fs_initcall so that GPIO driver
 * is loaded before the any of the PMU drivers were loaded on 
 * other products - leave it this way for future.
 */

arch_initcall(gpio_init);
module_exit(gpio_exit);

MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("GPIO Control Driver");
MODULE_LICENSE("GPL v2");



