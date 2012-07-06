/* kernel/arch/arm/mach-msm/felica-fuji_common.c
 *
 * Copyright (C) 2010-2012 Sony Ericsson Mobile Communications AB.
 *
 * Author: Hiroaki.Kuriyama <Hiroaki.Kuriyama@sonyericsson.com>
 * Author: Satomi.Watanabe <Satomi.Watanabe@sonyericsson.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/mfd/pmic8058.h>
#include <linux/regulator/consumer.h>
#include <linux/felica.h>
#include <mach/board-msm8660.h>

#define PM_GPIO_FELICA_PON    16
#define PM_GPIO_FELICA_RFS    26
#define PM_GPIO_FELICA_INT    27
#define MON_LEVEL_IS_2P90 0x0
#define MON_LEVEL_IS_2P95 0x1
#define MON_LEVEL_IS_3P00 0x2
#define MON_LEVEL_IS_3P05 0x3
#define TVDD_VOL_MIN 3050000
#define TVDD_VOL_MAX 3050000

static struct regulator *hvdd_reg;
static struct regulator *tvdd_reg;

static struct pm_gpio pm_felica_pon = {
	.direction      = PM_GPIO_DIR_OUT,
	.output_buffer  = PM_GPIO_OUT_BUF_CMOS,
	.output_value   = 0,
	.pull           = PM_GPIO_PULL_NO,
	.vin_sel        = PM8058_GPIO_VIN_S3,
	.out_strength   = PM_GPIO_STRENGTH_MED,
	.function       = PM_GPIO_FUNC_NORMAL,
	.inv_int_pol	= 0,
};

static struct pm_gpio pm_felica_rfs = {
	.direction      = PM_GPIO_DIR_IN,
	.output_buffer  = PM_GPIO_OUT_BUF_CMOS,
	.pull           = PM_GPIO_PULL_NO,
	.vin_sel        = PM8058_GPIO_VIN_S3,
	.out_strength   = PM_GPIO_STRENGTH_NO,
	.function       = PM_GPIO_FUNC_NORMAL,
	.inv_int_pol	= 0,
};

static struct pm_gpio pm_felica_int = {
	.direction      = PM_GPIO_DIR_IN,
	.output_buffer  = PM_GPIO_OUT_BUF_CMOS,
	.pull           = PM_GPIO_PULL_NO,
	.vin_sel        = PM8058_GPIO_VIN_S3,
	.out_strength   = PM_GPIO_STRENGTH_NO,
	.function       = PM_GPIO_FUNC_NORMAL,
	.inv_int_pol	= 0,
};

struct felica_pm_gpio_config {
	int                gpio;
	struct pm_gpio *param;
	const char *tag;
};

static struct felica_pm_gpio_config felica_pm_gpio[] = {
	{PM_GPIO_FELICA_PON, &pm_felica_pon, "felica_pon"},
	{PM_GPIO_FELICA_RFS, &pm_felica_rfs, "felica_rfs"},
	{PM_GPIO_FELICA_INT, &pm_felica_int, "felica_int"},
};

static int semc_felica_device_init(struct felica_dev *dev)
{
	int ret;
	int i, len;
	struct regulator *reg;

	if (!dev)
		return -EINVAL;

	dev_dbg(dev->dev, ": %s\n", __func__);

	/* Configure pm8xxx GPIO*/
	len = ARRAY_SIZE(felica_pm_gpio);
	for (i = 0; i < len; i++) {
		ret = gpio_request(
				PM8058_GPIO_PM_TO_SYS(felica_pm_gpio[i].gpio),
				felica_pm_gpio[i].tag);
		if (ret) {
			dev_err(dev->dev,
				"%s %s config failed.\n",
				 __func__, felica_pm_gpio[i].tag);
			return ret;
		}
		ret = pm8xxx_gpio_config(
				PM8058_GPIO_PM_TO_SYS(felica_pm_gpio[i].gpio),
				felica_pm_gpio[i].param);
		gpio_free(PM8058_GPIO_PM_TO_SYS(felica_pm_gpio[i].gpio));
		if (ret) {
			dev_err(dev->dev,
				"%s PM_GPIO_FELICA[%d] config failed.\n",
				 __func__, i);
			return ret;
		}
	}

	/* Configure VREG PM8901_MVS0 */
	reg = regulator_get(NULL, "8901_mvs0");
	if (IS_ERR(reg)) {
		dev_err(dev->dev, "%s: Get 8901_mvs0 failed.\n", __func__);
		return -ENODEV;
	}
	hvdd_reg = reg;
	ret = regulator_enable(hvdd_reg);
	if (ret) {
		dev_err(dev->dev, "%s: Enable regulator 8901_mvs0 failed.\n",
				__func__);
		regulator_put(hvdd_reg);
		return ret;
	}

	/* Request VREG PM8058 L19 */
	reg = regulator_get(NULL, "8058_l19");
	if (IS_ERR(reg)) {
		dev_err(dev->dev, "%s: Get 8058_l19 failed.\n", __func__);
		regulator_put(hvdd_reg);
		return -ENODEV;
	}
	tvdd_reg = reg;

	return 0;
}

static int semc_felica_reg_release(struct felica_dev *dev)
{
	if (!dev)
		return -EINVAL;

	dev_dbg(dev->dev, ": %s\n", __func__);

	regulator_put(hvdd_reg);
	regulator_put(tvdd_reg);

	return 0;
}

static int semc_felica_cen_init(struct felica_dev *dev)
{
	int ret;
	unsigned int st;
	struct pm8xxx_nfc_device *nfcdev;

	if (!dev)
		return -EINVAL;

	dev_dbg(dev->dev, ": %s\n", __func__);

	/* Get pm8xxx NFC device */
	nfcdev = pm8xxx_nfc_request();
	if (NULL == nfcdev) {
		dev_err(dev->dev,
				"%s: Error. PM-nfc not found.\n", __func__);
		return -ENODEV;
	}

	/* Read NFC support register */
	ret = pm8xxx_nfc_get_status(nfcdev, PM_NFC_CTRL_REQ, &st);
	if (ret) {
		dev_err(dev->dev,
				"%s: Error. PM-nfc access failed.\n", __func__);
		return -EIO;
	}
	dev_dbg(dev->dev,
			": pm8xxx NFC register = 0x%x\n", st);

	/* [If NFC support is not enabled,] write initial value. */
	if (!(st & PM_NFC_SUPPORT_EN)) {
		ret = pm8xxx_nfc_config(nfcdev, PM_NFC_CTRL_REQ,
			PM_NFC_SUPPORT_EN | PM_NFC_LDO_EN | MON_LEVEL_IS_2P95);
		if (ret) {
			dev_err(dev->dev,
				"%s: Cannot enable PM-nfc.\n", __func__);
			return -EIO;
		}
		dev_dbg(dev->dev,
				": Successfully enabled PM-nfc.\n");
	}

	return 0;
}

static int semc_felica_cen_read(u8 *buf, struct felica_dev *dev)
{
	int ret;
	unsigned int st = 0;
	struct pm8xxx_nfc_device *nfcdev;

	if (!buf || !dev)
		return -EINVAL;

	dev_dbg(dev->device_cen.this_device, ": %s\n", __func__);

	/* Get pm8xxx NFC device */
	nfcdev = pm8xxx_nfc_request();
	if (NULL == nfcdev) {
		dev_err(dev->device_cen.this_device,
				"%s: Error. PM-nfc not found.\n", __func__);
		return -ENODEV;
	}

	/* Read PMIC8058 NFC support register */
	ret = pm8xxx_nfc_get_status(nfcdev, PM_NFC_CTRL_REQ, &st);
	if (ret) {
		dev_err(dev->device_cen.this_device,
				"%s: Error. PM-nfc access failed.\n", __func__);
		return -EIO;
	}
	dev_dbg(dev->device_cen.this_device,
			": pm8xxx NFC register = 0x%x\n", st);
	*buf = (st & PM_NFC_EN) ? 0x1 : 0x0;

	return 0;
}

static int semc_felica_cen_write(u8 arg, struct felica_dev *dev)
{
	int ret;
	unsigned int val;
	struct pm8xxx_nfc_device *nfcdev;

	if (!dev)
		return -EINVAL;

	dev_dbg(dev->device_cen.this_device, ": %s\n", __func__);

	/* Write CEN value */
	if (0x0 == arg) {
		val = 0x0;
	} else if (0x1 == arg) {
		val = PM_NFC_EN;
	} else {
		dev_err(dev->device_cen.this_device,
			"%s: Error. Invalid val @CEN write.\n", __func__);
		return -EINVAL;
	}

	/* Get pm8xxx NFC device */
	nfcdev = pm8xxx_nfc_request();
	if (NULL == nfcdev) {
		dev_err(dev->device_cen.this_device,
				"%s: Error. PM-nfc not found.\n", __func__);
		return -ENODEV;
	}

	/* Write PMIC8058 NFC support register */
	ret = pm8xxx_nfc_config(nfcdev, PM_NFC_EN, val);
	if (ret) {
		dev_err(dev->device_cen.this_device,
				"%s: Error. Cannot write PM-nfc.\n", __func__);
		return -EIO;
	}

	return 0;
}

static int semc_felica_pon_init(struct felica_dev *dev)
{
	int ret;

	if (!dev)
		return -EINVAL;

	dev_dbg(dev->dev, ": %s\n", __func__);

	ret = gpio_request(
		PM8058_GPIO_PM_TO_SYS(PM_GPIO_FELICA_PON), "felica_pon");
	if (ret) {
		dev_err(dev->dev,
			"%s: Error. PON GPIO request failed.\n", __func__);
		return -ENODEV;
	}

	return 0;
}

static void semc_felica_pon_write(int val, struct felica_dev *dev)
{
	if (!dev)
		return;

	dev_dbg(dev->device_pon.this_device, ": %s\n", __func__);
	gpio_set_value_cansleep(
		PM8058_GPIO_PM_TO_SYS(PM_GPIO_FELICA_PON), val);
}

static void semc_felica_pon_release(struct felica_dev *dev)
{
	if (!dev)
		return;

	dev_dbg(dev->dev, ": %s\n", __func__);
	gpio_free(PM8058_GPIO_PM_TO_SYS(PM_GPIO_FELICA_PON));
}

static int semc_felica_tvdd_on(struct felica_dev *dev)
{
	int ret;

	if (!dev)
		return -EINVAL;

	dev_dbg(dev->dev, ": %s\n", __func__);

	ret = regulator_set_voltage(tvdd_reg, TVDD_VOL_MIN, TVDD_VOL_MAX);
	if (ret) {
		dev_err(dev->dev, "%s: Set 8058_l19 failed.\n", __func__);
		goto exit_free_reg_19;
	}
	ret = regulator_enable(tvdd_reg);
	if (ret) {
		dev_err(dev->dev, "%s: Enable regulator 8058_l19 failed.\n",
				__func__);
		goto exit_free_reg_19;
	}
	return 0;

exit_free_reg_19:
	return ret;
}

static void semc_felica_tvdd_off(struct felica_dev *dev)
{
	int ret;

	if (!dev)
		return;

	dev_dbg(dev->dev, ": %s\n", __func__);

	if (regulator_is_enabled(tvdd_reg)) {
		ret = regulator_disable(tvdd_reg);
		if (ret)
			dev_err(dev->dev,
				"%s: Disable regulator 8058_l19 failed.\n",
				__func__);
	}
}

static int semc_felica_rfs_init(struct felica_dev *dev)
{
	int ret;

	if (!dev)
		return -EINVAL;

	dev_dbg(dev->dev, ": %s\n", __func__);

	ret = gpio_request(
		PM8058_GPIO_PM_TO_SYS(PM_GPIO_FELICA_RFS), "felica_rfs");
	if (ret) {
		dev_err(dev->dev,
			"%s: Error. RFS GPIO request failed.\n", __func__);
		return -ENODEV;
	}

	return 0;
}

static int semc_felica_rfs_read(struct felica_dev *dev)
{
	if (!dev)
		return -EINVAL;

	dev_dbg(dev->device_rfs.this_device, ": %s\n", __func__);

	return gpio_get_value_cansleep(
		PM8058_GPIO_PM_TO_SYS(PM_GPIO_FELICA_RFS));
}

static void semc_felica_rfs_release(struct felica_dev *dev)
{
	if (!dev)
		return;

	dev_dbg(dev->dev, ": %s\n", __func__);
	gpio_free(PM8058_GPIO_PM_TO_SYS(PM_GPIO_FELICA_RFS));
}

static int semc_felica_int_init(struct felica_dev *dev)
{
	int ret;

	if (!dev)
		return -EINVAL;

	dev_dbg(dev->dev, ": %s\n", __func__);

	ret = gpio_request(
		PM8058_GPIO_PM_TO_SYS(PM_GPIO_FELICA_INT), "felica_int");
	if (ret) {
		dev_err(dev->dev,
			"%s: Error. INT GPIO request failed.\n\n", __func__);
		return -ENODEV;
	}

	return 0;
}

static int semc_felica_int_read(struct felica_dev *dev)
{
	if (!dev)
		return -EINVAL;

	dev_dbg(dev->dev, ": %s\n", __func__);

	return gpio_get_value_cansleep(
		PM8058_GPIO_PM_TO_SYS(PM_GPIO_FELICA_INT));
}

static void semc_felica_int_release(struct felica_dev *dev)
{
	if (!dev)
		return;

	dev_dbg(dev->dev, ": %s\n", __func__);
	gpio_free(PM8058_GPIO_PM_TO_SYS(PM_GPIO_FELICA_INT));
}

static struct felica_platform_data semc_felica_pfdata = {
	.cen_pfdata = {
		.cen_init = semc_felica_cen_init,
		.cen_read = semc_felica_cen_read,
		.cen_write = semc_felica_cen_write,
	},
	.pon_pfdata = {
		.pon_init = semc_felica_pon_init,
		.pon_write = semc_felica_pon_write,
		.pon_release = semc_felica_pon_release,
		.tvdd_on = semc_felica_tvdd_on,
		.tvdd_off = semc_felica_tvdd_off,
	},
	.rfs_pfdata = {
		.rfs_init = semc_felica_rfs_init,
		.rfs_read = semc_felica_rfs_read,
		.rfs_release = semc_felica_rfs_release,
	},
	.int_pfdata = {
		.int_init = semc_felica_int_init,
		.int_read = semc_felica_int_read,
		.int_release = semc_felica_int_release,
		.irq_int = PM8058_GPIO_IRQ(PM8058_IRQ_BASE,
						PM_GPIO_FELICA_INT),
	},
	.gpio_init = semc_felica_device_init,
	.reg_release = semc_felica_reg_release,
};

struct platform_device semc_felica_device = {
	.name = FELICA_DEV_NAME,
	.id = 0,
	.dev  = {
		.platform_data = &semc_felica_pfdata,
	},
};
