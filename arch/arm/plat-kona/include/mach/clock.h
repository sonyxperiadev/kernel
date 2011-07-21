/*****************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/
#ifndef __ARM_ARCH_KONA_CLOCK_H
#define __ARM_ARCH_KONA_CLOCK_H

#include <linux/errno.h>
#include <linux/spinlock.h>
#include <linux/init.h>
#include <asm/clkdev.h>

/* CCU IDs */
enum {
    BCM2165x_ROOT_CCU = 1,
    BCM2165x_HUB_CCU,
    BCM2165x_AON_CCU,
    BCM2165x_MM_CCU,
    BCM2165x_KONA_MST_CCU,
    BCM2165x_PROC_CCU,
    BCM2165x_KONA_SLV_CCU,
};

/* Clock flags */
enum {
    TYPE_CCU 			= (1 << 0),
    TYPE_BUS_CLK 		= (1 << 1),
    TYPE_PERI_CLK 		= (1 << 2),
    HYST_ENABLE 		= (1 << 3),
    HYST_VALUE_LOW 		= (1 << 4),
    HYST_VALUE_HIGH 		= (1 << 5),
    VOLT_LEVEL_LOW 		= (1 << 6),
    VOLT_LEVEL_HIGH 		= (1 << 7),
    AUTO_GATE 			= (1 << 8),
    SW_GATE 			= (1 << 9),
    ENABLE_ON_INIT		= (1 << 10),
    DEFAULT_ACTIVE_CLK		= (1 << 11),
    DISABLE_ON_INIT		= (1 << 12),
};

struct clk;
int clk_register(struct clk_lookup *clk_lkup);

/**
 * struct clk_ops - standard clock operations
 * @enable: enable/disable clock, see clk_enable() and clk_disable()
 * @set_rate: set the clock rate, see clk_set_rate().
 * @get_rate: get the clock rate, see clk_get_rate().
 * @round_rate: round a given clock rate, see clk_round_rate().
 * @set_parent: set the clock's parent, see clk_set_parent().
 *
 * Group the common clock implementations together so that we
 * don't have to keep setting the same fiels again. We leave
 * enable in struct clk.
 *
 */
struct clk_ops {
	int		(*init)(struct clk *c);
	int		(*enable)(struct clk *c, int enable);
	int		(*set_rate)(struct clk *c, unsigned long rate);
	unsigned long	(*get_rate)(struct clk *c);
	unsigned long	(*round_rate)(struct clk *c, unsigned long rate);
	int		(*set_parent)(struct clk *c, struct clk *parent);
};

/**
 * struct clk_src - clock source
 * @total: number of clock sources, 0 means root clock, no parent
 * @parents: array of parents - source
 */
struct clk_src {
	int		total;
	int		sel;
	struct clk 	**parents;
};

struct clk {
	struct list_head	list;
	struct module	*owner;
	struct clk	*parent;
	const char	*name;
	int		id;
	int		use_cnt;
	int             ccu_id;
	unsigned int	flags;

	unsigned long	rate;		/* in HZ */
	unsigned long	div;		/* programmable divider. 0 means fixed ratio to parent clock */
	unsigned long	pre_div;	/* programmable divider. 0 means fixed ratio to parent clock */
	unsigned long	fraction;

	struct clk_src	*src;
	struct clk_ops	*ops;
};

struct proc_clock {
	struct clk	clk;
	unsigned long	proc_clk_mgr_base;
};

struct peri_clock {
	struct clk	clk;

	unsigned long	ccu_clk_mgr_base;
	unsigned long	wr_access_offset;
	unsigned long	clkgate_offset;
	unsigned long	div_offset;
	unsigned long	div_trig_offset;

	unsigned long	stprsts_mask;
	unsigned long	hw_sw_gating_mask;
	unsigned long	clk_en_mask;
	unsigned long	div_mask;
	int		div_shift;
	unsigned long	div_max;
	unsigned long	pre_div_mask;
	int		pre_div_shift;
	unsigned long	pre_div_max;
	int		div_dithering;		/* dithering franctional bit(s) */
	unsigned long	pll_select_mask;
	int		pll_select_shift;
	unsigned long	trigger_mask;
	unsigned long	pre_trigger_mask;
	unsigned long	policy_bit_shift;
};

struct ccu_clock {
	struct clk	clk;

	unsigned long	ccu_clk_mgr_base;
	unsigned long	wr_access_offset;
	unsigned long	policy_freq_offset;
	int		freq_bit_shift;		/* 8 for most CCU. MM in Rhea is special with 3 */
	unsigned long	policy_ctl_offset;
	unsigned long	policy0_mask_offset;
	unsigned long	policy1_mask_offset;
	unsigned long	policy2_mask_offset;
	unsigned long	policy3_mask_offset;
	unsigned long	policy0_mask1_offset;
	unsigned long	policy1_mask1_offset;
	unsigned long	policy2_mask1_offset;
	unsigned long	policy3_mask1_offset;

	unsigned long	lvm_en_offset;

	int		freq_id;
	unsigned long	freq_tbl[8];
};

struct bus_clock {
	struct clk	clk;

	unsigned long	ccu_clk_mgr_base;
	unsigned long	wr_access_offset;
	unsigned long	clkgate_offset;

	unsigned long	stprsts_mask;
	unsigned long	hw_sw_gating_mask;
	unsigned long	clk_en_mask;
	unsigned long	policy_bit_shift;

	unsigned long	freq_tbl[8];
};

struct ref_clock {
	struct clk	clk;
};

struct clk_rate_div {
    unsigned long div;
    unsigned long pre_div;
    unsigned long fraction;
    unsigned long sel;
};

static inline int is_same_clock(struct clk *a, struct clk *b)
{
	return (a==b);
}

#define	to_clk(p) (&((p)->clk))
#define	name_to_clk(name) (&((name##_clk).clk))
/* declare a struct clk_lookup */
#define	CLK_LK(name) {.con_id=__stringify(name##_clk), .clk=name_to_clk(name),}

static inline struct proc_clock *to_proc_clk(struct clk *clock)
{
	return container_of(clock, struct proc_clock, clk);
}

static inline struct peri_clock *to_peri_clk(struct clk *clock)
{
	return container_of(clock, struct peri_clock, clk);
}

static inline struct ccu_clock *to_ccu_clk(struct clk *clock)
{
	return container_of(clock, struct ccu_clock, clk);
}

static inline struct bus_clock *to_bus_clk(struct clk *clock)
{
	return container_of(clock, struct bus_clock, clk);
}

static inline struct ref_clock *to_ref_clk(struct clk *clock)
{
	return container_of(clock, struct ref_clock, clk);
}

extern struct clk_ops proc_clk_ops;
extern struct clk_ops peri_clk_ops;
extern struct clk_ops ccu_clk_ops;
extern struct clk_ops bus_clk_ops;
extern struct clk_ops ref_clk_ops;

extern int clk_debug;

#ifdef CONFIG_DEBUG_FS
int clock_debug_init(void);
int clock_debug_add_clock(struct clk *c);
#else
#define	clock_debug_init() do {} while(0)
#define	clock_debug_add_clock(clk) do {} while(0)
#endif

int __init clock_init(void);
int __init clock_late_init(void);

unsigned long clock_get_xtal(void);

#define	CLK_WR_ACCESS_PASSWORD	0x00A5A501
#define	CLOCK_1K		1000
#define	CLOCK_1M		(CLOCK_1K * 1000)
#define	CLOCK_UNUSED		0

#if defined(DEBUG)
#define	clk_dbg printk
#else
#define	clk_dbg(format...)		\
	do {				\
		if (clk_debug) 		\
			printk(format);	\
	} while(0)
#endif

/******************************* CCU flags *****************************/
#define		BCM2165x_KONA_MST_CCU_FLAGS			(TYPE_CCU)
#define		BCM2165x_KONA_SLV_CCU_FLAGS			(TYPE_CCU)
#define		BCM2165x_MM_CCU_FLAGS				(TYPE_CCU)
#define		BCM2165x_ROOT_CCU_FLAGS				(TYPE_CCU)
#define		BCM2165x_HUB_CCU_FLAGS				(TYPE_CCU)
#define		BCM2165x_AON_CCU_FLAGS				(TYPE_CCU)
#define		BCM2165x_PROC_CCU_FLAGS				(TYPE_CCU)
/**************************** BUS clock flags **************************/
/* Under KPM CCU */
#define		BCM2165x_CLK_USB_OTG_AHB_FLAGS			(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_USBH_AHB_FLAGS			(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_USBH_12M_FLAGS			(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_SDIO4_AHB_FLAGS			(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_SDIO3_AHB_FLAGS			(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_SDIO2_AHB_FLAGS			(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_SDIO1_AHB_FLAGS			(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_SDIO4_SLEEP_FLAGS			(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_SDIO3_SLEEP_FLAGS			(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_SDIO2_SLEEP_FLAGS			(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_SDIO1_SLEEP_FLAGS			(TYPE_BUS_CLK | SW_GATE)
/* Under KPS CCU */
#define		BCM2165x_CLK_HSM_AHB_FLAGS			(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_HSM_APB_FLAGS			(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_SPUM_OPEN_APB_FLAGS		(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_SPUM_SEC_APB_FLAGS			(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_APB1_FLAGS				(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_TIMERS_APB_FLAGS			(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_SSP0_APB_FLAGS			(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_DMAC_MUX_APB_FLAGS			(TYPE_BUS_CLK | SW_GATE)
#define         BCM2165x_CLK_UARTB4_APB_FLAGS                   (TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_UARTB3_APB_FLAGS			(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_UARTB2_APB_FLAGS			(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_UARTB_APB_FLAGS			(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_PWM_APB_FLAGS			(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_PWM_FLAGS				(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_BSC1_APB_FLAGS			(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_BSC2_APB_FLAGS			(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_BBL_REG_APB_FLAGS			(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_APB2_REG_FLAGS			(TYPE_BUS_CLK | SW_GATE)
/* Under MM CCU */
#define		BCM2165x_CLK_SPI_APB_FLAGS			(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_CSI0_AXI_FLAGS			(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_CSI1_AXI_FLAGS			(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_DSI0_AXI_FLAGS			(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_DSI1_AXI_FLAGS			(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_SMI_AXI_FLAGS			(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_V3D_AXI_FLAGS			(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_VCE_AXI_FLAGS			(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_ISP_AXI_FLAGS			(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_MM_DMA_AXI_FLAGS			(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_MM_APB_FLAGS			(TYPE_BUS_CLK | SW_GATE)
/* Under KHUBAON CCU */
#define		BCM2165x_CLK_PWRMGR_AXI_FLAGS			(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_GPIOKP_APB_FLAGS			(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_HUB_TIMER_APB_FLAGS		(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_PMU_BSC_APB_FLAGS			(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_CHIPREG_APB_FLAGS			(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_FMON_APB_FLAGS			(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_HUB_TZCFG_APB_FLAGS		(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_SEC_WD_APB_FLAGS			(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_SYSEMI_SEC_APB_FLAGS		(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_SYSEMI_OPEN_APB_FLAGS		(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_VCEMI_SEC_APB_FLAGS		(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_VCEMI_OPEN_APB_FLAGS		(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_ACI_APB_FLAGS			(TYPE_BUS_CLK | SW_GATE | ENABLE_ON_INIT)
#define		BCM2165x_CLK_SIM_APB_FLAGS			(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_SPM_APB_FLAGS			(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_SIM2_APB_FLAGS			(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_DAP_FLAGS				(TYPE_BUS_CLK | SW_GATE)
/* Under KHUB CCU */
#define		BCM2165x_CLK_DAP_SWITCH_FLAGS			(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_BROM_FLAGS				(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_NOR_APB_FLAGS			(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_NOR_FLAGS				(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_MDIOMASTER_FLAGS			(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_APB5_FLAGS				(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_CTI_APB_FLAGS			(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_FUNNEL_APB_FLAGS			(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_TPIU_APB_FLAGS			(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_VC_ITM_APB_FLAGS			(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_SEC_VIOL_TRAP_4_APB_FLAGS		(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_SEC_VIOL_TRAP_5_APB_FLAGS		(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_SEC_VIOL_TRAP_7_APB_FLAGS		(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_HSI_APB_FLAGS			(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_AXI_TRACE_19_APB_FLAGS		(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_AXI_TRACE_11_APB_FLAGS		(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_AXI_TRACE_12_APB_FLAGS		(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_AXI_TRACE_13_APB_FLAGS		(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_ETB_APB_FLAGS			(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_FINAL_FUNNEL_APB_FLAGS		(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_APB10_FLAGS			(TYPE_BUS_CLK | SW_GATE | DEFAULT_ACTIVE_CLK)
#define		BCM2165x_CLK_APB9_FLAGS				(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_ATB_FILTER_APB_FLAGS		(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_BT_SLIM_AHB_APB_FLAGS		(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_ETB2AXI_APB_FLAGS			(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_SSP3_APB_FLAGS			(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_AUDIOH_APB_FLAGS			(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_TMON_APB_FLAGS			(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_VAR_SPM_APB_FLAGS			(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_SSP4_APB_FLAGS			(TYPE_BUS_CLK | SW_GATE)

/*TBD*/
#define		BCM2165x_CLK_I2S_APB_FLAGS			(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_USB_IC_AHB_FLAGS			(TYPE_BUS_CLK | SW_GATE)
//#define		BCM2165x_CLK_EHCI_48M_AHB_FLAGS			(TYPE_BUS_CLK | SW_GATE)
//#define		BCM2165x_CLK_EHCI_12M_AHB_FLAGS			(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_NAND_AHB_FLAGS			(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_AUDIOH_APB_FLAGS			(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_AUDIOH_156M_FLAGS			(TYPE_BUS_CLK | SW_GATE)
#define		BCM2165x_CLK_AUDIOH_2P4M_FLAGS			(TYPE_BUS_CLK | SW_GATE)

/**************************** Peripheral clock flags **************************/
#define		BCM2165x_CLK_USB_OTG_FLAGS			(TYPE_PERI_CLK | SW_GATE)
#define		BCM2165x_CLK_SDIO1_FLAGS			(TYPE_PERI_CLK | SW_GATE | DISABLE_ON_INIT)
#define		BCM2165x_CLK_SDIO2_FLAGS			(TYPE_PERI_CLK | SW_GATE | DISABLE_ON_INIT)
#define		BCM2165x_CLK_SDIO3_FLAGS			(TYPE_PERI_CLK | SW_GATE | DISABLE_ON_INIT)
#define		BCM2165x_CLK_SDIO4_FLAGS			(TYPE_PERI_CLK | SW_GATE | DISABLE_ON_INIT)
#define		BCM2165x_CLK_SSP0_FLAGS				(TYPE_PERI_CLK | SW_GATE)
#define		BCM2165x_CLK_UARTB_FLAGS			(TYPE_PERI_CLK | SW_GATE)
#define		BCM2165x_CLK_UARTB2_FLAGS			(TYPE_PERI_CLK | SW_GATE)
#define		BCM2165x_CLK_UARTB3_FLAGS			(TYPE_PERI_CLK | SW_GATE)
#define         BCM2165x_CLK_UARTB4_FLAGS                       (TYPE_PERI_CLK | SW_GATE)
#define		BCM2165x_CLK_SPUM_OPEN_FLAGS			(TYPE_PERI_CLK | SW_GATE)
#define		BCM2165x_CLK_SPUM_SEC_FLAGS			(TYPE_PERI_CLK | SW_GATE)
#define		BCM2165x_CLK_SMI_FLAGS				(TYPE_PERI_CLK | SW_GATE)
#define		BCM2165x_CLK_PMU_BSC_FLAGS			(TYPE_PERI_CLK | SW_GATE)
#define		BCM2165x_CLK_SIM_FLAGS				(TYPE_PERI_CLK | SW_GATE | ENABLE_ON_INIT)
#define		BCM2165x_CLK_SIM2_FLAGS				(TYPE_PERI_CLK | SW_GATE | ENABLE_ON_INIT)
#define		BCM2165x_CLK_SSP0_AUDIO_FLAGS			(TYPE_PERI_CLK | SW_GATE)

/* Under KHUB CCU */
#define		BCM2165x_CLK_HUB_FLAGS				(TYPE_PERI_CLK | SW_GATE)
#define		BCM2165x_CLK_CAPH_SRCMIXER_FLAGS		(TYPE_PERI_CLK | SW_GATE)
#define		BCM2165x_CLK_SSP3_FLAGS				(TYPE_PERI_CLK | SW_GATE)
#define		BCM2165x_CLK_SSP4_FLAGS				(TYPE_PERI_CLK | SW_GATE)
#define		BCM2165x_CLK_SSP3_AUDIO_FLAGS			(TYPE_PERI_CLK | SW_GATE)
#define		BCM2165x_CLK_SSP4_AUDIO_FLAGS			(TYPE_PERI_CLK | SW_GATE)
/*under MM CCU*/
#define		BCM2165x_CLK_CSI0_LP_FLAGS			(TYPE_PERI_CLK | SW_GATE)
#define		BCM2165x_CLK_CSI1_LP_FLAGS			(TYPE_PERI_CLK | SW_GATE)
#define		BCM2165x_CLK_DSI0_ESC_FLAGS			(TYPE_PERI_CLK | SW_GATE)
#define		BCM2165x_CLK_DSI1_ESC_FLAGS			(TYPE_PERI_CLK | SW_GATE)
#define		BCM2165x_CLK_DSI_PLL_FLAGS			(TYPE_PERI_CLK | SW_GATE)

/**************************** Peripheral clock(No DIV) flags **************************/
#define		BCM2165x_CLK_BSC1_FLAGS				(TYPE_PERI_CLK | SW_GATE)
#define		BCM2165x_CLK_BSC2_FLAGS				(TYPE_PERI_CLK | SW_GATE)
#define		BCM2165x_CLK_TIMERS_FLAGS			(TYPE_PERI_CLK | SW_GATE | DISABLE_ON_INIT)
#define		BCM2165x_CLK_HUB_TIMER_FLAGS			(TYPE_PERI_CLK | SW_GATE)
#define		BCM2165x_CLK_TMON_1M_FLAGS			(TYPE_PERI_CLK | SW_GATE)

/*TBD*/
#define		BCM2165x_CLK_DMAC_FLAGS				(TYPE_PERI_CLK | SW_GATE)
#define		BCM2165x_CLK_NAND_FLAGS				(TYPE_PERI_CLK | SW_GATE)
//#define		BCM2165x_CLK_EHCI_12M_FLAGS			(TYPE_PERI_CLK | SW_GATE)
#define		BCM2165x_CLK_USBH_48M_FLAGS			(TYPE_PERI_CLK | SW_GATE)
#define		BCM2165x_CLK_USB_IC_FLAGS			(TYPE_PERI_CLK | SW_GATE)
#define		BCM2165x_CLK_I2S_FRAC_FLAGS			(TYPE_PERI_CLK | SW_GATE)
#define		BCM2165x_CLK_I2S_FLAGS				(TYPE_PERI_CLK | SW_GATE)
#define		BCM2165x_CLK_AUDIOH_26M_FLAGS			(TYPE_PERI_CLK | SW_GATE)


/******** Divider max, pre-divider max and fraction max for perepheral clocks  *********/
#define		CLK_SDIO1_DIV_MAX				0x3FFF
#define		CLK_SDIO1_PREDIV_MAX				1
#define		CLK_SDIO1_FRACTION_MAX				1

#define		CLK_SDIO2_DIV_MAX				0x3FFF
#define		CLK_SDIO2_PREDIV_MAX				1
#define		CLK_SDIO2_FRACTION_MAX				1

#define		CLK_SDIO3_DIV_MAX				0x3FFF
#define		CLK_SDIO3_PREDIV_MAX				1
#define		CLK_SDIO3_FRACTION_MAX				1

#define		CLK_SDIO4_DIV_MAX				0x3FFF
#define		CLK_SDIO4_PREDIV_MAX				1
#define		CLK_SDIO4_FRACTION_MAX				1

#define		CLK_SSP0_DIV_MAX				0x3FFF
#define		CLK_SSP0_PREDIV_MAX				1
#define		CLK_SSP0_FRACTION_MAX				1

#define		CLK_SSP0_AUDIO_DIV_MAX				0xFFF
#define		CLK_SSP0_AUDIO_PREDIV_MAX			0xF
#define		CLK_SSP0_AUDIO_FRACTION_MAX			1

#define		CLK_BSC1_DIV_MAX				1
#define		CLK_BSC1_PREDIV_MAX				1
#define		CLK_BSC1_FRACTION_MAX				1

#define		CLK_BSC2_DIV_MAX				1
#define		CLK_BSC2_PREDIV_MAX				1
#define		CLK_BSC2_FRACTION_MAX				1

#define		CLK_UARTB_DIV_MAX				0xF
#define		CLK_UARTB_PREDIV_MAX				1
#define		CLK_UARTB_FRACTION_MAX				0xFF

#define		CLK_UARTB2_DIV_MAX				0xF
#define		CLK_UARTB2_PREDIV_MAX				1
#define		CLK_UARTB2_FRACTION_MAX				0xFF

#define		CLK_UARTB3_DIV_MAX				0xF
#define		CLK_UARTB3_PREDIV_MAX				1
#define		CLK_UARTB3_FRACTION_MAX				0xFF

#define         CLK_UARTB4_DIV_MAX                              0xF
#define         CLK_UARTB4_PREDIV_MAX                           1
#define         CLK_UARTB4_FRACTION_MAX                         0xFF

#define		CLK_TIMERS_DIV_MAX				1
#define		CLK_TIMERS_PREDIV_MAX				1
#define		CLK_TIMERS_FRACTION_MAX				1

#define		CLK_SPUM_OPEN_DIV_MAX				0x7
#define		CLK_SPUM_OPEN_PREDIV_MAX			1
#define		CLK_SPUM_OPEN_FRACTION_MAX			1

#define		CLK_SPUM_SEC_DIV_MAX				0x7
#define		CLK_SPUM_SEC_PREDIV_MAX				1
#define		CLK_SPUM_SEC_FRACTION_MAX			1

#define		CLK_SMI_DIV_MAX					0xF
#define		CLK_SMI_PREDIV_MAX				1
#define		CLK_SMI_FRACTION_MAX				1

#define		CLK_SIM_DIV_MAX					0xF
#define		CLK_SIM_PREDIV_MAX				0xF
#define		CLK_SIM_FRACTION_MAX				1

#define		CLK_SIM2_DIV_MAX				0xF
#define		CLK_SIM2_PREDIV_MAX				0xF
#define		CLK_SIM2_FRACTION_MAX				1

#define		CLK_AUDIOH_26M_DIV_MAX				1
#define		CLK_AUDIOH_26M_PREDIV_MAX			1
#define		CLK_AUDIOH_26M_FRACTION_MAX			1

#define		CLK_PMU_BSC_DIV_MAX				0xF
#define		CLK_PMU_BSC_PREDIV_MAX				1
#define		CLK_PMU_BSC_FRACTION_MAX			1

#define		CLK_TMON_1M_DIV_MAX				1
#define		CLK_TMON_1M_PREDIV_MAX				1
#define		CLK_TMON_1M_FRACTION_MAX			1

#define		CLK_CAPH_SRCMIXER_DIV_MAX			0x3
#define		CLK_CAPH_SRCMIXER_PREDIV_MAX			1
#define		CLK_CAPH_SRCMIXER_FRACTION_MAX			1

#define		CLK_HUB_DIV_MAX					0xF
#define		CLK_HUB_PREDIV_MAX				1
#define		CLK_HUB_FRACTION_MAX				1

#define		CLK_CSI0_LP_DIV_MAX				0x7
#define		CLK_CSI0_LP_PREDIV_MAX				1
#define		CLK_CSI0_LP_FRACTION_MAX			1

#define		CLK_CSI1_LP_DIV_MAX				0x7
#define		CLK_CSI1_LP_PREDIV_MAX				1
#define		CLK_CSI1_LP_FRACTION_MAX			1

#define		CLK_DSI0_ESC_DIV_MAX				0x7
#define		CLK_DSI0_ESC_PREDIV_MAX				1
#define		CLK_DSI0_ESC_FRACTION_MAX			1

#define		CLK_DSI1_ESC_DIV_MAX				0x7
#define		CLK_DSI1_ESC_PREDIV_MAX				1
#define		CLK_DSI1_ESC_FRACTION_MAX			1

#define		CLK_DSI_PLL_DIV_MAX				0xF
#define		CLK_DSI_PLL_PREDIV_MAX				1
#define		CLK_DSI_PLL_FRACTION_MAX			1

#define		CLK_SSP3_DIV_MAX				0xFFF
#define		CLK_SSP3_PREDIV_MAX				0xF
#define		CLK_SSP3_FRACTION_MAX				1

#define		CLK_SSP3_AUDIO_DIV_MAX				0xFFF
#define		CLK_SSP3_AUDIO_PREDIV_MAX			0xF
#define		CLK_SSP3_AUDIO_FRACTION_MAX			1

#define		CLK_SSP4_DIV_MAX				0xFFF
#define		CLK_SSP4_PREDIV_MAX				0xF
#define		CLK_SSP4_FRACTION_MAX				1

#define		CLK_SSP4_AUDIO_DIV_MAX				0xFFF
#define		CLK_SSP4_AUDIO_PREDIV_MAX			0xF
#define		CLK_SSP4_AUDIO_FRACTION_MAX			1

/* declare a reference clock */
#define	DECLARE_REF_CLK(clk_name, clk_id, clk_rate, clk_div, clk_parent)		\
	static struct proc_clock clk_name##_clk = {				\
		.clk	=	{						\
			.name	=	__stringify(clk_name##_clk),		\
			.parent	=	clk_parent,				\
			.rate	=	clk_rate,				\
			.div	=	clk_div,				\
			.id	=	BCM2165x_CLK_##clk_id,			\
			.ops	=	&ref_clk_ops,				\
		},								\
	}

/* declare c CCU clock */
#define	DECLARE_CCU_CLK(clk_name, id, ccu, pfx, mask, ... )						\
	static struct ccu_clock clk_name##_clk = {						\
		.clk	=	{								\
			.name	=	__stringify(clk_name##_clk),				\
			.ops	=	&ccu_clk_ops,						\
			.ccu_id =       BCM2165x_##ccu##_CCU,					\
			.flags	=	BCM2165x_##ccu##_CCU_FLAGS,				\
		},										\
		.ccu_clk_mgr_base	=	ccu##_CLK_BASE_ADDR,				\
		.wr_access_offset	=	pfx##_CLK_MGR_REG_WR_ACCESS_OFFSET,		\
		.policy_freq_offset	=	pfx##_CLK_MGR_REG_POLICY_FREQ_OFFSET,		\
		.freq_bit_shift 	=	pfx##_CLK_MGR_REG_POLICY_FREQ_POLICY1_FREQ_SHIFT,	\
		.policy_ctl_offset	=	pfx##_CLK_MGR_REG_POLICY_CTL_OFFSET,		\
		.policy0_mask_offset	=	pfx##_CLK_MGR_REG_POLICY0_##mask##_OFFSET,		\
		.policy1_mask_offset	=	pfx##_CLK_MGR_REG_POLICY1_##mask##_OFFSET,		\
		.policy2_mask_offset	=	pfx##_CLK_MGR_REG_POLICY2_##mask##_OFFSET,		\
		.policy3_mask_offset	=	pfx##_CLK_MGR_REG_POLICY3_##mask##_OFFSET,		\
		.lvm_en_offset		=	pfx##_CLK_MGR_REG_LVM_EN_OFFSET,		\
		.freq_id	=	id,							\
		.freq_tbl	=	{__VA_ARGS__},						\
	}

/* declare c CCU clock */
#define	DECLARE_CCU_CLK2(clk_name, id, ccu, pfx, mask, mask1, ... )						\
	static struct ccu_clock clk_name##_clk = {						\
		.clk	=	{								\
			.name	=	__stringify(clk_name##_clk),				\
			.ops	=	&ccu_clk_ops,						\
			.ccu_id =       BCM2165x_##ccu##_CCU,					\
			.flags	=	BCM2165x_##ccu##_CCU_FLAGS,				\
		},										\
		.ccu_clk_mgr_base	=	ccu##_CLK_BASE_ADDR,				\
		.wr_access_offset	=	pfx##_CLK_MGR_REG_WR_ACCESS_OFFSET,		\
		.policy_freq_offset	=	pfx##_CLK_MGR_REG_POLICY_FREQ_OFFSET,		\
		.freq_bit_shift 	=	pfx##_CLK_MGR_REG_POLICY_FREQ_POLICY1_FREQ_SHIFT,	\
		.policy_ctl_offset	=	pfx##_CLK_MGR_REG_POLICY_CTL_OFFSET,		\
		.policy0_mask_offset	=	pfx##_CLK_MGR_REG_POLICY0_##mask##_OFFSET,		\
		.policy1_mask_offset	=	pfx##_CLK_MGR_REG_POLICY1_##mask##_OFFSET,		\
		.policy2_mask_offset	=	pfx##_CLK_MGR_REG_POLICY2_##mask##_OFFSET,		\
		.policy3_mask_offset	=	pfx##_CLK_MGR_REG_POLICY3_##mask##_OFFSET,		\
		.policy0_mask1_offset	=	pfx##_CLK_MGR_REG_POLICY0_##mask1##_OFFSET,		\
		.policy1_mask1_offset	=	pfx##_CLK_MGR_REG_POLICY1_##mask1##_OFFSET,		\
		.policy2_mask1_offset	=	pfx##_CLK_MGR_REG_POLICY2_##mask1##_OFFSET,		\
		.policy3_mask1_offset	=	pfx##_CLK_MGR_REG_POLICY3_##mask1##_OFFSET,		\
		.lvm_en_offset		=	pfx##_CLK_MGR_REG_LVM_EN_OFFSET,		\
		.freq_id	=	id,							\
		.freq_tbl	=	{__VA_ARGS__},						\
	}


/* declare a bus clock*/
#define	DECLARE_BUS_CLK(clk_name, NAME1, NAME2, clk_parent, ccu, pfx, ... )			\
	static struct bus_clock clk_name##_clk = {						\
		.clk	=	{								\
			.name	=	__stringify(clk_name##_clk),				\
			.parent =	name_to_clk(clk_parent),				\
			.id	=	BCM2165x_CLK_##NAME2,							\
			.ccu_id =       BCM2165x_##ccu##_CCU,					\
			.flags	=	BCM2165x_CLK_##NAME2##_FLAGS,				\
			.ops	=	&bus_clk_ops,						\
		},										\
		.ccu_clk_mgr_base	=	ccu##_CLK_BASE_ADDR,				\
		.wr_access_offset	=	pfx##_CLK_MGR_REG_WR_ACCESS_OFFSET,		\
		.clkgate_offset 	=	pfx##_CLK_MGR_REG_##NAME1##_CLKGATE_OFFSET,	\
		.stprsts_mask		=	pfx##_CLK_MGR_REG_##NAME1##_CLKGATE_##NAME2##_STPRSTS_MASK,	\
		.hw_sw_gating_mask	=	pfx##_CLK_MGR_REG_##NAME1##_CLKGATE_##NAME2##_HW_SW_GATING_SEL_SHIFT,	\
		.clk_en_mask		=	pfx##_CLK_MGR_REG_##NAME1##_CLKGATE_##NAME2##_CLK_EN_MASK,	\
		.freq_tbl	=	{	__VA_ARGS__	},				\
	}

/* declare a bus clock no H/W S/W gating selection control. It will have clock
 * enable/disable control*/
#define	DECLARE_BUS_CLK_NO_GATING_SEL(clk_name, NAME1, NAME2, clk_parent, ccu, pfx, ... )		\
	static struct bus_clock clk_name##_clk = {						\
		.clk	=	{								\
			.name	=	__stringify(clk_name##_clk),				\
			.parent =	name_to_clk(clk_parent),				\
			.id	=	BCM2165x_CLK_##NAME2,							\
			.ccu_id =       BCM2165x_##ccu##_CCU,					\
			.flags	=	BCM2165x_CLK_##NAME2##_FLAGS,				\
			.ops	=	&bus_clk_ops,						\
		},										\
		.ccu_clk_mgr_base	=	ccu##_CLK_BASE_ADDR,				\
		.wr_access_offset	=	pfx##_CLK_MGR_REG_WR_ACCESS_OFFSET,		\
		.clkgate_offset 	=	pfx##_CLK_MGR_REG_##NAME1##_CLKGATE_OFFSET,	\
		.stprsts_mask		=	pfx##_CLK_MGR_REG_##NAME1##_CLKGATE_##NAME2##_STPRSTS_MASK,	\
		.clk_en_mask		=	pfx##_CLK_MGR_REG_##NAME1##_CLKGATE_##NAME2##_CLK_EN_MASK,	\
		.freq_tbl	=	{	__VA_ARGS__	},				\
	}

/* declare a peripheral clock */
#define	DECLARE_PERI_CLK(clk_name, NAME1, NAME2, clk_parent, clk_rate, clk_div, trigger, ccu, pfx, dthr)	\
	static struct peri_clock clk_name##_clk = {						\
		.clk	=	{								\
			.name	=	__stringify(clk_name##_clk),				\
			.parent =	name_to_clk(clk_parent),				\
			.rate	=	clk_rate,						\
			.div	=	clk_div,						\
			.id	=	BCM2165x_CLK_##NAME2,							\
			.ccu_id =       BCM2165x_##ccu##_CCU,					\
			.flags	=	BCM2165x_CLK_##NAME2##_FLAGS,				\
			.src	=	&clk_name##_clk_src,					\
			.ops	=	&peri_clk_ops,						\
		},										\
		.ccu_clk_mgr_base	=	ccu##_CLK_BASE_ADDR,				\
		.wr_access_offset	=	pfx##_CLK_MGR_REG_WR_ACCESS_OFFSET,		\
		.clkgate_offset 	=	pfx##_CLK_MGR_REG_##NAME1##_CLKGATE_OFFSET,	\
		.div_offset		=	pfx##_CLK_MGR_REG_##NAME1##_DIV_OFFSET,	\
		.div_trig_offset	=	pfx##_CLK_MGR_REG_##trigger##_OFFSET,		\
		.stprsts_mask		=	pfx##_CLK_MGR_REG_##NAME1##_CLKGATE_##NAME2##_STPRSTS_MASK,		\
		.hw_sw_gating_mask	=	pfx##_CLK_MGR_REG_##NAME1##_CLKGATE_##NAME2##_HW_SW_GATING_SEL_MASK,	\
		.clk_en_mask		=	pfx##_CLK_MGR_REG_##NAME1##_CLKGATE_##NAME2##_CLK_EN_MASK,		\
		.div_mask		=	pfx##_CLK_MGR_REG_##NAME1##_DIV_##NAME2##_DIV_MASK,		\
		.div_shift		=	pfx##_CLK_MGR_REG_##NAME1##_DIV_##NAME2##_DIV_SHIFT,		\
		.div_max		=	CLK_##NAME2##_DIV_MAX,						\
		.pre_div_max		=	CLK_##NAME2##_PREDIV_MAX,						\
		.div_dithering		=	dthr,									\
		.pll_select_mask	=	pfx##_CLK_MGR_REG_##NAME1##_DIV_##NAME2##_PLL_SELECT_MASK,	\
		.pll_select_shift	=	pfx##_CLK_MGR_REG_##NAME1##_DIV_##NAME2##_PLL_SELECT_SHIFT,	\
		.trigger_mask		=	pfx##_CLK_MGR_REG_##trigger##_##NAME2##_TRIGGER_MASK,	\
	}

#define	DECLARE_PERI_CLK_PRE_DIV(clk_name, NAME1, NAME2, clk_parent, clk_rate, clk_div, trigger, ccu, pfx, dthr)	\
	static struct peri_clock clk_name##_clk = {						\
		.clk	=	{								\
			.name	=	__stringify(clk_name##_clk),				\
			.parent =	name_to_clk(clk_parent),				\
			.rate	=	clk_rate,						\
			.div	=	clk_div,						\
			.id	=	BCM2165x_CLK_##NAME2,							\
			.ccu_id =       BCM2165x_##ccu##_CCU,					\
			.flags	=	BCM2165x_CLK_##NAME2##_FLAGS,				\
			.src	=	&clk_name##_clk_src,					\
			.ops	=	&peri_clk_ops,						\
		},										\
		.ccu_clk_mgr_base	=	ccu##_CLK_BASE_ADDR,				\
		.wr_access_offset	=	pfx##_CLK_MGR_REG_WR_ACCESS_OFFSET,		\
		.clkgate_offset 	=	pfx##_CLK_MGR_REG_##NAME1##_CLKGATE_OFFSET,	\
		.div_offset		=	pfx##_CLK_MGR_REG_##NAME1##_DIV_OFFSET,	\
		.div_trig_offset	=	pfx##_CLK_MGR_REG_##trigger##_OFFSET,		\
		.stprsts_mask		=	pfx##_CLK_MGR_REG_##NAME1##_CLKGATE_##NAME2##_STPRSTS_MASK,		\
		.hw_sw_gating_mask	=	pfx##_CLK_MGR_REG_##NAME1##_CLKGATE_##NAME2##_HW_SW_GATING_SEL_MASK,	\
		.clk_en_mask		=	pfx##_CLK_MGR_REG_##NAME1##_CLKGATE_##NAME2##_CLK_EN_MASK,		\
		.div_mask		=	pfx##_CLK_MGR_REG_##NAME1##_DIV_##NAME2##_DIV_MASK,		\
		.div_shift		=	pfx##_CLK_MGR_REG_##NAME1##_DIV_##NAME2##_DIV_SHIFT,		\
		.div_max		=	CLK_##NAME2##_DIV_MAX,						\
		.pre_div_mask		=	pfx##_CLK_MGR_REG_##NAME1##_DIV_##NAME2##_PRE_DIV_MASK,		\
		.pre_div_shift		=	pfx##_CLK_MGR_REG_##NAME1##_DIV_##NAME2##_PRE_DIV_SHIFT,		\
		.pre_div_max		=	CLK_##NAME2##_PREDIV_MAX,						\
		.div_dithering		=	dthr,									\
		.pll_select_mask	=	pfx##_CLK_MGR_REG_##NAME1##_DIV_##NAME2##_PRE_PLL_SELECT_MASK,	\
		.pll_select_shift	=	pfx##_CLK_MGR_REG_##NAME1##_DIV_##NAME2##_PRE_PLL_SELECT_SHIFT,	\
		.trigger_mask		=	pfx##_CLK_MGR_REG_##trigger##_##NAME2##_TRIGGER_MASK,	\
		.pre_trigger_mask	=	pfx##_CLK_MGR_REG_##trigger##_##NAME2##_PRE_TRIGGER_MASK,	\
	}

/* for clock gate and div register with different prefix
 * see ssp3_audio in Island Khub
 * */
#define	DECLARE_PERI_CLK_PRE_DIV2(clk_name, NAME1, NAME2, NAME3, clk_parent, clk_rate, clk_div, trigger, ccu, pfx, dthr)	\
	static struct peri_clock clk_name##_clk = {						\
		.clk	=	{								\
			.name	=	__stringify(clk_name##_clk),				\
			.parent =	name_to_clk(clk_parent),				\
			.rate	=	clk_rate,						\
			.div	=	clk_div,						\
			.id	=	BCM2165x_CLK_##NAME2,							\
			.ccu_id =       BCM2165x_##ccu##_CCU,					\
			.flags	=	BCM2165x_CLK_##NAME2##_FLAGS,				\
			.src	=	&clk_name##_clk_src,					\
			.ops	=	&peri_clk_ops,						\
		},										\
		.ccu_clk_mgr_base	=	ccu##_CLK_BASE_ADDR,				\
		.wr_access_offset	=	pfx##_CLK_MGR_REG_WR_ACCESS_OFFSET,		\
		.clkgate_offset 	=	pfx##_CLK_MGR_REG_##NAME1##_CLKGATE_OFFSET,	\
		.div_offset		=	pfx##_CLK_MGR_REG_##NAME1##_DIV_OFFSET,	\
		.div_trig_offset	=	pfx##_CLK_MGR_REG_##trigger##_OFFSET,		\
		.stprsts_mask		=	pfx##_CLK_MGR_REG_##NAME1##_CLKGATE_##NAME2##_STPRSTS_MASK,		\
		.hw_sw_gating_mask	=	pfx##_CLK_MGR_REG_##NAME1##_CLKGATE_##NAME2##_HW_SW_GATING_SEL_MASK,	\
		.clk_en_mask		=	pfx##_CLK_MGR_REG_##NAME1##_CLKGATE_##NAME2##_CLK_EN_MASK,		\
		.div_mask		=	pfx##_CLK_MGR_REG_##NAME3##_DIV_##NAME2##_DIV_MASK,		\
		.div_shift		=	pfx##_CLK_MGR_REG_##NAME3##_DIV_##NAME2##_DIV_SHIFT,		\
		.div_max		=	CLK_##NAME2##_DIV_MAX,						\
		.pre_div_mask		=	pfx##_CLK_MGR_REG_##NAME3##_DIV_##NAME2##_PRE_DIV_MASK,		\
		.pre_div_shift		=	pfx##_CLK_MGR_REG_##NAME3##_DIV_##NAME2##_PRE_DIV_SHIFT,		\
		.pre_div_max		=	CLK_##NAME2##_PREDIV_MAX,						\
		.div_dithering		=	dthr,									\
		.pll_select_mask	=	pfx##_CLK_MGR_REG_##NAME3##_DIV_##NAME2##_PRE_PLL_SELECT_MASK,	\
		.pll_select_shift	=	pfx##_CLK_MGR_REG_##NAME3##_DIV_##NAME2##_PRE_PLL_SELECT_SHIFT,	\
		.trigger_mask		=	pfx##_CLK_MGR_REG_##trigger##_##NAME2##_TRIGGER_MASK,	\
		.pre_trigger_mask	=	pfx##_CLK_MGR_REG_##trigger##_##NAME2##_PRE_TRIGGER_MASK,	\
	}

/* for clock gate and div register with different prefix
 * see ssp0_audio in Island IKPS
 * */
#define	DECLARE_PERI_CLK_PRE_DIV3(clk_name, NAME1, NAME2, NAME3, clk_parent, clk_rate, clk_div, trigger, ccu, pfx, dthr)	\
	static struct peri_clock clk_name##_clk = {						\
		.clk	=	{								\
			.name	=	__stringify(clk_name##_clk),				\
			.parent =	name_to_clk(clk_parent),				\
			.rate	=	clk_rate,						\
			.div	=	clk_div,						\
			.pre_div = 1,                    \
			.id	=	BCM2165x_CLK_##NAME2,							\
			.ccu_id =       BCM2165x_##ccu##_CCU,					\
			.flags	=	BCM2165x_CLK_##NAME2##_FLAGS,				\
			.src	=	&clk_name##_clk_src,					\
			.ops	=	&peri_clk_ops,						\
		},										\
		.ccu_clk_mgr_base	=	ccu##_CLK_BASE_ADDR,				\
		.wr_access_offset	=	pfx##_CLK_MGR_REG_WR_ACCESS_OFFSET,		\
		.clkgate_offset 	=	pfx##_CLK_MGR_REG_##NAME1##_CLKGATE_OFFSET,	\
		.div_offset		=	pfx##_CLK_MGR_REG_##NAME2##_DIV_OFFSET,	\
		.div_trig_offset	=	pfx##_CLK_MGR_REG_##trigger##_OFFSET,		\
		.stprsts_mask		=	pfx##_CLK_MGR_REG_##NAME1##_CLKGATE_##NAME2##_STPRSTS_MASK,		\
		.hw_sw_gating_mask	=	pfx##_CLK_MGR_REG_##NAME1##_CLKGATE_##NAME2##_HW_SW_GATING_SEL_MASK,	\
		.clk_en_mask		=	pfx##_CLK_MGR_REG_##NAME1##_CLKGATE_##NAME2##_CLK_EN_MASK,		\
		.div_mask		=	pfx##_CLK_MGR_REG_##NAME3##_DIV_##NAME2##_DIV_MASK,		\
		.div_shift		=	pfx##_CLK_MGR_REG_##NAME3##_DIV_##NAME2##_DIV_SHIFT,		\
		.div_max		=	CLK_##NAME2##_DIV_MAX,						\
		.pre_div_mask		=	pfx##_CLK_MGR_REG_##NAME3##_DIV_##NAME2##_PRE_DIV_MASK,		\
		.pre_div_shift		=	pfx##_CLK_MGR_REG_##NAME3##_DIV_##NAME2##_PRE_DIV_SHIFT,		\
		.pre_div_max		=	CLK_##NAME2##_PREDIV_MAX,						\
		.div_dithering		=	dthr,									\
		.pll_select_mask	=	pfx##_CLK_MGR_REG_##NAME3##_DIV_##NAME2##_PRE_PLL_SELECT_MASK,	\
		.pll_select_shift	=	pfx##_CLK_MGR_REG_##NAME3##_DIV_##NAME2##_PRE_PLL_SELECT_SHIFT,	\
		.trigger_mask		=	pfx##_CLK_MGR_REG_##trigger##_##NAME2##_TRIGGER_MASK,	\
		.pre_trigger_mask	=	pfx##_CLK_MGR_REG_##trigger##_##NAME2##_PRE_TRIGGER_MASK,	\
	}


/* declare a peripheral clock without divider count value. It will have source
 * selection*/
#define	DECLARE_PERI_CLK_NO_DIV_COUNT(clk_name, CLK_NAME, clk_parent, clk_rate, trigger, ccu, pfx)	\
	static struct peri_clock clk_name##_clk = {						\
		.clk	=	{								\
			.name	=	__stringify(clk_name##_clk),				\
			.parent =	name_to_clk(clk_parent),				\
			.rate	=	clk_rate,						\
			.div	=	1,							\
			.id	=	BCM2165x_CLK_##CLK_NAME,				\
			.ccu_id =       BCM2165x_##ccu##_CCU,					\
			.flags	=	BCM2165x_CLK_##CLK_NAME##_FLAGS,				\
			.src	=	&clk_name##_clk_src,					\
			.ops	=	&peri_clk_ops,						\
		},										\
		.ccu_clk_mgr_base	=	ccu##_CLK_BASE_ADDR,				\
		.wr_access_offset	=	pfx##_CLK_MGR_REG_WR_ACCESS_OFFSET,		\
		.clkgate_offset 	=	pfx##_CLK_MGR_REG_##CLK_NAME##_CLKGATE_OFFSET,	\
		.div_offset		=	pfx##_CLK_MGR_REG_##CLK_NAME##_DIV_OFFSET,	\
		.div_trig_offset	=	pfx##_CLK_MGR_REG_##trigger##_OFFSET,		\
		.stprsts_mask		=	pfx##_CLK_MGR_REG_##CLK_NAME##_CLKGATE_##CLK_NAME##_STPRSTS_MASK,		\
		.hw_sw_gating_mask	=	pfx##_CLK_MGR_REG_##CLK_NAME##_CLKGATE_##CLK_NAME##_HW_SW_GATING_SEL_MASK,	\
		.clk_en_mask		=	pfx##_CLK_MGR_REG_##CLK_NAME##_CLKGATE_##CLK_NAME##_CLK_EN_MASK,		\
		.pll_select_mask	=	pfx##_CLK_MGR_REG_##CLK_NAME##_DIV_##CLK_NAME##_PLL_SELECT_MASK,	\
		.pll_select_shift	=	pfx##_CLK_MGR_REG_##CLK_NAME##_DIV_##CLK_NAME##_PLL_SELECT_SHIFT,	\
		.trigger_mask		=	pfx##_CLK_MGR_REG_##trigger##_##CLK_NAME##_TRIGGER_MASK,		\
	}

#define	DECLARE_PERI_CLK_NO_DIV_COUNT2(clk_name, CLK_NAME, NAME2, clk_parent, clk_rate, trigger, ccu, pfx)	\
	static struct peri_clock clk_name##_clk = {						\
		.clk	=	{								\
			.name	=	__stringify(clk_name##_clk),				\
			.parent =	name_to_clk(clk_parent),				\
			.rate	=	clk_rate,						\
			.div	=	1,							\
			.id	=	BCM2165x_CLK_##CLK_NAME,				\
			.ccu_id =       BCM2165x_##ccu##_CCU,					\
			.flags	=	BCM2165x_CLK_##CLK_NAME##_FLAGS,				\
			.src	=	&clk_name##_clk_src,					\
			.ops	=	&peri_clk_ops,						\
		},										\
		.ccu_clk_mgr_base	=	ccu##_CLK_BASE_ADDR,				\
		.wr_access_offset	=	pfx##_CLK_MGR_REG_WR_ACCESS_OFFSET,		\
		.clkgate_offset 	=	pfx##_CLK_MGR_REG_##NAME2##_CLKGATE_OFFSET,	\
		.div_offset		=	pfx##_CLK_MGR_REG_##NAME2##_DIV_OFFSET,	\
		.div_trig_offset	=	pfx##_CLK_MGR_REG_##trigger##_OFFSET,		\
		.stprsts_mask		=	pfx##_CLK_MGR_REG_##NAME2##_CLKGATE_##CLK_NAME##_STPRSTS_MASK,		\
		.hw_sw_gating_mask	=	pfx##_CLK_MGR_REG_##NAME2##_CLKGATE_##CLK_NAME##_HW_SW_GATING_SEL_MASK,	\
		.clk_en_mask		=	pfx##_CLK_MGR_REG_##NAME2##_CLKGATE_##CLK_NAME##_CLK_EN_MASK,		\
		.pll_select_mask	=	pfx##_CLK_MGR_REG_##NAME2##_DIV_##CLK_NAME##_PLL_SELECT_MASK,	\
		.pll_select_shift	=	pfx##_CLK_MGR_REG_##NAME2##_DIV_##CLK_NAME##_PLL_SELECT_SHIFT,	\
		.trigger_mask		=	pfx##_CLK_MGR_REG_##trigger##_##CLK_NAME##_TRIGGER_MASK,		\
	}

#define	DECLARE_PERI_CLK_FIXED_DIV(clk_name, NAME1, NAME2, clk_parent, clk_rate, trigger, ccu, pfx)	\
	static struct peri_clock clk_name##_clk = {						\
		.clk	=	{								\
			.name	=	__stringify(clk_name##_clk),				\
			.parent =	name_to_clk(clk_parent),				\
			.rate	=	clk_rate,						\
			.div	=	1,							\
			.id	=	BCM2165x_CLK_##NAME2,					\
			.ccu_id =       BCM2165x_##ccu##_CCU,					\
			.flags	=	BCM2165x_CLK_##NAME2##_FLAGS,				\
			.src	=	&clk_name##_clk_src,					\
			.ops	=	&peri_clk_ops,						\
		},										\
		.ccu_clk_mgr_base	=	ccu##_CLK_BASE_ADDR,				\
		.wr_access_offset	=	pfx##_CLK_MGR_REG_WR_ACCESS_OFFSET,		\
		.clkgate_offset 	=	pfx##_CLK_MGR_REG_##NAME1##_CLKGATE_OFFSET,	\
		.div_offset		=	pfx##_CLK_MGR_REG_##NAME1##_DIV_OFFSET,	\
		.div_trig_offset	=	pfx##_CLK_MGR_REG_##trigger##_OFFSET,		\
		.stprsts_mask		=	pfx##_CLK_MGR_REG_##NAME1##_CLKGATE_##NAME2##_STPRSTS_MASK,		\
		.hw_sw_gating_mask	=	pfx##_CLK_MGR_REG_##NAME1##_CLKGATE_##NAME2##_HW_SW_GATING_SEL_MASK,	\
		.clk_en_mask		=	pfx##_CLK_MGR_REG_##NAME1##_CLKGATE_##NAME2##_CLK_EN_MASK,		\
		.pll_select_mask	=	pfx##_CLK_MGR_REG_##NAME1##_DIV_##NAME2##_PLL_SELECT_MASK,	\
		.pll_select_shift	=	pfx##_CLK_MGR_REG_##NAME1##_DIV_##NAME2##_PLL_SELECT_SHIFT,	\
		.trigger_mask		=	pfx##_CLK_MGR_REG_##trigger##_##NAME2##_TRIGGER_MASK,	\
	}

/* BCM2165x clock Ids */
enum {
    BCM2165x_CLK_ARM = 1,
    BCM2165x_CLK_ARM_PERIPH,
    BCM2165x_CLK_CRYSTAL,
    BCM2165x_CLK_DUMMY,
    BCM2165x_CLK_FRAC_1M,
    BCM2165x_CLK_REF_96M_VARVDD,
    BCM2165x_CLK_VAR_96M,
    BCM2165x_CLK_REF_96M,
    BCM2165x_CLK_VAR_500M,
    BCM2165x_CLK_VAR_500M_VARVDD,
    BCM2165x_CLK_REF_1M,
    BCM2165x_CLK_REF_32K,
    BCM2165x_CLK_MISC_32K,
    BCM2165x_CLK_BBL_32K,
    BCM2165x_CLK_DFT_19_5M,
    BCM2165x_CLK_REF_312M,
    BCM2165x_CLK_REF_208M,
    BCM2165x_CLK_REF_156M,
    BCM2165x_CLK_REF_104M,
    BCM2165x_CLK_REF_52M,
    BCM2165x_CLK_REF_13M,
    BCM2165x_CLK_REF_26M,
    BCM2165x_CLK_REF_2P4M,
    BCM2165x_CLK_VAR_312M,
    BCM2165x_CLK_VAR_208M,
    BCM2165x_CLK_VAR_156M,
    BCM2165x_CLK_VAR_104M,
    BCM2165x_CLK_VAR_52M,
    BCM2165x_CLK_VAR_13M,
    BCM2165x_CLK_USBH_48M,
    BCM2165x_CLK_REF_CX40,
    BCM2165x_CLK_CSI0_PIX_PHY,
    BCM2165x_CLK_CSI0_BYTE0_PHY,
    BCM2165x_CLK_CSI0_BYTE1_PHY,
    BCM2165x_CLK_CSI1_PIX_PHY,
    BCM2165x_CLK_CSI1_BYTE0_PHY,
    BCM2165x_CLK_CSI1_BYTE1_PHY,
    BCM2165x_CLK_DSI0_PIX_PHY,
    BCM2165x_CLK_DSI1_PIX_PHY,
    BCM2165x_CLK_TEST_DEBUG,


    BCM2165x_CLK_DMAC,
    BCM2165x_CLK_NAND,
    BCM2165x_CLK_USB_OTG,
    BCM2165x_CLK_EHCI_12M,
    BCM2165x_CLK_EHCI_48M,
    BCM2165x_CLK_USB_IC,
    BCM2165x_CLK_SDIO1,
    BCM2165x_CLK_SDIO2,
    BCM2165x_CLK_SDIO3,
    BCM2165x_CLK_SDIO4,
    BCM2165x_CLK_PWM,
    BCM2165x_CLK_SSP0,
    BCM2165x_CLK_SSP0_AUDIO,
    BCM2165x_CLK_I2S_FRAC,
    BCM2165x_CLK_I2S,
    BCM2165x_CLK_BSC1,
    BCM2165x_CLK_BSC2,
    BCM2165x_CLK_UARTB,
    BCM2165x_CLK_UARTB2,
    BCM2165x_CLK_UARTB3,
    BCM2165x_CLK_UARTB4,
    BCM2165x_CLK_TIMERS,
    BCM2165x_CLK_SPUM_OPEN,
    BCM2165x_CLK_SPUM_SEC,
    BCM2165x_CLK_SMI,
    BCM2165x_CLK_AUDIOH_26M,

    BCM2165x_CLK_HUB_TIMER,
    BCM2165x_CLK_PMU_BSC,
    BCM2165x_CLK_SIM,
    BCM2165x_CLK_SIM2,
    BCM2165x_CLK_TMON_1M,
    BCM2165x_CLK_CAPH_SRCMIXER,
    BCM2165x_CLK_SSP3,
    BCM2165x_CLK_SSP4,
    BCM2165x_CLK_SSP3_AUDIO,
    BCM2165x_CLK_SSP4_AUDIO,

    BCM2165x_CLK_HUB,
/*MM CCU pereheral clock IDs*/
    BCM2165x_CLK_CSI0_LP,
    BCM2165x_CLK_CSI1_LP,
    BCM2165x_CLK_DSI0_ESC,
    BCM2165x_CLK_DSI1_ESC,
    BCM2165x_CLK_DSI_PLL,

    BCM2165x_CLK_END, /*End of functional clocks*/
    /*Bus clock IDs -- For internal use only */
    BCM2165x_CLK_UARTB4_APB,
    BCM2165x_CLK_UARTB3_APB,
    BCM2165x_CLK_UARTB2_APB,
    BCM2165x_CLK_UARTB_APB,
    BCM2165x_CLK_BSC2_APB,
    BCM2165x_CLK_BSC1_APB,
    BCM2165x_CLK_BBL_REG_APB,
    BCM2165x_CLK_SSP0_APB,
    BCM2165x_CLK_SPI_APB,
    BCM2165x_CLK_I2S_APB,
    BCM2165x_CLK_SDIO4_AHB,
    BCM2165x_CLK_SDIO3_AHB,
    BCM2165x_CLK_SDIO2_AHB,
    BCM2165x_CLK_SDIO1_AHB,
    BCM2165x_CLK_SDIO4_SLEEP,
    BCM2165x_CLK_SDIO3_SLEEP,
    BCM2165x_CLK_SDIO2_SLEEP,
    BCM2165x_CLK_SDIO1_SLEEP,
    BCM2165x_CLK_USB_IC_AHB,
    BCM2165x_CLK_USBH_12M,
    BCM2165x_CLK_USBH_AHB,
    BCM2165x_CLK_USB_OTG_AHB,
    BCM2165x_CLK_NAND_AHB,
    BCM2165x_CLK_APB2_REG,
    BCM2165x_CLK_HSM_APB,
    BCM2165x_CLK_HSM_AHB,
    BCM2165x_CLK_SPUM_OPEN_APB,
    BCM2165x_CLK_SPUM_SEC_APB,
    BCM2165x_CLK_APB1,
    BCM2165x_CLK_TIMERS_APB,
    BCM2165x_CLK_DMAC_MUX_APB,
    BCM2165x_CLK_PWM_APB,
    BCM2165x_CLK_CSI0_AXI,
    BCM2165x_CLK_CSI1_AXI,
    BCM2165x_CLK_DSI0_AXI,
    BCM2165x_CLK_DSI1_AXI,
    BCM2165x_CLK_SMI_AXI,
    BCM2165x_CLK_V3D_AXI,
    BCM2165x_CLK_VCE_AXI,
    BCM2165x_CLK_ISP_AXI,
    BCM2165x_CLK_MM_DMA_AXI,
    BCM2165x_CLK_MM_APB,
    BCM2165x_CLK_PWRMGR_AXI,
    BCM2165x_CLK_GPIOKP_APB,
    BCM2165x_CLK_HUB_TIMER_APB,
    BCM2165x_CLK_PMU_BSC_APB,
    BCM2165x_CLK_CHIPREG_APB,
    BCM2165x_CLK_FMON_APB,
    BCM2165x_CLK_HUB_TZCFG_APB,
    BCM2165x_CLK_SEC_WD_APB,
    BCM2165x_CLK_SYSEMI_SEC_APB,
    BCM2165x_CLK_SYSEMI_OPEN_APB,
    BCM2165x_CLK_VCEMI_SEC_APB,
    BCM2165x_CLK_VCEMI_OPEN_APB,
    BCM2165x_CLK_ACI_APB,
    BCM2165x_CLK_SIM_APB,
    BCM2165x_CLK_SPM_APB,
    BCM2165x_CLK_SIM2_APB,
    BCM2165x_CLK_DAP,

    /* HUB CCU clock Ids*/
    BCM2165x_CLK_DAP_SWITCH,
    BCM2165x_CLK_BROM,
    BCM2165x_CLK_NOR_APB,
    BCM2165x_CLK_NOR,
    BCM2165x_CLK_MDIOMASTER,
    BCM2165x_CLK_APB5,
    BCM2165x_CLK_CTI_APB,
    BCM2165x_CLK_FUNNEL_APB,
    BCM2165x_CLK_TPIU_APB,
    BCM2165x_CLK_VC_ITM_APB,
    BCM2165x_CLK_SEC_VIOL_TRAP_4_APB,
    BCM2165x_CLK_SEC_VIOL_TRAP_5_APB,
    BCM2165x_CLK_SEC_VIOL_TRAP_7_APB,
    BCM2165x_CLK_HSI_APB,
    BCM2165x_CLK_AXI_TRACE_19_APB,
    BCM2165x_CLK_AXI_TRACE_11_APB,
    BCM2165x_CLK_AXI_TRACE_12_APB,
    BCM2165x_CLK_AXI_TRACE_13_APB,
    BCM2165x_CLK_ETB_APB,
    BCM2165x_CLK_FINAL_FUNNEL_APB,
    BCM2165x_CLK_APB10,
    BCM2165x_CLK_APB9,
    BCM2165x_CLK_ATB_FILTER_APB,
    BCM2165x_CLK_BT_SLIM_AHB_APB,
    BCM2165x_CLK_ETB2AXI_APB,
    BCM2165x_CLK_SSP3_APB,
    BCM2165x_CLK_AUDIOH_APB,
    BCM2165x_CLK_AUDIOH_156M,
    BCM2165x_CLK_AUDIOH_2P4M,
    BCM2165x_CLK_TMON_APB,
    BCM2165x_CLK_VAR_SPM_APB,
    BCM2165x_CLK_SSP4_APB,

    BCM2165x_BUS_CLK_END, /*End of Bus clocks*/
};


#endif /*__ARM_ARCH_KONA_CLOCK_H*/
