/*****************************************************************************
* Copyright 2011 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL"). 
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/

#ifndef _ADC121C021_I2C_H_
#define _ADC121C021_I2C_H_

#define I2C_ADC121C021_DRIVER_SLAVE_NUMBER   0x54
#define I2C_ADC121C021_DRIVER_NAME           "adc121c021_i2c_drvr"

/* Used to set platform information. */
struct I2C_ADC121C021_t {
	/* specify which I2C host */
	int id;

	int i2c_slave_address;
	int gpio_irq_pin;
	int timer_wait;
	int num_bytes_to_read;
	int is_resetable;
	int gpio_reset_pin;
	int battery_max_voltage;
	int battery_min_voltage;
	int resistor_1;
	int resistor_2;

#if defined(CONFIG_BCM_CMP_BATTERY_MULTI) || defined(CONFIG_BCM_CMP_BATTERY_MULTI_MODULE)
	int gpio_ac_power;
	int ac_power_on_level;
	int gpio_charger;
#endif
};

#define ADC121C021_ADC_REG          0
#define ADC121C021_STATUS_REG       1
#define ADC121C021_CONFIG_REG       2
#define ADC121C021_UNDER_ALERT_REG  3
#define ADC121C021_OVER_ALERT_REG   4
#define ADC121C021_HSYT_ALERT_REG   5
#define ADC121C021_LOWEST_REG       6
#define ADC121C021_HIGHEST_REG      7
#define ADC121C021_NUM_REGISTERS    8

#define ADC121C021_WRITE_REG_LENGTH 1

struct ADC121C021_REGISTER {
	int reg_idx;
	int num_bytes;
	int is_read_write;
	int default_val;
};

#endif /* _ADC121C021_I2C_H_ */
