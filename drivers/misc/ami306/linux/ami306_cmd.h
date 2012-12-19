/**
 * @file	ami306_cmd.h
 * @brief	AMI306 Device Driver
 */
#ifndef AMI306_CMD_H
#define AMI306_CMD_H

#include <linux/ami306_def.h>

/* device driver initialize */
#ifdef USER_MEMORY
int AMI_GetMemSize(void);
void *AMI_InitDriver(void *mem, void *i2c_handle, int ami_dir, int ami_polarity);
#else
void *AMI_InitDriver(void *i2c_handle, int ami_dir, int ami_polarity);
#endif
/* device driver command */
int AMI_StartSensor(void *handle);
int AMI_StopSensor(void *handle);
int AMI_GetValue(void *handle, struct ami_sensor_value *val);
int AMI_SearchOffset(void *handle);
int AMI_WriteOffset(void *handle, u8 offset[3]);
int AMI_ReadOffset(void *handle, u8 offset[3]);
int AMI_SetSoftIron(void *handle, s16 si[9]);
int AMI_GetSoftIron(void *handle, s16 si[9]);
int AMI_SetDirection(void *handle, s16 dir, s16 polarity);
int AMI_GetDirection(void *handle, s16 *dir, s16 *polarity);

int AMI_SelfTest(void *i2c_handle);
/* for debug */
int AMI_ReadParameter(void *handle, struct ami_sensor_parameter *prm);
int AMI_DriverInformation(void *handle, struct ami_driverinfo *drv);
int AMI_GetRawValue(void *handle, struct ami_sensor_value *raw_val);

int AMI_i2c_send_b(void *handle, u8 cmd, u8 buf);
int AMI_i2c_recv_b(void *handle, u8 cmd, u8 *buf);
int AMI_i2c_send_w(void *handle, u8 cmd, u16 buf);
int AMI_i2c_recv_w(void *handle, u8 cmd, u16 *buf);

#endif
