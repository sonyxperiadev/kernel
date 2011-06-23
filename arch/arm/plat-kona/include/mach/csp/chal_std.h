/***************************************************************************
 *     Copyright (c) 2008, Broadcom Corporation
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
 * 
 ***************************************************************************/
#ifndef CHAL_STD_H__
#define CHAL_STD_H__

/* standard types */
#include <mach/csp/chal_defs.h>

/* standard defined */
#define CHAL_ENDIAN_BIG 4321
#define CHAL_ENDIAN_LITTLE 1234

/* platform specific include file */
#include <mach/csp/chal_cfg.h>

/*
 * insuring we have a proper configuration
 */

#if ((CHAL_CPU_ENDIAN != CHAL_ENDIAN_BIG) && \
     (CHAL_CPU_ENDIAN != CHAL_ENDIAN_LITTLE))
#error Must define CHAL_CPU_ENDIAN as CHAL_ENDIAN_BIG or CHAL_ENDIAN_LITTLE.
#endif

#endif /* #ifndef CHAL_STD_H__ */

/* end of file */
