/*
 * linux/arch/arm/mach-bcm2153/include/mach/io.h
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

#ifndef __ASM_ARCH_IO_H
#define __ASM_ARCH_IO_H

#define IO_SPACE_LIMIT          0xffffffff

#define __io(a)                 __typesafe_io(a)
#define __mem_pci(a)            (a)

/*
 * IO space mapping
 */
#define IO_PHYS                 0x34000000
#define IO_OFFSET               0xF0000000
#define IO_SIZE                 0x0C000000  /* from 0x34000000 to 0x3FF0XXXX */
#define __IO_ADDRESS(x)         ((x) + IO_OFFSET - IO_PHYS)

#ifdef __ASSEMBLER__
#define IOMEM(x)                x
#else
#define IOMEM(x)                ((void __force __iomem *)(x))
#endif

#define IO_ADDRESS(pa)          IOMEM(__IO_ADDRESS(pa))

#endif
