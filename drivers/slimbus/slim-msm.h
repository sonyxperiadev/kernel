/* Copyright (c) 2011-2019, The Linux Foundation. All rights reserved.
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

#ifndef _SLIM_MSM_H
#define _SLIM_MSM_H

#include <linux/ipc_logging.h>
#include <linux/irq.h>
#include <linux/kthread.h>
#include <linux/qrtr.h>
#include <linux/soc/qcom/qmi.h>
#include <net/sock.h>

/* Per spec.max 40 bytes per received message */
#define SLIM_MSGQ_BUF_LEN	40

#define MSM_TX_BUFS		32

#define SLIM_USR_MC_GENERIC_ACK		0x25
#define SLIM_USR_MC_MASTER_CAPABILITY	0x0
#define SLIM_USR_MC_REPORT_SATELLITE	0x1
#define SLIM_USR_MC_ADDR_QUERY		0xD
#define SLIM_USR_MC_ADDR_REPLY		0xE
#define SLIM_USR_MC_DEFINE_CHAN		0x20
#define SLIM_USR_MC_DEF_ACT_CHAN	0x21
#define SLIM_USR_MC_CHAN_CTRL		0x23
#define SLIM_USR_MC_RECONFIG_NOW	0x24
#define SLIM_USR_MC_REQ_BW		0x28
#define SLIM_USR_MC_CONNECT_SRC		0x2C
#define SLIM_USR_MC_CONNECT_SINK	0x2D
#define SLIM_USR_MC_DISCONNECT_PORT	0x2E

#define SLIM_USR_MC_REPEAT_CHANGE_VALUE	0x0
#define MSM_SLIM_VE_MAX_MAP_ADDR	0xFFF
#define SLIM_MAX_VE_SLC_BYTES		16

#define MSM_SLIM_AUTOSUSPEND		(MSEC_PER_SEC / 10)

#define SLIM_RX_MSGQ_TIMEOUT_VAL	0x10000
/*
 * Messages that can be received simultaneously:
 * Client reads, LPASS master responses, announcement messages
 * Receive upto 10 messages simultaneously.
 */
#define MSM_SLIM_DESC_NUM		32

/* MSM Slimbus peripheral settings */
#define MSM_SLIM_PERF_SUMM_THRESHOLD	0x8000
#define MSM_SLIM_NPORTS			24
#define MSM_SLIM_NCHANS			32

#define QC_MFGID_LSB	0x2
#define QC_MFGID_MSB	0x17
#define QC_CHIPID_SL	0x10
#define QC_DEVID_SAT1	0x3
#define QC_DEVID_SAT2	0x4
#define QC_DEVID_PGD	0x5

#define SLIM_MSG_ASM_FIRST_WORD(l, mt, mc, dt, ad) \
		((l) | ((mt) << 5) | ((mc) << 8) | ((dt) << 15) | ((ad) << 16))

#define INIT_MX_RETRIES 3
#define DEF_RETRY_MS	10
#define MSM_CONCUR_MSG	8
#define SAT_CONCUR_MSG	8

#define DEF_WATERMARK	(8 << 1)
#define DEF_ALIGN_LSB	0
#define DEF_ALIGN_MSB	(1 << 7)
#define DEF_PACK	(1 << 6)
#define DEF_NO_PACK	0
#define ENABLE_PORT	1

#define DEF_BLKSZ	0
#define DEF_TRANSZ	0

#define SAT_MAGIC_LSB	0xD9
#define SAT_MAGIC_MSB	0xC5
#define SAT_MSG_VER	0x1
#define SAT_MSG_PROT	0x1
#define MSM_SAT_SUCCSS	0x20
#define MSM_MAX_NSATS	2
#define MSM_MAX_SATCH	32

/* Slimbus QMI service */
#define SLIMBUS_QMI_SVC_ID 0x0301
#define SLIMBUS_QMI_SVC_V1 1
#define SLIMBUS_QMI_INS_ID 0

/* QMI response timeout of 1000ms */
#define SLIM_QMI_RESP_TOUT (HZ)

#define PGD_THIS_EE(r, v) ((v) ? PGD_THIS_EE_V2(r) : PGD_THIS_EE_V1(r))
#define PGD_PORT(r, p, v) ((v) ? PGD_PORT_V2(r, p) : PGD_PORT_V1(r, p))
#define CFG_PORT(r, v) ((v) ? CFG_PORT_V2(r) : CFG_PORT_V1(r))

#define PGD_THIS_EE_V2(r) (dev->base + (r ## _V2) + (dev->ee * 0x1000))
#define PGD_PORT_V2(r, p) (dev->base + (r ## _V2) + ((p) * 0x1000))
#define CFG_PORT_V2(r) ((r ## _V2))
/* Component registers */
enum comp_reg_v2 {
	COMP_CFG_V2		= 4,
	COMP_TRUST_CFG_V2	= 0x3000,
};

/* Manager PGD registers */
enum pgd_reg_v2 {
	PGD_CFG_V2		= 0x800,
	PGD_STAT_V2		= 0x804,
	PGD_INT_EN_V2		= 0x810,
	PGD_INT_STAT_V2		= 0x814,
	PGD_INT_CLR_V2		= 0x818,
	PGD_OWN_EEn_V2		= 0x300C,
	PGD_PORT_INT_EN_EEn_V2	= 0x5000,
	PGD_PORT_INT_ST_EEn_V2	= 0x5004,
	PGD_PORT_INT_CL_EEn_V2	= 0x5008,
	PGD_PORT_CFGn_V2	= 0x14000,
	PGD_PORT_STATn_V2	= 0x14004,
	PGD_PORT_PARAMn_V2	= 0x14008,
	PGD_PORT_BLKn_V2	= 0x1400C,
	PGD_PORT_TRANn_V2	= 0x14010,
	PGD_PORT_MCHANn_V2	= 0x14014,
	PGD_PORT_PSHPLLn_V2	= 0x14018,
	PGD_PORT_PC_CFGn_V2	= 0x8000,
	PGD_PORT_PC_VALn_V2	= 0x8004,
	PGD_PORT_PC_VFR_TSn_V2	= 0x8008,
	PGD_PORT_PC_VFR_STn_V2	= 0x800C,
	PGD_PORT_PC_VFR_CLn_V2	= 0x8010,
	PGD_IE_STAT_V2		= 0x820,
	PGD_VE_STAT_V2		= 0x830,
};

#define PGD_THIS_EE_V1(r) (dev->base + (r ## _V1) + (dev->ee * 16))
#define PGD_PORT_V1(r, p) (dev->base + (r ## _V1) + ((p) * 32))
#define CFG_PORT_V1(r) ((r ## _V1))
/* Component registers */
enum comp_reg_v1 {
	COMP_CFG_V1		= 0,
	COMP_TRUST_CFG_V1	= 0x14,
};

/* Manager PGD registers */
enum pgd_reg_v1 {
	PGD_CFG_V1		= 0x1000,
	PGD_STAT_V1		= 0x1004,
	PGD_INT_EN_V1		= 0x1010,
	PGD_INT_STAT_V1		= 0x1014,
	PGD_INT_CLR_V1		= 0x1018,
	PGD_OWN_EEn_V1		= 0x1020,
	PGD_PORT_INT_EN_EEn_V1	= 0x1030,
	PGD_PORT_INT_ST_EEn_V1	= 0x1034,
	PGD_PORT_INT_CL_EEn_V1	= 0x1038,
	PGD_PORT_CFGn_V1	= 0x1080,
	PGD_PORT_STATn_V1	= 0x1084,
	PGD_PORT_PARAMn_V1	= 0x1088,
	PGD_PORT_BLKn_V1	= 0x108C,
	PGD_PORT_TRANn_V1	= 0x1090,
	PGD_PORT_MCHANn_V1	= 0x1094,
	PGD_PORT_PSHPLLn_V1	= 0x1098,
	PGD_PORT_PC_CFGn_V1	= 0x1600,
	PGD_PORT_PC_VALn_V1	= 0x1604,
	PGD_PORT_PC_VFR_TSn_V1	= 0x1608,
	PGD_PORT_PC_VFR_STn_V1	= 0x160C,
	PGD_PORT_PC_VFR_CLn_V1	= 0x1610,
	PGD_IE_STAT_V1		= 0x1700,
	PGD_VE_STAT_V1		= 0x1710,
};

enum msm_slim_port_status {
	MSM_PORT_OVERFLOW	= 1 << 2,
	MSM_PORT_UNDERFLOW	= 1 << 3,
	MSM_PORT_DISCONNECT	= 1 << 19,
};

enum msm_ctrl_state {
	MSM_CTRL_AWAKE,
	MSM_CTRL_IDLE,
	MSM_CTRL_ASLEEP,
	MSM_CTRL_DOWN,
};

enum msm_slim_msgq {
	MSM_MSGQ_DISABLED,
	MSM_MSGQ_RESET,
	MSM_MSGQ_ENABLED,
	MSM_MSGQ_DOWN,
};

struct msm_slim_sps_bam {
	unsigned long		hdl;
	void __iomem		*base;
	int			irq;
};

/*
 * struct slim_pshpull_parm: Structure to store push pull protocol parameters
 * @num_samples: Number of samples in a period
 * @rpt_period: Repeat period value
 */
struct msm_slim_pshpull_parm {
	int		num_samples;
	int		rpt_period;
};

struct msm_slim_endp {
	struct sps_pipe			*sps;
	struct sps_connect		config;
	struct sps_register_event	event;
	struct sps_mem_buffer		buf;
	bool				connected;
	int				port_b;
	struct msm_slim_pshpull_parm	psh_pull;
};

struct msm_slim_qmi {
	struct qmi_handle		*handle;
	struct sockaddr_qrtr		svc_info;
	struct task_struct		*slave_thread;
	struct completion		slave_notify;
	struct completion		qmi_comp;
	struct qmi_handle		svc_event_hdl;
	bool				deferred_resp;
	struct qmi_response_type_v01	resp;
	struct qmi_txn			deferred_txn;
};

enum msm_slim_dom {
	MSM_SLIM_DOM_NONE,
	MSM_SLIM_DOM_PD,
	MSM_SLIM_DOM_SS,
};

struct msm_slim_ss {
	struct notifier_block nb;
	void *domr;
	enum msm_ctrl_state state;
	struct work_struct dom_up;
	enum msm_slim_dom dom_t;
};

struct msm_slim_pdata {
	u32 apps_pipes;
	u32 eapc;
};

struct msm_slim_bulk_wr {
	dma_addr_t	wr_dma;
	void		*base;
	int		size;
	int		buf_sz;
	int		(*cb)(void *ctx, int err);
	void		*ctx;
	bool		in_progress;
};

struct msm_slim_iommu {
	struct device			*cb_dev;
	struct dma_iommu_mapping	*iommu_map;
	bool				s1_bypass;
};

struct msm_slim_ctrl {
	struct slim_controller  ctrl;
	struct slim_framer	framer;
	struct device		*dev;
	struct msm_slim_iommu	iommu_desc;
	void __iomem		*base;
	struct resource		*slew_mem;
	struct resource		*bam_mem;
	u32			curr_bw;
	u8			msg_cnt;
	u32			tx_buf[10];
	u8			rx_msgs[MSM_CONCUR_MSG][SLIM_MSGQ_BUF_LEN];
	int			tx_tail;
	int			tx_head;
	spinlock_t		rx_lock;
	int			head;
	int			tail;
	int			irq;
	int			err;
	int			ee;
	struct completion	**wr_comp;
	struct msm_slim_sat	*satd[MSM_MAX_NSATS];
	struct msm_slim_endp	*pipes;
	struct msm_slim_sps_bam	bam;
	struct msm_slim_endp	tx_msgq;
	struct msm_slim_endp	rx_msgq;
	struct completion	rx_msgq_notify;
	struct task_struct	*rx_msgq_thread;
	struct clk		*rclk;
	struct clk		*hclk;
	struct mutex		tx_lock;
	struct mutex		ssr_lock;
	spinlock_t		tx_buf_lock;
	u8			pgdla;
	enum msm_slim_msgq	use_rx_msgqs;
	enum msm_slim_msgq	use_tx_msgqs;
	int			port_nums;
	struct completion	reconf;
	bool			reconf_busy;
	bool			chan_active;
	enum msm_ctrl_state	state;
	struct completion	ctrl_up;
	int			nsats;
	u32			ver;
	struct msm_slim_qmi	qmi;
	struct msm_slim_pdata	pdata;
	struct msm_slim_ss	ext_mdm;
	struct msm_slim_ss	dsp;
	struct msm_slim_bulk_wr	bulk;
	int			default_ipc_log_mask;
	int			ipc_log_mask;
	bool			sysfs_created;
	bool			legacy_pwr_msg;
	void			*ipc_slimbus_log;
	void			*ipc_slimbus_log_err;
	void (*rx_slim)(struct msm_slim_ctrl *dev, u8 *buf);
	u32			current_rx_buf[10];
	int			current_count;
	atomic_t		ssr_in_progress;
};

struct msm_sat_chan {
	u8 chan;
	u16 chanh;
	int req_rem;
	int req_def;
	bool reconf;
};

struct msm_slim_sat {
	struct slim_device	satcl;
	struct msm_slim_ctrl	*dev;
	struct workqueue_struct *wq;
	struct work_struct	wd;
	u8			sat_msgs[SAT_CONCUR_MSG][40];
	struct msm_sat_chan	*satch;
	u8			nsatch;
	bool			sent_capability;
	bool			pending_reconf;
	bool			pending_capability;
	int			shead;
	int			stail;
	spinlock_t lock;
};

enum rsc_grp {
	EE_MGR_RSC_GRP	= 1 << 10,
	EE_NGD_2	= 2 << 6,
	EE_NGD_1	= 0,
};


/* IPC logging stuff */
#define IPC_SLIMBUS_LOG_PAGES 5

/* Log levels */
enum {
	FATAL_LEV = 0U,
	ERR_LEV = 1U,
	WARN_LEV = 2U,
	INFO_LEV = 3U,
	DBG_LEV = 4U,
};

/* Default IPC log level INFO */
#define SLIM_DBG(dev, x...) do { \
	pr_debug(x); \
	if (dev->ipc_slimbus_log && dev->ipc_log_mask >= DBG_LEV) { \
		ipc_log_string(dev->ipc_slimbus_log, x); \
	} \
	if (dev->ipc_slimbus_log_err && dev->ipc_log_mask == FATAL_LEV) { \
		ipc_log_string(dev->ipc_slimbus_log_err, x); \
	} \
} while (0)

#define SLIM_INFO(dev, x...) do { \
	pr_debug(x); \
	if (dev->ipc_slimbus_log && dev->ipc_log_mask >= INFO_LEV) {\
		ipc_log_string(dev->ipc_slimbus_log, x); \
	} \
	if (dev->ipc_slimbus_log_err && dev->ipc_log_mask == FATAL_LEV) { \
		ipc_log_string(dev->ipc_slimbus_log_err, x); \
	} \
} while (0)

/* warnings and errors show up on console always */
#define SLIM_WARN(dev, x...) do { \
	if (dev->ipc_slimbus_log && dev->ipc_log_mask >= WARN_LEV) { \
		pr_warn(x); \
		ipc_log_string(dev->ipc_slimbus_log, x); \
	} \
	if (dev->ipc_slimbus_log_err && dev->ipc_log_mask == FATAL_LEV) { \
		ipc_log_string(dev->ipc_slimbus_log_err, x); \
	} \
} while (0)

/* ERROR condition in the driver sets the hs_serial_debug_mask
 * to ERR_FATAL level, so that this message can be seen
 * in IPC logging. Further errors continue to log on the error IPC logging.
 */
#define SLIM_ERR(dev, x...) do { \
	if (dev->ipc_slimbus_log && dev->ipc_log_mask >= ERR_LEV) { \
		pr_err(x); \
		ipc_log_string(dev->ipc_slimbus_log, x); \
		dev->default_ipc_log_mask = dev->ipc_log_mask; \
		dev->ipc_log_mask = FATAL_LEV; \
	} \
	if (dev->ipc_slimbus_log_err && dev->ipc_log_mask == FATAL_LEV) { \
		ipc_log_string(dev->ipc_slimbus_log_err, x); \
	} \
} while (0)

#define SLIM_RST_LOGLVL(dev) { \
	dev->ipc_log_mask = dev->default_ipc_log_mask; \
}

int msm_slim_rx_enqueue(struct msm_slim_ctrl *dev, u32 *buf, u8 len);
int msm_slim_rx_dequeue(struct msm_slim_ctrl *dev, u8 *buf);
int msm_slim_get_ctrl(struct msm_slim_ctrl *dev);
void msm_slim_put_ctrl(struct msm_slim_ctrl *dev);
irqreturn_t msm_slim_port_irq_handler(struct msm_slim_ctrl *dev, u32 pstat);
int msm_slim_init_endpoint(struct msm_slim_ctrl *dev, struct msm_slim_endp *ep);
void msm_slim_free_endpoint(struct msm_slim_endp *ep);
void msm_hw_set_port(struct msm_slim_ctrl *dev, u8 pipenum, u8 portnum);
int msm_alloc_port(struct slim_controller *ctrl, u8 pn);
void msm_dealloc_port(struct slim_controller *ctrl, u8 pn);
int msm_slim_connect_pipe_port(struct msm_slim_ctrl *dev, u8 pn);
enum slim_port_err msm_slim_port_xfer_status(struct slim_controller *ctr,
				u8 pn, phys_addr_t *done_buf, u32 *done_len);
int msm_slim_port_xfer(struct slim_controller *ctrl, u8 pn, void *buf,
			u32 len, struct completion *comp);
int msm_send_msg_buf(struct msm_slim_ctrl *dev, u32 *buf, u8 len, u32 tx_reg);
u32 *msm_get_msg_buf(struct msm_slim_ctrl *dev, int len,
			struct completion *comp);
u32 *msm_slim_manage_tx_msgq(struct msm_slim_ctrl *dev, bool getbuf,
			struct completion *comp, int err);
int msm_slim_rx_msgq_get(struct msm_slim_ctrl *dev, u32 *data, int offset);
int msm_slim_sps_init(struct msm_slim_ctrl *dev, struct resource *bam_mem,
			u32 pipe_reg, bool remote);
void msm_slim_sps_exit(struct msm_slim_ctrl *dev, bool dereg);

int msm_slim_connect_endp(struct msm_slim_ctrl *dev,
				struct msm_slim_endp *endpoint);
void msm_slim_disconnect_endp(struct msm_slim_ctrl *dev,
					struct msm_slim_endp *endpoint,
					enum msm_slim_msgq *msgq_flag);
void msm_slim_deinit_ep(struct msm_slim_ctrl *dev,
				struct msm_slim_endp *endpoint,
				enum msm_slim_msgq *msgq_flag);

void msm_slim_qmi_exit(struct msm_slim_ctrl *dev);
int msm_slim_qmi_init(struct msm_slim_ctrl *dev, bool apps_is_master);
int msm_slim_qmi_power_request(struct msm_slim_ctrl *dev, bool active);
int msm_slim_qmi_check_framer_request(struct msm_slim_ctrl *dev);
int msm_slim_qmi_deferred_status_req(struct msm_slim_ctrl *dev);
#endif
