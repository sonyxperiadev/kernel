/*****************************************************************************
*
* Kona DT support
*
* Copyright 2011 Broadcom Corporation.  All rights reserved.
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

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/list.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/string.h>
#include <linux/spinlock.h>
#include <linux/io.h>
#include <linux/module.h>
#include <asm/setup.h>

#include <linux/of.h>
#include <linux/of_fdt.h>
#include <mach/atag_dt.h>
#include <mach/hardware.h>


static int __init early_init_dt_scan_kona_root(unsigned long node, const char *uname,
				     int depth, void *data)
{
	const char *prop;
	unsigned long size;

	/*printk(KERN_INFO "%s: node=0x%lx, uname=%s, depth=%d\n", __func__, node, uname, depth);*/

	if (depth != 0 || strcmp(uname, "") != 0)
		return 0; /* not found, continue... */

	prop = (char *)of_get_flat_dt_prop(node, "model", &size);
	printk(KERN_INFO "model: %s\n", prop);

	prop = (char *)of_get_flat_dt_prop(node, "compatible", &size);
	printk(KERN_INFO "compatible: %s\n", prop);

	return 1;
}

static int __init parse_tag_dt(const struct tag *tag)
{
	unsigned long dt_root;

	/* set DT base address so we can use kernel flat tree API */
	initial_boot_params = (struct boot_param_header *)tag->u.cmdline.cmdline;

	dt_root = of_get_flat_dt_root();
	printk(KERN_INFO "Kona dt @0x%x, size of 0x%x words, rooted @ 0x%lx\n",
		(unsigned int)tag->u.cmdline.cmdline, tag->hdr.size, dt_root);

	/* Retrieve info from the / node */
	of_scan_flat_dt(early_init_dt_scan_kona_root, NULL);

	/* Retrieve info from the /pinmux node */
	of_scan_flat_dt(early_init_dt_scan_pinmux, NULL);

	/* Retrieve info from the /gpio node */
	of_scan_flat_dt(early_init_dt_scan_gpio, NULL);

#ifdef CONFIG_KONA_DT_BCMPMU
	of_scan_flat_dt(early_init_dt_scan_pmu, NULL);
	of_scan_flat_dt(early_init_dt_scan_batt, NULL);
#endif
	/* Retrieve info from other nodes ... */

	/* clear DT base address */
	initial_boot_params = 0;

	return 0;
}

__tagtable(ATAG_DTBLOB, parse_tag_dt);
