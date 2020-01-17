/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2018 The Linux Foundation. All rights reserved.
 */

#undef TRACE_SYSTEM
#define TRACE_SYSTEM ion

#if !defined(_TRACE_ION_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_ION_H

#include <linux/types.h>
#include <linux/tracepoint.h>
#include <trace/events/mmflags.h>

#define DEV_NAME_NONE "None"

DECLARE_EVENT_CLASS(ion_dma_map_cmo_class,

	TP_PROTO(const struct device *dev, const char *name,
		 bool cached, bool hlos_accessible, unsigned long map_attrs,
		 enum dma_data_direction dir),

	TP_ARGS(dev, name, cached, hlos_accessible, map_attrs, dir),

	TP_STRUCT__entry(
		__string(dev_name, dev ? dev_name(dev) : DEV_NAME_NONE)
		__string(name, name)
		__field(bool, cached)
		__field(bool, hlos_accessible)
		__field(unsigned long, map_attrs)
		__field(enum dma_data_direction, dir)
	),

	TP_fast_assign(
		__assign_str(dev_name, dev ? dev_name(dev) : DEV_NAME_NONE);
		__assign_str(name, name);
		__entry->cached = cached;
		__entry->hlos_accessible = hlos_accessible;
		__entry->map_attrs = map_attrs;
		__entry->dir = dir;
	),

	TP_printk("dev=%s name=%s cached=%d access=%d map_attrs=0x%lx dir=%d",
		__get_str(dev_name),
		__get_str(name),
		__entry->cached,
		__entry->hlos_accessible,
		__entry->map_attrs,
		__entry->dir)
);

DEFINE_EVENT(ion_dma_map_cmo_class, ion_dma_map_cmo_apply,

	TP_PROTO(const struct device *dev, const char *name,
		 bool cached, bool hlos_accessible, unsigned long map_attrs,
		 enum dma_data_direction dir),

	TP_ARGS(dev, name, cached, hlos_accessible, map_attrs, dir)
);

DEFINE_EVENT(ion_dma_map_cmo_class, ion_dma_map_cmo_skip,

	TP_PROTO(const struct device *dev, const char *name,
		 bool cached, bool hlos_accessible, unsigned long map_attrs,
		 enum dma_data_direction dir),

	TP_ARGS(dev, name, cached, hlos_accessible, map_attrs, dir)
);

DEFINE_EVENT(ion_dma_map_cmo_class, ion_dma_unmap_cmo_apply,

	TP_PROTO(const struct device *dev, const char *name,
		 bool cached, bool hlos_accessible, unsigned long map_attrs,
		 enum dma_data_direction dir),

	TP_ARGS(dev, name, cached, hlos_accessible, map_attrs, dir)
);

DEFINE_EVENT(ion_dma_map_cmo_class, ion_dma_unmap_cmo_skip,

	TP_PROTO(const struct device *dev, const char *name,
		 bool cached, bool hlos_accessible, unsigned long map_attrs,
		 enum dma_data_direction dir),

	TP_ARGS(dev, name, cached, hlos_accessible, map_attrs, dir)
);

DECLARE_EVENT_CLASS(ion_access_cmo_class,

	TP_PROTO(const struct device *dev, const char *name,
		 bool cached, bool hlos_accessible, enum dma_data_direction dir,
		 bool only_mapped),

	TP_ARGS(dev, name, cached, hlos_accessible, dir, only_mapped),

	TP_STRUCT__entry(
		__string(dev_name, dev ? dev_name(dev) : DEV_NAME_NONE)
		__string(name, name)
		__field(bool, cached)
		__field(bool, hlos_accessible)
		__field(enum dma_data_direction, dir)
		__field(bool, only_mapped)
	),

	TP_fast_assign(
		__assign_str(dev_name, dev ? dev_name(dev) : DEV_NAME_NONE);
		__assign_str(name, name);
		__entry->cached = cached;
		__entry->hlos_accessible = hlos_accessible;
		__entry->dir = dir;
		__entry->only_mapped = only_mapped;
	),

	TP_printk("dev=%s name=%s cached=%d access=%d dir=%d, only_mapped=%d",
		  __get_str(dev_name),
		  __get_str(name),
		  __entry->cached,
		  __entry->hlos_accessible,
		  __entry->dir,
		  __entry->only_mapped)
);

DEFINE_EVENT(ion_access_cmo_class, ion_begin_cpu_access_cmo_apply,
	TP_PROTO(const struct device *dev, const char *name,
		 bool cached, bool hlos_accessible, enum dma_data_direction dir,
		 bool only_mapped),

	TP_ARGS(dev, name, cached, hlos_accessible, dir, only_mapped)
);

DEFINE_EVENT(ion_access_cmo_class, ion_begin_cpu_access_cmo_skip,
	TP_PROTO(const struct device *dev, const char *name,
		 bool cached, bool hlos_accessible, enum dma_data_direction dir,
		 bool only_mapped),

	TP_ARGS(dev, name, cached, hlos_accessible, dir, only_mapped)
);

DEFINE_EVENT(ion_access_cmo_class, ion_begin_cpu_access_notmapped,
	TP_PROTO(const struct device *dev, const char *name,
		 bool cached, bool hlos_accessible, enum dma_data_direction dir,
		 bool only_mapped),

	TP_ARGS(dev, name, cached, hlos_accessible, dir, only_mapped)
);

DEFINE_EVENT(ion_access_cmo_class, ion_end_cpu_access_cmo_apply,
	TP_PROTO(const struct device *dev, const char *name,
		 bool cached, bool hlos_accessible, enum dma_data_direction dir,
		 bool only_mapped),

	TP_ARGS(dev, name, cached, hlos_accessible, dir, only_mapped)
);

DEFINE_EVENT(ion_access_cmo_class, ion_end_cpu_access_cmo_skip,
	TP_PROTO(const struct device *dev, const char *name,
		 bool cached, bool hlos_accessible, enum dma_data_direction dir,
		 bool only_mapped),

	TP_ARGS(dev, name, cached, hlos_accessible, dir, only_mapped)
);

DEFINE_EVENT(ion_access_cmo_class, ion_end_cpu_access_notmapped,
	TP_PROTO(const struct device *dev, const char *name,
		 bool cached, bool hlos_accessible, enum dma_data_direction dir,
		 bool only_mapped),

	TP_ARGS(dev, name, cached, hlos_accessible, dir, only_mapped)
);

DECLARE_EVENT_CLASS(
	ion_alloc,

	TP_PROTO(const char *client_name, const char *heap_name, size_t len,
		 unsigned int mask, unsigned int flags),

	TP_ARGS(client_name, heap_name, len, mask, flags),

	TP_STRUCT__entry(__array(char, client_name, 64) __field(const char *,
								heap_name)
				 __field(size_t, len)
					 __field(unsigned int, mask)
						 __field(unsigned int, flags)),

	TP_fast_assign(strlcpy(__entry->client_name, client_name, 64);
		       __entry->heap_name = heap_name; __entry->len = len;
		       __entry->mask = mask; __entry->flags = flags;),

	TP_printk("client_name=%s heap_name=%s len=%zu mask=0x%x flags=0x%x",
		  __entry->client_name, __entry->heap_name, __entry->len,
		  __entry->mask, __entry->flags));

DEFINE_EVENT(ion_alloc, ion_alloc_buffer_start,

	     TP_PROTO(const char *client_name, const char *heap_name,
		      size_t len, unsigned int mask, unsigned int flags),

	     TP_ARGS(client_name, heap_name, len, mask, flags));

DEFINE_EVENT(ion_alloc, ion_alloc_buffer_end,

	     TP_PROTO(const char *client_name, const char *heap_name,
		      size_t len, unsigned int mask, unsigned int flags),

	     TP_ARGS(client_name, heap_name, len, mask, flags));

DECLARE_EVENT_CLASS(
	ion_alloc_error,

	TP_PROTO(const char *client_name, const char *heap_name, size_t len,
		 unsigned int mask, unsigned int flags, long error),

	TP_ARGS(client_name, heap_name, len, mask, flags, error),

	TP_STRUCT__entry(__field(const char *,
				 client_name) __field(const char *, heap_name)
				 __field(size_t, len)
					 __field(unsigned int, mask)
						 __field(unsigned int, flags)
							 __field(long, error)),

	TP_fast_assign(__entry->client_name = client_name;
		       __entry->heap_name = heap_name; __entry->len = len;
		       __entry->mask = mask; __entry->flags = flags;
		       __entry->error = error;),

	TP_printk(
		"client_name=%s heap_name=%s len=%zu mask=0x%x flags=0x%x error=%ld",
		__entry->client_name, __entry->heap_name, __entry->len,
		__entry->mask, __entry->flags, __entry->error));

DEFINE_EVENT(ion_alloc_error, ion_alloc_buffer_fallback,

	     TP_PROTO(const char *client_name, const char *heap_name,
		      size_t len, unsigned int mask, unsigned int flags,
		      long error),

	     TP_ARGS(client_name, heap_name, len, mask, flags, error));

DEFINE_EVENT(ion_alloc_error, ion_alloc_buffer_fail,

	     TP_PROTO(const char *client_name, const char *heap_name,
		      size_t len, unsigned int mask, unsigned int flags,
		      long error),

	     TP_ARGS(client_name, heap_name, len, mask, flags, error));

DECLARE_EVENT_CLASS(ion_alloc_pages,

		    TP_PROTO(gfp_t gfp_flags, unsigned int order),

		    TP_ARGS(gfp_flags, order),

		    TP_STRUCT__entry(__field(gfp_t, gfp_flags)
					     __field(unsigned int, order)),

		    TP_fast_assign(__entry->gfp_flags = gfp_flags;
				   __entry->order = order;),

		    TP_printk("gfp_flags=%s order=%d",
			      show_gfp_flags(__entry->gfp_flags),
			      __entry->order));

DEFINE_EVENT(ion_alloc_pages, alloc_pages_iommu_start,
	     TP_PROTO(gfp_t gfp_flags, unsigned int order),

	     TP_ARGS(gfp_flags, order));

DEFINE_EVENT(ion_alloc_pages, alloc_pages_iommu_end,
	     TP_PROTO(gfp_t gfp_flags, unsigned int order),

	     TP_ARGS(gfp_flags, order));

DEFINE_EVENT(ion_alloc_pages, alloc_pages_iommu_fail,
	     TP_PROTO(gfp_t gfp_flags, unsigned int order),

	     TP_ARGS(gfp_flags, order));

DEFINE_EVENT(ion_alloc_pages, alloc_pages_sys_start,
	     TP_PROTO(gfp_t gfp_flags, unsigned int order),

	     TP_ARGS(gfp_flags, order));

DEFINE_EVENT(ion_alloc_pages, alloc_pages_sys_end,
	     TP_PROTO(gfp_t gfp_flags, unsigned int order),

	     TP_ARGS(gfp_flags, order));

DEFINE_EVENT(ion_alloc_pages, alloc_pages_sys_fail,
	     TP_PROTO(gfp_t gfp_flags, unsigned int order),

	     TP_ARGS(gfp_flags, order)

);

DECLARE_EVENT_CLASS(
	ion_secure_cma_add_to_pool,

	TP_PROTO(unsigned long len, int pool_total, bool is_prefetch),

	TP_ARGS(len, pool_total, is_prefetch),

	TP_STRUCT__entry(__field(unsigned long, len) __field(int, pool_total)
				 __field(bool, is_prefetch)),

	TP_fast_assign(__entry->len = len; __entry->pool_total = pool_total;
		       __entry->is_prefetch = is_prefetch;),

	TP_printk("len %lx, pool total %x is_prefetch %d", __entry->len,
		  __entry->pool_total, __entry->is_prefetch));

DEFINE_EVENT(ion_secure_cma_add_to_pool, ion_secure_cma_add_to_pool_start,
	     TP_PROTO(unsigned long len, int pool_total, bool is_prefetch),

	     TP_ARGS(len, pool_total, is_prefetch));

DEFINE_EVENT(ion_secure_cma_add_to_pool, ion_secure_cma_add_to_pool_end,
	     TP_PROTO(unsigned long len, int pool_total, bool is_prefetch),

	     TP_ARGS(len, pool_total, is_prefetch));

DECLARE_EVENT_CLASS(
	ion_secure_cma_shrink_pool,

	TP_PROTO(unsigned long drained_size, unsigned long skipped_size),

	TP_ARGS(drained_size, skipped_size),

	TP_STRUCT__entry(__field(unsigned long, drained_size)
				 __field(unsigned long, skipped_size)),

	TP_fast_assign(__entry->drained_size = drained_size;
		       __entry->skipped_size = skipped_size;),

	TP_printk("drained size %lx, skipped size %lx", __entry->drained_size,
		  __entry->skipped_size));

DEFINE_EVENT(ion_secure_cma_shrink_pool, ion_secure_cma_shrink_pool_start,
	     TP_PROTO(unsigned long drained_size, unsigned long skipped_size),

	     TP_ARGS(drained_size, skipped_size));

DEFINE_EVENT(ion_secure_cma_shrink_pool, ion_secure_cma_shrink_pool_end,
	     TP_PROTO(unsigned long drained_size, unsigned long skipped_size),

	     TP_ARGS(drained_size, skipped_size));

TRACE_EVENT(ion_prefetching,

	    TP_PROTO(unsigned long len),

	    TP_ARGS(len),

	    TP_STRUCT__entry(__field(unsigned long, len)),

	    TP_fast_assign(__entry->len = len;),

	    TP_printk("prefetch size %lx", __entry->len));

DECLARE_EVENT_CLASS(
	ion_secure_cma_allocate,

	TP_PROTO(const char *heap_name, unsigned long len, unsigned long flags),

	TP_ARGS(heap_name, len, flags),

	TP_STRUCT__entry(__field(const char *, heap_name)
				 __field(unsigned long, len)
					 __field(unsigned long, flags)),

	TP_fast_assign(__entry->heap_name = heap_name; __entry->len = len;
		       __entry->flags = flags;),

	TP_printk("heap_name=%s len=%lx flags=%lx", __entry->heap_name,
		  __entry->len, __entry->flags));

DEFINE_EVENT(ion_secure_cma_allocate, ion_secure_cma_allocate_start,
	     TP_PROTO(const char *heap_name, unsigned long len,
		      unsigned long flags),

	     TP_ARGS(heap_name, len, flags));

DEFINE_EVENT(ion_secure_cma_allocate, ion_secure_cma_allocate_end,
	     TP_PROTO(const char *heap_name, unsigned long len,
		      unsigned long flags),

	     TP_ARGS(heap_name, len, flags));

DECLARE_EVENT_CLASS(
	ion_cp_secure_buffer,

	TP_PROTO(const char *heap_name, unsigned long len, unsigned long flags),

	TP_ARGS(heap_name, len, flags),

	TP_STRUCT__entry(__field(const char *, heap_name)
				 __field(unsigned long, len)
					 __field(unsigned long, flags)),

	TP_fast_assign(__entry->heap_name = heap_name; __entry->len = len;
		       __entry->flags = flags;),

	TP_printk("heap_name=%s len=%lx flags=%lx", __entry->heap_name,
		  __entry->len, __entry->flags));

DEFINE_EVENT(ion_cp_secure_buffer, ion_cp_secure_buffer_start,
	     TP_PROTO(const char *heap_name, unsigned long len,
		      unsigned long flags),

	     TP_ARGS(heap_name, len, flags));

DEFINE_EVENT(ion_cp_secure_buffer, ion_cp_secure_buffer_end,
	     TP_PROTO(const char *heap_name, unsigned long len,
		      unsigned long flags),

	     TP_ARGS(heap_name, len, flags));
#endif /* _TRACE_ION_H */

#include <trace/define_trace.h>
