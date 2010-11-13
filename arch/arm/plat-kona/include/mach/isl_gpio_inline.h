/*****************************************************************************
*  Copyright 2001 - 2008 Broadcom Corporation.  All rights reserved.
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2, available at
*  http://www.gnu.org/licenses/old-license/gpl-2.0.html (the "GPL").
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
*****************************************************************************/
/**
*
*  @file   isl_gpio.h
*  @brief  gpio cHAL interface
*  @note  
*
*****************************************************************************/
#ifndef ISL_GPIO_INLINE_H__
#define ISL_GPIO_INLINE_H__

#ifdef __cplusplus
extern "C" {
#endif

// SARU #include <mach/csp/mm_io.h>
// SARU #include <mach/csp/reg.h>

#include <mach/brcm_rdb_gpio.h>
#include <mach/map.h>
	/**
 * @addtogroup cHAL_Interface 
 * @{
 */

#define __REG32(x)      (*((volatile uint32_t *)(x)))
 
#define NR_ISL_GPIO	192

#define ISL_GPIO_REG_OFFSET( data ) ( ((data)/32) )
#define ISL_GPIO_BIT_OFFSET( data ) ( (data) - ((ISL_GPIO_REG_OFFSET(data))*32) )

/**
*  Define:  ISL_GPIO Control Register
*  
******************************************************************************/
#define ISL_GPIO_GPCTR_IOTR        0x0001
#define ISL_GPIO_GPCTR_IOTR_SHIFT  0
#define ISL_GPIO_GPCTR_PED         0x0002
#define ISL_GPIO_GPCTR_PED_SHIFT   1
#define ISL_GPIO_GPCTR_PUD         0x0004
#define ISL_GPIO_GPCTR_PUD_SHIFT   2
#define ISL_GPIO_GPCTR_ITR         0x0018
#define ISL_GPIO_GPCTR_ITR_SHIFT   3
#define ISL_GPIO_GPCTR_DBR         0x01E0
#define ISL_GPIO_GPCTR_DBR_SHIFT   5

/**
*  Define:  ISL_GPIO_VALUE_xxx
*
*  Following defines values for ISL_GPIO pins.
*****************************************************************************/

/**
*  Define:  ISL_GPIO_DIR_xxx/ISL_GPIO_INT_xxx
*
*  Following defines config values for ISL_GPIO pins.
******************************************************************************/
#define ISL_GPIO_DIR_INPUT          0x01
#define ISL_GPIO_DIR_OUTPUT         0x00
#define ISL_GPIO_INT_DISABLE		  0
#define ISL_GPIO_INT_RISING         1
#define ISL_GPIO_INT_FALLING        2
#define ISL_GPIO_INT_BOTH           3

/**
*  Define:  ISL_GPIO_DB_xxx
*
*  Following defines debounce values for ISL_GPIO pins.
*****************************************************************************/
#define ISL_GPIO_DB_DISABLE         0x00
#define ISL_GPIO_DB_ENABLE          0x08
#define ISL_GPIO_DB_1MS        	 	0
#define ISL_GPIO_DB_2MS        	 	1
#define ISL_GPIO_DB_4MS        	 	2
#define ISL_GPIO_DB_8MS        	 	3
#define ISL_GPIO_DB_16MS       	 	4
#define ISL_GPIO_DB_32MS       	 	5
#define ISL_GPIO_DB_64MS       	 	6
#define ISL_GPIO_DB_128MS      	 	7
#define ISL_GPIO_DB_NONE    			8
/**
*  Define:  ISL_GPIO_PULL_xxx
*
*  Following defines pull values for ISL_GPIO pins.
*****************************************************************************/
#define ISL_GPIO_PULL_DISABLE        0x00
#define ISL_GPIO_PULL_ENABLE         0x01
#define ISL_GPIO_PULL_DOWN        	 (0 << 1)
#define ISL_GPIO_PULL_UP         	 (1 << 1)

#define ISL_GPIO_IRQ_ENABLE          0x01
#define ISL_GPIO_IRQ_DISABLE         0x00

#define ISL_GPIO_PWR                 0x00a5a501

// #define ISL_GPIO_BASE MM_IO_BASE_GPIO2
#define ISL_GPIO_BASE KONA_GPIO2_VA

#define ISL_GPIO_PWR_REG        (__REG32(ISL_GPIO_BASE + GPIO_GPPWR_OFFSET))
#define ISL_GPIO_PLSR_REGSET(set) (__REG32(ISL_GPIO_BASE + GPIO_GPPLSR0_OFFSET + (set*4)))
#define ISL_GPIO_PLSR_REG(gpio) (__REG32(ISL_GPIO_BASE + GPIO_GPPLSR0_OFFSET + (ISL_GPIO_REG_OFFSET(gpio)*4)))
#define ISL_GPIO_CTR_REG(gpio) 	(__REG32(ISL_GPIO_BASE + GPIO_GPCTR0_OFFSET + ((gpio)*4)))
#define ISL_GPIO_IMR_REG(gpio) 	(__REG32(ISL_GPIO_BASE + GPIO_IMR0_OFFSET + (ISL_GPIO_REG_OFFSET(gpio)*4)))
#define ISL_GPIO_IMRC_REG(gpio) (__REG32(ISL_GPIO_BASE + GPIO_IMRC0_OFFSET + (ISL_GPIO_REG_OFFSET(gpio)*4)))
#define ISL_GPIO_ORC_REG(gpio) 	(__REG32(ISL_GPIO_BASE + GPIO_GPORC0_OFFSET + (ISL_GPIO_REG_OFFSET(gpio)*4)))
#define ISL_GPIO_ORS_REG(gpio) 	(__REG32(ISL_GPIO_BASE + GPIO_GPORS0_OFFSET + (ISL_GPIO_REG_OFFSET(gpio)*4)))
#define ISL_GPIO_OR_REG(gpio) 	(__REG32(ISL_GPIO_BASE + GPIO_GPOR0_OFFSET + (ISL_GPIO_REG_OFFSET(gpio)*4)))
#define ISL_GPIO_IR_REG(gpio) 	(__REG32(ISL_GPIO_BASE + GPIO_GPIR0_OFFSET + (ISL_GPIO_REG_OFFSET(gpio)*4)))
#define ISL_GPIO_ISR_REG(gpio) 	(__REG32(ISL_GPIO_BASE + GPIO_ISR0_OFFSET + (ISL_GPIO_REG_OFFSET(gpio)*4)))

	
/**
*
*  @brief  gpio initialization
*
*  @param  none
*
*  @return none 
*
******************************************************************************/
static inline void isl_gpio_init(void)
{
   ISL_GPIO_PWR_REG = ISL_GPIO_PWR;
   ISL_GPIO_PLSR_REGSET(0) = 0x00000000;
   ISL_GPIO_PWR_REG = ISL_GPIO_PWR;
   ISL_GPIO_PLSR_REGSET(1) = 0x00000000;
   ISL_GPIO_PWR_REG = ISL_GPIO_PWR;
   ISL_GPIO_PLSR_REGSET(2) = 0x00000000;
   ISL_GPIO_PWR_REG = ISL_GPIO_PWR;
   ISL_GPIO_PLSR_REGSET(3) = 0x00000000;
   ISL_GPIO_PWR_REG = ISL_GPIO_PWR;
   ISL_GPIO_PLSR_REGSET(4) = 0x00000000;
   ISL_GPIO_PWR_REG = ISL_GPIO_PWR;
   ISL_GPIO_PLSR_REGSET(5) = 0x00000000;
}

/**
*
*  @brief  gpio set bit 
*
*  @param  gpio  (in) ISL_GPIO ID 
*  @param  value  	(in) value to set 
*
*  @return none 
*
*  @note   For any pins that toggles between input & output, make sure the pin
*          is in output mode before calling this function.
******************************************************************************/
static inline void isl_gpio_set_bit(int gpio, int value)
{
	if (value) 
	{
		ISL_GPIO_ORS_REG(gpio) = (1 << ISL_GPIO_BIT_OFFSET(gpio));	/* Atomic write */
	}
	else
	{
		ISL_GPIO_ORC_REG(gpio) = (1 << ISL_GPIO_BIT_OFFSET(gpio));	/* Atomic write */
	}
}

/**
*
*  @brief  gpio get bit 
*
*  @param  gpio  (in) ISL_GPIO ID
*
*  @return ISL_GPIO pin status 
******************************************************************************/
static inline uint32_t isl_gpio_get_bit(uint32_t gpio)
{
	if ((ISL_GPIO_CTR_REG(gpio) & ISL_GPIO_GPCTR_IOTR) == ISL_GPIO_DIR_OUTPUT) 
	{
		return ((ISL_GPIO_OR_REG(gpio) >> ISL_GPIO_BIT_OFFSET(gpio)) & 1);
	}
	return ((ISL_GPIO_IR_REG(gpio) >> ISL_GPIO_BIT_OFFSET(gpio)) & 1);
}

/**
*
*  @brief  gpio get mode 
*
*  @param  handle (in) Handle returned in isl_gpio_init() 
*  @param  gpio     (in) ISL_GPIO ID 
*
*  @return mode info 
******************************************************************************/
static inline uint32_t isl_gpio_get_mode(uint32_t gpio)
{
	uint32_t rdvalue = ISL_GPIO_CTR_REG(gpio);
	uint32_t direction = rdvalue & 0x1;
	uint32_t trigger_type = (rdvalue & ISL_GPIO_GPCTR_ITR) >> ISL_GPIO_GPCTR_ITR_SHIFT;

	return ((trigger_type << 1)|direction);
}

/**
*
*  @brief  gpio set pull 
*
*  @param  handle (in) Handle returned in isl_gpio_init() 
*  @param  gpio     (in) ISL_GPIO ID 
*  @param  value  (in) pull value 
*
*  @return none 
******************************************************************************/
static inline void isl_gpio_set_pull(uint32_t gpio, uint32_t value)
{
	uint32_t rdvalue = ISL_GPIO_CTR_REG(gpio);

	if (value & ISL_GPIO_PULL_ENABLE)
		rdvalue |= ISL_GPIO_GPCTR_PED;
	else
		rdvalue &= ~ISL_GPIO_GPCTR_PED;

	if (value & ISL_GPIO_PULL_UP)
		rdvalue |= ISL_GPIO_GPCTR_PUD;
	else
		rdvalue &= ~ISL_GPIO_GPCTR_PUD;

	/* Set the control register */
	ISL_GPIO_CTR_REG(gpio) = rdvalue;	/* Read-modify-write */
}

/**
*
*  @brief  gpio get pull 
*
*  @param  handle (in) Handle returned in isl_gpio_init() 
*  @param  gpio     (in) ISL_GPIO ID
*
*  @return pull info 
******************************************************************************/
static inline uint32_t isl_gpio_get_pull(uint32_t gpio)
{
	return ((ISL_GPIO_CTR_REG(gpio) &(ISL_GPIO_GPCTR_PED|ISL_GPIO_GPCTR_PUD))>>1);
}

/**
*
*  @brief  gpio set debounce 
*
*  @param  handle (in) Handle returned in isl_gpio_init() 
*  @param  gpio     (in) ISL_GPIO ID
*  @param  value  (in) debounce value 
*
*  @return none 
******************************************************************************/
static inline int isl_gpio_set_db(uint32_t gpio, uint32_t value)
{
	uint32_t rdvalue = ISL_GPIO_CTR_REG(gpio) & ~ISL_GPIO_GPCTR_DBR;

	switch (value)
	{
		case ISL_GPIO_DB_1MS :
			rdvalue |= (ISL_GPIO_DB_ENABLE | ISL_GPIO_DB_1MS) << ISL_GPIO_GPCTR_DBR_SHIFT;
			break;
		case ISL_GPIO_DB_2MS :
			rdvalue |= (ISL_GPIO_DB_ENABLE | ISL_GPIO_DB_2MS) << ISL_GPIO_GPCTR_DBR_SHIFT;
			break;
		case ISL_GPIO_DB_4MS :
			rdvalue |= (ISL_GPIO_DB_ENABLE | ISL_GPIO_DB_4MS) << ISL_GPIO_GPCTR_DBR_SHIFT;
			break;
		case ISL_GPIO_DB_8MS :
			rdvalue |= (ISL_GPIO_DB_ENABLE | ISL_GPIO_DB_8MS) << ISL_GPIO_GPCTR_DBR_SHIFT;
			break;
		case ISL_GPIO_DB_16MS :
			rdvalue |= (ISL_GPIO_DB_ENABLE | ISL_GPIO_DB_16MS) << ISL_GPIO_GPCTR_DBR_SHIFT;
			break;
		case ISL_GPIO_DB_32MS :
			rdvalue |= (ISL_GPIO_DB_ENABLE | ISL_GPIO_DB_32MS) << ISL_GPIO_GPCTR_DBR_SHIFT;
			break;
		case ISL_GPIO_DB_64MS :
			rdvalue |= (ISL_GPIO_DB_ENABLE | ISL_GPIO_DB_64MS) << ISL_GPIO_GPCTR_DBR_SHIFT;
			break;
		case ISL_GPIO_DB_128MS :
			rdvalue |= (ISL_GPIO_DB_ENABLE | ISL_GPIO_DB_128MS) << ISL_GPIO_GPCTR_DBR_SHIFT;
			break;
		case ISL_GPIO_DB_NONE :
			rdvalue &= (~ISL_GPIO_GPCTR_DBR);
			break;
		default :
			return -1;
	}

	/* Set the control register */
	ISL_GPIO_CTR_REG(gpio) = rdvalue;	/* Read-modify-write */

	return 0;
}


/**
*
*  @brief  gpio get debounce 
*
*  @param  handle (in) Handle returned in isl_gpio_init() 
*  @param  gpio     (in) ISL_GPIO ID 
*
*  @return debounce value 
******************************************************************************/
static inline uint32_t isl_gpio_get_db(uint32_t gpio)
{
	uint32_t value = ((ISL_GPIO_CTR_REG(gpio) & ISL_GPIO_GPCTR_DBR)>> ISL_GPIO_GPCTR_DBR_SHIFT);

	if( value & ISL_GPIO_DB_ENABLE) 
		value = ((value & ~ISL_GPIO_DB_ENABLE)<<1) | 0x01;
	else 
		value = value<<1;

	return value;
}


/**
*
*  @brief  gpio set interrupt mask 
*
*  @param  handle 	(in) Handle returned in isl_gpio_init() 
*  @param  gpio  (in) ISL_GPIO ID 
*
*  @return none 
******************************************************************************/
static inline void isl_gpio_set_irqmask(uint32_t gpio, uint32_t value)
{
	/* 
	 * IMR is a R/W register and documentation doesn't say if this is atomic. 
	 * IMRC is a W/O Register and doesn't need a read and is atomic.
	 */
	if (value) 
	{
		/* FIXME not sure if this is atomic */
		ISL_GPIO_IMR_REG(gpio) |= (1 << ISL_GPIO_BIT_OFFSET(gpio));
	}
	else
	{
		/* Atomic write */
		ISL_GPIO_IMRC_REG(gpio) = (1 << ISL_GPIO_BIT_OFFSET(gpio));
	}
	
}

/**
*
*  @brief  gpio get interrupt mask 
*
*  @param  handle  	(in) Handle returned in isl_gpio_init() 
*  @param  gpio   (in) ISL_GPIO ID 
*
*  @return interrupt mask  
******************************************************************************/
static inline uint32_t isl_gpio_get_irqmask(uint32_t gpio)
{
	return((ISL_GPIO_IMR_REG(gpio) >> ISL_GPIO_BIT_OFFSET(gpio)) & 1);
}

/**
*
*  @brief  gpio clear interrupt mask 
*
*  @param  handle  	(in) Handle returned in isl_gpio_init() 
*  @param  gpio   (in) ISL_GPIO ID 
*
*  @return none  
******************************************************************************/
static inline void isl_gpio_clear_irqmask(uint32_t gpio)
{
	/* Atomic write */
	ISL_GPIO_IMRC_REG(gpio) = (1 << ISL_GPIO_BIT_OFFSET(gpio));
}

/**
*
*  @brief  gpio get intrrupt status 
*
*  @param  handle 	(in) Handle returned in isl_gpio_init() 
*  @param  gpio   (in) ISL_GPIO ID 
*
*  @return status 
******************************************************************************/
static inline uint32_t isl_gpio_get_irqstatus(uint32_t gpio)
{
	return ((ISL_GPIO_ISR_REG(gpio) >> ISL_GPIO_BIT_OFFSET(gpio)) & 1);
}

/**
*
*  @brief  gpio get intrrupt status 
*
*  @param  handle 	(in) Handle returned in isl_gpio_init() 
*  @param  gpio   (in) ISL_GPIO ID 
*
*  @return status 
******************************************************************************/
static inline uint32_t isl_gpio_get_irqstatus_register(uint32_t gpio)
{
	/* Atomic write to clear int, read valid for statuses only */
	return ISL_GPIO_ISR_REG(gpio);
}

/**
*
*  @brief  gpio clear int status 
*
*  @param  handle 	(in) Handle returned in isl_gpio_init() 
*  @param  gpio  (in) ISL_GPIO ID 
*
*  @return none 
******************************************************************************/
static inline void isl_gpio_clear_int(uint32_t gpio)
{
	/* Atomic write to clear int, read valid for statuses only */
	ISL_GPIO_ISR_REG(gpio) = (1 << ISL_GPIO_BIT_OFFSET(gpio));
}

/**
*
*  @brief  gpio clear int status 
*
*  @param  handle 	(in) Handle returned in isl_gpio_init() 
*  @param  gpio  (in) ISL_GPIO ID 
*
*  @return none 
******************************************************************************/
static inline void isl_gpio_clear_int_register(uint32_t gpio)
{
	/* Atomic write, read valid for statuses only */
	ISL_GPIO_ISR_REG(gpio) = ~0;
}

/**
*
*  @brief  gpio disable irq 
*
*  @param  handle (in) Handle returned in isl_gpio_init() 
*  @param  gpio     (in) ISL_GPIO ID 
*
*  @return none 
******************************************************************************/
static inline void isl_gpio_disable_int(uint32_t gpio)
{
	ISL_GPIO_CTR_REG(gpio) &= ~ISL_GPIO_GPCTR_ITR;  /* Read-modify-write */
}

/**
*
*  @brief  isl_gpio_set_mode
*
*  @param  handle (in) Handle returned in isl_gpio_init() 
*  @param  gpio     (in) ISL_GPIO ID
*
*  @return none 
******************************************************************************/
static inline void isl_gpio_set_mode(uint32_t gpio, uint32_t mode)
{
	uint32_t direction = mode & 0x01;
	uint32_t trigger_type = (mode >> 1) & 3;
	uint32_t val = ISL_GPIO_CTR_REG(gpio);

	/* Setup pin direction */

	val &= ~ISL_GPIO_GPCTR_IOTR;

	if (direction == ISL_GPIO_DIR_INPUT) 
	{
		val |= (ISL_GPIO_DIR_INPUT << ISL_GPIO_GPCTR_IOTR_SHIFT);
	}
	else
	{
		val |= (ISL_GPIO_DIR_OUTPUT << ISL_GPIO_GPCTR_IOTR_SHIFT);
	}

	/* Setup pin pull type */

	val &= ~ISL_GPIO_GPCTR_ITR;
	switch (trigger_type)
	{
		case ISL_GPIO_INT_DISABLE :
			val |= (ISL_GPIO_INT_DISABLE << ISL_GPIO_GPCTR_ITR_SHIFT);
			break;
		case ISL_GPIO_INT_RISING :
			val |= (ISL_GPIO_INT_RISING << ISL_GPIO_GPCTR_ITR_SHIFT);
			break;
		case ISL_GPIO_INT_FALLING :
			val |= (ISL_GPIO_INT_FALLING << ISL_GPIO_GPCTR_ITR_SHIFT);
			break;
		case ISL_GPIO_INT_BOTH :
			val |= (ISL_GPIO_INT_BOTH << ISL_GPIO_GPCTR_ITR_SHIFT);
			break;
	}

	/* Set the control register */
	ISL_GPIO_CTR_REG(gpio) = val;	/* Read-modify-write */
}


/** @} */

#ifdef __cplusplus
}
#endif

#endif /* ISL_GPIO_INLINE_H__ */
