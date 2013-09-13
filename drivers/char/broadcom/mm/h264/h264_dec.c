/*******************************************************************************
  Copyright 2010 Broadcom Corporation.  All rights reserved.

  Unless you and Broadcom execute a separate written software license agreement
  governing use of this software, this software is licensed to you under the
  terms of the GNU General Public License version 2, available at
http://www.gnu.org/copyleft/gpl.html (the "GPL").

Notwithstanding the above, under no circumstances may you combine this software
in any way with any other Broadcom software provided under a license other than
the GPL, without Broadcom's express prior written consent.
*******************************************************************************/
#define pr_fmt(fmt) "h264_dec: " fmt
#include <linux/kernel.h>
#include <mach/rdb/brcm_rdb_h264.h>
#include "h264_dec.h"

#define RESET_TRY 200

void h264_write(void *id, u32 reg, u32 value);
u32 h264_read(void *id, u32 reg);

#undef IS_WRITE
#undef GET_OFFSET

#define IS_WRITE(x) (dec_regs[x].offset >> 31)
#define GET_OFFSET(x) (dec_regs[x].offset & ~(0xff000000))

void decodeSlice(void *id, struct dec_info_t *dec_info)
{
	u32 i, j, temp;
	struct decoder_regs_t *dec_regs = &(dec_info->dec_regs);
	for (i = 0; i < dec_info->num_dec_regs; i++) {

		if (IS_WRITE(i) == 1) {
			switch (GET_OFFSET(i)) {
			/*DecSd_RegSdParam*/
			case H264_DECSD_REGSDPARAM_OFFSET:
				if (dec_regs[i].value & 0x10000000) {
					temp = h264_read(id, GET_OFFSET(i));
					dec_regs[i].value |= temp;
				}
				h264_write(id, GET_OFFSET(i), dec_regs[i].value);
				break;
			/*DecMn_RegMainctl*/
			case H264_REG_MAINCTL_OFFSET:
				/*Handling "DecMn_RegMainctl" reset*/
				if (dec_regs[i].value & 0x1) {
					if ((dec_regs[i].value & 0xfffffffe) == 0) {
						dec_regs[i].value = h264_read(id, GET_OFFSET(i));
						dec_regs[i].value |= 0x1;
					}
					h264_write(id, GET_OFFSET(i), dec_regs[i].value);
					/*Trying to see the client is planning to wait for the reset*/
					for (j = 0; j < RESET_TRY; j++) {
						temp = h264_read(id, \
								GET_OFFSET(i));
					if ((temp & 0x1) == 0)
							break;
					}
				} else {
					h264_write(id, GET_OFFSET(i), dec_regs[i].value);
				}
				break;
			/*DecSt_RegSintStrmStat*/
			case H264_REG_SINT_STRM_STAT_OFFSET:
				if (dec_regs[i].value == 0x20000) {
					h264_write(id, GET_OFFSET(i), dec_regs[i].value);
					if ((GET_OFFSET(i+1) == H264_REG_SINT_STRM_STAT_OFFSET) &&
						(IS_WRITE(i+1) == 0)) {
						for (j = 0; j < RESET_TRY; j++) {
							temp = h264_read(id, GET_OFFSET(i));
							if ((temp & 0x800) == 0)
								break;
						}
					}
				} else if (dec_regs[i].value & 0x10000) {
					h264_write(id, GET_OFFSET(i), dec_regs[i].value);
					/*Trying to see the client is planning to wait for the reset*/
					if ((GET_OFFSET(i+1) == H264_REG_SINT_STRM_STAT_OFFSET) &&
						(IS_WRITE(i+1) == 0)) {
						for (j = 0; j < RESET_TRY; j++) {
							temp = h264_read(id, GET_OFFSET(i));
							if ((temp & (0x1 << 16)) == 0)
								break;
						}
					}
				} else {
					h264_write(id, GET_OFFSET(i), dec_regs[i].value);
				}
				break;
			/*VC_CACHE_CTL*/
			case H264_VCCACHECTL_OFFSET:
				if (GET_OFFSET(i-1) == H264_VCCACHECTL_OFFSET &&
					IS_WRITE(i-1) == 0) {
					temp = h264_read(id, GET_OFFSET(i));
					dec_regs[i].value |= temp;
				}
				h264_write(id, GET_OFFSET(i), dec_regs[i].value);
				break;
			/*DecSt_RegSintVecRefpic*/
			case H264_REG_SINT_VEC_REFPIC_OFFSET:
				if (GET_OFFSET(i-1) == H264_REG_SINT_VEC_REFPIC_OFFSET &&
					IS_WRITE(i-1) == 0) {
					temp = h264_read(id, GET_OFFSET(i));
					if (dec_regs[i].value & 0x1)
						dec_regs[i].value |= 1;
					else
						dec_regs[i].value &= (~1);
				}
				h264_write(id, GET_OFFSET(i), dec_regs[i].value);
				break;
			default:
				h264_write(id, GET_OFFSET(i), dec_regs[i].value);
			}
		} else {
			switch (GET_OFFSET(i)) {
			/*DecMn_RegStatus*/
			case H264_REG_STATUS_OFFSET:
				for (j = 0; j < RESET_TRY; j++) {
					temp = h264_read(id, GET_OFFSET(i));
					if ((temp & STATUS_ALL_RDY_DONE) == STATUS_ALL_RDY_DONE)
						break;
				}
				break;
			default:
				dec_regs[i].value = h264_read(id, GET_OFFSET(i));
			}
		}
	}

}

void completeDecodeFrame(void *id)
{
	u32 j, temp;
	/*Flush IL Context*/
	h264_write(id, H264_REG_SINT_STRM_STAT_OFFSET, 0x20000);
	for (j = 0; j < RESET_TRY; j++) {
		temp = h264_read(id, H264_REG_SINT_STRM_STAT_OFFSET);
		if ((temp & 0x800) == 0)
			break;
	}
	/*Reset IL Context*/
	h264_write(id, H264_REG_SINT_STRM_STAT_OFFSET, 0x10300);
	for (j = 0; j < RESET_TRY; j++) {
		temp = h264_read(id, H264_REG_SINT_STRM_STAT_OFFSET);
		if ((temp & (0x1 << 16)) == 0)
			break;
	}
	/*VC_CACHE_CTL Flush*/
	temp = h264_read(id, H264_VCCACHECTL_OFFSET);
	temp |= 0x1;
	h264_write(id, H264_VCCACHECTL_OFFSET, temp);

	/*Reset DecMn_RegMainctl*/
	temp = h264_read(id, H264_REG_MAINCTL_OFFSET);
	temp |= 0x1;
	h264_write(id, H264_REG_MAINCTL_OFFSET, temp);
	for (j = 0; j < RESET_TRY; j++) {
		temp = h264_read(id, H264_REG_MAINCTL_OFFSET);
		if ((temp & 0x1) == 0)
			break;
	}
}
