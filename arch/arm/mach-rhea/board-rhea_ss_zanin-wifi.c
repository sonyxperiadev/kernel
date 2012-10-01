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
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/err.h>
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

//#include "hsmmc.h"
//#include "control.h"
//#include "mux.h"

//#include "mach/pinmux.h"
#include <mach/rdb/brcm_rdb_sysmap.h>
#include <mach/rdb/brcm_rdb_padctrlreg.h>

#include <mach/sdio_platform.h>
#include <mach/rdb/brcm_rdb_padctrlreg.h>
#include "board-rhea_ss_zanin-wifi.h"


#ifdef CONFIG_BRCM_UNIFIED_DHD_SUPPORT



#define GPIO_WLAN_PMENA		104
#define GPIO_WLAN_IRQ		2

#define ATAG_RHEA_MAC	0x57464d41
/* #define ATAG_RHEA_MAC_DEBUG */

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

static void *rhea_wifi_mem_prealloc(int section, unsigned long size)
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

int __init rhea_init_wifi_mem(void)
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

	for (i = 0 ; i < PREALLOC_WLAN_SEC_NUM ; i++) {
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

	printk(KERN_INFO " %s: WIFI MEM Allocated\n", __FUNCTION__);
	return 0;

 err_mem_alloc:
	pr_err("Failed to mem_alloc for WLAN\n");
	for (j = 0 ; j < i ; j++)
		kfree(wlan_mem_array[j].mem_ptr);

	i = WLAN_SKB_BUF_NUM;

 err_skb_alloc:
	pr_err("Failed to skb_alloc for WLAN\n");
	for (j = 0 ; j < i ; j++)
		dev_kfree_skb(wlan_static_skb[j]);

	return -ENOMEM;
}
#endif /* CONFIG_BROADCOM_WIFI_RESERVED_MEM */


extern int bcm_sdiowl_init(void);
extern int bcm_sdiowl_term(void);


int rhea_wifi_status_register(
		void (*callback)(int card_present, void *dev_id),
		void *dev_id);


EXPORT_SYMBOL(rhea_wifi_status_register);


int omap4_rhea_get_type(void);

/**
 * omap_mux_init_signal - initialize a signal based on the signal name
 * @muxname:		Mux name in mode0_name.signal_name format
 * @val:		Options for the mux register value
 */
int omap_mux_init_signal(const char *muxname, int val);


int omap4_rhea_get_type(void)
{
//	return tuna_hw_rev & TUNA_TYPE_MASK;
	return 0;
}

int __init omap_mux_init_signal(const char *muxname, int val)
{


	return 0;
}



static int rhea_wifi_cd = 0; /* WIFI virtual 'card detect' status */
static void (*wifi_status_cb)(int card_present, void *dev_id);
static void *wifi_status_cb_devid;
//static struct regulator *clk32kaudio_reg;

int rhea_wifi_status_register(
		void (*callback)(int card_present, void *dev_id),
		void *dev_id)
{
	printk(KERN_INFO " %s ENTRY\n", __FUNCTION__);

	if (wifi_status_cb)
		return -EAGAIN;
	wifi_status_cb = callback;
	wifi_status_cb_devid = dev_id;
	return 0;
}

static unsigned int rhea_wifi_status(struct device *dev)
{
	return rhea_wifi_cd;
}

struct mmc_platform_data rhea_wifi_data = {
	.ocr_mask		= MMC_VDD_165_195 | MMC_VDD_20_21,
	.built_in		= 1,
	.status			= rhea_wifi_status,
	.card_present		= 0,
	.register_status_notify	= rhea_wifi_status_register,
};

static int rhea_wifi_set_carddetect(int val)
{
	pr_debug("%s: %d\n", __func__, val);
	printk(KERN_INFO " %s INSIDE rhea_wifi_set_carddetect\n", __FUNCTION__);
	rhea_wifi_cd = val;
	if (wifi_status_cb) {
		printk(KERN_INFO " %s CALLBACK NOT NULL\n", __FUNCTION__);
		wifi_status_cb(val, wifi_status_cb_devid);
		printk(KERN_INFO " %s CALLBACK COMPLETE\n", __FUNCTION__);
	} else
		pr_warning("%s: Nobody to notify\n", __func__);
	
	if(val==0)
		bcm_sdiowl_term();
	return 0;
}

static int rhea_wifi_power_state;

struct fixed_voltage_data {
	struct regulator_desc desc;
	struct regulator_dev *dev;
	int microvolts;
	int gpio;
	unsigned startup_delay;
	bool enable_high;
	bool is_enabled;
};

static int rhea_wifi_power(int on)
{
	printk(KERN_INFO " %s INSIDE rhea_wifi_power\n", __FUNCTION__);
#if 0
	if (!clk32kaudio_reg) {
		clk32kaudio_reg = regulator_get(0, "clk32kaudio");
		if (IS_ERR(clk32kaudio_reg)) {
			pr_err("%s: clk32kaudio reg not found!\n", __func__);
			clk32kaudio_reg = NULL;
		}
	}

	if (clk32kaudio_reg && on && !rhea_wifi_power_state)
		regulator_enable(clk32kaudio_reg);

	pr_debug("%s: %d\n", __func__, on);
	mdelay(100);
	gpio_set_value(GPIO_WLAN_PMENA, on);
	mdelay(200);

	if (clk32kaudio_reg && !on && rhea_wifi_power_state)
		regulator_disable(clk32kaudio_reg);
#endif
	if(on)
		bcm_sdiowl_init();
		
	rhea_wifi_power_state = on;
	
	return 0;
}

static int rhea_wifi_reset_state;

static int rhea_wifi_reset(int on)
{
	pr_debug("%s: do nothing\n", __func__);
		printk(KERN_INFO " %s INSIDE rhea_wifi_reset\n", __FUNCTION__);
	rhea_wifi_reset_state = on;
	return 0;
}

static unsigned char rhea_mac_addr[IFHWADDRLEN] = { 0,0x90,0x4c,0,0,0 };

static int __init rhea_mac_addr_setup(char *str)
{
	char macstr[IFHWADDRLEN*3];
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
		res = strict_strtoul(token, 0x10, &val);
		if (res < 0)
			return 0;
		rhea_mac_addr[i++] = (u8)val;
	}

	return 1;
}

__setup("androidboot.macaddr=", rhea_mac_addr_setup);


static int rhea_wifi_get_mac_addr(unsigned char *buf)
{
  //	int type = omap4_rhea_get_type();
	uint rand_mac;

//	if (type != RHEA_TYPE_TORO)
//		return -EINVAL;

//	if (!buf)
//		return -EFAULT;

	if ((rhea_mac_addr[4] == 0) && (rhea_mac_addr[5] == 0)) {
		srandom32((uint)jiffies);
		rand_mac = random32();
		rhea_mac_addr[3] = (unsigned char)rand_mac;
		rhea_mac_addr[4] = (unsigned char)(rand_mac >> 8);
		rhea_mac_addr[5] = (unsigned char)(rand_mac >> 16);
	}
	memcpy(buf, rhea_mac_addr, IFHWADDRLEN);
	return 0;
}

/* Customized Locale table : OPTIONAL feature */
#define WLC_CNTRY_BUF_SZ	4
typedef struct cntry_locales_custom {
	char iso_abbrev[WLC_CNTRY_BUF_SZ];
	char custom_locale[WLC_CNTRY_BUF_SZ];
	int  custom_locale_rev;
} cntry_locales_custom_t;

static cntry_locales_custom_t rhea_wifi_translate_custom_table[] = {
/* Table should be filled out based on custom platform regulatory requirement */
	{"",   "XY", 4},  /* universal */
	{"US", "US", 69}, /* input ISO "US" to : US regrev 69 */
	{"CA", "US", 69}, /* input ISO "CA" to : US regrev 69 */
	{"EU", "EU", 5},  /* European union countries */
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
	{"GB", "EU", 5},  /* input ISO "GB" to : EU regrev 05 */
	{"IL", "IL", 0},
	{"CH", "CH", 0},
	{"TR", "TR", 0},
	{"NO", "NO", 0},
	{"KR", "XY", 3},
	{"AU", "XY", 3},
	{"CN", "XY", 3},  /* input ISO "CN" to : XY regrev 03 */
	{"TW", "XY", 3},
	{"AR", "XY", 3},
	{"MX", "XY", 3}
};

static void *rhea_wifi_get_country_code(char *ccode)
{
	int size = ARRAY_SIZE(rhea_wifi_translate_custom_table);
	int i;

	if (!ccode)
		return NULL;

	for (i = 0; i < size; i++)
		if (strcmp(ccode, rhea_wifi_translate_custom_table[i].iso_abbrev) == 0)
			return &rhea_wifi_translate_custom_table[i];
	return &rhea_wifi_translate_custom_table[0];
}


static struct resource rhea_wifi_resources[] = {
	[0] = {
		.name		= "bcmdhd_wlan_irq",
		.start		= gpio_to_irq(7),
		.end		= gpio_to_irq(7),
		.flags          = IORESOURCE_IRQ | IORESOURCE_IRQ_LOWEDGE/* IORESOURCE_IRQ_HIGHLEVEL */| IORESOURCE_IRQ_SHAREABLE,
	},
};



static struct wifi_platform_data rhea_wifi_control = {
	.set_power      = rhea_wifi_power,
	.set_reset      = rhea_wifi_reset,
	.set_carddetect = rhea_wifi_set_carddetect,
#ifdef CONFIG_BROADCOM_WIFI_RESERVED_MEM
	.mem_prealloc	= rhea_wifi_mem_prealloc,
#endif
	.get_mac_addr	= rhea_wifi_get_mac_addr,
	.get_country_code = rhea_wifi_get_country_code,
};




static struct platform_device rhea_wifi_device = {
	.name = "bcmdhd_wlan",
	.id = 2,
	.resource = rhea_wifi_resources,
	.num_resources   = ARRAY_SIZE(rhea_wifi_resources),
	.dev      = {
		.platform_data = &rhea_wifi_control,
	},
};


/* Common devices among all the Rhea boards (Rhea Ray, Rhea Berri, etc.) */
//static struct platform_device *board_sdio_plat_devices[] __initdata = {
//	&rhea_wifi_device,
//};


static void __init rhea_wlan_gpio(void)
{
	pr_debug("%s: start\n", __func__);
}

int __init rhea_wlan_init(void)
{
	pr_debug("%s: start\n", __func__);
	printk(KERN_INFO " %s Calling GPIO INIT!\n", __FUNCTION__);

	rhea_wlan_gpio();
	printk(KERN_INFO " %s Calling GPIO INIT DONE !\n", __FUNCTION__);
#ifdef CONFIG_BROADCOM_WIFI_RESERVED_MEM
	rhea_init_wifi_mem();
#endif
	printk(KERN_INFO " %s Calling MEM INIT DONE !\n", __FUNCTION__);
	

	return platform_device_register(&rhea_wifi_device);


}

#endif
