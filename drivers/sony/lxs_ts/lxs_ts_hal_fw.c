/*
 * SPDX-License-Identifier: GPL-2.0
 *
 * lxs_ts_hal_fw.c
 *
 * LXS touch fw upgrade
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "lxs_ts.h"

static int lxs_hal_condition_wait(struct lxs_ts *ts,
				u32 addr, u32 *value, u32 expect,
			    u32 mask, u32 delay, u32 retry, int cond)
{
	u32 addr1 = 0;
	u32 addr2 = 0;
	u32 data = 0;
	u32 data1 = 0;
	u32 data2 = 0;
	int dual = 0;
	int match = 0;
	int ret = 0;

	dual = !!(addr & BIT(31));

	addr1 = addr & 0xFFFF;
	addr2 = (dual) ? ((addr>>16) & 0x7FFF) : 0;

	do {
		lxs_ts_delay(delay);

		ret = lxs_hal_read_value(ts, addr1, &data1);
		if (dual)
			lxs_hal_read_value(ts, addr2, &data2);

		data = data1 | data2;
		if (ret >= 0) {
			match = (cond == NOT_COND) ?	\
				!!((data & mask) != expect) : !!((data & mask) == expect);

			if (match) {
				if (value)
					*value = data;

				t_dev_dbg_fwup(ts->dev,
					"wait done: addr[%04Xh] data[%08Xh], "
					"mask[%08Xh], expect[%s%08Xh]\n",
					addr, data, mask,
					(cond == NOT_COND) ? "not " : "",
					expect);
				return 0;
			}
		}
	} while (--retry);

	if (value)
		*value = data;

	t_dev_err(ts->dev,
		"wait fail: addr[%04Xh] data[%08Xh], "
		"mask[%08Xh], expect[%s%08Xh]\n",
		addr, data, mask,
		(cond == NOT_COND) ? "not " : "",
		expect);

	return -EPERM;
}

static u32 lxs_hal_fw_act_buf_size(struct lxs_ts *ts)
{
	int buf_size = (ts->buf_size - TS_BUF_MARGIN) & (~0x3FF);

	return buf_size;
}

static int lxs_hal_fw_rd_value(struct lxs_ts *ts, u32 addr, u32 *value, char *title)
{
	u32 data;
	int ret;

	if (title == NULL)
		title = "FW upgrade";

	t_dev_dbg_fwup_wr(ts->dev,
		"%s: read: addr[%04Xh]\n",
		title, addr);

	ret = lxs_hal_read_value(ts, addr, &data);
	if (ret < 0) {
		t_dev_err(ts->dev,
			"%s: read failed(%d) - addr[%04Xh]\n",
			title, ret, addr);
		return ret;
	}

	if (value)
		*value = data;

	return 0;
}

static int lxs_hal_fw_wr_value(struct lxs_ts *ts, u32 addr, u32 value)
{
	int ret;

	t_dev_dbg_fwup_wr(ts->dev,
		"%s: write: addr[%04Xh], value[%08Xh]\n",
		__func__, addr, value);

	ret = lxs_hal_write_value(ts, addr, value);
	if (ret < 0) {
		t_dev_err(ts->dev,
			"%s: write failed(%d) - addr[%04Xh], value[%08Xh]\n",
			__func__, ret, addr, value);
		return ret;
	}

	return 0;
}

static void __fw_wr_data_dbg(struct lxs_ts *ts, u8 *buf, int size)
{
	int prt_len = 0;
	int prt_idx = 0;
	int prd_sz = size;

	if (!(t_dev_dbg_mask & DBG_FWUP_DBG))
		return;

	while (size) {
		prt_len = min(size, 16);

		__t_dev_dbg_tag(ts->dev,
			" 0x%04X buf[%3d~%3d] %*ph\n",
			(u32)prd_sz, prt_idx, prt_idx + prt_len - 1,
			prt_len, &buf[prt_idx]);

		size -= prt_len;
		prt_idx += prt_len;
	}
}

static int lxs_hal_fw_wr_data(struct lxs_ts *ts,
				u32 addr, u8 *dn_buf, int dn_size)
{
	struct lxs_ts_chip *chip = &ts->chip;
	struct lxs_hal_reg *reg = chip->reg;
	u32 offset = reg->serial_data_offset;
	u32 data_base = reg->data_base_addr;
	int ret = 0;

	if (!dn_size)
		goto out;

	t_dev_dbg_fwup_wr(ts->dev,
		"%s: wdata: addr 0x%04X, size 0x%08X\n",
		__func__, addr, dn_size);
	__fw_wr_data_dbg(ts, dn_buf, dn_size);

	ret = lxs_hal_fw_wr_value(ts, offset, addr);
	if (ret < 0) {
		t_dev_err(ts->dev,
			"%s: wdata failed(%d) - addr 0x%04X, value 0x%X\n",
			__func__, ret, offset, addr);
		goto out;
	}

	ret = lxs_hal_reg_write(ts, data_base, (void *)dn_buf, dn_size);
	if (ret < 0) {
		int prt_len = min(dn_size, 16);
		t_dev_err(ts->dev,
			"%s: wdata failed(%d) - addr 0x%04X, buf %*ph %s\n",
			__func__, ret, data_base, prt_len, dn_buf,
			(dn_size > prt_len) ? "..." : "");
		goto out;
	}

out:
	return ret;
}

static int lxs_hal_oled_fwup_bdma_sts(struct lxs_ts *ts)
{
	struct lxs_ts_chip *chip = &ts->chip;
	struct lxs_hal_fw_info *fw = &chip->fw;
	u32 sts = fw->bdma_sts;
	u32 sts_tr_busy = fw->bdma_sts_tr_busy;
	int ret = 0;

	if (!sts)
		goto out;

	ret = lxs_hal_condition_wait(ts, sts, NULL,
			sts_tr_busy, sts_tr_busy, 5, 2000, NOT_COND);
	if (ret < 0)
		goto out;

out:
	return ret;
}

static int lxs_hal_oled_fw_read_crc_pass(struct lxs_ts *ts, u32 *data, char *title)
{
	struct lxs_ts_chip *chip = &ts->chip;
	struct lxs_hal_fw_info *fw = &chip->fw;
	u32 addr = fw->gdma_crc_pass;
	u32 __data = 1;
	int ret = 0;

	if (!addr)
		goto out;

	if (ts->plat_data->use_skip_reset)
		goto out;

	if (chip->ops_quirk.hw_reset != NULL)
		goto out;

	ret = lxs_hal_fw_rd_value(ts, addr, &__data, title);
	if (ret < 0)
		return ret;

out:
	if (data)
		*data = __data;

	return 0;
}

static int __oled_fw_sw_crc_t3(struct lxs_ts *ts, const char *title)
{
	struct lxs_ts_chip *chip = &ts->chip;
	struct lxs_hal_fw_info *fw = &chip->fw;
	u32 dsp_saddr = fw->bdma_saddr;
	u32 dsp_ctrl1 =	fw->bdma_ctrl;
	u32 ctrl_data = 0;
	int ret = 0;

	if (!dsp_saddr || !dsp_ctrl1) {
		t_dev_err(ts->dev, "%s: setup error, saddr %04Xh, ctrl1 %04Xh\n",
			title, dsp_saddr, dsp_ctrl1);
		return -EINVAL;
	}

	ret = lxs_hal_fw_wr_value(ts, dsp_saddr, 0);
	if (ret < 0) {
		t_dev_err(ts->dev, "%s: failed - DSP_SADDR(%04Xh) set zero, %d\n",
			title, dsp_saddr, ret);
		goto out;
	}

	ctrl_data = 0x015FFFE9;
	ret = lxs_hal_fw_wr_value(ts, dsp_ctrl1, ctrl_data);
	if (ret < 0) {
		t_dev_err(ts->dev, "%s: failed - DSP_CTRL1(%04Xh) write %08Xh, %d\n",
			title, dsp_ctrl1, ctrl_data, ret);
		goto out;
	}

	ret = lxs_hal_oled_fwup_bdma_sts(ts);

out:
	return ret;
}

static int __oled_fw_sw_crc(struct lxs_ts *ts, const char *title)
{
	struct lxs_ts_chip *chip = &ts->chip;
	struct lxs_hal_fw_info *fw = &chip->fw;
	u32 gdma_saddr = fw->gdma_saddr;
	u32 gdma_ctrl = fw->gdma_ctrl;
	u32 gmda_start = fw->gdma_start;
	u32 ctrl_data = 0;
	int ret = 0;

	if (!gdma_saddr || !gdma_ctrl || !gmda_start) {
		t_dev_err(ts->dev, "%s: setup error, saddr %04Xh, ctrl %04Xh, start %04Xh\n",
			title, gdma_saddr, gdma_ctrl, gmda_start);
		return -EINVAL;
	}

	ret = lxs_hal_fw_wr_value(ts, gdma_saddr, 0);
	if (ret < 0) {
		t_dev_err(ts->dev, "%s: failed - GDMA_SADDR(%04Xh) set zero, %d\n",
			title, gdma_saddr, ret);
		goto out;
	}

	ctrl_data = (fw->sizeof_flash>>2) - 1;
	ctrl_data |= fw->gdma_ctrl_en | fw->gdma_ctrl_ro;
	ret = lxs_hal_fw_wr_value(ts, gdma_ctrl, ctrl_data);
	if (ret < 0) {
		t_dev_err(ts->dev, "%s: failed - GDMA_CTL(%04Xh) write %08Xh, %d\n",
			title, gdma_ctrl, ctrl_data, ret);
		goto out;
	}
	lxs_ts_delay(10);

	ret = lxs_hal_fw_wr_value(ts, gmda_start, 1);
	if (ret < 0) {
		t_dev_err(ts->dev, "%s: failed - GDMA_START(%04Xh) on, %d\n",
			title, gmda_start, ret);
		goto out;
	}

out:
	return ret;
}

static int lxs_hal_oled_fw_sw_crc(struct lxs_ts *ts, const char *title)
{
	struct lxs_ts_chip *chip = &ts->chip;
	int ret = 0;

	switch (chip->opt.t_oled) {
	case 3:
		ret = __oled_fw_sw_crc_t3(ts, title);
		break;
	default:
		ret = __oled_fw_sw_crc(ts, title);
		break;
	}

	return ret;
}

static int lxs_hal_oled_fwup_rst_ctl(struct lxs_ts *ts, int val, const char *str)
{
	struct lxs_ts_chip *chip = &ts->chip;
	struct lxs_hal_reg *reg = chip->reg;
	u32 spr_rst_ctl = reg->spr_rst_ctl;
	char log[64] = { 0, };
	char *name = NULL;
	int ret = 0;

	switch (val) {
	case 2:
		name = "system hold";
		break;
	case 1:
		name = "release cm3";
		break;
	default:
		name = "system release";
		break;
	}

	if (str == NULL)
		snprintf(log, sizeof(log), "%s", name);
	else
		snprintf(log, sizeof(log), "%s for %s", name, str);

	ret = lxs_hal_fw_wr_value(ts, spr_rst_ctl, val);
	if (ret < 0)
		goto out;

	t_dev_dbg_fwup(ts->dev, "FW upgrade: spr_rst_ctl(%d) - %s\n", val, log);

out:
	return ret;
}

static int __used lxs_hal_oled_fwup_flash_crc(struct lxs_ts *ts, u32 *crc_val)
{
	struct lxs_ts_chip *chip = &ts->chip;
	struct lxs_hal_fw_info *fw = &chip->fw;
	u32 gdma_crc_result = fw->gdma_crc_result;
	u32 gdma_crc_pass = fw->gdma_crc_pass;
	u32 data = 0;
	int ret = 0;

	ret = lxs_hal_oled_fw_sw_crc(ts, "FW upgrade");
	if (ret < 0)
		goto out;

	lxs_ts_delay(100);

	ret = lxs_hal_fw_rd_value(ts, gdma_crc_result, &data, NULL);
	if (ret < 0)
		goto out;

	t_dev_info(ts->dev, "FW upgrade: flash crc result(%04Xh) %08Xh\n",
		gdma_crc_result, data);

	if (crc_val != NULL)
		*crc_val = data;

	if (fw->gdma_crc_pass) {
		ret = lxs_hal_oled_fw_read_crc_pass(ts, &data, NULL);
		if (ret < 0)
			goto out;

		t_dev_info(ts->dev, "FW upgrade: flash crc pass(%04Xh) %Xh\n",
			gdma_crc_pass, data);
	}

out:
	lxs_ts_delay(100);

	return ret;
}

static int lax_hal_oled_fwup_flash_sts(struct lxs_ts *ts)
{
	struct lxs_ts_chip *chip = &ts->chip;
	struct lxs_hal_fw_info *fw = &chip->fw;
	u32 sts = fw->flash_status;
	int ret = 0;

	if (!sts)
		goto out;

	ret = lxs_hal_condition_wait(ts, sts, NULL,
			1, 1, 5, 2000, NOT_COND);
	if (ret < 0)
		goto out;

out:
	return ret;
}

static int __oled_fwup_flash_erase_t3(struct lxs_ts *ts)
{
	struct lxs_ts_chip *chip = &ts->chip;
	struct lxs_hal_fw_info *fw = &chip->fw;
	u32 fc_ctrl = fw->fc_ctrl;
	u32 fc_prot = fw->fc_prot;
	u32 fc_start = fw->fc_start;
	u32 sts = fw->flash_status;
	int fc_err = 0;
	int busy_time = fw->fc_erase_wait_time;
	int busy_cnt = fw->fc_erase_wait_cnt;
	u32 chk_resp, data;
	int i;
	int ret = 0;

	for (i = 0; i < 2; i++) {
		fc_err = 0;

		do {
			ret = lxs_hal_fw_wr_value(ts, fc_ctrl, 5);
			if (ret < 0) {
				fc_err = 1;
				break;
			}

			ret = lxs_hal_fw_wr_value(ts, fc_prot, 1);
			if (ret < 0) {
				fc_err = 2;
				break;
			}

			ret = lxs_hal_fw_wr_value(ts, fc_start, 1);
			if (ret < 0) {
				fc_err = 3;
				break;
			}

			chk_resp = 1;
			ret = lxs_hal_condition_wait(ts, sts, &data,
					chk_resp, chk_resp, busy_time, busy_cnt, NOT_COND);
			if (ret < 0) {
				fc_err = 4;
				t_dev_err(ts->dev, "%s: failed - flash erase wait(%Xh), %Xh\n",
					__func__, chk_resp, data);
				break;
			}
		} while (0);

		lxs_ts_delay(10);

		lxs_hal_fw_wr_value(ts, fc_ctrl, 0);
		lxs_hal_fw_wr_value(ts, fc_prot, 0);

		if (!fc_err)
			break;
	}

	if (fc_err) {
		t_dev_err(ts->dev, "%s: failed - flash mass erase error, %d, %d\n",
			__func__, fc_err, ret);
	} else {
		u32 crc_erase_value = 0x48F6B687;
		u32 crc_val= 0;

		ret = lxs_hal_oled_fwup_flash_crc(ts, &crc_val);
		if (ret < 0) {
			t_dev_err(ts->dev, "%s: failed - flash  erase crc, %d\n", __func__, ret);
			goto done;
		}

		if (crc_val != crc_erase_value) {
			t_dev_err(ts->dev, "%s: flash erase crc error %08Xh != %08Xh, %d\n",
				__func__, crc_erase_value, crc_val, ret);
			ret = -EFAULT;
			goto done;
		}

		t_dev_info(ts->dev, "FW upgrade: flash mass erase done\n");
	}

done:
	return ret;
}

static void __oled_fwup_flash_poss_chk(struct lxs_ts *ts)
{
	struct lxs_ts_chip *chip = &ts->chip;
	struct lxs_hal_reg *reg = chip->reg;
	struct lxs_hal_fw_info *fw = &chip->fw;
	u32 fc_ctrl = fw->fc_ctrl;

	switch (chip->opt.t_oled) {
	case 2:
		lxs_hal_fw_wr_value(ts, reg->spr_code_offset, (ts->fw_size - 4)>>2);
		lxs_hal_fw_wr_value(ts, fc_ctrl, fw->fc_ctrl_wr_en);
		lxs_hal_fw_wr_value(ts, reg->code_access_addr, 0);
		lxs_hal_fw_wr_value(ts, fc_ctrl, 0);
		break;
	default:
		break;
	}
}

static int __oled_fwup_flash_erase(struct lxs_ts *ts)
{
	struct lxs_ts_chip *chip = &ts->chip;
	struct lxs_hal_fw_info *fw = &chip->fw;
	u32 fc_addr = fw->fc_addr;
	u32 fc_ctrl = fw->fc_ctrl;
	u32 fc_start = fw->fc_start;
	u32 flash_status = fw->flash_status;
	int fc_err = 0;
	int busy_time = fw->fc_erase_wait_time;
	int busy_cnt = (4<<10)/busy_time;
	u32 chk_resp, data;
	int i;
	int ret = 0;

	__oled_fwup_flash_poss_chk(ts);

	for (i = 0; i < 3; i++) {
		fc_err = 0;

		do {
			ret = lxs_hal_fw_wr_value(ts, fc_addr, 0);
			if (ret < 0) {
				fc_err = 1;
				break;
			}

			ret = lxs_hal_fw_wr_value(ts, fc_ctrl, fw->fc_ctrl_mass_erase);
			if (ret < 0) {
				fc_err = 2;
				break;
			}

			ret = lxs_hal_fw_wr_value(ts, fc_start, 1);
			if (ret < 0) {
				fc_err = 3;
				break;
			}

			chk_resp = 1;
			ret = lxs_hal_condition_wait(ts, flash_status, &data,
					chk_resp, ~0, busy_time, busy_cnt, NOT_COND);
			if (ret < 0) {
				fc_err = 4;
				t_dev_err(ts->dev, "%s: failed - flash erase wait(%Xh), %Xh\n",
					__func__, chk_resp, data);
				break;
			}
		} while (0);

		lxs_ts_delay(10);

		lxs_hal_fw_wr_value(ts, fc_ctrl, 0);
	}

	if (fc_err)
		t_dev_err(ts->dev, "%s: failed - flash mass erase error, %d, %d\n",
			__func__, fc_err, ret);
	else
		t_dev_info(ts->dev, "FW upgrade: flash mass erase done\n");

	return ret;
}

static int lxs_hal_oled_fwup_flash_prot_con(struct lxs_ts *ts, int wr)
{
	struct lxs_hal_fw_info *fw = &ts->chip.fw;

	if (fw->fc_prot)
		return lxs_hal_fw_wr_value(ts, fw->fc_prot, (wr) ? 1 : 0);

	return 0;
}

static int lxs_hal_oled_fwup_flash_write_en(struct lxs_ts *ts, int en)
{
	struct lxs_hal_fw_info *fw = &ts->chip.fw;

	if (fw->fc_ctrl)
		return lxs_hal_fw_wr_value(ts, fw->fc_ctrl, (en) ? fw->fc_ctrl_wr_en : 0);

	return 0;
}

static int lxs_hal_oled_fwup_bdma_saddr(struct lxs_ts *ts)
{
	struct lxs_hal_fw_info *fw = &ts->chip.fw;

	if (fw->bdma_saddr)
		return lxs_hal_fw_wr_value(ts, fw->bdma_saddr, fw->datasram_addr);

	return 0;
}

static int lxs_hal_oled_fwup_bdma_daddr(struct lxs_ts *ts, int dst)
{
	struct lxs_hal_fw_info *fw = &ts->chip.fw;

	if (fw->bdma_daddr)
		return lxs_hal_fw_wr_value(ts, fw->bdma_daddr, dst);

	return 0;
}

static int lxs_hal_oled_fwup_bdma_ctrl(struct lxs_ts *ts, int ctrl_val)
{
	struct lxs_hal_fw_info *fw = &ts->chip.fw;

	if (fw->bdma_ctrl)
		return lxs_hal_fw_wr_value(ts, fw->bdma_ctrl, fw->bdma_ctrl_en | ctrl_val);

	return 0;
}

static int lxs_hal_oled_fwup_bdma_cal(struct lxs_ts *ts)
{
	struct lxs_hal_fw_info *fw = &ts->chip.fw;

	if (fw->bdma_cal_op)
		return lxs_hal_fw_wr_value(ts, fw->bdma_cal_op, fw->bdma_cal_op_ctrl);

	return 0;
}

static int lxs_hal_oled_fwup_bdma_start(struct lxs_ts *ts)
{
	struct lxs_hal_fw_info *fw = &ts->chip.fw;

	if (fw->bdma_start)
		return lxs_hal_fw_wr_value(ts, fw->bdma_start, 1);

	return 0;
}

static int lxs_hal_oled_fwup_fw_pre(struct lxs_ts *ts)
{
	struct lxs_ts_chip *chip = &ts->chip;
	int ret = 0;

	ret = lxs_hal_oled_fwup_rst_ctl(ts, 2, NULL);
	if (ret < 0)
		goto out;

	switch (chip->opt.t_oled) {
	case 3:
		ret = __oled_fwup_flash_erase_t3(ts);
		break;
	default:
		ret = __oled_fwup_flash_erase(ts);
		break;
	}
	if (ret < 0)
		goto out;

	switch (chip->opt.t_oled) {
	case 1:
		ret = lxs_hal_oled_fwup_rst_ctl(ts, 0, NULL);
		if (ret < 0)
			goto out;
		break;
	}

out:
	return ret;
}

static int __oled_fwup_flash_con_t3(struct lxs_ts *ts, int fw_addr, int curr_size)
{
	int ret;

	ret = lxs_hal_oled_fwup_bdma_saddr(ts);
	if (ret < 0)
		return ret;

	ret = lxs_hal_oled_fwup_bdma_daddr(ts, fw_addr);
	if (ret < 0)
		return ret;

	ret = lxs_hal_oled_fwup_bdma_cal(ts);
	if (ret < 0)
		return ret;

	ret = lxs_hal_oled_fwup_bdma_ctrl(ts, (((curr_size>>2)-1)<<5));
	if (ret < 0)
		return ret;

	return 0;
}

static int __oled_fwup_flash_con(struct lxs_ts *ts, int fw_addr, int curr_size)
{
	int ret;

	ret = lxs_hal_oled_fwup_bdma_saddr(ts);
	if (ret < 0)
		return ret;

	ret = lxs_hal_oled_fwup_bdma_ctrl(ts, (curr_size>>2));
	if (ret < 0)
		return ret;

	ret = lxs_hal_oled_fwup_bdma_cal(ts);
	if (ret < 0)
		return ret;

	ret = lxs_hal_oled_fwup_bdma_daddr(ts, fw_addr);
	if (ret < 0)
		return ret;

	return 0;
}

static int lxs_hal_oled_fwup_fw_core(struct lxs_ts *ts, u8 *dn_buf, int dn_size)
{
	struct lxs_ts_chip *chip = &ts->chip;
	u8 *fw_data = NULL;
	int fw_size = 0;
	int fw_addr = 0;
	int curr_size = 0;
	int fw_size_org = dn_size;
	int fw_dn_size = 0, fw_dn_percent;
	int buf_size = 0;
	int dn_wr_en_pre = 0;
	int dn_wr_en_mid = 0;
	int dn_log_unit = FW_DN_LOG_UNIT;
	int flash_sts = 0;
	int ret = 0;
	int (*fw_flash_con)(struct lxs_ts *ts, int fw_addr, int curr_size);

	buf_size = min(FW_DN_LOG_UNIT, (int)lxs_hal_fw_act_buf_size(ts));

	fw_flash_con = __oled_fwup_flash_con;

	switch (chip->opt.t_oled) {
	case 3:
		fw_flash_con = __oled_fwup_flash_con_t3;

		buf_size = min(TS_MAX_BUF_SIZE, (int)lxs_hal_fw_act_buf_size(ts));
		dn_wr_en_pre = 1;
		flash_sts = 1;
		break;
	case 2:
		dn_wr_en_mid = 1;
		/* fall through */
	case 1:
		ret = lxs_hal_oled_fwup_rst_ctl(ts, 2, "flash write");
		if (ret < 0)
			goto out;
		break;
	}

	dn_log_unit = min(FW_DN_LOG_UNIT, buf_size);

	if (dn_wr_en_pre) {
		ret = lxs_hal_oled_fwup_flash_prot_con(ts, 1);
		if (ret < 0)
			goto out;
	}

	ret = lxs_hal_oled_fwup_bdma_sts(ts);
	if (ret < 0)
		goto out;

	fw_data = dn_buf;
	fw_size = dn_size;
	fw_addr = 0;

	while (fw_size) {
		curr_size = min(fw_size, buf_size);

		t_dev_dbg_fwup(ts->dev, "FW upgrade: flash write %08Xh %04Xh\n", fw_addr, curr_size);

		ret = lxs_hal_fw_wr_data(ts, 0, fw_data, curr_size);
		if (ret < 0)
			break;

		ret = fw_flash_con(ts, fw_addr, curr_size);
		if (ret < 0)
			goto out;

		if (dn_wr_en_mid) {
			ret = lxs_hal_oled_fwup_flash_write_en(ts, 1);
			if (ret < 0)
				goto out;
		}

		ret = lxs_hal_oled_fwup_bdma_start(ts);
		if (ret < 0)
			goto out;

		ret = lxs_hal_oled_fwup_bdma_sts(ts);
		if (ret < 0)
			goto out;

		if (flash_sts) {
			ret = lax_hal_oled_fwup_flash_sts(ts);
			if (ret < 0)
				goto out;
		}

		fw_addr += curr_size;
		fw_data += curr_size;
		fw_size -= curr_size;

		fw_dn_size += curr_size;
		if (!fw_size || !(fw_dn_size & (dn_log_unit-1))) {
			fw_dn_percent = (fw_dn_size * 100);
			fw_dn_percent /= fw_size_org;

			t_dev_info(ts->dev, "FW upgrade: downloading...(%d%c)\n",
				fw_dn_percent, '%');
		}
	}

out:
	lxs_hal_oled_fwup_flash_write_en(ts, 0);

//	lxs_hal_oled_fwup_rst_ctl(ts, 0, NULL);

	return ret;
}

static int lxs_hal_oled_fwup_fw_post(struct lxs_ts *ts)
{
	struct lxs_ts_chip *chip = &ts->chip;
	u32 boot_code_addr = chip->fw.boot_code_addr;
	u32 chk_resp, data;
	int ret = 0;

	if (!boot_code_addr)
		goto out;

	ret = lxs_hal_fw_wr_value(ts, boot_code_addr, FW_BOOT_LOADER_INIT);
	if (ret < 0)
		goto out;

	lxs_ts_delay(100);

	ret = lxs_hal_oled_fwup_rst_ctl(ts, 0, NULL);
	if (ret < 0)
		goto out;

	lxs_ts_delay(200);

	chk_resp = FW_BOOT_LOADER_CODE;
	ret = lxs_hal_condition_wait(ts, boot_code_addr, &data,
			chk_resp, ~0, 10, 20, EQ_COND);
	if (ret < 0) {
		t_dev_err(ts->dev, "%s: failed - boot check(%Xh), %08Xh\n",
			__func__, chk_resp, data);
		goto out;
	}

	t_dev_info(ts->dev, "FW upgrade: boot loader ready\n");

out:
	return ret;
}

static int __used lxs_hal_oled_fwup_fw(struct lxs_ts *ts,
				u8 *fw_buf, int fw_size)
{
	int fw_size_max = ts->fw_size;
	int ret = 0;

	ret = lxs_hal_oled_fwup_fw_pre(ts);
	if (ret < 0)
		goto out;

	ret = lxs_hal_oled_fwup_fw_core(ts, fw_buf, fw_size_max);
	if (ret < 0)
		goto out;

	ret = lxs_hal_oled_fwup_fw_post(ts);
	if (ret < 0)
		goto out;

out:
	return ret;
}

static int lxs_hal_oled_fwup_post(struct lxs_ts *ts)
{
	struct lxs_ts_chip *chip = &ts->chip;
	struct lxs_hal_fw_info *fw = &chip->fw;
	u32 boot_code_addr = chip->fw.boot_code_addr;
	u32 crc_fixed_value = fw->crc_fixed_value;
	u32 crc_val;
	int ret = 0;

	if (boot_code_addr) {
		ret = lxs_hal_oled_fwup_rst_ctl(ts, 0, NULL);
		if (ret < 0)
			goto out;

		lxs_ts_delay(100);
	}

	ret = lxs_hal_oled_fwup_rst_ctl(ts, 2, "crc");
	if (ret < 0)
		goto out;

	lxs_ts_delay(100);

	ret = lxs_hal_oled_fwup_flash_crc(ts, &crc_val);
	if (ret < 0) {
		t_dev_err(ts->dev, "%s: failed - flash crc, %d\n", __func__, ret);
		goto out;
	}
	if (crc_val != crc_fixed_value) {
		t_dev_err(ts->dev, "%s: flash crc error %08Xh != %08Xh, %d\n",
			__func__, crc_fixed_value, crc_val, ret);
		ret = -EFAULT;
		goto out;
	}
	t_dev_info(ts->dev, "FW upgrade: flash crc check done\n");

	ret = lxs_hal_oled_fwup_rst_ctl(ts, 1, NULL);
	if (ret < 0)
		goto out;

	lxs_ts_delay(100);

	ret = lxs_hal_oled_fwup_rst_ctl(ts, 0, NULL);
	if (ret < 0)
		goto out;

	lxs_ts_delay(100);

	t_dev_dbg_fwup(ts->dev, "FW upgrade: post done\n");

	return 0;

out:
	lxs_hal_oled_fwup_rst_ctl(ts, 0, NULL);

	lxs_ts_delay(100);

	return ret;
}

static int lxs_hal_oled_fwup_verify(struct lxs_ts *ts, u8 *fw_buf, int fw_size)
{
	int fw_size_max = ts->fw_size;
	u32 fw_code_crc = *(u32 *)&fw_buf[fw_size_max - 4];
	u32 fw_code_size = *(u32 *)&fw_buf[fw_size_max - 8];

	if (fw_size < fw_size_max) {
		t_dev_err(ts->dev, "FW chk_img: too small img size(%Xh), must be >= fw_size_max(%Xh)\n",
			fw_size, fw_size_max);
		return OLED_E_FW_CODE_SIZE_ERR;
	}

	t_dev_info(ts->dev, "FW chk_img: code size %Xh, code crc %Xh\n",
		fw_code_size, fw_code_crc);

	if (fw_code_size > fw_size_max) {
		t_dev_err(ts->dev, "FW chk_img: invalid code_size(%Xh), must be <= fw_size_max(%Xh)\n",
			fw_code_size, fw_size_max);
		return OLED_E_FW_CODE_SIZE_ERR;
	}

	if (fw_size == fw_size_max)
		return OLED_E_FW_CODE_ONLY_VALID;

	return -EINVAL;
}

#define t_dev_dbg_fw_setup(_dev, _format, _fw, _element, args...)	\
		t_dev_dbg_fwup(_dev, "%-20s = " _format "\n", #_element, _fw->_element, ##args)

static void lxs_hal_oled_fwup_setup(struct lxs_ts *ts)
{
	struct lxs_ts_chip *chip = &ts->chip;
	struct lxs_hal_fw_info *fw = &chip->fw;

	fw->boot_code_addr = 0x01A;
	fw->sizeof_flash = ts->fw_size;
	fw->gdma_saddr = 0x056;
	fw->gdma_ctrl = 0x058;
	fw->gdma_ctrl_en = GDMA_CTRL_EN;
	fw->gdma_ctrl_ro = GDMA_CTRL_READONLY;
	fw->gdma_start = 0x059;
	fw->fc_ctrl = 0x06B;
	fw->fc_ctrl_mass_erase = FC_CTRL_MASS_ERASE;
	fw->fc_ctrl_wr_en = FC_CTRL_WR_EN;
	fw->fc_start = 0x06C;
	fw->fc_addr = 0x06D;
	fw->flash_status = 0xFE2;
	fw->fc_erase_wait_cnt = 200;
	fw->fc_erase_wait_time = 5;
	fw->bdma_saddr = 0x072;
	fw->bdma_daddr = 0x073;
	fw->bdma_cal_op = 0;
	fw->bdma_cal_op_ctrl = 0;
	fw->bdma_ctrl = 0x074;
	fw->bdma_ctrl_en = BDMA_CTRL_EN;
	fw->bdma_ctrl_bst = BDMA_CTRL_BST;
	fw->bdma_start = 0x075;
	fw->bdma_sts = 0x077;
	fw->bdma_sts_tr_busy = BDMA_STS_TR_BUSY;
	fw->datasram_addr = 0x20000000;
	fw->gdma_crc_result = 0x05D;
	fw->gdma_crc_pass = 0x05E;
	fw->crc_fixed_value = 0x800D800D;
	fw->cfg_chip_id = 0;

	fw->info_ptr = chip->reg->info_ptr;

	switch (chip->opt.t_oled) {
	case 3:
		fw->boot_code_addr = 0;
		fw->gdma_saddr = 0;
		fw->gdma_ctrl = 0;
		fw->gdma_start = 0;
		fw->fc_ctrl = 0x084;
		fw->fc_prot = 0x085;
		fw->fc_start = 0x086;
		fw->fc_addr = 0;
		fw->flash_status = 0x08A;
		fw->fc_erase_wait_cnt = 400;
		fw->fc_erase_wait_time = 5;
		fw->bdma_saddr = 0x08E;
		fw->bdma_daddr = 0x090;
		fw->bdma_cal_op = 0x08D;
		fw->bdma_cal_op_ctrl = 0x00200400;
		fw->bdma_ctrl = 0x08C;
		fw->bdma_ctrl_en = ((1<<0) | (1<<1) | (2<<21) | (2<<23) | (2<<25));
		fw->bdma_ctrl_bst = 0;
		fw->bdma_start = 0;
		fw->bdma_sts = BIT(31) | (0xFF6<<16) | 0x096;
		fw->bdma_sts_tr_busy = 0x1;
		fw->datasram_addr = 0x20000000;
		fw->gdma_crc_result = 0x099;
		fw->gdma_crc_pass = 0;
		fw->crc_fixed_value = 0xDEBB20E3;
		break;
	case 2:
		fw->sys_id_addr = 0x8014;
		fw->boot_code_addr = 0;
		fw->gdma_saddr = 0x085;
		fw->gdma_ctrl = 0x087;
		fw->gdma_start = 0x088;
		fw->fc_ctrl = 0x09A;
		fw->fc_start = 0x09B;
		fw->fc_addr = 0x09C;
		fw->bdma_saddr = 0x0A1;
		fw->bdma_daddr = 0x0A3;
		fw->bdma_cal_op = 0xA5;
		fw->bdma_cal_op_ctrl = 3 | (1024<<2) | (1024<<13);
		fw->bdma_ctrl = 0x0A6;
		fw->bdma_ctrl_en = BDMA_CTRL_EN | (2<<23) | (2<<25) | (2<<27);
		fw->bdma_ctrl_bst = BIT(17);
		fw->bdma_start = 0x0A7;
		fw->bdma_sts = 0x0A9;
		fw->datasram_addr = 0x20000000;
		fw->gdma_crc_result = 0x08C;
		fw->gdma_crc_pass = 0x08D;
		break;
	}

	t_dev_dbg_fw_setup(ts->dev, "0x%08X", fw, sizeof_flash);
	t_dev_dbg_fw_setup(ts->dev, "0x%04X", fw, boot_code_addr);
	t_dev_dbg_fw_setup(ts->dev, "0x%04X", fw, gdma_saddr);
	t_dev_dbg_fw_setup(ts->dev, "0x%04X", fw, gdma_ctrl);
	t_dev_dbg_fw_setup(ts->dev, "0x%08X", fw, gdma_ctrl_en);
	t_dev_dbg_fw_setup(ts->dev, "0x%08X", fw, gdma_ctrl_ro);
	t_dev_dbg_fw_setup(ts->dev, "0x%04X", fw, gdma_start);
	t_dev_dbg_fw_setup(ts->dev, "0x%04X", fw, fc_ctrl);
	t_dev_dbg_fw_setup(ts->dev, "0x%04X", fw, fc_prot);
	t_dev_dbg_fw_setup(ts->dev, "0x%08X", fw, fc_ctrl_mass_erase);
	t_dev_dbg_fw_setup(ts->dev, "0x%08X", fw, fc_ctrl_wr_en);
	t_dev_dbg_fw_setup(ts->dev, "0x%04X", fw, fc_start);
	t_dev_dbg_fw_setup(ts->dev, "0x%04X", fw, fc_addr);
	t_dev_dbg_fw_setup(ts->dev, "0x%04X", fw, flash_status);
	t_dev_dbg_fw_setup(ts->dev, "%d", fw, fc_erase_wait_cnt);
	t_dev_dbg_fw_setup(ts->dev, "%d", fw, fc_erase_wait_time);
	t_dev_dbg_fw_setup(ts->dev, "0x%04X", fw, bdma_saddr);
	t_dev_dbg_fw_setup(ts->dev, "0x%04X", fw, bdma_daddr);
	if (fw->bdma_cal_op)
		t_dev_dbg_fw_setup(ts->dev, "0x%04X", fw, bdma_cal_op);

	t_dev_dbg_fw_setup(ts->dev, "0x%04X", fw, bdma_ctrl);
	t_dev_dbg_fw_setup(ts->dev, "0x%08X", fw, bdma_ctrl_en);
	t_dev_dbg_fw_setup(ts->dev, "0x%08X", fw, bdma_ctrl_bst);
	t_dev_dbg_fw_setup(ts->dev, "0x%04X", fw, bdma_start);
	t_dev_dbg_fw_setup(ts->dev, "0x%04X", fw, bdma_sts);
	t_dev_dbg_fw_setup(ts->dev, "0x%08X", fw, bdma_sts_tr_busy);
	t_dev_dbg_fw_setup(ts->dev, "0x%08X", fw, datasram_addr);
	t_dev_dbg_fw_setup(ts->dev, "0x%04X", fw, info_ptr);
	t_dev_dbg_fw_setup(ts->dev, "0x%04X", fw, gdma_crc_result);
	t_dev_dbg_fw_setup(ts->dev, "0x%04X", fw, gdma_crc_pass);
	t_dev_dbg_fw_setup(ts->dev, "0x%08X", fw, crc_fixed_value);
}

static int __used lxs_hal_oled_fwup_upgrade(struct lxs_ts *ts,
			u8 *fw_buf, int fw_size, int retry)
{
	int fw_size_max = ts->fw_size;
	u32 include_conf = !!(fw_size > fw_size_max);
	int ret = 0;

	if (!retry) {
		lxs_hal_oled_fwup_setup(ts);
	}

	t_dev_info(ts->dev, "FW upgrade:%s include conf data\n",
			(include_conf) ? "" : " not");

	t_dev_dbg_fwup(ts->dev, "FW upgrade: fw size %08Xh, fw_size_max %08Xh\n",
			fw_size, fw_size_max);

	ret = lxs_hal_oled_fwup_verify(ts, fw_buf, fw_size);
	switch (ret) {
	case OLED_E_FW_CODE_ONLY_VALID:
		include_conf = 0;
		break;
	case OLED_E_FW_CODE_AND_CFG_VALID:
		break;
	case OLED_E_FW_CODE_SIZE_ERR:
	case OLED_E_FW_CODE_CFG_ERR:
	default:
		ret = -EPERM;
		goto out;
	}

	if (ts->fquirks->init_pre) {
		ret = ts->fquirks->init_pre(ts);
		if (ret < 0)
			goto out;
	}

	ret = lxs_hal_oled_fwup_fw(ts, fw_buf, fw_size);
	if (ret < 0) {
		lxs_hal_set_fwup_status(ts, FWUP_STATUS_NG_CODE);
		goto out;
	}

	if (include_conf) {
		/* TBD */
	}

	ret = lxs_hal_oled_fwup_post(ts);

out:
	return ret;
}

static int lxs_hal_oled_boot_result(struct lxs_ts *ts, u32 *boot_st)
{
	struct lxs_ts_chip *chip = &ts->chip;
	struct lxs_hal_fw_info *fw = &chip->fw;
	char *title = "boot result";
	u32 gdma_crc_result = fw->gdma_crc_result;
	u32 crc_fixed_value = fw->crc_fixed_value;
	u32 info_ptr = fw->info_ptr;
	u32 rdata_crc = 0;
	u32 rdata_pass = 0;
	u32 rdata_ptr = 0;
	int err = 0;
	int ret = 0;

	/*
	 * Check CRC
	 */
	ret = lxs_hal_fw_rd_value(ts, gdma_crc_result, &rdata_crc, title);
	if (ret < 0)
		return ret;

	err |= (rdata_crc != crc_fixed_value);

	ret = lxs_hal_oled_fw_read_crc_pass(ts, &rdata_pass, title);
	if (ret < 0)
		return ret;

	err |= (!rdata_pass)<<1;

	/*
	 * Check Boot
	 */
	if (info_ptr) {
		ret = lxs_hal_fw_rd_value(ts, info_ptr, &rdata_ptr, title);
		if (ret < 0)
			return ret;

		err |= (!rdata_ptr)<<2;
	}

	if (boot_st != NULL)
		(*boot_st) = (err) ? BIT(BOOT_STS_POS_DUMP_ERR) : BIT(BOOT_STS_POS_DUMP_DONE);

	if (err)
		t_dev_err(ts->dev, "%s: %Xh(%Xh, %Xh, %Xh)\n",
			title, err, rdata_crc, rdata_pass, rdata_ptr);

	return 0;
}

static int lxs_hal_oled_sw_reset_post(struct lxs_ts *ts)
{
	int ret = 0;

	t_dev_info(ts->dev, "boot run sw crc\n");

	lxs_ts_delay(ts->sw_reset_delay);

	ret = lxs_hal_oled_fw_sw_crc(ts, "boot");

	return ret;
}

static int __used lxs_hal_fw_compare(struct lxs_ts *ts, u8 *fw_buf)
{
	struct lxs_ts_chip *chip = &ts->chip;
	struct lxs_hal_fw_info *fw = &chip->fw;
	struct lxs_hal_tc_version_bin *bin_ver;
	int fw_max_size = ts->fw_size;
	u32 bin_ver_offset = 0;
	u32 bin_pid_offset = 0;
	u32 dev_major = 0;
	u32 dev_minor = 0;
	u32 dev_build = 0;
	char pid[12] = {0, };
	u32 bin_major = 0;
	u32 bin_minor = 0;
	u32 bin_build = 0;
//	u32 bin_raw = 0;
	int update = 0;
	int use_fw_ver_diff = !!ts->plat_data->use_fw_ver_diff;
	int use_fw_skip_pid = !!ts->plat_data->use_fw_skip_pid;
	int boot_failed = !!(atomic_read(&chip->boot) == TC_IC_BOOT_FAIL);
//	int ret = 0;

	use_fw_skip_pid |= !!(ts->force_fwup & TS_FORCE_FWUP_SKIP_PID);

	if (!boot_failed) {
		dev_major = fw->v.version.major;
		dev_minor = fw->v.version.minor;
		dev_build = fw->v.version.build;

		if (!use_fw_skip_pid) {
			if (!dev_major && !dev_minor) {
				t_dev_err(ts->dev, "fw can not be 0.0!! Check your panel connection!!\n");
			//	return 0;
			}
		}
	}

	bin_ver_offset = *((u32 *)&fw_buf[BIN_VER_OFFSET_POS]);
	if (!bin_ver_offset) {
		t_dev_err(ts->dev, "%s: zero ver offset\n", __func__);
		return -EINVAL;
	}

	bin_pid_offset = *((u32 *)&fw_buf[BIN_PID_OFFSET_POS]);
	if (!bin_pid_offset) {
		t_dev_err(ts->dev, "%s: zero pid offset\n", __func__);
		return -EINVAL;
	}

	if (((bin_ver_offset + 4) > fw_max_size) ||
		((bin_pid_offset + 8) > fw_max_size)) {
		t_dev_err(ts->dev, "%s: invalid offset - ver %06Xh, pid %06Xh, max %06Xh\n",
			__func__, bin_ver_offset, bin_pid_offset, fw_max_size);
		return -EINVAL;
	}

	t_dev_dbg_fwup(ts->dev, "ver %06Xh, pid %06Xh\n",
		bin_ver_offset, bin_pid_offset);

	memcpy(pid, &fw_buf[bin_pid_offset], 8);
	t_dev_dbg_fwup(ts->dev, "pid %s\n", pid);

	if (lxs_hal_fw_check_pid(pid)) {
		t_dev_err(ts->dev, "[fw-bin] invalid pid - \"%s\"\n", pid);
		return -EINVAL;
	}

	bin_ver = (struct lxs_hal_tc_version_bin *)&fw_buf[bin_ver_offset];
	switch (chip->opt.t_bin) {
	case 1:
		bin_major = bin_ver->build;
		bin_minor = bin_ver->minor;
		bin_build = bin_ver->major;
		break;
	default:
		bin_major = bin_ver->major;
		bin_minor = bin_ver->minor;
		bin_build = bin_ver->build;
		break;
	}

	t_dev_info(ts->dev,
		"FW compare: bin-ver: %d.%02d(%d) (%s)\n",
		bin_major, bin_minor, bin_build, pid);

	if (boot_failed) {
		update |= BIT(7);
		goto out;
	}

	t_dev_info(ts->dev, "FW compare: dev-ver: %d.%02d(%d) (%s)\n",
			dev_major, dev_minor, dev_build, fw->product_id);

	if (ts->force_fwup) {
		update |= BIT(0);
	} else {
		if (use_fw_ver_diff) {
			u32 bin_ver_val = (bin_major<<16) | (bin_minor<<8) | bin_build;
			u32 dev_ver_val = (dev_major<<16) | (dev_minor<<8) | dev_build;
			if (bin_ver_val != dev_ver_val) {
				update |= BIT(8);
			}
		} else {
			update |= lxs_hal_fw_ver_cmp(bin_major, bin_minor, bin_build,
						dev_major, dev_minor, dev_build);
		}
	}

	if (use_fw_skip_pid) {
		t_dev_warn(ts->dev, "FW compare: skip pid check\n");
		goto out;
	}

#if 0	/* requires more consideration */
	if (fw->invalid_pid) {
		t_dev_warn(ts->dev,
			"FW compare: dev-pid is invalid, skip pid check\n");
		goto out;
	}
#endif

	if (memcmp(pid, fw->product_id, 8)) {
		if (fw->invalid_pid) {
			t_dev_err(ts->dev,
				"%s: bin-pid[%s], dev-pid invalid, halted (up %02X, fup %02X)\n",
				__func__, pid, update, ts->force_fwup);
			return -EINVAL;

		}

		t_dev_err(ts->dev,
			"%s: bin-pid[%s] != dev-pid[%s], halted (up %02X, fup %02X)\n",
			__func__, pid, fw->product_id, update, ts->force_fwup);
		return -EINVAL;
	}

out:
	t_dev_info(ts->dev,
		"FW compare: up 0x%02X, fup 0x%02X\n",
		update, ts->force_fwup);

	return update;
}

static int lxs_hal_fw_do_get_fw_abs(struct lxs_ts *ts,
			const struct firmware **fw_p, const char *name)
{
	return -EIO;
}

static int lxs_hal_fw_do_get_file(struct lxs_ts *ts,
			const struct firmware **fw_p,
			const char *name, int abs_path)
{
	if (abs_path)
		return lxs_hal_fw_do_get_fw_abs(ts, fw_p, name);

	return request_firmware(fw_p, name, ts->dev);
}

//#define __LXS_DEBUG_FW_FILE_PATH

static int lxs_hal_fw_get_file(struct lxs_ts *ts,
		const struct firmware **fw_p, char *fwpath)
{
	struct lxs_ts_chip *chip = &ts->chip;
	const struct firmware *fw = NULL;
	char *src_path;
	int src_len;
	int abs_path = 0;
	int ret = 0;

	if (ts->test_fwpath[0]) {
		src_path = (char *)ts->test_fwpath;
	} else if (ts->plat_data->fw_name) {
		src_path = (char *)ts->plat_data->fw_name;
	} else {
		t_dev_err(ts->dev, "no target fw defined\n");
		ret = -ENOENT;
		goto out;
	}

	/*
	 * path option : following first character '/'
	 * Absolute : first character shall be '/'
	 *            data loading via kernel_read_file_from_path/filp_open
	 *            echo /sdcard/firmware/target_fw_img > fw_upgrade
	 * Relative : first character shall not be '/'
	 *            data loading via request_firmware
	 *            echo firmware/target_fw_img > fw_upgrade
	 */
	src_len = strlen(src_path);
#if defined(__LXS_DEBUG_FW_FILE_PATH)
	t_dev_info(ts->dev, "src input: %s (%d)\n", src_path, src_len);
#endif

	abs_path = !!(src_path[0] == '/');

	chip->fw_abs_path = abs_path;

	strncpy(fwpath, src_path, src_len);
	fwpath[src_len] = 0;

#if defined(__LXS_DEBUG_FW_FILE_PATH)
	t_dev_info(ts->dev, "target fw: %s (%s) (%d)\n",
		fwpath, (abs_path) ? "abs" : "rel", src_len);
#else
	t_dev_info(ts->dev, "target fw: %s (%s)\n",
		fwpath, (abs_path) ? "abs" : "rel");
#endif

	ret = lxs_hal_fw_do_get_file(ts, &fw,
			(const char *)fwpath, abs_path);
	if (ret < 0) {
		if (ret == -ENOENT)
			t_dev_err(ts->dev, "can't find fw: %s\n", fwpath);
		else
			t_dev_err(ts->dev, "can't %s fw: %s, %d\n",
				(abs_path) ? "read" : "request",
				fwpath, ret);

		goto out;
	}

	if (fw_p)
		*fw_p = fw;

out:
	return ret;
}

static void lxs_hal_fw_release_firm(struct lxs_ts *ts,
			const struct firmware *fw)
{
	struct lxs_ts_chip *chip = &ts->chip;

	if (fw == NULL) {
		return;
	}

	if (chip->fw_abs_path) {
		chip->fw_abs_path = 0;
	#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
		vfree(fw->data);
	#else
		kfree(fw->data);
	#endif
		kfree(fw);
		return;
	}

	release_firmware(fw);
}

void lxs_hal_upgrade_setup(struct lxs_ts *ts)
{
	struct lxs_ts_chip *chip = &ts->chip;

	t_dev_info(ts->dev, "fw upgrade setup\n");

	chip->fwup_ops.fwup_check = lxs_hal_fw_compare;

	if (chip->opt.t_oled) {
		lxs_hal_oled_fwup_setup(ts);

		if (chip->opt.t_sw_rst != TC_SW_RST_TYPE_NONE)
			chip->ops_quirk.sw_reset_post = lxs_hal_oled_sw_reset_post;

		chip->ops_quirk.boot_result = lxs_hal_oled_boot_result;
		chip->fwup_ops.fwup_upgrade = lxs_hal_oled_fwup_upgrade;
	}

	if (ts->fquirks->fwup_check)
		chip->fwup_ops.fwup_check = ts->fquirks->fwup_check;

	if (ts->fquirks->fwup_upgrade)
		chip->fwup_ops.fwup_upgrade = ts->fquirks->fwup_upgrade;
}

/*
 * FW upgrade option
 *
 * 1. Default upgrade (through version comparison)
 *    do upgarde using request_firmware (relative path)
 * 2. echo /.../fw_img > fw_upgrade
 *    do force-upgrade using request_firmware (relative path)
 * 3. echo {root}/.../fw_img > fw_upgrade
 *    do force-upgrade using normal file open control (absolute path)
 */
static int lxs_hal_upgrade_not_allowed(struct lxs_ts *ts)
{
	struct lxs_ts_chip *chip = &ts->chip;

	if (chip->lcd_mode != LCD_MODE_U3) {
		t_dev_warn(ts->dev, "%s: not NP mode, %s(%d)\n", __func__,
			lxs_lcd_driving_mode_str(chip->lcd_mode), chip->lcd_mode);
		return 1;
	}

	if (atomic_read(&chip->boot) == TC_IC_BOOT_DONE)
		return 0;

	if (lxs_hal_access_not_allowed(ts, "FW_Upgrade", HAL_ACCESS_CHK_SKIP_INIT))
		return 1;

	return 0;
}

static int lxs_hal_upgrade_pre(struct lxs_ts *ts)
{
	struct lxs_ts_chip *chip = &ts->chip;
	struct lxs_hal_reg *reg = chip->reg;
	int ctrl = chip->tc_cmd_table[LCD_MODE_STOP];
	u32 rdata;
	int ret = 0;

	if ((ctrl < 0) || !touch_mode_allowed(ts, LCD_MODE_STOP))
		goto out;

	if (lxs_addr_is_invalid(reg->tc_drive_ctl))
		goto out;

	/*
	 * TC_STOP before fw upgrade
	 * to avoid unexpected IRQ drop by internal watchdog
	 */
	rdata = reg->tc_drive_ctl;

	ret = lxs_hal_write_value(ts, rdata, ctrl);
	if (ret < 0) {
		t_dev_err(ts->dev, "%s: TC stop(%04Xh, 0x%08X) failed\n",
			__func__, rdata, ctrl);
		goto out;
	}

	t_dev_info(ts->dev, "FW upgrade: TC stop(%04Xh, 0x%08X)\n", rdata, ctrl);

	rdata = chip->drv_delay;
	lxs_ts_delay(rdata);

out:
	return ret;
}

int lxs_hal_upgrade(struct lxs_ts *ts)
{
	struct lxs_ts_chip *chip = &ts->chip;
	struct lxs_hal_fwup_ops *fwup_ops = &chip->fwup_ops;
	const struct firmware *fw = NULL;
	char fwpath[128] = { 0, };
	u8 *fw_buf = NULL;
	int fw_max_size = ts->fw_size;
	int fw_size = 0;
	int i = 0;
	int chk = 0;
	int ret = 0;

	if (!fw_max_size) {
		t_dev_warn(ts->dev, "%s: fw_size not set\n", __func__);
		lxs_hal_set_fwup_status(ts, FWUP_STATUS_NG_OP);
		return -EFAULT;
	}

	if ((fwup_ops->fwup_check == NULL) || (fwup_ops->fwup_upgrade == NULL)) {
		t_dev_warn(ts->dev, "%s: ops not set\n", __func__);
		lxs_hal_set_fwup_status(ts, FWUP_STATUS_NG_OP);
		return -EFAULT;
	}

	lxs_hal_set_fwup_status(ts, FWUP_STATUS_BUSY);

	chip->fw_abs_path = 0;

	if (lxs_hal_upgrade_not_allowed(ts)) {
		t_dev_warn(ts->dev, "%s: not granted\n", __func__);
		lxs_hal_set_fwup_status(ts, FWUP_STATUS_NG_OP);
		return EACCES;
	}

	t_dev_info(ts->dev, "fw type: %s\n", FW_TYPE_STR);

	t_dev_info(ts->dev, "getting fw from file\n");
	ret = lxs_hal_fw_get_file(ts, &fw, fwpath);
	if (ret < 0) {
		lxs_hal_set_fwup_status(ts, FWUP_STATUS_NG_F_OPEN);
		goto out;
	}
	fw_buf = (u8 *)fw->data;
	fw_size = (int)fw->size;

//	ret = -EINVAL;
	ret = -EPERM;

	if ((fw_buf == NULL) || !fw_size) {
		t_dev_err(ts->dev, "invalid fw info\n");
		lxs_hal_set_fwup_status(ts, FWUP_STATUS_NG_F_CHK);
		goto out_fw;
	}

	if (fw_size < fw_max_size) {
		t_dev_err(ts->dev, "invalid fw size: %Xh < %Xh\n",
			fw_size, fw_max_size);
		lxs_hal_set_fwup_status(ts, FWUP_STATUS_NG_F_CHK);
		goto out_fw;
	}

	t_dev_info(ts->dev, "fw size: %d\n", fw_size);

	chk = fwup_ops->fwup_check(ts, fw_buf);
	if (chk < 0) {
		lxs_hal_set_fwup_status(ts, FWUP_STATUS_NG_F_CHK);
		ret = chk;
		goto out_fw;
	}

	if (!chk) {
		lxs_hal_set_fwup_status(ts, FWUP_STATUS_NG_F_CHK);
		goto out_fw;
	}

	ret = lxs_hal_upgrade_pre(ts);
	if (ret < 0) {
		lxs_hal_set_fwup_status(ts, FWUP_STATUS_NG_IO);
		goto out_fw;
	}

	lxs_ts_delay(100);

	for (i = 0; i < 2; i++) {
		ret = fwup_ops->fwup_upgrade(ts, fw_buf, fw_size, i);
		if (ret >= 0)
			break;
	}

out_fw:
	lxs_hal_fw_release_firm(ts, fw);

out:
	if (ret < 0) {
		if (lxs_hal_get_fwup_status(ts) == FWUP_STATUS_BUSY)
			lxs_hal_set_fwup_status(ts, FWUP_STATUS_NG_IO);
	} else {
		lxs_hal_set_fwup_status(ts, FWUP_STATUS_OK);
	}

	return ret;
}


