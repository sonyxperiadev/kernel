/*
 * Additional userspace pages setup for AArch64 and AArch32.
 *  - AArch64: vDSO pages setup, vDSO data page update.
 *  - AArch32: sigreturn and kuser helpers pages setup.
 *
 * Copyright (C) 2012 ARM Limited
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Will Deacon <will.deacon@arm.com>
 */

#include <linux/cache.h>
#include <linux/clocksource.h>
#include <linux/elf.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/gfp.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/signal.h>
#include <linux/slab.h>
#include <linux/timekeeper_internal.h>
#include <linux/vmalloc.h>

#include <asm/cacheflush.h>
#include <asm/signal32.h>
#include <asm/vdso.h>
#include <asm/vdso_datapage.h>

struct vdso_mappings {
	unsigned long num_code_pages;
	struct vm_special_mapping data_mapping;
	struct vm_special_mapping code_mapping;
};

/*
 * The vDSO data page.
 */
static union {
	struct vdso_data	data;
	u8			page[PAGE_SIZE];
} vdso_data_store __page_aligned_data;
struct vdso_data *vdso_data = &vdso_data_store.data;

#ifdef CONFIG_COMPAT
/*
 * Create and map the vectors page for AArch32 tasks.
 */
#if !defined(CONFIG_VDSO32) || defined(CONFIG_KUSER_HELPERS)
static struct page *vectors_page[] __ro_after_init;
static const struct vm_special_mapping compat_vdso_spec[] = {
	{
		/* Must be named [sigpage] for compatibility with arm. */
		.name	= "[sigpage]",
		.pages	= &vectors_page[0],
	},
#ifdef CONFIG_KUSER_HELPERS
	{
		.name	= "[kuserhelpers]",
		.pages	= &vectors_page[1],
	},
#endif
};
static struct page *vectors_page[ARRAY_SIZE(compat_vdso_spec)] __ro_after_init;
#endif

static int __init alloc_vectors_page(void)
{
#ifdef CONFIG_KUSER_HELPERS
	extern char __kuser_helper_start[], __kuser_helper_end[];
	size_t kuser_sz = __kuser_helper_end - __kuser_helper_start;
	unsigned long kuser_vpage;
#endif

#ifndef CONFIG_VDSO32
	extern char __aarch32_sigret_code_start[], __aarch32_sigret_code_end[];
	size_t sigret_sz =
		__aarch32_sigret_code_end - __aarch32_sigret_code_start;
	unsigned long sigret_vpage;

	sigret_vpage = get_zeroed_page(GFP_ATOMIC);
	if (!sigret_vpage)
		return -ENOMEM;
#endif

#ifdef CONFIG_KUSER_HELPERS
	kuser_vpage = get_zeroed_page(GFP_ATOMIC);
	if (!kuser_vpage) {
#ifndef CONFIG_VDSO32
		free_page(sigret_vpage);
#endif
		return -ENOMEM;
	}
#endif

#ifndef CONFIG_VDSO32
	/* sigreturn code */
	memcpy((void *)sigret_vpage, __aarch32_sigret_code_start, sigret_sz);
	flush_icache_range(sigret_vpage, sigret_vpage + PAGE_SIZE);
	vectors_page[0] = virt_to_page(sigret_vpage);
#endif

#ifdef CONFIG_KUSER_HELPERS
	/* kuser helpers */
	memcpy((void *)kuser_vpage + 0x1000 - kuser_sz, __kuser_helper_start,
		kuser_sz);
	flush_icache_range(kuser_vpage, kuser_vpage + PAGE_SIZE);
	vectors_page[1] = virt_to_page(kuser_vpage);
#endif

	return 0;
}
arch_initcall(alloc_vectors_page);

#ifndef CONFIG_VDSO32
int aarch32_setup_vectors_page(struct linux_binprm *bprm, int uses_interp)
{
	struct mm_struct *mm = current->mm;
	unsigned long addr;
	void *ret;

	if (down_write_killable(&mm->mmap_sem))
		return -EINTR;
	addr = get_unmapped_area(NULL, 0, PAGE_SIZE, 0, 0);
	if (IS_ERR_VALUE(addr)) {
		ret = ERR_PTR(addr);
		goto out;
	}

	ret = _install_special_mapping(mm, addr, PAGE_SIZE,
				       VM_READ|VM_EXEC|
				       VM_MAYREAD|VM_MAYWRITE|VM_MAYEXEC,
				       &compat_vdso_spec[0]);
	if (IS_ERR(ret))
		goto out;

	current->mm->context.vdso = (void *)addr;

#ifdef CONFIG_KUSER_HELPERS
	/* Map the kuser helpers at the ABI-defined high address. */
	ret = _install_special_mapping(mm, AARCH32_KUSER_HELPERS_BASE,
				       PAGE_SIZE,
				       VM_READ|VM_EXEC|VM_MAYREAD|VM_MAYEXEC,
				       &compat_vdso_spec[1]);
#endif
out:
	up_write(&mm->mmap_sem);

	return PTR_ERR_OR_ZERO(ret);
}
#endif /* !CONFIG_VDSO32 */
#endif /* CONFIG_COMPAT */

static int vdso_mremap(const struct vm_special_mapping *sm,
		struct vm_area_struct *new_vma)
{
	unsigned long new_size = new_vma->vm_end - new_vma->vm_start;
	unsigned long vdso_size = vdso_end - vdso_start;

	if (vdso_size != new_size)
		return -EINVAL;

	current->mm->context.vdso = (void *)new_vma->vm_start;

	return 0;
}

static int __init vdso_mappings_init(const char *name,
				     const char *code_start,
				     const char *code_end,
				     struct vdso_mappings *mappings)
{
	unsigned long i, vdso_page;
	struct page **vdso_pagelist;
	unsigned long pfn;

	if (memcmp(code_start, "\177ELF", 4)) {
		pr_err("%s is not a valid ELF object!\n", name);
		return -EINVAL;
	}

	vdso_pages = (code_end - code_start) >> PAGE_SHIFT;
	pr_info("%s: %ld pages (%ld code @ %p, %ld data @ %p)\n",
		name, vdso_pages + 1, vdso_pages, code_start, 1L,
		vdso_data);

	/*
	 * Allocate space for storing pointers to the vDSO code pages + the
	 * data page. The pointers must have the same lifetime as the mappings,
	 * which are static, so there is no need to keep track of the pointer
	 * array to free it.
	 */
	vdso_pagelist = kmalloc_array(vdso_pages + 1, sizeof(struct page *),
				      GFP_KERNEL);
	if (vdso_pagelist == NULL)
		return -ENOMEM;

	/* Grab the vDSO data page. */
	vdso_pagelist[0] = phys_to_page(__pa_symbol(vdso_data));

	/* Grab the vDSO code pages. */
	pfn = sym_to_pfn(code_start);

	for (i = 0; i < vdso_pages; i++)
		vdso_pagelist[i + 1] = pfn_to_page(pfn + i);

	/* Populate the special mapping structures */
	mappings->data_mapping = (struct vm_special_mapping) {
		.name	= "[vvar]",
		.pages	= &vdso_pagelist[0],
	};

	mappings->code_mapping = (struct vm_special_mapping) {
		.name	= "[vdso]",
		.pages	= &vdso_pagelist[1],
	};

	mappings->num_code_pages = vdso_pages;
	return 0;
}

#ifdef CONFIG_COMPAT
#ifdef CONFIG_VDSO32

static struct vdso_mappings vdso32_mappings __ro_after_init;

static int __init vdso32_init(void)
{
	extern char vdso32_start[], vdso32_end[];

	return vdso_mappings_init("vdso32", vdso32_start, vdso32_end,
				  &vdso32_mappings);
}
arch_initcall(vdso32_init);

#endif /* CONFIG_VDSO32 */
#endif /* CONFIG_COMPAT */

static struct vdso_mappings vdso_mappings __ro_after_init;

static int __init vdso_init(void)
{
	extern char vdso_start[], vdso_end[];

	return vdso_mappings_init("vdso", vdso_start, vdso_end,
				  &vdso_mappings);
}
arch_initcall(vdso_init);

static int vdso_setup(struct mm_struct *mm,
		      const struct vdso_mappings *mappings)
{
	unsigned long vdso_base, vdso_text_len, vdso_mapping_len;
	void *ret;

	vdso_text_len = mappings->num_code_pages << PAGE_SHIFT;
	/* Be sure to map the data page */
	vdso_mapping_len = vdso_text_len + PAGE_SIZE;

	vdso_base = get_unmapped_area(NULL, 0, vdso_mapping_len, 0, 0);
	if (IS_ERR_VALUE(vdso_base))
		ret = PTR_ERR_OR_ZERO(ERR_PTR(vdso_base));

	ret = _install_special_mapping(mm, vdso_base, PAGE_SIZE,
				       VM_READ|VM_MAYREAD,
				       &mappings->data_mapping);
	if (IS_ERR(ret))
		return PTR_ERR_OR_ZERO(ret);

	vdso_base += PAGE_SIZE;
	ret = _install_special_mapping(mm, vdso_base, vdso_text_len,
				       VM_READ|VM_EXEC|
				       VM_MAYREAD|VM_MAYWRITE|VM_MAYEXEC,
				       &mappings->code_mapping);
	if (!IS_ERR(ret))
		mm->context.vdso = (void *)vdso_base;

	return PTR_ERR_OR_ZERO(ret);
}

#ifdef CONFIG_COMPAT
#ifdef CONFIG_VDSO32
int aarch32_setup_vectors_page(struct linux_binprm *bprm, int uses_interp)
{
	struct mm_struct *mm = current->mm;
	void *ret;

	if (down_write_killable(&mm->mmap_sem))
		return -EINTR;

	ret = ERR_PTR(vdso_setup(mm, &vdso32_mappings));
#ifdef CONFIG_KUSER_HELPERS
	if (!IS_ERR(ret))
		/* Map the kuser helpers at the ABI-defined high address. */
		ret = _install_special_mapping(mm, AARCH32_KUSER_HELPERS_BASE,
					       PAGE_SIZE,
					       VM_READ|VM_EXEC|
					       VM_MAYREAD|VM_MAYEXEC,
					       &compat_vdso_spec[1]);
#endif

	up_write(&mm->mmap_sem);

	return PTR_ERR_OR_ZERO(ret);
}
#endif /* CONFIG_VDSO32 */
#endif /* CONFIG_COMPAT */

int arch_setup_additional_pages(struct linux_binprm *bprm, int uses_interp)
{
	struct mm_struct *mm = current->mm;
	int ret;

	if (down_write_killable(&mm->mmap_sem))
		return -EINTR;

	ret = vdso_setup(mm, &vdso_mappings);

	up_write(&mm->mmap_sem);
	return ret;
}

/*
 * Update the vDSO data page to keep in sync with kernel timekeeping.
 */
void update_vsyscall(struct timekeeper *tk)
{
	u32 use_syscall = !tk->tkr_mono.clock->archdata.vdso_direct;

	++vdso_data->tb_seq_count;
	smp_wmb();

	vdso_data->use_syscall			= use_syscall;
	vdso_data->xtime_coarse_sec		= tk->xtime_sec;
	vdso_data->xtime_coarse_nsec		= tk->tkr_mono.xtime_nsec >>
							tk->tkr_mono.shift;
	vdso_data->wtm_clock_sec		= tk->wall_to_monotonic.tv_sec;
	vdso_data->wtm_clock_nsec		= tk->wall_to_monotonic.tv_nsec;

	if (!use_syscall) {
		struct timespec btm = ktime_to_timespec(tk->offs_boot);

		/* tkr_mono.cycle_last == tkr_raw.cycle_last */
		vdso_data->cs_cycle_last	= tk->tkr_mono.cycle_last;
		vdso_data->raw_time_sec         = tk->raw_sec;
		vdso_data->raw_time_nsec        = tk->tkr_raw.xtime_nsec;
		vdso_data->xtime_clock_sec	= tk->xtime_sec;
		vdso_data->xtime_clock_snsec	= tk->tkr_mono.xtime_nsec;
		vdso_data->cs_mono_mult		= tk->tkr_mono.mult;
		vdso_data->cs_raw_mult		= tk->tkr_raw.mult;
		/* tkr_mono.shift == tkr_raw.shift */
		vdso_data->cs_shift		= tk->tkr_mono.shift;
		vdso_data->btm_sec		= btm.tv_sec;
		vdso_data->btm_nsec		= btm.tv_nsec;
	}

	smp_wmb();
	++vdso_data->tb_seq_count;
}

void update_vsyscall_tz(void)
{
	vdso_data->tz_minuteswest	= sys_tz.tz_minuteswest;
	vdso_data->tz_dsttime		= sys_tz.tz_dsttime;
}
