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

#ifndef V3D_STATISTICS_H_
#define V3D_STATISTICS_H_

#include <linux/types.h>


typedef struct statistics_tag {
	uint64_t     sum;
	uint64_t     sum2;
	unsigned int minimum;
	unsigned int maximum;
	unsigned int samples;
} statistics_t;

extern void statistics_initialise(statistics_t *instance);
extern void statistics_add(statistics_t *instance, unsigned int sample);

/* Multiplier multiplies up all calculated values, so that decimal places can be represented */
extern void statistics_calculate(
	const statistics_t *instance,
	unsigned int          mean_multiplier,
	uint64_t             *mean,
	unsigned int          s_d_multiplier,
	uint64_t             *standard_deviation,
	unsigned int         *minimum,
	unsigned int         *maximum,
	unsigned int         *samples);


#endif /* ifndef V3D_STATISTICS_H_ */
