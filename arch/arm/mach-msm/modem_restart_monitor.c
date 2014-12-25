/* kernel/arch/arm/mach-msm/modem_restart_monitor.c
 *
 * Copyright (C) 2013-2014 Sony Mobile Communications AB.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/notifier.h>
#include <linux/switch.h>

#include <mach/subsystem_notif.h>

#define MODEM_POWERUP	0
#define MODEM_SHUTDOWN	1

#define MODEM_NAME	"modem"

static void *ss_handle;

static struct switch_dev modem_state_switch = {
	.name = "modem_restart_monitor"
};

static int modem_restart_notifier_cb(struct notifier_block *this,
		unsigned long code, void *handle)
{
	pr_debug("%s: code = %lu.\n", __func__, code);

	switch (code) {
	case SUBSYS_BEFORE_SHUTDOWN:
		switch_set_state(&modem_state_switch, MODEM_SHUTDOWN);
		break;
	case SUBSYS_AFTER_POWERUP:
		switch_set_state(&modem_state_switch, MODEM_POWERUP);
		break;
	default:
		break;
	}

	return NOTIFY_DONE;
}

static struct notifier_block nb = {
	.notifier_call = modem_restart_notifier_cb,
};

static int __init modem_restart_monitor_init(void)
{
	int rc;

	rc = switch_dev_register(&modem_state_switch);
	if (IS_ERR_VALUE(rc)) {
		pr_err("%s: switch_dev_register: err = %d\n", __func__, rc);
		return -ENOMEM;
	}

	ss_handle = subsys_notif_register_notifier(MODEM_NAME, &nb);
	if (IS_ERR(ss_handle)) {
		pr_err("%s: subsys_notif_register_notifier: err\n", __func__);
		switch_dev_unregister(&modem_state_switch);
		return -ENOMEM;
	}

	switch_set_state(&modem_state_switch, MODEM_POWERUP);
	pr_debug("%s: modem restart monitor initialized.\n", __func__);
	return 0;
}

static void __exit modem_restart_monitor_exit(void)
{
	subsys_notif_unregister_notifier(ss_handle, &nb);
	switch_dev_unregister(&modem_state_switch);
}

module_init(modem_restart_monitor_init);
module_exit(modem_restart_monitor_exit);

MODULE_DESCRIPTION("MSM Modem Subsystem Restart Monitor");
MODULE_LICENSE("GPL v2");
