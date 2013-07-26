/*******************************************************************************
  Copyright 2010 Broadcom Corporation.  All rights reserved.

  Unless you and Broadcom execute a separate written software license agreement
  governing use of this software, this software is licensed to you under the
  terms of the GNU General Public License version 2, available at
http://www.gnu.org/copyleft/gpl.html (the "GPL").

Notwithstanding the above, under no circumstances may you combine this software
in any way with any other Broadcom software provided under a license other than
the GPL, without Broadcom's express prior written consent.
 ******************************************************************************/
#ifndef _H264_DEC_H_
#define _H264_DEC_H_

/*Decoder internals start*/
#define REG_STATUS			0x00000110

#define STATUS_MOCOMP_RDY	(1 << 2)
#define STATUS_MOCOMP_DONE	(1 << 3)
#define STATUS_SPAT_RDY	(1 << 4)
#define STATUS_SPAT_DONE	(1 << 5)
#define STATUS_XFM_RDY	(1 << 6)
#define STATUS_XFM_DONE	(1 << 7)
#define STATUS_RECON_RDY	(1 << 8)
#define STATUS_RECON_DONE	(1 << 9)
#define STATUS_DBLK_RDY	(1 << 10)
#define STATUS_DBLK_DONE	(1 << 11)

/*Updated bits for new status register*/
#define STATUS_FLUSHCTX		(1<<17)
#define STATUS_RESET	(1<<16)
#define STATUS_CTXDMAACT	(1<<11)

#define STATUS_ALL_RDY (STATUS_MOCOMP_RDY | STATUS_SPAT_RDY | \
		STATUS_XFM_RDY    | STATUS_RECON_RDY | \
		STATUS_DBLK_RDY)

#define STATUS_ALL_DONE (STATUS_MOCOMP_DONE | STATUS_SPAT_DONE | \
		STATUS_XFM_DONE     | STATUS_RECON_DONE | \
		STATUS_DBLK_DONE)

#define STATUS_ALL_RDY_DONE (STATUS_ALL_DONE | STATUS_ALL_RDY)


#define NUM_DECODER_REGRW 1000

/*Decoder internals end*/
struct decoder_regs_t {
	unsigned int offset;
	unsigned int value;
};

struct dec_info_t {
	unsigned int num_dec_regs;
	struct decoder_regs_t dec_regs;
};

void decodeSlice(void *id, struct dec_info_t *dec_info);

void completeDecodeFrame(void *id);

void h264_write(void *id, u32 reg, u32 value);

u32 h264_read(void *id, u32 reg);

#endif /*_H264_DEC_H_*/
