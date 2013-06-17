/*********************************************************************
*
*  Copyright 2010 Broadcom Corporation
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed
*  to you under the terms of the GNU
*  General Public License version 2 (the GPL), available at
*  http://www.broadcom.com/licenses/GPLv2.php with the following added
*  to such license:
*  As a special exception, the copyright holders of this software give
*  you permission to link this software with independent modules, and
*  to copy and distribute the resulting executable under terms of your
*  choice, provided that you also meet, for each linked independent module,
*  the terms and conditions of the license of that module. An independent
*  module is a module which is not derived from this software.  The special
*  exception does not apply to any modifications of the software.
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
************************************************************************/
#ifndef __NS_IOREMAP_H_
#define __NS_IOREMAP_H_

#define get_vaddr_frm_vmstruct(vms) (vms->addr)

extern struct vm_struct	*plat_get_vm_area(int pages);
extern void plat_free_vm_area(struct vm_struct *vmas);
extern void __iomem *plat_ioremap_ns(unsigned long vaddr, unsigned long size,
		phys_addr_t phys_addr);
extern void plat_iounmap_ns(void __iomem *vaddr, unsigned long size);

#endif /* __NS_IOREMAP_H_ */
