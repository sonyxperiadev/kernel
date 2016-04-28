/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*******************************************************************************/
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/dma-mapping.h>
#include <linux/notifier.h>

#include <trace/events/irq.h>
#include <trace/events/sched.h>
#include <trace/events/workqueue.h>

#define MIN_ENTRIES 2

enum trace_entries {
	IRQ_TRACE,
	SOFTIRQ_TRACE,
	SCHED_TRACE,
	WORKQUEUE_TRACE,
	MAX_TRACES
};

struct trace_log_header {
	/* Number of percpu entries
	 * Here one entry means one
	 * set of traces (trace_log_entry).
	 */
	int entries;
	int index[MAX_TRACES][NR_CPUS];
};

/* Log entry definitions */
struct trace_log_entry {

	struct irq_trace_log {
		unsigned long timestamp;
		int irq;
	} itl;

	struct softirq_trace_log {
		unsigned long timestamp;
		unsigned int vec_nr;
	} stl;

	struct sched_switch_trace_log {
		unsigned long timestamp;
		pid_t pid;
	} sstl;

	struct workqueue_trace_log {
		unsigned long timestamp;
		int entry;
		work_func_t func;
	} wtl;
};

#define SIZEOF_ENTRIES	sizeof(struct trace_log_entry)

struct trace_log {
	void *start_vaddr;
	dma_addr_t start_paddr;
	size_t sz;

	/* This is to replace the
	 * (memory read + calculation)
	 * of percpu start in the trace
	 * probes, with a memory read
	 * alone.
	 */
	void *start[NR_CPUS];

	int entries;
	/* The index in trace_log_header is
	 * costly to read. So have a
	 * cached copy.
	 */
	int __percpu *index[MAX_TRACES];
} trace_log_data;

/*
 * We dont really gain by having
 * a lot of entries of each type.
 * So we define a min of 2.
 * Then we start calculation with a page,
 * reserve space for the header,
 * divide rest of memory by number
 * of cpus, give an equal weightage
 * for each type and calculate the
 * max entries possible. If it comes
 * to be less than 2, allocate 2 pages.
 * And continue this.
 */
static int __init entries_ok(size_t size)
{
	int entries_possible;
	size_t header_size = sizeof(struct trace_log_header);
	size_t available_size = size - header_size;

	entries_possible =
		((available_size / num_possible_cpus()) / SIZEOF_ENTRIES);

	if (entries_possible >= MIN_ENTRIES)
		return entries_possible;
	else
		return 0;
}

static int __init init_trace_buf(int entries)
{
	int cpu, i, *index;
	struct trace_log_header *header =
		(struct trace_log_header *)trace_log_data.start_vaddr;
	memset(trace_log_data.start_vaddr, 0 , trace_log_data.sz);
	header->entries = entries;
	trace_log_data.entries = entries;

	for (i = 0; i < MAX_TRACES; i++) {
		trace_log_data.index[i] = alloc_percpu(int);

		if (!trace_log_data.index[i]) {
			while (--i >= 0)
				free_percpu(trace_log_data.index[i]);
			return -ENOMEM;
		}

		for_each_possible_cpu(cpu) {
			index =
				per_cpu_ptr(trace_log_data.index[i], cpu);
			*index = 0;
		}
	}

	for_each_possible_cpu(cpu) {
	trace_log_data.start[cpu] = trace_log_data.start_vaddr +
			sizeof(struct trace_log_header) +
			(cpu * entries * SIZEOF_ENTRIES);
	}

	return 0;
}

static inline void *get_wr_ptr(enum trace_entries entry)
{
	int *index;
	struct trace_log_entry *ptr;
	struct trace_log_header *header =
		(struct trace_log_header *)trace_log_data.start_vaddr;
	int cpu = smp_processor_id();

	index =
		per_cpu_ptr(trace_log_data.index[entry], cpu);

	ptr = trace_log_data.start[cpu] +
		(*index * SIZEOF_ENTRIES);

	if (++(*index) == trace_log_data.entries)
		*index = 0;

	header->index[entry][cpu] = *index;

	return ptr;
}

static void trace_irq_handler_common(int irq, int entry)
{
	struct trace_log_entry *ptr;

	ptr = (struct trace_log_entry *)
		get_wr_ptr(IRQ_TRACE);

	ptr->itl.irq = irq | entry;
	/* We dont really need accurate time */
	ptr->itl.timestamp = jiffies;
}

static void trace_softirq_handler_common(unsigned int vec_nr, int entry)
{
	struct trace_log_entry *ptr;

	ptr = (struct trace_log_entry *)
		get_wr_ptr(SOFTIRQ_TRACE);

	ptr->stl.vec_nr = vec_nr | entry;
	ptr->stl.timestamp = jiffies;
}

static void trace_workqueue_common(work_func_t func, int entry)
{
	struct trace_log_entry *ptr;

	ptr = (struct trace_log_entry *)
		get_wr_ptr(WORKQUEUE_TRACE);

	ptr->wtl.func = func;
	ptr->wtl.entry = entry;
	ptr->wtl.timestamp = jiffies;
}

static void notrace probe_irq_handler_entry(void *ignore,
	int irq, struct irqaction *action)
{
	/* Set bit 30 to identify irq entry*/
	trace_irq_handler_common(irq, 0x40000000);
}

static void notrace
probe_irq_handler_exit(void *ignore, int irq,
	struct irqaction *action, int ret)
{
	trace_irq_handler_common(irq, 0);
}

static void notrace
probe_softirq_entry(void *ignore, unsigned int vec_nr)
{
	/* Set bit 31 to identify softirq entry*/
	trace_softirq_handler_common(vec_nr, 0x80000000);
}

static void notrace
probe_softirq_exit(void *ignore, unsigned int vec_nr)
{
	trace_softirq_handler_common(vec_nr, 0);
}

static void notrace
probe_sched_switch(void *ignore, struct task_struct *prev,
	struct task_struct *next)
{
	struct trace_log_entry *ptr;

	ptr = (struct trace_log_entry *)
		get_wr_ptr(SCHED_TRACE);

	ptr->sstl.pid = next->pid;

	ptr->sstl.timestamp = jiffies;
}

static void notrace
probe_workqueue_execute_start(void *ignore, struct work_struct *work)
{
	trace_workqueue_common(work->func, 1);
}

static void notrace
probe_workqueue_execute_end(void *ignore, struct work_struct *work)
{
	trace_workqueue_common(work->func, 0);
}

static int __init register_tracepoints(void)
{
	int ret;
	ret = register_trace_irq_handler_entry(probe_irq_handler_entry, NULL);
	if (WARN_ON(ret))
		return -1;

	ret = register_trace_irq_handler_exit(probe_irq_handler_exit, NULL);
	if (WARN_ON(ret))
		return -1;

	ret = register_trace_softirq_entry(probe_softirq_entry, NULL);
	if (WARN_ON(ret))
		return -1;

	ret = register_trace_softirq_exit(probe_softirq_exit, NULL);
	if (WARN_ON(ret))
		return -1;

	ret = register_trace_sched_switch(probe_sched_switch, NULL);
	if (WARN_ON(ret))
		return -1;

	ret = register_trace_workqueue_execute_start
		(probe_workqueue_execute_start, NULL);
	if (WARN_ON(ret))
		return -1;

	ret = register_trace_workqueue_execute_end
		(probe_workqueue_execute_end, NULL);
	if (WARN_ON(ret))
		return -1;

	return 0;
}

static void unregister_tracepoints(void)
{
	/* It is safe to call unregister,
	 * even if the register had failed
	 */
	unregister_trace_irq_handler_entry(probe_irq_handler_entry, NULL);
	unregister_trace_irq_handler_exit(probe_irq_handler_exit, NULL);
	unregister_trace_softirq_entry(probe_softirq_entry, NULL);
	unregister_trace_softirq_exit(probe_softirq_exit, NULL);
	unregister_trace_sched_switch(probe_sched_switch, NULL);
	unregister_trace_workqueue_execute_start
		(probe_workqueue_execute_start, NULL);
	unregister_trace_workqueue_execute_end
		(probe_workqueue_execute_end, NULL);

	tracepoint_synchronize_unregister();
}

static void trace_freeup(int level)
{
	int i;

	if (level & 0x1)
		dma_free_coherent(NULL, trace_log_data.sz,
			trace_log_data.start_vaddr,
			trace_log_data.start_paddr);

	if (level & 0x2)
		for (i = 0; i < MAX_TRACES; i++)
			free_percpu(trace_log_data.index[i]);

	if (level & 0x4)
		unregister_tracepoints();
}

#ifdef TL_TEST
static void dump_traces(void)
{
	int *index;
	struct trace_log_entry *ptr;
	int i, cpu, max_entries, idx;
	struct trace_log_header *header;

	header =
		(struct trace_log_header *)trace_log_data.start_vaddr;

	pr_info("\n-------TRACE LOG---->8---------\n");

	max_entries = header->entries;
	pr_info("max entries:%d\n", max_entries);

	if (trace_log_data.entries != max_entries)
		pr_err("max entries mismatch - %d, %d\n", max_entries,
			trace_log_data.entries);

	for_each_possible_cpu(cpu) {

		idx = header->index[IRQ_TRACE][cpu];
		pr_info("\nIRQ_TRACE, cpu:%d, current index:%d", cpu, idx - 1);
		index =
			per_cpu_ptr(trace_log_data.index[IRQ_TRACE], cpu);
		if (idx != *index)
			pr_err("IRQ_TRACE indices mismatch. %d, %d\n",
				idx, *index);

		for (i = 0; i < max_entries; i++) {
			ptr = trace_log_data.start[cpu] +
				(i * SIZEOF_ENTRIES);

			pr_info("%d:%s: irq:%d, timestamp:%lu\n", i,
				(ptr->itl.irq & 0x40000000)
					? "ENTRY" : "EXIT",
				ptr->itl.irq & ~0x40000000,
				ptr->itl.timestamp);
		}

		idx = header->index[SOFTIRQ_TRACE][cpu];
		pr_info("\nSOFTIRQ_TRACE, cpu:%d, current index:%d", cpu,
			idx - 1);

		index =
			per_cpu_ptr(trace_log_data.index[SOFTIRQ_TRACE], cpu);
		if (idx != *index)
			pr_err("SOFTIRQ_TRACE indices mismatch. %d, %d\n",
				idx, *index);

		for (i = 0; i < max_entries; i++) {
			ptr = trace_log_data.start[cpu] +
				(i * SIZEOF_ENTRIES);

			pr_info("%d:%s: vec_nr:%d, timestamp:%lu\n", i,
				(ptr->stl.vec_nr &
					0x80000000) ? "ENTRY" : "EXIT",
				ptr->stl.vec_nr &
					~0x80000000,
				ptr->stl.timestamp);
		}

		idx = header->index[SCHED_TRACE][cpu];
		pr_info("\nSCHED_TRACE, cpu:%d, current index:%d", cpu,
			idx - 1);

		index =
			per_cpu_ptr(trace_log_data.index[SCHED_TRACE], cpu);
		if (idx != *index)
			pr_err("SCHED_TRACE indices mismatch. %d, %d\n",
				idx, *index);

		for (i = 0; i < max_entries; i++) {
			ptr = trace_log_data.start[cpu] +
				(i * SIZEOF_ENTRIES);

			pr_info("%d:pid:%d, timestamp:%lu\n", i,
				ptr->sstl.pid,
				ptr->sstl.timestamp);
		}

		idx = header->index[WORKQUEUE_TRACE][cpu];
		pr_info("\nWORKQUEUE_TRACE, cpu:%d, current index:%d", cpu,
			idx - 1);

		index =
			per_cpu_ptr(trace_log_data.index[WORKQUEUE_TRACE], cpu);
		if (idx != *index)
			pr_err("WORKQUEUE_TRACE indices mismatch. %d, %d\n",
				idx, *index);

		for (i = 0; i < max_entries; i++) {
			ptr = trace_log_data.start[cpu] +
				(i * SIZEOF_ENTRIES);

			pr_info("%d:%s: %p, timestamp:%lu\n", i,
				ptr->wtl.entry ? "ENTRY" : "EXIT",
				ptr->wtl.func,
				ptr->wtl.timestamp);
		}
	}
	pr_info("------END OF TRACE LOG----->8-----------\n");
}

static int tl_panic_handler(struct notifier_block *nb,
				   unsigned long l, void *buf)
{
	unregister_tracepoints();

	dump_traces();

	return NOTIFY_DONE;
}

static struct notifier_block tl_panic_nb = {
	.notifier_call = tl_panic_handler,
	.priority = 1
};
#endif

static int __init trace_log_init(void)
{
	int o, sz, entries;

	for (sz = SZ_4K, o = 0; !(entries = entries_ok(sz)); sz <<= ++o)
		;

	trace_log_data.start_vaddr =
		dma_alloc_coherent(NULL, sz,
			&trace_log_data.start_paddr, GFP_KERNEL);
	if (!trace_log_data.start_vaddr) {
		pr_err("trace log allocation failed\n");
		return 0;
	}

	trace_log_data.sz = sz;
	pr_info("trace log: va:%p, pa:0x%x, sz:%d, en:%d\n",
			trace_log_data.start_vaddr,
			(unsigned int)trace_log_data.start_paddr,
			sz, entries);

	if (init_trace_buf(entries)) {
		pr_err("trace log failed in init_trace_buf\n");
		trace_freeup(0x1);
		goto error;
	}

	if (register_tracepoints()) {
		pr_err("trace log failed in register_tracepoints\n");
		trace_freeup(0x7);
		goto error;
	}

#ifdef TL_TEST
	atomic_notifier_chain_register(&panic_notifier_list, &tl_panic_nb);
#endif

	return 0;
error:
	return -1;
}

static void __exit trace_log_exit(void)
{
	trace_freeup(0x7);
}

module_init(trace_log_init);
module_exit(trace_log_exit);
