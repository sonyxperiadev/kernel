/*****************************************************************************
 *  Copyright 2001 - 20012 Broadcom Corporation.  All rights reserved.
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
#include <linux/stringify.h>
#include <linux/mfd/bcmpmu59xxx.h>
#include <linux/mfd/bcmpmu59xxx_reg.h>
#include <linux/usb/bcm_hsotgctrl.h>
#include <plat/pi_mgr.h>
#include <mach/pm.h>
#include <plat/kona_reset_reason.h>
#include <linux/io.h>
#include <mach/io_map.h>
#include <mach/rdb_A0/brcm_rdb_hsotg_ctrl.h>
#ifdef CONFIG_DEBUG_FS
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#endif

/* TPROBE_MAX definition */
#define TPROBE_MAX_USEC			16000
#define TPROBE_MAX_MSB_MASK		0xC0
#define TPROBE_MAX_DATA(msb, lsb)	((((msb >> 6) & 0x3) << 8) | \
		(lsb & 0xFF))
#define TPROBE_MAX_GET_MSB(data)	(((data >> 8) & 0x3) << 6)
#define TPROBE_MAX_GET_LSB(data)	(data & 0xFF)

#define PMU_USB_FC_CC_84mA	0
#define PMU_USB_FC_CC_400mA	2

#define PMU_USB_FC_CC_MIN	0
#define PMU_USB_FC_CC_MAX	(ARRAY_SIZE(bcmpmu_pmu_curr_acld_table) - 1)

#define PMU_USB_CC_TRIM_MIN	0
#define PMU_USB_CC_TRIM_MAX	0xF

#define PMU_CHRGR_CURR_DEFAULT	500
#define MAX_EVENTS		100

#ifdef CONFIG_DEBUG_FS
#define DEBUG_FS_PERMISSIONS	(S_IRUSR | S_IWUSR)
#endif

#define to_bcmpmu_accy_data(ptr, mem)	container_of((ptr), \
		struct bcmpmu_accy_data, mem)

struct bcmpmu_accy_event {
	struct list_head node;
	int event;
	int *data;
};

struct bcmpmu_accy_irq_evt_map {
	u32 irq;
	u32 event;
};

struct bcmpmu_accy_data {
	struct bcmpmu59xxx *bcmpmu;
	struct bcmpmu59xxx_accy_pdata *pdata;
	struct workqueue_struct *wq;
	struct delayed_work evt_notify_work;
	struct notifier_block chrgr_type_nb;
	struct bcmpmu_accy_event event_pool[MAX_EVENTS];
	struct list_head event_pending_list;
	struct list_head event_free_list;
	spinlock_t accy_lock;
#ifdef CONFIG_HAS_WAKELOCK
	struct wake_lock accy_wake_lock;
#endif
#ifdef CONFIG_KONA_PI_MGR
	struct pi_mgr_qos_node qos_client;
#endif
	enum bcmpmu_chrgr_type_t chrgr_type;
	enum bcmpmu_chrgr_type_t chrgr_type_prev;
	enum bcmpmu_usb_id_lvl_t pmu_usb_id;
	bool pmu_session_a;
	int charging_curr;
	int otg_block_enabled;
	int adp_block_enabled;
	int adp_sns_comp;
	int adp_prob_comp;
	int adp_cal_done;
	atomic_t usb_allow_bc_detect;
	bool icc_host_ctrl;
	int usb_host_en;
};

struct trim_to_percentage {
	int trim;
	int perc;
};

static struct bcmpmu_accy_data *gp_accy_data;
static atomic_t drv_init_done;
static struct bcmpmu_accy_irq_evt_map accy_irq_evt_map[] = {
	{PMU_IRQ_USBINS, PMU_ACCY_EVT_OUT_USB_IN},
	{PMU_IRQ_USBRM, PMU_ACCY_EVT_OUT_USB_RM},
	{PMU_IRQ_CHGDET_LATCH, PMU_ACCY_EVT_OUT_CHGDET_LATCH},
	{PMU_IRQ_CHGDET_TO, PMU_ACCY_EVT_OUT_CHGDET_LATCH_TO},
	{PMU_IRQ_VA_SESS_VALID_R, PMU_ACCY_EVT_OUT_SESSION_VALID},
	{PMU_IRQ_VBUS_VALID_R, PMU_ACCY_EVT_OUT_VBUS_VALID},
	{PMU_IRQ_OTG_SESS_VALID_F, PMU_ACCY_EVT_OUT_SESSION_INVALID},
	{PMU_IRQ_VBUS_VALID_F, PMU_ACCY_EVT_OUT_VBUS_INVALID},
	{PMU_IRQ_IDCHG, PMU_ACCY_EVT_OUT_ID_CHANGE},
	{PMU_IRQ_RESUME_VBUS, PMU_ACCY_EVT_OUT_CHRG_RESUME_VBUS},
	{PMU_IRQ_USBOV, PMU_ACCY_EVT_OUT_USBOV},
	{PMU_IRQ_USBOV_DIS, PMU_ACCY_EVT_OUT_USBOV_DIS},
	{PMU_IRQ_CHGERRDIS, PMU_ACCY_EVT_OUT_CHGERRDIS},
};
#define NUM_ACCY_CORE_IRQS	ARRAY_SIZE(accy_irq_evt_map)

static char *chrgr_types_str[PMU_CHRGR_TYPE_MAX] = {
	[PMU_CHRGR_TYPE_NONE]	= "none",
	[PMU_CHRGR_TYPE_SDP]	= "sdp",
	[PMU_CHRGR_TYPE_CDP]	= "cdp",
	[PMU_CHRGR_TYPE_DCP]	= "dcp",
	[PMU_CHRGR_TYPE_PS2]	= "ps2",
	[PMU_CHRGR_TYPE_TYPE1]	= "type1",
	[PMU_CHRGR_TYPE_TYPE2]	= "type2",
	[PMU_CHRGR_TYPE_ACA_DOCK]	= "aca_dock",
	[PMU_CHRGR_TYPE_ACA]	= "aca",
};

static const int bcmpmu_usb_id_map[PMU_USB_ID_LVL_MAX] = {
	[0] =	PMU_USB_ID_GROUND,
	[1] =	PMU_USB_B_DEVICE,
	[2] =	PMU_USB_ID_RESERVED1,
	[3] =	PMU_USB_ID_RID_A,
	[4] =	PMU_USB_ID_RID_B,
	[5] =	PMU_USB_ID_RID_C,
	[6] =	PMU_USB_ID_RESERVED2,
	[7] =	PMU_USB_ID_FLOAT,
};

/** Current in MA */
static u32 bcmpmu_pmu_curr_table[] = {
	84,	/* 0x000 */
	270,	/* 0x001 */
	400,	/* 0x010 */
	510,	/* 0x11	*/
	550,	/* 0x100 */
	650,	/* 0x101 */
	695,	/* 0x110 */
	765,	/* 0x111 */
	935,	/* 0x1000 */
};

static u32 bcmpmu_pmu_curr_acld_table[] = {
	88,
	244,
	368,
	449,
	483,
	590,
	624,
	691,
	836,
	1051,
	1255,
	1565,
};

static struct trim_to_percentage trim_to_per[] = {
	{0x0, -3},
	{0x1, 0},
	{0x2, 3},
	{0x3, 6},
	{0x4, 9},
	{0x5, 12},
	{0x6, 16},
	{0x7, 20},
	{0x8, 24},
	{0x9, 29},
	{0xA, 33},
	{0xB, 38},
	{0xC, 44},
	{0xD, 50},
	{0xE, 56},
	{0xF, 64},
	{0x10, -32},
	{0x11, -31},
	{0x12, -29},
	{0x13, -28},
	{0x14, -27},
	{0x15, -25},
	{0x16, -23},
	{0x17, -22},
	{0x18, -20},
	{0x19, -18},
	{0x1A, -16},
	{0x1B, -14},
	{0x1C, -12},
	{0x1D, -10},
	{0x1E, -8},
	{0x1F, -5},
};
static int charging_enable = 1;
module_param_named(charging_enable, charging_enable, int,
		S_IRUGO | S_IWUSR | S_IWGRP);

#if 0
static int debug_mask = BCMPMU_PRINT_ERROR | BCMPMU_PRINT_INIT | \
			BCMPMU_PRINT_FLOW;
#endif
static int debug_mask = 0xff;

#define pr_accy(debug_level, args...) \
	do { \
		if (debug_mask & BCMPMU_PRINT_##debug_level) { \
			pr_info("[ACCY]:"args); \
		} \
	} while (0)

static void bcmpmu_accy_pm_qos_request(struct bcmpmu_accy_data *di,
		bool active);
static int bcmpmu_accy_queue_event(struct bcmpmu_accy_data *di,
		int event, u32 *data);

static inline int  _irq_to_index(int irq)
{
	int i;
	int index = -1;

	for (i = 0; i < NUM_ACCY_CORE_IRQS; i++)
		if (irq == accy_irq_evt_map[i].irq)
			break;
	if (i < NUM_ACCY_CORE_IRQS)
		index = i;
	return index;
}

static bool _chrgr_type_usb(int chrgr_type)
{
	bool is_usb = false;

	switch (chrgr_type) {
	case PMU_CHRGR_TYPE_SDP:
	case PMU_CHRGR_TYPE_CDP:
	case PMU_CHRGR_TYPE_ACA:
		is_usb = true;
		break;
	case PMU_CHRGR_TYPE_DCP:
	case PMU_CHRGR_TYPE_TYPE1:
	case PMU_CHRGR_TYPE_TYPE2:
	case PMU_CHRGR_TYPE_PS2:
	case PMU_CHRGR_TYPE_ACA_DOCK:
		is_usb = false;
	default:
		is_usb = false;
		break;
	}
	return is_usb;
}

static int _usb_host_en(struct bcmpmu_accy_data *di, int enable)
{
	struct bcmpmu59xxx *bcmpmu = di->bcmpmu;
	int ret = 0;
	u8 reg;

	if (!charging_enable)
		return 0;

	if (!atomic_read(&drv_init_done)) {
		pr_accy(ERROR, "%s: accy driver not initialized\n", __func__);
		return -EAGAIN;
	}

	ret = bcmpmu->read_dev(bcmpmu, PMU_REG_MBCCTRL3, &reg);
	if (ret) {
		pr_accy(ERROR, "%s: PMU read failed\n", __func__);
		return ret;
	}

	/* If charging is already enabled/disabled just return */
	if ((di->usb_host_en == enable) &&
			((reg & MBCCTRL3_USB_HOSTEN_MASK) == enable)) {
		pr_accy(INIT, "USB host is already in the reqested state\n");

	} else if (((reg & MBCCTRL3_USB_HOSTEN_MASK) != enable)) {
		if (enable)
			reg |= MBCCTRL3_USB_HOSTEN_MASK;
		else
			reg &= ~MBCCTRL3_USB_HOSTEN_MASK;

		ret = bcmpmu->write_dev(bcmpmu, PMU_REG_MBCCTRL3, reg);
		if (ret) {
			pr_accy(ERROR, "%s: PMU write failed\n", __func__);
			return ret;
		}
		di->usb_host_en = enable;
		ret = bcmpmu_accy_queue_event(di, PMU_CHRGR_EVT_CHRG_STATUS,
			&di->usb_host_en);
	} else {
		di->usb_host_en = enable;
		ret = bcmpmu_accy_queue_event(di, PMU_CHRGR_EVT_CHRG_STATUS,
			&di->usb_host_en);
	}

	pr_accy(FLOW, "%s:ENABLE %d\n", __func__, di->usb_host_en);

	return ret;
}

static int _curr_to_pmu_reg_idx(struct bcmpmu_accy_data *di, int curr)
{
	int i = 0;
	u8 reg;
	int ret = 0;
	bool acld_enabled = 0;

	if (di->bcmpmu->flags & BCMPMU_ACLD_EN) {
		ret = di->bcmpmu->read_dev(di->bcmpmu,
				PMU_REG_OTG_BOOSTCTRL3, &reg);
		if (ret)
			return ret;
		if (reg & ACLD_ENABLE_MASK)
			acld_enabled = true;
		else
			acld_enabled = false;
	}

	if (acld_enabled) {
		for (i = (ARRAY_SIZE(bcmpmu_pmu_curr_acld_table) - 1);
				i >= 0; i--) {
			if (curr >= bcmpmu_pmu_curr_acld_table[i])
				return i;
		}
	} else {
		for (i = (ARRAY_SIZE(bcmpmu_pmu_curr_table) - 1); i >= 0; i--) {
			if (curr >= bcmpmu_pmu_curr_table[i])
				return i;
		}
	}
	return 0;
}

static int _get_next_icc_fc(struct bcmpmu_accy_data *di)
{
	struct bcmpmu59xxx *bcmpmu = di->bcmpmu;
	int ret = 0;
	u8 reg = 0;
	bool acld_enabled = 0;
	int curr;

	if (!atomic_read(&drv_init_done)) {
		pr_accy(ERROR, "%s: accy driver not initialized\n", __func__);
		return -EAGAIN;
	}

	acld_enabled = bcmpmu_is_acld_enabled(bcmpmu);

	ret = bcmpmu->read_dev(bcmpmu, PMU_REG_MBCCTRL10, &reg);
	if (ret)
		return -EINVAL;

	/**
	 * if ACLD is enable, we use typical CC values
	 * from bcmpmu_pmu_curr_acld_table
	 */
	if (acld_enabled) {
		if (reg >= PMU_USB_FC_CC_MAX)
			reg = PMU_USB_FC_CC_MAX;
		else
			reg++;
		curr = bcmpmu_pmu_curr_acld_table[reg];
	}
	else
		curr = bcmpmu_pmu_curr_table[reg];

	return curr;
}
static int _get_icc_fc(struct bcmpmu_accy_data *di)
{
	struct bcmpmu59xxx *bcmpmu = di->bcmpmu;
	int ret = 0;
	u8 reg = 0;
	bool acld_enabled = 0;
	int curr;

	if (!atomic_read(&drv_init_done)) {
		pr_accy(ERROR, "%s: accy driver not initialized\n", __func__);
		return -EAGAIN;
	}

	acld_enabled = bcmpmu_is_acld_enabled(bcmpmu);

	ret = bcmpmu->read_dev(bcmpmu, PMU_REG_MBCCTRL10, &reg);
	if (ret)
		return -EINVAL;
	/**
	 * if ACLD is enable, we use typical CC values
	 * from bcmpmu_pmu_curr_acld_table
	 */
	if (acld_enabled)
		curr = bcmpmu_pmu_curr_acld_table[reg];
	else
		curr = bcmpmu_pmu_curr_table[reg];

	return curr;
}

static int _set_icc_fc(struct bcmpmu_accy_data *di, int curr)
{
	struct bcmpmu59xxx *bcmpmu = di->bcmpmu;
	int ret = 0;
	int val;

	if (!atomic_read(&drv_init_done)) {
		pr_accy(ERROR, "%s: accy driver not initialized\n", __func__);
		return -EAGAIN;
	}

	if (curr < 0)
		return -EINVAL;

	val = _curr_to_pmu_reg_idx(di, curr);
	ret = bcmpmu->write_dev(bcmpmu, PMU_REG_MBCCTRL10, (val & 0xF));
	if (ret)
		return ret;

	di->charging_curr = val;
	bcmpmu_accy_queue_event(di, PMU_ACCY_EVT_OUT_CHRG_CURR,
			&di->charging_curr);

	pr_accy(FLOW , "%s: curr = %d set to val %x\n",
			__func__, curr, (val & 0xF));

	/* If the curr passed is less than the minimum supported curr,
	 * disbale charging
	 * */
	if (curr < bcmpmu_pmu_curr_table[0]) {
		ret = _usb_host_en(di, 0);
		di->icc_host_ctrl = false;
	} else if ((!di->icc_host_ctrl) &&
			(curr >= bcmpmu_pmu_curr_table[0])) {
		ret = _usb_host_en(di, 1);
		di->icc_host_ctrl = true;
	}
	return ret;
}

static void bcmpmu_accy_evt_notify_work(struct work_struct *work)
{
	struct bcmpmu_accy_data *di = container_of(work,
			struct bcmpmu_accy_data, evt_notify_work.work);
	struct bcmpmu_accy_event *event_node;
	unsigned long flags;
	int event;
	u32 *data;

	pr_accy(VERBOSE, "%s\n", __func__);
	BUG_ON((!di) || (!di->bcmpmu));

	if (!atomic_read(&di->usb_allow_bc_detect)) {
		queue_delayed_work(di->wq, &di->evt_notify_work, 100);
		return;
	}

	for (; ;) {
		spin_lock_irqsave(&di->accy_lock, flags);
		if (list_empty(&di->event_pending_list)) {
			spin_unlock_irqrestore(&di->accy_lock, flags);
			break;
		}
		event_node = list_first_entry(&di->event_pending_list,
				struct bcmpmu_accy_event, node);
		event = event_node->event;
		data = event_node->data;
		list_del(&event_node->node);
		list_add_tail(&event_node->node, &di->event_free_list);
		spin_unlock_irqrestore(&di->accy_lock, flags);
		pr_accy(VERBOSE, "posting event: %d data:%p\n", event,
				data);
		bcmpmu_call_notifier(di->bcmpmu, event, data);
	}
}

static int bcmpmu_accy_queue_event(struct bcmpmu_accy_data *di,
		int event, u32 *data)
{
	struct bcmpmu_accy_event *event_node;
	unsigned long flags;

	if (!atomic_read(&drv_init_done))
		return -EAGAIN;

	BUG_ON(!di);

	spin_lock_irqsave(&di->accy_lock, flags);

	if (list_empty(&di->event_free_list)) {
		pr_accy(ERROR, "Accy event Q full!!\n");
		spin_unlock_irqrestore(&di->accy_lock, flags);
		return -ENOMEM;
	} else {
		event_node = list_first_entry(&di->event_free_list,
				struct bcmpmu_accy_event, node);
		event_node->event = event;
		event_node->data = data;
		list_del(&event_node->node);
		list_add_tail(&event_node->node, &di->event_pending_list);
	}

	spin_unlock_irqrestore(&di->accy_lock, flags);
	queue_delayed_work(di->wq, &di->evt_notify_work, 0);
	return 0;
}

static int bcmpmu_usb_otg_bost_en(struct bcmpmu59xxx *bcmpmu, bool en)
{
	int ret = 0;
	u8 reg;

	ret = bcmpmu->read_dev(bcmpmu,
				PMU_REG_OTGCTRL1, &reg);
	/*
	OTGCTRL1_OTG_SHUTDOWNB  = 1 => Enable the OTG block
	OTGCTRL1_OFFVBUSB = 1 => enable Vbus boost
	*/
	if (en)
		reg |= OTGCTRL1_OFFVBUSB_MASK |
				OTGCTRL1_OTG_SHUTDOWNB_MASK;
	else
		reg &= ~(OTGCTRL1_OFFVBUSB_MASK |
					OTGCTRL1_OTG_SHUTDOWNB_MASK);

	ret |= bcmpmu->write_dev(bcmpmu,
				PMU_REG_OTGCTRL1, reg);
	pr_accy(FLOW, "%s,en = %d, reg = %x\n", __func__, en,
		reg);
	return ret;

}

static int bcmpmu_accy_chrgr_type_handler(struct bcmpmu_accy_data *di,
		int chrgr_type)
{
	unsigned long flags;

	spin_lock_irqsave(&gp_accy_data->accy_lock, flags);
	if ((gp_accy_data->chrgr_type_prev == PMU_CHRGR_TYPE_NONE) &&
			(chrgr_type != PMU_CHRGR_TYPE_NONE))
		gp_accy_data->chrgr_type_prev = chrgr_type;
	else if (gp_accy_data->chrgr_type != PMU_CHRGR_TYPE_NONE)
		gp_accy_data->chrgr_type_prev =
			gp_accy_data->chrgr_type;
	gp_accy_data->chrgr_type = chrgr_type;
	spin_unlock_irqrestore(&gp_accy_data->accy_lock, flags);

	bcmpmu_accy_queue_event(gp_accy_data,
			PMU_ACCY_EVT_OUT_CHRGR_TYPE,
			&gp_accy_data->chrgr_type);

	if (gp_accy_data->chrgr_type == PMU_CHRGR_TYPE_NONE)
		bcmpmu_accy_pm_qos_request(gp_accy_data, false);
	else
		bcmpmu_accy_pm_qos_request(gp_accy_data, true);

	return 0;
}

int bcmpmu_usb_set(struct bcmpmu59xxx *bcmpmu,
		int ctrl, unsigned long data)
{
	struct bcmpmu_accy_data *di = bcmpmu->accyinfo;
	u8 temp;
	u32 val;
	int ret = 0;

	pr_accy(FLOW, "%s, ctrl=%d, val=0x%lX\n", __func__, ctrl, data);

	switch (ctrl) {
	case BCMPMU_USB_CTRL_CHRG_CURR_LMT:
		if (!is_charging_state())
			_set_icc_fc(di, data);
		break;
	case BCMPMU_USB_CTRL_VBUS_ON_OFF:
		ret = bcmpmu_usb_otg_bost_en(bcmpmu, !!data);
		break;

	case BCMPMU_USB_CTRL_SET_VBUS_DEB_TIME:
		ret = bcmpmu->read_dev(bcmpmu, PMU_REG_OTGCTRL2, &temp);
		temp &= (~OTGCTRL2_VBUS_ADP_COMP_DB_MASK);
		temp |= (data & OTGCTRL2_VBUS_ADP_COMP_DB_MASK);
		ret = bcmpmu->write_dev(bcmpmu, PMU_REG_OTGCTRL2, temp);
		break;

	case BCMPMU_USB_CTRL_SRP_VBUS_PULSE:
		ret = bcmpmu->read_dev(bcmpmu, PMU_REG_OTGCTRL1, &temp);
		if (data == 0)
			temp &= (~OTGCTRL1_EN_VBUS_PULSE_MASK);
		else
			temp |= (OTGCTRL1_EN_VBUS_PULSE_MASK);
		ret = bcmpmu->write_dev(bcmpmu, PMU_REG_OTGCTRL1, temp);
		break;

	case BCMPMU_USB_CTRL_DISCHRG_VBUS:
		ret = bcmpmu->read_dev(bcmpmu, PMU_REG_OTGCTRL1, &temp);
		if (data == 0)
			temp &= (~OTGCTRL1_EN_PD_SRP_MASK);
		else
			temp |= (OTGCTRL1_EN_PD_SRP_MASK);
		ret = bcmpmu->write_dev(bcmpmu, PMU_REG_OTGCTRL1, temp);
		break;

	case BCMPMU_USB_CTRL_START_STOP_ADP_SENS_PRB:
		if (data == 0)
			ret = bcmpmu->write_dev(bcmpmu, PMU_REG_OTGCTRL2, 0);
		else {
			if (di->otg_block_enabled == 0) {
				bcmpmu->read_dev(bcmpmu, PMU_REG_OTGCTRL1,
						&temp);
				temp |= (1 << OTGCTRL1_OTG_SHUTDOWNB_SHIFT);
				ret =
					bcmpmu->write_dev(bcmpmu,
							PMU_REG_OTGCTRL1,
							temp);
				di->otg_block_enabled = 1;
			}
			if (di->adp_block_enabled == 0) {
				bcmpmu->read_dev(bcmpmu,
						PMU_REG_OTGCTRL12,
						&temp);
				temp |= (1 << OTGCTRL12_ADP_THR_CTRL_SHIFT);
				ret =
					bcmpmu->write_dev(bcmpmu,
							PMU_REG_OTGCTRL12,
							temp);
				di->adp_block_enabled = 1;
			}
			if (di->adp_sns_comp == 0) {
				bcmpmu->read_dev(bcmpmu,
						PMU_REG_OTGCTRL12,
						&temp);
				temp |= (1 << OTGCTRL12_EN_ADP_SNS_COMP_SHIFT);
				ret =
					bcmpmu->write_dev(bcmpmu,
							PMU_REG_OTGCTRL12,
							temp);
				di->adp_sns_comp = 1;
			}

			bcmpmu->read_dev(bcmpmu, PMU_REG_OTGCTRL2,
					&temp);
			temp |= (1 << OTGCTRL2_ADP_SNS_SHIFT);
			ret = bcmpmu->write_dev(bcmpmu, PMU_REG_OTGCTRL2,
					temp);
		}
		break;

	case BCMPMU_USB_CTRL_START_STOP_ADP_PRB:
		if (data == 0) {
			ret = bcmpmu->read_dev(bcmpmu,
					PMU_REG_OTGCTRL11, &temp);
			temp &= ~OTGCTRL11_ADP_PRB_MASK;
			ret |= bcmpmu->write_dev(bcmpmu,
					PMU_REG_OTGCTRL11, temp);
		} else {
			if (di->otg_block_enabled == 0) {
				ret = bcmpmu->read_dev(bcmpmu,
						PMU_REG_OTGCTRL11, &temp);
				temp |= OTGCTRL1_OTG_SHUTDOWNB_MASK;
				ret = bcmpmu->write_dev(bcmpmu,
						PMU_REG_OTGCTRL11, temp);
				di->otg_block_enabled = 1;
			}
			if (di->adp_block_enabled == 0) {

				ret = bcmpmu->read_dev(bcmpmu,
						PMU_REG_OTGCTRL12,
						&temp);
				temp |= OTGCTRL12_ADP_SHUTDOWNB_MASK;
				ret |= bcmpmu->write_dev(bcmpmu,
						PMU_REG_OTGCTRL12,
						temp);
				di->adp_block_enabled = 1;
			}
			if (di->adp_prob_comp == 0) {
				ret = bcmpmu->read_dev(bcmpmu,
						PMU_REG_OTGCTRL12, &temp);

				temp |= OTGCTRL12_EN_ADP_PRB_COMPS_MASK;
				ret = bcmpmu->write_dev(bcmpmu,
						PMU_REG_OTGCTRL12, temp);

				di->adp_prob_comp = 1;
			}
			ret = bcmpmu->read_dev(bcmpmu,
				PMU_REG_OTGCTRL11, &temp);
			temp |= OTGCTRL11_ADP_PRB_MASK;
			ret |= bcmpmu->write_dev(bcmpmu,
				PMU_REG_OTGCTRL11, temp);

		}
		break;

	case BCMPMU_USB_CTRL_START_ADP_CAL_PRB:
		if (di->otg_block_enabled == 0) {
			ret = bcmpmu->read_dev(bcmpmu,
					PMU_REG_OTGCTRL1, &temp);
			temp |= OTGCTRL1_OTG_SHUTDOWNB_MASK;
			ret |= bcmpmu->write_dev(bcmpmu,
					PMU_REG_OTGCTRL1, temp);
			di->otg_block_enabled = 1;
		}
		if (di->adp_block_enabled == 0) {
			ret = bcmpmu->read_dev(bcmpmu,
					PMU_REG_OTGCTRL12, &temp);
			temp |= OTGCTRL12_ADP_SHUTDOWNB_MASK;
			ret |= bcmpmu->write_dev(bcmpmu,
					PMU_REG_OTGCTRL12, temp);
			di->adp_block_enabled = 1;
		}
		if (di->adp_prob_comp == 0) {
			ret = bcmpmu->read_dev(bcmpmu,
					PMU_REG_OTGCTRL12, &temp);
			temp |=  OTGCTRL12_EN_ADP_PRB_COMPS_MASK ;
			ret = bcmpmu->write_dev(bcmpmu,
					PMU_REG_OTGCTRL12, temp);
			di->adp_prob_comp = 1;
		}
		ret = bcmpmu->read_dev(bcmpmu,
				PMU_REG_OTGCTRL11, &temp);

		temp |= (PMU_USB_ADP_MODE_CALIBRATE <<
				OTGCTRL11_ADP_PRB_MODE_MASK);
		temp |= OTGCTRL11_ADP_PRB_MASK;

		ret |= bcmpmu->write_dev(bcmpmu,
				PMU_REG_OTGCTRL11, temp);
		di->adp_cal_done = 1;
		bcmpmu_accy_queue_event(di,
				PMU_ACCY_EVT_OUT_ADP_CALIBRATION_DONE,
				NULL);
		break;

	case BCMPMU_USB_CTRL_SET_ADP_PRB_MOD:
		ret = bcmpmu->read_dev(bcmpmu,
				PMU_REG_OTGCTRL11, &temp);
		temp &= ~OTGCTRL11_ADP_PRB_MODE_MASK;
		temp |=  (data << OTGCTRL11_ADP_PRB_MODE_SHIFT);

		ret = bcmpmu->write_dev(bcmpmu,
				PMU_REG_OTGCTRL11, temp);
		break;

	case BCMPMU_USB_CTRL_SET_ADP_PRB_CYC_TIME:
		ret = bcmpmu->read_dev(bcmpmu,
				PMU_REG_OTGCTRL2, &temp);
		temp &= ~OTGCTRL2_ADP_PRB_CYCLE_TIME_MASK;
		temp |= (data << OTGCTRL2_ADP_PRB_CYCLE_TIME_SHIFT);
		ret |= bcmpmu->write_dev(bcmpmu,
				PMU_REG_OTGCTRL2, temp);
		break;

	case BCMPMU_USB_CTRL_SET_ADP_COMP_METHOD:
		ret = bcmpmu->read_dev(bcmpmu,
				PMU_REG_OTGCTRL12, &temp);

		temp &= ~OTGCTRL12_ADP_COMP_METHOD_MASK;
		temp |= (data << OTGCTRL12_ADP_COMP_METHOD_SHIFT);

		ret |= bcmpmu->write_dev(bcmpmu,
				PMU_REG_OTGCTRL12, temp);
		break;

	case BCMPMU_USB_CTRL_SW_UP:
		ret = bcmpmu->read_dev(bcmpmu,
				PMU_REG_MBCCTRL3, &temp);
		if (data == 0)
			temp &= ~MBCCTRL3_SWUP_MASK;
		else
			temp |= MBCCTRL3_SWUP_MASK;
		ret |= bcmpmu->write_dev(bcmpmu,
				PMU_REG_MBCCTRL3, temp);
		break;
	case BCMPMU_USB_CTRL_TPROBE_MAX:
		if (data <= TPROBE_MAX_USEC) {
			val = (data * 2 * 1000) / 31250;
			bcmpmu->read_dev(bcmpmu,
					PMU_REG_OTGCTRL9, &temp);
			temp &= ~TPROBE_MAX_MSB_MASK;
			temp |= TPROBE_MAX_GET_MSB(val);
			ret = bcmpmu->write_dev(bcmpmu,
					PMU_REG_OTGCTRL9, temp);
			ret |= bcmpmu->write_dev(bcmpmu,
					PMU_REG_OTGCTRL10,
					TPROBE_MAX_GET_LSB(val));
		}
		break;
	case BCMPMU_USB_CTRL_ALLOW_BC_DETECT:
		if (data == 1)
			atomic_set(&di->usb_allow_bc_detect, 1);
		else
			atomic_set(&di->usb_allow_bc_detect, 0);
		break;

	default:
		ret = -EINVAL;
		break;
	}
	return ret;
}
EXPORT_SYMBOL_GPL(bcmpmu_usb_set);

int bcmpmu_usb_get(struct bcmpmu59xxx *bcmpmu,
		int ctrl, void *data)
{
	int ret = -EINVAL;
	struct bcmpmu_accy_data *di = bcmpmu->accyinfo;
	u8 val, val1;
	int tprobe;

	switch (ctrl) {
	case BCMPMU_USB_CTRL_TPROBE_MAX:
		ret = bcmpmu->read_dev(bcmpmu,
				PMU_REG_OTGCTRL9, &val);
		ret |= bcmpmu->read_dev(bcmpmu,
				PMU_REG_OTGCTRL10, &val1);
		tprobe = TPROBE_MAX_DATA(val, val1);
		val = ((tprobe * 31250) / (2 * 1000));
		break;
	case BCMPMU_USB_CTRL_GET_ADP_CHANGE_STATUS:
		ret = bcmpmu->read_dev(bcmpmu,
				PMU_REG_OTG_STS1,
				&val);
		val &= OTG_STS1_ADP_ATTACH_DET_MASK;
		val = val >> OTG_STS1_ADP_ATTACH_DET_SHIFT;
		break;
	case BCMPMU_USB_CTRL_GET_ADP_SENSE_STATUS:
		ret = bcmpmu->read_dev(bcmpmu,
				PMU_REG_OTG_STS1,
				&val);
		val &= val & OTG_STS1_ADP_SNS_DET_MASK;
		val = val >> OTG_STS1_ADP_SNS_DET_SHIFT;
		break;
#ifdef CONFIG_MFD_BCM59055
	case BCMPMU_USB_CTRL_GET_ADP_PRB_RISE_TIMES:
		ret = bcmpmu->read_dev(bcmpmu,
				PMU_REG_ADP_STATUS_RISE_TIMES_LSB,
				&val,
				bcmpmu->
				regmap
				[PMU_REG_ADP_STATUS_RISE_TIMES_LSB].
				mask);
		val = val >>
			bcmpmu->
			regmap[PMU_REG_ADP_STATUS_RISE_TIMES_LSB].shift;
		if (ret != 0)
			break;
		ret = bcmpmu->read_dev(bcmpmu,
				PMU_REG_ADP_STATUS_RISE_TIMES_MSB,
				&val1,
				bcmpmu->
				regmap
				[PMU_REG_ADP_STATUS_RISE_TIMES_MSB].
				mask);
		val1 = val1 >>
			bcmpmu->regmap[PMU_REG_ADP_STATUS_RISE_TIMES_MSB].shift;
		val = val | val1 << 8;
		break;
#endif
	case BCMPMU_USB_CTRL_GET_VBUS_STATUS:
		ret = bcmpmu->read_dev(bcmpmu,
				PMU_REG_ENV4,
				&val);
		val &= ENV4_VBUS_STATUS_MASK;
		val = val >> ENV4_VBUS_STATUS_SHIFT;
		break;
	case BCMPMU_USB_CTRL_GET_SESSION_STATUS:
		ret = bcmpmu->read_dev(bcmpmu,
				PMU_REG_ENV2,
				&val);
		pr_accy(FLOW, "ENV2 val  %x , usb valid mask %x\n",
			val, ENV2_P_UBPD_INT_SHIFT);
		val &= ENV2_P_UBPD_INT;
		val =  val >> ENV2_P_UBPD_INT_SHIFT;
		break;
	case BCMPMU_USB_CTRL_GET_USB_VALID:
		ret = bcmpmu->read_dev(bcmpmu,
				PMU_REG_ENV2,
				&val);
		val &= ENV2_USB_VALID;
		val =  (val >> ENV2_USB_VALID_SHIFT);
		break;
	case BCMPMU_USB_CTRL_GET_UBPD_INT:
		ret = bcmpmu->read_dev(bcmpmu,
				PMU_REG_ENV2,
				&val);
		val &= ENV2_P_UBPD_INT;
		val = (val >> ENV2_P_UBPD_INT_SHIFT);
		break;
	case BCMPMU_USB_CTRL_GET_USB_PORT_DISABLED:
		ret = bcmpmu->read_dev(bcmpmu,
				PMU_REG_ENV2,
				&val);
		val &= ENV2_USB_PORT_DISABLED;
		val = (val >> ENV2_USB_PORT_DISABLED_SHIFT);
		break;
	case BCMPMU_USB_CTRL_GET_SESSION_END_STATUS:
		ret = bcmpmu->read_dev(bcmpmu,
				PMU_REG_ENV7,
				&val);
		val &= OTG_VBUS_STAT_MASK;
		val = val >> OTG_VBUS_STAT_SHIFT;
		break;
	case BCMPMU_USB_CTRL_GET_ID_VALUE:
		ret = bcmpmu->read_dev(bcmpmu,
				PMU_REG_ENV4,
				&val);
		val &= ENV4_ID_CODE_MASK;
		val = val >> ENV4_ID_CODE_SHIFT;
		if (val < PMU_USB_ID_LVL_MAX)
			val = bcmpmu_usb_id_map[val];
		pr_accy(FLOW, "ENV4 %x val, id mak %x\n",
			val, ENV4_ID_CODE_SHIFT);
		break;
	case BCMPMU_USB_CTRL_GET_CHRGR_TYPE:
		ret = 0;
		val = di->chrgr_type;
		break;
	case BCMPMU_USB_CTRL_ALLOW_BC_DETECT:
		ret = 0;
		val = atomic_read(&di->usb_allow_bc_detect);
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

int bcmpmu_chrgr_usb_en(struct bcmpmu59xxx *bcmpmu, int enable)
{
	struct bcmpmu_accy_data *di = bcmpmu->accyinfo;

	return _usb_host_en(di, enable);
}
EXPORT_SYMBOL(bcmpmu_chrgr_usb_en);

int bcmpmu_set_icc_fc(struct bcmpmu59xxx *bcmpmu, int curr)
{
	struct bcmpmu_accy_data *di = bcmpmu->accyinfo;
	return _set_icc_fc(di, curr);
}
EXPORT_SYMBOL(bcmpmu_set_icc_fc);

int bcmpmu_get_icc_fc(struct bcmpmu59xxx *bcmpmu)
{
	return _get_icc_fc(bcmpmu->accyinfo);
}
EXPORT_SYMBOL(bcmpmu_get_icc_fc);

int bcmpmu_get_next_icc_fc(struct bcmpmu59xxx *bcmpmu)
{
	return _get_next_icc_fc(bcmpmu->accyinfo);
}
EXPORT_SYMBOL(bcmpmu_get_next_icc_fc);

int bcmpmu_icc_fc_step_down(struct bcmpmu59xxx *bcmpmu)
{
	int ret = 0;
	u8 reg;

	ret = bcmpmu->read_dev(bcmpmu, PMU_REG_MBCCTRL10, &reg);
	if (reg <= PMU_USB_FC_CC_MIN) {
		pr_accy(INIT, "Already at min CC\n");
		return -ENOSPC;
	} else
		reg--;

	ret = bcmpmu->write_dev(bcmpmu, PMU_REG_MBCCTRL10, reg);
	return ret;
}
EXPORT_SYMBOL(bcmpmu_icc_fc_step_down);

int bcmpmu_icc_fc_step_up(struct bcmpmu59xxx *bcmpmu)
{
	int ret = 0;
	u8 reg;

	ret = bcmpmu->read_dev(bcmpmu, PMU_REG_MBCCTRL10, &reg);
	reg++;
	if (reg > PMU_USB_FC_CC_MAX) {
		pr_accy(INIT, "Already at max CC\n");
		return -ENOSPC;
	}

	ret = bcmpmu->write_dev(bcmpmu, PMU_REG_MBCCTRL10, reg);
	return ret;
}
EXPORT_SYMBOL(bcmpmu_icc_fc_step_up);

int bcmpmu_set_cc_trim(struct bcmpmu59xxx *bcmpmu, int cc_trim)
{
	int ret = 0;
	u8 reg = 0;

	if ((cc_trim < PMU_USB_CC_TRIM_MIN) ||
			(cc_trim > PMU_USB_CC_TRIM_MAX)) {
		pr_accy(INIT, "cc_trim beyond limit\n");
		BUG_ON(1);
	}

	reg = cc_trim;
	ret = bcmpmu->write_dev(bcmpmu, PMU_REG_MBCCTRL18, reg);
	return ret;
}
EXPORT_SYMBOL(bcmpmu_set_cc_trim);

int bcmpmu_get_cc_trim(struct bcmpmu59xxx *bcmpmu)
{
	int ret = 0;
	u8 reg = 0;

	ret = bcmpmu->read_dev(bcmpmu, PMU_REG_MBCCTRL18, &reg);

	if (reg > PMU_USB_CC_TRIM_MAX) {
		pr_accy(FLOW, "%s: cc_trim beyond limit\n", __func__);
		BUG_ON(1);
	}
	return reg;
}
int  bcmpmu_get_trim_curr(struct bcmpmu59xxx *bcmpmu)
{
	int curr;
	int icc_fc;
	u8 trim = 0;

	icc_fc = bcmpmu_get_icc_fc(bcmpmu);
	trim = bcmpmu_get_cc_trim(bcmpmu);
	curr = ((icc_fc * trim_to_per[trim].perc) / 100);
	pr_accy(INIT, "icc_fc = %d trim_reg = 0x%x curr = %d",
			icc_fc, trim, curr);

	return curr;
}
int bcmpmu_cc_trim_up(struct bcmpmu59xxx *bcmpmu)
{
	int ret = 0;
	u8 reg;

	ret = bcmpmu->read_dev(bcmpmu, PMU_REG_MBCCTRL18, &reg);
	reg++;
	if (reg > PMU_USB_CC_TRIM_MAX) {
		pr_accy(INIT, "Already at max trim code\n");
		return -ENOSPC;
	}
	ret = bcmpmu->write_dev(bcmpmu, PMU_REG_MBCCTRL18, reg);
	return ret;

}
EXPORT_SYMBOL(bcmpmu_cc_trim_up);

int bcmpmu_cc_trim_down(struct bcmpmu59xxx *bcmpmu)
{
	int ret = 0;
	u8 reg;

	ret = bcmpmu->read_dev(bcmpmu, PMU_REG_MBCCTRL18, &reg);
	if (reg <= PMU_USB_CC_TRIM_MIN) {
		pr_accy(INIT, "Already at min trim code\n");
		return -ENOSPC;
	} else
		reg--;
	ret = bcmpmu->write_dev(bcmpmu, PMU_REG_MBCCTRL18, reg);
	return ret;

}
EXPORT_SYMBOL(bcmpmu_cc_trim_down);

bool bcmpmu_get_mbc_faults(struct bcmpmu59xxx *bcmpmu)
{
	int ret = 0;
	u8 reg;

	ret = bcmpmu->read_dev(bcmpmu, PMU_REG_ENV3, &reg);
	if (reg & ENV3_CV_TMR_EXP || (reg & ENV3_TRUE_TF))
		return false;

	ret = bcmpmu->read_dev(bcmpmu, PMU_REG_ENV6, &reg);
	if ((reg & ENV6_HW_TCH_EXP) || (reg & ENV6_MBCERROR))
		return false;

	return true;

}
bool bcmpmu_is_usb_host_enabled(struct bcmpmu59xxx *bcmpmu)
{
	int ret = 0;
	u8 reg;

	ret = bcmpmu->read_dev(bcmpmu, PMU_REG_MBCCTRL3, &reg);
	if (ret)
		return false;

	if (reg & MBCCTRL3_USB_HOSTEN_MASK)
		return true;

	return false;
}
EXPORT_SYMBOL(bcmpmu_is_usb_host_enabled);

int bcmpmu_check_vbus(void)
{
	int ret;
	int valid;
	u8 reg;

	if (!atomic_read(&drv_init_done)) {
		pr_accy(ERROR, "%s: BCMPMU Accy driver is not initialized\n",
				__func__);
		return -1;
	}

	ret = gp_accy_data->bcmpmu->read_dev(gp_accy_data->bcmpmu, PMU_REG_ENV2,
									&reg);
	if (ret)
		return -1;

	if (reg & ENV2_P_UBPD_ENV)
		valid = 1;
	else
		valid = 0;

	return valid;
}
EXPORT_SYMBOL_GPL(bcmpmu_check_vbus);

int bcmpmu_accy_chrgr_type_notify(int chrgr_type)
{
	int ret = 0;

	if (!atomic_read(&drv_init_done)) {
		pr_accy(ERROR, "%s: BCMPMU Accy driver is not initialized\n",
				__func__);
		return -1;
	}
	if ((chrgr_type >= PMU_CHRGR_TYPE_NONE) &&
			(chrgr_type < PMU_CHRGR_TYPE_MAX)) {
		pr_accy(FLOW, "----charger type: %s ---\n",
				chrgr_types_str[chrgr_type]);
		ret = bcmpmu_accy_chrgr_type_handler(gp_accy_data, chrgr_type);
	}
	return ret;
}
EXPORT_SYMBOL(bcmpmu_accy_chrgr_type_notify);

static void bcmpmu_accy_pm_qos_request(struct bcmpmu_accy_data *di, bool active)
{
	pr_accy(FLOW, "%s: active - %d\n", __func__, active);

	if (active) {
#ifdef CONFIG_HAS_WAKELOCK
		if (!wake_lock_active(&di->accy_wake_lock))
			wake_lock(&di->accy_wake_lock);
#endif
#ifdef CONFIG_KONA_PI_MGR
		if (di->qos_client.valid)
			pi_mgr_qos_request_update(&di->qos_client, 0);
#endif
	} else {
#ifdef CONFIG_HAS_WAKELOCK
		wake_unlock(&di->accy_wake_lock);
#endif
#ifdef CONFIG_KONA_PI_MGR
		if (di->qos_client.valid)
			pi_mgr_qos_request_update(&di->qos_client,
					PI_MGR_QOS_DEFAULT_VALUE);
#endif
	}
}

static void bcmpmu_accy_isr(enum bcmpmu59xxx_irq irq, void *data)
{
	struct bcmpmu_accy_data *di = data;
	int board_id = di->bcmpmu->pdata->board_id;
	int idx;
	int chrgr_type;
	int chrgr_type_prev;
	bool chrgr_is_usb;
	unsigned long flags;
	int event = PMU_EVENT_MAX;


	idx = _irq_to_index(irq);
	pr_accy(FLOW, "%s: IRQ: %d idx %d\n", __func__, irq, idx);
	BUG_ON(idx < 0);

	spin_lock_irqsave(&gp_accy_data->accy_lock, flags);
	chrgr_type = di->chrgr_type;
	chrgr_type_prev = di->chrgr_type_prev;
	chrgr_is_usb = _chrgr_type_usb(chrgr_type_prev);
	spin_unlock_irqrestore(&gp_accy_data->accy_lock, flags);
	pr_accy(FLOW, "chrgr_type: %d chrgr_type_prev:%d\n",
			chrgr_type, chrgr_type_prev);
	switch (irq) {
	case PMU_IRQ_USBINS:
		event = accy_irq_evt_map[idx].event;
		break;
	case PMU_IRQ_CHGDET_TO:
		/**
		 * In EDN01x hardware baseband chrgr detect latch pin
		 * is not connected to PMU because of which
		 * PMU triggers chrger detect timeout interrupt
		 * As as workaround for EDN01x HW, we will post
		 * PMU_ACCY_EVT_OUT_CHGDET_LATCH event to charger
		 * detection driver
		 */
		if (board_id == EDN01x)
			event = PMU_ACCY_EVT_OUT_CHGDET_LATCH;
		else
			event = PMU_ACCY_EVT_OUT_CHGDET_LATCH_TO;
		break;
	case PMU_IRQ_OTG_SESS_VALID_F:
		spin_lock_irqsave(&gp_accy_data->accy_lock, flags);
		di->chrgr_type = PMU_CHRGR_TYPE_NONE;
		di->chrgr_type_prev = PMU_CHRGR_TYPE_NONE;
		event = accy_irq_evt_map[idx].event;
		spin_unlock_irqrestore(&gp_accy_data->accy_lock, flags);
		pr_accy(VERBOSE, "PMU_IRQ_OTG_SESS_VALID_F\n");
		if (chrgr_is_usb) {
			pr_accy(VERBOSE,
				"posting event PMU_IRQ_OTG_SESS_VALID_F\n");
			event = accy_irq_evt_map[idx].event;
		} else if (di->pmu_session_a) {
			pr_accy(VERBOSE,
				"posting event PMU_IRQ_OTG_SESS_VALID_F\n");
			event = accy_irq_evt_map[idx].event;
			di->pmu_session_a = false;
		} else {
			event = -1;
		}
		break;
	case PMU_IRQ_VBUS_VALID_F:
		if (chrgr_is_usb) {
			pr_accy(VERBOSE,
				"posting event PMU_IRQ_VBUS_VALID_F\n");
			event = accy_irq_evt_map[idx].event;
		} else {
			event = -1;
		}
		break;
	case PMU_IRQ_IDCHG:
		bcmpmu_usb_get(di->bcmpmu,
			BCMPMU_USB_CTRL_GET_ID_VALUE, &di->pmu_usb_id);

		if ((di->pmu_usb_id == PMU_USB_ID_RID_A) ||
				(di->pmu_usb_id == PMU_USB_ID_GROUND))
			di->pmu_session_a = true;

		event = accy_irq_evt_map[idx].event;
		pr_accy(VERBOSE, " ==== pmu_id %d\n", di->pmu_usb_id);
		break;
	case PMU_IRQ_USBRM:
	case PMU_IRQ_CHGDET_LATCH:
	case PMU_IRQ_VA_SESS_VALID_R:
	case PMU_IRQ_VBUS_VALID_R:
	case PMU_IRQ_USBOV:
	case PMU_IRQ_USBOV_DIS:
	case PMU_IRQ_CHGERRDIS:
	case PMU_IRQ_RESUME_VBUS:
		event = accy_irq_evt_map[idx].event;
		break;
	default:
		BUG_ON(1);
	}

	if ((event > 0) && (event < PMU_EVENT_MAX))
		bcmpmu_accy_queue_event(di, event, NULL);
}

static int bcmpmu_accy_eventq_init(struct bcmpmu_accy_data *di)
{
	int idx;
	BUG_ON(!di);
	INIT_LIST_HEAD(&di->event_free_list);
	INIT_LIST_HEAD(&di->event_pending_list);

	for (idx = 0; idx < MAX_EVENTS; idx++)
		list_add_tail(&di->event_pool[idx].node, &di->event_free_list);

	return 0;
}

static int bcmpmu_accy_register_irqs(struct bcmpmu_accy_data *di)
{
	int ret = 0;
	int i;
	int j;

	for (i = 0; i < NUM_ACCY_CORE_IRQS; i++) {
		ret = di->bcmpmu->register_irq(di->bcmpmu,
				accy_irq_evt_map[i].irq,
				bcmpmu_accy_isr, di);
		if (ret)
			break;
	}
	if (i == NUM_ACCY_CORE_IRQS) {
		for (i = 0; i < NUM_ACCY_CORE_IRQS; i++)
			ret = di->bcmpmu->unmask_irq(di->bcmpmu,
					accy_irq_evt_map[i].irq);
	} else {
		for (j = i; j < 1; j--)
			di->bcmpmu->unregister_irq(di->bcmpmu,
					accy_irq_evt_map[j].irq);
	}
	return ret;
}

#ifdef CONFIG_DEBUG_FS
static int debugfs_usb_host_en(void *data, u64 enable)
{
	struct bcmpmu_accy_data *di = (struct bcmpmu_accy_data *)data;

	bcmpmu_chrgr_usb_en(di->bcmpmu, (enable & MBCCTRL3_USB_HOSTEN_MASK));
	return 0;
}
DEFINE_SIMPLE_ATTRIBUTE(usb_host_en_fops,
		NULL, debugfs_usb_host_en, "%llu\n");

static void bcmpmu_accy_debugfs_init(struct bcmpmu_accy_data *di)
{
	struct dentry *dentry_dir;
	struct dentry *dentry_file;

	if (!di->bcmpmu || !di->bcmpmu->dent_bcmpmu)
		return;

	dentry_dir = debugfs_create_dir("accy_core", di->bcmpmu->dent_bcmpmu);
	if (!dentry_dir) {
		pr_accy(FLOW, "failed to create directory\n");
		return;
	}
	dentry_file = debugfs_create_u32("chrgr_type", DEBUG_FS_PERMISSIONS,
						dentry_dir,
						&di->chrgr_type);
	if (!dentry_file)
		goto clean_debugfs;
	dentry_file = debugfs_create_u32("debug_mask", DEBUG_FS_PERMISSIONS,
			dentry_dir,
			&debug_mask);
	if (!dentry_file)
		goto clean_debugfs;

	dentry_file = debugfs_create_file("usb_host_en", DEBUG_FS_PERMISSIONS,
			dentry_dir, di, &usb_host_en_fops);
	if (!dentry_file)
		goto clean_debugfs;
	return;
clean_debugfs:
	debugfs_remove_recursive(dentry_dir);
}
#endif

#if CONFIG_PM
static int bcmpmu_accy_resume(struct platform_device *pdev)
{
	return 0;
}

static int bcmpmu_accy_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct bcmpmu59xxx *bcmpmu = dev_get_drvdata(pdev->dev.parent);
	struct bcmpmu_accy_data *di;
	di = bcmpmu->accyinfo;
	flush_delayed_work_sync(&di->evt_notify_work);
	return 0;
}

#else
#define bcmpmu_accy_resume	NULL
#define bcmpmu_accy_suspend	NULL
#endif

static int __devinit bcmpmu_accy_probe(struct platform_device *pdev)
{
	int ret;
	struct bcmpmu_accy_data *di;
	struct bcmpmu59xxx *bcmpmu = dev_get_drvdata(pdev->dev.parent);
	struct bcmpmu59xxx_accy_pdata *pdata = pdev->dev.platform_data;

	pr_accy(INIT, "%s, called\n", __func__);

	if (!pdata) {
		pr_accy(INIT, "%s: platform data NULL\n", __func__);
		return -EINVAL;
	}

	di = kzalloc(sizeof(struct bcmpmu_accy_data), GFP_KERNEL);
	if (!di) {
		pr_accy(INIT, "%s: failed to allocate memory\n", __func__);
		return -ENOMEM;
	}

	bcmpmu = dev_get_drvdata(pdev->dev.parent);
	bcmpmu->accyinfo = di;
	di->bcmpmu = bcmpmu;
	di->pdata = pdata;

	ret = bcmpmu_accy_register_irqs(di);
	if (ret) {
		pr_accy(INIT, "%s: failed to register irqs\n", __func__);
		goto free_mem;
	}

	di->wq = create_singlethread_workqueue("accy_wq");
	if (!di->wq) {
		pr_accy(INIT, "failed to create workq\n");
		ret = -ENOMEM;
		goto free_mem;
	}
	INIT_DELAYED_WORK(&di->evt_notify_work,
			bcmpmu_accy_evt_notify_work);

	spin_lock_init(&di->accy_lock);
	bcmpmu_accy_eventq_init(di);

#ifdef CONFIG_HAS_WAKELOCK
	wake_lock_init(&di->accy_wake_lock, WAKE_LOCK_SUSPEND, "accy_core");
#endif

#ifdef CONFIG_KONA_PI_MGR
	if (pdata && (pdata->flags & ACCY_USE_PM_QOS)) {
		ret = pi_mgr_qos_add_request(&di->qos_client, "accy_core",
				pdata->qos_pi_id,
				PI_MGR_QOS_DEFAULT_VALUE);
		BUG_ON(ret);
	}
#endif
#ifdef CONFIG_DEBUG_FS
	bcmpmu_accy_debugfs_init(di);
#endif
	gp_accy_data = di;
	di->chrgr_type = PMU_CHRGR_TYPE_NONE;
	di->chrgr_type_prev = PMU_CHRGR_TYPE_NONE;
	atomic_set(&drv_init_done, 1);
	pr_accy(INIT, "%s: success\n", __func__);
	return 0;

free_mem:
	bcmpmu->accyinfo = NULL;
	kfree(di);
	return ret;
}

static int __devexit bcmpmu_accy_remove(struct platform_device *pdev)
{
	int i;
	struct bcmpmu_accy_data *di = platform_get_drvdata(pdev);

	for (i = 0; i < NUM_ACCY_CORE_IRQS; i++)
		di->bcmpmu->unregister_irq(di->bcmpmu,
				accy_irq_evt_map[i].irq);
	cancel_delayed_work_sync(&di->evt_notify_work);
	destroy_workqueue(di->wq);
	di->bcmpmu->accyinfo = NULL;
	kfree(di);
	return 0;
}

static struct platform_driver bcmpmu_accy_driver = {
	.driver = {
		.name = "bcmpmu_accy",
	},
	.probe = bcmpmu_accy_probe,
	.remove = __devexit_p(bcmpmu_accy_remove),
	.suspend = bcmpmu_accy_suspend,
	.resume = bcmpmu_accy_resume,
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

MODULE_DESCRIPTION("BCM PMIC accy core driver");
MODULE_LICENSE("GPL");
