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

#include <asm/barrier.h>
#include <linux/compiler.h>	/* for notrace				*/
#include <linux/math64.h>	/* for __iter_div_u64_rem()		*/
#include <uapi/linux/time.h>	/* for struct timespec			*/

#include "compiler.h"
#include "datapage.h"

DEFINE_FALLBACK(gettimeofday, struct timeval *, tv, struct timezone *, tz)
DEFINE_FALLBACK(clock_gettime, clockid_t, clock, struct timespec *, ts)
DEFINE_FALLBACK(clock_getres, clockid_t, clock, struct timespec *, ts)

static notrace u32 vdso_read_begin(const struct vdso_data *vd)
{
	u32 seq;

	do {
		seq = READ_ONCE(vd->tb_seq_count);

		if ((seq & 1) == 0)
			break;

		cpu_relax();
	} while (true);

	smp_rmb(); /* Pairs with second smp_wmb in update_vsyscall */
	return seq;
}

static notrace int vdso_read_retry(const struct vdso_data *vd, u32 start)
{
	u32 seq;

	smp_rmb(); /* Pairs with first smp_wmb in update_vsyscall */
	seq = READ_ONCE(vd->tb_seq_count);
	return seq != start;
}

static notrace int do_realtime_coarse(const struct vdso_data *vd,
				      struct timespec *ts)
{
	u32 seq;

	do {
		seq = vdso_read_begin(vd);

		ts->tv_sec = vd->xtime_coarse_sec;
		ts->tv_nsec = vd->xtime_coarse_nsec;

	} while (vdso_read_retry(vd, seq));

	return 0;
}

static notrace int do_monotonic_coarse(const struct vdso_data *vd,
				       struct timespec *ts)
{
	struct timespec tomono;
	u32 seq;
	u64 nsec;

	do {
		seq = vdso_read_begin(vd);

		ts->tv_sec = vd->xtime_coarse_sec;
		ts->tv_nsec = vd->xtime_coarse_nsec;

		tomono.tv_sec = vd->wtm_clock_sec;
		tomono.tv_nsec = vd->wtm_clock_nsec;

	} while (vdso_read_retry(vd, seq));

	ts->tv_sec += tomono.tv_sec;
	/* open coding timespec_add_ns */
	ts->tv_sec += __iter_div_u64_rem(ts->tv_nsec + tomono.tv_nsec,
					 NSEC_PER_SEC, &nsec);
	ts->tv_nsec = nsec;

	return 0;
}

#ifdef ARCH_PROVIDES_TIMER

/*
 * Returns the clock delta, in nanoseconds left-shifted by the clock
 * shift.
 */
static notrace u64 get_clock_shifted_nsec(const u64 cycle_last,
					  const u32 mult,
					  const u64 mask)
{
	u64 res;

	/* Read the virtual counter. */
	res = arch_vdso_read_counter();

	res = res - cycle_last;

	res &= mask;
	return res * mult;
}

static notrace int do_realtime(const struct vdso_data *vd, struct timespec *ts)
{
	u32 seq, mult, shift;
	u64 nsec, cycle_last;
#ifdef ARCH_CLOCK_FIXED_MASK
	static const u64 mask = ARCH_CLOCK_FIXED_MASK;
#else
	u64 mask;
#endif
	vdso_xtime_clock_sec_t sec;

	do {
		seq = vdso_read_begin(vd);

		if (vd->use_syscall)
			return -1;

		cycle_last = vd->cs_cycle_last;

		mult = vd->cs_mono_mult;
		shift = vd->cs_shift;
#ifndef ARCH_CLOCK_FIXED_MASK
		mask = vd->cs_mask;
#endif

		sec = vd->xtime_clock_sec;
		nsec = vd->xtime_clock_snsec;

	} while (unlikely(vdso_read_retry(vd, seq)));

	nsec += get_clock_shifted_nsec(cycle_last, mult, mask);
	nsec >>= shift;
	/* open coding timespec_add_ns to save a ts->tv_nsec = 0 */
	ts->tv_sec = sec + __iter_div_u64_rem(nsec, NSEC_PER_SEC, &nsec);
	ts->tv_nsec = nsec;

	return 0;
}

static notrace int do_monotonic(const struct vdso_data *vd, struct timespec *ts)
{
	u32 seq, mult, shift;
	u64 nsec, cycle_last;
#ifdef ARCH_CLOCK_FIXED_MASK
	static const u64 mask = ARCH_CLOCK_FIXED_MASK;
#else
	u64 mask;
#endif
	vdso_wtm_clock_nsec_t wtm_nsec;
	__kernel_time_t sec;

	do {
		seq = vdso_read_begin(vd);

		if (vd->use_syscall)
			return -1;

		cycle_last = vd->cs_cycle_last;

		mult = vd->cs_mono_mult;
		shift = vd->cs_shift;
#ifndef ARCH_CLOCK_FIXED_MASK
		mask = vd->cs_mask;
#endif

		sec = vd->xtime_clock_sec;
		nsec = vd->xtime_clock_snsec;

		sec += vd->wtm_clock_sec;
		wtm_nsec = vd->wtm_clock_nsec;

	} while (unlikely(vdso_read_retry(vd, seq)));

	nsec += get_clock_shifted_nsec(cycle_last, mult, mask);
	nsec >>= shift;
	nsec += wtm_nsec;
	/* open coding timespec_add_ns to save a ts->tv_nsec = 0 */
	ts->tv_sec = sec + __iter_div_u64_rem(nsec, NSEC_PER_SEC, &nsec);
	ts->tv_nsec = nsec;

	return 0;
}

static notrace int do_monotonic_raw(const struct vdso_data *vd,
				    struct timespec *ts)
{
	u32 seq, mult, shift;
	u64 nsec, cycle_last;
#ifdef ARCH_CLOCK_FIXED_MASK
	static const u64 mask = ARCH_CLOCK_FIXED_MASK;
#else
	u64 mask;
#endif
	vdso_raw_time_sec_t sec;

	do {
		seq = vdso_read_begin(vd);

		if (vd->use_syscall)
			return -1;

		cycle_last = vd->cs_cycle_last;

		mult = vd->cs_raw_mult;
		shift = vd->cs_shift;
#ifndef ARCH_CLOCK_FIXED_MASK
		mask = vd->cs_mask;
#endif

		sec = vd->raw_time_sec;
		nsec = vd->raw_time_nsec;

	} while (unlikely(vdso_read_retry(vd, seq)));

	nsec += get_clock_shifted_nsec(cycle_last, mult, mask);
	nsec >>= shift;
	/* open coding timespec_add_ns to save a ts->tv_nsec = 0 */
	ts->tv_sec = sec + __iter_div_u64_rem(nsec, NSEC_PER_SEC, &nsec);
	ts->tv_nsec = nsec;

	return 0;
}

static notrace int do_boottime(const struct vdso_data *vd, struct timespec *ts)
{
	u32 seq, mult, shift;
	u64 nsec, cycle_last, wtm_nsec;
#ifdef ARCH_CLOCK_FIXED_MASK
	static const u64 mask = ARCH_CLOCK_FIXED_MASK;
#else
	u64 mask;
#endif
	__kernel_time_t sec;

	do {
		seq = vdso_read_begin(vd);

		if (vd->use_syscall)
			return -1;

		cycle_last = vd->cs_cycle_last;

		mult = vd->cs_mono_mult;
		shift = vd->cs_shift;
#ifndef ARCH_CLOCK_FIXED_MASK
		mask = vd->cs_mask;
#endif

		sec = vd->xtime_clock_sec;
		nsec = vd->xtime_clock_snsec;

		sec += vd->wtm_clock_sec;
		wtm_nsec = vd->wtm_clock_nsec + vd->btm_nsec;

	} while (unlikely(vdso_read_retry(vd, seq)));

	nsec += get_clock_shifted_nsec(cycle_last, mult, mask);
	nsec >>= shift;
	nsec += wtm_nsec;

	/* open coding timespec_add_ns to save a ts->tv_nsec = 0 */
	ts->tv_sec = sec + __iter_div_u64_rem(nsec, NSEC_PER_SEC, &nsec);
	ts->tv_nsec = nsec;

	return 0;
}

#else /* ARCH_PROVIDES_TIMER */

static notrace int do_realtime(const struct vdso_data *vd, struct timespec *ts)
{
	return -1;
}

static notrace int do_monotonic(const struct vdso_data *vd, struct timespec *ts)
{
	return -1;
}

static notrace int do_monotonic_raw(const struct vdso_data *vd,
				    struct timespec *ts)
{
	return -1;
}

static notrace int do_boottime(const struct vdso_data *vd,
			       struct timespec *ts)
{
	return -1;
}

#endif /* ARCH_PROVIDES_TIMER */

notrace int __vdso_clock_gettime(clockid_t clock, struct timespec *ts)
{
	const struct vdso_data *vd = __get_datapage();

	switch (clock) {
	case CLOCK_REALTIME_COARSE:
		do_realtime_coarse(vd, ts);
		break;
	case CLOCK_MONOTONIC_COARSE:
		do_monotonic_coarse(vd, ts);
		break;
	case CLOCK_REALTIME:
		if (do_realtime(vd, ts))
			goto fallback;
		break;
	case CLOCK_MONOTONIC:
		if (do_monotonic(vd, ts))
			goto fallback;
		break;
	case CLOCK_MONOTONIC_RAW:
		if (do_monotonic_raw(vd, ts))
			goto fallback;
		break;
	case CLOCK_BOOTTIME:
		if (do_boottime(vd, ts))
			goto fallback;
		break;
	default:
		goto fallback;
	}

	return 0;
fallback:
	return clock_gettime_fallback(clock, ts);
}

notrace int __vdso_gettimeofday(struct timeval *tv, struct timezone *tz)
{
	const struct vdso_data *vd = __get_datapage();

	if (likely(tv != NULL)) {
		struct timespec ts;

		if (do_realtime(vd, &ts))
			return gettimeofday_fallback(tv, tz);

		tv->tv_sec = ts.tv_sec;
		tv->tv_usec = ts.tv_nsec / 1000;
	}

	if (unlikely(tz != NULL)) {
		tz->tz_minuteswest = vd->tz_minuteswest;
		tz->tz_dsttime = vd->tz_dsttime;
	}

	return 0;
}

int __vdso_clock_getres(clockid_t clock, struct timespec *res)
{
	long nsec;

	if (clock == CLOCK_REALTIME ||
	    clock == CLOCK_BOOTTIME ||
	    clock == CLOCK_MONOTONIC ||
	    clock == CLOCK_MONOTONIC_RAW)
		nsec = MONOTONIC_RES_NSEC;
	else if (clock == CLOCK_REALTIME_COARSE ||
		 clock == CLOCK_MONOTONIC_COARSE)
		nsec = LOW_RES_NSEC;
	else
		return clock_getres_fallback(clock, res);

	if (likely(res != NULL)) {
		res->tv_sec = 0;
		res->tv_nsec = nsec;
	}

	return 0;
}
