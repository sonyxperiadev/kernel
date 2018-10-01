#ifndef __BP_PAGE_REF_H
#define __BP_PAGE_REF_H
#include <linux/version.h>
#if LINUX_VERSION_IS_GEQ(4,6,0)
#include_next <linux/page_ref.h>
#else
static inline void page_ref_inc(struct page *page)
{
	atomic_inc(&page->_count);
}
#endif

#endif /* __BP_PAGE_REF_H */
