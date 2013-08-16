/******************************************************************************
  Copyright 2010 Broadcom Corporation.  All rights reserved.

  Unless you and Broadcom execute a separate written software license agreement
  governing use of this software, this software is licensed to you under the
  terms of the GNU General Public License version 2, available at
http://www.gnu.org/copyleft/gpl.html (the "GPL").

Notwithstanding the above, under no circumstances may you combine this software
in any way with any other Broadcom software provided under a license other than
the GPL, without Broadcom's express prior written consent.
******************************************************************************/

#define pr_fmt(fmt) "vce: " fmt

#include <linux/kernel.h>
#include <mach/irqs.h>
#include <mach/clock.h>
#include <mach/rdb/brcm_rdb_sysmap.h>
#include <mach/rdb/brcm_rdb_h264.h>
#include <linux/broadcom/mm_fw_hw_ifc.h>

#include "h264_enc.h"

#define FLAG_CABAC 0x010000
#define FLUSH_COMPLETE

static void update_sg_regs(void *id, struct enc_info_t *enc_info)
{
	enc_info->h264_enc_regs.out_reg_cnt = 0;
	memset(enc_info->h264_enc_regs.out_regs, 0, \
		(sizeof(struct reg_info_t) * MAX_OUT_REGS));
	enc_info->h264_enc_regs.inp_reg_cnt = 0;
	enc_info->h264_enc_regs.fme_reg_cnt = 0;
	enc_info->h264_enc_regs.out_regs[enc_info->h264_enc_regs.out_reg_cnt].\
					reg = ENC_SG_BUF_ADDR;
	enc_info->h264_enc_regs.out_regs[enc_info->h264_enc_regs.out_reg_cnt].\
					val = h264_read(id, ENC_SG_BUF_ADDR);
	enc_info->h264_enc_regs.out_reg_cnt++;

	enc_info->h264_enc_regs.out_regs[enc_info->h264_enc_regs.out_reg_cnt].\
					reg = ENC_SG_BUF_START_ADDR;
	enc_info->h264_enc_regs.out_regs[enc_info->h264_enc_regs.out_reg_cnt].\
				val = h264_read(id, ENC_SG_BUF_START_ADDR);
	enc_info->h264_enc_regs.out_reg_cnt++;

	enc_info->h264_enc_regs.out_regs[enc_info->h264_enc_regs.out_reg_cnt].\
					reg = ENC_SG_BUF_END_ADDR;
	enc_info->h264_enc_regs.out_regs[enc_info->h264_enc_regs.out_reg_cnt].\
				val = h264_read(id, ENC_SG_BUF_END_ADDR);
	enc_info->h264_enc_regs.out_reg_cnt++;

	enc_info->h264_enc_regs.out_regs[enc_info->h264_enc_regs.out_reg_cnt].\
					reg = ENC_SG_BUF_BITS_WRITTEN;
	enc_info->h264_enc_regs.out_regs[enc_info->h264_enc_regs.out_reg_cnt].\
				val = h264_read(id, ENC_SG_BUF_BITS_WRITTEN);
	enc_info->h264_enc_regs.out_reg_cnt++;

}

static void writenbits(void *id, struct enc_info_t *enc_info)
{
	int i = 0;
	while (enc_info->numBits > 8) {
		h264_write(id, ENC_SG_POSTED_SYMBOL, enc_info->sg_buffer[i++]);
		h264_write(id, ENC_SG_POSTED_PUTSYM, 8);
		enc_info->numBits -= 8;
		}
	if (enc_info->numBits) {
		h264_write(id, ENC_SG_POSTED_SYMBOL, \
		(enc_info->sg_buffer[i++] >> (8 - enc_info->numBits)));
		h264_write(id, ENC_SG_POSTED_PUTSYM, enc_info->numBits);
		enc_info->numBits = 0;
		}
}

static int entropy_flush(void *id)
{
	int bits;
	h264_write(id, ENC_SG_POSTED_PUTSYM, 0x00008000);
	while (((h264_read(id, ENC_SG_STATUS)) & ((unsigned int)0x00000001)))
		continue;
	bits = h264_read(id, ENC_SG_BUF_BITS_WRITTEN);

	h264_write(id, ENC_SG_POSTED_SYMBOL, 0);
	h264_write(id, ENC_SG_POSTED_PUTSYM, 1);
	h264_write(id, ENC_SG_POSTED_PUTSYM, 0x00008000);
	return bits;
}


void encodeSlice(void *id, struct enc_info_t *enc_info)
{
	int i, j;
	for (i = 0; i < enc_info->h264_enc_regs.inp_reg_cnt - \
			enc_info->h264_enc_regs.fme_reg_cnt; i++) {
		if (enc_info->h264_enc_regs.inp_regs[i].reg == \
					ENC_SINT_VEC_REFPIC) {
			h264_write(id, ENC_SINT_VEC_REFPIC, \
				h264_read(id, ENC_SINT_VEC_REFPIC) | 1);
			h264_write(id, ENC_SINT_OPIC_MEM_BASE, 0);
			h264_write(id, ENC_SINT_VEC_REFPIC, \
				h264_read(id, ENC_SINT_VEC_REFPIC) & ~1);
		} else if (enc_info->h264_enc_regs.inp_regs[i].reg == \
				H264_DECSD_REGSDPARAM_OFFSET) {
			h264_write(id, H264_DECSD_REGSDPARAM_OFFSET, \
				(h264_read(id, H264_DECSD_REGSDPARAM_OFFSET) |\
					0x10000000));
		} else if (enc_info->h264_enc_regs.inp_regs[i].reg == \
					VC_CACHE_CTL) {
			h264_write(id, \
			enc_info->h264_enc_regs.inp_regs[i].reg, \
			enc_info->h264_enc_regs.inp_regs[i].val);

			while (h264_read(id, VCCACHESETUP) & 1)
				;
		} else if (enc_info->h264_enc_regs.inp_regs[i].reg == \
				ENC_SG_BUF_BITS_WRITTEN) {
			u32 sg_status = h264_read(id, ENC_SG_STATUS);
			h264_write(id, ENC_SG_STATUS, 0x00000000);
			h264_write(id, \
			enc_info->h264_enc_regs.inp_regs[i].reg, \
			enc_info->h264_enc_regs.inp_regs[i].val);
			h264_write(id, ENC_SG_STATUS, sg_status);
		}

		else {
			h264_write(id, \
			enc_info->h264_enc_regs.inp_regs[i].reg, \
			enc_info->h264_enc_regs.inp_regs[i].val);
		}
	}

	/* write slice header */
	writenbits(id, enc_info);

	if (enc_info->flags & FLAG_CABAC)
		entropy_flush(id);

	enc_info->startAddr = h264_read(id, ENC_SG_BUF_ADDR);
	enc_info->startBits = h264_read(id, \
				ENC_SG_BUF_BITS_WRITTEN);

	j = enc_info->h264_enc_regs.inp_reg_cnt - \
		enc_info->h264_enc_regs.fme_reg_cnt;
	for (i = j; i < enc_info->h264_enc_regs.inp_reg_cnt; i++) {
		if (enc_info->h264_enc_regs.inp_regs[i].reg == \
			H264_REG_FRAMESIZE_OFFSET) {
			h264_write(id, H264_REG_FRAMESIZE_OFFSET, \
			h264_read(id, H264_ENCICL_ENCIFRAMECTL_OFFSET));
		} else {
			h264_write(id, \
			enc_info->h264_enc_regs.inp_regs[i].reg, \
			enc_info->h264_enc_regs.inp_regs[i].val);
		}
	}
}

void completeEncodeSlice(void *id, struct enc_info_t *enc_info)
{
	if (!(enc_info->flags & FLAG_CABAC)) {
		h264_write(id, ENC_SG_POSTED_SYMBOL, 1);
		h264_write(id, ENC_SG_POSTED_PUTSYM, 1);
	}
#ifdef FLUSH_COMPLETE
	h264_write(id, ENC_SG_POSTED_SYMBOL, 0);
	h264_write(id, ENC_SG_POSTED_PUTSYM, 32);
	h264_write(id, ENC_SG_POSTED_SYMBOL, 0);
	h264_write(id, ENC_SG_POSTED_PUTSYM, 32);
	h264_write(id, ENC_SG_POSTED_SYMBOL, 0);
	h264_write(id, ENC_SG_POSTED_PUTSYM, 32);
	h264_write(id, ENC_SG_POSTED_SYMBOL, 0);
	h264_write(id, ENC_SG_POSTED_PUTSYM, 32);
	h264_write(id, ENC_SG_POSTED_SYMBOL, 0);
	h264_write(id, ENC_SG_POSTED_PUTSYM, 32);
#endif
	enc_info->endBits = entropy_flush(id);

	h264_write(id, VC_CACHE_CTL, h264_read(id, VC_CACHE_CTL) | 1);
	update_sg_regs(id, enc_info);
}
