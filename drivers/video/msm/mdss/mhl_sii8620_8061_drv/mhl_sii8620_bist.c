/* vendor/semc/hardware/mhl/mhl_sii8620_8061_drv/mhl_sii8620_bist.h
 *
 * Copyright (C) 2014 Sony Mobile Communications inc.
 * Copyright (C) 2014 Silicon Image Inc.
 *
 */

#include <linux/device.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/semaphore.h>
#include <linux/ctype.h>
#include <linux/interrupt.h>

#include "mhl_common.h"
#include "mhl_sii8620_bist.h"
#include "mhl_sii8620_8061_device.h"
#include "mhl_tx.h"

#define MAX_DEBUG_TRANSFER_SIZE		32

struct bist_control_context {
	struct device dev;

	u8 debug_i2c_address;
	u8 debug_i2c_offset;
	u8 debug_i2c_xfer_length;

	struct semaphore isr_lock;	/* semaphore used to prevent driver */

	bool intr_disable;
};

static struct bist_control_context bist_context;

static void mhl_disable_irq_handler(void)
{
	pr_info("%s:\n", __func__);

	mhl_pf_switch_unregister_cb();
	disable_irq(mhl_pf_get_irq_number());
	mhl_tx_release();
	enable_irq(mhl_pf_get_irq_number());

	if (mhl_pf_is_chip_power_on())
		mhl_sii8620_clear_and_mask_interrupts(false);

	bist_context.intr_disable = true;
	pr_info("INTR disabled\n");

	mhl_discovery_timer_stop();
}

static void mhl_enable_irq_handler(void)
{
	pr_info("%s:\n", __func__);

	if (mhl_pf_is_chip_power_on())
		mhl_device_release_resource();

	mhl_pf_switch_to_usb();

	if (mhl_tx_initialize() < 0)
		pr_err("could not enable INTR\n");
	else {
		bist_context.intr_disable = false;
		pr_info("INTR enabled\n");
	}
}

/* chip power on API */
static void mhl_bist_hw_preparation(void)
{
	if (!bist_context.intr_disable)
		mhl_disable_irq_handler();

	if (!mhl_pf_is_chip_power_on()) {
		/* switch to mhl */
		pr_info("%s: switch to mhl\n", __func__);
		mhl_pf_switch_to_mhl();
		mhl_pf_chip_power_on();
		mhl_regs_init_for_ready_to_mhl_connection();
		mhl_sii8620_clear_and_mask_interrupts(false);
	}
}

/*
 * Wrapper for kstrtoul() that nul-terminates the input string at
 * the first non-digit character instead of returning an error.
 *
 * This function is destructive to the input string.
 *
 */
static int mhl_bist_strtoul(char **str, int base, unsigned long *val)
{
	int tok_length, status, nul_offset;
	char *tstr = *str;

	nul_offset = 1;
	status = -EINVAL;
	if ((base == 0) && (tstr[0] == '0') && (tolower(tstr[1]) == 'x')) {
		tstr += 2;
		base = 16;
	}

	tok_length = strspn(tstr, "0123456789ABCDEFabcdef");
	if (tok_length) {
		if ((tstr[tok_length] == '\n') || (tstr[tok_length] == 0))
			nul_offset = 0;

		tstr[tok_length] = 0;
		status = kstrtoul(tstr, base, val);
		if (status == 0) {
			tstr = (tstr + tok_length) + nul_offset;
			*str = tstr;
		}
	}
	return status;
}

static ssize_t set_intr_disable(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned long value = 0;
	int status = -EINVAL;
	char my_buf[20];
	unsigned int i;

	pr_info("%s: received string: %c%s%c\n", __func__, '"', buf, '"');
	if (count >= sizeof(my_buf)) {
		pr_err("string too long %c%s%c\n", '"', buf, '"');
		return status;
	}
	for (i = 0; i < count; ++i) {
		if ('\n' == buf[i]) {
			my_buf[i] = '\0';
			break;
		}
		if ('\t' == buf[i]) {
			my_buf[i] = '\0';
			break;
		}
		if (' ' == buf[i]) {
			my_buf[i] = '\0';
			break;
		}
		my_buf[i] = buf[i];
	}

	status = kstrtoul(my_buf, 0, &value);
	if (-ERANGE == status)
		pr_err("ERANGE %s\n", my_buf);
	else if (-EINVAL == status)
		pr_err("EINVAL %s\n", my_buf);
	else if (status != 0)
		pr_err("status:%d buf:%s\n", status, my_buf);
	else {
		if (value == 1) {
			if (!bist_context.intr_disable)
				mhl_disable_irq_handler();
			else
				pr_err("already INTR disabled\n");
		} else if (value == 0) {
			if (bist_context.intr_disable)
				mhl_enable_irq_handler();
			else
				pr_err("already INTR enabled\n");
		} else
			pr_err("invalid value:%d\n", (int)value);

		status = count;
	}

	return status;
}

static ssize_t show_intr_disable(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int status = -EINVAL;

	pr_info("%s\n", __func__);

	status = scnprintf(buf, PAGE_SIZE, "%d\n", bist_context.intr_disable ? 1 : 0);

	return status;
}

/*
 * set_reg_access_page() - Handle write request to set the
 *		reg access page value.
 *
 * The format of the string in buf must be:
 *	<pageaddr>
 * Where: <pageaddr> specifies the reg page of the register(s)
 *			to be written/read
 */
static ssize_t set_reg_access_page(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned long address = 0x100;
	int status = -EINVAL;
	char my_buf[20];
	unsigned int i;

	mhl_bist_hw_preparation();

	pr_info("%s: received string: %c%s%c\n", __func__, '"', buf, '"');
	if (count >= sizeof(my_buf)) {
		pr_err("string too long %c%s%c\n", '"', buf, '"');
		return status;
	}
	for (i = 0; i < count; ++i) {
		if ('\n' == buf[i]) {
			my_buf[i] = '\0';
			break;
		}
		if ('\t' == buf[i]) {
			my_buf[i] = '\0';
			break;
		}
		if (' ' == buf[i]) {
			my_buf[i] = '\0';
			break;
		}
		my_buf[i] = buf[i];
	}

	status = kstrtoul(my_buf, 0, &address);
	if (-ERANGE == status) {
		pr_err("ERANGE %s\n", my_buf);
	} else if (-EINVAL == status) {
		pr_err("EINVAL %s\n", my_buf);
	} else if (status != 0) {
		pr_err("status:%d buf:%s\n", status, my_buf);
	} else if (address > 0xFF) {
		pr_err("address:0x%x buf:%s\n", (unsigned int)address, my_buf);
	} else {
		if (down_interruptible(&bist_context.isr_lock)) {
			pr_err("could not get mutex\n");
			return -ERESTARTSYS;
		}
		bist_context.debug_i2c_address = address;
		status = count;

		up(&bist_context.isr_lock);
	}

	return status;
}

/*
 * show_reg_access_page()	- Show the current page number to be used when
 *	reg_access/data is accessed.
 *
 */
static ssize_t show_reg_access_page(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int status = -EINVAL;

	pr_info("%s\n", __func__);

	mhl_bist_hw_preparation();

	status = scnprintf(buf, PAGE_SIZE, "0x%02x",
			bist_context.debug_i2c_address);

	return status;
}

/*
 * set_reg_access_offset() - Handle write request to set the
 *		reg access page value.
 *
 * The format of the string in buf must be:
 *	<pageaddr>
 * Where: <pageaddr> specifies the reg page of the register(s)
 *			to be written/read
 */
static ssize_t set_reg_access_offset(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned long offset = 0x100;
	int status = -EINVAL;

	pr_info("%s: received string: " "%s" "\n", __func__, buf);

	mhl_bist_hw_preparation();

	status = kstrtoul(buf, 0, &offset);
	if (-ERANGE == status) {
		pr_err("ERANGE %s\n", buf);
	} else if (-EINVAL == status) {
		pr_err("EINVAL %s\n", buf);
	} else if (status != 0) {
		pr_err("status:%d buf:%c%s%c\n",
				status, '"', buf, '"');
	} else if (offset > 0xFF) {
		pr_err("offset:0x%x buf:%c%s%c\n",
				(unsigned int)offset, '"', buf, '"');
	} else {

		if (down_interruptible(&bist_context.isr_lock))
			return -ERESTARTSYS;
		bist_context.debug_i2c_offset = offset;
		status = count;

		up(&bist_context.isr_lock);
	}

	return status;
}

/*
 * show_reg_access_offset()	- Show the current page number to be used when
 *	reg_access/data is accessed.
 *
 */
static ssize_t show_reg_access_offset(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int status = -EINVAL;

	pr_info("%s\n", __func__);

	mhl_bist_hw_preparation();

	status = scnprintf(buf, PAGE_SIZE, "0x%02x",
			bist_context.debug_i2c_offset);

	return status;
}

/*
 * set_reg_access_length() - Handle write request to set the
 *		reg access page value.
 *
 * The format of the string in buf must be:
 *	<pageaddr>
 * Where: <pageaddr> specifies the reg page of the register(s)
 *			to be written/read
 */
static ssize_t set_reg_access_length(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned long length = 0x100;
	int status = -EINVAL;

	pr_info("%s: received string: " "%s" "\n", __func__, buf);

	mhl_bist_hw_preparation();

	status = kstrtoul(buf, 0, &length);
	if (-ERANGE == status) {
		pr_err("ERANGE %s\n", buf);
	} else if (-EINVAL == status) {
		pr_err("EINVAL %s\n", buf);
	} else if (status != 0) {
		pr_err("status:%d buf:%c%s%c\n",
				status, '"', buf, '"');
	} else if (length > 0xFF) {
		pr_err("length:0x%x buf:%c%s%c\n",
				(unsigned int)length, '"', buf, '"');
	} else {

		if (down_interruptible(&bist_context.isr_lock))
			return -ERESTARTSYS;
		bist_context.debug_i2c_xfer_length = length;
		status = count;

		up(&bist_context.isr_lock);
	}

	return status;
}

/*
 * show_reg_access_length()	- Show the current page number to be used when
 *	reg_access/data is accessed.
 *
 */
static ssize_t show_reg_access_length(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int status = -EINVAL;

	pr_info("%s\n", __func__);

	mhl_bist_hw_preparation();

	status = scnprintf(buf, PAGE_SIZE, "0x%02x",
			bist_context.debug_i2c_xfer_length);

	return status;
}

/*
 * set_reg_access_data() - Handle write request to the
 *	reg_access_data attribute file.
 *
 * This file is used to either perform a write to registers of the transmitter
 * or to set the address, offset and byte count for a subsequent from the
 * register(s) of the transmitter.
 *
 * The format of the string in buf must be:
 *	data_byte_0 ... data_byte_length-1
 * Where: data_byte is a space separated list of <length_value> data bytes
 *			to be written.  If no data bytes are present then
 *			the write to this file will only be used to set
 *			the  page address, offset and length for a
 *			subsequent read from this file.
 */
static ssize_t set_reg_access_data(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned long value;
	u8 data[MAX_DEBUG_TRANSFER_SIZE];
	int i;
	char *str;
	int status = -EINVAL;
	char *pinput = kmalloc(count, GFP_KERNEL);

	pr_info("%s: received string: %c%s%c\n", __func__, '"', buf, '"');

	mhl_bist_hw_preparation();

	if (pinput == 0)
		return -EINVAL;
	memcpy(pinput, buf, count);

	/*
	 * Parse the input string and extract the scratch pad register
	 * selection parameters
	 */
	str = pinput;
	for (i = 0; (i < MAX_DEBUG_TRANSFER_SIZE) && ('\0' != *str); i++) {

		status = mhl_bist_strtoul(&str, 0, &value);
		if (-ERANGE == status) {
			pr_err("ERANGE %s\n", str);
			goto exit_reg_access_data;
		} else if (-EINVAL == status) {
			pr_err("EINVAL %s\n", str);
			goto exit_reg_access_data;
		} else if (status != 0) {
			pr_err("status:%d %s\n", status, str);
			goto exit_reg_access_data;
		} else if (value > 0xFF) {
			pr_err("value:0x%x str:%s\n", (unsigned int)value, str);
			goto exit_reg_access_data;
		} else {
			data[i] = value;
		}
	}

	if (i == 0) {
		pr_info("No data specified\n");
		goto exit_reg_access_data;
	}

	if (down_interruptible(&bist_context.isr_lock)) {
		status = -ERESTARTSYS;
		goto exit_reg_access_data;
	}


	status = mhl_pf_write_reg_block(
			(bist_context.debug_i2c_address << 8)
			| bist_context.debug_i2c_offset,
			i, data);
	if (status == 0)
		status = count;

	up(&bist_context.isr_lock);

exit_reg_access_data:
	kfree(pinput);
	return status;
}

/*
 * show_reg_access_data()	- Handle read request to the
				reg_access_data attribute file.
 *
 * Reads from this file return one or more transmitter register values in
 * hexadecimal string format.  The registers returned are specified by the
 * address, offset and length values previously written to this file.
 *
 * The return value is the number characters written to buf, or an error
 * code if the I2C read fails.
 *
 * The format of the string returned in buf is:
 *	"address:<pageaddr> offset:<offset> length:<lenvalue> data:<datavalues>
 *	where:	<pageaddr>  is the last I2C register page address written
 *				to this file
 *		<offset>    is the last register offset written to this file
 *		<lenvalue>  is the last register transfer length written
 *				to this file
 *		<datavalue> space separated list of <lenvalue> register
 *				values in OxXX format
 */
static ssize_t show_reg_access_data(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	u8 data[MAX_DEBUG_TRANSFER_SIZE] = {0};
	u8 idx;
	int status = -EINVAL;

	pr_info("%s\n", __func__);

	mhl_bist_hw_preparation();

	if (down_interruptible(&bist_context.isr_lock))
		return -ERESTARTSYS;

	status = mhl_pf_read_reg_block(
			(bist_context.debug_i2c_address << 8)
			| bist_context.debug_i2c_offset,
			bist_context.debug_i2c_xfer_length,
			data);

	up(&bist_context.isr_lock);

	if (status == 0) {
		status = scnprintf(buf, PAGE_SIZE,
			"0x%02x'0x%02x:",
			bist_context.debug_i2c_address,
			bist_context.debug_i2c_offset
			);

		for (idx = 0; idx < bist_context.debug_i2c_xfer_length;
			idx++) {
			status += scnprintf(&buf[status], PAGE_SIZE, " 0x%02x",
				data[idx]);
		}
		pr_info("%s buf: %s\n", __func__, buf);
	}

	return status;
}

#define SYS_ATTR_NAME_INTR_DISABLE		intr_disable

#define SYS_OBJECT_NAME_REG_ACCESS		reg_access
#define		SYS_ATTR_NAME_REG_ACCESS_PAGE	page
#define		SYS_ATTR_NAME_REG_ACCESS_OFFSET	offset
#define		SYS_ATTR_NAME_REG_ACCESS_LENGTH	length
#define		SYS_ATTR_NAME_REG_ACCESS_DATA	data

static struct device_attribute intr_disable_attr =
	__ATTR(SYS_ATTR_NAME_INTR_DISABLE,
	S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP,
	show_intr_disable, set_intr_disable);

static struct device_attribute reg_access_page_attr =
	__ATTR(SYS_ATTR_NAME_REG_ACCESS_PAGE,
	S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP,
	show_reg_access_page, set_reg_access_page);

static struct device_attribute reg_access_offset_attr =
	__ATTR(SYS_ATTR_NAME_REG_ACCESS_OFFSET,
	S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP,
	show_reg_access_offset, set_reg_access_offset);

static struct device_attribute reg_access_length_attr =
	__ATTR(SYS_ATTR_NAME_REG_ACCESS_LENGTH,
	S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP,
	show_reg_access_length, set_reg_access_length);

static struct device_attribute reg_access_data_attr =
	__ATTR(SYS_ATTR_NAME_REG_ACCESS_DATA,
	S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP,
	show_reg_access_data, set_reg_access_data);

static struct attribute *reg_access_attrs[] = {
	&intr_disable_attr.attr,
	&reg_access_page_attr.attr,
	&reg_access_offset_attr.attr,
	&reg_access_length_attr.attr,
	&reg_access_data_attr.attr,
	NULL
};

static struct attribute_group reg_access_attribute_group = {
	.name = __stringify(SYS_OBJECT_NAME_REG_ACCESS),
	.attrs = reg_access_attrs
};

void mhl_bist_initilize(struct device *parent)
{
	int rc;
	struct class *cls = parent->class;

	pr_info("%s\n", __func__);
	if (IS_ERR(cls)) {
		pr_err("%s: failed to create class", __func__);
		return;
	}

	bist_context.dev.class = cls;
	dev_set_name(&bist_context.dev, "sii-8620");
	rc = device_register(&bist_context.dev);
	if (rc) {
		pr_err("%s: failed to register device", __func__);
		return;
	}

	rc = sysfs_create_group(&bist_context.dev.kobj,
				&reg_access_attribute_group);

	/* from si_mhl_tx_reset_states */

	sema_init(&bist_context.isr_lock, 1);

	bist_context.intr_disable = false;
}

void mhl_bist_release(void)
{
	sysfs_remove_group(&bist_context.dev.kobj,
			&reg_access_attribute_group);
	device_unregister(&bist_context.dev);
}

