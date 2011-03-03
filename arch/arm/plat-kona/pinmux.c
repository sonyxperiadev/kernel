/*****************************************************************************
*
* Kona generic pinmux
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

#include <mach/pinmux.h>

int pinmux_init()
{
	pinmux_board_init();
	return 0;
}

/*
  get pin configuration at run time
  caller provides pin ball name
*/
int pinmux_get_pin_config(struct pin_config *config)
{
	int ret = 0;
	void __iomem *base;
	enum PIN_NAME name;

	if(!config)
		return -EINVAL;
	name = config->name;
	if(!is_ball_valid(name))
		return -EINVAL;

	base = ioremap(g_chip_pin_desc.base_addr, g_chip_pin_desc.mapping_size);
	if(!base) {
		printk (KERN_WARNING "%s ioremap failed\n", __func__);
		return -ENOMEM;
	}

	config->reg.val = readl(base+g_chip_pin_desc.desc_tbl[name].reg_offset);

	/* populate func */
	config->func = g_chip_pin_desc.desc_tbl[name].f_tbl[config->reg.b.sel];

	iounmap(base);
	return ret;
}

/*
  set pin configuration at run time
  caller fills pin_configuration, except sel, which will derived from func in this routine.
*/
int pinmux_set_pin_config(struct pin_config *config)
{
	int ret = 0, i;
	void __iomem *base;
	enum PIN_NAME name;

	if(!config)
		return -EINVAL;
	name = config->name;
	if(!is_ball_valid(name))
		return -EINVAL;

	base = ioremap(g_chip_pin_desc.base_addr, g_chip_pin_desc.mapping_size);
	if(!base) {
		printk (KERN_WARNING "%s ioremap failed\n", __func__);
		return -ENOMEM;
	}

	/* get the sel bits */
	for (i=0; i<MAX_ALT_FUNC; i++) {
		if (g_chip_pin_desc.desc_tbl[name].f_tbl[i] == config->func) {
			config->reg.b.sel = i;
			break;
		}
	}
	if (i==MAX_ALT_FUNC) {
		printk (KERN_WARNING "%s no matching\n", __func__);
		ret = -EINVAL;
		goto err;
	}

	writel(config->reg.val, base + g_chip_pin_desc.desc_tbl[name].reg_offset);
err:
	iounmap(base);
	return ret;
}
