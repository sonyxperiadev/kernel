/****************************************************************************
*
*     Copyright (c) 2007-2008 Broadcom Corporation
*
*   Unless you and Broadcom execute a separate written software license
*   agreement governing use of this software, this software is licensed to you
*   under the terms of the GNU General Public License version 2, available
*    at http://www.gnu.org/licenses/old-licenses/gpl-2.0.html (the "GPL").
*
*   Notwithstanding the above, under no circumstances may you combine this
*   software in any way with any other Broadcom software provided under a license
*   other than the GPL, without Broadcom's express prior written consent.
*
****************************************************************************/
/**
*
*   @file   mobcom_types.h
*
*   @brief  This file contains the global typedefs of the mobile station
*		    processor software component
*
****************************************************************************/

#ifndef _MOBCOM_TYPES_H_
#define _MOBCOM_TYPES_H_

/** @{ */
typedef char		Int8;
typedef short		Int16;
typedef long		Int32;
typedef unsigned char	UInt8;
typedef unsigned short	UInt16;
typedef unsigned long	UInt32;
#ifndef _CHAL_TYPES_H_
typedef bool		Boolean;
#define TRUE		true
#define FALSE		false
#endif
#endif

