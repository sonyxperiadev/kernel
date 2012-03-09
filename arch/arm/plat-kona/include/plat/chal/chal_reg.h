/***************************************************************************
 *     Copyright (c) 2003-2008, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *   The common include header file that all C-files will be using.
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#ifndef CHAL_REG_H__
#define CHAL_REG_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
*
* Register access defines
*
*****************************************************************************/
#define CHAL_REG_READ8(addr)  (*((volatile unsigned char *)(addr)))
#define CHAL_REG_READ16(addr) (*((volatile unsigned short *)(addr)))
#define CHAL_REG_READ32(addr) (*((volatile unsigned int *)(addr)))

#define CHAL_REG_WRITE8(addr, val)  (*((volatile unsigned char *)(addr)) = (val))
#define CHAL_REG_WRITE16(addr, val) (*((volatile unsigned short *)(addr)) = (val))
#define CHAL_REG_WRITE32(addr, val) (*((volatile unsigned int *)(addr)) = (val))

#define CHAL_REG_CLRBIT8(addr, bits)   (CHAL_REG_WRITE8(addr, (CHAL_REG_READ8(addr)  & (~(bits)))))
#define CHAL_REG_CLRBIT16(addr, bits)  (CHAL_REG_WRITE16(addr, (CHAL_REG_READ16(addr) & (~(bits)))))
#define CHAL_REG_CLRBIT32(addr, bits)  (CHAL_REG_WRITE32(addr, (CHAL_REG_READ32(addr) & (~(bits)))))

#define CHAL_REG_SETBIT8(addr, bits)   (CHAL_REG_WRITE8(addr, (CHAL_REG_READ8(addr)  | (bits))))
#define CHAL_REG_SETBIT16(addr, bits)  (CHAL_REG_WRITE16(addr, (CHAL_REG_READ16(addr) | (bits))))
#define CHAL_REG_SETBIT32(addr, bits)  (CHAL_REG_WRITE32(addr, (CHAL_REG_READ32(addr) | (bits))))

#define CHAL_REG_WRITE8_P(addr, val)   (*(volatile unsigned char  *)(&(addr)) = ((unsigned char)(val)))
#define CHAL_REG_WRITE16_P(addr, val)  (*(volatile unsigned short *)(&(addr)) = ((unsigned short)(val)))
#define CHAL_REG_WRITE32_P(addr, val)  (*(volatile unsigned long  *)(&(addr))  = ((unsigned long)(val)))

#define CHAL_REG_READ8_P(addr)      (*(volatile unsigned char  *)(&(addr)))
#define CHAL_REG_READ16_P(addr)     (*(volatile unsigned short *)(&(addr)))
#define CHAL_REG_READ32_P(addr)     (*(volatile unsigned long  *)(&(addr)))

#define CHAL_REG_CLRBIT8_P(addr, bits)   (CHAL_REG_WRITE8_P(addr, (CHAL_REG_READ8_P(addr)  & (~(bits)))))
#define CHAL_REG_CLRBIT16_P(addr,bits)  (CHAL_REG_WRITE16_P(addr, (CHAL_REG_READ16_P(addr) & (~(bits)))))
#define CHAL_REG_CLRBIT32_P(addr,bits)  (CHAL_REG_WRITE32_P(addr, (CHAL_REG_READ32_P(addr) & (~(bits)))))

#define CHAL_REG_SETBIT8_P(addr, bits)   (CHAL_REG_WRITE8_P(addr, (CHAL_REG_READ8_P(addr)  | (bits))))
#define CHAL_REG_SETBIT16_P(addr, bits)  (CHAL_REG_WRITE16_P(addr, (CHAL_REG_READ16_P(addr) | (bits))))
#define CHAL_REG_SETBIT32_P(addr, bits)  (CHAL_REG_WRITE32_P(addr, (CHAL_REG_READ32_P(addr) | (bits))))

/* Following macros are added for code that uses structure of core registers */
#define CHAL_REG_READ32_S(reg) (reg)

#define CHAL_REG_WRITE32_S(reg, val) (reg) = (val)

#define CHAL_REG_CLRBIT32_S(reg, bits)  (CHAL_REG_WRITE32_S(reg, (CHAL_REG_READ32_S(reg) & (~(bits)))))

#define CHAL_REG_SETBIT32_S(reg, bits)  (CHAL_REG_WRITE32_S(reg, (CHAL_REG_READ32_S(reg) | (bits))))

#ifdef __cplusplus
}
#endif
#endif				/* CHAL_REG_H__ */
