/*
 * Copyright (c) 2014-2016 The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * DOC: i_qdf_mc_timer.h
 * Linux-specific definitions for QDF timers serialized to MC thread
 */

#if !defined(__I_QDF_MC_TIMER_H)
#define __I_QDF_MC_TIMER_H

/* Include Files */
#include <qdf_mc_timer.h>
#include <qdf_types.h>
#include <linux/timer.h>
#include <linux/time.h>
#include <linux/jiffies.h>
#include <linux/rtc.h>

/* Preprocessor definitions and constants */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/* Type declarations */

typedef struct qdf_mc_timer_platform_s {
	struct timer_list timer;
	int thread_id;
	uint32_t cookie;
	qdf_spinlock_t spinlock;
} qdf_mc_timer_platform_t;

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __I_QDF_MC_TIMER_H */
