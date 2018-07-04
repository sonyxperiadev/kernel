/*
 * SiW touch system interface
 *
 * Copyright (C) 2016 Silicon Works - http://www.siliconworks.co.kr
 * Copyright (C) 2018 Sony Mobile Communications Inc.
 * Author: Hyunho Kim <kimhh@siliconworks.co.kr>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 */

#ifndef __SIW_TOUCH_SYS_H
#define __SIW_TOUCH_SYS_H

extern int siw_touch_sys_bus_use_dma(struct device *dev);

extern int siw_touch_get_boot_mode(void);
extern int siw_touch_boot_mode_check(struct device *dev);

extern int siw_touch_boot_mode_tc_check(struct device *dev);

extern int siw_touch_sys_gpio_set_pull(int pin, int value);

extern int siw_touch_sys_panel_reset(struct device *dev);
extern int siw_touch_sys_get_panel_bl(struct device *dev);
extern int siw_touch_sys_set_panel_bl(struct device *dev, int level);

extern int siw_touch_sys_osc(struct device *dev, int onoff);

extern int siw_touch_sys_fb_register_client(struct device *dev);
extern int siw_touch_sys_fb_unregister_client(struct device *dev);

#endif	/* __SIW_TOUCH_SYS_H */

