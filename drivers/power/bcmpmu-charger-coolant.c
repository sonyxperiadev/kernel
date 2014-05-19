/*************************************************************************
 * linux/drivers/power/bcmpmu-charger-coolant.c
 *
 * simple charger current control, board code has to setup
 * platform_data being correctly configured
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *************************************************************************/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/delay.h>
#include <linux/mfd/bcmpmu59xxx.h>
#include <linux/bcmpmu-charger-coolant.h>
#include <linux/slab.h>
#include <mach/pinmux.h>
#include <linux/notifier.h>
#ifdef CONFIG_THERMAL
#include <linux/thermal.h>
#endif
#ifdef CONFIG_DEBUG_FS
#include <linux/debugfs.h>
#include <linux/uaccess.h>
#endif

#define MAX_ACTIVE_STATE	4
#define ACLD_MAX_WAIT_COUNT	10
#define TRIM_MARGIN_0		0

#ifdef DEBUG
#define DEBUG_MASK (BCMPMU_PRINT_ERROR | BCMPMU_PRINT_INIT |		\
			BCMPMU_PRINT_FLOW | BCMPMU_PRINT_DATA |		\
			BCMPMU_PRINT_WARNING | BCMPMU_PRINT_VERBOSE)
#else
#define DEBUG_MASK (BCMPMU_PRINT_ERROR | BCMPMU_PRINT_INIT |	\
			BCMPMU_PRINT_WARNING)
#endif

static u32 debug_mask = DEBUG_MASK;

#define pr_ccool(debug_level, args...) \
	do { \
		if (debug_mask & BCMPMU_PRINT_##debug_level) { \
			pr_info("[CCOOLANT]:"args); \
		} \
	} while (0)

struct bcmpmu_chrgr_trim_reg {
	u32 addr;
	u8 def_val;
	u8 saved_val;
};

struct bcmpmu_cc_data {
	struct bcmpmu59xxx *bcmpmu;
	struct bcmpmu_cc_pdata *pdata;
	struct thermal_cooling_device *chrgr_cdev;
	struct workqueue_struct *coolant_wq;
	struct delayed_work coolant_work;
	struct notifier_block usb_det_nb;
	struct notifier_block acld_nb;
	struct notifier_block chrgr_status_nb;
	enum bcmpmu_chrgr_type_t chrgr_type;
	bool dietemp_coolant_running;
	/* Variables for restoring charger state */
	int icc_fc_saved;
	int total_cur_saved;
	struct bcmpmu_chrgr_trim_reg *chrgr_trim_reg;
	int chrgr_trim_reg_sz;
	bool is_charging;
	bool charging_halted;/*to know whether this module disabled charging*/
	bool acld_algo_finished;
	u8 acld_wait_count;
	long curr_state;
	long max_state;
};

/* Forward declarations */
static void bcmpmu_coolant_restore_charger_state(struct bcmpmu_cc_data *cdata);
static void bcmpmu_coolant_store_charger_state(struct bcmpmu_cc_data *cdata);
static void bcmpmu_set_chrgr_trim_default(struct bcmpmu_cc_data *cdata);
static void bcmpmu_set_curr(struct bcmpmu_cc_data *cdata, unsigned long state);

static void bcmpmu_coolant_post_event(struct bcmpmu_cc_data *cdata)
{
	pr_ccool(FLOW, "%s Posting Status %d\n",
		__func__, cdata->dietemp_coolant_running);
	bcmpmu_call_notifier(cdata->bcmpmu,
	PMU_THEMAL_THROTTLE_STATUS, &cdata->dietemp_coolant_running);
}

int charger_cooling_get_level(struct thermal_cooling_device *cdev,
							int level_offset,
							u32 trip_current)
{
	struct bcmpmu_cc_data *cdata = cdev->devdata;
	int i;

	pr_ccool(VERBOSE, "%s:Trip_curr:%u\n", __func__, trip_current);
	for (i = level_offset; i < cdata->pdata->state_no; i++) {
		if (cdata->pdata->states[i] == trip_current)
			return i;
	}
	return -EINVAL;
}
EXPORT_SYMBOL_GPL(charger_cooling_get_level);

static void bcmpmu_coolant_restore_charger_state(struct bcmpmu_cc_data *cdata)
{
	struct bcmpmu59xxx *bcmpmu = cdata->bcmpmu;
	int ret = 0;
	u8 reg = 0;
	u8 num;

	pr_ccool(FLOW, "Restored Charger state\n");
	if (cdata->charging_halted && cdata->icc_fc_saved)
		cdata->charging_halted = false;
	/* Restore Charger registers to previous state */
	bcmpmu_set_icc_fc(bcmpmu, cdata->icc_fc_saved);
	pr_ccool(VERBOSE, "icc_fc=%d\n", cdata->icc_fc_saved);

	for (num = 0; num < cdata->chrgr_trim_reg_sz; num++) {
		ret = bcmpmu->write_dev(bcmpmu,
			cdata->chrgr_trim_reg[num].addr,
			cdata->chrgr_trim_reg[num].saved_val);
		if (ret)
			pr_ccool(ERROR, "Register[0x%08x] write Failed\n",
				cdata->chrgr_trim_reg[num].addr);

		ret = bcmpmu->read_dev(bcmpmu,
			cdata->chrgr_trim_reg[num].addr, &reg);
		if (ret)
			pr_ccool(ERROR, "Register[0x%08x] readback Failed\n",
				cdata->chrgr_trim_reg[num].addr);

		pr_ccool(VERBOSE, "Restored Register[0x%08x] = 0x%x\n",
			cdata->chrgr_trim_reg[num].addr, reg);
	}
}

static void bcmpmu_coolant_store_charger_state(struct bcmpmu_cc_data *cdata)
{
	struct bcmpmu59xxx *bcmpmu = cdata->bcmpmu;
	int ret = 0;
	u8 reg = 0;
	u8 num;

	/* Store Charger registers */
	pr_ccool(FLOW, "Saved Charger state\n");
		cdata->icc_fc_saved = bcmpmu_get_icc_fc(bcmpmu);

	for (num = 0; num < cdata->chrgr_trim_reg_sz; num++) {
		ret = bcmpmu->read_dev(bcmpmu,
		cdata->chrgr_trim_reg[num].addr, &reg);

		if (ret)
			pr_ccool(ERROR, "Register[0x%08x] read Failed\n",
				cdata->chrgr_trim_reg[num].addr);
		else {
			cdata->chrgr_trim_reg[num].saved_val = reg;
			pr_ccool(VERBOSE, "Stored Register[0x%08x] = 0x%x\n",
				cdata->chrgr_trim_reg[num].addr, reg);
		}
	}
	cdata->total_cur_saved = cdata->icc_fc_saved;
	if (cdata->chrgr_type != PMU_CHRGR_TYPE_SDP)
		cdata->total_cur_saved += bcmpmu_get_trim_curr(bcmpmu,
								TRIM_MARGIN_0);
	pr_ccool(VERBOSE, "Total cur saved=%d\n", cdata->total_cur_saved);
}

static void bcmpmu_set_chrgr_trim_default(struct bcmpmu_cc_data *cdata)
{
	struct bcmpmu59xxx *bcmpmu = cdata->bcmpmu;
	int i;
	pr_ccool(FLOW, "Set default values for trim registers\n");
	for (i = 0; i < cdata->chrgr_trim_reg_sz; i++)
		bcmpmu->write_dev(bcmpmu, cdata->chrgr_trim_reg[i].addr,
			cdata->chrgr_trim_reg[i].def_val);
}

static void bcmpmu_set_curr(struct bcmpmu_cc_data *cdata, unsigned long state)
{
	if (!cdata->pdata->states[state]) {
		cdata->charging_halted = true;
		pr_ccool(FLOW, "%s:Charging halted by coolant\n", __func__);
	} else if (cdata->charging_halted) {
		cdata->charging_halted = false;
	}
	pr_ccool(FLOW, "Set current to %d mA\n", cdata->pdata->states[state]);
	bcmpmu_set_icc_fc(cdata->bcmpmu, cdata->pdata->states[state]);
}

static void bcmpmu_coolant_set_optimal_curr(struct bcmpmu_cc_data *cdata,
							unsigned long state)
{
	if (!cdata->dietemp_coolant_running)
		bcmpmu_coolant_store_charger_state(cdata);

	if (cdata->total_cur_saved <
			cdata->pdata->states[state]) {
		pr_ccool(VERBOSE,
			"%s:Normal charging curr < curr to set\n", __func__);
		if (cdata->dietemp_coolant_running) {
			cdata->dietemp_coolant_running = false;
			bcmpmu_coolant_post_event(cdata);
			bcmpmu_coolant_restore_charger_state(cdata);
		}
	} else {
		if (!cdata->dietemp_coolant_running) {
			cdata->dietemp_coolant_running = true;
			bcmpmu_coolant_post_event(cdata);
			bcmpmu_set_chrgr_trim_default(cdata);
		}
		bcmpmu_set_curr(cdata, state);
	}
}

static void bcmpmu_coolant_work(struct work_struct *work)
{
	struct bcmpmu_cc_data *cdata =
		container_of(work, struct bcmpmu_cc_data, coolant_work.work);

	pr_ccool(FLOW, "%s\n", __func__);
	if (!cdata->acld_algo_finished) {
		if (cdata->acld_wait_count >= ACLD_MAX_WAIT_COUNT) {
			/* No Event from ACLD,
			 * so Forcefully set ACLD as finished*/
			pr_ccool(FLOW, "No ACLD FINISH event\n");
			cdata->acld_algo_finished = true;
			cdata->acld_wait_count = 0;
		} else {
			pr_ccool(FLOW, "Waiting for ACLD FINISH event\n");
			cdata->acld_wait_count++;
			queue_delayed_work(cdata->coolant_wq,
				&cdata->coolant_work, msecs_to_jiffies
					(cdata->pdata->coolant_poll_time));
		}
	}
	if (cdata->acld_algo_finished)
		bcmpmu_coolant_set_optimal_curr(cdata, cdata->curr_state);
}

static int bcmpmu_ccoolant_get_max_state(struct thermal_cooling_device *cdev,
			unsigned long *state)
{
	struct bcmpmu_cc_data *cdata = cdev->devdata;
	*state = cdata->max_state;
	pr_ccool(FLOW, "%s called, sending:%lu state\n",
				__func__, cdata->max_state);
	return 0;
}

static int bcmpmu_ccoolant_get_cur_state(struct thermal_cooling_device *cdev,
			unsigned long *state)
{
	struct bcmpmu_cc_data *cdata = cdev->devdata;
	*state = cdata->curr_state;
	pr_ccool(FLOW, "%s called, sending: state = %lu\n",
			__func__, cdata->curr_state);
	return 0;
}

static int bcmpmu_ccoolant_set_cur_state(struct thermal_cooling_device *cdev,
			unsigned long state)
{
	struct bcmpmu_cc_data *cdata = cdev->devdata;

	pr_ccool(FLOW, "%s:state = %lu\n", __func__, state);
	if (state < 0 || state > cdata->max_state)
		return -EINVAL;
	if (cdata->curr_state == state)
		return 0;

	cdata->curr_state = state;
	if ((cdata->is_charging || cdata->charging_halted) &&
					cdata->acld_algo_finished) {
		pr_ccool(VERBOSE, "%s called, state:%lu\tcurr = %u\n",
			__func__, state, cdata->pdata->states[state]);
		bcmpmu_coolant_set_optimal_curr(cdata, state);
	}
	return 0;
}

static struct thermal_cooling_device_ops chrgr_cooling_ops = {
	.get_max_state = bcmpmu_ccoolant_get_max_state,
	.set_cur_state = bcmpmu_ccoolant_set_cur_state,
	.get_cur_state = bcmpmu_ccoolant_get_cur_state,
};

static int bcmpmu_chrgr_coolant_event_handler(struct notifier_block *nb,
		unsigned long event, void *data)
{
	struct bcmpmu_cc_data *cdata;
	bool enable;
	pr_ccool(FLOW, "%s:event:%lu\n", __func__, event);
	switch (event) {
	case PMU_ACCY_EVT_OUT_CHRGR_TYPE:
		cdata = container_of(nb, struct bcmpmu_cc_data, usb_det_nb);
		cdata->chrgr_type = *(enum bcmpmu_chrgr_type_t *)data;
		if (cdata->chrgr_type == PMU_CHRGR_TYPE_NONE) {
			pr_ccool(FLOW, "%s:Charger Removed\n", __func__);
			cancel_delayed_work_sync(&cdata->coolant_work);
			cdata->acld_algo_finished = false;
			cdata->charging_halted = false;
			cdata->dietemp_coolant_running = false;
			cdata->acld_wait_count = 0;
		} else {
			pr_ccool(FLOW, "%s:Charger Connected\n", __func__);
			if (!bcmpmu_is_acld_supported
					(cdata->bcmpmu, cdata->chrgr_type)) {
				cdata->acld_algo_finished = true;
			}
		}
		break;
	case PMU_ACLD_EVT_ACLD_STATUS:
		enable = *(bool *)data;
		cdata = container_of(nb, struct bcmpmu_cc_data, acld_nb);
		if (enable) {
			cdata->acld_algo_finished = false;
			pr_ccool(FLOW, "%s:ACLD algo START Event Received\n",
						__func__);
			cdata->acld_wait_count = 0;
			/*For SDP, charging current is not set by ACLD algo
			 * Charging enable event will execute coolant work */
			if (cdata->chrgr_type == PMU_CHRGR_TYPE_SDP)
				return 0;
			queue_delayed_work(cdata->coolant_wq,
				&cdata->coolant_work, msecs_to_jiffies
					(cdata->pdata->coolant_poll_time));
		} else {
			cdata->acld_algo_finished = true;
			pr_ccool(FLOW, "%s:ACLD algo FINISH Event Received\n",
								__func__);
			if (cdata->chrgr_type == PMU_CHRGR_TYPE_SDP)
				return 0;
			queue_delayed_work(cdata->coolant_wq,
					&cdata->coolant_work, 0);
		}
		break;
	case PMU_CHRGR_EVT_CHRG_STATUS:
		enable = *(bool *)data;
		pr_ccool(FLOW, "%s: ==== chrgr_status %d\n",
			__func__, enable);
		cdata =
		container_of(nb, struct bcmpmu_cc_data, chrgr_status_nb);
		if (enable) {
			cdata->is_charging = true;
			pr_ccool(FLOW,
				"%s:Charging enabled\n", __func__);
			queue_delayed_work(cdata->coolant_wq,
					&cdata->coolant_work, 0);
		} else if (!enable) {
			pr_ccool(FLOW,
				"%s:Charging Disabled\n", __func__);
			cancel_delayed_work_sync(&cdata->coolant_work);
			cdata->acld_wait_count = 0;
			cdata->is_charging = false;
		}
		break;
	}
	return 0;
}

static int bcmpmu_ccoolant_register(struct bcmpmu_cc_data *cdata)
{

	cdata->chrgr_cdev = thermal_cooling_device_register(CHARGER_CDEV_NAME,
						cdata, &chrgr_cooling_ops);
	if (IS_ERR(cdata->chrgr_cdev)) {
		pr_ccool(ERROR,
		"charger cooling device registration failed:%ld\n",
					PTR_ERR(cdata->chrgr_cdev));
		return PTR_ERR(cdata->chrgr_cdev);
	}
	return 0;
}

static void bcmpmu_ccoolant_unregister(struct bcmpmu_cc_data *cdata)
{
	if (IS_ERR_OR_NULL(cdata->chrgr_cdev))
		return;

	thermal_cooling_device_unregister(cdata->chrgr_cdev);
}


static int bcmpmu_ccoolant_probe(struct platform_device *pdev)
{
	int ret = 0;
	int i;
	struct bcmpmu59xxx *bcmpmu = dev_get_drvdata(pdev->dev.parent);
	struct bcmpmu_cc_data *cdata;
	struct bcmpmu_cc_pdata *pdata;

	pr_ccool(FLOW, "%s\n", __func__);
	cdata = devm_kzalloc(&pdev->dev, sizeof(*cdata), GFP_KERNEL);
	if (cdata == NULL) {
		pr_ccool(ERROR, "%s failed to alloc mem\n", __func__);
		return -ENOMEM;
	}

	pdata =	(struct bcmpmu_cc_pdata *)pdev->dev.platform_data;
	cdata->pdata = pdata;
	cdata->bcmpmu = bcmpmu;
	cdata->chrgr_trim_reg_sz = cdata->pdata->chrgr_trim_reg_lut_sz;
	cdata->chrgr_trim_reg = devm_kzalloc(&pdev->dev,
			(sizeof(struct bcmpmu_chrgr_trim_reg) *
				cdata->chrgr_trim_reg_sz), GFP_KERNEL);
	if (cdata->chrgr_trim_reg == NULL) {
		pr_ccool(ERROR, "%s failed to alloc mem\n", __func__);
		ret = -ENOMEM;
	}
	for (i = 0; i < cdata->chrgr_trim_reg_sz; i++) {
		cdata->chrgr_trim_reg[i].addr =
			cdata->pdata->chrgr_trim_reg_lut[i].addr;
		cdata->chrgr_trim_reg[i].def_val =
			cdata->pdata->chrgr_trim_reg_lut[i].def_val;
	}

	cdata->acld_algo_finished = false;
	cdata->dietemp_coolant_running = false;
	cdata->is_charging = false;
	cdata->charging_halted = false;
	cdata->max_state = cdata->pdata->state_no - 1;
	cdata->curr_state = 0;
	cdata->icc_fc_saved = cdata->pdata->states[cdata->max_state];

	cdata->coolant_wq =
		create_singlethread_workqueue("bcmpmu_coolant_wq");
	if (IS_ERR_OR_NULL(cdata->coolant_wq)) {
		ret = PTR_ERR(cdata->coolant_wq);
		pr_ccool(ERROR, "%s Failed to create WQ\n", __func__);
		goto error;
	}
	INIT_DELAYED_WORK(&cdata->coolant_work, bcmpmu_coolant_work);

	cdata->usb_det_nb.notifier_call = bcmpmu_chrgr_coolant_event_handler;
	ret = bcmpmu_add_notifier(PMU_ACCY_EVT_OUT_CHRGR_TYPE,
					&cdata->usb_det_nb);
	if (ret) {
		pr_ccool(ERROR,
			"%s Failed to add usb notifier\n", __func__);
		goto destroy_wq;
	}

	cdata->acld_nb.notifier_call = bcmpmu_chrgr_coolant_event_handler;
	ret = bcmpmu_add_notifier(PMU_ACLD_EVT_ACLD_STATUS,
					&cdata->acld_nb);

	if (ret) {
		pr_ccool(ERROR,
			"%s Failed to add acld notifier\n", __func__);
		goto unreg_usb_det_nb;
	}

	cdata->chrgr_status_nb.notifier_call =
				bcmpmu_chrgr_coolant_event_handler;
	ret = bcmpmu_add_notifier(PMU_CHRGR_EVT_CHRG_STATUS,
				&cdata->chrgr_status_nb);
	if (ret) {
		pr_ccool(ERROR,
			"%s Failed to add chrgr st notifier\n", __func__);
		goto unreg_acld_nb;
	}
	/* Register  as coolant */
	ret = bcmpmu_ccoolant_register(cdata);
	if (ret) {
		pr_ccool(ERROR, "%s Failed to register coolant\n", __func__);
		goto unreg_chrgr_nb;
	}
	return 0;
unreg_chrgr_nb:
	bcmpmu_remove_notifier(PMU_CHRGR_EVT_CHRG_STATUS,
			&cdata->chrgr_status_nb);
unreg_acld_nb:
	bcmpmu_remove_notifier(PMU_ACLD_EVT_ACLD_STATUS,
			&cdata->acld_nb);
unreg_usb_det_nb:
	bcmpmu_remove_notifier(PMU_ACCY_EVT_OUT_CHRGR_TYPE,
				&cdata->usb_det_nb);
destroy_wq:
	destroy_workqueue(cdata->coolant_wq);
error:
	return ret;
}

static int bcmpmu_ccoolant_remove(struct platform_device *pdev)
{
	struct bcmpmu_cc_data *cdata = platform_get_drvdata(pdev);
	pr_ccool(FLOW, "%s\n", __func__);
	bcmpmu_remove_notifier(PMU_CHRGR_EVT_CHRG_STATUS,
					&cdata->chrgr_status_nb);
	bcmpmu_remove_notifier(PMU_ACLD_EVT_ACLD_STATUS,
					&cdata->acld_nb);
	bcmpmu_remove_notifier(PMU_ACCY_EVT_OUT_CHRGR_TYPE,
					&cdata->usb_det_nb);
	destroy_workqueue(cdata->coolant_wq);
	bcmpmu_ccoolant_unregister(cdata);
	return 0;
}

static struct platform_driver bcmpmu_ccoolant_drv = {
	.driver = {
		.owner = THIS_MODULE,
		.name = "bcmpmu_charger_coolant",
	},
	.probe = bcmpmu_ccoolant_probe,
	.remove = bcmpmu_ccoolant_remove,
};

static int __init bcmpmu_ccoolant_init(void)
{
	return platform_driver_register(&bcmpmu_ccoolant_drv);
}
module_init(bcmpmu_ccoolant_init);

static void __exit bcmpmu_ccoolant_exit(void)
{
	platform_driver_unregister(&bcmpmu_ccoolant_drv);
}
module_exit(bcmpmu_ccoolant_exit);

MODULE_DESCRIPTION("Broadcom PMU Charger Coolant Driver");
MODULE_LICENSE("GPL");

