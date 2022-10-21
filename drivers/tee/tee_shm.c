/*
 * Copyright (c) 2015-2017, 2019-2021 Linaro Limited
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
#include <linux/anon_inodes.h>
#include <linux/device.h>
#include <linux/idr.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/tee_drv.h>
#include "tee_private.h"

static void tee_shm_release(struct tee_device *teedev, struct tee_shm *shm)
{
	struct tee_shm_pool_mgr *poolm;

	if (shm->flags & TEE_SHM_DMA_BUF)
		poolm = &teedev->pool->dma_buf_mgr;
	else
		poolm = &teedev->pool->private_mgr;

	poolm->ops->free(poolm, shm);
	kfree(shm);

	tee_device_put(teedev);
}

/**
 * tee_shm_alloc() - Allocate shared memory
 * @ctx:	Context that allocates the shared memory
 * @size:	Requested size of shared memory
 * @flags:	Flags setting properties for the requested shared memory.
 *
 * Memory allocated as global shared memory is automatically freed when the
 * TEE file pointer is closed. The @flags field uses the bits defined by
 * TEE_SHM_* in <linux/tee_drv.h>. TEE_SHM_MAPPED must currently always be
 * set. If TEE_SHM_DMA_BUF global shared memory will be allocated and
 * associated with a dma-buf handle, else driver private memory.
 */
struct tee_shm *tee_shm_alloc(struct tee_context *ctx, size_t size, u32 flags)
{
	struct tee_device *teedev = ctx->teedev;
	struct tee_shm_pool_mgr *poolm = NULL;
	struct tee_shm *shm;
	void *ret;
	int rc;

	if (!(flags & TEE_SHM_MAPPED)) {
		dev_err(teedev->dev.parent,
			"only mapped allocations supported\n");
		return ERR_PTR(-EINVAL);
	}

	if ((flags & ~(TEE_SHM_MAPPED | TEE_SHM_DMA_BUF))) {
		dev_err(teedev->dev.parent, "invalid shm flags 0x%x", flags);
		return ERR_PTR(-EINVAL);
	}

	if (!tee_device_get(teedev))
		return ERR_PTR(-EINVAL);

	if (!teedev->pool) {
		/* teedev has been detached from driver */
		ret = ERR_PTR(-EINVAL);
		goto err_dev_put;
	}

	shm = kzalloc(sizeof(*shm), GFP_KERNEL);
	if (!shm) {
		ret = ERR_PTR(-ENOMEM);
		goto err_dev_put;
	}

	refcount_set(&shm->refcount, 1);
	shm->flags = flags;
	shm->teedev = teedev;
	shm->ctx = ctx;
	if (flags & TEE_SHM_DMA_BUF)
		poolm = &teedev->pool->dma_buf_mgr;
	else
		poolm = &teedev->pool->private_mgr;

	rc = poolm->ops->alloc(poolm, shm, size);
	if (rc) {
		ret = ERR_PTR(rc);
		goto err_kfree;
	}

	mutex_lock(&teedev->mutex);
	shm->id = idr_alloc(&teedev->idr, shm, 1, 0, GFP_KERNEL);
	mutex_unlock(&teedev->mutex);
	if (shm->id < 0) {
		ret = ERR_PTR(shm->id);
		goto err_pool_free;
	}

	mutex_lock(&teedev->mutex);
	list_add_tail(&shm->link, &ctx->list_shm);
	mutex_unlock(&teedev->mutex);

	return shm;
err_pool_free:
	poolm->ops->free(poolm, shm);
err_kfree:
	kfree(shm);
err_dev_put:
	tee_device_put(teedev);
	return ret;
}
EXPORT_SYMBOL_GPL(tee_shm_alloc);

static int tee_shm_fop_release(struct inode *inode, struct file *filp)
{
	tee_shm_put(filp->private_data);
	return 0;
}

static int tee_shm_fop_mmap(struct file *filp, struct vm_area_struct *vma)
{
	struct tee_shm *shm = filp->private_data;
	size_t size = vma->vm_end - vma->vm_start;

	/* check for overflowing the buffer's size */
	if (vma->vm_pgoff + vma_pages(vma) > shm->size >> PAGE_SHIFT)
		return -EINVAL;

	return remap_pfn_range(vma, vma->vm_start, shm->paddr >> PAGE_SHIFT,
			       size, vma->vm_page_prot);
}

static const struct file_operations tee_shm_fops = {
	.owner = THIS_MODULE,
	.release = tee_shm_fop_release,
	.mmap = tee_shm_fop_mmap,
};

/**
 * tee_shm_get_fd() - Increase reference count and return file descriptor
 * @shm:	Shared memory handle
 * @returns user space file descriptor to shared memory
 */
int tee_shm_get_fd(struct tee_shm *shm)
{
	u32 req_flags = TEE_SHM_MAPPED | TEE_SHM_DMA_BUF;
	int fd;

	if ((shm->flags & req_flags) != req_flags)
		return -EINVAL;

	/* matched by tee_shm_put() in tee_shm_op_release() */
	refcount_inc(&shm->refcount);
	fd = anon_inode_getfd("tee_shm", &tee_shm_fops, shm, O_RDWR);
	if (fd < 0)
		tee_shm_put(shm);
	return fd;
}

/**
 * tee_shm_free() - Free shared memory
 * @shm:	Handle to shared memory to free
 */
void tee_shm_free(struct tee_shm *shm)
{
	tee_shm_put(shm);
}
EXPORT_SYMBOL_GPL(tee_shm_free);

/**
 * tee_shm_va2pa() - Get physical address of a virtual address
 * @shm:	Shared memory handle
 * @va:		Virtual address to tranlsate
 * @pa:		Returned physical address
 * @returns 0 on success and < 0 on failure
 */
int tee_shm_va2pa(struct tee_shm *shm, void *va, phys_addr_t *pa)
{
	/* Check that we're in the range of the shm */
	if ((char *)va < (char *)shm->kaddr)
		return -EINVAL;
	if ((char *)va >= ((char *)shm->kaddr + shm->size))
		return -EINVAL;

	return tee_shm_get_pa(
			shm, (unsigned long)va - (unsigned long)shm->kaddr, pa);
}
EXPORT_SYMBOL_GPL(tee_shm_va2pa);

/**
 * tee_shm_pa2va() - Get virtual address of a physical address
 * @shm:	Shared memory handle
 * @pa:		Physical address to tranlsate
 * @va:		Returned virtual address
 * @returns 0 on success and < 0 on failure
 */
int tee_shm_pa2va(struct tee_shm *shm, phys_addr_t pa, void **va)
{
	/* Check that we're in the range of the shm */
	if (pa < shm->paddr)
		return -EINVAL;
	if (pa >= (shm->paddr + shm->size))
		return -EINVAL;

	if (va) {
		void *v = tee_shm_get_va(shm, pa - shm->paddr);

		if (IS_ERR(v))
			return PTR_ERR(v);
		*va = v;
	}
	return 0;
}
EXPORT_SYMBOL_GPL(tee_shm_pa2va);

/**
 * tee_shm_get_va() - Get virtual address of a shared memory plus an offset
 * @shm:	Shared memory handle
 * @offs:	Offset from start of this shared memory
 * @returns virtual address of the shared memory + offs if offs is within
 *	the bounds of this shared memory, else an ERR_PTR
 */
void *tee_shm_get_va(struct tee_shm *shm, size_t offs)
{
	if (offs >= shm->size)
		return ERR_PTR(-EINVAL);
	return (char *)shm->kaddr + offs;
}
EXPORT_SYMBOL_GPL(tee_shm_get_va);

/**
 * tee_shm_get_pa() - Get physical address of a shared memory plus an offset
 * @shm:	Shared memory handle
 * @offs:	Offset from start of this shared memory
 * @pa:		Physical address to return
 * @returns 0 if offs is within the bounds of this shared memory, else an
 *	error code.
 */
int tee_shm_get_pa(struct tee_shm *shm, size_t offs, phys_addr_t *pa)
{
	if (offs >= shm->size)
		return -EINVAL;
	if (pa)
		*pa = shm->paddr + offs;
	return 0;
}
EXPORT_SYMBOL_GPL(tee_shm_get_pa);

/**
 * tee_shm_get_from_id() - Find shared memory object and increase reference
 * count
 * @ctx:	Context owning the shared memory
 * @id:		Id of shared memory object
 * @returns a pointer to 'struct tee_shm' on success or an ERR_PTR on failure
 */
struct tee_shm *tee_shm_get_from_id(struct tee_context *ctx, int id)
{
	struct tee_device *teedev;
	struct tee_shm *shm;

	if (!ctx)
		return ERR_PTR(-EINVAL);

	teedev = ctx->teedev;
	mutex_lock(&teedev->mutex);
	shm = idr_find(&teedev->idr, id);
	/*
	 * If the tee_shm was found in the IDR it must have a refcount
	 * larger than 0 due to the guarantee in tee_shm_put() below. So
	 * it's safe to use refcount_inc().
	 */
	if (!shm || shm->ctx != ctx)
		shm = ERR_PTR(-EINVAL);
	else
		refcount_inc(&shm->refcount);
	mutex_unlock(&teedev->mutex);
	return shm;
}
EXPORT_SYMBOL_GPL(tee_shm_get_from_id);

/**
 * tee_shm_get_id() - Get id of a shared memory object
 * @shm:	Shared memory handle
 * @returns id
 */
int tee_shm_get_id(struct tee_shm *shm)
{
	return shm->id;
}
EXPORT_SYMBOL_GPL(tee_shm_get_id);

/**
 * tee_shm_put() - Decrease reference count on a shared memory handle
 * @shm:	Shared memory handle
 */
void tee_shm_put(struct tee_shm *shm)
{
	struct tee_device *teedev = shm->teedev;
	bool do_release = false;

	mutex_lock(&teedev->mutex);
	if (refcount_dec_and_test(&shm->refcount)) {
		/*
		 * refcount has reached 0, we must now remove it from the
		 * IDR before releasing the mutex.  This will guarantee
		 * that the refcount_inc() in tee_shm_get_from_id() never
		 * starts from 0.
		 */
		if (shm->ctx)
			list_del(&shm->link);
		do_release = true;
	}
	mutex_unlock(&teedev->mutex);

	if (do_release)
		tee_shm_release(teedev, shm);
}
EXPORT_SYMBOL_GPL(tee_shm_put);
