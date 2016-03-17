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
#include <linux/string.h>
#include "hva.h"
#include "hva_io.h"

#define FLAG_CABAC		0x010000
#define BITS_AT_A_TIME		32
#define NR_BITS_TO_FLUSH	(BITS_AT_A_TIME * 5)
#define FLUSH_TRIES		500

#define SG_FLUSH_VAL		0
#define SG_FLUSH_NBITS		1
#define SG_RBSP_STOP_VAL	1
#define SG_RBSP_STOP_NBITS	1

#define ENC_REG_ADDR(reg) HVA_REG_ADDR(ENCSN_ENCSE2BIN ## reg)
#define ENC_REG_WT(hva, reg, value) \
	HVA_REG_WT(hva, ENCSN_ENCSE2BIN ## reg, value)
#define ENC_REG_RD(hva, reg) HVA_REG_RD(hva, ENCSN_ENCSE2BIN ## reg)
#define ENC_REG_FIELD_GET(hva, reg, field) \
	HVA_REG_FIELD_GET(hva, ENCSN_ENCSE2BIN ## reg, field)
#define ENC_REG_FIELD_SET(hva, reg, field, val) \
	HVA_REG_FIELD_SET(hva, ENCSN_ENCSE2BIN ## reg, field, val)
#define ENC_REG_FIELD_SET_ENUM(hva, reg, field, name) \
	ENC_REG_FIELD_SET(hva, reg, field, \
			  HVA_FIELD_ENUM(ENCSN_ENCSE2BIN ## reg, field, name))

static inline void read_reg(struct hva *hva, struct hva_enc_reg *reg_info,
			    u32 reg)
{
	reg_info->reg = reg;
	reg_info->val = hva_readl(hva, reg);
}

static inline void put_symbol(struct hva *hva, u32 sym_type, u32 sym_value)
{
	ENC_REG_WT(hva, POSTEDSYMBOL, sym_value);
	ENC_REG_WT(hva, POSTEDPUTSYM, sym_type);
}

static void update_sg_regs(struct hva *hva, struct hva_enc_info *enc)
{
	struct hva_enc_regset *regs = &enc->regset;
	struct hva_enc_reg *out_regs = regs->out_regs;

	regs->out_reg_cnt = 0;
	memset(regs->out_regs, 0, sizeof(regs->out_regs));
	regs->inp_reg_cnt = 0;
	regs->fme_reg_cnt = 0;
	read_reg(hva, &out_regs[regs->out_reg_cnt++], ENC_REG_ADDR(BUFADDR));
	read_reg(hva, &out_regs[regs->out_reg_cnt++],
		 ENC_REG_ADDR(BUFSTARTADDR));
	read_reg(hva, &out_regs[regs->out_reg_cnt++],
		 ENC_REG_ADDR(BUFENDADDR));
	read_reg(hva, &out_regs[regs->out_reg_cnt++],
		 ENC_REG_ADDR(BUFBITSWRITTEN));
	read_reg(hva, &out_regs[regs->out_reg_cnt++],
		 ENC_REG_ADDR(BUFMARKADDR));
}

static void write_n_bits(struct hva *hva, struct hva_enc_info *enc)
{
	int i = 0;

	while (enc->num_bits > 8) {
		put_symbol(hva, 8, enc->sg_buffer[i++]);
		enc->num_bits -= 8;
	}

	if (enc->num_bits) {
		put_symbol(hva, enc->num_bits,
			   enc->sg_buffer[i++] >> (8 - enc->num_bits));
		enc->num_bits = 0;
	}
}

static int entropy_flush(struct hva *hva)
{
	int bits;
	int i;

	ENC_REG_FIELD_SET_ENUM(hva, POSTEDPUTSYM, TYPE, FLUSH);

	for (i = 0; i < FLUSH_TRIES; i++)
		if (!ENC_REG_FIELD_GET(hva, STATUS, BUFF_CLOSE))
			break;

	bits = ENC_REG_RD(hva, BUFBITSWRITTEN);
	put_symbol(hva, SG_FLUSH_NBITS, SG_FLUSH_VAL);
	ENC_REG_FIELD_SET_ENUM(hva, POSTEDPUTSYM, TYPE, FLUSH);
	return bits;
}

void hva_enc_slice(struct hva *hva, struct hva_enc_info *enc)
{
	struct hva_enc_regset *enc_regs = &enc->regset;
	unsigned int i;
	struct hva_enc_reg *in_reg;
	unsigned fme_reg_cnt;
	u32 val;
	int j;

	fme_reg_cnt = min(enc_regs->inp_reg_cnt, enc_regs->fme_reg_cnt);

	for (i = 0; i < enc_regs->inp_reg_cnt - fme_reg_cnt; i++) {
		in_reg = &enc_regs->inp_regs[i];

		switch (in_reg->reg) {
		case HVA_REG_ENC_SINT_VEC_REFPIC_OFFSET:
			HVA_REG_WT(hva, REG_ENC_SINT_VEC_REFPIC,
				   HVA_REG_RD(hva, REG_ENC_SINT_VEC_REFPIC) |
				   HVA_FIELD_MASK(REG_ENC_SINT_VEC_REFPIC,
						  RAMSEL));
			HVA_REG_WT(hva, REG_ENC_SINT_OPIC_MEM_BASE, 0);
			HVA_REG_WT(hva, REG_ENC_SINT_VEC_REFPIC,
				   HVA_REG_RD(hva, REG_ENC_SINT_VEC_REFPIC) &
				   ~HVA_FIELD_MASK(REG_ENC_SINT_VEC_REFPIC,
						   RAMSEL));
			break;

		case HVA_DECSD_REGSDPARAM_OFFSET:
			val = HVA_REG_RD(hva, DECSD_REGSDPARAM);
			HVA_REG_WT(hva, DECSD_REGSDPARAM,
				   val | HVA_FIELD_MASK(DECSD_REGSDPARAM,
							USECHRHT));
			break;

		case HVA_VCCACHECTL_OFFSET:
			HVA_REG_WT(hva, VCCACHECTL, in_reg->val);

			for (j = 0; j < FLUSH_TRIES; j++)
				if (!(HVA_REG_FIELD_TEST(hva, VCCACHESETUP,
							 FLUSHING)))
					break;

			break;

		case HVA_ENCSN_ENCSE2BINBUFBITSWRITTEN_OFFSET:
			val = ENC_REG_RD(hva, STATUS);
			ENC_REG_WT(hva, STATUS, 0);
			ENC_REG_WT(hva, BUFBITSWRITTEN, in_reg->val);
			ENC_REG_WT(hva, STATUS, val);
			break;

		default:
			hva_writel(hva, in_reg->val, in_reg->reg);
			break;
		}
	}

	/* write slice header */
	write_n_bits(hva, enc);

	if (enc->flags & FLAG_CABAC)
		entropy_flush(hva);

	enc->start_addr = ENC_REG_RD(hva, BUFADDR);
	enc->start_bits = ENC_REG_RD(hva, BUFBITSWRITTEN);

	for (i = enc_regs->inp_reg_cnt - fme_reg_cnt; i < enc_regs->inp_reg_cnt;
	     i++) {
		in_reg = &enc_regs->inp_regs[i];

		if (in_reg->reg == HVA_REG_FRAMESIZE_OFFSET)
			HVA_REG_WT(hva, REG_FRAMESIZE,
				   HVA_REG_RD(hva, ENCICL_ENCIFRAMECTL));
		else
			hva_writel(hva, in_reg->val, in_reg->reg);
	}
}

void hva_enc_complete_slice(struct hva *hva, struct hva_enc_info *enc)
{
	int i;

	if (!(enc->flags & FLAG_CABAC))
		put_symbol(hva, SG_RBSP_STOP_NBITS, SG_RBSP_STOP_VAL);

	/*
	 * Write some arbitrary amount of zeroes to the stream.
	 * TODO: Shouldn't be necessary; needs revisiting.
	 *	 Even if is necessary, should probably be max 16 bits at a time.
	 */
	for (i = 0; i < NR_BITS_TO_FLUSH / BITS_AT_A_TIME; i++)
		put_symbol(hva, BITS_AT_A_TIME, 0);

	enc->end_bits = entropy_flush(hva);

	HVA_REG_WT(hva, VCCACHECTL, HVA_REG_RD(hva, VCCACHECTL) |
				    HVA_FIELD_MASK(VCCACHECTL, FLUSH));
	update_sg_regs(hva, enc);
}
