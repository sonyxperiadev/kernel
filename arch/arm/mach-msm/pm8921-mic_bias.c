/*
 * Copyright (C) 2011 Sony Ericsson Mobile Communications AB.
 *
 * Authors: Shuhei Miyazaki <Shuhei.Miyazaki@sonyericsson.com>
 *          Hiroaki Hayashi <Hiroaki.Hayashi@sonyericsson.com>
 *          Daiki Yanagihara <Daiki.X.Yanagihara@sonyericsson.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * NOTE: This file contains code from:
 *     /kernel/arch/arm/mach-msm/simple_remote_msm8960_pf.c
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/mutex.h>

#include <linux/mfd/pm8xxx/core.h>

#include <mach/pm8921-mic_bias.h>

#define PM8921_OTHC_EN_SIG_MASK	0x3F
#define PM8921_OTHC_EN_SIG_SHIFT	0x6

#define OTHC_SIGNAL_OFF	0
#define OTHC_SIGNAL_ALWAYS_ON	3

#define WAIT_MIC_BIAS_VOLTAGE_STABLE_DELAY	20

#define LOCK(x)							\
	do {								\
		dev_dbg(loc_dat->dev, "%s - %d Locking mutex\n", __func__, \
			__LINE__);					\
		mutex_lock(x);						\
	} while (0)

#define UNLOCK(x)							\
	do {								\
		dev_dbg(loc_dat->dev, "%s - %d Unlocking mutex\n",	\
		       __func__, __LINE__);				\
		mutex_unlock(x);					\
	} while (0)

#define TRY_LOCK(x)				mutex_trylock(x)

struct local_data {
	struct pm8921_mic_bias_platform_data *mic_bias_pf;
	struct mutex lock;

	struct device *dev;

	int mic_bias_enable_counter;
};

static struct local_data *loc_dat;

int pm8921_mic_bias_enable(bool enable)
{
	int rc = 0;
	u8 reg;
	u16 mic_bias_addr = loc_dat->mic_bias_pf->mic_bias_addr;

	LOCK(&loc_dat->lock);

	dev_dbg(loc_dat->dev, "%s - %s MIC Bias\n", __func__,
			enable ? "Enabling" : "Disabling");

	dev_vdbg(loc_dat->dev, "%s - MIC_BIAS_COUNTER = %d\n", __func__,
			loc_dat->mic_bias_enable_counter);

	if (enable) {
		if (!loc_dat->mic_bias_enable_counter) {
			rc = pm8xxx_readb(loc_dat->dev->parent,
					mic_bias_addr, &reg);
			if (rc < 0)
				dev_err(loc_dat->dev, "PM8921 read failed\n");

			reg &= PM8921_OTHC_EN_SIG_MASK;
			reg |= (OTHC_SIGNAL_ALWAYS_ON <<
					PM8921_OTHC_EN_SIG_SHIFT);

			rc = pm8xxx_writeb(loc_dat->dev->parent,
					mic_bias_addr, reg);
			if (rc < 0)
				dev_err(loc_dat->dev, "PM8921 write failed\n");

			/*Wait a bit to get basicly stable ADC value*/
			msleep(WAIT_MIC_BIAS_VOLTAGE_STABLE_DELAY);
		}
		if (!rc) {
			dev_vdbg(loc_dat->dev,
					"%s - Increasing MIC_BIAS_COUNTER\n",
					__func__);
			loc_dat->mic_bias_enable_counter++;
			dev_vdbg(loc_dat->dev,
					"%s - MIC_BIAS_COUNTER = %u\n",
					__func__,
					loc_dat->mic_bias_enable_counter);
		}
	} else {
		if (1 == loc_dat->mic_bias_enable_counter) {
			rc = pm8xxx_readb(loc_dat->dev->parent,
					mic_bias_addr, &reg);
			if (rc < 0)
				dev_err(loc_dat->dev, "PM8921 read failed\n");

			reg &= PM8921_OTHC_EN_SIG_MASK;
			reg |= (OTHC_SIGNAL_OFF << PM8921_OTHC_EN_SIG_SHIFT);

			rc = pm8xxx_writeb(loc_dat->dev->parent,
					mic_bias_addr, reg);
			if (rc < 0)
				dev_err(loc_dat->dev, "PM8921 write failed\n");

			if (!rc)
				dev_dbg(loc_dat->dev,
						"%s - MIC Bias disabled\n",
						__func__);
		}
		if (1 <= loc_dat->mic_bias_enable_counter) {
			dev_vdbg(loc_dat->dev,
					"%s - Decreasing MIC_BIAS_COUNTER\n",
					__func__);
			loc_dat->mic_bias_enable_counter--;
			dev_vdbg(loc_dat->dev, "%s - MIC_BIAS_COUNTER = %d\n",
					__func__,
					loc_dat->mic_bias_enable_counter);
		} else {
			dev_vdbg(loc_dat->dev, "%s - No need to decrease "
					"MIC_BIAS_COUNTER\n", __func__);
			dev_vdbg(loc_dat->dev, "%s - MIC_BIAS_COUNTER = %u\n",
					__func__,
					loc_dat->mic_bias_enable_counter);
		}
	}

	if (rc)
		dev_err(loc_dat->dev, "Unable to toggle MIC Bias\n");

	UNLOCK(&loc_dat->lock);

	return rc;
}

static int pm8921_mic_bias_probe(struct platform_device *pdev)
{
	loc_dat = kzalloc(sizeof(*loc_dat), GFP_KERNEL);
	if (!loc_dat)
		return -ENOMEM;

	loc_dat->mic_bias_pf = pdev->dev.platform_data;
	loc_dat->dev = &pdev->dev;

	mutex_init(&loc_dat->lock);

	dev_info(loc_dat->dev, "Successfully registered\n");

	return 0;
}


static int pm8921_mic_bias_remove(struct platform_device *pd)
{
	(void)pm8921_mic_bias_enable(0);
	mutex_destroy(&loc_dat->lock);

	kfree(loc_dat);
	return 0;
}


static struct platform_driver pm8921_mic_bias_driver = {
	.probe = pm8921_mic_bias_probe,
	.remove = pm8921_mic_bias_remove,
	.driver		= {
		.name		= PM8921_MIC_BIAS_NAME,
		.owner		= THIS_MODULE,
	},
};


static int __init pm8921_mic_bias_init(void)
{
	return platform_driver_register(&pm8921_mic_bias_driver);
}

static void __exit pm8921_mic_bias_exit(void)
{
	platform_driver_unregister(&pm8921_mic_bias_driver);
}

module_init(pm8921_mic_bias_init);
module_exit(pm8921_mic_bias_exit);

MODULE_AUTHOR("Shuhei Miyazaki, Hiroaki Hayashi, Daiki Yanagihara");
MODULE_DESCRIPTION("PM8921 mic bias driver");
MODULE_LICENSE("GPL");
