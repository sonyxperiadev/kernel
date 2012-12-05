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
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/vmalloc.h>
#include <linux/io.h>
#include <linux/mm.h>
#include <asm/tlbflush.h>
#include <asm/pgtable.h>

#define PROT_PTE_DEVICE	(L_PTE_PRESENT|L_PTE_YOUNG|L_PTE_DIRTY|L_PTE_XN)

/**
 * allocates "pages" number of pages and returns
 * the start addr of the vm area.
 */
struct vm_struct *plat_get_vm_area(int pages)
{
	struct vm_struct *vmas;

	vmas = __get_vm_area(PAGE_SIZE * pages, VM_IOREMAP,
			VMALLOC_START, VMALLOC_END);

	return vmas;
}
EXPORT_SYMBOL(plat_get_vm_area);

void plat_free_vm_area(struct vm_struct *vmas)
{
	free_vm_area(vmas);
}
EXPORT_SYMBOL(plat_free_vm_area);

/**
 * The caller has to make sure that there is enough guard
 * vm area allocated, so that the allignment adjustment done here
 * does not overflow the vm area. Unlike ioremap, this function cant
 * take care of this, as the vm area is pre-allocated
 * by calling plat_get_vm_area.
 */
void __iomem *plat_ioremap_ns(unsigned long vaddr, unsigned long size,
		phys_addr_t phys_addr)
{
	unsigned long pfn;
	unsigned long offset;

	pfn = __phys_to_pfn(phys_addr);
	offset = phys_addr & ~PAGE_MASK;

	size = PAGE_ALIGN(offset + size);

	if (ioremap_page_range(vaddr, vaddr + size, __pfn_to_phys(pfn),
		__pgprot(PROT_PTE_DEVICE | L_PTE_MT_DEV_SHARED |
			L_PTE_SHARED))) {
			pr_err("ERROR: ns_ioremap failed\n");
			return (void __iomem *)NULL;
	}

	return (void __iomem *)(vaddr + offset);
}
EXPORT_SYMBOL(plat_ioremap_ns);

void plat_iounmap_ns(void __iomem *vaddr, unsigned long size)
{
	unmap_kernel_range_noflush((unsigned long __force)vaddr, size);
	flush_tlb_kernel_range((unsigned long __force)vaddr,
			(unsigned long __force)vaddr + size);
}
EXPORT_SYMBOL(plat_iounmap_ns);
