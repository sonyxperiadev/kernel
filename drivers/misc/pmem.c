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

#define PMEM_MAX_DEVICES	(2)
#define PMEM_MAX_ORDER		(128)
#define PMEM_MIN_ALLOC		PAGE_SIZE

#define PMEM_DEBUG		0

/* indicates that a refernce to this file has been taken via get_pmem_file,
 * the file should not be released until put_pmem_file is called */
#define PMEM_FLAGS_BUSY		(0x1)
/* indicates that this is a suballocation of a larger master range */
#define PMEM_FLAGS_CONNECTED	(0x1 << 1)
/* indicates this is a master and not a sub allocation and that it is mmaped */
#define PMEM_FLAGS_MASTERMAP	(0x1 << 2)
/* submap and unsubmap flags indicate:
 * 00: subregion has never been mmaped
 * 10: subregion has been mmaped, reference to the mm was taken
 * 11: subretion has ben released, refernece to the mm still held
 * 01: subretion has been released, reference to the mm has been released
 */
#define PMEM_FLAGS_UNSUBMAP	(0x1 << 3)
#define PMEM_FLAGS_SUBMAP	(0x1 << 4)

#define PMEM_FLAGS_DIRTY_REGION	(0x1 << 5)
/* Set if kernel mappings were change to match userspace mmap() */
#define PMEM_FLAGS_MMAP_CHANGED	(0x1 << 6)

/* What type of allocation ? */
#define PMEM_FLAGS_KMALLOC	(0x1 << 8)
#define PMEM_FLAGS_CMA		(0x1 << 9)
#define PMEM_FLAGS_CARVEOUT	(0x1 << 10)
#define PMEM_FLAGS_ALLOCMASK	(PMEM_FLAGS_KMALLOC | PMEM_FLAGS_CMA \
					| PMEM_FLAGS_CARVEOUT)

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
	/* file descriptor of the master */
	int master_fd;
	/* file struct of the master */
	struct file *master_file;
	/* a list of currently available regions if this is a suballocation */
	struct list_head region_list;
	/* a linked list of data so we can access them for debugging */
	struct list_head list;
	/* a linked list of data that has a submap, so we can access them
	 * quickly in pmem_release()
	 */
	struct list_head sub_data_list;

	int ref;
};

struct pmem_region_node {
	struct pmem_region region;
	struct list_head list;
};

#define PMEM_DEBUG_MSGS	0
#if PMEM_DEBUG_MSGS
#define DLOG(fmt,args...) \
	do { printk(KERN_INFO "[%s:%s:%d] "fmt, __FILE__, __func__, __LINE__, \
		    ##args); } \
	while (0)
#else
#define DLOG(x...) do {} while (0)
#endif

DECLARE_WAIT_QUEUE_HEAD(cleaners);

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
	/* Task killed by the pmem_shrinker */
	struct task_struct *deathpending;
	/* Wait Queue to wait for killed process to die .. */
	wait_queue_head_t deatheaters;
	/* Stats for the CMA region for this device */
	struct dev_cma_info cma;
	/* high water mark in pages for this pmem space */
	unsigned short hwm;
	/* protects data list */
	struct mutex data_list_lock;
	/* total size of the pmem space */
	struct list_head data_list;
	/* carved out memory for us */
	phys_addr_t carveout_base;
	phys_addr_t carveout_size;
	unsigned long carveout_vbase;
	/* genpool to manage carved out memory */
	struct gen_pool *pool;
};

static struct pmem_info pmem[PMEM_MAX_DEVICES];
static int id_count;

#define PMEM_IS_PAGE_ALIGNED(addr)	(!((addr) & (~PAGE_MASK)))
#define PMEM_START_PAGE(data)		pfn_to_page(data->pfn)
#define PMEM_START_ADDR(data)		__pfn_to_phys(data->pfn)
#define PMEM_START_VADDR(data)		phys_to_virt(PMEM_START_ADDR(data))

/* macros for carveout allocations */
#define PMEM_CARVEOUT_START_VADDR(id, data) \
	(pmem[id].carveout_vbase + \
	 (PMEM_START_ADDR(data) - pmem[id].carveout_base))
#define PMEM_CARVEOUT_VIRT_TO_PHYS(id, addr) \
	(pmem[id].carveout_base + (addr - pmem[id].carveout_vbase))
#define PMEM_CARVEOUT_PHYS_TO_VIRT(id, addr) \
	(pmem[id].carveout_vbase + (addr - pmem[id].carveout_pbase))

#define PMEM_IS_SUBMAP(data) \
	((data->flags & PMEM_FLAGS_SUBMAP) && \
		 (!(data->flags & PMEM_FLAGS_UNSUBMAP)))

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

static int pmem_release(struct inode *, struct file *);
static int pmem_mmap(struct file *, struct vm_area_struct *);
static int pmem_open(struct inode *, struct file *);
static long pmem_ioctl(struct file *, unsigned int, unsigned long);
static unsigned long pmem_len(struct pmem_data *);

struct file_operations pmem_fops = {
	.release = pmem_release,
	.mmap = pmem_mmap,
	.open = pmem_open,
	.unlocked_ioctl = pmem_ioctl,
};

static int get_id(struct file *file)
{
	return MINOR(file->f_dentry->d_inode->i_rdev);
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

static bool has_allocation(struct file *file)
{
	struct pmem_data *data;
	/* check is_pmem_file first if not accessed via pmem_file_ops */

	if (unlikely(!file->private_data))
		return false;
	data = (struct pmem_data *)file->private_data;
	if (unlikely(data->pfn == -1UL))
		return false;
	return true;
}

static int is_master_owner(struct file *file)
{
	struct file *master_file;
	struct pmem_data *data;
	int put_needed, ret = 0;

	if (!is_pmem_file(file) || !has_allocation(file))
		return 0;
	data = (struct pmem_data *)file->private_data;
	if (PMEM_FLAGS_MASTERMAP & data->flags)
		return 1;
	master_file = fget_light(data->master_fd, &put_needed);
	if (master_file && data->master_file == master_file)
		ret = 1;
	fput_light(master_file, put_needed);
	return ret;
}

/* Must be called with p_info->lock held */
static inline bool pmem_watermark_ok(struct pmem_info *p_info)
{
	get_dev_cma_info(&p_info->pdev->dev, &p_info->cma);

	return (p_info->cma.max_free_block >= p_info->hwm);
}

static void pmem_restore_kernel_mappings(int id, struct pmem_data *data,
					 unsigned long offset,
					 unsigned long len);

static int pmem_cma_free(int id, struct pmem_data *data)
{
	int ret;
	struct page *page;
	int nr_pages = pmem_len(data) >> PAGE_SHIFT;

	BUG_ON(!nr_pages);
	BUG_ON(!(data->flags & PMEM_FLAGS_CMA));

	page = PMEM_START_PAGE(data);
	DLOG("pfn %d\n", data->pfn);

	ret = dma_release_from_contiguous(&pmem[id].pdev->dev, page, nr_pages);
	BUG_ON(ret == 0);

	if (current->group_leader && current->group_leader->mm) {
		add_mm_counter(current->group_leader->mm,
			       MM_CMAPAGES, -nr_pages);
	}

	data->flags &= ~PMEM_FLAGS_CMA;

	return 0;
}

/* must have down_write on data->sem */
static int pmem_free(int id, struct pmem_data *data)
{
	int ret = 0;
	unsigned long addr;

	BUG_ON((data->pfn == -1UL) || !data->size);
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
	case PMEM_FLAGS_CMA:
		ret = pmem_cma_free(id, data);
		break;
	default:
		printk(KERN_ALERT "pmem: invalid allocation flags\n");
		ret = -EINVAL;
	}

	data->pfn = -1UL;
	data->size = 0;

	return ret;
}

static void pmem_revoke(struct file *file, struct pmem_data *data);

static int pmem_release(struct inode *inode, struct file *file)
{
	struct pmem_data *data = (struct pmem_data *)file->private_data;
	struct pmem_region_node *region_node;
	struct list_head *elt, *elt2;
	int id = get_id(file), ret = 0;

	mutex_lock(&pmem[id].data_list_lock);
	list_del(&data->list);
	mutex_unlock(&pmem[id].data_list_lock);

	down_write(&data->sem);
	/* if this file is a master, revoke all the memory in the connected
	 *  files */
	if ((PMEM_FLAGS_MASTERMAP & data->flags)
	    && !list_empty(&data->sub_data_list)) {
		struct pmem_data *sub_data;
		list_for_each_safe(elt, elt2, &data->sub_data_list) {
			sub_data =
			    list_entry(elt, struct pmem_data, sub_data_list);
			down_read(&sub_data->sem);
			BUG_ON(file != sub_data->master_file);
			if (PMEM_IS_SUBMAP(sub_data)) {
				up_read(&sub_data->sem);
				pmem_revoke(file, sub_data);
			} else {
				up_read(&sub_data->sem);
			}
			list_del(elt);
		}
	}

	/* if its not a connected file and it has an allocation, free it */
	if (!(PMEM_FLAGS_CONNECTED & data->flags) && has_allocation(file)) {
		WARN_ON(data->ref);
		ret = pmem_free(id, data);
	}

	/* if this file is a submap (mapped, connected file), downref the
	 * task struct */
	if (PMEM_FLAGS_SUBMAP & data->flags) {
		if (data->task) {
			put_task_struct(data->task);
			data->task = NULL;
		}
	}

	file->private_data = NULL;
	list_for_each_safe(elt, elt2, &data->region_list) {
		region_node = list_entry(elt, struct pmem_region_node, list);
		list_del(elt);
		kfree(region_node);
	}

	BUG_ON(!list_empty(&data->region_list));
	up_write(&data->sem);
	kfree(data);

	return ret;
}

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
	INIT_LIST_HEAD(&data->region_list);
	INIT_LIST_HEAD(&data->list);
	INIT_LIST_HEAD(&data->sub_data_list);
	init_rwsem(&data->sem);

	file->private_data = data;

	mutex_lock(&pmem[id].data_list_lock);
	list_add_tail(&data->list, &pmem[id].data_list);
	mutex_unlock(&pmem[id].data_list_lock);

	return 0;
}

static int pmem_cma_allocate(int id, unsigned long len, struct pmem_data *data)
{
	struct page *page;
	unsigned long nr_pages = len >> PAGE_SHIFT;

	if (nr_pages > pmem[id].cma.nr_pages) {
		printk(KERN_ERR "pmem:%d Failed to allocate %lu pages\n",
		       __LINE__, nr_pages);
		return -ENOMEM;
	}

	page = dma_alloc_from_contiguous(&pmem[id].pdev->dev, nr_pages, 0);
	if (!page) {
		printk(KERN_ERR "pmem:%d Failed to allocate %lu pages\n",
		       __LINE__, nr_pages);
		return -ENOMEM;
	}

	BUG_ON(!current->group_leader->mm);
	add_mm_counter(current->group_leader->mm, MM_CMAPAGES, nr_pages);
	if (!pmem_watermark_ok(&pmem[id])) {
		schedule_work(&pmem[id].pmem_shrinker);
		wake_up_all(&cleaners);
	}

	data->pfn = page_to_pfn(page);
	data->size = len;
	data->flags |= PMEM_FLAGS_CMA;

	return 0;
}

static int pmem_allocate(struct file *file, int id, struct pmem_data *data,
			 unsigned long len)
{
	unsigned long addr;

	/* We only do allocation in pages */
	BUG_ON(len < PAGE_SIZE);

	/* do not use kmalloc yet */
	if (false && (is_power_of_2(len) && (len <= KMALLOC_MAX_SIZE))) {
		/* Try kmalloc allocation first */
		addr = (unsigned long)kmalloc(len, GFP_KERNEL);
		if (addr) {
			data->flags |= PMEM_FLAGS_KMALLOC;
			data->pfn = __phys_to_pfn(virt_to_phys((void *)addr));
			data->size = len;
			return 0;
		}
	}

	/* if we have a carveout heap and allocation is uncached */
	if (pmem[id].carveout_base && (file->f_flags & O_SYNC)) {
		addr = gen_pool_alloc(pmem[id].pool, len);
		if (addr) {
			data->flags |= PMEM_FLAGS_CARVEOUT;
			data->pfn = __phys_to_pfn(addr);
			data->size = len;
			return 0;
		}
	}

	if (pmem_cma_allocate(id, len, data))
		return -ENOMEM;

	return 0;
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

static int pmem_map_garbage(int id, struct vm_area_struct *vma,
			    struct pmem_data *data, unsigned long offset,
			    unsigned long len)
{
	int i, garbage_pages = len >> PAGE_SHIFT;

	vma->vm_flags |= VM_IO | VM_RESERVED | VM_PFNMAP | VM_SHARED | VM_WRITE;
	for (i = 0; i < garbage_pages; i++) {
		if (vm_insert_pfn(vma, vma->vm_start + offset + (i * PAGE_SIZE),
				  pmem[id].garbage_pfn))
			return -EAGAIN;
	}
	return 0;
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

	/* if allocated from carveout heap, we dont change any mappings */
	if (data->flags & PMEM_FLAGS_CARVEOUT) {
		BUG_ON(!(file->f_flags & O_SYNC));
		return;
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

static int pmem_unmap_pfn_range(int id, struct vm_area_struct *vma,
				struct pmem_data *data, unsigned long offset,
				unsigned long len)
{
	int garbage_pages;
	unsigned long end;
	DLOG("unmap offset %lx len %lx\n", offset, len);

	BUG_ON(!PMEM_IS_PAGE_ALIGNED(len));

	garbage_pages = len >> PAGE_SHIFT;
	end = zap_page_range(vma, vma->vm_start + offset, len, NULL);
	printk(KERN_ERR "vma_start=0x%p len=0x%p end=0x%p\n",
	       (void *)(vma->vm_start + offset), (void *)len, (void *)end);
	pmem_map_garbage(id, vma, data, offset, len);
	return 0;
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
	BUG_ON(!PMEM_IS_PAGE_ALIGNED(offset));
	BUG_ON(!has_allocation(file));

	/* If this is the first mmap */
	if (!(data->flags & PMEM_FLAGS_MASTERMAP)) {
		BUG_ON(data->ref != 0);
		pmem_update_kernel_mappings(id, file, data, offset, len);
	}

	if (io_remap_pfn_range(vma, vma->vm_start + offset,
			       (PMEM_START_ADDR(data) + offset) >> PAGE_SHIFT,
			       len, vma->vm_page_prot)) {
		if ((data->flags & PMEM_FLAGS_MMAP_CHANGED) && (data->ref == 0))
			pmem_restore_kernel_mappings(id, data, offset, len);
		return -EAGAIN;
	}

	return 0;
}

static int pmem_remap_pfn_range(int id, struct file *file,
				struct vm_area_struct *vma,
				struct pmem_data *data, unsigned long offset,
				unsigned long len)
{
	unsigned long end;

	/* hold the mm semp for the vma you are modifying when you call this */
	BUG_ON(!vma);

	end = zap_page_range(vma, vma->vm_start + offset, len, NULL);
	if (end != vma->vm_start + offset) {
		printk(KERN_ERR
		       "%s: unexpected end (%lu), expected was (%lu)\n",
		       __func__, end, vma->vm_start + offset);
	}

	return pmem_map_pfn_range(id, file, vma, data, offset, len);
}

static void pmem_vma_open(struct vm_area_struct *vma)
{
	struct file *file = vma->vm_file;
	struct pmem_data *data = file->private_data;
	int id = get_id(file);
	/* this should never be called as we don't support copying pmem
	 * ranges via fork */
	BUG_ON(!has_allocation(file));
	down_write(&data->sem);
	/* remap the garbage pages, forkers don't get access to the data */
	if (data->pid != task_pid_nr(current->group_leader)) {
		printk(KERN_WARNING
		       "Warning! remapping pmem area with garbage page\n");
		pmem_unmap_pfn_range(id, vma, data, 0,
				     vma->vm_end - vma->vm_start);
	}
	up_write(&data->sem);
}

static void pmem_vma_close(struct vm_area_struct *vma)
{
	struct file *file = vma->vm_file;
	struct pmem_data *data = file->private_data;
	unsigned long vma_size = vma->vm_end - vma->vm_start;

	DLOG("current %u ppid %u file %p count %ld\n", current->pid,
	     current->parent->pid, file, file_count(file));

	if (unlikely(!is_pmem_file(file) || !has_allocation(file))) {
		printk(KERN_WARNING "pmem: something is very wrong, you are "
		       "closing a vm backing an allocation that doesn't "
		       "exist!\n");
		return;
	}

	down_write(&data->sem);
	if (data->vma == vma) {
		data->vma = NULL;
		if ((data->flags & PMEM_FLAGS_CONNECTED) &&
		    (data->flags & PMEM_FLAGS_SUBMAP))
			data->flags |= PMEM_FLAGS_UNSUBMAP;
	}

	if (data->flags & PMEM_FLAGS_MASTERMAP) {
		if (data->ref > 0)
			data->ref--;
	}

	BUG_ON(vma_size != pmem_len(data));
	/* the kernel is going to free this vma now anyway */
	up_write(&data->sem);
}

static struct vm_operations_struct vm_ops = {
	.open = pmem_vma_open,
	.close = pmem_vma_close,
};

static bool should_retry_allocation(int id)
{

	if (pmem_watermark_ok(&pmem[id]) || fatal_signal_pending(current))
		goto out;

	/* Dont retry any allocations for now */
	goto out;

	/* retry only if we have a pending death .. */
	if (pmem[id].deathpending) {
		printk(KERN_INFO "%s: waiting for deathpending!\n", __func__);
		if (wait_event_interruptible(pmem[id].deatheaters,
					     (pmem[id].deathpending == NULL)))
			goto out;

		return true;
	}
out:
	return false;
}

static int pmem_mmap(struct file *file, struct vm_area_struct *vma)
{
	struct pmem_data *data;
	unsigned long vma_size = vma->vm_end - vma->vm_start;
	int ret = 0, id = get_id(file);

	if (vma->vm_pgoff || !PMEM_IS_PAGE_ALIGNED(vma_size)) {
		printk(KERN_ERR "pmem: mmaps must be at offset zero, aligned"
		       " and a multiple of pages_size.\n");
		return -EINVAL;
	}

	data = (struct pmem_data *)file->private_data;

	if (!data) {
		ret = -ENODEV;
		goto error;
	}

	down_write(&data->sem);

	/* check this file isn't already mmaped, for submaps check this file
	 * has never been mmaped */
	if ((data->flags & PMEM_FLAGS_SUBMAP) ||
	    (data->flags & PMEM_FLAGS_UNSUBMAP)) {
		printk(KERN_ERR "pmem: you can only mmap a pmem file once, "
		       "this file is already mmaped. %x\n", data->flags);
		ret = -EINVAL;
		goto error_up_write;
	}

	if (likely(!has_allocation(file))) {
		do {
			ret = pmem_allocate(file, id, data,
					    vma->vm_end - vma->vm_start);
			if (ret != 0)
				printk(KERN_ERR "pmem: could not allocate\n");
			else
				break;
		} while (should_retry_allocation(id));
	}

	if (pmem_len(data) != vma_size) {
		printk(KERN_WARNING "pmem: mmap size [%lu] does not match"
		       "size of backing region [%lu].\n", vma_size,
		       pmem_len(data));
		ret = -EINVAL;
		goto error_up_write;
	}

	vma->vm_pgoff = PMEM_START_ADDR(data) >> PAGE_SHIFT;
	vma->vm_page_prot = pmem_access_prot(file, vma->vm_page_prot);

	if (data->flags & PMEM_FLAGS_CONNECTED) {
		struct pmem_region_node *region_node;
		struct list_head *elt;
		if (pmem_map_garbage(id, vma, data, 0, vma_size)) {
			printk("pmem: mmap failed in kernel!\n");
			ret = -EAGAIN;
			goto error_up_write;
		}
		list_for_each(elt, &data->region_list) {
			region_node = list_entry(elt, struct pmem_region_node,
						 list);
			DLOG("remapping file: %p %lx %lx\n", file,
			     region_node->region.offset,
			     region_node->region.len);
			if (pmem_remap_pfn_range(id, file, vma, data,
						 region_node->region.offset,
						 region_node->region.len)) {
				ret = -EAGAIN;
				goto error_up_write;
			}
		}
		data->flags |= PMEM_FLAGS_SUBMAP;
		get_task_struct(current->group_leader);
		data->task = current->group_leader;
		data->vma = vma;
		data->pid = task_pid_nr(current->group_leader);
		DLOG("submmapped file %p vma %p pid %u\n", file, vma,
		     current->pid);
	} else {
		if (pmem_map_pfn_range(id, file, vma, data, 0, vma_size)) {
			printk(KERN_INFO "pmem: mmap failed in kernel!\n");
			ret = -EAGAIN;
			goto error_free_mem;
		}

		if (!(data->flags & PMEM_FLAGS_MASTERMAP)) {
			data->flags |= PMEM_FLAGS_MASTERMAP;
			data->pid = task_pid_nr(current->group_leader);
		}
	}

	data->ref++;
	vma->vm_ops = &vm_ops;
	up_write(&data->sem);

	return ret;

error_free_mem:
	if (data->ref == 0) {
		printk(KERN_ERR "pmem: failed to free allocated memory\n");
		ret = pmem_free(id, data);
	}
error_up_write:
	up_write(&data->sem);
	return ret;
error:
	return ret;

}

#if 0
/* the following are the api for accessing pmem regions by other drivers
 * from inside the kernel */
int get_pmem_user_addr(struct file *file, unsigned long *start,
		       unsigned long *len)
{
	struct pmem_data *data;
	if (!is_pmem_file(file) || !has_allocation(file)) {
#if PMEM_DEBUG
		printk(KERN_INFO "pmem: requested pmem data from invalid"
		       "file.\n");
#endif
		return -1;
	}
	data = (struct pmem_data *)file->private_data;
	down_read(&data->sem);
	if (data->vma) {
		*start = data->vma->vm_start;
		*len = data->vma->vm_end - data->vma->vm_start;
	} else {
		*start = 0;
		*len = 0;
	}
	up_read(&data->sem);
	return 0;
}
#endif

int get_pmem_addr(struct file *file, unsigned long *start,
		  unsigned long *vstart, unsigned long *len)
{
	struct pmem_data *data;
	int id;

	if (!is_pmem_file(file) || !has_allocation(file)) {
		return -1;
	}

	data = (struct pmem_data *)file->private_data;
	if (data->pfn == -1) {
#if PMEM_DEBUG
		printk(KERN_INFO "pmem: requested pmem data from file with no "
		       "allocation.\n");
		return -1;
#endif
	}
	id = get_id(file);

	down_read(&data->sem);
	*start = PMEM_START_ADDR(data);
	*len = pmem_len(data);
	if (data->flags & PMEM_FLAGS_CARVEOUT)
		*vstart = (unsigned long)PMEM_CARVEOUT_START_VADDR(id, data);
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

void flush_pmem_file(struct file *file, unsigned long offset, unsigned long len)
{
	struct pmem_data *data;
	int id;
	void *vaddr;
	phys_addr_t paddr;
	struct pmem_region_node *region_node;
	struct list_head *elt;
	void *flush_start, *flush_end;

	if (!is_pmem_file(file) || !has_allocation(file)) {
		return;
	}

	id = get_id(file);
	data = (struct pmem_data *)file->private_data;
	if (file->f_flags & O_SYNC || file->f_flags & FASYNC)
		return;

	down_read(&data->sem);
	paddr = PMEM_START_ADDR(data);
	vaddr = PMEM_START_VADDR(data);
	/* if this isn't a submmapped file, flush the whole thing */
	if (!(data->flags & PMEM_FLAGS_CONNECTED)) {
		dmac_flush_range(vaddr, vaddr + pmem_len(data));
		outer_flush_range(paddr, paddr + pmem_len(data));
		goto end;
	}

	/* otherwise, flush the region of the file we are drawing */
	list_for_each(elt, &data->region_list) {
		region_node = list_entry(elt, struct pmem_region_node, list);
		if ((offset >= region_node->region.offset) &&
		    ((offset + len) <= (region_node->region.offset +
					region_node->region.len))) {
			flush_start = vaddr + region_node->region.offset;
			flush_end = flush_start + region_node->region.len;
			dmac_flush_range(flush_start, flush_end);
			outer_flush_range(virt_to_phys(flush_start),
					  virt_to_phys(flush_end));
			break;
		}
	}
end:
	up_read(&data->sem);
}

void invalidate_pmem_file(struct file *file, unsigned long offset,
			  unsigned long len)
{
	struct pmem_data *data;
	int id;
	void *vaddr;
	phys_addr_t paddr, inv_start, inv_end;
	struct pmem_region_node *region_node;
	struct list_head *elt;

	if (!is_pmem_file(file) || !has_allocation(file)) {
		return;
	}

	id = get_id(file);
	data = (struct pmem_data *)file->private_data;
	if (file->f_flags & O_SYNC)
		return;

	down_read(&data->sem);
	paddr = PMEM_START_ADDR(data);
	vaddr = PMEM_START_VADDR(data);
	/* if this isn't a submmapped file, invalidate the whole thing */
	if (unlikely(!(data->flags & PMEM_FLAGS_CONNECTED))) {
		outer_inv_range(paddr, paddr + pmem_len(data));
		dmac_unmap_area(vaddr, pmem_len(data), DMA_FROM_DEVICE);
		goto end;
	}

	/* otherwise, invalidate the region of the file we are drawing */
	list_for_each(elt, &data->region_list) {
		region_node = list_entry(elt, struct pmem_region_node, list);
		if ((offset >= region_node->region.offset) &&
		    ((offset + len) <= (region_node->region.offset +
					region_node->region.len))) {
			inv_start = paddr + region_node->region.offset;
			inv_end = inv_start + region_node->region.len;
			outer_inv_range(inv_start, inv_end);
			dmac_unmap_area(vaddr + region_node->region.offset,
					region_node->region.len,
					DMA_FROM_DEVICE);
			break;
		}
	}
end:
	up_read(&data->sem);
}

static inline void
do_cache_op(unsigned long start, unsigned long end, int flags)
{
	struct mm_struct *mm = current->active_mm;
	struct vm_area_struct *vma;

	if (end < start || flags)
		return;

	down_read(&mm->mmap_sem);
	vma = find_vma(mm, start);
	if (vma && vma->vm_start < end) {
		if (start < vma->vm_start)
			start = vma->vm_start;
		if (end > vma->vm_end)
			end = vma->vm_end;

		up_read(&mm->mmap_sem);
		flush_cache_user_range(start, end);
		return;
	}
	up_read(&mm->mmap_sem);
}

void flush_pmem_process_file(struct file *file, void *virt_base,
			     unsigned long offset)
{
	struct pmem_data *data;
	int id;
	void *vaddr;

	if (!is_pmem_file(file) || !has_allocation(file)) {
		return;
	}

	id = get_id(file);
	data = (struct pmem_data *)file->private_data;
	if (file->f_flags & O_SYNC || file->f_flags & FASYNC)
		return;

	down_read(&data->sem);
	vaddr = virt_base;
	printk(KERN_ERR "%s start=0x%08lx end=0x%08lx",
	       __func__, (unsigned long)vaddr + offset,
	       (unsigned long)vaddr + pmem_len(data));
	do_cache_op((unsigned long)vaddr + offset,
		    (unsigned long)vaddr + pmem_len(data), 0);
	up_read(&data->sem);
}

static int pmem_connect(unsigned long connect, struct file *file)
{
	struct pmem_data *data = (struct pmem_data *)file->private_data;
	struct pmem_data *src_data;
	struct file *src_file;
	int ret = 0, put_needed;

	down_write(&data->sem);
	/* retrieve the src file and check it is a pmem file with an alloc */
	src_file = fget_light(connect, &put_needed);
	DLOG("connect %p to %p\n", file, src_file);
	if (!src_file) {
		printk("pmem: src file not found!\n");
		ret = -EINVAL;
		goto err_no_file;
	}
	if (unlikely(!is_pmem_file(src_file) || !has_allocation(src_file))) {
		printk(KERN_INFO "pmem: src file is not a pmem file or has no "
		       "alloc!\n");
		ret = -EINVAL;
		goto err_bad_file;
	}

	src_data = (struct pmem_data *)src_file->private_data;

	if (has_allocation(file) && (data->pfn != src_data->pfn)) {
		printk("pmem: file is already mapped but doesn't match this"
		       " src_file!\n");
		ret = -EINVAL;
		goto err_bad_file;
	}

	WARN_ON((src_file->f_flags & O_SYNC) != (file->f_flags & O_SYNC));
	WARN_ON((src_file->f_flags & FASYNC) != (file->f_flags & FASYNC));
	data->pfn = src_data->pfn;
	data->flags |= PMEM_FLAGS_CONNECTED;
	data->master_fd = connect;
	data->size = src_data->size;
	data->master_file = src_file;
	/* Add subdata to the source data list, so we can find it quickly in
	 * pmem_release
	 */
	list_add_tail(&data->sub_data_list, &src_data->sub_data_list);

err_bad_file:
	fput_light(src_file, put_needed);
err_no_file:
	up_write(&data->sem);

	return ret;
}

static void pmem_unlock_data_and_mm(struct pmem_data *data,
				    struct mm_struct *mm)
{
	up_write(&data->sem);
	if (mm != NULL) {
		up_write(&mm->mmap_sem);
		mmput(mm);
	}
}

static int pmem_lock_data_and_mm(struct file *file, struct pmem_data *data,
				 struct mm_struct **locked_mm)
{
	int ret = 0;
	struct mm_struct *mm = NULL;
	*locked_mm = NULL;
lock_mm:
	down_read(&data->sem);
	if (PMEM_IS_SUBMAP(data)) {
		mm = get_task_mm(data->task);
		if (!mm) {
			printk("pmem: can't remap task is gone!\n");
			up_read(&data->sem);
			return -1;
		}
	}
	up_read(&data->sem);

	if (mm)
		down_write(&mm->mmap_sem);

	down_write(&data->sem);
	/* check that the file didn't get mmaped before we could take the
	 * data sem, this should be safe b/c you can only submap each file
	 * once */
	if (PMEM_IS_SUBMAP(data) && !mm) {
		pmem_unlock_data_and_mm(data, mm);
		goto lock_mm;
	}
	/* now check that vma.mm is still there, it could have been
	 * deleted by vma_close before we could get the data->sem */
	if ((data->flags & PMEM_FLAGS_UNSUBMAP) && (mm != NULL)) {
		/* might as well release this */
		if (data->flags & PMEM_FLAGS_SUBMAP) {
			put_task_struct(data->task);
			data->task = NULL;
			/* lower the submap flag to show the mm is gone */
			data->flags &= ~(PMEM_FLAGS_SUBMAP);
		}
		pmem_unlock_data_and_mm(data, mm);
		return -1;
	}
	*locked_mm = mm;
	return ret;
}

int pmem_remap(struct pmem_region *region, struct file *file,
	       unsigned operation)
{
	int ret;
	struct pmem_region_node *region_node;
	struct mm_struct *mm = NULL;
	struct list_head *elt, *elt2;
	int id = get_id(file);
	struct pmem_data *data = (struct pmem_data *)file->private_data;

	/* pmem region must be aligned on a page boundry */
	if (unlikely(!PMEM_IS_PAGE_ALIGNED(region->offset) ||
		     !PMEM_IS_PAGE_ALIGNED(region->len))) {
		printk("pmem: request for unaligned pmem suballocation "
		       "%lx %lx\n", region->offset, region->len);
		return -EINVAL;
	}

	/* if userspace requests a region of len 0, there's nothing to do */
	if (region->len == 0)
		return 0;

	/* lock the mm and data */
	ret = pmem_lock_data_and_mm(file, data, &mm);
	if (ret)
		return 0;

	/* only the owner of the master file can remap the client fds
	 * that back in it */
	if (!is_master_owner(file)) {
		printk("pmem: remap requested from non-master process\n");
		ret = -EINVAL;
		goto err;
	}

	/* check that the requested range is within the src allocation */
	if (unlikely((region->offset > pmem_len(data)) ||
		     (region->len > pmem_len(data)) ||
		     (region->offset + region->len > pmem_len(data)))) {
		printk(KERN_INFO "pmem: suballoc doesn't fit in src_file!\n");
		ret = -EINVAL;
		goto err;
	}

	if (operation == PMEM_MAP) {
		region_node = kmalloc(sizeof(struct pmem_region_node),
				      GFP_KERNEL);
		if (!region_node) {
			ret = -ENOMEM;
			printk(KERN_INFO "No space to allocate metadata!");
			goto err;
		}
		region_node->region = *region;
		list_add(&region_node->list, &data->region_list);
	} else if (operation == PMEM_UNMAP) {
		int found = 0;
		list_for_each_safe(elt, elt2, &data->region_list) {
			region_node = list_entry(elt, struct pmem_region_node,
						 list);
			if (region->len == 0 ||
			    (region_node->region.offset == region->offset &&
			     region_node->region.len == region->len)) {
				list_del(elt);
				kfree(region_node);
				found = 1;
			}
		}
		if (!found) {
			printk("pmem: Unmap region does not map any mapped "
			       "region!");
			ret = -EINVAL;
			goto err;
		}
	}

	if (data->vma && PMEM_IS_SUBMAP(data)) {
		if (operation == PMEM_MAP)
			ret = pmem_remap_pfn_range(id, file, data->vma, data,
						   region->offset, region->len);
		else if (operation == PMEM_UNMAP)
			ret = pmem_unmap_pfn_range(id, data->vma, data,
						   region->offset, region->len);
	}

err:
	pmem_unlock_data_and_mm(data, mm);
	return ret;
}

static void pmem_revoke(struct file *file, struct pmem_data *data)
{
	struct pmem_region_node *region_node;
	struct list_head *elt, *elt2;
	struct mm_struct *mm = NULL;
	int id = get_id(file);
	int ret = 0;

	data->master_file = NULL;
	ret = pmem_lock_data_and_mm(file, data, &mm);
	/* if lock_data_and_mm fails either the task that mapped the fd, or
	 * the vma that mapped it have already gone away, nothing more
	 * needs to be done */
	if (ret)
		return;
	/* unmap everything */
	/* delete the regions and region list nothing is mapped any more */
	if (data->vma) {
		list_for_each_safe(elt, elt2, &data->region_list) {
			region_node = list_entry(elt, struct pmem_region_node,
						 list);
			pmem_unmap_pfn_range(id, data->vma, data,
					     region_node->region.offset,
					     region_node->region.len);
			list_del(elt);
			kfree(region_node);
		}
	}
	/* delete the master file */
	pmem_unlock_data_and_mm(data, mm);
}

static long pmem_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct pmem_data *data;
	int id = get_id(file);
	struct pmem_region region;
	bool should_wait;
	int ret = 0;

	switch (cmd) {
	case PMEM_GET_PHYS:
		DLOG("get_phys\n");
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
	case PMEM_MAP:
		if (copy_from_user(&region, (void __user *)arg,
				   sizeof(struct pmem_region)))
			return -EFAULT;
		data = (struct pmem_data *)file->private_data;
		ret = pmem_remap(&region, file, PMEM_MAP);
		break;
	case PMEM_UNMAP:
		if (copy_from_user(&region, (void __user *)arg,
				   sizeof(struct pmem_region)))
			return -EFAULT;
		data = (struct pmem_data *)file->private_data;
		ret = pmem_remap(&region, file, PMEM_UNMAP);
		break;
	case PMEM_GET_SIZE:
		DLOG("get_size\n");
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
	case PMEM_GET_TOTAL_SIZE:
		DLOG("get total size\n");
		region.offset = 0;
		get_id(file);
		region.len = pmem[id].cma.nr_pages << PAGE_SHIFT;
		if (copy_to_user((void __user *)arg, &region,
				 sizeof(struct pmem_region)))
			return -EFAULT;
		break;
	case PMEM_ALLOCATE:
		if (!has_allocation(file))
			return -EINVAL;
		data = (struct pmem_data *)file->private_data;
		down_write(&data->sem);
		data->flags |= PMEM_FLAGS_DIRTY_REGION;
		up_write(&data->sem);
		break;
	case PMEM_CONNECT:
		DLOG("connect\n");
		ret = pmem_connect(arg, file);
		break;
	case PMEM_CACHE_FLUSH:
		data = (struct pmem_data *)file->private_data;

		if (copy_from_user(&region, (void __user *)arg,
				   sizeof(struct pmem_region)))
			return -EFAULT;

		DLOG("flush with offset=0x%08lx len=0x%08lx\n", region.offset,
		     region.len);
		flush_pmem_file(file, region.offset, region.len);

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

static ssize_t debug_open(struct inode *inode, struct file *file)
{
	file->private_data = inode->i_private;
	return 0;
}

static ssize_t debug_read(struct file *file, char __user * buf, size_t count,
			  loff_t *ppos)
{
	struct list_head *elt, *elt2;
	struct pmem_data *data;
	struct pmem_region_node *region_node;
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
			      "process (pid #) : ref | flags | size | range | mapped regions (start, end) (start, end)...\n");

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
			if (task)
				get_task_struct(task);
			rcu_read_unlock();
			if (task) {
				task_lock(task);
				n += scnprintf(buffer + n, debug_bufmax - n,
					       "%-16s (%6u) :",
					       task->comm, data->pid);
				task_unlock(task);
				put_task_struct(task);
			} else {
				n += scnprintf(buffer + n, debug_bufmax - n,
					       "%-25s :",
					       "non-allocating task");
				up_read(&data->sem);
			}

			size = pmem_len(data);

			n += scnprintf(buffer + n, debug_bufmax - n,
				       "   %08d", data->ref);
			n += scnprintf(buffer + n, debug_bufmax - n,
				       " 0x%08x", data->flags);
			n += scnprintf(buffer + n, debug_bufmax - n,
				       " %08ldkB", size / SZ_1K);

			n += scnprintf(buffer + n, debug_bufmax - n,
				       " (0x%08x-0x%08lx)",
				       PMEM_START_ADDR(data),
				       PMEM_START_ADDR(data) + size);

			list_for_each(elt2, &data->region_list) {
				region_node = list_entry(elt2, struct
							 pmem_region_node,
							 list);
				n += scnprintf(buffer + n, debug_bufmax - n,
					       "(%lx,%lx) ",
					       region_node->region.offset,
					       region_node->region.len);
			}

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

		n += scnprintf(buffer + n, debug_bufmax - n,
			       "HWM     : %u pages, %08ukB\n",
			       pmem[id].hwm,
			       (pmem[id].hwm << PAGE_SHIFT) / SZ_1K);
		n++;

		if (n >= debug_bufmax) {
			buf_order++;
			if (buf_order > 3) {
				printk(KERN_ERR
				       "pmem: user list is too long\n");
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

static int
pmem_task_notify_func(struct notifier_block *self,
		      unsigned long val, void *data)
{
	int id;
	struct task_struct *task = data;

	for (id = 0; id < PMEM_MAX_DEVICES; id++) {
		if (task == pmem[id].deathpending) {
			printk(KERN_INFO
			       "%s: %s(%d) pmem deathpending killed\n",
			       __func__, task->comm, task->pid);
			pmem[id].deathpending = NULL;
			wake_up_all(&pmem[id].deatheaters);
		}
	}

	return NOTIFY_OK;
}

static struct notifier_block pmem_task_nb = {
	.notifier_call = pmem_task_notify_func,
};

/* Can only be called from allocation path after wmark checks are done */
/* Must be called with alloc_stat_lock held */
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

	if (pmem_watermark_ok(p_info) || p_info->deathpending)
		goto out;

	/* Scan the list and find the task with minimum oom_adj value
	 */
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
		if (task) {
			task_lock(task);
			get_task_struct(task);
		}
		rcu_read_unlock();
		up_read(&data->sem);

		if (!task)
			continue;

		if (!task->mm || !task->signal) {
			task_unlock(task);
			put_task_struct(task);
			continue;
		}

		oom_adj = task->signal->oom_adj;
		/* The task is too important to kill */
		if (oom_adj <= 0) {
			task_unlock(task);
			put_task_struct(task);
			continue;
		}

		task_cmasize = get_mm_cma(task->mm);
		task_unlock(task);
		put_task_struct(task);

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
		printk(KERN_INFO
		       "pmem: killing (%s/%d),adj %d,size %lu pages\n",
		       selected->comm, selected->pid, selected_oom_adj,
		       selected_task_cmasize);
		p_info->deathpending = selected;
		force_sig(SIGKILL, selected);
	} else {
		printk(KERN_ALERT "pmem: didn't find suitable task to kill\n");
		goto out;
	}

	/* wait on queue ... */
	wait_event(p_info->deatheaters, (p_info->deathpending == NULL));
out:
	mutex_unlock(&p_info->shrinker_lock);
}

int pmem_setup(struct platform_device *pdev,
	       struct android_pmem_platform_data *pdata)
{
	int err = 0;
	int id = id_count;
	struct page *page;

	id_count++;

	if (pdata->cmasize) {
		get_dev_cma_info(&pdev->dev, &pmem[id].cma);
		BUG_ON(pmem[id].cma.nr_pages != (pdata->cmasize >> PAGE_SHIFT));
		pmem[id].pdev = pdev;
		/* These are only used when we have associated CMA region */
		mutex_init(&pmem[id].shrinker_lock);
		INIT_WORK(&pmem[id].pmem_shrinker, pmem_shrink);
		init_waitqueue_head(&pmem[id].deatheaters);
		pmem[id].deathpending = NULL;
		/*
		 * High watermark is set so we have atleast 10MB of
		 * contiguous block free in our CMA region
		 */
		pmem[id].hwm = (10 * SZ_1M) / PAGE_SIZE;
	} else {
		memset(&pmem[id].cma, 0, sizeof(pmem[id].cma));
	}

	if (pdata->carveout_base && pdata->carveout_size) {
		BUG_ON(pdata->carveout_size & (PAGE_SIZE - 1));
		pmem[id].carveout_vbase =
		    (unsigned long)ioremap_nocache(pdata->carveout_base,
						   pdata->carveout_size);
		if (!pmem[id].carveout_vbase) {
			printk(KERN_ERR "pmem: ioremap failed\n");
		} else {
			pmem[id].pool = gen_pool_create(12, -1);
			if (!pmem[id].pool) {
				printk(KERN_ERR
				       "pmem: genpool_create failed\n");
				iounmap((void *)pmem[id].carveout_vbase);
			} else {
				gen_pool_add(pmem[id].pool,
					     pdata->carveout_base,
					     pdata->carveout_size, -1);
				pmem[id].carveout_base = pdata->carveout_base;
				pmem[id].carveout_size = pdata->carveout_size;
			}
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

	debugfs_create_file(pdata->name, S_IFREG | S_IRUGO, NULL, (void *)id,
			    &debug_fops);

	/* register task free notifier */
	task_free_register(&pmem_task_nb);

	printk(KERN_INFO "Pmem initialised with %lu CMA & %hu hwm pages\n",
	       pmem[id].cma.nr_pages, pmem[id].hwm);
	return 0;

err_cant_register_device:
	__free_page(page);
error:
	if (pmem[id].pool)
		gen_pool_destroy(pmem[id].pool);
	if (pmem[id].carveout_vbase)
		iounmap((void __iomem *)pmem[id].carveout_vbase);
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
	if (pmem[id].carveout_vbase)
		iounmap((void __iomem *)pmem[id].carveout_vbase);

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
module_exit(pmem_exit);
