/*****************************************************************************
* Copyright 2010 - 2011 Broadcom Corporation.  All rights reserved.
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

#ifndef ADC121C021_I2C_SETTINGS_H
#define ADC121C021_I2C_SETTINGS_H

/*
 * Refer to include/linux/broadcom/adc121c021_driver.h for details
 */

/* connected to BSC0 */
#define HW_ADC121C021_I2C_BUS_ID    1

/* use GPIO0 to signal the host processor for battery alerts */
#define HW_ADC121C021_GPIO_EVENT    18 /* GPIO #18 */

#define HW_ADC121C021_BYTES_TO_READ 2

#define HW_ADC121C021_RESISTOR_1    158
#define HW_ADC121C021_RESISTOR_2    100

#define HW_ADC121C021_GPIO_AC_POWER      181 /* AC_OK_B */
#define HW_ADC121C021_AC_POWER_ON_LEVEL    1

#define HW_ADC121C021_GPIO_CHARGER       180 /* BAT_CHG_B */

#endif /* ADC121C021_I2C_SETTINGS_H */
