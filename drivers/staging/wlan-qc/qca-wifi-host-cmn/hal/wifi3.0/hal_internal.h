/*
 * Copyright (c) 2016-2020 The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _HAL_INTERNAL_H_
#define _HAL_INTERNAL_H_

#include "qdf_types.h"
#include "qdf_atomic.h"
#include "qdf_lock.h"
#include "qdf_mem.h"
#include "qdf_nbuf.h"
#include "pld_common.h"
#ifdef FEATURE_HAL_DELAYED_REG_WRITE
#include "qdf_defer.h"
#endif

#define hal_alert(params...) QDF_TRACE_FATAL(QDF_MODULE_ID_HAL, params)
#define hal_err(params...) QDF_TRACE_ERROR(QDF_MODULE_ID_HAL, params)
#define hal_warn(params...) QDF_TRACE_WARN(QDF_MODULE_ID_HAL, params)
#define hal_info(params...) QDF_TRACE_INFO(QDF_MODULE_ID_HAL, params)
#define hal_debug(params...) QDF_TRACE_DEBUG(QDF_MODULE_ID_HAL, params)

#define hal_alert_rl(params...) QDF_TRACE_FATAL_RL(QDF_MODULE_ID_HAL, params)
#define hal_err_rl(params...) QDF_TRACE_ERROR_RL(QDF_MODULE_ID_HAL, params)
#define hal_warn_rl(params...) QDF_TRACE_WARN_RL(QDF_MODULE_ID_HAL, params)
#define hal_info_rl(params...) QDF_TRACE_INFO_RL(QDF_MODULE_ID_HAL, params)
#define hal_debug_rl(params...) QDF_TRACE_DEBUG_RL(QDF_MODULE_ID_HAL, params)

#ifdef ENABLE_VERBOSE_DEBUG
extern bool is_hal_verbose_debug_enabled;
#define hal_verbose_debug(params...) \
	if (unlikely(is_hal_verbose_debug_enabled)) \
		do {\
			QDF_TRACE_DEBUG(QDF_MODULE_ID_HAL, params); \
		} while (0)
#define hal_verbose_hex_dump(params...) \
	if (unlikely(is_hal_verbose_debug_enabled)) \
		do {\
			QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_HAL, \
					   QDF_TRACE_LEVEL_DEBUG, \
					   params); \
		} while (0)
#else
#define hal_verbose_debug(params...) QDF_TRACE_DEBUG(QDF_MODULE_ID_HAL, params)
#define hal_verbose_hex_dump(params...) \
		QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_HAL, QDF_TRACE_LEVEL_DEBUG, \
				   params)
#endif

/*
 * dp_hal_soc - opaque handle for DP HAL soc
 */
struct hal_soc_handle;
typedef struct hal_soc_handle *hal_soc_handle_t;

/**
 * hal_ring_desc - opaque handle for DP ring descriptor
 */
struct hal_ring_desc;
typedef struct hal_ring_desc *hal_ring_desc_t;

/**
 * hal_link_desc - opaque handle for DP link descriptor
 */
struct hal_link_desc;
typedef struct hal_link_desc *hal_link_desc_t;

/**
 * hal_rxdma_desc - opaque handle for DP rxdma dst ring descriptor
 */
struct hal_rxdma_desc;
typedef struct hal_rxdma_desc *hal_rxdma_desc_t;

/**
 * hal_buff_addrinfo - opaque handle for DP buffer address info
 */
struct hal_buff_addrinfo;
typedef struct hal_buff_addrinfo *hal_buff_addrinfo_t;

/**
 * hal_rx_mon_desc_info - opaque handle for sw monitor ring desc info
 */
struct hal_rx_mon_desc_info;
typedef struct hal_rx_mon_desc_info *hal_rx_mon_desc_info_t;

/* TBD: This should be movded to shared HW header file */
enum hal_srng_ring_id {
	/* UMAC rings */
	HAL_SRNG_REO2SW1 = 0,
	HAL_SRNG_REO2SW2 = 1,
	HAL_SRNG_REO2SW3 = 2,
	HAL_SRNG_REO2SW4 = 3,
	HAL_SRNG_REO2TCL = 4,
	HAL_SRNG_SW2REO = 5,
	/* 6-7 unused */
	HAL_SRNG_REO_CMD = 8,
	HAL_SRNG_REO_STATUS = 9,
	/* 10-15 unused */
	HAL_SRNG_SW2TCL1 = 16,
	HAL_SRNG_SW2TCL2 = 17,
	HAL_SRNG_SW2TCL3 = 18,
	HAL_SRNG_SW2TCL4 = 19, /* FW2TCL ring */
	/* 20-23 unused */
	HAL_SRNG_SW2TCL_CMD = 24,
	HAL_SRNG_TCL_STATUS = 25,
	/* 26-31 unused */
	HAL_SRNG_CE_0_SRC = 32,
	HAL_SRNG_CE_1_SRC = 33,
	HAL_SRNG_CE_2_SRC = 34,
	HAL_SRNG_CE_3_SRC = 35,
	HAL_SRNG_CE_4_SRC = 36,
	HAL_SRNG_CE_5_SRC = 37,
	HAL_SRNG_CE_6_SRC = 38,
	HAL_SRNG_CE_7_SRC = 39,
	HAL_SRNG_CE_8_SRC = 40,
	HAL_SRNG_CE_9_SRC = 41,
	HAL_SRNG_CE_10_SRC = 42,
	HAL_SRNG_CE_11_SRC = 43,
	/* 44-55 unused */
	HAL_SRNG_CE_0_DST = 56,
	HAL_SRNG_CE_1_DST = 57,
	HAL_SRNG_CE_2_DST = 58,
	HAL_SRNG_CE_3_DST = 59,
	HAL_SRNG_CE_4_DST = 60,
	HAL_SRNG_CE_5_DST = 61,
	HAL_SRNG_CE_6_DST = 62,
	HAL_SRNG_CE_7_DST = 63,
	HAL_SRNG_CE_8_DST = 64,
	HAL_SRNG_CE_9_DST = 65,
	HAL_SRNG_CE_10_DST = 66,
	HAL_SRNG_CE_11_DST = 67,
	/* 68-79 unused */
	HAL_SRNG_CE_0_DST_STATUS = 80,
	HAL_SRNG_CE_1_DST_STATUS = 81,
	HAL_SRNG_CE_2_DST_STATUS = 82,
	HAL_SRNG_CE_3_DST_STATUS = 83,
	HAL_SRNG_CE_4_DST_STATUS = 84,
	HAL_SRNG_CE_5_DST_STATUS = 85,
	HAL_SRNG_CE_6_DST_STATUS = 86,
	HAL_SRNG_CE_7_DST_STATUS = 87,
	HAL_SRNG_CE_8_DST_STATUS = 88,
	HAL_SRNG_CE_9_DST_STATUS = 89,
	HAL_SRNG_CE_10_DST_STATUS = 90,
	HAL_SRNG_CE_11_DST_STATUS = 91,
	/* 92-103 unused */
	HAL_SRNG_WBM_IDLE_LINK = 104,
	HAL_SRNG_WBM_SW_RELEASE = 105,
	HAL_SRNG_WBM2SW0_RELEASE = 106,
	HAL_SRNG_WBM2SW1_RELEASE = 107,
	HAL_SRNG_WBM2SW2_RELEASE = 108,
	HAL_SRNG_WBM2SW3_RELEASE = 109,
	/* 110-127 unused */
	HAL_SRNG_UMAC_ID_END = 127,
	/* LMAC rings - The following set will be replicated for each LMAC */
	HAL_SRNG_LMAC1_ID_START = 128,
	HAL_SRNG_WMAC1_SW2RXDMA0_BUF0 = HAL_SRNG_LMAC1_ID_START,
#ifdef IPA_OFFLOAD
	HAL_SRNG_WMAC1_SW2RXDMA0_BUF1 = (HAL_SRNG_LMAC1_ID_START + 1),
	HAL_SRNG_WMAC1_SW2RXDMA0_BUF2 = (HAL_SRNG_LMAC1_ID_START + 2),
	HAL_SRNG_WMAC1_SW2RXDMA1_BUF = (HAL_SRNG_WMAC1_SW2RXDMA0_BUF2 + 1),
#else
	HAL_SRNG_WMAC1_SW2RXDMA1_BUF = (HAL_SRNG_WMAC1_SW2RXDMA0_BUF0 + 1),
#endif
	HAL_SRNG_WMAC1_SW2RXDMA2_BUF = (HAL_SRNG_WMAC1_SW2RXDMA1_BUF + 1),
	HAL_SRNG_WMAC1_SW2RXDMA0_STATBUF = (HAL_SRNG_WMAC1_SW2RXDMA2_BUF + 1),
	HAL_SRNG_WMAC1_SW2RXDMA1_STATBUF =
					(HAL_SRNG_WMAC1_SW2RXDMA0_STATBUF + 1),
	HAL_SRNG_WMAC1_RXDMA2SW0 = (HAL_SRNG_WMAC1_SW2RXDMA1_STATBUF + 1),
	HAL_SRNG_WMAC1_RXDMA2SW1 = (HAL_SRNG_WMAC1_RXDMA2SW0 + 1),
	HAL_SRNG_WMAC1_SW2RXDMA1_DESC = (HAL_SRNG_WMAC1_RXDMA2SW1 + 1),
#ifdef WLAN_FEATURE_CIF_CFR
	HAL_SRNG_WIFI_POS_SRC_DMA_RING = (HAL_SRNG_WMAC1_SW2RXDMA1_DESC + 1),
	HAL_SRNG_DIR_BUF_RX_SRC_DMA_RING = (HAL_SRNG_WIFI_POS_SRC_DMA_RING + 1),
#else
	HAL_SRNG_DIR_BUF_RX_SRC_DMA_RING = (HAL_SRNG_WMAC1_SW2RXDMA1_DESC + 1),
#endif
	/* -142 unused */
	HAL_SRNG_LMAC1_ID_END = 143
};

#define HAL_RXDMA_MAX_RING_SIZE 0xFFFF
#define HAL_MAX_LMACS 3
#define HAL_MAX_RINGS_PER_LMAC (HAL_SRNG_LMAC1_ID_END - HAL_SRNG_LMAC1_ID_START)
#define HAL_MAX_LMAC_RINGS (HAL_MAX_LMACS * HAL_MAX_RINGS_PER_LMAC)

#define HAL_SRNG_ID_MAX (HAL_SRNG_UMAC_ID_END + HAL_MAX_LMAC_RINGS)

enum hal_srng_dir {
	HAL_SRNG_SRC_RING,
	HAL_SRNG_DST_RING
};

/* Lock wrappers for SRNG */
#define hal_srng_lock_t qdf_spinlock_t
#define SRNG_LOCK_INIT(_lock) qdf_spinlock_create(_lock)
#define SRNG_LOCK(_lock) qdf_spin_lock_bh(_lock)
#define SRNG_TRY_LOCK(_lock) qdf_spin_trylock_bh(_lock)
#define SRNG_UNLOCK(_lock) qdf_spin_unlock_bh(_lock)
#define SRNG_LOCK_DESTROY(_lock) qdf_spinlock_destroy(_lock)

struct hal_soc;

/**
 * dp_hal_ring - opaque handle for DP HAL SRNG
 */
struct hal_ring_handle;
typedef struct hal_ring_handle *hal_ring_handle_t;

#define MAX_SRNG_REG_GROUPS 2

/* Hal Srng bit mask
 * HAL_SRNG_FLUSH_EVENT: SRNG HP TP flush in case of link down
 */
#define HAL_SRNG_FLUSH_EVENT BIT(0)

#ifdef FEATURE_HAL_DELAYED_REG_WRITE

/**
 * struct hal_reg_write_q_elem - delayed register write queue element
 * @srng: hal_srng queued for a delayed write
 * @addr: iomem address of the register
 * @enqueue_val: register value at the time of delayed write enqueue
 * @dequeue_val: register value at the time of delayed write dequeue
 * @valid: whether this entry is valid or not
 * @enqueue_time: enqueue time (qdf_log_timestamp)
 * @work_scheduled_time: work scheduled time (qdf_log_timestamp)
 * @dequeue_time: dequeue time (qdf_log_timestamp)
 */
struct hal_reg_write_q_elem {
	struct hal_srng *srng;
	void __iomem *addr;
	uint32_t enqueue_val;
	uint32_t dequeue_val;
	uint8_t valid;
	qdf_time_t enqueue_time;
	qdf_time_t work_scheduled_time;
	qdf_time_t dequeue_time;
};

/**
 * struct hal_reg_write_srng_stats - srng stats to keep track of register writes
 * @enqueues: writes enqueued to delayed work
 * @dequeues: writes dequeued from delayed work (not written yet)
 * @coalesces: writes not enqueued since srng is already queued up
 * @direct: writes not enqueued and written to register directly
 */
struct hal_reg_write_srng_stats {
	uint32_t enqueues;
	uint32_t dequeues;
	uint32_t coalesces;
	uint32_t direct;
};

/**
 * enum hal_reg_sched_delay - ENUM for write sched delay histogram
 * @REG_WRITE_SCHED_DELAY_SUB_100us: index for delay < 100us
 * @REG_WRITE_SCHED_DELAY_SUB_1000us: index for delay < 1000us
 * @REG_WRITE_SCHED_DELAY_SUB_5000us: index for delay < 5000us
 * @REG_WRITE_SCHED_DELAY_GT_5000us: index for delay >= 5000us
 * @REG_WRITE_SCHED_DELAY_HIST_MAX: Max value (nnsize of histogram array)
 */
enum hal_reg_sched_delay {
	REG_WRITE_SCHED_DELAY_SUB_100us,
	REG_WRITE_SCHED_DELAY_SUB_1000us,
	REG_WRITE_SCHED_DELAY_SUB_5000us,
	REG_WRITE_SCHED_DELAY_GT_5000us,
	REG_WRITE_SCHED_DELAY_HIST_MAX,
};

/**
 * struct hal_reg_write_soc_stats - soc stats to keep track of register writes
 * @enqueues: writes enqueued to delayed work
 * @dequeues: writes dequeued from delayed work (not written yet)
 * @coalesces: writes not enqueued since srng is already queued up
 * @direct: writes not enqueud and writted to register directly
 * @prevent_l1_fails: prevent l1 API failed
 * @q_depth: current queue depth in delayed register write queue
 * @max_q_depth: maximum queue for delayed register write queue
 * @sched_delay: = kernel work sched delay + bus wakeup delay, histogram
 */
struct hal_reg_write_soc_stats {
	qdf_atomic_t enqueues;
	uint32_t dequeues;
	qdf_atomic_t coalesces;
	qdf_atomic_t direct;
	uint32_t prevent_l1_fails;
	qdf_atomic_t q_depth;
	uint32_t max_q_depth;
	uint32_t sched_delay[REG_WRITE_SCHED_DELAY_HIST_MAX];
};
#endif

/* Common SRNG ring structure for source and destination rings */
struct hal_srng {
	/* Unique SRNG ring ID */
	uint8_t ring_id;

	/* Ring initialization done */
	uint8_t initialized;

	/* Interrupt/MSI value assigned to this ring */
	int irq;

	/* Physical base address of the ring */
	qdf_dma_addr_t ring_base_paddr;

	/* Virtual base address of the ring */
	uint32_t *ring_base_vaddr;

	/* Number of entries in ring */
	uint32_t num_entries;

	/* Ring size */
	uint32_t ring_size;

	/* Ring size mask */
	uint32_t ring_size_mask;

	/* Size of ring entry */
	uint32_t entry_size;

	/* Interrupt timer threshold – in micro seconds */
	uint32_t intr_timer_thres_us;

	/* Interrupt batch counter threshold – in number of ring entries */
	uint32_t intr_batch_cntr_thres_entries;

	/* Applicable only for CE dest ring */
	uint32_t prefetch_timer;

	/* MSI Address */
	qdf_dma_addr_t msi_addr;

	/* MSI data */
	uint32_t msi_data;

	/* Misc flags */
	uint32_t flags;

	/* Lock for serializing ring index updates */
	hal_srng_lock_t lock;

	/* Start offset of SRNG register groups for this ring
	 * TBD: See if this is required - register address can be derived
	 * from ring ID
	 */
	void *hwreg_base[MAX_SRNG_REG_GROUPS];

	/* Source or Destination ring */
	enum hal_srng_dir ring_dir;

	union {
		struct {
			/* SW tail pointer */
			uint32_t tp;

			/* Shadow head pointer location to be updated by HW */
			uint32_t *hp_addr;

			/* Cached head pointer */
			uint32_t cached_hp;

			/* Tail pointer location to be updated by SW – This
			 * will be a register address and need not be
			 * accessed through SW structure */
			uint32_t *tp_addr;

			/* Current SW loop cnt */
			uint32_t loop_cnt;

			/* max transfer size */
			uint16_t max_buffer_length;
		} dst_ring;

		struct {
			/* SW head pointer */
			uint32_t hp;

			/* SW reap head pointer */
			uint32_t reap_hp;

			/* Shadow tail pointer location to be updated by HW */
			uint32_t *tp_addr;

			/* Cached tail pointer */
			uint32_t cached_tp;

			/* Head pointer location to be updated by SW – This
			 * will be a register address and need not be accessed
			 * through SW structure */
			uint32_t *hp_addr;

			/* Low threshold – in number of ring entries */
			uint32_t low_threshold;
		} src_ring;
	} u;

	struct hal_soc *hal_soc;

	/* Number of times hp/tp updated in runtime resume */
	uint32_t flush_count;
	/* hal srng event flag*/
	unsigned long srng_event;
	/* last flushed time stamp */
	uint64_t last_flush_ts;
#ifdef FEATURE_HAL_DELAYED_REG_WRITE
	/* flag to indicate whether srng is already queued for delayed write */
	uint8_t reg_write_in_progress;

	/* srng specific delayed write stats */
	struct hal_reg_write_srng_stats wstats;
#endif
};

/* HW SRNG configuration table */
struct hal_hw_srng_config {
	int start_ring_id;
	uint16_t max_rings;
	uint16_t entry_size;
	uint32_t reg_start[MAX_SRNG_REG_GROUPS];
	uint16_t reg_size[MAX_SRNG_REG_GROUPS];
	uint8_t lmac_ring;
	enum hal_srng_dir ring_dir;
	uint32_t max_size;
};

#define MAX_SHADOW_REGISTERS 36
#define MAX_GENERIC_SHADOW_REG 5

/**
 * struct shadow_reg_config - Hal soc structure that contains
 * the list of generic shadow registers
 * @target_register: target reg offset
 * @shadow_config_index: shadow config index in shadow config
 *				list sent to FW
 * @va: virtual addr of shadow reg
 *
 * This structure holds the generic registers that are mapped to
 * the shadow region and holds the mapping of the target
 * register offset to shadow config index provided to FW during
 * init
 */
struct shadow_reg_config {
	uint32_t target_register;
	int shadow_config_index;
	uint64_t va;
};

/* REO parameters to be passed to hal_reo_setup */
struct hal_reo_params {
	/** rx hash steering enabled or disabled */
	bool rx_hash_enabled;
	/** reo remap 1 register */
	uint32_t remap1;
	/** reo remap 2 register */
	uint32_t remap2;
	/** fragment destination ring */
	uint8_t frag_dst_ring;
	/** padding */
	uint8_t padding[3];
};

struct hal_hw_txrx_ops {

	/* init and setup */
	void (*hal_srng_dst_hw_init)(struct hal_soc *hal,
				     struct hal_srng *srng);
	void (*hal_srng_src_hw_init)(struct hal_soc *hal,
				     struct hal_srng *srng);
	void (*hal_get_hw_hptp)(struct hal_soc *hal,
				hal_ring_handle_t hal_ring_hdl,
				uint32_t *headp, uint32_t *tailp,
				uint8_t ring_type);
	void (*hal_reo_setup)(struct hal_soc *hal_soc, void *reoparams);
	void (*hal_setup_link_idle_list)(
				struct hal_soc *hal_soc,
				qdf_dma_addr_t scatter_bufs_base_paddr[],
				void *scatter_bufs_base_vaddr[],
				uint32_t num_scatter_bufs,
				uint32_t scatter_buf_size,
				uint32_t last_buf_end_offset,
				uint32_t num_entries);
	qdf_iomem_t (*hal_get_window_address)(struct hal_soc *hal_soc,
					      qdf_iomem_t addr);
	void (*hal_reo_set_err_dst_remap)(void *hal_soc);

	/* tx */
	void (*hal_tx_desc_set_dscp_tid_table_id)(void *desc, uint8_t id);
	void (*hal_tx_set_dscp_tid_map)(struct hal_soc *hal_soc, uint8_t *map,
					uint8_t id);
	void (*hal_tx_update_dscp_tid)(struct hal_soc *hal_soc, uint8_t tid,
				       uint8_t id,
				       uint8_t dscp);
	void (*hal_tx_desc_set_lmac_id)(void *desc, uint8_t lmac_id);
	 void (*hal_tx_desc_set_buf_addr)(void *desc, dma_addr_t paddr,
			uint8_t pool_id, uint32_t desc_id, uint8_t type);
	void (*hal_tx_desc_set_search_type)(void *desc, uint8_t search_type);
	void (*hal_tx_desc_set_search_index)(void *desc, uint32_t search_index);
	void (*hal_tx_desc_set_cache_set_num)(void *desc, uint8_t search_index);
	void (*hal_tx_comp_get_status)(void *desc, void *ts,
				       struct hal_soc *hal);
	uint8_t (*hal_tx_comp_get_release_reason)(void *hal_desc);
	uint8_t (*hal_get_wbm_internal_error)(void *hal_desc);
	void (*hal_tx_desc_set_mesh_en)(void *desc, uint8_t en);
	void (*hal_tx_init_cmd_credit_ring)(hal_soc_handle_t hal_soc_hdl,
					    hal_ring_handle_t hal_ring_hdl);

	/* rx */
	uint32_t (*hal_rx_msdu_start_nss_get)(uint8_t *);
	void (*hal_rx_mon_hw_desc_get_mpdu_status)(void *hw_desc_addr,
						   struct mon_rx_status *rs);
	uint8_t (*hal_rx_get_tlv)(void *rx_tlv);
	void (*hal_rx_proc_phyrx_other_receive_info_tlv)(void *rx_tlv_hdr,
							void *ppdu_info_handle);
	void (*hal_rx_dump_msdu_start_tlv)(void *msdu_start, uint8_t dbg_level);
	void (*hal_rx_dump_msdu_end_tlv)(void *msdu_end,
					 uint8_t dbg_level);
	uint32_t (*hal_get_link_desc_size)(void);
	uint32_t (*hal_rx_mpdu_start_tid_get)(uint8_t *buf);
	uint32_t (*hal_rx_msdu_start_reception_type_get)(uint8_t *buf);
	uint16_t (*hal_rx_msdu_end_da_idx_get)(uint8_t *buf);
	void* (*hal_rx_msdu_desc_info_get_ptr)(void *msdu_details_ptr);
	void* (*hal_rx_link_desc_msdu0_ptr)(void *msdu_link_ptr);
	void (*hal_reo_status_get_header)(uint32_t *d, int b, void *h);
	uint32_t (*hal_rx_status_get_tlv_info)(void *rx_tlv_hdr,
					       void *ppdu_info,
					       hal_soc_handle_t hal_soc_hdl,
					       qdf_nbuf_t nbuf);
	void (*hal_rx_wbm_err_info_get)(void *wbm_desc,
				void *wbm_er_info);
	void (*hal_rx_dump_mpdu_start_tlv)(void *mpdustart,
						uint8_t dbg_level);

	void (*hal_tx_set_pcp_tid_map)(struct hal_soc *hal_soc, uint8_t *map);
	void (*hal_tx_update_pcp_tid_map)(struct hal_soc *hal_soc, uint8_t pcp,
					  uint8_t id);
	void (*hal_tx_set_tidmap_prty)(struct hal_soc *hal_soc, uint8_t prio);
	uint8_t (*hal_rx_get_rx_fragment_number)(uint8_t *buf);
	uint8_t (*hal_rx_msdu_end_da_is_mcbc_get)(uint8_t *buf);
	uint8_t (*hal_rx_msdu_end_sa_is_valid_get)(uint8_t *buf);
	uint16_t (*hal_rx_msdu_end_sa_idx_get)(uint8_t *buf);
	uint32_t (*hal_rx_desc_is_first_msdu)(void *hw_desc_addr);
	uint32_t (*hal_rx_msdu_end_l3_hdr_padding_get)(uint8_t *buf);
	uint32_t (*hal_rx_encryption_info_valid)(uint8_t *buf);
	void (*hal_rx_print_pn)(uint8_t *buf);
	uint8_t (*hal_rx_msdu_end_first_msdu_get)(uint8_t *buf);
	uint8_t (*hal_rx_msdu_end_da_is_valid_get)(uint8_t *buf);
	uint8_t (*hal_rx_msdu_end_last_msdu_get)(uint8_t *buf);
	bool (*hal_rx_get_mpdu_mac_ad4_valid)(uint8_t *buf);
	uint32_t (*hal_rx_mpdu_start_sw_peer_id_get)(uint8_t *buf);
	uint32_t (*hal_rx_mpdu_get_to_ds)(uint8_t *buf);
	uint32_t (*hal_rx_mpdu_get_fr_ds)(uint8_t *buf);
	uint8_t (*hal_rx_get_mpdu_frame_control_valid)(uint8_t *buf);
	QDF_STATUS
		(*hal_rx_mpdu_get_addr1)(uint8_t *buf, uint8_t *mac_addr);
	QDF_STATUS
		(*hal_rx_mpdu_get_addr2)(uint8_t *buf, uint8_t *mac_addr);
	QDF_STATUS
		(*hal_rx_mpdu_get_addr3)(uint8_t *buf, uint8_t *mac_addr);
	QDF_STATUS
		(*hal_rx_mpdu_get_addr4)(uint8_t *buf, uint8_t *mac_addr);
	uint8_t (*hal_rx_get_mpdu_sequence_control_valid)(uint8_t *buf);
	bool (*hal_rx_is_unicast)(uint8_t *buf);
	uint32_t (*hal_rx_tid_get)(hal_soc_handle_t hal_soc_hdl, uint8_t *buf);
	uint32_t (*hal_rx_hw_desc_get_ppduid_get)(void *rx_tlv_hdr,
						  void *rxdma_dst_ring_desc);
	uint32_t (*hal_rx_mpdu_start_mpdu_qos_control_valid_get)(uint8_t *buf);
	uint32_t (*hal_rx_msdu_end_sa_sw_peer_id_get)(uint8_t *buf);
	void * (*hal_rx_msdu0_buffer_addr_lsb)(void *link_desc_addr);
	void * (*hal_rx_msdu_desc_info_ptr_get)(void *msdu0);
	void * (*hal_ent_mpdu_desc_info)(void *hw_addr);
	void * (*hal_dst_mpdu_desc_info)(void *hw_addr);
	uint8_t (*hal_rx_get_fc_valid)(uint8_t *buf);
	uint8_t (*hal_rx_get_to_ds_flag)(uint8_t *buf);
	uint8_t (*hal_rx_get_mac_addr2_valid)(uint8_t *buf);
	uint8_t (*hal_rx_get_filter_category)(uint8_t *buf);
	uint32_t (*hal_rx_get_ppdu_id)(uint8_t *buf);
	void (*hal_reo_config)(struct hal_soc *soc,
			       uint32_t reg_val,
			       struct hal_reo_params *reo_params);
	uint32_t (*hal_rx_msdu_flow_idx_get)(uint8_t *buf);
	bool (*hal_rx_msdu_flow_idx_invalid)(uint8_t *buf);
	bool (*hal_rx_msdu_flow_idx_timeout)(uint8_t *buf);
	uint32_t (*hal_rx_msdu_fse_metadata_get)(uint8_t *buf);
	uint16_t (*hal_rx_msdu_cce_metadata_get)(uint8_t *buf);
	void
	    (*hal_rx_msdu_get_flow_params)(
					  uint8_t *buf,
					  bool *flow_invalid,
					  bool *flow_timeout,
					  uint32_t *flow_index);
	uint16_t (*hal_rx_tlv_get_tcp_chksum)(uint8_t *buf);
	uint16_t (*hal_rx_get_rx_sequence)(uint8_t *buf);
	void (*hal_rx_get_bb_info)(void *rx_tlv, void *ppdu_info_handle);
	void (*hal_rx_get_rtt_info)(void *rx_tlv, void *ppdu_info_handle);
	void (*hal_rx_msdu_packet_metadata_get)(uint8_t *buf,
						void *msdu_pkt_metadata);
	uint16_t (*hal_rx_get_fisa_cumulative_l4_checksum)(uint8_t *buf);
	uint16_t (*hal_rx_get_fisa_cumulative_ip_length)(uint8_t *buf);
	bool (*hal_rx_get_udp_proto)(uint8_t *buf);
	bool (*hal_rx_get_fisa_flow_agg_continuation)(uint8_t *buf);
	uint8_t (*hal_rx_get_fisa_flow_agg_count)(uint8_t *buf);
	bool (*hal_rx_get_fisa_timeout)(uint8_t *buf);
	uint8_t (*hal_rx_mpdu_start_tlv_tag_valid)(void *rx_tlv_hdr);
	void (*hal_rx_sw_mon_desc_info_get)(hal_ring_desc_t rxdma_dst_ring_desc,
					    hal_rx_mon_desc_info_t mon_desc_info);
	uint8_t (*hal_rx_wbm_err_msdu_continuation_get)(void *ring_desc);
	uint32_t (*hal_rx_msdu_end_offset_get)(void);
	uint32_t (*hal_rx_attn_offset_get)(void);
	uint32_t (*hal_rx_msdu_start_offset_get)(void);
	uint32_t (*hal_rx_mpdu_start_offset_get)(void);
	uint32_t (*hal_rx_mpdu_end_offset_get)(void);
	void * (*hal_rx_flow_setup_fse)(uint8_t *rx_fst,
					uint32_t table_offset,
					uint8_t *rx_flow);
	void (*hal_compute_reo_remap_ix2_ix3)(uint32_t *ring,
					      uint32_t num_rings,
					      uint32_t *remap1,
					      uint32_t *remap2);
	uint32_t (*hal_rx_flow_setup_cmem_fse)(
				struct hal_soc *soc, uint32_t cmem_ba,
				uint32_t table_offset, uint8_t *rx_flow);
	uint32_t (*hal_rx_flow_get_cmem_fse_ts)(struct hal_soc *soc,
						uint32_t fse_offset);
	void (*hal_rx_flow_get_cmem_fse)(struct hal_soc *soc,
					 uint32_t fse_offset,
					 uint32_t *fse, qdf_size_t len);
	void (*hal_rx_msdu_get_reo_destination_indication)(uint8_t *buf,
							   uint32_t *reo_destination_indication);
};

/**
 * struct hal_soc_stats - Hal layer stats
 * @reg_write_fail: number of failed register writes
 * @wstats: delayed register write stats
 * @shadow_reg_write_fail: shadow reg write failure stats
 * @shadow_reg_write_succ: shadow reg write success stats
 *
 * This structure holds all the statistics at HAL layer.
 */
struct hal_soc_stats {
	uint32_t reg_write_fail;
#ifdef FEATURE_HAL_DELAYED_REG_WRITE
	struct hal_reg_write_soc_stats wstats;
#endif
#ifdef GENERIC_SHADOW_REGISTER_ACCESS_ENABLE
	uint32_t shadow_reg_write_fail;
	uint32_t shadow_reg_write_succ;
#endif
};

#ifdef ENABLE_HAL_REG_WR_HISTORY
/* The history size should always be a power of 2 */
#define HAL_REG_WRITE_HIST_SIZE 8

/**
 * struct hal_reg_write_fail_entry - Record of
 *		register write which failed.
 * @timestamp: timestamp of reg write failure
 * @reg_offset: offset of register where the write failed
 * @write_val: the value which was to be written
 * @read_val: the value read back from the register after write
 */
struct hal_reg_write_fail_entry {
	uint64_t timestamp;
	uint32_t reg_offset;
	uint32_t write_val;
	uint32_t read_val;
};

/**
 * struct hal_reg_write_fail_history - Hal layer history
 *		of all the register write failures.
 * @index: index to add the new record
 * @record: array of all the records in history
 *
 * This structure holds the history of register write
 * failures at HAL layer.
 */
struct hal_reg_write_fail_history {
	qdf_atomic_t index;
	struct hal_reg_write_fail_entry record[HAL_REG_WRITE_HIST_SIZE];
};
#endif

/**
 * struct hal_soc - HAL context to be used to access SRNG APIs
 *		    (currently used by data path and
 *		    transport (CE) modules)
 * @list_shadow_reg_config: array of generic regs mapped to
 *			    shadow regs
 * @num_generic_shadow_regs_configured: number of generic regs
 *					mapped to shadow regs
 */
struct hal_soc {
	/* HIF handle to access HW registers */
	struct hif_opaque_softc *hif_handle;

	/* QDF device handle */
	qdf_device_t qdf_dev;

	/* Device base address */
	void *dev_base_addr;
	/* Device base address for ce - qca5018 target */
	void *dev_base_addr_ce;

	/* HAL internal state for all SRNG rings.
	 * TODO: See if this is required
	 */
	struct hal_srng srng_list[HAL_SRNG_ID_MAX];

	/* Remote pointer memory for HW/FW updates */
	uint32_t *shadow_rdptr_mem_vaddr;
	qdf_dma_addr_t shadow_rdptr_mem_paddr;

	/* Shared memory for ring pointer updates from host to FW */
	uint32_t *shadow_wrptr_mem_vaddr;
	qdf_dma_addr_t shadow_wrptr_mem_paddr;

	/* REO blocking resource index */
	uint8_t reo_res_bitmap;
	uint8_t index;
	uint32_t target_type;

	/* shadow register configuration */
	struct pld_shadow_reg_v2_cfg shadow_config[MAX_SHADOW_REGISTERS];
	int num_shadow_registers_configured;
	bool use_register_windowing;
	uint32_t register_window;
	qdf_spinlock_t register_access_lock;

	/* Static window map configuration for multiple window write*/
	bool static_window_map;

	/* srng table */
	struct hal_hw_srng_config *hw_srng_table;
	int32_t *hal_hw_reg_offset;
	struct hal_hw_txrx_ops *ops;

	/* Indicate srngs initialization */
	bool init_phase;
	/* Hal level stats */
	struct hal_soc_stats stats;
#ifdef ENABLE_HAL_REG_WR_HISTORY
	struct hal_reg_write_fail_history *reg_wr_fail_hist;
#endif
#ifdef FEATURE_HAL_DELAYED_REG_WRITE
	/* queue(array) to hold register writes */
	struct hal_reg_write_q_elem *reg_write_queue;
	/* delayed work to be queued into workqueue */
	qdf_work_t reg_write_work;
	/* workqueue for delayed register writes */
	qdf_workqueue_t *reg_write_wq;
	/* write index used by caller to enqueue delayed work */
	qdf_atomic_t write_idx;
	/* read index used by worker thread to dequeue/write registers */
	uint32_t read_idx;
#endif
	qdf_atomic_t active_work_cnt;
#ifdef GENERIC_SHADOW_REGISTER_ACCESS_ENABLE
	struct shadow_reg_config
		list_shadow_reg_config[MAX_GENERIC_SHADOW_REG];
	int num_generic_shadow_regs_configured;
#endif
};

#ifdef FEATURE_HAL_DELAYED_REG_WRITE
/**
 *  hal_delayed_reg_write() - delayed regiter write
 * @hal_soc: HAL soc handle
 * @srng: hal srng
 * @addr: iomem address
 * @value: value to be written
 *
 * Return: none
 */
void hal_delayed_reg_write(struct hal_soc *hal_soc,
			   struct hal_srng *srng,
			   void __iomem *addr,
			   uint32_t value);
#endif

void hal_qca6750_attach(struct hal_soc *hal_soc);
void hal_qca6490_attach(struct hal_soc *hal_soc);
void hal_qca6390_attach(struct hal_soc *hal_soc);
void hal_qca6290_attach(struct hal_soc *hal_soc);
void hal_qca8074_attach(struct hal_soc *hal_soc);

/*
 * hal_soc_to_dp_hal_roc - API to convert hal_soc to opaque
 * dp_hal_soc handle type
 * @hal_soc - hal_soc type
 *
 * Return: hal_soc_handle_t type
 */
static inline
hal_soc_handle_t hal_soc_to_hal_soc_handle(struct hal_soc *hal_soc)
{
	return (hal_soc_handle_t)hal_soc;
}

/*
 * hal_srng_to_hal_ring_handle - API to convert hal_srng to opaque
 * dp_hal_ring handle type
 * @hal_srng - hal_srng type
 *
 * Return: hal_ring_handle_t type
 */
static inline
hal_ring_handle_t hal_srng_to_hal_ring_handle(struct hal_srng *hal_srng)
{
	return (hal_ring_handle_t)hal_srng;
}

/*
 * hal_ring_handle_to_hal_srng - API to convert dp_hal_ring to hal_srng handle
 * @hal_ring - hal_ring_handle_t type
 *
 * Return: hal_srng pointer type
 */
static inline
struct hal_srng *hal_ring_handle_to_hal_srng(hal_ring_handle_t hal_ring)
{
	return (struct hal_srng *)hal_ring;
}
#endif /* _HAL_INTERNAL_H_ */
