/* Copyright (c) 2014-2016, The Linux Foundation. All rights reserved.
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

/*
 * MSM PCIe controller driver.
 */

#include <linux/module.h>
#include <linux/bitops.h>
#include <linux/clk.h>
#include <linux/debugfs.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/iopoll.h>
#include <linux/kernel.h>
#include <linux/of_pci.h>
#include <linux/pci.h>
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>
#include <linux/regulator/rpm-smd-regulator.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/of_gpio.h>
#include <linux/clk/msm-clk.h>
#include <linux/msm-bus.h>
#include <linux/msm-bus-board.h>
#include <linux/debugfs.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/msi.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/irqdomain.h>
#include <linux/pm_wakeup.h>
#include <linux/compiler.h>
#include <soc/qcom/scm.h>
#include <linux/ipc_logging.h>
#include <linux/msm_pcie.h>

#ifdef CONFIG_ARCH_MDMCALIFORNIUM
#define PCIE_VENDOR_ID_RCP		0x17cb
#define PCIE_DEVICE_ID_RCP		0x0302

#define PCIE20_PARF_DBI_BASE_ADDR	0x350
#define PCIE20_PARF_SLV_ADDR_SPACE_SIZE	0x358

#define PCIE_N_PCS_STATUS(n, m)			(PCS_PORT(n, m) + 0x174)

#define TX_BASE 0x200
#define RX_BASE 0x400
#define PCS_BASE 0x800
#define PCS_MISC_BASE 0x600
#elif defined(CONFIG_ARCH_MDM9640)
#define PCIE_VENDOR_ID_RCP		0x17cb
#define PCIE_DEVICE_ID_RCP		0x0301

#define PCIE20_PARF_DBI_BASE_ADDR	0x168
#define PCIE20_PARF_SLV_ADDR_SPACE_SIZE	0x16C

#define PCIE_N_PCS_STATUS(n, m)			(PCS_PORT(n, m) + 0x128)

#define TX_BASE 0x200
#define RX_BASE 0x400
#define PCS_BASE 0x600
#define PCS_MISC_BASE 0
#else
#define PCIE_VENDOR_ID_RCP		0x17cb
#define PCIE_DEVICE_ID_RCP		0x0104

#define PCIE20_PARF_DBI_BASE_ADDR	0x168
#define PCIE20_PARF_SLV_ADDR_SPACE_SIZE	0x16C

#define PCIE_N_PCS_STATUS(n, m)			(PCS_PORT(n, m) + 0x174)

#define TX_BASE 0x1000
#define RX_BASE 0x1200
#define PCS_BASE 0x1400
#define PCS_MISC_BASE 0
#endif

#define TX(n, m) (TX_BASE + n * m * 0x1000)
#define RX(n, m) (RX_BASE + n * m * 0x1000)
#define PCS_PORT(n, m) (PCS_BASE + n * m * 0x1000)
#define PCS_MISC_PORT(n, m) (PCS_MISC_BASE + n * m * 0x1000)

#define QSERDES_COM_BG_TIMER			0x00C
#define QSERDES_COM_SSC_EN_CENTER		0x010
#define QSERDES_COM_SSC_ADJ_PER1		0x014
#define QSERDES_COM_SSC_ADJ_PER2		0x018
#define QSERDES_COM_SSC_PER1			0x01C
#define QSERDES_COM_SSC_PER2			0x020
#define QSERDES_COM_SSC_STEP_SIZE1		0x024
#define QSERDES_COM_SSC_STEP_SIZE2		0x028
#define QSERDES_COM_BIAS_EN_CLKBUFLR_EN		0x034
#define QSERDES_COM_CLK_ENABLE1			0x038
#define QSERDES_COM_SYS_CLK_CTRL		0x03C
#define QSERDES_COM_SYSCLK_BUF_ENABLE		0x040
#define QSERDES_COM_PLL_IVCO			0x048
#define QSERDES_COM_LOCK_CMP1_MODE0		0x04C
#define QSERDES_COM_LOCK_CMP2_MODE0		0x050
#define QSERDES_COM_LOCK_CMP3_MODE0		0x054
#define QSERDES_COM_BG_TRIM			0x070
#define QSERDES_COM_CLK_EP_DIV			0x074
#define QSERDES_COM_CP_CTRL_MODE0		0x078
#define QSERDES_COM_PLL_RCTRL_MODE0		0x084
#define QSERDES_COM_PLL_CCTRL_MODE0		0x090
#define QSERDES_COM_SYSCLK_EN_SEL		0x0AC
#define QSERDES_COM_RESETSM_CNTRL		0x0B4
#define QSERDES_COM_RESTRIM_CTRL		0x0BC
#define QSERDES_COM_RESCODE_DIV_NUM		0x0C4
#define QSERDES_COM_LOCK_CMP_EN			0x0C8
#define QSERDES_COM_DEC_START_MODE0		0x0D0
#define QSERDES_COM_DIV_FRAC_START1_MODE0	0x0DC
#define QSERDES_COM_DIV_FRAC_START2_MODE0	0x0E0
#define QSERDES_COM_DIV_FRAC_START3_MODE0	0x0E4
#define QSERDES_COM_INTEGLOOP_GAIN0_MODE0	0x108
#define QSERDES_COM_INTEGLOOP_GAIN1_MODE0	0x10C
#define QSERDES_COM_VCO_TUNE_CTRL		0x124
#define QSERDES_COM_VCO_TUNE_MAP		0x128
#define QSERDES_COM_VCO_TUNE1_MODE0		0x12C
#define QSERDES_COM_VCO_TUNE2_MODE0		0x130
#define QSERDES_COM_VCO_TUNE_TIMER1		0x144
#define QSERDES_COM_VCO_TUNE_TIMER2		0x148
#define QSERDES_COM_BG_CTRL			0x170
#define QSERDES_COM_CLK_SELECT			0x174
#define QSERDES_COM_HSCLK_SEL			0x178
#define QSERDES_COM_CORECLK_DIV			0x184
#define QSERDES_COM_CORE_CLK_EN			0x18C
#define QSERDES_COM_C_READY_STATUS		0x190
#define QSERDES_COM_CMN_CONFIG			0x194
#define QSERDES_COM_SVS_MODE_CLK_SEL		0x19C
#define QSERDES_COM_DEBUG_BUS0			0x1A0
#define QSERDES_COM_DEBUG_BUS1			0x1A4
#define QSERDES_COM_DEBUG_BUS2			0x1A8
#define QSERDES_COM_DEBUG_BUS3			0x1AC
#define QSERDES_COM_DEBUG_BUS_SEL		0x1B0

#define QSERDES_TX_N_RES_CODE_LANE_OFFSET(n, m)		(TX(n, m) + 0x4C)
#define QSERDES_TX_N_DEBUG_BUS_SEL(n, m)		(TX(n, m) + 0x64)
#define QSERDES_TX_N_HIGHZ_TRANSCEIVEREN_BIAS_DRVR_EN(n, m) (TX(n, m) + 0x68)
#define QSERDES_TX_N_LANE_MODE(n, m)			(TX(n, m) + 0x94)
#define QSERDES_TX_N_RCV_DETECT_LVL_2(n, m)		(TX(n, m) + 0xAC)

#define QSERDES_RX_N_UCDR_SO_GAIN_HALF(n, m)		(RX(n, m) + 0x010)
#define QSERDES_RX_N_UCDR_SO_GAIN(n, m)			(RX(n, m) + 0x01C)
#define QSERDES_RX_N_UCDR_SO_SATURATION_AND_ENABLE(n, m) (RX(n, m) + 0x048)
#define QSERDES_RX_N_RX_EQU_ADAPTOR_CNTRL2(n, m)	(RX(n, m) + 0x0D8)
#define QSERDES_RX_N_RX_EQU_ADAPTOR_CNTRL3(n, m)	(RX(n, m) + 0x0DC)
#define QSERDES_RX_N_RX_EQU_ADAPTOR_CNTRL4(n, m)	(RX(n, m) + 0x0E0)
#define QSERDES_RX_N_SIGDET_ENABLES(n, m)		(RX(n, m) + 0x110)
#define QSERDES_RX_N_SIGDET_DEGLITCH_CNTRL(n, m)	(RX(n, m) + 0x11C)
#define QSERDES_RX_N_SIGDET_LVL(n, m)			(RX(n, m) + 0x118)
#define QSERDES_RX_N_RX_BAND(n, m)			(RX(n, m) + 0x120)

#define PCIE_MISC_N_DEBUG_BUS_BYTE0_INDEX(n, m)	(PCS_MISC_PORT(n, m) + 0x00)
#define PCIE_MISC_N_DEBUG_BUS_BYTE1_INDEX(n, m)	(PCS_MISC_PORT(n, m) + 0x04)
#define PCIE_MISC_N_DEBUG_BUS_BYTE2_INDEX(n, m)	(PCS_MISC_PORT(n, m) + 0x08)
#define PCIE_MISC_N_DEBUG_BUS_BYTE3_INDEX(n, m)	(PCS_MISC_PORT(n, m) + 0x0C)
#define PCIE_MISC_N_DEBUG_BUS_0_STATUS(n, m)	(PCS_MISC_PORT(n, m) + 0x14)
#define PCIE_MISC_N_DEBUG_BUS_1_STATUS(n, m)	(PCS_MISC_PORT(n, m) + 0x18)
#define PCIE_MISC_N_DEBUG_BUS_2_STATUS(n, m)	(PCS_MISC_PORT(n, m) + 0x1C)
#define PCIE_MISC_N_DEBUG_BUS_3_STATUS(n, m)	(PCS_MISC_PORT(n, m) + 0x20)

#define PCIE_N_SW_RESET(n, m)			(PCS_PORT(n, m) + 0x00)
#define PCIE_N_POWER_DOWN_CONTROL(n, m)		(PCS_PORT(n, m) + 0x04)
#define PCIE_N_START_CONTROL(n, m)		(PCS_PORT(n, m) + 0x08)
#define PCIE_N_TXDEEMPH_M6DB_V0(n, m)		(PCS_PORT(n, m) + 0x24)
#define PCIE_N_TXDEEMPH_M3P5DB_V0(n, m)		(PCS_PORT(n, m) + 0x28)
#define PCIE_N_ENDPOINT_REFCLK_DRIVE(n, m)	(PCS_PORT(n, m) + 0x54)
#define PCIE_N_RX_IDLE_DTCT_CNTRL(n, m)		(PCS_PORT(n, m) + 0x58)
#define PCIE_N_POWER_STATE_CONFIG1(n, m)	(PCS_PORT(n, m) + 0x60)
#define PCIE_N_POWER_STATE_CONFIG4(n, m)	(PCS_PORT(n, m) + 0x6C)
#define PCIE_N_PWRUP_RESET_DLY_TIME_AUXCLK(n, m)	(PCS_PORT(n, m) + 0xA0)
#define PCIE_N_LP_WAKEUP_DLY_TIME_AUXCLK(n, m)	(PCS_PORT(n, m) + 0xA4)
#define PCIE_N_PLL_LOCK_CHK_DLY_TIME(n, m)	(PCS_PORT(n, m) + 0xA8)
#define PCIE_N_TEST_CONTROL4(n, m)		(PCS_PORT(n, m) + 0x11C)
#define PCIE_N_TEST_CONTROL5(n, m)		(PCS_PORT(n, m) + 0x120)
#define PCIE_N_TEST_CONTROL6(n, m)		(PCS_PORT(n, m) + 0x124)
#define PCIE_N_TEST_CONTROL7(n, m)		(PCS_PORT(n, m) + 0x128)
#define PCIE_N_DEBUG_BUS_0_STATUS(n, m)		(PCS_PORT(n, m) + 0x198)
#define PCIE_N_DEBUG_BUS_1_STATUS(n, m)		(PCS_PORT(n, m) + 0x19C)
#define PCIE_N_DEBUG_BUS_2_STATUS(n, m)		(PCS_PORT(n, m) + 0x1A0)
#define PCIE_N_DEBUG_BUS_3_STATUS(n, m)		(PCS_PORT(n, m) + 0x1A4)
#define PCIE_N_LP_WAKEUP_DLY_TIME_AUXCLK_MSB(n, m)	(PCS_PORT(n, m) + 0x1A8)
#define PCIE_N_OSC_DTCT_ACTIONS(n, m)			(PCS_PORT(n, m) + 0x1AC)
#define PCIE_N_SIGDET_CNTRL(n, m)			(PCS_PORT(n, m) + 0x1B0)
#define PCIE_N_L1SS_WAKEUP_DLY_TIME_AUXCLK_LSB(n, m)	(PCS_PORT(n, m) + 0x1DC)
#define PCIE_N_L1SS_WAKEUP_DLY_TIME_AUXCLK_MSB(n, m)	(PCS_PORT(n, m) + 0x1E0)

#define PCIE_COM_SW_RESET		0x400
#define PCIE_COM_POWER_DOWN_CONTROL	0x404
#define PCIE_COM_START_CONTROL		0x408
#define PCIE_COM_DEBUG_BUS_BYTE0_INDEX	0x438
#define PCIE_COM_DEBUG_BUS_BYTE1_INDEX	0x43C
#define PCIE_COM_DEBUG_BUS_BYTE2_INDEX	0x440
#define PCIE_COM_DEBUG_BUS_BYTE3_INDEX	0x444
#define PCIE_COM_PCS_READY_STATUS	0x448
#define PCIE_COM_DEBUG_BUS_0_STATUS	0x45C
#define PCIE_COM_DEBUG_BUS_1_STATUS	0x460
#define PCIE_COM_DEBUG_BUS_2_STATUS	0x464
#define PCIE_COM_DEBUG_BUS_3_STATUS	0x468

#define PCIE20_PARF_SYS_CTRL	     0x00
#define PCIE20_PARF_PM_STTS		0x24
#define PCIE20_PARF_PCS_DEEMPH	   0x34
#define PCIE20_PARF_PCS_SWING	    0x38
#define PCIE20_PARF_PHY_CTRL	     0x40
#define PCIE20_PARF_PHY_REFCLK	   0x4C
#define PCIE20_PARF_CONFIG_BITS	  0x50
#define PCIE20_PARF_TEST_BUS		0xE4
#define PCIE20_PARF_MHI_CLOCK_RESET_CTRL	0x174
#define PCIE20_PARF_AXI_MSTR_WR_ADDR_HALT   0x1A8
#define PCIE20_PARF_LTSSM              0x1B0
#define PCIE20_PARF_INT_ALL_STATUS	0x224
#define PCIE20_PARF_INT_ALL_CLEAR	0x228
#define PCIE20_PARF_INT_ALL_MASK	0x22C
#define PCIE20_PARF_SID_OFFSET		0x234
#define PCIE20_PARF_BDF_TRANSLATE_CFG	0x24C
#define PCIE20_PARF_BDF_TRANSLATE_N	0x250

#define PCIE20_ELBI_VERSION		0x00
#define PCIE20_ELBI_SYS_CTRL	     0x04
#define PCIE20_ELBI_SYS_STTS		 0x08

#define PCIE20_CAP			   0x70
#define PCIE20_CAP_DEVCTRLSTATUS	(PCIE20_CAP + 0x08)
#define PCIE20_CAP_LINKCTRLSTATUS	(PCIE20_CAP + 0x10)

#define PCIE20_COMMAND_STATUS	    0x04
#define PCIE20_HEADER_TYPE		0x0C
#define PCIE20_BUSNUMBERS		  0x18
#define PCIE20_MEMORY_BASE_LIMIT	 0x20
#define PCIE20_BRIDGE_CTRL		0x3C
#define PCIE20_L1SUB_CONTROL1	    0x158
#define PCIE20_DEVICE_CONTROL_STATUS	0x78
#define PCIE20_DEVICE_CONTROL2_STATUS2 0x98

#define PCIE20_AUX_CLK_FREQ_REG		0xB40
#define PCIE20_ACK_F_ASPM_CTRL_REG     0x70C
#define PCIE20_ACK_N_FTS		   0xff00

#define PCIE20_PLR_IATU_VIEWPORT	 0x900
#define PCIE20_PLR_IATU_CTRL1	    0x904
#define PCIE20_PLR_IATU_CTRL2	    0x908
#define PCIE20_PLR_IATU_LBAR	     0x90C
#define PCIE20_PLR_IATU_UBAR	     0x910
#define PCIE20_PLR_IATU_LAR		0x914
#define PCIE20_PLR_IATU_LTAR	     0x918
#define PCIE20_PLR_IATU_UTAR	     0x91c

#define PCIE20_CTRL1_TYPE_CFG0		0x04
#define PCIE20_CTRL1_TYPE_CFG1		0x05

#define PCIE20_CAP_ID			0x10
#define L1SUB_CAP_ID			0x1E

#define PCIE_CAP_PTR_OFFSET		0x34
#define PCIE_EXT_CAP_OFFSET		0x100

#define PCIE20_AER_UNCORR_ERR_STATUS_REG	0x104
#define PCIE20_AER_CORR_ERR_STATUS_REG		0x110
#define PCIE20_AER_ROOT_ERR_STATUS_REG		0x130
#define PCIE20_AER_ERR_SRC_ID_REG		0x134

#define RD 0
#define WR 1
#define MSM_PCIE_ERROR -1

#define PERST_PROPAGATION_DELAY_US_MIN	  1000
#define PERST_PROPAGATION_DELAY_US_MAX	  1005
#define REFCLK_STABILIZATION_DELAY_US_MIN     1000
#define REFCLK_STABILIZATION_DELAY_US_MAX     1005
#define LINK_UP_TIMEOUT_US_MIN		    5000
#define LINK_UP_TIMEOUT_US_MAX		    5100
#define LINK_UP_CHECK_MAX_COUNT		   20
#define PHY_STABILIZATION_DELAY_US_MIN	  995
#define PHY_STABILIZATION_DELAY_US_MAX	  1005
#define POWER_DOWN_DELAY_US_MIN		10
#define POWER_DOWN_DELAY_US_MAX		11
#define LINKDOWN_INIT_WAITING_US_MIN    995
#define LINKDOWN_INIT_WAITING_US_MAX    1005
#define LINKDOWN_WAITING_US_MIN	   4900
#define LINKDOWN_WAITING_US_MAX	   5100
#define LINKDOWN_WAITING_COUNT	    200

#define PHY_READY_TIMEOUT_COUNT		   10
#define XMLH_LINK_UP				  0x400
#define MAX_LINK_RETRIES 5
#define MAX_BUS_NUM 3
#define MAX_PROP_SIZE 32
#define MAX_RC_NAME_LEN 15
#define MSM_PCIE_MAX_VREG 4
#define MSM_PCIE_MAX_CLK 13
#define MSM_PCIE_MAX_PIPE_CLK 1
#define MAX_RC_NUM 3
#define MAX_DEVICE_NUM 20
#define MAX_SHORT_BDF_NUM 16
#define PCIE_TLP_RD_SIZE 0x5
#define PCIE_MSI_NR_IRQS 256
#define MSM_PCIE_MAX_MSI 32
#define MAX_MSG_LEN 80
#define PCIE_LOG_PAGES (50)
#define PCIE_CONF_SPACE_DW			1024
#define PCIE_CLEAR				0xDEADBEEF
#define PCIE_LINK_DOWN				0xFFFFFFFF

#define MSM_PCIE_MSI_PHY 0xa0000000
#define PCIE20_MSI_CTRL_ADDR		(0x820)
#define PCIE20_MSI_CTRL_UPPER_ADDR	(0x824)
#define PCIE20_MSI_CTRL_INTR_EN	   (0x828)
#define PCIE20_MSI_CTRL_INTR_MASK	 (0x82C)
#define PCIE20_MSI_CTRL_INTR_STATUS     (0x830)
#define PCIE20_MSI_CTRL_MAX 8

/* PM control options */
#define PM_IRQ			 0x1
#define PM_CLK			 0x2
#define PM_GPIO			0x4
#define PM_VREG			0x8
#define PM_PIPE_CLK		  0x10
#define PM_ALL (PM_IRQ | PM_CLK | PM_GPIO | PM_VREG | PM_PIPE_CLK)

#ifdef CONFIG_PHYS_ADDR_T_64BIT
#define PCIE_UPPER_ADDR(addr) ((u32)((addr) >> 32))
#else
#define PCIE_UPPER_ADDR(addr) (0x0)
#endif
#define PCIE_LOWER_ADDR(addr) ((u32)((addr) & 0xffffffff))

/* Config Space Offsets */
#define BDF_OFFSET(bus, devfn) \
	((bus << 24) | (devfn << 16))

#define PCIE_GEN_DBG(x...) do { \
	if (msm_pcie_debug_mask) \
		pr_alert(x); \
	} while (0)

#define PCIE_DBG(dev, fmt, arg...) do {			 \
	if ((dev) && (dev)->ipc_log_long)   \
		ipc_log_string((dev)->ipc_log_long, \
			"DBG1:%s: " fmt, __func__, arg); \
	if ((dev) && (dev)->ipc_log)   \
		ipc_log_string((dev)->ipc_log, "%s: " fmt, __func__, arg); \
	if (msm_pcie_debug_mask)   \
		pr_alert("%s: " fmt, __func__, arg);		  \
	} while (0)

#define PCIE_DBG2(dev, fmt, arg...) do {			 \
	if ((dev) && (dev)->ipc_log)   \
		ipc_log_string((dev)->ipc_log, "DBG2:%s: " fmt, __func__, arg);\
	if (msm_pcie_debug_mask)   \
		pr_alert("%s: " fmt, __func__, arg);              \
	} while (0)

#define PCIE_DBG3(dev, fmt, arg...) do {			 \
	if ((dev) && (dev)->ipc_log)   \
		ipc_log_string((dev)->ipc_log, "DBG3:%s: " fmt, __func__, arg);\
	if (msm_pcie_debug_mask)   \
		pr_alert("%s: " fmt, __func__, arg);              \
	} while (0)

#define PCIE_DUMP(dev, fmt, arg...) do {			\
	if ((dev) && (dev)->ipc_log_dump) \
		ipc_log_string((dev)->ipc_log_dump, \
			"DUMP:%s: " fmt, __func__, arg); \
	} while (0)

#define PCIE_DBG_FS(dev, fmt, arg...) do {			\
	if ((dev) && (dev)->ipc_log_dump) \
		ipc_log_string((dev)->ipc_log_dump, \
			"DBG_FS:%s: " fmt, __func__, arg); \
	pr_alert("%s: " fmt, __func__, arg); \
	} while (0)

#define PCIE_INFO(dev, fmt, arg...) do {			 \
	if ((dev) && (dev)->ipc_log_long)   \
		ipc_log_string((dev)->ipc_log_long, \
			"INFO:%s: " fmt, __func__, arg); \
	if ((dev) && (dev)->ipc_log)   \
		ipc_log_string((dev)->ipc_log, "%s: " fmt, __func__, arg); \
	pr_info("%s: " fmt, __func__, arg);  \
	} while (0)

#define PCIE_ERR(dev, fmt, arg...) do {			 \
	if ((dev) && (dev)->ipc_log_long)   \
		ipc_log_string((dev)->ipc_log_long, \
			"ERR:%s: " fmt, __func__, arg); \
	if ((dev) && (dev)->ipc_log)   \
		ipc_log_string((dev)->ipc_log, "%s: " fmt, __func__, arg); \
	pr_err("%s: " fmt, __func__, arg);  \
	} while (0)


enum msm_pcie_res {
	MSM_PCIE_RES_PARF,
	MSM_PCIE_RES_PHY,
	MSM_PCIE_RES_DM_CORE,
	MSM_PCIE_RES_ELBI,
	MSM_PCIE_RES_CONF,
	MSM_PCIE_RES_IO,
	MSM_PCIE_RES_BARS,
	MSM_PCIE_RES_TCSR,
	MSM_PCIE_MAX_RES,
};

enum msm_pcie_irq {
	MSM_PCIE_INT_MSI,
	MSM_PCIE_INT_A,
	MSM_PCIE_INT_B,
	MSM_PCIE_INT_C,
	MSM_PCIE_INT_D,
	MSM_PCIE_INT_PLS_PME,
	MSM_PCIE_INT_PME_LEGACY,
	MSM_PCIE_INT_PLS_ERR,
	MSM_PCIE_INT_AER_LEGACY,
	MSM_PCIE_INT_LINK_UP,
	MSM_PCIE_INT_LINK_DOWN,
	MSM_PCIE_INT_BRIDGE_FLUSH_N,
	MSM_PCIE_INT_GLOBAL_INT,
	MSM_PCIE_MAX_IRQ,
};

enum msm_pcie_irq_event {
	MSM_PCIE_INT_EVT_LINK_DOWN = 1,
	MSM_PCIE_INT_EVT_BME,
	MSM_PCIE_INT_EVT_PM_TURNOFF,
	MSM_PCIE_INT_EVT_DEBUG,
	MSM_PCIE_INT_EVT_LTR,
	MSM_PCIE_INT_EVT_MHI_Q6,
	MSM_PCIE_INT_EVT_MHI_A7,
	MSM_PCIE_INT_EVT_DSTATE_CHANGE,
	MSM_PCIE_INT_EVT_L1SUB_TIMEOUT,
	MSM_PCIE_INT_EVT_MMIO_WRITE,
	MSM_PCIE_INT_EVT_CFG_WRITE,
	MSM_PCIE_INT_EVT_BRIDGE_FLUSH_N,
	MSM_PCIE_INT_EVT_LINK_UP,
	MSM_PCIE_INT_EVT_AER_LEGACY,
	MSM_PCIE_INT_EVT_AER_ERR,
	MSM_PCIE_INT_EVT_PME_LEGACY,
	MSM_PCIE_INT_EVT_PLS_PME,
	MSM_PCIE_INT_EVT_INTD,
	MSM_PCIE_INT_EVT_INTC,
	MSM_PCIE_INT_EVT_INTB,
	MSM_PCIE_INT_EVT_INTA,
	MSM_PCIE_INT_EVT_EDMA,
	MSM_PCIE_INT_EVT_MSI_0,
	MSM_PCIE_INT_EVT_MSI_1,
	MSM_PCIE_INT_EVT_MSI_2,
	MSM_PCIE_INT_EVT_MSI_3,
	MSM_PCIE_INT_EVT_MSI_4,
	MSM_PCIE_INT_EVT_MSI_5,
	MSM_PCIE_INT_EVT_MSI_6,
	MSM_PCIE_INT_EVT_MSI_7,
	MSM_PCIE_INT_EVT_MAX = 30,
};

enum msm_pcie_gpio {
	MSM_PCIE_GPIO_PERST,
	MSM_PCIE_GPIO_WAKE,
	MSM_PCIE_GPIO_EP,
	MSM_PCIE_MAX_GPIO
};

enum msm_pcie_link_status {
	MSM_PCIE_LINK_DEINIT,
	MSM_PCIE_LINK_ENABLED,
	MSM_PCIE_LINK_DISABLED
};

/* gpio info structure */
struct msm_pcie_gpio_info_t {
	char	*name;
	uint32_t   num;
	bool	 out;
	uint32_t   on;
	uint32_t   init;
	bool	required;
};

/* voltage regulator info structrue */
struct msm_pcie_vreg_info_t {
	struct regulator  *hdl;
	char		  *name;
	uint32_t	     max_v;
	uint32_t	     min_v;
	uint32_t	     opt_mode;
	bool		   required;
};

/* clock info structure */
struct msm_pcie_clk_info_t {
	struct clk  *hdl;
	char	  *name;
	u32	   freq;
	bool	  required;
};

/* resource info structure */
struct msm_pcie_res_info_t {
	char		*name;
	struct resource *resource;
	void __iomem    *base;
};

/* irq info structrue */
struct msm_pcie_irq_info_t {
	char		  *name;
	uint32_t	    num;
};

/* phy info structure */
struct msm_pcie_phy_info_t {
	u32	offset;
	u32	val;
	u32	delay;
};

/* PCIe device info structure */
struct msm_pcie_device_info {
	u32			bdf;
	struct pci_dev		*dev;
	short			short_bdf;
	u32			sid;
	int			domain;
	void __iomem		*conf_base;
	unsigned long		phy_address;
	u32			dev_ctrlstts_offset;
	struct msm_pcie_register_event *event_reg;
	bool			registered;
};

/* msm pcie device structure */
struct msm_pcie_dev_t {
	struct platform_device	 *pdev;
	struct pci_dev *dev;
	struct regulator *gdsc;
	struct regulator *gdsc_smmu;
	struct msm_pcie_vreg_info_t  vreg[MSM_PCIE_MAX_VREG];
	struct msm_pcie_gpio_info_t  gpio[MSM_PCIE_MAX_GPIO];
	struct msm_pcie_clk_info_t   clk[MSM_PCIE_MAX_CLK];
	struct msm_pcie_clk_info_t   pipeclk[MSM_PCIE_MAX_PIPE_CLK];
	struct msm_pcie_res_info_t   res[MSM_PCIE_MAX_RES];
	struct msm_pcie_irq_info_t   irq[MSM_PCIE_MAX_IRQ];
	struct msm_pcie_irq_info_t	msi[MSM_PCIE_MAX_MSI];

	void __iomem		     *parf;
	void __iomem		     *phy;
	void __iomem		     *elbi;
	void __iomem		     *dm_core;
	void __iomem		     *conf;
	void __iomem		     *bars;
	void __iomem		     *tcsr;

	uint32_t			    axi_bar_start;
	uint32_t			    axi_bar_end;

	struct resource		   *dev_mem_res;
	struct resource		   *dev_io_res;

	uint32_t			    wake_n;
	uint32_t			    vreg_n;
	uint32_t			    gpio_n;
	uint32_t			    parf_deemph;
	uint32_t			    parf_swing;

	bool				 cfg_access;
	spinlock_t			 cfg_lock;
	unsigned long		    irqsave_flags;
	struct mutex		     setup_lock;

	struct irq_domain		*irq_domain;
	DECLARE_BITMAP(msi_irq_in_use, PCIE_MSI_NR_IRQS);
	uint32_t			   msi_gicm_addr;
	uint32_t			   msi_gicm_base;
	bool				 use_msi;

	enum msm_pcie_link_status    link_status;
	bool				 user_suspend;
	bool                         disable_pc;
	struct pci_saved_state	     *saved_state;

	struct wakeup_source	     ws;
	struct msm_bus_scale_pdata   *bus_scale_table;
	uint32_t			   bus_client;

	bool				l0s_supported;
	bool				l1_supported;
	bool				 l1ss_supported;
	bool				common_clk_en;
	bool				clk_power_manage_en;
	bool				 aux_clk_sync;
	bool				aer_enable;
	bool				smmu_exist;
	uint32_t			   n_fts;
	bool				 ext_ref_clk;
	bool				common_phy;
	uint32_t			   ep_latency;
	uint32_t			cpl_timeout;
	uint32_t			current_bdf;
	short				current_short_bdf;
	uint32_t			perst_delay_us_min;
	uint32_t			perst_delay_us_max;
	uint32_t			tlp_rd_size;
	bool				linkdown_panic;
	bool				 ep_wakeirq;

	uint32_t			   rc_idx;
	uint32_t			phy_ver;
	bool				drv_ready;
	bool				 enumerated;
	struct work_struct	     handle_wake_work;
	struct mutex		     recovery_lock;
	spinlock_t                   linkdown_lock;
	spinlock_t                   wakeup_lock;
	spinlock_t			global_irq_lock;
	spinlock_t			aer_lock;
	ulong				linkdown_counter;
	ulong				link_turned_on_counter;
	ulong				link_turned_off_counter;
	ulong				rc_corr_counter;
	ulong				rc_non_fatal_counter;
	ulong				rc_fatal_counter;
	ulong				ep_corr_counter;
	ulong				ep_non_fatal_counter;
	ulong				ep_fatal_counter;
	bool				 suspending;
	ulong				wake_counter;
	u32				num_active_ep;
	u32				num_ep;
	bool				pending_ep_reg;
	u32				phy_len;
	u32				port_phy_len;
	struct msm_pcie_phy_info_t	*phy_sequence;
	struct msm_pcie_phy_info_t	*port_phy_sequence;
	u32		ep_shadow[MAX_DEVICE_NUM][PCIE_CONF_SPACE_DW];
	u32				  rc_shadow[PCIE_CONF_SPACE_DW];
	bool				 shadow_en;
	bool				bridge_found;
	struct msm_pcie_register_event *event_reg;
	unsigned int			scm_dev_id;
	bool				 power_on;
	void				 *ipc_log;
	void				*ipc_log_long;
	void				*ipc_log_dump;
	bool				use_19p2mhz_aux_clk;
	bool				use_pinctrl;
	struct pinctrl			*pinctrl;
	struct pinctrl_state		*pins_default;
	struct pinctrl_state		*pins_sleep;
	struct msm_pcie_device_info   pcidev_table[MAX_DEVICE_NUM];
};


/* debug mask sys interface */
static int msm_pcie_debug_mask;
module_param_named(debug_mask, msm_pcie_debug_mask,
			    int, S_IRUGO | S_IWUSR | S_IWGRP);

/* debugfs values */
static u32 rc_sel;
static u32 base_sel;
static u32 wr_offset;
static u32 wr_mask;
static u32 wr_value;
static ulong corr_counter_limit = 5;

/* counter to keep track if common PHY needs to be configured */
static u32 num_rc_on;

/* global lock for PCIe common PHY */
static struct mutex com_phy_lock;

/* Table to track info of PCIe devices */
static struct msm_pcie_device_info
	msm_pcie_dev_tbl[MAX_RC_NUM * MAX_DEVICE_NUM];

/* PCIe driver state */
struct pcie_drv_sta {
	u32 rc_num;
	struct mutex drv_lock;
} pcie_drv;

/* msm pcie device data */
static struct msm_pcie_dev_t msm_pcie_dev[MAX_RC_NUM];

/* regulators */
static struct msm_pcie_vreg_info_t msm_pcie_vreg_info[MSM_PCIE_MAX_VREG] = {
	{NULL, "vreg-3.3", 0, 0, 0, false},
	{NULL, "vreg-1.8", 1800000, 1800000, 14000, true},
	{NULL, "vreg-0.9", 1000000, 1000000, 40000, true},
	{NULL, "vreg-cx", 0, 0, 0, false}
};

/* GPIOs */
static struct msm_pcie_gpio_info_t msm_pcie_gpio_info[MSM_PCIE_MAX_GPIO] = {
	{"perst-gpio",		0, 1, 0, 0, 1},
	{"wake-gpio",		0, 0, 0, 0, 0},
	{"qcom,ep-gpio",	0, 1, 1, 0, 0}
};

/* clocks */
static struct msm_pcie_clk_info_t
	msm_pcie_clk_info[MAX_RC_NUM][MSM_PCIE_MAX_CLK] = {
	{
	{NULL, "pcie_0_ref_clk_src", 0, false},
	{NULL, "pcie_0_aux_clk", 1010000, true},
	{NULL, "pcie_0_cfg_ahb_clk", 0, true},
	{NULL, "pcie_0_mstr_axi_clk", 0, true},
	{NULL, "pcie_0_slv_axi_clk", 0, true},
	{NULL, "pcie_0_ldo", 0, true},
	{NULL, "pcie_0_smmu_clk", 0, false},
	{NULL, "pcie_phy_cfg_ahb_clk", 0, false},
	{NULL, "pcie_phy_aux_clk", 0, false},
	{NULL, "pcie_phy_reset", 0, false},
	{NULL, "pcie_phy_com_reset", 0, false},
	{NULL, "pcie_phy_nocsr_com_phy_reset", 0, false},
	{NULL, "pcie_0_phy_reset", 0, true}
	},
	{
	{NULL, "pcie_1_ref_clk_src", 0, false},
	{NULL, "pcie_1_aux_clk", 1010000, true},
	{NULL, "pcie_1_cfg_ahb_clk", 0, true},
	{NULL, "pcie_1_mstr_axi_clk", 0, true},
	{NULL, "pcie_1_slv_axi_clk", 0, true},
	{NULL, "pcie_1_ldo", 0, true},
	{NULL, "pcie_1_smmu_clk", 0, false},
	{NULL, "pcie_phy_cfg_ahb_clk", 0, false},
	{NULL, "pcie_phy_aux_clk", 0, false},
	{NULL, "pcie_phy_reset", 0, false},
	{NULL, "pcie_phy_com_reset", 0, false},
	{NULL, "pcie_phy_nocsr_com_phy_reset", 0, false},
	{NULL, "pcie_1_phy_reset", 0, true}
	},
	{
	{NULL, "pcie_2_ref_clk_src", 0, false},
	{NULL, "pcie_2_aux_clk", 1010000, true},
	{NULL, "pcie_2_cfg_ahb_clk", 0, true},
	{NULL, "pcie_2_mstr_axi_clk", 0, true},
	{NULL, "pcie_2_slv_axi_clk", 0, true},
	{NULL, "pcie_2_ldo", 0, true},
	{NULL, "pcie_2_smmu_clk", 0, false},
	{NULL, "pcie_phy_cfg_ahb_clk", 0, false},
	{NULL, "pcie_phy_aux_clk", 0, false},
	{NULL, "pcie_phy_reset", 0, false},
	{NULL, "pcie_phy_com_reset", 0, false},
	{NULL, "pcie_phy_nocsr_com_phy_reset", 0, false},
	{NULL, "pcie_2_phy_reset", 0, true}
	}
};

/* Pipe Clocks */
static struct msm_pcie_clk_info_t
	msm_pcie_pipe_clk_info[MAX_RC_NUM][MSM_PCIE_MAX_PIPE_CLK] = {
	{
	{NULL, "pcie_0_pipe_clk", 125000000, true},
	},
	{
	{NULL, "pcie_1_pipe_clk", 125000000, true},
	},
	{
	{NULL, "pcie_2_pipe_clk", 125000000, true},
	}
};

/* resources */
static const struct msm_pcie_res_info_t msm_pcie_res_info[MSM_PCIE_MAX_RES] = {
	{"parf",	0, 0},
	{"phy",     0, 0},
	{"dm_core",	0, 0},
	{"elbi",	0, 0},
	{"conf",	0, 0},
	{"io",		0, 0},
	{"bars",	0, 0},
	{"tcsr",	0, 0}
};

/* irqs */
static const struct msm_pcie_irq_info_t msm_pcie_irq_info[MSM_PCIE_MAX_IRQ] = {
	{"int_msi",	0},
	{"int_a",	0},
	{"int_b",	0},
	{"int_c",	0},
	{"int_d",	0},
	{"int_pls_pme",		0},
	{"int_pme_legacy",	0},
	{"int_pls_err",		0},
	{"int_aer_legacy",	0},
	{"int_pls_link_up",	0},
	{"int_pls_link_down",	0},
	{"int_bridge_flush_n",	0},
	{"int_global_int",	0}
};

/* MSIs */
static const struct msm_pcie_irq_info_t msm_pcie_msi_info[MSM_PCIE_MAX_MSI] = {
	{"msi_0", 0}, {"msi_1", 0}, {"msi_2", 0}, {"msi_3", 0},
	{"msi_4", 0}, {"msi_5", 0}, {"msi_6", 0}, {"msi_7", 0},
	{"msi_8", 0}, {"msi_9", 0}, {"msi_10", 0}, {"msi_11", 0},
	{"msi_12", 0}, {"msi_13", 0}, {"msi_14", 0}, {"msi_15", 0},
	{"msi_16", 0}, {"msi_17", 0}, {"msi_18", 0}, {"msi_19", 0},
	{"msi_20", 0}, {"msi_21", 0}, {"msi_22", 0}, {"msi_23", 0},
	{"msi_24", 0}, {"msi_25", 0}, {"msi_26", 0}, {"msi_27", 0},
	{"msi_28", 0}, {"msi_29", 0}, {"msi_30", 0}, {"msi_31", 0}
};

#ifdef CONFIG_ARM
#define PCIE_BUS_PRIV_DATA(bus) \
	(((struct pci_sys_data *)bus->sysdata)->private_data)

static struct pci_sys_data msm_pcie_sys_data[MAX_RC_NUM];

static inline void *msm_pcie_setup_sys_data(struct msm_pcie_dev_t *dev)
{
	msm_pcie_sys_data[dev->rc_idx].domain = dev->rc_idx;
	msm_pcie_sys_data[dev->rc_idx].private_data = dev;

	return &msm_pcie_sys_data[dev->rc_idx];
}

static inline void msm_pcie_fixup_irqs(struct msm_pcie_dev_t *dev)
{
	pci_fixup_irqs(pci_common_swizzle, of_irq_parse_and_map_pci);
}
#else
#define PCIE_BUS_PRIV_DATA(bus) \
	(struct msm_pcie_dev_t *)(bus->sysdata)

static inline void *msm_pcie_setup_sys_data(struct msm_pcie_dev_t *dev)
{
	return dev;
}

static inline void msm_pcie_fixup_irqs(struct msm_pcie_dev_t *dev)
{
}
#endif

static inline void msm_pcie_write_reg(void *base, u32 offset, u32 value)
{
	writel_relaxed(value, base + offset);
	wmb();
}

static inline void msm_pcie_write_reg_field(void *base, u32 offset,
	const u32 mask, u32 val)
{
	u32 shift = find_first_bit((void *)&mask, 32);
	u32 tmp = readl_relaxed(base + offset);

	tmp &= ~mask; /* clear written bits */
	val = tmp | (val << shift);
	writel_relaxed(val, base + offset);
	wmb();
}

#if defined(CONFIG_ARCH_FSM9010)
#define PCIE20_PARF_PHY_STTS         0x3c
#define PCIE2_PHY_RESET_CTRL         0x44
#define PCIE20_PARF_PHY_REFCLK_CTRL2 0xa0
#define PCIE20_PARF_PHY_REFCLK_CTRL3 0xa4
#define PCIE20_PARF_PCS_SWING_CTRL1  0x88
#define PCIE20_PARF_PCS_SWING_CTRL2  0x8c
#define PCIE20_PARF_PCS_DEEMPH1      0x74
#define PCIE20_PARF_PCS_DEEMPH2      0x78
#define PCIE20_PARF_PCS_DEEMPH3      0x7c
#define PCIE20_PARF_CONFIGBITS       0x84
#define PCIE20_PARF_PHY_CTRL3        0x94
#define PCIE20_PARF_PCS_CTRL         0x80

#define TX_AMP_VAL                   127
#define PHY_RX0_EQ_GEN1_VAL          0
#define PHY_RX0_EQ_GEN2_VAL          4
#define TX_DEEMPH_GEN1_VAL           24
#define TX_DEEMPH_GEN2_3_5DB_VAL     24
#define TX_DEEMPH_GEN2_6DB_VAL       34
#define PHY_TX0_TERM_OFFST_VAL       0

static inline void pcie_phy_dump(struct msm_pcie_dev_t *dev)
{
}

static inline void pcie20_phy_reset(struct msm_pcie_dev_t *dev, uint32_t assert)
{
	msm_pcie_write_reg_field(dev->phy, PCIE2_PHY_RESET_CTRL,
					 BIT(0), (assert) ? 1 : 0);
}

static void pcie_phy_init(struct msm_pcie_dev_t *dev)
{
	PCIE_DBG(dev, "RC%d: Initializing 28LP SNS phy - 100MHz\n",
		dev->rc_idx);

	/* De-assert Phy SW Reset */
	pcie20_phy_reset(dev, 1);

	/* Program SSP ENABLE */
	if (readl_relaxed(dev->phy + PCIE20_PARF_PHY_REFCLK_CTRL2) & BIT(0))
		msm_pcie_write_reg_field(dev->phy, PCIE20_PARF_PHY_REFCLK_CTRL2,
								 BIT(0), 0);
	if ((readl_relaxed(dev->phy + PCIE20_PARF_PHY_REFCLK_CTRL3) &
								 BIT(0)) == 0)
		msm_pcie_write_reg_field(dev->phy, PCIE20_PARF_PHY_REFCLK_CTRL3,
								 BIT(0), 1);
	/* Program Tx Amplitude */
	if ((readl_relaxed(dev->phy + PCIE20_PARF_PCS_SWING_CTRL1) &
		(BIT(6)|BIT(5)|BIT(4)|BIT(3)|BIT(2)|BIT(1)|BIT(0))) !=
				TX_AMP_VAL)
		msm_pcie_write_reg_field(dev->phy, PCIE20_PARF_PCS_SWING_CTRL1,
			BIT(6)|BIT(5)|BIT(4)|BIT(3)|BIT(2)|BIT(1)|BIT(0),
				TX_AMP_VAL);
	if ((readl_relaxed(dev->phy + PCIE20_PARF_PCS_SWING_CTRL2) &
		(BIT(6)|BIT(5)|BIT(4)|BIT(3)|BIT(2)|BIT(1)|BIT(0))) !=
				TX_AMP_VAL)
		msm_pcie_write_reg_field(dev->phy, PCIE20_PARF_PCS_SWING_CTRL2,
			BIT(6)|BIT(5)|BIT(4)|BIT(3)|BIT(2)|BIT(1)|BIT(0),
				TX_AMP_VAL);
	/* Program De-Emphasis */
	if ((readl_relaxed(dev->phy + PCIE20_PARF_PCS_DEEMPH1) &
			(BIT(5)|BIT(4)|BIT(3)|BIT(2)|BIT(1)|BIT(0))) !=
				TX_DEEMPH_GEN2_6DB_VAL)
		msm_pcie_write_reg_field(dev->phy, PCIE20_PARF_PCS_DEEMPH1,
			BIT(5)|BIT(4)|BIT(3)|BIT(2)|BIT(1)|BIT(0),
				TX_DEEMPH_GEN2_6DB_VAL);

	if ((readl_relaxed(dev->phy + PCIE20_PARF_PCS_DEEMPH2) &
			(BIT(5)|BIT(4)|BIT(3)|BIT(2)|BIT(1)|BIT(0))) !=
				TX_DEEMPH_GEN2_3_5DB_VAL)
		msm_pcie_write_reg_field(dev->phy, PCIE20_PARF_PCS_DEEMPH2,
			BIT(5)|BIT(4)|BIT(3)|BIT(2)|BIT(1)|BIT(0),
				TX_DEEMPH_GEN2_3_5DB_VAL);

	if ((readl_relaxed(dev->phy + PCIE20_PARF_PCS_DEEMPH3) &
			(BIT(5)|BIT(4)|BIT(3)|BIT(2)|BIT(1)|BIT(0))) !=
				TX_DEEMPH_GEN1_VAL)
		msm_pcie_write_reg_field(dev->phy, PCIE20_PARF_PCS_DEEMPH3,
			BIT(5)|BIT(4)|BIT(3)|BIT(2)|BIT(1)|BIT(0),
				TX_DEEMPH_GEN1_VAL);

	/* Program Rx_Eq */
	if ((readl_relaxed(dev->phy + PCIE20_PARF_CONFIGBITS) &
			(BIT(2)|BIT(1)|BIT(0))) != PHY_RX0_EQ_GEN1_VAL)
		msm_pcie_write_reg_field(dev->phy, PCIE20_PARF_CONFIGBITS,
				 BIT(2)|BIT(1)|BIT(0), PHY_RX0_EQ_GEN1_VAL);

	/* Program Tx0_term_offset */
	if ((readl_relaxed(dev->phy + PCIE20_PARF_PHY_CTRL3) &
			(BIT(4)|BIT(3)|BIT(2)|BIT(1)|BIT(0))) !=
				PHY_TX0_TERM_OFFST_VAL)
		msm_pcie_write_reg_field(dev->phy, PCIE20_PARF_PHY_CTRL3,
			 BIT(4)|BIT(3)|BIT(2)|BIT(1)|BIT(0),
				PHY_TX0_TERM_OFFST_VAL);

	/* Program REF_CLK source */
	msm_pcie_write_reg_field(dev->phy, PCIE20_PARF_PHY_REFCLK_CTRL2, BIT(1),
		(dev->ext_ref_clk) ? 1 : 0);
	/* disable Tx2Rx Loopback */
	if (readl_relaxed(dev->phy + PCIE20_PARF_PCS_CTRL) & BIT(1))
		msm_pcie_write_reg_field(dev->phy, PCIE20_PARF_PCS_CTRL,
								 BIT(1), 0);
	/* De-assert Phy SW Reset */
	pcie20_phy_reset(dev, 0);
}

static bool pcie_phy_is_ready(struct msm_pcie_dev_t *dev)
{

	/* read PCIE20_PARF_PHY_STTS twice */
	readl_relaxed(dev->phy + PCIE20_PARF_PHY_STTS);
	if (readl_relaxed(dev->phy + PCIE20_PARF_PHY_STTS) & BIT(0))
		return false;
	else
		return true;
}
#else
static void pcie_phy_dump_test_cntrl(struct msm_pcie_dev_t *dev,
					u32 cntrl4_val, u32 cntrl5_val,
					u32 cntrl6_val, u32 cntrl7_val)
{
	msm_pcie_write_reg(dev->phy,
		PCIE_N_TEST_CONTROL4(dev->rc_idx, dev->common_phy), cntrl4_val);
	msm_pcie_write_reg(dev->phy,
		PCIE_N_TEST_CONTROL5(dev->rc_idx, dev->common_phy), cntrl5_val);
	msm_pcie_write_reg(dev->phy,
		PCIE_N_TEST_CONTROL6(dev->rc_idx, dev->common_phy), cntrl6_val);
	msm_pcie_write_reg(dev->phy,
		PCIE_N_TEST_CONTROL7(dev->rc_idx, dev->common_phy), cntrl7_val);

	PCIE_DUMP(dev,
		"PCIe: RC%d PCIE_N_TEST_CONTROL4: 0x%x\n", dev->rc_idx,
		readl_relaxed(dev->phy +
			PCIE_N_TEST_CONTROL4(dev->rc_idx,
				dev->common_phy)));
	PCIE_DUMP(dev,
		"PCIe: RC%d PCIE_N_TEST_CONTROL5: 0x%x\n", dev->rc_idx,
		readl_relaxed(dev->phy +
			PCIE_N_TEST_CONTROL5(dev->rc_idx,
				dev->common_phy)));
	PCIE_DUMP(dev,
		"PCIe: RC%d PCIE_N_TEST_CONTROL6: 0x%x\n", dev->rc_idx,
		readl_relaxed(dev->phy +
			PCIE_N_TEST_CONTROL6(dev->rc_idx,
				dev->common_phy)));
	PCIE_DUMP(dev,
		"PCIe: RC%d PCIE_N_TEST_CONTROL7: 0x%x\n", dev->rc_idx,
		readl_relaxed(dev->phy +
			PCIE_N_TEST_CONTROL7(dev->rc_idx,
				dev->common_phy)));
	PCIE_DUMP(dev,
		"PCIe: RC%d PCIE_N_DEBUG_BUS_0_STATUS: 0x%x\n", dev->rc_idx,
		readl_relaxed(dev->phy +
			PCIE_N_DEBUG_BUS_0_STATUS(dev->rc_idx,
				dev->common_phy)));
	PCIE_DUMP(dev,
		"PCIe: RC%d PCIE_N_DEBUG_BUS_1_STATUS: 0x%x\n", dev->rc_idx,
		readl_relaxed(dev->phy +
			PCIE_N_DEBUG_BUS_1_STATUS(dev->rc_idx,
				dev->common_phy)));
	PCIE_DUMP(dev,
		"PCIe: RC%d PCIE_N_DEBUG_BUS_2_STATUS: 0x%x\n", dev->rc_idx,
		readl_relaxed(dev->phy +
			PCIE_N_DEBUG_BUS_2_STATUS(dev->rc_idx,
				dev->common_phy)));
	PCIE_DUMP(dev,
		"PCIe: RC%d PCIE_N_DEBUG_BUS_3_STATUS: 0x%x\n\n", dev->rc_idx,
		readl_relaxed(dev->phy +
			PCIE_N_DEBUG_BUS_3_STATUS(dev->rc_idx,
				dev->common_phy)));
}

static void pcie_phy_dump(struct msm_pcie_dev_t *dev)
{
	int i, size;
	u32 write_val;

	PCIE_DUMP(dev, "PCIe: RC%d PHY testbus\n", dev->rc_idx);

	pcie_phy_dump_test_cntrl(dev, 0x18, 0x19, 0x1A, 0x1B);
	pcie_phy_dump_test_cntrl(dev, 0x1C, 0x1D, 0x1E, 0x1F);
	pcie_phy_dump_test_cntrl(dev, 0x20, 0x21, 0x22, 0x23);

	for (i = 0; i < 3; i++) {
		write_val = 0x1 + i;
		msm_pcie_write_reg(dev->phy,
			QSERDES_TX_N_DEBUG_BUS_SEL(dev->rc_idx,
				dev->common_phy), write_val);
		PCIE_DUMP(dev,
			"PCIe: RC%d QSERDES_TX_N_DEBUG_BUS_SEL: 0x%x\n",
			dev->rc_idx,
			readl_relaxed(dev->phy +
				QSERDES_TX_N_DEBUG_BUS_SEL(dev->rc_idx,
					dev->common_phy)));

		pcie_phy_dump_test_cntrl(dev, 0x30, 0x31, 0x32, 0x33);
	}

	pcie_phy_dump_test_cntrl(dev, 0, 0, 0, 0);

	if (dev->phy_ver >= 0x10 && dev->phy_ver < 0x20) {
		pcie_phy_dump_test_cntrl(dev, 0x01, 0x02, 0x03, 0x0A);
		pcie_phy_dump_test_cntrl(dev, 0x0E, 0x0F, 0x12, 0x13);
		pcie_phy_dump_test_cntrl(dev, 0, 0, 0, 0);

		for (i = 0; i < 8; i += 4) {
			write_val = 0x1 + i;
			msm_pcie_write_reg(dev->phy,
				PCIE_MISC_N_DEBUG_BUS_BYTE0_INDEX(dev->rc_idx,
					dev->common_phy), write_val);
			msm_pcie_write_reg(dev->phy,
				PCIE_MISC_N_DEBUG_BUS_BYTE1_INDEX(dev->rc_idx,
					dev->common_phy), write_val + 1);
			msm_pcie_write_reg(dev->phy,
				PCIE_MISC_N_DEBUG_BUS_BYTE2_INDEX(dev->rc_idx,
					dev->common_phy), write_val + 2);
			msm_pcie_write_reg(dev->phy,
				PCIE_MISC_N_DEBUG_BUS_BYTE3_INDEX(dev->rc_idx,
					dev->common_phy), write_val + 3);

			PCIE_DUMP(dev,
				"PCIe: RC%d to PCIE_MISC_N_DEBUG_BUS_BYTE0_INDEX: 0x%x\n",
				dev->rc_idx,
				readl_relaxed(dev->phy +
					PCIE_MISC_N_DEBUG_BUS_BYTE0_INDEX(
						dev->rc_idx, dev->common_phy)));
			PCIE_DUMP(dev,
				"PCIe: RC%d to PCIE_MISC_N_DEBUG_BUS_BYTE1_INDEX: 0x%x\n",
				dev->rc_idx,
				readl_relaxed(dev->phy +
					PCIE_MISC_N_DEBUG_BUS_BYTE1_INDEX(
						dev->rc_idx, dev->common_phy)));
			PCIE_DUMP(dev,
				"PCIe: RC%d to PCIE_MISC_N_DEBUG_BUS_BYTE2_INDEX: 0x%x\n",
				dev->rc_idx,
				readl_relaxed(dev->phy +
					PCIE_MISC_N_DEBUG_BUS_BYTE2_INDEX(
						dev->rc_idx, dev->common_phy)));
			PCIE_DUMP(dev,
				"PCIe: RC%d to PCIE_MISC_N_DEBUG_BUS_BYTE3_INDEX: 0x%x\n",
				dev->rc_idx,
				readl_relaxed(dev->phy +
					PCIE_MISC_N_DEBUG_BUS_BYTE3_INDEX(
						dev->rc_idx, dev->common_phy)));
			PCIE_DUMP(dev,
				"PCIe: RC%d PCIE_MISC_N_DEBUG_BUS_0_STATUS: 0x%x\n",
				dev->rc_idx,
				readl_relaxed(dev->phy +
					PCIE_MISC_N_DEBUG_BUS_0_STATUS(
						dev->rc_idx, dev->common_phy)));
			PCIE_DUMP(dev,
				"PCIe: RC%d PCIE_MISC_N_DEBUG_BUS_1_STATUS: 0x%x\n",
				dev->rc_idx,
				readl_relaxed(dev->phy +
					PCIE_MISC_N_DEBUG_BUS_1_STATUS(
						dev->rc_idx, dev->common_phy)));
			PCIE_DUMP(dev,
				"PCIe: RC%d PCIE_MISC_N_DEBUG_BUS_2_STATUS: 0x%x\n",
				dev->rc_idx,
				readl_relaxed(dev->phy +
					PCIE_MISC_N_DEBUG_BUS_2_STATUS(
						dev->rc_idx, dev->common_phy)));
			PCIE_DUMP(dev,
				"PCIe: RC%d PCIE_MISC_N_DEBUG_BUS_3_STATUS: 0x%x\n",
				dev->rc_idx,
				readl_relaxed(dev->phy +
					PCIE_MISC_N_DEBUG_BUS_3_STATUS(
						dev->rc_idx, dev->common_phy)));
		}

		msm_pcie_write_reg(dev->phy,
			PCIE_MISC_N_DEBUG_BUS_BYTE0_INDEX(
				dev->rc_idx, dev->common_phy), 0);
		msm_pcie_write_reg(dev->phy,
			PCIE_MISC_N_DEBUG_BUS_BYTE1_INDEX(
				dev->rc_idx, dev->common_phy), 0);
		msm_pcie_write_reg(dev->phy,
			PCIE_MISC_N_DEBUG_BUS_BYTE2_INDEX(
				dev->rc_idx, dev->common_phy), 0);
		msm_pcie_write_reg(dev->phy,
			PCIE_MISC_N_DEBUG_BUS_BYTE3_INDEX(
				dev->rc_idx, dev->common_phy), 0);
	}

	for (i = 0; i < 2; i++) {
		write_val = 0x2 + i;

		msm_pcie_write_reg(dev->phy, QSERDES_COM_DEBUG_BUS_SEL,
			write_val);

		PCIE_DUMP(dev,
			"PCIe: RC%d to QSERDES_COM_DEBUG_BUS_SEL: 0x%x\n",
			dev->rc_idx,
			readl_relaxed(dev->phy + QSERDES_COM_DEBUG_BUS_SEL));
		PCIE_DUMP(dev,
			"PCIe: RC%d QSERDES_COM_DEBUG_BUS0: 0x%x\n",
			dev->rc_idx,
			readl_relaxed(dev->phy + QSERDES_COM_DEBUG_BUS0));
		PCIE_DUMP(dev,
			"PCIe: RC%d QSERDES_COM_DEBUG_BUS1: 0x%x\n",
			dev->rc_idx,
			readl_relaxed(dev->phy + QSERDES_COM_DEBUG_BUS1));
		PCIE_DUMP(dev,
			"PCIe: RC%d QSERDES_COM_DEBUG_BUS2: 0x%x\n",
			dev->rc_idx,
			readl_relaxed(dev->phy + QSERDES_COM_DEBUG_BUS2));
		PCIE_DUMP(dev,
			"PCIe: RC%d QSERDES_COM_DEBUG_BUS3: 0x%x\n\n",
			dev->rc_idx,
			readl_relaxed(dev->phy + QSERDES_COM_DEBUG_BUS3));
	}

	msm_pcie_write_reg(dev->phy, QSERDES_COM_DEBUG_BUS_SEL, 0);

	if (dev->common_phy) {
		msm_pcie_write_reg(dev->phy, PCIE_COM_DEBUG_BUS_BYTE0_INDEX,
			0x01);
		msm_pcie_write_reg(dev->phy, PCIE_COM_DEBUG_BUS_BYTE1_INDEX,
			0x02);
		msm_pcie_write_reg(dev->phy, PCIE_COM_DEBUG_BUS_BYTE2_INDEX,
			0x03);
		msm_pcie_write_reg(dev->phy, PCIE_COM_DEBUG_BUS_BYTE3_INDEX,
			0x04);

		PCIE_DUMP(dev,
			"PCIe: RC%d to PCIE_COM_DEBUG_BUS_BYTE0_INDEX: 0x%x\n",
			dev->rc_idx,
			readl_relaxed(dev->phy +
				PCIE_COM_DEBUG_BUS_BYTE0_INDEX));
		PCIE_DUMP(dev,
			"PCIe: RC%d to PCIE_COM_DEBUG_BUS_BYTE1_INDEX: 0x%x\n",
			dev->rc_idx,
			readl_relaxed(dev->phy +
				PCIE_COM_DEBUG_BUS_BYTE1_INDEX));
		PCIE_DUMP(dev,
			"PCIe: RC%d to PCIE_COM_DEBUG_BUS_BYTE2_INDEX: 0x%x\n",
			dev->rc_idx,
			readl_relaxed(dev->phy +
				PCIE_COM_DEBUG_BUS_BYTE2_INDEX));
		PCIE_DUMP(dev,
			"PCIe: RC%d to PCIE_COM_DEBUG_BUS_BYTE3_INDEX: 0x%x\n",
			dev->rc_idx,
			readl_relaxed(dev->phy +
				PCIE_COM_DEBUG_BUS_BYTE3_INDEX));
		PCIE_DUMP(dev,
			"PCIe: RC%d PCIE_COM_DEBUG_BUS_0_STATUS: 0x%x\n",
			dev->rc_idx,
			readl_relaxed(dev->phy +
				PCIE_COM_DEBUG_BUS_0_STATUS));
		PCIE_DUMP(dev,
			"PCIe: RC%d PCIE_COM_DEBUG_BUS_1_STATUS: 0x%x\n",
			dev->rc_idx,
			readl_relaxed(dev->phy +
				PCIE_COM_DEBUG_BUS_1_STATUS));
		PCIE_DUMP(dev,
			"PCIe: RC%d PCIE_COM_DEBUG_BUS_2_STATUS: 0x%x\n",
			dev->rc_idx,
			readl_relaxed(dev->phy +
				PCIE_COM_DEBUG_BUS_2_STATUS));
		PCIE_DUMP(dev,
			"PCIe: RC%d PCIE_COM_DEBUG_BUS_3_STATUS: 0x%x\n",
			dev->rc_idx,
			readl_relaxed(dev->phy +
				PCIE_COM_DEBUG_BUS_3_STATUS));

		msm_pcie_write_reg(dev->phy, PCIE_COM_DEBUG_BUS_BYTE0_INDEX,
			0x05);

		PCIE_DUMP(dev,
			"PCIe: RC%d to PCIE_COM_DEBUG_BUS_BYTE0_INDEX: 0x%x\n",
			dev->rc_idx,
			readl_relaxed(dev->phy +
				PCIE_COM_DEBUG_BUS_BYTE0_INDEX));
		PCIE_DUMP(dev,
			"PCIe: RC%d PCIE_COM_DEBUG_BUS_0_STATUS: 0x%x\n\n",
			dev->rc_idx,
			readl_relaxed(dev->phy +
				PCIE_COM_DEBUG_BUS_0_STATUS));
	}

	size = resource_size(dev->res[MSM_PCIE_RES_PHY].resource);
	for (i = 0; i < size; i += 32) {
		PCIE_DUMP(dev,
			"PCIe PHY of RC%d: 0x%04x %08x %08x %08x %08x %08x %08x %08x %08x\n",
			dev->rc_idx, i,
			readl_relaxed(dev->phy + i),
			readl_relaxed(dev->phy + (i + 4)),
			readl_relaxed(dev->phy + (i + 8)),
			readl_relaxed(dev->phy + (i + 12)),
			readl_relaxed(dev->phy + (i + 16)),
			readl_relaxed(dev->phy + (i + 20)),
			readl_relaxed(dev->phy + (i + 24)),
			readl_relaxed(dev->phy + (i + 28)));
	}
}

#ifdef CONFIG_ARCH_MDMCALIFORNIUM
static void pcie_phy_init(struct msm_pcie_dev_t *dev)
{
	u8 common_phy;

	PCIE_DBG(dev,
		"RC%d: Initializing MDM 14nm QMP phy - 19.2MHz with Common Mode Clock (SSC ON)\n",
		dev->rc_idx);

	if (dev->common_phy)
		common_phy = 1;
	else
		common_phy = 0;

	msm_pcie_write_reg(dev->phy,
		PCIE_N_SW_RESET(dev->rc_idx, common_phy),
		0x01);
	msm_pcie_write_reg(dev->phy,
		PCIE_N_POWER_DOWN_CONTROL(dev->rc_idx, common_phy),
		0x03);

	msm_pcie_write_reg(dev->phy, QSERDES_COM_BIAS_EN_CLKBUFLR_EN, 0x18);
	msm_pcie_write_reg(dev->phy, QSERDES_COM_CLK_ENABLE1, 0x10);

	msm_pcie_write_reg(dev->phy,
			QSERDES_TX_N_LANE_MODE(dev->rc_idx, common_phy), 0x06);

	msm_pcie_write_reg(dev->phy, QSERDES_COM_LOCK_CMP_EN, 0x01);
	msm_pcie_write_reg(dev->phy, QSERDES_COM_VCO_TUNE_MAP, 0x00);
	msm_pcie_write_reg(dev->phy, QSERDES_COM_VCO_TUNE_TIMER1, 0xFF);
	msm_pcie_write_reg(dev->phy, QSERDES_COM_VCO_TUNE_TIMER2, 0x1F);
	msm_pcie_write_reg(dev->phy, QSERDES_COM_BG_TRIM, 0x0F);
	msm_pcie_write_reg(dev->phy, QSERDES_COM_PLL_IVCO, 0x0F);
	msm_pcie_write_reg(dev->phy, QSERDES_COM_HSCLK_SEL, 0x00);
	msm_pcie_write_reg(dev->phy, QSERDES_COM_SVS_MODE_CLK_SEL, 0x01);
	msm_pcie_write_reg(dev->phy, QSERDES_COM_CORE_CLK_EN, 0x20);
	msm_pcie_write_reg(dev->phy, QSERDES_COM_CORECLK_DIV, 0x0A);
	msm_pcie_write_reg(dev->phy, QSERDES_COM_BG_TIMER, 0x09);

	if (dev->tcsr) {
		PCIE_DBG(dev, "RC%d: TCSR PHY clock scheme is 0x%x\n",
			dev->rc_idx, readl_relaxed(dev->tcsr));

		if (readl_relaxed(dev->tcsr) & (BIT(1) | BIT(0)))
			msm_pcie_write_reg(dev->phy,
					QSERDES_COM_SYSCLK_EN_SEL, 0x0A);
		else
			msm_pcie_write_reg(dev->phy,
					QSERDES_COM_SYSCLK_EN_SEL, 0x04);
	}

	msm_pcie_write_reg(dev->phy, QSERDES_COM_DEC_START_MODE0, 0x82);
	msm_pcie_write_reg(dev->phy, QSERDES_COM_DIV_FRAC_START3_MODE0, 0x03);
	msm_pcie_write_reg(dev->phy, QSERDES_COM_DIV_FRAC_START2_MODE0, 0x55);
	msm_pcie_write_reg(dev->phy, QSERDES_COM_DIV_FRAC_START1_MODE0, 0x55);
	msm_pcie_write_reg(dev->phy, QSERDES_COM_LOCK_CMP3_MODE0, 0x00);
	msm_pcie_write_reg(dev->phy, QSERDES_COM_LOCK_CMP2_MODE0, 0x0D);
	msm_pcie_write_reg(dev->phy, QSERDES_COM_LOCK_CMP1_MODE0, 0x04);
	msm_pcie_write_reg(dev->phy, QSERDES_COM_CLK_SELECT, 0x33);
	msm_pcie_write_reg(dev->phy, QSERDES_COM_SYS_CLK_CTRL, 0x02);
	msm_pcie_write_reg(dev->phy, QSERDES_COM_SYSCLK_BUF_ENABLE, 0x1F);
	msm_pcie_write_reg(dev->phy, QSERDES_COM_CP_CTRL_MODE0, 0x0B);
	msm_pcie_write_reg(dev->phy, QSERDES_COM_PLL_RCTRL_MODE0, 0x16);
	msm_pcie_write_reg(dev->phy, QSERDES_COM_PLL_CCTRL_MODE0, 0x28);
	msm_pcie_write_reg(dev->phy, QSERDES_COM_INTEGLOOP_GAIN1_MODE0, 0x00);
	msm_pcie_write_reg(dev->phy, QSERDES_COM_INTEGLOOP_GAIN0_MODE0, 0x80);

	msm_pcie_write_reg(dev->phy, QSERDES_COM_SSC_EN_CENTER, 0x01);
	msm_pcie_write_reg(dev->phy, QSERDES_COM_SSC_PER1, 0x31);
	msm_pcie_write_reg(dev->phy, QSERDES_COM_SSC_PER2, 0x01);
	msm_pcie_write_reg(dev->phy, QSERDES_COM_SSC_ADJ_PER1, 0x02);
	msm_pcie_write_reg(dev->phy, QSERDES_COM_SSC_ADJ_PER2, 0x00);
	msm_pcie_write_reg(dev->phy, QSERDES_COM_SSC_STEP_SIZE1, 0x2f);
	msm_pcie_write_reg(dev->phy, QSERDES_COM_SSC_STEP_SIZE2, 0x19);

	msm_pcie_write_reg(dev->phy,
		QSERDES_TX_N_HIGHZ_TRANSCEIVEREN_BIAS_DRVR_EN(dev->rc_idx,
		common_phy), 0x45);

	msm_pcie_write_reg(dev->phy, QSERDES_COM_CMN_CONFIG, 0x06);

	msm_pcie_write_reg(dev->phy,
		QSERDES_TX_N_RES_CODE_LANE_OFFSET(dev->rc_idx, common_phy),
		0x02);
	msm_pcie_write_reg(dev->phy,
		QSERDES_TX_N_RCV_DETECT_LVL_2(dev->rc_idx, common_phy),
		0x12);

	msm_pcie_write_reg(dev->phy,
		QSERDES_RX_N_SIGDET_ENABLES(dev->rc_idx, common_phy),
		0x1C);
	msm_pcie_write_reg(dev->phy,
		QSERDES_RX_N_SIGDET_DEGLITCH_CNTRL(dev->rc_idx, common_phy),
		0x14);
	msm_pcie_write_reg(dev->phy,
		QSERDES_RX_N_RX_EQU_ADAPTOR_CNTRL2(dev->rc_idx, common_phy),
		0x01);
	msm_pcie_write_reg(dev->phy,
		QSERDES_RX_N_RX_EQU_ADAPTOR_CNTRL3(dev->rc_idx, common_phy),
		0x00);
	msm_pcie_write_reg(dev->phy,
		QSERDES_RX_N_RX_EQU_ADAPTOR_CNTRL4(dev->rc_idx, common_phy),
		0xDB);
	msm_pcie_write_reg(dev->phy,
		QSERDES_RX_N_UCDR_SO_SATURATION_AND_ENABLE(dev->rc_idx,
		common_phy),
		0x4B);
	msm_pcie_write_reg(dev->phy,
		QSERDES_RX_N_UCDR_SO_GAIN(dev->rc_idx, common_phy),
		0x04);
	msm_pcie_write_reg(dev->phy,
		QSERDES_RX_N_UCDR_SO_GAIN_HALF(dev->rc_idx, common_phy),
		0x04);

	msm_pcie_write_reg(dev->phy, QSERDES_COM_CLK_EP_DIV, 0x19);

	msm_pcie_write_reg(dev->phy,
		PCIE_N_ENDPOINT_REFCLK_DRIVE(dev->rc_idx, common_phy),
		0x04);
	msm_pcie_write_reg(dev->phy,
		PCIE_N_OSC_DTCT_ACTIONS(dev->rc_idx, common_phy),
		0x00);
	msm_pcie_write_reg(dev->phy,
		PCIE_N_PWRUP_RESET_DLY_TIME_AUXCLK(dev->rc_idx, common_phy),
		0x40);
	msm_pcie_write_reg(dev->phy,
		PCIE_N_L1SS_WAKEUP_DLY_TIME_AUXCLK_MSB(dev->rc_idx, common_phy),
		0x00);
	msm_pcie_write_reg(dev->phy,
		PCIE_N_L1SS_WAKEUP_DLY_TIME_AUXCLK_LSB(dev->rc_idx, common_phy),
		0x40);
	msm_pcie_write_reg(dev->phy,
		PCIE_N_LP_WAKEUP_DLY_TIME_AUXCLK_MSB(dev->rc_idx, common_phy),
		0x00);
	msm_pcie_write_reg(dev->phy,
		PCIE_N_LP_WAKEUP_DLY_TIME_AUXCLK(dev->rc_idx, common_phy),
		0x40);
	msm_pcie_write_reg(dev->phy,
		PCIE_N_PLL_LOCK_CHK_DLY_TIME(dev->rc_idx, common_phy),
		0x73);
	msm_pcie_write_reg(dev->phy,
		QSERDES_RX_N_SIGDET_LVL(dev->rc_idx, common_phy),
		0x99);
	msm_pcie_write_reg(dev->phy,
		PCIE_N_TXDEEMPH_M6DB_V0(dev->rc_idx, common_phy),
		0x15);
	msm_pcie_write_reg(dev->phy,
		PCIE_N_TXDEEMPH_M3P5DB_V0(dev->rc_idx, common_phy),
		0x0E);

	msm_pcie_write_reg(dev->phy,
		PCIE_N_SIGDET_CNTRL(dev->rc_idx, common_phy),
		0x07);

	msm_pcie_write_reg(dev->phy,
		PCIE_N_SW_RESET(dev->rc_idx, common_phy),
		0x00);
	msm_pcie_write_reg(dev->phy,
		PCIE_N_START_CONTROL(dev->rc_idx, common_phy),
		0x03);
}

static void pcie_pcs_port_phy_init(struct msm_pcie_dev_t *dev)
{
}

static bool pcie_phy_is_ready(struct msm_pcie_dev_t *dev)
{
	if (readl_relaxed(dev->phy +
		PCIE_N_PCS_STATUS(dev->rc_idx, dev->common_phy)) & BIT(6))
		return false;
	else
		return true;
}
#else
static void pcie_phy_init(struct msm_pcie_dev_t *dev)
{
	int i;
	struct msm_pcie_phy_info_t *phy_seq;

	PCIE_DBG(dev,
		"RC%d: Initializing 14nm QMP phy - 19.2MHz with Common Mode Clock (SSC ON)\n",
		dev->rc_idx);

	if (dev->phy_sequence) {
		i =  dev->phy_len;
		phy_seq = dev->phy_sequence;
		while (i--) {
			msm_pcie_write_reg(dev->phy,
				phy_seq->offset,
				phy_seq->val);
			if (phy_seq->delay)
				usleep_range(phy_seq->delay,
					phy_seq->delay + 1);
			phy_seq++;
		}
		return;
	}

	if (dev->common_phy)
		msm_pcie_write_reg(dev->phy, PCIE_COM_POWER_DOWN_CONTROL, 0x01);

	msm_pcie_write_reg(dev->phy, QSERDES_COM_BIAS_EN_CLKBUFLR_EN, 0x1C);
	msm_pcie_write_reg(dev->phy, QSERDES_COM_CLK_ENABLE1, 0x10);
	msm_pcie_write_reg(dev->phy, QSERDES_COM_CLK_SELECT, 0x33);
	msm_pcie_write_reg(dev->phy, QSERDES_COM_CMN_CONFIG, 0x06);
	msm_pcie_write_reg(dev->phy, QSERDES_COM_LOCK_CMP_EN, 0x42);
	msm_pcie_write_reg(dev->phy, QSERDES_COM_VCO_TUNE_MAP, 0x00);
	msm_pcie_write_reg(dev->phy, QSERDES_COM_VCO_TUNE_TIMER1, 0xFF);
	msm_pcie_write_reg(dev->phy, QSERDES_COM_VCO_TUNE_TIMER2, 0x1F);
	msm_pcie_write_reg(dev->phy, QSERDES_COM_HSCLK_SEL, 0x01);
	msm_pcie_write_reg(dev->phy, QSERDES_COM_SVS_MODE_CLK_SEL, 0x01);
	msm_pcie_write_reg(dev->phy, QSERDES_COM_CORE_CLK_EN, 0x00);
	msm_pcie_write_reg(dev->phy, QSERDES_COM_CORECLK_DIV, 0x0A);
	msm_pcie_write_reg(dev->phy, QSERDES_COM_BG_TIMER, 0x09);
	msm_pcie_write_reg(dev->phy, QSERDES_COM_DEC_START_MODE0, 0x82);
	msm_pcie_write_reg(dev->phy, QSERDES_COM_DIV_FRAC_START3_MODE0, 0x03);
	msm_pcie_write_reg(dev->phy, QSERDES_COM_DIV_FRAC_START2_MODE0, 0x55);
	msm_pcie_write_reg(dev->phy, QSERDES_COM_DIV_FRAC_START1_MODE0, 0x55);
	msm_pcie_write_reg(dev->phy, QSERDES_COM_LOCK_CMP3_MODE0, 0x00);
	msm_pcie_write_reg(dev->phy, QSERDES_COM_LOCK_CMP2_MODE0, 0x1A);
	msm_pcie_write_reg(dev->phy, QSERDES_COM_LOCK_CMP1_MODE0, 0x0A);
	msm_pcie_write_reg(dev->phy, QSERDES_COM_CLK_SELECT, 0x33);
	msm_pcie_write_reg(dev->phy, QSERDES_COM_SYS_CLK_CTRL, 0x02);
	msm_pcie_write_reg(dev->phy, QSERDES_COM_SYSCLK_BUF_ENABLE, 0x1F);
	msm_pcie_write_reg(dev->phy, QSERDES_COM_SYSCLK_EN_SEL, 0x04);
	msm_pcie_write_reg(dev->phy, QSERDES_COM_CP_CTRL_MODE0, 0x0B);
	msm_pcie_write_reg(dev->phy, QSERDES_COM_PLL_RCTRL_MODE0, 0x16);
	msm_pcie_write_reg(dev->phy, QSERDES_COM_PLL_CCTRL_MODE0, 0x28);
	msm_pcie_write_reg(dev->phy, QSERDES_COM_INTEGLOOP_GAIN1_MODE0, 0x00);
	msm_pcie_write_reg(dev->phy, QSERDES_COM_INTEGLOOP_GAIN0_MODE0, 0x80);
	msm_pcie_write_reg(dev->phy, QSERDES_COM_SSC_EN_CENTER, 0x01);
	msm_pcie_write_reg(dev->phy, QSERDES_COM_SSC_PER1, 0x31);
	msm_pcie_write_reg(dev->phy, QSERDES_COM_SSC_PER2, 0x01);
	msm_pcie_write_reg(dev->phy, QSERDES_COM_SSC_ADJ_PER1, 0x02);
	msm_pcie_write_reg(dev->phy, QSERDES_COM_SSC_ADJ_PER2, 0x00);
	msm_pcie_write_reg(dev->phy, QSERDES_COM_SSC_STEP_SIZE1, 0x2f);
	msm_pcie_write_reg(dev->phy, QSERDES_COM_SSC_STEP_SIZE2, 0x19);

	msm_pcie_write_reg(dev->phy, QSERDES_COM_RESCODE_DIV_NUM, 0x15);
	msm_pcie_write_reg(dev->phy, QSERDES_COM_BG_TRIM, 0x0F);
	msm_pcie_write_reg(dev->phy, QSERDES_COM_PLL_IVCO, 0x0F);

	msm_pcie_write_reg(dev->phy, QSERDES_COM_CLK_EP_DIV, 0x19);
	msm_pcie_write_reg(dev->phy, QSERDES_COM_CLK_ENABLE1, 0x10);

	if (dev->phy_ver == 0x3) {
		msm_pcie_write_reg(dev->phy, QSERDES_COM_HSCLK_SEL, 0x00);
		msm_pcie_write_reg(dev->phy, QSERDES_COM_RESCODE_DIV_NUM, 0x40);
	}

	if (dev->common_phy) {
		msm_pcie_write_reg(dev->phy, PCIE_COM_SW_RESET, 0x00);
		msm_pcie_write_reg(dev->phy, PCIE_COM_START_CONTROL, 0x03);
	}
}

static void pcie_pcs_port_phy_init(struct msm_pcie_dev_t *dev)
{
	int i;
	struct msm_pcie_phy_info_t *phy_seq;
	u8 common_phy;

	if (dev->phy_ver == 0x90)
		return;

	PCIE_DBG(dev, "RC%d: Initializing PCIe PHY Port\n", dev->rc_idx);

	if (dev->common_phy)
		common_phy = 1;
	else
		common_phy = 0;

	if (dev->port_phy_sequence) {
		i =  dev->port_phy_len;
		phy_seq = dev->port_phy_sequence;
		while (i--) {
			msm_pcie_write_reg(dev->phy,
				phy_seq->offset,
				phy_seq->val);
			if (phy_seq->delay)
				usleep_range(phy_seq->delay,
					phy_seq->delay + 1);
			phy_seq++;
		}
		return;
	}

	msm_pcie_write_reg(dev->phy,
		QSERDES_TX_N_HIGHZ_TRANSCEIVEREN_BIAS_DRVR_EN(dev->rc_idx,
		common_phy), 0x45);
	msm_pcie_write_reg(dev->phy,
		QSERDES_TX_N_LANE_MODE(dev->rc_idx, common_phy),
		0x06);

	msm_pcie_write_reg(dev->phy,
		QSERDES_RX_N_SIGDET_ENABLES(dev->rc_idx, common_phy),
		0x1C);
	msm_pcie_write_reg(dev->phy,
		QSERDES_RX_N_SIGDET_LVL(dev->rc_idx, common_phy),
		0x17);
	msm_pcie_write_reg(dev->phy,
		QSERDES_RX_N_RX_EQU_ADAPTOR_CNTRL2(dev->rc_idx, common_phy),
		0x01);
	msm_pcie_write_reg(dev->phy,
		QSERDES_RX_N_RX_EQU_ADAPTOR_CNTRL3(dev->rc_idx, common_phy),
		0x00);
	msm_pcie_write_reg(dev->phy,
		QSERDES_RX_N_RX_EQU_ADAPTOR_CNTRL4(dev->rc_idx, common_phy),
		0xDB);
	msm_pcie_write_reg(dev->phy,
		QSERDES_RX_N_RX_BAND(dev->rc_idx, common_phy),
		0x18);
	msm_pcie_write_reg(dev->phy,
		QSERDES_RX_N_UCDR_SO_GAIN(dev->rc_idx, common_phy),
		0x04);
	msm_pcie_write_reg(dev->phy,
		QSERDES_RX_N_UCDR_SO_GAIN_HALF(dev->rc_idx, common_phy),
		0x04);
	msm_pcie_write_reg(dev->phy,
		PCIE_N_RX_IDLE_DTCT_CNTRL(dev->rc_idx, common_phy),
		0x4C);
	msm_pcie_write_reg(dev->phy,
		PCIE_N_PWRUP_RESET_DLY_TIME_AUXCLK(dev->rc_idx, common_phy),
		0x00);
	msm_pcie_write_reg(dev->phy,
		PCIE_N_LP_WAKEUP_DLY_TIME_AUXCLK(dev->rc_idx, common_phy),
		0x01);
	msm_pcie_write_reg(dev->phy,
		PCIE_N_PLL_LOCK_CHK_DLY_TIME(dev->rc_idx, common_phy),
		0x05);
	msm_pcie_write_reg(dev->phy,
		QSERDES_RX_N_UCDR_SO_SATURATION_AND_ENABLE(dev->rc_idx,
		common_phy), 0x4B);
	msm_pcie_write_reg(dev->phy,
		QSERDES_RX_N_SIGDET_DEGLITCH_CNTRL(dev->rc_idx, common_phy),
		0x14);

	msm_pcie_write_reg(dev->phy,
		PCIE_N_ENDPOINT_REFCLK_DRIVE(dev->rc_idx, common_phy),
		0x05);
	msm_pcie_write_reg(dev->phy,
		PCIE_N_POWER_DOWN_CONTROL(dev->rc_idx, common_phy),
		0x02);
	msm_pcie_write_reg(dev->phy,
		PCIE_N_POWER_STATE_CONFIG4(dev->rc_idx, common_phy),
		0x00);
	msm_pcie_write_reg(dev->phy,
		PCIE_N_POWER_STATE_CONFIG1(dev->rc_idx, common_phy),
		0xA3);

	if (dev->phy_ver == 0x3) {
		msm_pcie_write_reg(dev->phy,
			QSERDES_RX_N_SIGDET_LVL(dev->rc_idx, common_phy),
			0x19);

		msm_pcie_write_reg(dev->phy,
			PCIE_N_TXDEEMPH_M3P5DB_V0(dev->rc_idx, common_phy),
			0x0E);
	}

	msm_pcie_write_reg(dev->phy,
		PCIE_N_POWER_DOWN_CONTROL(dev->rc_idx, common_phy),
		0x03);
	usleep_range(POWER_DOWN_DELAY_US_MIN, POWER_DOWN_DELAY_US_MAX);

	msm_pcie_write_reg(dev->phy,
		PCIE_N_SW_RESET(dev->rc_idx, common_phy),
		0x00);
	msm_pcie_write_reg(dev->phy,
		PCIE_N_START_CONTROL(dev->rc_idx, common_phy),
		0x0A);
}

static bool pcie_phy_is_ready(struct msm_pcie_dev_t *dev)
{
	if (dev->phy_ver == 0x90) {
		if (readl_relaxed(dev->phy +
			PCIE_N_PCS_STATUS(dev->rc_idx, dev->common_phy)) &
			BIT(6))
			return false;
		else
			return true;
	}

	if (!(readl_relaxed(dev->phy + PCIE_COM_PCS_READY_STATUS) & 0x1))
		return false;
	else
		return true;
}
#endif
#endif

static int msm_pcie_restore_sec_config(struct msm_pcie_dev_t *dev)
{
	int ret, scm_ret;

	if (!dev) {
		pr_err("PCIe: the input pcie dev is NULL.\n");
		return -ENODEV;
	}

	ret = scm_restore_sec_cfg(dev->scm_dev_id, 0, &scm_ret);
	if (ret || scm_ret) {
		PCIE_ERR(dev,
			"PCIe: RC%d failed(%d) to restore sec config, scm_ret=%d\n",
			dev->rc_idx, ret, scm_ret);
		return ret ? ret : -EINVAL;
	}

	return 0;
}

static inline int msm_pcie_check_align(struct msm_pcie_dev_t *dev,
						u32 offset)
{
	if (offset % 4) {
		PCIE_ERR(dev,
			"PCIe: RC%d: offset 0x%x is not correctly aligned\n",
			dev->rc_idx, offset);
		return MSM_PCIE_ERROR;
	}

	return 0;
}

static bool msm_pcie_confirm_linkup(struct msm_pcie_dev_t *dev,
						bool check_sw_stts,
						bool check_ep)
{
	u32 val;

	if (check_sw_stts && (dev->link_status != MSM_PCIE_LINK_ENABLED)) {
		PCIE_DBG(dev, "PCIe: The link of RC %d is not enabled.\n",
			dev->rc_idx);
		return false;
	}

	if (!(readl_relaxed(dev->dm_core + 0x80) & BIT(29))) {
		PCIE_DBG(dev, "PCIe: The link of RC %d is not up.\n",
			dev->rc_idx);
		return false;
	}

	val = readl_relaxed(dev->dm_core);
	PCIE_DBG(dev, "PCIe: device ID and vender ID of RC %d are 0x%x.\n",
		dev->rc_idx, val);
	if (val == PCIE_LINK_DOWN) {
		PCIE_ERR(dev,
			"PCIe: The link of RC %d is not really up; device ID and vender ID of RC %d are 0x%x.\n",
			dev->rc_idx, dev->rc_idx, val);
		return false;
	}

	if (check_ep) {
		val = readl_relaxed(dev->conf);
		PCIE_DBG(dev,
			"PCIe: device ID and vender ID of EP of RC %d are 0x%x.\n",
			dev->rc_idx, val);
		if (val == PCIE_LINK_DOWN) {
			PCIE_ERR(dev,
				"PCIe: The link of RC %d is not really up; device ID and vender ID of EP of RC %d are 0x%x.\n",
				dev->rc_idx, dev->rc_idx, val);
			return false;
		}
	}

	return true;
}

static void msm_pcie_cfg_recover(struct msm_pcie_dev_t *dev, bool rc)
{
	int i, j;
	u32 val = 0;
	u32 *shadow;
	void *cfg = dev->conf;

	for (i = 0; i < MAX_DEVICE_NUM; i++) {
		if (!rc && !dev->pcidev_table[i].bdf)
			break;
		if (rc) {
			cfg = dev->dm_core;
			shadow = dev->rc_shadow;
		} else {
			shadow = dev->ep_shadow[i];
			PCIE_DBG(dev,
				"PCIe Device: %02x:%02x.%01x\n",
				dev->pcidev_table[i].bdf >> 24,
				dev->pcidev_table[i].bdf >> 19 & 0x1f,
				dev->pcidev_table[i].bdf >> 16 & 0x07);
		}
		for (j = PCIE_CONF_SPACE_DW - 1; j >= 0; j--) {
			val = shadow[j];
			if (val != PCIE_CLEAR) {
				PCIE_DBG3(dev,
					"PCIe: before recovery:cfg 0x%x:0x%x\n",
					j * 4, readl_relaxed(cfg + j * 4));
				PCIE_DBG3(dev,
					"PCIe: shadow_dw[%d]:cfg 0x%x:0x%x\n",
					j, j * 4, val);
				writel_relaxed(val, cfg + j * 4);
				wmb();
				PCIE_DBG3(dev,
					"PCIe: after recovery:cfg 0x%x:0x%x\n\n",
					j * 4, readl_relaxed(cfg + j * 4));
			}
		}
		if (rc)
			break;

		pci_save_state(dev->pcidev_table[i].dev);
		cfg += SZ_4K;
	}
}

static void msm_pcie_write_mask(void __iomem *addr,
				uint32_t clear_mask, uint32_t set_mask)
{
	uint32_t val;

	val = (readl_relaxed(addr) & ~clear_mask) | set_mask;
	writel_relaxed(val, addr);
	wmb();  /* ensure data is written to hardware register */
}

static void pcie_parf_dump(struct msm_pcie_dev_t *dev)
{
	int i, size;
	u32 original;

	PCIE_DUMP(dev, "PCIe: RC%d PARF testbus\n", dev->rc_idx);

	original = readl_relaxed(dev->parf + PCIE20_PARF_SYS_CTRL);
	for (i = 1; i <= 0x1A; i++) {
		msm_pcie_write_mask(dev->parf + PCIE20_PARF_SYS_CTRL,
				0xFF0000, i << 16);
		PCIE_DUMP(dev,
			"RC%d: PARF_SYS_CTRL: 0%08x PARF_TEST_BUS: 0%08x\n",
			dev->rc_idx,
			readl_relaxed(dev->parf + PCIE20_PARF_SYS_CTRL),
			readl_relaxed(dev->parf + PCIE20_PARF_TEST_BUS));
	}
	writel_relaxed(original, dev->parf + PCIE20_PARF_SYS_CTRL);

	PCIE_DUMP(dev, "PCIe: RC%d PARF register dump\n", dev->rc_idx);

	size = resource_size(dev->res[MSM_PCIE_RES_PARF].resource);
	for (i = 0; i < size; i += 32) {
		PCIE_DUMP(dev,
			"RC%d: 0x%04x %08x %08x %08x %08x %08x %08x %08x %08x\n",
			dev->rc_idx, i,
			readl_relaxed(dev->parf + i),
			readl_relaxed(dev->parf + (i + 4)),
			readl_relaxed(dev->parf + (i + 8)),
			readl_relaxed(dev->parf + (i + 12)),
			readl_relaxed(dev->parf + (i + 16)),
			readl_relaxed(dev->parf + (i + 20)),
			readl_relaxed(dev->parf + (i + 24)),
			readl_relaxed(dev->parf + (i + 28)));
	}
}

static void msm_pcie_show_status(struct msm_pcie_dev_t *dev)
{
	PCIE_DBG_FS(dev, "PCIe: RC%d is %s enumerated\n",
		dev->rc_idx, dev->enumerated ? "" : "not");
	PCIE_DBG_FS(dev, "PCIe: link is %s\n",
		(dev->link_status == MSM_PCIE_LINK_ENABLED)
		? "enabled" : "disabled");
	PCIE_DBG_FS(dev, "cfg_access is %s allowed\n",
		dev->cfg_access ? "" : "not");
	PCIE_DBG_FS(dev, "use_msi is %d\n",
		dev->use_msi);
	PCIE_DBG_FS(dev, "use_pinctrl is %d\n",
		dev->use_pinctrl);
	PCIE_DBG_FS(dev, "use_19p2mhz_aux_clk is %d\n",
		dev->use_19p2mhz_aux_clk);
	PCIE_DBG_FS(dev, "user_suspend is %d\n",
		dev->user_suspend);
	PCIE_DBG_FS(dev, "num_ep: %d\n",
		dev->num_ep);
	PCIE_DBG_FS(dev, "num_active_ep: %d\n",
		dev->num_active_ep);
	PCIE_DBG_FS(dev, "pending_ep_reg: %s\n",
		dev->pending_ep_reg ? "true" : "false");
	PCIE_DBG_FS(dev, "phy_len is %d",
		dev->phy_len);
	PCIE_DBG_FS(dev, "port_phy_len is %d",
		dev->port_phy_len);
	PCIE_DBG_FS(dev, "disable_pc is %d",
		dev->disable_pc);
	PCIE_DBG_FS(dev, "l0s_supported is %s supported\n",
		dev->l0s_supported ? "" : "not");
	PCIE_DBG_FS(dev, "l1_supported is %s supported\n",
		dev->l1_supported ? "" : "not");
	PCIE_DBG_FS(dev, "l1ss_supported is %s supported\n",
		dev->l1ss_supported ? "" : "not");
	PCIE_DBG_FS(dev, "common_clk_en is %d\n",
		dev->common_clk_en);
	PCIE_DBG_FS(dev, "clk_power_manage_en is %d\n",
		dev->clk_power_manage_en);
	PCIE_DBG_FS(dev, "aux_clk_sync is %d\n",
		dev->aux_clk_sync);
	PCIE_DBG_FS(dev, "AER is %s enable\n",
		dev->aer_enable ? "" : "not");
	PCIE_DBG_FS(dev, "ext_ref_clk is %d\n",
		dev->ext_ref_clk);
	PCIE_DBG_FS(dev, "ep_wakeirq is %d\n",
		dev->ep_wakeirq);
	PCIE_DBG_FS(dev, "phy_ver is %d\n",
		dev->phy_ver);
	PCIE_DBG_FS(dev, "drv_ready is %d\n",
		dev->drv_ready);
	PCIE_DBG_FS(dev, "linkdown_panic is %d\n",
		dev->linkdown_panic);
	PCIE_DBG_FS(dev, "the link is %s suspending\n",
		dev->suspending ? "" : "not");
	PCIE_DBG_FS(dev, "shadow is %s enabled\n",
		dev->shadow_en ? "" : "not");
	PCIE_DBG_FS(dev, "the power of RC is %s on\n",
		dev->power_on ? "" : "not");
	PCIE_DBG_FS(dev, "msi_gicm_addr: 0x%x\n",
		dev->msi_gicm_addr);
	PCIE_DBG_FS(dev, "msi_gicm_base: 0x%x\n",
		dev->msi_gicm_base);
	PCIE_DBG_FS(dev, "bus_client: %d\n",
		dev->bus_client);
	PCIE_DBG_FS(dev, "current short bdf: %d\n",
		dev->current_short_bdf);
	PCIE_DBG_FS(dev, "smmu does %s exist\n",
		dev->smmu_exist ? "" : "not");
	PCIE_DBG_FS(dev, "n_fts: %d\n",
		dev->n_fts);
	PCIE_DBG_FS(dev, "common_phy: %d\n",
		dev->common_phy);
	PCIE_DBG_FS(dev, "ep_latency: %dms\n",
		dev->ep_latency);
	PCIE_DBG_FS(dev, "cpl_timeout: 0x%x\n",
		dev->cpl_timeout);
	PCIE_DBG_FS(dev, "current_bdf: 0x%x\n",
		dev->current_bdf);
	PCIE_DBG_FS(dev, "perst_delay_us_min: %dus\n",
		dev->perst_delay_us_min);
	PCIE_DBG_FS(dev, "perst_delay_us_max: %dus\n",
		dev->perst_delay_us_max);
	PCIE_DBG_FS(dev, "tlp_rd_size: 0x%x\n",
		dev->tlp_rd_size);
	PCIE_DBG_FS(dev, "rc_corr_counter: %lu\n",
		dev->rc_corr_counter);
	PCIE_DBG_FS(dev, "rc_non_fatal_counter: %lu\n",
		dev->rc_non_fatal_counter);
	PCIE_DBG_FS(dev, "rc_fatal_counter: %lu\n",
		dev->rc_fatal_counter);
	PCIE_DBG_FS(dev, "ep_corr_counter: %lu\n",
		dev->ep_corr_counter);
	PCIE_DBG_FS(dev, "ep_non_fatal_counter: %lu\n",
		dev->ep_non_fatal_counter);
	PCIE_DBG_FS(dev, "ep_fatal_counter: %lu\n",
		dev->ep_fatal_counter);
	PCIE_DBG_FS(dev, "linkdown_counter: %lu\n",
		dev->linkdown_counter);
	PCIE_DBG_FS(dev, "wake_counter: %lu\n",
		dev->wake_counter);
	PCIE_DBG_FS(dev, "link_turned_on_counter: %lu\n",
		dev->link_turned_on_counter);
	PCIE_DBG_FS(dev, "link_turned_off_counter: %lu\n",
		dev->link_turned_off_counter);
}

static void msm_pcie_shadow_dump(struct msm_pcie_dev_t *dev, bool rc)
{
	int i, j;
	u32 val = 0;
	u32 *shadow;

	for (i = 0; i < MAX_DEVICE_NUM; i++) {
		if (!rc && !dev->pcidev_table[i].bdf)
			break;
		if (rc) {
			shadow = dev->rc_shadow;
		} else {
			shadow = dev->ep_shadow[i];
			PCIE_DBG_FS(dev, "PCIe Device: %02x:%02x.%01x\n",
				dev->pcidev_table[i].bdf >> 24,
				dev->pcidev_table[i].bdf >> 19 & 0x1f,
				dev->pcidev_table[i].bdf >> 16 & 0x07);
		}
		for (j = 0; j < PCIE_CONF_SPACE_DW; j++) {
			val = shadow[j];
			if (val != PCIE_CLEAR) {
				PCIE_DBG_FS(dev,
					"PCIe: shadow_dw[%d]:cfg 0x%x:0x%x\n",
					j, j * 4, val);
			}
		}
		if (rc)
			break;
	}
}

static void msm_pcie_sel_debug_testcase(struct msm_pcie_dev_t *dev,
					u32 testcase)
{
	int ret, i;
	u32 base_sel_size = 0;
	u32 val = 0;
	u32 current_offset = 0;
	u32 ep_l1sub_ctrl1_offset = 0;
	u32 ep_l1sub_cap_reg1_offset = 0;
	u32 ep_link_ctrlstts_offset = 0;
	u32 ep_dev_ctrl2stts2_offset = 0;

	if (testcase >= 5 && testcase <= 10) {
		current_offset =
			readl_relaxed(dev->conf + PCIE_CAP_PTR_OFFSET) & 0xff;

		while (current_offset) {
			val = readl_relaxed(dev->conf + current_offset);
			if ((val & 0xff) == PCIE20_CAP_ID) {
				ep_link_ctrlstts_offset = current_offset +
								0x10;
				ep_dev_ctrl2stts2_offset = current_offset +
								0x28;
				break;
			}
			current_offset = (val >> 8) & 0xff;
		}

		if (!ep_link_ctrlstts_offset)
			PCIE_DBG(dev,
				"RC%d endpoint does not support PCIe capability registers\n",
				dev->rc_idx);
		else
			PCIE_DBG(dev,
				"RC%d: ep_link_ctrlstts_offset: 0x%x\n",
				dev->rc_idx, ep_link_ctrlstts_offset);
	}

	switch (testcase) {
	case 0: /* output status */
		PCIE_DBG_FS(dev, "\n\nPCIe: Status for RC%d:\n",
			dev->rc_idx);
		msm_pcie_show_status(dev);
		break;
	case 1: /* disable link */
		PCIE_DBG_FS(dev,
			"\n\nPCIe: RC%d: disable link\n\n", dev->rc_idx);
		ret = msm_pcie_pm_control(MSM_PCIE_SUSPEND, 0,
			dev->dev, NULL,
			MSM_PCIE_CONFIG_NO_CFG_RESTORE);
		if (ret)
			PCIE_DBG_FS(dev, "PCIe:%s:failed to disable link\n",
				__func__);
		else
			PCIE_DBG_FS(dev, "PCIe:%s:disabled link\n",
				__func__);
		break;
	case 2: /* enable link and recover config space for RC and EP */
		PCIE_DBG_FS(dev,
			"\n\nPCIe: RC%d: enable link and recover config space\n\n",
			dev->rc_idx);
		ret = msm_pcie_pm_control(MSM_PCIE_RESUME, 0,
			dev->dev, NULL,
			MSM_PCIE_CONFIG_NO_CFG_RESTORE);
		if (ret)
			PCIE_DBG_FS(dev, "PCIe:%s:failed to enable link\n",
				__func__);
		else {
			PCIE_DBG_FS(dev, "PCIe:%s:enabled link\n", __func__);
			msm_pcie_recover_config(dev->dev);
		}
		break;
	case 3: /*
		 * disable and enable link, recover config space for
		 * RC and EP
		 */
		PCIE_DBG_FS(dev,
			"\n\nPCIe: RC%d: disable and enable link then recover config space\n\n",
			dev->rc_idx);
		ret = msm_pcie_pm_control(MSM_PCIE_SUSPEND, 0,
			dev->dev, NULL,
			MSM_PCIE_CONFIG_NO_CFG_RESTORE);
		if (ret)
			PCIE_DBG_FS(dev, "PCIe:%s:failed to disable link\n",
				__func__);
		else
			PCIE_DBG_FS(dev, "PCIe:%s:disabled link\n", __func__);
		ret = msm_pcie_pm_control(MSM_PCIE_RESUME, 0,
			dev->dev, NULL,
			MSM_PCIE_CONFIG_NO_CFG_RESTORE);
		if (ret)
			PCIE_DBG_FS(dev, "PCIe:%s:failed to enable link\n",
				__func__);
		else {
			PCIE_DBG_FS(dev, "PCIe:%s:enabled link\n", __func__);
			msm_pcie_recover_config(dev->dev);
		}
		break;
	case 4: /* dump shadow registers for RC and EP */
		PCIE_DBG_FS(dev,
			"\n\nPCIe: RC%d: dumping RC shadow registers\n",
			dev->rc_idx);
		msm_pcie_shadow_dump(dev, true);

		PCIE_DBG_FS(dev,
			"\n\nPCIe: RC%d: dumping EP shadow registers\n",
			dev->rc_idx);
		msm_pcie_shadow_dump(dev, false);
		break;
	case 5: /* disable L0s */
		PCIE_DBG_FS(dev, "\n\nPCIe: RC%d: disable L0s\n\n",
			dev->rc_idx);
		msm_pcie_write_mask(dev->dm_core +
				PCIE20_CAP_LINKCTRLSTATUS,
				BIT(0), 0);
		msm_pcie_write_mask(dev->conf +
				ep_link_ctrlstts_offset,
				BIT(0), 0);
		if (dev->shadow_en) {
			dev->rc_shadow[PCIE20_CAP_LINKCTRLSTATUS / 4] =
					readl_relaxed(dev->dm_core +
					PCIE20_CAP_LINKCTRLSTATUS);
			dev->ep_shadow[0][ep_link_ctrlstts_offset / 4] =
					readl_relaxed(dev->conf +
					ep_link_ctrlstts_offset);
		}
		PCIE_DBG_FS(dev, "PCIe: RC's CAP_LINKCTRLSTATUS:0x%x\n",
			readl_relaxed(dev->dm_core +
			PCIE20_CAP_LINKCTRLSTATUS));
		PCIE_DBG_FS(dev, "PCIe: EP's CAP_LINKCTRLSTATUS:0x%x\n",
			readl_relaxed(dev->conf +
			ep_link_ctrlstts_offset));
		break;
	case 6: /* enable L0s */
		PCIE_DBG_FS(dev, "\n\nPCIe: RC%d: enable L0s\n\n",
			dev->rc_idx);
		msm_pcie_write_mask(dev->dm_core +
				PCIE20_CAP_LINKCTRLSTATUS,
				0, BIT(0));
		msm_pcie_write_mask(dev->conf +
				ep_link_ctrlstts_offset,
				0, BIT(0));
		if (dev->shadow_en) {
			dev->rc_shadow[PCIE20_CAP_LINKCTRLSTATUS / 4] =
					readl_relaxed(dev->dm_core +
					PCIE20_CAP_LINKCTRLSTATUS);
			dev->ep_shadow[0][ep_link_ctrlstts_offset / 4] =
					readl_relaxed(dev->conf +
					ep_link_ctrlstts_offset);
		}
		PCIE_DBG_FS(dev, "PCIe: RC's CAP_LINKCTRLSTATUS:0x%x\n",
			readl_relaxed(dev->dm_core +
			PCIE20_CAP_LINKCTRLSTATUS));
		PCIE_DBG_FS(dev, "PCIe: EP's CAP_LINKCTRLSTATUS:0x%x\n",
			readl_relaxed(dev->conf +
			ep_link_ctrlstts_offset));
		break;
	case 7: /* disable L1 */
		PCIE_DBG_FS(dev, "\n\nPCIe: RC%d: disable L1\n\n",
			dev->rc_idx);
		msm_pcie_write_mask(dev->dm_core +
				PCIE20_CAP_LINKCTRLSTATUS,
				BIT(1), 0);
		msm_pcie_write_mask(dev->conf +
				ep_link_ctrlstts_offset,
				BIT(1), 0);
		if (dev->shadow_en) {
			dev->rc_shadow[PCIE20_CAP_LINKCTRLSTATUS / 4] =
					readl_relaxed(dev->dm_core +
					PCIE20_CAP_LINKCTRLSTATUS);
			dev->ep_shadow[0][ep_link_ctrlstts_offset / 4] =
					readl_relaxed(dev->conf +
					ep_link_ctrlstts_offset);
		}
		PCIE_DBG_FS(dev, "PCIe: RC's CAP_LINKCTRLSTATUS:0x%x\n",
			readl_relaxed(dev->dm_core +
			PCIE20_CAP_LINKCTRLSTATUS));
		PCIE_DBG_FS(dev, "PCIe: EP's CAP_LINKCTRLSTATUS:0x%x\n",
			readl_relaxed(dev->conf +
			ep_link_ctrlstts_offset));
		break;
	case 8: /* enable L1 */
		PCIE_DBG_FS(dev, "\n\nPCIe: RC%d: enable L1\n\n",
			dev->rc_idx);
		msm_pcie_write_mask(dev->dm_core +
				PCIE20_CAP_LINKCTRLSTATUS,
				0, BIT(1));
		msm_pcie_write_mask(dev->conf +
				ep_link_ctrlstts_offset,
				0, BIT(1));
		if (dev->shadow_en) {
			dev->rc_shadow[PCIE20_CAP_LINKCTRLSTATUS / 4] =
					readl_relaxed(dev->dm_core +
					PCIE20_CAP_LINKCTRLSTATUS);
			dev->ep_shadow[0][ep_link_ctrlstts_offset / 4] =
					readl_relaxed(dev->conf +
					ep_link_ctrlstts_offset);
		}
		PCIE_DBG_FS(dev, "PCIe: RC's CAP_LINKCTRLSTATUS:0x%x\n",
			readl_relaxed(dev->dm_core +
			PCIE20_CAP_LINKCTRLSTATUS));
		PCIE_DBG_FS(dev, "PCIe: EP's CAP_LINKCTRLSTATUS:0x%x\n",
			readl_relaxed(dev->conf +
			ep_link_ctrlstts_offset));
		break;
	case 9: /* disable L1ss */
		PCIE_DBG_FS(dev, "\n\nPCIe: RC%d: disable L1ss\n\n",
			dev->rc_idx);
		current_offset = PCIE_EXT_CAP_OFFSET;
		while (current_offset) {
			val = readl_relaxed(dev->conf + current_offset);
			if ((val & 0xffff) == L1SUB_CAP_ID) {
				ep_l1sub_ctrl1_offset =
						current_offset + 0x8;
				break;
			}
			current_offset = val >> 20;
		}
		if (!ep_l1sub_ctrl1_offset) {
			PCIE_DBG_FS(dev,
				"PCIe: RC%d endpoint does not support l1ss registers\n",
				dev->rc_idx);
			break;
		}

		PCIE_DBG_FS(dev, "PCIe: RC%d: ep_l1sub_ctrl1_offset: 0x%x\n",
				dev->rc_idx, ep_l1sub_ctrl1_offset);

		msm_pcie_write_reg_field(dev->dm_core,
					PCIE20_L1SUB_CONTROL1,
					0xf, 0);
		msm_pcie_write_mask(dev->dm_core +
					PCIE20_DEVICE_CONTROL2_STATUS2,
					BIT(10), 0);
		msm_pcie_write_reg_field(dev->conf,
					ep_l1sub_ctrl1_offset,
					0xf, 0);
		msm_pcie_write_mask(dev->conf +
					ep_dev_ctrl2stts2_offset,
					BIT(10), 0);
		if (dev->shadow_en) {
			dev->rc_shadow[PCIE20_L1SUB_CONTROL1 / 4] =
				readl_relaxed(dev->dm_core +
				PCIE20_L1SUB_CONTROL1);
			dev->rc_shadow[PCIE20_DEVICE_CONTROL2_STATUS2 / 4] =
				readl_relaxed(dev->dm_core +
				PCIE20_DEVICE_CONTROL2_STATUS2);
			dev->ep_shadow[0][ep_l1sub_ctrl1_offset / 4] =
				readl_relaxed(dev->conf +
				ep_l1sub_ctrl1_offset);
			dev->ep_shadow[0][ep_dev_ctrl2stts2_offset / 4] =
				readl_relaxed(dev->conf +
				ep_dev_ctrl2stts2_offset);
		}
		PCIE_DBG_FS(dev, "PCIe: RC's L1SUB_CONTROL1:0x%x\n",
			readl_relaxed(dev->dm_core +
			PCIE20_L1SUB_CONTROL1));
		PCIE_DBG_FS(dev, "PCIe: RC's DEVICE_CONTROL2_STATUS2:0x%x\n",
			readl_relaxed(dev->dm_core +
			PCIE20_DEVICE_CONTROL2_STATUS2));
		PCIE_DBG_FS(dev, "PCIe: EP's L1SUB_CONTROL1:0x%x\n",
			readl_relaxed(dev->conf +
			ep_l1sub_ctrl1_offset));
		PCIE_DBG_FS(dev, "PCIe: EP's DEVICE_CONTROL2_STATUS2:0x%x\n",
			readl_relaxed(dev->conf +
			ep_dev_ctrl2stts2_offset));
		break;
	case 10: /* enable L1ss */
		PCIE_DBG_FS(dev, "\n\nPCIe: RC%d: enable L1ss\n\n",
			dev->rc_idx);
		current_offset = PCIE_EXT_CAP_OFFSET;
		while (current_offset) {
			val = readl_relaxed(dev->conf + current_offset);
			if ((val & 0xffff) == L1SUB_CAP_ID) {
				ep_l1sub_cap_reg1_offset =
						current_offset + 0x4;
				ep_l1sub_ctrl1_offset =
						current_offset + 0x8;
				break;
			}
			current_offset = val >> 20;
		}
		if (!ep_l1sub_ctrl1_offset) {
			PCIE_DBG_FS(dev,
				"PCIe: RC%d endpoint does not support l1ss registers\n",
				dev->rc_idx);
			break;
		}

		val = readl_relaxed(dev->conf +
				ep_l1sub_cap_reg1_offset);

		PCIE_DBG_FS(dev, "PCIe: EP's L1SUB_CAPABILITY_REG_1: 0x%x\n",
			val);
		PCIE_DBG_FS(dev, "PCIe: RC%d: ep_l1sub_ctrl1_offset: 0x%x\n",
			dev->rc_idx, ep_l1sub_ctrl1_offset);

		val &= 0xf;

		msm_pcie_write_reg_field(dev->dm_core,
					PCIE20_L1SUB_CONTROL1,
					0xf, val);
		msm_pcie_write_mask(dev->dm_core +
					PCIE20_DEVICE_CONTROL2_STATUS2,
					0, BIT(10));
		msm_pcie_write_reg_field(dev->conf,
					ep_l1sub_ctrl1_offset,
					0xf, val);
		msm_pcie_write_mask(dev->conf +
					ep_dev_ctrl2stts2_offset,
					0, BIT(10));
		if (dev->shadow_en) {
			dev->rc_shadow[PCIE20_L1SUB_CONTROL1 / 4] =
				readl_relaxed(dev->dm_core +
					PCIE20_L1SUB_CONTROL1);
			dev->rc_shadow[PCIE20_DEVICE_CONTROL2_STATUS2 / 4] =
				readl_relaxed(dev->dm_core +
				PCIE20_DEVICE_CONTROL2_STATUS2);
			dev->ep_shadow[0][ep_l1sub_ctrl1_offset / 4] =
				readl_relaxed(dev->conf +
				ep_l1sub_ctrl1_offset);
			dev->ep_shadow[0][ep_dev_ctrl2stts2_offset / 4] =
				readl_relaxed(dev->conf +
				ep_dev_ctrl2stts2_offset);
		}
		PCIE_DBG_FS(dev, "PCIe: RC's L1SUB_CONTROL1:0x%x\n",
			readl_relaxed(dev->dm_core +
			PCIE20_L1SUB_CONTROL1));
		PCIE_DBG_FS(dev, "PCIe: RC's DEVICE_CONTROL2_STATUS2:0x%x\n",
			readl_relaxed(dev->dm_core +
			PCIE20_DEVICE_CONTROL2_STATUS2));
		PCIE_DBG_FS(dev, "PCIe: EP's L1SUB_CONTROL1:0x%x\n",
			readl_relaxed(dev->conf +
			ep_l1sub_ctrl1_offset));
		PCIE_DBG_FS(dev, "PCIe: EP's DEVICE_CONTROL2_STATUS2:0x%x\n",
			readl_relaxed(dev->conf +
			ep_dev_ctrl2stts2_offset));
		break;
	case 11: /* enumerate PCIe  */
		PCIE_DBG_FS(dev, "\n\nPCIe: attempting to enumerate RC%d\n\n",
			dev->rc_idx);
		if (dev->enumerated)
			PCIE_DBG_FS(dev, "PCIe: RC%d is already enumerated\n",
				dev->rc_idx);
		else {
			if (!msm_pcie_enumerate(dev->rc_idx))
				PCIE_DBG_FS(dev,
					"PCIe: RC%d is successfully enumerated\n",
					dev->rc_idx);
			else
				PCIE_DBG_FS(dev,
					"PCIe: RC%d enumeration failed\n",
					dev->rc_idx);
		}
		break;
	case 12: /* write a value to a register */
		PCIE_DBG_FS(dev,
			"\n\nPCIe: RC%d: writing a value to a register\n\n",
			dev->rc_idx);

		if (!base_sel) {
			PCIE_DBG_FS(dev, "Invalid base_sel: 0x%x\n", base_sel);
			break;
		}

		PCIE_DBG_FS(dev,
			"base: %s: 0x%p\nwr_offset: 0x%x\nwr_mask: 0x%x\nwr_value: 0x%x\n",
			dev->res[base_sel - 1].name,
			dev->res[base_sel - 1].base,
			wr_offset, wr_mask, wr_value);

		msm_pcie_write_reg_field(dev->res[base_sel - 1].base,
			wr_offset, wr_mask, wr_value);

		break;
	case 13: /* dump all registers of base_sel */
		if (!base_sel) {
			PCIE_DBG_FS(dev, "Invalid base_sel: 0x%x\n", base_sel);
			break;
		} else if (base_sel - 1 == MSM_PCIE_RES_PARF) {
			pcie_parf_dump(dev);
			break;
		} else if (base_sel - 1 == MSM_PCIE_RES_PHY) {
			pcie_phy_dump(dev);
			break;
		} else if (base_sel - 1 == MSM_PCIE_RES_CONF) {
			base_sel_size = 0x1000;
		} else {
			base_sel_size = resource_size(
				dev->res[base_sel - 1].resource);
		}

		PCIE_DBG_FS(dev, "\n\nPCIe: Dumping %s Registers for RC%d\n\n",
			dev->res[base_sel - 1].name, dev->rc_idx);

		for (i = 0; i < base_sel_size; i += 32) {
			PCIE_DBG_FS(dev,
			"0x%04x %08x %08x %08x %08x %08x %08x %08x %08x\n",
			i, readl_relaxed(dev->res[base_sel - 1].base + i),
			readl_relaxed(dev->res[base_sel - 1].base + (i + 4)),
			readl_relaxed(dev->res[base_sel - 1].base + (i + 8)),
			readl_relaxed(dev->res[base_sel - 1].base + (i + 12)),
			readl_relaxed(dev->res[base_sel - 1].base + (i + 16)),
			readl_relaxed(dev->res[base_sel - 1].base + (i + 20)),
			readl_relaxed(dev->res[base_sel - 1].base + (i + 24)),
			readl_relaxed(dev->res[base_sel - 1].base + (i + 28)));
		}
		break;
	default:
		PCIE_DBG_FS(dev, "Invalid testcase: %d.\n", testcase);
		break;
	}
}

int msm_pcie_debug_info(struct pci_dev *dev, u32 option, u32 base,
			u32 offset, u32 mask, u32 value)
{
	int ret = 0;
	struct msm_pcie_dev_t *pdev;

	if (!dev) {
		pr_err("PCIe: the input pci dev is NULL.\n");
		return -ENODEV;
	}

	if (option == 12 || option == 13) {
		if (!base || base > 5) {
			PCIE_DBG_FS(pdev, "Invalid base_sel: 0x%x\n", base);
			PCIE_DBG_FS(pdev,
				"PCIe: base_sel is still 0x%x\n", base_sel);
			return -EINVAL;
		} else {
			base_sel = base;
			PCIE_DBG_FS(pdev,
				"PCIe: base_sel is now 0x%x\n", base_sel);
		}

		if (option == 12) {
			wr_offset = offset;
			wr_mask = mask;
			wr_value = value;

			PCIE_DBG_FS(pdev,
				"PCIe: wr_offset is now 0x%x\n", wr_offset);
			PCIE_DBG_FS(pdev,
				"PCIe: wr_mask is now 0x%x\n", wr_mask);
			PCIE_DBG_FS(pdev,
				"PCIe: wr_value is now 0x%x\n", wr_value);
		}
	}

	pdev = PCIE_BUS_PRIV_DATA(dev->bus);
	rc_sel = 1 << pdev->rc_idx;

	msm_pcie_sel_debug_testcase(pdev, option);

	return ret;
}
EXPORT_SYMBOL(msm_pcie_debug_info);

#ifdef CONFIG_DEBUG_FS
static struct dentry *dent_msm_pcie;
static struct dentry *dfile_rc_sel;
static struct dentry *dfile_case;
static struct dentry *dfile_base_sel;
static struct dentry *dfile_linkdown_panic;
static struct dentry *dfile_wr_offset;
static struct dentry *dfile_wr_mask;
static struct dentry *dfile_wr_value;
static struct dentry *dfile_ep_wakeirq;
static struct dentry *dfile_aer_enable;
static struct dentry *dfile_corr_counter_limit;

static u32 rc_sel_max;

static ssize_t msm_pcie_cmd_debug(struct file *file,
				const char __user *buf,
				size_t count, loff_t *ppos)
{
	unsigned long ret;
	char str[MAX_MSG_LEN];
	unsigned int testcase = 0;
	int i;

	memset(str, 0, sizeof(str));
	ret = copy_from_user(str, buf, sizeof(str));
	if (ret)
		return -EFAULT;

	for (i = 0; i < sizeof(str) && (str[i] >= '0') && (str[i] <= '9'); ++i)
		testcase = (testcase * 10) + (str[i] - '0');

	if (!rc_sel)
		rc_sel = 1;

	pr_alert("PCIe: TEST: %d\n", testcase);

	for (i = 0; i < MAX_RC_NUM; i++) {
		if (!((rc_sel >> i) & 0x1))
			continue;
		msm_pcie_sel_debug_testcase(&msm_pcie_dev[i], testcase);
	}

	return count;
}

const struct file_operations msm_pcie_cmd_debug_ops = {
	.write = msm_pcie_cmd_debug,
};

static ssize_t msm_pcie_set_rc_sel(struct file *file,
				const char __user *buf,
				size_t count, loff_t *ppos)
{
	unsigned long ret;
	char str[MAX_MSG_LEN];
	int i;
	u32 new_rc_sel = 0;

	memset(str, 0, sizeof(str));
	ret = copy_from_user(str, buf, sizeof(str));
	if (ret)
		return -EFAULT;

	for (i = 0; i < sizeof(str) && (str[i] >= '0') && (str[i] <= '9'); ++i)
		new_rc_sel = (new_rc_sel * 10) + (str[i] - '0');

	if ((!new_rc_sel) || (new_rc_sel > rc_sel_max)) {
		pr_alert("PCIe: invalid value for rc_sel: 0x%x\n", new_rc_sel);
		pr_alert("PCIe: rc_sel is still 0x%x\n", rc_sel ? rc_sel : 0x1);
	} else {
		rc_sel = new_rc_sel;
		pr_alert("PCIe: rc_sel is now: 0x%x\n", rc_sel);
	}

	pr_alert("PCIe: the following RC(s) will be tested:\n");
	for (i = 0; i < MAX_RC_NUM; i++) {
		if (!rc_sel) {
			pr_alert("RC %d\n", i);
			break;
		} else if (rc_sel & (1 << i)) {
			pr_alert("RC %d\n", i);
		}
	}

	return count;
}

const struct file_operations msm_pcie_rc_sel_ops = {
	.write = msm_pcie_set_rc_sel,
};

static ssize_t msm_pcie_set_base_sel(struct file *file,
				const char __user *buf,
				size_t count, loff_t *ppos)
{
	unsigned long ret;
	char str[MAX_MSG_LEN];
	int i;
	u32 new_base_sel = 0;
	char *base_sel_name;

	memset(str, 0, sizeof(str));
	ret = copy_from_user(str, buf, sizeof(str));
	if (ret)
		return -EFAULT;

	for (i = 0; i < sizeof(str) && (str[i] >= '0') && (str[i] <= '9'); ++i)
		new_base_sel = (new_base_sel * 10) + (str[i] - '0');

	if (!new_base_sel || new_base_sel > 5) {
		pr_alert("PCIe: invalid value for base_sel: 0x%x\n",
			new_base_sel);
		pr_alert("PCIe: base_sel is still 0x%x\n", base_sel);
	} else {
		base_sel = new_base_sel;
		pr_alert("PCIe: base_sel is now 0x%x\n", base_sel);
	}

	switch (base_sel) {
	case 1:
		base_sel_name = "PARF";
		break;
	case 2:
		base_sel_name = "PHY";
		break;
	case 3:
		base_sel_name = "RC CONFIG SPACE";
		break;
	case 4:
		base_sel_name = "ELBI";
		break;
	case 5:
		base_sel_name = "EP CONFIG SPACE";
		break;
	default:
		base_sel_name = "INVALID";
		break;
	}

	pr_alert("%s\n", base_sel_name);

	return count;
}

const struct file_operations msm_pcie_base_sel_ops = {
	.write = msm_pcie_set_base_sel,
};

static ssize_t msm_pcie_set_linkdown_panic(struct file *file,
				const char __user *buf,
				size_t count, loff_t *ppos)
{
	unsigned long ret;
	char str[MAX_MSG_LEN];
	u32 new_linkdown_panic = 0;
	int i;

	memset(str, 0, sizeof(str));
	ret = copy_from_user(str, buf, sizeof(str));
	if (ret)
		return -EFAULT;

	for (i = 0; i < sizeof(str) && (str[i] >= '0') && (str[i] <= '9'); ++i)
		new_linkdown_panic = (new_linkdown_panic * 10) + (str[i] - '0');

	if (new_linkdown_panic <= 1) {
		for (i = 0; i < MAX_RC_NUM; i++) {
			if (!rc_sel) {
				msm_pcie_dev[0].linkdown_panic =
					new_linkdown_panic;
				PCIE_DBG_FS(&msm_pcie_dev[0],
					"PCIe: RC0: linkdown_panic is now %d\n",
					msm_pcie_dev[0].linkdown_panic);
				break;
			} else if (rc_sel & (1 << i)) {
				msm_pcie_dev[i].linkdown_panic =
					new_linkdown_panic;
				PCIE_DBG_FS(&msm_pcie_dev[i],
					"PCIe: RC%d: linkdown_panic is now %d\n",
					i, msm_pcie_dev[i].linkdown_panic);
			}
		}
	} else {
		pr_err("PCIe: Invalid input for linkdown_panic: %d. Please enter 0 or 1.\n",
			new_linkdown_panic);
	}

	return count;
}

const struct file_operations msm_pcie_linkdown_panic_ops = {
	.write = msm_pcie_set_linkdown_panic,
};

static ssize_t msm_pcie_set_wr_offset(struct file *file,
				const char __user *buf,
				size_t count, loff_t *ppos)
{
	unsigned long ret;
	char str[MAX_MSG_LEN];
	int i;

	memset(str, 0, sizeof(str));
	ret = copy_from_user(str, buf, sizeof(str));
	if (ret)
		return -EFAULT;

	wr_offset = 0;
	for (i = 0; i < sizeof(str) && (str[i] >= '0') && (str[i] <= '9'); ++i)
		wr_offset = (wr_offset * 10) + (str[i] - '0');

	pr_alert("PCIe: wr_offset is now 0x%x\n", wr_offset);

	return count;
}

const struct file_operations msm_pcie_wr_offset_ops = {
	.write = msm_pcie_set_wr_offset,
};

static ssize_t msm_pcie_set_wr_mask(struct file *file,
				const char __user *buf,
				size_t count, loff_t *ppos)
{
	unsigned long ret;
	char str[MAX_MSG_LEN];
	int i;

	memset(str, 0, sizeof(str));
	ret = copy_from_user(str, buf, sizeof(str));
	if (ret)
		return -EFAULT;

	wr_mask = 0;
	for (i = 0; i < sizeof(str) && (str[i] >= '0') && (str[i] <= '9'); ++i)
		wr_mask = (wr_mask * 10) + (str[i] - '0');

	pr_alert("PCIe: wr_mask is now 0x%x\n", wr_mask);

	return count;
}

const struct file_operations msm_pcie_wr_mask_ops = {
	.write = msm_pcie_set_wr_mask,
};
static ssize_t msm_pcie_set_wr_value(struct file *file,
				const char __user *buf,
				size_t count, loff_t *ppos)
{
	unsigned long ret;
	char str[MAX_MSG_LEN];
	int i;

	memset(str, 0, sizeof(str));
	ret = copy_from_user(str, buf, sizeof(str));
	if (ret)
		return -EFAULT;

	wr_value = 0;
	for (i = 0; i < sizeof(str) && (str[i] >= '0') && (str[i] <= '9'); ++i)
		wr_value = (wr_value * 10) + (str[i] - '0');

	pr_alert("PCIe: wr_value is now 0x%x\n", wr_value);

	return count;
}

const struct file_operations msm_pcie_wr_value_ops = {
	.write = msm_pcie_set_wr_value,
};

static ssize_t msm_pcie_set_ep_wakeirq(struct file *file,
				const char __user *buf,
				size_t count, loff_t *ppos)
{
	unsigned long ret;
	char str[MAX_MSG_LEN];
	u32 new_ep_wakeirq = 0;
	int i;

	memset(str, 0, sizeof(str));
	ret = copy_from_user(str, buf, sizeof(str));
	if (ret)
		return -EFAULT;

	for (i = 0; i < sizeof(str) && (str[i] >= '0') && (str[i] <= '9'); ++i)
		new_ep_wakeirq = (new_ep_wakeirq * 10) + (str[i] - '0');

	if (new_ep_wakeirq <= 1) {
		for (i = 0; i < MAX_RC_NUM; i++) {
			if (!rc_sel) {
				msm_pcie_dev[0].ep_wakeirq = new_ep_wakeirq;
				PCIE_DBG_FS(&msm_pcie_dev[0],
					"PCIe: RC0: ep_wakeirq is now %d\n",
					msm_pcie_dev[0].ep_wakeirq);
				break;
			} else if (rc_sel & (1 << i)) {
				msm_pcie_dev[i].ep_wakeirq = new_ep_wakeirq;
				PCIE_DBG_FS(&msm_pcie_dev[i],
					"PCIe: RC%d: ep_wakeirq is now %d\n",
					i, msm_pcie_dev[i].ep_wakeirq);
			}
		}
	} else {
		pr_err("PCIe: Invalid input for ep_wakeirq: %d. Please enter 0 or 1.\n",
			new_ep_wakeirq);
	}

	return count;
}

const struct file_operations msm_pcie_ep_wakeirq_ops = {
	.write = msm_pcie_set_ep_wakeirq,
};

static ssize_t msm_pcie_set_aer_enable(struct file *file,
				const char __user *buf,
				size_t count, loff_t *ppos)
{
	unsigned long ret;
	char str[MAX_MSG_LEN];
	u32 new_aer_enable = 0;
	u32 temp_rc_sel;
	int i;

	memset(str, 0, sizeof(str));
	ret = copy_from_user(str, buf, sizeof(str));
	if (ret)
		return -EFAULT;

	for (i = 0; i < sizeof(str) && (str[i] >= '0') && (str[i] <= '9'); ++i)
		new_aer_enable = (new_aer_enable * 10) + (str[i] - '0');

	if (new_aer_enable > 1) {
		pr_err(
			"PCIe: Invalid input for aer_enable: %d. Please enter 0 or 1.\n",
			new_aer_enable);
		return count;
	}

	if (rc_sel)
		temp_rc_sel = rc_sel;
	else
		temp_rc_sel = 0x1;

	for (i = 0; i < MAX_RC_NUM; i++) {
		if (temp_rc_sel & (1 << i)) {
			msm_pcie_dev[i].aer_enable = new_aer_enable;
			PCIE_DBG_FS(&msm_pcie_dev[i],
				"PCIe: RC%d: aer_enable is now %d\n",
				i, msm_pcie_dev[i].aer_enable);

			msm_pcie_write_mask(msm_pcie_dev[i].dm_core +
					PCIE20_BRIDGE_CTRL,
					new_aer_enable ? 0 : BIT(16),
					new_aer_enable ? BIT(16) : 0);

			PCIE_DBG_FS(&msm_pcie_dev[i],
				"RC%d: PCIE20_BRIDGE_CTRL: 0x%x\n", i,
				readl_relaxed(msm_pcie_dev[i].dm_core +
					PCIE20_BRIDGE_CTRL));
		}
	}

	return count;
}

const struct file_operations msm_pcie_aer_enable_ops = {
	.write = msm_pcie_set_aer_enable,
};

static ssize_t msm_pcie_set_corr_counter_limit(struct file *file,
				const char __user *buf,
				size_t count, loff_t *ppos)
{
	unsigned long ret;
	char str[MAX_MSG_LEN];
	int i;

	memset(str, 0, sizeof(str));
	ret = copy_from_user(str, buf, sizeof(str));
	if (ret)
		return -EFAULT;

	corr_counter_limit = 0;
	for (i = 0; i < sizeof(str) && (str[i] >= '0') && (str[i] <= '9'); ++i)
		corr_counter_limit = (corr_counter_limit * 10) + (str[i] - '0');

	pr_info("PCIe: corr_counter_limit is now %lu\n", corr_counter_limit);

	return count;
}

const struct file_operations msm_pcie_corr_counter_limit_ops = {
	.write = msm_pcie_set_corr_counter_limit,
};

static void msm_pcie_debugfs_init(void)
{
	rc_sel_max = (0x1 << MAX_RC_NUM) - 1;
	wr_mask = 0xffffffff;

	dent_msm_pcie = debugfs_create_dir("pci-msm", 0);
	if (IS_ERR(dent_msm_pcie)) {
		pr_err("PCIe: fail to create the folder for debug_fs.\n");
		return;
	}

	dfile_rc_sel = debugfs_create_file("rc_sel", 0664,
					dent_msm_pcie, 0,
					&msm_pcie_rc_sel_ops);
	if (!dfile_rc_sel || IS_ERR(dfile_rc_sel)) {
		pr_err("PCIe: fail to create the file for debug_fs rc_sel.\n");
		goto rc_sel_error;
	}

	dfile_case = debugfs_create_file("case", 0664,
					dent_msm_pcie, 0,
					&msm_pcie_cmd_debug_ops);
	if (!dfile_case || IS_ERR(dfile_case)) {
		pr_err("PCIe: fail to create the file for debug_fs case.\n");
		goto case_error;
	}

	dfile_base_sel = debugfs_create_file("base_sel", 0664,
					dent_msm_pcie, 0,
					&msm_pcie_base_sel_ops);
	if (!dfile_base_sel || IS_ERR(dfile_base_sel)) {
		pr_err("PCIe: fail to create the file for debug_fs base_sel.\n");
		goto base_sel_error;
	}

	dfile_linkdown_panic = debugfs_create_file("linkdown_panic", 0644,
					dent_msm_pcie, 0,
					&msm_pcie_linkdown_panic_ops);
	if (!dfile_linkdown_panic || IS_ERR(dfile_linkdown_panic)) {
		pr_err("PCIe: fail to create the file for debug_fs linkdown_panic.\n");
		goto linkdown_panic_error;
	}

	dfile_wr_offset = debugfs_create_file("wr_offset", 0664,
					dent_msm_pcie, 0,
					&msm_pcie_wr_offset_ops);
	if (!dfile_wr_offset || IS_ERR(dfile_wr_offset)) {
		pr_err("PCIe: fail to create the file for debug_fs wr_offset.\n");
		goto wr_offset_error;
	}

	dfile_wr_mask = debugfs_create_file("wr_mask", 0664,
					dent_msm_pcie, 0,
					&msm_pcie_wr_mask_ops);
	if (!dfile_wr_mask || IS_ERR(dfile_wr_mask)) {
		pr_err("PCIe: fail to create the file for debug_fs wr_mask.\n");
		goto wr_mask_error;
	}

	dfile_wr_value = debugfs_create_file("wr_value", 0664,
					dent_msm_pcie, 0,
					&msm_pcie_wr_value_ops);
	if (!dfile_wr_value || IS_ERR(dfile_wr_value)) {
		pr_err("PCIe: fail to create the file for debug_fs wr_value.\n");
		goto wr_value_error;
	}

	dfile_ep_wakeirq = debugfs_create_file("ep_wakeirq", 0664,
					dent_msm_pcie, 0,
					&msm_pcie_ep_wakeirq_ops);
	if (!dfile_ep_wakeirq || IS_ERR(dfile_ep_wakeirq)) {
		pr_err("PCIe: fail to create the file for debug_fs ep_wakeirq.\n");
		goto ep_wakeirq_error;
	}

	dfile_aer_enable = debugfs_create_file("aer_enable", 0664,
					dent_msm_pcie, 0,
					&msm_pcie_aer_enable_ops);
	if (!dfile_aer_enable || IS_ERR(dfile_aer_enable)) {
		pr_err("PCIe: fail to create the file for debug_fs aer_enable.\n");
		goto aer_enable_error;
	}

	dfile_corr_counter_limit = debugfs_create_file("corr_counter_limit",
					0664, dent_msm_pcie, 0,
					&msm_pcie_corr_counter_limit_ops);
	if (!dfile_corr_counter_limit || IS_ERR(dfile_corr_counter_limit)) {
		pr_err("PCIe: fail to create the file for debug_fs corr_counter_limit.\n");
		goto corr_counter_limit_error;
	}
	return;

corr_counter_limit_error:
	debugfs_remove(dfile_aer_enable);
aer_enable_error:
	debugfs_remove(dfile_ep_wakeirq);
ep_wakeirq_error:
	debugfs_remove(dfile_wr_value);
wr_value_error:
	debugfs_remove(dfile_wr_mask);
wr_mask_error:
	debugfs_remove(dfile_wr_offset);
wr_offset_error:
	debugfs_remove(dfile_linkdown_panic);
linkdown_panic_error:
	debugfs_remove(dfile_base_sel);
base_sel_error:
	debugfs_remove(dfile_case);
case_error:
	debugfs_remove(dfile_rc_sel);
rc_sel_error:
	debugfs_remove(dent_msm_pcie);
}

static void msm_pcie_debugfs_exit(void)
{
	debugfs_remove(dfile_rc_sel);
	debugfs_remove(dfile_case);
	debugfs_remove(dfile_base_sel);
	debugfs_remove(dfile_linkdown_panic);
	debugfs_remove(dfile_wr_offset);
	debugfs_remove(dfile_wr_mask);
	debugfs_remove(dfile_wr_value);
	debugfs_remove(dfile_ep_wakeirq);
	debugfs_remove(dfile_aer_enable);
	debugfs_remove(dfile_corr_counter_limit);
}
#else
static void msm_pcie_debugfs_init(void)
{
	return;
}

static void msm_pcie_debugfs_exit(void)
{
	return;
}
#endif

static inline int msm_pcie_is_link_up(struct msm_pcie_dev_t *dev)
{
	return readl_relaxed(dev->dm_core +
			PCIE20_CAP_LINKCTRLSTATUS) & BIT(29);
}

/**
 * msm_pcie_iatu_config - configure outbound address translation region
 * @dev: root commpex
 * @nr: region number
 * @type: target transaction type, see PCIE20_CTRL1_TYPE_xxx
 * @host_addr: - region start address on host
 * @host_end: - region end address (low 32 bit) on host,
 *	upper 32 bits are same as for @host_addr
 * @target_addr: - region start address on target
 */
static void msm_pcie_iatu_config(struct msm_pcie_dev_t *dev, int nr, u8 type,
				unsigned long host_addr, u32 host_end,
				unsigned long target_addr)
{
	void __iomem *pcie20 = dev->dm_core;

	if (dev->shadow_en) {
		dev->rc_shadow[PCIE20_PLR_IATU_VIEWPORT / 4] =
			nr;
		dev->rc_shadow[PCIE20_PLR_IATU_CTRL1 / 4] =
			type;
		dev->rc_shadow[PCIE20_PLR_IATU_LBAR / 4] =
			lower_32_bits(host_addr);
		dev->rc_shadow[PCIE20_PLR_IATU_UBAR / 4] =
			upper_32_bits(host_addr);
		dev->rc_shadow[PCIE20_PLR_IATU_LAR / 4] =
			host_end;
		dev->rc_shadow[PCIE20_PLR_IATU_LTAR / 4] =
			lower_32_bits(target_addr);
		dev->rc_shadow[PCIE20_PLR_IATU_UTAR / 4] =
			upper_32_bits(target_addr);
		dev->rc_shadow[PCIE20_PLR_IATU_CTRL2 / 4] =
			BIT(31);
	}

	/* select region */
	writel_relaxed(nr, pcie20 + PCIE20_PLR_IATU_VIEWPORT);
	/* ensure that hardware locks it */
	wmb();

	/* switch off region before changing it */
	writel_relaxed(0, pcie20 + PCIE20_PLR_IATU_CTRL2);
	/* and wait till it propagates to the hardware */
	wmb();

	writel_relaxed(type, pcie20 + PCIE20_PLR_IATU_CTRL1);
	writel_relaxed(lower_32_bits(host_addr),
		       pcie20 + PCIE20_PLR_IATU_LBAR);
	writel_relaxed(upper_32_bits(host_addr),
		       pcie20 + PCIE20_PLR_IATU_UBAR);
	writel_relaxed(host_end, pcie20 + PCIE20_PLR_IATU_LAR);
	writel_relaxed(lower_32_bits(target_addr),
		       pcie20 + PCIE20_PLR_IATU_LTAR);
	writel_relaxed(upper_32_bits(target_addr),
		       pcie20 + PCIE20_PLR_IATU_UTAR);
	wmb();
	writel_relaxed(BIT(31), pcie20 + PCIE20_PLR_IATU_CTRL2);

	/* ensure that changes propagated to the hardware */
	wmb();

	if (dev->enumerated) {
		PCIE_DBG2(dev, "IATU for Endpoint %02x:%02x.%01x\n",
			dev->pcidev_table[nr].bdf >> 24,
			dev->pcidev_table[nr].bdf >> 19 & 0x1f,
			dev->pcidev_table[nr].bdf >> 16 & 0x07);
		PCIE_DBG2(dev, "PCIE20_PLR_IATU_VIEWPORT:0x%x\n",
			readl_relaxed(dev->dm_core + PCIE20_PLR_IATU_VIEWPORT));
		PCIE_DBG2(dev, "PCIE20_PLR_IATU_CTRL1:0x%x\n",
			readl_relaxed(dev->dm_core + PCIE20_PLR_IATU_CTRL1));
		PCIE_DBG2(dev, "PCIE20_PLR_IATU_LBAR:0x%x\n",
			readl_relaxed(dev->dm_core + PCIE20_PLR_IATU_LBAR));
		PCIE_DBG2(dev, "PCIE20_PLR_IATU_UBAR:0x%x\n",
			readl_relaxed(dev->dm_core + PCIE20_PLR_IATU_UBAR));
		PCIE_DBG2(dev, "PCIE20_PLR_IATU_LAR:0x%x\n",
			readl_relaxed(dev->dm_core + PCIE20_PLR_IATU_LAR));
		PCIE_DBG2(dev, "PCIE20_PLR_IATU_LTAR:0x%x\n",
			readl_relaxed(dev->dm_core + PCIE20_PLR_IATU_LTAR));
		PCIE_DBG2(dev, "PCIE20_PLR_IATU_UTAR:0x%x\n",
			readl_relaxed(dev->dm_core + PCIE20_PLR_IATU_UTAR));
		PCIE_DBG2(dev, "PCIE20_PLR_IATU_CTRL2:0x%x\n\n",
			readl_relaxed(dev->dm_core + PCIE20_PLR_IATU_CTRL2));
	}
}

/**
 * msm_pcie_cfg_bdf - configure for config access
 * @dev: root commpex
 * @bus: PCI bus number
 * @devfn: PCI dev and function number
 *
 * Remap if required region 0 for config access of proper type
 * (CFG0 for bus 1, CFG1 for other buses)
 * Cache current device bdf for speed-up
 */
static void msm_pcie_cfg_bdf(struct msm_pcie_dev_t *dev, u8 bus, u8 devfn)
{
	struct resource *axi_conf = dev->res[MSM_PCIE_RES_CONF].resource;
	u32 bdf  = BDF_OFFSET(bus, devfn);
	u8 type = bus == 1 ? PCIE20_CTRL1_TYPE_CFG0 : PCIE20_CTRL1_TYPE_CFG1;
	if (dev->current_bdf == bdf)
		return;

	msm_pcie_iatu_config(dev, 0, type,
			axi_conf->start,
			axi_conf->start + SZ_4K - 1,
			bdf);

	dev->current_bdf = bdf;
}

static inline void msm_pcie_save_shadow(struct msm_pcie_dev_t *dev,
					u32 word_offset, u32 wr_val,
					u32 bdf, bool rc)
{
	int i, j;
	u32 max_dev = MAX_RC_NUM * MAX_DEVICE_NUM;

	if (rc) {
		dev->rc_shadow[word_offset / 4] = wr_val;
	} else {
		for (i = 0; i < MAX_DEVICE_NUM; i++) {
			if (!dev->pcidev_table[i].bdf) {
				for (j = 0; j < max_dev; j++)
					if (!msm_pcie_dev_tbl[j].bdf) {
						msm_pcie_dev_tbl[j].bdf = bdf;
						break;
					}
				dev->pcidev_table[i].bdf = bdf;
				if ((!dev->bridge_found) && (i > 0))
					dev->bridge_found = true;
			}
			if (dev->pcidev_table[i].bdf == bdf) {
				dev->ep_shadow[i][word_offset / 4] = wr_val;
				break;
			}
		}
	}
}

static inline int msm_pcie_oper_conf(struct pci_bus *bus, u32 devfn, int oper,
				     int where, int size, u32 *val)
{
	uint32_t word_offset, byte_offset, mask;
	uint32_t rd_val, wr_val;
	struct msm_pcie_dev_t *dev;
	void __iomem *config_base;
	bool rc = false;
	u32 rc_idx;
	int rv = 0;
	u32 bdf = BDF_OFFSET(bus->number, devfn);
	int i;

	dev = PCIE_BUS_PRIV_DATA(bus);

	if (!dev) {
		pr_err("PCIe: No device found for this bus.\n");
		*val = ~0;
		rv = PCIBIOS_DEVICE_NOT_FOUND;
		goto out;
	}

	rc_idx = dev->rc_idx;
	rc = (bus->number == 0);

	spin_lock_irqsave(&dev->cfg_lock, dev->irqsave_flags);

	if (!dev->cfg_access) {
		PCIE_DBG3(dev,
			"Access denied for RC%d %d:0x%02x + 0x%04x[%d]\n",
			rc_idx, bus->number, devfn, where, size);
		*val = ~0;
		rv = PCIBIOS_DEVICE_NOT_FOUND;
		goto unlock;
	}

	if (rc && (devfn != 0)) {
		PCIE_DBG3(dev, "RC%d invalid %s - bus %d devfn %d\n", rc_idx,
			 (oper == RD) ? "rd" : "wr", bus->number, devfn);
		*val = ~0;
		rv = PCIBIOS_DEVICE_NOT_FOUND;
		goto unlock;
	}

	if (dev->link_status != MSM_PCIE_LINK_ENABLED) {
		PCIE_DBG3(dev,
			"Access to RC%d %d:0x%02x + 0x%04x[%d] is denied because link is down\n",
			rc_idx, bus->number, devfn, where, size);
		*val = ~0;
		rv = PCIBIOS_DEVICE_NOT_FOUND;
		goto unlock;
	}

	/* check if the link is up for endpoint */
	if (!rc && !msm_pcie_is_link_up(dev)) {
		PCIE_ERR(dev,
			"PCIe: RC%d %s fail, link down - bus %d devfn %d\n",
				rc_idx, (oper == RD) ? "rd" : "wr",
				bus->number, devfn);
			*val = ~0;
			rv = PCIBIOS_DEVICE_NOT_FOUND;
			goto unlock;
	}

	if (!rc && !dev->enumerated)
		msm_pcie_cfg_bdf(dev, bus->number, devfn);

	word_offset = where & ~0x3;
	byte_offset = where & 0x3;
	mask = (~0 >> (8 * (4 - size))) << (8 * byte_offset);

	if (rc || !dev->enumerated) {
		config_base = rc ? dev->dm_core : dev->conf;
	} else {
		for (i = 0; i < MAX_DEVICE_NUM; i++) {
			if (dev->pcidev_table[i].bdf == bdf) {
				config_base = dev->pcidev_table[i].conf_base;
				break;
			}
		}
		if (i == MAX_DEVICE_NUM) {
			*val = ~0;
			rv = PCIBIOS_DEVICE_NOT_FOUND;
			goto unlock;
		}
	}

	rd_val = readl_relaxed(config_base + word_offset);

	if (oper == RD) {
		*val = ((rd_val & mask) >> (8 * byte_offset));
		PCIE_DBG3(dev,
			"RC%d %d:0x%02x + 0x%04x[%d] -> 0x%08x; rd 0x%08x\n",
			rc_idx, bus->number, devfn, where, size, *val, rd_val);
	} else {
		wr_val = (rd_val & ~mask) |
				((*val << (8 * byte_offset)) & mask);

		if ((bus->number == 0) && (where == 0x3c))
			wr_val = wr_val | (3 << 16);

		writel_relaxed(wr_val, config_base + word_offset);
		wmb(); /* ensure config data is written to hardware register */

		if (rd_val == PCIE_LINK_DOWN)
			PCIE_ERR(dev,
				"Read of RC%d %d:0x%02x + 0x%04x[%d] is all FFs\n",
				rc_idx, bus->number, devfn, where, size);
		else if (dev->shadow_en)
			msm_pcie_save_shadow(dev, word_offset, wr_val, bdf, rc);

		PCIE_DBG3(dev,
			"RC%d %d:0x%02x + 0x%04x[%d] <- 0x%08x; rd 0x%08x val 0x%08x\n",
			rc_idx, bus->number, devfn, where, size,
			wr_val, rd_val, *val);
	}

unlock:
	spin_unlock_irqrestore(&dev->cfg_lock, dev->irqsave_flags);
out:
	return rv;
}

static int msm_pcie_rd_conf(struct pci_bus *bus, u32 devfn, int where,
			    int size, u32 *val)
{
	int ret = msm_pcie_oper_conf(bus, devfn, RD, where, size, val);

	if ((bus->number == 0) && (where == PCI_CLASS_REVISION)) {
		*val = (*val & 0xff) | (PCI_CLASS_BRIDGE_PCI << 16);
		PCIE_GEN_DBG("change class for RC:0x%x\n", *val);
	}

	return ret;
}

static int msm_pcie_wr_conf(struct pci_bus *bus, u32 devfn,
			    int where, int size, u32 val)
{
	return msm_pcie_oper_conf(bus, devfn, WR, where, size, &val);
}

static struct pci_ops msm_pcie_ops = {
	.read = msm_pcie_rd_conf,
	.write = msm_pcie_wr_conf,
};

static int msm_pcie_gpio_init(struct msm_pcie_dev_t *dev)
{
	int rc, i;
	struct msm_pcie_gpio_info_t *info;

	PCIE_DBG(dev, "RC%d\n", dev->rc_idx);

	for (i = 0; i < dev->gpio_n; i++) {
		info = &dev->gpio[i];

		if (!info->num)
			continue;

		rc = gpio_request(info->num, info->name);
		if (rc) {
			PCIE_ERR(dev, "PCIe: RC%d can't get gpio %s; %d\n",
				dev->rc_idx, info->name, rc);
			break;
		}

		if (info->out)
			rc = gpio_direction_output(info->num, info->init);
		else
			rc = gpio_direction_input(info->num);
		if (rc) {
			PCIE_ERR(dev,
				"PCIe: RC%d can't set direction for GPIO %s:%d\n",
				dev->rc_idx, info->name, rc);
			gpio_free(info->num);
			break;
		}
	}

	if (rc)
		while (i--)
			gpio_free(dev->gpio[i].num);

	return rc;
}

static void msm_pcie_gpio_deinit(struct msm_pcie_dev_t *dev)
{
	int i;

	PCIE_DBG(dev, "RC%d\n", dev->rc_idx);

	for (i = 0; i < dev->gpio_n; i++)
		gpio_free(dev->gpio[i].num);
}

int msm_pcie_vreg_init(struct msm_pcie_dev_t *dev)
{
	int i, rc = 0;
	struct regulator *vreg;
	struct msm_pcie_vreg_info_t *info;

	PCIE_DBG(dev, "RC%d: entry\n", dev->rc_idx);

	for (i = 0; i < MSM_PCIE_MAX_VREG; i++) {
		info = &dev->vreg[i];
		vreg = info->hdl;

		if (!vreg)
			continue;

		PCIE_DBG2(dev, "RC%d Vreg %s is being enabled\n",
			dev->rc_idx, info->name);
		if (info->max_v) {
			rc = regulator_set_voltage(vreg,
						   info->min_v, info->max_v);
			if (rc) {
				PCIE_ERR(dev,
					"PCIe: RC%d can't set voltage for %s: %d\n",
					dev->rc_idx, info->name, rc);
				break;
			}
		}

		if (info->opt_mode) {
			rc = regulator_set_optimum_mode(vreg, info->opt_mode);
			if (rc < 0) {
				PCIE_ERR(dev,
					"PCIe: RC%d can't set mode for %s: %d\n",
					dev->rc_idx, info->name, rc);
				break;
			}
		}

		rc = regulator_enable(vreg);
		if (rc) {
			PCIE_ERR(dev,
				"PCIe: RC%d can't enable regulator %s: %d\n",
				dev->rc_idx, info->name, rc);
			break;
		}
	}

	if (rc)
		while (i--) {
			struct regulator *hdl = dev->vreg[i].hdl;
			if (hdl) {
				regulator_disable(hdl);
				if (!strcmp(dev->vreg[i].name, "vreg-cx")) {
					PCIE_DBG(dev,
						"RC%d: Removing %s vote.\n",
						dev->rc_idx,
						dev->vreg[i].name);
					regulator_set_voltage(hdl,
						RPM_REGULATOR_CORNER_NONE,
						INT_MAX);
				}
			}

		}

	PCIE_DBG(dev, "RC%d: exit\n", dev->rc_idx);

	return rc;
}

static void msm_pcie_vreg_deinit(struct msm_pcie_dev_t *dev)
{
	int i;

	PCIE_DBG(dev, "RC%d: entry\n", dev->rc_idx);

	for (i = MSM_PCIE_MAX_VREG - 1; i >= 0; i--) {
		if (dev->vreg[i].hdl) {
			PCIE_DBG(dev, "Vreg %s is being disabled\n",
				dev->vreg[i].name);
			regulator_disable(dev->vreg[i].hdl);

			if (!strcmp(dev->vreg[i].name, "vreg-cx")) {
				PCIE_DBG(dev,
					"RC%d: Removing %s vote.\n",
					dev->rc_idx,
					dev->vreg[i].name);
				regulator_set_voltage(dev->vreg[i].hdl,
					RPM_REGULATOR_CORNER_NONE,
					INT_MAX);
			}
		}
	}

	PCIE_DBG(dev, "RC%d: exit\n", dev->rc_idx);
}

static int msm_pcie_clk_init(struct msm_pcie_dev_t *dev)
{
	int i, rc = 0;
	struct msm_pcie_clk_info_t *info;

	PCIE_DBG(dev, "RC%d: entry\n", dev->rc_idx);

	rc = regulator_enable(dev->gdsc);

	if (rc) {
		PCIE_ERR(dev, "PCIe: fail to enable GDSC for RC%d (%s)\n",
			dev->rc_idx, dev->pdev->name);
		return rc;
	}

	if (dev->gdsc_smmu) {
		rc = regulator_enable(dev->gdsc_smmu);

		if (rc) {
			PCIE_ERR(dev,
				"PCIe: fail to enable SMMU GDSC for RC%d (%s)\n",
				dev->rc_idx, dev->pdev->name);
			return rc;
		}
	}

	PCIE_DBG(dev, "PCIe: requesting bus vote for RC%d\n", dev->rc_idx);
	if (dev->bus_client) {
		rc = msm_bus_scale_client_update_request(dev->bus_client, 1);
		if (rc) {
			PCIE_ERR(dev,
				"PCIe: fail to set bus bandwidth for RC%d:%d.\n",
				dev->rc_idx, rc);
			return rc;
		} else {
			PCIE_DBG2(dev,
				"PCIe: set bus bandwidth for RC%d.\n",
				dev->rc_idx);
		}
	}

	for (i = 0; i < MSM_PCIE_MAX_CLK; i++) {
		info = &dev->clk[i];

		if (!info->hdl)
			continue;

		if (i >=  MSM_PCIE_MAX_CLK - (dev->common_phy ? 4 : 1))
			clk_reset(info->hdl, CLK_RESET_DEASSERT);

		if (info->freq) {
			rc = clk_set_rate(info->hdl, info->freq);
			if (rc) {
				PCIE_ERR(dev,
					"PCIe: RC%d can't set rate for clk %s: %d.\n",
					dev->rc_idx, info->name, rc);
				break;
			} else {
				PCIE_DBG2(dev,
					"PCIe: RC%d set rate for clk %s.\n",
					dev->rc_idx, info->name);
			}
		}

		rc = clk_prepare_enable(info->hdl);

		if (rc)
			PCIE_ERR(dev, "PCIe: RC%d failed to enable clk %s\n",
				dev->rc_idx, info->name);
		else
			PCIE_DBG2(dev, "enable clk %s for RC%d.\n",
				info->name, dev->rc_idx);
	}

	if (rc) {
		PCIE_DBG(dev, "RC%d disable clocks for error handling.\n",
			dev->rc_idx);
		while (i--) {
			struct clk *hdl = dev->clk[i].hdl;
			if (hdl)
				clk_disable_unprepare(hdl);
		}

		if (dev->gdsc_smmu)
			regulator_disable(dev->gdsc_smmu);

		regulator_disable(dev->gdsc);
	}

	PCIE_DBG(dev, "RC%d: exit\n", dev->rc_idx);

	return rc;
}

static void msm_pcie_clk_deinit(struct msm_pcie_dev_t *dev)
{
	int i;
	int rc;

	PCIE_DBG(dev, "RC%d: entry\n", dev->rc_idx);

	for (i = 0; i < MSM_PCIE_MAX_CLK; i++)
		if (dev->clk[i].hdl)
			clk_disable_unprepare(dev->clk[i].hdl);

	if (dev->bus_client) {
		PCIE_DBG(dev, "PCIe: removing bus vote for RC%d\n",
			dev->rc_idx);

		rc = msm_bus_scale_client_update_request(dev->bus_client, 0);
		if (rc)
			PCIE_ERR(dev,
				"PCIe: fail to relinquish bus bandwidth for RC%d:%d.\n",
				dev->rc_idx, rc);
		else
			PCIE_DBG(dev,
				"PCIe: relinquish bus bandwidth for RC%d.\n",
				dev->rc_idx);
	}

	if (dev->gdsc_smmu)
		regulator_disable(dev->gdsc_smmu);

	regulator_disable(dev->gdsc);

	PCIE_DBG(dev, "RC%d: exit\n", dev->rc_idx);
}

static int msm_pcie_pipe_clk_init(struct msm_pcie_dev_t *dev)
{
	int i, rc = 0;
	struct msm_pcie_clk_info_t *info;

	PCIE_DBG(dev, "RC%d: entry\n", dev->rc_idx);

	for (i = 0; i < MSM_PCIE_MAX_PIPE_CLK; i++) {
		info = &dev->pipeclk[i];

		if (!info->hdl)
			continue;

		clk_reset(info->hdl, CLK_RESET_DEASSERT);

		if (info->freq) {
			rc = clk_set_rate(info->hdl, info->freq);
			if (rc) {
				PCIE_ERR(dev,
					"PCIe: RC%d can't set rate for clk %s: %d.\n",
					dev->rc_idx, info->name, rc);
				break;
			} else {
				PCIE_DBG2(dev,
					"PCIe: RC%d set rate for clk %s: %d.\n",
					dev->rc_idx, info->name, rc);
			}
		}

		rc = clk_prepare_enable(info->hdl);

		if (rc)
			PCIE_ERR(dev, "PCIe: RC%d failed to enable clk %s.\n",
				dev->rc_idx, info->name);
		else
			PCIE_DBG2(dev, "RC%d enabled pipe clk %s.\n",
				dev->rc_idx, info->name);
	}

	if (rc) {
		PCIE_DBG(dev, "RC%d disable pipe clocks for error handling.\n",
			dev->rc_idx);
		while (i--)
			if (dev->pipeclk[i].hdl)
				clk_disable_unprepare(dev->pipeclk[i].hdl);
	}

	PCIE_DBG(dev, "RC%d: exit\n", dev->rc_idx);

	return rc;
}

static void msm_pcie_pipe_clk_deinit(struct msm_pcie_dev_t *dev)
{
	int i;

	PCIE_DBG(dev, "RC%d: entry\n", dev->rc_idx);

	for (i = 0; i < MSM_PCIE_MAX_PIPE_CLK; i++)
		if (dev->pipeclk[i].hdl)
			clk_disable_unprepare(
				dev->pipeclk[i].hdl);

	PCIE_DBG(dev, "RC%d: exit\n", dev->rc_idx);
}

static void msm_pcie_iatu_config_all_ep(struct msm_pcie_dev_t *dev)
{
	int i;
	u8 type;
	struct msm_pcie_device_info *dev_table = dev->pcidev_table;

	for (i = 0; i < MAX_DEVICE_NUM; i++) {
		if (!dev_table[i].bdf)
			break;

		type = dev_table[i].bdf >> 24 == 0x1 ?
			PCIE20_CTRL1_TYPE_CFG0 : PCIE20_CTRL1_TYPE_CFG1;

		msm_pcie_iatu_config(dev, i, type, dev_table[i].phy_address,
			dev_table[i].phy_address + SZ_4K - 1,
			dev_table[i].bdf);
	}
}

static void msm_pcie_config_controller(struct msm_pcie_dev_t *dev)
{
	int i;

	PCIE_DBG(dev, "RC%d\n", dev->rc_idx);

	/*
	 * program and enable address translation region 0 (device config
	 * address space); region type config;
	 * axi config address range to device config address range
	 */
	if (dev->enumerated) {
		msm_pcie_iatu_config_all_ep(dev);
	} else {
		dev->current_bdf = 0; /* to force IATU re-config */
		msm_pcie_cfg_bdf(dev, 1, 0);
	}

	/* configure N_FTS */
	PCIE_DBG2(dev, "Original PCIE20_ACK_F_ASPM_CTRL_REG:0x%x\n",
		readl_relaxed(dev->dm_core + PCIE20_ACK_F_ASPM_CTRL_REG));
	if (!dev->n_fts)
		msm_pcie_write_mask(dev->dm_core + PCIE20_ACK_F_ASPM_CTRL_REG,
					0, BIT(15));
	else
		msm_pcie_write_mask(dev->dm_core + PCIE20_ACK_F_ASPM_CTRL_REG,
					PCIE20_ACK_N_FTS,
					dev->n_fts << 8);

	if (dev->shadow_en)
		dev->rc_shadow[PCIE20_ACK_F_ASPM_CTRL_REG / 4] =
			readl_relaxed(dev->dm_core +
			PCIE20_ACK_F_ASPM_CTRL_REG);

	PCIE_DBG2(dev, "Updated PCIE20_ACK_F_ASPM_CTRL_REG:0x%x\n",
		readl_relaxed(dev->dm_core + PCIE20_ACK_F_ASPM_CTRL_REG));

	/* configure AUX clock frequency register for PCIe core */
	if (dev->use_19p2mhz_aux_clk)
		msm_pcie_write_reg(dev->dm_core, PCIE20_AUX_CLK_FREQ_REG, 0x14);
	else
		msm_pcie_write_reg(dev->dm_core, PCIE20_AUX_CLK_FREQ_REG, 0x01);

	/* configure the completion timeout value for PCIe core */
	if (dev->cpl_timeout && dev->bridge_found)
		msm_pcie_write_reg_field(dev->dm_core,
					PCIE20_DEVICE_CONTROL2_STATUS2,
					0xf, dev->cpl_timeout);

	/* Enable AER on RC */
	if (dev->aer_enable) {
		msm_pcie_write_mask(dev->dm_core + PCIE20_BRIDGE_CTRL, 0,
						BIT(16)|BIT(17));
		msm_pcie_write_mask(dev->dm_core +  PCIE20_CAP_DEVCTRLSTATUS, 0,
						BIT(3)|BIT(2)|BIT(1)|BIT(0));

		PCIE_DBG(dev, "RC's PCIE20_CAP_DEVCTRLSTATUS:0x%x\n",
			readl_relaxed(dev->dm_core + PCIE20_CAP_DEVCTRLSTATUS));
	}

	/* configure SMMU registers */
	if (dev->smmu_exist) {
		msm_pcie_write_reg(dev->parf,
			PCIE20_PARF_BDF_TRANSLATE_CFG, 0);
		msm_pcie_write_reg(dev->parf,
			PCIE20_PARF_SID_OFFSET, 0);

		if (dev->enumerated) {
			for (i = 0; i < MAX_DEVICE_NUM; i++) {
				if (dev->pcidev_table[i].dev &&
					dev->pcidev_table[i].short_bdf) {
					msm_pcie_write_reg(dev->parf,
						PCIE20_PARF_BDF_TRANSLATE_N +
						dev->pcidev_table[i].short_bdf
						* 4,
						dev->pcidev_table[i].bdf >> 16);
				}
			}
		}
	}
}

static void msm_pcie_config_link_state(struct msm_pcie_dev_t *dev)
{
	u32 val;
	u32 current_offset;
	u32 ep_l1sub_ctrl1_offset = 0;
	u32 ep_l1sub_cap_reg1_offset = 0;
	u32 ep_link_cap_offset = 0;
	u32 ep_link_ctrlstts_offset = 0;
	u32 ep_dev_ctrl2stts2_offset = 0;

	/* Enable the AUX Clock and the Core Clk to be synchronous for L1SS*/
	if (!dev->aux_clk_sync && dev->l1ss_supported)
		msm_pcie_write_mask(dev->parf +
				PCIE20_PARF_SYS_CTRL, BIT(3), 0);

	current_offset = readl_relaxed(dev->conf + PCIE_CAP_PTR_OFFSET) & 0xff;

	while (current_offset) {
		if (msm_pcie_check_align(dev, current_offset))
			return;

		val = readl_relaxed(dev->conf + current_offset);
		if ((val & 0xff) == PCIE20_CAP_ID) {
			ep_link_cap_offset = current_offset + 0x0c;
			ep_link_ctrlstts_offset = current_offset + 0x10;
			ep_dev_ctrl2stts2_offset = current_offset + 0x28;
			break;
		}
		current_offset = (val >> 8) & 0xff;
	}

	if (!ep_link_cap_offset) {
		PCIE_DBG(dev,
			"RC%d endpoint does not support PCIe capability registers\n",
			dev->rc_idx);
		return;
	} else {
		PCIE_DBG(dev,
			"RC%d: ep_link_cap_offset: 0x%x\n",
			dev->rc_idx, ep_link_cap_offset);
	}

	if (dev->common_clk_en) {
		msm_pcie_write_mask(dev->dm_core + PCIE20_CAP_LINKCTRLSTATUS,
					0, BIT(6));

		msm_pcie_write_mask(dev->conf + ep_link_ctrlstts_offset,
					0, BIT(6));

		if (dev->shadow_en) {
			dev->rc_shadow[PCIE20_CAP_LINKCTRLSTATUS / 4] =
				readl_relaxed(dev->dm_core +
					PCIE20_CAP_LINKCTRLSTATUS);

			dev->ep_shadow[0][ep_link_ctrlstts_offset / 4] =
				readl_relaxed(dev->conf +
					ep_link_ctrlstts_offset);
		}

		PCIE_DBG2(dev, "RC's CAP_LINKCTRLSTATUS:0x%x\n",
			readl_relaxed(dev->dm_core +
			PCIE20_CAP_LINKCTRLSTATUS));
		PCIE_DBG2(dev, "EP's CAP_LINKCTRLSTATUS:0x%x\n",
			readl_relaxed(dev->conf + ep_link_ctrlstts_offset));
	}

	if (dev->clk_power_manage_en) {
		val = readl_relaxed(dev->conf + ep_link_cap_offset);
		if (val & BIT(18)) {
			msm_pcie_write_mask(dev->conf + ep_link_ctrlstts_offset,
						0, BIT(8));

			if (dev->shadow_en)
				dev->ep_shadow[0][ep_link_ctrlstts_offset / 4] =
					readl_relaxed(dev->conf +
						ep_link_ctrlstts_offset);

			PCIE_DBG2(dev, "EP's CAP_LINKCTRLSTATUS:0x%x\n",
				readl_relaxed(dev->conf +
					ep_link_ctrlstts_offset));
		}
	}

	if (dev->l0s_supported) {
		msm_pcie_write_mask(dev->dm_core + PCIE20_CAP_LINKCTRLSTATUS,
					0, BIT(0));
		msm_pcie_write_mask(dev->conf + ep_link_ctrlstts_offset,
					0, BIT(0));
		if (dev->shadow_en) {
			dev->rc_shadow[PCIE20_CAP_LINKCTRLSTATUS / 4] =
						readl_relaxed(dev->dm_core +
						PCIE20_CAP_LINKCTRLSTATUS);
			dev->ep_shadow[0][ep_link_ctrlstts_offset / 4] =
						readl_relaxed(dev->conf +
						ep_link_ctrlstts_offset);
		}
		PCIE_DBG2(dev, "RC's CAP_LINKCTRLSTATUS:0x%x\n",
			readl_relaxed(dev->dm_core +
			PCIE20_CAP_LINKCTRLSTATUS));
		PCIE_DBG2(dev, "EP's CAP_LINKCTRLSTATUS:0x%x\n",
			readl_relaxed(dev->conf + ep_link_ctrlstts_offset));
	}

	if (dev->l1_supported) {
		msm_pcie_write_mask(dev->dm_core + PCIE20_CAP_LINKCTRLSTATUS,
					0, BIT(1));
		msm_pcie_write_mask(dev->conf + ep_link_ctrlstts_offset,
					0, BIT(1));
		if (dev->shadow_en) {
			dev->rc_shadow[PCIE20_CAP_LINKCTRLSTATUS / 4] =
						readl_relaxed(dev->dm_core +
						PCIE20_CAP_LINKCTRLSTATUS);
			dev->ep_shadow[0][ep_link_ctrlstts_offset / 4] =
						readl_relaxed(dev->conf +
						ep_link_ctrlstts_offset);
		}
		PCIE_DBG2(dev, "RC's CAP_LINKCTRLSTATUS:0x%x\n",
			readl_relaxed(dev->dm_core +
			PCIE20_CAP_LINKCTRLSTATUS));
		PCIE_DBG2(dev, "EP's CAP_LINKCTRLSTATUS:0x%x\n",
			readl_relaxed(dev->conf + ep_link_ctrlstts_offset));
	}

	if (dev->l1ss_supported) {
		current_offset = PCIE_EXT_CAP_OFFSET;
		while (current_offset) {
			if (msm_pcie_check_align(dev, current_offset))
				return;

			val = readl_relaxed(dev->conf + current_offset);
			if ((val & 0xffff) == L1SUB_CAP_ID) {
				ep_l1sub_cap_reg1_offset = current_offset + 0x4;
				ep_l1sub_ctrl1_offset = current_offset + 0x8;
				break;
			}
			current_offset = val >> 20;
		}
		if (!ep_l1sub_ctrl1_offset) {
			PCIE_DBG(dev,
				"RC%d endpoint does not support l1ss registers\n",
				dev->rc_idx);
			return;
		}

		val = readl_relaxed(dev->conf + ep_l1sub_cap_reg1_offset);

		PCIE_DBG2(dev, "EP's L1SUB_CAPABILITY_REG_1: 0x%x\n", val);
		PCIE_DBG2(dev, "RC%d: ep_l1sub_ctrl1_offset: 0x%x\n",
				dev->rc_idx, ep_l1sub_ctrl1_offset);

		val &= 0xf;

		msm_pcie_write_reg_field(dev->dm_core, PCIE20_L1SUB_CONTROL1,
					0xf, val);
		msm_pcie_write_mask(dev->dm_core +
					PCIE20_DEVICE_CONTROL2_STATUS2,
					0, BIT(10));
		msm_pcie_write_reg_field(dev->conf, ep_l1sub_ctrl1_offset,
					0xf, val);
		msm_pcie_write_mask(dev->conf + ep_dev_ctrl2stts2_offset,
					0, BIT(10));
		if (dev->shadow_en) {
			dev->rc_shadow[PCIE20_L1SUB_CONTROL1 / 4] =
					readl_relaxed(dev->dm_core +
					PCIE20_L1SUB_CONTROL1);
			dev->rc_shadow[PCIE20_DEVICE_CONTROL2_STATUS2 / 4] =
					readl_relaxed(dev->dm_core +
					PCIE20_DEVICE_CONTROL2_STATUS2);
			dev->ep_shadow[0][ep_l1sub_ctrl1_offset / 4] =
					readl_relaxed(dev->conf +
					ep_l1sub_ctrl1_offset);
			dev->ep_shadow[0][ep_dev_ctrl2stts2_offset / 4] =
					readl_relaxed(dev->conf +
					ep_dev_ctrl2stts2_offset);
		}
		PCIE_DBG2(dev, "RC's L1SUB_CONTROL1:0x%x\n",
			readl_relaxed(dev->dm_core + PCIE20_L1SUB_CONTROL1));
		PCIE_DBG2(dev, "RC's DEVICE_CONTROL2_STATUS2:0x%x\n",
			readl_relaxed(dev->dm_core +
			PCIE20_DEVICE_CONTROL2_STATUS2));
		PCIE_DBG2(dev, "EP's L1SUB_CONTROL1:0x%x\n",
			readl_relaxed(dev->conf + ep_l1sub_ctrl1_offset));
		PCIE_DBG2(dev, "EP's DEVICE_CONTROL2_STATUS2:0x%x\n",
			readl_relaxed(dev->conf +
			ep_dev_ctrl2stts2_offset));
	}
}

void msm_pcie_config_msi_controller(struct msm_pcie_dev_t *dev)
{
	int i;

	PCIE_DBG(dev, "RC%d\n", dev->rc_idx);

	/* program MSI controller and enable all interrupts */
	writel_relaxed(MSM_PCIE_MSI_PHY, dev->dm_core + PCIE20_MSI_CTRL_ADDR);
	writel_relaxed(0, dev->dm_core + PCIE20_MSI_CTRL_UPPER_ADDR);

	for (i = 0; i < PCIE20_MSI_CTRL_MAX; i++)
		writel_relaxed(~0, dev->dm_core +
			       PCIE20_MSI_CTRL_INTR_EN + (i * 12));

	/* ensure that hardware is configured before proceeding */
	wmb();
}

static int msm_pcie_get_resources(struct msm_pcie_dev_t *dev,
					struct platform_device *pdev)
{
	int i, len, cnt, ret = 0, size = 0;
	struct msm_pcie_vreg_info_t *vreg_info;
	struct msm_pcie_gpio_info_t *gpio_info;
	struct msm_pcie_clk_info_t  *clk_info;
	struct resource *res;
	struct msm_pcie_res_info_t *res_info;
	struct msm_pcie_irq_info_t *irq_info;
	struct msm_pcie_irq_info_t *msi_info;
	char prop_name[MAX_PROP_SIZE];
	const __be32 *prop;
	u32 *clkfreq = NULL;

	PCIE_DBG(dev, "RC%d: entry\n", dev->rc_idx);

	cnt = of_property_count_strings((&pdev->dev)->of_node,
			"clock-names");
	if (cnt > 0) {
		clkfreq = kzalloc(cnt * sizeof(*clkfreq),
					GFP_KERNEL);
		if (!clkfreq) {
			PCIE_ERR(dev, "PCIe: memory alloc failed for RC%d\n",
					dev->rc_idx);
			return -ENOMEM;
		}
		ret = of_property_read_u32_array(
			(&pdev->dev)->of_node,
			"max-clock-frequency-hz", clkfreq, cnt);
		if (ret) {
			PCIE_ERR(dev,
				"PCIe: invalid max-clock-frequency-hz property for RC%d:%d\n",
				dev->rc_idx, ret);
			goto out;
		}
	}

	for (i = 0; i < MSM_PCIE_MAX_VREG; i++) {
		vreg_info = &dev->vreg[i];
		vreg_info->hdl =
				devm_regulator_get(&pdev->dev, vreg_info->name);

		if (PTR_ERR(vreg_info->hdl) == -EPROBE_DEFER) {
			PCIE_DBG(dev, "EPROBE_DEFER for VReg:%s\n",
				vreg_info->name);
			ret = PTR_ERR(vreg_info->hdl);
			goto out;
		}

		if (IS_ERR(vreg_info->hdl)) {
			if (vreg_info->required) {
				PCIE_DBG(dev, "Vreg %s doesn't exist\n",
					vreg_info->name);
				ret = PTR_ERR(vreg_info->hdl);
				goto out;
			} else {
				PCIE_DBG(dev,
					"Optional Vreg %s doesn't exist\n",
					vreg_info->name);
				vreg_info->hdl = NULL;
			}
		} else {
			dev->vreg_n++;
			snprintf(prop_name, MAX_PROP_SIZE,
				"qcom,%s-voltage-level", vreg_info->name);
			prop = of_get_property((&pdev->dev)->of_node,
						prop_name, &len);
			if (!prop || (len != (3 * sizeof(__be32)))) {
				PCIE_DBG(dev, "%s %s property\n",
					prop ? "invalid format" :
					"no", prop_name);
			} else {
				vreg_info->max_v = be32_to_cpup(&prop[0]);
				vreg_info->min_v = be32_to_cpup(&prop[1]);
				vreg_info->opt_mode =
					be32_to_cpup(&prop[2]);
			}
		}
	}

	dev->gdsc = devm_regulator_get(&pdev->dev, "gdsc-vdd");

	if (IS_ERR(dev->gdsc)) {
		PCIE_ERR(dev, "PCIe: RC%d Failed to get %s GDSC:%ld\n",
			dev->rc_idx, dev->pdev->name, PTR_ERR(dev->gdsc));
		if (PTR_ERR(dev->gdsc) == -EPROBE_DEFER)
			PCIE_DBG(dev, "PCIe: EPROBE_DEFER for %s GDSC\n",
					dev->pdev->name);
		ret = PTR_ERR(dev->gdsc);
		goto out;
	}

	dev->gdsc_smmu = devm_regulator_get(&pdev->dev, "gdsc-smmu");

	if (IS_ERR(dev->gdsc_smmu)) {
		PCIE_DBG(dev, "PCIe: RC%d SMMU GDSC does not exist",
			dev->rc_idx);
		dev->gdsc_smmu = NULL;
	}

	dev->gpio_n = 0;
	for (i = 0; i < MSM_PCIE_MAX_GPIO; i++) {
		gpio_info = &dev->gpio[i];
		ret = of_get_named_gpio((&pdev->dev)->of_node,
					gpio_info->name, 0);
		if (ret >= 0) {
			gpio_info->num = ret;
			dev->gpio_n++;
			PCIE_DBG(dev, "GPIO num for %s is %d\n",
				gpio_info->name, gpio_info->num);
		} else {
			if (gpio_info->required) {
				PCIE_ERR(dev,
					"Could not get required GPIO %s\n",
					gpio_info->name);
				goto out;
			} else {
				PCIE_DBG(dev,
					"Could not get optional GPIO %s\n",
					gpio_info->name);
			}
		}
		ret = 0;
	}

	of_get_property(pdev->dev.of_node, "qcom,phy-sequence", &size);
	if (size) {
		dev->phy_sequence = (struct msm_pcie_phy_info_t *)
			devm_kzalloc(&pdev->dev, size, GFP_KERNEL);

		if (dev->phy_sequence) {
			dev->phy_len =
				size / sizeof(*dev->phy_sequence);

			of_property_read_u32_array(pdev->dev.of_node,
				"qcom,phy-sequence",
				(unsigned int *)dev->phy_sequence,
				size / sizeof(dev->phy_sequence->offset));
		} else {
			PCIE_ERR(dev,
				"RC%d: Could not allocate memory for phy init sequence.\n",
				dev->rc_idx);
			ret = -ENOMEM;
			goto out;
		}
	} else {
		PCIE_DBG(dev, "RC%d: phy sequence is not present in DT\n",
			dev->rc_idx);
	}

	of_get_property(pdev->dev.of_node, "qcom,port-phy-sequence", &size);
	if (size) {
		dev->port_phy_sequence = (struct msm_pcie_phy_info_t *)
			devm_kzalloc(&pdev->dev, size, GFP_KERNEL);

		if (dev->port_phy_sequence) {
			dev->port_phy_len =
				size / sizeof(*dev->port_phy_sequence);

			of_property_read_u32_array(pdev->dev.of_node,
				"qcom,port-phy-sequence",
				(unsigned int *)dev->port_phy_sequence,
				size / sizeof(dev->port_phy_sequence->offset));
		} else {
			PCIE_ERR(dev,
				"RC%d: Could not allocate memory for port phy init sequence.\n",
				dev->rc_idx);
			ret = -ENOMEM;
			goto out;
		}
	} else {
		PCIE_DBG(dev, "RC%d: port phy sequence is not present in DT\n",
			dev->rc_idx);
	}

	for (i = 0; i < MSM_PCIE_MAX_CLK; i++) {
		clk_info = &dev->clk[i];

		clk_info->hdl = devm_clk_get(&pdev->dev, clk_info->name);

		if (IS_ERR(clk_info->hdl)) {
			if (clk_info->required) {
				PCIE_DBG(dev, "Clock %s isn't available:%ld\n",
				clk_info->name, PTR_ERR(clk_info->hdl));
				ret = PTR_ERR(clk_info->hdl);
				goto out;
			} else {
				PCIE_DBG(dev, "Ignoring Clock %s\n",
					clk_info->name);
				clk_info->hdl = NULL;
			}
		} else {
			if (clkfreq != NULL) {
				clk_info->freq = clkfreq[i +
					MSM_PCIE_MAX_PIPE_CLK];
				PCIE_DBG(dev, "Freq of Clock %s is:%d\n",
					clk_info->name, clk_info->freq);
			}
		}
	}

	for (i = 0; i < MSM_PCIE_MAX_PIPE_CLK; i++) {
		clk_info = &dev->pipeclk[i];

		clk_info->hdl = devm_clk_get(&pdev->dev, clk_info->name);

		if (IS_ERR(clk_info->hdl)) {
			if (clk_info->required) {
				PCIE_DBG(dev, "Clock %s isn't available:%ld\n",
				clk_info->name, PTR_ERR(clk_info->hdl));
				ret = PTR_ERR(clk_info->hdl);
				goto out;
			} else {
				PCIE_DBG(dev, "Ignoring Clock %s\n",
					clk_info->name);
				clk_info->hdl = NULL;
			}
		} else {
			if (clkfreq != NULL) {
				clk_info->freq = clkfreq[i];
				PCIE_DBG(dev, "Freq of Clock %s is:%d\n",
					clk_info->name, clk_info->freq);
			}
		}
	}


	dev->bus_scale_table = msm_bus_cl_get_pdata(pdev);
	if (!dev->bus_scale_table) {
		PCIE_DBG(dev, "PCIe: No bus scale table for RC%d (%s)\n",
			dev->rc_idx, dev->pdev->name);
		dev->bus_client = 0;
	} else {
		dev->bus_client =
			msm_bus_scale_register_client(dev->bus_scale_table);
		if (!dev->bus_client) {
			PCIE_ERR(dev,
				"PCIe: Failed to register bus client for RC%d (%s)\n",
				dev->rc_idx, dev->pdev->name);
			msm_bus_cl_clear_pdata(dev->bus_scale_table);
			ret = -ENODEV;
			goto out;
		}
	}

	for (i = 0; i < MSM_PCIE_MAX_RES; i++) {
		res_info = &dev->res[i];

		res = platform_get_resource_byname(pdev, IORESOURCE_MEM,
							   res_info->name);

		if (!res) {
			PCIE_ERR(dev, "PCIe: RC%d can't get %s resource.\n",
				dev->rc_idx, res_info->name);
		} else {
			PCIE_DBG(dev, "start addr for %s is %pa.\n",
				res_info->name,	&res->start);

			res_info->base = devm_ioremap(&pdev->dev,
						res->start, resource_size(res));
			if (!res_info->base) {
				PCIE_ERR(dev, "PCIe: RC%d can't remap %s.\n",
					dev->rc_idx, res_info->name);
				ret = -ENOMEM;
				goto out;
			} else {
				res_info->resource = res;
			}
		}
	}

	for (i = 0; i < MSM_PCIE_MAX_IRQ; i++) {
		irq_info = &dev->irq[i];

		res = platform_get_resource_byname(pdev, IORESOURCE_IRQ,
							   irq_info->name);

		if (!res) {
			PCIE_DBG(dev, "PCIe: RC%d can't find IRQ # for %s.\n",
				dev->rc_idx, irq_info->name);
		} else {
			irq_info->num = res->start;
			PCIE_DBG(dev, "IRQ # for %s is %d.\n", irq_info->name,
					irq_info->num);
		}
	}

	for (i = 0; i < MSM_PCIE_MAX_MSI; i++) {
		msi_info = &dev->msi[i];

		res = platform_get_resource_byname(pdev, IORESOURCE_IRQ,
							   msi_info->name);

		if (!res) {
			PCIE_DBG(dev, "PCIe: RC%d can't find IRQ # for %s.\n",
				dev->rc_idx, msi_info->name);
		} else {
			msi_info->num = res->start;
			PCIE_DBG(dev, "IRQ # for %s is %d.\n", msi_info->name,
					msi_info->num);
		}
	}

	/* All allocations succeeded */

	if (dev->gpio[MSM_PCIE_GPIO_WAKE].num)
		dev->wake_n = gpio_to_irq(dev->gpio[MSM_PCIE_GPIO_WAKE].num);
	else
		dev->wake_n = 0;

	dev->parf = dev->res[MSM_PCIE_RES_PARF].base;
	dev->phy = dev->res[MSM_PCIE_RES_PHY].base;
	dev->elbi = dev->res[MSM_PCIE_RES_ELBI].base;
	dev->dm_core = dev->res[MSM_PCIE_RES_DM_CORE].base;
	dev->conf = dev->res[MSM_PCIE_RES_CONF].base;
	dev->bars = dev->res[MSM_PCIE_RES_BARS].base;
	dev->tcsr = dev->res[MSM_PCIE_RES_TCSR].base;
	dev->dev_mem_res = dev->res[MSM_PCIE_RES_BARS].resource;
	dev->dev_io_res = dev->res[MSM_PCIE_RES_IO].resource;
	dev->dev_io_res->flags = IORESOURCE_IO;

out:
	kfree(clkfreq);

	PCIE_DBG(dev, "RC%d: exit\n", dev->rc_idx);

	return ret;
}

static void msm_pcie_release_resources(struct msm_pcie_dev_t *dev)
{
	dev->parf = NULL;
	dev->elbi = NULL;
	dev->dm_core = NULL;
	dev->conf = NULL;
	dev->bars = NULL;
	dev->tcsr = NULL;
	dev->dev_mem_res = NULL;
	dev->dev_io_res = NULL;
}

int msm_pcie_enable(struct msm_pcie_dev_t *dev, u32 options)
{
	int ret = 0;
	uint32_t val;
	long int retries = 0;
	int link_check_count = 0;

	PCIE_DBG(dev, "RC%d: entry\n", dev->rc_idx);

	mutex_lock(&dev->setup_lock);

	if (dev->link_status == MSM_PCIE_LINK_ENABLED) {
		PCIE_ERR(dev, "PCIe: the link of RC%d is already enabled\n",
			dev->rc_idx);
		goto out;
	}

	/* assert PCIe reset link to keep EP in reset */

	PCIE_INFO(dev, "PCIe: Assert the reset of endpoint of RC%d.\n",
		dev->rc_idx);
	gpio_set_value(dev->gpio[MSM_PCIE_GPIO_PERST].num,
				dev->gpio[MSM_PCIE_GPIO_PERST].on);
	usleep_range(PERST_PROPAGATION_DELAY_US_MIN,
				 PERST_PROPAGATION_DELAY_US_MAX);

	/* enable power */

	if (options & PM_VREG) {
		ret = msm_pcie_vreg_init(dev);
		if (ret)
			goto out;
	}

	/* enable clocks */
	if (options & PM_CLK) {
		ret = msm_pcie_clk_init(dev);
		wmb();
		if (ret)
			goto clk_fail;
	}

	if (dev->scm_dev_id) {
		PCIE_DBG(dev, "RC%d: restoring sec config\n", dev->rc_idx);
		msm_pcie_restore_sec_config(dev);
	}

	/* enable PCIe clocks and resets */
	msm_pcie_write_mask(dev->parf + PCIE20_PARF_PHY_CTRL, BIT(0), 0);

	/* change DBI base address */
	writel_relaxed(0, dev->parf + PCIE20_PARF_DBI_BASE_ADDR);

	writel_relaxed(0x365E, dev->parf + PCIE20_PARF_SYS_CTRL);

	msm_pcie_write_mask(dev->parf + PCIE20_PARF_MHI_CLOCK_RESET_CTRL,
				0, BIT(4));

	/* enable selected IRQ */
	if (dev->irq[MSM_PCIE_INT_GLOBAL_INT].num) {
		msm_pcie_write_reg(dev->parf, PCIE20_PARF_INT_ALL_MASK, 0);

		msm_pcie_write_mask(dev->parf + PCIE20_PARF_INT_ALL_MASK, 0,
					BIT(MSM_PCIE_INT_EVT_LINK_DOWN) |
					BIT(MSM_PCIE_INT_EVT_AER_LEGACY) |
					BIT(MSM_PCIE_INT_EVT_AER_ERR) |
					BIT(MSM_PCIE_INT_EVT_MSI_0) |
					BIT(MSM_PCIE_INT_EVT_MSI_1) |
					BIT(MSM_PCIE_INT_EVT_MSI_2) |
					BIT(MSM_PCIE_INT_EVT_MSI_3) |
					BIT(MSM_PCIE_INT_EVT_MSI_4) |
					BIT(MSM_PCIE_INT_EVT_MSI_5) |
					BIT(MSM_PCIE_INT_EVT_MSI_6) |
					BIT(MSM_PCIE_INT_EVT_MSI_7));

		PCIE_DBG(dev, "PCIe: RC%d: PCIE20_PARF_INT_ALL_MASK: 0x%x\n",
			dev->rc_idx,
			readl_relaxed(dev->parf + PCIE20_PARF_INT_ALL_MASK));
	}

	if (dev->dev_mem_res->end - dev->dev_mem_res->start > SZ_16M)
		writel_relaxed(SZ_32M, dev->parf +
			PCIE20_PARF_SLV_ADDR_SPACE_SIZE);
	else if (dev->dev_mem_res->end - dev->dev_mem_res->start > SZ_8M)
		writel_relaxed(SZ_16M, dev->parf +
			PCIE20_PARF_SLV_ADDR_SPACE_SIZE);
	else
		writel_relaxed(SZ_8M, dev->parf +
			PCIE20_PARF_SLV_ADDR_SPACE_SIZE);

	if (dev->use_msi) {
		PCIE_DBG(dev, "RC%d: enable WR halt.\n", dev->rc_idx);
		msm_pcie_write_mask(dev->parf +
			PCIE20_PARF_AXI_MSTR_WR_ADDR_HALT, 0, BIT(31));
	}

	mutex_lock(&com_phy_lock);
	/* init PCIe PHY */
	if (!num_rc_on)
		pcie_phy_init(dev);

	num_rc_on++;
	mutex_unlock(&com_phy_lock);

	if (options & PM_PIPE_CLK) {
		usleep_range(PHY_STABILIZATION_DELAY_US_MIN,
					 PHY_STABILIZATION_DELAY_US_MAX);
		/* Enable the pipe clock */
		ret = msm_pcie_pipe_clk_init(dev);
		wmb();
		if (ret)
			goto link_fail;
	}

	PCIE_DBG(dev, "RC%d: waiting for phy ready...\n", dev->rc_idx);

	do {
		if (pcie_phy_is_ready(dev))
			break;
		retries++;
		usleep_range(REFCLK_STABILIZATION_DELAY_US_MIN,
					 REFCLK_STABILIZATION_DELAY_US_MAX);
	} while (retries < PHY_READY_TIMEOUT_COUNT);

	PCIE_DBG(dev, "RC%d: number of PHY retries:%ld.\n",
		dev->rc_idx, retries);

	if (pcie_phy_is_ready(dev))
		PCIE_INFO(dev, "PCIe RC%d PHY is ready!\n", dev->rc_idx);
	else {
		PCIE_ERR(dev, "PCIe PHY RC%d failed to come up!\n",
			dev->rc_idx);
		ret = -ENODEV;
		pcie_phy_dump(dev);
		goto link_fail;
	}

	pcie_pcs_port_phy_init(dev);

	if (dev->ep_latency)
		usleep_range(dev->ep_latency * 1000, dev->ep_latency * 1000);

	if (dev->gpio[MSM_PCIE_GPIO_EP].num)
		gpio_set_value(dev->gpio[MSM_PCIE_GPIO_EP].num,
				dev->gpio[MSM_PCIE_GPIO_EP].on);

	/* de-assert PCIe reset link to bring EP out of reset */

	PCIE_INFO(dev, "PCIe: Release the reset of endpoint of RC%d.\n",
		dev->rc_idx);
	gpio_set_value(dev->gpio[MSM_PCIE_GPIO_PERST].num,
				1 - dev->gpio[MSM_PCIE_GPIO_PERST].on);
	usleep_range(dev->perst_delay_us_min, dev->perst_delay_us_max);

	/* set max tlp read size */
	msm_pcie_write_reg_field(dev->dm_core, PCIE20_DEVICE_CONTROL_STATUS,
				0x7000, dev->tlp_rd_size);

	/* enable link training */
	msm_pcie_write_mask(dev->parf + PCIE20_PARF_LTSSM, 0, BIT(8));

	PCIE_DBG(dev, "%s", "check if link is up\n");

	/* Wait for up to 100ms for the link to come up */
	do {
		usleep_range(LINK_UP_TIMEOUT_US_MIN, LINK_UP_TIMEOUT_US_MAX);
		val =  readl_relaxed(dev->elbi + PCIE20_ELBI_SYS_STTS);
	} while ((!(val & XMLH_LINK_UP) ||
		!msm_pcie_confirm_linkup(dev, false, false))
		&& (link_check_count++ < LINK_UP_CHECK_MAX_COUNT));

	if ((val & XMLH_LINK_UP) &&
		msm_pcie_confirm_linkup(dev, false, false)) {
		PCIE_DBG(dev, "Link is up after %d checkings\n",
			link_check_count);
		PCIE_INFO(dev, "PCIe RC%d link initialized\n", dev->rc_idx);
	} else {
		PCIE_INFO(dev, "PCIe: Assert the reset of endpoint of RC%d.\n",
			dev->rc_idx);
		gpio_set_value(dev->gpio[MSM_PCIE_GPIO_PERST].num,
			dev->gpio[MSM_PCIE_GPIO_PERST].on);
		PCIE_ERR(dev, "PCIe RC%d link initialization failed\n",
			dev->rc_idx);
		ret = -1;
		goto link_fail;
	}

	msm_pcie_config_controller(dev);

	if (!dev->msi_gicm_addr)
		msm_pcie_config_msi_controller(dev);

	msm_pcie_config_link_state(dev);

	dev->link_status = MSM_PCIE_LINK_ENABLED;
	dev->power_on = true;
	dev->suspending = false;
	dev->link_turned_on_counter++;

	if (dev->bridge_found && dev->ep_latency) {
		PCIE_DBG(dev, "PCIe RC%d add ref clk propagation latency.\n",
			dev->rc_idx);
		usleep_range(dev->ep_latency * 1000, dev->ep_latency * 1000);
	}

	goto out;

link_fail:
	if (dev->gpio[MSM_PCIE_GPIO_EP].num)
		gpio_set_value(dev->gpio[MSM_PCIE_GPIO_EP].num,
				1 - dev->gpio[MSM_PCIE_GPIO_EP].on);
	msm_pcie_write_reg(dev->phy,
		PCIE_N_SW_RESET(dev->rc_idx, dev->common_phy), 0x1);
	msm_pcie_write_reg(dev->phy,
		PCIE_N_POWER_DOWN_CONTROL(dev->rc_idx, dev->common_phy), 0);

	mutex_lock(&com_phy_lock);
	num_rc_on--;
	if (!num_rc_on && dev->common_phy) {
		PCIE_DBG(dev, "PCIe: RC%d is powering down the common phy\n",
			dev->rc_idx);
		msm_pcie_write_reg(dev->phy, PCIE_COM_SW_RESET, 0x1);
		msm_pcie_write_reg(dev->phy, PCIE_COM_POWER_DOWN_CONTROL, 0);
	}
	mutex_unlock(&com_phy_lock);

	msm_pcie_pipe_clk_deinit(dev);
	msm_pcie_clk_deinit(dev);
clk_fail:
	msm_pcie_vreg_deinit(dev);
out:
	mutex_unlock(&dev->setup_lock);

	PCIE_DBG(dev, "RC%d: exit\n", dev->rc_idx);

	return ret;
}

void msm_pcie_disable(struct msm_pcie_dev_t *dev, u32 options)
{
	PCIE_DBG(dev, "RC%d: entry\n", dev->rc_idx);

	mutex_lock(&dev->setup_lock);

	if (!dev->power_on) {
		PCIE_DBG(dev,
			"PCIe: the link of RC%d is already power down.\n",
			dev->rc_idx);
		mutex_unlock(&dev->setup_lock);
		return;
	}

	dev->link_status = MSM_PCIE_LINK_DISABLED;
	dev->power_on = false;
	dev->link_turned_off_counter++;

	PCIE_INFO(dev, "PCIe: Assert the reset of endpoint of RC%d.\n",
		dev->rc_idx);

	gpio_set_value(dev->gpio[MSM_PCIE_GPIO_PERST].num,
				dev->gpio[MSM_PCIE_GPIO_PERST].on);

	msm_pcie_write_reg(dev->phy,
		PCIE_N_SW_RESET(dev->rc_idx, dev->common_phy), 0x1);
	msm_pcie_write_reg(dev->phy,
		PCIE_N_POWER_DOWN_CONTROL(dev->rc_idx, dev->common_phy), 0);

	mutex_lock(&com_phy_lock);
	num_rc_on--;
	if (!num_rc_on && dev->common_phy) {
		PCIE_DBG(dev, "PCIe: RC%d is powering down the common phy\n",
			dev->rc_idx);
		msm_pcie_write_reg(dev->phy, PCIE_COM_SW_RESET, 0x1);
		msm_pcie_write_reg(dev->phy, PCIE_COM_POWER_DOWN_CONTROL, 0);
	}
	mutex_unlock(&com_phy_lock);

	if (options & PM_CLK) {
		msm_pcie_write_mask(dev->parf + PCIE20_PARF_PHY_CTRL, 0,
					BIT(0));
		msm_pcie_clk_deinit(dev);
	}

	if (options & PM_VREG)
		msm_pcie_vreg_deinit(dev);

	if (options & PM_PIPE_CLK)
		msm_pcie_pipe_clk_deinit(dev);

	if (dev->gpio[MSM_PCIE_GPIO_EP].num)
		gpio_set_value(dev->gpio[MSM_PCIE_GPIO_EP].num,
				1 - dev->gpio[MSM_PCIE_GPIO_EP].on);

	mutex_unlock(&dev->setup_lock);

	PCIE_DBG(dev, "RC%d: exit\n", dev->rc_idx);
}

static void msm_pcie_config_ep_aer(struct msm_pcie_dev_t *dev,
				struct msm_pcie_device_info *ep_dev_info)
{
	u32 val;
	void __iomem *ep_base = ep_dev_info->conf_base;
	u32 current_offset = readl_relaxed(ep_base + PCIE_CAP_PTR_OFFSET) &
						0xff;

	while (current_offset) {
		if (msm_pcie_check_align(dev, current_offset))
			return;

		val = readl_relaxed(ep_base + current_offset);
		if ((val & 0xff) == PCIE20_CAP_ID) {
			ep_dev_info->dev_ctrlstts_offset =
				current_offset + 0x8;
			break;
		}
		current_offset = (val >> 8) & 0xff;
	}

	if (!ep_dev_info->dev_ctrlstts_offset) {
		PCIE_DBG(dev,
			"RC%d endpoint does not support PCIe cap registers\n",
			dev->rc_idx);
		return;
	}

	PCIE_DBG2(dev, "RC%d: EP dev_ctrlstts_offset: 0x%x\n",
		dev->rc_idx, ep_dev_info->dev_ctrlstts_offset);

	/* Enable AER on EP */
	msm_pcie_write_mask(ep_base + ep_dev_info->dev_ctrlstts_offset, 0,
				BIT(3)|BIT(2)|BIT(1)|BIT(0));

	PCIE_DBG(dev, "EP's PCIE20_CAP_DEVCTRLSTATUS:0x%x\n",
		readl_relaxed(ep_base + ep_dev_info->dev_ctrlstts_offset));
}

static int msm_pcie_config_device_table(struct device *dev, void *pdev)
{
	struct pci_dev *pcidev = to_pci_dev(dev);
	struct msm_pcie_dev_t *pcie_dev = (struct msm_pcie_dev_t *) pdev;
	struct msm_pcie_device_info *dev_table_t = pcie_dev->pcidev_table;
	struct resource *axi_conf = pcie_dev->res[MSM_PCIE_RES_CONF].resource;
	int ret = 0;
	u32 rc_idx = pcie_dev->rc_idx;
	u32 i, index;
	u32 bdf = 0;
	u8 type;
	u32 h_type;
	u32 bme;

	if (!pcidev) {
		PCIE_ERR(pcie_dev,
			"PCIe: Did not find PCI device in list for RC%d.\n",
			pcie_dev->rc_idx);
		return -ENODEV;
	} else {
		PCIE_DBG(pcie_dev,
			"PCI device found: vendor-id:0x%x device-id:0x%x\n",
			pcidev->vendor, pcidev->device);
	}

	if (!pcidev->bus->number)
		return ret;

	bdf = BDF_OFFSET(pcidev->bus->number, pcidev->devfn);
	type = pcidev->bus->number == 1 ?
		PCIE20_CTRL1_TYPE_CFG0 : PCIE20_CTRL1_TYPE_CFG1;

	for (i = 0; i < (MAX_RC_NUM * MAX_DEVICE_NUM); i++) {
		if (msm_pcie_dev_tbl[i].bdf == bdf &&
			!msm_pcie_dev_tbl[i].dev) {
			for (index = 0; index < MAX_DEVICE_NUM; index++) {
				if (dev_table_t[index].bdf == bdf) {
					msm_pcie_dev_tbl[i].dev = pcidev;
					msm_pcie_dev_tbl[i].domain = rc_idx;
					msm_pcie_dev_tbl[i].conf_base =
						pcie_dev->conf + index * SZ_4K;
					msm_pcie_dev_tbl[i].phy_address =
						axi_conf->start + index * SZ_4K;

					dev_table_t[index].dev = pcidev;
					dev_table_t[index].domain = rc_idx;
					dev_table_t[index].conf_base =
						pcie_dev->conf + index * SZ_4K;
					dev_table_t[index].phy_address =
						axi_conf->start + index * SZ_4K;

					msm_pcie_iatu_config(pcie_dev, index,
						type,
						dev_table_t[index].phy_address,
						dev_table_t[index].phy_address
						+ SZ_4K - 1,
						bdf);

					h_type = readl_relaxed(
						dev_table_t[index].conf_base +
						PCIE20_HEADER_TYPE);

					bme = readl_relaxed(
						dev_table_t[index].conf_base +
						PCIE20_COMMAND_STATUS);

					if (h_type & (1 << 16)) {
						pci_write_config_dword(pcidev,
							PCIE20_COMMAND_STATUS,
							bme | 0x06);
					} else {
						pcie_dev->num_ep++;
						dev_table_t[index].registered =
							false;
					}

					if (pcie_dev->num_ep > 1)
						pcie_dev->pending_ep_reg = true;

					msm_pcie_config_ep_aer(pcie_dev,
						&dev_table_t[index]);

					break;
				}
			}
			if (index == MAX_DEVICE_NUM) {
				PCIE_ERR(pcie_dev,
					"RC%d PCI device table is full.\n",
					rc_idx);
				ret = index;
			} else {
				break;
			}
		} else if (msm_pcie_dev_tbl[i].bdf == bdf &&
			pcidev == msm_pcie_dev_tbl[i].dev) {
			break;
		}
	}
	if (i == MAX_RC_NUM * MAX_DEVICE_NUM) {
		PCIE_ERR(pcie_dev,
			"Global PCI device table is full: %d elements.\n",
			i);
		PCIE_ERR(pcie_dev,
			"Bus number is 0x%x\nDevice number is 0x%x\n",
			pcidev->bus->number, pcidev->devfn);
		ret = i;
	}
	return ret;
}

int msm_pcie_configure_sid(struct device *dev, u32 *sid, int *domain)
{
	struct pci_dev *pcidev;
	struct msm_pcie_dev_t *pcie_dev;
	struct pci_bus *bus;
	int i;
	u32 bdf;

	if (!dev) {
		pr_err("%s: PCIe: endpoint device passed in is NULL\n",
			__func__);
		return MSM_PCIE_ERROR;
	}

	pcidev = to_pci_dev(dev);
	if (!pcidev) {
		pr_err("%s: PCIe: PCI device of endpoint is NULL\n",
			__func__);
		return MSM_PCIE_ERROR;
	}

	bus = pcidev->bus;
	if (!bus) {
		pr_err("%s: PCIe: Bus of PCI device is NULL\n",
			__func__);
		return MSM_PCIE_ERROR;
	}

	while (!pci_is_root_bus(bus))
		bus = bus->parent;

	pcie_dev = (struct msm_pcie_dev_t *)(bus->sysdata);
	if (!pcie_dev) {
		pr_err("%s: PCIe: Could not get PCIe structure\n",
			__func__);
		return MSM_PCIE_ERROR;
	}

	if (!pcie_dev->smmu_exist) {
		PCIE_DBG(pcie_dev,
			"PCIe: RC:%d: smmu does not exist\n",
			pcie_dev->rc_idx);
		return MSM_PCIE_ERROR;
	}

	PCIE_DBG(pcie_dev, "PCIe: RC%d: device address is: %p\n",
		pcie_dev->rc_idx, dev);
	PCIE_DBG(pcie_dev, "PCIe: RC%d: PCI device address is: %p\n",
		pcie_dev->rc_idx, pcidev);

	*domain = pcie_dev->rc_idx;

	if (pcie_dev->current_short_bdf < (MAX_SHORT_BDF_NUM - 1)) {
		pcie_dev->current_short_bdf++;
	} else {
		PCIE_ERR(pcie_dev,
			"PCIe: RC%d: No more short BDF left\n",
			pcie_dev->rc_idx);
		return MSM_PCIE_ERROR;
	}

	bdf = BDF_OFFSET(pcidev->bus->number, pcidev->devfn);

	for (i = 0; i < MAX_DEVICE_NUM; i++) {
		if (pcie_dev->pcidev_table[i].bdf == bdf) {
			*sid = (pcie_dev->rc_idx << 4) |
				pcie_dev->current_short_bdf;

			msm_pcie_write_reg(pcie_dev->parf,
				PCIE20_PARF_BDF_TRANSLATE_N +
				pcie_dev->current_short_bdf * 4,
				bdf >> 16);

			pcie_dev->pcidev_table[i].sid = *sid;
			pcie_dev->pcidev_table[i].short_bdf =
				pcie_dev->current_short_bdf;
			break;
		}
	}

	if (i == MAX_DEVICE_NUM) {
		pcie_dev->current_short_bdf--;
		PCIE_ERR(pcie_dev,
			"PCIe: RC%d could not find BDF:%d\n",
			pcie_dev->rc_idx, bdf);
		return MSM_PCIE_ERROR;
	}

	PCIE_DBG(pcie_dev,
		"PCIe: RC%d: Device: %02x:%02x.%01x received SID %d\n",
		pcie_dev->rc_idx,
		bdf >> 24,
		bdf >> 19 & 0x1f,
		bdf >> 16 & 0x07,
		*sid);

	return 0;
}
EXPORT_SYMBOL(msm_pcie_configure_sid);

int msm_pcie_enumerate(u32 rc_idx)
{
	int ret = 0, bus_ret = 0, scan_ret = 0;
	struct msm_pcie_dev_t *dev = &msm_pcie_dev[rc_idx];

	PCIE_DBG(dev, "Enumerate RC%d\n", rc_idx);

	if (!dev->drv_ready) {
		PCIE_DBG(dev, "RC%d has not been successfully probed yet\n",
			rc_idx);
		return -EPROBE_DEFER;
	}

	if (!dev->enumerated) {
		ret = msm_pcie_enable(dev, PM_ALL);

		/* kick start ARM PCI configuration framework */
		if (!ret) {
			struct pci_dev *pcidev = NULL;
			bool found = false;
			struct pci_bus *bus;
			resource_size_t iobase = 0;
			u32 ids = readl_relaxed(msm_pcie_dev[rc_idx].dm_core);
			u32 vendor_id = ids & 0xffff;
			u32 device_id = (ids & 0xffff0000) >> 16;
			LIST_HEAD(res);

			PCIE_DBG(dev, "vendor-id:0x%x device_id:0x%x\n",
					vendor_id, device_id);

			ret = of_pci_get_host_bridge_resources(
						dev->pdev->dev.of_node,
						0, 0xff, &res, &iobase);
			if (ret) {
				PCIE_ERR(dev,
					"PCIe: failed to get host bridge resources for RC%d: %d\n",
					dev->rc_idx, ret);

				return ret;
			}

			if (dev->ep_latency) {
				PCIE_DBG(dev,
					"PCIe RC%d add ref clk propagation latency.\n",
					dev->rc_idx);
				usleep_range(dev->ep_latency * 1000,
					dev->ep_latency * 1000);
			}

			bus = pci_create_root_bus(&dev->pdev->dev, 0,
						&msm_pcie_ops,
						msm_pcie_setup_sys_data(dev),
						&res);
			if (!bus) {
				PCIE_ERR(dev,
					"PCIe: failed to create root bus for RC%d\n",
					dev->rc_idx);

				return -ENOMEM;
			}

			scan_ret = pci_scan_child_bus(bus);
			PCIE_DBG(dev,
				"PCIe: RC%d: The max subordinate bus number discovered is %d\n",
				dev->rc_idx, ret);

			msm_pcie_fixup_irqs(dev);
			pci_assign_unassigned_bus_resources(bus);
			pci_bus_add_devices(bus);

			dev->enumerated = true;

			msm_pcie_write_mask(dev->dm_core +
				PCIE20_COMMAND_STATUS, 0, BIT(2)|BIT(1));

			if (dev->cpl_timeout && dev->bridge_found)
				msm_pcie_write_reg_field(dev->dm_core,
					PCIE20_DEVICE_CONTROL2_STATUS2,
					0xf, dev->cpl_timeout);

			if (dev->shadow_en) {
				u32 val = readl_relaxed(dev->dm_core +
						PCIE20_COMMAND_STATUS);
				PCIE_DBG(dev, "PCIE20_COMMAND_STATUS:0x%x\n",
					val);
				dev->rc_shadow[PCIE20_COMMAND_STATUS / 4] = val;
			}

			do {
				pcidev = pci_get_device(vendor_id,
					device_id, pcidev);
				if (pcidev && (&msm_pcie_dev[rc_idx] ==
					(struct msm_pcie_dev_t *)
					PCIE_BUS_PRIV_DATA(pcidev->bus))) {
					msm_pcie_dev[rc_idx].dev = pcidev;
					found = true;
					PCIE_DBG(&msm_pcie_dev[rc_idx],
						"PCI device is found for RC%d\n",
						rc_idx);
				}
			} while (!found && pcidev);

			if (!pcidev) {
				PCIE_ERR(dev,
					"PCIe: Did not find PCI device for RC%d.\n",
					dev->rc_idx);
				return -ENODEV;
			}

			bus_ret = bus_for_each_dev(&pci_bus_type, NULL, dev,
					&msm_pcie_config_device_table);

			if (bus_ret) {
				PCIE_ERR(dev,
					"PCIe: Failed to set up device table for RC%d\n",
					dev->rc_idx);
				return -ENODEV;
			}
		} else {
			PCIE_ERR(dev, "PCIe: failed to enable RC%d.\n",
				dev->rc_idx);
		}
	} else {
		PCIE_ERR(dev, "PCIe: RC%d has already been enumerated.\n",
			dev->rc_idx);
	}

	return ret;
}
EXPORT_SYMBOL(msm_pcie_enumerate);

static void msm_pcie_notify_client(struct msm_pcie_dev_t *dev,
					enum msm_pcie_event event)
{
	if (dev->event_reg && dev->event_reg->callback &&
		(dev->event_reg->events & event)) {
		struct msm_pcie_notify *notify = &dev->event_reg->notify;
		notify->event = event;
		notify->user = dev->event_reg->user;
		PCIE_DBG(dev, "PCIe: callback RC%d for event %d\n",
			dev->rc_idx, event);
		dev->event_reg->callback(notify);

		if ((dev->event_reg->options & MSM_PCIE_CONFIG_NO_RECOVERY) &&
				(event == MSM_PCIE_EVENT_LINKDOWN)) {
			dev->user_suspend = true;
			PCIE_DBG(dev,
				"PCIe: Client of RC%d will recover the link later.\n",
				dev->rc_idx);
			return;
		}
	} else {
		PCIE_DBG2(dev,
			"PCIe: Client of RC%d does not have registration for event %d\n",
			dev->rc_idx, event);
	}
}

static void handle_wake_func(struct work_struct *work)
{
	int i, ret;
	struct msm_pcie_dev_t *dev = container_of(work, struct msm_pcie_dev_t,
					handle_wake_work);

	PCIE_DBG(dev, "PCIe: Wake work for RC%d\n", dev->rc_idx);

	mutex_lock(&dev->recovery_lock);

	if (!dev->enumerated) {
		PCIE_DBG(dev,
			"PCIe: Start enumeration for RC%d upon the wake from endpoint.\n",
			dev->rc_idx);

		ret = msm_pcie_enumerate(dev->rc_idx);
		if (ret) {
			PCIE_ERR(dev,
				"PCIe: failed to enable RC%d upon wake request from the device.\n",
				dev->rc_idx);
			goto out;
		}

		if (dev->num_ep > 1) {
			for (i = 0; i < MAX_DEVICE_NUM; i++) {
				dev->event_reg = dev->pcidev_table[i].event_reg;

				if ((dev->link_status == MSM_PCIE_LINK_ENABLED)
					&& dev->event_reg &&
					dev->event_reg->callback &&
					(dev->event_reg->events &
					MSM_PCIE_EVENT_LINKUP)) {
					struct msm_pcie_notify *notify =
						&dev->event_reg->notify;
					notify->event = MSM_PCIE_EVENT_LINKUP;
					notify->user = dev->event_reg->user;
					PCIE_DBG(dev,
						"PCIe: Linkup callback for RC%d after enumeration is successful in wake IRQ handling\n",
						dev->rc_idx);
					dev->event_reg->callback(notify);
				}
			}
		} else {
			if ((dev->link_status == MSM_PCIE_LINK_ENABLED) &&
				dev->event_reg && dev->event_reg->callback &&
				(dev->event_reg->events &
				MSM_PCIE_EVENT_LINKUP)) {
				struct msm_pcie_notify *notify =
						&dev->event_reg->notify;
				notify->event = MSM_PCIE_EVENT_LINKUP;
				notify->user = dev->event_reg->user;
				PCIE_DBG(dev,
					"PCIe: Linkup callback for RC%d after enumeration is successful in wake IRQ handling\n",
					dev->rc_idx);
				dev->event_reg->callback(notify);
			} else {
				PCIE_DBG(dev,
					"PCIe: Client of RC%d does not have registration for linkup event.\n",
					dev->rc_idx);
			}
		}
		goto out;
	} else {
		PCIE_ERR(dev,
			"PCIe: The enumeration for RC%d has already been done.\n",
			dev->rc_idx);
		goto out;
	}

out:
	mutex_unlock(&dev->recovery_lock);
}

static irqreturn_t handle_aer_irq(int irq, void *data)
{
	struct msm_pcie_dev_t *dev = data;

	int corr_val = 0, uncorr_val = 0, rc_err_status = 0;
	int ep_corr_val = 0, ep_uncorr_val = 0;
	int rc_dev_ctrlstts = 0, ep_dev_ctrlstts = 0;
	u32 ep_dev_ctrlstts_offset = 0;
	int i, j, ep_src_bdf = 0;
	void __iomem *ep_base = NULL;
	unsigned long irqsave_flags;

	PCIE_DBG2(dev,
		"AER Interrupt handler fired for RC%d irq %d\nrc_corr_counter: %lu\nrc_non_fatal_counter: %lu\nrc_fatal_counter: %lu\nep_corr_counter: %lu\nep_non_fatal_counter: %lu\nep_fatal_counter: %lu\n",
		dev->rc_idx, irq, dev->rc_corr_counter,
		dev->rc_non_fatal_counter, dev->rc_fatal_counter,
		dev->ep_corr_counter, dev->ep_non_fatal_counter,
		dev->ep_fatal_counter);

	spin_lock_irqsave(&dev->aer_lock, irqsave_flags);

	if (dev->suspending) {
		PCIE_DBG2(dev,
			"PCIe: RC%d is currently suspending.\n",
			dev->rc_idx);
		spin_unlock_irqrestore(&dev->aer_lock, irqsave_flags);
		return IRQ_HANDLED;
	}

	uncorr_val = readl_relaxed(dev->dm_core +
				PCIE20_AER_UNCORR_ERR_STATUS_REG);
	corr_val = readl_relaxed(dev->dm_core +
				PCIE20_AER_CORR_ERR_STATUS_REG);
	rc_err_status = readl_relaxed(dev->dm_core +
				PCIE20_AER_ROOT_ERR_STATUS_REG);
	rc_dev_ctrlstts = readl_relaxed(dev->dm_core +
				PCIE20_CAP_DEVCTRLSTATUS);

	if (uncorr_val)
		PCIE_DBG(dev, "RC's PCIE20_AER_UNCORR_ERR_STATUS_REG:0x%x\n",
				uncorr_val);
	if (corr_val && (dev->rc_corr_counter < corr_counter_limit))
		PCIE_DBG(dev, "RC's PCIE20_AER_CORR_ERR_STATUS_REG:0x%x\n",
				corr_val);

	if ((rc_dev_ctrlstts >> 18) & 0x1)
		dev->rc_fatal_counter++;
	if ((rc_dev_ctrlstts >> 17) & 0x1)
		dev->rc_non_fatal_counter++;
	if ((rc_dev_ctrlstts >> 16) & 0x1)
		dev->rc_corr_counter++;

	msm_pcie_write_mask(dev->dm_core + PCIE20_CAP_DEVCTRLSTATUS, 0,
				BIT(18)|BIT(17)|BIT(16));

	if (dev->link_status == MSM_PCIE_LINK_DISABLED) {
		PCIE_DBG2(dev, "RC%d link is down\n", dev->rc_idx);
		goto out;
	}

	for (i = 0; i < 2; i++) {
		if (i)
			ep_src_bdf = readl_relaxed(dev->dm_core +
				PCIE20_AER_ERR_SRC_ID_REG) & ~0xffff;
		else
			ep_src_bdf = (readl_relaxed(dev->dm_core +
				PCIE20_AER_ERR_SRC_ID_REG) & 0xffff) << 16;

		if (!ep_src_bdf)
			continue;

		for (j = 0; j < MAX_DEVICE_NUM; j++) {
			if (ep_src_bdf == dev->pcidev_table[j].bdf) {
				PCIE_DBG2(dev,
					"PCIe: %s Error from Endpoint: %02x:%02x.%01x\n",
					i ? "Uncorrectable" : "Correctable",
					dev->pcidev_table[j].bdf >> 24,
					dev->pcidev_table[j].bdf >> 19 & 0x1f,
					dev->pcidev_table[j].bdf >> 16 & 0x07);
				ep_base = dev->pcidev_table[j].conf_base;
				ep_dev_ctrlstts_offset = dev->
					pcidev_table[j].dev_ctrlstts_offset;
				break;
			}
		}

		if (!ep_base) {
			PCIE_ERR(dev,
				"PCIe: RC%d no endpoint found for reported error\n",
				dev->rc_idx);
			goto out;
		}

		ep_uncorr_val = readl_relaxed(ep_base +
					PCIE20_AER_UNCORR_ERR_STATUS_REG);
		ep_corr_val = readl_relaxed(ep_base +
					PCIE20_AER_CORR_ERR_STATUS_REG);
		ep_dev_ctrlstts = readl_relaxed(ep_base +
					ep_dev_ctrlstts_offset);

		if (ep_uncorr_val)
			PCIE_DBG(dev,
				"EP's PCIE20_AER_UNCORR_ERR_STATUS_REG:0x%x\n",
				ep_uncorr_val);
		if (ep_corr_val && (dev->ep_corr_counter < corr_counter_limit))
			PCIE_DBG(dev,
				"EP's PCIE20_AER_CORR_ERR_STATUS_REG:0x%x\n",
				ep_corr_val);

		if ((ep_dev_ctrlstts >> 18) & 0x1)
			dev->ep_fatal_counter++;
		if ((ep_dev_ctrlstts >> 17) & 0x1)
			dev->ep_non_fatal_counter++;
		if ((ep_dev_ctrlstts >> 16) & 0x1)
			dev->ep_corr_counter++;

		msm_pcie_write_mask(ep_base + ep_dev_ctrlstts_offset, 0,
					BIT(18)|BIT(17)|BIT(16));

		msm_pcie_write_reg_field(ep_base,
				PCIE20_AER_UNCORR_ERR_STATUS_REG,
				0x3fff031, 0x3fff031);
		msm_pcie_write_reg_field(ep_base,
				PCIE20_AER_CORR_ERR_STATUS_REG,
				0xf1c1, 0xf1c1);
	}
out:
	if (((dev->rc_corr_counter < corr_counter_limit) &&
		(dev->ep_corr_counter < corr_counter_limit)) ||
		uncorr_val || ep_uncorr_val)
		PCIE_DBG(dev, "RC's PCIE20_AER_ROOT_ERR_STATUS_REG:0x%x\n",
				rc_err_status);
	msm_pcie_write_reg_field(dev->dm_core,
			PCIE20_AER_UNCORR_ERR_STATUS_REG,
			0x3fff031, 0x3fff031);
	msm_pcie_write_reg_field(dev->dm_core,
			PCIE20_AER_CORR_ERR_STATUS_REG,
			0xf1c1, 0xf1c1);
	msm_pcie_write_reg_field(dev->dm_core,
			PCIE20_AER_ROOT_ERR_STATUS_REG,
			0x7f, 0x7f);

	spin_unlock_irqrestore(&dev->aer_lock, irqsave_flags);
	return IRQ_HANDLED;
}

static irqreturn_t handle_wake_irq(int irq, void *data)
{
	struct msm_pcie_dev_t *dev = data;
	unsigned long irqsave_flags;
	int i;

	spin_lock_irqsave(&dev->wakeup_lock, irqsave_flags);

	dev->wake_counter++;
	PCIE_DBG(dev, "PCIe: No. %ld wake IRQ for RC%d\n",
			dev->wake_counter, dev->rc_idx);

	PCIE_DBG2(dev, "PCIe WAKE is asserted by Endpoint of RC%d\n",
		dev->rc_idx);

	if (!dev->enumerated) {
		PCIE_DBG(dev, "Start enumeating RC%d\n", dev->rc_idx);
		if (dev->ep_wakeirq)
			schedule_work(&dev->handle_wake_work);
		else
			PCIE_DBG(dev,
				"wake irq is received but ep_wakeirq is not supported for RC%d.\n",
				dev->rc_idx);
	} else {
		PCIE_DBG2(dev, "Wake up RC%d\n", dev->rc_idx);
		__pm_stay_awake(&dev->ws);
		__pm_relax(&dev->ws);

		if (dev->num_ep > 1) {
			for (i = 0; i < MAX_DEVICE_NUM; i++) {
				dev->event_reg =
					dev->pcidev_table[i].event_reg;
				msm_pcie_notify_client(dev,
					MSM_PCIE_EVENT_WAKEUP);
			}
		} else {
			msm_pcie_notify_client(dev, MSM_PCIE_EVENT_WAKEUP);
		}
	}

	spin_unlock_irqrestore(&dev->wakeup_lock, irqsave_flags);

	return IRQ_HANDLED;
}

static irqreturn_t handle_linkdown_irq(int irq, void *data)
{
	struct msm_pcie_dev_t *dev = data;
	unsigned long irqsave_flags;
	int i;

	spin_lock_irqsave(&dev->linkdown_lock, irqsave_flags);

	dev->linkdown_counter++;

	PCIE_DBG(dev,
		"PCIe: No. %ld linkdown IRQ for RC%d.\n",
		dev->linkdown_counter, dev->rc_idx);

	if (!dev->enumerated || dev->link_status != MSM_PCIE_LINK_ENABLED) {
		PCIE_DBG(dev,
			"PCIe:Linkdown IRQ for RC%d when the link is not enabled\n",
			dev->rc_idx);
	} else if (dev->suspending) {
		PCIE_DBG(dev,
			"PCIe:the link of RC%d is suspending.\n",
			dev->rc_idx);
	} else {
		dev->link_status = MSM_PCIE_LINK_DISABLED;
		dev->shadow_en = false;

		pcie_phy_dump(dev);
		pcie_parf_dump(dev);

		if (dev->linkdown_panic)
			panic("User has chosen to panic on linkdown\n");

		/* assert PERST */
		gpio_set_value(dev->gpio[MSM_PCIE_GPIO_PERST].num,
				dev->gpio[MSM_PCIE_GPIO_PERST].on);
		PCIE_ERR(dev, "PCIe link is down for RC%d\n", dev->rc_idx);

		if (dev->num_ep > 1) {
			for (i = 0; i < MAX_DEVICE_NUM; i++) {
				dev->event_reg =
					dev->pcidev_table[i].event_reg;
				msm_pcie_notify_client(dev,
					MSM_PCIE_EVENT_LINKDOWN);
			}
		} else {
			msm_pcie_notify_client(dev, MSM_PCIE_EVENT_LINKDOWN);
		}
	}

	spin_unlock_irqrestore(&dev->linkdown_lock, irqsave_flags);

	return IRQ_HANDLED;
}

static irqreturn_t handle_msi_irq(int irq, void *data)
{
	int i, j;
	unsigned long val;
	struct msm_pcie_dev_t *dev = data;
	void __iomem *ctrl_status;

	PCIE_DUMP(dev, "irq: %d\n", irq);

	/* check for set bits, clear it by setting that bit
	   and trigger corresponding irq */
	for (i = 0; i < PCIE20_MSI_CTRL_MAX; i++) {
		ctrl_status = dev->dm_core +
				PCIE20_MSI_CTRL_INTR_STATUS + (i * 12);

		val = readl_relaxed(ctrl_status);
		while (val) {
			j = find_first_bit(&val, 32);
			writel_relaxed(BIT(j), ctrl_status);
			/* ensure that interrupt is cleared (acked) */
			wmb();
			generic_handle_irq(
			   irq_find_mapping(dev->irq_domain, (j + (32*i)))
			   );
			val = readl_relaxed(ctrl_status);
		}
	}

	return IRQ_HANDLED;
}

static irqreturn_t handle_global_irq(int irq, void *data)
{
	int i;
	struct msm_pcie_dev_t *dev = data;
	unsigned long irqsave_flags;
	u32 status;

	spin_lock_irqsave(&dev->global_irq_lock, irqsave_flags);

	status = readl_relaxed(dev->parf + PCIE20_PARF_INT_ALL_STATUS) &
			readl_relaxed(dev->parf + PCIE20_PARF_INT_ALL_MASK);

	msm_pcie_write_mask(dev->parf + PCIE20_PARF_INT_ALL_CLEAR, 0, status);

	PCIE_DBG2(dev, "RC%d: Global IRQ %d received: 0x%x\n",
		dev->rc_idx, irq, status);

	for (i = 0; i <= MSM_PCIE_INT_EVT_MAX; i++) {
		if (status & BIT(i)) {
			switch (i) {
			case MSM_PCIE_INT_EVT_LINK_DOWN:
				PCIE_DBG(dev,
					"PCIe: RC%d: handle linkdown event.\n",
					dev->rc_idx);
				handle_linkdown_irq(irq, data);
				break;
			case MSM_PCIE_INT_EVT_AER_LEGACY:
				PCIE_DBG(dev,
					"PCIe: RC%d: AER legacy event.\n",
					dev->rc_idx);
				handle_aer_irq(irq, data);
				break;
			case MSM_PCIE_INT_EVT_AER_ERR:
				PCIE_DBG(dev,
					"PCIe: RC%d: AER event.\n",
					dev->rc_idx);
				handle_aer_irq(irq, data);
				break;
			default:
				PCIE_ERR(dev,
					"PCIe: RC%d: Unexpected event %d is caught!\n",
					dev->rc_idx, i);
			}
		}
	}

	spin_unlock_irqrestore(&dev->global_irq_lock, irqsave_flags);

	return IRQ_HANDLED;
}

void msm_pcie_destroy_irq(unsigned int irq, struct msm_pcie_dev_t *pcie_dev)
{
	int pos, i;
	struct msm_pcie_dev_t *dev;

	if (pcie_dev)
		dev = pcie_dev;
	else
		dev = irq_get_chip_data(irq);

	if (!dev) {
		pr_err("PCIe: device is null. IRQ:%d\n", irq);
		return;
	}

	if (dev->msi_gicm_addr) {
		PCIE_DBG(dev, "destroy QGIC based irq %d\n", irq);

		for (i = 0; i < MSM_PCIE_MAX_MSI; i++)
			if (irq == dev->msi[i].num)
				break;
		if (i == MSM_PCIE_MAX_MSI) {
			PCIE_ERR(dev,
				"Could not find irq: %d in RC%d MSI table\n",
				irq, dev->rc_idx);
			return;
		} else {
			pos = i;
		}
	} else {
		PCIE_DBG(dev, "destroy default MSI irq %d\n", irq);
		pos = irq - irq_find_mapping(dev->irq_domain, 0);
	}

	PCIE_DBG(dev, "RC%d\n", dev->rc_idx);

	PCIE_DBG(dev, "Before clear_bit pos:%d msi_irq_in_use:%ld\n",
		pos, *dev->msi_irq_in_use);
	clear_bit(pos, dev->msi_irq_in_use);
	PCIE_DBG(dev, "After clear_bit pos:%d msi_irq_in_use:%ld\n",
		pos, *dev->msi_irq_in_use);
}

/* hookup to linux pci msi framework */
void arch_teardown_msi_irq(unsigned int irq)
{
	PCIE_GEN_DBG("irq %d deallocated\n", irq);
	msm_pcie_destroy_irq(irq, NULL);
}

void arch_teardown_msi_irqs(struct pci_dev *dev)
{
	struct msi_desc *entry;
	struct msm_pcie_dev_t *pcie_dev = PCIE_BUS_PRIV_DATA(dev->bus);

	PCIE_DBG(pcie_dev, "RC:%d EP: vendor_id:0x%x device_id:0x%x\n",
		pcie_dev->rc_idx, dev->vendor, dev->device);

	pcie_dev->use_msi = false;

	list_for_each_entry(entry, &dev->msi_list, list) {
		int i, nvec;
		if (entry->irq == 0)
			continue;
		nvec = 1 << entry->msi_attrib.multiple;
		for (i = 0; i < nvec; i++)
			msm_pcie_destroy_irq(entry->irq + i, pcie_dev);
	}
}

static void msm_pcie_msi_nop(struct irq_data *d)
{
	return;
}

static struct irq_chip pcie_msi_chip = {
	.name = "msm-pcie-msi",
	.irq_ack = msm_pcie_msi_nop,
	.irq_enable = unmask_msi_irq,
	.irq_disable = mask_msi_irq,
	.irq_mask = mask_msi_irq,
	.irq_unmask = unmask_msi_irq,
};

static int msm_pcie_create_irq(struct msm_pcie_dev_t *dev)
{
	int irq, pos;

	PCIE_DBG(dev, "RC%d\n", dev->rc_idx);

again:
	pos = find_first_zero_bit(dev->msi_irq_in_use, PCIE_MSI_NR_IRQS);

	if (pos >= PCIE_MSI_NR_IRQS)
		return -ENOSPC;

	PCIE_DBG(dev, "pos:%d msi_irq_in_use:%ld\n", pos, *dev->msi_irq_in_use);

	if (test_and_set_bit(pos, dev->msi_irq_in_use))
		goto again;
	else
		PCIE_DBG(dev, "test_and_set_bit is successful pos=%d\n", pos);

	irq = irq_create_mapping(dev->irq_domain, pos);
	if (!irq)
		return -EINVAL;

	return irq;
}

static int arch_setup_msi_irq_default(struct pci_dev *pdev,
		struct msi_desc *desc, int nvec)
{
	int irq;
	struct msi_msg msg;
	struct msm_pcie_dev_t *dev = PCIE_BUS_PRIV_DATA(pdev->bus);

	PCIE_DBG(dev, "RC%d\n", dev->rc_idx);

	irq = msm_pcie_create_irq(dev);

	PCIE_DBG(dev, "IRQ %d is allocated.\n", irq);

	if (irq < 0)
		return irq;

	PCIE_DBG(dev, "irq %d allocated\n", irq);

	irq_set_msi_desc(irq, desc);

	/* write msi vector and data */
	msg.address_hi = 0;
	msg.address_lo = MSM_PCIE_MSI_PHY;
	msg.data = irq - irq_find_mapping(dev->irq_domain, 0);
	write_msi_msg(irq, &msg);

	return 0;
}

static int msm_pcie_create_irq_qgic(struct msm_pcie_dev_t *dev)
{
	int irq, pos;

	PCIE_DBG(dev, "RC%d\n", dev->rc_idx);

again:
	pos = find_first_zero_bit(dev->msi_irq_in_use, PCIE_MSI_NR_IRQS);

	if (pos >= PCIE_MSI_NR_IRQS)
		return -ENOSPC;

	PCIE_DBG(dev, "pos:%d msi_irq_in_use:%ld\n", pos, *dev->msi_irq_in_use);

	if (test_and_set_bit(pos, dev->msi_irq_in_use))
		goto again;
	else
		PCIE_DBG(dev, "test_and_set_bit is successful pos=%d\n", pos);

	if (pos >= MSM_PCIE_MAX_MSI) {
		PCIE_ERR(dev,
			"PCIe: RC%d: pos %d is not less than %d\n",
			dev->rc_idx, pos, MSM_PCIE_MAX_MSI);
		return MSM_PCIE_ERROR;
	}

	irq = dev->msi[pos].num;
	if (!irq) {
		PCIE_ERR(dev, "PCIe: RC%d failed to create QGIC MSI IRQ.\n",
			dev->rc_idx);
		return -EINVAL;
	}

	return irq;
}

static int arch_setup_msi_irq_qgic(struct pci_dev *pdev,
		struct msi_desc *desc, int nvec)
{
	int irq, index, firstirq = 0;
	struct msi_msg msg;
	struct msm_pcie_dev_t *dev = PCIE_BUS_PRIV_DATA(pdev->bus);

	PCIE_DBG(dev, "RC%d\n", dev->rc_idx);

	for (index = 0; index < nvec; index++) {
		irq = msm_pcie_create_irq_qgic(dev);
		PCIE_DBG(dev, "irq %d is allocated\n", irq);

		if (irq < 0)
			return irq;

		if (index == 0)
			firstirq = irq;

		irq_set_irq_type(irq, IRQ_TYPE_EDGE_RISING);
	}

	/* write msi vector and data */
	irq_set_msi_desc(firstirq, desc);
	msg.address_hi = 0;
	msg.address_lo = dev->msi_gicm_addr;
	msg.data = dev->msi_gicm_base;
	write_msi_msg(firstirq, &msg);

	return 0;
}

int arch_setup_msi_irq(struct pci_dev *pdev, struct msi_desc *desc)
{
	struct msm_pcie_dev_t *dev = PCIE_BUS_PRIV_DATA(pdev->bus);

	PCIE_DBG(dev, "RC%d\n", dev->rc_idx);

	if (dev->msi_gicm_addr)
		return arch_setup_msi_irq_qgic(pdev, desc, 1);
	else
		return arch_setup_msi_irq_default(pdev, desc, 1);
}

static int msm_pcie_get_msi_multiple(int nvec)
{
	int msi_multiple = 0;

	while (nvec) {
		nvec = nvec >> 1;
		msi_multiple++;
	}
	PCIE_GEN_DBG("log2 number of MSI multiple:%d\n",
		msi_multiple - 1);

	return msi_multiple - 1;
}

int arch_setup_msi_irqs(struct pci_dev *dev, int nvec, int type)
{
	struct msi_desc *entry;
	int ret;
	struct msm_pcie_dev_t *pcie_dev = PCIE_BUS_PRIV_DATA(dev->bus);

	PCIE_DBG(pcie_dev, "RC%d\n", pcie_dev->rc_idx);

	if (type != PCI_CAP_ID_MSI || nvec > 32)
		return -ENOSPC;

	PCIE_DBG(pcie_dev, "nvec = %d\n", nvec);

	list_for_each_entry(entry, &dev->msi_list, list) {
		entry->msi_attrib.multiple =
				msm_pcie_get_msi_multiple(nvec);

		if (pcie_dev->msi_gicm_addr)
			ret = arch_setup_msi_irq_qgic(dev, entry, nvec);
		else
			ret = arch_setup_msi_irq_default(dev, entry, nvec);

		PCIE_DBG(pcie_dev, "ret from msi_irq: %d\n", ret);

		if (ret < 0)
			return ret;
		if (ret > 0)
			return -ENOSPC;
	}

	pcie_dev->use_msi = true;

	return 0;
}

static int msm_pcie_msi_map(struct irq_domain *domain, unsigned int irq,
	   irq_hw_number_t hwirq)
{
	irq_set_chip_and_handler (irq, &pcie_msi_chip, handle_simple_irq);
	irq_set_chip_data(irq, domain->host_data);
	set_irq_flags(irq, IRQF_VALID);
	return 0;
}

static const struct irq_domain_ops msm_pcie_msi_ops = {
	.map = msm_pcie_msi_map,
};

int32_t msm_pcie_irq_init(struct msm_pcie_dev_t *dev)
{
	int rc;
	int msi_start =  0;
	struct device *pdev = &dev->pdev->dev;

	PCIE_DBG(dev, "RC%d\n", dev->rc_idx);

	if (dev->rc_idx)
		wakeup_source_init(&dev->ws, "RC1 pcie_wakeup_source");
	else
		wakeup_source_init(&dev->ws, "RC0 pcie_wakeup_source");

	/* register handler for linkdown interrupt */
	if (dev->irq[MSM_PCIE_INT_LINK_DOWN].num) {
		rc = devm_request_irq(pdev,
			dev->irq[MSM_PCIE_INT_LINK_DOWN].num,
			handle_linkdown_irq,
			IRQF_TRIGGER_RISING,
			dev->irq[MSM_PCIE_INT_LINK_DOWN].name,
			dev);
		if (rc) {
			PCIE_ERR(dev,
				"PCIe: Unable to request linkdown interrupt:%d\n",
				dev->irq[MSM_PCIE_INT_LINK_DOWN].num);
			return rc;
		}
	}

	/* register handler for physical MSI interrupt line */
	if (dev->irq[MSM_PCIE_INT_MSI].num) {
		rc = devm_request_irq(pdev,
			dev->irq[MSM_PCIE_INT_MSI].num,
			handle_msi_irq,
			IRQF_TRIGGER_RISING,
			dev->irq[MSM_PCIE_INT_MSI].name,
			dev);
		if (rc) {
			PCIE_ERR(dev,
				"PCIe: RC%d: Unable to request MSI interrupt\n",
				dev->rc_idx);
			return rc;
		}
	}

	/* register handler for AER interrupt */
	if (dev->irq[MSM_PCIE_INT_PLS_ERR].num) {
		rc = devm_request_irq(pdev,
				dev->irq[MSM_PCIE_INT_PLS_ERR].num,
				handle_aer_irq,
				IRQF_TRIGGER_RISING,
				dev->irq[MSM_PCIE_INT_PLS_ERR].name,
				dev);
		if (rc) {
			PCIE_ERR(dev,
				"PCIe: RC%d: Unable to request aer pls_err interrupt: %d\n",
				dev->rc_idx,
				dev->irq[MSM_PCIE_INT_PLS_ERR].num);
			return rc;
		}
	}

	/* register handler for AER legacy interrupt */
	if (dev->irq[MSM_PCIE_INT_AER_LEGACY].num) {
		rc = devm_request_irq(pdev,
				dev->irq[MSM_PCIE_INT_AER_LEGACY].num,
				handle_aer_irq,
				IRQF_TRIGGER_RISING,
				dev->irq[MSM_PCIE_INT_AER_LEGACY].name,
				dev);
		if (rc) {
			PCIE_ERR(dev,
				"PCIe: RC%d: Unable to request aer aer_legacy interrupt: %d\n",
				dev->rc_idx,
				dev->irq[MSM_PCIE_INT_AER_LEGACY].num);
			return rc;
		}
	}

	if (dev->irq[MSM_PCIE_INT_GLOBAL_INT].num) {
		rc = devm_request_irq(pdev,
				dev->irq[MSM_PCIE_INT_GLOBAL_INT].num,
				handle_global_irq,
				IRQF_TRIGGER_RISING,
				dev->irq[MSM_PCIE_INT_GLOBAL_INT].name,
				dev);
		if (rc) {
			PCIE_ERR(dev,
				"PCIe: RC%d: Unable to request global_int interrupt: %d\n",
				dev->rc_idx,
				dev->irq[MSM_PCIE_INT_GLOBAL_INT].num);
			return rc;
		}
	}

	/* register handler for PCIE_WAKE_N interrupt line */
	if (dev->wake_n) {
		rc = devm_request_irq(pdev,
				dev->wake_n, handle_wake_irq,
				IRQF_TRIGGER_FALLING, "msm_pcie_wake", dev);
		if (rc) {
			PCIE_ERR(dev,
				"PCIe: RC%d: Unable to request wake interrupt\n",
				dev->rc_idx);
			return rc;
		}

		INIT_WORK(&dev->handle_wake_work, handle_wake_func);

		rc = enable_irq_wake(dev->wake_n);
		if (rc) {
			PCIE_ERR(dev,
				"PCIe: RC%d: Unable to enable wake interrupt\n",
				dev->rc_idx);
			return rc;
		}
	}

	/* Create a virtual domain of interrupts */
	if (!dev->msi_gicm_addr) {
		dev->irq_domain = irq_domain_add_linear(dev->pdev->dev.of_node,
			PCIE_MSI_NR_IRQS, &msm_pcie_msi_ops, dev);

		if (!dev->irq_domain) {
			PCIE_ERR(dev,
				"PCIe: RC%d: Unable to initialize irq domain\n",
				dev->rc_idx);

			if (dev->wake_n)
				disable_irq(dev->wake_n);

			return PTR_ERR(dev->irq_domain);
		}

		msi_start = irq_create_mapping(dev->irq_domain, 0);
	}

	return 0;
}

void msm_pcie_irq_deinit(struct msm_pcie_dev_t *dev)
{
	PCIE_DBG(dev, "RC%d\n", dev->rc_idx);

	wakeup_source_trash(&dev->ws);

	if (dev->wake_n)
		disable_irq(dev->wake_n);
}


static int msm_pcie_probe(struct platform_device *pdev)
{
	int ret = 0;
	int rc_idx = -1;
	int i, j;

	PCIE_GEN_DBG("%s\n", __func__);

	mutex_lock(&pcie_drv.drv_lock);

	ret = of_property_read_u32((&pdev->dev)->of_node,
				"cell-index", &rc_idx);
	if (ret) {
		PCIE_GEN_DBG("Did not find RC index.\n");
		goto out;
	} else {
		if (rc_idx >= MAX_RC_NUM) {
			pr_err(
				"PCIe: Invalid RC Index %d (max supported = %d)\n",
				rc_idx, MAX_RC_NUM);
			goto out;
		}
		pcie_drv.rc_num++;
		PCIE_DBG(&msm_pcie_dev[rc_idx], "PCIe: RC index is %d.\n",
			rc_idx);
	}

	msm_pcie_dev[rc_idx].l0s_supported =
		of_property_read_bool((&pdev->dev)->of_node,
				"qcom,l0s-supported");
	PCIE_DBG(&msm_pcie_dev[rc_idx], "L0s is %s supported.\n",
		msm_pcie_dev[rc_idx].l0s_supported ? "" : "not");
	msm_pcie_dev[rc_idx].l1_supported =
		of_property_read_bool((&pdev->dev)->of_node,
				"qcom,l1-supported");
	PCIE_DBG(&msm_pcie_dev[rc_idx], "L1 is %s supported.\n",
		msm_pcie_dev[rc_idx].l1_supported ? "" : "not");
	msm_pcie_dev[rc_idx].l1ss_supported =
		of_property_read_bool((&pdev->dev)->of_node,
				"qcom,l1ss-supported");
	PCIE_DBG(&msm_pcie_dev[rc_idx], "L1ss is %s supported.\n",
		msm_pcie_dev[rc_idx].l1ss_supported ? "" : "not");
	msm_pcie_dev[rc_idx].common_clk_en =
		of_property_read_bool((&pdev->dev)->of_node,
				"qcom,common-clk-en");
	PCIE_DBG(&msm_pcie_dev[rc_idx], "Common clock is %s enabled.\n",
		msm_pcie_dev[rc_idx].common_clk_en ? "" : "not");
	msm_pcie_dev[rc_idx].clk_power_manage_en =
		of_property_read_bool((&pdev->dev)->of_node,
				"qcom,clk-power-manage-en");
	PCIE_DBG(&msm_pcie_dev[rc_idx],
		"Clock power management is %s enabled.\n",
		msm_pcie_dev[rc_idx].clk_power_manage_en ? "" : "not");
	msm_pcie_dev[rc_idx].aux_clk_sync =
		of_property_read_bool((&pdev->dev)->of_node,
				"qcom,aux-clk-sync");
	PCIE_DBG(&msm_pcie_dev[rc_idx],
		"AUX clock is %s synchronous to Core clock.\n",
		msm_pcie_dev[rc_idx].aux_clk_sync ? "" : "not");

	msm_pcie_dev[rc_idx].use_19p2mhz_aux_clk =
		of_property_read_bool((&pdev->dev)->of_node,
				"qcom,use-19p2mhz-aux-clk");
	PCIE_DBG(&msm_pcie_dev[rc_idx],
		"AUX clock frequency is %s 19.2MHz.\n",
		msm_pcie_dev[rc_idx].use_19p2mhz_aux_clk ? "" : "not");

	msm_pcie_dev[rc_idx].smmu_exist =
		of_property_read_bool((&pdev->dev)->of_node,
				"qcom,smmu-exist");
	PCIE_DBG(&msm_pcie_dev[rc_idx],
		"SMMU does %s exist.\n",
		msm_pcie_dev[rc_idx].smmu_exist ? "" : "not");

	msm_pcie_dev[rc_idx].ep_wakeirq =
		of_property_read_bool((&pdev->dev)->of_node,
				"qcom,ep-wakeirq");
	PCIE_DBG(&msm_pcie_dev[rc_idx],
		"PCIe: EP of RC%d does %s assert wake when it is up.\n",
		rc_idx, msm_pcie_dev[rc_idx].ep_wakeirq ? "" : "not");

	msm_pcie_dev[rc_idx].phy_ver = 1;
	ret = of_property_read_u32((&pdev->dev)->of_node,
				"qcom,pcie-phy-ver",
				&msm_pcie_dev[rc_idx].phy_ver);
	if (ret)
		PCIE_DBG(&msm_pcie_dev[rc_idx],
			"RC%d: pcie-phy-ver does not exist.\n",
			msm_pcie_dev[rc_idx].rc_idx);
	else
		PCIE_DBG(&msm_pcie_dev[rc_idx],
			"RC%d: pcie-phy-ver: %d.\n",
			msm_pcie_dev[rc_idx].rc_idx,
			msm_pcie_dev[rc_idx].phy_ver);

	msm_pcie_dev[rc_idx].n_fts = 0;
	ret = of_property_read_u32((&pdev->dev)->of_node,
				"qcom,n-fts",
				&msm_pcie_dev[rc_idx].n_fts);

	if (ret)
		PCIE_DBG(&msm_pcie_dev[rc_idx],
			"n-fts does not exist. ret=%d\n", ret);
	else
		PCIE_DBG(&msm_pcie_dev[rc_idx], "n-fts: 0x%x.\n",
				msm_pcie_dev[rc_idx].n_fts);

	msm_pcie_dev[rc_idx].common_phy =
		of_property_read_bool((&pdev->dev)->of_node,
				"qcom,common-phy");
	PCIE_DBG(&msm_pcie_dev[rc_idx],
		"PCIe: RC%d: Common PHY does %s exist.\n",
		rc_idx, msm_pcie_dev[rc_idx].common_phy ? "" : "not");

	msm_pcie_dev[rc_idx].ext_ref_clk =
		of_property_read_bool((&pdev->dev)->of_node,
				"qcom,ext-ref-clk");
	PCIE_DBG(&msm_pcie_dev[rc_idx], "ref clk is %s.\n",
		msm_pcie_dev[rc_idx].ext_ref_clk ? "external" : "internal");

	msm_pcie_dev[rc_idx].ep_latency = 0;
	ret = of_property_read_u32((&pdev->dev)->of_node,
				"qcom,ep-latency",
				&msm_pcie_dev[rc_idx].ep_latency);
	if (ret)
		PCIE_DBG(&msm_pcie_dev[rc_idx],
			"RC%d: ep-latency does not exist.\n",
			rc_idx);
	else
		PCIE_DBG(&msm_pcie_dev[rc_idx], "RC%d: ep-latency: 0x%x.\n",
			rc_idx, msm_pcie_dev[rc_idx].ep_latency);

	msm_pcie_dev[rc_idx].cpl_timeout = 0;
	ret = of_property_read_u32((&pdev->dev)->of_node,
				"qcom,cpl-timeout",
				&msm_pcie_dev[rc_idx].cpl_timeout);
	if (ret)
		PCIE_DBG(&msm_pcie_dev[rc_idx],
			"RC%d: Using default cpl-timeout.\n",
			rc_idx);
	else
		PCIE_DBG(&msm_pcie_dev[rc_idx], "RC%d: cpl-timeout: 0x%x.\n",
			rc_idx, msm_pcie_dev[rc_idx].cpl_timeout);

	msm_pcie_dev[rc_idx].perst_delay_us_min =
		PERST_PROPAGATION_DELAY_US_MIN;
	ret = of_property_read_u32(pdev->dev.of_node,
				"qcom,perst-delay-us-min",
				&msm_pcie_dev[rc_idx].perst_delay_us_min);
	if (ret)
		PCIE_DBG(&msm_pcie_dev[rc_idx],
			"RC%d: perst-delay-us-min does not exist. Use default value %dus.\n",
			rc_idx, msm_pcie_dev[rc_idx].perst_delay_us_min);
	else
		PCIE_DBG(&msm_pcie_dev[rc_idx],
			"RC%d: perst-delay-us-min: %dus.\n",
			rc_idx, msm_pcie_dev[rc_idx].perst_delay_us_min);

	msm_pcie_dev[rc_idx].perst_delay_us_max =
		PERST_PROPAGATION_DELAY_US_MAX;
	ret = of_property_read_u32(pdev->dev.of_node,
				"qcom,perst-delay-us-max",
				&msm_pcie_dev[rc_idx].perst_delay_us_max);
	if (ret)
		PCIE_DBG(&msm_pcie_dev[rc_idx],
			"RC%d: perst-delay-us-max does not exist. Use default value %dus.\n",
			rc_idx, msm_pcie_dev[rc_idx].perst_delay_us_max);
	else
		PCIE_DBG(&msm_pcie_dev[rc_idx],
			"RC%d: perst-delay-us-max: %dus.\n",
			rc_idx, msm_pcie_dev[rc_idx].perst_delay_us_max);

	msm_pcie_dev[rc_idx].tlp_rd_size = PCIE_TLP_RD_SIZE;
	ret = of_property_read_u32(pdev->dev.of_node,
				"qcom,tlp-rd-size",
				&msm_pcie_dev[rc_idx].tlp_rd_size);
	if (ret)
		PCIE_DBG(&msm_pcie_dev[rc_idx],
			"RC%d: tlp-rd-size does not exist. tlp-rd-size: 0x%x.\n",
			rc_idx, msm_pcie_dev[rc_idx].tlp_rd_size);
	else
		PCIE_DBG(&msm_pcie_dev[rc_idx], "RC%d: tlp-rd-size: 0x%x.\n",
			rc_idx, msm_pcie_dev[rc_idx].tlp_rd_size);

	msm_pcie_dev[rc_idx].msi_gicm_addr = 0;
	msm_pcie_dev[rc_idx].msi_gicm_base = 0;
	ret = of_property_read_u32((&pdev->dev)->of_node,
				"qcom,msi-gicm-addr",
				&msm_pcie_dev[rc_idx].msi_gicm_addr);

	if (ret) {
		PCIE_DBG(&msm_pcie_dev[rc_idx], "%s",
			"msi-gicm-addr does not exist.\n");
	} else {
		PCIE_DBG(&msm_pcie_dev[rc_idx], "msi-gicm-addr: 0x%x.\n",
				msm_pcie_dev[rc_idx].msi_gicm_addr);

		ret = of_property_read_u32((&pdev->dev)->of_node,
				"qcom,msi-gicm-base",
				&msm_pcie_dev[rc_idx].msi_gicm_base);

		if (ret) {
			PCIE_ERR(&msm_pcie_dev[rc_idx],
				"PCIe: RC%d: msi-gicm-base does not exist.\n",
				rc_idx);
			goto decrease_rc_num;
		} else {
			PCIE_DBG(&msm_pcie_dev[rc_idx], "msi-gicm-base: 0x%x\n",
					msm_pcie_dev[rc_idx].msi_gicm_base);
		}
	}

	msm_pcie_dev[rc_idx].scm_dev_id = 0;
	ret = of_property_read_u32((&pdev->dev)->of_node,
				"qcom,scm-dev-id",
				&msm_pcie_dev[rc_idx].scm_dev_id);

	msm_pcie_dev[rc_idx].rc_idx = rc_idx;
	msm_pcie_dev[rc_idx].pdev = pdev;
	msm_pcie_dev[rc_idx].vreg_n = 0;
	msm_pcie_dev[rc_idx].gpio_n = 0;
	msm_pcie_dev[rc_idx].parf_deemph = 0;
	msm_pcie_dev[rc_idx].parf_swing = 0;
	msm_pcie_dev[rc_idx].link_status = MSM_PCIE_LINK_DEINIT;
	msm_pcie_dev[rc_idx].user_suspend = false;
	msm_pcie_dev[rc_idx].disable_pc = false;
	msm_pcie_dev[rc_idx].saved_state = NULL;
	msm_pcie_dev[rc_idx].enumerated = false;
	msm_pcie_dev[rc_idx].num_active_ep = 0;
	msm_pcie_dev[rc_idx].num_ep = 0;
	msm_pcie_dev[rc_idx].pending_ep_reg = false;
	msm_pcie_dev[rc_idx].phy_len = 0;
	msm_pcie_dev[rc_idx].port_phy_len = 0;
	msm_pcie_dev[rc_idx].phy_sequence = NULL;
	msm_pcie_dev[rc_idx].port_phy_sequence = NULL;
	msm_pcie_dev[rc_idx].event_reg = NULL;
	msm_pcie_dev[rc_idx].linkdown_counter = 0;
	msm_pcie_dev[rc_idx].link_turned_on_counter = 0;
	msm_pcie_dev[rc_idx].link_turned_off_counter = 0;
	msm_pcie_dev[rc_idx].rc_corr_counter = 0;
	msm_pcie_dev[rc_idx].rc_non_fatal_counter = 0;
	msm_pcie_dev[rc_idx].rc_fatal_counter = 0;
	msm_pcie_dev[rc_idx].ep_corr_counter = 0;
	msm_pcie_dev[rc_idx].ep_non_fatal_counter = 0;
	msm_pcie_dev[rc_idx].ep_fatal_counter = 0;
	msm_pcie_dev[rc_idx].suspending = false;
	msm_pcie_dev[rc_idx].wake_counter = 0;
	msm_pcie_dev[rc_idx].aer_enable = true;
	msm_pcie_dev[rc_idx].power_on = false;
	msm_pcie_dev[rc_idx].current_short_bdf = 0;
	msm_pcie_dev[rc_idx].use_msi = false;
	msm_pcie_dev[rc_idx].use_pinctrl = false;
	msm_pcie_dev[rc_idx].linkdown_panic = false;
	msm_pcie_dev[rc_idx].bridge_found = false;
	memcpy(msm_pcie_dev[rc_idx].vreg, msm_pcie_vreg_info,
				sizeof(msm_pcie_vreg_info));
	memcpy(msm_pcie_dev[rc_idx].gpio, msm_pcie_gpio_info,
				sizeof(msm_pcie_gpio_info));
	memcpy(msm_pcie_dev[rc_idx].clk, msm_pcie_clk_info[rc_idx],
				sizeof(msm_pcie_clk_info[rc_idx]));
	memcpy(msm_pcie_dev[rc_idx].pipeclk, msm_pcie_pipe_clk_info[rc_idx],
				sizeof(msm_pcie_pipe_clk_info[rc_idx]));
	memcpy(msm_pcie_dev[rc_idx].res, msm_pcie_res_info,
				sizeof(msm_pcie_res_info));
	memcpy(msm_pcie_dev[rc_idx].irq, msm_pcie_irq_info,
				sizeof(msm_pcie_irq_info));
	memcpy(msm_pcie_dev[rc_idx].msi, msm_pcie_msi_info,
				sizeof(msm_pcie_msi_info));
	msm_pcie_dev[rc_idx].shadow_en = true;
	for (i = 0; i < PCIE_CONF_SPACE_DW; i++)
		msm_pcie_dev[rc_idx].rc_shadow[i] = PCIE_CLEAR;
	for (i = 0; i < MAX_DEVICE_NUM; i++)
		for (j = 0; j < PCIE_CONF_SPACE_DW; j++)
			msm_pcie_dev[rc_idx].ep_shadow[i][j] = PCIE_CLEAR;
	for (i = 0; i < MAX_DEVICE_NUM; i++) {
		msm_pcie_dev[rc_idx].pcidev_table[i].bdf = 0;
		msm_pcie_dev[rc_idx].pcidev_table[i].dev = NULL;
		msm_pcie_dev[rc_idx].pcidev_table[i].short_bdf = 0;
		msm_pcie_dev[rc_idx].pcidev_table[i].sid = 0;
		msm_pcie_dev[rc_idx].pcidev_table[i].domain = rc_idx;
		msm_pcie_dev[rc_idx].pcidev_table[i].conf_base = 0;
		msm_pcie_dev[rc_idx].pcidev_table[i].phy_address = 0;
		msm_pcie_dev[rc_idx].pcidev_table[i].dev_ctrlstts_offset = 0;
		msm_pcie_dev[rc_idx].pcidev_table[i].event_reg = NULL;
		msm_pcie_dev[rc_idx].pcidev_table[i].registered = true;
	}

	ret = msm_pcie_get_resources(&msm_pcie_dev[rc_idx],
				msm_pcie_dev[rc_idx].pdev);

	if (ret)
		goto decrease_rc_num;

	msm_pcie_dev[rc_idx].pinctrl = devm_pinctrl_get(&pdev->dev);
	if (IS_ERR_OR_NULL(msm_pcie_dev[rc_idx].pinctrl))
		PCIE_ERR(&msm_pcie_dev[rc_idx],
			"PCIe: RC%d failed to get pinctrl\n",
			rc_idx);
	else
		msm_pcie_dev[rc_idx].use_pinctrl = true;

	if (msm_pcie_dev[rc_idx].use_pinctrl) {
		msm_pcie_dev[rc_idx].pins_default =
			pinctrl_lookup_state(msm_pcie_dev[rc_idx].pinctrl,
						"default");
		if (IS_ERR(msm_pcie_dev[rc_idx].pins_default)) {
			PCIE_ERR(&msm_pcie_dev[rc_idx],
				"PCIe: RC%d could not get pinctrl default state\n",
				rc_idx);
			msm_pcie_dev[rc_idx].pins_default = NULL;
		}

		msm_pcie_dev[rc_idx].pins_sleep =
			pinctrl_lookup_state(msm_pcie_dev[rc_idx].pinctrl,
						"sleep");
		if (IS_ERR(msm_pcie_dev[rc_idx].pins_sleep)) {
			PCIE_ERR(&msm_pcie_dev[rc_idx],
				"PCIe: RC%d could not get pinctrl sleep state\n",
				rc_idx);
			msm_pcie_dev[rc_idx].pins_sleep = NULL;
		}
	}

	ret = msm_pcie_gpio_init(&msm_pcie_dev[rc_idx]);
	if (ret) {
		msm_pcie_release_resources(&msm_pcie_dev[rc_idx]);
		goto decrease_rc_num;
	}

	ret = msm_pcie_irq_init(&msm_pcie_dev[rc_idx]);
	if (ret) {
		msm_pcie_release_resources(&msm_pcie_dev[rc_idx]);
		msm_pcie_gpio_deinit(&msm_pcie_dev[rc_idx]);
		goto decrease_rc_num;
	}

	msm_pcie_dev[rc_idx].drv_ready = true;

	if (msm_pcie_dev[rc_idx].ep_wakeirq) {
		PCIE_DBG(&msm_pcie_dev[rc_idx],
			"PCIe: RC%d will be enumerated upon WAKE signal from Endpoint.\n",
			rc_idx);
		mutex_unlock(&pcie_drv.drv_lock);
		return 0;
	}

	ret = msm_pcie_enumerate(rc_idx);

	if (ret)
		PCIE_ERR(&msm_pcie_dev[rc_idx],
			"PCIe: RC%d is not enabled during bootup; it will be enumerated upon client request.\n",
			rc_idx);
	else
		PCIE_ERR(&msm_pcie_dev[rc_idx], "RC%d is enabled in bootup\n",
			rc_idx);

	PCIE_DBG(&msm_pcie_dev[rc_idx], "PCIE probed %s\n",
		dev_name(&(pdev->dev)));

	mutex_unlock(&pcie_drv.drv_lock);
	return 0;

decrease_rc_num:
	pcie_drv.rc_num--;
out:
	if (rc_idx < 0 || rc_idx >= MAX_RC_NUM)
		pr_err("PCIe: Invalid RC index %d. Driver probe failed\n",
		rc_idx);
	else
		PCIE_ERR(&msm_pcie_dev[rc_idx],
			"PCIe: Driver probe failed for RC%d:%d\n",
			rc_idx, ret);

	mutex_unlock(&pcie_drv.drv_lock);

	return ret;
}

static int msm_pcie_remove(struct platform_device *pdev)
{
	int ret = 0;
	int rc_idx;

	PCIE_GEN_DBG("PCIe:%s.\n", __func__);

	mutex_lock(&pcie_drv.drv_lock);

	ret = of_property_read_u32((&pdev->dev)->of_node,
				"cell-index", &rc_idx);
	if (ret) {
		pr_err("%s: Did not find RC index.\n", __func__);
		goto out;
	} else {
		pcie_drv.rc_num--;
		PCIE_GEN_DBG("%s: RC index is 0x%x.", __func__, rc_idx);
	}

	msm_pcie_irq_deinit(&msm_pcie_dev[rc_idx]);
	msm_pcie_vreg_deinit(&msm_pcie_dev[rc_idx]);
	msm_pcie_clk_deinit(&msm_pcie_dev[rc_idx]);
	msm_pcie_gpio_deinit(&msm_pcie_dev[rc_idx]);
	msm_pcie_release_resources(&msm_pcie_dev[rc_idx]);

out:
	mutex_unlock(&pcie_drv.drv_lock);

	return ret;
}

static struct of_device_id msm_pcie_match[] = {
	{	.compatible = "qcom,pci-msm",
	},
	{}
};

static struct platform_driver msm_pcie_driver = {
	.probe	= msm_pcie_probe,
	.remove	= msm_pcie_remove,
	.driver	= {
		.name		= "pci-msm",
		.owner		= THIS_MODULE,
		.of_match_table	= msm_pcie_match,
	},
};

int __init pcie_init(void)
{
	int ret = 0, i;
	char rc_name[MAX_RC_NAME_LEN];

	pr_alert("pcie:%s.\n", __func__);

	pcie_drv.rc_num = 0;
	mutex_init(&pcie_drv.drv_lock);
	mutex_init(&com_phy_lock);

	for (i = 0; i < MAX_RC_NUM; i++) {
		snprintf(rc_name, MAX_RC_NAME_LEN, "pcie%d-short", i);
#ifdef CONFIG_IPC_LOGGING
		msm_pcie_dev[i].ipc_log =
			ipc_log_context_create(PCIE_LOG_PAGES, rc_name, 0);
		if (msm_pcie_dev[i].ipc_log == NULL)
			pr_err("%s: unable to create IPC log context for %s\n",
				__func__, rc_name);
		else
			PCIE_DBG(&msm_pcie_dev[i],
				"PCIe IPC logging is enable for RC%d\n",
				i);
		snprintf(rc_name, MAX_RC_NAME_LEN, "pcie%d-long", i);
		msm_pcie_dev[i].ipc_log_long =
			ipc_log_context_create(PCIE_LOG_PAGES, rc_name, 0);
		if (msm_pcie_dev[i].ipc_log_long == NULL)
			pr_err("%s: unable to create IPC log context for %s\n",
				__func__, rc_name);
		else
			PCIE_DBG(&msm_pcie_dev[i],
				"PCIe IPC logging %s is enable for RC%d\n",
				rc_name, i);
		snprintf(rc_name, MAX_RC_NAME_LEN, "pcie%d-dump", i);
		msm_pcie_dev[i].ipc_log_dump =
			ipc_log_context_create(PCIE_LOG_PAGES, rc_name, 0);
		if (msm_pcie_dev[i].ipc_log_dump == NULL)
			pr_err("%s: unable to create IPC log context for %s\n",
				__func__, rc_name);
		else
			PCIE_DBG(&msm_pcie_dev[i],
				"PCIe IPC logging %s is enable for RC%d\n",
				rc_name, i);
#endif
		spin_lock_init(&msm_pcie_dev[i].cfg_lock);
		msm_pcie_dev[i].cfg_access = true;
		mutex_init(&msm_pcie_dev[i].setup_lock);
		mutex_init(&msm_pcie_dev[i].recovery_lock);
		spin_lock_init(&msm_pcie_dev[i].linkdown_lock);
		spin_lock_init(&msm_pcie_dev[i].wakeup_lock);
		spin_lock_init(&msm_pcie_dev[i].global_irq_lock);
		spin_lock_init(&msm_pcie_dev[i].aer_lock);
		msm_pcie_dev[i].drv_ready = false;
	}
	for (i = 0; i < MAX_RC_NUM * MAX_DEVICE_NUM; i++) {
		msm_pcie_dev_tbl[i].bdf = 0;
		msm_pcie_dev_tbl[i].dev = NULL;
		msm_pcie_dev_tbl[i].short_bdf = 0;
		msm_pcie_dev_tbl[i].sid = 0;
		msm_pcie_dev_tbl[i].domain = -1;
		msm_pcie_dev_tbl[i].conf_base = 0;
		msm_pcie_dev_tbl[i].phy_address = 0;
		msm_pcie_dev_tbl[i].dev_ctrlstts_offset = 0;
		msm_pcie_dev_tbl[i].event_reg = NULL;
		msm_pcie_dev_tbl[i].registered = true;
	}

	msm_pcie_debugfs_init();

	ret = platform_driver_register(&msm_pcie_driver);

	return ret;
}

static void __exit pcie_exit(void)
{
	PCIE_GEN_DBG("pcie:%s.\n", __func__);

	platform_driver_unregister(&msm_pcie_driver);

	msm_pcie_debugfs_exit();
}

subsys_initcall_sync(pcie_init);
module_exit(pcie_exit);


/* RC do not represent the right class; set it to PCI_CLASS_BRIDGE_PCI */
static void msm_pcie_fixup_early(struct pci_dev *dev)
{
	struct msm_pcie_dev_t *pcie_dev = PCIE_BUS_PRIV_DATA(dev->bus);
	PCIE_DBG(pcie_dev, "hdr_type %d\n", dev->hdr_type);
	if (dev->hdr_type == 1)
		dev->class = (dev->class & 0xff) | (PCI_CLASS_BRIDGE_PCI << 8);
}
DECLARE_PCI_FIXUP_EARLY(PCIE_VENDOR_ID_RCP, PCIE_DEVICE_ID_RCP,
			msm_pcie_fixup_early);

/* Suspend the PCIe link */
static int msm_pcie_pm_suspend(struct pci_dev *dev,
			void *user, void *data, u32 options)
{
	int ret = 0;
	u32 val = 0;
	int ret_l23;
	unsigned long irqsave_flags;
	struct msm_pcie_dev_t *pcie_dev = PCIE_BUS_PRIV_DATA(dev->bus);

	PCIE_DBG(pcie_dev, "RC%d: entry\n", pcie_dev->rc_idx);

	spin_lock_irqsave(&pcie_dev->aer_lock, irqsave_flags);
	pcie_dev->suspending = true;
	spin_unlock_irqrestore(&pcie_dev->aer_lock, irqsave_flags);

	if (!pcie_dev->power_on) {
		PCIE_DBG(pcie_dev,
			"PCIe: power of RC%d has been turned off.\n",
			pcie_dev->rc_idx);
		return ret;
	}

	if (dev && !(options & MSM_PCIE_CONFIG_NO_CFG_RESTORE)
		&& msm_pcie_confirm_linkup(pcie_dev, true, true)) {
		ret = pci_save_state(dev);
		pcie_dev->saved_state =	pci_store_saved_state(dev);
	}
	if (ret) {
		PCIE_ERR(pcie_dev, "PCIe: fail to save state of RC%d:%d.\n",
			pcie_dev->rc_idx, ret);
		pcie_dev->suspending = false;
		return ret;
	}

	spin_lock_irqsave(&pcie_dev->cfg_lock,
				pcie_dev->irqsave_flags);
	pcie_dev->cfg_access = false;
	spin_unlock_irqrestore(&pcie_dev->cfg_lock,
				pcie_dev->irqsave_flags);

	msm_pcie_write_mask(pcie_dev->elbi + PCIE20_ELBI_SYS_CTRL, 0,
				BIT(4));

	PCIE_DBG(pcie_dev, "RC%d: PME_TURNOFF_MSG is sent out\n",
		pcie_dev->rc_idx);

	ret_l23 = readl_poll_timeout((pcie_dev->parf
		+ PCIE20_PARF_PM_STTS), val, (val & BIT(5)), 10000, 100000);

	/* check L23_Ready */
	PCIE_DBG(pcie_dev, "RC%d: PCIE20_PARF_PM_STTS is 0x%x.\n",
		pcie_dev->rc_idx,
		readl_relaxed(pcie_dev->parf + PCIE20_PARF_PM_STTS));
	if (!ret_l23)
		PCIE_DBG(pcie_dev, "RC%d: PM_Enter_L23 is received\n",
			pcie_dev->rc_idx);
	else
		PCIE_DBG(pcie_dev, "RC%d: PM_Enter_L23 is NOT received\n",
			pcie_dev->rc_idx);

		msm_pcie_disable(pcie_dev, PM_PIPE_CLK | PM_CLK | PM_VREG);

	if (pcie_dev->use_pinctrl && pcie_dev->pins_sleep)
		pinctrl_select_state(pcie_dev->pinctrl,
					pcie_dev->pins_sleep);

	PCIE_DBG(pcie_dev, "RC%d: exit\n", pcie_dev->rc_idx);

	return ret;
}

static void msm_pcie_fixup_suspend(struct pci_dev *dev)
{
	int ret;
	struct msm_pcie_dev_t *pcie_dev = PCIE_BUS_PRIV_DATA(dev->bus);

	PCIE_DBG(pcie_dev, "RC%d\n", pcie_dev->rc_idx);

	if (pcie_dev->link_status != MSM_PCIE_LINK_ENABLED)
		return;

	spin_lock_irqsave(&pcie_dev->cfg_lock,
				pcie_dev->irqsave_flags);
	if (pcie_dev->disable_pc) {
		PCIE_DBG(pcie_dev,
			"RC%d: Skip suspend because of user request\n",
			pcie_dev->rc_idx);
		spin_unlock_irqrestore(&pcie_dev->cfg_lock,
				pcie_dev->irqsave_flags);
		return;
	}
	spin_unlock_irqrestore(&pcie_dev->cfg_lock,
				pcie_dev->irqsave_flags);

	mutex_lock(&pcie_dev->recovery_lock);

	ret = msm_pcie_pm_suspend(dev, NULL, NULL, 0);
	if (ret)
		PCIE_ERR(pcie_dev, "PCIe: RC%d got failure in suspend:%d.\n",
			pcie_dev->rc_idx, ret);

	mutex_unlock(&pcie_dev->recovery_lock);
}
DECLARE_PCI_FIXUP_SUSPEND(PCIE_VENDOR_ID_RCP, PCIE_DEVICE_ID_RCP,
			  msm_pcie_fixup_suspend);

/* Resume the PCIe link */
static int msm_pcie_pm_resume(struct pci_dev *dev,
			void *user, void *data, u32 options)
{
	int ret;
	struct msm_pcie_dev_t *pcie_dev = PCIE_BUS_PRIV_DATA(dev->bus);

	PCIE_DBG(pcie_dev, "RC%d: entry\n", pcie_dev->rc_idx);

	if (pcie_dev->use_pinctrl && pcie_dev->pins_default)
		pinctrl_select_state(pcie_dev->pinctrl,
					pcie_dev->pins_default);

	spin_lock_irqsave(&pcie_dev->cfg_lock,
				pcie_dev->irqsave_flags);
	pcie_dev->cfg_access = true;
	spin_unlock_irqrestore(&pcie_dev->cfg_lock,
				pcie_dev->irqsave_flags);

	ret = msm_pcie_enable(pcie_dev, PM_PIPE_CLK | PM_CLK | PM_VREG);
	if (ret) {
		PCIE_ERR(pcie_dev,
			"PCIe: RC%d fail to enable PCIe link in resume.\n",
			pcie_dev->rc_idx);
		return ret;
	} else {
		pcie_dev->suspending = false;
		PCIE_DBG(pcie_dev,
			"dev->bus->number = %d dev->bus->primary = %d\n",
			 dev->bus->number, dev->bus->primary);

		if (!(options & MSM_PCIE_CONFIG_NO_CFG_RESTORE)) {
			PCIE_DBG(pcie_dev,
				"RC%d: entry of PCI framework restore state\n",
				pcie_dev->rc_idx);

			pci_load_and_free_saved_state(dev,
					&pcie_dev->saved_state);
			pci_restore_state(dev);

			PCIE_DBG(pcie_dev,
				"RC%d: exit of PCI framework restore state\n",
				pcie_dev->rc_idx);
		}
	}

	if (pcie_dev->bridge_found) {
		PCIE_DBG(pcie_dev,
			"RC%d: entry of PCIe recover config\n",
			pcie_dev->rc_idx);

		msm_pcie_recover_config(dev);

		PCIE_DBG(pcie_dev,
			"RC%d: exit of PCIe recover config\n",
			pcie_dev->rc_idx);
	}

	PCIE_DBG(pcie_dev, "RC%d: exit\n", pcie_dev->rc_idx);

	return ret;
}

void msm_pcie_fixup_resume(struct pci_dev *dev)
{
	int ret;
	struct msm_pcie_dev_t *pcie_dev = PCIE_BUS_PRIV_DATA(dev->bus);

	PCIE_DBG(pcie_dev, "RC%d\n", pcie_dev->rc_idx);

	if ((pcie_dev->link_status != MSM_PCIE_LINK_DISABLED) ||
		pcie_dev->user_suspend)
		return;

	mutex_lock(&pcie_dev->recovery_lock);
	ret = msm_pcie_pm_resume(dev, NULL, NULL, 0);
	if (ret)
		PCIE_ERR(pcie_dev,
			"PCIe: RC%d got failure in fixup resume:%d.\n",
			pcie_dev->rc_idx, ret);

	mutex_unlock(&pcie_dev->recovery_lock);
}
DECLARE_PCI_FIXUP_RESUME(PCIE_VENDOR_ID_RCP, PCIE_DEVICE_ID_RCP,
				 msm_pcie_fixup_resume);

void msm_pcie_fixup_resume_early(struct pci_dev *dev)
{
	int ret;
	struct msm_pcie_dev_t *pcie_dev = PCIE_BUS_PRIV_DATA(dev->bus);

	PCIE_DBG(pcie_dev, "RC%d\n", pcie_dev->rc_idx);

	if ((pcie_dev->link_status != MSM_PCIE_LINK_DISABLED) ||
		pcie_dev->user_suspend)
		return;

	mutex_lock(&pcie_dev->recovery_lock);
	ret = msm_pcie_pm_resume(dev, NULL, NULL, 0);
	if (ret)
		PCIE_ERR(pcie_dev, "PCIe: RC%d got failure in resume:%d.\n",
			pcie_dev->rc_idx, ret);

	mutex_unlock(&pcie_dev->recovery_lock);
}
DECLARE_PCI_FIXUP_RESUME_EARLY(PCIE_VENDOR_ID_RCP, PCIE_DEVICE_ID_RCP,
				 msm_pcie_fixup_resume_early);

int msm_pcie_pm_control(enum msm_pcie_pm_opt pm_opt, u32 busnr, void *user,
			void *data, u32 options)
{
	int i, ret = 0;
	struct pci_dev *dev;
	u32 rc_idx = 0;
	struct msm_pcie_dev_t *pcie_dev;

	PCIE_GEN_DBG("PCIe: pm_opt:%d;busnr:%d;options:%d\n",
		pm_opt, busnr, options);


	if (!user) {
		pr_err("PCIe: endpoint device is NULL\n");
		ret = -ENODEV;
		goto out;
	}

	pcie_dev = PCIE_BUS_PRIV_DATA(((struct pci_dev *)user)->bus);

	if (pcie_dev) {
		rc_idx = pcie_dev->rc_idx;
		PCIE_DBG(pcie_dev,
			"PCIe: RC%d: pm_opt:%d;busnr:%d;options:%d\n",
			rc_idx, pm_opt, busnr, options);
	} else {
		pr_err(
			"PCIe: did not find RC for pci endpoint device.\n"
			);
		ret = -ENODEV;
		goto out;
	}

	for (i = 0; i < MAX_DEVICE_NUM; i++) {
		if (!busnr)
			break;
		if (user == pcie_dev->pcidev_table[i].dev) {
			if (busnr == pcie_dev->pcidev_table[i].bdf >> 24) {
				break;
			} else {
				PCIE_ERR(pcie_dev,
					"PCIe: RC%d: bus number %d does not match with the expected value %d\n",
					pcie_dev->rc_idx, busnr,
					pcie_dev->pcidev_table[i].bdf >> 24);
				ret = MSM_PCIE_ERROR;
				goto out;
			}
		}
	}

	if (i == MAX_DEVICE_NUM) {
		PCIE_ERR(pcie_dev,
			"PCIe: RC%d: endpoint device was not found in device table",
			pcie_dev->rc_idx);
		ret = MSM_PCIE_ERROR;
		goto out;
	}

	dev = msm_pcie_dev[rc_idx].dev;

	if (!msm_pcie_dev[rc_idx].drv_ready) {
		PCIE_ERR(&msm_pcie_dev[rc_idx],
			"RC%d has not been successfully probed yet\n",
			rc_idx);
		return -EPROBE_DEFER;
	}

	switch (pm_opt) {
	case MSM_PCIE_SUSPEND:
		PCIE_DBG(&msm_pcie_dev[rc_idx],
			"User of RC%d requests to suspend the link\n", rc_idx);
		if (msm_pcie_dev[rc_idx].link_status != MSM_PCIE_LINK_ENABLED)
			PCIE_DBG(&msm_pcie_dev[rc_idx],
				"PCIe: RC%d: requested to suspend when link is not enabled:%d.\n",
				rc_idx, msm_pcie_dev[rc_idx].link_status);

		if (!msm_pcie_dev[rc_idx].power_on) {
			PCIE_ERR(&msm_pcie_dev[rc_idx],
				"PCIe: RC%d: requested to suspend when link is powered down:%d.\n",
				rc_idx, msm_pcie_dev[rc_idx].link_status);
			break;
		}

		if (msm_pcie_dev[rc_idx].pending_ep_reg) {
			PCIE_DBG(&msm_pcie_dev[rc_idx],
				"PCIe: RC%d: request to suspend the link is rejected\n",
				rc_idx);
			break;
		}

		if (pcie_dev->num_active_ep) {
			PCIE_DBG(pcie_dev,
				"RC%d: an EP requested to suspend the link, but other EPs are still active: %d\n",
				pcie_dev->rc_idx, pcie_dev->num_active_ep);
			return ret;
		}

		msm_pcie_dev[rc_idx].user_suspend = true;

		mutex_lock(&msm_pcie_dev[rc_idx].recovery_lock);

		ret = msm_pcie_pm_suspend(dev, user, data, options);
		if (ret) {
			PCIE_ERR(&msm_pcie_dev[rc_idx],
				"PCIe: RC%d: user failed to suspend the link.\n",
				rc_idx);
			msm_pcie_dev[rc_idx].user_suspend = false;
		}

		mutex_unlock(&msm_pcie_dev[rc_idx].recovery_lock);
		break;
	case MSM_PCIE_RESUME:
		PCIE_DBG(&msm_pcie_dev[rc_idx],
			"User of RC%d requests to resume the link\n", rc_idx);
		if (msm_pcie_dev[rc_idx].link_status !=
					MSM_PCIE_LINK_DISABLED) {
			PCIE_ERR(&msm_pcie_dev[rc_idx],
				"PCIe: RC%d: requested to resume when link is not disabled:%d. Number of active EP(s): %d\n",
				rc_idx, msm_pcie_dev[rc_idx].link_status,
				msm_pcie_dev[rc_idx].num_active_ep);
			break;
		}

		mutex_lock(&msm_pcie_dev[rc_idx].recovery_lock);
		ret = msm_pcie_pm_resume(dev, user, data, options);
		if (ret) {
			PCIE_ERR(&msm_pcie_dev[rc_idx],
				"PCIe: RC%d: user failed to resume the link.\n",
				rc_idx);
		} else {
			PCIE_DBG(&msm_pcie_dev[rc_idx],
				"PCIe: RC%d: user succeeded to resume the link.\n",
				rc_idx);

			msm_pcie_dev[rc_idx].user_suspend = false;
		}

		mutex_unlock(&msm_pcie_dev[rc_idx].recovery_lock);

		break;
	case MSM_PCIE_DISABLE_PC:
		PCIE_DBG(&msm_pcie_dev[rc_idx],
			"User of RC%d requests to keep the link always alive.\n",
			rc_idx);
		spin_lock_irqsave(&msm_pcie_dev[rc_idx].cfg_lock,
				msm_pcie_dev[rc_idx].irqsave_flags);
		if (msm_pcie_dev[rc_idx].suspending) {
			PCIE_ERR(&msm_pcie_dev[rc_idx],
				"PCIe: RC%d Link has been suspended before request\n",
				rc_idx);
			ret = MSM_PCIE_ERROR;
		} else {
			msm_pcie_dev[rc_idx].disable_pc = true;
		}
		spin_unlock_irqrestore(&msm_pcie_dev[rc_idx].cfg_lock,
				msm_pcie_dev[rc_idx].irqsave_flags);
		break;
	case MSM_PCIE_ENABLE_PC:
		PCIE_DBG(&msm_pcie_dev[rc_idx],
			"User of RC%d cancels the request of alive link.\n",
			rc_idx);
		spin_lock_irqsave(&msm_pcie_dev[rc_idx].cfg_lock,
				msm_pcie_dev[rc_idx].irqsave_flags);
		msm_pcie_dev[rc_idx].disable_pc = false;
		spin_unlock_irqrestore(&msm_pcie_dev[rc_idx].cfg_lock,
				msm_pcie_dev[rc_idx].irqsave_flags);
		break;
	default:
		PCIE_ERR(&msm_pcie_dev[rc_idx],
			"PCIe: RC%d: unsupported pm operation:%d.\n",
			rc_idx, pm_opt);
		ret = -ENODEV;
		goto out;
	}

out:
	return ret;
}
EXPORT_SYMBOL(msm_pcie_pm_control);

int msm_pcie_register_event(struct msm_pcie_register_event *reg)
{
	int i, ret = 0;
	struct msm_pcie_dev_t *pcie_dev;

	if (!reg) {
		pr_err("PCIe: Event registration is NULL\n");
		return -ENODEV;
	}

	if (!reg->user) {
		pr_err("PCIe: User of event registration is NULL\n");
		return -ENODEV;
	}

	pcie_dev = PCIE_BUS_PRIV_DATA(((struct pci_dev *)reg->user)->bus);

	if (!pcie_dev) {
		PCIE_ERR(pcie_dev, "%s",
			"PCIe: did not find RC for pci endpoint device.\n");
		return -ENODEV;
	}

	if (pcie_dev->num_ep > 1) {
		for (i = 0; i < MAX_DEVICE_NUM; i++) {
			if (reg->user ==
				pcie_dev->pcidev_table[i].dev) {
				pcie_dev->event_reg =
					pcie_dev->pcidev_table[i].event_reg;

				if (!pcie_dev->event_reg) {
					pcie_dev->pcidev_table[i].registered =
						true;

					pcie_dev->num_active_ep++;
					PCIE_DBG(pcie_dev,
						"PCIe: RC%d: number of active EP(s): %d.\n",
						pcie_dev->rc_idx,
						pcie_dev->num_active_ep);
				}

				pcie_dev->event_reg = reg;
				pcie_dev->pcidev_table[i].event_reg = reg;
				PCIE_DBG(pcie_dev,
					"Event 0x%x is registered for RC %d\n",
					reg->events,
					pcie_dev->rc_idx);

				break;
			}
		}

		if (pcie_dev->pending_ep_reg) {
			for (i = 0; i < MAX_DEVICE_NUM; i++)
				if (!pcie_dev->pcidev_table[i].registered)
					break;

			if (i == MAX_DEVICE_NUM)
				pcie_dev->pending_ep_reg = false;
		}
	} else {
		pcie_dev->event_reg = reg;
		PCIE_DBG(pcie_dev,
			"Event 0x%x is registered for RC %d\n", reg->events,
			pcie_dev->rc_idx);
	}

	return ret;
}
EXPORT_SYMBOL(msm_pcie_register_event);

int msm_pcie_deregister_event(struct msm_pcie_register_event *reg)
{
	int i, ret = 0;
	struct msm_pcie_dev_t *pcie_dev;

	if (!reg) {
		pr_err("PCIe: Event deregistration is NULL\n");
		return -ENODEV;
	}

	if (!reg->user) {
		pr_err("PCIe: User of event deregistration is NULL\n");
		return -ENODEV;
	}

	pcie_dev = PCIE_BUS_PRIV_DATA(((struct pci_dev *)reg->user)->bus);

	if (!pcie_dev) {
		PCIE_ERR(pcie_dev, "%s",
			"PCIe: did not find RC for pci endpoint device.\n");
		return -ENODEV;
	}

	if (pcie_dev->num_ep > 1) {
		for (i = 0; i < MAX_DEVICE_NUM; i++) {
			if (reg->user == pcie_dev->pcidev_table[i].dev) {
				if (pcie_dev->pcidev_table[i].event_reg) {
					pcie_dev->num_active_ep--;
					PCIE_DBG(pcie_dev,
						"PCIe: RC%d: number of active EP(s) left: %d.\n",
						pcie_dev->rc_idx,
						pcie_dev->num_active_ep);
				}

				pcie_dev->event_reg = NULL;
				pcie_dev->pcidev_table[i].event_reg = NULL;
				PCIE_DBG(pcie_dev,
					"Event is deregistered for RC %d\n",
					pcie_dev->rc_idx);

				break;
			}
		}
	} else {
		pcie_dev->event_reg = NULL;
		PCIE_DBG(pcie_dev, "Event is deregistered for RC %d\n",
				pcie_dev->rc_idx);
	}

	return ret;
}
EXPORT_SYMBOL(msm_pcie_deregister_event);

int msm_pcie_recover_config(struct pci_dev *dev)
{
	int ret = 0;
	struct msm_pcie_dev_t *pcie_dev;

	if (dev) {
		pcie_dev = PCIE_BUS_PRIV_DATA(dev->bus);
		PCIE_DBG(pcie_dev,
			"Recovery for the link of RC%d\n", pcie_dev->rc_idx);
	} else {
		pr_err("PCIe: the input pci dev is NULL.\n");
		return -ENODEV;
	}

	if (msm_pcie_confirm_linkup(pcie_dev, true, true)) {
		PCIE_DBG(pcie_dev,
			"Recover config space of RC%d and its EP\n",
			pcie_dev->rc_idx);
		pcie_dev->shadow_en = false;
		PCIE_DBG(pcie_dev, "Recover RC%d\n", pcie_dev->rc_idx);
		msm_pcie_cfg_recover(pcie_dev, true);
		PCIE_DBG(pcie_dev, "Recover EP of RC%d\n", pcie_dev->rc_idx);
		msm_pcie_cfg_recover(pcie_dev, false);
		PCIE_DBG(pcie_dev,
			"Refreshing the saved config space in PCI framework for RC%d and its EP\n",
			pcie_dev->rc_idx);
		pci_save_state(pcie_dev->dev);
		pci_save_state(dev);
		pcie_dev->shadow_en = true;
		PCIE_DBG(pcie_dev, "Turn on shadow for RC%d\n",
			pcie_dev->rc_idx);
	} else {
		PCIE_ERR(pcie_dev,
			"PCIe: the link of RC%d is not up yet; can't recover config space.\n",
			pcie_dev->rc_idx);
		ret = -ENODEV;
	}

	return ret;
}
EXPORT_SYMBOL(msm_pcie_recover_config);

int msm_pcie_shadow_control(struct pci_dev *dev, bool enable)
{
	int ret = 0;
	struct msm_pcie_dev_t *pcie_dev;

	if (dev) {
		pcie_dev = PCIE_BUS_PRIV_DATA(dev->bus);
		PCIE_DBG(pcie_dev,
			"User requests to %s shadow\n",
			enable ? "enable" : "disable");
	} else {
		pr_err("PCIe: the input pci dev is NULL.\n");
		return -ENODEV;
	}

	PCIE_DBG(pcie_dev,
		"The shadowing of RC%d is %s enabled currently.\n",
		pcie_dev->rc_idx, pcie_dev->shadow_en ? "" : "not");

	pcie_dev->shadow_en = enable;

	PCIE_DBG(pcie_dev,
		"Shadowing of RC%d is turned %s upon user's request.\n",
		pcie_dev->rc_idx, enable ? "on" : "off");

	return ret;
}
EXPORT_SYMBOL(msm_pcie_shadow_control);
