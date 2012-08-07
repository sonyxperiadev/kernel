/*******************************************************************************
Copyright 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement
governing use of this software, this software is licensed to you under the
terms of the GNU General Public License version 2, available at
http://www.gnu.org/copyleft/gpl.html (the "GPL").

Notwithstanding the above, under no circumstances may you combine this software
in any way with any other Broadcom software provided under a license other than
the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

#ifndef _DEV_POWER_H_
#define _DEV_POWER_H_

#include "mm_fw.h"
#include <plat/scu.h>



int dev_power_init(struct device_power_t *dev_power,
					  char *dev_name,
					  const char *dev_clk_name,
					  MM_DVFS *dvfs_params,
					  unsigned int bulk_job_count);
void dev_power_exit(struct device_power_t *dev_power, const char *dev_name);

void dev_clock_enable(device_t *dev);
void dev_clock_disable(device_t *dev);

#endif
