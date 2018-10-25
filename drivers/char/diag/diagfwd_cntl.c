/* Copyright (c) 2011-2018, The Linux Foundation. All rights reserved.
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
#include <linux/diagchar.h>
#include <linux/kmemleak.h>
#include <linux/delay.h>
#include "diagchar.h"
#include "diagfwd.h"
#include "diagfwd_cntl.h"
#include "diagfwd_peripheral.h"
#include "diagfwd_bridge.h"
#include "diag_dci.h"
#include "diagmem.h"
#include "diag_masks.h"
#include "diag_ipc_logging.h"
#include "diag_mux.h"

#define FEATURE_SUPPORTED(x)	((feature_mask << (i * 8)) & (1 << x))

/* tracks which peripheral is undergoing SSR */
static uint16_t reg_dirty;
static uint8_t diag_id = DIAG_ID_APPS;
static void diag_notify_md_client(uint8_t peripheral, int data);

static void diag_mask_update_work_fn(struct work_struct *work)
{
	uint8_t peripheral;

	for (peripheral = 0; peripheral <= NUM_PERIPHERALS; peripheral++) {
		if (!(driver->mask_update & PERIPHERAL_MASK(peripheral)))
			continue;
		diag_send_updates_peripheral(peripheral);
	}
}

void diag_cntl_channel_open(struct diagfwd_info *p_info)
{
	if (!p_info) {
		DIAG_LOG(DIAG_DEBUG_PERIPHERALS,
		"diag: Invalid fwd_info structure\n");
		return;
	}
	driver->mask_update |= PERIPHERAL_MASK(p_info->peripheral);
	queue_work(driver->cntl_wq, &driver->mask_update_work);
	diag_notify_md_client(p_info->peripheral, DIAG_STATUS_OPEN);
}

void diag_cntl_channel_close(struct diagfwd_info *p_info)
{
	uint8_t peripheral;

	if (!p_info) {
		DIAG_LOG(DIAG_DEBUG_PERIPHERALS,
		"diag: Invalid fwd_info structure\n");
		return;
	}

	peripheral = p_info->peripheral;
	if (peripheral >= NUM_PERIPHERALS) {
		DIAG_LOG(DIAG_DEBUG_PERIPHERALS,
			"diag: Invalid peripheral (%d)\n", peripheral);
		return;
	}

	driver->feature[peripheral].sent_feature_mask = 0;
	driver->feature[peripheral].rcvd_feature_mask = 0;
	reg_dirty |= PERIPHERAL_MASK(peripheral);
	diag_cmd_remove_reg_by_proc(peripheral);
	driver->diag_id_sent[peripheral] = 0;
	driver->feature[peripheral].stm_support = DISABLE_STM;
	driver->feature[peripheral].log_on_demand = 0;
	driver->stm_state[peripheral] = DISABLE_STM;
	driver->stm_state_requested[peripheral] = DISABLE_STM;
	reg_dirty ^= PERIPHERAL_MASK(peripheral);
	diag_notify_md_client(peripheral, DIAG_STATUS_CLOSED);
}

static void diag_stm_update_work_fn(struct work_struct *work)
{
	uint8_t i;
	uint16_t peripheral_mask = 0;
	int err = 0;

	mutex_lock(&driver->cntl_lock);
	peripheral_mask = driver->stm_peripheral;
	driver->stm_peripheral = 0;
	mutex_unlock(&driver->cntl_lock);

	if (peripheral_mask == 0) {
		DIAG_LOG(DIAG_DEBUG_PERIPHERALS,
			"diag: Empty Peripheral mask\n");
		return;
	}

	for (i = 0; i < NUM_PERIPHERALS; i++) {
		if (!driver->feature[i].stm_support)
			continue;
		if (peripheral_mask & PERIPHERAL_MASK(i)) {
			err = diag_send_stm_state(i,
				(uint8_t)(driver->stm_state_requested[i]));
			if (!err) {
				driver->stm_state[i] =
					driver->stm_state_requested[i];
			}
		}
	}
}

void diag_notify_md_client(uint8_t peripheral, int data)
{
	int stat = 0;
	struct siginfo info;
	struct pid *pid_struct;
	struct task_struct *result;

	if (peripheral > NUM_PERIPHERALS) {
		DIAG_LOG(DIAG_DEBUG_PERIPHERALS,
			"diag: Invalid peripheral (%d)\n", peripheral);
		return;
	}

	if (driver->logging_mode != DIAG_MEMORY_DEVICE_MODE) {
		DIAG_LOG(DIAG_DEBUG_PERIPHERALS,
			"diag: Invalid logging_mode (%d)\n",
			driver->logging_mode);
		return;
	}

	mutex_lock(&driver->md_session_lock);
	memset(&info, 0, sizeof(struct siginfo));
	info.si_code = SI_QUEUE;
	info.si_int = (PERIPHERAL_MASK(peripheral) | data);
	info.si_signo = SIGCONT;

	if (!driver->md_session_map[peripheral] ||
		driver->md_session_map[peripheral]->pid <= 0) {
		pr_err("diag: md_session_map[%d] is invalid\n", peripheral);
		mutex_unlock(&driver->md_session_lock);
		return;
	}

	pid_struct = find_get_pid(
			driver->md_session_map[peripheral]->pid);
	DIAG_LOG(DIAG_DEBUG_PERIPHERALS,
		"md_session_map[%d] pid = %d task = %pK\n",
		peripheral,
		driver->md_session_map[peripheral]->pid,
		driver->md_session_map[peripheral]->task);

	if (pid_struct) {
		result = get_pid_task(pid_struct, PIDTYPE_PID);

		if (!result) {
			DIAG_LOG(DIAG_DEBUG_PERIPHERALS,
				"diag: md_session_map[%d] with pid = %d Exited..\n",
				peripheral,
				driver->md_session_map[peripheral]->pid);
			mutex_unlock(&driver->md_session_lock);
			return;
		}

		if (driver->md_session_map[peripheral] &&
			driver->md_session_map[peripheral]->task == result) {
			stat = send_sig_info(info.si_signo,
					&info, result);
			if (stat)
				pr_err("diag: Err sending signal to memory device client, signal data: 0x%x, stat: %d\n",
					info.si_int, stat);
		} else
			pr_err("diag: md_session_map[%d] data is corrupted, signal data: 0x%x, stat: %d\n",
				peripheral, info.si_int, stat);
	}
	mutex_unlock(&driver->md_session_lock);
}

static void process_pd_status(uint8_t *buf, uint32_t len,
			      uint8_t peripheral)
{
	struct diag_ctrl_msg_pd_status *pd_msg = NULL;
	uint32_t pd;
	int status = DIAG_STATUS_CLOSED;

	if (!buf || peripheral >= NUM_PERIPHERALS || len < sizeof(*pd_msg)) {
		DIAG_LOG(DIAG_DEBUG_PERIPHERALS,
		"diag: Invalid parameters:(!buf) = %d, peripheral = %d, len = %d, pd_msg_len = %d\n",
		!buf, peripheral, len, (int)sizeof(*pd_msg));
		return;
	}

	pd_msg = (struct diag_ctrl_msg_pd_status *)buf;
	pd = pd_msg->pd_id;
	status = (pd_msg->status == 0) ? DIAG_STATUS_OPEN : DIAG_STATUS_CLOSED;
	diag_notify_md_client(peripheral, status);
}

static void enable_stm_feature(uint8_t peripheral)
{
	if (peripheral >= NUM_PERIPHERALS) {
		DIAG_LOG(DIAG_DEBUG_PERIPHERALS,
		"diag: Invalid peripheral (%d)\n", peripheral);
		return;
	}

	mutex_lock(&driver->cntl_lock);
	driver->feature[peripheral].stm_support = ENABLE_STM;
	driver->stm_peripheral |= PERIPHERAL_MASK(peripheral);
	mutex_unlock(&driver->cntl_lock);

	queue_work(driver->cntl_wq, &(driver->stm_update_work));
}

static void enable_socket_feature(uint8_t peripheral)
{
	if (peripheral >= NUM_PERIPHERALS) {
		DIAG_LOG(DIAG_DEBUG_PERIPHERALS,
		"diag: Invalid peripheral (%d)\n", peripheral);
		return;
	}

	if (driver->supports_sockets)
		driver->feature[peripheral].sockets_enabled = 1;
	else
		driver->feature[peripheral].sockets_enabled = 0;
}

static void process_hdlc_encoding_feature(uint8_t peripheral)
{
	if (peripheral >= NUM_PERIPHERALS) {
		DIAG_LOG(DIAG_DEBUG_PERIPHERALS,
		"diag: Invalid peripheral (%d)\n", peripheral);
		return;
	}

	if (driver->supports_apps_hdlc_encoding) {
		driver->feature[peripheral].encode_hdlc =
					ENABLE_APPS_HDLC_ENCODING;
	} else {
		driver->feature[peripheral].encode_hdlc =
					DISABLE_APPS_HDLC_ENCODING;
	}
}

static void process_upd_header_untagging_feature(uint8_t peripheral)
{
	if (peripheral >= NUM_PERIPHERALS) {
		DIAG_LOG(DIAG_DEBUG_PERIPHERALS,
		"diag: Invalid peripheral (%d)\n", peripheral);
		return;
	}

	if (driver->supports_apps_header_untagging) {
		driver->feature[peripheral].untag_header =
					ENABLE_PKT_HEADER_UNTAGGING;
	} else {
		driver->feature[peripheral].untag_header =
					DISABLE_PKT_HEADER_UNTAGGING;
	}
}

static void process_command_deregistration(uint8_t *buf, uint32_t len,
					   uint8_t peripheral)
{
	uint8_t *ptr = buf;
	int i;
	int header_len = sizeof(struct diag_ctrl_cmd_dereg);
	int read_len = 0;
	struct diag_ctrl_cmd_dereg *dereg = NULL;
	struct cmd_code_range *range = NULL;
	struct diag_cmd_reg_entry_t del_entry;

	/*
	 * Perform Basic sanity. The len field is the size of the data payload.
	 * This doesn't include the header size.
	 */
	if (!buf || peripheral >= NUM_PERIPHERALS || len == 0) {
		DIAG_LOG(DIAG_DEBUG_PERIPHERALS,
		"diag: Invalid parameters:(!buf) = %d, peripheral = %d, len = %d\n",
		!buf, peripheral, len);
		return;
	}

	DIAG_LOG(DIAG_DEBUG_CONTROL,
		"diag:peripheral(%d) command deregistration packet processing started\n",
		peripheral);

	dereg = (struct diag_ctrl_cmd_dereg *)ptr;
	ptr += header_len;
	/* Don't account for pkt_id and length */
	read_len += header_len - (2 * sizeof(uint32_t));

	if (dereg->count_entries == 0) {
		DIAG_LOG(DIAG_DEBUG_PERIPHERALS,
			"diag: received reg tbl with no entries\n");
		return;
	}

	for (i = 0; i < dereg->count_entries && read_len < len; i++) {
		range = (struct cmd_code_range *)ptr;
		ptr += sizeof(struct cmd_code_range) - sizeof(uint32_t);
		read_len += sizeof(struct cmd_code_range) - sizeof(uint32_t);
		del_entry.cmd_code = dereg->cmd_code;
		del_entry.subsys_id = dereg->subsysid;
		del_entry.cmd_code_hi = range->cmd_code_hi;
		del_entry.cmd_code_lo = range->cmd_code_lo;
		diag_cmd_remove_reg(&del_entry, peripheral);
	}

	if (i != dereg->count_entries) {
		pr_err("diag: In %s, reading less than available, read_len: %d, len: %d count: %d\n",
		       __func__, read_len, len, dereg->count_entries);
	}
	DIAG_LOG(DIAG_DEBUG_CONTROL,
		"diag:peripheral(%d) command deregistration packet processing complete\n",
		peripheral);
}
static void process_command_registration(uint8_t *buf, uint32_t len,
					 uint8_t peripheral)
{
	uint8_t *ptr = buf;
	int i;
	int header_len = sizeof(struct diag_ctrl_cmd_reg);
	int read_len = 0;
	struct diag_ctrl_cmd_reg *reg = NULL;
	struct cmd_code_range *range = NULL;
	struct diag_cmd_reg_entry_t new_entry;

	/*
	 * Perform Basic sanity. The len field is the size of the data payload.
	 * This doesn't include the header size.
	 */
	if (!buf || peripheral >= NUM_PERIPHERALS || len == 0) {
		DIAG_LOG(DIAG_DEBUG_PERIPHERALS,
		"diag: Invalid parameters:(!buf) = %d, peripheral = %d, len = %d\n",
		!buf, peripheral, len);
		return;
	}
	DIAG_LOG(DIAG_DEBUG_CONTROL,
		"diag: peripheral(%d) command registration packet processing started\n",
		peripheral);

	reg = (struct diag_ctrl_cmd_reg *)ptr;
	ptr += header_len;
	/* Don't account for pkt_id and length */
	read_len += header_len - (2 * sizeof(uint32_t));

	if (reg->count_entries == 0) {
		DIAG_LOG(DIAG_DEBUG_PERIPHERALS,
			"diag: In %s, received reg tbl with no entries\n",
			 __func__);
		return;
	}

	for (i = 0; i < reg->count_entries && read_len < len; i++) {
		range = (struct cmd_code_range *)ptr;
		ptr += sizeof(struct cmd_code_range);
		read_len += sizeof(struct cmd_code_range);
		new_entry.cmd_code = reg->cmd_code;
		new_entry.subsys_id = reg->subsysid;
		new_entry.cmd_code_hi = range->cmd_code_hi;
		new_entry.cmd_code_lo = range->cmd_code_lo;
		diag_cmd_add_reg(&new_entry, peripheral, INVALID_PID);
	}

	if (i != reg->count_entries) {
		pr_err("diag: In %s, reading less than available, read_len: %d, len: %d count: %d\n",
		       __func__, read_len, len, reg->count_entries);
	}
	DIAG_LOG(DIAG_DEBUG_CONTROL,
		"diag: peripheral(%d) command registration packet processing complete\n",
		peripheral);
}

static void diag_close_transport_work_fn(struct work_struct *work)
{
	uint8_t transport;
	uint8_t peripheral;

	mutex_lock(&driver->cntl_lock);
	for (peripheral = 0; peripheral <= NUM_PERIPHERALS; peripheral++) {
		if (!(driver->close_transport & PERIPHERAL_MASK(peripheral)))
			continue;
		driver->close_transport ^= PERIPHERAL_MASK(peripheral);
#ifdef CONFIG_DIAG_USES_SMD
		transport = driver->feature[peripheral].sockets_enabled ?
					TRANSPORT_SMD : TRANSPORT_SOCKET;
#else
		transport = driver->feature[peripheral].sockets_enabled ?
					TRANSPORT_GLINK : TRANSPORT_SOCKET;
#endif
		diagfwd_close_transport(transport, peripheral);
	}
	mutex_unlock(&driver->cntl_lock);
}

static void process_socket_feature(uint8_t peripheral)
{
	if (peripheral >= NUM_PERIPHERALS) {
		DIAG_LOG(DIAG_DEBUG_PERIPHERALS,
		"diag: Invalid peripheral (%d)\n", peripheral);
		return;
	}

	mutex_lock(&driver->cntl_lock);
	driver->close_transport |= PERIPHERAL_MASK(peripheral);
	queue_work(driver->cntl_wq, &driver->close_transport_work);
	mutex_unlock(&driver->cntl_lock);
}

static void process_log_on_demand_feature(uint8_t peripheral)
{
	/* Log On Demand command is registered only on Modem */
	if (peripheral != PERIPHERAL_MODEM)
		return;

	if (driver->feature[PERIPHERAL_MODEM].log_on_demand)
		driver->log_on_demand_support = 1;
	else
		driver->log_on_demand_support = 0;
}

static void process_incoming_feature_mask(uint8_t *buf, uint32_t len,
					  uint8_t peripheral)
{
	int i;
	int header_len = sizeof(struct diag_ctrl_feature_mask);
	int read_len = 0;
	struct diag_ctrl_feature_mask *header = NULL;
	uint32_t feature_mask_len = 0;
	uint32_t feature_mask = 0;
	uint8_t *ptr = buf;

	if (!buf || peripheral >= NUM_PERIPHERALS || len == 0) {
		DIAG_LOG(DIAG_DEBUG_PERIPHERALS,
		"diag: Invalid parameters:(!buf) = %d, peripheral = %d, len = %d\n",
		!buf, peripheral, len);
		return;
	}

	header = (struct diag_ctrl_feature_mask *)ptr;
	ptr += header_len;
	feature_mask_len = header->feature_mask_len;

	if (feature_mask_len == 0) {
		DIAG_LOG(DIAG_DEBUG_PERIPHERALS,
			"diag: In %s, received invalid feature mask from peripheral %d\n",
			 __func__, peripheral);
		return;
	}

	if (feature_mask_len > FEATURE_MASK_LEN) {
		pr_alert("diag: Receiving feature mask length more than Apps support\n");
		feature_mask_len = FEATURE_MASK_LEN;
	}

	diag_cmd_remove_reg_by_proc(peripheral);

	driver->feature[peripheral].rcvd_feature_mask = 1;
	DIAG_LOG(DIAG_DEBUG_CONTROL,
	"diag: Received feature mask for peripheral %d\n", peripheral);

	for (i = 0; i < feature_mask_len && read_len < len; i++) {
		feature_mask = *(uint8_t *)ptr;
		driver->feature[peripheral].feature_mask[i] = feature_mask;
		ptr += sizeof(uint8_t);
		read_len += sizeof(uint8_t);

		if (FEATURE_SUPPORTED(F_DIAG_LOG_ON_DEMAND_APPS))
			driver->feature[peripheral].log_on_demand = 1;
		if (FEATURE_SUPPORTED(F_DIAG_REQ_RSP_SUPPORT))
			driver->feature[peripheral].separate_cmd_rsp = 1;
		if (FEATURE_SUPPORTED(F_DIAG_APPS_HDLC_ENCODE))
			process_hdlc_encoding_feature(peripheral);
		if (FEATURE_SUPPORTED(F_DIAG_PKT_HEADER_UNTAG))
			process_upd_header_untagging_feature(peripheral);
		if (FEATURE_SUPPORTED(F_DIAG_STM))
			enable_stm_feature(peripheral);
		if (FEATURE_SUPPORTED(F_DIAG_MASK_CENTRALIZATION))
			driver->feature[peripheral].mask_centralization = 1;
		if (FEATURE_SUPPORTED(F_DIAG_PERIPHERAL_BUFFERING))
			driver->feature[peripheral].peripheral_buffering = 1;
		if (FEATURE_SUPPORTED(F_DIAG_SOCKETS_ENABLED))
			enable_socket_feature(peripheral);
		if (FEATURE_SUPPORTED(F_DIAG_DIAGID_SUPPORT))
			driver->feature[peripheral].diag_id_support = 1;
		if (FEATURE_SUPPORTED(F_DIAG_PD_BUFFERING))
			driver->feature[peripheral].pd_buffering = 1;
	}

	process_socket_feature(peripheral);
	process_log_on_demand_feature(peripheral);

	DIAG_LOG(DIAG_DEBUG_CONTROL,
		"diag: Peripheral(%d) feature mask is processed\n",
		peripheral);
}

static void process_last_event_report(uint8_t *buf, uint32_t len,
				      uint8_t peripheral)
{
	struct diag_ctrl_last_event_report *header = NULL;
	uint8_t *ptr = buf;
	uint8_t *temp = NULL;
	uint32_t pkt_len = sizeof(uint32_t) + sizeof(uint16_t);
	uint16_t event_size = 0;

	if (!buf || peripheral >= NUM_PERIPHERALS || len != pkt_len) {
		DIAG_LOG(DIAG_DEBUG_PERIPHERALS,
		"diag: Invalid parameters:(!buf) = %d, peripheral = %d, len = %d, pkt_len = %d\n",
		!buf, peripheral, len, pkt_len);
		return;
	}

	DIAG_LOG(DIAG_DEBUG_CONTROL,
		"diag:started processing last event report for peripheral (%d)\n",
		peripheral);

	mutex_lock(&event_mask.lock);
	header = (struct diag_ctrl_last_event_report *)ptr;
	event_size = ((header->event_last_id / 8) + 1);
	if (event_size >= driver->event_mask_size) {
		DIAG_LOG(DIAG_DEBUG_MASKS,
		"diag: receiving event mask size more that Apps can handle\n");
		temp = krealloc(driver->event_mask->ptr, event_size,
				GFP_KERNEL);
		if (!temp) {
			pr_err("diag: In %s, unable to reallocate event mask to support events from %d\n",
			       __func__, peripheral);
			goto err;
		}
		driver->event_mask->ptr = temp;
		driver->event_mask_size = event_size;
	}

	driver->num_event_id[peripheral] = header->event_last_id;
	if (header->event_last_id > driver->last_event_id)
		driver->last_event_id = header->event_last_id;
err:
	mutex_unlock(&event_mask.lock);
	DIAG_LOG(DIAG_DEBUG_CONTROL,
		"diag: last event report processed for peripheral (%d)\n",
		peripheral);
}

static void process_log_range_report(uint8_t *buf, uint32_t len,
				     uint8_t peripheral)
{
	int i;
	int read_len = 0;
	int header_len = sizeof(struct diag_ctrl_log_range_report);
	uint8_t *ptr = buf;
	struct diag_ctrl_log_range_report *header = NULL;
	struct diag_ctrl_log_range *log_range = NULL;
	struct diag_log_mask_t *mask_ptr = NULL;

	if (!buf || peripheral >= NUM_PERIPHERALS || len < 0) {
		DIAG_LOG(DIAG_DEBUG_PERIPHERALS,
		"diag: Invalid parameters:(!buf) = %d, peripheral = %d, len = %d\n",
		!buf, peripheral, len);
		return;
	}
	DIAG_LOG(DIAG_DEBUG_CONTROL,
		"diag:started processing log range report for peripheral(%d)\n",
		peripheral);

	header = (struct diag_ctrl_log_range_report *)ptr;
	ptr += header_len;
	/* Don't account for pkt_id and length */
	read_len += header_len - (2 * sizeof(uint32_t));

	driver->num_equip_id[peripheral] = header->num_ranges;
	for (i = 0; i < header->num_ranges && read_len < len; i++) {
		log_range = (struct diag_ctrl_log_range *)ptr;
		ptr += sizeof(struct diag_ctrl_log_range);
		read_len += sizeof(struct diag_ctrl_log_range);

		if (log_range->equip_id >= MAX_EQUIP_ID) {
			pr_err("diag: receiving log equip id %d more than supported equip id: %d from peripheral: %d\n",
			       log_range->equip_id, MAX_EQUIP_ID, peripheral);
			continue;
		}
		mask_ptr = (struct diag_log_mask_t *)log_mask.ptr;
		mask_ptr = &mask_ptr[log_range->equip_id];

		mutex_lock(&(mask_ptr->lock));
		mask_ptr->num_items = log_range->num_items;
		mask_ptr->range = LOG_ITEMS_TO_SIZE(log_range->num_items);
		mutex_unlock(&(mask_ptr->lock));
	}
	DIAG_LOG(DIAG_DEBUG_CONTROL,
		"diag: log range report processed for peripheral (%d)\n",
		peripheral);
}

static int update_msg_mask_tbl_entry(struct diag_msg_mask_t *mask,
				     struct diag_ssid_range_t *range)
{
	uint32_t temp_range;

	if (!mask || !range) {
		DIAG_LOG(DIAG_DEBUG_PERIPHERALS,
		"diag: Invalid %s\n",
		(!mask ? "mask" : (!range ? "range" : " ")));
		return -EIO;
	}
	if (range->ssid_last < range->ssid_first) {
		pr_err("diag: In %s, invalid ssid range, first: %d, last: %d\n",
		       __func__, range->ssid_first, range->ssid_last);
		return -EINVAL;
	}
	if (range->ssid_last >= mask->ssid_last) {
		temp_range = range->ssid_last - mask->ssid_first + 1;
		mask->ssid_last = range->ssid_last;
		mask->range = temp_range;
	}

	return 0;
}

static void process_ssid_range_report(uint8_t *buf, uint32_t len,
				      uint8_t peripheral)
{
	int i;
	int j;
	int read_len = 0;
	int found = 0;
	int new_size = 0;
	int err = 0;
	struct diag_ctrl_ssid_range_report *header = NULL;
	struct diag_ssid_range_t *ssid_range = NULL;
	int header_len = sizeof(struct diag_ctrl_ssid_range_report);
	struct diag_msg_mask_t *mask_ptr = NULL;
	uint8_t *ptr = buf;
	uint8_t *temp = NULL;
	uint32_t min_len = header_len - sizeof(struct diag_ctrl_pkt_header_t);

	if (!buf || peripheral >= NUM_PERIPHERALS || len < min_len) {
		DIAG_LOG(DIAG_DEBUG_PERIPHERALS,
		"diag: Invalid parameters:(!buf) = %d, peripheral = %d, len = %d, min_len = %d\n",
		!buf, peripheral, len, min_len);
		return;
	}

	DIAG_LOG(DIAG_DEBUG_CONTROL,
		"diag: started processing ssid range for peripheral (%d)\n",
		peripheral);

	header = (struct diag_ctrl_ssid_range_report *)ptr;
	ptr += header_len;
	/* Don't account for pkt_id and length */
	read_len += header_len - (2 * sizeof(uint32_t));

	mutex_lock(&driver->msg_mask_lock);
	driver->max_ssid_count[peripheral] = header->count;
	for (i = 0; i < header->count && read_len < len; i++) {
		ssid_range = (struct diag_ssid_range_t *)ptr;
		ptr += sizeof(struct diag_ssid_range_t);
		read_len += sizeof(struct diag_ssid_range_t);
		mask_ptr = (struct diag_msg_mask_t *)msg_mask.ptr;
		found = 0;
		for (j = 0; j < driver->msg_mask_tbl_count; j++, mask_ptr++) {
			if (!mask_ptr || !ssid_range) {
				found = 1;
				break;
			}
			if (mask_ptr->ssid_first != ssid_range->ssid_first)
				continue;
			mutex_lock(&mask_ptr->lock);
			err = update_msg_mask_tbl_entry(mask_ptr, ssid_range);
			mutex_unlock(&mask_ptr->lock);
			if (err == -ENOMEM) {
				pr_err("diag: In %s, unable to increase the msg mask table range\n",
				       __func__);
			}
			found = 1;
			break;
		}

		if (found)
			continue;

		new_size = (driver->msg_mask_tbl_count + 1) *
			   sizeof(struct diag_msg_mask_t);
		DIAG_LOG(DIAG_DEBUG_MASKS,
			"diag: receiving msg mask size more that Apps can handle\n");
		temp = krealloc(msg_mask.ptr, new_size, GFP_KERNEL);
		if (!temp) {
			pr_err("diag: In %s, Unable to add new ssid table to msg mask, ssid first: %d, last: %d\n",
			       __func__, ssid_range->ssid_first,
			       ssid_range->ssid_last);
			continue;
		}
		msg_mask.ptr = temp;
		mask_ptr = (struct diag_msg_mask_t *)msg_mask.ptr;
		err = diag_create_msg_mask_table_entry(mask_ptr, ssid_range);
		if (err) {
			pr_err("diag: In %s, Unable to create a new msg mask table entry, first: %d last: %d err: %d\n",
			       __func__, ssid_range->ssid_first,
			       ssid_range->ssid_last, err);
			continue;
		}
		driver->msg_mask_tbl_count += 1;
	}
	mutex_unlock(&driver->msg_mask_lock);
	DIAG_LOG(DIAG_DEBUG_CONTROL,
		"diag: processed ssid range for peripheral(%d)\n",
		peripheral);
}

static void diag_build_time_mask_update(uint8_t *buf,
					struct diag_ssid_range_t *range)
{
	int i;
	int j;
	int num_items = 0;
	int err = 0;
	int found = 0;
	int new_size = 0;
	uint8_t *temp = NULL;
	uint32_t *mask_ptr = (uint32_t *)buf;
	uint32_t *dest_ptr = NULL;
	struct diag_msg_mask_t *build_mask = NULL;

	if (!range || !buf) {
		DIAG_LOG(DIAG_DEBUG_PERIPHERALS,
		"diag: Invalid %s\n",
		(!range ? "range" : (!buf ? "buf" : " ")));
		return;
	}

	if (range->ssid_last < range->ssid_first) {
		pr_err("diag: In %s, invalid ssid range, first: %d, last: %d\n",
		       __func__, range->ssid_first, range->ssid_last);
		return;
	}
	mutex_lock(&driver->msg_mask_lock);
	build_mask = (struct diag_msg_mask_t *)(driver->build_time_mask->ptr);
	num_items = range->ssid_last - range->ssid_first + 1;

	for (i = 0; i < driver->bt_msg_mask_tbl_count; i++, build_mask++) {
		if (!build_mask) {
			found = 1;
			break;
		}
		if (build_mask->ssid_first != range->ssid_first)
			continue;
		found = 1;
		mutex_lock(&build_mask->lock);
		err = update_msg_mask_tbl_entry(build_mask, range);
		if (err == -ENOMEM) {
			pr_err("diag: In %s, unable to increase the msg build mask table range\n",
			       __func__);
		}
		dest_ptr = build_mask->ptr;
		for (j = 0; (j < build_mask->range) && mask_ptr && dest_ptr;
			j++, mask_ptr++, dest_ptr++)
			*(uint32_t *)dest_ptr |= *mask_ptr;
		mutex_unlock(&build_mask->lock);
		break;
	}

	if (found)
		goto end;

	new_size = (driver->bt_msg_mask_tbl_count + 1) *
		   sizeof(struct diag_msg_mask_t);
	DIAG_LOG(DIAG_DEBUG_MASKS,
		"diag: receiving build time mask size more that Apps can handle\n");

	temp = krealloc(driver->build_time_mask->ptr, new_size, GFP_KERNEL);
	if (!temp) {
		pr_err("diag: In %s, unable to create a new entry for build time mask\n",
		       __func__);
		goto end;
	}
	driver->build_time_mask->ptr = temp;
	build_mask = (struct diag_msg_mask_t *)driver->build_time_mask->ptr;
	err = diag_create_msg_mask_table_entry(build_mask, range);
	if (err) {
		pr_err("diag: In %s, Unable to create a new msg mask table entry, err: %d\n",
		       __func__, err);
		goto end;
	}
	driver->bt_msg_mask_tbl_count += 1;
end:
	mutex_unlock(&driver->msg_mask_lock);
	return;
}

static void process_build_mask_report(uint8_t *buf, uint32_t len,
				      uint8_t peripheral)
{
	int i;
	int read_len = 0;
	int num_items = 0;
	int header_len = sizeof(struct diag_ctrl_build_mask_report);
	uint8_t *ptr = buf;
	struct diag_ctrl_build_mask_report *header = NULL;
	struct diag_ssid_range_t *range = NULL;

	if (!buf || peripheral >= NUM_PERIPHERALS || len < header_len) {
		DIAG_LOG(DIAG_DEBUG_PERIPHERALS,
		"diag: Invalid parameters:(!buf) = %d, peripheral = %d, len = %d, header_len = %d\n",
		!buf, peripheral, len, header_len);
		return;
	}

	DIAG_LOG(DIAG_DEBUG_CONTROL,
		"diag: started processing build mask for peripheral(%d)\n",
		peripheral);

	header = (struct diag_ctrl_build_mask_report *)ptr;
	ptr += header_len;
	/* Don't account for pkt_id and length */
	read_len += header_len - (2 * sizeof(uint32_t));

	for (i = 0; i < header->count && read_len < len; i++) {
		range = (struct diag_ssid_range_t *)ptr;
		ptr += sizeof(struct diag_ssid_range_t);
		read_len += sizeof(struct diag_ssid_range_t);
		num_items = range->ssid_last - range->ssid_first + 1;
		diag_build_time_mask_update(ptr, range);
		ptr += num_items * sizeof(uint32_t);
		read_len += num_items * sizeof(uint32_t);
	}
	DIAG_LOG(DIAG_DEBUG_CONTROL,
		"diag: processing build mask complete (%d)\n", peripheral);
}

int diag_add_diag_id_to_list(uint8_t diag_id, char *process_name,
	uint8_t pd_val, uint8_t peripheral)
{
	struct diag_id_tbl_t *new_item = NULL;

	if (!process_name || diag_id == 0) {
		DIAG_LOG(DIAG_DEBUG_PERIPHERALS,
		"diag: Invalid parameters: !process_name = %d, diag_id = %d\n",
		!process_name, diag_id);
		return -EINVAL;
	}

	new_item = kzalloc(sizeof(struct diag_id_tbl_t), GFP_KERNEL);
	if (!new_item)
		return -ENOMEM;
	kmemleak_not_leak(new_item);
	new_item->process_name = kzalloc(strlen(process_name) + 1, GFP_KERNEL);
	if (!new_item->process_name) {
		kfree(new_item);
		new_item = NULL;
		return -ENOMEM;
	}
	kmemleak_not_leak(new_item->process_name);
	new_item->diag_id = diag_id;
	new_item->pd_val = pd_val;
	new_item->peripheral = peripheral;
	strlcpy(new_item->process_name, process_name, strlen(process_name) + 1);
	INIT_LIST_HEAD(&new_item->link);
	mutex_lock(&driver->diag_id_mutex);
	list_add_tail(&new_item->link, &driver->diag_id_list);
	mutex_unlock(&driver->diag_id_mutex);
	return 0;
}

int diag_query_diag_id(char *process_name, uint8_t *diag_id)
{
	struct list_head *start;
	struct list_head *temp;
	struct diag_id_tbl_t *item = NULL;

	if (!process_name || !diag_id) {
		DIAG_LOG(DIAG_DEBUG_PERIPHERALS, "diag: Invalid parameters\n");
		return -EINVAL;
	}

	mutex_lock(&driver->diag_id_mutex);
	list_for_each_safe(start, temp, &driver->diag_id_list) {
		item = list_entry(start, struct diag_id_tbl_t, link);
		if (strcmp(item->process_name, process_name) == 0) {
			*diag_id = item->diag_id;
			mutex_unlock(&driver->diag_id_mutex);
			return 1;
		}
	}
	mutex_unlock(&driver->diag_id_mutex);
	return 0;
}
static void process_diagid(uint8_t *buf, uint32_t len,
				      uint8_t peripheral)
{
	struct diag_ctrl_diagid *header = NULL;
	struct diag_ctrl_diagid ctrl_pkt;
	struct diagfwd_info *fwd_info = NULL;
	char *process_name = NULL;
	int err = 0;
	int pd_val;
	char *root_str = NULL;
	uint8_t local_diag_id = 0;
	uint8_t new_request = 0, i = 0, ch_type = 0;

	if (!buf || len == 0 || peripheral >= NUM_PERIPHERALS) {
		DIAG_LOG(DIAG_DEBUG_PERIPHERALS,
		"diag: Invalid parameters: !buf = %d, len = %d, peripheral = %d\n",
		!buf, len, peripheral);
		return;
	}

	header = (struct diag_ctrl_diagid *)buf;
	process_name = (char *)&header->process_name;
	if (diag_query_diag_id(process_name, &local_diag_id))
		ctrl_pkt.diag_id = local_diag_id;
	else {
		diag_id++;
		new_request = 1;
		pd_val = diag_query_pd(process_name);
		if (pd_val < 0)
			return;
		diag_add_diag_id_to_list(diag_id, process_name,
			pd_val, peripheral);
		ctrl_pkt.diag_id = diag_id;
	}
	root_str = strnstr(process_name, DIAG_ID_ROOT_STRING,
		strlen(process_name));

	if (new_request) {
		for (ch_type = 0; ch_type < NUM_TYPES; ch_type++) {
			if (ch_type == TYPE_DCI ||
				ch_type == TYPE_DCI_CMD)
				continue;
			fwd_info = &peripheral_info[ch_type][peripheral];
			fwd_info->num_pd++;

			if (root_str) {
				fwd_info->root_diag_id.diagid_val =
					ctrl_pkt.diag_id;
				fwd_info->root_diag_id.reg_str =
					process_name;
				fwd_info->root_diag_id.pd = pd_val;
			} else {
				i = fwd_info->num_pd - 2;
				if (i >= 0 && i < MAX_PERIPHERAL_UPD) {
					fwd_info->upd_diag_id[i].diagid_val =
						ctrl_pkt.diag_id;
					fwd_info->upd_diag_id[i].reg_str =
						process_name;
					fwd_info->upd_diag_id[i].pd = pd_val;
				}
			}
		}
	}

	DIAG_LOG(DIAG_DEBUG_PERIPHERALS,
		"diag: peripheral = %d: diag_id string = %s,diag_id = %d\n",
		peripheral, process_name, ctrl_pkt.diag_id);

	ctrl_pkt.pkt_id = DIAG_CTRL_MSG_DIAGID;
	ctrl_pkt.version = 1;
	strlcpy((char *)&ctrl_pkt.process_name, process_name,
		strlen(process_name) + 1);
	ctrl_pkt.len = sizeof(ctrl_pkt.diag_id) + sizeof(ctrl_pkt.version) +
			strlen(process_name) + 1;
	err = diagfwd_write(peripheral, TYPE_CNTL, &ctrl_pkt, ctrl_pkt.len +
				sizeof(ctrl_pkt.pkt_id) + sizeof(ctrl_pkt.len));
	if (err && err != -ENODEV) {
		pr_err("diag: Unable to send diag id ctrl packet to peripheral %d, err: %d\n",
		       peripheral, err);
	} else {
		/*
		 * Masks (F3, logs and events) will be sent to
		 * peripheral immediately following feature mask update only
		 * if diag_id support is not present or
		 * diag_id support is present and diag_id has been sent to
		 * peripheral.
		 * With diag_id being sent now, mask will be updated
		 * to peripherals.
		 */
		if (root_str) {
			driver->diag_id_sent[peripheral] = 1;
			queue_work(driver->cntl_wq, &driver->mask_update_work);
		}
		fwd_info = &peripheral_info[TYPE_DATA][peripheral];
		diagfwd_buffers_init(fwd_info);
		DIAG_LOG(DIAG_DEBUG_PERIPHERALS,
		"diag: diag_id sent = %d to peripheral = %d with diag_id = %d for %s\n",
			driver->diag_id_sent[peripheral], peripheral,
			ctrl_pkt.diag_id, process_name);
	}
}

void diag_cntl_process_read_data(struct diagfwd_info *p_info, void *buf,
				 int len)
{
	uint32_t read_len = 0;
	uint32_t header_len = sizeof(struct diag_ctrl_pkt_header_t);
	uint8_t *ptr = buf;
	struct diag_ctrl_pkt_header_t *ctrl_pkt = NULL;

	if (!buf || len <= 0 || !p_info) {
		DIAG_LOG(DIAG_DEBUG_PERIPHERALS, "diag: Invalid parameters\n");
		return;
	}

	if (reg_dirty & PERIPHERAL_MASK(p_info->peripheral)) {
		pr_err_ratelimited("diag: dropping command registration from peripheral %d\n",
		       p_info->peripheral);
		return;
	}

	while (read_len + header_len < len) {
		ctrl_pkt = (struct diag_ctrl_pkt_header_t *)ptr;
		DIAG_LOG(DIAG_DEBUG_CONTROL,
			"diag:peripheral: %d: pkt_id: %d\n",
			p_info->peripheral, ctrl_pkt->pkt_id);
		switch (ctrl_pkt->pkt_id) {
		case DIAG_CTRL_MSG_REG:
			process_command_registration(ptr, ctrl_pkt->len,
						     p_info->peripheral);
			break;
		case DIAG_CTRL_MSG_DEREG:
			process_command_deregistration(ptr, ctrl_pkt->len,
						       p_info->peripheral);
			break;
		case DIAG_CTRL_MSG_FEATURE:
			process_incoming_feature_mask(ptr, ctrl_pkt->len,
						      p_info->peripheral);
			break;
		case DIAG_CTRL_MSG_LAST_EVENT_REPORT:
			process_last_event_report(ptr, ctrl_pkt->len,
						  p_info->peripheral);
			break;
		case DIAG_CTRL_MSG_LOG_RANGE_REPORT:
			process_log_range_report(ptr, ctrl_pkt->len,
						 p_info->peripheral);
			break;
		case DIAG_CTRL_MSG_SSID_RANGE_REPORT:
			process_ssid_range_report(ptr, ctrl_pkt->len,
						  p_info->peripheral);
			break;
		case DIAG_CTRL_MSG_BUILD_MASK_REPORT:
			process_build_mask_report(ptr, ctrl_pkt->len,
						  p_info->peripheral);
			break;
		case DIAG_CTRL_MSG_PD_STATUS:
			process_pd_status(ptr, ctrl_pkt->len,
						p_info->peripheral);
			break;
		case DIAG_CTRL_MSG_DIAGID:
			process_diagid(ptr, ctrl_pkt->len,
						   p_info->peripheral);
			break;
		default:
			DIAG_LOG(DIAG_DEBUG_CONTROL,
			"diag: Control packet %d not supported\n",
			 ctrl_pkt->pkt_id);
		}
		ptr += header_len + ctrl_pkt->len;
		read_len += header_len + ctrl_pkt->len;
	}
	DIAG_LOG(DIAG_DEBUG_CONTROL,
	"diag: control packet processing complete\n");
}

static int diag_compute_real_time(int idx)
{
	int real_time = MODE_REALTIME;

	if (driver->proc_active_mask == 0) {
		/*
		 * There are no DCI or Memory Device processes. Diag should
		 * be in Real Time mode irrespective of USB connection
		 */
		real_time = MODE_REALTIME;
	} else if (driver->proc_rt_vote_mask[idx] & driver->proc_active_mask) {
		/*
		 * Atleast one process is alive and is voting for Real Time
		 * data - Diag should be in real time mode irrespective of USB
		 * connection.
		 */
		real_time = MODE_REALTIME;
	} else if (driver->usb_connected) {
		/*
		 * If USB is connected, check individual process. If Memory
		 * Device Mode is active, set the mode requested by Memory
		 * Device process. Set to realtime mode otherwise.
		 */
		if ((driver->proc_rt_vote_mask[idx] &
						DIAG_PROC_MEMORY_DEVICE) == 0)
			real_time = MODE_NONREALTIME;
		else
			real_time = MODE_REALTIME;
	} else {
		/*
		 * We come here if USB is not connected and the active
		 * processes are voting for Non realtime mode.
		 */
		real_time = MODE_NONREALTIME;
	}
	return real_time;
}

static void diag_create_diag_mode_ctrl_pkt(unsigned char *dest_buf,
					   uint8_t diag_id, int real_time)
{
	struct diag_ctrl_msg_diagmode diagmode;
	struct diag_ctrl_msg_diagmode_v2 diagmode_v2;
	int msg_size = sizeof(struct diag_ctrl_msg_diagmode);
	int msg_size_2 = sizeof(struct diag_ctrl_msg_diagmode_v2);

	if (!dest_buf)
		return;

	if (diag_id) {
		diagmode_v2.ctrl_pkt_id = DIAG_CTRL_MSG_DIAGMODE;
		diagmode_v2.ctrl_pkt_data_len = DIAG_MODE_PKT_LEN_V2;
		diagmode_v2.version = 2;
		diagmode_v2.sleep_vote = real_time ? 1 : 0;
		/*
		 * 0 - Disables real-time logging (to prevent
		 *	   frequent APPS wake-ups, etc.).
		 * 1 - Enable real-time logging
		 */
		diagmode_v2.real_time = real_time;
		diagmode_v2.use_nrt_values = 0;
		diagmode_v2.commit_threshold = 0;
		diagmode_v2.sleep_threshold = 0;
		diagmode_v2.sleep_time = 0;
		diagmode_v2.drain_timer_val = 0;
		diagmode_v2.event_stale_timer_val = 0;
		diagmode_v2.diag_id = diag_id;
		memcpy(dest_buf, &diagmode_v2, msg_size_2);
	} else {
		diagmode.ctrl_pkt_id = DIAG_CTRL_MSG_DIAGMODE;
		diagmode.ctrl_pkt_data_len = DIAG_MODE_PKT_LEN;
		diagmode.version = 1;
		diagmode.sleep_vote = real_time ? 1 : 0;
		/*
		 * 0 - Disables real-time logging (to prevent
		 *     frequent APPS wake-ups, etc.).
		 * 1 - Enable real-time logging
		 */
		diagmode.real_time = real_time;
		diagmode.use_nrt_values = 0;
		diagmode.commit_threshold = 0;
		diagmode.sleep_threshold = 0;
		diagmode.sleep_time = 0;
		diagmode.drain_timer_val = 0;
		diagmode.event_stale_timer_val = 0;
		memcpy(dest_buf, &diagmode, msg_size);
	}
}

void diag_update_proc_vote(uint16_t proc, uint8_t vote, int index)
{
	int i;

	mutex_lock(&driver->real_time_mutex);
	if (vote)
		driver->proc_active_mask |= proc;
	else {
		driver->proc_active_mask &= ~proc;
		if (index == ALL_PROC) {
			for (i = 0; i < DIAG_NUM_PROC; i++)
				driver->proc_rt_vote_mask[i] |= proc;
		} else {
			driver->proc_rt_vote_mask[index] |= proc;
		}
	}
	mutex_unlock(&driver->real_time_mutex);
}

void diag_update_real_time_vote(uint16_t proc, uint8_t real_time, int index)
{
	int i;

	if (index >= DIAG_NUM_PROC) {
		pr_err("diag: In %s, invalid index %d\n", __func__, index);
		return;
	}

	mutex_lock(&driver->real_time_mutex);
	if (index == ALL_PROC) {
		for (i = 0; i < DIAG_NUM_PROC; i++) {
			if (real_time)
				driver->proc_rt_vote_mask[i] |= proc;
			else
				driver->proc_rt_vote_mask[i] &= ~proc;
		}
	} else {
		if (real_time)
			driver->proc_rt_vote_mask[index] |= proc;
		else
			driver->proc_rt_vote_mask[index] &= ~proc;
	}
	mutex_unlock(&driver->real_time_mutex);
}


#ifdef CONFIG_DIAGFWD_BRIDGE_CODE
static void diag_send_diag_mode_update_remote(int token, int real_time)
{
	unsigned char *buf = NULL;
	int err = 0;
	struct diag_dci_header_t dci_header;
	int dci_header_size = sizeof(struct diag_dci_header_t);
	int msg_size = sizeof(struct diag_ctrl_msg_diagmode);
	uint32_t write_len = 0;

	if (token < 0 || token >= NUM_DCI_PROC) {
		pr_err("diag: Invalid remote device channel in %s, token: %d\n",
							__func__, token);
		return;
	}

	if (real_time != MODE_REALTIME && real_time != MODE_NONREALTIME) {
		pr_err("diag: Invalid real time value in %s, type: %d\n",
							__func__, real_time);
		return;
	}

	buf = dci_get_buffer_from_bridge(token);
	if (!buf) {
		pr_err("diag: In %s, unable to get dci buffers to write data\n",
			__func__);
		return;
	}
	/* Frame the DCI header */
	dci_header.start = CONTROL_CHAR;
	dci_header.version = 1;
	dci_header.length = msg_size + 1;
	dci_header.cmd_code = DCI_CONTROL_PKT_CODE;

	memcpy(buf + write_len, &dci_header, dci_header_size);
	write_len += dci_header_size;
	diag_create_diag_mode_ctrl_pkt(buf + write_len, 0, real_time);
	write_len += msg_size;
	*(buf + write_len) = CONTROL_CHAR; /* End Terminator */
	write_len += sizeof(uint8_t);
	err = diagfwd_bridge_write(TOKEN_TO_BRIDGE(token), buf, write_len);
	if (err != write_len) {
		pr_err("diag: cannot send nrt mode ctrl pkt, err: %d\n", err);
		diagmem_free(driver, buf, dci_ops_tbl[token].mempool);
	} else {
		driver->real_time_mode[token + 1] = real_time;
	}
}
#else
static inline void diag_send_diag_mode_update_remote(int token, int real_time)
{
}
#endif

#ifdef CONFIG_DIAG_OVER_USB
void diag_real_time_work_fn(struct work_struct *work)
{
	int temp_real_time = MODE_REALTIME, i, j;
	uint8_t send_update = 1;

	/*
	 * If any peripheral in the local processor is in either threshold or
	 * circular buffering mode, don't send the real time mode control
	 * packet.
	 */
	for (i = 0; i < NUM_PERIPHERALS; i++) {
		if (!driver->feature[i].peripheral_buffering)
			continue;
		switch (driver->buffering_mode[i].mode) {
		case DIAG_BUFFERING_MODE_THRESHOLD:
		case DIAG_BUFFERING_MODE_CIRCULAR:
			send_update = 0;
			break;
		}
	}

	mutex_lock(&driver->mode_lock);
	for (i = 0; i < DIAG_NUM_PROC; i++) {
		temp_real_time = diag_compute_real_time(i);
		if (temp_real_time == driver->real_time_mode[i]) {
			DIAG_LOG(DIAG_DEBUG_PERIPHERALS,
				"diag: did not update real time mode on proc %d, already in the req mode %d\n",
				i, temp_real_time);
			continue;
		}

		if (i == DIAG_LOCAL_PROC) {
			if (!send_update) {
				DIAG_LOG(DIAG_DEBUG_PERIPHERALS,
				"diag: cannot send real time mode pkt since one of the periperhal is in buffering mode\n");
				break;
			}
			for (j = 0; j < NUM_PERIPHERALS; j++)
				diag_send_real_time_update(j,
						temp_real_time);
		} else {
			diag_send_diag_mode_update_remote(i - 1,
							   temp_real_time);
		}
	}
	mutex_unlock(&driver->mode_lock);

	if (driver->real_time_update_busy > 0)
		driver->real_time_update_busy--;
}
#else
void diag_real_time_work_fn(struct work_struct *work)
{
	int temp_real_time = MODE_REALTIME, i, j;

	for (i = 0; i < DIAG_NUM_PROC; i++) {
		if (driver->proc_active_mask == 0) {
			/*
			 * There are no DCI or Memory Device processes.
			 * Diag should be in Real Time mode.
			 */
			temp_real_time = MODE_REALTIME;
		} else if (!(driver->proc_rt_vote_mask[i] &
						driver->proc_active_mask)) {
			/* No active process is voting for real time mode */
			temp_real_time = MODE_NONREALTIME;
		}
		if (temp_real_time == driver->real_time_mode[i]) {
			DIAG_LOG(DIAG_DEBUG_PERIPHERALS,
				"diag: did not update real time mode on proc %d, already in the req mode %d\n",
				i, temp_real_time);
			continue;
		}

		if (i == DIAG_LOCAL_PROC) {
			for (j = 0; j < NUM_PERIPHERALS; j++)
				diag_send_real_time_update(
					j, temp_real_time);
		} else {
			diag_send_diag_mode_update_remote(i - 1,
							  temp_real_time);
		}
	}

	if (driver->real_time_update_busy > 0)
		driver->real_time_update_busy--;
}
#endif

static int __diag_send_real_time_update(uint8_t peripheral, int real_time,
						uint8_t diag_id)
{
	char buf[sizeof(struct diag_ctrl_msg_diagmode_v2)];
	int msg_size = 0;
	int err = 0;

	if (peripheral >= NUM_PERIPHERALS) {
		pr_err("diag: In %s, invalid peripheral %d\n", __func__,
		       peripheral);
		return -EINVAL;
	}

	if (!driver->diagfwd_cntl[peripheral] ||
	    !driver->diagfwd_cntl[peripheral]->ch_open) {
		DIAG_LOG(DIAG_DEBUG_PERIPHERALS,
		"diag: control channel is not open, p: %d\n", peripheral);
		return err;
	}

	if (real_time != MODE_NONREALTIME && real_time != MODE_REALTIME) {
		pr_err("diag: In %s, invalid real time mode %d, peripheral: %d\n",
		       __func__, real_time, peripheral);
		return -EINVAL;
	}

	msg_size = (diag_id ? sizeof(struct diag_ctrl_msg_diagmode_v2) :
		sizeof(struct diag_ctrl_msg_diagmode));

	diag_create_diag_mode_ctrl_pkt(buf, diag_id, real_time);

	mutex_lock(&driver->diag_cntl_mutex);

	err = diagfwd_write(peripheral, TYPE_CNTL, buf, msg_size);

	if (err && err != -ENODEV) {
		pr_err("diag: In %s, unable to write, peripheral: %d, type: %d, len: %d, err: %d\n",
		       __func__, peripheral, TYPE_CNTL,
		       msg_size, err);
	} else {
		driver->real_time_mode[DIAG_LOCAL_PROC] = real_time;
	}

	mutex_unlock(&driver->diag_cntl_mutex);

	return err;
}

int diag_send_real_time_update(uint8_t peripheral, int real_time)
{
	int i;

	for (i = 0; i < NUM_PERIPHERALS; i++) {
		if (!driver->buffering_flag[i])
			continue;
		/*
		 * One of the peripherals is in buffering mode. Don't set
		 * the RT value.
		 */
		return -EINVAL;
	}

	return __diag_send_real_time_update(peripheral, real_time, 0);
}

void diag_map_pd_to_diagid(uint8_t pd, uint8_t *diag_id, int *peripheral)
{
	if (!diag_search_diagid_by_pd(pd, (void *)diag_id,
		(void *)peripheral)) {
		*diag_id = 0;
		if ((pd >= 0) && pd < NUM_PERIPHERALS)
			*peripheral = pd;
		else
			*peripheral = -EINVAL;
	}

	if (*peripheral >= 0)
		if (!driver->feature[*peripheral].pd_buffering)
			*diag_id = 0;
}

int diag_send_peripheral_buffering_mode(struct diag_buffering_mode_t *params)
{
	int err = 0;
	int mode = MODE_REALTIME;
	int peripheral = 0;
	uint8_t diag_id = 0;

	if (!params)
		return -EIO;

	diag_map_pd_to_diagid(params->peripheral,
		&diag_id, &peripheral);

	if ((peripheral < 0) ||
		peripheral >= NUM_PERIPHERALS) {
		pr_err("diag: In %s, invalid peripheral %d\n", __func__,
		       peripheral);
		return -EINVAL;
	}

	if (!driver->buffering_flag[params->peripheral]) {
		pr_err("diag: In %s, buffering flag not set for %d\n", __func__,
		       params->peripheral);
		return -EINVAL;
	}

	if (!driver->feature[peripheral].peripheral_buffering) {
		pr_err("diag: In %s, peripheral %d doesn't support buffering\n",
		       __func__, peripheral);
		return -EIO;
	}

	switch (params->mode) {
	case DIAG_BUFFERING_MODE_STREAMING:
		mode = MODE_REALTIME;
		break;
	case DIAG_BUFFERING_MODE_THRESHOLD:
	case DIAG_BUFFERING_MODE_CIRCULAR:
		mode = MODE_NONREALTIME;
		break;
	default:
		pr_err("diag: In %s, invalid tx mode %d\n", __func__,
		       params->mode);
		return -EINVAL;
	}

	if (!driver->feature[peripheral].peripheral_buffering) {
		DIAG_LOG(DIAG_DEBUG_PERIPHERALS,
			"diag: peripheral %d doesn't support buffering\n",
			 peripheral);
		driver->buffering_flag[params->peripheral] = 0;
		return -EIO;
	}

	/*
	 * Perform sanity on watermark values. These values must be
	 * checked irrespective of the buffering mode.
	 */
	if (((params->high_wm_val > DIAG_MAX_WM_VAL) ||
	     (params->low_wm_val > DIAG_MAX_WM_VAL)) ||
	    (params->low_wm_val > params->high_wm_val) ||
	    ((params->low_wm_val == params->high_wm_val) &&
	     (params->low_wm_val != DIAG_MIN_WM_VAL))) {
		pr_err("diag: In %s, invalid watermark values, high: %d, low: %d, peripheral: %d\n",
		       __func__, params->high_wm_val, params->low_wm_val,
		       params->peripheral);
		return -EINVAL;
	}

	mutex_lock(&driver->mode_lock);
	err = diag_send_buffering_tx_mode_pkt(peripheral, diag_id, params);
	if (err) {
		pr_err("diag: In %s, unable to send buffering mode packet to peripheral %d, err: %d\n",
		       __func__, peripheral, err);
		goto fail;
	}
	err = diag_send_buffering_wm_values(peripheral, diag_id, params);
	if (err) {
		pr_err("diag: In %s, unable to send buffering wm value packet to peripheral %d, err: %d\n",
		       __func__, peripheral, err);
		goto fail;
	}
	err = __diag_send_real_time_update(peripheral, mode, diag_id);
	if (err) {
		pr_err("diag: In %s, unable to send mode update to peripheral %d, mode: %d, err: %d\n",
		       __func__, peripheral, mode, err);
		goto fail;
	}
	driver->buffering_mode[params->peripheral].peripheral =
		params->peripheral;
	driver->buffering_mode[params->peripheral].mode =
		params->mode;
	driver->buffering_mode[params->peripheral].low_wm_val =
		params->low_wm_val;
	driver->buffering_mode[params->peripheral].high_wm_val =
		params->high_wm_val;
	if (params->mode == DIAG_BUFFERING_MODE_STREAMING)
		driver->buffering_flag[params->peripheral] = 0;
fail:
	mutex_unlock(&driver->mode_lock);
	return err;
}

int diag_send_stm_state(uint8_t peripheral, uint8_t stm_control_data)
{
	struct diag_ctrl_msg_stm stm_msg;
	int msg_size = sizeof(struct diag_ctrl_msg_stm);
	int err = 0;

	if (peripheral >= NUM_PERIPHERALS)
		return -EIO;

	if (!driver->diagfwd_cntl[peripheral] ||
	    !driver->diagfwd_cntl[peripheral]->ch_open) {
		DIAG_LOG(DIAG_DEBUG_PERIPHERALS,
			"diag: control channel is not open, p: %d\n",
			 peripheral);
		return -ENODEV;
	}

	if (driver->feature[peripheral].stm_support == DISABLE_STM)
		return -EINVAL;

	stm_msg.ctrl_pkt_id = 21;
	stm_msg.ctrl_pkt_data_len = 5;
	stm_msg.version = 1;
	stm_msg.control_data = stm_control_data;
	err = diagfwd_write(peripheral, TYPE_CNTL, &stm_msg, msg_size);
	if (err && err != -ENODEV) {
		pr_err("diag: In %s, unable to write to socket, peripheral: %d, type: %d, len: %d, err: %d\n",
		       __func__, peripheral, TYPE_CNTL,
		       msg_size, err);
	}

	return err;
}

int diag_send_peripheral_drain_immediate(uint8_t pd,
		uint8_t diag_id, int peripheral)
{
	int err = 0;
	struct diag_ctrl_drain_immediate ctrl_pkt;
	struct diag_ctrl_drain_immediate_v2 ctrl_pkt_v2;

	if (!driver->feature[peripheral].peripheral_buffering) {
		DIAG_LOG(DIAG_DEBUG_PERIPHERALS,
			"diag: peripheral  %d doesn't support buffering\n",
			 peripheral);
		return -EINVAL;
	}

	if (!driver->diagfwd_cntl[peripheral] ||
	    !driver->diagfwd_cntl[peripheral]->ch_open) {
		DIAG_LOG(DIAG_DEBUG_PERIPHERALS,
			"diag: control channel is not open, p: %d\n",
			 peripheral);
		return -ENODEV;
	}

	if (diag_id && driver->feature[peripheral].pd_buffering) {
		ctrl_pkt_v2.pkt_id = DIAG_CTRL_MSG_PERIPHERAL_BUF_DRAIN_IMM;
		/*
		 * The length of the ctrl pkt is size of version,
		 * diag_id and stream id
		 */
		ctrl_pkt_v2.len = sizeof(uint32_t) + (2 * sizeof(uint8_t));
		ctrl_pkt_v2.version = 2;
		ctrl_pkt_v2.diag_id = diag_id;
		ctrl_pkt_v2.stream_id = 1;
		err = diagfwd_write(peripheral, TYPE_CNTL, &ctrl_pkt_v2,
				sizeof(ctrl_pkt_v2));
		if (err && err != -ENODEV) {
			pr_err("diag: Unable to send drain immediate ctrl packet to peripheral %d, err: %d\n",
			peripheral, err);
		}
	} else {
		ctrl_pkt.pkt_id = DIAG_CTRL_MSG_PERIPHERAL_BUF_DRAIN_IMM;
		/*
		 * The length of the ctrl pkt is
		 * size of version and stream id
		 */
		ctrl_pkt.len = sizeof(uint32_t) + sizeof(uint8_t);
		ctrl_pkt.version = 1;
		ctrl_pkt.stream_id = 1;
		err = diagfwd_write(peripheral, TYPE_CNTL, &ctrl_pkt,
				sizeof(ctrl_pkt));
		if (err && err != -ENODEV) {
			pr_err("diag: Unable to send drain immediate ctrl packet to peripheral %d, err: %d\n",
			peripheral, err);
		}
	}

	return err;
}

int diag_send_buffering_tx_mode_pkt(uint8_t peripheral,
		    uint8_t diag_id, struct diag_buffering_mode_t *params)
{
	int err = 0;
	struct diag_ctrl_peripheral_tx_mode ctrl_pkt;
	struct diag_ctrl_peripheral_tx_mode_v2 ctrl_pkt_v2;

	if (!params)
		return -EIO;

	if (peripheral >= NUM_PERIPHERALS) {
		pr_err("diag: In %s, invalid peripheral %d\n", __func__,
		       peripheral);
		return -EINVAL;
	}

	if (!driver->feature[peripheral].peripheral_buffering) {
		DIAG_LOG(DIAG_DEBUG_PERIPHERALS,
			"diag: peripheral  %d doesn't support buffering\n",
			 peripheral);
		return -EINVAL;
	}

	switch (params->mode) {
	case DIAG_BUFFERING_MODE_STREAMING:
	case DIAG_BUFFERING_MODE_THRESHOLD:
	case DIAG_BUFFERING_MODE_CIRCULAR:
		break;
	default:
		pr_err("diag: In %s, invalid tx mode: %d\n", __func__,
		       params->mode);
		return -EINVAL;
	}

	if (diag_id &&
		driver->feature[peripheral].pd_buffering) {

		ctrl_pkt_v2.pkt_id = DIAG_CTRL_MSG_CONFIG_PERIPHERAL_TX_MODE;
		/*
		 * Control packet length is size of version, diag_id,
		 * stream_id and tx_mode
		 */
		ctrl_pkt_v2.len = sizeof(uint32_t) +  (3 * sizeof(uint8_t));
		ctrl_pkt_v2.version = 2;
		ctrl_pkt_v2.diag_id = diag_id;
		ctrl_pkt_v2.stream_id = 1;
		ctrl_pkt_v2.tx_mode = params->mode;

		err = diagfwd_write(peripheral, TYPE_CNTL, &ctrl_pkt_v2,
			sizeof(ctrl_pkt_v2));
		if (err && err != -ENODEV) {
			pr_err("diag: Unable to send tx_mode ctrl packet to peripheral %d, err: %d\n",
				   peripheral, err);
			goto fail;
		}
	} else {
		ctrl_pkt.pkt_id = DIAG_CTRL_MSG_CONFIG_PERIPHERAL_TX_MODE;
		/*
		 * Control packet length is size of version,
		 * stream_id and tx_mode
		 */
		ctrl_pkt.len = sizeof(uint32_t) +  (2 * sizeof(uint8_t));
		ctrl_pkt.version = 1;
		ctrl_pkt.stream_id = 1;
		ctrl_pkt.tx_mode = params->mode;

		err = diagfwd_write(peripheral, TYPE_CNTL, &ctrl_pkt,
			sizeof(ctrl_pkt));
		if (err && err != -ENODEV) {
			pr_err("diag: Unable to send tx_mode ctrl packet to peripheral %d, err: %d\n",
			       peripheral, err);
			goto fail;
		}
	}
	driver->buffering_mode[params->peripheral].mode = params->mode;

fail:
	return err;
}

int diag_send_buffering_wm_values(uint8_t peripheral,
		uint8_t diag_id, struct diag_buffering_mode_t *params)
{
	int err = 0;
	struct diag_ctrl_set_wq_val ctrl_pkt;
	struct diag_ctrl_set_wq_val_v2 ctrl_pkt_v2;

	if (!params)
		return -EIO;

	if (peripheral >= NUM_PERIPHERALS) {
		pr_err("diag: In %s, invalid peripheral %d\n", __func__,
		       peripheral);
		return -EINVAL;
	}

	if (!driver->feature[peripheral].peripheral_buffering) {
		DIAG_LOG(DIAG_DEBUG_PERIPHERALS,
			"diag: peripheral  %d doesn't support buffering\n",
			 peripheral);
		return -EINVAL;
	}

	if (!driver->diagfwd_cntl[peripheral] ||
	    !driver->diagfwd_cntl[peripheral]->ch_open) {
		DIAG_LOG(DIAG_DEBUG_PERIPHERALS,
			"diag: control channel is not open, p: %d\n",
			 peripheral);
		return -ENODEV;
	}

	switch (params->mode) {
	case DIAG_BUFFERING_MODE_STREAMING:
	case DIAG_BUFFERING_MODE_THRESHOLD:
	case DIAG_BUFFERING_MODE_CIRCULAR:
		break;
	default:
		pr_err("diag: In %s, invalid tx mode: %d\n", __func__,
		       params->mode);
		return -EINVAL;
	}

	if (diag_id &&
		driver->feature[peripheral].pd_buffering) {
		ctrl_pkt_v2.pkt_id = DIAG_CTRL_MSG_CONFIG_PERIPHERAL_WMQ_VAL;
		/*
		 * Control packet length is size of version, diag_id,
		 * stream_id and wmq values
		 */
		ctrl_pkt_v2.len = sizeof(uint32_t) + (4 * sizeof(uint8_t));
		ctrl_pkt_v2.version = 2;
		ctrl_pkt_v2.diag_id = diag_id;
		ctrl_pkt_v2.stream_id = 1;
		ctrl_pkt_v2.high_wm_val = params->high_wm_val;
		ctrl_pkt_v2.low_wm_val = params->low_wm_val;

		err = diagfwd_write(peripheral, TYPE_CNTL, &ctrl_pkt_v2,
					sizeof(ctrl_pkt_v2));
		if (err && err != -ENODEV) {
			pr_err("diag: Unable to send watermark values to peripheral %d, err: %d\n",
				   peripheral, err);
		}
	} else {
		ctrl_pkt.pkt_id = DIAG_CTRL_MSG_CONFIG_PERIPHERAL_WMQ_VAL;
		/*
		 * Control packet length is size of version,
		 * stream_id and wmq values
		 */
		ctrl_pkt.len = sizeof(uint32_t) + (3 * sizeof(uint8_t));
		ctrl_pkt.version = 1;
		ctrl_pkt.stream_id = 1;
		ctrl_pkt.high_wm_val = params->high_wm_val;
		ctrl_pkt.low_wm_val = params->low_wm_val;

		err = diagfwd_write(peripheral, TYPE_CNTL, &ctrl_pkt,
				    sizeof(ctrl_pkt));
		if (err && err != -ENODEV) {
			pr_err("diag: Unable to send watermark values to peripheral %d, err: %d\n",
			       peripheral, err);
		}
	}
	return err;
}

int diagfwd_cntl_init(void)
{
	uint8_t peripheral = 0;

	reg_dirty = 0;
	driver->polling_reg_flag = 0;
	driver->log_on_demand_support = 1;
	driver->stm_peripheral = 0;
	driver->close_transport = 0;
	for (peripheral = 0; peripheral < NUM_PERIPHERALS; peripheral++)
		driver->buffering_flag[peripheral] = 0;

	mutex_init(&driver->cntl_lock);
	INIT_WORK(&(driver->stm_update_work), diag_stm_update_work_fn);
	INIT_WORK(&(driver->mask_update_work), diag_mask_update_work_fn);
	INIT_WORK(&(driver->close_transport_work),
		  diag_close_transport_work_fn);

	driver->cntl_wq = create_singlethread_workqueue("diag_cntl_wq");
	if (!driver->cntl_wq)
		return -ENOMEM;

	return 0;
}

void diagfwd_cntl_channel_init(void)
{
	uint8_t peripheral;

	for (peripheral = 0; peripheral < NUM_PERIPHERALS; peripheral++) {
		diagfwd_early_open(peripheral);
		diagfwd_open(peripheral, TYPE_CNTL);
	}
}

void diagfwd_cntl_exit(void)
{
	if (driver->cntl_wq)
		destroy_workqueue(driver->cntl_wq);
}
