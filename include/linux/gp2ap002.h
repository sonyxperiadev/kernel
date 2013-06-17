#ifndef __GP2A_PROX_H
#define __GP2A_PROX_H

#include <linux/types.h>
#include <linux/ioctl.h>

#define PROX_SENSOR_I2C_ADDR	0x44

/* power control */
#define ON      1
#define OFF		0

/*GP2AP002S00F Register Addresses1*/
#define GP2A_REG_PROX 	0x00
#define GP2A_REG_GAIN 	0x01
#define GP2A_REG_HYS 	0x02
#define GP2A_REG_CYCLE 	0x03
#define GP2A_REG_OPMOD 	0x04
#define GP2A_REG_CON 	0x06

/*Proximity value register*/	
#define REG0_PROX_VALUE_MASK   	0x01

/*IOCTLS*/
#define PROX_IOC_MAGIC  		0xBE

#define PROX_IOC_NORMAL_MODE         	_IO(PROX_IOC_MAGIC, 0)
#define PROX_IOC_SHUTDOWN_MODE      _IO(PROX_IOC_MAGIC, 1)

#define PROX_IOC_SET_CALIBRATION       _IOW(PROX_IOC_MAGIC, 0x90, short)
#define PROX_IOC_GET_CALIBRATION       _IOR(PROX_IOC_MAGIC, 0x91, short)
	
/* input device for proximity sensor */
#define USE_INPUT_DEVICE 	0  /* 0 : No Use  ,  1: Use  */

#define USE_INTERRUPT		1
#define INT_CLEAR    1 /* 0 = by polling operation, 1 = by interrupt operation */
	
#endif
