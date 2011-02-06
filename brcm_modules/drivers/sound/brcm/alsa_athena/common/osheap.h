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

#ifndef _OSHEAP_H_
#define _OSHEAP_H_

#include <linux/kernel.h>
#include <linux/slab.h>
#include "osdal_os.h"
/* Heap memory */
#define OSHEAP_Alloc(s)  OSDAL_ALLOCHEAPMEM(s)

#define OSHEAP_Delete(a) OSDAL_FREEHEAPMEM(a)

#endif /*_OSDAL_OS_H_*/

