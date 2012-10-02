/**
 * @file	ami_sensor.h
 * @brief	AMI306 Device Driver header
 */
#ifndef AMI_SENSOR_H
#define AMI_SENSOR_H

#include <linux/ioctl.h>
#include "ami306_def.h"

#define AMI_DRV_NAME		"ami_sensor"

/*---------------------------------------------------------------------------*/
/* ioctl request code */
#define AMI_IO			0x84

#define AMI_IOCTL_START_SENSOR	_IO(AMI_IO,  0x01  )
#define AMI_IOCTL_STOP_SENSOR	_IO(AMI_IO,  0x02  )
#define AMI_IOCTL_GET_VALUE	_IOR(AMI_IO, 0x03, struct ami_sensor_value)
#define AMI_IOCTL_SEARCH_OFFSET	_IO(AMI_IO,  0x04  )
#define AMI_IOCTL_WRITE_OFFSET	_IOW(AMI_IO,  0x05, unsigned char)
#define AMI_IOCTL_READ_OFFSET	_IOR(AMI_IO,  0x06, unsigned char)
#define AMI_IOCTL_SET_INTER_OFFSET	_IOW(AMI_IO,  0x07, short)
#define AMI_IOCTL_SET_DELAY     _IOW(AMI_IO,  0x08, int64_t)

#define AMI_IOCTL_READ_PARAMS	_IOR(AMI_IO, 0x11, struct ami_sensor_parameter)
#define AMI_IOCTL_DRIVERINFO	_IOR(AMI_IO, 0x12, struct ami_driverinfo)
#define AMI_IOCTL_DBG_READ	_IOR(AMI_IO, 0x13, struct ami_register)
#define AMI_IOCTL_DBG_READ_W	_IOR(AMI_IO, 0x14, struct ami_register)
#define AMI_IOCTL_DBG_WRITE	_IOW(AMI_IO, 0x15, struct ami_register)
#define AMI_IOCTL_DBG_WRITE_W	_IOW(AMI_IO, 0x16, struct ami_register)

#endif
