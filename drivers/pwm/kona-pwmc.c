/*****************************************************************************
* Copyright 2006 - 2011 Broadcom Corporation.  All rights reserved.
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
 *    - SMP:
 *    - GPIO:
 *    - MMU:          Fully supported.    Platform model with ioremap used (mostly).
 *    - Dynamic /dev:
 *    - Suspend:
 *    - Clocks:       Not done.           Awaiting clock framework to be completed.
 *    - Power:        Not done.
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
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/completion.h>
#include <linux/pwm.h>
#include <mach/rdb/brcm_rdb_pwm_top.h>

#define KONA_PWM_CHANNEL_CNT 6
#define PWM_PRESCALER_MAX    7

struct kona_pwmc {
	struct pwm_device *p[KONA_PWM_CHANNEL_CNT];
	struct pwm_device_ops ops;
	void __iomem *iobase;
	struct clk *clk;
	int	pwm_started;
	struct pwm_chip chip;
};

struct pwm_control {
	u32 smooth_type_mask;
	u32 smooth_type_shift;
	u32 pwmout_type_mask;
	u32 pwmout_type_shift;
	u32 pwmout_polarity_mask;
	u32 pwmout_polarity_shift;
	u32 pwmout_enable_mask;
	u32 pwmout_enable_shift;
	u32 offset;
};

struct pwm_reg_def {
	u32 mask;
	u32 shift;
	u32 offset;
};

#define PWM_CONTROL_PROP(chan, stm, sts, ptm, pts, pm, ps, em, es, addr) \
    [chan] = {  \
        .smooth_type_mask                       =       stm, \
        .smooth_type_shift                      =       sts, \
        .pwmout_type_mask                       =       ptm, \
        .pwmout_type_shift                      =       pts, \
        .pwmout_polarity_mask		=       pm,  \
        .pwmout_polarity_shift          =       ps,  \
        .pwmout_enable_mask             =       em,  \
        .pwmout_enable_shift            =       es,  \
        .offset                     =   addr \
    }

#define CHAN_SHIFT(shift,chan) (shift+chan)
#define CHAN_MASK(mask,shift,chan) (mask & ( 1 << CHAN_SHIFT(shift,chan) ) )
#define PWM_CONTROL_SET(chan) \
    PWM_CONTROL_PROP( chan , \
    CHAN_MASK(PWM_TOP_PWM_CONTROL_SMOOTH_TYPE_MASK,PWM_TOP_PWM_CONTROL_SMOOTH_TYPE_SHIFT,chan), \
    CHAN_SHIFT(PWM_TOP_PWM_CONTROL_SMOOTH_TYPE_SHIFT,chan),  \
    CHAN_MASK(PWM_TOP_PWM_CONTROL_PWMOUT_TYPE_MASK,PWM_TOP_PWM_CONTROL_PWMOUT_TYPE_SHIFT,chan), \
    CHAN_SHIFT(PWM_TOP_PWM_CONTROL_PWMOUT_TYPE_SHIFT,chan),  \
    CHAN_MASK(PWM_TOP_PWM_CONTROL_PWMOUT_POLARITY_MASK,PWM_TOP_PWM_CONTROL_PWMOUT_POLARITY_SHIFT,chan), \
    CHAN_SHIFT(PWM_TOP_PWM_CONTROL_PWMOUT_POLARITY_SHIFT,chan),  \
    CHAN_MASK(PWM_TOP_PWM_CONTROL_PWMOUT_ENABLE_MASK,PWM_TOP_PWM_CONTROL_PWMOUT_ENABLE_SHIFT,chan), \
    CHAN_SHIFT(PWM_TOP_PWM_CONTROL_PWMOUT_ENABLE_SHIFT,chan),  \
    PWM_TOP_PWM_CONTROL_OFFSET \
)

#define PWM_REG_DEF(c, m, s, a) \
    [c] = {     \
        .mask           =       m,  \
        .shift          =       s, \
        .offset         =       a \
    }

static const struct pwm_control pwm_chan_ctrl_info[KONA_PWM_CHANNEL_CNT] = {
	PWM_CONTROL_SET(0),
	PWM_CONTROL_SET(1),
	PWM_CONTROL_SET(2),
	PWM_CONTROL_SET(3),
	PWM_CONTROL_SET(4),
	PWM_CONTROL_SET(5)
};

static const struct pwm_reg_def pwm_chan_pre_scaler_info[KONA_PWM_CHANNEL_CNT] = {
	PWM_REG_DEF(0, PWM_TOP_PRESCALE_CONTROL_PWM0_PRESCALE_MASK,
		    PWM_TOP_PRESCALE_CONTROL_PWM0_PRESCALE_SHIFT,
		    PWM_TOP_PRESCALE_CONTROL_OFFSET),
	PWM_REG_DEF(1, PWM_TOP_PRESCALE_CONTROL_PWM1_PRESCALE_MASK,
		    PWM_TOP_PRESCALE_CONTROL_PWM1_PRESCALE_SHIFT,
		    PWM_TOP_PRESCALE_CONTROL_OFFSET),
	PWM_REG_DEF(2, PWM_TOP_PRESCALE_CONTROL_PWM2_PRESCALE_MASK,
		    PWM_TOP_PRESCALE_CONTROL_PWM2_PRESCALE_SHIFT,
		    PWM_TOP_PRESCALE_CONTROL_OFFSET),
	PWM_REG_DEF(3, PWM_TOP_PRESCALE_CONTROL_PWM3_PRESCALE_MASK,
		    PWM_TOP_PRESCALE_CONTROL_PWM3_PRESCALE_SHIFT,
		    PWM_TOP_PRESCALE_CONTROL_OFFSET),
	PWM_REG_DEF(4, PWM_TOP_PRESCALE_CONTROL_PWM4_PRESCALE_MASK,
		    PWM_TOP_PRESCALE_CONTROL_PWM4_PRESCALE_SHIFT,
		    PWM_TOP_PRESCALE_CONTROL_OFFSET),
	PWM_REG_DEF(5, PWM_TOP_PRESCALE_CONTROL_PWM5_PRESCALE_MASK,
		    PWM_TOP_PRESCALE_CONTROL_PWM5_PRESCALE_SHIFT,
		    PWM_TOP_PRESCALE_CONTROL_OFFSET),
};

static const struct pwm_reg_def pwm_chan_period_cnt_info[KONA_PWM_CHANNEL_CNT] = {
	PWM_REG_DEF(0, PWM_TOP_PWM0_PERIOD_COUNT_PWM0_CNT_MASK,
		    PWM_TOP_PWM0_PERIOD_COUNT_PWM0_CNT_SHIFT,
		    PWM_TOP_PWM0_PERIOD_COUNT_OFFSET),
	PWM_REG_DEF(1, PWM_TOP_PWM1_PERIOD_COUNT_PWM1_CNT_MASK,
		    PWM_TOP_PWM1_PERIOD_COUNT_PWM1_CNT_SHIFT,
		    PWM_TOP_PWM1_PERIOD_COUNT_OFFSET),
	PWM_REG_DEF(2, PWM_TOP_PWM2_PERIOD_COUNT_PWM2_CNT_MASK,
		    PWM_TOP_PWM2_PERIOD_COUNT_PWM2_CNT_SHIFT,
		    PWM_TOP_PWM2_PERIOD_COUNT_OFFSET),
	PWM_REG_DEF(3, PWM_TOP_PWM3_PERIOD_COUNT_PWM3_CNT_MASK,
		    PWM_TOP_PWM3_PERIOD_COUNT_PWM3_CNT_SHIFT,
		    PWM_TOP_PWM3_PERIOD_COUNT_OFFSET),
	PWM_REG_DEF(4, PWM_TOP_PWM4_PERIOD_COUNT_PWM4_CNT_MASK,
		    PWM_TOP_PWM4_PERIOD_COUNT_PWM4_CNT_SHIFT,
		    PWM_TOP_PWM4_PERIOD_COUNT_OFFSET),
	PWM_REG_DEF(5, PWM_TOP_PWM5_PERIOD_COUNT_PWM5_CNT_MASK,
		    PWM_TOP_PWM5_PERIOD_COUNT_PWM5_CNT_SHIFT,
		    PWM_TOP_PWM5_PERIOD_COUNT_OFFSET),
};

static const struct pwm_reg_def pwm_chan_duty_cycle_info[KONA_PWM_CHANNEL_CNT] = {
	PWM_REG_DEF(0, PWM_TOP_PWM0_DUTY_CYCLE_HIGH_PWM0_HIGH_MASK,
		    PWM_TOP_PWM0_DUTY_CYCLE_HIGH_PWM0_HIGH_SHIFT,
		    PWM_TOP_PWM0_DUTY_CYCLE_HIGH_OFFSET),
	PWM_REG_DEF(1, PWM_TOP_PWM1_DUTY_CYCLE_HIGH_PWM1_HIGH_MASK,
		    PWM_TOP_PWM1_DUTY_CYCLE_HIGH_PWM1_HIGH_SHIFT,
		    PWM_TOP_PWM1_DUTY_CYCLE_HIGH_OFFSET),
	PWM_REG_DEF(2, PWM_TOP_PWM2_DUTY_CYCLE_HIGH_PWM2_HIGH_MASK,
		    PWM_TOP_PWM2_DUTY_CYCLE_HIGH_PWM2_HIGH_SHIFT,
		    PWM_TOP_PWM2_DUTY_CYCLE_HIGH_OFFSET),
	PWM_REG_DEF(3, PWM_TOP_PWM3_DUTY_CYCLE_HIGH_PWM3_HIGH_MASK,
		    PWM_TOP_PWM3_DUTY_CYCLE_HIGH_PWM3_HIGH_SHIFT,
		    PWM_TOP_PWM3_DUTY_CYCLE_HIGH_OFFSET),
	PWM_REG_DEF(4, PWM_TOP_PWM4_DUTY_CYCLE_HIGH_PWM4_HIGH_MASK,
		    PWM_TOP_PWM4_DUTY_CYCLE_HIGH_PWM4_HIGH_SHIFT,
		    PWM_TOP_PWM4_DUTY_CYCLE_HIGH_OFFSET),
	PWM_REG_DEF(5, PWM_TOP_PWM5_DUTY_CYCLE_HIGH_PWM5_HIGH_MASK,
		    PWM_TOP_PWM5_DUTY_CYCLE_HIGH_PWM5_HIGH_SHIFT,
		    PWM_TOP_PWM5_DUTY_CYCLE_HIGH_OFFSET),
};

static int kona_get_chan(const struct kona_pwmc *ap, const struct pwm_device *p)
{
	int chan;
	for (chan = 0; chan < KONA_PWM_CHANNEL_CNT; chan++)
		if (p == ap->p[chan])
			return chan;
	BUG();
	return 0;
}

static void kona_pwmc_clear_set_bit(const struct kona_pwmc *ap,
				    unsigned int offset, unsigned int shift,
				    unsigned char en_dis)
{
	unsigned long val = readl(ap->iobase + offset);

	// Clear bit.
	clear_bit(shift, &val);

	if (en_dis == 1)
		set_bit(shift, &val);

	writel(val, (ap->iobase + offset));
}

static void kona_pwmc_set_field(const struct kona_pwmc *ap, unsigned int offset,
				unsigned int mask, unsigned int shift,
				unsigned int wval)
{
	unsigned int val = readl(ap->iobase + offset);
	val = (val & ~mask) | (wval << shift);
	writel(val, (ap->iobase + offset));
}

static void kona_pwmc_get_field(const struct kona_pwmc *ap, unsigned int offset,
				unsigned int mask, unsigned int shift,
				unsigned int *val)
{
	*val = readl(ap->iobase + offset);
	*val = (*val & mask) >> shift;
}

static void kona_pwmc_stop(const struct kona_pwmc *ap, int chan)
{
	kona_pwmc_clear_set_bit(ap, pwm_chan_ctrl_info[chan].offset,
				pwm_chan_ctrl_info[chan].pwmout_enable_shift,
				0);
}

static void kona_pwmc_start(const struct kona_pwmc *ap, int chan)
{
	kona_pwmc_clear_set_bit(ap, pwm_chan_ctrl_info[chan].offset,
				pwm_chan_ctrl_info[chan].pwmout_enable_shift,
				1);
}

static void kona_pwmc_config_polarity(struct kona_pwmc *ap, int chan,
				      struct pwm_config *c)
{
	struct pwm_device *p = ap->p[chan];
	clk_enable(ap->clk);

	if (c->polarity)
		kona_pwmc_clear_set_bit(ap, pwm_chan_ctrl_info[chan].offset,
					pwm_chan_ctrl_info[chan].
					pwmout_polarity_shift, 1);
	else
		kona_pwmc_clear_set_bit(ap, pwm_chan_ctrl_info[chan].offset,
					pwm_chan_ctrl_info[chan].
					pwmout_polarity_shift, 0);
	p->polarity = c->polarity ? 1 : 0;
	clk_disable(ap->clk);
}

static void kona_pwmc_config_duty_ticks(struct kona_pwmc *ap, int chan,
					struct pwm_config *c)
{
	struct pwm_device *p = ap->p[chan];
	unsigned int pre_scaler = 0;
	unsigned int duty_cnt = 0;

	clk_enable(ap->clk);
	kona_pwmc_get_field(ap, pwm_chan_pre_scaler_info[chan].offset,
			    pwm_chan_pre_scaler_info[chan].mask,
			    pwm_chan_pre_scaler_info[chan].shift, &pre_scaler);

	// Read prescaler value from register.
	duty_cnt = c->duty_ticks / (pre_scaler + 1);
	/* disable channel */
	kona_pwmc_stop(ap, chan);

	// program duty cycle.
	kona_pwmc_set_field(ap, pwm_chan_duty_cycle_info[chan].offset,
			    pwm_chan_duty_cycle_info[chan].mask,
			    pwm_chan_duty_cycle_info[chan].shift, duty_cnt);

	/* Workaround suggested by ASIC team */
	if (pre_scaler >= 2)
		ndelay(310);

	// enable channel.
	kona_pwmc_start(ap, chan);

	p->duty_ticks = c->duty_ticks;
	clk_disable(ap->clk);
}

static int kona_pwmc_config_period_ticks(struct kona_pwmc *ap, int chan,
					 struct pwm_config *c)
{
	unsigned int pcnt;
	unsigned char pre_scaler = 0;
	struct pwm_device *p = ap->p[chan];

	clk_enable(ap->clk);
	// pcnt = ( 26 * 1000000 * period_ns ) / (pre_scaler * 1000000000 )
	// Calculate period cnt.
	pre_scaler = c->period_ticks / 0xFFFFFF;
	if (pre_scaler > PWM_PRESCALER_MAX)
		pre_scaler = PWM_PRESCALER_MAX;

	pcnt = c->period_ticks / (pre_scaler + 1);

	// program prescaler
	kona_pwmc_set_field(ap, pwm_chan_pre_scaler_info[chan].offset,
			    pwm_chan_pre_scaler_info[chan].mask,
			    pwm_chan_pre_scaler_info[chan].shift, pre_scaler);

	// program period count.
	kona_pwmc_set_field(ap, pwm_chan_period_cnt_info[chan].offset,
			    pwm_chan_period_cnt_info[chan].mask,
			    pwm_chan_period_cnt_info[chan].shift, pcnt);

	// disable channel
	kona_pwmc_stop(ap, chan);

	/* Workaround suggested by ASIC team */
	if (pre_scaler >= 2)
		ndelay(310);

	// enable channel.
	kona_pwmc_start(ap, chan);

	p->period_ticks = c->period_ticks;
	clk_disable(ap->clk);

	return 0;
}

static int kona_pwmc_config(struct pwm_device *p, struct pwm_config *c)
{
	struct kona_pwmc *ap = pwm_get_drvdata(p);
	int chan = kona_get_chan(ap, p);
	int ret = 0;

	if (test_bit(PWM_CONFIG_POLARITY, &c->config_mask))
		kona_pwmc_config_polarity(ap, chan, c);

	if (test_bit(PWM_CONFIG_DUTY_TICKS, &c->config_mask))
		kona_pwmc_config_duty_ticks(ap, chan, c);

	if (test_bit(PWM_CONFIG_PERIOD_TICKS, &c->config_mask)) {
		ret = kona_pwmc_config_period_ticks(ap, chan, c);
		if (ret)
			goto out;

		if (!test_bit(PWM_CONFIG_DUTY_TICKS, &c->config_mask)) {
			struct pwm_config d = {
				.config_mask = PWM_CONFIG_DUTY_TICKS,
				.duty_ticks = p->duty_ticks,
			};
			kona_pwmc_config_duty_ticks(ap, chan, &d);
		}
	}

	if (!ap->pwm_started && test_bit(PWM_CONFIG_START, &c->config_mask)) {
		/* Restore duty ticks cater for STOP case. */
		struct pwm_config d = {
			.config_mask = PWM_CONFIG_DUTY_TICKS,
			.duty_ticks = p->duty_ticks,
		};

		ap->pwm_started = 1;
		usleep_range(3000, 7000);
		clk_enable(ap->clk);
		kona_pwmc_clear_set_bit(ap, pwm_chan_ctrl_info[chan].offset,
					pwm_chan_ctrl_info[chan].
					smooth_type_shift, 1);
		kona_pwmc_config_duty_ticks(ap, chan, &d);
		kona_pwmc_start(ap, chan);
	}

	if (ap->pwm_started && test_bit(PWM_CONFIG_STOP, &c->config_mask)) {
		struct pwm_config d = {
			.config_mask = PWM_CONFIG_DUTY_TICKS,
			.duty_ticks = 0,
		};

		ap->pwm_started = 0;
		kona_pwmc_config_duty_ticks(ap, chan, &d);
		kona_pwmc_clear_set_bit(ap, pwm_chan_ctrl_info[chan].offset,
					pwm_chan_ctrl_info[chan].
					smooth_type_shift, 0);
		/* turn-off the PWM clock i.e. enabled during pwm_start */
		ndelay(410);
		clk_disable(ap->clk);
	}

out:
	return ret;
}

static int kona_pwmc_request(struct pwm_device *p)
{
	struct kona_pwmc *ap = pwm_get_drvdata(p);
	int chan = kona_get_chan(ap, p);
	clk_enable(ap->clk);
	p->tick_hz = clk_get_rate(ap->clk);
	kona_pwmc_stop(ap, chan);
	clk_disable(ap->clk);
	return 0;
}

static void kona_pwmc_release(struct pwm_device *p)
{

}


static const struct pwm_device_ops kona_pwm_ops = {
	.request = kona_pwmc_request,
	.release = kona_pwmc_release,
	.config = kona_pwmc_config,
	.owner = THIS_MODULE,
};

static int kona_pwmc_probe(struct platform_device *pdev)
{
	struct kona_pwmc *ap;
	struct resource *r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	int chan;
	int ret = 0;

	ap = kzalloc(sizeof(*ap), GFP_KERNEL);
	if (!ap) {
		ret = -ENOMEM;
		goto err_kona_pwmc_alloc;
	}

	platform_set_drvdata(pdev, ap);
	ap->clk = clk_get(&pdev->dev, "pwm_clk");
	if (IS_ERR(ap->clk)) {
		ret = -ENODEV;
		goto err_clk_get;
	}

	ap->iobase = ioremap_nocache(r->start, resource_size(r));
	if (!ap->iobase) {
		ret = -ENODEV;
		goto err_ioremap;
	}

	ap->chip.dev = &pdev->dev;
	ap->chip.ops = &kona_pwm_ops;
	ap->chip.base = -1;
	ap>chip.npwm = KONA_PWM_CHANNEL_CNT;

	for (chan = 0; chan < KONA_PWM_CHANNEL_CNT; chan++) {
		ap->p[chan] = pwm_register(&kona_pwm_ops, &pdev->dev, "%s:%d",
					   "kona_pwmc", chan);
		if (IS_ERR_OR_NULL(ap->p[chan]))
			goto err_pwm_register;
		pwm_set_drvdata(ap->p[chan], ap);
		kona_pwmc_clear_set_bit(ap, pwm_chan_ctrl_info[chan].offset,
					pwm_chan_ctrl_info[chan].
					smooth_type_shift, 1);
	}

	printk(KERN_INFO "PWM: driver initialized properly");

	return 0;

err_pwm_register:
	while (--chan > 0)
		pwm_unregister(ap->p[chan]);

	iounmap(ap->iobase);
err_ioremap:
	clk_put(ap->clk);
err_clk_get:
	platform_set_drvdata(pdev, NULL);
	kfree(ap);
err_kona_pwmc_alloc:
	printk(KERN_ERR "%s: error, returning %d\n", __func__, ret);
	return ret;
}

static int kona_pwmc_remove(struct platform_device *pdev)
{
	struct kona_pwmc *ap = platform_get_drvdata(pdev);
	int chan;

	for (chan = 0; chan < KONA_PWM_CHANNEL_CNT; chan++)
		pwm_unregister(ap->p[chan]);

	clk_put(ap->clk);
	iounmap(ap->iobase);

	kfree(ap);

	return 0;
}

#ifdef CONFIG_PM
static int kona_pwmc_suspend(struct platform_device *pdev, pm_message_t state)
{
	/* TODO: add more resume support in the future */
	return 0;
}

static int kona_pwmc_resume(struct platform_device *pdev)
{
	/* TODO: add more resume support in the future */
	return 0;
}
#else
#define kona_pwmc_suspend    NULL
#define kona_pwmc_resume     NULL
#endif

static const struct of_device_id kona_pwmc_dt_ids[] = {
	{ .compatible = "bcm,pwmc", },
	{}
};

static struct platform_driver kona_pwmc_driver = {
	.driver = {
		.name = "kona_pwmc",
		.owner = THIS_MODULE,
		.of_match_table = kona_pwmc_dt_ids,
	},
	.probe = kona_pwmc_probe,
	.remove = kona_pwmc_remove,
	.suspend = kona_pwmc_suspend,
	.resume = kona_pwmc_resume,
};

static const char gBanner[] =
    KERN_INFO "Broadcom Pulse Width Modulator Driver: 1.00\n";
static int __init kona_pwmc_init(void)
{
	printk(gBanner);
	return platform_driver_register(&kona_pwmc_driver);
}

static void __exit kona_pwmc_exit(void)
{
	platform_driver_unregister(&kona_pwmc_driver);
}

module_init(kona_pwmc_init);
module_exit(kona_pwmc_exit);

MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("Driver for KONA PWMC");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");
