/*****************************************************************************
*  Copyright 2001 - 2008 Broadcom Corporation.  All rights reserved.
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2, available at
*  http://www.gnu.org/licenses/old-license/gpl-2.0.html (the "GPL").
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
*****************************************************************************/
/**
*
* @file   Nucleus/../chal_common_os.h
*
* @brief  this file includes all the common OS dependent headers
*
*****************************************************************************/
#ifndef _CHAL_COMMON_OS_H_
#define _CHAL_COMMON_OS_H_

#include <asm/io.h>
#include <linux/delay.h>
#include <plat/types.h>
//
//Notice!!!
//Please don't add new defines without everyone agree upon
//If you need additional abstraction for your modules, please do so in your module 
//

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup cHAL_Interface 
 * @{
 */

/**
* Nucleus OS dependent defines common to all cHAL drivers.
*
* Busy wait loop limit
* Assumes 3 clock per iteration:
* 000020  e2800001          ADD      r0,r0,#1
* 000024  e150000e          CMP      r0,lr
* 000028  3afffffc          BCC      |L1.32|
*
*****************************************************************************/

#define LOOP_1_MILLISECOND   udelay(1000)

/**
*
* Delay at least t milliseconds.
*****************************************************************************/
#define CHAL_DELAY_MS( t)  mdelay(t)


/**
*
* 
*****************************************************************************/
#define CHAL_ASSERT(_x)	 \
    do {                 \
           if (!_x){     \
               while(1); \
       }                 \
    }while(0)

/**
*
* Register access defines
*
*****************************************************************************/
#define CHAL_REG_READ8(addr)  readb(addr) 
#define CHAL_REG_READ16(addr) readw(addr) 
#define CHAL_REG_READ32(addr) readl(addr) 

#define CHAL_REG_WRITE8(addr, val)  writeb(val, addr)
#define CHAL_REG_WRITE16(addr, val) writew(val, addr)
#define CHAL_REG_WRITE32(addr, val) writel(val, addr)

#define CHAL_REG_CLRBIT8(addr, bits)   (CHAL_REG_WRITE8(addr, (CHAL_REG_READ8(addr)  & (~bits))))
#define CHAL_REG_CLRBIT16(addr, bits)  (CHAL_REG_WRITE16(addr, (CHAL_REG_READ16(addr) & (~bits))))
#define CHAL_REG_CLRBIT32(addr, bits)  (CHAL_REG_WRITE32(addr, (CHAL_REG_READ32(addr) & (~bits))))

#define CHAL_REG_SETBIT8(addr, bits)   (CHAL_REG_WRITE8(addr, (CHAL_REG_READ8(addr)  | bits)))
#define CHAL_REG_SETBIT16(addr, bits)  (CHAL_REG_WRITE16(addr, (CHAL_REG_READ16(addr) | bits)))
#define CHAL_REG_SETBIT32(addr, bits)  (CHAL_REG_WRITE32(addr, (CHAL_REG_READ32(addr) | bits)))

/**
*
* Debug level
*
*****************************************************************************/
#define CDBG_ZERO       0
#define CDBG_ERRO       1
#define CDBG_WARN       2
#define CDBG_INFO       3
#define CDBG_INFO2      4

#ifndef CHAL_NDEBUG_BUILD

extern void dprintf_impl(const UInt32 level, cInt8 *fmt, ...);
#define chal_dprintf dprintf_impl

#else

#define chal_dprintf

#endif

/** @} */

#ifdef __cplusplus
}
#endif

#endif //_CHAL_COMMON_OS_H_

