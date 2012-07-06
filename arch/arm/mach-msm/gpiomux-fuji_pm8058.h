#ifndef _GPIOMUX_FUJI_PM8058_H_
#define _GPIOMUX_FUJI_PM8058_H_

struct pmic8058_unused_gpio {
	int *unused_gpio;
	int unused_gpio_num;
};

extern struct pmic8058_unused_gpio pmic8058_unused_gpios;

#endif /* _GPIOMUX_FUJI_PM8058_H_ */
