/*
 * Copyright (c) 2013-2020 The Linux Foundation. All rights reserved.
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

/**
 * DOC: if_ahb_reset.c
 *
 * c file for ahb ipq4019 specific implementations.
 */

#include "hif.h"
#include "target_type.h"
#include "hif_main.h"
#include "hif_debug.h"
#include "hif_io32.h"
#include "ce_main.h"
#include "ce_tasklet.h"
#include "ahb_api.h"
#include "if_ahb.h"
#include "qal_vbus_dev.h"

#include <linux/clk.h>
#include <linux/of_address.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/of.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0)
#include <linux/reset.h>
#endif

/**
 * clk_enable_disable() -  Enable/disable clock
 * @dev : pointer to device structure
 * @str : clock name
 * @enable : should be true, if the clock needs to be enabled
 *           should be false, if the clock needs to be enabled
 *
 * This is a helper function for hif_ahb_clk_enable_disable to enable
 * disable clocks.
 * clk_prepare_enable will enable the clock
 * clk_disable_unprepare will disable the clock
 *
 * Return: zero on success, non-zero incase of error.
 */

static int clk_enable_disable(struct device *dev, const char *str, int enable)
{
	struct clk *clk_t = NULL;
	int ret;
	QDF_STATUS status;

	clk_t = clk_get(dev, str);
	if (IS_ERR(clk_t)) {
		hif_err("Failed to get %s clk %ld", str, PTR_ERR(clk_t));
		return -EFAULT;
	}
	if (true == enable) {
		/* Prepare and Enable clk */
		status = qal_vbus_enable_devclk((struct qdf_dev_clk *)clk_t);
		ret = qdf_status_to_os_return(status);
		if (ret) {
			hif_err("enabling clk: %s, error: %d", str, ret);
		}
	} else {
		/* Disable and unprepare clk */
		status = qal_vbus_disable_devclk((struct qdf_dev_clk *)clk_t);
		ret = qdf_status_to_os_return(status);
	}
	return ret;
}


/**
 * hif_ahb_clk_enable_disable() - Enable/disable ahb clock
 * @dev : pointer to device structure
 * @enable : should be true, if the clock needs to be enabled
 *           should be false, if the clock needs to be enabled
 *
 * This functions helps to enable/disable all the necesasary clocks
 * for bus access.
 *
 * Return: zero on success, non-zero incase of error
 */
int hif_ahb_clk_enable_disable(struct device *dev, int enable)
{
	int ret;

	ret = clk_enable_disable(dev, "wifi_wcss_cmd", enable);
	if (ret)
		return ret;
	ret = clk_enable_disable(dev, "wifi_wcss_ref", enable);
	if (ret)
		return ret;
	ret = clk_enable_disable(dev, "wifi_wcss_rtc", enable);
	if (ret)
		return ret;
	return 0;
}

/**
 * hif_enable_radio() - Enable the target radio.
 * @sc : pointer to the hif context
 *
 * This function helps to release the target from reset state
 *
 * Return : zero on success, non-zero incase of error.
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0)
int hif_ahb_enable_radio(struct hif_pci_softc *sc,
		struct platform_device *pdev,
		const struct platform_device_id *id)
{
	struct reset_control *reset_ctl = NULL;
	uint32_t msi_addr, msi_base, wifi_core_id;
	struct hif_softc *scn = HIF_GET_SOFTC(sc);
	struct device_node *dev_node = pdev->dev.of_node;
	bool msienable = false;
	int ret = 0;
	struct qdf_vbus_rstctl *vrstctl = NULL;

	ret = of_property_read_u32(dev_node, "qca,msi_addr", &msi_addr);
	if (ret) {
		hif_err("Unable to get msi_addr - error :%d", ret);
		return -EIO;
	}
	ret = of_property_read_u32(dev_node, "qca,msi_base", &msi_base);
	if (ret) {
		hif_err("Unable to get msi_base - error: %d", ret);
		return -EIO;
	}
	ret = of_property_read_u32(dev_node, "core-id", &wifi_core_id);
	if (ret) {
		hif_err("Unable to get core-id - error: %d", ret);
		return -EIO;
	}

	/* Program the above values into Wifi scratch regists */
	if (msienable) {
		hif_write32_mb(sc, sc->mem + FW_AXI_MSI_ADDR, msi_addr);
		hif_write32_mb(sc, sc->mem + FW_AXI_MSI_DATA, msi_base);
	}

	/* TBD: Temporary changes. Frequency should be
	 * retrieved through clk_xxx once kernel GCC driver is available
	 */
	{
		void __iomem *mem_gcc;
		uint32_t clk_sel;
		uint32_t gcc_fepll_pll_div;
		uint32_t wifi_cpu_freq[4] = {266700000, 250000000, 222200000,
								200000000};
		uint32_t current_freq = 0;

		/* Enable WIFI clock input */
		if (scn->target_info.target_type == TARGET_TYPE_IPQ4019) {
			ret = hif_ahb_clk_enable_disable(&pdev->dev, 1);
			if (ret) {
				hif_err("Error while enabling clock :%d", ret);
				return ret;
			}
		}

		mem_gcc = ioremap_nocache(GCC_BASE, GCC_SIZE);
		if (IS_ERR(mem_gcc)) {
			hif_err("GCC ioremap failed");
			return PTR_ERR(mem_gcc);
		}
		gcc_fepll_pll_div = hif_read32_mb(sc, mem_gcc +
						  GCC_FEPLL_PLL_DIV);
		clk_sel = (wifi_core_id == 0) ? ((gcc_fepll_pll_div &
				GCC_FEPLL_PLL_CLK_WIFI_0_SEL_MASK) >>
					GCC_FEPLL_PLL_CLK_WIFI_0_SEL_SHIFT) :
		((gcc_fepll_pll_div & GCC_FEPLL_PLL_CLK_WIFI_1_SEL_MASK)
					>> GCC_FEPLL_PLL_CLK_WIFI_1_SEL_SHIFT);
		current_freq = wifi_cpu_freq[clk_sel];

		hif_debug("Wifi%d CPU frequency %u", wifi_core_id, current_freq);
		hif_write32_mb(sc, sc->mem + FW_CPU_PLL_CONFIG,
			       gcc_fepll_pll_div);
		iounmap(mem_gcc);
	}

	/* De-assert radio cold reset */
	qal_vbus_get_dev_rstctl((struct qdf_pfm_hndl *)&pdev->dev,
				"wifi_radio_cold", &vrstctl);
	reset_ctl = (struct reset_control *)vrstctl;
	if (IS_ERR(reset_ctl)) {
		hif_err("Failed to get radio cold reset control");
		ret = PTR_ERR(reset_ctl);
		goto err_reset;
	}
	qal_vbus_deactivate_dev_rstctl((struct qdf_pfm_hndl *)&pdev->dev,
				       (struct qdf_vbus_rstctl *)reset_ctl);
	qal_vbus_release_dev_rstctl((struct qdf_pfm_hndl *)&pdev->dev,
				    (struct qdf_vbus_rstctl *)reset_ctl);

	/* De-assert radio warm reset */
	qal_vbus_get_dev_rstctl((struct qdf_pfm_hndl *)&pdev->dev,
				"wifi_radio_warm", &vrstctl);
	reset_ctl = (struct reset_control *)vrstctl;
	if (IS_ERR(reset_ctl)) {
		hif_err("Failed to get radio warm reset control");
		ret = PTR_ERR(reset_ctl);
		goto err_reset;
	}
	qal_vbus_deactivate_dev_rstctl((struct qdf_pfm_hndl *)&pdev->dev,
				       (struct qdf_vbus_rstctl *)reset_ctl);
	qal_vbus_release_dev_rstctl((struct qdf_pfm_hndl *)&pdev->dev,
				    (struct qdf_vbus_rstctl *)reset_ctl);

	/* De-assert radio srif reset */
	qal_vbus_get_dev_rstctl((struct qdf_pfm_hndl *)&pdev->dev,
				"wifi_radio_srif",  &vrstctl);
	reset_ctl = (struct reset_control *)vrstctl;
	if (IS_ERR(reset_ctl)) {
		hif_err("Failed to get radio srif reset control");
		ret = PTR_ERR(reset_ctl);
		goto err_reset;
	}
	qal_vbus_deactivate_dev_rstctl((struct qdf_pfm_hndl *)&pdev->dev,
				       (struct qdf_vbus_rstctl *)reset_ctl);
	qal_vbus_release_dev_rstctl((struct qdf_pfm_hndl *)&pdev->dev,
				    (struct qdf_vbus_rstctl *)reset_ctl);

	/* De-assert target CPU reset */
	qal_vbus_get_dev_rstctl((struct qdf_pfm_hndl *)&pdev->dev,
				"wifi_cpu_init", &vrstctl);
	reset_ctl = (struct reset_control *)vrstctl;
	if (IS_ERR(reset_ctl)) {
		hif_err("Failed to get cpu init reset control");
		ret = PTR_ERR(reset_ctl);
		goto err_reset;
	}
	qal_vbus_deactivate_dev_rstctl((struct qdf_pfm_hndl *)&pdev->dev,
				       (struct qdf_vbus_rstctl *)reset_ctl);
	qal_vbus_release_dev_rstctl((struct qdf_pfm_hndl *)&pdev->dev,
				    (struct qdf_vbus_rstctl *)reset_ctl);

	return 0;

err_reset:
	return -EIO;
}
#else
int hif_ahb_enable_radio(struct hif_pci_softc *sc,
		struct platform_device *pdev,
		const struct platform_device_id *id)
{
	qdf_print("%s:%d:Reset routines not available in kernel version.",
		  __func__, __LINE__);
	return -EIO;
}
#endif

/* "wifi_core_warm" is the other reset type */
#define AHB_RESET_TYPE "wifi_core_cold"

/**
 * hif_ahb_device_reset() - Disable the radio and held the radio is reset state.
 * @scn : pointer to the hif context
 *
 * This function will hold the target in reset state.
 * Will be called while unload the driver or any graceful unload path.
 *
 * Return : n/a.
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0)
void hif_ahb_device_reset(struct hif_softc *scn)
{
	struct reset_control *resetctl = NULL;
	struct reset_control *core_resetctl = NULL;
	struct hif_pci_softc *sc = HIF_GET_PCI_SOFTC(scn);
	struct platform_device *pdev = (struct platform_device *)(sc->pdev);
	uint32_t glb_cfg_offset;
	uint32_t haltreq_offset;
	uint32_t haltack_offset;
	void __iomem *mem_tcsr;
	uint32_t wifi_core_id = 0XFFFFFFFF;
	uint32_t reg_value;
	int wait_limit = ATH_AHB_RESET_WAIT_MAX;
	struct qdf_vbus_rstctl *vrstctl = NULL;


	wifi_core_id = hif_read32_mb(sc, sc->mem +
				     WLAN_SUBSYSTEM_CORE_ID_ADDRESS);
	glb_cfg_offset = (wifi_core_id == 0) ? TCSR_WIFI0_GLB_CFG :
							TCSR_WIFI1_GLB_CFG;
	haltreq_offset = (wifi_core_id == 0) ? TCSR_WCSS0_HALTREQ :
							TCSR_WCSS1_HALTREQ;
	haltack_offset = (wifi_core_id == 0) ? TCSR_WCSS0_HALTACK :
							TCSR_WCSS1_HALTACK;

	mem_tcsr = ioremap_nocache(TCSR_BASE, TCSR_SIZE);
	if (IS_ERR(mem_tcsr)) {
		hif_err("TCSR ioremap failed");
		return;
	}
	reg_value = hif_read32_mb(sc, mem_tcsr + haltreq_offset);
	hif_write32_mb(sc, mem_tcsr + haltreq_offset, reg_value | 0x1);
	/* Wait for halt ack before asserting reset */
	while (wait_limit) {

		if (hif_read32_mb(sc, mem_tcsr + haltack_offset) & 0x1)
			break;

		qdf_mdelay(1);
		wait_limit--;
	}

	reg_value = hif_read32_mb(sc, mem_tcsr + glb_cfg_offset);
	hif_write32_mb(sc, mem_tcsr + glb_cfg_offset, reg_value | (1 << 25));

	qal_vbus_get_dev_rstctl((struct qdf_pfm_hndl *)&pdev->dev,
				AHB_RESET_TYPE, &vrstctl);
	core_resetctl = (struct reset_control *)vrstctl;
	if (IS_ERR(core_resetctl)) {
		hif_err("Failed to get wifi core cold reset control");
		return;
	}

	/* Reset wifi core */
	qal_vbus_activate_dev_rstctl((struct qdf_pfm_hndl *)&pdev->dev,
				     (struct qdf_vbus_rstctl *)core_resetctl);

	/* TBD: Check if we should also assert other bits (radio_cold, radio_
	 * warm, radio_srif, cpu_ini)
	 */
	qdf_mdelay(1); /* TBD: Get reqd delay from HW team */

	/* Assert radio cold reset */
	qal_vbus_get_dev_rstctl((struct qdf_pfm_hndl *)&pdev->dev,
				"wifi_radio_cold", &vrstctl);
	resetctl = (struct reset_control *)vrstctl;
	if (IS_ERR(resetctl)) {
		hif_err("Failed to get radio cold reset control");
		return;
	}
	qal_vbus_activate_dev_rstctl((struct qdf_pfm_hndl *)&pdev->dev,
				     (struct qdf_vbus_rstctl *)resetctl);
	qdf_mdelay(1); /* TBD: Get reqd delay from HW team */
	qal_vbus_release_dev_rstctl((struct qdf_pfm_hndl *)&pdev->dev,
				    (struct qdf_vbus_rstctl *)resetctl);

	/* Assert radio warm reset */
	qal_vbus_get_dev_rstctl((struct qdf_pfm_hndl *)&pdev->dev,
				"wifi_radio_warm", &vrstctl);
	resetctl = (struct reset_control *)vrstctl;
	if (IS_ERR(resetctl)) {
		hif_err("Failed to get radio warm reset control");
		return;
	}
	qal_vbus_activate_dev_rstctl((struct qdf_pfm_hndl *)&pdev->dev,
				     (struct qdf_vbus_rstctl *)resetctl);
	qdf_mdelay(1); /* TBD: Get reqd delay from HW team */
	qal_vbus_release_dev_rstctl((struct qdf_pfm_hndl *)&pdev->dev,
				    (struct qdf_vbus_rstctl *)resetctl);

	/* Assert radio srif reset */
	qal_vbus_get_dev_rstctl((struct qdf_pfm_hndl *)&pdev->dev,
				"wifi_radio_srif", &vrstctl);
	resetctl = (struct reset_control *)vrstctl;
	if (IS_ERR(resetctl)) {
		hif_err("Failed to get radio srif reset control");
		return;
	}
	qal_vbus_activate_dev_rstctl((struct qdf_pfm_hndl *)&pdev->dev,
				     (struct qdf_vbus_rstctl *)resetctl);
	qdf_mdelay(1); /* TBD: Get reqd delay from HW team */
	qal_vbus_release_dev_rstctl((struct qdf_pfm_hndl *)&pdev->dev,
				    (struct qdf_vbus_rstctl *)resetctl);

	/* Assert target CPU reset */
	qal_vbus_get_dev_rstctl((struct qdf_pfm_hndl *)&pdev->dev,
				"wifi_cpu_init", &vrstctl);
	resetctl = (struct reset_control *)vrstctl;
	if (IS_ERR(resetctl)) {
		hif_err("Failed to get cpu init reset control");
		return;
	}
	qal_vbus_activate_dev_rstctl((struct qdf_pfm_hndl *)&pdev->dev,
				     (struct qdf_vbus_rstctl *)resetctl);
	qdf_mdelay(10); /* TBD: Get reqd delay from HW team */
	qal_vbus_release_dev_rstctl((struct qdf_pfm_hndl *)&pdev->dev,
				    (struct qdf_vbus_rstctl *)resetctl);

	/* Clear gbl_cfg and haltreq before clearing Wifi core reset */
	reg_value = hif_read32_mb(sc, mem_tcsr + haltreq_offset);
	hif_write32_mb(sc, mem_tcsr + haltreq_offset, reg_value & ~0x1);
	reg_value = hif_read32_mb(sc, mem_tcsr + glb_cfg_offset);
	hif_write32_mb(sc, mem_tcsr + glb_cfg_offset, reg_value & ~(1 << 25));

	/* de-assert wifi core reset */
	qal_vbus_deactivate_dev_rstctl((struct qdf_pfm_hndl *)&pdev->dev,
				       (struct qdf_vbus_rstctl *)core_resetctl);

	qdf_mdelay(1); /* TBD: Get reqd delay from HW team */

	/* TBD: Check if we should de-assert other bits here */
	qal_vbus_release_dev_rstctl((struct qdf_pfm_hndl *)&pdev->dev,
				    (struct qdf_vbus_rstctl *)core_resetctl);
	iounmap(mem_tcsr);
	hif_info("Reset complete for wifi core id: %d", wifi_core_id);
}
#else
void hif_ahb_device_reset(struct hif_softc *scn)
{
	qdf_print("%s:%d:Reset routines not available in kernel version.",
		  __func__, __LINE__);
}
#endif



