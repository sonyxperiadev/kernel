/******************************************************************************
Copyright 2011 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement
governing use of this software, this software is licensed to you under the
terms of the GNU General Public License version 2, available at
http://www.gnu.org/copyleft/gpl.html (the "GPL").

Notwithstanding the above, under no circumstances may you combine this software
in any way with any other Broadcom software provided under a license other than
the GPL, without Broadcom's express prior written consent.
******************************************************************************/
#ifndef _H264_SYMGEN_H_
#define _H264_SYMGEN_H_

#define MAX_NUM_REGS 100
#define MAX_OUT_REGS 4
#define MAX_HEADER_LEN 256

/* reg offset and corresponding val */
struct reg_info_t {
	unsigned long reg;
	unsigned long val;
};

/* List of i/p and o/p reg-val pairs to and from driver */
struct VENC_REGS_T {
	struct reg_info_t inp_regs[MAX_NUM_REGS];
	struct reg_info_t out_regs[MAX_OUT_REGS];
	int inp_reg_cnt;
	int out_reg_cnt;
	int fme_reg_cnt;
};

/* encoder info */
struct enc_info_t {
	int	flags;				/* in-out */
	uint32_t startAddr;		/* output */
	uint32_t startBits;		/* output */
	uint32_t endBits;		/* output */
	char sg_buffer[MAX_HEADER_LEN];
	uint32_t numBits;
	struct VENC_REGS_T h264_enc_regs;/* in-out */
};

#define ENC_SG_BUF_BITS_WRITTEN 0x500e20
#define ENC_SG_POSTED_SYMBOL 0x500e34
#define ENC_SG_STATUS 0x500e44

#define ENC_SG_BUF_START_ADDR                  0x500e0c
#define ENC_SG_BUF_END_ADDR                    0x500e10
#define ENC_SG_BUF_ADDR                        0x500e08
#define ENC_SG_BUF_MARK_ADDR                   0x500e14
#define ENC_SG_POSTED_PUTSYM                   0x500e30
#define ENC_SINT_VEC_REFPIC                    0x008064
#define ENC_SINT_OPIC_MEM_BASE                 0x0080c0
#define VC_CACHE_CTL    (0x4408CC)
#define VCCACHESETUP    (0x4408D8)

void encodeSlice(void *id, struct enc_info_t *enc_info);
void completeEncodeSlice(void *id, struct enc_info_t *enc_info);
void h264_write(void *id, u32 reg, u32 value);
u32 h264_read(void *id, u32 reg);


#endif
