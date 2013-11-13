/* drivers/mfd/richtek/rt8973.h
 * Driver to Richtek RT8973 micro USB switch device
 *
 * Copyright (C) 2012
 * Author: Patrick Chang <patrick_chang@richtek.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __RICHTEK_RT8973_H
#define __RICHTEK_RT8973_H
#include "rt_comm_defs.h"

#define RT8973_WAIT_DELAY   1

#define RT8973_REG_CHIP_ID         0x01
#define RT8973_REG_CONTROL_1       0x02
#define RT8973_REG_INT_FLAG        0x03
#define RT8973_REG_INTERRUPT_MASK  0x05
#define RT8973_REG_ADC             0x07
#define RT8973_REG_DEVICE_1        0x0A
#define RT8973_REG_DEVICE_2        0x0B
#define RT8973_REG_MANUAL_SW1      0x13
#define RT8973_REG_MANUAL_SW2      0x14
#define RT8973_REG_RESET           0x1B

#define RT8973_INT_ATTACH_MASK     0x01
#define RT8973_INT_DETACH_MASK     0x02
#define RT8973_INT_CHGDET_MASK     0x04
#define RT8973_INT_DCDTIMEOUT_MASK 0x08
#define RT8973_INT_ADCCHG_MASK     0x40

#define RT8973_DEV2_UNKNOWN_ACC	0x80

#define RT8973_INTM_ADC_CHG			(1<<6)

#if 1 // modify for Samsung ivory (AP does not support TRIGGER_LOW)
#define RT8973_IRQF_MODE (IRQF_TRIGGER_FALLING)
#else
#define RT8973_IRQF_MODE (IRQF_TRIGGER_LOW)
#endif
#define RTMUSC_DRIVER_VER "1.1.2"

struct rt8973_data
{
    struct i2c_client *client;
    int32_t usbid_adc; // 5 bits
    int32_t factory_mode; // 0~3
    int32_t operating_mode;
    int32_t chip_id;
    int32_t accessory_id;
    struct work_struct work;
    int32_t irq;
    uint8_t prev_int_flag;
    struct wake_lock muic_wake_lock;
};


/* Initial Setting */
#ifdef CONFIG_RT8973_CM_AUTO
#define OPERATING_MODE 0
#elif defined(CONFIG_RT8973_CM_MANUAL)
#define OPERATING_MODE 1
#else
#error No operating mode defined
#endif




#endif
