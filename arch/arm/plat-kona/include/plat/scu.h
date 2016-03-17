 /*****************************************************************************
 *
 * Kona extensions to ARM generic SCU driver
 *
 * Copyright 2012 Broadcom Corporation.  All rights reserved.
 *
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed to you
 * under the terms of the GNU General Public License version 2, available at
 * http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
 *
 * Notwithstanding the above, under no circumstances may you combine this
 * software in any way with any other Broadcom software provided under a
 * license other than the GPL, without Broadcom's express prior written
 * consent.
 *****************************************************************************/

#ifndef __SCU_H__
#define __SCU_H__

#ifdef CONFIG_MACH_HAWAII
#include <mach/rdb/brcm_rdb_scu.h>
#endif

enum {
	SCU_STATUS_NORMAL = 0,
	SCU_STATUS_DORMANT = 2,
	SCU_STATUS_OFF = 3,
};

int scu_standby(bool enable);
int scu_invalidate_all(void);
int scu_set_power_mode(unsigned int mode);
void scu_init(void __iomem * base);

#endif /* __SCU_H__ */
