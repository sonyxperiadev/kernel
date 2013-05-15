/*
 * Partly modified 2012 Sony Mobile Communications AB.
 */

/*
 * Definitions for akm8963 compass chip.
 */
#ifndef AKM8963_H
#define AKM8963_H

#ifdef __KERNEL__
#include <linux/device.h>
#endif

#define AKM8963_I2C_NAME "akm8963"
#define MSC_RX  MSC_SERIAL
#define MSC_RY  MSC_PULSELED
#define MSC_RZ  MSC_GESTURE
#define MSC_ST2 MSC_RAW

struct akm8963_platform_data {
	char layouts[3][3];
	int (*setup)(struct device *dev);
	void (*shutdown)(struct device *dev);
	int (*hw_config)(struct device *dev, int enable);
};

#endif

