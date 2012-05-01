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
#include <mach/hardware.h>

#ifdef CONFIG_KONA_ATAG_DT
#include <linux/of.h>
#include <linux/of_fdt.h>

/* pin-mux configuration data from DT */
static uint32_t dt_pinmux[PN_MAX];
static uint32_t dt_pinmux_nr;

extern uint32_t dt_pinmux_gpio_mask[];

int __init early_init_dt_scan_pinmux(unsigned long node, const char *uname,
				     int depth, void *data)
{
	const char *prop;
	unsigned long size, i;
	uint32_t *p;

	/*printk(KERN_INFO "%s: node=0x%lx, uname=%s, depth=%d\n", __func__, node, uname, depth); */

	if (depth != 1 || strcmp(uname, "pinmux") != 0)
		return 0;	/* not found, continue... */

	prop = of_get_flat_dt_prop(node, "reg", &i);

	p = (uint32_t *)prop;
	i = be32_to_cpu(p[1]);
	printk(KERN_INFO "reg: 0x%x, 0x%x\n", be32_to_cpu(p[0]),
	       be32_to_cpu(p[1]));

	/* check the base address passed */
	if (be32_to_cpu(p[0]) != PAD_CTRL_BASE_ADDR) {
		printk(KERN_ERR "Wrong base address!\n");
		return 1;
	}

	prop = of_get_flat_dt_prop(node, "data", &size);
	printk("data(0x%x): size=%ld\n", (unsigned int)prop, size);

	/* Save it */
	dt_pinmux_nr = size / 4;
	if (i != dt_pinmux_nr) {
		printk(KERN_ERR "Mismatch size! %ld & %d\n", i, dt_pinmux_nr);
		dt_pinmux_nr = 0;
	} else if (dt_pinmux_nr > PN_MAX) {
		printk(KERN_ERR "Wrong pad number!\n");
		dt_pinmux_nr = 0;
	} else {
		p = (uint32_t *)prop;
		for (i = 0; i < dt_pinmux_nr; i++) {
			dt_pinmux[i] = be32_to_cpu(p[i]);
			/* printk(KERN_INFO "%d: 0x%x\n", i, dt_pinmux[i]); */
		}
	}

	return 1;
}
#endif /* CONFIG_KONA_ATAG_DT */

int __init pinmux_init()
{
#ifdef CONFIG_KONA_ATAG_DT
	void __iomem *base;
	int i, sel, gpio, gpio_cnt;

	/* unlock and set base */
	pinmux_chip_init();

	base = g_chip_pin_desc.base;

	if (!dt_pinmux_nr) {
		printk(KERN_ERR
		       "%s Invalid DT-Pinmux! The board may not boot!\n",
		       __func__);
		return -EINVAL;
	}

	if (dt_pinmux_nr != PN_MAX) {
		printk(KERN_WARNING
		       "%s Not enough pins in DT-Pinmux! The board may not boot!\n",
		       __func__);
	}

	gpio_cnt = 0;
	for (i = 0; i < dt_pinmux_nr; i++) {
		writel(dt_pinmux[i], base + i * 4);
		/* printk(KERN_INFO "0x%08x  pad 0x%x\n", readl(base+i* 4), i* 4); */

		sel = ((union pinmux_reg)dt_pinmux[i]).b.sel;
		if (sel >= MAX_ALT_FUNC) {
			printk(KERN_ERR "function %d out of bound for pad %d\n",
			       sel, i);
			sel %= MAX_ALT_FUNC;
		}

		/* Set GPIO mask if this is a GPIO */
		if (g_chip_pin_desc.desc_tbl[i].f_tbl[sel] >= PF_FIRST_GPIO &&
		    g_chip_pin_desc.desc_tbl[i].f_tbl[sel] <= PF_LAST_GPIO) {

			gpio =
			    g_chip_pin_desc.desc_tbl[i].f_tbl[sel] -
			    PF_FIRST_GPIO;
			dt_pinmux_gpio_mask[(gpio / 32)] |= (1 << (gpio % 32));
			/* printk(KERN_INFO "pad%d (0x%x) is used as GPIO%d (sel=%d)\n", i, i* 4, gpio, sel); */
			gpio_cnt++;
		}
	}
	printk(KERN_INFO "Configured pin-mux! %d gpio pins!\n", gpio_cnt);
#else
	pinmux_chip_init();
	pinmux_board_init();
#endif

	return 0;
}

/*
  get pin configuration at run time
  caller provides pin ball name
*/
int pinmux_get_pin_config(struct pin_config *config)
{
	int ret = 0;
	void __iomem *base = g_chip_pin_desc.base;
	enum PIN_NAME name;

	if (!config)
		return -EINVAL;
	name = config->name;
	if (!is_ball_valid(name))
		return -EINVAL;

	config->reg.val =
	    readl(base + g_chip_pin_desc.desc_tbl[name].reg_offset);

	/* populate func */
	config->func = g_chip_pin_desc.desc_tbl[name].f_tbl[config->reg.b.sel];

	return ret;
}

/*
  set pin configuration at run time
  caller fills pin_configuration, except sel, which will derived from func in this routine.
*/
int pinmux_set_pin_config(struct pin_config *config)
{
	int ret = 0, i;
	void __iomem *base = g_chip_pin_desc.base;
	enum PIN_NAME name;

	if (!config)
		return -EINVAL;
	name = config->name;
	if (!is_ball_valid(name))
		return -EINVAL;

	/* get the sel bits */
	for (i = 0; i < MAX_ALT_FUNC; i++) {
		if (g_chip_pin_desc.desc_tbl[name].f_tbl[i] == config->func) {
			config->reg.b.sel = i;
			break;
		}
	}
	if (i == MAX_ALT_FUNC) {
		printk(KERN_WARNING "%s no matching\n", __func__);
		return -EINVAL;
	}

	writel(config->reg.val,
	       base + g_chip_pin_desc.desc_tbl[name].reg_offset);

	pr_debug("[PINMUX] - write value 0x%08x to register 0x%08x\n",
		 config->reg.val, g_chip_pin_desc.desc_tbl[name].reg_offset);

	return ret;
}

int pinmux_find_gpio(enum PIN_NAME name, unsigned *gpio, enum PIN_FUNC *PF_gpio)
{
	int func;
	for (func = 0; func < MAX_ALT_FUNC; func++) {
		if ((g_chip_pin_desc.desc_tbl[name].f_tbl[func] >=
		     PF_FIRST_GPIO)
		    && (g_chip_pin_desc.desc_tbl[name].f_tbl[func] <=
			PF_LAST_GPIO)) {
			*gpio =
			    g_chip_pin_desc.desc_tbl[name].f_tbl[func] -
			    PF_FIRST_GPIO;
			*PF_gpio = g_chip_pin_desc.desc_tbl[name].f_tbl[func];
			return 0;
		}
	}
	return -ENOENT;
}

#ifdef CONFIG_KONA_ATAG_DT
uint32_t get_dts_pinmux_nr()
{
	return dt_pinmux_nr;
}


uint32_t get_dts_pinmux_value(uint32_t index)
{
	return dt_pinmux[index];
}
#endif /* CONFIG_KONA_ATAG_DT */
