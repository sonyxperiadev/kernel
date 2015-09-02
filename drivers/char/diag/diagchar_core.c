/* Copyright (c) 2008-2015, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/slab.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/diagchar.h>
#include <linux/platform_device.h>
#include <linux/sched.h>
#include <linux/ratelimit.h>
#include <linux/timer.h>
#ifdef CONFIG_DIAG_OVER_USB
#include <linux/usb/usbdiag.h>
#endif
#include <asm/current.h>
#include "diagchar_hdlc.h"
#include "diagmem.h"
#include "diagchar.h"
#include "diagfwd.h"
#include "diagfwd_cntl.h"
#include "diag_dci.h"
#include "diag_debugfs.h"
#include "diag_masks.h"
#include "diagfwd_bridge.h"
#include "diag_usb.h"
#include "diag_memorydevice.h"
#include "diag_mux.h"

#include <linux/coresight-stm.h>
#include <linux/kernel.h>
#ifdef CONFIG_COMPAT
#include <linux/compat.h>
#endif

MODULE_DESCRIPTION("Diag Char Driver");
MODULE_LICENSE("GPL v2");
MODULE_VERSION("1.0");

#define MIN_SIZ_ALLOW 4
#define INIT	1
#define EXIT	-1
struct diagchar_dev *driver;
struct diagchar_priv {
	int pid;
};

/* Memory pool variables */
/* Used for copying any incoming packet from user space clients. */
static unsigned int itemsize = 4096;
static unsigned int poolsize = 12;
module_param(itemsize, uint, 0);
module_param(poolsize, uint, 0);

/*
 * Used for HDLC encoding packets coming from the user
 * space. Itemsize is defined in diagchar_init. HDLC
 * Buffer can be at most twice as long as the itemsize.
 * Add 3 bytes for CRC bytes and delimiter. Don't expose
 * itemsize_hdlc as it is dependent on itemsize.By
 * default it is set to DIAG_HDLC_BUF_SIZE (8K)
 */
static unsigned int itemsize_hdlc = DIAG_HDLC_BUF_SIZE;
static unsigned int poolsize_hdlc = 10;
module_param(poolsize_hdlc, uint, 0);

/*
 * This is used for incoming DCI requests from the user space clients.
 * Don't expose itemsize as it is internal.
 */
static unsigned int itemsize_user = 8192;
static unsigned int poolsize_user = 8;
module_param(poolsize_user, uint, 0);

/*
 * USB structures allocated for writing Diag data generated on the Apps to USB.
 * Don't expose itemsize as it is constant.
 */
static unsigned int itemsize_usb_apps = sizeof(struct diag_request);
static unsigned int poolsize_usb_apps = 10;
module_param(poolsize_usb_apps, uint, 0);

/* Used for DCI client buffers. Don't expose itemsize as it is constant. */
static unsigned int itemsize_dci = IN_BUF_SIZE;
static unsigned int poolsize_dci = 10;
module_param(poolsize_dci, uint, 0);

#ifdef CONFIG_DIAGFWD_BRIDGE_CODE
/* Used for reading data from the remote device. */
static unsigned int itemsize_mdm = DIAG_MDM_BUF_SIZE;
static unsigned int poolsize_mdm = 9;
module_param(itemsize_mdm, uint, 0);
module_param(poolsize_mdm, uint, 0);

/*
 * Used for reading DCI data from the remote device.
 * Don't expose poolsize for DCI data. There is only one read buffer
 */
static unsigned int itemsize_mdm_dci = DIAG_MDM_BUF_SIZE;
static unsigned int poolsize_mdm_dci = 1;
module_param(itemsize_mdm_dci, uint, 0);

/*
 * Used for USB structues associated with a remote device.
 * Don't expose the itemsize since it is constant.
 */
static unsigned int itemsize_mdm_usb = sizeof(struct diag_request);
static unsigned int poolsize_mdm_usb = 9;
module_param(poolsize_mdm_usb, uint, 0);

/*
 * Used for writing read DCI data to remote peripherals. Don't
 * expose poolsize for DCI data. There is only one read
 * buffer. Add 6 bytes for DCI header information: Start (1),
 * Version (1), Length (2), Tag (2)
 */
static unsigned int itemsize_mdm_dci_write = DIAG_MDM_DCI_BUF_SIZE;
static unsigned int poolsize_mdm_dci_write = 1;
module_param(itemsize_mdm_dci_write, uint, 0);

/*
 * Used for USB structures associated with a remote SMUX
 * device Don't expose the itemsize since it is constant
 */
static unsigned int itemsize_qsc_usb = sizeof(struct diag_request);
static unsigned int poolsize_qsc_usb = 8;
module_param(poolsize_qsc_usb, uint, 0);
#endif

/* This is the max number of user-space clients supported at initialization*/
static unsigned int max_clients = 15;
static unsigned int threshold_client_limit = 30;
module_param(max_clients, uint, 0);

/* This is the maximum number of pkt registrations supported at initialization*/
int diag_max_reg = 600;
int diag_threshold_reg = 750;

/* Timer variables */
static struct timer_list drain_timer;
static int timer_in_progress;
void *buf_hdlc;
static int buf_hdlc_ctxt;

#define DIAGPKT_MAX_DELAYED_RSP 0xFFFF

/*
 * Returns the next delayed rsp id. If wrapping is enabled,
 * wraps the delayed rsp id to DIAGPKT_MAX_DELAYED_RSP.
 */
static uint16_t diag_get_next_delayed_rsp_id(void)
{
	uint16_t rsp_id = 0;

	mutex_lock(&driver->delayed_rsp_mutex);
	rsp_id = driver->delayed_rsp_id;
	if (rsp_id < DIAGPKT_MAX_DELAYED_RSP)
		rsp_id++;
	else {
		if (wrap_enabled) {
			rsp_id = 1;
			wrap_count++;
		} else
			rsp_id = DIAGPKT_MAX_DELAYED_RSP;
	}
	driver->delayed_rsp_id = rsp_id;
	mutex_unlock(&driver->delayed_rsp_mutex);

	return rsp_id;
}

static int diag_switch_logging(int requested_mode);

#define COPY_USER_SPACE_OR_EXIT(buf, data, length)		\
do {								\
	if ((count < ret+length) || (copy_to_user(buf,		\
			(void *)&data, length))) {		\
		ret = -EFAULT;					\
		goto exit;					\
	}							\
	ret += length;						\
} while (0)

static void drain_timer_func(unsigned long data)
{
	queue_work(driver->diag_wq , &(driver->diag_drain_work));
}

void diag_drain_work_fn(struct work_struct *work)
{
	int err = 0;
	timer_in_progress = 0;

	mutex_lock(&driver->diagchar_mutex);
	if (buf_hdlc) {
		err = diag_mux_write(DIAG_LOCAL_PROC, buf_hdlc, driver->used,
				     buf_hdlc_ctxt);
		if (err)
			diagmem_free(driver, buf_hdlc, POOL_TYPE_HDLC);
		buf_hdlc = NULL;
#ifdef DIAG_DEBUG
		pr_debug("diag: Number of bytes written "
				 "from timer is %d ", driver->used);
#endif
		driver->used = 0;
	}

	mutex_unlock(&driver->diagchar_mutex);

}

void check_drain_timer(void)
{
	int ret = 0;

	if (!timer_in_progress) {
		timer_in_progress = 1;
		ret = mod_timer(&drain_timer, jiffies + msecs_to_jiffies(200));
	}
}

void diag_add_client(int i, struct file *file)
{
	struct diagchar_priv *diagpriv_data;

	driver->client_map[i].pid = current->tgid;
	diagpriv_data = kmalloc(sizeof(struct diagchar_priv),
							GFP_KERNEL);
	if (diagpriv_data)
		diagpriv_data->pid = current->tgid;
	file->private_data = diagpriv_data;
	strlcpy(driver->client_map[i].name, current->comm, 20);
	driver->client_map[i].name[19] = '\0';
}

static void diag_mempool_init(void)
{
	diagmem_init(driver, POOL_TYPE_COPY);
	diagmem_init(driver, POOL_TYPE_HDLC);
	diagmem_init(driver, POOL_TYPE_USER);
	diagmem_init(driver, POOL_TYPE_DCI);
}

static void diag_mempool_exit(void)
{
	diagmem_exit(driver, POOL_TYPE_COPY);
	diagmem_exit(driver, POOL_TYPE_HDLC);
	diagmem_exit(driver, POOL_TYPE_USER);
	diagmem_exit(driver, POOL_TYPE_DCI);
}

static int diagchar_open(struct inode *inode, struct file *file)
{
	int i = 0;
	void *temp;

	if (driver) {
		mutex_lock(&driver->diagchar_mutex);

		for (i = 0; i < driver->num_clients; i++)
			if (driver->client_map[i].pid == 0)
				break;

		if (i < driver->num_clients) {
			diag_add_client(i, file);
		} else {
			if (i < threshold_client_limit) {
				driver->num_clients++;
				temp = krealloc(driver->client_map
					, (driver->num_clients) * sizeof(struct
						 diag_client_map), GFP_KERNEL);
				if (!temp)
					goto fail;
				else
					driver->client_map = temp;
				temp = krealloc(driver->data_ready
					, (driver->num_clients) * sizeof(int),
							GFP_KERNEL);
				if (!temp)
					goto fail;
				else
					driver->data_ready = temp;
				diag_add_client(i, file);
			} else {
				mutex_unlock(&driver->diagchar_mutex);
				pr_alert("Max client limit for DIAG reached\n");
				pr_info("Cannot open handle %s"
					   " %d", current->comm, current->tgid);
				for (i = 0; i < driver->num_clients; i++)
					pr_debug("%d) %s PID=%d", i, driver->
						client_map[i].name,
						driver->client_map[i].pid);
				return -ENOMEM;
			}
		}
		driver->data_ready[i] = 0x0;
		driver->data_ready[i] |= MSG_MASKS_TYPE;
		driver->data_ready[i] |= EVENT_MASKS_TYPE;
		driver->data_ready[i] |= LOG_MASKS_TYPE;
		driver->data_ready[i] |= DCI_LOG_MASKS_TYPE;
		driver->data_ready[i] |= DCI_EVENT_MASKS_TYPE;

		if (driver->ref_count == 0)
			diag_mempool_init();
		driver->ref_count++;
		mutex_unlock(&driver->diagchar_mutex);
		return 0;
	}
	return -ENOMEM;

fail:
	mutex_unlock(&driver->diagchar_mutex);
	driver->num_clients--;
	pr_alert("diag: Insufficient memory for new client");
	return -ENOMEM;
}

static int diagchar_close(struct inode *inode, struct file *file)
{
	int i = -1;
	struct diagchar_priv *diagpriv_data = file->private_data;
	struct diag_dci_client_tbl *dci_entry = NULL;
	unsigned long flags;

	pr_debug("diag: process exit %s\n", current->comm);
	if (!(file->private_data)) {
		pr_alert("diag: Invalid file pointer");
		return -ENOMEM;
	}

	if (!driver)
		return -ENOMEM;

	/* clean up any DCI registrations, if this is a DCI client
	* This will specially help in case of ungraceful exit of any DCI client
	* This call will remove any pending registrations of such client
	*/
	dci_entry = dci_lookup_client_entry_pid(current->pid);
	if (dci_entry)
		diag_dci_deinit_client(dci_entry);
	/* If the exiting process is the socket process */
	mutex_lock(&driver->diagchar_mutex);
	if (driver->socket_process &&
		(driver->socket_process->tgid == current->tgid)) {
		driver->socket_process = NULL;
		diag_update_proc_vote(DIAG_PROC_MEMORY_DEVICE, VOTE_DOWN,
				      ALL_PROC);
	}
	if (driver->callback_process &&
		(driver->callback_process->tgid == current->tgid)) {
		driver->callback_process = NULL;
		diag_update_proc_vote(DIAG_PROC_MEMORY_DEVICE, VOTE_DOWN,
				      ALL_PROC);
	}
	mutex_unlock(&driver->diagchar_mutex);

#ifdef CONFIG_DIAG_OVER_USB
	/* If the SD logging process exits, change logging to USB mode */
	if (driver->logging_process_id == current->tgid) {
		if (driver->rsp_buf_busy) {
			/*
			 * This happens when the logging process did not get a
			 * chance to read the last response. Clear the busy flag
			 * for the response buffer.
			 */
			spin_lock_irqsave(&driver->rsp_buf_busy_lock, flags);
			driver->rsp_buf_busy = 0;
			spin_unlock_irqrestore(&driver->rsp_buf_busy_lock,
					       flags);
			pr_debug("diag: In %s, Resetting rsp_buf_busy explicitly due to pid: %d\n",
				 __func__, current->tgid);
		}
		diag_update_proc_vote(DIAG_PROC_MEMORY_DEVICE, VOTE_DOWN,
				      ALL_PROC);
		diag_switch_logging(USB_MODE);
	}
#endif /* DIAG over USB */
	/* Delete the pkt response table entry for the exiting process */
	for (i = 0; i < diag_max_reg; i++)
			if (driver->table[i].process_id == current->tgid)
					driver->table[i].process_id = 0;

	mutex_lock(&driver->diagchar_mutex);
	driver->ref_count--;
	if (driver->ref_count == 0)
		diag_mempool_exit();

	for (i = 0; i < driver->num_clients; i++) {
		if (NULL != diagpriv_data && diagpriv_data->pid ==
						driver->client_map[i].pid) {
			driver->client_map[i].pid = 0;
			kfree(diagpriv_data);
			diagpriv_data = NULL;
			break;
		}
	}
	mutex_unlock(&driver->diagchar_mutex);
	return 0;
}

int diag_find_polling_reg(int i)
{
	uint16_t subsys_id, cmd_code_lo, cmd_code_hi;

	subsys_id = driver->table[i].subsys_id;
	cmd_code_lo = driver->table[i].cmd_code_lo;
	cmd_code_hi = driver->table[i].cmd_code_hi;

	if (driver->table[i].cmd_code == 0xFF) {
		if (subsys_id == 0xFF && cmd_code_hi >= 0x0C &&
			 cmd_code_lo <= 0x0C)
			return 1;
		if (subsys_id == 0x04 && cmd_code_hi >= 0x0E &&
			 cmd_code_lo <= 0x0E)
			return 1;
		else if (subsys_id == 0x08 && cmd_code_hi >= 0x02 &&
			 cmd_code_lo <= 0x02)
			return 1;
		else if (subsys_id == 0x32 && cmd_code_hi >= 0x03  &&
			 cmd_code_lo <= 0x03)
			return 1;
		else if (subsys_id == 0x57 && cmd_code_hi >= 0x0E &&
			 cmd_code_lo <= 0x0E)
			return 1;
	}
	return 0;
}

void diag_clear_reg(int peripheral)
{
	int i;

	mutex_lock(&driver->diagchar_mutex);
	/* reset polling flag */
	driver->polling_reg_flag = 0;
	for (i = 0; i < diag_max_reg; i++) {
		if (driver->table[i].client_id == peripheral)
			driver->table[i].process_id = 0;
	}
	/* re-scan the registration table */
	for (i = 0; i < diag_max_reg; i++) {
		if (driver->table[i].process_id != 0 &&
				diag_find_polling_reg(i) == 1) {
			driver->polling_reg_flag = 1;
			break;
		}
	}
	mutex_unlock(&driver->diagchar_mutex);
}

int diag_add_reg(int j, struct bindpkt_params *params,
				  unsigned int *count_entries)
{
	if (j < 0 || j >= diag_max_reg || !params || !count_entries)
		return -EINVAL;

	driver->table[j].cmd_code = params->cmd_code;
	driver->table[j].subsys_id = params->subsys_id;
	driver->table[j].cmd_code_lo = params->cmd_code_lo;
	driver->table[j].cmd_code_hi = params->cmd_code_hi;

	/* check if incoming reg is polling & polling is yet not registered */
	if (driver->polling_reg_flag == 0)
		if (diag_find_polling_reg(j) == 1)
			driver->polling_reg_flag = 1;
	if (params->proc_id == APPS_PROC) {
		driver->table[j].process_id = current->tgid;
		driver->table[j].client_id = APPS_DATA;
	} else {
		driver->table[j].process_id = NON_APPS_PROC;
		driver->table[j].client_id = params->client_id;
	}
	(*count_entries)++;

	return 1;
}

void diag_get_timestamp(char *time_str)
{
	struct timeval t;
	struct tm broken_tm;
	do_gettimeofday(&t);
	if (!time_str)
		return;
	time_to_tm(t.tv_sec, 0, &broken_tm);
	scnprintf(time_str, DIAG_TS_SIZE, "%d:%d:%d:%ld", broken_tm.tm_hour,
				broken_tm.tm_min, broken_tm.tm_sec, t.tv_usec);
}

int diag_get_remote(int remote_info)
{
	int val = (remote_info < 0) ? -remote_info : remote_info;
	int remote_val;

	switch (val) {
	case MDM:
	case MDM2:
	case QSC:
		remote_val = -remote_info;
		break;
	default:
		remote_val = 0;
		break;
	}

	return remote_val;
}

static int diag_copy_dci(char __user *buf, size_t count,
			struct diag_dci_client_tbl *entry, int *pret)
{
	int total_data_len = 0;
	int ret = 0;
	int exit_stat = 1;
	struct diag_dci_buffer_t *buf_entry, *temp;
	struct diag_smd_info *smd_info = NULL;

	if (!buf || !entry || !pret)
		return exit_stat;

	ret = *pret;

	ret += 4;

	mutex_lock(&entry->write_buf_mutex);
	list_for_each_entry_safe(buf_entry, temp, &entry->list_write_buf,
								buf_track) {
		list_del(&buf_entry->buf_track);
		mutex_lock(&buf_entry->data_mutex);
		if ((buf_entry->data_len > 0) &&
		    (buf_entry->in_busy) &&
		    (buf_entry->data)) {
			if (copy_to_user(buf+ret, (void *)buf_entry->data,
					 buf_entry->data_len))
				goto drop;
			ret += buf_entry->data_len;
			total_data_len += buf_entry->data_len;
			diag_ws_on_copy(DIAG_WS_DCI);
drop:
			buf_entry->in_busy = 0;
			buf_entry->data_len = 0;
			buf_entry->in_list = 0;
			if (buf_entry->buf_type == DCI_BUF_CMD) {
				if (buf_entry->data_source == APPS_DATA) {
					mutex_unlock(&buf_entry->data_mutex);
					continue;
				}
				if (driver->separate_cmdrsp[
						buf_entry->data_source]) {
					smd_info = &driver->smd_dci_cmd[
						buf_entry->data_source];
				} else {
					smd_info = &driver->smd_dci[
						buf_entry->data_source];
				}
				smd_info->in_busy_1 = 0;
				mutex_unlock(&buf_entry->data_mutex);
				continue;
			} else if (buf_entry->buf_type == DCI_BUF_SECONDARY) {
				diagmem_free(driver, buf_entry->data,
					     POOL_TYPE_DCI);
				buf_entry->data = NULL;
				mutex_unlock(&buf_entry->data_mutex);
				kfree(buf_entry);
				continue;
			}

		}
		mutex_unlock(&buf_entry->data_mutex);
	}

	if (total_data_len > 0) {
		/* Copy the total data length */
		COPY_USER_SPACE_OR_EXIT(buf+8, total_data_len, 4);
		ret -= 4;
	} else {
		pr_debug("diag: In %s, Trying to copy ZERO bytes, total_data_len: %d\n",
			__func__, total_data_len);
	}

	entry->in_service = 0;
	exit_stat = 0;
exit:
	mutex_unlock(&entry->write_buf_mutex);
	*pret = ret;

	return exit_stat;
}

#ifdef CONFIG_DIAGFWD_BRIDGE_CODE
static int diag_remote_init(void)
{
	diagmem_setsize(POOL_TYPE_MDM, itemsize_mdm, poolsize_mdm);
	diagmem_setsize(POOL_TYPE_MDM2, itemsize_mdm, poolsize_mdm);
	diagmem_setsize(POOL_TYPE_MDM_DCI, itemsize_mdm_dci, poolsize_mdm_dci);
	diagmem_setsize(POOL_TYPE_MDM2_DCI, itemsize_mdm_dci,
			poolsize_mdm_dci);
	diagmem_setsize(POOL_TYPE_MDM_MUX, itemsize_mdm_usb, poolsize_mdm_usb);
	diagmem_setsize(POOL_TYPE_MDM2_MUX, itemsize_mdm_usb, poolsize_mdm_usb);
	diagmem_setsize(POOL_TYPE_MDM_DCI_WRITE, itemsize_mdm_dci_write,
			poolsize_mdm_dci_write);
	diagmem_setsize(POOL_TYPE_MDM2_DCI_WRITE, itemsize_mdm_dci_write,
			poolsize_mdm_dci_write);
	diagmem_setsize(POOL_TYPE_QSC_MUX, itemsize_qsc_usb,
			poolsize_qsc_usb);
	driver->cb_buf = kzalloc(HDLC_OUT_BUF_SIZE, GFP_KERNEL);
	if (!driver->cb_buf)
		return -ENOMEM;
	driver->cb_buf_len = 0;
	return 0;
}

static void diag_remote_exit(void)
{
	kfree(driver->cb_buf);
}

static int diag_cb_send_data_remote(int proc, void *buf, int len)
{
	int err = 0;
	int max_len = 0;
	uint8_t retry_count = 0;
	uint8_t max_retries = 3;
	struct diag_send_desc_type send = { NULL, NULL, DIAG_STATE_START, 0 };
	struct diag_hdlc_dest_type enc = { NULL, NULL, 0 };

	if (!buf)
		return -EINVAL;

	if (len <= 0) {
		pr_err("diag: In %s, invalid len: %d", __func__, len);
		return -EBADMSG;
	}

	/*
	 * The worst case length will be twice as the incoming packet length.
	 * Add 3 bytes for CRC bytes (2 bytes) and delimiter (1 byte)
	 */
	max_len = (2 * len) + 3;
	if (HDLC_OUT_BUF_SIZE < max_len) {
		pr_err("diag: Dropping packet, HDLC encoded packet payload size crosses buffer limit. Current payload size %d\n",
			max_len);
		return -EBADMSG;
	}

	do {
		if (driver->cb_buf_len == 0)
			break;
		usleep_range(10000, 10100);
		retry_count++;
	} while (retry_count < max_retries);

	if (driver->cb_buf_len != 0)
		return -EAGAIN;

	/* Perform HDLC encoding on incoming data */
	send.state = DIAG_STATE_START;
	send.pkt = (void *)(buf);
	send.last = (void *)(buf + len - 1);
	send.terminate = 1;

	enc.dest = driver->cb_buf;
	enc.dest_last = (void *)(driver->cb_buf + max_len - 1);
	diag_hdlc_encode(&send, &enc);
	driver->cb_buf_len = (int)(enc.dest - (void *)driver->cb_buf);
	err = diagfwd_bridge_write(proc, driver->cb_buf,
				   driver->cb_buf_len);
	if (err) {
		pr_err("diag: Error writing Callback packet to proc: %d, err: %d\n",
			proc, err);
		driver->cb_buf_len = 0;
	}

	return err;
}

static int diag_process_userspace_remote(int proc, void *buf, int len)
{
	int bridge_index = proc - 1;

	if (!buf || len < 0) {
		pr_err("diag: Invalid input in %s, buf: %p, len: %d\n",
		       __func__, buf, len);
		return -EINVAL;
	}

	if (bridge_index < 0 || bridge_index > NUM_REMOTE_DEV) {
		pr_err("diag: In %s, invalid bridge index: %d\n", __func__,
		       bridge_index);
		return -EINVAL;
	}

	driver->user_space_data_busy = 1;
	return diagfwd_bridge_write(bridge_index, buf, len);
}
#else
static int diag_remote_init(void)
{
	return 0;
}

static void diag_remote_exit(void)
{
	return;
}

int diagfwd_bridge_init(void)
{
	return 0;
}

void diagfwd_bridge_exit(void)
{
	return;
}

uint16_t diag_get_remote_device_mask(void)
{
	return 0;
}

static int diag_cb_send_data_remote(int proc, void *buf, int len)
{
	return -EINVAL;
}

static int diag_process_userspace_remote(int proc, void *buf, int len)
{
	return 0;
}
#endif

static int diag_command_reg(struct bindpkt_params_per_process *pkt_params)
{
	int retval = -EINVAL;
	int i = 0, j;
	void *temp_buf;
	unsigned int count_entries = 0, interim_count = 0;
	struct bindpkt_params *params;
	struct bindpkt_params *head_params;

	if (!pkt_params)
		return -EINVAL;

	if ((UINT_MAX/sizeof(struct bindpkt_params)) <
						pkt_params->count) {
		pr_warn("diag: integer overflow while multiply\n");
		return -EFAULT;
	}

	head_params = kzalloc(pkt_params->count*sizeof(struct bindpkt_params),
								GFP_KERNEL);
	if (ZERO_OR_NULL_PTR(head_params)) {
		pr_err("diag: unable to alloc memory\n");
		return -ENOMEM;
	} else {
		params = head_params;
	}

	if (copy_from_user(params, pkt_params->params,
			pkt_params->count*sizeof(struct bindpkt_params))) {
		kfree(head_params);
		return -EFAULT;
	}
	mutex_lock(&driver->diagchar_mutex);
	for (i = 0; i < diag_max_reg; i++) {
		if (driver->table[i].process_id == 0) {
			retval = diag_add_reg(i, params, &count_entries);
			if (retval == 1 && pkt_params->count > count_entries) {
				params++;
			} else {
				kfree(head_params);
				mutex_unlock(&driver->diagchar_mutex);
				return retval;
			}
		}
	}
	if (i < diag_threshold_reg) {
		/* Increase table size by amount required */
		if (pkt_params->count >= count_entries) {
			interim_count = pkt_params->count - count_entries;
		} else {
			pr_warn("diag: error in params count\n");
			kfree(head_params);
			mutex_unlock(&driver->diagchar_mutex);
			return -EFAULT;
		}
		if (UINT_MAX - diag_max_reg >= interim_count) {
			diag_max_reg += interim_count;
		} else {
			pr_warn("diag: Integer overflow\n");
			kfree(head_params);
			mutex_unlock(&driver->diagchar_mutex);
			return -EFAULT;
		}
		/* Make sure size doesnt go beyond threshold */
		if (diag_max_reg > diag_threshold_reg) {
			diag_max_reg = diag_threshold_reg;
			pr_err("diag: best case memory allocation\n");
		}
		if (UINT_MAX/sizeof(struct diag_master_table) < diag_max_reg) {
			pr_warn("diag: integer overflow\n");
			kfree(head_params);
			mutex_unlock(&driver->diagchar_mutex);
			return -EFAULT;
		}
		temp_buf = krealloc(driver->table,
				diag_max_reg*sizeof(struct
				diag_master_table), GFP_KERNEL);
		if (!temp_buf) {
			pr_err("diag: Insufficient memory for reg.\n");

			if (pkt_params->count >= count_entries) {
				interim_count = pkt_params->count -
					count_entries;
			} else {
				pr_warn("diag: params count error\n");
				kfree(head_params);
				mutex_unlock(&driver->diagchar_mutex);
				return -EFAULT;
			}
			if (diag_max_reg >= interim_count) {
				diag_max_reg -= interim_count;
			} else {
				pr_warn("diag: Integer underflow\n");
				kfree(head_params);
				mutex_unlock(&driver->diagchar_mutex);
				return -EFAULT;
			}
			kfree(head_params);
			mutex_unlock(&driver->diagchar_mutex);
			return 0;
		} else {
			driver->table = temp_buf;
		}
		for (j = i; j < diag_max_reg; j++) {
			retval = diag_add_reg(j, params, &count_entries);
			if (retval == 1 && pkt_params->count > count_entries) {
				params++;
			} else {
				kfree(head_params);
				mutex_unlock(&driver->diagchar_mutex);
				return retval;
			}
		}
		kfree(head_params);
		mutex_unlock(&driver->diagchar_mutex);
	} else {
		kfree(head_params);
		mutex_unlock(&driver->diagchar_mutex);
		pr_err("Max size reached, Pkt Registration failed for Process %d",
					current->tgid);
	}
	retval = 0;
	return retval;
}

static int diag_switch_logging(int requested_mode)
{
	int success = -EINVAL;
	int temp = 0, status = 0;
	int new_mode = DIAG_USB_MODE; /* set the mode from diag_mux.h */
	int old_logging_id;

	switch (requested_mode) {
	case USB_MODE:
	case MEMORY_DEVICE_MODE:
	case NO_LOGGING_MODE:
	case UART_MODE:
	case SOCKET_MODE:
	case CALLBACK_MODE:
		break;
	default:
		pr_err("diag: In %s, request to switch to invalid mode: %d\n",
			__func__, requested_mode);
		return -EINVAL;
	}

	if (requested_mode == driver->logging_mode) {
		if (requested_mode != MEMORY_DEVICE_MODE ||
					driver->real_time_mode)
			pr_info_ratelimited("diag: Already in logging mode change requested, mode: %d\n",
					driver->logging_mode);
		return 0;
	}
	mutex_lock(&driver->diagchar_mutex);
	temp = driver->logging_mode;
	driver->logging_mode = requested_mode;
	old_logging_id = driver->logging_process_id;

	if (driver->logging_mode == MEMORY_DEVICE_MODE) {
		driver->mask_check = 1;
		new_mode = DIAG_MEMORY_DEVICE_MODE;
		if (driver->socket_process) {
			/*
			 * Notify the socket logging process that we
			 * are switching to MEMORY_DEVICE_MODE
			 */
			status = send_sig(SIGCONT,
				 driver->socket_process, 0);
			if (status) {
				pr_err("diag: %s, Error notifying ",
					__func__);
				pr_err("socket process, status: %d\n",
					status);
			}
			driver->socket_process = NULL;
		}
	} else if (driver->logging_mode == SOCKET_MODE) {
		driver->socket_process = current;
	} else if (driver->logging_mode == CALLBACK_MODE) {
		driver->callback_process = current;
	}

	if (driver->logging_mode == UART_MODE ||
				driver->logging_mode == SOCKET_MODE ||
				driver->logging_mode == CALLBACK_MODE) {
		driver->mask_check = 0;
		driver->logging_mode = MEMORY_DEVICE_MODE;
		new_mode = DIAG_MEMORY_DEVICE_MODE;
	} else if (driver->logging_mode == NO_LOGGING_MODE) {
		new_mode = DIAG_NO_LOGGING_MODE;
	}

	driver->logging_process_id = current->tgid;
	if (driver->logging_mode != MEMORY_DEVICE_MODE) {
		diag_update_real_time_vote(DIAG_PROC_MEMORY_DEVICE,
						MODE_REALTIME, ALL_PROC);
	} else {
		diag_update_proc_vote(DIAG_PROC_MEMORY_DEVICE, VOTE_UP,
						ALL_PROC);
	}

	if (!(driver->logging_mode == MEMORY_DEVICE_MODE &&
					temp == USB_MODE))
		queue_work(driver->diag_real_time_wq,
						&driver->diag_real_time_work);

	status = diag_mux_switch_logging(new_mode);
	if (status) {
		if (requested_mode == MEMORY_DEVICE_MODE)
			driver->mask_check = 0;
		else if (requested_mode == SOCKET_MODE)
			driver->socket_process = NULL;
		else if (requested_mode == CALLBACK_MODE)
			driver->callback_process = NULL;

		driver->logging_process_id = old_logging_id;
		driver->logging_mode = temp;
		pr_err("diag: Error switching logging mode, current logging mode: %d\n",
						driver->logging_mode);
		mutex_unlock(&driver->diagchar_mutex);
		success = status ? success : 1;
		return success;
	}
	mutex_unlock(&driver->diagchar_mutex);
	success = status ? success : 1;
	return success;
}

static int diag_ioctl_dci_reg(unsigned long ioarg)
{
	int result = -EINVAL;
	struct diag_dci_reg_tbl_t dci_reg_params;

	if (copy_from_user(&dci_reg_params, (void __user *)ioarg,
				sizeof(struct diag_dci_reg_tbl_t)))
		return -EFAULT;

	result = diag_dci_register_client(&dci_reg_params);

	return result;
}

static int diag_ioctl_dci_health_stats(unsigned long ioarg)
{
	int result = -EINVAL;
	struct diag_dci_health_stats_proc stats;

	if (copy_from_user(&stats, (void __user *)ioarg,
				sizeof(struct diag_dci_health_stats_proc)))
		return -EFAULT;

	result = diag_dci_copy_health_stats(&stats);
	if (result == DIAG_DCI_NO_ERROR) {
		if (copy_to_user((void __user *)ioarg, &stats,
			sizeof(struct diag_dci_health_stats_proc)))
			return -EFAULT;
	}

	return result;
}

static int diag_ioctl_dci_log_status(unsigned long ioarg)
{
	struct diag_log_event_stats le_stats;
	struct diag_dci_client_tbl *dci_client = NULL;

	if (copy_from_user(&le_stats, (void __user *)ioarg,
				sizeof(struct diag_log_event_stats)))
		return -EFAULT;

	dci_client = diag_dci_get_client_entry(le_stats.client_id);
	if (!dci_client)
		return DIAG_DCI_NOT_SUPPORTED;
	le_stats.is_set = diag_dci_query_log_mask(dci_client, le_stats.code);
	if (copy_to_user((void __user *)ioarg, &le_stats,
				sizeof(struct diag_log_event_stats)))
		return -EFAULT;

	return DIAG_DCI_NO_ERROR;
}

static int diag_ioctl_dci_event_status(unsigned long ioarg)
{
	struct diag_log_event_stats le_stats;
	struct diag_dci_client_tbl *dci_client = NULL;

	if (copy_from_user(&le_stats, (void __user *)ioarg,
				sizeof(struct diag_log_event_stats)))
		return -EFAULT;

	dci_client = diag_dci_get_client_entry(le_stats.client_id);
	if (!dci_client)
		return DIAG_DCI_NOT_SUPPORTED;

	le_stats.is_set = diag_dci_query_event_mask(dci_client, le_stats.code);
	if (copy_to_user((void __user *)ioarg, &le_stats,
				sizeof(struct diag_log_event_stats)))
		return -EFAULT;

	return DIAG_DCI_NO_ERROR;
}

static int diag_ioctl_lsm_deinit(void)
{
	int i;

	for (i = 0; i < driver->num_clients; i++)
		if (driver->client_map[i].pid == current->tgid)
			break;

	if (i == driver->num_clients)
		return -EINVAL;

	driver->data_ready[i] |= DEINIT_TYPE;
	wake_up_interruptible(&driver->wait_q);

	return 1;
}

static int diag_ioctl_vote_real_time(unsigned long ioarg)
{
	int real_time = 0;
	struct real_time_vote_t vote;
	struct diag_dci_client_tbl *dci_client = NULL;

	if (copy_from_user(&vote, (void __user *)ioarg,
			sizeof(struct real_time_vote_t)))
		return -EFAULT;

	driver->real_time_update_busy++;
	if (vote.proc == DIAG_PROC_DCI) {
		dci_client = diag_dci_get_client_entry(vote.client_id);
		if (!dci_client) {
			driver->real_time_update_busy--;
			return DIAG_DCI_NOT_SUPPORTED;
		}
		diag_dci_set_real_time(dci_client, vote.real_time_vote);
		real_time = diag_dci_get_cumulative_real_time(
					dci_client->client_info.token);
		diag_update_real_time_vote(vote.proc, real_time,
					dci_client->client_info.token);
	} else {
		real_time = vote.real_time_vote;
		diag_update_real_time_vote(vote.proc, real_time,
						ALL_PROC);
	}
	queue_work(driver->diag_real_time_wq, &driver->diag_real_time_work);
	return 0;
}

static int diag_ioctl_get_real_time(unsigned long ioarg)
{
	int i;
	int retry_count = 0;
	int timer = 0;
	struct real_time_query_t rt_query;

	if (copy_from_user(&rt_query, (void __user *)ioarg,
					sizeof(struct real_time_query_t)))
		return -EFAULT;
	while (retry_count < 3) {
		if (driver->real_time_update_busy > 0) {
			retry_count++;
			/*
			 * The value 10000 was chosen empirically as an
			 * optimum value in order to give the work in
			 * diag_real_time_wq to complete processing.
			 */
			for (timer = 0; timer < 5; timer++)
				usleep_range(10000, 10100);
		} else {
			break;
		}
	}

	if (driver->real_time_update_busy > 0)
		return -EAGAIN;

	if (rt_query.proc < 0 || rt_query.proc >= DIAG_NUM_PROC) {
		pr_err("diag: Invalid proc %d in %s\n", rt_query.proc,
		       __func__);
		return -EINVAL;
	}
	rt_query.real_time = driver->real_time_mode[rt_query.proc];
	/*
	 * For the local processor, if any of the peripherals is in buffering
	 * mode, overwrite the value of real time with UNKNOWN_MODE
	 */
	if (rt_query.proc == DIAG_LOCAL_PROC) {
		for (i = 0; i < NUM_SMD_CONTROL_CHANNELS; i++) {
			if (!driver->peripheral_buffering_support[i])
				continue;
			switch (driver->buffering_mode[i].mode) {
			case DIAG_BUFFERING_MODE_CIRCULAR:
			case DIAG_BUFFERING_MODE_THRESHOLD:
				rt_query.real_time = MODE_UNKNOWN;
				break;
			}
		}
	}

	if (copy_to_user((void __user *)ioarg, &rt_query,
			 sizeof(struct real_time_query_t)))
		return -EFAULT;

	return 0;
}

static int diag_ioctl_set_buffering_mode(unsigned long ioarg)
{
	struct diag_buffering_mode_t params;

	if (copy_from_user(&params, (void __user *)ioarg, sizeof(params)))
		return -EFAULT;

	return diag_send_peripheral_buffering_mode(&params);
}

static int diag_ioctl_peripheral_drain_immediate(unsigned long ioarg)
{
	uint8_t peripheral;
	struct diag_smd_info *smd_info = NULL;

	if (copy_from_user(&peripheral, (void __user *)ioarg, sizeof(uint8_t)))
		return -EFAULT;

	if (peripheral > LAST_PERIPHERAL) {
		pr_err("diag: In %s, invalid peripheral %d\n", __func__,
		       peripheral);
		return -EINVAL;
	}

	if (!driver->peripheral_buffering_support[peripheral]) {
		pr_err("diag: In %s, peripheral %d doesn't support buffering\n",
		       __func__, peripheral);
		return -EIO;
	}

	smd_info = &driver->smd_cntl[peripheral];
	return diag_send_peripheral_drain_immediate(smd_info);
}

static int diag_ioctl_dci_support(unsigned long ioarg)
{
	struct diag_dci_peripherals_t dci_support;
	int result = -EINVAL;

	if (copy_from_user(&dci_support, (void __user *)ioarg,
				sizeof(struct diag_dci_peripherals_t)))
		return -EFAULT;

	result = diag_dci_get_support_list(&dci_support);
	if (result == DIAG_DCI_NO_ERROR)
		if (copy_to_user((void __user *)ioarg, &dci_support,
				sizeof(struct diag_dci_peripherals_t)))
			return -EFAULT;

	return result;
}

#ifdef CONFIG_COMPAT

struct bindpkt_params_per_process_compat {
	/* Name of the synchronization object associated with this proc */
	char sync_obj_name[MAX_SYNC_OBJ_NAME_SIZE];
	uint32_t count;	/* Number of entries in this bind */
	compat_uptr_t params; /* first bind params */
};

long diagchar_compat_ioctl(struct file *filp,
			   unsigned int iocmd, unsigned long ioarg)
{
	int result = -EINVAL;
	int req_logging_mode = 0;
	int client_id = 0;
	uint16_t delayed_rsp_id = 0;
	uint16_t remote_dev;
	struct bindpkt_params_per_process pkt_params;
	struct bindpkt_params_per_process_compat pkt_params_compat;
	struct diag_dci_client_tbl *dci_client = NULL;

	switch (iocmd) {
	case DIAG_IOCTL_COMMAND_REG:
		if (copy_from_user(&pkt_params_compat, (void __user *)ioarg,
			sizeof(struct bindpkt_params_per_process_compat))) {
				return -EFAULT;
		}
		strlcpy(pkt_params.sync_obj_name,
			pkt_params_compat.sync_obj_name,
			MAX_SYNC_OBJ_NAME_SIZE);
		pkt_params.count = pkt_params_compat.count;
		pkt_params.params = (struct bindpkt_params *)(uintptr_t)
						pkt_params_compat.params;
		result = diag_command_reg(&pkt_params);
		break;
	case DIAG_IOCTL_GET_DELAYED_RSP_ID:
		delayed_rsp_id = diag_get_next_delayed_rsp_id();
		if (copy_to_user((void __user *)ioarg, &delayed_rsp_id,
				 sizeof(uint16_t)))
			result = -EFAULT;
		else
			result = 0;
		break;
	case DIAG_IOCTL_DCI_REG:
		result = diag_ioctl_dci_reg(ioarg);
		break;
	case DIAG_IOCTL_DCI_DEINIT:
		if (copy_from_user((void *)&client_id, (void __user *)ioarg,
			sizeof(int)))
			return -EFAULT;
		dci_client = diag_dci_get_client_entry(client_id);
		if (!dci_client)
			return DIAG_DCI_NOT_SUPPORTED;
		result = diag_dci_deinit_client(dci_client);
		break;
	case DIAG_IOCTL_DCI_SUPPORT:
		result = diag_ioctl_dci_support(ioarg);
		break;
	case DIAG_IOCTL_DCI_HEALTH_STATS:
		result = diag_ioctl_dci_health_stats(ioarg);
		break;
	case DIAG_IOCTL_DCI_LOG_STATUS:
		result = diag_ioctl_dci_log_status(ioarg);
		break;
	case DIAG_IOCTL_DCI_EVENT_STATUS:
		result = diag_ioctl_dci_event_status(ioarg);
		break;
	case DIAG_IOCTL_DCI_CLEAR_LOGS:
		if (copy_from_user((void *)&client_id, (void __user *)ioarg,
			sizeof(int)))
			return -EFAULT;
		result = diag_dci_clear_log_mask(client_id);
		break;
	case DIAG_IOCTL_DCI_CLEAR_EVENTS:
		if (copy_from_user(&client_id, (void __user *)ioarg,
			sizeof(int)))
			return -EFAULT;
		result = diag_dci_clear_event_mask(client_id);
		break;
	case DIAG_IOCTL_LSM_DEINIT:
		result = diag_ioctl_lsm_deinit();
		break;
	case DIAG_IOCTL_SWITCH_LOGGING:
		if (copy_from_user((void *)&req_logging_mode,
					(void __user *)ioarg, sizeof(int)))
			return -EFAULT;
		result = diag_switch_logging(req_logging_mode);
		break;
	case DIAG_IOCTL_REMOTE_DEV:
		remote_dev = diag_get_remote_device_mask();
		if (copy_to_user((void __user *)ioarg, &remote_dev,
			sizeof(uint16_t)))
			result = -EFAULT;
		else
			result = 1;
		break;
	case DIAG_IOCTL_VOTE_REAL_TIME:
		result = diag_ioctl_vote_real_time(ioarg);
		break;
	case DIAG_IOCTL_GET_REAL_TIME:
		result = diag_ioctl_get_real_time(ioarg);
		break;
	case DIAG_IOCTL_PERIPHERAL_BUF_CONFIG:
		result = diag_ioctl_set_buffering_mode(ioarg);
		break;
	case DIAG_IOCTL_PERIPHERAL_BUF_DRAIN:
		result = diag_ioctl_peripheral_drain_immediate(ioarg);
		break;
	}
	return result;
}
#endif

long diagchar_ioctl(struct file *filp,
			   unsigned int iocmd, unsigned long ioarg)
{
	int result = -EINVAL;
	int req_logging_mode = 0;
	int client_id = 0;
	uint16_t delayed_rsp_id;
	uint16_t remote_dev;
	struct bindpkt_params_per_process pkt_params;
	struct diag_dci_client_tbl *dci_client = NULL;

	switch (iocmd) {
	case DIAG_IOCTL_COMMAND_REG:
		if (copy_from_user(&pkt_params, (void __user *)ioarg,
			sizeof(struct bindpkt_params_per_process))) {
				return -EFAULT;
		}
		result = diag_command_reg(&pkt_params);
		break;
	case DIAG_IOCTL_GET_DELAYED_RSP_ID:
		delayed_rsp_id = diag_get_next_delayed_rsp_id();
		if (copy_to_user((void __user *)ioarg, &delayed_rsp_id,
				 sizeof(uint16_t)))
			result = -EFAULT;
		else
			result = 0;
		break;
	case DIAG_IOCTL_DCI_REG:
		result = diag_ioctl_dci_reg(ioarg);
		break;
	case DIAG_IOCTL_DCI_DEINIT:
		if (copy_from_user((void *)&client_id, (void __user *)ioarg,
			sizeof(int)))
			return -EFAULT;
		dci_client = diag_dci_get_client_entry(client_id);
		if (!dci_client)
			return DIAG_DCI_NOT_SUPPORTED;
		result = diag_dci_deinit_client(dci_client);
		break;
	case DIAG_IOCTL_DCI_SUPPORT:
		result = diag_ioctl_dci_support(ioarg);
		break;
	case DIAG_IOCTL_DCI_HEALTH_STATS:
		result = diag_ioctl_dci_health_stats(ioarg);
		break;
	case DIAG_IOCTL_DCI_LOG_STATUS:
		result = diag_ioctl_dci_log_status(ioarg);
		break;
	case DIAG_IOCTL_DCI_EVENT_STATUS:
		result = diag_ioctl_dci_event_status(ioarg);
		break;
	case DIAG_IOCTL_DCI_CLEAR_LOGS:
		if (copy_from_user((void *)&client_id, (void __user *)ioarg,
			sizeof(int)))
			return -EFAULT;
		result = diag_dci_clear_log_mask(client_id);
		break;
	case DIAG_IOCTL_DCI_CLEAR_EVENTS:
		if (copy_from_user(&client_id, (void __user *)ioarg,
			sizeof(int)))
			return -EFAULT;
		result = diag_dci_clear_event_mask(client_id);
		break;
	case DIAG_IOCTL_LSM_DEINIT:
		result = diag_ioctl_lsm_deinit();
		break;
	case DIAG_IOCTL_SWITCH_LOGGING:
		if (copy_from_user((void *)&req_logging_mode,
					(void __user *)ioarg, sizeof(int)))
			return -EFAULT;
		result = diag_switch_logging(req_logging_mode);
		break;
	case DIAG_IOCTL_REMOTE_DEV:
		remote_dev = diag_get_remote_device_mask();
		if (copy_to_user((void __user *)ioarg, &remote_dev,
			sizeof(uint16_t)))
			result = -EFAULT;
		else
			result = 1;
		break;
	case DIAG_IOCTL_VOTE_REAL_TIME:
		result = diag_ioctl_vote_real_time(ioarg);
		break;
	case DIAG_IOCTL_GET_REAL_TIME:
		result = diag_ioctl_get_real_time(ioarg);
		break;
	case DIAG_IOCTL_PERIPHERAL_BUF_CONFIG:
		result = diag_ioctl_set_buffering_mode(ioarg);
		break;
	case DIAG_IOCTL_PERIPHERAL_BUF_DRAIN:
		result = diag_ioctl_peripheral_drain_immediate(ioarg);
		break;
	}
	return result;
}

static ssize_t diagchar_read(struct file *file, char __user *buf, size_t count,
			  loff_t *ppos)
{
	struct diag_dci_client_tbl *entry;
	struct list_head *start, *temp;
	int index = -1, i = 0, ret = 0;
	int data_type;
	int copy_dci_data = 0;
	int exit_stat;
	int write_len = 0;

	for (i = 0; i < driver->num_clients; i++)
		if (driver->client_map[i].pid == current->tgid)
			index = i;

	if (index == -1) {
		pr_err("diag: Client PID not found in table");
		return -EINVAL;
	}
	if (!buf) {
		pr_err("diag: bad address from user side\n");
		return -EFAULT;
	}
	wait_event_interruptible(driver->wait_q, driver->data_ready[index]);

	mutex_lock(&driver->diagchar_mutex);

	if ((driver->data_ready[index] & USER_SPACE_DATA_TYPE) && (driver->
					logging_mode == MEMORY_DEVICE_MODE)) {
		pr_debug("diag: process woken up\n");
		/*Copy the type of data being passed*/
		data_type = driver->data_ready[index] & USER_SPACE_DATA_TYPE;
		driver->data_ready[index] ^= USER_SPACE_DATA_TYPE;
		COPY_USER_SPACE_OR_EXIT(buf, data_type, sizeof(int));
		/* place holder for number of data field */
		ret += sizeof(int);
		exit_stat = diag_md_copy_to_user(buf, &ret);
		goto exit;
	} else if (driver->data_ready[index] & USER_SPACE_DATA_TYPE) {
		/* In case, the thread wakes up and the logging mode is
		not memory device any more, the condition needs to be cleared */
		driver->data_ready[index] ^= USER_SPACE_DATA_TYPE;
	}

	if (driver->data_ready[index] & DEINIT_TYPE) {
		/*Copy the type of data being passed*/
		data_type = driver->data_ready[index] & DEINIT_TYPE;
		COPY_USER_SPACE_OR_EXIT(buf, data_type, 4);
		driver->data_ready[index] ^= DEINIT_TYPE;
		goto exit;
	}

	if (driver->data_ready[index] & MSG_MASKS_TYPE) {
		/*Copy the type of data being passed*/
		data_type = driver->data_ready[index] & MSG_MASKS_TYPE;
		COPY_USER_SPACE_OR_EXIT(buf, data_type, sizeof(int));
		write_len = diag_copy_to_user_msg_mask(buf + ret, count);
		if (write_len > 0)
			ret += write_len;
		driver->data_ready[index] ^= MSG_MASKS_TYPE;
		goto exit;
	}

	if (driver->data_ready[index] & EVENT_MASKS_TYPE) {
		/*Copy the type of data being passed*/
		data_type = driver->data_ready[index] & EVENT_MASKS_TYPE;
		COPY_USER_SPACE_OR_EXIT(buf, data_type, 4);
		COPY_USER_SPACE_OR_EXIT(buf+4, *(event_mask.ptr),
					event_mask.mask_len);
		driver->data_ready[index] ^= EVENT_MASKS_TYPE;
		goto exit;
	}

	if (driver->data_ready[index] & LOG_MASKS_TYPE) {
		/*Copy the type of data being passed*/
		data_type = driver->data_ready[index] & LOG_MASKS_TYPE;
		COPY_USER_SPACE_OR_EXIT(buf, data_type, sizeof(int));
		write_len = diag_copy_to_user_log_mask(buf + ret, count);
		if (write_len > 0)
			ret += write_len;
		driver->data_ready[index] ^= LOG_MASKS_TYPE;
		goto exit;
	}

	if (driver->data_ready[index] & PKT_TYPE) {
		/*Copy the type of data being passed*/
		data_type = driver->data_ready[index] & PKT_TYPE;
		COPY_USER_SPACE_OR_EXIT(buf, data_type, 4);
		COPY_USER_SPACE_OR_EXIT(buf+4, *(driver->pkt_buf),
							 driver->pkt_length);
		driver->data_ready[index] ^= PKT_TYPE;
		driver->in_busy_pktdata = 0;
		goto exit;
	}

	if (driver->data_ready[index] & DCI_PKT_TYPE) {
		/* Copy the type of data being passed */
		data_type = driver->data_ready[index] & DCI_PKT_TYPE;
		COPY_USER_SPACE_OR_EXIT(buf, data_type, 4);
		COPY_USER_SPACE_OR_EXIT(buf+4, *(driver->dci_pkt_buf),
					driver->dci_pkt_length);
		driver->data_ready[index] ^= DCI_PKT_TYPE;
		driver->in_busy_dcipktdata = 0;
		goto exit;
	}

	if (driver->data_ready[index] & DCI_EVENT_MASKS_TYPE) {
		/*Copy the type of data being passed*/
		data_type = driver->data_ready[index] & DCI_EVENT_MASKS_TYPE;
		COPY_USER_SPACE_OR_EXIT(buf, data_type, 4);
		COPY_USER_SPACE_OR_EXIT(buf+4, driver->num_dci_client, 4);
		COPY_USER_SPACE_OR_EXIT(buf + 8, (dci_ops_tbl[DCI_LOCAL_PROC].
				event_mask_composite), DCI_EVENT_MASK_SIZE);
		driver->data_ready[index] ^= DCI_EVENT_MASKS_TYPE;
		goto exit;
	}

	if (driver->data_ready[index] & DCI_LOG_MASKS_TYPE) {
		/*Copy the type of data being passed*/
		data_type = driver->data_ready[index] & DCI_LOG_MASKS_TYPE;
		COPY_USER_SPACE_OR_EXIT(buf, data_type, 4);
		COPY_USER_SPACE_OR_EXIT(buf+4, driver->num_dci_client, 4);
		COPY_USER_SPACE_OR_EXIT(buf+8, (dci_ops_tbl[DCI_LOCAL_PROC].
				log_mask_composite), DCI_LOG_MASK_SIZE);
		driver->data_ready[index] ^= DCI_LOG_MASKS_TYPE;
		goto exit;
	}

	if (driver->data_ready[index] & DCI_DATA_TYPE) {
		/* Copy the type of data being passed */
		data_type = driver->data_ready[index] & DCI_DATA_TYPE;
		list_for_each_safe(start, temp, &driver->dci_client_list) {
			entry = list_entry(start, struct diag_dci_client_tbl,
									track);
			if (entry->client->tgid != current->tgid)
				continue;
			if (!entry->in_service)
				continue;
			COPY_USER_SPACE_OR_EXIT(buf + ret, data_type,
								sizeof(int));
			COPY_USER_SPACE_OR_EXIT(buf + ret,
					entry->client_info.token, sizeof(int));
			copy_dci_data = 1;
			exit_stat = diag_copy_dci(buf, count, entry, &ret);
			driver->data_ready[index] ^= DCI_DATA_TYPE;
			if (exit_stat == 1)
				goto exit;
		}
		for (i = 0; i < NUM_SMD_DCI_CHANNELS; i++) {
			if (driver->smd_dci[i].ch) {
				queue_work(driver->diag_dci_wq,
				&(driver->smd_dci[i].diag_read_smd_work));
			}
		}
		if (driver->supports_separate_cmdrsp) {
			for (i = 0; i < NUM_SMD_DCI_CMD_CHANNELS; i++) {
				if (!driver->separate_cmdrsp[i])
					continue;
				if (driver->smd_dci_cmd[i].ch) {
					queue_work(driver->diag_dci_wq,
						&(driver->smd_dci_cmd[i].
							diag_read_smd_work));
				}
			}
		}
		goto exit;
	}
exit:
	mutex_unlock(&driver->diagchar_mutex);
	/*
	 * Flush any read that is currently pending on DCI data and
	 * command channnels. This will ensure that the next read is not
	 * missed.
	 */
	if (copy_dci_data) {
		diag_ws_on_copy_complete(DIAG_WS_DCI);
		flush_workqueue(driver->diag_dci_wq);
	}
	return ret;
}

static ssize_t diagchar_write(struct file *file, const char __user *buf,
				size_t count, loff_t *ppos)
{
	int err, ret = 0, pkt_type, token_offset = 0;
	int remote_proc = 0, data_type;
	uint8_t index;
	uint8_t retry_count = 0;
	uint8_t max_retries = 3;
#ifdef DIAG_DEBUG
	int length = 0, i;
#endif
	struct diag_send_desc_type send = { NULL, NULL, DIAG_STATE_START, 0 };
	struct diag_hdlc_dest_type enc = { NULL, NULL, 0 };
	void *buf_copy = NULL;
	void *user_space_data = NULL;
	unsigned int payload_size;

	index = 0;
	/* Get the packet type F3/log/event/Pkt response */
	err = copy_from_user((&pkt_type), buf, 4);
	if (err) {
		pr_alert("diag: copy failed for pkt_type\n");
		return -EAGAIN;
	}
	/* First 4 bytes indicate the type of payload - ignore these */
	if (count < 4) {
		pr_err("diag: Client sending short data\n");
		return -EBADMSG;
	}
	payload_size = count - 4;
	if (payload_size > USER_SPACE_DATA) {
		pr_err("diag: Dropping packet, packet payload size crosses 8KB limit. Current payload size %d\n",
				payload_size);
		driver->dropped_count++;
		return -EBADMSG;
	}
#ifdef CONFIG_DIAG_OVER_USB
	if (driver->logging_mode == NO_LOGGING_MODE ||
	    (!((pkt_type == DCI_DATA_TYPE) ||
	       ((pkt_type & (DATA_TYPE_DCI_LOG | DATA_TYPE_DCI_EVENT)) == 0))
		&& (driver->logging_mode == USB_MODE) &&
		(!driver->usb_connected))) {
		/*Drop the diag payload */
		return -EIO;
	}
#endif /* DIAG over USB */
	if (pkt_type == DCI_DATA_TYPE) {
		user_space_data = diagmem_alloc(driver, payload_size,
								POOL_TYPE_USER);
		if (!user_space_data) {
			driver->dropped_count++;
			return -ENOMEM;
		}
		err = copy_from_user(user_space_data, buf + 4, payload_size);
		if (err) {
			pr_alert("diag: copy failed for DCI data\n");
			diagmem_free(driver, user_space_data, POOL_TYPE_USER);
			user_space_data = NULL;
			return DIAG_DCI_SEND_DATA_FAIL;
		}
		err = diag_process_dci_transaction(user_space_data,
							payload_size);
		diagmem_free(driver, user_space_data, POOL_TYPE_USER);
		user_space_data = NULL;
		return err;
	}
	if (pkt_type == CALLBACK_DATA_TYPE) {
		if (payload_size > driver->itemsize) {
			pr_err("diag: Dropping packet, invalid packet size. Current payload size %d\n",
				payload_size);
			driver->dropped_count++;
			return -EBADMSG;
		}

		buf_copy = diagmem_alloc(driver, payload_size, POOL_TYPE_COPY);
		if (!buf_copy) {
			driver->dropped_count++;
			return -ENOMEM;
		}

		err = copy_from_user(buf_copy, buf + 4, payload_size);
		if (err) {
			pr_err("diag: copy failed for user space data\n");
			diagmem_free(driver, buf_copy, POOL_TYPE_COPY);
			buf_copy = NULL;
			return -EIO;
		}
		/* Check for proc_type */
		remote_proc = diag_get_remote(*(int *)buf_copy);

		if (!remote_proc) {
			wait_event_interruptible(driver->wait_q,
				 (driver->in_busy_pktdata == 0));
			ret = diag_process_apps_pkt(buf_copy, payload_size);
			diagmem_free(driver, buf_copy, POOL_TYPE_COPY);
			buf_copy = NULL;
			return ret;
		}

		token_offset = sizeof(int);
		if (payload_size <= MIN_SIZ_ALLOW) {
			pr_err("diag: In %s, possible integer underflow, payload size: %d\n",
			       __func__, payload_size);
			return -EBADMSG;
		}
		payload_size -= sizeof(int);
		ret = diag_cb_send_data_remote(remote_proc - 1,
				(void *)(buf_copy + token_offset),
				payload_size);
		diagmem_free(driver, buf_copy, POOL_TYPE_COPY);
		buf_copy = NULL;
		return ret;
	}
	if (pkt_type == USER_SPACE_DATA_TYPE) {
		do {
			if (!driver->user_space_data_busy)
				break;
			retry_count++;
			usleep_range(10000, 10100);
		} while (retry_count < max_retries);

		if (driver->user_space_data_busy)
			return -EAGAIN;

		err = copy_from_user(driver->user_space_data_buf, buf + 4,
							 payload_size);
		if (err) {
			pr_err("diag: copy failed for user space data\n");
			return -EIO;
		}
		/* Check for proc_type */
		remote_proc =
			diag_get_remote(*(int *)driver->user_space_data_buf);

		if (remote_proc) {
			if (payload_size <= MIN_SIZ_ALLOW) {
				pr_err("diag: Integer underflow in %s, payload size: %d",
							__func__, payload_size);
				return -EBADMSG;
			}
			token_offset = 4;
			payload_size -= 4;
			buf += 4;
		}

		/* Check masks for On-Device logging */
		if (driver->mask_check) {
			if (!mask_request_validate(driver->user_space_data_buf +
							 token_offset)) {
				pr_alert("diag: mask request Invalid\n");
				return -EFAULT;
			}
		}
		buf = buf + 4;
#ifdef DIAG_DEBUG
		pr_debug("diag: user space data %d\n", payload_size);
		for (i = 0; i < payload_size; i++)
			pr_debug("\t %x", *((driver->user_space_data_buf
						+ token_offset)+i));
#endif
		err = diag_process_userspace_remote(remote_proc,
						    driver->user_space_data_buf
						    + token_offset,
						    payload_size);
		if (err) {
			driver->user_space_data_busy = 0;
			pr_err("diag: Error sending mask to remote proc %d, err: %d\n",
			       remote_proc, err);
			return err;
		}
		/* send masks to local processor now */
		if (!remote_proc)
			diag_process_hdlc((void *)
				(driver->user_space_data_buf + token_offset),
					payload_size);
		return 0;
	}

	if (payload_size > itemsize) {
		pr_err("diag: Dropping packet, packet payload size crosses"
				"4KB limit. Current payload size %d\n",
				payload_size);
		driver->dropped_count++;
		return -EBADMSG;
	}

	buf_copy = diagmem_alloc(driver, payload_size, POOL_TYPE_COPY);
	if (!buf_copy) {
		driver->dropped_count++;
		return -ENOMEM;
	}

	err = copy_from_user(buf_copy, buf + 4, payload_size);
	if (err) {
		printk(KERN_INFO "diagchar : copy_from_user failed\n");
		diagmem_free(driver, buf_copy, POOL_TYPE_COPY);
		buf_copy = NULL;
		return -EFAULT;
	}

	data_type = pkt_type &
		    (DATA_TYPE_DCI_LOG | DATA_TYPE_DCI_EVENT | DCI_PKT_TYPE);
	if (data_type) {
		diag_process_apps_dci_read_data(data_type, buf_copy,
						payload_size);
		if (pkt_type & DATA_TYPE_DCI_LOG)
			pkt_type ^= DATA_TYPE_DCI_LOG;
		else if (pkt_type & DATA_TYPE_DCI_EVENT) {
			pkt_type ^= DATA_TYPE_DCI_EVENT;
		} else {
			pkt_type ^= DCI_PKT_TYPE;
			diagmem_free(driver, buf_copy, POOL_TYPE_COPY);
			return 0;
		}

		/*
		 * If the data is not headed for normal processing or the usb
		 * is unplugged and we are in usb mode
		 */
		if ((pkt_type != DATA_TYPE_LOG && pkt_type != DATA_TYPE_EVENT)
			|| ((driver->logging_mode == USB_MODE) &&
			(!driver->usb_connected))) {
			diagmem_free(driver, buf_copy, POOL_TYPE_COPY);
			return 0;
		}
	}

	if (driver->stm_state[APPS_DATA] &&
		(pkt_type >= DATA_TYPE_EVENT && pkt_type <= DATA_TYPE_LOG)) {
		int stm_size = 0;

		stm_size = stm_log_inv_ts(OST_ENTITY_DIAG, 0, buf_copy,
			payload_size);

		if (stm_size == 0)
			pr_debug("diag: In %s, stm_log_inv_ts returned size of 0\n",
				__func__);

		diagmem_free(driver, buf_copy, POOL_TYPE_COPY);
		return 0;
	}

#ifdef DIAG_DEBUG
	printk(KERN_DEBUG "data is -->\n");
	for (i = 0; i < payload_size; i++)
		printk(KERN_DEBUG "\t %x \t", *(((unsigned char *)buf_copy)+i));
#endif
	send.state = DIAG_STATE_START;
	send.pkt = buf_copy;
	send.last = (void *)(buf_copy + payload_size - 1);
	send.terminate = 1;
#ifdef DIAG_DEBUG
	pr_debug("diag: Already used bytes in buffer %d, and"
	" incoming payload size is %d\n", driver->used, payload_size);
	printk(KERN_DEBUG "hdlc encoded data is -->\n");
	for (i = 0; i < payload_size + 8; i++) {
		printk(KERN_DEBUG "\t %x \t", *(((unsigned char *)buf_hdlc)+i));
		if (*(((unsigned char *)buf_hdlc)+i) != 0x7e)
			length++;
	}
#endif
	mutex_lock(&driver->diagchar_mutex);
	if (!buf_hdlc)
		buf_hdlc = diagmem_alloc(driver, HDLC_OUT_BUF_SIZE,
						 POOL_TYPE_HDLC);
	if (!buf_hdlc) {
		ret = -ENOMEM;
		goto fail_free_copy;
	}
	if (HDLC_OUT_BUF_SIZE < (2*payload_size) + 3) {
		pr_err("diag: Dropping packet, HDLC encoded packet payload size crosses buffer limit. Current payload size %d\n",
				((2*payload_size) + 3));
		driver->dropped_count++;
		ret = -EBADMSG;
		goto fail_free_hdlc;
	}
	if (HDLC_OUT_BUF_SIZE - driver->used <= (2*payload_size) + 3) {
		err = diag_mux_write(DIAG_LOCAL_PROC, buf_hdlc, driver->used,
				     buf_hdlc_ctxt);
		if (err) {
			ret = -EIO;
			goto fail_free_hdlc;
		}
		buf_hdlc = NULL;
		driver->used = 0;
		buf_hdlc = diagmem_alloc(driver, HDLC_OUT_BUF_SIZE,
							 POOL_TYPE_HDLC);
		if (!buf_hdlc) {
			ret = -ENOMEM;
			goto fail_free_copy;
		}
	}

	enc.dest = buf_hdlc + driver->used;
	enc.dest_last = (void *)(buf_hdlc + driver->used + 2*payload_size + 3);
	diag_hdlc_encode(&send, &enc);

	/* This is to check if after HDLC encoding, we are still within the
	 limits of aggregation buffer. If not, we write out the current buffer
	and start aggregation in a newly allocated buffer */
	if ((uintptr_t)enc.dest >=
		 (uintptr_t)(buf_hdlc + HDLC_OUT_BUF_SIZE)) {
		err = diag_mux_write(DIAG_LOCAL_PROC, buf_hdlc, driver->used,
				     buf_hdlc_ctxt);
		if (err) {
			ret = -EIO;
			goto fail_free_hdlc;
		}
		buf_hdlc = NULL;
		driver->used = 0;
		buf_hdlc = diagmem_alloc(driver, HDLC_OUT_BUF_SIZE,
							 POOL_TYPE_HDLC);
		if (!buf_hdlc) {
			ret = -ENOMEM;
			goto fail_free_copy;
		}
		enc.dest = buf_hdlc + driver->used;
		enc.dest_last = (void *)(buf_hdlc + driver->used +
							 (2*payload_size) + 3);
		diag_hdlc_encode(&send, &enc);
	}

	driver->used = ((uintptr_t)enc.dest - (uintptr_t)buf_hdlc <
						HDLC_OUT_BUF_SIZE) ?
			((uintptr_t)enc.dest - (uintptr_t)buf_hdlc) :
						HDLC_OUT_BUF_SIZE;
	if (pkt_type == DATA_TYPE_RESPONSE) {
		err = diag_mux_write(DIAG_LOCAL_PROC, buf_hdlc, driver->used,
				     buf_hdlc_ctxt);
		if (err) {
			ret = -EIO;
			goto fail_free_hdlc;
		}
		buf_hdlc = NULL;
		driver->used = 0;
	}

	diagmem_free(driver, buf_copy, POOL_TYPE_COPY);
	buf_copy = NULL;
	mutex_unlock(&driver->diagchar_mutex);

	check_drain_timer();

	return 0;

fail_free_hdlc:
	diagmem_free(driver, buf_hdlc, POOL_TYPE_HDLC);
	buf_hdlc = NULL;
	driver->used = 0;
	diagmem_free(driver, buf_copy, POOL_TYPE_COPY);
	buf_copy = NULL;
	mutex_unlock(&driver->diagchar_mutex);
	return ret;

fail_free_copy:
	diagmem_free(driver, buf_copy, POOL_TYPE_COPY);
	buf_copy = NULL;
	mutex_unlock(&driver->diagchar_mutex);
	return ret;
}

void diag_ws_init()
{
	driver->dci_ws.ref_count = 0;
	driver->dci_ws.copy_count = 0;
	spin_lock_init(&driver->dci_ws.lock);

	driver->md_ws.ref_count = 0;
	driver->md_ws.copy_count = 0;
	spin_lock_init(&driver->md_ws.lock);

	spin_lock_init(&driver->ws_lock);
}

void diag_ws_on_notify()
{
	/*
	 * Do not deal with reference count here as there can be spurious
	 * interrupts.
	 */
	pm_stay_awake(driver->diag_dev);
}

void diag_ws_on_read(int type, int pkt_len)
{
	unsigned long flags;
	struct diag_ws_ref_t *ws_ref = NULL;

	switch (type) {
	case DIAG_WS_DCI:
		ws_ref = &driver->dci_ws;
		break;
	case DIAG_WS_MD:
		ws_ref = &driver->md_ws;
		break;
	default:
		pr_err_ratelimited("diag: In %s, invalid type: %d\n",
				   __func__, type);
		return;
	}

	spin_lock_irqsave(&ws_ref->lock, flags);
	if (pkt_len > 0) {
		ws_ref->ref_count++;
	} else {
		if (ws_ref->ref_count < 1) {
			ws_ref->ref_count = 0;
			ws_ref->copy_count = 0;
		}
		diag_ws_release();
	}
	spin_unlock_irqrestore(&ws_ref->lock, flags);
}


void diag_ws_on_copy(int type)
{
	unsigned long flags;
	struct diag_ws_ref_t *ws_ref = NULL;

	switch (type) {
	case DIAG_WS_DCI:
		ws_ref = &driver->dci_ws;
		break;
	case DIAG_WS_MD:
		ws_ref = &driver->md_ws;
		break;
	default:
		pr_err_ratelimited("diag: In %s, invalid type: %d\n",
				   __func__, type);
		return;
	}

	spin_lock_irqsave(&ws_ref->lock, flags);
	ws_ref->copy_count++;
	spin_unlock_irqrestore(&ws_ref->lock, flags);
}

void diag_ws_on_copy_fail(int type)
{
	unsigned long flags;
	struct diag_ws_ref_t *ws_ref = NULL;

	switch (type) {
	case DIAG_WS_DCI:
		ws_ref = &driver->dci_ws;
		break;
	case DIAG_WS_MD:
		ws_ref = &driver->md_ws;
		break;
	default:
		pr_err_ratelimited("diag: In %s, invalid type: %d\n",
				   __func__, type);
		return;
	}

	spin_lock_irqsave(&ws_ref->lock, flags);
	ws_ref->ref_count--;
	spin_unlock_irqrestore(&ws_ref->lock, flags);

	diag_ws_release();
}

void diag_ws_on_copy_complete(int type)
{
	unsigned long flags;
	struct diag_ws_ref_t *ws_ref = NULL;

	switch (type) {
	case DIAG_WS_DCI:
		ws_ref = &driver->dci_ws;
		break;
	case DIAG_WS_MD:
		ws_ref = &driver->md_ws;
		break;
	default:
		pr_err_ratelimited("diag: In %s, invalid type: %d\n",
				   __func__, type);
		return;
	}

	spin_lock_irqsave(&ws_ref->lock, flags);
	ws_ref->ref_count -= ws_ref->copy_count;
		if (ws_ref->ref_count < 1)
			ws_ref->ref_count = 0;
		ws_ref->copy_count = 0;
	spin_unlock_irqrestore(&ws_ref->lock, flags);

	diag_ws_release();
}

void diag_ws_reset(int type)
{
	unsigned long flags;
	struct diag_ws_ref_t *ws_ref = NULL;

	switch (type) {
	case DIAG_WS_DCI:
		ws_ref = &driver->dci_ws;
		break;
	case DIAG_WS_MD:
		ws_ref = &driver->md_ws;
		break;
	default:
		pr_err_ratelimited("diag: In %s, invalid type: %d\n",
				   __func__, type);
		return;
	}

	spin_lock_irqsave(&ws_ref->lock, flags);
	ws_ref->ref_count = 0;
	ws_ref->copy_count = 0;
	spin_unlock_irqrestore(&ws_ref->lock, flags);

	diag_ws_release();
}

void diag_ws_release()
{
	unsigned long flags;

	spin_lock_irqsave(&driver->ws_lock, flags);
	if (driver->dci_ws.ref_count == 0 && driver->md_ws.ref_count == 0)
		pm_relax(driver->diag_dev);
	spin_unlock_irqrestore(&driver->ws_lock, flags);
}

static int diag_real_time_info_init(void)
{
	int i;
	if (!driver)
		return -EIO;
	for (i = 0; i < DIAG_NUM_PROC; i++) {
		driver->real_time_mode[i] = 1;
		driver->proc_rt_vote_mask[i] |= DIAG_PROC_DCI;
		driver->proc_rt_vote_mask[i] |= DIAG_PROC_MEMORY_DEVICE;
	}
	driver->real_time_update_busy = 0;
	driver->proc_active_mask = 0;
	driver->diag_real_time_wq = create_singlethread_workqueue(
							"diag_real_time_wq");
	if (!driver->diag_real_time_wq)
		return -ENOMEM;
	INIT_WORK(&(driver->diag_real_time_work), diag_real_time_work_fn);
	mutex_init(&driver->real_time_mutex);
	return 0;
}

int mask_request_validate(unsigned char mask_buf[])
{
	uint8_t packet_id;
	uint8_t subsys_id;
	uint16_t ss_cmd;

	packet_id = mask_buf[0];

	if (packet_id == 0x4B) {
		subsys_id = mask_buf[1];
		ss_cmd = *(uint16_t *)(mask_buf + 2);
		/* Packets with SSID which are allowed */
		switch (subsys_id) {
		case 0x04: /* DIAG_SUBSYS_WCDMA */
			if ((ss_cmd == 0) || (ss_cmd == 0xF))
				return 1;
			break;
		case 0x08: /* DIAG_SUBSYS_GSM */
			if ((ss_cmd == 0) || (ss_cmd == 0x1))
				return 1;
			break;
		case 0x09: /* DIAG_SUBSYS_UMTS */
		case 0x0F: /* DIAG_SUBSYS_CM */
			if (ss_cmd == 0)
				return 1;
			break;
		case 0x0C: /* DIAG_SUBSYS_OS */
			if ((ss_cmd == 2) || (ss_cmd == 0x100))
				return 1; /* MPU and APU */
			break;
		case 0x12: /* DIAG_SUBSYS_DIAG_SERV */
			if ((ss_cmd == 0) || (ss_cmd == 0x6) || (ss_cmd == 0x7))
				return 1;
			break;
		case 0x13: /* DIAG_SUBSYS_FS */
			if ((ss_cmd == 0) || (ss_cmd == 0x1))
				return 1;
			break;
		default:
			return 0;
			break;
		}
	} else {
		switch (packet_id) {
		case 0x00:    /* Version Number */
		case 0x0C:    /* CDMA status packet */
		case 0x1C:    /* Diag Version */
		case 0x1D:    /* Time Stamp */
		case 0x60:    /* Event Report Control */
		case 0x63:    /* Status snapshot */
		case 0x73:    /* Logging Configuration */
		case 0x7C:    /* Extended build ID */
		case 0x7D:    /* Extended Message configuration */
		case 0x81:    /* Event get mask */
		case 0x82:    /* Set the event mask */
			return 1;
			break;
		default:
			return 0;
			break;
		}
	}
	return 0;
}

static const struct file_operations diagcharfops = {
	.owner = THIS_MODULE,
	.read = diagchar_read,
	.write = diagchar_write,
#ifdef CONFIG_COMPAT
	.compat_ioctl = diagchar_compat_ioctl,
#endif
	.unlocked_ioctl = diagchar_ioctl,
	.open = diagchar_open,
	.release = diagchar_close
};

static int diagchar_setup_cdev(dev_t devno)
{

	int err;

	cdev_init(driver->cdev, &diagcharfops);

	driver->cdev->owner = THIS_MODULE;
	driver->cdev->ops = &diagcharfops;

	err = cdev_add(driver->cdev, devno, 1);

	if (err) {
		printk(KERN_INFO "diagchar cdev registration failed !\n\n");
		return -1;
	}

	driver->diagchar_class = class_create(THIS_MODULE, "diag");

	if (IS_ERR(driver->diagchar_class)) {
		printk(KERN_ERR "Error creating diagchar class.\n");
		return -1;
	}

	driver->diag_dev = device_create(driver->diagchar_class, NULL, devno,
					 (void *)driver, "diag");

	if (!driver->diag_dev)
		return -EIO;

	driver->diag_dev->power.wakeup = wakeup_source_register("DIAG_WS");
	return 0;

}

static int diagchar_cleanup(void)
{
	if (driver) {
		if (driver->cdev) {
			/* TODO - Check if device exists before deleting */
			device_destroy(driver->diagchar_class,
				       MKDEV(driver->major,
					     driver->minor_start));
			cdev_del(driver->cdev);
		}
		if (!IS_ERR(driver->diagchar_class))
			class_destroy(driver->diagchar_class);
		kfree(driver);
	}
	return 0;
}

static int __init diagchar_init(void)
{
	dev_t dev;
	int error, ret;

	pr_debug("diagfwd initializing ..\n");
	ret = 0;
	driver = kzalloc(sizeof(struct diagchar_dev) + 5, GFP_KERNEL);
	if (!driver)
		return -ENOMEM;
	kmemleak_not_leak(driver);

	driver->used = 0;
	timer_in_progress = 0;
	driver->delayed_rsp_id = 0;
	driver->debug_flag = 1;
	driver->dci_state = DIAG_DCI_NO_ERROR;
	setup_timer(&drain_timer, drain_timer_func, 1234);
	driver->itemsize = itemsize;
	driver->poolsize = poolsize;
	/*
	 * HDLC buffer can be at most twice as much as itemsize. Add 3 for
	 * 16-bit CRC and delimiter.
	 */
	itemsize_hdlc = (itemsize * 2) + 3;
	driver->itemsize_hdlc = itemsize_hdlc;
	driver->poolsize_hdlc = poolsize_hdlc;
	driver->itemsize_dci = itemsize_dci;
	driver->poolsize_dci = poolsize_dci;
	diagmem_setsize(POOL_TYPE_COPY, itemsize, poolsize);
	diagmem_setsize(POOL_TYPE_HDLC, itemsize_hdlc, poolsize_hdlc);
	diagmem_setsize(POOL_TYPE_USER, itemsize_user, poolsize_user);
	/*
	 * POOL_TYPE_MUX_APPS is for the buffers in the Diag MUX layer.
	 * The number of buffers encompasses Diag data generated on
	 * the Apss processor + 1 for the responses generated exclusively on
	 * the Apps processor + data from SMD data channels (2 channels per
	 * peripheral) + data from SMD command channels
	 */
	diagmem_setsize(POOL_TYPE_MUX_APPS, itemsize_usb_apps,
			poolsize_usb_apps + 1 + (NUM_SMD_DATA_CHANNELS * 2) +
			NUM_SMD_CMD_CHANNELS);
	diagmem_setsize(POOL_TYPE_DCI, itemsize_dci, poolsize_dci);
	driver->num_clients = max_clients;
	driver->logging_mode = USB_MODE;
	driver->socket_process = NULL;
	driver->callback_process = NULL;
	driver->mask_check = 0;
	driver->in_busy_pktdata = 0;
	driver->in_busy_dcipktdata = 0;
	driver->rsp_buf_ctxt = SET_BUF_CTXT(APPS_DATA, SMD_CMD_TYPE, 1);
	buf_hdlc_ctxt = SET_BUF_CTXT(APPS_DATA, SMD_DATA_TYPE, 1);
	mutex_init(&driver->diagchar_mutex);
	mutex_init(&driver->delayed_rsp_mutex);
	init_waitqueue_head(&driver->wait_q);
	init_waitqueue_head(&driver->smd_wait_q);
	INIT_WORK(&(driver->diag_drain_work), diag_drain_work_fn);
	diag_ws_init();
	ret = diag_real_time_info_init();
	if (ret)
		goto fail;
	ret = diag_debugfs_init();
	if (ret)
		goto fail;
	ret = diag_masks_init();
	if (ret)
		goto fail;
	ret = diag_mux_init();
	if (ret)
		goto fail;
	ret = diagfwd_init();
	if (ret)
		goto fail;
	ret = diag_remote_init();
	if (ret)
		goto fail;
	ret = diagfwd_bridge_init();
	if (ret)
		goto fail;
	ret = diagfwd_cntl_init();
	if (ret)
		goto fail;
	driver->dci_state = diag_dci_init();
	pr_debug("diagchar initializing ..\n");
	driver->num = 1;
	driver->name = ((void *)driver) + sizeof(struct diagchar_dev);
	strlcpy(driver->name, "diag", 4);
	/* Get major number from kernel and initialize */
	error = alloc_chrdev_region(&dev, driver->minor_start,
				    driver->num, driver->name);
	if (!error) {
		driver->major = MAJOR(dev);
		driver->minor_start = MINOR(dev);
	} else {
		pr_err("diag: Major number not allocated\n");
		goto fail;
	}
	driver->cdev = cdev_alloc();
	error = diagchar_setup_cdev(dev);
	if (error)
		goto fail;

	pr_debug("diagchar initialized now");
	return 0;

fail:
	pr_err("diagchar is not initialized, ret: %d\n", ret);
	diag_debugfs_cleanup();
	diagchar_cleanup();
	diag_mux_exit();
	diagfwd_bridge_exit();
	diagfwd_exit();
	diagfwd_cntl_exit();
	diag_dci_exit();
	diag_masks_exit();
	diag_remote_exit();
	return -1;
}

static void diagchar_exit(void)
{
	printk(KERN_INFO "diagchar exiting ..\n");
	diag_mempool_exit();
	diag_mux_exit();
	diagfwd_exit();
	diagfwd_cntl_exit();
	diag_dci_exit();
	diag_masks_exit();
	diag_remote_exit();
	diag_debugfs_cleanup();
	diagchar_cleanup();
	printk(KERN_INFO "done diagchar exit\n");
}

module_init(diagchar_init);
module_exit(diagchar_exit);
