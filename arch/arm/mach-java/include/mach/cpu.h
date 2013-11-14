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

#ifndef __JAVA_CPU_H__
#define __JAVA_CPU_H__

#include <plat/cpu.h>


#define cpu_is_java_A0() \
			(get_chip_id() == KONA_CHIP_ID_JAVA_A0)
#define cpu_is_java_A1() \
			(get_chip_id() == KONA_CHIP_ID_JAVA_A1)

void cpu_info_verbose(void);

#endif /*__JAVA_CPU_H__*/
