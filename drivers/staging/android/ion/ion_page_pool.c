/*
 * drivers/staging/android/ion/ion_page_pool.c
 *
 * Copyright (C) 2011 Google, Inc.
 * Copyright (c) 2016, 2018 The Linux Foundation. All rights reserved.
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

#include <linux/debugfs.h>
#include <linux/dma-mapping.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/list.h>
#include <linux/migrate.h>
#include <linux/mount.h>
#include <linux/init.h>
#include <linux/page-flags.h>
#include <linux/slab.h>
#include <linux/swap.h>
#include <linux/vmalloc.h>
#include <linux/compaction.h>
#include "ion_priv.h"

#define ION_PAGE_CACHE	1

static void *ion_page_pool_alloc_pages(struct ion_page_pool *pool)
{
	struct page *page;

	page = alloc_pages(pool->gfp_mask & ~__GFP_ZERO, pool->order);

	if (!page)
		return NULL;

	if (pool->gfp_mask & __GFP_ZERO)
		if (msm_ion_heap_high_order_page_zero(pool->dev, page,
						      pool->order))
			goto error_free_pages;

	ion_page_pool_alloc_set_cache_policy(pool, page);

	return page;
error_free_pages:
	__free_pages(page, pool->order);
	return NULL;
}

static void ion_page_pool_free_pages(struct ion_page_pool *pool,
				     struct page *page)
{
	ion_page_pool_free_set_cache_policy(pool, page);
	if (pool->inode && pool->order == 0) {
		lock_page(page);
		__ClearPageMovable(page);
		unlock_page(page);
	}
	__free_pages(page, pool->order);
}

static int ion_page_pool_add(struct ion_page_pool *pool, struct page *page)
{
	mutex_lock(&pool->mutex);
	page->private = ION_PAGE_CACHE;
	if (PageHighMem(page)) {
		list_add_tail(&page->lru, &pool->high_items);
		pool->high_count++;
	} else {
		list_add_tail(&page->lru, &pool->low_items);
		pool->low_count++;
	}

	if (pool->inode && pool->order == 0)
		__SetPageMovable(page, pool->inode->i_mapping);
	mod_node_page_state(page_pgdat(page), NR_INDIRECTLY_RECLAIMABLE_BYTES,
			    (1 << (PAGE_SHIFT + pool->order)));
	mutex_unlock(&pool->mutex);
	return 0;
}

static struct page *ion_page_pool_remove(struct ion_page_pool *pool, bool high)
{
	struct page *page;

	if (high) {
		BUG_ON(!pool->high_count);
		page = list_first_entry(&pool->high_items, struct page, lru);
		pool->high_count--;
	} else {
		BUG_ON(!pool->low_count);
		page = list_first_entry(&pool->low_items, struct page, lru);
		pool->low_count--;
	}

	clear_bit(ION_PAGE_CACHE, &page->private);

	list_del_init(&page->lru);
	mod_node_page_state(page_pgdat(page), NR_INDIRECTLY_RECLAIMABLE_BYTES,
			    -(1 << (PAGE_SHIFT + pool->order)));
	return page;
}

void *ion_page_pool_alloc(struct ion_page_pool *pool, bool *from_pool)
{
	struct page *page = NULL;

	BUG_ON(!pool);

	*from_pool = true;

	if (mutex_trylock(&pool->mutex)) {
		if (pool->high_count)
			page = ion_page_pool_remove(pool, true);
		else if (pool->low_count)
			page = ion_page_pool_remove(pool, false);
		mutex_unlock(&pool->mutex);
	}
	if (!page) {
		page = ion_page_pool_alloc_pages(pool);
		*from_pool = false;
	} else {
		lock_page(page);
		__ClearPageMovable(page);
		unlock_page(page);
	}
	return page;
}

/*
 * Tries to allocate from only the specified Pool and returns NULL otherwise
 */
void *ion_page_pool_alloc_pool_only(struct ion_page_pool *pool)
{
	struct page *page = NULL;

	if (!pool)
		return NULL;

	if (mutex_trylock(&pool->mutex)) {
		if (pool->high_count)
			page = ion_page_pool_remove(pool, true);
		else if (pool->low_count)
			page = ion_page_pool_remove(pool, false);
		mutex_unlock(&pool->mutex);
	}
	if (page) {
		lock_page(page);
		__ClearPageMovable(page);
		unlock_page(page);
	}
	return page;
}

void ion_page_pool_free(struct ion_page_pool *pool, struct page *page)
{
	int ret;

	ret = ion_page_pool_add(pool, page);
	if (ret)
		ion_page_pool_free_pages(pool, page);
}

void ion_page_pool_free_immediate(struct ion_page_pool *pool, struct page *page)
{
	ion_page_pool_free_pages(pool, page);
}

int ion_page_pool_total(struct ion_page_pool *pool, bool high)
{
	int count = pool->low_count;

	if (high)
		count += pool->high_count;

	return count << pool->order;
}

int ion_page_pool_shrink(struct ion_page_pool *pool, gfp_t gfp_mask,
				int nr_to_scan)
{
	int freed = 0;
	bool high;

	if (current_is_kswapd())
		high = true;
	else
		high = !!(gfp_mask & __GFP_HIGHMEM);

	if (nr_to_scan == 0)
		return ion_page_pool_total(pool, high);

	while (freed < nr_to_scan) {
		struct page *page;

		mutex_lock(&pool->mutex);
		if (pool->low_count) {
			page = ion_page_pool_remove(pool, false);
		} else if (high && pool->high_count) {
			page = ion_page_pool_remove(pool, true);
		} else {
			mutex_unlock(&pool->mutex);
			break;
		}
		mutex_unlock(&pool->mutex);
		ion_page_pool_free_pages(pool, page);
		freed += (1 << pool->order);
	}

	return freed;
}

static bool ion_page_pool_isolate(struct page *page, isolate_mode_t mode)
{
	struct ion_page_pool *pool;
	struct address_space *mapping = page_mapping(page);

	VM_BUG_ON_PAGE(PageIsolated(page), page);

	if (!mapping)
		return false;
	pool = mapping->private_data;

	mutex_lock(&pool->mutex);
	/* could be removed from the cache pool and thus become unmovable */
	if (!__PageMovable(page)) {
		mutex_unlock(&pool->mutex);
		return false;
	}

	if (unlikely(!test_bit(ION_PAGE_CACHE, &page->private))) {
		mutex_unlock(&pool->mutex);
		return false;
	}

	list_del(&page->lru);
	if (PageHighMem(page))
		pool->high_count--;
	else
		pool->low_count--;
	mutex_unlock(&pool->mutex);

	return true;
}

static int ion_page_pool_migrate(struct address_space *mapping,
				 struct page *newpage,
				 struct page *page, enum migrate_mode mode)
{
	struct ion_page_pool *pool = mapping->private_data;

	VM_BUG_ON_PAGE(!PageMovable(page), page);
	VM_BUG_ON_PAGE(!PageIsolated(page), page);

	lock_page(page);
	newpage->private = ION_PAGE_CACHE;
	__SetPageMovable(newpage, page_mapping(page));
	get_page(newpage);
	__ClearPageMovable(page);
	ClearPagePrivate(page);
	unlock_page(page);
	mutex_lock(&pool->mutex);
	if (PageHighMem(newpage)) {
		list_add_tail(&newpage->lru, &pool->high_items);
		pool->high_count++;
	} else {
		list_add_tail(&newpage->lru, &pool->low_items);
		pool->low_count++;
	}
	mutex_unlock(&pool->mutex);
	put_page(page);
	return 0;
}

static void ion_page_pool_putback(struct page *page)
{
	/*
	 * migrate function either succeeds or returns -EAGAIN, which
	 * results in calling it again until it succeeds, sothis callback
	 * is not needed.
	 */
}

static struct dentry *ion_pool_do_mount(struct file_system_type *fs_type,
				int flags, const char *dev_name, void *data)
{
	static const struct dentry_operations ops = {
		.d_dname = simple_dname,
	};

	return mount_pseudo(fs_type, "ion_pool:", NULL, &ops, 0x77);
}

static struct file_system_type ion_pool_fs = {
	.name		= "ion_pool",
	.mount		= ion_pool_do_mount,
	.kill_sb	= kill_anon_super,
};

static int ion_pool_cnt;
static struct vfsmount *ion_pool_mnt;
static int ion_pool_mount(void)
{
	int ret = 0;

	ion_pool_mnt = kern_mount(&ion_pool_fs);
	if (IS_ERR(ion_pool_mnt))
		ret = PTR_ERR(ion_pool_mnt);

	return ret;
}

static const struct address_space_operations ion_pool_aops = {
	.isolate_page = ion_page_pool_isolate,
	.migratepage = ion_page_pool_migrate,
	.putback_page = ion_page_pool_putback,
};

static int ion_pool_register_migration(struct ion_page_pool *pool)
{
	int  ret = simple_pin_fs(&ion_pool_fs, &ion_pool_mnt, &ion_pool_cnt);

	if (ret < 0) {
		pr_err("Cannot mount pseudo fs: %d\n", ret);
		return ret;
	}
	pool->inode = alloc_anon_inode(ion_pool_mnt->mnt_sb);
	if (IS_ERR(pool->inode)) {
		ret = PTR_ERR(pool->inode);
		pool->inode = NULL;
		simple_release_fs(&ion_pool_mnt, &ion_pool_cnt);
		return ret;
	}

	pool->inode->i_mapping->private_data = pool;
	pool->inode->i_mapping->a_ops = &ion_pool_aops;
	return 0;
}

static void ion_pool_unregister_migration(struct ion_page_pool *pool)
{
	if (pool->inode) {
		iput(pool->inode);
		pool->inode = NULL;
		simple_release_fs(&ion_pool_mnt, &ion_pool_cnt);
	}
}

struct ion_page_pool *ion_page_pool_create(struct device *dev, gfp_t gfp_mask,
					   unsigned int order, bool movable)
{
	struct ion_page_pool *pool = kmalloc(sizeof(*pool), GFP_KERNEL);

	if (!pool)
		return NULL;
	pool->dev = dev;
	pool->high_count = 0;
	pool->low_count = 0;
	INIT_LIST_HEAD(&pool->low_items);
	INIT_LIST_HEAD(&pool->high_items);
	pool->gfp_mask = gfp_mask;
	pool->order = order;
	mutex_init(&pool->mutex);
	plist_node_init(&pool->list, order);

	pool->inode = NULL;
	if (movable)
		ion_pool_register_migration(pool);

	return pool;
}

void ion_page_pool_destroy(struct ion_page_pool *pool)
{
	ion_pool_unregister_migration(pool);
	kfree(pool);
}

static int __init ion_page_pool_init(void)
{
	return ion_pool_mount();
}
device_initcall(ion_page_pool_init);
