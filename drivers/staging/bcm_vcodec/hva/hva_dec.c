/*****************************************************************************
* Copyright 2013 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
*****************************************************************************/

#define pr_fmt(fmt) "hva: " fmt
#include <linux/kernel.h>
#include <linux/slab.h>
#include "hva.h"
#include "hva_io.h"

#define RESET_TRIES 10000
#define STATUS_READY_TRIES 10000

struct hva *hva_open(void __iomem *base, size_t size)
{
	struct hva *hva;

	if (size < HVA_SIZE) {
		pr_err("invalid size %zu for io memory", size);
		return NULL;
	}

	hva = kmalloc(sizeof(*hva), GFP_KERNEL);
	if (!hva) {
		pr_err("unable to allocate memory for hva instance");
		return NULL;
	}

	hva->base = base;
	hva->size = size;
	return hva;
}

void hva_close(struct hva *hva)
{
	kfree(hva);
}

void hva_set_vcintmask_vce(struct hva *hva, int value)
{
	u32 old_value = HVA_REG_RD(hva, VCINTMASK0);

	if (value)
		HVA_REG_WT(hva, VCINTMASK0, old_value | HVA_VCSIGNAL0_VCE_SET);
	else
		HVA_REG_WT(hva, VCINTMASK0, old_value & ~HVA_VCSIGNAL0_VCE_SET);
}

/*
 * Is the register operation 'reg' a read operation on the specified offset?
 */
static inline bool hva_dec_reg_match_rd(const struct hva_dec_reg *reg,
					u32 offset)
{
	return reg && HVA_DR_GET_OFFSET(reg) == offset && !HVA_DR_IS_WRITE(reg);
}

/*
 * hva_dec_reg_write() - write hva codec block register.
 * @hva:	Codec instance.
 * @reg:	Register + value to write.
 * @prev:	Previous register that was read/written, or NULL if at the
 *		start.
 * @next:	Next register value to be read/written, or NULL if at the end.
 *
 * Write codec register, plus perform any related tasks depending on previous
 * or next register operation, such as waiting for a register write to take
 * effect.
 */
static void hva_dec_reg_write(struct hva *hva, struct hva_dec_reg *reg,
			      struct hva_dec_reg *prev,
			      struct hva_dec_reg *next)
{
	u32 offset = HVA_DR_GET_OFFSET(reg);
	unsigned int j;
	unsigned int temp;

	switch (offset) {
	case HVA_DECSD_REGSDPARAM_OFFSET:
		if (reg->value & HVA_FIELD_MASK(DECSD_REGSDPARAM, USECHRHT))
			reg->value |= HVA_REG_RD(hva, DECSD_REGSDPARAM);

		HVA_REG_WT(hva, DECSD_REGSDPARAM, reg->value);
		break;

	case HVA_REG_MAINCTL_OFFSET:
		if (reg->value & HVA_FIELD_MASK(REG_MAINCTL, RST)) {
			if ((reg->value & ~HVA_FIELD_MASK(REG_MAINCTL, RST))
			    == 0) {
				reg->value = HVA_REG_RD(hva, REG_MAINCTL);
				reg->value |= HVA_FIELD_MASK(REG_MAINCTL, RST);
			}

			HVA_REG_WT(hva, REG_MAINCTL, reg->value);

			/* If client wants to wait for the reset, then do so */
			if (hva_dec_reg_match_rd(next, offset))
				for (j = 0; j < RESET_TRIES; j++)
					if (!HVA_REG_FIELD_TEST(hva,
								REG_MAINCTL,
								RST))
						break;
		} else {
			HVA_REG_WT(hva, REG_MAINCTL, reg->value);
		}
		break;

	case HVA_REG_SINT_STRM_STAT_OFFSET:
		HVA_REG_WT(hva, REG_SINT_STRM_STAT, reg->value);

		if (reg->value == HVA_FIELD_MASK(REG_SINT_STRM_STAT, FLUSHCTX)
		    && hva_dec_reg_match_rd(next, offset))
			for (j = 0; j < RESET_TRIES; j++)
				if (!HVA_REG_FIELD_TEST(hva, REG_SINT_STRM_STAT,
							CTXDMAACT))
					break;
		else if (reg->value & HVA_FIELD_MASK(REG_SINT_STRM_STAT, RST)
			 && hva_dec_reg_match_rd(next, offset))
			/* Client wants to wait for the reset */
			for (j = 0; j < RESET_TRIES; j++)
				if (!HVA_REG_FIELD_GET(hva, REG_SINT_STRM_STAT,
						       RST))
					break;

		break;

	case HVA_VCCACHECTL_OFFSET:
		if (hva_dec_reg_match_rd(prev, offset))
			reg->value |= HVA_REG_RD(hva, VCCACHECTL);

		HVA_REG_WT(hva, VCCACHECTL, reg->value);
		break;

	case HVA_REG_SINT_VEC_REFPIC_OFFSET:
		if (hva_dec_reg_match_rd(prev, offset)) {
			temp = HVA_REG_RD(hva, REG_SINT_VEC_REFPIC);
			reg->value = (temp & ~1) | (reg->value & 1);
		}

		HVA_REG_WT(hva, REG_SINT_VEC_REFPIC, reg->value);
		break;

	default:
		hva_writel(hva, reg->value, offset);
		break;
	}
}

static void hva_dec_reset_hw(struct hva *hva)
{
	unsigned int j;

	HVA_REG_WT(hva, REG_MAINCTL, HVA_REG_RD(hva, REG_MAINCTL) |
				     HVA_FIELD_MASK(REG_MAINCTL, RST));

	for (j = 0; j < RESET_TRIES; j++)
		if (!HVA_REG_FIELD_TEST(hva, REG_MAINCTL, RST))
			break;
}

static void hva_dec_reg_read(struct hva *hva, struct hva_dec_reg *reg)
{
	u32 offset = HVA_DR_GET_OFFSET(reg);
	unsigned int j;

	if (offset == HVA_REG_STATUS_OFFSET) {
		/* Wait for the backend to be idle */
		for (j = 0; j < STATUS_READY_TRIES; j++)
			if ((HVA_REG_RD(hva, REG_STATUS) &
			     HVA_REG_STATUS_ALL_RDY_DONE)
			    == HVA_REG_STATUS_ALL_RDY_DONE)
				break;

		/* Reset the backend if have waited too long */
		if (j == STATUS_READY_TRIES)
			hva_dec_reset_hw(hva);
	}

	reg->value = hva_readl(hva, offset);
}

/*
 * hva_dec_slice() - perform HVA operations for the start of a slice decode.
 * @hva:	Codec instance.
 * @dec:	Registers + values to read/write.
 *
 * Read and write various codec registers, plus perform various related tasks,
 * such as waiting for register values to change after a write.
 */
void hva_dec_slice(struct hva *hva, struct hva_dec_info *dec)
{
	struct hva_dec_reg *first = dec->regs;
	struct hva_dec_reg *last = dec->regs + dec->nr_regs - 1;
	struct hva_dec_reg *reg;

	for (reg = first; reg <= last; reg++)
		if (HVA_DR_IS_WRITE(reg))
			hva_dec_reg_write(hva, reg,
					  reg == first ? NULL : reg - 1,
					  reg == last ? NULL : reg + 1);
		else
			hva_dec_reg_read(hva, reg);
}
