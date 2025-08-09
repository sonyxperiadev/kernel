/*
 * Copyright (C) 2010 - 2017 Novatek, Inc.
 *
 * $Revision: 24784 $
 * $Date: 2018-03-19 19:24:29 +0800 (Mon, 19 Mar 2018) $
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
#include <linux/slab.h>
#include <asm/uaccess.h>

#include "nt36xxx.h"
#include "nt36xxx_mp_ctrlram.h"

#if NVT_TOUCH_MP

#define NORMAL_MODE 0x00
#define TEST_MODE_1 0x21
#define TEST_MODE_2 0x22
#define MP_MODE_CC 0x41
#define FREQ_HOP_DISABLE 0x66
#define FREQ_HOP_ENABLE 0x65

#define SHORT_TEST_CSV_FILE "/sdcard/ShortTest.csv"
#define OPEN_TEST_CSV_FILE "/sdcard/OpenTest.csv"
#define FW_RAWDATA_CSV_FILE "/sdcard/FWMutualTest.csv"
#define FW_CC_CSV_FILE "/sdcard/FWCCTest.csv"
#define NOISE_TEST_CSV_FILE "/sdcard/NoiseTest.csv"

#define nvt_mp_seq_printf(m, fmt, args...) do {	\
	seq_printf(m, fmt, ##args);	\
	if (!nvt_mp_test_result_printed)	\
		printk(fmt, ##args);	\
} while (0)

static uint8_t *RecordResult_Short = NULL;
static uint8_t *RecordResult_Short_Diff = NULL;
static uint8_t *RecordResult_Short_Base = NULL;
static uint8_t *RecordResult_Open = NULL;
static uint8_t *RecordResult_FWMutual = NULL;
static uint8_t *RecordResult_FW_CC = NULL;
static uint8_t *RecordResult_FW_CC_I = NULL;
static uint8_t *RecordResult_FW_CC_Q = NULL;
static uint8_t *RecordResult_FW_DiffMax = NULL;
static uint8_t *RecordResult_FW_DiffMin = NULL;

static int32_t TestResult_Short = 0;
static int32_t TestResult_Short_Diff = 0;
static int32_t TestResult_Short_Base = 0;
static int32_t TestResult_Open = 0;
static int32_t TestResult_FW_Rawdata = 0;
static int32_t TestResult_FWMutual = 0;
static int32_t TestResult_FW_CC = 0;
static int32_t TestResult_FW_CC_I = 0;
static int32_t TestResult_FW_CC_Q = 0;
static int32_t TestResult_Noise = 0;
static int32_t TestResult_FW_DiffMax = 0;
static int32_t TestResult_FW_DiffMin = 0;

static int32_t *RawData_Short = NULL;
static int32_t *RawData_Short_Diff = NULL;
static int32_t *RawData_Short_Base = NULL;
static int32_t *RawData_Open = NULL;
static int32_t *RawData_Diff = NULL;
static int32_t *RawData_Diff_Min = NULL;
static int32_t *RawData_Diff_Max = NULL;
static int32_t *RawData_FWMutual = NULL;
static int32_t *RawData_FW_CC = NULL;
static int32_t *RawData_FW_CC_I = NULL;
static int32_t *RawData_FW_CC_Q = NULL;

static struct proc_dir_entry *NVT_proc_selftest_entry = NULL;
static int8_t nvt_mp_test_result_printed = 0;

extern void nvt_change_mode(uint8_t mode);
extern uint8_t nvt_get_fw_pipe(void);
extern void nvt_read_mdata(uint32_t xdata_addr, uint32_t xdata_btn_addr);
extern void nvt_get_mdata(int32_t *buf, uint8_t *m_x_num, uint8_t *m_y_num);
int32_t nvt_mp_parse_dt(struct device_node *root, const char *node_compatible);

/*---For SMx3 t2 pre-test start---*/
#define SHORT_PRETEST_CSV_FILE "/sdcard/preShortTest.csv"
#define OPEN_PRETEST_CSV_FILE "/sdcard/preOpenTest.csv"
static struct proc_dir_entry *NVT_proc_preselftest_entry;
bool isPreTest;
bool isGetCriteria;
/*---For SMx3 t2 pre-test end---*/

/*******************************************************
 *Description:
 *	Novatek touchscreen allocate buffer for mp selftest.
 *
 *return:
 *	Executive outcomes. 0---succeed. -12---Out of memory
 *
 *To reduce memory use, SMx3 modify this function to
 *avoid allocating memory for non-use array
 *******************************************************/
static int nvt_mp_buffer_init(void)
{
	size_t RecordResult_BufSize = IC_X_CFG_SIZE * IC_Y_CFG_SIZE + IC_KEY_CFG_SIZE;
	size_t RawData_BufSize = (IC_X_CFG_SIZE * IC_Y_CFG_SIZE + IC_KEY_CFG_SIZE) * sizeof(int32_t);
	char *error_tag = "kzalloc for ";

	RecordResult_Short = kzalloc(RecordResult_BufSize, GFP_KERNEL);
	if (!RecordResult_Short) {
		TP_LOGE("%sRecordResult_Short failed!", error_tag);
		goto alloc_fail;
	}

	RecordResult_Open = kzalloc(RecordResult_BufSize, GFP_KERNEL);
	if (!RecordResult_Open) {
		TP_LOGE("%sRecordResult_Open failed!", error_tag);
		goto alloc_fail;
	}

	if (ts->carrier_system) {
		RecordResult_Short_Diff = RecordResult_Short;

		RecordResult_Short_Base =
			kzalloc(RecordResult_BufSize, GFP_KERNEL);
		if (!RecordResult_Short_Base) {
			TP_LOGE("%sRecordResult_Short_Base failed!", error_tag);
			goto alloc_fail;
		}
	}

	if (!isPreTest) {
		RecordResult_FWMutual =
			kzalloc(RecordResult_BufSize, GFP_KERNEL);
		if (!RecordResult_FWMutual) {
			TP_LOGE("%sRecordResult_FWMutual failed!", error_tag);
			goto alloc_fail;
		}

		RecordResult_FW_CC =
			kzalloc(RecordResult_BufSize, GFP_KERNEL);
		if (!RecordResult_FW_CC) {
			TP_LOGE("%sRecordResult_FW_CC failed!", error_tag);
			goto alloc_fail;
		}

		RecordResult_FW_DiffMax =
			kzalloc(RecordResult_BufSize, GFP_KERNEL);
		if (!RecordResult_FW_DiffMax) {
			TP_LOGE("%sRecordResult_FW_DiffMax failed!", error_tag);
			goto alloc_fail;
		}

		RecordResult_FW_DiffMin =
			kzalloc(RecordResult_BufSize, GFP_KERNEL);
		if (!RecordResult_FW_DiffMin) {
			TP_LOGE("%sRecordResult_FW_DiffMin failed!", error_tag);
			goto alloc_fail;
		}

		if (ts->carrier_system) {
			RecordResult_FW_CC_I = RecordResult_FW_CC;

			RecordResult_FW_CC_Q =
				kzalloc(RecordResult_BufSize, GFP_KERNEL);
			if (!RecordResult_FW_CC_Q) {
				TP_LOGE("%sRecordResult_FW_CC_Q failed!",
					error_tag);
				goto alloc_fail;
			}
		}
	}

	RawData_Short = kzalloc(RawData_BufSize, GFP_KERNEL);
	if (!RawData_Short) {
		TP_LOGE("%sRawData_Short failed!", error_tag);
		goto alloc_fail;
	}

	RawData_Open = kzalloc(RawData_BufSize, GFP_KERNEL);
	if (!RawData_Open) {
		TP_LOGE("%sRawData_Open failed!", error_tag);
		goto alloc_fail;
	}

	if (ts->carrier_system) {
		RawData_Short_Diff = RawData_Short;

		RawData_Short_Base = kzalloc(RawData_BufSize, GFP_KERNEL);
		if (!RawData_Short_Base) {
			TP_LOGE("%sRawData_Short_Base failed!", error_tag);
			goto alloc_fail;
		}
	}

	if (!isPreTest) {
		RawData_Diff = kzalloc(RawData_BufSize, GFP_KERNEL);
		if (!RawData_Diff) {
			TP_LOGE("%sRawData_Diff failed!", error_tag);
			goto alloc_fail;
		}

		RawData_Diff_Min = kzalloc(RawData_BufSize, GFP_KERNEL);
		if (!RawData_Diff_Min) {
			TP_LOGE("%sRawData_Diff_Min failed!", error_tag);
			goto alloc_fail;
		}

		RawData_Diff_Max = kzalloc(RawData_BufSize, GFP_KERNEL);
		if (!RawData_Diff_Max) {
			TP_LOGE("%sRawData_Diff_Max failed!", error_tag);
			goto alloc_fail;
		}

		RawData_FWMutual = kzalloc(RawData_BufSize, GFP_KERNEL);
		if (!RawData_FWMutual) {
			TP_LOGE("%sRawData_FWMutual failed!", error_tag);
			goto alloc_fail;
		}

		RawData_FW_CC = kzalloc(RawData_BufSize, GFP_KERNEL);
		if (!RawData_FW_CC) {
			TP_LOGE("%sRawData_FW_CC failed!", error_tag);
			goto alloc_fail;
		}

		if (ts->carrier_system) {
			RawData_FW_CC_I = RawData_FW_CC;

			RawData_FW_CC_Q = kzalloc(RawData_BufSize, GFP_KERNEL);
			if (!RawData_FW_CC_Q) {
				TP_LOGE("%sRawData_FW_CC_Q failed!", error_tag);
				goto alloc_fail;
			}
		}
	}

	return 0;

alloc_fail:
	return -ENOMEM;
}

/*---For SMx3 self-test start---*/
/*******************************************************
 *Description:
 *	SMx3 touchscreen release buffer for mp selftest.
 *
 *return:
 *	Executive outcomes. 0---succeed.
 *******************************************************/
static int smx3_mp_buffer_free(void)
{
	kfree(RecordResult_Short);
	kfree(RecordResult_Open);
	kfree(RawData_Short);
	kfree(RawData_Open);
	if (ts->carrier_system) {
		kfree(RecordResult_Short_Base);
		kfree(RawData_Short_Base);
	}

	if (!isPreTest) {
		kfree(RecordResult_FWMutual);
		kfree(RecordResult_FW_CC);
		kfree(RecordResult_FW_DiffMax);
		kfree(RecordResult_FW_DiffMin);
		kfree(RawData_Diff);
		kfree(RawData_Diff_Min);
		kfree(RawData_Diff_Max);
		kfree(RawData_FWMutual);
		kfree(RawData_FW_CC);
		if (ts->carrier_system) {
			kfree(RecordResult_FW_CC_Q);
			kfree(RawData_FW_CC_Q);
		}
	}

	return 0;
}
/*---For SMx3 self-test end---*/

/*******************************************************
Description:
	Novatek touchscreen self-test criteria print function.

return:
	n.a.
*******************************************************/
#if PRINT_DT_CRITERIA
static void nvt_print_lmt_array(int32_t *array, int32_t x_ch, int32_t y_ch)
{
	int32_t i = 0;
	int32_t j = 0;
#if TOUCH_KEY_NUM > 0
	int32_t k = 0;
#endif /* #if TOUCH_KEY_NUM > 0 */

	for (j = 0; j < y_ch; j++) {
		for(i = 0; i < x_ch; i++) {
			printk("%5d, ", array[j * x_ch + i]);
		}
		printk("\n");
	}
#if TOUCH_KEY_NUM > 0
	for (k = 0; k < Key_Channel; k++) {
		printk("%5d, ", array[y_ch * x_ch + k]);
	}
	printk("\n");
#endif /* #if TOUCH_KEY_NUM > 0 */
}

static void nvt_print_criteria(void)
{
	TP_LOGD("+++");

	if (ts->carrier_system) {
		//---PS_Config_Lmt_Short_Diff---
		printk("PS_Config_Lmt_Short_Diff_P:\n");
		nvt_print_lmt_array(PS_Config_Lmt_Short_Diff_P, X_Channel, Y_Channel);
		printk("PS_Config_Lmt_Short_Diff_N:\n");
		nvt_print_lmt_array(PS_Config_Lmt_Short_Diff_N, X_Channel, Y_Channel);
		//---PS_Config_Lmt_Short_Base---
		printk("PS_Config_Lmt_Short_Base_P:\n");
		nvt_print_lmt_array(PS_Config_Lmt_Short_Base_P, X_Channel, Y_Channel);
		printk("PS_Config_Lmt_Short_Base_N:\n");
		nvt_print_lmt_array(PS_Config_Lmt_Short_Base_N, X_Channel, Y_Channel);
	} else {
		//---PS_Config_Lmt_Short_Rawdata---
		printk("PS_Config_Lmt_Short_Rawdata_P:\n");
		nvt_print_lmt_array(PS_Config_Lmt_Short_Rawdata_P, X_Channel, Y_Channel);
		printk("PS_Config_Lmt_Short_Rawdata_N:\n");
		nvt_print_lmt_array(PS_Config_Lmt_Short_Rawdata_N, X_Channel, Y_Channel);
	}

	//---PS_Config_Lmt_Open_Rawdata---
	printk("PS_Config_Lmt_Open_Rawdata_P:\n");
	nvt_print_lmt_array(PS_Config_Lmt_Open_Rawdata_P, X_Channel, Y_Channel);
	printk("PS_Config_Lmt_Open_Rawdata_N:\n");
	nvt_print_lmt_array(PS_Config_Lmt_Open_Rawdata_N, X_Channel, Y_Channel);

	//---PS_Config_Lmt_FW_Rawdata---
	printk("PS_Config_Lmt_FW_Rawdata_P:\n");
	nvt_print_lmt_array(PS_Config_Lmt_FW_Rawdata_P, X_Channel, Y_Channel);
	printk("PS_Config_Lmt_FW_Rawdata_N:\n");
	nvt_print_lmt_array(PS_Config_Lmt_FW_Rawdata_N, X_Channel, Y_Channel);

	if (ts->carrier_system) {
		//---PS_Config_Lmt_FW_CC_I---
		printk("PS_Config_Lmt_FW_CC_I_P:\n");
		nvt_print_lmt_array(PS_Config_Lmt_FW_CC_I_P, X_Channel, Y_Channel);
		printk("PS_Config_Lmt_FW_CC_I_N:\n");
		nvt_print_lmt_array(PS_Config_Lmt_FW_CC_I_N, X_Channel, Y_Channel);
		//---PS_Config_Lmt_FW_CC_Q---
		printk("PS_Config_Lmt_FW_CC_Q_P:\n");
		nvt_print_lmt_array(PS_Config_Lmt_FW_CC_Q_P, X_Channel, Y_Channel);
		printk("PS_Config_Lmt_FW_CC_Q_N:\n");
		nvt_print_lmt_array(PS_Config_Lmt_FW_CC_Q_N, X_Channel, Y_Channel);
	} else {
		//---PS_Config_Lmt_FW_CC---
		printk("PS_Config_Lmt_FW_CC_P:\n");
		nvt_print_lmt_array(PS_Config_Lmt_FW_CC_P, X_Channel, Y_Channel);
		printk("PS_Config_Lmt_FW_CC_N:\n");
		nvt_print_lmt_array(PS_Config_Lmt_FW_CC_N, X_Channel, Y_Channel);
	}

	//---PS_Config_Lmt_FW_Diff---
	printk("PS_Config_Lmt_FW_Diff_P:\n");
	nvt_print_lmt_array(PS_Config_Lmt_FW_Diff_P, X_Channel, Y_Channel);
	printk("PS_Config_Lmt_FW_Diff_N:\n");
	nvt_print_lmt_array(PS_Config_Lmt_FW_Diff_N, X_Channel, Y_Channel);

	TP_LOGD("---");
}
#endif /* #if PRINT_DT_CRITERIA */

uint32_t file_offset = -1;
static int32_t nvt_save_rawdata_to_csv(int32_t *rawdata, uint8_t *result,
	bool isPass, uint8_t x_ch, uint8_t y_ch, const char *file_path,
	uint32_t offset)
{
	int32_t x = 0;
	int32_t y = 0;
	int32_t iArrayIndex = 0;
	struct file *fp = NULL;
	char *fbufp = NULL;
	mm_segment_t org_fs;
	int32_t write_ret = 0;
	uint32_t output_len = 0;
	loff_t pos = 0;
#if TOUCH_KEY_NUM > 0
	int32_t k = 0;
	int32_t keydata_output_offset = 0;
#endif /* #if TOUCH_KEY_NUM > 0 */
	uint32_t raw_array_size = (y_ch * x_ch * 7 + y_ch * 2);
	uint32_t f_pos = 0;

	TP_LOGD("+++");
	fbufp = kzalloc(4096 * 4, GFP_KERNEL);
	if (!fbufp) {
		TP_LOGE("kzalloc for fbufp failed!");
		return -ENOMEM;
	}

	/* save test info to csv file */
	if (offset == 0) {
		/* result */
		snprintf(fbufp, 14, "Result, %s\r\n", isPass ? "Pass" : "Fail");
		f_pos += 14;
		/* project fw info. */
		snprintf(fbufp + f_pos, 14, "FW VER, 0x%02x\r\n", ts->fw_ver);
		f_pos += 14;
		snprintf(fbufp + f_pos, 17, "NVT PID, 0x%04x\r\n", ts->nvt_pid);
		f_pos += 17;
	}

	/* record result */
	snprintf(fbufp + f_pos, 17, "\r\nRecord Result\r\n");
	f_pos += 17;
	for (y = 0; y < y_ch; y++) {
		for (x = 0; x < x_ch; x++) {
			iArrayIndex = y * x_ch + x;
			/* pr_debug("%5d, ", result[iArrayIndex]); */
			snprintf(fbufp + f_pos + iArrayIndex * 7 + y * 2, 7,
				"%5d, ", result[iArrayIndex]);
		}
		/* pr_debug("\n"); */
		snprintf(fbufp + f_pos + (iArrayIndex + 1) * 7 + y * 2, 2,
			"\r\n");
	}
	f_pos += (raw_array_size + 2);

	/* rawdata */
	snprintf(fbufp + f_pos, 11, "\r\nRawdata\r\n");
	f_pos += 11;
	for (y = 0; y < y_ch; y++) {
		for (x = 0; x < x_ch; x++) {
			iArrayIndex = y * x_ch + x;
			/* pr_debug("%5d, ", rawdata[iArrayIndex]); */
			snprintf(fbufp + f_pos + iArrayIndex * 7 + y * 2, 7,
				"%5d, ", rawdata[iArrayIndex]);
		}
		/* pr_debug("\n"); */
		snprintf(fbufp + f_pos + (iArrayIndex + 1) * 7 + y * 2, 2,
			"\r\n");
	}
	f_pos += (raw_array_size + 2);

#if TOUCH_KEY_NUM > 0
	for (k = 0; k < Key_Channel; k++) {
		iArrayIndex = y_ch * x_ch + k;
		/* pr_debug("%5d, ", rawdata[iArrayIndex]); */
		snprintf(fbufp + f_pos + k * 7, 5, "%5d, ",
			rawdata[iArrayIndex]);
	}
	/* pr_debug("\n"); */
	snprintf(fbufp + f_pos + Key_Channel * 7, 2, "\r\n");
#endif /* #if TOUCH_KEY_NUM > 0 */

	org_fs = get_fs();
	set_fs(KERNEL_DS);
	fp = filp_open(file_path, O_RDWR | O_CREAT, 0644);
	if (fp == NULL || IS_ERR(fp)) {
		TP_LOGE("open %s failed", file_path);
		set_fs(org_fs);
		if (fbufp) {
			kfree(fbufp);
			fbufp = NULL;
		}
		return -1;
	}

	output_len = f_pos + Key_Channel * 7 + 2;
	/*  backup file offset for next result page (if need) */
	file_offset = output_len;

	pos = offset;
	write_ret = vfs_write(fp, (char __user *)fbufp, output_len, &pos);
	if (write_ret <= 0) {
		TP_LOGE("write %s failed", file_path);
		set_fs(org_fs);
		if (fp) {
			filp_close(fp, NULL);
			fp = NULL;
		}
		if (fbufp) {
			kfree(fbufp);
			fbufp = NULL;
		}
		return -1;
	}

	set_fs(org_fs);
	if (fp) {
		filp_close(fp, NULL);
		fp = NULL;
	}
	if (fbufp) {
		kfree(fbufp);
		fbufp = NULL;
	}

	TP_LOGD("---");

	return 0;
}

static int32_t nvt_polling_hand_shake_status(void)
{
	uint8_t buf[8] = {0};
	int32_t i = 0;
	const int32_t retry = 50;

	for (i = 0; i < retry; i++) {
		//---set xdata index to EVENT BUF ADDR---
		buf[0] = 0xFF;
		buf[1] = (ts->mmap->EVENT_BUF_ADDR >> 16) & 0xFF;
		buf[2] = (ts->mmap->EVENT_BUF_ADDR >> 8) & 0xFF;
		CTP_I2C_WRITE(ts->client, I2C_FW_Address, buf, 3);

		//---read fw status---
		buf[0] = EVENT_MAP_HANDSHAKING_or_SUB_CMD_BYTE;
		buf[1] = 0x00;
		CTP_I2C_READ(ts->client, I2C_FW_Address, buf, 2);

		if ((buf[1] == 0xA0) || (buf[1] == 0xA1))
			break;

		msleep(10);
	}

	if (i >= retry) {
		TP_LOGE("polling hand shake status failed, buf[1]=0x%02X",
			buf[1]);

		// Read back 5 bytes from offset EVENT_MAP_HOST_CMD for debug check
		buf[0] = 0xFF;
		buf[1] = (ts->mmap->EVENT_BUF_ADDR >> 16) & 0xFF;
		buf[2] = (ts->mmap->EVENT_BUF_ADDR >> 8) & 0xFF;
		CTP_I2C_WRITE(ts->client, I2C_FW_Address, buf, 3);

		buf[0] = EVENT_MAP_HOST_CMD;
		buf[1] = 0x00;
		buf[2] = 0x00;
		buf[3] = 0x00;
		buf[4] = 0x00;
		buf[5] = 0x00;
		CTP_I2C_READ(ts->client, I2C_FW_Address, buf, 6);
		TP_LOGE("Read back 5 bytes from offset EVENT_MAP_HOST_CMD:");
		TP_LOGE("0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X",
			buf[1], buf[2], buf[3], buf[4], buf[5]);

		return -1;
	} else {
		return 0;
	}
}

static int8_t nvt_switch_FreqHopEnDis(uint8_t FreqHopEnDis)
{
	uint8_t buf[8] = {0};
	uint8_t retry = 0;
	int8_t ret = 0;

	TP_LOGD("+++");

	for (retry = 0; retry < 20; retry++) {
		//---set xdata index to EVENT BUF ADDR---
		buf[0] = 0xFF;
		buf[1] = (ts->mmap->EVENT_BUF_ADDR >> 16) & 0xFF;
		buf[2] = (ts->mmap->EVENT_BUF_ADDR >> 8) & 0xFF;
		CTP_I2C_WRITE(ts->client, I2C_FW_Address, buf, 3);

		//---switch FreqHopEnDis---
		buf[0] = EVENT_MAP_HOST_CMD;
		buf[1] = FreqHopEnDis;
		CTP_I2C_WRITE(ts->client, I2C_FW_Address, buf, 2);

		msleep(35);

		buf[0] = EVENT_MAP_HOST_CMD;
		buf[1] = 0xFF;
		CTP_I2C_READ(ts->client, I2C_FW_Address, buf, 2);

		if (buf[1] == 0x00)
			break;
	}

	if (unlikely(retry == 20)) {
		TP_LOGE("switch FreqHopEnDis 0x%02X failed, buf[1]=0x%02X",
			FreqHopEnDis, buf[1]);
		ret = -1;
	}

	TP_LOGD("---");

	return ret;
}

static int32_t nvt_read_baseline(int32_t *xdata)
{
	uint8_t x_num = 0;
	uint8_t y_num = 0;
	uint32_t x = 0;
	uint32_t y = 0;
	int32_t iArrayIndex = 0;
#if TOUCH_KEY_NUM > 0
	int32_t k = 0;
#endif /* #if TOUCH_KEY_NUM > 0 */

	TP_LOGD("+++");

	nvt_read_mdata(ts->mmap->BASELINE_ADDR, ts->mmap->BASELINE_BTN_ADDR);

	nvt_get_mdata(xdata, &x_num, &y_num);

	for (y = 0; y < y_num; y++) {
		for (x = 0; x < x_num; x++) {
			iArrayIndex = y * x_num + x;
			if (ts->carrier_system) {
				xdata[iArrayIndex] = (uint16_t)xdata[iArrayIndex];
			} else {
				xdata[iArrayIndex] = (int16_t)xdata[iArrayIndex];
			}
		}
	}
#if TOUCH_KEY_NUM > 0
	for (k = 0; k < Key_Channel; k++) {
		iArrayIndex = Y_Channel * X_Channel + k;
		if (ts->carrier_system) {
			xdata[iArrayIndex] = (uint16_t)xdata[iArrayIndex];
		} else {
			xdata[iArrayIndex] = (int16_t)xdata[iArrayIndex];
		}
	}
#endif /* #if TOUCH_KEY_NUM > 0 */

	TP_LOGD("---");

	return 0;
}

static int32_t nvt_read_CC(int32_t *xdata)
{
	uint8_t x_num = 0;
	uint8_t y_num = 0;
	uint32_t x = 0;
	uint32_t y = 0;
	int32_t iArrayIndex = 0;
	int32_t xdata_tmp = 0;
#if TOUCH_KEY_NUM > 0
	int32_t k = 0;
#endif /* #if TOUCH_KEY_NUM > 0 */

	TP_LOGD("+++");

	if (nvt_get_fw_pipe() == 0)
		nvt_read_mdata(ts->mmap->DIFF_PIPE1_ADDR, ts->mmap->DIFF_BTN_PIPE1_ADDR);
	else
		nvt_read_mdata(ts->mmap->DIFF_PIPE0_ADDR, ts->mmap->DIFF_BTN_PIPE0_ADDR);

	nvt_get_mdata(xdata, &x_num, &y_num);

	for (y = 0; y < y_num; y++) {
		for (x = 0; x < x_num; x++) {
			iArrayIndex = y * x_num + x;
			if (ts->carrier_system) {
				xdata_tmp = xdata[iArrayIndex];
				RawData_FW_CC_I[iArrayIndex] = (uint8_t)(xdata_tmp & 0xFF);
				RawData_FW_CC_Q[iArrayIndex] = (uint8_t)((xdata_tmp >> 8) & 0xFF);
			} else {
				xdata[iArrayIndex] = (int16_t)xdata[iArrayIndex];
			}
		}
	}
#if TOUCH_KEY_NUM > 0
	for (k = 0; k < Key_Channel; k++) {
		iArrayIndex = Y_Channel * X_Channel + k;
		if (ts->carrier_system) {
			xdata_tmp = xdata[iArrayIndex];
			RawData_FW_CC_I[iArrayIndex] = (uint8_t)(xdata_tmp & 0xFF);
			RawData_FW_CC_Q[iArrayIndex] = (uint8_t)((xdata_tmp >> 8) & 0xFF);
		} else {
			xdata[iArrayIndex] = (int16_t)xdata[iArrayIndex];
		}
	}
#endif /* #if TOUCH_KEY_NUM > 0 */

	TP_LOGD("---");

	return 0;
}

static void nvt_enable_noise_collect(int32_t frame_num)
{
	uint8_t buf[8] = {0};

	//---set xdata index to EVENT BUF ADDR---
	buf[0] = 0xFF;
	buf[1] = (ts->mmap->EVENT_BUF_ADDR >> 16) & 0xFF;
	buf[2] = (ts->mmap->EVENT_BUF_ADDR >> 8) & 0xFF;
	CTP_I2C_WRITE(ts->client, I2C_FW_Address, buf, 3);

	//---enable noise collect---
	buf[0] = EVENT_MAP_HOST_CMD;
	buf[1] = 0x47;
	buf[2] = 0xAA;
	buf[3] = frame_num;
	buf[4] = 0x00;
	CTP_I2C_WRITE(ts->client, I2C_FW_Address, buf, 5);
}

static int32_t nvt_read_fw_noise(int32_t *xdata)
{
	uint8_t x_num = 0;
	uint8_t y_num = 0;
	uint32_t x = 0;
	uint32_t y = 0;
	int32_t iArrayIndex = 0;
	int32_t frame_num = 0;
#if TOUCH_KEY_NUM > 0
	int32_t k = 0;
#endif /* #if TOUCH_KEY_NUM > 0 */

	TP_LOGD("+++");

	//---Enter Test Mode---
	if (nvt_clear_fw_status()) {
		return -EAGAIN;
	}

	frame_num = PS_Config_Diff_Test_Frame / 10;
	if (frame_num <= 0)
		frame_num = 1;
	TP_LOGD("frame_num=%d", frame_num);
	nvt_enable_noise_collect(frame_num);
	// need wait PS_Config_Diff_Test_Frame * 8.3ms
	msleep(frame_num * 83);

	if (nvt_polling_hand_shake_status()) {
		return -EAGAIN;
	}

	if (nvt_get_fw_info()) {
		return -EAGAIN;
	}

	if (nvt_get_fw_pipe() == 0)
		nvt_read_mdata(ts->mmap->DIFF_PIPE0_ADDR, ts->mmap->DIFF_BTN_PIPE0_ADDR);
	else
		nvt_read_mdata(ts->mmap->DIFF_PIPE1_ADDR, ts->mmap->DIFF_BTN_PIPE1_ADDR);

	nvt_get_mdata(xdata, &x_num, &y_num);

	for (y = 0; y < y_num; y++) {
		for (x = 0; x < x_num; x++) {
			iArrayIndex = y * x_num + x;
			if (ts->carrier_system) {
				RawData_Diff_Max[iArrayIndex] = (uint16_t)xdata[iArrayIndex];
				RawData_Diff_Min[iArrayIndex] = 0;
			} else {
				RawData_Diff_Max[iArrayIndex] = (int8_t)((xdata[iArrayIndex] >> 8) & 0xFF);
				RawData_Diff_Min[iArrayIndex] = (int8_t)(xdata[iArrayIndex] & 0xFF);
			}
		}
	}
#if TOUCH_KEY_NUM > 0
	for (k = 0; k < Key_Channel; k++) {
		iArrayIndex = Y_Channel * X_Channel + k;
		if (ts->carrier_system) {
			RawData_Diff_Max[iArrayIndex] = (uint16_t)xdata[iArrayIndex];
			RawData_Diff_Min[iArrayIndex] = 0;
		} else {
			RawData_Diff_Max[iArrayIndex] = (int8_t)((xdata[iArrayIndex] >> 8) & 0xFF);
			RawData_Diff_Min[iArrayIndex] = (int8_t)(xdata[iArrayIndex] & 0xFF);
		}
	}
#endif /* #if TOUCH_KEY_NUM > 0 */

	//---Leave Test Mode---
	nvt_change_mode(NORMAL_MODE);

	TP_LOGD("---");

	return 0;
}

static void nvt_enable_open_test(void)
{
	uint8_t buf[8] = {0};

	//---set xdata index to EVENT BUF ADDR---
	buf[0] = 0xFF;
	buf[1] = (ts->mmap->EVENT_BUF_ADDR >> 16) & 0xFF;
	buf[2] = (ts->mmap->EVENT_BUF_ADDR >> 8) & 0xFF;
	CTP_I2C_WRITE(ts->client, I2C_FW_Address, buf, 3);

	//---enable open test---
	buf[0] = EVENT_MAP_HOST_CMD;
	buf[1] = 0x45;
	buf[2] = 0xAA;
	buf[3] = 0x02;
	buf[4] = 0x00;
	CTP_I2C_WRITE(ts->client, I2C_FW_Address, buf, 5);
}

static void nvt_enable_short_test(void)
{
	uint8_t buf[8] = {0};

	//---set xdata index to EVENT BUF ADDR---
	buf[0] = 0xFF;
	buf[1] = (ts->mmap->EVENT_BUF_ADDR >> 16) & 0xFF;
	buf[2] = (ts->mmap->EVENT_BUF_ADDR >> 8) & 0xFF;
	CTP_I2C_WRITE(ts->client, I2C_FW_Address, buf, 3);

	//---enable short test---
	buf[0] = EVENT_MAP_HOST_CMD;
	buf[1] = 0x43;
	buf[2] = 0xAA;
	buf[3] = 0x02;
	buf[4] = 0x00;
	CTP_I2C_WRITE(ts->client, I2C_FW_Address, buf, 5);
}

static int32_t nvt_read_fw_open(int32_t *xdata)
{
	uint32_t raw_pipe_addr = 0;
	uint8_t *rawdata_buf = NULL;
	uint32_t x = 0;
	uint32_t y = 0;
	uint8_t buf[128] = {0};
#if TOUCH_KEY_NUM > 0
	uint32_t raw_btn_pipe_addr = 0;
	int32_t k = 0;
#endif /* #if TOUCH_KEY_NUM > 0 */

	TP_LOGD("+++");

	//---Enter Test Mode---
	if (nvt_clear_fw_status()) {
		return -EAGAIN;
	}

	nvt_enable_open_test();

	if (nvt_polling_hand_shake_status()) {
		return -EAGAIN;
	}

#if TOUCH_KEY_NUM > 0
	rawdata_buf = (uint8_t *)kzalloc((IC_X_CFG_SIZE * IC_Y_CFG_SIZE + IC_KEY_CFG_SIZE) * 2, GFP_KERNEL);
#else
	rawdata_buf = (uint8_t *)kzalloc(IC_X_CFG_SIZE * IC_Y_CFG_SIZE * 2, GFP_KERNEL);
#endif /* #if TOUCH_KEY_NUM > 0 */
	if (!rawdata_buf) {
		TP_LOGE("kzalloc for rawdata_buf failed!");
		return -ENOMEM;
	}

	if (nvt_get_fw_pipe() == 0)
		raw_pipe_addr = ts->mmap->RAW_PIPE0_ADDR;
	else
		raw_pipe_addr = ts->mmap->RAW_PIPE1_ADDR;

	for (y = 0; y < IC_Y_CFG_SIZE; y++) {
		//---change xdata index---
		buf[0] = 0xFF;
		buf[1] = (uint8_t)(((raw_pipe_addr + y * IC_X_CFG_SIZE * 2) >> 16) & 0xFF);
		buf[2] = (uint8_t)(((raw_pipe_addr + y * IC_X_CFG_SIZE * 2) >> 8) & 0xFF);
		CTP_I2C_WRITE(ts->client, I2C_FW_Address, buf, 3);
		buf[0] = (uint8_t)((raw_pipe_addr + y * IC_X_CFG_SIZE * 2) & 0xFF);
		CTP_I2C_READ(ts->client, I2C_FW_Address, buf, IC_X_CFG_SIZE * 2 + 1);
		memcpy(rawdata_buf + y * IC_X_CFG_SIZE * 2, buf + 1, IC_X_CFG_SIZE * 2);
	}
#if TOUCH_KEY_NUM > 0
	if (nvt_get_fw_pipe() == 0)
		raw_btn_pipe_addr = ts->mmap->RAW_BTN_PIPE0_ADDR;
	else
		raw_btn_pipe_addr = ts->mmap->RAW_BTN_PIPE1_ADDR;

	//---change xdata index---
	buf[0] = 0xFF;
	buf[1] = (uint8_t)((raw_btn_pipe_addr >> 16) & 0xFF);
	buf[2] = (uint8_t)((raw_btn_pipe_addr >> 8) & 0xFF);
	CTP_I2C_WRITE(ts->client, I2C_FW_Address, buf, 3);
	buf[0] = (uint8_t)(raw_btn_pipe_addr & 0xFF);
	CTP_I2C_READ(ts->client, I2C_FW_Address, buf, IC_KEY_CFG_SIZE * 2 + 1);
	memcpy(rawdata_buf + IC_Y_CFG_SIZE * IC_X_CFG_SIZE * 2, buf + 1, IC_KEY_CFG_SIZE * 2);
#endif /* #if TOUCH_KEY_NUM > 0 */

	for (y = 0; y < IC_Y_CFG_SIZE; y++) {
		for (x = 0; x < IC_X_CFG_SIZE; x++) {
			if ((AIN_Y[y] != 0xFF) && (AIN_X[x] != 0xFF)) {
				xdata[AIN_Y[y] * X_Channel + AIN_X[x]] = (int16_t)((rawdata_buf[(y * IC_X_CFG_SIZE + x) * 2] + 256 * rawdata_buf[(y * IC_X_CFG_SIZE + x) * 2 + 1]));
			}
		}
	}
#if TOUCH_KEY_NUM > 0
	for (k = 0; k < IC_KEY_CFG_SIZE; k++) {
		if (AIN_KEY[k] != 0xFF)
			xdata[Y_Channel * X_Channel + AIN_KEY[k]] = (int16_t)(rawdata_buf[(IC_Y_CFG_SIZE * IC_X_CFG_SIZE + k) * 2] + 256 * rawdata_buf[(IC_Y_CFG_SIZE * IC_X_CFG_SIZE + k) * 2 + 1]);
	}
#endif /* #if TOUCH_KEY_NUM > 0 */

	if (rawdata_buf) {
		kfree(rawdata_buf);
		rawdata_buf = NULL;
	}

	//---Leave Test Mode---
	nvt_change_mode(NORMAL_MODE);

	TP_LOGD("---");

	return 0;
}

static int32_t nvt_read_fw_short(int32_t *xdata)
{
	uint32_t raw_pipe_addr = 0;
	uint8_t *rawdata_buf = NULL;
	uint32_t x = 0;
	uint32_t y = 0;
	uint8_t buf[128] = {0};
	int32_t iArrayIndex = 0;
#if TOUCH_KEY_NUM > 0
	uint32_t raw_btn_pipe_addr = 0;
	int32_t k = 0;
#endif /* #if TOUCH_KEY_NUM > 0 */

	TP_LOGD("+++");

	//---Enter Test Mode---
	if (nvt_clear_fw_status()) {
		return -EAGAIN;
	}

	nvt_enable_short_test();

	if (nvt_polling_hand_shake_status()) {
		return -EAGAIN;
	}

#if TOUCH_KEY_NUM > 0
    rawdata_buf = (uint8_t *)kzalloc((X_Channel * Y_Channel + Key_Channel) * 2, GFP_KERNEL);
#else
    rawdata_buf = (uint8_t *)kzalloc(X_Channel * Y_Channel * 2, GFP_KERNEL);
#endif /* #if TOUCH_KEY_NUM > 0 */
	if (!rawdata_buf) {
		TP_LOGE("kzalloc for rawdata_buf failed!");
		return -ENOMEM;
	}

	if (ts->carrier_system) {
		// to get short diff rawdata at pipe0
		raw_pipe_addr = ts->mmap->RAW_PIPE0_ADDR;
	} else {
		if (nvt_get_fw_pipe() == 0)
			raw_pipe_addr = ts->mmap->RAW_PIPE0_ADDR;
		else
			raw_pipe_addr = ts->mmap->RAW_PIPE1_ADDR;
	}

	for (y = 0; y < Y_Channel; y++) {
		//---change xdata index---
		buf[0] = 0xFF;
		buf[1] = (uint8_t)(((raw_pipe_addr + y * X_Channel * 2) >> 16) & 0xFF);
		buf[2] = (uint8_t)(((raw_pipe_addr + y * X_Channel * 2) >> 8) & 0xFF);
		CTP_I2C_WRITE(ts->client, I2C_FW_Address, buf, 3);
		buf[0] = (uint8_t)((raw_pipe_addr + y * X_Channel * 2) & 0xFF);
		CTP_I2C_READ(ts->client, I2C_FW_Address, buf, X_Channel * 2 + 1);
		memcpy(rawdata_buf + y * X_Channel * 2, buf + 1, X_Channel * 2);
	}
#if TOUCH_KEY_NUM > 0
	if (ts->carrier_system) {
		// to get button short diff rawdata at pipe0
		raw_btn_pipe_addr = ts->mmap->RAW_BTN_PIPE0_ADDR;
	} else {
		if (nvt_get_fw_pipe() == 0)
			raw_btn_pipe_addr = ts->mmap->RAW_BTN_PIPE0_ADDR;
		else
			raw_btn_pipe_addr = ts->mmap->RAW_BTN_PIPE1_ADDR;
	}

    //---change xdata index---
	buf[0] = 0xFF;
	buf[1] = (uint8_t)((raw_btn_pipe_addr >> 16) & 0xFF);
	buf[2] = (uint8_t)((raw_btn_pipe_addr >> 8) & 0xFF);
	CTP_I2C_WRITE(ts->client, I2C_FW_Address, buf, 3);
	buf[0] = (uint8_t)(raw_btn_pipe_addr & 0xFF);
	CTP_I2C_READ(ts->client, I2C_FW_Address, buf, Key_Channel * 2 + 1);
	memcpy(rawdata_buf + Y_Channel * X_Channel * 2, buf + 1, Key_Channel * 2);
#endif /* #if TOUCH_KEY_NUM > 0 */

	for (y = 0; y < Y_Channel; y++) {
		for (x = 0; x < X_Channel; x++) {
			iArrayIndex = y * X_Channel + x;
			xdata[iArrayIndex] = (int16_t)(rawdata_buf[iArrayIndex * 2] + 256 * rawdata_buf[iArrayIndex * 2 + 1]);
		}
	}
#if TOUCH_KEY_NUM > 0
	for (k = 0; k < Key_Channel; k++) {
		iArrayIndex = Y_Channel * X_Channel + k;
		xdata[iArrayIndex] = (int16_t)(rawdata_buf[iArrayIndex * 2] + 256 * rawdata_buf[iArrayIndex * 2 + 1]);
	}
#endif /* #if TOUCH_KEY_NUM > 0 */

	// for carrier sensing system to get short baseline rawdata
	if (ts->carrier_system) {
		// to get short baseline rawdata at pipe1
		raw_pipe_addr = ts->mmap->RAW_PIPE1_ADDR;

		for (y = 0; y < Y_Channel; y++) {
			//---change xdata index---
			buf[0] = 0xFF;
			buf[1] = (uint8_t)(((raw_pipe_addr + y * X_Channel * 2) >> 16) & 0xFF);
			buf[2] = (uint8_t)(((raw_pipe_addr + y * X_Channel * 2) >> 8) & 0xFF);
			CTP_I2C_WRITE(ts->client, I2C_FW_Address, buf, 3);
			buf[0] = (uint8_t)((raw_pipe_addr + y * X_Channel * 2) & 0xFF);
			CTP_I2C_READ(ts->client, I2C_FW_Address, buf, X_Channel * 2 + 1);
			memcpy(rawdata_buf + y * X_Channel * 2, buf + 1, X_Channel * 2);
		}
#if TOUCH_KEY_NUM > 0
		// to get button short baseline rawdata at pipe1
		raw_btn_pipe_addr = ts->mmap->RAW_BTN_PIPE1_ADDR;

	    //---change xdata index---
		buf[0] = 0xFF;
		buf[1] = (uint8_t)((raw_btn_pipe_addr >> 16) & 0xFF);
		buf[2] = (uint8_t)((raw_btn_pipe_addr >> 8) & 0xFF);
		CTP_I2C_WRITE(ts->client, I2C_FW_Address, buf, 3);
		buf[0] = (uint8_t)(raw_btn_pipe_addr & 0xFF);
		CTP_I2C_READ(ts->client, I2C_FW_Address, buf, Key_Channel * 2 + 1);
		memcpy(rawdata_buf + Y_Channel * X_Channel * 2, buf + 1, Key_Channel * 2);
#endif /* #if TOUCH_KEY_NUM > 0 */

		for (y = 0; y < Y_Channel; y++) {
			for (x = 0; x < X_Channel; x++) {
				iArrayIndex = y * X_Channel + x;
				RawData_Short_Base[iArrayIndex] = (int16_t)(rawdata_buf[iArrayIndex * 2] + 256 * rawdata_buf[iArrayIndex * 2 + 1]);
			}
		}
#if TOUCH_KEY_NUM > 0
		for (k = 0; k < Key_Channel; k++) {
			iArrayIndex = Y_Channel * X_Channel + k;
			RawData_Short_Base[iArrayIndex] = (int16_t)(rawdata_buf[iArrayIndex * 2] + 256 * rawdata_buf[iArrayIndex * 2 + 1]);
		}
#endif /* #if TOUCH_KEY_NUM > 0 */
	}

	if (rawdata_buf) {
		kfree(rawdata_buf);
		rawdata_buf = NULL;
	}

	//---Leave Test Mode---
	nvt_change_mode(NORMAL_MODE);

	TP_LOGD("---");

	return 0;
}

/*******************************************************
Description:
	Novatek touchscreen raw data test for each single point function.

return:
	Executive outcomes. 0---passed. negative---failed.
*******************************************************/
static int32_t RawDataTest_SinglePoint_Sub(int32_t rawdata[],
	uint8_t RecordResult[], uint8_t x_ch, uint8_t y_ch,
	int32_t Rawdata_Limit_Postive[], int32_t Rawdata_Limit_Negative[],
	const char *file_path, uint32_t offset)
{
	int32_t i = 0;
	int32_t j = 0;
#if TOUCH_KEY_NUM > 0
    int32_t k = 0;
#endif /* #if TOUCH_KEY_NUM > 0 */
	int32_t iArrayIndex = 0;
	bool isPass = true;

	for (j = 0; j < y_ch; j++) {
		for (i = 0; i < x_ch; i++) {
			iArrayIndex = j * x_ch + i;

			RecordResult[iArrayIndex] = 0x00; // default value for PASS

			if(rawdata[iArrayIndex] > Rawdata_Limit_Postive[iArrayIndex])
				RecordResult[iArrayIndex] |= 0x01;

			if(rawdata[iArrayIndex] < Rawdata_Limit_Negative[iArrayIndex])
				RecordResult[iArrayIndex] |= 0x02;
		}
	}
#if TOUCH_KEY_NUM > 0
	for (k = 0; k < Key_Channel; k++) {
		iArrayIndex = y_ch * x_ch + k;

		RecordResult[iArrayIndex] = 0x00; // default value for PASS

		if(rawdata[iArrayIndex] > Rawdata_Limit_Postive[iArrayIndex])
			RecordResult[iArrayIndex] |= 0x01;

		if(rawdata[iArrayIndex] < Rawdata_Limit_Negative[iArrayIndex])
			RecordResult[iArrayIndex] |= 0x02;
	}
#endif /* #if TOUCH_KEY_NUM > 0 */

	//---Check RecordResult---
	for (j = 0; j < y_ch; j++) {
		for (i = 0; i < x_ch; i++) {
			if (RecordResult[j * x_ch + i] != 0) {
				isPass = false;
				break;
			}
		}
	}
#if TOUCH_KEY_NUM > 0
	for (k = 0; k < Key_Channel; k++) {
		iArrayIndex = y_ch * x_ch + k;
		if (RecordResult[iArrayIndex] != 0) {
			isPass = false;
			break;
		}
	}
#endif /* #if TOUCH_KEY_NUM > 0 */

	/* Save Rawdata to CSV file */
	if (nvt_save_rawdata_to_csv(
		rawdata, RecordResult, isPass,
		X_Channel, Y_Channel, file_path, offset) < 0) {
		NVT_ERR("save rawdata to CSV file failed\n");
		return -EAGAIN;
	}

	if (isPass == false) {
		return -1; // FAIL
	} else {
		return 0; // PASS
	}
}

/*******************************************************
Description:
	Novatek touchscreen print self-test result function.

return:
	n.a.
*******************************************************/
void print_selftest_result(struct seq_file *m, int32_t TestResult, uint8_t RecordResult[], int32_t rawdata[], uint8_t x_len, uint8_t y_len)
{
	int32_t i = 0;
	int32_t j = 0;
	int32_t iArrayIndex = 0;
#if TOUCH_KEY_NUM > 0
	int32_t k = 0;
#endif /* #if TOUCH_KEY_NUM > 0 */

	switch (TestResult) {
		case 0:
			nvt_mp_seq_printf(m, " PASS!\n");
			break;

		case 1:
			nvt_mp_seq_printf(m, " ERROR! Read Data FAIL!\n");
			break;

		case -1:
			nvt_mp_seq_printf(m, " FAIL!\n");
			nvt_mp_seq_printf(m, "RecordResult:\n");
			for (i = 0; i < y_len; i++) {
				for (j = 0; j < x_len; j++) {
					iArrayIndex = i * x_len + j;
					nvt_mp_seq_printf(m, "0x%02X, ", RecordResult[iArrayIndex]);
				}
				nvt_mp_seq_printf(m, "\n");
			}
#if TOUCH_KEY_NUM > 0
			for (k = 0; k < Key_Channel; k++) {
				iArrayIndex = y_len * x_len + k;
				nvt_mp_seq_printf(m, "0x%02X, ", RecordResult[iArrayIndex]);
			}
			nvt_mp_seq_printf(m, "\n");
#endif /* #if TOUCH_KEY_NUM > 0 */
			nvt_mp_seq_printf(m, "ReadData:\n");
			for (i = 0; i < y_len; i++) {
				for (j = 0; j < x_len; j++) {
					iArrayIndex = i * x_len + j;
					nvt_mp_seq_printf(m, "%5d, ", rawdata[iArrayIndex]);
				}
				nvt_mp_seq_printf(m, "\n");
			}
#if TOUCH_KEY_NUM > 0
			for (k = 0; k < Key_Channel; k++) {
				iArrayIndex = y_len * x_len + k;
				nvt_mp_seq_printf(m, "%5d, ", rawdata[iArrayIndex]);
			}
			nvt_mp_seq_printf(m, "\n");
#endif /* #if TOUCH_KEY_NUM > 0 */
			break;
	}
	nvt_mp_seq_printf(m, "\n");
}

/*******************************************************
Description:
	Novatek touchscreen self-test sequence print show
	function.

return:
	Executive outcomes. 0---succeed.
*******************************************************/
static int32_t c_show_selftest(struct seq_file *m, void *v)
{
	TP_LOGD("+++");

	nvt_mp_seq_printf(m, "FW Version: 0x%02x\n", ts->fw_ver);
	nvt_mp_seq_printf(m, "\n");

	if (TestResult_Short == 0 && TestResult_Open == 0 &&
		TestResult_FWMutual == 0 && TestResult_FW_CC == 0 &&
		TestResult_Noise == 0) {
		nvt_mp_seq_printf(m, "Test Result PASS ");
	} else {
		nvt_mp_seq_printf(m, "Test Result FAIL ");
	}

	if (isPreTest)
		nvt_mp_seq_printf(m, "(T2 pre-test)\n");
	else
		nvt_mp_seq_printf(m, "(T2 test)\n");

	nvt_mp_seq_printf(m, "\n");

	nvt_mp_seq_printf(m, "Short Test");
	if ((TestResult_Short == 0) || (TestResult_Short == 1)) {
		print_selftest_result(m, TestResult_Short, RecordResult_Short, RawData_Short, X_Channel, Y_Channel);
	} else { // TestResult_Short is -1
		if (ts->carrier_system) {
			nvt_mp_seq_printf(m, " FAIL!\n");
			if (TestResult_Short_Diff == -1) {
				nvt_mp_seq_printf(m, "Short Diff");
				print_selftest_result(m, TestResult_Short_Diff, RecordResult_Short_Diff, RawData_Short_Diff, X_Channel, Y_Channel);
			}
			if (TestResult_Short_Base == -1) {
				nvt_mp_seq_printf(m, "Short Base");
				print_selftest_result(m, TestResult_Short_Base, RecordResult_Short_Base, RawData_Short_Base, X_Channel, Y_Channel);
			}
		} else {
			print_selftest_result(m, TestResult_Short, RecordResult_Short, RawData_Short, X_Channel, Y_Channel);
		}
	}

	nvt_mp_seq_printf(m, "Open Test");
	print_selftest_result(m, TestResult_Open, RecordResult_Open, RawData_Open, X_Channel, Y_Channel);

	if (!isPreTest) {
		nvt_mp_seq_printf(m, "FW Rawdata Test");
		print_selftest_result(m, TestResult_FWMutual,
			RecordResult_FWMutual, RawData_FWMutual,
			X_Channel, Y_Channel);

		if (ts->carrier_system) {
			nvt_mp_seq_printf(m, "FW CC_I");
			print_selftest_result(m, TestResult_FW_CC_I,
				RecordResult_FW_CC_I, RawData_FW_CC_I,
				X_Channel, Y_Channel);

			nvt_mp_seq_printf(m, "FW CC_Q");
			print_selftest_result(m, TestResult_FW_CC_Q,
				RecordResult_FW_CC_Q, RawData_FW_CC_Q,
				X_Channel, Y_Channel);
		} else {
			nvt_mp_seq_printf(m, "FW CC Test");
			print_selftest_result(m, TestResult_FW_CC,
				RecordResult_FW_CC, RawData_FW_CC,
				X_Channel, Y_Channel);
		}

		nvt_mp_seq_printf(m, "Noise Test");
		if ((TestResult_Noise == 0) || (TestResult_Noise == 1)) {
			print_selftest_result(m, TestResult_FW_DiffMax,
				RecordResult_FW_DiffMax, RawData_Diff_Max,
				X_Channel, Y_Channel);
		} else { /* TestResult_Noise is -1 */
			nvt_mp_seq_printf(m, " FAIL!\n");

			if (TestResult_FW_DiffMax == -1) {
				nvt_mp_seq_printf(m, "FW Diff Max");
				print_selftest_result(m, TestResult_FW_DiffMax,
				RecordResult_FW_DiffMax, RawData_Diff_Max,
				X_Channel, Y_Channel);
			}
			if (TestResult_FW_DiffMin == -1) {
				nvt_mp_seq_printf(m, "FW Diff Min");
				print_selftest_result(m, TestResult_FW_DiffMin,
					RecordResult_FW_DiffMin,
					RawData_Diff_Min, X_Channel, Y_Channel);
			}
		}
	}

	nvt_mp_test_result_printed = 1;

	TP_LOGD("---");

    return 0;
}

/*******************************************************
Description:
	Novatek touchscreen self-test sequence print start
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
	Novatek touchscreen self-test sequence print next
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
	Novatek touchscreen self-test sequence print stop
	function.

return:
	n.a.
*******************************************************/
static void c_stop(struct seq_file *m, void *v)
{
	return;
}

const struct seq_operations nvt_selftest_seq_ops = {
	.start  = c_start,
	.next   = c_next,
	.stop   = c_stop,
	.show   = c_show_selftest
};

/*---For SMx3 self-test start---*/
/*******************************************************
 *Description:
 *	SMx3 touchscreen mp selftest init. Due to SMx3 has
 *	two step self-test : T2 pre-test and T2 test, we move
 *	self test Initialization step into this function.
 *
 *return:
 *	Executive outcomes. 0---succeed. negative---failed.
 *******************************************************/
static int32_t smx3_selftest_init(struct device_node *np, bool _isPreTest)
{
	/* novatek-mp-criteria-default */
	unsigned char mpcriteria[32] = {0};
	int32_t ret = 0;

	TestResult_Short = 0;
	TestResult_Short_Diff = 0;
	TestResult_Short_Base = 0;
	TestResult_Open = 0;
	TestResult_FW_Rawdata = 0;
	TestResult_FWMutual = 0;
	TestResult_FW_CC = 0;
	TestResult_FW_CC_I = 0;
	TestResult_FW_CC_Q = 0;
	TestResult_Noise = 0;
	TestResult_FW_DiffMax = 0;
	TestResult_FW_DiffMin = 0;

	isPreTest = _isPreTest;

	ret = nvt_mp_buffer_init();
	if (ret < 0) {
		TP_LOGE("Allocate mp memory failed");
		return ret;
	}

	if (mutex_lock_interruptible(&ts->lock)) {
		return -ERESTARTSYS;
	}

#if NVT_TOUCH_ESD_PROTECT
	nvt_esd_check_enable(false);
#endif /* #if NVT_TOUCH_ESD_PROTECT */

	if (nvt_get_fw_info()) {
		mutex_unlock(&ts->lock);
		TP_LOGE("get fw info failed!");
		return -EAGAIN;
	}

	/* Parsing criteria from dts */
	if (!isGetCriteria) {
		if (of_property_read_bool(np, "novatek,mp-support-dt")) {
			/*
			 * Parsing Criteria by Novatek PID
			 * The string rule is "novatek-mp-criteria-<nvt_pid>"
			 * nvt_pid is 2 bytes (show hex).
			 *
			 * Ex. nvt_pid = 500A
			 *     mpcriteria = "novatek-mp-criteria-500A"
			 */
			if (ts->nvt_pid == 0x0108 || ts->nvt_pid == 0x1114)
				snprintf(mpcriteria, PAGE_SIZE,
					"novatek-mp-criteria-%04X",
					ts->nvt_pid);
			else
				snprintf(mpcriteria, PAGE_SIZE,
					"novatek-mp-criteria-default");

			if (!nvt_mp_parse_dt(np, mpcriteria)) {
				isGetCriteria = true;
			} else {
				mutex_unlock(&ts->lock);
				TP_LOGE("mp parse device tree failed!");
				return -EINVAL;
			}
		} else {
			TP_LOGI("Not found keyword, use default setting");
			/* ---Print Test Criteria--- */
#if PRINT_DT_CRITERIA
			nvt_print_criteria();
#endif /* #if PRINT_DT_CRITERIA */
		}
	}

	if (nvt_switch_FreqHopEnDis(FREQ_HOP_DISABLE)) {
		mutex_unlock(&ts->lock);
		TP_LOGE("switch frequency hopping disable failed!");
		return -EAGAIN;
	}

	if (nvt_check_fw_reset_state(RESET_STATE_NORMAL_RUN)) {
		mutex_unlock(&ts->lock);
		TP_LOGE("check fw reset state failed!");
		return -EAGAIN;
	}

	msleep(100);

	//---Enter Test Mode---
	if (nvt_clear_fw_status()) {
		mutex_unlock(&ts->lock);
		TP_LOGE("clear fw status failed!");
		return -EAGAIN;
	}

	if (!isPreTest) {
		nvt_change_mode(MP_MODE_CC);

		if (nvt_check_fw_status()) {
			mutex_unlock(&ts->lock);
			TP_LOGE("check fw status failed!");
			return -EAGAIN;
		}
	}

	return 0;
}

/*******************************************************
 *Description:
 *	SMx3 touchscreen /proc/nvt_preselftest open function.
 *
 *return:
 *	Executive outcomes. 0---succeed. negative---failed.
 *******************************************************/
static int32_t nvt_preselftest_open(struct inode *inode, struct file *file)
{
	struct device_node *np = ts->client->dev.of_node;
	int32_t ret = 0;

	TP_LOGD("+++");

	ret = smx3_selftest_init(np, true);

	if (ret != 0)
		return ret;

	/* ---Short Test--- */
	if (nvt_read_fw_short(RawData_Short) != 0) {
		TestResult_Short = 1; /* 1:ERROR */
		if (ts->carrier_system) {
			TestResult_Short_Diff = 1;
			TestResult_Short_Base = 1;
		}
	} else {
		/*---Self Test Check --- 0:PASS, -1:FAIL */
		if (ts->carrier_system) {
			TestResult_Short_Diff =
				RawDataTest_SinglePoint_Sub(RawData_Short_Diff,
					RecordResult_Short_Diff,
					X_Channel, Y_Channel,
					PS_Config_Lmt_Short_Diff_P,
					PS_Config_Lmt_Short_Diff_N,
					SHORT_PRETEST_CSV_FILE, 0);
			TestResult_Short_Base =
				RawDataTest_SinglePoint_Sub(RawData_Short_Base,
					RecordResult_Short_Base,
					X_Channel, Y_Channel,
					PS_Config_Lmt_Short_Base_P,
					PS_Config_Lmt_Short_Base_N,
					SHORT_PRETEST_CSV_FILE, file_offset);

			if ((TestResult_Short_Diff == -1) ||
				(TestResult_Short_Base == -1))
				TestResult_Short = -1;
			else
				TestResult_Short = 0;
		} else {
			TestResult_Short =
				RawDataTest_SinglePoint_Sub(RawData_Short,
					RecordResult_Short,
					X_Channel, Y_Channel,
					PS_Config_Lmt_Short_Rawdata_P,
					PS_Config_Lmt_Short_Rawdata_N,
					SHORT_PRETEST_CSV_FILE, 0);
		}
	}

	/* ---Open Test--- */
	if (nvt_read_fw_open(RawData_Open) != 0) {
		TestResult_Open = 1;    /* 1:ERROR */
	} else {
		/*---Self Test Check --- 0:PASS, -1:FAIL */
		TestResult_Open =
			RawDataTest_SinglePoint_Sub(RawData_Open,
				RecordResult_Open,
				X_Channel, Y_Channel,
				PS_Config_Lmt_Open_Rawdata_P,
				PS_Config_Lmt_Open_Rawdata_N,
				OPEN_PRETEST_CSV_FILE, 0);
	}

	/*---Reset IC--- */
	nvt_bootloader_reset();

	mutex_unlock(&ts->lock);

	TP_LOGD("---");

	nvt_mp_test_result_printed = 0;

	return seq_open(file, &nvt_selftest_seq_ops);
}
/*---For SMx3 self-test end---*/

/*******************************************************
 *Description:
 *	Novatek touchscreen /proc/nvt_selftest open function.
 *
 *return:
 *	Executive outcomes. 0---succeed. negative---failed.
 *
 *To inclease the reuse rate for the codes, SMx3 move
 *initialization step to smx3_selftest_init.
 *******************************************************/
static int32_t nvt_selftest_open(struct inode *inode, struct file *file)
{
	struct device_node *np = ts->client->dev.of_node;
	int32_t ret = 0;

	TP_LOGD("+++");

	ret = smx3_selftest_init(np, false);

	if (ret != 0)
		return ret;

	//---FW Rawdata Test---
	if (nvt_read_baseline(RawData_FWMutual) != 0) {
		TestResult_FWMutual = 1;
	} else {
		TestResult_FWMutual =
			RawDataTest_SinglePoint_Sub(RawData_FWMutual,
				RecordResult_FWMutual,
				X_Channel, Y_Channel,
				PS_Config_Lmt_FW_Rawdata_P,
				PS_Config_Lmt_FW_Rawdata_N,
				FW_RAWDATA_CSV_FILE, 0);
	}
	if (nvt_read_CC(RawData_FW_CC) != 0) {
		TestResult_FW_CC = 1;
		if (ts->carrier_system) {
			TestResult_FW_CC_I = 1;
			TestResult_FW_CC_Q = 1;
		}
	} else {
		if (ts->carrier_system) {
			TestResult_FW_CC_I =
				RawDataTest_SinglePoint_Sub(RawData_FW_CC_I,
					RecordResult_FW_CC_I,
					X_Channel, Y_Channel,
					PS_Config_Lmt_FW_CC_I_P,
					PS_Config_Lmt_FW_CC_I_N,
					FW_CC_CSV_FILE, 0);
			TestResult_FW_CC_Q =
				RawDataTest_SinglePoint_Sub(RawData_FW_CC_Q,
					RecordResult_FW_CC_Q,
					X_Channel, Y_Channel,
					PS_Config_Lmt_FW_CC_Q_P,
					PS_Config_Lmt_FW_CC_Q_N,
					FW_CC_CSV_FILE, file_offset);
			if ((TestResult_FW_CC_I == -1) ||
				(TestResult_FW_CC_Q == -1))
				TestResult_FW_CC = -1;
			else
				TestResult_FW_CC = 0;
		} else {
			TestResult_FW_CC =
				RawDataTest_SinglePoint_Sub(RawData_FW_CC,
					RecordResult_FW_CC,
					X_Channel, Y_Channel,
					PS_Config_Lmt_FW_CC_P,
					PS_Config_Lmt_FW_CC_N,
					FW_CC_CSV_FILE, 0);
		}
	}

	if ((TestResult_FWMutual == 1) || (TestResult_FW_CC == 1)) {
		TestResult_FW_Rawdata = 1;
	} else {
		if ((TestResult_FWMutual == -1) || (TestResult_FW_CC == -1))
			TestResult_FW_Rawdata = -1;
		else
			TestResult_FW_Rawdata = 0;
	}

	//---Leave Test Mode---
	nvt_change_mode(NORMAL_MODE);

	//---Noise Test---
	if (nvt_read_fw_noise(RawData_Diff) != 0) {
		TestResult_Noise = 1;	// 1: ERROR
		TestResult_FW_DiffMax = 1;
		TestResult_FW_DiffMin = 1;
	} else {
		TestResult_FW_DiffMax =
			RawDataTest_SinglePoint_Sub(RawData_Diff_Max,
				RecordResult_FW_DiffMax,
				X_Channel, Y_Channel,
				PS_Config_Lmt_FW_Diff_P,
				PS_Config_Lmt_FW_Diff_N,
				NOISE_TEST_CSV_FILE, 0);

		// for carrier sensing system, only positive noise data
		if (ts->carrier_system) {
			TestResult_FW_DiffMin = 0;
		} else {
			TestResult_FW_DiffMin =
				RawDataTest_SinglePoint_Sub(RawData_Diff_Min,
					RecordResult_FW_DiffMin,
					X_Channel, Y_Channel,
					PS_Config_Lmt_FW_Diff_P,
					PS_Config_Lmt_FW_Diff_N,
					NOISE_TEST_CSV_FILE, file_offset);
		}

		if ((TestResult_FW_DiffMax == -1) || (TestResult_FW_DiffMin == -1))
			TestResult_Noise = -1;
		else
			TestResult_Noise = 0;
	}

	//--Short Test---
	if (nvt_read_fw_short(RawData_Short) != 0) {
		TestResult_Short = 1; // 1:ERROR
		if (ts->carrier_system) {
			TestResult_Short_Diff = 1;
			TestResult_Short_Base = 1;
		}
	} else {
		//---Self Test Check --- // 0:PASS, -1:FAIL
		if (ts->carrier_system) {
			TestResult_Short_Diff =
				RawDataTest_SinglePoint_Sub(RawData_Short_Diff,
					RecordResult_Short_Diff,
					X_Channel, Y_Channel,
					PS_Config_Lmt_Short_Diff_P,
					PS_Config_Lmt_Short_Diff_N,
					SHORT_TEST_CSV_FILE, 0);
			TestResult_Short_Base =
				RawDataTest_SinglePoint_Sub(RawData_Short_Base,
					RecordResult_Short_Base,
					X_Channel, Y_Channel,
					PS_Config_Lmt_Short_Base_P,
					PS_Config_Lmt_Short_Base_N,
					SHORT_TEST_CSV_FILE, file_offset);

			if ((TestResult_Short_Diff == -1) ||
				(TestResult_Short_Base == -1))
				TestResult_Short = -1;
			else
				TestResult_Short = 0;
		} else {
			TestResult_Short =
				RawDataTest_SinglePoint_Sub(RawData_Short,
					RecordResult_Short,
					X_Channel, Y_Channel,
					PS_Config_Lmt_Short_Rawdata_P,
					PS_Config_Lmt_Short_Rawdata_N,
					SHORT_TEST_CSV_FILE, 0);
		}
	}

	//---Open Test---
	if (nvt_read_fw_open(RawData_Open) != 0) {
		TestResult_Open = 1;    // 1:ERROR
	} else {
		//---Self Test Check --- // 0:PASS, -1:FAIL
		TestResult_Open =
			RawDataTest_SinglePoint_Sub(RawData_Open,
				RecordResult_Open,
				X_Channel, Y_Channel,
				PS_Config_Lmt_Open_Rawdata_P,
				PS_Config_Lmt_Open_Rawdata_N,
				OPEN_TEST_CSV_FILE, 0);
	}

	//---Reset IC---
	nvt_bootloader_reset();

	mutex_unlock(&ts->lock);

	TP_LOGD("---");

	nvt_mp_test_result_printed = 0;

	return seq_open(file, &nvt_selftest_seq_ops);
}

/*---For SMx3 self-test start---*/
/*******************************************************
 *Description:
 *	SMx3 touchscreen /proc/nvt_selftest and
 *	/proc/nvt_preselftest release function.
 *
 *return:
 *	Executive outcomes. 0---succeed. negative---failed.
 *
 *To reduce memory use, SMx3 add this function to
 *release memory for non-use array
 *******************************************************/
int nvt_selftest_release(struct inode *inode, struct file *file)
{
	smx3_mp_buffer_free();
	return seq_release(inode, file);
}

static const struct file_operations nvt_preselftest_fops = {
	.owner = THIS_MODULE,
	.open = nvt_preselftest_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = nvt_selftest_release,
};

static const struct file_operations nvt_selftest_fops = {
	.owner = THIS_MODULE,
	.open = nvt_selftest_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = nvt_selftest_release,
};
/*---For SMx3 self-test end---*/

#ifdef CONFIG_OF
/*******************************************************
Description:
	Novatek touchscreen parse AIN setting for array type.

return:
	n.a.
*******************************************************/
int32_t nvt_mp_parse_ain(struct device_node *np, const char *name, uint8_t *array, int32_t size)
{
	struct property *data;
	int32_t len, ret;
	int32_t tmp[40];
	int32_t i;

	data = of_find_property(np, name, &len);
	len /= sizeof(u32);
	if ((!data) || (!len) || (len != size)) {
		TP_LOGE("error find %s. len=%d", name, len);
		return -1;
	} else {
		TP_LOGI("%s. len=%d", name, len);
		ret = of_property_read_u32_array(np, name, tmp, len);
		if (ret) {
			TP_LOGE("error reading %s. ret=%d", name, ret);
			return -1;
		}

		for (i = 0; i < len; i++)
			array[i] = tmp[i];

#if PRINT_DT_CRITERIA
		printk("[NVT-ts] %s = ", name);
		for (i = 0; i < len; i++) {
			printk("%02d ", array[i]);
		}
		printk("\n");
#endif /* #if PRINT_DT_CRITERIA */
	}

	return 0;
}

/*******************************************************
Description:
	Novatek touchscreen parse criterion for u32 type.

return:
	n.a.
*******************************************************/
int32_t nvt_mp_parse_u32(struct device_node *np, const char *name, int32_t *para)
{
	int32_t ret;

	ret = of_property_read_u32(np, name, para);
	if (ret) {
		TP_LOGE("error reading %s. ret=%d", name, ret);
		return -1;
	} else {
		TP_LOGI("%s=%d", name, *para);
	}

	return 0;
}

/*******************************************************
Description:
	Novatek touchscreen parse criterion for array type.

return:
	n.a.
*******************************************************/
int32_t nvt_mp_parse_array(struct device_node *np, const char *name, int32_t *array,
		int32_t size)
{
	struct property *data;
	int32_t len, ret;
#if PRINT_DT_CRITERIA
	int32_t i, j, iArrayIndex = 0;
#endif /* #if PRINT_DT_CRITERIA */

	data = of_find_property(np, name, &len);
	len /= sizeof(u32);
	if ((!data) || (!len) || (len < size)) {
		TP_LOGE("error find %s. len=%d", name, len);
		return -1;
	} else {
		TP_LOGI("%s. len=%d", name, len);
		ret = of_property_read_u32_array(np, name, array, len);
		if (ret) {
			TP_LOGE("error reading %s. ret=%d", name, ret);
			return -1;
		}

#if PRINT_DT_CRITERIA
		TP_LOGD("%s =", name);
		for (j = 0; j < Y_Channel; j++) {
			printk("[NVT-ts] ");
			for (i = 0; i < X_Channel; i++) {
				iArrayIndex = j * X_Channel + i;
				printk("%5d, ", array[iArrayIndex]);
			}
			printk("\n");
		}
#if TOUCH_KEY_NUM > 0
		printk("[NVT-ts] ");
		for (i = 0; i < Key_Channel; i++) {
			iArrayIndex++;
			printk("%5d, ", array[iArrayIndex]);
		}
		printk("\n");
#endif /* #if TOUCH_KEY_NUM > 0 */
#endif /* #if PRINT_DT_CRITERIA */
	}

	return 0;
}

/*******************************************************
Description:
	Novatek touchscreen parse device tree mp function.

return:
	n.a.
*******************************************************/
int32_t nvt_mp_parse_dt(struct device_node *root, const char *node_compatible)
{
	struct device_node *np = root;
	struct device_node *child = NULL;

	TP_LOGI("Parse mp criteria for node %s", node_compatible);

	/* find each MP sub-nodes */
	for_each_child_of_node(root, child) {
		/* find the specified node */
		if (of_device_is_compatible(child, node_compatible)) {
			TP_LOGI("found child node %s", node_compatible);
			np = child;
			break;
		}
	}
	if (child == NULL) {
		TP_LOGE("Not found compatible node %s!", node_compatible);
		return -1;
	}

	/* MP Config*/
	if (nvt_mp_parse_u32(np, "IC_X_CFG_SIZE", &IC_X_CFG_SIZE))
		return -1;

	if (nvt_mp_parse_u32(np, "IC_Y_CFG_SIZE", &IC_Y_CFG_SIZE))
		return -1;

#if TOUCH_KEY_NUM > 0
	if (nvt_mp_parse_u32(np, "IC_KEY_CFG_SIZE", &IC_KEY_CFG_SIZE))
		return -1;
#endif

	if (nvt_mp_parse_u32(np, "X_Channel", &X_Channel))
		return -1;

	if (nvt_mp_parse_u32(np, "Y_Channel", &Y_Channel))
		return -1;

	if (nvt_mp_parse_ain(np, "AIN_X", AIN_X, IC_X_CFG_SIZE))
		return -1;

	if (nvt_mp_parse_ain(np, "AIN_Y", AIN_Y, IC_Y_CFG_SIZE))
		return -1;

#if TOUCH_KEY_NUM > 0
	if (nvt_mp_parse_ain(np, "AIN_KEY", AIN_KEY, IC_KEY_CFG_SIZE))
		return -1;
#endif

	/* MP Criteria */
	if (ts->carrier_system) {
		if (nvt_mp_parse_array(np, "PS_Config_Lmt_Short_Diff_P", PS_Config_Lmt_Short_Diff_P,
				X_Channel * Y_Channel + Key_Channel))
			return -1;

		if (nvt_mp_parse_array(np, "PS_Config_Lmt_Short_Diff_N", PS_Config_Lmt_Short_Diff_N,
				X_Channel * Y_Channel + Key_Channel))
			return -1;

		if (nvt_mp_parse_array(np, "PS_Config_Lmt_Short_Base_P", PS_Config_Lmt_Short_Base_P,
				X_Channel * Y_Channel + Key_Channel))
			return -1;

		if (nvt_mp_parse_array(np, "PS_Config_Lmt_Short_Base_N", PS_Config_Lmt_Short_Base_N,
				X_Channel * Y_Channel + Key_Channel))
			return -1;
	} else {
		if (nvt_mp_parse_array(np, "PS_Config_Lmt_Short_Rawdata_P", PS_Config_Lmt_Short_Rawdata_P,
				X_Channel * Y_Channel + Key_Channel))
			return -1;

		if (nvt_mp_parse_array(np, "PS_Config_Lmt_Short_Rawdata_N", PS_Config_Lmt_Short_Rawdata_N,
				X_Channel * Y_Channel + Key_Channel))
			return -1;
	}

	if (nvt_mp_parse_array(np, "PS_Config_Lmt_Open_Rawdata_P", PS_Config_Lmt_Open_Rawdata_P,
			X_Channel * Y_Channel + Key_Channel))
		return -1;

	if (nvt_mp_parse_array(np, "PS_Config_Lmt_Open_Rawdata_N", PS_Config_Lmt_Open_Rawdata_N,
			X_Channel * Y_Channel + Key_Channel))
		return -1;

	if (nvt_mp_parse_array(np, "PS_Config_Lmt_FW_Rawdata_P", PS_Config_Lmt_FW_Rawdata_P,
			X_Channel * Y_Channel + Key_Channel))
		return -1;

	if (nvt_mp_parse_array(np, "PS_Config_Lmt_FW_Rawdata_N", PS_Config_Lmt_FW_Rawdata_N,
			X_Channel * Y_Channel + Key_Channel))
		return -1;

	if (ts->carrier_system) {
		if (nvt_mp_parse_array(np, "PS_Config_Lmt_FW_CC_I_P", PS_Config_Lmt_FW_CC_I_P,
				X_Channel * Y_Channel + Key_Channel))
			return -1;

		if (nvt_mp_parse_array(np, "PS_Config_Lmt_FW_CC_I_N", PS_Config_Lmt_FW_CC_I_N,
				X_Channel * Y_Channel + Key_Channel))
			return -1;

		if (nvt_mp_parse_array(np, "PS_Config_Lmt_FW_CC_Q_P", PS_Config_Lmt_FW_CC_Q_P,
				X_Channel * Y_Channel + Key_Channel))
			return -1;

		if (nvt_mp_parse_array(np, "PS_Config_Lmt_FW_CC_Q_N", PS_Config_Lmt_FW_CC_Q_N,
				X_Channel * Y_Channel + Key_Channel))
			return -1;
	} else {
		if (nvt_mp_parse_array(np, "PS_Config_Lmt_FW_CC_P", PS_Config_Lmt_FW_CC_P,
				X_Channel * Y_Channel + Key_Channel))
			return -1;

		if (nvt_mp_parse_array(np, "PS_Config_Lmt_FW_CC_N", PS_Config_Lmt_FW_CC_N,
				X_Channel * Y_Channel + Key_Channel))
			return -1;
	}

	if (nvt_mp_parse_array(np, "PS_Config_Lmt_FW_Diff_P", PS_Config_Lmt_FW_Diff_P,
			X_Channel * Y_Channel + Key_Channel))
		return -1;

	if (nvt_mp_parse_array(np, "PS_Config_Lmt_FW_Diff_N", PS_Config_Lmt_FW_Diff_N,
			X_Channel * Y_Channel + Key_Channel))
		return -1;

	if (nvt_mp_parse_u32(np, "PS_Config_Diff_Test_Frame", &PS_Config_Diff_Test_Frame))
		return -1;

	TP_LOGI("Parse mp criteria done!");

	return 0;
}
#endif /* #ifdef CONFIG_OF */

/*******************************************************
 *Description:
 *	Novatek touchscreen MP function proc. file node
 *	initial function.
 *
 *return:
 *	Executive outcomes. 0---succeed. -1---failed.
 *
 *To reduce memory use, SMx3 move allocating memory step
 *to smx3_selftest_init function
 *******************************************************/
int32_t nvt_mp_proc_init(void)
{
	int32_t ret = 0;

	isPreTest = false;
	isGetCriteria = false;
	NVT_proc_preselftest_entry = NULL;

	NVT_proc_preselftest_entry =
		proc_create("nvt_preselftest", 0444, NULL,
			&nvt_preselftest_fops);
	if (NVT_proc_preselftest_entry == NULL) {
		TP_LOGE("create /proc/nvt_preselftest Failed!");
		ret = -1;
	} else {
		TP_LOGI("create /proc/nvt_preselftest Succeeded!");
	}

	NVT_proc_selftest_entry =
		proc_create("nvt_selftest", 0444, NULL, &nvt_selftest_fops);
	if (NVT_proc_selftest_entry == NULL) {
		TP_LOGE("create /proc/nvt_selftest Failed!");
		ret = -1;
	} else {
		TP_LOGI("create /proc/nvt_selftest Succeeded!");
	}

	return ret;
}

#endif /* #if NVT_TOUCH_MP */
