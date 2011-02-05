/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	include/linux/broadcom/chip_version.h
*
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

/* Description:  This file sets the chip version to be built based on -D$(BASEBAND_CHIP) setting. */
/* Note:     To add support for a new chip, do the following: */
/*         1. add a new chip ID, say BCMXXXX, for the new chip */
/*         2. define CHIPID to BCMXXXX if defined(_BCMXXXX_) */
/* $File: depot/Sources/Common/inc/chip_revision.h $ */
/* $Revision: #1 $ */
/* $DateTime: 2004/06/18 $ */
/* ****************************************************************************** */

#ifndef __CHIP_VERSION_H__
#define __CHIP_VERSION_H__

/* For MARTINI HEDGE platform */
#ifndef _BCM2153_
#define _BCM2153_
#endif
#define CHIP_REVISION 51

#define CHIP_VERSION(x, y) ((x)*100 + (y))

/* !< --- PreProcess --- >! */

/* Chip IDs: make sure they are the same as those defined in memmap.h */
#define ML2000B         0
#define ML2010          1
#define ML2010B         2
#define ML2020          3
#define ML2020B         4
#define ML2029          5
#define ML2010C         6
#define ML2021          7	/* Development chip having HS DROM. */
#define BCM2132         8
#define BCM2133         9
#define BCM2124         10
#define BCM2152         11

#ifndef BCM2153
#define BCM2153         12
#endif
#define BCM213x1        13
#define BCM21551        14
#define BCM213x1v       15
#define BCM213x1j       16
/*  Code generation:  the '!<gensym>! SYMBOL defined' comments */
/*  that follow cause assembly code to be generated in 'chip_version.inc'; */
/*  see headerpreprocess.pl and pputil.c for details. */
/*            GBLL SYMBOL */
/*          SYMBOL SETL {TRUE} ; if SYMBOL is defined */
/*      or */
/*            GBLL SYMBOL */
/*          SYMBOL SETL {FALSE} ; if SYMBOL is undefined */

/* !<gensym>! _ML2029_  defined */
/* !<gensym>! _BCM2121_ defined */
/* !<gensym>! _BCM2132_ defined */
/* !<gensym>! _BCM2133_ defined */
/* !<gensym>! _BCM2124_ defined */

/* Set chip version based on -D$(BASEBAND_CHIP) build setting */
#if defined(_ML2029_)
#error Obsolete chip not supported !!!
/* #define CHIPID   ML2029 */

#elif defined(_BCM2121_)
#error Obsolete chip not supported !!!
/* #define CHIPID   ML2021 */

#elif defined(_BCM2132_)
#define CHIPID    BCM2132

#elif defined(_BCM2133_)
#define CHIPID    BCM2133

#elif defined(_BCM2124_)
#define CHIPID    BCM2124

#elif defined(_BCM2152_)
#define CHIPID    BCM2152

#elif defined(_BCM2153_)
#define CHIPID    BCM2153

#elif defined(_BCM213x1_) && !defined(CHIP_VARIANT_VENUS) && !defined(CHIP_VARIANT_JUNO)
#define CHIPID    BCM213x1

#elif defined(_BCM213x1_) && defined(CHIP_VARIANT_VENUS)
#define CHIPID    BCM213x1v

#elif defined(_BCM213x1_) && defined(CHIP_VARIANT_JUNO)
#define CHIPID    BCM213x1j

#elif defined(_BCM21551_)
#define CHIPID    BCM21551

#elif defined(_ATHENA_)
#define CHIPID		ATHENA

#elif defined(_HERA_)
#define CHIPID		HERA

#elif defined(_RHEA_)
#define CHIPID		RHEA

#elif defined(_SAMOA_)
#define CHIPID		SAMOA
#else
#error No chip version defined!!!
#endif

/* !< --- PreProcess --- >! */

#define CHIPVERSION CHIP_VERSION(CHIPID, CHIP_REVISION)

#endif /* __CHIP_VERSION_H__ */
