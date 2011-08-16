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

#include <asm/gpio.h>

#include <mach/sdio_platform.h>
#include "sdhci-pltfm.h"

#define PRINT_ERR(format, args...) printk(KERN_ERR "SDIO-Wifi: " format, ## args)
#define PRINT_INFO(format, args...) printk(KERN_INFO "SDIO-Wifi: " format, ## args)

struct sdio_wifi_dev
{
   atomic_t dev_is_ready;
   struct sdio_wifi_gpio_cfg *wifi_gpio;
};

static struct sdio_wifi_dev gDev;

/*
 * It's stupid that this API cannot return error code, but what can you do?
 */
void bcm_sdiowl_reset_b(int onoff)
{
   struct sdio_wifi_dev *dev = &gDev;
   struct sdio_wifi_gpio_cfg *wifi_gpio = dev->wifi_gpio;

   if (!atomic_read(&dev->dev_is_ready))
   {
      PRINT_ERR("device not ready\n");
      return;
   }

   if (wifi_gpio->reset < 0)
   {
      PRINT_ERR("gpio reset pin not defined\n");
      return;
   }

   gpio_set_value(wifi_gpio->reset, onoff);

	msleep(200);
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
   int rc;

   PRINT_INFO("gpio pins reset:%d, req:%d wake:%d shutdown:%d\n",
	gpio->reset, gpio->reg, gpio->host_wake, gpio->shutdown);

   if (gpio->reg >= 0)
   {
      rc = gpio_request(gpio->reg, "wl_reg_on");
      if (rc < 0)
      {
         PRINT_ERR("unable to request reg GPIO pin %d\n", gpio->reg);
         return -EBUSY;
      }
      gpio_direction_output(gpio->reg, 1);
      gpio_set_value(gpio->reg, 1);
   }

   if (gpio->reset >= 0)
   {
      rc = gpio_request(gpio->reset, "wl_reset");
      if (rc < 0)
      {
         PRINT_ERR("unable to request reset GPIO pin %d\n", gpio->reset);
         goto err_free_gpio_reg;
      }
      gpio_direction_output(gpio->reset, 1);
      gpio_set_value(gpio->reset, 1);
   }

   if (gpio->shutdown >= 0)
   {
      rc = gpio_request(gpio->shutdown, "wl_shutdown");
      if (rc < 0)
      {
         PRINT_ERR("unable to request shutdown GPIO pin %d\n", gpio->shutdown);
         goto err_free_gpio_reset;
      }
      gpio_direction_output(gpio->shutdown, 1);
      gpio_set_value(gpio->shutdown, 1);
   }

   if (gpio->host_wake >= 0) {
      rc = gpio_request(gpio->host_wake, "wl_host_wake");
      if (rc < 0)
      {
         PRINT_ERR("unable to request wake GPIO pin %d\n", gpio->host_wake);
         goto err_free_gpio_shutdown;
      }
      gpio_direction_input(gpio->host_wake);
      rc = set_irq_type(gpio_to_irq(gpio->host_wake), IRQ_TYPE_EDGE_RISING);
      if (rc < 0)
      {
         PRINT_ERR("unable to set irq type for GPIO pin %d\n", gpio->host_wake);
         goto err_free_gpio_shutdown;
      }
   }

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
}

int bcm_sdiowl_init(void)
{
   int rc;
   struct sdio_wifi_dev *dev = &gDev;
   
   /* check if the SDIO device is already up */
   rc = sdio_dev_is_initialized(SDIO_DEV_TYPE_WIFI);
   if (rc <= 0)
   {
      PRINT_ERR("sdio interface is not initialized or err=%d\n", rc);
      return rc;  
   }

   dev->wifi_gpio = sdio_get_wifi_gpio(SDIO_DEV_TYPE_WIFI);
   if (dev->wifi_gpio == NULL)
   {
      PRINT_ERR("wifi gpio hardware config is missing\n");
      return -EFAULT;
   }

   /* reserve GPIOs */
   rc = wifi_gpio_request(dev->wifi_gpio);
   if (rc < 0)
   {
      PRINT_ERR("unable to reserve certain gpio pins\n");
      return rc;
   }

   atomic_set(&dev->dev_is_ready, 1);

	/* reset the wifi chip */
   bcm_sdiowl_reset_b(0);
   bcm_sdiowl_reset_b(1);
   
   /* now, emulate the card insertion */
   rc = sdio_card_emulate(SDIO_DEV_TYPE_WIFI, 1);
   if (rc < 0)
   {
      PRINT_ERR("sdio_card_emulate failed\n");
      goto err_free_gpio;
   }

	/* need to wait for the mmc device population to finish */
	msleep(500);
	return 0;

err_free_gpio:
   atomic_set(&dev->dev_is_ready, 0);
   wifi_gpio_free(dev->wifi_gpio);

   return rc;
}
EXPORT_SYMBOL(bcm_sdiowl_init);

void bcm_sdiowl_term(void)
{
   struct sdio_wifi_dev *dev = &gDev;

   sdio_card_emulate(SDIO_DEV_TYPE_WIFI, 0);
   atomic_set(&dev->dev_is_ready, 0);

   /* free GPIOs */
   wifi_gpio_free(dev->wifi_gpio);

   dev->wifi_gpio = NULL;
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
