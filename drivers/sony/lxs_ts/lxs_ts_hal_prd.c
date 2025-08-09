/*
 * SPDX-License-Identifier: GPL-2.0
 *
 * lxs_ts_hal_prd.c
 *
 * LXS touch raw-data debugging
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "lxs_ts_hal_prd.h"

static u32 t_prd_dbg_flag;

/* usage
 * (1) echo <value> > /sys/module/{Touch Module Name}/parameters/s_prd_dbg_flag
 * (2) insmod {Touch Module Name}.ko s_prd_dbg_flag=<value>
 */
module_param_named(s_prd_dbg_flag, t_prd_dbg_flag, int, S_IRUGO|S_IWUSR|S_IWGRP);

static u32 t_prd_dbg_mask;

/* usage
 * (1) echo <value> > /sys/module/{Touch Module Name}/parameters/s_prd_dbg_mask
 * (2) insmod {Touch Module Name}.ko s_prd_dbg_mask=<value>
 */
module_param_named(s_prd_dbg_mask, t_prd_dbg_mask, int, S_IRUGO|S_IWUSR|S_IWGRP);

static int prd_upd_swap_info(struct lxs_ts *ts);

static struct lxs_prd_data *__dev_get_prd(struct device *dev)
{
	return ((struct lxs_ts *)dev_get_drvdata(dev))->prd;
}

static int __get_digit_range_u16(uint16_t *buf, int size)
{
	int min = RAWDATA_MAX_DIGIT6;
	int max = RAWDATA_MIN_DIGIT6;
	int num;

	while (size--) {
		num = *buf++;

		if (num < min)
			min = num;

		if (num > max)
			max = num;
	}

	if ((max > RAWDATA_MAX_DIGIT5) || (min < RAWDATA_MIN_DIGIT5))
		return 6;

	if ((max > RAWDATA_MAX_DIGIT4) || (min < RAWDATA_MIN_DIGIT4))
		return 5;

	return DIGIT_RANGE_BASE;
}

static int __get_digit_range_s16(int16_t *buf, int size)
{
	int min = RAWDATA_MAX_DIGIT6;
	int max = RAWDATA_MIN_DIGIT6;
	int num;

	while (size--) {
		num = *buf++;

		if (num < min)
			min = num;

		if (num > max)
			max = num;
	}

	if ((max > RAWDATA_MAX_DIGIT5) || (min < RAWDATA_MIN_DIGIT5))
		return 6;

	if ((max > RAWDATA_MAX_DIGIT4) || (min < RAWDATA_MIN_DIGIT4))
		return 5;

	return DIGIT_RANGE_BASE;
}

static int __get_digit_range(void *buf, int size, int type)
{
	if (buf == NULL)
		return DIGIT_RANGE_BASE;

	if (type)
		return __get_digit_range_u16(buf, size);

	return __get_digit_range_s16(buf, size);
}

static int prd_check_exception(struct lxs_prd_data *prd)
{
	struct device *dev = prd->dev;
	struct lxs_ts *ts = dev_get_drvdata(dev);
	struct lxs_ts_chip *chip = &ts->chip;

	if (atomic_read(&chip->boot) == TC_IC_BOOT_FAIL) {
		t_prd_warn(prd, "%s\n", "boot failed");
		return 1;
	}

	if (atomic_read(&chip->init) != TC_IC_INIT_DONE) {
		t_prd_warn(prd, "%s\n", "not ready, need IC init");
		return 2;
	}

	if (ts->state_sleep != TS_IC_NORMAL) {
		t_prd_warn(prd, "%s\n", "not IC normal");
		return 3;
	}

	if (ts->chip.lcd_mode == LCD_MODE_STOP) {
		t_prd_warn(prd, "%s\n", "Touch is stopped");
		return 4;
	}

#if 0
	if (chip->fw.invalid_pid) {
		t_prd_warn(prd, "[drv] invalid PID - \"%s\" (%03Xh)\n",
			fw->product_id, fw->invalid_pid);
		return 5;
	}
#endif

	return 0;
}

static void __used prd_pin_reset(struct lxs_prd_data *prd, int delay)
{
	struct lxs_ts *ts = prd->ts;
	struct lxs_ts_chip *chip = &ts->chip;

	chip->driving_mode = LCD_MODE_STOP;

	lxs_ts_gpio_set_reset(ts, 0);
	lxs_ts_delay(chip->drv_reset_low);
	lxs_ts_gpio_set_reset(ts, 1);

	if (!delay)
		delay = ts->hw_reset_delay;

	lxs_ts_delay(delay);
}

static int __used prd_chip_reset(struct lxs_prd_data *prd)
{
	struct lxs_ts *ts = prd->ts;
	int ret;

//	mutex_lock(&ts->lock);
	ret = lxs_hal_reset(ts, TC_HW_RESET_SYNC, 0);
//	mutex_unlock(&ts->lock);

	return ret;
}

static int __used prd_chip_info(struct lxs_prd_data *prd)
{
	struct lxs_ts *ts = prd->ts;

	return lxs_hal_ic_info(ts);
}

static int __used prd_chip_driving(struct lxs_prd_data *prd, int mode)
{
	struct lxs_ts *ts = prd->ts;

	return lxs_hal_tc_driving(ts, mode);
}

static int prd_stop_firmware(struct lxs_prd_data *prd, struct lxs_prd_raw_ctrl *ctrl)
{
	struct device *dev = prd->dev;
	struct lxs_ts *ts = dev_get_drvdata(dev);
	struct lxs_ts_chip *chip = &ts->chip;
	struct lxs_hal_reg *reg = chip->reg;
	int is_fw_skip = !!(ctrl->flag & PRD_RAW_FLAG_FW_SKIP);
	int is_type_sys = !!(ctrl->flag & PRD_RAW_FLAG_TYPE_SYS);
	u32 addr = (is_type_sys) ? prd->sys_cmd : reg->prd_ic_ait_start_reg;
	u32 cmd = ctrl->cmd;
	u32 read_val;
	u32 check_data = 0;
	int try_cnt = 0;
	int ret = 0;

	if (is_fw_skip)
		return 0;

	if (lxs_addr_is_invalid(addr)) {
		if (is_type_sys) {
			t_prd_err(prd, "sys_cmd is invalid, %X\n", addr);
			return -EINVAL;
		}
		return 0;
	}

	/*
	 * STOP F/W to check
	 */
	ret = lxs_hal_write_value(ts, addr, cmd);
	if (ret < 0)
		goto out;

	ret = lxs_hal_read_value(ts, addr, &check_data);
	if (ret < 0)
		goto out;

	if (!prd->app_read)
		t_prd_info(prd, "check_data : %Xh\n", check_data);

	if (ctrl->flag & PRD_RAW_FLAG_FW_NO_STS) {
		lxs_ts_delay(10);
		return 0;
	}

	addr = reg->prd_ic_ait_data_readystatus;
	if (lxs_addr_is_invalid(addr))
		return 0;

	try_cnt = 1000;
	do {
		--try_cnt;
		if (try_cnt == 0) {
			t_prd_err(prd, "[ERR] stop FW: timeout, %08Xh\n", read_val);
			ret = -ETIMEDOUT;
			goto out;
		}
		lxs_hal_read_value(ts, addr, &read_val);

		if (!prd->app_read)
			t_prd_dbg_base(prd, "Read RS_IMAGE = [%x] , OK RS_IMAGE = [%x]\n",
				read_val, (u32)RS_IMAGE);

		lxs_ts_delay(2);
	} while (read_val != (u32)RS_IMAGE);

out:
	return ret;
}

static int prd_start_firmware(struct lxs_prd_data *prd, struct lxs_prd_raw_ctrl *ctrl)
{
	struct device *dev = prd->dev;
	struct lxs_ts *ts = dev_get_drvdata(dev);
	struct lxs_ts_chip *chip = &ts->chip;
	struct lxs_hal_reg *reg = chip->reg;
	u32 const cmd = PRD_CMD_NONE;
	int is_fw_skip = !!(ctrl->flag & PRD_RAW_FLAG_FW_SKIP);
	int is_type_sys = !!(ctrl->flag & PRD_RAW_FLAG_TYPE_SYS);
	u32 addr = (is_type_sys) ? prd->sys_cmd : reg->prd_ic_ait_start_reg;
	u32 check_data = 0;
	u32 read_val = 0;
	int ret = 0;

	if (is_fw_skip)
		return 0;

	if (lxs_addr_is_invalid(addr)) {
		if (is_type_sys) {
			t_prd_err(prd, "sys_cmd is invalid, %X\n", addr);
			return -EINVAL;
		}
		return 0;
	}

	/* Release F/W to operate */
	ret = lxs_hal_write_value(ts, addr, cmd);
	if (ret < 0)
		goto out;

	ret = lxs_hal_read_value(ts, addr, &check_data);
	if (ret < 0)
		goto out;

	if (!prd->app_read)
		t_prd_dbg_base(prd, "check_data : %Xh\n", check_data);

	if (ctrl->flag & PRD_RAW_FLAG_FW_NO_STS) {
		lxs_ts_delay(10);
		return 0;
	}

	addr = reg->prd_ic_ait_data_readystatus;
	if (lxs_addr_is_invalid(addr))
		return 0;

	//for test
	ret = lxs_hal_read_value(ts, addr, &read_val);
	if (ret < 0)
		goto out;

	t_prd_dbg_base(prd, "Read RS_IMAGE = [%x]\n", read_val);

out:
	return ret;
}

static void __used __buff_16to8(struct lxs_prd_data *prd, void *dst, void *src, int size)
{
	u16 *tbuf16 = NULL;
	u16 *sbuf16 = src;
	s8 *dbuf8 = dst;
	int i;

	if ((dst == NULL) || (src == NULL)) {
		t_prd_err(prd, "%s: invalid buffer: dst %p, src %p\n", __func__, dst, src);
		return;
	}

	if (dst == src) {
		tbuf16 = kzalloc(size<<1, GFP_KERNEL);
		if (tbuf16 == NULL) {
			t_prd_err(prd, "%s: failed to allocate tbuf\n", __func__);
			return;
		}

		memcpy(tbuf16, sbuf16, size<<1);
		sbuf16 = tbuf16;
	}

	for (i = 0 ; i < size; i++)
		*dbuf8++ = *sbuf16++;

	if (tbuf16 != NULL)
		kfree(tbuf16);
}

static void __used __buff_8to16(struct lxs_prd_data *prd, void *dst, void *src, int size)
{
	s8 *tbuf8 = NULL;
	s8 *sbuf8 = src;
	u16 *dbuf16 = dst;
	int i;

	if ((dst == NULL) || (src == NULL)) {
		t_prd_err(prd, "%s: invalid buffer: dst %p, src %p\n", __func__, dst, src);
		return;
	}

	if (dst == src) {
		tbuf8 = kzalloc(size, GFP_KERNEL);
		if (tbuf8 == NULL) {
			t_prd_err(prd, "%s: failed to allocate tbuf\n", __func__);
			return;
		}

		memcpy(tbuf8, sbuf8, size);
		sbuf8 = tbuf8;
	}

	for (i = 0 ; i < size; i++)
		*dbuf16++ = *sbuf8++;

	if (tbuf8 != NULL)
		kfree(tbuf8);
}

/*
 * Flip Combo : Horizontal & Vertical
 */
static void __used __buff_flip_c(struct lxs_prd_data *prd, void *dst, void *src, int row, int col, int data_fmt)
{
	u8 *tbuf8 = NULL;
	u16 *tbuf16 = NULL;
	u8 *dst8, *src8;
	u16 *dst16, *src16;
	int is_16bit = !!(data_fmt & PRD_DATA_FMT_MASK);
	int size;
	int i, j;

	if ((dst == NULL) || (src == NULL)) {
		t_prd_err(prd, "%s: invalid buffer: dst %p, src %p\n", __func__, dst, src);
		return;
	}

	size = (row * col);

	if (is_16bit) {
		dst16 = (u16 *)dst;
		src16 = (u16 *)src;

		if (dst == src) {
			tbuf16 = kzalloc(size<<1, GFP_KERNEL);
			if (tbuf16 == NULL) {
				t_prd_err(prd, "%s: failed to allocate tbuf6\n", __func__);
				return;
			}

			memcpy(tbuf16, src16, size<<1);
			src16 = tbuf16;
		}

		for (i = 0; i < row; i++) {
			for (j = 0; j < col; j++) {
				dst16[(i * col) + (col - j - 1)] = src16[((row - i - 1) * col) + j];
			}
		}

		if (tbuf16 != NULL) {
			kfree(tbuf16);
		}
	} else {
		dst8 = (u8 *)dst;
		src8 = (u8 *)src;

		if (dst == src) {
			tbuf8 = kzalloc(size, GFP_KERNEL);
			if (tbuf8 == NULL) {
				t_prd_err(prd, "%s: failed to allocate tbuf8\n", __func__);
				return;
			}

			memcpy(tbuf8, src8, size);
			src8 = tbuf8;
		}

		for (i = 0; i < row; i++) {
			for (j = 0; j < col; j++) {
				dst8[(i * col) + (col - j - 1)] = src8[((row - i - 1) * col) + j];
			}
		}

		if (tbuf8 != NULL) {
			kfree(tbuf8);
		}
	}
}

/*
 * Flip Horizontal
 */
static void __used __buff_flip_h(struct lxs_prd_data *prd, void *dst, void *src, int row, int col, int data_fmt)
{
	u8 *tbuf8 = NULL;
	u16 *tbuf16 = NULL;
	u8 *dst8, *src8;
	u16 *dst16, *src16;
	int is_16bit = !!(data_fmt & PRD_DATA_FMT_MASK);
	int size;
	int i, j;

	if ((dst == NULL) || (src == NULL)) {
		t_prd_err(prd, "%s: invalid buffer: dst %p, src %p\n", __func__, dst, src);
		return;
	}

	size = (row * col);

	if (is_16bit) {
		u16 *srcl, *srcr;
		u16 *dstl, *dstr;

		dst16 = (u16 *)dst;
		src16 = (u16 *)src;

		if (dst == src) {
			tbuf16 = kzalloc(size<<1, GFP_KERNEL);
			if (tbuf16 == NULL) {
				t_prd_err(prd, "%s: failed to allocate tbuf6\n", __func__);
				return;
			}

			memcpy(tbuf16, src16, size<<1);
			src16 = tbuf16;
		}

		for (i = 0; i < row; i++) {
			srcl = &src16[(i * col)];
			srcr = &src16[(i * col) + col - 1];

			dstl = &dst16[(i * col)];
			dstr = &dst16[(i * col) + col - 1];
			for (j = 0; j < (col>>1); j++) {
				*dstl++ = *srcr--;
				*dstr-- = *srcl++;
			}
		}

		if (tbuf16 != NULL) {
			kfree(tbuf16);
		}
	} else {
		u8 *srcl, *srcr;
		u8 *dstl, *dstr;

		dst8 = (u8 *)dst;
		src8 = (u8 *)src;

		if (dst == src) {
			tbuf8 = kzalloc(size, GFP_KERNEL);
			if (tbuf8 == NULL) {
				t_prd_err(prd, "%s: failed to allocate tbuf8\n", __func__);
				return;
			}

			memcpy(tbuf8, src8, size);
			src8 = tbuf8;
		}

		for (i = 0; i < row; i++) {
			srcl = &src8[(i * col)];
			srcr = &src8[(i * col) + col - 1];

			dstl = &dst8[(i * col)];
			dstr = &dst8[(i * col) + col - 1];
			for (j = 0; j < (col>>1); j++) {
				*dstl++ = *srcr--;
				*dstr-- = *srcl++;
			}
		}

		if (tbuf8 != NULL) {
			kfree(tbuf8);
		}
	}
}

/*
 * Flip Vertical
 */
static void __used __buff_flip_v(struct lxs_prd_data *prd, void *dst, void *src, int row, int col, int data_fmt)
{
	u8 *tbuf8 = NULL;
	u16 *tbuf16 = NULL;
	u8 *dst8, *src8;
	u16 *dst16, *src16;
	int is_16bit = !!(data_fmt & PRD_DATA_FMT_MASK);
	int size;
	int i, j;

	if ((dst == NULL) || (src == NULL)) {
		t_prd_err(prd, "%s: invalid buffer: dst %p, src %p\n", __func__, dst, src);
		return;
	}

	size = (row * col);

	if (is_16bit) {
		u16 *srcl, *srch;
		u16 *dstl, *dsth;

		dst16 = (u16 *)dst;
		src16 = (u16 *)src;

		if (dst == src) {
			tbuf16 = kzalloc(size<<1, GFP_KERNEL);
			if (tbuf16 == NULL) {
				t_prd_err(prd, "%s: failed to allocate tbuf6\n", __func__);
				return;
			}

			memcpy(tbuf16, src16, size<<1);
			src16 = tbuf16;
		}

		for (i = 0; i < (row>>1); i++) {
			srcl = &src16[(i * col)];
			srch = &src16[((row - i - 1) * col)];

			dstl = &dst16[(i * col)];
			dsth = &dst16[((row - i - 1) * col)];
			for (j = 0; j < col; j++) {
				*dstl++ = *srch++;
				*dsth++ = *srcl++;
			}
		}

		if (tbuf16 != NULL) {
			kfree(tbuf16);
		}
	} else {
		u8 *srcl, *srch;
		u8 *dstl, *dsth;

		dst8 = (u8 *)dst;
		src8 = (u8 *)src;

		if (dst == src) {
			tbuf8 = kzalloc(size, GFP_KERNEL);
			if (tbuf8 == NULL) {
				t_prd_err(prd, "%s: failed to allocate tbuf8\n", __func__);
				return;
			}

			memcpy(tbuf8, src8, size);
			src8 = tbuf8;
		}

		for (i = 0; i < (row>>1); i++) {
			srcl = &src8[(i * col)];
			srch = &src8[((row - i - 1) * col)];

			dstl = &dst8[(i * col)];
			dsth = &dst8[((row - i - 1) * col)];
			for (j = 0; j < col; j++) {
				*dstl++ = *srch++;
				*dsth++ = *srcl++;
			}
		}

		if (tbuf8 != NULL) {
			kfree(tbuf8);
		}
	}
}

/*
 * @reverse
 * 0 : for 'Left-Top to Left-Bottom'
 * 1 : for 'Right-Bottom to Right-Top'
 * 2 : for 'Left-Bottom to Left-Top'
 * 3 : for 'Right-Top to Right-Bottom'
 */
static void __used __buff_swap(struct lxs_prd_data *prd, void *dst, void *src,
						int row, int col, int data_fmt, int reverse)
{
	u8 *tbuf8 = NULL;
	u16 *tbuf16 = NULL;
	u8 *dst8, *src8;
	u16 *dst16, *src16;
	int is_16bit = !!(data_fmt & PRD_DATA_FMT_MASK);
	int size;
	int r, c;
	int i;

	if (reverse < 0)
		return;

	if ((dst == NULL) || (src == NULL)) {
		t_prd_err(prd, "%s: invalid buffer: dst %p, src %p\n", __func__, dst, src);
		return;
	}

	size = (row * col);

	if (is_16bit) {
		dst16 = (reverse & 0x1) ? &((u16 *)dst)[size - 1] : (u16 *)dst;
		src16 = (u16 *)src;

		if (dst == src) {
			tbuf16 = kzalloc(size<<1, GFP_KERNEL);
			if (tbuf16 == NULL) {
				t_prd_err(prd, "%s: failed to allocate tbuf6\n", __func__);
				return;
			}

			memcpy(tbuf16, src16, size<<1);
			src16 = tbuf16;
		}

		switch (reverse) {
		case 3:
			for (i = 0; i < size; i++) {
				r = i / col;
				c = i % col;
				*dst16-- = src16[(c * row) + (row - r - 1)];
			}
			break;
		case 2:
			for (i = 0; i < size; i++) {
				r = i / col;
				c = i % col;
				*dst16++ = src16[(c * row) + (row - r - 1)];
			}
			break;
		case 1:
			for (i = 0; i < size; i++) {
				r = i / col;
				c = i % col;
				*dst16-- = src16[r + (row * c)];
			}
			break;
		default:
			for (i = 0; i < size; i++) {
				r = i / col;
				c = i % col;
				*dst16++ = src16[r + (row * c)];
			}
			break;
		}

		if (tbuf16 != NULL) {
			kfree(tbuf16);
		}
	} else {
		dst8 = (reverse & 0x1) ? &((u8 *)dst)[size - 1] : (u8 *)dst;
		src8 = (u8 *)src;

		if (dst == src) {
			tbuf8 = kzalloc(size, GFP_KERNEL);
			if (tbuf8 == NULL) {
				t_prd_err(prd, "%s: failed to allocate tbuf8\n", __func__);
				return;
			}

			memcpy(tbuf8, src8, size);
			src8 = tbuf8;
		}

		switch (reverse) {
		case 3:
			for (i = 0; i < size; i++) {
				r = i / col;
				c = i % col;
				*dst8-- = src8[(c * row) + (row - r - 1)];
			}
			break;
		case 2:
			for (i = 0; i < size; i++) {
				r = i / col;
				c = i % col;
				*dst8++ = src8[(c * row) + (row - r - 1)];
			}
			break;
		case 1:
			for (i = 0; i < size; i++) {
				r = i / col;
				c = i % col;
				*dst8-- = src8[r + (row * c)];
			}
			break;
		default:
			for (i = 0; i < size; i++) {
				r = i / col;
				c = i % col;
				*dst8++ = src8[r + (row * c)];
			}
			break;
		}

		if (tbuf8 != NULL) {
			kfree(tbuf8);
		}
	}
}

static int __cal_data_size(u32 row_size, u32 col_size, u32 data_fmt)
{
	int data_size = 0;

	if (data_fmt & PRD_DATA_FMT_RXTX)
		data_size = row_size + col_size;
	else
		data_size = row_size * col_size;

	if (data_fmt & PRD_DATA_FMT_SELF)
		data_size += (row_size + col_size);

	if (data_fmt & PRD_DATA_FMT_MASK)	//if 16bit
		data_size <<= 1;

	return data_size;
}

static int prd_read_memory(struct lxs_prd_data *prd, u32 offset, char *buf, int size)
{
	struct device *dev = prd->dev;
	struct lxs_ts *ts = dev_get_drvdata(dev);
	struct lxs_ts_chip *chip = &ts->chip;
	struct lxs_hal_reg *reg = chip->reg;
	int ret = 0;

	if (!offset) {
		t_prd_err(prd, "%s\n", "raw memory failed: zero offset");
		ret = -EFAULT;
		goto out;
	}

	if (!size) {
		t_prd_err(prd, "%s\n", "raw memory failed: zero size");
		ret = -EFAULT;
		goto out;
	}

	if (buf == NULL) {
		t_prd_err(prd, "%s\n", "raw memory failed: NULL buf");
		ret = -EFAULT;
		goto out;
	}

	//offset write
	ret = lxs_hal_write_value(ts, reg->serial_data_offset, offset);
	if (ret < 0)
		goto out;

	//read raw data
	ret = lxs_hal_reg_read(ts, reg->data_base_addr, buf, size);
	if (ret < 0)
		goto out;

out:
	return ret;
}

static int prd_print_pre(struct lxs_prd_data *prd, char *prt_buf,
				int prt_size, int row_size, int col_size,
				char *name, int digit_range, int is_self)
{
	char *log_line = prd->log_line;
	const char *raw_fmt_col_pre = NULL;
	const char *raw_fmt_col_no = NULL;
	int log_size = 0;
	int i;

	if (is_self) {
		switch (digit_range) {
		case 6:
			raw_fmt_col_pre = "   :   [SF]   ";
			raw_fmt_col_no = "[%2d]   ";
			break;
		case 5:
			raw_fmt_col_pre = "   :  [SF]  ";
			raw_fmt_col_no = "[%2d]  ";
			break;
		default:
			raw_fmt_col_pre = "   : [SF] ";
			raw_fmt_col_no = "[%2d] ";
			break;
		}
	} else {
		switch (digit_range) {
		case 6:
			raw_fmt_col_pre = "   :   ";
			raw_fmt_col_no = "[%2d]   ";
			break;
		case 5:
			raw_fmt_col_pre = "   :  ";
			raw_fmt_col_no = "[%2d]  ";
			break;
		default:
			raw_fmt_col_pre = "   : ";
			raw_fmt_col_no = "[%2d] ";
			break;
		}
	}

	log_size += lxs_prd_log_buf_snprintf(log_line, log_size,
					"-------- %s(%d %d) --------",
					name, row_size, col_size);

	prt_size += lxs_prd_log_flush(prd, log_line, prt_buf, log_size, prt_size);

	log_size = 0;
	log_size += lxs_prd_log_buf_snprintf(log_line, log_size, "%s", raw_fmt_col_pre);

	for (i = 0; i < col_size; i++)
		log_size += lxs_prd_log_buf_snprintf(log_line, log_size, raw_fmt_col_no, i);

	prt_size += lxs_prd_log_flush(prd, log_line, prt_buf, log_size, prt_size);

	return prt_size;
}

static int prd_print_min_max(struct lxs_prd_data *prd, char *prt_buf,
				int prt_size, int min, int max, char *name)
{
	t_prd_info(prd, "%s min %d, max %d\n", name, min, max);

	if (prd->dbg_mask & BIT(8)) {
		prt_size += lxs_prd_buf_snprintf(prt_buf, prt_size,
					"\n%s min %d, max %d\n",
					name, min, max);
	}

	return prt_size;
}

/*
 * Default - Row : Tx, Col : Rx
 *
 * [Non-Self format]
 *    : [ 0] [ 1] [ 2] ... [cc]
 * [ 0] xxxx xxxx xxxx ... xxxx
 * [ 1] xxxx xxxx xxxx ... xxxx
 * ...
 * [rr] xxxx xxxx xxxx ... xxxx
 *
 *
 *
 * [Self format]
 *	  : [SF] [ 0] [ 1] [ 2] ... [cc]
 * [SF]      xxxx xxxx xxxx ... xxxx
 * [ 0] xxxx xxxx xxxx xxxx ... xxxx
 * [ 1] xxxx xxxx xxxx xxxx ... xxxx
 * ...
 * [rr] xxxx xxxx xxxx xxxx ... xxxx
 *
 * cc : col_size - 1
 * rr : row_size - 1
 * SF : SELF line
 *
 */
static int prd_print_rawdata(struct lxs_prd_data *prd, char *prt_buf,
			int prt_size, char *raw_buf, int row_size, int col_size,
			int data_fmt, u32 flag, char *name, int post)
{
	const char *raw_fmt_row_no = NULL;
	const char *raw_fmt_empty = NULL;
	const char *raw_fmt = NULL;
	char *log_line = prd->log_line;
	void *rawdata_buf = (raw_buf) ? raw_buf : prd->raw_buf;
	uint16_t *rawdata_u16 = (uint16_t *)rawdata_buf;
	int16_t *rawdata_s16 = (int16_t *)rawdata_buf;
	s8 *rawdata_s8 = (s8 *)rawdata_buf;
	uint16_t *buf_self_u16 = NULL;
	int16_t *buf_self_s16 = NULL;
	s8 *buf_self_s8 = NULL;
	int is_self = !!(data_fmt & PRD_DATA_FMT_SELF);
	int is_16bit = !!(data_fmt & PRD_DATA_FMT_MASK);
	int is_u16bit = !!(data_fmt & PRD_DATA_FMT_U16);
	int is_self_misalign = !!(data_fmt & PRD_DATA_FMT_SELF_MISALIGN);
	int is_self_rev_c = !!(data_fmt & PRD_DATA_FMT_SELF_REV_C);
	int is_self_rev_r = !!(data_fmt & PRD_DATA_FMT_SELF_REV_R);
	int is_ign_edge_t = !!(flag & PRD_RAW_FLAG_IGN_EDGE_T);
	int is_ign_edge_b = !!(flag & PRD_RAW_FLAG_IGN_EDGE_B);
	int is_ign_row_t = !!(flag & PRD_RAW_FLAG_IGN_ROW_T);
	int is_ign_row_b = !!(flag & PRD_RAW_FLAG_IGN_ROW_B);
	int is_ign_col_r = !!(flag & PRD_RAW_FLAG_IGN_COL_R);
	int is_ign_col_l = !!(flag & PRD_RAW_FLAG_IGN_COL_L);
	int digit_range = DIGIT_RANGE_BASE;
	int digit_width_self = DIGIT_RANGE_BASE;
	int self_offset_col = 0;
	int self_offset_row = col_size;
	int self_step_col = 1;
	int self_step_row = 1;
	int min_raw = PRD_RAWDATA_MAX;
	int max_raw = PRD_RAWDATA_MIN;
	int log_size = 0;
	int curr_raw;
	int i, j;

	if (is_self) {
		buf_self_u16 = &rawdata_u16[row_size * col_size];
		buf_self_s16 = &rawdata_s16[row_size * col_size];
		buf_self_s8 = &rawdata_s8[row_size * col_size];

		if (is_self_misalign) {
			self_offset_col = row_size;
			self_offset_row = 0;
		}

		if (is_self_rev_c) {
			self_offset_col += (col_size - 1);
			self_step_col = -1;
		}

		if (is_self_rev_r) {
			self_offset_row += (row_size - 1);
			self_step_row = -1;
		}
	}

	if (is_16bit) {
		digit_range = __get_digit_range(rawdata_buf, row_size * col_size, is_u16bit);

		if (is_self) {
			digit_width_self = __get_digit_range((is_u16bit) ? (void *)buf_self_u16 : (void *)buf_self_s16, row_size + col_size, is_u16bit);
			if (digit_width_self > digit_range)
				digit_range = digit_width_self;
		}
	}

	raw_fmt_row_no = "[%2d] ";

	switch (digit_range) {
	case 6:
		raw_fmt_empty = "       ";
		raw_fmt = "%6d ";
		break;
	case 5:
		raw_fmt_empty = "      ";
		raw_fmt = "%5d ";
		break;
	default:
		raw_fmt_empty = "     ";
		raw_fmt = "%4d ";
		break;
	}

	prt_size = prd_print_pre(prd, prt_buf, prt_size, row_size, col_size, name, digit_range, is_self);

	if (is_self) {
		/* self data - horizental line */
		log_size = 0;
		memset(log_line, 0, sizeof(prd->log_line));

		log_size += lxs_prd_log_buf_snprintf(log_line, log_size, "[SF] %s", raw_fmt_empty);

		for (i = 0; i < col_size; i++) {
			if (is_16bit)
				curr_raw = (is_u16bit) ? buf_self_u16[self_offset_col] : buf_self_s16[self_offset_col];
			else
				curr_raw = buf_self_s8[self_offset_col];

			self_offset_col += self_step_col;

			log_size += lxs_prd_log_buf_snprintf(log_line, log_size, raw_fmt, curr_raw);
		}

		prt_size += lxs_prd_log_flush(prd, log_line, prt_buf, log_size, prt_size);
	}

	for (i = 0; i < row_size; i++) {
		log_size = 0;
		memset(log_line, 0, sizeof(prd->log_line));
		log_size += lxs_prd_log_buf_snprintf(log_line, log_size, raw_fmt_row_no, i);

		if (is_self) {
			/* self data : vertical line */
			if (is_16bit)
				curr_raw = (is_u16bit) ? buf_self_u16[self_offset_row] : buf_self_s16[self_offset_row];
			else
				curr_raw = buf_self_s8[self_offset_row];

			self_offset_row += self_step_row;

			log_size += lxs_prd_log_buf_snprintf(log_line, log_size, raw_fmt, curr_raw);
		}

		for (j = 0; j < col_size; j++) {
			/* rawdata */
			if (is_16bit)
				curr_raw = (is_u16bit) ? *rawdata_u16++ : *rawdata_s16++;
			else
				curr_raw = *rawdata_s8++;

			log_size += lxs_prd_log_buf_snprintf(log_line, log_size, raw_fmt, curr_raw);

			if (is_ign_col_l && !j)
				continue;

			if (is_ign_col_r && (j == (col_size - 1)))
				continue;

			if (is_ign_row_t && !i)
				continue;
			else if (is_ign_edge_t && !i)
				if (!j || (j == (col_size-1)))
					continue;

			if (is_ign_row_b && (i == (row_size - 1)))
				continue;
			else if (is_ign_edge_b && (i == (row_size - 1)))
				if (!j || (j == (col_size-1)))
					continue;

			if (curr_raw < min_raw)
				min_raw = curr_raw;

			if (curr_raw > max_raw)
				max_raw = curr_raw;
		}

		prt_size += lxs_prd_log_flush(prd, log_line, prt_buf, log_size, prt_size);
	}

	if (post)
		prt_size = prd_print_min_max(prd, prt_buf, prt_size, min_raw, max_raw, name);

	return prt_size;
}

static int prd_print_data_pre(struct lxs_prd_data *prd, char *prt_buf,
				int prt_size, int row_size, int col_size,
				char *name, int digit_range, int is_data)
{
	char *log_line = prd->log_line;
	const char *raw_fmt_col_pre = NULL;
	const char *raw_fmt = NULL;
	int log_size = 0;
	int c_size = (is_data) ? col_size : max(row_size, col_size);
	int i;

	switch (digit_range) {
	case 6:
		raw_fmt_col_pre = "     :   ";
		raw_fmt = "[%2d]   ";
		break;
	case 5:
		raw_fmt_col_pre = "     :  ";
		raw_fmt = "[%2d]  ";
		break;
	default:
		raw_fmt_col_pre = "     : ";
		raw_fmt = "[%2d] ";
		break;
	}

	log_size += lxs_prd_log_buf_snprintf(log_line, log_size,
					"-------- %s(%d %d) --------",
					name, row_size, col_size);

	prt_size += lxs_prd_log_flush(prd, log_line, prt_buf, log_size, prt_size);

	log_size = 0;
	if (is_data)
		log_size += lxs_prd_log_buf_snprintf(log_line, log_size, "  ");

	log_size += lxs_prd_log_buf_snprintf(log_line, log_size, "%s", raw_fmt_col_pre);

	for (i = 0; i < c_size; i++)
		log_size += lxs_prd_log_buf_snprintf(log_line, log_size, raw_fmt, i);

	prt_size += lxs_prd_log_flush(prd, log_line, prt_buf, log_size, prt_size);

	return prt_size;
}

/*
* Base - Row : Tx, Col : Rx, Rx first
 *
 *      : [ 0] [ 1] [ 2] ... [cc]
 * Rx[rr] xxxx xxxx xxxx ... xxxx //col_size
 * Tx[tt] xxxx xxxx xxxx ...      //row_size
 *
 * if is_self_tx_first,
 *      : [ 0] [ 1] [ 2] ... [cc]
 * Tx[tt] xxxx xxxx xxxx ...      //row_size
 * Rx[rr] xxxx xxxx xxxx ...      //col_size
 *
 */
static int prd_print_data_rxtx(struct lxs_prd_data *prd, char *prt_buf,
			int prt_size, char *raw_buf, int row_size, int col_size,
			int data_fmt, u32 flag, char *name)
{
	const char *raw_fmt = NULL;
	char *log_line = prd->log_line;
	void *rawdata_buf = (raw_buf) ? raw_buf : prd->raw_buf;
	uint16_t *rawdata_u16 = (uint16_t *)rawdata_buf;
	int16_t *rawdata_s16 = (int16_t *)rawdata_buf;
	s8 *rawdata_s8 = (s8 *)rawdata_buf;
	char *name1 = NULL;
	char *name2 = NULL;
	int is_16bit = !!(data_fmt & PRD_DATA_FMT_MASK);
	int is_u16bit = !!(data_fmt & PRD_DATA_FMT_U16);
	int is_self_tx_first = !!(data_fmt & PRD_DATA_FMT_SELF_TX_FIRST);
	int digit_range = DIGIT_RANGE_BASE;
	int line1_size = col_size;
	int line2_size = row_size;
	int log_size = 0;
	int curr_raw;
	int j;

	name1 = (is_self_tx_first) ? "Tx" : "Rx";
	name2 = (is_self_tx_first) ? "Rx" : "Tx";

	/*
	 * row/col swap
	 */
	if (is_self_tx_first) {
		line1_size = row_size;
		line2_size = col_size;
	}

	if (is_16bit)
		digit_range = __get_digit_range(rawdata_s16, row_size + col_size, is_u16bit);

	switch (digit_range) {
	case 6:
		raw_fmt = "%6d ";
		break;
	case 5:
		raw_fmt = "%5d ";
		break;
	default:
		raw_fmt = "%4d ";
		break;
	}

	prt_size = prd_print_data_pre(prd, prt_buf, prt_size, row_size, col_size, name, digit_range, 0);

	log_size = 0;
	memset(log_line, 0, sizeof(prd->log_line));

	log_size += lxs_prd_log_buf_snprintf(log_line, log_size, "%s[%2d] ", name1, line1_size);

	for (j = 0; j < line1_size; j++) {
		if (is_16bit)
			curr_raw = (is_u16bit) ? *rawdata_u16++ : *rawdata_s16++;
		else
			curr_raw = *rawdata_s8++;

		log_size += lxs_prd_log_buf_snprintf(log_line, log_size, raw_fmt, curr_raw);
	}

	prt_size += lxs_prd_log_flush(prd, log_line, prt_buf, log_size, prt_size);

	log_size = 0;
	memset(log_line, 0, sizeof(prd->log_line));

	log_size += lxs_prd_log_buf_snprintf(log_line, log_size, "%s[%2d] ", name2, line2_size);

	for (j = 0; j < line2_size; j++) {
		if (is_16bit)
			curr_raw = (is_u16bit) ? *rawdata_u16++ : *rawdata_s16++;
		else
			curr_raw = *rawdata_s8++;

		log_size += lxs_prd_log_buf_snprintf(log_line, log_size, raw_fmt, curr_raw);
	}

	prt_size += lxs_prd_log_flush(prd, log_line, prt_buf, log_size, prt_size);

	log_size = 0;
	memset(log_line, 0, sizeof(prd->log_line));

	return prt_size;
}

/*
 *        : [ 0] [ 1] [ 2] ... [cc]
 * Data[ 0] xxxx xxxx xxxx ... xxxx
 * Data[rr] xxxx xxxx xxxx ... xxxx
 *
 * cc : col_size - 1
 * rr : col_size
 *
 */
static int prd_print_data_raw(struct lxs_prd_data *prd, char *prt_buf,
			int prt_size, char *raw_buf, int row_size, int col_size,
			int data_fmt, u32 flag, char *name)
{
	const char *raw_fmt = NULL;
	char *log_line = prd->log_line;
	void *rawdata_buf = (raw_buf) ? raw_buf : prd->raw_buf;
	uint16_t *rawdata_u16 = (uint16_t *)rawdata_buf;
	int16_t *rawdata_s16 = (int16_t *)rawdata_buf;
	s8 *rawdata_s8 = (s8 *)rawdata_buf;
	int is_16bit = !!(data_fmt & PRD_DATA_FMT_MASK);
	int is_u16bit = !!(data_fmt & PRD_DATA_FMT_U16);
	int digit_range = DIGIT_RANGE_BASE;
	int log_size = 0;
	int curr_raw;
	int offset;
	int i, j;

	if (is_16bit)
		digit_range = __get_digit_range(rawdata_s16, row_size + col_size, is_u16bit);

	switch (digit_range) {
	case 6:
		raw_fmt = "%6d ";
		break;
	case 5:
		raw_fmt = "%5d ";
		break;
	default:
		raw_fmt = "%4d ";
		break;
	}

	prt_size = prd_print_data_pre(prd, prt_buf, prt_size, row_size, col_size, name, digit_range, 1);

	offset = 0;
	for (i = 0; i < row_size; i++) {
		log_size = 0;
		memset(log_line, 0, sizeof(prd->log_line));

		log_size += lxs_prd_log_buf_snprintf(log_line,
						log_size, "Data[%2d] ", offset);

		for (j = 0; j < col_size; j++) {
			if (is_16bit)
				curr_raw = (is_u16bit) ? *rawdata_u16++ : *rawdata_s16++;
			else
				curr_raw = *rawdata_s8++;

			log_size += lxs_prd_log_buf_snprintf(log_line, log_size, raw_fmt, curr_raw);
		}

		prt_size += lxs_prd_log_flush(prd, log_line, prt_buf, log_size, prt_size);

		offset += col_size;
	}

	return prt_size;
}

static int __used prd_prt_rawdata(void *prd_data, void *ctrl_data, int prt_size)
{
	struct lxs_prd_data *prd = (struct lxs_prd_data *)prd_data;
	struct lxs_prd_raw_ctrl *ctrl = (struct lxs_prd_raw_ctrl *)ctrl_data;
	int ret;

	ret = prd_print_rawdata(prd, ctrl->prt_buf, prt_size,
			ctrl->raw_buf, ctrl->row_act, ctrl->col_act,
			ctrl->data_fmt, ctrl->flag, ctrl->name, 1);

	return ret;
}

static int __used prd_prt_pen_rxtx(void *prd_data, void *ctrl_data, int prt_size)
{
	struct lxs_prd_data *prd = (struct lxs_prd_data *)prd_data;
	struct lxs_prd_raw_ctrl *ctrl = (struct lxs_prd_raw_ctrl *)ctrl_data;
	int ret;

	ret = prd_print_data_rxtx(prd, ctrl->prt_buf, prt_size,
			ctrl->raw_buf, ctrl->row_act, ctrl->col_act,
			ctrl->data_fmt, ctrl->flag, ctrl->name);

	return ret;
}

static int __used prd_prt_pen_data(void *prd_data, void *ctrl_data, int prt_size)
{
	struct lxs_prd_data *prd = (struct lxs_prd_data *)prd_data;
	struct lxs_prd_raw_ctrl *ctrl = (struct lxs_prd_raw_ctrl *)ctrl_data;

	int ret;

	ret = prd_print_data_raw(prd, ctrl->prt_buf, prt_size,
			ctrl->raw_buf, ctrl->row_act, ctrl->col_act,
			ctrl->data_fmt, ctrl->flag, ctrl->name);

	return ret;
}

static void prd_mod_self_adj(struct lxs_prd_data *prd,
			char *raw_buf, int row_size, int col_size,
			int data_fmt, u32 flag, int is_get, int reverse)
{
	void *raw_c = (reverse) ? prd->raw_r : prd->raw_c;
	void *raw_r = (reverse) ? prd->raw_c : prd->raw_r;
	uint16_t *buf_self_u16 = &((uint16_t *)raw_buf)[row_size * col_size];
	int16_t *buf_self_s16 = &((int16_t *)raw_buf)[row_size * col_size];
	s8 *buf_self_s8 = &((s8 *)raw_buf)[row_size * col_size];
	uint16_t *buf_col_u16 = raw_c;
	uint16_t *buf_row_u16 = raw_r;
	int16_t *buf_col_s16 = raw_c;
	int16_t *buf_row_s16 = raw_r;
	s8 *buf_col_s8 = raw_c;
	s8 *buf_row_s8 = raw_r;
//	int is_self = !!(data_fmt & PRD_DATA_FMT_SELF);
	int is_16bit = !!(data_fmt & PRD_DATA_FMT_MASK);
	int is_u16bit = !!(data_fmt & PRD_DATA_FMT_U16);
	int is_self_misalign = !!(data_fmt & PRD_DATA_FMT_SELF_MISALIGN);
	int is_self_rev_c = !!(data_fmt & PRD_DATA_FMT_SELF_REV_C);
	int is_self_rev_r = !!(data_fmt & PRD_DATA_FMT_SELF_REV_R);
	int self_offset_col = 0;
	int self_offset_row = col_size;
	int self_step_col = 1;
	int self_step_row = 1;
	int i;

	t_prd_dbg_base(prd, "%s: row %d, col %d, fmt 0x%X, flag 0x%X, get %d\n",
		__func__, row_size, col_size, data_fmt, flag, is_get);

	if (!raw_buf || !row_size || !col_size)
		return;

	if (is_self_misalign) {
		self_offset_col = row_size;
		self_offset_row = 0;
	}

	if (is_self_rev_c) {
		self_offset_col += (col_size - 1);
		self_step_col = -1;
	}

	if (is_self_rev_r) {
		self_offset_row += (row_size - 1);
		self_step_row = -1;
	}

	if (is_get) {
		/* get */
		for (i = 0; i < col_size; i++) {
			if (is_16bit) {
				if (is_u16bit)
					*buf_col_u16++ = buf_self_u16[self_offset_col];
				else
					*buf_col_s16++ = buf_self_s16[self_offset_col];
			} else
				*buf_col_s8++ = buf_self_s8[self_offset_col];

			self_offset_col += self_step_col;
		}

		for (i = 0; i < row_size; i++) {
			if (is_16bit) {
				if (is_u16bit)
					*buf_row_u16++ = buf_self_u16[self_offset_row];
				else
					*buf_row_s16++ = buf_self_s16[self_offset_row];
			} else
				*buf_row_s8++ = buf_self_s8[self_offset_row];

			self_offset_row += self_step_row;
		}
	}else {
		/* put */
		for (i = 0; i < col_size; i++) {
			if (is_16bit) {
				if (is_u16bit)
					buf_self_u16[self_offset_col] = *buf_col_u16++;
				else
					buf_self_s16[self_offset_col] = *buf_col_s16++;
			} else
				buf_self_s8[self_offset_col] = *buf_col_s8++;

			self_offset_col += self_step_col;
		}

		for (i = 0; i < row_size; i++) {
			if (is_16bit) {
				if (is_u16bit)
					buf_self_u16[self_offset_row] = *buf_row_u16++;
				else
					buf_self_s16[self_offset_row] = *buf_row_s16++;
			} else
				buf_self_s8[self_offset_row] = *buf_row_s8++;

			self_offset_row += self_step_row;
		}
	}
}

static void prd_mod_self_get(struct lxs_prd_data *prd,
			char *raw_buf, int row_size, int col_size,
			int data_fmt, u32 flag, int reverse)
{
	prd_mod_self_adj(prd, raw_buf, row_size, col_size, data_fmt, flag, 1, reverse);
}

static void prd_mod_self_put(struct lxs_prd_data *prd,
			char *raw_buf, int row_size, int col_size,
			int data_fmt, u32 flag)
{
	prd_mod_self_adj(prd, raw_buf, row_size, col_size, data_fmt, flag, 0, 0);
}

static int prd_mod_raw_data_dim(struct lxs_prd_data *prd, struct lxs_prd_raw_ctrl *ctrl)
{
	struct lxs_prd_dim_ctl *dim_ctl = NULL;
	char *raw_buf = ctrl->raw_buf;
	char *raw_tmp = ctrl->raw_tmp;
	int row_size = ctrl->row_act;
	int col_size = ctrl->col_act;
	int data_fmt = ctrl->data_fmt;
	int is_self = !!(data_fmt & PRD_DATA_FMT_SELF);
	int do_self_conv = (ctrl == &prd->conv_app);
	int swap = 0;
	int flip_v = 0;
	int flip_h = 0;

	if (!ctrl->is_dim)
		return 0;

	dim_ctl = (ctrl->flag & PRD_RAW_FLAG_TYPE_SYS) ? &prd->sys_dim_ctl : &prd->raw_dim_ctl;

	swap = dim_ctl->swap;
	flip_v = dim_ctl->flip_v;
	flip_h = dim_ctl->flip_h;

	memcpy(raw_tmp, raw_buf, ctrl->data_size);

	if (swap)
		__buff_swap(prd, raw_buf, raw_tmp, row_size, col_size, data_fmt, swap - 1);
	else if (flip_v && flip_h)
		__buff_flip_c(prd, raw_buf, raw_tmp, row_size, col_size, data_fmt);
	else if (flip_v)
		__buff_flip_v(prd, raw_buf, raw_tmp, row_size, col_size, data_fmt);
	else if (flip_h)
		__buff_flip_h(prd, raw_buf, raw_tmp, row_size, col_size, data_fmt);

	if (prd->dbg_mask & BIT(16))
		do_self_conv |= (ctrl == &prd->conv_raw);

	if (do_self_conv && is_self) {
		if (ctrl->data_fmt & PRD_DATA_FMT_SELF_REV_CHK) {
			struct lxs_prd_raw_ctrl *orig = prd->orig_raw;
			int dim_dir = dim_ctl->data_fmt & PRD_DATA_FMT_SELF_REV_DIR;

			prd_mod_self_get(prd, ctrl->raw_tmp, orig->row_size, orig->col_size,
				orig->data_fmt ^ dim_dir, orig->flag, !!swap);

			ctrl->data_fmt &= ~PRD_DATA_FMT_SELF_REV_CHK;
			prd_mod_self_put(prd, ctrl->raw_buf, row_size, col_size,
				ctrl->data_fmt, ctrl->flag);
		}
	}

	return 0;
}

/*
 * eliminate pad-area
 */
static int prd_mod_raw_data_pad(struct lxs_prd_data *prd, struct lxs_prd_raw_ctrl *ctrl)
{
	struct lxs_prd_raw_pad *pad = ctrl->pad;
	int16_t *rbuf_s16 = (int16_t *)ctrl->raw_buf;
	s8 *rbuf_s8 = (s8 *)ctrl->raw_buf;
	int16_t *tbuf_s16 = (int16_t *)ctrl->raw_tmp;
	s8 *tbuf_s8 = (s8 *)ctrl->raw_tmp;
	int is_16bit = !!(ctrl->data_fmt & PRD_DATA_FMT_MASK);
	int node_size = 0;
	int ctrl_col = 0;
	int off_row = 0;
	int off_col = 0;
	int pad_col = 0;
	int row, col;
	int i;

	if (!ctrl->is_pad)
		return 0;

	node_size = (ctrl->row_act * ctrl->col_act);
	ctrl_col = ctrl->col_act;

	pad = ctrl->pad;

	off_row = pad->row;
	off_col = pad->col;
	pad_col = (pad->col<<1);

	for (i = 0; i < node_size; i++) {
		row = i / ctrl_col;
		col = i % ctrl_col;
		if (is_16bit)
			rbuf_s16[i] = tbuf_s16[(row + off_row)*(ctrl_col + pad_col) + (col + off_col)];
		else
			rbuf_s8[i] = tbuf_s8[(row + off_row)*(ctrl_col + pad_col) + (col + off_col)];
	}

	return 0;
}

static int __used prd_mod_raw_data(void *prd_data, void *ctrl_data)
{
	struct lxs_prd_data *prd = (struct lxs_prd_data *)prd_data;
	struct lxs_prd_raw_ctrl *ctrl = (struct lxs_prd_raw_ctrl *)ctrl_data;

	prd_mod_raw_data_pad(prd, ctrl);

	prd_mod_raw_data_dim(prd, ctrl);

	return 0;
}

static int prd_mod_com_type(u32 data_fmt, u32 flag, int type)
{
	if (flag & PRD_RAW_FLAG_TBL_RXTX)
		return 0;

	if (data_fmt & PRD_DATA_FMT_DATA)
		return 0;

	switch (type) {
	case 0:
		if (data_fmt & PRD_DATA_FMT_RXTX)
			return 0;
		break;
	case 1:
		return (data_fmt & PRD_DATA_FMT_RXTX);
	}

	return 1;
}

static int prd_mod_raw_type(struct lxs_prd_raw_ctrl *ctrl, int type)
{
	return prd_mod_com_type(ctrl->data_fmt, ctrl->flag, type);
}

static void prd_mod_raw_alt(struct lxs_prd_data *prd,
				struct lxs_prd_raw_ctrl *ctrl,
				struct lxs_prd_dim_ctl *dim_ctl)
{
	int is_self = !!(ctrl->data_fmt & PRD_DATA_FMT_SELF);

	if (!dim_ctl->swap && !dim_ctl->flip_v && !dim_ctl->flip_h)
		return;

	ctrl->is_dim = 1;
	ctrl->data_fmt |= PRD_DATA_FMT_DIM;

	if (dim_ctl->swap) {
		swap(ctrl->row_size, ctrl->col_size);
		if (is_self) {
			t_prd_dbg_base(prd, "%s: data_fmt(1) 0x%08X\n", __func__, ctrl->data_fmt);
			if (!!(dim_ctl->rev_rx) ^ !!(dim_ctl->rev_tx)) {
				ctrl->data_fmt ^= PRD_DATA_FMT_SELF_REV_DIR;
				t_prd_dbg_base(prd, "%s: data_fmt(2) 0x%08X\n", __func__, ctrl->data_fmt);
			}
		}
	}
	if (is_self)
		ctrl->data_fmt ^= dim_ctl->data_fmt;
	t_prd_dbg_base(prd, "%s: data_fmt(3) 0x%08X\n", __func__, ctrl->data_fmt);

	ctrl->mod_raw_func = prd_mod_raw_data;
}

static void __used *prd_mod_raw_try(struct lxs_prd_data *prd,
						struct lxs_prd_raw_ctrl *ctrl,
						struct lxs_prd_raw_ctrl *replace)
{
	struct lxs_prd_dim_ctl *dim_ctl = &prd->raw_dim_ctl;

	if (!prd->panel_type)
		return ctrl;

	if ((ctrl == NULL) || (replace == NULL))
		return ctrl;

	if (ctrl->is_dim)
		return ctrl;

	if (!prd_mod_raw_type(ctrl, 0))
		return ctrl;

	if (ctrl->flag & PRD_RAW_FLAG_TYPE_SYS)
		dim_ctl = &prd->sys_dim_ctl;

	if (!dim_ctl->swap && !dim_ctl->flip_v && !dim_ctl->flip_h)
		return ctrl;

	prd->orig_raw = ctrl;

	memcpy(replace, ctrl, sizeof(struct lxs_prd_raw_ctrl));
	ctrl = replace;

	prd_mod_raw_alt(prd, ctrl, dim_ctl);

	return ctrl;
}

static void __used prd_mod_raw_scan(struct lxs_prd_data *prd,
					struct lxs_prd_raw_ctrl *ctrl)
{
	struct lxs_prd_dim_ctl *dim_ctl;
	int is_self = !!(ctrl->data_fmt & PRD_DATA_FMT_SELF);
	int fmt_rev = 0;

	if (!prd->panel_type)
		return;

	dim_ctl = (ctrl->flag & PRD_RAW_FLAG_TYPE_SYS) ? &prd->sys_dim_ctl : &prd->raw_dim_ctl;

	if (is_self) {
		if (dim_ctl->rev_rx)
			fmt_rev |= (dim_ctl->swap) ? PRD_DATA_FMT_SELF_REV_C : PRD_DATA_FMT_SELF_REV_R;
		if (dim_ctl->rev_tx)
			fmt_rev |= (dim_ctl->swap) ? PRD_DATA_FMT_SELF_REV_R : PRD_DATA_FMT_SELF_REV_C;
	}

	if (prd_mod_raw_type(ctrl, 0)) {
		if (is_self) {
			ctrl->data_fmt |= fmt_rev;
			ctrl->data_fmt |= (dim_ctl->rev_ord) ? PRD_DATA_FMT_SELF_MISALIGN : 0;
		}
		if (ctrl->data_fmt & PRD_DATA_FMT_DIM)
			prd_mod_raw_alt(prd, ctrl, dim_ctl);
	} else if (prd_mod_raw_type(ctrl, 1)) {
		ctrl->data_fmt |= fmt_rev;
		ctrl->data_fmt |= (dim_ctl->rev_ord) ? PRD_DATA_FMT_SELF_TX_FIRST : 0;
		if (dim_ctl->rev_ord)
			swap(ctrl->row_size, ctrl->col_size);
	}
}

static int __used prd_get_raw_data(void *prd_data, void *ctrl_data)
{
	struct lxs_prd_data *prd = (struct lxs_prd_data *)prd_data;
	struct lxs_prd_raw_ctrl *ctrl = (struct lxs_prd_raw_ctrl *)ctrl_data;
	void *rbuf = ctrl->raw_buf;
	int data_size = ctrl->data_size;
	u32 offset = ctrl->offset;
	int ret = 0;

	if (ctrl->is_pad) {
		struct lxs_prd_raw_pad *pad = ctrl->pad;
		int row_size = ctrl->row_act + (pad->row<<1);
		int col_size = ctrl->col_act + (pad->col<<1);

		data_size = __cal_data_size(row_size, col_size, ctrl->data_fmt);

		rbuf = ctrl->raw_tmp;
	}

	ret = prd_read_memory(prd, offset, rbuf, data_size);
	if (ret < 0)
		return ret;

	return 0;
}

#if defined(__PRD_SUPPORT_SD_TEST)
static int prd_write_sd_log(struct lxs_prd_data *prd, char *write_data, int write_time)
{
	return 0;
}

static void prd_fw_version_log(struct lxs_prd_data *prd)
{
	struct device *dev = prd->dev;
	struct lxs_ts *ts = dev_get_drvdata(dev);
	struct lxs_ts_chip *chip = &ts->chip;
	struct lxs_hal_fw_info *fw = &chip->fw;
	char *log_buf = prd->log_line;
	int ret = 0;

	memset(log_buf, 0, sizeof(prd->log_line));

	t_prd_info(prd, "%s\n", "======== Firmware Info ========");
	t_prd_info(prd, "v%d.%02d (%s, %d)\n",
		fw->v.version.major, fw->v.version.minor,
		fw->product_id, fw->revision);

	ret = snprintf(log_buf, PRD_LOG_LINE_SIZE,
				"======== Firmware Info ========\n");
	ret += snprintf(log_buf + ret, PRD_LOG_LINE_SIZE - ret,
				"v%d.%02d (%s, %d)\n",
				fw->v.version.major, fw->v.version.minor,
				fw->product_id, fw->revision);

	prd_write_sd_log(prd, log_buf, TIME_INFO_SKIP);
}

static void prd_ic_run_info_print(struct lxs_prd_data *prd)
{
	struct device *dev = prd->dev;
	struct lxs_ts *ts = dev_get_drvdata(dev);
	struct lxs_ts_chip *chip = &ts->chip;
	struct lxs_hal_reg *reg = chip->reg;
	char *log_buf = prd->log_line;
	int ret = 0;
	u32 rdata[4] = {0, };
	struct lxs_hal_rw_multi multi[] = {
		{ 0, reg->info_lot_num, &rdata[0], sizeof(rdata[0]), "lot" },
		{ 0, reg->info_serial_num, &rdata[1], sizeof(rdata[1]), "sn" },
		{ 0, reg->info_date, &rdata[2], sizeof(rdata[2]), "date" },
		{ 0, reg->info_time, &rdata[3], sizeof(rdata[3]), "time" },
		{ -1, -1, NULL, }
	};

	memset(log_buf, 0, sizeof(prd->log_line));

	ret = lxs_hal_reg_rw_multi(ts, multi, "prd ic info");
	if (ret < 0)
		return;

	ret = snprintf(log_buf, PRD_LOG_LINE_SIZE,
				"\n===== Production Info =====\n");
	ret += snprintf(log_buf + ret, PRD_LOG_LINE_SIZE - ret,
				"lot : %d\n", rdata[0]);
	ret += snprintf(log_buf + ret, PRD_LOG_LINE_SIZE - ret,
				"serial : 0x%X\n", rdata[1]);
	ret += snprintf(log_buf + ret, PRD_LOG_LINE_SIZE - ret,
				"date : 0x%X 0x%X\n",
				rdata[2], rdata[3]);
	ret += snprintf(log_buf + ret, PRD_LOG_LINE_SIZE - ret,
				"date : %04d.%02d.%02d %02d:%02d:%02d Site%d\n",
				rdata[2] & 0xFFFF, (rdata[2] >> 16 & 0xFF),
				(rdata[2] >> 24 & 0xFF), rdata[3] & 0xFF,
				(rdata[3] >> 8 & 0xFF),
				(rdata[3] >> 16 & 0xFF),
				(rdata[3] >> 24 & 0xFF));

	prd_write_sd_log(prd, log_buf, TIME_INFO_SKIP);
}

static int prd_write_test_control(struct lxs_prd_data *prd, u32 mode)
{
	struct device *dev = prd->dev;
	struct lxs_ts *ts = dev_get_drvdata(dev);
	struct lxs_ts_chip *chip = &ts->chip;
	struct lxs_hal_reg *reg = chip->reg;
	u32 mode_check = -1;
	unsigned int delay_ms = 30;
	int addr = reg->prd_tc_test_mode_ctl;
	int ret = 0;

	if (lxs_addr_is_invalid(addr))
		return 0;

	ret = lxs_hal_write_value(ts, addr, mode);
	if (ret < 0)
		goto out;

	t_prd_info(prd, "wr prd_tc_test_mode_ctl[%04Xh] = %d\n",
		addr, mode);

	lxs_ts_delay(delay_ms);

	ret = lxs_hal_read_value(ts, addr, &mode_check);
	if (ret < 0)
		goto out;

	t_prd_info(prd, "rd prd_tc_test_mode_ctl[%04Xh] = %d\n",
		addr, mode_check);

out:
	return ret;
}

static int __prd_sd_pre(struct lxs_prd_data *prd, char *buf)
{
	struct lxs_ts *ts = prd->ts;
	struct lxs_ts_chip *chip = &ts->chip;
	int do_reset = !(prd->dbg_mask & BIT(1));
	int ret = 0;

	if (do_reset) {
		t_prd_info(prd, "%s: pin reset\n", __func__);
		prd_pin_reset(prd, 0);
		return 0;
	}

	ret = prd_write_test_control(prd, CMD_TEST_ENTER);
	if (ret < 0)
		goto out;

	if (chip->driving_mode == LCD_MODE_STOP)
		goto out;

	ret = prd_chip_driving(prd, LCD_MODE_STOP);
	if (ret < 0)
		goto out;

out:
	return ret;
}

static int __prd_sd_post(struct lxs_prd_data *prd, char *prt_buf)
{
	t_prd_info(prd, "%s\n", "======== RESULT =======");
	t_prd_info(prd, "%s", prt_buf);

	prd_write_sd_log(prd, prt_buf, TIME_INFO_SKIP);

	prd_write_test_control(prd, CMD_TEST_EXIT);

	return 0;
}

static int prd_mod_sd_data_dim(struct lxs_prd_data *prd, struct lxs_prd_sd_ctrl *sd_ctrl)
{
	struct lxs_prd_dim_ctl *dim_ctl = &prd->sd_dim_ctl;
	char *raw_buf = sd_ctrl->raw_buf;
	char *raw_tmp = sd_ctrl->raw_tmp;
	int row_size = sd_ctrl->row_act;
	int col_size = sd_ctrl->col_act;
	int data_fmt = sd_ctrl->data_fmt;
	int swap = 0;
	int flip_v = 0;
	int flip_h = 0;

	if (!sd_ctrl->is_dim)
		return SD_RESULT_PASS;

	swap = dim_ctl->swap;
	flip_v = dim_ctl->flip_v;
	flip_h = dim_ctl->flip_h;

	memcpy(raw_tmp, raw_buf, sd_ctrl->data_size);

	if (swap)
		__buff_swap(prd, raw_buf, raw_tmp, row_size, col_size, data_fmt, swap - 1);
	else if (flip_v && flip_h)
		__buff_flip_c(prd, raw_buf, raw_tmp, row_size, col_size, data_fmt);
	else if (flip_v)
		__buff_flip_v(prd, raw_buf, raw_tmp, row_size, col_size, data_fmt);
	else if (flip_h)
		__buff_flip_h(prd, raw_buf, raw_tmp, row_size, col_size, data_fmt);

	return SD_RESULT_PASS;
}

/*
 * eliminate pad-area
 */
static int prd_mod_sd_data_pad(struct lxs_prd_data *prd, struct lxs_prd_sd_ctrl *sd_ctrl)
{
	if (!sd_ctrl->is_pad)
		return SD_RESULT_PASS;

	/* TBD */

	return SD_RESULT_PASS;
}

static int __used prd_mod_sd_data(void *prd_data, void *ctrl_data)
{
	struct lxs_prd_data *prd = (struct lxs_prd_data *)prd_data;
	struct lxs_prd_sd_ctrl *sd_ctrl = (struct lxs_prd_sd_ctrl *)ctrl_data;

	prd_mod_sd_data_pad(prd, sd_ctrl);

	prd_mod_sd_data_dim(prd, sd_ctrl);

	return SD_RESULT_PASS;
}

static int __used prd_mod_sd_m1(void *prd_data, void *ctrl_data)
{
	struct lxs_prd_data *prd = (struct lxs_prd_data *)prd_data;
	struct lxs_prd_sd_ctrl *sd_ctrl = (struct lxs_prd_sd_ctrl *)ctrl_data;

	memcpy(sd_ctrl->raw_tmp, sd_ctrl->raw_buf, sd_ctrl->data_size);

	__buff_swap(prd, sd_ctrl->raw_buf, sd_ctrl->raw_tmp,
		sd_ctrl->row_act, sd_ctrl->col_act, sd_ctrl->data_fmt, 0);

	return SD_RESULT_PASS;
}

static int prd_mod_sd_type(struct lxs_prd_sd_ctrl *sd_ctrl, int type)
{
	if (sd_ctrl->odd_sd_func)
		return 0;

	return prd_mod_com_type(sd_ctrl->data_fmt, sd_ctrl->flag, type);
}

static void prd_mod_sd_alt(struct lxs_prd_data *prd,
				struct lxs_prd_sd_ctrl *sd_ctrl,
				struct lxs_prd_dim_ctl *dim_ctl)
{
	int is_self = !!(sd_ctrl->data_fmt & PRD_DATA_FMT_SELF);

	if (!dim_ctl->swap && !dim_ctl->flip_v && !dim_ctl->flip_h)
		return;

	sd_ctrl->is_dim = 1;
	sd_ctrl->data_fmt |= PRD_DATA_FMT_DIM;

	if (dim_ctl->swap) {
		swap(sd_ctrl->row_size, sd_ctrl->col_size);
		if (is_self) {
			t_prd_dbg_base(prd, "%s: data_fmt(1) 0x%08X\n", __func__, sd_ctrl->data_fmt);
			if (!!(dim_ctl->rev_rx) ^ !!(dim_ctl->rev_tx)) {
				sd_ctrl->data_fmt ^= PRD_DATA_FMT_SELF_REV_DIR;
				t_prd_dbg_base(prd, "%s: data_fmt(2) 0x%08X\n", __func__, sd_ctrl->data_fmt);
			}
		}
	}
	if (is_self)
		sd_ctrl->data_fmt ^= dim_ctl->data_fmt;
	t_prd_dbg_base(prd, "%s: data_fmt(3) 0x%08X\n", __func__, sd_ctrl->data_fmt);

	sd_ctrl->mod_sd_func = prd_mod_sd_data;
}

static void __used *prd_mod_sd_try(struct lxs_prd_data *prd,
						struct lxs_prd_sd_ctrl *sd_ctrl,
						struct lxs_prd_sd_ctrl *replace)
{
	struct lxs_prd_dim_ctl *dim_ctl = &prd->sd_dim_ctl;

	if (!prd->panel_type)
		return sd_ctrl;

	if ((sd_ctrl == NULL) || (replace == NULL))
		return sd_ctrl;

	if (sd_ctrl->is_dim)
		return sd_ctrl;

	if (!prd_mod_sd_type(sd_ctrl, 0))
		return sd_ctrl;

	if (!dim_ctl->swap && !dim_ctl->flip_v && !dim_ctl->flip_h)
		return sd_ctrl;

	prd->orig_sd = sd_ctrl;

	memcpy(replace, sd_ctrl, sizeof(struct lxs_prd_sd_ctrl));
	sd_ctrl = replace;

	prd_mod_sd_alt(prd, sd_ctrl, dim_ctl);

	return sd_ctrl;
}

static void __used prd_mod_sd_scan(struct lxs_prd_data *prd,
					struct lxs_prd_sd_ctrl *sd_ctrl)
{
	struct lxs_prd_dim_ctl *dim_ctl = &prd->sd_dim_ctl;
	int is_self = !!(sd_ctrl->data_fmt & PRD_DATA_FMT_SELF);
	int fmt_rev = 0;

	if (!prd->panel_type)
		return;

	if (is_self) {
		if (dim_ctl->rev_rx)
			fmt_rev |= (dim_ctl->swap) ? PRD_DATA_FMT_SELF_REV_C : PRD_DATA_FMT_SELF_REV_R;
		if (dim_ctl->rev_tx)
			fmt_rev |= (dim_ctl->swap) ? PRD_DATA_FMT_SELF_REV_R : PRD_DATA_FMT_SELF_REV_C;
	}

	if (prd_mod_sd_type(sd_ctrl, 0)) {
		if (is_self) {
			sd_ctrl->data_fmt |= fmt_rev;
			sd_ctrl->data_fmt |= (dim_ctl->rev_ord) ? PRD_DATA_FMT_SELF_MISALIGN : 0;
		}
		if (sd_ctrl->data_fmt & PRD_DATA_FMT_DIM)
			prd_mod_sd_alt(prd, sd_ctrl, dim_ctl);
	} else if (prd_mod_sd_type(sd_ctrl, 1)) {
		sd_ctrl->data_fmt |= fmt_rev;
		sd_ctrl->data_fmt |= (dim_ctl->rev_ord) ? PRD_DATA_FMT_SELF_TX_FIRST : 0;
		if (dim_ctl->rev_ord)
			swap(sd_ctrl->row_size, sd_ctrl->col_size);
	}
}

static int __used prd_get_sd_raw(void *prd_data, void *ctrl_data)
{
	struct lxs_prd_data *prd = (struct lxs_prd_data *)prd_data;
	struct lxs_prd_sd_ctrl *sd_ctrl = (struct lxs_prd_sd_ctrl *)ctrl_data;
	char *rbuf = sd_ctrl->raw_buf;
	int data_size = sd_ctrl->data_size;
	u32 offset = sd_ctrl->offset;
	int ret = 0;

	if (sd_ctrl->is_pad) {
		struct lxs_prd_raw_pad *pad = sd_ctrl->pad;
		int row_size = sd_ctrl->row_act + (pad->row<<1);
		int col_size = sd_ctrl->col_act + (pad->col<<1);

		data_size = __cal_data_size(row_size, col_size, sd_ctrl->data_fmt);

		rbuf = sd_ctrl->raw_tmp;
	}

	ret = prd_read_memory(prd, offset, rbuf, data_size);
	if (ret < 0)
		return ret;

	return SD_RESULT_PASS;
}

static int __used prd_prt_sd_raw(void *prd_data, void *ctrl_data)
{
	struct lxs_prd_data *prd = (struct lxs_prd_data *)prd_data;
	struct lxs_prd_sd_ctrl *sd_ctrl = (struct lxs_prd_sd_ctrl *)ctrl_data;
	int size = 0;

	size = prd_print_rawdata(prd, sd_ctrl->prt_buf, 0,
				sd_ctrl->raw_buf, sd_ctrl->row_act, sd_ctrl->col_act,
				sd_ctrl->data_fmt, sd_ctrl->flag, sd_ctrl->name, 0);

	prd_write_sd_log(prd, sd_ctrl->prt_buf, TIME_INFO_SKIP);

	memset(sd_ctrl->prt_buf, 0, strlen(sd_ctrl->prt_buf));

	return size;
}

static int prd_chk_sd_base(struct lxs_prd_data *prd,
			struct lxs_prd_sd_ctrl *sd_ctrl, char *raw_tmp, int is_cmp)
{
	char *title = (is_cmp) ? "Compare" : "Check";
	char *prt_buf = sd_ctrl->prt_buf;
	char *raw_buf = (raw_tmp) ? raw_tmp : sd_ctrl->raw_buf;
	uint16_t *rawdata_u16 = (uint16_t *)raw_buf;
	int16_t *rawdata_s16 = (int16_t *)raw_buf;
	s8 *rawdata_s8 = (s8 *)raw_buf;
	int is_16bit = !!(sd_ctrl->data_fmt & PRD_DATA_FMT_MASK);
	int is_u16bit = !!(sd_ctrl->data_fmt & PRD_DATA_FMT_U16);
	int empty_node = !!(sd_ctrl->empty_node != NULL);
	int is_ign_edge_t = !!(sd_ctrl->flag & PRD_RAW_FLAG_IGN_EDGE_T);
	int is_ign_edge_b = !!(sd_ctrl->flag & PRD_RAW_FLAG_IGN_EDGE_B);
	int is_ign_row_t = !!(sd_ctrl->flag & PRD_RAW_FLAG_IGN_ROW_T);
	int is_ign_row_b = !!(sd_ctrl->flag & PRD_RAW_FLAG_IGN_ROW_B);
	int is_ign_col_r = !!(sd_ctrl->flag & PRD_RAW_FLAG_IGN_COL_R);
	int is_ign_col_l = !!(sd_ctrl->flag & PRD_RAW_FLAG_IGN_COL_L);
	int row_size = sd_ctrl->row_act;
	int col_size = sd_ctrl->col_act;
	int curr_lower = prd->cmp_lower;
	int curr_upper = prd->cmp_upper;
	int empty_col_s = 0;
	int empty_col_e = 0;
	int empty_row_s = 0;
	int empty_row_e = 0;
	int curr_raw;
	int prt_size = 0;
	int min_raw = PRD_RAWDATA_MAX;
	int max_raw = PRD_RAWDATA_MIN;
	int cnt = 0;
	int i, j;
	int cmp_result = SD_RESULT_PASS;

	if (empty_node) {
		empty_col_s = sd_ctrl->empty_node->col_s;
		empty_col_e = sd_ctrl->empty_node->col_e;
		empty_row_s = sd_ctrl->empty_node->row_s;
		empty_row_e = sd_ctrl->empty_node->row_e;
	}

	t_prd_info(prd, "-------- %s --------\n", title);
	prt_size += lxs_prd_buf_snprintf(prt_buf, prt_size, "-------- %s --------\n", title);

	if (is_cmp) {
		snprintf(prd->log_line, sizeof(prd->log_line), "%s lower %d, upper %d\n",
			sd_ctrl->name, curr_lower, curr_upper);

		t_prd_info(prd, "%s", prd->log_line);
		prt_size += lxs_prd_buf_snprintf(prt_buf, prt_size, "%s", prd->log_line);
	}

	if (raw_buf == NULL) {
		t_prd_err(prd, "%s failed: NULL buf\n", title);
		prt_size += lxs_prd_buf_snprintf(prt_buf, prt_size, "%s failed: NULL buf\n", title);
		return SD_RESULT_FAIL;
	}

	if (sd_ctrl->data_fmt & PRD_DATA_FMT_RXTX) {
		col_size += row_size;
		row_size = 1;
	}

	for (i = 0; i < row_size; i++) {
		for (j = 0; j < col_size; j++) {
			if (is_16bit)
				curr_raw = (is_u16bit) ? *rawdata_u16++ : *rawdata_s16++;
			else
				curr_raw = *rawdata_s8++;

			if (empty_node &&
				((i >= empty_row_s) && (i <= empty_row_e)) &&
				((j >= empty_col_s) && (j <= empty_col_e))) {
				t_prd_dbg_trace(prd, "empty node: [%d, %d] <= [%d, %d] < [%d, %d]\n",
					empty_row_s, empty_col_s, i, j, empty_row_e, empty_col_e);
				continue;
			}

			if (is_ign_col_l && !j)
				continue;

			if (is_ign_col_r && (j == (col_size - 1)))
				continue;

			if (is_ign_row_t && !i)
				continue;
			else if (is_ign_edge_t && !i)
				if (!j || (j == (col_size-1)))
					continue;

			if (is_ign_row_b && (i == (row_size - 1)))
				continue;
			else if (is_ign_edge_b && (i == (row_size - 1)))
				if (!j || (j == (col_size-1)))
					continue;

			if (curr_raw < min_raw)
				min_raw = curr_raw;

			if (curr_raw > max_raw)
				max_raw = curr_raw;

			if (!is_cmp)
				continue;

			if ((curr_raw >= curr_lower) && (curr_raw <= curr_upper)) {
				t_prd_dbg_trace(prd, "cnt %d, %d(lower) <= %d <= %d(upper)\n",
					cnt++, curr_lower, curr_raw, curr_upper);
				continue;
			}

			snprintf(prd->log_line, sizeof(prd->log_line),
				"F [%d][%d] = %d\n", i, j, curr_raw);

			t_prd_info(prd, "%s", prd->log_line);
			prt_size += lxs_prd_buf_snprintf(prt_buf, prt_size, "%s", prd->log_line);

			cmp_result = SD_RESULT_FAIL;
		}
	}

	sd_ctrl->min = min_raw;
	sd_ctrl->max = max_raw;

	t_prd_info(prd, "%s min %d, max %d\n", sd_ctrl->name, min_raw, max_raw);
	prt_size += lxs_prd_buf_snprintf(prt_buf, prt_size,
		"%s min %d, max %d\n", sd_ctrl->name, min_raw, max_raw);

	if (!is_cmp)
		return SD_RESULT_PASS;

	if (cmp_result != SD_RESULT_PASS) {
		t_prd_info(prd, "%s NG\n", sd_ctrl->name);
		prt_size += lxs_prd_buf_snprintf(prt_buf, prt_size, "%s NG\n", sd_ctrl->name);
		return cmp_result;
	}

	t_prd_info(prd, "%s OK\n", sd_ctrl->name);
	prt_size += lxs_prd_buf_snprintf(prt_buf, prt_size, "%s OK\n", sd_ctrl->name);

	return SD_RESULT_PASS;
}

static int prd_cmp_sd_common(struct lxs_prd_data *prd,
			struct lxs_prd_sd_ctrl *sd_ctrl, char *raw_tmp)
{
	return prd_chk_sd_base(prd, sd_ctrl, raw_tmp, 1);
}

static int prd_cmp_sd_raw_single(struct lxs_prd_data *prd, struct lxs_prd_sd_ctrl *sd_ctrl)
{
	int invalid = 0;
	int cmp_result = SD_RESULT_PASS;

	prd->cmp_lower = sd_ctrl->cmp_min;
	prd->cmp_upper = sd_ctrl->cmp_max;

	invalid |= !!(prd->cmp_lower < 0);
	invalid |= !!(prd->cmp_upper < 0);
	invalid |= !!(prd->cmp_lower >= prd->cmp_upper);

	if (invalid) {
		sprintf(sd_ctrl->prt_buf, "wrong condition: type %d, lower %d, upper %d\n",
			sd_ctrl->id, prd->cmp_lower, prd->cmp_upper);

		t_prd_err(prd, "%s", sd_ctrl->prt_buf);

		return -EINVAL;
	}

	cmp_result = prd_cmp_sd_common(prd, sd_ctrl, NULL);

	return cmp_result;
}

static int __used prd_cmp_sd_raw(void *prd_data, void *ctrl_data)
{
	struct lxs_prd_data *prd = (struct lxs_prd_data *)prd_data;
	struct lxs_prd_sd_ctrl *sd_ctrl = (struct lxs_prd_sd_ctrl *)ctrl_data;
	int cmp_result = SD_RESULT_PASS;

	cmp_result = prd_cmp_sd_raw_single(prd, sd_ctrl);

	prd_write_sd_log(prd, sd_ctrl->prt_buf, TIME_INFO_SKIP);

	return cmp_result;
}

static int prd_chk_sd_common(struct lxs_prd_data *prd,
			struct lxs_prd_sd_ctrl *sd_ctrl, char *raw_tmp)
{
	return prd_chk_sd_base(prd, sd_ctrl, raw_tmp, 0);
}

static int __used prd_chk_sd_raw(void *prd_data, void *ctrl_data)
{
	struct lxs_prd_data *prd = (struct lxs_prd_data *)prd_data;
	struct lxs_prd_sd_ctrl *sd_ctrl = (struct lxs_prd_sd_ctrl *)ctrl_data;

	return prd_chk_sd_common(prd, sd_ctrl, NULL);
}

static int __used prd_get_sd_os(void *prd_data, void *ctrl_data)
{
	return prd_get_sd_raw(prd_data, ctrl_data);
}

static int __used prd_prt_sd_rxtx(void *prd_data, void *ctrl_data)
{
	struct lxs_prd_data *prd = (struct lxs_prd_data *)prd_data;
	struct lxs_prd_sd_ctrl *sd_ctrl = (struct lxs_prd_sd_ctrl *)ctrl_data;
	int size = 0;

	size = prd_print_data_rxtx(prd, sd_ctrl->prt_buf, 0,
				sd_ctrl->raw_buf, sd_ctrl->row_act, sd_ctrl->col_act,
				sd_ctrl->data_fmt, sd_ctrl->flag, sd_ctrl->name);

	prd_write_sd_log(prd, sd_ctrl->prt_buf, TIME_INFO_SKIP);

	memset(sd_ctrl->prt_buf, 0, strlen(sd_ctrl->prt_buf));

	return size;
}

static int __used prd_cmp_sd_rxtx(void *prd_data, void *ctrl_data)
{
	return prd_cmp_sd_raw(prd_data, ctrl_data);
}

static int __used prd_chk_sd_rxtx(void *prd_data, void *ctrl_data)
{
	return prd_chk_sd_raw(prd_data, ctrl_data);
}

static int __used prd_sd_calb_test(void *prd_data, void *ctrl_data)
{
	struct lxs_prd_data *prd = (struct lxs_prd_data *)prd_data;
	struct device *dev = prd->dev;
	struct lxs_ts *ts = dev_get_drvdata(dev);
	int ret = SD_RESULT_PASS;

	t_prd_info(prd, "%s\n", __func__);

	ret = lxs_hal_write_value(ts, 0xd21, 1);
	if (ret < 0)
		return ret;

	t_prd_info(prd, "%s: done\n", __func__);

	return SD_RESULT_PASS;
}

struct lxs_prd_sync_tbl {
	char *name;
	u32 cmd;
	u32 header;
	u32 min;
	u32 max;
	u32 ret;
};

static int __used prd_sd_sync_test(void *prd_data, void *ctrl_data)
{
	struct lxs_prd_data *prd = (struct lxs_prd_data *)prd_data;
	struct lxs_ts *ts = prd->ts;
	struct lxs_ts_chip *chip = &ts->chip;
	struct lxs_hal_reg *reg = chip->reg;
	struct lxs_prd_sd_ctrl *sd_ctrl = (struct lxs_prd_sd_ctrl *)ctrl_data;
	struct lxs_prd_sync_tbl sync_table[] = {
		{	.name = "SYNC(V)",
			.cmd = 0x15,
			.header = 0x1A2B,
			.min = sd_ctrl->cmp_min & 0xFFFF,
			.max = sd_ctrl->cmp_max & 0xFFFF,
		},
	};
	struct lxs_prd_sync_tbl *table;
	char *name;
	u32 rbuf[2] = { 0, };
	u32 addr = 0;
	int retry = 80;
	u32 rdata = 0x1;
	u32 rdata_lsb = !!(sd_ctrl->flag & PRD_RAW_FLAG_RESP_LSB);
	u32 sync_header = 0;
	u32 sync_result = 0;
	u32 sync_cnt = 0;
	u32 sync_freq = 0;
	int do_reset = !(prd->dbg_mask & BIT(5));
	int i;
	int ret = SD_RESULT_PASS;

	if (do_reset) {
		t_prd_info(prd, "%s: pin reset\n", __func__);
		prd_pin_reset(prd, 500);
	}

	t_prd_info(prd, "%s: run test\n", __func__);

	for (i = 0; i < ARRAY_SIZE(sync_table); i++) {
		table = &sync_table[i];

		table->ret = SD_RESULT_FAIL;

		name = table->name;

		ret = lxs_hal_write_value(ts, 0xC09, table->cmd);
		if (ret < 0)
			goto out;

		lxs_ts_delay(500);

		/* Check Test Result */
		addr = reg->tc_tsp_test_status;
		retry = 20;
		do {
			lxs_ts_delay(100);
			ret = lxs_hal_read_value(ts, addr, &rdata);
			if (ret < 0)
				goto out;

			if (ret >= 0)
				t_prd_dbg_base(prd, "rdata[%04Xh] = 0x%x\n",
					addr, rdata);

			if (rdata_lsb)
				rdata &= 0xFFFF;
		} while ((rdata != 0xAA) && retry--);

		if (rdata != 0xAA) {
			t_prd_err(prd, "%s test '%s' Time out, %08Xh\n",
				(sd_ctrl->is_sd) ? "sd_np" : "sd_lp", name, rdata);

			t_prd_err(prd, "%s test failed\n", name);
			continue;
		}

		ret = prd_read_memory(prd, 0xEE7, (char *)rbuf, sizeof(rbuf));
		if (ret < 0)
			goto out;

		sync_result = (rbuf[0] >> 16) & 0xFFFF;
		sync_header = rbuf[0] & 0xFFFF;

		sync_cnt = (rbuf[1] >> 16) & 0xFFFF;
		sync_freq = rbuf[1] & 0xFFFF;

		t_prd_info(prd, "%s test: head 0x%04X\n",
			name, sync_header);

		t_prd_info(prd, "%s test: freq %d, cnt %d\n",
			name, sync_freq, sync_cnt);

		if (sync_header != table->header) {
			t_prd_err(prd, "%s test: invalid header 0x%04X, shall be 0x%04X\n",
				name, sync_header, table->header);
			continue;
		}

		if ((sync_freq < table->min) || (sync_freq > table->max)) {
			t_prd_err(prd, "%s test: invalid freq %d, shall be %d <= x <= %d\n",
				name, sync_freq, table->min, table->max);
			continue;
		}

		table->ret = SD_RESULT_PASS;
	}

	for (i = 0; i < ARRAY_SIZE(sync_table); i++) {
		table = &sync_table[i];
		if (table->ret != SD_RESULT_PASS)
			goto out;
	}

	t_prd_info(prd, "%s: done\n", __func__);

	return SD_RESULT_PASS;

out:
	t_prd_info(prd, "%s: failed\n", __func__);

	return SD_RESULT_FAIL;
}

static int __used prd_sd_irq_test(void *prd_data, void *ctrl_data)
{
	struct lxs_prd_data *prd = (struct lxs_prd_data *)prd_data;
	struct device *dev = prd->dev;
	struct lxs_ts *ts = dev_get_drvdata(dev);
	int pin = ts->plat_data->irq_pin;
	int sys_attr_out = 0x002;
	int i;
	int ret = SD_RESULT_PASS;

	t_prd_info(prd, "%s\n", __func__);

	ret = lxs_hal_write_value(ts, 0xFF3, 0);
	if (ret < 0)
		return ret;

	//ATTR set low
	ret = lxs_hal_write_value(ts, sys_attr_out, 1);
	if (ret < 0)
		return ret;

	ret = -EIO;
	for (i = 0; i < 100; i++) {
		if (!gpio_get_value(pin)) {
			t_prd_info(prd, "%s: INT pin low detected\n", __func__);
			ret = 0;
			break;
		}
		lxs_ts_delay(10);
	}
	if (ret < 0) {
		t_prd_err(prd, "%s: INT pin low failed\n", __func__);
		return ret;
	}

	//ATTR set high
	ret = lxs_hal_write_value(ts, sys_attr_out, 3);
	if (ret < 0)
		return ret;

	ret = -EIO;
	for (i = 0; i < 100; i++) {
		if (gpio_get_value(pin)) {
			t_prd_info(prd, "%s: INT pin high detected\n", __func__);
			ret = 0;
			break;
		}
		lxs_ts_delay(10);
	}
	if (ret < 0) {
		t_prd_err(prd, "%s: INT pin high failed\n", __func__);
		return ret;
	}

	t_prd_info(prd, "%s: done\n", __func__);

	return SD_RESULT_PASS;
}

static int __used prd_sd_rst_test(void *prd_data, void *ctrl_data)
{
	struct lxs_prd_data *prd = (struct lxs_prd_data *)prd_data;
	struct device *dev = prd->dev;
	struct lxs_ts *ts = dev_get_drvdata(dev);
	struct lxs_ts_chip *chip = &ts->chip;
	struct lxs_hal_reg *reg = chip->reg;
	u32 rst_sts = reg->spr_chip_test;
	u32 rst_chk = 0;
	u32 chk_val = 0x5A5A0000;
	u32 data = 0;
	int ret = SD_RESULT_PASS;

	t_prd_info(prd, "%s\n", __func__);

	/* Step-1. Write Test Pattern */
	ret = lxs_hal_write_value(ts, rst_sts, chk_val);
	if (ret < 0) {
		t_prd_err(prd, "%s: wr err, %08Xh, %d\n",
			__func__, chk_val, ret);
		return ret;
	}

	/* Step-2. Verify Data Before Reset */
	ret = lxs_hal_read_value(ts, rst_sts, &data);
	if (ret < 0) {
		t_prd_err(prd, "%s: rd(before) err: %08Xh, %d\n",
			__func__, data, ret);
		return ret;
	}

	if (data != chk_val) {
		t_prd_err(prd, "%s: check(before): NG: 0x%08X != 0x%08X\n",
			__func__, data, rst_chk);
		return -EFAULT;
	}

	/* Step-3. Reset */
	if (chip->ops_quirk.hw_reset != NULL) {
		chip->ops_quirk.hw_reset(ts, 0);
	} else {
		t_prd_info(prd, "%s: pin reset\n", __func__);
		prd_pin_reset(prd, 0);
	}

	/* Step-4. Verify Data After Reset */
	ret = lxs_hal_read_value(ts, rst_sts, &rst_chk);
	if (ret < 0) {
		t_prd_err(prd, "%s: rd(after) err: %08Xh, %d\n",
			__func__, rst_chk, ret);
		return ret;
	}

	if (rst_chk == chk_val) {
		t_prd_err(prd, "%s: check(after): NG: 0x%08X\n", __func__, rst_chk);
		return -EFAULT;
	}

	t_prd_info(prd, "%s: done\n", __func__);

	return SD_RESULT_PASS;
}

static uint16_t __gap_u16(uint16_t *src_u16, int offset, int offset2)
{
	int n1, n2, curr_raw;

	n1 = src_u16[offset];
	n2 = src_u16[offset2];
	if (!n1)
		n1 = 1;
	if (!n2)
		n2 = 1;
	curr_raw = (n1 >= n2) ? (1000 * n2/n1) : (1000 * n1/n2);
	return (100 - (int)((curr_raw + 5)/10));
}

static int16_t __gap_s16(int16_t *src_s16, int offset, int offset2)
{
	int n1, n2, curr_raw;

	n1 = src_s16[offset];
	n2 = src_s16[offset2];
	if (!n1)
		n1 = 1;
	if (!n2)
		n2 = 1;
	curr_raw = (n1 >= n2) ? (1000 * n2/n1) : (1000 * n1/n2);
	return (100 - (int)((curr_raw + 5)/10));
}

static s8 __gap_s8(s8 *src_s8, int offset, int offset2)
{
	int n1, n2, curr_raw;

	n1 = src_s8[offset];
	n2 = src_s8[offset2];
	if (!n1)
		n1 = 1;
	if (!n2)
		n2 = 1;
	curr_raw = (n1 >= n2) ? (1000 * n2/n1) : (1000 * n1/n2);
	return (100 - (int)((curr_raw + 5)/10));
}

static void __used __cal_gap_h(struct lxs_prd_data *prd, char *dst, char *src,
			int row_size, int col_size, int data_fmt, u32 flag)
{
	uint16_t *dst_u16 = (uint16_t *)dst;
	int16_t *dst_s16 = (int16_t *)dst;
	s8 *dst_s8 = (s8 *)dst;
	uint16_t *src_u16 = (uint16_t *)src;
	int16_t *src_s16 = (int16_t *)src;
	s8 *src_s8 = (s8 *)src;
//	int is_self = !!(data_fmt & PRD_DATA_FMT_SELF);
	int is_16bit = !!(data_fmt & PRD_DATA_FMT_MASK);
	int is_u16bit = !!(data_fmt & PRD_DATA_FMT_U16);
	int offset, offset2;
	int i, j;

	t_prd_dbg_base(prd, "%s\n", __func__);

	if (dst == NULL) {
		t_prd_err(prd, "%s: dst is null\n", __func__);
		return;
	}

	memset(dst, 0, (row_size * col_size)<<1);

	if (src == NULL) {
		t_prd_err(prd, "%s: src is null\n", __func__);
		return;
	}

	for (j = 0; j < row_size; j++) {
		for (i = 0; i < (col_size-1); i++) {
			offset = (j * col_size) + i;
			offset2 = offset + 1;
			if (is_16bit)
				if (is_u16bit)
					dst_u16[offset] = __gap_u16(src_u16, offset, offset2);
				else
					dst_s16[offset] = __gap_s16(src_s16, offset, offset2);
			else
				dst_s8[offset] = __gap_s8(src_s8, offset, offset2);
		}
	}
}

static void __used __cal_gap_v(struct lxs_prd_data *prd, char *dst, char *src,
			int row_size, int col_size, int data_fmt, u32 flag)
{
	uint16_t *dst_u16 = (uint16_t *)dst;
	int16_t *dst_s16 = (int16_t *)dst;
	s8 *dst_s8 = (s8 *)dst;
	uint16_t *src_u16 = (uint16_t *)src;
	int16_t *src_s16 = (int16_t *)src;
	s8 *src_s8 = (s8 *)src;
//	int is_self = !!(data_fmt & PRD_DATA_FMT_SELF);
	int is_16bit = !!(data_fmt & PRD_DATA_FMT_MASK);
	int is_u16bit = !!(data_fmt & PRD_DATA_FMT_U16);
	int offset, offset2;
	int i, j;

	t_prd_dbg_base(prd, "%s\n", __func__);

	if (dst == NULL) {
		t_prd_err(prd, "%s: dst is null\n", __func__);
		return;
	}

	memset(dst, 0, (row_size * col_size)<<1);

	if (src == NULL) {
		t_prd_err(prd, "%s: src is null\n", __func__);
		return;
	}

	for (j = 0; j < (row_size-1); j++) {
		for (i = 0; i < col_size; i++) {
			offset = (j * col_size) + i;
			offset2 = ((j+1) * col_size) + i;
			if (is_16bit)
				if (is_u16bit)
					dst_u16[offset] = __gap_u16(src_u16, offset, offset2);
				else
					dst_s16[offset] = __gap_s16(src_s16, offset, offset2);
			else
				dst_s8[offset] = __gap_s8(src_s8, offset, offset2);
		}
	}
}

static void __used prd_cal_sd_gap_x(struct lxs_prd_data *prd, char *dst, char *src,
			int row_size, int col_size, int data_fmt, u32 flag)
{
	int is_horz = !prd->panel_type;

	if (prd->sd_dim_ctl.type)
		is_horz = (prd->sd_dim_ctl.swap) ? !!(data_fmt & PRD_DATA_FMT_DIM) : 1;

	if (is_horz)
		__cal_gap_h(prd, dst, src, row_size, col_size, data_fmt, flag);
	else
		__cal_gap_v(prd, dst, src, row_size, col_size, data_fmt, flag);
}

static void __used prd_cal_sd_gap_y(struct lxs_prd_data *prd, char *dst, char *src,
			int row_size, int col_size, int data_fmt, u32 flag)
{
	int is_vert = !prd->panel_type;

	if (prd->sd_dim_ctl.type)
		is_vert = (prd->sd_dim_ctl.swap) ? !!(data_fmt & PRD_DATA_FMT_DIM) : 1;

	if (is_vert)
		__cal_gap_v(prd, dst, src, row_size, col_size, data_fmt, flag);
	else
		__cal_gap_h(prd, dst, src, row_size, col_size, data_fmt, flag);
}

static void __used prd_cal_sd_gap_a(struct lxs_prd_data *prd, char *dst, char *src,
			int row_size, int col_size, int data_fmt, u32 flag)
{
	uint16_t *dst_u16 = (uint16_t *)dst;
	int16_t *dst_s16 = (int16_t *)dst;
	s8 *dst_s8 = (s8 *)dst;
//	int is_self = !!(data_fmt & PRD_DATA_FMT_SELF);
	int is_16bit = !!(data_fmt & PRD_DATA_FMT_MASK);
	int is_u16bit = !!(data_fmt & PRD_DATA_FMT_U16);
	char *dst_x, *dst_y;
	int offset;
	int i, j;

	if (dst == NULL) {
		t_prd_err(prd, "%s: dst is null\n", __func__);
		return;
	}

	memset(dst, 0, (row_size * col_size)<<1);

	if (src == NULL) {
		t_prd_err(prd, "%s: src is null\n", __func__);
		return;
	}

	dst_x = prd->raw_tmp + (PRD_RAWBUF_SIZE>>1);
	dst_y = dst_x + ((row_size * col_size)<<1);

	prd_cal_sd_gap_x(prd, dst_x, src, row_size, col_size, data_fmt, flag);
	prd_cal_sd_gap_y(prd, dst_y, src, row_size, col_size, data_fmt, flag);

	offset = 0;
	for (j = 0; j < row_size; j++) {
		for (i = 0; i < col_size; i++) {
			if (is_16bit)
				if (is_u16bit)
					*dst_u16++ = max(((uint16_t *)dst_x)[offset], ((uint16_t *)dst_y)[offset]);
				else
					*dst_s16++ = max(((int16_t *)dst_x)[offset], ((int16_t *)dst_y)[offset]);
			else
				*dst_s8++ = max(((s8 *)dst_x)[offset], ((s8 *)dst_y)[offset]);
			offset++;
		}
	}
}

static int __used prd_prt_sd_gap_x(void *prd_data, void *ctrl_data)
{
	struct lxs_prd_data *prd = (struct lxs_prd_data *)prd_data;
	struct lxs_prd_sd_ctrl *sd_ctrl = (struct lxs_prd_sd_ctrl *)ctrl_data;

	memcpy(sd_ctrl->raw_tmp, sd_ctrl->raw_buf, sd_ctrl->data_size);
	prd_cal_sd_gap_x(prd, sd_ctrl->raw_buf, sd_ctrl->raw_tmp,
		sd_ctrl->row_act, sd_ctrl->col_act, sd_ctrl->data_fmt, sd_ctrl->flag);

	return prd_prt_sd_raw(prd_data, ctrl_data);
}

static int __used prd_prt_sd_gap_y(void *prd_data, void *ctrl_data)
{
	struct lxs_prd_data *prd = (struct lxs_prd_data *)prd_data;
	struct lxs_prd_sd_ctrl *sd_ctrl = (struct lxs_prd_sd_ctrl *)ctrl_data;

	memcpy(sd_ctrl->raw_tmp, sd_ctrl->raw_buf, sd_ctrl->data_size);
	prd_cal_sd_gap_y(prd, sd_ctrl->raw_buf, sd_ctrl->raw_tmp,
		sd_ctrl->row_act, sd_ctrl->col_act, sd_ctrl->data_fmt, sd_ctrl->flag);

	return prd_prt_sd_raw(prd_data, ctrl_data);
}

static int __used prd_prt_sd_gap_a(void *prd_data, void *ctrl_data)
{
	struct lxs_prd_data *prd = (struct lxs_prd_data *)prd_data;
	struct lxs_prd_sd_ctrl *sd_ctrl = (struct lxs_prd_sd_ctrl *)ctrl_data;

	memcpy(sd_ctrl->raw_tmp, sd_ctrl->raw_buf, sd_ctrl->data_size);
	prd_cal_sd_gap_a(prd, sd_ctrl->raw_buf, sd_ctrl->raw_tmp,
		sd_ctrl->row_act, sd_ctrl->col_act, sd_ctrl->data_fmt, sd_ctrl->flag);

	return prd_prt_sd_raw(prd_data, ctrl_data);
}

static void prd_test_mode_debug(struct lxs_prd_data *prd, struct lxs_prd_sd_ctrl *sd_ctrl)
{
	/* TBD */
}

static int prd_write_test_mode(struct lxs_prd_data *prd, struct lxs_prd_sd_ctrl *sd_ctrl)
{
	struct device *dev = prd->dev;
	struct lxs_ts *ts = dev_get_drvdata(dev);
	struct lxs_ts_chip *chip = &ts->chip;
	struct lxs_hal_reg *reg = chip->reg;
	int cmd = sd_ctrl->cmd;
	int is_fw_skip = !!(sd_ctrl->flag & PRD_RAW_FLAG_FW_SKIP);
	int delay = (sd_ctrl->delay) ? sd_ctrl->delay : 200;
	u32 addr = reg->tc_tsp_test_ctl;
	int retry = 80;
	u32 rdata = 0x1;
	u32 rdata_lsb = !!(sd_ctrl->flag & PRD_RAW_FLAG_RESP_LSB);
	int ret = 0;

	if (is_fw_skip)
		return 0;

	ret = lxs_hal_write_value(ts, addr, cmd);
	if (ret < 0)
		return ret;

	t_prd_info(prd, "write testmode[%04Xh] = %Xh\n", addr, cmd);

	lxs_ts_delay(delay);

	/* Check Test Result - wait until 0 is written */
	addr = reg->tc_tsp_test_status;
	do {
		lxs_ts_delay(100);
		ret = lxs_hal_read_value(ts, addr, &rdata);
		if (ret < 0)
			return ret;

		if (ret >= 0)
			t_prd_dbg_base(prd, "rdata[%04Xh] = 0x%x\n",
				addr, rdata);

		if (rdata_lsb)
			rdata &= 0xFFFF;
	} while ((rdata != 0xAA) && retry--);

	if (rdata != 0xAA) {
		t_prd_err(prd, "%s test '%s' Time out, %08Xh\n",
			(sd_ctrl->is_sd) ? "sd_np" : "sd_lp", sd_ctrl->name, rdata);
		prd_test_mode_debug(prd, sd_ctrl);

		t_prd_err(prd, "%s\n", "Write test mode failed");
		return -ETIMEDOUT;
	}

	return 0;
}

static int prd_sd_test_core(struct lxs_prd_data *prd, struct lxs_prd_sd_ctrl *sd_ctrl)
{
	struct device *dev = prd->dev;
	struct lxs_ts *ts = dev_get_drvdata(dev);
	struct lxs_ts_chip *chip = &ts->chip;
	struct lxs_hal_reg *reg = chip->reg;
	char *swap_str = (sd_ctrl->is_dim) ? "(S) " : "";
	char *test_str = sd_ctrl->name;
	char info_str[64] = { 0, };
	char info_str_wr[64] = { 0, };
	int cmd = sd_ctrl->cmd;
	int is_os = sd_ctrl->is_os;
	int is_os_data = !!(sd_ctrl->data_fmt & PRD_DATA_FMT_OS_DATA);
	int test_result = 0;
	int cmp_result = SD_RESULT_PASS;
	int ret = 0;

	sd_ctrl->min = 0;
	sd_ctrl->max = 0;

	t_prd_info(prd, "======== %s %s========\n", test_str, swap_str);

	if (is_os && !cmd) {
		snprintf(info_str, sizeof(info_str),
			"os test: test %s, but command is zero\n", sd_ctrl->name);
		prd_write_sd_log(prd, info_str, TIME_INFO_SKIP);
		t_prd_err(prd, "%s", info_str);
		return SD_RESULT_FAIL;
	}

	if (is_os)
		snprintf(info_str, sizeof(info_str), "[%s %s] Data Offset = 0x%X",
			sd_ctrl->name, (is_os_data) ? "Data" : "Raw", sd_ctrl->offset);
	else
		snprintf(info_str, sizeof(info_str), "[%s] Data Offset = 0x%X",
			sd_ctrl->name, sd_ctrl->offset);

	snprintf(info_str_wr, sizeof(info_str_wr), "\n%s\n", info_str);

	prd_write_sd_log(prd, info_str_wr, TIME_INFO_SKIP);

	t_prd_info(prd, "%s\n", info_str);

	/* Test Start & Finish Check */
	ret = prd_write_test_mode(prd, sd_ctrl);
	if (ret < 0)
		return ret;

	if (is_os) {
		ret = lxs_hal_read_value(ts, reg->tc_tsp_test_pf_result, &test_result);
		if (ret < 0)
			return ret;

		t_prd_info(prd, "%s test result = %d(%s)\n", sd_ctrl->name,
			test_result, (test_result) ? "NG" : "OK");
	}

	memset(prd->raw_buf, 0, sd_ctrl->data_size);

	sd_ctrl->row_act = sd_ctrl->row_size;
	sd_ctrl->col_act = sd_ctrl->col_size;
	sd_ctrl->raw_buf = prd->raw_buf;
	sd_ctrl->raw_tmp = prd->raw_tmp;
	sd_ctrl->prt_buf = prd->log_pool;

	/* specific case */
	if (sd_ctrl->odd_sd_func) {
		return sd_ctrl->odd_sd_func(prd, sd_ctrl);
	}

	if (sd_ctrl->get_sd_func) {
		ret = sd_ctrl->get_sd_func(prd, sd_ctrl);
		if (ret < 0)
			return ret;
	}

	if (sd_ctrl->mod_sd_func) {
		ret = sd_ctrl->mod_sd_func(prd, sd_ctrl);
		if (ret < 0)
			return ret;
	}

	if (sd_ctrl->prt_sd_func) {
		u32 flag_back = t_prd_dbg_flag;
		t_prd_dbg_flag |= PRD_DBG_FLAG_RAW_LOG_ON;
		sd_ctrl->prt_sd_func(prd, sd_ctrl);
		t_prd_dbg_flag = flag_back;
	}

	if (sd_ctrl->cmp_sd_func) {
		cmp_result = sd_ctrl->cmp_sd_func(prd, sd_ctrl);
		if (cmp_result != SD_RESULT_PASS)
			return cmp_result;
	}

	if (is_os)
		return (test_result) ? SD_RESULT_FAIL : SD_RESULT_PASS;

	return SD_RESULT_PASS;
}

static int prd_sd_test_common(struct lxs_prd_data *prd, int *sd_result_list, int is_sd)
{
	struct lxs_prd_sd_ctrl *__sd_ctrl = (is_sd) ? prd->sd_ctrl : prd->lpwg_sd_ctrl;
	struct lxs_prd_sd_ctrl *sd_ctrl = NULL;
	int num_ctrl = (is_sd) ? prd->num_sd_ctrl : prd->num_lpwg_sd_ctrl;
	int i;
	int ret = 0;

	for (i = 0; i < num_ctrl ; i++, __sd_ctrl++) {
		sd_ctrl = __sd_ctrl;
		if (prd->enable_swap_sd)
			sd_ctrl = prd_mod_sd_try(prd, sd_ctrl, &prd->conv_sd);

		ret = prd_sd_test_core(prd, sd_ctrl);

		if (sd_result_list)
			sd_result_list[sd_ctrl->id] = (ret < 0) ? SD_RESULT_FAIL : ret;

		if (ret < 0) {
			t_prd_err(prd, "%s test(%s) failed, %d\n",
				(sd_ctrl->is_os) ? "os" : "raw",
				sd_ctrl->name, ret);
		//	return ret;
		}

		if (sd_ctrl != __sd_ctrl) {
			__sd_ctrl->min = sd_ctrl->min;
			__sd_ctrl->max = sd_ctrl->max;
		}
	}

	return 0;
}

static int prd_sd_check_mode(struct lxs_prd_data *prd, int chk_mode, char *buf)
{
	struct lxs_ts *ts = prd->ts;
	struct lxs_ts_chip *chip = &ts->chip;
	char *chk_str = (chk_mode == LCD_MODE_U3) ? "NP" : "LP";
	int touch_invalid = 0;

	if (prd->dbg_mask & BIT(4))
		return 0;

	if (chip->lcd_mode != chk_mode)
		return lxs_snprintf(buf, 0, "LCD mode is not %s. test canceled\n", chk_str);

	if (chip->driving_mode != LCD_MODE_STOP)
		touch_invalid = (chip->driving_mode != chk_mode);
	else if (chk_mode == LCD_MODE_U3)
		touch_invalid = chip->prev_lpm;

	if (touch_invalid)
		return lxs_snprintf(buf, 0, "Touch is not %s. test canceled\n", chk_str);

	return 0;
}

static int prd_show_do_sd(struct lxs_prd_data *prd, char *prt_buf)
{
	struct lxs_prd_sd_ctrl *sd_ctrl = prd->sd_ctrl;
	int sd_result_list[PRD_CTRL_IDX_SD_MAX] = { 0, };
	int size = 0;
	char *item = NULL;
	int i;
	int ret = 0;

	ret = __prd_sd_pre(prd, prt_buf);
	if (ret < 0)
		goto out;

	ret = prd_sd_test_common(prd, sd_result_list, 1);
	if (ret < 0)
		goto out_test;

	size += lxs_snprintf(prt_buf, size, "%d ", prd->num_sd_ctrl);

	for (i = 0; i < prd->num_sd_ctrl; i++, sd_ctrl++) {
		item = sd_ctrl->name;
	#if defined(__PRD_SUPPORT_SD_MIN_MAX)
		if (sd_ctrl->offset)
			size += lxs_snprintf(prt_buf, size, "%s:%d,%d ",
					item, sd_ctrl->min, sd_ctrl->max);
		else
			size += lxs_snprintf_sd_result(prt_buf, size,
					item, sd_result_list[sd_ctrl->id]);
	#else
		size += lxs_snprintf_sd_result(prt_buf, size,
				item, sd_result_list[sd_ctrl->id]);
	#endif
	}

	size += lxs_snprintf(prt_buf, size, "\n");

out_test:
	__prd_sd_post(prd, prt_buf);

out:
	return size;
}

static ssize_t prd_show_sd(struct device *dev, char *buf)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);
//	struct lxs_ts_chip *chip = &ts->chip;
	struct lxs_prd_data *prd = (struct lxs_prd_data *)ts->prd;
	char log_tmp[64] = {0, };
	int size = 0;
	int ret = 0;

	mutex_lock(&prd->lock);

	if (!prd->num_sd_ctrl) {
		size += lxs_snprintf(buf, size, "sd_np disabled\n");
		t_prd_err(prd, "%s", buf);
		goto out;
	}

	ret = prd_check_exception(prd);
	if (ret) {
		size += lxs_snprintf(buf, size,
					"drv exception(%d) detected, test canceled\n", ret);
		t_prd_err(prd, "%s", buf);
		goto out;
	}

	/* LCD mode check */
	size = prd_sd_check_mode(prd, LCD_MODE_U3, buf);
	if (size)
		goto out;

	lxs_ts_irq_control(ts, 0);

	lxs_ts_watchdog_stop(ts);

	/* file create , time log */
	snprintf(log_tmp, sizeof(log_tmp), "\nsd_np test start");
	prd_write_sd_log(prd, log_tmp, TIME_INFO_SKIP);
	prd_write_sd_log(prd, "\n", TIME_INFO_WRITE);

	t_prd_info(prd, "%s\n", "sd_np test begins");

	prd_fw_version_log(prd);
	prd_ic_run_info_print(prd);

	mutex_lock(&ts->lock);
	size = prd_show_do_sd(prd, buf);
	mutex_unlock(&ts->lock);

	prd_write_sd_log(prd, "sd_np test end\n", TIME_INFO_WRITE);

	t_prd_info(prd, "%s\n", "sd_np test terminated");

	prd_chip_reset(prd);

out:
	mutex_unlock(&prd->lock);

	lxs_ts_delay(100);

	return (ssize_t)size;
}
static TS_ATTR_PRD(sd_np, prd_show_sd, NULL);

static int prd_show_do_lpwg_sd(struct lxs_prd_data *prd, char *prt_buf)
{
	struct lxs_prd_sd_ctrl *lpwg_sd_ctrl = prd->lpwg_sd_ctrl;
	int lpwg_sd_result_list[PRD_CTRL_IDX_LPWG_SD_MAX] = { 0, };
	int size = 0;
	char *item = NULL;
	int i;
	int ret = 0;

	ret = __prd_sd_pre(prd, prt_buf);
	if (ret < 0)
		goto out;

	ret = prd_sd_test_common(prd, lpwg_sd_result_list, 0);
	if (ret < 0)
		goto out_test;

	size += lxs_snprintf(prt_buf, size, "%d ", prd->num_lpwg_sd_ctrl);

	for (i = 0; i < prd->num_lpwg_sd_ctrl; i++, lpwg_sd_ctrl++) {
		item = lpwg_sd_ctrl->name;
	#if defined(__PRD_SUPPORT_SD_MIN_MAX)
		if (lpwg_sd_ctrl->offset)
			size += lxs_snprintf(prt_buf, size, "%s:%d,%d ",
					item, lpwg_sd_ctrl->min, lpwg_sd_ctrl->max);
		else
			size += lxs_snprintf_sd_result(prt_buf, size,
					item, lpwg_sd_result_list[lpwg_sd_ctrl->id]);
	#else
		size += lxs_snprintf_sd_result(prt_buf, size,
				item, lpwg_sd_result_list[lpwg_sd_ctrl->id]);
	#endif
	}

	size += lxs_snprintf(prt_buf, size, "\n");

out_test:
	__prd_sd_post(prd, prt_buf);

out:
	return size;
}

static ssize_t prd_show_lpwg_sd(struct device *dev, char *buf)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);
//	struct lxs_ts_chip *chip = &ts->chip;
	struct lxs_prd_data *prd = (struct lxs_prd_data *)ts->prd;
	char log_tmp[64] = {0, };
	int size = 0;
	int ret = 0;

	mutex_lock(&prd->lock);

	if (!prd->num_lpwg_sd_ctrl) {
		size += lxs_snprintf(buf, size, "sd_lp disabled\n");
		t_prd_err(prd, "%s", buf);
		goto out;
	}

	ret = prd_check_exception(prd);
	if (ret) {
		size += lxs_snprintf(buf, size,
					"drv exception(%d) detected, test canceled\n", ret);
		t_prd_err(prd, "%s", buf);
		goto out;
	}

	/* LCD mode check */
	size = prd_sd_check_mode(prd, LCD_MODE_U0, buf);
	if (size)
		goto out;

	lxs_ts_irq_control(ts, 0);

	/* file create , time log */
	snprintf(log_tmp, sizeof(log_tmp), "\nsd_lp test start");
	prd_write_sd_log(prd, log_tmp, TIME_INFO_SKIP);
	prd_write_sd_log(prd, "\n", TIME_INFO_WRITE);

	t_prd_info(prd, "%s\n", "sd_lp test begins");

	prd_fw_version_log(prd);
	prd_ic_run_info_print(prd);

	mutex_lock(&ts->lock);
	size = prd_show_do_lpwg_sd(prd, buf);
	mutex_unlock(&ts->lock);

	prd_write_sd_log(prd, "sd_lp test end\n", TIME_INFO_WRITE);

	t_prd_info(prd, "%s\n", "sd_lp test terminated");

	prd_chip_reset(prd);

out:
	mutex_unlock(&prd->lock);

	lxs_ts_delay(100);

	return (ssize_t)size;
}
static TS_ATTR_PRD(sd_lp, prd_show_lpwg_sd, NULL);

static void *prd_lookup_sd_ctrl(struct lxs_prd_data *prd, int type)
{
	if (prd->sd_ctrl == NULL) {
		t_prd_err(prd, "%s: no sd_ctrl table\n", __func__);
		return NULL;
	}

	if (type < PRD_CTRL_IDX_SD_MAX) {
		int idx = prd->sd_lookup_table[type];
		if (idx != -1)
			return &prd->sd_ctrl[idx];
	}

	return NULL;
}

static void *prd_lookup_lpwg_sd_ctrl(struct lxs_prd_data *prd, int type)
{
	if (prd->lpwg_sd_ctrl == NULL) {
		t_prd_err(prd, "%s: no lpwg_sd_ctrl table\n", __func__);
		return NULL;
	}

	if (type < PRD_CTRL_IDX_LPWG_SD_MAX) {
		int idx = prd->lpwg_sd_lookup_table[type];
		if (idx != -1)
			return &prd->lpwg_sd_ctrl[idx];
	}

	return NULL;
}

static int prd_sd_cmp_setup(struct lxs_prd_data *prd)
{
	struct lxs_ts *ts = prd->ts;
	struct lxs_prd_sd_ctrl *sd_ctrl = NULL;

	sd_ctrl = prd_lookup_lpwg_sd_ctrl(prd, PRD_CTRL_IDX_LPWG_SD_OPEN);
	if (sd_ctrl) {
		sd_ctrl->cmp_min = ts->plat_data->pt_open_cmp[0];
		sd_ctrl->cmp_max = ts->plat_data->pt_open_cmp[1];
		t_prd_info(prd, "%s: cmp set: %s, min %d, max %d\n",
			__func__, sd_ctrl->name, sd_ctrl->cmp_min, sd_ctrl->cmp_max);
	}

	sd_ctrl = prd_lookup_lpwg_sd_ctrl(prd, PRD_CTRL_IDX_LPWG_SD_SHORT_GND);
	if (sd_ctrl) {
		sd_ctrl->cmp_min = ts->plat_data->pt_short_gnd_cmp[0];
		sd_ctrl->cmp_max = ts->plat_data->pt_short_gnd_cmp[1];
		t_prd_info(prd, "%s: cmp set: %s, min %d, max %d\n",
			__func__, sd_ctrl->name, sd_ctrl->cmp_min, sd_ctrl->cmp_max);
	}

	sd_ctrl = prd_lookup_lpwg_sd_ctrl(prd, PRD_CTRL_IDX_LPWG_SD_SHORT_TRX);
	if (sd_ctrl) {
		sd_ctrl->cmp_min = ts->plat_data->pt_short_trx_cmp[0];
		sd_ctrl->cmp_max = ts->plat_data->pt_short_trx_cmp[1];
		t_prd_info(prd, "%s: cmp set: %s, min %d, max %d\n",
			__func__, sd_ctrl->name, sd_ctrl->cmp_min, sd_ctrl->cmp_max);
	}

	sd_ctrl = prd_lookup_lpwg_sd_ctrl(prd, PRD_CTRL_IDX_LPWG_SD_SHORT_VDD);
	if (sd_ctrl) {
		sd_ctrl->cmp_min = ts->plat_data->pt_short_vdd_cmp[0];
		sd_ctrl->cmp_max = ts->plat_data->pt_short_vdd_cmp[1];
		t_prd_info(prd, "%s: cmp set: %s, min %d, max %d\n",
			__func__, sd_ctrl->name, sd_ctrl->cmp_min, sd_ctrl->cmp_max);
	}

	sd_ctrl = prd_lookup_sd_ctrl(prd, PRD_CTRL_IDX_SD_SYNC);
	if (sd_ctrl) {
		int sync_v_cmp[2];

		sync_v_cmp[0] = ts->plat_data->pt_sync_v_cmp[0];
		sync_v_cmp[1] = ts->plat_data->pt_sync_v_cmp[1];
		t_prd_info(prd, "%s: cmp set: %s(V), min %d, max %d\n",
			__func__, sd_ctrl->name, sync_v_cmp[0], sync_v_cmp[1]);

		sd_ctrl->cmp_min = sync_v_cmp[0];
		sd_ctrl->cmp_max = sync_v_cmp[1];
	}

	return 0;
}

static void __run_print_rawdata(char *prt_buf, char *raw_buf,
			int row_size, int col_size, int data_fmt, u32 flag)
{
	uint16_t *rawdata_u16 = (uint16_t *)raw_buf;
	int16_t *rawdata_s16 = (int16_t *)raw_buf;
	s8 *rawdata_s8 = (s8 *)raw_buf;
	int is_16bit = !!(data_fmt & PRD_DATA_FMT_MASK);
	int is_u16bit = !!(data_fmt & PRD_DATA_FMT_U16);
	int curr_raw;
	char tmp[RUN_DATA_WORD_LEN];
	int i, j;

	if (prt_buf == NULL)
		return;

	if (data_fmt & PRD_DATA_FMT_RXTX) {
		col_size += row_size;
		row_size = 1;
	}

	for (i = 0; i < row_size; i++) {
		for (j = 0; j < col_size; j++) {
			if (is_16bit)
				curr_raw = (is_u16bit) ? *rawdata_u16++ : *rawdata_s16++;
			else
				curr_raw = *rawdata_s8++;

			memset(tmp, 0x00, RUN_DATA_WORD_LEN);
			if ((i == (row_size - 1)) && (j == (col_size - 1)))
				snprintf(tmp, RUN_DATA_WORD_LEN, "%d", curr_raw);
			else
				snprintf(tmp, RUN_DATA_WORD_LEN, "%d,", curr_raw);
			strncat(prt_buf, tmp, RUN_DATA_WORD_LEN);
		}
	}
}

static int __run_sd_func(void *ts_data, int type, int *min, int *max, char *prt_buf, int opt, int is_sd)
{
	struct lxs_ts *ts = ts_data;
	struct lxs_prd_data *prd = ts->prd;
	struct lxs_prd_sd_ctrl *sd_ctrl;
	struct lxs_prd_sd_ctrl *ctrl = &prd->sd_run;
	char *raw_buf;
	int ret;

	if (min)
		*min = 0;

	if (max)
		*max = 0;

	if (prd == NULL)
		return -EINVAL;

	if (is_sd)
		sd_ctrl = prd_lookup_sd_ctrl(prd, type);
	else
		sd_ctrl = prd_lookup_lpwg_sd_ctrl(prd, type);
	if (sd_ctrl == NULL) {
		t_prd_err(prd, "%s: %s not support type %d\n",
			__func__, (is_sd) ? "sd_np" : "sd_lp", type);
		return -EINVAL;
	}

	memcpy(ctrl, sd_ctrl, sizeof(struct lxs_prd_sd_ctrl));

	if (opt & BIT(0))
		ctrl->flag |= PRD_RAW_FLAG_FW_SKIP;
	else
		ctrl->flag &= ~PRD_RAW_FLAG_FW_SKIP;

	if (opt & BIT(8))
		ctrl->cmp_sd_func = prd_cmp_sd_raw;
	else if (opt & BIT(9))
		ctrl->cmp_sd_func = prd_cmp_sd_rxtx;

	if (prd->enable_swap_sd)
		ctrl = prd_mod_sd_try(prd, ctrl, &prd->conv_sd);

	ret = prd_sd_test_core(prd, ctrl);
	if (ret < 0)
		return ret;

	raw_buf = (ctrl->flag & PRD_RAW_FLAG_DATA_TMP) ? ctrl->raw_tmp : ctrl->raw_buf;

	__run_print_rawdata(prt_buf, raw_buf,
		ctrl->row_act, ctrl->col_act,
		ctrl->data_fmt, ctrl->flag);

	if (min)
		*min = ctrl->min;

	if (max)
		*max = ctrl->max;

	return (ret != SD_RESULT_PASS) ? SD_RESULT_FAIL : SD_RESULT_PASS;
}

static int __used run_np_calb(void *ts_data, int *min, int *max, char *prt_buf, int opt)
{
	return __run_sd_func(ts_data, PRD_CTRL_IDX_SD_CALB, min, max, prt_buf, opt | BIT(0), 1);
}

static int __used run_np_sync(void *ts_data, int *min, int *max, char *prt_buf, int opt)
{
	return __run_sd_func(ts_data, PRD_CTRL_IDX_SD_SYNC, min, max, prt_buf, opt | BIT(0), 1);
}

static int __used run_np_irq(void *ts_data, int *min, int *max, char *prt_buf, int opt)
{
	return __run_sd_func(ts_data, PRD_CTRL_IDX_SD_IRQ, min, max, prt_buf, opt | BIT(0), 1);
}

static int __used run_np_rst(void *ts_data, int *min, int *max, char *prt_buf, int opt)
{
	return __run_sd_func(ts_data, PRD_CTRL_IDX_SD_RST, min, max, prt_buf, opt | BIT(0), 1);
}

static int __used run_np_raw(void *ts_data, int *min, int *max, char *prt_buf, int opt)
{
	return __run_sd_func(ts_data, PRD_CTRL_IDX_SD_U3_M2_RAW, min, max, prt_buf, opt, 1);
}

static int __used run_np_raw_self(void *ts_data, int *min, int *max, char *prt_buf, int opt)
{
	return __run_sd_func(ts_data, PRD_CTRL_IDX_SD_U3_M2_RAW_SELF, min, max, prt_buf, opt, 1);
}

static int __used run_np_jitter(void *ts_data, int *min, int *max, char *prt_buf, int opt)
{
	return __run_sd_func(ts_data, PRD_CTRL_IDX_SD_U3_JITTER, min, max, prt_buf, opt, 1);
}

static int __used run_np_jitter_self(void *ts_data, int *min, int *max, char *prt_buf, int opt)
{
	return __run_sd_func(ts_data, PRD_CTRL_IDX_SD_U3_JITTER_SELF, min, max, prt_buf, opt, 1);
}

static int __used run_lp_open_cmp(void *ts_data, int *min, int *max, char *prt_buf, int opt)
{
	struct lxs_ts *ts = ts_data;
	struct lxs_prd_data *prd = ts->prd;

	t_prd_info(prd, "======== %s ========\n", "LP_OPEN_CMP");

	return __run_sd_func(ts_data, PRD_CTRL_IDX_LPWG_SD_OPEN, min, max, prt_buf, opt | BIT(8), 0);
}

static int __used run_lp_short_cmp(void *ts_data, int *min, int *max, char *prt_buf, int opt)
{
	struct lxs_ts *ts = ts_data;
	struct lxs_prd_data *prd = ts->prd;
	int ret[3] = {SD_RESULT_FAIL, };
	int i;

	t_prd_info(prd, "======== %s ========\n", "LP_SHORT_CMP");

	ret[0] = __run_sd_func(ts_data, PRD_CTRL_IDX_LPWG_SD_SHORT_GND, min, max, prt_buf, opt | BIT(9), 0);

	ret[1] = __run_sd_func(ts_data, PRD_CTRL_IDX_LPWG_SD_SHORT_TRX, min, max, prt_buf, opt | BIT(0) | BIT(9), 0);

	ret[2] = __run_sd_func(ts_data, PRD_CTRL_IDX_LPWG_SD_SHORT_VDD, min, max, prt_buf, opt | BIT(0) | BIT(9), 0);

	for (i = 0; i < ARRAY_SIZE(ret); i++) {
		if (ret[i] != SD_RESULT_PASS)
			return (ret[i] < 0) ? ret[i] : SD_RESULT_FAIL;
	}

	return SD_RESULT_PASS;
}

static int __used run_lp_open(void *ts_data, int *min, int *max, char *prt_buf, int opt)
{
	return __run_sd_func(ts_data, PRD_CTRL_IDX_LPWG_SD_OPEN, min, max, prt_buf, opt, 0);
}

static int __used run_lp_gap_x(void *ts_data, int *min, int *max, char *prt_buf, int opt)
{
	return __run_sd_func(ts_data, PRD_CTRL_IDX_LPWG_SD_GAP_X, min, max, prt_buf, opt, 0);
}

static int __used run_lp_gap_y(void *ts_data, int *min, int *max, char *prt_buf, int opt)
{
	return __run_sd_func(ts_data, PRD_CTRL_IDX_LPWG_SD_GAP_Y, min, max, prt_buf, opt, 0);
}

static int __used run_lp_gap_a(void *ts_data, int *min, int *max, char *prt_buf, int opt)
{
	return __run_sd_func(ts_data, PRD_CTRL_IDX_LPWG_SD_GAP_A, min, max, prt_buf, opt, 0);
}

static int __used run_lp_short_gnd(void *ts_data, int *min, int *max, char *prt_buf, int opt)
{
	return __run_sd_func(ts_data, PRD_CTRL_IDX_LPWG_SD_SHORT_GND, min, max, prt_buf, opt, 0);
}

static int __used run_lp_short_trx(void *ts_data, int *min, int *max, char *prt_buf, int opt)
{
	return __run_sd_func(ts_data, PRD_CTRL_IDX_LPWG_SD_SHORT_TRX, min, max, prt_buf, opt, 0);
}

static int __used run_lp_short_vdd(void *ts_data, int *min, int *max, char *prt_buf, int opt)
{
	return __run_sd_func(ts_data, PRD_CTRL_IDX_LPWG_SD_SHORT_VDD, min, max, prt_buf, opt, 0);
}

static int __used run_lp_raw(void *ts_data, int *min, int *max, char *prt_buf, int opt)
{
	return __run_sd_func(ts_data, PRD_CTRL_IDX_LPWG_SD_U0_M2_RAW, min, max, prt_buf, opt, 0);
}

static int __used run_lp_raw_self(void *ts_data, int *min, int *max, char *prt_buf, int opt)
{
	return __run_sd_func(ts_data, PRD_CTRL_IDX_LPWG_SD_U0_M2_RAW_SELF, min, max, prt_buf, opt, 0);
}

static int __used run_lp_jitter(void *ts_data, int *min, int *max, char *prt_buf, int opt)
{
	return __run_sd_func(ts_data, PRD_CTRL_IDX_LPWG_SD_U0_JITTER, min, max, prt_buf, opt, 0);
}

static int __used run_lp_jitter_self(void *ts_data, int *min, int *max, char *prt_buf, int opt)
{
	return __run_sd_func(ts_data, PRD_CTRL_IDX_LPWG_SD_U0_JITTER_SELF, min, max, prt_buf, opt, 0);
}

static ssize_t run_xx_op(struct device *dev, char *buf, const char *name, int buf_size, int is_np, int reset, void *__run_op)
{
	int (*run_op)(void *ts_data, int *min, int *max, char *prt_buf, int opt) = __run_op;
	struct lxs_ts *ts = dev_get_drvdata(dev);
//	struct lxs_ts_chip *chip = &ts->chip;
	struct lxs_prd_data *prd = (struct lxs_prd_data *)ts->prd;
	char *prt_buf = NULL;
	int chk_mode = (is_np & 0x1) ? LCD_MODE_U3 : LCD_MODE_U0;
	int size = 0;
	int min = 0;
	int max = 0;
	int is_ret, is_min, is_max, is_mmd;
	int ret = 0;

	is_ret = is_np & BIT(1);
	is_min = is_np & BIT(2);
	is_max = is_np & BIT(3);
	is_mmd = is_np & BIT(4);
	is_np &= 0x1;

	mutex_lock(&prd->lock);

	ret = prd_check_exception(prd);
	if (ret) {
		size += lxs_snprintf(buf, size,
					"drv exception(%d) detected, test canceled\n", ret);
		t_prd_err(prd, "%s", buf);
		goto out;
	}

	size = prd_sd_check_mode(prd, chk_mode, buf);
	if (size)
		goto out;

	if (buf_size) {
		prt_buf = kzalloc(buf_size, GFP_KERNEL);
		if (prt_buf == NULL) {
			size += lxs_snprintf(buf, size, "kzalloc failed\n");
			t_prd_err(prd, "%s", buf);
			goto out;
		}
	}

	lxs_ts_irq_control(ts, 0);

	lxs_ts_watchdog_stop(ts);

	mutex_lock(&ts->lock);

	ret = __prd_sd_pre(prd, buf);
	if (ret < 0)
		goto out_fault;

	ret = -EFAULT;
	if (run_op)
		ret = run_op(ts, &min, &max, prt_buf, 0);
	else
		t_prd_err(prd, "%s: no run_op\n", __func__);

out_fault:
	if (name)
		size += lxs_snprintf(buf, size, "%s:", name);

	if (ret < 0) {
		if (is_ret)
			size += lxs_snprintf(buf, size, "%s\n", SD_RESULT_STR_FAIL);
		else
			size += lxs_snprintf(buf, size, "NG\n");
	} else {
		if (prt_buf)
			size += lxs_snprintf(buf, size, "%s\n", prt_buf);
		else
			if (is_ret)
				size += lxs_snprintf(buf, size, "%s\n", (ret != SD_RESULT_PASS) ? SD_RESULT_STR_FAIL : SD_RESULT_STR_PASS);
			else if (is_min)
				size += lxs_snprintf(buf, size, "%d\n", min);
			else if (is_max)
				size += lxs_snprintf(buf, size, "%d\n", max);
			else if (is_mmd)
				size += lxs_snprintf(buf, size, "%d,%d,%d\n", max, min, max-min);
			else
				size += lxs_snprintf(buf, size, "%d,%d\n", min, max);
	}
	mutex_unlock(&ts->lock);

	if (prd->dbg_mask & BIT(0))
		reset = 0;

	if (reset)
		prd_chip_reset(prd);

out:
	if (prt_buf)
		kfree(prt_buf);

	mutex_unlock(&prd->lock);

	return (ssize_t)size;
}

static const void *prd_sd_sub_op[] = {
	[PRD_SD_SUB_IDX_NP_RAW]			= run_np_raw,
	[PRD_SD_SUB_IDX_NP_RAW_SELF]	= run_np_raw_self,
	[PRD_SD_SUB_IDX_NP_JITTER]		= run_np_jitter,
	[PRD_SD_SUB_IDX_NP_JITTER_SELF]	= run_np_jitter_self,
	[PRD_SD_SUB_IDX_NP_CALB]		= run_np_calb,
	[PRD_SD_SUB_IDX_NP_SYNC]		= run_np_sync,
	[PRD_SD_SUB_IDX_NP_IRQ] 		= run_np_irq,
	[PRD_SD_SUB_IDX_NP_RST] 		= run_np_rst,
	/* */
	[PRD_SD_SUB_IDX_LP_OPEN_CMP]	= run_lp_open_cmp,
	[PRD_SD_SUB_IDX_LP_SHORT_CMP]	= run_lp_short_cmp,
	[PRD_SD_SUB_IDX_LP_OPEN]		= run_lp_open,
	[PRD_SD_SUB_IDX_LP_GAP_X] 		= run_lp_gap_x,
	[PRD_SD_SUB_IDX_LP_GAP_Y]		= run_lp_gap_y,
	[PRD_SD_SUB_IDX_LP_GAP_A]		= run_lp_gap_a,
	[PRD_SD_SUB_IDX_LP_SHORT_GND]	= run_lp_short_gnd,
	[PRD_SD_SUB_IDX_LP_SHORT_TRX]	= run_lp_short_trx,
	[PRD_SD_SUB_IDX_LP_SHORT_VDD]	= run_lp_short_vdd,
	[PRD_SD_SUB_IDX_LP_RAW]			= run_lp_raw,
	[PRD_SD_SUB_IDX_LP_RAW_SELF]	= run_lp_raw_self,
	[PRD_SD_SUB_IDX_LP_JITTER]		= run_lp_jitter,
	[PRD_SD_SUB_IDX_LP_JITTER_SELF]	= run_lp_jitter_self,
};

static ssize_t prd_show_sd_sub_common(struct device *dev, char *buf, char *name, int size, int idx, int is_np)
{
	void *ops = NULL;
	int reset = 1;

	if (idx < ARRAY_SIZE(prd_sd_sub_op))
		ops = (void *)prd_sd_sub_op[idx];

	return run_xx_op(dev, buf, name, size, is_np, reset, ops);
}

TS_ATTR_PRD_SD_OD(sd_np_rst, NP_RST);
TS_ATTR_PRD_SD_OD(sd_np_irq, NP_IRQ);
TS_ATTR_PRD_SD_OD(sd_np_sync, NP_SYNC);
TS_ATTR_PRD_SD_OD(sd_np_calb, NP_CALB);

TS_ATTR_PRD_SD_NP(sd_np_a_raw, NP_RAW, 1);
TS_ATTR_PRD_SD_NP(sd_np_a_raw_self, NP_RAW_SELF, 1);
TS_ATTR_PRD_SD_NP(sd_np_a_jitter, NP_JITTER, 1);
TS_ATTR_PRD_SD_NP(sd_np_a_jitter_self, NP_JITTER_SELF, 1);
TS_ATTR_PRD_SD_NP(sd_np_m_raw, NP_RAW, 0);
TS_ATTR_PRD_SD_NP(sd_np_m_raw_self, NP_RAW_SELF, 0);
//TS_ATTR_PRD_SD_NP(sd_np_m_jitter, NP_JITTER, 0);
//TS_ATTR_PRD_SD_NP(sd_np_m_jitter_self, NP_JITTER_SELF, 0);
TS_ATTR_PRD_SD_NP_MIN(sd_np_m_jitter_min, NP_JITTER, 0);
TS_ATTR_PRD_SD_NP_MAX(sd_np_m_jitter_max, NP_JITTER, 0);
TS_ATTR_PRD_SD_NP_MMD(sd_np_m_jitter_mmd, NP_JITTER, 0);
TS_ATTR_PRD_SD_NP_MIN(sd_np_m_jitter_self_min, NP_JITTER_SELF, 0);
TS_ATTR_PRD_SD_NP_MAX(sd_np_m_jitter_self_max, NP_JITTER_SELF, 0);
TS_ATTR_PRD_SD_NP_MMD(sd_np_m_jitter_self_mmd, NP_JITTER_SELF, 0);

TS_ATTR_PRD_SD_LP(sd_lp_a_open, LP_OPEN, 1);
TS_ATTR_PRD_SD_LP(sd_lp_a_gap_x, LP_GAP_X, 1);
TS_ATTR_PRD_SD_LP(sd_lp_a_gap_y, LP_GAP_Y, 1);
TS_ATTR_PRD_SD_LP(sd_lp_a_gap_a, LP_GAP_A, 1);
TS_ATTR_PRD_SD_LP(sd_lp_a_short_gnd, LP_SHORT_GND, 1);
TS_ATTR_PRD_SD_LP(sd_lp_a_short_trx, LP_SHORT_TRX, 1);
TS_ATTR_PRD_SD_LP(sd_lp_a_short_vdd, LP_SHORT_VDD, 1);
TS_ATTR_PRD_SD_LP(sd_lp_a_raw, LP_RAW, 1);
TS_ATTR_PRD_SD_LP(sd_lp_a_raw_self, LP_RAW_SELF, 1);
TS_ATTR_PRD_SD_LP(sd_lp_a_jitter, LP_JITTER, 1);
TS_ATTR_PRD_SD_LP(sd_lp_a_jitter_self, LP_JITTER_SELF, 1);
TS_ATTR_PRD_SD_LP_CMP(sd_lp_c_open, LP_OPEN_CMP, 0);
TS_ATTR_PRD_SD_LP_CMP(sd_lp_c_short, LP_SHORT_CMP, 0);
TS_ATTR_PRD_SD_LP(sd_lp_m_open, LP_OPEN, 0);
TS_ATTR_PRD_SD_LP(sd_lp_m_gap_x, LP_GAP_X, 0);
TS_ATTR_PRD_SD_LP(sd_lp_m_gap_y, LP_GAP_Y, 0);
TS_ATTR_PRD_SD_LP(sd_lp_m_gap_a, LP_GAP_A, 0);
TS_ATTR_PRD_SD_LP(sd_lp_m_short_gnd, LP_SHORT_GND, 0);
TS_ATTR_PRD_SD_LP(sd_lp_m_short_trx, LP_SHORT_TRX, 0);
TS_ATTR_PRD_SD_LP(sd_lp_m_short_vdd, LP_SHORT_VDD, 0);
TS_ATTR_PRD_SD_LP(sd_lp_m_raw, LP_RAW, 0);
TS_ATTR_PRD_SD_LP(sd_lp_m_raw_self, LP_RAW_SELF, 0);
//TS_ATTR_PRD_SD_LP(sd_lp_m_jitter, LP_JITTER, 0);
//TS_ATTR_PRD_SD_LP(sd_lp_m_jitter_self, LP_JITTER_SELF, 0);
TS_ATTR_PRD_SD_LP_MIN(sd_lp_m_jitter_min, LP_JITTER, 0);
TS_ATTR_PRD_SD_LP_MAX(sd_lp_m_jitter_max, LP_JITTER, 0);
TS_ATTR_PRD_SD_LP_MMD(sd_lp_m_jitter_mmd, LP_JITTER, 0);
TS_ATTR_PRD_SD_LP_MIN(sd_lp_m_jitter_self_min, LP_JITTER_SELF, 0);
TS_ATTR_PRD_SD_LP_MAX(sd_lp_m_jitter_self_max, LP_JITTER_SELF, 0);
TS_ATTR_PRD_SD_LP_MMD(sd_lp_m_jitter_self_mmd, LP_JITTER_SELF, 0);
#endif	/* __PRD_SUPPORT_SD_TEST */

static void *prd_lookup_raw_ctrl(struct lxs_prd_data *prd, int type)
{
	if (type < PRD_CTRL_IDX_MAX) {
		int idx = prd->raw_lookup_table[type];
		if (idx != -1)
			return &prd->raw_ctrl[idx];
	}

	return NULL;
}

static ssize_t prd_get_data_core(struct lxs_prd_data *prd, struct lxs_prd_raw_ctrl *ctrl, char *raw_buf)
{
	struct lxs_ts *ts = (struct lxs_ts *)prd->ts;
	int data_size = ctrl->data_size;
	int ret = 0;

	if (raw_buf == NULL)
		raw_buf = prd->raw_buf;

	memset(raw_buf, 0, data_size);

	ctrl->row_act = ctrl->row_size;
	ctrl->col_act = ctrl->col_size;
	ctrl->raw_buf = raw_buf;
	ctrl->raw_tmp = prd->raw_tmp;

	mutex_lock(&ts->lock);

	ret = prd_stop_firmware(prd, ctrl);
	if (ret < 0)
		goto out;

	if (ctrl->get_raw_func) {
		ret = ctrl->get_raw_func(prd, ctrl);
		if (ret < 0)
			goto out;
	}

	ret = prd_start_firmware(prd, ctrl);
	if (ret < 0)
		goto out;

	if (ctrl->mod_raw_func) {
		ret = ctrl->mod_raw_func(prd, ctrl);
		if (ret < 0)
			goto out;
	}

out:
	mutex_unlock(&ts->lock);
	return ret;
}

static ssize_t prd_show_check_power(struct device *dev, char *buf)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);
	int size = 0;

	if (ts->shutdown_called) {
		size += sprintf(buf, "shutdown called\n");
		goto out;
	}

	if (ts->chip.lcd_mode == LCD_MODE_STOP) {
		size += sprintf(buf, "touch stopped\n");
		goto out;
	}

out:
	return (ssize_t)size;
}

static ssize_t prd_show_common(struct device *dev, char *buf, int type)
{
	struct lxs_prd_data *prd = __dev_get_prd(dev);
	struct lxs_prd_raw_ctrl *ctrl = NULL;
	char *swap_str = "";
	int prt_size = 0;
	int size = 0;
	int ret = 0;

	size = prd_show_check_power(dev, buf);
	if (size)
		return (ssize_t)size;

	mutex_lock(&prd->lock);

	ret = prd_check_exception(prd);
	if (ret) {
		size = sprintf(buf, "drv exception(%d) detected, test canceled\n", ret);
		t_prd_err(prd, "%s", buf);
		goto out;
	}

	ctrl = prd_lookup_raw_ctrl(prd, type);
	if (ctrl == NULL) {
		size = sprintf(buf, "type(%d) disabled, test canceled\n", ret);
		t_prd_err(prd, "%s", buf);
		goto out;
	}

	if (ctrl->flag & PRD_RAW_FLAG_TYPE_SYS) {
		if (prd->enable_swap_sys)
			ctrl = prd_mod_raw_try(prd, ctrl, &prd->conv_raw);
	} else {
		if (prd->enable_swap_raw)
			ctrl = prd_mod_raw_try(prd, ctrl, &prd->conv_raw);
	}

	if (ctrl->is_dim)
		swap_str = "(S) ";

	ctrl->prt_buf = prd->log_pool;

	t_prd_info(prd, "======== %s(%d %d %d 0x%X) %s========\n",
		ctrl->name, ctrl->cmd, ctrl->row_size, ctrl->col_size, ctrl->offset, swap_str);
	prt_size += lxs_prd_buf_snprintf(ctrl->prt_buf, prt_size,
					"======== %s(%d %d %d 0x%X) %s========\n",
					ctrl->name, ctrl->cmd,
					ctrl->row_size, ctrl->col_size, ctrl->offset, swap_str);

	ret = prd_get_data_core(prd, ctrl, prd->raw_buf);
	if (ret < 0) {
		t_prd_err(prd, "%s(%d) failed, %d\n", __func__, type, ret);
		prt_size += lxs_prd_buf_snprintf(ctrl->prt_buf, prt_size,
						"%s(%d) failed, %d\n", __func__, type, ret);
		goto done;
	}

	if (ctrl->prt_raw_func)
		ctrl->prt_raw_func(prd, ctrl, prt_size);

done:
	size += lxs_prd_buf_snprintf(buf, size, "%s\n", ctrl->prt_buf);

#if 0
	size += lxs_prd_buf_snprintf(buf, size, "Get Data[%s] result: %s\n",
				ctrl->name, (ret < 0) ? "Fail" : "Pass");
#endif

out:
	mutex_unlock(&prd->lock);

	return (ssize_t)size;
}

TS_ATTR_PRD_BIN(debug_buf, DEBUG, 0);

TS_ATTR_PRD_RAW(rawdata, RAW, 1);
TS_ATTR_PRD_RAW(base, BASE, 1);
TS_ATTR_PRD_RAW(delta, DELTA, 1);
TS_ATTR_PRD_RAW(label, LABEL, 1);

static int prd_show_app_blocked(struct lxs_ts *ts)
{
	struct lxs_ts_chip *chip = &ts->chip;

	if (atomic_read(&chip->init) == TC_IC_INIT_NEED)
		return 1;

	if (ts->state_core == TS_CORE_UPGRADE)
		return 2;

	return 0;
}

static ssize_t prd_show_app_op_end(struct lxs_prd_data *prd, char *buf, int prev_mode)
{
	struct lxs_ts *ts = (struct lxs_ts *)prd->ts;
	struct lxs_prd_raw_ctrl *raw_ctrl = prd->app_raw_ctrl;
	int ret = 0;

	if (prd_show_app_blocked(ts))
		return 1;

	buf[0] = REPORT_END_RS_OK;
	if (prev_mode != REPORT_OFF) {
		prd->app_mode = REPORT_OFF;
		ret = prd_start_firmware(prd, raw_ctrl);
		if (ret < 0) {
			t_prd_err(prd, "prd_start_firmware failed, %d\n", ret);
			buf[0] = REPORT_END_RS_NG;
		}
	}

	return 1;
}

static ssize_t prd_show_app_common(struct device *dev, char *buf, int mode)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);
	struct lxs_prd_data *prd = (struct lxs_prd_data *)ts->prd;
	struct lxs_prd_raw_ctrl *ctrl = NULL;
	int prev_mode = prd->app_mode;
	int data_size = 0;
	int ctrl_id = -1;
	int is_16bit = 0;
	int do_conv_16to8 = 0;

	mutex_lock(&prd->lock);

	prd->app_read = 1;

	if (prd_show_app_blocked(ts)) {
		memset(buf, 0, PAGE_SIZE);
		data_size = 1;
		goto out;
	}

	if (mode < REPORT_MAX) {
		t_prd_dbg_base(prd, "show app mode : %s(%d)\n",
			prd_app_mode_str[mode], mode);
	}

	if (mode == REPORT_OFF) {
		data_size = prd_show_app_op_end(prd, buf, prev_mode);
		goto out;
	}

	prd->app_mode = mode;

	ctrl_id = prd_app_mode_ctrl_id[mode];

	if (ctrl_id < 0) {
		t_prd_err(prd, "unknown mode, %d\n", mode);
		if (prev_mode != REPORT_OFF) {
			prd_show_app_op_end(prd, buf, prev_mode);
		}

		data_size = PAGE_SIZE;
		memset(buf, 0, PAGE_SIZE);
		goto out;
	}

	ctrl = prd_lookup_raw_ctrl(prd, ctrl_id);
	if (ctrl == NULL)
		goto out;

	if (prd->enable_swap_app)
		ctrl = prd_mod_raw_try(prd, ctrl, &prd->conv_app);

	prd->app_raw_ctrl = ctrl;

	data_size = ctrl->data_size;

	is_16bit = !!(ctrl->data_fmt & PRD_DATA_FMT_MASK);

	switch (mode) {
	case REPORT_LABEL:
		do_conv_16to8 = is_16bit;
		break;
	}

	/*
	 * if do_conv_16to8
	 *   memory > prd->raw_app > prd->raw_buf > buf
	 * else
	 *   mempry > prd->raw_app > buf
	 */
	prd_get_data_core(prd, ctrl, prd->raw_app);

	if (do_conv_16to8) {
		data_size >>= 1;	//node size

		__buff_16to8(prd, prd->raw_buf, prd->raw_app, data_size);
		ctrl->raw_buf = prd->raw_buf;
	}

	memcpy(buf, ctrl->raw_buf, data_size);

out:
	prd->app_read = 0;

	mutex_unlock(&prd->lock);

	if (ts->prd_quirks & PRD_QUIRK_RAW_RETURN_MODE_VAL)
		return (ssize_t)mode;

	return (ssize_t)data_size;
}

TS_ATTR_PRD_APP(raw, RAW);
TS_ATTR_PRD_APP(base, BASE);
TS_ATTR_PRD_APP(delta, DELTA);
TS_ATTR_PRD_APP(label, LABEL);
TS_ATTR_PRD_APP(debug_buf, DEBUG_BUG);
TS_ATTR_PRD_APP(end, OFF);

static ssize_t prd_show_app_info(struct device *dev, char *buf)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);
	struct lxs_prd_data *prd = (struct lxs_prd_data *)ts->prd;
	u32 temp = prd->sysfs_flag;
	u32 row_size = prd->row;
	u32 col_size = prd->col;

	if (prd->enable_swap_app) {
		if (prd->raw_dim_ctl.swap)
			swap(row_size, col_size);
	}

	memset(buf, 0, PRD_APP_INFO_SIZE);

	((u32 *)buf)[0] = temp;
	buf[8] = row_size;
	buf[9] = col_size;
	buf[10] = 0;	/* col_add */
	buf[11] = prd->ch;
	buf[12] = 2;	/* m1_col size */

	buf[19] = prd->enable_swap_app;
	buf[20] = prd->raw_dim_ctl.type;
	buf[21] = prd->raw_dim_ctl.rev_rx;
	buf[22] = prd->raw_dim_ctl.rev_tx;
	buf[23] = prd->raw_dim_ctl.rev_ord;
	buf[24] = prd->sys_dim_ctl.type;
	buf[25] = prd->sys_dim_ctl.rev_rx;
	buf[26] = prd->sys_dim_ctl.rev_tx;
	buf[27] = prd->sys_dim_ctl.rev_ord;

	if (prd_show_app_blocked(ts))
		t_prd_info(prd, "%s\n", "[Warning] prd app access blocked");

	t_prd_info(prd, "prd info: f %08Xh, r %d, c %d, ca %d, ch %d, m1 %d\n",
		temp, prd->row, prd->col, 0, prd->ch, 2);
	t_prd_info(prd, "prd info: sw %d, rd %d %d %d %d, sd %d %d %d %d\n",
		buf[19], buf[20], buf[21], buf[22], buf[23], buf[24], buf[25], buf[26], buf[27]);

	return PRD_APP_INFO_SIZE + !!(prd->self);
}
static TS_ATTR_PRD(prd_app_info, prd_show_app_info, NULL);

static ssize_t prd_show_app_reg(struct device *dev, char *buf)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);
	struct lxs_prd_data *prd = (struct lxs_prd_data *)ts->prd;
	u32 addr = prd->app_reg_addr;
	u32 size = prd->app_reg_size;
	int ret = 0;

	if (prd_show_app_blocked(ts)) {
		t_prd_err(prd, "%s blocked\n", __func__);
		return (ssize_t)size;
	}

	if (size) {
		ret = lxs_hal_reg_read(ts, addr, buf, size);
		if (ret < 0)
			t_prd_err(prd, "prd_app_reg read(addr 0x%X, size 0x%X) failed, %d\n",
				addr, size, ret);
	}

	return (ssize_t)size;
}

/*
 * [Read control]
 * echo 0x1234 0x100 0x00 > prd_app_reg : Set read access w/o kernel log
 * echo 0x1234 0x100 0x01 > prd_app_reg : Set read access w/h kernel log
 * cat prd_app_reg                      : Actual read (binary)
 *
 * [Write control]
 * echo 0x1234 0x100 0x10 > prd_app_reg : Direct reg. write w/o kernel log
 * echo 0x1234 0x100 0x11 > prd_app_reg : Direct reg. write w/h kernel log
 * : 0x100 is data itself, not size value
 */
static ssize_t prd_store_app_reg(struct device *dev,
				const char *buf, size_t count)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);
	struct lxs_prd_data *prd = (struct lxs_prd_data *)ts->prd;
	u32 addr = 0;
	u32 size = 4;
	u32 flag = 0;
	int ret = 0;

	if (prd_show_app_blocked(ts)) {
		t_prd_err(prd, "%s blocked\n", __func__);
		return count;
	}

	if (sscanf(buf, "%X %X %X", &addr, &size, &flag) <= 0) {
		t_prd_err(prd, "%s\n", "Invalid param");
		return count;
	}

	if (addr >= 0x0FFF) {
		t_prd_err(prd, "Invalid addr, %X\n", addr);
		return count;
	}

	if (flag & APP_REG_FLAG_WR) {
		u32 data = size;

		ret = lxs_hal_write_value(ts, addr, data);
		if (ret < 0) {
			t_prd_err(prd, "prd_app_reg write(addr 0x%X, data 0x%X) failed, %d\n",
				addr, data, ret);
		} else {
			if (flag & APP_REG_FLAG_LOG) {
				t_prd_info(prd, "prd_app_reg write(addr 0x%X, data 0x%X)\n",
					addr, data);
			}
		}
		return count;
	}

	if (!size || (size > PAGE_SIZE)) {
		t_prd_err(prd, "Invalid size, %X\n", size);
		return count;
	}

	prd->app_reg_addr = addr;
	prd->app_reg_size = size;

	if (flag & APP_REG_FLAG_LOG)
		t_prd_info(prd, "prd_app_reg set(addr 0x%X, size 0x%X)\n", addr, size);

	return count;
}
static TS_ATTR_PRD(prd_app_reg, prd_show_app_reg, prd_store_app_reg);

static ssize_t prd_show_dbg_mask(struct device *dev, char *buf)
{
	struct lxs_prd_data *prd = __dev_get_prd(dev);
	int size = 0;

	size += lxs_snprintf(buf, size,
				"prd->dbg_mask  %08Xh\n",
				prd->dbg_mask);
	size += lxs_snprintf(buf, size,
				"t_prd_dbg_flag %08Xh\n",
				t_prd_dbg_flag);
	size += lxs_snprintf(buf, size,
				"t_prd_dbg_mask %08Xh\n",
				t_prd_dbg_mask);

	size += lxs_snprintf(buf, size,
				"\nUsage:\n");
	size += lxs_snprintf(buf, size,
				" prd->dbg_mask  : echo 0 {mask_value} > prd_dbg_mask\n");
	size += lxs_snprintf(buf, size,
				" t_prd_dbg_flag : echo 8 {mask_value} > prd_dbg_mask\n");
	size += lxs_snprintf(buf, size,
				" t_prd_dbg_mask : echo 9 {mask_value} > prd_dbg_mask\n");

	return (ssize_t)size;
}

static void prd_store_dbg_mask_usage(struct lxs_prd_data *prd)
{
	t_prd_info(prd, "%s\n", "Usage:");
	t_prd_info(prd, "%s\n", " prd->dbg_mask  : echo 0 {mask_value(hex)} > prd_dbg_mask");
	t_prd_info(prd, "%s\n", " t_prd_dbg_flag : echo 8 {mask_value(hex)} > prd_dbg_mask");
	t_prd_info(prd, "%s\n", " t_prd_dbg_mask : echo 9 {mask_value(hex)} > prd_dbg_mask");
}

static ssize_t prd_store_dbg_mask(struct device *dev,
				const char *buf, size_t count)
{
	struct lxs_prd_data *prd = __dev_get_prd(dev);
	int type = 0;
	u32 old_value, new_value = 0;

	if (sscanf(buf, "%d %X", &type, &new_value) <= 0) {
		t_prd_err(prd, "%s\n", "Invalid param");
		prd_store_dbg_mask_usage(prd);
		return count;
	}

	mutex_lock(&prd->lock);

	switch (type) {
	case 0:
		old_value = prd->dbg_mask;
		prd->dbg_mask = new_value;
		t_prd_info(prd, "prd->dbg_mask changed : %08Xh -> %08xh\n",
			old_value, new_value);
		break;
	case 8:
		old_value = t_prd_dbg_flag;
		t_prd_dbg_flag = new_value;
		t_prd_info(prd, "t_prd_dbg_flag changed : %08Xh -> %08xh\n",
			old_value, new_value);
		break;
	case 9:
		old_value = t_prd_dbg_mask;
		t_prd_dbg_mask = new_value;
		t_prd_info(prd, "t_prd_dbg_mask changed : %08Xh -> %08xh\n",
			old_value, new_value);
		break;
	default:
		prd_store_dbg_mask_usage(prd);
		break;
	}

	mutex_unlock(&prd->lock);

	return count;
}
static TS_ATTR_PRD(prd_dbg_mask, prd_show_dbg_mask, prd_store_dbg_mask);

static ssize_t prd_show_info(struct device *dev, char *buf)
{
	struct lxs_prd_data *prd = __dev_get_prd(dev);
//	struct lxs_ts *ts = (struct lxs_ts *)prd->ts;
	struct lxs_prd_raw_ctrl *raw_ctrl = prd->raw_ctrl;
	struct lxs_prd_sd_ctrl *sd_ctrl = prd->sd_ctrl;
	struct lxs_prd_sd_ctrl *lpwg_sd_ctrl = prd->lpwg_sd_ctrl;
	char scmp[32] = {0, };
	char strs[16] = {0, };
	char *grp, *cal;
	int pad_row, pad_col;
	int i;
	int size = 0;

	size += lxs_snprintf(buf, size, "row %d, col %d, ch %d\n",
				prd->row, prd->col, prd->ch);

	if (prd->num_raw_ctrl)
		size += lxs_snprintf(buf, size, "----\n");

	for (i = 0; i < prd->num_raw_ctrl; i++, raw_ctrl++) {
		grp = (raw_ctrl->flag & PRD_RAW_FLAG_TYPE_SYS) ? "sys" : "raw";
		cal = (raw_ctrl->data_fmt & PRD_DATA_FMT_RXTX) ? "+" : "x";

		if (raw_ctrl->is_pad) {
			pad_row = raw_ctrl->pad->row;
			pad_col = raw_ctrl->pad->col;
		} else {
			pad_row = 0;
			pad_col = 0;
		}

		size += lxs_snprintf(buf, size,
				"  %s[%2d] (%2d) %16s (%5d, 0x%08X, 0x%08X), %2d %s %2d (%4d)(%d,%d), off 0x%04X\n",
				grp, i, raw_ctrl->id, raw_ctrl->name, raw_ctrl->cmd,
				raw_ctrl->data_fmt, raw_ctrl->flag,
				raw_ctrl->row_size, cal, raw_ctrl->col_size, raw_ctrl->data_size,
				pad_row, pad_col,
				raw_ctrl->offset);
	}

	if (prd->num_sd_ctrl)
		size += lxs_snprintf(buf, size, "----\n");

	for (i = 0; i < prd->num_sd_ctrl; i++, sd_ctrl++) {
		cal = (sd_ctrl->data_fmt & PRD_DATA_FMT_RXTX) ? "+" : "x";

		if (sd_ctrl->odd_sd_func) {
			snprintf(strs, sizeof(strs) - 1, " custom");
		} else {
			snprintf(strs, sizeof(strs) - 1, "%2d %s %2d",
				sd_ctrl->row_size, cal, sd_ctrl->col_size);
		}

		if (sd_ctrl->is_pad) {
			pad_row = raw_ctrl->pad->row;
			pad_col = raw_ctrl->pad->col;
		} else {
			pad_row = 0;
			pad_col = 0;
		}

		if (sd_ctrl->flag & PRD_RAW_FLAG_CMP16) {
			int cmp1[2], cmp2[2];

			cmp1[0] = sd_ctrl->cmp_min & 0xFFFF;
			cmp1[1] = sd_ctrl->cmp_max & 0xFFFF;
			cmp2[0] = sd_ctrl->cmp_min >> 16;
			cmp2[1] = sd_ctrl->cmp_max >> 16;
			snprintf(scmp, sizeof(scmp) - 1, "%d/%d, %d/%d",
				cmp1[0], cmp1[1], cmp2[0], cmp2[1]);
		} else {
			snprintf(scmp, sizeof(scmp) - 1, "%d/%d",
				sd_ctrl->cmp_min, sd_ctrl->cmp_max);
		}

		size += lxs_snprintf(buf, size,
				"sd_np[%2d] (%2d) %16s (0x%03X, 0x%08X, 0x%08X), %s (%4d)(%d,%d), off 0x%04X, sd/os %d/%d, dly %d, cmp %s\n",
				i, sd_ctrl->id, sd_ctrl->name, sd_ctrl->cmd,
				sd_ctrl->data_fmt, sd_ctrl->flag,
				strs, sd_ctrl->data_size,
				pad_row, pad_col,
				sd_ctrl->offset, sd_ctrl->is_sd, sd_ctrl->is_os,
				sd_ctrl->delay, scmp);
	}

	if (prd->num_lpwg_sd_ctrl)
		size += lxs_snprintf(buf, size, "----\n");

	for (i = 0; i < prd->num_lpwg_sd_ctrl; i++, lpwg_sd_ctrl++) {
		cal = (lpwg_sd_ctrl->data_fmt & PRD_DATA_FMT_RXTX) ? "+" : "x";

		if (lpwg_sd_ctrl->odd_sd_func) {
			snprintf(strs, sizeof(strs) - 1, " custom");
		} else {
			snprintf(strs, sizeof(strs) - 1, "%2d %s %2d",
				lpwg_sd_ctrl->row_size, cal, lpwg_sd_ctrl->col_size);
		}

		if (lpwg_sd_ctrl->is_pad) {
			pad_row = raw_ctrl->pad->row;
			pad_col = raw_ctrl->pad->col;
		} else {
			pad_row = 0;
			pad_col = 0;
		}

		if (lpwg_sd_ctrl->flag & PRD_RAW_FLAG_CMP16) {
			int cmp1[2], cmp2[2];

			cmp1[0] = lpwg_sd_ctrl->cmp_min & 0xFFFF;
			cmp1[1] = lpwg_sd_ctrl->cmp_max & 0xFFFF;
			cmp2[0] = lpwg_sd_ctrl->cmp_min >> 16;
			cmp2[1] = lpwg_sd_ctrl->cmp_max >> 16;
			snprintf(scmp, sizeof(scmp) - 1, "%d/%d, %d/%d",
				cmp1[0], cmp1[1], cmp2[0], cmp2[1]);
		} else {
			snprintf(scmp, sizeof(scmp) - 1, "%d/%d",
				lpwg_sd_ctrl->cmp_min, lpwg_sd_ctrl->cmp_max);
		}

		size += lxs_snprintf(buf, size,
				"sd_lp[%2d] (%2d) %16s (0x%03X, 0x%08X, 0x%08X), %s (%4d)(%d,%d), off 0x%04X, sd/os %d/%d, dly %d, cmp %s\n",
				i, lpwg_sd_ctrl->id, lpwg_sd_ctrl->name, lpwg_sd_ctrl->cmd,
				lpwg_sd_ctrl->data_fmt, lpwg_sd_ctrl->flag,
				strs, lpwg_sd_ctrl->data_size,
				pad_row, pad_col,
				lpwg_sd_ctrl->offset, lpwg_sd_ctrl->is_sd, lpwg_sd_ctrl->is_os,
				lpwg_sd_ctrl->delay, scmp);
	}

	size += lxs_snprintf(buf, size, "\n");

	return (ssize_t)size;
}
static TS_ATTR_PRD(prd_info, prd_show_info, NULL);

static ssize_t prd_show_swap(struct device *dev, char *buf)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);
	struct lxs_prd_data *prd = (struct lxs_prd_data *)ts->prd;
	struct lxs_prd_dim_ctl *dim_ctl = NULL;
	int size = 0;

	dim_ctl = &prd->raw_dim_ctl;
	size += lxs_snprintf(buf, size, " raw: t %d, rr %d, rt %d, ro %d, s %d, v %d, h %d, fmt 0x%X\n",
				dim_ctl->type, dim_ctl->rev_rx, dim_ctl->rev_tx, dim_ctl->rev_ord,
				dim_ctl->swap, dim_ctl->flip_v, dim_ctl->flip_h, dim_ctl->data_fmt);

	dim_ctl = &prd->sys_dim_ctl;
	size += lxs_snprintf(buf, size, " sys: t %d, rr %d, rt %d, ro %d, s %d, v %d, h %d, fmt 0x%X\n",
				dim_ctl->type, dim_ctl->rev_rx, dim_ctl->rev_tx, dim_ctl->rev_ord,
				dim_ctl->swap, dim_ctl->flip_v, dim_ctl->flip_h, dim_ctl->data_fmt);

	dim_ctl = &prd->sd_dim_ctl;
	size += lxs_snprintf(buf, size, "  sd: t %d, rr %d, rt %d, ro %d, s %d, v %d, h %d, fmt 0x%X\n",
				dim_ctl->type, dim_ctl->rev_rx, dim_ctl->rev_tx, dim_ctl->rev_ord,
				dim_ctl->swap, dim_ctl->flip_v, dim_ctl->flip_h, dim_ctl->data_fmt);

	size += lxs_snprintf(buf, size, "flag: raw %d, sys %d, sd %d, app %d\n",
				prd->enable_swap_raw, prd->enable_swap_sys, prd->enable_swap_sd, prd->enable_swap_app);

	return (ssize_t)size;
}

static ssize_t prd_store_swap(struct device *dev,
				const char *buf, size_t count)
{
	struct lxs_prd_data *prd = __dev_get_prd(dev);
	int type = 0;
	u32 old_value, new_value = 0;

	if (sscanf(buf, "%d %d", &type, &new_value) <= 0) {
		t_prd_err(prd, "%s\n", "Invalid param");
		return count;
	}

	if (!prd->panel_type)
		return count;

	mutex_lock(&prd->lock);

	switch (type) {
	case 0:
		old_value = prd->enable_swap_raw;
		prd->enable_swap_raw = new_value;
		t_prd_info(prd, "prd->enable_swap_raw changed : %d -> %d\n",
			old_value, new_value);
		break;
	case 1:
		old_value = prd->enable_swap_sys;
		prd->enable_swap_sys = new_value;
		t_prd_info(prd, "prd->enable_swap_sys changed : %d -> %d\n",
			old_value, new_value);
		break;
	case 2:
		old_value = prd->enable_swap_sd;
		prd->enable_swap_sd = new_value;
		t_prd_info(prd, "prd->enable_swap_sd changed : %d -> %d\n",
			old_value, new_value);
		break;
	case 11:
		old_value = prd->enable_swap_app;
		prd->enable_swap_app = new_value;
		t_prd_info(prd, "prd->enable_swap_app changed : %d -> %d\n",
			old_value, new_value);
		break;
	case 21:
		prd_upd_swap_info(prd->ts);
		break;
	}

	mutex_unlock(&prd->lock);

	return count;
}
static TS_ATTR_PRD(prd_swap, prd_show_swap, prd_store_swap);

static struct attribute *lxs_hal_prd_attribute_list[] = {
#if defined(__PRD_SUPPORT_SD_TEST)
	&ts_attr_sd_np.attr,
	&ts_attr_sd_lp.attr,
	/* */
	&ts_attr_sd_np_rst.attr,
	&ts_attr_sd_np_irq.attr,
	&ts_attr_sd_np_sync.attr,
	&ts_attr_sd_np_calb.attr,
	/* */
	&ts_attr_sd_np_a_raw.attr,
	&ts_attr_sd_np_a_raw_self.attr,
	&ts_attr_sd_np_a_jitter.attr,
	&ts_attr_sd_np_a_jitter_self.attr,
	&ts_attr_sd_np_m_raw.attr,
	&ts_attr_sd_np_m_raw_self.attr,
	//&ts_attr_sd_np_m_jitter.attr,
	//&ts_attr_sd_np_m_jitter_self.attr,
	&ts_attr_sd_np_m_jitter_min.attr,
	&ts_attr_sd_np_m_jitter_max.attr,
	&ts_attr_sd_np_m_jitter_mmd.attr,
	&ts_attr_sd_np_m_jitter_self_min.attr,
	&ts_attr_sd_np_m_jitter_self_max.attr,
	&ts_attr_sd_np_m_jitter_self_mmd.attr,
	/* */
	&ts_attr_sd_lp_a_open.attr,
	&ts_attr_sd_lp_a_gap_x.attr,
	&ts_attr_sd_lp_a_gap_y.attr,
	&ts_attr_sd_lp_a_gap_a.attr,
	&ts_attr_sd_lp_a_short_gnd.attr,
	&ts_attr_sd_lp_a_short_trx.attr,
	&ts_attr_sd_lp_a_short_vdd.attr,
	&ts_attr_sd_lp_a_raw.attr,
	&ts_attr_sd_lp_a_raw_self.attr,
	&ts_attr_sd_lp_a_jitter.attr,
	&ts_attr_sd_lp_a_jitter_self.attr,
	&ts_attr_sd_lp_c_open.attr,
	&ts_attr_sd_lp_c_short.attr,
	&ts_attr_sd_lp_m_open.attr,
	&ts_attr_sd_lp_m_gap_x.attr,
	&ts_attr_sd_lp_m_gap_y.attr,
	&ts_attr_sd_lp_m_gap_a.attr,
	&ts_attr_sd_lp_m_short_gnd.attr,
	&ts_attr_sd_lp_m_short_trx.attr,
	&ts_attr_sd_lp_m_short_vdd.attr,
	&ts_attr_sd_lp_m_raw.attr,
	&ts_attr_sd_lp_m_raw_self.attr,
	//&ts_attr_sd_lp_m_jitter.attr,
	//&ts_attr_sd_lp_m_jitter_self.attr,
	&ts_attr_sd_lp_m_jitter_min.attr,
	&ts_attr_sd_lp_m_jitter_max.attr,
	&ts_attr_sd_lp_m_jitter_mmd.attr,
	&ts_attr_sd_lp_m_jitter_self_min.attr,
	&ts_attr_sd_lp_m_jitter_self_max.attr,
	&ts_attr_sd_lp_m_jitter_self_mmd.attr,
#endif
	/* */
#if !defined(__PRD_USE_BIG_ATTR)
	&ts_attr_rawdata.attr,
	&ts_attr_delta.attr,
	&ts_attr_label.attr,
	&ts_attr_base.attr,
#endif
	/* */
	&ts_attr_prd_app_raw.attr,
	&ts_attr_prd_app_base.attr,
	&ts_attr_prd_app_delta.attr,
	&ts_attr_prd_app_label.attr,
	&ts_attr_prd_app_debug_buf.attr,
	&ts_attr_prd_app_end.attr,
	&ts_attr_prd_app_info.attr,
	&ts_attr_prd_app_reg.attr,
	/* */
	&ts_attr_prd_dbg_mask.attr,
	&ts_attr_prd_info.attr,
	&ts_attr_prd_swap.attr,
	/* */
	NULL,
};

static struct bin_attribute *lxs_hal_prd_attribute_bin_list[] = {
#if defined(__PRD_USE_BIG_ATTR)
	&ts_bin_attr_rawdata,
	&ts_bin_attr_base,
	&ts_bin_attr_delta,
	&ts_bin_attr_label,
#endif
	&ts_bin_attr_debug_buf,
	/* */
	NULL,	/* end mask */
};

static struct attribute_group lxs_hal_prd_attribute_group = {
	.attrs = lxs_hal_prd_attribute_list,
	.bin_attrs = lxs_hal_prd_attribute_bin_list,
};

static int lxs_hal_prd_create_group(struct lxs_ts *ts)
{
	struct lxs_prd_data *prd = ts->prd;
	int ret = 0;

	ret = sysfs_create_group(&ts->kobj, &lxs_hal_prd_attribute_group);
	if (ret < 0) {
		t_prd_err(prd, "sysfs_create_group failed, %d\n", ret);
		goto out;
	}

out:
	return ret;
}

static void lxs_hal_prd_remove_group(struct lxs_ts *ts)
{
	sysfs_remove_group(&ts->kobj, &lxs_hal_prd_attribute_group);
}

#define __ENABLE_RAW_CTRL

#if defined(__PRD_SUPPORT_SD_TEST)
#define __ENABLE_SD_CTRL
#define __ENABLE_LPWG_SD_CTRL
#endif

#if defined(CONFIG_TOUCHSCREEN_LXS_SW82907)
/*
 * Chip Setup : SW82907
 */
#define __ROW				16
#define __COL				37

#define __FMT_RAW			(PRD_DATA_FMT_U16)
#define __FMT_DELTA			(PRD_DATA_FMT_S16)

#define __FMT_RAW_SELF		(PRD_DATA_FMT_U16 | PRD_DATA_FMT_SELF)
#define __FMT_DELTA_SELF	(PRD_DATA_FMT_S16 | PRD_DATA_FMT_SELF)

#define __FMT_RAW_RXTX		(PRD_DATA_FMT_U16 | PRD_DATA_FMT_RXTX)
#define __FMT_DELTA_RXTX	(PRD_DATA_FMT_S16 | PRD_DATA_FMT_RXTX)

#define __FMT_RAW_DATA		(PRD_DATA_FMT_U16 | PRD_DATA_FMT_DATA)
#define __FMT_DELTA_DATA	(PRD_DATA_FMT_S16 | PRD_DATA_FMT_DATA)

#define __FLAG_SYS			(PRD_RAW_FLAG_FW_NO_STS | PRD_RAW_FLAG_TYPE_SYS)

#define __FLAG_SD_DEF		(PRD_RAW_FLAG_TBL_OFFSET | PRD_RAW_FLAG_RESP_LSB)

#define __FLAG_LPWG_SD_DEF		(PRD_RAW_FLAG_TBL_OFFSET | PRD_RAW_FLAG_RESP_LSB)
#define __FLAG_LPWG_SD_OS_1ST	(__FLAG_LPWG_SD_DEF)
#define __FLAG_LPWG_SD_OS_2ND	(__FLAG_LPWG_SD_DEF | PRD_RAW_FLAG_FW_SKIP)

#if defined(__ENABLE_RAW_CTRL)
static const struct lxs_prd_raw_ctrl __used raw_ctrl_sw82907[] = {
	/* 37 x 16 */
	RAW_CTRL_INIT(PRD_CTRL_IDX_RAW, "raw", PRD_CMD_RAW,
		__FMT_RAW_SELF, 0,
		__ROW, __COL, 0x1786, prd_get_raw_data, NULL, prd_prt_rawdata),
	RAW_CTRL_INIT(PRD_CTRL_IDX_BASE, "base", PRD_CMD_BASE,
		__FMT_RAW_SELF, 0,
		__ROW, __COL, 0x1786, prd_get_raw_data, NULL, prd_prt_rawdata),
	RAW_CTRL_INIT(PRD_CTRL_IDX_DELTA, "delta", PRD_CMD_DELTA,
		__FMT_DELTA_SELF, 0,
		__ROW, __COL, 0x1786, prd_get_raw_data, NULL, prd_prt_rawdata),
	RAW_CTRL_INIT(PRD_CTRL_IDX_LABEL, "label", PRD_CMD_LABEL,
		__FMT_DELTA_SELF, 0,
		__ROW, __COL, 0x1786, prd_get_raw_data, NULL, prd_prt_rawdata),
	RAW_CTRL_INIT(PRD_CTRL_IDX_DEBUG, "debug", PRD_CMD_DEBUG,
		__FMT_DELTA_SELF, 0,
		__ROW, __COL, 0x1786, prd_get_raw_data, NULL, prd_prt_rawdata),
};
#define TBL_RAW_CTRL	raw_ctrl_sw82907
#define NUM_RAW_CTRL	ARRAY_SIZE(raw_ctrl_sw82907)
#else
#define TBL_RAW_CTRL	NULL
#define NUM_RAW_CTRL	0
#endif

#if defined(__ENABLE_SD_CTRL)
static const struct lxs_prd_sd_ctrl __used sd_ctrl_sw82907[] = {
	/* */
	SD_CTRL_INIT(PRD_CTRL_IDX_SD_U3_M2_RAW, "NP_RAW", 0x305,
		__FMT_RAW, __FLAG_SD_DEF,
		__ROW, __COL, 0xEE6, prd_get_sd_raw, NULL, prd_prt_sd_raw, prd_chk_sd_raw, 0),
	SD_CTRL_INIT(PRD_CTRL_IDX_SD_U3_M2_RAW_SELF, "NP_RAW_SELF", 0x30F,
		__FMT_RAW_RXTX, __FLAG_SD_DEF,
		__ROW, __COL, 0xEE6, prd_get_sd_raw, NULL, prd_prt_sd_rxtx, prd_chk_sd_rxtx, 0),
	SD_CTRL_INIT(PRD_CTRL_IDX_SD_U3_JITTER, "NP_JITTER", 0x311,
		__FMT_RAW, __FLAG_SD_DEF,
		__ROW, __COL, 0xEE6, prd_get_sd_raw, NULL, prd_prt_sd_raw, prd_chk_sd_raw, 800),
	SD_CTRL_INIT(PRD_CTRL_IDX_SD_U3_JITTER_SELF, "NP_JITTER_SELF", 0x312,
		__FMT_RAW_RXTX, __FLAG_SD_DEF,
		__ROW, __COL, 0xEE6, prd_get_sd_raw, NULL, prd_prt_sd_rxtx, prd_chk_sd_rxtx, 800),
	/* */
	SD_CTRL_OD_INIT(PRD_CTRL_IDX_SD_CALB, "CALB", 0,
		__FMT_RAW, __FLAG_SD_DEF | PRD_RAW_FLAG_FW_SKIP,
		__ROW, __COL, 0, prd_sd_calb_test),
	SD_CTRL_OD_INIT(PRD_CTRL_IDX_SD_SYNC, "SYNC", 0,
		__FMT_RAW, __FLAG_SD_DEF | PRD_RAW_FLAG_FW_SKIP | PRD_RAW_FLAG_CMP16,
		__ROW, __COL, 0, prd_sd_sync_test),
	SD_CTRL_OD_INIT(PRD_CTRL_IDX_SD_IRQ, "IRQ", 0,
		__FMT_RAW, __FLAG_SD_DEF | PRD_RAW_FLAG_FW_SKIP,
		__ROW, __COL, 0, prd_sd_irq_test),
	SD_CTRL_OD_INIT(PRD_CTRL_IDX_SD_RST, "RST", 0,
		__FMT_RAW, __FLAG_SD_DEF | PRD_RAW_FLAG_FW_SKIP,
		__ROW, __COL, 0, prd_sd_rst_test),
};
#define TBL_SD_CTRL	sd_ctrl_sw82907
#define NUM_SD_CTRL	ARRAY_SIZE(sd_ctrl_sw82907)
#else
#define TBL_SD_CTRL	NULL
#define NUM_SD_CTRL	0
#endif

#if defined(__ENABLE_LPWG_SD_CTRL)
static const struct lxs_prd_sd_ctrl __used lpwg_sd_ctrl_sw82907[] = {
	/* OS */
	LPWG_SD_OS_INIT(PRD_CTRL_IDX_LPWG_SD_OPEN, "LP_OPEN", 0x001,
		__FMT_RAW, __FLAG_LPWG_SD_OS_1ST,
		__ROW, __COL, 0xEE6, prd_get_sd_os, NULL, prd_prt_sd_raw, prd_chk_sd_raw, 100),
	LPWG_SD_OS_INIT(PRD_CTRL_IDX_LPWG_SD_GAP_X, "LP_GAP_X", 0x001,
		__FMT_DELTA, __FLAG_LPWG_SD_OS_2ND,
		__ROW, __COL, 0xEE6, prd_get_sd_os, NULL, prd_prt_sd_gap_x, prd_chk_sd_raw, 100),
	LPWG_SD_OS_INIT(PRD_CTRL_IDX_LPWG_SD_GAP_Y, "LP_GAP_Y", 0x001,
		__FMT_DELTA, __FLAG_LPWG_SD_OS_2ND,
		__ROW, __COL, 0xEE6, prd_get_sd_os, NULL, prd_prt_sd_gap_y, prd_chk_sd_raw, 100),
	LPWG_SD_OS_INIT(PRD_CTRL_IDX_LPWG_SD_GAP_A, "LP_GAP_A", 0x001,
		__FMT_DELTA, __FLAG_LPWG_SD_OS_2ND,
		__ROW, __COL, 0xEE6, prd_get_sd_os, NULL, prd_prt_sd_gap_a, prd_chk_sd_raw, 100),
	LPWG_SD_OS_INIT(PRD_CTRL_IDX_LPWG_SD_SHORT_GND, "LP_SHORT_GND", 0x002,
		__FMT_RAW_RXTX, __FLAG_LPWG_SD_OS_1ST,
		__ROW, __COL, 	0xEE6, prd_get_sd_os, NULL, prd_prt_sd_rxtx, prd_chk_sd_rxtx, 100),
	LPWG_SD_OS_INIT(PRD_CTRL_IDX_LPWG_SD_SHORT_TRX, "LP_SHORT_TRX", 0x002,
		__FMT_RAW_RXTX, __FLAG_LPWG_SD_OS_2ND,
		__ROW, __COL, 0xEE6 + 0x025, prd_get_sd_os, NULL, prd_prt_sd_rxtx, prd_chk_sd_rxtx, 100),
	LPWG_SD_OS_INIT(PRD_CTRL_IDX_LPWG_SD_SHORT_VDD, "LP_SHORT_VDD", 0x002,
		__FMT_RAW_RXTX, __FLAG_LPWG_SD_OS_2ND,
		__ROW, __COL, 0xEE6 + 0x04A, prd_get_sd_os, NULL, prd_prt_sd_rxtx, prd_chk_sd_rxtx, 100),
	/* */
	LPWG_SD_CTRL_INIT(PRD_CTRL_IDX_LPWG_SD_U0_M2_RAW, "LP_RAW", 0x005,
		__FMT_RAW, __FLAG_LPWG_SD_DEF,
		__ROW, __COL, 0xEE6, prd_get_sd_raw, NULL, prd_prt_sd_raw, prd_chk_sd_raw, 0),
	LPWG_SD_CTRL_INIT(PRD_CTRL_IDX_LPWG_SD_U0_M2_RAW_SELF, "LP_RAW_SELF", 0x00F,
		__FMT_RAW_RXTX, __FLAG_SD_DEF,
		__ROW, __COL, 0xEE6, prd_get_sd_raw, NULL, prd_prt_sd_rxtx, prd_chk_sd_rxtx, 0),
	LPWG_SD_CTRL_INIT(PRD_CTRL_IDX_LPWG_SD_U0_JITTER, "LP_JITTER", 0x011,
		__FMT_RAW, __FLAG_LPWG_SD_DEF,
		__ROW, __COL, 0xEE6, prd_get_sd_raw, NULL, prd_prt_sd_raw, prd_chk_sd_raw, 800),
	LPWG_SD_CTRL_INIT(PRD_CTRL_IDX_LPWG_SD_U0_JITTER_SELF, "LP_JITTER_SELF", 0x012,
		__FMT_RAW_RXTX, __FLAG_SD_DEF,
		__ROW, __COL, 0xEE6, prd_get_sd_raw, NULL, prd_prt_sd_rxtx, prd_chk_sd_rxtx, 800),
};
#define TBL_LPWG_SD_CTRL	lpwg_sd_ctrl_sw82907
#define NUM_LPWG_SD_CTRL	ARRAY_SIZE(lpwg_sd_ctrl_sw82907)
#else
#define TBL_LPWG_SD_CTRL	NULL
#define NUM_LPWG_SD_CTRL	0
#endif

#else	/* !CONFIG_TOUCHSCREEN_LXS_XXXXX */

#define __ROW	0
#define __COL	0

#define TBL_RAW_CTRL	NULL
#define NUM_RAW_CTRL	0

#define TBL_SD_CTRL	NULL
#define NUM_SD_CTRL	0

#define TBL_LPWG_SD_CTRL	NULL
#define NUM_LPWG_SD_CTRL	0

#endif	/* CONFIG_TOUCHSCREEN_LXS_XXXXX */

static void __used prd_check_raw_ctrl(struct lxs_prd_data *prd, struct lxs_prd_raw_ctrl *ctrl, int num)
{
	struct lxs_prd_raw_pad *pad;
	u32 data_fmt;
	u32 flag;
	int row_size;
	int col_size;
	int i;

	for (i = 0; i < num; i++, ctrl++) {
		pad = ctrl->pad;
		data_fmt = ctrl->data_fmt;
		flag = ctrl->flag;

		prd->raw_lookup_table[ctrl->id] = i;

		if (flag & PRD_RAW_FLAG_TBL_RXTX) {
			if (data_fmt & PRD_DATA_FMT_SYS_PDATA) {
				if (prd->sys_pdata_rx && prd->sys_pdata_tx) {
					ctrl->row_size = prd->sys_pdata_tx;
					ctrl->col_size = prd->sys_pdata_rx;
				}
			}

			if (data_fmt & PRD_DATA_FMT_SYS_PDBG) {
				if (prd->sys_pdebug_rx && prd->sys_pdebug_tx) {
					ctrl->row_size = prd->sys_pdebug_tx;
					ctrl->col_size = prd->sys_pdebug_rx;
				}
			}
		} else {
			if (prd->row_adj && prd->col_adj) {
				ctrl->row_size = prd->row_adj;
				ctrl->col_size = prd->col_adj;

				if (data_fmt & PRD_DATA_FMT_SYS_ROW)
					ctrl->row_size++;

				if (data_fmt & PRD_DATA_FMT_SYS_COL)
					ctrl->col_size++;
			}
		}

		prd_mod_raw_scan(prd, ctrl);
		if (!ctrl->is_dim)
			ctrl->data_fmt &= ~PRD_DATA_FMT_DIM;
		data_fmt = ctrl->data_fmt;

		row_size = ctrl->row_size;
		col_size = ctrl->col_size;

		ctrl->data_size = __cal_data_size(row_size, col_size, data_fmt);

		if (!(flag & PRD_RAW_FLAG_TBL_OFFSET)) {
			if (flag & PRD_RAW_FLAG_TYPE_SYS) {
				if (prd->sys_base)
					ctrl->offset = prd->sys_base;
			} else {
				if (prd->raw_base)
					ctrl->offset = prd->raw_base;
			}
		}

		if (pad)
			ctrl->is_pad = !!(data_fmt & PRD_DATA_FMT_PAD) & !!(pad->row || pad->col);
	}
}

static void __used prd_check_sd_ctrl(struct lxs_prd_data *prd, struct lxs_prd_sd_ctrl *sd_ctrl, int num, int is_sd)
{
	struct lxs_prd_raw_pad *pad;
	int *lookup_table = (is_sd) ? prd->sd_lookup_table : prd->lpwg_sd_lookup_table;
	u32 data_fmt;
	u32 flag;
	int row_size;
	int col_size;
	int i;

	for (i = 0; i < num; i++, sd_ctrl++) {
		pad = sd_ctrl->pad;
		data_fmt = sd_ctrl->data_fmt;
		flag = sd_ctrl->flag;

		lookup_table[sd_ctrl->id] = i;

		if (flag & PRD_RAW_FLAG_TBL_RXTX) {
			/* TBD */
		} else {
			if (prd->row_adj && prd->col_adj) {
				sd_ctrl->row_size = prd->row_adj;
				sd_ctrl->col_size = prd->col_adj;

				if (data_fmt & PRD_DATA_FMT_SYS_ROW)
					sd_ctrl->row_size++;

				if (data_fmt & PRD_DATA_FMT_SYS_COL)
					sd_ctrl->col_size++;
			}
		}

	#if defined(__PRD_SUPPORT_SD_TEST)
		prd_mod_sd_scan(prd, sd_ctrl);
	#endif
		if (!sd_ctrl->is_dim)
			sd_ctrl->data_fmt &= ~PRD_DATA_FMT_DIM;
		data_fmt = sd_ctrl->data_fmt;

		row_size = sd_ctrl->row_size;
		col_size = sd_ctrl->col_size;

		sd_ctrl->data_size = __cal_data_size(row_size, col_size, data_fmt);;

		if (!(flag & PRD_RAW_FLAG_TBL_OFFSET)) {
			/* TBD */
		}

		if (pad)
			sd_ctrl->is_pad = !!(data_fmt & PRD_DATA_FMT_PAD) & !!(pad->row || pad->col);
	}
}

static void prd_free_param(struct lxs_ts *ts)
{
	struct lxs_prd_data *prd = ts->prd;

	if (prd->raw_ctrl) {
		kfree(prd->raw_ctrl);
		prd->raw_ctrl = NULL;
	}

	if (prd->sd_ctrl) {
		kfree(prd->sd_ctrl);
		prd->sd_ctrl = NULL;
	}

	if (prd->lpwg_sd_ctrl) {
		kfree(prd->lpwg_sd_ctrl);
		prd->lpwg_sd_ctrl = NULL;
	}
}

#if defined(__LXS_SUPPORT_RINFO)
static void prd_init_rinfo(struct lxs_ts *ts)
{
	struct lxs_ts_chip *chip = &ts->chip;
	struct lxs_prd_data *prd = (struct lxs_prd_data *)ts->prd;
	u32 temp = 0;
	int ret = 0;

	temp = chip->rinfo_data[3];
	prd->raw_base = temp >> 16; //0x5970

	temp = chip->rinfo_data[4];
	prd->sys_base = temp >> 16; //0x1AE8

	temp = chip->rinfo_data[6];
	prd->sys_cmd = temp >> 16;	//0x066B

	temp = chip->rinfo_data[9];
	prd->sys_dbg_addr = temp & 0xFFFF;	//0x06A4

	if (prd->sys_dbg_addr) {
		ret = lxs_hal_read_value(ts, prd->sys_dbg_addr, &temp);
		if (ret >= 0) {
			prd->sys_pdata_rx = temp & 0xFF;
			prd->sys_pdata_tx = (temp>>8) & 0xFF;
			prd->sys_pdebug_rx = (temp>>16) & 0xFF;
			prd->sys_pdebug_tx = (temp>>24) & 0xFF;
		}
	}

	t_prd_info(prd, "param: rinfo: raw_base 0x%X, sys_base 0x%X, sys_cmd 0x%X\n",
		prd->raw_base, prd->sys_base, prd->sys_cmd);
	t_prd_info(prd, "param: rinfo: dbg_addr 0x%X = 0x%08X\n", prd->sys_dbg_addr, temp);
	t_prd_info(prd, "param: rinfo: sys_pdata_rx %d, sys_pdata_tx %d\n", prd->sys_pdata_rx, prd->sys_pdata_tx);
	t_prd_info(prd, "param: rinfo: sys_pdebug_rx %d, sys_pdebug_tx %d\n", prd->sys_pdebug_rx, prd->sys_pdebug_tx);
}
#endif

static void prd_chk_swap_info(struct lxs_prd_data *prd,
			struct lxs_prd_dim_ctl *dim_ctl, char *name)
{
	struct lxs_prd_dim_ctl *dim_tbl = (struct lxs_prd_dim_ctl *)prd_dim_table;

	dim_ctl->swap = 0;
	dim_ctl->flip_v = 0;
	dim_ctl->flip_h = 0;
	dim_ctl->data_fmt = 0;

	while (1) {
		if (dim_tbl->type == -1)
			break;

		if (dim_ctl->type == dim_tbl->type) {
			dim_ctl->swap = dim_tbl->swap;
			dim_ctl->flip_v = dim_tbl->flip_v;
			dim_ctl->flip_h = dim_tbl->flip_h;
			dim_ctl->data_fmt = dim_tbl->data_fmt;
			return;
		}

		dim_tbl++;
	}

	if (dim_ctl->type)
		t_prd_info(prd, "%s: %s: invalid type %d\n",
			__func__, name, dim_ctl->type);
}

static int prd_read_swap_info(struct lxs_ts *ts)
{
	struct lxs_prd_data *prd = (struct lxs_prd_data *)ts->prd;
	struct lxs_ts_chip *chip = &ts->chip;
	struct lxs_hal_reg *reg = chip->reg;
	struct lxs_prd_dim_ctl *dim_ctl;
	struct lxs_prd_dim_reg *dim_reg;
	u32 dim_eng = reg->dim_eng;
	u32 dim_sys = reg->dim_sys;
	int ret = 0;

	if (lxs_addr_is_invalid(dim_eng) || lxs_addr_is_invalid(dim_sys)) {
		t_prd_info(prd, "%s: no dim_eng or no dim_sys\n", __func__);
		return 0;
	}

	dim_ctl = &prd->raw_dim_ctl;
	dim_reg = &prd->raw_dim_reg;
	memset(dim_reg, 0, sizeof(struct lxs_prd_dim_reg));
	ret = lxs_hal_read_value(ts, dim_eng, (u32 *)dim_reg);
	if (ret < 0) {
		t_prd_err(prd, "%s: failed to read dim_eng, %d\n", __func__, ret);
		return ret;
	}

	dim_ctl->type = dim_reg->type;
	dim_ctl->rev_rx = dim_reg->rev_rx;
	dim_ctl->rev_tx = dim_reg->rev_tx;
	dim_ctl->rev_ord = dim_reg->rev_ord;

	dim_ctl = &prd->sys_dim_ctl;
	dim_reg = &prd->sys_dim_reg;
	memset(dim_reg, 0, sizeof(struct lxs_prd_dim_reg));
	ret = lxs_hal_read_value(ts, dim_sys, (u32 *)dim_reg);
	if (ret < 0) {
		t_prd_err(prd, "%s: failed to read dim_sys, %d\n", __func__, ret);
		return ret;
	}

	dim_ctl->type = dim_reg->type;
	dim_ctl->rev_rx = dim_reg->rev_rx;
	dim_ctl->rev_tx = dim_reg->rev_tx;
	dim_ctl->rev_ord = dim_reg->rev_ord;

	return 0;
}

static int prd_init_swap_info(struct lxs_ts *ts)
{
	struct lxs_prd_data *prd = (struct lxs_prd_data *)ts->prd;
	struct lxs_prd_dim_ctl *dim_ctl = NULL;
	int ret = 0;

	if (!prd->panel_type)
		return 0;

	prd->enable_swap_app = 1;

	memset(&prd->raw_dim_ctl, 0, sizeof(struct lxs_prd_dim_ctl));
	memset(&prd->sys_dim_ctl, 0, sizeof(struct lxs_prd_dim_ctl));
	memset(&prd->sd_dim_ctl, 0, sizeof(struct lxs_prd_dim_ctl));

	prd->raw_dim_ctl.type = 1;
	prd->sys_dim_ctl.type = 1;

	ret = prd_read_swap_info(ts);
	if (ret < 0)
		return ret;

	prd_chk_swap_info(prd, &prd->raw_dim_ctl, "raw");
	prd_chk_swap_info(prd, &prd->sys_dim_ctl, "sys");
	memcpy(&prd->sd_dim_ctl, &prd->sys_dim_ctl, sizeof(struct lxs_prd_dim_ctl));

	dim_ctl = &prd->raw_dim_ctl;
	t_prd_info(prd, " raw: t %d, rr %d, rt %d, ro %d, s %d, v %d, h %d, fmt 0x%X\n",
		dim_ctl->type, dim_ctl->rev_rx, dim_ctl->rev_tx, dim_ctl->rev_ord,
		dim_ctl->swap, dim_ctl->flip_v, dim_ctl->flip_h, dim_ctl->data_fmt);

	dim_ctl = &prd->sys_dim_ctl;
	t_prd_info(prd, " sys: t %d, rr %d, rt %d, ro %d, s %d, v %d, h %d, fmt 0x%X\n",
		dim_ctl->type, dim_ctl->rev_rx, dim_ctl->rev_tx, dim_ctl->rev_ord,
		dim_ctl->swap, dim_ctl->flip_v, dim_ctl->flip_h, dim_ctl->data_fmt);

	dim_ctl = &prd->sd_dim_ctl;
	t_prd_info(prd, "  sd: t %d, rr %d, rt %d, ro %d, s %d, v %d, h %d, fmt 0x%X\n",
		dim_ctl->type , dim_ctl->rev_rx, dim_ctl->rev_tx, dim_ctl->rev_ord,
		dim_ctl->swap, dim_ctl->flip_v, dim_ctl->flip_h, dim_ctl->data_fmt);

	t_prd_info(prd, "flag: raw %d, sys %d, sd %d, app %d\n",
		prd->enable_swap_raw, prd->enable_swap_sys, prd->enable_swap_sd, prd->enable_swap_app);

	return 0;
}

static int prd_upd_swap_info(struct lxs_ts *ts)
{
	struct lxs_prd_data *prd = (struct lxs_prd_data *)ts->prd;
	struct lxs_ts_chip *chip = &ts->chip;
	struct lxs_hal_reg *reg = chip->reg;
	int ret = 0;

	if (!prd->panel_type)
		return 0;

	if (!prd->sysfs_done)
		return 0;

	if (lxs_addr_is_invalid(reg->dim_eng) || lxs_addr_is_invalid(reg->dim_sys)) {
		t_prd_dbg_base(prd, "%s: no dim_eng or no dim_sys\n", __func__);
		return 0;
	}

	if (atomic_read(&chip->init) != TC_IC_INIT_DONE) {
		t_prd_err(prd, "%s: not ready, need IC init\n", __func__);
		return 0;
	}

	if (atomic_read(&chip->boot) == TC_IC_BOOT_FAIL) {
		t_prd_err(prd, "%s: boot failed\n", __func__);
		return 0;
	}

	t_prd_info(prd, "%s\n", "update swap info");

	ret = prd_init_swap_info(ts);
	if (ret < 0)
		return ret;

	if (prd->raw_ctrl) {
		memcpy(prd->raw_ctrl, prd->tbl_raw,
			sizeof(struct lxs_prd_raw_ctrl) * prd->num_raw_ctrl);
		prd_check_raw_ctrl(prd, prd->raw_ctrl, prd->num_raw_ctrl);
	}

	if (prd->sd_ctrl) {
		memcpy(prd->sd_ctrl, prd->tbl_sd,
			sizeof(struct lxs_prd_sd_ctrl) * prd->num_sd_ctrl);
		prd_check_sd_ctrl(prd, prd->sd_ctrl, prd->num_sd_ctrl, 1);
	}

	if (prd->lpwg_sd_ctrl) {
		memcpy(prd->lpwg_sd_ctrl, prd->tbl_lpwg_sd,
			sizeof(struct lxs_prd_sd_ctrl) * prd->num_lpwg_sd_ctrl);
		prd_check_sd_ctrl(prd, prd->lpwg_sd_ctrl, prd->num_lpwg_sd_ctrl, 0);
	}

#if defined(__PRD_SUPPORT_SD_TEST)
	prd_sd_cmp_setup(prd);
#endif

	return ret;
}

static int prd_init_param(struct lxs_ts *ts)
{
	struct lxs_ts_chip *chip = &ts->chip;
	struct lxs_hal_fw_info *fw = &chip->fw;
	struct lxs_prd_data *prd = (struct lxs_prd_data *)ts->prd;
	void *buf = NULL;
	int size, i;
	int ret = 0;

	if (t_prd_dbg_flag & PRD_DBG_FLAG_DISABLE)
		goto out;

	prd->tbl_raw = TBL_RAW_CTRL;
	prd->tbl_sd = TBL_SD_CTRL;
	prd->tbl_lpwg_sd = TBL_LPWG_SD_CTRL;

	prd->chip_type = ts->chip_type;

	prd->panel_type = chip->opt.t_oled;

	prd->self = !!prd->panel_type;
	if (chip->tx_count && chip->rx_count) {
		prd->row = chip->tx_count;
		prd->col = chip->rx_count;

		prd->row_adj = prd->row;
		prd->col_adj = prd->col;
	} else {
		prd->row = __ROW;
		prd->col = __COL;

		chip->tx_count = prd->row;
		chip->rx_count = prd->col;
	}
	prd->ch = prd->col;

	if (!prd->row || !prd->col)
		return -EINVAL;

	t_prd_info(prd, "param: row %d, col %d\n", prd->row, prd->col);

#if defined(__LXS_SUPPORT_RINFO)
	prd_init_rinfo(ts);
#endif

	ret = prd_init_swap_info(ts);
	if (ret < 0)
		goto out;

	for (i = 0 ; i < PRD_CTRL_IDX_MAX; i++)
		prd->raw_lookup_table[i] = -1;

	for (i = 0 ; i < PRD_CTRL_IDX_SD_MAX; i++)
		prd->sd_lookup_table[i] = -1;

	for (i = 0 ; i < PRD_CTRL_IDX_LPWG_SD_MAX; i++)
		prd->lpwg_sd_lookup_table[i] = -1;

	if (prd->tbl_raw) {
		prd->num_raw_ctrl = NUM_RAW_CTRL;
		size = sizeof(struct lxs_prd_raw_ctrl) * prd->num_raw_ctrl;
		buf = kzalloc(size, GFP_KERNEL);
		if (!buf) {
			t_prd_err(prd, "%s\n", "failed to allocate prd->raw_ctrl");
			goto out;
		}
		prd->raw_ctrl = buf;
		memcpy(prd->raw_ctrl, prd->tbl_raw, size);
		prd_check_raw_ctrl(prd, prd->raw_ctrl, prd->num_raw_ctrl);
	}

	if (prd->tbl_sd) {
		prd->num_sd_ctrl = NUM_SD_CTRL;
		size = sizeof(struct lxs_prd_sd_ctrl) * prd->num_sd_ctrl;
		buf = kzalloc(size, GFP_KERNEL);
		if (!buf) {
			t_prd_err(prd, "%s\n", "failed to allocate prd->sd_ctrl");
			goto out;
		}
		prd->sd_ctrl = buf;
		memcpy(prd->sd_ctrl, prd->tbl_sd, size);
		prd_check_sd_ctrl(prd, prd->sd_ctrl, prd->num_sd_ctrl, 1);
	}

	if (prd->tbl_lpwg_sd) {
		prd->num_lpwg_sd_ctrl = NUM_LPWG_SD_CTRL;
		size = sizeof(struct lxs_prd_sd_ctrl) * prd->num_lpwg_sd_ctrl;
		buf = kzalloc(size, GFP_KERNEL);
		if (!buf) {
			t_prd_err(prd, "%s\n", "failed to allocate prd->lpwg_sd_ctrl");
			goto out;
		}
		prd->lpwg_sd_ctrl = buf;
		memcpy(prd->lpwg_sd_ctrl, prd->tbl_lpwg_sd, size);
		prd_check_sd_ctrl(prd, prd->lpwg_sd_ctrl, prd->num_lpwg_sd_ctrl, 0);
	}

#if defined(__PRD_SUPPORT_SD_TEST)
	prd_sd_cmp_setup(prd);
#endif

	prd->sysfs_flag = ~0;

	if (!prd->num_raw_ctrl && !prd->num_sd_ctrl && !prd->num_lpwg_sd_ctrl)
		return -ENODEV;

#if defined(__PRD_TEST_FILE_OFF)
	t_prd_info(prd, "%s\n", "test: file_wr_off, file_rd_off");
	prd->file_wr_off = 1;
	prd->file_rd_off = 1;
#endif

	t_prd_info(prd, "%s[%s] param init done\n",
		ts->chip_name, fw->product_id);

	return 0;

out:
	prd_free_param(ts);

	t_prd_info(prd, "%s[%s] param init failed\n",
		ts->chip_name, fw->product_id);

	return -EFAULT;
}

static struct lxs_prd_data *prd_alloc(struct lxs_ts *ts)
{
	struct lxs_prd_data *prd;

	prd = kzalloc(sizeof(*prd), GFP_KERNEL);
	if (!prd) {
		t_dev_err(ts->dev, "failed to allocate memory for prd\n");
		return NULL;
	}

	t_dev_dbg_base(ts->dev, "create prd (0x%X)\n", (int)sizeof(*prd));

	prd->ts = ts;

	prd->dev = ts->dev;

	ts->prd = prd;

	mutex_init(&prd->lock);

	return prd;;
}

static void prd_free(struct lxs_ts *ts)
{
	struct lxs_prd_data *prd = (struct lxs_prd_data *)ts->prd;

	if (prd) {
		t_dev_dbg_base(ts->dev, "free prd\n");

		mutex_destroy(&prd->lock);

		ts->prd = NULL;
		kfree(prd);
	}
}

static int prd_create(struct lxs_ts *ts)
{
	struct lxs_prd_data *prd = (struct lxs_prd_data *)ts->prd;
	int ret = 0;

	if (prd) {
		if (ts->state_core == TS_CORE_UPGRADE) {
			mutex_lock(&prd->lock);
			prd_upd_swap_info(ts);
			mutex_unlock(&prd->lock);
		}
		return 0;
	}

	prd = prd_alloc(ts);
	if (!prd)
		return -ENOMEM;

	ret = prd_init_param(ts);
	/* Just skip sysfs generation */
	if (ret < 0)
		return 0;

	ret = lxs_hal_prd_create_group(ts);
	if (ret < 0) {
		t_dev_err(ts->dev, "%s prd sysfs register failed, %d\n",
			ts->chip_name, ret);
		goto out_sysfs;
	}

	t_dev_dbg_base(ts->dev, "%s prd sysfs registered\n", ts->chip_name);

	prd->sysfs_done = 1;

	return 0;

out_sysfs:
	prd_free_param(ts);

	prd_free(ts);

	return ret;
}

static void prd_remove(struct lxs_ts *ts)
{
	struct lxs_prd_data *prd = (struct lxs_prd_data *)ts->prd;

	if (prd == NULL) {
		t_dev_dbg_base(ts->dev, "prd sysfs not initialized\n");
		return;
	}

	if (!prd->sysfs_done) {
		t_dev_dbg_base(ts->dev, "prd sysfs group not initialized\n");
		goto skip_prd_normal_remove;
	}

	lxs_hal_prd_remove_group(ts);

	prd_free_param(ts);

skip_prd_normal_remove:
	prd_free(ts);

	t_dev_dbg_base(ts->dev, "%s prd sysfs unregistered\n", ts->chip_name);
}

int lxs_hal_prd_sysfs(struct lxs_ts *ts, int on_off)
{
	if (on_off)
		return prd_create(ts);

	prd_remove(ts);
	return 0;
}


