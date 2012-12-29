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
#include <plat/kona_reset_reason.h>
#ifdef CONFIG_DEBUG_FS
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#endif
#define MAX_EVENT_QUEUE 30
#define BC_MAX_RETRIES		3

#define ACCY_WORK_DELAY  msecs_to_jiffies(5)
static int debug_mask = BCMPMU_PRINT_ERROR | BCMPMU_PRINT_INIT;
static int prev_chrgr_type;

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

#define PMU_BC_SDP		1
#define PMU_BC_CDP		2
#define PMU_BC_DCP		3
#define PMU_CHP_TYPE_MASK	0x3


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
	[PMU_CHRGR_TYPE_TYPE1] = 1000,
	[PMU_CHRGR_TYPE_TYPE2] = 1000,
	[PMU_CHRGR_TYPE_PS2] = 0,
	[PMU_CHRGR_TYPE_ACA] = 1000,
};

static struct bcmpmu_accy *bcmpmu_accy;
static void bcmpmu_accy_isr(enum bcmpmu59xxx_irq irq, void *data);
static void usb_handle_state(struct bcmpmu_accy *paccy);
static void bcmpmu_notify_charger_state(struct bcmpmu_accy *paccy);
static void bcmpmu_accy_chrgr_detect_state(struct bcmpmu59xxx *bcmpmu, int val);
static void bcmpmu_paccy_latch_event(struct bcmpmu_accy *accy,
		u32 event, void *para);

static unsigned int get_bc_status(struct bcmpmu_accy *paccy)
{
	u32 status = 0;
	struct bcmpmu59xxx *bcmpmu = paccy->bcmpmu;

	if (paccy->bc == BCMPMU_BC_PMU_BC12) {
		bcmpmu->read_dev(bcmpmu,
				PMU_REG_MBCCTRL5, (u8 *)&status);
		pr_accy(FLOW, " MBCTRL5 %x\n", status);
		status &= MBCCTRL5_CHP_TYP_MASK;
		pr_accy(FLOW, "MBCTRL5 %x  mask %x\n",
			status, MBCCTRL5_CHP_TYP_MASK);
		status = status >> MBCCTRL5_CHP_TYP_SHIFT;
		pr_accy(FLOW, " MBCTRL5 %x shift % d\n",
			status, MBCCTRL5_CHP_TYP_SHIFT);
		return status;
	} else
		return 0;
}

static enum bcmpmu_chrgr_type_t get_charger_type(struct bcmpmu_accy *paccy,
		unsigned int bc_status)
{
	enum bcmpmu_chrgr_type_t type;
	u8 chp_typ =  (bc_status & PMU_CHP_TYPE_MASK);
	if (chp_typ) {
		if (chp_typ == PMU_BC_DCP)
			type = PMU_CHRGR_TYPE_DCP;
		else if (chp_typ == PMU_BC_SDP)
			type = PMU_CHRGR_TYPE_SDP;
		else if (chp_typ == PMU_BC_CDP)
			type = PMU_CHRGR_TYPE_CDP;
		else
			type = PMU_CHRGR_TYPE_NONE;
	} else
		type = PMU_CHRGR_TYPE_NONE;
	return type;
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

void paccy_set_ldo_bit(struct bcmpmu_accy *paccy, int val)
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
}

static void usb_detect_state(struct bcmpmu_accy *paccy)
{
	enum bcmpmu_chrgr_type_t chrgr_type = PMU_USB_TYPE_NONE;
	enum bcmpmu_usb_type_t usb_type = PMU_CHRGR_TYPE_NONE;
	u32 vbus_status = 0, id_status = 0 , bc_status;
	struct bcmpmu59xxx *bcmpmu = paccy->bcmpmu;
	bcmpmu_usb_get(bcmpmu,
			BCMPMU_USB_CTRL_GET_SESSION_STATUS,
			(void *)&vbus_status);
	bcmpmu_usb_get(bcmpmu,
			BCMPMU_USB_CTRL_GET_ID_VALUE,
			(void *)&id_status);
	pr_accy(FLOW, "%s, vbus status= %d, id status =%d\n",
		__func__, vbus_status, id_status);

	if ((vbus_status != 0) && (id_status != PMU_USB_ID_GROUND)) {
		bc_status = get_bc_status(paccy);
		pr_accy(FLOW, "%s, bc_status=0x%X, retry=%d\n",
				__func__, bc_status, paccy->retry_cnt);
		if (bc_status) {
			chrgr_type = get_charger_type(paccy,
					bc_status);
			prev_chrgr_type = chrgr_type;
			log_charger_type(chrgr_type);
			if (chrgr_type != PMU_CHRGR_TYPE_NONE) {
				paccy->det_state = USB_CONNECTED;
#ifdef CONFIG_HAS_WAKELOCK
				if (!wake_lock_active(&paccy->wake_lock))
					wake_lock(&paccy->wake_lock);
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
			} else {
				paccy->det_state = USB_RETRY;
			}
		} else {
				paccy->det_state = USB_RETRY;
		}
		paccy->usb_accy_data.usb_type = usb_type;
		paccy->usb_accy_data.chrgr_type = chrgr_type;
	} else {
		paccy->usb_accy_data.usb_type = PMU_USB_TYPE_NONE;
		paccy->usb_accy_data.chrgr_type = PMU_CHRGR_TYPE_NONE;
		paccy->det_state = USB_IDLE;
	}

	pr_accy(FLOW, "<%s>det_state %d chrgr_type %d\n",
		__func__, paccy->det_state, paccy->usb_accy_data.chrgr_type);
	usb_handle_state(paccy);

}

static void usb_handle_state(struct bcmpmu_accy *paccy)
{
	switch (paccy->det_state) {
	case USB_DETECT:
		usb_detect_state(paccy);
		break;

	case USB_RETRY:
		paccy->retry_cnt++;
		if (paccy->retry_cnt < 3) {
			paccy->det_state = USB_DETECT;
			/*
			bc_det_sts_clear(paccy);
			bc_det_restart(paccy);
			*/
			schedule_delayed_work(&paccy->det_work,
						ACCY_WORK_DELAY);
		} else {
			pr_accy(ERROR, "%s, failed, retry times=%d\n",
					__func__, paccy->retry_cnt);
			paccy->usb_accy_data.usb_type = PMU_USB_TYPE_NONE;
			paccy->usb_accy_data.chrgr_type = PMU_CHRGR_TYPE_NONE;
			paccy->det_state = USB_IDLE;
			paccy->retry_cnt = 0;
		}
		break;

	case USB_CONNECTED:
		pr_accy(FLOW, "*** Charger Connected event\n");
		bcmpmu_notify_charger_state(paccy);
		bcmpmu_accy_chrgr_detect_state(paccy->bcmpmu, 0);
		schedule_delayed_work(&paccy->det_work, ACCY_WORK_DELAY);
		break;

	case USB_DISCONNECTED:
		pr_accy(FLOW, "*** Charger disconnected event\n");
		paccy->det_state = USB_IDLE;
		bcmpmu_accy_chrgr_detect_state(paccy->bcmpmu, 1);
		paccy->usb_accy_data.usb_type = PMU_USB_TYPE_NONE;
		paccy->usb_accy_data.chrgr_type = PMU_CHRGR_TYPE_NONE;
		bcmpmu_notify_charger_state(paccy);
#ifdef CONFIG_HAS_WAKELOCK
		if (wake_lock_active(&paccy->wake_lock))
			wake_unlock(&paccy->wake_lock);
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

	pr_accy(FLOW, "#### %s interrupt = %d\n", __func__, irq);

	switch (irq) {
	case PMU_IRQ_VA_SESS_VALID_R:
		pr_accy(INIT, "### ISR  PMU_IRQ_VA_SESS_VALID_R: %x\n",
			PMU_IRQ_VA_SESS_VALID_R);

		bcmpmu_paccy_latch_event(paccy,
				BCMPMU_USB_EVENT_SESSION_VALID, NULL);
		schedule_delayed_work(&paccy->det_work, ACCY_WORK_DELAY);
		break;

	case PMU_IRQ_USBINS:
		pr_accy(INIT, "### ISR  PMU_IRQ_USBINS: %x\n",
			PMU_IRQ_USBINS);
			/*
		bcmpmu_paccy_latch_event(paccy,
				BCMPMU_USB_EVENT_IN, NULL);
		schedule_delayed_work(&paccy->det_work, ACCY_WORK_DELAY);
		*/
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
		if (paccy->det_state != USB_CONNECTED) {
			bcmpmu_paccy_latch_event(paccy,
				BCMPMU_USB_EVENT_CHGDET_LATCH, NULL);
			paccy->det_state = USB_DETECT;
			usb_handle_state(paccy);
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
		bcmpmu_paccy_latch_event(paccy,
				BCMPMU_USB_EVENT_VBUS_VALID, NULL);
		schedule_delayed_work(&paccy->det_work, ACCY_WORK_DELAY);
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
		pr_accy(INIT, "### ISR  PMU_IRQ_VB_SESS_END_R: %x\n",
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
		pr_accy(INIT, "### ISR  PMU_IRQ_RESUME_VBUS: %x\n",
			PMU_IRQ_RESUME_VBUS);
		bcmpmu_paccy_latch_event(paccy,
				BCMPMU_CHRGR_EVENT_CHRG_RESUME_VBUS, NULL);
		schedule_delayed_work(&paccy->det_work, ACCY_WORK_DELAY);
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
	enum bcmpmu_chrgr_type_t chrgr_type = paccy->usb_accy_data.chrgr_type;
	struct bcmpmu59xxx *bcmpmu = paccy->bcmpmu;
	int board_id = bcmpmu->pdata->board_id;
	pr_accy(FLOW, "===chrgr_type %d, usb_type %d\n", chrgr_type, usb_type);

	if (is_charging_state()) {
		bcmpmu_paccy_latch_event(paccy,
				BCMPMU_CHRGR_EVENT_CHRG_CURR_LMT,
				&chrgr_curr_lmt[chrgr_type]);
		return;
	}

	if ((usb_type < PMU_USB_TYPE_MAX) && (usb_type > PMU_USB_TYPE_NONE)) {
		bcmpmu_paccy_latch_event(paccy,
				BCMPMU_USB_EVENT_USB_DETECTION,
				&usb_type);
		if (board_id == EDN01x)
			bcmpmu_paccy_latch_event(paccy,
					BCMPMU_CHRGR_EVENT_CHRG_CURR_LMT,
					&chrgr_curr_lmt[chrgr_type]);
	} else
		bcmpmu_paccy_latch_event(paccy,
				BCMPMU_CHRGR_EVENT_CHRG_CURR_LMT,
				&chrgr_curr_lmt[chrgr_type]);
}

static void bcmpmu_accy_chrgr_detect_state(struct bcmpmu59xxx *bcmpmu, int val)
{
	u8 reg;
	bcmpmu->read_dev(bcmpmu, PMU_REG_MBCCTRL5, &reg);
	if (val)
		reg |= MBCCTRL5_BC12_EN_MASK;
	else
		reg &= ~MBCCTRL5_BC12_EN_MASK;
	bcmpmu->write_dev(bcmpmu, PMU_REG_MBCCTRL5, reg);
}

/* workq func */
static void usb_deferred_work(struct work_struct *work)
{
	int vbus_status = 0;
	struct list_head *pos, *temp;
	u32 event;
	void *para;
	unsigned long flags;
	struct event_list *entry ;
	struct bcmpmu_accy *paccy =
		container_of(work, struct bcmpmu_accy, det_work.work);
	struct bcmpmu59xxx *bcmpmu = paccy->bcmpmu;

	pr_accy(FLOW, "%s, enter state=%d, vbus=0x%X\n", __func__,
			paccy->det_state, vbus_status);
	pr_accy(FLOW, "###<%s> , DEV_STATE %x\n", __func__, paccy->det_state);

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
		paccy->usb_accy_data.max_curr_chrgr = (int)data;
		bcmpmu_call_notifier(paccy->bcmpmu,
				BCMPMU_CHRGR_EVENT_CHRG_CURR_LMT,
				&paccy->usb_accy_data.max_curr_chrgr);
		break;
	case BCMPMU_USB_CTRL_VBUS_ON_OFF:
		ret = bcmpmu->read_dev(bcmpmu, PMU_REG_OTGCTRL1, &temp);
		if (data == 0)
			temp &= (~OTGCTRL1_OFFVBUSB_MASK);
		else
			temp |= (OTGCTRL1_OFFVBUSB_MASK);
		ret = bcmpmu->write_dev(bcmpmu, PMU_REG_OTGCTRL1, temp);
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
		if ((data >= 0) && (data <= TPROBE_MAX_USEC)) {
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
		pr_accy(FLOW, "ENV4 %x val, id mak %x\n",
			val, ENV4_ID_CODE_SHIFT);
		val &= ENV4_ID_CODE_MASK;
		val = val >> ENV4_ID_CODE_SHIFT;
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
		val = paccy->usb_accy_data.chrgr_type;
		break;

	case BCMPMU_USB_CTRL_GET_USB_TYPE:
		ret = 0;
		val = paccy->usb_accy_data.usb_type;
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
	kfree(accy->free_list);

}

static int __devinit bcmpmu_accy_probe(struct platform_device *pdev)
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
	paccy->det_state = USB_IDLE;
	/*
	   paccy->usb_id_map = bcmpmu_get_usb_id_map(bcmpmu,
	   &paccy->usb_id_map_len);
	   */
	paccy->otg_block_enabled = 0;
	paccy->adp_block_enabled = 0;
	paccy->adp_prob_comp = 0;
	paccy->adp_sns_comp = 0;
	paccy->bc = pdata->bc;
	paccy->piggyback_chrg = pdata->piggyback_chrg;
	spin_lock_init(&paccy->accy_lock);

	INIT_WORK(&paccy->adp_work, usb_adp_work);
	INIT_DELAYED_WORK(&paccy->det_work, usb_deferred_work);

	ret = bcmpmu_accy_eventq_init(paccy);
	if (ret)
		goto err;

#ifdef CONFIG_HAS_WAKELOCK
	wake_lock_init(&paccy->wake_lock, WAKE_LOCK_SUSPEND, "usb_accy");
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
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_IDCHG,
				bcmpmu_accy_isr, paccy);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_RESUME_VBUS,
				bcmpmu_accy_isr, paccy);
	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_USBINS);
	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_USBRM);
	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_CHGDET_LATCH);
	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_CHGDET_TO);
	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_VA_SESS_VALID_R);
	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_VBUS_VALID_R);
	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_OTG_SESS_VALID_F);
	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_VBUS_VALID_F);
	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_IDCHG);
	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_RESUME_VBUS);
	/* in 59039 BC happened from PMU and BB's BCDAVDD33 pin is floated
	 * need to set bc_iso_i in order to avoid any impact due to
	 * BCDAVDD33 as floated.
	 */
#if defined(CONFIG_MFD_BCM59039) || defined(CONFIG_MFD_BCM59042) || \
	defined(CONFIG_MFD_BCM_PMU59xxx)
	/* Enable/Disable USB OTG clock aroudn bc_iso change */
	bcm_hsotgctrl_set_bc_iso(true);
#endif

#ifdef CONFIG_DEBUG_FS
	bcmpmu_accy_debug_init(paccy);
#endif
	usb_detect_state(paccy);
	return 0;

err:
	kfree(paccy);
	return ret;
}

static int __devexit bcmpmu_accy_remove(struct platform_device *pdev)
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
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_IDCHG);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_RESUME_VBUS);
	cancel_delayed_work_sync(&paccy->det_work);
	cancel_work_sync(&paccy->adp_work);
#ifdef CONFIG_HAS_WAKELOCK
	wake_lock_destroy(&paccy->wake_lock);
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
