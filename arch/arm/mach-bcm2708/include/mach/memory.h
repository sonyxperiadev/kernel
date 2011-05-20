/*
 *  arch/arm/mach-versatile/include/mach/memory.h
 *
 *  Copyright (C) 2003 ARM Limited
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifndef __ASM_ARCH_MEMORY_H
#define __ASM_ARCH_MEMORY_H

/* Memory overview:

   [ARMcore] <--virtual addr-->
   [ARMmmu] <--physical addr-->
   [GERTmap] <--bus add-->
   [VCperiph]

*/

/*
 * Physical DRAM offset.
 */
#define PHYS_OFFSET	 UL(0x00000000)
#define ARMMEM_OFFSET    UL(0x03000000)   /* offset in VC of ARM memory */
#define _REAL_BUS_OFFSET UL(0xC0000000)   /* don't use L1 or L2 caches */
#define VC_BASE_ADDR_IN_ARM_MAP UL(0x0D000000) /* offset to the VC physical 0x0 in ARM's view */

/* We're using the memory at 64M in the VideoCore for Linux - this adjustment
   will provide the offset into this area as well as setting the bits that
   stop the L1 and L2 cache from being used

   WARNING: this only works because the ARM is given memory at a fixed location
            (ARMMEM_OFFSET)
*/
#define BUS_OFFSET          (ARMMEM_OFFSET + _REAL_BUS_OFFSET)
#define __virt_to_bus(x)    ((x) + (BUS_OFFSET - PAGE_OFFSET))
#define __bus_to_virt(x)    ((x) - (BUS_OFFSET - PAGE_OFFSET))
#define __pfn_to_bus(x)     (__pfn_to_phys(x) + (BUS_OFFSET - PHYS_OFFSET))
#define __bus_to_pfn(x)     __phys_to_pfn((x) - (BUS_OFFSET - PHYS_OFFSET))
#define __bus_to_phys(x)    ((x) - _REAL_BUS_OFFSET + VC_BASE_ADDR_IN_ARM_MAP) /* Assuming this is always IPC memory */


/*
 *  Two definitions are required for sparsemem:
 *  
 *  MAX_PHYSMEM_BITS: The number of physical address bits required
 *  to address the last byte of memory.
 *
 *  SECTION_SIZE_BITS: The number of physical address bits to cover
 *  the maximum amount of memory in a section.
 *
 *  Eg, if you have 2 banks of up to 64MB at 0x80000000, 0x84000000,
 *  then MAX_PHYSMEM_BITS is 32, SECTION_SIZE_BITS is 26.
 *            *
 *  Define these in your mach/memory.h.
 */

#define MAX_PHYSMEM_BITS  28
#define SECTION_SIZE_BITS 23

/*
 * Boost our consistent DMA area to 4M to support 1920x1080 framebuffers
 */

/*
#define CONSISTENT_DMA_SIZE SZ_4M
*/
#endif
