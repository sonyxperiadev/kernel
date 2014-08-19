/*****************************************************************************
 *  Copyright 2013 - 2014 Broadcom Corporation.  All rights reserved.
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
#include <linux/bug.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#ifdef CONFIG_HAS_WAKELOCK
#include <linux/wakelock.h>
#endif
#include <linux/mfd/bcmpmu59xxx.h>
#include <linux/mfd/bcmpmu59xxx_reg.h>
#include <linux/usb/bcm_hsotgctrl.h>
#include <linux/io.h>
#include <mach/io_map.h>
#include <mach/rdb_A0/brcm_rdb_hsotg_ctrl.h>
#ifdef CONFIG_DEBUG_FS
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#endif

#define BC_RETRIES	10
#define T2_RETRY	1
#define ACT_ACCY_PROBE	0
#define ACT_ACCY_IN	1
#define ACT_ACCY_RM	2

#define ACD_DELAY msecs_to_jiffies(100)
#define ACD_RETRY_DELAY msecs_to_jiffies(1000)
#define DISCONNECT_EVENT_TIME (HZ * 2)
#define REGL_BC "bc_vcc"

static int debug_mask = BCMPMU_PRINT_ERROR |
	BCMPMU_PRINT_INIT |  BCMPMU_PRINT_FLOW;

/*
 *Possible State MC
 * IDLE	-> DETECT
 * DETECT |-->CONNECTED
 *	|--> IDLE
 * CONNECTED ->	DISCONNECTED
 * DISCONNECTED -> IDLE
 *
 */
enum accyd_state {
	STATE_IDLE,
	STATE_DETECT,
	STATE_CONNECTED,
	STATE_DISCONNECTED,
};
struct accy_det {
	struct bcmpmu59xxx *bcmpmu;
	struct workqueue_struct *wq;
	struct delayed_work d_work;
	struct notifier_block chrgr_latch_event;
	struct notifier_block chrgr_rm_event;
	struct notifier_block id_change_event;
	struct regulator *regl_bc;
#ifdef CONFIG_HAS_WAKELOCK
	struct wake_lock wake_lock;
	struct wake_lock notify_wake_lock;
#endif
	int chrgr_type;
	int retry_cnt;
	atomic_t usb_allow_bc_detect;
	enum bcmpmu_bc_t bc;
	enum accyd_state state;
	u32 act;
	u32 xceiv_start;
	bool reschedule;
};
static atomic_t drv_init_done;

#define pr_acd(debug_level, args...) \
	do { \
		if (debug_mask & BCMPMU_PRINT_##debug_level) { \
			pr_info("[ACD]: "args); \
		} \
	} while (0)

static void enable_bc_clock(struct accy_det *accy_d, bool en)
{
	bcm_hsotgctrl_en_clock(en);
	pr_acd(VERBOSE, "======<%s> paccy clock %x\n"
		, __func__, en);
}

static void reset_bc(struct accy_det *accy_d)
{
	if (accy_d->bc == BCMPMU_BC_BB_BC12) {
		bcm_hsotgctrl_bc_reset();
		msleep(50);
	}
}

static int is_bb_detection_done(u32 status)
{
	int ret_status = 0;
	u32 type;
	if (status & BB_BC_STS_BC_DONE_MSK) {
		type = status & 0x7F;
		/*Make sure that only one type bit is set*/
		if (type && (type & (type - 1)))
			ret_status = 0;
		else
			ret_status = 1;
	}

	return ret_status;
}

static unsigned int get_bc_status(struct accy_det *accy_d)
{
	u32 status = 0;
	unsigned long st;
	struct bcmpmu59xxx *bcmpmu = accy_d->bcmpmu;

	if (accy_d->bc == BCMPMU_BC_PMU_BC12) {
		bcmpmu->read_dev(bcmpmu,
				PMU_REG_MBCCTRL5, (u8 *)&status);
		pr_acd(VERBOSE, " MBCCTRL5 %x\n", status);
		status &= MBCCTRL5_CHP_TYP_MASK;
		pr_acd(FLOW, "MBCCTRL5 %x  mask %x\n",
				status, MBCCTRL5_CHP_TYP_MASK);
		status = status >> MBCCTRL5_CHP_TYP_SHIFT;
		pr_acd(VERBOSE, " MBCCTRL5 %x shift % d\n",
				status, MBCCTRL5_CHP_TYP_SHIFT);
		return status;
	} else if (accy_d->bc == BCMPMU_BC_BB_BC12) {
		bcm_hsotgctrl_bc_status(&st);
		pr_acd(FLOW, "<%s> = status %x\n",
				__func__, (u32)st);
		if (is_bb_detection_done(st))
			return (u32)st;
	}

	return status;
}

static enum bcmpmu_chrgr_type_t get_chrgr_type(struct accy_det *accy_d,
		unsigned int bc_status)
{
	u8 chp_typ;
	enum bcmpmu_chrgr_type_t ret = PMU_CHRGR_TYPE_NONE;

	if (accy_d->bc == BCMPMU_BC_PMU_BC12) {
		chp_typ =  (bc_status & PMU_CHP_TYPE_MASK);
		if (chp_typ) {
			if (chp_typ == PMU_BC_DCP)
				ret = PMU_CHRGR_TYPE_DCP;
			else if (chp_typ == PMU_BC_SDP)
				ret = PMU_CHRGR_TYPE_SDP;
			else if (chp_typ == PMU_BC_CDP)
				ret = PMU_CHRGR_TYPE_CDP;
		}
	} else if (accy_d->bc == BCMPMU_BC_BB_BC12) {
		if (bc_status & BB_BC_STS_DCP_MSK)
			ret = PMU_CHRGR_TYPE_DCP;
		else if (bc_status & BB_BC_STS_SDP_MSK)
			ret = PMU_CHRGR_TYPE_SDP;
		else if (bc_status & BB_BC_STS_CDP_MSK)
			ret = PMU_CHRGR_TYPE_CDP;
		else if (bc_status & BB_BC_STS_PS2_MSK)
			ret = PMU_CHRGR_TYPE_PS2;
		else if (bc_status & BB_BC_STS_TYPE1_MSK)
			ret = PMU_CHRGR_TYPE_TYPE1;
		else if (bc_status & BB_BC_STS_TYPE2_MSK)
			ret = PMU_CHRGR_TYPE_TYPE2;
		else if (bc_status & BB_BC_STS_ACA_MSK)
			ret = PMU_CHRGR_TYPE_ACA_DOCK;
	}

	return ret;
}

static void accy_d_set_ldo_bit(struct accy_det *accy_d, int val)
{
	u8 status;
	accy_d->bcmpmu->read_dev(accy_d->bcmpmu,
			PMU_REG_MBCCTRL5, &status);
	if (val)
		status |= USB_DET_LDO_EN_MASK;
	else
		status &= ~USB_DET_LDO_EN_MASK;
	accy_d->bcmpmu->write_dev(accy_d->bcmpmu,
			PMU_REG_MBCCTRL5, status);
	pr_acd(VERBOSE, "###<%s> MBCCTRL5 %x\n", __func__, status);
}

static void bcmpmu_accy_set_pmu_BC12(struct bcmpmu59xxx *bcmpmu, int val)
{
	u8 reg;
	bcmpmu->read_dev(bcmpmu, PMU_REG_MBCCTRL5, &reg);
	pr_acd(VERBOSE, "###<%s> MBCCTRL5 %x\n", __func__, reg);
	if (val)
		reg |= MBCCTRL5_BC12_EN_MASK;
	else
		reg &= ~MBCCTRL5_BC12_EN_MASK;
	bcmpmu->write_dev(bcmpmu, PMU_REG_MBCCTRL5, reg);
	pr_acd(FLOW, "###<%s> MBCCTRL5 %x\n", __func__, reg);
}

static void bcdldo_cycle_power(struct accy_det *accy_d)
{
	accy_d_set_ldo_bit(accy_d, 0);
	/* Scope trace shows the decay time of the BCDLDO line
	 * is about 4ms. Hence a 5ms delay here to ensure proper
	 * power cycle.
	 */
	msleep(30);
	accy_d_set_ldo_bit(accy_d, 1);
}

static bool bcmpmu_supported_chrgr_usbid
		(struct accy_det *accy_d, int usb_id)
{
	bool id_valid = true;

	switch (usb_id) {
	case PMU_USB_ID_RID_A:
		id_valid = true;
		pr_acd(FLOW, "----PMU_USB_ID_RID_A\n");
		break;
	case PMU_USB_ID_RID_B:
		id_valid = true;
		pr_acd(FLOW, "----PMU_USB_ID_RID_B\n");
		break;
	case PMU_USB_ID_RID_C:
		id_valid = true;
		pr_acd(FLOW, "----PMU_USB_ID_RID_C\n");
		break;
	case PMU_USB_ID_FLOAT:
		id_valid = true;
		break;
	case PMU_USB_ID_GROUND:
	case PMU_USB_ID_RESERVED1:
	case PMU_USB_ID_RESERVED2:
	default:
		id_valid = false;
		break;
	}
	if (!id_valid)
		pr_acd(FLOW, "----Charger not supported\n");
	return id_valid;
}

static int bcmpmu_accy_detect_func(struct accy_det *accy_d)
{
	u32 vbus_status = 0, id_status = 0, bc_status;
	struct bcmpmu59xxx *bcmpmu = accy_d->bcmpmu;
	int chrgr_type = PMU_CHRGR_TYPE_NONE;
	bool id_check = true;
	int state = -1;
	static u8 type2;

	pr_acd(VERBOSE, "=== %s try %d\n",
			__func__, accy_d->retry_cnt);
	bcmpmu_usb_get(bcmpmu,
			BCMPMU_USB_CTRL_GET_SESSION_STATUS,
			(void *)&vbus_status);
	pr_acd(FLOW, "%s, vbus %d\n",
			__func__, vbus_status);

	if (accy_d->retry_cnt >= BC_RETRIES) {
		if (accy_d->chrgr_type)
			state = STATE_CONNECTED;
		else {
			pr_acd(ERROR, "---No accy detected\n");
			state = STATE_IDLE;
		}
		goto err;
	}

	bcmpmu_usb_get(bcmpmu,
			BCMPMU_USB_CTRL_GET_ID_VALUE,
			(void *)&id_status);
	id_check = bcmpmu_supported_chrgr_usbid(accy_d, id_status);
	pr_acd(VERBOSE, "%s, id %d id_check %d\n",
			__func__, id_status, id_check);
	if (!id_check) {
		pr_acd(ERROR, "---Err id_check nt valid id = %d\n", id_status);
		state = STATE_IDLE;
		goto err;
	}

	if ((id_status == PMU_USB_ID_FLOAT) && vbus_status) {
		bc_status = get_bc_status(accy_d);
		if (bc_status & 0x7F) {
			chrgr_type = get_chrgr_type(accy_d, bc_status);
			pr_acd(FLOW, "%s, bc=0x%x, chrgr=%d, retry=%d\n",
				__func__, bc_status, chrgr_type,
				accy_d->retry_cnt);
		}

		if ((!bc_status) || (!chrgr_type)) {
			if (bc_status)
				accy_d->chrgr_type = PMU_CHRGR_TYPE_MISC;
			goto err;
		}
		if ((chrgr_type == PMU_CHRGR_TYPE_TYPE2) &&
				(type2 < T2_RETRY)) {
			type2++;
		} else {
			type2 = 0;
			accy_d->chrgr_type = chrgr_type;
			state = STATE_CONNECTED;
		}
	} else if (vbus_status) {
		accy_d->chrgr_type = PMU_CHRGR_TYPE_ACA;
		state = STATE_CONNECTED;
	}
err:
	accy_d->retry_cnt++;
	return state;
}

void bcmpmu_enable_bc_regl(struct accy_det *accy_d, bool en)
{
	int ret;
	accy_d->regl_bc = regulator_get(NULL, REGL_BC);
	if (IS_ERR(accy_d->regl_bc)) {
		pr_acd(ERROR, "-----BC Detect regulator nt found\n");
		return;
	}

	if (en) {
		ret = regulator_enable(accy_d->regl_bc);
		if (ret) {
			pr_acd(ERROR, "Failed to enable BC Detect regulator\n");
		} else {
			usleep_range(2000, 5000);
			regulator_set_mode(accy_d->regl_bc,
						REGULATOR_MODE_IDLE);
		}
	} else {
		regulator_disable(accy_d->regl_bc);
	}

	regulator_put(accy_d->regl_bc);
}


static void bcmpmu_notify_charger_state(struct accy_det *accy_d)
{
	static int chrgr_t;
	pr_acd(VERBOSE, "===chrgr_type %d\n", accy_d->chrgr_type);
	if (chrgr_t != accy_d->chrgr_type) {
		bcmpmu_accy_chrgr_type_notify(accy_d->chrgr_type);
		chrgr_t = accy_d->chrgr_type;
	}
}

static void bcmpmu_accy_handle_state(struct accy_det *accy_d)
{
	pr_acd(VERBOSE, "=== %s state %d\n", __func__, accy_d->state);
	switch (accy_d->state) {

	case STATE_IDLE:
		cancel_delayed_work(&accy_d->d_work);
		accy_d->retry_cnt = 0;
		break;

	case STATE_CONNECTED:
		pr_acd(FLOW, "*** Charger Connected event\n");
		accy_d->retry_cnt = 0;
		if (accy_d->chrgr_type != PMU_CHRGR_TYPE_DCP)
			bcm_hsotgctrl_bc_enable_sw_ovwr();
		bcmpmu_notify_charger_state(accy_d);
		break;

	case STATE_DISCONNECTED:
		pr_acd(FLOW, "*** Charger disconnected event\n");
		accy_d->retry_cnt = 0;
		accy_d->state = STATE_IDLE;
		accy_d->chrgr_type = PMU_CHRGR_TYPE_NONE;
#ifdef CONFIG_HAS_WAKELOCK
		wake_lock_timeout(&accy_d->notify_wake_lock,
				DISCONNECT_EVENT_TIME);
#endif
		bcmpmu_notify_charger_state(accy_d);
		bcdldo_cycle_power(accy_d);
		reset_bc(accy_d);
		break;

	default:
		pr_acd(ERROR, "----Invalid state %d\n",
				accy_d->state);
		break;
	}

}

static int bcmpmu_accy_event_handler(struct notifier_block *nb,
		unsigned long event, void *para)
{
	int ret = 0;
	struct accy_det *accy_d;
	u32 id_status;
	pr_acd(FLOW, "=== %s event %ld\n", __func__, event);

	if (!atomic_read(&drv_init_done)) {
		pr_acd(ERROR, "%s: Driver not initialized yet\n", __func__);
		return -ENOMEM;
	}

	switch (event) {
	case PMU_ACCY_EVT_OUT_CHGDET_LATCH:
		accy_d = container_of(nb,
			struct accy_det, chrgr_latch_event);
		accy_d->act = ACT_ACCY_IN;
		accy_d->retry_cnt = 0;
		break;
	case PMU_ACCY_EVT_OUT_USB_RM:
		accy_d = container_of(nb,
			struct accy_det, chrgr_rm_event);
		accy_d->act = ACT_ACCY_RM;
		break;
	case PMU_ACCY_EVT_OUT_ID_CHANGE:
		accy_d = container_of(nb,
				struct accy_det, id_change_event);
		bcmpmu_usb_get(accy_d->bcmpmu,
				BCMPMU_USB_CTRL_GET_ID_VALUE,
				(void *)&id_status);
		if (id_status != PMU_USB_ID_FLOAT)
			accy_d->act = ACT_ACCY_IN;
		else
			ret = 1;
		accy_d->retry_cnt = 0;
		break;
	default:
		ret = -1;
		pr_acd(ERROR, "Invalid Event Received %s\n", __func__);
		break;
	}
	if (!ret) {
		if (queue_delayed_work(accy_d->wq,
					&accy_d->d_work, 0)) {
#ifdef CONFIG_HAS_WAKELOCK
			wake_lock(&accy_d->wake_lock);
#endif
		} else {
			cancel_delayed_work_sync(&accy_d->d_work);
			queue_delayed_work(accy_d->wq, &accy_d->d_work, 0);
		}
	}

	return 0;
}

static void bcmpmu_detect_wq(struct work_struct *work)
{
	int state;
	int bc_detect;
	struct accy_det *accy_d =
		container_of(work, struct accy_det, d_work.work);
	struct bcmpmu59xxx *bcmpmu = accy_d->bcmpmu;

	pr_acd(VERBOSE, "=== %s\n", __func__);

	accy_d->reschedule = false;
	if ((accy_d->act == ACT_ACCY_IN || accy_d->act == ACT_ACCY_PROBE) &&
			(accy_d->state != STATE_CONNECTED)) {

		bcmpmu_usb_get(bcmpmu,
				BCMPMU_USB_CTRL_ALLOW_BC_DETECT, &bc_detect);
		if (!bc_detect) {
			pr_acd(FLOW, "!!!! bc_detect is not set !!!\n");
			accy_d->reschedule =  true;
			goto exit;
		}

		enable_bc_clock(accy_d, true);
		accy_d->state = STATE_DETECT;
		state = bcmpmu_accy_detect_func(accy_d);
		enable_bc_clock(accy_d, false);

		if (state >= 0) {
			accy_d->state = state;
			pr_acd(VERBOSE, "*** accy updated state %d\n",
					accy_d->state);
			bcmpmu_accy_handle_state(accy_d);
		} else {
			if (accy_d->chrgr_type)
				bcmpmu_notify_charger_state(accy_d);
			reset_bc(accy_d);
			accy_d->reschedule =  true;
		}
	} else if (accy_d->act == ACT_ACCY_RM) {
		if ((accy_d->state == STATE_CONNECTED) ||
				accy_d->chrgr_type) {
			accy_d->state = STATE_DISCONNECTED;
			bcmpmu_accy_handle_state(accy_d);
		} else if (accy_d->state == STATE_DETECT) {
			accy_d->state = STATE_IDLE;
			bcmpmu_accy_handle_state(accy_d);
		}
	}

exit:
	if (accy_d->reschedule)
		queue_delayed_work(accy_d->wq,
				&accy_d->d_work, ACD_RETRY_DELAY);
	else {
#ifdef CONFIG_HAS_WAKELOCK
		wake_unlock(&accy_d->wake_lock);
#endif
	}

}

#if CONFIG_PM
static int bcmpmu_accy_detect_resume(struct platform_device *pdev)
{
	return 0;
}

static int bcmpmu_accy_detect_suspend(struct platform_device *pdev,
					pm_message_t state)
{
	struct bcmpmu59xxx *bcmpmu = dev_get_drvdata(pdev->dev.parent);
	struct accy_det *accy_d;
	accy_d = bcmpmu->accy_d;
	flush_delayed_work(&accy_d->d_work);
	return 0;
}

#else
#define bcmpmu_accy_detect_resume	NULL
#define bcmpmu_accy_detect_suspend	NULL
#endif

static int bcmpmu_accy_detect_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct bcmpmu59xxx *bcmpmu =  dev_get_drvdata(pdev->dev.parent);
	struct accy_det *accy_d;
	struct bcmpmu59xxx_platform_data *pdata ;
	u32 vbus_status;

	pdata = bcmpmu->pdata;

	pr_acd(INIT, "%s, called\n", __func__);

	accy_d = kzalloc(sizeof(struct accy_det), GFP_KERNEL);
	if (accy_d == NULL) {
		pr_acd(INIT, "%s: failed to alloc mem.\n", __func__);
		ret = -ENOMEM;
		return ret;
	}
	accy_d->bcmpmu = bcmpmu;
	bcmpmu->accy_d = accy_d;
	accy_d->bc = pdata->bc;
	accy_d->chrgr_latch_event.notifier_call = bcmpmu_accy_event_handler;

	ret = bcmpmu_add_notifier(PMU_ACCY_EVT_OUT_CHGDET_LATCH,
						&accy_d->chrgr_latch_event);

	accy_d->chrgr_rm_event.notifier_call = bcmpmu_accy_event_handler;
	ret |= bcmpmu_add_notifier(PMU_ACCY_EVT_OUT_USB_RM,
						&accy_d->chrgr_rm_event);
	if (ret) {
		pr_acd(INIT, "%s, failed on chrgr event notifier, err=%d\n",
				__func__, ret);
		goto err;
	}
	accy_d->id_change_event.notifier_call = bcmpmu_accy_event_handler;
	ret |= bcmpmu_add_notifier(PMU_ACCY_EVT_OUT_ID_CHANGE,
						&accy_d->id_change_event);
	if (ret) {
		pr_acd(INIT, "%s, failed on id change event notifier, err=%d\n",
				__func__, ret);
		goto err;
	}

	accy_d->wq = create_singlethread_workqueue("accy_d_wq");
	if (!accy_d->wq) {
		pr_acd(INIT, "failed to create accy_d wq\n");
		ret = -ENOMEM;
		goto err;
	}

	INIT_DELAYED_WORK(&accy_d->d_work, bcmpmu_detect_wq);

#ifdef CONFIG_HAS_WAKELOCK
	wake_lock_init(&accy_d->wake_lock,
		WAKE_LOCK_SUSPEND, "accy_detect");
	wake_lock_init(&accy_d->notify_wake_lock,
		WAKE_LOCK_SUSPEND, "notify_lock");
#endif
	/*MobC00367095*/
	bcmpmu_enable_bc_regl(accy_d, true);
	accy_d_set_ldo_bit(accy_d, 1);
	bcmpmu_accy_set_pmu_BC12(accy_d->bcmpmu, 1);
	reset_bc(accy_d);
	bcmpmu_usb_get(bcmpmu,
			BCMPMU_USB_CTRL_GET_SESSION_STATUS,
			(void *)&vbus_status);
	pr_acd(INIT, "%s vbus_status %d\n", __func__, vbus_status);
	if (vbus_status)
		queue_delayed_work(accy_d->wq, &accy_d->d_work, ACD_DELAY);

	atomic_set(&drv_init_done, 1);
	return ret;
err:
	kfree(accy_d);
	return ret;
}

static int bcmpmu_accy_detect_remove(struct platform_device *pdev)
{
	struct bcmpmu59xxx *bcmpmu = pdev->dev.platform_data;
	struct accy_det *accy_d = bcmpmu->accy_d;
	bcmpmu_remove_notifier(PMU_ACCY_EVT_OUT_CHGDET_LATCH,
				&accy_d->chrgr_latch_event);
	bcmpmu_remove_notifier(PMU_ACCY_EVT_OUT_USB_RM,
				&accy_d->chrgr_rm_event);
	cancel_delayed_work_sync(&accy_d->d_work);
	kfree(accy_d);
	return 0;
}

static struct platform_driver bcmpmu_accy_detect_driver = {
	.driver = {
		.name = "bcmpmu_accy_detect",
	},
	.probe = bcmpmu_accy_detect_probe,
	.remove = bcmpmu_accy_detect_remove,
	.suspend = bcmpmu_accy_detect_suspend,
	.resume = bcmpmu_accy_detect_resume,
};

static int __init bcmpmu_accy_detect_init(void)
{
	return platform_driver_register(&bcmpmu_accy_detect_driver);
}

module_init(bcmpmu_accy_detect_init);

static void __exit bcmpmu_accy_detect_exit(void)
{
	platform_driver_unregister(&bcmpmu_accy_detect_driver);
}

module_exit(bcmpmu_accy_detect_exit);

MODULE_DESCRIPTION("BCM PMIC accy detect driver");
MODULE_LICENSE("GPL");
