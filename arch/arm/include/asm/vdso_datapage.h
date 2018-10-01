/*
 * Adapted from arm64 version.
 *
 * Copyright (C) 2012 ARM Limited
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef __ASM_VDSO_DATAPAGE_H
#define __ASM_VDSO_DATAPAGE_H

#ifdef __KERNEL__

#ifndef __ASSEMBLY__

#include <asm/page.h>

#ifndef _VDSO_WTM_CLOCK_SEC_T
#define _VDSO_WTM_CLOCK_SEC_T
typedef u32 vdso_wtm_clock_nsec_t;
#endif

#ifndef _VDSO_XTIME_CLOCK_SEC_T
#define _VDSO_XTIME_CLOCK_SEC_T
typedef u32 vdso_xtime_clock_sec_t;
#endif

#ifndef _VDSO_RAW_TIME_SEC_T
#define _VDSO_RAW_TIME_SEC_T
typedef u32 vdso_raw_time_sec_t;
#endif

/* Try to be cache-friendly on systems that don't implement the
 * generic timer: fit the unconditionally updated fields in the first
 * 32 bytes.
 */
struct vdso_data {
	u32 tb_seq_count;	/* sequence count - odd during updates */
	u16 use_syscall;	/* fall back to syscall if true */
	u16 cs_shift;		/* clocksource shift */
	u32 xtime_coarse_sec;	/* coarse time */
	u32 xtime_coarse_nsec;

	/* wall to monotonic offset */
	u32 wtm_clock_sec;
	vdso_wtm_clock_nsec_t	wtm_clock_nsec;
	/* CLOCK_REALTIME - seconds */
	vdso_xtime_clock_sec_t	xtime_clock_sec;
	u32 cs_mono_mult;	/* clocksource multiplier */

	u64 cs_cycle_last;	/* last cycle value */
	u64 cs_mask;		/* clocksource mask */

	u64 xtime_clock_snsec;	/* CLOCK_REALTIME sub-ns base */
	u32 tz_minuteswest;	/* timezone info for gettimeofday(2) */
	u32 tz_dsttime;

	u32 btm_sec;		/* monotonic to boot time */
	u32 btm_nsec;
	/* Raw clocksource multipler */
	u32 cs_raw_mult;
	/* Raw time */
	vdso_raw_time_sec_t raw_time_sec;
	u32 raw_time_nsec;
};

union vdso_data_store {
	struct vdso_data data;
	u8 page[PAGE_SIZE];
};

#endif /* !__ASSEMBLY__ */

#endif /* __KERNEL__ */

#endif /* __ASM_VDSO_DATAPAGE_H */
