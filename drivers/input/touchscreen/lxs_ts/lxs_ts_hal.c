/*
 * SPDX-License-Identifier: GPL-2.0
 *
 * lxs_ts_hal.c
 *
 * LXS touch hal layer
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "lxs_ts.h"

int __weak lxs_hal_prd_sysfs(struct lxs_ts *ts, int on_off)

{
	t_dev_info_once(ts->dev, "PRD disabled\n");
	return 0;
}

#define __LAYER_REG_SETUP__

static const struct lxs_hal_reg lxs_hal_reg_table = {
	.info_ptr				= INFO_PTR,
	.spr_chip_test			= SPR_CHIP_TEST,
	.spr_chip_id			= SPR_CHIP_ID,
	.spr_rst_ctl			= SPR_RST_CTL,
	.spr_boot_ctl			= SPR_BOOT_CTL,
	.spr_sram_ctl			= SPR_SRAM_CTL,
	.spr_boot_status		= SPR_BOOT_STS,
	.spr_code_offset		= SPR_CODE_OFFSET,
	.tc_ic_status			= TC_IC_STATUS,
	.tc_status				= TC_FW_STATUS,
	.tc_version				= TC_VERSION,
	.tc_product_id1			= TC_PRODUCT_ID1,
	.tc_product_id2			= TC_PRODUCT_ID2,
	.info_fpc_type			= INFO_FPC_TYPE,
	.info_wfr_type			= INFO_WFR_TYPE,
	.info_chip_version		= INFO_CHIP_VERSION,
	.info_cg_type			= INFO_CG_TYPE,
	.info_lot_num			= INFO_LOT_NUM,
	.info_serial_num		= INFO_SERIAL_NUM,
	.info_date				= INFO_DATE,
	.info_time				= INFO_TIME,
	.code_access_addr		= CODE_ACCESS_ADDR,
	.data_base_addr			= DATA_BASE_ADDR,
	.prd_tcm_base_addr		= PRD_TCM_BASE_ADDR,
	.tc_device_ctl			= TC_DEVICE_CTL,
	.tc_drive_ctl			= TC_DRIVE_CTL,
	.tci_debug_fail_ctrl	= TCI_DEBUG_FAIL_CTRL,
	.tci_debug_fail_buffer	= TCI_DEBUG_FAIL_BUFFER,
	.tci_debug_fail_status	= TCI_DEBUG_FAIL_STATUS,
	.cmd_raw_data_report_mode_read	= CMD_RAW_DATA_REPORT_MODE_READ,
	.cmd_raw_data_compress_write	= CMD_RAW_DATA_COMPRESS_WRITE,
	.cmd_raw_data_report_mode_write	= CMD_RAW_DATA_REPORT_MODE_WRITE,
	.res_info				= RES_INFO,
	.channel_info			= CHANNEL_INFO,
	.t_frame_rate			= T_FRAME_RATE,
	.charger_info			= CHARGER_INFO,
	.frame_rate				= FRAME_RATE,
	.gaming_mode			= GAMING_MODE,
	.glove_mode				= GLOVE_MODE,
	.ime_state				= IME_STATE,
	.call_state				= CALL_STATE,
	.grab_mode				= GRAB_MODE,
	.dim_sys				= DIM_SYS,
	.dim_eng				= DIM_ENG,
	.tc_tsp_test_ctl		= TC_TSP_TEST_CTL,
	.tc_tsp_test_status		= TC_TSP_TEST_STS,
	.tc_tsp_test_pf_result	= TC_TSP_TEST_PF_RESULT,
	.tc_tsp_test_off_info	= TC_TSP_TEST_OFF_INFO,
	.tc_flash_dn_status		= TC_FLASH_DN_STS,
	.tc_confdn_base_addr	= TC_CONFDN_BASE_ADDR,
	.tc_flash_dn_ctl		= TC_FLASH_DN_CTL,
	.raw_data_ctl_read		= RAW_DATA_CTL_READ,
	.raw_data_ctl_write		= RAW_DATA_CTL_WRITE,
	.serial_data_offset		= SERIAL_DATA_OFFSET,
	/* */
	.prd_serial_tcm_offset	= PRD_SERIAL_TCM_OFFSET,
	.prd_tc_mem_sel			= PRD_TC_MEM_SEL,
	.prd_tc_test_mode_ctl	= PRD_TC_TEST_MODE_CTL,
	.prd_m1_m2_raw_offset	= PRD_M1_M2_RAW_OFFSET,
	.prd_tune_result_offset	= PRD_TUNE_RESULT_OFFSET,
	.prd_open3_short_offset	= PRD_OPEN3_SHORT_OFFSET,
	.prd_ic_ait_start_reg	= PRD_IC_AIT_START_REG,
	.prd_ic_ait_data_readystatus = PRD_IC_AIT_DATA_READYSTATUS,
};

static void lxs_hal_reg_setup(struct lxs_ts *ts)
{
	struct lxs_ts_chip *chip = &ts->chip;
	struct lxs_hal_reg *reg_def = (struct lxs_hal_reg *)&lxs_hal_reg_table;
	struct lxs_ts_reg_quirk *reg_quirks = ts->reg_quirks;
	u32 *curr_reg;
	u32 *copy_reg;
	int cnt = sizeof(struct lxs_hal_reg)>>2;
	u32 new_addr, old_addr;
	int total = 0;
	int missing = 0;
	int show_log = 1;
	int found = 0;
	int i;

	memcpy(chip->reg, reg_def, sizeof(struct lxs_hal_reg));

	if (reg_quirks == NULL)
		return;

	while (1) {
		old_addr = reg_quirks->old_addr;
		new_addr = reg_quirks->new_addr;

		if (old_addr == (1<<31)) {
			t_dev_info(ts->dev, "%s reg quirks: ...\n", ts->chip_name);
			show_log = 0;
			reg_quirks++;
			continue;
		}

		if ((old_addr == ~0) || (new_addr == ~0))
			break;

		found = 0;
		copy_reg = (u32 *)reg_def;
		curr_reg = (u32 *)chip->reg;
		for (i = 0; i < cnt; i++) {
			if ((*copy_reg) == old_addr) {
				(*curr_reg) = new_addr;
				found = 1;
				break;
			}
			copy_reg++;
			curr_reg++;
		}
		if (found) {
			if (show_log)
				t_dev_info(ts->dev, "%s reg quirks: [%d] %04Xh -> %04Xh\n",
					ts->chip_name, total,
					old_addr, new_addr);
		} else {
			t_dev_warn(ts->dev, "%s reg quirks: [%d] %04Xh not found\n",
				ts->chip_name, total,
				old_addr);
			missing++;
		}
		total++;

		reg_quirks++;
	}
	t_dev_info(ts->dev, "%s reg quirks: t %d, m %d\n",
		ts->chip_name, total, missing);
}

#define __LAYER_ADD_SYSFS__

static int lxs_hal_add_sysfs(struct lxs_ts *ts)
{
	int ret;

	ret = lxs_hal_prd_sysfs(ts, 1);
	if (ret < 0)
		t_dev_err(ts->dev, "LXS(%s) prd sysfs register failed\n", ts->chip_name);

	return 0;
}

static void lxs_hal_del_sysfs(struct lxs_ts *ts)
{
	lxs_hal_prd_sysfs(ts, 0);
}

#define __LAYER_BUS_ACCESS__

int lxs_hal_reg_read(struct lxs_ts *ts, u32 addr, void *data, int size)
{
	struct lxs_ts_chip *chip = &ts->chip;
	struct lxs_hal_reg *reg = chip->reg;

	if (!size) {
		t_bus_dbg_base(ts->dev, "rd: size zero\n");
		return 0;
	}

	if (lxs_addr_is_skip(addr)) {
		t_bus_dbg_base(ts->dev, "rd: skip by ADDR_SKIP_MASK\n");
		return 0;
	}

	if (!data) {
		t_bus_err(ts->dev, "rd: NULL data(0x%04X, 0x%04X)\n", addr, size);
		return -EFAULT;
	}

	switch (ts->chip.opt.t_bus_opt) {
	case 1:
		if (addr == reg->tc_ic_status)
			addr += !!(size > 4);
		else
			addr += !!(addr == reg->tc_status);
		break;
	}

	return lxs_ts_bus_read(ts, addr, data, size);
}

int lxs_hal_reg_write(struct lxs_ts *ts, u32 addr, void *data, int size)
{
	if (!size) {
		t_bus_dbg_base(ts->dev, "wr: size zero\n");
		return 0;
	}

	if (lxs_addr_is_skip(addr)) {
		t_bus_dbg_base(ts->dev, "wr: skip by ADDR_SKIP_MASK\n");
		return 0;
	}

	if (!data) {
		t_bus_err(ts->dev, "wr: NULL data(0x%04X, 0x%04X)\n", addr, size);
		return -EFAULT;
	}

	return lxs_ts_bus_write(ts, addr, data, size);
}

int lxs_hal_read_value(struct lxs_ts *ts, u32 addr, u32 *value)
{
	return lxs_hal_reg_read(ts, addr, value, sizeof(u32));
}

int lxs_hal_write_value(struct lxs_ts *ts, u32 addr, u32 value)
{
	return lxs_hal_reg_write(ts, addr, &value, sizeof(u32));
}

int lxs_hal_reg_read_single(struct lxs_ts *ts, u32 addr, void *data, int size)
{
	u32 *__data = (u32 *)data;
	int __size;
	int ret = 0;

	while (size) {
		__size = min(4, size);
		ret = lxs_hal_reg_read(ts, addr, __data, __size);
		if (ret < 0)
			break;

		addr++;
		__data++;
		size -= __size;
	}

	return ret;
}

int lxs_hal_reg_write_single(struct lxs_ts *ts, u32 addr, void *data, int size)
{
	u32 *__data = (u32 *)data;
	int __size;
	int ret = 0;

	while (size) {
		__size = min(4, size);
		ret = lxs_hal_reg_write(ts, addr, __data, __size);
		if (ret < 0)
			break;

		addr++;
		__data++;
		size -= __size;
	}

	return ret;
}

int lxs_hal_reg_rw_multi(struct lxs_ts *ts,
		struct lxs_hal_rw_multi *multi, char *title)
{
	int (*func)(struct lxs_ts *ts, u32 addr, void *data, int size);
	int ret = 0;

	while (1) {
		if ((multi->wr == -1) ||
			(multi->addr == -1) ||
			(multi->data == NULL))
			break;

		func = (multi->wr) ? lxs_hal_reg_write : lxs_hal_reg_read;

		ret = func(ts, multi->addr, multi->data, multi->size);
		if (ret < 0) {
			t_dev_err(ts->dev, "%s: %s %s failed, %d\n",
				title, (multi->name) ? multi->name : "",
				(multi->wr) ? "write" : "read",
				ret);
			break;
		}

		multi++;
	}

	return ret;
}

static int __lxs_hal_reg_bit_mask(struct lxs_ts *ts, u32 addr, u32 *value, u32 mask, int set)
{
	const char *str = (set) ? "set" : "clr";
	u32 rdata = 0;
	u32 data = 0;
	int ret = 0;

	if (set == 2) {
		if (value == NULL)
			return 0;
		str = "chg";
	}

	ret = lxs_hal_read_value(ts, addr, &data);
	if (ret < 0) {
		t_dev_err(ts->dev, "bit %s failed, addr 0x%04X, mask 0x%08X\n",
			str, addr, mask);
		return ret;
	}

	rdata = data;

	switch (set) {
	case 2:
		data &= ~mask;
		data |= *value;
		break;
	case 1:
		data |= mask;
		break;
	default:
		data &= ~mask;
		break;
	}

	ret = lxs_hal_write_value(ts, addr, data);
	if (ret < 0) {
		t_dev_err(ts->dev, "bit %s failed, addr 0x%04X, mask 0x%08X (0x%08X <- 0x%08X)\n",
			str, addr, mask, data, rdata);
		return ret;
	}

#if 0
	t_dev_info(ts->dev, "bit %s done, addr 0x%04X, mask 0x%08X (0x%08X <- 0x%08X)\n",
		str, addr, mask, data, rdata);
#endif

	if (value)
		*value = data;

	return 0;
}

int lxs_hal_reg_bit_chg(struct lxs_ts *ts, u32 addr, u32 *value, u32 mask)
{
	return __lxs_hal_reg_bit_mask(ts, addr, value, mask, 2);
}

int lxs_hal_reg_bit_set(struct lxs_ts *ts, u32 addr, u32 *value, u32 mask)
{
	return __lxs_hal_reg_bit_mask(ts, addr, value, mask, 1);
}

int lxs_hal_reg_bit_clr(struct lxs_ts *ts, u32 addr, u32 *value, u32 mask)
{
	return __lxs_hal_reg_bit_mask(ts, addr, value, mask, 0);
}

#define __LAYER_HAL__

int lxs_hal_access_not_allowed(struct lxs_ts *ts, char *title, int skip_flag)
{
	struct lxs_ts_chip *chip = &ts->chip;
	char *sub = "";

	if (!(skip_flag & HAL_ACCESS_CHK_SKIP_SLEEP))
		if (ts->state_sleep != TS_IC_NORMAL) {
			sub = "not TS_IC_NORMAL";
			goto out;
		}

	if (!(skip_flag & HAL_ACCESS_CHK_SKIP_FB))
		if (ts->state_mode != TS_MODE_RESUME) {
			sub = "not TS_MODE_RESUME";
			goto out;
		}

	if (!(skip_flag & HAL_ACCESS_CHK_SKIP_PM))
		if (ts->state_pm != TS_DEV_PM_RESUME) {
			sub = "not TS_DEV_PM_RESUME";
			goto out;
		}

	if (!(skip_flag & HAL_ACCESS_CHK_SKIP_INIT))
		if (atomic_read(&chip->init) != TC_IC_INIT_DONE) {
			sub = "not TC_IC_INIT_DONE";
			goto out;
		}

	return 0;

out:
	if (title)
		t_dev_warn(ts->dev, "%s: %s\n", title, sub);

	return 1;
}

static int lxs_hal_tc_not_allowed(struct lxs_ts *ts, char *title)
{
	int skip_flag = HAL_ACCESS_CHK_SKIP_PM | HAL_ACCESS_CHK_SKIP_FB;

	return lxs_hal_access_not_allowed(ts, title, skip_flag);
}

int lxs_hal_get_boot_result(struct lxs_ts *ts, u32 *boot_st)
{
	struct lxs_ts_chip *chip = &ts->chip;
//	struct lxs_ts *ts = chip->ts;
	struct lxs_hal_ops_quirk *ops_quirk = &chip->ops_quirk;
	struct lxs_hal_reg *reg = chip->reg;
	u32 bootmode = 0;
	int ret = 0;

	if (ops_quirk->boot_result) {
		ret = ops_quirk->boot_result(ts, boot_st);
		return ret;
	}

	ret = lxs_hal_read_value(ts, reg->spr_boot_status, &bootmode);
	if (ret < 0)
		return ret;

	if (boot_st)
		*boot_st = bootmode;

	return 0;
}

static int lxs_hal_chk_boot_result(struct lxs_ts *ts)
{
	u32 boot_failed = 0;
	u32 bootmode = 0;
	u32 boot_chk_offset_busy = lxs_hal_boot_sts_pos_busy(ts);
	u32 boot_chk_offset_err = lxs_hal_boot_sts_pos_dump_err(ts);
	u32 boot_chk_empty_mask = lxs_hal_boot_sts_mask_empty(ts);
	int ret = 0;

	ret = lxs_hal_get_boot_result(ts, &bootmode);
	if (ret < 0)
		return ret;

	/* maybe nReset is low state */
	if (!bootmode || (bootmode == ~0))
		return BOOT_CHK_SKIP;

	/* booting... need to wait */
	if ((bootmode >> boot_chk_offset_busy) & 0x1)
		return BOOT_CHK_SKIP;

	boot_failed |= !!((bootmode >> boot_chk_offset_err) & 0x1);	/* CRC error */
	boot_failed |= (!!(bootmode & boot_chk_empty_mask))<<1;

	if (boot_failed)
		t_dev_err(ts->dev, "boot fail: boot sts = %08Xh(%02Xh)\n",
			bootmode, boot_failed);

	return boot_failed;
}

static int lxs_hal_chk_boot_status(struct lxs_ts *ts)
{
	struct lxs_ts_chip *chip = &ts->chip;
	struct lxs_hal_reg *reg = chip->reg;
	struct lxs_hal_status_mask_bit *mask_bit = &chip->status_mask_bit;
	u32 boot_failed = 0;
	u32 tc_status = 0;
	u32 valid_cfg_crc_mask = 0;
	u32 valid_code_crc_mask = 0;
	int ret = 0;

	valid_cfg_crc_mask = mask_bit->valid_cfg_crc;
	valid_code_crc_mask = mask_bit->valid_code_crc;

	ret = lxs_hal_read_value(ts, reg->tc_status, &tc_status);
	if (ret < 0)
		return ret;

	/* maybe nReset is low state */
	if (!tc_status || (tc_status == ~0))
		return BOOT_CHK_SKIP;

	if (valid_cfg_crc_mask && !(tc_status & valid_cfg_crc_mask))
		boot_failed |= BIT(5);

	if (valid_code_crc_mask && !(tc_status & valid_code_crc_mask))
		boot_failed |= BIT(4);

	if (boot_failed)
		t_dev_err(ts->dev, "boot fail: tc_status = %08Xh(%02Xh)\n",
			tc_status, boot_failed);

	return boot_failed;
}

static int lxs_hal_chk_boot(struct lxs_ts *ts)
{
	u32 boot_failed = 0;
	int retry;
	int ret = 0;

	retry = BOOT_CHK_MODE_RETRY;
	while (retry--) {
		ret = lxs_hal_chk_boot_result(ts);
		if (ret < 0)
			return ret;

		if (ret == BOOT_CHK_SKIP)
			return 0;

		if (!ret)
			break;

		lxs_ts_delay(BOOT_CHK_MODE_DELAY);
	}
	boot_failed |= ret;

	retry = BOOT_CHK_STS_RETRY;
	while (retry--) {
		ret = lxs_hal_chk_boot_status(ts);
		if (ret < 0)
			return ret;

		if (ret == BOOT_CHK_SKIP)
			return boot_failed;

		if (!ret)
			break;

		lxs_ts_delay(BOOT_CHK_STS_DELAY);
	}
	boot_failed |= ret;

	return boot_failed;
}

static const struct lxs_hal_status_filter status_filter_type_0[] = {
	_STS_FILTER(STS_ID_VALID_DEV_CTL, 1, STS_POS_VALID_DEV_CTL,
		0, "device ctl not set"),
	_STS_FILTER(STS_ID_VALID_CODE_CRC, 1, STS_POS_VALID_CODE_CRC_TYPE_0,
		0, "code crc invalid"),
	_STS_FILTER(STS_ID_ERROR_ABNORMAL, 1, STS_POS_ERROR_ABNORMAL,
		STS_FILTER_FLAG_TYPE_ERROR | STS_FILTER_FLAG_CHK_FAULT,
		"abnormal status detected"),
	_STS_FILTER(STS_ID_ERROR_SYSTEM, 1, STS_POS_ERROR_SYSTEM,
		STS_FILTER_FLAG_TYPE_ERROR | STS_FILTER_FLAG_ESD_SEND,
		"system error detected"),
	_STS_FILTER(STS_ID_ERROR_MISMTACH, 2, STS_POS_ERROR_MISMTACH,
		STS_FILTER_FLAG_TYPE_ERROR,
		"display mode mismatch"),
	_STS_FILTER(STS_ID_VALID_IRQ_PIN, 1, STS_POS_VALID_IRQ_PIN,
		0, "irq pin invalid"),
	_STS_FILTER(STS_ID_VALID_IRQ_EN, 1, STS_POS_VALID_IRQ_EN,
		0, "irq status invalid"),
	/* end mask */
	_STS_FILTER(STS_ID_NONE, 0, 0, 0, NULL),
};

static const struct lxs_hal_status_filter status_filter_type_1[] = {
	_STS_FILTER(STS_ID_VALID_DEV_CTL, 1, STS_POS_VALID_DEV_CTL,
		0, "device ctl not set"),
	_STS_FILTER(STS_ID_VALID_CODE_CRC, 1, STS_POS_VALID_CODE_CRC,
		0, "code crc invalid"),
#if defined(__LXS_SUPPORT_STATUS_ERROR_CFG)
	_STS_FILTER(STS_ID_VALID_CFG_CRC, 1, STS_POS_VALID_CFG_CRC,
		0, "cfg crc invalid"),
#endif
	_STS_FILTER(STS_ID_ERROR_ABNORMAL, 1, STS_POS_ERROR_ABNORMAL,
		STS_FILTER_FLAG_TYPE_ERROR | STS_FILTER_FLAG_CHK_FAULT,
		"abnormal status detected"),
	_STS_FILTER(STS_ID_ERROR_SYSTEM, 1, STS_POS_ERROR_SYSTEM,
		STS_FILTER_FLAG_TYPE_ERROR | STS_FILTER_FLAG_ESD_SEND,
		"system error detected"),
	_STS_FILTER(STS_ID_ERROR_MISMTACH, 1, STS_POS_ERROR_MISMTACH,
		STS_FILTER_FLAG_TYPE_ERROR,
		"display mode mismatch"),
	_STS_FILTER(STS_ID_VALID_IRQ_PIN, 1, STS_POS_VALID_IRQ_PIN,
		0, "irq pin invalid"),
	_STS_FILTER(STS_ID_VALID_IRQ_EN, 1, STS_POS_VALID_IRQ_EN,
		0, "irq status invalid"),
	_STS_FILTER(STS_ID_VALID_TC_DRV, 1, STS_POS_VALID_TC_DRV,
		0, "driving invalid"),
	/* end mask */
	_STS_FILTER(STS_ID_NONE, 0, 0, 0, NULL),
};

static u32 lxs_hal_get_status_mask(struct lxs_ts *ts, int id)
{
	struct lxs_ts_chip *chip = &ts->chip;
	struct lxs_hal_status_filter *filter = chip->status_filter;
	u32 mask = 0;

	if (filter == NULL)
		goto out;

	while (1) {
		if (!filter->id || !filter->width)
			break;

		if (filter->id == id) {
			mask = ((1<<filter->width)-1)<<filter->pos;
			break;
		}
		filter++;
	}

out:
	return mask;
}

static int __irq_abs_palm_chk(struct lxs_ts *ts, int track_id, int event)
{
	if (ts->plat_data->use_palm_opt)
		return 0;

	if (track_id != PALM_ID)
		return 0;

	if (event == TOUCHSTS_DOWN) {
		ts->intr_palm |= BIT(16);

		ts->is_cancel = 1;
		t_dev_info(ts->dev, "Palm Detected\n");
	} else if (event == TOUCHSTS_UP) {
		ts->is_cancel = 0;
		t_dev_info(ts->dev, "Palm Released\n");
	}

	ts->tcount = 0;
	ts->intr_status |= TS_IRQ_FINGER;

	return TC_PALM_DETECTED;
}

static int __irq_abs_data_default(struct lxs_ts *ts)
{
	struct lxs_hal_touch_info *info = lxs_report_info(ts);
	struct lxs_hal_touch_data *data = lxs_report_data(ts);
	struct touch_data *tdata = NULL;
	u32 palm_bit = (ts->plat_data->use_palm_opt) ? info->palm_bit : 0;
	u32 touch_count = info->touch_cnt;
	int finger_index = 0;
	int i = 0;
	int ret = 0;

	ts->intr_status &= ~TS_IRQ_FINGER;

	ts->new_mask = 0;

	if (!touch_count)
		goto out;

	/* check if palm detected */
	ret = __irq_abs_palm_chk(ts, data->track_id, data->event);
	if (ret == TC_PALM_DETECTED)
		goto out;

	for (i = 0; i < touch_count; i++, data++) {
		if (data->track_id >= ts->max_finger)
			continue;

		if (palm_bit & BIT(data->track_id)) {
			ts->intr_palm |= BIT(data->track_id);
			continue;
		}

		if ((data->event == TOUCHSTS_DOWN) ||
			(data->event == TOUCHSTS_MOVE)) {
			ts->new_mask |= BIT(data->track_id);
			tdata = ts->tdata + data->track_id;

			tdata->id = data->track_id;
			tdata->type = data->tool_type;
			tdata->event = data->event;
			tdata->x = data->x;
			tdata->y = data->y;
			tdata->pressure = data->pressure;
			tdata->width_major = data->width_major;
			tdata->width_minor = data->width_minor;
			tdata->orientation = (s8)data->angle;

			finger_index++;

			t_dev_dbg_abs(ts->dev,
				"touch data [id %d, t %d, e %d, x %d, y %d, z %d - %d, %d, %d]\n",
				tdata->id,
				tdata->type,
				tdata->event,
				tdata->x,
				tdata->y,
				tdata->pressure,
				tdata->width_major,
				tdata->width_minor,
				tdata->orientation);
		}
	}

	ts->tcount = finger_index;
	ts->intr_status |= TS_IRQ_FINGER;

out:
	return ret;
}

static int __irq_abs_pre_default(struct lxs_ts *ts)
{
	struct lxs_hal_touch_info *info = lxs_report_info(ts);
	struct lxs_hal_touch_data *data = lxs_report_data(ts);
	int touch_cnt = info->touch_cnt;

	/* check if touch cnt is valid */
	if (!touch_cnt || (touch_cnt > MAX_FINGER)) {
		t_dev_dbg_abs(ts->dev, "Invalid touch count, %d\n", touch_cnt);

		/* debugging */
		t_dev_dbg_abs(ts->dev, "t %d, ev %d, id %d, x %d, y %d, p %d, a %d, w %d %d\n",
			data->tool_type, data->event, data->track_id,
			data->x, data->y, data->pressure, data->angle,
			data->width_major, data->width_minor);

		return -ERANGE;
	}

	return 0;
}

static int __irq_abs_data_type_1(struct lxs_ts *ts)
{
	struct lxs_hal_touch_info *info = lxs_report_info(ts);
	struct lxs_hal_touch_data_type_1 *data = lxs_report_data(ts);
	struct touch_data *tdata = NULL;
	u32 palm_bit = (ts->plat_data->use_palm_opt) ? info->palm_bit : 0;
	u32 touch_count = info->touch_cnt;
	int finger_index = 0;
	int i = 0;
	int ret = 0;

	ts->intr_status &= ~TS_IRQ_FINGER;

	ts->new_mask = 0;

	if (!touch_count)
		goto out;

	/* check if palm detected */
	ret = __irq_abs_palm_chk(ts, data->track_id, data->event);
	if (ret == TC_PALM_DETECTED)
		goto out;

	for (i = 0; i < touch_count; i++, data++) {
		if (data->track_id >= ts->max_finger)
			continue;

		if (palm_bit & BIT(data->track_id)) {
			ts->intr_palm |= BIT(data->track_id);
			continue;
		}

		if ((data->event == TOUCHSTS_DOWN) ||
			(data->event == TOUCHSTS_MOVE)) {
			ts->new_mask |= BIT(data->track_id);
			tdata = ts->tdata + data->track_id;

			tdata->id = data->track_id;
			tdata->type = data->tool_type;
			tdata->event = data->event;
			tdata->x = data->x;
			tdata->y = data->y;
			tdata->pressure = data->pressure;
			tdata->width_major = data->width_major;
			tdata->width_minor = data->width_minor;
			tdata->orientation = (s8)data->angle;

			finger_index++;

			t_dev_dbg_abs(ts->dev,
				"touch data [id %d, t %d, e %d, x %d, y %d, z %d - %d, %d, %d]\n",
				tdata->id,
				tdata->type,
				tdata->event,
				tdata->x,
				tdata->y,
				tdata->pressure,
				tdata->width_major,
				tdata->width_minor,
				tdata->orientation);
		}
	}

	ts->tcount = finger_index;
	ts->intr_status |= TS_IRQ_FINGER;

out:
	return ret;
}

static int __irq_abs_pre_type_1(struct lxs_ts *ts)
{
	return __irq_abs_pre_default(ts);
}

static int lxs_hal_chk_report_type(struct lxs_ts *ts)
{
	struct lxs_ts_chip *chip = &ts->chip;
	struct lxs_hal_report_ops *ops = &chip->report_ops;

	ops->abs_name = "";
	ops->lpwg_name = "";

	switch (chip->report_type) {
	case CHIP_REPORT_TYPE_1:
		chip->report_info = &chip->info_grp.info;
		chip->report_data = chip->info_grp.info.data;
		chip->report_size = sizeof(chip->info_grp.info);
		ops->abs_name = " (type 1)";
		ops->abs_pre = __irq_abs_pre_type_1;
		ops->abs_data = __irq_abs_data_type_1;
		break;
	default:
		chip->report_info = &chip->info_grp.info;
		chip->report_data = chip->info_grp.info.data;
		chip->report_size = sizeof(chip->info_grp.info);
		break;
	}

	t_dev_info(ts->dev, "report type  : %d\n", chip->report_type);

	return 0;
}

static void lxs_hal_clr_report_type(struct lxs_ts *ts)
{
	struct lxs_ts_chip *chip = &ts->chip;

	chip->report_info = NULL;
	chip->report_data = NULL;
	chip->report_size = 0;

	chip->report_type = 0;
}

static int lxs_hal_chk_status_type(struct lxs_ts *ts)
{
	struct lxs_ts_chip *chip = &ts->chip;
	struct lxs_hal_status_mask_bit *mask_bit = &chip->status_mask_bit;
	int t_sts_mask = chip->opt.t_sts_mask;

	switch (chip->status_type) {
	case CHIP_STATUS_TYPE_0:
		chip->status_filter = (struct lxs_hal_status_filter *)status_filter_type_0;
		break;
	default:
		chip->status_filter = (struct lxs_hal_status_filter *)status_filter_type_1;
		break;
	}

	mask_bit->valid_dev_ctl = lxs_hal_get_status_mask(ts, STS_ID_VALID_DEV_CTL);
	mask_bit->valid_code_crc = lxs_hal_get_status_mask(ts, STS_ID_VALID_CODE_CRC);
	mask_bit->valid_cfg_crc = lxs_hal_get_status_mask(ts, STS_ID_VALID_CFG_CRC);;
	mask_bit->error_abnormal = lxs_hal_get_status_mask(ts, STS_ID_ERROR_ABNORMAL);
	mask_bit->error_system = lxs_hal_get_status_mask(ts, STS_ID_ERROR_SYSTEM);
	mask_bit->error_mismtach = lxs_hal_get_status_mask(ts, STS_ID_ERROR_MISMTACH);
	mask_bit->valid_irq_pin = lxs_hal_get_status_mask(ts, STS_ID_VALID_IRQ_PIN);
	mask_bit->valid_irq_en = lxs_hal_get_status_mask(ts, STS_ID_VALID_IRQ_EN);
	mask_bit->error_mem = lxs_hal_get_status_mask(ts, STS_ID_ERROR_MEM);
	mask_bit->valid_tv_drv = lxs_hal_get_status_mask(ts, STS_ID_VALID_TC_DRV);
	mask_bit->error_disp = lxs_hal_get_status_mask(ts, STS_ID_ERROR_DISP);

	t_dev_dbg_base(ts->dev, "mask[v_dev]  : %08Xh\n", mask_bit->valid_dev_ctl);
	t_dev_dbg_base(ts->dev, "mask[v_code] : %08Xh\n", mask_bit->valid_code_crc);
	t_dev_dbg_base(ts->dev, "mask[v_cfg]  : %08Xh\n", mask_bit->valid_cfg_crc);
	t_dev_dbg_base(ts->dev, "mask[e_abn]  : %08Xh\n", mask_bit->error_abnormal);
	t_dev_dbg_base(ts->dev, "mask[e_sys]  : %08Xh\n", mask_bit->error_system);
	t_dev_dbg_base(ts->dev, "mask[e_mis]  : %08Xh\n", mask_bit->error_mismtach);
	t_dev_dbg_base(ts->dev, "mask[v_i_p]  : %08Xh\n", mask_bit->valid_irq_pin);
	t_dev_dbg_base(ts->dev, "mask[v_i_e]  : %08Xh\n", mask_bit->valid_irq_en);
	t_dev_dbg_base(ts->dev, "mask[e_mem]  : %08Xh\n", mask_bit->error_mem);
	t_dev_dbg_base(ts->dev, "mask[v_tc]   : %08Xh\n", mask_bit->valid_tv_drv);
	t_dev_dbg_base(ts->dev, "mask[e_disp] : %08Xh\n", mask_bit->error_disp);

	chip->status_mask_normal = mask_bit->valid_dev_ctl |
						mask_bit->valid_code_crc |
						mask_bit->valid_cfg_crc |
						mask_bit->valid_irq_pin |
						mask_bit->valid_irq_en |
						mask_bit->valid_tv_drv |
						0;

	chip->status_mask_logging = mask_bit->error_mismtach |
						mask_bit->valid_irq_pin |
						mask_bit->valid_irq_en |
						mask_bit->valid_tv_drv |
						0;

	chip->status_mask_reset = mask_bit->valid_dev_ctl |
						mask_bit->valid_code_crc |
						mask_bit->valid_cfg_crc |
						mask_bit->error_abnormal |
						mask_bit->error_system |
						mask_bit->error_mem |
						0;

	chip->status_mask = chip->status_mask_normal |
						chip->status_mask_logging |
						chip->status_mask_reset |
						0;

	chip->status_mask_ic_abnormal = INT_IC_ABNORMAL_STATUS;
	chip->status_mask_ic_error = INT_IC_ERROR_STATUS;
	chip->status_mask_ic_disp_err = 0;
	chip->status_mask_ic_debug = 0;

	switch (t_sts_mask) {
	case 1:
		chip->status_mask_ic_valid = 0xFFFF;
		chip->status_mask_ic_disp_err = (0x3<<6);
		break;
	case 4:
	case 2:
		chip->status_mask_ic_abnormal |= (0x3<<1);
		chip->status_mask_ic_error = (BIT(7) | BIT(5));
		chip->status_mask_ic_valid = (t_sts_mask == 4) ? 0x1FFFFF : 0x7FFFF;
		chip->status_mask_ic_disp_err = (0x3<<8);
		chip->status_mask_ic_debug = 0x1FF00 & ~chip->status_mask_ic_disp_err;
		break;
	case 5:
	case 3:
		chip->status_mask_ic_abnormal = 0;
		chip->status_mask_ic_error = (BIT(3) | BIT(1));
		chip->status_mask_ic_valid = (t_sts_mask == 5) ? 0x3FF : 0x7FFFF;
		break;
	default:
		chip->status_mask_ic_valid = 0xFF;
		break;
	}

	chip->status_mask_ic_normal = chip->status_mask_ic_valid;
	chip->status_mask_ic_normal &= ~chip->status_mask_ic_abnormal;
	chip->status_mask_ic_normal &= ~chip->status_mask_ic_error;
	chip->status_mask_ic_normal &= ~chip->status_mask_ic_disp_err;

	t_dev_info(ts->dev, "status type  : %d\n", chip->status_type);
	t_dev_info(ts->dev, "status mask  : %08Xh\n", chip->status_mask);
	t_dev_info(ts->dev, " normal      : %08Xh\n", chip->status_mask_normal);
	t_dev_info(ts->dev, " logging     : %08Xh\n", chip->status_mask_logging);
	t_dev_info(ts->dev, " reset       : %08Xh\n", chip->status_mask_reset);
	t_dev_info(ts->dev, " ic normal   : %08Xh\n", chip->status_mask_ic_normal);
	t_dev_info(ts->dev, " ic abnormal : %08Xh\n", chip->status_mask_ic_abnormal);
	t_dev_info(ts->dev, " ic error    : %08Xh\n", chip->status_mask_ic_error);
	t_dev_info(ts->dev, " ic valid    : %08Xh\n", chip->status_mask_ic_valid);

	if (chip->status_mask_ic_disp_err)
		t_dev_info(ts->dev, " ic disp err : %08Xh\n", chip->status_mask_ic_disp_err);

	if (chip->status_mask_ic_debug)
		t_dev_info(ts->dev, " ic debug    : %08Xh\n", chip->status_mask_ic_debug);

	return 0;
}

static void lxs_hal_clr_status_type(struct lxs_ts *ts)
{
	struct lxs_ts_chip *chip = &ts->chip;

	chip->status_filter = NULL;

	chip->status_type = 0;
}

static const struct lxs_ic_info_chip_proto lxs_ic_info_chip_protos[] = {
	{ CHIP_SW82907, 10, 4 },
	{ CHIP_NONE, 0, 0 },	//End mark
};

static int lxs_hal_ic_info_ver_check(struct lxs_ts *ts)
{
	struct lxs_ts_chip *chip = &ts->chip;
	struct lxs_hal_fw_info *fw = &chip->fw;
	const struct lxs_ic_info_chip_proto *chip_proto = lxs_ic_info_chip_protos;
	u32 vchip = fw->v.version.chip;
	u32 vproto = fw->v.version.protocol;

	while (1) {
		if (chip_proto->chip_type == CHIP_NONE)
			break;

		if (ts->chip_type == chip_proto->chip_type) {
			if ((chip_proto->vchip != vchip) ||
				(chip_proto->vproto != vproto))
				break;

			t_dev_info(ts->dev, "[%s] IC info is good: %d, %d\n",
				ts->chip_name, vchip, vproto);

			return 0;
		}

		chip_proto++;
	}

	t_dev_err(ts->dev, "[%s] IC info is abnormal: %d, %d\n",
		ts->chip_name, vchip, vproto);

	return -EINVAL;
}

static int lxs_hal_ic_info_boot(struct lxs_ts *ts)
{
	struct lxs_ts_chip *chip = &ts->chip;
	int boot_fail_cnt = chip->boot_fail_cnt;
	int ret = 0;

	ret = lxs_hal_chk_boot(ts);
	if (ret < 0)
		return ret;

	if (ret) {
		atomic_set(&chip->boot, TC_IC_BOOT_FAIL);

		/* Limit to avoid infinite repetition */
		if (boot_fail_cnt >= BOOT_FAIL_RECOVERY_MAX) {
			t_dev_err(ts->dev, "Boot fail can't be recovered(%d) - %02Xh\n",
				boot_fail_cnt, ret);
			return -EFAULT;
		}

		t_dev_err(ts->dev, "Boot fail detected(%d) - %02Xh\n",
			boot_fail_cnt, ret);

		chip->boot_fail_cnt++;

		/* return special flag to let the core layer know */
		return -ETDBOOTFAIL;
	}
	chip->boot_fail_cnt = 0;

	return 0;
}

static int __lxs_hal_sys_id(struct lxs_ts *ts)
{
	struct lxs_ts_chip *chip = &ts->chip;
	struct lxs_hal_reg *reg = chip->reg;
	struct lxs_hal_fw_info *fw = &chip->fw;
	u32 sys_id_addr = fw->sys_id_addr;
	u32 data = 0;
	int ret = 0;

	fw->sys_id_raw = 0;
	memset(fw->sys_id, 0, sizeof(fw->sys_id));

	if (!sys_id_addr)
		return 0;

	ret = lxs_hal_write_value(ts, reg->spr_code_offset, sys_id_addr);
	if (ret < 0)
		goto out;

	ret = lxs_hal_read_value(ts, reg->code_access_addr, &data);

	/* ignore return value */
	lxs_hal_write_value(ts, reg->spr_code_offset, 0);

	if (ret < 0)
		goto out;

	if (!data || (data == ~0)) {
	//	t_dev_warn(ts->dev, "%s has no sys id\n", ts->chip_name);
		goto out;
	}

	fw->sys_id_raw = data & 0xFFFFF;
	snprintf(fw->sys_id, sizeof(fw->sys_id) - 1, "%X", fw->sys_id_raw);

out:
	return ret;
}

static int __lxs_hal_ic_info(struct lxs_ts *ts, int prt_on)
{
	struct lxs_ts_chip *chip = &ts->chip;
	struct lxs_hal_reg *reg = chip->reg;
	struct lxs_hal_fw_info *fw = &chip->fw;
//	char sys_id_str[16] = {0, };
	u32 product[2] = {0};
	u32 chip_id = 0;
	u32 version = 0;
	u32 revision = 0;
	u32 bootmode = 0;
	u32 boot_chk_offset = 0;
	int invalid_pid = 0;
	struct lxs_hal_rw_multi multi[] = {
		{ 0, reg->spr_chip_id, &chip_id, sizeof(chip_id), "chip_id" },
		{ 0, reg->tc_version, &version, sizeof(version), "version" },
		{ 0, reg->info_chip_version, &revision, sizeof(revision), "revision" },
		{ 0, reg->tc_product_id1, product, sizeof(product), "product_id" },
		{ -1, -1, NULL, },
	};
	int ret = 0;

	ret = lxs_hal_reg_rw_multi(ts, multi, "ic_info(1)");
	if (ret < 0)
		return ret;

	ret = lxs_hal_get_boot_result(ts, &bootmode);
	if (ret < 0) {
		t_dev_err(ts->dev, "ic_info(1): failed to get boot status, %d\n", ret);
		return ret;
	}

	__lxs_hal_sys_id(ts);

#if 0
	if (fw->sys_id_raw)
		snprintf(sys_id_str, sizeof(sys_id_str) - 1, "(%s)", fw->sys_id);
#endif

	lxs_hal_fw_set_chip_id(fw, chip_id);
	lxs_hal_fw_set_version(fw, version);
	lxs_hal_fw_set_revision(fw, revision);
	lxs_hal_fw_set_prod_id(fw, (u8 *)product, sizeof(product));

	invalid_pid = fw->invalid_pid;
	if (invalid_pid)
		t_dev_err(ts->dev, "[info] invalid PID - \"%s\" (%03Xh)\n",
			fw->product_id, invalid_pid);

	t_dev_info_sel(ts->dev, prt_on,
		"[T] chip class %s, version v%u.%02u (0x%08X, 0x%02X)\n",
		fw->chip_id,
		fw->v.version.major, fw->v.version.minor,
		version, fw->revision);

	boot_chk_offset = lxs_hal_boot_sts_pos_busy(ts);
	t_dev_info_sel(ts->dev, prt_on,
		"[T] product id %s, flash boot %s(%s), crc %s (0x%08X)\n",
		fw->product_id,
		((bootmode >> boot_chk_offset) & 0x1) ? "BUSY" : "idle",
		((bootmode >> (boot_chk_offset + 1)) & 0x1) ? "done" : "booting",
		((bootmode >> (boot_chk_offset + 2)) & 0x1) ? "ERROR" : "ok",
		bootmode);

	ret = lxs_hal_ic_info_boot(ts);
	if (ret)
		return ret;

	if (strcmp(fw->chip_id, ts->chip_id)) {
		t_dev_err(ts->dev, "Invalid chip class(%s), shall be %s\n",
			fw->chip_id, ts->chip_id);
		return -EINVAL;
	}

	ret = lxs_hal_ic_info_ver_check(ts);
	if (ret < 0)
		return ret;

	if (invalid_pid)
		return -EINVAL;

	return 0;
}

int lxs_hal_ic_info(struct lxs_ts *ts)
{
	return __lxs_hal_ic_info(ts, 1);
}

static int lxs_hal_init_reg_set(struct lxs_ts *ts)
{
	struct lxs_ts_chip *chip = &ts->chip;
	struct lxs_hal_reg *reg = chip->reg;
	int ret = 0;

	ret = lxs_hal_write_value(ts, reg->tc_device_ctl, 1);
	if (ret < 0)
		goto out;

	if (!lxs_addr_is_invalid(reg->charger_info)) {
		ret = lxs_hal_write_value(ts, reg->charger_info, chip->charger);
		if (ret < 0)
			goto out;
	}

	if (!lxs_addr_is_invalid(reg->ime_state)) {
		ret = lxs_hal_write_value(ts, reg->ime_state, ts->state_ime);
		if (ret < 0)
			goto out;
	}

out:
	return ret;
}

static int __ic_test_unit(struct lxs_ts *ts, u32 addr, u32 data)
{
	u32 data_rd;
	int ret;

	ret = lxs_hal_write_value(ts, addr, data);
	if (ret < 0) {
		t_dev_err(ts->dev, "ic test: wr err, %08Xh, %d\n", data, ret);
		return ret;
	}

	ret = lxs_hal_read_value(ts, addr, &data_rd);
	if (ret < 0) {
		t_dev_err(ts->dev, "ic test: rd err: %08Xh, %d\n", data, ret);
		return ret;
	}

	if (data != data_rd) {
		t_dev_err(ts->dev, "ic test: cmp err, %08Xh, %08Xh\n", data, data_rd);
		return -EFAULT;
	}

	return 0;
}

static int lxs_hal_ic_test(struct lxs_ts *ts)
{
	struct lxs_ts_chip *chip = &ts->chip;
	struct lxs_hal_reg *reg = chip->reg;
	u32 addr = reg->spr_chip_test;
	u32 data[] = {
		0x5A5A5A5A,
		0xA5A5A5A5,
		0xF0F0F0F0,
		0x0F0F0F0F,
		0xFF00FF00,
		0x00FF00FF,
		0xFFFF0000,
		0x0000FFFF,
		0xFFFFFFFF,
		0x00000000,
	};
	int i;
	int ret = 0;

	if (lxs_addr_is_invalid(addr)) {
		t_dev_warn(ts->dev, "ic test: invalid addr\n");
		return 0;
	}

	for (i = 0; i < ARRAY_SIZE(data); i++) {
		ret = __ic_test_unit(ts, addr, data[i]);
		if (ret < 0)
			break;
	}

	if (!ret)
		t_dev_info(ts->dev, "ic test done\n");

	return ret;
}

static void lxs_hal_init_reset(struct lxs_ts *ts)
{
	struct lxs_ts_chip *chip = &ts->chip;

	if (chip->ops_quirk.hw_reset != NULL) {
		chip->ops_quirk.hw_reset(ts, 1);
		return;
	}

	lxs_ts_irq_control(ts, 0);
	lxs_ts_power_reset(ts, TS_POWER_OFF);
	lxs_ts_power_reset(ts, TS_POWER_ON);
	lxs_ts_delay(ts->hw_reset_delay);
}

static int lxs_hal_init_pre(struct lxs_ts *ts)
{
	int ret = 0;

	ret = lxs_hal_ic_test(ts);
	if (ret < 0)
		goto out;

	if (ts->fquirks->init_pre) {
		ret = ts->fquirks->init_pre(ts);
		if (ret < 0)
			goto out;
	}

out:
	return ret;
}

static int lxs_hal_init_post(struct lxs_ts *ts)
{
	int ret = 0;

	lxs_hal_init_reg_set(ts);

	if (ts->fquirks->init_post) {
		ret = ts->fquirks->init_post(ts);
		if (ret < 0)
			return ret;
	}

	return 0;
}

static int lxs_hal_init_end(struct lxs_ts *ts)
{
	int ret = 0;

	if (ts->fquirks->init_end) {
		ret = ts->fquirks->init_end(ts);
		if (ret < 0)
			return ret;
	}

	return 0;
}

int lxs_hal_init(struct lxs_ts *ts)
{
	struct lxs_ts_chip *chip = &ts->chip;
	int is_probe = !!(ts->state_core == TS_CORE_PROBE);
	int init_retry = 2<<(is_probe);
	int i;
	int ret = 0;

	lxs_ts_release_all_event(ts);

	chip->reset_is_on_going = true;

	atomic_set(&chip->boot, TC_IC_BOOT_DONE);

	ret = lxs_hal_init_pre(ts);
	if (ret < 0) {
		lxs_hal_init_reset(ts);
		goto out;
	}

	t_dev_dbg_base(ts->dev, "charger_state = 0x%02X\n", chip->charger);

	for (i = 0; i < init_retry; i++) {
		ret = lxs_hal_ic_info(ts);
		if (ret >= 0)
			break;

		if (ret == -ETDBOOTFAIL) {
			/* For the probe stage */
			if (ts->state_core == TS_CORE_PROBE)
				break;
		}

		t_dev_dbg_base(ts->dev, "retry getting ic info (%d)\n", i);

		lxs_hal_init_reset(ts);
	}
	if (ret < 0)
		goto out;

	ret = lxs_hal_init_post(ts);
	if (ret < 0)
		goto out;

	atomic_set(&chip->init, TC_IC_INIT_DONE);

	ts->state_sleep = TS_IC_NORMAL;

	ret = lxs_hal_enable_touch(ts, ts->state_mode);
	if (ret < 0) {
		t_dev_err(ts->dev, "failed to enable device, %d\n", ret);
		goto out;
	}

	lxs_hal_init_end(ts);

	lxs_hal_add_sysfs(ts);

	t_dev_info(ts->dev, "LXS(%s) init done\n", ts->chip_name);

	chip->reset_is_on_going = false;

	return 0;

out:
	t_dev_err(ts->dev, "LXS(%s) init failed, %d\n", ts->chip_name, ret);

	chip->reset_is_on_going = false;

	return ret;
}

static int __reinit_quirk(struct lxs_ts *ts, int pwr_con, int init)
{
	struct lxs_ts_chip *chip = &ts->chip;
	int ret = 0;

	chip->ops_quirk.hw_reset(ts, pwr_con);

	lxs_ts_delay(ts->hw_reset_delay);

	ret = (init) ? lxs_hal_init(ts) : 0;

	return ret;
}

static int __reinit_normal(struct lxs_ts *ts, int pwr_con, int init)
{
	struct lxs_ts_chip *chip = &ts->chip;
	int ret = 0;

	lxs_ts_irq_control(ts, 0);

	atomic_set(&chip->init, TC_IC_INIT_NEED);

	if (pwr_con) {
		lxs_ts_power_reset(ts, TS_POWER_OFF);
		lxs_ts_power_reset(ts, TS_POWER_ON);
	} else {
		lxs_ts_gpio_set_reset(ts, 0);
		lxs_ts_delay(chip->drv_reset_low);
		lxs_ts_gpio_set_reset(ts, 1);
	}

	lxs_ts_delay(ts->hw_reset_delay);

	ret = (init) ? lxs_hal_init(ts) : 0;

	return ret;
}

static int lxs_hal_reinit(struct lxs_ts *ts, int skip_quirk, int pwr_con, int init)
{
	struct lxs_ts_chip *chip = &ts->chip;

	if (!skip_quirk && (chip->ops_quirk.hw_reset != NULL))
		return __reinit_quirk(ts, pwr_con, init);

	return __reinit_normal(ts, pwr_con, init);
}

static int __sw_reset_type_5(struct lxs_ts *ts)
{
	struct lxs_ts_chip *chip = &ts->chip;
	struct lxs_hal_reg *reg = chip->reg;
	u32 addr = reg->spr_rst_ctl;
	u32 value = 1;

	t_dev_dbg_trace(ts->dev, "spr_rst_ctl[%04Xh] = %08Xh\n", addr, value);
	lxs_hal_write_value(ts, addr, value);

	lxs_ts_delay(20);

	return 0;
}

static int __sw_reset(struct lxs_ts *ts)
{
	struct lxs_ts_chip *chip = &ts->chip;
	int t_sw_rst = chip->opt.t_sw_rst;
	int ret = 0;

	if (t_sw_rst > TC_SW_RST_TYPE_MAX) {
		t_dev_warn(ts->dev, "sw reset not supported\n");
		ret = -EPERM;
		goto out;
	}

	lxs_ts_irq_control(ts, 0);

	atomic_set(&chip->init, TC_IC_INIT_NEED);

	t_dev_info(ts->dev, "SW Reset(%d)\n", t_sw_rst);

	lxs_ts_delay(chip->drv_reset_low + 5);

	switch (t_sw_rst) {
	case 5:
		ret = __sw_reset_type_5(ts);
		break;
	default:
		t_dev_warn(ts->dev, "unknown sw reset type, %d\n", t_sw_rst);
		ret = -ESRCH;
		break;
	}

	if (chip->ops_quirk.sw_reset_post)
		chip->ops_quirk.sw_reset_post(ts);

out:
	return ret;
}

static int lxs_hal_sw_reset(struct lxs_ts *ts)
{
	struct lxs_ts_chip *chip = &ts->chip;
	int delay = msecs_to_jiffies(ts->sw_reset_delay);
	int ret = 0;

	ret = __sw_reset(ts);

	if (chip->ops_quirk.hw_reset != NULL)
		delay = msecs_to_jiffies(ts->hw_reset_delay);

	queue_delayed_work(ts->wq, &ts->init_work, delay);

	return ret;
}

static int lxs_hal_hw_reset_quirk(struct lxs_ts *ts, int pwr_con)
{
	int ret = 0;

	t_dev_info(ts->dev, "run sw reset (reset gpio deactivated)\n");

	if (pwr_con) {
		lxs_ts_irq_control(ts, 0);

		lxs_ts_power_ctrl(ts, TS_POWER_OFF);
		lxs_ts_power_ctrl(ts, TS_POWER_ON);
		lxs_ts_delay(ts->hw_reset_delay);
	}

	ret = __sw_reset(ts);

	lxs_ts_delay(ts->hw_reset_delay);

	return ret;
}

static int lxs_hal_hw_reset(struct lxs_ts *ts, int ctrl, int pwr_con)
{
	int skip_quirk = !!(ctrl & TC_HW_RESET_SKIP_QUIRK);
	int err = 0;
	int ret = 0;

	ctrl &= 0x0F;

	t_dev_info(ts->dev, "HW Reset(%s)\n",
		(ctrl == TC_HW_RESET_ASYNC) ? "Async" : "Sync");

	if (ctrl == TC_HW_RESET_ASYNC) {
		lxs_hal_reinit(ts, skip_quirk, pwr_con, 0);

		queue_delayed_work(ts->wq, &ts->init_work, 0);
		return 0;
	}

	err = lxs_ts_pw_lock(ts, INCELL_DISPLAY_POWER_LOCK);
	if (err) {
		t_dev_err(ts->dev, "%s: failed lock power, %d\n", __func__, err);
		return err;
	}

	ret = lxs_hal_reinit(ts, skip_quirk, pwr_con, 1);
	if (!ret)
		lxs_ts_irq_control(ts, 1);

	err = lxs_ts_pw_lock(ts, INCELL_DISPLAY_POWER_UNLOCK);
	if (err)
		t_dev_err(ts->dev, "%s: failed unlock power, %d\n", __func__, err);

	if (ret == -ETDBOOTFAIL) {
		t_dev_info(ts->dev, "%s: F/W upgrade triggered\n", __func__);
		queue_delayed_work(ts->wq, &ts->upgrade_work, msecs_to_jiffies(100));
	}

	if (ret < 0)
		return ret;

	lxs_ts_watchdog_run(ts);

	return 0;
}

int lxs_hal_reset(struct lxs_ts *ts, int ctrl, int pwr_con)
{
	struct lxs_ts_chip *chip = &ts->chip;
	int ret = -EINVAL;

	mutex_lock(&ts->reset_lock);

	if (chip->reset_is_on_going) {
		t_dev_err(ts->dev, "%s: reset is ongoing\n", __func__);
		goto out;
	}

	lxs_ts_watchdog_stop(ts);

	chip->reset_is_on_going = true;

	ts->power_enabled = TS_POWER_ON;

	t_dev_info(ts->dev, "LXS(%s) reset control(0x%X)\n",
		ts->chip_name, ctrl);

	switch (ctrl) {
	case TC_SW_RESET:
		ret = lxs_hal_sw_reset(ts);
		break;
	case TC_HW_RESET_ASYNC:
	case TC_HW_RESET_SYNC:
		ret = lxs_hal_hw_reset(ts, ctrl, pwr_con);
		break;

	default:
		t_dev_err(ts->dev, "unknown reset type, 0x%X\n", ctrl);
		break;
	}

out:
	mutex_unlock(&ts->reset_lock);

	return ret;
}

static void lxs_hal_connect(struct lxs_ts *ts)
{
	struct lxs_ts_chip *chip = &ts->chip;
	struct lxs_hal_reg *reg = chip->reg;
	int charger_state = ts->state_connect;
	int wireless_state = ts->state_wireless;

	chip->charger = 0;

	switch (ts->plat_data->use_charger_opt) {
	case 1:
		chip->charger = (charger_state || wireless_state);
		break;
	default:
		switch (charger_state) {
		case TS_CONNECT_INVALID:
			chip->charger = TS_CONNECT_NONE;
			break;
		case TS_CONNECT_DCP:
		case TS_CONNECT_PROPRIETARY:
			chip->charger = TS_CONNECT_DC;
			break;
		case TS_CONNECT_HUB:
			chip->charger = TS_CONNECT_OTG;
			break;
		default:
			chip->charger = TS_CONNECT_USB;
			break;
		}

		/* wireless */
		chip->charger |= (wireless_state) ? TS_CONNECT_WIRELESS : 0;
		break;
	}

	t_dev_info(ts->dev,
		"charger: %Xh (%Xh, %Xh)\n",
		chip->charger, charger_state, wireless_state);

	lxs_hal_write_value(ts, reg->charger_info, chip->charger);
}

int lxs_hal_usb_status(struct lxs_ts *ts)
{
	t_dev_info(ts->dev, "TA Type: %d\n", ts->state_connect);

	lxs_hal_connect(ts);

	return 0;
}

int lxs_hal_wireless_status(struct lxs_ts *ts)
{
	t_dev_info(ts->dev, "Wireless charger: 0x%02X\n", ts->state_wireless);

	lxs_hal_connect(ts);

	return 0;
}

int lxs_hal_ime_state(struct lxs_ts *ts)
{
	struct lxs_ts_chip *chip = &ts->chip;
	struct lxs_hal_reg *reg = chip->reg;

	if (lxs_addr_is_invalid(reg->ime_state))
		return 0;

	t_dev_info(ts->dev, "state_ime: %d\n", ts->state_ime);

	lxs_hal_write_value(ts, reg->ime_state, ts->state_ime);

	return 0;
}

int lxs_hal_incoming_call(struct lxs_ts *ts)
{
	struct lxs_ts_chip *chip = &ts->chip;
	struct lxs_hal_reg *reg = chip->reg;
	char *__strs[] = {
		[TS_INCOMING_CALL_IDLE]	= "IDLE",
		[TS_INCOMING_CALL_RINGING] = "RINGING",
		[TS_INCOMING_CALL_OFFHOOK] = "OFFHOOK",
	};
	char *str;
	int value = ts->state_incoming_call;

	if (lxs_addr_is_invalid(reg->call_state))
		return 0;

	str = (value <= TS_INCOMING_CALL_OFFHOOK) ? __strs[value] : "(unknown)";

	t_dev_info(ts->dev, "state_call: %d(%s)\n", value, str);

	lxs_hal_write_value(ts, reg->call_state, ts->state_incoming_call);

	return 0;
}

static int lxs_hal_tc_con_type_g(struct lxs_ts *ts, u32 addr, int value, char *name)
{
	int ret = 0;

	ret = lxs_hal_write_value(ts, addr, value);
	if (ret < 0) {
		t_dev_err(ts->dev, "failed to set %s[%04Xh], %d\n",
			name, addr, ret);
		goto out;
	}

	t_dev_info(ts->dev, "%s[%04Xh]: %s(%08Xh)\n",
		name, addr, (value & 0x1) ? "ON" : "OFF", value);

out:
	return ret;
}

static int lxs_hal_tc_con_glove(struct lxs_ts *ts)
{
	struct lxs_ts_chip *chip = &ts->chip;
	struct lxs_hal_reg *reg = chip->reg;
	u32 addr = reg->glove_mode;
	int value = chip->glove;
	int ret = 0;

	if (chip->opt.f_glove_en)
		ret = lxs_hal_tc_con_type_g(ts, addr, value, "glove_mode");

	return ret;
}

static int lxs_hal_tc_con_grab(struct lxs_ts *ts)
{
	struct lxs_ts_chip *chip = &ts->chip;
	struct lxs_hal_reg *reg = chip->reg;
	u32 addr = reg->grab_mode;
	int value = chip->grab;
	int ret = 0;

	if (chip->opt.f_grab_en)
		ret = lxs_hal_tc_con_type_g(ts, addr, value, "grab_mode");

	return ret;
}

int lxs_hal_tc_con(struct lxs_ts *ts, u32 code, void *param)
{
	int ret = 0;

	if (lxs_hal_tc_not_allowed(ts, "tc con not allowed"))
		return 0;

	switch (code) {
	case TCON_GLOVE:
		ret = lxs_hal_tc_con_glove(ts);
		break;
	case TCON_GRAB:
		ret = lxs_hal_tc_con_grab(ts);
		break;
	}

	return ret;
}

static int lxs_hal_disp_frame_rate(struct lxs_ts *ts, int mode)
{
	struct lxs_ts_chip *chip = &ts->chip;
	struct lxs_hal_reg *reg = chip->reg;
	int is_resume = !!(ts->state_mode == TS_MODE_RESUME);
	int is_u3 = !!(mode == LCD_MODE_U3);
	int *d_frame_tbl = (is_u3) ? ts->d_frame_tbl_np : ts->d_frame_tbl_lp;
	int value = (is_u3) ? ts->d_frame_rate_np : ts->d_frame_rate_lp;

	if (mode == LCD_MODE_STOP)
		return 0;

	if (lxs_addr_is_invalid(reg->frame_rate))
		return 0;

	//AOD or OFF(EWU)
	if (!is_u3) {
		int __value = (is_resume) ? ts->d_frame_rate_aod : ts->d_frame_rate_off;
		if (value != __value)
			t_dev_dbg_trace(ts->dev, "display frame rate: invalid %d\n", value);
		value = __value;
	}

	if (touch_lookup_frame_tbl(d_frame_tbl, value) < 0 )
		return 0;

	t_dev_info(ts->dev, "display frame rate: %d\n", value);

	lxs_hal_write_value(ts, reg->frame_rate, value);

	return 0;
}

static int lxs_hal_touch_frame_rate(struct lxs_ts *ts, int mode)
{
	struct lxs_ts_chip *chip = &ts->chip;
	struct lxs_hal_reg *reg = chip->reg;
	int is_u3 = !!(mode == LCD_MODE_U3);
	int *t_frame_tbl = (is_u3) ? ts->t_frame_tbl_np : ts->t_frame_tbl_lp;
	int value = (is_u3) ? ts->t_frame_rate_np : ts->t_frame_rate_lp;

	if (mode == LCD_MODE_STOP)
		return 0;

	if (lxs_addr_is_invalid(reg->t_frame_rate))
		return 0;

	if (touch_lookup_frame_tbl(t_frame_tbl, value) < 0 )
		return 0;

	t_dev_info(ts->dev, "touch frame rate: %d\n", value);

	lxs_hal_write_value(ts, reg->t_frame_rate, value);

	return 0;
}

static void lxs_hal_tc_restore(struct lxs_ts *ts)
{
	lxs_hal_tc_con_glove(ts);
	lxs_hal_tc_con_grab(ts);

	lxs_hal_connect(ts);
	lxs_hal_ime_state(ts);
	lxs_hal_incoming_call(ts);

#if defined(__LXS_SUPPORT_FILTER_CON)
	lxs_hal_blend_filter(ts);
	lxs_hal_stop_filter(ts);
#endif
}

static void lxs_hal_check_dbg_report(struct lxs_ts *ts, u32 status, int irq)
{
	struct lxs_ts_chip *chip = &ts->chip;
	struct lxs_hal_reg *reg = chip->reg;
	u32 addr = reg->tc_ic_status;
	u32 irq_type = lxs_tc_sts_irq_type(status);
	u32 tc_debug[4] = {0, };
	int ret = 0;

	if (!chip->opt.f_dbg_report) {
		return;
	}

	switch (irq_type) {
	case TC_STS_IRQ_TYPE_ABNORMAL:
	case TC_STS_IRQ_TYPE_DEBUG:
		break;
	default:
		return;
	}

	addr += ((0x100>>2) - 2);

	ret = lxs_hal_reg_read(ts, addr, tc_debug, sizeof(tc_debug));
	if (ret < 0) {
		t_dev_err(ts->dev, "[%d] tc_debug: failed to get dbg_report, %d\n", irq, ret);
		return;
	}

	t_dev_info(ts->dev, "[%d] tc_debug: 0x%08X 0x%08X 0x%08X 0x%08X\n",
		irq, tc_debug[0], tc_debug[1], tc_debug[2], tc_debug[3]);
}

static int lxs_hal_tc_driving_cmd(struct lxs_ts *ts, int mode)
{
	struct lxs_ts_chip *chip = &ts->chip;
	int ctrl = 0;

	if ((mode < LCD_MODE_U0) || (mode >= LCD_MODE_MAX)) {
		t_dev_err(ts->dev, "invalid mode, %d\n", mode);
		return -EINVAL;
	}

	ctrl = chip->tc_cmd_table[mode];
	if (ctrl < 0) {
		t_dev_err(ts->dev, "%s(%d) not granted\n",
			lxs_lcd_driving_mode_str(mode), mode);
		return -ESRCH;
	}

	chip->driving_ctrl = ctrl;

	return ctrl;
}

static int lxs_hal_tc_driving_pre(struct lxs_ts *ts, int mode)
{
	struct lxs_ts_chip *chip = &ts->chip;
	struct lxs_hal_reg *reg = chip->reg;
	u32 rdata = 0;
	int ctrl = 0;
	int ret = 0;

	switch (chip->opt.t_tc_quirk) {
	case 1:
		if (chip->reset_is_on_going)
			break;

		if (chip->driving_mode == LCD_MODE_STOP)
			break;

		if (mode == LCD_MODE_STOP)
			break;

		ctrl = lxs_hal_tc_driving_cmd(ts, LCD_MODE_STOP);
		if (ctrl < 0)
			return ctrl;

		rdata = reg->tc_drive_ctl;
		if (lxs_addr_is_invalid(rdata))
			break;

		ret = lxs_hal_write_value(ts, rdata, ctrl);
		if (ret < 0)
			t_dev_err(ts->dev, "driving quirk - stop(0x%04X, 0x%X), %d\n",
				rdata, ctrl, ret);
		else
			t_dev_info(ts->dev, "driving quirk - stop(0x%04X, 0x%X)\n",
				rdata, ctrl);

		rdata = chip->drv_delay;
		lxs_ts_delay(rdata);
		break;
	}

	lxs_hal_disp_frame_rate(ts, mode);
	lxs_hal_touch_frame_rate(ts, mode);

	return ret;
}

int lxs_hal_tc_driving(struct lxs_ts *ts, int mode)
{
	struct lxs_ts_chip *chip = &ts->chip;
	struct lxs_hal_reg *reg = chip->reg;
	u32 tc_status = 0;
	u32 running_status = 0;
	u32 rdata;
	int ctrl = 0;
	int re_init = 0;
	int ret = 0;

	/* for restore */
	if (mode < 0) {
		mode = chip->driving_mode;
		atomic_set(&chip->recur, 1);
	}

	if (lxs_hal_tc_not_allowed(ts, "tc driving not allowed"))
		return 0;

	if (touch_mode_not_allowed(ts, mode))
		return -EPERM;

	if (atomic_read(&chip->recur)) {
		/* keep the last value for retry case */
		mode = chip->driving_mode;
		t_dev_info(ts->dev, "keep the last mode(%d) for retry\n", mode);
	}

	lxs_hal_tc_driving_pre(ts, mode);

	ctrl = lxs_hal_tc_driving_cmd(ts, mode);
	if (ctrl < 0)
		return ctrl;

	chip->driving_mode = mode;

	if (mode == LCD_MODE_U0)
		lxs_ts_delay(chip->drv_opt_delay);

	t_dev_info(ts->dev, "current driving mode is %s\n",
		lxs_lcd_driving_mode_str(mode));

	rdata = reg->tc_drive_ctl;

	if (!lxs_addr_is_invalid(rdata)) {
		ret = lxs_hal_write_value(ts, rdata, ctrl);
		if (ret < 0) {
			t_dev_err(ts->dev, "TC Driving[%04Xh](0x%X) failed, %d\n",
				rdata, ctrl, ret);
			return ret;
		}
		t_dev_info(ts->dev, "TC Driving[%04Xh] wr 0x%X\n",
			rdata, ctrl);

		rdata = chip->drv_delay;
		lxs_ts_delay(rdata);
		t_dev_dbg_base(ts->dev, "waiting %d msecs\n", rdata);
	}

	ret = lxs_hal_read_value(ts, reg->tc_status, &tc_status);
	if (ret < 0) {
		t_dev_err(ts->dev, "failed to get tc_status\n");
		atomic_set(&chip->recur, 0);
		return ret;
	}

	lxs_hal_check_dbg_report(ts, tc_status, 0);

	running_status = lxs_tc_sts_running_sts(tc_status);

	re_init = 0;
	if (mode == LCD_MODE_STOP) {
		re_init = !!running_status;
	} else {
		if (!running_status ||
			(running_status == 0x10) ||
			(running_status == 0x0F)) {
			re_init = 1;
		}
	}

	if (re_init) {
		if (atomic_read(&chip->recur)) {
			t_dev_err(ts->dev, "command failed: mode %d, tc_status %08Xh\n",
				mode, tc_status);
			atomic_set(&chip->recur, 0);
			return -EFAULT;
		}

		t_dev_err(ts->dev, "command missed: mode %d, tc_status %08Xh\n",
			mode, tc_status);

		atomic_set(&chip->recur, 1);

		ret = lxs_hal_reinit(ts, 0, 1, 1);
		lxs_ts_irq_control(ts, 1);
		if (ret < 0)
			return ret;
	} else {
		t_dev_info(ts->dev, "command done: mode %d, running_sts %02Xh\n",
			mode, running_status);
	}

	atomic_set(&chip->recur, 0);

	return 0;
}

#if defined(__LXS_SUPPORT_SLEEP)
static void __clock_reg_off(struct lxs_ts *ts, u32 addr, u32 mask, const char *str)
{
	u32 data = 0;
	int ret = 0;

	ret = lxs_hal_reg_bit_clr(ts, addr, &data, mask);
	if (ret < 0) {
		t_dev_err(ts->dev, "%s(0x%04X) failed, mask %X\n",
		str, addr, mask);
		goto out;
	}

	t_dev_info(ts->dev, "%s(0x%04X), data 0x%08X, mask %X\n",
		str, addr, data, mask);

out:
	lxs_ts_delay(1);
}

static void __clock_reg_chg(struct lxs_ts *ts, u32 addr, u32 data, u32 mask, const char *str)
{
	int ret = 0;

	ret = lxs_hal_reg_bit_chg(ts, addr, &data, mask);
	if (ret < 0) {
		t_dev_err(ts->dev, "%s(0x%04X) failed, mask %X\n",
		str, addr, mask);
		goto out;
	}

	t_dev_info(ts->dev, "%s(0x%04X), data 0x%08X, mask %X\n",
		str, addr, data, mask);

out:
	lxs_ts_delay(1);
}

static int __clock_type_4(struct lxs_ts *ts, bool onoff)
{
	if (onoff) {
		ts->state_sleep = TS_IC_NORMAL;
		t_dev_info(ts->dev, "lxs_hal_clock(4) -> reset(sync)\n");
		lxs_hal_reset(ts, TC_HW_RESET_SYNC, 0);
	} else {
		lxs_ts_irq_control(ts, 0);

		__clock_reg_off(ts, LDO15_CTL_T4, (3<<4),
			"lxs_hal_clock(4) LDO15_CTL");
	#if 1
		__clock_reg_chg(ts, SYS_LDO_CTL_T4,
			(1<<3), (3<<3),
			"lxs_hal_clock(4) SYS_LDO_CTL");
	#else
		__clock_reg_off(ts, SYS_LDO_CTL_T4, (3<<3),
			"lxs_hal_clock(4) SYS_LDO_CTL");
	#endif
		lxs_hal_write_value(ts, SYS_OSC_CTL_T4, 0);

		ts->state_sleep = TS_IC_SLEEP;
	}

	t_dev_info(ts->dev, "lxs_hal_clock(4) -> %s\n",
		(onoff) ? "ON" : "OFF");

	return 0;
}

static int lxs_hal_clock(struct lxs_ts *ts, bool onoff)
{
	struct lxs_ts_chip *chip = &ts->chip;
	int t_clock = chip->opt.t_clock;
	int state = ts->state_sleep;
	int ret = 0;

	if (onoff) {
		if (state != TS_IC_SLEEP) {
			return 0;
		}
	} else {
		if (state == TS_IC_SLEEP) {
			t_dev_info(ts->dev, "already sleep state\n");
			return 0;
		}
	}

	switch (t_clock) {
	case 4:
		ret = __clock_type_4(ts, onoff);
		break;
	default:
		ts->state_sleep = (onoff) ? TS_IC_NORMAL : TS_IC_SLEEP;
		t_dev_info(ts->dev, "sleep state -> %s\n",
			(onoff) ? "TS_IC_NORMAL" : "TS_IC_SLEEP");
		break;
	}

	return ret;
}

void lxs_hal_deep_sleep(struct lxs_ts *ts)
{
	t_dev_info(ts->dev, "deep sleep\n");

	lxs_hal_tc_driving(ts, LCD_MODE_STOP);
	lxs_hal_clock(ts, 0);
}
#else	/* !__LXS_SUPPORT_SLEEP */
void lxs_hal_deep_sleep(struct lxs_ts *ts)
{
	t_dev_info(ts->dev, "deep sleep(off)\n");

	lxs_ts_irq_control(ts, 0);

	lxs_ts_power_reset(ts, TS_POWER_OFF);

	ts->state_sleep = TS_IC_SLEEP;
}
#endif	/* __LXS_SUPPORT_SLEEP */

static int lxs_hal_suspend_touch(struct lxs_ts *ts)
{
	struct lxs_ts_chip *chip = &ts->chip;
	int lcd_mode = chip->lcd_mode;
	int ret = 0;

	t_dev_info(ts->dev, "%s suspend touch: knock %d, prox %d, sleep %d\n",
		ts->chip_name, chip->enable_knock, chip->enable_prox, chip->enable_sleep);

	if (!chip->enable_knock || chip->enable_prox) {
		if (chip->enable_sleep) {
			lxs_hal_deep_sleep(ts);
			ts->power_enabled = TS_POWER_OFF;
			t_dev_info(ts->dev, "%s suspend touch: deep sleep\n",
				ts->chip_name);
			return 0;
		}

		lcd_mode = LCD_MODE_STOP;
	}

	ts->power_enabled = TS_POWER_ON;
	ret = lxs_hal_tc_driving(ts, lcd_mode);

	t_dev_info(ts->dev, "%s suspend touch(%d): lcd_mode %d, driving_mode %d\n",
		ts->chip_name, ret, chip->lcd_mode, chip->driving_mode);

	return ret;
}

static int lxs_hal_resume_touch(struct lxs_ts *ts)
{
	struct lxs_ts_chip *chip = &ts->chip;
	int lcd_mode = chip->lcd_mode;
	int ret = 0;

	t_dev_info(ts->dev, "%s resume touch: lpm %d\n",
		ts->chip_name, chip->enable_lpm);

	if (chip->enable_lpm)
		lcd_mode = LCD_MODE_U0;

	ts->power_enabled = TS_POWER_ON;
	ret = lxs_hal_tc_driving(ts, lcd_mode);

	if (lcd_mode == LCD_MODE_U3)
		lxs_hal_tc_restore(ts);

	t_dev_info(ts->dev, "%s resume touch(%d): lcd_mode %d, driving_mode %d\n",
		ts->chip_name, ret, chip->lcd_mode, chip->driving_mode);

	return ret;
}

int lxs_hal_enable_touch(struct lxs_ts *ts, int state_mode)
{
	struct lxs_ts_chip *chip = &ts->chip;
	int ret = 0;

	if (ts->shutdown_called) {
		t_dev_err(ts->dev, "%s shutdown was called\n", __func__);
		return 0;
	}

	chip->prev_lpm = chip->enable_lpm;
	chip->prev_knock = chip->enable_knock;
	chip->prev_prox = chip->enable_prox;
	chip->prev_sleep = chip->enable_sleep;

	if (state_mode == TS_MODE_RESUME)
		ret = lxs_hal_resume_touch(ts);
	else
		ret = lxs_hal_suspend_touch(ts);

	return ret;
}

/*
 * U0   : echo 0 > enabled
 * U3   : echo 1 > enabled
 * STOP : echo 2 > enabled
 */
int lxs_hal_enable_device(struct lxs_ts *ts, int enable)
{
	struct lxs_ts_chip *chip = &ts->chip;
	int enable_swt = enable % 10;
	int mode;

	if (ts->shutdown_called) {
		t_dev_err(ts->dev, "%s shutdown was called\n", __func__);
		return 0;
	}

	if (ts->state_core != TS_CORE_NORMAL)
		return -EINVAL;

	switch (enable_swt) {
	case LCD_MODE_U0:
	case LCD_MODE_U3:
	case LCD_MODE_STOP:
		mode = enable_swt;
		break;
	default:
		return -EPERM;
	}

	if (touch_mode_not_allowed(ts, mode))
		return -EPERM;

	mutex_lock(&ts->lock);

	chip->enable = enable;

	t_dev_info(ts->dev, "touch enable device, %d\n", enable);

	mutex_unlock(&ts->lock);

	if (ts->mode_async)
		queue_delayed_work(ts->wq, &ts->enable_work, 0);
	else
		if (ts->enable_work.work.func)
			ts->enable_work.work.func(&ts->enable_work.work);

	return 0;
}

static int __lxs_hal_enable_work_func(struct lxs_ts *ts)
{
	struct lxs_ts_chip *chip = &ts->chip;
	int do_reset = !!(ts->power_enabled == TS_POWER_OFF);
	int enable_swt = chip->enable % 10;
	int enable_ext = chip->enable / 10;
	int mode = LCD_MODE_U3;
	int run_init = 0;
	int changed = 0;
	int err = 0;
	int ret = 0;

	cancel_delayed_work_sync(&ts->init_work);
	cancel_delayed_work_sync(&ts->upgrade_work);

	mutex_lock(&ts->lock);

	switch (enable_ext) {
	case 1:
		if(ts->state_mode == TS_MODE_SUSPEND) {
			t_dev_info(ts->dev, "%s: enable(%d) blocked in suspend\n",
				__func__, chip->enable);
			goto out;
		}
		break;
	default:
		if (enable_ext)
			goto out;
		break;
	}

	if (atomic_read(&chip->init) != TC_IC_INIT_DONE)
		do_reset |= BIT(1);

	switch (enable_swt) {
	case LCD_MODE_U0:
	case LCD_MODE_U3:
	case LCD_MODE_STOP:
		mode = enable_swt;
		break;
	default:
		goto out;
	}

	if (chip->lcd_mode != mode)
		changed |= BIT(0);

	switch (mode) {
	case LCD_MODE_U0:
		if (chip->prev_knock != chip->enable_knock)
			changed |= BIT(1);

		if (chip->prev_prox != chip->enable_prox)
			changed |= BIT(2);

		if (chip->prev_sleep != chip->enable_sleep)
			changed |= BIT(3);
		break;
	case LCD_MODE_U3:
		if (chip->prev_lpm != chip->enable_lpm)
			changed |= BIT(4);
		break;
	}

	t_dev_info(ts->dev, "%s: lcd %d(%d, %d, %d), c 0x%X(%d)\n",
		__func__, mode, chip->driving_mode, chip->lcd_mode, chip->prev_lcd_mode,
		changed, do_reset);

	if (!changed && !do_reset)
		goto out;

	if (touch_mode_not_allowed(ts, mode))
		goto out;

	err = lxs_ts_pw_lock(ts, INCELL_DISPLAY_POWER_LOCK);
	if (err) {
		t_dev_err(ts->dev, "%s: failed lock power, %d\n", __func__, err);
		goto out;
	}

	ts->power_enabled = TS_POWER_ON;

	chip->prev_lcd_mode = chip->lcd_mode;
	chip->lcd_mode = mode;

	switch (mode) {
	case LCD_MODE_STOP:
		if (do_reset)
			break;

		ret = lxs_hal_tc_driving(ts, mode);
		if (ret < 0) {
			t_dev_err(ts->dev, "stop control failed, retry init\n");
			run_init = 1;
		}
		break;

	case LCD_MODE_U0:
		t_dev_info(ts->dev, "touch disable work(suspend) start\n");

		ts->state_mode = TS_MODE_SUSPEND;

		run_init = !!do_reset;
		if (!run_init)
			lxs_hal_enable_touch(ts, TS_MODE_SUSPEND);

		t_dev_info(ts->dev, "touch disable work(suspend) done(%d)\n", ret);
		break;

	default:
		t_dev_info(ts->dev, "touch enable work(resume) start\n");

		ts->state_mode = TS_MODE_RESUME;

		run_init = 1;

		t_dev_info(ts->dev, "touch enable work(resume) done(%d)\n", ret);
		break;
	}

	err = lxs_ts_pw_lock(ts, INCELL_DISPLAY_POWER_UNLOCK);
	if (err)
		t_dev_err(ts->dev, "%s: failed unlock power, %d\n", __func__, err);

	if (run_init) {
		int reset_ctrl = (ts->mode_async) ? TC_HW_RESET_ASYNC : TC_HW_RESET_SYNC;
		lxs_hal_reset(ts, reset_ctrl, 0);
	} else {
		lxs_ts_release_all_event(ts);
	}

out:
	if (!chip->reset_is_on_going)
		lxs_ts_irq_control(ts, 1);

	mutex_unlock(&ts->lock);

	return run_init;
}

static void lxs_hal_enable_work_func(struct work_struct *work)
{
	struct lxs_ts *ts = container_of(to_delayed_work(work), struct lxs_ts, enable_work);
	int ret;

	lxs_ts_watchdog_stop(ts);

	ret = __lxs_hal_enable_work_func(ts);

	if (!ret)
		lxs_ts_watchdog_run(ts);
}

int lxs_hal_touch_mode(struct lxs_ts *ts, int state_mode)
{
	struct lxs_ts_chip *chip = &ts->chip;
	static int smode = TS_MODE_RESUME;
	int do_reset = !!(ts->power_enabled == TS_POWER_OFF);
	int mode = LCD_MODE_U3;
	int chk_mode = 0;
	int keep_sleep = 0;
	int changed = 0;
	int err = 0;

	if (ts->shutdown_called) {
		t_dev_err(ts->dev, "%s shutdown was called\n", __func__);
		return 0;
	}

	if ((state_mode == TS_MODE_SUSPEND) || chip->enable_lpm)
		lxs_ts_watchdog_stop(ts);

	mutex_lock(&ts->lock);

	if (atomic_read(&chip->init) != TC_IC_INIT_DONE)
		do_reset |= BIT(1);

	switch (state_mode) {
	case TS_MODE_RESUME:
		/* Normal or AOD */
		chk_mode = (chip->enable_lpm) ? chip->lcd_mode : chip->driving_mode;
		if ((state_mode != smode) || (mode != chk_mode)) {
			do_reset |= BIT(2);
			changed |= BIT(0);
			break;
		}

		if (chip->prev_lpm != chip->enable_lpm)
			changed |= BIT(4);
		break;
	case TS_MODE_SUSPEND:
		/* Off or Off(EWU) */
		if (chip->enable_sleep && (ts->power_enabled == TS_POWER_OFF)) {
			if (!chip->enable_knock || chip->enable_prox) {
				keep_sleep = 1;
				do_reset = 0;
				break;
			}
		}

		mode = LCD_MODE_U0;
		if ((state_mode != smode) || (mode != chip->driving_mode)) {
			changed |= BIT(0);
			break;
		}

		if (chip->prev_knock != chip->enable_knock)
			changed |= BIT(1);

		if (chip->prev_prox != chip->enable_prox)
			changed |= BIT(2);

		if (chip->prev_sleep != chip->enable_sleep)
			changed |= BIT(3);
		break;
	}

	t_dev_info(ts->dev, "%s: state %d(%d), lcd %d(%d, %d, %d), c 0x%X(%d)\n",
		__func__, state_mode, smode,
		mode, chip->driving_mode, chip->lcd_mode, chip->prev_lcd_mode,
		changed, do_reset);

	if (!changed && !do_reset)
		goto out;

	if (touch_mode_not_allowed(ts, mode))
		goto out;

	if (do_reset) {
		int reset_ctrl = (ts->mode_async) ? TC_HW_RESET_ASYNC : TC_HW_RESET_SYNC;
		chip->prev_lcd_mode = chip->lcd_mode;
		chip->lcd_mode = mode;

		ts->state_mode = state_mode;
		smode = state_mode;

		ts->power_enabled = TS_POWER_ON;

		lxs_hal_reset(ts, reset_ctrl, 0);
		goto out;
	}

	err = lxs_ts_pw_lock(ts, INCELL_DISPLAY_POWER_LOCK);
	if (err) {
		t_dev_err(ts->dev, "%s: failed lock power, %d\n", __func__, err);
		goto out;
	}

	chip->prev_lcd_mode = chip->lcd_mode;
	chip->lcd_mode = mode;

	ts->state_mode = state_mode;
	smode = state_mode;

	lxs_hal_enable_touch(ts, state_mode);

	err = lxs_ts_pw_lock(ts, INCELL_DISPLAY_POWER_UNLOCK);
	if (err)
		t_dev_err(ts->dev, "%s: failed unlock power, %d\n", __func__, err);

out:
	if (!chip->reset_is_on_going)
		lxs_ts_irq_control(ts, 1);

	mutex_unlock(&ts->lock);

	return err;
}

#define lxs_chk_sts_snprintf(_buf, _buf_max, _size, _fmt, _args...) \
		({	\
			int _n_size = 0;	\
			_n_size = __lxs_snprintf(_buf, _buf_max, _size, _fmt, ##_args);	\
			_n_size;	\
		})

static int lxs_hal_check_fault_type(struct lxs_ts *ts)
{
	/* TBD */
	return NON_FAULT_INT;
}

static int lxs_hal_check_status_type_x(struct lxs_ts *ts,
				u32 status, u32 ic_status, int irq)
{
	struct lxs_ts_chip *chip = &ts->chip;
	struct lxs_hal_status_filter *filter = chip->status_filter;
	struct lxs_hal_status_mask_bit *mask_bit = &chip->status_mask_bit;
	u32 irq_type = lxs_tc_sts_irq_type(status);
	u32 log_flag = 0;
	u32 esd_send = 0;
	u32 tc_disp_err = 0;
	u32 ic_abnormal, ic_error, ic_disp_err;
	int log_max = IC_CHK_LOG_MAX;
	char log[IC_CHK_LOG_MAX] = {0, };
	u32 check_mask = 0;
	u32 detected = 0;
	int type_error = 0;
	int type_esd = 0;
	int type_fault = 0;
	int fault_val = 0;
	int len = 0;
	int ret = 0;

	if (filter == NULL)
		return -EINVAL;

	if (ic_status & ~chip->status_mask_ic_valid) {
		t_dev_err(ts->dev, "[%d] status %08Xh, ic_status %08Xh, ic_status invalid\n",
			irq, status, ic_status);

		if (chip->lcd_mode != LCD_MODE_U0)
			return -ETDSENTESDIRQ;

		return -ERESTART;
	}

	while (1) {
		if (!filter->id || !filter->width)
			break;

		type_error = !!(filter->flag & STS_FILTER_FLAG_TYPE_ERROR);
		type_esd = !!(filter->flag & STS_FILTER_FLAG_ESD_SEND);
		type_fault = !!(filter->flag & STS_FILTER_FLAG_CHK_FAULT);

		check_mask = ((1<<filter->width)-1)<<filter->pos;

		detected = (type_error) ? (status & check_mask) : !(status & check_mask);

		if (check_mask && detected) {
			log_flag |= check_mask;
			esd_send |= (type_esd) ? check_mask : 0;

			fault_val = (type_fault) ? lxs_hal_check_fault_type(ts) : -1;

			if (type_fault && (fault_val >= 0)) {
				len += lxs_chk_sts_snprintf(log, log_max, len,
						"[b%d] %s(%Xh) ", filter->pos, filter->str, fault_val);
			} else {
				len += lxs_chk_sts_snprintf(log, log_max, len,
						"[b%d] %s ", filter->pos, filter->str);
			}

			tc_disp_err |= !!(filter->id == STS_ID_ERROR_DISP);
		}

		filter++;
	}

	if (chip->opt.t_oled && !tc_disp_err)
		esd_send = 0;

	if (log_flag)
		t_dev_err(ts->dev, "[%d] status %08Xh, ic_status %08Xh, (%08Xh) %s\n",
			irq, status, ic_status, log_flag, log);

	ic_abnormal = ic_status & chip->status_mask_ic_abnormal;
	ic_error = ic_status & chip->status_mask_ic_error;
	ic_disp_err = ic_status & chip->status_mask_ic_disp_err;
	ic_disp_err |= status & mask_bit->error_disp;

	if (ic_abnormal || ic_error || ic_disp_err) {
		u32 err_val[3] = { ic_abnormal, ic_error, ic_disp_err };
		char *err_str_ait[3] = {
			"esd",
			"watchdog",
			"dic"
		};
		char *err_str_oled[3] = {
			"spck",
			"watchdog",
			"dic"
		};
		char **err_str = (chip->opt.t_oled) ? err_str_oled : err_str_ait;
		int log_add = !log_flag;
		int err_pre, i;

		len = lxs_chk_sts_snprintf(log, log_max, 0, "[%d] ", irq);

		err_pre = 0;
		for (i = 0; i < ARRAY_SIZE(err_val) ; i++) {
			if (!err_val[i])
				continue;

			if (err_pre)
				len += lxs_chk_sts_snprintf(log, log_max, len, " & ");

			len += lxs_chk_sts_snprintf(log, log_max, len, "%s", err_str[i]);
			err_pre |= err_val[i];
		}

		if (log_add) {
			len += lxs_chk_sts_snprintf(log, log_max, len,
					" - status %08Xh, ic_status %08Xh",
					status, ic_status);
		}

		t_dev_err(ts->dev, "%s\n", log);

		if (chip->opt.t_oled)
			esd_send |= (ic_disp_err);
		else
			esd_send |= (ic_abnormal | ic_disp_err);

		if (!esd_send)
			ret = -ERESTART;	//touch reset
	}

	if (esd_send) {
		ret = -ERESTART;
		if (chip->lcd_mode != LCD_MODE_U0)
			return -ETDSENTESDIRQ;
	}

	if (ret == -ERESTART)
		return ret;

	/*
	 * Check interrupt_type[19:16] in TC_STATUS
	 */
	switch (irq_type) {
	case TC_STS_IRQ_TYPE_INIT_DONE:
		if (irq)
			t_dev_info(ts->dev, "[%d] TC Driving OK\n", irq);
		ret = -ERANGE;
		break;
	case TC_STS_IRQ_TYPE_REPORT:	/* Touch report */
		break;
	default:
		t_dev_dbg_irq_sts(ts->dev, "[%d] irq_type %Xh\n",
			irq, irq_type);
		ret = -ERANGE;
		break;
	}

	return ret;
}

static int lxs_hal_do_check_status(struct lxs_ts *ts,
				u32 status, u32 ic_status, int irq)
{
	struct lxs_ts_chip *chip = &ts->chip;
	u32 reset_clr_bit = 0;
	u32 logging_clr_bit = 0;
	u32 int_norm_mask = 0;
	u32 status_mask = 0;
	int ret_pre = 0;
	int ret = 0;

	irq &= 0x01;

	if (!status || !ic_status) {
		t_dev_err(ts->dev, "[%d] all low detected: status %08Xh, ic_status %08Xh\n",
			irq, status, ic_status);
		return -ERESTART;
	}

	if ((status == ~0) || (ic_status == ~0)) {
		t_dev_err(ts->dev, "[%d] all high detected: status %08Xh, ic_status %08Xh\n",
			irq, status, ic_status);
		return -ERESTART;
	}

	reset_clr_bit = chip->status_mask_reset;
	logging_clr_bit = chip->status_mask_logging;
	int_norm_mask = chip->status_mask_normal;

	status_mask = status ^ int_norm_mask;

	t_dev_dbg_irq_sts(ts->dev, "[%d] status %08Xh(%Xh), ic_status %08Xh\n",
		irq, status, status_mask, ic_status);

	if (status_mask & reset_clr_bit) {
		t_dev_err(ts->dev,
			"[%d] need reset : status %08Xh, ic_status %08Xh, chk %08Xh (%08Xh)\n",
			irq, status, ic_status, status_mask & reset_clr_bit, reset_clr_bit);
		ret_pre = -ERESTART;
	} else if (status_mask & logging_clr_bit) {
		t_dev_err(ts->dev,
			"[%d] need logging : status %08Xh, ic_status %08Xh, chk %08Xh (%08Xh)\n",
			irq, status, ic_status, status_mask & logging_clr_bit, logging_clr_bit);
		ret_pre = -ERANGE;
	}

	switch (chip->status_type) {
	case CHIP_STATUS_TYPE_2:
	case CHIP_STATUS_TYPE_1:
	case CHIP_STATUS_TYPE_0:
		ret = lxs_hal_check_status_type_x(ts, status, ic_status, irq);
		lxs_hal_check_dbg_report(ts, status, irq);
		break;
	default:
		t_dev_warn(ts->dev, "unknown status type, %d\n", chip->status_type);
		break;
	}

	if (ret == -ETDSENTESDIRQ)
		return ret;

	if (ret_pre) {
		if (ret != -ERESTART)
			ret = ret_pre;
	}

	return ret;
}

int lxs_hal_check_status(struct lxs_ts *ts)
{
	u32 ic_status = lxs_report_ic_status(ts);
	u32 status = lxs_report_tc_status(ts);

	return lxs_hal_do_check_status(ts, status, ic_status, 1);
}

int lxs_hal_irq_abs(struct lxs_ts *ts)
{
	struct lxs_ts_chip *chip = &ts->chip;
	struct lxs_hal_report_ops *ops = &chip->report_ops;
	char *name = ops->abs_name;
	int ret = 0;

	if (lxs_report_info(ts) == NULL) {
		t_dev_err(ts->dev, "irq_abs: report_info not defined\n");
		return -ESRCH;
	}

	if (ops->abs_pre)
		ret = ops->abs_pre(ts);
	else
		ret = __irq_abs_pre_default(ts);
	if (ret < 0) {
		t_dev_err(ts->dev, "irq_abs_pre%s failed, %d\n", name, ret);
		return ret;
	}

	if (ops->abs_data)
		ret = ops->abs_data(ts);
	else
		ret = __irq_abs_data_default(ts);
	if (ret < 0) {
		t_dev_err(ts->dev, "irq_abs_data%s failed, %d\n", name, ret);
		return ret;
	}

	return 0;
}

static int lxs_hal_irq_lpwg_base(struct lxs_ts *ts, int type)
{
	struct lxs_ts_chip *chip = &ts->chip;
	u32 rdata[8] = { 0, };
	int i;
	int ret = 0;

	switch (type) {
	case TS_KNOCK:
		t_dev_info(ts->dev, "LPWG: KNOCK\n");

		memcpy(&rdata, lxs_report_data(ts), sizeof(u32) * TS_MAX_LPWG_CODE);

		for (i = 0; i < TS_MAX_LPWG_CODE; i++) {
			chip->knock[i].x = rdata[i] & 0xffff;
			chip->knock[i].y = (rdata[i] >> 16) & 0xffff;

			t_dev_info(ts->dev, "KNOCK data %d, %d\n",
				chip->knock[i].x, chip->knock[i].y);
		}

		ts->intr_status = TS_IRQ_KNOCK;
		break;
	case TS_SWIPE:
		t_dev_info(ts->dev, "LPWG: SWIPE\n");

		memcpy(&rdata, lxs_report_data(ts), sizeof(u32) * (TS_MAX_SWIPE_CODE + 1));

		for (i = 0; i < TS_MAX_SWIPE_CODE; i++) {
			chip->swipe[i].x = rdata[i] & 0xffff;
			chip->swipe[i].y = (rdata[i] >> 16) & 0xffff;

			t_dev_info(ts->dev, "SWIPE data %d, %d\n",
				chip->swipe[i].x, chip->swipe[i].y);
		}
		chip->swipe_time = rdata[i] & 0xFFFF;
		t_dev_info(ts->dev, "SWIPE time %d\n", chip->swipe_time);

		ts->intr_status = TS_IRQ_GESTURE;
		break;
	default:
	//	t_dev_warn(dev, "unkown lpwg: %d\n", type);
		break;
	}

	return ret;
}

static int __irq_lpwg_default(struct lxs_ts *ts)
{
	u32 type = lxs_report_info_wakeup_type(ts);

	if (!type || (type > TS_SWIPE)) {
		t_dev_err(ts->dev, "LPWG: unknown type, %d\n", type);

		return -EINVAL;
	}

	return lxs_hal_irq_lpwg_base(ts, type);
}

int lxs_hal_irq_lpwg(struct lxs_ts *ts)
{
	struct lxs_ts_chip *chip = &ts->chip;
	struct lxs_hal_report_ops *ops = &chip->report_ops;
	char *name = ops->lpwg_name;
	int ret = 0;

	if (lxs_report_info(ts) == NULL) {
		t_dev_err(ts->dev, "irq_abs: report_info not defined\n");
		return -ESRCH;
	}

	if (ops->lpwg_data)
		ret = ops->lpwg_data(ts);
	else
		ret = __irq_lpwg_default(ts);
	if (ret < 0) {
		t_dev_err(ts->dev, "irq_lpwg_data%s failed, %d\n", name, ret);
	}

	return ret;
}

static int lxs_hal_irq_get_report(struct lxs_ts *ts)
{
	struct lxs_ts_chip *chip = &ts->chip;
	struct lxs_hal_reg *reg = chip->reg;
	struct lxs_hal_report_ops *ops = &chip->report_ops;
	int is_flex_report = chip->opt.f_flex_report;
	u32 addr = reg->tc_ic_status;
	char *buf = (char *)lxs_report_info(ts);
	int size = 0;
	int pkt_unit = 0;
	int pkt_cnt = 0;
	int wakeup_type = 0;
	int touch_cnt = 0;
	int ret = 0;

	if (buf == NULL) {
		t_dev_err(ts->dev, "get_report: report_info not defined\n");
		return -ESRCH;
	}

	pkt_unit = sizeof(struct lxs_hal_touch_data);
	pkt_cnt = TC_REPORT_BASE_PKT;
	touch_cnt = ts->max_finger - pkt_cnt;

	size = (TC_REPORT_BASE_HDR<<2);
	size += (pkt_unit * pkt_cnt);

	/*
	 * Dynamic read access
	 */
	if (is_flex_report) {
		t_dev_dbg_irq(ts->dev, "get dynamic report%s, 0x%04X %d\n",
			ops->abs_name, addr, size);

		ret = lxs_hal_reg_read(ts, addr, (void *)buf, size);
		if (ret < 0)
			return ret;

		wakeup_type = lxs_report_info_wakeup_type(ts);
		touch_cnt = lxs_report_info_touch_cnt(ts);

		/* No need to read more */
		if (wakeup_type != TS_ABS_MODE)
			return 0;

		/* No need to read more */
		if ((touch_cnt <= pkt_cnt) || (touch_cnt > ts->max_finger))
			return 0;

		addr += (size>>2);
		buf += size;
		size = 0;

		touch_cnt -= pkt_cnt;
	}

	size += (pkt_unit * touch_cnt);

	ret = lxs_hal_reg_read(ts, addr, (void *)buf, size);

	return ret;
}

static int lxs_hal_irq_exception(struct lxs_ts *ts)
{
	struct lxs_ts_chip *chip = &ts->chip;
	char *title = NULL;

	if (ts->suspend_is_on_going) {
		title = "suspend";
		goto out;
	}

	if (atomic_read(&chip->init) == TC_IC_INIT_NEED) {
		title = "Not Ready, Need IC init (irq)";
		goto out;
	}

	if (!chip->status_type) {
		title = "No status type";
		goto out;
	}

	if (!chip->report_type) {
		title = "No report type";
		goto out;
	}

	return 0;

out:
	t_dev_warn(ts->dev, "irq_exception: %s\n", title);

	return 1;
}

static int lxs_hal_irq_skip_event(struct lxs_ts *ts)
{
	struct lxs_ts_chip *chip = &ts->chip;
	char *title = NULL;

	switch (chip->driving_mode) {
	case LCD_MODE_STOP:
		title = "stop state";
		goto out;
	}

	return 0;

out:
	t_dev_info(ts->dev, "skip event - %s\n", title);
	lxs_ts_release_all_event(ts);

	return 1;
}

int lxs_hal_irq_handler(struct lxs_ts *ts)
{
	struct lxs_ts_chip *chip = &ts->chip;
	struct lxs_hal_ops_quirk *ops_quirk = &chip->ops_quirk;
	int wakeup_type = 0;
	int touch_cnt = 0;
	int ret = 0;

	if (ops_quirk->irq_handler)
		return ops_quirk->irq_handler(ts);

	if (lxs_hal_irq_exception(ts))
		return 0;

	ret = lxs_hal_irq_get_report(ts);
	if (ret < 0)
		goto out;

	ret = lxs_hal_check_status(ts);
	if (ret < 0)
		goto out;

	wakeup_type = lxs_report_info_wakeup_type(ts);
	touch_cnt = lxs_report_info_touch_cnt(ts);

	t_dev_dbg_irq(ts->dev, "hal irq handler: wakeup_type %d\n", wakeup_type);

	if (lxs_hal_irq_skip_event(ts))
		goto out;

	if (wakeup_type == TS_ABS_MODE) {
		ret = lxs_hal_irq_abs(ts);
		if (ret) {
			t_dev_err(ts->dev, "lxs_hal_irq_abs failed(%d), %d\n",
				touch_cnt, ret);
			goto out;
		}
	} else {
		ret = lxs_hal_irq_lpwg(ts);
		if (ret) {
			t_dev_err(ts->dev, "lxs_hal_irq_lpwg failed, %d\n", ret);
			goto out;
		}
	}

out:
	return ret;
}

static void __cmd_set_log(struct lxs_ts *ts)
{
	struct lxs_ts_chip *chip = &ts->chip;
	int *tc_cmd_table = chip->tc_cmd_table;
	char *mode_str = NULL;
	char *ext_str = NULL;
	int ctrl = 0;
	int i = 0;

	t_dev_info(ts->dev, "[tc cmd set] (mode bit %04Xh)\n",
		ts->mode_allowed);

	for (i = 0; i < LCD_MODE_MAX; i++) {
		mode_str = (char *)lxs_lcd_driving_mode_str(i);
		ctrl = tc_cmd_table[i];

		if (ctrl < 0)
			ext_str = "(not granted)";
		else if (!touch_mode_allowed(ts, i))
			ext_str = "(not allowed)";
		else
			ext_str = "";

		t_dev_info(ts->dev, " %04Xh [%-4s] %s\n",
			ctrl, mode_str, ext_str);
	}
}

static void __cmd_set_type_a(struct lxs_ts *ts)
{
	struct lxs_ts_chip *chip = &ts->chip;
	int *tc_cmd_table = chip->tc_cmd_table;
	u32 ux_cmd = LCD_TYPE_A_U3_FINGER;

	tc_cmd_table[LCD_MODE_U0] = ux_cmd;
	tc_cmd_table[LCD_MODE_U3] = 0x300 | ux_cmd;
	tc_cmd_table[LCD_MODE_STOP] = LCD_TYPE_A_STOP;
}

static void __cmd_set_type_1(struct lxs_ts *ts)
{
	struct lxs_ts_chip *chip = &ts->chip;
	int *tc_cmd_table = chip->tc_cmd_table;

	tc_cmd_table[LCD_MODE_U0] = 0x1;
	tc_cmd_table[LCD_MODE_U3] = 0x301;
	tc_cmd_table[LCD_MODE_STOP] = TC_DRIVE_CTL_STOP;
}

static void __cmd_set_default(struct lxs_ts *ts)
{
	struct lxs_ts_chip *chip = &ts->chip;
	int *tc_cmd_table = chip->tc_cmd_table;
	int ctrl;

	ctrl = (TC_DRIVE_CTL_DISP_U3 | TC_DRIVE_CTL_MODE_6LHB | TC_DRIVE_CTL_START);

	if (ts->plat_data->use_tc_vblank)
		ctrl &= ~TC_DRIVE_CTL_MODE_6LHB;

	if (ts->plat_data->use_tc_doze)
		ctrl |= TC_DRIVE_CTL_MODE_DOZE;

	tc_cmd_table[LCD_MODE_U0] = TC_DRIVE_CTL_START;
	tc_cmd_table[LCD_MODE_U3] = ctrl;
	tc_cmd_table[LCD_MODE_STOP] = TC_DRIVE_CTL_STOP;
}

static void lxs_hal_chipset_cmd_set(struct lxs_ts *ts)
{
	struct lxs_ts_chip *chip = &ts->chip;
	int t_tc_cmd = chip->opt.t_tc_cmd;
	int i;

	for (i = 0; i < ARRAY_SIZE(chip->tc_cmd_table); i++)
		chip->tc_cmd_table[i] = -1;

	switch (t_tc_cmd) {
	case 0xA:
		__cmd_set_type_a(ts);
		break;
	case 1:
		__cmd_set_type_1(ts);
		break;
	default:
		__cmd_set_default(ts);
		break;
	}

	__cmd_set_log(ts);
}

static void lxs_hal_chipset_option_log(struct lxs_ts *ts)
{
	struct lxs_ts_chip *chip = &ts->chip;
	struct lxs_ts_chip_opt *opt = &chip->opt;
	struct chip_options {
		const char *fmt;
		int value;
		int chk;
	} *options, lists[] = {
		{	" f_flex_report   : %d\n", opt->f_flex_report, 0	},
		{	" f_glove_en      : %d\n", opt->f_glove_en, 0	},
		{	" f_grab_en       : %d\n", opt->f_grab_en, 0	},
		{	" f_dbg_report    : %d\n", opt->f_dbg_report, 0	},
		/* */
		{	" t_bus_opt       : %d\n", opt->t_bus_opt, 0	},
		{	" t_boot_mode     : %d\n", opt->t_boot_mode, 0	},
		{	" t_sts_mask      : %d\n", opt->t_sts_mask, 0	},
		{	" t_sw_rst        : %d\n", opt->t_sw_rst, TC_SW_RST_TYPE_NONE	},
		{	" t_clock         : %d\n", opt->t_clock, 0	},
		{	" t_bin           : %d\n", opt->t_bin, 0	},
		{	" t_oled          : %d\n", opt->t_oled, 0	},
		{	" t_tc_cmd        : %d\n", opt->t_tc_cmd, 0	},
		{	" t_tc_quirk      : %d\n", opt->t_tc_quirk, 0	},
		{ NULL, 0, 0	},
	};

	options = lists;

	t_dev_info(ts->dev, "[opt summary]\n");
	while (options->fmt != NULL) {
		if (options->value != options->chk)
			t_dev_info(ts->dev, options->fmt, options->value);

		options++;
	}

	t_dev_info(ts->dev, " drv_reset_low   : %d ms\n", chip->drv_reset_low);
	t_dev_info(ts->dev, " drv_delay       : %d ms\n", chip->drv_delay);
	if (chip->drv_opt_delay)
		t_dev_info(ts->dev, " drv_opt_delay   : %d ms\n", chip->drv_opt_delay);
}

static void lxs_hal_chipset_option(struct lxs_ts *ts)
{
	struct lxs_ts_chip *chip = &ts->chip;
	struct lxs_ts_chip_opt *opt = &chip->opt;
	int is_i2c = !!(ts->bus_type == BUS_I2C);

	chip->report_type = CHIP_REPORT_TYPE_0;
	chip->status_type = CHIP_STATUS_TYPE_1;

	chip->drv_reset_low = 5;
	chip->drv_delay = 30;
	chip->drv_opt_delay = 0;

	opt->f_flex_report = is_i2c;

	opt->t_sw_rst = TC_SW_RST_TYPE_NONE;

	switch (ts->chip_type) {
	case CHIP_SW82907:
		opt->f_grab_en = 1;
		opt->t_sw_rst = 5;
		opt->t_clock = 4;
		opt->t_oled = 2;
		opt->t_tc_cmd = 0xA;
		opt->t_tc_quirk = 1;
		break;
	}

	lxs_hal_chipset_option_log(ts);
}

static void lxs_hal_chipset_quirk_reset(struct lxs_ts *ts)
{
	struct lxs_ts_chip *chip = &ts->chip;

	if (!ts->plat_data->use_skip_reset)
		return;

	if (ts->fquirks->gpio_set_reset != NULL)
		return;

	if (ts->chip.opt.t_sw_rst > TC_SW_RST_TYPE_MAX) {
		t_dev_warn(ts->dev, "sw reset not supported\n");
		return;
	}

	t_dev_info(ts->dev, "hw_reset_quirk activated\n");

	chip->ops_quirk.hw_reset = lxs_hal_hw_reset_quirk;
}

static void lxs_hal_chipset_setup(struct lxs_ts *ts)
{
	lxs_hal_chipset_option(ts);

	lxs_hal_chipset_cmd_set(ts);

	lxs_hal_chipset_quirk_reset(ts);

	lxs_hal_upgrade_setup(ts);
}

static void lxs_hal_rinfo_channel(struct lxs_ts *ts)
{
	struct lxs_ts_chip *chip = &ts->chip;
	struct lxs_hal_reg *reg = chip->reg;
	u32 channel;
	u32 tx, rx;
	int ret = 0;

	if (lxs_addr_is_invalid(reg->channel_info))
		return;

	ret = lxs_hal_read_value(ts, reg->channel_info, &channel);
	if (ret < 0) {
		t_dev_err(ts->dev, "rinfo: failed to get channel info, %d\n", ret);
		return;
	}

	tx = channel & 0xFF;
	rx = (channel >> 16) & 0xFF;

	chip->tx_count = tx;
	chip->rx_count = rx;

	if (!tx || !rx) {
		t_dev_err(ts->dev, "rinfo: invalid tx/rx: tx %d, rx %d\n", tx, rx);
		return;
	}

	t_dev_info(ts->dev, "rinfo: tx %d, rx %d\n", tx, rx);
}

static void lxs_hal_rinfo_resolution(struct lxs_ts *ts)
{
	struct lxs_ts_chip *chip = &ts->chip;
	struct lxs_hal_reg *reg = chip->reg;
	u32 resolution;
	u32 max_x, max_y;
	int ret = 0;

	if (lxs_addr_is_invalid(reg->res_info))
		return;

	ret = lxs_hal_read_value(ts, reg->res_info, &resolution);
	if (ret < 0) {
		t_dev_info(ts->dev, "failed to get resolution info, %d\n", ret);
		return;
	}

	max_x = resolution & 0xFFFF;
	max_y = resolution >> 16;

	chip->res_x = max_x;
	chip->res_y = max_y;

	if (!max_x || !max_y) {
		t_dev_err(ts->dev, "rinfo: invalid resolution: x %d, y %d\n", max_x, max_y);
		return;
	}

	ts->plat_data->max_x = max_x;
	ts->plat_data->max_y = max_y;

	t_dev_info(ts->dev, "rinfo: max_x %d, max_y %d\n", max_x, max_y);
}

#if defined(__LXS_SUPPORT_RINFO)
static void lxs_hal_rinfo_base(struct lxs_ts *ts)
{
	struct lxs_ts_chip *chip = &ts->chip;
	struct lxs_hal_reg *reg = chip->reg;
	u32 info_data;
	u32 temp, tmsb, tlsb;

	if (lxs_addr_is_invalid(reg->info_ptr))
		return;

	lxs_hal_read_value(ts, reg->info_ptr, &info_data);
	chip->iinfo_addr = info_data & 0xFFF;			//0x642
	chip->rinfo_addr = (info_data >> 12) & 0xFFF;	//0x64A

	t_dev_info(ts->dev, "reg_info_addr: 0x%X\n", chip->rinfo_addr);

	lxs_hal_reg_read(ts, chip->rinfo_addr, chip->rinfo_data, sizeof(chip->rinfo_data));

	temp = chip->rinfo_data[7];
	tlsb = temp & 0xFFFF;
	tmsb = temp >> 16;

	for (temp = 0 ; temp < ARRAY_SIZE(chip->rinfo_data); temp++)
		t_dev_info(ts->dev, "rinfo_data[%d] = 0x%08X\n", temp, chip->rinfo_data[temp]);

	if ((tlsb != chip->iinfo_addr) || (tmsb != chip->rinfo_addr)) {
		t_dev_err(ts->dev, "param invalid: 0x%08X vs. 0x%08X\n", info_data, temp);
		return;
	}

	chip->rinfo_ok = 1;
}
#endif

static int lxs_hal_hw_init(struct lxs_ts *ts)
{
	int ret = 0;

	ret = lxs_hal_ic_test(ts);
	if (ret < 0)
		return ret;

#if defined(__LXS_SUPPORT_RINFO)
	lxs_hal_rinfo_base(ts);
#endif

	lxs_hal_rinfo_resolution(ts);

	lxs_hal_rinfo_channel(ts);

	return 0;
}

void lxs_hal_activate(struct lxs_ts *ts)
{
	struct lxs_ts_chip *chip = &ts->chip;

	lxs_hal_chk_report_type(ts);
	lxs_hal_chk_status_type(ts);

	chip->driving_mode = LCD_MODE_STOP;
	chip->lcd_mode = LCD_MODE_U3;
	chip->prev_lcd_mode = LCD_MODE_U3;
	chip->enable = 1;

	chip->grab = 1;

	//for SOMC
	chip->enable_sleep = 1;
}

void lxs_hal_deactivate(struct lxs_ts *ts)
{
	lxs_hal_deep_sleep(ts);

	lxs_hal_del_sysfs(ts);

	lxs_hal_clr_report_type(ts);
	lxs_hal_clr_status_type(ts);
}

int lxs_hal_probe(struct lxs_ts *ts)
{
	struct lxs_ts_chip *chip = &ts->chip;
	int ret = 0;

	chip->reg = &chip->reg_cur;

	lxs_hal_reg_setup(ts);

	lxs_hal_chipset_setup(ts);

	INIT_DELAYED_WORK(&ts->enable_work, lxs_hal_enable_work_func);

	lxs_ts_power_reset(ts, TS_POWER_ON);
	lxs_ts_delay(ts->hw_reset_delay);

	ret = lxs_hal_hw_init(ts);
	if (ret) {
		lxs_ts_power_reset(ts, TS_POWER_OFF);
		t_dev_err(ts->dev, "LXS(%s) hal hw_init failed, %d\n",
			ts->chip_name, ret);
		return ret;
	}

	return 0;
}

void lxs_hal_remove(struct lxs_ts *ts)
{
	cancel_delayed_work_sync(&ts->enable_work);

	lxs_ts_power_reset(ts, TS_POWER_OFF);
}

#if defined(__LXS_SUPPORT_WATCHDOG)
static int lxs_hal_watchdog_chk_id(struct lxs_ts *ts)
{
	struct lxs_ts_chip *chip = &ts->chip;
	struct lxs_hal_reg *reg = chip->reg;
	struct lxs_hal_fw_info *fw = &chip->fw;
	u32 chip_id;
	int ret = 0;

	ret = lxs_hal_read_value(ts, reg->spr_chip_id, &chip_id);
	if (ret < 0) {
		t_dev_err(ts->dev, "%s: failed to read chip class\n", __func__);
		return ret;
	}

	if (fw->chip_id_raw != chip_id) {
		t_dev_err(ts->dev, "%s: chip class error: 0x%08X != 0x%08X\n",
			__func__, fw->chip_id_raw, chip_id);
		return -EINVAL;
	}

	t_dev_dbg_trace(ts->dev, "%s: ok\n", __func__);

	return 0;
}

static int lxs_hal_watchdog_chk_status(struct lxs_ts *ts)
{
	struct lxs_ts_chip *chip = &ts->chip;
	struct lxs_hal_reg *reg = chip->reg;
	u32 ic_status, status;
	int ret = 0;

	ret = lxs_hal_read_value(ts, reg->tc_ic_status, &ic_status);
	if (ret < 0) {
		t_dev_err(ts->dev, "%s: failed to read ic status\n", __func__);
		return ret;
	}

	ret = lxs_hal_read_value(ts, reg->tc_status, &status);
	if (ret < 0) {
		t_dev_err(ts->dev, "%s: failed to read tc status\n", __func__);
		return ret;
	}

	status |= chip->status_mask_bit.valid_irq_pin;
	ret = lxs_hal_do_check_status(ts, status, ic_status, 0);
	switch (ret) {
	case -ERESTART:
	case -ETDSENTESDIRQ:
		return ret;
	}

	t_dev_dbg_trace(ts->dev, "%s: ok\n", __func__);

	return 0;
}

const char *lxs_hal_watchdog_sts(struct lxs_ts *ts)
{
	struct lxs_ts_chip *chip = &ts->chip;

	if (ts->shutdown_called)
		return "shutdown called";

	if (ts->power_enabled != TS_POWER_ON)
		return "not power on";

	if (ts->state_core != TS_CORE_NORMAL)
		return "state_core not normal";

	if (ts->state_sleep != TS_IC_NORMAL)
		return "state_sleep not IC normal";

	if (ts->state_pm != TS_DEV_PM_RESUME)
		return "state_pm not TS_DEV_PM_RESUME";

	if (chip->reset_is_on_going)
		return "reset is ongoing";

	if (atomic_read(&chip->boot) == TC_IC_BOOT_FAIL)
		return "boot failed";

	if (atomic_read(&chip->init) != TC_IC_INIT_DONE)
		return "not ready, need IC init";

	if (chip->lcd_mode != LCD_MODE_U3)
		return "lcd_mode not NP";

	if (chip->driving_mode != LCD_MODE_U3)
		return "driving mode not NP";

	return NULL;
}

int lxs_hal_watchdog_work(struct lxs_ts *ts)
{
	int ret = 0;

	ret = lxs_hal_watchdog_chk_id(ts);
	if (ret < 0)
		return ret;

	ret = lxs_hal_watchdog_chk_status(ts);
	if (ret < 0)
		return ret;

	return 0;
}
#endif

#if defined(__LXS_SUPPORT_FILTER_CON)
struct lxs_ts_filter_ctrl {
	char *name;
	u32 addr;
	int *value;
};

int lxs_hal_chk_filter(struct lxs_ts *ts, int idx, int wr)
{
	struct lxs_ts_filter_ctrl filter_table_wr[] = {
		{	.name = "blend_filter",
			.addr = 0xD10,
			.value = &ts->blend_filter,
		},
		{	.name = "stop_filter",
			.addr = 0xD11,
			.value = &ts->stop_filter,
		},
	};
	struct lxs_ts_filter_ctrl filter_table_rd[] = {
		{	.name = "wet_mode",
			.addr = 0xD12,
			.value = &ts->wet_mode,
		},
	};
	struct lxs_ts_filter_ctrl *filter;
	int idx_ovr = (wr) ? ARRAY_SIZE(filter_table_wr) : ARRAY_SIZE(filter_table_rd);
	int ret = 0;

	if ((idx < 0) || (idx >= idx_ovr))
		return -EINVAL;

	if (wr) {
		filter = &filter_table_wr[idx];

		ret = lxs_hal_write_value(ts, filter->addr, *filter->value);
		if (ret < 0) {
			t_dev_err(ts->dev, "%s(set): failed to write\n", filter->name);
			return ret;
		}

		t_dev_info(ts->dev, "%s(set): %d\n", filter->name, *filter->value);
	} else {
		filter = &filter_table_rd[idx];

		ret = lxs_hal_read_value(ts, filter->addr, filter->value);
		if (ret < 0) {
			t_dev_err(ts->dev, "%s(get): failed to read\n", filter->name);
			return ret;
		}

		t_dev_info(ts->dev, "%s(get): %d\n", filter->name, *filter->value);
	}

	return 0;
}
#endif	/* __LXS_SUPPORT_FILTER_CON */
