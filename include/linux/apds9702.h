/*
 * Definitions for apds9702 proximity sensor chip.
 */

#ifndef __APDS9702_H__
#define __APDS9702_H__

#define APDS9702_NAME "apds9702"

struct device;

struct apds9702_ctl_reg {
	unsigned int trg:1;
	unsigned int pwr:1;
	unsigned int burst:4;
	unsigned int frq:2;
	unsigned int dur:2;
	unsigned int th:4;
	unsigned int rfilt:2;
};

struct apds9702_platform_data {
	u16 gpio_dout;
	struct apds9702_ctl_reg ctl_reg;
	char *phys_dev_path;
	unsigned int is_irq_wakeup:1;
	void (*hw_config)(struct device *dev, int enable);
	int (*gpio_setup)(struct device *dev, int request);
};
#endif
