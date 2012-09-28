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
#include <linux/bug.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/wakelock.h>
#include <linux/notifier.h>
#include <linux/stringify.h>
#include <plat/pi_mgr.h>
#include <mach/pm.h>

#include <linux/mfd/bcmpmu.h>
#include <linux/usb/bcm_hsotgctrl.h>

#define BCMPMU_PRINT_ERROR (1U << 0)
#define BCMPMU_PRINT_INIT (1U << 1)
#define BCMPMU_PRINT_FLOW (1U << 2)
#define BCMPMU_PRINT_DATA (1U << 3)

static int debug_mask = BCMPMU_PRINT_ERROR | BCMPMU_PRINT_INIT;

#define pr_accy(debug_level, args...) \
	do { \
		if (debug_mask & BCMPMU_PRINT_##debug_level) { \
			pr_info(args); \
		} \
	} while (0)

#define PMU_BC_STATUS_CODE_SHIFT	0
#define PMU_BC_STATUS_DONE_SHIFT	8

#define BB_BC_STS_SDP_MSK	(1<<0)
#define BB_BC_STS_DCP_MSK	(1<<1)
#define BB_BC_STS_CDP_MSK	(1<<2)
#define BB_BC_STS_TYPE1_MSK	(1<<3)
#define BB_BC_STS_TYPE2_MSK	(1<<4)
#define BB_BC_STS_ACA_MSK	(1<<5)
#define BB_BC_STS_PS2_MSK	(1<<6)
#define BB_BC_STS_BC_DONE_MSK	(1<<7)

#define PMU_BC_STS_SDP_MSK	(1<<2)
#define PMU_BC_STS_DCP_MSK	(1<<1)
#define PMU_BC_STS_CDP_MSK	(1<<3)
#define PMU_BC_STS_TYPE1_MSK	(1<<5)
#define PMU_BC_STS_TYPE2_MSK	(1<<6)
#define PMU_BC_STS_ACA_MSK	(1<<7)
#define PMU_BC_STS_PS2_MSK	(1<<4)
#define PMU_BC_STS_BC_DONE_MSK	(1<<8)
/* TPROBE_MAX definition */
#define TPROBE_MAX_USEC			16000
#define TPROBE_MAX_MSB_MASK		0xC0
#define TPROBE_MAX_DATA(msb, lsb)	((((msb >> 6) & 0x3) << 8) | \
							(lsb & 0xFF))
#define TPROBE_MAX_GET_MSB(data)	(((data >> 8) & 0x3) << 6)
#define TPROBE_MAX_GET_LSB(data)	(data & 0xFF)

/* pattern to write before accessing PMU BC CTRL reg */
#define PMU_BC_CTRL_OVWR_PATTERN       0x5
struct accy_cb {
	void (*callback) (struct bcmpmu *, unsigned char event, void *, void *);
	void *clientdata;
};

enum bcmpmu_usb_det_state_t {
	USB_IDLE,
	USB_CONNECTED,
	USB_DETECT,
	USB_RETRY,
	USB_DISCONNECTED,
};

static const char *const chrgr_type_str[] = {
	[PMU_CHRGR_TYPE_NONE]	= __stringify(PMU_CHRGR_TYPE_NONE),
	[PMU_CHRGR_TYPE_SDP]	= __stringify(PMU_CHRGR_TYPE_SDP),
	[PMU_CHRGR_TYPE_CDP]	= __stringify(PMU_CHRGR_TYPE_CDP),
	[PMU_CHRGR_TYPE_DCP]	= __stringify(PMU_CHRGR_TYPE_DCP),
	[PMU_CHRGR_TYPE_TYPE1]	= __stringify(PMU_CHRGR_TYPE_TYPE1),
	[PMU_CHRGR_TYPE_TYPE2]	= __stringify(PMU_CHRGR_TYPE_TYPE2),
	[PMU_CHRGR_TYPE_PS2]	= __stringify(PMU_CHRGR_TYPE_PS2),
	[PMU_CHRGR_TYPE_ACA]	= __stringify(PMU_CHRGR_TYPE_ACA),
};

static int prev_chrgr_type;

#define log_charger_type(x)  \
	do {\
		if ((x) < PMU_CHRGR_TYPE_MAX) \
			pr_accy(FLOW, "Charger type = %s\n",\
						chrgr_type_str[x]);\
		else\
		pr_accy(FLOW, "Charger type = unknown\n");\
	} while (0)


/* Values in this table need to be revisited */
static int chrgr_curr_lmt[PMU_CHRGR_TYPE_MAX] = {
	[PMU_CHRGR_TYPE_NONE] = 0,
	[PMU_CHRGR_TYPE_SDP] = 0,
	[PMU_CHRGR_TYPE_CDP] = 1500,
	[PMU_CHRGR_TYPE_DCP] = 1500,
	[PMU_CHRGR_TYPE_TYPE1] = 1000,
	[PMU_CHRGR_TYPE_TYPE2] = 1000,
	[PMU_CHRGR_TYPE_PS2] = 0,
	[PMU_CHRGR_TYPE_ACA] = 1000,
};

struct bcmpmu_accy {
	struct bcmpmu *bcmpmu;
	const int *usb_id_map;
	int usb_id_map_len;
	wait_queue_head_t wait;
	struct delayed_work adp_work;
	struct delayed_work det_work;
	struct mutex lock;
	struct wake_lock wake_lock;
	struct pi_mgr_qos_node qos;
	struct accy_cb usb_cb;
	int adp_cal_done;
	enum bcmpmu_usb_det_state_t det_state;
	int otg_block_enabled;
	int adp_block_enabled;
	int adp_prob_comp;
	int adp_sns_comp;
	int retry_cnt;
	int poll_cnt;
	bool clock_en;
	enum bcmpmu_bc_t bc;
	int piggyback_chrg;
};
static struct bcmpmu_accy *bcmpmu_accy;
static void bc_detection(struct bcmpmu_accy *paccy);

static unsigned int get_bc_status(struct bcmpmu_accy *paccy)
{
	unsigned long status = 0;
	int ret;
	unsigned long temp = 0;
	struct bcmpmu *bcmpmu = paccy->bcmpmu;

	if ((paccy->bc == BCMPMU_BC_BB_BC11) ||
	    (paccy->bc == BCMPMU_BC_BB_BC12)) {
		bcm_hsotgctrl_bc_status(&status);
		return status;
	} else if (paccy->bc == BCMPMU_BC_PMU_BC12) {
		ret = bcmpmu->read_dev(bcmpmu,
				       PMU_REG_BC_STATUS_DONE,
				       (unsigned int *)&temp,
				       bcmpmu->regmap[PMU_REG_BC_STATUS_DONE].
				       mask);
		pr_accy(DATA, "%s: bc_done=0x%lx\n", __func__, temp);

		temp = temp >> bcmpmu->regmap[PMU_REG_BC_STATUS_DONE].shift;
		status |= temp << PMU_BC_STATUS_DONE_SHIFT;
		pr_accy(DATA, "%s: bc_status bc_done=0x%lx, value=0x%lx\n",
			__func__, temp, status);

		ret = bcmpmu->read_dev(bcmpmu,
				       PMU_REG_BC_STATUS_CODE,
				       (unsigned int *)&temp,
				       bcmpmu->regmap[PMU_REG_BC_STATUS_CODE].
				       mask);
		pr_accy(DATA, "%s: bc_code=0x%lX\n", __func__, temp);

		temp = temp >> bcmpmu->regmap[PMU_REG_BC_STATUS_CODE].shift;
		status |= temp << PMU_BC_STATUS_CODE_SHIFT;
		pr_accy(DATA, "%s: bc_status=0x%lX\n", __func__, status);
		return (unsigned int)status;
	} else
		return 0;
}

static enum bcmpmu_chrgr_type_t _get_charger_type(struct bcmpmu_accy *paccy,
						 unsigned int bc_status)
{
	enum bcmpmu_chrgr_type_t type;
	if (paccy->bc == BCMPMU_BC_BB_BC11) {
		if (bc_status & BB_BC_STS_SDP_MSK)
			type = PMU_CHRGR_TYPE_SDP;
		else if (bc_status & BB_BC_STS_CDP_MSK)
			type = PMU_CHRGR_TYPE_CDP;
		else if (bc_status & BB_BC_STS_DCP_MSK)
			type = PMU_CHRGR_TYPE_DCP;
		else
			type = PMU_CHRGR_TYPE_NONE;
	} else if (paccy->bc == BCMPMU_BC_BB_BC12) {
		if (bc_status & BB_BC_STS_DCP_MSK)
			type = PMU_CHRGR_TYPE_DCP;
		else if (bc_status & BB_BC_STS_SDP_MSK)
			type = PMU_CHRGR_TYPE_SDP;
		else if (bc_status & BB_BC_STS_CDP_MSK)
			type = PMU_CHRGR_TYPE_CDP;
		else if (bc_status & BB_BC_STS_PS2_MSK)
			type = PMU_CHRGR_TYPE_PS2;
		else if (bc_status & BB_BC_STS_TYPE1_MSK)
			type = PMU_CHRGR_TYPE_TYPE1;
		else if (bc_status & BB_BC_STS_TYPE2_MSK)
			type = PMU_CHRGR_TYPE_TYPE2;
		else if (bc_status & BB_BC_STS_ACA_MSK)
			type = PMU_CHRGR_TYPE_ACA;
		else
			type = PMU_CHRGR_TYPE_NONE;
	} else if (paccy->bc == BCMPMU_BC_PMU_BC12) {
		if (bc_status & PMU_BC_STS_DCP_MSK)
			type = PMU_CHRGR_TYPE_DCP;
		else if (bc_status & PMU_BC_STS_SDP_MSK)
			type = PMU_CHRGR_TYPE_SDP;
		else if (bc_status & PMU_BC_STS_CDP_MSK)
			type = PMU_CHRGR_TYPE_CDP;
		else if (bc_status & PMU_BC_STS_PS2_MSK)
			type = PMU_CHRGR_TYPE_PS2;
		else if (bc_status & PMU_BC_STS_TYPE1_MSK)
			type = PMU_CHRGR_TYPE_TYPE1;
		else if (bc_status & PMU_BC_STS_TYPE2_MSK)
			type = PMU_CHRGR_TYPE_TYPE2;
		else if (bc_status & PMU_BC_STS_ACA_MSK)
			type = PMU_CHRGR_TYPE_ACA;
		else
			type = PMU_CHRGR_TYPE_NONE;
	} else
		type = PMU_CHRGR_TYPE_NONE;
	return type;
}

static int is_detection_done(struct bcmpmu_accy *paccy, unsigned int status)
{
	int ret_status = 0;
	if ((paccy->bc == BCMPMU_BC_BB_BC11) ||
	    (paccy->bc == BCMPMU_BC_BB_BC12)) {

		if (status & BB_BC_STS_BC_DONE_MSK) {
			/*Make sure that only one type bit is set*/
			u32 type = status & 0x3F;
			if (type && ((type & (type - 1)) == 0))
				ret_status = 1;
		}
	}
	else if (paccy->bc == BCMPMU_BC_PMU_BC12)
		ret_status = (status & PMU_BC_STS_BC_DONE_MSK);
	else
		ret_status = 0;

	return ret_status;
}

static void enable_bc_clock(struct bcmpmu_accy *paccy, bool en)
{
	if ((paccy->bc == BCMPMU_BC_BB_BC11) ||
	    (paccy->bc == BCMPMU_BC_BB_BC12)) {
		bcm_hsotgctrl_en_clock(en);
		paccy->clock_en = en;
	}
}

static void reset_bc(struct bcmpmu_accy *paccy)
{
	if ((paccy->bc == BCMPMU_BC_BB_BC11) ||
	    (paccy->bc == BCMPMU_BC_BB_BC12))
		bcm_hsotgctrl_bc_reset();
}

static void bc_det_sts_clear(struct bcmpmu_accy *paccy)
{
	struct bcmpmu *bcmpmu = paccy->bcmpmu;
	u8 val;
	u8 mask;
	if ((paccy->bc == BCMPMU_BC_BB_BC11) ||
	    (paccy->bc == BCMPMU_BC_BB_BC12))
		return;

	val = (PMU_BC_CTRL_OVWR_PATTERN <<
	       bcmpmu->regmap[PMU_REG_BC_OVWR_KEY].shift);
	/* clear BC_DET_EN, statement not required
	 *kept for readablity
	 */
	val &= ~(1 << bcmpmu->regmap[PMU_REG_BC_DET_EN].shift);
	mask = bcmpmu->regmap[PMU_REG_BC_OVWR_KEY].mask |
	    bcmpmu->regmap[PMU_REG_BC_DET_EN].mask;

	bcmpmu->write_dev(bcmpmu, PMU_REG_BC_OVWR_KEY, val, mask);
	/* lock the register */
	bcmpmu->write_dev(bcmpmu, PMU_REG_BC_OVWR_KEY,
			  0, bcmpmu->regmap[PMU_REG_BC_OVWR_KEY].mask);

}

static void bc_det_restart(struct bcmpmu_accy *paccy)
{
	struct bcmpmu *bcmpmu = paccy->bcmpmu;
	u8 val;
	u8 mask;
	if ((paccy->bc == BCMPMU_BC_BB_BC11) ||
	    (paccy->bc == BCMPMU_BC_BB_BC12))
		return;

	val = (PMU_BC_CTRL_OVWR_PATTERN <<
	       bcmpmu->regmap[PMU_REG_BC_OVWR_KEY].shift);
	val |= (1 << bcmpmu->regmap[PMU_REG_BC_DET_EN].shift);
	mask = bcmpmu->regmap[PMU_REG_BC_OVWR_KEY].mask |
	    bcmpmu->regmap[PMU_REG_BC_DET_EN].mask;

	bcmpmu->write_dev(bcmpmu, PMU_REG_BC_OVWR_KEY, val, mask);
	/* lock the register */
	bcmpmu->write_dev(bcmpmu, PMU_REG_BC_OVWR_KEY,
			  0, bcmpmu->regmap[PMU_REG_BC_OVWR_KEY].mask);

}

int bcmpmu_add_notifier(u32 event_id, struct notifier_block *notifier)
{
	if (!bcmpmu_accy) {
		pr_accy(ERROR, "%s: BCMPMU Accy driver is not initialized\n",
			__func__);
		return -EAGAIN;
	}
	if (unlikely(event_id >= BCMPMU_EVENT_MAX)) {
		pr_accy(ERROR, "%s: Invalid event id\n", __func__);
		return -EINVAL;
	}
	return blocking_notifier_chain_register(
		&bcmpmu_accy->bcmpmu->event[event_id].notifiers, notifier);
}

EXPORT_SYMBOL_GPL(bcmpmu_add_notifier);

int bcmpmu_remove_notifier(u32 event_id, struct notifier_block *notifier)
{
	if (!bcmpmu_accy) {
		pr_accy(ERROR, "%s: BCMPMU accy driver is not initialized\n",
			__func__);
		return -EAGAIN;
	}
	if (unlikely(event_id >= BCMPMU_EVENT_MAX)) {
		pr_accy(ERROR, "%s: Invalid event id\n", __func__);
		return -EINVAL;
	}

	return blocking_notifier_chain_unregister(
		&bcmpmu_accy->bcmpmu->event[event_id].notifiers, notifier);
}

EXPORT_SYMBOL_GPL(bcmpmu_remove_notifier);

int bcmpmu_set_uas_det_mode(u32 det_mode)
{
	struct bcmpmu *bcmpmu;
	int ret;
	if (!bcmpmu_accy) {
		pr_accy(ERROR, "%s: BCMPMU accy driver is not initialized\n",
			__func__);
		return -EAGAIN;
	}
	if (unlikely(det_mode >= BCMPMU_UAS_MODE_MAX)) {
		pr_accy(ERROR, "%s: Invalid mode\n", __func__);
		return -EINVAL;
	}

	bcmpmu = bcmpmu_accy->bcmpmu;
	BUG_ON(bcmpmu == NULL);

	det_mode <<= bcmpmu->regmap[PMU_REG_UAS_DET_MODE].shift;
	det_mode &= bcmpmu->regmap[PMU_REG_UAS_DET_MODE].mask;
	ret = bcmpmu->write_dev(bcmpmu, PMU_REG_UAS_DET_MODE,
			  det_mode, bcmpmu->regmap[PMU_REG_UAS_DET_MODE].mask);
	return ret;
}

EXPORT_SYMBOL_GPL(bcmpmu_set_uas_det_mode);


int bcmpmu_get_uas_sw_grp(void)
{

	struct bcmpmu *bcmpmu;
	unsigned int val;
	int ret;
	if (!bcmpmu_accy) {
		pr_accy(ERROR, "%s: BCMPMU accy driver is not initialized\n",
			__func__);
		return -EAGAIN;
	}

	bcmpmu = bcmpmu_accy->bcmpmu;

	BUG_ON(bcmpmu == NULL);

	ret = bcmpmu->read_dev(bcmpmu, PMU_REG_UAS_SW_GRP, &val,
			bcmpmu->regmap[PMU_REG_UAS_SW_GRP].mask);

	val >>= bcmpmu->regmap[PMU_REG_UAS_SW_GRP].shift;
	return val;
}

EXPORT_SYMBOL_GPL(bcmpmu_get_uas_sw_grp);


int bcmpmu_set_uas_sw_grp(u32 sw_grp)
{
	struct bcmpmu *bcmpmu;
	int ret;
	if (!bcmpmu_accy) {
		pr_accy(ERROR, "%s: BCMPMU accy driver is not initialized\n",
			__func__);
		return -EAGAIN;
	}

	bcmpmu = bcmpmu_accy->bcmpmu;
	BUG_ON(bcmpmu == NULL);

	/*SW grp cannot be changed in HW det mode*/
	if ((sw_grp & bcmpmu->regmap[PMU_REG_UAS_SW_GRP].mask) >=
			BCMPMU_UAS_MODE_HW)
		return -EINVAL;

	sw_grp >>= bcmpmu->regmap[PMU_REG_UAS_SW_GRP].shift;

	ret = bcmpmu->write_dev(bcmpmu, PMU_REG_UAS_SW_GRP,
			  sw_grp, bcmpmu->regmap[PMU_REG_UAS_SW_GRP].mask);
	return ret;
}
EXPORT_SYMBOL_GPL(bcmpmu_set_uas_sw_grp);

int bcmpmu_set_uas_switch_enable(u32 uas_switch, bool enable)
{
	struct bcmpmu *bcmpmu;
	unsigned int val;
	int ret;
	u32 shift = uas_switch;
	u32 reg = PMU_REG_UASCTRL3;
	if (!bcmpmu_accy) {
		pr_accy(ERROR, "%s: BCMPMU accy driver is not initialized\n",
			__func__);
		return -EAGAIN;
	}

	if (unlikely(uas_switch >= UAS_SW_MAX)) {
		pr_accy(ERROR, "%s: Invalid SW\n", __func__);
		return -EINVAL;
	}
	bcmpmu = bcmpmu_accy->bcmpmu;
	BUG_ON(bcmpmu == NULL);

	if (uas_switch == UAS_SW7 ||
		uas_switch == UAS_SW8) {
		reg = PMU_REG_UASCTRL4;
		shift = uas_switch - UAS_SW7;
	}

	ret = bcmpmu->read_dev(bcmpmu,
				       reg,
				       &val,
				       bcmpmu->
				       regmap[reg].mask);
	if (enable)
		val |= 1 << shift;
	else
		val &= ~(1 << shift);

	ret = bcmpmu->write_dev(bcmpmu, reg,
			  val, bcmpmu->regmap[reg].mask);
	return ret;
}
EXPORT_SYMBOL_GPL(bcmpmu_set_uas_switch_enable);

static void send_usb_event(struct bcmpmu *pmu,
			   enum bcmpmu_event_t event, void *para)
{

	struct bcmpmu_accy *paccy = (struct bcmpmu_accy *)pmu->accyinfo;

	if (paccy->usb_cb.callback != NULL) {
		paccy->usb_cb.callback(paccy->bcmpmu, event, para,
				       paccy->usb_cb.clientdata);
		pr_accy(FLOW, "%s, event=%d, val=%p\n", __func__, event, para);
	}
	blocking_notifier_call_chain(&paccy->bcmpmu->event[event].notifiers,
				     event, para);

}

static char *get_supply_type_str(int chrgr_type)
{

	switch (chrgr_type) {
	case PMU_CHRGR_TYPE_SDP:
	case PMU_CHRGR_TYPE_CDP:
	case PMU_CHRGR_TYPE_ACA:
		return"usb";

	case PMU_CHRGR_TYPE_DCP:
	case PMU_CHRGR_TYPE_TYPE1:
	case PMU_CHRGR_TYPE_TYPE2:
	case PMU_CHRGR_TYPE_PS2:
		return "charger";

	default:
		break;
	}

	return NULL;
}

static void send_chrgr_event(struct bcmpmu *pmu,
			     enum bcmpmu_event_t event, void *para)
{
	struct power_supply *ps;
	char *chrgr_str;
	static char *last_chgr_str;
	union power_supply_propval propval;
	struct bcmpmu_accy *paccy = (struct bcmpmu_accy *)pmu->accyinfo;

	blocking_notifier_call_chain(&paccy->bcmpmu->event[event].notifiers,
				     event, para);
	if (paccy->bcmpmu->usb_accy_data.chrgr_type == PMU_CHRGR_TYPE_NONE) {
		chrgr_str = last_chgr_str;
		last_chgr_str = NULL;
	} else
		last_chgr_str = chrgr_str =
		get_supply_type_str(paccy->bcmpmu->usb_accy_data.chrgr_type);

	if (NULL == chrgr_str)
		return;
	if (paccy->piggyback_chrg)
		return;
	ps = power_supply_get_by_name(chrgr_str);
	if (ps == 0)
		return;
	if (event == BCMPMU_CHRGR_EVENT_CHGR_DETECTION) {
		pr_accy(FLOW, "%s, chrgr change, chrgr_type=0x%X\n",
			__func__, paccy->bcmpmu->usb_accy_data.chrgr_type);
		propval.intval = paccy->bcmpmu->usb_accy_data.chrgr_type;
		ps->set_property(ps, POWER_SUPPLY_PROP_TYPE, &propval);
		if (paccy->det_state == USB_CONNECTED)
			propval.intval = 1;
		else
			propval.intval = 0;
		ps->set_property(ps, POWER_SUPPLY_PROP_ONLINE, &propval);

	} else if (event == BCMPMU_CHRGR_EVENT_CHRG_CURR_LMT) {
		pr_accy(FLOW, "%s, charge current limit =0x%X\n",
			__func__, paccy->bcmpmu->usb_accy_data.max_curr_chrgr);
		propval.intval = paccy->bcmpmu->usb_accy_data.max_curr_chrgr;
		ps->set_property(ps, POWER_SUPPLY_PROP_CURRENT_NOW, &propval);
	}
	power_supply_changed(ps);
}

static void bcdldo_off(struct bcmpmu_accy *paccy)
{
	struct bcmpmu *bcmpmu = paccy->bcmpmu;

	bcmpmu->write_dev(bcmpmu, PMU_REG_MBCCTRL5_USB_DET_LDO_EN, false,
			  bcmpmu->regmap[PMU_REG_MBCCTRL5_USB_DET_LDO_EN].mask);
}

static void bcdldo_on(struct bcmpmu_accy *paccy)
{
	struct bcmpmu *bcmpmu = paccy->bcmpmu;

	bcmpmu->write_dev(bcmpmu, PMU_REG_MBCCTRL5_USB_DET_LDO_EN,
			  bcmpmu->regmap[PMU_REG_MBCCTRL5_USB_DET_LDO_EN].mask,
			  bcmpmu->regmap[PMU_REG_MBCCTRL5_USB_DET_LDO_EN].mask);
}

static void bcdldo_cycle_power(struct bcmpmu_accy *paccy)
{
	bcdldo_off(paccy);
	/* Scope trace shows the decay time of the BCDLDO line
	 * is about 4ms. Hence a 5ms delay here to ensure proper
	 * power cycle.
	 */
	usleep_range(4000, 6000);
	bcdldo_on(paccy);
}

static void bcmpmu_accy_isr(enum bcmpmu_irq irq, void *data)
{
	struct bcmpmu_accy *paccy = data;
	struct bcmpmu *bcmpmu = paccy->bcmpmu;
	pr_accy(FLOW, "%s interrupt = %d\n", __func__, irq);

	switch (irq) {
	case PMU_IRQ_VBUS_1V5_R:
		send_usb_event(bcmpmu, BCMPMU_USB_EVENT_SESSION_VALID, NULL);
		break;

	case PMU_IRQ_USBINS:
		if ((paccy->bc != BCMPMU_BC_BB_BC11) &&
		    (paccy->bc != BCMPMU_BC_BB_BC12))
			send_usb_event(bcmpmu, BCMPMU_USB_EVENT_IN, NULL);
		break;
	case PMU_IRQ_USBRM:
		send_usb_event(bcmpmu, BCMPMU_USB_EVENT_RM, NULL);

		if ((paccy->bc != BCMPMU_BC_BB_BC11) &&
		    (paccy->bc != BCMPMU_BC_BB_BC12)) {
			schedule_delayed_work(&paccy->det_work, 0);
		} else {
			/* The state m/c already has a check of UBPD status
			 * to determine a removal and move the state machine
			 * to IDLE. But control can sometimes reach the state
			 * m/c before the PMU UBPD bit has changed. Hence
			 * using a dummy DISCONNECTED state to force-reset
			 * the state m/c back to IDLE.
			 */
			paccy->det_state = USB_DISCONNECTED;
			bc_detection(paccy);
		}
		break;
	case PMU_IRQ_CHGDET_LATCH:
		if ((paccy->bc != BCMPMU_BC_BB_BC11) &&
		    (paccy->bc != BCMPMU_BC_BB_BC12)) {
			send_usb_event(bcmpmu, BCMPMU_USB_EVENT_CHGDET_LATCH,
				       NULL);
			schedule_delayed_work(&paccy->det_work,
					      msecs_to_jiffies(0));
		} else {
			bc_detection(paccy);
		}
		break;

	case PMU_IRQ_VBUS_1V5_F:
		if ((paccy->bc != BCMPMU_BC_BB_BC11) &&
		    (paccy->bc != BCMPMU_BC_BB_BC12))
			schedule_delayed_work(&paccy->det_work, 0);
		if ((get_supply_type_str(prev_chrgr_type) != NULL) &&
		(strcmp(get_supply_type_str(prev_chrgr_type), "usb") == 0))
			send_usb_event(bcmpmu,
					BCMPMU_USB_EVENT_SESSION_INVALID, NULL);

		break;

	case PMU_IRQ_VBUS_4V5_R:
		paccy->det_state = USB_IDLE;
		if ((paccy->bc != BCMPMU_BC_BB_BC11) &&
		    (paccy->bc != BCMPMU_BC_BB_BC12)) {
			send_usb_event(bcmpmu, BCMPMU_USB_EVENT_VBUS_VALID,
				       NULL);
			schedule_delayed_work(&paccy->det_work, 0);
		} else {
			bc_detection(paccy);
		}
		break;

	case PMU_IRQ_VBUS_4V5_F:
		if ((get_supply_type_str(prev_chrgr_type) != NULL) &&
		(strcmp(get_supply_type_str(prev_chrgr_type), "usb") == 0))
			send_usb_event(bcmpmu,
					BCMPMU_USB_EVENT_VBUS_INVALID, NULL);
		if ((paccy->bc != BCMPMU_BC_BB_BC11) &&
		    (paccy->bc != BCMPMU_BC_BB_BC12))
			schedule_delayed_work(&paccy->det_work, 0);
		break;

	case PMU_IRQ_IDCHG:
		send_usb_event(bcmpmu, BCMPMU_USB_EVENT_ID_CHANGE, NULL);
		break;

	case PMU_IRQ_ADP_CHANGE:
		send_usb_event(bcmpmu, BCMPMU_USB_EVENT_ADP_CHANGE, NULL);
		break;

	case PMU_IRQ_ADP_SNS_END:
		send_usb_event(bcmpmu, BCMPMU_USB_EVENT_ADP_SENSE_END, NULL);
		break;

	case PMU_IRQ_SESSION_END_VLD:
		send_usb_event(bcmpmu, BCMPMU_USB_EVENT_SESSION_END_VALID,
			       NULL);
		break;

	case PMU_IRQ_SESSION_END_INVLD:
		send_usb_event(bcmpmu, BCMPMU_USB_EVENT_SESSION_END_INVALID,
			       NULL);
		break;

	case PMU_IRQ_VBUS_OVERCURRENT:
		send_usb_event(bcmpmu, BCMPMU_USB_EVENT_VBUS_OVERCURRENT, NULL);
		break;

	case PMU_IRQ_CHGDET_TO:
		if ((paccy->bc == BCMPMU_BC_BB_BC11) ||
		    (paccy->bc == BCMPMU_BC_BB_BC12))
			bc_detection(paccy);
		break;

	case PMU_IRQ_FGC:
		blocking_notifier_call_chain(
			&paccy->bcmpmu->event[BCMPMU_FG_EVENT_FGC].notifiers,
			BCMPMU_FG_EVENT_FGC, NULL);
		break;

	case PMU_IRQ_JIG_UART_INS:
		pr_info("%s : PMU_IRQ_JIG_UART_INS\n",__func__);
		blocking_notifier_call_chain(
			&paccy->bcmpmu->event[BCMPMU_JIG_EVENT_UART].notifiers,
			BCMPMU_JIG_EVENT_UART, NULL);
		break;

	case PMU_IRQ_JIG_USB_INS:
		pr_info("%s : PMU_IRQ_JIG_USB_INS\n",__func__);
		blocking_notifier_call_chain(
			&paccy->bcmpmu->event[BCMPMU_JIG_EVENT_USB].notifiers,
			BCMPMU_JIG_EVENT_USB, NULL);
		break;

	case PMU_IRQ_RESUME_VBUS:
		blocking_notifier_call_chain(
			&paccy->bcmpmu->
			event[BCMPMU_CHRGR_EVENT_CHRG_RESUME_VBUS].notifiers,
			BCMPMU_CHRGR_EVENT_CHRG_RESUME_VBUS, NULL);
		break;

	default:
		break;
	}
}

#ifdef CONFIG_MFD_BCMPMU_DBG
static ssize_t
dbgmsk_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "debug_mask is %x\n", debug_mask);
}

static ssize_t
dbgmsk_set(struct device *dev, struct device_attribute *attr,
	   const char *buf, size_t count)
{
	unsigned long val = simple_strtoul(buf, NULL, 0);
	if (val > 0xFF || val == 0)
		return -EINVAL;
	debug_mask = val;
	return count;
}

static ssize_t
bcmpmu_dbg_show_bc_status(struct device *dev, struct device_attribute *attr,
			  char *buf)
{
	struct bcmpmu *bcmpmu = dev->platform_data;
	unsigned int val = get_bc_status(bcmpmu->accyinfo);
	return sprintf(buf, "%X\n", val);
}

static ssize_t
bcmpmu_dbg_usb_set(struct device *dev, struct device_attribute *attr,
		   const char *buf, size_t n)
{
	struct bcmpmu *bcmpmu = dev->platform_data;
	int ctrl, val;
	sscanf(buf, "%x %x", &ctrl, &val);
	bcmpmu->usb_set(bcmpmu, ctrl, val);
	pr_info("%s, ctrl=0x%X, val=0x%X\n", __func__, ctrl, val);
	return n;
}

static ssize_t
bcmpmu_dbg_usb_get(struct device *dev, struct device_attribute *attr,
		   const char *buf, size_t n)
{
	struct bcmpmu *bcmpmu = dev->platform_data;
	int ctrl, val;
	sscanf(buf, "%x", &ctrl);
	bcmpmu->usb_get(bcmpmu, ctrl, &val);
	pr_info("%s, ctrl=0x%X, val=0x%X\n", __func__, ctrl, val);
	return n;
}

static DEVICE_ATTR(dbgmsk, 0644, dbgmsk_show, dbgmsk_set);
static DEVICE_ATTR(bc_status, 0644, bcmpmu_dbg_show_bc_status, NULL);
static DEVICE_ATTR(usb_set, 0644, NULL, bcmpmu_dbg_usb_set);
static DEVICE_ATTR(usb_get, 0644, NULL, bcmpmu_dbg_usb_get);

static struct attribute *bcmpmu_accy_attrs[] = {
	&dev_attr_dbgmsk.attr,
	&dev_attr_bc_status.attr,
	&dev_attr_usb_get.attr,
	&dev_attr_usb_set.attr,
	NULL
};

static const struct attribute_group bcmpmu_accy_attr_group = {
	.attrs = bcmpmu_accy_attrs,
};
#endif

#define BC_MAX_RETRIES		3

static int get_charger_type(struct bcmpmu_accy *paccy)
{
	u32 bc_status;
	int charger_type = PMU_CHRGR_TYPE_NONE;

	bc_status = get_bc_status(paccy);
	pr_accy(FLOW, "%s: bc_status: 0x%x\n", __func__, bc_status);

	if (is_detection_done(paccy, bc_status)) {
		charger_type = _get_charger_type(paccy, bc_status);
		log_charger_type(charger_type);
	}

	return charger_type;
}

static enum bcmpmu_usb_type_t get_usb_type(enum bcmpmu_chrgr_type_t chrgr_type)
{
	enum bcmpmu_usb_type_t usb_type;

	switch (chrgr_type) {
	case PMU_CHRGR_TYPE_SDP:
		usb_type = PMU_USB_TYPE_SDP;
		break;
	case PMU_CHRGR_TYPE_CDP:
		usb_type = PMU_USB_TYPE_CDP;
		break;
	case PMU_CHRGR_TYPE_ACA:
		usb_type = PMU_USB_TYPE_ACA;
		break;
	default:
		usb_type = PMU_USB_TYPE_NONE;
		break;
	}

	return usb_type;
}

static void bc_detection(struct bcmpmu_accy *paccy)
{
	struct bcmpmu *bcmpmu = paccy->bcmpmu;
	int id;
	int vbus;
	enum bcmpmu_chrgr_type_t chrgr_type = PMU_USB_TYPE_NONE;
	enum bcmpmu_usb_type_t usb_type = PMU_CHRGR_TYPE_NONE;
	static bool type2_retry;

	bcmpmu_usb_get(bcmpmu, BCMPMU_USB_CTRL_GET_SESSION_STATUS, &vbus);
	bcmpmu_usb_get(bcmpmu, BCMPMU_USB_CTRL_GET_ID_VALUE, &id);

	pr_accy(FLOW, "%s: vbus=0x%x, id: 0x%x, state: %d\n", __func__, vbus,
		id, paccy->det_state);

	switch (paccy->det_state) {
	case USB_IDLE:
		if (vbus != 0 && id != PMU_USB_ID_GROUND) {
			/* Scheduling delay can cause control to reach here
			 * after BCDLDO is fully ON and BC clock OFF. BC clock
			 * gets enabled here. Hence, restart BC by
			 * power-cycling BCDLDO to get proper detection.
			 */

			/* In some cases, this state machine comes back to
			 * USB_IDLE state when clocks are already enabled
			 */
			if (!paccy->clock_en)
				enable_bc_clock(paccy, true);

			wake_lock(&paccy->wake_lock);
			bcdldo_cycle_power(paccy);
			reset_bc(paccy);
			paccy->retry_cnt = 0;
			paccy->det_state = USB_DETECT;
			type2_retry = false;
		}
		break;
	case USB_DETECT:
		chrgr_type = get_charger_type(paccy);
		if (chrgr_type == PMU_CHRGR_TYPE_NONE) {
			/* No valid charger type detected, retry */
			if (paccy->retry_cnt < BC_MAX_RETRIES) {
				paccy->retry_cnt++;
				bcdldo_cycle_power(paccy);
			} else {
				pr_accy(FLOW, "%s: max retries done\n",
					__func__);
				paccy->det_state = USB_IDLE;
				bcdldo_off(paccy);
			}
		} else if (chrgr_type == PMU_CHRGR_TYPE_TYPE2 &&
			   type2_retry == false) {
			type2_retry = true;
			/* If the USB cable is inserted very slowly, the BC
			 * hardware returns charger type as TYPE2. So to really
			 * confirm whether the charger type is type2, we need
			 * to restart detection. This is done by power cycling
			 * the BCDLDO.
			 */
			pr_accy(FLOW, "%s: re-checking PMU_CHRGR_TYPE_TYPE2\n",
				__func__);
			chrgr_type = PMU_CHRGR_TYPE_NONE;
			bcdldo_cycle_power(paccy);
		} else {
			/* Detection is successful */
			paccy->det_state = USB_CONNECTED;
			bcdldo_off(paccy);
		}
		break;
	case USB_CONNECTED:
	{
		bool stat;
		stat = bcmpmu->get_env_bit_status(bcmpmu, PMU_ENV_P_UBPD_INT);
		if (stat != true) {
			paccy->det_state = USB_IDLE;
		} else {
			usb_type = bcmpmu->usb_accy_data.usb_type;
			chrgr_type = bcmpmu->usb_accy_data.chrgr_type;
		}
		break;
	}
	case USB_DISCONNECTED:
		/* After USB_RM interrupt, the USBPD_INT may still be active
		 * when control reaches this state machine. Hence this new
		 * state to reset the state machine on USB removal. This will
		 * avoid having to add a undertain delay in the code.
		 */
		paccy->det_state = USB_IDLE;
		break;
	default:
		paccy->det_state = USB_IDLE;
		break;
	} /* end of state m/c */

	usb_type = get_usb_type(chrgr_type);
	pr_accy(FLOW, "%s, usb=%d, chrgr=%d, state: %d\n", __func__, usb_type,
		chrgr_type, paccy->det_state);

	if (paccy->det_state == USB_IDLE &&
			wake_lock_active(&paccy->wake_lock)) {
		wake_unlock(&paccy->wake_lock);
	}

	if ((usb_type < PMU_USB_TYPE_MAX) &&
	    (usb_type != bcmpmu->usb_accy_data.usb_type)) {
		bcmpmu->usb_accy_data.usb_type = usb_type;
		send_usb_event(paccy->bcmpmu,
			       BCMPMU_USB_EVENT_USB_DETECTION, &usb_type);
	}

	if ((chrgr_type < PMU_CHRGR_TYPE_MAX) &&
	    (chrgr_type != bcmpmu->usb_accy_data.chrgr_type)) {
		bcmpmu->usb_accy_data.chrgr_type = chrgr_type;
		bcmpmu->usb_accy_data.max_curr_chrgr =
		    chrgr_curr_lmt[chrgr_type];
		send_chrgr_event(paccy->bcmpmu,
				 BCMPMU_CHRGR_EVENT_CHGR_DETECTION,
				 &chrgr_type);
		send_chrgr_event(paccy->bcmpmu,
				 BCMPMU_CHRGR_EVENT_CHRG_CURR_LMT,
				 &chrgr_curr_lmt[chrgr_type]);
	}

	if (paccy->det_state == USB_IDLE && paccy->clock_en != 0)
		enable_bc_clock(paccy, false);
}

static void bc_det_work(struct work_struct *work)
{
	struct bcmpmu_accy *paccy =
	    container_of(work, struct bcmpmu_accy, det_work.work);

	bc_detection(paccy);
}

static void usb_det_work(struct work_struct *work)
{
	int ret;
	enum bcmpmu_chrgr_type_t chrgr_type = PMU_USB_TYPE_NONE;
	enum bcmpmu_usb_type_t usb_type = PMU_CHRGR_TYPE_NONE;
	unsigned int bc_status;
	int vbus_status;
	int id_status;
	struct bcmpmu_accy *paccy =
	    container_of(work, struct bcmpmu_accy, det_work.work);
	struct bcmpmu *bcmpmu = paccy->bcmpmu;

	ret = bcmpmu_usb_get(bcmpmu,
			     BCMPMU_USB_CTRL_GET_SESSION_STATUS,
			     (void *)&vbus_status);

	ret = bcmpmu_usb_get(bcmpmu,
			     BCMPMU_USB_CTRL_GET_ID_VALUE,
			     (void *)&id_status);

	pr_accy(FLOW, "%s, enter state=%d, vbus=0x%X\n", __func__,
		paccy->det_state, vbus_status);

	switch (paccy->det_state) {
	case USB_IDLE:
		enable_bc_clock(paccy, true);
		pr_accy(FLOW, "%s, enable clock\n", __func__);
		if (paccy->clock_en)
			mdelay(1);
		reset_bc(paccy);
		paccy->retry_cnt = 0;
		paccy->poll_cnt = 0;
		paccy->det_state = USB_DETECT;
		schedule_delayed_work(&paccy->det_work, msecs_to_jiffies(100));
		break;
	case USB_DETECT:
		if ((vbus_status != 0) && (id_status != PMU_USB_ID_GROUND)) {
			bc_status = get_bc_status(paccy);
			pr_accy(FLOW, "%s, bc_status=0x%X, retry=%d\n",
				__func__, bc_status, paccy->retry_cnt);
			if (is_detection_done(paccy, bc_status)) {
				chrgr_type = _get_charger_type(paccy,
							       bc_status);
				prev_chrgr_type = chrgr_type;
				log_charger_type(chrgr_type);
				if (chrgr_type != PMU_CHRGR_TYPE_NONE) {
					paccy->det_state = USB_CONNECTED;
					switch (chrgr_type) {
					case PMU_CHRGR_TYPE_SDP:
						usb_type = PMU_USB_TYPE_SDP;
						break;
					case PMU_CHRGR_TYPE_CDP:
						usb_type = PMU_USB_TYPE_CDP;
						break;
					case PMU_CHRGR_TYPE_ACA:
						usb_type = PMU_USB_TYPE_ACA;
						break;
					default:
						usb_type = PMU_USB_TYPE_NONE;
						break;
					}
				} else {
					paccy->det_state = USB_RETRY;
					schedule_delayed_work(&paccy->det_work,
							      msecs_to_jiffies
							      (0));
				}
			} else {
				paccy->poll_cnt++;
				if (paccy->poll_cnt > 10) {
					paccy->det_state = USB_RETRY;
					paccy->poll_cnt = 0;
				}
				schedule_delayed_work(&paccy->det_work,
						      msecs_to_jiffies(100));
			}
		} else {
			usb_type = PMU_USB_TYPE_NONE;
			chrgr_type = PMU_CHRGR_TYPE_NONE;
			paccy->det_state = USB_IDLE;
		}
		break;

	case USB_RETRY:
		paccy->retry_cnt++;
		if (paccy->retry_cnt < 3) {
			paccy->det_state = USB_DETECT;
			bc_det_sts_clear(paccy);
			bc_det_restart(paccy);
			schedule_delayed_work(&paccy->det_work,
					      msecs_to_jiffies(100));
		} else {
			pr_accy(ERROR, "%s, failed, retry times=%d\n",
				__func__, paccy->retry_cnt);
			usb_type = PMU_USB_TYPE_NONE;
			chrgr_type = PMU_CHRGR_TYPE_NONE;
			paccy->det_state = USB_IDLE;
		}
		break;

	case USB_CONNECTED:
		if (paccy->bcmpmu->get_env_bit_status(paccy->bcmpmu,
						      PMU_ENV_P_UBPD_INT) !=
		    true) {
			usb_type = PMU_USB_TYPE_NONE;
			chrgr_type = PMU_CHRGR_TYPE_NONE;
			paccy->det_state = USB_IDLE;
			if (chrgr_type == PMU_CHRGR_TYPE_DCP)
				bc_det_sts_clear(paccy);
		} else {
			usb_type = bcmpmu->usb_accy_data.usb_type;
			chrgr_type = bcmpmu->usb_accy_data.chrgr_type;
		}
		break;
	default:
		break;
	}

	pr_accy(FLOW, "%s, exit state=%d, usb=%d, chrgr=%d\n",
		__func__, paccy->det_state, usb_type, chrgr_type);

	if (usb_type != PMU_USB_TYPE_NONE && chrgr_type != PMU_CHRGR_TYPE_DCP)
		bc_det_sts_clear(paccy);

	if (paccy->det_state == USB_IDLE &&
			wake_lock_active(&paccy->wake_lock)) {
		wake_unlock(&paccy->wake_lock);
	}

	else if (paccy->det_state != USB_IDLE &&
			!wake_lock_active(&paccy->wake_lock)) {
		wake_lock(&paccy->wake_lock);
	}

	if ((paccy->det_state == USB_DETECT) || (paccy->det_state == USB_RETRY))
		return;

	/* temp solution. move the BD_DET_EN clear out
	 *of the condition <(paccy->clock_en != 0)>,
	 *this condition not meet and the BC_DET_EN
	 *never get called */
	if (chrgr_type != PMU_CHRGR_TYPE_DCP)
		bc_det_sts_clear(paccy);

	if (paccy->clock_en != 0) {
		enable_bc_clock(paccy, false);
		pr_accy(FLOW, "%s, disable clock\n", __func__);
	}
	if ((usb_type < PMU_USB_TYPE_MAX) &&
	    (usb_type != bcmpmu->usb_accy_data.usb_type)) {
		bcmpmu->usb_accy_data.usb_type = usb_type;
		send_usb_event(paccy->bcmpmu,
			       BCMPMU_USB_EVENT_USB_DETECTION, &usb_type);
	}
	if ((chrgr_type < PMU_CHRGR_TYPE_MAX) &&
	    (chrgr_type != bcmpmu->usb_accy_data.chrgr_type)) {
		bcmpmu->usb_accy_data.chrgr_type = chrgr_type;
		bcmpmu->usb_accy_data.max_curr_chrgr =
		    chrgr_curr_lmt[chrgr_type];
		send_chrgr_event(paccy->bcmpmu,
				 BCMPMU_CHRGR_EVENT_CHGR_DETECTION,
				 &chrgr_type);
		send_chrgr_event(paccy->bcmpmu,
				 BCMPMU_CHRGR_EVENT_CHRG_CURR_LMT,
				 &chrgr_curr_lmt[chrgr_type]);
	}
}

static void usb_adp_work(struct work_struct *work)
{
	struct bcmpmu_accy *data =
	    container_of(work, struct bcmpmu_accy, adp_work.work);
	if (data->adp_cal_done == 0) {
		data->adp_cal_done = 1;
		send_usb_event(data->bcmpmu,
			       BCMPMU_USB_EVENT_ADP_CALIBRATION_DONE, NULL);
	}
}

int bcmpmu_usb_set(struct bcmpmu *bcmpmu,
		   enum bcmpmu_usb_ctrl_t ctrl, unsigned long data)
{
	struct bcmpmu_accy *paccy = (struct bcmpmu_accy *)bcmpmu->accyinfo;
	int ret = 0, val;
	int reg;
	pr_accy(FLOW, "%s, ctrl=%d, val=0x%lX\n", __func__, ctrl, data);

	switch (ctrl) {
	case BCMPMU_USB_CTRL_CHRG_CURR_LMT:
		paccy->bcmpmu->usb_accy_data.max_curr_chrgr = (int)data;
		send_chrgr_event(paccy->bcmpmu,
				 BCMPMU_CHRGR_EVENT_CHRG_CURR_LMT,
				 &paccy->bcmpmu->usb_accy_data.max_curr_chrgr);
		break;

	case BCMPMU_USB_CTRL_VBUS_ON_OFF:
		if (data == 0)
			ret = bcmpmu->write_dev(bcmpmu, PMU_REG_OTG_VBUS_BOOST,
						0,
						bcmpmu->
						regmap[PMU_REG_OTG_VBUS_BOOST].
						mask);
		else
			ret = bcmpmu->write_dev(bcmpmu, PMU_REG_OTG_VBUS_BOOST,
						bcmpmu->
						regmap[PMU_REG_OTG_VBUS_BOOST].
						mask,
						bcmpmu->
						regmap[PMU_REG_OTG_VBUS_BOOST].
						mask);
		break;

	case BCMPMU_USB_CTRL_SET_VBUS_DEB_TIME:
		ret = bcmpmu->write_dev(bcmpmu, PMU_REG_ADP_COMP_DB_TM,
					data << bcmpmu->
					regmap[PMU_REG_ADP_COMP_DB_TM].shift,
					bcmpmu->regmap[PMU_REG_ADP_COMP_DB_TM].
					mask);
		break;

	case BCMPMU_USB_CTRL_SRP_VBUS_PULSE:
		if (data == 0)
			ret = bcmpmu->write_dev(bcmpmu, PMU_REG_OTG_VBUS_PULSE,
						0,
						bcmpmu->
						regmap[PMU_REG_OTG_VBUS_PULSE].
						mask);
		else
			ret = bcmpmu->write_dev(bcmpmu, PMU_REG_OTG_VBUS_PULSE,
						bcmpmu->
						regmap[PMU_REG_OTG_VBUS_PULSE].
						mask,
						bcmpmu->
						regmap[PMU_REG_OTG_VBUS_PULSE].
						mask);
		break;

	case BCMPMU_USB_CTRL_DISCHRG_VBUS:
		if (data == 0)
			ret =
			    bcmpmu->write_dev(bcmpmu, PMU_REG_OTG_VBUS_DISCHRG,
					      0,
					      bcmpmu->
					      regmap[PMU_REG_OTG_VBUS_DISCHRG].
					      mask);
		else
			ret =
			    bcmpmu->write_dev(bcmpmu, PMU_REG_OTG_VBUS_DISCHRG,
					      bcmpmu->
					      regmap[PMU_REG_OTG_VBUS_DISCHRG].
					      mask,
					      bcmpmu->
					      regmap[PMU_REG_OTG_VBUS_DISCHRG].
					      mask);
		break;

	case BCMPMU_USB_CTRL_START_STOP_ADP_SENS_PRB:
		if (data == 0)
			ret = bcmpmu->write_dev(bcmpmu, PMU_REG_ADP_SENSE,
						0,
						bcmpmu->
						regmap[PMU_REG_ADP_SENSE].mask);
		else {
			if (paccy->otg_block_enabled == 0) {
				ret =
				    bcmpmu->write_dev(bcmpmu,
						      PMU_REG_OTG_ENABLE,
						      bcmpmu->
						      regmap
						      [PMU_REG_OTG_ENABLE].mask,
						      bcmpmu->
						      regmap
						      [PMU_REG_OTG_ENABLE].
						      mask);
				paccy->otg_block_enabled = 1;
			}
			if (paccy->adp_block_enabled == 0) {
				ret =
				    bcmpmu->write_dev(bcmpmu,
						      PMU_REG_ADP_ENABLE,
						      bcmpmu->
						      regmap
						      [PMU_REG_ADP_ENABLE].mask,
						      bcmpmu->
						      regmap
						      [PMU_REG_ADP_ENABLE].
						      mask);
				paccy->adp_block_enabled = 1;
			}
			if (paccy->adp_sns_comp == 0) {
				ret =
				    bcmpmu->write_dev(bcmpmu,
						      PMU_REG_ADP_SNS_COMP,
						      bcmpmu->
						      regmap
						      [PMU_REG_ADP_SNS_COMP].
						      mask,
						      bcmpmu->
						      regmap
						      [PMU_REG_ADP_SNS_COMP].
						      mask);
				paccy->adp_sns_comp = 1;
			}
			ret = bcmpmu->write_dev(bcmpmu, PMU_REG_ADP_SENSE,
						bcmpmu->
						regmap[PMU_REG_ADP_SENSE].mask,
						bcmpmu->
						regmap[PMU_REG_ADP_SENSE].mask);
		}
		break;

	case BCMPMU_USB_CTRL_START_STOP_ADP_PRB:
		if (data == 0)
			ret = bcmpmu->write_dev(bcmpmu, PMU_REG_ADP_PRB,
						0,
						bcmpmu->regmap[PMU_REG_ADP_PRB].
						mask);
		else {
			if (paccy->otg_block_enabled == 0) {
				ret =
				    bcmpmu->write_dev(bcmpmu,
						      PMU_REG_OTG_ENABLE,
						      bcmpmu->
						      regmap
						      [PMU_REG_OTG_ENABLE].mask,
						      bcmpmu->
						      regmap
						      [PMU_REG_OTG_ENABLE].
						      mask);
				paccy->otg_block_enabled = 1;
			}
			if (paccy->adp_block_enabled == 0) {
				ret =
				    bcmpmu->write_dev(bcmpmu,
						      PMU_REG_ADP_ENABLE,
						      bcmpmu->
						      regmap
						      [PMU_REG_ADP_ENABLE].mask,
						      bcmpmu->
						      regmap
						      [PMU_REG_ADP_ENABLE].
						      mask);
				paccy->adp_block_enabled = 1;
			}
			if (paccy->adp_prob_comp == 0) {
				ret =
				    bcmpmu->write_dev(bcmpmu,
						      PMU_REG_ADP_PRB_COMP,
						      bcmpmu->
						      regmap
						      [PMU_REG_ADP_PRB_COMP].
						      mask,
						      bcmpmu->
						      regmap
						      [PMU_REG_ADP_PRB_COMP].
						      mask);
				paccy->adp_prob_comp = 1;
			}

			ret = bcmpmu->write_dev(bcmpmu, PMU_REG_ADP_PRB,
						bcmpmu->regmap[PMU_REG_ADP_PRB].
						mask,
						bcmpmu->regmap[PMU_REG_ADP_PRB].
						mask);
		}
		break;

	case BCMPMU_USB_CTRL_START_ADP_CAL_PRB:
		if (paccy->otg_block_enabled == 0) {
			ret = bcmpmu->write_dev(bcmpmu, PMU_REG_OTG_ENABLE,
						bcmpmu->
						regmap[PMU_REG_OTG_ENABLE].mask,
						bcmpmu->
						regmap[PMU_REG_OTG_ENABLE].
						mask);
			paccy->otg_block_enabled = 1;
		}
		if (paccy->adp_block_enabled == 0) {
			ret = bcmpmu->write_dev(bcmpmu, PMU_REG_ADP_ENABLE,
						bcmpmu->
						regmap[PMU_REG_ADP_ENABLE].mask,
						bcmpmu->
						regmap[PMU_REG_ADP_ENABLE].
						mask);
			paccy->adp_block_enabled = 1;
		}
		if (paccy->adp_prob_comp == 0) {
			ret = bcmpmu->write_dev(bcmpmu, PMU_REG_ADP_PRB_COMP,
						bcmpmu->
						regmap[PMU_REG_ADP_PRB_COMP].
						mask,
						bcmpmu->
						regmap[PMU_REG_ADP_PRB_COMP].
						mask);
			paccy->adp_prob_comp = 1;
		}
		ret = bcmpmu->write_dev(bcmpmu, PMU_REG_ADP_PRB_MOD,
					PMU_USB_ADP_MODE_CALIBRATE <<
					bcmpmu->regmap[PMU_REG_ADP_PRB_MOD].
					shift,
					bcmpmu->regmap[PMU_REG_ADP_PRB_MOD].
					mask);
		ret =
		    bcmpmu->write_dev(bcmpmu, PMU_REG_ADP_PRB,
				      bcmpmu->regmap[PMU_REG_ADP_PRB].mask,
				      bcmpmu->regmap[PMU_REG_ADP_PRB].mask);
		/* start timer */
		schedule_delayed_work(&paccy->adp_work, msecs_to_jiffies(15));
		break;

	case BCMPMU_USB_CTRL_SET_ADP_PRB_MOD:
		ret = bcmpmu->write_dev(bcmpmu, PMU_REG_ADP_PRB_MOD,
					data << bcmpmu->
					regmap[PMU_REG_ADP_PRB_MOD].shift,
					bcmpmu->regmap[PMU_REG_ADP_PRB_MOD].
					mask);
		break;

	case BCMPMU_USB_CTRL_SET_ADP_PRB_CYC_TIME:
		ret = bcmpmu->write_dev(bcmpmu, PMU_REG_ADP_PRB_CYC_TIME,
					data << bcmpmu->
					regmap[PMU_REG_ADP_PRB_CYC_TIME].shift,
					bcmpmu->
					regmap[PMU_REG_ADP_PRB_CYC_TIME].mask);
		break;

	case BCMPMU_USB_CTRL_SET_ADP_COMP_METHOD:
		ret = bcmpmu->write_dev(bcmpmu, PMU_REG_ADP_COMP_METHOD,
					data << bcmpmu->
					regmap[PMU_REG_ADP_COMP_METHOD].shift,
					bcmpmu->regmap[PMU_REG_ADP_COMP_METHOD].
					mask);
		break;

	case BCMPMU_USB_CTRL_SW_UP:
		if (data == 0) {
			ret = bcmpmu->write_dev(bcmpmu, PMU_REG_SWUP,
						0,
						bcmpmu->regmap[PMU_REG_SWUP].
						mask);
		} else {
			ret = bcmpmu->write_dev(bcmpmu, PMU_REG_SWUP,
						bcmpmu->regmap[PMU_REG_SWUP].
						mask,
						bcmpmu->regmap[PMU_REG_SWUP].
						mask);
		}
		break;
	case BCMPMU_USB_CTRL_TPROBE_MAX:
		if ((data >= 0) && (data <= TPROBE_MAX_USEC)) {
			val = (data * 2 * 1000) / 31250;
			bcmpmu->read_dev(bcmpmu, PMU_REG_OTGCTRL9, &reg,
					bcmpmu->regmap[PMU_REG_OTGCTRL9].mask);
			reg &= ~TPROBE_MAX_MSB_MASK;
			reg |= TPROBE_MAX_GET_MSB(val);
			ret = bcmpmu->write_dev(bcmpmu, PMU_REG_OTGCTRL9, reg,
					bcmpmu->regmap[PMU_REG_OTGCTRL9].mask);
			ret |= bcmpmu->write_dev(bcmpmu, PMU_REG_OTGCTRL10,
					TPROBE_MAX_GET_LSB(val),
					bcmpmu->regmap[PMU_REG_OTGCTRL10].mask);
		}
		break;

	default:
		ret = -EINVAL;
		break;
	}
	return ret;
}

EXPORT_SYMBOL_GPL(bcmpmu_usb_set);

int bcmpmu_usb_get(struct bcmpmu *bcmpmu,
		   enum bcmpmu_usb_ctrl_t ctrl, void *data)
{
	struct bcmpmu_accy *paccy = (struct bcmpmu_accy *)bcmpmu->accyinfo;
	unsigned int val, val1;
	int tprobe;
	int ret = -EINVAL;
	switch (ctrl) {
	case BCMPMU_USB_CTRL_TPROBE_MAX:
		ret = bcmpmu->read_dev(bcmpmu, PMU_REG_OTGCTRL9, &val,
				bcmpmu->regmap[PMU_REG_OTGCTRL9].mask);
		ret |= bcmpmu->read_dev(bcmpmu, PMU_REG_OTGCTRL10, &val1,
				bcmpmu->regmap[PMU_REG_OTGCTRL10].mask);
		tprobe = TPROBE_MAX_DATA(val, val1);
		val = ((tprobe * 31250) / (2 * 1000));
		break;
	case BCMPMU_USB_CTRL_GET_ADP_CHANGE_STATUS:
		ret = bcmpmu->read_dev(bcmpmu,
				       PMU_REG_ADP_STATUS_ATTACH_DET,
				       &val,
				       bcmpmu->
				       regmap[PMU_REG_ADP_STATUS_ATTACH_DET].
				       mask);
		val =
		    val >> bcmpmu->regmap[PMU_REG_ADP_STATUS_ATTACH_DET].shift;
		break;
	case BCMPMU_USB_CTRL_GET_ADP_SENSE_STATUS:
		ret = bcmpmu->read_dev(bcmpmu,
				       PMU_REG_ADP_STATUS_SNS_DET,
				       &val,
				       bcmpmu->
				       regmap[PMU_REG_ADP_STATUS_SNS_DET].mask);
		val = val >> bcmpmu->regmap[PMU_REG_ADP_STATUS_SNS_DET].shift;
		break;
	case BCMPMU_USB_CTRL_GET_ADP_PRB_RISE_TIMES:
		ret = bcmpmu->read_dev(bcmpmu,
				       PMU_REG_ADP_STATUS_RISE_TIMES_LSB,
				       &val,
				       bcmpmu->
				       regmap
				       [PMU_REG_ADP_STATUS_RISE_TIMES_LSB].
				       mask);
		val =
		    val >> bcmpmu->regmap[PMU_REG_ADP_STATUS_RISE_TIMES_LSB].
		    shift;
		if (ret != 0)
			break;
		ret = bcmpmu->read_dev(bcmpmu,
				       PMU_REG_ADP_STATUS_RISE_TIMES_MSB,
				       &val1,
				       bcmpmu->
				       regmap
				       [PMU_REG_ADP_STATUS_RISE_TIMES_MSB].
				       mask);
		val1 =
		    val1 >> bcmpmu->regmap[PMU_REG_ADP_STATUS_RISE_TIMES_MSB].
		    shift;
		val = val | val1 << 8;
		break;
	case BCMPMU_USB_CTRL_GET_VBUS_STATUS:
		ret = bcmpmu->read_dev(bcmpmu,
				       PMU_REG_OTG_STATUS_VBUS,
				       &val,
				       bcmpmu->regmap[PMU_REG_OTG_STATUS_VBUS].
				       mask);
		val = val >> bcmpmu->regmap[PMU_REG_OTG_STATUS_VBUS].shift;
		break;
	case BCMPMU_USB_CTRL_GET_SESSION_STATUS:
		ret = bcmpmu->read_dev(bcmpmu,
				       PMU_REG_OTG_STATUS_SESS,
				       &val,
				       bcmpmu->regmap[PMU_REG_OTG_STATUS_SESS].
				       mask);
		val = val >> bcmpmu->regmap[PMU_REG_OTG_STATUS_SESS].shift;
		break;
	case BCMPMU_USB_CTRL_GET_SESSION_END_STATUS:
		ret = bcmpmu->read_dev(bcmpmu,
				       PMU_REG_OTG_STATUS_SESS_END,
				       &val,
				       bcmpmu->
				       regmap[PMU_REG_OTG_STATUS_SESS_END].
				       mask);
		val = val >> bcmpmu->regmap[PMU_REG_OTG_STATUS_SESS_END].shift;
		break;
	case BCMPMU_USB_CTRL_GET_ID_VALUE:
		ret = bcmpmu->read_dev(bcmpmu,
				       PMU_REG_USB_STATUS_ID_CODE,
				       &val,
				       bcmpmu->
				       regmap[PMU_REG_USB_STATUS_ID_CODE].mask);
		val = val >> bcmpmu->regmap[PMU_REG_USB_STATUS_ID_CODE].shift;
		if ((paccy->usb_id_map) && (val < paccy->usb_id_map_len))
			val = paccy->usb_id_map[val];
		break;
		/* client for bcmpmu-accy notification can not register
		 *before accy driver get probed if, USB/Charger is
		 *plugged while system boot up, em driver
		 *will miss the first notification. So,
		 * BCMPMU_USB_CTRL_GET_CHRGR_TYPE is required
		 *to know the type explicitly
		 */
	case BCMPMU_USB_CTRL_GET_CHRGR_TYPE:
		ret = 0;
		val = bcmpmu->usb_accy_data.chrgr_type;
		break;

	case BCMPMU_USB_CTRL_GET_USB_TYPE:
		ret = 0;
		val = bcmpmu->usb_accy_data.usb_type;
		break;
	default:
		ret = -EINVAL;
		break;
	}
	if ((ret == 0) && (data != NULL))
		*(unsigned int *)data = val;

	pr_accy(FLOW, "%s, ctrl=%d, val=0x%X\n", __func__, ctrl,
		((data) ? (*(unsigned int *)data) : 0));
	return ret;
}

EXPORT_SYMBOL_GPL(bcmpmu_usb_get);

static int bcmpmu_register_usb_callback(struct bcmpmu *pmu,
					void (*callback) (struct bcmpmu *pmu,
							  unsigned char event,
							  void *, void *),
					void *data)
{
	struct bcmpmu_accy *paccy = (struct bcmpmu_accy *)pmu->accyinfo;
	int ret = -EINVAL;

	if (paccy != NULL) {
		paccy->usb_cb.callback = callback;
		paccy->usb_cb.clientdata = data;
		ret = 0;
	}

	return ret;
}

static int __devinit bcmpmu_accy_probe(struct platform_device *pdev)
{
	int ret;
	struct bcmpmu *bcmpmu = pdev->dev.platform_data;
	struct bcmpmu_accy *paccy;
	struct bcmpmu_platform_data *pdata = bcmpmu->pdata;
	int i;

	pr_accy(INIT, "%s, called\n", __func__);

	paccy = kzalloc(sizeof(struct bcmpmu_accy), GFP_KERNEL);
	if (paccy == NULL) {
		pr_accy(INIT, "%s: failed to alloc mem.\n", __func__);
		ret = -ENOMEM;
		goto err;
	}

	paccy->bcmpmu = bcmpmu;
	bcmpmu->accyinfo = paccy;
	bcmpmu_accy = paccy;
	bcmpmu->register_usb_callback = bcmpmu_register_usb_callback;

	bcmpmu->usb_get = bcmpmu_usb_get;
	bcmpmu->usb_set = bcmpmu_usb_set;
	bcmpmu->usb_accy_data.usb_type = PMU_USB_TYPE_NONE;
	bcmpmu->usb_accy_data.chrgr_type = PMU_CHRGR_TYPE_MAX;
	bcmpmu->usb_accy_data.max_curr_chrgr = 0;

	paccy->usb_cb.callback = NULL;
	paccy->usb_cb.clientdata = NULL;
	paccy->adp_cal_done = 0;
	paccy->det_state = USB_IDLE;
	paccy->usb_id_map = bcmpmu_get_usb_id_map(bcmpmu,
					&paccy->usb_id_map_len);
	paccy->otg_block_enabled = 0;
	paccy->adp_block_enabled = 0;
	paccy->adp_prob_comp = 0;
	paccy->adp_sns_comp = 0;
	paccy->bc = pdata->bc;

#ifdef CONFIG_CHARGER_BCMPMU_SPA
	paccy->piggyback_chrg = pdata->piggyback_chrg;
#endif

	INIT_DELAYED_WORK(&paccy->adp_work, usb_adp_work);
	if ((paccy->bc == BCMPMU_BC_BB_BC11) ||
	    (paccy->bc == BCMPMU_BC_BB_BC12)) {
		INIT_DELAYED_WORK(&paccy->det_work, bc_det_work);
	} else {
		INIT_DELAYED_WORK(&paccy->det_work, usb_det_work);
	}

	wake_lock_init(&paccy->wake_lock, WAKE_LOCK_SUSPEND, "usb_accy");

	/*Register QoS request to diable deep sleep when charger
		is connected*/
	pi_mgr_qos_add_request(&paccy->qos, "usb_accy",
				PI_MGR_PI_ID_ARM_CORE,
				PI_MGR_QOS_DEFAULT_VALUE);




	for (i = 0; i < BCMPMU_EVENT_MAX; i++) {
		bcmpmu->event[i].event_id = i;
		BLOCKING_INIT_NOTIFIER_HEAD(&bcmpmu->event[i].notifiers);
	}

	bcmpmu->register_irq(bcmpmu, PMU_IRQ_USBINS, bcmpmu_accy_isr, paccy);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_USBRM, bcmpmu_accy_isr, paccy);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_CHGDET_LATCH, bcmpmu_accy_isr,
			     paccy);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_CHGDET_TO, bcmpmu_accy_isr, paccy);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_VBUS_1V5_R, bcmpmu_accy_isr,
			     paccy);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_VBUS_4V5_R, bcmpmu_accy_isr,
			     paccy);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_VBUS_1V5_F, bcmpmu_accy_isr,
			     paccy);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_VBUS_4V5_F, bcmpmu_accy_isr,
			     paccy);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_IDCHG, bcmpmu_accy_isr, paccy);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_ADP_CHANGE, bcmpmu_accy_isr,
			     paccy);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_ADP_SNS_END, bcmpmu_accy_isr,
			     paccy);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_SESSION_END_VLD, bcmpmu_accy_isr,
			     paccy);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_SESSION_END_INVLD, bcmpmu_accy_isr,
			     paccy);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_VBUS_OVERCURRENT, bcmpmu_accy_isr,
			     paccy);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_FGC, bcmpmu_accy_isr, paccy);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_RESUME_VBUS, bcmpmu_accy_isr, paccy);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_JIG_UART_INS, bcmpmu_accy_isr, paccy);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_JIG_USB_INS, bcmpmu_accy_isr, paccy);

	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_USBINS);
	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_USBRM);
	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_CHGDET_LATCH);
	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_CHGDET_TO);
	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_VBUS_1V5_R);
	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_VBUS_4V5_R);
	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_VBUS_1V5_F);
	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_VBUS_4V5_F);
	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_IDCHG);
	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_ADP_CHANGE);
	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_ADP_SNS_END);
	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_SESSION_END_VLD);
	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_SESSION_END_INVLD);
	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_VBUS_OVERCURRENT);
	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_FGC);
	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_RESUME_VBUS);
	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_JIG_UART_INS);
	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_JIG_USB_INS);
	/* in 59039 BC happened from PMU and BB's BCDAVDD33 pin is floated
	 * need to set bc_iso_i in order to avoid any impact due to
	 * BCDAVDD33 as floated.
	*/
#if defined(CONFIG_MFD_BCM59039) || defined(CONFIG_MFD_BCM59042)
	/* Enable/Disable USB OTG clock aroudn bc_iso change */
	enable_bc_clock(paccy, true);
	bcm_hsotgctrl_set_bc_iso(true);
	enable_bc_clock(paccy, false);
#endif

#ifdef CONFIG_MFD_BCMPMU_DBG
	ret = sysfs_create_group(&pdev->dev.kobj, &bcmpmu_accy_attr_group);
#endif

	bc_det_sts_clear(paccy);
	bc_det_restart(paccy);

	if ((paccy->bc == BCMPMU_BC_BB_BC11) ||
	    (paccy->bc == BCMPMU_BC_BB_BC12))
		schedule_delayed_work(&paccy->det_work, 0);
	else
		schedule_delayed_work(&paccy->det_work, 500);
	return 0;

      err:
	kfree(paccy);
	return ret;
}

static int __devexit bcmpmu_accy_remove(struct platform_device *pdev)
{
	struct bcmpmu *bcmpmu = pdev->dev.platform_data;
	struct bcmpmu_accy *paccy = bcmpmu->accyinfo;

	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_USBINS);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_USBRM);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_CHGDET_LATCH);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_CHGDET_TO);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_VBUS_1V5_R);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_VBUS_4V5_R);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_VBUS_1V5_F);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_VBUS_4V5_F);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_IDCHG);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_ADP_CHANGE);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_ADP_SNS_END);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_SESSION_END_VLD);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_SESSION_END_INVLD);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_VBUS_OVERCURRENT);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_FGC);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_RESUME_VBUS);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_JIG_USB_INS);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_JIG_UART_INS);
	cancel_delayed_work_sync(&paccy->det_work);
	cancel_delayed_work_sync(&paccy->adp_work);
	wake_lock_destroy(&paccy->wake_lock);
	pi_mgr_qos_request_remove(&paccy->qos);

#ifdef CONFIG_MFD_BCMPMU_DBG
	sysfs_remove_group(&pdev->dev.kobj, &bcmpmu_accy_attr_group);
#endif
	kfree(paccy);
	return 0;
}

static struct platform_driver bcmpmu_accy_driver = {
	.driver = {
		   .name = "bcmpmu_accy",
		   },
	.probe = bcmpmu_accy_probe,
	.remove = __devexit_p(bcmpmu_accy_remove),
};

static int __init bcmpmu_accy_init(void)
{
	return platform_driver_register(&bcmpmu_accy_driver);
}

subsys_initcall(bcmpmu_accy_init);

static void __exit bcmpmu_accy_exit(void)
{
	platform_driver_unregister(&bcmpmu_accy_driver);
}

module_exit(bcmpmu_accy_exit);

MODULE_DESCRIPTION("BCM PMIC accy driver");
MODULE_LICENSE("GPL");
