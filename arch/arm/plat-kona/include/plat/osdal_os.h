/*******************************************************************************
 * * Copyright 2010 Broadcom Corporation.  All rights reserved.
 * *
 * *       @file   arch/arm/plat-bcmap/include/plat/osdal_os.h
 * *
 * * Unless you and Broadcom execute a separate written software license agreement
 * * governing use of this software, this software is licensed to you under the
 * * terms of the GNU General Public License version 2, available at
 * * http://www.gnu.org/copyleft/gpl.html (the "GPL").
 * *
 * * Notwithstanding the above, under no circumstances may you combine this
 * * software in any way with any other Broadcom software provided under a license
 * * other than the GPL, without Broadcom's express prior written consent.
 * *******************************************************************************/

/**
*
*  @file   osdal_os.h
*
*  @brief  OS Driver Abstraction Layer API
*          This header offers macro-based abstraction. One per OS.
*          This is Linux implementation.
*
*
*  @note   This file is to help OS independent CSL implementation
****************************************************************************/

#ifndef _OSDAL_OS_H_
#define _OSDAL_OS_H_

#include <linux/kernel.h>
#include "asm/posix_types.h"
#include "asm/string.h"
/* Heap memory */
#define OSDAL_ALLOCHEAPMEM(s)	kzalloc((s), GFP_KERNEL)

#define OSDAL_FREEHEAPMEM(a)	kfree((a))


/* IRQ */
#define OSDAL_IRQ_Enable(irq)    enable_irq((irq))
#define OSDAL_IRQ_Disable(irq)   disable_irq((irq))
#define OSDAL_IRQ_Clear(irq)
#define OSDAL_IRQ_IsEnabled(irq)


/*  Synchronization */
#define OSDAL_SENDEVENT(e) complete((struct completion *)&(e))

#define OSDAL_WAITEVENT(e)	\
wait_for_completion((struct completion *)&(e))

#define OSDAL_WAITEVENT_TIMEOUT(e, t) \
wait_for_completion_timeout((struct completion *)&(e), (t))

#define OSDAL_CLEAREVENT(e)


/* Time stamp in ms */
#define  OSDAL_TIMEVAL()


/* Delays */
#define OSDAL_MDELAY(x)	mdelay(x)

#define OSDAL_UDELAY(x)	udelay(x)

/* Debug Print */
#define dprintf(prio, format, args...)	pr_debug("%s:%s"" format", __FILE__, __FUNCTION__)

#endif /*_OSDAL_OS_H_*/

