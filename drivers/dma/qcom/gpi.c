/* Copyright (c) 2017-2018, The Linux Foundation. All rights reserved.
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

#include <asm/dma-iommu.h>
#include <linux/atomic.h>
#include <linux/completion.h>
#include <linux/debugfs.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <linux/dmaengine.h>
#include <linux/io.h>
#include <linux/iommu.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/ipc_logging.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_dma.h>
#include <linux/of_irq.h>
#include <linux/platform_device.h>
#include <linux/scatterlist.h>
#include <linux/sched_clock.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <asm/cacheflush.h>
#include <linux/msm_gpi.h>
#include "../dmaengine.h"
#include "../virt-dma.h"
#include "msm_gpi_mmio.h"

/* global logging macros */
#define GPI_LOG(gpi_dev, fmt, ...) do { \
	if (gpi_dev->klog_lvl != LOG_LVL_MASK_ALL) \
		dev_dbg(gpi_dev->dev, "%s: " fmt, __func__, ##__VA_ARGS__); \
	if (gpi_dev->ilctxt && gpi_dev->ipc_log_lvl != LOG_LVL_MASK_ALL) \
		ipc_log_string(gpi_dev->ilctxt, \
			"%s: " fmt, __func__, ##__VA_ARGS__); \
	} while (0)
#define GPI_ERR(gpi_dev, fmt, ...) do { \
	if (gpi_dev->klog_lvl >= LOG_LVL_ERROR) \
		dev_err(gpi_dev->dev, "%s: " fmt, __func__, ##__VA_ARGS__); \
	if (gpi_dev->ilctxt && gpi_dev->ipc_log_lvl >= LOG_LVL_ERROR) \
		ipc_log_string(gpi_dev->ilctxt, \
			"%s: " fmt, __func__, ##__VA_ARGS__); \
	} while (0)

/* gpii specific logging macros */
#define GPII_INFO(gpii, ch, fmt, ...) do { \
	if (gpii->klog_lvl >= LOG_LVL_INFO) \
		pr_info("%s:%u:%s: " fmt, gpii->label, ch, \
			__func__, ##__VA_ARGS__); \
	if (gpii->ilctxt && gpii->ipc_log_lvl >= LOG_LVL_INFO) \
		ipc_log_string(gpii->ilctxt, \
			       "ch:%u %s: " fmt, ch, \
			       __func__, ##__VA_ARGS__); \
	} while (0)
#define GPII_ERR(gpii, ch, fmt, ...) do { \
	if (gpii->klog_lvl >= LOG_LVL_ERROR) \
		pr_err("%s:%u:%s: " fmt, gpii->label, ch, \
		       __func__, ##__VA_ARGS__); \
	if (gpii->ilctxt && gpii->ipc_log_lvl >= LOG_LVL_ERROR) \
		ipc_log_string(gpii->ilctxt, \
			       "ch:%u %s: " fmt, ch, \
			       __func__, ##__VA_ARGS__); \
	} while (0)
#define GPII_CRITIC(gpii, ch, fmt, ...) do { \
	if (gpii->klog_lvl >= LOG_LVL_CRITICAL) \
		pr_err("%s:%u:%s: " fmt, gpii->label, ch, \
		       __func__, ##__VA_ARGS__); \
	if (gpii->ilctxt && gpii->ipc_log_lvl >= LOG_LVL_CRITICAL) \
		ipc_log_string(gpii->ilctxt, \
			       "ch:%u %s: " fmt, ch, \
			       __func__, ##__VA_ARGS__); \
	} while (0)

enum DEBUG_LOG_LVL {
	LOG_LVL_MASK_ALL,
	LOG_LVL_CRITICAL,
	LOG_LVL_ERROR,
	LOG_LVL_INFO,
	LOG_LVL_VERBOSE,
	LOG_LVL_REG_ACCESS,
};

enum EV_PRIORITY {
	EV_PRIORITY_ISR,
	EV_PRIORITY_TASKLET,
};

#define GPI_DMA_DRV_NAME "gpi_dma"
#define DEFAULT_KLOG_LVL (LOG_LVL_CRITICAL)
#ifdef CONFIG_QCOM_GPI_DMA_DEBUG
#define DEFAULT_IPC_LOG_LVL (LOG_LVL_VERBOSE)
#define IPC_LOG_PAGES (40)
#define GPI_DBG_LOG_SIZE (SZ_1K) /* size must be power of 2 */
#define CMD_TIMEOUT_MS (1000)
#define GPII_REG(gpii, ch, fmt, ...) do { \
	if (gpii->klog_lvl >= LOG_LVL_REG_ACCESS) \
		pr_info("%s:%u:%s: " fmt, gpii->label, \
			ch, __func__, ##__VA_ARGS__); \
	if (gpii->ilctxt && gpii->ipc_log_lvl >= LOG_LVL_REG_ACCESS) \
		ipc_log_string(gpii->ilctxt, \
			       "ch:%u %s: " fmt, ch, \
			       __func__, ##__VA_ARGS__); \
	} while (0)
#define GPII_VERB(gpii, ch, fmt, ...) do { \
	if (gpii->klog_lvl >= LOG_LVL_VERBOSE) \
		pr_info("%s:%u:%s: " fmt, gpii->label, \
			ch, __func__, ##__VA_ARGS__); \
	if (gpii->ilctxt && gpii->ipc_log_lvl >= LOG_LVL_VERBOSE) \
		ipc_log_string(gpii->ilctxt, \
			       "ch:%u %s: " fmt, ch, \
			       __func__, ##__VA_ARGS__); \
	} while (0)

#else
#define IPC_LOG_PAGES (2)
#define GPI_DBG_LOG_SIZE (0) /* size must be power of 2 */
#define DEFAULT_IPC_LOG_LVL (LOG_LVL_ERROR)
#define CMD_TIMEOUT_MS (250)
/* verbose and register logging are disabled if !debug */
#define GPII_REG(gpii, ch, fmt, ...)
#define GPII_VERB(gpii, ch, fmt, ...)
#endif

#define GPI_LABEL_SIZE (256)
#define GPI_DBG_COMMON (99)
#define MAX_CHANNELS_PER_GPII (2)
#define GPI_TX_CHAN (0)
#define GPI_RX_CHAN (1)
#define STATE_IGNORE (U32_MAX)
#define REQ_OF_DMA_ARGS (5) /* # of arguments required from client */

struct __packed gpi_error_log_entry {
	u32 routine : 4;
	u32 type : 4;
	u32 reserved0 : 4;
	u32 code : 4;
	u32 reserved1 : 3;
	u32 chid : 5;
	u32 reserved2 : 1;
	u32 chtype : 1;
	u32 ee : 1;
};

struct __packed xfer_compl_event {
	u64 ptr;
	u32 length : 24;
	u8 code;
	u16 status;
	u8 type;
	u8 chid;
};

struct __packed immediate_data_event {
	u8 data_bytes[8];
	u8 length : 4;
	u8 resvd : 4;
	u16 tre_index;
	u8 code;
	u16 status;
	u8 type;
	u8 chid;
};

struct __packed qup_notif_event {
	u32 status;
	u32 time;
	u32 count :24;
	u8 resvd;
	u16 resvd1;
	u8 type;
	u8 chid;
};

struct __packed gpi_ere {
	u32 dword[4];
};

enum GPI_EV_TYPE {
	XFER_COMPLETE_EV_TYPE = 0x22,
	IMMEDIATE_DATA_EV_TYPE = 0x30,
	QUP_NOTIF_EV_TYPE = 0x31,
	STALE_EV_TYPE = 0xFF,
};

union __packed gpi_event {
	struct __packed xfer_compl_event xfer_compl_event;
	struct __packed immediate_data_event immediate_data_event;
	struct __packed qup_notif_event qup_notif_event;
	struct __packed gpi_ere gpi_ere;
};

enum gpii_irq_settings {
	DEFAULT_IRQ_SETTINGS,
	MASK_IEOB_SETTINGS,
};

enum gpi_ev_state {
	DEFAULT_EV_CH_STATE = 0,
	EV_STATE_NOT_ALLOCATED = DEFAULT_EV_CH_STATE,
	EV_STATE_ALLOCATED,
	MAX_EV_STATES
};

static const char *const gpi_ev_state_str[MAX_EV_STATES] = {
	[EV_STATE_NOT_ALLOCATED] = "NOT ALLOCATED",
	[EV_STATE_ALLOCATED] = "ALLOCATED",
};

#define TO_GPI_EV_STATE_STR(state) ((state >= MAX_EV_STATES) ? \
				    "INVALID" : gpi_ev_state_str[state])

enum gpi_ch_state {
	DEFAULT_CH_STATE = 0x0,
	CH_STATE_NOT_ALLOCATED = DEFAULT_CH_STATE,
	CH_STATE_ALLOCATED = 0x1,
	CH_STATE_STARTED = 0x2,
	CH_STATE_STOPPED = 0x3,
	CH_STATE_STOP_IN_PROC = 0x4,
	CH_STATE_ERROR = 0xf,
	MAX_CH_STATES
};

static const char *const gpi_ch_state_str[MAX_CH_STATES] = {
	[CH_STATE_NOT_ALLOCATED] = "NOT ALLOCATED",
	[CH_STATE_ALLOCATED] = "ALLOCATED",
	[CH_STATE_STARTED] = "STARTED",
	[CH_STATE_STOPPED] = "STOPPED",
	[CH_STATE_STOP_IN_PROC] = "STOP IN PROCESS",
	[CH_STATE_ERROR] = "ERROR",
};

#define TO_GPI_CH_STATE_STR(state) ((state >= MAX_CH_STATES) ? \
				    "INVALID" : gpi_ch_state_str[state])

enum gpi_cmd {
	GPI_CH_CMD_BEGIN,
	GPI_CH_CMD_ALLOCATE = GPI_CH_CMD_BEGIN,
	GPI_CH_CMD_START,
	GPI_CH_CMD_STOP,
	GPI_CH_CMD_RESET,
	GPI_CH_CMD_DE_ALLOC,
	GPI_CH_CMD_UART_SW_STALE,
	GPI_CH_CMD_UART_RFR_READY,
	GPI_CH_CMD_UART_RFR_NOT_READY,
	GPI_CH_CMD_END = GPI_CH_CMD_UART_RFR_NOT_READY,
	GPI_EV_CMD_BEGIN,
	GPI_EV_CMD_ALLOCATE = GPI_EV_CMD_BEGIN,
	GPI_EV_CMD_RESET,
	GPI_EV_CMD_DEALLOC,
	GPI_EV_CMD_END = GPI_EV_CMD_DEALLOC,
	GPI_MAX_CMD,
};

#define IS_CHAN_CMD(cmd) (cmd <= GPI_CH_CMD_END)

static const char *const gpi_cmd_str[GPI_MAX_CMD] = {
	[GPI_CH_CMD_ALLOCATE] = "CH ALLOCATE",
	[GPI_CH_CMD_START] = "CH START",
	[GPI_CH_CMD_STOP] = "CH STOP",
	[GPI_CH_CMD_RESET] = "CH_RESET",
	[GPI_CH_CMD_DE_ALLOC] = "DE ALLOC",
	[GPI_CH_CMD_UART_SW_STALE] = "UART SW STALE",
	[GPI_CH_CMD_UART_RFR_READY] = "UART RFR READY",
	[GPI_CH_CMD_UART_RFR_NOT_READY] = "UART RFR NOT READY",
	[GPI_EV_CMD_ALLOCATE] = "EV ALLOCATE",
	[GPI_EV_CMD_RESET] = "EV RESET",
	[GPI_EV_CMD_DEALLOC] = "EV DEALLOC",
};

#define TO_GPI_CMD_STR(cmd) ((cmd >= GPI_MAX_CMD) ? "INVALID" : \
			     gpi_cmd_str[cmd])

static const char *const gpi_cb_event_str[MSM_GPI_QUP_MAX_EVENT] = {
	[MSM_GPI_QUP_NOTIFY] = "NOTIFY",
	[MSM_GPI_QUP_ERROR] = "GLOBAL ERROR",
	[MSM_GPI_QUP_CH_ERROR] = "CHAN ERROR",
	[MSM_GPI_QUP_PENDING_EVENT] = "PENDING EVENT",
	[MSM_GPI_QUP_EOT_DESC_MISMATCH] = "EOT/DESC MISMATCH",
	[MSM_GPI_QUP_SW_ERROR] = "SW ERROR",
};

#define TO_GPI_CB_EVENT_STR(event) ((event >= MSM_GPI_QUP_MAX_EVENT) ? \
				    "INVALID" : gpi_cb_event_str[event])

enum se_protocol {
	SE_PROTOCOL_SPI = 1,
	SE_PROTOCOL_UART = 2,
	SE_PROTOCOL_I2C = 3,
	SE_MAX_PROTOCOL
};

/*
 * @DISABLE_STATE: no register access allowed
 * @CONFIG_STATE:  client has configured the channel
 * @PREP_HARDWARE: register access is allowed
 *		   however, no processing EVENTS
 * @ACTIVE_STATE: channels are fully operational
 * @PREPARE_TERIMNATE: graceful termination of channels
 *		       register access is allowed
 * @PAUSE_STATE: channels are active, but not processing any events
 */
enum gpi_pm_state {
	DISABLE_STATE,
	CONFIG_STATE,
	PREPARE_HARDWARE,
	ACTIVE_STATE,
	PREPARE_TERMINATE,
	PAUSE_STATE,
	MAX_PM_STATE
};

#define REG_ACCESS_VALID(pm_state) (pm_state >= PREPARE_HARDWARE)

static const char *const gpi_pm_state_str[MAX_PM_STATE] = {
	[DISABLE_STATE] = "DISABLE",
	[CONFIG_STATE] = "CONFIG",
	[PREPARE_HARDWARE] = "PREPARE HARDWARE",
	[ACTIVE_STATE] = "ACTIVE",
	[PREPARE_TERMINATE] = "PREPARE TERMINATE",
	[PAUSE_STATE] = "PAUSE",
};

#define TO_GPI_PM_STR(state) ((state >= MAX_PM_STATE) ? \
			      "INVALID" : gpi_pm_state_str[state])

static const struct {
	enum gpi_cmd gpi_cmd;
	u32 opcode;
	u32 state;
	u32 timeout_ms;
} gpi_cmd_info[GPI_MAX_CMD] = {
	{
		GPI_CH_CMD_ALLOCATE,
		GPI_GPII_n_CH_CMD_ALLOCATE,
		CH_STATE_ALLOCATED,
		CMD_TIMEOUT_MS,
	},
	{
		GPI_CH_CMD_START,
		GPI_GPII_n_CH_CMD_START,
		CH_STATE_STARTED,
		CMD_TIMEOUT_MS,
	},
	{
		GPI_CH_CMD_STOP,
		GPI_GPII_n_CH_CMD_STOP,
		CH_STATE_STOPPED,
		CMD_TIMEOUT_MS,
	},
	{
		GPI_CH_CMD_RESET,
		GPI_GPII_n_CH_CMD_RESET,
		CH_STATE_ALLOCATED,
		CMD_TIMEOUT_MS,
	},
	{
		GPI_CH_CMD_DE_ALLOC,
		GPI_GPII_n_CH_CMD_DE_ALLOC,
		CH_STATE_NOT_ALLOCATED,
		CMD_TIMEOUT_MS,
	},
	{
		GPI_CH_CMD_UART_SW_STALE,
		GPI_GPII_n_CH_CMD_UART_SW_STALE,
		STATE_IGNORE,
		CMD_TIMEOUT_MS,
	},
	{
		GPI_CH_CMD_UART_RFR_READY,
		GPI_GPII_n_CH_CMD_UART_RFR_READY,
		STATE_IGNORE,
		CMD_TIMEOUT_MS,
	},
	{
		GPI_CH_CMD_UART_RFR_NOT_READY,
		GPI_GPII_n_CH_CMD_UART_RFR_NOT_READY,
		STATE_IGNORE,
		CMD_TIMEOUT_MS,
	},
	{
		GPI_EV_CMD_ALLOCATE,
		GPI_GPII_n_EV_CH_CMD_ALLOCATE,
		EV_STATE_ALLOCATED,
		CMD_TIMEOUT_MS,
	},
	{
		GPI_EV_CMD_RESET,
		GPI_GPII_n_EV_CH_CMD_RESET,
		EV_STATE_ALLOCATED,
		CMD_TIMEOUT_MS,
	},
	{
		GPI_EV_CMD_DEALLOC,
		GPI_GPII_n_EV_CH_CMD_DE_ALLOC,
		EV_STATE_NOT_ALLOCATED,
		CMD_TIMEOUT_MS,
	},
};

struct gpi_ring {
	void *pre_aligned;
	size_t alloc_size;
	phys_addr_t phys_addr;
	dma_addr_t dma_handle;
	void *base;
	void *wp;
	void *rp;
	u32 len;
	u32 el_size;
	u32 elements;
	bool configured;
};

struct sg_tre {
	void *ptr;
	void *wp; /* store chan wp for debugging */
};

struct gpi_dbg_log {
	void *addr;
	u64 time;
	u32 val;
	bool read;
};

struct gpi_dev {
	struct dma_device dma_device;
	struct device *dev;
	struct resource *res;
	void __iomem *regs;
	u32 max_gpii; /* maximum # of gpii instances available per gpi block */
	u32 gpii_mask; /* gpii instances available for apps */
	u32 ev_factor; /* ev ring length factor */
	u32 smmu_cfg;
	dma_addr_t iova_base;
	size_t iova_size;
	struct gpii *gpiis;
	void *ilctxt;
	u32 ipc_log_lvl;
	u32 klog_lvl;
	struct dentry *dentry;
};

struct gpii_chan {
	struct virt_dma_chan vc;
	u32 chid;
	u32 seid;
	enum se_protocol protocol;
	enum EV_PRIORITY priority; /* comes from clients DT node */
	struct gpii *gpii;
	enum gpi_ch_state ch_state;
	enum gpi_pm_state pm_state;
	void __iomem *ch_cntxt_base_reg;
	void __iomem *ch_cntxt_db_reg;
	void __iomem *ch_ring_base_lsb_reg,
		*ch_ring_rp_lsb_reg,
		*ch_ring_wp_lsb_reg;
	void __iomem *ch_cmd_reg;
	u32 req_tres; /* # of tre's client requested */
	u32 dir;
	struct gpi_ring ch_ring;
	struct gpi_client_info client_info;
};

struct gpii {
	u32 gpii_id;
	struct gpii_chan gpii_chan[MAX_CHANNELS_PER_GPII];
	struct gpi_dev *gpi_dev;
	enum EV_PRIORITY ev_priority;
	enum se_protocol protocol;
	int irq;
	void __iomem *regs; /* points to gpi top */
	void __iomem *ev_cntxt_base_reg;
	void __iomem *ev_cntxt_db_reg;
	void __iomem *ev_ring_base_lsb_reg,
		*ev_ring_rp_lsb_reg,
		*ev_ring_wp_lsb_reg;
	void __iomem *ev_cmd_reg;
	void __iomem *ieob_src_reg;
	void __iomem *ieob_clr_reg;
	struct mutex ctrl_lock;
	enum gpi_ev_state ev_state;
	bool configured_irq;
	enum gpi_pm_state pm_state;
	rwlock_t pm_lock;
	struct gpi_ring ev_ring;
	struct tasklet_struct ev_task; /* event processing tasklet */
	struct completion cmd_completion;
	enum gpi_cmd gpi_cmd;
	u32 cntxt_type_irq_msk;
	void *ilctxt;
	u32 ipc_log_lvl;
	u32 klog_lvl;
	struct gpi_dbg_log dbg_log[GPI_DBG_LOG_SIZE];
	atomic_t dbg_index;
	char label[GPI_LABEL_SIZE];
	struct dentry *dentry;
};

struct gpi_desc {
	struct virt_dma_desc vd;
	void *wp; /* points to TRE last queued during issue_pending */
	void *db; /* DB register to program */
	struct gpii_chan *gpii_chan;
};

#define GPI_SMMU_ATTACH BIT(0)
#define GPI_SMMU_S1_BYPASS BIT(1)
#define GPI_SMMU_FAST BIT(2)
#define GPI_SMMU_ATOMIC BIT(3)

const u32 GPII_CHAN_DIR[MAX_CHANNELS_PER_GPII] = {
	GPI_CHTYPE_DIR_OUT, GPI_CHTYPE_DIR_IN
};

struct dentry *pdentry;
static irqreturn_t gpi_handle_irq(int irq, void *data);
static void gpi_ring_recycle_ev_element(struct gpi_ring *ring);
static int gpi_ring_add_element(struct gpi_ring *ring, void **wp);
static void gpi_process_events(struct gpii *gpii);

static inline struct gpii_chan *to_gpii_chan(struct dma_chan *dma_chan)
{
	return container_of(dma_chan, struct gpii_chan, vc.chan);
}

static inline struct gpi_desc *to_gpi_desc(struct virt_dma_desc *vd)
{
	return container_of(vd, struct gpi_desc, vd);
}

static inline phys_addr_t to_physical(const struct gpi_ring *const ring,
				      void *addr)
{
	return ring->phys_addr + (addr - ring->base);
}

static inline void *to_virtual(const struct gpi_ring *const ring,
				      phys_addr_t addr)
{
	return ring->base + (addr - ring->phys_addr);
}

#ifdef CONFIG_QCOM_GPI_DMA_DEBUG
static inline u32 gpi_read_reg(struct gpii *gpii, void __iomem *addr)
{
	u64 time = sched_clock();
	unsigned int index = atomic_inc_return(&gpii->dbg_index) - 1;
	u32 val;

	val = readl_relaxed(addr);
	index &= (GPI_DBG_LOG_SIZE - 1);
	(gpii->dbg_log + index)->addr = addr;
	(gpii->dbg_log + index)->time = time;
	(gpii->dbg_log + index)->val = val;
	(gpii->dbg_log + index)->read = true;
	GPII_REG(gpii, GPI_DBG_COMMON, "offset:0x%lx val:0x%x\n",
		 addr - gpii->regs, val);
	return val;
}
static inline void gpi_write_reg(struct gpii *gpii, void __iomem *addr, u32 val)
{
	u64 time = sched_clock();
	unsigned int index = atomic_inc_return(&gpii->dbg_index) - 1;

	index &= (GPI_DBG_LOG_SIZE - 1);
	(gpii->dbg_log + index)->addr = addr;
	(gpii->dbg_log + index)->time = time;
	(gpii->dbg_log + index)->val = val;
	(gpii->dbg_log + index)->read = false;

	GPII_REG(gpii, GPI_DBG_COMMON, "offset:0x%lx  val:0x%x\n",
		 addr - gpii->regs, val);
	writel_relaxed(val, addr);
}
#else
static inline u32 gpi_read_reg(struct gpii *gpii, void __iomem *addr)
{
	u32 val = readl_relaxed(addr);

	GPII_REG(gpii, GPI_DBG_COMMON, "offset:0x%lx val:0x%x\n",
		 addr - gpii->regs, val);
	return val;
}
static inline void gpi_write_reg(struct gpii *gpii, void __iomem *addr, u32 val)
{
	GPII_REG(gpii, GPI_DBG_COMMON, "offset:0x%lx  val:0x%x\n",
		 addr - gpii->regs, val);
	writel_relaxed(val, addr);
}
#endif

/* gpi_write_reg_field - write to specific bit field */
static inline void gpi_write_reg_field(struct gpii *gpii,
				       void __iomem *addr,
				       u32 mask,
				       u32 shift,
				       u32 val)
{
	u32 tmp = gpi_read_reg(gpii, addr);

	tmp &= ~mask;
	val = tmp | ((val << shift) & mask);
	gpi_write_reg(gpii, addr, val);
}

static void gpi_disable_interrupts(struct gpii *gpii)
{
	struct {
		u32 offset;
		u32 mask;
		u32 shift;
		u32 val;
	} default_reg[] = {
		{
			GPI_GPII_n_CNTXT_TYPE_IRQ_MSK_OFFS
			(gpii->gpii_id),
			GPI_GPII_n_CNTXT_TYPE_IRQ_MSK_BMSK,
			GPI_GPII_n_CNTXT_TYPE_IRQ_MSK_SHFT,
			0,
		},
		{
			GPI_GPII_n_CNTXT_SRC_IEOB_IRQ_MSK_OFFS
			(gpii->gpii_id),
			GPI_GPII_n_CNTXT_SRC_IEOB_IRQ_MSK_BMSK,
			GPI_GPII_n_CNTXT_SRC_IEOB_IRQ_MSK_SHFT,
			0,
		},
		{
			GPI_GPII_n_CNTXT_SRC_CH_IRQ_MSK_OFFS
			(gpii->gpii_id),
			GPI_GPII_n_CNTXT_SRC_CH_IRQ_MSK_BMSK,
			GPI_GPII_n_CNTXT_SRC_CH_IRQ_MSK_SHFT,
			0,
		},
		{
			GPI_GPII_n_CNTXT_SRC_EV_CH_IRQ_MSK_OFFS
			(gpii->gpii_id),
			GPI_GPII_n_CNTXT_SRC_EV_CH_IRQ_MSK_BMSK,
			GPI_GPII_n_CNTXT_SRC_EV_CH_IRQ_MSK_SHFT,
			0,
		},
		{
			GPI_GPII_n_CNTXT_GLOB_IRQ_EN_OFFS
			(gpii->gpii_id),
			GPI_GPII_n_CNTXT_GLOB_IRQ_EN_BMSK,
			GPI_GPII_n_CNTXT_GLOB_IRQ_EN_SHFT,
			0,
		},
		{
			GPI_GPII_n_CNTXT_GPII_IRQ_EN_OFFS
			(gpii->gpii_id),
			GPI_GPII_n_CNTXT_GPII_IRQ_EN_BMSK,
			GPI_GPII_n_CNTXT_GPII_IRQ_EN_SHFT,
			0,
		},
		{
			GPI_GPII_n_CNTXT_INTSET_OFFS
			(gpii->gpii_id),
			GPI_GPII_n_CNTXT_INTSET_BMSK,
			GPI_GPII_n_CNTXT_INTSET_SHFT,
			0,
		},
		{ 0 },
	};
	int i;

	for (i = 0; default_reg[i].offset; i++)
		gpi_write_reg_field(gpii, gpii->regs +
				    default_reg[i].offset,
				    default_reg[i].mask,
				    default_reg[i].shift,
				    default_reg[i].val);
	gpii->cntxt_type_irq_msk = 0;
	devm_free_irq(gpii->gpi_dev->dev, gpii->irq, gpii);
	gpii->configured_irq = false;
}

/* configure and enable interrupts */
static int gpi_config_interrupts(struct gpii *gpii,
				 enum gpii_irq_settings settings,
				 bool mask)
{
	int ret;
	int i;
	const u32 def_type = (GPI_GPII_n_CNTXT_TYPE_IRQ_MSK_GENERAL |
			      GPI_GPII_n_CNTXT_TYPE_IRQ_MSK_IEOB |
			      GPI_GPII_n_CNTXT_TYPE_IRQ_MSK_GLOB |
			      GPI_GPII_n_CNTXT_TYPE_IRQ_MSK_EV_CTRL |
			      GPI_GPII_n_CNTXT_TYPE_IRQ_MSK_CH_CTRL);
	struct {
		u32 offset;
		u32 mask;
		u32 shift;
		u32 val;
	} default_reg[] = {
		{
			GPI_GPII_n_CNTXT_TYPE_IRQ_MSK_OFFS
			(gpii->gpii_id),
			GPI_GPII_n_CNTXT_TYPE_IRQ_MSK_BMSK,
			GPI_GPII_n_CNTXT_TYPE_IRQ_MSK_SHFT,
			def_type,
		},
		{
			GPI_GPII_n_CNTXT_SRC_IEOB_IRQ_MSK_OFFS
			(gpii->gpii_id),
			GPI_GPII_n_CNTXT_SRC_IEOB_IRQ_MSK_BMSK,
			GPI_GPII_n_CNTXT_SRC_IEOB_IRQ_MSK_SHFT,
			GPI_GPII_n_CNTXT_SRC_IEOB_IRQ_MSK_BMSK,
		},
		{
			GPI_GPII_n_CNTXT_SRC_CH_IRQ_MSK_OFFS
			(gpii->gpii_id),
			GPI_GPII_n_CNTXT_SRC_CH_IRQ_MSK_BMSK,
			GPI_GPII_n_CNTXT_SRC_CH_IRQ_MSK_SHFT,
			GPI_GPII_n_CNTXT_SRC_CH_IRQ_MSK_BMSK,
		},
		{
			GPI_GPII_n_CNTXT_SRC_EV_CH_IRQ_MSK_OFFS
			(gpii->gpii_id),
			GPI_GPII_n_CNTXT_SRC_EV_CH_IRQ_MSK_BMSK,
			GPI_GPII_n_CNTXT_SRC_EV_CH_IRQ_MSK_SHFT,
			GPI_GPII_n_CNTXT_SRC_EV_CH_IRQ_MSK_BMSK,
		},
		{
			GPI_GPII_n_CNTXT_GLOB_IRQ_EN_OFFS
			(gpii->gpii_id),
			GPI_GPII_n_CNTXT_GLOB_IRQ_EN_BMSK,
			GPI_GPII_n_CNTXT_GLOB_IRQ_EN_SHFT,
			GPI_GPII_n_CNTXT_GLOB_IRQ_EN_ERROR_INT,
		},
		{
			GPI_GPII_n_CNTXT_GPII_IRQ_EN_OFFS
			(gpii->gpii_id),
			GPI_GPII_n_CNTXT_GPII_IRQ_EN_BMSK,
			GPI_GPII_n_CNTXT_GPII_IRQ_EN_SHFT,
			GPI_GPII_n_CNTXT_GPII_IRQ_EN_BMSK,
		},
		{
			GPI_GPII_n_CNTXT_MSI_BASE_LSB_OFFS
			(gpii->gpii_id),
			U32_MAX,
			0,
			0x0,
		},
		{
			GPI_GPII_n_CNTXT_MSI_BASE_MSB_OFFS
			(gpii->gpii_id),
			U32_MAX,
			0,
			0x0,
		},
		{
			GPI_GPII_n_CNTXT_SCRATCH_0_OFFS
			(gpii->gpii_id),
			U32_MAX,
			0,
			0x0,
		},
		{
			GPI_GPII_n_CNTXT_SCRATCH_1_OFFS
			(gpii->gpii_id),
			U32_MAX,
			0,
			0x0,
		},
		{
			GPI_GPII_n_CNTXT_INTSET_OFFS
			(gpii->gpii_id),
			GPI_GPII_n_CNTXT_INTSET_BMSK,
			GPI_GPII_n_CNTXT_INTSET_SHFT,
			0x01,
		},
		{
			GPI_GPII_n_ERROR_LOG_OFFS
			(gpii->gpii_id),
			U32_MAX,
			0,
			0x00,
		},
		{ 0 },
	};

	GPII_VERB(gpii, GPI_DBG_COMMON, "configured:%c setting:%s mask:%c\n",
		  (gpii->configured_irq) ? 'F' : 'T',
		  (settings == DEFAULT_IRQ_SETTINGS) ? "default" : "user_spec",
		  (mask) ? 'T' : 'F');

	if (gpii->configured_irq == false) {
		ret = devm_request_irq(gpii->gpi_dev->dev, gpii->irq,
				       gpi_handle_irq, IRQF_TRIGGER_HIGH,
				       gpii->label, gpii);
		if (ret < 0) {
			GPII_CRITIC(gpii, GPI_DBG_COMMON,
				    "error request irq:%d ret:%d\n",
				    gpii->irq, ret);
			return ret;
		}
	}

	if (settings == MASK_IEOB_SETTINGS) {
		/*
		 * GPII only uses one EV ring per gpii so we can globally
		 * enable/disable IEOB interrupt
		 */
		if (mask)
			gpii->cntxt_type_irq_msk |=
				GPI_GPII_n_CNTXT_TYPE_IRQ_MSK_IEOB;
		else
			gpii->cntxt_type_irq_msk &=
				~(GPI_GPII_n_CNTXT_TYPE_IRQ_MSK_IEOB);
		gpi_write_reg_field(gpii, gpii->regs +
			GPI_GPII_n_CNTXT_TYPE_IRQ_MSK_OFFS(gpii->gpii_id),
			GPI_GPII_n_CNTXT_TYPE_IRQ_MSK_BMSK,
			GPI_GPII_n_CNTXT_TYPE_IRQ_MSK_SHFT,
			gpii->cntxt_type_irq_msk);
	} else {
		for (i = 0; default_reg[i].offset; i++)
			gpi_write_reg_field(gpii, gpii->regs +
					    default_reg[i].offset,
					    default_reg[i].mask,
					    default_reg[i].shift,
					    default_reg[i].val);
		gpii->cntxt_type_irq_msk = def_type;
	};

	gpii->configured_irq = true;

	return 0;
}

/* Sends gpii event or channel command */
static int gpi_send_cmd(struct gpii *gpii,
			struct gpii_chan *gpii_chan,
			enum gpi_cmd gpi_cmd)
{
	u32 chid = MAX_CHANNELS_PER_GPII;
	u32 cmd;
	unsigned long timeout;
	void __iomem *cmd_reg;

	if (gpi_cmd >= GPI_MAX_CMD)
		return -EINVAL;
	if (IS_CHAN_CMD(gpi_cmd))
		chid = gpii_chan->chid;

	GPII_INFO(gpii, chid,
		  "sending cmd: %s\n", TO_GPI_CMD_STR(gpi_cmd));

	/* send opcode and wait for completion */
	reinit_completion(&gpii->cmd_completion);
	gpii->gpi_cmd = gpi_cmd;

	cmd_reg = IS_CHAN_CMD(gpi_cmd) ? gpii_chan->ch_cmd_reg :
		gpii->ev_cmd_reg;
	cmd = IS_CHAN_CMD(gpi_cmd) ?
		GPI_GPII_n_CH_CMD(gpi_cmd_info[gpi_cmd].opcode, chid) :
		GPI_GPII_n_EV_CH_CMD(gpi_cmd_info[gpi_cmd].opcode, 0);
	gpi_write_reg(gpii, cmd_reg, cmd);
	timeout = wait_for_completion_timeout(&gpii->cmd_completion,
			msecs_to_jiffies(gpi_cmd_info[gpi_cmd].timeout_ms));

	if (!timeout) {
		GPII_ERR(gpii, chid, "cmd: %s completion timeout\n",
			 TO_GPI_CMD_STR(gpi_cmd));
		return -EIO;
	}

	/* confirm new ch state is correct , if the cmd is a state change cmd */
	if (gpi_cmd_info[gpi_cmd].state == STATE_IGNORE)
		return 0;
	if (IS_CHAN_CMD(gpi_cmd) &&
	    gpii_chan->ch_state == gpi_cmd_info[gpi_cmd].state)
		return 0;
	if (!IS_CHAN_CMD(gpi_cmd) &&
	    gpii->ev_state == gpi_cmd_info[gpi_cmd].state)
		return 0;

	return -EIO;
}

/* program transfer ring DB register */
static inline void gpi_write_ch_db(struct gpii_chan *gpii_chan,
				   struct gpi_ring *ring,
				   void *wp)
{
	struct gpii *gpii = gpii_chan->gpii;
	phys_addr_t p_wp;

	p_wp = to_physical(ring, wp);
	gpi_write_reg(gpii, gpii_chan->ch_cntxt_db_reg, (u32)p_wp);
}

/* program event ring DB register */
static inline void gpi_write_ev_db(struct gpii *gpii,
				   struct gpi_ring *ring,
				   void *wp)
{
	phys_addr_t p_wp;

	p_wp = ring->phys_addr + (wp - ring->base);
	gpi_write_reg(gpii, gpii->ev_cntxt_db_reg, (u32)p_wp);
}

/* notify client with generic event */
static void gpi_generate_cb_event(struct gpii_chan *gpii_chan,
				  enum msm_gpi_cb_event event,
				  u64 status)
{
	struct gpii *gpii = gpii_chan->gpii;
	struct gpi_client_info *client_info = &gpii_chan->client_info;
	struct msm_gpi_cb msm_gpi_cb = {0};

	GPII_ERR(gpii, gpii_chan->chid,
		 "notifying event:%s with status:%llu\n",
		 TO_GPI_CB_EVENT_STR(event), status);

	msm_gpi_cb.cb_event = event;
	msm_gpi_cb.status = status;
	msm_gpi_cb.timestamp = sched_clock();
	client_info->callback(&gpii_chan->vc.chan, &msm_gpi_cb,
			      client_info->cb_param);
}

/* process transfer completion interrupt */
static void gpi_process_ieob(struct gpii *gpii)
{

	gpi_write_reg(gpii, gpii->ieob_clr_reg, BIT(0));

	/* process events based on priority */
	if (likely(gpii->ev_priority >= EV_PRIORITY_TASKLET)) {
		GPII_VERB(gpii, GPI_DBG_COMMON, "scheduling tasklet\n");
		gpi_config_interrupts(gpii, MASK_IEOB_SETTINGS, 0);
		tasklet_schedule(&gpii->ev_task);
	} else {
		GPII_VERB(gpii, GPI_DBG_COMMON, "processing events from isr\n");
		gpi_process_events(gpii);
	}
}

/* process channel control interrupt */
static void gpi_process_ch_ctrl_irq(struct gpii *gpii)
{
	u32 gpii_id = gpii->gpii_id;
	u32 offset = GPI_GPII_n_CNTXT_SRC_GPII_CH_IRQ_OFFS(gpii_id);
	u32 ch_irq = gpi_read_reg(gpii, gpii->regs + offset);
	u32 chid;
	struct gpii_chan *gpii_chan;
	u32 state;

	/* clear the status */
	offset = GPI_GPII_n_CNTXT_SRC_CH_IRQ_CLR_OFFS(gpii_id);
	gpi_write_reg(gpii, gpii->regs + offset, (u32)ch_irq);

	for (chid = 0; chid < MAX_CHANNELS_PER_GPII; chid++) {
		if (!(BIT(chid) & ch_irq))
			continue;

		gpii_chan = &gpii->gpii_chan[chid];
		GPII_VERB(gpii, chid, "processing channel ctrl irq\n");
		state = gpi_read_reg(gpii, gpii_chan->ch_cntxt_base_reg +
				     CNTXT_0_CONFIG);
		state = (state & GPI_GPII_n_CH_k_CNTXT_0_CHSTATE_BMSK) >>
			GPI_GPII_n_CH_k_CNTXT_0_CHSTATE_SHFT;

		/*
		 * CH_CMD_DEALLOC cmd always successful. However cmd does
		 * not change hardware status. So overwriting software state
		 * to default state.
		 */
		if (gpii->gpi_cmd == GPI_CH_CMD_DE_ALLOC)
			state = DEFAULT_CH_STATE;
		gpii_chan->ch_state = state;
		GPII_VERB(gpii, chid, "setting channel to state:%s\n",
			  TO_GPI_CH_STATE_STR(gpii_chan->ch_state));

		/*
		 * Triggering complete all if ch_state is not a stop in process.
		 * Stop in process is a transition state and we will wait for
		 * stop interrupt before notifying.
		 */
		if (gpii_chan->ch_state != CH_STATE_STOP_IN_PROC)
			complete_all(&gpii->cmd_completion);

		/* notifying clients if in error state */
		if (gpii_chan->ch_state == CH_STATE_ERROR)
			gpi_generate_cb_event(gpii_chan, MSM_GPI_QUP_CH_ERROR,
					      __LINE__);
	}
}

/* processing gpi level error interrupts */
static void gpi_process_glob_err_irq(struct gpii *gpii)
{
	u32 gpii_id = gpii->gpii_id;
	u32 offset = GPI_GPII_n_CNTXT_GLOB_IRQ_STTS_OFFS(gpii_id);
	u32 irq_stts = gpi_read_reg(gpii, gpii->regs + offset);
	u32 error_log;
	u32 chid;
	struct gpii_chan *gpii_chan;
	struct gpi_client_info *client_info;
	struct msm_gpi_cb msm_gpi_cb;
	struct gpi_error_log_entry *log_entry =
		(struct gpi_error_log_entry *)&error_log;

	offset = GPI_GPII_n_CNTXT_GLOB_IRQ_CLR_OFFS(gpii_id);
	gpi_write_reg(gpii, gpii->regs + offset, irq_stts);

	/* only error interrupt should be set */
	if (irq_stts & ~GPI_GLOB_IRQ_ERROR_INT_MSK) {
		GPII_ERR(gpii, GPI_DBG_COMMON, "invalid error status:0x%x\n",
			 irq_stts);
		goto error_irq;
	}

	offset = GPI_GPII_n_ERROR_LOG_OFFS(gpii_id);
	error_log = gpi_read_reg(gpii, gpii->regs + offset);
	gpi_write_reg(gpii, gpii->regs + offset, 0);

	/* get channel info */
	chid = ((struct gpi_error_log_entry *)&error_log)->chid;
	if (unlikely(chid >= MAX_CHANNELS_PER_GPII)) {
		GPII_ERR(gpii, GPI_DBG_COMMON, "invalid chid reported:%u\n",
			 chid);
		goto error_irq;
	}

	gpii_chan = &gpii->gpii_chan[chid];
	client_info = &gpii_chan->client_info;

	/* notify client with error log */
	msm_gpi_cb.cb_event = MSM_GPI_QUP_ERROR;
	msm_gpi_cb.error_log.routine = log_entry->routine;
	msm_gpi_cb.error_log.type = log_entry->type;
	msm_gpi_cb.error_log.error_code = log_entry->code;
	GPII_INFO(gpii, gpii_chan->chid, "sending CB event:%s\n",
		  TO_GPI_CB_EVENT_STR(msm_gpi_cb.cb_event));
	GPII_ERR(gpii, gpii_chan->chid,
		 "ee:%u chtype:%u routine:%u type:%u error_code:%u\n",
		 log_entry->ee, log_entry->chtype,
		 msm_gpi_cb.error_log.routine,
		 msm_gpi_cb.error_log.type,
		 msm_gpi_cb.error_log.error_code);
	client_info->callback(&gpii_chan->vc.chan, &msm_gpi_cb,
			      client_info->cb_param);

	return;

error_irq:
	for (chid = 0, gpii_chan = gpii->gpii_chan;
	     chid < MAX_CHANNELS_PER_GPII; chid++, gpii_chan++)
		gpi_generate_cb_event(gpii_chan, MSM_GPI_QUP_FW_ERROR,
				      irq_stts);
}

/* gpii interrupt handler */
static irqreturn_t gpi_handle_irq(int irq, void *data)
{
	struct gpii *gpii = data;
	u32 type;
	unsigned long flags;
	u32 offset;
	u32 gpii_id = gpii->gpii_id;

	GPII_VERB(gpii, GPI_DBG_COMMON, "enter\n");

	read_lock_irqsave(&gpii->pm_lock, flags);

	/*
	 * States are out of sync to receive interrupt
	 * while software state is in DISABLE state, bailing out.
	 */
	if (!REG_ACCESS_VALID(gpii->pm_state)) {
		GPII_CRITIC(gpii, GPI_DBG_COMMON,
			    "receive interrupt while in %s state\n",
			    TO_GPI_PM_STR(gpii->pm_state));
		goto exit_irq;
	}

	offset = GPI_GPII_n_CNTXT_TYPE_IRQ_OFFS(gpii->gpii_id);
	type = gpi_read_reg(gpii, gpii->regs + offset);

	do {
		GPII_VERB(gpii, GPI_DBG_COMMON, "CNTXT_TYPE_IRQ:0x%08x\n",
			  type);
		/* global gpii error */
		if (type & GPI_GPII_n_CNTXT_TYPE_IRQ_MSK_GLOB) {
			GPII_ERR(gpii, GPI_DBG_COMMON,
				 "processing global error irq\n");
			gpi_process_glob_err_irq(gpii);
			type &= ~(GPI_GPII_n_CNTXT_TYPE_IRQ_MSK_GLOB);
		}

		/* transfer complete interrupt */
		if (type & GPI_GPII_n_CNTXT_TYPE_IRQ_MSK_IEOB) {
			GPII_VERB(gpii, GPI_DBG_COMMON,
				  "process IEOB interrupts\n");
			gpi_process_ieob(gpii);
			type &= ~GPI_GPII_n_CNTXT_TYPE_IRQ_MSK_IEOB;
		}

		/* event control irq */
		if (type & GPI_GPII_n_CNTXT_TYPE_IRQ_MSK_EV_CTRL) {
			u32 ev_state;
			u32 ev_ch_irq;

			GPII_INFO(gpii, GPI_DBG_COMMON,
				  "processing EV CTRL interrupt\n");
			offset = GPI_GPII_n_CNTXT_SRC_EV_CH_IRQ_OFFS(gpii_id);
			ev_ch_irq = gpi_read_reg(gpii, gpii->regs + offset);

			offset = GPI_GPII_n_CNTXT_SRC_EV_CH_IRQ_CLR_OFFS
				(gpii_id);
			gpi_write_reg(gpii, gpii->regs + offset, ev_ch_irq);
			ev_state = gpi_read_reg(gpii, gpii->ev_cntxt_base_reg +
						CNTXT_0_CONFIG);
			ev_state &= GPI_GPII_n_EV_CH_k_CNTXT_0_CHSTATE_BMSK;
			ev_state >>= GPI_GPII_n_EV_CH_k_CNTXT_0_CHSTATE_SHFT;

			/*
			 * CMD EV_CMD_DEALLOC is always successful. However
			 * cmd does not change hardware status. So overwriting
			 * software state to default state.
			 */
			if (gpii->gpi_cmd == GPI_EV_CMD_DEALLOC)
				ev_state = DEFAULT_EV_CH_STATE;

			gpii->ev_state = ev_state;
			GPII_INFO(gpii, GPI_DBG_COMMON,
				  "setting EV state to %s\n",
				  TO_GPI_EV_STATE_STR(gpii->ev_state));
			complete_all(&gpii->cmd_completion);
			type &= ~(GPI_GPII_n_CNTXT_TYPE_IRQ_MSK_EV_CTRL);
		}

		/* channel control irq */
		if (type & GPI_GPII_n_CNTXT_TYPE_IRQ_MSK_CH_CTRL) {
			GPII_INFO(gpii, GPI_DBG_COMMON,
				  "process CH CTRL interrupts\n");
			gpi_process_ch_ctrl_irq(gpii);
			type &= ~(GPI_GPII_n_CNTXT_TYPE_IRQ_MSK_CH_CTRL);
		}

		if (type) {
			GPII_CRITIC(gpii, GPI_DBG_COMMON,
				 "Unhandled interrupt status:0x%x\n", type);
			goto exit_irq;
		}
		offset = GPI_GPII_n_CNTXT_TYPE_IRQ_OFFS(gpii->gpii_id);
		type = gpi_read_reg(gpii, gpii->regs + offset);
	} while (type);

exit_irq:
	read_unlock_irqrestore(&gpii->pm_lock, flags);
	GPII_VERB(gpii, GPI_DBG_COMMON, "exit\n");

	return IRQ_HANDLED;
}

/* process qup notification events */
static void gpi_process_qup_notif_event(struct gpii_chan *gpii_chan,
					struct qup_notif_event *notif_event)
{
	struct gpi_client_info *client_info = &gpii_chan->client_info;
	struct msm_gpi_cb msm_gpi_cb;

	GPII_VERB(gpii_chan->gpii, gpii_chan->chid,
		  "status:0x%x time:0x%x count:0x%x\n",
		  notif_event->status, notif_event->time, notif_event->count);

	msm_gpi_cb.cb_event = MSM_GPI_QUP_NOTIFY;
	msm_gpi_cb.status = notif_event->status;
	msm_gpi_cb.timestamp = notif_event->time;
	msm_gpi_cb.count = notif_event->count;
	GPII_VERB(gpii_chan->gpii, gpii_chan->chid, "sending CB event:%s\n",
		  TO_GPI_CB_EVENT_STR(msm_gpi_cb.cb_event));
	client_info->callback(&gpii_chan->vc.chan, &msm_gpi_cb,
			      client_info->cb_param);
}

/* process DMA Immediate completion data events */
static void gpi_process_imed_data_event(struct gpii_chan *gpii_chan,
					struct immediate_data_event *imed_event)
{
	struct gpii *gpii = gpii_chan->gpii;
	struct gpi_ring *ch_ring = &gpii_chan->ch_ring;
	struct virt_dma_desc *vd;
	struct gpi_desc *gpi_desc;
	void *tre = ch_ring->base +
		(ch_ring->el_size * imed_event->tre_index);
	struct msm_gpi_dma_async_tx_cb_param *tx_cb_param;
	unsigned long flags;

	/*
	 * If channel not active don't process event but let
	 * client know pending event is available
	 */
	if (gpii_chan->pm_state != ACTIVE_STATE) {
		GPII_ERR(gpii, gpii_chan->chid,
			 "skipping processing event because ch @ %s state\n",
			 TO_GPI_PM_STR(gpii_chan->pm_state));
		gpi_generate_cb_event(gpii_chan, MSM_GPI_QUP_PENDING_EVENT,
				      __LINE__);
		return;
	}

	spin_lock_irqsave(&gpii_chan->vc.lock, flags);
	vd = vchan_next_desc(&gpii_chan->vc);
	if (!vd) {
		struct gpi_ere *gpi_ere;
		struct msm_gpi_tre *gpi_tre;

		spin_unlock_irqrestore(&gpii_chan->vc.lock, flags);
		GPII_ERR(gpii, gpii_chan->chid,
			 "event without a pending descriptor!\n");
		gpi_ere = (struct gpi_ere *)imed_event;
		GPII_ERR(gpii, gpii_chan->chid, "Event: %08x %08x %08x %08x\n",
			 gpi_ere->dword[0], gpi_ere->dword[1],
			 gpi_ere->dword[2], gpi_ere->dword[3]);
		gpi_tre = tre;
		GPII_ERR(gpii, gpii_chan->chid,
			 "Pending TRE: %08x %08x %08x %08x\n",
			 gpi_tre->dword[0], gpi_tre->dword[1],
			 gpi_tre->dword[2], gpi_tre->dword[3]);
		gpi_generate_cb_event(gpii_chan, MSM_GPI_QUP_EOT_DESC_MISMATCH,
				      __LINE__);
		return;
	}
	gpi_desc = to_gpi_desc(vd);

	/* Event TR RP gen. don't match descriptor TR */
	if (gpi_desc->wp != tre) {
		spin_unlock_irqrestore(&gpii_chan->vc.lock, flags);
		GPII_ERR(gpii, gpii_chan->chid,
			 "EOT/EOB received for wrong TRE 0x%0llx != 0x%0llx\n",
			 to_physical(ch_ring, gpi_desc->wp),
			 to_physical(ch_ring, tre));
		gpi_generate_cb_event(gpii_chan, MSM_GPI_QUP_EOT_DESC_MISMATCH,
				      __LINE__);
		return;
	}

	list_del(&vd->node);
	spin_unlock_irqrestore(&gpii_chan->vc.lock, flags);


	/*
	 * RP pointed by Event is to last TRE processed,
	 * we need to update ring rp to tre + 1
	 */
	tre += ch_ring->el_size;
	if (tre >= (ch_ring->base + ch_ring->len))
		tre = ch_ring->base;
	ch_ring->rp = tre;

	/* make sure rp updates are immediately visible to all cores */
	smp_wmb();

	tx_cb_param = vd->tx.callback_param;
	if (vd->tx.callback && tx_cb_param) {
		struct msm_gpi_tre *imed_tre = &tx_cb_param->imed_tre;

		GPII_VERB(gpii, gpii_chan->chid,
			  "cb_length:%u compl_code:0x%x status:0x%x\n",
			  imed_event->length, imed_event->code,
			  imed_event->status);
		/* Update immediate data if any from event */
		*imed_tre = *((struct msm_gpi_tre *)imed_event);
		tx_cb_param->length = imed_event->length;
		tx_cb_param->completion_code = imed_event->code;
		tx_cb_param->status = imed_event->status;
		vd->tx.callback(tx_cb_param);
	}
	kfree(gpi_desc);
}

/* processing transfer completion events */
static void gpi_process_xfer_compl_event(struct gpii_chan *gpii_chan,
					 struct xfer_compl_event *compl_event)
{
	struct gpii *gpii = gpii_chan->gpii;
	struct gpi_ring *ch_ring = &gpii_chan->ch_ring;
	void *ev_rp = to_virtual(ch_ring, compl_event->ptr);
	struct virt_dma_desc *vd;
	struct msm_gpi_dma_async_tx_cb_param *tx_cb_param;
	struct gpi_desc *gpi_desc;
	unsigned long flags;

	/* only process events on active channel */
	if (unlikely(gpii_chan->pm_state != ACTIVE_STATE)) {
		GPII_ERR(gpii, gpii_chan->chid,
			 "skipping processing event because ch @ %s state\n",
			 TO_GPI_PM_STR(gpii_chan->pm_state));
		gpi_generate_cb_event(gpii_chan, MSM_GPI_QUP_PENDING_EVENT,
				      __LINE__);
		return;
	}

	spin_lock_irqsave(&gpii_chan->vc.lock, flags);
	vd = vchan_next_desc(&gpii_chan->vc);
	if (!vd) {
		struct gpi_ere *gpi_ere;

		spin_unlock_irqrestore(&gpii_chan->vc.lock, flags);
		GPII_ERR(gpii, gpii_chan->chid,
			 "Event without a pending descriptor!\n");
		gpi_ere = (struct gpi_ere *)compl_event;
		GPII_ERR(gpii, gpii_chan->chid, "Event: %08x %08x %08x %08x\n",
			 gpi_ere->dword[0], gpi_ere->dword[1],
			 gpi_ere->dword[2], gpi_ere->dword[3]);
		gpi_generate_cb_event(gpii_chan, MSM_GPI_QUP_EOT_DESC_MISMATCH,
				      __LINE__);
		return;
	}

	gpi_desc = to_gpi_desc(vd);

	/* TRE Event generated didn't match descriptor's TRE */
	if (gpi_desc->wp != ev_rp) {
		spin_unlock_irqrestore(&gpii_chan->vc.lock, flags);
		GPII_ERR(gpii, gpii_chan->chid,
			 "EOT\EOB received for wrong TRE 0x%0llx != 0x%0llx\n",
			 to_physical(ch_ring, gpi_desc->wp),
			 to_physical(ch_ring, ev_rp));
		gpi_generate_cb_event(gpii_chan, MSM_GPI_QUP_EOT_DESC_MISMATCH,
				      __LINE__);
		return;
	}

	list_del(&vd->node);
	spin_unlock_irqrestore(&gpii_chan->vc.lock, flags);


	/*
	 * RP pointed by Event is to last TRE processed,
	 * we need to update ring rp to ev_rp + 1
	 */
	ev_rp += ch_ring->el_size;
	if (ev_rp >= (ch_ring->base + ch_ring->len))
		ev_rp = ch_ring->base;
	ch_ring->rp = ev_rp;

	/* update must be visible to other cores */
	smp_wmb();

	tx_cb_param = vd->tx.callback_param;
	if (vd->tx.callback && tx_cb_param) {
		GPII_VERB(gpii, gpii_chan->chid,
			  "cb_length:%u compl_code:0x%x status:0x%x\n",
			  compl_event->length, compl_event->code,
			  compl_event->status);
		tx_cb_param->length = compl_event->length;
		tx_cb_param->completion_code = compl_event->code;
		tx_cb_param->status = compl_event->status;
		vd->tx.callback(tx_cb_param);
	}
	kfree(gpi_desc);
}

/* process all events */
static void gpi_process_events(struct gpii *gpii)
{
	struct gpi_ring *ev_ring = &gpii->ev_ring;
	phys_addr_t cntxt_rp, local_rp;
	void *rp;
	union gpi_event *gpi_event;
	struct gpii_chan *gpii_chan;
	u32 chid, type;

	cntxt_rp = gpi_read_reg(gpii, gpii->ev_ring_rp_lsb_reg);
	rp = to_virtual(ev_ring, cntxt_rp);
	local_rp = to_physical(ev_ring, ev_ring->rp);

	GPII_VERB(gpii, GPI_DBG_COMMON, "cntxt_rp:%pa local_rp:%pa\n",
		  &cntxt_rp, &local_rp);

	do {
		while (rp != ev_ring->rp) {
			gpi_event = ev_ring->rp;
			chid = gpi_event->xfer_compl_event.chid;
			type = gpi_event->xfer_compl_event.type;
			GPII_VERB(gpii, GPI_DBG_COMMON,
				  "chid:%u type:0x%x %08x %08x %08x %08x\n",
				  chid, type,
				  gpi_event->gpi_ere.dword[0],
				  gpi_event->gpi_ere.dword[1],
				  gpi_event->gpi_ere.dword[2],
				  gpi_event->gpi_ere.dword[3]);

			switch (type) {
			case XFER_COMPLETE_EV_TYPE:
				gpii_chan = &gpii->gpii_chan[chid];
				gpi_process_xfer_compl_event(gpii_chan,
						&gpi_event->xfer_compl_event);
				break;
			case STALE_EV_TYPE:
				GPII_VERB(gpii, GPI_DBG_COMMON,
					  "stale event, not processing\n");
				break;
			case IMMEDIATE_DATA_EV_TYPE:
				gpii_chan = &gpii->gpii_chan[chid];
				gpi_process_imed_data_event(gpii_chan,
					&gpi_event->immediate_data_event);
				break;
			case QUP_NOTIF_EV_TYPE:
				gpii_chan = &gpii->gpii_chan[chid];
				gpi_process_qup_notif_event(gpii_chan,
						&gpi_event->qup_notif_event);
				break;
			default:
				GPII_VERB(gpii, GPI_DBG_COMMON,
					  "not supported event type:0x%x\n",
					  type);
			}
			gpi_ring_recycle_ev_element(ev_ring);
		}
		gpi_write_ev_db(gpii, ev_ring, ev_ring->wp);

		/* clear pending IEOB events */
		gpi_write_reg(gpii, gpii->ieob_clr_reg, BIT(0));

		cntxt_rp = gpi_read_reg(gpii, gpii->ev_ring_rp_lsb_reg);
		rp = to_virtual(ev_ring, cntxt_rp);

	} while (rp != ev_ring->rp);

	GPII_VERB(gpii, GPI_DBG_COMMON, "exit: c_rp:%pa\n", &cntxt_rp);
}

/* processing events using tasklet */
static void gpi_ev_tasklet(unsigned long data)
{
	struct gpii *gpii = (struct gpii *)data;

	GPII_VERB(gpii, GPI_DBG_COMMON, "enter\n");

	read_lock_bh(&gpii->pm_lock);
	if (!REG_ACCESS_VALID(gpii->pm_state)) {
		read_unlock_bh(&gpii->pm_lock);
		GPII_ERR(gpii, GPI_DBG_COMMON,
			 "not processing any events, pm_state:%s\n",
			 TO_GPI_PM_STR(gpii->pm_state));
		return;
	}

	/* process the events */
	gpi_process_events(gpii);

	/* enable IEOB, switching back to interrupts */
	gpi_config_interrupts(gpii, MASK_IEOB_SETTINGS, 1);
	read_unlock_bh(&gpii->pm_lock);

	GPII_VERB(gpii, GPI_DBG_COMMON, "exit\n");
}

/* marks all pending events for the channel as stale */
void gpi_mark_stale_events(struct gpii_chan *gpii_chan)
{
	struct gpii *gpii = gpii_chan->gpii;
	struct gpi_ring *ev_ring = &gpii->ev_ring;
	void *ev_rp;
	u32 cntxt_rp, local_rp;

	GPII_INFO(gpii, gpii_chan->chid, "Enter\n");
	cntxt_rp = gpi_read_reg(gpii, gpii->ev_ring_rp_lsb_reg);

	ev_rp = ev_ring->rp;
	local_rp = (u32)to_physical(ev_ring, ev_rp);
	while (local_rp != cntxt_rp) {
		union gpi_event *gpi_event = ev_rp;
		u32 chid = gpi_event->xfer_compl_event.chid;

		if (chid == gpii_chan->chid)
			gpi_event->xfer_compl_event.type = STALE_EV_TYPE;
		ev_rp += ev_ring->el_size;
		if (ev_rp >= (ev_ring->base + ev_ring->len))
			ev_rp = ev_ring->base;
		cntxt_rp = gpi_read_reg(gpii, gpii->ev_ring_rp_lsb_reg);
		local_rp = (u32)to_physical(ev_ring, ev_rp);
	}
}

/* reset sw state and issue channel reset or de-alloc */
static int gpi_reset_chan(struct gpii_chan *gpii_chan, enum gpi_cmd gpi_cmd)
{
	struct gpii *gpii = gpii_chan->gpii;
	struct gpi_ring *ch_ring = &gpii_chan->ch_ring;
	unsigned long flags;
	LIST_HEAD(list);
	int ret;

	GPII_INFO(gpii, gpii_chan->chid, "Enter\n");
	ret = gpi_send_cmd(gpii, gpii_chan, gpi_cmd);
	if (ret) {
		GPII_ERR(gpii, gpii_chan->chid,
			 "Error with cmd:%s ret:%d\n",
			 TO_GPI_CMD_STR(gpi_cmd), ret);
		return ret;
	}

	/* initialize the local ring ptrs */
	ch_ring->rp = ch_ring->base;
	ch_ring->wp = ch_ring->base;

	/* visible to other cores */
	smp_wmb();

	/* check event ring for any stale events */
	write_lock_irq(&gpii->pm_lock);
	gpi_mark_stale_events(gpii_chan);

	/* remove all async descriptors */
	spin_lock_irqsave(&gpii_chan->vc.lock, flags);
	vchan_get_all_descriptors(&gpii_chan->vc, &list);
	spin_unlock_irqrestore(&gpii_chan->vc.lock, flags);
	write_unlock_irq(&gpii->pm_lock);
	vchan_dma_desc_free_list(&gpii_chan->vc, &list);

	return 0;
}

static int gpi_start_chan(struct gpii_chan *gpii_chan)
{
	struct gpii *gpii = gpii_chan->gpii;
	int ret;

	GPII_INFO(gpii, gpii_chan->chid, "Enter\n");

	ret = gpi_send_cmd(gpii, gpii_chan, GPI_CH_CMD_START);
	if (ret) {
		GPII_ERR(gpii, gpii_chan->chid,
			 "Error with cmd:%s ret:%d\n",
			 TO_GPI_CMD_STR(GPI_CH_CMD_START), ret);
		return ret;
	}

	/* gpii CH is active now */
	write_lock_irq(&gpii->pm_lock);
	gpii_chan->pm_state = ACTIVE_STATE;
	write_unlock_irq(&gpii->pm_lock);

	return 0;
}

/* allocate and configure the transfer channel */
static int gpi_alloc_chan(struct gpii_chan *gpii_chan, bool send_alloc_cmd)
{
	struct gpii *gpii = gpii_chan->gpii;
	struct gpi_ring *ring = &gpii_chan->ch_ring;
	int i;
	int ret;
	struct {
		void *base;
		int offset;
		u32 val;
	} ch_reg[] = {
		{
			gpii_chan->ch_cntxt_base_reg,
			CNTXT_0_CONFIG,
			GPI_GPII_n_CH_k_CNTXT_0(ring->el_size, 0,
						gpii_chan->dir,
						GPI_CHTYPE_PROTO_GPI),
		},
		{
			gpii_chan->ch_cntxt_base_reg,
			CNTXT_1_R_LENGTH,
			ring->len,
		},
		{
			gpii_chan->ch_cntxt_base_reg,
			CNTXT_2_RING_BASE_LSB,
			(u32)ring->phys_addr,
		},
		{
			gpii_chan->ch_cntxt_base_reg,
			CNTXT_3_RING_BASE_MSB,
			(u32)(ring->phys_addr >> 32),
		},
		{ /* program MSB of DB register with ring base */
			gpii_chan->ch_cntxt_db_reg,
			CNTXT_5_RING_RP_MSB - CNTXT_4_RING_RP_LSB,
			(u32)(ring->phys_addr >> 32),
		},
		{
			gpii->regs,
			GPI_GPII_n_CH_k_SCRATCH_0_OFFS(gpii->gpii_id,
						       gpii_chan->chid),
			GPI_GPII_n_CH_K_SCRATCH_0(!gpii_chan->chid,
						  gpii_chan->protocol,
						  gpii_chan->seid),
		},
		{
			gpii->regs,
			GPI_GPII_n_CH_k_SCRATCH_1_OFFS(gpii->gpii_id,
						       gpii_chan->chid),
			0,
		},
		{
			gpii->regs,
			GPI_GPII_n_CH_k_SCRATCH_2_OFFS(gpii->gpii_id,
						       gpii_chan->chid),
			0,
		},
		{
			gpii->regs,
			GPI_GPII_n_CH_k_SCRATCH_3_OFFS(gpii->gpii_id,
						       gpii_chan->chid),
			0,
		},
		{
			gpii->regs,
			GPI_GPII_n_CH_k_QOS_OFFS(gpii->gpii_id,
						 gpii_chan->chid),
			1,
		},
		{ NULL },
	};

	GPII_INFO(gpii, gpii_chan->chid, "Enter\n");

	if (send_alloc_cmd) {
		ret = gpi_send_cmd(gpii, gpii_chan, GPI_CH_CMD_ALLOCATE);
		if (ret) {
			GPII_ERR(gpii, gpii_chan->chid,
				 "Error with cmd:%s ret:%d\n",
				 TO_GPI_CMD_STR(GPI_CH_CMD_ALLOCATE), ret);
			return ret;
		}
	}

	/* program channel cntxt registers */
	for (i = 0; ch_reg[i].base; i++)
		gpi_write_reg(gpii, ch_reg[i].base + ch_reg[i].offset,
			      ch_reg[i].val);
	/* flush all the writes */
	wmb();
	return 0;
}

/* allocate and configure event ring */
static int gpi_alloc_ev_chan(struct gpii *gpii)
{
	struct gpi_ring *ring = &gpii->ev_ring;
	int i;
	int ret;
	struct {
		void *base;
		int offset;
		u32 val;
	} ev_reg[] = {
		{
			gpii->ev_cntxt_base_reg,
			CNTXT_0_CONFIG,
			GPI_GPII_n_EV_CH_k_CNTXT_0(ring->el_size,
						   GPI_INTTYPE_IRQ,
						   GPI_CHTYPE_GPI_EV),
		},
		{
			gpii->ev_cntxt_base_reg,
			CNTXT_1_R_LENGTH,
			ring->len,
		},
		{
			gpii->ev_cntxt_base_reg,
			CNTXT_2_RING_BASE_LSB,
			(u32)ring->phys_addr,
		},
		{
			gpii->ev_cntxt_base_reg,
			CNTXT_3_RING_BASE_MSB,
			(u32)(ring->phys_addr >> 32),
		},
		{
			/* program db msg with ring base msb */
			gpii->ev_cntxt_db_reg,
			CNTXT_5_RING_RP_MSB - CNTXT_4_RING_RP_LSB,
			(u32)(ring->phys_addr >> 32),
		},
		{
			gpii->ev_cntxt_base_reg,
			CNTXT_8_RING_INT_MOD,
			0,
		},
		{
			gpii->ev_cntxt_base_reg,
			CNTXT_10_RING_MSI_LSB,
			0,
		},
		{
			gpii->ev_cntxt_base_reg,
			CNTXT_11_RING_MSI_MSB,
			0,
		},
		{
			gpii->ev_cntxt_base_reg,
			CNTXT_8_RING_INT_MOD,
			0,
		},
		{
			gpii->ev_cntxt_base_reg,
			CNTXT_12_RING_RP_UPDATE_LSB,
			0,
		},
		{
			gpii->ev_cntxt_base_reg,
			CNTXT_13_RING_RP_UPDATE_MSB,
			0,
		},
		{ NULL },
	};

	GPII_INFO(gpii, GPI_DBG_COMMON, "enter\n");

	ret = gpi_send_cmd(gpii, NULL, GPI_EV_CMD_ALLOCATE);
	if (ret) {
		GPII_ERR(gpii, GPI_DBG_COMMON, "error with cmd:%s ret:%d\n",
			 TO_GPI_CMD_STR(GPI_EV_CMD_ALLOCATE), ret);
		return ret;
	}

	/* program event context */
	for (i = 0; ev_reg[i].base; i++)
		gpi_write_reg(gpii, ev_reg[i].base + ev_reg[i].offset,
			      ev_reg[i].val);

	/* add events to ring */
	ring->wp = (ring->base + ring->len - ring->el_size);

	/* flush all the writes */
	wmb();

	/* gpii is active now */
	write_lock_irq(&gpii->pm_lock);
	gpii->pm_state = ACTIVE_STATE;
	write_unlock_irq(&gpii->pm_lock);
	gpi_write_ev_db(gpii, ring, ring->wp);

	return 0;
}

/* calculate # of ERE/TRE available to queue */
static int gpi_ring_num_elements_avail(const struct gpi_ring * const ring)
{
	int elements = 0;

	if (ring->wp < ring->rp)
		elements = ((ring->rp - ring->wp) / ring->el_size) - 1;
	else {
		elements = (ring->rp - ring->base) / ring->el_size;
		elements += ((ring->base + ring->len - ring->wp) /
			     ring->el_size) - 1;
	}

	return elements;
}

static int gpi_ring_add_element(struct gpi_ring *ring, void **wp)
{

	if (gpi_ring_num_elements_avail(ring) <= 0)
		return -ENOMEM;

	*wp = ring->wp;
	ring->wp += ring->el_size;
	if (ring->wp  >= (ring->base + ring->len))
		ring->wp = ring->base;

	/* visible to other cores */
	smp_wmb();

	return 0;
}

static void gpi_ring_recycle_ev_element(struct gpi_ring *ring)
{
	/* Update the WP */
	ring->wp += ring->el_size;
	if (ring->wp  >= (ring->base + ring->len))
		ring->wp = ring->base;

	/* Update the RP */
	ring->rp += ring->el_size;
	if (ring->rp  >= (ring->base + ring->len))
		ring->rp = ring->base;

	/* visible to other cores */
	smp_wmb();
}

static void gpi_free_ring(struct gpi_ring *ring,
			  struct gpii *gpii)
{
	dma_free_coherent(gpii->gpi_dev->dev, ring->alloc_size,
			  ring->pre_aligned, ring->dma_handle);
	memset(ring, 0, sizeof(*ring));
}

/* allocate memory for transfer and event rings */
static int gpi_alloc_ring(struct gpi_ring *ring,
			  u32 elements,
			  u32 el_size,
			  struct gpii *gpii)
{
	u64 len = elements * el_size;
	int bit;

	/* ring len must be power of 2 */
	bit = find_last_bit((unsigned long *)&len, 32);
	if (((1 << bit) - 1) & len)
		bit++;
	len = 1 << bit;
	ring->alloc_size = (len + (len - 1));
	GPII_INFO(gpii, GPI_DBG_COMMON,
		  "#el:%u el_size:%u len:%u actual_len:%llu alloc_size:%lu\n",
		  elements, el_size, (elements * el_size), len,
		  ring->alloc_size);
	ring->pre_aligned = dma_alloc_coherent(gpii->gpi_dev->dev,
					       ring->alloc_size,
					       &ring->dma_handle, GFP_KERNEL);
	if (!ring->pre_aligned) {
		GPII_CRITIC(gpii, GPI_DBG_COMMON,
			    "could not alloc size:%lu mem for ring\n",
			    ring->alloc_size);
		return -ENOMEM;
	}

	/* align the physical mem */
	ring->phys_addr = (ring->dma_handle + (len - 1)) & ~(len - 1);
	ring->base = ring->pre_aligned + (ring->phys_addr - ring->dma_handle);
	ring->rp = ring->base;
	ring->wp = ring->base;
	ring->len = len;
	ring->el_size = el_size;
	ring->elements = ring->len / ring->el_size;
	memset(ring->base, 0, ring->len);
	ring->configured = true;

	/* update to other cores */
	smp_wmb();

	GPII_INFO(gpii, GPI_DBG_COMMON,
		  "phy_pre:0x%0llx phy_alig:0x%0llx len:%u el_size:%u elements:%u\n",
		  ring->dma_handle, ring->phys_addr, ring->len, ring->el_size,
		  ring->elements);

	return 0;
}

/* copy tre into transfer ring */
static void gpi_queue_xfer(struct gpii *gpii,
			   struct gpii_chan *gpii_chan,
			   struct msm_gpi_tre *gpi_tre,
			   void **wp)
{
	struct msm_gpi_tre *ch_tre;
	int ret;

	/* get next tre location we can copy */
	ret = gpi_ring_add_element(&gpii_chan->ch_ring, (void **)&ch_tre);
	if (unlikely(ret)) {
		GPII_CRITIC(gpii, gpii_chan->chid,
			    "Error adding ring element to xfer ring\n");
		return;
	}

	/* copy the tre info */
	memcpy(ch_tre, gpi_tre, sizeof(*ch_tre));
	*wp = ch_tre;
}

/* reset and restart transfer channel */
int gpi_terminate_all(struct dma_chan *chan)
{
	struct gpii_chan *gpii_chan = to_gpii_chan(chan);
	struct gpii *gpii = gpii_chan->gpii;
	int schid, echid, i;
	int ret = 0;

	GPII_INFO(gpii, gpii_chan->chid, "Enter\n");
	mutex_lock(&gpii->ctrl_lock);

	/*
	 * treat both channels as a group if its protocol is not UART
	 * STOP, RESET, or START needs to be in lockstep
	 */
	schid = (gpii->protocol == SE_PROTOCOL_UART) ? gpii_chan->chid : 0;
	echid = (gpii->protocol == SE_PROTOCOL_UART) ? schid + 1 :
		MAX_CHANNELS_PER_GPII;

	/* stop the channel */
	for (i = schid; i < echid; i++) {
		gpii_chan = &gpii->gpii_chan[i];

		/* disable ch state so no more TRE processing */
		write_lock_irq(&gpii->pm_lock);
		gpii_chan->pm_state = PREPARE_TERMINATE;
		write_unlock_irq(&gpii->pm_lock);

		/* send command to Stop the channel */
		ret = gpi_send_cmd(gpii, gpii_chan, GPI_CH_CMD_STOP);
		if (ret)
			GPII_ERR(gpii, gpii_chan->chid,
				 "Error Stopping Channel:%d resetting anyway\n",
				 ret);
	}

	/* reset the channels (clears any pending tre) */
	for (i = schid; i < echid; i++) {
		gpii_chan = &gpii->gpii_chan[i];

		ret = gpi_reset_chan(gpii_chan, GPI_CH_CMD_RESET);
		if (ret) {
			GPII_ERR(gpii, gpii_chan->chid,
				 "Error resetting channel ret:%d\n", ret);
			goto terminate_exit;
		}

		/* reprogram channel CNTXT */
		ret = gpi_alloc_chan(gpii_chan, false);
		if (ret) {
			GPII_ERR(gpii, gpii_chan->chid,
				 "Error alloc_channel ret:%d\n", ret);
			goto terminate_exit;
		}
	}

	/* restart the channels */
	for (i = schid; i < echid; i++) {
		gpii_chan = &gpii->gpii_chan[i];

		ret = gpi_start_chan(gpii_chan);
		if (ret) {
			GPII_ERR(gpii, gpii_chan->chid,
				 "Error Starting Channel ret:%d\n", ret);
			goto terminate_exit;
		}
	}

terminate_exit:
	mutex_unlock(&gpii->ctrl_lock);
	return ret;
}

/* pause dma transfer for all channels */
static int gpi_pause(struct dma_chan *chan)
{
	struct gpii_chan *gpii_chan = to_gpii_chan(chan);
	struct gpii *gpii = gpii_chan->gpii;
	int i, ret;

	GPII_INFO(gpii, gpii_chan->chid, "Enter\n");
	mutex_lock(&gpii->ctrl_lock);

	/*
	 * pause/resume are per gpii not per channel, so
	 * client needs to call pause only once
	 */
	if (gpii->pm_state == PAUSE_STATE) {
		GPII_INFO(gpii, gpii_chan->chid,
			  "channel is already paused\n");
		mutex_unlock(&gpii->ctrl_lock);
		return 0;
	}

	/* send stop command to stop the channels */
	for (i = 0; i < MAX_CHANNELS_PER_GPII; i++) {
		ret = gpi_send_cmd(gpii, &gpii->gpii_chan[i], GPI_CH_CMD_STOP);
		if (ret) {
			GPII_ERR(gpii, gpii->gpii_chan[i].chid,
				 "Error stopping chan, ret:%d\n", ret);
			mutex_unlock(&gpii->ctrl_lock);
			return ret;
		}
	}

	disable_irq(gpii->irq);

	/* Wait for threads to complete out */
	tasklet_kill(&gpii->ev_task);

	write_lock_irq(&gpii->pm_lock);
	gpii->pm_state = PAUSE_STATE;
	write_unlock_irq(&gpii->pm_lock);
	mutex_unlock(&gpii->ctrl_lock);

	return 0;
}

/* resume dma transfer */
static int gpi_resume(struct dma_chan *chan)
{
	struct gpii_chan *gpii_chan = to_gpii_chan(chan);
	struct gpii *gpii = gpii_chan->gpii;
	int i;
	int ret;

	GPII_INFO(gpii, gpii_chan->chid, "enter\n");

	mutex_lock(&gpii->ctrl_lock);
	if (gpii->pm_state == ACTIVE_STATE) {
		GPII_INFO(gpii, gpii_chan->chid,
			  "channel is already active\n");
		mutex_unlock(&gpii->ctrl_lock);
		return 0;
	}

	enable_irq(gpii->irq);

	/* send start command to start the channels */
	for (i = 0; i < MAX_CHANNELS_PER_GPII; i++) {
		ret = gpi_send_cmd(gpii, &gpii->gpii_chan[i], GPI_CH_CMD_START);
		if (ret) {
			GPII_ERR(gpii, gpii->gpii_chan[i].chid,
				 "Erro starting chan, ret:%d\n", ret);
			mutex_unlock(&gpii->ctrl_lock);
			return ret;
		}
	}

	write_lock_irq(&gpii->pm_lock);
	gpii->pm_state = ACTIVE_STATE;
	write_unlock_irq(&gpii->pm_lock);
	mutex_unlock(&gpii->ctrl_lock);

	return 0;
}

void gpi_desc_free(struct virt_dma_desc *vd)
{
	struct gpi_desc *gpi_desc = to_gpi_desc(vd);

	kfree(gpi_desc);
}

/* copy tre into transfer ring */
struct dma_async_tx_descriptor *gpi_prep_slave_sg(struct dma_chan *chan,
					struct scatterlist *sgl,
					unsigned int sg_len,
					enum dma_transfer_direction direction,
					unsigned long flags,
					void *context)
{
	struct gpii_chan *gpii_chan = to_gpii_chan(chan);
	struct gpii *gpii = gpii_chan->gpii;
	u32 nr;
	u32 nr_req = 0;
	int i, j;
	struct scatterlist *sg;
	struct gpi_ring *ch_ring = &gpii_chan->ch_ring;
	void *tre, *wp = NULL;
	const gfp_t gfp = GFP_ATOMIC;
	struct gpi_desc *gpi_desc;

	GPII_VERB(gpii, gpii_chan->chid, "enter\n");

	if (!is_slave_direction(direction)) {
		GPII_ERR(gpii, gpii_chan->chid,
			 "invalid dma direction: %d\n", direction);
		return NULL;
	}

	/* calculate # of elements required & available */
	nr = gpi_ring_num_elements_avail(ch_ring);
	for_each_sg(sgl, sg, sg_len, i) {
		GPII_VERB(gpii, gpii_chan->chid,
			  "%d of %u len:%u\n", i, sg_len, sg->length);
		nr_req += (sg->length / ch_ring->el_size);
	}
	GPII_VERB(gpii, gpii_chan->chid, "el avail:%u req:%u\n", nr, nr_req);

	if (nr < nr_req) {
		GPII_ERR(gpii, gpii_chan->chid,
			 "not enough space in ring, avail:%u required:%u\n",
			 nr, nr_req);
		return NULL;
	}

	gpi_desc = kzalloc(sizeof(*gpi_desc), gfp);
	if (!gpi_desc) {
		GPII_ERR(gpii, gpii_chan->chid,
			 "out of memory for descriptor\n");
		return NULL;
	}

	/* copy each tre into transfer ring */
	for_each_sg(sgl, sg, sg_len, i)
		for (j = 0, tre = sg_virt(sg); j < sg->length;
		     j += ch_ring->el_size, tre += ch_ring->el_size)
			gpi_queue_xfer(gpii, gpii_chan, tre, &wp);

	/* set up the descriptor */
	gpi_desc->db = ch_ring->wp;
	gpi_desc->wp = wp;
	gpi_desc->gpii_chan = gpii_chan;
	GPII_VERB(gpii, gpii_chan->chid, "exit wp:0x%0llx rp:0x%0llx\n",
		  to_physical(ch_ring, ch_ring->wp),
		  to_physical(ch_ring, ch_ring->rp));

	return vchan_tx_prep(&gpii_chan->vc, &gpi_desc->vd, flags);
}

/* rings transfer ring db to being transfer */
static void gpi_issue_pending(struct dma_chan *chan)
{
	struct gpii_chan *gpii_chan = to_gpii_chan(chan);
	struct gpii *gpii = gpii_chan->gpii;
	unsigned long flags, pm_lock_flags;
	struct virt_dma_desc *vd = NULL;
	struct gpi_desc *gpi_desc;

	GPII_VERB(gpii, gpii_chan->chid, "Enter\n");

	read_lock_irqsave(&gpii->pm_lock, pm_lock_flags);

	/* move all submitted discriptors to issued list */
	spin_lock_irqsave(&gpii_chan->vc.lock, flags);
	if (vchan_issue_pending(&gpii_chan->vc))
		vd = list_last_entry(&gpii_chan->vc.desc_issued,
				     struct virt_dma_desc, node);
	spin_unlock_irqrestore(&gpii_chan->vc.lock, flags);

	/* nothing to do list is empty */
	if (!vd) {
		read_unlock_irqrestore(&gpii->pm_lock, pm_lock_flags);
		GPII_VERB(gpii, gpii_chan->chid, "no descriptors submitted\n");
		return;
	}

	gpi_desc = to_gpi_desc(vd);
	gpi_write_ch_db(gpii_chan, &gpii_chan->ch_ring, gpi_desc->db);
	read_unlock_irqrestore(&gpii->pm_lock, pm_lock_flags);
}

/* configure or issue async command */
static int gpi_config(struct dma_chan *chan,
		      struct dma_slave_config *config)
{
	struct gpii_chan *gpii_chan = to_gpii_chan(chan);
	struct gpii *gpii = gpii_chan->gpii;
	struct msm_gpi_ctrl *gpi_ctrl = chan->private;
	const int ev_factor = gpii->gpi_dev->ev_factor;
	u32 elements;
	int i = 0;
	int ret = 0;

	GPII_INFO(gpii, gpii_chan->chid, "enter\n");
	if (!gpi_ctrl) {
		GPII_ERR(gpii, gpii_chan->chid,
			 "no config ctrl data provided");
		return -EINVAL;
	}

	mutex_lock(&gpii->ctrl_lock);

	switch (gpi_ctrl->cmd) {
	case MSM_GPI_INIT:
		GPII_INFO(gpii, gpii_chan->chid, "cmd: msm_gpi_init\n");

		gpii_chan->client_info.callback = gpi_ctrl->init.callback;
		gpii_chan->client_info.cb_param = gpi_ctrl->init.cb_param;
		gpii_chan->pm_state = CONFIG_STATE;

		/* check if both channels are configured before continue */
		for (i = 0; i < MAX_CHANNELS_PER_GPII; i++)
			if (gpii->gpii_chan[i].pm_state != CONFIG_STATE)
				goto exit_gpi_init;

		/* configure to highest priority from  two channels */
		gpii->ev_priority = min(gpii->gpii_chan[0].priority,
					gpii->gpii_chan[1].priority);

		/* protocol must be same for both channels */
		if (gpii->gpii_chan[0].protocol !=
		    gpii->gpii_chan[1].protocol) {
			GPII_ERR(gpii, gpii_chan->chid,
				 "protocol did not match protocol %u != %u\n",
				 gpii->gpii_chan[0].protocol,
				 gpii->gpii_chan[1].protocol);
			ret = -EINVAL;
			goto exit_gpi_init;
		}
		gpii->protocol = gpii_chan->protocol;

		/* allocate memory for event ring */
		elements = max(gpii->gpii_chan[0].req_tres,
			       gpii->gpii_chan[1].req_tres);
		ret = gpi_alloc_ring(&gpii->ev_ring, elements << ev_factor,
				     sizeof(union gpi_event), gpii);
		if (ret) {
			GPII_ERR(gpii, gpii_chan->chid,
				 "error allocating mem for ev ring\n");
			goto exit_gpi_init;
		}

		/* configure interrupts */
		write_lock_irq(&gpii->pm_lock);
		gpii->pm_state = PREPARE_HARDWARE;
		write_unlock_irq(&gpii->pm_lock);
		ret = gpi_config_interrupts(gpii, DEFAULT_IRQ_SETTINGS, 0);
		if (ret) {
			GPII_ERR(gpii, gpii_chan->chid,
				 "error config. interrupts, ret:%d\n", ret);
			goto error_config_int;
		}

		/* allocate event rings */
		ret = gpi_alloc_ev_chan(gpii);
		if (ret) {
			GPII_ERR(gpii, gpii_chan->chid,
				 "error alloc_ev_chan:%d\n", ret);
				goto error_alloc_ev_ring;
		}

		/* Allocate all channels */
		for (i = 0; i < MAX_CHANNELS_PER_GPII; i++) {
			ret = gpi_alloc_chan(&gpii->gpii_chan[i], true);
			if (ret) {
				GPII_ERR(gpii, gpii->gpii_chan[i].chid,
					 "Error allocating chan:%d\n", ret);
				goto error_alloc_chan;
			}
		}

		/* start channels  */
		for (i = 0; i < MAX_CHANNELS_PER_GPII; i++) {
			ret = gpi_start_chan(&gpii->gpii_chan[i]);
			if (ret) {
				GPII_ERR(gpii, gpii->gpii_chan[i].chid,
					 "Error start chan:%d\n", ret);
				goto error_start_chan;
			}
		}

		break;
	case MSM_GPI_CMD_UART_SW_STALE:
		GPII_INFO(gpii, gpii_chan->chid, "sending UART SW STALE cmd\n");
		ret = gpi_send_cmd(gpii, gpii_chan, GPI_CH_CMD_UART_SW_STALE);
		break;
	case MSM_GPI_CMD_UART_RFR_READY:
		GPII_INFO(gpii, gpii_chan->chid,
			  "sending UART RFR READY cmd\n");
		ret = gpi_send_cmd(gpii, gpii_chan, GPI_CH_CMD_UART_RFR_READY);
		break;
	case MSM_GPI_CMD_UART_RFR_NOT_READY:
		GPII_INFO(gpii, gpii_chan->chid,
			  "sending UART RFR READY NOT READY cmd\n");
		ret = gpi_send_cmd(gpii, gpii_chan,
				   GPI_CH_CMD_UART_RFR_NOT_READY);
		break;
	default:
		GPII_ERR(gpii, gpii_chan->chid,
			 "unsupported ctrl cmd:%d\n", gpi_ctrl->cmd);
		ret = -EINVAL;
	}

	mutex_unlock(&gpii->ctrl_lock);
	return ret;

error_start_chan:
	for (i = i - 1; i >= 0; i++) {
		gpi_send_cmd(gpii, gpii_chan, GPI_CH_CMD_STOP);
		gpi_send_cmd(gpii, gpii_chan, GPI_CH_CMD_RESET);
	}
	i = 2;
error_alloc_chan:
	for (i = i - 1; i >= 0; i--)
		gpi_reset_chan(gpii_chan, GPI_CH_CMD_DE_ALLOC);
error_alloc_ev_ring:
	gpi_disable_interrupts(gpii);
error_config_int:
	gpi_free_ring(&gpii->ev_ring, gpii);
exit_gpi_init:
	mutex_unlock(&gpii->ctrl_lock);
	return ret;
}

/* release all channel resources */
static void gpi_free_chan_resources(struct dma_chan *chan)
{
	struct gpii_chan *gpii_chan = to_gpii_chan(chan);
	struct gpii *gpii = gpii_chan->gpii;
	enum gpi_pm_state cur_state;
	int ret, i;

	GPII_INFO(gpii, gpii_chan->chid, "enter\n");

	mutex_lock(&gpii->ctrl_lock);

	cur_state = gpii_chan->pm_state;

	/* disable ch state so no more TRE processing for this channel */
	write_lock_irq(&gpii->pm_lock);
	gpii_chan->pm_state = PREPARE_TERMINATE;
	write_unlock_irq(&gpii->pm_lock);

	/* attemp to do graceful hardware shutdown */
	if (cur_state == ACTIVE_STATE) {
		ret = gpi_send_cmd(gpii, gpii_chan, GPI_CH_CMD_STOP);
		if (ret)
			GPII_ERR(gpii, gpii_chan->chid,
				 "error stopping channel:%d\n", ret);

		ret = gpi_send_cmd(gpii, gpii_chan, GPI_CH_CMD_RESET);
		if (ret)
			GPII_ERR(gpii, gpii_chan->chid,
				 "error resetting channel:%d\n", ret);

		gpi_reset_chan(gpii_chan, GPI_CH_CMD_DE_ALLOC);
	}

	/* free all allocated memory */
	gpi_free_ring(&gpii_chan->ch_ring, gpii);
	vchan_free_chan_resources(&gpii_chan->vc);

	write_lock_irq(&gpii->pm_lock);
	gpii_chan->pm_state = DISABLE_STATE;
	write_unlock_irq(&gpii->pm_lock);

	/* if other rings are still active exit */
	for (i = 0; i < MAX_CHANNELS_PER_GPII; i++)
		if (gpii->gpii_chan[i].ch_ring.configured)
			goto exit_free;

	GPII_INFO(gpii, gpii_chan->chid, "disabling gpii\n");

	/* deallocate EV Ring */
	cur_state = gpii->pm_state;
	write_lock_irq(&gpii->pm_lock);
	gpii->pm_state = PREPARE_TERMINATE;
	write_unlock_irq(&gpii->pm_lock);

	/* wait for threads to complete out */
	tasklet_kill(&gpii->ev_task);

	/* send command to de allocate event ring */
	if (cur_state == ACTIVE_STATE)
		gpi_send_cmd(gpii, NULL, GPI_EV_CMD_DEALLOC);

	gpi_free_ring(&gpii->ev_ring, gpii);

	/* disable interrupts */
	if (cur_state == ACTIVE_STATE)
		gpi_disable_interrupts(gpii);

	/* set final state to disable */
	write_lock_irq(&gpii->pm_lock);
	gpii->pm_state = DISABLE_STATE;
	write_unlock_irq(&gpii->pm_lock);

exit_free:
	mutex_unlock(&gpii->ctrl_lock);
}

/* allocate channel resources */
static int gpi_alloc_chan_resources(struct dma_chan *chan)
{
	struct gpii_chan *gpii_chan = to_gpii_chan(chan);
	struct gpii *gpii = gpii_chan->gpii;
	int ret;

	GPII_INFO(gpii, gpii_chan->chid, "enter\n");

	mutex_lock(&gpii->ctrl_lock);

	/* allocate memory for transfer ring */
	ret = gpi_alloc_ring(&gpii_chan->ch_ring, gpii_chan->req_tres,
			     sizeof(struct msm_gpi_tre), gpii);
	if (ret) {
		GPII_ERR(gpii, gpii_chan->chid,
			 "error allocating xfer ring, ret:%d\n", ret);
		goto xfer_alloc_err;
	}
	mutex_unlock(&gpii->ctrl_lock);

	return 0;
xfer_alloc_err:
	mutex_unlock(&gpii->ctrl_lock);

	return ret;
}

static int gpi_find_avail_gpii(struct gpi_dev *gpi_dev, u32 seid)
{
	int gpii;
	struct gpii_chan *tx_chan, *rx_chan;

	/* check if same seid is already configured for another chid */
	for (gpii = 0; gpii < gpi_dev->max_gpii; gpii++) {
		if (!((1 << gpii) & gpi_dev->gpii_mask))
			continue;

		tx_chan = &gpi_dev->gpiis[gpii].gpii_chan[GPI_TX_CHAN];
		rx_chan = &gpi_dev->gpiis[gpii].gpii_chan[GPI_RX_CHAN];

		if (rx_chan->vc.chan.client_count && rx_chan->seid == seid)
			return gpii;
		if (tx_chan->vc.chan.client_count && tx_chan->seid == seid)
			return gpii;
	}

	/* no channels configured with same seid, return next avail gpii */
	for (gpii = 0; gpii < gpi_dev->max_gpii; gpii++) {
		if (!((1 << gpii) & gpi_dev->gpii_mask))
			continue;

		tx_chan = &gpi_dev->gpiis[gpii].gpii_chan[GPI_TX_CHAN];
		rx_chan = &gpi_dev->gpiis[gpii].gpii_chan[GPI_RX_CHAN];

		/* check if gpii is configured */
		if (tx_chan->vc.chan.client_count ||
		    rx_chan->vc.chan.client_count)
			continue;

		/* found a free gpii */
		return gpii;
	}

	/* no gpii instance available to use */
	return -EIO;
}

/* gpi_of_dma_xlate: open client requested channel */
static struct dma_chan *gpi_of_dma_xlate(struct of_phandle_args *args,
					 struct of_dma *of_dma)
{
	struct gpi_dev *gpi_dev = (struct gpi_dev *)of_dma->of_dma_data;
	u32 seid, chid;
	int gpii;
	struct gpii_chan *gpii_chan;

	if (args->args_count < REQ_OF_DMA_ARGS) {
		GPI_ERR(gpi_dev,
			"gpii require minimum 6 args, client passed:%d args\n",
			args->args_count);
		return NULL;
	}

	chid = args->args[0];
	if (chid >= MAX_CHANNELS_PER_GPII) {
		GPI_ERR(gpi_dev, "gpii channel:%d not valid\n", chid);
		return NULL;
	}

	seid = args->args[1];

	/* find next available gpii to use */
	gpii = gpi_find_avail_gpii(gpi_dev, seid);
	if (gpii < 0) {
		GPI_ERR(gpi_dev, "no available gpii instances\n");
		return NULL;
	}

	gpii_chan = &gpi_dev->gpiis[gpii].gpii_chan[chid];
	if (gpii_chan->vc.chan.client_count) {
		GPI_ERR(gpi_dev, "gpii:%d chid:%d seid:%d already configured\n",
			gpii, chid, gpii_chan->seid);
		return NULL;
	}

	/* get ring size, protocol, se_id, and priority */
	gpii_chan->seid = seid;
	gpii_chan->protocol = args->args[2];
	gpii_chan->req_tres = args->args[3];
	gpii_chan->priority = args->args[4];

	GPI_LOG(gpi_dev,
		"client req. gpii:%u chid:%u #_tre:%u priority:%u protocol:%u\n",
		gpii, chid, gpii_chan->req_tres, gpii_chan->priority,
		gpii_chan->protocol);

	return dma_get_slave_channel(&gpii_chan->vc.chan);
}

/* gpi_setup_debug - setup debug capabilities */
static void gpi_setup_debug(struct gpi_dev *gpi_dev)
{
	char node_name[GPI_LABEL_SIZE];
	const umode_t mode = 0600;
	int i;

	snprintf(node_name, sizeof(node_name), "%s%llx", GPI_DMA_DRV_NAME,
		 (u64)gpi_dev->res->start);

	gpi_dev->ilctxt = ipc_log_context_create(IPC_LOG_PAGES,
						 node_name, 0);
	gpi_dev->ipc_log_lvl = DEFAULT_IPC_LOG_LVL;
	if (!IS_ERR_OR_NULL(pdentry)) {
		snprintf(node_name, sizeof(node_name), "%llx",
			 (u64)gpi_dev->res->start);
		gpi_dev->dentry = debugfs_create_dir(node_name, pdentry);
		if (!IS_ERR_OR_NULL(gpi_dev->dentry)) {
			debugfs_create_u32("ipc_log_lvl", mode, gpi_dev->dentry,
					   &gpi_dev->ipc_log_lvl);
			debugfs_create_u32("klog_lvl", mode,
					   gpi_dev->dentry, &gpi_dev->klog_lvl);
		}
	}

	for (i = 0; i < gpi_dev->max_gpii; i++) {
		struct gpii *gpii;

		if (!((1 << i) & gpi_dev->gpii_mask))
			continue;

		gpii = &gpi_dev->gpiis[i];
		snprintf(gpii->label, sizeof(gpii->label),
			 "%s%llx_gpii%d",
			 GPI_DMA_DRV_NAME, (u64)gpi_dev->res->start, i);
		gpii->ilctxt = ipc_log_context_create(IPC_LOG_PAGES,
						      gpii->label, 0);
		gpii->ipc_log_lvl = DEFAULT_IPC_LOG_LVL;
		gpii->klog_lvl = DEFAULT_KLOG_LVL;

		if (IS_ERR_OR_NULL(gpi_dev->dentry))
			continue;

		snprintf(node_name, sizeof(node_name), "gpii%d", i);
		gpii->dentry = debugfs_create_dir(node_name, gpi_dev->dentry);
		if (IS_ERR_OR_NULL(gpii->dentry))
			continue;

		debugfs_create_u32("ipc_log_lvl", mode, gpii->dentry,
				   &gpii->ipc_log_lvl);
		debugfs_create_u32("klog_lvl", mode, gpii->dentry,
				   &gpii->klog_lvl);
	}
}

static struct dma_iommu_mapping *gpi_create_mapping(struct gpi_dev *gpi_dev)
{
	dma_addr_t base;
	size_t size;

	/*
	 * If S1_BYPASS enabled then iommu space is not used, however framework
	 * still require clients to create a mapping space before attaching. So
	 * set to smallest size required by iommu framework.
	 */
	if (gpi_dev->smmu_cfg & GPI_SMMU_S1_BYPASS) {
		base = 0;
		size = PAGE_SIZE;
	} else {
		base = gpi_dev->iova_base;
		size = gpi_dev->iova_size;
	}

	GPI_LOG(gpi_dev, "Creating iommu mapping of base:0x%llx size:%lu\n",
		base, size);

	return arm_iommu_create_mapping(&platform_bus_type, base, size);
}

static int gpi_smmu_init(struct gpi_dev *gpi_dev)
{
	struct dma_iommu_mapping *mapping = NULL;
	int ret;

	if (gpi_dev->smmu_cfg) {

		/* create mapping table */
		mapping = gpi_create_mapping(gpi_dev);
		if (IS_ERR(mapping)) {
			GPI_ERR(gpi_dev,
				"Failed to create iommu mapping, ret:%ld\n",
				PTR_ERR(mapping));
			return PTR_ERR(mapping);
		}

		if (gpi_dev->smmu_cfg & GPI_SMMU_S1_BYPASS) {
			int s1_bypass = 1;

			ret = iommu_domain_set_attr(mapping->domain,
					DOMAIN_ATTR_S1_BYPASS, &s1_bypass);
			if (ret) {
				GPI_ERR(gpi_dev,
					"Failed to set attr S1_BYPASS, ret:%d\n",
					ret);
				goto release_mapping;
			}
		}

		if (gpi_dev->smmu_cfg & GPI_SMMU_FAST) {
			int fast = 1;

			ret = iommu_domain_set_attr(mapping->domain,
						    DOMAIN_ATTR_FAST, &fast);
			if (ret) {
				GPI_ERR(gpi_dev,
					"Failed to set attr FAST, ret:%d\n",
					ret);
				goto release_mapping;
			}
		}

		if (gpi_dev->smmu_cfg & GPI_SMMU_ATOMIC) {
			int atomic = 1;

			ret = iommu_domain_set_attr(mapping->domain,
						DOMAIN_ATTR_ATOMIC, &atomic);
			if (ret) {
				GPI_ERR(gpi_dev,
					"Failed to set attr ATOMIC, ret:%d\n",
					ret);
				goto release_mapping;
			}
		}

		ret = arm_iommu_attach_device(gpi_dev->dev, mapping);
		if (ret) {
			GPI_ERR(gpi_dev,
				"Failed with iommu_attach, ret:%d\n", ret);
			goto release_mapping;
		}
	}

	GPI_LOG(gpi_dev, "Setting dma mask to 64\n");
	ret = dma_set_mask(gpi_dev->dev, DMA_BIT_MASK(64));
	if (ret) {
		GPI_ERR(gpi_dev, "Error setting dma_mask to 64, ret:%d\n", ret);
		goto error_set_mask;
	}

	return ret;

error_set_mask:
	if (gpi_dev->smmu_cfg)
		arm_iommu_detach_device(gpi_dev->dev);
release_mapping:
	if (mapping)
		arm_iommu_release_mapping(mapping);
	return ret;
}

static int gpi_probe(struct platform_device *pdev)
{
	struct gpi_dev *gpi_dev;
	int ret, i;

	gpi_dev = devm_kzalloc(&pdev->dev, sizeof(*gpi_dev), GFP_KERNEL);
	if (!gpi_dev)
		return -ENOMEM;

	gpi_dev->dev = &pdev->dev;
	gpi_dev->klog_lvl = DEFAULT_KLOG_LVL;
	gpi_dev->res = platform_get_resource_byname(pdev, IORESOURCE_MEM,
						    "gpi-top");
	if (!gpi_dev->res) {
		GPI_ERR(gpi_dev, "missing 'reg' DT node\n");
		return -EINVAL;
	}
	gpi_dev->regs = devm_ioremap_nocache(gpi_dev->dev, gpi_dev->res->start,
					     resource_size(gpi_dev->res));
	if (!gpi_dev->regs) {
		GPI_ERR(gpi_dev, "IO remap failed\n");
		return -EFAULT;
	}

	ret = of_property_read_u32(gpi_dev->dev->of_node, "qcom,max-num-gpii",
				   &gpi_dev->max_gpii);
	if (ret) {
		GPI_ERR(gpi_dev, "missing 'max-no-gpii' DT node\n");
		return ret;
	}

	ret = of_property_read_u32(gpi_dev->dev->of_node, "qcom,gpii-mask",
				   &gpi_dev->gpii_mask);
	if (ret) {
		GPI_ERR(gpi_dev, "missing 'gpii-mask' DT node\n");
		return ret;
	}

	ret = of_property_read_u32(gpi_dev->dev->of_node, "qcom,ev-factor",
				   &gpi_dev->ev_factor);
	if (ret) {
		GPI_ERR(gpi_dev, "missing 'qcom,ev-factor' DT node\n");
		return ret;
	}

	ret = of_property_read_u32(gpi_dev->dev->of_node, "qcom,smmu-cfg",
				   &gpi_dev->smmu_cfg);
	if (ret) {
		GPI_ERR(gpi_dev, "missing 'qcom,smmu-cfg' DT node\n");
		return ret;
	}
	if (gpi_dev->smmu_cfg && !(gpi_dev->smmu_cfg & GPI_SMMU_S1_BYPASS)) {
		u64 iova_range[2];

		ret = of_property_count_elems_of_size(gpi_dev->dev->of_node,
						      "qcom,iova-range",
						      sizeof(iova_range));
		if (ret != 1) {
			GPI_ERR(gpi_dev,
				"missing or incorrect 'qcom,iova-range' DT node ret:%d\n",
				ret);
		}

		ret = of_property_read_u64_array(gpi_dev->dev->of_node,
					"qcom,iova-range", iova_range,
					sizeof(iova_range) / sizeof(u64));
		if (ret) {
			GPI_ERR(gpi_dev,
				"could not read DT prop 'qcom,iova-range\n");
			return ret;
		}
		gpi_dev->iova_base = iova_range[0];
		gpi_dev->iova_size = iova_range[1];
	}

	ret = gpi_smmu_init(gpi_dev);
	if (ret) {
		GPI_ERR(gpi_dev, "error configuring smmu, ret:%d\n", ret);
		return ret;
	}

	gpi_dev->gpiis = devm_kzalloc(gpi_dev->dev,
				sizeof(*gpi_dev->gpiis) * gpi_dev->max_gpii,
				GFP_KERNEL);
	if (!gpi_dev->gpiis)
		return -ENOMEM;


	/* setup all the supported gpii */
	INIT_LIST_HEAD(&gpi_dev->dma_device.channels);
	for (i = 0; i < gpi_dev->max_gpii; i++) {
		struct gpii *gpii = &gpi_dev->gpiis[i];
		int chan;

		if (!((1 << i) & gpi_dev->gpii_mask))
			continue;

		/* set up ev cntxt register map */
		gpii->ev_cntxt_base_reg = gpi_dev->regs +
			GPI_GPII_n_EV_CH_k_CNTXT_0_OFFS(i, 0);
		gpii->ev_cntxt_db_reg = gpi_dev->regs +
			GPI_GPII_n_EV_CH_k_DOORBELL_0_OFFS(i, 0);
		gpii->ev_ring_base_lsb_reg = gpii->ev_cntxt_base_reg +
			CNTXT_2_RING_BASE_LSB;
		gpii->ev_ring_rp_lsb_reg = gpii->ev_cntxt_base_reg +
			CNTXT_4_RING_RP_LSB;
		gpii->ev_ring_wp_lsb_reg = gpii->ev_cntxt_base_reg +
			CNTXT_6_RING_WP_LSB;
		gpii->ev_cmd_reg = gpi_dev->regs +
			GPI_GPII_n_EV_CH_CMD_OFFS(i);
		gpii->ieob_src_reg = gpi_dev->regs +
			GPI_GPII_n_CNTXT_SRC_IEOB_IRQ_OFFS(i);
		gpii->ieob_clr_reg = gpi_dev->regs +
			GPI_GPII_n_CNTXT_SRC_IEOB_IRQ_CLR_OFFS(i);

		/* set up irq */
		ret = platform_get_irq(pdev, i);
		if (ret < 0) {
			GPI_ERR(gpi_dev, "could not req. irq for gpii%d ret:%d",
				i, ret);
			return ret;
		}
		gpii->irq = ret;

		/* set up channel specific register info */
		for (chan = 0; chan < MAX_CHANNELS_PER_GPII; chan++) {
			struct gpii_chan *gpii_chan = &gpii->gpii_chan[chan];

			/* set up ch cntxt register map */
			gpii_chan->ch_cntxt_base_reg = gpi_dev->regs +
				GPI_GPII_n_CH_k_CNTXT_0_OFFS(i, chan);
			gpii_chan->ch_cntxt_db_reg = gpi_dev->regs +
				GPI_GPII_n_CH_k_DOORBELL_0_OFFS(i, chan);
			gpii_chan->ch_ring_base_lsb_reg =
				gpii_chan->ch_cntxt_base_reg +
				CNTXT_2_RING_BASE_LSB;
			gpii_chan->ch_ring_rp_lsb_reg =
				gpii_chan->ch_cntxt_base_reg +
				CNTXT_4_RING_RP_LSB;
			gpii_chan->ch_ring_wp_lsb_reg =
				gpii_chan->ch_cntxt_base_reg +
				CNTXT_6_RING_WP_LSB;
			gpii_chan->ch_cmd_reg = gpi_dev->regs +
				GPI_GPII_n_CH_CMD_OFFS(i);

			/* vchan setup */
			vchan_init(&gpii_chan->vc, &gpi_dev->dma_device);
			gpii_chan->vc.desc_free = gpi_desc_free;
			gpii_chan->chid = chan;
			gpii_chan->gpii = gpii;
			gpii_chan->dir = GPII_CHAN_DIR[chan];
		}
		mutex_init(&gpii->ctrl_lock);
		rwlock_init(&gpii->pm_lock);
		tasklet_init(&gpii->ev_task, gpi_ev_tasklet,
			     (unsigned long)gpii);
		init_completion(&gpii->cmd_completion);
		gpii->gpii_id = i;
		gpii->regs = gpi_dev->regs;
		gpii->gpi_dev = gpi_dev;
		atomic_set(&gpii->dbg_index, 0);
	}

	platform_set_drvdata(pdev, gpi_dev);

	/* clear and Set capabilities */
	dma_cap_zero(gpi_dev->dma_device.cap_mask);
	dma_cap_set(DMA_SLAVE, gpi_dev->dma_device.cap_mask);

	/* configure dmaengine apis */
	gpi_dev->dma_device.directions =
		BIT(DMA_DEV_TO_MEM) | BIT(DMA_MEM_TO_DEV);
	gpi_dev->dma_device.residue_granularity =
		DMA_RESIDUE_GRANULARITY_DESCRIPTOR;
	gpi_dev->dma_device.src_addr_widths = DMA_SLAVE_BUSWIDTH_8_BYTES;
	gpi_dev->dma_device.dst_addr_widths = DMA_SLAVE_BUSWIDTH_8_BYTES;
	gpi_dev->dma_device.device_alloc_chan_resources =
		gpi_alloc_chan_resources;
	gpi_dev->dma_device.device_free_chan_resources =
		gpi_free_chan_resources;
	gpi_dev->dma_device.device_tx_status = dma_cookie_status;
	gpi_dev->dma_device.device_issue_pending = gpi_issue_pending;
	gpi_dev->dma_device.device_prep_slave_sg = gpi_prep_slave_sg;
	gpi_dev->dma_device.device_config = gpi_config;
	gpi_dev->dma_device.device_terminate_all = gpi_terminate_all;
	gpi_dev->dma_device.dev = gpi_dev->dev;
	gpi_dev->dma_device.device_pause = gpi_pause;
	gpi_dev->dma_device.device_resume = gpi_resume;

	/* register with dmaengine framework */
	ret = dma_async_device_register(&gpi_dev->dma_device);
	if (ret) {
		GPI_ERR(gpi_dev, "async_device_register failed ret:%d", ret);
		return ret;
	}

	ret = of_dma_controller_register(gpi_dev->dev->of_node,
					 gpi_of_dma_xlate, gpi_dev);
	if (ret) {
		GPI_ERR(gpi_dev, "of_dma_controller_reg failed ret:%d", ret);
		return ret;
	}

	/* setup debug capabilities */
	gpi_setup_debug(gpi_dev);
	GPI_LOG(gpi_dev, "probe success\n");

	return ret;
}

static const struct of_device_id gpi_of_match[] = {
	{ .compatible = "qcom,gpi-dma" },
	{}
};
MODULE_DEVICE_TABLE(of, gpi_of_match);

static struct platform_driver gpi_driver = {
	.probe = gpi_probe,
	.driver = {
		.name = GPI_DMA_DRV_NAME,
		.of_match_table = gpi_of_match,
	},
};

static int __init gpi_init(void)
{
	pdentry = debugfs_create_dir(GPI_DMA_DRV_NAME, NULL);
	return platform_driver_register(&gpi_driver);
}
module_init(gpi_init)

MODULE_DESCRIPTION("QCOM GPI DMA engine driver");
MODULE_LICENSE("GPL v2");
