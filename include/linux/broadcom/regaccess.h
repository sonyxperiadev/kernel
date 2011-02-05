/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	include/linux/broadcom/regaccess.h
*
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

/****************************************************************************
*
*  regaccess.h
*
*  PURPOSE:
*
*     This file contains code to control shared access to hardware registers.
*
*  NOTES:
*
****************************************************************************/

#if !defined(__ASM_ARCH_REGACCESS_H)
#define __ASM_ARCH_REGACCESS_H

/* ---- Include Files ---------------------------------------------------- */

#if defined(__KERNEL__)

#include <linux/broadcom/hw.h>

/* ---- Constants and Types ---------------------------------------------- */
HW_EXTERN_SPINLOCK(Arch)

static inline void reg32_or_bits(volatile u32 *reg, u32 bits)
{
	unsigned long flags;

	HW_IRQ_SAVE(Arch, flags);
	*reg |= bits;
	HW_IRQ_RESTORE(Arch, flags);
}

static inline void reg32_and_bits(volatile u32 *reg, u32 bits)
{
	unsigned long flags;

	HW_IRQ_SAVE(Arch, flags);
	*reg &= bits;
	HW_IRQ_RESTORE(Arch, flags);
}

static inline void reg16_or_bits(volatile u16 *reg, u16 bits)
{
	unsigned long flags;

	HW_IRQ_SAVE(Arch, flags);
	*reg |= bits;
	HW_IRQ_RESTORE(Arch, flags);
}

static inline void reg16_and_bits(volatile u16 *reg, u16 bits)
{
	unsigned long flags;

	HW_IRQ_SAVE(Arch, flags);
	*reg &= bits;
	HW_IRQ_RESTORE(Arch, flags);
}

static inline void reg8_or_bits(volatile u8 *reg, u8 bits)
{
	unsigned long flags;

	HW_IRQ_SAVE(Arch, flags);
	*reg |= bits;
	HW_IRQ_RESTORE(Arch, flags);
}

static inline void reg8_and_bits(volatile u8 *reg, u8 bits)
{
	unsigned long flags;

	HW_IRQ_SAVE(Arch, flags);
	*reg &= bits;
	HW_IRQ_RESTORE(Arch, flags);
}

#define regaccess_or_bits(reg, bits) (\
   (sizeof(*reg) == sizeof(u32)) ? \
   reg32_or_bits((volatile u32 *)reg, (u32)bits) : \
   ((sizeof(*reg) == sizeof(u16)) ? \
   reg16_or_bits((volatile u16 *)reg, (u16)bits) : \
   reg8_or_bits((volatile u8 *)reg, (u8)bits)))

#define regaccess_and_bits(reg, bits) (\
   (sizeof(*reg) == sizeof(u32)) ? \
   reg32_and_bits((volatile u32 *)reg, (u32)bits) : \
   ((sizeof(*reg) == sizeof(u16)) ? \
   reg16_and_bits((volatile u16 *)reg, (u16)bits) : \
   reg8_and_bits((volatile u8 *)reg, (u8)bits)))

#endif /* __KERNEL__ */
#endif /* __ASM_ARCH_REGACESS_H */
