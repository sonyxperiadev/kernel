/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2010  Broadcom Corporation                                                        */
/*                                                                                              */
/*     Unless you and Broadcom execute a separate written software license agreement governing  */
/*     use of this software, this software is licensed to you under the terms of the GNU        */
/*     General Public License version 2 (the GPL), available at                                 */
/*                                                                                              */
/*          http://www.broadcom.com/licenses/GPLv2.php                                          */
/*                                                                                              */
/*     with the following added to such license:                                                */
/*                                                                                              */
/*     As a special exception, the copyright holders of this software give you permission to    */
/*     link this software with independent modules, and to copy and distribute the resulting    */
/*     executable under terms of your choice, provided that you also meet, for each linked      */
/*     independent module, the terms and conditions of the license of that module.              */
/*     An independent module is a module which is not derived from this software.  The special  */
/*     exception does not apply to any modifications of the software.                           */
/*                                                                                              */
/*     Notwithstanding the above, under no circumstances may you combine this software in any   */
/*     way with any other Broadcom software provided under a license other than the GPL,        */
/*     without Broadcom's express prior written consent.                                        */
/*                                                                                              */
/************************************************************************************************/

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/cpumask.h>
#ifdef CONFIG_OF
#include <linux/of_irq.h>
#endif
#include <asm/io.h>
#include <asm/mach/map.h>
#include <linux/irqchip/arm-gic.h>

#include <mach/hardware.h>
#include <mach/irqs.h>
#include <mach/io_map.h>
#include <linux/of.h>

int gic_of_init(struct device_node *node, struct device_node *parent);

#ifdef CONFIG_OF
static const struct of_device_id kona_dt_irq_match[] __initconst = {
	{ .compatible = "arm,cortex-a9-gic", .data = gic_of_init },
	{ }
};
#endif

void __init kona_init_irq(void)
{
	/* start with GLBTIMER */
	if (!of_have_populated_dt())
		gic_init(0, BCM_INT_ID_PPI11, IOMEM(KONA_GICDIST_VA),
			IOMEM(KONA_GICCPU_VA));
#ifdef CONFIG_OF
	of_irq_init(kona_dt_irq_match);
#endif
}
