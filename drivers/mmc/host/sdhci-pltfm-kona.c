/*
 * sdhci-pltfm-kona.c Support for SDHCI KONA platform devices
 * Copyright (c) 2009 Intel Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/* Portions Copyright (c) 2010 - 2012 Broadcom Corporation */

/*
 * Supports:
 * SDHCI platform devices specific for KONA
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/highmem.h>
#include <linux/platform_device.h>
#include <linux/mmc/host.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/clk.h>
#include <linux/regulator/consumer.h>
#include <linux/pm_runtime.h>
#include <mach/sdio_platform.h>

#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>
#include <linux/slab.h>
#include <linux/of_gpio.h>
#include <linux/tick.h>

#ifdef CONFIG_APANIC_ON_MMC
#include <linux/mmc-poll/mmc_poll_stack.h>
#endif

#include "sdhci-pltfm.h"
#include "sdhci.h"

#define SDHCI_SOFT_RESET            0x01000000
#define KONA_SDHOST_CORECTRL        0x8000
#define KONA_SDHOST_CD_PINCTRL      0x00000008
#define KONA_SDHOST_STOP_HCLK       0x00000004
#define KONA_SDHOST_RESET           0x00000002
#define KONA_SDHOST_EN              0x00000001

#define KONA_SDHOST_CORESTAT        0x8004
#define KONA_SDHOST_CD_SW           0x00000001

#define KONA_SDHOST_COREIMR         0x8008
#define KONA_SDHOST_IP              0x00000001

#define KONA_SDHOST_COREISR         0x800C
#define KONA_SDHOST_COREIMSR        0x8010
#define KONA_SDHOST_COREDBG1        0x8014
#define KONA_SDHOST_COREGPO_MASK    0x8018

#define DEV_NAME                    "sdio"
#define MAX_DEV_NAME_SIZE           20

#define MAX_PROC_BUF_SIZE           256
#define MAX_PROC_NAME_SIZE          20
#define PROC_ENTRY_CARD_CTRL        "card_ctrl"

#define SD_DETECT_GPIO_DEBOUNCE_128MS	128

#define KONA_SDMMC_DISABLE_DELAY	(100)
#ifdef CONFIG_MACH_BCM_FPGA
#define KONA_SDMMC_OFF_TIMEOUT		(800000)
#else
#define KONA_SDMMC_OFF_TIMEOUT		(8000)
#endif

#define KONA_SD_CONTRLR_REG_LPCNT	(50)

#define KONA_MMC_AUTOSUSPEND_DELAY	(200)
#define KONA_MMC_WIFI_AUTOSUSPEND_DELAY	(250)

/* Enable this quirk if regulators are always ON
 * but the regulator framework is not funtional.
 * In such a case, we dont want our probe to fail.
 */
#if 0
#define BCM_REGULATOR_SKIP_QUIRK
#endif

enum {ENABLED = 0, DISABLED, OFF};

struct sdio_dev {
	atomic_t initialized;
	struct device *dev;
	struct sdhci_host *host;
	unsigned long clk_hz;
	enum sdio_devtype devtype;
	int cd_gpio;
	int suspended;
	int runtime_pm_enabled;
	struct sdio_wifi_gpio_cfg *wifi_gpio;
	struct clk *peri_clk;
	struct clk *sleep_clk;
	struct regulator *vddo_sd_regulator;
	struct regulator *vdd_sdxc_regulator;
	struct wake_lock cd_int_wake_lock;
	struct mutex regulator_lock;
	int sdio_regulator_enable;
	unsigned char *cd_int_wake_lock_name;
};

#ifdef CONFIG_MACH_BCM2850_FPGA
/* user can specify the clock when this driver is installed */
static unsigned int clock;
module_param(clock, uint, 0444);
#endif

static struct sdio_dev *gDevs[SDIO_DEV_TYPE_MAX];

static int sdhci_pltfm_regulator_init(struct sdio_dev *dev, char *reg_name);
static int sdhci_pltfm_regulator_sdxc_init(struct sdio_dev *dev,
					   char *reg_name);
static int
sdhci_set_regulator_power(struct sdhci_host *host, int power_state);
static int kona_sdio_regulator_power(struct sdio_dev *dev,
		int power_state);
static int sdhci_pltfm_clk_enable(struct sdio_dev *dev, int enable);
static int sdhci_pltfm_set_signalling(struct sdhci_host *host, int sig_vol);
static int sdhci_pltfm_set_3v3_signalling(struct sdhci_host *host);
static int sdhci_pltfm_set_1v8_signalling(struct sdhci_host *host);
static void sdhci_pltfm_init_74_clocks(struct sdhci_host *host,
							u8 power_mode);
static int sdhci_pltfm_rpm_enabled(struct sdio_dev *dev);
static int sdhci_rpm_enabled(struct sdhci_host *host);
static int sdhci_clk_enable(struct sdhci_host *host, int enable);
static void sdhci_print_critical(struct sdhci_host *host);

/*
 * Get the base clock. Use central clock source for now. Not sure if different
 * clock speed to each dev is allowed
 */
static unsigned int sdhci_get_max_clk(struct sdhci_host *host)
{
	unsigned int i;

	for (i = 0; i < SDIO_DEV_TYPE_MAX; i++) {
		if (gDevs[i] != NULL && host == gDevs[i]->host)
			return gDevs[i]->clk_hz;
	}

	pr_err("unable to obtain sd max clock\n");
	return 0;
}

static unsigned int sdhci_get_timeout_clock(struct sdhci_host *host)
{
	return sdhci_get_max_clk(host);
}

static struct sdhci_ops sdhci_pltfm_ops = {
	.get_max_clock = sdhci_get_max_clk,
	.get_timeout_clock = sdhci_get_timeout_clock,
	.set_regulator = sdhci_set_regulator_power,
	.clk_enable = sdhci_clk_enable,
	.rpm_enabled = sdhci_rpm_enabled,
	.set_signalling = sdhci_pltfm_set_signalling,
	.platform_send_init_74_clocks = sdhci_pltfm_init_74_clocks,
	.sdhci_debug = sdhci_print_critical,
};

static int bcm_kona_sd_reset(struct sdio_dev *dev)
{
	struct sdhci_host *host = dev->host;
	unsigned int val;
	unsigned long timeout;

	/* Reset host controller by setting 'Software Reset for All' */
	sdhci_writeb(host, SDHCI_RESET_ALL, SDHCI_SOFTWARE_RESET);

	/* Wait for 100 ms max (100ms timeout is taken from sdhci.c) */
	timeout = jiffies + msecs_to_jiffies(100);

	while (sdhci_readb(host, SDHCI_SOFTWARE_RESET) & SDHCI_RESET_ALL) {
		if (time_is_before_jiffies(timeout)) {
			dev_err(dev->dev, "Error: sd host is in reset!!!\n");
			return -EFAULT;
		}
	}

	/* reset the host using the top level reset */
	val = sdhci_readl(host, KONA_SDHOST_CORECTRL);
	val |= KONA_SDHOST_RESET;
	sdhci_writel(host, val, KONA_SDHOST_CORECTRL);
	do {
		val = sdhci_readl(host, KONA_SDHOST_CORECTRL);
	} while (0 == (val & KONA_SDHOST_RESET));

	/* bring the host out of reset */
	val = sdhci_readl(host, KONA_SDHOST_CORECTRL);
	val &= ~KONA_SDHOST_RESET;

	/* Back-to-Back register write needs a delay of 1ms
	 * at bootup (min 10uS)
	 */
	udelay(1000);
	sdhci_writel(host, val, KONA_SDHOST_CORECTRL);

	return 0;
}

static int bcm_kona_sd_init(struct sdio_dev *dev)
{
	struct sdhci_host *host = dev->host;
	unsigned int val;

	/* enable the interrupt from the IP core */
	val = sdhci_readl(host, KONA_SDHOST_COREIMR);
	val |= KONA_SDHOST_IP;
	sdhci_writel(host, val, KONA_SDHOST_COREIMR);

	/*
	 * Enable DAT3 logic for card detection and enable the AHB clock to the
	 * host
	 */
	val = sdhci_readl(host, KONA_SDHOST_CORECTRL);
	val |= /*KONA_SDHOST_CD_PINCTRL | */ KONA_SDHOST_EN;

	/* Back-to-Back register write needs a delay of 1ms
	 * at bootup (min 10uS)
	 */
	udelay(1000);
	sdhci_writel(host, val, KONA_SDHOST_CORECTRL);

	return 0;
}

/*
 * Software emulation of the SD card insertion/removal. Set insert=1 for insert
 * and insert=0 for removal
 */
static int bcm_kona_sd_card_emulate(struct sdio_dev *dev, int insert)
{
	struct sdhci_host *host = dev->host;
	uint32_t val;
	unsigned long flags;
	int ret = 0;

#ifndef CONFIG_ARCH_ISLAND
	/*
	 * The clock enabled here will be disabled in
	 * sdhci_tasklet_card
	 */
	if (sdhci_pltfm_rpm_enabled(dev)) {
		/*
		 * The code below can be executed just
		 * after a device resume. pm core
		 * enables the runtime pm for the device
		 * when the resume callback returns.
		 * It is possible that we reach here before
		 * that. One option is to wait for the RPM
		 * to be enabled, but this can create an
		 * obvious issue when we perform card
		 * insert/removal during the probe. We cant
		 * remove the pm_runtime_get_sync with a
		 * direct clock enable here, because the
		 * sdhci_irq will complain about "interrupt
		 * while runtime suspened". We cant even check
		 * for pm_runtime status and take an appropriate
		 * action, because if the status changes by the
		 * time the clcok disable is done in card tasklet,
		 * it will result in unbalanced RPM usage. Moreover
		 * why to wait for something in an ISR when we have
		 * an option.
		 * So a better option is to do both.
		 */
		pm_runtime_get_sync(dev->dev);
	}

	/* Enable clock once irrespective of RPM state */
	ret = sdhci_pltfm_clk_enable(dev, 1);
	if (ret) {
		dev_err(dev->dev,
			"enable clock during card emulate failed\n");
		return -EAGAIN;
	}
#endif
	/* this function can be called from various contexts including ISR */
	spin_lock_irqsave(&host->lock, flags);

	/* Ensure SD bus scanning to detect media change */
	host->mmc->rescan_disable = 0;

	/* Back-to-Back register write needs a delay of min 10uS.
	 * We keep 20uS
	 */
	udelay(20);
	val = sdhci_readl(host, KONA_SDHOST_CORESTAT);

	if (insert) {
		val |= KONA_SDHOST_CD_SW;
		sdhci_writel(host, val, KONA_SDHOST_CORESTAT);
	} else {
		val &= ~KONA_SDHOST_CD_SW;
		sdhci_writel(host, val, KONA_SDHOST_CORESTAT);
	}

	spin_unlock_irqrestore(&host->lock, flags);

	return 0;
}

/* Reads 1 when SD/MMC card is inserted, 0 otherwise */
static ssize_t card_ctrl_show(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	struct sdio_dev *sdio = dev_get_drvdata(dev);
	struct sdhci_host *host = sdio->host;

	return scnprintf(buf, PAGE_SIZE, "%d",
			sdhci_readl(host, KONA_SDHOST_CORESTAT)
			& KONA_SDHOST_CD_SW ? 1 : 0);
}

/* Write 1 emulates card insert, 0 emulate card remove */
static ssize_t card_ctrl_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct sdio_dev *sdio = dev_get_drvdata(dev);
	int insert;

	if (sscanf(buf, "%d", &insert) != 1) {
		pr_err("Error: Invalid argument.\n");
		pr_err("Usage: echo <insert> > %s, where <insert> is 1 or 0.\n",
		       PROC_ENTRY_CARD_CTRL);
		return -EINVAL;
	}

	if (insert) {
		bcm_kona_sd_card_emulate(sdio, 1);
		pr_info("Emulated card insert!\n");
	} else {
		bcm_kona_sd_card_emulate(sdio, 0);
		pr_info("Emulated card remove!\n");
	}

	return count;
}

static DEVICE_ATTR(card_ctrl, 0644, card_ctrl_show, card_ctrl_store);

/*
 * SD card detection interrupt handler
 */
static irqreturn_t sdhci_pltfm_cd_interrupt(int irq, void *dev_id)
{
	struct sdio_dev *dev = (struct sdio_dev *)dev_id;
	struct sdhci_host *host = dev->host;
	int gpio_status;

	wake_lock(&dev->cd_int_wake_lock);

	gpio_status = gpio_get_value_cansleep(dev->cd_gpio);

	/* If suspended, wait for the device to be resumed.
	 * It is important that we get the gpio status first
	 * and then go for sleep. Otherwise a state change can
	 * happen during the sleep, and we end up passing consecutive
	 * card insert or removal indications to uppper layer.
	 */
	while (dev->suspended)
		usleep_range(1000, 5000);

	/* card insert */
	if (!gpio_status) {

		/* Turn ON the SD controller regulator.
		 * The card regulator will be truned on
		 * during set_ios.
		 */
		kona_sdio_regulator_power(dev, 1);
		host->flags &= ~SDHCI_DEVICE_DEAD;
		bcm_kona_sd_card_emulate(dev, 1);
	} else {	/* card removal */

		/* Set the device as dead, so that
		 * set_ios will turn off the regulator.
		 */
		host->flags |= SDHCI_DEVICE_DEAD;
		bcm_kona_sd_card_emulate(dev, 0);
	}

	wake_unlock(&dev->cd_int_wake_lock);
	return IRQ_HANDLED;
}

static int sdhci_pltfm_clk_enable(struct sdio_dev *dev, int enable)
{
#if defined(CONFIG_ARCH_SAMOA) || defined(CONFIG_MACH_BCM_FPGA)
	return 0;
#else
	int ret = 0;

	BUG_ON(!dev);
	if (enable) {
#if defined(CONFIG_ARCH_JAVA)
		pause_nohz();
#endif
		/* peripheral clock */
		ret = clk_enable(dev->peri_clk);
		if (ret)
			return ret;
	} else {
		clk_disable(dev->peri_clk);
#if defined(CONFIG_ARCH_JAVA)
		resume_nohz();
#endif

	}
	return ret;
#endif
}

static int sdhci_clk_enable(struct sdhci_host *host, int enable)
{
	struct sdio_dev *dev = sdhci_priv(host);
	int ret = 0;

	ret = sdhci_pltfm_clk_enable(dev, enable);
	if (ret)
		dev_err(dev->dev,
			"clock %s failed\n", enable ? "enable" : "disable");
	return ret;
}

#ifdef CONFIG_BRCM_UNIFIED_DHD_SUPPORT
static void kona_sdio_status_notify_cb(int card_present, void *dev_id)
{
	struct sdhci_host *host;
	int rc;
	struct sdio_dev *dev;

	pr_debug("%s: ENTRY\n", __func__);

	rc = sdio_dev_is_initialized(SDIO_DEV_TYPE_WIFI);
	if (rc <= 0) {
		pr_err("%s: CARD IS NOT INITIALIZED\n", __func__);
		return;
	}
	dev = gDevs[SDIO_DEV_TYPE_WIFI];

	pr_debug("%s: DEV=%x\n", __func__, (unsigned int)dev);

	host = dev_id;
	if (host == NULL) {
		pr_err("%s: Invalid host structure pointer\n", __func__);
		return;
	}
	pr_debug("%s: CALL EMULATION=%x\n", __func__, (unsigned int)dev);
	if (card_present)
		bcm_kona_sd_card_emulate(dev, 1);
	else
		bcm_kona_sd_card_emulate(dev, 0);

	pr_debug("%s: EMULATION DONE=%x\n", __func__, (unsigned int)dev);
	/*
	 * TODO: The required implementtion to check the status of the card
	 * etc
	 */
}
#endif

#ifdef CONFIG_PM_RUNTIME

static int sdhci_pltfm_runtime_suspend(struct device *device)
{
	int ret = 0;
	unsigned long flags;
	struct sdio_dev *dev =
		platform_get_drvdata(to_platform_device(device));
	struct sdhci_host *host = dev->host;

	/* This is never going to happen, but still */
	if (!sdhci_pltfm_rpm_enabled(dev)) {
		dev_err(dev->dev,
			"Spurious rpm suspend call\n");
		/* But no meaning in returning error */
		return 0;
	}

	spin_lock_irqsave(&host->lock, flags);
	host->runtime_suspended = true;
	spin_unlock_irqrestore(&host->lock, flags);

	ret = sdhci_pltfm_clk_enable(dev, 0);
	if (ret) {
		dev_err(dev->dev,
			"Failed to disable clock during run time suspend\n");
		sdhci_runtime_resume_host(host);
		return -EAGAIN;
	}

	return 0;
}

static int sdhci_pltfm_runtime_resume(struct device *device)
{
	int ret = 0;
	unsigned long flags;
	struct sdio_dev *dev =
		platform_get_drvdata(to_platform_device(device));
	struct sdhci_host *host = dev->host;

	/* This is never going to happen, but still */
	if (!sdhci_pltfm_rpm_enabled(dev)) {
		dev_err(dev->dev, "Spurious rpm resume call\n");
		/* But no menaing in returning error */
		return 0;
	}

	spin_lock_irqsave(&host->lock, flags);
	host->runtime_suspended = false;
	spin_unlock_irqrestore(&host->lock, flags);

	ret = sdhci_pltfm_clk_enable(dev, 1);
	if (ret) {
		dev_err(dev->dev,
			"Failed to enable clock during run time resume\n");
		return -EAGAIN;
	}

	return 0;
}

static int sdhci_pltfm_runtime_idle(struct device *device)
{
	/*
	 * Make sure we return 0 here.
	 * When the device resume returns
	 * the pm_runtime_put_sync is called instead
	 * of pm_runtime_put_sync suspend, which means
	 * first the idle will be called (this function).
	 * If idle returns zero, the runtime suspend
	 * will be initiated, otherwise not.
	 */
	return 0;
}

static void sdhci_pltfm_runtime_pm_init(struct device *device)
{
	struct sdio_dev *dev =
		platform_get_drvdata(to_platform_device(device));

	if (!sdhci_pltfm_rpm_enabled(dev))
		return;

	pm_runtime_irq_safe(device);
	pm_runtime_enable(device);

	if (dev->devtype == SDIO_DEV_TYPE_WIFI)
		pm_runtime_set_autosuspend_delay(device,
				KONA_MMC_WIFI_AUTOSUSPEND_DELAY);
	else
		pm_runtime_set_autosuspend_delay(device,
				KONA_MMC_AUTOSUSPEND_DELAY);

	pm_runtime_use_autosuspend(device);
}

static void sdhci_pltfm_runtime_pm_forbid(struct device *device)
{
	struct sdio_dev *dev =
		platform_get_drvdata(to_platform_device(device));

	if (!sdhci_pltfm_rpm_enabled(dev))
		return;

	pm_runtime_forbid(device);
	pm_runtime_get_noresume(device);
	pm_runtime_disable(device);
}

#else

#define sdhci_pltfm_runtime_suspend      NULL
#define sdhci_pltfm_runtime_resume       NULL
#define sdhci_pltfm_runtime_idle         NULL

#endif

irqreturn_t cd_irq_handler(int irq, void *ptr)
{
	return IRQ_WAKE_THREAD;
}

static int sdhci_pltfm_probe(struct platform_device *pdev)
{
	struct sdhci_host *host;
	struct sdio_dev *dev;
	struct resource *iomem;
	struct sdio_platform_cfg *hw_cfg = NULL;
	char devname[MAX_DEV_NAME_SIZE];
	int ret = 0;
	char *emmc_regulator = NULL;

	pr_debug("%s: ENTRY\n", __func__);

	BUG_ON(pdev == NULL);

	hw_cfg = (struct sdio_platform_cfg *)pdev->dev.platform_data;
	if (pdev->dev.of_node) {
		u32 val;
		const char *prop;
		if (!pdev->dev.platform_data)
			hw_cfg = kzalloc(sizeof(struct sdio_platform_cfg),
				GFP_KERNEL);

		if (!hw_cfg) {
			dev_err(&pdev->dev,
				"unable to allocate mem for private data\n");
			ret = -ENOMEM;
			goto err;
		}

		if (of_property_read_u32(pdev->dev.of_node, "id", &val)) {
			dev_err(&pdev->dev, "id read failed in %s\n", __func__);
			goto err_free_priv_data_mem;
		}

		hw_cfg->id = val;
		pdev->id = val;

		if (of_property_read_u32(pdev->dev.of_node, "data-pullup",
			&val)) {
			dev_err(&pdev->dev, "data-pullup read failed in %s\n",
			__func__);
			goto err_free_priv_data_mem;
		}

		hw_cfg->data_pullup = val;

		if (of_property_read_u32(pdev->dev.of_node, "devtype", &val)) {
			dev_err(&pdev->dev, "devtype read failed in %s\n",
			__func__);
			goto err_free_priv_data_mem;
		}

		hw_cfg->devtype = val;

		if (of_property_read_u32(pdev->dev.of_node, "flags", &val)) {
			dev_err(&pdev->dev, "flags read failed in %s\n",
			__func__);
			goto err_free_priv_data_mem;
		}

		hw_cfg->flags = val;

		if (of_property_read_u32(pdev->dev.of_node, "quirks", &val)) {
			dev_warn(&pdev->dev, "No quirks found via DT in %s\n",
			__func__);
			val = 0;
		}

		hw_cfg->quirks = val;

		if (of_property_read_u32(pdev->dev.of_node, "quirks2", &val)) {
			dev_warn(&pdev->dev, "No quirks2 found via DT in %s\n",
			__func__);
			val = 0;
		}

		hw_cfg->quirks2 = val;

		if (of_property_read_u32(pdev->dev.of_node, "pm_caps", &val)) {
			dev_warn(&pdev->dev, "No pm_caps found via DT in %s\n",
			__func__);
			val = 0;
		}

		hw_cfg->pm_caps = val;

		if (of_property_read_string(pdev->dev.of_node, "peri-clk-name",
			&prop)) {
			dev_err(&pdev->dev, "peri-clk-name read failed in %s\n",
			__func__);
			goto err_free_priv_data_mem;
		}

		hw_cfg->peri_clk_name = (char *)prop;

		if (of_property_read_string(pdev->dev.of_node, "ahb-clk-name",
			&prop)) {
			dev_err(&pdev->dev, "ahb-clk-name read failed in %s\n",
			__func__);
			goto err_free_priv_data_mem;
		}

		hw_cfg->ahb_clk_name = (char *)prop;

		if (of_property_read_string(pdev->dev.of_node, "sleep-clk-name",
			&prop)) {
			dev_err(&pdev->dev, "sleep-clk-name read failed in %s\n",
			__func__);
			goto err_free_priv_data_mem;
		}

		hw_cfg->sleep_clk_name = (char *)prop;

		if (of_property_read_u32(pdev->dev.of_node, "peri-clk-rate",
			&val)) {
			dev_err(&pdev->dev, "peri-clk-rate read failed in %s\n",
			__func__);
			goto err_free_priv_data_mem;
		}

		hw_cfg->peri_clk_rate = val;

		if (hw_cfg->devtype == SDIO_DEV_TYPE_SDMMC) {
			if (of_property_read_string(pdev->dev.of_node,
				"vddo-regulator-name", &prop)) {
				dev_err(&pdev->dev, "vddo-regulator-name read "\
				"failed in %s\n", __func__);
				goto err_free_priv_data_mem;
			}

			hw_cfg->vddo_regulator_name = (char *)prop;

			if (of_property_read_string(pdev->dev.of_node,
				"vddsdxc-regulator-name", &prop)) {
				dev_err(&pdev->dev, "vddsdxc-regulator-name"\
				"read failed in %s\n", __func__);
				goto err_free_priv_data_mem;
			}

			hw_cfg->vddsdxc_regulator_name = (char *)prop;

			hw_cfg->cd_gpio = of_get_named_gpio(pdev->dev.of_node,
			"cd-gpio", 0);
			if (!gpio_is_valid(hw_cfg->cd_gpio)) {
				dev_err(&pdev->dev,
				"%s: ERROR setting -1 to cd-gpio\n",
				__func__);
				hw_cfg->cd_gpio = -1;
			}
		}

		else if (hw_cfg->devtype == SDIO_DEV_TYPE_EMMC) {

			if (of_property_read_u32(pdev->dev.of_node,
				"is-8bit", &val)) {
				dev_err(&pdev->dev, "is-8bit read failed in %s\n",
				__func__);
				goto err_free_priv_data_mem;
			}

			hw_cfg->is_8bit = val;
			if (!(of_property_read_string(pdev->dev.of_node,
					"vddsdmmc-regulator-name", &prop)))
				emmc_regulator = (char *)prop;
		}

		pdev->dev.platform_data = hw_cfg;
	}
	if (!hw_cfg) {
			dev_err(&pdev->dev, "hw_cfg is NULL\n");
			ret = -ENOMEM;
			goto err;
	}

	if (hw_cfg->devtype >= SDIO_DEV_TYPE_MAX) {
		dev_err(&pdev->dev, "unknown device type\n");
		ret = -EFAULT;
		goto err;
	}
	pr_debug("%s: GET PLATFORM RESOURCES\n", __func__);

	iomem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!iomem) {
		ret = -ENOMEM;
		goto err;
	}

	/* Some PCI-based MFD need the parent here */
	if (pdev->dev.parent != &platform_bus)
		host =
		    sdhci_alloc_host(pdev->dev.parent, sizeof(struct sdio_dev));
	else
		host = sdhci_alloc_host(&pdev->dev, sizeof(struct sdio_dev));
	if (IS_ERR(host)) {
		ret = PTR_ERR(host);
		goto err;
	}

	pr_debug("%s: ALLOC HOST\n", __func__);

	host->hw_name = "bcm_kona_sd";
	host->ops = &sdhci_pltfm_ops;
	host->irq = platform_get_irq(pdev, 0);
	host->quirks = SDHCI_QUIRK_NO_CARD_NO_RESET
	    | SDHCI_QUIRK_BROKEN_TIMEOUT_VAL
	    | SDHCI_QUIRK_32BIT_DMA_ADDR
	    | SDHCI_QUIRK_32BIT_DMA_SIZE | SDHCI_QUIRK_32BIT_ADMA_SIZE;

#ifdef CONFIG_MACH_RHEA_DALTON2_EB30
        host->quirks |= SDHCI_QUIRK_NO_MULTIBLOCK;
#endif
	host->quirks |= hw_cfg->quirks;
	host->quirks2 |= hw_cfg->quirks2;

        pr_debug("%s: GET IRQ\n", __func__);

	host->mmc->caps |= MMC_CAP_WAIT_WHILE_BUSY;

	if (hw_cfg->flags & KONA_SDIO_FLAGS_DEVICE_NON_REMOVABLE)
		host->mmc->caps |= MMC_CAP_NONREMOVABLE;

	if (!request_mem_region(iomem->start, resource_size(iomem),
				mmc_hostname(host->mmc))) {
		dev_err(&pdev->dev, "cannot request region\n");
		ret = -EBUSY;
		goto err_free_host;
	}

	host->ioaddr = ioremap(iomem->start, resource_size(iomem));
	if (!host->ioaddr) {
		dev_err(&pdev->dev, "failed to remap registers\n");
		ret = -ENOMEM;
		goto err_free_mem_region;
	}

	pr_debug("%s: MEM and IO REGION OKAY\n", __func__);

	dev = sdhci_priv(host);
	dev->dev = &pdev->dev;
	dev->host = host;
	dev->devtype = hw_cfg->devtype;
	dev->cd_gpio = hw_cfg->cd_gpio;
	host->mmc->parent = dev->dev;
	if (dev->devtype == SDIO_DEV_TYPE_WIFI)
		dev->wifi_gpio = &hw_cfg->wifi_gpio;
	if (dev->devtype == SDIO_DEV_TYPE_EMMC && emmc_regulator) {
		dev->vdd_sdxc_regulator = regulator_get(NULL, emmc_regulator);
		if (IS_ERR(dev->vdd_sdxc_regulator)) {
			dev->vdd_sdxc_regulator = NULL;
		}
	}

	if (dev->devtype == SDIO_DEV_TYPE_EMMC)
		host->detect_delay = 0;
	else
		host->detect_delay = 200;

	pr_debug("%s: DEV TYPE %x\n", __func__, dev->devtype);

	gDevs[dev->devtype] = dev;

	platform_set_drvdata(pdev, dev);

	snprintf(devname, sizeof(devname), "%s%d", DEV_NAME, pdev->id);

	/* enable clocks */
#ifdef CONFIG_MACH_BCM2850_FPGA
	if (clock) {		/* clock override */
		dev->clk_hz = clock;
	} else {
		dev->clk_hz = gClock[dev->devtype];
	}
#elif defined(CONFIG_MACH_BCM_FPGA)
	dev->clk_hz = hw_cfg->peri_clk_rate;
#else
	/* peripheral clock */
	dev->peri_clk = clk_get(&pdev->dev, hw_cfg->peri_clk_name);
	if (IS_ERR_OR_NULL(dev->peri_clk)) {
		ret = -EINVAL;
		goto err_unset_pltfm;
	}
	ret = clk_set_rate(dev->peri_clk, hw_cfg->peri_clk_rate);
	if (ret)
		goto err_peri_clk_put;

	/* sleep clock */
	dev->sleep_clk = clk_get(&pdev->dev, hw_cfg->sleep_clk_name);
	if (IS_ERR_OR_NULL(dev->sleep_clk)) {
		ret = -EINVAL;
		goto err_peri_clk_put;
	}

	ret = clk_enable(dev->sleep_clk);
	if (ret) {
		dev_err(&pdev->dev, "failed to enable sleep clock for %s\n",
			devname);
		goto err_sleep_clk_put;
	}

	ret = sdhci_pltfm_clk_enable(dev, 1);
	if (ret) {
		dev_err(&pdev->dev, "failed to initialize core clock for %s\n",
			devname);
		goto err_sleep_clk_disable;
	}

	dev->clk_hz = clk_get_rate(dev->peri_clk);
#endif

	dev->suspended = 0;

	if (hw_cfg->vddo_regulator_name) {
		ret =
		    sdhci_pltfm_regulator_init(dev,
					       hw_cfg->vddo_regulator_name);
#ifndef BCM_REGULATOR_SKIP_QUIRK
		if (ret < 0)
			goto err_term_clk;
#endif
	}

	if (hw_cfg->vddsdxc_regulator_name &&
			dev->devtype == SDIO_DEV_TYPE_SDMMC) {
		ret =
		    sdhci_pltfm_regulator_sdxc_init(dev,
					       hw_cfg->vddsdxc_regulator_name);
#ifndef BCM_REGULATOR_SKIP_QUIRK
		if (ret < 0)
			goto err_term_clk;
#endif
	}

	mutex_init(&dev->regulator_lock);
	kona_sdio_regulator_power(dev, 1);

	ret = bcm_kona_sd_reset(dev);
	if (ret)
		goto err_term_clk;

	ret = bcm_kona_sd_init(dev);
	if (ret)
		goto err_reset;

	if (hw_cfg->is_8bit)
		host->mmc->caps |= MMC_CAP_8_BIT_DATA;

	/* Note that sdhci_add_host calls --> mmc_add_host, which in turn
	 * checks for the flag MMC_PM_IGNORE_PM_NOTIFY before registering a PM
	 * notifier for the specific instance of SDIO host controller. For
	 * WiFi case, we don't want to get notified, becuase then from there
	 * mmc_power_off is called which will reset the Host registers that
	 * needs to be re-programmed by starting SDIO handsake again. We want
	 * to prevent this in case of WiFi. So enable MMC_PM_IGNORE_PM_NOTIFY
	 * flag, so that notifier never gets registered.
	 */
	if (dev->devtype == SDIO_DEV_TYPE_WIFI) {
		/* The Wireless LAN drivers call the API sdio_get_host_pm_caps
		 * to know the PM capabilities of the driver, which would
		 * return pm_caps. While the internal code decides based on
		 * pm_flags, the pm_caps also should reflect the same.
		 */
		host->mmc->pm_caps =
		    MMC_PM_KEEP_POWER | MMC_PM_IGNORE_PM_NOTIFY;
		host->mmc->pm_flags =
		    MMC_PM_KEEP_POWER | MMC_PM_IGNORE_PM_NOTIFY;
	}

	host->mmc->pm_caps |= hw_cfg->pm_caps;

#if !defined(CONFIG_MACH_BCM_FPGA_E)
	/* Enable 1.8V DDR operation for e.MMC */
	if (dev->devtype == SDIO_DEV_TYPE_EMMC)
		host->mmc->caps |= MMC_CAP_1_8V_DDR;
#endif

	dev->runtime_pm_enabled = hw_cfg->flags & KONA_SDIO_FLAGS_DEVICE_RPM_EN;
	sdhci_pltfm_runtime_pm_init(dev->dev);

	ret = sdhci_add_host(host);
	if (ret)
		goto err_reset;

	ret = device_create_file(&pdev->dev, &dev_attr_card_ctrl);
	if (ret)
		goto err_rm_host;

	if (dev->devtype == SDIO_DEV_TYPE_SDMMC) {
		/* support SD card detect interrupts for insert/removal */
		host->mmc->card_detect_cap = true;
	}

	/* if device is eMMC, emulate card insert right here */
	if (dev->devtype == SDIO_DEV_TYPE_EMMC) {
		ret = bcm_kona_sd_card_emulate(dev, 1);
		if (ret) {
			dev_err(&pdev->dev,
				"unable to emulate card insertion\n");
			goto err_rm_sysfs;
		}
		pr_info("%s: card insert emulated!\n", devname);
	} else if (dev->devtype == SDIO_DEV_TYPE_SDMMC && dev->cd_gpio >= 0) {

		dev->cd_int_wake_lock_name = kasprintf(GFP_KERNEL,
				"%s_cd_int", devname);

		if (!dev->cd_int_wake_lock_name) {
			dev_err(&pdev->dev,
				"error allocating mem for wake_lock_name\n");
			goto err_rm_sysfs;
		}

		wake_lock_init(&dev->cd_int_wake_lock, WAKE_LOCK_SUSPEND,
				dev->cd_int_wake_lock_name);


		ret = gpio_request(dev->cd_gpio, "sdio cd");

		if (ret < 0) {
			dev_err(&pdev->dev, "Unable to request GPIO pin %d\n",
				dev->cd_gpio);
			goto err_rm_sysfs;
		}
		gpio_direction_input(dev->cd_gpio);

		ret = request_threaded_irq(gpio_to_irq(dev->cd_gpio),
					   cd_irq_handler,
					   sdhci_pltfm_cd_interrupt,
					   IRQF_TRIGGER_FALLING |
					   IRQF_TRIGGER_RISING  |
					   IRQF_NO_SUSPEND, "sdio cd", dev);
		if (ret) {
			dev_err(&pdev->dev,
				"Unable to request card detection irq=%d"
				" for gpio=%d\n",
				gpio_to_irq(dev->cd_gpio), dev->cd_gpio);
			goto err_free_cd_gpio;
		}

		/* Set debounce for SD Card detect to maximum value (128ms)
		 *
		 * NOTE-1: If gpio_set_debounce() returns error we still
		 * continue with the default debounce value set. Another reason
		 * for doing this is that on rhea-ray boards the SD Detect GPIO
		 * is on GPIO Expander and gpio_set_debounce() will return error
		 * and if we return error from here, then probe() would fail and
		 * SD detection would always fail.
		 *
		 * NOTE-2: We also give a msleep() of the "debounce" time here
		 * so that we give enough time for the debounce to stabilize
		 * before we read the gpio value in gpio_get_value_cansleep().
		 */
		ret =
		    gpio_set_debounce(dev->cd_gpio,
				      (SD_DETECT_GPIO_DEBOUNCE_128MS * 1000));
		if (ret < 0) {
			dev_err(&pdev->dev, "%s: gpio set debounce failed."
				"default debounce value assumed\n", __func__);
		}

		/* Sleep for 128ms to allow debounce to stabilize */
		msleep(SD_DETECT_GPIO_DEBOUNCE_128MS);

		/*
		 * Since the card detection GPIO interrupt is configured to be
		 * edge sensitive, check the initial GPIO value here, emulate
		 * only if the card is present
		 */
		if (gpio_get_value_cansleep(dev->cd_gpio) == 0)
			bcm_kona_sd_card_emulate(dev, 1);
		else
			/* If card is not present disable the regulator */
			kona_sdio_regulator_power(dev, 0);
	}

	/* Force insertion interrupt, in case of no card detect registered.
	 */
	if (dev->cd_gpio < 0)
		bcm_kona_sd_card_emulate(dev, 1);
#ifdef CONFIG_BRCM_UNIFIED_DHD_SUPPORT
	if ((dev->devtype == SDIO_DEV_TYPE_WIFI) &&
	    (hw_cfg->register_status_notify != NULL)) {
		hw_cfg->register_status_notify(kona_sdio_status_notify_cb,
					       host);
	}
	pr_debug("%s: CALL BACK IS REGISTERED\n", __func__);

#endif

	atomic_set(&dev->initialized, 1);
	sdhci_pltfm_clk_enable(dev, 0);

	pr_info("%s: initialized properly\n", devname);

	return 0;

err_free_cd_gpio:
	if (dev->devtype == SDIO_DEV_TYPE_SDMMC && dev->cd_gpio >= 0)
		gpio_free(dev->cd_gpio);

err_rm_sysfs:
	device_remove_file(&pdev->dev, &dev_attr_card_ctrl);

err_rm_host:
	sdhci_remove_host(host, 0);

err_reset:
	bcm_kona_sd_reset(dev);

err_term_clk:
	sdhci_pltfm_clk_enable(dev, 0);

#if !defined(CONFIG_MACH_BCM2850_FPGA) && !defined(CONFIG_MACH_BCM_FPGA)
err_sleep_clk_disable:
	clk_disable(dev->sleep_clk);

err_sleep_clk_put:
	clk_put(dev->sleep_clk);

err_peri_clk_put:
	clk_put(dev->peri_clk);

err_unset_pltfm:
	platform_set_drvdata(pdev, NULL);
	iounmap(host->ioaddr);
#endif

err_free_mem_region:
	release_mem_region(iomem->start, resource_size(iomem));

err_free_host:
	sdhci_free_host(host);

err_free_priv_data_mem:
	if (pdev->dev.of_node) {
		ret = -EFAULT;
		kfree(hw_cfg);
	}
err:
	pr_err("Probing of sdhci-pltfm %d failed: %d\n", pdev->id,
	       ret);
	return ret;
}

static int sdhci_pltfm_remove(struct platform_device *pdev)
{
	struct sdio_dev *dev = platform_get_drvdata(pdev);
	struct sdhci_host *host = dev->host;
	struct resource *iomem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	int dead;
	u32 scratch;
	int ret = 0;

	atomic_set(&dev->initialized, 0);
	gDevs[dev->devtype] = NULL;

	if (dev->devtype == SDIO_DEV_TYPE_SDMMC && dev->cd_gpio >= 0) {
		free_irq(gpio_to_irq(dev->cd_gpio), dev);
		gpio_free(dev->cd_gpio);
	}

	if (dev->vdd_sdxc_regulator && dev->devtype == SDIO_DEV_TYPE_SDMMC) {
		/* Playing safe- if regulator is enabled, disable it first */
		if (regulator_is_enabled(dev->vdd_sdxc_regulator) > 0)
			regulator_disable(dev->vdd_sdxc_regulator);

		regulator_put(dev->vdd_sdxc_regulator);
	}

	device_remove_file(&pdev->dev, &dev_attr_card_ctrl);

	if (sdhci_pltfm_rpm_enabled(dev)) {
		pm_runtime_get_sync(dev->dev);
	} else {
		ret = sdhci_pltfm_clk_enable(dev, 1);
		if (ret)
			dev_err(dev->dev,
				"enable clock during pltfm remove failed\n");
	}

	dead = 0;
	scratch = readl(host->ioaddr + SDHCI_INT_STATUS);
	if (scratch == (u32)-1)
		dead = 1;
	sdhci_remove_host(host, dead);

	if (sdhci_pltfm_rpm_enabled(dev)) {
		pm_runtime_put_sync_suspend(dev->dev);
	} else {
		ret = sdhci_pltfm_clk_enable(dev, 0);
		if (ret)
			dev_err(dev->dev,
				"disable clock during pltfm remove failed\n");
	}

#if !defined(CONFIG_MACH_BCM2850_FPGA) && !defined(CONFIG_MACH_BCM_FPGA)
	clk_disable(dev->sleep_clk);
	clk_put(dev->sleep_clk);
	clk_put(dev->peri_clk);
#endif

	sdhci_pltfm_runtime_pm_forbid(dev->dev);
	kfree(dev->cd_int_wake_lock_name);
	wake_lock_destroy(&dev->cd_int_wake_lock);
	platform_set_drvdata(pdev, NULL);
	if (dev->devtype == SDIO_DEV_TYPE_EMMC ||
		dev->devtype == SDIO_DEV_TYPE_SDMMC) {
		kfree(pdev->dev.platform_data);
		pdev->dev.platform_data = NULL;
	}
	kfree(dev);
	iounmap(host->ioaddr);
	release_mem_region(iomem->start, resource_size(iomem));
	sdhci_free_host(host);

	return 0;
}

#ifdef CONFIG_PM
static int sdhci_pltfm_suspend(struct device *device)
{
	struct sdio_dev *dev =
		platform_get_drvdata(to_platform_device(device));
	struct sdhci_host *host = dev->host;
	int ret = 0;

	if (!sdhci_pltfm_rpm_enabled(dev)) {
		ret = sdhci_pltfm_clk_enable(dev, 1);
		if (ret) {
			dev_err(dev->dev,
				"Failed to enable clock during suspend\n");
			return -EAGAIN;
		}
	}

	host->mmc->pm_flags |= host->mmc->pm_caps;

	ret = sdhci_suspend_host(host);
	if (ret) {
		dev_err(dev->dev, "Unable to suspend sdhci host err=%d\n",
			ret);
		return ret;
	}

	if (sdhci_pltfm_rpm_enabled(dev)) {
		/*
		 * Note that we havent done a get_sync. The
		 * pm core takes care of that.
		 */
		pm_runtime_put_sync_suspend(dev->dev);
	} else {
		ret = sdhci_pltfm_clk_enable(dev, 0);
		if (ret) {
			dev_err(dev->dev,
				"Failed to disable clock during suspend\n");
			/* Not really a big error to cry and return */
		}
	}

	dev->suspended = 1;
	return 0;
}

static int sdhci_pltfm_resume(struct device *device)
{
	struct sdio_dev *dev =
		platform_get_drvdata(to_platform_device(device));
	struct sdhci_host *host = dev->host;
	int ret = 0;

	if (sdhci_pltfm_rpm_enabled(dev)) {
		/*
		 * Note that we havent done a put_sync. The
		 * pm core takes care of that.
		 */
		pm_runtime_get_sync(dev->dev);
	} else {
		ret = sdhci_pltfm_clk_enable(dev, 1);
		if (ret) {
			dev_err(dev->dev,
				"Failed to enable clock during resume\n");
			return -EAGAIN;
		}
	}

	ret = sdhci_resume_host(host);
	if (ret) {
		dev_err(dev->dev,
		 "Unable to resume sdhci host err=%d\n", ret);
		return ret;
	}

	if (!sdhci_pltfm_rpm_enabled(dev)) {
		ret = sdhci_pltfm_clk_enable(dev, 0);
		if (ret) {
			dev_err(dev->dev,
				"Failed to disable clock during resume\n");
			/* Not really a big error to cry and return*/
		}
	}

	dev->suspended = 0;
	return 0;
}
#else
#define sdhci_pltfm_suspend NULL
#define sdhci_pltfm_resume NULL
#endif /* CONFIG_PM */

static inline int sdhci_pltfm_rpm_enabled(struct sdio_dev *dev)
{
	return dev->runtime_pm_enabled;
}

static int sdhci_rpm_enabled(struct sdhci_host *host)
{
	struct sdio_dev *dev = sdhci_priv(host);

	return sdhci_pltfm_rpm_enabled(dev);
}

static const struct dev_pm_ops sdhci_pltfm_pm_ops = {
	.runtime_suspend = sdhci_pltfm_runtime_suspend,
	.runtime_resume = sdhci_pltfm_runtime_resume,
	.runtime_idle = sdhci_pltfm_runtime_idle,
	.suspend = sdhci_pltfm_suspend,
	.resume = sdhci_pltfm_resume,
};

static const struct of_device_id sdhci_of_match[] = {
	{ .compatible = "bcm,sdhci", },
	{},
}

MODULE_DEVICE_TABLE(of, sdhci_of_match)
;
static struct platform_driver sdhci_pltfm_driver = {
	.driver = {
		   .name = "sdhci",
		   .owner = THIS_MODULE,
		   .pm = &sdhci_pltfm_pm_ops,
		   .of_match_table = sdhci_of_match,
		   },
	.probe = sdhci_pltfm_probe,
	.remove = sdhci_pltfm_remove,
};

static int __init sdhci_drv_init(void)
{
	int rc;

	rc = platform_driver_register(&sdhci_pltfm_driver);
	if (rc < 0) {
		pr_err("%s: sdhci_drv_init failed\n", __func__);
		return rc;
	}

	return 0;
}

static void __exit sdhci_drv_exit(void)
{
	platform_driver_unregister(&sdhci_pltfm_driver);
}

fs_initcall(sdhci_drv_init);
module_exit(sdhci_drv_exit);

MODULE_DESCRIPTION("SDHCI platform driver for KONA architecture");
MODULE_AUTHOR("Broadcom Corporation");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:sdhci");

int sdio_dev_is_initialized(enum sdio_devtype devtype)
{
	struct sdio_dev *dev;

	if (devtype >= SDIO_DEV_TYPE_MAX)
		return -EINVAL;

	if (gDevs[devtype] == NULL)
		return 0;

	dev = gDevs[devtype];

	return atomic_read(&dev->initialized);
}
EXPORT_SYMBOL(sdio_dev_is_initialized);

struct mmc_card *sdio_get_mmc_card(enum sdio_devtype devtype)
{
	int rc;
	struct sdio_dev *dev;

	rc = sdio_dev_is_initialized(devtype);
	if (rc <= 0)
		return NULL;

	dev = gDevs[devtype];
	return dev->host->mmc->card;
}
EXPORT_SYMBOL(sdio_get_mmc_card);

struct sdio_wifi_gpio_cfg *sdio_get_wifi_gpio(enum sdio_devtype devtype)
{
	int rc;
	struct sdio_dev *dev;

	rc = sdio_dev_is_initialized(devtype);
	if (rc <= 0)
		return NULL;

	dev = gDevs[devtype];

	return dev->wifi_gpio;
}
EXPORT_SYMBOL(sdio_get_wifi_gpio);

int sdio_card_emulate(enum sdio_devtype devtype, int insert)
{
	int rc;
	struct sdio_dev *dev;

	rc = sdio_dev_is_initialized(devtype);
	if (rc <= 0)
		return -EFAULT;

	dev = gDevs[devtype];

	return bcm_kona_sd_card_emulate(dev, insert);
}
EXPORT_SYMBOL(sdio_card_emulate);

static int sdhci_pltfm_regulator_sdxc_init(struct sdio_dev *dev, char *reg_name)
{
	int ret;

	dev->vdd_sdxc_regulator = regulator_get(NULL, reg_name);

	if (!IS_ERR(dev->vdd_sdxc_regulator)) {
		/* Configure 3.0V default */
		ret = regulator_set_voltage(dev->vdd_sdxc_regulator,
						3000000, 3000000);
		if (ret < 0) {
			pr_err("%s: can't set 3.0V\n",
				   reg_name);
			ret = -1;
		} else {
			pr_debug("%s: set to 3.0V\n",
				reg_name);
			ret = 0;
		}
	} else {
		pr_err("%s: could not get sdxc regulator\n", reg_name);
		/* We have found out the errno and need not do IS_ERR
		 * in future.
		 */
		dev->vdd_sdxc_regulator = NULL;
		ret = -1;
	}
	return ret;
}

static int sdhci_pltfm_regulator_init(struct sdio_dev *dev, char *reg_name)
{
	int ret;

	dev->vddo_sd_regulator = regulator_get(NULL, reg_name);

	if (!IS_ERR(dev->vddo_sd_regulator)) {
		/* Configure 3.0V default */
		ret = regulator_set_voltage(dev->vddo_sd_regulator,
						3000000, 3000000);
		if (ret < 0) {
			pr_err("%s: can't set 3.0V\n",
				   reg_name);
			ret = -1;
		} else {
			pr_debug("%s: set to 3.0V\n",
				reg_name);
			ret = 0;
		}
	} else {
		pr_err("%s: could not get sd card regulator\n", reg_name);
		/* We have found out the errno and need not do IS_ERR
		 * in future.
		 */
		dev->vddo_sd_regulator = NULL;
		ret = -1;
	}
	return ret;
}

static int sdhci_pltfm_set_signalling(struct sdhci_host *host, int sig_vol)
{
	if (sig_vol == MMC_SIGNAL_VOLTAGE_330)
		return sdhci_pltfm_set_3v3_signalling(host);
	else if (sig_vol == MMC_SIGNAL_VOLTAGE_180)
		return sdhci_pltfm_set_1v8_signalling(host);
	else
		return -ENOSYS;
}

static int sdhci_pltfm_set_3v3_signalling(struct sdhci_host *host)
{
	struct sdio_dev *dev = sdhci_priv(host);
	int ret = 0;

	if (dev->vdd_sdxc_regulator &&
			dev->devtype == SDIO_DEV_TYPE_SDMMC) {
		ret =
		    regulator_set_voltage(dev->vdd_sdxc_regulator, 3000000,
					  3000000);
		if (ret < 0)
			dev_err(dev->dev, "cant set vddo regulator to 3.0V!\n");
		else
			dev_dbg(dev->dev, "vddo regulator is set to 3.0V\n");
	}
	return ret;
}

static int sdhci_pltfm_set_1v8_signalling(struct sdhci_host *host)
{
	struct sdio_dev *dev = sdhci_priv(host);
	int ret = 0;

	if (dev->vdd_sdxc_regulator &&
			dev->devtype == SDIO_DEV_TYPE_SDMMC) {
		ret =
		    regulator_set_voltage(dev->vdd_sdxc_regulator, 1800000,
					  1800000);
		if (ret < 0)
			dev_err(dev->dev, "Cant set vddo regulator to 1.8V!\n");
		else
			dev_dbg(dev->dev, "vddo regulator is set to 1.8V\n");
	}
	return ret;
}

static int
sdhci_set_regulator_power(struct sdhci_host *host, int power_state)
{
	struct sdio_dev *dev = sdhci_priv(host);

	return kona_sdio_regulator_power(dev, power_state);
}

static int
kona_sdio_regulator_power(struct sdio_dev *dev, int power_state)
{
	int ret = 0;
	struct device *pdev = dev->dev;
	struct sdio_platform_cfg *hw_cfg =
		(struct sdio_platform_cfg *)pdev->platform_data;

	mutex_lock(&dev->regulator_lock);
	/*
	 * Note that from the board file the appropriate regualtor names are
	 * populated. For example, in SD Card case there are two regulators to
	 * control
	 * vddo - That controls the power to the external card
	 * vddsdxc - That controls the power to the IO lines
	 * For the interfaces used for eMMC and WLAN only vddo is present.
	 * The understanding is that,  if for some intefaces like WLAN if the
	 * regulator need not be switched OFF then from the board file do not
	 * populate the regulator names.
	 */
	if (dev->devtype == SDIO_DEV_TYPE_SDMMC && dev->vddo_sd_regulator &&
			dev->vdd_sdxc_regulator) {
		if (power_state && !dev->sdio_regulator_enable) {
			dev_dbg(dev->dev, "Turning ON sdldo and sdxldo\r\n");
			ret = regulator_enable(dev->vddo_sd_regulator);
			if (ret) {
				pr_err("FAIL:regulator_enable(ldo): %d\n", ret);
				goto end;
			}

			ret = regulator_enable(dev->vdd_sdxc_regulator);
			if (ret) {
				pr_err("FAIL:regulator_enable(xldo):%d\n", ret);
				goto end;
			}

			mdelay(2);
			if (hw_cfg->configure_sdio_pullup) {
				dev_dbg(dev->dev, "Pull Up CMD/DAT Line\n");
				/* Pull-up SDCMD, SDDAT[0:3] */
				hw_cfg->configure_sdio_pullup(1);
				mdelay(2);
			}
			dev->sdio_regulator_enable = 1;
		} else if (!power_state && dev->sdio_regulator_enable) {
			dev_dbg(dev->dev, "Turning OFF sdldo and sdxldo\r\n");
			ret = regulator_disable(dev->vddo_sd_regulator);
			if (ret) {
				pr_err("FAIL:regulator_disable(ldo):%d\n", ret);
				goto end;
			}
			if (hw_cfg->configure_sdio_pullup) {
				dev_dbg(dev->dev, "Pull down CMD/DAT Line\n");
				/* Pull-down SDCMD, SDDAT[0:3] */
				hw_cfg->configure_sdio_pullup(0);
			}
			ret = regulator_disable(dev->vdd_sdxc_regulator);
			if (ret) {
				pr_err("FAIL:regulator_disable(xldo)%d\n", ret);
				goto end;
			}
			dev->sdio_regulator_enable = 0;
		}
	}
end:	mutex_unlock(&dev->regulator_lock);

	return ret;
}

#if !defined(CONFIG_WIFI_CONTROL_FUNC)
/*********************************************************
 * Description:
 * This function is used to return the host wake irq needed
 * by WLAN driver
 *
 * Input:
 * irq_flags_ptr (not used)
 *
 * Returns:
 * IRQ number corresponding to the host wake int, or
 * error code if this is not set
 *
 */

int wifi_get_irq_number(unsigned long *irq_flags_ptr)
{
	int rc;
	struct sdio_wifi_gpio_cfg *wifi_gpio_ptr;
	int host_wake_irq_num;

	rc = sdio_dev_is_initialized(SDIO_DEV_TYPE_WIFI);
	if (rc < 0)
		return rc;

	wifi_gpio_ptr = sdio_get_wifi_gpio(SDIO_DEV_TYPE_WIFI);
	if (wifi_gpio_ptr == NULL) {
		pr_err("Wifi GPIO not allocated!\n");
		return -EINVAL;

	}

	host_wake_irq_num = gpio_to_irq(wifi_gpio_ptr->host_wake);

	return host_wake_irq_num;

}
EXPORT_SYMBOL(wifi_get_irq_number);
#endif

static void sdhci_pltfm_init_74_clocks(struct sdhci_host *host, u8 power_mode)
{
	if (power_mode == MMC_POWER_OFF)
		return;
	else
	/*For 74 clocks the worst case delay is 740usecs
	  considering 100kHz clock*/
		mdelay(1);
}

static void sdhci_print_critical(struct sdhci_host *host)
{
	struct sdio_dev *dev = sdhci_priv(host);
	int ret = 0;

	if (dev->vdd_sdxc_regulator) {
		ret = irqsafe_is_regulator_enable(dev->vdd_sdxc_regulator);
		printk(KERN_ALERT "regulator enable:%d\n", ret);
	}
	if (dev->vddo_sd_regulator) {
		ret = irqsafe_is_regulator_enable(dev->vddo_sd_regulator);
		printk(KERN_ALERT "sd regulator enable:%d\n", ret);
	}
	printk(KERN_ALERT "runtime_suspended:%d\n", host->runtime_suspended);
	ret = atomic_read(&dev->dev->power.usage_count);
	printk(KERN_ALERT "pm runtime usage count:%d\n", ret);
}
