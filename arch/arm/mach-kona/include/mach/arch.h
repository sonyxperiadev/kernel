/*
 * linux/arch/arm/mach-kona/include/mach/arch.h
 *
 * Copyright (C) 2009 Broadcom Corporation.
 *
 * This software is licensed under the terms of the GNU General Public
 * License, version 2, as published by the Free Software Foundation
 * (the "GPL"), and may be copied, distributed, and modified under
 * those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GPL
 * for more details.
 *
 * A copy of the GPL is available at
 * http://www.broadcom.com/licenses/GPLv2.php or by writing to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA  02111-1307, USA
 */

#ifndef __ASM_ARCH_ARCH_H
#define __ASM_ARCH_ARCH_H

extern void __init kona_init_irq(void);
extern void __init kona_map_io(void);
#endif
