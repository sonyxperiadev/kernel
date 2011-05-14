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

#ifndef REG_UTILS
#define REG_UTILS

/* ---- Include Files ---------------------------------------------------- */

#include <linux/types.h>

/* ---- Public Constants and Types --------------------------------------- */

#define __REG32(x)      (*((volatile uint32_t *)(x)))
#define __REG16(x)      (*((volatile uint16_t *)(x)))
#define __REG8(x)       (*((volatile uint8_t *) (x)))

/* ---- Public Variable Externs ------------------------------------------ */
/* ---- Public Function Prototypes --------------------------------------- */

/****************************************************************************/
/*
 *   32-bit register access functions
 */
/****************************************************************************/

static inline void reg32_clear_bits(volatile uint32_t *reg, uint32_t value)
{
    *reg &= ~(value);
}

static inline void reg32_set_bits(volatile uint32_t *reg, uint32_t value)
{
    *reg |= value;
}

static inline void reg32_toggle_bits(volatile uint32_t *reg, uint32_t value)
{
    *reg ^= value;
}

static inline void reg32_write_masked(volatile uint32_t *reg, uint32_t mask, uint32_t value)
{
    *reg = (*reg & (~mask)) | (value & mask);
}

static inline void reg32_write(volatile uint32_t *reg, uint32_t value)
{
    *reg = value;
}

static inline uint32_t reg32_read(volatile uint32_t *reg)
{
    return *reg;
}

/****************************************************************************/
/*
 *   16-bit register access functions
 */
/****************************************************************************/

static inline void reg16_clear_bits(volatile uint16_t *reg, uint16_t value)
{
    *reg &= ~(value);
}

static inline void reg16_set_bits(volatile uint16_t *reg, uint16_t value)
{
    *reg |= value;
}

static inline void reg16_toggle_bits(volatile uint16_t *reg, uint16_t value)
{
    *reg ^= value;
}

static inline void reg16_write_masked(volatile uint16_t *reg, uint16_t mask, uint16_t value)
{
    *reg = (*reg & (~mask)) | (value & mask);
}

static inline void reg16_write(volatile uint16_t *reg, uint16_t value)
{
    *reg = value;
}

static inline uint16_t reg16_read(volatile uint16_t *reg)
{
    return *reg;
}

/****************************************************************************/
/*
 *   8-bit register access functions
 */
/****************************************************************************/

static inline void reg8_clear_bits(volatile uint8_t *reg, uint8_t value)
{
    *reg &= ~(value);
}

static inline void reg8_set_bits(volatile uint8_t *reg, uint8_t value)
{
    *reg |= value;
}

static inline void reg8_toggle_bits(volatile uint8_t *reg, uint8_t value)
{
    *reg ^= value;
}

static inline void reg8_write_masked(volatile uint8_t *reg, uint8_t mask, uint8_t value)
{
    *reg = (*reg & (~mask)) | (value & mask);
}

static inline void reg8_write(volatile uint8_t *reg, uint8_t value)
{
    *reg = value;
}

static inline uint8_t reg8_read(volatile uint8_t *reg)
{
    return *reg;
}
#endif /* REG_UTILS */

