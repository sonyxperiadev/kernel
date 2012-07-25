#include <linux/kernel.h>

#include <asm/cputype.h>
#include <asm/idmap.h>
#include <asm/pgalloc.h>
#include <asm/pgtable.h>
#include <asm/sections.h>
#include <asm/system_info.h>

pgd_t *idmap_pgd;

#ifdef CONFIG_ARM_LPAE
static void idmap_add_pmd(pud_t *pud, unsigned long addr, unsigned long end,
	unsigned long prot)
{
	pmd_t *pmd;
	unsigned long next;

	if (pud_none_or_clear_bad(pud) || (pud_val(*pud) & L_PGD_SWAPPER)) {
		pmd = pmd_alloc_one(&init_mm, addr);
		if (!pmd) {
			pr_warning("Failed to allocate identity pmd.\n");
			return;
		}
		pud_populate(&init_mm, pud, pmd);
		pmd += pmd_index(addr);
	} else
		pmd = pmd_offset(pud, addr);

	do {
		next = pmd_addr_end(addr, end);
		*pmd = __pmd((addr & PMD_MASK) | prot);
		flush_pmd_entry(pmd);
	} while (pmd++, addr = next, addr != end);
}
#else	/* !CONFIG_ARM_LPAE */
static void idmap_add_pmd(pud_t *pud, unsigned long addr, unsigned long end,
	unsigned long prot)
{
	pmd_t *pmd = pmd_offset(pud, addr);

	addr = (addr & PMD_MASK) | prot;
	pmd[0] = __pmd(addr);
	addr += SECTION_SIZE;
	pmd[1] = __pmd(addr);
	flush_pmd_entry(pmd);
}
#endif	/* CONFIG_ARM_LPAE */

static void idmap_add_pud(pgd_t *pgd, unsigned long addr, unsigned long end,
	unsigned long prot)
{
	pud_t *pud = pud_offset(pgd, addr);
	unsigned long next;

	do {
		next = pud_addr_end(addr, end);
		idmap_add_pmd(pud, addr, next, prot);
	} while (pud++, addr = next, addr != end);
}

static void identity_mapping_add(pgd_t *pgd, unsigned long addr, unsigned long end)
{
	unsigned long prot, next;

	prot = PMD_TYPE_SECT | PMD_SECT_AP_WRITE | PMD_SECT_AF;
	if (cpu_architecture() <= CPU_ARCH_ARMv5TEJ && !cpu_is_xscale())
		prot |= PMD_BIT4;

	pgd += pgd_index(addr);
	do {
		next = pgd_addr_end(addr, end);
		idmap_add_pud(pgd, addr, next, prot);
	} while (pgd++, addr = next, addr != end);
}

extern char  __idmap_text_start[], __idmap_text_end[];

static int __init init_static_idmap(void)
{
	phys_addr_t idmap_start, idmap_end;

	idmap_pgd = pgd_alloc(&init_mm);
	if (!idmap_pgd)
		return -ENOMEM;

	/* Add an identity mapping for the physical address of the section. */
	idmap_start = virt_to_phys((void *)__idmap_text_start);
	idmap_end = virt_to_phys((void *)__idmap_text_end);

	pr_info("Setting up static identity map for 0x%llx - 0x%llx\n",
		(long long)idmap_start, (long long)idmap_end);
	identity_mapping_add(idmap_pgd, idmap_start, idmap_end);

	return 0;
}
#else
void identity_mapping_del(pgd_t *pgd, unsigned long addr, unsigned long end)
{
}
#endif

/*
 * In order to soft-boot, we need to insert a 1:1 mapping of memory.
 * This will then ensure that we have predictable results when turning
 * the mmu off.
 */
void setup_mm_for_reboot(char mode, pgd_t *pgd)
{
	unsigned long kernel_end;

	/* If we don't have a pgd, hijack the current task. */
	if (pgd == NULL) {
		pgd = current->active_mm->pgd;
		identity_mapping_add(pgd, 0, TASK_SIZE);
	} else {
		identity_mapping_add(pgd, 0, TASK_SIZE);
		/*
		 * Extend the flat mapping into kernelspace.
		 * We leave room for the kernel image and the reserved
		 * page below swapper.
		 */
		kernel_end = ALIGN((unsigned long)_end, PMD_SIZE);
		identity_mapping_add(pgd, kernel_end, 0);
	}

	/* Clean and invalidate L1. */
	flush_cache_all();

	/* Switch exclusively to kernel mappings. */
	cpu_switch_mm(pgd, &init_mm);

	/* Flush the TLB. */
	local_flush_tlb_all();
}
