/* arch/arm/mach-rhea/include/mach/cdebugger.h
 *
 * Derived from sec_debug.h by Samsung Electronics Co.Ltd.
 * Copyright (c) 2010 Samsung Electronics Co., Ltd.
 *      http://www.samsung.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */
#ifndef CDEBUGGER_H
#define CDEBUGGER_H

#include <linux/sched.h>
#include <linux/semaphore.h>

#if defined(CONFIG_CDEBUGGER)

#define MAGIC_ADDR	(BCM21553_SCRATCHRAM_BASE + 0x7800)

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

extern void cdebugger_set_upload_magic(unsigned magic);

extern unsigned char _buf_log_main[512 * 1024];
extern unsigned char _buf_log_radio[256 * 1024];
extern unsigned char _buf_log_events[256 * 1024];
extern unsigned char _buf_log_system[256 * 1024];
extern char *log_buf;
extern int log_buf_len;
#else
static inline int cdebugger_init(void)
{
}
static inline int cdebugger_dump_stack(void)
{
}
static inline void cdebugger_check_crash_key(unsigned int code, int value)
{
}

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

void cdebugger_save_pte(void *pte, unsigned int faulttype)
{
}
static inline void cdebugger_set_upload_magic(unsigned magic)
{
}
#endif

#endif /* CDEBUGGER_H */
