/*******************************************************************************
Copyright 2012 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement
governing use of this software, this software is licensed to you under the
terms of the GNU General Public License version 2, available at
http://www.gnu.org/copyleft/gpl.html (the "GPL").

Notwithstanding the above, under no circumstances may you combine this software
in any way with any other Broadcom software provided under a license other than
the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

#include <linux/kernel.h>
#include <linux/math64.h>
#include "statistics.h"


void statistics_initialise(statistics_t *instance)
{
	instance->sum = instance->sum2 = 0;
	instance->minimum = ~0;
	instance->maximum = 0;
	instance->samples = 0;
}

void statistics_add(statistics_t *instance, unsigned int sample)
{
	instance->sum  += sample;
	instance->sum2 += (uint64_t) sample * (uint64_t) sample;
	if (sample > instance->maximum)
		instance->maximum = sample;
	if (sample < instance->minimum)
		instance->minimum = sample;
	++instance->samples;
}


/* ================================================================ */

static uint32_t square_root(uint64_t x)
{
	uint64_t a = 0;
	uint64_t e = 0;
	uint64_t r = 0;
	unsigned int i;
	for (i = 0 ; i != (sizeof(x) << 2) ; ++i) {
		r <<= 2;
		r += x >> ((sizeof(x) << 3) - 2);
		x <<= 2;

		a <<= 1;
		e = (a << 1) | 1;
		if (r >= e) {
			r -= e;
			++a;
		}
	}
	return (uint32_t) a;
}

/* Multiplier multiplies up all calculated values, so that decimal places can be represented */
void statistics_calculate(
	const statistics_t *instance,
	unsigned int          mean_multiplier,
	uint64_t             *mean,
	unsigned int          s_d_multiplier,
	uint64_t             *standard_deviation,
	unsigned int         *minimum,
	unsigned int         *maximum,
	unsigned int         *samples)
{
	uint64_t sum, sum2, standard_deviation2n2, standard_deviation_mn;
	*samples = instance->samples;
	*minimum = instance->minimum;
	*maximum = instance->maximum;
	if (instance->samples == 0)
		return;

	sum   = instance->sum;
	*mean = div64_u64((uint64_t) mean_multiplier * sum + (uint64_t) (instance->samples / 2), (uint64_t) instance->samples);

	sum2 = instance->sum2;
	standard_deviation2n2 = (uint64_t) instance->samples * sum2 - sum * sum;
	standard_deviation_mn  = square_root(s_d_multiplier * s_d_multiplier * standard_deviation2n2);
	*standard_deviation = div64_u64(standard_deviation_mn + (uint64_t) (instance->samples / 2), (uint64_t) instance->samples);
}
