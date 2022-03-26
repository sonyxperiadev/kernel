/*
 * Copyright (c) 2016-2020 The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include "hif.h"
#include "hif_main.h"
#include "multibus.h"
#include "pci_api.h"
#include "hif_io32.h"
#include "dummy.h"
#include "ce_api.h"

/**
 * hif_initialize_pci_ops() - initialize the pci ops
 * @bus_ops: hif_bus_ops table pointer to initialize
 *
 * Return: QDF_STATUS_SUCCESS
 */
QDF_STATUS hif_initialize_pci_ops(struct hif_softc *hif_sc)
{
	struct hif_bus_ops *bus_ops = &hif_sc->bus_ops;

	bus_ops->hif_bus_open = &hif_pci_open;
	bus_ops->hif_bus_close = &hif_pci_close;
	bus_ops->hif_bus_prevent_linkdown = &hif_pci_prevent_linkdown;
	bus_ops->hif_reset_soc = &hif_pci_reset_soc;
	bus_ops->hif_bus_suspend = &hif_pci_bus_suspend;
	bus_ops->hif_bus_resume = &hif_pci_bus_resume;
	bus_ops->hif_bus_suspend_noirq = &hif_pci_bus_suspend_noirq;
	bus_ops->hif_bus_resume_noirq = &hif_pci_bus_resume_noirq;

	/* do not put the target to sleep for epping or maxperf mode */
	if (CONFIG_ATH_PCIE_MAX_PERF == 0 &&
	    !QDF_IS_EPPING_ENABLED(hif_get_conparam(hif_sc)))
		bus_ops->hif_target_sleep_state_adjust =
			&hif_pci_target_sleep_state_adjust;
	else
		bus_ops->hif_target_sleep_state_adjust =
			&hif_dummy_target_sleep_state_adjust;

	bus_ops->hif_disable_isr = &hif_pci_disable_isr;
	bus_ops->hif_nointrs = &hif_pci_nointrs;
	bus_ops->hif_enable_bus = &hif_pci_enable_bus;
	bus_ops->hif_disable_bus = &hif_pci_disable_bus;
#ifdef FEATURE_RUNTIME_PM
	bus_ops->hif_bus_get_rpm_ctx = &hif_pci_get_rpm_ctx;
	bus_ops->hif_bus_get_dev = &hif_pci_get_dev;
#endif
	bus_ops->hif_bus_configure = &hif_pci_bus_configure;
	bus_ops->hif_get_config_item = &hif_dummy_get_config_item;
	bus_ops->hif_set_mailbox_swap = &hif_dummy_set_mailbox_swap;
	bus_ops->hif_claim_device = &hif_dummy_claim_device;
	bus_ops->hif_shutdown_device = &hif_ce_stop;
	bus_ops->hif_stop = &hif_ce_stop;
	bus_ops->hif_cancel_deferred_target_sleep =
					&hif_pci_cancel_deferred_target_sleep;
	bus_ops->hif_irq_disable = &hif_pci_irq_disable;
	bus_ops->hif_irq_enable = &hif_pci_irq_enable;
	bus_ops->hif_dump_registers = &hif_pci_dump_registers;
	bus_ops->hif_dump_target_memory = &hif_ce_dump_target_memory;
	bus_ops->hif_ipa_get_ce_resource = &hif_ce_ipa_get_ce_resource;
	bus_ops->hif_mask_interrupt_call = &hif_dummy_mask_interrupt_call;
	bus_ops->hif_enable_power_management =
		&hif_pci_enable_power_management;
	bus_ops->hif_disable_power_management =
		&hif_pci_disable_power_management;
	bus_ops->hif_grp_irq_configure = &hif_pci_configure_grp_irq;
	bus_ops->hif_display_stats =
		&hif_pci_display_stats;
	bus_ops->hif_clear_stats =
		&hif_pci_clear_stats;
	bus_ops->hif_addr_in_boundary = &hif_pci_addr_in_boundary;

	/* default to legacy mapping handler; override as needed */
	bus_ops->hif_map_ce_to_irq = &hif_pci_legacy_map_ce_to_irq;
	bus_ops->hif_needs_bmi = &hif_pci_needs_bmi;

	bus_ops->hif_config_irq_affinity =
		&hif_pci_config_irq_affinity;
	bus_ops->hif_log_bus_info = &hif_log_pcie_info;

	return QDF_STATUS_SUCCESS;
}

/**
 * hif_update_irq_ops_with_pci() - reinitialize the pci ops
 * for hybrid bus type device qcn9100 ie.connected to pci slot
 * but act as ahb bus device from host perspective
 *
 * @hif_sc: hif_softc to get bus ops
 *
 * Return: QDF_STATUS_SUCCESS
 */
QDF_STATUS hif_update_irq_ops_with_pci(struct hif_softc *hif_sc)
{
	struct hif_bus_ops *bus_ops = &hif_sc->bus_ops;

	bus_ops->hif_grp_irq_configure = &hif_pci_configure_grp_irq;
	bus_ops->hif_nointrs = &hif_pci_nointrs;
	bus_ops->hif_irq_disable = &hif_pci_irq_disable;
	bus_ops->hif_irq_enable = &hif_pci_irq_enable;
	bus_ops->hif_disable_isr = &hif_pci_disable_isr;

	return QDF_STATUS_SUCCESS;
}

/**
 * hif_pci_get_context_size() - return the size of the pci context
 *
 * Return the size of the context.  (0 for invalid bus)
 */
int hif_pci_get_context_size(void)
{
	return sizeof(struct hif_pci_softc);
}
