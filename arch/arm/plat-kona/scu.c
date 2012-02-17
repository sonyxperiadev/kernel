 /*****************************************************************************
 *
 * Kona extensions to generic ARM scu API
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

#include <linux/module.h>
#include <linux/io.h>
#include <linux/err.h>
#include <linux/mutex.h>
#include <plat/scu.h>

static void __iomem *scu_base;
static DEFINE_MUTEX(scu_lock);

#if defined(CONFIG_RHEA_B0_PM_ASIC_WORKAROUND)
/* Ref counting for scu_standby signal disable requests */
static int scu_standby_disable_cnt;
module_param_named(scu_standby_disable, scu_standby_disable_cnt, int,
		   S_IRUGO | S_IWUSR | S_IWGRP);

int scu_standby(bool enable)
{
	unsigned int val;

	if (!scu_base)
		return -ENODEV;

	mutex_lock(&scu_lock);
	if (enable) {
		if (scu_standby_disable_cnt)
			scu_standby_disable_cnt--;

		if (scu_standby_disable_cnt == 0) {
			val = readl(scu_base + SCU_CONTROL_OFFSET);
			val |= SCU_CONTROL_SCU_STANDBY_EN_MASK;
			writel(val, scu_base + SCU_CONTROL_OFFSET);
		}
	} else {
		if (scu_standby_disable_cnt == 0) {
			val = readl(scu_base + SCU_CONTROL_OFFSET);
			val &= ~SCU_CONTROL_SCU_STANDBY_EN_MASK;
			writel(val, scu_base + SCU_CONTROL_OFFSET);
		}

		scu_standby_disable_cnt++;
	}
	mutex_unlock(&scu_lock);

	return 0;
}
#else
int scu_standby(bool enable)
{
	return 0;
}
#endif /* CONFIG_RHEA_B0_PM_ASIC_WORKAROUND */

void scu_invalidate_all(void)
{
	writel(0xFFFF, scu_base + SCU_INVALIDATE_ALL_OFFSET);
}

void scu_init(void __iomem *base)
{
	unsigned int val;

	scu_base = base;

	val = readl(scu_base + SCU_CONTROL_OFFSET);
	val |= SCU_CONTROL_SCU_STANDBY_EN_MASK;
	writel(val, scu_base + SCU_CONTROL_OFFSET);
}
