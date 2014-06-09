/*
 * EM Micro SFP Sensor Hub
 *
 * Copyright  (C) 2013-2014 EM Microelectronic
 */

#ifndef _EM7180_H_
#define _EM7180_H_

#include <linux/types.h>
#include <linux/bitops.h>
#include <linux/device.h>
#include <linux/sysfs.h>

#include "portable.h"
#include "sentral_registers.h"
#include "driver_core.h"

#if 1
PREPACK
struct  MIDPACK quat_sensor_data  {
	u32					x,y,z,w;
	u16					timestamp;
}; 
POSTPACK

PREPACK
struct  MIDPACK mag_sensor_data  {
	s16					x,y,z;
	u16					timestamp;
}; 
POSTPACK

PREPACK
struct  MIDPACK accel_sensor_data  {
	s16					x,y,z;
	u16					timestamp;
}; 
POSTPACK

PREPACK
struct  MIDPACK gyro_sensor_data  {
	s16					x,y,z;
	u16					timestamp;
}; 
POSTPACK

PREPACK
struct  MIDPACK feature_sensor_data  {
	s16					x;
	u16					timestamp;
}; 
POSTPACK

PREPACK
struct  MIDPACK em7180_status  {
	RegEventStatus		event;
	RegSensorStatus		sensor;
	RegSentralStatus	sentral;
	RegAlgorithmStatus	algo;
}; 
POSTPACK

#define SENSOR_ENABLE_QUAT	BIT(2)
#define SENSOR_ENABLE_MAG	BIT(3)
#define SENSOR_ENABLE_ACCEL BIT(4)
#define SENSOR_ENABLE_GYRO  BIT(5)
#define SENSOR_ENABLE_CUST0 BIT(6)
#define SENSOR_ENABLE_CUST1 BIT(6)
#define SENSOR_ENABLE_CUST2 BIT(6)


#endif

#define SENSOR_ACCEL_DEFAULT_RATE	5
#define SENSOR_MAG_DEFAULT_RATE		30
#define SENSOR_GYRO_DEFAULT_RATE	10
#define SENSOR_QUAT_DEFAULT_RATE	1
#define SENSOR_BARO_DEFAULT_RATE	1
#define SENSOR_TEMP_DEFAULT_RATE	1
#define SENSOR_HUMID_DEFAULT_RATE	1
#define SENSOR_CUST0_DEFAULT_RATE	1
#define SENSOR_CUST1_DEFAULT_RATE	1
#define SENSOR_CUST2_DEFAULT_RATE	1

#define SENSOR_PATH_MAX		128
struct em7180_sensor {
	struct em7180			*dev;
	struct input_dev		*input;

	unsigned				enable_mask;
	char   					path[SENSOR_PATH_MAX];
	const char				*name;
	const char				*desc;
	DI_SENSOR_INFO_T		*info;
	u32						rate_scale_num;
	u32						rate_scale_denom;
	u8						rate_reg;
	u8						actual_rate_reg;

//	struct attribute		sysfs_attr;

#if 0
	unsigned				enabled;
	unsigned 				requested_rate;
	unsigned 				actual_rate;
	unsigned				timestamp_prev;
	u64						timestamp;
#endif

	int		(*init)	(struct em7180_sensor*);
};


struct em7180 {
	struct device 	 		*dev;
	struct mutex 	 		mutex;	
	int 					irq;
	struct DI_INSTANCE		*di;
	struct em7180_sensor  	sensors[DST_NUM_SENSOR_TYPES];

	u8 						product_id;
	u8						revision_id;
	u16						rom_version;
	u16						ram_version;
	struct em7180_status	status;
	u8						enabled_sensors;
	u32						fastest_rate;
//	struct  kobject			*sysfs_sensors;
//	struct	attribute_group	sys_attr_group;
};


#define EM7180_PRODUCT_ID	0x80

#define EM7180_STATUS_EEPROM_DETECT				0x01
#define EM7180_STATUS_EEPROM_UPLOAD_COMPLETE	0x02


#endif
