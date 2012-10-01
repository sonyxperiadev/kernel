/*****************************************************************************
* Copyright 2003 - 2008 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/

#include <linux/version.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/spinlock.h>
#include <linux/kthread.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/broadcom/knllog.h>
#include <linux/module.h>
#include <linux/threads.h>
#include <linux/percpu.h>

#include <linux/proc_fs.h>
#include <linux/sysctl.h>

#include <mach/profile_timer.h>

DEFINE_PER_CPU(spinlock_t, idle_lock);
DEFINE_PER_CPU(u32, idle_count);
EXPORT_PER_CPU_SYMBOL_GPL(idle_count);

#define DEFAULT_UPDATE_INTERVAL_MSEC 500
#define HISTORY_SIZE  10
typedef struct {
	unsigned int minCount;
	unsigned int maxCount;
	unsigned int avgCount;
	unsigned int currCount;
	unsigned int countHistory[HISTORY_SIZE];
} ICP_STATS;

typedef struct {
	/* CPU monitor on of off. Initialized at startup, read-only by kernel
	 * and write/read by user via proc entry */
	unsigned int enable;
	/* Forced reset of stats, Intiailized at startup, read-only by kernel
	 * and write/read by user via proc entry */
	unsigned int reset;
	/* Window size over which measurement is made. Initialized at startup,
	 * readonly by kernel, write/read by user via proc entry */
	unsigned int updateIntervalMsec;
	/* Weightage of current value to the average (in %). Initialized at
	 * startup, readonly by kernel, write/read by user via proc entry */
	unsigned int alphaFactor;
	/* Resets the stats after this interval expires . Initialized at
	 * startup, readonly by kernel, write/read by user via proc entry */
	unsigned int resetInterval;
	/* local timer to reset the stats, read/write by kernel,
	 * read only by user via proc entry */
	unsigned int resetTimer;
	/* read/write by lernel, read only by user via proc entry */
	unsigned int iterations;
	/* Read/write by kernel, read only by user via proc entry */
	ICP_STATS stats;
} ICP_STATE;

typedef struct {
	/* Load test on of off. Initialized at startup, read-only by kernel
	 * and write/read by user via proc entry */
	unsigned int enable;
	/* Period of when the load test is run, in mSec */
	unsigned int loadPeriod;
	/* Load percentage (in %). */
	unsigned int loadPercentage;
	/* Test load using thread context. */
	unsigned int useThread;
	/* Priority of load testing thread */
	unsigned int threadPriority;
	/* Number of time slices to spread the load across */
	unsigned int timeSlices;
	/* Flag indicating whether kernel timer used for CPU loading has
	 * started */
	unsigned int timerStarted;

	/* Load testing thread controls */
	int threadPid;
	struct completion threadLoadWaitLock;
	struct completion threadCompletion;

	struct timer_list timer;

	int load;
	int slice;
} ICP_LOAD_TEST;

typedef struct {
	u32 last_smtclk;
	u32 last_idle_count;
} idle_handle_t;

typedef struct {
	idle_handle_t idle_handle;
	u32 idle;
	u32 total;
} ICP_DATA;

typedef struct {
	int cpu_enum;
	struct task_struct *task;
	ICP_STATE state;
	ICP_STATS stats;
	ICP_LOAD_TEST loadtest;
	ICP_DATA data;
} ICP_OBJ;

static ICP_OBJ obj[NR_CPUS];

/* sysctl */
static int hostIcpThread(void *data);

static void init_idle_profile(idle_handle_t *handle);
static int get_idle_profile(idle_handle_t *handle, u32 *idle, u32 *total);

static struct ctl_table_header *gSysCtlHeader;
static int proc_do_hosticp_intvec_clear0(ctl_table *table, int write,
					 void __user *buffer, size_t *lenp,
					 loff_t *ppos);
static int proc_do_hosticp_intvec_clear1(ctl_table *table, int write,
					 void __user *buffer, size_t *lenp,
					 loff_t *ppos);

static int proc_do_hosticp_loadtest_intvec_enable0(ctl_table *table, int write,
						   void __user *buffer,
						   size_t *lenp, loff_t *ppos);
static int proc_do_hosticp_loadtest_intvec_enable1(ctl_table *table, int write,
						   void __user *buffer,
						   size_t *lenp, loff_t *ppos);

#define DEFINE_SYSCTL_ICPSTATS(id)   { \
   { \
      .procname   = "minCpu", \
      .data       = &obj[id].stats.minCount, \
      .maxlen     = sizeof( int ), \
      .mode       = 0644, \
      .proc_handler  = &proc_dointvec \
   }, \
   { \
      .procname   = "maxCpu", \
      .data       = &obj[id].stats.maxCount, \
      .maxlen     = sizeof( int ), \
      .mode       = 0644, \
      .proc_handler  = &proc_dointvec \
   }, \
   { \
      .procname   = "avgCpu", \
      .data       = &obj[id].stats.avgCount, \
      .maxlen     = sizeof( int ), \
      .mode       = 0644, \
      .proc_handler  = &proc_dointvec \
   }, \
   { \
      .procname   = "currCpu", \
      .data       = &obj[id].stats.currCount, \
      .maxlen     = sizeof( int ), \
      .mode       = 0644, \
      .proc_handler  = &proc_dointvec \
   }, \
   { \
      .procname   = "history", \
      .data       = &obj[id].stats.countHistory, \
      .maxlen     = HISTORY_SIZE * sizeof( int ), \
      .mode       = 0644, \
      .proc_handler  = &proc_dointvec \
   }, \
   { \
      .procname   = "summary", \
      .data       = &obj[id].stats, \
      .maxlen     = sizeof( ICP_STATS ), \
      .mode       = 0644, \
      .proc_handler  = &proc_dointvec \
   }, \
   {} \
}

static struct ctl_table gSysCtlIcpStats0[] = DEFINE_SYSCTL_ICPSTATS(0);
static struct ctl_table gSysCtlIcpStats1[] = DEFINE_SYSCTL_ICPSTATS(1);

#define DEFINE_SYSCTL_ICPLOADTEST(id)   { \
   { \
      .procname   = "enable", \
      .data       = &obj[id].loadtest.enable, \
      .maxlen     = sizeof( int ), \
      .mode       = 0644, \
      .proc_handler  = &proc_do_hosticp_loadtest_intvec_enable##id \
   }, \
   { \
      .procname   = "loadPeriod", \
      .data       = &obj[id].loadtest.loadPeriod, \
      .maxlen     = sizeof( int ), \
      .mode       = 0644, \
      .proc_handler  = &proc_dointvec \
   }, \
   { \
      .procname   = "loadPercentage", \
      .data       = &obj[id].loadtest.loadPercentage, \
      .maxlen     = sizeof( int ), \
      .mode       = 0644, \
      .proc_handler  = &proc_dointvec \
   }, \
   { \
      .procname   = "useThread", \
      .data       = &obj[id].loadtest.useThread, \
      .maxlen     = sizeof( int ), \
      .mode       = 0644, \
      .proc_handler  = &proc_dointvec \
   }, \
   { \
      .procname   = "threadPriority", \
      .data       = &obj[id].loadtest.threadPriority, \
      .maxlen     = sizeof( int ), \
      .mode       = 0644, \
      .proc_handler  = &proc_dointvec \
   }, \
   { \
      .procname   = "timeSlices", \
      .data       = &obj[id].loadtest.timeSlices, \
      .maxlen     = sizeof( int ), \
      .mode       = 0644, \
      .proc_handler  = &proc_dointvec \
   }, \
   {} \
}

static struct ctl_table gSysCtlIcpLoadTest0[] = DEFINE_SYSCTL_ICPLOADTEST(0);
static struct ctl_table gSysCtlIcpLoadTest1[] = DEFINE_SYSCTL_ICPLOADTEST(1);

#define DEFINE_SYSCTL_ICP(id)   { \
   { \
      .procname   = "enable", \
      .data       = &obj[id].state.enable, \
      .maxlen     = sizeof( int ), \
      .mode       = 0644, \
      .proc_handler  = &proc_dointvec \
   }, \
   { \
      .procname   = "reset", \
      .data       = &obj[id].state.reset, \
      .maxlen     = sizeof( int ), \
      .mode       = 0644, \
      .proc_handler  = &proc_do_hosticp_intvec_clear##id \
   }, \
   { \
      .procname   = "alpha-factor", \
      .data       = &obj[id].state.alphaFactor, \
      .maxlen     = sizeof( int ), \
      .mode       = 0644, \
      .proc_handler  = &proc_dointvec \
   }, \
   { \
      .procname   = "resetInterval", \
      .data       = &obj[id].state.resetInterval, \
      .maxlen     = sizeof( int ), \
      .mode       = 0644, \
      .proc_handler  = &proc_dointvec \
   }, \
   { \
      .procname   = "stats", \
      .child      = gSysCtlIcpStats##id, \
      .mode       = 0555, \
   }, \
   { \
      .procname   = "state", \
      .data       = &obj[id].state, \
      .maxlen     = sizeof( ICP_STATE ), \
      .mode       = 0644, \
      .proc_handler  = &proc_dointvec \
   }, \
   { \
      .procname   = "loadTest", \
      .child      = gSysCtlIcpLoadTest##id, \
      .mode       = 0555, \
   }, \
   {} \
}

static struct ctl_table gSysCtlIcp0[] = DEFINE_SYSCTL_ICP(0);
static struct ctl_table gSysCtlIcp1[] = DEFINE_SYSCTL_ICP(1);

static ctl_table gSysCtl[] = {
	{
	 .procname = "hostUsage0",
	 .mode = 0555,
	 .child = gSysCtlIcp0},
	{
	 .procname = "hostUsage1",
	 .mode = 0555,
	 .child = gSysCtlIcp1},
	{}
};

/* Actually create (and remove) the /proc file(s). */
static void icp_create_proc(void)
{
	create_proc_entry("hostUsage0", 0, NULL);
	if (num_online_cpus() != 1)
		create_proc_entry("hostUsage1", 0, NULL);

}

static void icp_remove_proc(void)
{
	/* no problem if it was not registered */
	remove_proc_entry("hostUsage0", NULL);
	if (num_online_cpus() != 1)
		remove_proc_entry("hostUsage1", NULL);
}

/* If we are writing the clear field, we reset the stats and start logging */
static int intvec_clear(int cpu, ctl_table *table, int write,
			void __user *buffer, size_t *lenp, loff_t *ppos)
{
	int rc = 0;

	if (!table || !table->data)
		return -EINVAL;

	if (write) {
		ICP_STATS *statsp = &obj[cpu].stats;
		ICP_STATE *statep = &obj[cpu].state;
		memset(statsp, 0, sizeof(*statsp));
		memset(statep, 0, sizeof(*statep));
		statsp->minCount = 1000;
		statep->alphaFactor = (1 << 4);	/* 1/16 in Q8 number */
	} else {
		/* No special processing for read. */
		rc = proc_dointvec(table, write, buffer, lenp, ppos);
	}
	return rc;
}

/* If we are writing the clear field, we reset the stats and start logging */
static int proc_do_hosticp_intvec_clear0(ctl_table *table, int write,
					 void __user *buffer, size_t *lenp,
					 loff_t *ppos)
{
	return intvec_clear(0, table, write, buffer, lenp, ppos);
}

/* If we are writing the clear field, we reset the stats and start logging */
static int proc_do_hosticp_intvec_clear1(ctl_table *table, int write,
					 void __user *buffer, size_t *lenp,
					 loff_t *ppos)
{
	return intvec_clear(1, table, write, buffer, lenp, ppos);
}

#define MAX_LOAD_PERIOD 1000
#define MAX_LOAD_PERCENTAGE 98
#define MAX_TIME_SLICES 50
static void ipc_load(int cpu)
{
	ICP_LOAD_TEST *lt = &obj[cpu].loadtest;
	int load_this_slice;
	int loops_this_slice;

	/*
	 * The test load is implemented in fixed length udelay loops.
	 * The target load percentage is specifed by loadPercentage,
	 * and can be spread across multiple time slots in the
	 * following manner:
	 *
	 * 1 time slot, 60% load:
	 *
	 * | 60% | 60% | 60%| ...
	 *
	 * 5 time slots, 60% load:
	 *
	 * | 98% | 98% | 98% |  6% |  0% | 98% | 98% | 98% |  6% |  0%| ...
	 *
	 * The maximum load of 98% per slot is enforced to prevent system
	 * deadlocks.
	 *
	 */
	if (lt->slice == 0) {
		lt->slice = lt->timeSlices;
		lt->load = lt->loadPercentage << 4;
	}
	load_this_slice = (MAX_LOAD_PERCENTAGE << 4) / lt->timeSlices;
	if (lt->load < load_this_slice)
		load_this_slice = lt->load;
	lt->load -= load_this_slice;
	load_this_slice = (load_this_slice * lt->timeSlices) >> 4;
	/*
	 * Convert load percentage for this slot into number of 10uSec loops,
	 * which is conveniently just multiplying the load percentage to the
	 * loop period in mSec.
	 */
	loops_this_slice = (load_this_slice * lt->loadPeriod);
	/*KNLLOG("loops_this_slice=%u load_this_slice=%u lt->loadPeriod=%u
	 * lt->timeSlices=%u lt->loadPercentage=%u lt->load=%u",
	 * loops_this_slice, load_this_slice, lt->loadPeriod, lt->timeSlices,
	 * lt->loadPercentage, lt->load); */

	while (loops_this_slice) {
		udelay(10);
		loops_this_slice--;
	}

	lt->slice--;
}

static int ipc_load_test_thread(void *data)
{
	int cpu = *((int *)data);
	ICP_LOAD_TEST *lt = &obj[cpu].loadtest;
	char strg[20];
	if (cpu > num_online_cpus()) {
		printk(KERN_ERR "%s: bad cpu number 0x%x\n", __func__, cpu);
		return -1;
	}
	sprintf(strg, "loadTest/%d", cpu);
	daemonize(strg);

	if (lt->threadPriority > 0) {
		struct sched_param param;
		param.sched_priority = (lt->threadPriority < MAX_RT_PRIO) ?
		    lt->threadPriority : (MAX_RT_PRIO - 1);
		sched_setscheduler(current, SCHED_FIFO, &param);
	}

	allow_signal(SIGKILL);
	allow_signal(SIGTERM);

	/* Run until signal received */
	while (1) {
		if (wait_for_completion_interruptible(&lt->threadLoadWaitLock)
		    == 0) {
			ipc_load(cpu);
		} else
			break;
	}
	complete_and_exit(&lt->threadCompletion, 0);
}

static void ipc_load_timer_func(ulong data)
{
	int cpu = (int)data;
	ICP_LOAD_TEST *lt = &obj[cpu].loadtest;
	struct timer_list *timer;
	timer = (struct timer_list *)&lt->timer;

	if (lt->timerStarted) {
		if (lt->loadPeriod > MAX_LOAD_PERIOD)
			lt->loadPeriod = MAX_LOAD_PERIOD;

		timer->expires += msecs_to_jiffies(lt->loadPeriod);
		add_timer(timer);

		if (lt->timeSlices < 1)
			lt->timeSlices = 1;
		if (lt->timeSlices > MAX_TIME_SLICES)
			lt->timeSlices = MAX_TIME_SLICES;
		if (lt->loadPercentage > MAX_LOAD_PERCENTAGE)
			lt->loadPercentage = MAX_LOAD_PERCENTAGE;

		if (lt->useThread)
			complete(&lt->threadLoadWaitLock);
		else
			ipc_load(cpu);
	}
}

/* If we are writing the enable field, we start/stop the kernel timer */
static int loadtest_intvec_enable(int cpu, ctl_table *table, int write,
				  void __user *buffer, size_t *lenp,
				  loff_t *ppos)
{
	int rc;
	ICP_OBJ *o = &obj[cpu];
	ICP_LOAD_TEST *lt = &o->loadtest;

	init_timer(&lt->timer);
	lt->timer.function = ipc_load_timer_func;
	lt->timer.expires = 0;
	lt->timer.data = (ulong) cpu;

	if (!table || !table->data)
		return -EINVAL;

	if (write) {
		/* use generic int handler to get input value */
		rc = proc_dointvec(table, write, buffer, lenp, ppos);

		if (rc < 0)
			return rc;

		if (!lt->timerStarted && lt->enable) {
			if (lt->useThread) {
				cpumask_t cpu_mask;
				init_completion(&lt->threadLoadWaitLock);
				init_completion(&lt->threadCompletion);

				cpumask_set_cpu(cpu, &cpu_mask);
				lt->threadPid =
				    kernel_thread(ipc_load_test_thread,
						  &o->cpu_enum, 0);
				sched_setaffinity(lt->threadPid, &cpu_mask);
				cpumask_clear_cpu(cpu, &cpu_mask);
			}
			if (lt->loadPeriod > MAX_LOAD_PERIOD)
				lt->loadPeriod = MAX_LOAD_PERIOD;
			lt->timer.expires =
			    jiffies + msecs_to_jiffies(lt->loadPeriod);
			add_timer(&lt->timer);
			lt->timerStarted = 1;
		} else if (lt->timerStarted && !lt->enable) {
			lt->timerStarted = 0;
			/* Kill load testing thread */
			if (lt->useThread) {
				if (lt->threadPid >= 0) {
					kill_proc_info(SIGTERM, SEND_SIG_PRIV,
						       lt->threadPid);
					wait_for_completion
					    (&lt->threadCompletion);
				}
				lt->threadPid = -1;
			}
		}
	} else {
		/* nothing special for read */
		return proc_dointvec(table, write, buffer, lenp, ppos);
	}
	return rc;
}

/* If we are writing the enable field, we start/stop the kernel timer */
static int proc_do_hosticp_loadtest_intvec_enable0(ctl_table *table, int write,
						   void __user *buffer,
						   size_t *lenp, loff_t *ppos)
{
	return loadtest_intvec_enable(0, table, write, buffer, lenp, ppos);
}

/* If we are writing the enable field, we start/stop the kernel timer */
static int proc_do_hosticp_loadtest_intvec_enable1(ctl_table *table, int write,
						   void __user *buffer,
						   size_t *lenp, loff_t *ppos)
{
	return loadtest_intvec_enable(1, table, write, buffer, lenp, ppos);
}

/*
 * Initialize by setting up the sysctl and proc/knllog entries, allocating 
 * default storage and resetting the variables. 
 */
static int __init host_cpu_usage_init(void)
{
	int i;

	gSysCtlHeader = register_sysctl_table(gSysCtl);

	icp_create_proc();

	memset(&obj, 0, sizeof(obj));	/* zero all */

	for (i = 0; i < num_online_cpus(); i++) {

		ICP_OBJ *o = &obj[i];
		ICP_LOAD_TEST *lt = &o->loadtest;
		ICP_STATS *statsp = &o->stats;
		ICP_STATE *statep = &o->state;
		ICP_DATA *datap = &o->data;
		cpumask_t cpu_mask;

		spin_lock_init(&per_cpu(idle_lock, i));
		o->cpu_enum = i;

		/* set affinity of each thread to force it to run on
		 * separate CPU */
		statsp->minCount = 1000;
		statep->enable = 1;
		statep->updateIntervalMsec = DEFAULT_UPDATE_INTERVAL_MSEC;
		statep->alphaFactor = (1 << 4);	/* 1/16 in Q8 number */

		/* Fixme move to loadtest thread startup? */
		lt->enable = 0;
		lt->loadPeriod = 10;
		lt->loadPercentage = 0;
		lt->useThread = 1;
		lt->threadPriority = 99;
		lt->timeSlices = 1;
		lt->timerStarted = 0;
		lt->threadPid = -1;

		init_idle_profile(&datap->idle_handle);
		cpumask_set_cpu(i, &cpu_mask);

		/* Launch a kernel thread */
		if ((o->task == NULL) || IS_ERR(o->task)) {
			o->task =
			    kthread_run(hostIcpThread, &o->cpu_enum,
					"hosticp/%d", i);
			if (IS_ERR(o->task)) {
				printk(KERN_ERR
				       "Init: failed to start host ICP thread: %ld\n",
				       PTR_ERR(o->task));
				return -1;
			}
		}
		sched_setaffinity(o->task->pid, &cpu_mask);
		cpumask_clear_cpu(i, &cpu_mask);
		printk(KERN_INFO "called host_cpu_usage_init for CPU%d\n", i);
	}
	return 0;
}

subsys_initcall(host_cpu_usage_init);

/* Exit and cleanup (probably not done) */
int __exit host_cpu_usage_exit(void)
{
	int i = 0;

	if (gSysCtlHeader != NULL)
		unregister_sysctl_table(gSysCtlHeader);

	icp_remove_proc();

	for (i = 0; i < num_online_cpus(); i++)
		kthread_stop(obj[i].task);

	return 0;
}

/*subsys_exitcall(host_cpu_usage_exit); */

int hostIcpThread(void *data)
{
	int cpu = *((int *)data);

	ICP_OBJ *o = &obj[cpu];
	ICP_STATS *statsp = &o->stats;
	ICP_STATE *statep = &o->state;
	ICP_DATA *datap = &o->data;

	printk(KERN_INFO "*************Starting host ICP thread for CPU%d"
				"**************\n", cpu);
	while (1) {
		set_current_state(TASK_INTERRUPTIBLE);
		schedule_timeout(((HZ * statep->updateIntervalMsec) / 1000) +
				 1);

		if (!(statep->enable))
			continue;
		{
			unsigned int temp[HISTORY_SIZE - 1];
			memcpy(&temp[0], &statsp->countHistory[0],
			       HISTORY_SIZE * sizeof(unsigned int));
			memcpy(&statsp->countHistory[1], &temp[0],
			       (HISTORY_SIZE - 1) * sizeof(unsigned int));
			statsp->countHistory[0] = statsp->currCount;
		}
		get_idle_profile(&datap->idle_handle, &datap->idle,
				 &datap->total);

		if (datap->idle > datap->total) {
			printk(KERN_ERR "%s: idle=0x%x > total=0x%x\n",
			       __func__, datap->idle, datap->total);
			datap->idle = datap->total;
		}
		if (datap->total < (0xFFFFFFFF / 1000)) {
			statsp->currCount =
			    (1000 * datap->idle) / (datap->total);
		} else {
			statsp->currCount =
			    (datap->idle) / (datap->total / 1000);
		}
		if (statsp->minCount > statsp->currCount)
			statsp->minCount = statsp->currCount;
		if (statsp->maxCount < statsp->currCount)
			statsp->maxCount = statsp->currCount;

		statsp->avgCount = ((statsp->currCount * statep->alphaFactor)
				    +
				    (statsp->avgCount *
				     ((1 << 8) - statep->alphaFactor))) >> 8;
		statsp->currCount = 1000 - statsp->currCount;
		statsp->minCount = 1000 - statsp->maxCount;
		statsp->maxCount = 1000 - statsp->minCount;
		statsp->avgCount = 1000 - statsp->avgCount;
		if (statsp->minCount < 0)
			statsp->minCount = 0;
		if (statsp->maxCount > 1000)
			statsp->maxCount = 1000;

		statep->iterations++;

		statep->resetTimer += statep->updateIntervalMsec;

		if ((statep->resetTimer > statep->resetInterval)
		    && (statep->resetInterval)) {
			memset(&statep->stats, 0, sizeof(ICP_STATS));
			statsp->minCount = -1;
			statep->resetTimer = 0;
		}
	}
	return 0;
}

/*
 * init_idle_profile - initialize profiling handle
 *
 * parameters:
 *   handle [in/out] - pointer to the profiling handle
 */
static void init_idle_profile(idle_handle_t *handle)
{
	unsigned long flags;
	spinlock_t *lockp = &get_cpu_var(idle_lock);
	spin_lock_irqsave(lockp, flags);

	handle->last_smtclk = timer_get_tick_count();
	handle->last_idle_count = idle_count;

	spin_unlock_irqrestore(lockp, flags);
	put_cpu_var(idle_lock);
}

/*
 * get_idle_profile - get the idle profiling results for the provided handle
 *
 * parameters:
 *   handle [in] - pointer to the profiling handle, must be previously initialized
 *   idle [out]  - returns the number of idle cycles since last init or get call
 *   total [out] - returns the number of total cycles since last init or get call
 *
 * return:
 *   The number of cycles per second
 *
 * note:
 *   To prevent overflowing the cycle counters, the get call must be made no
 *   later than (2^32 / ticks_per_second) seconds from the last init or get call.
 *   For 1024 ticks per second, the time is 4,194,304 seconds = 48.54 days;
 *   For 812500 ticks per second, the time is 5286 seconds = 1 hour 28 minutes.
 */
static int get_idle_profile(idle_handle_t *handle, u32 *idle, u32 *total)
{
	unsigned long flags;
	spinlock_t *lockp = &get_cpu_var(idle_lock);
	u32 now;
	u32 cur_idle_count;

	spin_lock_irqsave(lockp, flags);

	now = timer_get_tick_count();
	cur_idle_count = get_cpu_var(idle_count);
	put_cpu_var(idle_count);

	*idle = cur_idle_count - handle->last_idle_count;
	*total = now - handle->last_smtclk;

	handle->last_idle_count = cur_idle_count;
	handle->last_smtclk = now;

	spin_unlock_irqrestore(lockp, flags);
	put_cpu_var(idle_lock);
	return timer_get_tick_rate();
}
