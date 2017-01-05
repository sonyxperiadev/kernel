/*
 *  linux/include/linux/mmc/host.h
 *
 * Copyright (c) 2015 Sony Mobile Communications Inc
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *  Host driver specific definitions.
 */
#ifndef LINUX_MMC_HOST_H
#define LINUX_MMC_HOST_H

#include <linux/leds.h>
#include <linux/mutex.h>
#include <linux/sched.h>
#include <linux/device.h>
#include <linux/devfreq.h>
#include <linux/fault-inject.h>

#include <linux/mmc/core.h>
#include <linux/mmc/card.h>
#include <linux/mmc/pm.h>

/* Default idle timeout for MMC devices: 3 seconds. */
#define MMC_AUTOSUSPEND_DELAY_MS	3000
/* Default idle timeout for SD cards: 5 minutes. */
#define MMC_SDCARD_AUTOSUSPEND_DELAY_MS 30000

struct mmc_ios {
	unsigned int	clock;			/* clock rate */
	unsigned int	old_rate;       /* saved clock rate */
	unsigned long	clk_ts;         /* time stamp of last updated clock */
	unsigned short	vdd;

/* vdd stores the bit number of the selected voltage range from below. */

	unsigned char	bus_mode;		/* command output mode */

#define MMC_BUSMODE_OPENDRAIN	1
#define MMC_BUSMODE_PUSHPULL	2

	unsigned char	chip_select;		/* SPI chip select */

#define MMC_CS_DONTCARE		0
#define MMC_CS_HIGH		1
#define MMC_CS_LOW		2

	unsigned char	power_mode;		/* power supply mode */

#define MMC_POWER_OFF		0
#define MMC_POWER_UP		1
#define MMC_POWER_ON		2
#define MMC_POWER_UNDEFINED	3

	unsigned char	bus_width;		/* data bus width */

#define MMC_BUS_WIDTH_1		0
#define MMC_BUS_WIDTH_4		2
#define MMC_BUS_WIDTH_8		3

	unsigned char	timing;			/* timing specification used */

#define MMC_TIMING_LEGACY	0
#define MMC_TIMING_MMC_HS	1
#define MMC_TIMING_SD_HS	2
#define MMC_TIMING_UHS_SDR12	3
#define MMC_TIMING_UHS_SDR25	4
#define MMC_TIMING_UHS_SDR50	5
#define MMC_TIMING_UHS_SDR104	6
#define MMC_TIMING_UHS_DDR50	7
#define MMC_TIMING_MMC_DDR52	8
#define MMC_TIMING_MMC_HS200	9
#define MMC_TIMING_MMC_HS400	10

	unsigned char	signal_voltage;		/* signalling voltage (1.8V or 3.3V) */

#define MMC_SIGNAL_VOLTAGE_330	0
#define MMC_SIGNAL_VOLTAGE_180	1
#define MMC_SIGNAL_VOLTAGE_120	2

	unsigned char	drv_type;		/* driver type (A, B, C, D) */

#define MMC_SET_DRIVER_TYPE_B	0
#define MMC_SET_DRIVER_TYPE_A	1
#define MMC_SET_DRIVER_TYPE_C	2
#define MMC_SET_DRIVER_TYPE_D	3
};

/* states to represent load on the host */
enum mmc_load {
	MMC_LOAD_HIGH,
	MMC_LOAD_LOW,
};

struct mmc_cmdq_host_ops {
	int (*init)(struct mmc_host *host);
	int (*enable)(struct mmc_host *host);
	void (*disable)(struct mmc_host *host, bool soft);
	int (*request)(struct mmc_host *host, struct mmc_request *mrq);
	void (*post_req)(struct mmc_host *host, int tag, int err);
	int (*halt)(struct mmc_host *host, bool halt);
	void (*reset)(struct mmc_host *host, bool soft);
	void (*dumpstate)(struct mmc_host *host);
};

struct mmc_host_ops {
	int (*init)(struct mmc_host *host);
	/*
	 * 'enable' is called when the host is claimed and 'disable' is called
	 * when the host is released. 'enable' and 'disable' are deprecated.
	 */
	int (*enable)(struct mmc_host *host);
	int (*disable)(struct mmc_host *host);
	/*
	 * It is optional for the host to implement pre_req and post_req in
	 * order to support double buffering of requests (prepare one
	 * request while another request is active).
	 * pre_req() must always be followed by a post_req().
	 * To undo a call made to pre_req(), call post_req() with
	 * a nonzero err condition.
	 */
	void	(*post_req)(struct mmc_host *host, struct mmc_request *req,
			    int err);
	void	(*pre_req)(struct mmc_host *host, struct mmc_request *req,
			   bool is_first_req);
	void	(*request)(struct mmc_host *host, struct mmc_request *req);
	/*
	 * Avoid calling these three functions too often or in a "fast path",
	 * since underlaying controller might implement them in an expensive
	 * and/or slow way.
	 *
	 * Also note that these functions might sleep, so don't call them
	 * in the atomic contexts!
	 *
	 * Return values for the get_ro callback should be:
	 *   0 for a read/write card
	 *   1 for a read-only card
	 *   -ENOSYS when not supported (equal to NULL callback)
	 *   or a negative errno value when something bad happened
	 *
	 * Return values for the get_cd callback should be:
	 *   0 for a absent card
	 *   1 for a present card
	 *   -ENOSYS when not supported (equal to NULL callback)
	 *   or a negative errno value when something bad happened
	 */
	void	(*set_ios)(struct mmc_host *host, struct mmc_ios *ios);
	int	(*get_ro)(struct mmc_host *host);
	int	(*get_cd)(struct mmc_host *host);

	void	(*enable_sdio_irq)(struct mmc_host *host, int enable);

	/* optional callback for HC quirks */
	void	(*init_card)(struct mmc_host *host, struct mmc_card *card);

	int	(*start_signal_voltage_switch)(struct mmc_host *host, struct mmc_ios *ios);

	/* Check if the card is pulling dat[0:3] low */
	int	(*card_busy)(struct mmc_host *host);

	/* The tuning command opcode value is different for SD and eMMC cards */
	int	(*execute_tuning)(struct mmc_host *host, u32 opcode);

	/* Prepare HS400 target operating frequency depending host driver */
	int	(*prepare_hs400_tuning)(struct mmc_host *host, struct mmc_ios *ios);
	int	(*enhanced_strobe)(struct mmc_host *host);
	int	(*select_drive_strength)(unsigned int max_dtr, int host_drv, int card_drv);
	void	(*hw_reset)(struct mmc_host *host);
	void	(*card_event)(struct mmc_host *host);

	/*
	 * Optional callback to support controllers with HW issues for multiple
	 * I/O. Returns the number of supported blocks for the request.
	 */
	int	(*multi_io_quirk)(struct mmc_card *card,
				  unsigned int direction, int blk_size);

	unsigned long (*get_max_frequency)(struct mmc_host *host);
	unsigned long (*get_min_frequency)(struct mmc_host *host);
	int	(*notify_load)(struct mmc_host *, enum mmc_load);
	void	(*notify_halt)(struct mmc_host *mmc, bool halt);
	void	(*force_err_irq)(struct mmc_host *host, u64 errmask);
};

struct mmc_card;
struct device;

struct mmc_cmdq_req {
	unsigned int cmd_flags;
	u32 blk_addr;
	/* active mmc request */
	struct mmc_request	mrq;
	struct mmc_data		data;
	struct mmc_command	cmd;
#define DCMD		(1 << 0)
#define QBR		(1 << 1)
#define DIR		(1 << 2)
#define PRIO		(1 << 3)
#define REL_WR		(1 << 4)
#define DAT_TAG	(1 << 5)
#define FORCED_PRG	(1 << 6)
	unsigned int		cmdq_req_flags;

	unsigned int		resp_idx;
	unsigned int		resp_arg;
	unsigned int		dev_pend_tasks;
	bool			resp_err;
	int			tag; /* used for command queuing */
	u8			ctx_id;
};

struct mmc_async_req {
	/* active mmc request */
	struct mmc_request	*mrq;
	/*
	 * Check error status of completed mmc request.
	 * Returns 0 if success otherwise non zero.
	 */
	int (*err_check) (struct mmc_card *, struct mmc_async_req *);
};

/**
 * struct mmc_slot - MMC slot functions
 *
 * @cd_irq:		MMC/SD-card slot hotplug detection IRQ or -EINVAL
 * @lock:		protect the @handler_priv pointer
 * @handler_priv:	MMC/SD-card slot context
 *
 * Some MMC/SD host controllers implement slot-functions like card and
 * write-protect detection natively. However, a large number of controllers
 * leave these functions to the CPU. This struct provides a hook to attach
 * such slot-function drivers.
 */
struct mmc_slot {
	int cd_irq;
	struct mutex lock;
	void *handler_priv;
};


/**
 * mmc_cmdq_context_info - describes the contexts of cmdq
 * @active_reqs		requests being processed
 * @data_active_reqs	data requests being processed
 * @curr_state		state of cmdq engine
 * @cmdq_ctx_lock	acquire this before accessing this structure
 * @queue_empty_wq	workqueue for waiting for all
 *			the outstanding requests to be completed
 * @wait		waiting for all conditions described in
 *			mmc_cmdq_ready_wait to be satisified before
 *			issuing the new request to LLD.
 */
struct mmc_cmdq_context_info {
	unsigned long	active_reqs; /* in-flight requests */
	unsigned long	data_active_reqs; /* in-flight data requests */
	unsigned long	curr_state;
#define	CMDQ_STATE_ERR 0
#define	CMDQ_STATE_DCMD_ACTIVE 1
#define	CMDQ_STATE_HALT 2
#define	CMDQ_STATE_CQ_DISABLE 3
#define	CMDQ_STATE_REQ_TIMED_OUT 4
	wait_queue_head_t	queue_empty_wq;
	wait_queue_head_t	wait;
	int active_small_sector_read_reqs;
};

/**
 * mmc_context_info - synchronization details for mmc context
 * @is_done_rcv		wake up reason was done request
 * @is_new_req		wake up reason was new request
 * @is_waiting_last_req	mmc context waiting for single running request
 * @wait		wait queue
 * @lock		lock to protect data fields
 */
struct mmc_context_info {
	bool			is_done_rcv;
	bool			is_new_req;
	bool			is_waiting_last_req;
	wait_queue_head_t	wait;
	spinlock_t		lock;
};

struct regulator;

struct mmc_supply {
	struct regulator *vmmc;		/* Card power supply */
	struct regulator *vqmmc;	/* Optional Vccq supply */
};

enum dev_state {
	DEV_SUSPENDING = 1,
	DEV_SUSPENDED,
	DEV_RESUMED,
};

/**
 * struct mmc_devfeq_clk_scaling - main context for MMC clock scaling logic
 *
 * @lock: spinlock to protect statistics
 * @devfreq: struct that represent mmc-host as a client for devfreq
 * @devfreq_profile: MMC device profile, mostly polling interval and callbacks
 * @ondemand_gov_data: struct supplied to ondemmand governor (thresholds)
 * @state: load state, can be HIGH or LOW. used to notify mmc_host_ops callback
 * @start_busy: timestamped armed once a data request is started
 * @measure_interval_start: timestamped armed once a measure interval started
 * @devfreq_abort: flag to sync between different contexts relevant to devfreq
 * @skip_clk_scale_freq_update: flag that enable/disable frequency change
 * @freq_table_sz: table size of frequencies supplied to devfreq
 * @freq_table: frequencies table supplied to devfreq
 * @curr_freq: current frequency
 * @polling_delay_ms: polling interval for status collection used by devfreq
 * @upthreshold: up-threshold supplied to ondemand governor
 * @downthreshold: down-threshold supplied to ondemand governor
 * @need_freq_change: flag indicating if a frequency change is required
 * @clk_scaling_in_progress: flag indicating if there's ongoing frequency change
 * @is_busy_started: flag indicating if a request is handled by the HW
 * @enable: flag indicating if the clock scaling logic is enabled for this host
 */
struct mmc_devfeq_clk_scaling {
	spinlock_t	lock;
	struct		devfreq *devfreq;
	struct		devfreq_dev_profile devfreq_profile;
	struct		devfreq_simple_ondemand_data ondemand_gov_data;
	enum mmc_load	state;
	ktime_t		start_busy;
	ktime_t		measure_interval_start;
	atomic_t	devfreq_abort;
	u32		skip_clk_scale_freq_update;
	int		freq_table_sz;
	u32		*freq_table;
	unsigned long	total_busy_time_us;
	unsigned long	target_freq;
	unsigned long	curr_freq;
	unsigned long	polling_delay_ms;
	unsigned int	upthreshold;
	unsigned int	downthreshold;
	unsigned int	lower_bus_speed_mode;
#define MMC_SCALING_LOWER_DDR52_MODE	1
	bool		need_freq_change;
	bool		clk_scaling_in_progress;
	bool		is_busy_started;
	bool		enable;
};

struct mmc_host {
	struct device		*parent;
	struct device		class_dev;
	struct mmc_devfeq_clk_scaling	clk_scaling;
	int			index;
	const struct mmc_host_ops *ops;
	const struct mmc_cmdq_host_ops *cmdq_ops;
	unsigned int		f_min;
	unsigned int		f_max;
	unsigned int		f_init;
	u32			ocr_avail;
	u32			ocr_avail_sdio;	/* SDIO-specific OCR */
	u32			ocr_avail_sd;	/* SD-specific OCR */
	u32			ocr_avail_mmc;	/* MMC-specific OCR */
	struct notifier_block	pm_notify;
	u32			max_current_330;
	u32			max_current_300;
	u32			max_current_180;

#define MMC_VDD_165_195		0x00000080	/* VDD voltage 1.65 - 1.95 */
#define MMC_VDD_20_21		0x00000100	/* VDD voltage 2.0 ~ 2.1 */
#define MMC_VDD_21_22		0x00000200	/* VDD voltage 2.1 ~ 2.2 */
#define MMC_VDD_22_23		0x00000400	/* VDD voltage 2.2 ~ 2.3 */
#define MMC_VDD_23_24		0x00000800	/* VDD voltage 2.3 ~ 2.4 */
#define MMC_VDD_24_25		0x00001000	/* VDD voltage 2.4 ~ 2.5 */
#define MMC_VDD_25_26		0x00002000	/* VDD voltage 2.5 ~ 2.6 */
#define MMC_VDD_26_27		0x00004000	/* VDD voltage 2.6 ~ 2.7 */
#define MMC_VDD_27_28		0x00008000	/* VDD voltage 2.7 ~ 2.8 */
#define MMC_VDD_28_29		0x00010000	/* VDD voltage 2.8 ~ 2.9 */
#define MMC_VDD_29_30		0x00020000	/* VDD voltage 2.9 ~ 3.0 */
#define MMC_VDD_30_31		0x00040000	/* VDD voltage 3.0 ~ 3.1 */
#define MMC_VDD_31_32		0x00080000	/* VDD voltage 3.1 ~ 3.2 */
#define MMC_VDD_32_33		0x00100000	/* VDD voltage 3.2 ~ 3.3 */
#define MMC_VDD_33_34		0x00200000	/* VDD voltage 3.3 ~ 3.4 */
#define MMC_VDD_34_35		0x00400000	/* VDD voltage 3.4 ~ 3.5 */
#define MMC_VDD_35_36		0x00800000	/* VDD voltage 3.5 ~ 3.6 */

	u32			caps;		/* Host capabilities */

#define MMC_CAP_4_BIT_DATA	(1 << 0)	/* Can the host do 4 bit transfers */
#define MMC_CAP_MMC_HIGHSPEED	(1 << 1)	/* Can do MMC high-speed timing */
#define MMC_CAP_SD_HIGHSPEED	(1 << 2)	/* Can do SD high-speed timing */
#define MMC_CAP_SDIO_IRQ	(1 << 3)	/* Can signal pending SDIO IRQs */
#define MMC_CAP_SPI		(1 << 4)	/* Talks only SPI protocols */
#define MMC_CAP_NEEDS_POLL	(1 << 5)	/* Needs polling for card-detection */
#define MMC_CAP_8_BIT_DATA	(1 << 6)	/* Can the host do 8 bit transfers */
#define MMC_CAP_AGGRESSIVE_PM	(1 << 7)	/* Suspend (e)MMC/SD at idle  */
#define MMC_CAP_NONREMOVABLE	(1 << 8)	/* Nonremovable e.g. eMMC */
#define MMC_CAP_WAIT_WHILE_BUSY	(1 << 9)	/* Waits while card is busy */
#define MMC_CAP_ERASE		(1 << 10)	/* Allow erase/trim commands */
#define MMC_CAP_1_8V_DDR	(1 << 11)	/* can support */
						/* DDR mode at 1.8V */
#define MMC_CAP_1_2V_DDR	(1 << 12)	/* can support */
						/* DDR mode at 1.2V */
#define MMC_CAP_POWER_OFF_CARD	(1 << 13)	/* Can power off after boot */
#define MMC_CAP_BUS_WIDTH_TEST	(1 << 14)	/* CMD14/CMD19 bus width ok */
#define MMC_CAP_UHS_SDR12	(1 << 15)	/* Host supports UHS SDR12 mode */
#define MMC_CAP_UHS_SDR25	(1 << 16)	/* Host supports UHS SDR25 mode */
#define MMC_CAP_UHS_SDR50	(1 << 17)	/* Host supports UHS SDR50 mode */
#define MMC_CAP_UHS_SDR104	(1 << 18)	/* Host supports UHS SDR104 mode */
#define MMC_CAP_UHS_DDR50	(1 << 19)	/* Host supports UHS DDR50 mode */
#define MMC_CAP_RUNTIME_RESUME	(1 << 20)	/* Resume at runtime_resume. */
#define MMC_CAP_DRIVER_TYPE_A	(1 << 23)	/* Host supports Driver Type A */
#define MMC_CAP_DRIVER_TYPE_C	(1 << 24)	/* Host supports Driver Type C */
#define MMC_CAP_DRIVER_TYPE_D	(1 << 25)	/* Host supports Driver Type D */
#define MMC_CAP_CMD23		(1 << 30)	/* CMD23 supported. */
#define MMC_CAP_HW_RESET	(1 << 31)	/* Hardware reset */

	u32			caps2;		/* More host capabilities */

#define MMC_CAP2_BOOTPART_NOACC	(1 << 0)	/* Boot partition no access */
#define MMC_CAP2_FULL_PWR_CYCLE	(1 << 2)	/* Can do full power cycle */
#define MMC_CAP2_HS200_1_8V_SDR	(1 << 5)        /* can support */
#define MMC_CAP2_HS200_1_2V_SDR	(1 << 6)        /* can support */
#define MMC_CAP2_HS200		(MMC_CAP2_HS200_1_8V_SDR | \
				 MMC_CAP2_HS200_1_2V_SDR)
#define MMC_CAP2_HC_ERASE_SZ	(1 << 9)	/* High-capacity erase size */
#define MMC_CAP2_CD_ACTIVE_HIGH	(1 << 10)	/* Card-detect signal active high */
#define MMC_CAP2_RO_ACTIVE_HIGH	(1 << 11)	/* Write-protect signal active high */
#define MMC_CAP2_PACKED_RD	(1 << 12)	/* Allow packed read */
#define MMC_CAP2_PACKED_WR	(1 << 13)	/* Allow packed write */
#define MMC_CAP2_PACKED_CMD	(MMC_CAP2_PACKED_RD | \
				 MMC_CAP2_PACKED_WR)
#define MMC_CAP2_NO_PRESCAN_POWERUP (1 << 14)	/* Don't power up before scan */
#define MMC_CAP2_HS400_1_8V	(1 << 15)	/* Can support HS400 1.8V */
#define MMC_CAP2_HS400_1_2V	(1 << 16)	/* Can support HS400 1.2V */
#define MMC_CAP2_HS400		(MMC_CAP2_HS400_1_8V | \
				 MMC_CAP2_HS400_1_2V)
#define MMC_CAP2_SDIO_IRQ_NOTHREAD (1 << 17)
#define MMC_CAP2_PACKED_WR_CONTROL (1 << 18)	/* Allow write packing control */
#define MMC_CAP2_CLK_SCALE	(1 << 19)	/* Allow dynamic clk scaling */
/* Allows Asynchronous SDIO irq while card is in 4-bit mode */
#define MMC_CAP2_ASYNC_SDIO_IRQ_4BIT_MODE (1 << 20)
/* Some hosts need additional tuning */
#define MMC_CAP2_HS400_POST_TUNING	(1 << 21)
#define MMC_CAP2_NONHOTPLUG	(1 << 25)	/*Don't support hotplug*/
#define MMC_CAP2_CMD_QUEUE	(1 << 26)	/* support eMMC command queue */
#define MMC_CAP2_SANITIZE       (1 << 27)               /* Support Sanitize */
#define MMC_CAP2_SLEEP_AWAKE	(1 << 28)	/* Use Sleep/Awake (CMD5) */
/* use max discard ignoring max_busy_timeout parameter */
#define MMC_CAP2_MAX_DISCARD_SIZE	(1 << 29)

	mmc_pm_flag_t		pm_caps;	/* supported pm features */

#ifdef CONFIG_MMC_CLKGATE
	int			clk_requests;	/* internal reference counter */
	unsigned int		clk_delay;	/* number of MCI clk hold cycles */
	bool			clk_gated;	/* clock gated */
	struct delayed_work	clk_gate_work; /* delayed clock gate */
	unsigned int		clk_old;	/* old clock value cache */
	spinlock_t		clk_lock;	/* lock for clk fields */
	struct mutex		clk_gate_mutex;	/* mutex for clock gating */
	struct device_attribute clkgate_delay_attr;
	unsigned long           clkgate_delay;
#endif

	/* host specific block data */
	unsigned int		max_seg_size;	/* see blk_queue_max_segment_size */
	unsigned short		max_segs;	/* see blk_queue_max_segments */
	unsigned short		unused;
	unsigned int		max_req_size;	/* maximum number of bytes in one req */
	unsigned int		max_blk_size;	/* maximum size of one mmc block */
	unsigned int		max_blk_count;	/* maximum number of blocks in one req */
	unsigned int		max_busy_timeout; /* max busy timeout in ms */

	/* private data */
	spinlock_t		lock;		/* lock for claim and bus ops */

	struct mmc_ios		ios;		/* current io bus settings */
	struct mmc_ios		cached_ios;

	/* group bitfields together to minimize padding */
	unsigned int		use_spi_crc:1;
	unsigned int		claimed:1;	/* host exclusively claimed */
	unsigned int		bus_dead:1;	/* bus has been released */
#ifdef CONFIG_MMC_DEBUG
	unsigned int		removed:1;	/* host is being removed */
#endif

	int			rescan_disable;	/* disable card detection */
	int			rescan_entered;	/* used with nonremovable devices */

	bool			trigger_card_event; /* card_event necessary */

	struct mmc_card		*card;		/* device attached to this host */

	wait_queue_head_t	wq;
	struct task_struct	*claimer;	/* task that has host claimed */
	struct task_struct	*suspend_task;
	int			claim_cnt;	/* "claim" nesting count */

	struct delayed_work	detect;
	int			detect_change;	/* card detect flag */
	struct mmc_slot		slot;

	const struct mmc_bus_ops *bus_ops;	/* current bus driver */
	unsigned int		bus_refs;	/* reference counter */

	unsigned int		bus_resume_flags;
#define MMC_BUSRESUME_MANUAL_RESUME	(1 << 0)
#define MMC_BUSRESUME_NEEDS_RESUME	(1 << 1)

	unsigned int		sdio_irqs;
	struct task_struct	*sdio_irq_thread;
	bool			sdio_irq_pending;
	atomic_t		sdio_irq_thread_abort;

	mmc_pm_flag_t		pm_flags;	/* requested pm features */

	struct led_trigger	*led;		/* activity led */

#ifdef CONFIG_REGULATOR
	bool			regulator_enabled; /* regulator state */
#endif
	struct mmc_supply	supply;

	struct dentry		*debugfs_root;

	struct mmc_async_req	*areq;		/* active async req */
	struct mmc_context_info	context_info;	/* async synchronization info */

#ifdef CONFIG_FAIL_MMC_REQUEST
	struct fault_attr	fail_mmc_request;
#endif

	unsigned int		actual_clock;	/* Actual HC clock rate */

	unsigned int		slotno;	/* used for sdio acpi binding */

	int			dsr_req;	/* DSR value is valid */
	u32			dsr;	/* optional driver stage (DSR) value */

#ifdef CONFIG_MMC_EMBEDDED_SDIO
	struct {
		struct sdio_cis			*cis;
		struct sdio_cccr		*cccr;
		struct sdio_embedded_func	*funcs;
		int				num_funcs;
	} embedded_sdio_data;
#endif

	/*
	 * Set to 1 to just stop the SDCLK to the card without
	 * actually disabling the clock from it's source.
	 */
	bool			card_clock_off;

#ifdef CONFIG_MMC_PERF_PROFILING
	struct {

		unsigned long rbytes_drv;  /* Rd bytes MMC Host  */
		unsigned long wbytes_drv;  /* Wr bytes MMC Host  */
		ktime_t rtime_drv;	   /* Rd time  MMC Host  */
		ktime_t wtime_drv;	   /* Wr time  MMC Host  */
		ktime_t start;
	} perf;
	bool perf_enable;
#endif
	enum dev_state dev_status;
	bool			wakeup_on_idle;
	struct mmc_cmdq_context_info	cmdq_ctx;
	int num_cq_slots;
	int dcmd_cq_slot;
	u32			cmdq_thist_enabled;
	/*
	 * several cmdq supporting host controllers are extensions
	 * of legacy controllers. This variable can be used to store
	 * a reference to the cmdq extension of the existing host
	 * controller.
	 */
	void *cmdq_private;
	struct mmc_request	*err_mrq;
	unsigned long		private[0] ____cacheline_aligned;
};

struct mmc_host *mmc_alloc_host(int extra, struct device *);
extern bool mmc_host_may_gate_card(struct mmc_card *);
int mmc_add_host(struct mmc_host *);
void mmc_remove_host(struct mmc_host *);
void mmc_free_host(struct mmc_host *);
int mmc_of_parse(struct mmc_host *host);

#ifdef CONFIG_MMC_EMBEDDED_SDIO
extern void mmc_set_embedded_sdio_data(struct mmc_host *host,
				       struct sdio_cis *cis,
				       struct sdio_cccr *cccr,
				       struct sdio_embedded_func *funcs,
				       int num_funcs);
#endif

static inline void *mmc_priv(struct mmc_host *host)
{
	return (void *)host->private;
}

static inline void *mmc_cmdq_private(struct mmc_host *host)
{
	return host->cmdq_private;
}

#define mmc_host_is_spi(host)	((host)->caps & MMC_CAP_SPI)

#define mmc_dev(x)	((x)->parent)
#define mmc_classdev(x)	(&(x)->class_dev)
#define mmc_hostname(x)	(dev_name(&(x)->class_dev))
#define mmc_bus_needs_resume(host) ((host)->bus_resume_flags & \
				    MMC_BUSRESUME_NEEDS_RESUME)
#define mmc_bus_manual_resume(host) ((host)->bus_resume_flags & \
				MMC_BUSRESUME_MANUAL_RESUME)

static inline void mmc_set_bus_resume_policy(struct mmc_host *host, int manual)
{
	if (manual)
		host->bus_resume_flags |= MMC_BUSRESUME_MANUAL_RESUME;
	else
		host->bus_resume_flags &= ~MMC_BUSRESUME_MANUAL_RESUME;
}

extern int mmc_resume_bus(struct mmc_host *host);

int mmc_power_save_host(struct mmc_host *host);
int mmc_power_restore_host(struct mmc_host *host);

void mmc_detect_change(struct mmc_host *, unsigned long delay);
void mmc_request_done(struct mmc_host *, struct mmc_request *);

int mmc_cache_ctrl(struct mmc_host *, u8);

static inline void mmc_signal_sdio_irq(struct mmc_host *host)
{
	host->ops->enable_sdio_irq(host, 0);
	host->sdio_irq_pending = true;
	wake_up_process(host->sdio_irq_thread);
}

void sdio_run_irqs(struct mmc_host *host);

#ifdef CONFIG_REGULATOR
int mmc_regulator_get_ocrmask(struct regulator *supply);
int mmc_regulator_set_ocr(struct mmc_host *mmc,
			struct regulator *supply,
			unsigned short vdd_bit);
#else
static inline int mmc_regulator_get_ocrmask(struct regulator *supply)
{
	return 0;
}

static inline int mmc_regulator_set_ocr(struct mmc_host *mmc,
				 struct regulator *supply,
				 unsigned short vdd_bit)
{
	return 0;
}
#endif

int mmc_regulator_get_supply(struct mmc_host *mmc);

int mmc_pm_notify(struct notifier_block *notify_block, unsigned long, void *);

static inline int mmc_card_is_removable(struct mmc_host *host)
{
	return !(host->caps & MMC_CAP_NONREMOVABLE);
}

static inline int mmc_card_keep_power(struct mmc_host *host)
{
	return host->pm_flags & MMC_PM_KEEP_POWER;
}

static inline int mmc_card_wake_sdio_irq(struct mmc_host *host)
{
	return host->pm_flags & MMC_PM_WAKE_SDIO_IRQ;
}

static inline int mmc_host_cmd23(struct mmc_host *host)
{
	return host->caps & MMC_CAP_CMD23;
}

static inline int mmc_boot_partition_access(struct mmc_host *host)
{
	return !(host->caps2 & MMC_CAP2_BOOTPART_NOACC);
}

static inline bool mmc_card_and_host_support_async_int(struct mmc_host *host)
{
	return ((host->caps2 & MMC_CAP2_ASYNC_SDIO_IRQ_4BIT_MODE) &&
			(host->card->cccr.async_intr_sup));
}

static inline int mmc_host_uhs(struct mmc_host *host)
{
	return host->caps &
		(MMC_CAP_UHS_SDR12 | MMC_CAP_UHS_SDR25 |
		 MMC_CAP_UHS_SDR50 | MMC_CAP_UHS_SDR104 |
		 MMC_CAP_UHS_DDR50);
}

static inline int mmc_host_packed_wr(struct mmc_host *host)
{
	return host->caps2 & MMC_CAP2_PACKED_WR;
}

static inline void mmc_host_set_halt(struct mmc_host *host)
{
	set_bit(CMDQ_STATE_HALT, &host->cmdq_ctx.curr_state);
}

static inline void mmc_host_clr_halt(struct mmc_host *host)
{
	clear_bit(CMDQ_STATE_HALT, &host->cmdq_ctx.curr_state);
}

static inline int mmc_host_halt(struct mmc_host *host)
{
	return test_bit(CMDQ_STATE_HALT, &host->cmdq_ctx.curr_state);
}

static inline void mmc_host_set_cq_disable(struct mmc_host *host)
{
	set_bit(CMDQ_STATE_CQ_DISABLE, &host->cmdq_ctx.curr_state);
}

static inline void mmc_host_clr_cq_disable(struct mmc_host *host)
{
	clear_bit(CMDQ_STATE_CQ_DISABLE, &host->cmdq_ctx.curr_state);
}

static inline int mmc_host_cq_disable(struct mmc_host *host)
{
	return test_bit(CMDQ_STATE_CQ_DISABLE, &host->cmdq_ctx.curr_state);
}

#ifdef CONFIG_MMC_CLKGATE
void mmc_host_clk_hold(struct mmc_host *host);
void mmc_host_clk_release(struct mmc_host *host);
unsigned int mmc_host_clk_rate(struct mmc_host *host);

#else
static inline void mmc_host_clk_hold(struct mmc_host *host)
{
}

static inline void mmc_host_clk_release(struct mmc_host *host)
{
}

static inline unsigned int mmc_host_clk_rate(struct mmc_host *host)
{
	return host->ios.clock;
}
#endif

static inline int mmc_card_hs(struct mmc_card *card)
{
	return card->host->ios.timing == MMC_TIMING_SD_HS ||
		card->host->ios.timing == MMC_TIMING_MMC_HS;
}

static inline int mmc_card_uhs(struct mmc_card *card)
{
	return card->host->ios.timing >= MMC_TIMING_UHS_SDR12 &&
		card->host->ios.timing <= MMC_TIMING_UHS_DDR50;
}

static inline bool mmc_card_hs200(struct mmc_card *card)
{
	return card->host->ios.timing == MMC_TIMING_MMC_HS200;
}

static inline bool mmc_card_ddr52(struct mmc_card *card)
{
	return card->host->ios.timing == MMC_TIMING_MMC_DDR52;
}

static inline bool mmc_card_hs400(struct mmc_card *card)
{
	return card->host->ios.timing == MMC_TIMING_MMC_HS400;
}

#endif /* LINUX_MMC_HOST_H */
