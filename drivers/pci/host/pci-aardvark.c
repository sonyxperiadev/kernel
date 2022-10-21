/*
 * Driver for the Aardvark PCIe controller, used on Marvell Armada
 * 3700.
 *
 * Copyright (C) 2016 Marvell
 *
 * Author: Hezi Shahmoon <hezi.shahmoon@marvell.com>
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/delay.h>
#include <linux/gpio/consumer.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/irqdomain.h>
#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/of_pci.h>

/* PCIe core registers */
#define PCIE_CORE_CMD_STATUS_REG				0x4
#define     PCIE_CORE_CMD_IO_ACCESS_EN				BIT(0)
#define     PCIE_CORE_CMD_MEM_ACCESS_EN				BIT(1)
#define     PCIE_CORE_CMD_MEM_IO_REQ_EN				BIT(2)
#define PCIE_CORE_PCIEXP_CAP					0xc0
#define PCIE_CORE_ERR_CAPCTL_REG				0x118
#define     PCIE_CORE_ERR_CAPCTL_ECRC_CHK_TX			BIT(5)
#define     PCIE_CORE_ERR_CAPCTL_ECRC_CHK_TX_EN			BIT(6)
#define     PCIE_CORE_ERR_CAPCTL_ECRC_CHCK			BIT(7)
#define     PCIE_CORE_ERR_CAPCTL_ECRC_CHCK_RCV			BIT(8)

/* PIO registers base address and register offsets */
#define PIO_BASE_ADDR				0x4000
#define PIO_CTRL				(PIO_BASE_ADDR + 0x0)
#define   PIO_CTRL_TYPE_MASK			GENMASK(3, 0)
#define   PIO_CTRL_ADDR_WIN_DISABLE		BIT(24)
#define PIO_STAT				(PIO_BASE_ADDR + 0x4)
#define   PIO_COMPLETION_STATUS_SHIFT		7
#define   PIO_COMPLETION_STATUS_MASK		GENMASK(9, 7)
#define   PIO_COMPLETION_STATUS_OK		0
#define   PIO_COMPLETION_STATUS_UR		1
#define   PIO_COMPLETION_STATUS_CRS		2
#define   PIO_COMPLETION_STATUS_CA		4
#define   PIO_NON_POSTED_REQ			BIT(10)
#define   PIO_ERR_STATUS			BIT(11)
#define PIO_ADDR_LS				(PIO_BASE_ADDR + 0x8)
#define PIO_ADDR_MS				(PIO_BASE_ADDR + 0xc)
#define PIO_WR_DATA				(PIO_BASE_ADDR + 0x10)
#define PIO_WR_DATA_STRB			(PIO_BASE_ADDR + 0x14)
#define PIO_RD_DATA				(PIO_BASE_ADDR + 0x18)
#define PIO_START				(PIO_BASE_ADDR + 0x1c)
#define PIO_ISR					(PIO_BASE_ADDR + 0x20)
#define PIO_ISRM				(PIO_BASE_ADDR + 0x24)

/* Aardvark Control registers */
#define CONTROL_BASE_ADDR			0x4800
#define PCIE_CORE_CTRL0_REG			(CONTROL_BASE_ADDR + 0x0)
#define     PCIE_GEN_SEL_MSK			0x3
#define     PCIE_GEN_SEL_SHIFT			0x0
#define     SPEED_GEN_1				0
#define     SPEED_GEN_2				1
#define     SPEED_GEN_3				2
#define     IS_RC_MSK				1
#define     IS_RC_SHIFT				2
#define     LANE_CNT_MSK			0x18
#define     LANE_CNT_SHIFT			0x3
#define     LANE_COUNT_1			(0 << LANE_CNT_SHIFT)
#define     LANE_COUNT_2			(1 << LANE_CNT_SHIFT)
#define     LANE_COUNT_4			(2 << LANE_CNT_SHIFT)
#define     LANE_COUNT_8			(3 << LANE_CNT_SHIFT)
#define     LINK_TRAINING_EN			BIT(6)
#define     LEGACY_INTA				BIT(28)
#define     LEGACY_INTB				BIT(29)
#define     LEGACY_INTC				BIT(30)
#define     LEGACY_INTD				BIT(31)
#define PCIE_CORE_CTRL1_REG			(CONTROL_BASE_ADDR + 0x4)
#define     HOT_RESET_GEN			BIT(0)
#define PCIE_CORE_CTRL2_REG			(CONTROL_BASE_ADDR + 0x8)
#define     PCIE_CORE_CTRL2_RESERVED		0x7
#define     PCIE_CORE_CTRL2_TD_ENABLE		BIT(4)
#define     PCIE_CORE_CTRL2_STRICT_ORDER_ENABLE	BIT(5)
#define     PCIE_CORE_CTRL2_OB_WIN_ENABLE	BIT(6)
#define     PCIE_CORE_CTRL2_MSI_ENABLE		BIT(10)
#define PCIE_ISR0_REG				(CONTROL_BASE_ADDR + 0x40)
#define PCIE_ISR0_MASK_REG			(CONTROL_BASE_ADDR + 0x44)
#define     PCIE_ISR0_MSI_INT_PENDING		BIT(24)
#define     PCIE_ISR0_INTX_ASSERT(val)		BIT(16 + (val))
#define     PCIE_ISR0_INTX_DEASSERT(val)	BIT(20 + (val))
#define     PCIE_ISR0_ALL_MASK			GENMASK(31, 0)
#define PCIE_ISR1_REG				(CONTROL_BASE_ADDR + 0x48)
#define PCIE_ISR1_MASK_REG			(CONTROL_BASE_ADDR + 0x4C)
#define     PCIE_ISR1_POWER_STATE_CHANGE	BIT(4)
#define     PCIE_ISR1_FLUSH			BIT(5)
#define     PCIE_ISR1_INTX_ASSERT(val)		BIT(8 + (val))
#define     PCIE_ISR1_ALL_MASK			GENMASK(31, 0)
#define PCIE_MSI_ADDR_LOW_REG			(CONTROL_BASE_ADDR + 0x50)
#define PCIE_MSI_ADDR_HIGH_REG			(CONTROL_BASE_ADDR + 0x54)
#define PCIE_MSI_STATUS_REG			(CONTROL_BASE_ADDR + 0x58)
#define PCIE_MSI_MASK_REG			(CONTROL_BASE_ADDR + 0x5C)
#define PCIE_MSI_PAYLOAD_REG			(CONTROL_BASE_ADDR + 0x9C)
#define     PCIE_MSI_DATA_MASK			GENMASK(15, 0)

/* PCIe window configuration */
#define OB_WIN_BASE_ADDR			0x4c00
#define OB_WIN_BLOCK_SIZE			0x20
#define OB_WIN_COUNT				8
#define OB_WIN_REG_ADDR(win, offset)		(OB_WIN_BASE_ADDR + \
						 OB_WIN_BLOCK_SIZE * (win) + \
						 (offset))
#define OB_WIN_MATCH_LS(win)			OB_WIN_REG_ADDR(win, 0x00)
#define     OB_WIN_ENABLE			BIT(0)
#define OB_WIN_MATCH_MS(win)			OB_WIN_REG_ADDR(win, 0x04)
#define OB_WIN_REMAP_LS(win)			OB_WIN_REG_ADDR(win, 0x08)
#define OB_WIN_REMAP_MS(win)			OB_WIN_REG_ADDR(win, 0x0c)
#define OB_WIN_MASK_LS(win)			OB_WIN_REG_ADDR(win, 0x10)
#define OB_WIN_MASK_MS(win)			OB_WIN_REG_ADDR(win, 0x14)
#define OB_WIN_ACTIONS(win)			OB_WIN_REG_ADDR(win, 0x18)
#define OB_WIN_DEFAULT_ACTIONS			(OB_WIN_ACTIONS(OB_WIN_COUNT-1) + 0x4)
#define     OB_WIN_FUNC_NUM_MASK		GENMASK(31, 24)
#define     OB_WIN_FUNC_NUM_SHIFT		24
#define     OB_WIN_FUNC_NUM_ENABLE		BIT(23)
#define     OB_WIN_BUS_NUM_BITS_MASK		GENMASK(22, 20)
#define     OB_WIN_BUS_NUM_BITS_SHIFT		20
#define     OB_WIN_MSG_CODE_ENABLE		BIT(22)
#define     OB_WIN_MSG_CODE_MASK		GENMASK(21, 14)
#define     OB_WIN_MSG_CODE_SHIFT		14
#define     OB_WIN_MSG_PAYLOAD_LEN		BIT(12)
#define     OB_WIN_ATTR_ENABLE			BIT(11)
#define     OB_WIN_ATTR_TC_MASK			GENMASK(10, 8)
#define     OB_WIN_ATTR_TC_SHIFT		8
#define     OB_WIN_ATTR_RELAXED			BIT(7)
#define     OB_WIN_ATTR_NOSNOOP			BIT(6)
#define     OB_WIN_ATTR_POISON			BIT(5)
#define     OB_WIN_ATTR_IDO			BIT(4)
#define     OB_WIN_TYPE_MASK			GENMASK(3, 0)
#define     OB_WIN_TYPE_SHIFT			0
#define     OB_WIN_TYPE_MEM			0x0
#define     OB_WIN_TYPE_IO			0x4
#define     OB_WIN_TYPE_CONFIG_TYPE0		0x8
#define     OB_WIN_TYPE_CONFIG_TYPE1		0x9
#define     OB_WIN_TYPE_MSG			0xc

/* LMI registers base address and register offsets */
#define LMI_BASE_ADDR				0x6000
#define CFG_REG					(LMI_BASE_ADDR + 0x0)
#define     LTSSM_SHIFT				24
#define     LTSSM_MASK				0x3f
#define     RC_BAR_CONFIG			0x300

/* LTSSM values in CFG_REG */
enum {
	LTSSM_DETECT_QUIET			= 0x0,
	LTSSM_DETECT_ACTIVE			= 0x1,
	LTSSM_POLLING_ACTIVE			= 0x2,
	LTSSM_POLLING_COMPLIANCE		= 0x3,
	LTSSM_POLLING_CONFIGURATION		= 0x4,
	LTSSM_CONFIG_LINKWIDTH_START		= 0x5,
	LTSSM_CONFIG_LINKWIDTH_ACCEPT		= 0x6,
	LTSSM_CONFIG_LANENUM_ACCEPT		= 0x7,
	LTSSM_CONFIG_LANENUM_WAIT		= 0x8,
	LTSSM_CONFIG_COMPLETE			= 0x9,
	LTSSM_CONFIG_IDLE			= 0xa,
	LTSSM_RECOVERY_RCVR_LOCK		= 0xb,
	LTSSM_RECOVERY_SPEED			= 0xc,
	LTSSM_RECOVERY_RCVR_CFG			= 0xd,
	LTSSM_RECOVERY_IDLE			= 0xe,
	LTSSM_L0				= 0x10,
	LTSSM_RX_L0S_ENTRY			= 0x11,
	LTSSM_RX_L0S_IDLE			= 0x12,
	LTSSM_RX_L0S_FTS			= 0x13,
	LTSSM_TX_L0S_ENTRY			= 0x14,
	LTSSM_TX_L0S_IDLE			= 0x15,
	LTSSM_TX_L0S_FTS			= 0x16,
	LTSSM_L1_ENTRY				= 0x17,
	LTSSM_L1_IDLE				= 0x18,
	LTSSM_L2_IDLE				= 0x19,
	LTSSM_L2_TRANSMIT_WAKE			= 0x1a,
	LTSSM_DISABLED				= 0x20,
	LTSSM_LOOPBACK_ENTRY_MASTER		= 0x21,
	LTSSM_LOOPBACK_ACTIVE_MASTER		= 0x22,
	LTSSM_LOOPBACK_EXIT_MASTER		= 0x23,
	LTSSM_LOOPBACK_ENTRY_SLAVE		= 0x24,
	LTSSM_LOOPBACK_ACTIVE_SLAVE		= 0x25,
	LTSSM_LOOPBACK_EXIT_SLAVE		= 0x26,
	LTSSM_HOT_RESET				= 0x27,
	LTSSM_RECOVERY_EQUALIZATION_PHASE0	= 0x28,
	LTSSM_RECOVERY_EQUALIZATION_PHASE1	= 0x29,
	LTSSM_RECOVERY_EQUALIZATION_PHASE2	= 0x2a,
	LTSSM_RECOVERY_EQUALIZATION_PHASE3	= 0x2b,
};

/* PCIe core controller registers */
#define CTRL_CORE_BASE_ADDR			0x18000
#define CTRL_CONFIG_REG				(CTRL_CORE_BASE_ADDR + 0x0)
#define     CTRL_MODE_SHIFT			0x0
#define     CTRL_MODE_MASK			0x1
#define     PCIE_CORE_MODE_DIRECT		0x0
#define     PCIE_CORE_MODE_COMMAND		0x1

/* PCIe Central Interrupts Registers */
#define CENTRAL_INT_BASE_ADDR			0x1b000
#define HOST_CTRL_INT_STATUS_REG		(CENTRAL_INT_BASE_ADDR + 0x0)
#define HOST_CTRL_INT_MASK_REG			(CENTRAL_INT_BASE_ADDR + 0x4)
#define     PCIE_IRQ_CMDQ_INT			BIT(0)
#define     PCIE_IRQ_MSI_STATUS_INT		BIT(1)
#define     PCIE_IRQ_CMD_SENT_DONE		BIT(3)
#define     PCIE_IRQ_DMA_INT			BIT(4)
#define     PCIE_IRQ_IB_DXFERDONE		BIT(5)
#define     PCIE_IRQ_OB_DXFERDONE		BIT(6)
#define     PCIE_IRQ_OB_RXFERDONE		BIT(7)
#define     PCIE_IRQ_COMPQ_INT			BIT(12)
#define     PCIE_IRQ_DIR_RD_DDR_DET		BIT(13)
#define     PCIE_IRQ_DIR_WR_DDR_DET		BIT(14)
#define     PCIE_IRQ_CORE_INT			BIT(16)
#define     PCIE_IRQ_CORE_INT_PIO		BIT(17)
#define     PCIE_IRQ_DPMU_INT			BIT(18)
#define     PCIE_IRQ_PCIE_MIS_INT		BIT(19)
#define     PCIE_IRQ_MSI_INT1_DET		BIT(20)
#define     PCIE_IRQ_MSI_INT2_DET		BIT(21)
#define     PCIE_IRQ_RC_DBELL_DET		BIT(22)
#define     PCIE_IRQ_EP_STATUS			BIT(23)
#define     PCIE_IRQ_ALL_MASK			GENMASK(31, 0)
#define     PCIE_IRQ_ENABLE_INTS_MASK		PCIE_IRQ_CORE_INT

/* Transaction types */
#define PCIE_CONFIG_RD_TYPE0			0x8
#define PCIE_CONFIG_RD_TYPE1			0x9
#define PCIE_CONFIG_WR_TYPE0			0xa
#define PCIE_CONFIG_WR_TYPE1			0xb

#define PCIE_CONF_BUS(bus)			(((bus) & 0xff) << 20)
#define PCIE_CONF_DEV(dev)			(((dev) & 0x1f) << 15)
#define PCIE_CONF_FUNC(fun)			(((fun) & 0x7)	<< 12)
#define PCIE_CONF_REG(reg)			((reg) & 0xffc)
#define PCIE_CONF_ADDR(bus, devfn, where)	\
	(PCIE_CONF_BUS(bus) | PCIE_CONF_DEV(PCI_SLOT(devfn))	| \
	 PCIE_CONF_FUNC(PCI_FUNC(devfn)) | PCIE_CONF_REG(where))

#define PIO_RETRY_CNT			750000 /* 1.5 s */
#define PIO_RETRY_DELAY			2 /* 2 us*/

#define LINK_WAIT_MAX_RETRIES		10
#define LINK_WAIT_USLEEP_MIN		90000
#define LINK_WAIT_USLEEP_MAX		100000

#define MSI_IRQ_NUM			32

struct advk_pcie {
	struct platform_device *pdev;
	void __iomem *base;
	struct list_head resources;
	struct {
		phys_addr_t match;
		phys_addr_t remap;
		phys_addr_t mask;
		u32 actions;
	} wins[OB_WIN_COUNT];
	u8 wins_count;
	struct irq_domain *irq_domain;
	struct irq_chip irq_chip;
	raw_spinlock_t irq_lock;
	struct irq_domain *msi_domain;
	struct irq_domain *msi_inner_domain;
	struct irq_chip msi_bottom_irq_chip;
	struct irq_chip msi_irq_chip;
	struct msi_domain_info msi_domain_info;
	DECLARE_BITMAP(msi_used, MSI_IRQ_NUM);
	struct mutex msi_used_lock;
	u16 msi_msg;
	int root_bus_nr;
	int link_gen;
	struct gpio_desc *reset_gpio;
};

static inline void advk_writel(struct advk_pcie *pcie, u32 val, u64 reg)
{
	writel(val, pcie->base + reg);
}

static inline u32 advk_readl(struct advk_pcie *pcie, u64 reg)
{
	return readl(pcie->base + reg);
}

static u8 advk_pcie_ltssm_state(struct advk_pcie *pcie)
{
	u32 val;
	u8 ltssm_state;

	val = advk_readl(pcie, CFG_REG);
	ltssm_state = (val >> LTSSM_SHIFT) & LTSSM_MASK;
	return ltssm_state;
}

static inline bool advk_pcie_link_up(struct advk_pcie *pcie)
{
	/* check if LTSSM is in normal operation - some L* state */
	u8 ltssm_state = advk_pcie_ltssm_state(pcie);
	return ltssm_state >= LTSSM_L0 && ltssm_state < LTSSM_DISABLED;
}

static inline bool advk_pcie_link_training(struct advk_pcie *pcie)
{
	/*
	  * According to PCIe Base specification 3.0, Table 4-14: Link
	  * Status Mapped to the LTSSM is Link Training mapped to LTSSM
	  * Configuration and Recovery states.
	  */
	u8 ltssm_state = advk_pcie_ltssm_state(pcie);
	return ((ltssm_state >= LTSSM_CONFIG_LINKWIDTH_START &&
		  ltssm_state < LTSSM_L0) ||
		(ltssm_state >= LTSSM_RECOVERY_EQUALIZATION_PHASE0 &&
		  ltssm_state <= LTSSM_RECOVERY_EQUALIZATION_PHASE3));
}

static int advk_pcie_wait_for_link(struct advk_pcie *pcie)
{
	int retries;

	/* check if the link is up or not */
	for (retries = 0; retries < LINK_WAIT_MAX_RETRIES; retries++) {
		if (advk_pcie_link_up(pcie))
			return 0;

		usleep_range(LINK_WAIT_USLEEP_MIN, LINK_WAIT_USLEEP_MAX);
	}

	return -ETIMEDOUT;
}

static void advk_pcie_issue_perst(struct advk_pcie *pcie)
{
	if (!pcie->reset_gpio)
		return;

	/* 10ms delay is needed for some cards */
	dev_info(&pcie->pdev->dev, "issuing PERST via reset GPIO for 10ms\n");
	gpiod_set_value_cansleep(pcie->reset_gpio, 1);
	usleep_range(10000, 11000);
	gpiod_set_value_cansleep(pcie->reset_gpio, 0);
}

static void advk_pcie_train_link(struct advk_pcie *pcie)
{
	struct device *dev = &pcie->pdev->dev;
	u32 reg;
	int ret;

	/*
	 * Setup PCIe rev / gen compliance based on device tree property
	 * 'max-link-speed' which also forces maximal link speed.
	 */
	reg = advk_readl(pcie, PCIE_CORE_CTRL0_REG);
	reg &= ~PCIE_GEN_SEL_MSK;
	if (pcie->link_gen == 3)
		reg |= SPEED_GEN_3;
	else if (pcie->link_gen == 2)
		reg |= SPEED_GEN_2;
	else
		reg |= SPEED_GEN_1;
	advk_writel(pcie, reg, PCIE_CORE_CTRL0_REG);

	/*
	 * Set maximal link speed value also into PCIe Link Control 2 register.
	 * Armada 3700 Functional Specification says that default value is based
	 * on SPEED_GEN but tests showed that default value is always 8.0 GT/s.
	 */
	reg = advk_readl(pcie, PCIE_CORE_PCIEXP_CAP + PCI_EXP_LNKCTL2);
	reg &= ~PCI_EXP_LNKCTL2_TLS;
	if (pcie->link_gen == 3)
		reg |= PCI_EXP_LNKCTL2_TLS_8_0GT;
	else if (pcie->link_gen == 2)
		reg |= PCI_EXP_LNKCTL2_TLS_5_0GT;
	else
		reg |= PCI_EXP_LNKCTL2_TLS_2_5GT;
	advk_writel(pcie, reg, PCIE_CORE_PCIEXP_CAP + PCI_EXP_LNKCTL2);

	/* Enable link training after selecting PCIe generation */
	reg = advk_readl(pcie, PCIE_CORE_CTRL0_REG);
	reg |= LINK_TRAINING_EN;
	advk_writel(pcie, reg, PCIE_CORE_CTRL0_REG);

	/*
	 * Reset PCIe card via PERST# signal. Some cards are not detected
	 * during link training when they are in some non-initial state.
	 */
	advk_pcie_issue_perst(pcie);

	/*
	 * PERST# signal could have been asserted by pinctrl subsystem before
	 * probe() callback has been called or issued explicitly by reset gpio
	 * function advk_pcie_issue_perst(), making the endpoint going into
	 * fundamental reset. As required by PCI Express spec (PCI Express
	 * Base Specification, REV. 4.0 PCI Express, February 19 2014, 6.6.1
	 * Conventional Reset) a delay for at least 100ms after such a reset
	 * before sending a Configuration Request to the device is needed.
	 * So wait until PCIe link is up. Function advk_pcie_wait_for_link()
	 * waits for link at least 900ms.
	 */
	ret = advk_pcie_wait_for_link(pcie);
	if (ret < 0)
		dev_err(dev, "link never came up\n");
	else
		dev_info(dev, "link up\n");
}

/*
 * Set PCIe address window register which could be used for memory
 * mapping.
 */
static void advk_pcie_set_ob_win(struct advk_pcie *pcie, u8 win_num,
				 phys_addr_t match, phys_addr_t remap,
				 phys_addr_t mask, u32 actions)
{
	advk_writel(pcie, OB_WIN_ENABLE |
			  lower_32_bits(match), OB_WIN_MATCH_LS(win_num));
	advk_writel(pcie, upper_32_bits(match), OB_WIN_MATCH_MS(win_num));
	advk_writel(pcie, lower_32_bits(remap), OB_WIN_REMAP_LS(win_num));
	advk_writel(pcie, upper_32_bits(remap), OB_WIN_REMAP_MS(win_num));
	advk_writel(pcie, lower_32_bits(mask), OB_WIN_MASK_LS(win_num));
	advk_writel(pcie, upper_32_bits(mask), OB_WIN_MASK_MS(win_num));
	advk_writel(pcie, actions, OB_WIN_ACTIONS(win_num));
}

static void advk_pcie_disable_ob_win(struct advk_pcie *pcie, u8 win_num)
{
	advk_writel(pcie, 0, OB_WIN_MATCH_LS(win_num));
	advk_writel(pcie, 0, OB_WIN_MATCH_MS(win_num));
	advk_writel(pcie, 0, OB_WIN_REMAP_LS(win_num));
	advk_writel(pcie, 0, OB_WIN_REMAP_MS(win_num));
	advk_writel(pcie, 0, OB_WIN_MASK_LS(win_num));
	advk_writel(pcie, 0, OB_WIN_MASK_MS(win_num));
	advk_writel(pcie, 0, OB_WIN_ACTIONS(win_num));
}

static void advk_pcie_setup_hw(struct advk_pcie *pcie)
{
	u32 reg;
	int i;

	/* Set to Direct mode */
	reg = advk_readl(pcie, CTRL_CONFIG_REG);
	reg &= ~(CTRL_MODE_MASK << CTRL_MODE_SHIFT);
	reg |= ((PCIE_CORE_MODE_DIRECT & CTRL_MODE_MASK) << CTRL_MODE_SHIFT);
	advk_writel(pcie, reg, CTRL_CONFIG_REG);

	/* Set PCI global control register to RC mode */
	reg = advk_readl(pcie, PCIE_CORE_CTRL0_REG);
	reg |= (IS_RC_MSK << IS_RC_SHIFT);
	advk_writel(pcie, reg, PCIE_CORE_CTRL0_REG);

	/* Set Advanced Error Capabilities and Control PF0 register */
	reg = PCIE_CORE_ERR_CAPCTL_ECRC_CHK_TX |
		PCIE_CORE_ERR_CAPCTL_ECRC_CHK_TX_EN |
		PCIE_CORE_ERR_CAPCTL_ECRC_CHCK |
		PCIE_CORE_ERR_CAPCTL_ECRC_CHCK_RCV;
	advk_writel(pcie, reg, PCIE_CORE_ERR_CAPCTL_REG);

	/* Set PCIe Device Control register */
	reg = advk_readl(pcie, PCIE_CORE_PCIEXP_CAP + PCI_EXP_DEVCTL);
	reg &= ~PCI_EXP_DEVCTL_RELAX_EN;
	reg &= ~PCI_EXP_DEVCTL_NOSNOOP_EN;
	reg &= ~PCI_EXP_DEVCTL_PAYLOAD;
	reg &= ~PCI_EXP_DEVCTL_READRQ;
	reg |= PCI_EXP_DEVCTL_PAYLOAD_512B;
	reg |= PCI_EXP_DEVCTL_READRQ_512B;
	advk_writel(pcie, reg, PCIE_CORE_PCIEXP_CAP + PCI_EXP_DEVCTL);

	/* Program PCIe Control 2 to disable strict ordering */
	reg = PCIE_CORE_CTRL2_RESERVED |
		PCIE_CORE_CTRL2_TD_ENABLE;
	advk_writel(pcie, reg, PCIE_CORE_CTRL2_REG);

	/* Set lane X1 */
	reg = advk_readl(pcie, PCIE_CORE_CTRL0_REG);
	reg &= ~LANE_CNT_MSK;
	reg |= LANE_COUNT_1;
	advk_writel(pcie, reg, PCIE_CORE_CTRL0_REG);

	/* Enable MSI */
	reg = advk_readl(pcie, PCIE_CORE_CTRL2_REG);
	reg |= PCIE_CORE_CTRL2_MSI_ENABLE;
	advk_writel(pcie, reg, PCIE_CORE_CTRL2_REG);

	/* Clear all interrupts */
	advk_writel(pcie, PCIE_ISR0_ALL_MASK, PCIE_ISR0_REG);
	advk_writel(pcie, PCIE_ISR1_ALL_MASK, PCIE_ISR1_REG);
	advk_writel(pcie, PCIE_IRQ_ALL_MASK, HOST_CTRL_INT_STATUS_REG);

	/* Disable All ISR0/1 Sources */
	reg = PCIE_ISR0_ALL_MASK;
	reg &= ~PCIE_ISR0_MSI_INT_PENDING;
	advk_writel(pcie, reg, PCIE_ISR0_MASK_REG);

	advk_writel(pcie, PCIE_ISR1_ALL_MASK, PCIE_ISR1_MASK_REG);

	/* Unmask all MSI's */
	advk_writel(pcie, 0, PCIE_MSI_MASK_REG);

	/* Enable summary interrupt for GIC SPI source */
	reg = PCIE_IRQ_ALL_MASK & (~PCIE_IRQ_ENABLE_INTS_MASK);
	advk_writel(pcie, reg, HOST_CTRL_INT_MASK_REG);

	/*
	 * Enable AXI address window location generation:
	 * When it is enabled, the default outbound window
	 * configurations (Default User Field: 0xD0074CFC)
	 * are used to transparent address translation for
	 * the outbound transactions. Thus, PCIe address
	 * windows are not required for transparent memory
	 * access when default outbound window configuration
	 * is set for memory access.
	 */
	reg = advk_readl(pcie, PCIE_CORE_CTRL2_REG);
	reg |= PCIE_CORE_CTRL2_OB_WIN_ENABLE;
	advk_writel(pcie, reg, PCIE_CORE_CTRL2_REG);

	/*
	 * Set memory access in Default User Field so it
	 * is not required to configure PCIe address for
	 * transparent memory access.
	 */
	advk_writel(pcie, OB_WIN_TYPE_MEM, OB_WIN_DEFAULT_ACTIONS);

	/*
	 * Bypass the address window mapping for PIO:
	 * Since PIO access already contains all required
	 * info over AXI interface by PIO registers, the
	 * address window is not required.
	 */
	reg = advk_readl(pcie, PIO_CTRL);
	reg |= PIO_CTRL_ADDR_WIN_DISABLE;
	advk_writel(pcie, reg, PIO_CTRL);

	/*
	 * Configure PCIe address windows for non-memory or
	 * non-transparent access as by default PCIe uses
	 * transparent memory access.
	 */
	for (i = 0; i < pcie->wins_count; i++)
		advk_pcie_set_ob_win(pcie, i,
				     pcie->wins[i].match, pcie->wins[i].remap,
				     pcie->wins[i].mask, pcie->wins[i].actions);

	/* Disable remaining PCIe outbound windows */
	for (i = pcie->wins_count; i < OB_WIN_COUNT; i++)
		advk_pcie_disable_ob_win(pcie, i);

	advk_pcie_train_link(pcie);

	reg = advk_readl(pcie, PCIE_CORE_CMD_STATUS_REG);
	reg |= PCIE_CORE_CMD_MEM_ACCESS_EN |
		PCIE_CORE_CMD_IO_ACCESS_EN |
		PCIE_CORE_CMD_MEM_IO_REQ_EN;
	advk_writel(pcie, reg, PCIE_CORE_CMD_STATUS_REG);
}

static int advk_pcie_check_pio_status(struct advk_pcie *pcie, u32 *val)
{
	struct device *dev = &pcie->pdev->dev;
	u32 reg;
	unsigned int status;
	char *strcomp_status, *str_posted;

	reg = advk_readl(pcie, PIO_STAT);
	status = (reg & PIO_COMPLETION_STATUS_MASK) >>
		PIO_COMPLETION_STATUS_SHIFT;

	/*
	 * According to HW spec, the PIO status check sequence as below:
	 * 1) even if COMPLETION_STATUS(bit9:7) indicates successful,
	 *    it still needs to check Error Status(bit11), only when this bit
	 *    indicates no error happen, the operation is successful.
	 * 2) value Unsupported Request(1) of COMPLETION_STATUS(bit9:7) only
	 *    means a PIO write error, and for PIO read it is successful with
	 *    a read value of 0xFFFFFFFF.
	 * 3) value Completion Retry Status(CRS) of COMPLETION_STATUS(bit9:7)
	 *    only means a PIO write error, and for PIO read it is successful
	 *    with a read value of 0xFFFF0001.
	 * 4) value Completer Abort (CA) of COMPLETION_STATUS(bit9:7) means
	 *    error for both PIO read and PIO write operation.
	 * 5) other errors are indicated as 'unknown'.
	 */
	switch (status) {
	case PIO_COMPLETION_STATUS_OK:
		if (reg & PIO_ERR_STATUS) {
			strcomp_status = "COMP_ERR";
			break;
		}
		/* Get the read result */
		if (val)
			*val = advk_readl(pcie, PIO_RD_DATA);
		/* No error */
		strcomp_status = NULL;
		break;
	case PIO_COMPLETION_STATUS_UR:
		strcomp_status = "UR";
		break;
	case PIO_COMPLETION_STATUS_CRS:
		/* PCIe r4.0, sec 2.3.2, says:
		 * If CRS Software Visibility is not enabled, the Root Complex
		 * must re-issue the Configuration Request as a new Request.
		 * A Root Complex implementation may choose to limit the number
		 * of Configuration Request/CRS Completion Status loops before
		 * determining that something is wrong with the target of the
		 * Request and taking appropriate action, e.g., complete the
		 * Request to the host as a failed transaction.
		 *
		 * To simplify implementation do not re-issue the Configuration
		 * Request and complete the Request as a failed transaction.
		 */
		strcomp_status = "CRS";
		break;
	case PIO_COMPLETION_STATUS_CA:
		strcomp_status = "CA";
		break;
	default:
		strcomp_status = "Unknown";
		break;
	}

	if (!strcomp_status)
		return 0;

	if (reg & PIO_NON_POSTED_REQ)
		str_posted = "Non-posted";
	else
		str_posted = "Posted";

	dev_dbg(dev, "%s PIO Response Status: %s, %#x @ %#x\n",
		str_posted, strcomp_status, reg, advk_readl(pcie, PIO_ADDR_LS));

	return -EFAULT;
}

static int advk_pcie_wait_pio(struct advk_pcie *pcie)
{
	struct device *dev = &pcie->pdev->dev;
	int i;

	for (i = 0; i < PIO_RETRY_CNT; i++) {
		u32 start, isr;

		start = advk_readl(pcie, PIO_START);
		isr = advk_readl(pcie, PIO_ISR);
		if (!start && isr)
			return 0;
		udelay(PIO_RETRY_DELAY);
	}

	dev_err(dev, "PIO read/write transfer time out\n");
	return -ETIMEDOUT;
}

static bool advk_pcie_pio_is_running(struct advk_pcie *pcie)
{
	struct device *dev = &pcie->pdev->dev;

	/*
	 * Trying to start a new PIO transfer when previous has not completed
	 * cause External Abort on CPU which results in kernel panic:
	 *
	 *     SError Interrupt on CPU0, code 0xbf000002 -- SError
	 *     Kernel panic - not syncing: Asynchronous SError Interrupt
	 *
	 * Functions advk_pcie_rd_conf() and advk_pcie_wr_conf() are protected
	 * by raw_spin_lock_irqsave() at pci_lock_config() level to prevent
	 * concurrent calls at the same time. But because PIO transfer may take
	 * about 1.5s when link is down or card is disconnected, it means that
	 * advk_pcie_wait_pio() does not always have to wait for completion.
	 *
	 * Some versions of ARM Trusted Firmware handles this External Abort at
	 * EL3 level and mask it to prevent kernel panic. Relevant TF-A commit:
	 * https://git.trustedfirmware.org/TF-A/trusted-firmware-a.git/commit/?id=3c7dcdac5c50
	 */
	if (advk_readl(pcie, PIO_START)) {
		dev_err(dev, "Previous PIO read/write transfer is still running\n");
		return true;
	}

	return false;
}

static bool advk_pcie_valid_device(struct advk_pcie *pcie, struct pci_bus *bus,
				  int devfn)
{
	if ((bus->number == pcie->root_bus_nr) && PCI_SLOT(devfn) != 0)
		return false;

	/*
	 * If the link goes down after we check for link-up, nothing bad
	 * happens but the config access times out.
	 */
	if (bus->number != pcie->root_bus_nr && !advk_pcie_link_up(pcie))
		return false;

	return true;
}

static int advk_pcie_rd_conf(struct pci_bus *bus, u32 devfn,
			     int where, int size, u32 *val)
{
	struct advk_pcie *pcie = bus->sysdata;
	u32 reg;
	int ret;

	if (!advk_pcie_valid_device(pcie, bus, devfn)) {
		*val = 0xffffffff;
		return PCIBIOS_DEVICE_NOT_FOUND;
	}

	if (advk_pcie_pio_is_running(pcie)) {
		*val = 0xffffffff;
		return PCIBIOS_SET_FAILED;
	}

	/* Program the control register */
	reg = advk_readl(pcie, PIO_CTRL);
	reg &= ~PIO_CTRL_TYPE_MASK;
	if (bus->number ==  pcie->root_bus_nr)
		reg |= PCIE_CONFIG_RD_TYPE0;
	else
		reg |= PCIE_CONFIG_RD_TYPE1;
	advk_writel(pcie, reg, PIO_CTRL);

	/* Program the address registers */
	reg = PCIE_CONF_ADDR(bus->number, devfn, where);
	advk_writel(pcie, reg, PIO_ADDR_LS);
	advk_writel(pcie, 0, PIO_ADDR_MS);

	/* Program the data strobe */
	advk_writel(pcie, 0xf, PIO_WR_DATA_STRB);

	/* Clear PIO DONE ISR and start the transfer */
	advk_writel(pcie, 1, PIO_ISR);
	advk_writel(pcie, 1, PIO_START);

	ret = advk_pcie_wait_pio(pcie);
	if (ret < 0) {
		*val = 0xffffffff;
		return PCIBIOS_SET_FAILED;
	}

	/* Check PIO status and get the read result */
	ret = advk_pcie_check_pio_status(pcie, val);
	if (ret < 0) {
		*val = 0xffffffff;
		return PCIBIOS_SET_FAILED;
	}

	if (size == 1)
		*val = (*val >> (8 * (where & 3))) & 0xff;
	else if (size == 2)
		*val = (*val >> (8 * (where & 3))) & 0xffff;

	return PCIBIOS_SUCCESSFUL;
}

static int advk_pcie_wr_conf(struct pci_bus *bus, u32 devfn,
				int where, int size, u32 val)
{
	struct advk_pcie *pcie = bus->sysdata;
	u32 reg;
	u32 data_strobe = 0x0;
	int offset;
	int ret;

	if (!advk_pcie_valid_device(pcie, bus, devfn))
		return PCIBIOS_DEVICE_NOT_FOUND;

	if (where % size)
		return PCIBIOS_SET_FAILED;

	if (advk_pcie_pio_is_running(pcie))
		return PCIBIOS_SET_FAILED;

	/* Program the control register */
	reg = advk_readl(pcie, PIO_CTRL);
	reg &= ~PIO_CTRL_TYPE_MASK;
	if (bus->number == pcie->root_bus_nr)
		reg |= PCIE_CONFIG_WR_TYPE0;
	else
		reg |= PCIE_CONFIG_WR_TYPE1;
	advk_writel(pcie, reg, PIO_CTRL);

	/* Program the address registers */
	reg = PCIE_CONF_ADDR(bus->number, devfn, where);
	advk_writel(pcie, reg, PIO_ADDR_LS);
	advk_writel(pcie, 0, PIO_ADDR_MS);

	/* Calculate the write strobe */
	offset      = where & 0x3;
	reg         = val << (8 * offset);
	data_strobe = GENMASK(size - 1, 0) << offset;

	/* Program the data register */
	advk_writel(pcie, reg, PIO_WR_DATA);

	/* Program the data strobe */
	advk_writel(pcie, data_strobe, PIO_WR_DATA_STRB);

	/* Clear PIO DONE ISR and start the transfer */
	advk_writel(pcie, 1, PIO_ISR);
	advk_writel(pcie, 1, PIO_START);

	ret = advk_pcie_wait_pio(pcie);
	if (ret < 0)
		return PCIBIOS_SET_FAILED;

	ret = advk_pcie_check_pio_status(pcie, NULL);
	if (ret < 0)
		return PCIBIOS_SET_FAILED;

	return PCIBIOS_SUCCESSFUL;
}

static struct pci_ops advk_pcie_ops = {
	.read = advk_pcie_rd_conf,
	.write = advk_pcie_wr_conf,
};

static void advk_msi_irq_compose_msi_msg(struct irq_data *data,
					 struct msi_msg *msg)
{
	struct advk_pcie *pcie = irq_data_get_irq_chip_data(data);
	phys_addr_t msi_msg = virt_to_phys(&pcie->msi_msg);

	msg->address_lo = lower_32_bits(msi_msg);
	msg->address_hi = upper_32_bits(msi_msg);
	msg->data = data->irq;
}

static int advk_msi_set_affinity(struct irq_data *irq_data,
				 const struct cpumask *mask, bool force)
{
	return -EINVAL;
}

static int advk_msi_irq_domain_alloc(struct irq_domain *domain,
				     unsigned int virq,
				     unsigned int nr_irqs, void *args)
{
	struct advk_pcie *pcie = domain->host_data;
	int hwirq, i;

	mutex_lock(&pcie->msi_used_lock);
	hwirq = bitmap_find_next_zero_area(pcie->msi_used, MSI_IRQ_NUM,
					   0, nr_irqs, 0);
	if (hwirq >= MSI_IRQ_NUM) {
		mutex_unlock(&pcie->msi_used_lock);
		return -ENOSPC;
	}

	bitmap_set(pcie->msi_used, hwirq, nr_irqs);
	mutex_unlock(&pcie->msi_used_lock);

	for (i = 0; i < nr_irqs; i++)
		irq_domain_set_info(domain, virq + i, hwirq + i,
				    &pcie->msi_bottom_irq_chip,
				    domain->host_data, handle_simple_irq,
				    NULL, NULL);

	return 0;
}

static void advk_msi_irq_domain_free(struct irq_domain *domain,
				     unsigned int virq, unsigned int nr_irqs)
{
	struct irq_data *d = irq_domain_get_irq_data(domain, virq);
	struct advk_pcie *pcie = domain->host_data;

	mutex_lock(&pcie->msi_used_lock);
	bitmap_clear(pcie->msi_used, d->hwirq, nr_irqs);
	mutex_unlock(&pcie->msi_used_lock);
}

static const struct irq_domain_ops advk_msi_domain_ops = {
	.alloc = advk_msi_irq_domain_alloc,
	.free = advk_msi_irq_domain_free,
};

static void advk_pcie_irq_mask(struct irq_data *d)
{
	struct advk_pcie *pcie = d->domain->host_data;
	irq_hw_number_t hwirq = irqd_to_hwirq(d);
	unsigned long flags;
	u32 mask;

	raw_spin_lock_irqsave(&pcie->irq_lock, flags);
	mask = advk_readl(pcie, PCIE_ISR1_MASK_REG);
	mask |= PCIE_ISR1_INTX_ASSERT(hwirq);
	advk_writel(pcie, mask, PCIE_ISR1_MASK_REG);
	raw_spin_unlock_irqrestore(&pcie->irq_lock, flags);
}

static void advk_pcie_irq_unmask(struct irq_data *d)
{
	struct advk_pcie *pcie = d->domain->host_data;
	irq_hw_number_t hwirq = irqd_to_hwirq(d);
	unsigned long flags;
	u32 mask;

	raw_spin_lock_irqsave(&pcie->irq_lock, flags);
	mask = advk_readl(pcie, PCIE_ISR1_MASK_REG);
	mask &= ~PCIE_ISR1_INTX_ASSERT(hwirq);
	advk_writel(pcie, mask, PCIE_ISR1_MASK_REG);
	raw_spin_unlock_irqrestore(&pcie->irq_lock, flags);
}

static int advk_pcie_irq_map(struct irq_domain *h,
			     unsigned int virq, irq_hw_number_t hwirq)
{
	struct advk_pcie *pcie = h->host_data;

	advk_pcie_irq_mask(irq_get_irq_data(virq));
	irq_set_status_flags(virq, IRQ_LEVEL);
	irq_set_chip_and_handler(virq, &pcie->irq_chip,
				 handle_level_irq);
	irq_set_chip_data(virq, pcie);

	return 0;
}

static const struct irq_domain_ops advk_pcie_irq_domain_ops = {
	.map = advk_pcie_irq_map,
	.xlate = irq_domain_xlate_onecell,
};

static int advk_pcie_init_msi_irq_domain(struct advk_pcie *pcie)
{
	struct device *dev = &pcie->pdev->dev;
	struct device_node *node = dev->of_node;
	struct irq_chip *bottom_ic, *msi_ic;
	struct msi_domain_info *msi_di;
	phys_addr_t msi_msg_phys;

	mutex_init(&pcie->msi_used_lock);

	bottom_ic = &pcie->msi_bottom_irq_chip;

	bottom_ic->name = "MSI";
	bottom_ic->irq_compose_msi_msg = advk_msi_irq_compose_msi_msg;
	bottom_ic->irq_set_affinity = advk_msi_set_affinity;

	msi_ic = &pcie->msi_irq_chip;
	msi_ic->name = "advk-MSI";

	msi_di = &pcie->msi_domain_info;
	msi_di->flags = MSI_FLAG_USE_DEF_DOM_OPS | MSI_FLAG_USE_DEF_CHIP_OPS |
		MSI_FLAG_MULTI_PCI_MSI;
	msi_di->chip = msi_ic;

	msi_msg_phys = virt_to_phys(&pcie->msi_msg);

	advk_writel(pcie, lower_32_bits(msi_msg_phys),
		    PCIE_MSI_ADDR_LOW_REG);
	advk_writel(pcie, upper_32_bits(msi_msg_phys),
		    PCIE_MSI_ADDR_HIGH_REG);

	pcie->msi_inner_domain =
		irq_domain_add_linear(NULL, MSI_IRQ_NUM,
				      &advk_msi_domain_ops, pcie);
	if (!pcie->msi_inner_domain)
		return -ENOMEM;

	pcie->msi_domain =
		pci_msi_create_irq_domain(of_node_to_fwnode(node),
					  msi_di, pcie->msi_inner_domain);
	if (!pcie->msi_domain) {
		irq_domain_remove(pcie->msi_inner_domain);
		return -ENOMEM;
	}

	return 0;
}

static void advk_pcie_remove_msi_irq_domain(struct advk_pcie *pcie)
{
	irq_domain_remove(pcie->msi_domain);
	irq_domain_remove(pcie->msi_inner_domain);
}

static int advk_pcie_init_irq_domain(struct advk_pcie *pcie)
{
	struct device *dev = &pcie->pdev->dev;
	struct device_node *node = dev->of_node;
	struct device_node *pcie_intc_node;
	struct irq_chip *irq_chip;
	int ret = 0;

	raw_spin_lock_init(&pcie->irq_lock);

	pcie_intc_node =  of_get_next_child(node, NULL);
	if (!pcie_intc_node) {
		dev_err(dev, "No PCIe Intc node found\n");
		return -ENODEV;
	}

	irq_chip = &pcie->irq_chip;

	irq_chip->name = devm_kasprintf(dev, GFP_KERNEL, "%s-irq",
					dev_name(dev));
	if (!irq_chip->name) {
		ret = -ENOMEM;
		goto out_put_node;
	}

	irq_chip->irq_mask = advk_pcie_irq_mask;
	irq_chip->irq_mask_ack = advk_pcie_irq_mask;
	irq_chip->irq_unmask = advk_pcie_irq_unmask;

	pcie->irq_domain =
		irq_domain_add_linear(pcie_intc_node, PCI_NUM_INTX,
				      &advk_pcie_irq_domain_ops, pcie);
	if (!pcie->irq_domain) {
		dev_err(dev, "Failed to get a INTx IRQ domain\n");
		ret = -ENOMEM;
		goto out_put_node;
	}

out_put_node:
	of_node_put(pcie_intc_node);
	return ret;
}

static void advk_pcie_remove_irq_domain(struct advk_pcie *pcie)
{
	irq_domain_remove(pcie->irq_domain);
}

static void advk_pcie_handle_msi(struct advk_pcie *pcie)
{
	u32 msi_val, msi_mask, msi_status, msi_idx;
	u16 msi_data;

	msi_mask = advk_readl(pcie, PCIE_MSI_MASK_REG);
	msi_val = advk_readl(pcie, PCIE_MSI_STATUS_REG);
	msi_status = msi_val & ~msi_mask;

	for (msi_idx = 0; msi_idx < MSI_IRQ_NUM; msi_idx++) {
		if (!(BIT(msi_idx) & msi_status))
			continue;

		/*
		 * msi_idx contains bits [4:0] of the msi_data and msi_data
		 * contains 16bit MSI interrupt number
		 */
		advk_writel(pcie, BIT(msi_idx), PCIE_MSI_STATUS_REG);
		msi_data = advk_readl(pcie, PCIE_MSI_PAYLOAD_REG) & PCIE_MSI_DATA_MASK;
		generic_handle_irq(msi_data);
	}

	advk_writel(pcie, PCIE_ISR0_MSI_INT_PENDING,
		    PCIE_ISR0_REG);
}

static void advk_pcie_handle_int(struct advk_pcie *pcie)
{
	u32 isr0_val, isr0_mask, isr0_status;
	u32 isr1_val, isr1_mask, isr1_status;
	int i, virq;

	isr0_val = advk_readl(pcie, PCIE_ISR0_REG);
	isr0_mask = advk_readl(pcie, PCIE_ISR0_MASK_REG);
	isr0_status = isr0_val & ((~isr0_mask) & PCIE_ISR0_ALL_MASK);

	isr1_val = advk_readl(pcie, PCIE_ISR1_REG);
	isr1_mask = advk_readl(pcie, PCIE_ISR1_MASK_REG);
	isr1_status = isr1_val & ((~isr1_mask) & PCIE_ISR1_ALL_MASK);

	/* Process MSI interrupts */
	if (isr0_status & PCIE_ISR0_MSI_INT_PENDING)
		advk_pcie_handle_msi(pcie);

	/* Process legacy interrupts */
	for (i = 0; i < PCI_NUM_INTX; i++) {
		if (!(isr1_status & PCIE_ISR1_INTX_ASSERT(i)))
			continue;

		advk_writel(pcie, PCIE_ISR1_INTX_ASSERT(i),
			    PCIE_ISR1_REG);

		virq = irq_find_mapping(pcie->irq_domain, i);
		generic_handle_irq(virq);
	}
}

static irqreturn_t advk_pcie_irq_handler(int irq, void *arg)
{
	struct advk_pcie *pcie = arg;
	u32 status;

	status = advk_readl(pcie, HOST_CTRL_INT_STATUS_REG);
	if (!(status & PCIE_IRQ_CORE_INT))
		return IRQ_NONE;

	advk_pcie_handle_int(pcie);

	/* Clear interrupt */
	advk_writel(pcie, PCIE_IRQ_CORE_INT, HOST_CTRL_INT_STATUS_REG);

	return IRQ_HANDLED;
}

static int advk_pcie_parse_request_of_pci_ranges(struct advk_pcie *pcie)
{
	int err, res_valid = 0;
	struct device *dev = &pcie->pdev->dev;
	struct device_node *np = dev->of_node;
	struct resource_entry *win, *tmp;
	resource_size_t iobase;

	INIT_LIST_HEAD(&pcie->resources);

	err = of_pci_get_host_bridge_resources(np, 0, 0xff, &pcie->resources,
					       &iobase);
	if (err)
		return err;

	err = devm_request_pci_bus_resources(dev, &pcie->resources);
	if (err)
		goto out_release_res;

	resource_list_for_each_entry_safe(win, tmp, &pcie->resources) {
		struct resource *res = win->res;

		switch (resource_type(res)) {
		case IORESOURCE_IO:
			err = devm_pci_remap_iospace(dev, res, iobase);
			if (err) {
				dev_warn(dev, "error %d: failed to map resource %pR\n",
					 err, res);
				resource_list_destroy_entry(win);
			}
			break;
		case IORESOURCE_MEM:
			res_valid |= !(res->flags & IORESOURCE_PREFETCH);
			break;
		case IORESOURCE_BUS:
			pcie->root_bus_nr = res->start;
			break;
		}
	}

	if (!res_valid) {
		dev_err(dev, "non-prefetchable memory resource required\n");
		err = -EINVAL;
		goto out_release_res;
	}

	return 0;

out_release_res:
	pci_free_resource_list(&pcie->resources);
	return err;
}

static int advk_pcie_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct advk_pcie *pcie;
	struct resource *res;
	struct pci_bus *bus, *child;
	struct pci_host_bridge *bridge;
	struct resource_entry *entry;
	int ret, irq;

	bridge = devm_pci_alloc_host_bridge(dev, sizeof(struct advk_pcie));
	if (!bridge)
		return -ENOMEM;

	pcie = pci_host_bridge_priv(bridge);
	pcie->pdev = pdev;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	pcie->base = devm_ioremap_resource(dev, res);
	if (IS_ERR(pcie->base))
		return PTR_ERR(pcie->base);

	irq = platform_get_irq(pdev, 0);
	ret = devm_request_irq(dev, irq, advk_pcie_irq_handler,
			       IRQF_SHARED | IRQF_NO_THREAD, "advk-pcie",
			       pcie);
	if (ret) {
		dev_err(dev, "Failed to register interrupt\n");
		return ret;
	}

	ret = advk_pcie_parse_request_of_pci_ranges(pcie);
	if (ret) {
		dev_err(dev, "Failed to parse resources\n");
		return ret;
	}

	resource_list_for_each_entry(entry, &pcie->resources) {
		resource_size_t start = entry->res->start;
		resource_size_t size = resource_size(entry->res);
		unsigned long type = resource_type(entry->res);
		u64 win_size;

		/*
		 * Aardvark hardware allows to configure also PCIe window
		 * for config type 0 and type 1 mapping, but driver uses
		 * only PIO for issuing configuration transfers which does
		 * not use PCIe window configuration.
		 */
		if (type != IORESOURCE_MEM && type != IORESOURCE_MEM_64 &&
		    type != IORESOURCE_IO)
			continue;

		/*
		 * Skip transparent memory resources. Default outbound access
		 * configuration is set to transparent memory access so it
		 * does not need window configuration.
		 */
		if ((type == IORESOURCE_MEM || type == IORESOURCE_MEM_64) &&
		    entry->offset == 0)
			continue;

		/*
		 * The n-th PCIe window is configured by tuple (match, remap, mask)
		 * and an access to address A uses this window if A matches the
		 * match with given mask.
		 * So every PCIe window size must be a power of two and every start
		 * address must be aligned to window size. Minimal size is 64 KiB
		 * because lower 16 bits of mask must be zero. Remapped address
		 * may have set only bits from the mask.
		 */
		while (pcie->wins_count < OB_WIN_COUNT && size > 0) {
			/* Calculate the largest aligned window size */
			win_size = (1ULL << (fls64(size)-1)) |
				   (start ? (1ULL << __ffs64(start)) : 0);
			win_size = 1ULL << __ffs64(win_size);
			if (win_size < 0x10000)
				break;

			dev_dbg(dev,
				"Configuring PCIe window %d: [0x%llx-0x%llx] as %lu\n",
				pcie->wins_count, (unsigned long long)start,
				(unsigned long long)start + win_size, type);

			if (type == IORESOURCE_IO) {
				pcie->wins[pcie->wins_count].actions = OB_WIN_TYPE_IO;
				pcie->wins[pcie->wins_count].match = pci_pio_to_address(start);
			} else {
				pcie->wins[pcie->wins_count].actions = OB_WIN_TYPE_MEM;
				pcie->wins[pcie->wins_count].match = start;
			}
			pcie->wins[pcie->wins_count].remap = start - entry->offset;
			pcie->wins[pcie->wins_count].mask = ~(win_size - 1);

			if (pcie->wins[pcie->wins_count].remap & (win_size - 1))
				break;

			start += win_size;
			size -= win_size;
			pcie->wins_count++;
		}

		if (size > 0) {
			dev_err(&pcie->pdev->dev,
				"Invalid PCIe region [0x%llx-0x%llx]\n",
				(unsigned long long)entry->res->start,
				(unsigned long long)entry->res->end + 1);
			return -EINVAL;
		}
	}

	pcie->reset_gpio = devm_fwnode_get_index_gpiod_from_child(dev, "reset",
								  0,
								  dev_fwnode(dev),
								  GPIOD_OUT_LOW,
								  "pcie1-reset");
	ret = PTR_ERR_OR_ZERO(pcie->reset_gpio);
	if (ret) {
		if (ret == -ENOENT) {
			pcie->reset_gpio = NULL;
		} else {
			if (ret != -EPROBE_DEFER)
				dev_err(dev, "Failed to get reset-gpio: %i\n",
					ret);
			return ret;
		}
	}

	ret = of_pci_get_max_link_speed(dev->of_node);
	if (ret <= 0 || ret > 3)
		pcie->link_gen = 3;
	else
		pcie->link_gen = ret;

	advk_pcie_setup_hw(pcie);

	ret = advk_pcie_init_irq_domain(pcie);
	if (ret) {
		dev_err(dev, "Failed to initialize irq\n");
		return ret;
	}

	ret = advk_pcie_init_msi_irq_domain(pcie);
	if (ret) {
		dev_err(dev, "Failed to initialize irq\n");
		advk_pcie_remove_irq_domain(pcie);
		return ret;
	}

	list_splice_init(&pcie->resources, &bridge->windows);
	bridge->dev.parent = dev;
	bridge->sysdata = pcie;
	bridge->busnr = 0;
	bridge->ops = &advk_pcie_ops;
	bridge->map_irq = of_irq_parse_and_map_pci;
	bridge->swizzle_irq = pci_common_swizzle;

	ret = pci_scan_root_bus_bridge(bridge);
	if (ret < 0) {
		advk_pcie_remove_msi_irq_domain(pcie);
		advk_pcie_remove_irq_domain(pcie);
		return ret;
	}

	bus = bridge->bus;

	pci_bus_size_bridges(bus);
	pci_bus_assign_resources(bus);

	list_for_each_entry(child, &bus->children, node)
		pcie_bus_configure_settings(child);

	pci_bus_add_devices(bus);
	return 0;
}

static const struct of_device_id advk_pcie_of_match_table[] = {
	{ .compatible = "marvell,armada-3700-pcie", },
	{},
};

static struct platform_driver advk_pcie_driver = {
	.driver = {
		.name = "advk-pcie",
		.of_match_table = advk_pcie_of_match_table,
		/* Driver unloading/unbinding currently not supported */
		.suppress_bind_attrs = true,
	},
	.probe = advk_pcie_probe,
};
builtin_platform_driver(advk_pcie_driver);
