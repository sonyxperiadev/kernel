/*****************************************************************************
*  Copyright 2001 - 2008 Broadcom Corporation.  All rights reserved.
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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/bug.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>

#include <linux/mfd/bcmpmu.h>

struct device *thermal_test_dev;

unsigned int thermal_get_adc(int adc_sig)
{
	struct bcmpmu *bcmpmu = thermal_test_dev->platform_data;
	struct bcmpmu_adc_req req;

	req.sig = adc_sig;
	req.tm = PMU_ADC_TM_HK;
	req.flags = PMU_ADC_RAW_AND_UNIT;
	bcmpmu->adc_req(bcmpmu, &req);
	return  req.raw;
}
EXPORT_SYMBOL(thermal_get_adc);

static int __devinit bcmpmu_thermal_probe(struct platform_device *pdev)
{

	hwmon_device_register(&pdev->dev);
	thermal_test_dev = &pdev->dev;

	return 0;

}

static int __devexit bcmpmu_thermal_remove(struct platform_device *pdev)
{
	hwmon_device_register(&pdev->dev);

	return 0;
}

static struct platform_driver bcmpmu_thermal_driver = {
	.driver = {
		   .name = "bcmpmu_thermal",
		   },
	.probe = bcmpmu_thermal_probe,
	.remove = __devexit_p(bcmpmu_thermal_remove),
};

static int __init thermal_init(void)
{
	return platform_driver_register(&bcmpmu_thermal_driver);
}

module_init(thermal_init);

static void __exit thermal_exit(void)
{
	platform_driver_unregister(&bcmpmu_thermal_driver);
}

module_exit(thermal_exit);

MODULE_DESCRIPTION("BCM thermal adc driver");
MODULE_LICENSE("GPL");
