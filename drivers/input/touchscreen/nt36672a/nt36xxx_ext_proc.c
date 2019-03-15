/*
 * Copyright (C) 2010 - 2017 Novatek, Inc.
 *
 * $Revision: 20251 $
 * $Date: 2017-12-13 17:41:29 +0800 (Wed, 13 Dec 2017) $
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 */


#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/delay.h>

#include "nt36xxx.h"

#if NVT_TOUCH_EXT_PROC
#define NVT_FW_VERSION "nvt_fw_version"
#define NVT_BASELINE "nvt_baseline"
#define NVT_RAW "nvt_raw"
#define NVT_DIFF "nvt_diff"

#if NVT_PROJECT_INFO_READ
#define NVT_PROJECT_INFO "nvt_projectinfo"
#endif

#define I2C_TANSFER_LENGTH  64

#define NORMAL_MODE 0x00
#define TEST_MODE_1 0x21
#define TEST_MODE_2 0x22
#define HANDSHAKING_HOST_READY 0xBB

#define XDATA_SECTOR_SIZE   256

static uint8_t xdata_tmp[2048] = {0};
static int32_t xdata[2048] = {0};
static int32_t xdata_i[2048] = {0};
static int32_t xdata_q[2048] = {0};

static struct proc_dir_entry *NVT_proc_fw_version_entry;
static struct proc_dir_entry *NVT_proc_baseline_entry;
static struct proc_dir_entry *NVT_proc_raw_entry;
static struct proc_dir_entry *NVT_proc_diff_entry;

/*---For SMx3 use start---*/
#if NVT_PROJECT_INFO_READ
static struct proc_dir_entry *NVT_proc_projectinfo_entry;
#endif
#define NVT_BEFORE_DIFF "nvt_before_diff"
#define NVT_PROBE_LOG "smx3_probe_log"
#define NVT_FW_VER_CHECK "smx3_fw_ver_check"
static struct proc_dir_entry *NVT_proc_before_diff_entry;
static struct proc_dir_entry *SMx3_proc_probe_log_entry;
static struct proc_dir_entry *SMx3_proc_fw_ver_check_entry;

#if BOOT_UPDATE_FIRMWARE
#define NVT_FW_VER_VERIFY "smx3_fw_ver_verify"
static struct proc_dir_entry *SMx3_proc_fw_ver_verify_entry;
#endif
/*---For SMx3 use end---*/


/*******************************************************
Description:
	Novatek touchscreen change mode function.

return:
	n.a.
*******************************************************/
void nvt_change_mode(uint8_t mode)
{
	uint8_t buf[8] = {0};

	TP_LOGD("+++");

	//---set xdata index to EVENT BUF ADDR---
	buf[0] = 0xFF;
	buf[1] = (ts->mmap->EVENT_BUF_ADDR >> 16) & 0xFF;
	buf[2] = (ts->mmap->EVENT_BUF_ADDR >> 8) & 0xFF;
	CTP_I2C_WRITE(ts->client, I2C_FW_Address, buf, 3);

	//---set mode---
	buf[0] = EVENT_MAP_HOST_CMD;
	buf[1] = mode;
	CTP_I2C_WRITE(ts->client, I2C_FW_Address, buf, 2);

	if (mode == NORMAL_MODE) {
		buf[0] = EVENT_MAP_HANDSHAKING_or_SUB_CMD_BYTE;
		buf[1] = HANDSHAKING_HOST_READY;
		CTP_I2C_WRITE(ts->client, I2C_FW_Address, buf, 2);
		msleep(20);
	}

	TP_LOGD("---");

}

/*******************************************************
Description:
	Novatek touchscreen get firmware pipe function.

return:
	Executive outcomes. 0---pipe 0. 1---pipe 1.
*******************************************************/
uint8_t nvt_get_fw_pipe(void)
{
	uint8_t buf[8]= {0};

	TP_LOGD("+++");

	//---set xdata index to EVENT BUF ADDR---
	buf[0] = 0xFF;
	buf[1] = (ts->mmap->EVENT_BUF_ADDR >> 16) & 0xFF;
	buf[2] = (ts->mmap->EVENT_BUF_ADDR >> 8) & 0xFF;
	CTP_I2C_WRITE(ts->client, I2C_FW_Address, buf, 3);

	//---read fw status---
	buf[0] = EVENT_MAP_HANDSHAKING_or_SUB_CMD_BYTE;
	buf[1] = 0x00;
	CTP_I2C_READ(ts->client, I2C_FW_Address, buf, 2);

	/* TP_LOGD("FW pipe=%d, buf[1]=0x%02X", (buf[1]&0x01), buf[1]); */

	TP_LOGD("---");

	return (buf[1] & 0x01);
}

/*******************************************************
Description:
	Novatek touchscreen read meta data function.

return:
	n.a.
*******************************************************/
void nvt_read_mdata(uint32_t xdata_addr, uint32_t xdata_btn_addr)
{
	int32_t i = 0;
	int32_t j = 0;
	int32_t k = 0;
	uint8_t buf[I2C_TANSFER_LENGTH + 1] = {0};
	uint32_t head_addr = 0;
	int32_t dummy_len = 0;
	int32_t data_len = 0;
	int32_t residual_len = 0;

	TP_LOGD("+++");

	//---set xdata sector address & length---
	head_addr = xdata_addr - (xdata_addr % XDATA_SECTOR_SIZE);
	dummy_len = xdata_addr - head_addr;
	data_len = ts->x_num * ts->y_num * 2;
	residual_len = (head_addr + dummy_len + data_len) % XDATA_SECTOR_SIZE;

	//printk("head_addr=0x%05X, dummy_len=0x%05X, data_len=0x%05X, residual_len=0x%05X\n", head_addr, dummy_len, data_len, residual_len);

	//read xdata : step 1
	for (i = 0; i < ((dummy_len + data_len) / XDATA_SECTOR_SIZE); i++) {
		//---change xdata index---
		buf[0] = 0xFF;
		buf[1] = ((head_addr + XDATA_SECTOR_SIZE * i) >> 16) & 0xFF;
		buf[2] = ((head_addr + XDATA_SECTOR_SIZE * i) >> 8) & 0xFF;
		CTP_I2C_WRITE(ts->client, I2C_FW_Address, buf, 3);

		//---read xdata by I2C_TANSFER_LENGTH
		for (j = 0; j < (XDATA_SECTOR_SIZE / I2C_TANSFER_LENGTH); j++) {
			//---read data---
			buf[0] = I2C_TANSFER_LENGTH * j;
			CTP_I2C_READ(ts->client, I2C_FW_Address, buf, I2C_TANSFER_LENGTH + 1);

			//---copy buf to xdata_tmp---
			for (k = 0; k < I2C_TANSFER_LENGTH; k++) {
				xdata_tmp[XDATA_SECTOR_SIZE * i + I2C_TANSFER_LENGTH * j + k] = buf[k + 1];
				//printk("0x%02X, 0x%04X\n", buf[k+1], (XDATA_SECTOR_SIZE*i + I2C_TANSFER_LENGTH*j + k));
			}
		}
		//printk("addr=0x%05X\n", (head_addr+XDATA_SECTOR_SIZE*i));
	}

	//read xdata : step2
	if (residual_len != 0) {
		//---change xdata index---
		buf[0] = 0xFF;
		buf[1] = ((xdata_addr + data_len - residual_len) >> 16) & 0xFF;
		buf[2] = ((xdata_addr + data_len - residual_len) >> 8) & 0xFF;
		CTP_I2C_WRITE(ts->client, I2C_FW_Address, buf, 3);

		//---read xdata by I2C_TANSFER_LENGTH
		for (j = 0; j < (residual_len / I2C_TANSFER_LENGTH + 1); j++) {
			//---read data---
			buf[0] = I2C_TANSFER_LENGTH * j;
			CTP_I2C_READ(ts->client, I2C_FW_Address, buf, I2C_TANSFER_LENGTH + 1);

			//---copy buf to xdata_tmp---
			for (k = 0; k < I2C_TANSFER_LENGTH; k++) {
				xdata_tmp[(dummy_len + data_len - residual_len) + I2C_TANSFER_LENGTH * j + k] = buf[k + 1];
				//printk("0x%02X, 0x%04x\n", buf[k+1], ((dummy_len+data_len-residual_len) + I2C_TANSFER_LENGTH*j + k));
			}
		}
		//printk("addr=0x%05X\n", (xdata_addr+data_len-residual_len));
	}

	//---remove dummy data and 2bytes-to-1data---
	for (i = 0; i < (data_len / 2); i++) {
		xdata[i] = (int16_t)(xdata_tmp[dummy_len + i * 2] + 256 * xdata_tmp[dummy_len + i * 2 + 1]);
	}

	//---set xdata index to EVENT BUF ADDR---
	buf[0] = 0xFF;
	buf[1] = (ts->mmap->EVENT_BUF_ADDR >> 16) & 0xFF;
	buf[2] = (ts->mmap->EVENT_BUF_ADDR >> 8) & 0xFF;
	CTP_I2C_WRITE(ts->client, I2C_FW_Address, buf, 3);

	TP_LOGD("---");
}

/*******************************************************
Description:
	Novatek touchscreen read meta data from IQ to rss function.

return:
	n.a.
*******************************************************/
void nvt_read_mdata_rss(uint32_t xdata_i_addr, uint32_t xdata_q_addr, uint32_t xdata_btn_i_addr, uint32_t xdata_btn_q_addr)
{
	int i = 0;

	nvt_read_mdata(xdata_i_addr, xdata_btn_i_addr);
	memcpy(xdata_i, xdata, ((ts->x_num * ts->y_num) * sizeof(int32_t)));

	nvt_read_mdata(xdata_q_addr, xdata_btn_q_addr);
	memcpy(xdata_q, xdata, ((ts->x_num * ts->y_num) * sizeof(int32_t)));

	for (i = 0; i < (ts->x_num * ts->y_num); i++) {
		xdata[i] = (int32_t)int_sqrt((unsigned long)(xdata_i[i] * xdata_i[i]) + (unsigned long)(xdata_q[i] * xdata_q[i]));
	}
}

/*******************************************************
Description:
    Novatek touchscreen get meta data function.

return:
    n.a.
*******************************************************/
void nvt_get_mdata(int32_t *buf, uint8_t *m_x_num, uint8_t *m_y_num)
{
    *m_x_num = ts->x_num;
    *m_y_num = ts->y_num;
    memcpy(buf, xdata, ((ts->x_num * ts->y_num) * sizeof(int32_t)));
}

/*******************************************************
Description:
	Novatek touchscreen firmware version show function.

return:
	Executive outcomes. 0---succeed.
*******************************************************/
static int32_t c_fw_version_show(struct seq_file *m, void *v)
{
	seq_printf(m, "fw_ver=%d, x_num=%d, y_num=%d, button_num=%d\n", ts->fw_ver, ts->x_num, ts->y_num, ts->max_button_num);
	return 0;
}

/*******************************************************
Description:
	Novatek touchscreen xdata sequence print show
	function.

return:
	Executive outcomes. 0---succeed.
*******************************************************/
static int32_t c_show(struct seq_file *m, void *v)
{
	int32_t i = 0;
	int32_t j = 0;

	for (i = 0; i < ts->y_num; i++) {
		for (j = 0; j < ts->x_num; j++) {
			seq_printf(m, "%5d, ", xdata[i * ts->x_num + j]);
		}
		seq_puts(m, "\n");
	}

	seq_printf(m, "\n\n");
	return 0;
}

/*******************************************************
Description:
	Novatek touchscreen xdata sequence print start
	function.

return:
	Executive outcomes. 1---call next function.
	NULL---not call next function and sequence loop
	stop.
*******************************************************/
static void *c_start(struct seq_file *m, loff_t *pos)
{
	return *pos < 1 ? (void *)1 : NULL;
}

/*******************************************************
Description:
	Novatek touchscreen xdata sequence print next
	function.

return:
	Executive outcomes. NULL---no next and call sequence
	stop function.
*******************************************************/
static void *c_next(struct seq_file *m, void *v, loff_t *pos)
{
	++*pos;
	return NULL;
}

/*******************************************************
Description:
	Novatek touchscreen xdata sequence print stop
	function.

return:
	n.a.
*******************************************************/
static void c_stop(struct seq_file *m, void *v)
{
	return;
}

const struct seq_operations nvt_fw_version_seq_ops = {
	.start  = c_start,
	.next   = c_next,
	.stop   = c_stop,
	.show   = c_fw_version_show
};

const struct seq_operations nvt_seq_ops = {
	.start  = c_start,
	.next   = c_next,
	.stop   = c_stop,
	.show   = c_show
};

/*******************************************************
Description:
	Novatek touchscreen /proc/nvt_fw_version open
	function.

return:
	n.a.
*******************************************************/
static int32_t nvt_fw_version_open(struct inode *inode, struct file *file)
{
	TP_LOGI("+++");

	if (mutex_lock_interruptible(&ts->lock)) {
		TP_LOGE("mutex lock fail");
		TP_LOGI("---");
		return -ERESTARTSYS;
	}

	if (nvt_get_fw_info()) {
		mutex_unlock(&ts->lock);
		TP_LOGI("---");
		return -EAGAIN;
	}

	mutex_unlock(&ts->lock);

	TP_LOGI("---");

	return seq_open(file, &nvt_fw_version_seq_ops);
}

static const struct file_operations nvt_fw_version_fops = {
	.owner = THIS_MODULE,
	.open = nvt_fw_version_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release,
};

/*******************************************************
Description:
	Novatek touchscreen /proc/nvt_baseline open function.

return:
	Executive outcomes. 0---succeed.
*******************************************************/
static int32_t nvt_baseline_open(struct inode *inode, struct file *file)
{
	TP_LOGI("+++");

	if (mutex_lock_interruptible(&ts->lock)) {
		TP_LOGE("mutex lock fail");
		TP_LOGI("---");
		return -ERESTARTSYS;
	}

	if (nvt_clear_fw_status()) {
		mutex_unlock(&ts->lock);
		TP_LOGI("---");
		return -EAGAIN;
	}

	nvt_change_mode(TEST_MODE_2);

	if (nvt_check_fw_status()) {
		mutex_unlock(&ts->lock);
		TP_LOGI("---");
		return -EAGAIN;
	}

	if (nvt_get_fw_info()) {
		mutex_unlock(&ts->lock);
		TP_LOGI("---");
		return -EAGAIN;
	}

	if (ts->carrier_system) {
		nvt_read_mdata_rss(ts->mmap->BASELINE_ADDR, ts->mmap->BASELINE_Q_ADDR,
				ts->mmap->BASELINE_BTN_ADDR, ts->mmap->BASELINE_BTN_Q_ADDR);
	} else {
		nvt_read_mdata(ts->mmap->BASELINE_ADDR, ts->mmap->BASELINE_BTN_ADDR);
	}

	nvt_change_mode(NORMAL_MODE);

	mutex_unlock(&ts->lock);

	TP_LOGI("---");

	return seq_open(file, &nvt_seq_ops);
}

static const struct file_operations nvt_baseline_fops = {
	.owner = THIS_MODULE,
	.open = nvt_baseline_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release,
};

/*******************************************************
Description:
	Novatek touchscreen /proc/nvt_raw open function.

return:
	Executive outcomes. 0---succeed.
*******************************************************/
static int32_t nvt_raw_open(struct inode *inode, struct file *file)
{
	TP_LOGI("+++");

	if (mutex_lock_interruptible(&ts->lock)) {
		TP_LOGE("mutex lock fail");
		TP_LOGI("---");
		return -ERESTARTSYS;
	}

	if (nvt_clear_fw_status()) {
		mutex_unlock(&ts->lock);
		TP_LOGI("---");
		return -EAGAIN;
	}

	nvt_change_mode(TEST_MODE_2);

	if (nvt_check_fw_status()) {
		mutex_unlock(&ts->lock);
		TP_LOGI("---");
		return -EAGAIN;
	}

	if (nvt_get_fw_info()) {
		mutex_unlock(&ts->lock);
		TP_LOGI("---");
		return -EAGAIN;
	}

	if (ts->carrier_system) {
		if (nvt_get_fw_pipe() == 0)
			nvt_read_mdata_rss(ts->mmap->RAW_PIPE0_ADDR, ts->mmap->RAW_PIPE0_Q_ADDR,
				ts->mmap->RAW_BTN_PIPE0_ADDR, ts->mmap->RAW_BTN_PIPE0_Q_ADDR);
		else
			nvt_read_mdata_rss(ts->mmap->RAW_PIPE1_ADDR, ts->mmap->RAW_PIPE1_Q_ADDR,
				ts->mmap->RAW_BTN_PIPE1_ADDR, ts->mmap->RAW_BTN_PIPE1_Q_ADDR);
	} else {
		if (nvt_get_fw_pipe() == 0)
			nvt_read_mdata(ts->mmap->RAW_PIPE0_ADDR, ts->mmap->RAW_BTN_PIPE0_ADDR);
		else
			nvt_read_mdata(ts->mmap->RAW_PIPE1_ADDR, ts->mmap->RAW_BTN_PIPE1_ADDR);
	}

	nvt_change_mode(NORMAL_MODE);

	mutex_unlock(&ts->lock);

	TP_LOGI("---");

	return seq_open(file, &nvt_seq_ops);
}

static const struct file_operations nvt_raw_fops = {
	.owner = THIS_MODULE,
	.open = nvt_raw_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release,
};

/*******************************************************
Description:
	Novatek touchscreen /proc/nvt_diff open function.

return:
	Executive outcomes. 0---succeed. negative---failed.
*******************************************************/
static int32_t nvt_diff_open(struct inode *inode, struct file *file)
{
	TP_LOGI("+++");

	if (mutex_lock_interruptible(&ts->lock)) {
		TP_LOGE("mutex lock fail");
		TP_LOGI("---");
		return -ERESTARTSYS;
	}

	if (nvt_clear_fw_status()) {
		mutex_unlock(&ts->lock);
		TP_LOGI("---");
		return -EAGAIN;
	}

	nvt_change_mode(TEST_MODE_2);

	if (nvt_check_fw_status()) {
		mutex_unlock(&ts->lock);
		TP_LOGI("---");
		return -EAGAIN;
	}

	if (nvt_get_fw_info()) {
		mutex_unlock(&ts->lock);
		TP_LOGI("---");
		return -EAGAIN;
	}

	if (ts->carrier_system) {
		if (nvt_get_fw_pipe() == 0)
			nvt_read_mdata_rss(ts->mmap->DIFF_PIPE0_ADDR, ts->mmap->DIFF_PIPE0_Q_ADDR,
				ts->mmap->DIFF_BTN_PIPE0_ADDR, ts->mmap->DIFF_BTN_PIPE0_Q_ADDR);
		else
			nvt_read_mdata_rss(ts->mmap->DIFF_PIPE1_ADDR, ts->mmap->DIFF_PIPE1_Q_ADDR,
				ts->mmap->DIFF_BTN_PIPE1_ADDR, ts->mmap->DIFF_BTN_PIPE1_Q_ADDR);
	} else {
		if (nvt_get_fw_pipe() == 0)
			nvt_read_mdata(ts->mmap->DIFF_PIPE0_ADDR, ts->mmap->DIFF_BTN_PIPE0_ADDR);
		else
			nvt_read_mdata(ts->mmap->DIFF_PIPE1_ADDR, ts->mmap->DIFF_BTN_PIPE1_ADDR);
	}

	nvt_change_mode(NORMAL_MODE);

	mutex_unlock(&ts->lock);

	TP_LOGI("---");

	return seq_open(file, &nvt_seq_ops);
}

static const struct file_operations nvt_diff_fops = {
	.owner = THIS_MODULE,
	.open = nvt_diff_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release,
};

/*---For SMx3 use start---*/
static int32_t nvt_before_diff_open(struct inode *inode, struct file *file)
{
	TP_LOGI("+++");

	if (mutex_lock_interruptible(&ts->lock)) {
		TP_LOGE("mutex lock fail");
		TP_LOGI("---");
		return -ERESTARTSYS;
	}

	if (nvt_clear_fw_status()) {
		mutex_unlock(&ts->lock);
		TP_LOGI("---");
		return -EAGAIN;
	}

	nvt_change_mode(TEST_MODE_1);

	if (nvt_check_fw_status()) {
		mutex_unlock(&ts->lock);
		TP_LOGI("---");
		return -EAGAIN;
	}

	if (nvt_get_fw_info()) {
		mutex_unlock(&ts->lock);
		TP_LOGI("---");
		return -EAGAIN;
	}

	if (ts->carrier_system) {
		if (nvt_get_fw_pipe() == 0)
			nvt_read_mdata_rss(ts->mmap->DIFF_PIPE0_ADDR,
				ts->mmap->DIFF_PIPE0_Q_ADDR,
				ts->mmap->DIFF_BTN_PIPE0_ADDR,
				ts->mmap->DIFF_BTN_PIPE0_Q_ADDR);
		else
			nvt_read_mdata_rss(ts->mmap->DIFF_PIPE1_ADDR,
				ts->mmap->DIFF_PIPE1_Q_ADDR,
				ts->mmap->DIFF_BTN_PIPE1_ADDR,
				ts->mmap->DIFF_BTN_PIPE1_Q_ADDR);
	} else {
		if (nvt_get_fw_pipe() == 0)
			nvt_read_mdata(ts->mmap->DIFF_PIPE0_ADDR,
				ts->mmap->DIFF_BTN_PIPE0_ADDR);
		else
			nvt_read_mdata(ts->mmap->DIFF_PIPE1_ADDR,
				ts->mmap->DIFF_BTN_PIPE1_ADDR);
	}

	nvt_change_mode(NORMAL_MODE);

	mutex_unlock(&ts->lock);

	TP_LOGI("---");

	return seq_open(file, &nvt_seq_ops);
}

static const struct file_operations nvt_before_diff_fops = {
	.owner = THIS_MODULE,
	.open = nvt_before_diff_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release,
};

#if NVT_PROJECT_INFO_READ
/*******************************************************
 *Description:
 *	Novatek touchscreen /proc/nvt_projectinfo open function.
 *
 *return:
 *	Executive outcomes. 0---succeed. negative---failed.
 *
 *To save the project info into ts->project_info, SMx3 move the
 *read project info's into nt36xxx.c:nvt_read_project_info and
 *modify this function which can snow the project info.
 *******************************************************/
static int32_t c_project_info_show(struct seq_file *m, void *v)
{
	unsigned char i;

	mutex_lock(&ts->lock);
	seq_puts(m, "Project info = 0x");

	for (i = 0; i < PROJECT_INFO_LEN_VALID; i++)
		seq_printf(m, "%02X", ts->project_info[i]);

	seq_puts(m, "\n");
	mutex_unlock(&ts->lock);

	return 0;
}

const struct seq_operations nvt_projectinfo_seq_ops = {
	.start  = c_start,
	.next   = c_next,
	.stop   = c_stop,
	.show   = c_project_info_show
};

static int32_t nvt_projectinfo_open(struct inode *inode, struct file *file)
{
	TP_LOGI("+++");

	mutex_lock(&ts->lock);

	/* Get external flash id (project info) */
	nvt_read_project_info();
	mutex_unlock(&ts->lock);

	TP_LOGI("---");

	return seq_open(file, &nvt_projectinfo_seq_ops);
}

static const struct file_operations nvt_projectinfo_fops = {
	.owner = THIS_MODULE,
	.open = nvt_projectinfo_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release,
};
#endif

static int32_t c_fw_ver_show(struct seq_file *m, void *v)
{
	seq_printf(m, "0x%02X\n", ts->fw_ver);

	return 0;
}

const struct seq_operations smx3_fw_ver_check_seq_ops = {
	.start  = c_start,
	.next   = c_next,
	.stop   = c_stop,
	.show   = c_fw_ver_show
};

static int32_t smx3_fw_ver_check_open(struct inode *inode, struct file *file)
{
	TP_LOGI("+++");

	if (mutex_lock_interruptible(&ts->lock)) {
		TP_LOGE("mutex lock fail");
		TP_LOGI("---");
		return -ERESTARTSYS;
	}

	nvt_get_fw_info();

	mutex_unlock(&ts->lock);

	TP_LOGI("---");

	return seq_open(file, &smx3_fw_ver_check_seq_ops);
}

static const struct file_operations smx3_fw_ver_check_fops = {
	.owner = THIS_MODULE,
	.open = smx3_fw_ver_check_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release,
};

#if BOOT_UPDATE_FIRMWARE
static int32_t c_fw_ver_verify_show(struct seq_file *m, void *v)
{
	seq_printf(m, "%s, IC_FW_Ver = 0x%02X, SW_FW_Ver = 0x%02X\n",
			ts->fw_ver == ts->sw_fw_ver ? "Pass" : "Fail",
			ts->fw_ver, ts->sw_fw_ver);

	return 0;
}

const struct seq_operations smx3_fw_ver_verify_seq_ops = {
	.start  = c_start,
	.next   = c_next,
	.stop   = c_stop,
	.show   = c_fw_ver_verify_show
};

static int32_t smx3_fw_ver_verify_open(struct inode *inode, struct file *file)
{
	TP_LOGI("+++");

	if (mutex_lock_interruptible(&ts->lock)) {
		TP_LOGE("mutex lock fail");
		TP_LOGI("---");
		return -ERESTARTSYS;
	}

	nvt_get_fw_info();

	mutex_unlock(&ts->lock);

	TP_LOGI("---");

	return seq_open(file, &smx3_fw_ver_verify_seq_ops);
}

static const struct file_operations smx3_fw_ver_verify_fops = {
	.owner = THIS_MODULE,
	.open = smx3_fw_ver_verify_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release,
};
#endif
/*---For SMx3 use end---*/

/*******************************************************
Description:
	Novatek touchscreen extra function proc. file node
	initial function.

return:
	Executive outcomes. 0---succeed. -12---failed.
*******************************************************/
int32_t nvt_extra_proc_init(void)
{
	NVT_proc_fw_version_entry = proc_create(NVT_FW_VERSION, 0444, NULL,
		&nvt_fw_version_fops);
	if (NVT_proc_fw_version_entry != NULL) {
		TP_LOGD("create proc/nvt_fw_version Succeeded!");
	} else {
		TP_LOGE("create proc/nvt_fw_version Failed!");
		goto add_proc_fail;
	}

	NVT_proc_baseline_entry = proc_create(NVT_BASELINE, 0444, NULL,
		&nvt_baseline_fops);
	if (NVT_proc_baseline_entry != NULL) {
		TP_LOGD("create proc/nvt_baseline Succeeded!");
	} else {
		TP_LOGE("create proc/nvt_baseline Failed!");
		goto add_proc_fail;
	}

	NVT_proc_raw_entry = proc_create(NVT_RAW, 0444, NULL, &nvt_raw_fops);
	if (NVT_proc_raw_entry != NULL) {
		TP_LOGD("create proc/nvt_raw Succeeded!");
	} else {
		TP_LOGE("create proc/nvt_raw Failed!");
		goto add_proc_fail;
	}

	NVT_proc_diff_entry = proc_create(NVT_DIFF, 0444, NULL, &nvt_diff_fops);
	if (NVT_proc_diff_entry != NULL) {
		TP_LOGD("create proc/nvt_diff Succeeded!");
	} else {
		TP_LOGE("create proc/nvt_diff Failed!");
		goto add_proc_fail;
	}

	/*---For SMx3 use start---*/
	NVT_proc_before_diff_entry = proc_create(NVT_BEFORE_DIFF, 0444, NULL,
		&nvt_before_diff_fops);
	if (NVT_proc_before_diff_entry != NULL) {
		TP_LOGD("create proc/nvt_before_diff Succeeded!");
	} else {
		TP_LOGE("create proc/nvt_before_diff Failed!");
		goto add_proc_fail;
	}
#if NVT_PROJECT_INFO_READ
	NVT_proc_projectinfo_entry = proc_create(NVT_PROJECT_INFO, 0444, NULL,
		&nvt_projectinfo_fops);
	if (NVT_proc_projectinfo_entry != NULL) {
		TP_LOGD("create proc/nvt_projectinfo Succeeded!");
	} else {
		TP_LOGE("create proc/nvt_projectinfo Failed!");
		goto add_proc_fail;
	}
#endif

	SMx3_proc_fw_ver_check_entry = proc_create(NVT_FW_VER_CHECK, 0444, NULL,
		&smx3_fw_ver_check_fops);
	if (SMx3_proc_fw_ver_check_entry != NULL) {
		TP_LOGD("create proc/smx3_fw_ver_check Succeeded!");
	} else {
		TP_LOGE("create proc/smx3_fw_ver_check Failed!");
		goto add_proc_fail;
	}

#if BOOT_UPDATE_FIRMWARE
	SMx3_proc_fw_ver_verify_entry = proc_create(NVT_FW_VER_VERIFY, 0444,
		NULL, &smx3_fw_ver_verify_fops);
	if (SMx3_proc_fw_ver_verify_entry != NULL) {
		TP_LOGD("create proc/smx3_fw_ver_verify Succeeded!");
	} else {
		TP_LOGE("create proc/smx3_fw_ver_verify Failed!");
		goto add_proc_fail;
	}
#endif

	TP_LOGI("create extra proc node Succeeded!");
	/*---For SMx3 use end---*/

	return 0;

	/*---For SMx3 use start---*/
add_proc_fail:
	return -ENOMEM;
	/*---For SMx3 use end---*/
}
#endif

/*******************************************************
 *Description:
 *	SMx3 touchscreen /proc/smx3_probe_log open function.
 *
 *return:
 *	Executive outcomes. 0---succeed. negative---failed.
 *******************************************************/
static int32_t c_smx3_probe_log_show(struct seq_file *m, void *v)
{
	seq_printf(m, "******start******\n%s*******end*******\n", tp_probe_log);
	return 0;
}

const struct seq_operations smx3_probe_log_seq_ops = {
	.start  = c_start,
	.next   = c_next,
	.stop   = c_stop,
	.show   = c_smx3_probe_log_show
};

static int32_t smx3_probe_log_open(struct inode *inode, struct file *file)
{
	return seq_open(file, &smx3_probe_log_seq_ops);
}

static const struct file_operations smx3_probe_log_fops = {
	.owner = THIS_MODULE,
	.open = smx3_probe_log_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release,
};

int32_t smx3_probe_log_proc_init(void)
{
	int32_t ret = 0;

	SMx3_proc_probe_log_entry =
		proc_create(NVT_PROBE_LOG, 0444, NULL, &smx3_probe_log_fops);
	if (SMx3_proc_probe_log_entry == NULL) {
		TP_LOGE("create proc/smx3_probe_log Failed!");
		ret = -ENOMEM;
	} else {
		TP_LOGI("create proc/smx3_probe_log Succeeded!");
	}

	return ret;
}
