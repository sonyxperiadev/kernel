/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*	@file	include/linux/broadcom/bcm_memalloc_ioctl.h
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

#ifndef _MEMALLOC_IOCTL_H_
#define _MEMALLOC_IOCTL_H_
#include <linux/ioctl.h>	/* needed for the _IOW etc stuff used later */

/*
 * Ioctl definitions
 */
#define HANTRO_WRAP_MAGIC  'B'
#define HANTRO_WRAP_ACQUIRE_BUFFER _IOWR(HANTRO_WRAP_MAGIC,  1, unsigned long)
#define HANTRO_WRAP_RELEASE_BUFFER _IOW(HANTRO_WRAP_MAGIC,  2, unsigned long)

#define HANTRO_WRAP_MAXNR 15

typedef struct {
	unsigned long busAddress;
	unsigned int size;
} MemallocwrapParams;

#endif /* _MEMALLOC_IOCTL_H_ */
