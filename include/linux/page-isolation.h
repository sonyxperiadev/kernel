#ifndef __LINUX_PAGEISOLATION_H
#define __LINUX_PAGEISOLATION_H

/*
 * Changes migrate type in [start_pfn, end_pfn) to be MIGRATE_ISOLATE.
 * If specified range includes migrate types other than MOVABLE or CMA,
 * this will fail with -EBUSY.
 *
 * For isolating all pages in the range finally, the caller have to
 * free all pages in the range. test_page_isolated() can be used for
 * test it.
 */
int __start_isolate_page_range(unsigned long start_pfn, unsigned long end_pfn,
			       unsigned migratetype);

static inline int
start_isolate_page_range(unsigned long start_pfn, unsigned long end_pfn)
{
	return __start_isolate_page_range(start_pfn, end_pfn, MIGRATE_MOVABLE);
}

int __undo_isolate_page_range(unsigned long start_pfn, unsigned long end_pfn,
			      unsigned migratetype);

/*
 * Changes MIGRATE_ISOLATE to MIGRATE_MOVABLE.
 * target range is [start_pfn, end_pfn)
 */
static inline int
undo_isolate_page_range(unsigned long start_pfn, unsigned long end_pfn)
{
	return __undo_isolate_page_range(start_pfn, end_pfn, MIGRATE_MOVABLE);
}

/*
 * Test all pages in [start_pfn, end_pfn) are isolated or not.
 */
int test_pages_isolated(unsigned long start_pfn, unsigned long end_pfn);

/*
 * Internal functions. Changes pageblock's migrate type.
 */
int set_migratetype_isolate(struct page *page);
void __unset_migratetype_isolate(struct page *page, unsigned migratetype);
static inline void unset_migratetype_isolate(struct page *page)
{
	__unset_migratetype_isolate(page, MIGRATE_MOVABLE);
}

/* The below functions must be run on a range from a single zone. */
extern unsigned long alloc_contig_freed_pages(unsigned long start,
					      unsigned long end, gfp_t flag);
extern int alloc_contig_range(unsigned long start, unsigned long end,
			      gfp_t flags, unsigned migratetype);
extern void free_contig_pages(unsigned long pfn, unsigned nr_pages);

/*
 * For migration.
 */

int test_pages_in_a_zone(unsigned long start_pfn, unsigned long end_pfn);
unsigned long scan_lru_pages(unsigned long start, unsigned long end);
int do_migrate_range(unsigned long start_pfn, unsigned long end_pfn);

extern void init_cma_reserved_pageblock(struct page *page);
#endif
