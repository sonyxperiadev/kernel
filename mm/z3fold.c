/*
 * z3fold.c
 *
 * Author: Vitaly Wool <vitaly.wool@konsulko.com>
 * Copyright (C) 2016, Sony Mobile Communications Inc.
 *
 * This implementation is based on zbud written by Seth Jennings.
 *
 * z3fold is an special purpose allocator for storing compressed pages. It
 * can store up to three compressed pages per page which improves the
 * compression ratio of zbud while retaining its main concepts (e. g. always
 * storing an integral number of objects per page) and simplicity.
 * It still has simple and deterministic reclaim properties that make it
 * preferable to a higher density approach (with no requirement on integral
 * number of object per page) when reclaim is used.
 *
 * As in zbud, pages are divided into "chunks".  The size of the chunks is
 * fixed at compile time and is determined by NCHUNKS_ORDER below.
 *
 * z3fold doesn't export any API and is meant to be used via zpool API.
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/atomic.h>
#include <linux/sched.h>
#include <linux/list.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/percpu.h>
#include <linux/preempt.h>
#include <linux/workqueue.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/zpool.h>

/*****************
 * Structures
*****************/
struct z3fold_pool;
struct z3fold_ops {
	int (*evict)(struct z3fold_pool *pool, unsigned long handle);
};

enum buddy {
	HEADLESS = 0,
	FIRST,
	MIDDLE,
	LAST,
	BUDDIES_MAX
};

/*
 * struct z3fold_header - z3fold page metadata occupying first chunks of each
 *			z3fold page, except for HEADLESS pages
 * @buddy:		links the z3fold page into the relevant list in the
 *			pool
 * @page_lock:		per-page lock
 * @refcount:		reference count for the z3fold page
 * @work:		work_struct for page layout optimization
 * @pool:		pointer to the pool which this page belongs to
 * @cpu:		CPU which this page "belongs" to
 * @first_chunks:	the size of the first buddy in chunks, 0 if free
 * @middle_chunks:	the size of the middle buddy in chunks, 0 if free
 * @last_chunks:	the size of the last buddy in chunks, 0 if free
 * @first_num:		the starting number (for the first handle)
 */
struct z3fold_header {
	struct list_head buddy;
	spinlock_t page_lock;
	struct kref refcount;
	struct work_struct work;
	struct z3fold_pool *pool;
	short cpu;
	unsigned short first_chunks;
	unsigned short middle_chunks;
	unsigned short last_chunks;
	unsigned short start_middle;
	unsigned short first_num:2;
};

/*
 * NCHUNKS_ORDER determines the internal allocation granularity, effectively
 * adjusting internal fragmentation.  It also determines the number of
 * freelists maintained in each pool. NCHUNKS_ORDER of 6 means that the
 * allocation granularity will be in chunks of size PAGE_SIZE/64. Some chunks
 * in the beginning of an allocated page are occupied by z3fold header, so
 * NCHUNKS will be calculated to 63 (or 62 in case CONFIG_DEBUG_SPINLOCK=y),
 * which shows the max number of free chunks in z3fold page, also there will
 * be 63, or 62, respectively, freelists per pool.
 */
#define NCHUNKS_ORDER	6

#define CHUNK_SHIFT	(PAGE_SHIFT - NCHUNKS_ORDER)
#define CHUNK_SIZE	(1 << CHUNK_SHIFT)
#define ZHDR_SIZE_ALIGNED round_up(sizeof(struct z3fold_header), CHUNK_SIZE)
#define ZHDR_CHUNKS	(ZHDR_SIZE_ALIGNED >> CHUNK_SHIFT)
#define TOTAL_CHUNKS	(PAGE_SIZE >> CHUNK_SHIFT)
#define NCHUNKS		((PAGE_SIZE - ZHDR_SIZE_ALIGNED) >> CHUNK_SHIFT)

#define BUDDY_MASK	(0x3)
#define BUDDY_SHIFT	2

/**
 * struct z3fold_pool - stores metadata for each z3fold pool
 * @name:	pool name
 * @lock:	protects pool unbuddied/lru lists
 * @stale_lock:	protects pool stale page list
 * @unbuddied:	per-cpu array of lists tracking z3fold pages that contain 2-
 *		buddies; the list each z3fold page is added to depends on
 *		the size of its free region.
 * @lru:	list tracking the z3fold pages in LRU order by most recently
 *		added buddy.
 * @stale:	list of pages marked for freeing
 * @pages_nr:	number of z3fold pages in the pool.
 * @ops:	pointer to a structure of user defined operations specified at
 *		pool creation time.
 * @compact_wq:	workqueue for page layout background optimization
 * @release_wq:	workqueue for safe page release
 * @work:	work_struct for safe page release
 *
 * This structure is allocated at pool creation time and maintains metadata
 * pertaining to a particular z3fold pool.
 */
struct z3fold_pool {
	const char *name;
	spinlock_t lock;
	spinlock_t stale_lock;
	struct list_head *unbuddied;
	struct list_head lru;
	struct list_head stale;
	atomic64_t pages_nr;
	const struct z3fold_ops *ops;
	struct zpool *zpool;
	const struct zpool_ops *zpool_ops;
	struct workqueue_struct *compact_wq;
	struct workqueue_struct *release_wq;
	struct work_struct work;
};

/*
 * Internal z3fold page flags
 */
enum z3fold_page_flags {
	PAGE_HEADLESS = 0,
	MIDDLE_CHUNK_MAPPED,
	NEEDS_COMPACTING,
	PAGE_STALE,
	PAGE_CLAIMED, /* by either reclaim or free */
};

/*****************
 * Helpers
*****************/

/* Converts an allocation size in bytes to size in z3fold chunks */
static int size_to_chunks(size_t size)
{
	return (size + CHUNK_SIZE - 1) >> CHUNK_SHIFT;
}

#define for_each_unbuddied_list(_iter, _begin) \
	for ((_iter) = (_begin); (_iter) < NCHUNKS; (_iter)++)

static void compact_page_work(struct work_struct *w);

/* Initializes the z3fold header of a newly allocated z3fold page */
static struct z3fold_header *init_z3fold_page(struct page *page,
					struct z3fold_pool *pool)
{
	struct z3fold_header *zhdr = page_address(page);

	INIT_LIST_HEAD(&page->lru);
	clear_bit(PAGE_HEADLESS, &page->private);
	clear_bit(MIDDLE_CHUNK_MAPPED, &page->private);
	clear_bit(NEEDS_COMPACTING, &page->private);
	clear_bit(PAGE_STALE, &page->private);
	clear_bit(PAGE_CLAIMED, &page->private);

	spin_lock_init(&zhdr->page_lock);
	kref_init(&zhdr->refcount);
	zhdr->first_chunks = 0;
	zhdr->middle_chunks = 0;
	zhdr->last_chunks = 0;
	zhdr->first_num = 0;
	zhdr->start_middle = 0;
	zhdr->cpu = -1;
	zhdr->pool = pool;
	INIT_LIST_HEAD(&zhdr->buddy);
	INIT_WORK(&zhdr->work, compact_page_work);
	return zhdr;
}

/* Resets the struct page fields and frees the page */
static void free_z3fold_page(struct page *page)
{
	__free_page(page);
}

/* Lock a z3fold page */
static inline void z3fold_page_lock(struct z3fold_header *zhdr)
{
	spin_lock(&zhdr->page_lock);
}

/* Try to lock a z3fold page */
static inline int z3fold_page_trylock(struct z3fold_header *zhdr)
{
	return spin_trylock(&zhdr->page_lock);
}

/* Unlock a z3fold page */
static inline void z3fold_page_unlock(struct z3fold_header *zhdr)
{
	spin_unlock(&zhdr->page_lock);
}

/*
 * Encodes the handle of a particular buddy within a z3fold page
 * Pool lock should be held as this function accesses first_num
 */
static unsigned long encode_handle(struct z3fold_header *zhdr, enum buddy bud)
{
	unsigned long handle;

	handle = (unsigned long)zhdr;
	if (bud != HEADLESS) {
		handle |= (bud + zhdr->first_num) & BUDDY_MASK;
		if (bud == LAST)
			handle |= (zhdr->last_chunks << BUDDY_SHIFT);
	}
	return handle;
}

/* Returns the z3fold page where a given handle is stored */
static struct z3fold_header *handle_to_z3fold_header(unsigned long handle)
{
	return (struct z3fold_header *)(handle & PAGE_MASK);
}

/* only for LAST bud, returns zero otherwise */
static unsigned short handle_to_chunks(unsigned long handle)
{
	return (handle & ~PAGE_MASK) >> BUDDY_SHIFT;
}

/*
 * (handle & BUDDY_MASK) < zhdr->first_num is possible in encode_handle
 *  but that doesn't matter. because the masking will result in the
 *  correct buddy number.
 */
static enum buddy handle_to_buddy(unsigned long handle)
{
	struct z3fold_header *zhdr = handle_to_z3fold_header(handle);
	return (handle - zhdr->first_num) & BUDDY_MASK;
}

static void __release_z3fold_page(struct z3fold_header *zhdr, bool locked)
{
	struct page *page = virt_to_page(zhdr);
	struct z3fold_pool *pool = zhdr->pool;

	WARN_ON(!list_empty(&zhdr->buddy));
	set_bit(PAGE_STALE, &page->private);
	clear_bit(NEEDS_COMPACTING, &page->private);
	spin_lock(&pool->lock);
	if (!list_empty(&page->lru))
		list_del(&page->lru);
	spin_unlock(&pool->lock);
	if (locked)
		z3fold_page_unlock(zhdr);
	spin_lock(&pool->stale_lock);
	list_add(&zhdr->buddy, &pool->stale);
	queue_work(pool->release_wq, &pool->work);
	spin_unlock(&pool->stale_lock);
}

static void __attribute__((__unused__))
			release_z3fold_page(struct kref *ref)
{
	struct z3fold_header *zhdr = container_of(ref, struct z3fold_header,
						refcount);
	__release_z3fold_page(zhdr, false);
}

static void release_z3fold_page_locked(struct kref *ref)
{
	struct z3fold_header *zhdr = container_of(ref, struct z3fold_header,
						refcount);
	WARN_ON(z3fold_page_trylock(zhdr));
	__release_z3fold_page(zhdr, true);
}

static void release_z3fold_page_locked_list(struct kref *ref)
{
	struct z3fold_header *zhdr = container_of(ref, struct z3fold_header,
					       refcount);
	spin_lock(&zhdr->pool->lock);
	list_del_init(&zhdr->buddy);
	spin_unlock(&zhdr->pool->lock);

	WARN_ON(z3fold_page_trylock(zhdr));
	__release_z3fold_page(zhdr, true);
}

static void free_pages_work(struct work_struct *w)
{
	struct z3fold_pool *pool = container_of(w, struct z3fold_pool, work);

	spin_lock(&pool->stale_lock);
	while (!list_empty(&pool->stale)) {
		struct z3fold_header *zhdr = list_first_entry(&pool->stale,
						struct z3fold_header, buddy);
		struct page *page = virt_to_page(zhdr);

		list_del(&zhdr->buddy);
		if (WARN_ON(!test_bit(PAGE_STALE, &page->private)))
			continue;
		spin_unlock(&pool->stale_lock);
		cancel_work_sync(&zhdr->work);
		free_z3fold_page(page);
		cond_resched();
		spin_lock(&pool->stale_lock);
	}
	spin_unlock(&pool->stale_lock);
}

/*
 * Returns the number of free chunks in a z3fold page.
 * NB: can't be used with HEADLESS pages.
 */
static int num_free_chunks(struct z3fold_header *zhdr)
{
	int nfree;
	/*
	 * If there is a middle object, pick up the bigger free space
	 * either before or after it. Otherwise just subtract the number
	 * of chunks occupied by the first and the last objects.
	 */
	if (zhdr->middle_chunks != 0) {
		int nfree_before = zhdr->first_chunks ?
			0 : zhdr->start_middle - ZHDR_CHUNKS;
		int nfree_after = zhdr->last_chunks ?
			0 : TOTAL_CHUNKS -
				(zhdr->start_middle + zhdr->middle_chunks);
		nfree = max(nfree_before, nfree_after);
	} else
		nfree = NCHUNKS - zhdr->first_chunks - zhdr->last_chunks;
	return nfree;
}

static inline void *mchunk_memmove(struct z3fold_header *zhdr,
				unsigned short dst_chunk)
{
	void *beg = zhdr;
	return memmove(beg + (dst_chunk << CHUNK_SHIFT),
		       beg + (zhdr->start_middle << CHUNK_SHIFT),
		       zhdr->middle_chunks << CHUNK_SHIFT);
}

#define BIG_CHUNK_GAP	3
/* Has to be called with lock held */
static int z3fold_compact_page(struct z3fold_header *zhdr)
{
	struct page *page = virt_to_page(zhdr);

	if (test_bit(MIDDLE_CHUNK_MAPPED, &page->private))
		return 0; /* can't move middle chunk, it's used */

	if (zhdr->middle_chunks == 0)
		return 0; /* nothing to compact */

	if (zhdr->first_chunks == 0 && zhdr->last_chunks == 0) {
		/* move to the beginning */
		mchunk_memmove(zhdr, ZHDR_CHUNKS);
		zhdr->first_chunks = zhdr->middle_chunks;
		zhdr->middle_chunks = 0;
		zhdr->start_middle = 0;
		zhdr->first_num++;
		return 1;
	}

	/*
	 * moving data is expensive, so let's only do that if
	 * there's substantial gain (at least BIG_CHUNK_GAP chunks)
	 */
	if (zhdr->first_chunks != 0 && zhdr->last_chunks == 0 &&
	    zhdr->start_middle - (zhdr->first_chunks + ZHDR_CHUNKS) >=
			BIG_CHUNK_GAP) {
		mchunk_memmove(zhdr, zhdr->first_chunks + ZHDR_CHUNKS);
		zhdr->start_middle = zhdr->first_chunks + ZHDR_CHUNKS;
		return 1;
	} else if (zhdr->last_chunks != 0 && zhdr->first_chunks == 0 &&
		   TOTAL_CHUNKS - (zhdr->last_chunks + zhdr->start_middle
					+ zhdr->middle_chunks) >=
			BIG_CHUNK_GAP) {
		unsigned short new_start = TOTAL_CHUNKS - zhdr->last_chunks -
			zhdr->middle_chunks;
		mchunk_memmove(zhdr, new_start);
		zhdr->start_middle = new_start;
		return 1;
	}

	return 0;
}

static void do_compact_page(struct z3fold_header *zhdr, bool locked)
{
	struct z3fold_pool *pool = zhdr->pool;
	struct page *page;
	struct list_head *unbuddied;
	int fchunks;

	page = virt_to_page(zhdr);
	if (locked)
		WARN_ON(z3fold_page_trylock(zhdr));
	else
		z3fold_page_lock(zhdr);
	if (WARN_ON(!test_and_clear_bit(NEEDS_COMPACTING, &page->private))) {
		z3fold_page_unlock(zhdr);
		return;
	}
	spin_lock(&pool->lock);
	list_del_init(&zhdr->buddy);
	spin_unlock(&pool->lock);

	if (kref_put(&zhdr->refcount, release_z3fold_page_locked)) {
		atomic64_dec(&pool->pages_nr);
		return;
	}

	z3fold_compact_page(zhdr);
	unbuddied = get_cpu_ptr(pool->unbuddied);
	fchunks = num_free_chunks(zhdr);
	if (fchunks < NCHUNKS &&
	    (!zhdr->first_chunks || !zhdr->middle_chunks ||
			!zhdr->last_chunks)) {
		/* the page's not completely free and it's unbuddied */
		spin_lock(&pool->lock);
		list_add(&zhdr->buddy, &unbuddied[fchunks]);
		spin_unlock(&pool->lock);
		zhdr->cpu = smp_processor_id();
	}
	put_cpu_ptr(pool->unbuddied);
	z3fold_page_unlock(zhdr);
}

static void compact_page_work(struct work_struct *w)
{
	struct z3fold_header *zhdr = container_of(w, struct z3fold_header,
						work);

	do_compact_page(zhdr, false);
}


/*
 * API Functions
 */

/**
 * z3fold_create_pool() - create a new z3fold pool
 * @name:	pool name
 * @gfp:	gfp flags when allocating the z3fold pool structure
 * @ops:	user-defined operations for the z3fold pool
 *
 * Return: pointer to the new z3fold pool or NULL if the metadata allocation
 * failed.
 */
static struct z3fold_pool *z3fold_create_pool(const char *name, gfp_t gfp,
		const struct z3fold_ops *ops)
{
	struct z3fold_pool *pool = NULL;
	int i, cpu;

	pool = kzalloc(sizeof(struct z3fold_pool), gfp);
	if (!pool)
		goto out;
	spin_lock_init(&pool->lock);
	spin_lock_init(&pool->stale_lock);
	pool->unbuddied = __alloc_percpu(sizeof(struct list_head)*NCHUNKS, 2);
	if (!pool->unbuddied)
		goto out_pool;
	for_each_possible_cpu(cpu) {
		struct list_head *unbuddied =
				per_cpu_ptr(pool->unbuddied, cpu);
		for_each_unbuddied_list(i, 0)
			INIT_LIST_HEAD(&unbuddied[i]);
	}
	INIT_LIST_HEAD(&pool->lru);
	INIT_LIST_HEAD(&pool->stale);
	atomic64_set(&pool->pages_nr, 0);
	pool->name = name;
	pool->compact_wq = create_singlethread_workqueue(pool->name);
	if (!pool->compact_wq)
		goto out_unbuddied;
	pool->release_wq = create_singlethread_workqueue(pool->name);
	if (!pool->release_wq)
		goto out_wq;
	INIT_WORK(&pool->work, free_pages_work);
	pool->ops = ops;
	return pool;

out_wq:
	destroy_workqueue(pool->compact_wq);
out_unbuddied:
	free_percpu(pool->unbuddied);
out_pool:
	kfree(pool);
out:
	return NULL;
}

/**
 * z3fold_destroy_pool() - destroys an existing z3fold pool
 * @pool:	the z3fold pool to be destroyed
 *
 * The pool should be emptied before this function is called.
 */
static void z3fold_destroy_pool(struct z3fold_pool *pool)
{
	destroy_workqueue(pool->release_wq);
	destroy_workqueue(pool->compact_wq);
	kfree(pool);
}

/**
 * z3fold_alloc() - allocates a region of a given size
 * @pool:	z3fold pool from which to allocate
 * @size:	size in bytes of the desired allocation
 * @gfp:	gfp flags used if the pool needs to grow
 * @handle:	handle of the new allocation
 *
 * This function will attempt to find a free region in the pool large enough to
 * satisfy the allocation request.  A search of the unbuddied lists is
 * performed first. If no suitable free region is found, then a new page is
 * allocated and added to the pool to satisfy the request.
 *
 * gfp should not set __GFP_HIGHMEM as highmem pages cannot be used
 * as z3fold pool pages.
 *
 * Return: 0 if success and handle is set, otherwise -EINVAL if the size or
 * gfp arguments are invalid or -ENOMEM if the pool was unable to allocate
 * a new page.
 */
static int z3fold_alloc(struct z3fold_pool *pool, size_t size, gfp_t gfp,
			unsigned long *handle)
{
	int chunks = 0, i, freechunks;
	struct z3fold_header *zhdr = NULL;
	struct page *page = NULL;
	enum buddy bud;
	bool can_sleep = gfpflags_allow_blocking(gfp);

	if (!size)
		return -EINVAL;

	if (size > PAGE_SIZE)
		return -ENOSPC;

	gfp &= ~__GFP_HIGHMEM;
	if (size > PAGE_SIZE - ZHDR_SIZE_ALIGNED - CHUNK_SIZE)
		bud = HEADLESS;
	else {
		struct list_head *unbuddied;
		chunks = size_to_chunks(size);

lookup:
		/* First, try to find an unbuddied z3fold page. */
		unbuddied = get_cpu_ptr(pool->unbuddied);
		for_each_unbuddied_list(i, chunks) {
			struct list_head *l = &unbuddied[i];

			zhdr = list_first_entry_or_null(READ_ONCE(l),
						struct z3fold_header, buddy);

			if (!zhdr)
				continue;

			/* Re-check under lock. */
			spin_lock(&pool->lock);
			l = &unbuddied[i];
			if (unlikely(zhdr != list_first_entry(READ_ONCE(l),
					struct z3fold_header, buddy)) ||
			    !z3fold_page_trylock(zhdr)) {
				spin_unlock(&pool->lock);
				put_cpu_ptr(pool->unbuddied);
				goto lookup;
			}
			list_del_init(&zhdr->buddy);
			zhdr->cpu = -1;
			spin_unlock(&pool->lock);

			page = virt_to_page(zhdr);
			if (test_bit(NEEDS_COMPACTING, &page->private)) {
				z3fold_page_unlock(zhdr);
				zhdr = NULL;
				put_cpu_ptr(pool->unbuddied);
				if (can_sleep)
					cond_resched();
				goto lookup;
			}

			/*
			 * this page could not be removed from its unbuddied
			 * list while pool lock was held, and then we've taken
			 * page lock so kref_put could not be called before
			 * we got here, so it's safe to just call kref_get()
			 */
			kref_get(&zhdr->refcount);
			break;
		}
		put_cpu_ptr(pool->unbuddied);

		if (zhdr) {
			if (zhdr->first_chunks == 0) {
				if (zhdr->middle_chunks != 0 &&
				    chunks >= zhdr->start_middle)
					bud = LAST;
				else
					bud = FIRST;
			} else if (zhdr->last_chunks == 0)
				bud = LAST;
			else if (zhdr->middle_chunks == 0)
				bud = MIDDLE;
			else {
				if (kref_put(&zhdr->refcount,
					     release_z3fold_page_locked))
					atomic64_dec(&pool->pages_nr);
				else
					z3fold_page_unlock(zhdr);
				pr_err("No free chunks in unbuddied\n");
				WARN_ON(1);
				goto lookup;
			}
			goto found;
		}
		bud = FIRST;
	}

	page = NULL;
	if (can_sleep) {
		spin_lock(&pool->stale_lock);
		zhdr = list_first_entry_or_null(&pool->stale,
						struct z3fold_header, buddy);
		/*
		 * Before allocating a page, let's see if we can take one from
		 * the stale pages list. cancel_work_sync() can sleep so we
		 * limit this case to the contexts where we can sleep
		 */
		if (zhdr) {
			list_del(&zhdr->buddy);
			spin_unlock(&pool->stale_lock);
			cancel_work_sync(&zhdr->work);
			page = virt_to_page(zhdr);
		} else {
			spin_unlock(&pool->stale_lock);
		}
	}
	if (!page)
		page = alloc_page(gfp);

	if (!page)
		return -ENOMEM;

	atomic64_inc(&pool->pages_nr);
	zhdr = init_z3fold_page(page, pool);

	if (bud == HEADLESS) {
		set_bit(PAGE_HEADLESS, &page->private);
		goto headless;
	}
	z3fold_page_lock(zhdr);

found:
	if (bud == FIRST)
		zhdr->first_chunks = chunks;
	else if (bud == LAST)
		zhdr->last_chunks = chunks;
	else {
		zhdr->middle_chunks = chunks;
		zhdr->start_middle = zhdr->first_chunks + ZHDR_CHUNKS;
	}

	if (zhdr->first_chunks == 0 || zhdr->last_chunks == 0 ||
			zhdr->middle_chunks == 0) {
		struct list_head *unbuddied = get_cpu_ptr(pool->unbuddied);

		/* Add to unbuddied list */
		freechunks = num_free_chunks(zhdr);
		spin_lock(&pool->lock);
		list_add(&zhdr->buddy, &unbuddied[freechunks]);
		spin_unlock(&pool->lock);
		zhdr->cpu = smp_processor_id();
		put_cpu_ptr(pool->unbuddied);
	}

headless:
	spin_lock(&pool->lock);
	/* Add/move z3fold page to beginning of LRU */
	if (!list_empty(&page->lru))
		list_del(&page->lru);

	list_add(&page->lru, &pool->lru);

	*handle = encode_handle(zhdr, bud);
	spin_unlock(&pool->lock);
	if (bud != HEADLESS)
		z3fold_page_unlock(zhdr);

	return 0;
}

/**
 * z3fold_free() - frees the allocation associated with the given handle
 * @pool:	pool in which the allocation resided
 * @handle:	handle associated with the allocation returned by z3fold_alloc()
 *
 * In the case that the z3fold page in which the allocation resides is under
 * reclaim, as indicated by the PG_reclaim flag being set, this function
 * only sets the first|last_chunks to 0.  The page is actually freed
 * once both buddies are evicted (see z3fold_reclaim_page() below).
 */
static void z3fold_free(struct z3fold_pool *pool, unsigned long handle)
{
	struct z3fold_header *zhdr;
	struct page *page;
	enum buddy bud;

	zhdr = handle_to_z3fold_header(handle);
	page = virt_to_page(zhdr);

	if (test_bit(PAGE_HEADLESS, &page->private)) {
		/* if a headless page is under reclaim, just leave.
		 * NB: we use test_and_set_bit for a reason: if the bit
		 * has not been set before, we release this page
		 * immediately so we don't care about its value any more.
		 */
		if (!test_and_set_bit(PAGE_CLAIMED, &page->private)) {
			spin_lock(&pool->lock);
			list_del(&page->lru);
			spin_unlock(&pool->lock);
			free_z3fold_page(page);
			atomic64_dec(&pool->pages_nr);
		}
		return;
	}

	/* Non-headless case */
	z3fold_page_lock(zhdr);
	bud = handle_to_buddy(handle);

	switch (bud) {
	case FIRST:
		zhdr->first_chunks = 0;
		break;
	case MIDDLE:
		zhdr->middle_chunks = 0;
		break;
	case LAST:
		zhdr->last_chunks = 0;
		break;
	default:
		pr_err("%s: unknown bud %d\n", __func__, bud);
		WARN_ON(1);
		z3fold_page_unlock(zhdr);
		return;
	}

	if (kref_put(&zhdr->refcount, release_z3fold_page_locked_list)) {
		atomic64_dec(&pool->pages_nr);
		return;
	}
	if (test_bit(PAGE_CLAIMED, &page->private)) {
		z3fold_page_unlock(zhdr);
		return;
	}
	if (test_and_set_bit(NEEDS_COMPACTING, &page->private)) {
		z3fold_page_unlock(zhdr);
		return;
	}
	if (zhdr->cpu < 0 || !cpu_online(zhdr->cpu)) {
		spin_lock(&pool->lock);
		list_del_init(&zhdr->buddy);
		spin_unlock(&pool->lock);
		zhdr->cpu = -1;
		kref_get(&zhdr->refcount);
		do_compact_page(zhdr, true);
		return;
	}
	kref_get(&zhdr->refcount);
	queue_work_on(zhdr->cpu, pool->compact_wq, &zhdr->work);
	z3fold_page_unlock(zhdr);
}

/**
 * z3fold_reclaim_page() - evicts allocations from a pool page and frees it
 * @pool:	pool from which a page will attempt to be evicted
 * @retires:	number of pages on the LRU list for which eviction will
 *		be attempted before failing
 *
 * z3fold reclaim is different from normal system reclaim in that it is done
 * from the bottom, up. This is because only the bottom layer, z3fold, has
 * information on how the allocations are organized within each z3fold page.
 * This has the potential to create interesting locking situations between
 * z3fold and the user, however.
 *
 * To avoid these, this is how z3fold_reclaim_page() should be called:

 * The user detects a page should be reclaimed and calls z3fold_reclaim_page().
 * z3fold_reclaim_page() will remove a z3fold page from the pool LRU list and
 * call the user-defined eviction handler with the pool and handle as
 * arguments.
 *
 * If the handle can not be evicted, the eviction handler should return
 * non-zero. z3fold_reclaim_page() will add the z3fold page back to the
 * appropriate list and try the next z3fold page on the LRU up to
 * a user defined number of retries.
 *
 * If the handle is successfully evicted, the eviction handler should
 * return 0 _and_ should have called z3fold_free() on the handle. z3fold_free()
 * contains logic to delay freeing the page if the page is under reclaim,
 * as indicated by the setting of the PG_reclaim flag on the underlying page.
 *
 * If all buddies in the z3fold page are successfully evicted, then the
 * z3fold page can be freed.
 *
 * Returns: 0 if page is successfully freed, otherwise -EINVAL if there are
 * no pages to evict or an eviction handler is not registered, -EAGAIN if
 * the retry limit was hit.
 */
static int z3fold_reclaim_page(struct z3fold_pool *pool, unsigned int retries)
{
	int i, ret = 0;
	struct z3fold_header *zhdr = NULL;
	struct page *page = NULL;
	struct list_head *pos;
	unsigned long first_handle = 0, middle_handle = 0, last_handle = 0;

	spin_lock(&pool->lock);
	if (!pool->ops || !pool->ops->evict || retries == 0) {
		spin_unlock(&pool->lock);
		return -EINVAL;
	}
	for (i = 0; i < retries; i++) {
		if (list_empty(&pool->lru)) {
			spin_unlock(&pool->lock);
			return -EINVAL;
		}
		list_for_each_prev(pos, &pool->lru) {
			page = list_entry(pos, struct page, lru);

			/* this bit could have been set by free, in which case
			 * we pass over to the next page in the pool.
			 */
			if (test_and_set_bit(PAGE_CLAIMED, &page->private))
				continue;

			zhdr = page_address(page);
			if (test_bit(PAGE_HEADLESS, &page->private))
				break;

			if (!z3fold_page_trylock(zhdr)) {
				zhdr = NULL;
				continue; /* can't evict at this point */
			}
			kref_get(&zhdr->refcount);
			list_del_init(&zhdr->buddy);
			zhdr->cpu = -1;
			break;
		}

		if (!zhdr)
			break;

		list_del_init(&page->lru);
		spin_unlock(&pool->lock);

		if (!test_bit(PAGE_HEADLESS, &page->private)) {
			/*
			 * We need encode the handles before unlocking, since
			 * we can race with free that will set
			 * (first|last)_chunks to 0
			 */
			first_handle = 0;
			last_handle = 0;
			middle_handle = 0;
			if (zhdr->first_chunks)
				first_handle = encode_handle(zhdr, FIRST);
			if (zhdr->middle_chunks)
				middle_handle = encode_handle(zhdr, MIDDLE);
			if (zhdr->last_chunks)
				last_handle = encode_handle(zhdr, LAST);
			/*
			 * it's safe to unlock here because we hold a
			 * reference to this page
			 */
			z3fold_page_unlock(zhdr);
		} else {
			first_handle = encode_handle(zhdr, HEADLESS);
			last_handle = middle_handle = 0;
		}

		/* Issue the eviction callback(s) */
		if (middle_handle) {
			ret = pool->ops->evict(pool, middle_handle);
			if (ret)
				goto next;
		}
		if (first_handle) {
			ret = pool->ops->evict(pool, first_handle);
			if (ret)
				goto next;
		}
		if (last_handle) {
			ret = pool->ops->evict(pool, last_handle);
			if (ret)
				goto next;
		}
next:
		if (test_bit(PAGE_HEADLESS, &page->private)) {
			if (ret == 0) {
				free_z3fold_page(page);
				atomic64_dec(&pool->pages_nr);
				return 0;
			}
			spin_lock(&pool->lock);
			list_add(&page->lru, &pool->lru);
			spin_unlock(&pool->lock);
		} else {
			z3fold_page_lock(zhdr);
			clear_bit(PAGE_CLAIMED, &page->private);
			if (kref_put(&zhdr->refcount,
					release_z3fold_page_locked)) {
				atomic64_dec(&pool->pages_nr);
				return 0;
			}
			/*
			 * if we are here, the page is still not completely
			 * free. Take the global pool lock then to be able
			 * to add it back to the lru list
			 */
			spin_lock(&pool->lock);
			list_add(&page->lru, &pool->lru);
			spin_unlock(&pool->lock);
			z3fold_page_unlock(zhdr);
		}

		/* We started off locked to we need to lock the pool back */
		spin_lock(&pool->lock);
	}
	spin_unlock(&pool->lock);
	return -EAGAIN;
}

/**
 * z3fold_map() - maps the allocation associated with the given handle
 * @pool:	pool in which the allocation resides
 * @handle:	handle associated with the allocation to be mapped
 *
 * Extracts the buddy number from handle and constructs the pointer to the
 * correct starting chunk within the page.
 *
 * Returns: a pointer to the mapped allocation
 */
static void *z3fold_map(struct z3fold_pool *pool, unsigned long handle)
{
	struct z3fold_header *zhdr;
	struct page *page;
	void *addr;
	enum buddy buddy;

	zhdr = handle_to_z3fold_header(handle);
	addr = zhdr;
	page = virt_to_page(zhdr);

	if (test_bit(PAGE_HEADLESS, &page->private))
		goto out;

	z3fold_page_lock(zhdr);
	buddy = handle_to_buddy(handle);
	switch (buddy) {
	case FIRST:
		addr += ZHDR_SIZE_ALIGNED;
		break;
	case MIDDLE:
		addr += zhdr->start_middle << CHUNK_SHIFT;
		set_bit(MIDDLE_CHUNK_MAPPED, &page->private);
		break;
	case LAST:
		addr += PAGE_SIZE - (handle_to_chunks(handle) << CHUNK_SHIFT);
		break;
	default:
		pr_err("unknown buddy id %d\n", buddy);
		WARN_ON(1);
		addr = NULL;
		break;
	}

	z3fold_page_unlock(zhdr);
out:
	return addr;
}

/**
 * z3fold_unmap() - unmaps the allocation associated with the given handle
 * @pool:	pool in which the allocation resides
 * @handle:	handle associated with the allocation to be unmapped
 */
static void z3fold_unmap(struct z3fold_pool *pool, unsigned long handle)
{
	struct z3fold_header *zhdr;
	struct page *page;
	enum buddy buddy;

	zhdr = handle_to_z3fold_header(handle);
	page = virt_to_page(zhdr);

	if (test_bit(PAGE_HEADLESS, &page->private))
		return;

	z3fold_page_lock(zhdr);
	buddy = handle_to_buddy(handle);
	if (buddy == MIDDLE)
		clear_bit(MIDDLE_CHUNK_MAPPED, &page->private);
	z3fold_page_unlock(zhdr);
}

/**
 * z3fold_get_pool_size() - gets the z3fold pool size in pages
 * @pool:	pool whose size is being queried
 *
 * Returns: size in pages of the given pool.
 */
static u64 z3fold_get_pool_size(struct z3fold_pool *pool)
{
	return atomic64_read(&pool->pages_nr);
}

/*****************
 * zpool
 ****************/

static int z3fold_zpool_evict(struct z3fold_pool *pool, unsigned long handle)
{
	if (pool->zpool && pool->zpool_ops && pool->zpool_ops->evict)
		return pool->zpool_ops->evict(pool->zpool, handle);
	else
		return -ENOENT;
}

static const struct z3fold_ops z3fold_zpool_ops = {
	.evict =	z3fold_zpool_evict
};

static void *z3fold_zpool_create(const char *name, gfp_t gfp,
			       const struct zpool_ops *zpool_ops,
			       struct zpool *zpool)
{
	struct z3fold_pool *pool;

	pool = z3fold_create_pool(name, gfp,
				zpool_ops ? &z3fold_zpool_ops : NULL);
	if (pool) {
		pool->zpool = zpool;
		pool->zpool_ops = zpool_ops;
	}
	return pool;
}

static void z3fold_zpool_destroy(void *pool)
{
	z3fold_destroy_pool(pool);
}

static int z3fold_zpool_malloc(void *pool, size_t size, gfp_t gfp,
			unsigned long *handle)
{
	return z3fold_alloc(pool, size, gfp, handle);
}
static void z3fold_zpool_free(void *pool, unsigned long handle)
{
	z3fold_free(pool, handle);
}

static int z3fold_zpool_shrink(void *pool, unsigned int pages,
			unsigned int *reclaimed)
{
	unsigned int total = 0;
	int ret = -EINVAL;

	while (total < pages) {
		ret = z3fold_reclaim_page(pool, 8);
		if (ret < 0)
			break;
		total++;
	}

	if (reclaimed)
		*reclaimed = total;

	return ret;
}

static void *z3fold_zpool_map(void *pool, unsigned long handle,
			enum zpool_mapmode mm)
{
	return z3fold_map(pool, handle);
}
static void z3fold_zpool_unmap(void *pool, unsigned long handle)
{
	z3fold_unmap(pool, handle);
}

static u64 z3fold_zpool_total_size(void *pool)
{
	return z3fold_get_pool_size(pool) * PAGE_SIZE;
}

static struct zpool_driver z3fold_zpool_driver = {
	.type =		"z3fold",
	.owner =	THIS_MODULE,
	.create =	z3fold_zpool_create,
	.destroy =	z3fold_zpool_destroy,
	.malloc =	z3fold_zpool_malloc,
	.free =		z3fold_zpool_free,
	.shrink =	z3fold_zpool_shrink,
	.map =		z3fold_zpool_map,
	.unmap =	z3fold_zpool_unmap,
	.total_size =	z3fold_zpool_total_size,
};

MODULE_ALIAS("zpool-z3fold");

static int __init init_z3fold(void)
{
	/* Make sure the z3fold header is not larger than the page size */
	BUILD_BUG_ON(ZHDR_SIZE_ALIGNED > PAGE_SIZE);
	zpool_register_driver(&z3fold_zpool_driver);

	return 0;
}

static void __exit exit_z3fold(void)
{
	zpool_unregister_driver(&z3fold_zpool_driver);
}

module_init(init_z3fold);
module_exit(exit_z3fold);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Vitaly Wool <vitalywool@gmail.com>");
MODULE_DESCRIPTION("3-Fold Allocator for Compressed Pages");
