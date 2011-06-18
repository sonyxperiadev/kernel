/***************************************************************************
 *     Copyright (c) 2002-2008, Broadcom Corporation
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
 *
 * Revision History:
 *
 * $brcm_Log: $
 ***************************************************************************/
#ifndef BCM_REG_MEM__
#define BCM_REG_MEM__

#ifdef __cplusplus
extern "C" {
#endif

/* handle is not used for now */
#define BCM_REG_WRITE64( handle, addr, wdata ) (*((volatile unsigned long long *)(addr)) = (wdata))
#define BCM_REG_WRITE32( handle, addr, wdata ) (*((volatile unsigned long      *)(addr)) = (wdata))
#define BCM_REG_WRITE16( handle, addr, wdata ) (*((volatile unsigned short     *)(addr)) = (wdata))
#define BCM_REG_WRITE8( handle, addr, wdata ) (*((volatile unsigned char      *)(addr)) = (wdata))
#define BCM_REG_READ64( handle, addr )         (*((volatile unsigned long long * )(addr)))
#define BCM_REG_READ32( handle, addr )         (*((volatile unsigned long      * )(addr)))
#define BCM_REG_READ16( handle, addr )         (*((volatile unsigned short     * )(addr)))
#define BCM_REG_READ8( handle, addr )         (*((volatile unsigned char      * )(addr)))

#ifdef __cplusplus
}
#endif

#endif /* BCM_REG_MEM__ */
