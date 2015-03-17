/* arch/arm/mach-msm/board-sony_shinano-hw.h
 *
 * Copyright (C) 2014 Sony Mobile Communications Inc.
 *
 * Author: Kouhei Fujiya <Kouhei.X.Fujiya@sonymobile.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _ARCH_ARM_MACH_MSM_BOARD_SONY_SHINANO_HW_H
#define _ARCH_ARM_MACH_MSM_BOARD_SONY_SHINANO_HW_H

enum sony_hw {
	HW_UNKNOWN,
	HW_LEO,
	HW_LEO_SAMBA,
	HW_SIRIUS,
	HW_SIRIUS_SAMBA,
};

int get_sony_hw(void);

#endif /* _ARCH_ARM_MACH_MSM_BOARD_SONY_SHINANO_HW_H */
