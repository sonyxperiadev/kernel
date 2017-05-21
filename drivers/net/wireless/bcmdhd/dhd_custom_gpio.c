/*
 * Customer code to add GPIO control during WLAN start/stop
 *
 * Copyright (C) 1999-2017, Broadcom Corporation
 * Copyright (C) 2013 Sony Mobile Communications Inc.
 * 
 *      Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed to you
 * under the terms of the GNU General Public License version 2 (the "GPL"),
 * available at http://www.broadcom.com/licenses/GPLv2.php, with the
 * following added to such license:
 * 
 *      As a special exception, the copyright holders of this software give you
 * permission to link this software with independent modules, and to copy and
 * distribute the resulting executable under terms of your choice, provided that
 * you also meet, for each linked independent module, the terms and conditions of
 * the license of that module.  An independent module is a module which is not
 * derived from this software.  The special exception does not apply to any
 * modifications of the software.
 * 
 *      Notwithstanding the above, under no circumstances may you combine this
 * software in any way with any other Broadcom software provided under a license
 * other than the GPL, without Broadcom's express prior written consent.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: dhd_custom_gpio.c 586390 2015-09-15 10:34:19Z $
 */

#include <typedefs.h>
#include <linuxver.h>
#include <osl.h>
#include <bcmutils.h>
#include <dngl_stats.h>
#include <dhd.h>
#include <dhd_linux.h>

#include <wlioctl.h>
#if defined(WL_WIRELESS_EXT)
#include <wl_iw.h>
#endif

#define WL_ERROR(x) printf x
#define WL_TRACE(x)

#if defined(CUSTOMER_HW2)


#endif 

#ifdef GET_CUSTOM_MAC_ENABLE
#define MACADDR_BUF_LEN 64
#define MACADDR_PATH "/data/etc/wlan_macaddr0"
#endif /* GET_CUSTOM_MAC_ENABLE */

#if defined(OOB_INTR_ONLY)

#if defined(BCMLXSDMMC)
extern int sdioh_mmc_irq(int irq);
#endif /* (BCMLXSDMMC)  */


/* Customer specific Host GPIO defintion  */
static int dhd_oob_gpio_num = -1;

module_param(dhd_oob_gpio_num, int, 0644);
MODULE_PARM_DESC(dhd_oob_gpio_num, "DHD oob gpio number");

/* This function will return:
 *  1) return :  Host gpio interrupt number per customer platform
 *  2) irq_flags_ptr : Type of Host interrupt as Level or Edge
 *
 *  NOTE :
 *  Customer should check his platform definitions
 *  and his Host Interrupt spec
 *  to figure out the proper setting for his platform.
 *  Broadcom provides just reference settings as example.
 *
 */
int dhd_customer_oob_irq_map(void *adapter, unsigned long *irq_flags_ptr)
{
	int  host_oob_irq = 0;

#if defined(CUSTOMER_HW2)
	host_oob_irq = wifi_platform_get_irq_number(adapter, irq_flags_ptr);

#else
#if defined(CUSTOM_OOB_GPIO_NUM)
	if (dhd_oob_gpio_num < 0) {
		dhd_oob_gpio_num = CUSTOM_OOB_GPIO_NUM;
	}
#endif /* CUSTOMER_OOB_GPIO_NUM */

	if (dhd_oob_gpio_num < 0) {
		WL_ERROR(("%s: ERROR customer specific Host GPIO is NOT defined \n",
		__FUNCTION__));
		return (dhd_oob_gpio_num);
	}

	WL_ERROR(("%s: customer specific Host GPIO number is (%d)\n",
	         __FUNCTION__, dhd_oob_gpio_num));

#endif 

	return (host_oob_irq);
}
#endif 

/* Customer function to control hw specific wlan gpios */
int
dhd_customer_gpio_wlan_ctrl(void *adapter, int onoff)
{
	int err = 0;

	return err;
}

#ifdef GET_CUSTOM_MAC_ENABLE
int somc_get_mac_address(unsigned char *buf)
{
	int ret = -EINVAL;
	int len;
	unsigned char macaddr_buf[MACADDR_BUF_LEN];
	void *fp = NULL;
	struct ether_addr eth;

	if (!buf)
		return -EINVAL;

	fp = dhd_os_open_image(MACADDR_PATH);
	if (!fp) {
		WL_ERROR(("%s: file open error\n", __FUNCTION__));
		goto err;
	}

	len = dhd_os_get_image_block(macaddr_buf, MACADDR_BUF_LEN, fp);
	if (len <= 0 || MACADDR_BUF_LEN <= len) {
		WL_ERROR(("%s: file read error\n", __FUNCTION__));
		goto err;
	}
	macaddr_buf[len] = '\0';

	/* convert mac address */
	ret = !bcm_ether_atoe(macaddr_buf, &eth);
	if (ret) {
		WL_ERROR(("%s: convert mac value fail\n", __FUNCTION__));
		goto err;
	}

	memcpy(buf, eth.octet, ETHER_ADDR_LEN);
err:
	if (fp)
		dhd_os_close_image(fp);
	return ret;
}

/* Function to get custom MAC address */
int
dhd_custom_get_mac_address(void *adapter, unsigned char *buf)
{
	int ret = 0;

	WL_TRACE(("%s Enter\n", __FUNCTION__));
	if (!buf)
		return -EINVAL;

	/* Customer access to MAC address stored outside of DHD driver */
#if defined(CUSTOMER_HW2) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
	ret = wifi_platform_get_mac_addr(adapter, buf);
#endif

#ifdef EXAMPLE_GET_MAC
	/* EXAMPLE code */
	{
		struct ether_addr ea_example = {{0x00, 0x11, 0x22, 0x33, 0x44, 0xFF}};
		bcopy((char *)&ea_example, buf, sizeof(struct ether_addr));
	}
#endif /* EXAMPLE_GET_MAC */

	return ret;
}
#endif /* GET_CUSTOM_MAC_ENABLE */

#if !defined(WL_WIRELESS_EXT)
struct cntry_locales_custom {
	char iso_abbrev[WLC_CNTRY_BUF_SZ];	/* ISO 3166-1 country abbreviation */
	char custom_locale[WLC_CNTRY_BUF_SZ];	/* Custom firmware locale */
	int32 custom_locale_rev;		/* Custom local revisin default -1 */
};
#endif /* WL_WIRELESS_EXT */

/* Customized Locale table : OPTIONAL feature */
const struct cntry_locales_custom translate_custom_table[] = {
/* Table should be filled out based on custom platform regulatory requirement */
#ifdef EXAMPLE_TABLE
	{"",   "XY", 4},  /* Universal if Country code is unknown or empty */
	{"US", "US", 69}, /* input ISO "US" to : US regrev 69 */
	{"CA", "US", 69}, /* input ISO "CA" to : US regrev 69 */
	{"EU", "EU", 5},  /* European union countries to : EU regrev 05 */
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
	{"GB", "EU", 5},
	{"KR", "XY", 3},
	{"AU", "XY", 3},
	{"CN", "XY", 3}, /* input ISO "CN" to : XY regrev 03 */
	{"TW", "XY", 3},
	{"AR", "XY", 3},
	{"MX", "XY", 3},
	{"IL", "IL", 0},
	{"CH", "CH", 0},
	{"TR", "TR", 0},
	{"NO", "NO", 0},
#endif /* EXMAPLE_TABLE */
#if defined(CUSTOMER_HW2) && !defined(CUSTOMER_HW5)
#if defined(BCM4335_CHIP)
	{"",   "XZ", 11},  /* Universal if Country code is unknown or empty */
#endif
	{"AE", "AE", 1},
	{"AR", "AR", 1},
	{"AT", "AT", 1},
	{"AU", "AU", 2},
	{"BE", "BE", 1},
	{"BG", "BG", 1},
	{"BN", "BN", 1},
	{"CA", "CA", 2},
	{"CH", "CH", 1},
	{"CY", "CY", 1},
	{"CZ", "CZ", 1},
	{"DE", "DE", 3},
	{"DK", "DK", 1},
	{"EE", "EE", 1},
	{"ES", "ES", 1},
	{"FI", "FI", 1},
	{"FR", "FR", 1},
	{"GB", "GB", 1},
	{"GR", "GR", 1},
	{"HR", "HR", 1},
	{"HU", "HU", 1},
	{"IE", "IE", 1},
	{"IS", "IS", 1},
	{"IT", "IT", 1},
	{"ID", "ID", 1},
	{"JP", "JP", 8},
	{"KR", "KR", 24},
	{"KW", "KW", 1},
	{"LI", "LI", 1},
	{"LT", "LT", 1},
	{"LU", "LU", 1},
	{"LV", "LV", 1},
	{"MA", "MA", 1},
	{"MT", "MT", 1},
	{"MX", "MX", 1},
	{"NL", "NL", 1},
	{"NO", "NO", 1},
	{"PL", "PL", 1},
	{"PT", "PT", 1},
	{"PY", "PY", 1},
	{"RO", "RO", 1},
	{"SE", "SE", 1},
	{"SI", "SI", 1},
	{"SK", "SK", 1},
	{"TR", "TR", 7},
	{"TW", "TW", 1},
	{"IR", "XZ", 11},	/* Universal if Country code is IRAN, (ISLAMIC REPUBLIC OF) */
	{"SD", "XZ", 11},	/* Universal if Country code is SUDAN */
	{"SY", "XZ", 11},	/* Universal if Country code is SYRIAN ARAB REPUBLIC */
	{"GL", "XZ", 11},	/* Universal if Country code is GREENLAND */
	{"PS", "XZ", 11},	/* Universal if Country code is PALESTINIAN TERRITORY, OCCUPIED */
	{"TL", "XZ", 11},	/* Universal if Country code is TIMOR-LESTE (EAST TIMOR) */
	{"MH", "XZ", 11},	/* Universal if Country code is MARSHALL ISLANDS */
#ifdef BCM4330_CHIP
	{"RU", "RU", 1},
	{"US", "US", 5}
#endif

#elif defined(CUSTOMER_HW5)
	/* default ccode/regrev */
	{"",   "XT", 54},	/* Universal if Country code is unknown or empty */
	{"IR", "XT", 54},	/* Universal if Country code is IRAN, (ISLAMIC REPUBLIC OF) */
	{"SD", "XT", 54},	/* Universal if Country code is SUDAN */
	{"SY", "XT", 54},	/* Universal if Country code is SYRIAN ARAB REPUBLIC */
	{"GL", "E0", 979},	/* Universal if Country code is GREENLAND */
	{"PS", "XT", 54},	/* Universal if Country code is PALESTINIAN TERRITORY, OCCUPIED */
	{"TL", "XT", 54},	/* Universal if Country code is TIMOR-LESTE (EAST TIMOR) */
	{"MH", "XT", 54},	/* Universal if Country code is MARSHALL ISLANDS */
	{"CK", "XT", 54},	/* Universal if Country code is Cook Islands */
	{"CU", "XT", 54},	/* Universal if Country code is Cuba */
	{"FO", "XT", 54},	/* Universal if Country code is Faroe Islands */
	{"GI", "XT", 54},	/* Universal if Country code is Gibraltar */
	{"KP", "XT", 54},	/* Universal if Country code is North Korea */
	{"NE", "XT", 54},	/* Universal if Country code is Niger (Republic of the) */
	{"PM", "XT", 54},	/* Universal if Country code is Saint Pierre and Miquelon */
	{"WF", "XT", 54},	/* Universal if Country code is Wallis and Futuna */

	{"XA", "XX", 4},	/* Default country code for INDONESIA */
	{"XC", "XT", 998},	/* Default country code for RUSSIA */
	{"XD", "XT", 65},	/* Default country code for CHILE */
	{"AD", "AD", 0},
	{"AE", "AE", 212},
	{"AF", "AF", 0},
	{"AG", "XT", 54},
	{"AI", "AI", 2},
	{"AL", "AL", 2},
	{"AM", "AM", 0},
	{"AN", "AN", 3},
	{"AO", "AO", 0},
	{"AR", "AR", 212},
	{"AS", "AS", 15},
	{"AT", "E0", 979},
	{"AU", "AU", 212},
	{"AW", "XT", 54},
	{"AZ", "XT", 54},
	{"BA", "BA", 2},
	{"BB", "BB", 0},
	{"BD", "XT", 54},
	{"BE", "E0", 979},
	{"BF", "XT", 54},
	{"BG", "E0", 979},
	{"BH", "BH", 4},
	{"BI", "BI", 0},
	{"BJ", "BJ", 0},
	{"BM", "BM", 15},
	{"BN", "BN", 4},
	{"BO", "XT", 54},
	{"BR", "BR", 212},
	{"BS", "XT", 54},
	{"BT", "XT", 54},
	{"BW", "BW", 1},
	{"BY", "XT", 54},
	{"BZ", "XT", 54},
	{"CA", "CA", 212},
	{"CD", "CD", 0},
	{"CF", "CF", 0},
	{"CG", "CG", 0},
	{"CH", "E0", 979},
	{"CI", "CI", 0},
	{"CL", "CL", 212},
	{"CM", "CM", 0},
	{"CN", "CN", 212},
	{"CO", "CO", 212},
	{"CR", "CR", 21},
	{"CV", "CV", 0},
	{"CX", "CX", 1},
	{"CY", "E0", 979},
	{"CZ", "E0", 979},
	{"DE", "E0", 979},
	{"DJ", "DJ", 0},
	{"DK", "E0", 979},
	{"DM", "XT", 54},
	{"DO", "XT", 54},
	{"DZ", "DZ", 1},
	{"EC", "EC", 23},
	{"EE", "E0", 979},
	{"EG", "EG", 212},
	{"ER", "ER", 0},
	{"ES", "E0", 979},
	{"ET", "ET", 2},
	{"FI", "E0", 979},
	{"FJ", "XT", 54},
	{"FK", "FK", 0},
	{"FM", "XT", 54},
	{"FR", "E0", 979},
	{"GA", "GA", 0},
	{"GB", "E0", 979},
	{"GD", "XT", 54},
	{"GE", "GE", 0},
	{"GF", "GF", 2},
	{"GH", "GH", 0},
	{"GM", "GM", 0},
	{"GN", "GN", 0},
	{"GP", "GP", 2},
	{"GQ", "GQ", 0},
	{"GR", "E0", 979},
	{"GT", "XT", 54},
	{"GU", "GU", 17},
	{"GW", "GW", 0},
	{"GY", "GY", 0},
	{"HK", "HK", 212},
	{"HN", "HN", 0},
	{"HR", "E0", 979},
	{"HT", "HT", 0},
	{"HU", "E0", 979},
	{"ID", "ID", 212},
	{"IE", "E0", 979},
	{"IL", "IL", 7},
	{"IN", "IN", 212},
	{"IQ", "IQ", 0},
	{"IS", "E0", 979},
	{"IT", "E0", 979},
	{"JM", "JM", 0},
	{"JO", "JO", 3},
	{"JP", "JP", 212},
	{"KE", "KE", 0},
	{"KG", "KG", 0},
	{"KH", "KH", 4},
	{"KI", "KI", 1},
	{"KM", "KM", 0},
	{"KN", "XT", 54},
	{"KR", "KR", 212},
	{"KW", "KW", 5},
	{"KY", "KY", 4},
	{"KZ", "KZ", 212},
	{"LA", "LA", 4},
	{"LB", "LB", 6},
	{"LC", "XT", 54},
	{"LI", "E0", 979},
	{"LK", "XT", 54},
	{"LR", "LR", 2},
	{"LS", "LS", 2},
	{"LT", "E0", 979},
	{"LU", "E0", 979},
	{"LV", "E0", 979},
	{"LY", "LY", 0},
	{"MA", "MA", 2},
	{"MC", "E0", 979},
	{"MD", "MD", 2},
	{"ME", "ME", 2},
	{"MF", "XT", 54},
	{"MG", "MG", 0},
	{"MK", "E0", 979},
	{"ML", "ML", 0},
	{"MM", "MM", 0},
	{"MN", "XT", 54},
	{"MO", "MO", 2},
	{"MP", "XT", 54},
	{"MQ", "MQ", 2},
	{"MR", "MR", 2},
	{"MS", "MS", 0},
	{"MT", "E0", 979},
	{"MU", "MU", 2},
	{"MV", "MV", 3},
	{"MW", "XT", 54},
	{"MX", "MX", 212},
	{"MY", "MY", 998},
	{"MZ", "XT", 54},
	{"NA", "XT", 54},
	{"NC", "NC", 0},
	{"NG", "NG", 0},
	{"NI", "NI", 0},
	{"NL", "E0", 979},
	{"NO", "E0", 979},
	{"NP", "NP", 3},
	{"NR", "NR", 0},
	{"NZ", "NZ", 9},
	{"OM", "OM", 4},
	{"PA", "PA", 17},
	{"PE", "PE", 212},
	{"PF", "PF", 0},
	{"PG", "PG", 2},
	{"PH", "PH", 212},
	{"PK", "PK", 0},
	{"PL", "E0", 979},
	{"PR", "PR", 25},
	{"PT", "E0", 979},
	{"PW", "XT", 54},
	{"PY", "PY", 4},
	{"QA", "QA", 0},
	{"RE", "RE", 2},
	{"RO", "E0", 979},
	{"RS", "RS", 2},
	{"RU", "RU", 212},
	{"RW", "XT", 54},
	{"SA", "SA", 212},
	{"SB", "SB", 0},
	{"SC", "XT", 54},
	{"SE", "E0", 979},
	{"SG", "SG", 212},
	{"SI", "E0", 979},
	{"SK", "E0", 979},
	{"SL", "SL", 0},
	{"SM", "SM", 0},
	{"SN", "SN", 2},
	{"SO", "SO", 0},
	{"SR", "SR", 0},
	{"ST", "ST", 0},
	{"SV", "XT", 54},
	{"SZ", "SZ", 0},
	{"TC", "TC", 0},
	{"TD", "TD", 0},
	{"TF", "TF", 0},
	{"TG", "TG", 0},
	{"TH", "TH", 212},
	{"TJ", "TJ", 0},
	{"TM", "TM", 0},
	{"TN", "TN", 0},
	{"TO", "TO", 0},
	{"TR", "E0", 979},
	{"TT", "TT", 5},
	{"TV", "TV", 0},
	{"TW", "TW", 996},
	{"TZ", "XT", 54},
	{"UA", "UA", 999},
	{"UG", "XT", 54},
	{"US", "US", 996},
	{"UY", "UY", 5},
	{"UZ", "XT", 54},
	{"VA", "VA", 2},
	{"VC", "XT", 54},
	{"VE", "VE", 3},
	{"VG", "XT", 54},
	{"VI", "VI", 18},
	{"VN", "XT", 54},
	{"VU", "XT", 54},
	{"WS", "XT", 54},
	{"YE", "XT", 54},
	{"YT", "YT", 2},
	{"ZA", "ZA", 212},
	{"ZM", "ZM", 2},
	{"ZW", "XT", 54},
#endif /* CUSTOMER_HW2 and  CUSTOMER_HW5 */
};


/* Customized Locale convertor
*  input : ISO 3166-1 country abbreviation
*  output: customized cspec
*/
#ifdef CUSTOM_COUNTRY_CODE
void get_customized_country_code(void *adapter, char *country_iso_code,
  wl_country_t *cspec, u32 flags)
#else
void get_customized_country_code(void *adapter, char *country_iso_code, wl_country_t *cspec)
#endif /* CUSTOM_COUNTRY_CODE */
{
#if 0 && (defined(CUSTOMER_HW2) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 39)))

	struct cntry_locales_custom *cloc_ptr;

	if (!cspec)
		return;
#ifdef CUSTOM_COUNTRY_CODE
	cloc_ptr = wifi_platform_get_country_code(adapter, country_iso_code,
	           flags);
#else
	cloc_ptr = wifi_platform_get_country_code(adapter, country_iso_code);
#endif /* CUSTOM_COUNTRY_CODE */
	if (cloc_ptr) {
		strlcpy(cspec->ccode, cloc_ptr->custom_locale, WLC_CNTRY_BUF_SZ);
		cspec->rev = cloc_ptr->custom_locale_rev;
	}
	return;
#else
	int size, i;

	size = ARRAYSIZE(translate_custom_table);

	if (cspec == 0)
		 return;

	if (size == 0)
		 return;

	for (i = 0; i < size; i++) {
		if (strcmp(country_iso_code, translate_custom_table[i].iso_abbrev) == 0) {
			memcpy(cspec->ccode,
				translate_custom_table[i].custom_locale, WLC_CNTRY_BUF_SZ);
			cspec->rev = translate_custom_table[i].custom_locale_rev;
			return;
		}
	}
	/* if no country code matched return first universal code from translate_custom_table */
	memcpy(cspec->ccode, translate_custom_table[0].custom_locale, WLC_CNTRY_BUF_SZ);
	cspec->rev = translate_custom_table[0].custom_locale_rev;
	return;
#endif /* 0 && (defined(CUSTOMER_HW2) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36))) */
}
