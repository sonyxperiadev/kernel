/* Copyright (c) 2012, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/proc_fs.h>
#include <mach/msm-krait-l2-accessors.h>

#define L2ESR_IND_ADDR		0x204
#define L2ESYNR0_IND_ADDR	0x208
#define L2ESYNR1_IND_ADDR	0x209
#define L2EAR0_IND_ADDR		0x20C
#define L2EAR1_IND_ADDR		0x20D

#define L2ESR_MPDCD		BIT(0)
#define L2ESR_MPSLV             BIT(1)
#define L2ESR_TSESB             BIT(2)
#define L2ESR_TSEDB             BIT(3)
#define L2ESR_DSESB             BIT(4)
#define L2ESR_DSEDB             BIT(5)

#ifdef CONFIG_MSM_L2_ERP_PORT_PANIC
#define ERP_PORT_ERR(a) panic(a)
#else
#define ERP_PORT_ERR(a) WARN(1, a)
#endif

#ifdef CONFIG_MSM_L2_ERP_1BIT_PANIC
#define ERP_1BIT_ERR(a) panic(a)
#else
#define ERP_1BIT_ERR(a) do { } while (0)
#endif

#ifdef CONFIG_MSM_L2_ERP_2BIT_PANIC
#define ERP_2BIT_ERR(a) panic(a)
#else
#define ERP_2BIT_ERR(a) do { } while (0)
#endif

#define MODULE_NAME "msm_l2_erp"

static unsigned long msm_l2_mpdcd_err;
static unsigned long msm_l2_mpslv_err;
static unsigned long msm_l2_tsesb_err;
static unsigned long msm_l2_tsedb_err;
static unsigned long msm_l2_dsesb_err;
static unsigned long msm_l2_dsedb_err;

static struct proc_dir_entry *procfs_entry;

static int proc_read_status(char *page, char **start, off_t off, int count,
			    int *eof, void *data)
{
	char *p = page;
	int len;

	p += snprintf(p, PAGE_SIZE,
			"L2 master port decode errors:\t\t%lu\n"	\
			"L2 master port slave errors:\t\t%lu\n"		\
			"L2 tag soft errors, single-bit:\t\t%lu\n"	\
			"L2 tag soft errors, double-bit:\t\t%lu\n"	\
			"L2 data soft errors, single-bit:\t%lu\n"	\
			"L2 data soft errors, double-bit:\t%lu\n",	\
			msm_l2_mpdcd_err,
			msm_l2_mpslv_err,
			msm_l2_tsesb_err,
			msm_l2_tsedb_err,
			msm_l2_dsesb_err,
			msm_l2_dsedb_err);

	len = (p - page) - off;
	if (len < 0)
		len = 0;

	*eof = (len <= count) ? 1 : 0;
	*start = page + off;

	return len;
}

static irqreturn_t msm_l2_erp_irq(int irq, void *dev_id)
{
	unsigned int l2esr;
	unsigned int l2esynr0;
	unsigned int l2esynr1;
	unsigned int l2ear0;
	unsigned int l2ear1;
	int soft_error = 0;
	int port_error = 0;
	int unrecoverable = 0;

	l2esr = get_l2_indirect_reg(L2ESR_IND_ADDR);
	l2esynr0 = get_l2_indirect_reg(L2ESYNR0_IND_ADDR);
	l2esynr1 = get_l2_indirect_reg(L2ESYNR1_IND_ADDR);
	l2ear0 = get_l2_indirect_reg(L2EAR0_IND_ADDR);
	l2ear1 = get_l2_indirect_reg(L2EAR1_IND_ADDR);

	pr_alert("L2 Error detected!\n");
	pr_alert("\tL2ESR    = 0x%08x\n", l2esr);
	pr_alert("\tL2ESYNR0 = 0x%08x\n", l2esynr0);
	pr_alert("\tL2ESYNR1 = 0x%08x\n", l2esynr1);
	pr_alert("\tL2EAR0   = 0x%08x\n", l2ear0);
	pr_alert("\tL2EAR1   = 0x%08x\n", l2ear1);

	if (l2esr & L2ESR_MPDCD) {
		pr_alert("L2 master port decode error\n");
		port_error++;
		msm_l2_mpdcd_err++;
	}

	if (l2esr & L2ESR_MPSLV) {
		pr_alert("L2 master port slave error\n");
		port_error++;
		msm_l2_mpslv_err++;
	}

	if (l2esr & L2ESR_TSESB) {
		pr_alert("L2 tag soft error, single-bit\n");
		soft_error++;
		msm_l2_tsesb_err++;
	}

	if (l2esr & L2ESR_TSEDB) {
		pr_alert("L2 tag soft error, double-bit\n");
		soft_error++;
		unrecoverable++;
		msm_l2_tsedb_err++;
	}

	if (l2esr & L2ESR_DSESB) {
		pr_alert("L2 data soft error, single-bit\n");
		soft_error++;
		msm_l2_dsesb_err++;
	}

	if (l2esr & L2ESR_DSEDB) {
		pr_alert("L2 data soft error, double-bit\n");
		soft_error++;
		unrecoverable++;
		msm_l2_dsedb_err++;
	}

	if (port_error)
		ERP_PORT_ERR("L2 Master port error detected");

	if (soft_error && !unrecoverable)
		ERP_1BIT_ERR("L2 single-bit error detected");

	if (unrecoverable)
		ERP_2BIT_ERR("L2 double-bit error detected, trouble ahead");

	set_l2_indirect_reg(L2ESR_IND_ADDR, l2esr);
	return IRQ_HANDLED;
}

static int msm_l2_erp_probe(struct platform_device *pdev)
{
	struct resource *r;
	int ret;

	r = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	ret = request_irq(r->start, msm_l2_erp_irq, 0, "MSM_L2", NULL);

	if (ret) {
		pr_err("Failed to request the L2 cache error interrupt\n");
		return ret;
	}

	procfs_entry = create_proc_entry("cpu/msm_l2_erp", S_IRUGO, NULL);

	/*
	 * Don't bail out if this fails. We want to respond to L2 errors even
	 * if we could not register the procfs entry.
	 */
	if (procfs_entry)
		procfs_entry->read_proc = proc_read_status;
	else
		pr_err("Failed to create procfs node for L2 error reporting\n");

	return 0;
}

static int msm_l2_erp_remove(struct platform_device *pdev)
{
	struct resource *r;

	if (procfs_entry)
		remove_proc_entry("cpu/msm_l2_erp", NULL);

	r = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	free_irq(r->start, NULL);
	return 0;
}

static struct platform_driver msm_l2_erp_driver = {
	.probe = msm_l2_erp_probe,
	.remove = msm_l2_erp_remove,
	.driver = {
		.name = MODULE_NAME,
		.owner = THIS_MODULE,
	},
};

static int __init msm_l2_erp_init(void)
{
	return platform_driver_register(&msm_l2_erp_driver);
}

static void __exit msm_l2_erp_exit(void)
{
	platform_driver_unregister(&msm_l2_erp_driver);
}


module_init(msm_l2_erp_init);
module_exit(msm_l2_erp_exit);
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("MSM L2 error reporting driver");
