/*
 * linux/arch/arm/mach-kona/kona.c
 *
 * Copyright (C) 20010 Broadcom Corporation.
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
#include <linux/init.h>

#include <asm/mach/map.h>
#include <asm/mach/arch.h>
#include <mach/hardware.h>
#include <mach/io.h>
#include <mach/arch.h>


static struct map_desc kona_io_desc[] __initdata = {
	{
		.virtual        = (unsigned long)IO_OFFSET,
		.pfn            = __phys_to_pfn(IO_PHYS),
		.length         = IO_SIZE,
		.type           = MT_DEVICE,
	}
};

void __init kona_map_io(void)
{
	iotable_init(kona_io_desc, ARRAY_SIZE(kona_io_desc));
}

void __init kona_init_irq(void)
{
	/* start with GLBTIMER */ 
	gic_dist_init(0, __io(MM_IO_BASE_GICDIST), BCM_INT_ID_PPI11);
	gic_cpu_init(0, __io(MM_IO_BASE_GICCPU));
}


