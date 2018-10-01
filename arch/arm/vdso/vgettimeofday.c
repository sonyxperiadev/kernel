/*
 * Userspace implementations of gettimeofday() and friends.
 *
 * Copyright (C) 2017 Cavium, Inc.
 * Copyright (C) 2015 Mentor Graphics Corporation
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
 *
 * Author: Will Deacon <will.deacon@arm.com>
 * Rewriten from arch64 version into C by: Andrew Pinski <apinski@cavium.com>
 * Reworked and rebased over arm version by: Mark Salyzyn <salyzyn@android.com>
 */

#include <linux/compiler.h>
#include <linux/hrtimer.h>
#include <linux/time.h>
#include <asm/arch_timer.h>
#include <asm/barrier.h>
#include <asm/bug.h>
#include <asm/page.h>
#include <asm/unistd.h>
#include <asm/vdso_datapage.h>

#ifndef CONFIG_AEABI
#error This code depends on AEABI system call conventions
#endif

extern struct vdso_data *__get_datapage(void);

static notrace u32 vdso_read_begin(const struct vdso_data *vdata)
{
	u32 seq;

	do {
		seq = READ_ONCE(vdata->tb_seq_count);

		if ((seq & 1) == 0)
			break;

		cpu_relax();
	} while (true);

	smp_rmb(); /* Pairs with second smp_wmb in update_vsyscall */
	return seq;
}

static notrace int vdso_read_retry(const struct vdso_data *vdata, u32 start)
{
	u32 seq;

	smp_rmb(); /* Pairs with first smp_wmb in update_vsyscall */
	seq = READ_ONCE(vdata->tb_seq_count);
	return seq != start;
}

static notrace long clock_gettime_fallback(clockid_t _clkid,
					   struct timespec *_ts)
{
	register struct timespec *ts asm("r1") = _ts;
	register clockid_t clkid asm("r0") = _clkid;
	register long ret asm ("r0");
	register long nr asm("r7") = __NR_clock_gettime;

	asm volatile(
	"	swi #0\n"
	: "=r" (ret)
	: "r" (clkid), "r" (ts), "r" (nr)
	: "memory");

	return ret;
}

static notrace int do_realtime_coarse(struct timespec *ts,
				      struct vdso_data *vdata)
{
	u32 seq;

	do {
		seq = vdso_read_begin(vdata);

		ts->tv_sec = vdata->xtime_coarse_sec;
		ts->tv_nsec = vdata->xtime_coarse_nsec;

	} while (vdso_read_retry(vdata, seq));

	return 0;
}

static notrace int do_monotonic_coarse(struct timespec *ts,
				       struct vdso_data *vdata)
{
	struct timespec tomono;
	u32 seq;

	do {
		seq = vdso_read_begin(vdata);

		ts->tv_sec = vdata->xtime_coarse_sec;
		ts->tv_nsec = vdata->xtime_coarse_nsec;

		tomono.tv_sec = vdata->wtm_clock_sec;
		tomono.tv_nsec = vdata->wtm_clock_nsec;

	} while (vdso_read_retry(vdata, seq));

	ts->tv_sec += tomono.tv_sec;
	timespec_add_ns(ts, tomono.tv_nsec);

	return 0;
}

#ifdef CONFIG_ARM_ARCH_TIMER

static notrace u64 get_ns(struct vdso_data *vdata)
{
	u64 cycle_delta;
	u64 cycle_now;
	u64 nsec;

	cycle_now = arch_counter_get_cntvct();

	cycle_delta = (cycle_now - vdata->cs_cycle_last) & vdata->cs_mask;

	nsec = (cycle_delta * vdata->cs_mono_mult) + vdata->xtime_clock_snsec;
	nsec >>= vdata->cs_shift;

	return nsec;
}

static notrace int do_realtime(struct timespec *ts, struct vdso_data *vdata)
{
	u64 nsecs;
	u32 seq;

	do {
		seq = vdso_read_begin(vdata);

		if (vdata->use_syscall)
			return -1;

		ts->tv_sec = vdata->xtime_clock_sec;
		nsecs = get_ns(vdata);

	} while (vdso_read_retry(vdata, seq));

	ts->tv_nsec = 0;
	timespec_add_ns(ts, nsecs);

	return 0;
}

static notrace int do_monotonic(struct timespec *ts, struct vdso_data *vdata)
{
	struct timespec tomono;
	u64 nsecs;
	u32 seq;

	do {
		seq = vdso_read_begin(vdata);

		if (vdata->use_syscall)
			return -1;

		ts->tv_sec = vdata->xtime_clock_sec;
		nsecs = get_ns(vdata);

		tomono.tv_sec = vdata->wtm_clock_sec;
		tomono.tv_nsec = vdata->wtm_clock_nsec;

	} while (vdso_read_retry(vdata, seq));

	ts->tv_sec += tomono.tv_sec;
	ts->tv_nsec = 0;
	timespec_add_ns(ts, nsecs + tomono.tv_nsec);

	return 0;
}

#else /* CONFIG_ARM_ARCH_TIMER */

static notrace int do_realtime(struct timespec *ts, struct vdso_data *vdata)
{
	return -1;
}

static notrace int do_monotonic(struct timespec *ts, struct vdso_data *vdata)
{
	return -1;
}

#endif /* CONFIG_ARM_ARCH_TIMER */

notrace int __vdso_clock_gettime(clockid_t clkid, struct timespec *ts)
{
	struct vdso_data *vdata;
	int ret = -1;

	vdata = __get_datapage();

	switch (clkid) {
	case CLOCK_REALTIME_COARSE:
		ret = do_realtime_coarse(ts, vdata);
		break;
	case CLOCK_MONOTONIC_COARSE:
		ret = do_monotonic_coarse(ts, vdata);
		break;
	case CLOCK_REALTIME:
		ret = do_realtime(ts, vdata);
		break;
	case CLOCK_MONOTONIC:
		ret = do_monotonic(ts, vdata);
		break;
	default:
		break;
	}

	if (ret)
		ret = clock_gettime_fallback(clkid, ts);

	return ret;
}

static notrace long gettimeofday_fallback(struct timeval *_tv,
					  struct timezone *_tz)
{
	register struct timezone *tz asm("r1") = _tz;
	register struct timeval *tv asm("r0") = _tv;
	register long ret asm ("r0");
	register long nr asm("r7") = __NR_gettimeofday;

	asm volatile(
	"	swi #0\n"
	: "=r" (ret)
	: "r" (tv), "r" (tz), "r" (nr)
	: "memory");

	return ret;
}

notrace int __vdso_gettimeofday(struct timeval *tv, struct timezone *tz)
{
	struct timespec ts;
	struct vdso_data *vdata;
	int ret;

	vdata = __get_datapage();

	ret = do_realtime(&ts, vdata);
	if (ret)
		return gettimeofday_fallback(tv, tz);

	if (tv) {
		tv->tv_sec = ts.tv_sec;
		tv->tv_usec = ts.tv_nsec / 1000;
	}
	if (tz) {
		tz->tz_minuteswest = vdata->tz_minuteswest;
		tz->tz_dsttime = vdata->tz_dsttime;
	}

	return ret;
}

/* Avoid unresolved references emitted by GCC */

void __aeabi_unwind_cpp_pr0(void)
{
}

void __aeabi_unwind_cpp_pr1(void)
{
}

void __aeabi_unwind_cpp_pr2(void)
{
}
