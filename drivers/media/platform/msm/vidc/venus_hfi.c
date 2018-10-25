/* Copyright (c) 2012-2018, The Linux Foundation. All rights reserved.
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

#include <asm/dma-iommu.h>
#include <asm/memory.h>
#include <linux/clk/qcom.h>
#include <linux/coresight-stm.h>
#include <linux/delay.h>
#include <linux/devfreq.h>
#include <linux/hash.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/iommu.h>
#include <linux/iopoll.h>
#include <linux/of.h>
#include <linux/pm_qos.h>
#include <linux/regulator/consumer.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
#include <linux/platform_device.h>
#include <linux/soc/qcom/llcc-qcom.h>
#include <soc/qcom/scm.h>
#include <soc/qcom/smem.h>
#include <soc/qcom/subsystem_restart.h>
#include "hfi_packetization.h"
#include "msm_vidc_debug.h"
#include "venus_hfi.h"
#include "vidc_hfi_io.h"

#define FIRMWARE_SIZE			0X00A00000
#define REG_ADDR_OFFSET_BITMASK	0x000FFFFF
#define QDSS_IOVA_START 0x80001000

static struct hal_device_data hal_ctxt;

#define TZBSP_MEM_PROTECT_VIDEO_VAR 0x8
struct tzbsp_memprot {
	u32 cp_start;
	u32 cp_size;
	u32 cp_nonpixel_start;
	u32 cp_nonpixel_size;
};

struct tzbsp_resp {
	int ret;
};

#define TZBSP_VIDEO_SET_STATE 0xa

/* Poll interval in uS */
#define POLL_INTERVAL_US 50

#define VENUS_AXI_HALT_ACK_TIMEOUT_US		500000


enum tzbsp_video_state {
	TZBSP_VIDEO_STATE_SUSPEND = 0,
	TZBSP_VIDEO_STATE_RESUME = 1,
	TZBSP_VIDEO_STATE_RESTORE_THRESHOLD = 2,
};

struct tzbsp_video_set_state_req {
	u32 state; /* should be tzbsp_video_state enum value */
	u32 spare; /* reserved for future, should be zero */
};

const struct msm_vidc_gov_data DEFAULT_BUS_VOTE = {
	.data = NULL,
	.data_count = 0,
};

const int max_packets = 1000;

static void venus_hfi_pm_handler(struct work_struct *work);
static DECLARE_DELAYED_WORK(venus_hfi_pm_work, venus_hfi_pm_handler);
static inline int __resume(struct venus_hfi_device *device);
static inline int __suspend(struct venus_hfi_device *device);
static int __disable_regulators(struct venus_hfi_device *device);
static int __enable_regulators(struct venus_hfi_device *device);
static inline int __prepare_enable_clks(struct venus_hfi_device *device);
static inline void __disable_unprepare_clks(struct venus_hfi_device *device);
static void __flush_debug_queue(struct venus_hfi_device *device, u8 *packet);
static int __initialize_packetization(struct venus_hfi_device *device);
static struct hal_session *__get_session(struct venus_hfi_device *device,
		u32 session_id);
static bool __is_session_valid(struct venus_hfi_device *device,
		struct hal_session *session, const char *func);
static int __set_clocks(struct venus_hfi_device *device, u32 freq);
static int __iface_cmdq_write(struct venus_hfi_device *device,
					void *pkt);
static int __load_fw(struct venus_hfi_device *device);
static void __unload_fw(struct venus_hfi_device *device);
static int __tzbsp_set_video_state(enum tzbsp_video_state state);
static int __enable_subcaches(struct venus_hfi_device *device);
static int __set_subcaches(struct venus_hfi_device *device);
static int __release_subcaches(struct venus_hfi_device *device);
static int __disable_subcaches(struct venus_hfi_device *device);
static int venus_hfi_noc_error_info(void *dev);

/**
 * Utility function to enforce some of our assumptions.  Spam calls to this
 * in hotspots in code to double check some of the assumptions that we hold.
 */
static inline void __strict_check(struct venus_hfi_device *device)
{
	msm_vidc_res_handle_fatal_hw_error(device->res,
		!mutex_is_locked(&device->lock));
}

static inline void __set_state(struct venus_hfi_device *device,
		enum venus_hfi_state state)
{
	device->state = state;
}

static inline bool __core_in_valid_state(struct venus_hfi_device *device)
{
	return device->state != VENUS_STATE_DEINIT;
}

static inline bool is_sys_cache_present(struct venus_hfi_device *device)
{
	return device->res->sys_cache_present;
}

static void __dump_packet(u8 *packet, enum vidc_msg_prio log_level)
{
	u32 c = 0, packet_size = *(u32 *)packet;
	const int row_size = 32;
	/*
	 * row must contain enough for 0xdeadbaad * 8 to be converted into
	 * "de ad ba ab " * 8 + '\0'
	 */
	char row[3 * row_size];

	for (c = 0; c * row_size < packet_size; ++c) {
		int bytes_to_read = ((c + 1) * row_size > packet_size) ?
			packet_size % row_size : row_size;
		hex_dump_to_buffer(packet + c * row_size, bytes_to_read,
				row_size, 4, row, sizeof(row), false);
		dprintk(log_level, "%s\n", row);
	}
}

static void __sim_modify_cmd_packet(u8 *packet, struct venus_hfi_device *device)
{
	struct hfi_cmd_sys_session_init_packet *sys_init;
	struct hal_session *session = NULL;
	u8 i;
	phys_addr_t fw_bias = 0;

	if (!device || !packet) {
		dprintk(VIDC_ERR, "Invalid Param\n");
		return;
	} else if (!device->hal_data->firmware_base
			|| is_iommu_present(device->res)) {
		return;
	}

	fw_bias = device->hal_data->firmware_base;
	sys_init = (struct hfi_cmd_sys_session_init_packet *)packet;

	session = __get_session(device, sys_init->session_id);
	if (!session) {
		dprintk(VIDC_DBG, "%s :Invalid session id: %x\n",
				__func__, sys_init->session_id);
		return;
	}

	switch (sys_init->packet_type) {
	case HFI_CMD_SESSION_EMPTY_BUFFER:
		if (session->is_decoder) {
			struct hfi_cmd_session_empty_buffer_compressed_packet
			*pkt = (struct
			hfi_cmd_session_empty_buffer_compressed_packet
			*) packet;
			pkt->packet_buffer -= fw_bias;
		} else {
			struct
			hfi_cmd_session_empty_buffer_uncompressed_plane0_packet
			*pkt = (struct
			hfi_cmd_session_empty_buffer_uncompressed_plane0_packet
			*) packet;
			pkt->packet_buffer -= fw_bias;
		}
		break;
	case HFI_CMD_SESSION_FILL_BUFFER:
	{
		struct hfi_cmd_session_fill_buffer_packet *pkt =
			(struct hfi_cmd_session_fill_buffer_packet *)packet;
		pkt->packet_buffer -= fw_bias;
		break;
	}
	case HFI_CMD_SESSION_SET_BUFFERS:
	{
		struct hfi_cmd_session_set_buffers_packet *pkt =
			(struct hfi_cmd_session_set_buffers_packet *)packet;
		if (pkt->buffer_type == HFI_BUFFER_OUTPUT ||
			pkt->buffer_type == HFI_BUFFER_OUTPUT2) {
			struct hfi_buffer_info *buff;

			buff = (struct hfi_buffer_info *) pkt->rg_buffer_info;
			buff->buffer_addr -= fw_bias;
			if (buff->extra_data_addr >= fw_bias)
				buff->extra_data_addr -= fw_bias;
		} else {
			for (i = 0; i < pkt->num_buffers; i++)
				pkt->rg_buffer_info[i] -= fw_bias;
		}
		break;
	}
	case HFI_CMD_SESSION_RELEASE_BUFFERS:
	{
		struct hfi_cmd_session_release_buffer_packet *pkt =
			(struct hfi_cmd_session_release_buffer_packet *)packet;

		if (pkt->buffer_type == HFI_BUFFER_OUTPUT ||
			pkt->buffer_type == HFI_BUFFER_OUTPUT2) {
			struct hfi_buffer_info *buff;

			buff = (struct hfi_buffer_info *) pkt->rg_buffer_info;
			buff->buffer_addr -= fw_bias;
			buff->extra_data_addr -= fw_bias;
		} else {
			for (i = 0; i < pkt->num_buffers; i++)
				pkt->rg_buffer_info[i] -= fw_bias;
		}
		break;
	}
	default:
		break;
	}
}

static int __acquire_regulator(struct regulator_info *rinfo,
				struct venus_hfi_device *device)
{
	int rc = 0;

	if (rinfo->has_hw_power_collapse) {
		rc = regulator_set_mode(rinfo->regulator,
				REGULATOR_MODE_NORMAL);
		if (rc) {
			/*
			 * This is somewhat fatal, but nothing we can do
			 * about it. We can't disable the regulator w/o
			 * getting it back under s/w control
			 */
			dprintk(VIDC_WARN,
				"Failed to acquire regulator control: %s\n",
					rinfo->name);
		} else {

			dprintk(VIDC_DBG,
					"Acquire regulator control from HW: %s\n",
					rinfo->name);

		}
	}

	if (!regulator_is_enabled(rinfo->regulator)) {
		dprintk(VIDC_WARN, "Regulator is not enabled %s\n",
			rinfo->name);
		msm_vidc_res_handle_fatal_hw_error(device->res, true);
	}

	return rc;
}

static int __hand_off_regulator(struct regulator_info *rinfo)
{
	int rc = 0;

	if (rinfo->has_hw_power_collapse) {
		rc = regulator_set_mode(rinfo->regulator,
				REGULATOR_MODE_FAST);
		if (rc) {
			dprintk(VIDC_WARN,
				"Failed to hand off regulator control: %s\n",
					rinfo->name);
		} else {
			dprintk(VIDC_DBG,
					"Hand off regulator control to HW: %s\n",
					rinfo->name);
		}
	}

	return rc;
}

static int __hand_off_regulators(struct venus_hfi_device *device)
{
	struct regulator_info *rinfo;
	int rc = 0, c = 0;

	venus_hfi_for_each_regulator(device, rinfo) {
		rc = __hand_off_regulator(rinfo);
		/*
		 * If one regulator hand off failed, driver should take
		 * the control for other regulators back.
		 */
		if (rc)
			goto err_reg_handoff_failed;
		c++;
	}

	return rc;
err_reg_handoff_failed:
	venus_hfi_for_each_regulator_reverse_continue(device, rinfo, c)
		__acquire_regulator(rinfo, device);

	return rc;
}

static int __write_queue(struct vidc_iface_q_info *qinfo, u8 *packet,
		bool *rx_req_is_set)
{
	struct hfi_queue_header *queue;
	u32 packet_size_in_words, new_write_idx;
	u32 empty_space, read_idx;
	u32 *write_ptr;

	if (!qinfo || !packet) {
		dprintk(VIDC_ERR, "Invalid Params\n");
		return -EINVAL;
	} else if (!qinfo->q_array.align_virtual_addr) {
		dprintk(VIDC_WARN, "Queues have already been freed\n");
		return -EINVAL;
	}

	queue = (struct hfi_queue_header *) qinfo->q_hdr;
	if (!queue) {
		dprintk(VIDC_ERR, "queue not present\n");
		return -ENOENT;
	}

	if (msm_vidc_debug & VIDC_PKT) {
		dprintk(VIDC_PKT, "%s: %pK\n", __func__, qinfo);
		__dump_packet(packet, VIDC_PKT);
	}

	packet_size_in_words = (*(u32 *)packet) >> 2;
	if (!packet_size_in_words) {
		dprintk(VIDC_ERR, "Zero packet size\n");
		return -ENODATA;
	}

	read_idx = queue->qhdr_read_idx;

	empty_space = (queue->qhdr_write_idx >=  read_idx) ?
		(queue->qhdr_q_size - (queue->qhdr_write_idx -  read_idx)) :
		(read_idx - queue->qhdr_write_idx);
	if (empty_space <= packet_size_in_words) {
		queue->qhdr_tx_req =  1;
		dprintk(VIDC_ERR, "Insufficient size (%d) to write (%d)\n",
					  empty_space, packet_size_in_words);
		return -ENOTEMPTY;
	}

	queue->qhdr_tx_req =  0;

	new_write_idx = (queue->qhdr_write_idx + packet_size_in_words);
	write_ptr = (u32 *)((qinfo->q_array.align_virtual_addr) +
		(queue->qhdr_write_idx << 2));
	if (new_write_idx < queue->qhdr_q_size) {
		memcpy(write_ptr, packet, packet_size_in_words << 2);
	} else {
		new_write_idx -= queue->qhdr_q_size;
		memcpy(write_ptr, packet, (packet_size_in_words -
			new_write_idx) << 2);
		memcpy((void *)qinfo->q_array.align_virtual_addr,
			packet + ((packet_size_in_words - new_write_idx) << 2),
			new_write_idx  << 2);
	}

	/*
	 * Memory barrier to make sure packet is written before updating the
	 * write index
	 */
	mb();
	queue->qhdr_write_idx = new_write_idx;
	if (rx_req_is_set)
		*rx_req_is_set = queue->qhdr_rx_req == 1;
	/*
	 * Memory barrier to make sure write index is updated before an
	 * interrupt is raised on venus.
	 */
	mb();
	return 0;
}

static void __hal_sim_modify_msg_packet(u8 *packet,
					struct venus_hfi_device *device)
{
	struct hfi_msg_sys_session_init_done_packet *sys_idle;
	struct hal_session *session = NULL;
	phys_addr_t fw_bias = 0;

	if (!device || !packet) {
		dprintk(VIDC_ERR, "Invalid Param\n");
		return;
	} else if (!device->hal_data->firmware_base
			|| is_iommu_present(device->res)) {
		return;
	}

	fw_bias = device->hal_data->firmware_base;
	sys_idle = (struct hfi_msg_sys_session_init_done_packet *)packet;
	session = __get_session(device, sys_idle->session_id);

	if (!session) {
		dprintk(VIDC_DBG, "%s: Invalid session id: %x\n",
				__func__, sys_idle->session_id);
		return;
	}

	switch (sys_idle->packet_type) {
	case HFI_MSG_SESSION_FILL_BUFFER_DONE:
		if (session->is_decoder) {
			struct
			hfi_msg_session_fbd_uncompressed_plane0_packet
			*pkt_uc = (struct
			hfi_msg_session_fbd_uncompressed_plane0_packet
			*) packet;
			pkt_uc->packet_buffer += fw_bias;
		} else {
			struct
			hfi_msg_session_fill_buffer_done_compressed_packet
			*pkt = (struct
			hfi_msg_session_fill_buffer_done_compressed_packet
			*) packet;
			pkt->packet_buffer += fw_bias;
		}
		break;
	case HFI_MSG_SESSION_EMPTY_BUFFER_DONE:
	{
		struct hfi_msg_session_empty_buffer_done_packet *pkt =
		(struct hfi_msg_session_empty_buffer_done_packet *)packet;
		pkt->packet_buffer += fw_bias;
		break;
	}
	case HFI_MSG_SESSION_GET_SEQUENCE_HEADER_DONE:
	{
		struct
		hfi_msg_session_get_sequence_header_done_packet
		*pkt =
		(struct hfi_msg_session_get_sequence_header_done_packet *)
		packet;
		pkt->sequence_header += fw_bias;
		break;
	}
	default:
		break;
	}
}

static int __read_queue(struct vidc_iface_q_info *qinfo, u8 *packet,
		u32 *pb_tx_req_is_set)
{
	struct hfi_queue_header *queue;
	u32 packet_size_in_words, new_read_idx;
	u32 *read_ptr;
	u32 receive_request = 0;
		int rc = 0;

	if (!qinfo || !packet || !pb_tx_req_is_set) {
		dprintk(VIDC_ERR, "Invalid Params\n");
		return -EINVAL;
	} else if (!qinfo->q_array.align_virtual_addr) {
		dprintk(VIDC_WARN, "Queues have already been freed\n");
		return -EINVAL;
	}

	/*
	 * Memory barrier to make sure data is valid before
	 *reading it
	 */
	mb();
	queue = (struct hfi_queue_header *) qinfo->q_hdr;

	if (!queue) {
		dprintk(VIDC_ERR, "Queue memory is not allocated\n");
		return -ENOMEM;
	}

	/*
	 * Do not set receive request for debug queue, if set,
	 * Venus generates interrupt for debug messages even
	 * when there is no response message available.
	 * In general debug queue will not become full as it
	 * is being emptied out for every interrupt from Venus.
	 * Venus will anyway generates interrupt if it is full.
	 */
	if (queue->qhdr_type & HFI_Q_ID_CTRL_TO_HOST_MSG_Q)
		receive_request = 1;

	if (queue->qhdr_read_idx == queue->qhdr_write_idx) {
		queue->qhdr_rx_req = receive_request;
		/*
		 * mb() to ensure qhdr is updated in main memory
		 * so that venus reads the updated header values
		 */
		mb();
		*pb_tx_req_is_set = 0;
		dprintk(VIDC_DBG,
			"%s queue is empty, rx_req = %u, tx_req = %u, read_idx = %u\n",
			receive_request ? "message" : "debug",
			queue->qhdr_rx_req, queue->qhdr_tx_req,
			queue->qhdr_read_idx);
		return -ENODATA;
	}

	read_ptr = (u32 *)((qinfo->q_array.align_virtual_addr) +
				(queue->qhdr_read_idx << 2));
	packet_size_in_words = (*read_ptr) >> 2;
	if (!packet_size_in_words) {
		dprintk(VIDC_ERR, "Zero packet size\n");
		return -ENODATA;
	}

	new_read_idx = queue->qhdr_read_idx + packet_size_in_words;
	if (((packet_size_in_words << 2) <= VIDC_IFACEQ_VAR_HUGE_PKT_SIZE)
			&& queue->qhdr_read_idx <= queue->qhdr_q_size) {
		if (new_read_idx < queue->qhdr_q_size) {
			memcpy(packet, read_ptr,
					packet_size_in_words << 2);
		} else {
			new_read_idx -= queue->qhdr_q_size;
			memcpy(packet, read_ptr,
			(packet_size_in_words - new_read_idx) << 2);
			memcpy(packet + ((packet_size_in_words -
					new_read_idx) << 2),
					(u8 *)qinfo->q_array.align_virtual_addr,
					new_read_idx << 2);
		}
	} else {
		dprintk(VIDC_WARN,
			"BAD packet received, read_idx: %#x, pkt_size: %d\n",
			queue->qhdr_read_idx, packet_size_in_words << 2);
		dprintk(VIDC_WARN, "Dropping this packet\n");
		new_read_idx = queue->qhdr_write_idx;
		rc = -ENODATA;
	}

	queue->qhdr_read_idx = new_read_idx;

	if (queue->qhdr_read_idx != queue->qhdr_write_idx)
		queue->qhdr_rx_req = 0;
	else
		queue->qhdr_rx_req = receive_request;
	/*
	 * mb() to ensure qhdr is updated in main memory
	 * so that venus reads the updated header values
	 */
	mb();

	*pb_tx_req_is_set = (queue->qhdr_tx_req == 1) ? 1 : 0;

	if ((msm_vidc_debug & VIDC_PKT) &&
		!(queue->qhdr_type & HFI_Q_ID_CTRL_TO_HOST_DEBUG_Q)) {
		dprintk(VIDC_PKT, "%s: %pK\n", __func__, qinfo);
		__dump_packet(packet, VIDC_PKT);
	}

	return rc;
}

static int __smem_alloc(struct venus_hfi_device *dev,
			struct vidc_mem_addr *mem, u32 size, u32 align,
			u32 flags, u32 usage)
{
	struct msm_smem *alloc = &mem->mem_data;
	int rc = 0;

	if (!dev || !dev->hal_client || !mem || !size) {
		dprintk(VIDC_ERR, "Invalid Params\n");
		return -EINVAL;
	}

	dprintk(VIDC_INFO, "start to alloc size: %d, flags: %d\n", size, flags);
	rc = msm_smem_alloc(dev->hal_client, size, align, flags,
		usage, 1, alloc);
	if (rc) {
		dprintk(VIDC_ERR, "Alloc failed\n");
		rc = -ENOMEM;
		goto fail_smem_alloc;
	}

	dprintk(VIDC_DBG, "__smem_alloc: ptr = %pK, size = %d\n",
			alloc->kvaddr, size);
	rc = msm_smem_cache_operations(dev->hal_client, alloc->handle, 0,
			alloc->size, SMEM_CACHE_CLEAN);
	if (rc) {
		dprintk(VIDC_WARN, "Failed to clean cache\n");
	}

	mem->mem_size = alloc->size;
	mem->align_virtual_addr = alloc->kvaddr;
	mem->align_device_addr = alloc->device_addr;

	return rc;
fail_smem_alloc:
	return rc;
}

static void __smem_free(struct venus_hfi_device *dev, struct msm_smem *mem)
{
	if (!dev || !mem) {
		dprintk(VIDC_ERR, "invalid param %pK %pK\n", dev, mem);
		return;
	}

	msm_smem_free(dev->hal_client, mem);
}

static void __write_register(struct venus_hfi_device *device,
		u32 reg, u32 value)
{
	u32 hwiosymaddr = reg;
	u8 *base_addr;

	if (!device) {
		dprintk(VIDC_ERR, "Invalid params: %pK\n", device);
		return;
	}

	__strict_check(device);

	if (!device->power_enabled) {
		dprintk(VIDC_WARN,
			"HFI Write register failed : Power is OFF\n");
		msm_vidc_res_handle_fatal_hw_error(device->res, true);
		return;
	}

	base_addr = device->hal_data->register_base;
	dprintk(VIDC_DBG, "Base addr: %pK, written to: %#x, Value: %#x...\n",
		base_addr, hwiosymaddr, value);
	base_addr += hwiosymaddr;
	writel_relaxed(value, base_addr);

	/*
	 * Memory barrier to make sure value is written into the register.
	 */
	wmb();
}

static int __read_register(struct venus_hfi_device *device, u32 reg)
{
	int rc = 0;
	u8 *base_addr;

	if (!device) {
		dprintk(VIDC_ERR, "Invalid params: %pK\n", device);
		return -EINVAL;
	}

	__strict_check(device);

	if (!device->power_enabled) {
		dprintk(VIDC_WARN,
			"HFI Read register failed : Power is OFF\n");
		msm_vidc_res_handle_fatal_hw_error(device->res, true);
		return -EINVAL;
	}

	base_addr = device->hal_data->register_base;

	rc = readl_relaxed(base_addr + reg);
	/*
	 * Memory barrier to make sure value is read correctly from the
	 * register.
	 */
	rmb();
	dprintk(VIDC_DBG, "Base addr: %pK, read from: %#x, value: %#x...\n",
		base_addr, reg, rc);

	return rc;
}

static void __set_registers(struct venus_hfi_device *device)
{
	struct reg_set *reg_set;
	int i;

	if (!device->res) {
		dprintk(VIDC_ERR,
			"device resources null, cannot set registers\n");
		return;
	}

	reg_set = &device->res->reg_set;
	for (i = 0; i < reg_set->count; i++) {
		__write_register(device, reg_set->reg_tbl[i].reg,
				reg_set->reg_tbl[i].value);
	}
}

/*
 * The existence of this function is a hack for 8996 (or certain Venus versions)
 * to overcome a hardware bug.  Whenever the GDSCs momentarily power collapse
 * (after calling __hand_off_regulators()), the values of the threshold
 * registers (typically programmed by TZ) are incorrectly reset.  As a result
 * reprogram these registers at certain agreed upon points.
 */
static void __set_threshold_registers(struct venus_hfi_device *device)
{
	u32 version = __read_register(device, VIDC_WRAPPER_HW_VERSION);

	version &= ~GENMASK(15, 0);
	if (version != (0x3 << 28 | 0x43 << 16))
		return;

	if (__tzbsp_set_video_state(TZBSP_VIDEO_STATE_RESTORE_THRESHOLD))
		dprintk(VIDC_ERR, "Failed to restore threshold values\n");
}

static void __iommu_detach(struct venus_hfi_device *device)
{
	struct context_bank_info *cb;

	if (!device || !device->res) {
		dprintk(VIDC_ERR, "Invalid parameter: %pK\n", device);
		return;
	}

	list_for_each_entry(cb, &device->res->context_banks, list) {
		if (cb->dev)
			arm_iommu_detach_device(cb->dev);
		if (cb->mapping)
			arm_iommu_release_mapping(cb->mapping);
	}
}

static int __devfreq_target(struct device *devfreq_dev,
		unsigned long *freq, u32 flags)
{
	int rc = 0;
	uint64_t ab = 0;
	struct bus_info *bus = NULL, *temp = NULL;
	struct venus_hfi_device *device = dev_get_drvdata(devfreq_dev);

	venus_hfi_for_each_bus(device, temp) {
		if (temp->dev == devfreq_dev) {
			bus = temp;
			break;
		}
	}

	if (!bus) {
		rc = -EBADHANDLE;
		goto err_unknown_device;
	}

	/*
	 * Clamp for all non zero frequencies. This clamp is necessary to stop
	 * devfreq driver from spamming - Couldn't update frequency - logs, if
	 * the scaled ab value is not part of the frequency table.
	 */
	if (*freq)
		*freq = clamp_t(typeof(*freq), *freq, bus->range[0],
				bus->range[1]);

	/* we expect governors to provide values in kBps form, convert to Bps */
	ab = *freq * 1000;
	rc = msm_bus_scale_update_bw(bus->client, ab, 0);
	if (rc) {
		dprintk(VIDC_ERR, "Failed voting bus %s to ab %llu\n: %d",
				bus->name, ab, rc);
		goto err_unknown_device;
	}

	dprintk(VIDC_PROF, "Voting bus %s to ab %llu\n", bus->name, ab);

	return 0;
err_unknown_device:
	return rc;
}

static int __devfreq_get_status(struct device *devfreq_dev,
		struct devfreq_dev_status *stat)
{
	int rc = 0;
	struct bus_info *bus = NULL, *temp = NULL;
	struct venus_hfi_device *device = dev_get_drvdata(devfreq_dev);

	venus_hfi_for_each_bus(device, temp) {
		if (temp->dev == devfreq_dev) {
			bus = temp;
			break;
		}
	}

	if (!bus) {
		rc = -EBADHANDLE;
		goto err_unknown_device;
	}

	*stat = (struct devfreq_dev_status) {
		.private_data = &device->bus_vote,
		/*
		 * Put in dummy place holder values for upstream govs, our
		 * custom gov only needs .private_data.  We should fill this in
		 * properly if we can actually measure busy_time accurately
		 * (which we can't at the moment)
		 */
		.total_time = 1,
		.busy_time = 1,
		.current_frequency = 0,
	};

err_unknown_device:
	return rc;
}

static int __unvote_buses(struct venus_hfi_device *device)
{
	int rc = 0;
	struct bus_info *bus = NULL;

	kfree(device->bus_vote.data);
	device->bus_vote.data = NULL;
	device->bus_vote.data_count = 0;

	venus_hfi_for_each_bus(device, bus) {
		unsigned long zero = 0;

		if (!bus->is_prfm_gov_used)
			rc = devfreq_suspend_device(bus->devfreq);
		else
			rc = __devfreq_target(bus->dev, &zero, 0);

		if (rc)
			goto err_unknown_device;
	}

err_unknown_device:
	return rc;
}

static int __vote_buses(struct venus_hfi_device *device,
		struct vidc_bus_vote_data *data, int num_data)
{
	int rc = 0;
	struct bus_info *bus = NULL;
	struct vidc_bus_vote_data *new_data = NULL;

	if (!num_data) {
		dprintk(VIDC_DBG, "No vote data available\n");
		goto no_data_count;
	} else if (!data) {
		dprintk(VIDC_ERR, "Invalid voting data\n");
		return -EINVAL;
	}

	new_data = kmemdup(data, num_data * sizeof(*new_data), GFP_KERNEL);
	if (!new_data) {
		dprintk(VIDC_ERR, "Can't alloc memory to cache bus votes\n");
		rc = -ENOMEM;
		goto err_no_mem;
	}

no_data_count:
	kfree(device->bus_vote.data);
	device->bus_vote.data = new_data;
	device->bus_vote.data_count = num_data;

	venus_hfi_for_each_bus(device, bus) {
		if (bus && bus->devfreq) {
			if (!bus->is_prfm_gov_used) {
				rc = devfreq_resume_device(bus->devfreq);
				if (rc)
					goto err_no_mem;
			} else {
				bus->devfreq->nb.notifier_call(
					&bus->devfreq->nb, 0, NULL);
			}
		}
	}

err_no_mem:
	return rc;
}

static int venus_hfi_vote_buses(void *dev, struct vidc_bus_vote_data *d, int n)
{
	int rc = 0;
	struct venus_hfi_device *device = dev;

	if (!device)
		return -EINVAL;

	mutex_lock(&device->lock);
	rc = __vote_buses(device, d, n);
	mutex_unlock(&device->lock);

	return rc;

}
static int __core_set_resource(struct venus_hfi_device *device,
		struct vidc_resource_hdr *resource_hdr, void *resource_value)
{
	struct hfi_cmd_sys_set_resource_packet *pkt;
	u8 packet[VIDC_IFACEQ_VAR_SMALL_PKT_SIZE];
	int rc = 0;

	if (!device || !resource_hdr || !resource_value) {
		dprintk(VIDC_ERR, "set_res: Invalid Params\n");
		return -EINVAL;
	}

	pkt = (struct hfi_cmd_sys_set_resource_packet *) packet;

	rc = call_hfi_pkt_op(device, sys_set_resource,
			pkt, resource_hdr, resource_value);
	if (rc) {
		dprintk(VIDC_ERR, "set_res: failed to create packet\n");
		goto err_create_pkt;
	}

	rc = __iface_cmdq_write(device, pkt);
	if (rc)
		rc = -ENOTEMPTY;

err_create_pkt:
	return rc;
}

static int __core_release_resource(struct venus_hfi_device *device,
		struct vidc_resource_hdr *resource_hdr)
{
	struct hfi_cmd_sys_release_resource_packet *pkt;
	u8 packet[VIDC_IFACEQ_VAR_SMALL_PKT_SIZE];
	int rc = 0;

	if (!device || !resource_hdr) {
		dprintk(VIDC_ERR, "release_res: Invalid Params\n");
		return -EINVAL;
	}

	pkt = (struct hfi_cmd_sys_release_resource_packet *) packet;

	rc = call_hfi_pkt_op(device, sys_release_resource,
			pkt, resource_hdr);

	if (rc) {
		dprintk(VIDC_ERR, "release_res: failed to create packet\n");
		goto err_create_pkt;
	}

	rc = __iface_cmdq_write(device, pkt);
	if (rc)
		rc = -ENOTEMPTY;

err_create_pkt:
	return rc;
}

static int __tzbsp_set_video_state(enum tzbsp_video_state state)
{
	struct tzbsp_video_set_state_req cmd = {0};
	int tzbsp_rsp = 0;
	int rc = 0;
	struct scm_desc desc = {0};

	desc.args[0] = cmd.state = state;
	desc.args[1] = cmd.spare = 0;
	desc.arginfo = SCM_ARGS(2);

	if (!is_scm_armv8()) {
		rc = scm_call(SCM_SVC_BOOT, TZBSP_VIDEO_SET_STATE, &cmd,
				sizeof(cmd), &tzbsp_rsp, sizeof(tzbsp_rsp));
	} else {
		rc = scm_call2(SCM_SIP_FNID(SCM_SVC_BOOT,
				TZBSP_VIDEO_SET_STATE), &desc);
		tzbsp_rsp = desc.ret[0];
	}

	if (rc) {
		dprintk(VIDC_ERR, "Failed scm_call %d\n", rc);
		return rc;
	}

	dprintk(VIDC_DBG, "Set state %d, resp %d\n", state, tzbsp_rsp);
	if (tzbsp_rsp) {
		dprintk(VIDC_ERR,
				"Failed to set video core state to suspend: %d\n",
				tzbsp_rsp);
		return -EINVAL;
	}

	return 0;
}

static inline int __boot_firmware(struct venus_hfi_device *device)
{
	int rc = 0;
	u32 ctrl_status = 0, count = 0, max_tries = 1000;

	__write_register(device, VIDC_CTRL_INIT, 0x1);
	while (!ctrl_status && count < max_tries) {
		ctrl_status = __read_register(device, VIDC_CPU_CS_SCIACMDARG0);
		if ((ctrl_status & 0xFE) == 0x4) {
			dprintk(VIDC_ERR, "invalid setting for UC_REGION\n");
			break;
		}

		usleep_range(50, 100);
		count++;
	}

	if (count >= max_tries) {
		dprintk(VIDC_ERR, "Error booting up vidc firmware\n");
		rc = -ETIME;
	}
	return rc;
}

static int venus_hfi_suspend(void *dev)
{
	int rc = 0;
	struct venus_hfi_device *device = (struct venus_hfi_device *) dev;

	if (!device) {
		dprintk(VIDC_ERR, "%s invalid device\n", __func__);
		return -EINVAL;
	} else if (!device->res->sw_power_collapsible) {
		return -ENOTSUPP;
	}

	dprintk(VIDC_DBG, "Suspending Venus\n");
	flush_delayed_work(&venus_hfi_pm_work);

	mutex_lock(&device->lock);
	if (device->power_enabled) {
		dprintk(VIDC_WARN, "%s: Venus is busy\n", __func__);
		rc = -EBUSY;
	}
	mutex_unlock(&device->lock);
	return rc;
}

static int venus_hfi_flush_debug_queue(void *dev)
{
	int rc = 0;
	struct venus_hfi_device *device = (struct venus_hfi_device *) dev;

	if (!device) {
		dprintk(VIDC_ERR, "%s invalid device\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&device->lock);

	if (!device->power_enabled) {
		dprintk(VIDC_WARN, "%s: venus power off\n", __func__);
		rc = -EINVAL;
		goto exit;
	}
	__flush_debug_queue(device, NULL);
exit:
	mutex_unlock(&device->lock);
	return rc;
}

static enum hal_default_properties venus_hfi_get_default_properties(void *dev)
{
	enum hal_default_properties prop = 0;
	struct venus_hfi_device *device = (struct venus_hfi_device *) dev;

	if (!device) {
		dprintk(VIDC_ERR, "%s invalid device\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&device->lock);

	prop = HAL_VIDEO_DYNAMIC_BUF_MODE;

	mutex_unlock(&device->lock);
	return prop;
}

static int __set_clocks(struct venus_hfi_device *device, u32 freq)
{
	struct clock_info *cl;
	int rc = 0;

	venus_hfi_for_each_clock(device, cl) {
		if (cl->disable_memcore_only)
			continue;
		if (cl->has_scaling) {/* has_scaling */
			device->clk_freq = freq;
			rc = clk_set_rate(cl->clk, freq);
			if (rc) {
				dprintk(VIDC_ERR,
					"Failed to set clock rate %u %s: %d %s\n",
					freq, cl->name, rc, __func__);
				return rc;
			}

			trace_msm_vidc_perf_clock_scale(cl->name, freq);
			dprintk(VIDC_PROF, "Scaling clock %s to %u\n",
					cl->name, freq);
		}
	}

	return 0;
}

static int venus_hfi_scale_clocks(void *dev, u32 freq)
{
	int rc = 0;
	struct venus_hfi_device *device = dev;

	if (!device) {
		dprintk(VIDC_ERR, "Invalid args: %pK\n", device);
		return -EINVAL;
	}

	mutex_lock(&device->lock);

	if (__resume(device)) {
		dprintk(VIDC_ERR, "Resume from power collapse failed\n");
		rc = -ENODEV;
		goto exit;
	}

	rc = __set_clocks(device, freq);
exit:
	mutex_unlock(&device->lock);

	return rc;
}

static int __scale_clocks(struct venus_hfi_device *device)
{
	int rc = 0;
	struct allowed_clock_rates_table *allowed_clks_tbl = NULL;
	u32 rate = 0;

	allowed_clks_tbl = device->res->allowed_clks_tbl;

	dprintk(VIDC_DBG, "%s: NULL scale data\n", __func__);
	rate = device->clk_freq ? device->clk_freq :
		allowed_clks_tbl[0].clock_rate;

	rc = __set_clocks(device, rate);
	return rc;
}

/* Writes into cmdq without raising an interrupt */
static int __iface_cmdq_write_relaxed(struct venus_hfi_device *device,
		void *pkt, bool *requires_interrupt)
{
	struct vidc_iface_q_info *q_info;
	struct vidc_hal_cmd_pkt_hdr *cmd_packet;
	int result = -E2BIG;

	if (!device || !pkt) {
		dprintk(VIDC_ERR, "Invalid Params\n");
		return -EINVAL;
	}

	__strict_check(device);

	if (!__core_in_valid_state(device)) {
		dprintk(VIDC_DBG, "%s - fw not in init state\n", __func__);
		result = -EINVAL;
		goto err_q_null;
	}

	cmd_packet = (struct vidc_hal_cmd_pkt_hdr *)pkt;
	device->last_packet_type = cmd_packet->packet_type;

	q_info = &device->iface_queues[VIDC_IFACEQ_CMDQ_IDX];
	if (!q_info) {
		dprintk(VIDC_ERR, "cannot write to shared Q's\n");
		goto err_q_null;
	}

	if (!q_info->q_array.align_virtual_addr) {
		dprintk(VIDC_ERR, "cannot write to shared CMD Q's\n");
		result = -ENODATA;
		goto err_q_null;
	}

	__sim_modify_cmd_packet((u8 *)pkt, device);
	if (__resume(device)) {
		dprintk(VIDC_ERR, "%s: Power on failed\n", __func__);
		goto err_q_write;
	}

	if (!__write_queue(q_info, (u8 *)pkt, requires_interrupt)) {
		if (device->res->sw_power_collapsible) {
			cancel_delayed_work(&venus_hfi_pm_work);
			if (!queue_delayed_work(device->venus_pm_workq,
				&venus_hfi_pm_work,
				msecs_to_jiffies(
				device->res->msm_vidc_pwr_collapse_delay))) {
				dprintk(VIDC_DBG,
				"PM work already scheduled\n");
			}
		}

		result = 0;
	} else {
		dprintk(VIDC_ERR, "__iface_cmdq_write: queue full\n");
	}

err_q_write:
err_q_null:
	return result;
}

static int __iface_cmdq_write(struct venus_hfi_device *device, void *pkt)
{
	bool needs_interrupt = false;
	int rc = __iface_cmdq_write_relaxed(device, pkt, &needs_interrupt);

	if (!rc && needs_interrupt) {
		/* Consumer of cmdq prefers that we raise an interrupt */
		rc = 0;
		__write_register(device, VIDC_CPU_IC_SOFTINT,
				1 << VIDC_CPU_IC_SOFTINT_H2A_SHFT);
	}

	return rc;
}

static int __iface_msgq_read(struct venus_hfi_device *device, void *pkt)
{
	u32 tx_req_is_set = 0;
	int rc = 0;
	struct vidc_iface_q_info *q_info;

	if (!pkt) {
		dprintk(VIDC_ERR, "Invalid Params\n");
		return -EINVAL;
	}

	__strict_check(device);

	if (!__core_in_valid_state(device)) {
		dprintk(VIDC_DBG, "%s - fw not in init state\n", __func__);
		rc = -EINVAL;
		goto read_error_null;
	}

	if (device->iface_queues[VIDC_IFACEQ_MSGQ_IDX].
		q_array.align_virtual_addr == 0) {
		dprintk(VIDC_ERR, "cannot read from shared MSG Q's\n");
		rc = -ENODATA;
		goto read_error_null;
	}

	q_info = &device->iface_queues[VIDC_IFACEQ_MSGQ_IDX];
	if (!__read_queue(q_info, (u8 *)pkt, &tx_req_is_set)) {
		__hal_sim_modify_msg_packet((u8 *)pkt, device);
		if (tx_req_is_set)
			__write_register(device, VIDC_CPU_IC_SOFTINT,
				1 << VIDC_CPU_IC_SOFTINT_H2A_SHFT);
		rc = 0;
	} else
		rc = -ENODATA;

read_error_null:
	return rc;
}

static int __iface_dbgq_read(struct venus_hfi_device *device, void *pkt)
{
	u32 tx_req_is_set = 0;
	int rc = 0;
	struct vidc_iface_q_info *q_info;

	if (!pkt) {
		dprintk(VIDC_ERR, "Invalid Params\n");
		return -EINVAL;
	}

	__strict_check(device);

	if (device->iface_queues[VIDC_IFACEQ_DBGQ_IDX].
		q_array.align_virtual_addr == 0) {
		dprintk(VIDC_ERR, "cannot read from shared DBG Q's\n");
		rc = -ENODATA;
		goto dbg_error_null;
	}

	q_info = &device->iface_queues[VIDC_IFACEQ_DBGQ_IDX];
	if (!__read_queue(q_info, (u8 *)pkt, &tx_req_is_set)) {
		if (tx_req_is_set)
			__write_register(device, VIDC_CPU_IC_SOFTINT,
				1 << VIDC_CPU_IC_SOFTINT_H2A_SHFT);
		rc = 0;
	} else
		rc = -ENODATA;

dbg_error_null:
	return rc;
}

static void __set_queue_hdr_defaults(struct hfi_queue_header *q_hdr)
{
	q_hdr->qhdr_status = 0x1;
	q_hdr->qhdr_type = VIDC_IFACEQ_DFLT_QHDR;
	q_hdr->qhdr_q_size = VIDC_IFACEQ_QUEUE_SIZE / 4;
	q_hdr->qhdr_pkt_size = 0;
	q_hdr->qhdr_rx_wm = 0x1;
	q_hdr->qhdr_tx_wm = 0x1;
	q_hdr->qhdr_rx_req = 0x1;
	q_hdr->qhdr_tx_req = 0x0;
	q_hdr->qhdr_rx_irq_status = 0x0;
	q_hdr->qhdr_tx_irq_status = 0x0;
	q_hdr->qhdr_read_idx = 0x0;
	q_hdr->qhdr_write_idx = 0x0;
}

static void __interface_queues_release(struct venus_hfi_device *device)
{
	int i;
	struct hfi_mem_map_table *qdss;
	struct hfi_mem_map *mem_map;
	int num_entries = device->res->qdss_addr_set.count;
	unsigned long mem_map_table_base_addr;
	struct context_bank_info *cb;

	if (device->qdss.align_virtual_addr) {
		qdss = (struct hfi_mem_map_table *)
			device->qdss.align_virtual_addr;
		qdss->mem_map_num_entries = num_entries;
		mem_map_table_base_addr =
			device->qdss.align_device_addr +
			sizeof(struct hfi_mem_map_table);
		qdss->mem_map_table_base_addr =
			(u32)mem_map_table_base_addr;
		if ((unsigned long)qdss->mem_map_table_base_addr !=
			mem_map_table_base_addr) {
			dprintk(VIDC_ERR,
				"Invalid mem_map_table_base_addr %#lx",
				mem_map_table_base_addr);
		}

		mem_map = (struct hfi_mem_map *)(qdss + 1);
		cb = msm_smem_get_context_bank(device->hal_client,
					false, HAL_BUFFER_INTERNAL_CMD_QUEUE);

		for (i = 0; cb && i < num_entries; i++) {
			iommu_unmap(cb->mapping->domain,
						mem_map[i].virtual_addr,
						mem_map[i].size);
		}

		__smem_free(device, &device->qdss.mem_data);
	}

	__smem_free(device, &device->iface_q_table.mem_data);
	__smem_free(device, &device->sfr.mem_data);

	for (i = 0; i < VIDC_IFACEQ_NUMQ; i++) {
		device->iface_queues[i].q_hdr = NULL;
		device->iface_queues[i].q_array.align_virtual_addr = NULL;
		device->iface_queues[i].q_array.align_device_addr = 0;
	}

	device->iface_q_table.align_virtual_addr = NULL;
	device->iface_q_table.align_device_addr = 0;

	device->qdss.align_virtual_addr = NULL;
	device->qdss.align_device_addr = 0;

	device->sfr.align_virtual_addr = NULL;
	device->sfr.align_device_addr = 0;

	device->mem_addr.align_virtual_addr = NULL;
	device->mem_addr.align_device_addr = 0;

	msm_smem_delete_client(device->hal_client);
	device->hal_client = NULL;
}

static int __get_qdss_iommu_virtual_addr(struct venus_hfi_device *dev,
		struct hfi_mem_map *mem_map, struct dma_iommu_mapping *mapping)
{
	int i;
	int rc = 0;
	dma_addr_t iova = QDSS_IOVA_START;
	int num_entries = dev->res->qdss_addr_set.count;
	struct addr_range *qdss_addr_tbl = dev->res->qdss_addr_set.addr_tbl;

	if (!num_entries)
		return -ENODATA;

	for (i = 0; i < num_entries; i++) {
		if (mapping) {
			rc = iommu_map(mapping->domain, iova,
					qdss_addr_tbl[i].start,
					qdss_addr_tbl[i].size,
					IOMMU_READ | IOMMU_WRITE);

			if (rc) {
				dprintk(VIDC_ERR,
						"IOMMU QDSS mapping failed for addr %#x\n",
						qdss_addr_tbl[i].start);
				rc = -ENOMEM;
				break;
			}
		} else {
			iova =  qdss_addr_tbl[i].start;
		}

		mem_map[i].virtual_addr = (u32)iova;
		mem_map[i].physical_addr = qdss_addr_tbl[i].start;
		mem_map[i].size = qdss_addr_tbl[i].size;
		mem_map[i].attr = 0x0;

		iova += mem_map[i].size;
	}

	if (i < num_entries) {
		dprintk(VIDC_ERR,
			"QDSS mapping failed, Freeing other entries %d\n", i);

		for (--i; mapping && i >= 0; i--) {
			iommu_unmap(mapping->domain,
				mem_map[i].virtual_addr,
				mem_map[i].size);
		}
	}

	return rc;
}

static void __setup_ucregion_memory_map(struct venus_hfi_device *device)
{
	__write_register(device, VIDC_UC_REGION_ADDR,
			(u32)device->iface_q_table.align_device_addr);
	__write_register(device, VIDC_UC_REGION_SIZE, SHARED_QSIZE);
	__write_register(device, VIDC_CPU_CS_SCIACMDARG2,
			(u32)device->iface_q_table.align_device_addr);
	__write_register(device, VIDC_CPU_CS_SCIACMDARG1, 0x01);
	if (device->sfr.align_device_addr)
		__write_register(device, VIDC_SFR_ADDR,
				(u32)device->sfr.align_device_addr);
	if (device->qdss.align_device_addr)
		__write_register(device, VIDC_MMAP_ADDR,
				(u32)device->qdss.align_device_addr);
}

static int __interface_queues_init(struct venus_hfi_device *dev)
{
	struct hfi_queue_table_header *q_tbl_hdr;
	struct hfi_queue_header *q_hdr;
	u32 i;
	int rc = 0;
	struct hfi_mem_map_table *qdss;
	struct hfi_mem_map *mem_map;
	struct vidc_iface_q_info *iface_q;
	struct hfi_sfr_struct *vsfr;
	struct vidc_mem_addr *mem_addr;
	int offset = 0;
	int num_entries = dev->res->qdss_addr_set.count;
	phys_addr_t fw_bias = 0;
	size_t q_size;
	unsigned long mem_map_table_base_addr;
	struct context_bank_info *cb;

	q_size = SHARED_QSIZE - ALIGNED_SFR_SIZE - ALIGNED_QDSS_SIZE;
	mem_addr = &dev->mem_addr;
	if (!is_iommu_present(dev->res))
		fw_bias = dev->hal_data->firmware_base;
	rc = __smem_alloc(dev, mem_addr, q_size, 1, 0,
			HAL_BUFFER_INTERNAL_CMD_QUEUE);
	if (rc) {
		dprintk(VIDC_ERR, "iface_q_table_alloc_fail\n");
		goto fail_alloc_queue;
	}

	dev->iface_q_table.align_virtual_addr = mem_addr->align_virtual_addr;
	dev->iface_q_table.align_device_addr = mem_addr->align_device_addr -
					fw_bias;
	dev->iface_q_table.mem_size = VIDC_IFACEQ_TABLE_SIZE;
	dev->iface_q_table.mem_data = mem_addr->mem_data;
	offset += dev->iface_q_table.mem_size;

	for (i = 0; i < VIDC_IFACEQ_NUMQ; i++) {
		iface_q = &dev->iface_queues[i];
		iface_q->q_array.align_device_addr = mem_addr->align_device_addr
			+ offset - fw_bias;
		iface_q->q_array.align_virtual_addr =
			mem_addr->align_virtual_addr + offset;
		iface_q->q_array.mem_size = VIDC_IFACEQ_QUEUE_SIZE;
		offset += iface_q->q_array.mem_size;
		iface_q->q_hdr = VIDC_IFACEQ_GET_QHDR_START_ADDR(
				dev->iface_q_table.align_virtual_addr, i);
		__set_queue_hdr_defaults(iface_q->q_hdr);
	}

	if ((msm_vidc_fw_debug_mode & HFI_DEBUG_MODE_QDSS) && num_entries) {
		rc = __smem_alloc(dev, mem_addr,
				ALIGNED_QDSS_SIZE, 1, 0,
				HAL_BUFFER_INTERNAL_CMD_QUEUE);
		if (rc) {
			dprintk(VIDC_WARN,
				"qdss_alloc_fail: QDSS messages logging will not work\n");
			dev->qdss.align_device_addr = 0;
		} else {
			dev->qdss.align_device_addr =
				mem_addr->align_device_addr - fw_bias;
			dev->qdss.align_virtual_addr =
				mem_addr->align_virtual_addr;
			dev->qdss.mem_size = ALIGNED_QDSS_SIZE;
			dev->qdss.mem_data = mem_addr->mem_data;
		}
	}

	rc = __smem_alloc(dev, mem_addr,
			ALIGNED_SFR_SIZE, 1, 0,
			HAL_BUFFER_INTERNAL_CMD_QUEUE);
	if (rc) {
		dprintk(VIDC_WARN, "sfr_alloc_fail: SFR not will work\n");
		dev->sfr.align_device_addr = 0;
	} else {
		dev->sfr.align_device_addr = mem_addr->align_device_addr -
					fw_bias;
		dev->sfr.align_virtual_addr = mem_addr->align_virtual_addr;
		dev->sfr.mem_size = ALIGNED_SFR_SIZE;
		dev->sfr.mem_data = mem_addr->mem_data;
	}

	q_tbl_hdr = (struct hfi_queue_table_header *)
			dev->iface_q_table.align_virtual_addr;
	q_tbl_hdr->qtbl_version = 0;
	q_tbl_hdr->device_addr = (void *)dev;
	strlcpy(q_tbl_hdr->name, "msm_v4l2_vidc", sizeof(q_tbl_hdr->name));
	q_tbl_hdr->qtbl_size = VIDC_IFACEQ_TABLE_SIZE;
	q_tbl_hdr->qtbl_qhdr0_offset = sizeof(struct hfi_queue_table_header);
	q_tbl_hdr->qtbl_qhdr_size = sizeof(struct hfi_queue_header);
	q_tbl_hdr->qtbl_num_q = VIDC_IFACEQ_NUMQ;
	q_tbl_hdr->qtbl_num_active_q = VIDC_IFACEQ_NUMQ;

	iface_q = &dev->iface_queues[VIDC_IFACEQ_CMDQ_IDX];
	q_hdr = iface_q->q_hdr;
	q_hdr->qhdr_start_addr = iface_q->q_array.align_device_addr;
	q_hdr->qhdr_type |= HFI_Q_ID_HOST_TO_CTRL_CMD_Q;

	iface_q = &dev->iface_queues[VIDC_IFACEQ_MSGQ_IDX];
	q_hdr = iface_q->q_hdr;
	q_hdr->qhdr_start_addr = iface_q->q_array.align_device_addr;
	q_hdr->qhdr_type |= HFI_Q_ID_CTRL_TO_HOST_MSG_Q;

	iface_q = &dev->iface_queues[VIDC_IFACEQ_DBGQ_IDX];
	q_hdr = iface_q->q_hdr;
	q_hdr->qhdr_start_addr = iface_q->q_array.align_device_addr;
	q_hdr->qhdr_type |= HFI_Q_ID_CTRL_TO_HOST_DEBUG_Q;
	/*
	 * Set receive request to zero on debug queue as there is no
	 * need of interrupt from video hardware for debug messages
	 */
	q_hdr->qhdr_rx_req = 0;

	if (dev->qdss.align_virtual_addr) {
		qdss = (struct hfi_mem_map_table *)dev->qdss.align_virtual_addr;
		qdss->mem_map_num_entries = num_entries;
		mem_map_table_base_addr = dev->qdss.align_device_addr +
			sizeof(struct hfi_mem_map_table);
		qdss->mem_map_table_base_addr = mem_map_table_base_addr;

		mem_map = (struct hfi_mem_map *)(qdss + 1);
		cb = msm_smem_get_context_bank(dev->hal_client, false,
				HAL_BUFFER_INTERNAL_CMD_QUEUE);
		if (!cb) {
			dprintk(VIDC_ERR,
				"%s: failed to get context bank\n", __func__);
			return -EINVAL;
		}

		rc = __get_qdss_iommu_virtual_addr(dev, mem_map, cb->mapping);
		if (rc) {
			dprintk(VIDC_ERR,
				"IOMMU mapping failed, Freeing qdss memdata\n");
			__smem_free(dev, &dev->qdss.mem_data);
			dev->qdss.align_virtual_addr = NULL;
			dev->qdss.align_device_addr = 0;
		}
	}

	vsfr = (struct hfi_sfr_struct *) dev->sfr.align_virtual_addr;
	vsfr->bufSize = ALIGNED_SFR_SIZE;

	__setup_ucregion_memory_map(dev);
	return 0;
fail_alloc_queue:
	return -ENOMEM;
}

static int __sys_set_debug(struct venus_hfi_device *device, u32 debug)
{
	u8 packet[VIDC_IFACEQ_VAR_SMALL_PKT_SIZE];
	int rc = 0;
	struct hfi_cmd_sys_set_property_packet *pkt =
		(struct hfi_cmd_sys_set_property_packet *) &packet;

	rc = call_hfi_pkt_op(device, sys_debug_config, pkt, debug);
	if (rc) {
		dprintk(VIDC_WARN,
			"Debug mode setting to FW failed\n");
		return -ENOTEMPTY;
	}

	if (__iface_cmdq_write(device, pkt))
		return -ENOTEMPTY;
	return 0;
}

static int __sys_set_coverage(struct venus_hfi_device *device, u32 mode)
{
	u8 packet[VIDC_IFACEQ_VAR_SMALL_PKT_SIZE];
	int rc = 0;
	struct hfi_cmd_sys_set_property_packet *pkt =
		(struct hfi_cmd_sys_set_property_packet *) &packet;

	rc = call_hfi_pkt_op(device, sys_coverage_config,
			pkt, mode);
	if (rc) {
		dprintk(VIDC_WARN,
			"Coverage mode setting to FW failed\n");
		return -ENOTEMPTY;
	}

	if (__iface_cmdq_write(device, pkt)) {
		dprintk(VIDC_WARN, "Failed to send coverage pkt to f/w\n");
		return -ENOTEMPTY;
	}

	return 0;
}

static int __sys_set_idle_message(struct venus_hfi_device *device,
	bool enable)
{
	u8 packet[VIDC_IFACEQ_VAR_SMALL_PKT_SIZE];
	struct hfi_cmd_sys_set_property_packet *pkt =
		(struct hfi_cmd_sys_set_property_packet *) &packet;

	if (!enable) {
		dprintk(VIDC_DBG, "sys_idle_indicator is not enabled\n");
		return 0;
	}

	call_hfi_pkt_op(device, sys_idle_indicator, pkt, enable);
	if (__iface_cmdq_write(device, pkt))
		return -ENOTEMPTY;
	return 0;
}

static int __sys_set_power_control(struct venus_hfi_device *device,
	bool enable)
{
	struct regulator_info *rinfo;
	bool supported = false;
	u8 packet[VIDC_IFACEQ_VAR_SMALL_PKT_SIZE];
	struct hfi_cmd_sys_set_property_packet *pkt =
		(struct hfi_cmd_sys_set_property_packet *) &packet;

	venus_hfi_for_each_regulator(device, rinfo) {
		if (rinfo->has_hw_power_collapse) {
			supported = true;
			break;
		}
	}

	if (!supported)
		return 0;

	call_hfi_pkt_op(device, sys_power_control, pkt, enable);
	if (__iface_cmdq_write(device, pkt))
		return -ENOTEMPTY;
	return 0;
}

static int venus_hfi_core_init(void *device)
{
	int rc = 0;
	struct hfi_cmd_sys_init_packet pkt;
	struct hfi_cmd_sys_get_property_packet version_pkt;
	struct venus_hfi_device *dev;

	if (!device) {
		dprintk(VIDC_ERR, "Invalid device\n");
		return -ENODEV;
	}

	dev = device;

	dprintk(VIDC_DBG, "Core initializing\n");

	mutex_lock(&dev->lock);

	dev->bus_vote.data =
		kzalloc(sizeof(struct vidc_bus_vote_data), GFP_KERNEL);
	if (!dev->bus_vote.data) {
		dprintk(VIDC_ERR, "Bus vote data memory is not allocated\n");
		rc = -ENOMEM;
		goto err_no_mem;
	}

	dev->bus_vote.data_count = 1;
	dev->bus_vote.data->power_mode = VIDC_POWER_TURBO;

	rc = __load_fw(dev);
	if (rc) {
		dprintk(VIDC_ERR, "Failed to load Venus FW\n");
		goto err_load_fw;
	}

	__set_state(dev, VENUS_STATE_INIT);

	if (!dev->hal_client) {
		dev->hal_client = msm_smem_new_client(
				SMEM_ION, dev->res, MSM_VIDC_UNKNOWN);
		if (dev->hal_client == NULL) {
			dprintk(VIDC_ERR, "Failed to alloc ION_Client\n");
			rc = -ENODEV;
			goto err_core_init;
		}

		dprintk(VIDC_DBG, "Dev_Virt: %pa, Reg_Virt: %pK\n",
			&dev->hal_data->firmware_base,
			dev->hal_data->register_base);

		rc = __interface_queues_init(dev);
		if (rc) {
			dprintk(VIDC_ERR, "failed to init queues\n");
			rc = -ENOMEM;
			goto err_core_init;
		}
	} else {
		dprintk(VIDC_ERR, "hal_client exists\n");
		rc = -EEXIST;
		goto err_core_init;
	}

	rc = __boot_firmware(dev);
	if (rc) {
		dprintk(VIDC_ERR, "Failed to start core\n");
		rc = -ENODEV;
		goto err_core_init;
	}

	rc =  call_hfi_pkt_op(dev, sys_init, &pkt, HFI_VIDEO_ARCH_OX);
	if (rc) {
		dprintk(VIDC_ERR, "Failed to create sys init pkt\n");
		goto err_core_init;
	}

	if (__iface_cmdq_write(dev, &pkt)) {
		rc = -ENOTEMPTY;
		goto err_core_init;
	}

	rc = call_hfi_pkt_op(dev, sys_image_version, &version_pkt);
	if (rc || __iface_cmdq_write(dev, &version_pkt))
		dprintk(VIDC_WARN, "Failed to send image version pkt to f/w\n");

	__enable_subcaches(device);
	__set_subcaches(device);

	if (dev->res->pm_qos_latency_us) {
#ifdef CONFIG_SMP
		dev->qos.type = PM_QOS_REQ_AFFINE_IRQ;
		dev->qos.irq = dev->hal_data->irq;
#endif
		pm_qos_add_request(&dev->qos, PM_QOS_CPU_DMA_LATENCY,
				dev->res->pm_qos_latency_us);
	}
	dprintk(VIDC_DBG, "Core inited successfully\n");
	mutex_unlock(&dev->lock);
	return rc;
err_core_init:
	__set_state(dev, VENUS_STATE_DEINIT);
	__unload_fw(dev);
err_load_fw:
err_no_mem:
	dprintk(VIDC_ERR, "Core init failed\n");
	mutex_unlock(&dev->lock);
	return rc;
}

static int venus_hfi_core_release(void *dev)
{
	int rc = 0;
	struct venus_hfi_device *device = dev;
	struct hal_session *session, *next;

	if (!device) {
		dprintk(VIDC_ERR, "invalid device\n");
		return -ENODEV;
	}

	mutex_lock(&device->lock);
	dprintk(VIDC_DBG, "Core releasing\n");
	if (device->res->pm_qos_latency_us &&
		pm_qos_request_active(&device->qos))
		pm_qos_remove_request(&device->qos);

	__resume(device);
	__set_state(device, VENUS_STATE_DEINIT);
	__unload_fw(device);

	/* unlink all sessions from device */
	list_for_each_entry_safe(session, next, &device->sess_head, list)
		list_del(&session->list);

	dprintk(VIDC_DBG, "Core released successfully\n");
	mutex_unlock(&device->lock);

	return rc;
}

static int __get_q_size(struct venus_hfi_device *dev, unsigned int q_index)
{
	struct hfi_queue_header *queue;
	struct vidc_iface_q_info *q_info;
	u32 write_ptr, read_ptr;

	if (q_index >= VIDC_IFACEQ_NUMQ) {
		dprintk(VIDC_ERR, "Invalid q index: %d\n", q_index);
		return -ENOENT;
	}

	q_info = &dev->iface_queues[q_index];
	if (!q_info) {
		dprintk(VIDC_ERR, "cannot read shared Q's\n");
		return -ENOENT;
	}

	queue = (struct hfi_queue_header *)q_info->q_hdr;
	if (!queue) {
		dprintk(VIDC_ERR, "queue not present\n");
		return -ENOENT;
	}

	write_ptr = (u32)queue->qhdr_write_idx;
	read_ptr = (u32)queue->qhdr_read_idx;
	return read_ptr - write_ptr;
}

static void __core_clear_interrupt(struct venus_hfi_device *device)
{
	u32 intr_status = 0;

	if (!device) {
		dprintk(VIDC_ERR, "%s: NULL device\n", __func__);
		return;
	}

	intr_status = __read_register(device, VIDC_WRAPPER_INTR_STATUS);

	if (intr_status & VIDC_WRAPPER_INTR_STATUS_A2H_BMSK ||
		intr_status & VIDC_WRAPPER_INTR_STATUS_A2HWD_BMSK ||
		intr_status &
			VIDC_CPU_CS_SCIACMDARG0_HFI_CTRL_INIT_IDLE_MSG_BMSK) {
		device->intr_status |= intr_status;
		device->reg_count++;
		dprintk(VIDC_DBG,
			"INTERRUPT for device: %pK: times: %d interrupt_status: %d\n",
			device, device->reg_count, intr_status);
	} else {
		device->spur_count++;
		dprintk(VIDC_INFO,
			"SPURIOUS_INTR for device: %pK: times: %d interrupt_status: %d\n",
			device, device->spur_count, intr_status);
	}

	__write_register(device, VIDC_CPU_CS_A2HSOFTINTCLR, 1);
	__write_register(device, VIDC_WRAPPER_INTR_CLEAR, intr_status);
}

static int venus_hfi_core_ping(void *device)
{
	struct hfi_cmd_sys_ping_packet pkt;
	int rc = 0;
	struct venus_hfi_device *dev;

	if (!device) {
		dprintk(VIDC_ERR, "invalid device\n");
		return -ENODEV;
	}

	dev = device;
	mutex_lock(&dev->lock);

	rc = call_hfi_pkt_op(dev, sys_ping, &pkt);
	if (rc) {
		dprintk(VIDC_ERR, "core_ping: failed to create packet\n");
		goto err_create_pkt;
	}

	if (__iface_cmdq_write(dev, &pkt))
		rc = -ENOTEMPTY;

err_create_pkt:
	mutex_unlock(&dev->lock);
	return rc;
}

static int venus_hfi_core_trigger_ssr(void *device,
		enum hal_ssr_trigger_type type)
{
	struct hfi_cmd_sys_test_ssr_packet pkt;
	int rc = 0;
	struct venus_hfi_device *dev;

	if (!device) {
		dprintk(VIDC_ERR, "invalid device\n");
		return -ENODEV;
	}

	dev = device;
	mutex_lock(&dev->lock);

	rc = call_hfi_pkt_op(dev, ssr_cmd, type, &pkt);
	if (rc) {
		dprintk(VIDC_ERR, "core_ping: failed to create packet\n");
		goto err_create_pkt;
	}

	if (__iface_cmdq_write(dev, &pkt))
		rc = -ENOTEMPTY;

err_create_pkt:
	mutex_unlock(&dev->lock);
	return rc;
}

static int venus_hfi_session_set_property(void *sess,
					enum hal_property ptype, void *pdata)
{
	u8 packet[VIDC_IFACEQ_VAR_LARGE_PKT_SIZE];
	struct hfi_cmd_session_set_property_packet *pkt =
		(struct hfi_cmd_session_set_property_packet *) &packet;
	struct hal_session *session = sess;
	struct venus_hfi_device *device;
	int rc = 0;

	if (!session || !session->device || !pdata) {
		dprintk(VIDC_ERR, "Invalid Params\n");
		return -EINVAL;
	}

	device = session->device;
	mutex_lock(&device->lock);

	dprintk(VIDC_INFO, "in set_prop,with prop id: %#x\n", ptype);
	if (!__is_session_valid(device, session, __func__)) {
		rc = -EINVAL;
		goto err_set_prop;
	}

	rc = call_hfi_pkt_op(device, session_set_property,
			pkt, session, ptype, pdata);

	if (rc == -ENOTSUPP) {
		dprintk(VIDC_DBG,
			"set property: unsupported prop id: %#x\n", ptype);
		rc = 0;
		goto err_set_prop;
	} else if (rc) {
		dprintk(VIDC_ERR, "set property: failed to create packet\n");
		rc = -EINVAL;
		goto err_set_prop;
	}

	if (__iface_cmdq_write(session->device, pkt)) {
		rc = -ENOTEMPTY;
		goto err_set_prop;
	}

err_set_prop:
	mutex_unlock(&device->lock);
	return rc;
}

static int venus_hfi_session_get_property(void *sess,
					enum hal_property ptype)
{
	struct hfi_cmd_session_get_property_packet pkt = {0};
	struct hal_session *session = sess;
	int rc = 0;
	struct venus_hfi_device *device;

	if (!session || !session->device) {
		dprintk(VIDC_ERR, "Invalid Params\n");
		return -EINVAL;
	}

	device = session->device;
	mutex_lock(&device->lock);

	dprintk(VIDC_INFO, "%s: property id: %d\n", __func__, ptype);
	if (!__is_session_valid(device, session, __func__)) {
		rc = -EINVAL;
		goto err_create_pkt;
	}

	rc = call_hfi_pkt_op(device, session_get_property,
				&pkt, session, ptype);
	if (rc) {
		dprintk(VIDC_ERR, "get property profile: pkt failed\n");
		goto err_create_pkt;
	}

	if (__iface_cmdq_write(session->device, &pkt)) {
		rc = -ENOTEMPTY;
		dprintk(VIDC_ERR, "%s cmdq_write error\n", __func__);
	}

err_create_pkt:
	mutex_unlock(&device->lock);
	return rc;
}

static void __set_default_sys_properties(struct venus_hfi_device *device)
{
	if (__sys_set_debug(device, msm_vidc_fw_debug))
		dprintk(VIDC_WARN, "Setting fw_debug msg ON failed\n");
	if (__sys_set_idle_message(device,
		device->res->sys_idle_indicator || msm_vidc_sys_idle_indicator))
		dprintk(VIDC_WARN, "Setting idle response ON failed\n");
	if (__sys_set_power_control(device, msm_vidc_fw_low_power_mode))
		dprintk(VIDC_WARN, "Setting h/w power collapse ON failed\n");
}

static void __session_clean(struct hal_session *session)
{
	struct hal_session *temp, *next;
	struct venus_hfi_device *device;

	if (!session || !session->device) {
		dprintk(VIDC_WARN, "%s: invalid params\n", __func__);
		return;
	}
	device = session->device;
	dprintk(VIDC_DBG, "deleted the session: %pK\n", session);
	/*
	 * session might have been removed from the device list in
	 * core_release, so check and remove if it is in the list
	 */
	list_for_each_entry_safe(temp, next, &device->sess_head, list) {
		if (session == temp) {
			list_del(&session->list);
			break;
		}
	}
	/* Poison the session handle with zeros */
	*session = (struct hal_session){ {0} };
	kfree(session);
}

static int venus_hfi_session_clean(void *session)
{
	struct hal_session *sess_close;
	struct venus_hfi_device *device;

	if (!session) {
		dprintk(VIDC_ERR, "Invalid Params %s\n", __func__);
		return -EINVAL;
	}

	sess_close = session;
	device = sess_close->device;

	if (!device) {
		dprintk(VIDC_ERR, "Invalid device handle %s\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&device->lock);

	__session_clean(sess_close);

	mutex_unlock(&device->lock);
	return 0;
}

static int venus_hfi_session_init(void *device, void *session_id,
		enum hal_domain session_type, enum hal_video_codec codec_type,
		void **new_session)
{
	struct hfi_cmd_sys_session_init_packet pkt;
	struct venus_hfi_device *dev;
	struct hal_session *s;

	if (!device || !new_session) {
		dprintk(VIDC_ERR, "%s - invalid input\n", __func__);
		return -EINVAL;
	}

	dev = device;
	mutex_lock(&dev->lock);

	s = kzalloc(sizeof(struct hal_session), GFP_KERNEL);
	if (!s) {
		dprintk(VIDC_ERR, "new session fail: Out of memory\n");
		goto err_session_init_fail;
	}

	s->session_id = session_id;
	s->is_decoder = (session_type == HAL_VIDEO_DOMAIN_DECODER);
	s->device = dev;
	s->codec = codec_type;
	s->domain = session_type;
	dprintk(VIDC_DBG,
		"%s: inst %pK, session %pK, codec 0x%x, domain 0x%x\n",
		__func__, session_id, s, s->codec, s->domain);

	list_add_tail(&s->list, &dev->sess_head);

	__set_default_sys_properties(device);

	if (call_hfi_pkt_op(dev, session_init, &pkt,
			s, session_type, codec_type)) {
		dprintk(VIDC_ERR, "session_init: failed to create packet\n");
		goto err_session_init_fail;
	}

	*new_session = s;
	if (__iface_cmdq_write(dev, &pkt))
		goto err_session_init_fail;

	mutex_unlock(&dev->lock);
	return 0;

err_session_init_fail:
	if (s)
		__session_clean(s);
	*new_session = NULL;
	mutex_unlock(&dev->lock);
	return -EINVAL;
}

static int __send_session_cmd(struct hal_session *session, int pkt_type)
{
	struct vidc_hal_session_cmd_pkt pkt;
	int rc = 0;
	struct venus_hfi_device *device = session->device;

	if (!__is_session_valid(device, session, __func__))
		return -EINVAL;

	rc = call_hfi_pkt_op(device, session_cmd,
			&pkt, pkt_type, session);
	if (rc == -EPERM)
		return 0;

	if (rc) {
		dprintk(VIDC_ERR, "send session cmd: create pkt failed\n");
		goto err_create_pkt;
	}

	if (__iface_cmdq_write(session->device, &pkt))
		rc = -ENOTEMPTY;

err_create_pkt:
	return rc;
}

static int venus_hfi_session_end(void *session)
{
	struct hal_session *sess;
	struct venus_hfi_device *device;
	int rc = 0;

	if (!session) {
		dprintk(VIDC_ERR, "Invalid Params %s\n", __func__);
		return -EINVAL;
	}

	sess = session;
	device = sess->device;

	mutex_lock(&device->lock);

	if (msm_vidc_fw_coverage) {
		if (__sys_set_coverage(sess->device, msm_vidc_fw_coverage))
			dprintk(VIDC_WARN, "Fw_coverage msg ON failed\n");
	}

	rc = __send_session_cmd(session, HFI_CMD_SYS_SESSION_END);

	mutex_unlock(&device->lock);

	return rc;
}

static int venus_hfi_session_abort(void *sess)
{
	struct hal_session *session = sess;
	struct venus_hfi_device *device;
	int rc = 0;

	if (!session || !session->device) {
		dprintk(VIDC_ERR, "Invalid Params %s\n", __func__);
		return -EINVAL;
	}

	device = session->device;

	mutex_lock(&device->lock);

	__flush_debug_queue(device, NULL);
	rc = __send_session_cmd(session, HFI_CMD_SYS_SESSION_ABORT);

	mutex_unlock(&device->lock);

	return rc;
}

static int venus_hfi_session_set_buffers(void *sess,
				struct vidc_buffer_addr_info *buffer_info)
{
	struct hfi_cmd_session_set_buffers_packet *pkt;
	u8 packet[VIDC_IFACEQ_VAR_LARGE_PKT_SIZE];
	int rc = 0;
	struct hal_session *session = sess;
	struct venus_hfi_device *device;

	if (!session || !session->device || !buffer_info) {
		dprintk(VIDC_ERR, "Invalid Params\n");
		return -EINVAL;
	}

	device = session->device;
	mutex_lock(&device->lock);

	if (!__is_session_valid(device, session, __func__)) {
		rc = -EINVAL;
		goto err_create_pkt;
	}
	if (buffer_info->buffer_type == HAL_BUFFER_INPUT) {
		/*
		 * Hardware doesn't care about input buffers being
		 * published beforehand
		 */
		rc = 0;
		goto err_create_pkt;
	}

	pkt = (struct hfi_cmd_session_set_buffers_packet *)packet;

	rc = call_hfi_pkt_op(device, session_set_buffers,
			pkt, session, buffer_info);
	if (rc) {
		dprintk(VIDC_ERR, "set buffers: failed to create packet\n");
		goto err_create_pkt;
	}

	dprintk(VIDC_INFO, "set buffers: %#x\n", buffer_info->buffer_type);
	if (__iface_cmdq_write(session->device, pkt))
		rc = -ENOTEMPTY;

err_create_pkt:
	mutex_unlock(&device->lock);
	return rc;
}

static int venus_hfi_session_release_buffers(void *sess,
				struct vidc_buffer_addr_info *buffer_info)
{
	struct hfi_cmd_session_release_buffer_packet *pkt;
	u8 packet[VIDC_IFACEQ_VAR_LARGE_PKT_SIZE];
	int rc = 0;
	struct hal_session *session = sess;
	struct venus_hfi_device *device;

	if (!session || !session->device || !buffer_info) {
		dprintk(VIDC_ERR, "Invalid Params\n");
		return -EINVAL;
	}

	device = session->device;
	mutex_lock(&device->lock);

	if (!__is_session_valid(device, session, __func__)) {
		rc = -EINVAL;
		goto err_create_pkt;
	}
	if (buffer_info->buffer_type == HAL_BUFFER_INPUT) {
		rc = 0;
		goto err_create_pkt;
	}

	pkt = (struct hfi_cmd_session_release_buffer_packet *) packet;

	rc = call_hfi_pkt_op(device, session_release_buffers,
			pkt, session, buffer_info);
	if (rc) {
		dprintk(VIDC_ERR, "release buffers: failed to create packet\n");
		goto err_create_pkt;
	}

	dprintk(VIDC_INFO, "Release buffers: %#x\n", buffer_info->buffer_type);
	if (__iface_cmdq_write(session->device, pkt))
		rc = -ENOTEMPTY;

err_create_pkt:
	mutex_unlock(&device->lock);
	return rc;
}

static int venus_hfi_session_load_res(void *session)
{
	struct hal_session *sess;
	struct venus_hfi_device *device;
	int rc = 0;

	if (!session) {
		dprintk(VIDC_ERR, "Invalid Params %s\n", __func__);
		return -EINVAL;
	}

	sess = session;
	device = sess->device;

	mutex_lock(&device->lock);
	rc = __send_session_cmd(sess, HFI_CMD_SESSION_LOAD_RESOURCES);
	mutex_unlock(&device->lock);

	return rc;
}

static int venus_hfi_session_release_res(void *session)
{
	struct hal_session *sess;
	struct venus_hfi_device *device;
	int rc = 0;

	if (!session) {
		dprintk(VIDC_ERR, "Invalid Params %s\n", __func__);
		return -EINVAL;
	}

	sess = session;
	device = sess->device;

	mutex_lock(&device->lock);
	rc = __send_session_cmd(sess, HFI_CMD_SESSION_RELEASE_RESOURCES);
	mutex_unlock(&device->lock);

	return rc;
}

static int venus_hfi_session_start(void *session)
{
	struct hal_session *sess;
	struct venus_hfi_device *device;
	int rc = 0;

	if (!session) {
		dprintk(VIDC_ERR, "Invalid Params %s\n", __func__);
		return -EINVAL;
	}

	sess = session;
	device = sess->device;

	mutex_lock(&device->lock);
	rc = __send_session_cmd(sess, HFI_CMD_SESSION_START);
	mutex_unlock(&device->lock);

	return rc;
}

static int venus_hfi_session_continue(void *session)
{
	struct hal_session *sess;
	struct venus_hfi_device *device;
	int rc = 0;

	if (!session) {
		dprintk(VIDC_ERR, "Invalid Params %s\n", __func__);
		return -EINVAL;
	}

	sess = session;
	device = sess->device;

	mutex_lock(&device->lock);
	rc = __send_session_cmd(sess, HFI_CMD_SESSION_CONTINUE);
	mutex_unlock(&device->lock);

	return rc;
}

static int venus_hfi_session_stop(void *session)
{
	struct hal_session *sess;
	struct venus_hfi_device *device;
	int rc = 0;

	if (!session) {
		dprintk(VIDC_ERR, "Invalid Params %s\n", __func__);
		return -EINVAL;
	}

	sess = session;
	device = sess->device;

	mutex_lock(&device->lock);
	rc = __send_session_cmd(sess, HFI_CMD_SESSION_STOP);
	mutex_unlock(&device->lock);

	return rc;
}

static int __session_etb(struct hal_session *session,
		struct vidc_frame_data *input_frame, bool relaxed)
{
	int rc = 0;
	struct venus_hfi_device *device = session->device;

	if (!__is_session_valid(device, session, __func__))
		return -EINVAL;

	if (session->is_decoder) {
		struct hfi_cmd_session_empty_buffer_compressed_packet pkt;

		rc = call_hfi_pkt_op(device, session_etb_decoder,
				&pkt, session, input_frame);
		if (rc) {
			dprintk(VIDC_ERR,
					"Session etb decoder: failed to create pkt\n");
			goto err_create_pkt;
		}

		if (!relaxed)
			rc = __iface_cmdq_write(session->device, &pkt);
		else
			rc = __iface_cmdq_write_relaxed(session->device,
					&pkt, NULL);
		if (rc)
			goto err_create_pkt;
	} else {
		struct hfi_cmd_session_empty_buffer_uncompressed_plane0_packet
			pkt;

		rc = call_hfi_pkt_op(device, session_etb_encoder,
					 &pkt, session, input_frame);
		if (rc) {
			dprintk(VIDC_ERR,
					"Session etb encoder: failed to create pkt\n");
			goto err_create_pkt;
		}

		if (!relaxed)
			rc = __iface_cmdq_write(session->device, &pkt);
		else
			rc = __iface_cmdq_write_relaxed(session->device,
					&pkt, NULL);
		if (rc)
			goto err_create_pkt;
	}

err_create_pkt:
	return rc;
}

static int venus_hfi_session_etb(void *sess,
				struct vidc_frame_data *input_frame)
{
	int rc = 0;
	struct hal_session *session = sess;
	struct venus_hfi_device *device;

	if (!session || !session->device || !input_frame) {
		dprintk(VIDC_ERR, "Invalid Params\n");
		return -EINVAL;
	}

	device = session->device;
	mutex_lock(&device->lock);
	rc = __session_etb(session, input_frame, false);
	mutex_unlock(&device->lock);
	return rc;
}

static int __session_ftb(struct hal_session *session,
		struct vidc_frame_data *output_frame, bool relaxed)
{
	int rc = 0;
	struct venus_hfi_device *device = session->device;
	struct hfi_cmd_session_fill_buffer_packet pkt;

	if (!__is_session_valid(device, session, __func__))
		return -EINVAL;

	rc = call_hfi_pkt_op(device, session_ftb,
			&pkt, session, output_frame);
	if (rc) {
		dprintk(VIDC_ERR, "Session ftb: failed to create pkt\n");
		goto err_create_pkt;
	}

	if (!relaxed)
		rc = __iface_cmdq_write(session->device, &pkt);
	else
		rc = __iface_cmdq_write_relaxed(session->device,
				&pkt, NULL);

err_create_pkt:
	return rc;
}

static int venus_hfi_session_ftb(void *sess,
				struct vidc_frame_data *output_frame)
{
	int rc = 0;
	struct hal_session *session = sess;
	struct venus_hfi_device *device;

	if (!session || !session->device || !output_frame) {
		dprintk(VIDC_ERR, "Invalid Params\n");
		return -EINVAL;
	}

	device = session->device;
	mutex_lock(&device->lock);
	rc = __session_ftb(session, output_frame, false);
	mutex_unlock(&device->lock);
	return rc;
}

static int venus_hfi_session_process_batch(void *sess,
		int num_etbs, struct vidc_frame_data etbs[],
		int num_ftbs, struct vidc_frame_data ftbs[])
{
	int rc = 0, c = 0;
	struct hal_session *session = sess;
	struct venus_hfi_device *device;
	struct hfi_cmd_session_sync_process_packet pkt;

	if (!session || !session->device) {
		dprintk(VIDC_ERR, "%s: Invalid Params\n", __func__);
		return -EINVAL;
	}

	device = session->device;

	mutex_lock(&device->lock);

	if (!__is_session_valid(device, session, __func__)) {
		rc = -EINVAL;
		goto err_etbs_and_ftbs;
	}

	for (c = 0; c < num_ftbs; ++c) {
		rc = __session_ftb(session, &ftbs[c], true);
		if (rc) {
			dprintk(VIDC_ERR, "Failed to queue batched ftb: %d\n",
					rc);
			goto err_etbs_and_ftbs;
		}
	}

	for (c = 0; c < num_etbs; ++c) {
		rc = __session_etb(session, &etbs[c], true);
		if (rc) {
			dprintk(VIDC_ERR, "Failed to queue batched etb: %d\n",
					rc);
			goto err_etbs_and_ftbs;
		}
	}

	rc = call_hfi_pkt_op(device, session_sync_process, &pkt, session);
	if (rc) {
		dprintk(VIDC_ERR, "Failed to create sync packet\n");
		goto err_etbs_and_ftbs;
	}

	if (__iface_cmdq_write(session->device, &pkt))
		rc = -ENOTEMPTY;

err_etbs_and_ftbs:
	mutex_unlock(&device->lock);
	return rc;
}

static int venus_hfi_session_get_buf_req(void *sess)
{
	struct hfi_cmd_session_get_property_packet pkt;
	int rc = 0;
	struct hal_session *session = sess;
	struct venus_hfi_device *device;

	if (!session || !session->device) {
		dprintk(VIDC_ERR, "invalid session");
		return -ENODEV;
	}

	device = session->device;
	mutex_lock(&device->lock);

	if (!__is_session_valid(device, session, __func__)) {
		rc = -EINVAL;
		goto err_create_pkt;
	}
	rc = call_hfi_pkt_op(device, session_get_buf_req,
			&pkt, session);
	if (rc) {
		dprintk(VIDC_ERR,
				"Session get buf req: failed to create pkt\n");
		goto err_create_pkt;
	}

	if (__iface_cmdq_write(session->device, &pkt))
		rc = -ENOTEMPTY;
err_create_pkt:
	mutex_unlock(&device->lock);
	return rc;
}

static int venus_hfi_session_flush(void *sess, enum hal_flush flush_mode)
{
	struct hfi_cmd_session_flush_packet pkt;
	int rc = 0;
	struct hal_session *session = sess;
	struct venus_hfi_device *device;

	if (!session || !session->device) {
		dprintk(VIDC_ERR, "invalid session");
		return -ENODEV;
	}

	device = session->device;
	mutex_lock(&device->lock);

	if (!__is_session_valid(device, session, __func__)) {
		rc = -EINVAL;
		goto err_create_pkt;
	}
	rc = call_hfi_pkt_op(device, session_flush,
			&pkt, session, flush_mode);
	if (rc) {
		dprintk(VIDC_ERR, "Session flush: failed to create pkt\n");
		goto err_create_pkt;
	}

	if (__iface_cmdq_write(session->device, &pkt))
		rc = -ENOTEMPTY;
err_create_pkt:
	mutex_unlock(&device->lock);
	return rc;
}

static int __check_core_registered(struct hal_device_data core,
		phys_addr_t fw_addr, u8 *reg_addr, u32 reg_size,
		phys_addr_t irq)
{
	struct venus_hfi_device *device;
	struct list_head *curr, *next;

	if (core.dev_count) {
		list_for_each_safe(curr, next, &core.dev_head) {
			device = list_entry(curr,
				struct venus_hfi_device, list);
			if (device && device->hal_data->irq == irq &&
				(CONTAINS(device->hal_data->
						firmware_base,
						FIRMWARE_SIZE, fw_addr) ||
				CONTAINS(fw_addr, FIRMWARE_SIZE,
						device->hal_data->
						firmware_base) ||
				CONTAINS(device->hal_data->
						register_base,
						reg_size, reg_addr) ||
				CONTAINS(reg_addr, reg_size,
						device->hal_data->
						register_base) ||
				OVERLAPS(device->hal_data->
						register_base,
						reg_size, reg_addr, reg_size) ||
				OVERLAPS(reg_addr, reg_size,
						device->hal_data->
						register_base, reg_size) ||
				OVERLAPS(device->hal_data->
						firmware_base,
						FIRMWARE_SIZE, fw_addr,
						FIRMWARE_SIZE) ||
				OVERLAPS(fw_addr, FIRMWARE_SIZE,
						device->hal_data->
						firmware_base,
						FIRMWARE_SIZE))) {
				return 0;
			}

			dprintk(VIDC_INFO, "Device not registered\n");
			return -EINVAL;
		}
	} else {
		dprintk(VIDC_INFO, "no device Registered\n");
	}

	return -EINVAL;
}

static void __process_fatal_error(
		struct venus_hfi_device *device)
{
	struct msm_vidc_cb_cmd_done cmd_done = {0};

	cmd_done.device_id = device->device_id;
	device->callback(HAL_SYS_ERROR, &cmd_done);
}

static int __prepare_pc(struct venus_hfi_device *device)
{
	int rc = 0;
	struct hfi_cmd_sys_pc_prep_packet pkt;

	rc = call_hfi_pkt_op(device, sys_pc_prep, &pkt);
	if (rc) {
		dprintk(VIDC_ERR, "Failed to create sys pc prep pkt\n");
		goto err_pc_prep;
	}

	if (__iface_cmdq_write(device, &pkt))
		rc = -ENOTEMPTY;
	if (rc)
		dprintk(VIDC_ERR, "Failed to prepare venus for power off");
err_pc_prep:
	return rc;
}

static void venus_hfi_pm_handler(struct work_struct *work)
{
	int rc = 0;
	u32 wfi_status = 0, idle_status = 0, pc_ready = 0;
	int count = 0;
	const int max_tries = 10;
	struct venus_hfi_device *device = list_first_entry(
			&hal_ctxt.dev_head, struct venus_hfi_device, list);

	if (!device) {
		dprintk(VIDC_ERR, "%s: NULL device\n", __func__);
		return;
	}

	dprintk(VIDC_PROF,
		"Entering venus_hfi_pm_handler\n");
	/*
	 * It is ok to check this variable outside the lock since
	 * it is being updated in this context only
	 */
	if (device->skip_pc_count >= VIDC_MAX_PC_SKIP_COUNT) {
		dprintk(VIDC_WARN, "Failed to PC for %d times\n",
				device->skip_pc_count);
		device->skip_pc_count = 0;
		__process_fatal_error(device);
		return;
	}
	mutex_lock(&device->lock);
	if (!device->power_enabled) {
		dprintk(VIDC_DBG, "%s: Power already disabled\n",
				__func__);
		goto exit;
	}

	rc = __core_in_valid_state(device);
	if (!rc) {
		dprintk(VIDC_WARN,
				"Core is in bad state, Skipping power collapse\n");
		goto skip_power_off;
	}
	pc_ready = __read_register(device, VIDC_CPU_CS_SCIACMDARG0) &
		VIDC_CPU_CS_SCIACMDARG0_HFI_CTRL_PC_READY;
	if (!pc_ready) {
		wfi_status = __read_register(device,
				VIDC_WRAPPER_CPU_STATUS);
		idle_status = __read_register(device,
				VIDC_CPU_CS_SCIACMDARG0);
		if (!(wfi_status & BIT(0))) {
			dprintk(VIDC_WARN,
				"Skipping PC as wfi_status (%#x) bit not set\n",
				wfi_status);
			goto skip_power_off;
		}
		if (device->res->sys_idle_indicator &&
			!(idle_status & BIT(30))) {
			dprintk(VIDC_WARN,
				"Skipping PC as idle_status (%#x) bit not set\n",
				idle_status);
			goto skip_power_off;
		}

		rc = __prepare_pc(device);
		if (rc) {
			dprintk(VIDC_WARN, "Failed PC %d\n", rc);
			goto skip_power_off;
		}

		while (count < max_tries) {
			wfi_status = __read_register(device,
					VIDC_WRAPPER_CPU_STATUS);
			pc_ready = __read_register(device,
					VIDC_CPU_CS_SCIACMDARG0);
			if ((wfi_status & BIT(0)) && (pc_ready &
				VIDC_CPU_CS_SCIACMDARG0_HFI_CTRL_PC_READY))
				break;
			usleep_range(150, 250);
			count++;
		}

		if (count == max_tries) {
			dprintk(VIDC_ERR,
					"Skip PC. Core is not in right state (%#x, %#x)\n",
					wfi_status, pc_ready);
			goto skip_power_off;
		}
	}

	__flush_debug_queue(device, device->raw_packet);

	rc = __suspend(device);
	if (rc)
		dprintk(VIDC_ERR, "Failed __suspend\n");

	/* Cancel pending delayed works if any */
	cancel_delayed_work(&venus_hfi_pm_work);
	device->skip_pc_count = 0;

	mutex_unlock(&device->lock);
	return;

skip_power_off:
	device->skip_pc_count++;
	dprintk(VIDC_WARN, "Skip PC(%d, %#x, %#x, %#x)\n",
		device->skip_pc_count, wfi_status, idle_status, pc_ready);
	queue_delayed_work(device->venus_pm_workq,
			&venus_hfi_pm_work,
			msecs_to_jiffies(
			device->res->msm_vidc_pwr_collapse_delay));
exit:
	mutex_unlock(&device->lock);
}

static int __halt_axi(struct venus_hfi_device *device)
{
	u32 reg;
	int rc = 0;

	if (!device) {
		dprintk(VIDC_ERR, "Invalid input\n");
		return -EINVAL;
	}

	/*
	 * Driver needs to make sure that clocks are enabled to read Venus AXI
	 * registers. If not skip AXI HALT.
	 */
	if (!device->power_enabled) {
		dprintk(VIDC_WARN,
			"Clocks are OFF, skipping AXI HALT\n");
		return -EINVAL;
	}

	/* Halt AXI and AXI IMEM VBIF Access */
	reg = __read_register(device, VENUS_WRAPPER_AXI_HALT);
	reg |= BRIC_AXI_HALT;
	__write_register(device, VENUS_WRAPPER_AXI_HALT, reg);

	/* Request for AXI bus port halt */
	rc = readl_poll_timeout(device->hal_data->register_base
			+ VENUS_WRAPPER_AXI_HALT_STATUS,
			reg, reg & BRIC_AXI_HALT_ACK,
			POLL_INTERVAL_US,
			VENUS_AXI_HALT_ACK_TIMEOUT_US);
	if (rc)
		dprintk(VIDC_WARN, "AXI bus port halt timeout\n");

	return rc;
}

static void __process_sys_error(struct venus_hfi_device *device)
{
	struct hfi_sfr_struct *vsfr = NULL;

	__set_state(device, VENUS_STATE_DEINIT);

	if (__halt_axi(device))
		dprintk(VIDC_WARN, "Failed to halt AXI after SYS_ERROR\n");

	vsfr = (struct hfi_sfr_struct *)device->sfr.align_virtual_addr;
	if (vsfr) {
		void *p = memchr(vsfr->rg_data, '\0', vsfr->bufSize);
		/*
		 * SFR isn't guaranteed to be NULL terminated
		 * since SYS_ERROR indicates that Venus is in the
		 * process of crashing.
		 */
		if (p == NULL)
			vsfr->rg_data[vsfr->bufSize - 1] = '\0';

		dprintk(VIDC_ERR, "SFR Message from FW: %s\n",
				vsfr->rg_data);
	}
}

static void __flush_debug_queue(struct venus_hfi_device *device, u8 *packet)
{
	bool local_packet = false;
	enum vidc_msg_prio log_level = VIDC_FW;

	if (!device) {
		dprintk(VIDC_ERR, "%s: Invalid params\n", __func__);
		return;
	}

	if (!packet) {
		packet = kzalloc(VIDC_IFACEQ_VAR_HUGE_PKT_SIZE, GFP_TEMPORARY);
		if (!packet) {
			dprintk(VIDC_ERR, "In %s() Fail to allocate mem\n",
				__func__);
			return;
		}

		local_packet = true;

		/*
		 * Local packek is used when something FATAL occurred.
		 * It is good to print these logs by default.
		 */

		log_level = VIDC_ERR;
	}

	while (!__iface_dbgq_read(device, packet)) {
		struct hfi_msg_sys_coverage_packet *pkt =
			(struct hfi_msg_sys_coverage_packet *) packet;

		if (pkt->packet_type == HFI_MSG_SYS_COV) {
			int stm_size = 0;

			stm_size = stm_log_inv_ts(0, 0,
				pkt->rg_msg_data, pkt->msg_size);
			if (stm_size == 0)
				dprintk(VIDC_ERR,
					"In %s, stm_log returned size of 0\n",
					__func__);
		} else {
			struct hfi_msg_sys_debug_packet *pkt =
				(struct hfi_msg_sys_debug_packet *) packet;
			dprintk(log_level, "%s", pkt->rg_msg_data);
		}
	}

	if (local_packet)
		kfree(packet);
}

static bool __is_session_valid(struct venus_hfi_device *device,
		struct hal_session *session, const char *func)
{
	struct hal_session *temp = NULL;

	if (!device || !session)
		goto invalid;

	list_for_each_entry(temp, &device->sess_head, list)
		if (session == temp)
			return true;

invalid:
	dprintk(VIDC_WARN, "%s: device %pK, invalid session %pK\n",
			func, device, session);
	return false;
}

static struct hal_session *__get_session(struct venus_hfi_device *device,
		u32 session_id)
{
	struct hal_session *temp = NULL;

	list_for_each_entry(temp, &device->sess_head, list) {
		if (session_id == hash32_ptr(temp))
			return temp;
	}

	return NULL;
}

static int __response_handler(struct venus_hfi_device *device)
{
	struct msm_vidc_cb_info *packets;
	int packet_count = 0;
	u8 *raw_packet = NULL;
	bool requeue_pm_work = true;

	if (!device || device->state != VENUS_STATE_INIT)
		return 0;

	packets = device->response_pkt;

	raw_packet = device->raw_packet;

	if (!raw_packet || !packets) {
		dprintk(VIDC_ERR,
			"%s: Invalid args : Res packet = %p, Raw packet = %p\n",
			__func__, packets, raw_packet);
		return 0;
	}

	if (device->intr_status & VIDC_WRAPPER_INTR_CLEAR_A2HWD_BMSK) {
		struct hfi_sfr_struct *vsfr = (struct hfi_sfr_struct *)
			device->sfr.align_virtual_addr;
		struct msm_vidc_cb_info info = {
			.response_type = HAL_SYS_WATCHDOG_TIMEOUT,
			.response.cmd = {
				.device_id = device->device_id,
			}
		};

		if (vsfr)
			dprintk(VIDC_ERR, "SFR Message from FW: %s\n",
					vsfr->rg_data);

		dprintk(VIDC_ERR, "Received watchdog timeout\n");
		packets[packet_count++] = info;
		goto exit;
	}

	/* Bleed the msg queue dry of packets */
	while (!__iface_msgq_read(device, raw_packet)) {
		void **session_id = NULL;
		struct msm_vidc_cb_info *info = &packets[packet_count++];
		struct vidc_hal_sys_init_done sys_init_done = {0};
		int rc = 0;

		rc = hfi_process_msg_packet(device->device_id,
			(struct vidc_hal_msg_pkt_hdr *)raw_packet, info);
		if (rc) {
			dprintk(VIDC_WARN,
					"Corrupt/unknown packet found, discarding\n");
			--packet_count;
			continue;
		}

		/* Process the packet types that we're interested in */
		switch (info->response_type) {
		case HAL_SYS_ERROR:
			__process_sys_error(device);
			break;
		case HAL_SYS_RELEASE_RESOURCE_DONE:
			dprintk(VIDC_DBG, "Received SYS_RELEASE_RESOURCE\n");
			break;
		case HAL_SYS_INIT_DONE:
			dprintk(VIDC_DBG, "Received SYS_INIT_DONE\n");

			sys_init_done.capabilities =
				device->sys_init_capabilities;
			hfi_process_sys_init_done_prop_read(
				(struct hfi_msg_sys_init_done_packet *)
					raw_packet, &sys_init_done);
			info->response.cmd.data.sys_init_done = sys_init_done;
			break;
		case HAL_SESSION_LOAD_RESOURCE_DONE:
			/*
			 * Work around for H/W bug, need to re-program these
			 * registers as part of a handshake agreement with the
			 * firmware.  This strictly only needs to be done for
			 * decoder secure sessions, but there's no harm in doing
			 * so for all sessions as it's at worst a NO-OP.
			 */
			__set_threshold_registers(device);
			break;
		default:
			break;
		}

		/* For session-related packets, validate session */
		switch (info->response_type) {
		case HAL_SESSION_LOAD_RESOURCE_DONE:
		case HAL_SESSION_INIT_DONE:
		case HAL_SESSION_END_DONE:
		case HAL_SESSION_ABORT_DONE:
		case HAL_SESSION_START_DONE:
		case HAL_SESSION_STOP_DONE:
		case HAL_SESSION_FLUSH_DONE:
		case HAL_SESSION_SUSPEND_DONE:
		case HAL_SESSION_RESUME_DONE:
		case HAL_SESSION_SET_PROP_DONE:
		case HAL_SESSION_GET_PROP_DONE:
		case HAL_SESSION_RELEASE_BUFFER_DONE:
		case HAL_SESSION_RELEASE_RESOURCE_DONE:
		case HAL_SESSION_PROPERTY_INFO:
			session_id = &info->response.cmd.session_id;
			break;
		case HAL_SESSION_ERROR:
		case HAL_SESSION_ETB_DONE:
		case HAL_SESSION_FTB_DONE:
			session_id = &info->response.data.session_id;
			break;
		case HAL_SESSION_EVENT_CHANGE:
			session_id = &info->response.event.session_id;
			break;
		case HAL_RESPONSE_UNUSED:
		default:
			session_id = NULL;
			break;
		}

		/*
		 * hfi_process_msg_packet provides a session_id that's a hashed
		 * value of struct hal_session, we need to coerce the hashed
		 * value back to pointer that we can use. Ideally, hfi_process\
		 * _msg_packet should take care of this, but it doesn't have
		 * required information for it
		 */
		if (session_id) {
			struct hal_session *session = NULL;

			if (upper_32_bits((uintptr_t)*session_id) != 0) {
				dprintk(VIDC_ERR,
					"Upper 32-bits != 0 for sess_id=%pK\n",
					*session_id);
			}
			session = __get_session(device,
					(u32)(uintptr_t)*session_id);
			if (!session) {
				dprintk(VIDC_ERR,
						"Received a packet (%#x) for an unrecognized session (%pK), discarding\n",
						info->response_type,
						*session_id);
				--packet_count;
				continue;
			}

			*session_id = session->session_id;
		}

		if (packet_count >= max_packets &&
				__get_q_size(device, VIDC_IFACEQ_MSGQ_IDX)) {
			dprintk(VIDC_WARN,
					"Too many packets in message queue to handle at once, deferring read\n");
			break;
		}
	}

	if (requeue_pm_work && device->res->sw_power_collapsible) {
		cancel_delayed_work(&venus_hfi_pm_work);
		if (!queue_delayed_work(device->venus_pm_workq,
			&venus_hfi_pm_work,
			msecs_to_jiffies(
				device->res->msm_vidc_pwr_collapse_delay))) {
			dprintk(VIDC_ERR, "PM work already scheduled\n");
		}
	}

exit:
	__flush_debug_queue(device, raw_packet);

	return packet_count;
}

static void venus_hfi_core_work_handler(struct work_struct *work)
{
	struct venus_hfi_device *device = list_first_entry(
		&hal_ctxt.dev_head, struct venus_hfi_device, list);
	int num_responses = 0, i = 0;
	u32 intr_status;

	mutex_lock(&device->lock);

	dprintk(VIDC_DBG, "Handling interrupt\n");

	if (!__core_in_valid_state(device)) {
		dprintk(VIDC_DBG, "%s - Core not in init state\n", __func__);
		goto err_no_work;
	}

	if (!device->callback) {
		dprintk(VIDC_ERR, "No interrupt callback function: %pK\n",
				device);
		goto err_no_work;
	}

	if (__resume(device)) {
		dprintk(VIDC_ERR, "%s: Power enable failed\n", __func__);
		goto err_no_work;
	}

	__core_clear_interrupt(device);
	num_responses = __response_handler(device);

err_no_work:

	/* Keep the interrupt status before releasing device lock */
	intr_status = device->intr_status;
	mutex_unlock(&device->lock);

	/*
	 * Issue the callbacks outside of the locked contex to preserve
	 * re-entrancy.
	 */

	for (i = 0; !IS_ERR_OR_NULL(device->response_pkt) &&
		i < num_responses; ++i) {
		struct msm_vidc_cb_info *r = &device->response_pkt[i];
		dprintk(VIDC_DBG, "Processing response %d of %d, type %d\n",
			(i + 1), num_responses, r->response_type);
		device->callback(r->response_type, &r->response);
	}

	/* We need re-enable the irq which was disabled in ISR handler */
	if (!(intr_status & VIDC_WRAPPER_INTR_STATUS_A2HWD_BMSK))
		enable_irq(device->hal_data->irq);

	dprintk(VIDC_DBG, "Handling interrupt done\n");
	/*
	 * XXX: Don't add any code beyond here.  Reacquiring locks after release
	 * it above doesn't guarantee the atomicity that we're aiming for.
	 */
}

static DECLARE_WORK(venus_hfi_work, venus_hfi_core_work_handler);

static irqreturn_t venus_hfi_isr(int irq, void *dev)
{
	struct venus_hfi_device *device = dev;

	dprintk(VIDC_INFO, "Received an interrupt %d\n", irq);
	disable_irq_nosync(irq);
	queue_work(device->vidc_workq, &venus_hfi_work);
	return IRQ_HANDLED;
}

static int __init_regs_and_interrupts(struct venus_hfi_device *device,
		struct msm_vidc_platform_resources *res)
{
	struct hal_data *hal = NULL;
	int rc = 0;

	rc = __check_core_registered(hal_ctxt, res->firmware_base,
			(u8 *)(uintptr_t)res->register_base,
			res->register_size, res->irq);
	if (!rc) {
		dprintk(VIDC_ERR, "Core present/Already added\n");
		rc = -EEXIST;
		goto err_core_init;
	}

	dprintk(VIDC_DBG, "HAL_DATA will be assigned now\n");
	hal = (struct hal_data *)
		kzalloc(sizeof(struct hal_data), GFP_KERNEL);
	if (!hal) {
		dprintk(VIDC_ERR, "Failed to alloc\n");
		rc = -ENOMEM;
		goto err_core_init;
	}

	hal->irq = res->irq;
	hal->firmware_base = res->firmware_base;
	hal->register_base = devm_ioremap_nocache(&res->pdev->dev,
			res->register_base, res->register_size);
	hal->register_size = res->register_size;
	if (!hal->register_base) {
		dprintk(VIDC_ERR,
			"could not map reg addr %pa of size %d\n",
			&res->register_base, res->register_size);
		goto error_irq_fail;
	}

	device->hal_data = hal;
	rc = request_irq(res->irq, venus_hfi_isr, IRQF_TRIGGER_HIGH,
			"msm_vidc", device);
	if (unlikely(rc)) {
		dprintk(VIDC_ERR, "() :request_irq failed\n");
		goto error_irq_fail;
	}

	disable_irq_nosync(res->irq);
	dprintk(VIDC_INFO,
		"firmware_base = %pa, register_base = %pa, register_size = %d\n",
		&res->firmware_base, &res->register_base,
		res->register_size);
	return rc;

error_irq_fail:
	kfree(hal);
err_core_init:
	return rc;

}

static inline void __deinit_clocks(struct venus_hfi_device *device)
{
	struct clock_info *cl;

	device->clk_freq = 0;
	venus_hfi_for_each_clock_reverse(device, cl) {
		if (cl->clk) {
			clk_put(cl->clk);
			cl->clk = NULL;
		}
	}
}

static inline int __init_clocks(struct venus_hfi_device *device)
{
	int rc = 0;
	struct clock_info *cl = NULL;

	if (!device) {
		dprintk(VIDC_ERR, "Invalid params: %pK\n", device);
		return -EINVAL;
	}

	venus_hfi_for_each_clock(device, cl) {

		dprintk(VIDC_DBG, "%s: scalable? %d, count %d\n",
				cl->name, cl->has_scaling, cl->count);
	}

	venus_hfi_for_each_clock(device, cl) {
		if (!cl->clk) {
			cl->clk = clk_get(&device->res->pdev->dev, cl->name);
			if (IS_ERR_OR_NULL(cl->clk)) {
				dprintk(VIDC_ERR,
					"Failed to get clock: %s\n", cl->name);
				rc = PTR_ERR(cl->clk) ?: -EINVAL;
				cl->clk = NULL;
				goto err_clk_get;
			}
		}
	}
	device->clk_freq = 0;
	return 0;

err_clk_get:
	__deinit_clocks(device);
	return rc;
}


static inline void __disable_unprepare_clks(struct venus_hfi_device *device)
{
	struct clock_info *cl;
	int rc = 0;

	if (!device) {
		dprintk(VIDC_ERR, "Invalid params: %pK\n", device);
		return;
	}

	venus_hfi_for_each_clock_reverse(device, cl) {
		if (cl->disable_memcore_only)
			continue;
		dprintk(VIDC_DBG, "Clock: %s disable and unprepare\n",
				cl->name);
		rc = clk_set_flags(cl->clk, CLKFLAG_NORETAIN_PERIPH);
		if (rc) {
			dprintk(VIDC_WARN,
				"Failed set flag NORETAIN_PERIPH %s\n",
					cl->name);
		}
		rc = clk_set_flags(cl->clk, CLKFLAG_NORETAIN_MEM);
		if (rc) {
			dprintk(VIDC_WARN,
				"Failed set flag NORETAIN_MEM %s\n",
					cl->name);
		}
		clk_disable_unprepare(cl->clk);
	}
}

static inline int __prepare_enable_clks(struct venus_hfi_device *device)
{
	struct clock_info *cl = NULL, *cl_fail = NULL;
	int rc = 0, c = 0;

	if (!device) {
		dprintk(VIDC_ERR, "Invalid params: %pK\n", device);
		return -EINVAL;
	}

	venus_hfi_for_each_clock(device, cl) {
		/* MEM CORE is ON by default. Unset it for unused clocks*/
		if (cl->disable_memcore_only) {
			clk_set_flags(cl->clk, CLKFLAG_NORETAIN_MEM);
			continue;
		}
		/*
		 * For the clocks we control, set the rate prior to preparing
		 * them.  Since we don't really have a load at this point, scale
		 * it to the lowest frequency possible
		 */
		if (cl->has_scaling)
			clk_set_rate(cl->clk, clk_round_rate(cl->clk, 0));

		rc = clk_set_flags(cl->clk, CLKFLAG_RETAIN_PERIPH);
		if (rc) {
			dprintk(VIDC_WARN,
				"Failed set flag RETAIN_PERIPH %s\n",
					cl->name);
		}
		rc = clk_set_flags(cl->clk, CLKFLAG_RETAIN_MEM);
		if (rc) {
			dprintk(VIDC_WARN,
				"Failed set flag RETAIN_MEM %s\n",
					cl->name);
		}
		rc = clk_prepare_enable(cl->clk);
		if (rc) {
			dprintk(VIDC_ERR, "Failed to enable clocks\n");
			cl_fail = cl;
			goto fail_clk_enable;
		}

		c++;
		dprintk(VIDC_DBG, "Clock: %s prepared and enabled\n", cl->name);
	}

	__write_register(device, VIDC_WRAPPER_CLOCK_CONFIG, 0);
	__write_register(device, VIDC_WRAPPER_CPU_CLOCK_CONFIG, 0);
	return rc;

fail_clk_enable:
	venus_hfi_for_each_clock_reverse_continue(device, cl, c) {
		if (cl->disable_memcore_only)
			continue;
		dprintk(VIDC_ERR, "Clock: %s disable and unprepare\n",
			cl->name);
		clk_disable_unprepare(cl->clk);
	}

	return rc;
}

static void __deinit_bus(struct venus_hfi_device *device)
{
	struct bus_info *bus = NULL;

	if (!device)
		return;

	kfree(device->bus_vote.data);
	device->bus_vote = DEFAULT_BUS_VOTE;

	venus_hfi_for_each_bus_reverse(device, bus) {
		devfreq_remove_device(bus->devfreq);
		bus->devfreq = NULL;
		dev_set_drvdata(bus->dev, NULL);

		msm_bus_scale_unregister(bus->client);
		bus->client = NULL;
	}
}

static int __init_bus(struct venus_hfi_device *device)
{
	struct bus_info *bus = NULL;
	int rc = 0;

	if (!device)
		return -EINVAL;

	venus_hfi_for_each_bus(device, bus) {
		struct devfreq_dev_profile profile = {
			.initial_freq = 0,
			.polling_ms = INT_MAX,
			.freq_table = NULL,
			.max_state = 0,
			.target = __devfreq_target,
			.get_dev_status = __devfreq_get_status,
			.exit = NULL,
		};

		if (!strcmp(bus->governor, "msm-vidc-llcc")) {
			if (msm_vidc_syscache_disable) {
				dprintk(VIDC_DBG,
					 "Skipping LLC bus init %s: %s\n",
				bus->name, bus->governor);
				continue;
			}
		}

		/*
		 * This is stupid, but there's no other easy way to ahold
		 * of struct bus_info in venus_hfi_devfreq_*()
		 */
		WARN(dev_get_drvdata(bus->dev), "%s's drvdata already set\n",
				dev_name(bus->dev));
		dev_set_drvdata(bus->dev, device);

		bus->client = msm_bus_scale_register(bus->master, bus->slave,
				bus->name, false);
		if (IS_ERR_OR_NULL(bus->client)) {
			rc = PTR_ERR(bus->client) ?: -EBADHANDLE;
			dprintk(VIDC_ERR, "Failed to register bus %s: %d\n",
					bus->name, rc);
			bus->client = NULL;
			goto err_add_dev;
		}

		bus->devfreq_prof = profile;
		bus->devfreq = devfreq_add_device(bus->dev,
				&bus->devfreq_prof, bus->governor, NULL);
		if (IS_ERR_OR_NULL(bus->devfreq)) {
			rc = PTR_ERR(bus->devfreq) ?: -EBADHANDLE;
			dprintk(VIDC_ERR,
					"Failed to add devfreq device for bus %s and governor %s: %d\n",
					bus->name, bus->governor, rc);
			bus->devfreq = NULL;
			goto err_add_dev;
		}

		/*
		 * Devfreq starts monitoring immediately, since we are just
		 * initializing stuff at this point, force it to suspend
		 */
		devfreq_suspend_device(bus->devfreq);
	}

	return 0;

err_add_dev:
	__deinit_bus(device);
	return rc;
}

static void __deinit_regulators(struct venus_hfi_device *device)
{
	struct regulator_info *rinfo = NULL;

	venus_hfi_for_each_regulator_reverse(device, rinfo) {
		if (rinfo->regulator) {
			regulator_put(rinfo->regulator);
			rinfo->regulator = NULL;
		}
	}
}

static int __init_regulators(struct venus_hfi_device *device)
{
	int rc = 0;
	struct regulator_info *rinfo = NULL;

	venus_hfi_for_each_regulator(device, rinfo) {
		rinfo->regulator = regulator_get(&device->res->pdev->dev,
				rinfo->name);
		if (IS_ERR_OR_NULL(rinfo->regulator)) {
			rc = PTR_ERR(rinfo->regulator) ?: -EBADHANDLE;
			dprintk(VIDC_ERR, "Failed to get regulator: %s\n",
					rinfo->name);
			rinfo->regulator = NULL;
			goto err_reg_get;
		}
	}

	return 0;

err_reg_get:
	__deinit_regulators(device);
	return rc;
}

static void __deinit_subcaches(struct venus_hfi_device *device)
{
	struct subcache_info *sinfo = NULL;

	if (!device) {
		dprintk(VIDC_ERR, "deinit_subcaches: invalid device %pK\n",
			device);
		goto exit;
	}

	if (!is_sys_cache_present(device))
		goto exit;

	venus_hfi_for_each_subcache_reverse(device, sinfo) {
		if (sinfo->subcache) {
			dprintk(VIDC_DBG, "deinit_subcaches: %s\n",
				sinfo->name);
			llcc_slice_putd(sinfo->subcache);
			sinfo->subcache = NULL;
		}
	}

exit:
	return;
}

static int __init_subcaches(struct venus_hfi_device *device)
{
	int rc = 0;
	struct subcache_info *sinfo = NULL;

	if (!device) {
		dprintk(VIDC_ERR, "init_subcaches: invalid device %pK\n",
			device);
		return -EINVAL;
	}

	if (!is_sys_cache_present(device))
		return 0;

	venus_hfi_for_each_subcache(device, sinfo) {
		sinfo->subcache = llcc_slice_getd(&device->res->pdev->dev,
			sinfo->name);
		if (IS_ERR_OR_NULL(sinfo->subcache)) {
			rc = PTR_ERR(sinfo->subcache) ? : -EBADHANDLE;
			dprintk(VIDC_ERR,
				 "init_subcaches: invalid subcache: %s rc %d\n",
				sinfo->name, rc);
			sinfo->subcache = NULL;
			goto err_subcache_get;
		}
		dprintk(VIDC_DBG, "init_subcaches: %s\n",
			sinfo->name);
	}

	return 0;

err_subcache_get:
	__deinit_subcaches(device);
	return rc;
}

static int __init_resources(struct venus_hfi_device *device,
				struct msm_vidc_platform_resources *res)
{
	int rc = 0;

	rc = __init_regulators(device);
	if (rc) {
		dprintk(VIDC_ERR, "Failed to get all regulators\n");
		return -ENODEV;
	}

	rc = __init_clocks(device);
	if (rc) {
		dprintk(VIDC_ERR, "Failed to init clocks\n");
		rc = -ENODEV;
		goto err_init_clocks;
	}

	rc = __init_bus(device);
	if (rc) {
		dprintk(VIDC_ERR, "Failed to init bus: %d\n", rc);
		goto err_init_bus;
	}

	rc = __init_subcaches(device);
	if (rc)
		dprintk(VIDC_WARN, "Failed to init subcaches: %d\n", rc);

	device->sys_init_capabilities =
		kzalloc(sizeof(struct msm_vidc_capability)
		* VIDC_MAX_SESSIONS, GFP_TEMPORARY);

	return rc;

err_init_bus:
	__deinit_clocks(device);
err_init_clocks:
	__deinit_regulators(device);
	return rc;
}

static void __deinit_resources(struct venus_hfi_device *device)
{
	__deinit_subcaches(device);
	__deinit_bus(device);
	__deinit_clocks(device);
	__deinit_regulators(device);
	kfree(device->sys_init_capabilities);
	device->sys_init_capabilities = NULL;
}

static int __protect_cp_mem(struct venus_hfi_device *device)
{
	struct tzbsp_memprot memprot;
	unsigned int resp = 0;
	int rc = 0;
	struct context_bank_info *cb;
	struct scm_desc desc = {0};

	if (!device)
		return -EINVAL;

	memprot.cp_start = 0x0;
	memprot.cp_size = 0x0;
	memprot.cp_nonpixel_start = 0x0;
	memprot.cp_nonpixel_size = 0x0;

	list_for_each_entry(cb, &device->res->context_banks, list) {
		if (!strcmp(cb->name, "venus_ns")) {
			desc.args[1] = memprot.cp_size =
				cb->addr_range.start;
			dprintk(VIDC_DBG, "%s memprot.cp_size: %#x\n",
				__func__, memprot.cp_size);
		}

		if (!strcmp(cb->name, "venus_sec_non_pixel")) {
			desc.args[2] = memprot.cp_nonpixel_start =
				cb->addr_range.start;
			desc.args[3] = memprot.cp_nonpixel_size =
				cb->addr_range.size;
			dprintk(VIDC_DBG,
				"%s memprot.cp_nonpixel_start: %#x size: %#x\n",
				__func__, memprot.cp_nonpixel_start,
				memprot.cp_nonpixel_size);
		}
	}

	if (!is_scm_armv8()) {
		rc = scm_call(SCM_SVC_MP, TZBSP_MEM_PROTECT_VIDEO_VAR, &memprot,
			sizeof(memprot), &resp, sizeof(resp));
	} else {
		desc.arginfo = SCM_ARGS(4);
		rc = scm_call2(SCM_SIP_FNID(SCM_SVC_MP,
			       TZBSP_MEM_PROTECT_VIDEO_VAR), &desc);
		resp = desc.ret[0];
	}

	if (rc) {
		dprintk(VIDC_ERR, "Failed to protect memory(%d) response: %d\n",
				rc, resp);
	}

	trace_venus_hfi_var_done(
		memprot.cp_start, memprot.cp_size,
		memprot.cp_nonpixel_start, memprot.cp_nonpixel_size);
	return rc;
}

static int __disable_regulator(struct regulator_info *rinfo,
				struct venus_hfi_device *device)
{
	int rc = 0;

	dprintk(VIDC_DBG, "Disabling regulator %s\n", rinfo->name);

	/*
	 * This call is needed. Driver needs to acquire the control back
	 * from HW in order to disable the regualtor. Else the behavior
	 * is unknown.
	 */

	rc = __acquire_regulator(rinfo, device);
	if (rc) {
		/*
		 * This is somewhat fatal, but nothing we can do
		 * about it. We can't disable the regulator w/o
		 * getting it back under s/w control
		 */
		dprintk(VIDC_WARN,
			"Failed to acquire control on %s\n",
			rinfo->name);

		goto disable_regulator_failed;
	}

	rc = regulator_disable(rinfo->regulator);
	if (rc) {
		dprintk(VIDC_WARN,
			"Failed to disable %s: %d\n",
			rinfo->name, rc);
		goto disable_regulator_failed;
	}

	return 0;
disable_regulator_failed:

	/* Bring attention to this issue */
	msm_vidc_res_handle_fatal_hw_error(device->res, true);
	return rc;
}

static int __enable_hw_power_collapse(struct venus_hfi_device *device)
{
	int rc = 0;

	if (!msm_vidc_fw_low_power_mode) {
		dprintk(VIDC_DBG, "Not enabling hardware power collapse\n");
		return 0;
	}

	rc = __hand_off_regulators(device);
	if (rc)
		dprintk(VIDC_WARN,
			"%s : Failed to enable HW power collapse %d\n",
				__func__, rc);
	return rc;
}

static int __enable_regulators(struct venus_hfi_device *device)
{
	int rc = 0, c = 0;
	struct regulator_info *rinfo;

	dprintk(VIDC_DBG, "Enabling regulators\n");

	venus_hfi_for_each_regulator(device, rinfo) {
		rc = regulator_enable(rinfo->regulator);
		if (rc) {
			dprintk(VIDC_ERR,
					"Failed to enable %s: %d\n",
					rinfo->name, rc);
			goto err_reg_enable_failed;
		}

		dprintk(VIDC_DBG, "Enabled regulator %s\n",
				rinfo->name);
		c++;
	}

	return 0;

err_reg_enable_failed:
	venus_hfi_for_each_regulator_reverse_continue(device, rinfo, c)
		__disable_regulator(rinfo, device);

	return rc;
}

static int __disable_regulators(struct venus_hfi_device *device)
{
	struct regulator_info *rinfo;
	int rc = 0;

	dprintk(VIDC_DBG, "Disabling regulators\n");

	venus_hfi_for_each_regulator_reverse(device, rinfo)
		__disable_regulator(rinfo, device);

	return rc;
}

static int __enable_subcaches(struct venus_hfi_device *device)
{
	int rc = 0;
	u32 c = 0;
	struct subcache_info *sinfo;

	if (msm_vidc_syscache_disable || !is_sys_cache_present(device))
		return 0;

	/* Activate subcaches */
	venus_hfi_for_each_subcache(device, sinfo) {
		rc = llcc_slice_activate(sinfo->subcache);
		if (rc) {
			dprintk(VIDC_WARN, "Failed to activate %s: %d\n",
				sinfo->name, rc);
			msm_vidc_res_handle_fatal_hw_error(device->res, true);
			goto err_activate_fail;
		}
		sinfo->isactive = true;
		dprintk(VIDC_DBG, "Activated subcache %s\n", sinfo->name);
		c++;
	}

	dprintk(VIDC_DBG, "Activated %d Subcaches to Venus\n", c);

	return 0;

err_activate_fail:
	__release_subcaches(device);
	__disable_subcaches(device);
	return 0;
}

static int __set_subcaches(struct venus_hfi_device *device)
{
	int rc = 0;
	u32 c = 0;
	struct subcache_info *sinfo;
	u32 resource[VIDC_MAX_SUBCACHE_SIZE];
	struct hfi_resource_syscache_info_type *sc_res_info;
	struct hfi_resource_subcache_type *sc_res;
	struct vidc_resource_hdr rhdr;

	if (device->res->sys_cache_res_set) {
		dprintk(VIDC_DBG, "Subcaches already set to Venus\n");
		return 0;
	}

	memset((void *)resource, 0x0, (sizeof(u32) * VIDC_MAX_SUBCACHE_SIZE));

	sc_res_info = (struct hfi_resource_syscache_info_type *)resource;
	sc_res = &(sc_res_info->rg_subcache_entries[0]);

	venus_hfi_for_each_subcache(device, sinfo) {
		if (sinfo->isactive == true) {
			sc_res[c].size = sinfo->subcache->llcc_slice_size;
			sc_res[c].sc_id = sinfo->subcache->llcc_slice_id;
			c++;
		}
	}

	/* Set resource to Venus for activated subcaches */
	if (c) {
		dprintk(VIDC_DBG, "Setting %d Subcaches\n", c);

		rhdr.resource_handle = sc_res_info; /* cookie */
		rhdr.resource_id = VIDC_RESOURCE_SYSCACHE;

		sc_res_info->num_entries = c;

		rc = __core_set_resource(device, &rhdr, (void *)sc_res_info);
		if (rc) {
			dprintk(VIDC_WARN, "Failed to set subcaches %d\n", rc);
			goto err_fail_set_subacaches;
		}

		venus_hfi_for_each_subcache(device, sinfo) {
			if (sinfo->isactive == true)
				sinfo->isset = true;
		}

		dprintk(VIDC_DBG, "Set Subcaches done to Venus\n");
		device->res->sys_cache_res_set = true;
	}

	return 0;

err_fail_set_subacaches:
	__disable_subcaches(device);

	return 0;
}

static int __release_subcaches(struct venus_hfi_device *device)
{
	struct subcache_info *sinfo;
	int rc = 0;
	u32 c = 0;
	u32 resource[VIDC_MAX_SUBCACHE_SIZE];
	struct hfi_resource_syscache_info_type *sc_res_info;
	struct hfi_resource_subcache_type *sc_res;
	struct vidc_resource_hdr rhdr;

	if (msm_vidc_syscache_disable || !is_sys_cache_present(device))
		return 0;

	memset((void *)resource, 0x0, (sizeof(u32) * VIDC_MAX_SUBCACHE_SIZE));

	sc_res_info = (struct hfi_resource_syscache_info_type *)resource;
	sc_res = &(sc_res_info->rg_subcache_entries[0]);

	/* Release resource command to Venus */
	venus_hfi_for_each_subcache_reverse(device, sinfo) {
		if (sinfo->isset == true) {
			/* Update the entry */
			sc_res[c].size = sinfo->subcache->llcc_slice_size;
			sc_res[c].sc_id = sinfo->subcache->llcc_slice_id;
			c++;
			sinfo->isset = false;
		}
	}

	if (c > 0) {
		dprintk(VIDC_DBG, "Releasing %d subcaches\n", c);
		rhdr.resource_handle = sc_res_info; /* cookie */
		rhdr.resource_id = VIDC_RESOURCE_SYSCACHE;

		rc = __core_release_resource(device, &rhdr);
		if (rc)
			dprintk(VIDC_WARN,
				"Failed to release %d subcaches\n", c);
	}

	device->res->sys_cache_res_set = false;

	return 0;
}

static int __disable_subcaches(struct venus_hfi_device *device)
{
	struct subcache_info *sinfo;
	int rc = 0;

	if (msm_vidc_syscache_disable || !is_sys_cache_present(device))
		return 0;

	/* De-activate subcaches */
	venus_hfi_for_each_subcache_reverse(device, sinfo) {
		if (sinfo->isactive == true) {
			dprintk(VIDC_DBG, "De-activate subcache %s\n",
				sinfo->name);
			rc = llcc_slice_deactivate(sinfo->subcache);
			if (rc) {
				dprintk(VIDC_WARN,
					"Failed to de-activate %s: %d\n",
					sinfo->name, rc);
			}
			sinfo->isactive = false;
		}
	}

	return 0;
}

static int __venus_power_on(struct venus_hfi_device *device)
{
	int rc = 0;

	if (device->power_enabled)
		return 0;

	device->power_enabled = true;
	/* Vote for all hardware resources */
	rc = __vote_buses(device, device->bus_vote.data,
			device->bus_vote.data_count);
	if (rc) {
		dprintk(VIDC_ERR, "Failed to vote buses, err: %d\n", rc);
		goto fail_vote_buses;
	}

	rc = __enable_regulators(device);
	if (rc) {
		dprintk(VIDC_ERR, "Failed to enable GDSC, err = %d\n", rc);
		goto fail_enable_gdsc;
	}

	rc = __prepare_enable_clks(device);
	if (rc) {
		dprintk(VIDC_ERR, "Failed to enable clocks: %d\n", rc);
		goto fail_enable_clks;
	}

	rc = __scale_clocks(device);
	if (rc) {
		dprintk(VIDC_WARN,
				"Failed to scale clocks, performance might be affected\n");
		rc = 0;
	}

	/*
	 * Re-program all of the registers that get reset as a result of
	 * regulator_disable() and _enable()
	 */
	__set_registers(device);

	__write_register(device, VIDC_WRAPPER_INTR_MASK,
			VIDC_WRAPPER_INTR_MASK_A2HVCODEC_BMSK);
	device->intr_status = 0;
	enable_irq(device->hal_data->irq);

	/*
	 * Hand off control of regulators to h/w _after_ enabling clocks.
	 * Note that the GDSC will turn off when switching from normal
	 * (s/w triggered) to fast (HW triggered) unless the h/w vote is
	 * present. Since Venus isn't up yet, the GDSC will be off briefly.
	 */
	if (__enable_hw_power_collapse(device))
		dprintk(VIDC_ERR, "Failed to enabled inter-frame PC\n");

	return rc;

fail_enable_clks:
	__disable_regulators(device);
fail_enable_gdsc:
	__unvote_buses(device);
fail_vote_buses:
	device->power_enabled = false;
	return rc;
}

static void __venus_power_off(struct venus_hfi_device *device)
{
	if (!device->power_enabled)
		return;

	if (!(device->intr_status & VIDC_WRAPPER_INTR_STATUS_A2HWD_BMSK))
		disable_irq_nosync(device->hal_data->irq);
	device->intr_status = 0;

	__disable_unprepare_clks(device);
	if (__disable_regulators(device))
		dprintk(VIDC_WARN, "Failed to disable regulators\n");

	if (__unvote_buses(device))
		dprintk(VIDC_WARN, "Failed to unvote for buses\n");
	device->power_enabled = false;
}

static inline int __suspend(struct venus_hfi_device *device)
{
	int rc = 0;

	if (!device) {
		dprintk(VIDC_ERR, "Invalid params: %pK\n", device);
		return -EINVAL;
	} else if (!device->power_enabled) {
		dprintk(VIDC_DBG, "Power already disabled\n");
		return 0;
	}

	dprintk(VIDC_PROF, "Entering suspend\n");

	if (device->res->pm_qos_latency_us &&
		pm_qos_request_active(&device->qos))
		pm_qos_remove_request(&device->qos);

	rc = __tzbsp_set_video_state(TZBSP_VIDEO_STATE_SUSPEND);
	if (rc) {
		dprintk(VIDC_WARN, "Failed to suspend video core %d\n", rc);
		goto err_tzbsp_suspend;
	}

	__disable_subcaches(device);

	__venus_power_off(device);
	dprintk(VIDC_PROF, "Venus power off\n");
	return rc;

err_tzbsp_suspend:
	return rc;
}

static inline int __resume(struct venus_hfi_device *device)
{
	int rc = 0;

	if (!device) {
		dprintk(VIDC_ERR, "Invalid params: %pK\n", device);
		return -EINVAL;
	} else if (device->power_enabled) {
		goto exit;
	} else if (!__core_in_valid_state(device)) {
		dprintk(VIDC_DBG, "venus_hfi_device in deinit state.");
		return -EINVAL;
	}

	dprintk(VIDC_PROF, "Resuming from power collapse\n");
	rc = __venus_power_on(device);
	if (rc) {
		dprintk(VIDC_ERR, "Failed to power on venus\n");
		goto err_venus_power_on;
	}

	/* Reboot the firmware */
	rc = __tzbsp_set_video_state(TZBSP_VIDEO_STATE_RESUME);
	if (rc) {
		dprintk(VIDC_ERR, "Failed to resume video core %d\n", rc);
		goto err_set_video_state;
	}

	__setup_ucregion_memory_map(device);
	/* Wait for boot completion */
	rc = __boot_firmware(device);
	if (rc) {
		dprintk(VIDC_ERR, "Failed to reset venus core\n");
		goto err_reset_core;
	}

	/*
	 * Work around for H/W bug, need to reprogram these registers once
	 * firmware is out reset
	 */
	__set_threshold_registers(device);

	if (device->res->pm_qos_latency_us) {
#ifdef CONFIG_SMP
		device->qos.type = PM_QOS_REQ_AFFINE_IRQ;
		device->qos.irq = device->hal_data->irq;
#endif
		pm_qos_add_request(&device->qos, PM_QOS_CPU_DMA_LATENCY,
				device->res->pm_qos_latency_us);
	}

	__sys_set_debug(device, msm_vidc_fw_debug);

	__enable_subcaches(device);
	__set_subcaches(device);

	dprintk(VIDC_PROF, "Resumed from power collapse\n");
exit:
	/* Don't reset skip_pc_count for SYS_PC_PREP cmd */
	if (device->last_packet_type != HFI_CMD_SYS_PC_PREP)
		device->skip_pc_count = 0;
	return rc;
err_reset_core:
	__tzbsp_set_video_state(TZBSP_VIDEO_STATE_SUSPEND);
err_set_video_state:
	__venus_power_off(device);
err_venus_power_on:
	dprintk(VIDC_ERR, "Failed to resume from power collapse\n");
	return rc;
}

static int __load_fw(struct venus_hfi_device *device)
{
	int rc = 0;

	/* Initialize resources */
	rc = __init_resources(device, device->res);
	if (rc) {
		dprintk(VIDC_ERR, "Failed to init resources: %d\n", rc);
		goto fail_init_res;
	}

	rc = __initialize_packetization(device);
	if (rc) {
		dprintk(VIDC_ERR, "Failed to initialize packetization\n");
		goto fail_init_pkt;
	}
	trace_msm_v4l2_vidc_fw_load_start("msm_v4l2_vidc venus_fw load start");

	rc = __venus_power_on(device);
	if (rc) {
		dprintk(VIDC_ERR, "Failed to power on venus in in load_fw\n");
		goto fail_venus_power_on;
	}

	if ((!device->res->use_non_secure_pil && !device->res->firmware_base)
			|| device->res->use_non_secure_pil) {
		if (!device->resources.fw.cookie)
			device->resources.fw.cookie =
				subsystem_get_with_fwname("venus",
				device->res->fw_name);

		if (IS_ERR_OR_NULL(device->resources.fw.cookie)) {
			dprintk(VIDC_ERR, "Failed to download firmware\n");
			device->resources.fw.cookie = NULL;
			rc = -ENOMEM;
			goto fail_load_fw;
		}
	}

	if (!device->res->use_non_secure_pil && !device->res->firmware_base) {
		rc = __protect_cp_mem(device);
		if (rc) {
			dprintk(VIDC_ERR, "Failed to protect memory\n");
			goto fail_protect_mem;
		}
	}
	trace_msm_v4l2_vidc_fw_load_end("msm_v4l2_vidc venus_fw load end");
	return rc;
fail_protect_mem:
	if (device->resources.fw.cookie)
		subsystem_put(device->resources.fw.cookie);
	device->resources.fw.cookie = NULL;
fail_load_fw:
	__venus_power_off(device);
fail_venus_power_on:
fail_init_pkt:
	__deinit_resources(device);
fail_init_res:
	trace_msm_v4l2_vidc_fw_load_end("msm_v4l2_vidc venus_fw load end");
	return rc;
}

static void __unload_fw(struct venus_hfi_device *device)
{
	if (!device->resources.fw.cookie)
		return;

	cancel_delayed_work(&venus_hfi_pm_work);
	if (device->state != VENUS_STATE_DEINIT)
		flush_workqueue(device->venus_pm_workq);

	__vote_buses(device, NULL, 0);
	subsystem_put(device->resources.fw.cookie);
	__interface_queues_release(device);
	__venus_power_off(device);
	device->resources.fw.cookie = NULL;
	__deinit_resources(device);

	dprintk(VIDC_PROF, "Firmware unloaded successfully\n");
}

static int venus_hfi_get_fw_info(void *dev, struct hal_fw_info *fw_info)
{
	int i = 0, j = 0;
	struct venus_hfi_device *device = dev;
	u32 smem_block_size = 0;
	u8 *smem_table_ptr;
	char version[VENUS_VERSION_LENGTH] = "";
	const u32 smem_image_index_venus = 14 * 128;

	if (!device || !fw_info) {
		dprintk(VIDC_ERR,
			"%s Invalid parameter: device = %pK fw_info = %pK\n",
			__func__, device, fw_info);
		return -EINVAL;
	}

	mutex_lock(&device->lock);

	smem_table_ptr = smem_get_entry(SMEM_IMAGE_VERSION_TABLE,
			&smem_block_size, 0, SMEM_ANY_HOST_FLAG);
	if (smem_table_ptr &&
			((smem_image_index_venus +
			  VENUS_VERSION_LENGTH) <= smem_block_size))
		memcpy(version,
			smem_table_ptr + smem_image_index_venus,
			VENUS_VERSION_LENGTH);

	while (version[i++] != 'V' && i < VENUS_VERSION_LENGTH)
		;

	if (i == VENUS_VERSION_LENGTH - 1) {
		dprintk(VIDC_WARN, "Venus version string is not proper\n");
		fw_info->version[0] = '\0';
		goto fail_version_string;
	}

	for (i--; i < VENUS_VERSION_LENGTH && j < VENUS_VERSION_LENGTH - 1; i++)
		fw_info->version[j++] = version[i];
	fw_info->version[j] = '\0';

fail_version_string:
	dprintk(VIDC_DBG, "F/W version retrieved : %s\n", fw_info->version);
	fw_info->base_addr = device->hal_data->firmware_base;
	fw_info->register_base = device->res->register_base;
	fw_info->register_size = device->hal_data->register_size;
	fw_info->irq = device->hal_data->irq;

	mutex_unlock(&device->lock);
	return 0;
}

static int venus_hfi_get_core_capabilities(void *dev)
{
	struct venus_hfi_device *device = dev;
	int rc = 0;

	if (!device)
		return -EINVAL;

	mutex_lock(&device->lock);

	rc = HAL_VIDEO_ENCODER_ROTATION_CAPABILITY |
		HAL_VIDEO_ENCODER_SCALING_CAPABILITY |
		HAL_VIDEO_ENCODER_DEINTERLACE_CAPABILITY |
		HAL_VIDEO_DECODER_MULTI_STREAM_CAPABILITY;

	mutex_unlock(&device->lock);

	return rc;
}

static void __noc_error_info(struct venus_hfi_device *device, u32 core_num)
{
	u32 vcodec_core_video_noc_base_offs, val;

	if (!device) {
		dprintk(VIDC_ERR, "%s: null device\n", __func__);
		return;
	}
	if (!core_num) {
		vcodec_core_video_noc_base_offs =
			VCODEC_CORE0_VIDEO_NOC_BASE_OFFS;
	} else if (core_num == 1) {
		vcodec_core_video_noc_base_offs =
			VCODEC_CORE1_VIDEO_NOC_BASE_OFFS;
	} else {
		dprintk(VIDC_ERR, "%s: invalid core_num %u\n",
			__func__, core_num);
		return;
	}

	val = __read_register(device, vcodec_core_video_noc_base_offs +
			VCODEC_COREX_VIDEO_NOC_ERR_SWID_LOW_OFFS);
	dprintk(VIDC_ERR, "CORE%d_NOC_ERR_SWID_LOW:     %#x\n", core_num, val);
	val = __read_register(device, vcodec_core_video_noc_base_offs +
			VCODEC_COREX_VIDEO_NOC_ERR_SWID_HIGH_OFFS);
	dprintk(VIDC_ERR, "CORE%d_NOC_ERR_SWID_HIGH:    %#x\n", core_num, val);
	val = __read_register(device, vcodec_core_video_noc_base_offs +
			VCODEC_COREX_VIDEO_NOC_ERR_MAINCTL_LOW_OFFS);
	dprintk(VIDC_ERR, "CORE%d_NOC_ERR_MAINCTL_LOW:  %#x\n", core_num, val);
	val = __read_register(device, vcodec_core_video_noc_base_offs +
			VCODEC_COREX_VIDEO_NOC_ERR_ERRLOG0_LOW_OFFS);
	dprintk(VIDC_ERR, "CORE%d_NOC_ERR_ERRLOG0_LOW:  %#x\n", core_num, val);
	val = __read_register(device, vcodec_core_video_noc_base_offs +
			VCODEC_COREX_VIDEO_NOC_ERR_ERRLOG0_HIGH_OFFS);
	dprintk(VIDC_ERR, "CORE%d_NOC_ERR_ERRLOG0_HIGH: %#x\n", core_num, val);
	val = __read_register(device, vcodec_core_video_noc_base_offs +
			VCODEC_COREX_VIDEO_NOC_ERR_ERRLOG1_LOW_OFFS);
	dprintk(VIDC_ERR, "CORE%d_NOC_ERR_ERRLOG1_LOW:  %#x\n", core_num, val);
	val = __read_register(device, vcodec_core_video_noc_base_offs +
			VCODEC_COREX_VIDEO_NOC_ERR_ERRLOG1_HIGH_OFFS);
	dprintk(VIDC_ERR, "CORE%d_NOC_ERR_ERRLOG1_HIGH: %#x\n", core_num, val);
	val = __read_register(device, vcodec_core_video_noc_base_offs +
			VCODEC_COREX_VIDEO_NOC_ERR_ERRLOG2_LOW_OFFS);
	dprintk(VIDC_ERR, "CORE%d_NOC_ERR_ERRLOG2_LOW:  %#x\n", core_num, val);
	val = __read_register(device, vcodec_core_video_noc_base_offs +
			VCODEC_COREX_VIDEO_NOC_ERR_ERRLOG2_HIGH_OFFS);
	dprintk(VIDC_ERR, "CORE%d_NOC_ERR_ERRLOG2_HIGH: %#x\n", core_num, val);
	val = __read_register(device, vcodec_core_video_noc_base_offs +
			VCODEC_COREX_VIDEO_NOC_ERR_ERRLOG3_LOW_OFFS);
	dprintk(VIDC_ERR, "CORE%d_NOC_ERR_ERRLOG3_LOW:  %#x\n", core_num, val);
	val = __read_register(device, vcodec_core_video_noc_base_offs +
			VCODEC_COREX_VIDEO_NOC_ERR_ERRLOG3_HIGH_OFFS);
	dprintk(VIDC_ERR, "CORE%d_NOC_ERR_ERRLOG3_HIGH: %#x\n", core_num, val);
}

static int venus_hfi_noc_error_info(void *dev)
{
	struct venus_hfi_device *device;
	const u32 core0 = 0, core1 = 1;

	if (!dev) {
		dprintk(VIDC_ERR, "%s: null device\n", __func__);
		return -EINVAL;
	}
	device = dev;

	mutex_lock(&device->lock);
	dprintk(VIDC_ERR, "%s: non error information\n", __func__);

	if (__read_register(device, VCODEC_CORE0_VIDEO_NOC_BASE_OFFS +
			VCODEC_COREX_VIDEO_NOC_ERR_ERRVLD_LOW_OFFS))
		__noc_error_info(device, core0);

	if (__read_register(device, VCODEC_CORE1_VIDEO_NOC_BASE_OFFS +
			VCODEC_COREX_VIDEO_NOC_ERR_ERRVLD_LOW_OFFS))
		__noc_error_info(device, core1);

	mutex_unlock(&device->lock);

	return 0;
}

static int __initialize_packetization(struct venus_hfi_device *device)
{
	int rc = 0;

	if (!device || !device->res) {
		dprintk(VIDC_ERR, "%s - invalid param\n", __func__);
		return -EINVAL;
	}

	device->packetization_type = HFI_PACKETIZATION_4XX;

	device->pkt_ops = hfi_get_pkt_ops_handle(device->packetization_type);
	if (!device->pkt_ops) {
		rc = -EINVAL;
		dprintk(VIDC_ERR, "Failed to get pkt_ops handle\n");
	}

	return rc;
}

static struct venus_hfi_device *__add_device(u32 device_id,
			struct msm_vidc_platform_resources *res,
			hfi_cmd_response_callback callback)
{
	struct venus_hfi_device *hdevice = NULL;
	int rc = 0;

	if (!res || !callback) {
		dprintk(VIDC_ERR, "Invalid Parameters\n");
		return NULL;
	}

	dprintk(VIDC_INFO, "entered , device_id: %d\n", device_id);

	hdevice = (struct venus_hfi_device *)
			kzalloc(sizeof(struct venus_hfi_device), GFP_KERNEL);
	if (!hdevice) {
		dprintk(VIDC_ERR, "failed to allocate new device\n");
		goto exit;
	}

	hdevice->response_pkt = kmalloc_array(max_packets,
				sizeof(*hdevice->response_pkt), GFP_KERNEL);
	if (!hdevice->response_pkt) {
		dprintk(VIDC_ERR, "failed to allocate response_pkt\n");
		goto err_cleanup;
	}

	hdevice->raw_packet =
		kzalloc(VIDC_IFACEQ_VAR_HUGE_PKT_SIZE, GFP_TEMPORARY);
	if (!hdevice->raw_packet) {
		dprintk(VIDC_ERR, "failed to allocate raw packet\n");
		goto err_cleanup;
	}

	rc = __init_regs_and_interrupts(hdevice, res);
	if (rc)
		goto err_cleanup;

	hdevice->res = res;
	hdevice->device_id = device_id;
	hdevice->callback = callback;

	hdevice->vidc_workq = create_singlethread_workqueue(
		"msm_vidc_workerq_venus");
	if (!hdevice->vidc_workq) {
		dprintk(VIDC_ERR, ": create vidc workq failed\n");
		goto err_cleanup;
	}

	hdevice->venus_pm_workq = create_singlethread_workqueue(
			"pm_workerq_venus");
	if (!hdevice->venus_pm_workq) {
		dprintk(VIDC_ERR, ": create pm workq failed\n");
		goto err_cleanup;
	}

	if (!hal_ctxt.dev_count)
		INIT_LIST_HEAD(&hal_ctxt.dev_head);

	mutex_init(&hdevice->lock);
	INIT_LIST_HEAD(&hdevice->list);
	INIT_LIST_HEAD(&hdevice->sess_head);
	list_add_tail(&hdevice->list, &hal_ctxt.dev_head);
	hal_ctxt.dev_count++;

	return hdevice;

err_cleanup:
	if (hdevice->vidc_workq)
		destroy_workqueue(hdevice->vidc_workq);
	kfree(hdevice->response_pkt);
	kfree(hdevice->raw_packet);
	kfree(hdevice);
exit:
	return NULL;
}

static struct venus_hfi_device *__get_device(u32 device_id,
				struct msm_vidc_platform_resources *res,
				hfi_cmd_response_callback callback)
{
	if (!res || !callback) {
		dprintk(VIDC_ERR, "Invalid params: %pK %pK\n", res, callback);
		return NULL;
	}

	return __add_device(device_id, res, callback);
}

void venus_hfi_delete_device(void *device)
{
	struct venus_hfi_device *close, *tmp, *dev;

	if (!device)
		return;

	dev = (struct venus_hfi_device *) device;

	mutex_lock(&dev->lock);
	__iommu_detach(dev);
	mutex_unlock(&dev->lock);

	list_for_each_entry_safe(close, tmp, &hal_ctxt.dev_head, list) {
		if (close->hal_data->irq == dev->hal_data->irq) {
			hal_ctxt.dev_count--;
			list_del(&close->list);
			mutex_destroy(&close->lock);
			destroy_workqueue(close->vidc_workq);
			destroy_workqueue(close->venus_pm_workq);
			free_irq(dev->hal_data->irq, close);
			iounmap(dev->hal_data->register_base);
			kfree(close->hal_data);
			kfree(close->response_pkt);
			kfree(close->raw_packet);
			kfree(close);
			break;
		}
	}
}

static void venus_init_hfi_callbacks(struct hfi_device *hdev)
{
	hdev->core_init = venus_hfi_core_init;
	hdev->core_release = venus_hfi_core_release;
	hdev->core_ping = venus_hfi_core_ping;
	hdev->core_trigger_ssr = venus_hfi_core_trigger_ssr;
	hdev->session_init = venus_hfi_session_init;
	hdev->session_end = venus_hfi_session_end;
	hdev->session_abort = venus_hfi_session_abort;
	hdev->session_clean = venus_hfi_session_clean;
	hdev->session_set_buffers = venus_hfi_session_set_buffers;
	hdev->session_release_buffers = venus_hfi_session_release_buffers;
	hdev->session_load_res = venus_hfi_session_load_res;
	hdev->session_release_res = venus_hfi_session_release_res;
	hdev->session_start = venus_hfi_session_start;
	hdev->session_continue = venus_hfi_session_continue;
	hdev->session_stop = venus_hfi_session_stop;
	hdev->session_etb = venus_hfi_session_etb;
	hdev->session_ftb = venus_hfi_session_ftb;
	hdev->session_process_batch = venus_hfi_session_process_batch;
	hdev->session_get_buf_req = venus_hfi_session_get_buf_req;
	hdev->session_flush = venus_hfi_session_flush;
	hdev->session_set_property = venus_hfi_session_set_property;
	hdev->session_get_property = venus_hfi_session_get_property;
	hdev->scale_clocks = venus_hfi_scale_clocks;
	hdev->vote_bus = venus_hfi_vote_buses;
	hdev->get_fw_info = venus_hfi_get_fw_info;
	hdev->get_core_capabilities = venus_hfi_get_core_capabilities;
	hdev->suspend = venus_hfi_suspend;
	hdev->flush_debug_queue = venus_hfi_flush_debug_queue;
	hdev->noc_error_info = venus_hfi_noc_error_info;
	hdev->get_default_properties = venus_hfi_get_default_properties;
}

int venus_hfi_initialize(struct hfi_device *hdev, u32 device_id,
		struct msm_vidc_platform_resources *res,
		hfi_cmd_response_callback callback)
{
	int rc = 0;

	if (!hdev || !res || !callback) {
		dprintk(VIDC_ERR, "Invalid params: %pK %pK %pK\n",
			hdev, res, callback);
		rc = -EINVAL;
		goto err_venus_hfi_init;
	}

	hdev->hfi_device_data = __get_device(device_id, res, callback);

	if (IS_ERR_OR_NULL(hdev->hfi_device_data)) {
		rc = PTR_ERR(hdev->hfi_device_data) ?: -EINVAL;
		goto err_venus_hfi_init;
	}

	venus_init_hfi_callbacks(hdev);

err_venus_hfi_init:
	return rc;
}

