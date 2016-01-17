/*
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
#include <linux/gpio.h>
#include <linux/qpnp/pin.h>
#include <linux/regulator/consumer.h>
#include <linux/skbuff.h>
#include <linux/wlan_plat.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/pinctrl/consumer.h>
#include <linux/msm_pcie.h>
#include <linux/platform_device.h>
#include <linux/of_gpio.h>
#include <linux/module.h>
#include <linux/fs.h>
#if defined(CONFIG_BCMDHD_SDIO)
#include <asm/mach/mmc.h>
#include <../drivers/mmc/host/msm_sdcc.h>

static unsigned int g_wifi_detect;
static void *sdc_dev;
void (*sdc_status_cb)(int card_present, void *dev);
static void *wifi_mmc_host;
static struct regulator *wifi_batfet;
static int batfet_ena;
extern void sdio_ctrl_power(struct mmc_host *card, bool onoff);
#endif

static char *intf_macaddr = NULL;


#define WIFI_POWER_PMIC_GPIO 18
#define WIFI_IRQ_GPIO 67

/* These definitions need to be aligned with bcmdhd */
#define WLAN_STATIC_SCAN_BUF 5
#define WLAN_STATIC_DHD_INFO_BUF 7
#define WLAN_STATIC_DHD_IF_FLOW_LKUP 9
#define WLAN_STATIC_DHD_PKTID_MAP 12

#define ESCAN_BUF_SIZE (64 * 1024) /* for WIPHY_ESCAN0 */
#define PREALLOC_WLAN_SEC_NUM 4
#define PREALLOC_WLAN_BUF_NUM 160
#define PREALLOC_WLAN_SECTION_HEADER 24

#define WLAN_DHD_INFO_BUF_SIZE (24 * 1024)
#define WLAN_DHD_IF_FLOW_LKUP_SIZE (36 * 1024)

#define DHD_PKTIDMAP_FIFO_MAX 4
#define WLAN_MAX_PKTID_ITEMS (8192)
#define WLAN_DHD_PKTID_MAP_HDR_SIZE (20 + 4 * (WLAN_MAX_PKTID_ITEMS + 1))
#define WLAN_DHD_PKTID_MAP_ITEM_SIZE (48)
#define WLAN_DHD_PKTID_MAP_SIZE (WLAN_DHD_PKTID_MAP_HDR_SIZE + \
	(DHD_PKTIDMAP_FIFO_MAX * (WLAN_MAX_PKTID_ITEMS + 1) * \
	WLAN_DHD_PKTID_MAP_ITEM_SIZE))

#define WLAN_SECTION_SIZE_0 (PREALLOC_WLAN_BUF_NUM * 128)  /* for PROT */
#define WLAN_SECTION_SIZE_1 0                              /* for RXBUF */
#define WLAN_SECTION_SIZE_2 0                              /* for DATABUF */
#define WLAN_SECTION_SIZE_3 (PREALLOC_WLAN_BUF_NUM * 1024) /* for OSL_BUF */

/* These definitions are copied from bcmdhd */
#define DHD_SKB_1PAGE_BUFSIZE (PAGE_SIZE * 1)
#define DHD_SKB_2PAGE_BUFSIZE (PAGE_SIZE * 2)
#define DHD_SKB_4PAGE_BUFSIZE (PAGE_SIZE * 4)

#define DHD_SKB_1PAGE_BUF_NUM 0
#define DHD_SKB_2PAGE_BUF_NUM 64
#define DHD_SKB_4PAGE_BUF_NUM 0

#define WLAN_SKB_1_2PAGE_BUF_NUM ((DHD_SKB_1PAGE_BUF_NUM) + \
	(DHD_SKB_2PAGE_BUF_NUM))
#define WLAN_SKB_BUF_NUM ((WLAN_SKB_1_2PAGE_BUF_NUM) + \
	(DHD_SKB_4PAGE_BUF_NUM))

static struct sk_buff *wlan_static_skb[WLAN_SKB_BUF_NUM];

struct wifi_mem_prealloc {
	void *mem_ptr;
	unsigned long size;
};

static struct wifi_mem_prealloc wifi_mem_array[PREALLOC_WLAN_SEC_NUM] = {
	{ NULL, (WLAN_SECTION_SIZE_0 + PREALLOC_WLAN_SECTION_HEADER) },
	{ NULL, (WLAN_SECTION_SIZE_1 + PREALLOC_WLAN_SECTION_HEADER) },
	{ NULL, (WLAN_SECTION_SIZE_2 + PREALLOC_WLAN_SECTION_HEADER) },
	{ NULL, (WLAN_SECTION_SIZE_3 + PREALLOC_WLAN_SECTION_HEADER) }
};

struct bcmdhd_platform_data {
	struct platform_device *pdev;
	struct pinctrl *pinctrl;
	struct pinctrl_state *gpio_state_active;
	struct pinctrl_state *gpio_state_suspend;
	unsigned int wlan_reg_on;
	unsigned int pci_number;
};

static struct bcmdhd_platform_data *bcmdhd_data;

static void *wlan_static_scan_buf;
static void *wlan_static_dhd_info_buf;
static void *wlan_static_if_flow_lkup;
static void *wlan_static_dhd_pktid_map;


static int somc_wifi_init_mem(void)
{
	int i;
	for (i = 0; i < WLAN_SKB_BUF_NUM; i++)
		wlan_static_skb[i] = NULL;

	for (i = 0; i < DHD_SKB_1PAGE_BUF_NUM; i++) {
		wlan_static_skb[i] = dev_alloc_skb(DHD_SKB_1PAGE_BUFSIZE);
		if (!wlan_static_skb[i])
			goto err_skb_alloc;
	}

	for (i = DHD_SKB_1PAGE_BUF_NUM; i < WLAN_SKB_1_2PAGE_BUF_NUM; i++) {
		wlan_static_skb[i] = dev_alloc_skb(DHD_SKB_2PAGE_BUFSIZE);
		if (!wlan_static_skb[i])
			goto err_skb_alloc;
	}

	for (i = 0; i < PREALLOC_WLAN_SEC_NUM; i++) {
		if (wifi_mem_array[i].size > 0) {
			wifi_mem_array[i].mem_ptr =
				kzalloc(wifi_mem_array[i].size, GFP_KERNEL);

			if (!wifi_mem_array[i].mem_ptr)
				goto err_mem_alloc;
		}
	}

	wlan_static_scan_buf = kzalloc(ESCAN_BUF_SIZE, GFP_KERNEL);
	if (!wlan_static_scan_buf) {
		printk("%s: failed to allocate wlan_static_scan_buf\n",
			__func__);
		goto err_mem_alloc;
	}

	wlan_static_dhd_info_buf = kzalloc(WLAN_DHD_INFO_BUF_SIZE, GFP_KERNEL);
	if (!wlan_static_dhd_info_buf) {
		printk("%s: failed to allocate wlan_static_dhd_info_buf\n",
			__func__);
		goto err_mem_alloc;
	}

	wlan_static_if_flow_lkup = kzalloc(WLAN_DHD_IF_FLOW_LKUP_SIZE,
		GFP_KERNEL);
	if (!wlan_static_if_flow_lkup) {
		printk("%s: failed to allocate wlan_static_if_flow_lkup\n",
			__func__);
		goto err_mem_alloc;
	}
	wlan_static_dhd_pktid_map = kzalloc(WLAN_DHD_PKTID_MAP_SIZE,
		GFP_KERNEL);
	if (!wlan_static_dhd_pktid_map) {
		printk("%s: failed to allocate wlan_static_dhd_pktid_map\n",
			__func__);
		goto err_mem_alloc;
	}

	printk("%s: Wi-Fi static memory allocated\n", __func__);
	return 0;

err_mem_alloc:
	kzfree(wlan_static_dhd_pktid_map);
	kzfree(wlan_static_if_flow_lkup);
	kzfree(wlan_static_dhd_info_buf);
	kzfree(wlan_static_scan_buf);
	printk("%s: failed to allocate mem_alloc\n", __func__);
	for (i--; i >= 0; i--) {
		kzfree(wifi_mem_array[i].mem_ptr);
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

#if defined(CONFIG_BCMDHD_SDIO)
int wcf_status_register(void (*cb)(int card_present, void *dev), void *dev)
{
	pr_info("%s\n", __func__);

	if (sdc_status_cb)
		return -EINVAL;

	sdc_status_cb = cb;
	sdc_dev = dev;
	wifi_mmc_host = ((struct msmsdcc_host *)dev)->mmc;

	return 0;
}

unsigned int wcf_status(struct device *dev)
{
	pr_info("%s: wifi_detect = %d\n", __func__, g_wifi_detect);
	return g_wifi_detect;
}
#endif

static void *somc_wifi_mem_prealloc(int section, unsigned long size)
{
	if (section == PREALLOC_WLAN_SEC_NUM)
		return wlan_static_skb;
	if (section == WLAN_STATIC_SCAN_BUF)
		return wlan_static_scan_buf;

	if (section == WLAN_STATIC_DHD_INFO_BUF) {
		if (size > WLAN_DHD_INFO_BUF_SIZE) {
			printk("%s: request DHD_INFO size(%lu) is bigger than"
				" static size(%d).\n", __func__, size,
				WLAN_DHD_INFO_BUF_SIZE);
			return NULL;
		}
		return wlan_static_dhd_info_buf;
	}

	if (section == WLAN_STATIC_DHD_IF_FLOW_LKUP)  {
		if (size > WLAN_DHD_IF_FLOW_LKUP_SIZE) {
			printk("%s: request DHD_IF_FLOW size(%lu) is bigger"
				" than static size(%d).\n", __func__, size,
				WLAN_DHD_IF_FLOW_LKUP_SIZE);
			return NULL;
		}
		return wlan_static_if_flow_lkup;
	}

	if (section == WLAN_STATIC_DHD_PKTID_MAP)  {
		if (size > WLAN_DHD_PKTID_MAP_SIZE) {
			printk("%s: request DHD_PKTID size(%lu) is"
				" bigger than static size(%d).\n", __func__,
				size, WLAN_DHD_PKTID_MAP_SIZE);
			return NULL;
		}
		return wlan_static_dhd_pktid_map;
	}

	if ((section < 0) || (section > PREALLOC_WLAN_SEC_NUM))
		return NULL;
	if (size > wifi_mem_array[section].size)
		return NULL;
	return wifi_mem_array[section].mem_ptr;
}

int somc_wifi_set_power(int on)
{
#if defined(CONFIG_BCMDHD_SDIO)
	int gpio = qpnp_pin_map("pm8941-gpio", WIFI_POWER_PMIC_GPIO);
	int ret;

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
			ret = regulator_enable(wifi_batfet);
			if (ret != 0)
				pr_warn("%s: Can't enable batfet regulator!\n",
								__func__);
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

	sdio_ctrl_power((struct mmc_host *)wifi_mmc_host, on);
#else
	gpio_set_value(bcmdhd_data->wlan_reg_on, on);
#endif
	return 0;

}

static int somc_wifi_set_reset(int on)
{
	return 0;
}

int somc_wifi_set_carddetect(int present)
{
#if defined(CONFIG_BCMDHD_SDIO)
	g_wifi_detect = present;

	if (sdc_status_cb)
		sdc_status_cb(present, sdc_dev);
	else
		printk(KERN_WARNING "%s: Nobody to notify\n", __func__);
	return 0;
#else
	int ret = 0;
	if (present)
		ret = msm_pcie_enumerate(bcmdhd_data->pci_number);
	return ret;
#endif
}

int somc_wifi_init(struct platform_device *pdev)
{
	int ret, ret_sus, gpio;
	struct pinctrl *pinctrl;
	struct pinctrl_state *lookup_state;

	bcmdhd_data = kzalloc(sizeof(*bcmdhd_data), GFP_KERNEL);
	if (!bcmdhd_data) {
		dev_err(&pdev->dev, "%s: no memory\n", __func__);
		ret = -ENOMEM;
		goto err_alloc_bcmdhd_data;
	}

	bcmdhd_data->pdev = pdev;
	pinctrl = devm_pinctrl_get(&pdev->dev);
	if (IS_ERR(pinctrl)) {
		dev_err(&pdev->dev, "%s: pinctrl not defined\n", __func__);
		ret = PTR_ERR(pinctrl);
		goto err_pinctrl;
	}
	bcmdhd_data->pinctrl = pinctrl;

	lookup_state = pinctrl_lookup_state(pinctrl, PINCTRL_STATE_DEFAULT);
	if (IS_ERR_OR_NULL(lookup_state)) {
		dev_err(&pdev->dev, "%s: pinctrl lookup failed for default\n",
			__func__);
		ret = PTR_ERR(lookup_state);
		goto err_pinctrl;
	}
	bcmdhd_data->gpio_state_active = lookup_state;

	lookup_state = pinctrl_lookup_state(pinctrl, PINCTRL_STATE_SLEEP);
	if (IS_ERR_OR_NULL(lookup_state)) {
		dev_err(&pdev->dev, "%s: pinctrl lookup failed for sleep\n",
			__func__);
		ret = PTR_ERR(lookup_state);
		goto err_pinctrl;
	}
	bcmdhd_data->gpio_state_suspend = lookup_state;

	ret = pinctrl_select_state(bcmdhd_data->pinctrl,
		bcmdhd_data->gpio_state_active);
	if (ret) {
		dev_err(&pdev->dev, "%s: failed to select active state\n",
			__func__);
		goto err_pinctrl;
	}

	gpio = of_get_gpio(pdev->dev.of_node, 0);
	if (!gpio_is_valid(gpio)) {
		dev_err(&pdev->dev, "%s: invalid gpio #%s: %d\n",
			__func__, "wlan-reg-on", gpio);
		ret = -ENXIO;
		goto err_gpio;
	}
	bcmdhd_data->wlan_reg_on = gpio;

	ret = gpio_request(bcmdhd_data->wlan_reg_on,
			"wlan-reg-on");
	if (ret) {
		dev_err(&pdev->dev, "%s: request err %s: %d\n",
			__func__, "wlan-reg-on", ret);
		goto err_gpio;
	}

	ret = of_property_read_u32(pdev->dev.of_node, "wlan-pci-number",
		&bcmdhd_data->pci_number);
	if (ret < 0) {
		dev_err(&pdev->dev, "%s: failed to find PCI number %d %d \n",
			__func__, ret, bcmdhd_data->pci_number);
		goto err_gpio_request;
	}

	return 0;

err_gpio_request:
	gpio_free(bcmdhd_data->wlan_reg_on);
err_gpio:
	ret_sus = pinctrl_select_state(bcmdhd_data->pinctrl,
			bcmdhd_data->gpio_state_suspend);
	if (ret_sus)
		dev_err(&pdev->dev, "%s: failed to select suspend state\n",
			__func__);
err_pinctrl:
	kzfree(bcmdhd_data);
err_alloc_bcmdhd_data:
	return ret;
}
EXPORT_SYMBOL(somc_wifi_init);

void somc_wifi_deinit(struct platform_device *pdev)
{
	if (bcmdhd_data) {
		if (gpio_is_valid(bcmdhd_data->wlan_reg_on))
			gpio_free(bcmdhd_data->wlan_reg_on);
		if (!IS_ERR_OR_NULL(bcmdhd_data->pinctrl) &&
			!IS_ERR_OR_NULL(bcmdhd_data->gpio_state_suspend)) {
			int ret = pinctrl_select_state(bcmdhd_data->pinctrl,
				bcmdhd_data->gpio_state_suspend);
			if (ret)
				dev_err(&pdev->dev, "%s: failed to select"
					" suspend state\n", __func__);
		}
		kzfree(bcmdhd_data);
	}
}
EXPORT_SYMBOL(somc_wifi_deinit);

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

static int somc_wifi_get_mac_addr(unsigned char *buf)
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
	filp_close(fp, NULL);
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

	return ret;

random_mac:

	pr_debug("%s: %p\n", __func__, buf);

	if (memcmp( mymac, nullmac, ETHER_ADDR_LEN) != 0) {
		/* Mac displayed from UI is never updated..
		   So, mac obtained on initial time is used */
		memcpy(buf, mymac, ETHER_ADDR_LEN);
		return 0;
	}

	prandom_seed((uint)jiffies);
	rand_mac = prandom_u32();
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

struct wifi_platform_data somc_wifi_control = {
	.set_power	= somc_wifi_set_power,
#if defined(CONFIG_BCMDHD_SDIO)
	.set_reset	= somc_wifi_set_reset,
#endif
	.set_carddetect	= somc_wifi_set_carddetect,
	.mem_prealloc	= somc_wifi_mem_prealloc,
	.get_mac_addr	= somc_wifi_get_mac_addr,
};

EXPORT_SYMBOL(somc_wifi_control);

#if defined(CONFIG_BCMDHD_SDIO)
static struct resource somc_wifi_resources[] = {
	[0] = {
		.name	= "bcmdhd_wlan_irq",
		.start	= 0,
		.end	= 0,
		.flags	= IORESOURCE_IRQ | IORESOURCE_IRQ_HIGHLEVEL |
			  IORESOURCE_IRQ_SHAREABLE,
	},
};

static struct platform_device somc_wifi = {
	.name		= "bcmdhd_wlan",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(somc_wifi_resources),
	.resource	= somc_wifi_resources,
	.dev		= {
		.platform_data = &somc_wifi_control,
	},
};
#endif

static int __init somc_wifi_init_on_boot(void)
{
	if (somc_wifi_init_mem())
		return -ENOMEM;
#if defined(CONFIG_BCMDHD_SDIO)
	somc_wifi.resource->start = gpio_to_irq(WIFI_IRQ_GPIO);
	somc_wifi.resource->end = gpio_to_irq(WIFI_IRQ_GPIO);
	platform_device_register(&somc_wifi);

	intf_macaddr = kzalloc(20*(sizeof(char)), GFP_KERNEL);
	if (sysfs_create_group(&somc_wifi.dev.kobj, &wifi_attr_grp) < 0) {
		pr_err("%s: Unable to create sysfs\n", __func__);
		kfree(intf_macaddr);
	}
#endif
	return 0;
}

device_initcall(somc_wifi_init_on_boot);

MODULE_LICENSE("GPL v2");
