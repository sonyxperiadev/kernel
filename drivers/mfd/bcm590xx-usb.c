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
#include <linux/slab.h>
#include <linux/bug.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/proc_fs.h>
#include <linux/plist.h>
#include <linux/power_supply.h>
#include <linux/delay.h>
#include <linux/wakelock.h>

#include <asm/io.h>
#include <mach/io_map.h>
#include <mach/rdb/brcm_rdb_hsotg_ctrl.h>
#include <mach/rdb/brcm_rdb_kpm_clk_mgr_reg.h>
#include <linux/mfd/bcm590xx/core.h>
#include <linux/mfd/bcm590xx/bcm590xx-usb.h>
#include <linux/usb/bcm_hsotgctrl.h>

#define BB_BC_STATUS            KONA_USB_HSOTG_CTRL_VA + HSOTG_CTRL_BC_STATUS_OFFSET
#define BB_BC_STS_BC_DONE_MSK   HSOTG_CTRL_BC_STATUS_BC_DONE_MASK

#ifdef CONFIG_ARCH_RHEA_BX
#define BB_BC_STS_SDP_MSK       HSOTG_CTRL_BC_STATUS_SDP_MASK
#define BB_BC_STS_CDP_MSK       HSOTG_CTRL_BC_STATUS_CDP_MASK
#define BB_BC_STS_DCP_MSK       HSOTG_CTRL_BC_STATUS_DCP_MASK
/* No action indicating error status bits for BC1.2 */
#define BB_BC_ERROR_STS_BITS		0
#else
#define BB_BC_STS_SDP_MSK       HSOTG_CTRL_BC_STATUS_SHP_MASK
#define BB_BC_STS_CDP_MSK       HSOTG_CTRL_BC_STATUS_CHP_MASK
#define BB_BC_STS_DCP_MSK       HSOTG_CTRL_BC_STATUS_DCP_MASK
#define BB_BC_ERROR_STS_BITS	(HSOTG_CTRL_BC_STATUS_DM_TIMEOUT_MASK | HSOTG_CTRL_BC_STATUS_DP_TIMEOUT_MASK | HSOTG_CTRL_BC_STATUS_DM_ERROR_MASK | HSOTG_CTRL_BC_STATUS_DP_ERROR_MASK)
#endif

#define BB_BC_CFG		KONA_USB_HSOTG_CTRL_VA + HSOTG_CTRL_BC_CFG_OFFSET
#define BB_BC_CFG_OVWR_KEY	(0x5556 << 17)
#define BB_BC_CFG_SW_OVWR_EN	(0x1 << 16)
#define BB_BC_CFG_SW_RST	(0x1 << 15)

#define MAX_BC_DET_RETRY	5
#define MAX_BC_STATUS_CHECK	20


#define DETECT_USB_CHARGER_FROM_BB_REGISTERS

struct event_notifier {
	u32 event_id;
	struct blocking_notifier_head notifiers;
};

struct bcmpmu_usb {
	struct bcm590xx *bcm590xx;
	struct event_notifier event[BCMPMU_EVENT_MAX];
	int usb_type;
	struct wake_lock wake_lock;
};

static struct bcmpmu_usb *usb_evnt_hndlr;

int bcmpmu_usb_add_notifier(u32 event_id, struct notifier_block *notifier)
{
	if (!usb_evnt_hndlr) {
			pr_info("%s: BCM59055 USB driver is not initialized\n", __func__);
			return -EAGAIN;
	}
	if (unlikely(event_id >= BCMPMU_EVENT_MAX)) {
		pr_info("%s: Invalid event id\n", __func__);
		return -EINVAL;
	}
	return blocking_notifier_chain_register(
			&usb_evnt_hndlr->event[event_id].notifiers, notifier);
}
EXPORT_SYMBOL_GPL(bcmpmu_usb_add_notifier);

int bcmpmu_usb_remove_notifier(u32 event_id, struct notifier_block *notifier)
{
	if (!usb_evnt_hndlr) {
		pr_info("%s: BCM59055 USB driver is not initialized\n", __func__);
		return -EAGAIN;
	}
	if (unlikely(event_id >= BCMPMU_EVENT_MAX)) {
		pr_info("%s: Invalid event id\n", __func__);
		return -EINVAL;
	}

	return blocking_notifier_chain_unregister(
			&usb_evnt_hndlr->event[event_id].notifiers, notifier);
}
EXPORT_SYMBOL_GPL(bcmpmu_usb_remove_notifier);

static int bcm_bc_detection(struct bcm590xx *bcm590xx)
{
	u8 regVal1;
	u32 regVal2;
	u32 bcStatus;
	int usb_type = USB_CHARGER_UNKNOWN;
	int count = 0;
	bcm_hsotgctrl_en_clock(true);
	bcStatus = readl(BB_BC_STATUS);
	pr_debug("%s: BC STATUS (0x%x) = 0x%x\n", __func__, (unsigned int)BB_BC_STATUS, bcStatus);
	/* Check if error occured while BC detection happened */
	if (bcStatus & BB_BC_ERROR_STS_BITS) {
		pr_debug("%s: Error occured while BC detection\n", __func__);
		/* set the code and sw enable bit in BC_CFG in BB */
		regVal2 = readl(BB_BC_CFG);
		regVal2 |= (BB_BC_CFG_OVWR_KEY | BB_BC_CFG_SW_OVWR_EN);
		writel(regVal2,  BB_BC_CFG);
		/* set sw rst bit */
		regVal2 |= BB_BC_CFG_SW_RST;
		writel(regVal2, BB_BC_CFG);
		schedule_timeout_interruptible(HZ/10);
		/* reset sw_rst back */
		regVal2 &= ~BB_BC_CFG_SW_RST;
		writel(regVal2, BB_BC_CFG);
		/* s/w bc detection gets enabled when USB_DET_LDO_EN is 1 */
		regVal1 = bcm590xx_reg_read(bcm590xx, BCM59055_REG_MBCCTRL5);
		regVal1 |= MBCCTRL5_USB_DET_LDO_EN;
		bcm590xx_reg_write(bcm590xx, BCM59055_REG_MBCCTRL5, regVal1);
		bcm_hsotgctrl_en_clock(false);
		return -EAGAIN;
	} /* BC error condition */
#ifdef DETECT_USB_CHARGER_FROM_BB_REGISTERS
	while (count < MAX_BC_STATUS_CHECK) {
		if (bcStatus & BB_BC_STS_BC_DONE_MSK) {
			if (bcStatus & BB_BC_STS_SDP_MSK)
				usb_type = USB_CHARGER_SDP;
			else if (bcStatus & BB_BC_STS_CDP_MSK)
				usb_type = USB_CHARGER_CDP;
			else if (bcStatus & BB_BC_STS_DCP_MSK)
				usb_type = USB_CHARGER_DCP;
			else
				usb_type = USB_CHARGER_UNKNOWN; /* Need to update the logic for new charger types for RheaB0 */
			break;
		} else {
			msleep(10);
			count++;
			bcStatus = readl(BB_BC_STATUS);
			pr_debug("%s: BC STATUS (0x%x) = 0x%x..Count %d\n", __func__, (unsigned int)BB_BC_STATUS, bcStatus, count);
		}
	}
	if (count == MAX_BC_STATUS_CHECK) {
			pr_info("%s: BC Detection DONE bit is not set, check if BCDLDO was ON\n", __func__);
			bcm_hsotgctrl_en_clock(false);
			return -EIO;
	}
#else
	regVal1 = bcm590xx_reg_read(bcm590xx, BCM59055_REG_MBCCTRL5);
	regVal1 >>= MBCCTRL5_CHP_TYP_SHIFT;
	usb_type = (regVal1 & MBCCTRL5_CHP_TYP_MASK);
#endif
	bcm_hsotgctrl_en_clock(false);
	return usb_type;
}

static void usb_charger_det(struct bcmpmu_usb *bcmpmu_usb)
{
	struct bcm590xx *bcm590xx = bcmpmu_usb->bcm590xx;
	u8 regVal1, regVal2;
	int i, ret, event;
	pr_debug("%s ----\n", __func__);

	/* check if P_VBUS_VALID is set and USB_PORT_DISABLE is not set */
	regVal1 = bcm590xx_reg_read(bcm590xx, BCM59055_REG_ENV3);
	regVal2 = bcm590xx_reg_read(bcm590xx, BCM59055_REG_ENV4);

	if (!(regVal1 & BCM59055_ENV3_USB_PORT_DIS) &&
			(regVal2 & BCM59055_ENV4_P_VBUS_VALID)) {
		for (i = 0; i < MAX_BC_DET_RETRY; i++) {
			ret = bcm_bc_detection(bcm590xx);
			if (ret != -EAGAIN)
				break;
		}
		bcmpmu_usb->usb_type = ret;
		event = BCMPMU_USB_EVENT_CHGR_DETECTION;
		blocking_notifier_call_chain(&bcmpmu_usb->event[BCMPMU_USB_EVENT_CHGR_DETECTION].notifiers,
				event, &ret);
	}
}

int bcmpmu_usb_set(int command, int data, void *ptr)
{
	struct bcm590xx *bcm590xx;
	u8 reg;
	int ret = 0;
	pr_debug("Inside %s\n", __func__);
	if (!usb_evnt_hndlr->bcm590xx)
		bcm590xx  = (struct bcm590xx *)ptr;
	else
		bcm590xx = usb_evnt_hndlr->bcm590xx;

	switch (command) {
	case BCMPMU_CTRL_VBUS_ON_OFF:
		reg = bcm590xx_reg_read(bcm590xx, BCM59055_REG_OTGCTRL1);
		if (data == 0)
			reg &= ~OTGCTRL1_OFFVBUSB;
		else
			reg |= OTGCTRL1_OFFVBUSB;
		ret = bcm590xx_reg_write(bcm590xx, BCM59055_REG_OTGCTRL1, reg);
		break;
	case BCMPMU_CTRL_SET_VBUS_DEB_TIME:
		reg = bcm590xx_reg_read(bcm590xx, BCM59055_REG_OTGCTRL2);
		reg &= ~OTGCTRL2_VBUS_ADP_COMP_DB_MASK;
		reg |= (data & OTGCTRL2_VBUS_ADP_COMP_DB_MASK);
		ret = bcm590xx_reg_write(bcm590xx, BCM59055_REG_OTGCTRL1, reg);
		break;
	case BCMPMU_CTRL_SRP_VBUS_PULSE:
		reg = bcm590xx_reg_read(bcm590xx, BCM59055_REG_OTGCTRL1);
		if (data == 0)
			reg &= ~OTGCTRL1_EN_VBUS_PULSE;
		else
			reg |= OTGCTRL1_EN_VBUS_PULSE;
		ret = bcm590xx_reg_write(bcm590xx, BCM59055_REG_OTGCTRL1, reg);
		break;
	case BCMPMU_CTRL_DISCHRG_VBUS:
		reg = bcm590xx_reg_read(bcm590xx, BCM59055_REG_OTGCTRL1);
		if (data == 0)
			reg &= ~OTGCTRL1_EN_PD_SRP;
		else
			reg |= OTGCTRL1_EN_PD_SRP;
		ret = bcm590xx_reg_write(bcm590xx, BCM59055_REG_OTGCTRL1, reg);
		break;
	case BCMPMU_CTRL_START_STOP_ADP_SENS_PRB:
		reg = bcm590xx_reg_read(bcm590xx, BCM59055_REG_OTGCTRL11);
		if (data == 0)
			reg &= ~OTGCTRL11_ADP_SNS;
		else
			reg |= OTGCTRL11_ADP_SNS;
		ret = bcm590xx_reg_write(bcm590xx, BCM59055_REG_OTGCTRL11, reg);
		break;
	case BCMPMU_CTRL_START_STOP_ADP_PRB:
		reg = bcm590xx_reg_read(bcm590xx, BCM59055_REG_OTGCTRL11);
		if (data == 0)
			reg &= ~OTGCTRL11_ADP_PRB;
		else
			reg |= OTGCTRL11_ADP_PRB;
		ret = bcm590xx_reg_write(bcm590xx, BCM59055_REG_OTGCTRL11, reg);
		break;
	case BCMPMU_CTRL_START_ADP_CAL_PRB:
		break;
	case BCMPMU_CTRL_SET_ADP_PRB_MOD:
		break;
	case BCMPMU_CTRL_SET_ADP_PRB_CYC_TIME:
		break;
	case BCMPMU_CTRL_SET_ADP_COMP_METHOD:
		break;
	default:
		pr_info("%s: Invalid command\n", __func__);
		return -EINVAL;
	}
	return 0;
}
EXPORT_SYMBOL_GPL(bcmpmu_usb_set);

int bcmpmu_usb_get(int command, void *ptr)
{
	int val;
	struct bcm590xx *bcm590xx;
	u8 reg;
	pr_debug("Inside %s\n", __func__);
	if (!usb_evnt_hndlr->bcm590xx)
		bcm590xx  = (struct bcm590xx *)ptr;
	else
		bcm590xx = usb_evnt_hndlr->bcm590xx;

	switch (command) {
	case BCMPMU_CTRL_GET_ADP_CHANGE_STATUS:
		reg = bcm590xx_reg_read(bcm590xx, BCM59055_REG_OTGSTS1);
		val = (reg & OTGSTS1_ADP_ATTACH_DET) >> OTGSTS1_ADP_ATTACH_DET_SHIFT;
		break;
	case BCMPMU_CTRL_GET_ADP_SENSE_STATUS:
		reg = bcm590xx_reg_read(bcm590xx, BCM59055_REG_OTGSTS1);
		val = (reg & OTGSTS1_ADP_SNS_DET) >> OTGSTS1_ADP_SNS_DET_SHIFT;
		break;
	case BCMPMU_CTRL_GET_ADP_PRB_RISE_TIMES:
		reg = bcm590xx_reg_read(bcm590xx, BCM59055_REG_OTGSTS2); /*  LSB */
		val = reg;
		reg = bcm590xx_reg_read(bcm590xx, BCM59055_REG_OTGSTS3); /*  MSB */
		val |= (reg << 8);
		break;
	case BCMPMU_CTRL_GET_VBUS_STATUS:
		reg = bcm590xx_reg_read(bcm590xx, BCM59055_REG_ENV4);
		val = (reg & BCM59055_ENV4_P_VBUS_VALID);
		break;
	case BCMPMU_CTRL_GET_SESSION_STATUS:
		reg = bcm590xx_reg_read(bcm590xx, BCM59055_REG_ENV4);
		val = (reg & BCM59055_ENV4_P_OTG_SESS_VALID) >> 1;
		break;
	case BCMPMU_CTRL_GET_SESSION_END_STATUS:
		reg = bcm590xx_reg_read(bcm590xx, BCM59055_REG_ENV4);
		val = (reg & BCM59055_ENV4_P_VB_SESS_END) >> 2;
		break;
	case BCMPMU_CTRL_GET_ID_VALUE:
		reg = bcm590xx_reg_read(bcm590xx, BCM59055_REG_ENV4);
		val = (reg >> BCM59055_ENV4_RID_SHIFT) & BCM59055_ENV4_RID_MASK;
		break;
	case BCMPMU_CTRL_GET_CHARGER_TYPE:
	{
		int i;
		for (i = 0; i < MAX_BC_DET_RETRY; i++) {
			val = bcm_bc_detection(bcm590xx);
			if (val != -EAGAIN)
			break;
		}
		break;
	}
	default:
		pr_info("%s: Invalid command\n", __func__);
		val = -EINVAL;
	}
	return val;
}
EXPORT_SYMBOL_GPL(bcmpmu_usb_get);

static void bcmpmu_usb_isr(int intr, void *data)
{
	struct bcmpmu_usb *bcmpmu_usb = data;
	int event = -1;
	int evnt_sts;
	pr_debug("%s: Interrupt number %d\n", __func__, intr);

	switch(intr) {
	case BCM59055_IRQID_INT2_USBINS:
		wake_lock(&bcmpmu_usb->wake_lock);
		event = BCMPMU_USB_EVENT_IN_RM;
		evnt_sts = 1;
		blocking_notifier_call_chain(&bcmpmu_usb->event[BCMPMU_USB_EVENT_IN_RM].notifiers,
				event, &evnt_sts);
		break;
	case BCM59055_IRQID_INT2_USBRM:
		wake_unlock(&bcmpmu_usb->wake_lock);
		event = BCMPMU_USB_EVENT_IN_RM;
		evnt_sts = 0;
		blocking_notifier_call_chain(&bcmpmu_usb->event[BCMPMU_USB_EVENT_IN_RM].notifiers,
				event, &evnt_sts);
		break;
	case BCM59055_IRQID_INT5_ID_CHG:
		event = BCMPMU_USB_EVENT_ID_CHANGE;
		blocking_notifier_call_chain(&bcmpmu_usb->event[BCMPMU_USB_EVENT_ID_CHANGE].notifiers,
				event, NULL);
		break;
	case BCM59055_IRQID_INT4_VBUS_VALID_F:
		event = BCMPMU_USB_EVENT_VBUS_INVALID;
		blocking_notifier_call_chain(&bcmpmu_usb->event[BCMPMU_USB_EVENT_VBUS_INVALID].notifiers,
					event, NULL);
		break;
	case BCM59055_IRQID_INT4_VBUS_VALID_R:
		event = BCMPMU_USB_EVENT_VBUS_VALID;
		blocking_notifier_call_chain(&bcmpmu_usb->event[BCMPMU_USB_EVENT_VBUS_VALID].notifiers,
					event, NULL);
		break;

	case BCM59055_IRQID_INT4_OTG_SESS_VALID_F:
		event = BCMPMU_USB_EVENT_SESSION_INVALID;
		blocking_notifier_call_chain(&bcmpmu_usb->event[BCMPMU_USB_EVENT_SESSION_INVALID].notifiers,
				event, NULL);
		break;
	case BCM59055_IRQID_INT4_VB_SESS_END_F:
		event = BCMPMU_USB_EVENT_SESSION_END_INVALID;
		blocking_notifier_call_chain(&bcmpmu_usb->event[BCMPMU_USB_EVENT_SESSION_END_INVALID].notifiers,
					event, NULL);
		break;
	case BCM59055_IRQID_INT4_VB_SESS_END_R:
		event = BCMPMU_USB_EVENT_SESSION_END_VALID;
		blocking_notifier_call_chain(&bcmpmu_usb->event[BCMPMU_USB_EVENT_SESSION_END_VALID].notifiers,
					event, NULL);
		break;

	case BCM59055_IRQID_INT4_OTG_A_DEVICE:
		break;
	case BCM59055_IRQID_INT4_VA_SESS_VALID_R:
		event = BCMPMU_USB_EVENT_SESSION_VALID;
		blocking_notifier_call_chain(&bcmpmu_usb->event[BCMPMU_USB_EVENT_SESSION_VALID].notifiers,
				event, NULL);
		break;
	case BCM59055_IRQID_INT5_RIC_C_TO_FLOAT:
		event = BCMPMU_USB_EVENT_RIC_C_TO_FLOAT;
		blocking_notifier_call_chain(&bcmpmu_usb->event[BCMPMU_USB_EVENT_RIC_C_TO_FLOAT].notifiers,
				event, NULL);
		break;
	case BCM59055_IRQID_INT5_ADP_CHANGE:
		event = BCMPMU_USB_EVENT_ADP_CHANGE;
		blocking_notifier_call_chain(&bcmpmu_usb->event[BCMPMU_USB_EVENT_ADP_CHANGE].notifiers,
				event, NULL);
		break;
	case BCM59055_IRQID_INT5_ADP_SNS_END:
		event = BCMPMU_USB_EVENT_ADP_SENSE_END;
		blocking_notifier_call_chain(&bcmpmu_usb->event[BCMPMU_USB_EVENT_ADP_SENSE_END].notifiers,
				event, NULL);
		break;
	case BCM59055_IRQID_INT5_CHGDET_TO:
		usb_charger_det(bcmpmu_usb);
		break;
	case BCM59055_IRQID_INT5_CHGDET_LATCH:
		usb_charger_det(bcmpmu_usb);
		break;
	default:
		pr_info("%s: %d interrupt does not get handled\n", __func__, intr);
		break;
	}
}

static int __devinit bcmpmu_usb_probe(struct platform_device *pdev)
{
	struct bcmpmu_usb *handler;
	struct bcm590xx *bcm590xx = dev_get_drvdata(pdev->dev.parent);
	int ret = 0, i;
	printk("BCMPMU USB Accy Driver\n");
	handler = kzalloc(sizeof(struct bcmpmu_usb), GFP_KERNEL);
	if (!handler) {
		pr_info("%s: failed to alloc mem.\n", __func__);
		return -ENOMEM;
	}
	handler->bcm590xx = bcm590xx;
	usb_evnt_hndlr = handler;
	/* init all event notification heads */
	for (i = 0; i < BCMPMU_EVENT_MAX; i++) {
		handler->event[i].event_id = i;
		BLOCKING_INIT_NOTIFIER_HEAD(&handler->event[i].notifiers);
	}
	wake_lock_init(&handler->wake_lock, WAKE_LOCK_SUSPEND, "bcmpmu-usb");
	/* Register for interrupts */
	ret |= bcm590xx_request_irq(bcm590xx, BCM59055_IRQID_INT2_USBINS,
			true, bcmpmu_usb_isr, handler);
	ret |= bcm590xx_request_irq(bcm590xx, BCM59055_IRQID_INT2_USBRM,
			true, bcmpmu_usb_isr, handler);

	ret |= bcm590xx_request_irq(bcm590xx, BCM59055_IRQID_INT4_VBUS_VALID_F,
			true, bcmpmu_usb_isr, handler);

	ret |= bcm590xx_request_irq(bcm590xx, BCM59055_IRQID_INT4_OTG_SESS_VALID_F,
			true, bcmpmu_usb_isr, handler);
	ret |= bcm590xx_request_irq(bcm590xx, BCM59055_IRQID_INT4_VB_SESS_END_F,
			true, bcmpmu_usb_isr, handler);
	ret |= bcm590xx_request_irq(bcm590xx, BCM59055_IRQID_INT4_OTG_A_DEVICE,
			true, bcmpmu_usb_isr, handler);
	ret |= bcm590xx_request_irq(bcm590xx, BCM59055_IRQID_INT4_VBUS_VALID_R,
			true, bcmpmu_usb_isr, handler);
	ret |= bcm590xx_request_irq(bcm590xx, BCM59055_IRQID_INT4_VA_SESS_VALID_R,
			true, bcmpmu_usb_isr, handler);
	ret |= bcm590xx_request_irq(bcm590xx, BCM59055_IRQID_INT4_VB_SESS_END_R,
			true, bcmpmu_usb_isr, handler);
	ret |= bcm590xx_request_irq(bcm590xx, BCM59055_IRQID_INT4_OTG_B_DEVICE,
			true, bcmpmu_usb_isr, handler);

	ret |= bcm590xx_request_irq(bcm590xx, BCM59055_IRQID_INT5_ID_CHG,
			true, bcmpmu_usb_isr, handler);
	ret |= bcm590xx_request_irq(bcm590xx, BCM59055_IRQID_INT5_CHGDET_LATCH,
			true, bcmpmu_usb_isr, handler);
	ret |= bcm590xx_request_irq(bcm590xx, BCM59055_IRQID_INT5_CHGDET_TO,
			true, bcmpmu_usb_isr, handler);
	ret |= bcm590xx_request_irq(bcm590xx, BCM59055_IRQID_INT5_RIC_C_TO_FLOAT,
			true, bcmpmu_usb_isr, handler);
	ret |= bcm590xx_request_irq(bcm590xx, BCM59055_IRQID_INT5_ADP_CHANGE,
			true, bcmpmu_usb_isr, handler);
	ret |= bcm590xx_request_irq(bcm590xx, BCM59055_IRQID_INT5_ADP_SNS_END,
			true, bcmpmu_usb_isr, handler);

	if (ret) {
		pr_info("%s: Error registering for interrupts\n", __func__);
		goto Err_Interrupt;
	}
	platform_set_drvdata(pdev, handler);
	return ret;
Err_Interrupt:
	kfree(handler);
	return ret;
}

static int __devexit bcmpmu_usb_remove(struct platform_device *pdev)
{
	struct bcmpmu_usb *handler = platform_get_drvdata(pdev);
	struct bcm590xx *bcm590xx = handler->bcm590xx;


	bcm590xx_free_irq(bcm590xx, BCM59055_IRQID_INT2_USBINS);
	bcm590xx_free_irq(bcm590xx, BCM59055_IRQID_INT2_USBRM);
	bcm590xx_free_irq(bcm590xx, BCM59055_IRQID_INT4_VBUS_VALID_F);
	bcm590xx_free_irq(bcm590xx, BCM59055_IRQID_INT4_OTG_SESS_VALID_F);
	bcm590xx_free_irq(bcm590xx, BCM59055_IRQID_INT4_VB_SESS_END_F);
	bcm590xx_free_irq(bcm590xx, BCM59055_IRQID_INT4_OTG_A_DEVICE);
	bcm590xx_free_irq(bcm590xx, BCM59055_IRQID_INT4_VBUS_VALID_R);
	bcm590xx_free_irq(bcm590xx, BCM59055_IRQID_INT4_VA_SESS_VALID_R);
	bcm590xx_free_irq(bcm590xx, BCM59055_IRQID_INT4_VB_SESS_END_R);
	bcm590xx_free_irq(bcm590xx, BCM59055_IRQID_INT4_OTG_B_DEVICE);
	bcm590xx_free_irq(bcm590xx, BCM59055_IRQID_INT5_ID_CHG);
	bcm590xx_free_irq(bcm590xx, BCM59055_IRQID_INT5_CHGDET_LATCH);
	bcm590xx_free_irq(bcm590xx, BCM59055_IRQID_INT5_CHGDET_TO);
	bcm590xx_free_irq(bcm590xx, BCM59055_IRQID_INT5_RIC_C_TO_FLOAT);
	bcm590xx_free_irq(bcm590xx, BCM59055_IRQID_INT5_ADP_CHANGE);
	bcm590xx_free_irq(bcm590xx, BCM59055_IRQID_INT5_ADP_SNS_END);

	wake_lock_destroy(&handler->wake_lock);
	kfree(handler);
	return 0;
}

static struct platform_driver bcmpmu_usb_driver = {
	.driver = {
		.name = "bcmpmu_usb",
	},
	.probe = bcmpmu_usb_probe,
	.remove = __devexit_p(bcmpmu_usb_remove),
};

static int __init bcmpmu_usb_init(void)
{
	return platform_driver_register(&bcmpmu_usb_driver);
}
module_init(bcmpmu_usb_init);

static void __exit bcmpmu_usb_exit(void)
{
        platform_driver_unregister(&bcmpmu_usb_driver);
}
module_exit(bcmpmu_usb_exit);

MODULE_DESCRIPTION("BCMPMU USB Driver");
MODULE_LICENSE("GPL");
