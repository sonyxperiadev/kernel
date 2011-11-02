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

#include <asm/io.h>
#include <mach/io_map.h>
#include <mach/rdb/brcm_rdb_hsotg_ctrl.h>
#include <mach/rdb/brcm_rdb_kpm_clk_mgr_reg.h>

#include <linux/mfd/bcmpmu.h>

extern int bcm_hsotgctrl_en_clock(bool on);
extern int bcm_hsotgctrl_bc_reset(void);
extern int bcm_hsotgctrl_bc_vdp_src_off(void);

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

#define BB_BC_STATUS		KONA_USB_HSOTG_CTRL_VA + HSOTG_CTRL_BC11_STATUS_OFFSET
#define BB_BC_STS_SDP_MSK	HSOTG_CTRL_BC11_STATUS_SHP_MASK
#define BB_BC_STS_CDP_MSK	HSOTG_CTRL_BC11_STATUS_CHP_MASK
#define BB_BC_STS_DCP_MSK	HSOTG_CTRL_BC11_STATUS_DCP_MASK
#define BB_BC_STS_BC_DONE_MSK	HSOTG_CTRL_BC11_STATUS_BC_DONE_MASK
#define BB_BC_STS_DM_TO_MSK	HSOTG_CTRL_BC11_STATUS_DM_TIMEOUT_MASK
#define BB_BC_STS_DP_TO_MSK	HSOTG_CTRL_BC11_STATUS_DP_TIMEOUT_MASK
#define BB_BC_STS_DM_ERR_MSK	HSOTG_CTRL_BC11_STATUS_DM_ERROR_MASK
#define BB_BC_STS_DP_ERR_MSK	HSOTG_CTRL_BC11_STATUS_DP_ERROR_MASK

struct accy_cb {
	void (*callback)(struct bcmpmu *,
		unsigned char event, void *, void *);
	void *clientdata;
};

int bcmpmu_usb_set(struct bcmpmu *bcmpmu,
			enum bcmpmu_usb_ctrl_t ctrl, unsigned long data);
int bcmpmu_usb_get(struct bcmpmu *bcmpmu,
			enum bcmpmu_usb_ctrl_t ctrl, void *data);


enum bcmpmu_usb_det_state_t {
	USB_IDLE,
	USB_CONNECTED,
	USB_DETECT,
	USB_RETRY,
};

static int chrgr_curr_lmt[PMU_CHRGR_TYPE_MAX] = {
	[PMU_CHRGR_TYPE_NONE] = 0,
	[PMU_CHRGR_TYPE_USB] = 500,
	[PMU_CHRGR_TYPE_AC] = 500,
	[PMU_CHRGR_TYPE_DCP] = 1500,
	[PMU_CHRGR_TYPE_TYPE1] = 1000,
	[PMU_CHRGR_TYPE_TYPE2] = 1000,
};

struct event_notifier {
	u32 event_id;
	struct blocking_notifier_head notifiers;
};

struct bcmpmu_accy {
	struct bcmpmu *bcmpmu;
	struct event_notifier event[BCMPMU_EVENT_MAX];
	const int *usb_id_map;
	int usb_id_map_len;
	wait_queue_head_t wait;
	struct delayed_work adp_work;
	struct delayed_work det_work;
	struct mutex lock;
	struct wake_lock wake_lock;
	struct accy_cb usb_cb;
	int adp_cal_done;
	enum bcmpmu_usb_det_state_t det_state;
	int otg_block_enabled;
	int adp_block_enabled;
	int adp_prob_comp;
	int adp_sns_comp;
	int retry_cnt;
	int clock_en;
};
static struct bcmpmu_accy *bcmpmu_accy;

int bcmpmu_usb_add_notifier(u32 event_id, struct notifier_block *notifier)
{
	if (!bcmpmu_accy) {
		pr_accy(ERROR,"%s: BCMPMU Accy driver is not initialized\n", __func__);
		return -EAGAIN;
	}
	if (unlikely(event_id >= BCMPMU_EVENT_MAX)) {
		pr_accy(ERROR,"%s: Invalid event id\n", __func__);
		return -EINVAL;
	}
	return blocking_notifier_chain_register(
			&bcmpmu_accy->event[event_id].notifiers, notifier);
}
EXPORT_SYMBOL_GPL(bcmpmu_usb_add_notifier);

int bcmpmu_usb_remove_notifier(u32 event_id, struct notifier_block *notifier)
{
	if (!bcmpmu_accy) {
		pr_accy(ERROR,"%s: BCMPMU accy driver is not initialized\n", __func__);
		return -EAGAIN;
	}
	if (unlikely(event_id >= BCMPMU_EVENT_MAX)) {
		pr_accy(ERROR,"%s: Invalid event id\n", __func__);
		return -EINVAL;
	}

	return blocking_notifier_chain_unregister(
			&bcmpmu_accy->event[event_id].notifiers, notifier);
}
EXPORT_SYMBOL_GPL(bcmpmu_usb_remove_notifier);


static void send_usb_event(struct bcmpmu *pmu,
	enum bcmpmu_usb_event_t event, void *para)
{
	struct power_supply *ps;
	union power_supply_propval propval;
	struct bcmpmu_accy *paccy = (struct bcmpmu_accy *)pmu->accyinfo;

	if (paccy->usb_cb.callback != NULL) {
		paccy->usb_cb.callback(paccy->bcmpmu, event, para,
			paccy->usb_cb.clientdata);
		pr_accy(FLOW, "%s, event=%d, val=%p\n", __func__,
			event, para);
	}
	blocking_notifier_call_chain(&paccy->event[event].notifiers,
				event, para);

	if (event != BCMPMU_USB_EVENT_USB_DETECTION) return;
	/* update power supply usb property  */
	pr_accy(FLOW, "%s, usb change, usb_type=0x%X\n",
		__func__, paccy->bcmpmu->usb_accy_data.usb_type);
	ps = power_supply_get_by_name("usb");
	if (ps) {
		propval.intval = paccy->bcmpmu->usb_accy_data.usb_type;
		ps->set_property(ps, POWER_SUPPLY_PROP_TYPE, &propval);
		if (paccy->det_state == USB_CONNECTED)
			propval.intval = 1;
		else 
			propval.intval = 0;
		ps->set_property(ps, POWER_SUPPLY_PROP_ONLINE, &propval);
		power_supply_changed(ps);
	}
}

static void send_chrgr_event(struct bcmpmu *pmu,
	enum bcmpmu_usb_event_t event, void *para)
{
	struct power_supply *ps;
	union power_supply_propval propval;
	struct bcmpmu_accy *paccy = (struct bcmpmu_accy *)pmu->accyinfo;

	blocking_notifier_call_chain(&paccy->event[event].notifiers,
				event, para);

	ps = power_supply_get_by_name("charger");
	if (ps == 0) return;
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

static void bcmpmu_accy_isr(enum bcmpmu_irq irq, void *data)
{
	unsigned int addr, val;
	struct bcmpmu_accy *paccy = data;
	struct bcmpmu *bcmpmu = paccy->bcmpmu;
	pr_accy(FLOW, "%s interrupt = %d\n",__func__, irq);

	switch (irq) {
	case PMU_IRQ_VBUS_1V5_R:
		send_usb_event(bcmpmu, BCMPMU_USB_EVENT_SESSION_VALID, NULL);
		break;

	case PMU_IRQ_USBINS:
		send_usb_event(bcmpmu, BCMPMU_USB_EVENT_IN, NULL);
		break;
	case PMU_IRQ_USBRM:
		schedule_delayed_work(&paccy->det_work, 0);
		break;
	case PMU_IRQ_CHGDET_LATCH:
		schedule_delayed_work(&paccy->det_work, msecs_to_jiffies(0));
		break;

	case PMU_IRQ_VBUS_1V5_F:
		schedule_delayed_work(&paccy->det_work, 0);
		send_usb_event(bcmpmu, BCMPMU_USB_EVENT_SESSION_INVALID, NULL);
		break;

	case PMU_IRQ_VBUS_4V5_R:
		send_usb_event(bcmpmu, BCMPMU_USB_EVENT_VBUS_VALID, NULL);
		paccy->det_state = USB_IDLE;
		schedule_delayed_work(&paccy->det_work, 0);
		break;

	case PMU_IRQ_VBUS_4V5_F:
		send_usb_event(bcmpmu, BCMPMU_USB_EVENT_VBUS_INVALID, NULL);
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
		send_usb_event(bcmpmu, BCMPMU_USB_EVENT_SESSION_END_VALID, NULL);
		break;

	case PMU_IRQ_SESSION_END_INVLD:
		send_usb_event(bcmpmu, BCMPMU_USB_EVENT_SESSION_END_INVALID, NULL);
		break;

	case PMU_IRQ_VBUS_OVERCURRENT:
		send_usb_event(bcmpmu, BCMPMU_USB_EVENT_VBUS_OVERCURRENT, NULL);
		break;

	case PMU_IRQ_CHGDET_TO:
		break;
	
	default:
		break;
	}
}

#ifdef CONFIG_MFD_BCMPMU_DBG
static ssize_t
dbgmsk_show(struct device *dev, struct device_attribute *attr,
				char *buf)
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
bcmpmu_dbg_show_bb_usbotg_regs(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	int i;
	unsigned int addr, val;
	for (i=0; i < 10; i++) {
		addr = KONA_USB_HSOTG_CTRL_VA + i*4;
		val = readl(addr);
		sprintf(buf, "addr=%X, val=%X\n", addr, val);
		buf = buf + 28;
	}
	return 28 * 10;
}

static ssize_t
bcmpmu_dbg_show_usb_otg_clkgate(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	unsigned int addr = KONA_KPM_CLK_VA + KPM_CLK_MGR_REG_USB_OTG_CLKGATE_OFFSET;
	unsigned int val = readl(addr);
	return sprintf(buf, "%X\n", val);
}

static ssize_t
bcmpmu_dbg_show_usbotgcontrol(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	unsigned int addr = KONA_USB_HSOTG_CTRL_VA + HSOTG_CTRL_USBOTGCONTROL_OFFSET;
	unsigned int val = readl(addr);
	return sprintf(buf, "%X\n", val);
}
static ssize_t
bcmpmu_dbg_set_usbotgcontrol(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t n)
{
	unsigned int addr = KONA_USB_HSOTG_CTRL_VA + HSOTG_CTRL_USBOTGCONTROL_OFFSET;
	unsigned long val = simple_strtoul(buf, NULL, 0);
	writel((unsigned int)val, addr);
	return n;
}

static ssize_t
bcmpmu_dbg_show_phy_cfg(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	unsigned int addr = KONA_USB_HSOTG_CTRL_VA + HSOTG_CTRL_PHY_CFG_OFFSET;
	unsigned int val = readl(addr);
	return sprintf(buf, "%X\n", val);
}
static ssize_t
bcmpmu_dbg_set_phy_cfg(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t n)
{
	unsigned int addr = KONA_USB_HSOTG_CTRL_VA + HSOTG_CTRL_PHY_CFG_OFFSET;
	unsigned long val = simple_strtoul(buf, NULL, 0);
	writel((unsigned int)val, addr);
	return n;
}

static ssize_t
bcmpmu_dbg_show_phy_p1ctl(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	unsigned int addr = KONA_USB_HSOTG_CTRL_VA + HSOTG_CTRL_PHY_P1CTL_OFFSET;
	unsigned int val = readl(addr);
	return sprintf(buf, "%X\n", val);
}
static ssize_t
bcmpmu_dbg_set_phy_p1ctl(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t n)
{
	unsigned int addr = KONA_USB_HSOTG_CTRL_VA + HSOTG_CTRL_PHY_P1CTL_OFFSET;
	unsigned long val = simple_strtoul(buf, NULL, 0);
	writel((unsigned int)val, addr);
	return n;
}

static ssize_t
bcmpmu_dbg_show_bc_status(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	unsigned int addr = KONA_USB_HSOTG_CTRL_VA + HSOTG_CTRL_BC11_STATUS_OFFSET;
	unsigned int val = readl(addr);
	return sprintf(buf, "%X\n", val);
}

static ssize_t
bcmpmu_dbg_show_bc_cfg(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	unsigned int addr = KONA_USB_HSOTG_CTRL_VA + HSOTG_CTRL_BC11_CFG_OFFSET;
	unsigned int val = readl(addr);
	return sprintf(buf, "%X\n", val);
}

static ssize_t
bcmpmu_dbg_set_bc_cfg(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t n)
{
	unsigned int addr = KONA_USB_HSOTG_CTRL_VA + HSOTG_CTRL_BC11_CFG_OFFSET;
	unsigned long val = simple_strtoul(buf, NULL, 0);
	writel((unsigned int)val, addr);
	return n;
}

static ssize_t
bcmpmu_dbg_usb_set(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t n)
{
	struct bcmpmu *bcmpmu = dev->platform_data;
	int ctrl, val;
	sscanf(buf, "%x, %x", &ctrl, &val);
	bcmpmu->usb_set(bcmpmu, ctrl, val);
	pr_accy(FLOW, "%s, ctrl=0x%X, val=0x%X\n",
		__func__, ctrl, val);

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
	pr_accy(FLOW, "%s, ctrl=0x%X, val=0x%X\n",
		__func__, ctrl, val);
	return n;
}

static DEVICE_ATTR(dbgmsk, 0644, dbgmsk_show, dbgmsk_set);
static DEVICE_ATTR(usbotgcontrol, 0644, bcmpmu_dbg_show_usbotgcontrol, bcmpmu_dbg_set_usbotgcontrol);
static DEVICE_ATTR(phy_cfg, 0644, bcmpmu_dbg_show_phy_cfg, bcmpmu_dbg_set_phy_cfg);
static DEVICE_ATTR(phy_p1ctl, 0644, bcmpmu_dbg_show_phy_p1ctl, bcmpmu_dbg_set_phy_p1ctl);
static DEVICE_ATTR(bc_status, 0644, bcmpmu_dbg_show_bc_status, NULL);
static DEVICE_ATTR(bc_cfg, 0644, bcmpmu_dbg_show_bc_cfg, bcmpmu_dbg_set_bc_cfg);

static DEVICE_ATTR(bb_usbotg_regs, 0644, bcmpmu_dbg_show_bb_usbotg_regs, NULL);
static DEVICE_ATTR(usb_otg_clkgate, 0644, bcmpmu_dbg_show_usb_otg_clkgate, NULL);

static DEVICE_ATTR(usb_set, 0644, NULL, bcmpmu_dbg_usb_set);
static DEVICE_ATTR(usb_get, 0644, NULL, bcmpmu_dbg_usb_get);

static struct attribute *bcmpmu_accy_attrs[] = {
	&dev_attr_dbgmsk.attr,
	&dev_attr_usbotgcontrol.attr,
	&dev_attr_phy_cfg.attr,
	&dev_attr_phy_p1ctl.attr,
	&dev_attr_bc_status.attr,
	&dev_attr_bc_cfg.attr,
	&dev_attr_bb_usbotg_regs.attr,
	&dev_attr_usb_otg_clkgate.attr,
	&dev_attr_usb_get.attr,
	&dev_attr_usb_set.attr,
	NULL
};

static const struct attribute_group bcmpmu_accy_attr_group = {
	.attrs = bcmpmu_accy_attrs,
};
#endif

static void usb_det_work(struct work_struct *work)
{
	int ret;
	enum bcmpmu_chrgr_type_t chrgr_type = PMU_USB_TYPE_NONE;
	enum bcmpmu_usb_type_t usb_type = PMU_CHRGR_TYPE_NONE;
	unsigned int bc_status;
	int vbus_status;
	struct bcmpmu_accy  *paccy =
		container_of(work, struct bcmpmu_accy, det_work.work);
	struct bcmpmu *bcmpmu = paccy->bcmpmu;

	ret = bcmpmu_usb_get(bcmpmu,
		BCMPMU_USB_CTRL_GET_VBUS_STATUS, (void *)&vbus_status);
	pr_accy(FLOW, "%s, enter state=%d, vbus=0x%X\n",
		__func__, paccy->det_state, vbus_status);

	switch (paccy->det_state) {
	case USB_IDLE:
		bcm_hsotgctrl_en_clock(true);
		pr_accy(FLOW, "%s, enable clock\n", __func__);
		paccy->clock_en = 1;
		msleep(1);
		bcm_hsotgctrl_bc_reset();
		paccy->retry_cnt = 0;
		paccy->det_state = USB_DETECT;
		schedule_delayed_work(&paccy->det_work, msecs_to_jiffies(100));
		break;
	case USB_DETECT:
		if (vbus_status != 0) {
			bc_status = readl(BB_BC_STATUS);
			pr_accy(FLOW, "%s, bc_status=0x%X, retry=%d\n",
				__func__, bc_status, paccy->retry_cnt);
			if (bc_status & BB_BC_STS_BC_DONE_MSK) {
				if (bc_status & BB_BC_STS_SDP_MSK) {
					usb_type = PMU_USB_TYPE_SDP;
					chrgr_type = PMU_CHRGR_TYPE_USB;
					paccy->det_state = USB_CONNECTED;
				} else if (bc_status & BB_BC_STS_CDP_MSK) {
					usb_type = PMU_USB_TYPE_CDP;
					chrgr_type = PMU_CHRGR_TYPE_USB;
					paccy->det_state = USB_CONNECTED;
				} else if (bc_status & BB_BC_STS_DCP_MSK) {
					usb_type = PMU_USB_TYPE_NONE;
					chrgr_type = PMU_CHRGR_TYPE_DCP;
					paccy->det_state = USB_CONNECTED;
				} else {
					paccy->det_state = USB_RETRY;
					schedule_delayed_work(&paccy->det_work,
						msecs_to_jiffies(0));
				}
			} else if ((bc_status & BB_BC_STS_DM_TO_MSK) ||
				(bc_status & BB_BC_STS_DP_TO_MSK) ||
				(bc_status & BB_BC_STS_DM_ERR_MSK) ||
				(bc_status & BB_BC_STS_DP_ERR_MSK)) {
				paccy->det_state = USB_RETRY;
				schedule_delayed_work(&paccy->det_work,
					msecs_to_jiffies(0));
			} else
				schedule_delayed_work(&paccy->det_work,
					msecs_to_jiffies(100));
		} else {
			usb_type = PMU_USB_TYPE_NONE;
			chrgr_type = PMU_CHRGR_TYPE_NONE;
			paccy->det_state = USB_IDLE;
		}
		break;

	case USB_RETRY:
		paccy->retry_cnt++;
		if (paccy->retry_cnt < 10) {
			paccy->det_state = USB_DETECT;
			bcm_hsotgctrl_bc_reset();
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
		if (vbus_status == 0) {
			usb_type = PMU_USB_TYPE_NONE;
			chrgr_type = PMU_CHRGR_TYPE_NONE;
			paccy->det_state = USB_IDLE;
		}
		break;
	default:
		break;
	}

	pr_accy(FLOW, "%s, exit state=%d, usb=%d, chrgr=%d\n",
		__func__, paccy->det_state, usb_type, chrgr_type);

	if (paccy->det_state == USB_IDLE)
		wake_unlock(&paccy->wake_lock);
	else
		wake_lock(&paccy->wake_lock);

	if ((paccy->det_state == USB_DETECT) ||
		(paccy->det_state == USB_RETRY))
		return;

	if (paccy->clock_en != 0) {
		bcm_hsotgctrl_en_clock(false);
		paccy->clock_en = 0;
		pr_accy(FLOW, "%s, disable clock\n", __func__);
	}
	if ((usb_type < PMU_USB_TYPE_MAX) &&
		(usb_type != bcmpmu->usb_accy_data.usb_type)) {
		if ((usb_type != PMU_USB_TYPE_NONE) &&
			(bcmpmu->usb_accy_data.usb_type != PMU_USB_TYPE_MAX))
			send_usb_event(paccy->bcmpmu,
				BCMPMU_USB_EVENT_USB_DETECTION,
				&usb_type);
		bcmpmu->usb_accy_data.usb_type = usb_type;
	}
	if ((chrgr_type < PMU_CHRGR_TYPE_MAX) &&
		(chrgr_type != bcmpmu->usb_accy_data.chrgr_type)) {
		bcmpmu->usb_accy_data.chrgr_type = chrgr_type;
		bcmpmu->usb_accy_data.max_curr_chrgr = chrgr_curr_lmt[chrgr_type];
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
		send_usb_event(data->bcmpmu, BCMPMU_USB_EVENT_ADP_CALIBRATION_DONE, NULL);
	}
}

int bcmpmu_usb_set(struct bcmpmu *bcmpmu,
			enum bcmpmu_usb_ctrl_t ctrl, unsigned long data)
{
	struct bcmpmu_accy *paccy = (struct bcmpmu_accy *)bcmpmu->accyinfo;
	int ret = 0;
	pr_accy(FLOW, "%s, ctrl=%d, val=0x%lX\n", __func__, ctrl, data);

	switch(ctrl) {
	case BCMPMU_USB_CTRL_CHRG_CURR_LMT:
		paccy->bcmpmu->usb_accy_data.max_curr_chrgr = (int)data;
		send_chrgr_event(paccy->bcmpmu, BCMPMU_CHRGR_EVENT_CHRG_CURR_LMT,
			&paccy->bcmpmu->usb_accy_data.max_curr_chrgr);
		break;
		
	case BCMPMU_USB_CTRL_VBUS_ON_OFF:
		if (data == 0)
		ret = bcmpmu->write_dev(bcmpmu, PMU_REG_OTG_VBUS_BOOST,
				0, bcmpmu->regmap[PMU_REG_OTG_VBUS_BOOST].mask);
		else
		ret = bcmpmu->write_dev(bcmpmu, PMU_REG_OTG_VBUS_BOOST,
				bcmpmu->regmap[PMU_REG_OTG_VBUS_BOOST].mask,
				bcmpmu->regmap[PMU_REG_OTG_VBUS_BOOST].mask);
		break;

	case BCMPMU_USB_CTRL_SET_VBUS_DEB_TIME:
		ret = bcmpmu->write_dev(bcmpmu, PMU_REG_ADP_COMP_DB_TM,
			data << bcmpmu->regmap[PMU_REG_ADP_COMP_DB_TM].shift,
			bcmpmu->regmap[PMU_REG_ADP_COMP_DB_TM].mask);
		break;

	case BCMPMU_USB_CTRL_SRP_VBUS_PULSE:
		if (data == 0)
			ret = bcmpmu->write_dev(bcmpmu, PMU_REG_OTG_VBUS_PULSE,
				0, bcmpmu->regmap[PMU_REG_OTG_VBUS_PULSE].mask);
		else
			ret = bcmpmu->write_dev(bcmpmu, PMU_REG_OTG_VBUS_PULSE,
				bcmpmu->regmap[PMU_REG_OTG_VBUS_PULSE].mask,
				bcmpmu->regmap[PMU_REG_OTG_VBUS_PULSE].mask);
		break;

	case BCMPMU_USB_CTRL_DISCHRG_VBUS:
		if (data == 0)
		ret = bcmpmu->write_dev(bcmpmu, PMU_REG_OTG_VBUS_DISCHRG,
				0, bcmpmu->regmap[PMU_REG_OTG_VBUS_DISCHRG].mask);
		else
		ret = bcmpmu->write_dev(bcmpmu, PMU_REG_OTG_VBUS_DISCHRG,
				bcmpmu->regmap[PMU_REG_OTG_VBUS_DISCHRG].mask,
				bcmpmu->regmap[PMU_REG_OTG_VBUS_DISCHRG].mask);
		break;

	case BCMPMU_USB_CTRL_START_STOP_ADP_SENS_PRB:
		if (data == 0)
			ret = bcmpmu->write_dev(bcmpmu, PMU_REG_ADP_SENSE,
				0,
				bcmpmu->regmap[PMU_REG_ADP_SENSE].mask);
		else {
			if (paccy->otg_block_enabled == 0) {
				ret = bcmpmu->write_dev(bcmpmu, PMU_REG_OTG_ENABLE,
					bcmpmu->regmap[PMU_REG_OTG_ENABLE].mask,
					bcmpmu->regmap[PMU_REG_OTG_ENABLE].mask);
				paccy->otg_block_enabled = 1;
			}
			if (paccy->adp_block_enabled == 0) {
				ret = bcmpmu->write_dev(bcmpmu, PMU_REG_ADP_ENABLE,
					bcmpmu->regmap[PMU_REG_ADP_ENABLE].mask,
					bcmpmu->regmap[PMU_REG_ADP_ENABLE].mask);
				paccy->adp_block_enabled = 1;
			}
			if (paccy->adp_sns_comp == 0) {
				ret = bcmpmu->write_dev(bcmpmu, PMU_REG_ADP_SNS_COMP,
					bcmpmu->regmap[PMU_REG_ADP_SNS_COMP].mask,
					bcmpmu->regmap[PMU_REG_ADP_SNS_COMP].mask);
				paccy->adp_sns_comp = 1;
			}
			ret = bcmpmu->write_dev(bcmpmu, PMU_REG_ADP_SENSE,
				bcmpmu->regmap[PMU_REG_ADP_SENSE].mask,
				bcmpmu->regmap[PMU_REG_ADP_SENSE].mask);
		}
		break;

	case BCMPMU_USB_CTRL_START_STOP_ADP_PRB:
		if (data == 0)
			ret = bcmpmu->write_dev(bcmpmu, PMU_REG_ADP_PRB,
				0,
				bcmpmu->regmap[PMU_REG_ADP_PRB].mask);
		else {
			if (paccy->otg_block_enabled == 0) {
				ret = bcmpmu->write_dev(bcmpmu, PMU_REG_OTG_ENABLE,
					bcmpmu->regmap[PMU_REG_OTG_ENABLE].mask,
					bcmpmu->regmap[PMU_REG_OTG_ENABLE].mask);
				paccy->otg_block_enabled = 1;
			}
			if (paccy->adp_block_enabled == 0) {
				ret = bcmpmu->write_dev(bcmpmu, PMU_REG_ADP_ENABLE,
					bcmpmu->regmap[PMU_REG_ADP_ENABLE].mask,
					bcmpmu->regmap[PMU_REG_ADP_ENABLE].mask);
				paccy->adp_block_enabled = 1;
			}
			if (paccy->adp_prob_comp == 0) {
				ret = bcmpmu->write_dev(bcmpmu, PMU_REG_ADP_PRB_COMP,
					bcmpmu->regmap[PMU_REG_ADP_PRB_COMP].mask,
					bcmpmu->regmap[PMU_REG_ADP_PRB_COMP].mask);
				paccy->adp_prob_comp = 1;
			}

			ret = bcmpmu->write_dev(bcmpmu, PMU_REG_ADP_PRB,
				bcmpmu->regmap[PMU_REG_ADP_PRB].mask,
				bcmpmu->regmap[PMU_REG_ADP_PRB].mask);
		}
		break;

	case BCMPMU_USB_CTRL_START_ADP_CAL_PRB:
		if (paccy->otg_block_enabled == 0) {
			ret = bcmpmu->write_dev(bcmpmu, PMU_REG_OTG_ENABLE,
				bcmpmu->regmap[PMU_REG_OTG_ENABLE].mask,
				bcmpmu->regmap[PMU_REG_OTG_ENABLE].mask);
			paccy->otg_block_enabled = 1;
		}
		if (paccy->adp_block_enabled == 0) {
			ret = bcmpmu->write_dev(bcmpmu, PMU_REG_ADP_ENABLE,
				bcmpmu->regmap[PMU_REG_ADP_ENABLE].mask,
				bcmpmu->regmap[PMU_REG_ADP_ENABLE].mask);
			paccy->adp_block_enabled = 1;
		}
		if (paccy->adp_prob_comp == 0) {
			ret = bcmpmu->write_dev(bcmpmu, PMU_REG_ADP_PRB_COMP,
				bcmpmu->regmap[PMU_REG_ADP_PRB_COMP].mask,
				bcmpmu->regmap[PMU_REG_ADP_PRB_COMP].mask);
			paccy->adp_prob_comp = 1;
		}
		ret = bcmpmu->write_dev(bcmpmu, PMU_REG_ADP_PRB_MOD,
			PMU_USB_ADP_MODE_CALIBRATE <<
				bcmpmu->regmap[PMU_REG_ADP_PRB_MOD].shift,
			bcmpmu->regmap[PMU_REG_ADP_PRB_MOD].mask);
		ret = bcmpmu->write_dev(bcmpmu, PMU_REG_ADP_PRB,
			bcmpmu->regmap[PMU_REG_ADP_PRB].mask,
			bcmpmu->regmap[PMU_REG_ADP_PRB].mask);
		/* start timer */
		schedule_delayed_work(&paccy->adp_work, msecs_to_jiffies(15));
		break;

	case BCMPMU_USB_CTRL_SET_ADP_PRB_MOD:
		ret = bcmpmu->write_dev(bcmpmu, PMU_REG_ADP_PRB_MOD,
			data<<bcmpmu->regmap[PMU_REG_ADP_PRB_MOD].shift,
			bcmpmu->regmap[PMU_REG_ADP_PRB_MOD].mask);
		break;

	case BCMPMU_USB_CTRL_SET_ADP_PRB_CYC_TIME:
		ret = bcmpmu->write_dev(bcmpmu, PMU_REG_ADP_PRB_CYC_TIME,
			data<<bcmpmu->regmap[PMU_REG_ADP_PRB_CYC_TIME].shift,
			bcmpmu->regmap[PMU_REG_ADP_PRB_CYC_TIME].mask);
		break;

	case BCMPMU_USB_CTRL_SET_ADP_COMP_METHOD:
		ret = bcmpmu->write_dev(bcmpmu, PMU_REG_ADP_COMP_METHOD,
			data<<bcmpmu->regmap[PMU_REG_ADP_COMP_METHOD].shift,
			bcmpmu->regmap[PMU_REG_ADP_COMP_METHOD].mask);
		break;

	case BCMPMU_USB_CTRL_SW_UP:
		if (data == 0) {
			ret = bcmpmu->write_dev(bcmpmu, PMU_REG_SWUP,
				0,
				bcmpmu->regmap[PMU_REG_SWUP].mask);
		} else {
			ret = bcmpmu->write_dev(bcmpmu, PMU_REG_SWUP,
				bcmpmu->regmap[PMU_REG_SWUP].mask,
				bcmpmu->regmap[PMU_REG_SWUP].mask);
		}
		break;

	default:
		ret = -EINVAL;
		break;
	}
	return ret;
}

int bcmpmu_usb_get(struct bcmpmu *bcmpmu,
			enum bcmpmu_usb_ctrl_t ctrl, void *data)
{
	struct bcmpmu_accy *paccy = (struct bcmpmu_accy *)bcmpmu->accyinfo;
	unsigned int val, val1;
	int ret;
	switch(ctrl) {
	case BCMPMU_USB_CTRL_GET_ADP_CHANGE_STATUS:
		ret = bcmpmu->read_dev(bcmpmu,
			PMU_REG_ADP_STATUS_ATTACH_DET,
			&val,
			bcmpmu->regmap[PMU_REG_ADP_STATUS_ATTACH_DET].mask);
		val = val>>bcmpmu->regmap[PMU_REG_ADP_STATUS_ATTACH_DET].shift;
		break;
	case BCMPMU_USB_CTRL_GET_ADP_SENSE_STATUS:
		ret = bcmpmu->read_dev(bcmpmu,
			PMU_REG_ADP_STATUS_SNS_DET,
			&val,
			bcmpmu->regmap[PMU_REG_ADP_STATUS_SNS_DET].mask);
		val = val>>bcmpmu->regmap[PMU_REG_ADP_STATUS_SNS_DET].shift;
		break;
	case BCMPMU_USB_CTRL_GET_ADP_PRB_RISE_TIMES:
		ret = bcmpmu->read_dev(bcmpmu,
			PMU_REG_ADP_STATUS_RISE_TIMES_LSB,
			&val,
			bcmpmu->regmap[PMU_REG_ADP_STATUS_RISE_TIMES_LSB].mask);
		val = val>>bcmpmu->regmap[PMU_REG_ADP_STATUS_RISE_TIMES_LSB].shift;
		if (ret != 0) break;
		ret = bcmpmu->read_dev(bcmpmu,
			PMU_REG_ADP_STATUS_RISE_TIMES_MSB,
			&val1,
			bcmpmu->regmap[PMU_REG_ADP_STATUS_RISE_TIMES_MSB].mask);
		val1 = val1>>bcmpmu->regmap[PMU_REG_ADP_STATUS_RISE_TIMES_MSB].shift;
		val = val | val1<<8;
		break;
	case BCMPMU_USB_CTRL_GET_VBUS_STATUS:
		ret = bcmpmu->read_dev(bcmpmu,
			PMU_REG_OTG_STATUS_VBUS,
			&val,
			bcmpmu->regmap[PMU_REG_OTG_STATUS_VBUS].mask);
			val = val>>bcmpmu->regmap[PMU_REG_OTG_STATUS_VBUS].shift;
		break;
	case BCMPMU_USB_CTRL_GET_SESSION_STATUS:
		ret = bcmpmu->read_dev(bcmpmu,
			PMU_REG_OTG_STATUS_SESS,
			&val,
			bcmpmu->regmap[PMU_REG_OTG_STATUS_SESS].mask);
			val = val>>bcmpmu->regmap[PMU_REG_OTG_STATUS_SESS].shift;
		break;
	case BCMPMU_USB_CTRL_GET_SESSION_END_STATUS:
		ret = bcmpmu->read_dev(bcmpmu,
			PMU_REG_OTG_STATUS_SESS_END,
			&val,
			bcmpmu->regmap[PMU_REG_OTG_STATUS_SESS_END].mask);
			val = val>>bcmpmu->regmap[PMU_REG_OTG_STATUS_SESS_END].shift;
		break;
	case BCMPMU_USB_CTRL_GET_ID_VALUE:
		ret = bcmpmu->read_dev(bcmpmu,
			PMU_REG_USB_STATUS_ID_CODE,
			&val,
			bcmpmu->regmap[PMU_REG_USB_STATUS_ID_CODE].mask);
			val = val>>bcmpmu->regmap[PMU_REG_USB_STATUS_ID_CODE].shift;
			if ((paccy->usb_id_map) &&
				(val < paccy->usb_id_map_len))
				val = paccy->usb_id_map[val];
		break;
	default:
		ret = -EINVAL;
		break;
	}
	if (ret == 0)
		*(unsigned int*)data = val;

	pr_accy(FLOW, "%s, ctrl=%d, val=0x%X\n", __func__, ctrl, *(unsigned int*)data);
	return ret;
}

static int bcmpmu_register_usb_callback(struct bcmpmu *pmu,
	void (*callback)(struct bcmpmu *pmu,
		unsigned char event, void *, void *),
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
	bcmpmu->usb_accy_data.usb_type = PMU_USB_TYPE_MAX;
	bcmpmu->usb_accy_data.chrgr_type = PMU_CHRGR_TYPE_MAX;
	bcmpmu->usb_accy_data.max_curr_chrgr = 0;

	paccy->usb_cb.callback = NULL;
	paccy->usb_cb.clientdata = NULL;
	paccy->adp_cal_done = 0;
	paccy->det_state = USB_DETECT;
	paccy->usb_id_map = bcmpmu_get_usb_id_map(&paccy->usb_id_map_len);
	paccy->otg_block_enabled = 0;
	paccy->adp_block_enabled = 0;
	paccy->adp_prob_comp = 0;
	paccy->adp_sns_comp = 0;

	INIT_DELAYED_WORK(&paccy->adp_work, usb_adp_work);
	INIT_DELAYED_WORK(&paccy->det_work, usb_det_work);
	wake_lock_init(&paccy->wake_lock, WAKE_LOCK_SUSPEND, "usb_accy");
	for (i = 0; i < BCMPMU_EVENT_MAX; i++) {
		paccy->event[i].event_id = i;
		BLOCKING_INIT_NOTIFIER_HEAD(&paccy->event[i].notifiers);
	}

	bcmpmu->register_irq(bcmpmu, PMU_IRQ_USBINS, bcmpmu_accy_isr, paccy);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_USBRM, bcmpmu_accy_isr, paccy);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_CHGDET_LATCH, bcmpmu_accy_isr, paccy);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_CHGDET_TO, bcmpmu_accy_isr, paccy);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_VBUS_1V5_R, bcmpmu_accy_isr, paccy);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_VBUS_4V5_R, bcmpmu_accy_isr, paccy);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_VBUS_1V5_F, bcmpmu_accy_isr, paccy);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_VBUS_4V5_F, bcmpmu_accy_isr, paccy);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_IDCHG, bcmpmu_accy_isr, paccy);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_ADP_CHANGE, bcmpmu_accy_isr, paccy);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_ADP_SNS_END, bcmpmu_accy_isr, paccy);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_SESSION_END_VLD, bcmpmu_accy_isr, paccy);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_SESSION_END_INVLD, bcmpmu_accy_isr, paccy);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_VBUS_OVERCURRENT, bcmpmu_accy_isr, paccy);

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
#ifdef CONFIG_MFD_BCMPMU_DBG
	ret = sysfs_create_group(&pdev->dev.kobj, &bcmpmu_accy_attr_group);
#endif
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
	cancel_delayed_work_sync(&paccy->det_work);
	cancel_delayed_work_sync(&paccy->adp_work);
	wake_lock_destroy(&paccy->wake_lock);

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
