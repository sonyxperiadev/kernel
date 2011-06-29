/*****************************************************************************
* Copyright 2004 - 2008 Broadcom Corporation.  All rights reserved.
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


/*
*
*****************************************************************************
*
*  knllog.h
*
*  Usage:
*  #include <linux/broadcom/knllog.h>
*
*  Then to use it, do the following:
*  KNLLOG("foo has value=0x%x units", 100);
*
*  The output will have a timestamp, optional delta time, poly prefix e.g.
*  12345678: 00000012: create_proc_entry   foo has value=0xc8 units
*
*  NOTES:
*  There are entries in /proc/sys/knllog, and the actual log buffer is in /proc/knllog.
*  /proc/sys/knllog/enable - enable (1) or disable (1) logging. e.g. echo 1 >/proc/sys/knllog/enable
*  /proc/sys/knllog/entries - resize log buffer. e.g. echo 10000 >/proc/sys/knllog/entries
*  /proc/sys/knllog/clear - clear/empty the log buffer. e.g. echo 1 >/proc/sys/knllog/clear
*  /proc/sys/knllog/deltatime - display timestamp deltas on log dump. e.g. echo 1 >/proc/sys/knllog/deltatime
*  /proc/sys/knllog/idx - READ ONLY - show the current next log entry index 0 .. entries-1
*  /proc/sys/knllog/wrap - READ ONLY - show the current buffer wrapped flag
*
*  To dump the log buffer from the cmd line, use "echo 1 > /proc/sys/knllog/dump-log"
*  The log buffer will be automatically dumped when an oops occurs.
*  knllog_init() is called from init code in traps.c.
*****************************************************************************/


#if !defined( KNLLOG_H )
#define KNLLOG_H

#include <stdarg.h>
#include <linux/types.h>

#ifdef CONFIG_BCM_KNLLOG_SUPPORT
#define KNLLOG(fmt, args...) KNLLOGCALL(__FUNCTION__, fmt, ## args)
#define KNLLOGCALL(func, fmt, args...) knllog_entry(func, fmt, ## args)
#define KNLLOG_DUMP_MEM(addr,mem,numBytes) knllog_dump_mem(__FUNCTION__, addr,mem,numBytes)

extern void knllog_entry(const char *function, const char *fmt, ...);
extern void knllog_ventry(const char *function, const char *fmt, va_list args);
extern void knllog_init(void);
extern void knllog_dump(void);
extern void knllog_dump_mem( const char *function, uint32_t addr, const void *voidMem, size_t numBytes );
extern void knllog_enable(void);
extern void knllog_disable(void);
extern void knllog_clear(void);
extern int knllog_idx(void);
extern int knllog_wrap(void);
extern int knllog_entries(void);
void knllog_capture_and_stop(void);
void knllog_sleeptime(int msec);
void knllog_logfile(int isFile);

#ifdef CONFIG_BCM_KNLLOG_IRQ
/*
 * A global flag that code inside CONFIG_BCM_KNLLOG_IRQ checks to decide to log or not. 
 * This allows a powerful debug feature to be enabled at runtime i.e. "echo 0xf >irq_sched_enable" 
 */
extern int gKnllogIrqSchedEnable;
#define KNLLOG_THREAD      1  /* Log thread scheduling */
#define KNLLOG_IRQ         2  /* Log IRQ activity */
#define KNLLOG_SOFTIRQ     4  /* Log Soft IRQ activity */
#define KNLLOG_TASKLET     8  /* Log tasklet activity */
#define KNLLOG_TIMER      16  /* Log timer callbacks */
#define KNLLOG_FB         32  /* Frame buffer updates */
#define KNLLOG_DMA        64  /* DMA transfer times */
#define KNLLOG_PROFILING 128  /* Additional profiling */
#endif

#else

#define KNLLOG(fmt, args...) KNLLOGCALL(__FUNCTION__, fmt, ## args)
#define KNLLOGCALL(func, fmt, args...)      do {} while (0)
#define KNLLOG_DUMP_MEM(addr,mem,numBytes)  do {} while (0)

#endif

#endif /* !defined( KNLLOG_H ) */

