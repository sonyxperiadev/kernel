/*****************************************************************************
* Copyright 2006 - 2013 Broadcom Corporation.  All rights reserved.
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

/*
 * Frameworks:
 *
 *    - SMP:          Fully supported.    Big mutex to keep everything safe.
 *    - GPIO:         Fully supported.    No GPIOs are used.
 *    - MMU:          Fully supported.    Platform model with ioremap used.
 *    - Dynamic /dev: Fully supported.    Uses PWM framework.
 *    - Suspend:      Fully supported.    PMWs disabled and cloks released.
 *    - Clocks:       Fully supported.    Done.
 *    - Power:        Fully supported.    Clocks disabled when PWMs not in use.
 *
 */

/*
 * KONA PWM driver
 *
 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/completion.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/pwm.h>
#include <linux/of.h>

#define KONA_PWM_CHANNEL_CNT			6
#define PWM_PRESCALER_MAX			7
#define DEFAULT_DUTY				0
#define DEFAULT_PERIOD				0
#define DEFAULT_POLARITY			1

#define PWM_TOP_PWM_CONTROL_OFFSET			0
#define PWM_TOP_PWM_CONTROL_SMOOTH_TYPE_SHIFT		24
#define PWM_TOP_PWM_CONTROL_PWMOUT_ENABLE_SHIFT		0
#define PWM_TOP_PWM_CONTROL_PWMOUT_POLARITY_SHIFT	8

#define PWM_TOP_PRESCALE_CONTROL_OFFSET                 0x00000004
#define PWM_TOP_PRESCALE_CONTROL_PWM5_PRESCALE_SHIFT    20
#define PWM_TOP_PRESCALE_CONTROL_PWM5_PRESCALE_MASK     0x00700000
#define PWM_TOP_PRESCALE_CONTROL_PWM4_PRESCALE_SHIFT    16
#define PWM_TOP_PRESCALE_CONTROL_PWM4_PRESCALE_MASK     0x00070000
#define PWM_TOP_PRESCALE_CONTROL_PWM3_PRESCALE_SHIFT    12
#define PWM_TOP_PRESCALE_CONTROL_PWM3_PRESCALE_MASK     0x00007000
#define PWM_TOP_PRESCALE_CONTROL_PWM2_PRESCALE_SHIFT    8
#define PWM_TOP_PRESCALE_CONTROL_PWM2_PRESCALE_MASK     0x00000700
#define PWM_TOP_PRESCALE_CONTROL_PWM1_PRESCALE_SHIFT    4
#define PWM_TOP_PRESCALE_CONTROL_PWM1_PRESCALE_MASK     0x00000070
#define PWM_TOP_PRESCALE_CONTROL_PWM0_PRESCALE_SHIFT    0
#define PWM_TOP_PRESCALE_CONTROL_PWM0_PRESCALE_MASK     0x00000007

#define PWM_TOP_PWM0_PERIOD_COUNT_OFFSET                0x00000008
#define PWM_TOP_PWM0_PERIOD_COUNT_PWM0_CNT_SHIFT        0
#define PWM_TOP_PWM0_PERIOD_COUNT_PWM0_CNT_MASK         0x00FFFFFF
#define PWM_TOP_PWM0_DUTY_CYCLE_HIGH_OFFSET             0x0000000C
#define PWM_TOP_PWM0_DUTY_CYCLE_HIGH_PWM0_HIGH_SHIFT    0
#define PWM_TOP_PWM0_DUTY_CYCLE_HIGH_PWM0_HIGH_MASK     0x00FFFFFF

#define PWM_TOP_PWM1_PERIOD_COUNT_OFFSET                0x00000010
#define PWM_TOP_PWM1_PERIOD_COUNT_PWM1_CNT_SHIFT        0
#define PWM_TOP_PWM1_PERIOD_COUNT_PWM1_CNT_MASK         0x00FFFFFF
#define PWM_TOP_PWM1_DUTY_CYCLE_HIGH_OFFSET             0x00000014
#define PWM_TOP_PWM1_DUTY_CYCLE_HIGH_PWM1_HIGH_SHIFT    0
#define PWM_TOP_PWM1_DUTY_CYCLE_HIGH_PWM1_HIGH_MASK     0x00FFFFFF

#define PWM_TOP_PWM2_PERIOD_COUNT_OFFSET                0x00000018
#define PWM_TOP_PWM2_PERIOD_COUNT_PWM2_CNT_SHIFT        0
#define PWM_TOP_PWM2_PERIOD_COUNT_PWM2_CNT_MASK         0x00FFFFFF
#define PWM_TOP_PWM2_DUTY_CYCLE_HIGH_OFFSET             0x0000001C
#define PWM_TOP_PWM2_DUTY_CYCLE_HIGH_PWM2_HIGH_SHIFT    0
#define PWM_TOP_PWM2_DUTY_CYCLE_HIGH_PWM2_HIGH_MASK     0x00FFFFFF

#define PWM_TOP_PWM3_PERIOD_COUNT_OFFSET                0x00000020
#define PWM_TOP_PWM3_PERIOD_COUNT_PWM3_CNT_SHIFT        0
#define PWM_TOP_PWM3_PERIOD_COUNT_PWM3_CNT_MASK         0x00FFFFFF
#define PWM_TOP_PWM3_DUTY_CYCLE_HIGH_OFFSET             0x00000024
#define PWM_TOP_PWM3_DUTY_CYCLE_HIGH_PWM3_HIGH_SHIFT    0
#define PWM_TOP_PWM3_DUTY_CYCLE_HIGH_PWM3_HIGH_MASK     0x00FFFFFF

#define PWM_TOP_PWM4_PERIOD_COUNT_OFFSET                0x00000028
#define PWM_TOP_PWM4_PERIOD_COUNT_PWM4_CNT_SHIFT        0
#define PWM_TOP_PWM4_PERIOD_COUNT_PWM4_CNT_MASK         0x00FFFFFF
#define PWM_TOP_PWM4_DUTY_CYCLE_HIGH_OFFSET             0x0000002C
#define PWM_TOP_PWM4_DUTY_CYCLE_HIGH_PWM4_HIGH_SHIFT    0
#define PWM_TOP_PWM4_DUTY_CYCLE_HIGH_PWM4_HIGH_MASK     0x00FFFFFF

#define PWM_TOP_PWM5_PERIOD_COUNT_OFFSET                0x00000030
#define PWM_TOP_PWM5_PERIOD_COUNT_PWM5_CNT_SHIFT        0
#define PWM_TOP_PWM5_PERIOD_COUNT_PWM5_CNT_MASK         0x00FFFFFF
#define PWM_TOP_PWM5_DUTY_CYCLE_HIGH_OFFSET             0x00000034
#define PWM_TOP_PWM5_DUTY_CYCLE_HIGH_PWM5_HIGH_SHIFT    0
#define PWM_TOP_PWM5_DUTY_CYCLE_HIGH_PWM5_HIGH_MASK     0x00FFFFFF

static DEFINE_MUTEX(pwm_lock);         /* lock for data access */

struct pwm_reg_def {
	u32 mask;
	u32 shift;
	u32 offset;
};

#define PWM_REG_DEF(m, s, a) { \
		.mask           =       m,  \
		.shift          =       s, \
		.offset         =       a \
	}

static const struct pwm_reg_def
		pwm_chan_pre_scaler_info[KONA_PWM_CHANNEL_CNT] = {
	[0] = PWM_REG_DEF(PWM_TOP_PRESCALE_CONTROL_PWM0_PRESCALE_MASK,
		    PWM_TOP_PRESCALE_CONTROL_PWM0_PRESCALE_SHIFT,
		    PWM_TOP_PRESCALE_CONTROL_OFFSET),
	[1] = PWM_REG_DEF(PWM_TOP_PRESCALE_CONTROL_PWM1_PRESCALE_MASK,
		    PWM_TOP_PRESCALE_CONTROL_PWM1_PRESCALE_SHIFT,
		    PWM_TOP_PRESCALE_CONTROL_OFFSET),
	[2] = PWM_REG_DEF(PWM_TOP_PRESCALE_CONTROL_PWM2_PRESCALE_MASK,
		    PWM_TOP_PRESCALE_CONTROL_PWM2_PRESCALE_SHIFT,
		    PWM_TOP_PRESCALE_CONTROL_OFFSET),
	[3] = PWM_REG_DEF(PWM_TOP_PRESCALE_CONTROL_PWM3_PRESCALE_MASK,
		    PWM_TOP_PRESCALE_CONTROL_PWM3_PRESCALE_SHIFT,
		    PWM_TOP_PRESCALE_CONTROL_OFFSET),
	[4] = PWM_REG_DEF(PWM_TOP_PRESCALE_CONTROL_PWM4_PRESCALE_MASK,
		    PWM_TOP_PRESCALE_CONTROL_PWM4_PRESCALE_SHIFT,
		    PWM_TOP_PRESCALE_CONTROL_OFFSET),
	[5] = PWM_REG_DEF(PWM_TOP_PRESCALE_CONTROL_PWM5_PRESCALE_MASK,
		    PWM_TOP_PRESCALE_CONTROL_PWM5_PRESCALE_SHIFT,
		    PWM_TOP_PRESCALE_CONTROL_OFFSET),
};

static const struct pwm_reg_def pwm_duty_cycle_info[KONA_PWM_CHANNEL_CNT] = {
	[0] = PWM_REG_DEF(PWM_TOP_PWM0_DUTY_CYCLE_HIGH_PWM0_HIGH_MASK,
		    PWM_TOP_PWM0_DUTY_CYCLE_HIGH_PWM0_HIGH_SHIFT,
		    PWM_TOP_PWM0_DUTY_CYCLE_HIGH_OFFSET),
	[1] = PWM_REG_DEF(PWM_TOP_PWM1_DUTY_CYCLE_HIGH_PWM1_HIGH_MASK,
		    PWM_TOP_PWM1_DUTY_CYCLE_HIGH_PWM1_HIGH_SHIFT,
		    PWM_TOP_PWM1_DUTY_CYCLE_HIGH_OFFSET),
	[2] = PWM_REG_DEF(PWM_TOP_PWM2_DUTY_CYCLE_HIGH_PWM2_HIGH_MASK,
		    PWM_TOP_PWM2_DUTY_CYCLE_HIGH_PWM2_HIGH_SHIFT,
		    PWM_TOP_PWM2_DUTY_CYCLE_HIGH_OFFSET),
	[3] = PWM_REG_DEF(PWM_TOP_PWM3_DUTY_CYCLE_HIGH_PWM3_HIGH_MASK,
		    PWM_TOP_PWM3_DUTY_CYCLE_HIGH_PWM3_HIGH_SHIFT,
		    PWM_TOP_PWM3_DUTY_CYCLE_HIGH_OFFSET),
	[4] = PWM_REG_DEF(PWM_TOP_PWM4_DUTY_CYCLE_HIGH_PWM4_HIGH_MASK,
		    PWM_TOP_PWM4_DUTY_CYCLE_HIGH_PWM4_HIGH_SHIFT,
		    PWM_TOP_PWM4_DUTY_CYCLE_HIGH_OFFSET),
	[5] = PWM_REG_DEF(PWM_TOP_PWM5_DUTY_CYCLE_HIGH_PWM5_HIGH_MASK,
		    PWM_TOP_PWM5_DUTY_CYCLE_HIGH_PWM5_HIGH_SHIFT,
		    PWM_TOP_PWM5_DUTY_CYCLE_HIGH_OFFSET),
};

static const struct pwm_reg_def pwm_period_cnt_info[KONA_PWM_CHANNEL_CNT] = {
	[0] = PWM_REG_DEF(PWM_TOP_PWM0_PERIOD_COUNT_PWM0_CNT_MASK,
		    PWM_TOP_PWM0_PERIOD_COUNT_PWM0_CNT_SHIFT,
		    PWM_TOP_PWM0_PERIOD_COUNT_OFFSET),
	[1] = PWM_REG_DEF(PWM_TOP_PWM1_PERIOD_COUNT_PWM1_CNT_MASK,
		    PWM_TOP_PWM1_PERIOD_COUNT_PWM1_CNT_SHIFT,
		    PWM_TOP_PWM1_PERIOD_COUNT_OFFSET),
	[2] = PWM_REG_DEF(PWM_TOP_PWM2_PERIOD_COUNT_PWM2_CNT_MASK,
		    PWM_TOP_PWM2_PERIOD_COUNT_PWM2_CNT_SHIFT,
		    PWM_TOP_PWM2_PERIOD_COUNT_OFFSET),
	[3] = PWM_REG_DEF(PWM_TOP_PWM3_PERIOD_COUNT_PWM3_CNT_MASK,
		    PWM_TOP_PWM3_PERIOD_COUNT_PWM3_CNT_SHIFT,
		    PWM_TOP_PWM3_PERIOD_COUNT_OFFSET),
	[4] = PWM_REG_DEF(PWM_TOP_PWM4_PERIOD_COUNT_PWM4_CNT_MASK,
		    PWM_TOP_PWM4_PERIOD_COUNT_PWM4_CNT_SHIFT,
		    PWM_TOP_PWM4_PERIOD_COUNT_OFFSET),
	[5] = PWM_REG_DEF(PWM_TOP_PWM5_PERIOD_COUNT_PWM5_CNT_MASK,
		    PWM_TOP_PWM5_PERIOD_COUNT_PWM5_CNT_SHIFT,
		    PWM_TOP_PWM5_PERIOD_COUNT_OFFSET),
};


struct kona_pwmc {
	struct pwm_chip chip;
	struct device *dev;
	struct pwm_ops ops;
	void __iomem *iobase;
	struct clk *clk;
	unsigned long tick_hz;
	unsigned int duty_ns_array[KONA_PWM_CHANNEL_CNT];
};

void pwm_set_drvdata(struct pwm_chip *p, void *data)
{
	dev_set_drvdata(p->dev, data);
}

void *pwm_get_drvdata(const struct pwm_chip *p)
{
	return dev_get_drvdata(p->dev);
}

static void kona_pwmc_clear_set_bit(const struct kona_pwmc *ap,
				    unsigned int offset, unsigned int shift,
				    unsigned char en_dis)
{
	unsigned long val;

	val = readl(ap->iobase + offset);
	/*Clear bit */
	clear_bit(shift, &val);
	if (en_dis == 1)
		set_bit(shift, &val);
	writel(val, (ap->iobase + offset));
}

static void kona_pwmc_set_smooth(const struct kona_pwmc *ap, unsigned int chan,
				 unsigned char enable)
{
	kona_pwmc_clear_set_bit(ap, PWM_TOP_PWM_CONTROL_OFFSET,
		(chan + PWM_TOP_PWM_CONTROL_SMOOTH_TYPE_SHIFT), enable);
}

static void kona_pwmc_stop(const struct kona_pwmc *ap, unsigned int chan)
{
	kona_pwmc_clear_set_bit(ap, PWM_TOP_PWM_CONTROL_OFFSET,
		(chan + PWM_TOP_PWM_CONTROL_PWMOUT_ENABLE_SHIFT), 0);
}

static void kona_pwmc_start(const struct kona_pwmc *ap, unsigned int chan)
{
	/*From rdb : Minimum of 400ns is needed between setting
	PWMOUT_ENABLE to 0, changing the configuration and setting
	it back to 1 */
	ndelay(400);

	kona_pwmc_clear_set_bit(ap, PWM_TOP_PWM_CONTROL_OFFSET,
		(chan + PWM_TOP_PWM_CONTROL_PWMOUT_ENABLE_SHIFT), 1);

	/*From rdb : If user didn't hold PWMOUT_ENABLE=1 for longer than 400ns,
	PWM internal logic will discard the new PWM setting.
	Also while shutting down PWM, It takes 400ns from STEP3 to turn off the
	LCD backlight, and user should guarantee that the PWM clock will not be
	disabled in less than 400ns after STEP3*/
	ndelay(400);
}

static void kona_pwmc_config_polarity(struct kona_pwmc *ap, unsigned int chan,
				      unsigned char polarity)
{
	kona_pwmc_clear_set_bit(ap, PWM_TOP_PWM_CONTROL_OFFSET,
		(chan + PWM_TOP_PWM_CONTROL_PWMOUT_POLARITY_SHIFT), polarity);
}

static void kona_pwmc_set_field(const struct kona_pwmc *ap, unsigned int offset,
				unsigned int mask, unsigned int shift,
				unsigned int wval)
{
	unsigned int val = readl(ap->iobase + offset);
	val = (val & ~mask) | (wval << shift);
	writel(val, (ap->iobase + offset));
}

static void kona_pwmc_config_period_and_duty_ticks(struct kona_pwmc *ap,
			unsigned int chan, int period_ticks, int duty_ticks)
{
	unsigned int pcnt = 0, dcnt = 0;
	unsigned char pre_scaler = 0;

	/*pcnt = (clk * period_in_seconds) / pre_scalar) */
	pre_scaler = period_ticks / 1000000;
	if (pre_scaler > PWM_PRESCALER_MAX)
		pre_scaler = PWM_PRESCALER_MAX;

	/*program prescaler*/
	kona_pwmc_set_field(ap, pwm_chan_pre_scaler_info[chan].offset,
			    pwm_chan_pre_scaler_info[chan].mask,
			    pwm_chan_pre_scaler_info[chan].shift, pre_scaler);

	/*Calculate period cnt and duty_high cnt*/
	pcnt = period_ticks / (pre_scaler + 1);
	dcnt = duty_ticks / (pre_scaler + 1);

	/* program period cnt register*/
	kona_pwmc_set_field(ap, pwm_period_cnt_info[chan].offset,
			    pwm_period_cnt_info[chan].mask,
			    pwm_period_cnt_info[chan].shift, pcnt);

	/* program duty cycle high*/
	kona_pwmc_set_field(ap, pwm_duty_cycle_info[chan].offset,
			pwm_duty_cycle_info[chan].mask,
			pwm_duty_cycle_info[chan].shift, dcnt);

}

static unsigned long pwm_ns_to_ticks(struct kona_pwmc *ap, unsigned long nsecs)
{
	unsigned long long ticks;

	ticks = nsecs;
	ticks *= ap->tick_hz;
	do_div(ticks, 1000000000);
	return ticks;
}

static int kona_pwmc_config(struct pwm_chip *chip, struct pwm_device *p,
				int duty_ns, int period_ns)
{
	struct kona_pwmc *ap = pwm_get_drvdata(chip);
	unsigned int chan = p->pwm;
	int ret = 0, smooth = 0;
	int period_ticks = pwm_ns_to_ticks(ap, period_ns);
	int duty_ticks = pwm_ns_to_ticks(ap, duty_ns);

	pr_debug("bcm_kona_pwmc config, chan is %d\n", chan);

	mutex_lock(&pwm_lock);
	/*Check if there is a change in brightness*/
	if (ap->duty_ns_array[chan] == duty_ns) {
		mutex_unlock(&pwm_lock);
		return 0;
	}

	if (duty_ns) {
		/*Brightness is non-zero*/
		if (ap->duty_ns_array[chan] == 0) {
			/*Clock is not enabled */
			/*Turn on clock before writing to registers*/
#ifdef CONFIG_HAVE_CLK
			ret = clk_enable(ap->clk);
			if (ret) {
				pr_err("%s: clock enable failed: Err %d\n",
						__func__, ret);
				mutex_unlock(&pwm_lock);
				return ret;
			}
#endif
		}
		smooth = 1;
	} else /* brightness is zero */
		smooth = 0;

	kona_pwmc_set_smooth(ap, chan, smooth);
	kona_pwmc_stop(ap, chan);
	kona_pwmc_config_period_and_duty_ticks(ap, chan,
			period_ticks, duty_ticks);
	kona_pwmc_start(ap, chan);
	/*Update the current duty_ns value for the pwm channel*/
	ap->duty_ns_array[chan] = duty_ns;

	/*If brightness is zero, disable clock to save power*/
	if (duty_ns == 0) {
#ifdef CONFIG_HAVE_CLK
		clk_disable(ap->clk);
#endif
	}

	mutex_unlock(&pwm_lock);
	return ret;
}

static int kona_pwmc_enable(struct pwm_chip *chip, struct pwm_device *p)
{
	return 0;
}

static void kona_pwmc_disable(struct pwm_chip *chip, struct pwm_device *p)
{
}

static const struct pwm_ops kona_pwm_ops = {
	.config = kona_pwmc_config,
	.owner = THIS_MODULE,
	.enable = kona_pwmc_enable,
	.disable = kona_pwmc_disable,
};

/* Kona PWM driver probe routine */
static int kona_pwmc_probe(struct platform_device *pdev)
{
	struct kona_pwmc *ap;
	struct resource *r;
	unsigned int chan = 0;
	int ret = 0;

	pr_debug("bcm_kona_pwmc probe\n");
	ap = devm_kzalloc(&pdev->dev, sizeof(*ap), GFP_KERNEL);
	if (ap == NULL) {
		ret = -ENOMEM;
		dev_err(&pdev->dev, "pwm probe fn: Failed to allocate memory : Err %d\n",
			ret);
		goto err;
	}

	ap->dev = &pdev->dev;
	/*Set the driver data to the platform */
	platform_set_drvdata(pdev, ap);

#ifdef CONFIG_HAVE_CLK
	ap->clk = clk_get(&pdev->dev, "pwm_clk");
	if (IS_ERR(ap->clk)) {
		ret = PTR_ERR(ap->clk);
		dev_err(&pdev->dev, "pwm probe fn: Clock get failed : Err %d\n",
			ret);
		goto err;
	}

	/* clk is off by default by clk driver */
	ap->tick_hz = clk_get_rate(ap->clk);

	/*Enable clock before writing to PWM registers */
	ret = clk_prepare_enable(ap->clk);
	if (ret < 0) {
		dev_err(&pdev->dev, "clk_prepare_enable failed: %d\n", ret);
		goto err_put_clk;
	}
#endif

	r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!r) {
		ret = -ENODEV;
		dev_err(&pdev->dev, "pwm probe fn: Failed to get resource : Err %d\n",
			ret);
		goto err_put_clk;
	}

	ap->iobase = devm_request_and_ioremap(&pdev->dev, r);
	if (!ap->iobase) {
		ret = -ENODEV;
		dev_err(&pdev->dev, "pwm probe fn: ioremap failed : Err %d\n",
			ret);
		goto err_put_clk;
	}

	/* Init all channels to a know state and leave clock disabled. */
	for (chan = 0; chan < KONA_PWM_CHANNEL_CNT; chan++) {

		kona_pwmc_set_smooth(ap, chan, 1);
		kona_pwmc_stop(ap, chan);
		kona_pwmc_config_polarity(ap, chan, DEFAULT_POLARITY);
		kona_pwmc_config_period_and_duty_ticks(ap, chan,
				DEFAULT_PERIOD, DEFAULT_DUTY);
		kona_pwmc_start(ap, chan);
		ap->duty_ns_array[chan] = DEFAULT_DUTY;
	}

	ap->chip.dev = &pdev->dev;
	pwm_set_drvdata(&ap->chip, ap);
	ap->chip.ops = &kona_pwm_ops;
	ap->chip.base = -1;
	ap->chip.npwm = KONA_PWM_CHANNEL_CNT;
	ret = pwmchip_add(&ap->chip);
	if (ret < 0) {
		dev_err(&pdev->dev, "pwmchip_add() failed: Err %d\n",
				ret);
		goto err_put_clk;
	}

#ifdef CONFIG_HAVE_CLK
	/*Leave the clocks disabled. Config fn should enable it if required */
	clk_disable(ap->clk);
#endif
	return 0;

err_put_clk:
#ifdef CONFIG_HAVE_CLK
	clk_disable_unprepare(ap->clk);
	clk_put(ap->clk);
#endif
err:
	platform_set_drvdata(pdev, NULL);
	return ret;
}

static int kona_pwmc_remove(struct platform_device *pdev)
{
	struct kona_pwmc *ap = platform_get_drvdata(pdev);
	int ret;

	ret = pwmchip_remove(&ap->chip);
	if (ret < 0) {
		dev_err(&pdev->dev, "pwmchip_remove() failed: Err %d\n",
				ret);
		return ret;
	}
#ifdef CONFIG_HAVE_CLK
	clk_unprepare(ap->clk);
	clk_put(ap->clk);
#endif
	return 0;
}

#ifdef CONFIG_PM
static int kona_pwmc_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct kona_pwmc *ap = platform_get_drvdata(pdev);
	unsigned int chan;

	for (chan = 0; chan < KONA_PWM_CHANNEL_CNT; chan++) {
		if (ap->duty_ns_array[chan] > 0) {
			/*disable clock*/
#ifdef CONFIG_HAVE_CLK
			clk_disable(ap->clk);
#endif
		}
	}
	return 0;
}

static int kona_pwmc_resume(struct platform_device *pdev)
{
	struct kona_pwmc *ap = platform_get_drvdata(pdev);
	unsigned int chan;
	int ret = 0;

	for (chan = 0; chan < KONA_PWM_CHANNEL_CNT; chan++) {
		if (ap->duty_ns_array[chan] > 0) {
			/*enable clock*/
#ifdef CONFIG_HAVE_CLK
			ret = clk_enable(ap->clk);
			if (ret) {
				pr_err("%s: clock enable failed: Err %d\n",
						__func__, ret);
				return ret;
			}
#endif
		}
	}
	return 0;
}

#else
#define kona_pwmc_suspend	NULL
#define kona_pwmc_resume	NULL
#endif

#ifdef CONFIG_OF
static const struct of_device_id bcm_kona_pwmc_dt[] = {
	{.compatible = "bcm,pwmc"},
	{},
};
MODULE_DEVICE_TABLE(of, bcm_kona_pwmc_dt);
#endif

static struct platform_driver kona_pwmc_driver = {

	.driver = {
		   .name = "kona_pwmc",
		   .owner = THIS_MODULE,
		   .of_match_table = of_match_ptr(bcm_kona_pwmc_dt),
		   },

	.probe = kona_pwmc_probe,
	.remove = kona_pwmc_remove,
	.suspend = kona_pwmc_suspend,
	.resume = kona_pwmc_resume,
};

module_platform_driver(kona_pwmc_driver);

MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("Driver for KONA PWMC");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");

