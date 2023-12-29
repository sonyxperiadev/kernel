/*
 * SPDX-License-Identifier: GPL-2.0
 *
 * lxs_ts_sysfs.c
 *
 * LXS touch sysfs control
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "lxs_ts.h"

#define _reg_snprintf(_buf, _size, _reg, _element)	\
		lxs_snprintf(_buf, _size, "# 0x%04X [reg->%s]\n", _reg->_element, #_element)

#define _reg_snprintf_fw(_buf, _size, _fw, _element)	\
		lxs_snprintf(_buf, _size, "# 0x%04X [fw->%s]\n", _fw->_element, #_element)

static int __show_reg_list(struct device *dev, char *buf, int size)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);
	struct lxs_ts_chip *chip = &ts->chip;
	struct lxs_hal_reg *reg = chip->reg;

	size += lxs_snprintf(buf, size, "# Reg. Map List\n");

	size += _reg_snprintf(buf, size, reg, info_ptr);
	size += _reg_snprintf(buf, size, reg, spr_chip_test);
	size += _reg_snprintf(buf, size, reg, spr_chip_id);
	size += _reg_snprintf(buf, size, reg, spr_rst_ctl);
	size += _reg_snprintf(buf, size, reg, spr_boot_ctl);
	size += _reg_snprintf(buf, size, reg, spr_sram_ctl);
	size += _reg_snprintf(buf, size, reg, spr_boot_status);
	size += _reg_snprintf(buf, size, reg, spr_code_offset);
	size += _reg_snprintf(buf, size, reg, tc_ic_status);
	size += _reg_snprintf(buf, size, reg, tc_status);
	size += _reg_snprintf(buf, size, reg, tc_version);
	size += _reg_snprintf(buf, size, reg, tc_product_id1);
	size += _reg_snprintf(buf, size, reg, tc_product_id2);
	size += _reg_snprintf(buf, size, reg, info_chip_version);
	size += _reg_snprintf(buf, size, reg, info_lot_num);
	size += _reg_snprintf(buf, size, reg, info_serial_num);
	size += _reg_snprintf(buf, size, reg, info_date);
	size += _reg_snprintf(buf, size, reg, info_time);
	size += _reg_snprintf(buf, size, reg, code_access_addr);
	size += _reg_snprintf(buf, size, reg, data_base_addr);
	size += _reg_snprintf(buf, size, reg, prd_tcm_base_addr);
	size += _reg_snprintf(buf, size, reg, tc_device_ctl);
	if (!lxs_addr_is_invalid(reg->tc_drive_ctl))
		size += _reg_snprintf(buf, size, reg, tc_drive_ctl);

	size += _reg_snprintf(buf, size, reg, cmd_raw_data_report_mode_read);
	size += _reg_snprintf(buf, size, reg, cmd_raw_data_compress_write);
	size += _reg_snprintf(buf, size, reg, cmd_raw_data_report_mode_write);
	size += _reg_snprintf(buf, size, reg, res_info);
	size += _reg_snprintf(buf, size, reg, channel_info);
	size += _reg_snprintf(buf, size, reg, t_frame_rate);
	size += _reg_snprintf(buf, size, reg, charger_info);
	size += _reg_snprintf(buf, size, reg, frame_rate);
	size += _reg_snprintf(buf, size, reg, gaming_mode);
	size += _reg_snprintf(buf, size, reg, glove_mode);
	size += _reg_snprintf(buf, size, reg, ime_state);
	size += _reg_snprintf(buf, size, reg, call_state);
	size += _reg_snprintf(buf, size, reg, grab_mode);

	size += _reg_snprintf(buf, size, reg, dim_sys);
	size += _reg_snprintf(buf, size, reg, dim_eng);

	size += _reg_snprintf(buf, size, reg, tc_tsp_test_ctl);
	size += _reg_snprintf(buf, size, reg, tc_tsp_test_status);
	size += _reg_snprintf(buf, size, reg, tc_tsp_test_pf_result);
	size += _reg_snprintf(buf, size, reg, tc_tsp_test_off_info);
	size += _reg_snprintf(buf, size, reg, tc_flash_dn_status);
	size += _reg_snprintf(buf, size, reg, tc_confdn_base_addr);
	size += _reg_snprintf(buf, size, reg, tc_flash_dn_ctl);
	size += _reg_snprintf(buf, size, reg, raw_data_ctl_read);
	size += _reg_snprintf(buf, size, reg, raw_data_ctl_write);
	size += _reg_snprintf(buf, size, reg, serial_data_offset);

	size += _reg_snprintf(buf, size, reg, prd_serial_tcm_offset);
	size += _reg_snprintf(buf, size, reg, prd_tc_mem_sel);
	size += _reg_snprintf(buf, size, reg, prd_tc_test_mode_ctl);
	size += _reg_snprintf(buf, size, reg, prd_m1_m2_raw_offset);
	size += _reg_snprintf(buf, size, reg, prd_tune_result_offset);
	size += _reg_snprintf(buf, size, reg, prd_open3_short_offset);
	size += _reg_snprintf(buf, size, reg, prd_ic_ait_start_reg);
	size += _reg_snprintf(buf, size, reg, prd_ic_ait_data_readystatus);

	/* __LXS_FW_TYPE_OLED_BASE */
	if (chip->opt.t_oled) {
		struct lxs_hal_fw_info *fw = &chip->fw;

		size += lxs_snprintf(buf, size, "\n");

		size += _reg_snprintf_fw(buf, size, fw, boot_code_addr);
		size += _reg_snprintf_fw(buf, size, fw, gdma_saddr);
		size += _reg_snprintf_fw(buf, size, fw, gdma_ctrl);
		size += _reg_snprintf_fw(buf, size, fw, gdma_start);
		size += _reg_snprintf_fw(buf, size, fw, fc_ctrl);
		size += _reg_snprintf_fw(buf, size, fw, fc_start);
		size += _reg_snprintf_fw(buf, size, fw, fc_addr);
		size += _reg_snprintf_fw(buf, size, fw, flash_status);
		size += _reg_snprintf_fw(buf, size, fw, bdma_saddr);
		size += _reg_snprintf_fw(buf, size, fw, bdma_daddr);
		if (fw->bdma_cal_op) {
			size += _reg_snprintf_fw(buf, size, fw, bdma_cal_op);
		}
		size += _reg_snprintf_fw(buf, size, fw, bdma_ctrl);
		size += _reg_snprintf_fw(buf, size, fw, bdma_start);
		size += _reg_snprintf_fw(buf, size, fw, bdma_sts);
		size += _reg_snprintf_fw(buf, size, fw, info_ptr);
		size += _reg_snprintf_fw(buf, size, fw, gdma_crc_result);
		size += _reg_snprintf_fw(buf, size, fw, gdma_crc_pass);
	}

	return size;
}

static ssize_t show_reg_list(struct device *dev, char *buf)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);
	struct lxs_ts_chip *chip = &ts->chip;
//	struct lxs_hal_reg *reg = chip->reg;
	struct lxs_hal_fw_info *fw = &chip->fw;
	u32 bootmode = 0;
	u32 boot_chk_offset = 0;
	int size = 0;

	lxs_hal_get_boot_result(ts, &bootmode);

	size += __show_reg_list(dev, buf, size);

	size += lxs_snprintf(buf, size, "\n");

	size += lxs_snprintf(buf, size,
				">> version    : v%u.%02u, chip : %u, protocol : %u\n",
				fw->v.version.major,
				fw->v.version.minor,
				fw->v.version.chip,
				fw->v.version.protocol);

	size += lxs_snprintf(buf, size,
				">> revision   : %d\n",
				fw->revision);

	size += lxs_snprintf(buf, size,
				">> product id : %s\n",
				fw->product_id);

	boot_chk_offset = lxs_hal_boot_sts_pos_busy(ts);
	size += lxs_snprintf(buf, size,
				">> flash boot : %s(%s), crc %s (0x%08X)\n",
				(bootmode >> (boot_chk_offset) & 0x1) ? "BUSY" : "idle",
				(bootmode >> (boot_chk_offset + 1) & 0x1) ? "done" : "booting",
				(bootmode >> (boot_chk_offset + 2) & 0x1) ? "ERROR" : "ok",
				bootmode);

	size += lxs_snprintf(buf, size,
				">> status     : type %d[%08Xh, %08Xh, %08Xh]\n",
				chip->status_type,
				chip->status_mask_normal,
				chip->status_mask_logging,
				chip->status_mask_reset);

	size += lxs_snprintf(buf, size, "\n");

	return (ssize_t)size;
}

static int __show_reg_ctrl_log_history(struct lxs_ts *ts, char *buf)
{
	struct lxs_ts_chip *chip = &ts->chip;
	struct lxs_hal_reg_log *reg_log = chip->reg_log;
	char *dir_name;
	int reg_burst;
	int reg_err;
	int i;
	int size = 0;

	size += lxs_snprintf(buf, size, "[Test History]\n");
	for (i = 0; i < REG_LOG_MAX; i++) {
		if (reg_log->dir) {
			dir_name = !!((reg_log->dir & REG_DIR_MASK) == REG_DIR_WR) ?
						"wr" : "rd";
			reg_burst = !!(reg_log->dir & REG_BURST);
			reg_err = !!(reg_log->dir & REG_DIR_ERR);
		} else {
			dir_name = "__";
			reg_burst = 0;
			reg_err = 0;
		}

		if (reg_burst)
			size += lxs_snprintf(buf, size, " %s: reg[0x%04X] = (burst) %s\n",
						dir_name, reg_log->addr,
						(reg_err) ? "(err)" : "");
		else
			size += lxs_snprintf(buf, size, " %s: reg[0x%04X] = 0x%08X %s\n",
						dir_name, reg_log->addr, reg_log->data,
						(reg_err) ? "(err)" : "");

		reg_log++;
	}

	return size;
}

static ssize_t show_reg_ctrl(struct device *dev, char *buf)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);
	int size = 0;

	size += __show_reg_ctrl_log_history(ts, buf);

	size += lxs_snprintf(buf, size, "\n[Usage]\n");
	size += lxs_snprintf(buf, size, " echo wr 0x1234 {value} > reg_ctrl\n");
	size += lxs_snprintf(buf, size, " echo rd 0x1234 > reg_ctrl\n");
	size += lxs_snprintf(buf, size, " (burst access)\n");
	size += lxs_snprintf(buf, size, " echo rd 0x1234 0x111 > reg_ctrl, 0x111 is size(max 0x%X)\n",
		REG_BURST_MAX);

	return (ssize_t)size;
}

static void __store_reg_ctrl_log_add(struct lxs_ts *ts,
				struct lxs_hal_reg_log *new_log)

{
	struct lxs_ts_chip *chip = &ts->chip;
	struct lxs_hal_reg_log *reg_log = chip->reg_log;

	memmove(&reg_log[1], reg_log, sizeof(*reg_log) * (REG_LOG_MAX - 1));
	memcpy(reg_log, new_log, sizeof(*reg_log));
}

static inline void __store_reg_ctrl_rd_burst_log(struct lxs_ts *ts,
					u8 *row_buf, int row, int col)
{
	if (col)
		t_dev_info(ts->dev, "rd: [%3Xh] %*ph\n", row, col, row_buf);
}

static int __store_reg_ctrl_rd_burst(struct lxs_ts *ts, u32 addr, int size, int burst)
{
	u8 *rd_buf, *row_buf;
	int col_power = REG_BURST_COL_PWR;
	int col_width = (1<<col_power);
	int row_curr, col_curr;
	int ret = 0;

	size = min(size, REG_BURST_MAX);

	rd_buf = (u8 *)kzalloc(size, GFP_KERNEL);
	if (rd_buf == NULL) {
		t_dev_err(ts->dev, "failed to allocate rd_buf\n");
		return -ENOMEM;
	}

	if (burst)
		ret = lxs_hal_reg_read(ts, addr, rd_buf, size);
	else
		ret = lxs_hal_reg_read_single(ts, addr, rd_buf, size);
	if (ret < 0)
		goto out;

	t_dev_info(ts->dev, "rd: addr %04Xh, size %Xh %s\n", addr, size,
		(burst) ? "(burst)" : "");

	row_buf = rd_buf;
	row_curr = 0;
	while (size) {
		col_curr = min(col_width, size);

		__store_reg_ctrl_rd_burst_log(ts, row_buf, row_curr, col_curr);

		row_buf += col_curr;
		row_curr += col_curr;
		size -= col_curr;
	}

out:
	kfree(rd_buf);

	return ret;
}

static ssize_t store_reg_ctrl(struct device *dev,
				const char *buf, size_t count)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);
	struct lxs_hal_reg_log reg_log;
	char command[6] = {0};
	u32 reg = 0;
	u32 data = 1;
	u32 reg_addr;
	int wr = -1;
	int rd = -1;
	int value = 0;
	int ret = 0;

	if (sscanf(buf, "%5s %X %X", command, &reg, &value) <= 0)
		return count;

	t_dev_info(dev, "%s: command %s, reg 0x%X, value 0x%X\n",
		__func__, command, reg, value);

	if (!strcmp(command, "wr") || !strcmp(command, "write"))
		wr = 1;
	else if (!strcmp(command, "rd") || !strcmp(command, "read"))
		rd = 1;		/* single */
	else if (!strcmp(command, "rdb") || !strcmp(command, "readb"))
		rd = 2;		/* burst */

	reg_addr = reg;
	if (wr != -1) {
		data = value;
		ret = lxs_hal_write_value(ts, reg_addr, data);
		reg_log.dir = REG_DIR_WR;
		reg_log.addr = reg_addr;
		reg_log.data = data;
		if (ret >= 0)
			t_dev_info(dev,
				"wr: reg[0x%04X] = 0x%08X\n",
				reg_addr, data);
		else
			reg_log.dir |= REG_DIR_ERR;

		__store_reg_ctrl_log_add(ts, &reg_log);
		goto out;
	}

	if (rd != -1) {
		reg_log.dir = REG_DIR_RD;
		reg_log.addr = reg_addr;
		if (value <= 4) {
			ret = lxs_hal_read_value(ts, reg_addr, &data);
			reg_log.data = data;
			if (ret >= 0)
				t_dev_info(dev,
					"rd: reg[0x%04X] = 0x%08X\n",
					reg_addr, data);
		} else {
			reg_log.dir |= REG_BURST;
			ret = __store_reg_ctrl_rd_burst(ts, reg_addr, value, (rd == 2));
		}
		if (ret < 0)
			reg_log.dir |= REG_DIR_ERR;

		__store_reg_ctrl_log_add(ts, &reg_log);
		goto out;
	}

	t_dev_info(dev, "[Usage]\n");
	t_dev_info(dev, " echo wr 0x1234 {value} > reg_ctrl\n");
	t_dev_info(dev, " echo rd 0x1234 > reg_ctrl\n");
	t_dev_info(dev, " (burst access)\n");
	t_dev_info(dev, " echo rd 0x1234 0x111 > reg_ctrl, 0x111 is size(max 0x%X)\n",
		REG_BURST_MAX);

out:
	return count;
}

static ssize_t show_reset_ctrl(struct device *dev, char *buf)
{
	int size = 0;

	size += lxs_snprintf(buf, size, "Reset Control Usage\n");
	size += lxs_snprintf(buf, size,
				" SW Reset        : echo %d > hal_reset_ctrl\n",
				TC_SW_RESET);
	size += lxs_snprintf(buf, size,
				" HW Reset(Async) : echo %d > hal_reset_ctrl\n",
				TC_HW_RESET_ASYNC);
	size += lxs_snprintf(buf, size,
				" HW Reset(Sync)  : echo %d > hal_reset_ctrl\n",
				TC_HW_RESET_SYNC);

	size += lxs_snprintf(buf, size,
				" HW Reset(Cond)  : echo 0x%X > hal_reset_ctrl\n",
				TC_HW_RESET_COND);

	return size;
}

static ssize_t store_reset_ctrl(struct device *dev,
				const char *buf, size_t count)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);
	int type = 0;

	if (sscanf(buf, "%X", &type) <= 0)
		return count;

	t_dev_info(dev, "%s: type 0x%X\n", __func__, type);

	if (type != TC_HW_RESET_COND)
		lxs_hal_reset(ts, type, 0);

	return count;
}

static ssize_t show_debug_tc_cmd(struct device *dev, char *buf)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);
	struct lxs_ts_chip *chip = &ts->chip;
	char *mode_str, *ext_str;
	int *tc_cmd_table = chip->tc_cmd_table;
	int size = 0;
	int ctrl, i;

	size += lxs_snprintf(buf, size,
		"[%s tc cmd set] (mode bit %04Xh)\n",
		ts->chip_name, ts->mode_allowed);

	for (i = 0; i < LCD_MODE_MAX; i++) {
		mode_str = (char *)lxs_lcd_driving_mode_str(i);
		ctrl = tc_cmd_table[i];

		if (ctrl < 0)
			ext_str = "(not granted)";
		else if (!touch_mode_allowed(ts, i))
			ext_str = "(not allowed)";
		else
			ext_str = "";

		size += lxs_snprintf(buf, size,
			" %04Xh [%-13s] %s\n",
			ctrl, mode_str, ext_str);
	}

	size += lxs_snprintf(buf, size, "\n");

	return (ssize_t)size;
}

static ssize_t store_debug_tc_cmd(struct device *dev,
				const char *buf, size_t count)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);
	struct lxs_ts_chip *chip = &ts->chip;
	int *tc_cmd_table = chip->tc_cmd_table;
	char *mode_str;
	int mode, value;
	int ctrl;

	if (sscanf(buf, "%X %X", &mode, &value) <= 0)
		return count;

	t_dev_info(dev, "%s: mode 0x%X, value 0x%X\n",
		__func__, mode, value);

	if (mode >= LCD_MODE_MAX) {
		t_dev_err(dev, "Invalid mode: %d >= LCD_MODE_MAX(%d)\n",
			mode, LCD_MODE_MAX);
		return count;
	}

	mode_str = (char *)lxs_lcd_driving_mode_str(mode);
	ctrl = tc_cmd_table[mode];

	if (!touch_mode_allowed(ts, mode)) {
		t_dev_info(dev, "%s(%d) is not allowed\n", mode_str, mode);
		goto out;
	}

	if (value >= 0xFFFF)
		value = -1;

	if (value == ctrl)
		goto out;

	tc_cmd_table[mode] = value;

	t_dev_info(dev, "%s(%d) changed: %04Xh -> %04Xh\n",
		mode_str, mode, ctrl, value);

out:
	return count;
}

static ssize_t show_debug_power(struct device *dev, char *buf)
{
	int size = 0;

	size += lxs_snprintf(buf, size, "[Usage]\n");
	size += lxs_snprintf(buf, size, " off : echo 0 > debug_power\n");
	size += lxs_snprintf(buf, size, " on  : echo 1 > debug_power\n");
	size += lxs_snprintf(buf, size, "\n");

	return (ssize_t)size;
}

static ssize_t store_debug_power(struct device *dev,
					const char *buf, size_t count)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);
	int ctrl = 0;

	if (sscanf(buf, "%d", &ctrl) <= 0)
		return count;

	t_dev_info(dev, "%s: ctrl %d\n", __func__, ctrl);

	if (!ctrl)
		lxs_ts_irq_control(ts, 0);

	lxs_ts_power_reset(ts, (ctrl) ? TS_POWER_ON : TS_POWER_OFF);

	if (ctrl)
		queue_delayed_work(ts->wq, &ts->init_work, msecs_to_jiffies(ts->hw_reset_delay));

	return count;
}

static ssize_t show_fwup_status(struct device *dev, char *buf)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);
	int fwup_status = lxs_hal_get_fwup_status(ts);
	const char *str = lxs_fwup_status_str(fwup_status);
	int size = 0;

	if (fwup_status != FWUP_STATUS_BUSY)
		t_dev_info(dev, "fwup_status: %d(%s)\n", fwup_status, str);

	size += lxs_snprintf(buf, size, "%d\n", fwup_status);

	return (ssize_t)size;
}

static int __get_version(struct device *dev, char *buf, int flag)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);
	struct lxs_ts_chip *chip = &ts->chip;
//	struct lxs_hal_reg *reg = chip->reg;
	struct lxs_hal_fw_info *fw = &chip->fw;
	int offset = 0;
//	int ret = 0;

	if (!flag)
		return 0;

	if (flag & TS_GET_CHIP_NAME)
		offset += lxs_snprintf(buf, offset,
					"chip : %s\n",
					ts->chip_name);

	if (flag & (TS_GET_VERSION|TS_GET_VER_SIMPLE)) {
		char *ver_tag = (flag & TS_GET_VER_SIMPLE) ? "" : "version : ";
		offset += lxs_snprintf(buf, offset,
					"%sv%u.%02u\n",
					ver_tag,
					fw->v.version.major, fw->v.version.minor);
	}

	if (flag & TS_GET_REVISION) {
		if (chip->fw.revision == 0xFF) {
			offset += lxs_snprintf(buf, offset,
						"revision : Flash Erased(0xFF)\n");
		} else {
			offset += lxs_snprintf(buf, offset,
						"revision : 0x%02X\n", fw->revision);
		}
	}

	if (flag & TS_GET_PRODUCT)
		offset += lxs_snprintf(buf, offset,
					"product id : %s\n", fw->product_id);

	return offset;
}

static ssize_t show_version(struct device *dev, char *buf)
{
	return __get_version(dev, (char *)buf, TS_GET_ALL);
}

static ssize_t show_version_no(struct device *dev, char *buf)
{
	return __get_version(dev, (char *)buf, TS_GET_VER_SIMPLE);
}

static ssize_t show_lpwg_data(struct device *dev, char *buf)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);
	struct lxs_ts_chip *chip = &ts->chip;
	int i;
	int size = 0;

	mutex_lock(&ts->lock);

	for (i = 0; i < TS_MAX_LPWG_CODE; i++) {
		if ((chip->knock[i].x == -1) && (chip->knock[i].y == -1))
			break;

		size += lxs_snprintf(buf, size, "%d %d\n",
					chip->knock[i].x, chip->knock[i].y);

		if (!chip->knock[i].x && !chip->knock[i].y)
			break;
	}
	memset(chip->knock, 0, sizeof(chip->knock));

	mutex_unlock(&ts->lock);

	return size;
}

static ssize_t show_channel_info(struct device *dev, char *buf)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);
	struct lxs_ts_chip *chip = &ts->chip;
	int size = 0;

	t_dev_info(dev, "tx %d, rx %d, res_x %d, res_x %d, max_x %d, max_y %d\n",
		chip->tx_count, chip->rx_count, chip->res_x, chip->res_y,
		ts->plat_data->max_x, ts->plat_data->max_y);

	size += lxs_snprintf(buf, size,
			"tx %d, rx %d, res_x %d, res_x %d, max_x %d, max_y %d\n",
			chip->tx_count, chip->rx_count, chip->res_x, chip->res_y,
			ts->plat_data->max_x, ts->plat_data->max_y);

	return (ssize_t)size;
}

static ssize_t show_glove_state(struct device *dev, char *buf)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);
	struct lxs_ts_chip *chip = &ts->chip;
	int value = chip->glove;
	int size = 0;

	size += lxs_snprintf(buf, size, "Glove chk: %s(%08Xh)\n",
				(value & 0x1) ? "ON" : "OFF", value);

	return (ssize_t)size;
}

static ssize_t store_glove_state(struct device *dev,
				const char *buf, size_t count)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);
	struct lxs_ts_chip *chip = &ts->chip;
	int value = 0;

	if (ts->state_core != TS_CORE_NORMAL)
		return count;

	if (sscanf(buf, "%X", &value) <= 0)
		return count;

	t_dev_info(dev, "%s: value %d\n", __func__, value);

	mutex_lock(&ts->lock);

	chip->glove = value;

	lxs_hal_tc_con(ts, TCON_GLOVE, NULL);

	mutex_unlock(&ts->lock);

	return count;
}

static ssize_t show_grab_state(struct device *dev, char *buf)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);
	struct lxs_ts_chip *chip = &ts->chip;
	int value = chip->grab;
	int size = 0;

	size += lxs_snprintf(buf, size, "Grab chk: %s(%08Xh)\n",
				(value & 0x1) ? "ON" : "OFF", value);

	return (ssize_t)size;
}

static ssize_t store_grab_state(struct device *dev,
				const char *buf, size_t count)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);
	struct lxs_ts_chip *chip = &ts->chip;
	int value = 0;

	if (ts->state_core != TS_CORE_NORMAL)
		return count;

	if (sscanf(buf, "%X", &value) <= 0)
		return count;

	t_dev_info(dev, "%s: value %d\n", __func__, value);

	mutex_lock(&ts->lock);

	chip->grab = value;

	lxs_hal_tc_con(ts, TCON_GRAB, NULL);

	mutex_unlock(&ts->lock);

	return count;
}

static ssize_t show_usb_state(struct device *dev, char *buf)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);

	return lxs_snprintf(buf, 0, "%d\n", ts->state_connect);
}

static ssize_t store_usb_state(struct device *dev,
				const char *buf, size_t count)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);
	int value = 0;

	if (ts->state_core != TS_CORE_NORMAL)
		return count;

	if (sscanf(buf, "%d", &value) <= 0)
		goto out;

	t_dev_info(dev, "%s: value %d\n", __func__, value);

	mutex_lock(&ts->lock);

	ts->state_connect = value;

	lxs_hal_usb_status(ts);

	mutex_unlock(&ts->lock);

out:
	return count;
}

static ssize_t show_wireless_state(struct device *dev, char *buf)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);

	return lxs_snprintf(buf, 0, "%d\n", ts->state_wireless);
}

static ssize_t store_wireless_state(struct device *dev,
				const char *buf, size_t count)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);
	int value = 0;

	if (ts->state_core != TS_CORE_NORMAL)
		return count;

	if (sscanf(buf, "%d", &value) <= 0)
		goto out;

	t_dev_info(dev, "%s: value %d\n", __func__, value);

	mutex_lock(&ts->lock);

	ts->state_wireless = value;

	lxs_hal_wireless_status(ts);

	mutex_unlock(&ts->lock);

out:
	return count;
}

static ssize_t show_ime_state(struct device *dev, char *buf)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);

	return lxs_snprintf(buf, 0, "%d\n", ts->state_ime);
}

static ssize_t store_ime_state(struct device *dev,
				const char *buf, size_t count)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);
	int value = 0;

	if (ts->state_core != TS_CORE_NORMAL)
		return count;

	if (sscanf(buf, "%d", &value) <= 0)
		goto out;

	if ((value < TS_IME_OFF) || (value > TS_IME_SWYPE)) {
		t_dev_warn(dev, "IME : Unknown state, %d\n", value);
		goto out;
	}

	t_dev_info(dev, "%s: value %d\n", __func__, value);

	if (ts->state_ime == value)
		goto out;

	mutex_lock(&ts->lock);

	ts->state_ime = value;

	lxs_hal_ime_state(ts);

	mutex_unlock(&ts->lock);

out:
	return count;
}

static ssize_t show_incoming_call(struct device *dev, char *buf)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);

	return lxs_snprintf(buf, 0, "%d\n", ts->state_incoming_call);
}

static ssize_t store_incoming_call(struct device *dev,
				const char *buf, size_t count)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);
	int value = 0;

	if (ts->state_core != TS_CORE_NORMAL)
		return count;

	if (sscanf(buf, "%d", &value) <= 0)
		return count;

	t_dev_info(dev, "%s: value %d\n", __func__, value);

	if (value >= TS_INCOMING_CALL_IDLE && value <= TS_INCOMING_CALL_OFFHOOK) {
		if (ts->state_incoming_call == value)
			return count;

		mutex_lock(&ts->lock);

		ts->state_incoming_call = value;

		lxs_hal_incoming_call(ts);

		mutex_unlock(&ts->lock);
	} else {
		t_dev_info(dev, "Incoming-call : Unknown %d\n", value);
	}

	return count;
}

static ssize_t show_frame_table(struct device *dev, char *buf)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);
	char *sbuf = NULL;
	int *d_frame_tbl_np = ts->d_frame_tbl_np;
	int *d_frame_tbl_lp = ts->d_frame_tbl_lp;
	int *t_frame_tbl_np = ts->t_frame_tbl_np;
	int *t_frame_tbl_lp = ts->t_frame_tbl_lp;
	int slen = 128;
	int size = 0;
	int l_sz = 0;
	int val = 0;
	int i;

	sbuf = (char *)kzalloc(slen, GFP_KERNEL);
	if (sbuf == NULL)
		return (size_t)size;

	l_sz = __lxs_snprintf(sbuf, slen, 0, "d: np ");
	for (i = 0; i < TS_FRAME_TBL_SZ; i++) {
		val = *d_frame_tbl_np++;
		if (val < 0)
			break;
		l_sz += __lxs_snprintf(sbuf, slen, l_sz, "%d ", val);
	}

	l_sz += __lxs_snprintf(sbuf, slen, l_sz, "| lp ");
	for (i = 0; i < TS_FRAME_TBL_SZ; i++) {
		val = *d_frame_tbl_lp++;
		if (val < 0)
			break;
		l_sz += __lxs_snprintf(sbuf, slen, l_sz, "%d ", val);
	}

	l_sz += __lxs_snprintf(sbuf, slen, l_sz, "(off %d, aod %d)",
		ts->d_frame_rate_off, ts->d_frame_rate_aod);

	size += lxs_snprintf(buf, size, "%s\n", sbuf);
	t_dev_info(dev, "%s: %s\n", __func__, sbuf);

	memset(sbuf, 0, slen);
	l_sz = __lxs_snprintf(sbuf, slen, 0, "t: np ");
	for (i = 0; i < TS_FRAME_TBL_SZ; i++) {
		val = *t_frame_tbl_np++;
		if (val < 0)
			break;
		l_sz += __lxs_snprintf(sbuf, slen, l_sz, "%d ", val);
	}

	l_sz += __lxs_snprintf(sbuf, slen, l_sz, "| lp ");
	for (i = 0; i < TS_FRAME_TBL_SZ; i++) {
		val = *t_frame_tbl_lp++;
		if (val < 0)
			break;
		l_sz += __lxs_snprintf(sbuf, slen, l_sz, "%d ", val);
	}

	size += lxs_snprintf(buf, size, "%s\n", sbuf);
	t_dev_info(dev, "%s: %s\n", __func__, sbuf);

	kfree(sbuf);

	return (size_t)size;
}

static ssize_t __show_frame_rate(struct device *dev, char *buf, int mode)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);
	int d_rate = (mode == LCD_MODE_U3) ? ts->d_frame_rate_np : ts->d_frame_rate_lp;
	int t_rate = (mode == LCD_MODE_U3) ? ts->t_frame_rate_np : ts->t_frame_rate_lp;
	int size = 0;

	t_dev_info(dev, "%s: disp %d, touch %d\n", __func__, d_rate, t_rate);

	size += lxs_snprintf(buf, size, "%d,%d\n", d_rate, t_rate);

	return (ssize_t)size;
}

static ssize_t __store_frame_rate(struct device *dev,
				const char *buf, size_t count, int mode)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);
	struct lxs_ts_chip *chip = &ts->chip;
	int is_u3 = (mode == LCD_MODE_U3);
	char *str = (is_u3) ? "NP" : "LP";
	int *d_frame_rate = (is_u3) ? &ts->d_frame_rate_np : &ts->d_frame_rate_lp;
	int *t_frame_rate = (is_u3) ? &ts->t_frame_rate_np : &ts->t_frame_rate_lp;
	int *d_frame_tbl = (is_u3) ? ts->d_frame_tbl_np : ts->d_frame_tbl_lp;
	int *t_frame_tbl = (is_u3) ? ts->t_frame_tbl_np : ts->t_frame_tbl_lp;
	int d_idx = 0;
	int t_idx = 0;
	int d_idx_old = 0;
	int t_idx_old = 0;
	int tc_driving = 1;
	int d_valid = 0;
	int t_valid = 0;

	if (ts->state_core != TS_CORE_NORMAL)
		return count;

	if (sscanf(buf, "%d %d", &d_idx, &t_idx) <= 0)
		return count;

	d_valid = (touch_lookup_frame_tbl(d_frame_tbl, d_idx) >= 0);
	if (!d_valid)
		t_dev_err(dev, "%s: %s: invalid display frame rate index %d\n",
			__func__, str, d_idx);

	t_valid = (touch_lookup_frame_tbl(t_frame_tbl, t_idx) >= 0);
	if (!t_valid)
		t_dev_err(dev, "%s: %s: invalid touch frame rate index %d\n",
			__func__, str, t_idx);

	if (!d_valid || !t_valid)
		return count;

	if ((*d_frame_rate == d_idx) && (*t_frame_rate == t_idx)) {
		t_dev_info(dev, "%s: disp_index %d, touch_index %d\n",
			__func__, d_idx, t_idx);
		return count;
	}

	mutex_lock(&ts->lock);

	d_idx_old = *d_frame_rate;
	t_idx_old = *t_frame_rate;

	*d_frame_rate = d_idx;
	*t_frame_rate = t_idx;

	t_dev_info(dev, "%s: %s disp_index %d(%d), touch_index %d(%d)\n",
		__func__, str, d_idx, d_idx_old, t_idx, t_idx_old);

	if (!ts->probe_done || ts->after_probe.err)
		tc_driving = 0;
	else if (mode != chip->driving_mode)
		tc_driving = 0;

	if (tc_driving)
		lxs_hal_tc_driving(ts, -1);
	else
		t_dev_info(dev, "%s: skip tc_driving\n", __func__);

	mutex_unlock(&ts->lock);

	return count;
}

static ssize_t show_frame_rate_np(struct device *dev, char *buf)
{
	return __show_frame_rate(dev, buf, LCD_MODE_U3);
}

static ssize_t store_frame_rate_np(struct device *dev,
				const char *buf, size_t count)
{
	return __store_frame_rate(dev, buf, count, LCD_MODE_U3);
}

static ssize_t show_frame_rate_lp(struct device *dev, char *buf)
{
	return __show_frame_rate(dev, buf, LCD_MODE_U0);
}

static ssize_t store_frame_rate_lp(struct device *dev,
				const char *buf, size_t count)
{
	return __store_frame_rate(dev, buf, count, LCD_MODE_U0);
}

/*
 * U0   : echo 0 > enabled
 * U3   : echo 1 > enabled
 * STOP : echo 2 > enabled
 */
static ssize_t show_enable(struct device *dev, char *buf)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);
	struct lxs_ts_chip *chip = &ts->chip;

	return lxs_snprintf(buf, 0, "%d\n", chip->enable);
}

static ssize_t store_enable(struct device *dev,
				const char *buf, size_t count)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);
	int value = 0;

	if (ts->state_core != TS_CORE_NORMAL)
		return count;

	if (sscanf(buf, "%d", &value) <= 0)
		return count;

	t_dev_info(dev, "%s: value %d\n", __func__, value);

	switch (value) {
	case LCD_MODE_U0:
	case LCD_MODE_U3:
	case LCD_MODE_STOP:
	/*
	 * When it needs to control 'enable' with safe condition.
	 * The value 1x means that the command is allowed
	 * only when the current touch state is 'resume'.
	 * Ex) write enable 12 : Stop touch operation
	 *                       only when the current touch state is 'resume'
	 */
	case LCD_MODE_U3 + 10:
	case LCD_MODE_STOP + 10:
		lxs_hal_enable_device(ts, value);
		break;
	}

	return count;
}

#if defined(__LXS_SUPPORT_LPWG)
static ssize_t show_enable_lpm(struct device *dev, char *buf)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);
	struct lxs_ts_chip *chip = &ts->chip;

	return lxs_snprintf(buf, 0, "%d\n", chip->enable_lpm);
}

static ssize_t store_enable_lpm(struct device *dev,
				const char *buf, size_t count)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);
	struct lxs_ts_chip *chip = &ts->chip;
	int value;

	if (ts->state_core != TS_CORE_NORMAL)
		return count;

	if (sscanf(buf, "%d", &value) <= 0)
		return count;

	if (!ts->plat_data->use_lpwg) {
		t_dev_info(dev, "use_lpwg blocked\n");
		return count;
	}

	mutex_lock(&ts->lock);

	chip->enable_lpm = value;

	t_dev_info(dev, "set_lowpower_mode: %d\n", chip->enable_lpm);

	mutex_unlock(&ts->lock);

	return count;
}

static ssize_t show_enable_knock(struct device *dev, char *buf)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);
	struct lxs_ts_chip *chip = &ts->chip;

	return lxs_snprintf(buf, 0, "%d\n", chip->enable_knock);
}

static ssize_t store_enable_knock(struct device *dev,
				const char *buf, size_t count)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);
	struct lxs_ts_chip *chip = &ts->chip;
	int value;

	if (ts->state_core != TS_CORE_NORMAL)
		return count;

	if (sscanf(buf, "%d", &value) <= 0)
		return count;

	if (!ts->plat_data->use_lpwg) {
		t_dev_info(dev, "use_lpwg blocked\n");
		return count;
	}

	mutex_lock(&ts->lock);

	chip->enable_knock = value;

	t_dev_info(dev, "enable_knock: %d\n", chip->enable_knock);

	mutex_unlock(&ts->lock);

	return count;
}
#endif

static ssize_t show_enable_prox(struct device *dev, char *buf)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);
	struct lxs_ts_chip *chip = &ts->chip;

	return lxs_snprintf(buf, 0, "%d\n", chip->enable_prox);
}

static ssize_t store_enable_prox(struct device *dev,
				const char *buf, size_t count)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);
	struct lxs_ts_chip *chip = &ts->chip;
	int value;

	if (ts->state_core != TS_CORE_NORMAL)
		return count;

	if (sscanf(buf, "%d", &value) <= 0)
		return count;

	mutex_lock(&ts->lock);

	chip->enable_prox = value;

	t_dev_info(dev, "enable_prox: %d\n", chip->enable_prox);

	mutex_unlock(&ts->lock);

	return count;
}

static ssize_t after_probe_start_store(struct device *dev,
				const char *buf, size_t count)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);

	t_dev_info(dev, "after probe start \n");
	if (ts->probe_done) {
		t_dev_info(dev, "already post probed, need reboot\n");
	} else {
		t_dev_info(dev, "start init work\n");
		queue_delayed_work(ts->wq, &ts->after_probe.start, 0);
	}

	return count;
}

static ssize_t show_rejection_mode(struct device *dev, char *buf)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);

	t_dev_info(ts->dev, "rejection_mode: mode %d, flag %d\n",
		ts->rejection_mode, ts->report_rejected_event_flag);

	return lxs_snprintf(buf, 0, "mode %d, flag %d\n",
		ts->rejection_mode, ts->report_rejected_event_flag);
}

static ssize_t store_rejection_mode(struct device *dev,
				const char *buf, size_t count)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);
	int value, old;

	if (sscanf(buf, "%d", &value) <= 0)
		return count;

	if (value < 0 || value > 1) {
		t_dev_err(dev, "rejection_mode: param out of range\n");
		return count;
	}

	mutex_lock(&ts->lock);

	old = ts->rejection_mode;
	ts->rejection_mode = value;

	ts->report_rejected_event_flag = (value) ? false : true;

	t_dev_info(dev, "rejection_mode: mode %d(%d), flag %d\n",
		ts->rejection_mode, old, ts->report_rejected_event_flag);

	mutex_unlock(&ts->lock);

	return count;
}

static ssize_t show_orientation_change(struct device *dev, char *buf)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);

	t_dev_info(ts->dev, "orientation_change: landscape %d\n", ts->landscape);

	return lxs_snprintf(buf, 0, "landscape %d\n", ts->landscape);
}

static ssize_t store_orientation_change(struct device *dev,
				const char *buf, size_t count)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);
	struct lxs_ts_chip *chip = &ts->chip;
	int value, old;

	if (sscanf(buf, "%d", &value) <= 0)
		return count;

	if (value < 0 || value > 1) {
		t_dev_err(dev, "orientation_change: param out of range\n");
		return count;
	}

	mutex_lock(&ts->lock);

	old = ts->landscape;
	ts->landscape = value;

	t_dev_info(dev, "orientation_change: landscape %d(%d)\n",
		ts->landscape, old);

	chip->grab = !value;
	lxs_hal_tc_con(ts, TCON_GRAB, NULL);

	mutex_unlock(&ts->lock);

	return count;
}

static void update_game_enhancer_grip_rejection_para(struct lxs_ts *ts, bool portrait, int location, int value)
{
	int portrait_offset = 4;

	if (portrait) {
		location -= portrait_offset;
		ts->radius_portrait[location] = value;
		ts->circle_range_p[location] = value * value;

		t_dev_info(ts->dev, "%s: radius_portrait[%d] = %d\n",
			__func__, location, value);
		t_dev_info(ts->dev, "%s: circle_range_p[%d] = %d\n",
			__func__, location, value * value);
	} else {
		ts->radius_landscape[location] = value;
		ts->circle_range_l[location] = value * value;

		t_dev_info(ts->dev, "%s: radius_landscape[%d] = %d\n",
			__func__, location, value);
		t_dev_info(ts->dev, "%s: circle_range_l[%d] = %d\n",
			__func__, location, value * value);
	}
}

static void update_grip_rejection_para(struct lxs_ts *ts, int *value)
{
	int i;

	if (value[0] == 6) {
		memcpy(ts->portrait_buffer, value + 1, sizeof(ts->portrait_buffer));
		for (i = 0; i < ARRAY_SIZE(ts->portrait_buffer); i++) {
			t_dev_info(ts->dev, "%s: portrait_buffer[%d] = %d\n",
				__func__, i, ts->portrait_buffer[i]);
		}
	} else {
		memcpy(ts->landscape_buffer, value + 1, sizeof(ts->landscape_buffer));
		for (i = 0; i < ARRAY_SIZE(ts->landscape_buffer); i++) {
			t_dev_info(ts->dev, "%s: landscape_buffer[%d] = %d\n",
				__func__, i, ts->landscape_buffer[i]);
		}
	}
}

static int __show_range_changer_log_func(struct lxs_ts *ts, char *pbuf, int psize,
			char *name, u32 *data, int len)
{
	char sbuf[64] = { 0, };
	int slen = 0;
	char *estr = ",";
	int i;

	for (i = 0; i < len; i++) {
		if (i == (len - 1))
			estr = "";
		slen += snprintf(sbuf + slen, sizeof(sbuf) - slen, "%d%s", data[i], estr);
	}

	t_dev_info(ts->dev, "range_changer: %18s[0...%d] = %s\n", name, len-1, sbuf);

	return lxs_snprintf(pbuf, psize, "%18s[0...%d] = %s\n", name, len-1, sbuf);
}

#define __show_range_changer_log(_ts, _pbuf, _psize, _name)	\
		__show_range_changer_log_func(_ts, _pbuf, _psize, #_name, _ts->_name, ARRAY_SIZE(_ts->_name))

static ssize_t show_range_changer(struct device *dev, char *buf)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);
	int size = 0;

	size += __show_range_changer_log(ts, buf, size, portrait_buffer);

	size += __show_range_changer_log(ts, buf, size, radius_portrait);

	size += __show_range_changer_log(ts, buf, size, circle_range_p);

	size += __show_range_changer_log(ts, buf, size, landscape_buffer);

	size += __show_range_changer_log(ts, buf, size, radius_landscape);

	size += __show_range_changer_log(ts, buf, size, circle_range_l);

	return (ssize_t)size;
}

static ssize_t store_range_changer(struct device *dev,
				const char *buf, size_t count)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);
	int value[5] = {0, };

	if (sscanf(buf, "%d, %d, %d, %d, %d", &value[0], &value[1], &value[2], &value[3], &value[4]) <= 0)
		return count;

	if (value[0] < 0|| value[0] > 7) {
		t_dev_err(dev, "range_changer: param out of range\n");
		return count;
	}

	mutex_lock(&ts->lock);

	if (value[0] <= 3)
		update_game_enhancer_grip_rejection_para(ts, false, value[0], value[1]);
	else if (value[0] <= 5)
		update_game_enhancer_grip_rejection_para(ts, true, value[0], value[1]);
	else
		update_grip_rejection_para(ts, value);

	mutex_unlock(&ts->lock);

	return count;
}

#if defined(__LXS_SUPPORT_FILTER_CON)
static ssize_t show_blend_filter(struct device *dev, char *buf)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);
	int filter = ts->blend_filter;

	t_dev_info(ts->dev, "%s: %d\n", __func__, filter);

	return lxs_snprintf(buf, 0, "%d\n", filter);
}

/*
 * blend filter
 * 0 : off, 1 : weak, 2 : normal, 3 : strong
 */
static ssize_t store_blend_filter(struct device *dev,
				const char *buf, size_t count)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);
	int value;

	if (ts->state_core != TS_CORE_NORMAL)
		return count;

	if (sscanf(buf, "%d", &value) <= 0)
		return count;

	if (value < 0 || value > 3) {
		t_dev_err(ts->dev, "%s: invalid %d\n", __func__, value);
		return count;
	}

	t_dev_info(dev, "%s: value %d\n", __func__, value);

	mutex_lock(&ts->lock);

	ts->blend_filter = value;

	lxs_hal_blend_filter(ts);

	mutex_unlock(&ts->lock);

	return count;
}

static ssize_t show_stop_filter(struct device *dev, char *buf)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);
	int filter = ts->stop_filter;

	t_dev_info(ts->dev, "%s: %d\n", __func__, filter);

	return lxs_snprintf(buf, 0, "%d\n", filter);
}

/*
 * stop filter
 * 0 : off, 1 : weak, 2 : normal, 3 : strong
 */
static ssize_t store_stop_filter(struct device *dev,
				const char *buf, size_t count)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);
	int value;

	if (ts->state_core != TS_CORE_NORMAL)
		return count;

	if (sscanf(buf, "%d", &value) <= 0)
		return count;

	if (value < 0 || value > 3) {
		t_dev_err(ts->dev, "%s: invalid %d\n", __func__, value);
		return count;
	}

	t_dev_info(dev, "%s: value %d\n", __func__, value);

	mutex_lock(&ts->lock);

	ts->stop_filter = value;

	lxs_hal_stop_filter(ts);

	mutex_unlock(&ts->lock);

	return count;
}

static ssize_t show_wet_mode(struct device *dev, char *buf)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);

	mutex_lock(&ts->lock);

	lxs_hal_wet_mode(ts);

	mutex_unlock(&ts->lock);

	return lxs_snprintf(buf, 0, "%d\n", ts->wet_mode);
}
#endif	/* __LXS_SUPPORT_FILTER_CON */

static ssize_t show_enable_sleep(struct device *dev, char *buf)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);
	struct lxs_ts_chip *chip = &ts->chip;

	return lxs_snprintf(buf, 0, "%d\n", chip->enable_sleep);
}

static ssize_t store_enable_sleep(struct device *dev,
				const char *buf, size_t count)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);
	struct lxs_ts_chip *chip = &ts->chip;
	int value;

	if (ts->state_core != TS_CORE_NORMAL)
		return count;

	if (sscanf(buf, "%d", &value) <= 0)
		return count;

	mutex_lock(&ts->lock);

	chip->enable_sleep = value;

	t_dev_info(dev, "enable_sleep: %d\n", chip->enable_sleep);

	mutex_unlock(&ts->lock);

	return count;
}

#define _plat_data_snprintf(_buf, _size, args...)	\
		lxs_snprintf(_buf, _size, " %-25s = %d\n", ##args)

static ssize_t show_plat_data(struct device *dev, char *buf)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);
	struct lxs_ts_plat_data *plat_data = ts->plat_data;
	int size = 0;

	mutex_lock(&ts->lock);

	size += lxs_snprintf(buf, size, "=== Platform Data ===\n");
	size += _plat_data_snprintf(buf, size,
					"reset_pin", plat_data->reset_pin);
	size += _plat_data_snprintf(buf, size,
					"irq_pin", plat_data->irq_pin);

	size += _plat_data_snprintf(buf, size, "max_x", plat_data->max_x);
	size += _plat_data_snprintf(buf, size, "max_y", plat_data->max_y);

	if (plat_data->use_palm_opt)
		size += _plat_data_snprintf(buf, size,
					"use_palm_opt", plat_data->use_palm_opt);

	size += _plat_data_snprintf(buf, size,
					"use_lpwg", plat_data->use_lpwg);
	size += _plat_data_snprintf(buf, size,
					"use_firmware", plat_data->use_firmware);
	size += _plat_data_snprintf(buf, size,
					"use_fw_upgrade", plat_data->use_fw_upgrade);
	if (plat_data->use_fw_pwr_rst)
		size += _plat_data_snprintf(buf, size,
					"use_fw_pwr_rst", plat_data->use_fw_pwr_rst);

	if (plat_data->use_fw_ver_diff)
		size += _plat_data_snprintf(buf, size,
					"use_fw_ver_diff", plat_data->use_fw_ver_diff);

	if (plat_data->use_fw_skip_pid)
		size += _plat_data_snprintf(buf, size,
					" use_fw_skip_pid", plat_data->use_fw_skip_pid);

	size += lxs_snprintf(buf, size, "firmware: %s\n", plat_data->fw_name);

	mutex_unlock(&ts->lock);

	return size;
}

static ssize_t show_driver_data(struct device *dev, char *buf)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);
	int size = 0;

	mutex_lock(&ts->lock);

	size += lxs_snprintf(buf, size, "=== Driver Data ===\n");

	size += lxs_snprintf(buf, size,
						"Version : %s\n", LXS_DRV_VERSION);

	mutex_unlock(&ts->lock);

	return size;
}

static ssize_t store_fw_upgrade(struct device *dev,
				const char *buf, size_t count)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);
	int data = 0;

	if (sscanf(buf, "%255s %X", ts->test_fwpath, &data) <= 0)
		return count;

	t_dev_info(dev, "Manual F/W upgrade with %s\n", ts->test_fwpath);

	ts->force_fwup |= TS_FORCE_FWUP_SYS_STORE;
	ts->force_fwup |= (data == 0x5A5A) ? TS_FORCE_FWUP_SKIP_PID : 0;

	queue_delayed_work(ts->wq, &ts->upgrade_work, 0);

	return count;
}

static ssize_t show_fw_upgrade(struct device *dev, char *buf)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);

	ts->force_fwup |= TS_FORCE_FWUP_SYS_SHOW;
	ts->test_fwpath[0] = '\0';

	queue_delayed_work(ts->wq, &ts->upgrade_work, 0);

	return 0;
}

static ssize_t show_irq_flag(struct device *dev, char *buf)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);

	return lxs_snprintf(buf, 0, "irq flag: 0x%X\n\n", (u32)ts->irqflags);
}

static ssize_t show_irq_level(struct device *dev, char *buf)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);

	return lxs_snprintf(buf, 0, "%d\n", gpio_get_value(ts->plat_data->irq_pin));
}

static ssize_t show_irq_state(struct device *dev, char *buf)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);

	return lxs_snprintf(buf, 0, "irq state: %d\n", ts->state_irq_enable);
}

static ssize_t store_irq_state(struct device *dev,
				const char *buf, size_t count)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);
	int value = 0;
	int value_swt, value_ext;

	if (ts->state_core != TS_CORE_NORMAL)
		return count;

	if (sscanf(buf, "%d", &value) <= 0)
		return count;

	t_dev_info(dev, "%s: value %d\n", __func__, value);

	mutex_lock(&ts->lock);

	value_swt = value % 10;
	value_ext = value / 10;

	switch (value_ext) {
	case 1:
		/*
		 * When it needs to control 'irq_state' with safe condition.
		 * The value '1x' means that the command is allowed
		 * only when the current touch state is 'resume'.
		 * Ex) write irq_state 10 : disable irq
		 *                          only when the current touch state is 'resume'
		 */
		if(ts->state_mode == TS_MODE_SUSPEND) {
			t_dev_info(ts->dev, "%s: state(%d) blocked in suspend\n",
				__func__, value);
			goto out;
		}
		break;
	default:
		if (value_ext)
			goto out;
		break;
	}
	value = value_swt;

	lxs_ts_irq_control(ts, !!(value));

out:
	mutex_unlock(&ts->lock);

	return (ssize_t)count;
}

static ssize_t show_rst_level(struct device *dev, char *buf)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);

	if (ts->plat_data->use_skip_reset)
		return lxs_snprintf(buf, 0, "NA\n");

	return lxs_snprintf(buf, 0, "%d\n", gpio_get_value(ts->plat_data->reset_pin));
}

static ssize_t show_module_info(struct device *dev, char *buf)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);
	int size = 0;

	size += lxs_snprintf(buf, size, "%s/%s/%s, %s\n",
			dev_name(dev->parent->parent),
			dev_name(dev->parent),
			dev_name(dev),
			dev_name(&ts->input->dev));

	return (ssize_t)size;
}

static ssize_t show_dbg_state(struct device *dev, char *buf)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);

	return lxs_snprintf(buf, 0, "%d(TS_MODE_%s)\n", ts->state_mode,
		(ts->state_mode) ? "SUSPEND" : "RESUME");
}

static ssize_t show_dbg_driving(struct device *dev, char *buf)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);
	struct lxs_ts_chip *chip = &ts->chip;

	return lxs_snprintf(buf, 0, "driving_mode %d\n", chip->driving_mode);
}

static ssize_t store_dbg_driving(struct device *dev,
				const char *buf, size_t count)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);
	struct lxs_ts_chip *chip = &ts->chip;
	int value = 0;

	if (sscanf(buf, "%d", &value) <= 0)
		return count;

	t_dev_info(dev, "%s: value %d\n", __func__, value);

	switch (value) {
	case LCD_MODE_U0:
	case LCD_MODE_U3:
	case LCD_MODE_STOP:
		if (atomic_read(&chip->init) != TC_IC_INIT_DONE)
			break;
		lxs_hal_tc_driving(ts, value);
		break;
	}

	return count;
}

static ssize_t show_dbg_mask(struct device *dev, char *buf)
{
//	struct lxs_ts *ts = dev_get_drvdata(dev);
	int size = 0;

	size += lxs_snprintf(buf, size,
				"t_dev_dbg_mask %08Xh\n", t_dev_dbg_mask);
	size += lxs_snprintf(buf, size,
				"t_pr_dbg_mask  %08Xh\n", t_pr_dbg_mask);
	size += lxs_snprintf(buf, size,
				"t_bus_dbg_mask %08Xh\n", t_bus_dbg_mask);

	size += lxs_snprintf(buf, size,
				"Usage:\n");
	size += lxs_snprintf(buf, size,
				" t_dev_dbg_mask : echo 0 {mask_value} > dbg_mask\n");
	size += lxs_snprintf(buf, size,
				" t_pr_dbg_mask  : echo 1 {mask_value} > dbg_mask\n");
	size += lxs_snprintf(buf, size,
				" t_bus_dbg_mask : echo 3 {mask_value} > dbg_mask\n");

	return (ssize_t)size;
}

static ssize_t store_dbg_mask(struct device *dev,
				const char *buf, size_t count)
{
//	struct lxs_ts *ts = dev_get_drvdata(dev);
	int type = 0;
	u32 old_value, new_value = 0;

	if (sscanf(buf, "%d %X", &type, &new_value) <= 0)
		return count;

	switch (type) {
	case 0 :
		old_value = t_dev_dbg_mask;
		t_dev_dbg_mask = new_value;
		t_dev_info(dev, "t_dev_dbg_mask changed : %08Xh -> %08xh\n",
			old_value, new_value);
		break;
	case 1 :
		old_value = t_pr_dbg_mask;
		t_pr_dbg_mask = new_value;
		t_dev_info(dev, "t_pr_dbg_mask changed : %08Xh -> %08xh\n",
			old_value, new_value);
		break;
	case 2 :
		/* TBD */
		break;
	case 3 :
		old_value = t_bus_dbg_mask;
		t_bus_dbg_mask = new_value;
		t_dev_info(dev, "t_bus_dbg_mask changed : %08Xh -> %08xh\n",
			old_value, new_value);
		break;
	default :
		t_dev_info(dev, "Usage:\n");
		t_dev_info(dev, " t_dev_dbg_mask : echo 0 {mask_value(hex)} > dbg_mask\n");
		t_dev_info(dev, " t_pr_dbg_mask  : echo 1 {mask_value(hex)} > dbg_mask\n");
		t_dev_info(dev, " t_bus_dbg_mask : echo 3 {mask_value(hex)} > dbg_mask\n");
		break;
	}

	return count;
}

static ssize_t show_dbg_mode_async(struct device *dev, char *buf)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);

	t_dev_info(ts->dev, "mode_async %d\n", ts->mode_async);

	return lxs_snprintf(buf, 0, "%d\n", ts->mode_async);
}

static ssize_t store_dbg_mode_async(struct device *dev,
				const char *buf, size_t count)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);
	int value;

	if (ts->state_core != TS_CORE_NORMAL)
		return count;

	if (sscanf(buf, "%d", &value) <= 0)
		return count;

	t_dev_info(dev, "%s: value %d\n", __func__, value);

	mutex_lock(&ts->lock);
	ts->mode_async = value;
	mutex_unlock(&ts->lock);

	return count;
}

#if defined(__LXS_SUPPORT_WATCHDOG)
static ssize_t show_dbg_watchdog(struct device *dev, char *buf)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);

	t_dev_info(ts->dev, "watchdog: set %d, run %d, time %d\n",
		ts->watchdog_set, ts->watchdog_run, ts->watchdog_time);

	return lxs_snprintf(buf, 0, "set %d, run %d, time %d\n",
		ts->watchdog_set, ts->watchdog_run, ts->watchdog_time);
}

static ssize_t store_dbg_watchdog(struct device *dev,
				const char *buf, size_t count)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);
	int value;

	if (ts->state_core != TS_CORE_NORMAL)
		return count;

	if (sscanf(buf, "%d", &value) <= 0)
		return count;

	if (!ts->watchdog_set)
		return count;

	t_dev_info(dev, "%s: value %d\n", __func__, value);

	if (value)
		lxs_ts_watchdog_run(ts);
	else
		lxs_ts_watchdog_stop(ts);

	return count;
}

static ssize_t store_dbg_watchdog_time(struct device *dev,
				const char *buf, size_t count)
{
	struct lxs_ts *ts = dev_get_drvdata(dev);
	int value;

	if (ts->state_core != TS_CORE_NORMAL)
		return count;

	if (sscanf(buf, "%d", &value) <= 0)
		return count;

	if (!ts->watchdog_set)
		return count;

	t_dev_info(dev, "%s: value %d\n", __func__, value);

	lxs_ts_watchdog_time(ts, value);

	return count;
}
#endif

static TS_ATTR(platform_data, show_plat_data, NULL);
static TS_ATTR(driver_data, show_driver_data, NULL);
static TS_ATTR(fw_upgrade, show_fw_upgrade, store_fw_upgrade);
static TS_ATTR(irq_flag, show_irq_flag, NULL);
static TS_ATTR(irq_level, show_irq_level, NULL);
static TS_ATTR(irq_state, show_irq_state, store_irq_state);
static TS_ATTR(rst_level, show_rst_level, NULL);
static TS_ATTR(module_info, show_module_info, NULL);
static TS_ATTR(dbg_state, show_dbg_state, NULL);
static TS_ATTR(dbg_driving, show_dbg_driving, store_dbg_driving);
static TS_ATTR(dbg_mask, show_dbg_mask, store_dbg_mask);
static TS_ATTR(dbg_mode_async, show_dbg_mode_async, store_dbg_mode_async);
#if defined(__LXS_SUPPORT_WATCHDOG)
static TS_ATTR(dbg_watchdog, show_dbg_watchdog, store_dbg_watchdog);
static TS_ATTR(dbg_watchdog_time, NULL, store_dbg_watchdog_time);
#endif

static TS_ATTR(reg_list, show_reg_list, NULL);
static TS_ATTR(reg_ctrl, show_reg_ctrl, store_reg_ctrl);
static TS_ATTR(reset_ctrl, show_reset_ctrl, store_reset_ctrl);
static TS_ATTR(debug_tc_cmd, show_debug_tc_cmd, store_debug_tc_cmd);
static TS_ATTR(debug_power, show_debug_power, store_debug_power);
static TS_ATTR(fwup_status, show_fwup_status, NULL);
static TS_ATTR(version, show_version, NULL);
static TS_ATTR(version_no, show_version_no, NULL);
static TS_ATTR(lpwg_data, show_lpwg_data, NULL);
static TS_ATTR(channel_info, show_channel_info, NULL);
/* */
static TS_ATTR(glove_status, show_glove_state, store_glove_state);
static TS_ATTR(grab_status, show_grab_state, store_grab_state);
static TS_ATTR(usb_status, show_usb_state, store_usb_state);
static TS_ATTR(wirelss_status, show_wireless_state, store_wireless_state);
static TS_ATTR(ime_status, show_ime_state, store_ime_state);
static TS_ATTR(incoming_call, show_incoming_call, store_incoming_call);
static TS_ATTR(frame_table, show_frame_table, NULL);
static TS_ATTR(frame_rate_np, show_frame_rate_np, store_frame_rate_np);
static TS_ATTR(frame_rate_lp, show_frame_rate_lp, store_frame_rate_lp);
/* */
static TS_ATTR(enable, show_enable, store_enable);
#if defined(__LXS_SUPPORT_LPWG)
static TS_ATTR(set_lowpower_mode, show_enable_lpm, store_enable_lpm);
static TS_ATTR(enable_knock, show_enable_knock, store_enable_knock);
#endif
static TS_ATTR(enable_sleep, show_enable_sleep, store_enable_sleep);
static TS_ATTR(enable_prox, show_enable_prox, store_enable_prox);
static TS_ATTR(after_probe, NULL, after_probe_start_store);
static TS_ATTR(range_changer, show_range_changer, store_range_changer);
static TS_ATTR(orientation_change, show_orientation_change, store_orientation_change);
static TS_ATTR(rejection_mode, show_rejection_mode, store_rejection_mode);

#if defined(__LXS_SUPPORT_FILTER_CON)
static TS_ATTR(blend_filter, show_blend_filter, store_blend_filter);
static TS_ATTR(stop_filter, show_stop_filter, store_stop_filter);
static TS_ATTR(wet_mode, show_wet_mode, NULL);
#endif

static struct attribute *lxs_ts_attr_default[] = {
	&ts_attr_platform_data.attr,
	&ts_attr_driver_data.attr,
	&ts_attr_module_info.attr,
	&ts_attr_dbg_state.attr,
	&ts_attr_dbg_driving.attr,
	&ts_attr_dbg_mask.attr,
	&ts_attr_dbg_mode_async.attr,
#if defined(__LXS_SUPPORT_WATCHDOG)
	&ts_attr_dbg_watchdog.attr,
	&ts_attr_dbg_watchdog_time.attr,
#endif
	/* */
	&ts_attr_fw_upgrade.attr,
	&ts_attr_irq_flag.attr,
	&ts_attr_irq_level.attr,
	&ts_attr_irq_state.attr,
	&ts_attr_rst_level.attr,
	/* */
	&ts_attr_reg_list.attr,
	&ts_attr_reg_ctrl.attr,
	&ts_attr_reset_ctrl.attr,
	&ts_attr_debug_tc_cmd.attr,
	&ts_attr_debug_power.attr,
	&ts_attr_fwup_status.attr,
	&ts_attr_version.attr,
	&ts_attr_version_no.attr,
	&ts_attr_lpwg_data.attr,
	&ts_attr_channel_info.attr,
	/* */
	&ts_attr_glove_status.attr,
	&ts_attr_grab_status.attr,
	&ts_attr_usb_status.attr,
	&ts_attr_wirelss_status.attr,
	&ts_attr_ime_status.attr,
	&ts_attr_incoming_call.attr,
	&ts_attr_frame_table.attr,
	&ts_attr_frame_rate_lp.attr,
	&ts_attr_frame_rate_np.attr,
	/* */
	&ts_attr_enable.attr,
#if defined(__LXS_SUPPORT_LPWG)
	&ts_attr_set_lowpower_mode.attr,
	&ts_attr_enable_knock.attr,
#endif
	&ts_attr_enable_sleep.attr,
	&ts_attr_enable_prox.attr,
	&ts_attr_after_probe.attr,
	&ts_attr_range_changer.attr,
	&ts_attr_orientation_change.attr,
	&ts_attr_rejection_mode.attr,
	/* */
#if defined(__LXS_SUPPORT_FILTER_CON)
	&ts_attr_blend_filter.attr,
	&ts_attr_stop_filter.attr,
	&ts_attr_wet_mode.attr,
#endif
	/* */
	NULL,
};

static ssize_t lxs_ts_attr_show(struct kobject *kobj,
		struct attribute *attr, char *buf)
{
	struct lxs_ts *ts =
		container_of(kobj, struct lxs_ts, kobj);
	struct lxs_ts_attribute *priv =
		container_of(attr, struct lxs_ts_attribute, attr);
	ssize_t ret = 0;

	if (priv->show)
		ret = priv->show(ts->dev, buf);

	return ret;
}

static ssize_t lxs_ts_attr_store(struct kobject *kobj,
		struct attribute *attr, const char *buf, size_t count)
{
	struct lxs_ts *ts =
		container_of(kobj, struct lxs_ts, kobj);
	struct lxs_ts_attribute *priv =
		container_of(attr, struct lxs_ts_attribute, attr);
	ssize_t ret = count;

	if (priv->store)
		ret = priv->store(ts->dev, buf, count);

	return ret;
}

/*
 * To reegister LXS's unique sysfs functions
 */
static const struct sysfs_ops lxs_ts_sysfs_ops = {
	.show	= lxs_ts_attr_show,
	.store	= lxs_ts_attr_store,
};

static struct kobj_type lxs_ts_kobj_type = {
	.sysfs_ops		= &lxs_ts_sysfs_ops,
	.default_attrs	= lxs_ts_attr_default,
};

int lxs_ts_init_sysfs(struct lxs_ts *ts)
{
	struct device *idev = &ts->input->dev;
	struct kobject *kobj = &ts->kobj;
	int ret = 0;

	ret = kobject_init_and_add(kobj, &lxs_ts_kobj_type,
			idev->kobj.parent, "%s", ts->input_name);
	if (ret < 0) {
		t_dev_err(ts->dev, "failed to create sysfs entry\n");
		goto out;
	}

	if (ts->fquirks->sysfs_group != NULL) {
		ret = sysfs_create_group(kobj, ts->fquirks->sysfs_group);
		if (ret < 0) {
			t_dev_err(ts->dev, "failed to add fquirks sysfs\n");
			goto out_sysfs_grp;
		}
	}

	t_dev_info(ts->dev, "sysfs initialized\n");

	return 0;

out_sysfs_grp:
	kobject_del(&ts->kobj);
	kobject_put(&ts->kobj);

out:
	return ret;
}

void lxs_ts_free_sysfs(struct lxs_ts *ts)
{
	struct device *idev = &ts->input->dev;
	struct kobject *kobj = &ts->kobj;

	if (kobj->parent != idev->kobj.parent) {
		t_dev_warn(ts->dev, "Invalid kobject\n");
		return;
	}

	if (ts->fquirks->sysfs_group != NULL)
		sysfs_remove_group(kobj, ts->fquirks->sysfs_group);

	kobject_del(kobj);
	kobject_put(kobj);
}
