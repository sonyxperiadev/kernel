/* include/linux/usb/msm_hsusb.h
 *
 * Copyright (C) 2008 Google, Inc.
 * Author: Brian Swetland <swetland@google.com>
 * Copyright (c) 2009-2015, The Linux Foundation. All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef __ASM_ARCH_MSM_HSUSB_H
#define __ASM_ARCH_MSM_HSUSB_H

#include <linux/types.h>
#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>
#include <linux/usb/otg.h>
#include <linux/wakelock.h>
#include <linux/pm_qos.h>
#include <linux/hrtimer.h>
#include <linux/power_supply.h>
#include <linux/cdev.h>
/*
 * The following are bit fields describing the usb_request.udc_priv word.
 * These bit fields are set by function drivers that wish to queue
 * usb_requests with sps/bam parameters.
 */
#define MSM_PIPE_ID_MASK		(0x1F)
#define MSM_TX_PIPE_ID_OFS		(16)
#define MSM_SPS_MODE			BIT(5)
#define MSM_IS_FINITE_TRANSFER		BIT(6)
#define MSM_PRODUCER			BIT(7)
#define MSM_DISABLE_WB			BIT(8)
#define MSM_ETD_IOC			BIT(9)
#define MSM_INTERNAL_MEM		BIT(10)
#define MSM_VENDOR_ID			BIT(16)

/**
 * Requested USB votes for BUS bandwidth
 *
 * USB_NO_PERF_VOTE     BUS Vote for inactive USB session or disconnect
 * USB_MAX_PERF_VOTE    Maximum BUS bandwidth vote
 * USB_MIN_PERF_VOTE    Minimum BUS bandwidth vote (for some hw same as NO_PERF)
 *
 */
enum usb_bus_vote {
	USB_NO_PERF_VOTE = 0,
	USB_MAX_PERF_VOTE,
	USB_MIN_PERF_VOTE,
};

/**
 * Supported USB modes
 *
 * USB_PERIPHERAL       Only peripheral mode is supported.
 * USB_HOST             Only host mode is supported.
 * USB_OTG              OTG mode is supported.
 *
 */
enum usb_mode_type {
	USB_NONE = 0,
	USB_PERIPHERAL,
	USB_HOST,
	USB_OTG,
};

/**
 * OTG control
 *
 * OTG_NO_CONTROL	Id/VBUS notifications not required. Useful in host
 *                      only configuration.
 * OTG_PHY_CONTROL	Id/VBUS notifications comes form USB PHY.
 * OTG_PMIC_CONTROL	Id/VBUS notifications comes from PMIC hardware.
 * OTG_USER_CONTROL	Id/VBUS notifcations comes from User via sysfs.
 *
 */
enum otg_control_type {
	OTG_NO_CONTROL = 0,
	OTG_PHY_CONTROL,
	OTG_PMIC_CONTROL,
	OTG_USER_CONTROL,
};

/**
 * PHY used in
 *
 * INVALID_PHY			Unsupported PHY
 * CI_PHY			Chipidea PHY
 * SNPS_PICO_PHY		Synopsis Pico PHY
 * SNPS_FEMTO_PHY		Synopsis Femto PHY
 *
 */
enum msm_usb_phy_type {
	INVALID_PHY = 0,
	CI_PHY,			/* not supported */
	SNPS_PICO_PHY,
	SNPS_FEMTO_PHY,
	QUSB_ULPI_PHY,
};

#define IDEV_CHG_MAX	1500
#define IDEV_CHG_MIN	500
#define IUNIT		100

#define IDEV_ACA_CHG_MAX	1500
#define IDEV_ACA_CHG_LIMIT	500
#define IDEV_HVDCP_CHG_MAX	1800

/**
 * Different states involved in USB charger detection.
 *
 * USB_CHG_STATE_UNDEFINED	USB charger is not connected or detection
 *                              process is not yet started.
 * USB_CHG_STATE_WAIT_FOR_DCD	Waiting for Data pins contact.
 * USB_CHG_STATE_DCD_DONE	Data pin contact is detected.
 * USB_CHG_STATE_PRIMARY_DONE	Primary detection is completed (Detects
 *                              between SDP and DCP/CDP).
 * USB_CHG_STATE_SECONDARY_DONE	Secondary detection is completed (Detects
 *                              between DCP and CDP).
 * USB_CHG_STATE_DETECTED	USB charger type is determined.
 *
 */
enum usb_chg_state {
	USB_CHG_STATE_UNDEFINED = 0,
	USB_CHG_STATE_WAIT_FOR_DCD,
	USB_CHG_STATE_DCD_DONE,
	USB_CHG_STATE_PRIMARY_DONE,
	USB_CHG_STATE_SECONDARY_DONE,
	USB_CHG_STATE_DETECTED,
};

/**
 * USB charger types
 *
 * USB_INVALID_CHARGER	Invalid USB charger.
 * USB_SDP_CHARGER	Standard downstream port. Refers to a downstream port
 *                      on USB2.0 compliant host/hub.
 * USB_DCP_CHARGER	Dedicated charger port (AC charger/ Wall charger).
 * USB_CDP_CHARGER	Charging downstream port. Enumeration can happen and
 *                      IDEV_CHG_MAX can be drawn irrespective of USB state.
 * USB_ACA_A_CHARGER	B-device is connected on accessory port with charger
 *                      connected on charging port. This configuration allows
 *                      charging in host mode.
 * USB_ACA_B_CHARGER	No device (or A-device without VBUS) is connected on
 *                      accessory port with charger connected on charging port.
 * USB_ACA_C_CHARGER	A-device (with VBUS) is connected on
 *                      accessory port with charger connected on charging port.
 * USB_ACA_DOCK_CHARGER	A docking station that has one upstream port and one
 *			or more downstream ports. Capable of supplying
 *			IDEV_CHG_MAX irrespective of devices connected on
 *			accessory ports.
 * USB_PROPRIETARY_CHARGER A proprietary charger pull DP and DM to specific
 *			voltages between 2.0-3.3v for identification.
 *
 */
enum usb_chg_type {
	USB_INVALID_CHARGER = 0,
	USB_SDP_CHARGER,
	USB_DCP_CHARGER,
	USB_CDP_CHARGER,
	USB_ACA_A_CHARGER,
	USB_ACA_B_CHARGER,
	USB_ACA_C_CHARGER,
	USB_ACA_DOCK_CHARGER,
	USB_PROPRIETARY_CHARGER,
	USB_FLOATED_CHARGER,
};

/**
 * Used different VDDCX voltage voting mechnism
 * VDDCX_CORNER       Vote for VDDCX Corner voltage
 * VDDCX              Vote for VDDCX Absolute voltage
 */
enum usb_vdd_type {
	VDDCX_CORNER = 0,
	VDDCX,
	VDD_TYPE_MAX,
};

/**
 * Used different VDDCX voltage values
 */
enum usb_vdd_value {
	VDD_NONE = 0,
	VDD_MIN,
	VDD_MAX,
	VDD_VAL_MAX,
};

/**
 * Maintain state for hvdcp external charger status
 * DEFAULT	This is used when DCP is detected
 * ACTIVE	This is used when ioctl is called to block LPM
 * INACTIVE	This is used when ioctl is called to unblock LPM
 */

enum usb_ext_chg_status {
	DEFAULT = 1,
	ACTIVE,
	INACTIVE,
};

/**
 * Supported USB controllers
 */
enum usb_ctrl {
	DWC3_CTRL = 0,	/* DWC3 controller */
	CI_CTRL,	/* ChipIdea controller */
	HSIC_CTRL,	/* HSIC controller */
	NUM_CTRL,
};

/**
 * USB ID state
 */
enum usb_id_state {
	USB_ID_GROUND = 0,
	USB_ID_FLOAT,
};

/**
 * struct msm_otg_platform_data - platform device data
 *              for msm_otg driver.
 * @phy_init_seq: PHY configuration sequence. val, reg pairs
 *              terminated by -1.
 * @vbus_power: VBUS power on/off routine.It should return result
 *		as success(zero value) or failure(non-zero value).
 * @power_budget: VBUS power budget in mA (0 will be treated as 500mA).
 * @mode: Supported mode (OTG/peripheral/host).
 * @otg_control: OTG switch controlled by user/Id pin
 * @default_mode: Default operational mode. Applicable only if
 *              OTG switch is controller by user.
 * @pmic_id_irq: IRQ number assigned for PMIC USB ID line.
 * @mpm_otgsessvld_int: MPM wakeup pin assigned for OTG SESSVLD
 *              interrupt. Used when .otg_control == OTG_PHY_CONTROL.
 * @mpm_dpshv_int: MPM wakeup pin assigned for DP SHV interrupt.
 *		Used during host bus suspend.
 * @mpm_dmshv_int: MPM wakeup pin assigned for DM SHV interrupt.
 *		Used during host bus suspend.
 * @mhl_enable: indicates MHL connector or not.
 * @disable_reset_on_disconnect: perform USB PHY and LINK reset
 *              on USB cable disconnection.
 * @pnoc_errata_fix: workaround needed for PNOC hardware bug that
 *              affects USB performance.
 * @enable_lpm_on_suspend: Enable the USB core to go into Low
 *              Power Mode, when USB bus is suspended but cable
 *              is connected.
 * @core_clk_always_on_workaround: Don't disable core_clk when
 *              USB enters LPM.
 * @delay_lpm_on_disconnect: Use a delay before entering LPM
 *              upon USB cable disconnection.
 * @enable_sec_phy: Use second HSPHY with USB2 core
 * @bus_scale_table: parameters for bus bandwidth requirements
 * @mhl_dev_name: MHL device name used to register with MHL driver.
 * @log2_itc: value of 2^(log2_itc-1) will be used as the
 *              interrupt threshold (ITC), when log2_itc is
 *              between 1 to 7.
 * @l1_supported: enable link power management support.
 * @dpdm_pulldown_added: Indicates whether pull down resistors are
 *		connected on data lines or not.
 * @vddmin_gpio: dedictaed gpio in the platform that is used for
 *		pullup the D+ line in case of bus suspend with
 *		phy retention.
 * @rw_during_lpm_workaround: Determines whether remote-wakeup
 *		during low-power mode workaround will be
 *		applied.
 * @enable_ahb2ahb_bypass: Indicates whether enable AHB2AHB BYPASS
 *		mode with controller in device mode.
 * @bool disable_retention_with_vdd_min: Indicates whether to enable
		allowing VDDmin without putting PHY into retention.
 * @usb_id_gpio: Gpio used for USB ID detection.
 * @hub_reset_gpio: Gpio used for hub reset.
 * @switch_sel_gpio: Gpio used for controlling switch that
		routing D+/D- from the USB HUB to the USB jack type B
		for peripheral mode.
 * @bool phy_dvdd_always_on: PHY DVDD is supplied by always on PMIC LDO.
 * @bool emulation: Indicates whether we are running on emulation platform.
 * @bool enable_streaming: Indicates whether streaming to be enabled by default.
 * @bool enable_axi_prefetch: Indicates whether AXI Prefetch interface is used
		for improving data performance.
 */
struct msm_otg_platform_data {
	int *phy_init_seq;
	int (*vbus_power)(bool on);
	unsigned power_budget;
	enum usb_mode_type mode;
	enum otg_control_type otg_control;
	enum usb_mode_type default_mode;
	enum msm_usb_phy_type phy_type;
	int pmic_id_irq;
	unsigned int mpm_otgsessvld_int;
	unsigned int mpm_dpshv_int;
	unsigned int mpm_dmshv_int;
	bool mhl_enable;
	bool disable_reset_on_disconnect;
	bool pnoc_errata_fix;
	bool enable_lpm_on_dev_suspend;
	bool core_clk_always_on_workaround;
	bool delay_lpm_on_disconnect;
	bool dp_manual_pullup;
	bool enable_sec_phy;
	struct msm_bus_scale_pdata *bus_scale_table;
	const char *mhl_dev_name;
	int log2_itc;
	bool l1_supported;
	bool dpdm_pulldown_added;
	int vddmin_gpio;
	bool rw_during_lpm_workaround;
	bool enable_ahb2ahb_bypass;
	bool disable_retention_with_vdd_min;
	int usb_id_gpio;
	int hub_reset_gpio;
	int switch_sel_gpio;
	bool phy_dvdd_always_on;
	bool emulation;
	bool enable_streaming;
	bool enable_axi_prefetch;
	struct clk *system_clk;
	struct clk *pclk;
};

/* phy related flags */
#define ENABLE_DP_MANUAL_PULLUP		BIT(0)
#define ENABLE_SECONDARY_PHY		BIT(1)
#define PHY_HOST_MODE			BIT(2)
#define PHY_CHARGER_CONNECTED		BIT(3)
#define PHY_VBUS_VALID_OVERRIDE		BIT(4)

/* Timeout (in msec) values (min - max) associated with OTG timers */

#define TA_WAIT_VRISE	100	/* ( - 100)  */
#define TA_WAIT_VFALL	500	/* ( - 1000) */

/*
 * This option is set for embedded hosts or OTG devices in which leakage
 * currents are very minimal.
 */
#ifdef CONFIG_USB_OTG
#define TA_WAIT_BCON	30000	/* (1100 - 30000) */
#else
#define TA_WAIT_BCON	-1
#endif

#define TA_AIDL_BDIS	500	/* (200 - ) */
#define TA_BIDL_ADIS	155	/* (155 - 200) */
#define TB_SRP_FAIL	6000	/* (5000 - 6000) */
#define TB_ASE0_BRST	200	/* (155 - ) */

/* TB_SSEND_SRP and TB_SE0_SRP are combined */
#define TB_SRP_INIT	2000	/* (1500 - ) */

#define TA_TST_MAINT	10100	/* (9900 - 10100) */
#define TB_TST_SRP	3000	/* ( - 5000) */
#define TB_TST_CONFIG	300

/* Timeout variables */

#define A_WAIT_VRISE	0
#define A_WAIT_VFALL	1
#define A_WAIT_BCON	2
#define A_AIDL_BDIS	3
#define A_BIDL_ADIS	4
#define B_SRP_FAIL	5
#define B_ASE0_BRST	6
#define A_TST_MAINT	7
#define B_TST_SRP	8
#define B_TST_CONFIG	9

#define USB_NUM_BUS_CLOCKS      3

/**
 * struct msm_otg: OTG driver data. Shared by HCD and DCD.
 * @otg: USB OTG Transceiver structure.
 * @pdata: otg device platform data.
 * @irq: IRQ number assigned for HSUSB controller.
 * @async_irq: IRQ number used by some controllers during low power state
 * @phy_irq: IRQ number assigned for PHY to notify events like id and line
		state changes.
 * @pclk: clock struct of iface_clk.
 * @core_clk: clock struct of core_bus_clk.
 * @sleep_clk: clock struct of sleep_clk for USB PHY.
 * @phy_reset_clk: clock struct of phy_reset_clk for USB PHY. This clock is
		a reset only clock and resets the PHY, ULPI bridge and
		CSR wrapper.
 * @phy_por_clk: clock struct of phy_por_clk for USB PHY. This clock is
		a reset only clock and resets only the PHY (POR).
 * @phy_csr_clk: clock struct of phy_csr_clk for USB PHY. This clock is
		required to acess PHY CSR registers via AHB2PHY interface.
 * @bus_clks: bimc/snoc/pcnoc clock struct.
 * @core_clk_rate: core clk max frequency
 * @regs: ioremapped register base address.
 * @usb_phy_ctrl_reg: relevant PHY_CTRL_REG register base address.
 * @inputs: OTG state machine inputs(Id, SessValid etc).
 * @sm_work: OTG state machine work.
 * @pm_suspended: OTG device is system(PM) suspended.
 * @pm_notify: Notifier to receive system wide PM transition events.
		It is used to defer wakeup events processing until
		system is RESUMED.
 * @in_lpm: indicates low power mode (LPM) state.
 * @async_int: IRQ line on which ASYNC interrupt arrived in LPM.
 * @cur_power: The amount of mA available from downstream port.
 * @otg_wq: Strict order otg workqueue for OTG works (SM/ID/SUSPEND).
 * @chg_work: Charger detection work.
 * @chg_state: The state of charger detection process.
 * @chg_type: The type of charger attached.
 * @dcd_retires: The retry count used to track Data contact
 *               detection process.
 * @wlock: Wake lock struct to prevent system suspend when
 *               USB is active.
 * @usbdev_nb: The notifier block used to know about the B-device
 *             connected. Useful only when ACA_A charger is
 *             connected.
 * @mA_port: The amount of current drawn by the attached B-device.
 * @id_timer: The timer used for polling ID line to detect ACA states.
 * @xo_handle: TCXO buffer handle
 * @bus_perf_client: Bus performance client handle to request BUS bandwidth
 * @mhl_enabled: MHL driver registration successful and MHL enabled.
 * @host_bus_suspend: indicates host bus suspend or not.
 * @device_bus_suspend: indicates device bus suspend or not.
 * @bus_clks_enabled: indicates pcnoc/snoc/bimc clocks are on or not.
 * @chg_check_timer: The timer used to implement the workaround to detect
 *               very slow plug in of wall charger.
 * @bc1p2_current_max: Max charging current allowed as per bc1.2 chg detection
 * @typec_current_max: Max charging current allowed as per type-c chg detection
 * @is_ext_chg_dcp: To indicate whether charger detected by external entity
		SMB hardware is DCP charger or not.
 * @pm_done: It is used to increment the pm counter using pm_runtime_get_sync.
	     This handles the race case when PM resume thread returns before
	     the charger detection starts. When USB is disconnected and in lpm
	     pm_done is set to true.
 * @ext_id_irq: IRQ for ID interrupt.
 * @phy_irq_pending: Gets set when PHY IRQ arrives in LPM.
 * host_suspend_wait: wait_queue on which USB core waits for USB entering lpm
	     in host bus suspend case.
 * @id_state: Indicates USBID line status.
 * @rm_pulldown: Indicates pulldown status on D+ and D- data lines.
 * @dbg_idx: Dynamic debug buffer Index.
 * @dbg_lock: Dynamic debug buffer Lock.
 * @buf: Dynamic Debug Buffer.
 * @max_nominal_system_clk_rate: max freq at which system clock can run in
		nominal mode.
 */
struct msm_otg {
	struct usb_phy phy;
	struct msm_otg_platform_data *pdata;
	int irq;
	int async_irq;
	int phy_irq;
	struct clk *xo_clk;
	struct clk *pclk;
	struct clk *core_clk;
	struct clk *sleep_clk;
	struct clk *phy_reset_clk;
	struct clk *phy_por_clk;
	struct clk *phy_csr_clk;
	struct clk *bus_clks[USB_NUM_BUS_CLOCKS];
	struct clk *phy_ref_clk;
	long core_clk_rate;
	struct resource *io_res;
	void __iomem *regs;
	void __iomem *phy_csr_regs;
	void __iomem *usb_phy_ctrl_reg;
#define ID		0
#define B_SESS_VLD	1
#define ID_A		2
#define ID_B		3
#define ID_C		4
#define A_BUS_DROP	5
#define A_BUS_REQ	6
#define A_SRP_DET	7
#define A_VBUS_VLD	8
#define B_CONN		9
#define ADP_CHANGE	10
#define POWER_UP	11
#define A_CLR_ERR	12
#define A_BUS_RESUME	13
#define A_BUS_SUSPEND	14
#define A_CONN		15
#define B_BUS_REQ	16
#define MHL	        17
#define B_FALSE_SDP	18
	unsigned long inputs;
	struct work_struct sm_work;
	bool sm_work_pending;
	atomic_t pm_suspended;
	struct notifier_block pm_notify;
	atomic_t in_lpm;
	atomic_t set_fpr_with_lpm_exit;
	bool err_event_seen;
	int async_int;
	unsigned cur_power;
	struct workqueue_struct *otg_wq;
	struct delayed_work chg_work;
	struct delayed_work id_status_work;
	struct delayed_work suspend_work;
	enum usb_chg_state chg_state;
	enum usb_chg_type chg_type;
	unsigned dcd_time;
	struct wake_lock wlock;
	struct notifier_block usbdev_nb;
	unsigned mA_port;
	struct timer_list id_timer;
	unsigned long caps;
	struct msm_xo_voter *xo_handle;
	uint32_t bus_perf_client;
	bool mhl_enabled;
	bool host_bus_suspend;
	bool device_bus_suspend;
	bool bus_clks_enabled;
	struct timer_list chg_check_timer;
	/*
	 * Allowing PHY power collpase turns off the HSUSB 3.3v and 1.8v
	 * analog regulators while going to low power mode.
	 * Currently only 28nm PHY has the support to allowing PHY
	 * power collapse since it doesn't have leakage currents while
	 * turning off the power rails.
	 */
#define ALLOW_PHY_POWER_COLLAPSE	BIT(0)
	/*
	 * Allow PHY RETENTION mode before turning off the digital
	 * voltage regulator(VDDCX).
	 */
#define ALLOW_PHY_RETENTION		BIT(1)
	  /*
	   * Allow putting the core in Low Power mode, when
	   * USB bus is suspended but cable is connected.
	   */
#define ALLOW_LPM_ON_DEV_SUSPEND	BIT(2)
	/*
	 * Allowing PHY regulators LPM puts the HSUSB 3.3v and 1.8v
	 * analog regulators into LPM while going to USB low power mode.
	 */
#define ALLOW_PHY_REGULATORS_LPM	BIT(3)
	/*
	 * Allow PHY RETENTION mode before turning off the digital
	 * voltage regulator(VDDCX) during host mode.
	 */
#define ALLOW_HOST_PHY_RETENTION	BIT(4)
	/*
	* Allow VDD minimization without putting PHY into retention
	* for fixing PHY current leakage issue when LDOs ar turned off.
	*/
#define ALLOW_VDD_MIN_WITH_RETENTION_DISABLED BIT(5)

	/*
	 * PHY can keep D+ pull-up during peripheral bus suspend and
	 * D+/D- pull-down during host bus suspend without any
	 * re-work. This is possible only when PHY DVDD is supplied
	 * by a PMIC LDO (unlike VDDCX/VDDMX).
	 */
#define ALLOW_BUS_SUSPEND_WITHOUT_REWORK BIT(6)
	unsigned long lpm_flags;
#define PHY_PWR_COLLAPSED		BIT(0)
#define PHY_RETENTIONED			BIT(1)
#define XO_SHUTDOWN			BIT(2)
#define CLOCKS_DOWN			BIT(3)
#define PHY_REGULATORS_LPM	BIT(4)
	int reset_counter;
	unsigned long b_last_se0_sess;
	unsigned long tmouts;
	u8 active_tmout;
	struct hrtimer timer;
	struct power_supply usb_psy;
	unsigned int online;
	unsigned int host_mode;
	unsigned int voltage_max;
	unsigned int current_max;
	unsigned int bc1p2_current_max;
	unsigned int typec_current_max;
	unsigned int usbin_health;

	dev_t ext_chg_dev;
	struct cdev ext_chg_cdev;
	struct class *ext_chg_class;
	struct device *ext_chg_device;
	bool ext_chg_opened;
	enum usb_ext_chg_status ext_chg_active;
	struct completion ext_chg_wait;
	struct pinctrl *phy_pinctrl;
	bool is_ext_chg_dcp;
	bool pm_done;
	struct qpnp_vadc_chip	*vadc_dev;
	int ext_id_irq;
	bool phy_irq_pending;
	wait_queue_head_t	host_suspend_wait;
	enum usb_id_state id_state;
	bool rm_pulldown;
/* Maximum debug message length */
#define DEBUG_MSG_LEN   128UL
/* Maximum number of messages */
#define DEBUG_MAX_MSG   256UL
	unsigned int dbg_idx;
	rwlock_t dbg_lock;
	char (buf[DEBUG_MAX_MSG])[DEBUG_MSG_LEN];   /* buffer */
	u32 max_nominal_system_clk_rate;
	unsigned int vbus_state;
};

struct ci13xxx_platform_data {
	u8 usb_core_id;
	/*
	 * value of 2^(log2_itc-1) will be used as the interrupt threshold
	 * (ITC), when log2_itc is between 1 to 7.
	 */
	int log2_itc;
	void *prv_data;
	bool l1_supported;
	bool enable_ahb2ahb_bypass;
	bool enable_streaming;
	struct clk *system_clk;
	struct clk *pclk;
	u32 max_nominal_system_clk_rate;
	u32 default_system_clk_rate;
	bool enable_axi_prefetch;
};

/**
 * struct msm_hsic_host_platform_data - platform device data
 *              for msm_hsic_host driver.
 * @phy_sof_workaround: Enable ALL PHY SOF bug related workarounds for
		SUSPEND, RESET and RESUME.
 * @phy_susp_sof_workaround: Enable PHY SOF workaround for
 *      SUSPEND.
 * @phy_reset_sof_workaround: Enable PHY SOF workaround for
 *      RESET.
 * @dis_internal_clk_gating: If set, internal clock gating in controller
 *		is disabled.
 *
 */
struct msm_hsic_host_platform_data {
	unsigned strobe;
	unsigned data;
	bool ignore_cal_pad_config;
	bool phy_sof_workaround;
	bool dis_internal_clk_gating;
	bool phy_susp_sof_workaround;
	bool phy_reset_sof_workaround;
	u32 reset_delay;
	int strobe_pad_offset;
	int data_pad_offset;

	struct msm_bus_scale_pdata *bus_scale_table;
	unsigned log2_irq_thresh;

	/* gpio used to resume peripheral */
	unsigned resume_gpio;

	/*swfi latency is required while driving resume on to the bus */
	u32 swfi_latency;

	/*standalone latency is required when HSCI is active*/
	u32 standalone_latency;
	bool pool_64_bit_align;
	bool enable_hbm;
	bool disable_park_mode;
	bool consider_ipa_handshake;
	bool ahb_async_bridge_bypass;
	bool disable_cerr;
};

struct msm_usb_host_platform_data {
	unsigned int power_budget;
	int pmic_gpio_dp_irq;
	unsigned int dock_connect_irq;
	bool use_sec_phy;
	bool no_selective_suspend;
	int resume_gpio;
	int ext_hub_reset_gpio;
	bool is_uicc;
	int pm_qos_latency;
};

/**
 * struct msm_hsic_peripheral_platform_data: HSIC peripheral
 * platform data.
 * @core_clk_always_on_workaround: Don't disable core_clk when
 *                                 HSIC enters LPM.
 */
struct msm_hsic_peripheral_platform_data {
	bool core_clk_always_on_workaround;
};

/**
 * struct usb_ext_notification: event notification structure
 * @notify: pointer to client function to call when ID event is detected.
 *          The function parameter is provided by driver to be called back when
 *          external client indicates it is done using the USB. This function
 *          should return 0 if handled successfully, otherise an error code.
 * @ctxt: client-specific context pointer
 *
 * This structure should be used by clients wishing to register (via
 * msm_register_usb_ext_notification) for event notification whenever a USB
 * cable is plugged in and ID pin status changes. Clients must provide a
 * callback function pointer. If this callback returns 0, the USB driver will
 * assume the client is "taking over" the connection, and will relinquish any
 * further processing until its callback (passed via the third parameter) is
 * called with the online parameter set to false.
 */
struct usb_ext_notification {
	int (*notify)(void *, int, void (*)(void *, int online), void *);
#ifdef CONFIG_SONY_USB_EXTENSIONS
	bool (*check_vbus)(void *);
	void (*vbus_control)(void *, bool);
#endif
	void *ctxt;
};
#ifdef CONFIG_USB_BAM
bool msm_bam_usb_lpm_ok(enum usb_ctrl ctrl);
void msm_bam_notify_lpm_resume(enum usb_ctrl ctrl);
void msm_bam_set_usb_host_dev(struct device *dev);
void msm_bam_set_hsic_host_dev(struct device *dev);
void msm_bam_set_usb_dev(struct device *dev);
void msm_bam_wait_for_usb_host_prod_granted(void);
void msm_bam_wait_for_hsic_host_prod_granted(void);
bool msm_bam_hsic_lpm_ok(void);
void msm_bam_usb_host_notify_on_resume(void);
void msm_bam_hsic_host_notify_on_resume(void);
bool msm_bam_hsic_host_pipe_empty(void);
void msm_bam_set_qdss_usb_active(bool is_active);
bool msm_usb_bam_enable(enum usb_ctrl ctrl, bool bam_enable);
#else
static inline bool msm_bam_usb_lpm_ok(enum usb_ctrl ctrl) { return true; }
static inline void msm_bam_notify_lpm_resume(enum usb_ctrl ctrl) {}
static inline void msm_bam_set_usb_host_dev(struct device *dev) {}
static inline void msm_bam_set_hsic_host_dev(struct device *dev) {}
static inline void msm_bam_set_usb_dev(struct device *dev) {}
static inline void msm_bam_wait_for_usb_host_prod_granted(void) {}
static inline void msm_bam_wait_for_hsic_host_prod_granted(void) {}
static inline bool msm_bam_hsic_lpm_ok(void) { return true; }
static inline void msm_bam_hsic_host_notify_on_resume(void) {}
static inline void msm_bam_usb_host_notify_on_resume(void) {}
static inline bool msm_bam_hsic_host_pipe_empty(void) { return true; }
static inline void msm_bam_set_qdss_usb_active(bool is_active) {}
static inline bool msm_usb_bam_enable(enum usb_ctrl ctrl, bool bam_enable)
{
	return true;
}
#endif
#ifdef CONFIG_USB_CI13XXX_MSM
void msm_hw_bam_disable(bool bam_disable);
void msm_usb_irq_disable(bool disable);
#else
static inline void msm_hw_bam_disable(bool bam_disable)
{
}
static inline void msm_usb_irq_disable(bool disable)
{
}
#endif

/* CONFIG_PM_RUNTIME */
#ifdef CONFIG_PM_RUNTIME
static inline int get_pm_runtime_counter(struct device *dev)
{
	return atomic_read(&dev->power.usage_count);
}
#else /* !CONFIG_PM_RUNTIME */
static inline int get_pm_runtime_counter(struct device *dev) { return -ENOSYS; }
#endif

#ifdef CONFIG_USB_DWC3_MSM
int msm_ep_config(struct usb_ep *ep);
int msm_ep_unconfig(struct usb_ep *ep);
void dwc3_tx_fifo_resize_request(struct usb_ep *ep, bool qdss_enable);
int msm_data_fifo_config(struct usb_ep *ep, phys_addr_t addr, u32 size,
	u8 dst_pipe_idx);
bool msm_dwc3_reset_ep_after_lpm(struct usb_gadget *gadget);
int msm_dwc3_reset_dbm_ep(struct usb_ep *ep);

void msm_dwc3_restart_usb_session(struct usb_gadget *gadget);

int msm_register_usb_ext_notification(struct usb_ext_notification *info);
#else
static inline int msm_data_fifo_config(struct usb_ep *ep, phys_addr_t addr,
	u32 size, u8 dst_pipe_idx)
{
	return -ENODEV;
}

static inline int msm_ep_config(struct usb_ep *ep)
{
	return -ENODEV;
}

static inline int msm_ep_unconfig(struct usb_ep *ep)
{
	return -ENODEV;
}

static inline void dwc3_tx_fifo_resize_request(
					struct usb_ep *ep, bool qdss_enable)
{
	return;
}

static inline void msm_dwc3_restart_usb_session(struct usb_gadget *gadget)
{
	return;
}

static inline int msm_register_usb_ext_notification(
					struct usb_ext_notification *info)
{
	return -ENODEV;
}

static inline bool msm_dwc3_reset_ep_after_lpm(struct usb_gadget *gadget)
{
	return false;
}

static inline int msm_dwc3_reset_dbm_ep(struct usb_ep *ep)
{
	return -ENODEV;
}

#endif
#endif
