/*****************************************************************************
*  Copyright 2009 - 2009 Broadcom Corporation.  All rights reserved.
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2, available at
*  http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
*****************************************************************************/

// ***************************************************************************
// void ethHw_get64( uint32_t addr, uint64_t *datap );

   .global ethHw_get64

ethHw_get64:
   push  {r4}
   ldm   r0, {r3, r4}
   stm   r1, {r3, r4}
   pop   {r4}
   bx    lr


// ***************************************************************************
// void ethHw_set64_byVal( uint32_t addr, uint64_t data )

   .global ethHw_set64_byVal

ethHw_set64_byVal:
   stm   r0, {r2, r3}
   bx    lr


// ***************************************************************************
// void ethHw_set64_byRef( uint32_t addr, uint64_t *datap )

   .global ethHw_set64_byRef

ethHw_set64_byRef:
   push  {r4}
   ldm   r1, {r3, r4}
   stm   r0, {r3, r4}
   pop   {r4}
   bx    lr
