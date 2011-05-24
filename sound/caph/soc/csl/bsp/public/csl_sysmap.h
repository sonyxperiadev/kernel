/****************************************************************************
*
*     Copyright (c) 2008 Broadcom Corporation
*           All Rights Reserved
*
*     No portions of this material may be reproduced in any form without the
*     written permission of:
*
*           Broadcom Corporation
*           5300 California Avenue
*           Irvine, California 92617
*
*     All information contained in this document is Broadcom Corporation
*     company private, proprietary, and trade secret.
*
****************************************************************************/
//
//   ABSTRACT: this file includes all the OS dependent headers. All
//             platform share the same file name but with different contents.
//             there are mainly three header files needed to included here.
//             1. IO access definiation which varies from OS to OS.
//             2. Memory map, which will define the memory base for all blocks.
//                that can be different on each platform.
//             3. debug macros, which is different on each platform.
//
//   TARGET:  all
//
//   TOOLSET:
//
//   HISTORY:
//   DATE       AUTHOR           DESCRIPTION
//****************************************************************************
//!
//! \file   csl_sysmap.h
//! \brief  Header file for CSL_SYSMAP
//! \brief
//!
//****************************************************************************

#ifndef _CSL_SYSMAP_H_
#define _CSL_SYSMAP_H_

UInt32 csl_sysmap_get_base_addr(SYSMAP_BASE_t block);

#endif //_CSL_SYSMAP_H_

