/*
 * NOTE: This file has been modified by Sony Corporation.
 * Modifications are Copyright 2020 Sony Corporation,
 * and licensed under the license of the file.
 */
// SPDX-License-Identifier: GPL-2.0-only
/* Copyright (c) 2019 The Linux Foundation. All rights reserved.
 */

#include <linux/err.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/reboot.h>
#include <linux/pm.h>
#include <linux/of_address.h>
#include <linux/nvmem-consumer.h>

struct qcom_reboot_reason {
	struct device *dev;
	struct notifier_block reboot_nb;
	struct notifier_block panic_nb;
	bool use_custom_reasons;
	struct nvmem_cell *nvmem_cell;
};

struct poweroff_reason {
	const char *cmd;
	unsigned char pon_reason;
};

struct poweroff_reason_ext {
	struct poweroff_reason reason;
	unsigned int imem_reason;
};

static struct poweroff_reason reasons[] = {
	{ "recovery",			0x01 },
	{ "bootloader",			0x02 },
	{ "rtc",			0x03 },
	{ "dm-verity device corrupted",	0x04 },
	{ "dm-verity enforcing",	0x05 },
	{ "keys clear",			0x06 },
	{}
};

static struct poweroff_reason_ext custom_reasons[] = {
	{{ "poweroff",			0x00 }, 0x00000000 },
	{{ "unknown",			0x01 }, 0x77665501 },
	{{ "recovery",			0x02 }, 0x77665500 },
	{{ "bootloader",		0x03 }, 0x77665500 },
	{{ "rtc",			0x04 }, 0x77665500 },
	{{ "dm-verity device corrupted",0x05 }, 0x77665508 },
	{{ "dm-verity enforcing",	0x06 }, 0x77665509 },
	{{ "keys clear",		0x07 }, 0x7766550A },
	{{ "panic",			0x40 }, 0xC0DEDEAD },
	{{ "oem-50",			0x51 }, 0x6F656D4E },
	{{ "oem-53",			0x60 }, 0x6F656D53 },
	{{ "oem-4E",			0x61 }, 0x6F656D50 },
	{}
};

static void *restart_reason;

static int qcom_reboot_reason_panic(struct notifier_block *this,
				     unsigned long event, void *ptr)
{
	char *cmd = "panic";
	struct qcom_reboot_reason *reboot = container_of(this,
		struct qcom_reboot_reason, panic_nb);
	struct poweroff_reason_ext *reason;

	for (reason = custom_reasons; reason->reason.cmd; reason++) {
		if (!strcmp(cmd, reason->reason.cmd)) {
			nvmem_cell_write(reboot->nvmem_cell,
					 &reason->reason.pon_reason,
					 sizeof(reason->reason.pon_reason));
			__raw_writel(reason->imem_reason, restart_reason);
			break;
		}
	}

	pr_info("%s: reboot cmd:%s\n", __func__, cmd);
	return NOTIFY_OK;
}

static int qcom_reboot_reason_custom_reboot(struct notifier_block *this,
				     unsigned long event, void *ptr)
{
	char *cmd = ptr ? ptr : "unknown";
	struct qcom_reboot_reason *reboot = container_of(this,
		struct qcom_reboot_reason, reboot_nb);
	struct poweroff_reason_ext *reason;
	static int nvmem_write = 0;

	if (event == SYS_POWER_OFF)
		cmd = "poweroff";

	for (reason = custom_reasons; reason->reason.cmd; reason++) {
		if (!strcmp(cmd, reason->reason.cmd)) {
			nvmem_cell_write(reboot->nvmem_cell,
					 &reason->reason.pon_reason,
					 sizeof(reason->reason.pon_reason));
			__raw_writel(reason->imem_reason, restart_reason);
			nvmem_write = 1;
			break;
		}
	}

	if (!nvmem_write) {
		reason = &custom_reasons[1]; /* unknown */
		nvmem_cell_write(reboot->nvmem_cell,
				 &reason->reason.pon_reason,
				 sizeof(reason->reason.pon_reason));
		__raw_writel(reason->imem_reason, restart_reason);
	}

	pr_info("%s: reboot cmd:%s\n", __func__, cmd);
	return NOTIFY_OK;
}

static int qcom_reboot_reason_reboot(struct notifier_block *this,
				     unsigned long event, void *ptr)
{
	char *cmd = ptr;
	struct qcom_reboot_reason *reboot = container_of(this,
		struct qcom_reboot_reason, reboot_nb);
	struct poweroff_reason *reason;

	if (!cmd)
		return NOTIFY_OK;
	for (reason = reasons; reason->cmd; reason++) {
		if (!strcmp(cmd, reason->cmd)) {
			nvmem_cell_write(reboot->nvmem_cell,
					 &reason->pon_reason,
					 sizeof(reason->pon_reason));
			break;
		}
	}

	return NOTIFY_OK;
}

static int qcom_reboot_reason_probe(struct platform_device *pdev)
{
	struct qcom_reboot_reason *reboot;
	struct device_node *node = pdev->dev.of_node;
	struct device_node *np;

	reboot = devm_kzalloc(&pdev->dev, sizeof(*reboot), GFP_KERNEL);
	if (!reboot)
		return -ENOMEM;

	reboot->dev = &pdev->dev;

	reboot->nvmem_cell = nvmem_cell_get(reboot->dev, "restart_reason");

	if (IS_ERR(reboot->nvmem_cell))
		return PTR_ERR(reboot->nvmem_cell);

	np = of_find_compatible_node(NULL, NULL,
				"qcom,msm-imem-restart_reason");
	if (!np) {
		pr_err("unable to find DT imem restart reason node\n");
	} else {
		restart_reason = of_iomap(np, 0);
		if (!restart_reason) {
			pr_err("unable to map imem restart reason offset\n");
			devm_kfree(&pdev->dev, reboot);
			return -ENOMEM;
		}
	}

	reboot->use_custom_reasons = of_property_read_bool(node,
			               "qcom,use_custom_reasons");
	reboot->reboot_nb.notifier_call = qcom_reboot_reason_reboot;
	if (reboot->use_custom_reasons)
		reboot->reboot_nb.notifier_call =
			qcom_reboot_reason_custom_reboot;
	reboot->reboot_nb.priority = 255;
	register_reboot_notifier(&reboot->reboot_nb);

	if (reboot->use_custom_reasons) {
		reboot->panic_nb.notifier_call = qcom_reboot_reason_panic;
		reboot->panic_nb.priority = 255;
		atomic_notifier_chain_register(&panic_notifier_list,
                                       &reboot->panic_nb);
	}

	platform_set_drvdata(pdev, reboot);

	return 0;
}

static int qcom_reboot_reason_remove(struct platform_device *pdev)
{
	struct qcom_reboot_reason *reboot = platform_get_drvdata(pdev);

	unregister_reboot_notifier(&reboot->reboot_nb);

	return 0;
}

static const struct of_device_id of_qcom_reboot_reason_match[] = {
	{ .compatible = "qcom,reboot-reason", },
	{},
};
MODULE_DEVICE_TABLE(of, of_qcom_reboot_reason_match);

static struct platform_driver qcom_reboot_reason_driver = {
	.probe = qcom_reboot_reason_probe,
	.remove = qcom_reboot_reason_remove,
	.driver = {
		.name = "qcom-reboot-reason",
		.of_match_table = of_match_ptr(of_qcom_reboot_reason_match),
	},
};

module_platform_driver(qcom_reboot_reason_driver);

MODULE_DESCRIPTION("MSM Reboot Reason Driver");
MODULE_LICENSE("GPL v2");
