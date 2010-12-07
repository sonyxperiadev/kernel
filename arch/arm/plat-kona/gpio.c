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


/****************************************************************************
*
*  gpio.c
*
*  PURPOSE:
*
*       This file implements the GPIO chips required to support the onchip
*       gpio pins, as per gpiolib
*
*****************************************************************************/

/* ---- Include Files ---------------------------------------------------- */

#include <linux/version.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/seq_file.h>
#include <linux/irq.h>
#include <linux/init.h>

#include <asm/gpio.h>

#include <mach/kona_gpio_inline.h>

/* ---- Public Variables ------------------------------------------------- */

/* ---- Private Constants and Types -------------------------------------- */

typedef struct
{
    struct gpio_chip chip;
    volatile unsigned int *reg;

} kona_gpio_chip;

/* ---- Private Function Prototypes -------------------------------------- */

/* ---- Private Variables ------------------------------------------------ */

static spinlock_t gGpioLock = SPIN_LOCK_UNLOCKED;

/* ---- Functions -------------------------------------------------------- */

/****************************************************************************
*
*  Configure a GPIO pin as an input pin
*
*****************************************************************************/

static int gpio_kona_direction_input( struct gpio_chip *chip, unsigned gpio )
{
	 uint32_t mode;
	 unsigned long flags;

	 (void)chip;	/* Not required for this particular lower level API */
	
	 spin_lock_irqsave(&gGpioLock, flags);

	 mode = kona_gpio_get_mode(gpio);
	 
	 mode &= ~GPIO_GPCTR0_IOTR_MASK;
	 mode |= KONA_GPIO_DIR_INPUT;

	 kona_gpio_set_mode(gpio, mode);
	 
	 spin_unlock_irqrestore(&gGpioLock, flags);
    return 0;

} /* gpio_kona_direction_input */

/****************************************************************************
*
*  Configure a GPIO pin as an output pin and sets its initial value.
*
*****************************************************************************/

static int gpio_kona_direction_output( struct gpio_chip *chip, unsigned gpio, int value )
{
	 uint32_t mode;
	 unsigned long flags;

	(void)chip;	/* Not required for this particular lower level API */

	 spin_lock_irqsave(&gGpioLock, flags);

	 mode = kona_gpio_get_mode(gpio);
	 
	 mode &= ~GPIO_GPCTR0_IOTR_MASK;
	 mode |= KONA_GPIO_DIR_OUTPUT;

	 kona_gpio_set_mode(gpio, mode);
	 kona_gpio_set_bit(gpio, value);
	 
	 spin_unlock_irqrestore(&gGpioLock, flags);

    return 0;

} /* gpio_kona_direction_output */

/****************************************************************************
*
*  Retrieve the value of a GPIO pin. Note that this returns zero or the raw
*   value.
*
*****************************************************************************/

static int gpio_kona_get( struct gpio_chip *chip, unsigned gpio )
{
	(void)chip;	/* Not required for this particular lower level API */

    return kona_gpio_get_bit(gpio);

} /* gpio_kona_get */

/****************************************************************************
*
*  Set the value of a GPIO pin
*
*****************************************************************************/

static void gpio_kona_set( struct gpio_chip *chip, unsigned gpio, int value )
{
	(void)chip;	/* Not required for this particular lower level API */

	 kona_gpio_set_bit(gpio, value);

} /* gpio_kona_set */

/****************************************************************************
*
*  Prints an ASCII representation of the GPIO pin
*
*   Currently, the dbg_show is for the entire chip. So we need to essentially
*   copy the functionality from gpiolib.
*
*****************************************************************************/

static void gpio_kona_dbg_show( struct seq_file *s, struct gpio_chip *chip )
{
    unsigned    gpio = chip->base;
    int         i;

    for ( i = 0; i < chip->ngpio; i++, gpio++ )
    {
        const char *label;
        int         is_out;

        if (( label = gpiochip_is_requested( chip, i )) == NULL )
        {
            /* gpio is currently unassigned */

            label = "---unreq----";
        }

		is_out = KONA_GPIO_CTR_REG(gpio) & KONA_GPIO_GPCTR_IOTR ;
        seq_printf(s, " gpio-%-3d (%-12s) %s %s",
            gpio, label, is_out ? "out" : "in ",
            chip->get(chip, i) ? "hi" : "lo" );

        if ( !is_out ) 
        {
            int		irq = gpio_to_irq(gpio);
            struct irq_desc	*desc = irq_desc + irq;

            /* This races with request_irq(), set_irq_type(),
             * and set_irq_wake() ... but those are "rare".
             *
             * More significantly, trigger type flags aren't
             * currently maintained by genirq.
             */
            if (irq >= 0 && desc->action) {
                char *trigger;

                switch (desc->status & IRQ_TYPE_SENSE_MASK) {
                case IRQ_TYPE_NONE:
                    trigger = "(default)";
                    break;
                case IRQ_TYPE_EDGE_FALLING:
                    trigger = "edge-falling";
                    break;
                case IRQ_TYPE_EDGE_RISING:
                    trigger = "edge-rising";
                    break;
                case IRQ_TYPE_EDGE_BOTH:
                    trigger = "edge-both";
                    break;
                case IRQ_TYPE_LEVEL_HIGH:
                    trigger = "level-high";
                    break;
                case IRQ_TYPE_LEVEL_LOW:
                    trigger = "level-low";
                    break;
                default:
                    trigger = "?trigger?";
                    break;
                }

                seq_printf(s, " irq-%d %s%s",
                    irq, trigger,
                    (desc->status & IRQ_WAKEUP)
                        ? " wakeup" : "");
            }

        }
        seq_printf( s, "\n");
    }
    
} /* gpio_kona_dbg_show */

/****************************************************************************
*
*  gpiolib chip description
*
*****************************************************************************/

/*
 * Note: If you need to add a field, like a register base, then create a new 
 *       structure which includes the gpio_chip as the first element and has 
 *       the custom fields after. See asm-arm/arch-pxa/gpio.c for an example. 
 */

static kona_gpio_chip gpio_kona_chip[] =
{
    [0] = /* We could have 6 chips here, but lower level kona API based on chip doesn't exist */
    {
        .reg  = (volatile uint32_t *)KONA_GPIO2_VA,
        .chip =
        {
            .label              = "kona",
            .direction_input    = gpio_kona_direction_input,
            .direction_output   = gpio_kona_direction_output,
            .get                = gpio_kona_get,
            .set                = gpio_kona_set,
            .dbg_show           = gpio_kona_dbg_show,
            .base               = 0,
            .ngpio              = NR_KONA_GPIO,
        },
    },
};

/****************************************************************************
*
*  brcm_init_gpio
*
*   Sets up gpiolib so that it's aware of how to manipulate our GPIOs
*
*****************************************************************************/
static int __init brcm_init_gpio( void )
{
    int ret;

    printk("%s called\n", __func__);
    ret = gpiochip_add( &gpio_kona_chip[0].chip );
    if (ret)
    {
        printk(KERN_ERR "%s: ret=%d, NR_KONA_GPIO=%d\n", __func__, ret, NR_KONA_GPIO);
        return ret;
    }

    return ret;

} /* brcm_init_gpio */

arch_initcall(brcm_init_gpio);
