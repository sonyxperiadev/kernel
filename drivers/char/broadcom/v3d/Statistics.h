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


typedef struct StatisticsTag {
	uint64_t     Sum;
	uint64_t     Sum2;
	unsigned int Minimum;
	unsigned int Maximum;
	unsigned int Samples;
} StatisticsType;

extern void Statistics_Initialise(StatisticsType *Instance);
extern void Statistics_Add(StatisticsType *Instance, unsigned int Sample);

/* Multiplier multiplies up all calculated values, so that decimal places can be represented */
extern void Statistics_Calculate(
	const StatisticsType *Instance,
	unsigned int          MeanMultiplier,
	uint64_t             *Mean,
	unsigned int          SDMultiplier,
	uint64_t             *StandardDeviation,
	unsigned int         *Minimum,
	unsigned int         *Maximum,
	unsigned int         *Samples);


#endif /* ifndef V3D_STATISTICS_H_ */
