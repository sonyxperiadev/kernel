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
#define MAX_EVENT_QUEUE 30
#define BC_MAX_RETRIES	10
#define TYPE2_RETRY 1

#ifdef CONFIG_RT8973
#define ACCY_WORK_DELAY  msecs_to_jiffies(500)
#else
#define ACCY_WORK_DELAY  msecs_to_jiffies(5)
#endif
#define ACCY_RETRY_DELAY msecs_to_jiffies(500)

static int debug_mask = BCMPMU_PRINT_ERROR |
	BCMPMU_PRINT_INIT |  BCMPMU_PRINT_FLOW;
static int prev_chrgr_type;
static u32 bc_start;

#define pr_accy(debug_level, args...) \
	do { \
		if (debug_mask & BCMPMU_PRINT_##debug_level) { \
			pr_info(args); \
		} \
	} while (0)



/* TPROBE_MAX definition */
#define TPROBE_MAX_USEC			16000
#define TPROBE_MAX_MSB_MASK		0xC0
#define TPROBE_MAX_DATA(msb, lsb)	((((msb >> 6) & 0x3) << 8) | \
		(lsb & 0xFF))
#define TPROBE_MAX_GET_MSB(data)	(((data >> 8) & 0x3) << 6)
#define TPROBE_MAX_GET_LSB(data)	(data & 0xFF)

/* pattern to write before accessing PMU BC CTRL reg */
#define PMU_BC_CTRL_OVWR_PATTERN       0x5

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
	[PMU_CHRGR_TYPE_SDP] = 500,
	[PMU_CHRGR_TYPE_CDP] = 1500,
	[PMU_CHRGR_TYPE_DCP] = 1500,
	[PMU_CHRGR_TYPE_TYPE1] = 700,
	[PMU_CHRGR_TYPE_TYPE2] = 700,
	[PMU_CHRGR_TYPE_PS2] = 100,
	[PMU_CHRGR_TYPE_ACA] = 1000,
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


static struct bcmpmu_accy *bcmpmu_accy;
static void bcmpmu_accy_isr(enum bcmpmu59xxx_irq irq, void *data);
static void usb_handle_state(struct bcmpmu_accy *paccy);
static void bcmpmu_notify_charger_state(struct bcmpmu_accy *paccy);
static void bcmpmu_accy_set_pmu_BC12
	(struct bcmpmu59xxx *bcmpmu, int val);
static void bcmpmu_accy_check_BC12_EN(struct bcmpmu59xxx *bcmpmu);
static void bcmpmu_paccy_latch_event(struct bcmpmu_accy *accy,
		u32 event, void *para);
static void bcdldo_cycle_power(struct bcmpmu_accy *paccy);

const int *bcmpmu_get_usb_id_map(struct bcmpmu59xxx *bcmpmu, int *len)
{
	*len = PMU_USB_ID_LVL_MAX;
	return bcmpmu_usb_id_map;
}
static bool bcmpmu_supported_chrgr_usbid
		(struct bcmpmu_accy *paccy, int usb_id)
{
	bool id_valid = true;

	if (paccy->bc == BC_EXT_DETECT)
		return id_valid;

	switch (usb_id) {
	case PMU_USB_ID_GROUND:
	case PMU_USB_ID_RID_A:
	case PMU_USB_ID_RID_C:
	case PMU_USB_ID_RID_B:
	case PMU_USB_ID_RESERVED1:
	case PMU_USB_ID_RESERVED2:
		id_valid = false;
	break;
	case PMU_USB_ID_FLOAT:
		id_valid = true;
		break;
	default:
		id_valid = false;
	}
	if (!id_valid)
		pr_accy(FLOW, "----Charger not supported\n");
	return id_valid;
}

static void bcmpmu_set_chrgr_type(struct bcmpmu_accy *paccy, int chrgr)
{
	unsigned long flags;
	spin_lock_irqsave(&paccy->accy_lock, flags);
	paccy->usb_accy_data.chrgr_type = chrgr;
	spin_unlock_irqrestore(&paccy->accy_lock, flags);
}

int bcmpmu_get_chrgr_type(struct bcmpmu_accy *paccy)
{
	int chrgr = 0;
	unsigned long flags;
	spin_lock_irqsave(&paccy->accy_lock, flags);
	chrgr = paccy->usb_accy_data.chrgr_type;
	spin_unlock_irqrestore(&paccy->accy_lock, flags);
	return chrgr;
}
EXPORT_SYMBOL(bcmpmu_get_chrgr_type);

static void enable_bc_clock(struct bcmpmu_accy *paccy, bool en)
{
	if (paccy->bc == BCMPMU_BC_BB_BC12) {
		bcm_hsotgctrl_en_clock(en);
		paccy->clock_en = en;
		pr_accy(FLOW, "======<%s> paccy clock %x\n"
			, __func__, paccy->clock_en);
	}
}

static void reset_bc(struct bcmpmu_accy *paccy)
{
	if (paccy->bc == BCMPMU_BC_BB_BC12)
		bcm_hsotgctrl_bc_reset();
}

static int is_bb_detection_done(u32 status)
{
	int ret_status = 0;
	if (status & BB_BC_STS_BC_DONE_MSK) {
		/*Make sure that only one type bit is set*/
		u32 type = status & 0x7F;
		if (type && ((type & (type - 1)) == 0))
			ret_status = 1;
	}

	return ret_status;

}

/* for external charger detection  apart from PMU/BB*/
__weak int bcm_ext_bc_status(void)
{
	return 0;
}

static unsigned int get_bc_status(struct bcmpmu_accy *paccy)
{
	u32 status = 0;
	unsigned long st;
	struct bcmpmu59xxx *bcmpmu = paccy->bcmpmu;

	if (paccy->bc == BCMPMU_BC_PMU_BC12) {
		bcmpmu->read_dev(bcmpmu,
				PMU_REG_MBCCTRL5, (u8 *)&status);
		pr_accy(FLOW, " MBCCTRL5 %x\n", status);
		status &= MBCCTRL5_CHP_TYP_MASK;
		pr_accy(FLOW, "MBCCTRL5 %x  mask %x\n",
				status, MBCCTRL5_CHP_TYP_MASK);
		status = status >> MBCCTRL5_CHP_TYP_SHIFT;
		pr_accy(FLOW, " MBCCTRL5 %x shift % d\n",
				status, MBCCTRL5_CHP_TYP_SHIFT);
		return status;
	} else if (paccy->bc == BCMPMU_BC_BB_BC12) {
		bcm_hsotgctrl_bc_status(&st);
		pr_accy(FLOW, "<%s> = status %x\n", __func__, (u32)st);
		if (is_bb_detection_done(st))
			return (u32)st;
		else
			return 0;
	} else if (paccy->bc == BC_EXT_DETECT) {
		status = bcm_ext_bc_status();
	}

	return status;
}

/* for external charger detection  apart from PMU/BB*/
__weak enum bcmpmu_chrgr_type_t
get_ext_charger_type(struct bcmpmu_accy *paccy, unsigned int bc_status)
{
	return 0;
}

static enum bcmpmu_chrgr_type_t get_charger_type(struct bcmpmu_accy *paccy,
		unsigned int bc_status)
{
	enum bcmpmu_chrgr_type_t type;
	u8 chp_typ;
	if (paccy->bc == BCMPMU_BC_PMU_BC12) {
		chp_typ =  (bc_status & PMU_CHP_TYPE_MASK);
		if (chp_typ) {
			if (chp_typ == PMU_BC_DCP)
				type = PMU_CHRGR_TYPE_DCP;
			else if (chp_typ == PMU_BC_SDP)
				type = PMU_CHRGR_TYPE_SDP;
			else if (chp_typ == PMU_BC_CDP)
				type = PMU_CHRGR_TYPE_CDP;
		} else
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

	} else if (paccy->bc == BC_EXT_DETECT) {
		type = get_ext_charger_type(paccy, bc_status);
	} else
		type = PMU_CHRGR_TYPE_NONE;
	return type;
}

int bcmpmu_check_vbus()
{
	struct bcmpmu59xxx *bcmpmu = bcmpmu_accy->bcmpmu;
	u8 reg;

	if (!bcmpmu) {
		pr_accy(ERROR, "%s: BCMPMU Accy driver not initialized\n",
			__func__);
		return -1;
	}

	bcmpmu->read_dev(bcmpmu, PMU_REG_ENV2, &reg);

	if (reg)
		return 1;
	else
		return 0;

}
EXPORT_SYMBOL_GPL(bcmpmu_check_vbus);

static void send_usb_event(struct bcmpmu59xxx *pmu,
		enum bcmpmu_event_t event, void *para)
{
	struct bcmpmu_accy *paccy = (struct bcmpmu_accy *)pmu->accyinfo;
	pr_accy(FLOW, "Event send %x\n", event);
	blocking_notifier_call_chain(&paccy->bcmpmu->event[event].notifiers,
			event, para);
}

char *get_supply_type_str(int chrgr_type)
{
	switch (chrgr_type) {
	case PMU_CHRGR_TYPE_SDP:
	case PMU_CHRGR_TYPE_CDP:
	case PMU_CHRGR_TYPE_ACA:
		return"bcmpmu_usb";

	case PMU_CHRGR_TYPE_DCP:
	case PMU_CHRGR_TYPE_TYPE1:
	case PMU_CHRGR_TYPE_TYPE2:
	case PMU_CHRGR_TYPE_PS2:
		return "bcmpmu_ac";

	default:
		break;
	}

	return NULL;
}
EXPORT_SYMBOL_GPL(get_supply_type_str);

u32 bcmpmu_get_chrgr_curr_lmt(u32 chrgr_type)
{
	if (PMU_CHRGR_TYPE_MAX > chrgr_type)
		return chrgr_curr_lmt[chrgr_type];
	return 0;
}
EXPORT_SYMBOL_GPL(bcmpmu_get_chrgr_curr_lmt);

static void send_chrgr_event(struct bcmpmu59xxx *pmu,
		enum bcmpmu_event_t event, void *para)
{
	struct bcmpmu_accy *paccy = (struct bcmpmu_accy *)pmu->accyinfo;

	pr_accy(FLOW, "Event send %x\n", event);
	blocking_notifier_call_chain(&paccy->bcmpmu->event[event].notifiers,
			event, para);
}

static void paccy_set_ldo_bit(struct bcmpmu_accy *paccy, int val)
{
	u8 status;
	paccy->bcmpmu->read_dev(paccy->bcmpmu,
			PMU_REG_MBCCTRL5, &status);
	if (val)
		status |= USB_DET_LDO_EN_MASK;
	else
		status &= ~USB_DET_LDO_EN_MASK;
	paccy->bcmpmu->write_dev(paccy->bcmpmu,
			PMU_REG_MBCCTRL5, status);
	pr_accy(FLOW, "###<%s> MBCCTRL5 %x\n", __func__, status);
}

static int paccy_get_ldo_bit(struct bcmpmu_accy *paccy)
{
	u8 reg = 0;
	paccy->bcmpmu->read_dev(paccy->bcmpmu,
			PMU_REG_MBCCTRL5, &reg);

	pr_accy(FLOW, "###<%s> MBCCTRL5 %x\n",
			__func__, reg);

	return (reg & USB_DET_LDO_EN_MASK) ? 1 : 0;
}


static void usb_detect_state(struct bcmpmu_accy *paccy)
{
	enum bcmpmu_chrgr_type_t chrgr_type = PMU_USB_TYPE_NONE;
	enum bcmpmu_usb_type_t usb_type = PMU_CHRGR_TYPE_NONE;
	u32 vbus_status = 0, id_status = 0, bc_status;
	struct bcmpmu59xxx *bcmpmu = paccy->bcmpmu;
	static u8 type2;
	bool id_check = true;
	bcmpmu_usb_get(bcmpmu,
			BCMPMU_USB_CTRL_GET_SESSION_STATUS,
			(void *)&vbus_status);
	pr_accy(FLOW, "%s, vbus %d\n",
			__func__, vbus_status);
	if (paccy->bc != BC_EXT_DETECT) {
		bcmpmu_usb_get(bcmpmu,
			BCMPMU_USB_CTRL_GET_ID_VALUE,
			(void *)&id_status);
		pr_accy(FLOW, "%s, vbus %d, id %d\n",
			__func__, vbus_status, id_status);
		id_check = bcmpmu_supported_chrgr_usbid(paccy, id_status);
	}

	if (vbus_status && id_check) {
		bc_status = get_bc_status(paccy);
		pr_accy(FLOW, "%s, bc_status=0x%X, retry=%d\n",
				__func__, bc_status, paccy->retry_cnt);
		if (bc_status) {
			chrgr_type = get_charger_type(paccy,
					bc_status);
			prev_chrgr_type = chrgr_type;
			log_charger_type(chrgr_type);
			if (chrgr_type != PMU_CHRGR_TYPE_NONE) {
				if ((chrgr_type == PMU_CHRGR_TYPE_TYPE2) &&
					(type2 < TYPE2_RETRY) &&
					(paccy->retry_cnt < BC_MAX_RETRIES)) {
						paccy->det_state = USB_RETRY;
						chrgr_type =
							PMU_CHRGR_TYPE_NONE;
						type2++;
				} else {
					type2 = 0;
					paccy->det_state = USB_CONNECTED;
#ifdef CONFIG_HAS_WAKELOCK
					if (!wake_lock_active
						(&paccy->wake_lock))
						wake_lock(&paccy->wake_lock);
#endif

#ifdef CONFIG_KONA_PI_MGR
				pi_mgr_qos_request_update(&paccy->qos,
							DEEP_SLEEP_LATENCY);
#endif
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
			}
		} else {
				paccy->det_state = USB_RETRY;
			}
		} else {
			paccy->det_state = USB_RETRY;
		}
		paccy->usb_accy_data.usb_type = usb_type;
		bcmpmu_set_chrgr_type(paccy, chrgr_type);
	} else {
		paccy->usb_accy_data.usb_type = PMU_USB_TYPE_NONE;
		bcmpmu_set_chrgr_type(paccy, PMU_CHRGR_TYPE_NONE);
		paccy->det_state = USB_IDLE;
	}

	pr_accy(FLOW, "<%s>det_state %d chrgr_type %d\n",
			__func__, paccy->det_state,
			paccy->usb_accy_data.chrgr_type);
	usb_handle_state(paccy);

}

static void usb_handle_state(struct bcmpmu_accy *paccy)
{
	int id_status;
	bool id_check = true;
	switch (paccy->det_state) {
	case USB_DETECT:
		if (!atomic_read(&paccy->usb_allow_bc_detect))
			break;
		if (paccy->bc != BC_EXT_DETECT) {
			bcmpmu_usb_get(paccy->bcmpmu,
				BCMPMU_USB_CTRL_GET_ID_VALUE,
				(void *)&id_status);
			pr_accy(FLOW, "%s,id =%d\n",
				__func__, id_status);
			id_check = bcmpmu_supported_chrgr_usbid
				(paccy, id_status);
		}
		if (!id_check)
			break;
		if (!paccy->clock_en)
			enable_bc_clock(paccy, 1);
		if (paccy->bc == BCMPMU_BC_BB_BC12) {
			bc_start = PMU_BC_DETECTION_START;
			bcmpmu_call_notifier(paccy->bcmpmu,
					BCMPMU_USB_EVENT_USB_DETECTION,
					&bc_start);
		}
		usb_detect_state(paccy);
		break;

	case USB_IDLE:
		if (paccy->clock_en)
			enable_bc_clock(paccy, 0);
		if (bc_start == PMU_BC_DETECTION_START) {
			bc_start = PMU_BC_DETECTION_END;
			bcmpmu_call_notifier(paccy->bcmpmu,
					BCMPMU_USB_EVENT_USB_DETECTION,
					&bc_start);
		}
		break;

	case USB_RETRY:
		pr_accy(FLOW, "%s,  retry =%d\n",
					__func__, paccy->retry_cnt);
		if (paccy->retry_cnt < BC_MAX_RETRIES) {
			if (paccy->bc != BC_EXT_DETECT) {
				bcdldo_cycle_power(paccy);
				reset_bc(paccy);
			}
			schedule_delayed_work(&paccy->det_work,
						ACCY_RETRY_DELAY);
			paccy->retry_cnt++;
		} else {
			pr_accy(ERROR, "%s, failed, retry times=%d\n",
					__func__, paccy->retry_cnt);
			paccy->usb_accy_data.usb_type = PMU_USB_TYPE_NONE;
			bcmpmu_set_chrgr_type(paccy, PMU_CHRGR_TYPE_NONE);
			paccy->det_state = USB_IDLE;
			paccy->retry_cnt = 0;
			if (bc_start == PMU_BC_DETECTION_START) {
				bc_start = PMU_BC_DETECTION_END;
				bcmpmu_call_notifier(paccy->bcmpmu,
					BCMPMU_USB_EVENT_USB_DETECTION,
					&bc_start);
			}

		}
		break;

	case USB_CONNECTED:
		pr_accy(FLOW, "*** Charger Connected event\n");
		if (bc_start == PMU_BC_DETECTION_START) {
			bc_start = PMU_BC_DETECTION_END;
			bcmpmu_call_notifier(paccy->bcmpmu,
					BCMPMU_USB_EVENT_USB_DETECTION,
					&bc_start);
		}
		if (paccy->clock_en)
			enable_bc_clock(paccy, 0);
		if (paccy->bc != BC_EXT_DETECT)
			bcmpmu_accy_set_pmu_BC12(paccy->bcmpmu, 0);
		bcmpmu_notify_charger_state(paccy);
		schedule_delayed_work(&paccy->det_work, ACCY_WORK_DELAY);
		break;

	case USB_DISCONNECTED:
		pr_accy(FLOW, "*** Charger disconnected event\n");
		if (bc_start == PMU_BC_DETECTION_START) {
			bc_start = PMU_BC_DETECTION_END;
			bcmpmu_call_notifier(paccy->bcmpmu,
					BCMPMU_USB_EVENT_USB_DETECTION,
					&bc_start);
		}
		paccy->retry_cnt = 0;
		paccy->det_state = USB_IDLE;
		paccy->usb_accy_data.usb_type = PMU_USB_TYPE_NONE;
		bcmpmu_set_chrgr_type(paccy, PMU_CHRGR_TYPE_NONE);
		bcmpmu_notify_charger_state(paccy);
		/* Disable BC12 for cust platform */
		if (paccy->bc != BC_EXT_DETECT) {
			bcdldo_cycle_power(paccy);
			reset_bc(paccy);
			bcmpmu_accy_set_pmu_BC12(paccy->bcmpmu, 1);
		}

#ifdef CONFIG_HAS_WAKELOCK
		if (wake_lock_active(&paccy->wake_lock))
			wake_unlock(&paccy->wake_lock);
		/* Make sure the app can process the
		*  uevent of usb_disconnect
		*/
		wake_lock_timeout(&paccy->usb_plug_out_wake_lock, HZ * 2);
#endif
#ifdef CONFIG_KONA_PI_MGR
		pi_mgr_qos_request_update(&paccy->qos,
					PI_MGR_QOS_DEFAULT_VALUE);
#endif
		schedule_delayed_work(&paccy->det_work, ACCY_WORK_DELAY);
		break;

	default:
		break;
	}

}

static void bcmpmu_paccy_latch_event(struct bcmpmu_accy *accy,
		u32 event, void *para)
{
	unsigned long flags;
	struct event_list *entry;
	if (!list_empty(&accy->free_list->node)) {
		entry = list_first_entry(&accy->free_list->node,
			struct event_list, node);
		spin_lock_irqsave(&accy->accy_lock, flags);
		list_del(&entry->node);
		/* Add to work List */
		entry->event = event;
		entry->para = para;
		list_add_tail(&entry->node, &accy->dispatch_list->node);
		spin_unlock_irqrestore(&accy->accy_lock, flags);
		pr_accy(FLOW, "Event Latch %x para %p\n", event, para);
	} else {
		flush_delayed_work(&accy->det_work);
		entry = list_first_entry(&accy->free_list->node,
			struct event_list, node);
		spin_lock_irqsave(&accy->accy_lock, flags);
		list_del(&entry->node);
		/* Add to work List */
		entry->event = event;
		list_add_tail(&entry->node, &accy->dispatch_list->node);
		spin_unlock_irqrestore(&accy->accy_lock, flags);
		pr_accy(FLOW, "Event Latch %x\n", event);
	}
}

static void bcmpmu_accy_isr(enum bcmpmu59xxx_irq irq, void *data)
{
	struct bcmpmu_accy *paccy = data;
	struct bcmpmu59xxx *bcmpmu = paccy->bcmpmu;
	int board_id = bcmpmu->pdata->board_id;
	int ret, id_status;

	pr_accy(INIT, "#### %s interrupt = %d  det state %d\n",
		__func__, irq, paccy->det_state);
	if (paccy->bc == BC_EXT_DETECT)
		bcmpmu_accy_check_BC12_EN(bcmpmu);

	switch (irq) {
	case PMU_IRQ_VA_SESS_VALID_R:
		bc_start = 0;
		pr_accy(INIT, "### ISR  PMU_IRQ_VA_SESS_VALID_R: %x\n",
			PMU_IRQ_VA_SESS_VALID_R);

		bcmpmu_paccy_latch_event(paccy,
				BCMPMU_USB_EVENT_SESSION_VALID, NULL);
		schedule_delayed_work(&paccy->det_work, ACCY_WORK_DELAY);
		break;

	case PMU_IRQ_USBINS:
		pr_accy(INIT, "### ISR  PMU_IRQ_USBINS: %x\n",
			PMU_IRQ_USBINS);
		if (paccy->bc == BC_EXT_DETECT) {
			/* In customer platform(CONFIG_USB_SWITCH_TSU6111),
			CHGDET_LATCH and CHGDET_TO are both disabled by BC_ENABLE
			because the BC status will be read from TSU. So we need to
			move the detection code from CHGDET_LATCH to USBINS. Even
			though this problem is found for customer platform, but I
			think this move should be good for other platform too, (no
			longer need the workaround). */
			if (paccy->det_state != USB_CONNECTED) {
				bcmpmu_paccy_latch_event(paccy,
					BCMPMU_USB_EVENT_CHGDET_LATCH,
					NULL);
				paccy->det_state = USB_DETECT;
				usb_handle_state(paccy);
			}
		}
		break;

	case PMU_IRQ_USBRM:
		pr_accy(INIT, "### ISR  PMU_IRQ_USBRM: %x\n",
			PMU_IRQ_USBRM);
		paccy->det_state = USB_DISCONNECTED;
		usb_handle_state(paccy);
		break;

	case PMU_IRQ_CHGDET_TO:
		pr_accy(INIT, "### ISR  PMU_IRQ_CHGDET_TO: %x\n",
			PMU_IRQ_CHGDET_TO);
		if (board_id != EDN01x)
			break;
	case PMU_IRQ_CHGDET_LATCH:
			/* Because of the FFB EDN00 wiring issue (CQ245795),
			the PMU charger detection doesn't work and we will get
			CHGDET_TO interrupt.  To resolve this issue, we
			tempararily treat the CHGDET_TO interrupt as
			CHGDET_LATCH as a workaround
			*/
		pr_accy(INIT, "### ISR  PMU_IRQ_CHGDET_LATCH: %x\n",
			PMU_IRQ_CHGDET_LATCH);
		if (paccy->bc != BC_EXT_DETECT) {
			if ((paccy->det_state != USB_CONNECTED) &&
				(paccy->det_state != USB_RETRY)) {
				bcmpmu_paccy_latch_event(paccy,
					BCMPMU_USB_EVENT_CHGDET_LATCH, NULL);
				paccy->det_state = USB_DETECT;
				usb_handle_state(paccy);
			}
		}
		break;

	case PMU_IRQ_OTG_SESS_VALID_F:
		pr_accy(INIT, "### ISR  PMU_IRQ_OTG_SESS_VALID_F : %x\n",
				PMU_IRQ_OTG_SESS_VALID_F);
		if ((get_supply_type_str(prev_chrgr_type) != NULL) &&
			(strcmp(get_supply_type_str
				(prev_chrgr_type), "bcmpmu_usb") == 0)) {
			bcmpmu_paccy_latch_event(paccy,
					BCMPMU_USB_EVENT_SESSION_INVALID, NULL);
			schedule_delayed_work(&paccy->det_work,
						ACCY_WORK_DELAY);
		}
		break;

	case PMU_IRQ_VBUS_VALID_R:
		pr_accy(INIT, "### ISR  PMU_IRQ_VBUS_VALID_R: %x\n",
			PMU_IRQ_VBUS_VALID_R);
		if (paccy->bc != BC_EXT_DETECT) {
			ret = bcmpmu_usb_get(bcmpmu,
					 BCMPMU_USB_CTRL_GET_ID_VALUE,
					 &id_status);
			bcmpmu_paccy_latch_event(paccy,
				BCMPMU_USB_EVENT_VBUS_VALID, NULL);

			if (id_status != PMU_USB_ID_GROUND) {
				paccy->det_state = USB_IDLE;
				schedule_delayed_work(
					&paccy->det_work, ACCY_WORK_DELAY);
			} else {
				pr_accy(FLOW,
				"%s, USB Host mode skipping BC detect\n",
				__func__);
			}
		} else {
			bcmpmu_paccy_latch_event(paccy,
				BCMPMU_USB_EVENT_VBUS_VALID, NULL);
		}

		break;

	case PMU_IRQ_VBUS_VALID_F:
		pr_accy(INIT, "### ISR  PMU_IRQ_VBUS_VALID_F:%x\n",
				PMU_IRQ_VBUS_VALID_F);
		if ((get_supply_type_str(prev_chrgr_type) != NULL) &&
			(strcmp(get_supply_type_str
				(prev_chrgr_type), "bcmpmu_usb") == 0)) {
			bcmpmu_paccy_latch_event(paccy,
					BCMPMU_USB_EVENT_VBUS_INVALID, NULL);
			schedule_delayed_work(&paccy->det_work,
							ACCY_WORK_DELAY);
		}
		break;

	case PMU_IRQ_IDCHG:
		pr_accy(FLOW, "### ISR  PMU_IRQ_IDCHG: %x\n",
			PMU_IRQ_IDCHG);
		bcmpmu_paccy_latch_event(paccy,
				BCMPMU_USB_EVENT_ID_CHANGE, NULL);
		schedule_delayed_work(&paccy->det_work, ACCY_WORK_DELAY);
		break;

	case PMU_IRQ_ADP_CHANGE:
		pr_accy(FLOW, "### ISR  PMU_IRQ_ADP_CHANGE: %x\n",
			PMU_IRQ_ADP_CHANGE);
		bcmpmu_paccy_latch_event(paccy,
				BCMPMU_USB_EVENT_ADP_CHANGE, NULL);
		schedule_delayed_work(&paccy->det_work, ACCY_WORK_DELAY);
		break;

	case PMU_IRQ_ADP_SNS_END:
		pr_accy(FLOW, "### ISR  PMU_IRQ_ADP_SNS_END: %x\n",
			PMU_IRQ_ADP_SNS_END);
		bcmpmu_paccy_latch_event(paccy,
				BCMPMU_USB_EVENT_ADP_SENSE_END, NULL);
		schedule_delayed_work(&paccy->det_work, ACCY_WORK_DELAY);
		break;

	case PMU_IRQ_VB_SESS_END_F:
		pr_accy(FLOW, "### ISR  PMU_IRQ_VB_SESS_END_F: %x\n",
			PMU_IRQ_VB_SESS_END_F);
		bcmpmu_paccy_latch_event(paccy,
				BCMPMU_USB_EVENT_SESSION_END_VALID, NULL);
		schedule_delayed_work(&paccy->det_work, ACCY_WORK_DELAY);
		break;

	case PMU_IRQ_VB_SESS_END_R:
		pr_accy(FLOW, "### ISR  PMU_IRQ_VB_SESS_END_R: %x\n",
			PMU_IRQ_VB_SESS_END_R);
		bcmpmu_paccy_latch_event(paccy,
				BCMPMU_USB_EVENT_SESSION_END_VALID, NULL);
		schedule_delayed_work(&paccy->det_work, ACCY_WORK_DELAY);
		break;
#if 0 /* remove the TO interrupt for the time being */
	case PMU_IRQ_CHGDET_TO:
		pr_accy(FLOW, "### ISR  PMU_IRQ_CHGDET_TO: %x\n",
			PMU_IRQ_CHGDET_TO);
		break;
#endif
	case PMU_IRQ_RESUME_VBUS:
		pr_accy(FLOW, "### ISR  PMU_IRQ_RESUME_VBUS: %x\n",
			PMU_IRQ_RESUME_VBUS);
		bcmpmu_paccy_latch_event(paccy,
				BCMPMU_CHRGR_EVENT_CHRG_RESUME_VBUS, NULL);
		schedule_delayed_work(&paccy->det_work, ACCY_WORK_DELAY);
		break;
	case PMU_IRQ_USBOV:
		pr_accy(FLOW, "### ISR PMU_IRQ_USBOV: %x\n",
			PMU_IRQ_USBOV);
		bcmpmu_chrgr_usb_en(bcmpmu, 0);
		break;
	case PMU_IRQ_USBOV_DIS:
	case PMU_IRQ_CHGERRDIS:
		pr_accy(FLOW, "### ISR %x\n", irq);
		if (paccy->det_state == USB_CONNECTED) {
			if (!bcmpmu_is_usb_host_enabled(bcmpmu))
				bcmpmu_chrgr_usb_en(bcmpmu, 1);
		}
		break;
	default:
		break;
	}
}

#ifdef CONFIG_DEBUG_FS

static void bcmpmu_accy_debug_init(struct bcmpmu_accy *accy)
{
	struct dentry *dentry;
	struct bcmpmu59xxx *bcmpmu = accy->bcmpmu;

	if (!bcmpmu->dent_bcmpmu) {
		pr_err("Failed to initialize debugfs\n");
		return;
	}
	dentry = accy->dent_accy;
	dentry = debugfs_create_dir("accy", bcmpmu->dent_bcmpmu);
	if (!dentry)
		return;

	if (!debugfs_create_u32("dbg_mask", S_IWUSR | S_IRUSR,
				dentry, &debug_mask))
		goto err ;

	if (!debugfs_create_u32("chrgr_type", S_IRUSR,
				dentry, &accy->usb_accy_data.chrgr_type))
		goto err ;

	return;
err:
	debugfs_remove(dentry);
}

#endif

static void bcmpmu_notify_charger_state(struct bcmpmu_accy *paccy)
{
	enum bcmpmu_usb_type_t usb_type = paccy->usb_accy_data.usb_type;
	enum bcmpmu_chrgr_type_t chrgr_type;
	struct bcmpmu59xxx *bcmpmu = paccy->bcmpmu;
	int board_id = bcmpmu->pdata->board_id;
	chrgr_type = bcmpmu_get_chrgr_type(paccy);
	pr_accy(FLOW, "===chrgr_type %d, usb_type %d\n", chrgr_type, usb_type);

	paccy->usb_accy_data.max_curr_chrgr = chrgr_curr_lmt[chrgr_type];
	bcmpmu_paccy_latch_event(paccy, BCMPMU_CHRGR_EVENT_CHGR_DETECTION,
			&paccy->usb_accy_data.chrgr_type);

	if ((usb_type < PMU_USB_TYPE_MAX) &&
		(usb_type > PMU_USB_TYPE_NONE)) {
		bcmpmu_paccy_latch_event(paccy,
				BCMPMU_USB_EVENT_USB_DETECTION,
				&paccy->usb_accy_data.usb_type);
		if (is_charging_state() || (board_id == EDN01x))
			bcmpmu_paccy_latch_event(paccy,
					BCMPMU_CHRGR_EVENT_CHRG_CURR_LMT,
					&chrgr_curr_lmt[chrgr_type]);
	} else
		bcmpmu_paccy_latch_event(paccy,
				BCMPMU_CHRGR_EVENT_CHRG_CURR_LMT,
				&chrgr_curr_lmt[chrgr_type]);
}

void send_usb_insert_event(enum bcmpmu_event_t event, void *para)
{
	struct bcmpmu_accy *paccy = bcmpmu_accy;
	int usb_type = *(int *)para;
	if (!paccy)
		return;
	paccy->usb_accy_data.usb_type = usb_type;
	pr_accy(INIT, "====EXT USB Event send %x usb %d =====\n",
			event, usb_type);
}
/* cb for extern charger detection */
void send_chrgr_insert_event(enum bcmpmu_event_t event, void *para)
{
	struct bcmpmu_accy *paccy = bcmpmu_accy;
	struct bcmpmu59xxx *bcmpmu = paccy->bcmpmu;
	int chrgr_type = *(int *)para;
	u32 vbus_status = 0;
	int valid = 0;

	if (!paccy)
		return;

	pr_accy(INIT, "==== EXT Charger Insert Event ====para %d\n",
		*(int *)para);
	bcmpmu_usb_get(bcmpmu,
			BCMPMU_USB_CTRL_GET_SESSION_STATUS,
			(void *)&vbus_status);

	if (event == BCMPMU_CHRGR_EVENT_CHGR_DETECTION) {

		if ((chrgr_type == PMU_CHRGR_TYPE_SDP) && vbus_status) {
			if (paccy->det_state != USB_CONNECTED) {
				paccy->usb_accy_data.
					chrgr_type = PMU_CHRGR_TYPE_SDP;
#if 0 /*Remove to fix enumeration failure when boot with USB */
				paccy->usb_accy_data.
					usb_type = PMU_USB_TYPE_SDP;
				paccy->det_state = USB_CONNECTED;
				send_chrgr_event(paccy->bcmpmu,
						BCMPMU_USB_EVENT_USB_DETECTION,
						&paccy->usb_accy_data.usb_type);
				valid = 1;
#endif
			}

		} else if ((chrgr_type == PMU_CHRGR_TYPE_DCP) && vbus_status) {
			if (paccy->det_state != USB_CONNECTED) {
				paccy->usb_accy_data.
					chrgr_type = PMU_CHRGR_TYPE_DCP;
				paccy->usb_accy_data.
					usb_type = PMU_USB_TYPE_NONE;
				paccy->det_state = USB_CONNECTED;
				valid = 1;
			}

		} else if (chrgr_type == PMU_CHRGR_TYPE_NONE) {
			if (paccy->det_state == USB_CONNECTED) {
				paccy->usb_accy_data.
						chrgr_type = PMU_CHRGR_TYPE_NONE;
				paccy->usb_accy_data.
						usb_type = PMU_USB_TYPE_NONE;
				paccy->det_state = USB_DISCONNECTED;
				valid = 1;
			}
		}

		pr_accy(FLOW, "%s, chrgr change, chrgr_type=0x%X\n",
				__func__, paccy->usb_accy_data.chrgr_type);
		if (valid) {
			bcmpmu_paccy_latch_event(paccy, event,
				&paccy->usb_accy_data.chrgr_type);
			schedule_delayed_work(&paccy->det_work,
						ACCY_WORK_DELAY);
		}
	}
}
EXPORT_SYMBOL_GPL(send_chrgr_insert_event);

static void bcmpmu_accy_set_pmu_BC12(struct bcmpmu59xxx *bcmpmu, int val)
{
	u8 reg;
	bcmpmu->read_dev(bcmpmu, PMU_REG_MBCCTRL5, &reg);
	pr_accy(FLOW, "###<%s> MBCCTRL5 %x\n", __func__, reg);
	if (val)
		reg |= MBCCTRL5_BC12_EN_MASK;
	else
		reg &= ~MBCCTRL5_BC12_EN_MASK;
	bcmpmu->write_dev(bcmpmu, PMU_REG_MBCCTRL5, reg);
	pr_accy(FLOW, "###<%s> MBCCTRL5 %x\n", __func__, reg);
}

static void bcmpmu_accy_check_BC12_EN(struct bcmpmu59xxx *bcmpmu)
{
	u8 reg;
	bcmpmu->read_dev(bcmpmu, PMU_REG_MBCCTRL5, &reg);
	pr_accy(FLOW, "###<%s> MBCCTRL5 %x\n", __func__, reg);
	if (reg & MBCCTRL5_BC12_EN_MASK)
		pr_accy(FLOW, "<%s> BC12 is enabled\n", __func__);
	else
		pr_accy(FLOW, "<%s> BC12 is disabled\n", __func__);
}

static void bcdldo_cycle_power(struct bcmpmu_accy *paccy)
{
	pr_accy(FLOW, "<%s> MBCCTRL5 %x\n", __func__);

	paccy_set_ldo_bit(paccy, 0);
	/* Scope trace shows the decay time of the BCDLDO line
	 * is about 4ms. Hence a 5ms delay here to ensure proper
	 * power cycle.
	 */
	usleep_range(6000, 8000);
	paccy_set_ldo_bit(paccy, 1);
}
/*
static void bcmpmu_update_pmu_chrgr_type(struct bcmpmu_accy *paccy)
{
	struct bcmpmu59xxx *bcmpmu = paccy->bcmpmu;
	enum bcmpmu_chrgr_type_t type = paccy->usb_accy_data.chrgr_type;
	u8 chrgr = 0;
	if (paccy->bc == BCMPMU_BC_BB_BC12) {
		if (type)
			chrgr = type <<  MBCCTRL5_CHP_TYP_SHIFT;
		pr_accy(FLOW, "###<%s> MBCCTRL5 charger type bc %x pmu %x\n",
			__func__, type, chrgr);
		bcmpmu->write_dev(bcmpmu,
				PMU_REG_MBCCTRL5, chrgr);
	}
}
*/
/* workq func */
static void usb_deferred_work(struct work_struct *work)
{
	struct list_head *pos, *temp;
	u32 event, id_status = 0;
	void *para;
	unsigned long flags;
	struct event_list *entry ;
	struct bcmpmu_accy *paccy =
		container_of(work, struct bcmpmu_accy, det_work.work);
	struct bcmpmu59xxx *bcmpmu = paccy->bcmpmu;
	bool id_check = true;
	u32 vbus_status;

	if (paccy->bc != BC_EXT_DETECT) {
		bcmpmu_usb_get(bcmpmu,
				BCMPMU_USB_CTRL_GET_ID_VALUE,
				(void *)&id_status);
		id_check = bcmpmu_supported_chrgr_usbid
				(paccy, id_status);
		pr_accy(FLOW, "%s, id %x, vbus %d\n",
			__func__, id_status, vbus_status);
		bcmpmu_usb_get(bcmpmu,
			BCMPMU_USB_CTRL_GET_SESSION_STATUS,
			(void *)&vbus_status);
	}
	pr_accy(FLOW, "###<%s> , DEV_STATE %x\n",
			__func__, paccy->det_state);

	if (paccy->bc != BC_EXT_DETECT && id_check && vbus_status &&
		(bc_start != PMU_BC_DETECTION_END)) {
		schedule_delayed_work(&paccy->det_work,
					ACCY_RETRY_DELAY);
		}

	list_for_each_safe(pos, temp, (&paccy->dispatch_list->node)) {
		if (!list_empty(&paccy->dispatch_list->node)) {
			entry = list_first_entry(&paccy->dispatch_list->node,
				struct event_list, node);
			if (!entry)
				break;
			/* Add to work List */
			pr_accy(FLOW, "###<%s> , Event %x\n",
					__func__, entry->event);
			spin_lock_irqsave(&paccy->accy_lock, flags);
			event = entry->event;
			para = entry->para;
			list_del(&entry->node);
			list_add_tail(&entry->node, &paccy->free_list->node);
			spin_unlock_irqrestore(&paccy->accy_lock, flags);
			bcmpmu_call_notifier(bcmpmu, event, para);
		}
	}
	switch (paccy->det_state) {
	case USB_IDLE:
	/*
		bcdldo_cycle_power(paccy);
		reset_bc(paccy);
		if (paccy->clock_en)
			enable_bc_clock(paccy, 0);
			*/
		break;
	case USB_DETECT:
		break;
	case USB_RETRY:
		usb_detect_state(paccy);
		break;
	default:
		break;
	}
}

static void usb_adp_work(struct work_struct *work)
{
	struct bcmpmu_accy *data =
		container_of(work, struct bcmpmu_accy, adp_work);
	if (data->adp_cal_done == 0) {
		data->adp_cal_done = 1;
		bcmpmu_call_notifier(data->bcmpmu,
				BCMPMU_USB_EVENT_ADP_CALIBRATION_DONE, NULL);
	}
}

static void usb_delayed_init_work(struct work_struct *work)
{
	struct bcmpmu_accy *paccy =
	       container_of(work, struct bcmpmu_accy, init_work.work);

	usb_handle_state(paccy);
}

int bcmpmu_usb_otg_bost_en(struct bcmpmu59xxx *bcmpmu, bool en)
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
int bcmpmu_usb_set(struct bcmpmu59xxx *bcmpmu,
		int ctrl, unsigned long data)
{
	struct bcmpmu_accy *paccy = (struct bcmpmu_accy *)bcmpmu->accyinfo;
	int ret = 0;
	u8 temp;
	u32 val;
	pr_accy(FLOW, "%s, ctrl=%d, val=0x%lX\n", __func__, ctrl, data);

	switch (ctrl) {
	case BCMPMU_USB_CTRL_CHRG_CURR_LMT:
		if (!is_charging_state()) {
			paccy->usb_accy_data.max_curr_chrgr = (int)data;
			bcmpmu_call_notifier(paccy->bcmpmu,
				BCMPMU_CHRGR_EVENT_CHRG_CURR_LMT,
				&paccy->usb_accy_data.max_curr_chrgr);
		}
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
			if (paccy->otg_block_enabled == 0) {
				bcmpmu->read_dev(bcmpmu, PMU_REG_OTGCTRL1,
						&temp);
				temp |= (1 << OTGCTRL1_OTG_SHUTDOWNB_SHIFT);
				ret =
					bcmpmu->write_dev(bcmpmu,
							PMU_REG_OTGCTRL1,
							temp);
				paccy->otg_block_enabled = 1;
			}
			if (paccy->adp_block_enabled == 0) {
				bcmpmu->read_dev(bcmpmu,
						PMU_REG_OTGCTRL12,
						&temp);
				temp |= (1 << OTGCTRL12_ADP_THR_CTRL_SHIFT);
				ret =
					bcmpmu->write_dev(bcmpmu,
							PMU_REG_OTGCTRL12,
							temp);
				paccy->adp_block_enabled = 1;
			}
			if (paccy->adp_sns_comp == 0) {
				bcmpmu->read_dev(bcmpmu,
						PMU_REG_OTGCTRL12,
						&temp);
				temp |= (1 << OTGCTRL12_EN_ADP_SNS_COMP_SHIFT);
				ret =
					bcmpmu->write_dev(bcmpmu,
							PMU_REG_OTGCTRL12,
							temp);
				paccy->adp_sns_comp = 1;
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
			if (paccy->otg_block_enabled == 0) {
				ret = bcmpmu->read_dev(bcmpmu,
						PMU_REG_OTGCTRL11, &temp);
				temp |= OTGCTRL1_OTG_SHUTDOWNB_MASK;
				ret = bcmpmu->write_dev(bcmpmu,
						PMU_REG_OTGCTRL11, temp);
				paccy->otg_block_enabled = 1;
			}
			if (paccy->adp_block_enabled == 0) {

				ret = bcmpmu->read_dev(bcmpmu,
						PMU_REG_OTGCTRL12,
						&temp);
				temp |= OTGCTRL12_ADP_SHUTDOWNB_MASK;
				ret |= bcmpmu->write_dev(bcmpmu,
						PMU_REG_OTGCTRL12,
						temp);
				paccy->adp_block_enabled = 1;
			}
			if (paccy->adp_prob_comp == 0) {
				ret = bcmpmu->read_dev(bcmpmu,
						PMU_REG_OTGCTRL12, &temp);

				temp |= OTGCTRL12_EN_ADP_PRB_COMPS_MASK;
				ret = bcmpmu->write_dev(bcmpmu,
						PMU_REG_OTGCTRL12, temp);

				paccy->adp_prob_comp = 1;
			}
			ret = bcmpmu->read_dev(bcmpmu,
				PMU_REG_OTGCTRL11, &temp);
			temp |= OTGCTRL11_ADP_PRB_MASK;
			ret |= bcmpmu->write_dev(bcmpmu,
				PMU_REG_OTGCTRL11, temp);

		}
		break;

	case BCMPMU_USB_CTRL_START_ADP_CAL_PRB:
		if (paccy->otg_block_enabled == 0) {
			ret = bcmpmu->read_dev(bcmpmu,
					PMU_REG_OTGCTRL1, &temp);
			temp |= OTGCTRL1_OTG_SHUTDOWNB_MASK;
			ret |= bcmpmu->write_dev(bcmpmu,
					PMU_REG_OTGCTRL1, temp);
			paccy->otg_block_enabled = 1;
		}
		if (paccy->adp_block_enabled == 0) {
			ret = bcmpmu->read_dev(bcmpmu,
					PMU_REG_OTGCTRL12, &temp);
			temp |= OTGCTRL12_ADP_SHUTDOWNB_MASK;
			ret |= bcmpmu->write_dev(bcmpmu,
					PMU_REG_OTGCTRL12, temp);
			paccy->adp_block_enabled = 1;
		}
		if (paccy->adp_prob_comp == 0) {
			ret = bcmpmu->read_dev(bcmpmu,
					PMU_REG_OTGCTRL12, &temp);
			temp |=  OTGCTRL12_EN_ADP_PRB_COMPS_MASK ;
			ret = bcmpmu->write_dev(bcmpmu,
					PMU_REG_OTGCTRL12, temp);
			paccy->adp_prob_comp = 1;
		}
		ret = bcmpmu->read_dev(bcmpmu,
				PMU_REG_OTGCTRL11, &temp);

		temp |= (PMU_USB_ADP_MODE_CALIBRATE <<
				OTGCTRL11_ADP_PRB_MODE_MASK);
		temp |= OTGCTRL11_ADP_PRB_MASK;

		ret |= bcmpmu->write_dev(bcmpmu,
				PMU_REG_OTGCTRL11, temp);
		/* start timer */
		schedule_work(&paccy->adp_work);
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
		if (data == 1) {
			atomic_set(&paccy->usb_allow_bc_detect, 1);
			if (paccy->det_state != USB_CONNECTED) {
				pr_accy(FLOW, "== usb_allow_bc_detect\n");
				paccy->retry_cnt = 0;
				paccy->det_state = USB_DETECT;
				schedule_delayed_work(&paccy->init_work,
						ACCY_RETRY_DELAY);
			}
		} else
			atomic_set(&paccy->usb_allow_bc_detect, 0);
		break;
	case BCMPMU_USB_CTRL_BCDLDO:
		if (data == 1)
			paccy_set_ldo_bit(paccy, 1);
		else
			paccy_set_ldo_bit(paccy, 0);

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
	struct bcmpmu_accy *paccy = (struct bcmpmu_accy *)bcmpmu->accyinfo;
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
/*
	case BCMPMU_USB_CTRL_GET_ADP_SENSE_TIMER_VALUE:
		ret = bcmpmu->read_dev(bcmpmu,
				PMU_REG_ADP_SNS_TM,
				&val);
		val &= ADP_SNS_TM_MASK;
		val = val >> ADP_SNS_TM_SHIFT;
		break;
*/
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
			val, ENV2_P_UBPD_CHG_SHIFT);
		val &= ENV2_P_UBPD_CHG;
		val =  val >> ENV2_P_UBPD_CHG_SHIFT;
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
	case BCMPMU_USB_CTRL_GET_SESSION_END_STATUS:
		ret = bcmpmu->read_dev(bcmpmu,
				PMU_REG_ENV7,
				&val);
		val &= OTG_VBUS_STAT_MASK;
		val = val >> OTG_VBUS_STAT_SHIFT;
		break;
	case BCMPMU_USB_CTRL_GET_ID_VALUE:
		if (paccy->bc != BC_EXT_DETECT) {
			ret = bcmpmu->read_dev(bcmpmu,
				PMU_REG_ENV4,
				&val);
			val &= ENV4_ID_CODE_MASK;
			val = val >> ENV4_ID_CODE_SHIFT;
			if ((paccy->usb_id_map) &&
					(val < paccy->usb_id_map_len))
				val = paccy->usb_id_map[val];
			pr_accy(FLOW, "ENV4 %x val, id mak %x\n",
				val, ENV4_ID_CODE_SHIFT);
		} else
			val = PMU_USB_ID_FLOAT;
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
		val = bcmpmu_get_chrgr_type(paccy);
		break;

	case BCMPMU_USB_CTRL_GET_USB_TYPE:
		ret = 0;
		val = paccy->usb_accy_data.usb_type;
		break;
	case BCMPMU_USB_CTRL_BCDLDO:
		val = paccy_get_ldo_bit(paccy);
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

static int bcmpmu_accy_eventq_init(struct bcmpmu_accy *accy)
{
	struct event_list *ev_list;
	int i;

	accy->free_list = kmalloc(sizeof(struct event_list), GFP_ATOMIC);
	if (!accy->free_list)
		return -ENOMEM;

	accy->dispatch_list = kmalloc(sizeof(struct event_list), GFP_ATOMIC);
	if (!accy->dispatch_list)
		goto err;
	INIT_LIST_HEAD(&accy->free_list->node);
	INIT_LIST_HEAD(&accy->dispatch_list->node);

	ev_list = kmalloc(sizeof(struct event_list) *
		MAX_EVENT_QUEUE, GFP_ATOMIC);
	if (!ev_list)
		goto err;
	for (i = 0; i < MAX_EVENT_QUEUE; i++)
		list_add_tail(&ev_list[i].node, &accy->free_list->node);

	return 0;
err:
	kfree(accy->free_list);
	return -ENOMEM;

}

static void bcmpmu_accy_dinit_eventq(struct bcmpmu_accy *accy)
{
	struct list_head *pos, *temp;
	struct event_list *entry ;

	list_for_each_safe(pos, temp, (&accy->dispatch_list->node)) {
		if (!list_empty(&accy->dispatch_list->node)) {
			entry = list_first_entry(&accy->dispatch_list->node,
					struct event_list, node);
			if (!entry)
				break;
			list_del(&entry->node);
			kfree(entry);
		}
	}
	list_for_each_safe(pos, temp, (&accy->free_list->node)) {
		if (!list_empty(&accy->free_list->node)) {
			entry = list_first_entry(&accy->free_list->node,
					struct event_list, node);
			if (!entry)
				break;
			list_del(&entry->node);
			kfree(entry);
		}
	}

	kfree(accy->free_list);

}

static int bcmpmu_accy_probe(struct platform_device *pdev)
{
	int ret;
	struct bcmpmu59xxx *bcmpmu =  dev_get_drvdata(pdev->dev.parent);
	struct bcmpmu_accy *paccy;
	struct bcmpmu59xxx_platform_data *pdata ;
	pdata = bcmpmu->pdata;

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
	paccy->usb_get = bcmpmu_usb_get;
	paccy->usb_set = bcmpmu_usb_set;
	paccy->usb_accy_data.usb_type = PMU_USB_TYPE_NONE;
	paccy->usb_accy_data.chrgr_type = PMU_CHRGR_TYPE_NONE;
	paccy->usb_accy_data.max_curr_chrgr = 0;
	paccy->adp_cal_done = 0;
	paccy->clock_en = 0;
	paccy->usb_id_map = bcmpmu_get_usb_id_map
				(bcmpmu, &paccy->usb_id_map_len);
	paccy->otg_block_enabled = 0;
	paccy->adp_block_enabled = 0;
	paccy->adp_prob_comp = 0;
	paccy->adp_sns_comp = 0;
	paccy->bc = pdata->bc;
	paccy->piggyback_chrg = pdata->piggyback_chrg;
	spin_lock_init(&paccy->accy_lock);

	INIT_WORK(&paccy->adp_work, usb_adp_work);
	INIT_DELAYED_WORK(&paccy->det_work, usb_deferred_work);
	INIT_DELAYED_WORK(&paccy->init_work, usb_delayed_init_work);

	ret = bcmpmu_accy_eventq_init(paccy);
	if (ret)
		goto err;

#ifdef CONFIG_HAS_WAKELOCK
	wake_lock_init(&paccy->wake_lock, WAKE_LOCK_SUSPEND, "usb_accy");
	wake_lock_init(&paccy->usb_plug_out_wake_lock,
				WAKE_LOCK_SUSPEND, "usb_accy_plug_out");
#endif
#ifdef CONFIG_KONA_PI_MGR
	pi_mgr_qos_add_request(&paccy->qos, "usb_accy",
				PI_MGR_PI_ID_ARM_CORE,
				PI_MGR_QOS_DEFAULT_VALUE);
#endif
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_USBINS,
				bcmpmu_accy_isr, paccy);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_USBRM,
				bcmpmu_accy_isr, paccy);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_CHGDET_LATCH,
				bcmpmu_accy_isr, paccy);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_CHGDET_TO,
				bcmpmu_accy_isr, paccy);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_VA_SESS_VALID_R,
				bcmpmu_accy_isr, paccy);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_VBUS_VALID_R,
				bcmpmu_accy_isr, paccy);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_OTG_SESS_VALID_F,
				bcmpmu_accy_isr, paccy);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_VBUS_VALID_F,
				bcmpmu_accy_isr, paccy);
#ifndef CONFIG_MACH_HAWAII_SS_COMMON
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_IDCHG,
				bcmpmu_accy_isr, paccy);
#endif
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_RESUME_VBUS,
				bcmpmu_accy_isr, paccy);
#ifndef CONFIG_SEC_CHARGING_FEATURE
	/* register_irq for PMU_IRQ_USBOV/PMU_IRQ_USBOV_DIS is called by
	bcmpmu59xxx-spa-pb.c, so disable USBOV ISR in bcmpmu59xxx-accy.c */
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_USBOV,
				bcmpmu_accy_isr, paccy);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_USBOV_DIS,
				bcmpmu_accy_isr, paccy);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_CHGERRDIS,
				bcmpmu_accy_isr, paccy);
#endif
	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_USBINS);
	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_USBRM);
	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_CHGDET_LATCH);
	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_CHGDET_TO);
	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_VA_SESS_VALID_R);
	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_VBUS_VALID_R);
	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_OTG_SESS_VALID_F);
	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_VBUS_VALID_F);
#ifndef CONFIG_MACH_HAWAII_SS_COMMON
	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_IDCHG);
#endif
	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_RESUME_VBUS);
#ifndef CONFIG_SEC_CHARGING_FEATURE
	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_USBOV);
	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_USBOV_DIS);
	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_CHGERRDIS);
#endif

	/*
	 * For  BB's BC detection we need to select BCDAVDD33 pin
	 */
	if (paccy->bc == BCMPMU_BC_BB_BC12) {
		bcm_hsotgctrl_set_bc_iso(false);
		paccy_set_ldo_bit(paccy, 1);
		bcmpmu_accy_set_pmu_BC12(paccy->bcmpmu, 1);
		reset_bc(paccy);
	} else
		bcm_hsotgctrl_set_bc_iso(true);

	/* Disable BC12 for cust platform */
	if (paccy->bc == BC_EXT_DETECT) {
		bcmpmu_accy_check_BC12_EN(paccy->bcmpmu);
		bcmpmu_accy_set_pmu_BC12(paccy->bcmpmu, 0);
		paccy_set_ldo_bit(paccy, 0);
	}

#ifdef CONFIG_DEBUG_FS
	bcmpmu_accy_debug_init(paccy);
#endif
	paccy->det_state = USB_DETECT;

	if (paccy->bc == BC_EXT_DETECT) {
		schedule_delayed_work(&paccy->init_work,
			msecs_to_jiffies(15000));
	} else {
		schedule_delayed_work(&paccy->init_work, ACCY_RETRY_DELAY);
	}

	return 0;

err:
	kfree(paccy);
	return ret;
}

static int bcmpmu_accy_remove(struct platform_device *pdev)
{
	struct bcmpmu59xxx *bcmpmu = pdev->dev.platform_data;
	struct bcmpmu_accy *paccy = bcmpmu->accyinfo;

	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_USBINS);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_USBRM);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_CHGDET_LATCH);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_CHGDET_TO);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_VA_SESS_VALID_R);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_VBUS_VALID_R);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_OTG_SESS_VALID_F);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_VBUS_VALID_F);
#ifndef CONFIG_MACH_HAWAII_SS_COMMON
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_IDCHG);
#endif
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_RESUME_VBUS);
#ifndef CONFIG_SEC_CHARGING_FEATURE
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_USBOV);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_USBOV_DIS);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_CHGERRDIS);
#endif
	cancel_delayed_work_sync(&paccy->det_work);
	cancel_work_sync(&paccy->adp_work);
#ifdef CONFIG_HAS_WAKELOCK
	wake_lock_destroy(&paccy->wake_lock);
	wake_lock_destroy(&paccy->usb_plug_out_wake_lock);
#endif
#ifdef CONFIG_KONA_PI_MGR
	pi_mgr_qos_request_remove(&paccy->qos);
#endif
	bcmpmu_accy_dinit_eventq(paccy);
#ifdef CONFIG_DEBUG_FS
	debugfs_remove(paccy->dent_accy);
#endif
	kfree(paccy);
	return 0;
}

static struct platform_driver bcmpmu_accy_driver = {
	.driver = {
		.name = "bcmpmu_accy",
	},
	.probe = bcmpmu_accy_probe,
	.remove = bcmpmu_accy_remove,
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
