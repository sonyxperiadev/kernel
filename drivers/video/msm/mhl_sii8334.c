/*
 * Copyright (C) 2011 Sony Ericsson Mobile Communications AB.
 * Copyright (C) 2012 Sony Mobile Communications AB.
 * Copyright (C) 2011 Silicon Image Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

/* #define DEBUG */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/bitops.h>
#include <linux/mutex.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/completion.h>
#include <linux/sched.h>

#include <linux/i2c.h>
#include <linux/gpio.h>

#include <linux/mhl.h>
#include <linux/mhl_defs.h>
#include <linux/mhl_sii8334.h>

#include <linux/wakelock.h>
#include <linux/workqueue.h>

#ifdef DEBUG
#define MHL_DEV_DBG(format, arg...) \
	if (mhl_state->mhl_dev) \
		dev_info(&mhl_state->mhl_dev->dev, format, ##arg)
#else
#define MHL_DEV_DBG(format, arg...) \
	do {} while (0)
#endif

#define MSC_COMMAND_TIME_OUT 2050
#define CHARGER_INIT_WAIT 30
#define CHARGER_INIT_DELAYED_TIME 20
enum {
	POWER_STATE_D0_MHL = 0,
	POWER_STATE_D0_NO_MHL = 2,
	POWER_STATE_D3 = 3,
	POWER_STATE_FIRST_INIT = 0xFF,
};

struct mhl_sii_state_struct {
	int reset;
	int irq;
	int hpd_pin_mode;
	int int_pin_mode;
	struct i2c_client client;
	struct mhl_device *mhl_dev;

	int (*low_power_mode)(int enable);
	int (*charging_enable)(int enable, int max_curr);

	unsigned int adopter_id;
	unsigned int device_id;

	int power_state;

	/* device discovery stuff */
	int mhl_mode;
	int notify_plugged;
	struct completion rgnd_done;
	struct timer_list discovery_timer;

	/* MSC command stuff */
	struct completion msc_command_done;
	struct work_struct timer_work;
};

static struct mhl_sii_state_struct *mhl_state;
static DEFINE_MUTEX(mhl_state_mutex);
static DEFINE_MUTEX(mhl_hpd_tmds_mutex);
static struct wake_lock mhl_wake_lock;

static u8 chip_rev_id;

static void mhl_sii_chip_init(void);
static int mhl_sii_hpd_control(int enable);

static int mhl_sii_reg_read(u8 addr, u8 off, u8 *buff, u16 len)
{
	int rc;
	struct i2c_msg msgs[2];

	msgs[0].addr = addr >> 1;
	msgs[0].buf = &off;
	msgs[0].len = 1;
	msgs[0].flags = 0;

	msgs[1].addr = addr >> 1;
	msgs[1].buf = buff;
	msgs[1].len = len;
	msgs[1].flags = I2C_M_RD;

	rc = i2c_transfer(mhl_state->client.adapter, msgs, 2);
	if (rc < 2) {
		pr_err("%s: i2c_transfer failed (%d)\n",
			__func__, rc);
		goto out_reg_read;
	}

	return 0;
out_reg_read:
	return rc;
}

static int mhl_sii_reg_write(u8 addr, u8 off, u8 *buff, u16 len)
{
	int rc;
	struct i2c_msg msgs[1];
	u8 *buffer;

	buffer = kmalloc(1 + len, GFP_KERNEL);
	if (!buffer)
		return -ENOMEM;
	*buffer = off;
	memcpy(buffer + 1, buff, len);

	msgs[0].addr = addr >> 1;
	msgs[0].buf = buffer;
	msgs[0].len = 1 + len;
	msgs[0].flags = 0;

	rc = i2c_transfer(mhl_state->client.adapter, msgs, 1);
	if (rc < 1) {
		pr_err("%s: i2c_transfer failed (%d)\n",
			__func__, rc);
		goto out_reg_write;
	}

	kfree(buffer);
	return 0;
out_reg_write:
	kfree(buffer);
	return rc;
}

static u8 mhl_sii_reg_read_byte(u8 addr, u8 off)
{
	u8 val;
	return mhl_sii_reg_read(addr, off, &val, 1) ? (u8) 0xFF : val;
}

static int mhl_sii_reg_write_byte(u8 addr, u8 off, u8 value)
{
	return mhl_sii_reg_write(addr, off, &value, 1);
}

#define MHL_SII_I2C_ADDR_PAGE0	0x72	/* TPI */
#define MHL_SII_I2C_ADDR_PAGE1	0x7A	/* AVI */
#define MHL_SII_I2C_ADDR_PAGE2	0x92	/* Analog */
#define MHL_SII_I2C_ADDR_PAGE3	0x9A	/* Ctrl */
#define MHL_SII_I2C_ADDR_RSVD	0xC0	/* RFU */
#define MHL_SII_I2C_ADDR_CBUS	0xC8	/* CBUS */

#define MHL_SII_PAGE0_REG_READ(off) \
	mhl_sii_reg_read_byte(MHL_SII_I2C_ADDR_PAGE0, off)
#define MHL_SII_PAGE0_REG_WRITE(off, val) \
	mhl_sii_reg_write_byte(MHL_SII_I2C_ADDR_PAGE0, off, val)

#define MHL_SII_PAGE1_REG_READ(off) \
	mhl_sii_reg_read_byte(MHL_SII_I2C_ADDR_PAGE1, off)
#define MHL_SII_PAGE1_REG_WRITE(off, val) \
	mhl_sii_reg_write_byte(MHL_SII_I2C_ADDR_PAGE1, off, val)

#define MHL_SII_PAGE2_REG_READ(off) \
	mhl_sii_reg_read_byte(MHL_SII_I2C_ADDR_PAGE2, off)
#define MHL_SII_PAGE2_REG_WRITE(off, val) \
	mhl_sii_reg_write_byte(MHL_SII_I2C_ADDR_PAGE2, off, val)

#define MHL_SII_PAGE3_REG_READ(off) \
	mhl_sii_reg_read_byte(MHL_SII_I2C_ADDR_PAGE3, off)
#define MHL_SII_PAGE3_REG_WRITE(off, val) \
	mhl_sii_reg_write_byte(MHL_SII_I2C_ADDR_PAGE3, off, val)

#define MHL_SII_RSVD_REG_READ(off) \
	mhl_sii_reg_read_byte(MHL_SII_I2C_ADDR_RSVD, off)
#define MHL_SII_RSVD_REG_WRITE(off, val) \
	mhl_sii_reg_write_byte(MHL_SII_I2C_ADDR_RSVD, off, val)

#define MHL_SII_CBUS_REG_READ(off) \
	mhl_sii_reg_read_byte(MHL_SII_I2C_ADDR_CBUS, off)
#define MHL_SII_CBUS_REG_WRITE(off, val) \
	mhl_sii_reg_write_byte(MHL_SII_I2C_ADDR_CBUS, off, val)

#define MSC_START_BIT_MSC_CMD		        (0x01 << 0)
#define MSC_START_BIT_VS_CMD		        (0x01 << 1)
#define MSC_START_BIT_READ_REG		        (0x01 << 2)
#define MSC_START_BIT_WRITE_REG		        (0x01 << 3)
#define MSC_START_BIT_WRITE_BURST	        (0x01 << 4)

static int mhl_sii_send_msc_command(struct msc_command_struct *req)
{
	u8 start_bit = 0x00;
	u8 *burst_data;
	int timeout;
	int i;

	if (mhl_state->power_state != POWER_STATE_D0_MHL) {
		MHL_DEV_DBG("%s: power_state:%02x CBUS(0x0A):%02x\n",
		__func__,
		mhl_state->power_state, MHL_SII_CBUS_REG_READ(0x0A));
		return -EFAULT;
	}

	if (!req)
		return -EFAULT;

	MHL_DEV_DBG("%s: command=0x%02x offset=0x%02x %02x %02x",
		__func__,
		req->command,
		req->offset,
		req->payload.data[0],
		req->payload.data[1]);

	MHL_SII_CBUS_REG_WRITE(0x13, req->offset);
	MHL_SII_CBUS_REG_WRITE(0x14, req->payload.data[0]);

	switch (req->command) {
	case MHL_SET_INT:
	case MHL_WRITE_STAT:
		start_bit = MSC_START_BIT_WRITE_REG;
		break;
	case MHL_READ_DEVCAP:
		start_bit = MSC_START_BIT_READ_REG;
		break;
	case MHL_GET_STATE:
	case MHL_GET_VENDOR_ID:
	case MHL_SET_HPD:
	case MHL_CLR_HPD:
	case MHL_GET_SC1_ERRORCODE:
	case MHL_GET_DDC_ERRORCODE:
	case MHL_GET_MSC_ERRORCODE:
	case MHL_GET_SC3_ERRORCODE:
		start_bit = MSC_START_BIT_MSC_CMD;
		MHL_SII_CBUS_REG_WRITE(0x13, req->command);
		break;
	case MHL_MSC_MSG:
		start_bit = MSC_START_BIT_VS_CMD;
		MHL_SII_CBUS_REG_WRITE(0x15, req->payload.data[1]);
		MHL_SII_CBUS_REG_WRITE(0x13, req->command);
		break;
	case MHL_WRITE_BURST:
		start_bit = MSC_START_BIT_WRITE_BURST;
		MHL_SII_CBUS_REG_WRITE(0x20, req->length - 1);
		if (!(req->payload.burst_data)) {
			pr_err("%s: burst data is null!\n", __func__);
			goto cbus_command_send_out;
		}
		burst_data = req->payload.burst_data;
		for (i = 0; i < req->length; i++, burst_data++)
			MHL_SII_CBUS_REG_WRITE(0xC0 + i, *burst_data);
		break;
	default:
		pr_err("%s: unknown command! (%02x)\n",
			__func__, req->command);
		goto cbus_command_send_out;
	}

	init_completion(&mhl_state->msc_command_done);
	MHL_SII_CBUS_REG_WRITE(0x12, start_bit);
	timeout = wait_for_completion_interruptible_timeout
		(&mhl_state->msc_command_done,
		msecs_to_jiffies(MSC_COMMAND_TIME_OUT));
	if (!timeout) {
		pr_err("%s: cbus_command_send timed out!\n", __func__);
		goto cbus_command_send_out;
	}

	/* ok, call back mhl driver with return value */
	switch (req->command) {
	case MHL_READ_DEVCAP:
		req->retval = MHL_SII_CBUS_REG_READ(0x16);	/* devcap */
		break;
	case MHL_MSC_MSG:
		/* check if MSC_MSG NACKed */
		if (MHL_SII_CBUS_REG_READ(0x20) & BIT(6))
			return -EAGAIN;
		/* FALLTHROUGH */
	default:
		req->retval = 0;
		break;
	}
	mhl_msc_command_done(mhl_state->mhl_dev, req);

	MHL_DEV_DBG("%s: done\n", __func__);

	return 0;
cbus_command_send_out:
	return -EFAULT;
}

static void mhl_sii_switch_power_state(int state)
{
	u8 regval;

	switch (state) {
	case POWER_STATE_D3:
		mutex_lock(&mhl_state_mutex);
		mhl_state->power_state = POWER_STATE_D3;
		mutex_unlock(&mhl_state_mutex);

		/* Force HPD = 0 */
		mhl_sii_hpd_control(FALSE);

		/* Change TMDS termination to
		   high impedance on disconnection */
		MHL_SII_PAGE3_REG_WRITE(0x30, 0xD0);

		/* wait Tsrc:cbus_float */
		msleep(50);

		/* Change state to D3 by clearing bit 0 SW_TPI */
		regval = MHL_SII_PAGE1_REG_READ(0x3D);
		regval &= ~BIT(0);
		MHL_SII_PAGE1_REG_WRITE(0x3D, regval);

		MHL_DEV_DBG("mhl: power state switched to D3\n");
		break;
	case POWER_STATE_D0_NO_MHL:
		mutex_lock(&mhl_state_mutex);
		mhl_state->power_state = POWER_STATE_D0_NO_MHL;
		mutex_unlock(&mhl_state_mutex);

		mhl_sii_chip_init();

		MHL_SII_PAGE3_REG_WRITE(0x10, 0x25);
		regval = MHL_SII_PAGE0_REG_READ(0x1E);
		regval &= ~(BIT(0) | BIT(1));
		MHL_SII_PAGE0_REG_WRITE(0x1E, regval);

		MHL_DEV_DBG("mhl: power state switched to D0\n");
		break;
	default:
		pr_err("mhl: invalid power state! (%d)\n", state);
		break;
	}
}

static void mhl_sii_int1_isr(void)
{
	u8 regval;
	regval = MHL_SII_PAGE0_REG_READ(0x71);
	if (regval) {
		/* Clear all interrupts coming from this register. */
		MHL_SII_PAGE0_REG_WRITE(0x71, regval);
		if (regval & BIT(6)) {
			regval = MHL_SII_CBUS_REG_READ(0x0D);
			mhl_notify_hpd(mhl_state->mhl_dev,
				!!(regval & BIT(6)));
		}
	}
}

static void mhl_sii_scdt_status_change(void)
{
	u8 regval = MHL_SII_PAGE3_REG_READ(0x40);
	MHL_DEV_DBG("MHL: scdt status (0x%02x)\n", regval);
	if (regval & 0x02) {
		u8 fifo_status = MHL_SII_PAGE3_REG_READ(0x23);
		MHL_DEV_DBG("MHL: MHL FIFO status = 0x%02x\n", fifo_status);
		if (fifo_status & 0x0C) {
			MHL_SII_PAGE3_REG_WRITE(0x23, 0x0C);
			MHL_DEV_DBG("MHL: MHL FIFO reset!\n");
			MHL_SII_PAGE3_REG_WRITE(0x00, 0x94);
			MHL_SII_PAGE3_REG_WRITE(0x00, 0x84);
		}
	}
}

static int mhl_sii_rgnd(void)
{
	int ret;
	int counter;
	u8 regval = MHL_SII_PAGE3_REG_READ(0x1C) & (BIT(1) | BIT(0));
	/* 00, 01 or 11 means USB. */
	/* 10 means 1K impedance (MHL) */
	mutex_lock(&mhl_state_mutex);
	if (regval == 0x02) {
		if (timer_pending(&mhl_state->discovery_timer))
			del_timer(&mhl_state->discovery_timer);
		mhl_state->mhl_mode = TRUE;
		mhl_state->notify_plugged = TRUE;
		wake_lock(&mhl_wake_lock);
		mutex_unlock(&mhl_state_mutex);
		mhl_notify_plugged(mhl_state->mhl_dev);
		if (mhl_state->charging_enable) {
			/* 700 means 700mA charging ready */
			ret = mhl_state->charging_enable(TRUE, 700);
			if (ret) {
				/* We used late_initcall before for
				 * charger_enable(). Still, call of
				 * charger_enable() is some fail.
				 * Waits here until initialization success it.
				 */
				counter = CHARGER_INIT_WAIT;
				while (ret && counter--) {
					msleep(CHARGER_INIT_DELAYED_TIME);
					ret = mhl_state->charging_enable
						(TRUE, 700);
				}
			}
		}
		pr_info("mhl: MHL detected\n");
		complete_all(&mhl_state->rgnd_done);
	} else {
		wake_unlock(&mhl_wake_lock);
		mhl_state->mhl_mode = FALSE;
		mutex_unlock(&mhl_state_mutex);
		pr_info("mhl: USB detected\n");
		complete_all(&mhl_state->rgnd_done);
	}
	return mhl_state->mhl_mode ?
		MHL_DISCOVERY_RESULT_MHL : MHL_DISCOVERY_RESULT_USB;
}

static void mhl_sii_connection(void)
{
	u8 regval;

	mutex_lock(&mhl_state_mutex);
	if (mhl_state->power_state == POWER_STATE_D0_MHL) {
		mutex_unlock(&mhl_state_mutex);
		return;
	}
	mhl_state->power_state = POWER_STATE_D0_MHL;
	mutex_unlock(&mhl_state_mutex);

	MHL_SII_PAGE3_REG_WRITE(0x30, 0x10);

	MHL_SII_CBUS_REG_WRITE(0x07, 0xF2);

	/* Keep the discovery enabled. Need RGND interrupt */
	regval = MHL_SII_PAGE3_REG_READ(0x10);
	MHL_SII_PAGE3_REG_WRITE(0x10, regval | BIT(0));

	mhl_notify_online(mhl_state->mhl_dev);
}

static void mhl_sii_disconnection(void)
{
	MHL_SII_PAGE3_REG_WRITE(0x30, 0xD0);

	/* Force HPD = 0 */
	mhl_sii_hpd_control(FALSE);

	/* switch power state to D3 */
	mhl_sii_switch_power_state(POWER_STATE_D3);

	mhl_notify_offline(mhl_state->mhl_dev);
}

static void mhl_sii_force_usbidswitch_open(void)
{
	u8 regval;

	/* Disable discovery */
	MHL_SII_PAGE3_REG_WRITE(0x10, 0x26);

	/* Force USB ID switch to open */
	regval = MHL_SII_PAGE3_REG_READ(0x15);
	MHL_SII_PAGE3_REG_WRITE(0x15, regval | BIT(6));

	MHL_SII_PAGE3_REG_WRITE(0x12, 0x86);
}

static void mhl_sii_release_usbidswitch_open(void)
{
	u8 regval;

	msleep(50);

	regval = MHL_SII_PAGE3_REG_READ(0x15);
	regval &= ~(BIT(6));
	MHL_SII_PAGE3_REG_WRITE(0x15, regval);

	/* Enable discovery */
	regval = MHL_SII_PAGE3_REG_READ(0x10);
	MHL_SII_PAGE3_REG_WRITE(0x10, regval | BIT(0));
}

static void mhl_discovery_timer_work(struct work_struct *w)
{
	mutex_lock(&mhl_state_mutex);
	if (mhl_state->notify_plugged) {
		mhl_notify_unplugged(mhl_state->mhl_dev);
		wake_unlock(&mhl_wake_lock);
		if (mhl_state->charging_enable)
			mhl_state->charging_enable(FALSE, 0);
		del_timer(&mhl_state->discovery_timer);
		mhl_state->notify_plugged = FALSE;
	}
	mutex_unlock(&mhl_state_mutex);
}

static void mhl_discovery_timer(unsigned long data)
{
	schedule_work(&mhl_state->timer_work);
}

static void mhl_sii_int4_isr(void)
{
	u8 regval = MHL_SII_PAGE3_REG_READ(0x21);
	if (regval != 0xFF && regval != 0x00) {
		MHL_DEV_DBG("%s: %02x\n", __func__, regval);
		/* SCDT_CHANGE */
		if (regval & BIT(0))
			if (chip_rev_id < 1)
					mhl_sii_scdt_status_change();
		/* RPWR5V_CHANGE */
		if (regval & BIT(1))
			MHL_DEV_DBG("mhl: 5V status changed\n");
		/* MHL_EST */
		if (regval & BIT(2)) {
			/* MHL device detected */
			wake_lock(&mhl_wake_lock);
			mhl_sii_connection();
		}
		/* MHL_DISCOVERY_FAIL */
		if (regval & BIT(3)) {
			/* USB SLAVE device detected */
			mod_timer(&mhl_state->discovery_timer,
				jiffies + 4*HZ/10);
			/* re-enter power state to D3 */
			MHL_SII_PAGE3_REG_WRITE(0x21, regval);
			mhl_sii_switch_power_state
				(POWER_STATE_D3);
				return;
		}
		/* VBUS_LOW */
		if (regval & BIT(5)) {
			mod_timer(&mhl_state->discovery_timer,
				jiffies + 4*HZ/10);
			mutex_lock(&mhl_state_mutex);
			mhl_state->mhl_mode = FALSE;
			mutex_unlock(&mhl_state_mutex);
			MHL_SII_PAGE3_REG_WRITE(0x21, regval);
			mhl_sii_disconnection();
			wake_unlock(&mhl_wake_lock);
			return;
		}
		/* R_ID_DONE */
		if (regval & BIT(6)) {
			mutex_lock(&mhl_state_mutex);
			if (mhl_state->power_state == POWER_STATE_D3) {
				mutex_unlock(&mhl_state_mutex);
				mhl_sii_switch_power_state
					(POWER_STATE_D0_NO_MHL);
				if (mhl_sii_rgnd()
					== MHL_DISCOVERY_RESULT_USB) {
					/* Exit D3 via CBUS falling edge */;
					MHL_SII_PAGE3_REG_WRITE(0x1C, 0x80);
					/* USB SLAVE device detected */
					/* re-enter power state to D3 */
					MHL_SII_PAGE3_REG_WRITE(0x21, regval);
					mhl_sii_switch_power_state
						(POWER_STATE_D3);
					return;
				}
			} else
				mutex_unlock(&mhl_state_mutex);
		}
		/* CBUS_LKOUT */
		if (regval & BIT(4)) {
			mutex_lock(&mhl_state_mutex);
			if (mhl_state->power_state != POWER_STATE_D3) {
				mutex_unlock(&mhl_state_mutex);
				mhl_sii_force_usbidswitch_open();
				mhl_sii_release_usbidswitch_open();
			} else
				mutex_unlock(&mhl_state_mutex);
		}
	}
	/* clear all interrupts */
	MHL_SII_PAGE3_REG_WRITE(0x21, regval);
}

static void mhl_sii_int5_isr(void)
{
	u8 regval = MHL_SII_PAGE3_REG_READ(0x23);
	MHL_SII_PAGE3_REG_WRITE(0x23, regval);
}

static void mhl_sii_cbus_process_errors(u8 int_status)
{
	u8 abort_reason = 0;
	if (int_status & BIT(2)) {
		abort_reason = MHL_SII_CBUS_REG_READ(0x0B);
		MHL_DEV_DBG("%s: CBUS DDC Abort Reason(0x%02x)\n",
			__func__, abort_reason);
	}
	if (int_status & BIT(5)) {
		abort_reason = MHL_SII_CBUS_REG_READ(0x0D);
		MHL_DEV_DBG("%s: CBUS MSC Requestor Abort Reason(0x%02x)\n",
			__func__, abort_reason);
		MHL_SII_CBUS_REG_WRITE(0x0D, 0xFF);
	}
	if (int_status & BIT(6)) {
		abort_reason = MHL_SII_CBUS_REG_READ(0x0E);
		MHL_DEV_DBG("%s: CBUS MSC Responder Abort Reason(0x%02x)\n",
			__func__, abort_reason);
		MHL_SII_CBUS_REG_WRITE(0x0E, 0xFF);
	}
}

static void mhl_sii_cbus_isr(void)
{
	u8 regval;
	int req_done = FALSE;
	u8 sub_cmd;
	u8 cmd_data;
	int msc_msg_recved = FALSE;
	int rc;

	regval  = MHL_SII_CBUS_REG_READ(0x08);
	if (regval == 0xff)
		return;

	/* clear all interrupts that were raised even if we did not process */
	if (regval)
		MHL_SII_CBUS_REG_WRITE(0x08, regval);

	MHL_DEV_DBG("%s: CBUS_INT = %02x\n", __func__, regval);

	/* MSC_MSG (RCP/RAP) */
	if (regval & BIT(3)) {
		sub_cmd = MHL_SII_CBUS_REG_READ(0x18);
		cmd_data = MHL_SII_CBUS_REG_READ(0x19);
		msc_msg_recved = TRUE;
	}

	/* MSC_MT_ABRT/MSC_MR_ABRT/DDC_ABORT */
	if (regval & (BIT(6) | BIT(5) | BIT(2)))
		mhl_sii_cbus_process_errors(regval);

	/* MSC_REQ_DONE */
	if (regval & BIT(4))
		req_done = TRUE;

	/* Now look for interrupts on CBUS_MSC_INT2 */
	regval  = MHL_SII_CBUS_REG_READ(0x1E);

	/* clear all interrupts that were raised */
	/* even if we did not process */
	if (regval)
		MHL_SII_CBUS_REG_WRITE(0x1E, regval);

	MHL_DEV_DBG("%s: CBUS_MSC_INT2 = %02x\n", __func__, regval);

	/* received SET_INT */
	if (regval & BIT(2)) {
		u8 intr;

		intr = MHL_SII_CBUS_REG_READ(0xA0);
		MHL_SII_CBUS_REG_WRITE(0xA0, intr);
		MHL_DEV_DBG("%s: MHL_INT_0 = %02x\n", __func__, intr);
		mhl_msc_recv_set_int(mhl_state->mhl_dev, 0, intr);

		intr = MHL_SII_CBUS_REG_READ(0xA1);
		MHL_SII_CBUS_REG_WRITE(0xA1, intr);
		MHL_DEV_DBG("%s: MHL_INT_1 = %02x\n", __func__, intr);
		mhl_msc_recv_set_int(mhl_state->mhl_dev, 1, intr);
	}

	/* received WRITE_STAT */
	if (regval & BIT(3)) {
		u8 stat;

		stat = MHL_SII_CBUS_REG_READ(0xB0);
		MHL_DEV_DBG("%s: MHL_STATUS_0 = %02x\n", __func__, stat);
		if (stat ^ mhl_state->mhl_dev->state.device_status[0])
			mhl_msc_recv_write_stat(mhl_state->mhl_dev, 0, stat);

		stat = MHL_SII_CBUS_REG_READ(0xB1);
		MHL_DEV_DBG("%s: MHL_STATUS_1 = %02x\n", __func__, stat);
		if (stat ^ mhl_state->mhl_dev->state.device_status[1])
			mhl_msc_recv_write_stat(mhl_state->mhl_dev, 1, stat);

		MHL_SII_CBUS_REG_WRITE(0xB0, 0xFF);
		MHL_SII_CBUS_REG_WRITE(0xB1, 0xFF);
		MHL_SII_CBUS_REG_WRITE(0xB2, 0xFF);
		MHL_SII_CBUS_REG_WRITE(0xB3, 0xFF);
	}

	/* received MSC_MSG */
	if (msc_msg_recved) {
		rc = mhl_msc_recv_msc_msg(mhl_state->mhl_dev,
			sub_cmd, cmd_data);
		if (rc)
			pr_err("MHL: mhl_msc_recv_msc_msg failed(%d)!\n", rc);
	}

	/* complete last command */
	if (req_done)
		complete_all(&mhl_state->msc_command_done);
}


static irqreturn_t mhl_sii_isr(int irq, void *dev_id)
{
	mhl_sii_int4_isr();
	mutex_lock(&mhl_state_mutex);
	switch (mhl_state->power_state) {
	case POWER_STATE_D0_MHL:
		mutex_unlock(&mhl_state_mutex);
		mhl_sii_int5_isr();
		mhl_sii_cbus_isr();
		mhl_sii_int1_isr();
		break;
	case POWER_STATE_D0_NO_MHL:
		mutex_unlock(&mhl_state_mutex);
		break;
	case POWER_STATE_D3:
		mutex_unlock(&mhl_state_mutex);
		break;
	default:
		mutex_unlock(&mhl_state_mutex);
		pr_err("MHL: got interrupt in invalid state!\n");
		break;
	}

	return IRQ_HANDLED;
}

static int mhl_sii_charging_control(int enable, int max_curr)
{
	u8 regval;

	if (mhl_state->power_state != POWER_STATE_D0_MHL) {
		MHL_DEV_DBG("%s: power_state:%02x\n",
			__func__, mhl_state->power_state);
		return -EFAULT;
	}

	if (enable) {
		regval = MHL_SII_PAGE3_REG_READ(0x17);
		MHL_SII_PAGE3_REG_WRITE
			(0x17, regval | 0x04);
		if (mhl_state->charging_enable)
			mhl_state->charging_enable(TRUE, max_curr);
	} else {
		if (mhl_state->charging_enable)
			mhl_state->charging_enable(FALSE, max_curr);
	}
	return 0;
}

static int mhl_sii_hpd_control(int enable)
{
	u8 regval;

	if (mhl_state->power_state != POWER_STATE_D0_MHL) {
		MHL_DEV_DBG("%s: power_state:%02x\n",
			__func__, mhl_state->power_state);
		return -EFAULT;
	}

	if (enable) {
		mutex_lock(&mhl_hpd_tmds_mutex);
		/* disable HPD out override */
		regval = MHL_SII_PAGE3_REG_READ(0x20);
		regval &= ~BIT(4);
		MHL_SII_PAGE3_REG_WRITE(0x20, regval);
		mutex_unlock(&mhl_hpd_tmds_mutex);
		MHL_DEV_DBG("%s: enabled\n", __func__);

		/* check HPD status */
		regval = MHL_SII_CBUS_REG_READ(0x0D);
		if (regval & BIT(6))
			mhl_notify_hpd(mhl_state->mhl_dev, TRUE);
	} else {
		mutex_lock(&mhl_hpd_tmds_mutex);
		/* enable HPD out override */
		regval = MHL_SII_PAGE3_REG_READ(0x20);
		regval &= ~(BIT(4) | BIT(5));
		MHL_SII_PAGE3_REG_WRITE(0x20, regval | BIT(4));
		mutex_unlock(&mhl_hpd_tmds_mutex);
		MHL_DEV_DBG("%s: disabled\n", __func__);
		mhl_notify_hpd(mhl_state->mhl_dev, FALSE);
	}
	return 0;
}

static int mhl_sii_tmds_control(int enable)
{
	u8 regval;

	if (mhl_state->power_state != POWER_STATE_D0_MHL) {
		MHL_DEV_DBG("%s: power_state:%02x\n",
			__func__, mhl_state->power_state);
		return -EFAULT;
	}

	if (enable) {
		mutex_lock(&mhl_hpd_tmds_mutex);
		regval = MHL_SII_PAGE0_REG_READ(0x80);
		MHL_SII_PAGE0_REG_WRITE(0x80, regval | BIT(4));
		mutex_unlock(&mhl_hpd_tmds_mutex);
		MHL_DEV_DBG("%s: enabled\n", __func__);

		mhl_sii_hpd_control(TRUE);
	} else {
		mutex_lock(&mhl_hpd_tmds_mutex);
		regval = MHL_SII_PAGE0_REG_READ(0x80);
		regval &= ~BIT(4);
		MHL_SII_PAGE0_REG_WRITE(0x80, regval);
		mutex_unlock(&mhl_hpd_tmds_mutex);
		MHL_DEV_DBG("%s: disabled\n", __func__);
	}
	return 0;
}

static int mhl_sii_device_reset(int enable)
{
	if (!mhl_state->reset)
		return -EFAULT;

	if (enable) {
		gpio_set_value(mhl_state->reset, 0);
		usleep_range(10000, 100000);
		gpio_set_value(mhl_state->reset, 1);
	} else
		gpio_set_value(mhl_state->reset, 0);

	msleep(100);

	return 0;
}

static void mhl_sii_cbus_reset(void)
{
	int index = 0;
	u8 regval = MHL_SII_PAGE3_REG_READ(0x00);
	regval &= ~BIT(3);
	MHL_SII_PAGE3_REG_WRITE(0x00, regval | BIT(3));
	usleep_range(2000, 5000);
	MHL_SII_PAGE3_REG_WRITE(0x00, regval);

	/* unmask interrupts */
	/* INTR1_MASK */
	MHL_SII_PAGE0_REG_WRITE
		(0x75, (BIT(6) | BIT(5)));
	/* INTR4_MASK */
	MHL_SII_PAGE3_REG_WRITE
		(0x22, (BIT(6) | BIT(5) | BIT(4) | BIT(3) | BIT(2) | BIT(0)));
	/* INTR5_MASK */
	if (chip_rev_id < 1) {
		MHL_SII_PAGE3_REG_WRITE
			(0x24, (BIT(4) | BIT(3)));
	} else {
		MHL_SII_PAGE3_REG_WRITE(0x24, 0x00);
	}
	/* CBUS_INTR_ENABLE */
	MHL_SII_CBUS_REG_WRITE
		(0x09, (BIT(6) | BIT(5) | BIT(4) | BIT(3) | BIT(2)));
	/* CBUS_MSC_INT2_ENABLE */
	MHL_SII_CBUS_REG_WRITE
		(0x1F, (BIT(3) | BIT(2)));

	while (index < 4) {
		/* Enable WRITE_STAT interrupt for */
		/* writes to all 4 MSC Status registers. */
		MHL_SII_CBUS_REG_WRITE((0xE0 + index), 0xFF);
		/* Enable SET_INT interrupt for */
		/* writes to all 4 MSC Interrupt registers. */
		MHL_SII_CBUS_REG_WRITE((0xF0 + index), 0xFF);
		index++;
	}
}

static void mhl_sii_cbus_init(void)
{
	u8 regval;

	/* Increase DDC translation layer timer */
	MHL_SII_CBUS_REG_WRITE(0x07, 0xF2);
	/* Drive High Time */
	MHL_SII_CBUS_REG_WRITE(0x36, 0x0B);
	/* Use programmed timing */
	MHL_SII_CBUS_REG_WRITE(0x39, 0x30);
	/* CBUS Drive Strength */
	MHL_SII_CBUS_REG_WRITE(0x40, 0x03);

	/* Setup SiI8334 devcap */
	MHL_SII_CBUS_REG_WRITE(0x80, MHL_DEV_ACTIVE);
	MHL_SII_CBUS_REG_WRITE(0x81, MHL_VERSION);
	MHL_SII_CBUS_REG_WRITE(0x82, MHL_DEV_CAT_SOURCE);
	MHL_SII_CBUS_REG_WRITE(0x83, (mhl_state->adopter_id >> 8) & 0xFF);
	MHL_SII_CBUS_REG_WRITE(0x84, (mhl_state->adopter_id & 0xFF));
	MHL_SII_CBUS_REG_WRITE(0x85, (MHL_DEV_VID_LINK_SUPPRGB444 | \
				      MHL_DEV_VID_LINK_SUPP_ISLANDS));
	MHL_SII_CBUS_REG_WRITE(0x86, MHL_DEV_AUD_LINK_2CH);
	MHL_SII_CBUS_REG_WRITE(0x87, 0);
	MHL_SII_CBUS_REG_WRITE(0x88, MHL_DEV_LD_GUI);
	MHL_SII_CBUS_REG_WRITE(0x89, 0);
	MHL_SII_CBUS_REG_WRITE(0x8A, (MHL_FEATURE_RCP_SUPPORT | \
				      MHL_FEATURE_RAP_SUPPORT | \
				      MHL_FEATURE_SP_SUPPORT));
	MHL_SII_CBUS_REG_WRITE(0x8B, (mhl_state->device_id >> 8) & 0xFF);
	MHL_SII_CBUS_REG_WRITE(0x8C, (mhl_state->device_id & 0xFF));
	MHL_SII_CBUS_REG_WRITE(0x8D, MHL_SCRATCHPAD_SIZE);
	MHL_SII_CBUS_REG_WRITE(0x8E, MHL_INT_AND_STATUS_SIZE);
	MHL_SII_CBUS_REG_WRITE(0x8F, 0);

	/* Make bits 2,3 (initiator timeout) to 1,1 */
	/* for CBUS_LINK_CONTROL_2 */
	regval = MHL_SII_CBUS_REG_READ(0x31);
	MHL_SII_CBUS_REG_WRITE(0x31, (regval | 0x0C));

	/* Clear legacy bit on Wolverine TX. */
	MHL_SII_CBUS_REG_WRITE(0x22, 0x0F);

	/* Set NMax to 1 */
	MHL_SII_CBUS_REG_WRITE(0x30, 0x01);

	/* disallow vendor specific commands */
	regval = MHL_SII_CBUS_REG_READ(0x2E);
	MHL_SII_CBUS_REG_WRITE(0x2E, regval | BIT(4));
}

static void mhl_sii_chip_init(void)
{
	u8 regval;

	/* Power up CVCC 1.2V core */
	MHL_SII_PAGE1_REG_WRITE(0x3D, 0x3F);
	/* Enable TxPLL Clock */
	MHL_SII_PAGE2_REG_WRITE(0x11, 0x01);
	/* Enable Tx Clock Path & Equalizer */
	MHL_SII_PAGE2_REG_WRITE(0x12, 0x11);

	/* TX Source termination ON */
	MHL_SII_PAGE3_REG_WRITE(0x30, 0x10);
	/* Enable 1X MHL clock output */
	MHL_SII_PAGE3_REG_WRITE(0x35, 0xBC);
	/* TX Differential Driver Config */
	MHL_SII_PAGE3_REG_WRITE(0x31, 0x3C);
	MHL_SII_PAGE3_REG_WRITE(0x33, 0xC8);

	MHL_SII_PAGE3_REG_WRITE(0x36, 0x03);
	/* PLL bias current, PLL BW Control */
	MHL_SII_PAGE3_REG_WRITE(0x37, 0x0A);

	/* Analog PLL Control */
	/* Enable Rx PLL clock */
	MHL_SII_PAGE0_REG_WRITE(0x80, 0x08);
	/* USB charge pump clock */
	MHL_SII_PAGE0_REG_WRITE(0xF8, 0x8C);
	MHL_SII_PAGE0_REG_WRITE(0x85, 0x02);

	MHL_SII_PAGE2_REG_WRITE(0x00, 0x00);
	regval = MHL_SII_PAGE2_REG_READ(0x05);
	regval &= ~BIT(5);
	MHL_SII_PAGE2_REG_WRITE(0x05, regval);
	MHL_SII_PAGE2_REG_WRITE(0x13, 0x60);

	/* PLL Calrefsel */
	MHL_SII_PAGE2_REG_WRITE(0x17, 0x03);
	/* VCO Cal */
	MHL_SII_PAGE2_REG_WRITE(0x1A, 0x20);
	MHL_SII_PAGE2_REG_WRITE(0x22, 0xE0);
	MHL_SII_PAGE2_REG_WRITE(0x23, 0xC0);
	MHL_SII_PAGE2_REG_WRITE(0x24, 0xA0);
	MHL_SII_PAGE2_REG_WRITE(0x25, 0x80);
	MHL_SII_PAGE2_REG_WRITE(0x26, 0x60);
	MHL_SII_PAGE2_REG_WRITE(0x27, 0x40);
	MHL_SII_PAGE2_REG_WRITE(0x28, 0x20);
	MHL_SII_PAGE2_REG_WRITE(0x29, 0x00);

	/* Rx PLL BW ~ 4MHz */
	MHL_SII_PAGE2_REG_WRITE(0x31, 0x0A);
	/* Rx PLL BW value from I2C */
	MHL_SII_PAGE2_REG_WRITE(0x45, 0x06);
	MHL_SII_PAGE2_REG_WRITE(0x4B, 0x06);
	/* Manual zone control */
	MHL_SII_PAGE2_REG_WRITE(0x4C, 0x60);
	/* Manual zone control */
	MHL_SII_PAGE2_REG_WRITE(0x4C, 0xE0);
	/* PLL Mode Value */
	MHL_SII_PAGE2_REG_WRITE(0x4D, 0x00);

	/* bring out from power down (script moved this here from above) */
	MHL_SII_PAGE0_REG_WRITE(0x08, 0x35);

	MHL_SII_PAGE3_REG_WRITE(0x11, 0xAD);
	/* 1.8V CBUS VTH 5K pullup for MHL state */
	MHL_SII_PAGE3_REG_WRITE(0x14, 0x57);
	/* RGND & single discovery attempt (RGND blocking) */
	MHL_SII_PAGE3_REG_WRITE(0x15, 0x11);
	/* Ignore VBUS */
	MHL_SII_PAGE3_REG_WRITE(0x17, 0x82);
#ifndef CONFIG_MHL_SII8334_WAKE_PLUSE_DISABLE
	/* No OTG, Discovery pulse proceed, Wake pulse not bypassed */
	MHL_SII_PAGE3_REG_WRITE(0x18, 0x24);
#else
	/* No OTG, Discovery pulse proceed, Wake pulse bypassed */
	MHL_SII_PAGE3_REG_WRITE(0x18, 0x26);
#endif
	/* Pull-up resistance off for IDLE state and 10K for discovery state. */
	MHL_SII_PAGE3_REG_WRITE(0x13, 0x8C);
	/* Enable CBUS discovery */
	MHL_SII_PAGE3_REG_WRITE(0x10, 0x27);
	/* use 1K only setting */
	MHL_SII_PAGE3_REG_WRITE(0x16, 0x20);
	/* MHL CBUS discovery */
	MHL_SII_PAGE3_REG_WRITE(0x12, 0x86);

	/* setup int signal mode */
	regval = MHL_SII_PAGE3_REG_READ(0x20);
	regval &= ~(BIT(6) | BIT(1) | BIT(2));
	mutex_lock(&mhl_state_mutex);
	if (mhl_state->hpd_pin_mode & 0xF0)
		regval |= BIT(6);
	if (mhl_state->int_pin_mode & 0xF0)
		regval |= BIT(2);
	if (mhl_state->int_pin_mode & 0x0F)
		regval |= BIT(1);
	mutex_unlock(&mhl_state_mutex);
	MHL_SII_PAGE3_REG_WRITE(0x20, regval);
	MHL_DEV_DBG("%s: configured HPD mode as %s\n",
		__func__,
		(regval & BIT(6)) ? "opendrain" : "push-pull");
	MHL_DEV_DBG("%s: configured INT mode as %s, active %s\n",
		__func__,
		(regval & BIT(2)) ? "opendrain" : "push-pull",
		(regval & BIT(1)) ? "low" : "hi");

	/* Force HPD = 0 when not in MHL mode. */
	mutex_lock(&mhl_state_mutex);
	if (mhl_state->power_state != POWER_STATE_D0_MHL) {
		mutex_unlock(&mhl_state_mutex);
		mhl_sii_hpd_control(FALSE);
	} else
		mutex_unlock(&mhl_state_mutex);

	/* Enable Auto soft reset on SCDT = 0 */
	MHL_SII_PAGE3_REG_WRITE(0x00, 0x84);

	/* HDMI Transcode mode enable */
	MHL_SII_PAGE0_REG_WRITE(0x0D, 0x1C);

	mhl_sii_cbus_reset();

	mhl_sii_cbus_init();

}

static void mhl_sii_device_init(void)
{
	mhl_sii_device_reset(TRUE);

	chip_rev_id = MHL_SII_PAGE0_REG_READ(0x04);

	pr_info("%s: %02x%02x (%02x)\n",
		__func__,
		MHL_SII_PAGE0_REG_READ(0x03),
		MHL_SII_PAGE0_REG_READ(0x02),
		chip_rev_id);

	mhl_sii_chip_init();

	/* switch power state to D3 */
	mhl_sii_switch_power_state(POWER_STATE_D3);
}

static int mhl_sii_discovery_result_get(int *result)
{
	int timeout;
	MHL_DEV_DBG("%s: start\n", __func__);
	if (mhl_state->power_state != POWER_STATE_D0_MHL) {
		/* give MHL driver chance to handle RGND interrupt */
		init_completion(&mhl_state->rgnd_done);
		timeout = wait_for_completion_interruptible_timeout
			(&mhl_state->rgnd_done, HZ/2);
		if (!timeout) {
			MHL_DEV_DBG("%s: RGND timed out!\n", __func__);
			/* most likely nothing plugged in USB connector, */
			/* USB HOST connected or already in USB mode */
			*result = MHL_DISCOVERY_RESULT_USB;
			return 0;
		}
		*result = mhl_state->mhl_mode ?
			MHL_DISCOVERY_RESULT_MHL : MHL_DISCOVERY_RESULT_USB;
	} else {
		/* in POWER_STATE_D0_MHL. already in MHL mode */
		*result = MHL_DISCOVERY_RESULT_MHL;
	}
	MHL_DEV_DBG("%s: done\n", __func__);
	return 0;
}

#ifdef CONFIG_MHL_OSD_NAME
static int mhl_sii_scratchpad_data_get(void)
{
	unsigned char burst_data[MHL_SCRATCHPAD_SIZE];
	int i;
	MHL_DEV_DBG("%s: start\n", __func__);
	for (i = 0; i < MHL_SCRATCHPAD_SIZE; i++)
		burst_data[i] = MHL_SII_CBUS_REG_READ(0xC0 + i);
	mhl_notify_scpd_recv(mhl_state->mhl_dev, &burst_data[0]);
	MHL_DEV_DBG("%s: done\n", __func__);
	return 0;
}
#endif /* CONFIG_MHL_OSD_NAME */

const struct mhl_ops mhl_sii_ops = {
	.discovery_result_get = mhl_sii_discovery_result_get,
	.send_msc_command = mhl_sii_send_msc_command,
	.charging_control = mhl_sii_charging_control,
	.hpd_control = mhl_sii_hpd_control,
	.tmds_control = mhl_sii_tmds_control,
#ifdef CONFIG_MHL_OSD_NAME
	.scratchpad_data_get = mhl_sii_scratchpad_data_get,
#endif /* CONFIG_MHL_OSD_NAME */
};

static int mhl_sii_probe(struct i2c_client *client,
				const struct i2c_device_id *id)
{
	int rc = 0;
	struct mhl_sii_platform_data *pdata;

	pdata = client->dev.platform_data;
	if (!pdata) {
		pr_err("%s: invalid platform_data\n", __func__);
		return -EFAULT;
	}

	mhl_state->irq = client->irq;
	mhl_state->reset = pdata->reset;
	mhl_state->hpd_pin_mode = pdata->hpd_pin_mode;
	mhl_state->int_pin_mode = pdata->int_pin_mode;

	mhl_state->adopter_id = pdata->adopter_id;
	mhl_state->device_id = pdata->device_id;

	mhl_state->client = *client;

	mhl_state->power_state = POWER_STATE_FIRST_INIT;

	init_completion(&mhl_state->rgnd_done);
	INIT_WORK(&mhl_state->timer_work, mhl_discovery_timer_work);

	if (pdata->setup_power) {
		rc = pdata->setup_power(TRUE);
		if (rc) {
			pr_err("%s: setup power failed\n", __func__);
			goto probe_out;
		}
	} else
		pr_warn("%s: setup_power not supported\n", __func__);

	if (pdata->setup_low_power_mode)
		mhl_state->low_power_mode = pdata->setup_low_power_mode;
	else
		pr_warn("%s: MHL Low Power Mode not supported\n", __func__);

	if (pdata->setup_gpio) {
		rc = pdata->setup_gpio(TRUE);
		if (rc) {
			pr_err("%s: setup gpio failed\n", __func__);
			if (pdata->setup_power)
				pdata->setup_power(FALSE);
			goto probe_out;
		}
	} else
		pr_warn("%s: setup_gpio not supported\n", __func__);

	if (pdata->charging_enable)
		mhl_state->charging_enable = pdata->charging_enable;
	else
		pr_warn("%s: MHL charging not supported\n", __func__);

	/* register mhl device */
	mhl_state->mhl_dev = mhl_device_register(SII_DEV_NAME,
		&client->dev, NULL, &mhl_sii_ops);
	if (IS_ERR(mhl_state->mhl_dev)) {
		pr_err("%s: mhl_device_register failed!\n", __func__);
		goto probe_out;
	}

	mhl_sii_device_init();

	rc = request_threaded_irq(mhl_state->irq, NULL, &mhl_sii_isr,
		IRQF_TRIGGER_LOW | IRQF_ONESHOT, "mhl_sii_isr", NULL);
	if (rc) {
		pr_err("%s: request irq failed\n", __func__);
		if (pdata->setup_gpio)
			pdata->setup_gpio(FALSE);
		if (pdata->setup_power)
			pdata->setup_power(FALSE);
		goto probe_out;
	}

	init_timer(&mhl_state->discovery_timer);
	mhl_state->discovery_timer.function =
		mhl_discovery_timer;
	mhl_state->discovery_timer.data = (unsigned long)NULL;
	mhl_state->discovery_timer.expires = 0xffffffffL;
	add_timer(&mhl_state->discovery_timer);

	return 0;

probe_out:
	return rc;
}

static int mhl_sii_remove(struct i2c_client *client)
{
	struct mhl_sii_platform_data *pdata;

	pdata = client->dev.platform_data;
	if (!pdata) {
		pr_err("%s: invalid platform_data\n", __func__);
		return -EFAULT;
	}

	disable_irq(mhl_state->irq);
	free_irq(mhl_state->irq, NULL);

	mhl_sii_device_reset(FALSE);

	if (pdata->setup_power)
		pdata->setup_power(FALSE);

	if (pdata->setup_gpio)
		pdata->setup_gpio(FALSE);

	/* unregister mhl device */
	mhl_device_unregister(mhl_state->mhl_dev);

	return 0;
}

static void mhl_sii_shutdown(struct i2c_client *client)
{
	mhl_device_shutdown(mhl_state->mhl_dev);
}

#ifdef CONFIG_PM
static int mhl_sii_i2c_suspend(struct device *dev)
{
	flush_work_sync(&mhl_state->timer_work);

	/* enable_irq_wake to setup this irq for wakeup trigger */
	enable_irq_wake(mhl_state->irq);

	/* this is needed isr not to be executed before i2c resume */
	disable_irq(mhl_state->irq);

	/* sii8334 power shoule be keep enbaled and sii8334 shoule be in D3 */
	/* and if low_power_mode operation exist, call it for some reason. */
	if (mhl_state->low_power_mode)
		mhl_state->low_power_mode(1);

	return 0;
}

static int mhl_sii_i2c_resume(struct device *dev)
{
	/* exit from low_power_mode */
	if (mhl_state->low_power_mode)
		mhl_state->low_power_mode(0);

	disable_irq_wake(mhl_state->irq);

	enable_irq(mhl_state->irq);

	return 0;
}

SIMPLE_DEV_PM_OPS(mhl_sii_pm_ops, mhl_sii_i2c_suspend, mhl_sii_i2c_resume);
#endif /* CONFIG_PM */

static const struct i2c_device_id mhl_sii_id[] = {
	{ SII_DEV_NAME, 0 },
	{ }
};

static struct i2c_driver mhl_sii_i2c_driver = {
	.driver = {
		.name = SII_DEV_NAME,
		.owner = THIS_MODULE,
#ifdef CONFIG_PM
		.pm = &mhl_sii_pm_ops,
#endif /* CONFIG_PM */
	},
	.probe = mhl_sii_probe,
	.remove = mhl_sii_remove,
	.shutdown = mhl_sii_shutdown,
	.id_table = mhl_sii_id,
};

static int __init mhl_sii_init(void)
{
	int rc;

	mhl_state = kzalloc(sizeof(*mhl_state), GFP_KERNEL);
	if (!mhl_state) {
		pr_err("%s: out of memory!\n", __func__);
		return -ENOMEM;
	}

	rc = i2c_add_driver(&mhl_sii_i2c_driver);
	if (rc) {
		pr_err("%s: i2c_add_driver failed (%d)\n",
			__func__, rc);
		kfree(mhl_state);
		goto init_out;
	}

	wake_lock_init(&mhl_wake_lock, WAKE_LOCK_SUSPEND, "mhl_wake_lock");

	return 0;

init_out:
	return rc;
}

static void __exit mhl_sii_exit(void)
{
	wake_lock_destroy(&mhl_wake_lock);
	i2c_del_driver(&mhl_sii_i2c_driver);
	kfree(mhl_state);
}


/*
 * We have to use late_initcall instead of module_init.
 * Because, when we use module_init, the issue occurred
 * that cannot charge a phone from dongle or MHL straight cable
 * when a phone is a power off state.
 * This reason is cause that reading DEV_CAT of mhl_msc_command_done()
 * of the MHL driver is run before calling pm8921_charger_probe().
 * To solve this issue, It is necessary for us to call late_initcall
 * which pm8921_charger driver called.
 */
late_initcall(mhl_sii_init);
module_exit(mhl_sii_exit);

MODULE_LICENSE("GPL v2");
MODULE_VERSION("1.0");
MODULE_AUTHOR("Sony Ericsson Mobile Communications AB");
MODULE_DESCRIPTION("SiI8334 MHL Transmitter Driver");
