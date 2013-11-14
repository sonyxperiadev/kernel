/*
 * Generic PWM backlight driver data - see drivers/video/backlight/pwm_bl.c
 */
#ifndef __LINUX_PWM_BACKLIGHT_H
#define __LINUX_PWM_BACKLIGHT_H

#include <linux/backlight.h>

#ifdef CONFIG_KONA_TMON
/* Temperature compensation (used to limit max brightness) */
struct pb_temp_comp {
	unsigned long trigger_temp;
	int max_brightness;
};
#endif

struct platform_pwm_backlight_data {
	int pwm_id;
	unsigned int max_brightness;
	unsigned int dft_brightness;
	unsigned int lth_brightness;
	unsigned int pwm_period_ns;
	unsigned int *levels;
	unsigned int polarity;
#ifdef CONFIG_KONA_TMON
	struct pb_temp_comp *temp_comp_tbl;
	unsigned int temp_comp_size;
#endif
	int bl_delay_on;
	const char *pwm_request_label;
	int (*init)(struct device *dev);
	int (*notify)(struct device *dev, int brightness);
	void (*notify_after)(struct device *dev, int brightness);
	void (*exit)(struct device *dev);
	int (*check_fb)(struct device *dev, struct fb_info *info);
};

#endif
