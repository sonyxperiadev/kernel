/*
 * siw_touch_sys.c - SiW touch system interface
 *
 * Copyright (C) 2016 Silicon Works - http://www.siliconworks.co.kr
 * Copyright (C) 2018 Sony Mobile Communications Inc.
 * Author: Hyunho Kim <kimhh@siliconworks.co.kr>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/err.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/irqreturn.h>
#include <linux/slab.h>
#include <linux/pm.h>
#include <linux/gpio.h>
#include <linux/string.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/of_device.h>
#include <asm/page.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/memory.h>

#if defined(CONFIG_PLAT_SAMSUNG)
#include <plat/cpu.h>
#include <plat/gpio-core.h>
#include <plat/gpio-cfg.h>
#include <plat/gpio-cfg-helpers.h>
#include <plat/pm.h>
#endif

#include "siw_touch.h"
#include "siw_touch_gpio.h"
#include "siw_touch_sys.h"

#if defined(CONFIG_TOUCHSCREEN_SIW_LG4895_F650) ||	\
	defined(CONFIG_TOUCHSCREEN_SIW_LG4946_F700)
#include <soc/qcom/lge/board_lge.h>
#endif


/*
 * depends on AP bus
 */
int siw_touch_sys_bus_use_dma(struct device *dev)
{
	struct siw_ts *ts = to_touch_core(dev);

	if (touch_bus_type(ts) != BUS_IF_SPI) {
		return 0;
	}

#if defined(__If_the_bus_of_your_chipset_needs_dma_control__)
	{
		return 1;
	}
#elif defined(CONFIG_ARCH_EXYNOS5)
	{
		return 0;
	}
#endif

	return 0;
}

int siw_touch_get_boot_mode(void)
{
#if defined(CONFIG_SIW_GET_BOOT_MODE)
	if (sys_get_boot_mode() == BOOT_MODE_CHARGERLOGO) {
		return SIW_TOUCH_CHARGER_MODE;
	}
#elif defined(CONFIG_TOUCHSCREEN_SIW_LG4895_F650) ||	\
	defined(CONFIG_TOUCHSCREEN_SIW_LG4946_F700)
	if (lge_get_boot_mode() == LGE_BOOT_MODE_CHARGERLOGO) {
		return SIW_TOUCH_CHARGER_MODE;
	}
#endif

	return 0;
}

int siw_touch_boot_mode_check(struct device *dev)
{
	struct siw_ts *ts = to_touch_core(dev);
	int ret = NORMAL_BOOT;

#if defined(CONFIG_SIW_GET_FACTORY_MODE)
	ret = sys_get_factory_boot();
#elif defined(CONFIG_TOUCHSCREEN_SIW_LG4895_F650) ||	\
	defined(CONFIG_TOUCHSCREEN_SIW_LG4946_F700)
	ret = lge_get_factory_boot();
#endif

	if (ret != NORMAL_BOOT) {
		switch (atomic_read(&ts->state.mfts)) {
			case MFTS_NONE :
				ret = MINIOS_AAT;
				break;
			case MFTS_FOLDER :
				ret = MINIOS_MFTS_FOLDER;
				break;
			case MFTS_FLAT :
				ret = MINIOS_MFTS_FLAT;
				break;
			case MFTS_CURVED :
				ret = MINIOS_MFTS_CURVED;
				break;
			default :
				ret = MINIOS_AAT;
				break;
		}
	}

	return ret;
}

int siw_touch_boot_mode_tc_check(struct device *dev)
{
	int ret = 0;

#if defined(CONFIG_SIW_GET_BOOT_MODE)
	if (sys_get_boot_mode() == BOOT_MODE_CHARGERLOGO) {
		return 1;
	}
#elif defined(CONFIG_TOUCHSCREEN_SIW_LG4895_F650)
	if ((lge_get_boot_mode() == LGE_BOOT_MODE_QEM_910K) ||
		(lge_get_boot_mode() == LGE_BOOT_MODE_PIF_910K)) {
		return 1;
	}
#endif

	return ret;
}

int siw_touch_sys_gpio_set_pull(int pin, int value)
{
	int ret = 0;

#if defined(CONFIG_PLAT_SAMSUNG)
	{
		int pull_val;

		switch (value) {
		case GPIO_PULL_UP:
			pull_val = S3C_GPIO_PULL_UP;
			break;
		case GPIO_PULL_DOWN:
			pull_val = S3C_GPIO_PULL_DOWN;
			break;
		default:
			pull_val = S3C_GPIO_PULL_NONE;
			break;
		}
		ret = s3c_gpio_setpull(pin, pull_val);
	}
#endif

	return ret;
}

int siw_touch_sys_panel_reset(struct device *dev)
{
	return 0;
}

int siw_touch_sys_get_panel_bl(struct device *dev)
{
	return 0;
}

int siw_touch_sys_set_panel_bl(struct device *dev, int level)
{
	if (level == 0){
		t_dev_info(dev, "BLU control OFF\n");
		/* To Do : screen off */
	} else {
		t_dev_info(dev, "BLU control ON (level:%d)\n", level);
		/* To Do : screen on with level */
	}
	return 0;
}

int siw_touch_sys_osc(struct device *dev, int onoff)
{
	return 0;
}

#if defined(__SIW_CONFIG_USER_FB)
#if defined(CONFIG_DRM_SDE_SPECIFIC_PANEL)
#include <linux/drm_notify.h>

#define __SIW_SUPPORT_DRM_TYPE_1
#endif	/* CONFIG_DRM_SDE_SPECIFIC_PANEL */
#endif	/* __SIW_CONFIG_USER_FB */

#if defined(__SIW_SUPPORT_DRM_TYPE_1)
static int drm_notifier_callback(
			struct notifier_block *self,
			unsigned long event, void *data)
{
	struct siw_ts *ts =
		container_of(self, struct siw_ts, user_fb_notif);
	struct device *dev = ts->dev;
	struct drm_ext_event *ev = (struct drm_ext_event *)data;
	int *blank;

	if (!ev || !ev->data) {
		return 0;
	}

	blank = (int *)ev->data;

#if defined(__SIW_CONFIG_SYSTEM_PM)
	if (event == DRM_EXT_EVENT_BEFORE_BLANK) {
		if (*blank == DRM_BLANK_UNBLANK) {
			t_dev_info(dev, "drm_unblank(early)\n");
		} else if (*blank == DRM_BLANK_POWERDOWN) {
			t_dev_info(dev, "drm_blank(early)\n");
			siw_touch_suspend_call(dev);
		}
	}

	if (event == DRM_EXT_EVENT_AFTER_BLANK) {
		if (*blank == DRM_BLANK_UNBLANK) {
			t_dev_info(dev, "drm_unblank\n");
			siw_touch_resume_call(dev);
		} else if (*blank == DRM_BLANK_POWERDOWN) {
			t_dev_info(dev, "drm_blank\n");
		}
	}
#else	/* __SIW_CONFIG_SYSTEM_PM */
	if (event == DRM_EXT_EVENT_AFTER_BLANK) {
		if (*blank == DRM_BLANK_UNBLANK) {
			t_dev_info(dev, "drm_unblank\n");
			siw_touch_resume_call(dev);
		} else if (*blank == DRM_BLANK_POWERDOWN) {
			t_dev_info(dev, "drm_blank\n");
			siw_touch_suspend_call(dev);
		}
	}
#endif	/* __SIW_CONFIG_SYSTEM_PM */

	return 0;
}
#endif	/* __SIW_SUPPORT_DRM_TYPE_1 */

int siw_touch_sys_fb_register_client(struct device *dev)
{
#if defined(__SIW_SUPPORT_DRM_TYPE_1)
	struct siw_ts *ts = to_touch_core(dev);

	t_dev_info(dev, "drm_register_client - user_fb_notif\n");

	ts->user_fb_notif.notifier_call = drm_notifier_callback;

	return drm_register_client(&ts->user_fb_notif);
#else
	return 0;
#endif
}

int siw_touch_sys_fb_unregister_client(struct device *dev)
{
#if defined(__SIW_SUPPORT_DRM_TYPE_1)
	struct siw_ts *ts = to_touch_core(dev);

	t_dev_info(dev, "drm_unregister_client - user_fb_notif\n");

	return drm_unregister_client(&ts->user_fb_notif);
#else
	return 0;
#endif
}

