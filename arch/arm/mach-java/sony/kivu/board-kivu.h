/*****************************************************************************
*  Copyright 2011 - 2012 Broadcom Corporation.  All rights reserved.
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2, available at
*  http://www.gnu.org/licenses/old-license/gpl-2.0.html (the "GPL").
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
*****************************************************************************/
#if defined(CONFIG_INPUT_LSM303DLHC_ACCELEROMETER)
void __init kivu_add_accel(void);
#endif
#if defined(CONFIG_INPUT_LSM303DLH_MAGNETOMETER)
void __init kivu_add_magnetometer(void);
#endif
#if defined(CONFIG_INPUT_L3G4200D)
void __init kivu_add_gyro(void);
#endif
#if defined(CONFIG_LEDS_LM3530)
void __init kivu_add_backlight(void);
#endif
#if defined(CONFIG_BACKLIGHT_LM3630A)
void __init brooks_add_backlight(void);
#endif
#if defined(CONFIG_SENSORS_BMP18X_I2C) || defined(CONFIG_BMP280_I2C)
void __init kivu_add_barometer(void);
#endif
#if defined(CONFIG_INPUT_APDS9702)
void __init kivu_add_proximity(void);
#endif
#if defined(CONFIG_INPUT_AKM09911)
void __init brooks_add_compass(void);
#endif
#if defined(CONFIG_INPUT_BMG160)
void __init kivu_add_gyro(void);
#endif
#if defined(CONFIG_BACKLIGHT_LM3630)
void __init kivu_add_backlight(void);
#endif
#if defined(CONFIG_TOUCHSCREEN_SYNAPTICS_I2C_RMI4_2)
void __init kivu_add_touch(void);
#endif
#if defined(CONFIG_SENSORS_BH1721FVC)
void __init brooks_add_als(void);
#endif
