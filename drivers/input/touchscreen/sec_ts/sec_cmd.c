/*
 * sec_cmd.c - samsung factory command driver
 *
 * Copyright (C) 2014 Samsung Electronics
 * Copyright (C) 2018 Sony Mobile Communications Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include "include/sec_ts.h"

#if defined USE_SEC_CMD_QUEUE
static void sec_cmd_store_function(struct sec_cmd_data *data);
#endif

void sec_cmd_set_cmd_exit(struct sec_cmd_data *data)
{
	mutex_lock(&data->cmd_lock);
	data->cmd_is_running = false;
	mutex_unlock(&data->cmd_lock);

#ifdef USE_SEC_CMD_QUEUE
	mutex_lock(&data->fifo_lock);
	if (kfifo_len(&data->cmd_queue)) {
		pr_info("%s %s: do next cmd, left cmd[%d]\n", SECLOG, __func__,
			(int)(kfifo_len(&data->cmd_queue) / sizeof(struct command)));
		mutex_unlock(&data->fifo_lock);

		/* check lock	*/
		mutex_lock(&data->cmd_lock);
		data->cmd_is_running = true;
		mutex_unlock(&data->cmd_lock);

		data->cmd_state = SEC_CMD_STATUS_RUNNING;
		sec_cmd_store_function(data);

	} else {
		mutex_unlock(&data->fifo_lock);
	}
#endif
}

void sec_cmd_set_default_result(struct sec_cmd_data *data)
{
	char delim = ':';
	memset(data->cmd_result, 0x00, SEC_CMD_RESULT_STR_LEN);
	memcpy(data->cmd_result, data->cmd, SEC_CMD_STR_LEN);
	strncat(data->cmd_result, &delim, 1);
}

void sec_cmd_set_cmd_result(struct sec_cmd_data *data, char *buff, int len)
{
	strlcat(data->cmd_result, buff, SEC_CMD_RESULT_STR_LEN);
}

#ifndef USE_SEC_CMD_QUEUE
static ssize_t sec_cmd_store(struct device *dev,
		struct device_attribute *devattr, const char *buf, size_t count)
{
	struct sec_cmd_data *data = dev_get_drvdata(dev);
	char *cur, *start, *end;
	char buff[SEC_CMD_STR_LEN] = { 0 };
	int len, i;
	struct sec_cmd *sec_cmd_ptr = NULL;
	char delim = ',';
	bool cmd_found = false;
	int param_cnt = 0;

	if (!data) {
		pr_err("%s %s: No platform data found\n", SECLOG, __func__);
		return -EINVAL;
	}

	if (strlen(buf) >= SEC_CMD_STR_LEN) {		
		pr_err("%s %s: cmd length is over (%s,%d)!!\n", SECLOG, __func__, buf, (int)strlen(buf));
		return -EINVAL;
	}

	if (data->cmd_is_running == true) {
		pr_err("%s %s: other cmd is running.\n", SECLOG, __func__);
		return -EBUSY;
	}

	/* check lock   */
	mutex_lock(&data->cmd_lock);
	data->cmd_is_running = true;
	mutex_unlock(&data->cmd_lock);

	data->cmd_state = SEC_CMD_STATUS_RUNNING;
	for (i = 0; i < ARRAY_SIZE(data->cmd_param); i++)
		data->cmd_param[i] = 0;

	len = (int)count;
	if (*(buf + len - 1) == '\n')
		len--;

	memset(data->cmd, 0x00, ARRAY_SIZE(data->cmd));
	memcpy(data->cmd, buf, len);

	cur = strchr(buf, (int)delim);
	if (cur)
		memcpy(buff, buf, cur - buf);
	else
		memcpy(buff, buf, len);

	pr_debug("%s %s: COMMAND = %s\n", SECLOG, __func__, buff);

	/* find command */
	list_for_each_entry(sec_cmd_ptr, &data->cmd_list_head, list) {
		if (!strncmp(buff, sec_cmd_ptr->cmd_name, SEC_CMD_STR_LEN)) {
			cmd_found = true;
			break;
		}
	}

	/* set not_support_cmd */
	if (!cmd_found) {
		list_for_each_entry(sec_cmd_ptr, &data->cmd_list_head, list) {
			if (!strncmp("not_support_cmd", sec_cmd_ptr->cmd_name,
				SEC_CMD_STR_LEN))
				break;
		}
	}

	/* parsing parameters */
	if (cur && cmd_found) {
		cur++;
		start = cur;
		memset(buff, 0x00, ARRAY_SIZE(buff));

		do {
			if (*cur == delim || cur - buf == len) {
				end = cur;
				memcpy(buff, start, end - start);
				*(buff + strnlen(buff, ARRAY_SIZE(buff))) = '\0';
				if (kstrtoint(buff, 10, data->cmd_param + param_cnt) < 0)
					goto err_out;
				start = cur + 1;
				memset(buff, 0x00, ARRAY_SIZE(buff));
				param_cnt++;
			}
			cur++;
		} while ((cur - buf <= len) && (param_cnt < SEC_CMD_PARAM_NUM));
	}

	if (cmd_found) {
		pr_info("%s %s: cmd = %s", SECLOG, __func__, sec_cmd_ptr->cmd_name);
		for (i = 0; i < param_cnt; i++) {
			if (i == 0)
				pr_cont(" param =");
			pr_cont(" %d", data->cmd_param[i]);
		}
		pr_cont("\n");
	} else {
		pr_info("%s %s: cmd = %s(%s)\n", SECLOG, __func__, buff, sec_cmd_ptr->cmd_name);
	}

	sec_cmd_ptr->cmd_func(data);
	sec_cmd_set_cmd_exit(data);

err_out:
	return count;
}

#else	/* defined USE_SEC_CMD_QUEUE */
static void sec_cmd_store_function(struct sec_cmd_data *data)
{
	char *cur, *start, *end;
	char buff[SEC_CMD_STR_LEN] = { 0 };
	int len, i;
	struct sec_cmd *sec_cmd_ptr = NULL;
	char delim = ',';
	bool cmd_found = false;
	int param_cnt = 0;
	int ret;
	const char *buf;
	size_t count;
	struct command cmd = {{0}};

	if (!data) {
		pr_err("%s %s: No platform data found\n", SECLOG, __func__);
		return;
	}

	mutex_lock(&data->fifo_lock);
	if (kfifo_len(&data->cmd_queue)) {
		ret = kfifo_out(&data->cmd_queue, &cmd, sizeof(struct command));
		if (!ret) {
			pr_err("%s %s: kfifo_out failed, it seems empty, ret=%d\n", SECLOG, __func__, ret);
			mutex_unlock(&data->fifo_lock);
			return;
		}
	} else {
		pr_err("%s %s: left cmd is nothing\n", SECLOG, __func__);
		mutex_unlock(&data->fifo_lock);
		return;
	}
	mutex_unlock(&data->fifo_lock);

	buf = cmd.cmd;
	count = strlen(buf);

	for (i = 0; i < (int)ARRAY_SIZE(data->cmd_param); i++)
		data->cmd_param[i] = 0;

	len = (int)count;
	if (*(buf + len - 1) == '\n')
		len--;

	memset(data->cmd, 0x00, ARRAY_SIZE(data->cmd));
	memcpy(data->cmd, buf, len);

	cur = strchr(buf, (int)delim);
	if (cur)
		memcpy(buff, buf, cur - buf);
	else
		memcpy(buff, buf, len);

	pr_debug("%s %s: COMMAND : %s\n", SECLOG, __func__, buff);

	/* find command */
	list_for_each_entry(sec_cmd_ptr, &data->cmd_list_head, list) {
		if (!strncmp(buff, sec_cmd_ptr->cmd_name, SEC_CMD_STR_LEN)) {
			cmd_found = true;
			break;
		}
	}

	/* set not_support_cmd */
	if (!cmd_found) {
		list_for_each_entry(sec_cmd_ptr, &data->cmd_list_head, list) {
			if (!strncmp("not_support_cmd", sec_cmd_ptr->cmd_name,
				SEC_CMD_STR_LEN))
				break;
		}
	}

	/* parsing parameters */
	if (cur && cmd_found) {
		cur++;
		start = cur;
		memset(buff, 0x00, ARRAY_SIZE(buff));

		do {
			if (*cur == delim || cur - buf == len) {
				end = cur;
				memcpy(buff, start, end - start);
				*(buff + strnlen(buff, ARRAY_SIZE(buff))) = '\0';
				if (kstrtoint(buff, 10, data->cmd_param + param_cnt) < 0)
					return;
				start = cur + 1;
				memset(buff, 0x00, ARRAY_SIZE(buff));
				param_cnt++;
			}
			cur++;
		} while ((cur - buf <= len) && (param_cnt < SEC_CMD_PARAM_NUM));
	}

	if (cmd_found) {
		pr_info("%s %s: cmd = %s", SECLOG, __func__, sec_cmd_ptr->cmd_name);
		for (i = 0; i < param_cnt; i++) {
			if (i == 0)
				pr_cont(" param =");
			pr_cont(" %d", data->cmd_param[i]);
		}
		pr_cont("\n");
	} else {
		pr_info("%s %s: cmd = %s(%s)\n", SECLOG, __func__, buff, sec_cmd_ptr->cmd_name);
	}

	sec_cmd_ptr->cmd_func(data);

}

static ssize_t sec_cmd_store(struct device *dev, struct device_attribute *devattr,
			   const char *buf, size_t count)
{
	struct sec_cmd_data *data = dev_get_drvdata(dev);
	struct command cmd = {{0}};
	int queue_size;

	if (!data) {
		pr_err("%s %s: No platform data found\n", SECLOG, __func__);
		return -EINVAL;
	}

	if (strlen(buf) >= SEC_CMD_STR_LEN) {		
		pr_err("%s %s: cmd length is over (%s,%d)!!\n", SECLOG, __func__, buf, (int)strlen(buf));
		return -EINVAL;
	}

	strncpy(cmd.cmd, buf, count);

	mutex_lock(&data->fifo_lock);
	queue_size = (kfifo_len(&data->cmd_queue) / sizeof(struct command));

	if (kfifo_avail(&data->cmd_queue) && (queue_size < SEC_CMD_MAX_QUEUE)) {
		kfifo_in(&data->cmd_queue, &cmd, sizeof(struct command));
		pr_info("%s %s: push cmd: %s\n", SECLOG, __func__, cmd.cmd);
	} else {
		pr_err("%s %s: cmd_queue is full!!\n", SECLOG, __func__);

		kfifo_reset(&data->cmd_queue);
		pr_err("%s %s: cmd_queue is reset!!\n", SECLOG, __func__);
		mutex_unlock(&data->fifo_lock);

		mutex_lock(&data->cmd_lock);
		data->cmd_is_running = false;
		mutex_unlock(&data->cmd_lock);

		return -ENOSPC;
	}

	if (data->cmd_is_running == true) {
		pr_err("%s %s: other cmd is running. Wait until previous cmd is done[%d]\n",
			SECLOG, __func__, (int)(kfifo_len(&data->cmd_queue) / sizeof(struct command)));
		mutex_unlock(&data->fifo_lock);
		return -EBUSY;
	}
	mutex_unlock(&data->fifo_lock);

	/* check lock   */
	mutex_lock(&data->cmd_lock);
	data->cmd_is_running = true;
	mutex_unlock(&data->cmd_lock);

	data->cmd_state = SEC_CMD_STATUS_RUNNING;
	sec_cmd_store_function(data);

	return count;
}
#endif

static ssize_t sec_cmd_show_status(struct device *dev,
				 struct device_attribute *devattr, char *buf)
{
	struct sec_cmd_data *data = dev_get_drvdata(dev);
	char buff[16] = { 0 };

	if (!data) {
		pr_err("%s %s: No platform data found\n", SECLOG, __func__);
		return -EINVAL;
	}

	if (data->cmd_state == SEC_CMD_STATUS_WAITING)
		snprintf(buff, sizeof(buff), "WAITING");

	else if (data->cmd_state == SEC_CMD_STATUS_RUNNING)
		snprintf(buff, sizeof(buff), "RUNNING");

	else if (data->cmd_state == SEC_CMD_STATUS_OK)
		snprintf(buff, sizeof(buff), "OK");

	else if (data->cmd_state == SEC_CMD_STATUS_FAIL)
		snprintf(buff, sizeof(buff), "FAIL");

	else if (data->cmd_state == SEC_CMD_STATUS_NOT_APPLICABLE)
		snprintf(buff, sizeof(buff), "NOT_APPLICABLE");

	pr_debug("%s %s: %d, %s\n", SECLOG, __func__, data->cmd_state, buff);

	return snprintf(buf, SEC_CMD_BUF_SIZE, "%s\n", buff);
}

static ssize_t sec_cmd_show_result(struct device *dev,
				 struct device_attribute *devattr, char *buf)
{
	struct sec_cmd_data *data = dev_get_drvdata(dev);
	int size;

	if (!data) {
		pr_err("%s %s: No platform data found\n", SECLOG, __func__);
		return -EINVAL;
	}

	data->cmd_state = SEC_CMD_STATUS_WAITING;
	pr_info("%s %s: %s\n", SECLOG, __func__, data->cmd_result);
	size = snprintf(buf, SEC_CMD_RESULT_STR_LEN, "%s\n", data->cmd_result);

	sec_cmd_set_cmd_exit(data);

	return size;
}

static ssize_t sec_cmd_list_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct sec_cmd_data *data = dev_get_drvdata(dev);
	struct sec_cmd *sec_cmd_ptr = NULL;
	char *buffer = kmalloc_array(data->cmd_buffer_size + 30, sizeof(*buffer), GFP_KERNEL);
	char buffer_name[SEC_CMD_STR_LEN];
	ssize_t rc;

	snprintf(buffer, 30, "++factory command list++\n");

	list_for_each_entry(sec_cmd_ptr, &data->cmd_list_head, list) {
		if (strncmp(sec_cmd_ptr->cmd_name, "not_support_cmd", 15)) {
			snprintf(buffer_name, SEC_CMD_STR_LEN, "%s\n", sec_cmd_ptr->cmd_name);
			strncat(buffer, buffer_name, SEC_CMD_STR_LEN);
		}
	}

	rc = snprintf(buf, SEC_CMD_BUF_SIZE, "%s\n", buffer);
	kfree(buffer);

	return rc;
}

static DEVICE_ATTR(cmd, S_IWUSR | S_IWGRP, NULL, sec_cmd_store);
static DEVICE_ATTR(cmd_status, S_IRUGO, sec_cmd_show_status, NULL);
static DEVICE_ATTR(cmd_result, S_IRUGO, sec_cmd_show_result, NULL);
static DEVICE_ATTR(cmd_list, S_IRUGO, sec_cmd_list_show, NULL);

static struct attribute *sec_fac_attrs[] = {
	&dev_attr_cmd.attr,
	&dev_attr_cmd_status.attr,
	&dev_attr_cmd_result.attr,
	&dev_attr_cmd_list.attr,
	NULL,
};

static struct attribute_group sec_fac_attr_group = {
	.attrs = sec_fac_attrs,
};


int sec_cmd_init(struct sec_cmd_data *data, struct sec_cmd *cmds,
			int len, int devt)
{
	struct class *sec_class;
	struct sec_ts_data *ts = container_of(data, struct sec_ts_data, sec);
	const char *dev_name;
	int ret, i;

	INIT_LIST_HEAD(&data->cmd_list_head);

	data->cmd_buffer_size = 0;
	for (i = 0; i < len; i++) {
		list_add_tail(&cmds[i].list, &data->cmd_list_head);
		if (cmds[i].cmd_name)
			data->cmd_buffer_size += strlen(cmds[i].cmd_name) + 1;
	}

	mutex_init(&data->cmd_lock);

	mutex_lock(&data->cmd_lock);
	data->cmd_is_running = false;
	mutex_unlock(&data->cmd_lock);

#ifdef USE_SEC_CMD_QUEUE
	if (kfifo_alloc(&data->cmd_queue,
		SEC_CMD_MAX_QUEUE * sizeof(struct command), GFP_KERNEL)) {
		pr_err("%s %s: failed to alloc queue for cmd\n", SECLOG, __func__);
		goto err_alloc_queue;
	}
	mutex_init(&data->fifo_lock);
#endif

	if (devt == SEC_CLASS_DEVT_TSP) {
		dev_name = SEC_CLASS_DEV_NAME_TSP;

	} else if (devt == SEC_CLASS_DEVT_TKEY) {
		dev_name = SEC_CLASS_DEV_NAME_TKEY;

	} else if (devt == SEC_CLASS_DEVT_WACOM) {
		dev_name = SEC_CLASS_DEV_NAME_WACOM;

	} else {
		pr_err("%s %s: not defined devt=%d\n", SECLOG, __func__, devt);
		goto err_get_dev_name;
	}

#ifdef CONFIG_SEC_SYSFS
	data->fac_dev = sec_device_create(data, dev_name);
#else
	data->fac_dev = device_create(ts->sec_class, NULL, devt, data, dev_name);
#endif
	if (IS_ERR(data->fac_dev)) {
		pr_err("%s %s: failed to create device for the sysfs\n", SECLOG, __func__);
		goto err_sysfs_device;
	}

	dev_set_drvdata(data->fac_dev, data);

	ret = sysfs_create_group(&data->fac_dev->kobj, &sec_fac_attr_group);
	if (ret < 0) {
		pr_err("%s %s: failed to create sysfs group\n", SECLOG, __func__);
		goto err_sysfs_group;
	}

	return 0;

	sysfs_remove_group(&data->fac_dev->kobj, &sec_fac_attr_group);
err_sysfs_group:
#ifdef CONFIG_SEC_SYSFS
	sec_device_destroy(data->fac_dev->devt);
#else
	device_destroy(sec_class, devt);
#endif
err_sysfs_device:
err_get_dev_name:
#ifdef USE_SEC_CMD_QUEUE
	mutex_destroy(&data->fifo_lock);
	kfifo_free(&data->cmd_queue);
err_alloc_queue:
#endif
	mutex_destroy(&data->cmd_lock);
	list_del(&data->cmd_list_head);
	return -ENODEV;
}

void sec_cmd_exit(struct sec_cmd_data *data, int devt)
{
#ifdef USE_SEC_CMD_QUEUE
	struct command cmd = {{0}};
	int ret;
#endif

	pr_info("%s %s", SECLOG, __func__);
	sysfs_remove_group(&data->fac_dev->kobj, &sec_fac_attr_group);
	dev_set_drvdata(data->fac_dev, NULL);
#ifdef CONFIG_SEC_SYSFS
	sec_device_destroy(data->fac_dev->devt);
#else
	device_destroy(sec_class, devt);
#endif
#ifdef USE_SEC_CMD_QUEUE
	mutex_lock(&data->fifo_lock);
	while (kfifo_len(&data->cmd_queue)) {
		ret = kfifo_out(&data->cmd_queue, &cmd, sizeof(struct command));
		if (!ret) {
			pr_err("%s %s: kfifo_out failed, it seems empty, ret=%d\n", SECLOG, __func__, ret);
		}
		pr_info("%s %s: remove pending commands: %s", SECLOG, __func__, cmd.cmd);
	}
	mutex_unlock(&data->fifo_lock);
	mutex_destroy(&data->fifo_lock);
	kfifo_free(&data->cmd_queue);
#endif
	mutex_destroy(&data->cmd_lock);
	list_del(&data->cmd_list_head);
}

MODULE_DESCRIPTION("Samsung factory command");
MODULE_LICENSE("GPL");


