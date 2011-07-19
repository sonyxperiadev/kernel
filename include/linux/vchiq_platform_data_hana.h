/*****************************************************************************
* Copyright 2008 - 2010 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/

#ifndef VCHIQ_PLATFORM_DATA_HANA_H_

#define VCHIQ_PLATFORM_DATA_HANA_H_


#include "vchiq_platform_data.h"

typedef struct
{
    VCHIQ_PLATFORM_DATA_T   common;
    uint32_t                armSharedMemAddr;
    uint32_t                vcSharedMemAddr;

} VCHIQ_PLATFORM_DATA_HANA_T;


#endif  /* VCHIQ_PLATFORM_DATA_HANA_H_ */
