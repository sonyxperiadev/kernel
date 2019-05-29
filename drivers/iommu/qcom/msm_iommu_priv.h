/*
 * Copyright (C) 2017-2018, AngeloGioacchino Del Regno <kholk11@gmail.com>
 *
 * May contain portions of code (c) 2013-2014, The Linux Foundation.
 *
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

#ifndef MSM_IOMMU_PRIV_H
#define MSM_IOMMU_PRIV_H

/* commands for SCM_SVC_MP */
#define IOMMU_SECURE_CFG	2
#define IOMMU_SECURE_PTBL_SIZE  3
#define IOMMU_SECURE_PTBL_INIT  4
#define IOMMU_SET_CP_POOL_SIZE	5

/* commands for SCM_SVC_UTIL */
#define IOMMU_DUMP_SMMU_FAULT_REGS 0X0C

/* Other SEC definitions */
#define MAXIMUM_VIRT_SIZE	(300 * SZ_1M)
#define MAKE_VERSION(major, minor, patch) \
	(((major & 0x3FF) << 22) | ((minor & 0x3FF) << 12) | (patch & 0xFFF))

/* Maximum number of SMT entries allowed by the system */
#define MAX_NUM_SMR	128

/* Maximum number of BFB registers */
#define MAX_NUM_BFB_REGS	32

/* Register dump */
#define NUM_DUMP_REGS 14

/* Each entry is a (reg_addr, reg_val) pair, plus some wiggle room */
#define SEC_DUMP_SIZE (NUM_DUMP_REGS * 4)
#define COMBINE_DUMP_REG(upper, lower) (((u64) upper << 32) | lower)

#define DUMP_REG_INIT(dump_reg, cb_reg, mbp, drt)		\
	do {							\
		dump_regs_tbl[dump_reg].reg_offset = cb_reg;	\
		dump_regs_tbl[dump_reg].name = #cb_reg;		\
		dump_regs_tbl[dump_reg].must_be_present = mbp;	\
		dump_regs_tbl[dump_reg].dump_reg_type = drt;	\
	} while (0)

enum dump_reg {
	DUMP_REG_FIRST,
	DUMP_REG_FAR0 = DUMP_REG_FIRST,
	DUMP_REG_FAR1,
	DUMP_REG_PAR0,
	DUMP_REG_PAR1,
	DUMP_REG_FSR,
	DUMP_REG_FSYNR0,
	DUMP_REG_FSYNR1,
	DUMP_REG_TTBR0_0,
	DUMP_REG_TTBR0_1,
	DUMP_REG_TTBR1_0,
	DUMP_REG_TTBR1_1,
	DUMP_REG_SCTLR,
	DUMP_REG_ACTLR,
	DUMP_REG_PRRR,
	DUMP_REG_MAIR0 = DUMP_REG_PRRR,
	DUMP_REG_NMRR,
	DUMP_REG_MAIR1 = DUMP_REG_NMRR,
	DUMP_REG_CBAR_N,
	DUMP_REG_CBFRSYNRA_N,
	MAX_DUMP_REGS,
};

enum dump_reg_type {
	DRT_CTX_REG,
	DRT_GLOBAL_REG,
	DRT_GLOBAL_REG_N,
};

enum model_id {
	QSMMUv1 = 1,
	QSMMUv2,
	MMU_500 = 500,
	MAX_MODEL,
};

struct msm_iommu_context_reg {
	uint32_t val;
	bool valid;
};

struct dump_regs_tbl_entry {
	/*
	 * To keep things context-bank-agnostic, we only store the
	 * register offset in `reg_offset'
	 */
	unsigned int reg_offset;
	const char *name;
	int must_be_present;
	enum dump_reg_type dump_reg_type;
};
extern struct dump_regs_tbl_entry dump_regs_tbl[MAX_DUMP_REGS];

struct msm_scm_fault_regs_dump {
	uint32_t dump_size;
	uint32_t dump_data[SEC_DUMP_SIZE];
} __aligned(PAGE_SIZE);

/**
 * struct msm_iommu_priv - Container for page table attributes and other
 * private iommu domain information.
 * attributes.
 * pt: Page table attribute structure
 * list_attached: List of devices (contexts) attached to this domain.
 * client_name: Name of the domain client.
 */
struct msm_iommu_priv {
	struct list_head list_attached;
	struct iommu_domain domain;
	const char *client_name;
	struct io_pgtable_cfg pgtbl_cfg;
	struct io_pgtable_ops *pgtbl_ops;
	spinlock_t pgtbl_lock;
	struct mutex init_mutex;
	u32 procid;
	u32 asid;
	u32 attributes;
	struct iommu_domain *base;
};

static inline struct msm_iommu_priv *to_msm_priv(struct iommu_domain *dom)
{
	return container_of(dom, struct msm_iommu_priv, domain);
}

/**
 * struct msm_iommu_bfb_settings - a set of IOMMU BFB tuning parameters
 * regs		An array of register offsets to configure
 * data		Values to write to corresponding registers
 * length	Number of valid entries in the offset/val arrays
 */
struct msm_iommu_bfb_settings {
	unsigned int regs[MAX_NUM_BFB_REGS];
	unsigned int data[MAX_NUM_BFB_REGS];
	int length;
};

/**
 * struct msm_iommu_drvdata - A single IOMMU hardware instance
 * @base:	IOMMU config port base address (VA)
 * @glb_base:	IOMMU config port base address for global register space (VA)
 * @phys_base:  IOMMU physical base address.
 * @ncb		The number of contexts on this IOMMU
 * @irq:	Interrupt number
 * @core:	The bus clock for this IOMMU hardware instance
 * @iface:	The clock for the IOMMU bus interconnect
 * @name:	Human-readable name of this IOMMU device
 * @bfb_settings: Optional BFB performance tuning parameters
 * @dev:	Struct device this hardware instance is tied to
 * @list:	List head to link all iommus together
 * @halt_enabled: Set to 1 if IOMMU halt is supported in the IOMMU, 0 otherwise.
 * @ctx_attach_count: Count of how many context are attached.
 * @bus_client  : Bus client needed to vote for bus bandwidth.
 *
 * A msm_iommu_drvdata holds the global driver data about a single piece
 * of an IOMMU hardware instance.
 */
struct msm_iommu_drvdata {
	void __iomem *base;
	phys_addr_t phys_base;
	void __iomem *glb_base;
	void __iomem *cb_base;
	void __iomem *smmu_local_base;
	int ncb;
	struct clk *core;
	struct clk *iface;
	const char *name;
	struct msm_iommu_bfb_settings *bfb_settings;
	int sec_id;
	struct device *dev;
	struct list_head list;
	int halt_enabled;
	unsigned int ctx_attach_count;
	unsigned int bus_client;
	unsigned int model;
	struct iommu_device iommu;
	struct idr asid_idr;
	struct list_head masters;
};

/**
 * struct msm_iommu_ctx_drvdata - an IOMMU context bank instance
 * @num:		Hardware context number of this context
 * @pdev:		Platform device associated wit this HW instance
 * @attached_elm:	List element for domains to track which devices are
 *			attached to them
 * @attached_domain	Domain currently attached to this context (if any)
 * @name		Human-readable name of this context device
 * @sids		List of Stream IDs mapped to this context
 * @nsid		Number of Stream IDs mapped to this context
 * @secure_context	true if this is a secure context programmed by
			the secure environment, false otherwise
 * @asid		ASID used with this context.
 * @attach_count	Number of time this context has been attached.
 * @dynamic		true if any dynamic domain is ever attached to this CB
 *
 * A msm_iommu_ctx_drvdata holds the driver data for a single context bank
 * within each IOMMU hardware instance
 */
struct msm_iommu_ctx_drvdata {
	int num;
	struct platform_device *pdev;
	struct list_head attached_elm;
	struct iommu_domain *attached_domain;
	const char *name;
	u32 sids[MAX_NUM_SMR];
	unsigned int nsid;
	bool secure_context;
	int asid;
	int attach_count;
	u32 sid_mask[MAX_NUM_SMR];
	unsigned int n_sid_mask;
	bool dynamic;
	bool needs_secure_map;
};

/**
 * struct iommu_access_ops - Callbacks for accessing IOMMU
 * @iommu_bus_vote:     Vote for bus bandwidth
 * @iommu_clk_on:       Enable IOMMU clocks
 * @iommu_clk_off:      Disable IOMMU clocks
 * @iommu_lock_acquire: Acquire any locks needed
 * @iommu_lock_release: Release locks needed
 */
struct iommu_access_ops {
	int (*iommu_bus_vote)(struct msm_iommu_drvdata *drvdata,
			      unsigned int vote);
	int  (*iommu_clk_on)(struct msm_iommu_drvdata *);
	void (*iommu_clk_off)(struct msm_iommu_drvdata *);
	void (*iommu_lock_acquire)(unsigned int need_extra_lock);
	void (*iommu_lock_release)(unsigned int need_extra_lock);
};
void msm_set_iommu_access_ops(struct iommu_access_ops *ops);
struct iommu_access_ops *msm_get_iommu_access_ops(void);

int __enable_clocks(struct msm_iommu_drvdata *drvdata);
void __disable_clocks(struct msm_iommu_drvdata *drvdata);
void iommu_halt(const struct msm_iommu_drvdata *iommu_drvdata);
void iommu_resume(const struct msm_iommu_drvdata *iommu_drvdata);

int msm_iommu_init(struct msm_iommu_drvdata *drvdata);

void print_ctx_regs(struct msm_iommu_context_reg regs[]);

/*
 * Interrupt handler for the IOMMU context fault interrupt. Hooking the
 * interrupt is not supported in the API yet, but this will print an error
 * message and dump useful IOMMU registers.
 */
irqreturn_t msm_iommu_global_fault_handler(int irq, void *dev_id);
irqreturn_t msm_iommu_fault_handler(int irq, void *dev_id);
irqreturn_t msm_iommu_fault_handler_v2(int irq, void *dev_id);
irqreturn_t msm_iommu_secure_fault_handler_v2(int irq, void *dev_id);

#endif
