#undef TRACE_SYSTEM
#define TRACE_SYSTEM almk

#if !defined(_TRACE_EVENT_ALMK_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_EVENT_ALMK_H

#include <linux/tracepoint.h>
#include <linux/types.h>
#include <linux/sched.h>

TRACE_EVENT(almk_start,

	TP_PROTO(unsigned long nr_to_scan,
		gfp_t gfp_mask,
		char	*comm,
		int min_score_adj,
		int minfree,
		int other_free,
		int other_file,
		int cma_free,
		int cma_file,
		int active_anon,
		int inactive_anon,
		int active_file,
		int inactive_file,
		int rem),

	TP_ARGS(nr_to_scan, gfp_mask, comm,
		min_score_adj, minfree,
		other_free, other_file,
		cma_free, cma_file,
		active_anon, inactive_anon,
		active_file, inactive_file,
		rem),

	TP_STRUCT__entry(
		__field(unsigned long, nr_to_scan)
		__field(gfp_t, gfp_mask)
		__array(char, comm, TASK_COMM_LEN)
		__field(int, min_score_adj)
		__field(int, minfree)
		__field(int, other_free)
		__field(int, other_file)
		__field(int, cma_free)
		__field(int, cma_file)
		__field(int, active_anon)
		__field(int, inactive_anon)
		__field(int, active_file)
		__field(int, inactive_file)
		__field(int, rem)
	),

	TP_fast_assign(
		strncpy(__entry->comm, comm, TASK_COMM_LEN);
		__entry->nr_to_scan     = nr_to_scan;
		__entry->gfp_mask	= gfp_mask;
		__entry->min_score_adj	= min_score_adj;
		__entry->minfree	= minfree;
		__entry->other_free	= other_free;
		__entry->other_file	= other_file;
		__entry->cma_free	= cma_free;
		__entry->cma_file	= cma_file;
		__entry->active_anon	= active_anon;
		__entry->inactive_anon	= inactive_anon;
		__entry->active_file	= active_file;
		__entry->inactive_file	= inactive_file;
		__entry->rem		= rem;
	),

	TP_printk("%s:%lu:0x%x:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d",
			__entry->comm, __entry->nr_to_scan,
			__entry->gfp_mask, __entry->min_score_adj,
			__entry->minfree, __entry->other_free,
			__entry->other_file, __entry->cma_free,
			__entry->cma_file, __entry->active_anon,
			__entry->inactive_anon, __entry->active_file,
			__entry->inactive_file, __entry->rem)
);

TRACE_EVENT(almk_end,

	TP_PROTO(int selected_oom_score_adj,
		int selected_tasksize,
		char *comm,
		int minfree,
		int other_free,
		int other_file,
		int cma_free,
		int cma_file,
		gfp_t gfp_mask),

	TP_ARGS(selected_oom_score_adj,
		selected_tasksize, comm,
		minfree, other_free,
		other_file, cma_free,
		cma_file, gfp_mask),

	TP_STRUCT__entry(
		__field(int, selected_oom_score_adj)
		__field(int, selected_tasksize)
		__array(char, comm, TASK_COMM_LEN)
		__field(int, minfree)
		__field(int, other_free)
		__field(int, other_file)
		__field(int, cma_free)
		__field(int, cma_file)
		__field(int, gfp_mask)
	),

	TP_fast_assign(
		strncpy(__entry->comm, comm, TASK_COMM_LEN);
		__entry->selected_oom_score_adj	= selected_oom_score_adj;
		__entry->gfp_mask		= gfp_mask;
		__entry->selected_tasksize	= selected_tasksize;
		__entry->minfree		= minfree;
		__entry->other_free		= other_free;
		__entry->other_file		= other_file;
		__entry->cma_free		= cma_free;
		__entry->cma_file		= cma_file;
	),

	TP_printk("%s:%d:0x%x:%d:%d:%d:%d:%d:%d",
			__entry->comm, __entry->selected_oom_score_adj,
			__entry->gfp_mask, __entry->selected_tasksize,
			__entry->minfree, __entry->other_free,
			__entry->other_file, __entry->cma_free,
			__entry->cma_file)
);

#endif

#include <trace/define_trace.h>
