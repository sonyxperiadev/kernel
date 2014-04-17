/*****************************************************************************
*  Copyright 2012 Broadcom Corporation.  All rights reserved.
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
#include <linux/init.h>
#include <linux/bug.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/debugfs.h>
#include <linux/platform_device.h>
#include <linux/power_supply.h>

#include <linux/mfd/bcmpmu59xxx.h>
#include <linux/mfd/bcmpmu59xxx_reg.h>
#include <linux/spa_power.h>
#include <linux/power/bcmpmu-fg.h>

static int dbg_mask = BCMPMU_PRINT_ERROR | BCMPMU_PRINT_INIT |
					BCMPMU_PRINT_FLOW;
/* static int debug_mask = 0xFF; */
#define pr_pb(debug_level, args...) \
	do { \
		if (dbg_mask & BCMPMU_PRINT_##debug_level) { \
			pr_info(args); \
		} \
	} while (0)

#define SPA_FIFO_SIZE	16
#define SPA_FIFO_IS_EMPTY(f) ((f).head == (f).tail && !(f).full)
#define SPA_FIFO_IS_FULL(f) ((f).full)

#define SPA_WORK_SCHEDULE_DELAY	0

#define SPA_ADC_READ_TRIES 5
struct spa_event_fifo {
	u32 head;
	u32 tail;
	u32 full;
	u32 event[SPA_FIFO_SIZE];
	u32 data[SPA_FIFO_SIZE];
};

struct bcmpmu_spa_pb {
	struct bcmpmu59xxx *bcmpmu;
	struct mutex lock;
	struct notifier_block nb_chgr_det;
	struct notifier_block nb_usbin;
	struct delayed_work spa_work;
	struct spa_event_fifo spa_fifo;
	struct power_supply chrgr;
	u32 chg_type;
	u32 temp_lmt;
	u32 mb_ov;
	u32 usb_ov;
	u32 capacity;
	u32 temp_adc;
	u32 chgr_status;
	u32 icc_fc;
	u32 eoc;
	u32 vbat;
	u32 present;
#ifdef CONFIG_DEBUG_FS
	struct dentry *dent_pb;
#endif
};

static enum power_supply_property bcmpmu_spa_pb_chrgr_props[] = {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_CAPACITY,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_TEMP,
	POWER_SUPPLY_PROP_TYPE,
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_CURRENT_NOW,
	POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN,
	POWER_SUPPLY_PROP_BATT_TEMP_ADC,
};


static int __spa_fifo_add(struct spa_event_fifo *fifo, u32 event, u32 data)
{
	if (SPA_FIFO_IS_FULL(*fifo))
		return -ENOMEM;
	fifo->event[fifo->tail] = event;
	fifo->data[fifo->tail] = data;
	fifo->tail = (fifo->tail + 1) % SPA_FIFO_SIZE;
	if (fifo->tail == fifo->head)
		fifo->full = 1;
	return 0;
}

static int __spa_fifo_get(struct spa_event_fifo *fifo, u32 *event, u32 *data)
{
	if (SPA_FIFO_IS_EMPTY(*fifo))
		return -EPERM;
	*event = fifo->event[fifo->head];
	*data = fifo->data[fifo->head];
	fifo->head = (fifo->head + 1) % SPA_FIFO_SIZE;
	fifo->full = 0;
	return 0;
}

static int bcmpmu_spa_pb_chrgr_set_property(struct power_supply *ps,
		enum power_supply_property prop,
		const union power_supply_propval *propval)
{
	int ret = 0;
	struct bcmpmu_spa_pb *bcmpmu_spa_pb;
	struct bcmpmu59xxx *bcmpmu;
	int retry = 3;
	int retries = 6;
	u8 reg;

	bcmpmu_spa_pb = container_of(ps,
			struct bcmpmu_spa_pb, chrgr);
	bcmpmu = bcmpmu_spa_pb->bcmpmu;

	switch (prop) {
	case POWER_SUPPLY_PROP_STATUS:
		bcmpmu_spa_pb->chgr_status = (u32)propval->intval;
		if (propval->intval == POWER_SUPPLY_STATUS_CHARGING) {
			pr_pb(FLOW, "%s: enable charging\n", __func__);
			while (retries--) {
				msleep(100);
				bcmpmu->read_dev(bcmpmu, PMU_REG_ENV2, &reg);
				if (bcmpmu_spa_pb->chg_type == PMU_CHRGR_TYPE_NONE)
					break;

				if (reg & ENV2_P_UBPD_INT) {
			bcmpmu_chrgr_usb_en(bcmpmu, 1);
					break;
				} else {
					if (!retries)
						pr_pb(FLOW, "%s: enable charging fail\n", __func__);
				}
			}
		} else {
			pr_pb(FLOW, "%s: disable charging\n", __func__);
			bcmpmu_chrgr_usb_en(bcmpmu, 0);
		}
		break;

	case POWER_SUPPLY_PROP_CURRENT_NOW:
		bcmpmu_spa_pb->icc_fc = (u32)propval->intval;
		pr_pb(FLOW, "%s: set current %d\n", __func__,
				bcmpmu_spa_pb->icc_fc);
		bcmpmu_set_icc_fc(bcmpmu, (int)propval->intval);
		break;

	case POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN:
		pr_pb(FLOW, "%s: POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN\n",
			__func__);
		ret = bcmpmu_fg_set_sw_eoc_current(bcmpmu, propval->intval);
		if (ret)
			ret = -ENODATA;
		break;

	case POWER_SUPPLY_PROP_CAPACITY:
		pr_pb(FLOW, "%s: POWER_SUPPLY_PROP_CAPACITY\n", __func__);
		if (propval->intval == 1) {
			do {
				ret = bcmpmu_fg_calibrate_battery(bcmpmu);
				if (!ret)
					break;
				msleep(500);
			} while (retry--);
			if (retry <= 0)
				ret = -ENODATA;
		}
		break;

	default:
		ret = -EINVAL;
		break;
	}
	return ret;
}

static void  spa_bcmpmu_adc_read(struct bcmpmu59xxx *bcmpmu,
				enum bcmpmu_adc_channel channel,
				enum bcmpmu_adc_req req,
				struct bcmpmu_adc_result *result)
{
	int ret = 0;
	int retries = SPA_ADC_READ_TRIES;

	while (retries--) {
		ret = bcmpmu_adc_read(bcmpmu, channel, req, result);
		if (!ret)
			break;
	}
	BUG_ON(retries <= 0);
}

#if 0
extern unsigned int musb_get_charger_type(void);
#endif

static int bcmpmu_spa_pb_chrgr_get_property(struct power_supply *ps,
	enum power_supply_property prop,
	union power_supply_propval *propval)
{
	int ret = 0;
	struct bcmpmu_adc_result adc_result;
	struct bcmpmu59xxx *bcmpmu;
	struct bcmpmu_spa_pb *bcmpmu_spa_pb;
	bcmpmu_spa_pb = container_of(ps,
			struct bcmpmu_spa_pb, chrgr);
	bcmpmu = bcmpmu_spa_pb->bcmpmu;

	switch (prop) {
	case POWER_SUPPLY_PROP_STATUS:
		propval->intval = bcmpmu_spa_pb->chgr_status;
		break;

	case POWER_SUPPLY_PROP_TYPE:
#if 0
		propval->intval = musb_get_charger_type();
#else
		switch (bcmpmu_spa_pb->chg_type) {
		case PMU_CHRGR_TYPE_SDP:
			propval->intval = POWER_SUPPLY_TYPE_USB;
			break;

		case PMU_CHRGR_TYPE_CDP:
			propval->intval = POWER_SUPPLY_TYPE_USB_CDP;
			break;

		case PMU_CHRGR_TYPE_DCP:
			propval->intval = POWER_SUPPLY_TYPE_USB_DCP;
			break;

		default:
			propval->intval = POWER_SUPPLY_TYPE_BATTERY;
			break;
		}
#endif
		break;

	case POWER_SUPPLY_PROP_TEMP:
	case POWER_SUPPLY_PROP_BATT_TEMP_ADC:
		spa_bcmpmu_adc_read(bcmpmu, PMU_ADC_CHANN_NTC,
				PMU_ADC_REQ_SAR_MODE, &adc_result);
		bcmpmu_spa_pb->temp_adc = adc_result.raw;
		if (prop == POWER_SUPPLY_PROP_BATT_TEMP_ADC)
			propval->intval = adc_result.raw;
		else
			propval->intval = adc_result.conv;
		break;

	case POWER_SUPPLY_PROP_CAPACITY:
		propval->intval = bcmpmu_fg_get_current_capacity(bcmpmu);
		break;

	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		spa_bcmpmu_adc_read(bcmpmu, PMU_ADC_CHANN_VMBATT,
				PMU_ADC_REQ_SAR_MODE, &adc_result);

		bcmpmu_spa_pb->vbat = adc_result.conv;
		propval->intval = adc_result.conv;
		break;

	case POWER_SUPPLY_PROP_PRESENT:
		spa_bcmpmu_adc_read(bcmpmu, PMU_ADC_CHANN_BSI,
				PMU_ADC_REQ_SAR_MODE, &adc_result);

		bcmpmu_spa_pb->present = adc_result.raw;
		propval->intval = (adc_result.raw == 0x3FF) ? 0 : 1;
		break;

	case POWER_SUPPLY_PROP_CURRENT_NOW:
		pr_pb(FLOW, "%s: POWER_SUPPLY_PROP_CURRENT_NOW\n", __func__);
		break;

	case POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN:
		pr_pb(FLOW, "%s: POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN\n",
			__func__);
		break;

	default:
		ret = -ENODATA;
		break;
	}
	pr_pb(FLOW, "%s: prop = %d, ret = %d intval = %d\n",
			__func__,
			prop, ret, propval->intval);
	return ret;
}

int bcmpmu_post_spa_event_to_queue(struct bcmpmu59xxx *bcmpmu, u32 event,
	u32 param)
{
	struct bcmpmu_spa_pb *bcmpmu_spa_pb = bcmpmu->spa_pb_info;
	int ret = 0;
	int empty;
	pr_pb(FLOW, "%s: event = %d\n", __func__, event);

	/* BUG_ON(!bcmpmu_spa_pb); */
	if (!bcmpmu_spa_pb) {
		pr_pb(ERROR,
			"%s: spa_pb is null, and not ready yet\n",
			__func__);
		msleep(500);
		return -1;
	}

	if (SPA_FIFO_IS_FULL(bcmpmu_spa_pb->spa_fifo)) {
		pr_pb(ERROR, "%s: Queue full !!\n", __func__);
		return -ENOMEM;
	}
	mutex_lock(&bcmpmu_spa_pb->lock);
	empty = SPA_FIFO_IS_EMPTY(bcmpmu_spa_pb->spa_fifo);
	ret = __spa_fifo_add(&bcmpmu_spa_pb->spa_fifo, event, param);
	mutex_unlock(&bcmpmu_spa_pb->lock);

	if (empty && !ret)
		schedule_delayed_work(&bcmpmu_spa_pb->spa_work,
				SPA_WORK_SCHEDULE_DELAY);
	else
		pr_pb(ERROR, "%s: fails to schedule\n", __func__);

	return ret;
}
EXPORT_SYMBOL(bcmpmu_post_spa_event_to_queue);

int bcmpmu_post_spa_event(struct bcmpmu59xxx *bcmpmu, u32 event,
	u32 param)
{
	pr_pb(FLOW, "%s: event = %d\n", __func__, event);
	return spa_event_handler(event, (void *)param);
}
EXPORT_SYMBOL(bcmpmu_post_spa_event);

static int __map_to_spa_event(u32 event)
{
	int ret = -EINVAL;
	switch (event) {
	case PMU_CHRGR_EVT_MBTEMP:
		ret = SPA_EVT_TEMP;
		break;

	case PMU_ACCY_EVT_OUT_CHRGR_TYPE:
		ret = SPA_EVT_CHARGER;
		break;

	case PMU_CHRGR_EVT_MBOV:
		ret = SPA_EVT_OVP;
		break;

	case PMU_ACCY_EVT_OUT_USBOV:
		ret = SPA_EVT_OVP;
		break;

	case PMU_CHRGR_EVT_EOC:
		ret = SPA_EVT_EOC;
		break;

	case PMU_FG_EVT_CAPACITY:
		ret = SPA_EVT_CAPACITY;
		break;

	}
	return ret;
}

static void bcmpmu_spa_pb_work(struct work_struct *work)
{
	u32 event, data;
	int spa_evt;
	struct bcmpmu_spa_pb *bcmpmu_spa_pb;
	bcmpmu_spa_pb = container_of(work, struct bcmpmu_spa_pb, spa_work.work);
	BUG_ON(bcmpmu_spa_pb == NULL);
	mutex_lock(&bcmpmu_spa_pb->lock);
	if (!__spa_fifo_get(&bcmpmu_spa_pb->spa_fifo, &event, &data)) {
		pr_pb(FLOW, "%s: event = %d\n", __func__, event);
		spa_evt = __map_to_spa_event(event);
		BUG_ON(spa_evt < 0 || spa_evt >= SPA_EVT_MAX);
		if (PMU_FG_EVT_CAPACITY == event) {
			bcmpmu_spa_pb->capacity = data;
			pr_pb(FLOW, "%s: capacity = %d\n", __func__, data);
		}
		spa_event_handler(spa_evt, (void *)data);
	} else
		pr_pb(ERROR, "%s: Q empty\n", __func__);
	mutex_unlock(&bcmpmu_spa_pb->lock);
	if (!SPA_FIFO_IS_EMPTY(bcmpmu_spa_pb->spa_fifo))
		schedule_delayed_work(&bcmpmu_spa_pb->spa_work,
				SPA_WORK_SCHEDULE_DELAY);
}

static int bcmpmu_spa_pb_event_hndlr(struct notifier_block *nb,
		unsigned long event, void *para)
{
	struct bcmpmu_spa_pb *bcmpmu_spa_pb = NULL;
	u32 data;

	pr_pb(FLOW, "%s, event=%u\n",
			__func__, event);

	switch (event) {
	case PMU_ACCY_EVT_OUT_CHRGR_TYPE:
		bcmpmu_spa_pb = container_of(nb, struct bcmpmu_spa_pb,
				nb_chgr_det);
		BUG_ON(bcmpmu_spa_pb == NULL || para == NULL);
		bcmpmu_spa_pb->chg_type = *(enum bcmpmu_chrgr_type_t *)para;
		pr_pb(FLOW, "%s, chrgr type=%d\n",
			__func__, bcmpmu_spa_pb->chg_type);
		switch (bcmpmu_spa_pb->chg_type) {
		case PMU_CHRGR_TYPE_SDP:
			data = POWER_SUPPLY_TYPE_USB;
			break;

		case PMU_CHRGR_TYPE_CDP:
			data = POWER_SUPPLY_TYPE_USB_CDP;
			break;

		case PMU_CHRGR_TYPE_DCP:
			data = POWER_SUPPLY_TYPE_USB_DCP;
			break;

		default:
			data = POWER_SUPPLY_TYPE_BATTERY;
			break;
		}
#if 0
		bcmpmu_post_spa_event_to_queue(bcmpmu_spa_pb->bcmpmu,
			event, data);
#endif
		break;
	case PMU_ACCY_EVT_OUT_USB_IN:
		pr_pb(FLOW, "%s, PMU_ACCY_EVT_OUT_USB_IN\n",
					__func__);
		break;

	default:
		break;
		/* BUG(); */
	}
	return 0;
}

static void bcmpmu_spa_pb_isr(u32 irq, void *data)
{
	struct bcmpmu_spa_pb *bcmpmu_spa_pb = (struct bcmpmu_spa_pb *)data;
	pr_pb(FLOW, "%s, intr %d\n", __func__, irq);

	switch (irq) {
	case PMU_IRQ_MBTEMPHIGH:
		bcmpmu_spa_pb->temp_lmt = 1;
		bcmpmu_post_spa_event_to_queue(bcmpmu_spa_pb->bcmpmu,
			PMU_CHRGR_EVT_MBTEMP,
			POWER_SUPPLY_HEALTH_OVERHEAT);
		break;

	case PMU_IRQ_MBTEMPLOW:
		bcmpmu_spa_pb->temp_lmt = 1;
		bcmpmu_post_spa_event_to_queue(bcmpmu_spa_pb->bcmpmu,
			PMU_CHRGR_EVT_MBTEMP,
			POWER_SUPPLY_HEALTH_COLD);
		break;

	case PMU_IRQ_CHGERRDIS:
		if (bcmpmu_spa_pb->temp_lmt) {
			bcmpmu_post_spa_event_to_queue(bcmpmu_spa_pb->bcmpmu,
				PMU_CHRGR_EVT_MBTEMP,
				POWER_SUPPLY_HEALTH_GOOD);
			bcmpmu_spa_pb->temp_lmt = 0;
		}
		if (bcmpmu_spa_pb->mb_ov) {
			bcmpmu_post_spa_event_to_queue(bcmpmu_spa_pb->bcmpmu,
				PMU_CHRGR_EVT_MBOV, 0);
			bcmpmu_spa_pb->mb_ov = 0;
		}
		if (bcmpmu_spa_pb->usb_ov) {
			bcmpmu_post_spa_event_to_queue(bcmpmu_spa_pb->bcmpmu,
				PMU_ACCY_EVT_OUT_USBOV_DIS, 0);
			bcmpmu_spa_pb->usb_ov = 0;
		}
		break;

	case PMU_IRQ_MBOV:
		bcmpmu_spa_pb->mb_ov = 1;
		bcmpmu_post_spa_event_to_queue(bcmpmu_spa_pb->bcmpmu,
			PMU_CHRGR_EVT_MBOV, 1);
		break;

	case PMU_IRQ_MBOV_DIS:
		if (bcmpmu_spa_pb->mb_ov) {
			bcmpmu_post_spa_event_to_queue(bcmpmu_spa_pb->bcmpmu,
				PMU_CHRGR_EVT_MBOV, 0);
			bcmpmu_spa_pb->mb_ov = 0;
		}
		break;

	case PMU_IRQ_EOC:
		pr_pb(FLOW, "%s, intr %d\n", __func__, irq);
		break;
	default:
		break;
	}
}
#ifdef CONFIG_DEBUG_FS
static void bcmpmu_spa_pb_dbg_init(struct bcmpmu_spa_pb *bcmpmu_spa_pb)
{
	struct bcmpmu59xxx *bcmpmu = bcmpmu_spa_pb->bcmpmu;

	if (!bcmpmu->dent_bcmpmu) {
		pr_err("%s: Failed to initialize debugfs\n", __func__);
		return;
	}

	bcmpmu_spa_pb->dent_pb = debugfs_create_dir("spa_pb",
			bcmpmu->dent_bcmpmu);
	if (!bcmpmu_spa_pb->dent_pb)
		return;

	if (!debugfs_create_u32("dbg_mask", S_IWUSR | S_IRUSR,
		bcmpmu_spa_pb->dent_pb, &dbg_mask))
		goto err;

	if (!debugfs_create_u32("chg_type", S_IRUSR,
		bcmpmu_spa_pb->dent_pb, &bcmpmu_spa_pb->chg_type))
		goto err;
	if (!debugfs_create_u32("temp_lmt", S_IRUSR,
		bcmpmu_spa_pb->dent_pb, &bcmpmu_spa_pb->temp_lmt))
		goto err;
	if (!debugfs_create_u32("mb_ov", S_IRUSR,
		bcmpmu_spa_pb->dent_pb, &bcmpmu_spa_pb->mb_ov))
		goto err;
	if (!debugfs_create_u32("usb_ov", S_IRUSR,
		bcmpmu_spa_pb->dent_pb, &bcmpmu_spa_pb->usb_ov))
		goto err;
	if (!debugfs_create_u32("capacity", S_IRUSR,
		bcmpmu_spa_pb->dent_pb, &bcmpmu_spa_pb->capacity))
		goto err;
	if (!debugfs_create_u32("temp_adc", S_IRUSR,
		bcmpmu_spa_pb->dent_pb, &bcmpmu_spa_pb->temp_adc))
		goto err;
	if (!debugfs_create_u32("chgr_status", S_IRUSR,
		bcmpmu_spa_pb->dent_pb, &bcmpmu_spa_pb->chgr_status))
		goto err;
	if (!debugfs_create_u32("icc_fc", S_IRUSR,
		bcmpmu_spa_pb->dent_pb, &bcmpmu_spa_pb->icc_fc))
		goto err;
	if (!debugfs_create_u32("eoc", S_IRUSR,
		bcmpmu_spa_pb->dent_pb, &bcmpmu_spa_pb->eoc))
		goto err;
	if (!debugfs_create_u32("vbat", S_IRUSR,
		bcmpmu_spa_pb->dent_pb, &bcmpmu_spa_pb->vbat))
		goto err;
	if (!debugfs_create_u32("bat_present", S_IRUSR,
		bcmpmu_spa_pb->dent_pb, &bcmpmu_spa_pb->present))
		goto err;
	return;
err:
	debugfs_remove(bcmpmu_spa_pb->dent_pb);
}
#endif

static int bcmpmu_spa_pb_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct bcmpmu_spa_pb *bcmpmu_spa_pb;
	struct bcmpmu59xxx *bcmpmu = dev_get_drvdata(pdev->dev.parent);
	struct bcmpmu59xxx_spa_pb_pdata *pdata;
	pdata = (struct  bcmpmu59xxx_spa_pb_pdata *)pdev->dev.platform_data;
	pr_pb(INIT, "%s\n", __func__);
	BUG_ON(!(bcmpmu->flags & BCMPMU_SPA_EN) || !pdata);
	bcmpmu_spa_pb = kzalloc(sizeof(struct bcmpmu_spa_pb), GFP_KERNEL);
	if (bcmpmu_spa_pb == NULL) {
		dev_err(&pdev->dev, "failed to alloc mem: %d\n", ret);
		return -ENOMEM;
	}
	bcmpmu_spa_pb->bcmpmu = bcmpmu;
	bcmpmu->spa_pb_info = (void *)bcmpmu_spa_pb;

	mutex_init(&bcmpmu_spa_pb->lock);

	INIT_DELAYED_WORK(&bcmpmu_spa_pb->spa_work, bcmpmu_spa_pb_work);

	bcmpmu_spa_pb->chrgr.properties = bcmpmu_spa_pb_chrgr_props;
	bcmpmu_spa_pb->chrgr.num_properties =
			ARRAY_SIZE(bcmpmu_spa_pb_chrgr_props);
	bcmpmu_spa_pb->chrgr.get_property = bcmpmu_spa_pb_chrgr_get_property;
	bcmpmu_spa_pb->chrgr.set_property = bcmpmu_spa_pb_chrgr_set_property;
	bcmpmu_spa_pb->chrgr.name = pdata->chrgr_name;

	ret = power_supply_register(&pdev->dev, &bcmpmu_spa_pb->chrgr);
	if (ret)
		goto cghr_err;

	bcmpmu->register_irq(bcmpmu, PMU_IRQ_MBTEMPLOW,
		bcmpmu_spa_pb_isr, bcmpmu_spa_pb);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_MBTEMPHIGH,
		bcmpmu_spa_pb_isr, bcmpmu_spa_pb);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_CHGERRDIS,
		bcmpmu_spa_pb_isr, bcmpmu_spa_pb);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_MBOV,
		bcmpmu_spa_pb_isr, bcmpmu_spa_pb);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_MBOV_DIS,
		bcmpmu_spa_pb_isr, bcmpmu_spa_pb);
#if 0
    /* Don't use PMU_IRQ_USBOV/PMU_IRQ_USBOV_DIS interrupt
	   bacause PMU's USBOV threshold(6.6V/8V) can't meet SS spec(6.8V).
	   So, SS use MUIC USBOV function(6.8V) instead of PMU USBOV*/
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_USBOV,
	bcmpmu_spa_pb_isr, bcmpmu_spa_pb);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_USBOV_DIS,
	bcmpmu_spa_pb_isr, bcmpmu_spa_pb);
#endif

	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_MBTEMPLOW);
	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_MBTEMPHIGH);
	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_CHGERRDIS);
	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_MBOV);
	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_MBOV_DIS);
#if 0
	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_USBOV);
	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_USBOV_DIS);
#endif

	bcmpmu_spa_pb->nb_chgr_det.notifier_call = bcmpmu_spa_pb_event_hndlr;
	ret = bcmpmu_add_notifier(PMU_ACCY_EVT_OUT_CHRGR_TYPE,
		&bcmpmu_spa_pb->nb_chgr_det);
	if (ret) {
		pr_pb(ERROR, "%s, failed to register chrgr det notifier\n",
			__func__);
		goto err;
	}
	bcmpmu_spa_pb->nb_usbin.notifier_call = bcmpmu_spa_pb_event_hndlr;
	ret = bcmpmu_add_notifier(PMU_ACCY_EVT_OUT_USB_IN,
		&bcmpmu_spa_pb->nb_usbin);
	if (ret) {
		pr_pb(ERROR, "%s, failed to register chrgr det notifier\n",
			__func__);
		goto err;
	}

#ifdef CONFIG_DEBUG_FS
	bcmpmu_spa_pb_dbg_init(bcmpmu_spa_pb);
#endif

	return 0;

err:
	bcmpmu_remove_notifier(PMU_ACCY_EVT_OUT_CHRGR_TYPE,
			&bcmpmu_spa_pb->nb_chgr_det);
	bcmpmu_remove_notifier(PMU_ACCY_EVT_OUT_USB_IN,
			&bcmpmu_spa_pb->nb_usbin);
	power_supply_unregister(&bcmpmu_spa_pb->chrgr);
cghr_err:
	kfree(bcmpmu_spa_pb);
	return ret;
}

static int bcmpmu_spa_pb_remove(struct platform_device *pdev)
{
	struct bcmpmu59xxx *bcmpmu = dev_get_drvdata(pdev->dev.parent);
	struct bcmpmu_spa_pb *bcmpmu_spa_pb = bcmpmu->spa_pb_info;

	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_MBTEMPLOW);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_MBTEMPHIGH);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_CHGERRDIS);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_MBOV);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_MBOV_DIS);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_EOC);

	bcmpmu_remove_notifier(PMU_ACCY_EVT_OUT_CHRGR_TYPE,
			&bcmpmu_spa_pb->nb_chgr_det);
	bcmpmu_remove_notifier(PMU_ACCY_EVT_OUT_USB_IN,
			&bcmpmu_spa_pb->nb_usbin);
	power_supply_unregister(&bcmpmu_spa_pb->chrgr);
	kfree(bcmpmu_spa_pb);

	return 0;
}

static struct platform_driver bcmpmu_spa_pb_drv = {
	.driver = {
		.name = "bcmpmu_spa_pb",
	},
	.probe = bcmpmu_spa_pb_probe,
	.remove = bcmpmu_spa_pb_remove,
};

static int __init bcmpmu_spa_pb_init(void)
{
	return platform_driver_register(&bcmpmu_spa_pb_drv);
}
subsys_initcall_sync(bcmpmu_spa_pb_init);

static void __exit bcmpmu_spa_pb_exit(void)
{
	platform_driver_unregister(&bcmpmu_spa_pb_drv);
}
module_exit(bcmpmu_spa_pb_exit);

MODULE_DESCRIPTION("BECM PMIC SPA Piggyback driver");
MODULE_LICENSE("GPL");
