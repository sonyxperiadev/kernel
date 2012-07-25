/*
* Customer code to add GPIO control during WLAN start/stop
* Copyright (C) 1999-2012, Broadcom Corporation
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
* $Id: dhd_custom_gpio.c 275786 2011-08-04 22:42:42Z $
*/

#include <typedefs.h>
#include <linuxver.h>
#include <osl.h>
#include <bcmutils.h>

#include <dngl_stats.h>
#include <dhd.h>

#include <wlioctl.h>
#include <wl_iw.h>

#define WL_ERROR(x) printf x
#define WL_TRACE(x)

#ifdef CUSTOMER_HW
extern  void bcm_wlan_power_off(int);
extern  void bcm_wlan_power_on(int);
#endif /* CUSTOMER_HW */
#if defined(CUSTOMER_HW2) || defined (CUSTOMER_HW3)
#ifdef CONFIG_WIFI_CONTROL_FUNC
int wifi_set_power(int on, unsigned long msec);
int wifi_get_irq_number(unsigned long *irq_flags_ptr);
int wifi_get_mac_addr(unsigned char *buf);
void *wifi_get_country_code(char *ccode);
#else
int wifi_set_power(int on, unsigned long msec) { return -1; }
int wifi_get_irq_number(unsigned long *irq_flags_ptr) { return -1; }
int wifi_get_mac_addr(unsigned char *buf) { return -1; }
void *wifi_get_country_code(char *ccode) { return NULL; }
#endif /* CONFIG_WIFI_CONTROL_FUNC */
#endif /* CUSTOMER_HW2 */

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

#ifdef CUSTOMER_HW2
	host_oob_irq = wifi_get_irq_number(irq_flags_ptr);

#else
#if defined(CUSTOM_OOB_GPIO_NUM)
	if (dhd_oob_gpio_num < 0) {
		dhd_oob_gpio_num = CUSTOM_OOB_GPIO_NUM;
	}
#endif /* CUSTOMER_HW2 */

	if (dhd_oob_gpio_num < 0) {
		WL_ERROR(("%s: ERROR customer specific Host GPIO is NOT defined \n",
		__FUNCTION__));
		return (dhd_oob_gpio_num);
	}

	WL_ERROR(("%s: customer specific Host GPIO number is (%d)\n",
	         __FUNCTION__, dhd_oob_gpio_num));

#if defined CUSTOMER_HW
	host_oob_irq = MSM_GPIO_TO_INT(dhd_oob_gpio_num);
#elif defined CUSTOMER_HW3
	gpio_request(dhd_oob_gpio_num, "oob irq");
	host_oob_irq = gpio_to_irq(dhd_oob_gpio_num);
	gpio_direction_input(dhd_oob_gpio_num);
#endif /* CUSTOMER_HW */
#endif /* CUSTOMER_HW2 */

	return (host_oob_irq);
}
#endif /* defined(OOB_INTR_ONLY) */

/* Customer function to control hw specific wlan gpios */
void
dhd_customer_gpio_wlan_ctrl(int onoff)
{
	switch (onoff) {
		case WLAN_RESET_OFF:
			WL_TRACE(("%s: call customer specific GPIO to insert WLAN RESET\n",
				__FUNCTION__));
#ifdef CUSTOMER_HW
			bcm_wlan_power_off(2);
#endif /* CUSTOMER_HW */
#if defined CUSTOMER_HW2 || defined CUSTOMER_HW3
			wifi_set_power(0, 0);
#endif
			WL_ERROR(("=========== WLAN placed in RESET ========\n"));
		break;

		case WLAN_RESET_ON:
			WL_TRACE(("%s: callc customer specific GPIO to remove WLAN RESET\n",
				__FUNCTION__));
#ifdef CUSTOMER_HW
			bcm_wlan_power_on(2);
#endif /* CUSTOMER_HW */
#if defined CUSTOMER_HW2 || defined CUSTOMER_HW3
			wifi_set_power(1, 0);
#endif
			WL_ERROR(("=========== WLAN going back to live  ========\n"));
		break;

		case WLAN_POWER_OFF:
			WL_TRACE(("%s: call customer specific GPIO to turn off WL_REG_ON\n",
				__FUNCTION__));
#ifdef CUSTOMER_HW
			bcm_wlan_power_off(1);
#endif /* CUSTOMER_HW */
		break;

		case WLAN_POWER_ON:
			WL_TRACE(("%s: call customer specific GPIO to turn on WL_REG_ON\n",
				__FUNCTION__));
#ifdef CUSTOMER_HW
			bcm_wlan_power_on(1);
			/* Lets customer power to get stable */
			OSL_DELAY(200);
#endif /* CUSTOMER_HW */
		break;
	}
}

#ifdef GET_CUSTOM_MAC_ENABLE

enum {
	MACADDR_NONE =0 ,	// Initial state (nothing done yet)
	MACADDR_MOD_RANDOM,	// Mac address randomization final 3 octets and rewrite
	MACADDR_MOD_NONE	// Mac address already set from file (no need to rewrite)
};

static int g_iMacFlag = MACADDR_NONE;

int CheckRDWR_Macaddr(struct ether_addr *mac)
{
	struct file *fp_mac	= NULL;
	char buf[18]			= {0};
	char* filepath			= "/data/misc/wifi/.mac.info";
	mm_segment_t oldfs		= {0};

	int ret = -1;

	fp_mac = filp_open(filepath, O_RDONLY, 0);

	if(IS_ERR(fp_mac)) {
	/***************************************************************************
	 * No .macinfo file, initialize to dummy values
	 */
		mac->octet[0] = 0x00;
		mac->octet[1] = 0x90;
		mac->octet[2] = 0x4c;
		g_iMacFlag = MACADDR_MOD_RANDOM;
	}
	else {
	/***************************************************************************
	 * File .macinfo found. If zero, it is invalid and the "default" value should be used
	 * and randomized. Otherwise, use that value "as-is" in the .macinfo file
	 */
		int is_zeromac;
		int temp_mac[6];

		ret = kernel_read(fp_mac, 0, buf, 18);
		filp_close(fp_mac, NULL);
		buf[17] ='\0';

		is_zeromac = strncmp(buf, "00:00:00:00:00:00", 17);
		WL_ERROR(("MAC (FILE): [%s] [%d] \r\n" , buf, is_zeromac));

		if (is_zeromac == 0) {
			WL_ERROR(("Zero MAC detected. Trying Random MAC.\n"));
			mac->octet[0] = 0x00;
			mac->octet[1] = 0x90;
			mac->octet[2] = 0x4c;
			g_iMacFlag = MACADDR_MOD_RANDOM;
		} else {
			sscanf(buf,"%02X:%02X:%02X:%02X:%02X:%02X",
			   &(temp_mac[0]), &(temp_mac[1]), &(temp_mac[2]),
			   &(temp_mac[3]), &(temp_mac[4]), &(temp_mac[5]));
			mac->octet[0] = (char) temp_mac[0];
			mac->octet[1] = temp_mac[1];
			mac->octet[2] = temp_mac[2];
			mac->octet[3] = temp_mac[3];
			mac->octet[4] = temp_mac[4];
			mac->octet[5] = temp_mac[5];
			g_iMacFlag = MACADDR_MOD_NONE;
		}
	}

	if(g_iMacFlag == MACADDR_MOD_RANDOM) {
		unsigned long int rand_mac;
		srandom32((uint)jiffies);	/* Seed the random part of mac */
		rand_mac = random32();
		mac->octet[3] = (unsigned char)(rand_mac & 0x0F) | 0xF0;
		mac->octet[4] = (unsigned char)(rand_mac >> 8);
		mac->octet[5] = (unsigned char)(rand_mac >> 16);

		WL_ERROR(("MAC (Randomized) : [%02X:%02X:%02X:%02X:%02X:%02X] \r\n" ,
		  mac->octet[0],mac->octet[1],mac->octet[2],mac->octet[3],mac->octet[4],mac->octet[5]));
	}

	if (g_iMacFlag == MACADDR_MOD_NONE) {
		return 0;	// File is already there (no need to rewrite)
	}
	else if (g_iMacFlag == MACADDR_NONE) {
		WL_ERROR(("Logical error in CheckRDWR_Macaddr()!\n"));
		return -1;	// Should not get here!
	}
	else {
	/**************************************************
	 * Write back the randomized value into a the .mac.info file
	 */
		sprintf(buf,"%02X:%02X:%02X:%02X:%02X:%02X\n",
			mac->octet[0],mac->octet[1],mac->octet[2],
			mac->octet[3],mac->octet[4],mac->octet[5]);

		fp_mac = filp_open(filepath, O_RDWR | O_CREAT, 0666); // File is always created.
		if(IS_ERR(fp_mac)) {
			WL_ERROR(("[WIFI] %s: File open error\n", filepath));
			return -1;
		}
		else {
			oldfs = get_fs();
			set_fs(get_ds());

			if(fp_mac->f_mode & FMODE_WRITE) {
				ret = fp_mac->f_op->write(fp_mac, (const char *)buf, sizeof(buf), &fp_mac->f_pos);
				if(ret < 0)
					WL_ERROR(("[WIFI] Mac address [%s] Failed to write into File: %s\n", buf, filepath));
				else
					WL_TRACE(("[WIFI] Mac address [%s] written into File: %s\n", buf, filepath));
			}
			set_fs(oldfs);
			filp_close(fp_mac, NULL);
		}
	}

	return 0;
}


int
dhd_custom_get_mac_address(unsigned char *buf)
{

	WL_ERROR(("%s Enter\n", __FUNCTION__));
	if (!buf)
		return -EINVAL;

/**********************************************
 * Custom code written to randomly generate only once per phone power up.
 *
 */
	return (CheckRDWR_Macaddr((struct ether_addr *) buf));


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
