/* arch/arm/mach-msm/board-sony_shinano-wifi.c
 *
 * Copyright (C) 2013 Sony Mobile Communications AB.
 * Copyright (C) 2013 LGE, Inc
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

#include <asm/gpio.h>
#include <asm/mach/mmc.h>
#include <linux/qpnp/pin.h>
#include <linux/regulator/consumer.h>
#include <linux/skbuff.h>
#include <linux/wlan_plat.h>

#include <mach/board-sony_shinano-wifi.h>

static unsigned int g_wifi_detect;
static void *sdc_dev;
void (*sdc_status_cb)(int card_present, void *dev);
static struct regulator *wifi_batfet;
static int batfet_ena;

static char *intf_macaddr = NULL;

#define WIFI_POWER_PMIC_GPIO 18
#define WIFI_IRQ_GPIO 67

/* These definitions need to be aligned with bcmdhd */
#define WLAN_STATIC_SCAN_BUF 5
#define ESCAN_BUF_SIZE (64 * 1024) /* for WIPHY_ESCAN0 */

#define PREALLOC_WLAN_NUMBER_OF_SECTIONS	4
#define PREALLOC_WLAN_NUMBER_OF_BUFFERS		160
#define PREALLOC_WLAN_SECTION_HEADER		24

#define WLAN_SECTION_SIZE_0 (PREALLOC_WLAN_NUMBER_OF_BUFFERS * 128)  /* for PROT */
#define WLAN_SECTION_SIZE_1 (PREALLOC_WLAN_NUMBER_OF_BUFFERS * 128)  /* for RXBUF */
#define WLAN_SECTION_SIZE_2 (PREALLOC_WLAN_NUMBER_OF_BUFFERS * 512)  /* for DATABUF */
#define WLAN_SECTION_SIZE_3 (PREALLOC_WLAN_NUMBER_OF_BUFFERS * 1024) /* for OSL_BUF */

/* These definitions are copied from bcmdhd */
#define DHD_SKB_HDRSIZE 336
#define DHD_SKB_1PAGE_BUFSIZE ((PAGE_SIZE * 1) - DHD_SKB_HDRSIZE)
#define DHD_SKB_2PAGE_BUFSIZE ((PAGE_SIZE * 2) - DHD_SKB_HDRSIZE)
#define DHD_SKB_4PAGE_BUFSIZE ((PAGE_SIZE * 4) - DHD_SKB_HDRSIZE)

#define WLAN_SKB_BUF_NUM 17 /* 8 for 1PAGE, 8 for 2PAGE, 1 for 4PAGE */

static struct sk_buff *wlan_static_skb[WLAN_SKB_BUF_NUM];

typedef struct wifi_mem_prealloc_struct {
	    void *mem_ptr;
	    unsigned long size;
} wifi_mem_prealloc_t;

static wifi_mem_prealloc_t wifi_mem_array[PREALLOC_WLAN_NUMBER_OF_SECTIONS] = {
	{ NULL, (WLAN_SECTION_SIZE_0 + PREALLOC_WLAN_SECTION_HEADER) },
	{ NULL, (WLAN_SECTION_SIZE_1 + PREALLOC_WLAN_SECTION_HEADER) },
	{ NULL, (WLAN_SECTION_SIZE_2 + PREALLOC_WLAN_SECTION_HEADER) },
	{ NULL, (WLAN_SECTION_SIZE_3 + PREALLOC_WLAN_SECTION_HEADER) }
};

static void *wlan_static_scan_buf;

static int shinano_wifi_init_mem(void)
{
	int i;
	for (i = 0; i < WLAN_SKB_BUF_NUM; i++)
		wlan_static_skb[i] = NULL;

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

	for (i = 0; i < PREALLOC_WLAN_NUMBER_OF_SECTIONS; i++) {
		wifi_mem_array[i].mem_ptr =
			kmalloc(wifi_mem_array[i].size, GFP_KERNEL);
		if (!wifi_mem_array[i].mem_ptr)
			goto err_mem_alloc;
	}

	wlan_static_scan_buf = kmalloc(ESCAN_BUF_SIZE, GFP_KERNEL);
	if (!wlan_static_scan_buf)
		goto err_mem_alloc;

	return 0;

err_mem_alloc:
	printk(KERN_ERR "%s: failed to allocate mem_alloc\n", __func__);
	for (i--; i >= 0; i--) {
		kfree(wifi_mem_array[i].mem_ptr);
		wifi_mem_array[i].mem_ptr = NULL;
	}

	i = WLAN_SKB_BUF_NUM;
err_skb_alloc:
	printk(KERN_ERR "%s: failed to allocate skb_alloc\n", __func__);
	for (i--; i >= 0; i--) {
		dev_kfree_skb(wlan_static_skb[i]);
		wlan_static_skb[i] = NULL;
	}

	return -ENOMEM;
}

int wcf_status_register(void (*cb)(int card_present, void *dev), void *dev)
{
	pr_info("%s\n", __func__);

	if (sdc_status_cb)
		return -EINVAL;

	sdc_status_cb = cb;
	sdc_dev = dev;

	return 0;
}

unsigned int wcf_status(struct device *dev)
{
	pr_info("%s: wifi_detect = %d\n", __func__, g_wifi_detect);
	return g_wifi_detect;
}

static void *shinano_wifi_mem_prealloc(int section, unsigned long size)
{
	if (section == PREALLOC_WLAN_NUMBER_OF_SECTIONS)
		return wlan_static_skb;
	if (section == WLAN_STATIC_SCAN_BUF)
		return wlan_static_scan_buf;

	if ((section < 0) || (section > PREALLOC_WLAN_NUMBER_OF_SECTIONS))
		return NULL;
	if (size > wifi_mem_array[section].size)
		return NULL;
	return wifi_mem_array[section].mem_ptr;
}

int shinano_wifi_set_power(int on)
{
	int gpio = qpnp_pin_map("pm8941-gpio", WIFI_POWER_PMIC_GPIO);
	if (!wifi_batfet) {
		wifi_batfet = regulator_get(NULL, "batfet");
		if (IS_ERR_OR_NULL(wifi_batfet)) {
			printk(KERN_ERR "unable to get batfet reg. rc=%d\n",
				PTR_RET(wifi_batfet));
			wifi_batfet = NULL;
		}
	}
	if (on) {
		if (!batfet_ena && wifi_batfet) {
			regulator_enable(wifi_batfet);
			batfet_ena = 1;
		}
	}
	gpio_set_value(gpio, on);
	if (!on) {
		if (batfet_ena && wifi_batfet) {
			regulator_disable(wifi_batfet);
			batfet_ena = 0;
		}
	}
	return 0;
}

static int shinano_wifi_set_reset(int on)
{
	return 0;
}

int shinano_wifi_set_carddetect(int val)
{
	g_wifi_detect = val;

	if (sdc_status_cb)
		sdc_status_cb(val, sdc_dev);
	else
		printk(KERN_WARNING "%s: Nobody to notify\n", __func__);
	return 0;
}

static struct resource shinano_wifi_resources[] = {
	[0] = {
		.name	= "bcmdhd_wlan_irq",
		.start	= 0,
		.end	= 0,
		.flags	= IORESOURCE_IRQ | IORESOURCE_IRQ_HIGHLEVEL |
			  IORESOURCE_IRQ_SHAREABLE,
	},
};

#define ETHER_ADDR_LEN    6
#define FILE_WIFI_MACADDR "/sys/devices/platform/bcmdhd_wlan/macaddr"

static inline int xdigit (char c)
{
	unsigned d;

	d = (unsigned)(c-'0');
	if (d < 10)
		return (int)d;
	d = (unsigned)(c-'a');
	if (d < 6)
		return (int)(10+d);
	d = (unsigned)(c-'A');
	if (d < 6)
		return (int)(10+d);
	return -1;
}

struct ether_addr {
	unsigned char ether_addr_octet[ETHER_ADDR_LEN];
} __attribute__((__packed__));

struct ether_addr *
ether_aton_r (const char *asc, struct ether_addr * addr)
{
	int i, val0, val1;

	for (i = 0; i < ETHER_ADDR_LEN; ++i) {
		val0 = xdigit(*asc);
		asc++;
		if (val0 < 0)
			return NULL;

		val1 = xdigit(*asc);
		asc++;
		if (val1 < 0)
			return NULL;

		addr->ether_addr_octet[i] = (unsigned char)((val0 << 4) + val1);

		if (i < ETHER_ADDR_LEN - 1) {
			if (*asc != ':')
				return NULL;
			asc++;
		}
	}

	if (*asc != '\0')
		return NULL;

	return addr;
}

struct ether_addr * ether_aton (const char *asc)
{
	static struct ether_addr addr;
	return ether_aton_r(asc, &addr);
}

static int shinano_wifi_get_mac_addr(unsigned char *buf)
{
	int ret = 0;

	mm_segment_t oldfs;
	struct kstat stat;
	struct file* fp;
	int readlen = 0;
	char macasc[128] = {0,};
	uint rand_mac;
	static unsigned char mymac[ETHER_ADDR_LEN] = {0,};
	const unsigned char nullmac[ETHER_ADDR_LEN] = {0,};

	if (buf == NULL)
		return -EAGAIN;

	memset(buf, 0x00, ETHER_ADDR_LEN);

	oldfs = get_fs();
	set_fs(get_ds());

	ret = vfs_stat(FILE_WIFI_MACADDR, &stat);
	if (ret) {
		set_fs(oldfs);
		pr_err("%s: Failed to get information from file %s (%d)\n",
				__FUNCTION__, FILE_WIFI_MACADDR, ret);
		goto random_mac;
	}
	set_fs(oldfs);

	fp = filp_open(FILE_WIFI_MACADDR, O_RDONLY, 0);
	if (IS_ERR(fp)) {
		pr_err("%s: Failed to read error %s\n",
				__FUNCTION__, FILE_WIFI_MACADDR);
		goto random_mac;
	}

	readlen = kernel_read(fp, fp->f_pos, macasc, 17); // 17 = 12 + 5
	if (readlen > 0) {
		unsigned char* macbin;
		struct ether_addr* convmac = ether_aton( macasc );

		if (convmac == NULL) {
			pr_err("%s: Invalid Mac Address Format %s\n",
					__FUNCTION__, macasc );
			goto random_mac;
		}

		macbin = convmac->ether_addr_octet;

		pr_info("%s: READ MAC ADDRESS %02X:%02X:%02X:%02X:%02X:%02X\n",
				__FUNCTION__,
				macbin[0], macbin[1], macbin[2],
				macbin[3], macbin[4], macbin[5]);

		memcpy(buf, macbin, ETHER_ADDR_LEN);
	} else {
		goto random_mac;
	}

	filp_close(fp, NULL);
	return ret;

random_mac:

	pr_debug("%s: %p\n", __func__, buf);

	if (memcmp( mymac, nullmac, ETHER_ADDR_LEN) != 0) {
		/* Mac displayed from UI is never updated..
		   So, mac obtained on initial time is used */
		memcpy(buf, mymac, ETHER_ADDR_LEN);
		return 0;
	}

	srandom32((uint)jiffies);
	rand_mac = random32();
	buf[0] = 0x00;
	buf[1] = 0x90;
	buf[2] = 0x4c;
	buf[3] = (unsigned char)rand_mac;
	buf[4] = (unsigned char)(rand_mac >> 8);
	buf[5] = (unsigned char)(rand_mac >> 16);

	memcpy(mymac, buf, 6);

	pr_info("[%s] Exiting. MAC %02X:%02X:%02X:%02X:%02X:%02X\n",
			__FUNCTION__,
			buf[0], buf[1], buf[2], buf[3], buf[4], buf[5] );

	return 0;
}

static ssize_t macaddr_show(struct device *dev, struct device_attribute *attr,
				char *buf)
{
	return sprintf(buf, "%s", intf_macaddr);
}

static ssize_t macaddr_store(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t count)
{
	return snprintf(intf_macaddr, count, "%s\n", buf);
}

DEVICE_ATTR(macaddr, 0644, macaddr_show, macaddr_store);

static struct attribute *wifi_attrs[] = {
	&dev_attr_macaddr.attr,
	NULL
};

static struct attribute_group wifi_attr_grp = {
	.attrs = wifi_attrs,
};

struct wifi_platform_data shinano_wifi_control = {
	.mem_prealloc	= shinano_wifi_mem_prealloc,
	.set_power	= shinano_wifi_set_power,
	.set_reset	= shinano_wifi_set_reset,
	.set_carddetect	= shinano_wifi_set_carddetect,
	.get_mac_addr	= shinano_wifi_get_mac_addr,
};

static struct platform_device shinano_wifi = {
	.name		= "bcmdhd_wlan",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(shinano_wifi_resources),
	.resource	= shinano_wifi_resources,
	.dev		= {
		.platform_data = &shinano_wifi_control,
	},
};

static int __init shinano_wifi_init(void)
{
	if (shinano_wifi_init_mem())
		return -ENOMEM;
	shinano_wifi.resource->start = gpio_to_irq(WIFI_IRQ_GPIO);
	shinano_wifi.resource->end = gpio_to_irq(WIFI_IRQ_GPIO);
	platform_device_register(&shinano_wifi);

	intf_macaddr = kzalloc(20*(sizeof(char)), GFP_KERNEL);
	if (sysfs_create_group(&shinano_wifi.dev.kobj, &wifi_attr_grp) < 0) {
		pr_err("%s: Unable to create sysfs\n", __func__);
		kfree(intf_macaddr);
	}

	return 0;
}

device_initcall(shinano_wifi_init);
