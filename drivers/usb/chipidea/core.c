/*
 * core.c - ChipIdea USB IP core family device controller
 *
 * Copyright (C) 2008 Chipidea - MIPS Technologies, Inc. All rights reserved.
 *
 * Author: David Lopo
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/*
 * Description: ChipIdea USB IP core family device controller
 *
 * This driver is composed of several blocks:
 * - HW:     hardware interface
 * - DBG:    debug facilities (optional)
 * - UTIL:   utilities
 * - ISR:    interrupts handling
 * - ENDPT:  endpoint operations (Gadget API)
 * - GADGET: gadget operations (Gadget API)
 * - BUS:    bus glue code, bus abstraction layer
 *
 * Compile Options
 * - CONFIG_USB_GADGET_DEBUG_FILES: enable debug facilities
 * - STALL_IN:  non-empty bulk-in pipes cannot be halted
 *              if defined mass storage compliance succeeds but with warnings
 *              => case 4: Hi >  Dn
 *              => case 5: Hi >  Di
 *              => case 8: Hi <> Do
 *              if undefined usbtest 13 fails
 * - TRACE:     enable function tracing (depends on DEBUG)
 *
 * Main Features
 * - Chapter 9 & Mass Storage Compliance with Gadget File Storage
 * - Chapter 9 Compliance with Gadget Zero (STALL_IN undefined)
 * - Normal & LPM support
 *
 * USBTEST Report
 * - OK: 0-12, 13 (STALL_IN defined) & 14
 * - Not Supported: 15 & 16 (ISO)
 *
 * TODO List
 * - OTG
 * - Isochronous & Interrupt Traffic
 * - Handle requests which spawns into several TDs
 * - GET_STATUS(device) - always reports 0
 * - Gadget API (majority of optional features)
 */
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/idr.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/pm_runtime.h>
#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>
#include <linux/usb/otg.h>
#include <linux/usb/chipidea.h>

#include "ci.h"
#include "udc.h"
#include "bits.h"
#include "host.h"
#include "debug.h"

/* Controller register map */
static uintptr_t ci_regs_nolpm[] = {
	[CAP_CAPLENGTH]		= 0x000UL,
	[CAP_HCCPARAMS]		= 0x008UL,
	[CAP_DCCPARAMS]		= 0x024UL,
	[CAP_TESTMODE]		= 0x038UL,
	[OP_USBCMD]		= 0x000UL,
	[OP_USBSTS]		= 0x004UL,
	[OP_USBINTR]		= 0x008UL,
	[OP_DEVICEADDR]		= 0x014UL,
	[OP_ENDPTLISTADDR]	= 0x018UL,
	[OP_PORTSC]		= 0x044UL,
	[OP_DEVLC]		= 0x084UL,
	[OP_OTGSC]		= 0x064UL,
	[OP_USBMODE]		= 0x068UL,
	[OP_ENDPTSETUPSTAT]	= 0x06CUL,
	[OP_ENDPTPRIME]		= 0x070UL,
	[OP_ENDPTFLUSH]		= 0x074UL,
	[OP_ENDPTSTAT]		= 0x078UL,
	[OP_ENDPTCOMPLETE]	= 0x07CUL,
	[OP_ENDPTCTRL]		= 0x080UL,
	[OP_ENDPTPIPEID]	= 0x0BCUL,
};

static uintptr_t ci_regs_lpm[] = {
	[CAP_CAPLENGTH]		= 0x000UL,
	[CAP_HCCPARAMS]		= 0x008UL,
	[CAP_DCCPARAMS]		= 0x024UL,
	[CAP_TESTMODE]		= 0x0FCUL,
	[OP_USBCMD]		= 0x000UL,
	[OP_USBSTS]		= 0x004UL,
	[OP_USBINTR]		= 0x008UL,
	[OP_DEVICEADDR]		= 0x014UL,
	[OP_ENDPTLISTADDR]	= 0x018UL,
	[OP_PORTSC]		= 0x044UL,
	[OP_DEVLC]		= 0x084UL,
	[OP_OTGSC]		= 0x0C4UL,
	[OP_USBMODE]		= 0x0C8UL,
	[OP_ENDPTSETUPSTAT]	= 0x0D8UL,
	[OP_ENDPTPRIME]		= 0x0DCUL,
	[OP_ENDPTFLUSH]		= 0x0E0UL,
	[OP_ENDPTSTAT]		= 0x0E4UL,
	[OP_ENDPTCOMPLETE]	= 0x0E8UL,
	[OP_ENDPTCTRL]		= 0x0ECUL,
	[OP_ENDPTPIPEID]	= 0x0BCUL,
};

static int hw_alloc_regmap(struct ci13xxx *ci, bool is_lpm)
{
	int i;

	kfree(ci->hw_bank.regmap);

	ci->hw_bank.regmap = kzalloc((OP_LAST + 1) * sizeof(void *),
				     GFP_KERNEL);
	if (!ci->hw_bank.regmap)
		return -ENOMEM;

	for (i = 0; i < OP_ENDPTCTRL; i++)
		ci->hw_bank.regmap[i] =
			(i <= CAP_LAST ? ci->hw_bank.cap : ci->hw_bank.op) +
			(is_lpm ? ci_regs_lpm[i] : ci_regs_nolpm[i]);

	for (; i <= OP_LAST; i++)
		ci->hw_bank.regmap[i] = ci->hw_bank.op +
			4 * (i - OP_ENDPTCTRL) +
			(is_lpm
			 ? ci_regs_lpm[OP_ENDPTCTRL]
			 : ci_regs_nolpm[OP_ENDPTCTRL]);

	return 0;
}

/**
 * hw_port_test_set: writes port test mode (execute without interruption)
 * @mode: new value
 *
 * This function returns an error code
 */
int hw_port_test_set(struct ci13xxx *ci, u8 mode)
{
	const u8 TEST_MODE_MAX = 7;

	if (mode > TEST_MODE_MAX)
		return -EINVAL;

	hw_write(ci, OP_PORTSC, PORTSC_PTC, mode << __ffs(PORTSC_PTC));
	return 0;
}

/**
 * hw_port_test_get: reads port test mode value
 *
 * This function returns port test mode value
 */
u8 hw_port_test_get(struct ci13xxx *ci)
{
	return hw_read(ci, OP_PORTSC, PORTSC_PTC) >> __ffs(PORTSC_PTC);
}

static int hw_device_init(struct ci13xxx *ci, void __iomem *base)
{
	u32 reg;

	/* bank is a module variable */
	ci->hw_bank.abs = base;

	ci->hw_bank.cap = ci->hw_bank.abs;
	ci->hw_bank.cap += ci->platdata->capoffset;
	ci->hw_bank.op = ci->hw_bank.cap + (ioread32(ci->hw_bank.cap) & 0xff);

	hw_alloc_regmap(ci, false);
	reg = hw_read(ci, CAP_HCCPARAMS, HCCPARAMS_LEN) >>
		__ffs(HCCPARAMS_LEN);
	ci->hw_bank.lpm  = reg;
	hw_alloc_regmap(ci, !!reg);
	ci->hw_bank.size = ci->hw_bank.op - ci->hw_bank.abs;
	ci->hw_bank.size += OP_LAST;
	ci->hw_bank.size /= sizeof(u32);

	reg = hw_read(ci, CAP_DCCPARAMS, DCCPARAMS_DEN) >>
		__ffs(DCCPARAMS_DEN);
	ci->hw_ep_max = reg * 2;   /* cache hw ENDPT_MAX */

	if (ci->hw_ep_max > ENDPT_MAX)
		return -ENODEV;

	dev_dbg(ci->dev, "ChipIdea HDRC found, lpm: %d; cap: %p op: %p\n",
		ci->hw_bank.lpm, ci->hw_bank.cap, ci->hw_bank.op);

	/* setup lock mode ? */

	/* ENDPTSETUPSTAT is '0' by default */

	/* HCSPARAMS.bf.ppc SHOULD BE zero for device */

	return 0;
}

/**
 * hw_device_reset: resets chip (execute without interruption)
 * @ci: the controller
  *
 * This function returns an error code
 */
int hw_device_reset(struct ci13xxx *ci, u32 mode)
{
	int delay_count = 25; /* 250 usec */

	/* should flush & stop before reset */
	hw_write(ci, OP_ENDPTFLUSH, ~0, ~0);
	hw_write(ci, OP_USBCMD, USBCMD_RS, 0);

	hw_write(ci, OP_USBCMD, USBCMD_RST, USBCMD_RST);
	while (delay_count-- && hw_read(ci, OP_USBCMD, USBCMD_RST))
		udelay(10);
	if (delay_count < 0)
		pr_err("USB controller reset failed\n");

	if (ci->platdata->notify_event)
		ci->platdata->notify_event(ci,
			CI13XXX_CONTROLLER_RESET_EVENT);

	/* USBMODE should be configured step by step */
	hw_write(ci, OP_USBMODE, USBMODE_CM, USBMODE_CM_IDLE);
	hw_write(ci, OP_USBMODE, USBMODE_CM, mode);
	/* HW >= 2.3 */
	hw_write(ci, OP_USBMODE, USBMODE_SLOM, USBMODE_SLOM);

	/*
	 * ITC (Interrupt Threshold Control) field is to set the maximum
	 * rate at which the device controller will issue interrupts.
	 * The maximum interrupt interval measured in micro frames.
	 * Valid values are 0, 1, 2, 4, 8, 16, 32, 64. The default value is
	 * 8 micro frames. If CPU can handle interrupts at faster rate, ITC
	 * can be set to lesser value to gain performance.
	 */
	if (ci->platdata->nz_itc)
		hw_write(ci, OP_USBCMD, USBCMD_ITC_MASK,
			USBCMD_ITC(ci->platdata->nz_itc));
	if (ci->platdata->flags & CI13XXX_ZERO_ITC)
		hw_write(ci, OP_USBCMD, USBCMD_ITC_MASK, USBCMD_ITC(0));


	if (hw_read(ci, OP_USBMODE, USBMODE_CM) != mode) {
		pr_err("cannot enter in %s mode", ci_role(ci)->name);
		pr_err("lpm = %i", ci->hw_bank.lpm);
		return -ENODEV;
	}

	return 0;
}

/**
 * ci_otg_role - pick role based on ID pin state
 * @ci: the controller
 */
static enum ci_role ci_otg_role(struct ci13xxx *ci)
{
	u32 sts = hw_read(ci, OP_OTGSC, ~0);
	enum ci_role role = sts & OTGSC_ID
		? CI_ROLE_GADGET
		: CI_ROLE_HOST;

	return role;
}

/**
 * ci_role_work - perform role changing based on ID pin
 * @work: work struct
 */
static void ci_role_work(struct work_struct *work)
{
	struct ci13xxx *ci = container_of(work, struct ci13xxx, work);
	enum ci_role role = ci_otg_role(ci);

	if (role != ci->role) {
		dev_dbg(ci->dev, "switching from %s to %s\n",
			ci_role(ci)->name, ci->roles[role]->name);

		ci_role_stop(ci);
		ci_role_start(ci, role);
	}

	enable_irq(ci->irq);
}

static irqreturn_t ci_irq(int irq, void *data)
{
	struct ci13xxx *ci = data;
	irqreturn_t ret = IRQ_NONE;
	u32 otgsc = 0;

	if (ci->is_otg)
		otgsc = hw_read(ci, OP_OTGSC, ~0);

	if (ci->role != CI_ROLE_END)
		ret = ci_role(ci)->irq(ci);

	if (ci->is_otg && (otgsc & OTGSC_IDIS)) {
		hw_write(ci, OP_OTGSC, OTGSC_IDIS, OTGSC_IDIS);
		disable_irq_nosync(ci->irq);
		queue_work(ci->wq, &ci->work);
		ret = IRQ_HANDLED;
	}

	return ret;
}

static DEFINE_IDA(ci_ida);

struct platform_device *ci13xxx_add_device(struct device *dev,
			struct resource *res, int nres,
			struct ci13xxx_platform_data *platdata)
{
	struct platform_device *pdev;
	int id, ret;

	id = ida_simple_get(&ci_ida, 0, 0, GFP_KERNEL);
	if (id < 0)
		return ERR_PTR(id);

	pdev = platform_device_alloc("ci_hdrc", id);
	if (!pdev) {
		ret = -ENOMEM;
		goto put_id;
	}

	pdev->dev.parent = dev;
	pdev->dev.dma_mask = dev->dma_mask;
	pdev->dev.dma_parms = dev->dma_parms;
	dma_set_coherent_mask(&pdev->dev, dev->coherent_dma_mask);

	ret = platform_device_add_resources(pdev, res, nres);
	if (ret)
		goto err;

	ret = platform_device_add_data(pdev, platdata, sizeof(*platdata));
	if (ret)
		goto err;

	ret = platform_device_add(pdev);
	if (ret)
		goto err;

	return pdev;

err:
	platform_device_put(pdev);
put_id:
	ida_simple_remove(&ci_ida, id);
	return ERR_PTR(ret);
}
EXPORT_SYMBOL_GPL(ci13xxx_add_device);

void ci13xxx_remove_device(struct platform_device *pdev)
{
	int id = pdev->id;
	platform_device_unregister(pdev);
	ida_simple_remove(&ci_ida, id);
}
EXPORT_SYMBOL_GPL(ci13xxx_remove_device);

static int ci_hdrc_probe(struct platform_device *pdev)
{
	struct device	*dev = &pdev->dev;
	struct ci13xxx	*ci;
	struct resource	*res;
	void __iomem	*base;
	int		ret;

	if (!dev->platform_data) {
		dev_err(dev, "platform data missing\n");
		return -ENODEV;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	base = devm_ioremap_resource(dev, res);
	if (IS_ERR(base))
		return PTR_ERR(base);

	ci = devm_kzalloc(dev, sizeof(*ci), GFP_KERNEL);
	if (!ci) {
		dev_err(dev, "can't allocate device\n");
		return -ENOMEM;
	}

	spin_lock_init(&ci->lock);
	ci->dev = dev;
	ci->platdata = dev->platform_data;
	if (ci->platdata->phy)
		ci->transceiver = ci->platdata->phy;
	else
		ci->global_phy = true;

	ci->gadget.l1_supported = ci->platdata->l1_supported;

	ret = hw_device_init(ci, base);
	if (ret < 0) {
		dev_err(dev, "can't initialize hardware\n");
		return -ENODEV;
	}

	ci->hw_bank.phys = res->start;

	ci->irq = platform_get_irq(pdev, 0);
	if (ci->irq < 0) {
		dev_err(dev, "missing IRQ\n");
		return -ENODEV;
	}

	INIT_WORK(&ci->work, ci_role_work);
	ci->wq = create_singlethread_workqueue("ci_otg");
	if (!ci->wq) {
		dev_err(dev, "can't create workqueue\n");
		return -ENODEV;
	}

	/* initialize role(s) before the interrupt is requested */
	ret = ci_hdrc_host_init(ci);
	if (ret)
		dev_info(dev, "doesn't support host\n");

	ret = ci_hdrc_gadget_init(ci);
	if (ret)
		dev_info(dev, "doesn't support gadget\n");

	if (!ci->roles[CI_ROLE_HOST] && !ci->roles[CI_ROLE_GADGET]) {
		dev_err(dev, "no supported roles\n");
		ret = -ENODEV;
		goto rm_wq;
	}

	if (ci->roles[CI_ROLE_HOST] && ci->roles[CI_ROLE_GADGET]) {
		ci->is_otg = true;
		/* ID pin needs 1ms debouce time, we delay 2ms for safe */
		mdelay(2);
		ci->role = ci_otg_role(ci);
	} else {
		ci->role = ci->roles[CI_ROLE_HOST]
			? CI_ROLE_HOST
			: CI_ROLE_GADGET;
	}

	ret = ci_role_start(ci, ci->role);
	if (ret) {
		dev_err(dev, "can't start %s role\n", ci_role(ci)->name);
		ret = -ENODEV;
		goto rm_wq;
	}

	platform_set_drvdata(pdev, ci);
	ret = request_irq(ci->irq, ci_irq, IRQF_SHARED, ci->platdata->name,
			  ci);
	if (ret)
		goto stop;

	if (ci->is_otg)
		hw_write(ci, OP_OTGSC, OTGSC_IDIE, OTGSC_IDIE);

	ret = dbg_create_files(ci);
	if (!ret)
		return 0;

	free_irq(ci->irq, ci);
stop:
	ci_role_stop(ci);
rm_wq:
	flush_workqueue(ci->wq);
	destroy_workqueue(ci->wq);

	return ret;
}

static int ci_hdrc_remove(struct platform_device *pdev)
{
	struct ci13xxx *ci = platform_get_drvdata(pdev);

	dbg_remove_files(ci);
	flush_workqueue(ci->wq);
	destroy_workqueue(ci->wq);
	free_irq(ci->irq, ci);
	ci_role_stop(ci);

	return 0;
}

static struct platform_driver ci_hdrc_driver = {
	.probe	= ci_hdrc_probe,
	.remove	= ci_hdrc_remove,
	.driver	= {
		.name	= "ci_hdrc",
	},
};

module_platform_driver(ci_hdrc_driver);

MODULE_ALIAS("platform:ci_hdrc");
MODULE_ALIAS("platform:ci13xxx");
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("David Lopo <dlopo@chipidea.mips.com>");
MODULE_DESCRIPTION("ChipIdea HDRC Driver");
