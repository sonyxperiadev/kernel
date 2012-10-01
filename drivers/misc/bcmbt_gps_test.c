/*******************************************************************************
* Copyright 2012 Broadcom Corporation.  All rights reserved.
*
* @file	drivers/misc/bcmbt_gps_test.c
*
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

/* #define DEBUG
#define VERBOSE_DEBUG */

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/err.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <mach/gpio.h>
#include <mach/chip_pinmux.h>
#include <mach/pinmux.h>
#include <linux/ioctl.h>
#include <linux/broadcom/bcmbt_lpm.h>
#include <linux/broadcom/bcmbt_gps.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/io.h>

/* GPS IO test variables */
struct gps_gpio_data {
	bool setupAsGPIO; /*  = false */
	struct pin_config storedValue;
	int gpio_nr;
};
struct gps_test_cb {
	struct gps_gpio_data pablank;
	struct gps_gpio_data calreq;
};
static struct gps_test_cb *gps_test_cb;
static const char pablank_name[] = "GPS PABLANK";
static const char calreq_name[] = "GPS CALREQ";

static int bcm_gps_setup_gpio(struct gps_gpio_data *p_cb,
		const enum PIN_NAME pin_name, const char *name)
{
	int result = 0;
	int ret;
	enum PIN_FUNC PF_gpio;
	struct pin_config GPIOSetup;

	GPIOSetup.reg.val = 0;
	GPIOSetup.reg.b.drv_sth = DRIVE_STRENGTH_8MA;

	result = pinmux_find_gpio(pin_name,
			&p_cb->gpio_nr,	&PF_gpio);
	if ((result != 0) || (p_cb->setupAsGPIO == true))
		goto out;

	pr_debug("GPS_SELFTEST -- <%s>Setup_as_GPIO", name);
		p_cb->storedValue.name = pin_name;
	pinmux_get_pin_config(&p_cb->storedValue);
	GPIOSetup.name = pin_name;
	GPIOSetup.func = PF_gpio;
	pinmux_set_pin_config(&GPIOSetup);
	pr_debug("GPS_SELFTEST::<%s> value 0x%08X",
			name, p_cb->storedValue.reg.val);
	ret = gpio_request(p_cb->gpio_nr, name);
	if (ret < 0) {
		pinmux_set_pin_config(&p_cb->storedValue);
		pr_warn("GPS SELFTEST::<%s>gpio %d request failed",
				name, p_cb->gpio_nr);
		result = -EINVAL;
	} else
		p_cb->setupAsGPIO = true;
out:
	return result;
}

static int bcm_gps_free_gpio(struct gps_gpio_data *p_cb,
		const char *name)
{
	if (p_cb->setupAsGPIO == true) {
		pr_debug("GPS_SELFTEST _gps_io () <%s>"
			" - Restore", name);
		pinmux_set_pin_config(&p_cb->storedValue);
		gpio_free(p_cb->gpio_nr);
		p_cb->setupAsGPIO = false;
	}
	return 0;
}

static int bcm_selftest_setupandrestore(int setup, struct gps_test_cb **pp_cb)
{
	int result = 0;
	struct gps_test_cb *p_cb;

	if (setup) {
		if (!*pp_cb) {
			*pp_cb = kzalloc(sizeof(struct gps_test_cb),
					GFP_KERNEL);
			if (!*pp_cb)
				return -ENOMEM;
		}
		p_cb = *pp_cb;
		result = bcm_gps_setup_gpio(&p_cb->pablank, PN_GPS_PABLANK,
				pablank_name);
		result = bcm_gps_setup_gpio(&p_cb->calreq, PN_GPS_CALREQ,
				calreq_name);
		if (!p_cb->pablank.setupAsGPIO &&
				!p_cb->calreq.setupAsGPIO) {
			pr_warn("GPS SELFTEST:: unable to allocate GPIOs!");
			goto error_out;
		}
	} else {
		p_cb = *pp_cb;
		if (!p_cb) {
			result = -ENODEV;
			goto out;
		}
		result = bcm_gps_free_gpio(&p_cb->pablank, pablank_name);

		result = bcm_gps_free_gpio(&p_cb->calreq, calreq_name);
error_out:
		kfree(*pp_cb);
		*pp_cb = NULL;
	}
out:
	return result;
}

static int bcm_gps_selftest_set(int state, const struct gps_gpio_data *p_cb,
		const char *test_name)
{
	int result = 0;

	if ((state < GPS_SELFTEST_DEASSERT_PIN) || (state
			> GPS_SELFTEST_ASSERT_PIN)) {
		pr_warn("GPS SELFTEST::%s: out of range: state: %d",
				test_name, state);
		result = -EINVAL;
		goto out;
	}
	if (!p_cb || (!p_cb->setupAsGPIO)) {
		pr_warn("GPS SELFTEST::%s: NOT SETUP!", test_name);
		result = -EINVAL;
		goto out;
	}
	gpio_direction_output(p_cb->gpio_nr, state);
	gpio_set_value(p_cb->gpio_nr, state);
	pr_debug("GPS_SELFTEST::%s-Write-%s", test_name, state ? "High" :
			"Low");
out:
	return result;
}

int bcmbt_gps_selftest(struct bcmbt_gps_selftest_cmd *p_cmd)
{
	int rc = -EINVAL;

	switch (p_cmd->cmd) {
	case GPS_SELFTEST_SETUP:
		rc = bcm_selftest_setupandrestore(p_cmd->param, &gps_test_cb);
		break;
	case GPS_SELFTEST_PABLANK:
	case GPS_SELFTEST_CALREQ:
		if (!gps_test_cb) {
			rc = -ENODEV;
			break;
		}
		rc = bcm_gps_selftest_set(p_cmd->param,
				(p_cmd->cmd == GPS_SELFTEST_PABLANK) ?
						&gps_test_cb->pablank :
						&gps_test_cb->calreq,
				(p_cmd->cmd == GPS_SELFTEST_PABLANK) ?
						pablank_name : calreq_name);
		break;
	default:
		break;
	}
	return rc;
}

int __bcmbt_gps_selftest_init(void)
{
	gps_test_cb = (struct gps_test_cb *)NULL;
	return 0;
}
void __bcmbt_gps_selftest_exit(void)
{
	if (gps_test_cb)
		bcm_selftest_setupandrestore(GPS_SELFTEST_DEINIT, &gps_test_cb);
}
