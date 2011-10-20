/*****************************************************************************
*  Copyright 2001 - 2011 Broadcom Corporation.  All rights reserved.
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2, available at
*  http://www.gnu.org/licenses/old-license/gpl-2.0.html (the "GPL").
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
*****************************************************************************/

#include <linux/module.h>
#include <linux/platform_device.h>
#ifdef CONFIG_MFD_BCMPMU
#include <linux/mfd/bcmpmu.h>
#else
#include <linux/mfd/bcm590xx/core.h>
#endif
#include <linux/usb/otg.h>
#include "bcmpmu_otg_xceiv.h"

#ifdef CONFIG_MFD_BCMPMU
int bcm_otg_do_adp_calibration_probe(struct bcmpmu *bcmpmu)
{
	/* Call PMU API */
	/* Call controller wrapper API with otg_data->platform_priv passed as parameter */


	return 0;
}

int bcm_otg_do_adp_probe(struct bcmpmu *bcmpmu)
{
	/* Call PMU API */
	return 0;
}

int bcm_otg_do_adp_sense(struct bcmpmu *bcmpmu)
{
	/* Call PMU API */
	return 0;
}

int bcm_otg_do_adp_sense_then_probe(struct bcmpmu *bcmpmu)
{
	/* Call PMU API */
	return 0;
}

#if 0 /* For future use */
static int bcm_otg_adp_change_callback(struct bcmpmu *bcmpmu, void *cb_data)
{
	return 0;
}
#endif
#else
int bcm_otg_do_adp_calibration_probe(struct bcm590xx *bcm590xx)
{
	/* Call PMU API */
	/* Call controller wrapper API with otg_data->platform_priv passed as parameter */


	return 0;
}

int bcm_otg_do_adp_probe(struct bcm590xx *bcm590xx)
{
	/* Call PMU API */
	return 0;
}

int bcm_otg_do_adp_sense(struct bcm590xx *bcm590xx)
{
	/* Call PMU API */
	return 0;
}

int bcm_otg_do_adp_sense_then_probe(struct bcm590xx *bcm590xx)
{
	/* Call PMU API */
	return 0;
}

#if 0 /* For future use */
static int bcm_otg_adp_change_callback(struct bcm590xx *bcm590xx, void *cb_data)
{
	return 0;
}
#endif
#endif

MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("USB OTG ADP driver");
MODULE_LICENSE("GPL");

