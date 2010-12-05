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

// #include <mach/gpio_defs.h>
#include <mach/isl_gpio_inline.h>
// #include <mach/gpiomux.h>

/* ---- Public Variables ------------------------------------------------- */

/* ---- Private Constants and Types -------------------------------------- */

typedef struct
{
    struct gpio_chip chip;
    volatile unsigned int *reg;

} bigisland_gpio_chip;

/* ---- Private Function Prototypes -------------------------------------- */

/* ---- Private Variables ------------------------------------------------ */

static spinlock_t gGpioLock = SPIN_LOCK_UNLOCKED;

/* ---- Functions -------------------------------------------------------- */

/****************************************************************************
*
*  Configure a GPIO pin as an input pin
*
*****************************************************************************/

static int gpio_bigisland_direction_input( struct gpio_chip *chip, unsigned gpio )
{
	 uint32_t mode;
	 unsigned long flags;

	 (void)chip;	/* Not required for this particular lower level API */
	
	 spin_lock_irqsave(&gGpioLock, flags);

	 mode = isl_gpio_get_mode(gpio);
	 
	 mode &= ~GPIO_GPCTR0_IOTR_MASK;
	 mode |= ISL_GPIO_DIR_INPUT;

	 isl_gpio_set_mode(gpio, mode);
	 
	 spin_unlock_irqrestore(&gGpioLock, flags);
    return 0;

} /* gpio_bigisland_direction_input */

/****************************************************************************
*
*  Configure a GPIO pin as an output pin and sets its initial value.
*
*****************************************************************************/

static int gpio_bigisland_direction_output( struct gpio_chip *chip, unsigned gpio, int value )
{
	 uint32_t mode;
	 unsigned long flags;

	(void)chip;	/* Not required for this particular lower level API */

	 spin_lock_irqsave(&gGpioLock, flags);

	 mode = isl_gpio_get_mode(gpio);
	 
	 mode &= ~GPIO_GPCTR0_IOTR_MASK;
	 mode |= ISL_GPIO_DIR_OUTPUT;

	 isl_gpio_set_mode(gpio, mode);
	 isl_gpio_set_bit(gpio, value);
	 
	 spin_unlock_irqrestore(&gGpioLock, flags);

    return 0;

} /* gpio_bigisland_direction_output */

/****************************************************************************
*
*  Retrieve the value of a GPIO pin. Note that this returns zero or the raw
*   value.
*
*****************************************************************************/

static int gpio_bigisland_get( struct gpio_chip *chip, unsigned gpio )
{
	(void)chip;	/* Not required for this particular lower level API */

    return isl_gpio_get_bit(gpio);

} /* gpio_bigisland_get */

/****************************************************************************
*
*  Set the value of a GPIO pin
*
*****************************************************************************/

static void gpio_bigisland_set( struct gpio_chip *chip, unsigned gpio, int value )
{
	(void)chip;	/* Not required for this particular lower level API */

	 isl_gpio_set_bit(gpio, value);

} /* gpio_bigisland_set */

/****************************************************************************
*
*  Prints an ASCII representation of the GPIO pin
*
*   Currently, the dbg_show is for the entire chip. So we need to essentially
*   copy the functionality from gpiolib.
*
*****************************************************************************/

static void gpio_bigisland_dbg_show( struct seq_file *s, struct gpio_chip *chip )
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

        // is_out = gpio_direction_is_output( gpio ); SARU commented this out. See what is the relavent thing ??? 
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
    
} /* gpio_bigisland_dbg_show */

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

static bigisland_gpio_chip gpio_bigisland_chip[] =
{
    [0] = /* We could have 6 chips here, but lower level isl API based on chip doesn't exist */
    {
        .reg  = (volatile uint32_t *)KONA_GPIO2_VA,
        .chip =
        {
            .label              = "bigisland",
            .direction_input    = gpio_bigisland_direction_input,
            .direction_output   = gpio_bigisland_direction_output,
            .get                = gpio_bigisland_get,
            .set                = gpio_bigisland_set,
            .dbg_show           = gpio_bigisland_dbg_show,
            .base               = 0,
            .ngpio              = NR_ISL_GPIO,
        },
    },
};

#if 0
/****************************************************************************/
/**
*  @brief   mux_isRequested
*
*  Find out if a gpio pin has already been requested in the gpiolib sense.
*
*  @return
*     none
*/
/****************************************************************************/
static const char * mux_isRequested(int pin)
{
   int i;

   /*
    * For now, convert pin number to chip and offset.  Later, convert this
    * into a gpiomux register/chip array structure. Since this is typically
    * init time code, it may not be worth worrying about for this one case.
    */
   for ( i = 0; i < ARRAY_LEN(gpio_bigisland_chip); i++ )
   {
      struct gpio_chip *chipp = &gpio_bigisland_chip[i].chip;
      int base = chipp->base;
      if ((pin >= base) && (pin < base + chipp->ngpio))
      {
         return gpiochip_is_requested(chipp, pin - base);
      }
   }
   return NULL;

} /* mux_isRequested */

/****************************************************************************/
/**
*  @brief   mux_request
*
*  Request a pin from gpiolib.
*
*/
/****************************************************************************/
static int mux_request(int pin, const char *label)
{
   return gpio_request(pin, label);

} /* mux_request */

/****************************************************************************/
/**
*  @brief   mux_free
*
*  Free a pin from gpiolib.
*
*/
/****************************************************************************/
static void mux_free(int pin)
{
   return gpio_free(pin);

} /* mux_free */

/****************************************************************************/
/**
*  @brief   mux_initfunc
*
*  extra function to call in gpiomux init code.
*
*/
/****************************************************************************/
static void mux_initfunc(void)
{
   unsigned int i;
	printk("%s\n", __func__);
   for ( i = 0; i < gpio_defs_MAX_PINS; i++ )
   {
      if ( !(i >= GPIO_52_UARTB_UTXD_VC_TCK_SSP1_CLK && i <= GPIO_53_UARTB_URXD_VC_TMS_SSP1_FS) )
      {
         chipregHw_setPinFunction( (int)i, CHIPREGHW_PIN_FUNCTION_DEFAULT );
         chipregHw_setPinInputType( (int)i, CHIPREGHW_PIN_INPUTTYPE_DEFAULT );
         chipregHw_setPinPullup( (int)i, CHIPREGHW_PIN_PULL_DEFAULT );
         chipregHw_setPinOutputCurrent( (int)i, CHIPREGHW_PIN_CURRENT_STRENGTH_DEFAULT );
         chipregHw_setPinSlewRate( (int)i, CHIPREGHW_PIN_SLEW_RATE_DEFAULT );
      }
   }
}
#endif

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
   // gpiomux_init_t initstruct;

	printk("%s called\n", __func__);
   ret = gpiochip_add( &gpio_bigisland_chip[0].chip );
   if (ret)
   {
       printk(KERN_ERR "%s: ret=%d, NR_ISL_GPIO=%d\n",
              __func__, ret, NR_ISL_GPIO);
       return ret;
   }

#if 0
   /* Initialize callbacks for gpiomux function calls into gpiolib */
   initstruct.request_gpio = mux_request;
   initstruct.free_gpio = mux_free;
   initstruct.is_requested = mux_isRequested;
   initstruct.initfunc = mux_initfunc;

   gpiomux_Init(&initstruct);

   /* Manually set UART pins back to default, because these pins are skipped in gpiomux_init,
      and then re-request them back as UART so that gpiolib will own them
      */

   /* BIGISLAND Implementation */
   chipregHw_setPinFunction( GPIO_52_UARTB_UTXD_VC_TCK_SSP1_CLK, CHIPREGHW_PIN_FUNCTION_DEFAULT );
   chipregHw_setPinFunction( GPIO_53_UARTB_URXD_VC_TMS_SSP1_FS, CHIPREGHW_PIN_FUNCTION_DEFAULT );

   ret = gpiomux_requestGroup(gpiomux_group_uart, 0, "Debug UART");

   if (ret)
   {
       printk(KERN_ERR "%s: ret=%d, gpiomux_requestGroup failed\n", __func__, ret );
       return ret;
   }
#endif
	return ret;

} /* brcm_init_gpio */

arch_initcall(brcm_init_gpio);
