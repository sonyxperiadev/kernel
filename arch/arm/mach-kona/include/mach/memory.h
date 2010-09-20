/*
 * linux/arch/arm/mach-bcm2153/include/mach/memory.h
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

#ifndef __ASM_ARCH_MEMORY_H
#define __ASM_ARCH_MEMORY_H

/*
 * Physical DRAM offset.
 */
#define PHYS_OFFSET             UL(0x80000000)
/*
 * Two definitions are required for sparsemem:
 *
 * MAX_PHYSMEM_BITS: The number of physical address bits required
 *   to address the last byte of memory.
 *
 * SECTION_SIZE_BITS: The number of physical address bits to cover
 *   the maximum amount of memory in a section.
 *
 * Eg, if you have 2 banks of up to 64MB at 0x80000000, 0x84000000,
 * then MAX_PHYSMEM_BITS is 32, SECTION_SIZE_BITS is 26.
 *
 * Define these in your mach/memory.h.
 */

#define MAX_PHYSMEM_BITS 32
#define SECTION_SIZE_BITS 24

#endif
