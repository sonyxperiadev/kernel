#include <linux/i2c.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>
#include <linux/interrupt.h>
#include <linux/input/synaptics_dsx.h>
#include "board-brooks.h"

#define TOUCH_I2C_BUS_ID 3

#define TOUCH_I2C_ADDR 0x20
#define TOUCH_IRQ 73
#define TOUCH_RST 70

/* Synaptics Thin Driver */
static int synaptics_gpio_setup(unsigned gpio, bool configure)
{
	int retval = 0;
	if (configure) {
		retval = gpio_request(gpio, "rmi4_attn");
		if (retval) {
			pr_err("%s: Failed to get attn gpio %d. Code: %d.",
			       __func__, gpio, retval);
			return retval;
		}
		retval = gpio_direction_input(gpio);
		if (retval) {
			pr_err("%s: Failed to setup attn gpio %d. Code: %d.",
			       __func__, gpio, retval);
			gpio_free(gpio);
		}
	} else {
		pr_warn("%s: No way to deconfigure gpio %d.",
		       __func__, gpio);
	}

	return retval;
}

static unsigned char TM_SAMPLE1_f1a_button_codes[] = {};

static struct synaptics_rmi4_capacitance_button_map TM_SAMPLE1_capacitance_button_map = {
	.nbuttons = ARRAY_SIZE(TM_SAMPLE1_f1a_button_codes),
	.map = TM_SAMPLE1_f1a_button_codes,
};

static struct synaptics_rmi4_platform_data rmi4_platformdata = {
	.irq_flags = IRQF_TRIGGER_FALLING,
	.irq_gpio = TOUCH_IRQ,
	.gpio_config = synaptics_gpio_setup,
	.reset_gpio = TOUCH_RST,
	.capacitance_button_map = &TM_SAMPLE1_capacitance_button_map,
};

static struct i2c_board_info rmi4_i2c_device_info[] = {
	{
		I2C_BOARD_INFO("synaptics_rmi4_i2c", TOUCH_I2C_ADDR),
		.platform_data = &rmi4_platformdata,
	},
};

void __init brooks_add_touch(void)
{
	pr_info("Registering Synaptics Touch with I2C bus #%i",
			TOUCH_I2C_BUS_ID);
	i2c_register_board_info(TOUCH_I2C_BUS_ID,
				rmi4_i2c_device_info,
				ARRAY_SIZE(rmi4_i2c_device_info));
}
