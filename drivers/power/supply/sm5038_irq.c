/*
 * NOTE: This file has been modified by Sony Corporation.
 * Modifications are Copyright 2022 Sony Corporation,
 * and licensed under the license of the file.
 */
// SPDX-License-Identifier: GPL-2.0-only
/*
 *  sm5038_irq.c - Interrupt controller support for sm5038
 *
 */

#include <linux/err.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/module.h>
#include <linux/power/sm5038.h>
#include <linux/delay.h>

static const u8 sm5038_mask_reg[] = {
	[MUIC_INT1]     = SM5038_MUIC_REG_INTMASK1,
	[MUIC_INT2]     = SM5038_MUIC_REG_INTMASK2,
	[CHG_INT1]      = SM5038_CHG_REG_INTMSK1,
	[CHG_INT2]      = SM5038_CHG_REG_INTMSK2,
    [CHG_INT3]      = SM5038_CHG_REG_INTMSK3,
    [CHG_INT4]      = SM5038_CHG_REG_INTMSK4,
    [CHG_INT5]      = SM5038_CHG_REG_INTMSK5,
    [FG_INT]        = SM5038_FG_REG_INTFG_MASK,
};

struct sm5038_irq_data {
	int mask;
	int group;
};

#define DECLARE_IRQ(idx, _group, _mask)		\
	[(idx)] = { .group = (_group), .mask = (_mask) }

static const struct sm5038_irq_data sm5038_irqs[] = {
    /* MUIC-irqs */
    DECLARE_IRQ(SM5038_MUIC_IRQ_INT1_DPDM_OVP,	        MUIC_INT1,      1 << 5),
    DECLARE_IRQ(SM5038_MUIC_IRQ_INT1_VBUS_DETACH,		MUIC_INT1,      1 << 4),
    DECLARE_IRQ(SM5038_MUIC_IRQ_INT1_CHGTYPE,	        MUIC_INT1,      1 << 1),
    DECLARE_IRQ(SM5038_MUIC_IRQ_INT1_DCDTIMEOUT,	    MUIC_INT1,      1 << 0),
    DECLARE_IRQ(SM5038_MUIC_IRQ_INT2_VBUS_UPDATE,	    MUIC_INT2,      1 << 2),
    DECLARE_IRQ(SM5038_MUIC_IRQ_INT2_HVDCP,				MUIC_INT2,      1 << 0),

    /* Charger-irqs */
    DECLARE_IRQ(SM5038_CHG_IRQ_INT1_WPCINLIMIT,	        CHG_INT1,       1 << 7),
    DECLARE_IRQ(SM5038_CHG_IRQ_INT1_WPCINOVP,	        CHG_INT1,       1 << 6),
    DECLARE_IRQ(SM5038_CHG_IRQ_INT1_WPCINUVLO,	        CHG_INT1,       1 << 5),
    DECLARE_IRQ(SM5038_CHG_IRQ_INT1_WPCINPOK,	        CHG_INT1,       1 << 4),
    DECLARE_IRQ(SM5038_CHG_IRQ_INT1_VBUSLIMIT,	        CHG_INT1,       1 << 3),
    DECLARE_IRQ(SM5038_CHG_IRQ_INT1_VBUSOVP,	        CHG_INT1,       1 << 2),
    DECLARE_IRQ(SM5038_CHG_IRQ_INT1_VBUSUVLO,	        CHG_INT1,       1 << 1),
    DECLARE_IRQ(SM5038_CHG_IRQ_INT1_VBUSPOK,	        CHG_INT1,       1 << 0),

    DECLARE_IRQ(SM5038_CHG_IRQ_INT2_WDTMROFF,	        CHG_INT2,       1 << 7),
    DECLARE_IRQ(SM5038_CHG_IRQ_INT2_DONE,	            CHG_INT2,       1 << 6),
    DECLARE_IRQ(SM5038_CHG_IRQ_INT2_TOPOFF,	            CHG_INT2,       1 << 5),
    DECLARE_IRQ(SM5038_CHG_IRQ_INT2_Q4FULLON,	        CHG_INT2,       1 << 4),
    DECLARE_IRQ(SM5038_CHG_IRQ_INT2_CHGON,	            CHG_INT2,       1 << 3),
    DECLARE_IRQ(SM5038_CHG_IRQ_INT2_NOBAT,              CHG_INT2,       1 << 2),
    DECLARE_IRQ(SM5038_CHG_IRQ_INT2_BATOVP,             CHG_INT2,       1 << 1),
    DECLARE_IRQ(SM5038_CHG_IRQ_INT2_AICL,	            CHG_INT2,       1 << 0),

    DECLARE_IRQ(SM5038_CHG_IRQ_INT3_VSYSOVP,	        CHG_INT3,       1 << 7),
    DECLARE_IRQ(SM5038_CHG_IRQ_INT3_nENQ4,	            CHG_INT3,       1 << 6),
    DECLARE_IRQ(SM5038_CHG_IRQ_INT3_FASTTMROFF,	        CHG_INT3,       1 << 5),
    DECLARE_IRQ(SM5038_CHG_IRQ_INT3_TRICKLETMROFF,	    CHG_INT3,       1 << 4),
    DECLARE_IRQ(SM5038_CHG_IRQ_INT3_DISLIMIT,	        CHG_INT3,       1 << 3),
    DECLARE_IRQ(SM5038_CHG_IRQ_INT3_OTGFAIL,	        CHG_INT3,       1 << 2),
    DECLARE_IRQ(SM5038_CHG_IRQ_INT3_THEMSHDN,           CHG_INT3,       1 << 1),
    DECLARE_IRQ(SM5038_CHG_IRQ_INT3_THEMREG,            CHG_INT3,       1 << 0),

    DECLARE_IRQ(SM5038_CHG_IRQ_INT4_CVMODE,	            CHG_INT4,       1 << 7),
    DECLARE_IRQ(SM5038_CHG_IRQ_INT4_TXFAIL,	            CHG_INT4,       1 << 6),
    DECLARE_IRQ(SM5038_CHG_IRQ_INT4_WPC_UPDATE,         CHG_INT4,       1 << 5),
    DECLARE_IRQ(SM5038_CHG_IRQ_INT4_BOOSTPOK,	        CHG_INT4,       1 << 1),
    DECLARE_IRQ(SM5038_CHG_IRQ_INT4_BOOSTPOK_NG,	    CHG_INT4,       1 << 0),

    DECLARE_IRQ(SM5038_CHG_IRQ_INT5_ABSTMROFF,	        CHG_INT5,       1 << 6),
    DECLARE_IRQ(SM5038_CHG_IRQ_INT5_FLEDOPEN,           CHG_INT5,       1 << 1),
    DECLARE_IRQ(SM5038_CHG_IRQ_INT5_FLEDSHORT,          CHG_INT5,       1 << 0),

    /* FuelGauge-irqs */
    DECLARE_IRQ(SM5038_FG_IRQ_INT_LOW_VOLTAGE,	        FG_INT,         1 << 0),
};

static struct i2c_client *get_i2c(struct sm5038_dev *sm5038, int src)
{
	switch (src) {
	case MUIC_INT1:
	case MUIC_INT2:
		return sm5038->muic_i2c;
	case CHG_INT1:
	case CHG_INT2:
	case CHG_INT3:
	case CHG_INT4:
	case CHG_INT5:
		return sm5038->charger_i2c;
	case FG_INT:
		return sm5038->fuelgauge_i2c;
	}

	return ERR_PTR(-EINVAL);
}

static void sm5038_irq_lock(struct irq_data *data)
{
	struct sm5038_dev *sm5038 = irq_get_chip_data(data->irq);

	mutex_lock(&sm5038->irqlock);
}

static void sm5038_irq_sync_unlock(struct irq_data *data)
{
	struct sm5038_dev *sm5038 = irq_get_chip_data(data->irq);
	int i;

	for (i = 0; i < SM5038_IRQ_GROUP_NR; i++) {
		struct i2c_client *i2c = get_i2c(sm5038, i);

		if (IS_ERR_OR_NULL(i2c))
			continue;

		sm5038->irq_masks_cache[i] = sm5038->irq_masks_cur[i];

		if (i == FG_INT) {
			sm5038_write_word(i2c, sm5038_mask_reg[i], sm5038->irq_masks_cur[i]);
		} else {
			sm5038_write_reg(i2c, sm5038_mask_reg[i], sm5038->irq_masks_cur[i]);
		}

	}

	mutex_unlock(&sm5038->irqlock);
}

static const inline struct sm5038_irq_data *
irq_to_sm5038_irq(struct sm5038_dev *sm5038, int irq)
{
	return &sm5038_irqs[irq - sm5038->irq_base];
}

static void sm5038_irq_mask(struct irq_data *data)
{
	struct sm5038_dev *sm5038 = irq_get_chip_data(data->irq);
	const struct sm5038_irq_data *irq_data = irq_to_sm5038_irq(sm5038, data->irq);

	if (irq_data->group >= SM5038_IRQ_GROUP_NR)
		return;

    sm5038->irq_masks_cur[irq_data->group] |= irq_data->mask;
}

static void sm5038_irq_unmask(struct irq_data *data)
{
	struct sm5038_dev *sm5038 = irq_get_chip_data(data->irq);
    const struct sm5038_irq_data *irq_data = irq_to_sm5038_irq(sm5038, data->irq);

	if (irq_data->group >= SM5038_IRQ_GROUP_NR)
		return;

    sm5038->irq_masks_cur[irq_data->group] &= ~irq_data->mask;
}

static void sm5038_irq_disable(struct irq_data *data)
{
	sm5038_irq_mask(data);
}

static void sm5038_irq_enable(struct irq_data *data)
{
	sm5038_irq_unmask(data);
}

static struct irq_chip sm5038_irq_chip = {
	.name			        = SM5038_DEV_NAME,
	.irq_bus_lock		    = sm5038_irq_lock,
	.irq_bus_sync_unlock	= sm5038_irq_sync_unlock,
	.irq_mask		        = sm5038_irq_mask,
	.irq_unmask		        = sm5038_irq_unmask,
	.irq_disable		    = sm5038_irq_disable,
	.irq_enable		        = sm5038_irq_enable,
};

static irqreturn_t sm5038_irq_thread(int irq, void *data)
{
	struct sm5038_dev *sm5038 = data;
	u8 irq_reg[SM5038_IRQ_GROUP_NR] = {0};
	u8 irq_src;
	int i, ret;
	int rtn = -EINVAL;
	pr_debug("%s: irq gpio pre-state(0x%02x)\n", __func__, gpio_get_value(sm5038->irq_gpio));

	sm5038_irq_thread_lock();

	ret = sm5038_read_reg(sm5038->charger_i2c, SM5038_CHG_REG_INT_SOURCE, &irq_src);
	pr_info("%s\n irq_src = %x (ret=%d)", __func__, irq_src, ret);
	if (ret) {
		pr_info("%s:%s fail to read interrupt source: %d\n", SM5038_DEV_NAME, __func__, ret);
		sm5038_irq_thread_unlock();
		return IRQ_NONE;
	}
	pr_debug("%s: INT_SOURCE=0x%02x)\n", __func__, irq_src);

	/* Irregular case: check IC status */
	if (irq_src == 0) {
		msleep(10);
		ret = sm5038_read_reg(sm5038->charger_i2c, SM5038_CHG_REG_INT_SOURCE, &irq_src);
		pr_info("[%s:%s]retry irq_src = 0x%x (ret=%d)", SM5038_DEV_NAME, __func__, irq_src, ret);
		if (ret) {
			pr_info("%s:%s fail to read interrupt source: %d\n", SM5038_DEV_NAME, __func__, ret);
			sm5038_irq_thread_unlock();
			return IRQ_NONE;
		}
		
		ret = sm5038_bulk_read(sm5038->muic_i2c, SM5038_MUIC_REG_INT1, SM5038_NUM_IRQ_MUIC_REGS, &irq_reg[MUIC_INT1]);
		pr_info ("[%s:%s] MUIC_INT1 = 0x%x, MUIC_INT2 = 0x%x\n", SM5038_DEV_NAME, __func__, irq_reg[MUIC_INT1], irq_reg[MUIC_INT2]);
		if (ret) {
			pr_err("[%s:%s] fail to read MUIC_INT1 %d\n", SM5038_DEV_NAME, __func__, ret);
			sm5038_irq_thread_unlock();
			return IRQ_NONE;
		}
		if ( (irq_reg[MUIC_INT1] != 0x00) || (irq_reg[MUIC_INT2] != 0x00) ) {
			irq_src = irq_src & 0xFE;
		}

		if ( (irq_reg[MUIC_INT1] == 0x00) && (irq_reg[MUIC_INT2] == 0x00) ) {
		
			if (sm5038->check_muic_reset)
				sm5038->check_muic_reset(sm5038->muic_i2c, sm5038->muic_data);
			if (sm5038->check_chg_reset)
				sm5038->check_chg_reset(sm5038->charger_i2c, sm5038->chg_data);
			if (sm5038->check_fg_reset)
				sm5038->check_fg_reset(sm5038->fuelgauge_i2c, sm5038->fg_data);

			ret = gpio_request(sm5038->irq_gpio, "if_pmic_irq");
			if (ret) {
				dev_err(sm5038->dev, "%s: failed requesting gpio %d\n",
					__func__, sm5038->irq_gpio);
				sm5038_irq_thread_unlock();
				return IRQ_NONE;
			}				
			rtn = gpio_direction_output(sm5038->irq_gpio, 1);

			ret = gpio_request(sm5038->irq_gpio, "if_pmic_irq");
			if (ret) {
				dev_err(sm5038->dev, "%s: failed requesting gpio %d\n",
					__func__, sm5038->irq_gpio);
				sm5038_irq_thread_unlock();
				return IRQ_NONE;
			}
			rtn = gpio_direction_input(sm5038->irq_gpio);
			pr_info("%s:%s Done gpio_direction_input: %d(%d)\n", SM5038_DEV_NAME, __func__, rtn, ret);
			gpio_free(sm5038->irq_gpio);
		}
	}

	/* Charger INT 1 ~ 5 */
	if (irq_src & SM5038_IRQSRC_CHG) {
		ret = sm5038_bulk_read(sm5038->charger_i2c, SM5038_CHG_REG_INT1, SM5038_NUM_IRQ_CHG_REGS, &irq_reg[CHG_INT1]);
		if (ret) {
			pr_err("%s:%s fail to read CHG_INT source: %d\n", SM5038_DEV_NAME, __func__, ret);
			sm5038_irq_thread_unlock();
			return IRQ_NONE;
		}
		for (i = CHG_INT1; i <= CHG_INT5; i++) {
			pr_debug("%s:%s CHG_INT%d = 0x%x\n", SM5038_DEV_NAME, __func__, (i - 1), irq_reg[i]);
		}
	}

	/* MUIC INT 1 ~ 2 */
	if (irq_src & SM5038_IRQSRC_MUIC) {
		ret = sm5038_bulk_read(sm5038->muic_i2c, SM5038_MUIC_REG_INT1, SM5038_NUM_IRQ_MUIC_REGS, &irq_reg[MUIC_INT1]);
		pr_info ("%s:%s MUIC_INT1 = 0x%x, MUIC_INT2 = 0x%x\n", SM5038_DEV_NAME, __func__, irq_reg[MUIC_INT1], irq_reg[MUIC_INT2]);
		if (ret) {
			pr_err("%s:%s fail to read MUIC_INT source: %d\n", SM5038_DEV_NAME, __func__, ret);
			sm5038_irq_thread_unlock();
			return IRQ_NONE;
		}
		for (i = MUIC_INT1; i <= MUIC_INT2; i++) {
			pr_debug("%s:%s MUIC_INT%d = 0x%x\n", SM5038_DEV_NAME, __func__, (i + 1), irq_reg[i]);
		}
	}

	/* Fuel Gauge INT */
	if (irq_src & SM5038_IRQSRC_FG) {
		/* FG_INT Lock */
		sm5038->irq_masks_cur[FG_INT] |= (1 << 7);
		sm5038_write_word(sm5038->fuelgauge_i2c, sm5038_mask_reg[FG_INT], sm5038->irq_masks_cur[FG_INT]);

		irq_reg[FG_INT] = (u8)(sm5038_read_word(sm5038->fuelgauge_i2c, SM5038_FG_REG_INTFG) & 0x00FF);

		/* FG_INT Un-Lock */
		sm5038->irq_masks_cur[FG_INT] &= ~(1 << 7);
		sm5038_write_word(sm5038->fuelgauge_i2c, sm5038_mask_reg[FG_INT], sm5038->irq_masks_cur[FG_INT]);

		pr_info("%s:%s FG_INT = 0x%x\n", SM5038_DEV_NAME, __func__, irq_reg[FG_INT]);
	}

	/* Apply masking */
	for (i = 0; i < SM5038_IRQ_GROUP_NR; i++) {
		irq_reg[i] &= ~sm5038->irq_masks_cur[i];
	}

	/* Report */
	for (i = 0; i < SM5038_IRQ_NR; i++) {
		if (irq_reg[sm5038_irqs[i].group] & sm5038_irqs[i].mask) {
			handle_nested_irq(sm5038->irq_base + i);
		}
	}

	sm5038_irq_thread_unlock();

	return IRQ_HANDLED;
}

int sm5038_irq_init(struct sm5038_dev *sm5038)
{
	struct i2c_client *i2c;
	int i;
	int ret;

	if (!sm5038->irq_gpio) {
		dev_warn(sm5038->dev, "[sm5038]No interrupt specified.\n");
		sm5038->irq_base = 0;
		return 0;
	}

	if (!sm5038->irq_base) {
		dev_err(sm5038->dev, "[sm5038]No interrupt base specified.\n");
		return 0;
	}

	mutex_init(&sm5038->irqlock);

	sm5038->irq = gpio_to_irq(sm5038->irq_gpio);
	pr_err("%s:%s irq=%d, irq->gpio=%d\n", SM5038_DEV_NAME, __func__,
			sm5038->irq, sm5038->irq_gpio);

	ret = gpio_request(sm5038->irq_gpio, "if_pmic_irq");
	if (ret) {
		dev_err(sm5038->dev, "%s: failed requesting gpio %d\n",
			__func__, sm5038->irq_gpio);
		return ret;
	}
	gpio_direction_input(sm5038->irq_gpio);
	gpio_free(sm5038->irq_gpio);

	/* Mask individual interrupt sources */
	for (i = 0; i < SM5038_IRQ_GROUP_NR; i++) {
		i2c = get_i2c(sm5038, i);

		if (IS_ERR_OR_NULL(i2c))
			continue;

		if (i == FG_INT) {
			sm5038->irq_masks_cur[i] = 0x000f;
			sm5038->irq_masks_cache[i] = 0x000f;
			sm5038_write_word(i2c, sm5038_mask_reg[i], sm5038->irq_masks_cur[i]);
		} else {
			sm5038->irq_masks_cur[i] = 0xff;
			sm5038->irq_masks_cache[i] = 0xff;
			sm5038_write_reg(i2c, sm5038_mask_reg[i], sm5038->irq_masks_cur[i]);
		}
	}

	/* Register with genirq */
	for (i = 0; i < SM5038_IRQ_NR; i++) {
		int cur_irq;
		cur_irq = i + sm5038->irq_base;
		irq_set_chip_data(cur_irq, sm5038);
		irq_set_chip_and_handler(cur_irq, &sm5038_irq_chip, handle_level_irq);
		irq_set_nested_thread(cur_irq, 1);
		irq_set_noprobe(cur_irq);
	}

	ret = request_threaded_irq(sm5038->irq, NULL, sm5038_irq_thread, IRQF_TRIGGER_LOW | IRQF_ONESHOT,
			"sm5038_irq", sm5038);
	if (ret) {
		dev_err(sm5038->dev, "[sm5038]Fail to request IRQ %d: %d\n", sm5038->irq, ret);
		return ret;
	}

	return 0;
}
EXPORT_SYMBOL_GPL(sm5038_irq_init);

void sm5038_irq_exit(struct sm5038_dev *sm5038)
{
	if (sm5038->irq)
		free_irq(sm5038->irq, sm5038);
}
EXPORT_SYMBOL_GPL(sm5038_irq_exit);

MODULE_LICENSE("GPL");
