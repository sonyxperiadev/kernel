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
#ifndef _VD3_UTILS_H_
#define _VD3_UTILS_H_

#include <linux/kernel.h>
/*Header for avoiding inclusion of vd3 header files*/

#define UCHAR_MAX 0xFF

#define INST 0
#define KHRN_HW_ADDR(x) x

u32 	ol_read	(u32 reg);
void 	ol_write	(u32 reg, u32 value);

#define VCD_REG_RD(x, y) 	ol_read(VC4VCODEC0_##y)
#define VCD_REG_WT(x, y, z)	ol_write(VC4VCODEC0_##y, z)

#define vd3_wait_for_writes() (void)0
#define vd3_wait_for_reads()  (void)0


/*vcos_assert.h*/
#define vcos_assert(cond) (cond)
#define vcos_verify(cond) (cond)

//vc_asm_ops.h
static __inline int _msb(int val) {
   int msb=31;
   if (val==0) return -1;
   while ((val&(1<<msb))==0)
      msb--;
   return msb;
}

static __inline int _count(int val) {
   int i, res = 0;
   for (i = 0; i < 32; ++i)
      if (val & (1<<i)) ++res;
   return res;
}

static uint32_t _bitrev(uint32_t x, uint32_t y)
{
   uint32_t bitrev = 0;
   uint32_t i;
   for (i = 0; i != y; ++i)
   {      bitrev |= ((x >> i) & 1) << (y - i - 1);
   }
   return bitrev;
}

#endif /*_VD3_UTILS_H_*/
