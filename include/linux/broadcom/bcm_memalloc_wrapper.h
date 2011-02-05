/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*	@file	include/linux/broadcom/bcm_memalloc_wrapper.h
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

#ifndef _MEMALLOC_WRAPPER_H_
#define _MEMALLOC_WRAPPER_H_

typedef void *MEMALLOC_HDL;

struct memalloc_logic {
	int (*AllocMemory)(MEMALLOC_HDL hdl, unsigned long *busaddr,
			unsigned int size);
	int (*FreeMemory)(MEMALLOC_HDL hdl, unsigned long *busaddr);
	int (*open)(MEMALLOC_HDL *hdlp);
	int (*release)(MEMALLOC_HDL hdl);
	int (*mmap)(unsigned long size, unsigned long pgoff);
	int (*init)(unsigned int memory_size, unsigned int phy_start_address);
	int (*cleanup)(void);
};

int register_memalloc_wrapper(struct memalloc_logic *logic);
void deregister_memalloc_wrapper(void);

#endif /* _MEMALLOC_WRAPPER_H_ */
