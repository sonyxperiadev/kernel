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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/elf.h>
#include <linux/err.h>

#include "peripheral-loader.h"
#include "scm-pas.h"

static int nop_verify_blob(struct pil_desc *pil, u32 phy_addr, size_t size)
{
	return 0;
}

static int pil_vidc_init_image(struct pil_desc *pil, const u8 *metadata,
		size_t size)
{
	return pas_init_image(PAS_VIDC, metadata, size);
}

static int pil_vidc_reset(struct pil_desc *pil)
{
	return pas_auth_and_reset(PAS_VIDC);
}

static int pil_vidc_shutdown(struct pil_desc *pil)
{
	return pas_shutdown(PAS_VIDC);
}

static struct pil_reset_ops pil_vidc_ops = {
	.init_image = pil_vidc_init_image,
	.verify_blob = nop_verify_blob,
	.auth_and_reset = pil_vidc_reset,
	.shutdown = pil_vidc_shutdown,
};

static int __devinit pil_vidc_driver_probe(struct platform_device *pdev)
{
	struct pil_desc *desc;

	if (pas_supported(PAS_VIDC) < 0)
		return -ENOSYS;

	desc = devm_kzalloc(&pdev->dev, sizeof(*desc), GFP_KERNEL);
	if (!desc)
		return -ENOMEM;

	desc->name = "vidc";
	desc->dev = &pdev->dev;
	desc->ops = &pil_vidc_ops;
	if (msm_pil_register(desc))
		return -EINVAL;
	return 0;
}

static int __devexit pil_vidc_driver_exit(struct platform_device *pdev)
{
	return 0;
}

static struct platform_driver pil_vidc_driver = {
	.probe = pil_vidc_driver_probe,
	.remove = __devexit_p(pil_vidc_driver_exit),
	.driver = {
		.name = "pil_vidc",
		.owner = THIS_MODULE,
	},
};

static int __init pil_vidc_init(void)
{
	return platform_driver_register(&pil_vidc_driver);
}
module_init(pil_vidc_init);

static void __exit pil_vidc_exit(void)
{
	platform_driver_unregister(&pil_vidc_driver);
}
module_exit(pil_vidc_exit);

MODULE_DESCRIPTION("Support for secure booting vidc images");
MODULE_LICENSE("GPL v2");
