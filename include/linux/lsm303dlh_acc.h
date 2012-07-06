/*
 * lsm303dlh_acc.h -  lsm303dlh accelerometer driver API
 *
 * Copyright (C) 2010 Sony Ericsson Mobile Communications AB.
 * License terms: GNU General Public License (GPL) version 2
 * Author: Aleksej Makarov <aleksej.makarov@sonyericsson.com>
 *
 * Based upon STMicroelectronics code:
 * File Name          : lsm303dlh.h
 * Author             : MH - C&I BU - Application Team
 * Version            : V 1.1
 * Date               : 24/11/2009
 * Description        : LSM303DLH 6D module sensor API
 */
#ifndef _LSM303DLH_ACC_H_
#define _LSM303DLH_ACC_H_

#define LSM303DLH_ACC_DEV_NAME "lsm303dlh_acc"

enum lsm303dlh_acc_range {
	LSM303_RANGE_2G,
	LSM303_RANGE_4G,
	LSM303_RANGE_8G,
};

struct lsm303dlh_acc_platform_data {
	enum lsm303dlh_acc_range range;
	int poll_interval_ms;
	int irq_pad;
	int (*power_on)(void);
	int (*power_off)(void);
};
#endif

