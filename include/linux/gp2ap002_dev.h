#ifndef __GP2AP002_DEV_H
#define __GP2AP002_DEV_H

/*Platform data */
struct gp2ap002_platform_data {
	void (*power_on) (bool);
	void (*led_on) (bool);
	unsigned int irq_gpio;
	int irq;
};
	
#endif
