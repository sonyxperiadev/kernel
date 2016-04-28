#ifndef BRCM_VCEPRIVATE_H
#define BRCM_VCEPRIVATE_H

/*******************************************************************************
Copyright 2012 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement
governing use of this software, this software is licensed to you under the
terms of the GNU General Public License version 2, available at
http://www.gnu.org/copyleft/gpl.html (the "GPL").

Notwithstanding the above, under no circumstances may you combine this software
in any way with any other Broadcom software provided under a license other than
the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

/* This file gives direct access to VCE -- use with caution -- there
 * is no protection.  Better to use one of the higher level
 * libraries */

#include <linux/io.h>
#include <linux/types.h>
#include <mach/rdb/brcm_rdb_vce.h>

struct vce;
struct vce_regmap {
	uint32_t __iomem *base;
	uint32_t __iomem *datamem;
	uint32_t __iomem *progmem;
	uint32_t __iomem *regfile;
};
extern void __iomem *vce_get_base_address(struct vce *vce);

#define VCE_ACQUIRER_NONE  0x40000000
#define VCE_ACQUIRER_DONTCARE   1
#define VCE_ACQUIRER_VTQ        2
/* #define VCE_ACQUIRER_IOCTL      3 */

extern int vce_acquire(struct vce *vce,
		uint32_t this_acquirer,
		uint32_t *was_preserved);
extern void vce_release(struct vce *vce);

#define vce_init_base(regmap, vce)					\
	do {								\
		(regmap)->base = vce_get_base_address(vce);		\
		(regmap)->datamem = (regmap)->base +			\
			(VCE_DMEM_ACCESS_OFFSET>>2);			\
		(regmap)->progmem = (regmap)->base +			\
			(VCE_PMEM_ACCESS_OFFSET>>2);			\
		(regmap)->regfile = (regmap)->base +			\
			(VCE_GPRF_ACCESS_OFFSET>>2);			\
	} while (0)

#define vce_readdata(regmap, offset) \
	readl((regmap)->datamem + ((offset)>>2))
#define vce_writedata(regmap, offset, data) \
	writel((data), \
		(regmap)->datamem + ((offset)>>2))
#define vce_writeprog(regmap, offset, data) \
	writel((data), \
		(regmap)->progmem + ((offset)>>2))
#define vce_writereg(regmap, regnum, value) \
	writel((value), \
		(regmap)->regfile + (regnum))
#define vce_readreg(regmap, regnum) \
	readl((regmap)->regfile + regnum)
#define vce_clearsema(regmap, semanum) \
	writel(1<<(semanum), \
		(regmap)->base + (VCE_SEMA_CLEAR_OFFSET>>2))
#define vce_setsema(regmap, semanum) \
	writel(1<<(semanum), \
		(regmap)->base + (VCE_SEMA_SET_OFFSET>>2))
#define vce_setpc(regmap, pc) \
	writel((pc), \
		(regmap)->base + (VCE_PC_PF0_OFFSET>>2))
#define vce_run(regmap) \
	writel((1<<VCE_CONTROL_RUN_BIT_CMD_SHIFT), \
		(regmap)->base + (VCE_CONTROL_OFFSET>>2))
#define vce_stop(regmap) \
	writel((0<<VCE_CONTROL_RUN_BIT_CMD_SHIFT), \
		(regmap)->base + (VCE_CONTROL_OFFSET>>2))
#define vce_status(regmap) \
	readl((regmap)->base + (VCE_STATUS_OFFSET>>2))

#endif
