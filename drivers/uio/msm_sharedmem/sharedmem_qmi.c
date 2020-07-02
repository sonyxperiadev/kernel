/* Copyright (c) 2014-2015, 2017, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#define DRIVER_NAME "msm_sharedmem"
#define pr_fmt(fmt) DRIVER_NAME ": %s: " fmt, __func__

#include <linux/err.h>
#include <linux/module.h>
#include <linux/rwsem.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/debugfs.h>
#include <linux/soc/qcom/qmi.h>
#include <linux/ipa_qmi_service_v01.h>
#include "sharedmem_qmi.h"
#include "remote_filesystem_access_v01.h"

#define RFSA_SERVICE_INSTANCE_NUM 1
#define SHARED_ADDR_ENTRY_NAME_MAX_LEN 10

struct shared_addr_entry {
	u32 id;
	u64 address;
	u32 size;
	u64 request_count;
	bool is_addr_dynamic;
	char name[SHARED_ADDR_ENTRY_NAME_MAX_LEN + 1];
};

struct shared_addr_list {
	struct list_head node;
	struct shared_addr_entry entry;
};

static struct shared_addr_list list;

static struct qmi_handle *sharedmem_qmi_svc_handle;
static struct sockaddr_qrtr *client_sq;
static struct dentry *dir_ent;

static u32 rfsa_count;
static u32 rmts_count;

static DECLARE_RWSEM(sharedmem_list_lock); /* declare list lock semaphore */

static void sharedmem_qmi_get_buffer(struct qmi_handle *qmi_handle,
		struct sockaddr_qrtr *sq, struct qmi_txn *txn,
		const void *decoded_msg);

static struct work_struct sharedmem_qmi_init_work;

static struct qmi_msg_handler rfsa_get_buffer_addr_handlers = {
	.type = QMI_REQUEST,
	.msg_id = QMI_RFSA_GET_BUFF_ADDR_REQ_MSG_V01,
	.ei = rfsa_get_buff_addr_req_msg_v01_ei,
	.decoded_size = RFSA_GET_BUFF_ADDR_REQ_MSG_MAX_LEN_V01,
	.fn = sharedmem_qmi_get_buffer,
};

void sharedmem_qmi_add_entry(struct sharemem_qmi_entry *qmi_entry)
{
	struct shared_addr_list *list_entry;

	list_entry = kzalloc(sizeof(*list_entry), GFP_KERNEL);

	/* If we cannot add the entry log the failure and bail */
	if (list_entry == NULL) {
		pr_err("Alloc of new list entry failed\n");
		return;
	}

	/* Copy as much of the client name that can fit in the entry. */
	strlcpy(list_entry->entry.name, qmi_entry->client_name,
		sizeof(list_entry->entry.name));

	/* Setup the rest of the entry. */
	list_entry->entry.id = qmi_entry->client_id;
	list_entry->entry.address = qmi_entry->address;
	list_entry->entry.size = qmi_entry->size;
	list_entry->entry.is_addr_dynamic = qmi_entry->is_addr_dynamic;
	list_entry->entry.request_count = 0;

	down_write(&sharedmem_list_lock);
	list_add_tail(&(list_entry->node), &(list.node));
	up_write(&sharedmem_list_lock);
	pr_debug("Added new entry to list\n");

}

static int get_buffer_for_client(u32 id, u32 size, u64 *address)
{
	int result = -ENOENT;
	int client_found = 0;
	struct list_head *curr_node;
	struct shared_addr_list *list_entry;

	if (size == 0)
		return -ENOMEM;

	down_read(&sharedmem_list_lock);

	list_for_each(curr_node, &list.node) {
		list_entry = list_entry(curr_node, struct shared_addr_list,
					node);
		if (list_entry->entry.id == id) {
			if (list_entry->entry.size >= size) {
				*address = list_entry->entry.address;
				list_entry->entry.request_count++;
				result = 0;
			} else {
				pr_err("Shared mem req too large for id=%u\n",
					id);
				result = -ENOMEM;
			}
			client_found = 1;
			break;
		}
	}

	up_read(&sharedmem_list_lock);

	if (client_found != 1) {
		pr_err("Unknown client id %u\n", id);
		result = -ENOENT;
	}
	return result;
}

static void sharedmem_qmi_get_buffer(struct qmi_handle *qmi_handle,
		struct sockaddr_qrtr *sq, struct qmi_txn *txn,
		const void *decoded_msg)
{
	struct rfsa_get_buff_addr_req_msg_v01 *get_buffer_req;
	struct rfsa_get_buff_addr_resp_msg_v01 get_buffer_resp = {{0}, 0};
	int result;
	u64 address = 0;

	get_buffer_req = (struct rfsa_get_buff_addr_req_msg_v01 *)decoded_msg;
	pr_debug("req->client_id = 0x%X and req->size = %d\n",
		get_buffer_req->client_id, get_buffer_req->size);

	result = get_buffer_for_client(get_buffer_req->client_id,
					get_buffer_req->size, &address);
	if (result != 0) {
		pr_err("%s: Cannot get buffer!!\n", __func__);
		return;
	}

	if (address == 0) {
		pr_err("%s: Entry found for client id=0x%X but "
			"address is zero\n", __func__,
			get_buffer_req->client_id);
		return;
	}

	client_sq = sq;
	memset(&get_buffer_resp, 0, sizeof(get_buffer_resp));
	get_buffer_resp.address_valid = 1;
	get_buffer_resp.address = address;
	get_buffer_resp.resp.result = QMI_RESULT_SUCCESS_V01;

	result = qmi_send_response(sharedmem_qmi_svc_handle, sq, txn,
				QMI_RFSA_GET_BUFF_ADDR_RESP_MSG_V01,
				RFSA_GET_BUFF_ADDR_RESP_MSG_MAX_LEN_V01,
				rfsa_get_buff_addr_resp_msg_v01_ei,
				&get_buffer_resp);

	return;
}


static void sharedmem_qmi_disconnect_cb(struct qmi_handle *handle,
				unsigned int node, unsigned int port)
{
	/* Disconnect not called for this service */
	if (sharedmem_qmi_svc_handle != handle)
		return;

	/* Client is not connected */
	if (!client_sq)
		return;

	/* It's calling us! Let's take action. */
	if (client_sq->sq_node == node && client_sq->sq_port == port) {
		pr_err("%s: Called disconnect for Shared Memory QMI!"
		       " FIXME: This should not happen!!!\n", __func__);
		return;
	}
}

#define DEBUG_BUF_SIZE (2048)
static char *debug_buffer;
static u32 debug_data_size;
static struct mutex dbg_buf_lock;	/* mutex for debug_buffer */

static ssize_t debug_read(struct file *file, char __user *buf,
			  size_t count, loff_t *file_pos)
{
	return simple_read_from_buffer(buf, count, file_pos, debug_buffer,
					debug_data_size);
}

static u32 fill_debug_info(char *buffer, u32 buffer_size)
{
	u32 size = 0;
	struct list_head *curr_node;
	struct shared_addr_list *list_entry;

	memset(buffer, 0, buffer_size);
	size += scnprintf(buffer + size, buffer_size - size, "\n");

	down_read(&sharedmem_list_lock);
	list_for_each(curr_node, &list.node) {
		list_entry = list_entry(curr_node, struct shared_addr_list,
					node);
		size += scnprintf(buffer + size, buffer_size - size,
				"Client_name: %s\n", list_entry->entry.name);
		size += scnprintf(buffer + size, buffer_size - size,
				"Client_id: 0x%08X\n", list_entry->entry.id);
		size += scnprintf(buffer + size, buffer_size - size,
				"Buffer Size: 0x%08X (%d)\n",
				list_entry->entry.size,
				list_entry->entry.size);
		size += scnprintf(buffer + size, buffer_size - size,
				"Address: 0x%016llX\n",
				list_entry->entry.address);
		size += scnprintf(buffer + size, buffer_size - size,
				"Address Allocation: %s\n",
				(list_entry->entry.is_addr_dynamic ?
				"Dynamic" : "Static"));
		size += scnprintf(buffer + size, buffer_size - size,
				"Request count: %llu\n",
				list_entry->entry.request_count);
		size += scnprintf(buffer + size, buffer_size - size, "\n\n");
	}
	up_read(&sharedmem_list_lock);

	size += scnprintf(buffer + size, buffer_size - size,
			"RFSA server start count = %u\n", rfsa_count);
	size += scnprintf(buffer + size, buffer_size - size,
			"RMTS server start count = %u\n", rmts_count);

	size += scnprintf(buffer + size, buffer_size - size, "\n");
	return size;
}

static int debug_open(struct inode *inode, struct file *file)
{
	u32 buffer_size;

	mutex_lock(&dbg_buf_lock);
	if (debug_buffer != NULL) {
		mutex_unlock(&dbg_buf_lock);
		return -EBUSY;
	}
	buffer_size = DEBUG_BUF_SIZE;
	debug_buffer = kzalloc(buffer_size, GFP_KERNEL);
	if (debug_buffer == NULL) {
		mutex_unlock(&dbg_buf_lock);
		return -ENOMEM;
	}
	debug_data_size = fill_debug_info(debug_buffer, buffer_size);
	mutex_unlock(&dbg_buf_lock);
	return 0;
}

static int debug_close(struct inode *inode, struct file *file)
{
	mutex_lock(&dbg_buf_lock);
	kfree(debug_buffer);
	debug_buffer = NULL;
	debug_data_size = 0;
	mutex_unlock(&dbg_buf_lock);
	return 0;
}

static const struct file_operations debug_ops = {
	.read = debug_read,
	.open = debug_open,
	.release = debug_close,
};

static int rfsa_increment(void *data, u64 val)
{
	if (rfsa_count != ~0)
		rfsa_count++;
	return 0;
}

static int rmts_increment(void *data, u64 val)
{
	if (rmts_count != ~0)
		rmts_count++;
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(rfsa_fops, NULL, rfsa_increment, "%llu\n");
DEFINE_SIMPLE_ATTRIBUTE(rmts_fops, NULL, rmts_increment, "%llu\n");

static void debugfs_init(void)
{
	struct dentry *f_ent;

	mutex_init(&dbg_buf_lock);
	dir_ent = debugfs_create_dir("rmt_storage", NULL);
	if (IS_ERR(dir_ent)) {
		pr_err("Failed to create debug_fs directory\n");
		return;
	}

	f_ent = debugfs_create_file("info", 0400, dir_ent, NULL, &debug_ops);
	if (IS_ERR(f_ent)) {
		pr_err("Failed to create debug_fs info file\n");
		return;
	}

	f_ent = debugfs_create_file("rfsa", 0200, dir_ent, NULL, &rfsa_fops);
	if (IS_ERR(f_ent)) {
		pr_err("Failed to create debug_fs rfsa file\n");
		return;
	}

	f_ent = debugfs_create_file("rmts", 0200, dir_ent, NULL, &rmts_fops);
	if (IS_ERR(f_ent)) {
		pr_err("Failed to create debug_fs rmts file\n");
		return;
	}
}

static void debugfs_exit(void)
{
	debugfs_remove_recursive(dir_ent);
	mutex_destroy(&dbg_buf_lock);
}

static struct qmi_ops sharedmem_qmi_ops_options = {
	.del_client = sharedmem_qmi_disconnect_cb,
};

static int sharedmem_register_qmi(void)
{
	int rc;

	sharedmem_qmi_svc_handle = kzalloc(sizeof(*sharedmem_qmi_svc_handle),
					   GFP_KERNEL);
	if (!sharedmem_qmi_svc_handle)
		return -ENOMEM;

	rc = qmi_handle_init(sharedmem_qmi_svc_handle,
			     RFSA_GET_BUFF_ADDR_RESP_MSG_MAX_LEN_V01,
			     &sharedmem_qmi_ops_options,
			     &rfsa_get_buffer_addr_handlers);
	if (rc < 0) {
		pr_err("%s: Cannot init QMI handle for rfsa service\n",
			__func__);
		goto free_svc_handle;
	}

	rc = qmi_add_server(sharedmem_qmi_svc_handle, RFSA_SERVICE_ID_V01,
			    RFSA_SERVICE_VERS_V01, 0);
	if (rc < 0) {
		pr_err("%s: Cannot add shared memory svc server\n", __func__);
		goto release_service;
	}

	pr_info("MSM Shared Memory QMI Server initialized\n");

	return 0;

release_service:
	qmi_handle_release(sharedmem_qmi_svc_handle);
free_svc_handle:
	kfree(sharedmem_qmi_svc_handle);

	return rc;
}

static void sharedmem_qmi_init_worker(struct work_struct *work)
{
	sharedmem_register_qmi();
	debugfs_init();
}

int sharedmem_qmi_init(void)
{
	INIT_LIST_HEAD(&list.node);
	INIT_WORK(&sharedmem_qmi_init_work, sharedmem_qmi_init_worker);
	schedule_work(&sharedmem_qmi_init_work);
	return 0;
}

void sharedmem_qmi_exit(void)
{
	qmi_handle_release(sharedmem_qmi_svc_handle);
	kfree(sharedmem_qmi_svc_handle);
	debugfs_exit();
}
