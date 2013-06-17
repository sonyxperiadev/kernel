/*****************************************************************************
* Copyright 2012 Broadcom Corporation.  All rights reserved.
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

#ifndef __HAWAII_CPU_H__
#define __HAWAII_CPU_H__

#include <plat/cpu.h>


#define cpu_is_hawaii_A0() \
			(get_chip_id() == KONA_CHIP_ID_HAWAII_A0)


#endif /*__HAWAII_CPU_H__*/
