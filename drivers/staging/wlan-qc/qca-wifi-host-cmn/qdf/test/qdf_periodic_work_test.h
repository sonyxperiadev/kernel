/*
 * Copyright (c) 2019 The Linux Foundation. All rights reserved.
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

#ifndef __QDF_PERIODIC_WORK_TEST
#define __QDF_PERIODIC_WORK_TEST

#ifdef WLAN_PERIODIC_WORK_TEST
/**
 * qdf_periodic_work_unit_test() - run the qdf periodic work unit test suite
 *
 * Return: number of failed test cases
 */
uint32_t qdf_periodic_work_unit_test(void);
#else
static inline uint32_t qdf_periodic_work_unit_test(void)
{
	return 0;
}
#endif /* WLAN_PERIODIC_WORK_TEST */

#endif /* __QDF_PERIODIC_WORK_TEST */

