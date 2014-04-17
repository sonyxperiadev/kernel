/*
 * Generic PWM backlight driver data - see drivers/video/backlight/pwm_bl.c
 */
#ifndef __LINUX_PWM_BACKLIGHT_H
#define __LINUX_PWM_BACKLIGHT_H

#include <linux/backlight.h>

#ifdef CONFIG_THERMAL
#include <linux/thermal.h>
#define BACKLIGHT_CDEV_NAME "backlight_coolant"
#endif

struct platform_pwm_backlight_data {
	int pwm_id;
	unsigned int max_brightness;
	unsigned int dft_brightness;
	unsigned int lth_brightness;
	unsigned int pwm_period_ns;
	unsigned int *levels;
	unsigned int polarity;
	const char *pwm_request_label;
	u32 bl_delay_on;
	bool pb_enable_adapt_bright;
	int (*init)(struct device *dev);
	int (*notify)(struct device *dev, int brightness);
	void (*notify_after)(struct device *dev, int brightness);
	void (*exit)(struct device *dev);
	int (*check_fb)(struct device *dev, struct fb_info *info);
};

#ifdef CONFIG_THERMAL
u32 backlight_cooling_get_level(struct thermal_cooling_device *cdev,
			u32 brightness);
#endif /* CONFIG_THERMAL */

#endif /* __LINUX_PWM_BACKLIGHT_H */
