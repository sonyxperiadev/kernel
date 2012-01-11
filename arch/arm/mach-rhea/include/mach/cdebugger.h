#ifndef CDEBUGGER_H
#define CDEBUGGER_H

#include <linux/sched.h>
#include <linux/semaphore.h>



#if defined(CONFIG_CDEBUGGER)

#define MAGIC_ADDR				BCM21553_SCRATCHRAM_BASE + 0x7800
#define BCM215XX_INFORM0			(MAGIC_ADDR + 0x00000004)
#define BCM215XX_INFORM1			(BCM215XX_INFORM0 + 0x00000004)
#define BCM215XX_INFORM2			(BCM215XX_INFORM1 + 0x00000004)
#define BCM215XX_INFORM3			(BCM215XX_INFORM2 + 0x00000004)
#define BCM215XX_INFORM4			(BCM215XX_INFORM3 + 0x00000004)
#define BCM215XX_INFORM5			(BCM215XX_INFORM4 + 0x00000004)
#define BCM215XX_INFORM6			(BCM215XX_INFORM5 + 0x00000004)
#define BCM215XX_INFORM7			(BCM215XX_INFORM6 + 0x00000004)


extern int cdebugger_init(void);
extern int cdebugger_dump_stack(void);
extern void cdebugger_check_crash_key(unsigned int code, int value);

extern void sec_getlog_supply_fbinfo(void *p_fb, u32 res_x, u32 res_y, u32 bpp,
				     u32 frames);
extern void sec_getlog_supply_meminfo(u32 size0, u32 addr0, u32 size1,
				      u32 addr1);
extern void sec_getlog_supply_loggerinfo(void *p_main, void *p_radio,
					 void *p_events, void *p_system);
extern void sec_getlog_supply_kloginfo(void *klog_buf);

extern void sec_gaf_supply_rqinfo(unsigned short curr_offset,
				  unsigned short rq_offset);

extern void cdebugger_save_pte(void *pte, int task_addr); 


#else
static inline int cdebugger_init(void)
{
}
static inline int cdebugger_dump_stack(void) {}
static inline void cdebugger_check_crash_key(unsigned int code, int value) {}

static inline void sec_getlog_supply_fbinfo(void *p_fb, u32 res_x, u32 res_y,
					    u32 bpp, u32 frames)
{
}

static inline void sec_getlog_supply_meminfo(u32 size0, u32 addr0, u32 size1,
					     u32 addr1)
{
}

static inline void sec_getlog_supply_loggerinfo(void *p_main,
						void *p_radio, void *p_events,
						void *p_system)
{
}

static inline void sec_getlog_supply_kloginfo(void *klog_buf)
{
}

static inline void sec_gaf_supply_rqinfo(unsigned short curr_offset,
					 unsigned short rq_offset)
{
}

void cdebugger_save_pte(void *pte, unsigned int faulttype )
{
}

#endif

#ifdef CONFIG_CDEBUGGER_SCHED_LOG
extern void cdebugger_task_sched_log(int cpu, struct task_struct *task);
extern void cdebugger_irq_sched_log(unsigned int irq, void *fn, int en);
extern void cdebugger_sched_log_init(void);
#else
static inline void cdebugger_task_sched_log(int cpu, struct task_struct *task)
{
}
static inline void cdebugger_irq_sched_log(unsigned int irq, void *fn, int en)
{
}
static inline void cdebugger_sched_log_init(void)
{
}
#endif

#ifdef CONFIG_CDEBUGGER_IRQ_EXIT_LOG
extern void cdebugger_irq_last_exit_log(void);
#else
static void cdebugger_irq_last_exit_log(void)
{
}
#endif

#ifdef CONFIG_CDEBUGGER_SEMAPHORE_LOG
extern void debug_semaphore_init(void);
extern void debug_semaphore_down_log(struct semaphore *sem);
extern void debug_semaphore_up_log(struct semaphore *sem);
extern void debug_rwsemaphore_init(void);
extern void debug_rwsemaphore_down_log(struct rw_semaphore *sem, int dir);
extern void debug_rwsemaphore_up_log(struct rw_semaphore *sem);
#define debug_rwsemaphore_down_read_log(x) \
	debug_rwsemaphore_down_log(x,READ_SEM)
#define debug_rwsemaphore_down_write_log(x) \
	debug_rwsemaphore_down_log(x,WRITE_SEM)
#else
static inline void debug_semaphore_init(void)
{
}
static inline void debug_semaphore_down_log(struct semaphore *sem)
{
}
static inline void debug_semaphore_up_log(struct semaphore *sem)
{
}
static inline void debug_rwsemaphore_init(void)
{
}
static inline void debug_rwsemaphore_down_read_log(struct rw_semaphore *sem)
{
}
static inline void debug_rwsemaphore_down_write_log(struct rw_semaphore *sem)
{
}
static inline void debug_rwsemaphore_up_log(struct rw_semaphore *sem)
{
}
#endif

#ifdef CONFIG_CDEBUGGER_SCHED_LOG

/* klaatu - schedule log */
#define SCHED_LOG_MAX 4096

struct irq_log{
	int cpu;
	int irq;
	void *fn;
	int en;
};

struct task_info{
	char comm[TASK_COMM_LEN];
	int cpu;
	pid_t pid;
};

union task_log{
	struct task_info task;
	struct irq_log irq;
};

struct sched_log{
	unsigned long long time;
	union task_log log;
};

#define SEMAPHORE_LOG_MAX 100
struct sem_debug{
	struct list_head list;
	struct semaphore *sem;
	struct task_struct *task;
	pid_t pid;
	int cpu;
	/* char comm[TASK_COMM_LEN]; */
};

enum {
	READ_SEM,
	WRITE_SEM
};

#endif /* CONFIG_CDEBUGGER_SCHED_LOG */

#ifdef CONFIG_CDEBUGGER_SEMAPHORE_LOG

#define RWSEMAPHORE_LOG_MAX 100
struct rwsem_debug{
	struct list_head list;
	struct rw_semaphore *sem;
	struct task_struct *task;
	pid_t pid;
	int cpu;
	int direction;
	/* char comm[TASK_COMM_LEN]; */
};


#endif /* CONFIG_CDEBUGGER_SEMAPHORE_LOG */



#endif /* CDEBUGGER_H */


