/*
 * sgm3727 backlight driver data - see drivers/video/backlight/sgm3727_bl.c
 */
#ifndef __LINUX_SGM3727_BACKLIGHT_H
#define __LINUX_SGM3727_BACKLIGHT_H

#include <linux/backlight.h>

struct platform_sgm3727_backlight_data {
	unsigned int bl_gpio;
	unsigned int max_brightness;
	unsigned int *levels;
	unsigned int dft_brightness;
	unsigned int shutdown_time;
	unsigned int ready_time;
	unsigned int interval_time;

	int (*init)(struct device *dev);
	int (*notify)(struct device *dev, int brightness);
	void (*notify_after)(struct device *dev, int brightness);
	void (*exit)(struct device *dev);
	int (*check_fb)(struct device *dev, struct fb_info *info);
};

#endif
