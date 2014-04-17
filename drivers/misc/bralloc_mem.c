#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/mm.h>
#include <linux/list.h>
#include <linux/debugfs.h>
#include <linux/mempolicy.h>
#include <linux/sched.h>
#include <linux/semaphore.h>
#include <linux/dma-mapping.h>
#include <linux/dma-contiguous.h>
#include <linux/oom.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/module.h>

#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/cacheflush.h>
#include <asm/tlbflush.h>


/* Uncomment this to see file reference count in the *DEBUG* log
 * #define FOR_DEBUG
 */

#define bralloc_debug(fmt, args...)	\
	pr_debug("%s:%d (%d) " fmt, __func__, __LINE__, current->pid, ##args)

#define bralloc_error(fmt, args...)	\
	pr_err("%s:%d (%d) " fmt, __func__, __LINE__, current->pid, ##args)

#define bralloc_info(fmt, args...)	\
	pr_info("%s:%d (%d) " fmt, __func__, __LINE__, current->pid, ##args)

#define BRALLOC_FLAGS_OPENED	(1 << 0)
#define BRALLOC_FLAGS_ALLOCATED	(1 << 1)
#define BRALLOC_FLAGS_MAPPED	(1 << 2)

#define BRALLOC_DMA_COHERENT_ALLOC	(1 << 4)
#define BRALLOC_DMA_WRITECOMBINE_ALLOC	(1 << 5)
#define BRALLOC_WRITETHROUGH_ALLOC	(1 << 6)
#define BRALLOC_WRITEBACK_ALLOC		(1 << 7)
#define BRALLOC_ALLOC_TYPE_MASK		(0xf << 4)

#define BRALLOC_GET_ALLOC_TYPE(flags)	(flags & BRALLOC_ALLOC_TYPE_MASK)

#define IS_PAGE_ALIGNED(addr) (!((addr) & (~PAGE_MASK)))

/* Macros to set pgprot values to match write-through
 * and write-back cache bits
 **/

#define pgprot_writethrough(prot) \
	__pgprot((pgprot_val(prot) & ~L_PTE_MT_MASK) | L_PTE_MT_WRITETHROUGH)

/* Always assume writeback mappings are going to be used for
 * ACP, so they must also have their shared bit set
 **/
#define pgprot_writeback(prot) \
	__pgprot((pgprot_val(prot) & ~L_PTE_MT_MASK) | L_PTE_MT_WRITEBACK | L_PTE_SHARED)

typedef struct bralloc {
	struct miscdevice dev;
	struct platform_device *pdev;
	/* this page frame will be mapped in place of the actual device in the child,
	 * when a process forks children after mmaping this device
	 * "pmem" does the exact same thing
	 */
	unsigned long garbage_pfn;
#ifdef CONFIG_BRALLOC_MEM_STATS
	spinlock_t lock;
	uint32_t total_open;
	struct list_head brlist;
#endif
} bralloc_t;

typedef struct bralloc_data {
#ifdef CONFIG_BRALLOC_MEM_STATS
	struct list_head brlink;
	unsigned int map_count;
#endif
	/* Details of DMA allocation */
	struct page *start_page;
	void *kaddr;
	size_t size;
	dma_addr_t dma_handle;
	phys_addr_t phys_addr;

	/* Details of userspace allocations, do we need this? */
	struct vm_area_struct *vma;

	/* Flags maintained by the driver to update information
	 * about the stages this allocation goes through */
	int flags;

	/* Lock for bralloc_data */
	struct rw_semaphore sem;
#ifdef FOR_DEBUG
	struct file *file;
#endif

	/* Task that made this allocation
	 * We may use this for rSS accounting
	 **/
	struct task_struct *tsk;
	pid_t pid;

} bralloc_data_t;

typedef struct bralloc_region {
	unsigned long offset;
	unsigned long size;
} bralloc_region_t;

#define BRALLOC_GET_PHYS		_IOR('B', 0x21, bralloc_region_t)
#define BRALLOC_GET_SIZE		_IOR('B', 0x22, bralloc_region_t)
#define BRALLOC_GET_TOTAL_SIZE		_IOR('B', 0x23, bralloc_region_t)
#define BRALLOC_CACHE_FLUSH		_IOW('B', 0x24, bralloc_region_t)
#define BRALLOC_CACHE_INVALIDATE	_IOW('B', 0x25, bralloc_region_t)

static bralloc_t br;

static int has_allocation(struct file *file)
{
	bralloc_data_t *data;

	if (unlikely(!file->private_data))
		return 0;

	data = (bralloc_data_t *)file->private_data;

	if (unlikely(!(data->flags & BRALLOC_FLAGS_ALLOCATED)))
		return 0;

	return 1;
}

/* Taken from __dma_clear_buffer */
static inline void bralloc_mem_clear_buffer(void *ptr, size_t size)
{
	memset(ptr, 0, size);
	dmac_flush_range(ptr, ptr + size);
	outer_flush_range(__pa(ptr), __pa(ptr) + size);
}

/* Taken from __dma_update_pte */
static int bralloc_mem_update_pte(pte_t *pte, pgtable_t token, unsigned long addr,
			    void *data)
{
	struct page *page = virt_to_page(addr);
	pgprot_t prot = *(pgprot_t *)data;

	set_pte_ext(pte, mk_pte(page, prot), 0);
	return 0;
}

/* Taken from __dma_remap */
static void bralloc_mem_kernel_remap(struct page *page, size_t size, pgprot_t prot)
{
	unsigned long start = (unsigned long)page_address(page);
	unsigned end = start + size;

	apply_to_page_range(&init_mm, start, size, bralloc_mem_update_pte, &prot);
	dsb();
	flush_tlb_kernel_range(start, end);

}

/* must be called with write lock held on data->sem */
static int bralloc_mem_allocate(struct file *file, bralloc_data_t *data, unsigned long size)
{
	int alloc_flags;
	int ret = 0;

	bralloc_debug("Enter ..\n");

	/* Find what type of allocation and mapping we need to do, as they all
	 * need to be consistent with the mmaps we will be creating for
	 * userspace
	 */

	if (file->f_flags & O_SYNC) {

		alloc_flags = BRALLOC_DMA_COHERENT_ALLOC;

		/* Create uncached allocation, we still need to see if they
		 * need to be write-buffered or not, but we'll add in something
		 * later for that. For now, uncached allocations are coherent
		 * allocations.
		 */

		/* Allocate coherent (uncached, strongly ordered) contiguous dma memory here */
		data->kaddr = dma_alloc_coherent(&br.pdev->dev, size, &data->dma_handle, GFP_KERNEL);
		if (!data->kaddr) {
			bralloc_error("Failed to allocate contiguous DMA memory for (%d/%d), size(%lu)\n",
					current->pid, current->tgid, size);
			ret = -ENOMEM;
			goto done;
		}

		data->start_page = virt_to_page(data->kaddr);

	} else if (file->f_flags & O_DIRECT) {

		alloc_flags = BRALLOC_DMA_WRITECOMBINE_ALLOC;

		/* If file is opened with O_DIRECT we consider this writecombine
		 * allocation+mapping (using write buffers). Userspace *must*
		 * lock/unlock region before using it. For us, "lock" means
		 * do nothing, "unlock" means drain write buffer
		 */

		/* Allocate coherent (uncached, strongly ordered) contiguous dma memory here */
		data->kaddr = dma_alloc_writecombine(&br.pdev->dev, size, &data->dma_handle, GFP_KERNEL);
		if (!data->kaddr) {
			bralloc_error("Failed to allocate contiguous DMA memory for (%d/%d), size(%lu)\n",
					current->pid, current->tgid, size);
			ret = -ENOMEM;
			goto done;
		}

		data->start_page = virt_to_page(data->kaddr);

	} else if (file->f_flags & FASYNC) {

		alloc_flags = BRALLOC_WRITETHROUGH_ALLOC;

		/* If file is opened with O_ASYNC, we allocate+map using the
		 * Write-Through cached mappings. This means for all files
		 * opened with O_ASYNC, userspace must call lock/unlock ioctls
		 * before using the memory region. For us, "lock" means
		 * "invalidate cachelines" and "unlock" and do nothing
		 */

		data->start_page = dma_alloc_from_contiguous(&br.pdev->dev, (size >> PAGE_SHIFT),
								CONFIG_CMA_ALIGNMENT);
		if (!data->start_page) {
			bralloc_error("Failed to allocate contiguous DMA memory for (%d/%d), size(%lu)\n",
					current->pid, current->tgid, size);
			ret = -ENOMEM;
			goto done;
		}

		data->kaddr = page_address(data->start_page);

		/* We must clear the buffer and remap it with WT cachebility to
		 * make sure the userspace mappings and kernel mappings are
		 * identical in terms of memory attributes and cacheability,
		 * otherwise unthinkable things can happen according to ARM TRM
		 */
		bralloc_mem_clear_buffer(data->kaddr, size);
		bralloc_mem_kernel_remap(data->start_page, size, pgprot_writethrough(pgprot_kernel));
		data->dma_handle = pfn_to_dma(&br.pdev->dev, page_to_pfn(data->start_page));

	} else {

		alloc_flags = BRALLOC_WRITEBACK_ALLOC; /* for now .. */

		/* This is the default allocation+mapping type if none of the
		 * flags above are specified during open(). we map it
		 * as Write-Back cached. This means, userspace must lock/unlock
		 * the region before accessing it. For us, "lock" means
		 * "invalidate cachelines" and "unlock" means flush cachelines
		 */
		data->start_page = dma_alloc_from_contiguous(&br.pdev->dev, (size >> PAGE_SHIFT),
								CONFIG_CMA_ALIGNMENT);
		if (!data->start_page) {
			bralloc_error("Failed to allocate contiguous DMA memory for (%d/%d), size(%lu)\n",
					current->pid, current->tgid, size);
			ret = -ENOMEM;
			goto done;
		}

		data->kaddr = page_address(data->start_page);

		/* We must clear the buffer and remap it with WT cachebility to
		 * make sure the userspace mappings and kernel mappings are
		 * identical in terms of memory attributes and cacheability,
		 * otherwise unthinkable things can happen according to ARM TRM
		 */
		bralloc_mem_clear_buffer(data->kaddr, size);
		bralloc_mem_kernel_remap(data->start_page, size, pgprot_writeback(pgprot_kernel));
		data->dma_handle = pfn_to_dma(&br.pdev->dev, page_to_pfn(data->start_page));
	}


	data->phys_addr = page_to_pfn(data->start_page) <<  PAGE_SHIFT;
	data->size = size;
	data->flags |= BRALLOC_FLAGS_ALLOCATED;
	data->flags |= alloc_flags;
	data->tsk = current;
	data->pid = current->pid;

	bralloc_debug("Exit ..\n");

done:
	return ret;
}

/* Must be called with data->sem held for writing */
static void bralloc_mem_free(bralloc_data_t *data)
{

	bralloc_debug("Enter ..\n");

	/* Check for flags to see what kind of allocation is this
	 * and then free it up accordingly */

	switch(BRALLOC_GET_ALLOC_TYPE(data->flags)) {
	case BRALLOC_DMA_COHERENT_ALLOC:
		dma_free_coherent(&br.pdev->dev, data->size, data->kaddr, data->dma_handle);
		break;
	case BRALLOC_DMA_WRITECOMBINE_ALLOC:
		dma_free_writecombine(&br.pdev->dev, data->size, data->kaddr, data->dma_handle);
		break;
	case BRALLOC_WRITETHROUGH_ALLOC:
	case BRALLOC_WRITEBACK_ALLOC:
		bralloc_mem_kernel_remap(data->start_page, data->size, pgprot_kernel);
		dma_release_from_contiguous(&br.pdev->dev, data->start_page, data->size >> PAGE_SHIFT);
		break;
	default:
		bralloc_error("Invalid allocation type, nothing will be freed\n");
		BUG();
	}

	data->size = 0;
	data->kaddr = data->start_page = NULL;
	data->dma_handle = -1;
	data->flags &= ~(BRALLOC_FLAGS_ALLOCATED | BRALLOC_ALLOC_TYPE_MASK);

	bralloc_debug("Exit ..\n");
}

static int bralloc_map_pfn_range(bralloc_data_t *data, struct vm_area_struct *vma,
				unsigned long offset, unsigned long size)
{
	unsigned long pfn;
	int ret = 0;

	BUG_ON(!IS_PAGE_ALIGNED(vma->vm_start) ||
		!IS_PAGE_ALIGNED(vma->vm_end) ||
		!IS_PAGE_ALIGNED(size) ||
		!IS_PAGE_ALIGNED(offset));

	/* decide how we are going to mmap this */
	switch(BRALLOC_GET_ALLOC_TYPE(data->flags)) {
	case BRALLOC_DMA_COHERENT_ALLOC:
		ret = dma_mmap_coherent(&br.pdev->dev, vma, data->kaddr,
						data->dma_handle, size);

		break;
	case BRALLOC_DMA_WRITECOMBINE_ALLOC:
		ret = dma_mmap_writecombine(&br.pdev->dev, vma,	data->kaddr,
						data->dma_handle, size);
		break;
	case BRALLOC_WRITETHROUGH_ALLOC:
		vma->vm_page_prot = pgprot_writethrough(vma->vm_page_prot);
		pfn = __phys_to_pfn(data->phys_addr);

		ret = remap_pfn_range(vma, vma->vm_start,
					pfn + vma->vm_pgoff,
					vma->vm_end - vma->vm_start,
					vma->vm_page_prot);
		break;
	case BRALLOC_WRITEBACK_ALLOC:
		vma->vm_page_prot = pgprot_writeback(vma->vm_page_prot);
		pfn = __phys_to_pfn(data->phys_addr);

		ret = remap_pfn_range(vma, vma->vm_start,
					pfn + vma->vm_pgoff,
					vma->vm_end - vma->vm_start,
					vma->vm_page_prot);
		break;
	default:
		BUG();
	}

	return ret;
}

static void bralloc_mem_flush(struct file *file, unsigned long offset, unsigned long size)
{
	bralloc_data_t *data;

	if (!has_allocation(file)) {
		bralloc_error("User (%d) tried to CACHE_FLUSH without allocation\n", current->pid);
		return;
	}

	data = (bralloc_data_t *)file->private_data;

	down_read(&data->sem);

	BUG_ON(!data || !(data->flags & BRALLOC_FLAGS_MAPPED));


	switch(BRALLOC_GET_ALLOC_TYPE(data->flags)) {
	case BRALLOC_DMA_COHERENT_ALLOC:
		bralloc_debug("Allocation is dma coherent (uncached), no flush required\n");
		break;
	case BRALLOC_DMA_WRITECOMBINE_ALLOC:
		/* drain write buffer */
		dsb();
		break;
	case BRALLOC_WRITETHROUGH_ALLOC:
		bralloc_debug("Allocation is Cache Writethrough, no flush required\n");
		break;
	case BRALLOC_WRITEBACK_ALLOC:
		/* Assuming non-aliasing VIPT dcaches we flush using
		 * the kernel addresses
		 **/
		dma_sync_single_range_for_device(&br.pdev->dev, data->dma_handle, offset,
						data->size, DMA_TO_DEVICE);
		break;
	default:
		BUG();
	}

	up_read(&data->sem);
}

static void bralloc_mem_invalidate(struct file *file, unsigned long offset, unsigned long size)
{
	bralloc_data_t *data;

	if (!has_allocation(file)) {
		bralloc_error("User (%d) tried to CACHE_FLUSH without allocation\n", current->pid);
		return;
	}

	data = (bralloc_data_t *)file->private_data;

	down_read(&data->sem);

	BUG_ON(!data || !(data->flags & BRALLOC_FLAGS_MAPPED));

	switch(BRALLOC_GET_ALLOC_TYPE(data->flags)) {
	case BRALLOC_DMA_COHERENT_ALLOC:
		bralloc_debug("Allocation is dma coherent (uncached), no invalidate required\n");
		break;
	case BRALLOC_DMA_WRITECOMBINE_ALLOC:
		bralloc_debug("Allocation is dma writecombine (uncached), no invalidate required\n");
		break;
	case BRALLOC_WRITETHROUGH_ALLOC:
	case BRALLOC_WRITEBACK_ALLOC:
		/* Assuming non-aliasing VIPT dcaches we invalidate using
		 * kernel addresses
		 **/
		dma_sync_single_range_for_cpu(&br.pdev->dev, data->dma_handle, offset,
						data->size, DMA_FROM_DEVICE);
		break;
	default:
		BUG();
	}

	up_read(&data->sem);

}

static int bralloc_unmap_pfn_range(bralloc_data_t *data, struct vm_area_struct *vma,
			unsigned long offset, unsigned long size)
{
	int garbage_pages, i;

	BUG_ON(!IS_PAGE_ALIGNED(size));

	garbage_pages = size >> PAGE_SHIFT;

	/* Unmap the device pages from this vma */
	zap_page_range(vma, vma->vm_start + offset, size, NULL);

	/* Remap the same vma to our garbage page */
	vma->vm_flags |= VM_IO | VM_RESERVED | VM_PFNMAP | VM_SHARED | VM_WRITE;
	for (i = 0; i < garbage_pages; i++) {
		if (vm_insert_pfn(vma, vma->vm_start + offset + (i* PAGE_SIZE),
					br.garbage_pfn))
			return -EAGAIN;
	}

	return 0;
}

static int bralloc_mem_open(struct inode *node, struct file *file)
{
	bralloc_data_t *data;

	bralloc_debug("opening bralloc_mem device (%d)/(%d)\n", current->pid, current->tgid);

	if ((file->private_data != NULL) && (file->private_data != &br.dev)) {
		bralloc_error("File private data does not match the registered device\n");
		return -EINVAL;
	}

	data = kmalloc(sizeof(*data), GFP_KERNEL);
	if (data == NULL) {
		bralloc_error("Failed to allocate memory for process (%d)/(%d)\n", current->pid, current->tgid);
		return -ENOMEM;
	}

	/* Initialise process data */
	init_rwsem(&data->sem);

	data->tsk = data->tsk = data->kaddr = data->start_page = NULL;
	data->flags = data->pid = data->size = 0;
	data->dma_handle = -1;
	data->phys_addr = -1;

	file->private_data = (void *)data;

	data->flags |= BRALLOC_FLAGS_OPENED;

#ifdef FOR_DEBUG
	data->file = file;
	bralloc_debug("file fcount = %ld\n", atomic_long_read(&file->f_count));
#endif
	/* Add this process data to the global list
	 * for statistics and for deallocationg in "remove" if needed
	 **/
#ifdef CONFIG_BRALLOC_MEM_STATS
	data->map_count = 0;
	INIT_LIST_HEAD(&data->brlink);
	spin_lock(&br.lock);
	list_add_tail(&data->brlink, &br.brlist);
	br.total_open++;
	spin_unlock(&br.lock);
#endif

	bralloc_debug("open success, total_opened(%d)!\n", br.total_open);

	return 0;
}

static int bralloc_mem_release(struct inode *node, struct file *file)
{
	bralloc_data_t *data = (bralloc_data_t *)file->private_data;

	bralloc_debug("Enter ...\n");

	BUG_ON(!data);

	down_write(&data->sem);

#ifdef FOR_DEBUG
	bralloc_debug("file fcount = %ld\n", atomic_long_read(&file->f_count));
#endif

	/* mmap() takes a reference to the file, so we should never
	 * get called if someone has mapped this file.
	 * flags here should always be set to BRALLOC_FLAGS_OPENED
	 */
	if (data->flags & BRALLOC_FLAGS_MAPPED) {
		bralloc_error("Wrong flags for this allocation (0x%08x), somthing wrong !\n",
				 data->flags);
		BUG();
	}

	BUG_ON(!(data->flags & BRALLOC_FLAGS_OPENED));

	/* Free up the allocation */
	if (has_allocation(file)) {
		bralloc_debug("Free up allocation\n");
		bralloc_mem_free(data);
	}

#ifdef CONFIG_BRALLOC_MEM_STATS
	BUG_ON(data->map_count);
	spin_lock(&br.lock);
	list_del_init(&data->brlink);
	br.total_open--;
	spin_unlock(&br.lock);
#endif

	up_write(&data->sem);

	kfree(data);

	bralloc_debug("Exit...\n");

	return 0;
}

static long bralloc_mem_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	bralloc_region_t region;
	bralloc_data_t *data;
	long ret = 0;

	bralloc_debug("Enter ..\n");

#ifdef FOR_DEBUG
	bralloc_debug("file fcount = %ld\n", atomic_long_read(&file->f_count));
#endif

	switch(cmd) {
	case BRALLOC_GET_PHYS:
		bralloc_debug("Enter GET_PHYS..\n");
		if (!has_allocation(file)) {
			region.offset = 0;
			region.size = 0;
		} else {
			data = (bralloc_data_t *)file->private_data;
			down_read(&data->sem);
			region.offset = data->phys_addr;
			region.size = data->size;
			up_read(&data->sem);
		}

		bralloc_debug("Request for phys addr of region by process (%d)\n", current->pid);

		if (copy_to_user((void __user *)arg, &region, sizeof(region))) {
			bralloc_error("copy_to_user for GET_PHYS failed\n");
			ret = -EFAULT;
			goto out;
		}

		bralloc_debug("Exit GET_PHYS ..\n");

		break;
	case BRALLOC_GET_SIZE:
		bralloc_debug("Enter GET_SIZE ..\n");
		if (!has_allocation(file)) {
			region.offset = 0;
			region.size = 0;
		} else {
			data = (bralloc_data_t *)file->private_data;
			down_read(&data->sem);
			region.offset = data->phys_addr;
			region.size = data->size;
			up_read(&data->sem);
		}

		bralloc_debug("Request for SIZE of region by process (%d)\n", current->pid);

		if (copy_to_user((void __user *)arg, &region, sizeof(region))) {
			bralloc_error("copy_to_user for GET_SIZE failed\n");
			ret = -EFAULT;
			goto out;
		}

		bralloc_debug("Exit GET_SIZE ..\n");

		break;
	case BRALLOC_GET_TOTAL_SIZE:
		bralloc_debug("Enter GET_TOTAL_SIZE ..\n");
		bralloc_info("BRALLOC_GET_TOTAL_SIZE ioctl() is not implemented atm\n");
		bralloc_debug("Exit GET_TOTAL_SIZE ..\n");

		break;
	case BRALLOC_CACHE_FLUSH:
		bralloc_debug("Enter CACHE_FLUSH ..\n");
		if (copy_from_user(&region, (void __user *)arg, sizeof(region))) {
			bralloc_error("copy_from_user for CACHE_FLUSH failed\n");
			ret = -EFAULT;
			goto out;
		}

		bralloc_mem_flush(file, region.offset, region.size);

		bralloc_debug("Enter CACHE_FLUSH ..\n");

		break;

	case BRALLOC_CACHE_INVALIDATE:
		bralloc_debug("Enter CACHE_FLUSH ..\n");
		if (copy_from_user(&region, (void __user *)arg, sizeof(region))) {
			bralloc_error("copy_from_user for CACHE_FLUSH failed\n");
			ret = -EFAULT;
			goto out;
		}

		bralloc_mem_invalidate(file, region.offset, region.size);

		bralloc_debug("Enter CACHE_FLUSH ..\n");

		break;

	default:
		bralloc_error("Undefined ioctl request\n");
		ret = -EINVAL;
		goto out;
	}

	bralloc_debug("Exit ..\n");
out :
	return ret;
}

static void bralloc_vma_open(struct vm_area_struct *vma)
{
	struct file *file = vma->vm_file;
	bralloc_data_t *data = file->private_data;

	bralloc_debug("Enter ..\n");

#ifdef FOR_DEBUG
	bralloc_debug("file fcount = %ld\n", atomic_long_read(&file->f_count));
#endif

	BUG_ON(!has_allocation(file));

	down_write(&data->sem);
	/* dont allow fork'ed process to map this data */
	bralloc_unmap_pfn_range(data, vma, 0, vma->vm_end - vma->vm_start);
	up_write(&data->sem);

	bralloc_debug("Exit ..\n");

}

static void bralloc_vma_close(struct vm_area_struct *vma)
{
	struct file *file = vma->vm_file;
	bralloc_data_t *data = file->private_data;

	bralloc_debug("Enter ..\n");
#ifdef FOR_DEBUG
	bralloc_debug("file fcount = %ld\n", atomic_long_read(&file->f_count));
#endif

	if(!has_allocation(file)) {
		bralloc_error("Something is wrong, there is not allocation for the vma being closed\n");
		bralloc_error("File reg (%ld), current (%u)\n", file_count(file), current->pid);
		return;
	}

	down_write(&data->sem);

	data->flags &= ~BRALLOC_FLAGS_MAPPED;
#ifdef CONFIG_BRALLOC_MEM_STATS
	data->map_count--;
#endif

	up_write(&data->sem);

	bralloc_debug("Exit ..\n");
}

static struct vm_operations_struct bralloc_vm_ops = {
	.open = bralloc_vma_open,
	.close = bralloc_vma_close,
};

static int bralloc_mem_mmap(struct file *file, struct vm_area_struct *vma)
{
	bralloc_data_t *data;
	unsigned long vma_size = vma->vm_end - vma->vm_start;
	int ret = 0;

	bralloc_debug("Enter ...\n");
#ifdef FOR_DEBUG
	bralloc_debug("file fcount = %ld\n", atomic_long_read(&file->f_count));
#endif

	/* offset in mmap() should always be zero and size must be PAGE aligned
	 **/
	if (vma->vm_pgoff || !IS_PAGE_ALIGNED(vma_size)) {
		bralloc_error("mmaps must be at offset 0 and PAGE_SIZE aligned\n");
		ret = -EINVAL;
		goto done;
	}

	data = (bralloc_data_t *)file->private_data;

	BUG_ON(!data);

	/* Start allocation and mmaping now */
	down_write(&data->sem);

	if (!has_allocation(file)) {
		bralloc_debug("Allocationg for mmap()\n");
		ret = bralloc_mem_allocate(file, data, vma_size);
		if (ret) {
			bralloc_error("Failed to allocate (0x%08lx) bytes from bralloc mem\n", vma_size);
			ret = -ENOMEM;
			goto done_unlock;
		}
	}

	/* Add some check here to see if we can really allocate the asked size
	 * from the DMA/cma region
	 */
	if (bralloc_map_pfn_range(data, vma, 0, vma_size)) {
		/* we keep the allocation even if mmap() fails */
		bralloc_error("bralloc_map failed in kernel\n");
		ret = -EAGAIN;
		goto done_unlock;
	}

	data->flags |= BRALLOC_FLAGS_MAPPED;
#ifdef CONFIG_BRALLOC_MEM_STATS
	data->map_count++;
#endif

	vma->vm_ops = &bralloc_vm_ops;

done_unlock:
	up_write(&data->sem);
done:
	bralloc_debug("Exit ...\n");
	return ret;
}

struct file_operations bralloc_mem_fops = {
	.owner 		= THIS_MODULE,
	.unlocked_ioctl = bralloc_mem_ioctl,
	.open 		= bralloc_mem_open,
	.release 	= bralloc_mem_release,
	.mmap 		= bralloc_mem_mmap,
};

#ifdef CONFIG_BRALLOC_MEM_STATS
static void *bralloc_mem_start(struct seq_file *m, loff_t *pos)
{
	struct list_head *p = (struct list_head *)m->private;

	if (list_empty(&br.brlist))
		return NULL;

	if (p == &br.brlist) {
		seq_printf(m, "%-20s :   %8d", "Total Opened", br.total_open);
		seq_putc(m, '\n');
		seq_putc(m, '\n');
		seq_printf(m, "%-35s :    %-15s %-15s %-14s %-11s %-12s %-10s",
				"Allocating process(pid/tgid)",
				"Phys Addr",
				"Virt Addr",
				"DMA Addr",
				"Size",
				"Map Count",
				"Flags");
		seq_putc(m, '\n');
	}

	if (list_is_last(p, &br.brlist))
		return  NULL;

	*pos = *pos + 1;
	return p->next;
}

static void *bralloc_mem_next(struct seq_file *m, void *arg, loff_t *pos)
{
	struct list_head *p = (struct list_head *)m->private;


	if (list_is_last(p, &br.brlist))
		return NULL;

	*pos = *pos + 1;

	return p->next;
}

static void bralloc_mem_stop(struct seq_file *m, void *arg)
{
}

static int brallocstat_show(struct seq_file *m, void *arg)
{
	bralloc_data_t *data;

	data = list_entry((struct list_head *)arg, bralloc_data_t, brlink);

	/* Take the read lock for data */
	down_read(&data->sem);

	seq_printf(m, "%-16s(%08d/%08d) :",
			data->tsk->comm,
			data->tsk->pid,
			data->tsk->tgid);
	seq_printf(m, "   0x%08x      0x%p      0x%08x",
			data->phys_addr,
			data->kaddr,
			data->dma_handle);
	seq_printf(m, "     %08dkB   %08d     0x%08x",
			data->size/1024, data->map_count, data->flags);

	seq_putc(m, '\n');

	if (m->count < m->size) /* We have copied this element successfully */
		m->private = arg;

	up_read(&data->sem);

	return 0;
}


static const struct seq_operations brallocstat_op = {
	.start	= bralloc_mem_start,
	.next	= bralloc_mem_next,
	.stop	= bralloc_mem_stop,
	.show	= brallocstat_show,
};

static int brallocstat_open(struct inode *inode, struct file *file)
{
	int ret;
	ret = seq_open(file, &brallocstat_op);
	if (!ret) {
		struct seq_file *m = file->private_data;
		m->private = &br.brlist;
	}

	return ret;
}


static const struct file_operations brallocstat_file_ops = {
	.open		= brallocstat_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= seq_release,
};

#endif /* CONFIG_BRALLOC_MEM_STATS */

static int bralloc_probe(struct platform_device *pdev)
{
	int ret;

	br.dev.name = "bralloc_mem";
	br.dev.fops = &bralloc_mem_fops;
	br.dev.minor = 255;

	spin_lock_init(&br.lock);

	INIT_LIST_HEAD(&br.brlist);

	ret = misc_register(&br.dev);
	if (ret) {
		printk(KERN_ALERT"FATAL : Failed to register Bralloc Misc Device !!\n");
		goto err_failed_to_register;
	}

	/* We may need pdev->dev for dma allocations */
	br.pdev = pdev;

	/* allocate garbage page frame */
	br.garbage_pfn = page_to_pfn(alloc_page(GFP_KERNEL));
#ifdef CONFIG_BRALLOC_MEM_STATS
	proc_create("brallocstat", S_IRUGO, NULL, &brallocstat_file_ops);
#endif
	printk(KERN_INFO"Bralloc Probe OK!\n");

	return 0;

err_failed_to_register:
	printk(KERN_ERR"Brlloc Init Failed with ret = %d\n", ret);
	return ret;
}

static int bralloc_remove(struct platform_device *pdev)
{
	__free_page(pfn_to_page(br.garbage_pfn));
	misc_deregister(&br.dev);
	printk(KERN_INFO"BRCM alloc exit Ok !\n");

	return 0;
}

static struct platform_driver bralloc_driver = {
	.probe = bralloc_probe,
	.remove = bralloc_remove,
	.driver = { .name = "bralloc" }
};

static int __init bralloc_init(void)
{
	int ret;

	ret = platform_driver_register(&bralloc_driver);

	printk(KERN_INFO"Bralloc driver init (%s)\n", ret ? "Failed" : "Ok");

	return ret;
}

static void __exit bralloc_exit(void)
{
	platform_driver_unregister(&bralloc_driver);
}

module_init(bralloc_init);
module_exit(bralloc_exit);
