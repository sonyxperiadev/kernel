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
#include <linux/time.h>

#include "compiler.h"
#include "datapage.h"

DEFINE_FALLBACK(gettimeofday, struct timeval *, tv, struct timezone *, tz)
DEFINE_FALLBACK(clock_gettime, clockid_t, clock, struct timespec *, ts)

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

	do {
		seq = vdso_read_begin(vd);

		ts->tv_sec = vd->xtime_coarse_sec;
		ts->tv_nsec = vd->xtime_coarse_nsec;

		tomono.tv_sec = vd->wtm_clock_sec;
		tomono.tv_nsec = vd->wtm_clock_nsec;

	} while (vdso_read_retry(vd, seq));

	ts->tv_sec += tomono.tv_sec;
	timespec_add_ns(ts, tomono.tv_nsec);

	return 0;
}

#ifdef CONFIG_ARM_ARCH_TIMER

static notrace u64 get_ns(const struct vdso_data *vd)
{
	u64 cycle_delta;
	u64 cycle_now;
	u64 nsec;

	cycle_now = arch_vdso_read_counter();

	cycle_delta = (cycle_now - vd->cs_cycle_last) & vd->cs_mask;

	nsec = (cycle_delta * vd->cs_mono_mult) + vd->xtime_clock_snsec;
	nsec >>= vd->cs_shift;

	return nsec;
}

static notrace int do_realtime(const struct vdso_data *vd, struct timespec *ts)
{
	u64 nsecs;
	u32 seq;

	do {
		seq = vdso_read_begin(vd);

		if (vd->use_syscall)
			return -1;

		ts->tv_sec = vd->xtime_clock_sec;
		nsecs = get_ns(vd);

	} while (vdso_read_retry(vd, seq));

	ts->tv_nsec = 0;
	timespec_add_ns(ts, nsecs);

	return 0;
}

static notrace int do_monotonic(const struct vdso_data *vd, struct timespec *ts)
{
	struct timespec tomono;
	u64 nsecs;
	u32 seq;

	do {
		seq = vdso_read_begin(vd);

		if (vd->use_syscall)
			return -1;

		ts->tv_sec = vd->xtime_clock_sec;
		nsecs = get_ns(vd);

		tomono.tv_sec = vd->wtm_clock_sec;
		tomono.tv_nsec = vd->wtm_clock_nsec;

	} while (vdso_read_retry(vd, seq));

	ts->tv_sec += tomono.tv_sec;
	ts->tv_nsec = 0;
	timespec_add_ns(ts, nsecs + tomono.tv_nsec);

	return 0;
}

#else /* CONFIG_ARM_ARCH_TIMER */

static notrace int do_realtime(const struct vdso_data *vd, struct timespec *ts)
{
	return -1;
}

static notrace int do_monotonic(const struct vdso_data *vd, struct timespec *ts)
{
	return -1;
}

#endif /* CONFIG_ARM_ARCH_TIMER */

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
