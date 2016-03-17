/*****************************************************************************
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
/**
*
*  @file    pindump.c
*
*  @brief   Implements pinmux and gpio dump into proc file system
*
****************************************************************************/

/* ---- Include Files ---------------------------------------------------- */
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/list.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/string.h>
#include <linux/spinlock.h>
#include <linux/io.h>
#include <linux/fs.h>
#include <linux/seq_file.h>
#include <linux/debugfs.h>

#include <mach/pinmux.h>
#include <mach/hardware.h>

#include <mach/gpio.h>
#include <mach/rdb/brcm_rdb_gpio.h>
#include <mach/io_map.h>

#define GPIO_CTRL(gpio) (GPIO_GPCTR0_OFFSET + (gpio * 4))

static struct dentry *root_entry, *dump_dentry;

static int pindump_show(struct seq_file *m, void *v)
{
	void __iomem *base;
	void __iomem *reg_base = (void __iomem *)(KONA_GPIO2_VA);
	int i, sel, gpio, gpio_cnt = 0;
	uint32_t val, gpctr;

	base = g_chip_pin_desc.base;

	/* print pin-mux */
	for (i = 0; i < PN_MAX; i++) {
		val = readl(base + i * 4);
		seq_printf(m, "0x%08x /* pad 0x%x*/\n", val, i * 4);
	}
	/* print configured GPIO */
	seq_printf(m, "Pin-mux configured as GPIO\n");
	for (i = 0; i < PN_MAX; i++) {
		val = readl(base + i * 4);
		sel = ((union pinmux_reg)val).b.sel;
		if (g_chip_pin_desc.desc_tbl[i].f_tbl[sel] >= PF_FIRST_GPIO &&
		    g_chip_pin_desc.desc_tbl[i].f_tbl[sel] <= PF_LAST_GPIO) {
			gpio_cnt++;
			gpio =
			    g_chip_pin_desc.desc_tbl[i].f_tbl[sel] -
			    PF_FIRST_GPIO;
			gpctr = __raw_readl(reg_base + GPIO_CTRL(gpio));
			gpctr &= GPIO_GPCTR0_IOTR_MASK;
			seq_printf(m, "%d  0x%08x /*%s*/\n", gpio, gpctr,
				   ((gpctr ==
				     GPIO_GPCTR0_IOTR_CMD_INPUT) ? "Input" :
				    "Output"));
		}
	}
	seq_printf(m, "Toatl Pin-mux configured as GPIO= %d\n", gpio_cnt);

	return 0;

}

static int pindump_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, pindump_show, NULL);
}

static const struct file_operations pindump_proc_fops = {
	.open = pindump_proc_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

/***************************************************************************/

static int __init proc_pindump_init(void)
{
	struct proc_dir_entry* entry;
	entry = proc_create("pinmux", 0, NULL, &pindump_proc_fops);
	if (!entry) {
		printk(KERN_ERR "Failed to create proc entry for pindump\n");
		return -ENOMEM;
	}
	return 0;
}

/***************************************************************************/

module_init(proc_pindump_init);

/* dump initial values */
#ifdef CONFIG_KONA_ATAG_DT
static int dtsdump_open(struct inode *inode, struct file *file)
{
	return single_open(file, pindump_show, inode->i_private);
}

static const struct file_operations default_file_operations = {
	.open = dtsdump_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static int __init debug_dtsdump_init(void)
{
	root_entry = debugfs_create_dir("dtsdump", NULL);
	if (!root_entry && IS_ERR(root_entry))
		return PTR_ERR(root_entry);

	dump_dentry = debugfs_create_file("dtsdump", 0444, root_entry, NULL,
					  &default_file_operations);
	return 0;
}

static void __exit debug_dtsdump_exit(void)
{
	debugfs_remove(dump_dentry);
	debugfs_remove(root_entry);
}

module_init(debug_dtsdump_init);
module_exit(debug_dtsdump_exit);
#endif /* CONFIG_KONA_ATAG_DT */

MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("Dumps the pinmux and GPIO configuration in proc fs.");
MODULE_LICENSE("GPL");
