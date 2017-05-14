#undef TRACE_SYSTEM
#define TRACE_SYSTEM sched

#if !defined(_TRACE_SCHED_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_SCHED_H

#include <linux/sched.h>
#include <linux/tracepoint.h>
#include <linux/binfmts.h>

struct rq;
extern const char *task_event_names[];

/*
 * Tracepoint for calling kthread_stop, performed to end a kthread:
 */
TRACE_EVENT(sched_kthread_stop,

	TP_PROTO(struct task_struct *t),

	TP_ARGS(t),

	TP_STRUCT__entry(
		__array(	char,	comm,	TASK_COMM_LEN	)
		__field(	pid_t,	pid			)
	),

	TP_fast_assign(
		memcpy(__entry->comm, t->comm, TASK_COMM_LEN);
		__entry->pid	= t->pid;
	),

	TP_printk("comm=%s pid=%d", __entry->comm, __entry->pid)
);

/*
 * Tracepoint for the return value of the kthread stopping:
 */
TRACE_EVENT(sched_kthread_stop_ret,

	TP_PROTO(int ret),

	TP_ARGS(ret),

	TP_STRUCT__entry(
		__field(	int,	ret	)
	),

	TP_fast_assign(
		__entry->ret	= ret;
	),

	TP_printk("ret=%d", __entry->ret)
);

/*
 * Tracepoint for task enqueue/dequeue:
 */
TRACE_EVENT(sched_enq_deq_task,

	TP_PROTO(struct task_struct *p, int enqueue, unsigned int cpus_allowed),

	TP_ARGS(p, enqueue, cpus_allowed),

	TP_STRUCT__entry(
		__array(	char,	comm,	TASK_COMM_LEN	)
		__field(	pid_t,	pid			)
		__field(	int,	prio			)
		__field(	int,	cpu			)
		__field(	int,	enqueue			)
		__field(unsigned int,	nr_running		)
		__field(unsigned long,	cpu_load		)
		__field(unsigned int,	rt_nr_running		)
		__field(unsigned int,	cpus_allowed		)
#ifdef CONFIG_SCHED_HMP
		__field(unsigned int,	sum_scaled		)
		__field(unsigned int,	period			)
		__field(unsigned int,	demand			)
		__field(unsigned int,	grp_id			)
#endif
	),

	TP_fast_assign(
		memcpy(__entry->comm, p->comm, TASK_COMM_LEN);
		__entry->pid		= p->pid;
		__entry->prio		= p->prio;
		__entry->cpu		= task_cpu(p);
		__entry->enqueue	= enqueue;
		__entry->nr_running	= task_rq(p)->nr_running;
		__entry->cpu_load	= task_rq(p)->cpu_load[0];
		__entry->rt_nr_running	= task_rq(p)->rt.rt_nr_running;
		__entry->cpus_allowed	= cpus_allowed;
#ifdef CONFIG_SCHED_HMP
		__entry->sum_scaled	= p->se.avg.runnable_avg_sum_scaled;
		__entry->period		= p->se.avg.runnable_avg_period;
		__entry->demand		= p->ravg.demand;
		__entry->grp_id		= p->grp ? p->grp->id : 0;
#endif
	),

	TP_printk("cpu=%d %s comm=%s pid=%d prio=%d nr_running=%u cpu_load=%lu rt_nr_running=%u affine=%x"
#ifdef CONFIG_SCHED_HMP
		 " sum_scaled=%u period=%u demand=%u grp_id=%d"
#endif
			, __entry->cpu,
			__entry->enqueue ? "enqueue" : "dequeue",
			__entry->comm, __entry->pid,
			__entry->prio, __entry->nr_running,
			__entry->cpu_load, __entry->rt_nr_running,
			__entry->cpus_allowed
#ifdef CONFIG_SCHED_HMP
			, __entry->sum_scaled, __entry->period, __entry->demand,
			__entry->grp_id
#endif
			)
);

#ifdef CONFIG_SCHED_HMP

TRACE_EVENT(sched_task_load,

	TP_PROTO(struct task_struct *p, int small_task, int boost, int reason,
		 int sync, int prefer_idle),

	TP_ARGS(p, small_task, boost, reason, sync, prefer_idle),

	TP_STRUCT__entry(
		__array(	char,	comm,	TASK_COMM_LEN	)
		__field(	pid_t,	pid			)
		__field(unsigned int,	sum			)
		__field(unsigned int,	sum_scaled		)
		__field(unsigned int,	period			)
		__field(unsigned int,	demand			)
		__field(	int,	small_task		)
		__field(	int,	boost			)
		__field(	int,	reason			)
		__field(	int,	sync			)
		__field(	int,	prefer_idle		)
	),

	TP_fast_assign(
		memcpy(__entry->comm, p->comm, TASK_COMM_LEN);
		__entry->pid		= p->pid;
		__entry->sum		= p->se.avg.runnable_avg_sum;
		__entry->sum_scaled	= p->se.avg.runnable_avg_sum_scaled;
		__entry->period		= p->se.avg.runnable_avg_period;
		__entry->demand		= p->ravg.demand;
		__entry->small_task	= small_task;
		__entry->boost		= boost;
		__entry->reason		= reason;
		__entry->sync		= sync;
		__entry->prefer_idle	= prefer_idle;
	),

	TP_printk("%d (%s): sum=%u, sum_scaled=%u, period=%u demand=%u small=%d boost=%d reason=%d sync=%d prefer_idle=%d",
		__entry->pid, __entry->comm, __entry->sum,
		__entry->sum_scaled, __entry->period, __entry->demand,
		__entry->small_task, __entry->boost, __entry->reason,
		__entry->sync, __entry->prefer_idle)
);

TRACE_EVENT(sched_set_preferred_cluster,

	TP_PROTO(struct related_thread_group *grp, u64 total_demand),

	TP_ARGS(grp, total_demand),

	TP_STRUCT__entry(
		__field(		int,	id			)
		__field(		u64,	demand			)
		__field(		int,	cluster_first_cpu	)
	),

	TP_fast_assign(
		__entry->id			= grp->id;
		__entry->demand			= total_demand;
		__entry->cluster_first_cpu	= grp->preferred_cluster ?
							cluster_first_cpu(grp->preferred_cluster)
							: -1;
	),

	TP_printk("group_id %d total_demand %llu preferred_cluster_first_cpu %d",
			__entry->id, __entry->demand,
			__entry->cluster_first_cpu)
);

TRACE_EVENT(sched_cpu_load,

	TP_PROTO(struct rq *rq, int idle, int mostly_idle, u64 irqload,
		 unsigned int power_cost, int temp),

	TP_ARGS(rq, idle, mostly_idle, irqload, power_cost, temp),

	TP_STRUCT__entry(
		__field(unsigned int, cpu			)
		__field(unsigned int, idle			)
		__field(unsigned int, mostly_idle		)
		__field(unsigned int, nr_running		)
		__field(unsigned int, nr_big_tasks		)
		__field(unsigned int, nr_small_tasks		)
		__field(unsigned int, load_scale_factor		)
		__field(unsigned int, capacity			)
		__field(	 u64, cumulative_runnable_avg	)
		__field(	 u64, irqload			)
		__field(unsigned int, cur_freq			)
		__field(unsigned int, max_freq			)
		__field(unsigned int, power_cost		)
		__field(	 int, cstate			)
		__field(	 int, temp			)
	),

	TP_fast_assign(
		__entry->cpu			= rq->cpu;
		__entry->idle			= idle;
		__entry->mostly_idle		= mostly_idle;
		__entry->nr_running		= rq->nr_running;
		__entry->nr_big_tasks		= rq->hmp_stats.nr_big_tasks;
		__entry->nr_small_tasks		= rq->hmp_stats.nr_small_tasks;
		__entry->load_scale_factor	= rq->cluster->load_scale_factor;
		__entry->capacity		= rq->cluster->capacity;
		__entry->cumulative_runnable_avg = rq->hmp_stats.cumulative_runnable_avg;
		__entry->irqload		= irqload;
		__entry->cur_freq		= rq->cluster->cur_freq;
		__entry->max_freq		= rq->cluster->max_freq;
		__entry->power_cost		= power_cost;
		__entry->cstate			= rq->cstate;
		__entry->temp			= temp;
	),

	TP_printk("cpu %u idle %d mostly_idle %d nr_run %u nr_big %u nr_small %u lsf %u capacity %u cr_avg %llu irqload %llu fcur %u fmax %u power_cost %u cstate %d temp %d",
	__entry->cpu, __entry->idle, __entry->mostly_idle, __entry->nr_running,
	__entry->nr_big_tasks, __entry->nr_small_tasks,
	__entry->load_scale_factor, __entry->capacity,
	__entry->cumulative_runnable_avg, __entry->irqload,
	__entry->cur_freq, __entry->max_freq,
	__entry->power_cost, __entry->cstate, __entry->temp)
);

TRACE_EVENT(sched_set_boost,

	TP_PROTO(int ref_count),

	TP_ARGS(ref_count),

	TP_STRUCT__entry(
		__field(unsigned int, ref_count			)
	),

	TP_fast_assign(
		__entry->ref_count = ref_count;
	),

	TP_printk("ref_count=%d", __entry->ref_count)
);

TRACE_EVENT(sched_update_task_ravg,

	TP_PROTO(struct task_struct *p, struct rq *rq, enum task_event evt,
						u64 wallclock, u64 irqtime),

	TP_ARGS(p, rq, evt, wallclock, irqtime),

	TP_STRUCT__entry(
		__array(	char,	comm,   TASK_COMM_LEN	)
		__field(	pid_t,	pid			)
		__field(	pid_t,	cur_pid			)
		__field(unsigned int,	cur_freq		)
		__field(	u64,	wallclock		)
		__field(	u64,	mark_start		)
		__field(	u64,	delta_m			)
		__field(	u64,	win_start		)
		__field(	u64,	delta			)
		__field(	u64,	irqtime			)
		__field(enum task_event,	evt		)
		__field(unsigned int,	demand			)
		__field(unsigned int,	sum			)
		__field(	 int,	cpu			)
#ifdef CONFIG_SCHED_FREQ_INPUT
		__field(	u64,	cs			)
		__field(	u64,	ps			)
		__field(	u32,	curr_window		)
		__field(	u32,	prev_window		)
		__field(	u64,	nt_cs			)
		__field(	u64,	nt_ps			)
		__field(	u32,	active_windows		)
#endif
	),

	TP_fast_assign(
		__entry->wallclock      = wallclock;
		__entry->win_start      = rq->window_start;
		__entry->delta          = (wallclock - rq->window_start);
		__entry->evt            = evt;
		__entry->cpu            = rq->cpu;
		__entry->cur_pid        = rq->curr->pid;
		__entry->cur_freq       = cpu_cur_freq(rq->cpu);
		memcpy(__entry->comm, p->comm, TASK_COMM_LEN);
		__entry->pid            = p->pid;
		__entry->mark_start     = p->ravg.mark_start;
		__entry->delta_m        = (wallclock - p->ravg.mark_start);
		__entry->demand         = p->ravg.demand;
		__entry->sum            = p->ravg.sum;
		__entry->irqtime        = irqtime;
#ifdef CONFIG_SCHED_FREQ_INPUT
		__entry->cs             = rq->curr_runnable_sum;
		__entry->ps             = rq->prev_runnable_sum;
		__entry->curr_window	= p->ravg.curr_window;
		__entry->prev_window	= p->ravg.prev_window;
		__entry->nt_cs		= rq->nt_curr_runnable_sum;
		__entry->nt_ps		= rq->nt_prev_runnable_sum;
		__entry->active_windows	= p->ravg.active_windows;
#endif
	),

	TP_printk("wc %llu ws %llu delta %llu event %s cpu %d cur_freq %u cur_pid %d task %d (%s) ms %llu delta %llu demand %u sum %u irqtime %llu"
#ifdef CONFIG_SCHED_FREQ_INPUT
		" cs %llu ps %llu cur_window %u prev_window %u nt_cs %llu nt_ps %llu active_wins %u"
#endif
		, __entry->wallclock, __entry->win_start, __entry->delta,
		task_event_names[__entry->evt], __entry->cpu,
		__entry->cur_freq, __entry->cur_pid,
		__entry->pid, __entry->comm, __entry->mark_start,
		__entry->delta_m, __entry->demand,
		__entry->sum, __entry->irqtime
#ifdef CONFIG_SCHED_FREQ_INPUT
		, __entry->cs, __entry->ps, __entry->curr_window,
		  __entry->prev_window,
		  __entry->nt_cs, __entry->nt_ps,
		  __entry->active_windows
#endif
		)
);

TRACE_EVENT(sched_update_history,

	TP_PROTO(struct rq *rq, struct task_struct *p, u32 runtime, int samples,
			enum task_event evt),

	TP_ARGS(rq, p, runtime, samples, evt),

	TP_STRUCT__entry(
		__array(	char,	comm,   TASK_COMM_LEN	)
		__field(	pid_t,	pid			)
		__field(unsigned int,	runtime			)
		__field(	 int,	samples			)
		__field(enum task_event,	evt		)
		__field(unsigned int,	demand			)
		__array(	 u32,	hist, RAVG_HIST_SIZE_MAX)
		__field(unsigned int,	nr_big_tasks		)
		__field(unsigned int,	nr_small_tasks		)
		__field(	 int,	cpu			)
	),

	TP_fast_assign(
		memcpy(__entry->comm, p->comm, TASK_COMM_LEN);
		__entry->pid            = p->pid;
		__entry->runtime        = runtime;
		__entry->samples        = samples;
		__entry->evt            = evt;
		__entry->demand         = p->ravg.demand;
		memcpy(__entry->hist, p->ravg.sum_history,
					RAVG_HIST_SIZE_MAX * sizeof(u32));
		__entry->nr_big_tasks   = rq->hmp_stats.nr_big_tasks;
		__entry->nr_small_tasks = rq->hmp_stats.nr_small_tasks;
		__entry->cpu            = rq->cpu;
	),

	TP_printk("%d (%s): runtime %u samples %d event %s demand %u (hist: %u %u %u %u %u) cpu %d nr_big %u nr_small %u",
		__entry->pid, __entry->comm,
		__entry->runtime, __entry->samples,
		task_event_names[__entry->evt],
		__entry->demand, __entry->hist[0],
		__entry->hist[1], __entry->hist[2], __entry->hist[3],
		__entry->hist[4], __entry->cpu, __entry->nr_big_tasks,
		__entry->nr_small_tasks)
);

TRACE_EVENT(sched_reset_all_window_stats,

	TP_PROTO(u64 window_start, u64 window_size, u64 time_taken,
		int reason, unsigned int old_val, unsigned int new_val),

	TP_ARGS(window_start, window_size, time_taken,
		reason, old_val, new_val),

	TP_STRUCT__entry(
		__field(	u64,	window_start		)
		__field(	u64,	window_size		)
		__field(	u64,	time_taken		)
		__field(	int,	reason			)
		__field(unsigned int,	old_val			)
		__field(unsigned int,	new_val			)
	),

	TP_fast_assign(
		__entry->window_start = window_start;
		__entry->window_size = window_size;
		__entry->time_taken = time_taken;
		__entry->reason	= reason;
		__entry->old_val = old_val;
		__entry->new_val = new_val;
	),

	TP_printk("time_taken %llu window_start %llu window_size %llu reason %s old_val %u new_val %u",
		  __entry->time_taken, __entry->window_start,
		  __entry->window_size,
		  sched_window_reset_reasons[__entry->reason],
		  __entry->old_val, __entry->new_val)
);

#ifdef CONFIG_SCHED_FREQ_INPUT

TRACE_EVENT(sched_migration_update_sum,

	TP_PROTO(struct rq *rq, struct task_struct *p),

	TP_ARGS(rq, p),

	TP_STRUCT__entry(
		__field(int,		cpu			)
		__field(int,		pid			)
		__field(	u64,	cs			)
		__field(	u64,	ps			)
		__field(	s64,	nt_cs			)
		__field(	s64,	nt_ps			)
	),

	TP_fast_assign(
		__entry->cpu		= cpu_of(rq);
		__entry->cs		= rq->curr_runnable_sum;
		__entry->ps		= rq->prev_runnable_sum;
		__entry->nt_cs		= (s64)rq->nt_curr_runnable_sum;
		__entry->nt_ps		= (s64)rq->nt_prev_runnable_sum;
		__entry->pid		= p->pid;
	),

	TP_printk("cpu %d: cs %llu ps %llu nt_cs %lld nt_ps %lld pid %d",
		  __entry->cpu, __entry->cs, __entry->ps,
		  __entry->nt_cs, __entry->nt_ps, __entry->pid)
);

TRACE_EVENT(sched_get_busy,

	TP_PROTO(int cpu, u64 load, u64 nload),

	TP_ARGS(cpu, load, nload),

	TP_STRUCT__entry(
		__field(	int,	cpu			)
		__field(	u64,	load			)
		__field(	u64,	nload			)
	),

	TP_fast_assign(
		__entry->cpu		= cpu;
		__entry->load		= load;
		__entry->nload		= nload;
	),

	TP_printk("cpu %d load %lld new_task_load %lld",
		__entry->cpu, __entry->load, __entry->nload)
);

TRACE_EVENT(sched_freq_alert,

	TP_PROTO(int cpu, u64 old_load, u64 new_load),

	TP_ARGS(cpu, old_load, new_load),

	TP_STRUCT__entry(
		__field(	int,	cpu			)
		__field(	u64,	old_load		)
		__field(	u64,	new_load		)
	),

	TP_fast_assign(
		__entry->cpu		= cpu;
		__entry->old_load	= old_load;
		__entry->new_load	= new_load;
	),

	TP_printk("cpu %d old_load=%llu new_load=%llu",
		__entry->cpu, __entry->old_load, __entry->new_load)
);

#endif	/* CONFIG_SCHED_FREQ_INPUT */

#endif	/* CONFIG_SCHED_HMP */

/*
 * Tracepoint for waking up a task:
 */
DECLARE_EVENT_CLASS(sched_wakeup_template,

	TP_PROTO(struct task_struct *p, int success),

	TP_ARGS(p, success),

	TP_STRUCT__entry(
		__array(	char,	comm,	TASK_COMM_LEN	)
		__field(	pid_t,	pid			)
		__field(	int,	prio			)
		__field(	int,	success			)
		__field(	int,	target_cpu		)
	),

	TP_fast_assign(
		memcpy(__entry->comm, p->comm, TASK_COMM_LEN);
		__entry->pid		= p->pid;
		__entry->prio		= p->prio;
		__entry->success	= success;
		__entry->target_cpu	= task_cpu(p);
	)
	TP_perf_assign(
		__perf_task(p);
	),

	TP_printk("comm=%s pid=%d prio=%d success=%d target_cpu=%03d",
		  __entry->comm, __entry->pid, __entry->prio,
		  __entry->success, __entry->target_cpu)
);

DEFINE_EVENT(sched_wakeup_template, sched_wakeup,
	     TP_PROTO(struct task_struct *p, int success),
	     TP_ARGS(p, success));

/*
 * Tracepoint for waking up a new task:
 */
DEFINE_EVENT(sched_wakeup_template, sched_wakeup_new,
	     TP_PROTO(struct task_struct *p, int success),
	     TP_ARGS(p, success));

#ifdef CREATE_TRACE_POINTS
static inline long __trace_sched_switch_state(struct task_struct *p)
{
	long state = p->state;

#ifdef CONFIG_PREEMPT
	/*
	 * For all intents and purposes a preempted task is a running task.
	 */
	if (task_thread_info(p)->preempt_count & PREEMPT_ACTIVE)
		state = TASK_RUNNING | TASK_STATE_MAX;
#endif

	return state;
}
#endif

/*
 * Tracepoint for task switches, performed by the scheduler:
 */
TRACE_EVENT(sched_switch,

	TP_PROTO(struct task_struct *prev,
		 struct task_struct *next),

	TP_ARGS(prev, next),

	TP_STRUCT__entry(
		__array(	char,	prev_comm,	TASK_COMM_LEN	)
		__field(	pid_t,	prev_pid			)
		__field(	int,	prev_prio			)
		__field(	long,	prev_state			)
		__array(	char,	next_comm,	TASK_COMM_LEN	)
		__field(	pid_t,	next_pid			)
		__field(	int,	next_prio			)
	),

	TP_fast_assign(
		memcpy(__entry->next_comm, next->comm, TASK_COMM_LEN);
		__entry->prev_pid	= prev->pid;
		__entry->prev_prio	= prev->prio;
		__entry->prev_state	= __trace_sched_switch_state(prev);
		memcpy(__entry->prev_comm, prev->comm, TASK_COMM_LEN);
		__entry->next_pid	= next->pid;
		__entry->next_prio	= next->prio;
	),

	TP_printk("prev_comm=%s prev_pid=%d prev_prio=%d prev_state=%s%s ==> next_comm=%s next_pid=%d next_prio=%d",
		__entry->prev_comm, __entry->prev_pid, __entry->prev_prio,
		__entry->prev_state & (TASK_STATE_MAX-1) ?
		  __print_flags(__entry->prev_state & (TASK_STATE_MAX-1), "|",
				{ 1, "S"} , { 2, "D" }, { 4, "T" }, { 8, "t" },
				{ 16, "Z" }, { 32, "X" }, { 64, "x" },
				{ 128, "K" }, { 256, "W" }, { 512, "P" }) : "R",
		__entry->prev_state & TASK_STATE_MAX ? "+" : "",
		__entry->next_comm, __entry->next_pid, __entry->next_prio)
);

/*
 * Tracepoint for a task being migrated:
 */
TRACE_EVENT(sched_migrate_task,

	TP_PROTO(struct task_struct *p, int dest_cpu,
		 unsigned int load),

	TP_ARGS(p, dest_cpu, load),

	TP_STRUCT__entry(
		__array(	char,	comm,	TASK_COMM_LEN	)
		__field(	pid_t,	pid			)
		__field(	int,	prio			)
		__field(unsigned int,	load			)
		__field(	int,	orig_cpu		)
		__field(	int,	dest_cpu		)
	),

	TP_fast_assign(
		memcpy(__entry->comm, p->comm, TASK_COMM_LEN);
		__entry->pid		= p->pid;
		__entry->prio		= p->prio;
		__entry->load		= load;
		__entry->orig_cpu	= task_cpu(p);
		__entry->dest_cpu	= dest_cpu;
	),

	TP_printk("comm=%s pid=%d prio=%d load=%d orig_cpu=%d dest_cpu=%d",
		  __entry->comm, __entry->pid, __entry->prio,  __entry->load,
		  __entry->orig_cpu, __entry->dest_cpu)
);

/*
 * Tracepoint for a CPU going offline/online:
 */
TRACE_EVENT(sched_cpu_hotplug,

	TP_PROTO(int affected_cpu, int error, int status),

	TP_ARGS(affected_cpu, error, status),

	TP_STRUCT__entry(
		__field(	int,	affected_cpu		)
		__field(	int,	error			)
		__field(	int,	status			)
	),

	TP_fast_assign(
		__entry->affected_cpu	= affected_cpu;
		__entry->error		= error;
		__entry->status		= status;
	),

	TP_printk("cpu %d %s error=%d", __entry->affected_cpu,
		__entry->status ? "online" : "offline", __entry->error)
);

/*
 * Tracepoint for load balancing:
 */
#if NR_CPUS > 32
#error "Unsupported NR_CPUS for lb tracepoint."
#endif
TRACE_EVENT(sched_load_balance,

	TP_PROTO(int cpu, enum cpu_idle_type idle, int balance,
		 unsigned long group_mask, int busiest_nr_running,
		 unsigned long imbalance, unsigned int env_flags, int ld_moved,
		 unsigned int balance_interval),

	TP_ARGS(cpu, idle, balance, group_mask, busiest_nr_running,
		imbalance, env_flags, ld_moved, balance_interval),

	TP_STRUCT__entry(
		__field(	int,			cpu)
		__field(	enum cpu_idle_type,	idle)
		__field(	int,			balance)
		__field(	unsigned long,		group_mask)
		__field(	int,			busiest_nr_running)
		__field(	unsigned long,		imbalance)
		__field(	unsigned int,		env_flags)
		__field(	int,			ld_moved)
		__field(	unsigned int,		balance_interval)
	),

	TP_fast_assign(
		__entry->cpu			= cpu;
		__entry->idle			= idle;
		__entry->balance		= balance;
		__entry->group_mask		= group_mask;
		__entry->busiest_nr_running	= busiest_nr_running;
		__entry->imbalance		= imbalance;
		__entry->env_flags		= env_flags;
		__entry->ld_moved		= ld_moved;
		__entry->balance_interval	= balance_interval;
	),

	TP_printk("cpu=%d state=%s balance=%d group=%#lx busy_nr=%d imbalance=%ld flags=%#x ld_moved=%d bal_int=%d",
		  __entry->cpu,
		  __entry->idle == CPU_IDLE ? "idle" :
		  (__entry->idle == CPU_NEWLY_IDLE ? "newly_idle" : "busy"),
		  __entry->balance,
		  __entry->group_mask, __entry->busiest_nr_running,
		  __entry->imbalance, __entry->env_flags, __entry->ld_moved,
		  __entry->balance_interval)
);

DECLARE_EVENT_CLASS(sched_process_template,

	TP_PROTO(struct task_struct *p),

	TP_ARGS(p),

	TP_STRUCT__entry(
		__array(	char,	comm,	TASK_COMM_LEN	)
		__field(	pid_t,	pid			)
		__field(	int,	prio			)
	),

	TP_fast_assign(
		memcpy(__entry->comm, p->comm, TASK_COMM_LEN);
		__entry->pid		= p->pid;
		__entry->prio		= p->prio;
	),

	TP_printk("comm=%s pid=%d prio=%d",
		  __entry->comm, __entry->pid, __entry->prio)
);

/*
 * Tracepoint for freeing a task:
 */
DEFINE_EVENT(sched_process_template, sched_process_free,
	     TP_PROTO(struct task_struct *p),
	     TP_ARGS(p));


/*
 * Tracepoint for a task exiting:
 */
DEFINE_EVENT(sched_process_template, sched_process_exit,
	     TP_PROTO(struct task_struct *p),
	     TP_ARGS(p));

/*
 * Tracepoint for waiting on task to unschedule:
 */
DEFINE_EVENT(sched_process_template, sched_wait_task,
	TP_PROTO(struct task_struct *p),
	TP_ARGS(p));

/*
 * Tracepoint for a waiting task:
 */
TRACE_EVENT(sched_process_wait,

	TP_PROTO(struct pid *pid),

	TP_ARGS(pid),

	TP_STRUCT__entry(
		__array(	char,	comm,	TASK_COMM_LEN	)
		__field(	pid_t,	pid			)
		__field(	int,	prio			)
	),

	TP_fast_assign(
		memcpy(__entry->comm, current->comm, TASK_COMM_LEN);
		__entry->pid		= pid_nr(pid);
		__entry->prio		= current->prio;
	),

	TP_printk("comm=%s pid=%d prio=%d",
		  __entry->comm, __entry->pid, __entry->prio)
);

/*
 * Tracepoint for do_fork:
 */
TRACE_EVENT(sched_process_fork,

	TP_PROTO(struct task_struct *parent, struct task_struct *child),

	TP_ARGS(parent, child),

	TP_STRUCT__entry(
		__array(	char,	parent_comm,	TASK_COMM_LEN	)
		__field(	pid_t,	parent_pid			)
		__array(	char,	child_comm,	TASK_COMM_LEN	)
		__field(	pid_t,	child_pid			)
	),

	TP_fast_assign(
		memcpy(__entry->parent_comm, parent->comm, TASK_COMM_LEN);
		__entry->parent_pid	= parent->pid;
		memcpy(__entry->child_comm, child->comm, TASK_COMM_LEN);
		__entry->child_pid	= child->pid;
	),

	TP_printk("comm=%s pid=%d child_comm=%s child_pid=%d",
		__entry->parent_comm, __entry->parent_pid,
		__entry->child_comm, __entry->child_pid)
);

/*
 * Tracepoint for exec:
 */
TRACE_EVENT(sched_process_exec,

	TP_PROTO(struct task_struct *p, pid_t old_pid,
		 struct linux_binprm *bprm),

	TP_ARGS(p, old_pid, bprm),

	TP_STRUCT__entry(
		__string(	filename,	bprm->filename	)
		__field(	pid_t,		pid		)
		__field(	pid_t,		old_pid		)
	),

	TP_fast_assign(
		__assign_str(filename, bprm->filename);
		__entry->pid		= p->pid;
		__entry->old_pid	= old_pid;
	),

	TP_printk("filename=%s pid=%d old_pid=%d", __get_str(filename),
		  __entry->pid, __entry->old_pid)
);

/*
 * XXX the below sched_stat tracepoints only apply to SCHED_OTHER/BATCH/IDLE
 *     adding sched_stat support to SCHED_FIFO/RR would be welcome.
 */
DECLARE_EVENT_CLASS(sched_stat_template,

	TP_PROTO(struct task_struct *tsk, u64 delay),

	TP_ARGS(tsk, delay),

	TP_STRUCT__entry(
		__array( char,	comm,	TASK_COMM_LEN	)
		__field( pid_t,	pid			)
		__field( u64,	delay			)
	),

	TP_fast_assign(
		memcpy(__entry->comm, tsk->comm, TASK_COMM_LEN);
		__entry->pid	= tsk->pid;
		__entry->delay	= delay;
	)
	TP_perf_assign(
		__perf_count(delay);
		__perf_task(tsk);
	),

	TP_printk("comm=%s pid=%d delay=%Lu [ns]",
			__entry->comm, __entry->pid,
			(unsigned long long)__entry->delay)
);


/*
 * Tracepoint for accounting wait time (time the task is runnable
 * but not actually running due to scheduler contention).
 */
DEFINE_EVENT(sched_stat_template, sched_stat_wait,
	     TP_PROTO(struct task_struct *tsk, u64 delay),
	     TP_ARGS(tsk, delay));

/*
 * Tracepoint for accounting sleep time (time the task is not runnable,
 * including iowait, see below).
 */
DEFINE_EVENT(sched_stat_template, sched_stat_sleep,
	     TP_PROTO(struct task_struct *tsk, u64 delay),
	     TP_ARGS(tsk, delay));

/*
 * Tracepoint for accounting iowait time (time the task is not runnable
 * due to waiting on IO to complete).
 */
DEFINE_EVENT(sched_stat_template, sched_stat_iowait,
	     TP_PROTO(struct task_struct *tsk, u64 delay),
	     TP_ARGS(tsk, delay));

/*
 * Tracepoint for accounting blocked time (time the task is in uninterruptible).
 */
DEFINE_EVENT(sched_stat_template, sched_stat_blocked,
	     TP_PROTO(struct task_struct *tsk, u64 delay),
	     TP_ARGS(tsk, delay));

/*
 * Tracepoint for recording the cause of uninterruptible sleep.
 */
TRACE_EVENT(sched_blocked_reason,

	TP_PROTO(struct task_struct *tsk),

	TP_ARGS(tsk),

	TP_STRUCT__entry(
		__field( pid_t,	pid	)
		__field( void*, caller	)
		__field( bool, io_wait	)
	),

	TP_fast_assign(
		__entry->pid	= tsk->pid;
		__entry->caller = (void*)get_wchan(tsk);
		__entry->io_wait = tsk->in_iowait;
	),

	TP_printk("pid=%d iowait=%d caller=%pS", __entry->pid, __entry->io_wait, __entry->caller)
);

/*
 * Tracepoint for accounting runtime (time the task is executing
 * on a CPU).
 */
TRACE_EVENT(sched_stat_runtime,

	TP_PROTO(struct task_struct *tsk, u64 runtime, u64 vruntime),

	TP_ARGS(tsk, runtime, vruntime),

	TP_STRUCT__entry(
		__array( char,	comm,	TASK_COMM_LEN	)
		__field( pid_t,	pid			)
		__field( u64,	runtime			)
		__field( u64,	vruntime			)
	),

	TP_fast_assign(
		memcpy(__entry->comm, tsk->comm, TASK_COMM_LEN);
		__entry->pid		= tsk->pid;
		__entry->runtime	= runtime;
		__entry->vruntime	= vruntime;
	)
	TP_perf_assign(
		__perf_count(runtime);
	),

	TP_printk("comm=%s pid=%d runtime=%Lu [ns] vruntime=%Lu [ns]",
			__entry->comm, __entry->pid,
			(unsigned long long)__entry->runtime,
			(unsigned long long)__entry->vruntime)
);

/*
 * Tracepoint for showing priority inheritance modifying a tasks
 * priority.
 */
TRACE_EVENT(sched_pi_setprio,

	TP_PROTO(struct task_struct *tsk, int newprio),

	TP_ARGS(tsk, newprio),

	TP_STRUCT__entry(
		__array( char,	comm,	TASK_COMM_LEN	)
		__field( pid_t,	pid			)
		__field( int,	oldprio			)
		__field( int,	newprio			)
	),

	TP_fast_assign(
		memcpy(__entry->comm, tsk->comm, TASK_COMM_LEN);
		__entry->pid		= tsk->pid;
		__entry->oldprio	= tsk->prio;
		__entry->newprio	= newprio;
	),

	TP_printk("comm=%s pid=%d oldprio=%d newprio=%d",
			__entry->comm, __entry->pid,
			__entry->oldprio, __entry->newprio)
);

TRACE_EVENT(sched_get_nr_running_avg,

	TP_PROTO(int avg, int big_avg, int iowait_avg),

	TP_ARGS(avg, big_avg, iowait_avg),

	TP_STRUCT__entry(
		__field( int,	avg			)
		__field( int,	big_avg			)
		__field( int,	iowait_avg		)
	),

	TP_fast_assign(
		__entry->avg		= avg;
		__entry->big_avg	= big_avg;
		__entry->iowait_avg	= iowait_avg;
	),

	TP_printk("avg=%d big_avg=%d iowait_avg=%d",
		__entry->avg, __entry->big_avg, __entry->iowait_avg)
);

#endif /* _TRACE_SCHED_H */

/* This part must be outside protection */
#include <trace/define_trace.h>
