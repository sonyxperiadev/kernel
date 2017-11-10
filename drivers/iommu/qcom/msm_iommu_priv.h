/* Copyright (c) 2013-2014, The Linux Foundation. All rights reserved.
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
#define IOMMU_SECURE_MAP	6
#define IOMMU_SECURE_UNMAP      7
#define IOMMU_SECURE_MAP2 0x0B
#define IOMMU_SECURE_MAP2_FLAT 0x12
#define IOMMU_SECURE_UNMAP2 0x0C
#define IOMMU_SECURE_UNMAP2_FLAT 0x13
#define IOMMU_TLBINVAL_FLAG 0x00000001

/* commands for SCM_SVC_UTIL */
#define IOMMU_DUMP_SMMU_FAULT_REGS 0X0C

/* Other SEC definitions */
#define MAXIMUM_VIRT_SIZE	(300 * SZ_1M)
#define MAKE_VERSION(major, minor, patch) \
	(((major & 0x3FF) << 22) | ((minor & 0x3FF) << 12) | (patch & 0xFFF))

/* Register dump */
#define NUM_DUMP_REGS 14

/* Each entry is a (reg_addr, reg_val) pair, plus some wiggle room */
#define SEC_DUMP_SIZE (NUM_DUMP_REGS * 4)
#define COMBINE_DUMP_REG(upper, lower) (((u64) upper << 32) | lower)

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

struct msm_scm_paddr_list {
	phys_addr_t list;
	unsigned int list_size;
	unsigned int size;
};

struct msm_scm_mapping_info {
	unsigned int id;
	unsigned int ctx_id;
	unsigned int va;
	unsigned int size;
};

struct msm_scm_map2_req {
	struct msm_scm_paddr_list plist;
	struct msm_scm_mapping_info info;
	unsigned int flags;
};

struct msm_scm_unmap2_req {
	struct msm_scm_mapping_info info;
	unsigned int flags;
};

struct msm_cp_pool_size {
	uint32_t size;
	uint32_t spare;
};

struct msm_scm_fault_regs_dump {
	uint32_t dump_size;
	uint32_t dump_data[SEC_DUMP_SIZE];
} __aligned(PAGE_SIZE);

/**
 * struct msm_iommu_pt - Container for first level page table and its
 * attributes.
 * fl_table: Pointer to the first level page table.
 * redirect: Set to 1 if L2 redirect for page tables are enabled, 0 otherwise.
 * unaligned_fl_table: Original address of memory for the page table.
 * fl_table is manually aligned (as per spec) but we need the original address
 * to free the table.
 * fl_table_shadow: This is "copy" of the fl_table with some differences.
 * It stores the same information as fl_table except that instead of storing
 * second level page table address + page table entry descriptor bits it
 * stores the second level page table address and the number of used second
 * level page tables entries. This is used to check whether we need to free
 * the second level page table which allows us to also free the second level
 * page table after doing a TLB invalidate which should catch bugs with
 * clients trying to unmap an address that is being used.
 * fl_table_shadow will use the lower 9 bits for the use count and the upper
 * bits for the second level page table address.
 * sl_table_shadow uses the same concept as fl_table_shadow but for LPAE 2nd
 * level page tables.
 */
#ifdef CONFIG_IOMMU_LPAE
struct msm_iommu_pt {
	u64 *fl_table;
	u64 **sl_table_shadow;
	int redirect;
	u64 *unaligned_fl_table;
};
#else
struct msm_iommu_pt {
	u32 *fl_table;
	int redirect;
	u32 *fl_table_shadow;
};
#endif
/**
 * struct msm_iommu_priv - Container for page table attributes and other
 * private iommu domain information.
 * attributes.
 * pt: Page table attribute structure
 * list_attached: List of devices (contexts) attached to this domain.
 * client_name: Name of the domain client.
 */
struct msm_iommu_priv {
	struct msm_iommu_pt pt;
	struct list_head list_attached;
	struct iommu_domain domain;
	const char *client_name;
	u32 procid;
	u32 asid;
	u32 attributes;
	struct iommu_domain *base;
};

static inline struct msm_iommu_priv *to_msm_priv(struct iommu_domain *dom)
{
	return container_of(dom, struct msm_iommu_priv, domain);
}

int msm_iommu_init(struct device *dev);

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
