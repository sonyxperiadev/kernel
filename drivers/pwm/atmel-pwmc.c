/*
 * Atmel PWMC peripheral driver
 *
 * Copyright (C) 2011 Bill Gatliff <bgat@billgatliff.com>
 * Copyright (C) 2007 David Brownell
 *
 * This program is free software; you may redistribute and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
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
#include <linux/pwm/pwm.h>

enum {
	/* registers common to the PWMC peripheral */
	PWMC_MR = 0,
	PWMC_ENA = 4,
	PWMC_DIS = 8,
	PWMC_SR = 0xc,
	PWMC_IER = 0x10,
	PWMC_IDR = 0x14,
	PWMC_IMR = 0x18,
	PWMC_ISR = 0x1c,

	/* registers per each PWMC channel */
	PWMC_CMR = 0,
	PWMC_CDTY = 4,
	PWMC_CPRD = 8,
	PWMC_CCNT = 0xc,
	PWMC_CUPD = 0x10,

	/* how to find each channel */
	PWMC_CHAN_BASE = 0x200,
	PWMC_CHAN_STRIDE = 0x20,

	/* CMR bits of interest */
	PWMC_CMR_CPD = 10,
	PWMC_CMR_CPOL = 9,
	PWMC_CMR_CALG = 8,
	PWMC_CMR_CPRE_MASK = 0xf,
};

/* TODO: NCHAN==4 only for certain AT91-ish parts! */
#define NCHAN 4
struct atmel_pwmc {
	struct pwm_device *p[NCHAN];
	struct pwm_device_ops ops;
	spinlock_t lock;
	struct completion complete;
	void __iomem *iobase;
	struct clk *clk;
	u32 ccnt_mask;
};

/* TODO: debugfs attributes for peripheral register values */

static inline void pwmc_writel(const struct atmel_pwmc *p, unsigned offset, u32 val)
{
	__raw_writel(val, p->iobase + offset);
}

static inline u32 pwmc_readl(const struct atmel_pwmc *p, unsigned offset)
{
	return __raw_readl(p->iobase + offset);
}

static int __to_chan(const struct atmel_pwmc *ap, const struct pwm_device *p)
{
	int chan;
	for (chan = 0; chan < NCHAN; chan++)
		if (p == ap->p[chan])
			return chan;
	BUG();
	return 0;
}


static void pwmc_chan_writel(const struct atmel_pwmc *ap, int chan, int offset, int val)
{
	if (PWMC_CMR == offset)
		val &= ((1 << PWMC_CMR_CPD)
			| (1 << PWMC_CMR_CPOL)
			| (1 << PWMC_CMR_CALG)
			| (PWMC_CMR_CPRE_MASK));
	else
		val &= ap->ccnt_mask;

	pwmc_writel(ap, offset + PWMC_CHAN_BASE
		    + (chan * PWMC_CHAN_STRIDE), val);
}

static u32 pwmc_chan_readl(const struct atmel_pwmc *ap, int chan, int offset)
{
	return pwmc_readl(ap, offset + PWMC_CHAN_BASE
			  + (chan * PWMC_CHAN_STRIDE));
}

static int __atmel_pwmc_is_on(const struct atmel_pwmc *ap, int chan)
{
	return (pwmc_readl(ap, PWMC_SR) & BIT(chan)) ? 1 : 0;
}

static void __atmel_pwmc_stop(const struct atmel_pwmc *ap, int chan)
{
	pwmc_writel(ap, PWMC_DIS, BIT(chan));
}

static void __atmel_pwmc_start(const struct atmel_pwmc *ap, int chan)
{
	pwmc_writel(ap, PWMC_ENA, BIT(chan));
}

static void __atmel_pwmc_config_polarity(struct atmel_pwmc *ap, int chan,
					 struct pwm_config *c)
{
	unsigned long cmr = pwmc_chan_readl(ap, chan, PWMC_CMR);
	struct pwm_device *p = ap->p[chan];

	if (c->polarity)
		clear_bit(PWMC_CMR_CPOL, &cmr);
	else
		set_bit(PWMC_CMR_CPOL, &cmr);
	pwmc_chan_writel(ap, chan, PWMC_CMR, cmr);
	p->polarity = c->polarity ? 1 : 0;

	dev_dbg(&p->dev, "polarity %d\n", c->polarity);
}

static void __atmel_pwmc_config_duty_ticks(struct atmel_pwmc *ap, int chan,
					   struct pwm_config *c)
{
	unsigned long cmr, cprd, cpre, cdty;
	struct pwm_device *p = ap->p[chan];

	cmr = pwmc_chan_readl(ap, chan, PWMC_CMR);
	cprd = pwmc_chan_readl(ap, chan, PWMC_CPRD);

	cpre = cmr & PWMC_CMR_CPRE_MASK;
	clear_bit(PWMC_CMR_CPD, &cmr);

	cdty = cprd - (c->duty_ticks >> cpre);

	p->duty_ticks = c->duty_ticks;

	if (__atmel_pwmc_is_on(ap, chan)) {
		pwmc_chan_writel(ap, chan, PWMC_CMR, cmr);
		pwmc_chan_writel(ap, chan, PWMC_CUPD, cdty);
	} else
		pwmc_chan_writel(ap, chan, PWMC_CDTY, cdty);

	dev_dbg(&p->dev, "duty_ticks = %lu cprd = %lx"
		" cdty = %lx cpre = %lx\n", p->duty_ticks,
		cprd, cdty, cpre);
}

static int __atmel_pwmc_config_period_ticks(struct atmel_pwmc *ap, int chan,
					    struct pwm_config *c)
{
	u32 cmr, cprd, cpre;
	struct pwm_device *p = ap->p[chan];

	cpre = fls(c->period_ticks);
	if (cpre < 16)
		cpre = 0;
	else {
		cpre -= 15;
		if (cpre > 10)
			return -EINVAL;
	}

	cmr = pwmc_chan_readl(ap, chan, PWMC_CMR);
	cmr &= ~PWMC_CMR_CPRE_MASK;
	cmr |= cpre;

	cprd = c->period_ticks >> cpre;

	pwmc_chan_writel(ap, chan, PWMC_CMR, cmr);
	pwmc_chan_writel(ap, chan, PWMC_CPRD, cprd);
	p->period_ticks = c->period_ticks;

	dev_dbg(&p->dev, "period_ticks = %lu cprd = %x cpre = %x\n",
		 p->period_ticks, cprd, cpre);
	return 0;
}

static int atmel_pwmc_config_nosleep(struct pwm_device *p, struct pwm_config *c)
{
	struct atmel_pwmc *ap = pwm_get_drvdata(p);
	int chan = __to_chan(ap, p);
	int ret = 0;
	unsigned long flags;

	spin_lock_irqsave(&ap->lock, flags);

	switch (c->config_mask) {

	case BIT(PWM_CONFIG_DUTY_TICKS):
		__atmel_pwmc_config_duty_ticks(ap, chan, c);
		break;

	case BIT(PWM_CONFIG_STOP):
		__atmel_pwmc_stop(ap, chan);
		break;

	case BIT(PWM_CONFIG_START):
		__atmel_pwmc_start(ap, chan);
		break;

	case BIT(PWM_CONFIG_POLARITY):
		__atmel_pwmc_config_polarity(ap, chan, c);
		break;

	default:
		ret = -EINVAL;
		break;
	}

	spin_unlock_irqrestore(&ap->lock, flags);
	return ret;
}

static int atmel_pwmc_stop_sync(struct atmel_pwmc *ap, int chan)
{
	struct pwm_device *p = ap->p[chan];
	int was_on = __atmel_pwmc_is_on(ap, chan);
	int ret;

	if (!was_on)
		return 0;

	do {
		init_completion(&ap->complete);
		set_bit(PWM_FLAG_STOP, &p->flags);
		pwmc_writel(ap, PWMC_IER, BIT(chan));

		dev_dbg(&p->dev, "waiting on stop_sync completion...\n");

		ret = wait_for_completion_interruptible(&ap->complete);

		dev_dbg(&p->dev, "stop_sync complete (%d)\n", ret);

		if (ret)
			return ret;
	} while (test_bit(PWM_FLAG_STOP, &p->flags));

	return 1;
}

static int atmel_pwmc_config(struct pwm_device *p, struct pwm_config *c)
{
	struct atmel_pwmc *ap = pwm_get_drvdata(p);
	int chan = __to_chan(ap, p);
	int was_on = 0;
	int ret;


	if (!atmel_pwmc_config_nosleep(p, c))
		return 0;

	might_sleep();

	dev_dbg(&p->dev, "config_mask %lx\n", c->config_mask);

	was_on = atmel_pwmc_stop_sync(ap, chan);
	if (was_on < 0)
		return was_on;

	if (test_bit(PWM_CONFIG_PERIOD_TICKS, &c->config_mask)) {
		ret = __atmel_pwmc_config_period_ticks(ap, chan, c);
		if (ret)
			return ret;

		if (!test_bit(PWM_CONFIG_DUTY_TICKS, &c->config_mask)) {
			struct pwm_config d = {
				.config_mask = PWM_CONFIG_DUTY_TICKS,
				.duty_ticks = p->duty_ticks,
			};
			__atmel_pwmc_config_duty_ticks(ap, chan, &d);
		}
	}

	if (test_bit(PWM_CONFIG_DUTY_TICKS, &c->config_mask))
		__atmel_pwmc_config_duty_ticks(ap, chan, c);

	if (test_bit(PWM_CONFIG_POLARITY, &c->config_mask))
		__atmel_pwmc_config_polarity(ap, chan, c);

	if (test_bit(PWM_CONFIG_START, &c->config_mask)
	    || (was_on && !test_bit(PWM_CONFIG_STOP, &c->config_mask)))
		__atmel_pwmc_start(ap, chan);

	return 0;
}

static int atmel_pwmc_request(struct pwm_device *p)
{
	struct atmel_pwmc *ap = pwm_get_drvdata(p);
	int chan = __to_chan(ap, p);
	unsigned long flags;

	spin_lock_irqsave(&ap->lock, flags);
	clk_enable(ap->clk);
	p->tick_hz = clk_get_rate(ap->clk);
	__atmel_pwmc_stop(ap, chan);
	spin_unlock_irqrestore(&ap->lock, flags);

	return 0;
}

static void atmel_pwmc_release(struct pwm_device *p)
{
	struct atmel_pwmc *ap = pwm_get_drvdata(p);
	clk_disable(ap->clk);
}

const struct pwm_device_ops atmel_pwm_ops = {
	.request = atmel_pwmc_request,
	.release = atmel_pwmc_release,
	.config_nosleep = atmel_pwmc_config_nosleep,
	.config = atmel_pwmc_config,
	.owner = THIS_MODULE,
};

static int __devinit atmel_pwmc_probe(struct platform_device *pdev)
{
	struct atmel_pwmc *ap;
	struct resource *r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	signed int chan;
	int ret = 0;

	ap = kzalloc(sizeof(*ap), GFP_KERNEL);
	if (!ap) {
		ret = -ENOMEM;
		goto err_atmel_pwmc_alloc;
	}

	spin_lock_init(&ap->lock);
	init_completion(&ap->complete);
	platform_set_drvdata(pdev, ap);

	/* TODO: the datasheets are unclear as to how large CCNT
	 * actually is across all adopters of the PWMC; sixteen bits
	 * seems a safe assumption for now */
	ap->ccnt_mask = 0xffffUL;

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

	clk_enable(ap->clk);
	pwmc_writel(ap, PWMC_DIS, -1);
	pwmc_writel(ap, PWMC_IDR, -1);
	clk_disable(ap->clk);

	for (chan = 0; chan < NCHAN; chan++) {
		ap->p[chan] = pwm_register(&atmel_pwm_ops, &pdev->dev, "%s:%d",
					   dev_name(&pdev->dev), chan);
		if (IS_ERR_OR_NULL(ap->p[chan]))
			goto err_pwm_register;
		pwm_set_drvdata(ap->p[chan], ap);
	}

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
err_atmel_pwmc_alloc:
	dev_dbg(&pdev->dev, "%s: error, returning %d\n", __func__, ret);
	return ret;
}

static int __devexit atmel_pwmc_remove(struct platform_device *pdev)
{
	struct atmel_pwmc *ap = platform_get_drvdata(pdev);
	int chan;

	for (chan = 0; chan < NCHAN; chan++)
		pwm_unregister(ap->p[chan]);

	clk_enable(ap->clk);
	pwmc_writel(ap, PWMC_IDR, -1);
	pwmc_writel(ap, PWMC_DIS, -1);
	clk_disable(ap->clk);

	clk_put(ap->clk);
	iounmap(ap->iobase);

	kfree(ap);

	return 0;
}

static struct platform_driver atmel_pwmc_driver = {
	.driver = {
		/* note: this name has to match the one in at91*_devices.c */
		.name = "atmel_pwmc",
		.owner = THIS_MODULE,
	},
	.probe = atmel_pwmc_probe,
	.remove = __devexit_p(atmel_pwmc_remove),
};

static int __init atmel_pwmc_init(void)
{
	return platform_driver_register(&atmel_pwmc_driver);
}
module_init(atmel_pwmc_init);

static void __exit atmel_pwmc_exit(void)
{
	platform_driver_unregister(&atmel_pwmc_driver);
}
module_exit(atmel_pwmc_exit);

MODULE_AUTHOR("Bill Gatliff <bgat@billgatliff.com>");
MODULE_DESCRIPTION("Driver for Atmel PWMC peripheral");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:atmel_pwmc");
