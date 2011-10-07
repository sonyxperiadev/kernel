/*
 * linux/mm/page_isolation.c
 */

#include <linux/mm.h>
#include <linux/page-isolation.h>
#include <linux/pageblock-flags.h>
#include <linux/memcontrol.h>
#include <linux/migrate.h>
#include <linux/mm_inline.h>
#include "internal.h"

static inline struct page *
__first_valid_page(unsigned long pfn, unsigned long nr_pages)
{
	int i;
	for (i = 0; i < nr_pages; i++)
		if (pfn_valid_within(pfn + i))
			break;
	if (unlikely(i == nr_pages))
		return NULL;
	return pfn_to_page(pfn + i);
}

/*
 * start_isolate_page_range() -- make page-allocation-type of range of pages
 * to be MIGRATE_ISOLATE.
 * @start_pfn: The lower PFN of the range to be isolated.
 * @end_pfn: The upper PFN of the range to be isolated.
 *
 * Making page-allocation-type to be MIGRATE_ISOLATE means free pages in
 * the range will never be allocated. Any free pages and pages freed in the
 * future will not be allocated again.
 *
 * start_pfn/end_pfn must be aligned to pageblock_order.
 * Returns 0 on success and -EBUSY if any part of range cannot be isolated.
 */
int
start_isolate_page_range(unsigned long start_pfn, unsigned long end_pfn)
{
	unsigned long pfn;
	unsigned long undo_pfn;
	struct page *page;

	BUG_ON((start_pfn) & (pageblock_nr_pages - 1));
	BUG_ON((end_pfn) & (pageblock_nr_pages - 1));

	for (pfn = start_pfn;
	     pfn < end_pfn;
	     pfn += pageblock_nr_pages) {
		page = __first_valid_page(pfn, pageblock_nr_pages);
		if (page && set_migratetype_isolate(page)) {
			undo_pfn = pfn;
			goto undo;
		}
	}
	return 0;
undo:
	for (pfn = start_pfn;
	     pfn < undo_pfn;
	     pfn += pageblock_nr_pages)
		unset_migratetype_isolate(pfn_to_page(pfn));

	return -EBUSY;
}

/*
 * Make isolated pages available again.
 */
int
undo_isolate_page_range(unsigned long start_pfn, unsigned long end_pfn)
{
	unsigned long pfn;
	struct page *page;
	BUG_ON((start_pfn) & (pageblock_nr_pages - 1));
	BUG_ON((end_pfn) & (pageblock_nr_pages - 1));
	for (pfn = start_pfn;
	     pfn < end_pfn;
	     pfn += pageblock_nr_pages) {
		page = __first_valid_page(pfn, pageblock_nr_pages);
		if (!page || get_pageblock_migratetype(page) != MIGRATE_ISOLATE)
			continue;
		unset_migratetype_isolate(page);
	}
	return 0;
}
/*
 * Test all pages in the range is free(means isolated) or not.
 * all pages in [start_pfn...end_pfn) must be in the same zone.
 * zone->lock must be held before call this.
 *
 * Returns 1 if all pages in the range is isolated.
 */
static int
__test_page_isolated_in_pageblock(unsigned long pfn, unsigned long end_pfn)
{
	struct page *page;

	while (pfn < end_pfn) {
		if (!pfn_valid_within(pfn)) {
			pfn++;
			continue;
		}
		page = pfn_to_page(pfn);
		if (PageBuddy(page))
			pfn += 1 << page_order(page);
		else if (page_count(page) == 0 &&
				page_private(page) == MIGRATE_ISOLATE)
			pfn += 1;
		else
			break;
	}
	if (pfn < end_pfn)
		return 0;
	return 1;
}

int test_pages_isolated(unsigned long start_pfn, unsigned long end_pfn)
{
	unsigned long pfn, flags;
	struct page *page;
	struct zone *zone;
	int ret;

	/*
	 * Note: pageblock_nr_page != MAX_ORDER. Then, chunks of free page
	 * is not aligned to pageblock_nr_pages.
	 * Then we just check pagetype fist.
	 */
	for (pfn = start_pfn; pfn < end_pfn; pfn += pageblock_nr_pages) {
		page = __first_valid_page(pfn, pageblock_nr_pages);
		if (page && get_pageblock_migratetype(page) != MIGRATE_ISOLATE)
			break;
	}
	page = __first_valid_page(start_pfn, end_pfn - start_pfn);
	if ((pfn < end_pfn) || !page)
		return -EBUSY;
	/* Check all pages are free or Marked as ISOLATED */
	zone = page_zone(page);
	spin_lock_irqsave(&zone->lock, flags);
	ret = __test_page_isolated_in_pageblock(start_pfn, end_pfn);
	spin_unlock_irqrestore(&zone->lock, flags);
	return ret ? 0 : -EBUSY;
}


/*
 * Confirm all pages in a range [start, end) is belongs to the same zone.
 */
int test_pages_in_a_zone(unsigned long start_pfn, unsigned long end_pfn)
{
	unsigned long pfn;
	struct zone *zone = NULL;
	struct page *page;
	int i;
	for (pfn = start_pfn;
	     pfn < end_pfn;
	     pfn += MAX_ORDER_NR_PAGES) {
		i = 0;
		/* This is just a CONFIG_HOLES_IN_ZONE check.*/
		while ((i < MAX_ORDER_NR_PAGES) && !pfn_valid_within(pfn + i))
			i++;
		if (i == MAX_ORDER_NR_PAGES)
			continue;
		page = pfn_to_page(pfn + i);
		if (zone && page_zone(page) != zone)
			return 0;
		zone = page_zone(page);
	}
	return 1;
}

/*
 * Scanning pfn is much easier than scanning lru list.
 * Scan pfn from start to end and Find LRU page.
 */
unsigned long scan_lru_pages(unsigned long start, unsigned long end)
{
	unsigned long pfn;
	struct page *page;
	for (pfn = start; pfn < end; pfn++) {
		if (pfn_valid(pfn)) {
			page = pfn_to_page(pfn);
			if (PageLRU(page))
				return pfn;
		}
	}
	return 0;
}

struct page *
hotremove_migrate_alloc(struct page *page, unsigned long private, int **x)
{
	/* This should be improooooved!! */
	return alloc_page(GFP_HIGHUSER_MOVABLE);
}

#define NR_OFFLINE_AT_ONCE_PAGES	(256)
int do_migrate_range(unsigned long start_pfn, unsigned long end_pfn)
{
	unsigned long pfn;
	struct page *page;
	int move_pages = NR_OFFLINE_AT_ONCE_PAGES;
	int not_managed = 0;
	int ret = 0;
	LIST_HEAD(source);

	for (pfn = start_pfn; pfn < end_pfn && move_pages > 0; pfn++) {
		if (!pfn_valid(pfn))
			continue;
		page = pfn_to_page(pfn);
		if (!get_page_unless_zero(page))
			continue;
		/*
		 * We can skip free pages. And we can only deal with pages on
		 * LRU.
		 */
		ret = isolate_lru_page(page);
		if (!ret) { /* Success */
			put_page(page);
			list_add_tail(&page->lru, &source);
			move_pages--;
			inc_zone_page_state(page, NR_ISOLATED_ANON +
					    page_is_file_cache(page));

		} else {
#ifdef CONFIG_DEBUG_VM
			printk(KERN_ALERT "removing pfn %lx from LRU failed\n",
			       pfn);
			dump_page(page);
#endif
			put_page(page);
			/* Because we don't have big zone->lock. we should
			   check this again here. */
			if (page_count(page)) {
				not_managed++;
				ret = -EBUSY;
				break;
			}
		}
	}
	if (!list_empty(&source)) {
		if (not_managed) {
			putback_lru_pages(&source);
			goto out;
		}
		/* this function returns # of failed pages */
		ret = migrate_pages(&source, hotremove_migrate_alloc, 0,
								true, true);
		if (ret)
			putback_lru_pages(&source);
	}
out:
	return ret;
}
