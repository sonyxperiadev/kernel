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


/****************************************************************************/
/**
*  @file    reg.h
*
*  @brief   Generic register defintions used in CSP
*/
/****************************************************************************/

#ifndef CSP_REG_H
#define CSP_REG_H

/* ---- Include Files ---------------------------------------------------- */

#include <mach/csp/chal_types.h>

/* ---- Public Constants and Types --------------------------------------- */

#define __REG32(x)      (*((volatile uint32_t *)(x)))
#define __REG16(x)      (*((volatile uint16_t *)(x)))
#define __REG8(x)       (*((volatile uint8_t *) (x)))


 /*  Macros used to define a sequence of reserved registers. The start / end
  *  are byte offsets in the particular register definition, with the "end"
  *  being the offset of the next un-reserved register. E.g. if offsets
  *  0x10 through to 0x1f are reserved, then this reserved area could be
  *  specified as follows.
  */
/*  typedef struct
 *  {
 *      uint32_t reg1;          // offset 0x00
 *      uint32_t reg2;          // offset 0x04
 *      uint32_t reg3;          // offset 0x08
 *      uint32_t reg4;          // offset 0x0c
 *      REG32_RSVD(0x10, 0x20);
 *      uint32_t reg5;          // offset 0x20
 *      ...
 *} EXAMPLE_REG_t;
 */
#define REG8_RSVD(start, end)   uint8_t rsvd_##start[(end - start) / sizeof(uint8_t)]
#define REG16_RSVD(start, end)  uint16_t rsvd_##start[(end - start) / sizeof(uint16_t)]
#define REG32_RSVD(start, end)  uint32_t rsvd_##start[(end - start) / sizeof(uint32_t)]

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
#endif /* CSP_REG_H */

