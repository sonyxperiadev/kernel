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
#define PMEM_FLAGS_SUBMAP	(0x1 << 3)
#define PMEM_FLAGS_UNSUBMAP	(0x1 << 4)

#define PMEM_FLAGS_DIRTY_REGION (0x1 << 5)

struct pmem_data {
	/* in alloc mode: an index into the bitmap
	 * in no_alloc mode: the size of the allocation */
	int index;
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
	/* a lined list of data that has a submap, so we can access them
	 * quickly in pmem_release()
	 */
	struct list_head sub_data_list;

	int ref;
};

struct pmem_bits {
	unsigned allocated:1;		/* 1 if allocated, 0 if free */
	unsigned order:7;		/* size of the region in pmem space */
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
	/* physical start address of the remaped pmem space */
	unsigned long base;
	/* vitual start address of the remaped pmem space */
	unsigned char __iomem *vbase;
	unsigned long size;
	/* number of entries in the pmem space */
	unsigned long num_entries;
	/* pfn of the garbage page in memory */
	unsigned long garbage_pfn;
	/* index of the garbage page in the pmem space */
	int garbage_index;
	/* the bitmap for the region indicating which entries are allocated
	 * and which are free */
	struct pmem_bits *bitmap;
	/* indicates the region should not be managed with an allocator */
	unsigned allocator;
	/* indicates maps of this region should be cached, if a mix of
	 * cached and uncached is desired, set this and open the device with
	 * O_SYNC to get an uncached region */
	unsigned cached;
	unsigned buffered;
	/* mutually exclusive shrinker function */
	struct mutex shrinker_lock;
	/* Work that shrinks pmem allocations */
	struct work_struct pmem_shrinker;
	/* Task killed by the pmem_shrinker */
	struct task_struct *deathpending;
	/* Wait Queue to wait for killed process to die .. */
	wait_queue_head_t deatheaters;
	/* Stats for the CMA region for this device */
	struct dev_cma_stats stats;
	/* high water mark in pages for this pmem space */
	unsigned short hwm;
	/* protects data list */
	struct mutex data_list_lock;
	/* total size of the pmem space */
	struct list_head data_list;
	/* pmem_sem protects the bitmap array
	 * a write lock should be held when modifying entries in bitmap
	 * a read lock should be held when reading data from bits or
	 * dereferencing a pointer into bitmap
	 *
	 * pmem_data->sem protects the pmem data of a particular file
	 * Many of the function that require the pmem_data->sem have a non-
	 * locking version for when the caller is already holding that sem.
	 *
	 * IF YOU TAKE BOTH LOCKS TAKE THEM IN THIS ORDER:
	 * down(pmem_data->sem) => down(bitmap_sem)
	 */
	struct rw_semaphore bitmap_sem;

	long (*ioctl)(struct file *, unsigned int, unsigned long);
	int (*release)(struct inode *, struct file *);
};

static struct pmem_info pmem[PMEM_MAX_DEVICES];
static int id_count;

#define PMEM_IS_FREE(id, index) !(pmem[id].bitmap[index].allocated)
#define PMEM_ORDER(id, index) pmem[id].bitmap[index].order
#define PMEM_BUDDY_INDEX(id, index) (index ^ (1 << PMEM_ORDER(id, index)))
#define PMEM_NEXT_INDEX(id, index) (index + (1 << PMEM_ORDER(id, index)))
#define PMEM_OFFSET(id, index) (index * PMEM_MIN_ALLOC)
#define PMEM_START_ADDR(id, index) (PMEM_OFFSET(id, index) + pmem[id].base)
#define PMEM_LEN(id, index) ((1 << PMEM_ORDER(id, index)) * PMEM_MIN_ALLOC)
#define PMEM_END_ADDR(id, index) (PMEM_START_ADDR(id, index) + \
	PMEM_LEN(id, index))
#define PMEM_START_VADDR(id, index) (PMEM_OFFSET(id, index) + pmem[id].vbase)
#define PMEM_END_VADDR(id, index) (PMEM_START_VADDR(id, index) + \
	PMEM_LEN(id, index))
#define PMEM_REVOKED(data) (data->flags & PMEM_FLAGS_REVOKED)
#define PMEM_IS_PAGE_ALIGNED(addr) (!((addr) & (~PAGE_MASK)))
#define PMEM_IS_SUBMAP(data) ((data->flags & PMEM_FLAGS_SUBMAP) && \
	(!(data->flags & PMEM_FLAGS_UNSUBMAP)))

#define PMEM_CMA_PAGE_INDEX(id, page) (page_to_pfn(page) - __phys_to_pfn(pmem[id].base))
#define PMEM_CMA_START_ADDR(id, index) (pmem[id].base + (index * PAGE_SIZE))
#define PMEM_CMA_START_VADDR(id, index) (pmem[id].vbase + (index * PAGE_SIZE))

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
static unsigned long pmem_len(int, struct pmem_data *);

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

static int has_allocation(struct file *file)
{
	struct pmem_data *data;
	/* check is_pmem_file first if not accessed via pmem_file_ops */

	if (unlikely(!file->private_data))
		return 0;
	data = (struct pmem_data *)file->private_data;
	if (unlikely(data->index < 0))
		return 0;
	return 1;
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
	get_dev_cma_stats(&p_info->pdev->dev, &p_info->stats);

	return (p_info->stats.max_free_block >= p_info->hwm);
}

static void pmem_restore_kernel_mappings(int id, struct pmem_data *data,
					 unsigned long offset, unsigned long len);
static int pmem_cma_free(int id, struct pmem_data *data)
{
	int ret;

	struct page *start_page = phys_to_page(PMEM_CMA_START_ADDR(id, data->index));

	DLOG("index %d\n", data->index);

	pmem_restore_kernel_mappings(id, data, 0, data->size);

	ret = !dma_release_from_contiguous(&pmem[id].pdev->dev, start_page, (data->size >> PAGE_SHIFT));
	if (!ret) {
		if (current->group_leader && current->group_leader->mm)
			add_mm_counter(current->group_leader->mm,
				MM_CMAPAGES, -(data->size >> PAGE_SHIFT));
		data->size = 0;
	}

	return ret;
}

static int pmem_free(int id, int index)
{
	/* caller should hold the write lock on pmem_sem! */
	int buddy, curr = index;
	unsigned long order;

	DLOG("index %d\n", index);

	/* clean up the bitmap, merging any buddies */
	order = PMEM_ORDER(id, curr);
	pmem[id].bitmap[curr].allocated = 0;
	/* find a slots buddy Buddy# = Slot# ^ (1 << order)
	 * if the buddy is also free merge them
	 * repeat until the buddy is not free or end of the bitmap is reached
	 */
	do {
		buddy = PMEM_BUDDY_INDEX(id, curr);
		if (PMEM_IS_FREE(id, buddy) &&
				PMEM_ORDER(id, buddy) == PMEM_ORDER(id, curr)) {
			PMEM_ORDER(id, buddy)++;
			PMEM_ORDER(id, curr)++;
			curr = min(buddy, curr);
		} else {
			break;
		}
	} while (curr < pmem[id].num_entries);

	return 0;
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
			sub_data = list_entry(elt, struct pmem_data, sub_data_list);
			down_read(&sub_data->sem);
			BUG_ON(file != sub_data->master_file);
			if (PMEM_IS_SUBMAP(sub_data)) {
				up_read(&sub_data->sem);
				pmem_revoke(file, sub_data);
			}  else {
				up_read(&sub_data->sem);
			}
			list_del(elt);
		}
	}

	/* if its not a conencted file and it has an allocation, free it */
	if (!(PMEM_FLAGS_CONNECTED & data->flags) && has_allocation(file)) {
		if (pmem[id].allocator == CMA_ALLOC) {
			if (data->ref) {
				printk(KERN_ALERT"%s: (%s) Freeing file(%p) data(%p) with ref (%d)\n",
						__func__, current->group_leader ? current->group_leader->comm : current->comm,
						file, data, data->ref);
			}
			ret = pmem_cma_free(id, data);
		} else {
			down_write(&pmem[id].bitmap_sem);
			ret = pmem_free(id, data->index);
			up_write(&pmem[id].bitmap_sem);
		}
		data->index = -1;
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
	if (pmem[id].release)
		ret = pmem[id].release(inode, file);

	return ret;
}

static int pmem_open(struct inode *inode, struct file *file)
{
	struct pmem_data *data;
	int id = get_id(file);
	int ret = 0;

	DLOG("current %u file %p(%ld)\n", current->pid, file, file_count(file));
	/* setup file->private_data to indicate its unmapped */
	/*  you can only open a pmem device one time */

	/* pmem_open() is invoked by misc_open() which will initialse 
	 * file->private_data to misdevice pointer. 
	 */
	if ((file->private_data != NULL) && (file->private_data != &pmem[id].dev))
		return -1;

	data = kmalloc(sizeof(struct pmem_data), GFP_KERNEL);
	if (!data) {
		printk("pmem: unable to allocate memory for pmem metadata.");
		return -1;
	}
	data->flags = 0;
	data->index = -1;
	data->task = NULL;
	data->vma = NULL;
	data->pid = 0;
	data->master_file = NULL;
	data->size = 0;
	data->ref = 0;

	INIT_LIST_HEAD(&data->region_list);
	INIT_LIST_HEAD(&data->sub_data_list);
	init_rwsem(&data->sem);

	file->private_data = data;
	INIT_LIST_HEAD(&data->list);

	mutex_lock(&pmem[id].data_list_lock);
	list_add_tail(&data->list, &pmem[id].data_list);
	mutex_unlock(&pmem[id].data_list_lock);

	return ret;
}

static unsigned long pmem_order(int id, unsigned long len)
{
	int i;

	len = (len + PMEM_MIN_ALLOC - 1)/PMEM_MIN_ALLOC;
	len--;
	for (i = 0; i < sizeof(len)*8; i++)
		if (len >> i == 0)
			break;
	return i;
}

static int pmem_cma_allocate(int id, unsigned long len)
{
	struct page *page;

	if (len > pmem[id].size) {
		printk(KERN_ERR"Tried to allocate (0x%08lx) from a region of size (0x%08lx\n",
				len, pmem[id].size);
		return -1;
	}

	page = dma_alloc_from_contiguous(&pmem[id].pdev->dev, (len >> PAGE_SHIFT), 0);
	if (!page) {
		printk(KERN_ERR"pmem : cma allocation for size (%ld) failed\n", len);
		return -1;
	}

	BUG_ON(!current->group_leader->mm);
	add_mm_counter(current->group_leader->mm,
				MM_CMAPAGES, (len >> PAGE_SHIFT));
	if (!pmem_watermark_ok(&pmem[id])) {
		schedule_work(&pmem[id].pmem_shrinker);
		wake_up_all(&cleaners);
	}

	return PMEM_CMA_PAGE_INDEX(id, page);
}

static int pmem_allocate(int id, unsigned long len)
{
	/* caller should hold the write lock on pmem_sem! */
	/* return the corresponding pdata[] entry */
	int curr = 0;
	int end = pmem[id].num_entries;
	int best_fit = -1;
	unsigned long order = pmem_order(id, len);

	if (order > PMEM_MAX_ORDER)
		return -1;
	DLOG("order %lx\n", order);

	/* look through the bitmap:
	 * 	if you find a free slot of the correct order use it
	 * 	otherwise, use the best fit (smallest with size > order) slot
	 */
	while (curr < end) {
		if (PMEM_IS_FREE(id, curr)) {
			if (PMEM_ORDER(id, curr) == (unsigned char)order) {
				/* set the not free bit and clear others */
				best_fit = curr;
				break;
			}
			if (PMEM_ORDER(id, curr) > (unsigned char)order &&
					(best_fit < 0 ||
					 PMEM_ORDER(id, curr) < PMEM_ORDER(id, best_fit)))
				best_fit = curr;
		}
		curr = PMEM_NEXT_INDEX(id, curr);
	}

	/* if best_fit < 0, there are no suitable slots,
	 * return an error
	 */
	if (best_fit < 0) {
		printk("pmem: no space left to allocate!\n");
		return -1;
	}

	/* now partition the best fit:
	 * 	split the slot into 2 buddies of order - 1
	 * 	repeat until the slot is of the correct order
	 */
	while (PMEM_ORDER(id, best_fit) > (unsigned char)order) {
		int buddy;
		PMEM_ORDER(id, best_fit) -= 1;
		buddy = PMEM_BUDDY_INDEX(id, best_fit);
		PMEM_ORDER(id, buddy) = PMEM_ORDER(id, best_fit);
	}
	pmem[id].bitmap[best_fit].allocated = 1;

	return best_fit;
}

static pgprot_t pmem_access_prot(struct file *file, pgprot_t vma_prot)
{
	int id = get_id(file);
	if (pmem[id].cached == 0 || file->f_flags & O_SYNC)
		return pgprot_noncached(vma_prot);
#ifdef pgprot_ext_buffered
	else if (pmem[id].buffered)
		return pgprot_ext_buffered(vma_prot);
#endif
	else if (file->f_flags & FASYNC)
		return pgprot_writethrough(vma_prot);
	else
		return pgprot_writeback(vma_prot);
}

static unsigned long pmem_start_addr(int id, struct pmem_data *data)
{
	if (pmem[id].allocator == CMA_ALLOC)
		return PMEM_CMA_START_ADDR(id, data->index);
	else
		return PMEM_START_ADDR(id, data->index);

}

static void *pmem_start_vaddr(int id, struct pmem_data *data)
{
	return pmem_start_addr(id, data) - pmem[id].base + pmem[id].vbase;
}

static unsigned long pmem_len(int id, struct pmem_data *data)
{
	if (unlikely(data->index < 0)) {
		printk(KERN_WARNING"%s() called with negative pmem index\n", __func__);
		return 0;
	}

	if (pmem[id].allocator == CMA_ALLOC)
		return data->size;
	else
		return PMEM_LEN(id, data->index);
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
	pgprot_t prot = *(pgprot_t *)data;

	set_pte_ext(pte, mk_pte(page, prot), 0);
	return 0;
}

/* Taken from __dma_remap */
static void pmem_restore_kernel_mappings(int id, struct pmem_data *data,
					 unsigned long offset, unsigned long len)
{
	unsigned long start;

	start = (unsigned long)PMEM_CMA_START_VADDR(id, data->index) + offset;

	apply_to_page_range(&init_mm, start, len, pmem_update_pte, &pgprot_kernel);

	dsb();
	flush_tlb_kernel_range(start, start + len);
}

/* Taken from __dma_clear_buffer + __dma_remap */
static void pmem_update_kernel_mappings(int id, struct file *file,
				struct pmem_data *data, unsigned long offset,
				unsigned long len)
{
	pgprot_t new_prot = pmem_access_prot(file, pgprot_kernel);
	unsigned long start;

	start = (unsigned long)PMEM_CMA_START_VADDR(id, data->index) + offset;

	/*
	 * Ensure that the allocated pages are flushed and that any data
	 * lurking in the kernel direct-mapped region is invalidated.
	 */
	dmac_flush_range((void *)start, (void *)(start + len));
	outer_flush_range(__pa(start), __pa(start) + len);

	apply_to_page_range(&init_mm, start, len, pmem_update_pte, &new_prot);

	dsb();
	flush_tlb_kernel_range(start, start + len);
}

static inline int should_change_kernel_mappings(int id, struct pmem_data *data)
{
	if ((data->flags & PMEM_FLAGS_MASTERMAP) &&
		(pmem[id].allocator == CMA_ALLOC)) {
		return 1;
	}

	return 0;
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
	printk(KERN_ERR"vma_start=0x%p len=0x%p end=0x%p\n",
			(void *)(vma->vm_start + offset), (void *)len, (void *)end);
	pmem_map_garbage(id, vma, data, offset, len);
	return 0;
}

static int pmem_map_pfn_range(int id, struct file *file, struct vm_area_struct *vma,
			      struct pmem_data *data, unsigned long offset,
			      unsigned long len)
{
	DLOG("map offset %lx len %lx\n", offset, len);
	BUG_ON(!PMEM_IS_PAGE_ALIGNED(vma->vm_start));
	BUG_ON(!PMEM_IS_PAGE_ALIGNED(vma->vm_end));
	BUG_ON(!PMEM_IS_PAGE_ALIGNED(len));
	BUG_ON(!PMEM_IS_PAGE_ALIGNED(offset));

	if (should_change_kernel_mappings(id, data))
		pmem_update_kernel_mappings(id, file, data, offset, len);

	if (io_remap_pfn_range(vma, vma->vm_start + offset,
		(pmem_start_addr(id, data) + offset) >> PAGE_SHIFT,
		len, vma->vm_page_prot)) {
		if (should_change_kernel_mappings(id, data))
			pmem_restore_kernel_mappings(id, data, offset, len);
		return -EAGAIN;
	}

	return 0;
}

static int pmem_remap_pfn_range(int id, struct file *file, struct vm_area_struct *vma,
			      struct pmem_data *data, unsigned long offset,
			      unsigned long len)
{
	unsigned long end;

	/* hold the mm semp for the vma you are modifying when you call this */
	BUG_ON(!vma);

	end = zap_page_range(vma, vma->vm_start + offset, len, NULL);
	if (end != vma->vm_start + offset) {
		printk(KERN_ERR"%s: zap_page_range returned unexpected end (%lu), expected was (%lu)\n",
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
		printk(KERN_ERR"Warning! unmapping the pfn range and remapping it with garbage page\n");
		pmem_unmap_pfn_range(id, vma, data, 0, vma->vm_end - vma->vm_start);
	}
	up_write(&data->sem);
}

static void pmem_vma_close(struct vm_area_struct *vma)
{
	struct file *file = vma->vm_file;
	struct pmem_data *data = file->private_data;

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

	/* the kernel is going to free this vma now anyway */
	up_write(&data->sem);
}

static struct vm_operations_struct vm_ops = {
	.open = pmem_vma_open,
	.close = pmem_vma_close,
};

static bool should_retry_allocation(int id)
{

	if (pmem_watermark_ok(&pmem[id]) ||
			fatal_signal_pending(current)) {
		goto out;
	}

	/* retry only if we have a pending death .. */
	if (pmem[id].deathpending) {
		printk(KERN_INFO"%s: waiting for deathpending!\n", __func__);
		wait_event(pmem[id].deatheaters,
				(pmem[id].deathpending == NULL));
		return true;
	}
out:
	return false;
}

static int pmem_mmap(struct file *file, struct vm_area_struct *vma)
{
	struct pmem_data *data;
	int index = -1;
	unsigned long vma_size =  vma->vm_end - vma->vm_start;
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
			/* if file->private_data == unalloced, alloc*/
			if (pmem[id].allocator == CMA_ALLOC) {
				index = pmem_cma_allocate(id, vma->vm_end - vma->vm_start);
			} else {
				down_write(&pmem[id].bitmap_sem);
				index = pmem_allocate(id, vma->vm_end - vma->vm_start);
				up_write(&pmem[id].bitmap_sem);
			}
		} while (((index < 0) && should_retry_allocation(id)));

		if (index < 0)  {
			printk(KERN_ERR"pmem: could not find allocation for map.\n");
			ret = -ENOMEM;
			goto error_up_write;
		}

		data->index = index;
		data->size = vma->vm_end - vma->vm_start;
	}

	if (pmem_len(id, data) < vma_size) {
		printk(KERN_WARNING "pmem: mmap size [%lu] does not match"
		       "size of backing region [%lu].\n", vma_size,
		       pmem_len(id, data));
		ret = -EINVAL;
		goto error_up_write;
	}


	vma->vm_pgoff = pmem_start_addr(id, data) >> PAGE_SHIFT;
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
		printk(KERN_ERR"pmem_mmap() failed, freeing allocated memory\n");
		if (pmem[id].allocator == CMA_ALLOC) {
			ret = pmem_cma_free(id, data);
			if (ret)
				printk(KERN_ALERT"pmem: Unable to free allocated memory during error handling\n");
		} else {
			down_write(&pmem[id].bitmap_sem);
			ret = pmem_free(id, data->index);
			up_write(&pmem[id].bitmap_sem);
		}
		data->index = -1;
	}
error_up_write:
	up_write(&data->sem);
	return ret;
error:
	return ret;

}

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

int get_pmem_addr(struct file *file, unsigned long *start,
		  unsigned long *vstart, unsigned long *len)
{
	struct pmem_data *data;
	int id;

	if (!is_pmem_file(file) || !has_allocation(file)) {
		return -1;
	}

	data = (struct pmem_data *)file->private_data;
	if (data->index == -1) {
#if PMEM_DEBUG
		printk(KERN_INFO "pmem: requested pmem data from file with no "
		       "allocation.\n");
		return -1;
#endif
	}
	id = get_id(file);

	down_read(&data->sem);
	*start = pmem_start_addr(id, data);
	*len = pmem_len(id, data);
	*vstart = (unsigned long)pmem_start_vaddr(id, data);
	up_read(&data->sem);
#if PMEM_DEBUG
	down_write(&data->sem);
	data->ref++;
	up_write(&data->sem);
#endif
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
		return -1;
	}

	if (get_pmem_addr(file, start, vstart, len))
		goto end;

	if (filp)
		*filp = file;
	return 0;
end:
	fput(file);
	return -1;
}

void put_pmem_file(struct file *file)
{
	struct pmem_data *data;
	int id;

	if (!is_pmem_file(file))
		return;
	id = get_id(file);
	data = (struct pmem_data *)file->private_data;
#if PMEM_DEBUG
	down_write(&data->sem);
	if (data->ref == 0) {
		printk("pmem: pmem_put > pmem_get %s (pid %d)\n",
		       pmem[id].dev.name, data->pid);
		BUG();
	}
	data->ref--;
	up_write(&data->sem);
#endif
	fput(file);
}

void flush_pmem_file(struct file *file, unsigned long offset, unsigned long len)
{
	struct pmem_data *data;
	int id;
	void *vaddr;
	struct pmem_region_node *region_node;
	struct list_head *elt;
	void *flush_start, *flush_end;

	if (!is_pmem_file(file) || !has_allocation(file)) {
		return;
	}

	id = get_id(file);
	data = (struct pmem_data *)file->private_data;
	if (!pmem[id].cached || file->f_flags & O_SYNC || file->f_flags & FASYNC)
		return;

	down_read(&data->sem);
	vaddr = pmem_start_vaddr(id, data);
	/* if this isn't a submmapped file, flush the whole thing */
	if (unlikely(!(data->flags & PMEM_FLAGS_CONNECTED))) {
		dmac_flush_range(vaddr, vaddr + pmem_len(id, data));
		outer_flush_range(pmem_start_addr(id, data), pmem_start_addr(id, data)+pmem_len(id, data));
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
			break;
		}
	}
end:
	up_read(&data->sem);
}

void invalidate_pmem_file(struct file *file, unsigned long offset, unsigned long len)
{
	struct pmem_data *data;
	int id;
	void *vaddr;
	phys_addr_t paddr, invalidate_start, invalidate_end;
	struct pmem_region_node *region_node;
	struct list_head *elt;

	if (!is_pmem_file(file) || !has_allocation(file)) {
		return;
	}

	id = get_id(file);
	data = (struct pmem_data *)file->private_data;
	if (!pmem[id].cached || file->f_flags & O_SYNC)
		return;

	down_read(&data->sem);
	vaddr = pmem_start_vaddr(id, data);
	paddr = pmem_start_addr(id, data);
	/* if this isn't a submmapped file, invalidate the whole thing */
	if (unlikely(!(data->flags & PMEM_FLAGS_CONNECTED))) {
		outer_inv_range(paddr, paddr + pmem_len(id, data));
		dmac_unmap_area(vaddr, pmem_len(id, data), DMA_FROM_DEVICE);
		goto end;
	}

	/* otherwise, invalidate the region of the file we are drawing */
	list_for_each(elt, &data->region_list) {
		region_node = list_entry(elt, struct pmem_region_node, list);
		if ((offset >= region_node->region.offset) &&
		    ((offset + len) <= (region_node->region.offset +
			region_node->region.len))) {
			invalidate_start = paddr + region_node->region.offset;
			invalidate_end = invalidate_start + region_node->region.len;
			outer_inv_range(invalidate_start, invalidate_end);
			dmac_unmap_area(vaddr + region_node->region.offset,
					region_node->region.len, DMA_FROM_DEVICE);
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

void flush_pmem_process_file(struct file *file, void *virt_base, unsigned long offset)
{
	struct pmem_data *data;
	int id;
	void *vaddr;

	if (!is_pmem_file(file) || !has_allocation(file)) {
		return;
	}

	id = get_id(file);
	data = (struct pmem_data *)file->private_data;
	if (!pmem[id].cached || file->f_flags & O_SYNC || file->f_flags & FASYNC)
		return;

	down_read(&data->sem);
	vaddr = virt_base;
	printk(KERN_ERR "%s start=0x%08lx end=0x%08lx", __func__, (unsigned long)vaddr + offset, (unsigned long)vaddr + pmem_len(id, data));
	do_cache_op((unsigned long)vaddr + offset, (unsigned long)vaddr + pmem_len(id, data), 0);
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

	if (has_allocation(file) && (data->index != src_data->index)) {
		printk("pmem: file is already mapped but doesn't match this"
		       " src_file!\n");
		ret = -EINVAL;
		goto err_bad_file;
	}

	WARN_ON((src_file->f_flags & O_SYNC) != (file->f_flags & O_SYNC));
	WARN_ON((src_file->f_flags & FASYNC) != (file->f_flags & FASYNC));
	data->index = src_data->index;
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
	if (unlikely((region->offset > pmem_len(id, data)) ||
		     (region->len > pmem_len(id, data)) ||
		     (region->offset + region->len > pmem_len(id, data)))) {
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
	if (data->vma)
		list_for_each_safe(elt, elt2, &data->region_list) {
			region_node = list_entry(elt, struct pmem_region_node,
						 list);
			pmem_unmap_pfn_range(id, data->vma, data,
					     region_node->region.offset,
					     region_node->region.len);
			list_del(elt);
			kfree(region_node);
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
			region.offset = pmem_start_addr(id, data);
			region.len = pmem_len(id, data);
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
		region.len = pmem[id].size;
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

		DLOG("flush with offset=0x%08lx len=0x%08lx \n", region.offset, region.len);
		flush_pmem_file(file, region.offset, region.len);

		break;
	case PMEM_CACHE_INVALIDATE:
		data = (struct pmem_data *)file->private_data;

		if (copy_from_user(&region, (void __user *)arg,
					sizeof(struct pmem_region)))
			return -EFAULT;
		DLOG("Invalidate with offset=0x%08lx len=0x%08lx \n", region.offset, region.len);
		invalidate_pmem_file(file, region.offset, region.len);
		break;
	case PMEM_CLEANER_WAIT:
		data = (struct pmem_data *)file->private_data;

		should_wait = pmem_watermark_ok(&pmem[id]);

		if (should_wait) {
			ret = wait_event_interruptible(cleaners, !pmem_watermark_ok(&pmem[id]));
			ret = -ERESTARTSYS;
			if (ret == -ERESTARTSYS)
				ret = -EAGAIN;
		}
		break;
	default:
		if (pmem[id].ioctl)
			return pmem[id].ioctl(file, cmd, arg);
		ret = -EINVAL;
	}

	return ret;
}

static ssize_t debug_open(struct inode *inode, struct file *file)
{
	file->private_data = inode->i_private;
	return 0;
}

static ssize_t debug_read(struct file *file, char __user *buf, size_t count,
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
	int n, ret;


	for (;;) {
		n = 0;
		debug_bufmax = PAGE_SIZE * (1 << buf_order);
		buffer = kmalloc(debug_bufmax, GFP_KERNEL);
		if (!buffer)
			return -ENOMEM;

		n = scnprintf(buffer, debug_bufmax,
				"process (pid #) : ref | flags | size | range | mapped regions (start, end) (start, end)...\n");

		mutex_lock(&pmem[id].data_list_lock);
		list_for_each(elt, &pmem[id].data_list) {
			data = list_entry(elt, struct pmem_data, list);
			down_read(&data->sem);
			rcu_read_lock();
			task = find_task_by_pid_ns(data->pid, &init_pid_ns);
			if (task)
				get_task_struct(task);
			rcu_read_unlock();
			if (!task) {
				up_read(&data->sem);
				continue;
			}

			task_lock(task);
			n += scnprintf(buffer + n, debug_bufmax - n, "%-16s (%6u) :",
					task->comm, data->pid);
			task_unlock(task);
			put_task_struct(task);

			n += scnprintf(buffer + n, debug_bufmax - n, "   %08d",
					data->ref);
			n += scnprintf(buffer + n, debug_bufmax - n, " 0x%08x",
					data->flags);
			n += scnprintf(buffer + n, debug_bufmax - n, " %08ldkB",
					pmem_len(id, data) / SZ_1K);

			if (pmem[id].allocator == CMA_ALLOC) {
				n += scnprintf(buffer + n, debug_bufmax - n, " (0x%08lx-0x%08lx)",
						PMEM_CMA_START_ADDR(id, data->index),
						PMEM_CMA_START_ADDR(id, data->index) + data->size);
			} else {
				n += scnprintf(buffer + n, debug_bufmax - n, " (0x%08lx-0x%08lx)",
						PMEM_START_ADDR(id, data->index),
						PMEM_END_ADDR(id, data->index));
			}
			list_for_each(elt2, &data->region_list) {
				region_node = list_entry(elt2, struct pmem_region_node,
						list);
				n += scnprintf(buffer + n, debug_bufmax - n,
						"(%lx,%lx) ",
						region_node->region.offset,
						region_node->region.len);
			}

			n += scnprintf(buffer + n, debug_bufmax - n, "\n");

			up_read(&data->sem);
		}

		get_dev_cma_stats(&pmem[id].pdev->dev, &pmem[id].stats);

		n += scnprintf(buffer + n, debug_bufmax - n, "=========================================================\n");
		n += scnprintf(buffer + n, debug_bufmax - n, "Total Allocation : %lu pages, %08lukB\n", pmem[id].stats.total_alloc,
										(pmem[id].stats.total_alloc << PAGE_SHIFT)/SZ_1K);
		mutex_unlock(&pmem[id].data_list_lock);
		n += scnprintf(buffer + n, debug_bufmax - n, "High Watermark : %u pages, %08ukB\n", pmem[id].hwm,
										(pmem[id].hwm << PAGE_SHIFT)/SZ_1K);
		n++;

		if (n >= debug_bufmax) {
			buf_order++;
			if (buf_order > 3) {
				printk(KERN_WARNING"pmem allocation list is too long, and doesn't fit in 8 pages\n");
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
pmem_task_notify_func(struct notifier_block *self, unsigned long val, void *data)
{
	int id;
	struct task_struct *task = data;

	for (id = 0; id < PMEM_MAX_DEVICES; id++) {
		/* only if valid pmem device */
		if (pmem[id].base) {
			if (task == pmem[id].deathpending) {
				printk(KERN_INFO"%s: %s(%d) pmem deathpending killed\n",
						__func__, task->comm, task->pid);
				pmem[id].deathpending = NULL;
				wake_up_all(&pmem[id].deatheaters);
			}
		}
	}

	return NOTIFY_OK;
}

static struct notifier_block pmem_task_nb = {
	.notifier_call	= pmem_task_notify_func,
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
		if (task)
			task_lock(task);
		rcu_read_unlock();
		up_read(&data->sem);
		/* skip if no signal and oom_adj is 0 or less
		 * if (oom_adj <= 0) means the task is either too
		 * important or at the forground
		 */
		if (!task->mm || !task->signal) {
			task_unlock(task);
			continue;
		}

		oom_adj = task->signal->oom_adj;
		/* The task is too important to kill */
		if (oom_adj <= 0) {
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
		printk(KERN_INFO"%s:%d killing \"%s\"(%d), adj %d, size %lu pages\n",
				__func__, current->pid, selected->comm, selected->pid, selected_oom_adj,
				selected_task_cmasize);
		p_info->deathpending = selected;
		force_sig(SIGKILL, selected);
	} else {
		printk(KERN_ALERT"%s: didn't find a suitable task to kill\n", __func__);
		goto out;
	}

	/* wait on queue ...*/
	wait_event(p_info->deatheaters, (p_info->deathpending == NULL));
out:
	mutex_unlock(&p_info->shrinker_lock);
}

int pmem_setup(struct platform_device *pdev,
	       struct android_pmem_platform_data *pdata,
	       long (*ioctl)(struct file *, unsigned int, unsigned long),
	       int (*release)(struct inode *, struct file *))
{
	int err = 0;
	int i, index = 0;
	int id = id_count;

	id_count++;

	if ((pdata->allocator != CMA_ALLOC) &&
		(pdata->allocator != DEFAULT_ALLOC)) {
		printk(KERN_ERR"%s: ##### pmem allocator(%d) is not supported, FAILED #####\n",
					__func__, pdata->allocator);
		goto err_cant_register_device;
	}

	pmem[id].allocator = pdata->allocator;
	pmem[id].cached = pdata->cached;
	pmem[id].buffered = pdata->buffered;
	if (pmem[id].allocator == CMA_ALLOC) {
		/* Verify that the CMA region creation was successfull,
		 * otherwise return immediately */
		get_cma_area(&pdev->dev, (phys_addr_t *)&pmem[id].base, &pmem[id].size);

		if ((pmem[id].size != pdata->size) || (pmem[id].base != pdata->start)) {
			printk(KERN_ERR"%s : Device CMA region (0x%08lx + 0x%08lx) does"
					"not match platform data (0x%08lx + 0x%08lx)\n",
					__func__, pmem[id].base, pmem[id].size,
					pdata->start, pdata->size);
			printk(KERN_ERR"PMEM probe failed\n");
			goto err_cant_register_device;
		}

		WARN_ON(!pmem[id].cached);

	} else {
		pmem[id].base = pdata->start;
		pmem[id].size = pdata->size;
	}

	pmem[id].pdev = pdev;
	pmem[id].ioctl = ioctl;
	pmem[id].release = release;
	init_rwsem(&pmem[id].bitmap_sem);
	mutex_init(&pmem[id].data_list_lock);
	INIT_LIST_HEAD(&pmem[id].data_list);
	mutex_init(&pmem[id].shrinker_lock);
	INIT_WORK(&pmem[id].pmem_shrinker, pmem_shrink);
	init_waitqueue_head(&pmem[id].deatheaters);
	pmem[id].dev.name = pdata->name;
	pmem[id].dev.minor = id;
	pmem[id].dev.fops = &pmem_fops;
	pmem[id].deathpending = NULL;
	printk(KERN_INFO "%s: %d init\n", pdata->name, pdata->cached);

	err = misc_register(&pmem[id].dev);
	if (err) {
		printk(KERN_ALERT "Unable to register pmem driver!\n");
		goto err_cant_register_device;
	}

	if (pmem[id].allocator == CMA_ALLOC) {
		pmem[id].num_entries = pmem[id].size / PAGE_SIZE;
		pmem[id].vbase = phys_to_virt(pmem[id].base);
	} else {
		pmem[id].num_entries = pmem[id].size / PMEM_MIN_ALLOC;

		pmem[id].bitmap = kmalloc(pmem[id].num_entries *
				sizeof(struct pmem_bits), GFP_KERNEL);
		if (!pmem[id].bitmap)
			goto err_no_mem_for_metadata;

		memset(pmem[id].bitmap, 0, sizeof(struct pmem_bits) *
				pmem[id].num_entries);

		for (i = sizeof(pmem[id].num_entries) * 8 - 1; i >= 0; i--) {
			if ((pmem[id].num_entries) &  1<<i) {
				PMEM_ORDER(id, index) = i;
				index = PMEM_NEXT_INDEX(id, index);
			}
		}

		if (pmem[id].cached)
			pmem[id].vbase = ioremap_cached(pmem[id].base,
					pmem[id].size);
		else
			pmem[id].vbase = ioremap(pmem[id].base, pmem[id].size);

		if (pmem[id].vbase == 0)
			goto error_cant_remap;
	}

	if ((pmem[id].size/PAGE_SIZE) > USHRT_MAX) {
		printk(KERN_ERR"[pmem] region size of (%lukB) is not supported,	maximum pmem region can be (%lukB)\n",
				pmem[id].size / SZ_1K,
				USHRT_MAX * PAGE_SIZE / SZ_1K);
		goto error_cant_remap;
	}

	memset(&pmem[id].stats, 0, sizeof(struct dev_cma_stats));
	/* High watermark is set so we atleast have 16 MB of largest free block
	 * */
	pmem[id].hwm = (10 * SZ_1M)/PAGE_SIZE;
	pmem[id].garbage_pfn = page_to_pfn(alloc_page(GFP_KERNEL));

	debugfs_create_file(pdata->name, S_IFREG | S_IRUGO, NULL, (void *)id,
			    &debug_fops);

	/* register task free notifier */
	task_free_register(&pmem_task_nb);

	printk(KERN_INFO"Pmem driver initialised with (%s) allocator with size = %lu pages, hwm = %u pages\n",
			pmem[id].allocator == CMA_ALLOC ? "CMA" : "Buddy", pmem[id].size/PAGE_SIZE, pmem[id].hwm);
	return 0;
error_cant_remap:
	kfree(pmem[id].bitmap);
err_no_mem_for_metadata:
	misc_deregister(&pmem[id].dev);
err_cant_register_device:
	return -1;
}

static int pmem_probe(struct platform_device *pdev)
{
	struct android_pmem_platform_data *pdata;

	if (!pdev || !pdev->dev.platform_data) {
		printk(KERN_ALERT "Unable to probe pmem!\n");
		return -1;
	}
	pdata = pdev->dev.platform_data;
	return pmem_setup(pdev, pdata, NULL, NULL);
}


static int pmem_remove(struct platform_device *pdev)
{
	int id = pdev->id;
	__free_page(pfn_to_page(pmem[id].garbage_pfn));
	misc_deregister(&pmem[id].dev);
	return 0;
}

static struct platform_driver pmem_driver = {
	.probe = pmem_probe,
	.remove = pmem_remove,
	.driver = { .name = "android_pmem" }
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

