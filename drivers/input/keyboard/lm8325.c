/*******************************************************************************
* Copyright 2011 Broadcom Corporation.  All rights reserved.
*
* 	@file	 drivers/input/keyboard/lm8325.c
*
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

/**
 *@file
 *driver/input/keyboard/lm8325.c
 *
 * LM8325 Keypad Controller - A dedicated device to unburden a host processor
 *from scanning a matrix-addressed keypad and to provide flexible and general
 *purpose, host programmable input/output functions.
 *
 * Version 2
 */

#include <linux/module.h>
#include <linux/input.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/slab.h>
#include <linux/i2c/lm8325.h>

/* Enable this macro to use soft reset feature. Currently unused */
/* #define LM8325_SOFT_RESET */

/* Enable this flag to see the extra debug messages */
/* #define DEBUG_ENABLE */

/**
 *lm8325_write - I2C host write functionality for lm8325
 *
 * Function used to write a value into a given lm8325 controller register.
 * This takes care of the condition when the controller is in HALT mode by
 *calling the write API again on a fail.
 *
 *@param lm8325_chip *lm
 *  The lm8325 chip data.
 *@param len
 *  Length of the data to be written.
 *@param *data
 *  8-bit buffer from which the data is taken.
 *
 *@return 0 on wrong length param passed or negative error value on error
 *or the length of data read on success
 */
static int lm8325_write(struct lm8325_chip *lm, int len, u8 *data)
{
	int ret;
	if (len > LM8325_MAX_DATA) {
		dev_err(&lm->client->dev, "len greater than supported\n");
		return 0;
	}

	ret = i2c_master_send(lm->client, data, len);

	if (ret == -EREMOTEIO)
		ret = i2c_master_send(lm->client, data, len);

	if (ret != len) {
		dev_err(&lm->client->dev, "i2c send failed\n");
	}

	return ret;
}

/**
 *lm8325_read - I2C host read functionality for lm8325
 *
 * Function used to read a 8-bit value from a given lm8325 controller register.
 * This takes care of the condition when the controller is in HALT mode by
 *calling the read API again on a fail.
 *
 *@param lm8325_chip *lm
 *  The lm8325 chip data.
 *@param cmd
 *  8-bit physical address of thr controller register.
 *@param *buf
 *  8-bit buffer to which the data is read.
 *@param len
 *  length of the data to be read
 *
 *@return 0 if the controller does not resume from HALT mode or the length of
 *the data read on success.
 */
static int lm8325_read(struct lm8325_chip *lm, u8 cmd, u8 *buf, int len)
{
	int ret;

	/*
	 * If the host is asleep while we send the byte, we can get a NACK
	 *back while it wakes up, so try again, once.
	 */
	ret = i2c_master_send(lm->client, &cmd, 1);
	if (ret == -EREMOTEIO)
		ret = i2c_master_send(lm->client, &cmd, 1);
	if (ret != 1) {
		dev_err(&lm->client->dev, "sending read cmd 0x%02x failed\n",
			cmd);
		return 0;
	}

	ret = i2c_master_recv(lm->client, buf, len);
	if (ret != len)
		dev_err(&lm->client->dev, "wanted %d bytes, got %d\n",
			len, ret);

	return ret;
}

#ifdef DEBUG_ENABLE
/**
 *lm8325_reg_dump - Dump the values of all the registers on lm8325
 *
 * Function used to read all the registers on lm8325 controller and dump them
 *for debugging purposes only
 *
 *@param lm8325_chip *lm
 *  The lm8325 chip data.
 */
static void lm8325_reg_dump(struct lm8325_chip *lm)
{
	u8 temp[24];

	lm8325_read(lm, LM8325_CMD_MFGCODE, &temp[1], 1);
	lm8325_read(lm, LM8325_CMD_SWREV, &temp[2], 1);
	lm8325_read(lm, LM8325_CMD_RSTCTRL, &temp[3], 1);
	lm8325_read(lm, LM8325_CMD_CLKMODE, &temp[4], 1);
	lm8325_read(lm, LM8325_CMD_CLKCFG, &temp[5], 1);
	lm8325_read(lm, LM8325_CMD_CLKEN, &temp[6], 1);
	lm8325_read(lm, LM8325_CMD_AUTOSLP, &temp[7], 1);
	lm8325_read(lm, LM8325_CMD_AUTOSLPTIL, &temp[8], 1);
	lm8325_read(lm, LM8325_CMD_AUTOSLPTIH, &temp[22], 1);
	lm8325_read(lm, LM8325_CMD_IRQST, &temp[9], 1);
	lm8325_read(lm, LM8325_CMD_KBDSETTLE, &temp[10], 1);
	lm8325_read(lm, LM8325_CMD_KBDBOUNCE, &temp[11], 1);
	lm8325_read(lm, LM8325_CMD_KBDSIZE, &temp[12], 1);
	lm8325_read(lm, LM8325_CMD_KBDDEDCFG0, &temp[13], 1);
	lm8325_read(lm, LM8325_CMD_KBDDEDCFG1, &temp[14], 1);
	lm8325_read(lm, LM8325_CMD_KBDRIS, &temp[15], 1);
	lm8325_read(lm, LM8325_CMD_KBDMIS, &temp[16], 1);
	lm8325_read(lm, LM8325_CMD_KBDMSK, &temp[17], 1);
	lm8325_read(lm, LM8325_CMD_KBDCODE0, &temp[18], 1);
	lm8325_read(lm, LM8325_CMD_KBDCODE1, &temp[19], 1);
	lm8325_read(lm, LM8325_CMD_KBDCODE2, &temp[20], 1);
	lm8325_read(lm, LM8325_CMD_KBDCODE3, &temp[21], 1);
	lm8325_read(lm, LM8325_CMD_EVTCODE, &temp[0], 1);

	printk("\n\t\t\t\t LM8325_SYSTEM_REG \n");
	printk("LM8325_CMD_MFGCODE : 0x%x\t\t|", temp[1]);
	printk("\t\tLM8325_CMD_SWREV : 0x%x\n", temp[2]);
	printk("LM8325_CMD_RSTCTRL : 0x%x\t\t|", temp[3]);
	printk("\t\tLM8325_CMD_CLKMODE : 0x%x\n", temp[4]);
	printk("LM8325_CMD_CLKCFG : 0x%x\t\t|", temp[5]);
	printk("\t\tLM8325_CMD_CLKEN : 0x%x\n", temp[6]);
	printk("LM8325_CMD_AUTOSLP : 0x%x\t\t|", temp[7]);
	printk("\t\tLM8325_CMD_AUTOSLPTIL : 0x%x\n", temp[8]);
	printk("\n\t\t\t\t LM8325_GLOBAL_INTERRUPT_REG \n");
	printk("LM8325_CMD_IRQST : 0x%x\t\t\t|\t\t", temp[9]);
	printk("LM8325_CMD_AUTOSLPTIH : 0x%x\n", temp[22]);
	printk("\n\t\t\t\t LM8325_KEYBOARD_FUNC_REG \n");
	printk("LM8325_CMD_KBDSETTLE : 0x%x\t\t|", temp[10]);
	printk("\t\tLM8325_CMD_KBDBOUNCE : 0x%x\n", temp[11]);
	printk("LM8325_CMD_KBDSIZE : 0x%x\t\t|", temp[12]);
	printk("\t\tLM8325_CMD_KBDDEDCFG0 : 0x%x\n", temp[13]);
	printk("LM8325_CMD_KBDDEDCFG1 : 0x%x\t\t|", temp[14]);
	printk("\t\tLM8325_CMD_KBDRIS : 0x%x\n", temp[15]);
	printk("LM8325_CMD_KBDMIS : 0x%x\t\t\t|", temp[16]);
	printk("\t\tLM8325_CMD_KBDMSK : 0x%x\n", temp[17]);
	printk("LM8325_CMD_KBDCODE0 : 0x%x\t\t|", temp[18]);
	printk("\t\tLM8325_CMD_KBDCODE1 : 0x%x\n", temp[19]);
	printk("LM8325_CMD_KBDCODE2 : 0x%x\t\t|", temp[20]);
	printk("\t\tLM8325_CMD_KBDCODE3 : 0x%x\n", temp[21]);
	printk("LM8325_CMD_EVTCODE : 0x%x\n", temp[0]);
}
#endif

/**
 *lm8325_soft_reset - LM8325 soft reset sequence
 *
 * Function used to reset all the registers oon the LM8325 controller. This
 *mainly invovles reading the value from the SWREV regsiter and writing back
 *the invert of the read value.
 *
 *@param lm8325_chip *lm
 *  The lm8325 chip data.
 */
/* TODO : This functionality is not working as defined in the controller
 *datasheet. Need to check its functionality by further testing. The
 *controller registers can not be written too once it has been reset thus.
 */
#ifdef LM8325_SOFT_RESET
static void lm8325_soft_reset(struct lm8325_chip *lm)
{
	u8 buf[LM8325_MAX_DATA];
	u8 temp;

	/* Read the contents of SWREV reg */
	lm8325_read(lm, LM8325_CMD_SWREV, &temp, 1);

	/* Invert the read value */
	temp = ~(temp & 0xFF);

	/* Write back the inverted read value to reset */
	buf[0] = LM8325_CMD_SWREV;
	buf[1] = temp;
	lm8325_write(lm, 2, buf);

}
#endif

#if 0
/* This part of the code has been implemented from the RTOS driver. This
 *sequence is supposed to clear the EVTCODE and KDBCODE buffers and needs to
 *be called as a part of the initial configuration code */
static void clean_buffer_overflow(struct lm8325_chip *lm)
{
	u8 temp;
	lm8325_read(lm, LM8325_CMD_KBDCODE0, &temp, 1);
	lm8325_read(lm, LM8325_CMD_EVTCODE, &temp, 1);
	lm8325_read(lm, LM8325_CMD_KBDCODE1, &temp, 1);
	lm8325_read(lm, LM8325_CMD_EVTCODE, &temp, 1);
	lm8325_read(lm, LM8325_CMD_KBDCODE2, &temp, 1);
	lm8325_read(lm, LM8325_CMD_EVTCODE, &temp, 1);
	lm8325_read(lm, LM8325_CMD_KBDCODE3, &temp, 1);
	lm8325_read(lm, LM8325_CMD_EVTCODE, &temp, 1);
	lm8325_read(lm, LM8325_CMD_EVTCODE, &temp, 1);
	lm8325_read(lm, LM8325_CMD_EVTCODE, &temp, 1);
	lm8325_read(lm, LM8325_CMD_EVTCODE, &temp, 1);
	lm8325_read(lm, LM8325_CMD_EVTCODE, &temp, 1);
}
#endif

/**
 *lm8325_configure - Set the initial values for the LM8325 controller
 *configuration
 *
 * Function used to configure the LM8325 controller for the required
 *functionality. Most of the parameters are passed as a part of the board
 *specific platform data.
 *
 *@param lm8325_chip *lm
 *  The lm8325 chip data.
 *
 *@return 0 on success and -1 on failure of any of the write commands.
 */
static int lm8325_configure(struct lm8325_chip *lm)
{
	u8 buf[LM8325_MAX_DATA];
	u8 temp;
	int keysize = (lm->size_x << 4) | lm->size_y;

	/* Clear interrupts */
	lm8325_read(lm, LM8325_CMD_IRQST, &temp, 1);
	if (temp & 0x80) {	/* PORIRQ is set - need to power-on */
		dev_dbg(&lm->client->dev, "PORIRQ set, Resetting..\n");
		buf[0] = LM8325_CMD_RSTINTCLR;
		buf[1] = 0x01;
		lm8325_write(lm, 2, buf);
	}

	lm8325_read(lm, LM8325_CMD_MFGCODE, &temp, 1);
	if (temp == 0x00)
		dev_info(&lm->client->dev,
			 "%s : Found the right lm8325 chip with id = %d\n",
			 __func__, temp);

	/* Enable the key scan clock */
	buf[0] = LM8325_CMD_CLKEN;
	buf[1] = 0x01;
	lm8325_write(lm, 2, buf);

	/* Enable the key scan settle time */
	buf[0] = LM8325_CMD_KBDSETTLE;
	buf[1] = lm->settle_time;
	lm8325_write(lm, 2, buf);

	/* Enable the key scan debounce time */
	buf[0] = LM8325_CMD_KBDBOUNCE;
	buf[1] = lm->debounce_time;
	lm8325_write(lm, 2, buf);

	/* Set the key scan matrix size */
	buf[0] = LM8325_CMD_KBDSIZE;
	buf[1] = keysize;
	lm8325_write(lm, 2, buf);

	/* Configure Dedicated Keys */
	buf[0] = LM8325_CMD_KBDDEDCFG0;
	buf[1] = 0x3F;
	lm8325_write(lm, 2, buf);

	buf[0] = LM8325_CMD_KBDDEDCFG1;
	buf[1] = 0xFC;
	lm8325_write(lm, 2, buf);

	/* Enable the required ball configuration */
	buf[0] = LM8325_CMD_IOCFG;
	buf[1] = lm->iocfg;
	lm8325_write(lm, 2, buf);

	/* Enable Pull-ups of matrix keys & dedicated keys */
	buf[0] = LM8325_CMD_IOPC0;
	buf[1] = 0xAA;
	lm8325_write(lm, 2, buf);

	buf[0] = LM8325_CMD_IOPC0H;
	buf[1] = 0xAA;
	lm8325_write(lm, 2, buf);

	buf[0] = LM8325_CMD_IOPC1;
	buf[1] = 0x55;
	lm8325_write(lm, 2, buf);

	buf[0] = LM8325_CMD_IOPC1H;
	buf[1] = 0x55;
	lm8325_write(lm, 2, buf);

	/* Clear any pending interrupts */
	buf[0] = LM8325_CMD_KBDIC;
	buf[1] = 0x83;
	lm8325_write(lm, 2, buf);

	/* Mask the un-required interupts */
	buf[0] = LM8325_CMD_KBDMSK;
	buf[1] = 0x3;
	lm8325_write(lm, 2, buf);

	/* Set autosleep time */
	/* Set the lower 8 bits */
	buf[0] = LM8325_CMD_AUTOSLPTIL;
	buf[1] = ((lm->sleep_time) & 0xFF);
	lm8325_write(lm, 2, buf);

	/* Set the higher 8 bits */
	buf[0] = LM8325_CMD_AUTOSLPTIH;
	buf[1] = ((lm->sleep_time) & 0xFF00) >> 8;
	lm8325_write(lm, 2, buf);

	/* Enable autosleep */
	buf[0] = LM8325_CMD_AUTOSLP;
	buf[1] = lm->autosleep;
	lm8325_write(lm, 2, buf);

	/*clean_buffer_overflow(lm); - This call is not really necessary as i
	 *find no need for this at this instance of time */

	return 0;
}

/**
 *lm8325_process_key - Process the key pressed or released.
 *
 *
 * Function where the actual processing of the key press/release is done. The
 *row and column of the key pressed are computed and then the corresponding
 *key code is reported to the input subsystem.
 *
 *@param lm8325_chip *lm
 *  The lm8325 chip data.
 *
 *@return 0 on success and -1 on failure.
 */
static int lm8325_process_key(struct lm8325_chip *lm)
{
	int row, col, key_code, code_count, event_count, event, press_release;
	u8 key_code_list[4], event_code_list[15];

	if (!lm->kp_enabled) {
		dev_err(&lm->client->dev,
			"%s: LM8325 keypad disabled.. Kindly enable it to use it\n",
			__func__);
		return -1;
	}

	/* Reading the KBDCODE as long as there is key press info should clear
	 *the RSINT interrupt. Scan upto 4 keys is supported. */
	for (code_count = 0; code_count < 4; code_count++) {
		/* Read the appropriate KBDCODE. */
		lm8325_read(lm, (LM8325_CMD_KBDCODE0 + code_count),
			    &key_code_list[code_count], 1);
		/* If multi-key bit is not set, break. */
		if ((key_code_list[code_count] & 0x80) != 0x80)
			break;
		/* If the content read is 0x7F, no need to read further break. */
		if ((key_code_list[code_count] & 0x7F) == 0x7F)
			break;
	}

	/* Read the EVT Code until you read 0x7f, this clears REVTINT interrupt. */
	do {
		lm8325_read(lm, LM8325_CMD_EVTCODE,
			    &event_code_list[event_count], 1);
		if (event_code_list[event_count] == 0x7f)
			break;
		++event_count;
	} while (1);

	for (event = 0; event < event_count; event++) {
		row = event_code_list[event] & 0x70;
		col = event_code_list[event] & 0xf;
		key_code = row | col;
		/* If Bit 7 is ON then the key is released otherwise the key
		 *is pressed. */
		if (event_code_list[event] & 0x80)
			press_release = 0;
		else
			press_release = 1;

		input_report_key(lm->idev, lm->keymap[key_code], press_release);
		input_sync(lm->idev);
	}

	return 0;
}

/**
 *lm8325_work - Work function to handle the keyboard interrupts
 *
 *@param lm8325_chip *lm
 *  The lm8325 chip data.
 *
 *@return void.
 */
static void lm8325_work(struct work_struct *work)
{
	struct lm8325_chip *lm = container_of(work, struct lm8325_chip, work);
	u8 ints = 0, ret = 0;
	u8 buf[2];

	mutex_lock(&lm->lock);

	/* Read the interrupt status register and check if the interrupt
	 *issued was for the key press*/
	lm8325_read(lm, LM8325_CMD_IRQST, &ints, 1);

	if (ints & 0x40) {
		/* Handle the key-press interrupt */
		/* Check if the interrupt was for a key-press/release */
		lm8325_read(lm, LM8325_CMD_KBDMIS, &ints, 1);
		if (ints & 0x04) {
			/* MEVINT is set. Hence the interrupt was for a
			 *key-press/release */

			/* Process the data */
			ret = lm8325_process_key(lm);

			/* Error encountered if more than 4 keys are pressed */
			if (ret == -1) {
				dev_err(&lm->client->dev,
					"%s: Error encountered while processing the key pressed\n",
					__func__);
				/* clear any interrupts set */
				buf[0] = LM8325_CMD_KBDIC;
				buf[1] = 0x83;
				lm8325_write(lm, 2, buf);

				/* Mask the required interrupts */
				buf[0] = LM8325_CMD_KBDMSK;
				buf[1] = 0x3;
				lm8325_write(lm, 2, buf);
			}
		} else {
			/* Check the bits set in the KBDMIS register and handle the
			 *interrupt issued */
			if (ints & 0x02) {
				/* MKLINT set - Indicates a lost key-code */

				dev_err(&lm->client->dev,
					"%s: MKLINT set. Lost key code interrupt. Handled by clearing all pending interrupts\n",
					__func__);

				/* Clear the pending interrupts to get back to
				 *the right state */
				buf[0] = LM8325_CMD_KBDIC;
				buf[1] = 0x81;
				lm8325_write(lm, 2, buf);
			} else if (ints & 0x08) {
				/* MELINT set - Event buffer overflow. May
				 *have been caused by sudden occurence of
				 *more than 8 keyboard events */
				dev_err(&lm->client->dev,
					"%s: MELINT set. Event buffer overflow interrupt. Handled by clearing all pending interrupts\n",
					__func__);

				/* Clear the pending interrupts to get back to
				 *the right state */
				buf[0] = LM8325_CMD_KBDIC;
				buf[1] = 0x82;
				lm8325_write(lm, 2, buf);
			} else {
				dev_err(&lm->client->dev,
					"%s: Unknown state. Handled by clearing all pending interrupts\n",
					__func__);
				/* clear any interrupts set */
				buf[0] = LM8325_CMD_KBDIC;
				buf[1] = 0x83;
				lm8325_write(lm, 2, buf);

				/* Mask the required interrupts */
				buf[0] = LM8325_CMD_KBDMSK;
				buf[1] = 0x3;
				lm8325_write(lm, 2, buf);
			}
		}
	} else if (ints & 0x80) {
		/* PORINT set : Device has been reset - Wrong state to be in */
		dev_err(&lm->client->dev,
			"%s: PORINT set. Device in reset. Need to reinitialised\n",
			__func__);

		/* Re-configure the device */
		lm8325_configure(lm);
	} else {
		/* Some other type of interrupt - can not be handled here */
		dev_err(&lm->client->dev, "%s: Error type not handled\n",
			__func__);
		/* clear any interrupts set */
		buf[0] = LM8325_CMD_KBDIC;
		buf[1] = 0x83;
		lm8325_write(lm, 2, buf);

		/* Mask the required interrupts */
		buf[0] = LM8325_CMD_KBDMSK;
		buf[1] = 0x3;
		lm8325_write(lm, 2, buf);
	}

	mutex_unlock(&lm->lock);
}

/**
 *lm8325_irq - Interrupt service routine for keyboard interrupt.
 */
static irqreturn_t lm8325_irq(int irq, void *data)
{
	struct lm8325_chip *lm = data;

	schedule_work(&lm->work);

	return IRQ_HANDLED;
}

/**
 *sysfs implementation for the keyboard device to enable and disable it.
 */
static ssize_t lm8325_show_disable(struct device *dev,
				   struct device_attribute *attr, char *buf)
{
	struct lm8325_chip *lm = dev_get_drvdata(dev);

	return sprintf(buf, "%u\n", !lm->kp_enabled);
}

static ssize_t lm8325_set_disable(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t count)
{
	struct lm8325_chip *lm = dev_get_drvdata(dev);
	int ret;
	int i;

	i = sscanf(buf, "%d", &ret);

	mutex_lock(&lm->lock);
	lm->kp_enabled = !i;
	mutex_unlock(&lm->lock);

	return count;
}

static DEVICE_ATTR(disable_kp, 0644, lm8325_show_disable, lm8325_set_disable);

/**
 *lm8325_probe - LM8325 keyboard controller probe.
 */
static int lm8325_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	struct lm8325_platform_data *lm8325_pdata;
	struct input_dev *idev;
	struct lm8325_chip *lm;
	int i, err = 0;

	/* 1. Allocate the memory for lm chip struct */
	lm = kzalloc(sizeof *lm, GFP_KERNEL);
	if (!lm)
		return -ENOMEM;

	/* 2. Set the I2C client data */
	i2c_set_clientdata(client, lm);
	lm->client = client;

	/* 3. Get the platform data for the given board */
	lm8325_pdata = client->dev.platform_data;
	if (!lm8325_pdata) {
		err = -EINVAL;
		goto err1;
	}

	/* 4. Set the number of rows and columns as specified
	   in the platform data */
	lm->size_x = lm8325_pdata->size_x;
	if (lm->size_x == 0) {
		lm->size_x = 8;
	} else if (lm->size_x > 8) {
		dev_err(&client->dev, "invalid x size %d specified\n",
			lm->size_x);
		lm->size_x = 8;
	}

	lm->size_y = lm8325_pdata->size_y;
	if (lm->size_y == 0) {
		lm->size_y = 12;
	} else if (lm->size_y > 12) {
		dev_err(&client->dev, "invalid y size %d specified\n",
			lm->size_y);
		lm->size_x = 12;
	}

	/* 5. Set the debounce time, active time, iocfg and autosleep to be set */
	lm->debounce_time = lm8325_pdata->debounce_time;
	if (lm->debounce_time == 0)	/* Default. */
		lm->debounce_time = 0x80;

	lm->settle_time = lm8325_pdata->settle_time;
	if (lm->settle_time == 0)	/* Default. */
		lm->settle_time = 0x80;

	lm->iocfg = lm8325_pdata->iocfg;
	if (lm->iocfg == 0)	/* Default */
		lm->iocfg = 0xF8;

	lm->autosleep = lm8325_pdata->autosleep;
	lm->sleep_time = lm8325_pdata->sleep_time;

	/* 6. Reset the controller and wait till its done - This is done by
	   reading the controller register */
	/* lm8325_reset(lm); */

#ifdef DEBUG_ENABLE
	lm8325_reg_dump(lm);
#endif

	/* 7. Initial configuration of the controller */
	lm8325_configure(lm);

#ifdef DEBUG_ENABLE
	lm8325_reg_dump(lm);
#endif

	mutex_init(&lm->lock);

	/* 8. Initialize the work queue where the key press event is handled */
	INIT_WORK(&lm->work, lm8325_work);

	/* 9. Interrupt request for keypress interrupt from the controller */
	err =
	    request_irq(client->irq, lm8325_irq, IRQF_TRIGGER_FALLING,
			"lm8325_keypress", lm);
	if (err) {
		dev_err(&client->dev, "could not get IRQ %d\n", client->irq);
		goto err1;
	}

	irq_set_irq_wake(client->irq, 1);

	/* 10. Input device creation logic for the keypad device */
	/* 10a. Create a sysfs interface */
	err = device_create_file(&client->dev, &dev_attr_disable_kp);
	if (err < 0)
		goto err2;

	/* 10b. Allocate the device */
	idev = input_allocate_device();
	if (idev == NULL) {
		err = -ENOMEM;
		goto err3;
	}

	/* 10c. Set the device parameters and attributes */
	if (lm8325_pdata->name)
		idev->name = lm8325_pdata->name;
	else
		idev->name = "lm8325 keypad";

	idev->phys = "keypad/input0";
	idev->id.bustype = BUS_HOST;
	idev->id.vendor = 0x0001;
	idev->id.product = 0x0001;
	idev->id.version = 0x0100;

	idev->evbit[0] = BIT(EV_KEY);
	for (i = 0; i < LM8325_KEYMAP_SIZE; i++) {
		if (lm8325_pdata->keymap[i] > 0)
			set_bit(lm8325_pdata->keymap[i], idev->keybit);

		lm->keymap[i] = lm8325_pdata->keymap[i];
	}

	if (lm8325_pdata->repeat)
		set_bit(EV_REP, idev->evbit);

	lm->idev = idev;
	err = input_register_device(idev);
	if (err) {
		dev_dbg(&client->dev, "error registering input device\n");
		goto err3;
	}

	/* Enable the kaypad enable bit */
	lm->kp_enabled = 1;
	lm->key_pressed = 0;
	lm->error_count = 0;

	/* If the complete probe sequence is successfully executed, return 0 */
	dev_info(&client->dev,
		 "LM8325 keyboard controller initialization complete..\n");

	return 0;

      err3:
	device_remove_file(&client->dev, &dev_attr_disable_kp);
      err2:
	free_irq(client->irq, lm);
      err1:
	kfree(lm);
	return err;
}

/**
 *lm8325_remove - LM8325 keyboard controller remove.
 */
static int lm8325_remove(struct i2c_client *client)
{
	struct lm8325_chip *lm = i2c_get_clientdata(client);

	free_irq(client->irq, lm);
	cancel_work_sync(&lm->work);
	input_unregister_device(lm->idev);
	device_remove_file(&lm->client->dev, &dev_attr_disable_kp);
	kfree(lm);

	return 0;
}

static const struct i2c_device_id lm8325_id[] = {
	{"lm8325_kp", 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, lm8325_id);

static struct i2c_driver lm8325_i2c_driver = {
	.driver = {
		   .name = "lm8325_kp",
		   .owner = THIS_MODULE,
		   },
	.probe = lm8325_probe,
	.remove = __devexit_p(lm8325_remove),
	.id_table = lm8325_id,
};

static int __init lm8325_init(void)
{
	return i2c_add_driver(&lm8325_i2c_driver);
}

static void __exit lm8325_exit(void)
{
	i2c_del_driver(&lm8325_i2c_driver);
}

module_init(lm8325_init);
module_exit(lm8325_exit);

MODULE_AUTHOR("Broadcom Corporation");
MODULE_DESCRIPTION("LM8325 keypad driver");
MODULE_LICENSE("GPL");
