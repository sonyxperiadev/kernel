// SPDX-License-Identifier: ISC
/*
 * Copyright (c) 2014 Broadcom Corporation
 */
#include <linux/init.h>
#include <linux/of.h>
#include <linux/of_irq.h>

#include <defs.h>
#include "debug.h"
#include "core.h"
#include "common.h"
#include "of.h"

static const struct of_device_id fmac_devid[] = {
	{ .compatible = "brcm,bcm4329-fmac", },
	{ .compatible = "brcm,bcm4359-fmac", },
	{ }
};

void brcmf_of_probe(struct device *dev, enum brcmf_bus_type bus_type,
		    struct brcmf_mp_device *settings)
{
	struct brcmfmac_sdio_pd *sdio = &settings->bus.sdio;
	struct device_node *root, *soc_np, *np = NULL;
	struct property *prop;
	int irq;
	u32 irqf;
	u32 val;

	if (bus_type == BRCMF_BUSTYPE_USB)
		return;

	if (dev->of_node)
		np = dev->of_node;

	/* The PCI-E device has DT mostly only in embedded uses */
	if (!np && bus_type == BRCMF_BUSTYPE_PCIE) {
		soc_np = of_find_node_by_path("/soc");
		if (!soc_np)
			return;

		np = of_find_matching_node(soc_np, fmac_devid);
	}

	/* Set board-type to the first string of the machine compatible prop */
	root = of_find_node_by_path("/");
	if (root) {
		prop = of_find_property(root, "compatible", NULL);
		settings->board_type = of_prop_next_string(prop, NULL);
		of_node_put(root);
	}

	if (!np || !of_match_node(fmac_devid, np))
		return;

	if (!of_device_is_available(np))
		return;

	if (of_property_read_u32(np, "brcm,drive-strength", &val) == 0)
		sdio->drive_strength = val;

	/* make sure there are interrupts defined in the node */
	if (!of_find_property(np, "interrupts", NULL))
		return;

	irq = irq_of_parse_and_map(np, 0);
	if (!irq) {
		brcmf_err("interrupt could not be mapped\n");
		return;
	}
	irqf = irqd_get_trigger_type(irq_get_irq_data(irq));

	sdio->oob_irq_supported = true;
	sdio->oob_irq_nr = irq;
	sdio->oob_irq_flags = irqf;
}
