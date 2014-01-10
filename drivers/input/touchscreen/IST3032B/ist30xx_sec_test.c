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
#include <linux/module.h>

#include "ist30xx.h"
#include "ist30xx_update.h"
#include "ist30xx_misc.h"
#include "ist30xx_sec_tsp.h"

extern struct ist30xx_data *ts_data;
extern TSP_INFO ist30xx_tsp_info;
extern TKEY_INFO ist30xx_tkey_info;


#define TSP_BUF_SIZE    (1024)
/* /sys/class/sec/tsp/close_tsp_test */
ssize_t sec_tsp_test_close_show(struct device *dev,
struct device_attribute *attr, char *buf)
{
DMSG("[TSP] tsp test close\n");

return snprintf(buf, TSP_BUF_SIZE, "%u\n", 0);
}

/* /sys/class/sec/tsp/cmd_status */
ssize_t sec_tsp_cmd_status_show(struct device *dev,
struct device_attribute *attr, char *buf)
{
DMSG("[TSP] tsp cmd status : Ready");

return snprintf(buf, sizeof(char)*16, "Ready\n");
}

/* /sys/class/sec/tsp/cmd_result */
ssize_t sec_tsp_cmd_result_show(struct device *dev,
struct device_attribute *attr, char *buf)
{
DMSG("[TSP] tsp cmd result : Ready");

return snprintf(buf, sizeof(char)*16, "Ready\n");
}

/* /sys/class/sec/tsp/cmd */
ssize_t sec_tsp_cmd_store(struct device *dev,
struct device_attribute *attr, const char *buf, size_t size)
{
DMSG("[TSP] tsp cmd store\n");

return size;
}

/* /sys/class/sec/tsp/intensity_logging_on */
ssize_t sec_tsp_intensity_log_on(struct device *dev,
struct device_attribute *attr, char *buf)
{
DMSG("[TSP] tsp intensity logging on : Ready");

return snprintf(buf, sizeof(char)*16, "Ready\n");
}

/* /sys/class/sec/tsp/intensity_logging_off */
ssize_t sec_tsp_intensity_log_off(struct device *dev,
struct device_attribute *attr, char *buf)
{
DMSG("[TSP] tsp intensity logging off : Ready");

return snprintf(buf, sizeof(char)*16, "Ready\n");
}


extern ssize_t ist30xx_frame_refresh(struct device *dev,
struct device_attribute *attr, char *buf);
/* /sys/class/sec/tsp/read_frame */
ssize_t sec_tsp_read_frame_show(struct device *dev,
struct device_attribute *attr, char *buf)
{
int ret;

ret = ist30xx_enter_debug_mode();
	if (ret)
		return snprintf(buf, sizeof(char)*16, "fail, sec_read_frame\n");

ist30xx_frame_refresh(dev, attr, buf);

return snprintf(buf, sizeof(char)*16, "sec_read_frame\n");
}

static u16 *node_raw_buf;
static u16 *node_baseline_buf;
/* sysfs: /sys/class/sec/tsp/read_node */
ssize_t sec_tsp_read_node_show(struct device *dev,
struct device_attribute *attr, char *buf)
{
int ret;

ret = ist30xx_read_tsp_node(node_raw_buf, node_baseline_buf);
	if (ret)
		return snprintf(buf, sizeof(char)*16, "fail, sec_read_frame\n");
return 0;
}

/* sysfs: /sys/class/sec/tsp/raw */
ssize_t sec_tsp_raw_show(struct device *dev,
struct device_attribute *attr, char *buf)
{
	int i, j;
	int count = 0;
	const int msg_len = 128;
	char msg[msg_len];
	TSP_INFO *tsp = &ist30xx_tsp_info;

	ist30xx_parse_tsp_node(node_raw_buf, node_baseline_buf);

	for (i = 0; i < tsp->height; i++) {
		for (j = 0; j < tsp->width; j++) {
			count += snprintf(msg, msg_len,
				"%04d ", *node_raw_buf++);
			strncat(buf, msg, msg_len);
		}

		count += snprintf(msg, msg_len, "\n");
		strncat(buf, msg, msg_len);
	}

	return count;
}
/* sysfs: /sys/class/sec/tsp/baseline */
ssize_t sec_tsp_baseline_show(struct device *dev,
struct device_attribute *attr, char *buf)
{
	int i, j;
	int count = 0;
	const int msg_len = 128;
	char msg[msg_len];
	TSP_INFO *tsp = &ist30xx_tsp_info;

	ist30xx_parse_tsp_node(node_raw_buf, node_baseline_buf);

	for (i = 0; i < tsp->height; i++) {
		for (j = 0; j < tsp->width; j++) {
			count += snprintf(msg, msg_len, "%04d ",
				*node_baseline_buf++);
			strncat(buf, msg, msg_len);
		}

		count += snprintf(msg, msg_len, "\n");
		strncat(buf, msg, msg_len);
	}

	return count;
}


/* /sys/class/sec/tsp/chip_vendor */
ssize_t sec_tsp_chip_vendor_show(struct device *dev,
struct device_attribute *attr, char *buf)
{
	int ret;

ret = ist30xx_cmd_run_device(ts_data->client);
	if (ret)
		return sprintf(buf, "NULL");

ret = ist30xx_read_cmd(ts_data->client, CMD_GET_CHIP_ID, &ts_data->chip_id);
	if (ret)
		return sprintf(buf, "NULL");

if ((ts_data->chip_id != IST30XX_CHIP_ID) &&
	    (ts_data->chip_id != IST30XXA_CHIP_ID))
		return sprintf(buf, "NULL");

DMSG("[TSP] tsp chip vendor: %s\n", IST30XX_CHIP_VENDOR);

return sprintf(buf, "%s\n", IST30XX_CHIP_VENDOR);
}

/* /sys/class/sec/tsp/chip_name */
ssize_t sec_tsp_chip_name_show(struct device *dev,
struct device_attribute *attr, char *buf)
{
	int ret;

ret = ist30xx_cmd_run_device(ts_data->client);
	if (ret)
		return sprintf(buf, "NULL");

ret = ist30xx_read_cmd(ts_data->client, CMD_GET_CHIP_ID, &ts_data->chip_id);
	if (ret)
		return sprintf(buf, "NULL");

if ((ts_data->chip_id != IST30XX_CHIP_ID) &&
	    (ts_data->chip_id != IST30XXA_CHIP_ID))
		return sprintf(buf, "NULL");

DMSG("[TSP] tsp chip name: %s\n", IST30XX_CHIP_NAME);

return sprintf(buf, "%s\n", IST30XX_CHIP_NAME);
}

/* /sys/class/sec/tsp/panel_vendor */
ssize_t sec_tsp_panel_vendor_show(struct device *dev,
struct device_attribute *attr, char *buf)
{
	int ret;
u32 panel_vendor;

ret = ist30xx_cmd_run_device(ts_data->client);
	if (ret)
		return sprintf(buf, "NULL");

ret = ist30xx_read_cmd(ts_data->client, CMD_GET_TSP_VENDOR, &panel_vendor);
	if (ret)
		return sprintf(buf, "NULL");

DMSG("[TSP] tsp panel vendor: %d\n", panel_vendor);

return sprintf(buf, "%d\n", panel_vendor);
}

/* /sys/class/sec/tsp/fw_ver */
ssize_t sec_tsp_fw_ver_show(struct device *dev,
struct device_attribute *attr, char *buf)
{
	int ret;

ret = ist30xx_cmd_run_device(ts_data->client);
	if (ret)
		return sprintf(buf, "NULL");

ret = ist30xx_read_cmd(ts_data->client, CMD_GET_FW_VER, &ts_data->fw.ver);
	if (ret)
		return sprintf(buf, "NULL");

ret = ist30xx_read_cmd(ts_data->client, CMD_GET_PARAM_VER, &ts_data->param_ver);
	if (ret)
		return sprintf(buf, "NULL");

DMSG("[TSP] tsp version: %d(%d.%d.%d)\n", ts_data->param_ver,
(ts_data->fw.ver>>16) & 0xFF, (ts_data->fw.ver>>8) & 0xFF,
ts_data->fw.ver & 0xFF);

return sprintf(buf, "%d(%d.%d.%d)\n", ts_data->param_ver,
(ts_data->fw.ver>>16) & 0xFF, (ts_data->fw.ver>>8) & 0xFF,
ts_data->fw.ver & 0xFF);
}

/* /sys/class/sec/tsp/raw_range */
ssize_t sec_tsp_raw_range_show(struct device *dev,
struct device_attribute *attr, char *buf)
{
DMSG("[TSP] tsp raw range: %d~%d\n",
IST30XX_MIN_RAW_DATA, IST30XX_MAX_RAW_DATA);

return sprintf(buf, "%d,%d\n", IST30XX_MIN_RAW_DATA, IST30XX_MAX_RAW_DATA);
}

/* /sys/class/sec/tsp/base_range */
ssize_t sec_tsp_base_range_show(struct device *dev,
struct device_attribute *attr, char *buf)
{
DMSG("[TSP] tsp base range: %d~%d\n",
IST30XX_MIN_BASELINE, IST30XX_MAX_BASELINE);

return sprintf(buf, "%d,%d\n", IST30XX_MIN_BASELINE, IST30XX_MAX_BASELINE);
}

/* /sys/class/sec/tsp/zval_range */
ssize_t sec_tsp_zval_range_show(struct device *dev,
struct device_attribute *attr, char *buf)
{
DMSG("[TSP] tsp raw range: %d~%d\n",
IST30XX_MIN_ZVALUE_TSP, IST30XX_MAX_ZVALUE_TSP);

return sprintf(buf, "%d,%d\n",
IST30XX_MIN_ZVALUE_TSP, IST30XX_MAX_ZVALUE_TSP);
}

/* /sys/class/sec/tsp/node_num */
ssize_t sec_tsp_node_num_show(struct device *dev,
struct device_attribute *attr, char *buf)
{
DMSG("[TSP] tsp node num: width:%d, height: %d\n",
ist30xx_tsp_info.width, ist30xx_tsp_info.height);

return sprintf(buf, "%d,%d\n",
ist30xx_tsp_info.width, ist30xx_tsp_info.height);
}


/* /sys/class/sec/tkey/tkey_back */
ssize_t sec_back_key_state_show(struct device *dev,
struct device_attribute *attr, char *buf)
{
int key_state=0;

DMSG("[TSP] back tkey state: %d\n", key_state);

return snprintf(buf, sizeof(buf), "%d\n", key_state);
}

/* /sys/class/sec/tkey/tkey_menu */
ssize_t sec_menu_key_state_show(struct device *dev,
struct device_attribute *attr, char *buf)
{
int key_state=0;

DMSG("[TSP] menu tkey state: %d\n", key_state);

return snprintf(buf, sizeof(int), "%d\n", key_state);
}

/* /sys/class/sec/tkey/tkey_hom */
ssize_t sec_home_key_state_show(struct device *dev,
struct device_attribute *attr, char *buf)
{
int key_state=0;

DMSG("[TSP] home tkey state: %d\n", key_state);

return snprintf(buf, sizeof(int), "%d\n", key_state);
}

/* /sys/class/sec/tkey/tkey_recent */
ssize_t sec_recent_key_state_show(struct device *dev,
struct device_attribute *attr, char *buf)
{
int key_state=0;

DMSG("[TSP] recent tkey state: %d\n", key_state);

return snprintf(buf, sizeof(int), "%d\n", key_state);
}

/* /sys/class/sec/tkey/tkey_threshold */
ssize_t sec_tkey_threshold_show(struct device *dev,
struct device_attribute *attr, char *buf)
{
int threshold=0;

DMSG("[TSP] touch tkey threshold: %d\n", threshold);

return snprintf(buf, sizeof(int), "%d\n", threshold);
}

/* /sys/class/sec/tkey/tkey_raw_data0 */
ssize_t sec_tkey_rawcnt0_show(struct device *dev,
struct device_attribute *attr, char *buf)
{
int raw_cnt=0;

DMSG("[TSP] raw counter0: %d\n", raw_cnt);

return snprintf(buf, sizeof(int), "%d\n", raw_cnt);
}

/* /sys/class/sec/tkey/tkey_raw_data1 */
ssize_t sec_tkey_rawcnt1_show(struct device *dev,
struct device_attribute *attr, char *buf)
{
int raw_cnt=0;

DMSG("[TSP] raw counter1: %d\n", raw_cnt);

return snprintf(buf, sizeof(int), "%d\n", raw_cnt);
}

/* /sys/class/sec/tkey/tkey_raw_data2 */
ssize_t sec_tkey_rawcnt2_show(struct device *dev,
struct device_attribute *attr, char *buf)
{
int raw_cnt=0;

DMSG("[TSP] raw counter2: %d\n", raw_cnt);

return snprintf(buf, sizeof(int), "%d\n", raw_cnt);
}

/* /sys/class/sec/tkey/tkey_raw_data3 */
ssize_t sec_tkey_rawcnt3_show(struct device *dev,
struct device_attribute *attr, char *buf)
{
int raw_cnt=0;

DMSG("[TSP] raw counter3 : %d\n", raw_cnt);

return snprintf(buf, sizeof(int), "%d\n", raw_cnt);
}


extern bool get_zvalue_mode;
extern int ist30xx_zvalue;
/* /sys/class/sec/tkey/tkey_zvalue */
ssize_t sec_tkey_zvalue_show(struct device *dev,
struct device_attribute *attr, char *buf)
{
u32 zvalue = ist30xx_zvalue;
DMSG("[TSP] tkey zvalue: %d\n", zvalue);

ist30xx_zvalue = 0;

return snprintf(buf, sizeof(int), "%d\n", zvalue);
}

ssize_t sec_tkey_zvalue_store(struct device *dev,
struct device_attribute *attr, const char *buf, size_t size)
{
int ret;
int cmd;
u32 val;

sscanf(buf, "%d", &cmd);

DMSG("[TSP] tkey zvalue cmd(%d)\n", cmd);

val = (cmd > 0 ? 0x00C80001 : 0);

if (cmd) {
	ist30xx_disable_irq(ts_data);
	ist30xx_cmd_run_device(ts_data->client);
	ist30xx_enable_irq(ts_data);
}

ret = ist30xx_write_cmd(ts_data->client, CMD_ZVALUE_MODE, val);
	if (ret) {
		pr_err("[ TSP ] fail, zvalue mode enter(%d)\n", ret);
return size;
	}

get_zvalue_mode = (cmd > 0 ? true : false);

DMSG("[TSP] tkey zvalue val: %x, cmd: %d\n", val, cmd);

return size;
}

/* /sys/class/sec/tkey/tkey_zval_range */
ssize_t sec_tkey_zval_range_show(struct device *dev,
struct device_attribute *attr, char *buf)
{
DMSG("[TSP] zvalue range: %d~%d\n",
IST30XX_MIN_ZVALUE_TKEY, IST30XX_MAX_ZVALUE_TKEY);

return sprintf(buf, "%d,%d\n",
IST30XX_MIN_ZVALUE_TKEY, IST30XX_MAX_ZVALUE_TKEY);
}


/* sysfs  */
static DEVICE_ATTR(close_tsp_test, S_IRUGO, sec_tsp_test_close_show, NULL);
static DEVICE_ATTR(cmd_status, S_IRUGO, sec_tsp_cmd_status_show, NULL);
static DEVICE_ATTR(cmd_result, S_IRUGO, sec_tsp_cmd_result_show, NULL);
static DEVICE_ATTR(cmd, S_IWUSR | S_IWGRP, NULL, sec_tsp_cmd_store);

static DEVICE_ATTR(intensity_logging_on, S_IRUGO, sec_tsp_intensity_log_on,
NULL);
static DEVICE_ATTR(intensity_logging_off, S_IRUGO, sec_tsp_intensity_log_off,
NULL);

static DEVICE_ATTR(read_frame, S_IRUGO, sec_tsp_read_frame_show, NULL);
static DEVICE_ATTR(read_node, S_IRUGO, sec_tsp_read_node_show, NULL);
static DEVICE_ATTR(raw, S_IRUGO, sec_tsp_raw_show, NULL);
static DEVICE_ATTR(baseline, S_IRUGO, sec_tsp_baseline_show, NULL);
static DEVICE_ATTR(chip_vendor, S_IRUGO, sec_tsp_chip_vendor_show, NULL);
static DEVICE_ATTR(chip_name, S_IRUGO, sec_tsp_chip_name_show, NULL);
static DEVICE_ATTR(panel_vendor, S_IRUGO, sec_tsp_panel_vendor_show, NULL);
static DEVICE_ATTR(fw_ver, S_IRUGO, sec_tsp_fw_ver_show, NULL);
static DEVICE_ATTR(raw_range, S_IRUGO, sec_tsp_raw_range_show, NULL);
static DEVICE_ATTR(base_range, S_IRUGO, sec_tsp_base_range_show, NULL);
static DEVICE_ATTR(zval_range, S_IRUGO, sec_tsp_zval_range_show, NULL);
static DEVICE_ATTR(node_num, S_IRUGO, sec_tsp_node_num_show, NULL);

static struct attribute *sec_tsp_attributes[] = {
&dev_attr_close_tsp_test.attr,
&dev_attr_cmd.attr,
&dev_attr_cmd_status.attr,
&dev_attr_cmd_result.attr,

&dev_attr_intensity_logging_on.attr,
&dev_attr_intensity_logging_off.attr,

&dev_attr_read_frame.attr,
&dev_attr_read_node.attr,
&dev_attr_raw.attr,
&dev_attr_baseline.attr,
&dev_attr_chip_vendor.attr,
&dev_attr_chip_name.attr,
&dev_attr_panel_vendor.attr,
&dev_attr_fw_ver.attr,
&dev_attr_raw_range.attr,
&dev_attr_base_range.attr,
&dev_attr_zval_range.attr,
&dev_attr_node_num.attr,

NULL,
};
static struct attribute_group sec_tsp_attr_group = {
.attrs = sec_tsp_attributes,
};

static DEVICE_ATTR(tkey_back, S_IRUGO, sec_back_key_state_show, NULL);
static DEVICE_ATTR(tkey_home, S_IRUGO, sec_home_key_state_show, NULL);
static DEVICE_ATTR(tkey_menu, S_IRUGO, sec_menu_key_state_show, NULL);
static DEVICE_ATTR(tkey_recent, S_IRUGO, sec_recent_key_state_show, NULL);
static DEVICE_ATTR(tkey_threshold, S_IRUGO, sec_tkey_threshold_show, NULL);
static DEVICE_ATTR(tkey_raw_data0, S_IRUGO, sec_tkey_rawcnt0_show, NULL);
static DEVICE_ATTR(tkey_raw_data1, S_IRUGO, sec_tkey_rawcnt1_show, NULL);
static DEVICE_ATTR(tkey_raw_data2, S_IRUGO, sec_tkey_rawcnt2_show, NULL);
static DEVICE_ATTR(tkey_raw_data3, S_IRUGO, sec_tkey_rawcnt3_show, NULL);
static DEVICE_ATTR(tkey_zvalue, S_IRWXUGO,
		sec_tkey_zvalue_show, sec_tkey_zvalue_store);
static DEVICE_ATTR(tkey_zval_range, S_IRUGO, sec_tkey_zval_range_show, NULL);

static struct attribute *sec_tkey_attributes[] = {
&dev_attr_tkey_back.attr,
&dev_attr_tkey_home.attr,
&dev_attr_tkey_menu.attr,
&dev_attr_tkey_recent.attr,
&dev_attr_tkey_threshold.attr,
&dev_attr_tkey_raw_data0.attr,
&dev_attr_tkey_raw_data1.attr,
&dev_attr_tkey_raw_data2.attr,
&dev_attr_tkey_raw_data3.attr,

&dev_attr_tkey_zvalue.attr,
&dev_attr_tkey_zval_range.attr,
NULL,
};
static struct attribute_group sec_tkey_attr_group = {
.attrs = sec_tkey_attributes,
};


struct class *sec_dev_class;
struct device *sec_tsp_dev;
struct device *sec_tk_dev;


int ist30xx_sec_sysfs(void)
{

sec_dev_class = class_create(THIS_MODULE, "sec_factory");
if (!sec_dev_class)
		pr_err("[ TSP ] Failed to create device class(%s)!\n", "sec");

/* /sys/class/sec_factory/tsp */
	sec_tsp_dev = device_create(sec_dev_class, NULL, 0, NULL, "tsp");

	/* /sys/class/sec/tsp/... */
	if (sysfs_create_group(&sec_tsp_dev->kobj, &sec_tsp_attr_group))
		pr_err(
			"[ TSP ] Failed to create sysfs group(%s)!\n",
		"sec_tsp");

	/* /sys/class/sec/tkey */
	sec_tk_dev = device_create(sec_dev_class, NULL, 0, NULL, "tkey");

	/* /sys/class/sec/tkey/... */
if (sysfs_create_group(&sec_tk_dev->kobj, &sec_tkey_attr_group))
		pr_err("[ TSP ] Failed to create sysfs group(%s)!\n", "tkey");

	node_raw_buf = kmalloc(4096, GFP_KERNEL);
	node_baseline_buf = kmalloc(4096, GFP_KERNEL);

	return 0;
}
EXPORT_SYMBOL(ist30xx_sec_sysfs);
