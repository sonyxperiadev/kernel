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
*  @file   chal_gpio.h
*  @brief  gpio cHAL interface
*  @note  
*
*****************************************************************************/
#ifndef CHAL_GPIO_INLINE_H__
#define CHAL_GPIO_INLINE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <mach/csp/mm_io.h>
#include <mach/csp/reg.h>

/**
 * @addtogroup cHAL_Interface 
 * @{
 */

 
#define CHAL_GPIO_REG_OFFSET( data ) ( ((data)/32) )
#define CHAL_GPIO_BIT_OFFSET( data ) ( (data) - ((CHAL_GPIO_REG_OFFSET(data))*32) )

/**
*  Define:  CHAL_GPIO Control Register
*  
******************************************************************************/
#define CHAL_GPIO_GPCTR_IOTR        0x0001
#define CHAL_GPIO_GPCTR_IOTR_SHIFT  0
#define CHAL_GPIO_GPCTR_PED         0x0002
#define CHAL_GPIO_GPCTR_PED_SHIFT   1
#define CHAL_GPIO_GPCTR_PUD         0x0004
#define CHAL_GPIO_GPCTR_PUD_SHIFT   2
#define CHAL_GPIO_GPCTR_ITR         0x0018
#define CHAL_GPIO_GPCTR_ITR_SHIFT   3
#define CHAL_GPIO_GPCTR_DBR         0x01E0
#define CHAL_GPIO_GPCTR_DBR_SHIFT   5

/**
*  Define:  CHAL_GPIO_VALUE_xxx
*
*  Following defines values for CHAL_GPIO pins.
*****************************************************************************/

/**
*  Define:  CHAL_GPIO_DIR_xxx/CHAL_GPIO_INT_xxx
*
*  Following defines config values for CHAL_GPIO pins.
******************************************************************************/
#define CHAL_GPIO_DIR_INPUT          0x01
#define CHAL_GPIO_DIR_OUTPUT         0x00
#define CHAL_GPIO_INT_DISABLE		  0
#define CHAL_GPIO_INT_RISING         1
#define CHAL_GPIO_INT_FALLING        2
#define CHAL_GPIO_INT_BOTH           3

/**
*  Define:  CHAL_GPIO_DB_xxx
*
*  Following defines debounce values for CHAL_GPIO pins.
*****************************************************************************/
#define CHAL_GPIO_DB_DISABLE         0x00
#define CHAL_GPIO_DB_ENABLE          0x08
#define CHAL_GPIO_DB_1MS        	 	0
#define CHAL_GPIO_DB_2MS        	 	1
#define CHAL_GPIO_DB_4MS        	 	2
#define CHAL_GPIO_DB_8MS        	 	3
#define CHAL_GPIO_DB_16MS       	 	4
#define CHAL_GPIO_DB_32MS       	 	5
#define CHAL_GPIO_DB_64MS       	 	6
#define CHAL_GPIO_DB_128MS      	 	7
#define CHAL_GPIO_DB_NONE    			8
/**
*  Define:  CHAL_GPIO_PULL_xxx
*
*  Following defines pull values for CHAL_GPIO pins.
*****************************************************************************/
#define CHAL_GPIO_PULL_DISABLE        0x00
#define CHAL_GPIO_PULL_ENABLE         0x01
#define CHAL_GPIO_PULL_DOWN        	 (0 << 1)
#define CHAL_GPIO_PULL_UP         	 (1 << 1)

#define CHAL_GPIO_IRQ_ENABLE          0x01
#define CHAL_GPIO_IRQ_DISABLE         0x00

#define CHAL_GPIO_PWR                 0x00a5a501

#define CHAL_GPIO_BASE MM_IO_BASE_GPIO2

#define GPIO_GPPWR_OFFSET	0x00000520
#define GPIO_GPPLSR0_OFFSET	0x00000500
#define GPIO_GPCTR0_OFFSET	0x00000100
#define GPIO_IMR0_OFFSET	0x000000A0
#define GPIO_IMRC0_OFFSET	0x000000C0
#define GPIO_GPORC0_OFFSET	0x00000060
#define GPIO_GPORS0_OFFSET	0x00000040
#define GPIO_GPOR0_OFFSET	0x00000000
#define GPIO_GPIR0_OFFSET	0x00000020
#define GPIO_ISR0_OFFSET	0x00000080
#define GPIO_GPCTR0_IOTR_MASK	0x00000001


#define CHAL_GPIO_PWR_REG        (__REG32(CHAL_GPIO_BASE + GPIO_GPPWR_OFFSET))
#define CHAL_GPIO_PLSR_REGSET(set) (__REG32(CHAL_GPIO_BASE + GPIO_GPPLSR0_OFFSET + (set*4)))
#define CHAL_GPIO_PLSR_REG(gpio) (__REG32(CHAL_GPIO_BASE + GPIO_GPPLSR0_OFFSET + (CHAL_GPIO_REG_OFFSET(gpio)*4)))
#define CHAL_GPIO_CTR_REG(gpio) 	(__REG32(CHAL_GPIO_BASE + GPIO_GPCTR0_OFFSET + ((gpio)*4)))
#define CHAL_GPIO_IMR_REG(gpio) 	(__REG32(CHAL_GPIO_BASE + GPIO_IMR0_OFFSET + (CHAL_GPIO_REG_OFFSET(gpio)*4)))
#define CHAL_GPIO_IMRC_REG(gpio) (__REG32(CHAL_GPIO_BASE + GPIO_IMRC0_OFFSET + (CHAL_GPIO_REG_OFFSET(gpio)*4)))
#define CHAL_GPIO_ORC_REG(gpio) 	(__REG32(CHAL_GPIO_BASE + GPIO_GPORC0_OFFSET + (CHAL_GPIO_REG_OFFSET(gpio)*4)))
#define CHAL_GPIO_ORS_REG(gpio) 	(__REG32(CHAL_GPIO_BASE + GPIO_GPORS0_OFFSET + (CHAL_GPIO_REG_OFFSET(gpio)*4)))
#define CHAL_GPIO_OR_REG(gpio) 	(__REG32(CHAL_GPIO_BASE + GPIO_GPOR0_OFFSET + (CHAL_GPIO_REG_OFFSET(gpio)*4)))
#define CHAL_GPIO_IR_REG(gpio) 	(__REG32(CHAL_GPIO_BASE + GPIO_GPIR0_OFFSET + (CHAL_GPIO_REG_OFFSET(gpio)*4)))
#define CHAL_GPIO_ISR_REG(gpio) 	(__REG32(CHAL_GPIO_BASE + GPIO_ISR0_OFFSET + (CHAL_GPIO_REG_OFFSET(gpio)*4)))

	
/**
*
*  @brief  gpio initialization
*
*  @param  none
*
*  @return none 
*
******************************************************************************/
static inline void chal_gpio_init(void)
{
   CHAL_GPIO_PWR_REG = CHAL_GPIO_PWR;
   CHAL_GPIO_PLSR_REGSET(0) = 0x00000000;
   CHAL_GPIO_PWR_REG = CHAL_GPIO_PWR;
   CHAL_GPIO_PLSR_REGSET(1) = 0x00000000;
   CHAL_GPIO_PWR_REG = CHAL_GPIO_PWR;
   CHAL_GPIO_PLSR_REGSET(2) = 0x00000000;
   CHAL_GPIO_PWR_REG = CHAL_GPIO_PWR;
   CHAL_GPIO_PLSR_REGSET(3) = 0x00000000;
   CHAL_GPIO_PWR_REG = CHAL_GPIO_PWR;
   CHAL_GPIO_PLSR_REGSET(4) = 0x00000000;
   CHAL_GPIO_PWR_REG = CHAL_GPIO_PWR;
   CHAL_GPIO_PLSR_REGSET(5) = 0x00000000;
}

/**
*
*  @brief  gpio set bit 
*
*  @param  gpio  (in) CHAL_GPIO ID 
*  @param  value  	(in) value to set 
*
*  @return none 
*
*  @note   For any pins that toggles between input & output, make sure the pin
*          is in output mode before calling this function.
******************************************************************************/
static inline void chal_gpio_set_bit(int gpio, int value)
{
	if (value) 
	{
		CHAL_GPIO_ORS_REG(gpio) = (1 << CHAL_GPIO_BIT_OFFSET(gpio));	/* Atomic write */
	}
	else
	{
		CHAL_GPIO_ORC_REG(gpio) = (1 << CHAL_GPIO_BIT_OFFSET(gpio));	/* Atomic write */
	}
}

/**
*
*  @brief  gpio get bit 
*
*  @param  gpio  (in) CHAL_GPIO ID
*
*  @return CHAL_GPIO pin status 
******************************************************************************/
static inline uint32_t chal_gpio_get_bit(uint32_t gpio)
{
	if ((CHAL_GPIO_CTR_REG(gpio) & CHAL_GPIO_GPCTR_IOTR) == CHAL_GPIO_DIR_OUTPUT) 
	{
		return ((CHAL_GPIO_OR_REG(gpio) >> CHAL_GPIO_BIT_OFFSET(gpio)) & 1);
	}
	return ((CHAL_GPIO_IR_REG(gpio) >> CHAL_GPIO_BIT_OFFSET(gpio)) & 1);
}

/**
*
*  @brief  gpio get mode 
*
*  @param  handle (in) Handle returned in chal_gpio_init() 
*  @param  gpio     (in) CHAL_GPIO ID 
*
*  @return mode info 
******************************************************************************/
static inline uint32_t chal_gpio_get_mode(uint32_t gpio)
{
	uint32_t rdvalue = CHAL_GPIO_CTR_REG(gpio);
	uint32_t direction = rdvalue & 0x1;
	uint32_t trigger_type = (rdvalue & CHAL_GPIO_GPCTR_ITR) >> CHAL_GPIO_GPCTR_ITR_SHIFT;

	return ((trigger_type << 1)|direction);
}

/**
*
*  @brief  gpio set pull 
*
*  @param  handle (in) Handle returned in chal_gpio_init() 
*  @param  gpio     (in) CHAL_GPIO ID 
*  @param  value  (in) pull value 
*
*  @return none 
******************************************************************************/
static inline void chal_gpio_set_pull(uint32_t gpio, uint32_t value)
{
	uint32_t rdvalue = CHAL_GPIO_CTR_REG(gpio);

	if (value & CHAL_GPIO_PULL_ENABLE)
		rdvalue |= CHAL_GPIO_GPCTR_PED;
	else
		rdvalue &= ~CHAL_GPIO_GPCTR_PED;

	if (value & CHAL_GPIO_PULL_UP)
		rdvalue |= CHAL_GPIO_GPCTR_PUD;
	else
		rdvalue &= ~CHAL_GPIO_GPCTR_PUD;

	/* Set the control register */
	CHAL_GPIO_CTR_REG(gpio) = rdvalue;	/* Read-modify-write */
}

/**
*
*  @brief  gpio get pull 
*
*  @param  handle (in) Handle returned in chal_gpio_init() 
*  @param  gpio     (in) CHAL_GPIO ID
*
*  @return pull info 
******************************************************************************/
static inline uint32_t chal_gpio_get_pull(uint32_t gpio)
{
	return ((CHAL_GPIO_CTR_REG(gpio) &(CHAL_GPIO_GPCTR_PED|CHAL_GPIO_GPCTR_PUD))>>1);
}

/**
*
*  @brief  gpio set debounce 
*
*  @param  handle (in) Handle returned in chal_gpio_init() 
*  @param  gpio     (in) CHAL_GPIO ID
*  @param  value  (in) debounce value 
*
*  @return none 
******************************************************************************/
static inline int chal_gpio_set_db(uint32_t gpio, uint32_t value)
{
	uint32_t rdvalue = CHAL_GPIO_CTR_REG(gpio) & ~CHAL_GPIO_GPCTR_DBR;

	switch (value)
	{
		case CHAL_GPIO_DB_1MS :
			rdvalue |= (CHAL_GPIO_DB_ENABLE | CHAL_GPIO_DB_1MS) << CHAL_GPIO_GPCTR_DBR_SHIFT;
			break;
		case CHAL_GPIO_DB_2MS :
			rdvalue |= (CHAL_GPIO_DB_ENABLE | CHAL_GPIO_DB_2MS) << CHAL_GPIO_GPCTR_DBR_SHIFT;
			break;
		case CHAL_GPIO_DB_4MS :
			rdvalue |= (CHAL_GPIO_DB_ENABLE | CHAL_GPIO_DB_4MS) << CHAL_GPIO_GPCTR_DBR_SHIFT;
			break;
		case CHAL_GPIO_DB_8MS :
			rdvalue |= (CHAL_GPIO_DB_ENABLE | CHAL_GPIO_DB_8MS) << CHAL_GPIO_GPCTR_DBR_SHIFT;
			break;
		case CHAL_GPIO_DB_16MS :
			rdvalue |= (CHAL_GPIO_DB_ENABLE | CHAL_GPIO_DB_16MS) << CHAL_GPIO_GPCTR_DBR_SHIFT;
			break;
		case CHAL_GPIO_DB_32MS :
			rdvalue |= (CHAL_GPIO_DB_ENABLE | CHAL_GPIO_DB_32MS) << CHAL_GPIO_GPCTR_DBR_SHIFT;
			break;
		case CHAL_GPIO_DB_64MS :
			rdvalue |= (CHAL_GPIO_DB_ENABLE | CHAL_GPIO_DB_64MS) << CHAL_GPIO_GPCTR_DBR_SHIFT;
			break;
		case CHAL_GPIO_DB_128MS :
			rdvalue |= (CHAL_GPIO_DB_ENABLE | CHAL_GPIO_DB_128MS) << CHAL_GPIO_GPCTR_DBR_SHIFT;
			break;
		case CHAL_GPIO_DB_NONE :
			rdvalue &= (~CHAL_GPIO_GPCTR_DBR);
			break;
		default :
			return -1;
	}

	/* Set the control register */
	CHAL_GPIO_CTR_REG(gpio) = rdvalue;	/* Read-modify-write */

	return 0;
}


/**
*
*  @brief  gpio get debounce 
*
*  @param  handle (in) Handle returned in chal_gpio_init() 
*  @param  gpio     (in) CHAL_GPIO ID 
*
*  @return debounce value 
******************************************************************************/
static inline uint32_t chal_gpio_get_db(uint32_t gpio)
{
	uint32_t value = ((CHAL_GPIO_CTR_REG(gpio) & CHAL_GPIO_GPCTR_DBR)>> CHAL_GPIO_GPCTR_DBR_SHIFT);

	if( value & CHAL_GPIO_DB_ENABLE) 
		value = ((value & ~CHAL_GPIO_DB_ENABLE)<<1) | 0x01;
	else 
		value = value<<1;

	return value;
}


/**
*
*  @brief  gpio set interrupt mask 
*
*  @param  handle 	(in) Handle returned in chal_gpio_init() 
*  @param  gpio  (in) CHAL_GPIO ID 
*
*  @return none 
******************************************************************************/
static inline void chal_gpio_set_irqmask(uint32_t gpio, uint32_t value)
{
	/* 
	 * IMR is a R/W register and documentation doesn't say if this is atomic. 
	 * IMRC is a W/O Register and doesn't need a read and is atomic.
	 */
	if (value) 
	{
		/* FIXME not sure if this is atomic */
		CHAL_GPIO_IMR_REG(gpio) |= (1 << CHAL_GPIO_BIT_OFFSET(gpio));
	}
	else
	{
		/* Atomic write */
		CHAL_GPIO_IMRC_REG(gpio) = (1 << CHAL_GPIO_BIT_OFFSET(gpio));
	}
	
}

/**
*
*  @brief  gpio get interrupt mask 
*
*  @param  handle  	(in) Handle returned in chal_gpio_init() 
*  @param  gpio   (in) CHAL_GPIO ID 
*
*  @return interrupt mask  
******************************************************************************/
static inline uint32_t chal_gpio_get_irqmask(uint32_t gpio)
{
	return((CHAL_GPIO_IMR_REG(gpio) >> CHAL_GPIO_BIT_OFFSET(gpio)) & 1);
}

/**
*
*  @brief  gpio clear interrupt mask 
*
*  @param  handle  	(in) Handle returned in chal_gpio_init() 
*  @param  gpio   (in) CHAL_GPIO ID 
*
*  @return none  
******************************************************************************/
static inline void chal_gpio_clear_irqmask(uint32_t gpio)
{
	/* Atomic write */
	CHAL_GPIO_IMRC_REG(gpio) = (1 << CHAL_GPIO_BIT_OFFSET(gpio));
}

/**
*
*  @brief  gpio get intrrupt status 
*
*  @param  handle 	(in) Handle returned in chal_gpio_init() 
*  @param  gpio   (in) CHAL_GPIO ID 
*
*  @return status 
******************************************************************************/
static inline uint32_t chal_gpio_get_irqstatus(uint32_t gpio)
{
	return ((CHAL_GPIO_ISR_REG(gpio) >> CHAL_GPIO_BIT_OFFSET(gpio)) & 1);
}

/**
*
*  @brief  gpio get intrrupt status 
*
*  @param  handle 	(in) Handle returned in chal_gpio_init() 
*  @param  gpio   (in) CHAL_GPIO ID 
*
*  @return status 
******************************************************************************/
static inline uint32_t chal_gpio_get_irqstatus_register(uint32_t gpio)
{
	/* Atomic write to clear int, read valid for statuses only */
	return CHAL_GPIO_ISR_REG(gpio);
}

/**
*
*  @brief  gpio clear int status 
*
*  @param  handle 	(in) Handle returned in chal_gpio_init() 
*  @param  gpio  (in) CHAL_GPIO ID 
*
*  @return none 
******************************************************************************/
static inline void chal_gpio_clear_int(uint32_t gpio)
{
	/* Atomic write to clear int, read valid for statuses only */
	CHAL_GPIO_ISR_REG(gpio) = (1 << CHAL_GPIO_BIT_OFFSET(gpio));
}

/**
*
*  @brief  gpio clear int status 
*
*  @param  handle 	(in) Handle returned in chal_gpio_init() 
*  @param  gpio  (in) CHAL_GPIO ID 
*
*  @return none 
******************************************************************************/
static inline void chal_gpio_clear_int_register(uint32_t gpio)
{
	/* Atomic write, read valid for statuses only */
	CHAL_GPIO_ISR_REG(gpio) = ~0;
}

/**
*
*  @brief  gpio disable irq 
*
*  @param  handle (in) Handle returned in chal_gpio_init() 
*  @param  gpio     (in) CHAL_GPIO ID 
*
*  @return none 
******************************************************************************/
static inline void chal_gpio_disable_int(uint32_t gpio)
{
	CHAL_GPIO_CTR_REG(gpio) &= ~CHAL_GPIO_GPCTR_ITR;  /* Read-modify-write */
}

/**
*
*  @brief  chal_gpio_set_mode
*
*  @param  handle (in) Handle returned in chal_gpio_init() 
*  @param  gpio     (in) CHAL_GPIO ID
*
*  @return none 
******************************************************************************/
static inline void chal_gpio_set_mode(uint32_t gpio, uint32_t mode)
{
	uint32_t direction = mode & 0x01;
	uint32_t trigger_type = (mode >> 1) & 3;
	uint32_t val = CHAL_GPIO_CTR_REG(gpio);

	/* Setup pin direction */

	val &= ~CHAL_GPIO_GPCTR_IOTR;

	if (direction == CHAL_GPIO_DIR_INPUT) 
	{
		val |= (CHAL_GPIO_DIR_INPUT << CHAL_GPIO_GPCTR_IOTR_SHIFT);
	}
	else
	{
		val |= (CHAL_GPIO_DIR_OUTPUT << CHAL_GPIO_GPCTR_IOTR_SHIFT);
	}

	/* Setup pin pull type */

	val &= ~CHAL_GPIO_GPCTR_ITR;
	switch (trigger_type)
	{
		case CHAL_GPIO_INT_DISABLE :
			val |= (CHAL_GPIO_INT_DISABLE << CHAL_GPIO_GPCTR_ITR_SHIFT);
			break;
		case CHAL_GPIO_INT_RISING :
			val |= (CHAL_GPIO_INT_RISING << CHAL_GPIO_GPCTR_ITR_SHIFT);
			break;
		case CHAL_GPIO_INT_FALLING :
			val |= (CHAL_GPIO_INT_FALLING << CHAL_GPIO_GPCTR_ITR_SHIFT);
			break;
		case CHAL_GPIO_INT_BOTH :
			val |= (CHAL_GPIO_INT_BOTH << CHAL_GPIO_GPCTR_ITR_SHIFT);
			break;
	}

	/* Set the control register */
	CHAL_GPIO_CTR_REG(gpio) = val;	/* Read-modify-write */
}


/** @} */

#ifdef __cplusplus
}
#endif

#endif /* CHAL_GPIO_INLINE_H__ */
