/*
 * Copyright (C) 2011 Google, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#include <mach/chip_pinmux.h>
#include <mach/pinmux.h>

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/module.h>
#include <asm/mach-types.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/setup.h>
#include <linux/if.h>
#include <linux/skbuff.h>
#include <linux/wlan_plat.h>
#include <linux/pm_runtime.h>
#include <linux/regulator/machine.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/fixed.h>
#include <asm/mach/mmc.h>
#include <linux/random.h>
#include <linux/jiffies.h>
#include <mach/rdb/brcm_rdb_sysmap.h>
#include <mach/rdb/brcm_rdb_padctrlreg.h>
#include <mach/sdio_platform.h>
#include <mach/rdb/brcm_rdb_padctrlreg.h>
#include "hawaii_wifi.h"

#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>

#include <linux/irq.h>
#include <mach/sdio_platform.h>


#ifdef CONFIG_BRCM_UNIFIED_DHD_SUPPORT

static struct sdio_wifi_gpio_cfg devtreeWifiParms;

#ifndef CONFIG_MMC_KONA_SDIO_WIFI
/* Functions below imported from sdio-wifi.c */

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

static int bcm_sdiowl_init(int onoff)
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
	dev->wifi_gpio->reg = -1;		/* Unused */
	dev->wifi_gpio->shutdown = -1;	/* Unused */
	dev->wifi_gpio->reset = devtreeWifiParms.reset;
	dev->wifi_gpio->host_wake = devtreeWifiParms.host_wake;

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

	printk(KERN_ERR "%s: WLAN_REG_ON(GPIO%d) : value(%d)\n", __func__,
			dev->wifi_gpio->reset,
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

static void bcm_sdiowl_term(void)
{
	struct sdio_wifi_dev *dev = &gDev;
	printk(KERN_ERR " %s ENTRY\n", __func__);

	atomic_set(&dev->dev_is_ready, 0);

#ifndef CONFIG_BRCM_UNIFIED_DHD_SUPPORT
	sdio_card_emulate(SDIO_DEV_TYPE_WIFI, 0);

#endif

#ifdef CONFIG_BRCM_UNIFIED_DHD_SUPPORT
	msleep(20);
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




/*----------------------------------- */


}

#endif	/* CONFIG_MMC_KONA_SDIO_WIFI */

#ifdef CONFIG_BROADCOM_WIFI_RESERVED_MEM

#define WLAN_STATIC_SCAN_BUF0		5
#define WLAN_STATIC_SCAN_BUF1		6
#define PREALLOC_WLAN_SEC_NUM		4
#define PREALLOC_WLAN_BUF_NUM		160
#define PREALLOC_WLAN_SECTION_HEADER	24
#define WLAN_SECTION_SIZE_0	(PREALLOC_WLAN_BUF_NUM * 128)
#define WLAN_SECTION_SIZE_1	(PREALLOC_WLAN_BUF_NUM * 128)
#define WLAN_SECTION_SIZE_2	(PREALLOC_WLAN_BUF_NUM * 512)
#define WLAN_SECTION_SIZE_3	(PREALLOC_WLAN_BUF_NUM * 1024)
#define DHD_SKB_HDRSIZE			336
#define DHD_SKB_1PAGE_BUFSIZE	((PAGE_SIZE*1)-DHD_SKB_HDRSIZE)
#define DHD_SKB_2PAGE_BUFSIZE	((PAGE_SIZE*2)-DHD_SKB_HDRSIZE)
#define DHD_SKB_4PAGE_BUFSIZE	((PAGE_SIZE*4)-DHD_SKB_HDRSIZE)
#define WLAN_SKB_BUF_NUM	17

static struct sk_buff *wlan_static_skb[WLAN_SKB_BUF_NUM];

struct wlan_mem_prealloc {
	void *mem_ptr;
	unsigned long size;
};

static struct wlan_mem_prealloc wlan_mem_array[PREALLOC_WLAN_SEC_NUM] = {
	{NULL, (WLAN_SECTION_SIZE_0 + PREALLOC_WLAN_SECTION_HEADER)},
	{NULL, (WLAN_SECTION_SIZE_1 + PREALLOC_WLAN_SECTION_HEADER)},
	{NULL, (WLAN_SECTION_SIZE_2 + PREALLOC_WLAN_SECTION_HEADER)},
	{NULL, (WLAN_SECTION_SIZE_3 + PREALLOC_WLAN_SECTION_HEADER)}
};

void *wlan_static_scan_buf0;
void *wlan_static_scan_buf1;

static void *hawaii_wifi_mem_prealloc(int section, unsigned long size)
{
	if (section == PREALLOC_WLAN_SEC_NUM)
		return wlan_static_skb;
	if (section == WLAN_STATIC_SCAN_BUF0)
		return wlan_static_scan_buf0;
	if (section == WLAN_STATIC_SCAN_BUF1)
		return wlan_static_scan_buf1;
	if ((section < 0) || (section > PREALLOC_WLAN_SEC_NUM))
		return NULL;

	if (wlan_mem_array[section].size < size)
		return NULL;

	return wlan_mem_array[section].mem_ptr;
}

int __init hawaii_init_wifi_mem(void)
{
	int i;
	int j;

	for (i = 0; i < 8; i++) {
		wlan_static_skb[i] = dev_alloc_skb(DHD_SKB_1PAGE_BUFSIZE);
		if (!wlan_static_skb[i])
			goto err_skb_alloc;
	}

	for (; i < 16; i++) {
		wlan_static_skb[i] = dev_alloc_skb(DHD_SKB_2PAGE_BUFSIZE);
		if (!wlan_static_skb[i])
			goto err_skb_alloc;
	}

	wlan_static_skb[i] = dev_alloc_skb(DHD_SKB_4PAGE_BUFSIZE);
	if (!wlan_static_skb[i])
		goto err_skb_alloc;

	for (i = 0; i < PREALLOC_WLAN_SEC_NUM; i++) {
		wlan_mem_array[i].mem_ptr =
		    kmalloc(wlan_mem_array[i].size, GFP_KERNEL);

		if (!wlan_mem_array[i].mem_ptr)
			goto err_mem_alloc;
	}
	wlan_static_scan_buf0 = kmalloc(65536, GFP_KERNEL);
	if (!wlan_static_scan_buf0)
		goto err_mem_alloc;
	wlan_static_scan_buf1 = kmalloc(65536, GFP_KERNEL);
	if (!wlan_static_scan_buf1)
		goto err_mem_alloc;

	printk(KERN_WARNING "%s: WIFI MEM Allocated\n", __func__);
	return 0;

err_mem_alloc:
	pr_err("Failed to mem_alloc for WLAN\n");
	for (j = 0; j < i; j++)
		kfree(wlan_mem_array[j].mem_ptr);

	i = WLAN_SKB_BUF_NUM;

err_skb_alloc:
	pr_err("Failed to skb_alloc for WLAN\n");
	for (j = 0; j < i; j++)
		dev_kfree_skb(wlan_static_skb[j]);

	return -ENOMEM;
}
#endif /* CONFIG_BROADCOM_WIFI_RESERVED_MEM */


static int hawaii_wifi_cd;	/* WIFI virtual 'card detect' status */
static void (*wifi_status_cb) (int card_present, void *dev_id);
static void *wifi_status_cb_devid;

int hawaii_wifi_status_register(void (*callback) (int card_present,
				void *dev_id), void *dev_id)
{
	printk(KERN_ERR " %s ENTRY\n", __func__);

	if (wifi_status_cb)
		return -EAGAIN;
	wifi_status_cb = callback;
	wifi_status_cb_devid = dev_id;
	return 0;
}
EXPORT_SYMBOL(hawaii_wifi_status_register);


static unsigned int hawaii_wifi_status(struct device *dev)
{
	return hawaii_wifi_cd;
}

struct mmc_platform_data hawaii_wifi_data = {
	.ocr_mask = MMC_VDD_165_195 | MMC_VDD_20_21,
	.built_in = 1,
	.status = hawaii_wifi_status,
	.card_present = 0,
	.register_status_notify = hawaii_wifi_status_register,
};

static int hawaii_wifi_set_carddetect(int val)
{
	pr_debug("%s: %d\n", __func__, val);
	printk(KERN_ERR " %s INSIDE hawaii_wifi_set_carddetect\n", __func__);
	hawaii_wifi_cd = val;
	if (wifi_status_cb) {
		printk(KERN_ERR " %s CALLBACK NOT NULL\n", __func__);
		wifi_status_cb(val, wifi_status_cb_devid);
		printk(KERN_ERR " %s CALLBACK COMPLETE\n", __func__);
	} else
		pr_warning("%s: Nobody to notify\n", __func__);

	return 0;
}

static int hawaii_wifi_power_state;

static int hawaii_wifi_power(int on)
{
	printk(KERN_ERR " %s INSIDE hawaii_wifi_power\n", __func__);

	if (on)
		bcm_sdiowl_init(on);
	else
		bcm_sdiowl_term();

	hawaii_wifi_power_state = on;

	return 0;
}

static int hawaii_wifi_reset_state;

static int hawaii_wifi_reset(int on)
{
	pr_debug("%s: do nothing\n", __func__);
	printk(KERN_ERR " %s INSIDE hawaii_wifi_reset\n", __func__);
	hawaii_wifi_reset_state = on;
	return 0;
}

static unsigned char hawaii_mac_addr[IFHWADDRLEN] = { 0, 0x90, 0x4c, 0, 0, 0 };

static int __init hawaii_mac_addr_setup(char *str)
{
	char macstr[IFHWADDRLEN * 3];
	char *macptr = macstr;
	char *token;
	int i = 0;

	if (!str)
		return 0;
	pr_debug("wlan MAC = %s\n", str);
	if (strlen(str) >= sizeof(macstr))
		return 0;
	strcpy(macstr, str);

	while ((token = strsep(&macptr, ":")) != NULL) {
		unsigned long val;
		int res;

		if (i >= IFHWADDRLEN)
			break;
		res = kstrtoul(token, 0x10, &val);
		if (res < 0)
			return 0;
		hawaii_mac_addr[i++] = (u8)val;
	}

	return 1;
}

__setup("androidboot.macaddr=", hawaii_mac_addr_setup);

static int hawaii_wifi_get_mac_addr(unsigned char *buf)
{
	uint rand_mac;

	if ((hawaii_mac_addr[4] == 0) && (hawaii_mac_addr[5] == 0)) {
		srandom32((uint) jiffies);
		rand_mac = random32();
		hawaii_mac_addr[3] = (unsigned char)rand_mac;
		hawaii_mac_addr[4] = (unsigned char)(rand_mac >> 8);
		hawaii_mac_addr[5] = (unsigned char)(rand_mac >> 16);
	}
	memcpy(buf, hawaii_mac_addr, IFHWADDRLEN);
	return 0;
}

/* Customized Locale table : OPTIONAL feature */
#define WLC_CNTRY_BUF_SZ	4
struct cntry_locales_custom {
	char iso_abbrev[WLC_CNTRY_BUF_SZ];
	char custom_locale[WLC_CNTRY_BUF_SZ];
	int custom_locale_rev;
};

static struct cntry_locales_custom
	hawaii_wifi_translate_custom_table[] = {
/* Table should be filled out based on custom platform regulatory requirement */
	{"", "XY", 4},		/* universal */
	{"US", "US", 69},	/* input ISO "US" to : US regrev 69 */
	{"CA", "US", 69},	/* input ISO "CA" to : US regrev 69 */
	{"EU", "EU", 5},	/* European union countries */
	{"AT", "EU", 5},
	{"BE", "EU", 5},
	{"BG", "EU", 5},
	{"CY", "EU", 5},
	{"CZ", "EU", 5},
	{"DK", "EU", 5},
	{"EE", "EU", 5},
	{"FI", "EU", 5},
	{"FR", "EU", 5},
	{"DE", "EU", 5},
	{"GR", "EU", 5},
	{"HU", "EU", 5},
	{"IE", "EU", 5},
	{"IT", "EU", 5},
	{"LV", "EU", 5},
	{"LI", "EU", 5},
	{"LT", "EU", 5},
	{"LU", "EU", 5},
	{"MT", "EU", 5},
	{"NL", "EU", 5},
	{"PL", "EU", 5},
	{"PT", "EU", 5},
	{"RO", "EU", 5},
	{"SK", "EU", 5},
	{"SI", "EU", 5},
	{"ES", "EU", 5},
	{"SE", "EU", 5},
	{"GB", "EU", 5},	/* input ISO "GB" to : EU regrev 05 */
	{"IL", "IL", 0},
	{"CH", "CH", 0},
	{"TR", "TR", 0},
	{"NO", "NO", 0},
	{"KR", "XY", 3},
	{"AU", "XY", 3},
	{"CN", "XY", 3},	/* input ISO "CN" to : XY regrev 03 */
	{"TW", "XY", 3},
	{"AR", "XY", 3},
	{"MX", "XY", 3}
};

static void *hawaii_wifi_get_country_code(char *ccode)
{
	int size = ARRAY_SIZE(hawaii_wifi_translate_custom_table);
	int i;

	if (!ccode)
		return NULL;

	for (i = 0; i < size; i++)
		if (strcmp
		    (ccode,
		     hawaii_wifi_translate_custom_table[i].iso_abbrev) == 0)
			return &hawaii_wifi_translate_custom_table[i];
	return &hawaii_wifi_translate_custom_table[0];
}

static struct resource hawaii_wifi_resources[] = {
	[0] = {
		.name		= "bcmdhd_wlan_irq",
		.start		= gpio_to_irq(74),	/* GPIO74 */
		.end		= gpio_to_irq(74),	/* GPIO74 */
		.flags          = IORESOURCE_IRQ | IORESOURCE_IRQ_LOWEDGE
				| IORESOURCE_IRQ_SHAREABLE | IRQF_NO_SUSPEND,
	},
};


static struct wifi_platform_data hawaii_wifi_control = {
	.set_power = hawaii_wifi_power,
	.set_reset = hawaii_wifi_reset,
	.set_carddetect = hawaii_wifi_set_carddetect,
#ifdef CONFIG_BROADCOM_WIFI_RESERVED_MEM
	.mem_prealloc = hawaii_wifi_mem_prealloc,
#endif
	.get_mac_addr = hawaii_wifi_get_mac_addr,
	.get_country_code = hawaii_wifi_get_country_code,
};

static struct platform_device hawaii_wifi_device = {
	.name = "bcmdhd_wlan",
	.id = 2,
	.resource = hawaii_wifi_resources,
	.num_resources = ARRAY_SIZE(hawaii_wifi_resources),
	.dev = {
		.platform_data = &hawaii_wifi_control,
	},
};

#define MAX_WIFI_NVRAM_PATH_LEN		200
#define MAX_WIFI_DRIVER_NAME_LEN	20

static char custom_fw_path[MAX_WIFI_NVRAM_PATH_LEN];
static char custom_module_name[MAX_WIFI_DRIVER_NAME_LEN];


static ssize_t show_module_name(struct device_driver *driver, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%s\n", custom_module_name);
}

static DRIVER_ATTR(module_name, S_IRUGO, show_module_name, NULL);


static int __devinit bcm_wifi_pltfm_probe(struct platform_device *pdev)
{
u32 readval;
const char *prop;

	printk(KERN_INFO "%s: probe called!\n", __func__);

	if (pdev->dev.platform_data) {
		struct board_wifi_info *wifi_dev = pdev->dev.platform_data;

		printk(KERN_INFO "%s: calling with board platform data\n",
				__func__);

		devtreeWifiParms.reset = wifi_dev->wl_reset_gpio;
		devtreeWifiParms.host_wake = wifi_dev->host_wake_gpio;
		strcpy(custom_fw_path, wifi_dev->board_nvram_file);
		strcpy(custom_module_name, wifi_dev->module_name);

	} else {	/*Get parms from device tree */

		if (!pdev->dev.of_node)
			goto wifi_err1;

		/* Read device tree properties */

		printk(KERN_INFO "%s: calling DTS node\n", __func__);

		if (of_property_read_u32(pdev->dev.of_node,
					"wl-reset-gpio", &readval))
			goto wifi_err1;
		devtreeWifiParms.reset = readval;

		if (of_property_read_u32(pdev->dev.of_node,
					"host-wake-gpio", &readval))
			goto wifi_err1;
		devtreeWifiParms.host_wake = readval;

		if (of_property_read_string(pdev->dev.of_node,
					"board-nvram-file", &prop))
			goto wifi_err1;
		strcpy(custom_fw_path, prop);

		if (of_property_read_string(pdev->dev.of_node,
					"module-name", &prop))
			goto wifi_err1;
		strcpy(custom_module_name, prop);
	}

	hawaii_wifi_device.resource->start =
		hawaii_wifi_device.resource->end =
		gpio_to_irq(devtreeWifiParms.host_wake);

	/* Setup attributes to read from sysfs */
	if (driver_create_file(pdev->dev.driver, &driver_attr_module_name)) {
		printk(KERN_ERR "Error writing to dev_attr file\n");
		return -EINVAL;
	}

	return 0;

wifi_err1:
	printk(KERN_ERR "%s: Error parsing Devtree!\n", __func__);
	return -EINVAL;
}

static int bcm_wifi_pltfm_remove(struct platform_device *pdev)
{
/* Dummy function. No work needed here for now */

	return 0;
}


#ifndef CONFIG_BYPASS_WIFI_DEVTREE
static const struct of_device_id bcm_wifi_match[] = {
	{ .compatible = "bcm,bcm_wifi"},
	{ /* Sentinel */ }
};
#endif

static struct platform_driver bcm_wifi_pltfm_driver = {
	.driver = {
		.name = "bcm_wifi",
		.owner = THIS_MODULE,
#ifndef CONFIG_BYPASS_WIFI_DEVTREE
		.of_match_table = bcm_wifi_match,
#endif
		},
	.probe = bcm_wifi_pltfm_probe,
	.remove = __devexit_p(bcm_wifi_pltfm_remove),
};


int wifi_set_custom_nvram_path(char *nv_path)
{
	if (strlen(custom_fw_path) == 0)
		return -EINVAL;
	strcpy(nv_path, custom_fw_path);
	return 0;
}
EXPORT_SYMBOL(wifi_set_custom_nvram_path);


int __init hawaii_wlan_init(void)
{
	pr_debug("%s: start\n", __func__);

#ifdef CONFIG_BROADCOM_WIFI_RESERVED_MEM
	hawaii_init_wifi_mem();
	printk(KERN_ERR " %s Calling MEM INIT DONE !\n", __func__);

#endif
	if (platform_driver_register(&bcm_wifi_pltfm_driver) != 0)
		printk(KERN_ERR
			"%s: Error register wifi_pltfm_driver\n", __func__);

	return platform_device_register(&hawaii_wifi_device);
}

#endif /* top of file: CONFIG_BRCM_UNIFIED_DHD_SUPPORT */
