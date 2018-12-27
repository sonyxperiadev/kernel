/*
 * Copyright (C) 2013 Red Hat
 * Author: Rob Clark <robdclark@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <linux/spinlock.h>
#include <linux/shmem_fs.h>
#include <linux/dma-buf.h>
#include <linux/pfn_t.h>

#include "msm_drv.h"
#include "msm_fence.h"
#include "msm_gem.h"
#include "msm_gpu.h"
#include "msm_mmu.h"

static void *get_dmabuf_ptr(struct drm_gem_object *obj)
{
	return (obj && obj->import_attach) ? obj->import_attach->dmabuf : NULL;
}

static dma_addr_t physaddr(struct drm_gem_object *obj)
{
	struct msm_gem_object *msm_obj = to_msm_bo(obj);
	struct msm_drm_private *priv = obj->dev->dev_private;
	return (((dma_addr_t)msm_obj->vram_node->start) << PAGE_SHIFT) +
			priv->vram.paddr;
}

static bool use_pages(struct drm_gem_object *obj)
{
	struct msm_gem_object *msm_obj = to_msm_bo(obj);
	return !msm_obj->vram_node;
}

/* allocate pages from VRAM carveout, used when no IOMMU: */
static struct page **get_pages_vram(struct drm_gem_object *obj,
		int npages)
{
	struct msm_gem_object *msm_obj = to_msm_bo(obj);
	struct msm_drm_private *priv = obj->dev->dev_private;
	dma_addr_t paddr;
	struct page **p;
	int ret, i;

	p = drm_malloc_ab(npages, sizeof(struct page *));
	if (!p)
		return ERR_PTR(-ENOMEM);

	ret = drm_mm_insert_node(&priv->vram.mm, msm_obj->vram_node,
			npages, 0, DRM_MM_SEARCH_DEFAULT);
	if (ret) {
		drm_free_large(p);
		return ERR_PTR(ret);
	}

	paddr = physaddr(obj);
	for (i = 0; i < npages; i++) {
		p[i] = phys_to_page(paddr);
		paddr += PAGE_SIZE;
	}

	return p;
}

/* called with dev->struct_mutex held */
static struct page **get_pages(struct drm_gem_object *obj)
{
	struct msm_gem_object *msm_obj = to_msm_bo(obj);

	if (obj->import_attach)
		return msm_obj->pages;

	if (!msm_obj->pages) {
		struct drm_device *dev = obj->dev;
		struct page **p;
		int npages = obj->size >> PAGE_SHIFT;

		if (use_pages(obj))
			p = drm_gem_get_pages(obj);
		else
			p = get_pages_vram(obj, npages);

		if (IS_ERR(p)) {
			dev_err(dev->dev, "could not get pages: %ld\n",
					PTR_ERR(p));
			return p;
		}

		msm_obj->pages = p;

		msm_obj->sgt = drm_prime_pages_to_sg(p, npages);
		if (IS_ERR(msm_obj->sgt)) {
			void *ptr = ERR_CAST(msm_obj->sgt);

			dev_err(dev->dev, "failed to allocate sgt\n");
			msm_obj->sgt = NULL;
			return ptr;
		}

		/*
		 * Make sure to flush the CPU cache for newly allocated memory
		 * so we don't get ourselves into trouble with a dirty cache
		 */
		if (msm_obj->flags & (MSM_BO_WC|MSM_BO_UNCACHED))
			dma_sync_sg_for_device(dev->dev, msm_obj->sgt->sgl,
				msm_obj->sgt->nents, DMA_BIDIRECTIONAL);
	}

	return msm_obj->pages;
}

static void put_pages(struct drm_gem_object *obj)
{
	struct msm_gem_object *msm_obj = to_msm_bo(obj);

	if (msm_obj->pages) {
		if (msm_obj->sgt) {
			sg_free_table(msm_obj->sgt);
			kfree(msm_obj->sgt);
		}

		if (use_pages(obj))
			drm_gem_put_pages(obj, msm_obj->pages, true, false);
		else {
			drm_mm_remove_node(msm_obj->vram_node);
			drm_free_large(msm_obj->pages);
		}

		msm_obj->pages = NULL;
	}
}

struct page **msm_gem_get_pages(struct drm_gem_object *obj)
{
	struct drm_device *dev = obj->dev;
	struct page **p;
	mutex_lock(&dev->struct_mutex);
	p = get_pages(obj);
	mutex_unlock(&dev->struct_mutex);
	return p;
}

void msm_gem_put_pages(struct drm_gem_object *obj)
{
	/* when we start tracking the pin count, then do something here */
}

void msm_gem_sync(struct drm_gem_object *obj)
{
	struct msm_gem_object *msm_obj;

	if (!obj)
		return;

	msm_obj = to_msm_bo(obj);

	/*
	 * dma_sync_sg_for_device synchronises a single contiguous or
	 * scatter/gather mapping for the CPU and device.
	 */
	dma_sync_sg_for_device(obj->dev->dev, msm_obj->sgt->sgl,
		       msm_obj->sgt->nents, DMA_BIDIRECTIONAL);
}


int msm_gem_mmap_obj(struct drm_gem_object *obj,
		struct vm_area_struct *vma)
{
	struct msm_gem_object *msm_obj = to_msm_bo(obj);

	vma->vm_flags &= ~VM_PFNMAP;
	vma->vm_flags |= VM_MIXEDMAP;

	if (msm_obj->flags & MSM_BO_WC) {
		vma->vm_page_prot = pgprot_writecombine(vm_get_page_prot(vma->vm_flags));
	} else if (msm_obj->flags & MSM_BO_UNCACHED) {
		vma->vm_page_prot = pgprot_noncached(vm_get_page_prot(vma->vm_flags));
	} else {
		/*
		 * Shunt off cached objs to shmem file so they have their own
		 * address_space (so unmap_mapping_range does what we want,
		 * in particular in the case of mmap'd dmabufs)
		 */
		fput(vma->vm_file);
		get_file(obj->filp);
		vma->vm_pgoff = 0;
		vma->vm_file  = obj->filp;

		vma->vm_page_prot = vm_get_page_prot(vma->vm_flags);
	}

	return 0;
}

int msm_gem_mmap(struct file *filp, struct vm_area_struct *vma)
{
	int ret;

	ret = drm_gem_mmap(filp, vma);
	if (ret) {
		DBG("mmap failed: %d", ret);
		return ret;
	}

	return msm_gem_mmap_obj(vma->vm_private_data, vma);
}

int msm_gem_fault(struct vm_area_struct *vma, struct vm_fault *vmf)
{
	struct drm_gem_object *obj = vma->vm_private_data;
	struct drm_device *dev = obj->dev;
	struct msm_drm_private *priv = dev->dev_private;
	struct page **pages;
	unsigned long pfn;
	pgoff_t pgoff;
	int ret;

	/* This should only happen if userspace tries to pass a mmap'd
	 * but unfaulted gem bo vaddr into submit ioctl, triggering
	 * a page fault while struct_mutex is already held.  This is
	 * not a valid use-case so just bail.
	 */
	if (priv->struct_mutex_task == current)
		return VM_FAULT_SIGBUS;

	/* Make sure we don't parallel update on a fault, nor move or remove
	 * something from beneath our feet
	 */
	ret = mutex_lock_interruptible(&dev->struct_mutex);
	if (ret)
		goto out;

	/* make sure we have pages attached now */
	pages = get_pages(obj);
	if (IS_ERR(pages)) {
		ret = PTR_ERR(pages);
		goto out_unlock;
	}

	/* We don't use vmf->pgoff since that has the fake offset: */
	pgoff = ((unsigned long)vmf->virtual_address -
			vma->vm_start) >> PAGE_SHIFT;

	pfn = page_to_pfn(pages[pgoff]);

	VERB("Inserting %pK pfn %lx, pa %lx", vmf->virtual_address,
			pfn, pfn << PAGE_SHIFT);

	ret = vm_insert_mixed(vma, (unsigned long)vmf->virtual_address,
			__pfn_to_pfn_t(pfn, PFN_DEV));

out_unlock:
	mutex_unlock(&dev->struct_mutex);
out:
	switch (ret) {
	case -EAGAIN:
	case 0:
	case -ERESTARTSYS:
	case -EINTR:
	case -EBUSY:
		/*
		 * EBUSY is ok: this just means that another thread
		 * already did the job.
		 */
		return VM_FAULT_NOPAGE;
	case -ENOMEM:
		return VM_FAULT_OOM;
	default:
		return VM_FAULT_SIGBUS;
	}
}

/** get mmap offset */
static uint64_t mmap_offset(struct drm_gem_object *obj)
{
	struct drm_device *dev = obj->dev;
	int ret;

	WARN_ON(!mutex_is_locked(&dev->struct_mutex));

	/* Make it mmapable */
	ret = drm_gem_create_mmap_offset(obj);

	if (ret) {
		dev_err(dev->dev, "could not allocate mmap offset\n");
		return 0;
	}

	return drm_vma_node_offset_addr(&obj->vma_node);
}

uint64_t msm_gem_mmap_offset(struct drm_gem_object *obj)
{
	uint64_t offset;
	mutex_lock(&obj->dev->struct_mutex);
	offset = mmap_offset(obj);
	mutex_unlock(&obj->dev->struct_mutex);
	return offset;
}

dma_addr_t msm_gem_get_dma_addr(struct drm_gem_object *obj)
{
	struct msm_gem_object *msm_obj = to_msm_bo(obj);
	struct drm_device *dev = obj->dev;

	if (IS_ERR_OR_NULL(msm_obj->sgt)) {
		dev_err(dev->dev, "invalid scatter/gather table\n");
		return 0;
	}

	return sg_dma_address(msm_obj->sgt->sgl);
}

static void obj_remove_domain(struct msm_gem_vma *domain)
{
	if (domain) {
		list_del(&domain->list);
		kfree(domain);
	}
}

static void put_iova(struct drm_gem_object *obj)
{
	struct drm_device *dev = obj->dev;
	struct msm_gem_object *msm_obj = to_msm_bo(obj);
	struct msm_gem_vma *domain, *tmp;

	WARN_ON(!mutex_is_locked(&dev->struct_mutex));

	list_for_each_entry_safe(domain, tmp, &msm_obj->domains, list) {
		if (iommu_present(&platform_bus_type)) {
			msm_gem_unmap_vma(domain->aspace, domain,
				msm_obj->sgt, get_dmabuf_ptr(obj));
		}

		/*
		 * put_iova removes the domain connected to the obj which makes
		 * the aspace inaccessible. Store the aspace, as it is used to
		 * update the active_list during gem_free_obj and gem_purege.
		 */
		msm_obj->aspace = domain->aspace;
		obj_remove_domain(domain);
	}
}

static struct msm_gem_vma *obj_add_domain(struct drm_gem_object *obj,
		struct msm_gem_address_space *aspace)
{
	struct msm_gem_object *msm_obj = to_msm_bo(obj);
	struct msm_gem_vma *domain = kzalloc(sizeof(*domain), GFP_KERNEL);

	if (!domain)
		return ERR_PTR(-ENOMEM);

	domain->aspace = aspace;

	list_add_tail(&domain->list, &msm_obj->domains);

	return domain;
}

static struct msm_gem_vma *obj_get_domain(struct drm_gem_object *obj,
		struct msm_gem_address_space *aspace)
{
	struct msm_gem_object *msm_obj = to_msm_bo(obj);
	struct msm_gem_vma *domain;

	list_for_each_entry(domain, &msm_obj->domains, list) {
		if (domain->aspace == aspace)
			return domain;
	}

	return NULL;
}

/* should be called under struct_mutex.. although it can be called
 * from atomic context without struct_mutex to acquire an extra
 * iova ref if you know one is already held.
 *
 * That means when I do eventually need to add support for unpinning
 * the refcnt counter needs to be atomic_t.
 */
int msm_gem_get_iova_locked(struct drm_gem_object *obj,
		struct msm_gem_address_space *aspace, uint32_t *iova)
{
	struct msm_gem_object *msm_obj = to_msm_bo(obj);
	struct page **pages;
	struct msm_gem_vma *domain;
	int ret = 0;

	if (!iommu_present(&platform_bus_type)) {
		pages = get_pages(obj);

		if (IS_ERR(pages))
			return PTR_ERR(pages);

		*iova = physaddr(obj);
		return 0;
	}

	domain = obj_get_domain(obj, aspace);

	if (!domain) {
		domain = obj_add_domain(obj, aspace);
		if (IS_ERR(domain))
			return PTR_ERR(domain);

		pages = get_pages(obj);
		if (IS_ERR(pages)) {
			obj_remove_domain(domain);
			return PTR_ERR(pages);
		}

		ret = msm_gem_map_vma(aspace, domain, msm_obj->sgt,
			get_dmabuf_ptr(obj),
			msm_obj->flags);
	}

	if (!ret && domain) {
		*iova = domain->iova;
		if (aspace && !msm_obj->in_active_list)
			msm_gem_add_obj_to_aspace_active_list(aspace, obj);
	} else {
		obj_remove_domain(domain);
	}

	return ret;
}

/* get iova, taking a reference.  Should have a matching put */
int msm_gem_get_iova(struct drm_gem_object *obj,
		struct msm_gem_address_space *aspace, uint32_t *iova)
{
	struct msm_gem_vma *domain;
	int ret;

	domain = obj_get_domain(obj, aspace);
	if (domain) {
		*iova = domain->iova;
		return 0;
	}

	mutex_lock(&obj->dev->struct_mutex);
	ret = msm_gem_get_iova_locked(obj, aspace, iova);
	mutex_unlock(&obj->dev->struct_mutex);
	return ret;
}

/* get iova without taking a reference, used in places where you have
 * already done a 'msm_gem_get_iova()'.
 */
uint32_t msm_gem_iova(struct drm_gem_object *obj,
		struct msm_gem_address_space *aspace)
{
	struct msm_gem_vma *domain = obj_get_domain(obj, aspace);

	WARN_ON(!domain);

	return domain ? domain->iova : 0;
}

void msm_gem_put_iova(struct drm_gem_object *obj,
		struct msm_gem_address_space *aspace)
{
	// XXX TODO ..
	// NOTE: probably don't need a _locked() version.. we wouldn't
	// normally unmap here, but instead just mark that it could be
	// unmapped (if the iova refcnt drops to zero), but then later
	// if another _get_iova_locked() fails we can start unmapping
	// things that are no longer needed..
}

void msm_gem_aspace_domain_attach_detach_update(
		struct msm_gem_address_space *aspace,
		bool is_detach)
{
	struct msm_gem_object *msm_obj;
	struct drm_gem_object *obj;
	struct aspace_client *aclient;
	int ret;
	uint32_t iova;

	if (!aspace)
		return;

	mutex_lock(&aspace->dev->struct_mutex);
	if (is_detach) {
		/* Indicate to clients domain is getting detached */
		list_for_each_entry(aclient, &aspace->clients, list) {
			if (aclient->cb)
				aclient->cb(aclient->cb_data,
						is_detach);
		}

		/**
		 * Unmap active buffers,
		 * typically clients should do this when the callback is called,
		 * but this needs to be done for the buffers which are not
		 * attached to any planes.
		 */
		list_for_each_entry(msm_obj, &aspace->active_list, iova_list) {
			obj = &msm_obj->base;
			if (obj->import_attach)
				put_iova(obj);
		}
	} else {
		/* map active buffers */
		list_for_each_entry(msm_obj, &aspace->active_list, iova_list) {
			obj = &msm_obj->base;
			ret = msm_gem_get_iova_locked(obj, aspace, &iova);
			if (ret) {
				mutex_unlock(&aspace->dev->struct_mutex);
				return;
			}
		}

		/* Indicate to clients domain is attached */
		list_for_each_entry(aclient, &aspace->clients, list) {
			if (aclient->cb)
				aclient->cb(aclient->cb_data,
						is_detach);
		}
	}
	mutex_unlock(&aspace->dev->struct_mutex);
}

int msm_gem_dumb_create(struct drm_file *file, struct drm_device *dev,
		struct drm_mode_create_dumb *args)
{
	args->pitch = align_pitch(args->width, args->bpp);
	args->size  = PAGE_ALIGN(args->pitch * args->height);
	return msm_gem_new_handle(dev, file, args->size,
			MSM_BO_SCANOUT | MSM_BO_WC, &args->handle);
}

int msm_gem_dumb_map_offset(struct drm_file *file, struct drm_device *dev,
		uint32_t handle, uint64_t *offset)
{
	struct drm_gem_object *obj;
	int ret = 0;

	/* GEM does all our handle to object mapping */
	obj = drm_gem_object_lookup(file, handle);
	if (obj == NULL) {
		ret = -ENOENT;
		goto fail;
	}

	*offset = msm_gem_mmap_offset(obj);

	drm_gem_object_unreference_unlocked(obj);

fail:
	return ret;
}

void *msm_gem_get_vaddr_locked(struct drm_gem_object *obj)
{
	struct msm_gem_object *msm_obj = to_msm_bo(obj);
	WARN_ON(!mutex_is_locked(&obj->dev->struct_mutex));
	if (!msm_obj->vaddr) {
		struct page **pages = get_pages(obj);
		if (IS_ERR(pages))
			return ERR_CAST(pages);
		if (obj->import_attach)
			msm_obj->vaddr = dma_buf_vmap(
				      obj->import_attach->dmabuf);
		else
			msm_obj->vaddr = vmap(pages, obj->size >> PAGE_SHIFT,
				VM_MAP, pgprot_writecombine(PAGE_KERNEL));

		if (msm_obj->vaddr == NULL)
			return ERR_PTR(-ENOMEM);
	}
	msm_obj->vmap_count++;
	return msm_obj->vaddr;
}

void *msm_gem_get_vaddr(struct drm_gem_object *obj)
{
	void *ret;
	mutex_lock(&obj->dev->struct_mutex);
	ret = msm_gem_get_vaddr_locked(obj);
	mutex_unlock(&obj->dev->struct_mutex);
	return ret;
}

void msm_gem_put_vaddr_locked(struct drm_gem_object *obj)
{
	struct msm_gem_object *msm_obj = to_msm_bo(obj);
	WARN_ON(!mutex_is_locked(&obj->dev->struct_mutex));
	WARN_ON(msm_obj->vmap_count < 1);
	msm_obj->vmap_count--;
}

void msm_gem_put_vaddr(struct drm_gem_object *obj)
{
	mutex_lock(&obj->dev->struct_mutex);
	msm_gem_put_vaddr_locked(obj);
	mutex_unlock(&obj->dev->struct_mutex);
}

/* Update madvise status, returns true if not purged, else
 * false or -errno.
 */
int msm_gem_madvise(struct drm_gem_object *obj, unsigned madv)
{
	struct msm_gem_object *msm_obj = to_msm_bo(obj);

	WARN_ON(!mutex_is_locked(&obj->dev->struct_mutex));

	if (msm_obj->madv != __MSM_MADV_PURGED)
		msm_obj->madv = madv;

	return (msm_obj->madv != __MSM_MADV_PURGED);
}

void msm_gem_purge(struct drm_gem_object *obj)
{
	struct drm_device *dev = obj->dev;
	struct msm_gem_object *msm_obj = to_msm_bo(obj);

	WARN_ON(!mutex_is_locked(&dev->struct_mutex));
	WARN_ON(!is_purgeable(msm_obj));
	WARN_ON(obj->import_attach);

	put_iova(obj);
	msm_gem_remove_obj_from_aspace_active_list(msm_obj->aspace, obj);

	msm_gem_vunmap(obj);

	put_pages(obj);

	msm_obj->madv = __MSM_MADV_PURGED;

	drm_vma_node_unmap(&obj->vma_node, dev->anon_inode->i_mapping);
	drm_gem_free_mmap_offset(obj);

	/* Our goal here is to return as much of the memory as
	 * is possible back to the system as we are called from OOM.
	 * To do this we must instruct the shmfs to drop all of its
	 * backing pages, *now*.
	 */
	shmem_truncate_range(file_inode(obj->filp), 0, (loff_t)-1);

	invalidate_mapping_pages(file_inode(obj->filp)->i_mapping,
			0, (loff_t)-1);
}

void msm_gem_vunmap(struct drm_gem_object *obj)
{
	struct msm_gem_object *msm_obj = to_msm_bo(obj);

	if (!msm_obj->vaddr || WARN_ON(!is_vunmapable(msm_obj)))
		return;

	if (obj->import_attach)
		dma_buf_vunmap(obj->import_attach->dmabuf, msm_obj->vaddr);
	else
		vunmap(msm_obj->vaddr);

	msm_obj->vaddr = NULL;
}

/* must be called before _move_to_active().. */
int msm_gem_sync_object(struct drm_gem_object *obj,
		struct msm_fence_context *fctx, bool exclusive)
{
	struct msm_gem_object *msm_obj = to_msm_bo(obj);
	struct reservation_object_list *fobj;
	struct fence *fence;
	int i, ret;

	if (!exclusive) {
		/* NOTE: _reserve_shared() must happen before _add_shared_fence(),
		 * which makes this a slightly strange place to call it.  OTOH this
		 * is a convenient can-fail point to hook it in.  (And similar to
		 * how etnaviv and nouveau handle this.)
		 */
		ret = reservation_object_reserve_shared(msm_obj->resv);
		if (ret)
			return ret;
	}

	fobj = reservation_object_get_list(msm_obj->resv);
	if (!fobj || (fobj->shared_count == 0)) {
		fence = reservation_object_get_excl(msm_obj->resv);
		/* don't need to wait on our own fences, since ring is fifo */
		if (fence && (fence->context != fctx->context)) {
			ret = fence_wait(fence, true);
			if (ret)
				return ret;
		}
	}

	if (!exclusive || !fobj)
		return 0;

	for (i = 0; i < fobj->shared_count; i++) {
		fence = rcu_dereference_protected(fobj->shared[i],
						reservation_object_held(msm_obj->resv));
		if (fence->context != fctx->context) {
			ret = fence_wait(fence, true);
			if (ret)
				return ret;
		}
	}

	return 0;
}

void msm_gem_move_to_active(struct drm_gem_object *obj,
		struct msm_gpu *gpu, bool exclusive, struct fence *fence)
{
	struct msm_gem_object *msm_obj = to_msm_bo(obj);
	WARN_ON(msm_obj->madv != MSM_MADV_WILLNEED);
	msm_obj->gpu = gpu;
	if (exclusive)
		reservation_object_add_excl_fence(msm_obj->resv, fence);
	else
		reservation_object_add_shared_fence(msm_obj->resv, fence);
	list_del_init(&msm_obj->mm_list);
	list_add_tail(&msm_obj->mm_list, &gpu->active_list);
}

void msm_gem_move_to_inactive(struct drm_gem_object *obj)
{
	struct drm_device *dev = obj->dev;
	struct msm_drm_private *priv = dev->dev_private;
	struct msm_gem_object *msm_obj = to_msm_bo(obj);

	WARN_ON(!mutex_is_locked(&dev->struct_mutex));

	msm_obj->gpu = NULL;
	list_del_init(&msm_obj->mm_list);
	list_add_tail(&msm_obj->mm_list, &priv->inactive_list);
}

int msm_gem_cpu_prep(struct drm_gem_object *obj, uint32_t op, ktime_t *timeout)
{
	struct msm_gem_object *msm_obj = to_msm_bo(obj);
	bool write = !!(op & MSM_PREP_WRITE);
	unsigned long remain =
		op & MSM_PREP_NOSYNC ? 0 : timeout_to_jiffies(timeout);
	long ret;

	ret = reservation_object_wait_timeout_rcu(msm_obj->resv, write,
						  true,  remain);
	if (ret == 0)
		return remain == 0 ? -EBUSY : -ETIMEDOUT;
	else if (ret < 0)
		return ret;

	/* TODO cache maintenance */

	return 0;
}

int msm_gem_cpu_fini(struct drm_gem_object *obj)
{
	/* TODO cache maintenance */
	return 0;
}

#ifdef CONFIG_DEBUG_FS
static void describe_fence(struct fence *fence, const char *type,
		struct seq_file *m)
{
	if (!fence_is_signaled(fence))
		seq_printf(m, "\t%9s: %s %s seq %u\n", type,
				fence->ops->get_driver_name(fence),
				fence->ops->get_timeline_name(fence),
				fence->seqno);
}

void msm_gem_describe(struct drm_gem_object *obj, struct seq_file *m)
{
	struct msm_gem_object *msm_obj = to_msm_bo(obj);
	struct msm_gem_vma *domain;
	struct reservation_object *robj = msm_obj->resv;
	struct reservation_object_list *fobj;
	struct fence *fence;
	uint64_t off = drm_vma_node_start(&obj->vma_node);
	const char *madv;

	WARN_ON(!mutex_is_locked(&obj->dev->struct_mutex));

	switch (msm_obj->madv) {
	case __MSM_MADV_PURGED:
		madv = " purged";
		break;
	case MSM_MADV_DONTNEED:
		madv = " purgeable";
		break;
	case MSM_MADV_WILLNEED:
	default:
		madv = "";
		break;
	}

	seq_printf(m, "%08x: %c %2d (%2d) %08llx %pK %zu%s\n",

			msm_obj->flags, is_active(msm_obj) ? 'A' : 'I',
			obj->name, obj->refcount.refcount.counter,
			off, msm_obj->vaddr, obj->size, madv);

	rcu_read_lock();
	fobj = rcu_dereference(robj->fence);
	if (fobj) {
		unsigned int i, shared_count = fobj->shared_count;

		for (i = 0; i < shared_count; i++) {
			fence = rcu_dereference(fobj->shared[i]);
			describe_fence(fence, "Shared", m);
		}
	}

	fence = rcu_dereference(robj->fence_excl);
	if (fence)
		describe_fence(fence, "Exclusive", m);
	rcu_read_unlock();

	/* FIXME: we need to print the address space here too */
	list_for_each_entry(domain, &msm_obj->domains, list)
		seq_printf(m, " %08llx", domain->iova);

	seq_puts(m, "\n");
}

void msm_gem_describe_objects(struct list_head *list, struct seq_file *m)
{
	struct msm_gem_object *msm_obj;
	int count = 0;
	size_t size = 0;

	list_for_each_entry(msm_obj, list, mm_list) {
		struct drm_gem_object *obj = &msm_obj->base;
		seq_printf(m, "   ");
		msm_gem_describe(obj, m);
		count++;
		size += obj->size;
	}

	seq_printf(m, "Total %d objects, %zu bytes\n", count, size);
}
#endif

void msm_gem_free_object(struct drm_gem_object *obj)
{
	struct drm_device *dev = obj->dev;
	struct msm_gem_object *msm_obj = to_msm_bo(obj);

	WARN_ON(!mutex_is_locked(&dev->struct_mutex));

	/* object should not be on active list: */
	WARN_ON(is_active(msm_obj));

	list_del(&msm_obj->mm_list);

	put_iova(obj);
	msm_gem_remove_obj_from_aspace_active_list(msm_obj->aspace, obj);

	if (obj->import_attach) {
		if (msm_obj->vaddr)
			dma_buf_vunmap(obj->import_attach->dmabuf,
				msm_obj->vaddr);

		/* Don't drop the pages for imported dmabuf, as they are not
		 * ours, just free the array we allocated:
		 */
		if (msm_obj->pages)
			drm_free_large(msm_obj->pages);

		drm_prime_gem_destroy(obj, msm_obj->sgt);
	} else {
		msm_gem_vunmap(obj);
		put_pages(obj);
	}

	if (msm_obj->resv == &msm_obj->_resv)
		reservation_object_fini(msm_obj->resv);

	drm_gem_object_release(obj);

	kfree(msm_obj);
}

/* convenience method to construct a GEM buffer object, and userspace handle */
int msm_gem_new_handle(struct drm_device *dev, struct drm_file *file,
		uint32_t size, uint32_t flags, uint32_t *handle)
{
	struct drm_gem_object *obj;
	int ret;

	ret = mutex_lock_interruptible(&dev->struct_mutex);
	if (ret)
		return ret;

	obj = msm_gem_new(dev, size, flags);

	mutex_unlock(&dev->struct_mutex);

	if (IS_ERR(obj))
		return PTR_ERR(obj);

	ret = drm_gem_handle_create(file, obj, handle);

	/* drop reference from allocate - handle holds it now */
	drm_gem_object_unreference_unlocked(obj);

	return ret;
}

static int msm_gem_new_impl(struct drm_device *dev,
		uint32_t size, uint32_t flags,
		struct reservation_object *resv,
		struct drm_gem_object **obj)
{
	struct msm_drm_private *priv = dev->dev_private;
	struct msm_gem_object *msm_obj;
	bool use_vram = false;

	switch (flags & MSM_BO_CACHE_MASK) {
	case MSM_BO_UNCACHED:
	case MSM_BO_CACHED:
	case MSM_BO_WC:
		break;
	default:
		dev_err(dev->dev, "invalid cache flag: %x\n",
				(flags & MSM_BO_CACHE_MASK));
		return -EINVAL;
	}

	WARN_ON(!mutex_is_locked(&dev->struct_mutex));

	if (!iommu_present(&platform_bus_type))
		use_vram = true;
	else if ((flags & MSM_BO_STOLEN) && priv->vram.size)
		use_vram = true;

	if (WARN_ON(use_vram && !priv->vram.size))
		return -EINVAL;

	msm_obj = kzalloc(sizeof(*msm_obj), GFP_KERNEL);
	if (!msm_obj)
		return -ENOMEM;

	if (use_vram) {
		struct msm_gem_vma *domain = obj_add_domain(&msm_obj->base,
				NULL);

		if (!IS_ERR(domain))
			msm_obj->vram_node = &domain->node;
	}

	msm_obj->flags = flags;
	msm_obj->madv = MSM_MADV_WILLNEED;

	if (resv) {
		msm_obj->resv = resv;
	} else {
		msm_obj->resv = &msm_obj->_resv;
		reservation_object_init(msm_obj->resv);
	}

	INIT_LIST_HEAD(&msm_obj->submit_entry);
	INIT_LIST_HEAD(&msm_obj->domains);
	INIT_LIST_HEAD(&msm_obj->iova_list);
	msm_obj->aspace = NULL;
	msm_obj->in_active_list = false;

	list_add_tail(&msm_obj->mm_list, &priv->inactive_list);

	*obj = &msm_obj->base;

	return 0;
}

struct drm_gem_object *msm_gem_new(struct drm_device *dev,
		uint32_t size, uint32_t flags)
{
	struct drm_gem_object *obj = NULL;
	int ret;

	WARN_ON(!mutex_is_locked(&dev->struct_mutex));

	size = PAGE_ALIGN(size);

	ret = msm_gem_new_impl(dev, size, flags, NULL, &obj);
	if (ret)
		goto fail;

	if (use_pages(obj)) {
		ret = drm_gem_object_init(dev, obj, size);
		if (ret)
			goto fail;
	} else {
		drm_gem_private_object_init(dev, obj, size);
	}

	return obj;

fail:
	drm_gem_object_unreference(obj);
	return ERR_PTR(ret);
}

struct drm_gem_object *msm_gem_import(struct drm_device *dev,
		struct dma_buf *dmabuf, struct sg_table *sgt)
{
	struct msm_gem_object *msm_obj;
	struct drm_gem_object *obj = NULL;
	uint32_t size;
	int ret;

	/* if we don't have IOMMU, don't bother pretending we can import: */
	if (!iommu_present(&platform_bus_type)) {
		dev_err(dev->dev, "cannot import without IOMMU\n");
		return ERR_PTR(-EINVAL);
	}

	size = PAGE_ALIGN(dmabuf->size);

	ret = mutex_lock_interruptible(&dev->struct_mutex);
	if (ret)
		return ERR_PTR(ret);

	ret = msm_gem_new_impl(dev, size, MSM_BO_WC, dmabuf->resv, &obj);
	mutex_unlock(&dev->struct_mutex);
	if (ret)
		goto fail;

	drm_gem_private_object_init(dev, obj, size);

	msm_obj = to_msm_bo(obj);
	msm_obj->sgt = sgt;
	msm_obj->pages = NULL;

	return obj;

fail:
	drm_gem_object_unreference_unlocked(obj);
	return ERR_PTR(ret);
}
