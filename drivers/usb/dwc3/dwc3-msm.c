/* Copyright (c) 2012-2015, The Linux Foundation. All rights reserved.
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

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/cpu.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/pm_runtime.h>
#include <linux/ratelimit.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/list.h>
#include <linux/debugfs.h>
#include <linux/uaccess.h>
#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>
#include <linux/qpnp-misc.h>
#include <linux/usb/msm_hsusb.h>
#include <linux/usb/msm_ext_chg.h>
#include <linux/regulator/consumer.h>
#include <linux/pm_wakeup.h>
#include <linux/power_supply.h>
#include <linux/qpnp/qpnp-adc.h>
#include <linux/cdev.h>
#include <linux/completion.h>
#include <linux/clk/msm-clk.h>
#include <linux/msm-bus.h>
#include <linux/irq.h>
#include <soc/qcom/scm.h>

#ifdef CONFIG_SONY_USB_EXTENSIONS
#include <linux/gpio.h>
#include <linux/of_gpio.h>
 #ifdef CONFIG_QPNP_SMBCHARGER_EXTENSION
 #include <linux/qpnp/qpnp-smbcharger_extension.h>
 #endif
#endif

#include "dwc3_otg.h"
#include "core.h"
#include "gadget.h"
#include "dbm.h"
#include "debug.h"
#include "xhci.h"

/* cpu to fix usb interrupt */
static int cpu_to_affin;
module_param(cpu_to_affin, int, S_IRUGO|S_IWUSR);
MODULE_PARM_DESC(cpu_to_affin, "affin usb irq to this cpu");

/* ADC threshold values */
static int adc_low_threshold = 700;
module_param(adc_low_threshold, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(adc_low_threshold, "ADC ID Low voltage threshold");

static int adc_high_threshold = 950;
module_param(adc_high_threshold, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(adc_high_threshold, "ADC ID High voltage threshold");

static int adc_meas_interval = ADC_MEAS1_INTERVAL_1S;
module_param(adc_meas_interval, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(adc_meas_interval, "ADC ID polling period");

static int override_phy_init;
module_param(override_phy_init, int, S_IRUGO|S_IWUSR);
MODULE_PARM_DESC(override_phy_init, "Override HSPHY Init Seq");

/* Enable Proprietary charger detection */
static bool prop_chg_detect;
module_param(prop_chg_detect, bool, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(prop_chg_detect, "Enable Proprietary charger detection");

static bool usb_lpm_override;
module_param(usb_lpm_override, bool, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(usb_lpm_override, "Override no_suspend_resume with USB");

/* Max current to be drawn for HVDCP charger */
static int hvdcp_max_current = DWC3_HVDCP_CHG_MAX;
module_param(hvdcp_max_current, int, S_IRUGO|S_IWUSR);
MODULE_PARM_DESC(hvdcp_max_current, "max current drawn for HVDCP charger");

/* Max current to be drawn for DCP charger */
int dcp_max_current = DWC3_IDEV_CHG_MAX;
module_param(dcp_max_current, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(dcp_max_current, "max current drawn for DCP charger");

/* XHCI registers */
#define USB3_HCSPARAMS1		(0x4)
#define USB3_PORTSC		(0x420)

/**
 *  USB QSCRATCH Hardware registers
 *
 */
#define QSCRATCH_REG_OFFSET	(0x000F8800)
#define QSCRATCH_CTRL_REG      (QSCRATCH_REG_OFFSET + 0x04)
#define QSCRATCH_GENERAL_CFG	(QSCRATCH_REG_OFFSET + 0x08)
#define QSCRATCH_RAM1_REG	(QSCRATCH_REG_OFFSET + 0x0C)
#define HS_PHY_CTRL_REG		(QSCRATCH_REG_OFFSET + 0x10)
#define PARAMETER_OVERRIDE_X_REG (QSCRATCH_REG_OFFSET + 0x14)
#define CHARGING_DET_CTRL_REG	(QSCRATCH_REG_OFFSET + 0x18)
#define CHARGING_DET_OUTPUT_REG	(QSCRATCH_REG_OFFSET + 0x1C)
#define ALT_INTERRUPT_EN_REG	(QSCRATCH_REG_OFFSET + 0x20)
#define HS_PHY_IRQ_STAT_REG	(QSCRATCH_REG_OFFSET + 0x24)
#define CGCTL_REG		(QSCRATCH_REG_OFFSET + 0x28)
#define SS_PHY_CTRL_REG		(QSCRATCH_REG_OFFSET + 0x30)
#define SS_PHY_PARAM_CTRL_1	(QSCRATCH_REG_OFFSET + 0x34)
#define SS_PHY_PARAM_CTRL_2	(QSCRATCH_REG_OFFSET + 0x38)
#define SS_CR_PROTOCOL_DATA_IN_REG  (QSCRATCH_REG_OFFSET + 0x3C)
#define SS_CR_PROTOCOL_DATA_OUT_REG (QSCRATCH_REG_OFFSET + 0x40)
#define SS_CR_PROTOCOL_CAP_ADDR_REG (QSCRATCH_REG_OFFSET + 0x44)
#define SS_CR_PROTOCOL_CAP_DATA_REG (QSCRATCH_REG_OFFSET + 0x48)
#define SS_CR_PROTOCOL_READ_REG     (QSCRATCH_REG_OFFSET + 0x4C)
#define SS_CR_PROTOCOL_WRITE_REG    (QSCRATCH_REG_OFFSET + 0x50)
#define PWR_EVNT_IRQ_STAT_REG    (QSCRATCH_REG_OFFSET + 0x58)
#define PWR_EVNT_IRQ_MASK_REG    (QSCRATCH_REG_OFFSET + 0x5C)
#define HS_PHY_CTRL_COMMON_REG	(QSCRATCH_REG_OFFSET + 0xEC)

#define PWR_EVNT_POWERDOWN_IN_P3_MASK		BIT(2)
#define PWR_EVNT_POWERDOWN_OUT_P3_MASK		BIT(3)
#define PWR_EVNT_LPM_IN_L2_MASK			BIT(4)
#define PWR_EVNT_LPM_OUT_L2_MASK		BIT(5)
#define PWR_EVNT_LPM_OUT_L1_MASK		BIT(13)

/* QSCRATCH_GENERAL_CFG register bit offset */
#define PIPE_UTMI_CLK_SEL	BIT(0)
#define PIPE3_PHYSTATUS_SW	BIT(3)
#define PIPE_UTMI_CLK_DIS	BIT(8)

#define DWC3_3P3_VOL_MIN		3075000 /* uV */
#define DWC3_3P3_VOL_MAX		3200000 /* uV */
#define DWC3_3P3_HPM_LOAD		30000	/* uA */
#define DWC3_1P8_VOL_MIN		1800000 /* uV */
#define DWC3_1P8_VOL_MAX		1800000 /* uV */
#define DWC3_1P8_HPM_LOAD		30000   /* uA */

/* TZ SCM parameters */
#define DWC3_MSM_RESTORE_SCM_CFG_CMD 0x2
struct dwc3_msm_scm_cmd_buf {
	unsigned int device_id;
	unsigned int spare;
};

struct dwc3_msm_req_complete {
	struct list_head list_item;
	struct usb_request *req;
	void (*orig_complete)(struct usb_ep *ep,
			      struct usb_request *req);
};

struct dwc3_msm {
	struct device *dev;
	void __iomem *base;
	struct resource *io_res;
	struct platform_device	*dwc3;
	const struct usb_ep_ops *original_ep_ops[DWC3_ENDPOINTS_NUM];
	struct list_head req_complete_list;
	struct clk		*xo_clk;
	struct clk		*ref_clk;
	struct clk		*core_clk;
	struct clk		*iface_clk;
	struct clk		*sleep_clk;
	struct clk		*utmi_clk;
	struct clk		*phy_com_reset;
	unsigned int		utmi_clk_rate;
	struct clk		*utmi_clk_src;
	struct regulator	*dwc3_gdsc;

	struct usb_phy		*hs_phy, *ss_phy;

	struct dbm		*dbm;

	/* VBUS regulator if no OTG and running in host only mode */
	struct regulator	*vbus_otg;
	struct regulator	*vdda33;
	struct regulator	*vdda18;
	struct dwc3_ext_xceiv	ext_xceiv;
	bool			resume_pending;
	atomic_t                pm_suspended;
	int			hs_phy_irq;
	struct delayed_work	resume_work;
	struct work_struct	restart_usb_work;
	bool			in_restart;
	struct dwc3_charger	charger;
	struct usb_phy		*otg_xceiv;
	struct delayed_work	chg_work;
	enum usb_chg_state	chg_state;
	int			pmic_id_irq;
	struct work_struct	id_work;
	struct qpnp_adc_tm_btm_param	adc_param;
	struct qpnp_adc_tm_chip *adc_tm_dev;
	struct delayed_work	init_adc_work;
	bool			id_adc_detect;
	struct qpnp_vadc_chip	*vadc_dev;
	u8			dcd_retries;
	struct work_struct	bus_vote_w;
	unsigned int		bus_vote;
	u32			bus_perf_client;
	struct msm_bus_scale_pdata	*bus_scale_table;
	struct power_supply	usb_psy;
	struct power_supply	*ext_vbus_psy;
	unsigned int		online;
	unsigned int		scope;
	unsigned int		voltage_max;
	unsigned int		current_max;
	unsigned int		health_status;
	unsigned int		tx_fifo_size;
	unsigned int		qdss_tx_fifo_size;
	bool			vbus_active;
	bool			ext_inuse;
	bool			rm_pulldown;
	enum dwc3_id_state	id_state;
	unsigned long		lpm_flags;
#define MDWC3_PHY_REF_CLK_OFF		BIT(0)
#define MDWC3_TCXO_SHUTDOWN		BIT(1)
#define MDWC3_ASYNC_IRQ_WAKE_CAPABILITY	BIT(2)
#define MDWC3_POWER_COLLAPSE		BIT(3)
#define MDWC3_CORECLK_OFF		BIT(4)
#define MDWC3_PHY_REF_AND_CORECLK_OFF	\
			(MDWC3_PHY_REF_CLK_OFF | MDWC3_CORECLK_OFF)

	u32 qscratch_ctl_val;
	dev_t ext_chg_dev;
	struct cdev ext_chg_cdev;
	struct class *ext_chg_class;
	struct device *ext_chg_device;
	bool ext_chg_opened;
	bool ext_chg_active;
	struct completion ext_chg_wait;
	unsigned int scm_dev_id;
	bool suspend_resume_no_support;

	bool power_collapse; /* power collapse on cable disconnect */
	bool power_collapse_por; /* perform POR sequence after power collapse */

	unsigned int		irq_to_affin;
	struct notifier_block	dwc3_cpu_notifier;

	int  pwr_event_irq;
	atomic_t                in_p3;
	unsigned int		lpm_to_suspend_delay;

#ifdef CONFIG_SONY_USB_EXTENSIONS
	struct work_struct	ocp_work;
	struct wake_lock	id_wakelock;

	bool			ext_switching;
	bool			ext_bsv;

	struct qpnp_vadc_chip	*vadc_usb_dp;
	struct qpnp_vadc_chip	*vadc_usb_dm;

	/* mhl switch to USB2 when nothing is connected */
	int			mhl_switch_sel1_gpio;
	int			mhl_switch_sel2_gpio;
	int			usb_shell_sel_gpio;

	struct wakeup_source	wakeup_source_vbus;
#endif
};

#define USB_HSPHY_3P3_VOL_MIN		3050000 /* uV */
#define USB_HSPHY_3P3_VOL_MAX		3300000 /* uV */
#define USB_HSPHY_3P3_HPM_LOAD		16000	/* uA */

#define USB_HSPHY_1P8_VOL_MIN		1800000 /* uV */
#define USB_HSPHY_1P8_VOL_MAX		1800000 /* uV */
#define USB_HSPHY_1P8_HPM_LOAD		19000	/* uA */

#define USB_SSPHY_1P8_VOL_MIN		1800000 /* uV */
#define USB_SSPHY_1P8_VOL_MAX		1800000 /* uV */
#define USB_SSPHY_1P8_HPM_LOAD		23000	/* uA */

#define DSTS_CONNECTSPD_SS		0x4


static struct usb_ext_notification *usb_ext;

static void dwc3_pwr_event_handler(struct dwc3_msm *mdwc);

#ifdef CONFIG_SONY_USB_EXTENSIONS
#define USB_ID_WAKE_LOCK_TIMEOUT	(3 * HZ)
#define MHL_SWITCH_PORT_USB1	0
#define MHL_SWITCH_PORT_USB2	1
#define MHL_SWITCH_PORT_MHL	2

static void dwc3_select_mhl_switch(struct dwc3_msm *mdwc, int port)
{
	if (!gpio_is_valid(mdwc->mhl_switch_sel1_gpio) ||
				!gpio_is_valid(mdwc->mhl_switch_sel2_gpio)) {
		dev_dbg(mdwc->dev, "%s: mhl switch sel gpio is not set\n",
								__func__);
		return;
	}

	if (!gpio_is_valid(mdwc->usb_shell_sel_gpio))
		dev_dbg(mdwc->dev, "%s: usb_shell_sel is not set\n",
								__func__);
	switch (port) {
	case MHL_SWITCH_PORT_USB1:
		gpio_set_value(mdwc->mhl_switch_sel1_gpio, 0);
		gpio_set_value(mdwc->mhl_switch_sel2_gpio, 0);
		dev_info(mdwc->dev, "%s: mhl switch to USB1\n", __func__);
		if (gpio_is_valid(mdwc->usb_shell_sel_gpio)) {
			gpio_set_value(mdwc->usb_shell_sel_gpio, 0);
			dev_dbg(mdwc->dev, "%s: usb_shell_sel switch to low\n",
							__func__);
		}
		break;
	case MHL_SWITCH_PORT_USB2:
		gpio_set_value(mdwc->mhl_switch_sel1_gpio, 0);
		gpio_set_value(mdwc->mhl_switch_sel2_gpio, 1);
		dev_info(mdwc->dev, "%s: mhl switch to USB2\n", __func__);
		if (gpio_is_valid(mdwc->usb_shell_sel_gpio)) {
			gpio_set_value(mdwc->usb_shell_sel_gpio, 1);
			dev_dbg(mdwc->dev, "%s: usb_shell_sel switch to high\n",
							__func__);
		}
		break;
	case MHL_SWITCH_PORT_MHL:
		gpio_set_value(mdwc->mhl_switch_sel1_gpio, 1);
		gpio_set_value(mdwc->mhl_switch_sel2_gpio, 1);
		dev_info(mdwc->dev, "%s: mhl switch to MHL\n", __func__);
		break;
	default:
		gpio_set_value(mdwc->mhl_switch_sel1_gpio, 0);
		gpio_set_value(mdwc->mhl_switch_sel2_gpio, 0);
		dev_err(mdwc->dev, "%s: invalid value=%d, switch to USB1\n",
							__func__, port);
		break;
	}
}
#endif

/**
 *
 * Read register with debug info.
 *
 * @base - DWC3 base virtual address.
 * @offset - register offset.
 *
 * @return u32
 */
static inline u32 dwc3_msm_read_reg(void *base, u32 offset)
{
	u32 val = ioread32(base + offset);
	return val;
}

/**
 * Read register masked field with debug info.
 *
 * @base - DWC3 base virtual address.
 * @offset - register offset.
 * @mask - register bitmask.
 *
 * @return u32
 */
static inline u32 dwc3_msm_read_reg_field(void *base,
					  u32 offset,
					  const u32 mask)
{
	u32 shift = find_first_bit((void *)&mask, 32);
	u32 val = ioread32(base + offset);
	val &= mask;		/* clear other bits */
	val >>= shift;
	return val;
}

/**
 *
 * Write register with debug info.
 *
 * @base - DWC3 base virtual address.
 * @offset - register offset.
 * @val - value to write.
 *
 */
static inline void dwc3_msm_write_reg(void *base, u32 offset, u32 val)
{
	iowrite32(val, base + offset);
}

/**
 * Write register masked field with debug info.
 *
 * @base - DWC3 base virtual address.
 * @offset - register offset.
 * @mask - register bitmask.
 * @val - value to write.
 *
 */
static inline void dwc3_msm_write_reg_field(void *base, u32 offset,
					    const u32 mask, u32 val)
{
	u32 shift = find_first_bit((void *)&mask, 32);
	u32 tmp = ioread32(base + offset);

	tmp &= ~mask;		/* clear written bits */
	val = tmp | (val << shift);
	iowrite32(val, base + offset);
}

/**
 * Write register and read back masked value to confirm it is written
 *
 * @base - DWC3 base virtual address.
 * @offset - register offset.
 * @mask - register bitmask specifying what should be updated
 * @val - value to write.
 *
 */
static inline void dwc3_msm_write_readback(void *base, u32 offset,
					    const u32 mask, u32 val)
{
	u32 write_val, tmp = ioread32(base + offset);

	tmp &= ~mask;		/* retain other bits */
	write_val = tmp | val;

	iowrite32(write_val, base + offset);

	/* Read back to see if val was written */
	tmp = ioread32(base + offset);
	tmp &= mask;		/* clear other bits */

	if (tmp != val)
		pr_err("%s: write: %x to QSCRATCH: %x FAILED\n",
			__func__, val, offset);
}

static bool dwc3_msm_is_host_superspeed(struct dwc3_msm *mdwc)
{
	int i, num_ports;
	u32 reg;

	reg = dwc3_msm_read_reg(mdwc->base, USB3_HCSPARAMS1);
	num_ports = HCS_MAX_PORTS(reg);

	for (i = 0; i < num_ports; i++) {
		reg = dwc3_msm_read_reg(mdwc->base, USB3_PORTSC + i*0x10);
		if ((reg & PORT_PE) && DEV_SUPERSPEED(reg))
			return true;
	}

	return false;
}

static inline bool dwc3_msm_is_dev_superspeed(struct dwc3_msm *mdwc)
{
	u8 speed;

	speed = dwc3_msm_read_reg(mdwc->base, DWC3_DSTS) & DWC3_DSTS_CONNECTSPD;
	return !!(speed & DSTS_CONNECTSPD_SS);
}

static inline bool dwc3_msm_is_superspeed(struct dwc3_msm *mdwc)
{
	if (mdwc->scope == POWER_SUPPLY_SCOPE_SYSTEM)
		return dwc3_msm_is_host_superspeed(mdwc);

	return dwc3_msm_is_dev_superspeed(mdwc);
}

/**
 * Dump all QSCRATCH registers.
 *
 */
static void dwc3_msm_dump_phy_info(struct dwc3_msm *mdwc)
{

	dbg_print_reg("SSPHY_CTRL_REG", dwc3_msm_read_reg(mdwc->base,
						SS_PHY_CTRL_REG));
	dbg_print_reg("HSPHY_CTRL_REG", dwc3_msm_read_reg(mdwc->base,
						HS_PHY_CTRL_REG));
	dbg_print_reg("QSCRATCH_CTRL_REG", dwc3_msm_read_reg(mdwc->base,
						QSCRATCH_CTRL_REG));
	dbg_print_reg("QSCRATCH_GENERAL_CFG", dwc3_msm_read_reg(mdwc->base,
						QSCRATCH_GENERAL_CFG));
	dbg_print_reg("PARAMETER_OVERRIDE_X_REG", dwc3_msm_read_reg(mdwc->base,
						PARAMETER_OVERRIDE_X_REG));
	dbg_print_reg("HS_PHY_IRQ_STAT_REG", dwc3_msm_read_reg(mdwc->base,
						HS_PHY_IRQ_STAT_REG));
	dbg_print_reg("SS_PHY_PARAM_CTRL_1", dwc3_msm_read_reg(mdwc->base,
						SS_PHY_PARAM_CTRL_1));
	dbg_print_reg("SS_PHY_PARAM_CTRL_2", dwc3_msm_read_reg(mdwc->base,
						SS_PHY_PARAM_CTRL_2));
	dbg_print_reg("QSCRATCH_RAM1_REG", dwc3_msm_read_reg(mdwc->base,
						QSCRATCH_RAM1_REG));
	dbg_print_reg("PWR_EVNT_IRQ_STAT_REG", dwc3_msm_read_reg(mdwc->base,
						PWR_EVNT_IRQ_STAT_REG));
	dbg_print_reg("PWR_EVNT_IRQ_MASK_REG", dwc3_msm_read_reg(mdwc->base,
						PWR_EVNT_IRQ_MASK_REG));
}


/**
 * Configure the DBM with the BAM's data fifo.
 * This function is called by the USB BAM Driver
 * upon initialization.
 *
 * @ep - pointer to usb endpoint.
 * @addr - address of data fifo.
 * @size - size of data fifo.
 *
 */
int msm_data_fifo_config(struct usb_ep *ep, phys_addr_t addr,
			 u32 size, u8 dst_pipe_idx)
{
	struct dwc3_ep *dep = to_dwc3_ep(ep);
	struct dwc3 *dwc = dep->dwc;
	struct dwc3_msm *mdwc = dev_get_drvdata(dwc->dev->parent);

	dev_dbg(mdwc->dev, "%s\n", __func__);

	return	dbm_data_fifo_config(mdwc->dbm, dep->number, addr, size,
						dst_pipe_idx);
}


/**
* Cleanups for msm endpoint on request complete.
*
* Also call original request complete.
*
* @usb_ep - pointer to usb_ep instance.
* @request - pointer to usb_request instance.
*
* @return int - 0 on success, negative on error.
*/
static void dwc3_msm_req_complete_func(struct usb_ep *ep,
				       struct usb_request *request)
{
	struct dwc3_ep *dep = to_dwc3_ep(ep);
	struct dwc3 *dwc = dep->dwc;
	struct dwc3_msm *mdwc = dev_get_drvdata(dwc->dev->parent);
	struct dwc3_msm_req_complete *req_complete = NULL;

	/* Find original request complete function and remove it from list */
	list_for_each_entry(req_complete, &mdwc->req_complete_list, list_item) {
		if (req_complete->req == request)
			break;
	}
	if (!req_complete || req_complete->req != request) {
		dev_err(dep->dwc->dev, "%s: could not find the request\n",
					__func__);
		return;
	}
	list_del(&req_complete->list_item);

	/*
	 * Release another one TRB to the pool since DBM queue took 2 TRBs
	 * (normal and link), and the dwc3/gadget.c :: dwc3_gadget_giveback
	 * released only one.
	 */
	dep->busy_slot++;

	/* Unconfigure dbm ep */
	dbm_ep_unconfig(mdwc->dbm, dep->number);

	/*
	 * If this is the last endpoint we unconfigured, than reset also
	 * the event buffers; unless unconfiguring the ep due to lpm,
	 * in which case the event buffer only gets reset during the
	 * block reset.
	 */
	if (0 == dbm_get_num_of_eps_configured(mdwc->dbm) &&
		!dbm_reset_ep_after_lpm(mdwc->dbm))
			dbm_event_buffer_config(mdwc->dbm, 0, 0, 0);

	/*
	 * Call original complete function, notice that dwc->lock is already
	 * taken by the caller of this function (dwc3_gadget_giveback()).
	 */
	request->complete = req_complete->orig_complete;
	if (request->complete)
		request->complete(ep, request);

	kfree(req_complete);
}


/**
* Helper function
*
* Reset  DBM endpoint.
*
* @mdwc - pointer to dwc3_msm instance.
* @dep - pointer to dwc3_ep instance.
*
* @return int - 0 on success, negative on error.
*/
static int __dwc3_msm_dbm_ep_reset(struct dwc3_msm *mdwc, struct dwc3_ep *dep)
{
	int ret;

	dev_dbg(mdwc->dev, "Resetting dbm endpoint %d\n", dep->number);

	/* Reset the dbm endpoint */
	ret = dbm_ep_soft_reset(mdwc->dbm, dep->number, true);
	if (ret) {
		dev_err(mdwc->dev, "%s: failed to assert dbm ep reset\n",
				__func__);
		return ret;
	}

	/*
	 * The necessary delay between asserting and deasserting the dbm ep
	 * reset is based on the number of active endpoints. If there is more
	 * than one endpoint, a 1 msec delay is required. Otherwise, a shorter
	 * delay will suffice.
	 */
	if (dbm_get_num_of_eps_configured(mdwc->dbm) > 1)
		usleep_range(1000, 1200);
	else
		udelay(10);
	ret = dbm_ep_soft_reset(mdwc->dbm, dep->number, false);
	if (ret) {
		dev_err(mdwc->dev, "%s: failed to deassert dbm ep reset\n",
				__func__);
		return ret;
	}

	return 0;
}

/**
* Reset the DBM endpoint which is linked to the given USB endpoint.
*
* @usb_ep - pointer to usb_ep instance.
*
* @return int - 0 on success, negative on error.
*/

int msm_dwc3_reset_dbm_ep(struct usb_ep *ep)
{
	struct dwc3_ep *dep = to_dwc3_ep(ep);
	struct dwc3 *dwc = dep->dwc;
	struct dwc3_msm *mdwc = dev_get_drvdata(dwc->dev->parent);

	return __dwc3_msm_dbm_ep_reset(mdwc, dep);
}
EXPORT_SYMBOL(msm_dwc3_reset_dbm_ep);


/**
* Helper function.
* See the header of the dwc3_msm_ep_queue function.
*
* @dwc3_ep - pointer to dwc3_ep instance.
* @req - pointer to dwc3_request instance.
*
* @return int - 0 on success, negative on error.
*/
static int __dwc3_msm_ep_queue(struct dwc3_ep *dep, struct dwc3_request *req)
{
	struct dwc3_trb *trb;
	struct dwc3_trb *trb_link;
	struct dwc3_gadget_ep_cmd_params params;
	u32 cmd;
	int ret = 0;

	/* We push the request to the dep->req_queued list to indicate that
	 * this request is issued with start transfer. The request will be out
	 * from this list in 2 cases. The first is that the transfer will be
	 * completed (not if the transfer is endless using a circular TRBs with
	 * with link TRB). The second case is an option to do stop stransfer,
	 * this can be initiated by the function driver when calling dequeue.
	 */
	req->queued = true;
	list_add_tail(&req->list, &dep->req_queued);

	/* First, prepare a normal TRB, point to the fake buffer */
	trb = &dep->trb_pool[dep->free_slot & DWC3_TRB_MASK];
	dep->free_slot++;
	memset(trb, 0, sizeof(*trb));

	req->trb = trb;
	trb->bph = DBM_TRB_BIT | DBM_TRB_DMA | DBM_TRB_EP_NUM(dep->number);
	trb->size = DWC3_TRB_SIZE_LENGTH(req->request.length);
	trb->ctrl = DWC3_TRBCTL_NORMAL | DWC3_TRB_CTRL_HWO |
		DWC3_TRB_CTRL_CHN | (req->direction ? 0 : DWC3_TRB_CTRL_CSP);
	req->trb_dma = dwc3_trb_dma_offset(dep, trb);

	/* Second, prepare a Link TRB that points to the first TRB*/
	trb_link = &dep->trb_pool[dep->free_slot & DWC3_TRB_MASK];
	dep->free_slot++;
	memset(trb_link, 0, sizeof *trb_link);

	trb_link->bpl = lower_32_bits(req->trb_dma);
	trb_link->bph = DBM_TRB_BIT |
			DBM_TRB_DMA | DBM_TRB_EP_NUM(dep->number);
	trb_link->size = 0;
	trb_link->ctrl = DWC3_TRBCTL_LINK_TRB | DWC3_TRB_CTRL_HWO;

	/*
	 * Now start the transfer
	 */
	memset(&params, 0, sizeof(params));
	params.param0 = 0; /* TDAddr High */
	params.param1 = lower_32_bits(req->trb_dma); /* DAddr Low */

	/* DBM requires IOC to be set */
	cmd = DWC3_DEPCMD_STARTTRANSFER | DWC3_DEPCMD_CMDIOC;
	ret = dwc3_send_gadget_ep_cmd(dep->dwc, dep->number, cmd, &params);
	if (ret < 0) {
		dev_dbg(dep->dwc->dev,
			"%s: failed to send STARTTRANSFER command\n",
			__func__);

		list_del(&req->list);
		return ret;
	}
	dep->flags |= DWC3_EP_BUSY;
	dep->resource_index = dwc3_gadget_ep_get_transfer_index(dep->dwc,
		dep->number);

	return ret;
}

/**
* Queue a usb request to the DBM endpoint.
* This function should be called after the endpoint
* was enabled by the ep_enable.
*
* This function prepares special structure of TRBs which
* is familiar with the DBM HW, so it will possible to use
* this endpoint in DBM mode.
*
* The TRBs prepared by this function, is one normal TRB
* which point to a fake buffer, followed by a link TRB
* that points to the first TRB.
*
* The API of this function follow the regular API of
* usb_ep_queue (see usb_ep_ops in include/linuk/usb/gadget.h).
*
* @usb_ep - pointer to usb_ep instance.
* @request - pointer to usb_request instance.
* @gfp_flags - possible flags.
*
* @return int - 0 on success, negative on error.
*/
static int dwc3_msm_ep_queue(struct usb_ep *ep,
			     struct usb_request *request, gfp_t gfp_flags)
{
	struct dwc3_request *req = to_dwc3_request(request);
	struct dwc3_ep *dep = to_dwc3_ep(ep);
	struct dwc3 *dwc = dep->dwc;
	struct dwc3_msm *mdwc = dev_get_drvdata(dwc->dev->parent);
	struct dwc3_msm_req_complete *req_complete;
	unsigned long flags;
	int ret = 0, size;
	u8 bam_pipe;
	bool producer;
	bool disable_wb;
	bool internal_mem;
	bool ioc;
	bool superspeed;

	if (!(request->udc_priv & MSM_SPS_MODE)) {
		/* Not SPS mode, call original queue */
		dev_vdbg(mdwc->dev, "%s: not sps mode, use regular queue\n",
					__func__);

		return (mdwc->original_ep_ops[dep->number])->queue(ep,
								request,
								gfp_flags);
	}

	if (!dep->endpoint.desc) {
		dev_err(mdwc->dev,
			"%s: trying to queue request %p to disabled ep %s\n",
			__func__, request, ep->name);
		return -EPERM;
	}

	if (dep->number == 0 || dep->number == 1) {
		dev_err(mdwc->dev,
			"%s: trying to queue dbm request %p to control ep %s\n",
			__func__, request, ep->name);
		return -EPERM;
	}


	if (dep->busy_slot != dep->free_slot || !list_empty(&dep->request_list)
					 || !list_empty(&dep->req_queued)) {
		dev_err(mdwc->dev,
			"%s: trying to queue dbm request %p tp ep %s\n",
			__func__, request, ep->name);
		return -EPERM;
	} else {
		dep->busy_slot = 0;
		dep->free_slot = 0;
	}

	/* HW restriction regarding TRB size (8KB) */
	if (req->request.length < 0x2000) {
		dev_err(mdwc->dev, "%s: Min TRB size is 8KB\n", __func__);
		return -EINVAL;
	}

	/*
	 * Override req->complete function, but before doing that,
	 * store it's original pointer in the req_complete_list.
	 */
	req_complete = kzalloc(sizeof(*req_complete), gfp_flags);
	if (!req_complete) {
		dev_err(mdwc->dev, "%s: not enough memory\n", __func__);
		return -ENOMEM;
	}
	req_complete->req = request;
	req_complete->orig_complete = request->complete;
	list_add_tail(&req_complete->list_item, &mdwc->req_complete_list);
	request->complete = dwc3_msm_req_complete_func;

	/*
	 * Configure the DBM endpoint
	 */
	bam_pipe = request->udc_priv & MSM_PIPE_ID_MASK;
	producer = ((request->udc_priv & MSM_PRODUCER) ? true : false);
	disable_wb = ((request->udc_priv & MSM_DISABLE_WB) ? true : false);
	internal_mem = ((request->udc_priv & MSM_INTERNAL_MEM) ? true : false);
	ioc = ((request->udc_priv & MSM_ETD_IOC) ? true : false);

	ret = dbm_ep_config(mdwc->dbm, dep->number, bam_pipe, producer,
				disable_wb, internal_mem, ioc);
	if (ret < 0) {
		dev_err(mdwc->dev,
			"error %d after calling dbm_ep_config\n", ret);
		return ret;
	}

	dev_vdbg(dwc->dev, "%s: queing request %p to ep %s length %d\n",
			__func__, request, ep->name, request->length);
	size = dwc3_msm_read_reg(mdwc->base, DWC3_GEVNTSIZ(0));
	dbm_event_buffer_config(mdwc->dbm,
		dwc3_msm_read_reg(mdwc->base, DWC3_GEVNTADRLO(0)),
		dwc3_msm_read_reg(mdwc->base, DWC3_GEVNTADRHI(0)),
		DWC3_GEVNTSIZ_SIZE(size));

	/*
	 * We must obtain the lock of the dwc3 core driver,
	 * including disabling interrupts, so we will be sure
	 * that we are the only ones that configure the HW device
	 * core and ensure that we queuing the request will finish
	 * as soon as possible so we will release back the lock.
	 */
	spin_lock_irqsave(&dwc->lock, flags);
	ret = __dwc3_msm_ep_queue(dep, req);
	spin_unlock_irqrestore(&dwc->lock, flags);
	if (ret < 0) {
		dev_err(mdwc->dev,
			"error %d after calling __dwc3_msm_ep_queue\n", ret);
		return ret;
	}

	superspeed = dwc3_msm_is_dev_superspeed(mdwc);
	dbm_set_speed(mdwc->dbm, (u8)superspeed);

	return 0;
}


/**
 * Configure MSM endpoint.
 * This function do specific configurations
 * to an endpoint which need specific implementaion
 * in the MSM architecture.
 *
 * This function should be called by usb function/class
 * layer which need a support from the specific MSM HW
 * which wrap the USB3 core. (like DBM specific endpoints)
 *
 * @ep - a pointer to some usb_ep instance
 *
 * @return int - 0 on success, negetive on error.
 */
int msm_ep_config(struct usb_ep *ep)
{
	struct dwc3_ep *dep = to_dwc3_ep(ep);
	struct dwc3 *dwc = dep->dwc;
	struct dwc3_msm *mdwc = dev_get_drvdata(dwc->dev->parent);
	struct usb_ep_ops *new_ep_ops;


	/* Save original ep ops for future restore*/
	if (mdwc->original_ep_ops[dep->number]) {
		dev_err(mdwc->dev,
			"ep [%s,%d] already configured as msm endpoint\n",
			ep->name, dep->number);
		return -EPERM;
	}
	mdwc->original_ep_ops[dep->number] = ep->ops;

	/* Set new usb ops as we like */
	new_ep_ops = kzalloc(sizeof(struct usb_ep_ops), GFP_ATOMIC);
	if (!new_ep_ops) {
		dev_err(mdwc->dev,
			"%s: unable to allocate mem for new usb ep ops\n",
			__func__);
		return -ENOMEM;
	}
	(*new_ep_ops) = (*ep->ops);
	new_ep_ops->queue = dwc3_msm_ep_queue;
	ep->ops = new_ep_ops;

	/*
	 * Do HERE more usb endpoint configurations
	 * which are specific to MSM.
	 */

	return 0;
}
EXPORT_SYMBOL(msm_ep_config);

/**
 * Un-configure MSM endpoint.
 * Tear down configurations done in the
 * dwc3_msm_ep_config function.
 *
 * @ep - a pointer to some usb_ep instance
 *
 * @return int - 0 on success, negative on error.
 */
int msm_ep_unconfig(struct usb_ep *ep)
{
	struct dwc3_ep *dep = to_dwc3_ep(ep);
	struct dwc3 *dwc = dep->dwc;
	struct dwc3_msm *mdwc = dev_get_drvdata(dwc->dev->parent);
	struct usb_ep_ops *old_ep_ops;

	/* Restore original ep ops */
	if (!mdwc->original_ep_ops[dep->number]) {
		dev_err(mdwc->dev,
			"ep [%s,%d] was not configured as msm endpoint\n",
			ep->name, dep->number);
		return -EINVAL;
	}
	old_ep_ops = (struct usb_ep_ops	*)ep->ops;
	ep->ops = mdwc->original_ep_ops[dep->number];
	mdwc->original_ep_ops[dep->number] = NULL;
	kfree(old_ep_ops);

	/*
	 * Do HERE more usb endpoint un-configurations
	 * which are specific to MSM.
	 */

	return 0;
}
EXPORT_SYMBOL(msm_ep_unconfig);

void dwc3_tx_fifo_resize_request(struct usb_ep *ep, bool qdss_enabled)
{
	struct dwc3_ep *dep = to_dwc3_ep(ep);
	struct dwc3 *dwc = dep->dwc;
	struct dwc3_msm *mdwc = dev_get_drvdata(dwc->dev->parent);

	if (qdss_enabled) {
		dwc->tx_fifo_reduced = true;
		dwc->tx_fifo_size = mdwc->qdss_tx_fifo_size;
	} else {
		dwc->tx_fifo_reduced = false;
		dwc->tx_fifo_size = mdwc->tx_fifo_size;
	}
}
EXPORT_SYMBOL(dwc3_tx_fifo_resize_request);

static void dwc3_resume_work(struct work_struct *w);

static void dwc3_restart_usb_work(struct work_struct *w)
{
	struct dwc3_msm *mdwc = container_of(w, struct dwc3_msm,
						restart_usb_work);
	struct dwc3 *dwc = platform_get_drvdata(mdwc->dwc3);
	enum dwc3_chg_type chg_type;
	unsigned timeout = 50;

	dev_dbg(mdwc->dev, "%s\n", __func__);

	if (atomic_read(&dwc->in_lpm) || !mdwc->otg_xceiv) {
		dev_err(mdwc->dev, "%s failed!!!\n", __func__);
		return;
	}

	/* guard against concurrent VBUS handling */
	mdwc->in_restart = true;

	if (!mdwc->ext_xceiv.bsv) {
		dev_dbg(mdwc->dev, "%s bailing out in disconnect\n", __func__);
		dwc->err_evt_seen = false;
		mdwc->in_restart = false;
		return;
	}

	dbg_event(0xFF, "RestartUSB", 0);
	chg_type = mdwc->charger.chg_type;

	/* Reset active USB connection */
	mdwc->ext_xceiv.bsv = false;
	dwc3_resume_work(&mdwc->resume_work.work);

	/* Make sure disconnect is processed before sending connect */
	while (--timeout && !pm_runtime_suspended(mdwc->dev))
		msleep(20);

	if (!timeout) {
		dev_warn(mdwc->dev, "Not in LPM after disconnect, forcing suspend...\n");
		pm_runtime_suspend(mdwc->dev);
	}

	/* Force reconnect only if cable is still connected */
	if (mdwc->vbus_active) {
		mdwc->ext_xceiv.bsv = true;
		mdwc->charger.chg_type = chg_type;
		dwc3_resume_work(&mdwc->resume_work.work);
	}

	dwc->err_evt_seen = false;
	mdwc->in_restart = false;
}

#ifdef CONFIG_SONY_USB_EXTENSIONS
static bool msm_dwc3_check_vbus(void *ctx)
{
	struct dwc3_msm *mdwc = ctx;

	if (!mdwc) {
		pr_err("%s: DWC3 driver already removed\n", __func__);
		return false;
	}

	pr_debug("%s: vbus_active:%d\n", __func__, mdwc->vbus_active);
	return mdwc->vbus_active;
}

static void msm_dwc3_vbus_control(void *ctx, bool on)
{
	struct dwc3_msm *mdwc = ctx;
	struct dwc3	*dwc;

	if (!mdwc) {
		pr_err("%s: DWC3 driver already removed\n", __func__);
		return;
	}

	dwc = platform_get_drvdata(mdwc->dwc3);
	if (!dwc) {
		pr_err("%s: Failed to get dwc device\n", __func__);
		return;
	}

	pr_info("%s: source vbus %s\n", __func__, on ? "start" : "stop");

	if (!dwc->dotg->vbus_otg) {
		dwc->dotg->vbus_otg = devm_regulator_get(dwc->dev->parent,
								"vbus_dwc3");
		if (!dwc->dotg->vbus_otg) {
			pr_err("%s: Cannot get vbus_otg\n", __func__);
			return;
		}
	}

	if (on) {
		if (!regulator_is_enabled(dwc->dotg->vbus_otg)) {
			__pm_stay_awake(&mdwc->wakeup_source_vbus);
			if (regulator_enable(dwc->dotg->vbus_otg)) {
				pr_err("%s: unable to enable vbus_otg\n",
					__func__);
				__pm_relax(&mdwc->wakeup_source_vbus);
			}
		}
	} else {
		if (regulator_is_enabled(dwc->dotg->vbus_otg)) {
			if (regulator_disable(dwc->dotg->vbus_otg))
				pr_err("%s: unable to disable vbus_otg\n",
					__func__);
			else if (mdwc->wakeup_source_vbus.active)
				__pm_wakeup_event(&mdwc->wakeup_source_vbus, 100);
		}
	}
}
#endif

/**
 * Reset USB peripheral connection
 * Inform OTG for Vbus LOW followed by Vbus HIGH notification.
 * This performs full hardware reset and re-initialization which
 * might be required by some DBM client driver during uninit/cleanup.
 */
void msm_dwc3_restart_usb_session(struct usb_gadget *gadget)
{
	struct dwc3 *dwc = container_of(gadget, struct dwc3, gadget);
	struct dwc3_msm *mdwc = dev_get_drvdata(dwc->dev->parent);

	if (!mdwc)
		return;

	dev_dbg(mdwc->dev, "%s\n", __func__);
	queue_work(system_nrt_wq, &mdwc->restart_usb_work);
}
EXPORT_SYMBOL(msm_dwc3_restart_usb_session);

/**
 * msm_register_usb_ext_notification: register for event notification
 * @info: pointer to client usb_ext_notification structure. May be NULL.
 *
 * @return int - 0 on success, negative on error
 */
int msm_register_usb_ext_notification(struct usb_ext_notification *info)
{
	pr_debug("%s usb_ext: %p\n", __func__, info);

	if (info) {
		if (usb_ext) {
			pr_err("%s: already registered\n", __func__);
			return -EEXIST;
		}

		if (!info->notify) {
			pr_err("%s: notify is NULL\n", __func__);
			return -EINVAL;
		}
	}

	usb_ext = info;
#ifdef CONFIG_SONY_USB_EXTENSIONS
	usb_ext->vbus_control = msm_dwc3_vbus_control;
	usb_ext->check_vbus = msm_dwc3_check_vbus;
#endif
	return 0;
}
EXPORT_SYMBOL(msm_register_usb_ext_notification);

/*
 * Check whether the DWC3 requires resetting the ep
 * after going to Low Power Mode (lpm)
 */
bool msm_dwc3_reset_ep_after_lpm(struct usb_gadget *gadget)
{
	struct dwc3 *dwc = container_of(gadget, struct dwc3, gadget);
	struct dwc3_msm *mdwc = dev_get_drvdata(dwc->dev->parent);

	return dbm_reset_ep_after_lpm(mdwc->dbm);
}
EXPORT_SYMBOL(msm_dwc3_reset_ep_after_lpm);

/*
 * Config Global Distributed Switch Controller (GDSC)
 * to support controller power collapse
 */
static int dwc3_msm_config_gdsc(struct dwc3_msm *mdwc, int on)
{
	int ret = 0;

	if (IS_ERR(mdwc->dwc3_gdsc))
		return 0;

	if (!mdwc->dwc3_gdsc) {
		mdwc->dwc3_gdsc = devm_regulator_get(mdwc->dev,
			"USB3_GDSC");
		if (IS_ERR(mdwc->dwc3_gdsc))
			return 0;
	}

	if (on) {
		ret = regulator_enable(mdwc->dwc3_gdsc);
		if (ret) {
			dev_err(mdwc->dev, "unable to enable usb3 gdsc\n");
			return ret;
		}
	} else {
		regulator_disable(mdwc->dwc3_gdsc);
	}

	return 0;
}

/* Restores VMIDMT/xPU security configuration in TrustZone */
static int dwc3_msm_restore_sec_config(unsigned int device_id)
{
	struct dwc3_msm_scm_cmd_buf cbuf;
	int ret, scm_ret = 0;

	if (!device_id)
		return 0;

	cbuf.device_id = device_id;

	ret = scm_call(SCM_SVC_MP, DWC3_MSM_RESTORE_SCM_CFG_CMD, &cbuf,
			sizeof(cbuf), &scm_ret, sizeof(scm_ret));
	if (ret || scm_ret) {
		pr_err("%s: failed(%d) to restore sec config, scm_ret=%d\n",
			__func__, ret, scm_ret);
		return ret;
	}

	return 0;
}

static int dwc3_msm_link_clk_reset(struct dwc3_msm *mdwc, bool assert)
{
	int ret = 0;

	if (assert) {
		if (mdwc->pwr_event_irq)
			disable_irq(mdwc->pwr_event_irq);
		/* Using asynchronous block reset to the hardware */
		dev_dbg(mdwc->dev, "block_reset ASSERT\n");
		clk_disable_unprepare(mdwc->ref_clk);
		clk_disable_unprepare(mdwc->iface_clk);
		clk_disable_unprepare(mdwc->utmi_clk);
		clk_disable_unprepare(mdwc->sleep_clk);
		clk_disable_unprepare(mdwc->core_clk);
		ret = clk_reset(mdwc->core_clk, CLK_RESET_ASSERT);
		if (ret)
			dev_err(mdwc->dev, "dwc3 core_clk assert failed\n");
	} else {
		dev_dbg(mdwc->dev, "block_reset DEASSERT\n");
		ret = clk_reset(mdwc->core_clk, CLK_RESET_DEASSERT);
		ndelay(200);
		clk_prepare_enable(mdwc->core_clk);
		clk_prepare_enable(mdwc->sleep_clk);
		clk_prepare_enable(mdwc->utmi_clk);
		clk_prepare_enable(mdwc->ref_clk);
		clk_prepare_enable(mdwc->iface_clk);
		if (ret)
			dev_err(mdwc->dev, "dwc3 core_clk deassert failed\n");
		if (mdwc->pwr_event_irq)
			enable_irq(mdwc->pwr_event_irq);
	}

	return ret;
}

static void dwc3_msm_update_ref_clk(struct dwc3_msm *mdwc)
{
	u32 guctl, gfladj = 0;

	guctl = dwc3_msm_read_reg(mdwc->base, DWC3_GUCTL);
	guctl &= ~DWC3_GUCTL_REFCLKPER;

	/* GFLADJ register is used starting with revision 2.50a */
	if (dwc3_msm_read_reg(mdwc->base, DWC3_GSNPSID) >= DWC3_REVISION_250A) {
		gfladj = dwc3_msm_read_reg(mdwc->base, DWC3_GFLADJ);
		gfladj &= ~DWC3_GFLADJ_REFCLK_240MHZDECR_PLS1;
		gfladj &= ~DWC3_GFLADJ_REFCLK_240MHZ_DECR;
		gfladj &= ~DWC3_GFLADJ_REFCLK_LPM_SEL;
		gfladj &= ~DWC3_GFLADJ_REFCLK_FLADJ;
	}

	/* Refer to SNPS Databook Table 6-55 for calculations used */
	switch (mdwc->utmi_clk_rate) {
	case 19200000:
		guctl |= 52 << __ffs(DWC3_GUCTL_REFCLKPER);
		gfladj |= 12 << __ffs(DWC3_GFLADJ_REFCLK_240MHZ_DECR);
		gfladj |= DWC3_GFLADJ_REFCLK_240MHZDECR_PLS1;
		gfladj |= DWC3_GFLADJ_REFCLK_LPM_SEL;
		gfladj |= 200 << __ffs(DWC3_GFLADJ_REFCLK_FLADJ);
		break;
	case 24000000:
		guctl |= 41 << __ffs(DWC3_GUCTL_REFCLKPER);
		gfladj |= 10 << __ffs(DWC3_GFLADJ_REFCLK_240MHZ_DECR);
		gfladj |= DWC3_GFLADJ_REFCLK_LPM_SEL;
		gfladj |= 2032 << __ffs(DWC3_GFLADJ_REFCLK_FLADJ);
		break;
	default:
		dev_warn(mdwc->dev, "Unsupported utmi_clk_rate: %u\n",
				mdwc->utmi_clk_rate);
		break;
	}

	dwc3_msm_write_reg(mdwc->base, DWC3_GUCTL, guctl);
	if (gfladj)
		dwc3_msm_write_reg(mdwc->base, DWC3_GFLADJ, gfladj);
}

/* Initialize QSCRATCH registers for HSPHY and SSPHY operation */
static void dwc3_msm_qscratch_reg_init(struct dwc3_msm *mdwc)
{
	if (dwc3_msm_read_reg(mdwc->base, DWC3_GSNPSID) < DWC3_REVISION_250A)
		/* On older cores set XHCI_REV bit to specify revision 1.0 */
		dwc3_msm_write_reg_field(mdwc->base, QSCRATCH_GENERAL_CFG,
					 BIT(2), 1);

	/*
	 * Enable master clock for RAMs to allow BAM to access RAMs when
	 * RAM clock gating is enabled via DWC3's GCTL. Otherwise issues
	 * are seen where RAM clocks get turned OFF in SS mode
	 */
	dwc3_msm_write_reg(mdwc->base, CGCTL_REG,
		dwc3_msm_read_reg(mdwc->base, CGCTL_REG) | 0x18);

	/*
	 * This is required to restore the POR value after userspace
	 * is done with charger detection.
	 */
	mdwc->qscratch_ctl_val =
		dwc3_msm_read_reg(mdwc->base, QSCRATCH_CTRL_REG);
}

static void dwc3_msm_notify_event(struct dwc3 *dwc, unsigned event)
{
	struct dwc3_msm *mdwc = dev_get_drvdata(dwc->dev->parent);
	u32 reg;

	if (dwc->revision < DWC3_REVISION_230A)
		return;

	switch (event) {
	case DWC3_CONTROLLER_ERROR_EVENT:
		dev_info(mdwc->dev,
			"DWC3_CONTROLLER_ERROR_EVENT received, irq cnt %lu\n",
			dwc->irq_cnt);

		dwc3_msm_dump_phy_info(mdwc);
		dwc3_gadget_disable_irq(dwc);

		/* prevent core from generating interrupts until recovery */
		reg = dwc3_msm_read_reg(mdwc->base, DWC3_GCTL);
		reg |= DWC3_GCTL_CORESOFTRESET;
		dwc3_msm_write_reg(mdwc->base, DWC3_GCTL, reg);

		/* restart USB which performs full reset and reconnect */
		queue_work(system_nrt_wq, &mdwc->restart_usb_work);
		break;
	case DWC3_CONTROLLER_RESET_EVENT:
		dev_dbg(mdwc->dev, "DWC3_CONTROLLER_RESET_EVENT received\n");
		/* HS & SSPHYs get reset as part of core soft reset */
		dwc3_msm_qscratch_reg_init(mdwc);
		break;
	case DWC3_CONTROLLER_POST_RESET_EVENT:
		dev_dbg(mdwc->dev,
				"DWC3_CONTROLLER_POST_RESET_EVENT received\n");

		/*
		 * Below sequence is used when controller is working without
		 * having ssphy and only USB high speed is supported.
		 */
		if (dwc->maximum_speed == USB_SPEED_HIGH) {
			dwc3_msm_write_reg(mdwc->base, QSCRATCH_GENERAL_CFG,
				dwc3_msm_read_reg(mdwc->base,
				QSCRATCH_GENERAL_CFG)
				| PIPE_UTMI_CLK_DIS);

			usleep_range(2, 5);


			dwc3_msm_write_reg(mdwc->base, QSCRATCH_GENERAL_CFG,
				dwc3_msm_read_reg(mdwc->base,
				QSCRATCH_GENERAL_CFG)
				| PIPE_UTMI_CLK_SEL
				| PIPE3_PHYSTATUS_SW);

			usleep_range(2, 5);

			dwc3_msm_write_reg(mdwc->base, QSCRATCH_GENERAL_CFG,
				dwc3_msm_read_reg(mdwc->base,
				QSCRATCH_GENERAL_CFG)
				& ~PIPE_UTMI_CLK_DIS);
		}

		/* Re-initialize SSPHY after reset */
		usb_phy_set_params(mdwc->ss_phy);
		dwc3_msm_update_ref_clk(mdwc);
		dwc3_msm_restore_sec_config(mdwc->scm_dev_id);
		dwc->tx_fifo_size = mdwc->tx_fifo_size;
		break;
	case DWC3_CONTROLLER_POST_INITIALIZATION_EVENT:
		/*
		 * Workaround: Disable internal clock gating always, as there
		 * is a known HW bug that causes the internal RAM clock to get
		 * stuck when entering low power modes.
		 */
		dwc3_msm_write_reg_field(mdwc->base, DWC3_GCTL,
					DWC3_GCTL_DSBLCLKGTNG, 1);
		usb_phy_post_init(mdwc->ss_phy);
		break;
	case DWC3_CONTROLLER_CONNDONE_EVENT:
		dev_dbg(mdwc->dev, "DWC3_CONTROLLER_CONNDONE_EVENT received\n");
		/*
		 * Add power event if the dbm indicates coming out of L1 by
		 * interrupt
		 */
		if (mdwc->dbm && dbm_l1_lpm_interrupt(mdwc->dbm))
			dwc3_msm_write_reg_field(mdwc->base,
					PWR_EVNT_IRQ_MASK_REG,
					PWR_EVNT_LPM_OUT_L1_MASK, 1);

		atomic_set(&dwc->in_lpm, 0);
		break;
	case DWC3_CONTROLLER_NOTIFY_OTG_EVENT:
		dev_dbg(mdwc->dev, "DWC3_CONTROLLER_NOTIFY_OTG_EVENT received\n");
		if (dwc->enable_bus_suspend) {
			mdwc->ext_xceiv.suspend = dwc->b_suspend;
			queue_delayed_work(system_nrt_wq,
				&mdwc->resume_work, 0);
		}
		break;
	default:
		dev_dbg(mdwc->dev, "unknown dwc3 event\n");
		break;
	}
}

static void dwc3_msm_block_reset(struct dwc3_ext_xceiv *xceiv, bool core_reset)
{
	struct dwc3_msm *mdwc = container_of(xceiv, struct dwc3_msm, ext_xceiv);
	int ret  = 0;

	if (core_reset) {
		ret = dwc3_msm_link_clk_reset(mdwc, 1);
		if (ret)
			return;

		usleep_range(1000, 1200);
		ret = dwc3_msm_link_clk_reset(mdwc, 0);
		if (ret)
			return;

		usleep_range(10000, 12000);
	}

	if (mdwc->dbm) {
		/* Reset the DBM */
		dbm_soft_reset(mdwc->dbm, 1);
		usleep_range(1000, 1200);
		dbm_soft_reset(mdwc->dbm, 0);

		/*enable DBM*/
		dwc3_msm_write_reg_field(mdwc->base, QSCRATCH_GENERAL_CFG,
			DBM_EN_MASK, 0x1);
		dbm_enable(mdwc->dbm);
	}
}

static void dwc3_chg_enable_secondary_det(struct dwc3_msm *mdwc)
{
	u32 chg_ctrl;

	/* Turn off VDP_SRC */
	dwc3_msm_write_reg(mdwc->base, CHARGING_DET_CTRL_REG, 0x0);
	msleep(20);

	/* Before proceeding make sure VDP_SRC is OFF */
	chg_ctrl = dwc3_msm_read_reg(mdwc->base, CHARGING_DET_CTRL_REG);
	if (chg_ctrl & 0x3F)
		dev_err(mdwc->dev, "%s Unable to reset chg_det block: %x\n",
						 __func__, chg_ctrl);
	/*
	 * Configure DM as current source, DP as current sink
	 * and enable battery charging comparators.
	 */
	dwc3_msm_write_readback(mdwc->base, CHARGING_DET_CTRL_REG, 0x3F, 0x34);
}

static bool dwc3_chg_det_check_linestate(struct dwc3_msm *mdwc)
{
	u32 chg_det;

	if (!prop_chg_detect)
		return false;

	chg_det = dwc3_msm_read_reg(mdwc->base, CHARGING_DET_OUTPUT_REG);
	return chg_det & (3 << 8);
}

static bool dwc3_chg_det_check_output(struct dwc3_msm *mdwc)
{
	u32 chg_det;
	bool ret = false;

	chg_det = dwc3_msm_read_reg(mdwc->base, CHARGING_DET_OUTPUT_REG);
	ret = chg_det & 1;

	return ret;
}

static void dwc3_chg_enable_primary_det(struct dwc3_msm *mdwc)
{
	/*
	 * Configure DP as current source, DM as current sink
	 * and enable battery charging comparators.
	 */
	dwc3_msm_write_readback(mdwc->base, CHARGING_DET_CTRL_REG, 0x3F, 0x30);
}

static inline bool dwc3_chg_check_dcd(struct dwc3_msm *mdwc)
{
	u32 chg_state;
	bool ret = false;

	chg_state = dwc3_msm_read_reg(mdwc->base, CHARGING_DET_OUTPUT_REG);
	ret = chg_state & 2;

	return ret;
}

static inline void dwc3_chg_disable_dcd(struct dwc3_msm *mdwc)
{
	dwc3_msm_write_readback(mdwc->base, CHARGING_DET_CTRL_REG, 0x3F, 0x0);
}

static inline void dwc3_chg_enable_dcd(struct dwc3_msm *mdwc)
{
	/* Data contact detection enable, DCDENB */
	dwc3_msm_write_readback(mdwc->base, CHARGING_DET_CTRL_REG, 0x3F, 0x2);
}

static void dwc3_chg_block_reset(struct dwc3_msm *mdwc)
{
	u32 chg_ctrl;

	dwc3_msm_write_reg(mdwc->base, QSCRATCH_CTRL_REG,
			mdwc->qscratch_ctl_val);
	/* Clear charger detecting control bits */
	dwc3_msm_write_reg(mdwc->base, CHARGING_DET_CTRL_REG, 0x0);

	/* Clear alt interrupt latch and enable bits */
	dwc3_msm_write_reg(mdwc->base, HS_PHY_IRQ_STAT_REG, 0xFFF);
	dwc3_msm_write_reg(mdwc->base, ALT_INTERRUPT_EN_REG, 0x0);

	udelay(100);

	/* Before proceeding make sure charger block is RESET */
	chg_ctrl = dwc3_msm_read_reg(mdwc->base, CHARGING_DET_CTRL_REG);
	if (chg_ctrl & 0x3F)
		dev_err(mdwc->dev, "%s Unable to reset chg_det block: %x\n",
						 __func__, chg_ctrl);
}

static const char *chg_to_string(enum dwc3_chg_type chg_type)
{
	switch (chg_type) {
	case DWC3_SDP_CHARGER:		return "USB_SDP_CHARGER";
	case DWC3_DCP_CHARGER:		return "USB_DCP_CHARGER";
	case DWC3_CDP_CHARGER:		return "USB_CDP_CHARGER";
	case DWC3_PROPRIETARY_CHARGER:	return "USB_PROPRIETARY_CHARGER";
#ifdef CONFIG_SONY_USB_EXTENSIONS
	case DWC3_PROPRIETARY_1000MA:	return "USB_PROPRIETARY_1000MA";
	case DWC3_PROPRIETARY_500MA:	return "USB_PROPRIETARY_500MA";
#endif
	case DWC3_FLOATED_CHARGER:	return "USB_FLOATED_CHARGER";
	default:			return "UNKNOWN_CHARGER";
	}
}

#define DWC3_CHG_DCD_POLL_TIME		(100 * HZ/1000) /* 100 msec */
#define DWC3_CHG_DCD_MAX_RETRIES	6 /* Tdcd_tmout = 6 * 100 msec */
#define DWC3_CHG_PRIMARY_DET_TIME	(50 * HZ/1000) /* TVDPSRC_ON */
#define DWC3_CHG_SECONDARY_DET_TIME	(50 * HZ/1000) /* TVDMSRC_ON */

#ifdef CONFIG_SONY_USB_EXTENSIONS
static int
get_prop_usb_dp_voltage_now(struct dwc3_msm *mdwc)
{
	int rc = 0;
	struct qpnp_vadc_result results;

	if (IS_ERR_OR_NULL(mdwc->vadc_usb_dp)) {
		mdwc->vadc_usb_dp = qpnp_get_vadc(mdwc->dev, "usb_dp");
		if (IS_ERR(mdwc->vadc_usb_dp))
			return PTR_ERR(mdwc->vadc_usb_dp);
	}

	rc = qpnp_vadc_read(mdwc->vadc_usb_dp, USB_DP, &results);
	if (rc) {
		pr_err("Unable to read usb_dp rc=%d\n", rc);
		return 0;
	} else {
		return results.physical;
	}
}

static int
get_prop_usb_dm_voltage_now(struct dwc3_msm *mdwc)
{
	int rc = 0;
	struct qpnp_vadc_result results;

	if (IS_ERR_OR_NULL(mdwc->vadc_usb_dm)) {
		mdwc->vadc_usb_dm = qpnp_get_vadc(mdwc->dev, "usb_dm");
		if (IS_ERR(mdwc->vadc_usb_dm))
			return PTR_ERR(mdwc->vadc_usb_dm);
	}

	rc = qpnp_vadc_read(mdwc->vadc_usb_dm, USB_DM, &results);
	if (rc) {
		pr_err("Unable to read usb_dm rc=%d\n", rc);
		return 0;
	} else {
		return results.physical;
	}
}

static int
get_prop_proprietary_charger(struct dwc3_msm *mdwc)
{
	#define IN_RANGE(val, hi, lo) ((hi >= val) && (val >= lo))
	struct {
		struct {
			int hi;
			int lo;
		} dp, dm;
		int type;
		char *name;
	} chgs[] = {
		{{3600000, 3000000}, {3600000, 3000000},
				DWC3_PROPRIETARY_CHARGER, "Sony"},
		{{3000000, 2400000}, {3000000, 2400000},
				DWC3_PROPRIETARY_CHARGER, "Proprietary 12w"},
		{{3000000, 2400000}, {2300000, 1700000},
				DWC3_PROPRIETARY_CHARGER, "Proprietary 10w"},
		{{2300000, 1700000}, {3000000, 2400000},
				DWC3_PROPRIETARY_1000MA, "Proprietary 5w"},
		{{2300000, 1700000}, {2300000, 1700000},
				DWC3_PROPRIETARY_500MA, "Proprietary 2.5w"},
	};
	int dp, dm;
	int ret;
	int i;

	dp = get_prop_usb_dp_voltage_now(mdwc);
	if (IS_ERR_VALUE(dp)) {
		dev_err(mdwc->dev, "%s: read D+ voltage fail\n", __func__);
		return dp;
	}

	dm = get_prop_usb_dm_voltage_now(mdwc);
	if (IS_ERR_VALUE(dm)) {
		dev_err(mdwc->dev, "%s: read D- voltage fail\n", __func__);
		return dm;
	}

	ret = DWC3_INVALID_CHARGER;
	for (i = 0; i < (sizeof(chgs) / sizeof(chgs[0])); i++) {
		if (IN_RANGE(dp, chgs[i].dp.hi, chgs[i].dp.lo) &&
				IN_RANGE(dm, chgs[i].dm.hi, chgs[i].dm.lo)) {
			dev_dbg(mdwc->dev, "%s: %s charger, D+=%d, D-=%d\n",
					__func__, chgs[i].name, dp, dm);
			ret = chgs[i].type;
			break;
		}
	}

	if (ret == DWC3_INVALID_CHARGER) {
		dev_warn(mdwc->dev, "%s: voltage not in range, D+=%d, D-=%d\n",
							__func__, dp, dm);
		ret = -ERANGE;
	}

	return ret;
}
#endif

static void dwc3_chg_detect_work(struct work_struct *w)
{
	struct dwc3_msm *mdwc = container_of(w, struct dwc3_msm, chg_work.work);
	bool is_dcd = false, tmout, vout;
	static bool dcd;
	unsigned long delay;

	dev_dbg(mdwc->dev, "chg detection work, state=%d\n", mdwc->chg_state);
	switch (mdwc->chg_state) {
	case USB_CHG_STATE_UNDEFINED:
		pm_runtime_get_sync(mdwc->dev);
		dwc3_chg_block_reset(mdwc);
		dwc3_chg_enable_dcd(mdwc);
		mdwc->chg_state = USB_CHG_STATE_WAIT_FOR_DCD;
		mdwc->dcd_retries = 0;
		delay = DWC3_CHG_DCD_POLL_TIME;
		break;
	case USB_CHG_STATE_WAIT_FOR_DCD:
		is_dcd = dwc3_chg_check_dcd(mdwc);
		tmout = ++mdwc->dcd_retries == DWC3_CHG_DCD_MAX_RETRIES;
		if (is_dcd || tmout) {
#ifdef CONFIG_SONY_USB_EXTENSIONS
			int chg_type;
#endif
			if (is_dcd)
				dcd = true;
			else
				dcd = false;
			dwc3_chg_disable_dcd(mdwc);
			usleep_range(1000, 1200);
#ifdef CONFIG_SONY_USB_EXTENSIONS
			chg_type = get_prop_proprietary_charger(mdwc);
			if (!IS_ERR_VALUE(chg_type)) {
				mdwc->charger.chg_type = chg_type;
				mdwc->chg_state = USB_CHG_STATE_DETECTED;
				delay = 0;
				break;
			}
#endif
			if (dwc3_chg_det_check_linestate(mdwc)) {
				mdwc->charger.chg_type =
						DWC3_PROPRIETARY_CHARGER;
				mdwc->chg_state = USB_CHG_STATE_DETECTED;
				delay = 0;
				break;
			}
			dwc3_chg_enable_primary_det(mdwc);
			delay = DWC3_CHG_PRIMARY_DET_TIME;
			mdwc->chg_state = USB_CHG_STATE_DCD_DONE;
		} else {
			delay = DWC3_CHG_DCD_POLL_TIME;
		}
		break;
	case USB_CHG_STATE_DCD_DONE:
		vout = dwc3_chg_det_check_output(mdwc);
		if (vout) {
			dwc3_chg_enable_secondary_det(mdwc);
			delay = DWC3_CHG_SECONDARY_DET_TIME;
			mdwc->chg_state = USB_CHG_STATE_PRIMARY_DONE;
		} else {
			/*
			 * Detect floating charger only if propreitary
			 * charger detection is enabled.
			 */
			if (!dcd && prop_chg_detect)
				mdwc->charger.chg_type =
						DWC3_FLOATED_CHARGER;
#ifdef CONFIG_QPNP_SMBCHARGER_EXTENSION
			else if (!dcd)
				mdwc->charger.chg_type = DWC3_INVALID_CHARGER;
#endif
			else
				mdwc->charger.chg_type = DWC3_SDP_CHARGER;
			mdwc->chg_state = USB_CHG_STATE_DETECTED;
			delay = 0;
		}
		break;
	case USB_CHG_STATE_PRIMARY_DONE:
		vout = dwc3_chg_det_check_output(mdwc);
		if (vout)
			mdwc->charger.chg_type = DWC3_DCP_CHARGER;
		else
			mdwc->charger.chg_type = DWC3_CDP_CHARGER;
		mdwc->chg_state = USB_CHG_STATE_SECONDARY_DONE;
		/* fall through */
	case USB_CHG_STATE_SECONDARY_DONE:
		mdwc->chg_state = USB_CHG_STATE_DETECTED;
		/* fall through */
	case USB_CHG_STATE_DETECTED:
		dwc3_chg_block_reset(mdwc);
		/* Enable VDP_SRC */
		if (mdwc->charger.chg_type == DWC3_DCP_CHARGER) {
			dwc3_msm_write_readback(mdwc->base,
					CHARGING_DET_CTRL_REG, 0x1F, 0x10);
			if (mdwc->ext_chg_opened) {
				init_completion(&mdwc->ext_chg_wait);
				mdwc->ext_chg_active = true;
			}
		}
		dev_dbg(mdwc->dev, "chg_type = %s\n",
			chg_to_string(mdwc->charger.chg_type));
		mdwc->charger.notify_detection_complete(mdwc->otg_xceiv->otg,
								&mdwc->charger);
								return;
	default:
		pm_runtime_put_sync(mdwc->dev);
		return;
	}

	queue_delayed_work(system_nrt_wq, &mdwc->chg_work, delay);
}

static void dwc3_start_chg_det(struct dwc3_charger *charger, bool start)
{
	struct dwc3_msm *mdwc = container_of(charger, struct dwc3_msm, charger);

	if (start == false) {
		dev_dbg(mdwc->dev, "canceling charging detection work\n");
		cancel_delayed_work_sync(&mdwc->chg_work);
		if (mdwc->chg_state != USB_CHG_STATE_UNDEFINED &&
				mdwc->chg_state != USB_CHG_STATE_DETECTED)
			pm_runtime_put_sync(mdwc->dev);

		mdwc->chg_state = USB_CHG_STATE_UNDEFINED;
		charger->chg_type = DWC3_INVALID_CHARGER;
		return;
	}

	/* Skip if charger type was already detected externally */
	if (mdwc->chg_state == USB_CHG_STATE_DETECTED &&
		charger->chg_type != DWC3_INVALID_CHARGER)
		return;

	mdwc->chg_state = USB_CHG_STATE_UNDEFINED;
	charger->chg_type = DWC3_INVALID_CHARGER;
	queue_delayed_work(system_nrt_wq, &mdwc->chg_work, 0);
}

static void dwc3_msm_power_collapse_por(struct dwc3_msm *mdwc)
{
	struct dwc3 *dwc = platform_get_drvdata(mdwc->dwc3);
	dwc3_core_init(dwc);
	/* Re-configure event buffers */
	dwc3_event_buffers_setup(dwc);
	dwc3_msm_notify_event(dwc, DWC3_CONTROLLER_POST_INITIALIZATION_EVENT);
}

static int dwc3_msm_prepare_suspend(struct dwc3_msm *mdwc)
{
	unsigned long timeout;
	u32 reg = 0;

	if (dwc3_msm_is_superspeed(mdwc)) {
		if (!atomic_read(&mdwc->in_p3)) {
			dev_err(mdwc->dev,
				"Not in P3, stoping LPM sequence\n");
			return -EBUSY;
		}
	} else {
		/* Clear previous L2 events */
		dwc3_msm_write_reg(mdwc->base, PWR_EVNT_IRQ_STAT_REG,
			PWR_EVNT_LPM_IN_L2_MASK |
			PWR_EVNT_LPM_OUT_L2_MASK);

		/* Prepare HSPHY for suspend */
		dwc3_msm_write_reg(mdwc->base,
			DWC3_GUSB2PHYCFG(0),
			dwc3_msm_read_reg(mdwc->base,
			DWC3_GUSB2PHYCFG(0)) |
			DWC3_GUSB2PHYCFG_ENBLSLPM |
			DWC3_GUSB2PHYCFG_SUSPHY);

		/* Wait for PHY to go into L2 */
		timeout = jiffies + msecs_to_jiffies(5);
		while (!time_after(jiffies, timeout)) {
			reg = dwc3_msm_read_reg(mdwc->base,
				PWR_EVNT_IRQ_STAT_REG);
			if (reg & PWR_EVNT_LPM_IN_L2_MASK)
				break;
		}
		if (!(reg & PWR_EVNT_LPM_IN_L2_MASK)) {
			dev_err(mdwc->dev,
				"could not transition HS PHY to L2\n");
			return -EBUSY;
		}

		/* Clear L2 event bit */
		dwc3_msm_write_reg(mdwc->base, PWR_EVNT_IRQ_STAT_REG,
			PWR_EVNT_LPM_IN_L2_MASK);
	}

	return 0;
}

static void dwc3_msm_wake_interrupt_enable(struct dwc3_msm *mdwc, bool on)
{
	u32 irq_mask, irq_stat;
	u32 wakeup_events = PWR_EVNT_POWERDOWN_OUT_P3_MASK |
		PWR_EVNT_LPM_OUT_L2_MASK;

	irq_stat = dwc3_msm_read_reg(mdwc->base, PWR_EVNT_IRQ_STAT_REG);

	/* clear pending interrupts */
	dwc3_msm_write_reg(mdwc->base, PWR_EVNT_IRQ_STAT_REG, irq_stat);

	irq_mask = dwc3_msm_read_reg(mdwc->base, PWR_EVNT_IRQ_MASK_REG);

	if (on) /* Enable P3 and L2 OUT events */
		irq_mask |= wakeup_events;
	else /* Disable P3 and L2 OUT events */
		irq_mask &= ~wakeup_events;

	dwc3_msm_write_reg(mdwc->base, PWR_EVNT_IRQ_MASK_REG, irq_mask);
}

static void dwc3_msm_bus_vote_w(struct work_struct *w)
{
	struct dwc3_msm *mdwc = container_of(w, struct dwc3_msm, bus_vote_w);
	int ret;

	ret = msm_bus_scale_client_update_request(mdwc->bus_perf_client,
			mdwc->bus_vote);
	if (ret)
		dev_err(mdwc->dev, "Failed to reset bus bw vote %d\n", ret);
}

static int dwc3_msm_suspend(struct dwc3_msm *mdwc)
{
	int ret, i;
	bool dcp;
	bool host_bus_suspend;
	bool host_ss_active;
	bool can_suspend_ssphy;
	bool device_bus_suspend = false;
	struct dwc3 *dwc = platform_get_drvdata(mdwc->dwc3);

	dev_dbg(mdwc->dev, "%s: entering lpm. usb_lpm_override:%d\n",
					 __func__, usb_lpm_override);
	dbg_event(0xFF, "Ctl Sus", atomic_read(&dwc->in_lpm));

	if (!usb_lpm_override && mdwc->suspend_resume_no_support) {
		dev_dbg(mdwc->dev, "%s no support for suspend\n", __func__);
		return -EPERM;
	}

	if (atomic_read(&dwc->in_lpm)) {
		dev_dbg(mdwc->dev, "%s: Already suspended\n", __func__);
		return 0;
	}

	if (mdwc->otg_xceiv && mdwc->otg_xceiv->state == OTG_STATE_B_SUSPEND)
		device_bus_suspend = true;

	host_bus_suspend = (mdwc->scope == POWER_SUPPLY_SCOPE_SYSTEM);

	if (!host_bus_suspend) {
		/* pending device events unprocessed */
		for (i = 0; i < dwc->num_event_buffers; i++) {
			struct dwc3_event_buffer *evt = dwc->ev_buffs[i];
			if ((evt->flags & DWC3_EVENT_PENDING)) {
				dev_dbg(mdwc->dev,
				"%s: %d device events pending, abort suspend\n",
				__func__, evt->count / 4);
				dbg_print_reg("PENDING DEVICE EVENT",
						*(u32 *)(evt->buf + evt->lpos));
				return -EBUSY;
			}
		}

		if (!msm_bam_usb_lpm_ok(DWC3_CTRL)) {
			dev_dbg(mdwc->dev, "%s: IPA handshake not finished, will suspend when done\n",
					__func__);
			return -EBUSY;
		}
	}

	if (!mdwc->vbus_active && mdwc->otg_xceiv &&
		mdwc->otg_xceiv->state == OTG_STATE_B_PERIPHERAL) {
		/*
		 * In some cases, the pm_runtime_suspend may be called by
		 * usb_bam when there is pending lpm flag. However, if this is
		 * done when cable was disconnected and otg state has not
		 * yet changed to IDLE, then it means OTG state machine
		 * is running and we race against it. So cancel LPM for now,
		 * and OTG state machine will go for LPM later, after completing
		 * transition to IDLE state.
		*/
		dev_dbg(mdwc->dev,
			"%s: cable disconnected while not in idle otg state\n",
			__func__);
		return -EBUSY;
	}

	host_ss_active = dwc3_msm_is_host_superspeed(mdwc);

	if (cancel_delayed_work_sync(&mdwc->chg_work))
		dev_err(mdwc->dev,
		"%s: chg_work was pending. mdwc dev pm usage cnt %d\n",
			__func__, atomic_read(&mdwc->dev->power.usage_count));
	if (mdwc->chg_state != USB_CHG_STATE_DETECTED) {
		/* charger detection wasn't complete; re-init flags */
		mdwc->chg_state = USB_CHG_STATE_UNDEFINED;
		mdwc->charger.chg_type = DWC3_INVALID_CHARGER;
		dwc3_msm_write_readback(mdwc->base, CHARGING_DET_CTRL_REG,
								0x37, 0x0);
	}

	dcp = ((mdwc->charger.chg_type == DWC3_DCP_CHARGER) ||
	      (mdwc->charger.chg_type == DWC3_PROPRIETARY_CHARGER) ||
	      (mdwc->charger.chg_type == DWC3_FLOATED_CHARGER));
#ifdef CONFIG_SONY_USB_EXTENSIONS
	dcp = dcp || (mdwc->charger.chg_type == DWC3_PROPRIETARY_1000MA) ||
	      (mdwc->charger.chg_type == DWC3_PROPRIETARY_500MA);
#endif
	if (dcp) {
		mdwc->hs_phy->flags |= PHY_CHARGER_CONNECTED;
		mdwc->ss_phy->flags |= PHY_CHARGER_CONNECTED;
	} else {
		mdwc->hs_phy->flags &= ~PHY_CHARGER_CONNECTED;
		mdwc->ss_phy->flags &= ~PHY_CHARGER_CONNECTED;
	}

	can_suspend_ssphy = !(host_bus_suspend && host_ss_active);

	if (host_bus_suspend || device_bus_suspend) {

		/*
		 * Check if device is not in CONFIGURED state
		 * then check cotroller state of L2 and break
		 * LPM sequeunce.
		*/
		if (device_bus_suspend &&
			(dwc->gadget.state != USB_STATE_CONFIGURED)) {
			pr_err("%s(): Trying to go in LPM with state:%d\n",
						__func__, dwc->gadget.state);
			pr_err("%s(): LPM is not performed.\n", __func__);
			return -EBUSY;
		}

		ret = dwc3_msm_prepare_suspend(mdwc);
		if (ret)
			return ret;
	}

	/* Disable core irq */
	if (dwc->irq)
		disable_irq(dwc->irq);

	if (!dcp && !host_bus_suspend)
		dwc3_msm_write_reg(mdwc->base, QSCRATCH_CTRL_REG,
			mdwc->qscratch_ctl_val);

	/* Enable wakeup from LPM */
	if (mdwc->pwr_event_irq) {
		disable_irq(mdwc->pwr_event_irq);
		dwc3_msm_wake_interrupt_enable(mdwc, true);
		enable_irq_wake(mdwc->pwr_event_irq);
	}

	usb_phy_set_suspend(mdwc->hs_phy, 1);

	if (can_suspend_ssphy) {
		usb_phy_set_suspend(mdwc->ss_phy, 1);
		usleep_range(1000, 1200);
		clk_disable_unprepare(mdwc->ref_clk);
		mdwc->lpm_flags |= MDWC3_PHY_REF_CLK_OFF;
	}

	/* make sure above writes are completed before turning off clocks */
	wmb();

	/* Perform controller power collapse */
	if (!host_bus_suspend && !device_bus_suspend && mdwc->power_collapse) {
		mdwc->lpm_flags |= MDWC3_POWER_COLLAPSE;
		dev_dbg(mdwc->dev, "%s: power collapse\n", __func__);
		dwc3_msm_config_gdsc(mdwc, 0);
		clk_disable_unprepare(mdwc->sleep_clk);
	}

	clk_disable_unprepare(mdwc->iface_clk);
	clk_disable_unprepare(mdwc->utmi_clk);

	if (can_suspend_ssphy) {
		clk_set_rate(mdwc->core_clk, 19200000);
		clk_disable_unprepare(mdwc->core_clk);
		mdwc->lpm_flags |= MDWC3_CORECLK_OFF;
		/* USB PHY no more requires TCXO */
		clk_disable_unprepare(mdwc->xo_clk);
		mdwc->lpm_flags |= MDWC3_TCXO_SHUTDOWN;
	}

	if (mdwc->bus_perf_client) {
		mdwc->bus_vote = 0;
		queue_work(system_nrt_wq, &mdwc->bus_vote_w);
	}

	/*
	 * release wakeup source with timeout to defer system suspend to
	 * handle case where on USB cable disconnect, SUSPEND and DISCONNECT
	 * event is received.
	 */
	if (mdwc->lpm_to_suspend_delay) {
		dev_dbg(mdwc->dev, "defer suspend with %d(msecs)\n",
					mdwc->lpm_to_suspend_delay);
		pm_wakeup_event(mdwc->dev, mdwc->lpm_to_suspend_delay);
	} else {
		pm_relax(mdwc->dev);
	}

	if (mdwc->hs_phy_irq) {
		/*
		 * with DCP or during cable disconnect, we dont require wakeup
		 * using HS_PHY_IRQ. Hence enable wakeup only in case of host
		 * bus suspend and device bus suspend.
		 */
		if (host_bus_suspend || device_bus_suspend) {
			enable_irq_wake(mdwc->hs_phy_irq);
			mdwc->lpm_flags |= MDWC3_ASYNC_IRQ_WAKE_CAPABILITY;
		}
	}

	atomic_set(&dwc->in_lpm, 1);
	if (mdwc->pwr_event_irq)
		enable_irq(mdwc->pwr_event_irq);

	dev_info(mdwc->dev, "DWC3 in low power mode\n");
	return 0;
}

static int dwc3_msm_resume(struct dwc3_msm *mdwc)
{
	int ret;
	bool dcp;
	bool host_bus_suspend;
	struct dwc3 *dwc = platform_get_drvdata(mdwc->dwc3);

	dev_dbg(mdwc->dev, "%s: exiting lpm\n", __func__);

	if (!atomic_read(&dwc->in_lpm)) {
		dev_dbg(mdwc->dev, "%s: Already resumed\n", __func__);
		return 0;
	}

	pm_stay_awake(mdwc->dev);

	if (mdwc->bus_perf_client) {
		mdwc->bus_vote = 1;
		queue_work(system_nrt_wq, &mdwc->bus_vote_w);
	}

	dcp = ((mdwc->charger.chg_type == DWC3_DCP_CHARGER) ||
	      (mdwc->charger.chg_type == DWC3_PROPRIETARY_CHARGER) ||
	      (mdwc->charger.chg_type == DWC3_FLOATED_CHARGER));
#ifdef CONFIG_SONY_USB_EXTENSIONS
	dcp = dcp ||(mdwc->charger.chg_type == DWC3_PROPRIETARY_1000MA) ||
	      (mdwc->charger.chg_type == DWC3_PROPRIETARY_500MA);
#endif
	if (dcp) {
		mdwc->hs_phy->flags |= PHY_CHARGER_CONNECTED;
		mdwc->ss_phy->flags |= PHY_CHARGER_CONNECTED;
	} else {
		mdwc->hs_phy->flags &= ~PHY_CHARGER_CONNECTED;
		mdwc->ss_phy->flags &= ~PHY_CHARGER_CONNECTED;
	}

	host_bus_suspend = (mdwc->scope == POWER_SUPPLY_SCOPE_SYSTEM);

	if (mdwc->lpm_flags & MDWC3_TCXO_SHUTDOWN) {
		/* Vote for TCXO while waking up USB HSPHY */
		ret = clk_prepare_enable(mdwc->xo_clk);
		if (ret)
			dev_err(mdwc->dev, "%s failed to vote TCXO buffer%d\n",
						__func__, ret);
		mdwc->lpm_flags &= ~MDWC3_TCXO_SHUTDOWN;
	}

	/* Restore controller power collapse */
	if (mdwc->lpm_flags & MDWC3_POWER_COLLAPSE) {
		dev_dbg(mdwc->dev, "%s: exit power collapse\n", __func__);
		dwc3_msm_config_gdsc(mdwc, 1);

		if (mdwc->phy_com_reset)
			clk_reset(mdwc->phy_com_reset, CLK_RESET_ASSERT);
		clk_reset(mdwc->core_clk, CLK_RESET_ASSERT);
		/* HW requires a short delay for reset to take place properly */
		usleep_range(1000, 1200);
		clk_reset(mdwc->core_clk, CLK_RESET_DEASSERT);
		if (mdwc->phy_com_reset)
			clk_reset(mdwc->phy_com_reset, CLK_RESET_DEASSERT);
	}


	if (mdwc->lpm_flags & MDWC3_PHY_REF_CLK_OFF) {
		clk_prepare_enable(mdwc->ref_clk);
		usb_phy_set_suspend(mdwc->ss_phy, 0);
		mdwc->lpm_flags &= ~MDWC3_PHY_REF_CLK_OFF;
	}
	usleep_range(1000, 1200);

	clk_prepare_enable(mdwc->iface_clk);
	if (mdwc->lpm_flags & MDWC3_CORECLK_OFF) {
		clk_set_rate(mdwc->core_clk, 125000000);
		clk_prepare_enable(mdwc->core_clk);
		mdwc->lpm_flags &= ~MDWC3_CORECLK_OFF;
	}

	clk_prepare_enable(mdwc->utmi_clk);

	if (mdwc->lpm_flags & MDWC3_POWER_COLLAPSE)
		clk_prepare_enable(mdwc->sleep_clk);

	usb_phy_set_suspend(mdwc->hs_phy, 0);

	/* Recover from controller power collapse */
	if (mdwc->lpm_flags & MDWC3_POWER_COLLAPSE) {
		dev_dbg(mdwc->dev, "%s: exit power collapse (POR=%d)\n",
			__func__, mdwc->power_collapse_por);

		/* Reset HSPHY */
		ret = usb_phy_reset(mdwc->hs_phy);
		if (ret) {
			dev_err(mdwc->dev, "hsphy reset failed\n");
			return ret;
		}

		/* Reset SS PHY */
		ret = usb_phy_reset(mdwc->ss_phy);
		if (ret) {
			dev_err(mdwc->dev, "ssphy reset failed\n");
			return ret;
		}

		ret = dwc3_msm_restore_sec_config(mdwc->scm_dev_id);
		if (ret)
			return ret;

		if (mdwc->power_collapse_por)
			dwc3_msm_power_collapse_por(mdwc);

		/* Re-enable IN_P3 event */
		dwc3_msm_write_reg_field(mdwc->base, PWR_EVNT_IRQ_MASK_REG,
					PWR_EVNT_POWERDOWN_IN_P3_MASK, 1);

		mdwc->lpm_flags &= ~MDWC3_POWER_COLLAPSE;
	}

	atomic_set(&dwc->in_lpm, 0);

	msm_bam_notify_lpm_resume(DWC3_CTRL);
	/* disable wakeup from LPM */
	if (mdwc->pwr_event_irq) {
		disable_irq_wake(mdwc->pwr_event_irq);
		dwc3_msm_wake_interrupt_enable(mdwc, false);
	}

	/* Disable HSPHY auto suspend */
	dwc3_msm_write_reg(mdwc->base, DWC3_GUSB2PHYCFG(0),
		dwc3_msm_read_reg(mdwc->base, DWC3_GUSB2PHYCFG(0)) &
				~(DWC3_GUSB2PHYCFG_ENBLSLPM |
					DWC3_GUSB2PHYCFG_SUSPHY));

	/* Disable wakeup capable for HS_PHY IRQ, if enabled */
	if (mdwc->hs_phy_irq &&
			(mdwc->lpm_flags & MDWC3_ASYNC_IRQ_WAKE_CAPABILITY)) {
			disable_irq_wake(mdwc->hs_phy_irq);
			mdwc->lpm_flags &= ~MDWC3_ASYNC_IRQ_WAKE_CAPABILITY;
	}

	dev_info(mdwc->dev, "DWC3 exited from low power mode\n");

	/* Enable core irq */
	if (dwc->irq)
		enable_irq(dwc->irq);

	/*
	 * Handle other power events that could not have been handled during
	 * Low Power Mode
	 */
	dwc3_pwr_event_handler(mdwc);

	dbg_event(0xFF, "Ctl Res", atomic_read(&dwc->in_lpm));

	return 0;
}

static void dwc3_wait_for_ext_chg_done(struct dwc3_msm *mdwc)
{
	unsigned long t;

	/*
	 * Defer next cable connect event till external charger
	 * detection is completed.
	 */

	if (mdwc->ext_chg_active && (mdwc->ext_xceiv.bsv ||
				!mdwc->ext_xceiv.id)) {

		dev_dbg(mdwc->dev, "before ext chg wait\n");

		t = wait_for_completion_timeout(&mdwc->ext_chg_wait,
				msecs_to_jiffies(3000));
		if (!t)
			dev_err(mdwc->dev, "ext chg wait timeout\n");
		else
			dev_dbg(mdwc->dev, "ext chg wait done\n");
	}

}

static void dwc3_resume_work(struct work_struct *w)
{
	struct dwc3_msm *mdwc = container_of(w, struct dwc3_msm,
							resume_work.work);
	struct dwc3 *dwc = platform_get_drvdata(mdwc->dwc3);

	dev_dbg(mdwc->dev, "%s: dwc3 resume work\n", __func__);

	/*
	 * exit LPM first to meet resume timeline from device side.
	 * resume_pending flag would prevent calling
	 * dwc3_msm_resume() in case we are here due to system
	 * wide resume without usb cable connected. This flag is set
	 * only in case of power event irq in lpm.
	 */
	if (mdwc->resume_pending) {
		dwc3_msm_resume(mdwc);
		mdwc->resume_pending = false;
	}

	if (atomic_read(&mdwc->pm_suspended)) {
		dbg_event(0xFF, "RWrk PMSus", 0);
		/* let pm resume kick in resume work later */
		return;
	}

	dbg_event(0xFF, "RWrk", mdwc->otg_xceiv ? 1 : 0);
	if (mdwc->otg_xceiv) {
		dwc3_wait_for_ext_chg_done(mdwc);
		mdwc->ext_xceiv.notify_ext_events(mdwc->otg_xceiv->otg);
	} else if (mdwc->scope == POWER_SUPPLY_SCOPE_SYSTEM) {
		/* host-only mode: resume xhci directly */
		pm_runtime_resume(&dwc->xhci->dev);
	}
}

static void dwc3_pwr_event_handler(struct dwc3_msm *mdwc)
{
	struct dwc3 *dwc = platform_get_drvdata(mdwc->dwc3);
	u32 irq_stat, irq_clear = 0;

	/*
	 * Increment the PM count to prevent suspend from happening
	 * during this routine. noresume is fine, since this function
	 * should only be called when not in LPM.
	 */
	pm_runtime_get_noresume(mdwc->dev);

	irq_stat = dwc3_msm_read_reg(mdwc->base, PWR_EVNT_IRQ_STAT_REG);
	dev_dbg(mdwc->dev, "%s irq_stat=%X\n", __func__, irq_stat);

	/* Check for P3 events */
	if ((irq_stat & PWR_EVNT_POWERDOWN_OUT_P3_MASK) &&
			(irq_stat & PWR_EVNT_POWERDOWN_IN_P3_MASK)) {
		/* Can't tell if entered or exit P3, so check LINKSTATE */
		u32 ls = dwc3_msm_read_reg_field(mdwc->base,
				DWC3_GDBGLTSSM, DWC3_GDBGLTSSM_LINKSTATE_MASK);
		dev_dbg(mdwc->dev, "%s link state = 0x%04x\n", __func__, ls);
		atomic_set(&mdwc->in_p3, ls == DWC3_LINK_STATE_U3);

		irq_stat &= ~(PWR_EVNT_POWERDOWN_OUT_P3_MASK |
				PWR_EVNT_POWERDOWN_IN_P3_MASK);
		irq_clear |= (PWR_EVNT_POWERDOWN_OUT_P3_MASK |
				PWR_EVNT_POWERDOWN_IN_P3_MASK);
	} else if (irq_stat & PWR_EVNT_POWERDOWN_OUT_P3_MASK) {
		atomic_set(&mdwc->in_p3, 0);
		irq_stat &= ~PWR_EVNT_POWERDOWN_OUT_P3_MASK;
		irq_clear |= PWR_EVNT_POWERDOWN_OUT_P3_MASK;
	} else if (irq_stat & PWR_EVNT_POWERDOWN_IN_P3_MASK) {
		atomic_set(&mdwc->in_p3, 1);
		irq_stat &= ~PWR_EVNT_POWERDOWN_IN_P3_MASK;
		irq_clear |= PWR_EVNT_POWERDOWN_IN_P3_MASK;
	}

	/* Clear L2 exit */
	if (irq_stat & PWR_EVNT_LPM_OUT_L2_MASK) {
		irq_stat &= ~PWR_EVNT_LPM_OUT_L2_MASK;
		irq_stat |= PWR_EVNT_LPM_OUT_L2_MASK;
	}

	/* Handle exit from L1 events */
	if (irq_stat & PWR_EVNT_LPM_OUT_L1_MASK) {
		dev_dbg(mdwc->dev, "%s: handling PWR_EVNT_LPM_OUT_L1_MASK\n",
				__func__);
		if (usb_gadget_wakeup(&dwc->gadget))
			dev_err(mdwc->dev, "%s failed to take dwc out of L1\n",
					__func__);
		irq_stat &= ~PWR_EVNT_LPM_OUT_L1_MASK;
		irq_clear |= PWR_EVNT_LPM_OUT_L1_MASK;
	}

	/* Unhandled events */
	if (irq_stat)
		dev_dbg(mdwc->dev, "%s: unexpected PWR_EVNT, irq_stat=%X\n",
			__func__, irq_stat);

	dwc3_msm_write_reg(mdwc->base, PWR_EVNT_IRQ_STAT_REG, irq_clear);
	pm_runtime_put(mdwc->dev);
}

static irqreturn_t msm_dwc3_pwr_irq_thread(int irq, void *_mdwc)
{
	struct dwc3_msm *mdwc = _mdwc;
	struct dwc3 *dwc = platform_get_drvdata(mdwc->dwc3);

	dev_dbg(mdwc->dev, "%s\n", __func__);

	if (atomic_read(&dwc->in_lpm))
		dwc3_resume_work(&mdwc->resume_work.work);
	else
		dwc3_pwr_event_handler(mdwc);

	dbg_event(0xFF, "PWR IRQ", atomic_read(&dwc->in_lpm));

	return IRQ_HANDLED;
}

static irqreturn_t msm_dwc3_hs_phy_irq(int irq, void *data)
{
	struct dwc3_msm *mdwc = data;

	dev_dbg(mdwc->dev, "%s HS PHY IRQ handled\n", __func__);
	return IRQ_HANDLED;
}

static irqreturn_t msm_dwc3_pwr_irq(int irq, void *data)
{
	struct dwc3_msm *mdwc = data;
	struct dwc3 *dwc = platform_get_drvdata(mdwc->dwc3);

	dwc->t_pwr_evt_irq = ktime_get();
	dev_dbg(mdwc->dev, "%s received\n", __func__);
	/*
	 * When in Low Power Mode, can't read PWR_EVNT_IRQ_STAT_REG to acertain
	 * which interrupts have been triggered, as the clocks are disabled.
	 * Resume controller by waking up pwr event irq thread.After re-enabling
	 * clocks, dwc3_msm_resume will call dwc3_pwr_event_handler to handle
	 * all other power events.
	 */
	if (atomic_read(&dwc->in_lpm)) {
		/* set this to call dwc3_msm_resume() */
		mdwc->resume_pending = true;
		return IRQ_WAKE_THREAD;
	}

	dwc3_pwr_event_handler(mdwc);
	return IRQ_HANDLED;
}

static int dwc3_msm_remove_pulldown(struct dwc3_msm *mdwc, bool rm_pulldown)
{
	int ret = 0;

	if (!rm_pulldown)
		goto disable_vdda33;

	ret = regulator_set_optimum_mode(mdwc->vdda18, DWC3_1P8_HPM_LOAD);
	if (ret < 0) {
		dev_err(mdwc->dev, "Unable to set HPM of vdda18:%d\n", ret);
		return ret;
	}

	ret = regulator_set_voltage(mdwc->vdda18, DWC3_1P8_VOL_MIN,
						DWC3_1P8_VOL_MAX);
	if (ret) {
		dev_err(mdwc->dev,
				"Unable to set voltage for vdda18:%d\n", ret);
		goto put_vdda18_lpm;
	}

	ret = regulator_enable(mdwc->vdda18);
	if (ret) {
		dev_err(mdwc->dev, "Unable to enable vdda18:%d\n", ret);
		goto unset_vdda18;
	}

	ret = regulator_set_optimum_mode(mdwc->vdda33, DWC3_3P3_HPM_LOAD);
	if (ret < 0) {
		dev_err(mdwc->dev, "Unable to set HPM of vdda33:%d\n", ret);
		goto disable_vdda18;
	}

	ret = regulator_set_voltage(mdwc->vdda33, DWC3_3P3_VOL_MIN,
						DWC3_3P3_VOL_MAX);
	if (ret) {
		dev_err(mdwc->dev,
				"Unable to set voltage for vdda33:%d\n", ret);
		goto put_vdda33_lpm;
	}

	ret = regulator_enable(mdwc->vdda33);
	if (ret) {
		dev_err(mdwc->dev, "Unable to enable vdda33:%d\n", ret);
		goto unset_vdd33;
	}

	return 0;

disable_vdda33:
	ret = regulator_disable(mdwc->vdda33);
	if (ret)
		dev_err(mdwc->dev, "Unable to disable vdda33:%d\n", ret);

unset_vdd33:
	ret = regulator_set_voltage(mdwc->vdda33, 0, DWC3_3P3_VOL_MAX);
	if (ret)
		dev_err(mdwc->dev,
			"Unable to set (0) voltage for vdda33:%d\n", ret);

put_vdda33_lpm:
	ret = regulator_set_optimum_mode(mdwc->vdda33, 0);
	if (ret < 0)
		dev_err(mdwc->dev, "Unable to set (0) HPM of vdda33\n");

disable_vdda18:
	ret = regulator_disable(mdwc->vdda18);
	if (ret)
		dev_err(mdwc->dev, "Unable to disable vdda18:%d\n", ret);

unset_vdda18:
	ret = regulator_set_voltage(mdwc->vdda18, 0, DWC3_1P8_VOL_MAX);
	if (ret)
		dev_err(mdwc->dev,
			"Unable to set (0) voltage for vdda18:%d\n", ret);

put_vdda18_lpm:
	ret = regulator_set_optimum_mode(mdwc->vdda18, 0);
	if (ret < 0)
		dev_err(mdwc->dev, "Unable to set LPM of vdda18\n");

	return ret;
}

static int
get_prop_usbin_voltage_now(struct dwc3_msm *mdwc)
{
	int rc = 0;
	struct qpnp_vadc_result results;

	if (IS_ERR_OR_NULL(mdwc->vadc_dev)) {
		mdwc->vadc_dev = qpnp_get_vadc(mdwc->dev, "usbin");
		if (IS_ERR(mdwc->vadc_dev))
			return PTR_ERR(mdwc->vadc_dev);
	}

	rc = qpnp_vadc_read(mdwc->vadc_dev, USBIN, &results);
	if (rc) {
		pr_err("Unable to read usbin rc=%d\n", rc);
		return 0;
	} else {
		return results.physical;
	}
}

static int dwc3_msm_pmic_dp_dm(struct dwc3_msm *mdwc, int value)
{
	int ret = 0;

	switch (value) {
	case POWER_SUPPLY_DP_DM_DPF_DMF:
		if (!mdwc->rm_pulldown) {
			ret = dwc3_msm_remove_pulldown(mdwc, true);
			if (!ret) {
				mdwc->rm_pulldown = true;
				dbg_event(0xFF, "RM PuDwn", mdwc->rm_pulldown);
			}
		}
		break;
	case POWER_SUPPLY_DP_DM_DPR_DMR:
		if (mdwc->rm_pulldown) {
			ret = dwc3_msm_remove_pulldown(mdwc, false);
			if (!ret) {
				mdwc->rm_pulldown = false;
				dbg_event(0xFF, "RM PuDwn", mdwc->rm_pulldown);
			}
		}
		break;
	default:
		ret = -EINVAL;
		break;
	}
	return ret;
}

static int dwc3_msm_power_get_property_usb(struct power_supply *psy,
				  enum power_supply_property psp,
				  union power_supply_propval *val)
{
	struct dwc3_msm *mdwc = container_of(psy, struct dwc3_msm,
								usb_psy);
	switch (psp) {
	case POWER_SUPPLY_PROP_SCOPE:
		val->intval = mdwc->scope;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_MAX:
		val->intval = mdwc->voltage_max;
		break;
	case POWER_SUPPLY_PROP_CURRENT_MAX:
		val->intval = mdwc->current_max;
		break;
	case POWER_SUPPLY_PROP_PRESENT:
		val->intval = mdwc->vbus_active;
		break;
	case POWER_SUPPLY_PROP_ONLINE:
		val->intval = mdwc->online;
		break;
	case POWER_SUPPLY_PROP_TYPE:
		val->intval = psy->type;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		val->intval = get_prop_usbin_voltage_now(mdwc);
		break;
	case POWER_SUPPLY_PROP_HEALTH:
		val->intval = mdwc->health_status;
		break;
	case POWER_SUPPLY_PROP_USB_OTG:
		val->intval = !mdwc->id_state;
		break;
#ifdef CONFIG_SONY_USB_EXTENSIONS
	case POWER_SUPPLY_PROP_CHARGER_TYPE:
		if (psy->type == POWER_SUPPLY_TYPE_USB_HVDCP) {
			val->strval = "USB_HVDCP_CHARGER";
		} else {
			val->strval = chg_to_string(mdwc->charger.chg_type);
		}
		break;
#endif
	default:
		return -EINVAL;
	}
	return 0;
}

static int dwc3_msm_power_set_property_usb(struct power_supply *psy,
				  enum power_supply_property psp,
				  const union power_supply_propval *val)
{
	struct dwc3_msm *mdwc = container_of(psy, struct dwc3_msm,
								usb_psy);
	struct dwc3 *dwc = platform_get_drvdata(mdwc->dwc3);
	struct dwc3_otg *dotg = dwc->dotg;
	struct usb_phy *phy = dotg->otg.phy;

	switch (psp) {
	case POWER_SUPPLY_PROP_USB_OTG:
		/* Let OTG know about ID detection */
		mdwc->id_state = val->intval ? DWC3_ID_GROUND : DWC3_ID_FLOAT;
		if (mdwc->otg_xceiv) {
#ifdef CONFIG_SONY_USB_EXTENSIONS
			wake_lock_timeout(&mdwc->id_wakelock,
				USB_ID_WAKE_LOCK_TIMEOUT);
#endif
			queue_work(system_nrt_wq, &mdwc->id_work);
		}

		break;
	case POWER_SUPPLY_PROP_SCOPE:
		mdwc->scope = val->intval;
		if (mdwc->scope == POWER_SUPPLY_SCOPE_SYSTEM) {
			mdwc->hs_phy->flags |= PHY_HOST_MODE;
			mdwc->ss_phy->flags |= PHY_HOST_MODE;
		} else {
			mdwc->hs_phy->flags &= ~PHY_HOST_MODE;
			mdwc->ss_phy->flags &= ~PHY_HOST_MODE;
		}
		break;
	/* PMIC notification for DP_DM state */
	case POWER_SUPPLY_PROP_DP_DM:
		dwc3_msm_pmic_dp_dm(mdwc, val->intval);
		break;
#ifdef CONFIG_SONY_USB_EXTENSIONS
	case POWER_SUPPLY_PROP_USBIN_DET:
		dev_dbg(mdwc->dev, "%s: notify ext_bsv event %d\n",
							__func__, val->intval);
		mdwc->ext_bsv = val->intval ? true : false;
		if (mdwc->otg_xceiv && !mdwc->ext_inuse &&
					!mdwc->ext_switching &&
					mdwc->ext_xceiv.id) {
			if (val->intval)
				/* connect D+- to phy if VBUS is hi */
				dwc3_select_mhl_switch(mdwc,
							MHL_SWITCH_PORT_USB1);
			else
				/* ground D+- if VBUS is lo */
				dwc3_select_mhl_switch(mdwc,
							MHL_SWITCH_PORT_USB2);
		}
		return 0;
#endif
	/* Process PMIC notification in PRESENT prop */
	case POWER_SUPPLY_PROP_PRESENT:
		dev_dbg(mdwc->dev, "%s: notify xceiv event\n", __func__);
		mdwc->vbus_active = val->intval;
		if (mdwc->otg_xceiv && !mdwc->ext_inuse && !mdwc->in_restart) {
			if (mdwc->ext_xceiv.bsv == val->intval)
				break;

			mdwc->ext_xceiv.bsv = val->intval;
			/*
			 * Set debouncing delay to 120ms. Otherwise battery
			 * charging CDP complaince test fails if delay > 120ms.
			 */
			dbg_event(0xFF, "Q RW (vbus)", val->intval);
			queue_delayed_work(system_nrt_wq,
							&mdwc->resume_work, 12);
		}
		break;
	case POWER_SUPPLY_PROP_ONLINE:
		mdwc->online = val->intval;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_MAX:
		mdwc->voltage_max = val->intval;
		break;
	case POWER_SUPPLY_PROP_CURRENT_MAX:
		mdwc->current_max = val->intval;
		break;
	case POWER_SUPPLY_PROP_TYPE:
#ifdef CONFIG_SONY_USB_EXTENSIONS
		if (mdwc->otg_xceiv) {
			if ((mdwc->ext_inuse || mdwc->ext_switching) &&
			    (POWER_SUPPLY_TYPE_UNKNOWN != val->intval))
				return 0;
		}
		/* If it already received a notification of HVDCP,
		 * skip this setting of power_supply to avoid overwrite by DCP.
		 */
		if (psy->type == POWER_SUPPLY_TYPE_USB_HVDCP &&
				val->intval == POWER_SUPPLY_TYPE_USB_DCP)
			return 0;
#endif
		psy->type = val->intval;

		switch (psy->type) {
		case POWER_SUPPLY_TYPE_USB:
			mdwc->charger.chg_type = DWC3_SDP_CHARGER;
			break;
		case POWER_SUPPLY_TYPE_USB_DCP:
#ifdef CONFIG_SONY_USB_EXTENSIONS
			if (dotg->charger->chg_type == DWC3_INVALID_CHARGER)
#endif
			mdwc->charger.chg_type = DWC3_DCP_CHARGER;
			break;
		case POWER_SUPPLY_TYPE_USB_HVDCP:
			mdwc->charger.chg_type = DWC3_DCP_CHARGER;
			usb_phy_set_power(phy, hvdcp_max_current);
			break;
		case POWER_SUPPLY_TYPE_USB_CDP:
			mdwc->charger.chg_type = DWC3_CDP_CHARGER;
			break;
		case POWER_SUPPLY_TYPE_USB_ACA:
			mdwc->charger.chg_type = DWC3_PROPRIETARY_CHARGER;
			break;
		default:
			mdwc->charger.chg_type = DWC3_INVALID_CHARGER;
			break;
		}

#ifdef CONFIG_SONY_USB_EXTENSIONS
		/* set INVALID_CHARGER to start the charger detection and
		 * kick the ID detection because qpnp-smbcharger reports
		 * FLOATED charger and Powered MHL as POWER_SUPPLY_TYPE_USB
		 * in other than SDP.
		 */
		if ((mdwc->charger.chg_type == DWC3_SDP_CHARGER) &&
				(mdwc->chg_state != USB_CHG_STATE_DETECTED)) {
			dev_dbg(mdwc->dev, "%s: SDP/MHL/FLOATED\n", __func__);
			mdwc->charger.chg_type = DWC3_INVALID_CHARGER;
			wake_lock_timeout(&mdwc->id_wakelock,
						USB_ID_WAKE_LOCK_TIMEOUT);
			queue_work(system_nrt_wq, &mdwc->id_work);
		}
#endif

		if (mdwc->charger.chg_type != DWC3_INVALID_CHARGER)
			mdwc->chg_state = USB_CHG_STATE_DETECTED;

		dev_dbg(mdwc->dev, "%s: charger type: %s\n", __func__,
				chg_to_string(mdwc->charger.chg_type));
		break;
	case POWER_SUPPLY_PROP_HEALTH:
		mdwc->health_status = val->intval;
		break;
	default:
		return -EINVAL;
	}

	power_supply_changed(&mdwc->usb_psy);
	return 0;
}

static void dwc3_msm_external_power_changed(struct power_supply *psy)
{
	struct dwc3_msm *mdwc = container_of(psy, struct dwc3_msm, usb_psy);
	union power_supply_propval ret = {0,};

	if (!mdwc->ext_vbus_psy)
		mdwc->ext_vbus_psy = power_supply_get_by_name("ext-vbus");

	if (!mdwc->ext_vbus_psy) {
		pr_err("%s: Unable to get ext_vbus power_supply\n", __func__);
		return;
	}

	mdwc->ext_vbus_psy->get_property(mdwc->ext_vbus_psy,
					POWER_SUPPLY_PROP_ONLINE, &ret);
	if (ret.intval) {
		dwc3_start_chg_det(&mdwc->charger, false);
		mdwc->ext_vbus_psy->get_property(mdwc->ext_vbus_psy,
					POWER_SUPPLY_PROP_CURRENT_MAX, &ret);
#ifndef CONFIG_SONY_USB_EXTENSIONS
		power_supply_set_current_limit(&mdwc->usb_psy, ret.intval);
#endif
	}

#ifdef CONFIG_SONY_USB_EXTENSIONS
	power_supply_set_current_limit(&mdwc->usb_psy, ret.intval);
#endif
	power_supply_set_online(&mdwc->usb_psy, ret.intval);
	power_supply_changed(&mdwc->usb_psy);
}

static int
dwc3_msm_property_is_writeable(struct power_supply *psy,
				enum power_supply_property psp)
{
	switch (psp) {
	case POWER_SUPPLY_PROP_USB_OTG:
	case POWER_SUPPLY_PROP_PRESENT:
	case POWER_SUPPLY_PROP_VOLTAGE_MAX:
		return 1;
	default:
		break;
	}

	return 0;
}


static char *dwc3_msm_pm_power_supplied_to[] = {
	"battery",
	"bms",
};

static enum power_supply_property dwc3_msm_pm_power_props_usb[] = {
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_VOLTAGE_MAX,
	POWER_SUPPLY_PROP_CURRENT_MAX,
	POWER_SUPPLY_PROP_TYPE,
	POWER_SUPPLY_PROP_SCOPE,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_HEALTH,
	POWER_SUPPLY_PROP_USB_OTG,
#ifdef CONFIG_SONY_USB_EXTENSIONS
	POWER_SUPPLY_PROP_CHARGER_TYPE,
#endif
};

static void dwc3_init_adc_work(struct work_struct *w);

static void dwc3_ext_notify_online(void *ctx, int on)
{
	struct dwc3_msm *mdwc = ctx;
	bool notify_otg = false;
#ifdef CONFIG_SONY_USB_EXTENSIONS
	int ext_inused = 0;
#endif

	if (!mdwc) {
		pr_err("%s: DWC3 driver already removed\n", __func__);
		return;
	}

	dev_dbg(mdwc->dev, "notify %s%s\n", on ? "" : "dis", "connected");

	if (!mdwc->ext_vbus_psy)
		mdwc->ext_vbus_psy = power_supply_get_by_name("ext-vbus");

#ifdef CONFIG_SONY_USB_EXTENSIONS
	ext_inused = mdwc->ext_inuse;
#endif

	mdwc->ext_inuse = on;
	if (on) {
		/* force OTG to exit B-peripheral state */
		mdwc->ext_xceiv.bsv = false;
#ifdef CONFIG_SONY_USB_EXTENSIONS
		/*
		 * In race condition, ID value might not be updated
		 * even through ID pin is switched to MHL
		 */
		mdwc->ext_xceiv.id = DWC3_ID_FLOAT;
#endif
		notify_otg = true;
		dwc3_start_chg_det(&mdwc->charger, false);
	} else
#ifdef CONFIG_SONY_USB_EXTENSIONS
	if (ext_inused)
#endif
	{
		/* external client offline; tell OTG about cached ID/BSV */
		if (mdwc->ext_xceiv.id != mdwc->id_state) {
			mdwc->ext_xceiv.id = mdwc->id_state;
			notify_otg = true;
		}

		mdwc->ext_xceiv.bsv = mdwc->vbus_active;
		notify_otg |= mdwc->vbus_active;
	}
#ifdef CONFIG_SONY_USB_EXTENSIONS
	else {
		dev_dbg(mdwc->dev, "%s: not inused, ignore a disconnection\n",
								__func__);
		return;
	}
#endif

	if (mdwc->ext_vbus_psy)
		power_supply_set_present(mdwc->ext_vbus_psy, on);

#ifdef CONFIG_SONY_USB_EXTENSIONS
	if (!mdwc->id_state && !on && ext_inused) {
		dev_info(mdwc->dev, "%s: There may be unhandled ID GND\n",
								__func__);
		wake_lock_timeout(&mdwc->id_wakelock, USB_ID_WAKE_LOCK_TIMEOUT);
		queue_work(system_nrt_wq, &mdwc->id_work);
		return;
	}

	if (mdwc->id_state && !on && ext_inused && !mdwc->ext_xceiv.bsv) {
		dev_info(mdwc->dev, "%s: MHL was removed\n", __func__);
		/* ground D+- when MHL was removed */
		dwc3_select_mhl_switch(mdwc, MHL_SWITCH_PORT_USB2);
	}
#endif

	if (notify_otg)
		queue_delayed_work(system_nrt_wq, &mdwc->resume_work, 0);
}

static void dwc3_id_work(struct work_struct *w)
{
	struct dwc3_msm *mdwc = container_of(w, struct dwc3_msm, id_work);
	int ret;

	/* Give external client a chance to handle */
	if (!mdwc->ext_inuse && usb_ext) {
		if (mdwc->pmic_id_irq)
			disable_irq(mdwc->pmic_id_irq);
#ifdef CONFIG_SONY_USB_EXTENSIONS
		else
			mdwc->ext_switching = true;

		if (mdwc->ext_xceiv.bsv || mdwc->ext_bsv) {
			if ((mdwc->charger.chg_type == DWC3_DCP_CHARGER) &&
							!mdwc->id_state) {
				dev_info(mdwc->dev, "request APSD rerun\n");
				dwc3_select_mhl_switch(mdwc,
							MHL_SWITCH_PORT_USB2);
			}

			dev_dbg(mdwc->dev,
				"%s: interpret ID as ID_GND, bsv=%d, id=%d\n",
				__func__, mdwc->ext_xceiv.bsv, mdwc->id_state);
			ret = usb_ext->notify(usb_ext->ctxt, DWC3_ID_GROUND,
					dwc3_ext_notify_online, mdwc);
		} else
			ret = usb_ext->notify(usb_ext->ctxt, mdwc->id_state,
					dwc3_ext_notify_online, mdwc);
#else
		ret = usb_ext->notify(usb_ext->ctxt, mdwc->id_state,
				      dwc3_ext_notify_online, mdwc);
#endif
		dev_dbg(mdwc->dev, "%s: external handler returned %d\n",
			__func__, ret);

		if (mdwc->pmic_id_irq) {
			unsigned long flags;
			local_irq_save(flags);
			/* ID may have changed while IRQ disabled; update it */
			mdwc->id_state = !!irq_read_line(mdwc->pmic_id_irq);
			local_irq_restore(flags);
			enable_irq(mdwc->pmic_id_irq);
		}
#ifdef CONFIG_SONY_USB_EXTENSIONS
		else
			mdwc->ext_switching = false;
#endif

		mdwc->ext_inuse = (ret == 0);
	}

	if (mdwc->ext_inuse) {
		/* MHL cable: stop peripheral */
		mdwc->ext_xceiv.id = DWC3_ID_FLOAT;
		mdwc->ext_xceiv.bsv = false;
#ifdef CONFIG_SONY_USB_EXTENSIONS
	} else if (mdwc->ext_xceiv.bsv || mdwc->ext_bsv) {
		if (mdwc->ext_xceiv.id == DWC3_ID_FLOAT) {
			dev_dbg(mdwc->dev, "%s: ignore FLOAT\n", __func__);
			return;
		}
		dev_info(mdwc->dev, "%s: force FLOAT when bsv\n", __func__);
		mdwc->ext_xceiv.id = DWC3_ID_FLOAT;
#endif
	} else {
		/* A cable: start host */
		mdwc->ext_xceiv.id = mdwc->id_state;
#ifdef CONFIG_SONY_USB_EXTENSIONS
		dev_dbg(mdwc->dev, "%s: set id=%d\n", __func__,
						mdwc->ext_xceiv.id);
		if (!mdwc->ext_xceiv.id) {
			/* connect D+- to phy when detect ID_GND */
			dwc3_select_mhl_switch(mdwc, MHL_SWITCH_PORT_USB1);
		} else if (!mdwc->ext_xceiv.bsv) {
			/* ground D+- when detect ID_FLOAT.
			 * but do not ground it if VBUS is high because the
			 * charger detection may be using it.
			 * D+- will be grounded when VBUS is deasserted.
			 */
			dwc3_select_mhl_switch(mdwc, MHL_SWITCH_PORT_USB2);
		}
#endif
	}

	dbg_event(0xFF, "RW (id)", 0);
	/* notify OTG */
	dwc3_resume_work(&mdwc->resume_work.work);
}

static irqreturn_t dwc3_pmic_id_irq(int irq, void *data)
{
	struct dwc3_msm *mdwc = data;
	enum dwc3_id_state id;

	/* If we can't read ID line state for some reason, treat it as float */
	id = !!irq_read_line(irq);
	if (mdwc->id_state != id) {
		mdwc->id_state = id;
#ifdef CONFIG_SONY_USB_EXTENSIONS
		wake_lock_timeout(&mdwc->id_wakelock, USB_ID_WAKE_LOCK_TIMEOUT);
#endif
		queue_work(system_nrt_wq, &mdwc->id_work);
	}

	return IRQ_HANDLED;
}

static int dwc3_cpu_notifier_cb(struct notifier_block *nfb,
		unsigned long action, void *hcpu)
{
	uint32_t cpu = (uintptr_t)hcpu;
	struct dwc3_msm *mdwc =
			container_of(nfb, struct dwc3_msm, dwc3_cpu_notifier);

	if (cpu == cpu_to_affin && action == CPU_ONLINE) {
		pr_debug("%s: cpu online:%u irq:%d\n", __func__,
				cpu_to_affin, mdwc->irq_to_affin);
		irq_set_affinity(mdwc->irq_to_affin, get_cpu_mask(cpu));
	}

	return NOTIFY_OK;
}

#ifdef CONFIG_SONY_USB_EXTENSIONS
/**
 * dwc3_ocp_work - workqueue function.
 *
 * @w: Pointer to the dwc3 ocp workqueue
 *
 * NOTE: After ocp, resume and notify the event to OTG.
 */
static void dwc3_ocp_work(struct work_struct *w)
{
	struct dwc3_msm *mdwc = container_of(w, struct dwc3_msm, ocp_work);

	pr_info("%s: receive ocp notification\n", __func__);
	if (!mdwc->ext_inuse) { /* notify OTG */
		mdwc->ext_xceiv.ocp = true;
		dwc3_resume_work(&mdwc->resume_work.work);
	}
}

/**
 * dwc3_ocp_notification - ocp notification callback from regulator.
 * @ctxt: Pointer to the dwc3 context
 *
 * NOTE: This runs in interrupt context.
 */
static void dwc3_ocp_notification(void *ctxt)
{
	struct dwc3_msm *mdwc = (struct dwc3_msm *)ctxt;

	queue_work(system_nrt_wq, &mdwc->ocp_work);
}
#endif

static void dwc3_adc_notification(enum qpnp_tm_state state, void *ctx)
{
	struct dwc3_msm *mdwc = ctx;

	if (state >= ADC_TM_STATE_NUM) {
		pr_err("%s: invalid notification %d\n", __func__, state);
		return;
	}

	dev_dbg(mdwc->dev, "%s: state = %s\n", __func__,
			state == ADC_TM_HIGH_STATE ? "high" : "low");

	/* save ID state, but don't necessarily notify OTG */
	if (state == ADC_TM_HIGH_STATE) {
		mdwc->id_state = DWC3_ID_FLOAT;
		mdwc->adc_param.state_request = ADC_TM_LOW_THR_ENABLE;
	} else {
		mdwc->id_state = DWC3_ID_GROUND;
		mdwc->adc_param.state_request = ADC_TM_HIGH_THR_ENABLE;
	}

#ifdef CONFIG_SONY_USB_EXTENSIONS
	wake_lock_timeout(&mdwc->id_wakelock, USB_ID_WAKE_LOCK_TIMEOUT);
#endif
	dwc3_id_work(&mdwc->id_work);

	/* re-arm ADC interrupt */
	qpnp_adc_tm_usbid_configure(mdwc->adc_tm_dev, &mdwc->adc_param);
}

static void dwc3_init_adc_work(struct work_struct *w)
{
	struct dwc3_msm *mdwc = container_of(w, struct dwc3_msm,
							init_adc_work.work);
	int ret;

	mdwc->adc_tm_dev = qpnp_get_adc_tm(mdwc->dev, "dwc_usb3-adc_tm");
	if (IS_ERR(mdwc->adc_tm_dev)) {
		if (PTR_ERR(mdwc->adc_tm_dev) == -EPROBE_DEFER)
			queue_delayed_work(system_nrt_wq, to_delayed_work(w),
					msecs_to_jiffies(100));
		else
			mdwc->adc_tm_dev = NULL;

		return;
	}

	mdwc->adc_param.low_thr = adc_low_threshold;
	mdwc->adc_param.high_thr = adc_high_threshold;
	mdwc->adc_param.timer_interval = adc_meas_interval;
	mdwc->adc_param.state_request = ADC_TM_HIGH_LOW_THR_ENABLE;
	mdwc->adc_param.btm_ctx = mdwc;
	mdwc->adc_param.threshold_notification = dwc3_adc_notification;

	ret = qpnp_adc_tm_usbid_configure(mdwc->adc_tm_dev, &mdwc->adc_param);
	if (ret) {
		dev_err(mdwc->dev, "%s: request ADC error %d\n", __func__, ret);
		return;
	}

	mdwc->id_adc_detect = true;
}

static ssize_t adc_enable_show(struct device *dev,
			       struct device_attribute *attr, char *buf)
{
	struct dwc3_msm *mdwc = dev_get_drvdata(dev);

	if (!mdwc)
		return -EINVAL;

	return snprintf(buf, PAGE_SIZE, "%s\n", mdwc->id_adc_detect ?
						"enabled" : "disabled");
}

static ssize_t adc_enable_store(struct device *dev,
		struct device_attribute *attr, const char
		*buf, size_t size)
{
	struct dwc3_msm *mdwc = dev_get_drvdata(dev);

	if (!mdwc)
		return -EINVAL;

	if (!strnicmp(buf, "enable", 6)) {
		if (!mdwc->id_adc_detect)
			dwc3_init_adc_work(&mdwc->init_adc_work.work);
		return size;
	} else if (!strnicmp(buf, "disable", 7)) {
		qpnp_adc_tm_usbid_end(mdwc->adc_tm_dev);
		mdwc->id_adc_detect = false;
		return size;
	}

	return -EINVAL;
}

static DEVICE_ATTR(adc_enable, S_IRUGO | S_IWUSR, adc_enable_show,
		adc_enable_store);

static int dwc3_msm_ext_chg_open(struct inode *inode, struct file *file)
{
	struct dwc3_msm *mdwc =
		container_of(inode->i_cdev, struct dwc3_msm, ext_chg_cdev);

	pr_debug("dwc3-msm ext chg open\n");
	file->private_data = mdwc;
	mdwc->ext_chg_opened = true;

	return 0;
}

static long
dwc3_msm_ext_chg_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct dwc3_msm *mdwc = file->private_data;
	struct msm_usb_chg_info info = {0};
	int ret = 0, val;

	switch (cmd) {
	case MSM_USB_EXT_CHG_INFO:
		info.chg_block_type = USB_CHG_BLOCK_QSCRATCH;
		info.page_offset = (mdwc->io_res->start +
				QSCRATCH_REG_OFFSET) & ~PAGE_MASK;
		/*
		 * The charger block register address space is only
		 * 512 bytes.  But mmap() works on PAGE granularity.
		 */
		info.length = PAGE_SIZE;

		if (copy_to_user((void __user *)arg, &info, sizeof(info))) {
			pr_err("%s: copy to user failed\n\n", __func__);
			ret = -EFAULT;
		}
		break;
	case MSM_USB_EXT_CHG_BLOCK_LPM:
		if (get_user(val, (int __user *)arg)) {
			pr_err("%s: get_user failed\n\n", __func__);
			ret = -EFAULT;
			break;
		}
		pr_debug("%s: LPM block request %d\n", __func__, val);
		if (val) { /* block LPM */
			if (mdwc->charger.chg_type == DWC3_DCP_CHARGER) {
				dbg_event(0xFF, "CHGioct gsyn", 0);
				pm_runtime_get_sync(mdwc->dev);
			} else {
				mdwc->ext_chg_active = false;
				complete(&mdwc->ext_chg_wait);
				ret = -ENODEV;
			}
		} else {
			mdwc->ext_chg_active = false;
			complete(&mdwc->ext_chg_wait);
			dbg_event(0xFF, "CHGioct put", 0);
			pm_runtime_put(mdwc->dev);
		}
		break;
	case MSM_USB_EXT_CHG_VOLTAGE_INFO:
		if (get_user(val, (int __user *)arg)) {
			pr_err("%s: get_user failed\n\n", __func__);
			ret = -EFAULT;
			break;
		}

		if (val == USB_REQUEST_5V)
			pr_debug("%s:voting 5V voltage request\n", __func__);
		else if (val == USB_REQUEST_9V)
			pr_debug("%s:voting 9V voltage request\n", __func__);
		break;
	case MSM_USB_EXT_CHG_RESULT:
		if (get_user(val, (int __user *)arg)) {
			pr_err("%s: get_user failed\n\n", __func__);
			ret = -EFAULT;
			break;
		}

		if (!val)
			pr_debug("%s:voltage request successful\n", __func__);
		else
			pr_debug("%s:voltage request failed\n", __func__);
		break;
	case MSM_USB_EXT_CHG_TYPE:
		if (get_user(val, (int __user *)arg)) {
			pr_err("%s: get_user failed\n\n", __func__);
			ret = -EFAULT;
			break;
		}

		if (val)
			pr_debug("%s:charger is external charger\n", __func__);
		else
			pr_debug("%s:charger is not ext charger\n", __func__);
		break;
	default:
		ret = -EINVAL;
	}

	return ret;
}

static int dwc3_msm_ext_chg_mmap(struct file *file, struct vm_area_struct *vma)
{
	struct dwc3_msm *mdwc = file->private_data;
	unsigned long vsize = vma->vm_end - vma->vm_start;
	int ret;

	if (vma->vm_pgoff != 0 || vsize > PAGE_SIZE)
		return -EINVAL;

	vma->vm_pgoff = __phys_to_pfn(mdwc->io_res->start +
				QSCRATCH_REG_OFFSET);
	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

	ret = io_remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff,
				 vsize, vma->vm_page_prot);
	if (ret < 0)
		pr_err("%s: failed with return val %d\n", __func__, ret);

	return ret;
}

static int dwc3_msm_ext_chg_release(struct inode *inode, struct file *file)
{
	struct dwc3_msm *mdwc = file->private_data;

	pr_debug("dwc3-msm ext chg release\n");

	mdwc->ext_chg_opened = false;

	return 0;
}

static const struct file_operations dwc3_msm_ext_chg_fops = {
	.owner = THIS_MODULE,
	.open = dwc3_msm_ext_chg_open,
	.unlocked_ioctl = dwc3_msm_ext_chg_ioctl,
	.mmap = dwc3_msm_ext_chg_mmap,
	.release = dwc3_msm_ext_chg_release,
};

static int dwc3_msm_setup_cdev(struct dwc3_msm *mdwc)
{
	int ret;

	ret = alloc_chrdev_region(&mdwc->ext_chg_dev, 0, 1, "usb_ext_chg");
	if (ret < 0) {
		pr_err("Fail to allocate usb ext char dev region\n");
		return ret;
	}
	mdwc->ext_chg_class = class_create(THIS_MODULE, "dwc_ext_chg");
	if (ret < 0) {
		pr_err("Fail to create usb ext chg class\n");
		goto unreg_chrdev;
	}
	cdev_init(&mdwc->ext_chg_cdev, &dwc3_msm_ext_chg_fops);
	mdwc->ext_chg_cdev.owner = THIS_MODULE;

	ret = cdev_add(&mdwc->ext_chg_cdev, mdwc->ext_chg_dev, 1);
	if (ret < 0) {
		pr_err("Fail to add usb ext chg cdev\n");
		goto destroy_class;
	}
	mdwc->ext_chg_device = device_create(mdwc->ext_chg_class,
					NULL, mdwc->ext_chg_dev, NULL,
					"usb_ext_chg");
	if (IS_ERR(mdwc->ext_chg_device)) {
		pr_err("Fail to create usb ext chg device\n");
		ret = PTR_ERR(mdwc->ext_chg_device);
		mdwc->ext_chg_device = NULL;
		goto del_cdev;
	}

	pr_debug("dwc3 msm ext chg cdev setup success\n");
	return 0;

del_cdev:
	cdev_del(&mdwc->ext_chg_cdev);
destroy_class:
	class_destroy(mdwc->ext_chg_class);
unreg_chrdev:
	unregister_chrdev_region(mdwc->ext_chg_dev, 1);

	return ret;
}

static int dwc3_msm_probe(struct platform_device *pdev)
{
	struct device_node *node = pdev->dev.of_node, *dwc3_node;
	struct device	*dev = &pdev->dev;
	struct dwc3_msm *mdwc;
	struct dwc3	*dwc;
	struct resource *res;
	void __iomem *tcsr;
	unsigned long flags;
	u32 tmp;
	bool host_mode;
	int ret = 0;

	mdwc = devm_kzalloc(&pdev->dev, sizeof(*mdwc), GFP_KERNEL);
	if (!mdwc) {
		dev_err(&pdev->dev, "not enough memory\n");
		return -ENOMEM;
	}

	if (!dev->dma_mask)
		dev->dma_mask = &dev->coherent_dma_mask;
	if (!dev->coherent_dma_mask)
		dev->coherent_dma_mask = DMA_BIT_MASK(64);

	platform_set_drvdata(pdev, mdwc);
	mdwc->dev = &pdev->dev;

	INIT_LIST_HEAD(&mdwc->req_complete_list);
	INIT_DELAYED_WORK(&mdwc->chg_work, dwc3_chg_detect_work);
	INIT_DELAYED_WORK(&mdwc->resume_work, dwc3_resume_work);
	INIT_WORK(&mdwc->restart_usb_work, dwc3_restart_usb_work);
	INIT_WORK(&mdwc->id_work, dwc3_id_work);
	INIT_WORK(&mdwc->bus_vote_w, dwc3_msm_bus_vote_w);
	INIT_DELAYED_WORK(&mdwc->init_adc_work, dwc3_init_adc_work);
	init_completion(&mdwc->ext_chg_wait);

#ifdef CONFIG_SONY_USB_EXTENSIONS
	INIT_WORK(&mdwc->ocp_work, dwc3_ocp_work);
	wake_lock_init(&mdwc->id_wakelock, WAKE_LOCK_SUSPEND, "id_wakelock");
#endif

	/*
	 * This regulator needs to be turned on to remove pull down on Dp and
	 * Dm lines to detect charger type properly.
	 */
	mdwc->vdda33 = devm_regulator_get(dev, "vdda33");
	if (IS_ERR(mdwc->vdda33)) {
		dev_err(&pdev->dev, "unable to get vdda33 supply\n");
		return PTR_ERR(mdwc->vdda33);
	}

	mdwc->vdda18 = devm_regulator_get(dev, "vdda18");
	if (IS_ERR(mdwc->vdda18)) {
		dev_err(&pdev->dev, "unable to get vdda18 supply\n");
		return PTR_ERR(mdwc->vdda18);
	}

	ret = dwc3_msm_config_gdsc(mdwc, 1);
	if (ret) {
		dev_err(&pdev->dev, "unable to configure usb3 gdsc\n");
		return ret;
	}

	mdwc->xo_clk = clk_get(&pdev->dev, "xo");
	if (IS_ERR(mdwc->xo_clk)) {
		dev_err(&pdev->dev, "%s unable to get TCXO buffer handle\n",
								__func__);
		ret = PTR_ERR(mdwc->xo_clk);
		goto disable_dwc3_gdsc;
	}

	clk_set_rate(mdwc->xo_clk, 19200000);
	ret = clk_prepare_enable(mdwc->xo_clk);
	if (ret) {
		dev_err(&pdev->dev, "%s failed to vote for TCXO buffer%d\n",
						__func__, ret);
		goto put_xo;
	}

	/*
	 * DWC3 Core requires its CORE CLK (aka master / bus clk) to
	 * run at 125Mhz in SSUSB mode and >60MHZ for HSUSB mode.
	 */
	mdwc->core_clk = devm_clk_get(&pdev->dev, "core_clk");
	if (IS_ERR(mdwc->core_clk)) {
		dev_err(&pdev->dev, "failed to get core_clk\n");
		ret = PTR_ERR(mdwc->core_clk);
		goto disable_xo;
	}
	clk_set_rate(mdwc->core_clk, 125000000);
	clk_prepare_enable(mdwc->core_clk);

	mdwc->iface_clk = devm_clk_get(&pdev->dev, "iface_clk");
	if (IS_ERR(mdwc->iface_clk)) {
		dev_err(&pdev->dev, "failed to get iface_clk\n");
		ret = PTR_ERR(mdwc->iface_clk);
		goto disable_core_clk;
	}
	clk_set_rate(mdwc->iface_clk, 125000000);
	clk_prepare_enable(mdwc->iface_clk);

	mdwc->sleep_clk = devm_clk_get(&pdev->dev, "sleep_clk");
	if (IS_ERR(mdwc->sleep_clk)) {
		dev_err(&pdev->dev, "failed to get sleep_clk\n");
		ret = PTR_ERR(mdwc->sleep_clk);
		goto disable_iface_clk;
	}
	clk_set_rate(mdwc->sleep_clk, 32000);
	clk_prepare_enable(mdwc->sleep_clk);

	ret = of_property_read_u32(node, "qcom,utmi-clk-rate",
				   (u32 *)&mdwc->utmi_clk_rate);
	if (ret)
		mdwc->utmi_clk_rate = 19200000;

	mdwc->utmi_clk = devm_clk_get(&pdev->dev, "utmi_clk");
	if (IS_ERR(mdwc->utmi_clk)) {
		dev_err(&pdev->dev, "failed to get utmi_clk\n");
		ret = PTR_ERR(mdwc->utmi_clk);
		goto disable_sleep_clk;
	}

	if (mdwc->utmi_clk_rate == 24000000) {
		/*
		 * For setting utmi clock to 24MHz, first set 48MHz on parent
		 * clock "utmi_clk_src" and then set divider 2 on child branch
		 * "utmi_clk".
		 */
		mdwc->utmi_clk_src = devm_clk_get(&pdev->dev, "utmi_clk_src");
		if (IS_ERR(mdwc->utmi_clk_src)) {
			dev_err(&pdev->dev, "failed to get utmi_clk_src\n");
			ret = PTR_ERR(mdwc->utmi_clk_src);
			goto disable_sleep_clk;
		}
		clk_set_rate(mdwc->utmi_clk_src, 48000000);
		/* 1 means divide utmi_clk_src by 2 */
		clk_set_rate(mdwc->utmi_clk, 1);
	} else {
		clk_set_rate(mdwc->utmi_clk, mdwc->utmi_clk_rate);
	}
	clk_prepare_enable(mdwc->utmi_clk);

	mdwc->ref_clk = devm_clk_get(&pdev->dev, "ref_clk");
	if (IS_ERR(mdwc->ref_clk)) {
		dev_err(&pdev->dev, "failed to get ref_clk\n");
		ret = PTR_ERR(mdwc->ref_clk);
		goto disable_utmi_clk;
	}
	clk_set_rate(mdwc->ref_clk, 19200000);
	clk_prepare_enable(mdwc->ref_clk);

	mdwc->id_state = mdwc->ext_xceiv.id = DWC3_ID_FLOAT;
	mdwc->charger.charging_disabled = of_property_read_bool(node,
				"qcom,charging-disabled");

	mdwc->charger.skip_chg_detect = of_property_read_bool(node,
				"qcom,skip-charger-detection");

	mdwc->suspend_resume_no_support = of_property_read_bool(node,
				"qcom,no-suspend-resume");

	mdwc->power_collapse_por = of_property_read_bool(node,
		"qcom,por-after-power-collapse");

	mdwc->power_collapse = of_property_read_bool(node,
		"qcom,power-collapse-on-cable-disconnect");

	dev_dbg(&pdev->dev, "power collapse=%d, POR=%d\n",
		mdwc->power_collapse, mdwc->power_collapse_por);

	ret = of_property_read_u32(node, "qcom,lpm-to-suspend-delay-ms",
				&mdwc->lpm_to_suspend_delay);
	if (ret) {
		dev_dbg(&pdev->dev, "setting lpm_to_suspend_delay to zero.\n");
		mdwc->lpm_to_suspend_delay = 0;
	}

	if (mdwc->power_collapse) {
		/*
		 * If present, the phy_com_reset is used in conjunction
		 * with power collapse exit
		 */
		mdwc->phy_com_reset = devm_clk_get(&pdev->dev, "phy_com_reset");
		if (IS_ERR(mdwc->phy_com_reset)) {
			dev_dbg(&pdev->dev,
				"%s unable to get phy_com_reset clock, ret=%ld\n",
				__func__, PTR_ERR(mdwc->phy_com_reset));
			mdwc->phy_com_reset = NULL;
		}
	}

	/*
	 * DWC3 has separate IRQ line for OTG events (ID/BSV) and for
	 * DP and DM linestate transitions during low power mode.
	 */
	mdwc->hs_phy_irq = platform_get_irq_byname(pdev, "hs_phy_irq");
	if (mdwc->hs_phy_irq < 0) {
		dev_err(&pdev->dev, "pget_irq for hs_phy_irq failed\n");
		goto disable_ref_clk;
	} else {
		irq_set_status_flags(mdwc->hs_phy_irq, IRQ_NOAUTOEN);
		ret = devm_request_irq(&pdev->dev, mdwc->hs_phy_irq,
				msm_dwc3_hs_phy_irq, IRQF_TRIGGER_RISING,
			       "msm_hs_phy_irq", mdwc);
		if (ret) {
			dev_err(&pdev->dev, "irqreq HSPHYINT failed\n");
			goto disable_ref_clk;
		}
	}

	/*
	 * Some platforms have a special interrupt line for indicating resume
	 * while in low power mode, when clocks are disabled.
	 */
	mdwc->pwr_event_irq = platform_get_irq_byname(pdev, "pwr_event_irq");
	if (mdwc->pwr_event_irq < 0) {
		dev_err(&pdev->dev, "pget_irq for pwr_event_irq failed\n");
		//goto disable_ref_clk;
		mdwc->pwr_event_irq = 0;
	} else {
		/*
		 * enable pwr event irq early during PM resume to meet bus
		 * resume timeline from usb device
		 */
		ret = devm_request_threaded_irq(&pdev->dev, mdwc->pwr_event_irq,
					msm_dwc3_pwr_irq,
					msm_dwc3_pwr_irq_thread,
					IRQF_TRIGGER_RISING | IRQF_EARLY_RESUME,
					"msm_dwc3", mdwc);
		if (ret) {
			dev_err(&pdev->dev, "irqreq pwr_event_irq failed: %d\n",
					ret);
			goto disable_ref_clk;
		}
	}

	mdwc->pmic_id_irq = platform_get_irq_byname(pdev, "pmic_id_irq");
	if (mdwc->pmic_id_irq > 0) {
		/* check if PMIC MISC module monitors usb ID */
		if (of_get_property(pdev->dev.of_node, "qcom,misc-ref", NULL))
			/* check if PMIC ID IRQ is supported */
			ret = qpnp_misc_irqs_available(&pdev->dev);
		else
			ret = mdwc->pmic_id_irq;

		if (ret == -EPROBE_DEFER) {
			/* qpnp hasn't probed yet; defer dwc probe */
			goto disable_ref_clk;
		} else if (ret == 0) {
			mdwc->pmic_id_irq = 0;
		} else {
			irq_set_status_flags(mdwc->pmic_id_irq,
					IRQ_NOAUTOEN);
			ret = devm_request_irq(&pdev->dev,
					       mdwc->pmic_id_irq,
					       dwc3_pmic_id_irq,
					       IRQF_TRIGGER_RISING |
					       IRQF_TRIGGER_FALLING,
					       "dwc3_msm_pmic_id",
					       mdwc);
			if (ret) {
				dev_err(&pdev->dev, "irqreq IDINT failed\n");
				goto disable_ref_clk;
			}
		}
	}

	if (mdwc->pmic_id_irq <= 0) {
		/* If no PMIC ID IRQ, use ADC for ID pin detection */
		queue_work(system_nrt_wq, &mdwc->init_adc_work.work);
		device_create_file(&pdev->dev, &dev_attr_adc_enable);
		mdwc->pmic_id_irq = 0;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	if (!res) {
		dev_dbg(&pdev->dev, "missing TCSR memory resource\n");
	} else {
		tcsr = devm_ioremap_nocache(&pdev->dev, res->start,
			resource_size(res));
		if (!tcsr) {
			dev_dbg(&pdev->dev, "tcsr ioremap failed\n");
		} else {
			/* Enable USB3 on the primary USB port. */
			writel_relaxed(0x1, tcsr);
			/*
			 * Ensure that TCSR write is completed before
			 * USB registers initialization.
			 */
			mb();
		}
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(&pdev->dev, "missing memory base resource\n");
		ret = -ENODEV;
		goto disable_ref_clk;
	}

	mdwc->base = devm_ioremap_nocache(&pdev->dev, res->start,
		resource_size(res));
	if (!mdwc->base) {
		dev_err(&pdev->dev, "ioremap failed\n");
		ret = -ENODEV;
		goto disable_ref_clk;
	}

	mdwc->io_res = res; /* used to calculate chg block offset */

	if (of_get_property(pdev->dev.of_node, "qcom,usb-dbm", NULL)) {
		mdwc->dbm = usb_get_dbm_by_phandle(&pdev->dev, "qcom,usb-dbm",
						   0);
		if (IS_ERR(mdwc->dbm)) {
			dev_err(&pdev->dev, "unable to get dbm device\n");
			ret = -EPROBE_DEFER;
			goto disable_ref_clk;
		}
		/*
		 * Add power event if the dbm indicates coming out of L1
		 * by interrupt
		 */
		if (dbm_l1_lpm_interrupt(mdwc->dbm)) {
			if (!mdwc->pwr_event_irq) {
				dev_err(&pdev->dev,
					"need pwr_event_irq exiting L1\n");
				ret = -EINVAL;
				goto disable_ref_clk;
			}
		}
	}

	ret = of_property_read_u32(node, "qcom,restore-sec-cfg-for-scm-dev-id",
					&mdwc->scm_dev_id);
	if (ret)
		dev_dbg(&pdev->dev, "unable to read scm device id (%d)\n", ret);

	if (of_property_read_u32(node, "qcom,dwc-usb3-msm-tx-fifo-size",
				 &mdwc->tx_fifo_size))
		dev_err(&pdev->dev,
			"unable to read platform data tx fifo size\n");

	if (of_property_read_u32(node, "qcom,dwc-usb3-msm-qdss-tx-fifo-size",
				 &mdwc->qdss_tx_fifo_size))
		dev_err(&pdev->dev,
			"unable to read platform data qdss tx fifo size\n");

#ifdef CONFIG_SONY_USB_EXTENSIONS
	/* mhl switch to usb2 */
	mdwc->mhl_switch_sel1_gpio = of_get_named_gpio(node,
						"mhl_switch_sel1_gpio", 0);
	mdwc->mhl_switch_sel2_gpio = of_get_named_gpio(node,
						"mhl_switch_sel2_gpio", 0);
	if (!gpio_is_valid(mdwc->mhl_switch_sel1_gpio) ||
				!gpio_is_valid(mdwc->mhl_switch_sel2_gpio))
		dev_info(&pdev->dev, "MHL switch is not enabled\n");

	mdwc->usb_shell_sel_gpio = of_get_named_gpio(node,
						"usb_shell_sel_gpio", 0);
	if (!gpio_is_valid(mdwc->usb_shell_sel_gpio))
		dev_dbg(&pdev->dev, "usb_shell_sel is not supported\n");

	wakeup_source_init(&mdwc->wakeup_source_vbus, "wakeup_source_vbus");
#endif

	dwc3_set_notifier(&dwc3_msm_notify_event);

	/* Assumes dwc3 is the only DT child of dwc3-msm */
	dwc3_node = of_get_next_available_child(node, NULL);
	if (!dwc3_node) {
		dev_err(&pdev->dev, "failed to find dwc3 child\n");
		goto put_psupply;
	}

	host_mode = of_property_read_bool(dwc3_node, "snps,host-only-mode");
	if (host_mode && of_get_property(pdev->dev.of_node, "vbus_dwc3-supply",
									NULL)) {
		mdwc->vbus_otg = devm_regulator_get(&pdev->dev, "vbus_dwc3");
		if (IS_ERR(mdwc->vbus_otg)) {
			dev_err(&pdev->dev, "Failed to get vbus regulator\n");
			ret = PTR_ERR(mdwc->vbus_otg);
			of_node_put(dwc3_node);
			goto put_psupply;
		}
		ret = regulator_enable(mdwc->vbus_otg);
		if (ret) {
			mdwc->vbus_otg = 0;
			dev_err(&pdev->dev, "Failed to enable vbus_otg\n");
			of_node_put(dwc3_node);
			goto put_psupply;
		}
	}

	/* usb_psy required only for vbus_notifications */
	if (!host_mode) {
		mdwc->usb_psy.name = "usb";
		mdwc->usb_psy.type = POWER_SUPPLY_TYPE_USB;
		mdwc->usb_psy.supplied_to = dwc3_msm_pm_power_supplied_to;
		mdwc->usb_psy.num_supplicants = ARRAY_SIZE(
						dwc3_msm_pm_power_supplied_to);
		mdwc->usb_psy.properties = dwc3_msm_pm_power_props_usb;
		mdwc->usb_psy.num_properties =
					ARRAY_SIZE(dwc3_msm_pm_power_props_usb);
		mdwc->usb_psy.get_property = dwc3_msm_power_get_property_usb;
		mdwc->usb_psy.set_property = dwc3_msm_power_set_property_usb;
		mdwc->usb_psy.external_power_changed =
					dwc3_msm_external_power_changed;
		mdwc->usb_psy.property_is_writeable =
				dwc3_msm_property_is_writeable;

		ret = power_supply_register(&pdev->dev, &mdwc->usb_psy);
		if (ret < 0) {
			dev_err(&pdev->dev,
					"%s:power_supply_register usb failed\n",
						__func__);
			goto disable_ref_clk;
		}
	}

	/* Perform controller GCC reset */
	dwc3_msm_link_clk_reset(mdwc, 1);
	msleep(20);
	dwc3_msm_link_clk_reset(mdwc, 0);

	ret = of_platform_populate(node, NULL, NULL, &pdev->dev);
	if (ret) {
		dev_err(&pdev->dev,
			"failed to add create dwc3 core\n");
		of_node_put(dwc3_node);
		goto disable_vbus;
	}

	mdwc->dwc3 = of_find_device_by_node(dwc3_node);
	of_node_put(dwc3_node);
	if (!mdwc->dwc3) {
		dev_err(&pdev->dev, "failed to get dwc3 platform device\n");
		goto put_dwc3;
	}

	mdwc->hs_phy = devm_usb_get_phy_by_phandle(&mdwc->dwc3->dev,
							"usb-phy", 0);
	if (IS_ERR(mdwc->hs_phy)) {
		dev_err(&pdev->dev, "unable to get hsphy device\n");
		ret = PTR_ERR(mdwc->hs_phy);
		goto put_dwc3;
	}

	mdwc->ss_phy = devm_usb_get_phy_by_phandle(&mdwc->dwc3->dev,
							"usb-phy", 1);
	if (IS_ERR(mdwc->ss_phy)) {
		dev_err(&pdev->dev, "unable to get ssphy device\n");
		ret = PTR_ERR(mdwc->ss_phy);
		goto put_dwc3;
	}

	mdwc->bus_scale_table = msm_bus_cl_get_pdata(pdev);
	if (!mdwc->bus_scale_table) {
		dev_err(&pdev->dev, "bus scaling is disabled\n");
	} else {
		mdwc->bus_perf_client =
			msm_bus_scale_register_client(mdwc->bus_scale_table);
		ret = msm_bus_scale_client_update_request(
						mdwc->bus_perf_client, 1);
		if (ret)
			dev_err(&pdev->dev, "Failed to vote for bus scaling\n");
	}

	dwc = platform_get_drvdata(mdwc->dwc3);
	if (!dwc) {
		dev_err(&pdev->dev, "Failed to get dwc3 device\n");
		goto put_dwc3;
	}

	dwc->vbus_active = of_property_read_bool(node, "qcom,vbus-present");

	if (dwc->dotg)
		mdwc->otg_xceiv = dwc->dotg->otg.phy;
	/* Register with OTG if present */
	if (mdwc->otg_xceiv) {
		/* Skip charger detection for simulator targets */
		if (!mdwc->charger.skip_chg_detect) {
			mdwc->charger.start_detection = dwc3_start_chg_det;
			ret = dwc3_set_charger(mdwc->otg_xceiv->otg,
					&mdwc->charger);
			if (ret || !mdwc->charger.notify_detection_complete) {
				dev_err(&pdev->dev,
					"failed to register charger: %d\n",
					ret);
				goto put_dwc3;
			}
		}

		mdwc->ext_xceiv.ext_block_reset = dwc3_msm_block_reset;
#ifdef CONFIG_SONY_USB_EXTENSIONS
		mdwc->ext_xceiv.ext_ocp_notification.notify =
						dwc3_ocp_notification;
		mdwc->ext_xceiv.ext_ocp_notification.ctxt = mdwc;
#endif
		ret = dwc3_set_ext_xceiv(mdwc->otg_xceiv->otg,
						&mdwc->ext_xceiv);
		if (ret || !mdwc->ext_xceiv.notify_ext_events) {
			dev_err(&pdev->dev, "failed to register xceiver: %d\n",
									ret);
			goto put_dwc3;
		}
	} else if (host_mode) {
		dev_dbg(&pdev->dev, "No OTG, DWC3 running in host only mode\n");
		mdwc->scope = POWER_SUPPLY_SCOPE_SYSTEM;
		mdwc->hs_phy->flags |= PHY_HOST_MODE;
		mdwc->ss_phy->flags |= PHY_HOST_MODE;
	} else {
		dev_err(&pdev->dev, "DWC3 device-only mode not supported\n");
		ret = -ENODEV;
		goto put_dwc3;
	}

	if (mdwc->charger.start_detection) {
		ret = dwc3_msm_setup_cdev(mdwc);
		if (ret)
			dev_err(&pdev->dev, "Fail to setup dwc3 setup cdev\n");
	}

	mdwc->irq_to_affin = platform_get_irq(mdwc->dwc3, 0);
	mdwc->dwc3_cpu_notifier.notifier_call = dwc3_cpu_notifier_cb;

	if (cpu_to_affin)
		register_cpu_notifier(&mdwc->dwc3_cpu_notifier);

	device_init_wakeup(mdwc->dev, 1);
	pm_stay_awake(mdwc->dev);

	pm_runtime_set_active(mdwc->dev);
	pm_runtime_enable(mdwc->dev);

	/* Get initial P3 status and enable IN_P3 event */
	tmp = dwc3_msm_read_reg_field(mdwc->base,
			DWC3_GDBGLTSSM, DWC3_GDBGLTSSM_LINKSTATE_MASK);
	atomic_set(&mdwc->in_p3, tmp == DWC3_LINK_STATE_U3);
	dwc3_msm_write_reg_field(mdwc->base, PWR_EVNT_IRQ_MASK_REG,
				PWR_EVNT_POWERDOWN_IN_P3_MASK, 1);

	enable_irq(mdwc->hs_phy_irq);

	/* Update initial ID state */
	if (mdwc->pmic_id_irq) {
		enable_irq(mdwc->pmic_id_irq);
		local_irq_save(flags);
		mdwc->id_state = !!irq_read_line(mdwc->pmic_id_irq);
		if (mdwc->id_state == DWC3_ID_GROUND) {
#ifdef CONFIG_SONY_USB_EXTENSIONS
			wake_lock_timeout(&mdwc->id_wakelock,
				USB_ID_WAKE_LOCK_TIMEOUT);
#endif
			queue_work(system_nrt_wq, &mdwc->id_work);
		}
		local_irq_restore(flags);
		enable_irq_wake(mdwc->pmic_id_irq);
	}

	msm_bam_set_usb_dev(mdwc->dev);

	return 0;

put_dwc3:
	platform_device_put(mdwc->dwc3);
	if (mdwc->bus_perf_client)
		msm_bus_scale_unregister_client(mdwc->bus_perf_client);
disable_vbus:
	if (!IS_ERR_OR_NULL(mdwc->vbus_otg))
		regulator_disable(mdwc->vbus_otg);
put_psupply:
#ifdef CONFIG_SONY_USB_EXTENSIONS
	wakeup_source_trash(&mdwc->wakeup_source_vbus);
#endif
	if (mdwc->usb_psy.dev)
		power_supply_unregister(&mdwc->usb_psy);
disable_ref_clk:
	clk_disable_unprepare(mdwc->ref_clk);
disable_utmi_clk:
	clk_disable_unprepare(mdwc->utmi_clk);
disable_sleep_clk:
	clk_disable_unprepare(mdwc->sleep_clk);
disable_iface_clk:
	clk_disable_unprepare(mdwc->iface_clk);
disable_core_clk:
	clk_set_rate(mdwc->core_clk, 19200000);
	clk_disable_unprepare(mdwc->core_clk);
disable_xo:
	clk_disable_unprepare(mdwc->xo_clk);
put_xo:
	clk_put(mdwc->xo_clk);
disable_dwc3_gdsc:
	dwc3_msm_config_gdsc(mdwc, 0);

	return ret;
}

static int dwc3_msm_remove_children(struct device *dev, void *data)
{
	device_unregister(dev);
	return 0;
}

static int dwc3_msm_remove(struct platform_device *pdev)
{
	struct dwc3_msm	*mdwc = platform_get_drvdata(pdev);
	int ret_pm;

	if (cpu_to_affin)
		unregister_cpu_notifier(&mdwc->dwc3_cpu_notifier);

	/*
	 * In case of system suspend, pm_runtime_get_sync fails.
	 * Hence turn ON the clocks manually.
	 */
	ret_pm = pm_runtime_get_sync(mdwc->dev);
	dbg_event(0xFF, "Remov gsyn", ret_pm);
	if (ret_pm < 0) {
		dev_err(mdwc->dev,
			"pm_runtime_get_sync failed with %d\n", ret_pm);
		clk_prepare_enable(mdwc->utmi_clk);
		clk_prepare_enable(mdwc->core_clk);
		clk_prepare_enable(mdwc->iface_clk);
		clk_prepare_enable(mdwc->sleep_clk);
		clk_prepare_enable(mdwc->ref_clk);
		clk_prepare_enable(mdwc->xo_clk);
	}

	if (mdwc->ext_chg_device) {
		device_destroy(mdwc->ext_chg_class, mdwc->ext_chg_dev);
		cdev_del(&mdwc->ext_chg_cdev);
		class_destroy(mdwc->ext_chg_class);
		unregister_chrdev_region(mdwc->ext_chg_dev, 1);
	}

	if (mdwc->id_adc_detect)
		qpnp_adc_tm_usbid_end(mdwc->adc_tm_dev);
	if (mdwc->otg_xceiv)
		dwc3_start_chg_det(&mdwc->charger, false);
	if (mdwc->usb_psy.dev)
		power_supply_unregister(&mdwc->usb_psy);
	if (mdwc->hs_phy)
		mdwc->hs_phy->flags &= ~PHY_HOST_MODE;
	platform_device_put(mdwc->dwc3);
	device_for_each_child(&pdev->dev, NULL, dwc3_msm_remove_children);

	dbg_event(0xFF, "Remov put", 0);
	pm_runtime_disable(mdwc->dev);
	pm_runtime_barrier(mdwc->dev);
	pm_runtime_put_sync(mdwc->dev);
	pm_runtime_set_suspended(mdwc->dev);
	device_wakeup_disable(mdwc->dev);

	if (mdwc->bus_perf_client)
		msm_bus_scale_unregister_client(mdwc->bus_perf_client);

	if (!IS_ERR_OR_NULL(mdwc->vbus_otg))
		regulator_disable(mdwc->vbus_otg);

	if (mdwc->hs_phy_irq)
		disable_irq(mdwc->hs_phy_irq);
	if (mdwc->pwr_event_irq)
		disable_irq(mdwc->pwr_event_irq);

	clk_disable_unprepare(mdwc->utmi_clk);
	clk_set_rate(mdwc->core_clk, 19200000);
	clk_disable_unprepare(mdwc->core_clk);
	clk_disable_unprepare(mdwc->iface_clk);
	clk_disable_unprepare(mdwc->sleep_clk);
	clk_disable_unprepare(mdwc->ref_clk);
	clk_disable_unprepare(mdwc->xo_clk);
	clk_put(mdwc->xo_clk);

#ifdef CONFIG_SONY_USB_EXTENSIONS
	wake_lock_destroy(&mdwc->id_wakelock);
	wakeup_source_trash(&mdwc->wakeup_source_vbus);
#endif

	dwc3_msm_config_gdsc(mdwc, 0);

	return 0;
}

#ifdef CONFIG_SONY_USB_EXTENSIONS
static void dwc3_msm_shutdown(struct platform_device *pdev)
{
	struct dwc3_msm *mdwc = platform_get_drvdata(pdev);

	msm_dwc3_vbus_control(mdwc, false);

	if (!mdwc->ext_xceiv.bsv && !mdwc->ext_bsv) {
		struct dwc3 *dwc = platform_get_drvdata(mdwc->dwc3);
		struct regulator *vbus_rec = devm_regulator_get(
						dwc->dev->parent, "vbus_rec");
		if (IS_ERR_OR_NULL(vbus_rec)) {
			dev_err(mdwc->dev, "failed to get vbus_rec\n");
		} else {
			int rc;
			msleep(20);
			rc = regulator_enable(vbus_rec);
			if (!rc) {
				msleep(20);
				regulator_disable(vbus_rec);
			}
		}
	}

	msleep(200);
}
#endif

#ifdef CONFIG_PM_SLEEP
static int dwc3_msm_pm_suspend(struct device *dev)
{
	int ret = 0;
	struct dwc3_msm *mdwc = dev_get_drvdata(dev);
	struct dwc3 *dwc = platform_get_drvdata(mdwc->dwc3);

	dev_dbg(dev, "dwc3-msm PM suspend\n");
	dbg_event(0xFF, "PM Sus", 0);

	flush_delayed_work(&mdwc->resume_work);
	if (!atomic_read(&dwc->in_lpm)) {
		dev_err(mdwc->dev, "Abort PM suspend!! (USB is outside LPM)\n");
		return -EBUSY;
	}

	ret = dwc3_msm_suspend(mdwc);
	if (!ret)
		atomic_set(&mdwc->pm_suspended, 1);

	return ret;
}

static int dwc3_msm_pm_resume(struct device *dev)
{
	struct dwc3_msm *mdwc = dev_get_drvdata(dev);

	dev_dbg(dev, "dwc3-msm PM resume\n");

	dbg_event(0xFF, "PM Res", 0);

	/* flush to avoid race in read/write of pm_suspended */
	flush_delayed_work(&mdwc->resume_work);
	atomic_set(&mdwc->pm_suspended, 0);

	/* kick in otg state machine */
	queue_delayed_work(system_nrt_wq,
		&mdwc->resume_work, 0);

	return 0;
}
#endif

#ifdef CONFIG_PM_RUNTIME
static int dwc3_msm_runtime_idle(struct device *dev)
{
	struct dwc3_msm *mdwc = dev_get_drvdata(dev);

	dev_dbg(dev, "DWC3-msm runtime idle\n");

	if (mdwc->ext_chg_active) {
		dev_dbg(dev, "Deferring LPM\n");
		/*
		 * Charger detection may happen in user space.
		 * Delay entering LPM by 3 sec.  Otherwise we
		 * have to exit LPM when user space begins
		 * charger detection.
		 *
		 * This timer will be canceled when user space
		 * votes against LPM by incrementing PM usage
		 * counter.  We enter low power mode when
		 * PM usage counter is decremented.
		 */
		pm_schedule_suspend(dev, 3000);
		return -EAGAIN;
	}

	return 0;
}

static int dwc3_msm_runtime_suspend(struct device *dev)
{
	struct dwc3_msm *mdwc = dev_get_drvdata(dev);

	dev_dbg(dev, "DWC3-msm runtime suspend\n");
	dbg_event(0xFF, "RT Sus", 0);

	return dwc3_msm_suspend(mdwc);
}

static int dwc3_msm_runtime_resume(struct device *dev)
{
	struct dwc3_msm *mdwc = dev_get_drvdata(dev);

	dev_dbg(dev, "DWC3-msm runtime resume\n");
	dbg_event(0xFF, "RT Res", 0);

	return dwc3_msm_resume(mdwc);
}
#endif

static const struct dev_pm_ops dwc3_msm_dev_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(dwc3_msm_pm_suspend, dwc3_msm_pm_resume)
	SET_RUNTIME_PM_OPS(dwc3_msm_runtime_suspend, dwc3_msm_runtime_resume,
				dwc3_msm_runtime_idle)
};

static const struct of_device_id of_dwc3_matach[] = {
	{
		.compatible = "qcom,dwc-usb3-msm",
	},
	{ },
};
MODULE_DEVICE_TABLE(of, of_dwc3_matach);

static struct platform_driver dwc3_msm_driver = {
	.probe		= dwc3_msm_probe,
	.remove		= dwc3_msm_remove,
#ifdef CONFIG_SONY_USB_EXTENSIONS
	.shutdown	= dwc3_msm_shutdown,
#endif
	.driver		= {
		.name	= "msm-dwc3",
		.pm	= &dwc3_msm_dev_pm_ops,
		.of_match_table	= of_dwc3_matach,
	},
};

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("DesignWare USB3 MSM Glue Layer");

static int dwc3_msm_init(void)
{
	return platform_driver_register(&dwc3_msm_driver);
}
module_init(dwc3_msm_init);

static void __exit dwc3_msm_exit(void)
{
	platform_driver_unregister(&dwc3_msm_driver);
}
module_exit(dwc3_msm_exit);
