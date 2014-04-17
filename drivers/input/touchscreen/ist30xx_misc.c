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

#define TSP_CH_SCREEN   (1)
#define TSP_CH_KEY      (2)

#define CMD_MSG_LEN             (128)

extern struct ist30xx_data *ts_data;

TSP_INFO ist30xx_tsp_info;
TKEY_INFO ist30xx_tkey_info;

static u32 *ist30xx_frame_buf;
static u32 *ist30xx_frame_rawbuf;
static u32 *ist30xx_frame_fltbuf;
static char cmd_msg[CMD_MSG_LEN];


int ist30xx_tkey_update_info(void)
{
	int ret = 0;
	u32 tkey_info1, tkey_info2, tkey_info3;
	TKEY_INFO *tkey = &ist30xx_tkey_info;

	ret = ist30xx_cmd_run_device(ts_data->client);

	ret = ist30xx_read_cmd(ts_data->client, CMD_GET_KEY_INFO1, &tkey_info1);
	if (ret)
		return ret;
	ret = ist30xx_read_cmd(ts_data->client, CMD_GET_KEY_INFO2, &tkey_info2);
	if (ret)
		return ret;
	ret = ist30xx_read_cmd(ts_data->client, CMD_GET_KEY_INFO3, &tkey_info3);
	if (ret)
		return ret;

	tkey->enable = ((tkey_info1 & (0xFF << 24)) ? true : false);
	tkey->axis_chnum = (tkey_info1 & 0xFF);
	tkey->ch_num[0] = (tkey_info2 >> 24) & 0xFF;
	tkey->ch_num[1] = (tkey_info2 >> 16) & 0xFF;
	tkey->ch_num[2] = (tkey_info2 >> 8) & 0xFF;
	tkey->ch_num[3] = tkey_info2 & 0xFF;
	tkey->ch_num[4] = (tkey_info3 >> 24) & 0xFF;
	if (ist30xx_tsp_info.dir.txch_y)
		tkey->tx_line = ((tkey_info1 & (0x01 << 8)) ? false : true);
	else
		tkey->tx_line = ((tkey_info1 & (0x01 << 8)) ? true : false);
	if (ist30xx_tsp_info.dir.swap_xy)
		tkey->tx_line = (tkey->tx_line ? false : true);

	return ret;
}


#define TSP_INFO_SWAP_XY    (1 << 0)
#define TSP_INFO_FLIP_X     (1 << 1)
#define TSP_INFO_FLIP_Y     (1 << 2)
int ist30xx_tsp_update_info(void)
{
	int ret = 0;
	u32 tsp_ch_num1, tsp_ch_num2, tsp_swap_info, tsp_dir;
	TSP_INFO *tsp = &ist30xx_tsp_info;

	ret = ist30xx_cmd_run_device(ts_data->client);
	ret = ist30xx_read_cmd(ts_data->client,
			CMD_GET_TSP_SWAP_INFO, &tsp_swap_info);
	if (ret)
		return ret;

	ret = ist30xx_read_cmd(ts_data->client,
		CMD_GET_TSP_DIRECTION, &tsp_dir);
	if (ret)
		return ret;

	ret = ist30xx_read_cmd(ts_data->client,
		CMD_GET_TSP_CHNUM1, &tsp_ch_num1);
	if (ret || !tsp_ch_num1)
		return ret;

	ret = ist30xx_read_cmd(ts_data->client,
		CMD_GET_TSP_CHNUM2, &tsp_ch_num2);
	if (ret || !tsp_ch_num2)
		return ret;

	tsp->intl.x = tsp_ch_num1 >> 16;
	tsp->intl.y = tsp_ch_num1 & 0xFFFF;
	tsp->mod.x = tsp_ch_num2 >> 16;
	tsp->mod.y = tsp_ch_num2 & 0xFFFF;

	tsp->dir.txch_y = (tsp_dir & 1 ? true : false);
	tsp->dir.swap_xy = (tsp_swap_info & TSP_INFO_SWAP_XY ? 1 : 0);
	tsp->dir.flip_x = (tsp_swap_info & TSP_INFO_FLIP_X ? 1 : 0);
	tsp->dir.flip_y = (tsp_swap_info & TSP_INFO_FLIP_Y ? 1 : 0);

	tsp->buf.int_len = tsp->intl.x * tsp->intl.y;
	tsp->buf.mod_len = tsp->mod.x * tsp->mod.y;
	tsp->height = (tsp->dir.swap_xy ? tsp->mod.x : tsp->mod.y);
	tsp->width = (tsp->dir.swap_xy ? tsp->mod.y : tsp->mod.x);

	return ret;
}


int ist30xx_check_valid_ch(int width, int height)
{
	int w, h;
	TKEY_INFO *tkey = &ist30xx_tkey_info;

	if (tkey->tx_line) {
		w = width; h = height;
	} else {
		w = height; h = width;
	}

	if (tkey->enable) {
		if (h == tkey->axis_chnum) {
			if ((w == tkey->ch_num[0]) || (w == tkey->ch_num[1]) ||
			    (w == tkey->ch_num[2]) || (w == tkey->ch_num[3]) ||
			    (w == tkey->ch_num[4]))
				return TSP_CH_KEY;
			else
				return 0;
		} else {
			return TSP_CH_SCREEN;
		}
	} else {
		return TSP_CH_SCREEN;
	}

	return 0;
}


int ist30xx_parse_frame(u32 *raw_buf, u32 *flt_buf)
{
	int i, j, idx;
	u16 raw, base;
	u16 min_raw, max_raw, min_base, max_base;
	TSP_INFO *tsp = &ist30xx_tsp_info;

	max_raw = max_base = 0;
	min_raw = min_base = 0xFFF;

	for (i = 0; i < tsp->height; i++) {
		for (j = 0; j < tsp->width; j++) {
			idx = (j * tsp->height) + i;

			raw = raw_buf[idx] & 0xFFF;
			if (raw < min_raw)
				min_raw = raw;
			if (raw > max_raw)
				max_raw = raw;

			base = (raw_buf[idx] >> 16) & 0xFFF;
			if (base < min_base)
				min_base = base;
			if (base > max_base)
				max_base = base;

			if (ist30xx_check_valid_ch(j, i)) {
				tsp->buf.raw[j][i] = raw;
				tsp->buf.base[j][i] = base;
			} else {
				tsp->buf.raw[j][i] = tsp->buf.base[j][i] = 0;
			}
		}
	}

	for (i = 0; i < tsp->height; i++) {
		for (j = 0; j < tsp->width; j++) {
			idx = (j * tsp->height) + i;
			tsp->buf.filter[j][i] = flt_buf[idx] & 0xFFF;
		}
	}

	return 0;
}


int ist30xx_read_frame(u32 *raw_buf, u32 *flt_buf)
{
	int ret = 0;

	TSP_INFO *tsp = &ist30xx_tsp_info;

	ret = ist30xx_cmd_reg(ts_data->client, CMD_ENTER_REG_ACCESS);
	if (ret)
		return ret;

	ret = ist30xx_write_cmd(ts_data->client,
				IST30XX_RX_CNT_ADDR, tsp->buf.int_len);
	if (ret)
		return ret;

	ret = ist30xx_read_buf(ts_data->client, IST30XX_RAW_ADDR,
			       raw_buf, tsp->buf.int_len);
	if (ret)
		return ret;

	ret = ist30xx_write_cmd(ts_data->client,
				IST30XX_RX_CNT_ADDR, tsp->buf.int_len);
	if (ret)
		return ret;

	ret = ist30xx_read_buf(ts_data->client, IST30XX_FILTER_ADDR,
			       flt_buf, tsp->buf.int_len);
	if (ret)
		return ret;

	ret = ist30xx_cmd_reg(ts_data->client, CMD_EXIT_REG_ACCESS);
	if (ret)
		return ret;

	ist30xx_cmd_start_scan(ts_data->client);
	if (ret)
		return ret;

	return ret;
}


int ist30xx_parse_tsp_node(u16 *raw_buf, u16 *base_buf)
{
	int i, j, idx;
	u16 raw, base;
	TSP_INFO *tsp = &ist30xx_tsp_info;

	for (i = 0; i < tsp->height; i++) {
		for (j = 0; j < tsp->width; j++) {
			idx = (j * tsp->height) + i;

			raw = ist30xx_frame_rawbuf[idx] & 0xFFF;
			base = (ist30xx_frame_rawbuf[idx] >> 16) & 0xFFF;

			if (ist30xx_check_valid_ch(j, i) == TSP_CH_SCREEN) {
				*raw_buf++ = raw;
				*base_buf++ = base;
			}
		}
	}

	return 0;
}

int ist30xx_read_tsp_node(u16 *raw_buf, u16 *base_buf)
{
	int ret = 0;
	TSP_INFO *tsp = &ist30xx_tsp_info;

	ret = ist30xx_enter_debug_mode();
	if (ret)
		return ret;

	ret = ist30xx_cmd_reg(ts_data->client, CMD_ENTER_REG_ACCESS);
	if (ret)
		return ret;

	ret = ist30xx_write_cmd(ts_data->client,
				IST30XX_RX_CNT_ADDR, tsp->buf.int_len);
	if (ret)
		return ret;

	ret = ist30xx_read_buf(ts_data->client, IST30XX_RAW_ADDR,
			       ist30xx_frame_rawbuf, tsp->buf.int_len);
	if (ret)
		return ret;

	ret = ist30xx_cmd_reg(ts_data->client, CMD_EXIT_REG_ACCESS);
	if (ret)
		return ret;

	ret = ist30xx_cmd_start_scan(ts_data->client);
	if (ret)
		return ret;

	ret = ist30xx_parse_tsp_node(raw_buf, base_buf);

	return ret;
}


int ist30xx_enter_debug_mode(void)
{
	int ret;

	ist30xx_disable_irq(ts_data);
	ret = ist30xx_cmd_run_device(ts_data->client);
	if (ret)
		goto end_debug_mode;

	ret = ist30xx_write_cmd(ts_data->client, CMD_USE_IDLE, 0);
	if (ret)
		goto end_debug_mode;
	ret = ist30xx_write_cmd(ts_data->client, CMD_USE_DEBUG, 1);
	if (ret)
		goto end_debug_mode;

	ist30xx_enable_irq(ts_data);

	ret = ist30xx_cmd_start_scan(ts_data->client);

	msleep(700);

	return ret;

end_debug_mode:
	ist30xx_enable_irq(ts_data);
	return ret;
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
	ret = ist30xx_cmd_reg(ts_data->client, CMD_ENTER_REG_ACCESS);
	if (ret)
		return 0;

	switch (cmd) {
	case 'w':
	case 'W':
		ret = ist30xx_write_cmd(ts_data->client, addr, val);
		if (ret)
			count = sprintf(buf,
						"write  0x%08x : 0x%08x => failed...\n",
						addr, val);
		else
			count = sprintf(buf,
						"write 0x%08x : 0x%08x => Okay\n",
						addr, val);
		break;

	case 'r':
	case 'R':
		ret = ist30xx_read_cmd(ts_data->client, addr, &val);
		if (ret)
			count = sprintf(buf,
						"read  0x%08x : 0x%08x => failed\n",
						addr, val);
		else
			count = sprintf(buf,
						"read  0x%08x : 0x%08x => Okay\n",
						addr, val);
		break;

	case 'd':
	case 'D':
		addr_end = val;
		count = sprintf(buf, "=> dump from 0x%08x to 0x%08x\n",
						addr, addr_end);

		while (addr <= addr_end) {
			ret = ist30xx_read_cmd(ts_data->client, addr, &val);
			if (ret)
				count += snprintf(msg, msg_len,
						"dump 0x%08x : 0x%08x => failed\n",
						addr, val);
			else
				count += snprintf(msg, msg_len,
						"dump 0x%08x : 0x%08x => Okay\n",
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
	ret = ist30xx_cmd_reg(ts_data->client, CMD_EXIT_REG_ACCESS);
	if (ret)
		return 0;

	/* start scan */
	ret = ist30xx_cmd_start_scan(ts_data->client);
	if (ret)
		return 0;

	return count;
}


/* sysfs: /sys/class/touch/internal/refresh */
ssize_t ist30xx_frame_refresh(struct device *dev, struct device_attribute *attr,
			      char *buf)
{
	int ret = 0;

	ret = ist30xx_read_frame(ist30xx_frame_rawbuf, ist30xx_frame_fltbuf);
	if (ret)
		ret = sprintf(buf, "cmd 1frame raw update fail\n");

	ret = ist30xx_parse_frame(ist30xx_frame_rawbuf, ist30xx_frame_fltbuf);

	return ret;
}


/* sysfs: /sys/class/touch/internal/base */
ssize_t ist30xx_base_show(struct device *dev, struct device_attribute *attr,
			  char *buf)
{
	int i, j;
	int count = 0;
	const int msg_len = 128;
	char msg[msg_len];
	TSP_INFO *tsp = &ist30xx_tsp_info;

	buf[0] = '\0';
	count = sprintf(buf, "dump ist30xx base : %d\n", tsp->buf.mod_len);

	for (i = 0; i < tsp->height; i++) {
		for (j = 0; j < tsp->width; j++) {
			count += snprintf(msg, msg_len,
			"%04d ", tsp->buf.base[j][i]);
			strncat(buf, msg, msg_len);
		}

		count += snprintf(msg, msg_len, "\n");
		strncat(buf, msg, msg_len);
	}

	count += snprintf(msg, msg_len, "max : %d, min : %d, max - min : %d\n",
			  tsp->buf.max_base, tsp->buf.min_base,
			  tsp->buf.max_base - tsp->buf.min_raw);
		strncat(buf, msg, msg_len);

	return count;
}


/* sysfs: /sys/class/touch/internal/raw */
ssize_t ist30xx_raw_show(struct device *dev, struct device_attribute *attr,
			 char *buf)
{
	int i, j;
	int count = 0;
	TSP_INFO *tsp = &ist30xx_tsp_info;
	const int msg_len = 128;
	char msg[msg_len];

	buf[0] = '\0';
	count = sprintf(buf, "dump ist30xx raw : %d\n", tsp->buf.mod_len);

	for (i = 0; i < tsp->height; i++) {
		for (j = 0; j < tsp->width; j++) {
			count += snprintf(msg, msg_len,
			"%04d ", tsp->buf.raw[j][i]);
			strncat(buf, msg, msg_len);
		}

		count += snprintf(msg, msg_len, "\n");
		strncat(buf, msg, msg_len);
	}

	count += snprintf(msg, msg_len, "max : %d, min : %d, max - min : %d\n",
			  tsp->buf.max_base, tsp->buf.min_base,
			  tsp->buf.max_base - tsp->buf.min_raw);
	strncat(buf, msg, msg_len);

	return count;
}


/* sysfs: /sys/class/touch/internal/diff */
ssize_t ist30xx_diff_show(struct device *dev, struct device_attribute *attr,
			      char *buf)
{
	int i, j, diff;
	int count = 0;
	TSP_INFO *tsp = &ist30xx_tsp_info;
	const int msg_len = 128;
	char msg[msg_len];

	buf[0] = '\0';
	count = sprintf(buf,
		"dump ist30xx raw-base diff : %d\n", tsp->buf.mod_len);

	for (i = 0; i < tsp->height; i++) {
		for (j = 0; j < tsp->width; j++) {
			if (tsp->buf.raw[j][i] - tsp->buf.base[j][i] < 0)
				diff = 0;
			else
				diff = tsp->buf.raw[j][i] - tsp->buf.base[j][i];
			count += snprintf(msg, msg_len, "%4d ", diff);
			strncat(buf, msg, msg_len);
}

		count += snprintf(msg, msg_len, "\n");
		strncat(buf, msg, msg_len);
	}

	return count;
}


/* sysfs: /sys/class/touch/internal/filter */
ssize_t ist30xx_filter_show(struct device *dev, struct device_attribute *attr,
			 char *buf)
{
	int i, j;
	int count = 0;
	TSP_INFO *tsp = &ist30xx_tsp_info;
	const int msg_len = 128;
	char msg[msg_len];

	buf[0] = '\0';
	count = sprintf(buf, "dump ist30xx filter : %d\n", tsp->buf.mod_len);

	for (i = 0; i < tsp->height; i++) {
		for (j = 0; j < tsp->width; j++) {
			count += snprintf(msg, msg_len,
				"%4d ", tsp->buf.filter[j][i]);
			strncat(buf, msg, msg_len);
		}

		count += snprintf(msg, msg_len, "\n");
		strncat(buf, msg, msg_len);
	}

	return count;
}


extern int calib_ms_delay;
/* sysfs: /sys/class/touch/internal/clb */
ssize_t ist30xx_calib_store(struct device *dev, struct device_attribute *attr,
			    const char *buf, size_t size)
{
	int ret = -1;
	int ms_delay;

	sscanf(buf, "%d", &ms_delay);

	if (ms_delay > 10 && ms_delay < 1000) /* 1sec ~ 100sec */
		calib_ms_delay = ms_delay;

	DMSG("[ TSP ] Calibration wait time %dsec\n", calib_ms_delay / 10);

	ist30xx_disable_irq(ts_data);
	ret = ist30xx_cmd_run_device(ts_data->client);
	if (ret)
		goto calib_store_end;

	ret = ist30xx_cmd_calibrate(ts_data->client);
	if (ret)
		goto calib_store_end;

calib_store_end:
	ist30xx_enable_irq(ts_data);
	ret = ist30xx_calib_wait();

	return size;
}

ssize_t ist30xx_calib_show(struct device *dev, struct device_attribute *attr,
			   char *buf)
{
	int ret;
	int count = 0;
	u32 value;

	ist30xx_disable_irq(ts_data);
	ret = ist30xx_cmd_run_device(ts_data->client);
	if (ret)
		goto calib_show_end;

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
		pr_err("[ TSP ] Unknown argument value, %d, %d\n",
		       idle_en, debug_en);
		goto debug_store_end;
	}

	ist30xx_disable_irq(ts_data);
	ret = ist30xx_cmd_run_device(ts_data->client);
	if (ret)
		goto debug_store_end;

	ret = ist30xx_write_cmd(ts_data->client, CMD_USE_IDLE, idle_en);
	if (ret) {
		pr_err("[ TSP ] idle i2c fail, %d\n", ret);
		goto debug_store_end;
	}

	ret = ist30xx_write_cmd(ts_data->client, CMD_USE_DEBUG, debug_en);
	if (ret)
		pr_err("[ TSP ] debug i2c fail, %d\n", ret);

debug_store_end:
	ist30xx_enable_irq(ts_data);

	return size;
}


/* sysfs: /sys/class/touch/internal/power */
ssize_t ist30xx_power_store(struct device *dev, struct device_attribute *attr,
			    const char *buf, size_t size)
{
	u32 power_en;

	sscanf(buf, "%d", &power_en);

	DMSG("[ TSP ] Power enable: %d\n", power_en);
	if (power_en > 1) {
		pr_err("[ TSP ] Unknown argument value, %d\n", power_en);
		return size;
	}

	if (power_en) {
		mutex_lock(&ist30xx_mutex);
		ist30xx_internal_resume(ts_data);
		ist30xx_enable_irq(ts_data);
		mutex_unlock(&ist30xx_mutex);

		ist30xx_start(ts_data);
	} else {
		mutex_lock(&ist30xx_mutex);
		ist30xx_disable_irq(ts_data);
		ist30xx_internal_suspend(ts_data);
		mutex_unlock(&ist30xx_mutex);
	}

	return size;
}

extern int ist30xx_max_error_cnt;
/* sysfs: /sys/class/touch/internal/errcnt */
ssize_t ist30xx_errcnt_store(struct device *dev, struct device_attribute *attr,
			     const char *buf, size_t size)
{
	int err_cnt;

	sscanf(buf, "%d", &err_cnt);

	if (err_cnt < 0)
		return size;

	DMSG("[ TSP ] Request reset error count: %d\n", err_cnt);

	ist30xx_max_error_cnt = err_cnt;

	return size;
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
static bool tunes_cmd_done;
static bool ist30xx_reg_mode;

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
		ret = ist30xx_cmd_run_device(ts_data->client);
		if (ret)
			goto regcmd_fail;

		/* enter reg access mode */
		ret = ist30xx_cmd_reg(ts_data->client, CMD_ENTER_REG_ACCESS);
		if (ret)
			goto regcmd_fail;

		ist30xx_reg_mode = true;

		break;
	case TUNES_CMD_REG_EXIT:
		/* exit reg access mode */
		ret = ist30xx_cmd_reg(ts_data->client, CMD_EXIT_REG_ACCESS);
		if (ret)
			goto regcmd_fail;

		ret = ist30xx_cmd_start_scan(ts_data->client);
		if (ret)
			goto regcmd_fail;

		ist30xx_reg_mode = false;

		ist30xx_enable_irq(ts_data);
		break;
	default:
		ist30xx_enable_irq(ts_data);
		return size;
	}
	tunes_cmd_done = true;

	return size;

regcmd_fail:
	pr_err("[ TSP ] Tunes regcmd i2c_fail, ret=%d\n", ret);
	return size;
}

ssize_t tunes_regcmd_show(struct device *dev, struct device_attribute *attr,
			  char *buf)
{
	int size;

	size = sprintf(buf, "cmd: 0x%02x, addr: 0x%08x, len: 0x%04x\n",
				ist30xx_tunes.cmd, ist30xx_tunes.addr,
				ist30xx_tunes.len);

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
		pr_err("[ TSP ] error, IST30XX_REG_CMD is not correct!\n");
		return size;
	}

	if (!ist30xx_reg_mode) {
		pr_err("[ TSP ] error, IST30XX_REG_CMD is not ready!\n");
		return size;
	}

	if (!tunes_cmd_done) {
		pr_err("[ TSP ] error, IST30XX_REG_CMD is not ready!\n");
		return size;
	}

	waddr = ist30xx_tunes.addr;
	if (ist30xx_tunes.len >= MAX_WRITE_LEN)
		len = MAX_WRITE_LEN;
	else
		len = ist30xx_tunes.len;

	while (wcnt < ist30xx_tunes.len) {
		ret = ist30xx_write_buf(ts_data->client, waddr, buf32, len);
		if (ret) {
			pr_err(
				"[ TSP ] Tunes regstore i2c_fail, ret=%d\n"
				, ret);
			return size;
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

	/* unsigned long flags; */

	if (ist30xx_tunes.cmd != TUNES_CMD_READ) {
		pr_err("[ TSP ] error, IST30XX_REG_CMD is not correct!\n");
		return 0;
	}

	if (!tunes_cmd_done) {
		pr_err("[ TSP ] error, IST30XX_REG_CMD is not ready!\n");
		return 0;
	}

	size = ist30xx_tunes.len;
	ret = ist30xx_write_cmd(ts_data->client, IST30XX_RX_CNT_ADDR, size);
	if (ret) {
		pr_err("[ TSP ] Tunes regshow i2c_fail, ret=%d\n", ret);
		return 0;
	}

	/* local_irq_save(flags);
	// Activated only when the GPIO I2C is used */
	ret = ist30xx_read_buf(ts_data->client,
			ist30xx_tunes.addr, buf32, size);
	/* local_irq_restore(flags);
	// Activated only when the GPIO I2C is used */
	if (ret) {
		pr_err("[ TSP ] Tunes regshow i2c_fail, ret=%d\n", ret);
		return size;
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

	sscanf(buf, "%x %x %x", &cmd, &addr, &len);

	switch (cmd) {
	case TUNES_CMD_WRITE:   /* write cmd */
		write_len = 0;
		ptr = (char *)(buf + 15);

		while (write_len < len) {
			memcpy(token, ptr, 8);
			token[8] = 0;
			val = simple_strtoul(token, &tmp, 16);
			ret = ist30xx_write_buf(ts_data->client, addr, &val, 1);
			if (ret) {
				pr_err(
					"[ TSP ] Tunes regstore i2c_fail, ret=%d\n"
					, ret);
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
		ist30xx_disable_irq(ts_data);
		ret = ist30xx_cmd_run_device(ts_data->client);
		if (ret < 0)
			goto cmd_fail;

		ret = ist30xx_cmd_reg(ts_data->client, CMD_ENTER_REG_ACCESS);
		if (ret < 0)
			goto cmd_fail;
		ist30xx_reg_mode = true;
		break;

	case TUNES_CMD_REG_EXIT:   /* exit */
		if (ist30xx_reg_mode == true) {
			ret = ist30xx_cmd_reg(ts_data->client,
					CMD_EXIT_REG_ACCESS);
			if (ret < 0)
				goto cmd_fail;

			ret = ist30xx_cmd_start_scan(ts_data->client);
			if (ret < 0)
				goto cmd_fail;
			ist30xx_reg_mode = false;
			ist30xx_enable_irq(ts_data);
		}
		break;

	default:
		break;
	}

	return size;

cmd_fail:
	pr_err("[ TSP ] Tunes adb i2c_fail\n");
	return size;
}

ssize_t tunes_adb_show(struct device *dev, struct device_attribute *attr,
	char *buf)
{
	int ret;
	int i, len, size = 0;
	char reg_val[10];

	/* unsigned long flags; */

	ret = ist30xx_write_cmd(ts_data->client,
			IST30XX_RX_CNT_ADDR, ist30xx_tunes.len);
	if (ret) {
		pr_err("[ TSP ] Tunes regshow i2c_fail, ret=%d\n", ret);
		return size;
	}

	/* local_irq_save(flags); */
	ret = ist30xx_read_buf(ts_data->client, ist30xx_tunes.addr,
			       ist30xx_frame_buf, ist30xx_tunes.len);
	/* local_irq_restore(flags); */
	if (ret) {
		pr_err("[ TSP ] Tunes regshow i2c_fail, ret=%d\n", ret);
		return size;
	}

	size = 0;
	buf[0] = 0;
	len = sprintf(reg_val, "%08x", ist30xx_tunes.addr);
	strcat(buf, reg_val);
	size += len;
	for (i = 0; i < ist30xx_tunes.len; i++) {
		len = sprintf(reg_val, "%08x", ist30xx_frame_buf[i]);
		strcat(buf, reg_val);
		size += len;
	}

	return size;
}
#endif


/* sysfs  */
static DEVICE_ATTR(mem, 0666, ist30xx_mem_show, ist30xx_mem_store);
static DEVICE_ATTR(refresh, 0666, ist30xx_frame_refresh, NULL);
static DEVICE_ATTR(filter, 0666, ist30xx_filter_show, NULL);
static DEVICE_ATTR(raw, 0666, ist30xx_raw_show, NULL);
static DEVICE_ATTR(base, 0666, ist30xx_base_show, NULL);
static DEVICE_ATTR(diff, 0666, ist30xx_diff_show, NULL);
static DEVICE_ATTR(clb, 0666, ist30xx_calib_show, ist30xx_calib_store);
static DEVICE_ATTR(debug, 0666, NULL, ist30xx_debug_store);
static DEVICE_ATTR(tsp_power, 0666, NULL, ist30xx_power_store);
static DEVICE_ATTR(errcnt, 0666, NULL, ist30xx_errcnt_store);

#if IST30XX_TUNES
static DEVICE_ATTR(regcmd, 0666, tunes_regcmd_show, tunes_regcmd_store);
static DEVICE_ATTR(reg, 0666, tunes_reg_show, tunes_reg_store);
static DEVICE_ATTR(tunes_fw, 0666, NULL, tunes_fw_store);
static DEVICE_ATTR(tunes_param, 0666, NULL, tunes_param_store);
static DEVICE_ATTR(adb, 0666, tunes_adb_show, tunes_adb_store);
#endif

extern struct class *ist30xx_class;
struct device *ist30xx_internal_dev;

int ist30xx_init_misc_sysfs(void)
{
	int ret;

	ret = ist30xx_tsp_update_info();
	if (ret)
		pr_err(
			"[ TSP ] failed, ist30xx_tsp_update_info(), ret=%d\n"
			, ret);
	ret = ist30xx_tkey_update_info();
	if (ret)
		pr_err(
			"[ TSP ] failed, st30xx_tkey_update_info(), ret=%d\n"
			, ret);

	/* /sys/class/touch/internal */
	ist30xx_internal_dev = device_create(ist30xx_class,
		NULL, 0, NULL, "internal");

	/* /sys/class/touch/internal/mem */
	if (device_create_file(ist30xx_internal_dev, &dev_attr_mem) < 0)
		pr_err("[ TSP ] Failed to create device file(%s)!\n",
		       dev_attr_mem.attr.name);

	/* /sys/class/touch/internal/refresh */
	if (device_create_file(ist30xx_internal_dev, &dev_attr_refresh) < 0)
		pr_err("[ TSP ] Failed to create device file(%s)!\n",
		       dev_attr_refresh.attr.name);

	/* /sys/class/touch/internal/filter */
	if (device_create_file(ist30xx_internal_dev, &dev_attr_filter) < 0)
		pr_err("[ TSP ] Failed to create device file(%s)!\n",
		       dev_attr_filter.attr.name);

	/* /sys/class/touch/internal/raw */
	if (device_create_file(ist30xx_internal_dev, &dev_attr_raw) < 0)
		pr_err("[ TSP ] Failed to create device file(%s)!\n",
		       dev_attr_raw.attr.name);

	/* /sys/class/touch/internal/base */
	if (device_create_file(ist30xx_internal_dev, &dev_attr_base) < 0)
		pr_err("[ TSP ] Failed to create device file(%s)!\n",
		       dev_attr_base.attr.name);

	/* /sys/class/touch/internal/diff */
	if (device_create_file(ist30xx_internal_dev, &dev_attr_diff) < 0)
		pr_err("[ TSP ] Failed to create device file(%s)!\n",
		       dev_attr_diff.attr.name);

	/* /sys/class/touch/internal/clb */
	if (device_create_file(ist30xx_internal_dev, &dev_attr_clb) < 0)
		pr_err("[ TSP ] Failed to create device file(%s)!\n",
		       dev_attr_clb.attr.name);

	/* /sys/class/touch/internal/debug */
	if (device_create_file(ist30xx_internal_dev, &dev_attr_debug) < 0)
		pr_err("[ TSP ] Failed to create device file(%s)!\n",
		       dev_attr_debug.attr.name);

	/* /sys/class/touch/internal/tsp_power */
	if (device_create_file(ist30xx_internal_dev, &dev_attr_tsp_power) < 0)
		pr_err("[ TSP ] Failed to create device file(%s)!\n",
		       dev_attr_tsp_power.attr.name);

	/* /sys/class/touch/internal/errcnt */
	if (device_create_file(ist30xx_internal_dev, &dev_attr_errcnt) < 0)
		pr_err("[ TSP ] Failed to create device file(%s)!\n",
		       dev_attr_errcnt.attr.name);

#if IST30XX_TUNES
	/* /sys/class/touch/tunes */
	ist30xx_tunes_dev = device_create(ist30xx_class,
		NULL, 0, NULL, "tunes");

	/* /sys/class/touch/tunes/regcmd */
	if (device_create_file(ist30xx_tunes_dev, &dev_attr_regcmd) < 0)
		pr_err("[ TSP ] Failed to create device file(%s)!\n",
		       dev_attr_regcmd.attr.name);

	/* /sys/class/touch/tunes/reg */
	if (device_create_file(ist30xx_tunes_dev, &dev_attr_reg) < 0)
		pr_err("[ TSP ] Failed to create device file(%s)!\n",
		       dev_attr_reg.attr.name);

	/* /sys/class/touch/tunes/firmware */
	if (device_create_file(ist30xx_tunes_dev, &dev_attr_tunes_fw) < 0)
		pr_err("[ TSP ] Failed to create device file(%s)!\n",
		       dev_attr_tunes_fw.attr.name);

	/* /sys/class/touch/tunes/param */
	if (device_create_file(ist30xx_tunes_dev, &dev_attr_tunes_param) < 0)
		pr_err("[ TSP ] Failed to create device file(%s)!\n",
		       dev_attr_tunes_param.attr.name);

	/* /sys/class/touch/tunes/adb */
	if (device_create_file(ist30xx_tunes_dev, &dev_attr_adb) < 0)
		pr_err("[ TSP ] Failed to create device file(%s)!\n",
		       dev_attr_adb.attr.name);
#endif

	ist30xx_frame_buf = kmalloc(4096, GFP_KERNEL);
	ist30xx_frame_rawbuf = kmalloc(4096, GFP_KERNEL);
	ist30xx_frame_fltbuf = kmalloc(4096, GFP_KERNEL);
	if (!ist30xx_frame_buf ||
		!ist30xx_frame_rawbuf || !ist30xx_frame_fltbuf)
		return -ENOMEM;

	return 0;
}
