/*****************************************************************************
* Copyright 2010 - 2011 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/irq.h>

#include <mach/chip_pinmux.h>
#include <mach/pinmux.h>

#include <asm/gpio.h>

#include <mach/sdio_platform.h>
#include "sdhci-pltfm.h"

#define PRINT_ERR(format, args...) printk(KERN_ERR "%s: " format, __func__, ## args)
#define PRINT_INFO(format, args...) printk(KERN_INFO "%s: " format, __func__, ## args)

struct pin_config SdioPinCfgs;

struct sdio_wifi_dev {
	atomic_t dev_is_ready;
	struct sdio_wifi_gpio_cfg *wifi_gpio;
};

static struct sdio_wifi_dev gDev;

static void __wifi_reset(int reset_pin, int onoff)
{
	gpio_set_value(reset_pin, onoff);
	msleep(250);
}

/*
 * It's stupid that this API cannot return error code, but what can you do?
 */
void bcm_sdiowl_reset_b(int onoff)
{
	struct sdio_wifi_dev *dev = &gDev;
	struct sdio_wifi_gpio_cfg *wifi_gpio = dev->wifi_gpio;

	if (!atomic_read(&dev->dev_is_ready)) {
		PRINT_ERR("device not ready\n");
		return;
	}

	if (wifi_gpio->reset < 0) {
		PRINT_ERR("gpio reset pin not defined\n");
		return;
	}
	__wifi_reset(wifi_gpio->reset, onoff);
}
EXPORT_SYMBOL(bcm_sdiowl_reset_b);

int bcm_sdiowl_rescan(void)
{
	int rc;
	struct sdio_wifi_dev *dev = &gDev;
	struct mmc_card *card;

	if (!atomic_read(&dev->dev_is_ready)) {
		PRINT_ERR("device not ready\n");
		return -EFAULT;
	}

	card = sdio_get_mmc_card(SDIO_DEV_TYPE_WIFI);
	if (!card) {
		PRINT_ERR("unable to find WiFi SDIO device\n");
		return -EIO;
	}
	rc = sdio_reset_comm(card);

	return rc;
}
EXPORT_SYMBOL(bcm_sdiowl_rescan);

static int wifi_gpio_request(struct sdio_wifi_gpio_cfg *gpio)
{
	int rc = 0;

	printk(KERN_ERR "%s:ENTRY\n", __func__);

	if (gpio->reserved)
		return rc;

	PRINT_INFO("gpio pins reset:%d, req:%d wake:%d shutdown:%d\n",
		   gpio->reset, gpio->reg, gpio->host_wake, gpio->shutdown);

	if (gpio->reg >= 0) {
		rc = gpio_request(gpio->reg, "wl_reg_on");
		if (rc < 0) {
			PRINT_ERR("unable to request reg GPIO pin %d\n",
				  gpio->reg);
			return -EBUSY;
		}
		PRINT_INFO("current value of reg GPIO: %d\n",
			   gpio_get_value(gpio->reg));
		printk(KERN_ERR "%s: REG=%x\n", __func__, gpio->reg);
		gpio_direction_output(gpio->reg, 1);
		gpio_set_value(gpio->reg, 1);
	}

	if (gpio->reset >= 0) {
		rc = gpio_request(gpio->reset, "wl_reset");
		if (rc < 0) {
			PRINT_ERR("unable to request reset GPIO pin %d\n",
				  gpio->reset);
			goto err_free_gpio_reg;
		}
		printk(KERN_ERR "%s: RESET=%x\n", __func__, gpio->reset);
		PRINT_INFO("current value of reset GPIO: %d\n",
			   gpio_get_value(gpio->reset));
		gpio_direction_output(gpio->reset, 0);
/*      gpio_set_value(gpio->reset, 0); */
	}

	if (gpio->shutdown >= 0) {
		rc = gpio_request(gpio->shutdown, "wl_shutdown");
		if (rc < 0) {
			PRINT_ERR("unable to request shutdown GPIO pin %d\n",
				  gpio->shutdown);
			goto err_free_gpio_reset;
		}
		printk(KERN_ERR "%s: SHUTDOWN=%x\n", __func__,
		       gpio->shutdown);
		PRINT_INFO("current value of shutdown GPIO: %d\n",
			   gpio_get_value(gpio->shutdown));
		gpio_direction_output(gpio->shutdown, 1);
		gpio_set_value(gpio->shutdown, 1);
	}

	if (gpio->host_wake >= 0) {
		rc = gpio_request(gpio->host_wake, "wl_host_wake");
		if (rc < 0) {
			PRINT_ERR("unable to request wake GPIO pin %d\n",
				  gpio->host_wake);
			goto err_free_gpio_shutdown;
		}
		gpio_direction_input(gpio->host_wake);
		rc = irq_set_irq_type(gpio_to_irq(gpio->host_wake),
				      IRQ_TYPE_EDGE_RISING);
		if (rc < 0) {
			PRINT_ERR("unable to set irq type for GPIO pin %d\n",
				  gpio->host_wake);
			goto err_free_gpio_shutdown;
		}
	}
	printk(KERN_ERR "%s: HOST_WAKE=%x\n", __func__, gpio->host_wake);

	gpio->reserved = 1;

	return 0;

err_free_gpio_shutdown:
	if (gpio->shutdown >= 0)
		gpio_free(gpio->shutdown);

err_free_gpio_reset:
	if (gpio->reset >= 0)
		gpio_free(gpio->reset);

err_free_gpio_reg:
	if (gpio->reg >= 0)
		gpio_free(gpio->reg);

	return rc;
}

static void wifi_gpio_free(struct sdio_wifi_gpio_cfg *gpio)
{
	if (gpio->shutdown >= 0)
		gpio_free(gpio->shutdown);

	if (gpio->reset >= 0)
		gpio_free(gpio->reset);

	if (gpio->reg >= 0)
		gpio_free(gpio->reg);

	if (gpio->host_wake >= 0)
		gpio_free(gpio->host_wake);
		gpio->reserved = 0;
}

int bcm_sdiowl_init(int onoff)
{
	int rc;
	struct sdio_wifi_dev *dev = &gDev;
#ifndef CONFIG_BRCM_UNIFIED_DHD_SUPPORT
	int wait_cnt;
	struct mmc_card *card;
#endif

	printk(KERN_ERR "%s:ENTRY\n", __func__);


/* Set the Pull of Sdio Lines first */
	SdioPinCfgs.name = PN_MMC1CK;
	pinmux_get_pin_config(&SdioPinCfgs);
	SdioPinCfgs.reg.b.input_dis = 0;
	SdioPinCfgs.reg.b.pull_dn = 0;
	SdioPinCfgs.reg.b.pull_up = 0;
	SdioPinCfgs.reg.b.drv_sth = 3;
	pinmux_set_pin_config(&SdioPinCfgs);

	SdioPinCfgs.name = PN_MMC1CMD;
	pinmux_get_pin_config(&SdioPinCfgs);
	SdioPinCfgs.reg.b.pull_dn = 0;
	SdioPinCfgs.reg.b.pull_up = 1;
	SdioPinCfgs.reg.b.drv_sth = 3;
	pinmux_set_pin_config(&SdioPinCfgs);


	SdioPinCfgs.name = PN_MMC1DAT0;
	pinmux_get_pin_config(&SdioPinCfgs);
	SdioPinCfgs.reg.b.pull_dn = 0;
	SdioPinCfgs.reg.b.pull_up = 1;
	SdioPinCfgs.reg.b.drv_sth = 3;
	pinmux_set_pin_config(&SdioPinCfgs);

	SdioPinCfgs.name = PN_MMC1DAT1;
	pinmux_get_pin_config(&SdioPinCfgs);
	SdioPinCfgs.reg.b.pull_dn = 0;
	SdioPinCfgs.reg.b.pull_up = 1;
	SdioPinCfgs.reg.b.drv_sth = 3;
	pinmux_set_pin_config(&SdioPinCfgs);


	SdioPinCfgs.name = PN_MMC1DAT2;
	pinmux_get_pin_config(&SdioPinCfgs);
	SdioPinCfgs.reg.b.pull_dn = 0;
	SdioPinCfgs.reg.b.pull_up = 1;
	SdioPinCfgs.reg.b.drv_sth = 3;
	pinmux_set_pin_config(&SdioPinCfgs);


	SdioPinCfgs.name = PN_MMC1DAT3;
	pinmux_get_pin_config(&SdioPinCfgs);
	SdioPinCfgs.reg.b.pull_dn = 0;
	SdioPinCfgs.reg.b.pull_up = 1;
	SdioPinCfgs.reg.b.drv_sth = 3;
	pinmux_set_pin_config(&SdioPinCfgs);


	SdioPinCfgs.name = PN_MMC1DAT4;
	pinmux_get_pin_config(&SdioPinCfgs);
	SdioPinCfgs.reg.b.pull_dn = 1;
	SdioPinCfgs.reg.b.pull_up = 0;
	SdioPinCfgs.reg.b.drv_sth = 3;
	pinmux_set_pin_config(&SdioPinCfgs);
/*
	SdioPinCfgs.name = PN_LCDTE;
	pinmux_get_pin_config(&SdioPinCfgs);
	SdioPinCfgs.reg.b.pull_dn = 0;
	SdioPinCfgs.reg.b.pull_up = 0;
	SdioPinCfgs.reg.b.drv_sth = 3;
	pinmux_set_pin_config(&SdioPinCfgs);
*/


/* ----------------------------------- */



	/* check if the SDIO device is already up */
	rc = sdio_dev_is_initialized(SDIO_DEV_TYPE_WIFI);
	if (rc <= 0) {
		PRINT_ERR("sdio interface is not initialized or err=%d\n", rc);
		return rc;
	}
	printk(KERN_ERR "%s:GET_GPIO INFO\n", __func__);

	dev->wifi_gpio = sdio_get_wifi_gpio(SDIO_DEV_TYPE_WIFI);
#ifndef CONFIG_BRCM_UNIFIED_DHD_SUPPORT

	if (dev->wifi_gpio == NULL) {
		PRINT_ERR("wifi gpio hardware config is missing\n");
		return -EFAULT;
	}
#endif

#ifdef CONFIG_BRCM_UNIFIED_DHD_SUPPORT
#if defined(CONFIG_MACH_HAWAII_RAY) || defined(CONFIG_MACH_HAWAII_STONE) \
	|| defined(CONFIG_MACH_HAWAII_GARNET) \
	|| defined(CONFIG_MACH_HAWAII_SS_EVAL_REV00) \
	|| defined(CONFIG_MACH_HAWAII_SS_LOGAN_REV00) \
	|| defined(CONFIG_MACH_HAWAII_SS_LOGAN_REV01)
	dev->wifi_gpio->reset = 3;
	dev->wifi_gpio->reg = -1;
	dev->wifi_gpio->host_wake = 74;
	dev->wifi_gpio->shutdown = -1;
#endif
#endif

	/* reserve GPIOs */
	rc = wifi_gpio_request(dev->wifi_gpio);
	if (rc < 0) {
		PRINT_ERR("unable to reserve certain gpio pins\n");
		return rc;
	}

	/* reset the wifi chip */
	if (onoff)
		__wifi_reset(dev->wifi_gpio->reset, 1);
	else
		__wifi_reset(dev->wifi_gpio->reset, 0);

	printk(KERN_ERR "%s: WLAN_REG_ON(GPIO%d) : value(%d)\n",
		__func__, dev->wifi_gpio->reset,
		gpio_get_value(dev->wifi_gpio->reset));

	printk(KERN_ERR "%s:GPIO TOGGLED AND EXIT\n", __func__);

#ifndef CONFIG_BRCM_UNIFIED_DHD_SUPPORT

	/* now, emulate the card insertion */
	rc = sdio_card_emulate(SDIO_DEV_TYPE_WIFI, 1);
	if (rc < 0) {
		PRINT_ERR("sdio_card_emulate failed\n");
		goto err_free_gpio;
	}
#define WAIT_CNT 10
	/* need to wait for the mmc device population to finish */
	wait_cnt = 0;
	while (wait_cnt++ < WAIT_CNT) {
		card = sdio_get_mmc_card(SDIO_DEV_TYPE_WIFI);
		if (card) {
			atomic_set(&dev->dev_is_ready, 1);
			return 0;
		}
		msleep(100);
	}
	PRINT_ERR("timeout while populating sdio wifi device\n");
	rc = -EIO;
	sdio_card_emulate(SDIO_DEV_TYPE_WIFI, 0);

err_free_gpio:
	wifi_gpio_free(dev->wifi_gpio);
#endif /* CONFIG_BRCM_UNIFIED_DHD_SUPPORT */
	return rc;
}
EXPORT_SYMBOL(bcm_sdiowl_init);

void bcm_sdiowl_term(void)
{
	struct sdio_wifi_dev *dev = &gDev;
	printk(KERN_ERR " %s ENTRY\n", __func__);

	atomic_set(&dev->dev_is_ready, 0);

#ifndef CONFIG_BRCM_UNIFIED_DHD_SUPPORT
	sdio_card_emulate(SDIO_DEV_TYPE_WIFI, 0);

#endif

#ifdef CONFIG_BRCM_UNIFIED_DHD_SUPPORT
	msleep(2);
#endif

#ifdef CONFIG_BRCM_UNIFIED_DHD_SUPPORT
	__wifi_reset(dev->wifi_gpio->reset, 0);
#endif

	/* free GPIOs */
	wifi_gpio_free(dev->wifi_gpio);
	printk(KERN_ERR " %s GPIO Released\n", __func__);

	dev->wifi_gpio = NULL;


/*
 * 4334 bug requires us to Pull down on sdio lines on reset
 */

	SdioPinCfgs.name = PN_MMC1CK;
	pinmux_get_pin_config(&SdioPinCfgs);
	SdioPinCfgs.reg.b.input_dis = 1;
	SdioPinCfgs.reg.b.pull_dn = 0;
	SdioPinCfgs.reg.b.pull_up = 0;
	SdioPinCfgs.reg.b.drv_sth = 3;
	pinmux_set_pin_config(&SdioPinCfgs);

	SdioPinCfgs.name = PN_MMC1CMD;
	pinmux_get_pin_config(&SdioPinCfgs);
	SdioPinCfgs.reg.b.pull_dn = 1;
	SdioPinCfgs.reg.b.pull_up = 0;
	SdioPinCfgs.reg.b.drv_sth = 3;
	pinmux_set_pin_config(&SdioPinCfgs);


	SdioPinCfgs.name = PN_MMC1DAT0;
	pinmux_get_pin_config(&SdioPinCfgs);
	SdioPinCfgs.reg.b.pull_dn = 1;
	SdioPinCfgs.reg.b.pull_up = 0;
	SdioPinCfgs.reg.b.drv_sth = 3;
	pinmux_set_pin_config(&SdioPinCfgs);

	SdioPinCfgs.name = PN_MMC1DAT1;
	pinmux_get_pin_config(&SdioPinCfgs);
	SdioPinCfgs.reg.b.pull_dn = 1;
	SdioPinCfgs.reg.b.pull_up = 0;
	SdioPinCfgs.reg.b.drv_sth = 3;
	pinmux_set_pin_config(&SdioPinCfgs);


	SdioPinCfgs.name = PN_MMC1DAT2;
	pinmux_get_pin_config(&SdioPinCfgs);
	SdioPinCfgs.reg.b.pull_dn = 1;
	SdioPinCfgs.reg.b.pull_up = 0;
	SdioPinCfgs.reg.b.drv_sth = 3;
	pinmux_set_pin_config(&SdioPinCfgs);


	SdioPinCfgs.name = PN_MMC1DAT3;
	pinmux_get_pin_config(&SdioPinCfgs);
	SdioPinCfgs.reg.b.pull_dn = 1;
	SdioPinCfgs.reg.b.pull_up = 0;
	SdioPinCfgs.reg.b.drv_sth = 3;
	pinmux_set_pin_config(&SdioPinCfgs);

	SdioPinCfgs.name = PN_MMC1DAT4;
	pinmux_get_pin_config(&SdioPinCfgs);
	SdioPinCfgs.reg.b.pull_dn = 0;
	SdioPinCfgs.reg.b.pull_up = 1;
	SdioPinCfgs.reg.b.drv_sth = 3;
	pinmux_set_pin_config(&SdioPinCfgs);

/*
	SdioPinCfgs.name = PN_LCDTE;
	pinmux_get_pin_config(&SdioPinCfgs);
	SdioPinCfgs.reg.b.pull_dn=1;
	SdioPinCfgs.reg.b.pull_up=0;
	SdioPinCfgs.reg.b.drv_sth=3;
	pinmux_set_pin_config(&SdioPinCfgs);
*/


/* ----------------------------------- */


}
EXPORT_SYMBOL(bcm_sdiowl_term);

static int __init sdio_wifi_init(void)
{
	return 0;
}

static void __exit sdio_wifi_exit(void)
{
}

module_init(sdio_wifi_init);
module_exit(sdio_wifi_exit);

MODULE_DESCRIPTION("Broadcom SDIO WiFi/BT Control Driver");
MODULE_AUTHOR("Broadcom");
MODULE_LICENSE("GPL");
