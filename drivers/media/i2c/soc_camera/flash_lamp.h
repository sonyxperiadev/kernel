/*
 * Generic flash lamp interface
 *
 * Copyright (C) 2013 Broadcom Corporation
 *

 * This program is free software; you can redistribute it and/or
 *modify it under the terms of the GNU General Public License as
 *published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 *kind, whether express or implied; without even the implied warranty
 *of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef FLASH_LAMP_INTF_H
#define FLASH_LAMP_INTF_H

#include <linux/videodev2.h>

typedef enum v4l2_flash_led_mode (*get_flashamp_mode)(void);
typedef int (*get_flashamp_duration)(void);
typedef int (*get_flashlamp_intensity)(void);
typedef int (*reset_flashamp)(void);
typedef int (*setup_flashlamp)(enum v4l2_flash_led_mode mode,
		int duration_in_us, int intensity);
typedef int (*enable_flashlamp)(void);
typedef int (*disable_flashlamp)(void);

struct flash_lamp_s {
	char *name;
	get_flashamp_mode       get_mode;
	get_flashamp_duration   get_duration;
	get_flashlamp_intensity get_intensity;
	reset_flashamp          reset;
	setup_flashlamp         setup;
	enable_flashlamp        enable;
	disable_flashlamp       disable;
};

/* get flash lamp control interface
 * @lamp: pointer to a lamp instance
 * return: 0 - success, others - fail
 */
#ifdef CONFIG_VIDEO_FLASHLAMP
/* a certain flash lamp driver should have
 * implelemented this interface
 */
extern int get_flash_lamp(struct flash_lamp_s *lamp);
#else
/* no flash lamp support */
int get_flash_lamp(struct flash_lamp_s *lamp)
{
	return -1;
}
#endif

#endif

