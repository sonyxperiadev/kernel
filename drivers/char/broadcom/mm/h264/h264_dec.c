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


void decodeSlice(void *id, struct dec_info_t *dec_info)
{
	u32 i, j, temp;
	for (i = 0; i < dec_info->num_dec_regs; i++) {

		if (dec_info->dec_regs[i].write == 1) {
			switch (dec_info->dec_regs[i].offset) {
			/*DecSd_RegSdParam*/
			case H264_DECSD_REGSDPARAM_OFFSET:
				if (dec_info->dec_regs[i].value & 0x10000000) {
					temp = h264_read(id, dec_info->dec_regs[i].offset);
					dec_info->dec_regs[i].value |= temp;
				}
				h264_write(id, dec_info->dec_regs[i].offset, dec_info->dec_regs[i].value);
				break;
			/*DecMn_RegMainctl*/
			case H264_REG_MAINCTL_OFFSET:
				/*Handling "DecMn_RegMainctl" reset*/
				if (dec_info->dec_regs[i].value & 0x1) {
					if ((dec_info->dec_regs[i].value & 0xfffffffe) == 0) {
						dec_info->dec_regs[i].value = h264_read(id, dec_info->dec_regs[i].offset);
						dec_info->dec_regs[i].value |= 0x1;
					}
					h264_write(id, dec_info->dec_regs[i].offset, dec_info->dec_regs[i].value);
					/*Trying to see the client is planning to wait for the reset*/
					if ((dec_info->dec_regs[i+1].offset == H264_REG_MAINCTL_OFFSET) &&
						(dec_info->dec_regs[i+1].write == 0)) {
						for (j = 0; j < RESET_TRY; j++) {
							temp = h264_read(id, dec_info->dec_regs[i].offset);
							if ((temp & 0x1) == 0)
								break;
						}
					}
				} else {
					h264_write(id, dec_info->dec_regs[i].offset, dec_info->dec_regs[i].value);
				}
				break;
			/*DecSt_RegSintStrmStat*/
			case H264_REG_SINT_STRM_STAT_OFFSET:
				if (dec_info->dec_regs[i].value == 0x20000) {
					h264_write(id, dec_info->dec_regs[i].offset, dec_info->dec_regs[i].value);
					if ((dec_info->dec_regs[i+1].offset == H264_REG_SINT_STRM_STAT_OFFSET) &&
						(dec_info->dec_regs[i+1].write == 0)) {
						for (j = 0; j < RESET_TRY; j++) {
							temp = h264_read(id, dec_info->dec_regs[i].offset);
							if ((temp & 0x800) == 0)
								break;
						}
					}
				} else if (dec_info->dec_regs[i].value & 0x10000) {
					h264_write(id, dec_info->dec_regs[i].offset, dec_info->dec_regs[i].value);
					/*Trying to see the client is planning to wait for the reset*/
					if ((dec_info->dec_regs[i+1].offset == H264_REG_SINT_STRM_STAT_OFFSET) &&
						(dec_info->dec_regs[i+1].write == 0)) {
						for (j = 0; j < RESET_TRY; j++) {
							temp = h264_read(id, dec_info->dec_regs[i].offset);
							if ((temp & (0x1 << 16)) == 0)
								break;
						}
					}
				} else {
					h264_write(id, dec_info->dec_regs[i].offset, dec_info->dec_regs[i].value);
				}
				break;
			/*VC_CACHE_CTL*/
			case H264_VCCACHECTL_OFFSET:
				if (dec_info->dec_regs[i-1].offset == H264_VCCACHECTL_OFFSET &&
					dec_info->dec_regs[i-1].write == 0) {
					temp = h264_read(id, dec_info->dec_regs[i].offset);
					dec_info->dec_regs[i].value |= temp;
				}
				h264_write(id, dec_info->dec_regs[i].offset, dec_info->dec_regs[i].value);
				break;
			/*DecSt_RegSintVecRefpic*/
			case H264_REG_SINT_VEC_REFPIC_OFFSET:
				if (dec_info->dec_regs[i-1].offset == H264_REG_SINT_VEC_REFPIC_OFFSET &&
					dec_info->dec_regs[i-1].write == 0) {
					temp = h264_read(id, dec_info->dec_regs[i].offset);
					if (dec_info->dec_regs[i].value & 0x1)
						dec_info->dec_regs[i].value |= 1;
					else
						dec_info->dec_regs[i].value &= (~1);
				}
				h264_write(id, dec_info->dec_regs[i].offset, dec_info->dec_regs[i].value);
				break;
			default:
				h264_write(id, dec_info->dec_regs[i].offset, dec_info->dec_regs[i].value);
			}
		} else {
			switch (dec_info->dec_regs[i].offset) {
			/*DecMn_RegStatus*/
			case H264_REG_STATUS_OFFSET:
				for (j = 0; j < RESET_TRY; j++) {
					temp = h264_read(id, dec_info->dec_regs[i].offset);
					if ((temp & STATUS_ALL_RDY_DONE) == STATUS_ALL_RDY_DONE)
						break;
				}
			default:
				dec_info->dec_regs[i].value = h264_read(id, dec_info->dec_regs[i].offset);
			}
		}
	}

}
