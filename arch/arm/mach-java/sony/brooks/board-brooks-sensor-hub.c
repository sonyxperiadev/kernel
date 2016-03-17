#include <linux/i2c.h>
#include <asm/gpio.h>
#include <linux/gpio.h>
#include <linux/gpio_keys.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/irq.h>
#include "board-brooks.h"

#define SENSOR_I2C_BUS_ID 2

#define GPIO_EM7180_IRQ 111


static struct i2c_board_info __initdata em7180_i2c_boardinfo[] = {
	{
		I2C_BOARD_INFO("em7180", 0x28),
		.irq = gpio_to_irq(GPIO_EM7180_IRQ),
	},
};

void __init brooks_add_sensor_hub(void)
{
	pr_info("Registering %s on I2C bus #%i",
		"em7180", SENSOR_I2C_BUS_ID);
	i2c_register_board_info(SENSOR_I2C_BUS_ID,
				em7180_i2c_boardinfo,
				ARRAY_SIZE(em7180_i2c_boardinfo));

	if (gpio_request_one(GPIO_EM7180_IRQ, GPIOF_DIR_IN, "EM7180 IRQ")) {
		pr_err("EM7180 IRQ GPIO request failed");
	}
}
