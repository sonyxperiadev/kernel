/* drivers/input/touchscreen/sec_ts_fw.c
 *
 * Copyright (C) 2015 Samsung Electronics Co., Ltd.
 * http://www.samsungsemi.com/
 * Copyright (C) 2018 Sony Mobile Communications Inc.
 *
 * Core file for Samsung TSC driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "include/sec_ts.h"

#define SEC_TS_FW_BLK_SIZE		256

enum {
	BUILT_IN = 0,
	UMS,
	BL,
	FFU,
};

static int sec_ts_enter_fw_mode(struct sec_ts_data *ts)
{
	int ret;
	u8 fw_update_mode_passwd[] = {0x55, 0xAC};
	u8 fw_status;
	u8 id[3];

	ret = ts->sec_ts_i2c_write(ts, SEC_TS_CMD_ENTER_FW_MODE, fw_update_mode_passwd, sizeof(fw_update_mode_passwd));
	sec_ts_delay(20);
	if (ret < 0) {
		input_err(true, &ts->client->dev, "%s: write fail, enter_fw_mode\n", __func__);
		return 0;
	}

	input_info(true, &ts->client->dev, "%s: write ok, enter_fw_mode - 0x%x 0x%x 0x%x\n",
			__func__, SEC_TS_CMD_ENTER_FW_MODE, fw_update_mode_passwd[0], fw_update_mode_passwd[1]);

	ret = ts->sec_ts_i2c_read(ts, SEC_TS_READ_BOOT_STATUS, &fw_status, 1);
	if (ret < 0) {
		input_err(true, &ts->client->dev, "%s: read fail, read_boot_status\n", __func__);
		return 0;
	}
	if (fw_status != SEC_TS_STATUS_BOOT_MODE) {
		input_err(true, &ts->client->dev, "%s: enter fail! read_boot_status = 0x%x\n", __func__, fw_status);
		return 0;
	}

	input_info(true, &ts->client->dev, "%s: Success! read_boot_status = 0x%x\n", __func__, fw_status);

	sec_ts_delay(10);

	ret = ts->sec_ts_i2c_read(ts, SEC_TS_READ_ID, id, 3);
	if (ret < 0) {
		input_err(true, &ts->client->dev, "%s: read id fail\n", __func__);
		return 0;
	}

	ts->boot_ver[0] = id[0];
	ts->boot_ver[1] = id[1];
	ts->boot_ver[2] = id[2];

	ts->flash_page_size = SEC_TS_FW_BLK_SIZE_DEFAULT;

	input_info(true, &ts->client->dev, "%s: read_boot_id = %02X%02X%02X\n", __func__, id[0], id[1], id[2]);

	return 1;
}

static int sec_ts_sw_reset(struct sec_ts_data *ts)
{
	int ret;

	ret = ts->sec_ts_i2c_write(ts, SEC_TS_CMD_SW_RESET, NULL, 0);
	if (ret < 0) {
		input_err(true, &ts->client->dev, "%s: write fail, sw_reset\n", __func__);
		return 0;
	}

	sec_ts_delay(100);

	ret = sec_ts_wait_for_ready(ts, SEC_TS_ACK_BOOT_COMPLETE);
	if (ret < 0) {
		input_err(true, &ts->client->dev, "%s: time out\n", __func__);
		return 0;
	}

	input_info(true, &ts->client->dev, "%s: sw_reset\n", __func__);

	/* Sense_on */
	ret = ts->sec_ts_i2c_write(ts, SEC_TS_CMD_SENSE_ON, NULL, 0);
	if (ret < 0) {
		input_err(true, &ts->client->dev, "%s: write fail, Sense_on\n", __func__);
		return 0;
	}

	return ret;
}

static void sec_ts_save_version_of_bin(struct sec_ts_data *ts, const fw_header *fw_hd)
{
	ts->plat_data->img_version_of_bin[3] = ((fw_hd->img_ver >> 24) & 0xff);
	ts->plat_data->img_version_of_bin[2] = ((fw_hd->img_ver >> 16) & 0xff);
	ts->plat_data->img_version_of_bin[1] = ((fw_hd->img_ver >> 8) & 0xff);
	ts->plat_data->img_version_of_bin[0] = ((fw_hd->img_ver >> 0) & 0xff);

	ts->plat_data->core_version_of_bin[3] = ((fw_hd->fw_ver >> 24) & 0xff);
	ts->plat_data->core_version_of_bin[2] = ((fw_hd->fw_ver >> 16) & 0xff);
	ts->plat_data->core_version_of_bin[1] = ((fw_hd->fw_ver >> 8) & 0xff);
	ts->plat_data->core_version_of_bin[0] = ((fw_hd->fw_ver >> 0) & 0xff);

	ts->plat_data->config_version_of_bin[3] = ((fw_hd->para_ver >> 24) & 0xff);
	ts->plat_data->config_version_of_bin[2] = ((fw_hd->para_ver >> 16) & 0xff);
	ts->plat_data->config_version_of_bin[1] = ((fw_hd->para_ver >> 8) & 0xff);
	ts->plat_data->config_version_of_bin[0] = ((fw_hd->para_ver >> 0) & 0xff);

	input_info(true, &ts->client->dev, "%s: img_ver of bin = %x.%x.%x.%x\n", __func__,
			ts->plat_data->img_version_of_bin[0],
			ts->plat_data->img_version_of_bin[1],
			ts->plat_data->img_version_of_bin[2],
			ts->plat_data->img_version_of_bin[3]);

	input_info(true, &ts->client->dev, "%s: core_ver of bin = %x.%x.%x.%x\n", __func__,
			ts->plat_data->core_version_of_bin[0],
			ts->plat_data->core_version_of_bin[1],
			ts->plat_data->core_version_of_bin[2],
			ts->plat_data->core_version_of_bin[3]);

	input_info(true, &ts->client->dev, "%s: config_ver of bin = %x.%x.%x.%x\n", __func__,
			ts->plat_data->config_version_of_bin[0],
			ts->plat_data->config_version_of_bin[1],
			ts->plat_data->config_version_of_bin[2],
			ts->plat_data->config_version_of_bin[3]);
}

static int sec_ts_save_version_of_ic(struct sec_ts_data *ts)
{
	u8 img_ver[4] = {0,};
	u8 core_ver[4] = {0,};
	u8 config_ver[4] = {0,};
	int ret;

	/* Image ver */
	ret = ts->sec_ts_i2c_read(ts, SEC_TS_READ_IMG_VERSION, img_ver, 4);
	if (ret < 0) {
		input_err(true, &ts->client->dev, "%s: Image version read error\n", __func__);
		return -EIO;
	}
	input_info(true, &ts->client->dev, "%s: IC Image version info : %x.%x.%x.%x\n",
			__func__, img_ver[0], img_ver[1], img_ver[2], img_ver[3]);

	ts->plat_data->img_version_of_ic[0] = img_ver[0];
	ts->plat_data->img_version_of_ic[1] = img_ver[1];
	ts->plat_data->img_version_of_ic[2] = img_ver[2];
	ts->plat_data->img_version_of_ic[3] = img_ver[3];

	/* Core ver */
	ret = ts->sec_ts_i2c_read(ts, SEC_TS_READ_FW_VERSION, core_ver, 4);
	if (ret < 0) {
		input_err(true, &ts->client->dev, "%s: core version read error\n", __func__);
		return -EIO;
	}
	input_info(true, &ts->client->dev, "%s: IC Core version info : %x.%x.%x.%x,\n",
			__func__, core_ver[0], core_ver[1], core_ver[2], core_ver[3]);

	ts->plat_data->core_version_of_ic[0] = core_ver[0];
	ts->plat_data->core_version_of_ic[1] = core_ver[1];
	ts->plat_data->core_version_of_ic[2] = core_ver[2];
	ts->plat_data->core_version_of_ic[3] = core_ver[3];

	/* Config ver */
	ret = ts->sec_ts_i2c_read(ts, SEC_TS_READ_PARA_VERSION, config_ver, 4);
	if (ret < 0) {
		input_err(true, &ts->client->dev, "%s: config version read error\n", __func__);
		return -EIO;
	}
	input_info(true, &ts->client->dev, "%s: IC config version info : %x.%x.%x.%x\n",
			__func__, config_ver[0], config_ver[1], config_ver[2], config_ver[3]);

	ts->plat_data->config_version_of_ic[0] = config_ver[0];
	ts->plat_data->config_version_of_ic[1] = config_ver[1];
	ts->plat_data->config_version_of_ic[2] = config_ver[2];
	ts->plat_data->config_version_of_ic[3] = config_ver[3];

	return 1;
}

int sec_ts_check_firmware_version(struct sec_ts_data *ts, const u8 *fw_info)
{
	fw_header *fw_hd;
	u8 buff[1];
	int i;
	int ret;
	/*
	 * sec_ts_check_firmware_version
	 * return value = 1 : firmware download needed,
	 * return value = 0 : skip firmware download
	 */

	fw_hd = (fw_header *)fw_info;

	sec_ts_save_version_of_bin(ts, fw_hd);

	/* firmware download if READ_BOOT_STATUS = 0x10 */
	ret = ts->sec_ts_i2c_read(ts, SEC_TS_READ_BOOT_STATUS, buff, 1);
	if (ret < 0) {
		input_err(true, &ts->client->dev,
				"%s: fail to read BootStatus\n", __func__);
		return -EIO;
	}

	if (buff[0] == SEC_TS_STATUS_BOOT_MODE) {
		input_err(true, &ts->client->dev,
				"%s: ReadBootStatus = 0x%x, Firmware download Start!\n",
				__func__, buff[0]);
		return 1;
	}

	ret = sec_ts_save_version_of_ic(ts);
	if (ret < 0) {
		input_err(true, &ts->client->dev, "%s: fail to read ic version\n", __func__);
		return -EIO;
	}

	/* check f/w version
	 * ver[0] : IC version
	 * ver[1] : Project version
	 */
	for (i = 0; i < 2; i++) {
		if (ts->plat_data->img_version_of_ic[i] != ts->plat_data->img_version_of_bin[i]) {
			if (ts->plat_data->bringup == 3) {
				input_err(true, &ts->client->dev, "%s: bringup. force update\n", __func__);
				return 1;
			}
			input_err(true, &ts->client->dev, "%s: do not matched version info\n", __func__);
			return 0;
		}
	}

	if (ts->plat_data->img_version_of_ic[2] != ts->plat_data->img_version_of_bin[2])
		return 1;

	if (ts->plat_data->img_version_of_ic[3] < ts->plat_data->img_version_of_bin[3])
		return 1;

	return 0;
}

static u8 sec_ts_checksum(u8 *data, int offset, int size)
{
	int i;
	u8 checksum = 0;

	for (i = 0; i < size; i++)
		checksum += data[i + offset];

	return checksum;
}

static int sec_ts_flashpageerase(struct sec_ts_data *ts, u32 page_idx, u32 page_num)
{
	int ret;
	u8 tCmd[6];

	tCmd[0] = SEC_TS_CMD_FLASH_ERASE;
	tCmd[1] = (u8)((page_idx >> 8) & 0xFF);
	tCmd[2] = (u8)((page_idx >> 0) & 0xFF);
	tCmd[3] = (u8)((page_num >> 8) & 0xFF);
	tCmd[4] = (u8)((page_num >> 0) & 0xFF);
	tCmd[5] = sec_ts_checksum(tCmd, 1, 4);

	ret = ts->sec_ts_i2c_write_burst(ts, tCmd, 6);

	return ret;
}

static int sec_ts_flashpagewrite(struct sec_ts_data *ts, u32 page_idx, u8 *page_data)
{
	int ret;
	u8 tCmd[1 + 2 + SEC_TS_FW_BLK_SIZE_MAX + 1];
	int flash_page_size = (int)ts->flash_page_size;

	tCmd[0] = 0xD9;
	tCmd[1] = (u8)((page_idx >> 8) & 0xFF);
	tCmd[2] = (u8)((page_idx >> 0) & 0xFF);

	memcpy(&tCmd[3], page_data, flash_page_size);
	tCmd[1 + 2 + flash_page_size] = sec_ts_checksum(tCmd, 1, 2 + flash_page_size);

	ret = ts->sec_ts_i2c_write_burst(ts, tCmd, 1 + 2 + flash_page_size + 1);
	return ret;
}

static int sec_ts_limited_flashpagewrite(struct sec_ts_data *ts, u32 page_idx, u8 *page_data)
{
	int ret = 0;
	u8 *tCmd;
	u8 copy_data[3 + SEC_TS_FW_BLK_SIZE_MAX];
	int copy_left = (int)ts->flash_page_size + 3;
	int copy_size = 0;
	int copy_max = ts->i2c_burstmax - 1;
	int flash_page_size = (int)ts->flash_page_size;

	copy_data[0] = (u8)((page_idx >> 8) & 0xFF);	/* addH */
	copy_data[1] = (u8)((page_idx >> 0) & 0xFF);	/* addL */

	memcpy(&copy_data[2], page_data, flash_page_size);	/* DATA */
	copy_data[2 + flash_page_size] = sec_ts_checksum(copy_data, 0, 2 + flash_page_size);	/* CS */

	while (copy_left > 0) {
		int copy_cur = (copy_left > copy_max) ? copy_max : copy_left;

		tCmd = kzalloc(copy_cur + 1, GFP_KERNEL);
		if (!tCmd)
			goto err_write;

		if (copy_size == 0)
			tCmd[0] = SEC_TS_CMD_FLASH_WRITE;
		else
			tCmd[0] = SEC_TS_CMD_FLASH_PADDING;

		memcpy(&tCmd[1], &copy_data[copy_size], copy_cur);

		ret = ts->sec_ts_i2c_write_burst(ts, tCmd, 1 + copy_cur);
		if (ret < 0)
			input_err(true, &ts->client->dev,
					"%s: failed, ret:%d\n", __func__, ret);

		copy_size += copy_cur;
		copy_left -= copy_cur;
		kfree(tCmd);
	}
	return ret;

err_write:
	input_err(true, &ts->client->dev,
			"%s: failed to alloc.\n", __func__);
	return -ENOMEM;

}

static int sec_ts_flashwrite(struct sec_ts_data *ts, u32 mem_addr, u8 *mem_data, u32 mem_size, int retry)
{
	int ret;
	u32 page_idx;
	u32 size_copy;
	u32 flash_page_size;
	u32 page_idx_start;
	u32 page_idx_end;
	u32 page_num;
	u8 page_buf[SEC_TS_FW_BLK_SIZE_MAX];

	if (mem_size == 0)
		return 0;

	flash_page_size = ts->flash_page_size;
	page_idx_start = mem_addr / flash_page_size;
	page_idx_end = (mem_addr + mem_size - 1) / flash_page_size;
	page_num = page_idx_end - page_idx_start + 1;

	ret = sec_ts_flashpageerase(ts, page_idx_start, page_num);
	if (ret < 0) {
		input_err(true, &ts->client->dev,
				"%s: fw erase failed, mem_addr= %08X, pagenum = %d\n",
				__func__, mem_addr, page_num);
		return -EIO;
	}

	sec_ts_delay(page_num + 10);

	size_copy = mem_size % flash_page_size;
	if (size_copy == 0)
		size_copy = flash_page_size;

	memset(page_buf, 0, flash_page_size);

	for (page_idx = page_num - 1;; page_idx--) {
		memcpy(page_buf, mem_data + (page_idx * flash_page_size), size_copy);
		if (ts->boot_ver[0] == 0xB2) {
			ret = sec_ts_flashpagewrite(ts, (page_idx + page_idx_start), page_buf);
			if (ret < 0) {
				input_err(true, &ts->client->dev, "%s: fw write failed, page_idx = %u\n", __func__, page_idx);
				goto err;
			}

			if (retry) {
				sec_ts_delay(50);
				ret = sec_ts_flashpagewrite(ts, (page_idx + page_idx_start), page_buf);
				if (ret < 0) {
					input_err(true, &ts->client->dev, "%s: fw write failed, page_idx = %u\n", __func__, page_idx);
					goto err;
				}
			}
		} else {
			ret = sec_ts_limited_flashpagewrite(ts, (page_idx + page_idx_start), page_buf);
			if (ret < 0) {
				input_err(true, &ts->client->dev, "%s: fw write failed, page_idx = %u\n", __func__, page_idx);
				goto err;
			}

			if (retry) {
				sec_ts_delay(50);
				ret = sec_ts_limited_flashpagewrite(ts, (page_idx + page_idx_start), page_buf);
				if (ret < 0) {
					input_err(true, &ts->client->dev, "%s: fw write failed, page_idx = %u\n", __func__, page_idx);
					goto err;
				}
			}

		}

		size_copy = flash_page_size;
		sec_ts_delay(5);

		if (page_idx == 0) /* end condition (page_idx >= 0)   page_idx type unsinged int */
			break;
	}

	return mem_size;
err:
	return -EIO;
}

#ifdef SEC_TS_FIRMWARE_VERIFICATION
static int sec_ts_memoryblockread(struct sec_ts_data *ts, u32 mem_addr, int mem_size, u8 *buf)
{
	int ret;
	u8 cmd[5];
	u8 *data;

	if (mem_size >= 64 * 1024) {
		input_err(true, &ts->client->dev,
				"%s: mem size over 64K\n", __func__);
		return -EIO;
	}

	cmd[0] = (u8)SEC_TS_CMD_FLASH_READ_ADDR;
	cmd[1] = (u8)((mem_addr >> 24) & 0xff);
	cmd[2] = (u8)((mem_addr >> 16) & 0xff);
	cmd[3] = (u8)((mem_addr >> 8) & 0xff);
	cmd[4] = (u8)((mem_addr >> 0) & 0xff);

	ret = ts->sec_ts_i2c_write_burst(ts, cmd, 5);
	if (ret < 0) {
		input_err(true, &ts->client->dev,
				"%s: send command failed, %02X\n", __func__, cmd[0]);
		return -EIO;
	}

	udelay(10);
	cmd[0] = (u8)SEC_TS_CMD_FLASH_READ_SIZE;
	cmd[1] = (u8)((mem_size >> 8) & 0xff);
	cmd[2] = (u8)((mem_size >> 0) & 0xff);

	ret = ts->sec_ts_i2c_write_burst(ts, cmd, 3);
	if (ret < 0) {
		input_err(true, &ts->client->dev, "%s: send command failed, %02X\n", __func__, cmd[0]);
		return -EIO;
	}

	udelay(10);
	cmd[0] = (u8)SEC_TS_CMD_FLASH_READ_DATA;

	data = buf;


	ret = ts->sec_ts_i2c_read(ts, cmd[0], data, mem_size);
	if (ret < 0) {
		input_err(true, &ts->client->dev, "%s: memory read failed\n", __func__);
		return -EIO;
	}
#if 0
	ret = ts->sec_ts_i2c_write(ts, cmd[0], NULL, 0);
	ret = ts->sec_ts_i2c_read_bulk(ts, data, mem_size);
#endif
	return 0;
}
#endif

#ifdef SEC_TS_FIRMWARE_VERIFICATION
static int sec_ts_memoryread(struct sec_ts_data *ts, u32 mem_addr, u8 *mem_data, u32 mem_size)
{
	int ret;
	int retry = 3;
	int read_size = 0;
	int unit_size;
	int max_size = 1024;
	int read_left = (int)mem_size;
	u8 *tmp_data;

	tmp_data = kmalloc(max_size, GFP_KERNEL);
	if (!tmp_data) {
		input_err(true, &ts->client->dev,
				"%s: failed to kmalloc\n", __func__);
		return -ENOMEM;
	}

	while (read_left > 0) {
		unit_size = (read_left > max_size) ? max_size : read_left;
		retry = 3;
		do {
			ret = sec_ts_memoryblockread(ts, mem_addr, unit_size, tmp_data);
			if (retry-- == 0) {
				input_err(true, &ts->client->dev,
						"%s: fw read fail mem_addr=%08X,unit_size=%d\n",
						__func__, mem_addr, unit_size);
				kfree(tmp_data);
				return -1;
			}

			memcpy(mem_data + read_size, tmp_data, unit_size);
		} while (ret < 0);

		mem_addr += unit_size;
		read_size += unit_size;
		read_left -= unit_size;
	}

	kfree(tmp_data);

	return read_size;
}
#endif

static int sec_ts_chunk_update(struct sec_ts_data *ts, u32 addr, u32 size, u8 *data, int retry)
{
	u32 fw_size;
	u32 write_size;
#ifdef SEC_TS_FIRMWARE_VERIFICATION
	u8 *mem_rb;
#endif
	int ret = 0;

	fw_size = size;

	write_size = sec_ts_flashwrite(ts, addr, data, fw_size, retry);
	if (write_size != fw_size) {
		input_err(true, &ts->client->dev, "%s: fw write failed\n", __func__);
		ret = -1;
		goto err_write_fail;
	}

#ifdef SEC_TS_FIRMWARE_VERIFICATION
	mem_rb = vzalloc(fw_size);
	if (!mem_rb) {
		input_err(true, &ts->client->dev, "%s: vzalloc failed\n", __func__);
		ret = -1;
		goto err_write_fail;
	}

	if (sec_ts_memoryread(ts, addr, mem_rb, fw_size) >= 0) {
		u32 ii;

		for (ii = 0; ii < fw_size; ii++) {
			if (data[ii] != mem_rb[ii])
				break;
		}

		if (fw_size != ii) {
			input_err(true, &ts->client->dev, "%s: fw verify fail\n", __func__);
			ret = -1;
			goto out;
		}
	} else {
		ret = -1;
		goto out;
	}

	input_info(true, &ts->client->dev, "%s: verify done(%d)\n", __func__, ret);

out:
	vfree(mem_rb);
#endif
err_write_fail:
	sec_ts_delay(10);

	return ret;
}

static int sec_ts_firmware_update(struct sec_ts_data *ts, const u8 *data,
						size_t size, int bl_update, int restore_cal, int retry)
{
	int i;
	int ret;
	fw_header *fw_hd;
	fw_chunk *fw_ch;
	u8 fw_status = 0;
	u8 *fd = (u8 *)data;
	u8 tBuff[3];

	/* Check whether CRC is appended or not.
	 * Enter Firmware Update Mode
	 */
	if (!sec_ts_enter_fw_mode(ts)) {
		input_err(true, &ts->client->dev, "%s: firmware mode failed\n", __func__);
		return -1;
	}

	if (bl_update && (ts->boot_ver[0] == 0xB4)) {
		input_info(true, &ts->client->dev, "%s: bootloader is up to date\n", __func__);
		return 0;
	}

	input_info(true, &ts->client->dev, "%s: firmware update retry :%d\n", __func__, retry);

	fw_hd = (fw_header *)fd;
	fd += sizeof(fw_header);

	if (fw_hd->signature != SEC_TS_FW_HEADER_SIGN) {
		input_err(true, &ts->client->dev, "%s: firmware header error = %08X\n", __func__, fw_hd->signature);
		return -1;
	}

	input_info(true, &ts->client->dev, "%s: num_chunk : %d\n", __func__, fw_hd->num_chunk);

	for (i = 0; i < fw_hd->num_chunk; i++) {
		fw_ch = (fw_chunk *)fd;

		input_info(true, &ts->client->dev, "%s: [%d] 0x%08X, 0x%08X, 0x%08X, 0x%08X\n", __func__, i,
				fw_ch->signature, fw_ch->addr, fw_ch->size, fw_ch->reserved);

		if (fw_ch->signature != SEC_TS_FW_CHUNK_SIGN) {
			input_err(true, &ts->client->dev, "%s: firmware chunk error = %08X\n", __func__, fw_ch->signature);
			return -1;
		}
		fd += sizeof(fw_chunk);
		ret = sec_ts_chunk_update(ts, fw_ch->addr, fw_ch->size, fd, retry);
		if (ret < 0) {
			input_err(true, &ts->client->dev, "%s: firmware chunk write failed, addr=%08X, size = %d\n", __func__, fw_ch->addr, fw_ch->size);
			return -1;
		}
		fd += fw_ch->size;
	}

	sec_ts_sw_reset(ts);

	if (!bl_update) {
		if (restore_cal) {
			input_err(true, &ts->client->dev, "%s: RUN OFFSET CALIBRATION\n", __func__);

			ret = sec_ts_execute_force_calibration(ts, OFFSET_CAL_SET);
			if (ret < 0)
				input_err(true, &ts->client->dev, "%s: fail to write OFFSET CAL SET!\n", __func__);
		}

		/* Sense_on */
		ret = ts->sec_ts_i2c_write(ts, SEC_TS_CMD_SENSE_ON, NULL, 0);
		if (ret < 0) {
			input_err(true, &ts->client->dev, "%s: write fail, Sense_on\n", __func__);
			return -EIO;
		}

		if (ts->sec_ts_i2c_read(ts, SEC_TS_READ_BOOT_STATUS, &fw_status, 1) < 0) {
			input_err(true, &ts->client->dev, "%s: read fail, read_boot_status = 0x%x\n", __func__, fw_status);
			return -EIO;
		}

		if (fw_status != SEC_TS_STATUS_APP_MODE) {
			input_err(true, &ts->client->dev, "%s: fw update sequence done, BUT read_boot_status = 0x%x\n", __func__, fw_status);
			return -EIO;
		}

		input_info(true, &ts->client->dev, "%s: fw update Success! read_boot_status = 0x%x\n", __func__, fw_status);

		return 1;
	} else {

		if (ts->sec_ts_i2c_read(ts, SEC_TS_READ_ID, tBuff, 3) < 0) {
			input_err(true, &ts->client->dev, "%s: read device id fail after bl fw download\n", __func__);
			return -EIO;
		}

		if (tBuff[0] == 0xA0) {
			input_info(true, &ts->client->dev, "%s: bl fw download success - device id = %02X\n", __func__, tBuff[0]);
			return -EIO;
		} else {
			input_err(true, &ts->client->dev, "%s: bl fw id does not match - device id = %02X\n", __func__, tBuff[0]);
			return -EIO;
		}
	}

}

int sec_ts_firmware_update_bl(struct sec_ts_data *ts)
{
	const struct firmware *fw_entry;
	char fw_path[SEC_TS_MAX_FW_PATH];
	int result = -1;

	sec_ts_set_irq(ts, false);

	snprintf(fw_path, SEC_TS_MAX_FW_PATH, "%s", SEC_TS_DEFAULT_BL_NAME);

	input_info(true, &ts->client->dev, "%s: initial bl update %s\n", __func__, fw_path);

	/* Loading Firmware------------------------------------------ */
	if (request_firmware(&fw_entry, fw_path, &ts->client->dev) !=  0) {
		input_err(true, &ts->client->dev, "%s: bt is not available\n", __func__);
		goto err_request_fw;
	}
	input_info(true, &ts->client->dev, "%s: request bt done! size = %d\n", __func__, (int)fw_entry->size);

	result = sec_ts_firmware_update(ts, fw_entry->data, fw_entry->size, 1, false, 0);

err_request_fw:
	release_firmware(fw_entry);
	sec_ts_set_irq(ts, true);

	return result;
}

int sec_ts_bl_update(struct sec_ts_data *ts)
{
	int ret;
	u8 tCmd[5] = { 0xDE, 0xAD, 0xBE, 0xEF };
	u8 tBuff[3];

	ret = ts->sec_ts_i2c_write(ts, SEC_TS_READ_BL_UPDATE_STATUS, tCmd, 4);
	if (ret < 0) {
		input_err(true, &ts->client->dev, "%s: bl update command send fail!\n", __func__);
		goto err;
	}
	sec_ts_delay(10);

	do {
		ret = ts->sec_ts_i2c_read(ts, SEC_TS_READ_BL_UPDATE_STATUS, tBuff, 1);
		if (ret < 0) {
			input_err(true, &ts->client->dev, "%s: read bl update status fail!\n", __func__);
			goto err;
		}
		sec_ts_delay(2);

	} while (tBuff[0] == 0x1);

	tCmd[0] = 0x55;
	tCmd[1] = 0xAC;
	ret = ts->sec_ts_i2c_write(ts, 0x57, tCmd, 2);
	if (ret < 0) {
		input_err(true, &ts->client->dev, "%s: write passwd fail!\n", __func__);
		goto err;
	}

	ret = ts->sec_ts_i2c_read(ts, SEC_TS_READ_ID, tBuff, 3);

	if (tBuff[0]  == 0xB4) {
		input_info(true, &ts->client->dev, "%s: bl update completed!\n", __func__);
		ret = 1;
	} else {
		input_info(true, &ts->client->dev, "%s: bl updated but bl version not matching, ver=%02X\n", __func__, tBuff[0]);
		goto err;
	}

	return ret;
err:
	return -EIO;
}

int sec_ts_firmware_update_on_probe(struct sec_ts_data *ts, bool force_update)
{
	const struct firmware *fw_entry;
	char fw_path[SEC_TS_MAX_FW_PATH];
	int result = -1, restore_cal = 0;
	int ii = 0;
	int ret = 0;
	int skip_firmup = 0;

	if (ts->plat_data->bringup == 1) {
		input_err(true, &ts->client->dev, "%s: bringup. do not update\n", __func__);
		return 0;
	}

	if (ts->plat_data->firmware_name)
		snprintf(fw_path, SEC_TS_MAX_FW_PATH, "%s", ts->plat_data->firmware_name);
	else
		return 0;

	sec_ts_set_irq(ts, false);

	/* read cal status */
	ts->cal_status = sec_ts_read_calibration_report(ts);

	input_info(true, &ts->client->dev, "%s: initial firmware update %s, cal:%X\n",
			__func__, fw_path, ts->cal_status);

	/* Loading Firmware */
	if (request_firmware(&fw_entry, fw_path, &ts->client->dev) !=  0) {
		input_err(true, &ts->client->dev, "%s: firmware is not available\n", __func__);
		goto err_request_fw;
	}
	input_info(true, &ts->client->dev, "%s: request firmware done! size = %d\n", __func__, (int)fw_entry->size);

	result = sec_ts_check_firmware_version(ts, fw_entry->data);
	if (ts->plat_data->bringup == 2) {
		input_err(true, &ts->client->dev, "%s: bringup. do not update\n", __func__);
		result = 0;
		goto err_request_fw;
	}

	/* don't firmup case */
	if ((result <= 0) && (!force_update)) {
		input_info(true, &ts->client->dev, "%s: skip - fw update\n", __func__);
		/* goto err_request_fw; */
		skip_firmup = 1;
	}

	if ((skip_firmup == 1) && (restore_cal == 0))
		goto err_request_fw;

	for (ii = 0; ii < 3; ii++) {
		ret = sec_ts_firmware_update(ts, fw_entry->data, fw_entry->size, 0, restore_cal, ii);
		if (ret >= 0)
			break;
	}

	if (ret < 0)
		result = -1;
	else
		result = 0;

	sec_ts_save_version_of_ic(ts);

err_request_fw:
	release_firmware(fw_entry);
	return result;
}

static int sec_ts_load_fw_from_bin(struct sec_ts_data *ts)
{
	const struct firmware *fw_entry;
	char fw_path[SEC_TS_MAX_FW_PATH];
	int error = 0;
	int restore_cal = 0;

	if (ts->client->irq)
		sec_ts_set_irq(ts, false);

	if (!ts->plat_data->firmware_name)
		snprintf(fw_path, SEC_TS_MAX_FW_PATH, "%s", SEC_TS_DEFAULT_FW_NAME);
	else
		snprintf(fw_path, SEC_TS_MAX_FW_PATH, "%s", ts->plat_data->firmware_name);

	input_info(true, &ts->client->dev, "%s: initial firmware update  %s\n", __func__, fw_path);

	/* Loading Firmware */
	if (request_firmware(&fw_entry, fw_path, &ts->client->dev) !=  0) {
		input_err(true, &ts->client->dev, "%s: firmware is not available\n", __func__);
		error = -1;
		goto err_request_fw;
	}
	input_info(true, &ts->client->dev, "%s: request firmware done! size = %d\n", __func__, (int)fw_entry->size);

	/* use virtual pat_control - magic cal 1 */
	if (sec_ts_firmware_update(ts, fw_entry->data, fw_entry->size, 0, restore_cal, 0) < 0)
		error = -1;
	else
		error = 0;

	sec_ts_save_version_of_ic(ts);

err_request_fw:
	release_firmware(fw_entry);
	if (ts->client->irq)
		sec_ts_set_irq(ts, true);

	return error;
}

static int sec_ts_load_fw_from_ums(struct sec_ts_data *ts)
{
	fw_header *fw_hd;
	struct file *fp;
	mm_segment_t old_fs;
	long fw_size, nread;
	int error = 0;
	int restore_cal = 0;

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	fp = filp_open(SEC_TS_DEFAULT_UMS_FW, O_RDONLY, S_IRUSR);
	if (IS_ERR(fp)) {
		input_err(true, ts->dev, "%s: failed to open %s.\n", __func__,
				SEC_TS_DEFAULT_UMS_FW);
		error = -ENOENT;
		goto open_err;
	}

	fw_size = fp->f_path.dentry->d_inode->i_size;

	if (fw_size > 0) {
		unsigned char *fw_data;

		fw_data = vzalloc(fw_size);
		if (!fw_data) {
			input_err(true, ts->dev, "%s: failed to alloc mem\n", __func__);
			error = -ENOMEM;
			filp_close(fp, NULL);
			goto open_err;
		}
		nread = vfs_read(fp, (char __user *)fw_data,
				fw_size, &fp->f_pos);

		input_info(true, ts->dev,
				"%s: start, file path %s, size %ld Bytes\n",
				__func__, SEC_TS_DEFAULT_UMS_FW, fw_size);

		if (nread != fw_size) {
			input_err(true, ts->dev,
					"%s: failed to read firmware file, nread %ld Bytes\n",
					__func__, nread);
			error = -EIO;
		} else {
			fw_hd = (fw_header *)fw_data;
#if 0
			sec_ts_check_firmware_version(ts, fw_data);
#endif
			input_info(true, &ts->client->dev, "%s: firmware version %08X\n", __func__, fw_hd->fw_ver);
			input_info(true, &ts->client->dev, "%s: parameter version %08X\n", __func__, fw_hd->para_ver);

			if (ts->client->irq)
				sec_ts_set_irq(ts, false);
			/* use virtual pat_control - magic cal 1 */
			if (sec_ts_firmware_update(ts, fw_data, fw_size, 0, restore_cal, 0) < 0) {
				error = -1; /* firmware failed */
				goto done;
			}

			sec_ts_save_version_of_ic(ts);
		}

		if (error < 0)
			input_err(true, ts->dev, "%s: failed update firmware\n",
					__func__);

done:
		if (ts->client->irq)
			sec_ts_set_irq(ts, true);
		vfree(fw_data);
	}

	filp_close(fp, NULL);

open_err:
	set_fs(old_fs);
	return error;
}

static int sec_ts_load_fw_from_ffu(struct sec_ts_data *ts)
{
	const struct firmware *fw_entry;
	const char *fw_path = SEC_TS_DEFAULT_FFU_FW;
	int result = -1;

	sec_ts_set_irq(ts, false);

	input_info(true, ts->dev, "%s: Load firmware : %s\n", __func__, fw_path);

	/* Loading Firmware */
	if (request_firmware(&fw_entry, fw_path, &ts->client->dev) !=  0) {
		input_err(true, &ts->client->dev, "%s: firmware is not available\n", __func__);
		goto err_request_fw;
	}
	input_info(true, &ts->client->dev, "%s: request firmware done! size = %d\n", __func__, (int)fw_entry->size);

	sec_ts_check_firmware_version(ts, fw_entry->data);

	if (sec_ts_firmware_update(ts, fw_entry->data, fw_entry->size, 0, false, 0) < 0)
		result = -1;
	else
		result = 0;

	sec_ts_save_version_of_ic(ts);

err_request_fw:
	release_firmware(fw_entry);
	sec_ts_set_irq(ts, true);
	return result;
}

int sec_ts_firmware_update_on_hidden_menu(struct sec_ts_data *ts, int update_type)
{
	int ret = 0;

	/* Factory cmd for firmware update
	 * argument represent what is source of firmware like below.
	 *
	 * 0 : [BUILT_IN] Getting firmware which is for user.
	 * 1 : [UMS] Getting firmware from sd card.
	 * 2 : none
	 * 3 : [FFU] Getting firmware from air.
	 */

	switch (update_type) {
	case BUILT_IN:
		ret = sec_ts_load_fw_from_bin(ts);
		break;
	case UMS:
		ret = sec_ts_load_fw_from_ums(ts);
		break;
	case FFU:
		ret = sec_ts_load_fw_from_ffu(ts);
		break;
	case BL:
		ret = sec_ts_firmware_update_bl(ts);
		if (ret < 0) {
			break;
		} else if (!ret) {
			ret = sec_ts_firmware_update_on_probe(ts, false);
			break;
		} else {
			ret = sec_ts_bl_update(ts);
			if (ret < 0)
				break;
			ret = sec_ts_firmware_update_on_probe(ts, false);
			if (ret < 0)
				break;
		}
		break;
	default:
		input_err(true, ts->dev, "%s: Not support command[%d]\n",
				__func__, update_type);
		break;
	}

	return ret;
}
EXPORT_SYMBOL(sec_ts_firmware_update_on_hidden_menu);
