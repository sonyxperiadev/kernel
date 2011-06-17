/*****************************************************************************
*  Copyright 2003 - 2007 Broadcom Corporation.  All rights reserved.
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2, available at
*  http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
*****************************************************************************/

#ifndef _BCM_FUSE_LNX_IF_H_
#define _BCM_FUSE_LNX_IF_H_

#include <linux/interrupt.h>

void bcm_lnx_init(struct tasklet_struct *tsklet_ptr);

uint32_t bcm_get_shmem_address_offset(void *virt_addr);

unsigned int bcm_map_virt_to_phys(void *virt_addr);

void *bcm_map_phys_to_virt(unsigned int virt_addr);

void bcm_raise_cp_int(void);

void bcm_enable_reentrancy(void);

void bcm_disable_reentrancy(void);

#endif	//_BCM_FUSE_LNX_IF_H_


