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


#include <linux/mutex.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/stat.h>
#include <linux/err.h>

#include "ist30xx.h"
#include "ist30xx_update.h"
#include "ist30xx_misc.h"


static char IsfwUpdate[20] = { 0 };

#define FW_DOWNLOADING "Downloading"
#define FW_DOWNLOAD_COMPLETE "Complete"
#define FW_DOWNLOAD_FAIL "FAIL"

#define FACTORY_BUF_SIZE    (1024)
#define BUILT_IN            (0)

#define CMD_STATE_WAITING   (0)
#define CMD_STATE_RUNNING   (1)
#define CMD_STATE_OK        (2)
#define CMD_STATE_FAIL      (3)
#define CMD_STATE_NA        (4)

#define TSP_NODE_DEBUG      (1)

#define NODE_X_NUM 12
#define NODE_Y_NUM 18

static u16 node_value[TSP_TOTAL_NUM];

extern struct ist30xx_data *ts_data;
extern TSP_INFO ist30xx_tsp_info;
extern TKEY_INFO ist30xx_tkey_info;


#define TSP_CMD(name, func) .cmd_name = name, .cmd_func = func
struct tsp_cmd {
	struct list_head	list;
	const char *		cmd_name;
	void			(*cmd_func)(void *dev_data);
};

u32 ist30xxb_get_fw_ver(struct ist30xx_data *data)
{
	u32 addr = data->tags.cfg_addr + 4;
	u32 len = 1;
	u32 ver = 0;
	int ret = -EPERM;

	ist30xx_disable_irq(data);
	ret = ist30xx_cmd_reg(data->client, CMD_ENTER_REG_ACCESS);
	if (ret) goto get_fw_ver_fail;

	ret = ist30xx_write_cmd(data->client, IST30XX_RX_CNT_ADDR, len);
	if (ret) goto get_fw_ver_fail;

	ret = ist30xx_read_cmd(data->client, addr, &ver);
	if (ret) goto get_fw_ver_fail;

	tsp_debug("Reg addr: %x, ver: %x\n", addr, ver);

	ret = ist30xx_cmd_reg(data->client, CMD_EXIT_REG_ACCESS);
	if (ret == 0)
		goto get_fw_ver_end;

get_fw_ver_fail:
	ist30xx_cmd_run_device(data->client);

get_fw_ver_end:
	ist30xx_cmd_start_scan(data->client);
	ist30xx_enable_irq(data);

	return ver;
}

int ist30xxb_get_key_sensitivity(struct ist30xx_data *data, int id)
{
	u32 addr = IST30XXB_MEM_COUNT + 4 * sizeof(u32);
	u32 val = 0;

	if (id >= ist30xx_tkey_info.key_num)
		return 0;

	ist30xx_disable_irq(data);
	ist30xx_read_cmd(ts_data->client, addr, &val);
	ist30xx_enable_irq(data);

	tsp_debug("Reg addr: %x, val: %8x\n", addr, val);

	val >>= (16 * id);

	return (int)(val & 0xFFFF);
}


/* Factory CMD function */
static void set_default_result(struct sec_factory *sec)
{
	char delim = ':';

	memset(sec->cmd_result, 0, ARRAY_SIZE(sec->cmd_result));
	memcpy(sec->cmd_result, sec->cmd, strlen(sec->cmd));
	strncat(sec->cmd_result, &delim, CMD_STATE_RUNNING);
}

static void set_cmd_result(struct sec_factory *sec, char *buf, int len)
{
	strncat(sec->cmd_result, buf, len);
}

static void not_support_cmd(void *dev_data)
{
	char buf[16] = { 0 };

	struct ist30xx_data *data = (struct ist30xx_data *)dev_data;
	struct sec_factory *sec = (struct sec_factory *)&data->sec;

	set_default_result(sec);
	snprintf(buf, sizeof(buf), "%s", "NA");
	tsp_info("%s(), %s\n", __func__, buf);

	set_cmd_result(sec, buf, strnlen(buf, sizeof(buf)));
	sec->cmd_state = CMD_STATE_NA;
	dev_info(&data->client->dev, "%s: \"%s(%d)\"\n", __func__,
		 buf, strnlen(buf, sizeof(buf)));
	return;
}

static void get_chip_vendor(void *dev_data)
{
	char buf[16] = { 0 };

	struct ist30xx_data *data = (struct ist30xx_data *)dev_data;
	struct sec_factory *sec = (struct sec_factory *)&data->sec;

	set_default_result(sec);
	snprintf(buf, sizeof(buf), "%s", TSP_CHIP_VENDOR);
	tsp_info("%s(), %s\n", __func__, buf);

	set_cmd_result(sec, buf, strnlen(buf, sizeof(buf)));
	sec->cmd_state = CMD_STATE_OK;
	dev_info(&data->client->dev, "%s: %s(%d)\n", __func__,
		 buf, strnlen(buf, sizeof(buf)));
}

static void get_chip_name(void *dev_data)
{
	char buf[16] = { 0 };

	struct ist30xx_data *data = (struct ist30xx_data *)dev_data;
	struct sec_factory *sec = (struct sec_factory *)&data->sec;

	set_default_result(sec);
	snprintf(buf, sizeof(buf), "%s", TSP_CHIP_NAME);
	tsp_info("%s(), %s\n", __func__, buf);

	set_cmd_result(sec, buf, strnlen(buf, sizeof(buf)));
	sec->cmd_state = CMD_STATE_OK;
	dev_info(&data->client->dev, "%s: %s(%d)\n", __func__,
		 buf, strnlen(buf, sizeof(buf)));
}

static void get_chip_id(void *dev_data)
{
	char buf[16] = { 0 };

	struct ist30xx_data *data = (struct ist30xx_data *)dev_data;
	struct sec_factory *sec = (struct sec_factory *)&data->sec;

	set_default_result(sec);
	snprintf(buf, sizeof(buf), "%#02x", data->chip_id);
	tsp_info("%s(), %s\n", __func__, buf);

	set_cmd_result(sec, buf, strnlen(buf, sizeof(buf)));
	sec->cmd_state = CMD_STATE_OK;
	dev_info(&data->client->dev, "%s: %s(%d)\n", __func__,
		 buf, strnlen(buf, sizeof(buf)));
}

static void fw_update(void *dev_data)
{
	int ret;
	char buf[16] = { 0 };

	struct ist30xx_data *data = (struct ist30xx_data *)dev_data;
	struct sec_factory *sec = (struct sec_factory *)&data->sec;

	set_default_result(sec);

	tsp_info("%s(), %d\n", __func__, sec->cmd_param[0]);

	switch (sec->cmd_param[0]) {
	case BUILT_IN:
		sec->cmd_state = CMD_STATE_OK;
		ret = ist30xx_fw_recovery(data);
		if (ret < 0) {
			sec->cmd_state = CMD_STATE_FAIL;
			return;
		}
		break;

	default:
		tsp_warn("%s(), Invalid fw file type!\n", __func__);
		break;
	}

	if (sec->cmd_state == CMD_STATE_OK)
		snprintf(buf, sizeof(buf), "%s", "OK");
	else
		snprintf(buf, sizeof(buf), "%s", "NG");

	set_cmd_result(sec, buf, strnlen(buf, sizeof(buf)));
}


static void get_fw_ver_bin(void *dev_data)
{
	u32 ver = 0;
	char buf[16] = { 0 };

	struct ist30xx_data *data = (struct ist30xx_data *)dev_data;
	struct sec_factory *sec = (struct sec_factory *)&data->sec;

	set_default_result(sec);

	ver = ist30xx_parse_ver(FLAG_PARAM, data->fw.buf);
	snprintf(buf, sizeof(buf), "IM00%04x", ver);
	tsp_info("%s(), %s\n", __func__, buf);

		set_cmd_result(sec, buf, strnlen(buf, sizeof(buf)));
	sec->cmd_state = CMD_STATE_OK;
	dev_info(&data->client->dev, "%s: %s(%d)\n", __func__,
		 buf, strnlen(buf, sizeof(buf)));
}

static void get_fw_ver_ic(void *dev_data)
{
	u32 ver = 0;
	char buf[16] = { 0 };

	struct ist30xx_data *data = (struct ist30xx_data *)dev_data;
	struct sec_factory *sec = (struct sec_factory *)&data->sec;

	set_default_result(sec);

	if (data->status.power == 1)
		ver = ist30xxb_get_fw_ver(ts_data);

	snprintf(buf, sizeof(buf), "IM00%04x", ver & 0xFFFF);
	tsp_info("%s(), %s\n", __func__, buf);

	set_cmd_result(sec, buf, strnlen(buf, sizeof(buf)));
	sec->cmd_state = CMD_STATE_OK;
	dev_info(&data->client->dev, "%s: %s(%d)\n", __func__,
		 buf, strnlen(buf, sizeof(buf)));
}

static void get_threshold(void *dev_data)
{
	char buf[16] = { 0 };
	int val = TSP_THRESHOLD;

	struct ist30xx_data *data = (struct ist30xx_data *)dev_data;
	struct sec_factory *sec = (struct sec_factory *)&data->sec;

	set_default_result(sec);

	snprintf(buf, sizeof(buf), "%d", val);
	tsp_info("%s(), %s\n", __func__, buf);

	set_cmd_result(sec, buf, strnlen(buf, sizeof(buf)));
	sec->cmd_state = CMD_STATE_OK;
	dev_info(&data->client->dev, "%s: %s(%d)\n", __func__,
		 buf, strnlen(buf, sizeof(buf)));
}

static void get_x_num(void *dev_data)
{
	int val = ist30xx_tsp_info.width;
	char buf[16] = { 0 };

	struct ist30xx_data *data = (struct ist30xx_data *)dev_data;
	struct sec_factory *sec = (struct sec_factory *)&data->sec;

	set_default_result(sec);

	if (val >= 0) {
		snprintf(buf, sizeof(buf), "%u", val);
		sec->cmd_state = CMD_STATE_OK;
		dev_info(&data->client->dev, "%s: %s(%d)\n", __func__, buf,
			 strnlen(buf, sizeof(buf)));
	} else {
		snprintf(buf, sizeof(buf), "%s", "NG");
		sec->cmd_state = CMD_STATE_FAIL;
		dev_info(&data->client->dev,
			 "%s: fail to read num of x (%d).\n", __func__, val);
	}
	set_cmd_result(sec, buf, strnlen(buf, sizeof(buf)));
	tsp_info("%s(), %s\n", __func__, buf);
}

static void get_y_num(void *dev_data)
{
	int val = ist30xx_tsp_info.height;
	char buf[16] = { 0 };

	struct ist30xx_data *data = (struct ist30xx_data *)dev_data;
	struct sec_factory *sec = (struct sec_factory *)&data->sec;

	set_default_result(sec);

	if (val >= 0) {
		snprintf(buf, sizeof(buf), "%u", val);
		sec->cmd_state = CMD_STATE_OK;
		dev_info(&data->client->dev, "%s: %s(%d)\n", __func__, buf,
			 strnlen(buf, sizeof(buf)));
	} else {
		snprintf(buf, sizeof(buf), "%s", "NG");
		sec->cmd_state = CMD_STATE_FAIL;
		dev_info(&data->client->dev,
			 "%s: fail to read num of x (%d).\n", __func__, val);
	}
	set_cmd_result(sec, buf, strnlen(buf, sizeof(buf)));
	tsp_info("%s(), %s\n", __func__, buf);
}

static int check_rx_tx_num(void *dev_data)
{
	struct ist30xx_data *data = (struct ist30xx_data *)dev_data;
	struct sec_factory *sec = (struct sec_factory *)&data->sec;
	
	char buff[SEC_CMD_STR_LEN] = {0};
	int node;

	if (sec->cmd_param[0] < 0 ||
			sec->cmd_param[0] >= NODE_X_NUM  ||
			sec->cmd_param[1] < 0 ||
			sec->cmd_param[1] >= NODE_Y_NUM) {
		snprintf(buff, sizeof(buff) , "%s", "NG");
		set_cmd_result(sec, buff, strnlen(buff, sizeof(buff)));
		sec->cmd_state = 3;

		dev_info(&data->client->dev, "%s: parameter error: %u,%u\n",
				__func__, sec->cmd_param[0],
				sec->cmd_param[1]);
		node = -1;
		return node;
             }
	//node = info->cmd_param[1] * NODE_Y_NUM + info->cmd_param[0];
	node = sec->cmd_param[0] * NODE_Y_NUM + sec->cmd_param[1];
	dev_info(&data->client->dev, "%s: node = %d\n", __func__,
			node);
	return node;

 }

void get_cm_abs(void *dev_data)
{
	struct ist30xx_data *data = (struct ist30xx_data *)dev_data;
	struct sec_factory *sec = (struct sec_factory *)&data->sec;
	
	char buff[16] = {0};
	unsigned int val;
	int node;

	printk("[TSP] %s, %d\n", __func__, __LINE__ );

	set_default_result(sec);
	node = check_rx_tx_num(data);

	if (node < 0)
		return;

	val = node_value[node];
	snprintf(buff, sizeof(buff), "%u", val);
	set_cmd_result(sec, buff, strnlen(buff, sizeof(buff)));
	sec->cmd_state = 2;

	dev_info(&data->client->dev, "%s: %s(%d)\n", __func__, buff,
			strnlen(buff, sizeof(buff)));
    
}

void get_reference(void *dev_data)
{
	struct ist30xx_data *data = (struct ist30xx_data *)dev_data;
	struct sec_factory *sec = (struct sec_factory *)&data->sec;
	
	char buff[16] = {0};
	unsigned int val;
	int node;

	printk("[TSP] %s, %d\n", __func__, __LINE__ );

	set_default_result(sec);
	node = check_rx_tx_num(data);

	if (node < 0)
		return;

	val = node_value[node];
	snprintf(buff, sizeof(buff), "%u", val);
	set_cmd_result(sec, buff, strnlen(buff, sizeof(buff)));
	sec->cmd_state = 2;

	dev_info(&data->client->dev, "%s: %s(%d)\n", __func__, buff,
			strnlen(buff, sizeof(buff)));
    
}

int check_tsp_channel(void *dev_data)
{
	int node = -EPERM;

	struct ist30xx_data *data = (struct ist30xx_data *)dev_data;
	struct sec_factory *sec = (struct sec_factory *)&data->sec;
	TSP_INFO *tsp = &ist30xx_tsp_info;

	if ((sec->cmd_param[0] < 0) || (sec->cmd_param[0] >= tsp->width) ||
	    (sec->cmd_param[1] < 0) || (sec->cmd_param[1] >= tsp->height)) {
		dev_info(&data->client->dev, "%s: parameter error: %u,%u\n",
			 __func__, sec->cmd_param[0], sec->cmd_param[1]);
	} else {
		node = sec->cmd_param[0] + sec->cmd_param[1] * tsp->width;
		dev_info(&data->client->dev, "%s: node = %d\n", __func__, node);
	}

	return node;
}


extern int parse_tsp_node(u8 flag, struct TSP_NODE_BUF *node, s16 *buf16);
void run_raw_read(void *dev_data)
{
	int i, ret;
	int min_val, max_val;
	char buf[16] = { 0 };
	u8 flag = NODE_FLAG_RAW;

	struct ist30xx_data *data = (struct ist30xx_data *)dev_data;
	struct sec_factory *sec = (struct sec_factory *)&data->sec;
	TSP_INFO *tsp = &ist30xx_tsp_info;

	set_default_result(sec);

	ret = ist30xx_read_touch_node(flag, &tsp->node);
	if (ret) {
		sec->cmd_state = CMD_STATE_FAIL;
		tsp_warn("%s(), tsp node read fail!\n", __func__);
		return;
	}
	ist30xx_parse_touch_node(flag, &tsp->node);

	ret = parse_tsp_node(flag, &tsp->node, node_value);
	if (ret) {
		sec->cmd_state = CMD_STATE_FAIL;
		tsp_warn("%s(), tsp node parse fail - flag: %d\n", __func__, flag);
		return;
	}
	sec->cmd_state = CMD_STATE_OK;
}

void run_reference_read(void *dev_data)
{
	int i, ret;
	int min_val, max_val;
	char buf[16] = { 0 };
	u8 flag = NODE_FLAG_RAW;

	struct ist30xx_data *data = (struct ist30xx_data *)dev_data;
	struct sec_factory *sec = (struct sec_factory *)&data->sec;
	TSP_INFO *tsp = &ist30xx_tsp_info;

	set_default_result(sec);

	ret = ist30xx_read_touch_node(flag, &tsp->node);
	if (ret) {
		sec->cmd_state = CMD_STATE_FAIL;
		tsp_warn("%s(), tsp node read fail!\n", __func__);
		return;
	}
	ist30xx_parse_touch_node(flag, &tsp->node);

	ret = parse_tsp_node(flag, &tsp->node, node_value);
	if (ret) {
		sec->cmd_state = CMD_STATE_FAIL;
		tsp_warn("%s(), tsp node parse fail - flag: %d\n", __func__, flag);
		return;
	}

	min_val = max_val = node_value[0];
	for (i = 0; i < tsp->width * tsp->height; i++) {
#if TSP_NODE_DEBUG
		if ((i % tsp->width) == 0)
			printk("\n%s %4d: ", IST30XX_DEBUG_TAG, i);

		printk("%4d ", node_value[i]);
#endif

		max_val = max(max_val, (int)node_value[i]);
		min_val = min(min_val, (int)node_value[i]);
	}

	snprintf(buf, sizeof(buf), "%d,%d", min_val, max_val);
	tsp_info("%s(), %s\n", __func__, buf);

	set_cmd_result(sec, buf, strnlen(buf, sizeof(buf)));

	sec->cmd_state = CMD_STATE_OK;
	dev_info(&data->client->dev, "%s: %s(%d)\n", __func__, buf,
		 strnlen(buf, sizeof(buf)));
}

void get_raw_value(void *dev_data)
{
	int idx = 0;
	char buf[16] = { 0 };

	struct ist30xx_data *data = (struct ist30xx_data *)dev_data;
	struct sec_factory *sec = (struct sec_factory *)&data->sec;

	set_default_result(sec);

	idx = check_tsp_channel(data);
	if (idx < 0) { // Parameter parsing fail
		snprintf(buf, sizeof(buf), "%s", "NG");
		sec->cmd_state = CMD_STATE_FAIL;
	} else {
		snprintf(buf, sizeof(buf), "%d", node_value[idx]);
		sec->cmd_state = CMD_STATE_OK;
	}

	set_cmd_result(sec, buf, strnlen(buf, sizeof(buf)));
	tsp_info("%s(), [%d][%d]: %s\n", __func__,
		 sec->cmd_param[0], sec->cmd_param[1], buf);
	dev_info(&data->client->dev, "%s: %s(%d)\n", __func__, buf,
		 strnlen(buf, sizeof(buf)));
}



/* sysfs: /sys/class/sec/tsp/close_tsp_test */
static ssize_t show_close_tsp_test(struct device *dev,
				   struct device_attribute *attr, char *buf)
{
	return snprintf(buf, FACTORY_BUF_SIZE, "%u\n", 0);
}

/* sysfs: /sys/class/sec/tsp/cmd */
static ssize_t store_cmd(struct device *dev, struct device_attribute
			 *devattr, const char *buf, size_t count)
{
	struct ist30xx_data *data = dev_get_drvdata(dev);
	struct sec_factory *sec = (struct sec_factory *)&data->sec;
	struct i2c_client *client = data->client;

	char *cur, *start, *end;
	char msg[SEC_CMD_STR_LEN] = { 0 };
	int len, i;
	struct tsp_cmd *tsp_cmd_ptr = NULL;
	char delim = ',';
	bool cmd_found = false;
	int param_cnt = 0;
	int ret;

	if (sec->cmd_is_running == true) {
		dev_err(&client->dev, "tsp_cmd: other cmd is running.\n");
		tsp_err("tsp_cmd: other cmd is running.\n");
		goto err_out;
	}

	/* check lock  */
	mutex_lock(&sec->cmd_lock);
	sec->cmd_is_running = true;
	mutex_unlock(&sec->cmd_lock);

	sec->cmd_state = CMD_STATE_RUNNING;

	for (i = 0; i < ARRAY_SIZE(sec->cmd_param); i++)
		sec->cmd_param[i] = 0;

	len = (int)count;
	if (*(buf + len - 1) == '\n')
		len--;
	memset(sec->cmd, 0, ARRAY_SIZE(sec->cmd));
	memcpy(sec->cmd, buf, len);

	cur = strchr(buf, (int)delim);
	if (cur)
		memcpy(msg, buf, cur - buf);
	else
		memcpy(msg, buf, len);
	/* find command */
	list_for_each_entry(tsp_cmd_ptr, &sec->cmd_list_head, list) {
		if (!strcmp(msg, tsp_cmd_ptr->cmd_name)) {
			cmd_found = true;
			break;
		}
	}

	/* set not_support_cmd */
	if (!cmd_found) {
		list_for_each_entry(tsp_cmd_ptr, &sec->cmd_list_head, list) {
			if (!strcmp("not_support_cmd", tsp_cmd_ptr->cmd_name))
				break;
		}
	}

	/* parsing parameters */
	if (cur && cmd_found) {
		cur++;
		start = cur;
		memset(msg, 0, ARRAY_SIZE(msg));
		do {
			if (*cur == delim || cur - buf == len) {
				end = cur;
				memcpy(msg, start, end - start);
				*(msg + strlen(msg)) = '\0';
				ret = kstrtoint(msg, 10, \
						sec->cmd_param + param_cnt);
				start = cur + 1;
				memset(msg, 0, ARRAY_SIZE(msg));
				param_cnt++;
			}
			cur++;
		} while (cur - buf <= len);
	}
	dev_info(&client->dev, "cmd = %s\n", tsp_cmd_ptr->cmd_name);
	tsp_info("SEC CMD = %s\n", tsp_cmd_ptr->cmd_name);

	for (i = 0; i < param_cnt; i++)
		dev_info(&client->dev, "cmd param %d= %d\n", i, sec->cmd_param[i]);

	tsp_cmd_ptr->cmd_func(data);

err_out:
	return count;
}

/* sysfs: /sys/class/sec/tsp/cmd_status */
static ssize_t show_cmd_status(struct device *dev,
			       struct device_attribute *devattr, char *buf)
{
	struct ist30xx_data *data = dev_get_drvdata(dev);
	struct sec_factory *sec = (struct sec_factory *)&data->sec;
	char msg[16] = { 0 };

	dev_info(&data->client->dev, "tsp cmd: status:%d\n", sec->cmd_state);

	if (sec->cmd_state == CMD_STATE_WAITING)
		snprintf(msg, sizeof(msg), "WAITING");

	else if (sec->cmd_state == CMD_STATE_RUNNING)
		snprintf(msg, sizeof(msg), "RUNNING");

	else if (sec->cmd_state == CMD_STATE_OK)
		snprintf(msg, sizeof(msg), "OK");

	else if (sec->cmd_state == CMD_STATE_FAIL)
		snprintf(msg, sizeof(msg), "FAIL");

	else if (sec->cmd_state == CMD_STATE_NA)
		snprintf(msg, sizeof(msg), "NOT_APPLICABLE");

	return snprintf(buf, FACTORY_BUF_SIZE, "%s\n", msg);
}

/* sysfs: /sys/class/sec/tsp/result */
static ssize_t show_cmd_result(struct device *dev, struct device_attribute
			       *devattr, char *buf)
{
	struct ist30xx_data *data = dev_get_drvdata(dev);
	struct sec_factory *sec = (struct sec_factory *)&data->sec;

	dev_info(&data->client->dev, "tsp cmd: result: %s\n", sec->cmd_result);

	mutex_lock(&sec->cmd_lock);
	sec->cmd_is_running = false;
	mutex_unlock(&sec->cmd_lock);

	sec->cmd_state = CMD_STATE_WAITING;

	return snprintf(buf, FACTORY_BUF_SIZE, "%s\n", sec->cmd_result);
}



/* sysfs: /sys/class/sec/tsp/sec_touchscreen/tsp_firm_version_phone */
static ssize_t phone_firmware_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	u32 ver = ist30xx_parse_ver(FLAG_PARAM, ts_data->fw.buf);

	tsp_info("%s(), IM00%04x\n", __func__, ver);

	return sprintf(buf, "IM00%04x", ver);
}

/* sysfs: /sys/class/sec/tsp/sec_touchscreen/tsp_firm_version_panel */
static ssize_t part_firmware_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	u32 ver = 0;

	if (ts_data->status.power == 1)
		ver = ist30xxb_get_fw_ver(ts_data);

	tsp_info("%s(), IM00%04x\n", __func__, ver);

	return sprintf(buf, "IM00%04x", ver);
}

/* sysfs: /sys/class/sec/tsp/sec_touchscreen/tsp_threshold */
static ssize_t threshold_firmware_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int val = TSP_THRESHOLD;

	tsp_info("%s(), %d\n", __func__, val);

	return sprintf(buf, "%d", val);
}

/* sysfs: /sys/class/sec/tsp/sec_touchscreen/tsp_firm_update */
static ssize_t firmware_update(struct device *dev, struct device_attribute *attr, char *buf)
{
	bool ret;

	sprintf(IsfwUpdate, "%s\n", FW_DOWNLOADING);
	tsp_info("%s(), %s\n", __func__, IsfwUpdate);

	ret = ist30xx_fw_recovery(ts_data);
	ret = (ret == 0 ? 1 : -1);

	sprintf(IsfwUpdate, "%s\n",
		(ret > 0 ? FW_DOWNLOAD_COMPLETE : FW_DOWNLOAD_FAIL));
	tsp_info("%s(), %s\n", __func__, IsfwUpdate);

	return sprintf(buf, "%d", ret);
}

/* sysfs: /sys/class/sec/tsp/sec_touchscreen/tsp_firm_update_status */
static ssize_t firmware_update_status(struct device *dev, struct device_attribute *attr, char *buf)
{
	tsp_info("%s(), %s\n", __func__, IsfwUpdate);

	return sprintf(buf, "%s\n", IsfwUpdate);
}


/* sysfs: /sys/class/sec/tsp/sec_touchkey/touchkey_menu */
static ssize_t menu_sensitivity_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int sensitivity = ist30xxb_get_key_sensitivity(ts_data, 0);

	tsp_info("%s(), %d\n", __func__, sensitivity);

	return sprintf(buf, "%d\n", sensitivity);
}

/* sysfs: /sys/class/sec/tsp/sec_touchkey/touchkey_back */
static ssize_t back_sensitivity_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int sensitivity = ist30xxb_get_key_sensitivity(ts_data, 1);

	tsp_info("%s(), %d\n", __func__, sensitivity);

	return sprintf(buf, "%d\n", sensitivity);
}

/* sysfs: /sys/class/sec/tsp/sec_touchkey/touchkey_threshold */
static ssize_t touchkey_threshold_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int val = TKEY_THRESHOLD;

	tsp_info("%s(), %d\n", __func__, val);

	return snprintf(buf, sizeof(int), "%d\n", val);
}



struct tsp_cmd tsp_cmds[] = {
	{ TSP_CMD("fw_update",		not_support_cmd), },
	{ TSP_CMD("get_fw_ver_bin",	get_fw_ver_bin),  },
	{ TSP_CMD("get_fw_ver_ic",	get_fw_ver_ic),	  },
	{ TSP_CMD("get_config_ver",	not_support_cmd), },
	{ TSP_CMD("get_threshold",	not_support_cmd), },
	{ TSP_CMD("module_off_master",	not_support_cmd), },
	{ TSP_CMD("module_on_master",	not_support_cmd), },
	{ TSP_CMD("module_off_slave",	not_support_cmd), },
	{ TSP_CMD("module_on_slave",	not_support_cmd), },
	{ TSP_CMD("get_chip_vendor",	get_chip_vendor), },
	{ TSP_CMD("get_chip_name",	get_chip_name),	  },
	{ TSP_CMD("run_rawcap_read",	run_raw_read), },
	{ TSP_CMD("get_x_num",		get_x_num),	  },
	{ TSP_CMD("get_y_num",		get_y_num),	  },
	{TSP_CMD("get_reference", get_reference),},
	{ TSP_CMD("get_cm_abs",		get_cm_abs), },
	{ TSP_CMD("get_rawcap",		not_support_cmd), },
	{ TSP_CMD("get_cm_delta",	not_support_cmd), },
	{ TSP_CMD("get_intensity",	not_support_cmd), },
	{TSP_CMD("run_reference_read", run_reference_read),},
	{ TSP_CMD("run_cm_abs_read",	run_raw_read), },
	{ TSP_CMD("run_cm_delta_read",	not_support_cmd), },
	{ TSP_CMD("run_intensity_read", not_support_cmd), },
	{ TSP_CMD("not_support_cmd",	not_support_cmd), },
};


#define SEC_DEFAULT_ATTR    (S_IRUGO | S_IWUSR | S_IWOTH | S_IXOTH)
/* sysfs - touchscreen */
static DEVICE_ATTR(tsp_firm_version_phone, SEC_DEFAULT_ATTR,
		   phone_firmware_show, NULL);
static DEVICE_ATTR(tsp_firm_version_panel, SEC_DEFAULT_ATTR,
		   part_firmware_show, NULL);
static DEVICE_ATTR(tsp_threshold, SEC_DEFAULT_ATTR,
		   threshold_firmware_show, NULL);
static DEVICE_ATTR(tsp_firm_update, SEC_DEFAULT_ATTR,
		   firmware_update, NULL);
static DEVICE_ATTR(tsp_firm_update_status, SEC_DEFAULT_ATTR,
		   firmware_update_status, NULL);

/* sysfs - touchkey */
static DEVICE_ATTR(touchkey_menu, SEC_DEFAULT_ATTR,
		   menu_sensitivity_show, NULL);
static DEVICE_ATTR(touchkey_back, SEC_DEFAULT_ATTR,
		   back_sensitivity_show, NULL);
static DEVICE_ATTR(touchkey_threshold, SEC_DEFAULT_ATTR,
		   touchkey_threshold_show, NULL);

/* sysfs - tsp */
static DEVICE_ATTR(close_tsp_test, S_IRUGO, show_close_tsp_test, NULL);
static DEVICE_ATTR(cmd, S_IWUSR | S_IWGRP, NULL, store_cmd);
static DEVICE_ATTR(cmd_status, S_IRUGO, show_cmd_status, NULL);
static DEVICE_ATTR(cmd_result, S_IRUGO, show_cmd_result, NULL);

static struct attribute *sec_tsp_attributes[] = {
	&dev_attr_tsp_firm_version_phone.attr,
	&dev_attr_tsp_firm_version_panel.attr,
	&dev_attr_tsp_threshold.attr,
	&dev_attr_tsp_firm_update.attr,
	&dev_attr_tsp_firm_update_status.attr,
	NULL,
};

static struct attribute *sec_tkey_attributes[] = {
	&dev_attr_touchkey_menu.attr,
	&dev_attr_touchkey_back.attr,
	&dev_attr_touchkey_threshold.attr,
	NULL,
};

static struct attribute *sec_touch_facotry_attributes[] = {
	&dev_attr_close_tsp_test.attr,
	&dev_attr_cmd.attr,
	&dev_attr_cmd_status.attr,
	&dev_attr_cmd_result.attr,
	NULL,
};

static struct attribute_group sec_tsp_attr_group = {
	.attrs	= sec_tsp_attributes,
};

static struct attribute_group sec_tkey_attr_group = {
	.attrs	= sec_tkey_attributes,
};

static struct attribute_group sec_touch_factory_attr_group = {
	.attrs	= sec_touch_facotry_attributes,
};

extern struct class *sec_class;
struct device *sec_touchscreen;
EXPORT_SYMBOL(sec_touchscreen);
struct device *sec_touchkey;
EXPORT_SYMBOL(sec_touchkey);
struct device *sec_fac_dev;


int sec_touch_sysfs(struct ist30xx_data *data)
{

	printk("[TSP] %s\n", __func__);
	/* /sys/class/sec/sec_touchscreen */
	sec_touchscreen = device_create(sec_class, NULL, 0, NULL, "sec_touchscreen");
	if (IS_ERR(sec_touchscreen)) {
		tsp_err("Failed to create device (%s)!\n", "sec_touchscreen");
		return -ENODEV;
	}
	/* /sys/class/sec/sec_touchscreen/... */
	if (sysfs_create_group(&sec_touchscreen->kobj, &sec_tsp_attr_group))
		tsp_err("Failed to create sysfs group(%s)!\n", "sec_touchscreen");

	/* /sys/class/sec/sec_touchkey */
	sec_touchkey = device_create(sec_class, NULL, 0, NULL, "sec_touchkey");
	if (IS_ERR(sec_touchkey)) {
		tsp_err("Failed to create device (%s)!\n", "sec_touchkey");
		return -ENODEV;
	}
	/* /sys/class/sec/sec_touchkey/... */
	if (sysfs_create_group(&sec_touchkey->kobj, &sec_tkey_attr_group))
		tsp_err("Failed to create sysfs group(%s)!\n", "sec_touchkey");

	/* /sys/class/sec/tsp */
	sec_fac_dev = device_create(sec_class, NULL, 0, data, "tsp");
	if (IS_ERR(sec_fac_dev)) {
		tsp_err("Failed to create device (%s)!\n", "tsp");
		return -ENODEV;
	}
	/* /sys/class/sec/tsp/... */
	if (sysfs_create_group(&sec_fac_dev->kobj, &sec_touch_factory_attr_group))
		tsp_err("Failed to create sysfs group(%s)!\n", "tsp");

	return 0;
}
EXPORT_SYMBOL(sec_touch_sysfs);
int sec_fac_cmd_init(struct ist30xx_data *data)
{
	int i;
	struct sec_factory *sec = (struct sec_factory *)&data->sec;

	INIT_LIST_HEAD(&sec->cmd_list_head);
	for (i = 0; i < ARRAY_SIZE(tsp_cmds); i++)
		list_add_tail(&tsp_cmds[i].list, &sec->cmd_list_head);

	mutex_init(&sec->cmd_lock);
	sec->cmd_is_running = false;

	return 0;
}
EXPORT_SYMBOL(sec_fac_cmd_init);