/*****************************************************************************
*
* Kona profiler framework
*
* Copyright 2012 Broadcom Corporation.  All rights reserved.
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

#ifndef _KONA_PROFILER_H
#define _KONA_PROFILER_H

#define COUNTER_CLK_RATE		3250
#define COUNTER_TO_MS(cnt)		(cnt/COUNTER_CLK_RATE)

#define PROFILER_ROOT_DIR_NAME			("profiler")
#define PROFILER_SUFFIX				("prof")
#define DEFINE_PROFILER(module_name, profiler_name)	\
	(module_name##_##profiler_name##_prof)
#define PROFILER_NAME(name)		(name##_##PROFILER_SUFFIX)

enum profiler_flags {
	PROFILER_RUNNING = 0x00000001,
	PROFILER_OVERFLOW = 0x00000002,
};

#define OVERFLOW_VAL			0x3FFFFFFF
#ifndef PROF_CIRC_BUFF_MAX_ENTRIES
#define PROF_CIRC_BUFF_MAX_ENTRIES	(256)
#endif

#ifndef PROF_NAME_MAX_LEN
#define PROF_NAME_MAX_LEN		(16)
#endif

extern int profiler_debug;

#if defined(DEBUG)
#define	profiler_dbg printk
#else
#define	profiler_dbg(format...)		\
	do {				\
		if (profiler_debug)	\
			printk(format);	\
	} while (0);
#endif

enum profiler_type {
	PROFILER_CCU,
	PROFILER_PI_ON,
	PROFILER_AXI_BUS,
	PROFILER_CPU,
	PROFILER_MEMC,
	PROFILER_L2C,
};

struct profiler;

struct prof_ops {
	int (*init) (struct profiler *profiler);
	int (*start) (struct profiler *profiler, int start);
	int (*status) (struct profiler *profiler);
	int (*get_counter) (struct profiler *profiler,
			unsigned long *counter, int *overflow);
	int (*print) (struct profiler *profiler);
	int (*start_profiler) (struct profiler *profiler,
			void *data);
};

/**
 * Kona profiler structure
 * Each Kona profiler is instance
 * of this struct
 */

struct profiler {
	struct list_head node;
	struct module *owner;
	struct dentry *dentry_dir;
	const char *name;
	enum profiler_flags flags;
	unsigned long start_time;
	unsigned long stop_time;
	struct prof_ops *ops;
	unsigned long running_time;
	int overflow;
	enum profiler_type prof_type;
};

/**
 * interface to kona profilers
 */

int start_profiler(char *name, void *data);
int stop_profiler(char *name);
int profiler_register(struct profiler *profiler);
int profiler_unregister(struct profiler *profiler);
void profiler_print(const char *log);
void profiler_idle_entry_cb(void);

#endif /* _KONA_PROFILER_H */
