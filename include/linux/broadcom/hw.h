/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	include/linux/broadcom/hw.h
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

#ifndef HW_H
#define HW_H

/* Macros to make managing spinlocks a bit more controlled in terms of naming. */
#if defined(__KERNEL__)

#include <linux/spinlock.h>
#include <asm/types.h>

#define HW_DECLARE_SPINLOCK(name)  DEFINE_SPINLOCK(g##name##RegLock);
#define HW_EXTERN_SPINLOCK(name)   extern spinlock_t g##name##RegLock;
#define HW_IRQ_SAVE(name, val)     spin_lock_irqsave(&g##name##RegLock, (val))
#define HW_IRQ_RESTORE(name, val)  spin_unlock_irqrestore(&g##name##RegLock, (val))

#define __REG32(x)   (*((volatile u32 *)(x)))
#define __REG16(x)   (*((volatile u16 *)(x)))
#define __REG8(x)    (*((volatile u8  *)(x)))

#else

#define HW_DECLARE_SPINLOCK(name)  {(void)(name); }
#define HW_EXTERN_SPINLOCK(name)   {(void)(name); }
#define HW_IRQ_SAVE(name, val)     {(void)(name); (void)(val); }
#define HW_IRQ_RESTORE(name, val)  {(void)(name); (void)(val); }

#endif

#endif /* HW_SPINLOCK_H */
