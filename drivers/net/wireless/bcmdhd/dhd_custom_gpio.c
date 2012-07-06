/*
* Customer code to add GPIO control during WLAN start/stop
* Copyright (C) 1999-2011, Broadcom Corporation
* Copyright (C) 2012 Sony Mobile Communications AB.
*
*         Unless you and Broadcom execute a separate written software license
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
* $Id: dhd_custom_gpio.c 275786 2011-08-04 22:42:42Z $
*
* NOTE: This file has been modified by Sony Mobile Communications AB.
* Modifications are licensed under the Sony Mobile Communications AB.
* End User License Agreement ("EULA"). Any use of the modifications is subject
* to the terms of the EULA.
*/

#include <typedefs.h>
#include <linuxver.h>
#include <osl.h>
#include <bcmutils.h>

#include <dngl_stats.h>
#include <dhd.h>

#include <wlioctl.h>
#include <wl_iw.h>
#include <wl_android.h>

#include <asm/gpio.h>

#define WL_ERROR(x) printf x
#define WL_TRACE(x)

#ifdef GET_CUSTOM_MAC_ENABLE
#define MAC_LEN 6
#define MACADDR_BUF_LEN            64
#define MACADDR_PATH "/data/etc/wifi/fw"
#endif /* GET_CUSTOM_MAC_ENABLE */

#ifdef CUSTOMER_HW
#define WL_RST_N (130)
int wifi_set_carddetect(int on);
#endif

#if defined(OOB_INTR_ONLY)

#if defined(BCMLXSDMMC)
extern int sdioh_mmc_irq(int irq);
#endif /* (BCMLXSDMMC)  */

#ifdef CUSTOMER_HW3
#include <mach/gpio.h>
#endif

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
int dhd_customer_oob_irq_map(unsigned long *irq_flags_ptr)
{
	int  host_oob_irq = 0;

#if defined(CUSTOM_OOB_GPIO_NUM)
	if (dhd_oob_gpio_num < 0) {
		dhd_oob_gpio_num = CUSTOM_OOB_GPIO_NUM;
	}
#endif

	if (dhd_oob_gpio_num < 0) {
		WL_ERROR(("%s: ERROR customer specific Host GPIO is NOT defined \n",
			__FUNCTION__));
		return (dhd_oob_gpio_num);
	}

	WL_ERROR(("%s: customer specific Host GPIO number is (%d)\n",
	         __FUNCTION__, dhd_oob_gpio_num));

#if defined CUSTOMER_HW
	gpio_request(dhd_oob_gpio_num, "WL_HOST_WAKEUP");
	host_oob_irq = MSM_GPIO_TO_INT(dhd_oob_gpio_num);
	gpio_direction_input(dhd_oob_gpio_num);
#endif /* CUSTOMER_HW */

	return (host_oob_irq);
}
#endif /* defined(OOB_INTR_ONLY) */

/* Customer function to control hw specific wlan gpios */
void
dhd_customer_gpio_wlan_ctrl(int onoff)
{
	switch (onoff) {
		case WLAN_RESET_OFF:
			WL_TRACE(("%s: call customer specific GPIO to insert WLAN RESET_OFF\n",
				__FUNCTION__));
#ifdef CUSTOMER_HW
			gpio_set_value(WL_RST_N, 0);
#endif /* CUSTOMER_HW */
			WL_TRACE(("=========== WLAN placed in RESET ========\n"));
		break;

		case WLAN_RESET_ON:
			WL_TRACE(("%s: callc customer specific GPIO to remove WLAN RESET_ON\n",
				__FUNCTION__));
#ifdef CUSTOMER_HW
			gpio_set_value(WL_RST_N, 1);
#endif /* CUSTOMER_HW */
			WL_TRACE(("=========== WLAN going back to live  ========\n"));
		break;

		case WLAN_POWER_OFF:
			WL_TRACE(("%s: call customer specific GPIO to turn off WL_REG_OFF\n",
				__FUNCTION__));
#ifdef CUSTOMER_HW
			gpio_set_value(WL_RST_N, 0);
#endif /* CUSTOMER_HW */
		break;

		case WLAN_POWER_ON:
			WL_TRACE(("%s: call customer specific GPIO to turn on WL_REG_ON\n",
				__FUNCTION__));
#ifdef CUSTOMER_HW
			gpio_set_value(WL_RST_N, 1);
#endif /* CUSTOMER_HW */
		break;
	}

	/* Lets customer power to get stable */
	OSL_DELAY(200);

}

#ifdef GET_CUSTOM_MAC_ENABLE
/* Function to get custom MAC address */
int
dhd_custom_get_mac_address(unsigned char *buf)
{
	int ret = 0;
	int length;
	mm_segment_t oldfs;
	struct file *filp;
	struct inode *inode = NULL;
	unsigned char macaddr_buf[MACADDR_BUF_LEN];

	WL_TRACE(("%s Enter\n", __FUNCTION__));
	if (!buf)
		return -EINVAL;

	oldfs = get_fs();
	set_fs(KERNEL_DS);

	filp = filp_open(MACADDR_PATH, O_RDONLY, S_IRUSR);
	if (IS_ERR(filp)) {
		WL_ERROR(("%s: file filp_open error\n", __FUNCTION__));
		set_fs(oldfs);
		return -1;
	}
	if (!filp->f_op) {
		WL_ERROR(("%s: File Operation Method Error\n", __FUNCTION__));
		filp_close(filp, NULL);
		set_fs(oldfs);
		return -1;
	}
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,20)
	inode = filp->f_path.dentry->d_inode;
#else
	inode = filp->f_dentry->d_inode;
#endif
	if (!inode) {
		WL_ERROR(("%s: Get inode from filp failed\n", __FUNCTION__));
		filp_close(filp, NULL);
		set_fs(oldfs);
		return -1;
	}

	/* check file size */
	length = i_size_read(inode->i_mapping->host);
	if (length+1 > MACADDR_BUF_LEN) {
		WL_ERROR(("%s: MAC file's size is not as expected\n", __FUNCTION__));
		filp_close(filp, NULL);
		set_fs(oldfs);
		return -1;
	}

	/* read mac address */
	if (filp->f_op->read(filp, macaddr_buf, length, &filp->f_pos) != length) {
		WL_ERROR(("%s: file read error\n", __FUNCTION__));
		filp_close(filp, NULL);
		set_fs(oldfs);
		return -1;
	}
	macaddr_buf[length] = '\0';

	/* read mac address success */
	filp_close(filp, NULL);
	set_fs(oldfs);

	/* convert mac address */
	if (!dhd_ether_aton(macaddr_buf, length, buf)) {
		WL_ERROR(("%s: convert mac value fail\n", __FUNCTION__));
		return -1;
	}

	return ret;
}

int
dhd_ether_aton(const char *orig, size_t len, unsigned char *eth)
{
	const char *bufp;
	int i;

	i = 0;
	for(bufp = orig; bufp!=orig+len && *bufp; ++bufp) {
		unsigned int val;
		unsigned char c = *bufp++;

		if (c >= '0' && c <= '9') val = c - '0';
		else if (c >= 'a' && c <= 'f') val = c - 'a' + 10;
		else if (c >= 'A' && c <= 'F') val = c - 'A' + 10;
		else {
			WL_ERROR(("%s: MAC value is invalid\n", __FUNCTION__));
			break;
		}

		val <<= 4;
		c = *bufp++;
		if (c >= '0' && c <= '9') val |= c - '0';
		else if (c >= 'a' && c <= 'f') val |= c - 'a' + 10;
		else if (c >= 'A' && c <= 'F') val |= c - 'A' + 10;
		else {
			WL_ERROR(("%s: MAC value is invalid\n", __FUNCTION__));
			break;
		}

		eth[i] = (unsigned char) (val & 0377);
		if(++i == MAC_LEN) {
			return 1;
		}
		if (*bufp != ':')
			break;
	}
	return 0;
}
#endif /* GET_CUSTOM_MAC_ENABLE */

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
};


/* Customized Locale convertor
*  input : ISO 3166-1 country abbreviation
*  output: customized cspec
*/
void get_customized_country_code(char *country_iso_code, wl_country_t *cspec)
{
#if defined(CUSTOMER_HW2) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 39))

	struct cntry_locales_custom *cloc_ptr;

	if (!cspec)
		return;

	cloc_ptr = wifi_get_country_code(country_iso_code);
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
#ifdef EXAMPLE_TABLE
	/* if no country code matched return first universal code from translate_custom_table */
	memcpy(cspec->ccode, translate_custom_table[0].custom_locale, WLC_CNTRY_BUF_SZ);
	cspec->rev = translate_custom_table[0].custom_locale_rev;
#endif /* EXMAPLE_TABLE */
	return;
#endif /* defined(CUSTOMER_HW2) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)) */
}
