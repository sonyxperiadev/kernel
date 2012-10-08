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
#include "Statistics.h"


void Statistics_Initialise(StatisticsType *Instance)
{
	Instance->Sum = Instance->Sum2 = 0;
	Instance->Minimum = ~0;
	Instance->Maximum = 0;
	Instance->Samples = 0;
}

void Statistics_Add(StatisticsType *Instance, unsigned int Sample)
{
	Instance->Sum  += Sample;
	Instance->Sum2 += (uint64_t) Sample * (uint64_t) Sample;
	if (Sample > Instance->Maximum)
		Instance->Maximum = Sample;
	if (Sample < Instance->Minimum)
		Instance->Minimum = Sample;
	++Instance->Samples;
}


/* ================================================================ */

static uint32_t SquareRoot(uint64_t x)
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
void Statistics_Calculate(
	const StatisticsType *Instance,
	unsigned int          MeanMultiplier,
	uint64_t             *Mean,
	unsigned int          SDMultiplier,
	uint64_t             *StandardDeviation,
	unsigned int         *Minimum,
	unsigned int         *Maximum,
	unsigned int         *Samples)
{
	uint64_t Sum, Sum2, StandardDeviation2n2, StandardDeviationMn;
	*Samples = Instance->Samples;
	*Minimum = Instance->Minimum;
	*Maximum = Instance->Maximum;
	if (Instance->Samples == 0)
		return;

	Sum   = Instance->Sum;
	*Mean = div64_u64((uint64_t) MeanMultiplier * Sum + (uint64_t) (Instance->Samples / 2), (uint64_t) Instance->Samples);

	Sum2 = Instance->Sum2;
	StandardDeviation2n2 = (uint64_t) Instance->Samples * Sum2 - Sum * Sum;
	StandardDeviationMn  = SquareRoot(SDMultiplier * SDMultiplier * StandardDeviation2n2);
	*StandardDeviation = div64_u64(StandardDeviationMn + (uint64_t) (Instance->Samples / 2), (uint64_t) Instance->Samples);
}
