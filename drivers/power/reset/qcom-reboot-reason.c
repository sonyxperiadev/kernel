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
	struct nvmem_cell *nvmem_cell;
};

struct poweroff_reason {
	const char *cmd;
	unsigned char pon_reason;
};

static struct poweroff_reason reasons[] = {
	{ "poweroff",			0x00 },
	{ "recovery",			0x01 },
	{ "bootloader",			0x02 },
	{ "rtc",			0x03 },
	{ "dm-verity device corrupted",	0x04 },
	{ "dm-verity enforcing",	0x05 },
	{ "keys clear",			0x06 },
	{ "normal",			0x07 },
	{ "panic",			0x40 },
	{ "hwwd",			0x41 },
	{ "rd_ok",			0x49 },
	{ "rd_wd",			0x4C },
	{ "rd_space_err",		0x4D },
	{ "rd_kp",			0x4E },
	{ "rd_err",			0x4F },
	{ "oem-50",			0x51 },
	{ "oem-53",			0x60 },
	{ "oem-4E",			0x61 },
};


static struct platform_device *priv_pdev;
static int rd_recovery;

static int rd_recovery_set(const char *val,
					const struct kernel_param *kp)
{
	int ret;
	char *cmd = NULL;
	struct qcom_reboot_reason *reboot = platform_get_drvdata(priv_pdev);
	struct poweroff_reason *reason = NULL;

	ret = param_set_int(val, kp);
	if (ret)
		return ret;

	cmd = rd_recovery ? "rd_wd" : "hwwd";

	for (reason = reasons; reason->cmd; reason++) {
		if (!strcmp(cmd, reason->cmd)) {
			nvmem_cell_write(reboot->nvmem_cell,
				&reason->pon_reason,
				sizeof(reason->pon_reason));
			break;
		}
	}
	return 0;
}

const struct kernel_param_ops ramdump_recovery_ops = {
	.set = rd_recovery_set,
	.get = param_get_int,
};
module_param_cb(rd_recovery, &ramdump_recovery_ops, &rd_recovery, 0644);

static int qcom_reboot_reason_panic(struct notifier_block *this,
				     unsigned long event, void *ptr)
{
	char *cmd = rd_recovery ? "rd_kp" : "panic";
	struct qcom_reboot_reason *reboot = container_of(this,
		struct qcom_reboot_reason, panic_nb);
	struct poweroff_reason *reason;

	for (reason = reasons; reason->cmd; reason++) {
		if (!strcmp(cmd, reason->cmd)) {
			nvmem_cell_write(reboot->nvmem_cell,
					 &reason->pon_reason,
					 sizeof(reason->pon_reason));
			break;
		}
	}

	pr_info("%s: reboot cmd:%s\n", __func__, cmd);
	return NOTIFY_OK;
}

static int qcom_reboot_reason_reboot(struct notifier_block *this,
				     unsigned long event, void *ptr)
{
	char *cmd = ptr ? ptr : "normal";
	struct qcom_reboot_reason *reboot = container_of(this,
		struct qcom_reboot_reason, reboot_nb);
	struct poweroff_reason *reason;
	int nvmem_write = 0;

	if (event == SYS_POWER_OFF)
		cmd = "poweroff";

	for (reason = reasons; reason->cmd; reason++) {
		if (!strcmp(cmd, reason->cmd)) {
			nvmem_cell_write(reboot->nvmem_cell,
					 &reason->pon_reason,
					 sizeof(reason->pon_reason));
			nvmem_write = 1;
			break;
		}
	}

	if (!nvmem_write) {
		for (reason = reasons; reason->cmd; reason++) {
			if (!strcmp("normal", reason->cmd)) {
				nvmem_cell_write(reboot->nvmem_cell,
					&reason->pon_reason,
					sizeof(reason->pon_reason));
			}
		}
	}

	pr_info("%s: reboot cmd:%s\n", __func__, cmd);
	return NOTIFY_OK;
}

static int qcom_reboot_reason_probe(struct platform_device *pdev)
{
	struct qcom_reboot_reason *reboot;

	reboot = devm_kzalloc(&pdev->dev, sizeof(*reboot), GFP_KERNEL);
	if (!reboot)
		return -ENOMEM;

	reboot->dev = &pdev->dev;

	reboot->nvmem_cell = nvmem_cell_get(reboot->dev, "restart_reason");

	if (IS_ERR(reboot->nvmem_cell))
		return PTR_ERR(reboot->nvmem_cell);

	reboot->reboot_nb.notifier_call = qcom_reboot_reason_reboot;
	reboot->reboot_nb.priority = 255;
	register_reboot_notifier(&reboot->reboot_nb);

	reboot->panic_nb.notifier_call = qcom_reboot_reason_panic;
	reboot->panic_nb.priority = 255;
	atomic_notifier_chain_register(&panic_notifier_list,
                                       &reboot->panic_nb);

	platform_set_drvdata(pdev, reboot);

	priv_pdev = pdev;

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
