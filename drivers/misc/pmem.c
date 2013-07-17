/* drivers/android/pmem.c
 *
 * Copyright (C) 2007 Google, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/mm.h>
#include <linux/mm_types.h>
#include <linux/oom.h>
#include <linux/notifier.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/debugfs.h>
#include <linux/android_pmem.h>
#include <linux/mempolicy.h>
#include <linux/sched.h>
#include <linux/workqueue.h>
#include <linux/rcupdate.h>
#include <linux/dma-mapping.h>
#include <linux/wait.h>
#include <linux/dma-contiguous.h>
#include <linux/genalloc.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/cacheflush.h>
#include <asm/tlbflush.h>

#define PMEM_MAX_DEVICES	(1)
#define PMEM_DEBUG		0

struct pmem_data {
	/* start_pfn for allocation */
	unsigned long pfn;
	/* size of the allocation in alloc mode */
	unsigned int size;
	/* see flags above for descriptions */
	volatile unsigned int flags;
	/* protects this data field, if the mm_mmap sem will be held at the
	 * same time as this sem, the mm sem must be taken first (as this is
	 * the order for vma_open and vma_close ops */
	struct rw_semaphore sem;
	/* info about the mmaping process */
	struct vm_area_struct *vma;
	/* task struct of the mapping process */
	struct task_struct *task;
	/* process id of teh mapping process */
	pid_t pid;
	/* a linked list of data so we can access them for debugging */
	struct list_head list;
};

struct pmem_info {
	struct miscdevice dev;
	/* Platform device for the driver, used in CMA allocations */
	struct platform_device *pdev;
	/* pfn of the garbage page in memory */
	unsigned long garbage_pfn;
	/* mutually exclusive shrinker function */
	struct mutex shrinker_lock;
	/* Work that shrinks pmem allocations */
	struct work_struct pmem_shrinker;
	/* Semaphore that protects deathpending.. */
	struct semaphore shrinker_sem;
	/* Task killed by the pmem_shrinker */
	struct task_struct *deathpending;
	long deathpending_rss;
	/* Flag set by process that failed allocation
	 * in order to force shriker to kill one process
	 * w/o checking watermarks */
	int force_kill;
	/* Processes waiting for shrinker to finish when allocation fails */
	wait_queue_head_t deatheaters;
	/* Stats for the CMA region for this device */
	struct dev_cma_info cma;
	/* protects data list */
	struct mutex data_list_lock;
	/* total size of the pmem space */
	struct list_head data_list;
	/* carved out memory for us */
	phys_addr_t carveout_base;
	phys_addr_t carveout_size;
	/* genpool to manage carved out memory */
	struct gen_pool *pool;
};

/* indicates that a refernce to this file has been taken via get_pmem_file,
 * the file should not be released until put_pmem_file is called */
#define PMEM_FLAGS_BUSY		(0x1)
/* indicates that this is a suballocation of a larger master range */
#define PMEM_FLAGS_CONNECTED	(0x1 << 1)
/* indicates this is a master and not a sub allocation and that it is mmaped */
#define PMEM_FLAGS_MASTERMAP	(0x1 << 2)

#define PMEM_FLAGS_DIRTY_REGION	(0x1 << 5)
/* Set if kernel mappings were change to match userspace mmap() */
#define PMEM_FLAGS_MMAP_CHANGED	(0x1 << 6)

/* What type of allocation ? */
#define PMEM_FLAGS_KMALLOC	(0x1 << 8)
#define PMEM_FLAGS_CMA		(0x1 << 9)
#define PMEM_FLAGS_CARVEOUT	(0x1 << 10)
#define PMEM_FLAGS_ALLOCMASK	(PMEM_FLAGS_KMALLOC | PMEM_FLAGS_CMA \
					| PMEM_FLAGS_CARVEOUT)

#define PMEM_IS_PAGE_ALIGNED(addr)	(!((addr) & (~PAGE_MASK)))
#define PMEM_START_PAGE(data)		pfn_to_page(data->pfn)
#define PMEM_START_ADDR(data)		__pfn_to_phys(data->pfn)
#define PMEM_START_VADDR(data)		phys_to_virt(PMEM_START_ADDR(data))

/* Macros to set pgprot values to match uncached, write-through
 * and write-back cache bits
 **/
#define pgprot_noncached(prot) \
	__pgprot_modify(prot, L_PTE_MT_MASK, L_PTE_MT_UNCACHED)

#define pgprot_writethrough(prot) \
	__pgprot((pgprot_val(prot) & ~L_PTE_MT_MASK) | L_PTE_MT_WRITETHROUGH)

/* Always assume writeback mappings are going to be used for
 * ACP, so they must also have their shared bit set
 **/
#define pgprot_writeback(prot) \
	__pgprot((pgprot_val(prot) & ~L_PTE_MT_MASK) | L_PTE_MT_WRITEBACK | L_PTE_SHARED)

#define PMEM_DEBUG_MSGS	0
#if PMEM_DEBUG_MSGS
#define DLOG(fmt, args...) \
	do { printk(KERN_INFO "[%s:%s:%d] "fmt, __FILE__, __func__, __LINE__, \
		    ##args); } \
	while (0)
#else
#define DLOG(x...) do {} while (0)
#endif

DECLARE_WAIT_QUEUE_HEAD(cleaners);
static struct pmem_info pmem[PMEM_MAX_DEVICES];
static int id_count;

static int get_id(struct file *file)
{
	return MINOR(file->f_dentry->d_inode->i_rdev);
}

static bool has_allocation(struct file *file)
{
	struct pmem_data *data;

	if (unlikely(!file->private_data))
		return false;
	data = (struct pmem_data *)file->private_data;
	if (unlikely(data->pfn == -1UL))
		return false;
	return true;
}

/* called from cma free to modify task cma rss only if the task
 * is the original allocator
 */
static inline bool task_is_allocator(struct pmem_data *data,
				struct task_struct *task)
{
	return (data->pid == task_pid_nr(task));
}

/* Must be called with data->sem held */
static inline bool is_cma_allocation(struct pmem_data *data)
{
	return !!(data->flags & PMEM_FLAGS_CMA);
}

static inline bool is_kmalloc_allocation(struct pmem_data *data)
{
	return !!(data->flags & PMEM_FLAGS_KMALLOC);
}

static inline bool is_carveout_allocation(struct pmem_data *data)
{
	return !!(data->flags & PMEM_FLAGS_CARVEOUT);
}

static pgprot_t pmem_access_prot(struct file *file, pgprot_t vma_prot)
{
	if (file->f_flags & O_SYNC)
		return pgprot_noncached(vma_prot);
	else if (file->f_flags & FASYNC)
		return pgprot_writethrough(vma_prot);
	else
		return pgprot_writeback(vma_prot);
}

static unsigned long pmem_len(struct pmem_data *data)
{
	return data->pfn != -1UL ? data->size : 0UL;

}

/* Taken from __dma_update_pte */
static int pmem_update_pte(pte_t *pte, pgtable_t token, unsigned long addr,
			   void *data)
{
	struct page *page = virt_to_page(addr);
	pgprot_t prot = *(pgprot_t *) data;

	set_pte_ext(pte, mk_pte(page, prot), 0);
	return 0;
}

/* Taken from __dma_remap */
static void pmem_restore_kernel_mappings(int id, struct pmem_data *data,
					 unsigned long offset,
					 unsigned long len)
{
	unsigned long start;

	start = (unsigned long)PMEM_START_VADDR(data) + offset;

	apply_to_page_range(&init_mm, start, len, pmem_update_pte,
			    &pgprot_kernel);

	dsb();
	flush_tlb_kernel_range(start, start + len);

	data->flags &= ~PMEM_FLAGS_MMAP_CHANGED;
}

/* Taken from __dma_clear_buffer + __dma_remap */
static void pmem_update_kernel_mappings(int id, struct file *file,
					struct pmem_data *data,
					unsigned long offset, unsigned long len)
{
	pgprot_t new_prot;
	unsigned long start;
	phys_addr_t phys_start;

	/* if allocated from carveout or kmalloc heap, dont change anything */
	if (data->flags & (PMEM_FLAGS_CARVEOUT | PMEM_FLAGS_KMALLOC))
		return;

	if (!pfn_valid(data->pfn)) {
		printk(KERN_EMERG"pmem: pfn(%lx) is invalid\n"
				" flags(0x%08x) process(%s/%d/%d)"
				" allocsize(%ld pages). Crashing now!!\n",
				data->pfn, data->flags,
				current->group_leader->comm,
				current->group_leader->pid,
				current->pid, (pmem_len(data) >> PAGE_SHIFT));
		BUG();
	}

	new_prot = pmem_access_prot(file, pgprot_kernel);
	if (new_prot == pgprot_kernel) {
		BUG_ON(file->f_flags & (O_SYNC | FASYNC));
		return;
	}

	phys_start = PMEM_START_ADDR(data) + offset;
	start = (unsigned long)PMEM_START_VADDR(data) + offset;

	/*
	 * Ensure that the allocated pages are flushed and that any data
	 * lurking in the kernel direct-mapped region is invalidated.
	 */
	dmac_flush_range((void *)start, (void *)(start + len));
	outer_flush_range(phys_start, phys_start + len);

	apply_to_page_range(&init_mm, start, len, pmem_update_pte, &new_prot);

	dsb();
	flush_tlb_kernel_range(start, start + len);

	data->flags |= PMEM_FLAGS_MMAP_CHANGED;
}

static int pmem_map_pfn_range(int id, struct file *file,
			      struct vm_area_struct *vma,
			      struct pmem_data *data, unsigned long offset,
			      unsigned long len)
{
	DLOG("map offset %lx len %lx\n", offset, len);
	BUG_ON(!PMEM_IS_PAGE_ALIGNED(vma->vm_start));
	BUG_ON(!PMEM_IS_PAGE_ALIGNED(vma->vm_end));
	BUG_ON(!PMEM_IS_PAGE_ALIGNED(len));
	BUG_ON(!has_allocation(file));

	/* If this is the first mmap */
	if (!(data->flags & PMEM_FLAGS_MASTERMAP))
		pmem_update_kernel_mappings(id, file, data, offset, len);

	if (io_remap_pfn_range(vma, vma->vm_start + offset,
			       (PMEM_START_ADDR(data) + offset) >> PAGE_SHIFT,
			       len, vma->vm_page_prot)) {
		if ((data->flags & PMEM_FLAGS_MMAP_CHANGED) &&
			!(data->flags & PMEM_FLAGS_MASTERMAP))
			pmem_restore_kernel_mappings(id, data, offset, len);
		return -EAGAIN;
	}

	return 0;
}

#if 0
static bool pmem_watermark_ok(struct pmem_info *p_info)
{
	get_dev_cma_info(&p_info->pdev->dev, &p_info->cma);
	return (p_info->cma.max_free_block >= p_info->hwm);
}
#else
static bool pmem_watermark_ok(struct pmem_info *p_info)
{
	return true;
}
#endif

#ifdef CONFIG_ANDROID_PMEM_LOW_MEMORY_KILLER
static bool should_retry_allocation(int id, struct pmem_data *data)
{
	long ret;
	bool answer = false;

	up_write(&data->sem);

	if (fatal_signal_pending(current))
		goto out;

	/* if the work was idle, we rescheule with force_kill = 1
	 * if it wasn't idle, then just retry the allocation
	 * as the pending work must have killed someone
	 */
	if (flush_work_sync(&pmem[id].pmem_shrinker)) {
		if (!fatal_signal_pending(current))
			answer = true;
		goto out;
	}

	pmem[id].force_kill = 1;
	schedule_work(&pmem[id].pmem_shrinker);
	ret = wait_event_interruptible(pmem[id].deatheaters,
					(pmem[id].force_kill == 0));
	/* if we got a signal, dont retry */
	if (ret == -ERESTARTSYS)
		goto out;

	answer = true;

out:
	down_write(&data->sem);
	return answer;
}
#endif /* CONFIG_ANDROID_PMEM_LOW_MEMORY_KILLER */

#ifdef CONFIG_CMA
/* Must have down_write(&data->sem) locked */
static int pmem_cma_allocate(int id, unsigned long len, struct pmem_data *data)
{
	struct page *page;
	unsigned long nr_pages = len >> PAGE_SHIFT;
	unsigned int pass = 0;
	int ret = 0;

	if (nr_pages > pmem[id].cma.nr_pages) {
		ret = -ENOMEM;
		goto out;
	}

	do {
		page = dma_alloc_from_contiguous(&pmem[id].pdev->dev,
						nr_pages, 0);
		if (likely(page))
			break;

		if (fatal_signal_pending(current)) {
			ret = -EINTR;
			goto out;
		}

		if ((++pass % 10) == 0) {
			printk(KERN_INFO"pmem: %s/%d tried %u times"
					"to allocate %lu pages\n",
					current->group_leader->comm,
					current->pid, pass,
					nr_pages);
		}
		if (pass > 50) {
			ret = -ENOMEM;
			goto out;
		}
	} while (should_retry_allocation(id, data));

	if (unlikely(fatal_signal_pending(current)))
		goto out;

	BUG_ON(!current->group_leader->mm);

	atomic_long_add(nr_pages, &current->group_leader->mm->cma_stat);
	data->pfn = page_to_pfn(page);
	data->size = len;
	data->flags |= PMEM_FLAGS_CMA;

	if (!pmem_watermark_ok(&pmem[id]))
		wake_up_all(&cleaners);
out:
	return ret;
}
#endif

/* must be called with down_write(data->sem) */
static int pmem_allocate(struct file *file, int id, struct pmem_data *data,
			 unsigned long len)
{
	unsigned long addr;
	int ret = 0;


	if ((len < PAGE_SIZE) || !PMEM_IS_PAGE_ALIGNED(len)) {
		printk(KERN_ERR"pmem: allocation (%lu) must be aligned"
		       " to multiple of pages_size.\n", len);
		ret = -EINVAL;
		goto out;
	}

	if (!data) {
		printk(KERN_ERR"pmem: Invalid data passed to pmem_allocate\n");
		ret = -ENODEV;
		goto out;
	}

	down_write(&data->sem);

	BUG_ON(has_allocation(file));

	/* do not use kmalloc yet */
	if (false && is_power_of_2(len) && (len <= KMALLOC_MAX_SIZE)) {
		/* Try kmalloc allocation first */
		addr = (unsigned long)kmalloc(len, GFP_KERNEL);
		if (addr) {
			data->flags |= PMEM_FLAGS_KMALLOC;
			data->pfn = __phys_to_pfn(virt_to_phys((void *)addr));
			data->size = len;
			goto out_unlock;
		}
	}

	/* if we have a carveout heap and allocation is cached/uncached */
	if (pmem[id].carveout_base &&
		(file->f_flags & O_NONBLOCK)) {

		addr = gen_pool_alloc(pmem[id].pool, len);
		if (addr) {
			data->flags |= PMEM_FLAGS_CARVEOUT;
			data->pfn = __phys_to_pfn(addr);
			data->size = len;
			outer_inv_range(addr, addr+len);
			goto out_unlock;
		} else {
			printk(KERN_ALERT"carveout failed: %lukB\n", len/SZ_1K);
		}
		/* If we failed, fallback to CMA */
	}
#ifdef CONFIG_CMA
	ret = pmem_cma_allocate(id, len, data);
	if (ret == -ENOMEM) {
		printk(KERN_ERR"%s:%d pmem: Alloc failed (%ldkB, %ld pages)\n",
				current->group_leader->comm,
				current->group_leader->pid,
				len/SZ_1K, len >> PAGE_SHIFT);
		get_dev_cma_info(&pmem[id].pdev->dev, &pmem[id].cma);
		printk(KERN_ERR"CMA region details\n");
		printk(KERN_ERR "start PFN    : %lx\n"
				"nr_pages     : %ld pages\n"
				"biggest free : %ld pages\n"
				"total alloc  : %ld pages\n"
				"peak alloc   : %ld pages\n",
				 pmem[id].cma.start_pfn,
				 pmem[id].cma.nr_pages,
				 pmem[id].cma.max_free_block,
				 pmem[id].cma.total_alloc,
				 pmem[id].cma.peak_alloc);
	}
#endif

out_unlock:
	up_write(&data->sem);
out:
	return ret;
}

int is_pmem_file(struct file *file)
{
	int id;

	if (unlikely(!file || !file->f_dentry || !file->f_dentry->d_inode))
		return 0;
	id = get_id(file);
	if (unlikely(id >= PMEM_MAX_DEVICES))
		return 0;
	if (unlikely(file->f_dentry->d_inode->i_rdev !=
		     MKDEV(MISC_MAJOR, pmem[id].dev.minor)))
		return 0;
	return 1;
}

int get_pmem_addr(struct file *file, unsigned long *start,
		  unsigned long *vstart, unsigned long *len)
{
	struct pmem_data *data;
	int id;

	if (!is_pmem_file(file)) {
		printk(KERN_ERR
		       "pmem: requested reference from non-pmem file\n");
		return -EINVAL;
	}

	if (!has_allocation(file)) {
		printk(KERN_WARNING
			"pmem: requested reference from file with no "
		       "allocation.\n");
		*start = *len = *vstart = 0UL;
		return 0;
	}

	data = (struct pmem_data *)file->private_data;
	id = get_id(file);

	down_read(&data->sem);
	*start = PMEM_START_ADDR(data);
	*len = pmem_len(data);
	if (data->flags & PMEM_FLAGS_CARVEOUT)
		*vstart = 0; /* carveout has no kernel vaddr */
	else
		*vstart = (unsigned long)PMEM_START_VADDR(data);
	up_read(&data->sem);

	return 0;
}

int get_pmem_file(int fd, unsigned long *start, unsigned long *vstart,
		  unsigned long *len, struct file **filp)
{
	struct file *file;

	file = fget(fd);
	if (unlikely(file == NULL)) {
		printk(KERN_INFO "pmem: requested data from file descriptor "
		       "that doesn't exist.");
		return -EINVAL;
	}

	if (get_pmem_addr(file, start, vstart, len))
		goto end;

	if (filp)
		*filp = file;
	return 0;
end:
	fput(file);
	return -ENODEV;
}

void put_pmem_file(struct file *file)
{
	if (!is_pmem_file(file)) {
		printk(KERN_INFO "pmem: requested to release a non-pmem file");
		return;
	}

	fput(file);
}

void invalidate_pmem_file(struct file *file, unsigned long offset,
			  unsigned long len)
{
	struct pmem_data *data;
	int id;
	void *vaddr;
	phys_addr_t paddr;

	if (!is_pmem_file(file) || !has_allocation(file))
		return;

	id = get_id(file);
	data = (struct pmem_data *)file->private_data;
	if ((file->f_flags & O_SYNC) || (file->f_flags & O_NONBLOCK))
		return;

	down_read(&data->sem);
	paddr = PMEM_START_ADDR(data);
	vaddr = PMEM_START_VADDR(data);

	outer_inv_range(paddr, paddr + pmem_len(data));
	dmac_unmap_area(vaddr, pmem_len(data), DMA_FROM_DEVICE);

	up_read(&data->sem);
}

void flush_pmem_file(struct file *file, unsigned long offset,
		unsigned long len, int flush_all_flag)
{
	struct pmem_data *data;
	int id;
	void *vaddr;
	phys_addr_t paddr;

	if (!is_pmem_file(file) || !has_allocation(file))
		return;

	id = get_id(file);
	data = (struct pmem_data *)file->private_data;
	if (file->f_flags & O_SYNC || file->f_flags & FASYNC ||
			(file->f_flags & O_NONBLOCK))
		return;

	down_read(&data->sem);
	paddr = PMEM_START_ADDR(data);
	vaddr = PMEM_START_VADDR(data);

	if (flush_all_flag) {
		__cpuc_flush_kern_all();
		outer_flush_all();
		goto end;
	}

	/* if this isn't a submmapped file, flush the whole thing */
	dmac_flush_range(vaddr, vaddr + pmem_len(data));
	outer_flush_range(paddr, paddr + pmem_len(data));

end:
	up_read(&data->sem);
}

#ifdef CONFIG_CMA
static int pmem_cma_free(int id, struct pmem_data *data)
{
	int ret;
	struct page *page;
	int nr_pages = pmem_len(data) >> PAGE_SHIFT;
	struct task_struct *task = current->group_leader;

	BUG_ON(!nr_pages);
	BUG_ON(!(data->flags & PMEM_FLAGS_CMA));

	page = PMEM_START_PAGE(data);
	DLOG("pfn %d\n", data->pfn);

	ret = dma_release_from_contiguous(&pmem[id].pdev->dev, page, nr_pages);
	BUG_ON(ret == 0);

	if (!(task->flags & PF_EXITING) &&
		task_is_allocator(data, task) &&
		task->mm) {
		atomic_long_add(-nr_pages, &task->mm->cma_stat);
	}

	if (pmem[id].deathpending) {
		pmem[id].deathpending_rss -= nr_pages;
		if (pmem[id].deathpending_rss <= 0L) {
			pmem[id].deathpending = NULL;
			pmem[id].deathpending_rss = 0L;
			printk(KERN_INFO
				"pmem: memory released after killing!\n");
			up(&pmem[id].shrinker_sem);
		}
	}

	data->flags &= ~PMEM_FLAGS_CMA;

	return 0;
}
#endif

/* must have down_write on data->sem */
static int pmem_free(struct file *file, int id, struct pmem_data *data)
{
	unsigned long addr;
	int ret = 0;

	if (!data) {
		printk(KERN_ERR"pmem: Invalida 'data' passed to pmem_free\n");
		ret = -EINVAL;
		goto out;
	}

	down_write(&data->sem);

	/* If we have nothing to free here */
	if (!has_allocation(file))
		goto out_unlock;

	BUG_ON(!(data->flags & PMEM_FLAGS_ALLOCMASK));

	/* restore kernel mappings if we changed them
	 * after allocation
	 */
	if (data->flags & PMEM_FLAGS_MMAP_CHANGED)
		pmem_restore_kernel_mappings(id, data, 0, data->size);

	switch (data->flags & PMEM_FLAGS_ALLOCMASK) {
	case PMEM_FLAGS_KMALLOC:
		BUG_ON(!is_power_of_2(data->size));
		addr = (unsigned long)phys_to_virt(__pfn_to_phys(data->pfn));
		kfree((void *)addr);
		data->flags &= ~PMEM_FLAGS_KMALLOC;
		break;
	case PMEM_FLAGS_CARVEOUT:
		addr = PMEM_START_ADDR(data);
		gen_pool_free(pmem[id].pool, addr, data->size);
		data->flags &= ~PMEM_FLAGS_CARVEOUT;
		break;
#ifdef CONFIG_CMA
	case PMEM_FLAGS_CMA:
		ret = pmem_cma_free(id, data);
		break;
#endif
	default:
		printk(KERN_ALERT "pmem: invalid allocation flags\n");
		ret = -EINVAL;
	}

	data->pfn = -1UL;
	data->size = 0;

out_unlock:
	up_write(&data->sem);
out:
	return ret;
}

#ifdef CONFIG_CMA
/* pmem lowmemory killer */
static void pmem_shrink(struct work_struct *work)
{
	struct list_head *itr;
	struct task_struct *task, *selected = NULL;
	unsigned long task_cmasize, selected_task_cmasize = 0;
	int selected_oom_adj = OOM_DISABLE;
	struct pmem_data *data;
	struct pmem_info *p_info =
	    container_of(work, struct pmem_info, pmem_shrinker);

	mutex_lock(&p_info->shrinker_lock);

	if (!p_info->force_kill &&
	    (pmem_watermark_ok(p_info) || p_info->deathpending))
		goto out;

	/* Scan the list and find the task with minimum oom_adj value */
	mutex_lock(&p_info->data_list_lock);
	list_for_each(itr, &p_info->data_list) {
		int oom_adj;
		data = list_entry(itr, struct pmem_data, list);

		down_read(&data->sem);
		/* Skip data w/o MASTERMAP */
		if (!(data->flags & PMEM_FLAGS_MASTERMAP)) {
			up_read(&data->sem);
			continue;
		}

		rcu_read_lock();
		task = find_task_by_pid_ns(data->pid, &init_pid_ns);
		if (task)
			task_lock(task);
		rcu_read_unlock();
		up_read(&data->sem);

		if (!task)
			continue;

		if (!task->mm || !task->signal) {
			task_unlock(task);
			continue;
		}

		oom_adj = task->signal->oom_adj;
		/* The task is too important to kill */
		if (oom_adj < 0) {
			task_unlock(task);
			continue;
		}

		task_cmasize = get_mm_cma(task->mm);
		task_unlock(task);

		BUG_ON(!task_cmasize);
		if (selected) {
			/* Its possible we encounter the selected task again */
			if (selected == task)
				continue;
			if (oom_adj < selected_oom_adj)
				continue;
			if ((oom_adj == selected_oom_adj) &&
			    (task_cmasize <= selected_task_cmasize))
				continue;
		}

		selected = task;
		selected_task_cmasize = task_cmasize;
		selected_oom_adj = oom_adj;
	}

	mutex_unlock(&p_info->data_list_lock);

	if (selected) {
		int ret;
		printk(KERN_INFO
			"pmem: send sigkill (%s/%d),adj %d, %lu pages\n",
			selected->comm, selected->pid,
			selected_oom_adj, selected_task_cmasize);
		p_info->deathpending = selected;
		p_info->deathpending_rss = selected_task_cmasize;
		send_sig_info(SIGKILL, SEND_SIG_FORCED, selected);
		/* wait for process to die .... */
		ret = down_timeout(&p_info->shrinker_sem, HZ*20);
		if (ret == -ETIME) {
			p_info->deathpending = NULL;
			p_info->deathpending_rss = 0L;
			printk(KERN_ALERT
				"pmem: Shrinker took more than 20secs"
				" to release memory from task (%s)!\n",
				selected->comm);
		}

	}

out:
	mutex_unlock(&p_info->shrinker_lock);
	p_info->force_kill = 0;
	wake_up_all(&p_info->deatheaters);
}
#endif

static int pmem_open(struct inode *inode, struct file *file)
{
	struct pmem_data *data;
	int id = get_id(file);

	DLOG("current %u file %p(%ld)\n", current->pid, file, file_count(file));
	/* setup file->private_data to indicate its unmapped */
	/*  you can only open a pmem device one time */

	/* pmem_open() is invoked by misc_open() which will initialse
	 * file->private_data to misdevice pointer.
	 */
	if ((file->private_data != NULL)
	    && (file->private_data != &pmem[id].dev))
		return -ENODEV;

	data = kzalloc(sizeof(struct pmem_data), GFP_KERNEL);
	if (!data) {
		printk("pmem: unable to allocate memory for pmem metadata.");
		return -ENOMEM;
	}

	data->pfn = -1UL;
	INIT_LIST_HEAD(&data->list);
	init_rwsem(&data->sem);

	file->private_data = data;

	mutex_lock(&pmem[id].data_list_lock);
	list_add_tail(&data->list, &pmem[id].data_list);
	mutex_unlock(&pmem[id].data_list_lock);

	return 0;
}

static int pmem_mmap(struct file *file, struct vm_area_struct *vma)
{
	struct pmem_data *data;
	unsigned long vma_size = vma->vm_end - vma->vm_start;
	int ret = 0;

	if (vma->vm_pgoff || !PMEM_IS_PAGE_ALIGNED(vma_size)) {
		printk(KERN_ERR "pmem: mmaps must be at offset zero, aligned"
		       " and a multiple of pages_size.\n");
		ret = -EINVAL;
		goto out;
	}

	data = (struct pmem_data *)file->private_data;
	if (!data) {
		ret = -ENODEV;
		goto out;
	}

	down_write(&data->sem);

	if (unlikely(!has_allocation(file))) {
		printk(KERN_ALERT"pmem: mmap without calling PMEM_ALLOCATE"
				" is deprecated!!!\n");
		up_write(&data->sem);
		ret = pmem_allocate(file, get_id(file), data, vma_size);
		if (ret)
			goto out;
		down_write(&data->sem);
	}

	if (pmem_len(data) != vma_size) {
		printk(KERN_ERR"pmem: mmap (%lu pages) doesn't match"
				" the allocation (%lu pages)\n",
				(vma_size >> PAGE_SHIFT),
				(pmem_len(data) >> PAGE_SHIFT));
		ret = -EINVAL;
		goto out_unlock;
	}

	vma->vm_pgoff = PMEM_START_ADDR(data) >> PAGE_SHIFT;
	vma->vm_page_prot = pmem_access_prot(file, vma->vm_page_prot);

	if (pmem_map_pfn_range(get_id(file), file, vma, data, 0, vma_size)) {
		printk(KERN_ERR"pmem: mmap failed in kernel!\n");
		ret = -EAGAIN;
		goto out_unlock;
	}

	if (!(data->flags & PMEM_FLAGS_MASTERMAP)) {
		data->flags |= PMEM_FLAGS_MASTERMAP;
		data->pid = task_pid_nr(current->group_leader);
	}

out_unlock:
	up_write(&data->sem);
out:
	return ret;

}

static long pmem_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct pmem_data *data;
	int id = get_id(file);
	struct pmem_region region;
	bool should_wait;
	int ret = 0;

	switch (cmd) {
	case PMEM_ALLOCATE:
		if (unlikely(has_allocation(file)))
			return -EEXIST;
		if (copy_from_user(&region, (void __user *)arg,
				   sizeof(struct pmem_region)))
			return -EFAULT;
		data = (struct pmem_data *)file->private_data;
		ret = pmem_allocate(file, get_id(file), data, region.len);
		if (ret == 0) {
			region.offset = PMEM_START_ADDR(data);
		} else {
			region.offset = -1UL;
			region.len = -1UL;
		}
		if (copy_to_user((void __user *)arg, &region,
				 sizeof(struct pmem_region))) {
			pmem_free(file, get_id(file), data);
			ret = -EFAULT;
		}
		break;
	case PMEM_GET_PHYS:
	case PMEM_GET_SIZE:
		DLOG("get phys / get size\n");
		if (!has_allocation(file)) {
			region.offset = 0;
			region.len = 0;
		} else {
			data = (struct pmem_data *)file->private_data;
			region.offset = PMEM_START_ADDR(data);
			region.len = pmem_len(data);
		}
		if (copy_to_user((void __user *)arg, &region,
				 sizeof(struct pmem_region)))
			return -EFAULT;
		break;
	case PMEM_GET_TOTAL_SIZE:
		DLOG("get total size\n");
		region.offset = 0;
		region.len = pmem[id].cma.nr_pages << PAGE_SHIFT;
		if (copy_to_user((void __user *)arg, &region,
				 sizeof(struct pmem_region)))
			return -EFAULT;
		break;
	case PMEM_SET_DIRTY_REGION:
		if (!has_allocation(file))
			return -EINVAL;
		data = (struct pmem_data *)file->private_data;
		down_write(&data->sem);
		data->flags |= PMEM_FLAGS_DIRTY_REGION;
		up_write(&data->sem);
		break;
	case PMEM_GET_DIRTY_REGION:
		if (!has_allocation(file))
			return -EINVAL;
		data = (struct pmem_data *)file->private_data;
		down_write(&data->sem);
		if (data->flags & PMEM_FLAGS_DIRTY_REGION) {
			region.len = 1;
			data->flags &= ~PMEM_FLAGS_DIRTY_REGION;
		} else {
			region.len = 0;
		}
		up_write(&data->sem);

		if (copy_to_user((void __user *)arg, &region,
				 sizeof(struct pmem_region)))
			return -EFAULT;
		break;
	case PMEM_CACHE_FLUSH:
		data = (struct pmem_data *)file->private_data;

		if (copy_from_user(&region, (void __user *)arg,
				   sizeof(struct pmem_region)))
			return -EFAULT;

		DLOG("flush with offset=0x%08lx len=0x%08lx\n", region.offset,
		     region.len);
		flush_pmem_file(file, region.offset, region.len, 0);

		break;
	case PMEM_CACHE_FLUSH_ALL:
		data = (struct pmem_data *)file->private_data;

		if (copy_from_user(&region, (void __user *)arg,
				   sizeof(struct pmem_region)))
			return -EFAULT;

		DLOG("flush all with offset=0x%08lx len=0x%08lx\n",
				region.offset, region.len);
		flush_pmem_file(file, region.offset, region.len, 1);

		break;
	case PMEM_CACHE_INVALIDATE:
		data = (struct pmem_data *)file->private_data;

		if (copy_from_user(&region, (void __user *)arg,
				   sizeof(struct pmem_region)))
			return -EFAULT;
		DLOG("Invalidate with offset=0x%08lx len=0x%08lx\n",
		     region.offset, region.len);
		invalidate_pmem_file(file, region.offset, region.len);
		break;
	case PMEM_CLEANER_WAIT:
		data = (struct pmem_data *)file->private_data;

		should_wait = pmem_watermark_ok(&pmem[id]);

		if (should_wait) {
			ret =
			    wait_event_interruptible(cleaners,
						     !pmem_watermark_ok(&pmem
									[id]));
			ret = -ERESTARTSYS;
			if (ret == -ERESTARTSYS)
				ret = -EAGAIN;
		}
		break;
	default:
		ret = -EINVAL;
	}

	return ret;
}

static int pmem_release(struct inode *inode, struct file *file)
{
	struct pmem_data *data = (struct pmem_data *)file->private_data;
	int id = get_id(file), ret = 0;

	mutex_lock(&pmem[id].data_list_lock);
	list_del(&data->list);
	mutex_unlock(&pmem[id].data_list_lock);

	/* if it has an allocation, free it */
	ret = pmem_free(file, id, data);
	if (ret)
		printk(KERN_ERR"pmem: pmem_free failed\n");

	file->private_data = NULL;

	kfree(data);

	return ret;
}

static ssize_t debug_open(struct inode *inode, struct file *file)
{
	file->private_data = inode->i_private;
	return 0;
}

static ssize_t debug_read(struct file *file, char __user * buf, size_t count,
			  loff_t *ppos)
{
	struct list_head *elt;
	struct pmem_data *data;
	struct task_struct *task;
	int id = (int)file->private_data;
	int buf_order = 1;
	int debug_bufmax;
	char *buffer;
	unsigned long kmalloc_total, carveout_total;
	unsigned long cma_total, total;
	int n, ret;

	for (;;) {
		n = 0;
		kmalloc_total = carveout_total = cma_total = total = 0UL;
		debug_bufmax = PAGE_SIZE * (1 << buf_order);
		buffer = kmalloc(debug_bufmax, GFP_KERNEL);
		if (!buffer)
			return -ENOMEM;

		n = scnprintf(buffer, debug_bufmax,
			      "process (pid #) :  flags | size | range | mapped regions (start, end) (start, end)...\n");

		mutex_lock(&pmem[id].data_list_lock);
		list_for_each(elt, &pmem[id].data_list) {
			unsigned long size;

			data = list_entry(elt, struct pmem_data, list);
			down_read(&data->sem);
			/* Skip if we don't have allocation */
			if (!(data->flags & PMEM_FLAGS_MASTERMAP)) {
				up_read(&data->sem);
				continue;
			}

			rcu_read_lock();
			task = find_task_by_pid_ns(data->pid, &init_pid_ns);
			rcu_read_unlock();
			if (task) {
				task_lock(task);
				n += scnprintf(buffer + n, debug_bufmax - n,
					       "%-16s (%6u) :",
					       task->comm, data->pid);

				task_unlock(task);
			} else {
				n += scnprintf(buffer + n, debug_bufmax - n,
					       "%-25s :",
					       "non-allocating task");
			}

			size = pmem_len(data);

			n += scnprintf(buffer + n, debug_bufmax - n,
				       " 0x%08x", data->flags);
			n += scnprintf(buffer + n, debug_bufmax - n,
				       " %08ldkB", size / SZ_1K);

			n += scnprintf(buffer + n, debug_bufmax - n,
				       " (0x%08x-0x%08lx)",
				       PMEM_START_ADDR(data),
				       PMEM_START_ADDR(data) + size);


			n += scnprintf(buffer + n, debug_bufmax - n, "\n");

			if (data->flags & PMEM_FLAGS_KMALLOC)
				kmalloc_total += size;
			if (data->flags & PMEM_FLAGS_CMA)
				cma_total += size;
			if (data->flags & PMEM_FLAGS_CARVEOUT)
				carveout_total += size;

			total += size;

			up_read(&data->sem);
		}

		n += scnprintf(buffer + n, debug_bufmax - n,
			       "=========================================\n");
		n += scnprintf(buffer + n, debug_bufmax - n,
			       "CMA      : %lu pages, %08lukB\n",
			       cma_total >> PAGE_SHIFT, (cma_total / SZ_1K));
		n += scnprintf(buffer + n, debug_bufmax - n,
			       "Carveout : %lu pages, %08lukB\n",
			       carveout_total >> PAGE_SHIFT,
			       (carveout_total / SZ_1K));
		n += scnprintf(buffer + n, debug_bufmax - n,
			       "kmalloc  : %lu pages, %08lukB\n",
			       kmalloc_total >> PAGE_SHIFT,
			       (kmalloc_total / SZ_1K));
		n += scnprintf(buffer + n, debug_bufmax - n,
			       "Total    : %lu pages, %08lukB\n",
			       total >> PAGE_SHIFT, (total / SZ_1K));

		mutex_unlock(&pmem[id].data_list_lock);

		n++;

		if (n >= debug_bufmax) {
			buf_order++;
			if (buf_order > 3) {
				printk(KERN_ERR
				       "pmem: user list is too long\n");
				n = debug_bufmax - 1;
				break;
			}
			kfree(buffer);
			continue;
		}

		break;
	}

	buffer[n] = 0;
	ret = simple_read_from_buffer(buf, count, ppos, buffer, n);
	kfree(buffer);

	return ret;
}

static struct file_operations debug_fops = {
	.read = debug_read,
	.open = debug_open,
};

const struct file_operations pmem_fops = {
	.open = pmem_open,
	.mmap = pmem_mmap,
	.unlocked_ioctl = pmem_ioctl,
	.release = pmem_release,
};

static int pmem_setup(struct platform_device *pdev,
	       struct android_pmem_platform_data *pdata)
{
	int err = 0;
	int id = id_count;
	struct page *page;

	id_count++;

#ifdef CONFIG_CMA
	if (pdata->cmasize) {
		get_dev_cma_info(&pdev->dev, &pmem[id].cma);
		BUG_ON(pmem[id].cma.nr_pages != (pdata->cmasize >> PAGE_SHIFT));
		pmem[id].pdev = pdev;
		/* These are only used when we have associated CMA region */
		mutex_init(&pmem[id].shrinker_lock);
		INIT_WORK(&pmem[id].pmem_shrinker, pmem_shrink);
		sema_init(&pmem[id].shrinker_sem, 0);
		init_waitqueue_head(&pmem[id].deatheaters);
		pmem[id].deathpending = NULL;
	} else {
		memset(&pmem[id].cma, 0, sizeof(pmem[id].cma));
	}
#endif
	if (pdata->carveout_base && pdata->carveout_size) {
		BUG_ON(pdata->carveout_size & (PAGE_SIZE - 1));
		pmem[id].pool = gen_pool_create(12, -1);
		if (!pmem[id].pool) {
			printk(KERN_ERR
					"pmem: genpool_create failed\n");
		} else {
			gen_pool_add(pmem[id].pool,
					pdata->carveout_base,
					pdata->carveout_size, -1);
			pmem[id].carveout_base = pdata->carveout_base;
			pmem[id].carveout_size = pdata->carveout_size;
		}
	}

	mutex_init(&pmem[id].data_list_lock);
	INIT_LIST_HEAD(&pmem[id].data_list);
	pmem[id].dev.name = pdata->name;
	pmem[id].dev.minor = id;
	pmem[id].dev.fops = &pmem_fops;

	printk(KERN_INFO "%s: init\n", pdata->name);

	page = alloc_page(GFP_KERNEL);
	if (page == NULL) {
		err = -ENOMEM;
		goto error;
	}

	pmem[id].garbage_pfn = page_to_pfn(page);

	err = misc_register(&pmem[id].dev);
	if (err) {
		printk(KERN_ALERT "Unable to register pmem driver!\n");
		goto err_cant_register_device;
	}

	debugfs_create_file(pdata->name, S_IFREG | S_IRUSR, NULL, (void *)id,
			    &debug_fops);

	printk(KERN_INFO"Pmem initialised with %lu CMA pages\n",
			pmem[id].cma.nr_pages);
	return 0;

err_cant_register_device:
	__free_page(page);
error:
	if (pmem[id].pool)
		gen_pool_destroy(pmem[id].pool);
	return err;
}

static int pmem_probe(struct platform_device *pdev)
{
	struct android_pmem_platform_data *pdata;

	if (!pdev || !pdev->dev.platform_data) {
		printk(KERN_ALERT "Unable to probe pmem!\n");
		return -ENODEV;
	}
	pdata = pdev->dev.platform_data;
	return pmem_setup(pdev, pdata);
}

static int pmem_remove(struct platform_device *pdev)
{
	int id = pdev->id;

	mutex_lock(&pmem[id].data_list_lock);
	/* There should not be any allocations in the list */
	if (!list_empty(&pmem[id].data_list)) {
		mutex_unlock(&pmem[id].data_list_lock);
		return -EBUSY;
	}

	__free_page(pfn_to_page(pmem[id].garbage_pfn));
	if (pmem[id].pool)
		gen_pool_destroy(pmem[id].pool);

	misc_deregister(&pmem[id].dev);
	mutex_unlock(&pmem[id].data_list_lock);

	memset(&pmem[id], 0, sizeof(struct pmem_info));

	return 0;
}

static struct platform_driver pmem_driver = {
	.probe = pmem_probe,
	.remove = pmem_remove,
	.driver = {.name = "android_pmem"}
};

static int __init pmem_init(void)
{
	return platform_driver_register(&pmem_driver);
}

static void __exit pmem_exit(void)
{
	platform_driver_unregister(&pmem_driver);
}

module_init(pmem_init);
module_exit(pmem_exit)
