/* arch/arm/mach-msm/nfc-fuji_nozomi.c
 *
 * Copyright (C) 2012 Sony Mobile Communications AB.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include <linux/interrupt.h>
#include <linux/err.h>
#include <linux/mfd/pmic8058.h>
#include <linux/mfd/pm8xxx/nfc.h>
#include "nfc-fuji.h"

#ifdef CONFIG_NFC_PN544

int pn544_pm8xxx_config(unsigned int flag, int enable)
{
	int ret = 0;
	unsigned int status = 0;
	struct pm8xxx_nfc_device *pmic_nfc;

	pmic_nfc = pm8xxx_nfc_request();
	if (!pmic_nfc) {
		pr_err("%s: cannot open pmic-nfc\n", __func__);
		return -ENODEV;
	}

	ret = pm8xxx_nfc_get_status(pmic_nfc, PM_NFC_CTRL_REQ, &status);
	if (ret < 0) {
		pr_err("%s: cannot get status from pmic\n", __func__);
		return ret;
	}

	if (enable)
		status |= flag;
	else
		status &= ~flag;

	ret = pm8xxx_nfc_config(pmic_nfc, PM_NFC_CTRL_REQ, status);
	if (ret < 0)
		pr_err("%s: cannot set status to pmic\n", __func__);

	return ret;
}

int pn544_driver_loaded(void)
{
	int ret = 0;

	ret = pn544_gpio_request();
	/* Initialize non-related registers first */
	if (!IS_ERR_VALUE(ret))
		ret = pn544_pm8xxx_config(PM_NFC_VDDLDO_MON_LEVEL |
			PM_NFC_VPH_PWR_EN | PM_NFC_EXT_VDDLDO_EN, false);

	if (!IS_ERR_VALUE(ret))
		ret = pn544_pm8xxx_config(PM_NFC_SUPPORT_EN | PM_NFC_EN, true);

	return ret;
}

int pn544_driver_opened(void)
{
	return pn544_pm8xxx_config(PM_NFC_LDO_EN, true);
}

void pn544_driver_closed(void)
{
	pn544_pm8xxx_config(PM_NFC_LDO_EN, false);
}

struct pn544_i2c_platform_data pn544_pdata = {
	.irq_type = IRQF_TRIGGER_RISING,
	.chip_config = pn544_chip_config,
	.driver_loaded = pn544_driver_loaded,
	.driver_unloaded = pn544_gpio_release,
	.driver_opened = pn544_driver_opened,
	.driver_closed = pn544_driver_closed,
};

#endif

