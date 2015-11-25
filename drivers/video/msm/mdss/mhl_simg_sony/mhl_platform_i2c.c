/* kernel/drivers/video/msm/mdss/mhl_sii8620_8061_drv/mhl_platform_i2c.c
 *
 * Copyright (C) 2013 Silicon Image Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */
/*
 * Copyright (C) 2014 Sony Mobile Communications Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation.
 */

#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/delay.h>

#include "mhl_platform.h"

static bool mhl_reg_dump_enable;
module_param(mhl_reg_dump_enable, bool, 0200);

/*i2c adapter*/
static struct i2c_adapter *i2c_bus_adapter;

#define I2C_RETRY_MAX 10

#define MAX_DEBUG_MSG_SIZE	1024
#define DUMP_I2C_TRANSFER(cnt, page, offset, count, values, write_flag)	\
	do { \
		if (mhl_reg_dump_enable) \
			dump_transfer(cnt, TX_INTERFACE_TYPE_I2C, page, offset,	\
					count, values, write_flag); \
	} while (0)
enum tx_interface_types {
	TX_INTERFACE_TYPE_I2C,
	TX_INTERFACE_TYPE_SPI
};

/*
 * Return a pointer to the file name part of the
 * passed path spec string.
 */
char *find_file_name(const char *path_spec)
{
	char *pc;

	for (pc = (char *)&path_spec[strlen(path_spec)];
		 pc != path_spec; --pc) {
		if ('\\' == *pc) {
			++pc;
			break;
		}
		if ('/' == *pc) {
			++pc;
			break;
		}
	}
	return pc;
}

static void print_formatted_debug_msg(int level,
						 char *file_spec,
						 const char *func_name,
						 int line_num,
						 char *fmt, ...)
{
	uint8_t		*msg = NULL;
	uint8_t		*msg_offset;
	char		*file_spec_sep = NULL;
	int			remaining_msg_len = MAX_DEBUG_MSG_SIZE;
	int			len;
	va_list		ap;

	if (fmt == NULL)
		return;

	/*
	 * Only want to print the file name where the debug print
	 * statement originated, not the path to it.
	 */
	if (file_spec != NULL)
		file_spec = find_file_name(file_spec);

	msg = kmalloc(remaining_msg_len, GFP_KERNEL);
	if (msg == NULL)
		return;

	msg_offset = msg;

	if (file_spec != NULL) {
		if (func_name != NULL)
			file_spec_sep = "->";
		else if (line_num != -1)
			file_spec_sep = ":";
	}

	len = scnprintf(msg_offset, remaining_msg_len, "mhl: ");
	msg_offset += len;
	remaining_msg_len -= len;

	if (file_spec) {
		len = scnprintf(msg_offset, remaining_msg_len, "%s", file_spec);
		msg_offset += len;
		remaining_msg_len -= len;
	}

	if (file_spec_sep) {
		len = scnprintf(msg_offset,
						 remaining_msg_len,
						 "%s", file_spec_sep);
		msg_offset += len;
		remaining_msg_len -= len;
	}

	if (func_name) {
		len = scnprintf(msg_offset, remaining_msg_len, "%s", func_name);
		msg_offset += len;
		remaining_msg_len -= len;
	}

	if (line_num != -1) {
		if ((file_spec != NULL) || (func_name != NULL))
			len = scnprintf(msg_offset,
							 remaining_msg_len,
							 ":%d ",
							 line_num);
		else
			len = scnprintf(msg_offset,
							 remaining_msg_len,
							 "%d ",
							 line_num);

		msg_offset += len;
		remaining_msg_len -= len;
	}

	va_start(ap, fmt);
	len = vscnprintf(msg_offset, remaining_msg_len, fmt, ap);
	va_end(ap);

	printk(msg);

	kfree(msg);
}

static void dump_transfer(void *context, enum tx_interface_types if_type,
					 u8 page, u8 offset,
					 u16 count, u8 *values, bool write)
{
	int		buf_size = 64;
	u16		idx;
	int		buf_offset;
	char	*buf;
	char	*if_type_msg;

	switch (if_type) {
	case TX_INTERFACE_TYPE_I2C:
		if_type_msg = "I2C";
		break;
	case TX_INTERFACE_TYPE_SPI:
		if_type_msg = "SPI";
		break;
	default:
		return;
	};

	if (count > 1) {
		/* 3 chars per byte displayed */
		buf_size += count * 3;

		/* plus per display row overhead */
		buf_size += ((count / 16) + 1) * 8;
	}

	buf = kmalloc(buf_size, GFP_KERNEL);
	if (!buf)
		return;

	if (count == 1) {

		scnprintf(buf, buf_size, "   %s %02X.%02X %s %02X\n",
					if_type_msg,
					page, offset,
					write ? "W" : "R",
					values[0]);
	} else {
		idx = 0;
		buf_offset = scnprintf(buf, buf_size, "%s %02X.%02X %s(%d)",
								 if_type_msg,
								 page, offset,
								 write ? "W" : "R",
								 count);

		for (idx = 0; idx < count; idx++) {
			if (0 == (idx & 0x0F))
				buf_offset += scnprintf(&buf[buf_offset],
										 buf_size - buf_offset,
										"\n%04X: ", idx);

			buf_offset += scnprintf(&buf[buf_offset],
										buf_size - buf_offset,
										"%02X ", values[idx]);
		}
		buf_offset += scnprintf(&buf[buf_offset], buf_size - buf_offset, "\n");
	}

	print_formatted_debug_msg(0, NULL, NULL, -1, buf);
	kfree(buf);
}

int mhl_pf_modify_reg(u16 address,
					  u8 mask, u8 value)
{
	int	reg_value;
	int	write_status;

	reg_value = mhl_pf_read_reg(address);
	if (reg_value < 0)
		return reg_value;

	reg_value &= ~mask;
	reg_value |= mask & value;

	write_status = mhl_pf_write_reg(address, reg_value);

	if (write_status < 0)
		return write_status;
	else
		return reg_value;
}
EXPORT_SYMBOL(mhl_pf_modify_reg);

static inline int platform_write_i2c_block(
				struct i2c_adapter *i2c_bus,
				 u8 page,
				 u8 offset,
				 u16 count,
				 u8 *values)
{
	struct i2c_msg			msg;
	u8						*buffer;
	int						ret;
	int						i;


	if (!mhl_pf_is_chip_power_on()) {
		pr_warn("%s:chip power is off\n", __func__);
		return -MHL_I2C_NOT_AVAILABLE;
	}

	buffer = kmalloc(count + 1, GFP_KERNEL);

	if (!buffer) {
		printk("%s:%d buffer allocation failed\n", __FUNCTION__, __LINE__);
		return -ENOMEM;
	}

	buffer[0] = offset;
	memmove(&buffer[1], values, count);

	msg.flags = 0;
	msg.addr = page >> 1;
	msg.buf = buffer;
	msg.len = count + 1;

	for (i = 0; i < I2C_RETRY_MAX; i++) {
		ret = i2c_transfer(i2c_bus, &msg, 1);

		if (ret != 1) {
			pr_err("%s:%d I2c write failed, retry 0x%02x:0x%02x\n"
					, __func__, __LINE__, page, offset);
			ret = -EIO;
			msleep(20);
		} else {
			ret = 0;
			break;
		}
	}

	kfree(buffer);

	return ret;
}


static int mhl_pf_write_reg_block_i2c(u8 page, u8 offset,
				u16 count, u8 *values)
{
	DUMP_I2C_TRANSFER(NULL, page, offset, count, values, true);

	return platform_write_i2c_block(i2c_bus_adapter, page, offset, count, values);
}


static int mhl_pf_write_reg_i2c(u8 page, u8 offset, u8 value)
{
	return mhl_pf_write_reg_block_i2c(page, offset, 1, &value);
}

int mhl_pf_write_reg(u16 address, u8 value)
{
	u8 page = (u8)(address >> 8);
	u8 offset = (u8)address;

	return mhl_pf_write_reg_i2c(page, offset, value);
}
EXPORT_SYMBOL(mhl_pf_write_reg);

static inline int platform_read_i2c_block(struct i2c_adapter *i2c_bus
								, u8 page
								, u8 offset
								, u8 count
								, u8 *values
								)
{
	struct i2c_msg			msg[2];
	int				ret;
	int				i;

	if (!mhl_pf_is_chip_power_on()) {
		pr_warn("%s:chip power is off\n", __func__);
		return -MHL_I2C_NOT_AVAILABLE;
	}

	msg[0].flags = 0;
	msg[0].addr = page >> 1;
	msg[0].buf = &offset;
	msg[0].len = 1;

	msg[1].flags = I2C_M_RD;
	msg[1].addr = page >> 1;
	msg[1].buf = values;
	msg[1].len = count;

	for (i = 0; i < I2C_RETRY_MAX; i++) {
		ret = i2c_transfer(i2c_bus_adapter, msg, 2);

		if (ret != 2) {
			pr_err("%s:%d I2c read failed, retry 0x%02x:0x%02x\n"
					, __func__, __LINE__, page, offset);
			msleep(20);
		} else {
			break;
		}
	}

	return ret;
}



int mhl_pf_read_reg_block(u16 address,
							  u8 count, u8 *values)
{
	int						ret;

	u8 page = (u8)(address >> 8);
	u8 offset = (u8)address;

	ret = platform_read_i2c_block(i2c_bus_adapter
								, page
								, offset
								, count
								, values
								);
	if (ret == -MHL_I2C_NOT_AVAILABLE) {
		pr_warn("%s:chip power is off:0x%04x\n", __func__, ret);
		pr_warn("%s:0x%02x:0x%02x\n", __func__, page, offset);
		return ret;
	} else if (ret != 2) {
		pr_err("%s:I2c read failed,  0x%02x:0x%02x\n", __func__, page, offset);
		ret = -EIO;
	} else {
		ret = 0;
		DUMP_I2C_TRANSFER(NULL, page, offset, count, values, false);
	}

	return ret;
}
EXPORT_SYMBOL(mhl_pf_read_reg_block);

int mhl_pf_read_reg(u16 address)
{
	u8		byte_read = 0xFF;
	int		status;
	status = mhl_pf_read_reg_block(address,
									 1,
									 &byte_read);
	return status ? status : byte_read;
}
EXPORT_SYMBOL(mhl_pf_read_reg);

void mhl_pf_i2c_init(struct i2c_adapter *adapter)
{
	i2c_bus_adapter = adapter;
#ifdef DEBUG_PRINT
	mhl_reg_dump_enable = true;
#else
	mhl_reg_dump_enable = false;
#endif
}

int mhl_pf_write_reg_block(u16 address, u16 count, u8 *values)
{
	u8 page = (u8)(address >> 8);
	u8 offset = (u8)address;

	DUMP_I2C_TRANSFER(NULL, page, offset, count, values, true);
	return platform_write_i2c_block(i2c_bus_adapter, page, offset, count, values);
}
EXPORT_SYMBOL(mhl_pf_write_reg_block);
