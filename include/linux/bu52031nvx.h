#ifndef ___bu52031nvx
#define ___bu52031nvx

#define BU52031NVX_DEV_NAME "bu52031nvx"

struct device;

struct bu52031nvx_platform_data {
	int gpio_num;
	int (*gpio_setup)(struct device *dev, int enable);
};

#endif
