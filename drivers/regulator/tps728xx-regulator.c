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
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/regulator/tps728xx.h>
#include <linux/gpio.h>
#include <linux/slab.h>

/* In microVolt */
#define MIN_SUPPORTED_VOLTAGE		900000
#define MAX_SUPPORTED_VOLTAGE		3600000

#define HIGH				1
#define LOW				0

struct tps728xx {
	/* programmed output voltage in micro Volt */
	unsigned int vout0;
	unsigned int vout1;

	unsigned int vset_gpio;
	int enable_gpio;
	struct regulator_dev *regl;
};

static struct regulator_ops ts728xx_regl_ops;

static struct regulator_desc regl_desc = {
	.name	= "tps728xx_ldo",
	.ops	= &ts728xx_regl_ops,
	.type	= REGULATOR_VOLTAGE,
	.n_voltages = 2,
	.owner	= THIS_MODULE,
};

static int tps728xx_regulator_enable(struct regulator_dev *rdev)
{
	struct tps728xx *tps728xx = rdev_get_drvdata(rdev);
	pr_info("Inside %s\n", __func__);
	if (tps728xx->enable_gpio < 0) {
		/* If LDO EN line is not muxed with a GPIO then user of this LDO
		 * would take care of enabling/disabling the LDO
		*/
		pr_info("%s: TPS728XX LDO should be enabled by external driver\n", __func__);
		return 0;
	}

	gpio_set_value(tps728xx->enable_gpio, HIGH);
	return 0;
}

static int tps728xx_regulator_disable(struct regulator_dev *rdev)
{
	struct tps728xx *tps728xx = rdev_get_drvdata(rdev);
	pr_info("Inside %s\n", __func__);
	if (tps728xx->enable_gpio < 0) {
		/* If LDO EN line is not muxed with a GPIO then user of this LDO
		 * would take care of enabling/disabling the LDO
		*/
		pr_info("%s: TPS728XX LDO should be disabled by external driver\n", __func__);
		return 0;
	}

	gpio_set_value(tps728xx->enable_gpio, LOW);

	return 0;
}

static int tps728xx_regulator_is_enabled(struct regulator_dev *rdev)
{
	struct tps728xx *tps728xx = rdev_get_drvdata(rdev);
	int status;
	pr_info("Inside %s\n", __func__);
	if (tps728xx->enable_gpio < 0) {
		/* If LDO EN line is not muxed with a GPIO then user of this LDO
		 * would take care of enabling/disabling the LDO
		*/
		pr_info("%s: TPS728XX LDO enabling taken care by external driver\n", __func__);
		return 0;
	}

	status = gpio_get_value_cansleep(tps728xx->enable_gpio);
	if (status == 1)
		return 1;
	return 0;
}

static int tps728xx_regulator_set_voltage(struct regulator_dev *rdev,
						int min_uV, int max_uV,
						unsigned *selector)
{
	struct tps728xx *tps728xx = rdev_get_drvdata(rdev);
	int val = -1;

	pr_info("Inside %s: minUv %d, maxuV %d\n", __func__, min_uV, max_uV);

	if (min_uV > max_uV)
		val = -1;
	if (tps728xx->vout0 >= min_uV)
		val = 0;
	else if (tps728xx->vout1 == max_uV)
		val = 1;

	if (val == -1)
		return -EINVAL;
	if (val == 1)
		gpio_set_value(tps728xx->vset_gpio, HIGH);
	else
		gpio_set_value(tps728xx->vset_gpio, LOW);

	return 0;
}

static int tps728xx_regulator_get_voltage(struct regulator_dev *rdev)
{
	struct tps728xx *tps728xx = rdev_get_drvdata(rdev);
	int status;
	pr_info("Inside %s\n", __func__);

	status = gpio_get_value_cansleep(tps728xx->enable_gpio);
	if (status == 0)
		return status;
	status = gpio_get_value_cansleep(tps728xx->vset_gpio);
	if (status == 1)
		return tps728xx->vout1;
	return tps728xx->vout0;
}

static struct regulator_ops ts728xx_regl_ops = {
	.enable = tps728xx_regulator_enable,
	.disable = tps728xx_regulator_disable,
	.is_enabled = tps728xx_regulator_is_enabled,
	.set_voltage = tps728xx_regulator_set_voltage,
	.get_voltage = tps728xx_regulator_get_voltage,
};


static int tps728xx_regulator_probe(struct platform_device *pdev)
{
	struct tps728xx_plat_data *pdata = pdev->dev.platform_data;
	int status;
	struct tps728xx *tps728xx;
	pr_info("TPS728XX Regulator Driver\n");
	pr_info("Inside %s\n", __func__);
	tps728xx = kzalloc(sizeof(struct tps728xx), GFP_KERNEL);
	if (unlikely(!tps728xx)) {
		pr_info("%s: Could not create tps728xx\n", __func__);
		return -ENOMEM;
	}

	tps728xx->vout0= pdata->vout0;
	tps728xx->vout1= pdata->vout1;
	if ((tps728xx->vout0 < MIN_SUPPORTED_VOLTAGE) ||
			(tps728xx->vout1 > MAX_SUPPORTED_VOLTAGE)) {
		pr_info("%s: Wrong voltage has been passed from platform\n", __func__);
		status = -EINVAL;
		goto err_gpio;
	}

	tps728xx->vset_gpio = pdata->gpio_vset;
	tps728xx->enable_gpio = pdata->gpio_en;

	platform_set_drvdata(pdev, tps728xx);
	pr_debug("%s: EN_GPIO %d, VSET_GPIO %d, Vout0 %d, Vout1 %d\n", __func__,
			tps728xx->enable_gpio, tps728xx->vset_gpio,
			tps728xx->vout0, tps728xx->vout1);
	/* set the ts728xx enable GPIO as output and disable the regulator */
	status = gpio_request(tps728xx->enable_gpio, "tps728xx_enable");
	if (status < 0) {
		pr_info("%s: Unable to request tps728xx enable GPIO (%d)\n",
				__func__, tps728xx->enable_gpio);
		goto err_gpio;
	}
	gpio_direction_output(tps728xx->enable_gpio, 0);

	/* set the ts728xx vset GPIO as output and set it 0 (lowest voltage) */
	status = gpio_request(tps728xx->vset_gpio, "tps728xx_vset");
	if (status < 0) {
		pr_info("%s: Unable to request tps728xx vset GPIO (%d)\n",
				__func__, tps728xx->vset_gpio);
		goto err_gpio;
	}
	gpio_direction_output(tps728xx->vset_gpio, 0);


	/* register regulator */
	tps728xx->regl = regulator_register(&regl_desc, &pdev->dev, pdata->initdata, tps728xx, NULL);
	regulator_has_full_constraints();

	pr_info("%s SUCCESS\n", __func__);
	return 0;
err_gpio:
	kfree(tps728xx);
	return status;
}

static int tps728xx_regulator_remove(struct platform_device *pdev)
{
	struct tps728xx *tps728xx = platform_get_drvdata(pdev);
	regulator_unregister(tps728xx->regl);
	gpio_free(tps728xx->enable_gpio);
	gpio_free(tps728xx->vset_gpio);
	kfree(tps728xx);
	return 0;
}

static struct platform_driver tps728xx_regulator_driver = {
	.probe	= tps728xx_regulator_probe,
	.remove	= __devexit_p(tps728xx_regulator_remove),
	.driver	= {
		.name	= "tps728xx-regulator",
	},
};

static int __init tps728xx_regulator_init(void)
{
	return platform_driver_register(&tps728xx_regulator_driver);
}
subsys_initcall(tps728xx_regulator_init);

static void __exit tps728xx_regulator_exit(void)
{
	platform_driver_unregister(&tps728xx_regulator_driver);
}
module_exit(tps728xx_regulator_exit);

/* Module information */
MODULE_AUTHOR("TKG");
MODULE_DESCRIPTION("TPS728XX regulator driver");
MODULE_LICENSE("GPL");
