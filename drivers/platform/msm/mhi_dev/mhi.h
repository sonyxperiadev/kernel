/* Copyright (c) 2015-2016, The Linux Foundation. All rights reserved.
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

#ifndef __MHI_H
#define __MHI_H

#include <linux/msm_ep_pcie.h>
#include <linux/types.h>
#include <linux/ipc_logging.h>
#include <linux/dma-mapping.h>

/**
 * MHI control data structures alloted by the host, including
 * channel context array, event context array, command context and rings.
 */

/* Channel context state */
enum mhi_dev_ch_ctx_state {
	MHI_DEV_CH_STATE_DISABLED,
	MHI_DEV_CH_STATE_ENABLED,
	MHI_DEV_CH_STATE_RUNNING,
	MHI_DEV_CH_STATE_SUSPENDED,
	MHI_DEV_CH_STATE_STOP,
	MHI_DEV_CH_STATE_ERROR,
	MHI_DEV_CH_STATE_RESERVED,
	MHI_DEV_CH_STATE_32BIT = 0x7FFFFFFF
};

/* Channel type */
enum mhi_dev_ch_ctx_type {
	MHI_DEV_CH_TYPE_NONE,
	MHI_DEV_CH_TYPE_OUTBOUND_CHANNEL,
	MHI_DEV_CH_TYPE_INBOUND_CHANNEL,
	MHI_DEV_CH_RESERVED
};

/* Channel context type */
struct mhi_dev_ch_ctx {
	enum mhi_dev_ch_ctx_state	ch_state;
	enum mhi_dev_ch_ctx_type	ch_type;
	uint32_t			err_indx;
	uint64_t			rbase;
	uint64_t			rlen;
	uint64_t			rp;
	uint64_t			wp;
} __packed;

enum mhi_dev_ring_element_type_id {
	MHI_DEV_RING_EL_INVALID = 0,
	MHI_DEV_RING_EL_NOOP = 1,
	MHI_DEV_RING_EL_TRANSFER = 2,
	MHI_DEV_RING_EL_RESET = 16,
	MHI_DEV_RING_EL_STOP = 17,
	MHI_DEV_RING_EL_START = 18,
	MHI_DEV_RING_EL_MHI_STATE_CHG = 32,
	MHI_DEV_RING_EL_CMD_COMPLETION_EVT = 33,
	MHI_DEV_RING_EL_TRANSFER_COMPLETION_EVENT = 34,
	MHI_DEV_RING_EL_EE_STATE_CHANGE_NOTIFY = 64,
	MHI_DEV_RING_EL_UNDEF
};

enum mhi_dev_ring_state {
	RING_STATE_UINT = 0,
	RING_STATE_IDLE,
	RING_STATE_PENDING,
};

enum mhi_dev_ring_type {
	RING_TYPE_CMD = 0,
	RING_TYPE_ER,
	RING_TYPE_CH,
	RING_TYPE_INVAL
};

/* Event context interrupt moderation */
enum mhi_dev_evt_ctx_int_mod_timer {
	MHI_DEV_EVT_INT_MODERATION_DISABLED
};

/* Event ring type */
enum mhi_dev_evt_ctx_event_ring_type {
	MHI_DEV_EVT_TYPE_DEFAULT,
	MHI_DEV_EVT_TYPE_VALID,
	MHI_DEV_EVT_RESERVED
};

/* Event ring context type */
struct mhi_dev_ev_ctx {
	uint32_t				res1:16;
	enum mhi_dev_evt_ctx_int_mod_timer	intmodt:16;
	enum mhi_dev_evt_ctx_event_ring_type	ertype;
	uint32_t				msivec;
	uint64_t				rbase;
	uint64_t				rlen;
	uint64_t				rp;
	uint64_t				wp;
} __packed;

/* Command context */
struct mhi_dev_cmd_ctx {
	uint32_t				res1;
	uint32_t				res2;
	uint32_t				res3;
	uint64_t				rbase;
	uint64_t				rlen;
	uint64_t				rp;
	uint64_t				wp;
} __packed;

/* generic context */
struct mhi_dev_gen_ctx {
	uint32_t				res1;
	uint32_t				res2;
	uint32_t				res3;
	uint64_t				rbase;
	uint64_t				rlen;
	uint64_t				rp;
	uint64_t				wp;
} __packed;

/* Transfer ring element */
struct mhi_dev_transfer_ring_element {
	uint64_t				data_buf_ptr;
	uint32_t				len:16;
	uint32_t				res1:16;
	uint32_t				chain:1;
	uint32_t				res2:7;
	uint32_t				ieob:1;
	uint32_t				ieot:1;
	uint32_t				bei:1;
	uint32_t				res3:5;
	enum mhi_dev_ring_element_type_id	type:8;
	uint32_t				res4:8;
} __packed;

/* Command ring element */
/* Command ring No op command */
struct mhi_dev_cmd_ring_op {
	uint64_t				res1;
	uint32_t				res2;
	uint32_t				res3:16;
	enum mhi_dev_ring_element_type_id	type:8;
	uint32_t				chid:8;
} __packed;

/* Command ring reset channel command */
struct mhi_dev_cmd_ring_reset_channel_cmd {
	uint64_t				res1;
	uint32_t				res2;
	uint32_t				res3:16;
	enum mhi_dev_ring_element_type_id	type:8;
	uint32_t				chid:8;
} __packed;

/* Command ring stop channel command */
struct mhi_dev_cmd_ring_stop_channel_cmd {
	uint64_t				res1;
	uint32_t				res2;
	uint32_t				res3:16;
	enum mhi_dev_ring_element_type_id	type:8;
	uint32_t				chid:8;
} __packed;

/* Command ring start channel command */
struct mhi_dev_cmd_ring_start_channel_cmd {
	uint64_t				res1;
	uint32_t				seqnum;
	uint32_t				reliable:1;
	uint32_t				res2:15;
	enum mhi_dev_ring_element_type_id	type:8;
	uint32_t				chid:8;
} __packed;

enum mhi_dev_cmd_completion_code {
	MHI_CMD_COMPL_CODE_INVALID = 0,
	MHI_CMD_COMPL_CODE_SUCCESS = 1,
	MHI_CMD_COMPL_CODE_EOT = 2,
	MHI_CMD_COMPL_CODE_OVERFLOW = 3,
	MHI_CMD_COMPL_CODE_EOB = 4,
	MHI_CMD_COMPL_CODE_UNDEFINED = 16,
	MHI_CMD_COMPL_CODE_RING_EL = 17,
	MHI_CMD_COMPL_CODE_RES
};

/* Event ring elements */
/* Transfer completion event */
struct mhi_dev_event_ring_transfer_completion {
	uint64_t				ptr;
	uint32_t				len:16;
	uint32_t				res1:8;
	enum mhi_dev_cmd_completion_code	code:8;
	uint32_t				res2:16;
	enum mhi_dev_ring_element_type_id	type:8;
	uint32_t				chid:8;
} __packed;

/* Command completion event */
struct mhi_dev_event_ring_cmd_completion {
	uint64_t				ptr;
	uint32_t				res1:24;
	enum mhi_dev_cmd_completion_code	code:8;
	uint32_t				res2:16;
	enum mhi_dev_ring_element_type_id	type:8;
	uint32_t				res3:8;
} __packed;

enum mhi_dev_state {
	MHI_DEV_RESET_STATE = 0,
	MHI_DEV_READY_STATE,
	MHI_DEV_M0_STATE,
	MHI_DEV_M1_STATE,
	MHI_DEV_M2_STATE,
	MHI_DEV_M3_STATE,
	MHI_DEV_MAX_STATE,
	MHI_DEV_SYSERR_STATE = 0xff
};

/* MHI state change event */
struct mhi_dev_event_ring_state_change {
	uint64_t				ptr;
	uint32_t				res1:24;
	enum mhi_dev_state			mhistate:8;
	uint32_t				res2:16;
	enum mhi_dev_ring_element_type_id	type:8;
	uint32_t				res3:8;
} __packed;

enum mhi_dev_execenv {
	MHI_DEV_SBL_EE = 1,
	MHI_DEV_AMSS_EE = 2,
	MHI_DEV_UNRESERVED
};

/* EE state change event */
struct mhi_dev_event_ring_ee_state_change {
	uint64_t				ptr;
	uint32_t				res1:24;
	enum mhi_dev_execenv			execenv:8;
	uint32_t				res2:16;
	enum mhi_dev_ring_element_type_id	type:8;
	uint32_t				res3:8;
} __packed;

/* Generic cmd to parse common details like type and channel id */
struct mhi_dev_ring_generic {
	uint64_t				ptr;
	uint32_t				res1:24;
	enum mhi_dev_state			mhistate:8;
	uint32_t				res2:16;
	enum mhi_dev_ring_element_type_id	type:8;
	uint32_t				chid:8;
} __packed;

struct mhi_config {
	uint32_t	mhi_reg_len;
	uint32_t	version;
	uint32_t	event_rings;
	uint32_t	channels;
	uint32_t	chdb_offset;
	uint32_t	erdb_offset;
};

#define NUM_CHANNELS			128
#define HW_CHANNEL_BASE			100
#define HW_CHANNEL_END			107
#define MHI_ENV_VALUE			2
#define MHI_MASK_ROWS_CH_EV_DB		4
#define TRB_MAX_DATA_SIZE		4096

/* Possible ring element types */
union mhi_dev_ring_element_type {
	struct mhi_dev_cmd_ring_op			cmd_no_op;
	struct mhi_dev_cmd_ring_reset_channel_cmd	cmd_reset;
	struct mhi_dev_cmd_ring_stop_channel_cmd	cmd_stop;
	struct mhi_dev_cmd_ring_start_channel_cmd	cmd_start;
	struct mhi_dev_transfer_ring_element		tre;
	struct mhi_dev_event_ring_transfer_completion	evt_tr_comp;
	struct mhi_dev_event_ring_cmd_completion	evt_cmd_comp;
	struct mhi_dev_event_ring_state_change		evt_state_change;
	struct mhi_dev_event_ring_ee_state_change	evt_ee_state;
	struct mhi_dev_ring_generic			generic;
};

/* Transfer ring element type */
union mhi_dev_ring_ctx {
	struct mhi_dev_cmd_ctx		cmd;
	struct mhi_dev_ev_ctx		ev;
	struct mhi_dev_ch_ctx		ch;
	struct mhi_dev_gen_ctx		generic;
};

/* MHI host Control and data address region */
struct mhi_host_addr {
	uint32_t	ctrl_base_lsb;
	uint32_t	ctrl_base_msb;
	uint32_t	ctrl_limit_lsb;
	uint32_t	ctrl_limit_msb;
	uint32_t	data_base_lsb;
	uint32_t	data_base_msb;
	uint32_t	data_limit_lsb;
	uint32_t	data_limit_msb;
};

/* MHI physical and virtual address region */
struct mhi_meminfo {
	struct device	*dev;
	uintptr_t	pa_aligned;
	uintptr_t	pa_unaligned;
	uintptr_t	va_aligned;
	uintptr_t	va_unaligned;
	uintptr_t	size;
};

struct mhi_addr {
	uint64_t	host_pa;
	uintptr_t	device_pa;
	uintptr_t	device_va;
	uint32_t	size;
};

struct mhi_interrupt_state {
	uint32_t	mask;
	uint32_t	status;
};

enum mhi_dev_channel_state {
	MHI_DEV_CH_UNINT,
	MHI_DEV_CH_STARTED,
	MHI_DEV_CH_PENDING_START,
	MHI_DEV_CH_PENDING_STOP,
	MHI_DEV_CH_STOPPED,
	MHI_DEV_CH_CLOSED,
};

enum mhi_dev_ch_operation {
	MHI_DEV_OPEN_CH,
	MHI_DEV_CLOSE_CH,
	MHI_DEV_READ_CH,
	MHI_DEV_READ_WR,
	MHI_DEV_POLL,
};

struct mhi_dev_channel;

struct mhi_dev_ring {
	struct list_head			list;
	struct mhi_dev				*mhi_dev;

	uint32_t				id;
	uint32_t				rd_offset;
	uint32_t				wr_offset;
	uint32_t				ring_size;

	enum mhi_dev_ring_type			type;
	enum mhi_dev_ring_state			state;

	/* device virtual address location of the cached host ring ctx data */
	union mhi_dev_ring_element_type		*ring_cache;
	/* Physical address of the cached ring copy on the device side */
	dma_addr_t				ring_cache_dma_handle;
	/* Physical address of the host where we will write/read to/from */
	struct mhi_addr				ring_shadow;
	/* Ring type - cmd, event, transfer ring and its rp/wp... */
	union mhi_dev_ring_ctx			*ring_ctx;
	/* ring_ctx_shadow -> tracking ring_ctx in the host */
	union mhi_dev_ring_ctx			*ring_ctx_shadow;
	void (*ring_cb)(struct mhi_dev *dev,
			union mhi_dev_ring_element_type *el,
			void *ctx);
};

static inline void mhi_dev_ring_inc_index(struct mhi_dev_ring *ring,
						uint32_t rd_offset)
{
	ring->rd_offset++;
	if (ring->rd_offset == ring->ring_size)
		ring->rd_offset = 0;
}

/* trace information planned to use for read/write */
#define TRACE_DATA_MAX				128
#define MHI_DEV_DATA_MAX			512

#define MHI_DEV_MMIO_RANGE			0xc80

enum cb_reason {
	MHI_DEV_TRE_AVAILABLE = 0,
};

struct mhi_dev_client_cb_reason {
	uint32_t		ch_id;
	enum cb_reason		reason;
};

struct mhi_dev_client {
	struct list_head		list;
	struct mhi_dev_channel		*channel;
	void (*event_trigger)(struct mhi_dev_client_cb_reason *cb);

	/* mhi_dev calls are fully synchronous -- only one call may be
	 * active per client at a time for now.
	 */
	struct mutex			write_lock;
	wait_queue_head_t		wait;

	/* trace logs */
	spinlock_t			tr_lock;
	unsigned			tr_head;
	unsigned			tr_tail;
	struct mhi_dev_trace		*tr_log;

	/* client buffers */
	struct mhi_dev_iov		*iov;
	uint32_t			nr_iov;
};

struct mhi_dev_channel {
	struct list_head		list;
	struct list_head		clients;
	/* synchronization for changing channel state,
	 * adding/removing clients, mhi_dev callbacks, etc
	 */
	spinlock_t			lock;

	struct mhi_dev_ring		*ring;

	enum mhi_dev_channel_state	state;
	uint32_t			ch_id;
	enum mhi_dev_ch_ctx_type	ch_type;
	struct mutex			ch_lock;
	/* client which the current inbound/outbound message is for */
	struct mhi_dev_client		*active_client;
	/*
	 * Pointer to event request structs used to temporarily store
	 * completion events and meta data before sending them to host
	 */
	struct event_req		*ereqs;
	/* Pointer to completion event buffers */
	union mhi_dev_ring_element_type *tr_events;
	struct list_head		event_req_buffers;
	struct event_req		*curr_ereq;

	/* current TRE being processed */
	uint64_t			tre_loc;
	/* current TRE size */
	uint32_t			tre_size;
	/* tre bytes left to read/write */
	uint32_t			tre_bytes_left;
	/* td size being read/written from/to so far */
	uint32_t			td_size;
	bool				wr_request_active;
	bool				skip_td;
};

/* Structure device for mhi dev */
struct mhi_dev {
	struct platform_device		*pdev;
	struct device			*dev;
	/* MHI MMIO related members */
	phys_addr_t			mmio_base_pa_addr;
	void				*mmio_base_addr;
	phys_addr_t			ipa_uc_mbox_crdb;
	phys_addr_t			ipa_uc_mbox_erdb;

	uint32_t			*mmio_backup;
	struct mhi_config		cfg;
	bool				mmio_initialized;

	/* Host control base information */
	struct mhi_host_addr		host_addr;
	struct mhi_addr			ctrl_base;
	struct mhi_addr			data_base;
	struct mhi_addr			ch_ctx_shadow;
	struct mhi_dev_ch_ctx		*ch_ctx_cache;
	dma_addr_t			ch_ctx_cache_dma_handle;
	struct mhi_addr			ev_ctx_shadow;
	struct mhi_dev_ch_ctx		*ev_ctx_cache;
	dma_addr_t			ev_ctx_cache_dma_handle;

	struct mhi_addr			cmd_ctx_shadow;
	struct mhi_dev_ch_ctx		*cmd_ctx_cache;
	dma_addr_t			cmd_ctx_cache_dma_handle;
	struct mhi_dev_ring		*ring;
	struct mhi_dev_channel		*ch;

	int				ctrl_int;
	int				cmd_int;
	/* CHDB and EVDB device interrupt state */
	struct mhi_interrupt_state	chdb[4];
	struct mhi_interrupt_state	evdb[4];

	/* Scheduler work */
	struct work_struct		chdb_ctrl_work;
	struct mutex			mhi_lock;
	struct mutex			mhi_event_lock;

	/* process a ring element */
	struct workqueue_struct		*pending_ring_wq;
	struct work_struct		pending_work;

	struct list_head		event_ring_list;
	struct list_head		process_ring_list;

	uint32_t			cmd_ring_idx;
	uint32_t			ev_ring_start;
	uint32_t			ch_ring_start;

	/* IPA Handles */
	u32				ipa_clnt_hndl[4];
	struct workqueue_struct		*ring_init_wq;
	struct work_struct		ring_init_cb_work;

	/* EP PCIe registration */
	struct ep_pcie_register_event	event_reg;
	u32                             ifc_id;
	struct ep_pcie_hw               *phandle;

	atomic_t			write_active;
	atomic_t			is_suspended;
	struct mutex			mhi_write_test;
	u32				mhi_ep_msi_num;
	u32				mhi_version;
	void				*dma_cache;
	void				*read_handle;
	void				*write_handle;
	/* Physical scratch buffer for writing control data to the host */
	dma_addr_t			cache_dma_handle;
	/*
	 * Physical scratch buffer address used when picking host data
	 * from the host used in mhi_read()
	 */
	dma_addr_t			read_dma_handle;
	/*
	 * Physical scratch buffer address used when writing to the host
	 * region from device used in mhi_write()
	 */
	dma_addr_t			write_dma_handle;
};

enum mhi_msg_level {
	MHI_MSG_VERBOSE = 0x0,
	MHI_MSG_INFO = 0x1,
	MHI_MSG_DBG = 0x2,
	MHI_MSG_WARNING = 0x3,
	MHI_MSG_ERROR = 0x4,
	MHI_MSG_CRITICAL = 0x5,
	MHI_MSG_reserved = 0x80000000
};

extern enum mhi_msg_level mhi_msg_lvl;
extern enum mhi_msg_level mhi_ipc_msg_lvl;
extern void *mhi_ipc_log;

#define mhi_log(_msg_lvl, _msg, ...) do { \
	if (_msg_lvl >= mhi_msg_lvl) { \
		pr_err("[%s] "_msg, __func__, ##__VA_ARGS__); \
	} \
	if (mhi_ipc_log && (_msg_lvl >= mhi_ipc_msg_lvl)) { \
		ipc_log_string(mhi_ipc_log,                     \
			"[%s] " _msg, __func__, ##__VA_ARGS__);     \
	} \
} while (0)

/* SW channel client list */
enum mhi_client_channel {
	MHI_CLIENT_LOOPBACK_OUT = 0,
	MHI_CLIENT_LOOPBACK_IN = 1,
	MHI_CLIENT_SAHARA_OUT = 2,
	MHI_CLIENT_SAHARA_IN = 3,
	MHI_CLIENT_DIAG_OUT = 4,
	MHI_CLIENT_DIAG_IN = 5,
	MHI_CLIENT_SSR_OUT = 6,
	MHI_CLIENT_SSR_IN = 7,
	MHI_CLIENT_QDSS_OUT = 8,
	MHI_CLIENT_QDSS_IN = 9,
	MHI_CLIENT_EFS_OUT = 10,
	MHI_CLIENT_EFS_IN = 11,
	MHI_CLIENT_MBIM_OUT = 12,
	MHI_CLIENT_MBIM_IN = 13,
	MHI_CLIENT_QMI_OUT = 14,
	MHI_CLIENT_QMI_IN = 15,
	MHI_CLIENT_IP_CTRL_0_OUT = 16,
	MHI_CLIENT_IP_CTRL_0_IN = 17,
	MHI_CLIENT_IP_CTRL_1_OUT = 18,
	MHI_CLIENT_IP_CTRL_1_IN = 19,
	MHI_CLIENT_DCI_OUT = 20,
	MHI_CLIENT_DCI_IN = 21,
	MHI_CLIENT_IP_CTRL_3_OUT = 22,
	MHI_CLIENT_IP_CTRL_3_IN = 23,
	MHI_CLIENT_IP_CTRL_4_OUT = 24,
	MHI_CLIENT_IP_CTRL_4_IN = 25,
	MHI_CLIENT_IP_CTRL_5_OUT = 26,
	MHI_CLIENT_IP_CTRL_5_IN = 27,
	MHI_CLIENT_IP_CTRL_6_OUT = 28,
	MHI_CLIENT_IP_CTRL_6_IN = 29,
	MHI_CLIENT_IP_CTRL_7_OUT = 30,
	MHI_CLIENT_IP_CTRL_7_IN = 31,
	MHI_CLIENT_DUN_OUT = 32,
	MHI_CLIENT_DUN_IN = 33,
	MHI_CLIENT_IP_SW_0_OUT = 34,
	MHI_CLIENT_IP_SW_0_IN = 35,
	MHI_CLIENT_IP_SW_1_OUT = 36,
	MHI_CLIENT_IP_SW_1_IN = 37,
	MHI_CLIENT_IP_SW_2_OUT = 38,
	MHI_CLIENT_IP_SW_2_IN = 39,
	MHI_CLIENT_IP_SW_3_OUT = 40,
	MHI_CLIENT_IP_SW_3_IN = 41,
	MHI_CLIENT_CSVT_OUT = 42,
	MHI_CLIENT_CSVT_IN = 43,
	MHI_CLIENT_SMCT_OUT = 44,
	MHI_CLIENT_SMCT_IN = 45,
	MHI_CLIENT_IP_SW_4_OUT  = 46,
	MHI_CLIENT_IP_SW_4_IN  = 47,
	MHI_CLIENT_ADB_OUT = 48,
	MHI_CLIENT_ADB_IN = 49,
	MHI_MAX_SOFTWARE_CHANNELS,
	MHI_CLIENT_TEST_OUT = 60,
	MHI_CLIENT_TEST_IN = 61,
	MHI_CLIENT_RESERVED_1_LOWER = 62,
	MHI_CLIENT_RESERVED_1_UPPER = 99,
	MHI_CLIENT_IP_HW_0_OUT = 100,
	MHI_CLIENT_IP_HW_0_IN = 101,
	MHI_CLIENT_RESERVED_2_LOWER = 102,
	MHI_CLIENT_RESERVED_2_UPPER = 127,
	MHI_MAX_CHANNELS = 102,
};

struct mhi_dev_iov {
	void		*addr;
	uint32_t	buf_size;
};

/**
 * mhi_dev_open_channel() - Channel open for a given client done prior
 *		to read/write.
 * @chan_id:	Software Channel ID for the assigned client.
 * @handle_client: Structure device for client handle.
 * @notifier: Client issued callback notification.
 */
int mhi_dev_open_channel(uint32_t chan_id,
		struct mhi_dev_client **handle_client,
		void (*event_trigger)(struct mhi_dev_client_cb_reason *cb));
/**
 * mhi_dev_close_channel() - Channel close for a given client.
 */
int mhi_dev_close_channel(struct mhi_dev_client *handle_client);

/**
 * mhi_dev_read_channel() - Channel read for a given client
 * @handle_client:	Client Handle issued during mhi_dev_open_channel
 * @buf: Pointer to the buffer used by the MHI core to copy the data received
 *	 from the Host.
 * @buf_size: Size of the buffer pointer.
 * @chain : Indicate if the received data is part of chained packet.
 */
int mhi_dev_read_channel(struct mhi_dev_client *handle_client,
				void *buf, uint32_t buf_size, uint32_t *chain);

/**
 * mhi_dev_write_channel() - Channel write for a given software client.
 * @handle_client:	Client Handle issued during mhi_dev_open_channel
 * @buf: Pointer to the buffer used by the MHI core to copy the data from the
 *	 device to the host.
 * @buf_size: Size of the buffer pointer.
 */
int mhi_dev_write_channel(struct mhi_dev_client *handle_client, void *buf,
							uint32_t buf_size);

/**
 * mhi_dev_channel_isempty() - Checks if there is any pending TRE's to process.
 * @handle_client:	Client Handle issued during mhi_dev_open_channel
 */
int mhi_dev_channel_isempty(struct mhi_dev_client *handle);

struct mhi_dev_trace {
	unsigned timestamp;
	uint32_t data[TRACE_DATA_MAX];
};

/* MHI Ring related functions */

/**
 * mhi_ring_init() - Initializes the Ring id to the default un-initialized
 *		state. Once a start command is received, the respective ring
 *		is then prepared by fetching the context and updating the
 *		offset.
 * @ring:	Ring for the respective context - Channel/Event/Command.
 * @type:	Command/Event or Channel transfer ring.
 * @id:		Index to the ring id. For command its usually 1, Event rings
 *		may vary from 1 to 128. Channels vary from 1 to 256.
 */
void mhi_ring_init(struct mhi_dev_ring *ring,
			enum mhi_dev_ring_type type, int id);

/**
 * mhi_ring_start() - Fetches the respective transfer ring's context from
 *		the host and updates the write offset.
 * @ring:	Ring for the respective context - Channel/Event/Command.
 * @ctx:	Transfer ring of type mhi_dev_ring_ctx.
 * @dev:	MHI device structure.
 */
int mhi_ring_start(struct mhi_dev_ring *ring,
			union mhi_dev_ring_ctx *ctx, struct mhi_dev *mhi);

/**
 * mhi_dev_cache_ring() - Cache the data for the corresponding ring locally.
 * @ring:	Ring for the respective context - Channel/Event/Command.
 * @wr_offset:	Cache the TRE's upto the write offset value.
 */
int mhi_dev_cache_ring(struct mhi_dev_ring *ring, uint32_t wr_offset);

/**
 * mhi_dev_update_wr_offset() - Check for any updates in the write offset.
 * @ring:	Ring for the respective context - Channel/Event/Command.
 */
int mhi_dev_update_wr_offset(struct mhi_dev_ring *ring);

/**
 * mhi_dev_process_ring() - Update the Write pointer, fetch the ring elements
 *			    and invoke the clients callback.
 * @ring:	Ring for the respective context - Channel/Event/Command.
 */
int mhi_dev_process_ring(struct mhi_dev_ring *ring);

/**
 * mhi_dev_process_ring_element() - Fetch the ring elements and invoke the
 *			    clients callback.
 * @ring:	Ring for the respective context - Channel/Event/Command.
 * @offset:	Offset index into the respective ring's cache element.
 */
int mhi_dev_process_ring_element(struct mhi_dev_ring *ring, uint32_t offset);

/**
 * mhi_dev_add_element() - Copy the element to the respective transfer rings
 *			read pointer and increment the index.
 * @ring:	Ring for the respective context - Channel/Event/Command.
 * @element:	Transfer ring element to be copied to the host memory.
 */
int mhi_dev_add_element(struct mhi_dev_ring *ring,
				union mhi_dev_ring_element_type *element);

/**
 * mhi_transfer_device_to_host() - memcpy equivalent API to transfer data
 *		from device to the host.
 * @dst_pa:	Physical destination address.
 * @src:	Source virtual address.
 * @len:	Numer of bytes to be transferred.
 * @mhi:	MHI dev structure.
 */
int mhi_transfer_device_to_host(uint64_t dst_pa, void *src, uint32_t len,
				struct mhi_dev *mhi);

/**
 * mhi_transfer_host_to_dev() - memcpy equivalent API to transfer data
 *		from host to the device.
 * @dst:	Physical destination virtual address.
 * @src_pa:	Source physical address.
 * @len:	Numer of bytes to be transferred.
 * @mhi:	MHI dev structure.
 */
int mhi_transfer_host_to_device(void *device, uint64_t src_pa, uint32_t len,
				struct mhi_dev *mhi);

/**
 * mhi_dev_write_to_host() - memcpy equivalent API to transfer data
 *		from device to host.
 * @host:	Host and device address details.
 * @buf:	Data buffer that needs to be written to the host.
 * @size:	Data buffer size.
 */
void mhi_dev_write_to_host(struct mhi_addr *host, void *buf, size_t size,
				struct mhi_dev *mhi);

/**
 * mhi_dev_read_from_host() - memcpy equivalent API to transfer data
 *		from host to device.
 * @host:	Host and device address details.
 * @buf:	Data buffer that needs to be read from the host.
 * @size:	Data buffer size.
 */
void mhi_dev_read_from_host(struct mhi_addr *dst, dma_addr_t buf, size_t size);

/**
 * mhi_dev_read_from_host() - memcpy equivalent API to transfer data
 *		from host to device.
 * @host:	Host and device address details.
 * @buf:	Data buffer that needs to be read from the host.
 * @size:	Data buffer size.
 */
void mhi_ring_set_cb(struct mhi_dev_ring *ring,
			void (*ring_cb)(struct mhi_dev *dev,
			union mhi_dev_ring_element_type *el, void *ctx));

/**
 * mhi_ring_set_state() - Sets internal state of the ring for tracking whether
 *		a ring is being processed, idle or uninitialized.
 * @ring:	Ring for the respective context - Channel/Event/Command.
 * @state:	state of type mhi_dev_ring_state.
 */
void mhi_ring_set_state(struct mhi_dev_ring *ring,
			enum mhi_dev_ring_state state);

/**
 * mhi_ring_get_state() - Obtains the internal state of the ring.
 * @ring:	Ring for the respective context - Channel/Event/Command.
 */
enum mhi_dev_ring_state mhi_ring_get_state(struct mhi_dev_ring *ring);

/* MMIO related functions */

/**
 * mhi_dev_mmio_read() - Generic MHI MMIO register read API.
 * @dev:	MHI device structure.
 * @offset:	MHI address offset from base.
 * @reg_val:	Pointer the register value is stored to.
 */
int mhi_dev_mmio_read(struct mhi_dev *dev, uint32_t offset,
			uint32_t *reg_value);

/**
 * mhi_dev_mmio_read() - Generic MHI MMIO register write API.
 * @dev:	MHI device structure.
 * @offset:	MHI address offset from base.
 * @val:	Value to be written to the register offset.
 */
int mhi_dev_mmio_write(struct mhi_dev *dev, uint32_t offset,
				uint32_t val);

/**
 * mhi_dev_mmio_masked_write() - Generic MHI MMIO register write masked API.
 * @dev:	MHI device structure.
 * @offset:	MHI address offset from base.
 * @mask:	Register field mask.
 * @shift:	Register field mask shift value.
 * @val:	Value to be written to the register offset.
 */
int mhi_dev_mmio_masked_write(struct mhi_dev *dev, uint32_t offset,
						uint32_t mask, uint32_t shift,
						uint32_t val);
/**
 * mhi_dev_mmio_masked_read() - Generic MHI MMIO register read masked API.
 * @dev:	MHI device structure.
 * @offset:	MHI address offset from base.
 * @mask:	Register field mask.
 * @shift:	Register field mask shift value.
 * @reg_val:	Pointer the register value is stored to.
 */
int mhi_dev_mmio_masked_read(struct mhi_dev *dev, uint32_t offset,
						uint32_t mask, uint32_t shift,
						uint32_t *reg_val);
/**
 * mhi_dev_mmio_enable_ctrl_interrupt() - Enable Control interrupt.
 * @dev:	MHI device structure.
 */
int mhi_dev_mmio_enable_ctrl_interrupt(struct mhi_dev *dev);

/**
 * mhi_dev_mmio_disable_ctrl_interrupt() - Disable Control interrupt.
 * @dev:	MHI device structure.
 */
int mhi_dev_mmio_disable_ctrl_interrupt(struct mhi_dev *dev);

/**
 * mhi_dev_mmio_read_ctrl_status_interrupt() - Read Control interrupt status.
 * @dev:	MHI device structure.
 */
int mhi_dev_mmio_read_ctrl_status_interrupt(struct mhi_dev *dev);

/**
 * mhi_dev_mmio_enable_cmdb_interrupt() - Enable Command doorbell interrupt.
 * @dev:	MHI device structure.
 */
int mhi_dev_mmio_enable_cmdb_interrupt(struct mhi_dev *dev);

/**
 * mhi_dev_mmio_disable_cmdb_interrupt() - Disable Command doorbell interrupt.
 * @dev:	MHI device structure.
 */
int mhi_dev_mmio_disable_cmdb_interrupt(struct mhi_dev *dev);

/**
 * mhi_dev_mmio_read_cmdb_interrupt() - Read Command doorbell status.
 * @dev:	MHI device structure.
 */
int mhi_dev_mmio_read_cmdb_status_interrupt(struct mhi_dev *dev);

/**
 * mhi_dev_mmio_enable_chdb_a7() - Enable Channel doorbell for a given
 *		channel id.
 * @dev:	MHI device structure.
 * @chdb_id:	Channel id number.
 */
int mhi_dev_mmio_enable_chdb_a7(struct mhi_dev *dev, uint32_t chdb_id);
/**
 * mhi_dev_mmio_disable_chdb_a7() - Disable Channel doorbell for a given
 *		channel id.
 * @dev:	MHI device structure.
 * @chdb_id:	Channel id number.
 */
int mhi_dev_mmio_disable_chdb_a7(struct mhi_dev *dev, uint32_t chdb_id);

/**
 * mhi_dev_mmio_enable_erdb_a7() - Enable Event ring doorbell for a given
 *		event ring id.
 * @dev:	MHI device structure.
 * @erdb_id:	Event ring id number.
 */
int mhi_dev_mmio_enable_erdb_a7(struct mhi_dev *dev, uint32_t erdb_id);

/**
 * mhi_dev_mmio_disable_erdb_a7() - Disable Event ring doorbell for a given
 *		event ring id.
 * @dev:	MHI device structure.
 * @erdb_id:	Event ring id number.
 */
int mhi_dev_mmio_disable_erdb_a7(struct mhi_dev *dev, uint32_t erdb_id);

/**
 * mhi_dev_mmio_enable_chdb_interrupts() - Enable all Channel doorbell
 *		interrupts.
 * @dev:	MHI device structure.
 */
int mhi_dev_mmio_enable_chdb_interrupts(struct mhi_dev *dev);

/**
 * mhi_dev_mmio_mask_chdb_interrupts() - Mask all Channel doorbell
 *		interrupts.
 * @dev:	MHI device structure.
 */
int mhi_dev_mmio_mask_chdb_interrupts(struct mhi_dev *dev);

/**
 * mhi_dev_mmio_read_chdb_interrupts() - Read all Channel doorbell
 *		interrupts.
 * @dev:	MHI device structure.
 */
int mhi_dev_mmio_read_chdb_status_interrupts(struct mhi_dev *dev);

/**
 * mhi_dev_mmio_enable_erdb_interrupts() - Enable all Event doorbell
 *		interrupts.
 * @dev:	MHI device structure.
 */
int mhi_dev_mmio_enable_erdb_interrupts(struct mhi_dev *dev);

/**
 * mhi_dev_mmio_mask_erdb_interrupts() - Mask all Event doorbell
 *		interrupts.
 * @dev:	MHI device structure.
 */
int mhi_dev_mmio_mask_erdb_interrupts(struct mhi_dev *dev);

/**
 * mhi_dev_mmio_read_erdb_interrupts() - Read all Event doorbell
 *		interrupts.
 * @dev:	MHI device structure.
 */
int mhi_dev_mmio_read_erdb_status_interrupts(struct mhi_dev *dev);

/**
 * mhi_dev_mmio_clear_interrupts() - Clear all doorbell interrupts.
 * @dev:	MHI device structure.
 */
int mhi_dev_mmio_clear_interrupts(struct mhi_dev *dev);

/**
 * mhi_dev_mmio_get_chc_base() - Fetch the Channel ring context base address.
 @dev:	MHI device structure.
 */
int mhi_dev_mmio_get_chc_base(struct mhi_dev *dev);

/**
 * mhi_dev_mmio_get_erc_base() - Fetch the Event ring context base address.
 * @dev:	MHI device structure.
 */
int mhi_dev_mmio_get_erc_base(struct mhi_dev *dev);

/**
 * mhi_dev_get_crc_base() - Fetch the Command ring context base address.
 * @dev:	MHI device structure.
 */
int mhi_dev_mmio_get_crc_base(struct mhi_dev *dev);

/**
 * mhi_dev_mmio_get_ch_db() - Fetch the Write offset of the Channel ring ID.
 * @dev:	MHI device structure.
 * @wr_offset:	Pointer of the write offset to be written to.
 */
int mhi_dev_mmio_get_ch_db(struct mhi_dev_ring *ring, uint64_t *wr_offset);

/**
 * mhi_dev_get_erc_base() - Fetch the Write offset of the Event ring ID.
 * @dev:	MHI device structure.
 * @wr_offset:	Pointer of the write offset to be written to.
 */
int mhi_dev_mmio_get_erc_db(struct mhi_dev_ring *ring, uint64_t *wr_offset);

/**
 * mhi_dev_get_cmd_base() - Fetch the Write offset of the Command ring ID.
 * @dev:	MHI device structure.
 * @wr_offset:	Pointer of the write offset to be written to.
 */
int mhi_dev_mmio_get_cmd_db(struct mhi_dev_ring *ring, uint64_t *wr_offset);

/**
 * mhi_dev_mmio_set_env() - Write the Execution Enviornment.
 * @dev:	MHI device structure.
 * @value:	Value of the EXEC EVN.
 */
int mhi_dev_mmio_set_env(struct mhi_dev *dev, uint32_t value);

/**
 * mhi_dev_mmio_reset() - Reset the MMIO done as part of initialization.
 * @dev:	MHI device structure.
 */
int mhi_dev_mmio_reset(struct mhi_dev *dev);

/**
 * mhi_dev_get_mhi_addr() - Fetches the Data and Control region from the Host.
 * @dev:	MHI device structure.
 */
int mhi_dev_get_mhi_addr(struct mhi_dev *dev);

/**
 * mhi_dev_get_mhi_state() - Fetches the MHI state such as M0/M1/M2/M3.
 * @dev:	MHI device structure.
 * @state:	Pointer of type mhi_dev_state
 */
int mhi_dev_mmio_get_mhi_state(struct mhi_dev *dev, enum mhi_dev_state *state);

/**
 * mhi_dev_mmio_init() - Initializes the MMIO and reads the Number of event
 *		rings, support number of channels, and offsets to the Channel
 *		and Event doorbell from the host.
 * @dev:	MHI device structure.
 */
int mhi_dev_mmio_init(struct mhi_dev *dev);

/**
 * mhi_dev_update_ner() - Update the number of event rings (NER) programmed by
 *		the host.
 * @dev:	MHI device structure.
 */
int mhi_dev_update_ner(struct mhi_dev *dev);

/**
 * mhi_dev_restore_mmio() - Restores the MMIO when MHI device comes out of M3.
 * @dev:	MHI device structure.
 */
int mhi_dev_restore_mmio(struct mhi_dev *dev);

/**
 * mhi_dev_backup_mmio() - Backup MMIO before a MHI transition to M3.
 * @dev:	MHI device structure.
 */
int mhi_dev_backup_mmio(struct mhi_dev *dev);

/**
 * mhi_dev_dump_mmio() - Memory dump of the MMIO region for debug.
 * @dev:	MHI device structure.
 */
int mhi_dev_dump_mmio(struct mhi_dev *dev);

/**
 * mhi_dev_config_outbound_iatu() - Configure Outbound Address translation
 *		unit between device and host to map the Data and Control
 *		information.
 * @dev:	MHI device structure.
 */
int mhi_dev_config_outbound_iatu(struct mhi_dev *mhi);

/**
 * mhi_dev_send_state_change_event() - Send state change event to the host
 *		such as M0/M1/M2/M3.
 * @dev:	MHI device structure.
 * @state:	MHI state of type mhi_dev_state
 */
int mhi_dev_send_state_change_event(struct mhi_dev *mhi,
					enum mhi_dev_state state);
/**
 * mhi_dev_send_ee_event() - Send Execution enviornment state change
 *		event to the host.
 * @dev:	MHI device structure.
 * @state:	MHI state of type mhi_dev_execenv
 */
int mhi_dev_send_ee_event(struct mhi_dev *mhi,
					enum mhi_dev_execenv exec_env);
/**
 * mhi_dev_syserr() - System error when unexpected events are received.
 * @dev:	MHI device structure.
 */
int mhi_dev_syserr(struct mhi_dev *mhi);

/**
 * mhi_dev_suspend() - MHI device suspend to stop channel processing at the
 *		Transfer ring boundary, update the channel state to suspended.
 * @dev:	MHI device structure.
 */
int mhi_dev_suspend(struct mhi_dev *mhi);

/**
 * mhi_dev_resume() - MHI device resume to update the channel state to running.
 * @dev:	MHI device structure.
 */
int mhi_dev_resume(struct mhi_dev *mhi);

/**
 * mhi_dev_trigger_hw_acc_wakeup() - Notify State machine there is HW
 *		accelerated data to be send and prevent MHI suspend.
 * @dev:	MHI device structure.
 */
int mhi_dev_trigger_hw_acc_wakeup(struct mhi_dev *mhi);

/**
 * mhi_pcie_config_db_routing() - Configure Doorbell for Event and Channel
 *		context with IPA when performing a MHI resume.
 * @dev:	MHI device structure.
 */
int mhi_pcie_config_db_routing(struct mhi_dev *mhi);

/**
 * mhi_uci_init() - Initializes the User control interface (UCI) which
 *		exposes device nodes for the supported MHI software
 *		channels.
 */
int mhi_uci_init(void);

void mhi_dev_notify_a7_event(struct mhi_dev *mhi);

#endif /* _MHI_H_ */
