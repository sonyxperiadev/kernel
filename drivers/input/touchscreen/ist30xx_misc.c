/*
 *  Copyright (C) 2010,Imagis Technology Co. Ltd. All Rights Reserved.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 */

#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/slab.h>

#include "ist30xx.h"
#include "ist30xx_update.h"
#include "ist30xx_misc.h"

#define STATUS_MSG_LEN          (128)

extern struct ist30xx_data *ts_data;


#define CH_X                    (10)
#define CH_Y                    (13)
#define CH_Y_ALIGN              (14)

#define FRAME_READ_LEN          (CH_X * CH_Y_ALIGN * 2)
#define CMD_MSG_LEN             (128)

static u32 *ist30xx_frame_buf;
static char cmd_msg[CMD_MSG_LEN];

int ist30xx_read_frame(u32 *frame_buf, int len)
{
	int ret;
	u32 read_len;

	ret = ist30xx_write_cmd(ts_data->client, CMD_ENTER_REG_ACCESS, 0);
	if (ret)
		return -EIO;

	/* wait until mode changed */
	msleep(40);

	read_len = len / 2;
	ret = ist30xx_write_cmd(ts_data->client, IST30XX_RX_CNT_ADDR, read_len);
	if (ret)
		return -EIO;

	ret = ist30xx_read_buf(ts_data->client, IST30XX_RAW_ADDR, frame_buf, read_len);
	if (ret)
		return -EIO;

	read_len = len / 2;
	ret = ist30xx_write_cmd(ts_data->client, IST30XX_RX_CNT_ADDR, read_len);
	if (ret)
		return -EIO;
	ret = ist30xx_read_buf(ts_data->client, IST30XX_FILTER_ADDR, frame_buf + read_len,
			       read_len);
	if (ret)
		return -EIO;

	ret = ist30xx_write_cmd(ts_data->client, CMD_EXIT_REG_ACCESS, 0);
	if (ret)
		return -EIO;

	ret = ist30xx_write_cmd(ts_data->client, CMD_START_SCAN, 0);
	if (ret)
		return -EIO;

	return 0;
}


/* sysfs: /sys/class/touch/internal/mem */
ssize_t ist30xx_mem_store(struct device *dev, struct device_attribute *attr,
			  const char *buf, size_t size)
{
	sprintf(cmd_msg, "%s", buf);
	return size;
}

ssize_t ist30xx_mem_show(struct device *dev, struct device_attribute *attr,
			 char *buf)
{
	int ret;
	int count = 0;
	char cmd;
	u32 addr = 0;
	u32 val = 0;
	u32 addr_end;
	const int msg_len = 256;
	char msg[msg_len];

	sscanf(cmd_msg, "%c %x %x", &cmd, &addr, &val);

	/* enter reg access mode */
	ret = ist30xx_write_cmd(ts_data->client, CMD_ENTER_REG_ACCESS, 0);
	if (ret)
		return 0;

	/* wait until mode changed */
	msleep(40);

	switch (cmd) {
	case 'w':
	case 'W':
		ret = ist30xx_write_cmd(ts_data->client, addr, val);
		if (ret)
			count = sprintf(buf, "write  0x%08x : 0x%08x => failed...\n", addr, val);
		else
			count = sprintf(buf, "write 0x%08x : 0x%08x => Okay\n", addr, val);
		break;

	case 'r':
	case 'R':
		ret = ist30xx_read_cmd(ts_data->client, addr, &val);
		if (ret)
			count = sprintf(buf, "read  0x%08x : 0x%08x => failed\n", addr, val);
		else
			count = sprintf(buf, "read  0x%08x : 0x%08x => Okay\n", addr, val);
		break;

	case 'd':
	case 'D':
		addr_end = val;
		count = sprintf(buf, "=> dump from 0x%08x to 0x%08x\n", addr, addr_end);

		while (addr <= addr_end) {
			ret = ist30xx_read_cmd(ts_data->client, addr, &val);
			if (ret)
				count += snprintf(msg, msg_len, "dump 0x%08x : 0x%08x => failed\n",
						  addr, val);
			else
				count += snprintf(msg, msg_len, "dump 0x%08x : 0x%08x => Okay\n",
						  addr, val);
			strncat(buf, msg, msg_len);
			addr += 4;
		}
		break;

	default:
		count = sprintf(buf, "=> invalid commands");
		break;
	}

	/* exit reg access mode */
	ret = ist30xx_write_cmd(ts_data->client, CMD_EXIT_REG_ACCESS, 0);
	if (ret)
		return 0;

	/* start scan */
	ret = ist30xx_write_cmd(ts_data->client, CMD_START_SCAN, 0);
	if (ret)
		return 0;

	return count;
}


/* sysfs: /sys/class/touch/internal/raw */
ssize_t ist30xx_display_raw(u32 *frame_buf, char *buf, int mode)
{
	int diff;
	int index;
	int count = 0;
	int i, j;
	int raw, max_raw, min_raw, avg_raw;
	int base, max_base, min_base, avg_base;
	const int msg_len = 128;
	char msg[msg_len];

	avg_raw = avg_base = 0;
	max_raw = max_base = 0;
	min_raw = min_base = 0xfff;

	/* mode
	 *  0 : raw-base
	 *  1 : base
	 *  2 : raw, max_raw, min_raw
	 *  3 : raw only */

	for (i = 0; i < CH_Y; i++) {
		for (j = CH_X - 1; j >= 0; j--) {
			index = (j * CH_Y_ALIGN) + i;

			/* find max & min raw */
			raw = frame_buf[index] & 0xFFF;
			if (raw > max_raw)
				max_raw = raw;
			if (raw < min_raw)
				min_raw = raw;
			avg_raw += raw;

			/* find max & min base */
			base = (frame_buf[index] >> 16) & 0xFFF;
			if (base > max_base)
				max_base = base;
			if (base < min_base)
				min_base = base;
			avg_base += base;

			/* raw - base */
			diff = raw - base;
			diff = (diff < 0) ? 0 : diff;

			if (mode == 0)
				count += snprintf(msg, msg_len, "%4d ", diff);
			else if (mode == 1)
				count += snprintf(msg, msg_len, "%04d ", base);
			else
				count += snprintf(msg, msg_len, "%04d ", raw);

			strncat(buf, msg, msg_len);
		}

		count += snprintf(msg, msg_len, "\n");
		strncat(buf, msg, msg_len);
	}

	if (mode == 1) {
		avg_base /= (CH_X * CH_Y);
		count += snprintf(msg, msg_len,
				  "max : %d, min : %d, max - min : %d, avg : %d\n",
				  max_base, min_base, max_base - min_base, avg_base);
		strncat(buf, msg, msg_len);
	} else if (mode == 2) {
		avg_raw /= (CH_X * CH_Y);
		count += snprintf(msg, msg_len,
				  "max : %d, min : %d, max - min : %d, avg : %d\n",
				  max_raw, min_raw, max_raw - min_raw, avg_raw);
		strncat(buf, msg, msg_len);
	}

	return count;
}


/* sysfs: /sys/class/touch/internal/filter */
ssize_t ist30xx_display_filter(u32 *frame_buf, char *buf)
{
	int i, j;
	int index;
	int count = 0;
	const int msg_len = 128;
	char msg[msg_len];

	for (i = 0; i < CH_Y; i++) {
		for (j = CH_X - 1; j >= 0; j--) {
			index = (FRAME_READ_LEN / 2) + (j * CH_Y_ALIGN) + i;
			count += snprintf(msg, msg_len, "%4d ", frame_buf[index] & 0x7FFF);
			strncat(buf, msg, msg_len);
		}

		count += snprintf(msg, msg_len, "\n");
		strncat(buf, msg, msg_len);
	}

	return count;
}


/* sysfs: /sys/class/touch/internal/refresh */
ssize_t ist30xx_frame_refresh(struct device *dev, struct device_attribute *attr,
			      char *buf)
{
	int ret = 0;

	ret = ist30xx_read_frame(ist30xx_frame_buf, FRAME_READ_LEN);
	if (ret)
		ret = sprintf(buf, "cmd 1frame raw update fail\n");

	return ret;
}


/* sysfs: /sys/class/touch/internal/base */
ssize_t ist30xx_base_show(struct device *dev, struct device_attribute *attr,
			  char *buf)
{
	int count = 0;

	buf[0] = '\0';
	count = sprintf(buf, "dump ist30xx base : %d\n", FRAME_READ_LEN);
	count += ist30xx_display_raw(ist30xx_frame_buf, buf, 1);

	return count;
}


/* sysfs: /sys/class/touch/internal/raw */
ssize_t ist30xx_raw_show(struct device *dev, struct device_attribute *attr,
			 char *buf)
{
	int count = 0;

	buf[0] = '\0';
	count = sprintf(buf, "dump ist30xx raw : %d\n", FRAME_READ_LEN);
	count += ist30xx_display_raw(ist30xx_frame_buf, buf, 2);

	return count;
}


/* sysfs: /sys/class/touch/internal/diff */
ssize_t ist30xx_diff_show(struct device *dev, struct device_attribute *attr,
			  char *buf)
{
	int count = 0;

	buf[0] = '\0';
	count = sprintf(buf, "dump ist30xx raw-base diff : %d\n", FRAME_READ_LEN);
	count += ist30xx_display_raw(ist30xx_frame_buf, buf, 0);

	return count;
}


/* sysfs: /sys/class/touch/internal/filter */
ssize_t ist30xx_filter_show(struct device *dev, struct device_attribute *attr,
			    char *buf)
{
	int count = 0;

	buf[0] = '\0';
	count = sprintf(buf, "dump ist30xx filter : %d\n", FRAME_READ_LEN);
	count += ist30xx_display_filter(ist30xx_frame_buf, buf);

	return count;
}


/* sysfs: /sys/class/touch/internal/clb */
ssize_t ist30xx_calib_show(struct device *dev, struct device_attribute *attr,
			   char *buf)
{
	int ret;
	int count = 0;
	u32 value;

	ist30xx_disable_irq(ts_data);
	ist30xx_reset();
	ret = ist30xx_write_cmd(ts_data->client, CMD_RUN_DEVICE, 0);
	if (ret)
		goto calib_show_end;
	msleep(10);

	ret = ist30xx_read_cmd(ts_data->client, CMD_GET_CALIB_RESULT, &value);
	if (ret) {
		count = sprintf(buf, "Error Read Calibration Result\n");
		goto calib_show_end;
	}

	count = sprintf(buf,
			"Calibration Status : %d, Max raw gap : %d - (raw: %08x)\n",
			CALIB_TO_STATUS(value), CALIB_TO_GAP(value), value);

calib_show_end:
	ist30xx_start(ts_data);
	ist30xx_enable_irq(ts_data);

	return count;
}


/* sysfs: /sys/class/touch/internal/debug */
ssize_t ist30xx_debug_store(struct device *dev, struct device_attribute *attr,
			    const char *buf, size_t size)
{
	int ret = -1;
	u32 idle_en, debug_en;

	sscanf(buf, "%d %d", &idle_en, &debug_en);

	DMSG("[ TSP ] Use idle: %d, Use debug: %d\n", idle_en, debug_en);
	if (idle_en > 1 || debug_en > 1) {
		printk(KERN_ERR "[ TSP ] Unknown argument value, %d, %d\n",
		       idle_en, debug_en);
		goto debug_store_end;
	}

	ist30xx_disable_irq(ts_data);
	ist30xx_reset();
	ret = ist30xx_write_cmd(ts_data->client, CMD_RUN_DEVICE, 0);
	if (ret)
		goto debug_store_end;
	msleep(10);

	ret = ist30xx_write_cmd(ts_data->client, CMD_USE_IDLE, idle_en);
	if (ret) {
		printk(KERN_ERR "[ TSP ] idle i2c fail, %d\n", ret);
		goto debug_store_end;
	}

	ret = ist30xx_write_cmd(ts_data->client, CMD_USE_DEBUG, debug_en);
	if (ret)
		printk(KERN_ERR "[ TSP ] debug i2c fail, %d\n", ret);

debug_store_end:
	ist30xx_enable_irq(ts_data);

	return ret ? 0 : size;
}


#if IST30XX_TUNES
struct device *ist30xx_tunes_dev;

#define TUNES_CMD_WRITE         (1)
#define TUNES_CMD_READ          (2)
#define TUNES_CMD_REG_ENTER     (3)
#define TUNES_CMD_REG_EXIT      (4)
#define TUNES_CMD_UPDATE_PARAM  (5)
#define TUNES_CMD_UPDATE_FW     (6)

#pragma pack(1)
typedef struct {
	u8	cmd;
	u32	addr;
	u16	len;
} TUNES_INFO;
#pragma pack()

static TUNES_INFO ist30xx_tunes;
static bool tunes_cmd_done = false;
static bool ist30xx_reg_mode = false;

/* sysfs: /sys/class/touch/tunes/regcmd */
ssize_t tunes_regcmd_store(struct device *dev, struct device_attribute *attr,
			   const char *buf, size_t size)
{
	int ret = -1;
	u32 *buf32;

	memcpy(&ist30xx_tunes, buf, sizeof(ist30xx_tunes));
	buf += sizeof(ist30xx_tunes);
	buf32 = (u32 *)buf;

	tunes_cmd_done = false;

	switch (ist30xx_tunes.cmd) {
	case TUNES_CMD_WRITE:
		break;
	case TUNES_CMD_READ:
		break;
	case TUNES_CMD_REG_ENTER:
		ist30xx_disable_irq(ts_data);

		/* enter reg access mode */
		ist30xx_reset();

		ret = ist30xx_write_cmd(ts_data->client, CMD_RUN_DEVICE, 0);
		if (ret)
			goto regcmd_fail;
		msleep(10);

		ret = ist30xx_write_cmd(ts_data->client, CMD_ENTER_REG_ACCESS, 0);
		if (ret)
			goto regcmd_fail;

		ist30xx_reg_mode = true;

		break;
	case TUNES_CMD_REG_EXIT:
		/* exit reg access mode */
		ret = ist30xx_write_cmd(ts_data->client, CMD_EXIT_REG_ACCESS, 0);
		if (ret)
			goto regcmd_fail;
		msleep(10);

		/* start scan */
		ret = ist30xx_write_cmd(ts_data->client, CMD_START_SCAN, 0);
		if (ret)
			goto regcmd_fail;

		ist30xx_reg_mode = false;

		ist30xx_enable_irq(ts_data);
		break;
	default:
		ist30xx_enable_irq(ts_data);
		return 0;
	}
	tunes_cmd_done = true;

	return size;

regcmd_fail:
	printk(KERN_ERR "[ TSP ] Tunes regcmd i2c_fail, ret=%d\n", ret);
	return 0;
}

ssize_t tunes_regcmd_show(struct device *dev, struct device_attribute *attr,
			  char *buf)
{
	int size;

	size = sprintf(buf, "cmd: 0x%02x, addr: 0x%08x, len: 0x%04x\n",
		       ist30xx_tunes.cmd, ist30xx_tunes.addr, ist30xx_tunes.len);

	return size;
}

#define MAX_WRITE_LEN   (1)
/* sysfs: /sys/class/touch/tunes/reg */
ssize_t tunes_reg_store(struct device *dev, struct device_attribute *attr,
			const char *buf, size_t size)
{
	int ret;
	u32 *buf32 = (u32 *)buf;
	int waddr, wcnt = 0, len = 0;

	if (ist30xx_tunes.cmd != TUNES_CMD_WRITE) {
		printk(KERN_ERR "[ TSP ] error, IST30XX_REG_CMD is not correct!\n");
		return 0;
	}

	if (!ist30xx_reg_mode) {
		printk(KERN_ERR "[ TSP ] error, IST30XX_REG_CMD is not ready!\n");
		return 0;
	}

	if (!tunes_cmd_done) {
		printk(KERN_ERR "[ TSP ] error, IST30XX_REG_CMD is not ready!\n");
		return 0;
	}

	waddr = ist30xx_tunes.addr;
	if (ist30xx_tunes.len >= MAX_WRITE_LEN)
		len = MAX_WRITE_LEN;
	else
		len = ist30xx_tunes.len;

	while (wcnt < ist30xx_tunes.len) {
		ret = ist30xx_write_buf(ts_data->client, waddr, buf32, len);
		if (ret) {
			printk(KERN_ERR "[ TSP ] Tunes regstore i2c_fail, ret=%d\n", ret);
			return 0;
		}

		wcnt += len;

		if ((ist30xx_tunes.len - wcnt) < MAX_WRITE_LEN)
			len = ist30xx_tunes.len - wcnt;

		buf32 += MAX_WRITE_LEN;
		waddr += MAX_WRITE_LEN * IST30XX_DATA_LEN;
	}

	tunes_cmd_done = false;

	return size;
}

ssize_t tunes_reg_show(struct device *dev, struct device_attribute *attr,
		       char *buf)
{
	int ret;
	int size;
	u32 *buf32 = (u32 *)buf;

	//unsigned long flags;

	if (ist30xx_tunes.cmd != TUNES_CMD_READ) {
		printk(KERN_ERR "[ TSP ] error, IST30XX_REG_CMD is not correct!\n");
		return 0;
	}

	if (!tunes_cmd_done) {
		printk(KERN_ERR "[ TSP ] error, IST30XX_REG_CMD is not ready!\n");
		return 0;
	}

	size = ist30xx_tunes.len;
	ret = ist30xx_write_cmd(ts_data->client, IST30XX_RX_CNT_ADDR, size);
	if (ret) {
		printk(KERN_ERR "[ TSP ] Tunes regshow i2c_fail, ret=%d\n", ret);
		return 0;
	}

	//local_irq_save(flags);          // Activated only when the GPIO I2C is used
	ret = ist30xx_read_buf(ts_data->client, ist30xx_tunes.addr, buf32, size);
	//local_irq_restore(flags);       // Activated only when the GPIO I2C is used
	if (ret) {
		printk(KERN_ERR "[ TSP ] Tunes regshow i2c_fail, ret=%d\n", ret);
		return 0;
	}

	size = ist30xx_tunes.len * IST30XX_DATA_LEN;

	tunes_cmd_done = false;

	return size;
}


/* sysfs: /sys/class/touch/tunes/firmware */
ssize_t tunes_fw_store(struct device *dev, struct device_attribute *attr,
		       const char *buf, size_t size)
{
	ist30xx_get_update_info(ts_data, buf, size);

	mutex_lock(&ist30xx_mutex);
	ist30xx_fw_update(ts_data->client, buf, size, true);
	ist30xx_init_touch_driver(ts_data);
	mutex_unlock(&ist30xx_mutex);

	return size;
}


/* sysfs: /sys/class/touch/tunes/param */
ssize_t tunes_param_store(struct device *dev, struct device_attribute *attr,
			  const char *buf, size_t size)
{
	ist30xx_get_update_info(ts_data, buf, size);

	mutex_lock(&ist30xx_mutex);
	ist30xx_param_update(ts_data->client, buf, size);
	ist30xx_init_touch_driver(ts_data);
	mutex_unlock(&ist30xx_mutex);

	return size;
}

ssize_t tunes_adb_store(struct device *dev, struct device_attribute *attr,
			const char *buf, size_t size)
{
	int ret;
	char *tmp, *ptr;
	char token[9];
	u32 cmd, addr, len, val;
	int write_len;

	struct ist30xx_data *data = ts_data;

	sscanf(buf, "%x %x %x", &cmd, &addr, &len);

	switch (cmd) {
	case TUNES_CMD_WRITE:   /* write cmd */
		write_len = 0;
		ptr = (char *)(buf + 15);

		while (write_len < len) {
			memcpy(token, ptr, 8);
			token[8] = 0;
			val = simple_strtoul(token, &tmp, 16);
			ret = ist30xx_write_buf(data->client, addr, &val, 1);
			if (ret) {
				printk(KERN_ERR "[ TSP ] Tunes regstore i2c_fail, ret=%d\n", ret);
				return size;
			}

			ptr += 8;
			write_len++;
			addr += 4;
		}
		break;

	case TUNES_CMD_READ:   /* read cmd */
		ist30xx_tunes.cmd = cmd;
		ist30xx_tunes.addr = addr;
		ist30xx_tunes.len = len;
		break;

	case TUNES_CMD_REG_ENTER:   /* enter */
		ist30xx_disable_irq(data);
		ist30xx_reset();
		if (ist30xx_write_cmd(data->client, CMD_RUN_DEVICE, 0) < 0)
			goto cmd_fail;
		msleep(10);

		if (ist30xx_write_cmd(data->client, CMD_ENTER_REG_ACCESS, 0) < 0)
			goto cmd_fail;
		ist30xx_reg_mode = true;
		break;

	case TUNES_CMD_REG_EXIT:   /* exit */
		if (ist30xx_reg_mode == true) {
			if (ist30xx_write_cmd(data->client, CMD_EXIT_REG_ACCESS, 0) < 0)
				goto cmd_fail;
			msleep(10);

			if (ist30xx_write_cmd(data->client, CMD_START_SCAN, 0) < 0)
				goto cmd_fail;
			ist30xx_reg_mode = false;
			ist30xx_enable_irq(data);
		}
		break;

	default:
		break;
	}


	return size;

cmd_fail:
	printk(KERN_ERR "[ TSP ] Tunes adb i2c_fail\n");
	return size;
}

ssize_t tunes_adb_show(struct device *dev, struct device_attribute *attr,
		       char *buf)
{
	int ret;
	int i, len, size = 0;
	char reg_val[10];
	unsigned long flags;
	struct ist30xx_data *data = ts_data;

	ret = ist30xx_write_cmd(data->client, IST30XX_RX_CNT_ADDR, ist30xx_tunes.len);
	if (ret) {
		printk(KERN_ERR "[ TSP ] Tunes regshow i2c_fail, ret=%d\n", ret);
		return size;
	}

	local_irq_save(flags);
	ret = ist30xx_read_buf(data->client, ist30xx_tunes.addr, ist30xx_frame_buf, ist30xx_tunes.len);
	local_irq_restore(flags);
	if (ret) {
		printk(KERN_ERR "[ TSP ] Tunes regshow i2c_fail, ret=%d\n", ret);
		return size;
	}

	size = 0;
	buf[0] = 0;
	len = sprintf(reg_val, "%08x", ist30xx_tunes.addr);
	strcat(buf, reg_val);
	size += len;
	for (i = 0; i < ist30xx_tunes.len; i++) {
		//printk("cmd[r] : [%d] addr = %x, value %x\n",i, ist30xx_tunes.addr, ist30xx_frame_buf[i]);
		len = sprintf(reg_val, "%08x", ist30xx_frame_buf[i]);
		strcat(buf, reg_val);
		size += len;
	}

	return size;
}
#endif

#if 1
/* sysfs  */
static DEVICE_ATTR(mem, 0666, ist30xx_mem_show, ist30xx_mem_store);
static DEVICE_ATTR(refresh, 0666, ist30xx_frame_refresh, NULL);
static DEVICE_ATTR(filter, 0666, ist30xx_filter_show, NULL);
static DEVICE_ATTR(raw, 0666, ist30xx_raw_show, NULL);
static DEVICE_ATTR(base, 0666, ist30xx_base_show, NULL);
static DEVICE_ATTR(diff, 0666, ist30xx_diff_show, NULL);
static DEVICE_ATTR(clb, 0666, ist30xx_calib_show, NULL);
static DEVICE_ATTR(debug, 0666, NULL, ist30xx_debug_store);

#if IST30XX_TUNES
static DEVICE_ATTR(regcmd, 0666, tunes_regcmd_show, tunes_regcmd_store);
static DEVICE_ATTR(reg, 0666, tunes_reg_show, tunes_reg_store);
static DEVICE_ATTR(tunes_fw, 0666, NULL, tunes_fw_store);
static DEVICE_ATTR(tunes_param, 0666, NULL, tunes_param_store);
static DEVICE_ATTR(adb, 0666,  NULL, tunes_adb_store);
#endif
#endif

extern struct class *ist30xx_class;
extern struct device *ist30xx_param_dev;
struct device *ist30xx_internal_dev;


int ist30xx_init_misc_sysfs(void)
{
#if 1
	/* /sys/class/touch/internal */
	ist30xx_internal_dev = device_create(ist30xx_class, NULL, 0, NULL, "internal");

	/* /sys/class/touch/internal/mem */
	if (device_create_file(ist30xx_internal_dev, &dev_attr_mem) < 0)
		printk(KERN_ERR "[ TSP ] Failed to create device file(%s)!\n",
		       dev_attr_mem.attr.name);

	/* /sys/class/touch/internal/refresh */
	if (device_create_file(ist30xx_internal_dev, &dev_attr_refresh) < 0)
		printk(KERN_ERR "[ TSP ] Failed to create device file(%s)!\n",
		       dev_attr_refresh.attr.name);

	/* /sys/class/touch/internal/filter */
	if (device_create_file(ist30xx_internal_dev, &dev_attr_filter) < 0)
		printk(KERN_ERR "[ TSP ] Failed to create device file(%s)!\n",
		       dev_attr_filter.attr.name);

	/* /sys/class/touch/internal/raw */
	if (device_create_file(ist30xx_internal_dev, &dev_attr_raw) < 0)
		printk(KERN_ERR "[ TSP ] Failed to create device file(%s)!\n",
		       dev_attr_raw.attr.name);

	/* /sys/class/touch/internal/base */
	if (device_create_file(ist30xx_internal_dev, &dev_attr_base) < 0)
		printk(KERN_ERR "[ TSP ] Failed to create device file(%s)!\n",
		       dev_attr_base.attr.name);

	/* /sys/class/touch/internal/diff */
	if (device_create_file(ist30xx_internal_dev, &dev_attr_diff) < 0)
		printk(KERN_ERR "[ TSP ] Failed to create device file(%s)!\n",
		       dev_attr_diff.attr.name);

	/* /sys/class/touch/internal/clb */
	if (device_create_file(ist30xx_internal_dev, &dev_attr_clb) < 0)
		printk(KERN_ERR "[ TSP ] Failed to create device file(%s)!\n",
		       dev_attr_clb.attr.name);

	/* /sys/class/touch/internal/debug */
	if (device_create_file(ist30xx_internal_dev, &dev_attr_debug) < 0)
		printk(KERN_ERR "[ TSP ] Failed to create device file(%s)!\n",
		       dev_attr_debug.attr.name);

#if IST30XX_TUNES
	/* /sys/class/touch/tunes */
	ist30xx_tunes_dev = device_create(ist30xx_class, NULL, 0, NULL, "tunes");

	/* /sys/class/touch/tunes/regcmd */
	if (device_create_file(ist30xx_tunes_dev, &dev_attr_regcmd) < 0)
		printk(KERN_ERR "[ TSP ] Failed to create device file(%s)!\n",
		       dev_attr_regcmd.attr.name);

	/* /sys/class/touch/tunes/reg */
	if (device_create_file(ist30xx_tunes_dev, &dev_attr_reg) < 0)
		printk(KERN_ERR "[ TSP ] Failed to create device file(%s)!\n",
		       dev_attr_reg.attr.name);

	/* /sys/class/touch/tunes/firmware */
	if (device_create_file(ist30xx_tunes_dev, &dev_attr_tunes_fw) < 0)
		printk(KERN_ERR "[ TSP ] Failed to create device file(%s)!\n",
		       dev_attr_tunes_fw.attr.name);

	/* /sys/class/touch/tunes/param */
	if (device_create_file(ist30xx_tunes_dev, &dev_attr_tunes_param) < 0)
		printk(KERN_ERR "[ TSP ] Failed to create device file(%s)!\n",
		       dev_attr_tunes_param.attr.name);

	/* /sys/class/touch/tunes/adb */
	if (device_create_file(ist30xx_tunes_dev, &dev_attr_adb) < 0)
		printk(KERN_ERR "[ TSP ] Failed to create device file(%s)!\n",
		       dev_attr_adb.attr.name);
#endif

	ist30xx_frame_buf = kmalloc(4096, GFP_KERNEL);
	if (!ist30xx_frame_buf)
		return -1;
#endif
	return 0;
}
