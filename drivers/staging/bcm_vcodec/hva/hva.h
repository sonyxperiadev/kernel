/*****************************************************************************
* Copyright 2013 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
*****************************************************************************/

/*
 * Driver for Broadcom's Hardware Video Accelerator.
 */

#ifndef _HVA_H_
#define _HVA_H_

#include <linux/types.h>

#define HVA_ENC_NR_IN_REGS	100
#define HVA_ENC_NR_OUT_REGS	5
#define HVA_ENC_HEADER_LEN	256

#define HVA_DR_WRITE_MASK	0x80000000
#define HVA_DR_OFFSET_MASK	0x00ffffff
#define HVA_DR_IS_WRITE(dr)	((dr)->offset & HVA_DR_WRITE_MASK)
#define HVA_DR_GET_OFFSET(dr)	((dr)->offset & HVA_DR_OFFSET_MASK)
#define HVA_DR_SET_OFFSET(dr, off, wr) \
	((dr)->offset = ((off) & HVA_DR_OFFSET_MASK) | \
			((wr) ? HVA_DR_WRITE_MASK : 0))

/*
 * Definition of a decode operation.
 */
struct hva_dec_reg {
	__u32 offset;	/*
			 * Use HVA_DR_ macros to get/set/test read/write flag
			 * and offset.
			 */
	__u32 value;
};

/*
 * Collection of decode operations on an HVA block.
 */
struct hva_dec_info {
	unsigned int nr_regs;
	struct hva_dec_reg regs[1];	/* variable length array */
};

/*
 * Definition of an encode operation.
 */
struct hva_enc_reg {
	__u32 reg;
	__u32 val;
};


/*
 * Collection of encode operations.
 */
struct hva_enc_regset {
	struct hva_enc_reg inp_regs[HVA_ENC_NR_IN_REGS];
	struct hva_enc_reg out_regs[HVA_ENC_NR_OUT_REGS];
	unsigned int inp_reg_cnt;
	unsigned int out_reg_cnt;
	unsigned int fme_reg_cnt;
};

/*
 * Encoder operations on an HVA block.
 */
struct hva_enc_info {
	int flags;			/* inout */
	__u32 start_addr;		/* out */
	__u32 start_bits;		/* out */
	__u32 end_bits;			/* out */
	__u8 sg_buffer[HVA_ENC_HEADER_LEN];
	__u32 num_bits;
	struct hva_enc_regset regset;	/* inout */
};

#ifdef __KERNEL__

/* Instance of HVA driver */
struct hva;

/*
 * Create handle to HVA driver using already mapped memory.
 * The io-mapped memory region corresponds to a single instance of a hardware
 * codec block. A codec block should only be accessed by a single instance of
 * the HVA driver at once; this is left to the caller to manage, by reserving
 * the io-mapped region as normal.
 *
 * There is no internal locking in the hva_ functions to prevent simultaneous
 * access to the codec block registers. It is up to the caller to access them
 * sensibly.
 */
struct hva *hva_open(void __iomem *base, size_t size);

void hva_close(struct hva *hva);

/* Set/clear VCE bit in video codec interrupt mask */
void hva_set_vcintmask_vce(struct hva *hva, int value);

/* Perform HVA operations for the start of a slice decode */
void hva_dec_slice(struct hva *hva, struct hva_dec_info *dec);

/* Perform HVA operations for the start of a slice encode */
void hva_enc_slice(struct hva *hva, struct hva_enc_info *enc);

/* Perform HVA operations for the completion of a slice encode */
void hva_enc_complete_slice(struct hva *hva, struct hva_enc_info *enc);

#endif

#endif
