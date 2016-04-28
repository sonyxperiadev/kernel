/**
 * @file	ami_sensor_pif.h
 * @brief	AMI306 Device Driver platform interface
 */
#ifndef AMI_SENSOR_PIF_H
#define AMI_SENSOR_PIF_H

#include <linux/ami306_def.h>

void AMI_udelay(u32 usec);
void AMI_mdelay(u32 msec);
#ifdef USE_DRDY_PIN
int AMI_DRDY_Value(void);
#endif
int AMI_i2c_send(void *i2c, u8 adr, u8 len, u8 *buf);
int AMI_i2c_recv(void *i2c, u8 adr, u8 len, u8 *buf);
void AMI_LOG(const char *fmt, ...);
void AMI_DLOG(const char *fmt, ...);

#endif
