/*
 * dma-contiguous-trace.h
 *
 * Copyright (C) ST-Ericsson SA 2011
 * Author: <benjamin.gaignard@linaro.org> for ST-Ericsson.
 * License terms:  GNU General Public License (GPL), version 2
 */

#if !defined(__TRACE_CMA) || defined(TRACE_HEADER_MULTI_READ)
#define __TRACE_CMA

#include <linux/tracepoint.h>
#include <linux/dma-contiguous.h>

#if !defined(CONFIG_CMA_TRACER) || defined(__CHECKER__)
#undef TRACE_EVENT
#define TRACE_EVENT(name, proto, ...) \
static inline void trace_ ## name(proto) {}
#endif

#define PRIV_ENTRY	__field(struct cma *, priv)
#define PRIV_ASSIGN	__entry->priv = priv

#undef TRACE_SYSTEM
#define TRACE_SYSTEM cma

TRACE_EVENT(cma_alloc_start,
	    TP_PROTO(struct cma *priv, int count, unsigned int align),
	    TP_ARGS(priv, count, align),
	    TP_STRUCT__entry(PRIV_ENTRY __field(int, count)
			     __field(unsigned int, align)
	    ),
	    TP_fast_assign(PRIV_ASSIGN;
			   __entry->count = count;
			   __entry->align = align;),
	    TP_printk("[%p] CMA alloc start count=0x%x align=0x%x",
		      __entry->priv, __entry->count, __entry->align)
    );

TRACE_EVENT(cma_alloc_end_success,
	    TP_PROTO(struct cma *priv, unsigned long pfn, int count),
	    TP_ARGS(priv, pfn, count),
	    TP_STRUCT__entry(PRIV_ENTRY __field(unsigned long, pfn)
			     __field(int, count)
	    ),
	    TP_fast_assign(PRIV_ASSIGN;
			   __entry->pfn = pfn;
			   __entry->count = count;),
	    TP_printk("[%p] CMA alloc success pfn=0x%lx count=0x%x",
		      __entry->priv, __entry->pfn, __entry->count)
    );

TRACE_EVENT(cma_alloc_end_failed,
	    TP_PROTO(struct cma *priv, int count),
	    TP_ARGS(priv, count),
	    TP_STRUCT__entry(PRIV_ENTRY __field(int, count)
	    ),
	    TP_fast_assign(PRIV_ASSIGN;
			   __entry->count = count;),
	    TP_printk("[%p] CMA alloc failed count=0x%x", __entry->priv,
		      __entry->count)
    );

TRACE_EVENT(cma_release_start,
	    TP_PROTO(struct cma *priv, unsigned long pfn, int count),
	    TP_ARGS(priv, pfn, count),
	    TP_STRUCT__entry(PRIV_ENTRY __field(unsigned long, pfn)
			     __field(int, count)
	    ),
	    TP_fast_assign(PRIV_ASSIGN;
			   __entry->pfn = pfn;
			   __entry->count = count;),
	    TP_printk("[%p] CMA start release pfn=0x%lx count=0x%x",
		      __entry->priv, __entry->pfn, __entry->count)
    );

TRACE_EVENT(cma_release_end,
	    TP_PROTO(struct cma *priv, unsigned long pfn, int count),
	    TP_ARGS(priv, pfn, count),
	    TP_STRUCT__entry(PRIV_ENTRY __field(unsigned long, pfn)
			     __field(int, count)
	    ),
	    TP_fast_assign(PRIV_ASSIGN;
			   __entry->pfn = pfn;
			   __entry->count = count;),
	    TP_printk("[%p] CMA end release pfn=0x%lx count=0x%x",
		      __entry->priv, __entry->pfn, __entry->count)
    );

#endif /* __TRACE_CMA */

#undef TRACE_INCLUDE_PATH
#define TRACE_INCLUDE_PATH ../../drivers/base
#undef TRACE_INCLUDE_FILE
#define TRACE_INCLUDE_FILE dma-contiguous-trace
#include <trace/define_trace.h>
