/*****************************************************************************
* Copyright 2003 - 2008 Broadcom Corporation.  All rights reserved.
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

#if !defined( ASM_ARCH_REG_GPIO_H )
#define ASM_ARCH_REG_GPIO_H

/* ---- Include Files ---------------------------------------------------- */

#include <mach/isl_gpio_inline.h>

#if 0
#if defined(CONFIG_BCM_VCHIQ)
#include <linux/broadcom/vc_gpio.h>

#ifdef  GPIOHW_TOTAL_NUM_PINS
#undef  GPIOHW_TOTAL_NUM_PINS
#endif

#if (CONFIG_BCM_MAX_EXTERNAL_VIDEOCORES == 2)
#define  GPIOHW_TOTAL_NUM_PINS         (VC03_1_GPIO_PIN_OFFSET + VC03_1_GPIO_NUM_PINS)
#elif (CONFIG_BCM_MAX_EXTERNAL_VIDEOCORES == 1)
#define  GPIOHW_TOTAL_NUM_PINS         (VC03_0_GPIO_PIN_OFFSET + VC03_0_GPIO_NUM_PINS)
#else 
#error "CONFIG_BCM_MAX_EXTERNAL_VIDEOCORES must be defined as 1 or 2" 
#endif
#endif
#endif // end of #if 0.

// SARU #if defined( CONFIG_HAVE_GPIO_LIB ) && !defined( STANDALONE ) && !defined( USE_BCM_GPIO )
#if defined( CONFIG_ARCH_REQUIRE_GPIOLIB ) 

        #define   ARCH_NR_GPIOS   NR_ISL_GPIO
    #include <asm-generic/gpio.h>
    #define gpio_get_value(gpio)        __gpio_get_value(gpio)
    #define gpio_set_value(gpio,value)  __gpio_set_value(gpio,value)
    #define gpio_cansleep(gpio)         __gpio_cansleep(gpio)
#else

    /*
     * The else side of this definition provides a gpiolib emulation for versions
     * of the kernel which haven't got gpiolib ported to it yet.
     */
    /* These definitions occur in include/linux/broadcom/gpio.h */

#endif

#if 0
/* ---- Constants and Types ---------------------------------------------- */

/* ---- Variable Externs ------------------------------------------------- */

/* ---- Function Prototypes ---------------------------------------------- */

/****************************************************************************
*
*  Configure a GPIO pin as an input pin
*
*****************************************************************************/

static inline int bcm_gpio_direction_input( unsigned gpio )
{
	 uint32_t mode;

	 mode = isl_gpio_get_mode(gpio);
	 
	 mode &= ~GPIO_GPCTR0_IOTR_MASK;
	 mode |= ISL_GPIO_DIR_INPUT;

	 isl_gpio_set_mode(gpio, mode);
	 
    return 0;

} /* bcm_gpio_direction_input */

/****************************************************************************
*
*  Configure a GPIO pin as an output pin and sets its initial value.
*
*****************************************************************************/

static inline int bcm_gpio_direction_output( unsigned gpio, int initial_value )
{
	 uint32_t mode;

	 mode = isl_gpio_get_mode(gpio);
	 
	 mode &= ~GPIO_GPCTR0_IOTR_MASK;
	 mode |= ISL_GPIO_DIR_OUTPUT;

	 isl_gpio_set_mode(gpio, mode);
	 isl_gpio_set_bit(gpio, initial_value);
	 
    return 0;

} /* bcm_gpio_direction_output */

/****************************************************************************
*
*  Retrieve the value of a GPIO pin. Note that this returns zero or the raw
*   value.
*
*****************************************************************************/

static inline int bcm_gpio_get( unsigned gpio )
{
    return isl_gpio_get_bit(gpio);

} /* bcm_gpio_get */

/****************************************************************************
*
*  Set the value of a GPIO pin
*
*****************************************************************************/

static inline void bcm_gpio_set( unsigned gpio, int value )
{
	 isl_gpio_set_bit(gpio, value);

} /* bcm_gpio_set */

#if defined( __KERNEL__ ) && 0
#include <linux/init.h>
void __init brcm_init_gpio(void);
#endif
#endif

#endif  /* ASM_ARCH_REG_GPIO_H */

