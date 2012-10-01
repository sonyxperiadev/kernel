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

#if !defined __brvsens_driver_h__
#define      __brvsens_driver_h__

#include <linux/ioctl.h>

/*Note: This include should be shared between User and Kernel land
but we can't do that yet in this environment, so for now libsensors
will have to replicate same info.

However, to correctly propagate the concept of'private' and 'public' data
I am including here definitions that would be considered 'public', i.e.
shared between different software layers
*/

/* Generic Event Structure, as maintained in kfifo queue */
struct brvsens_event {
	unsigned char type;/*event type -- sensor handle*/
	unsigned char size;/*event size -- # of bytes (sanity checking)*/
	short data[5];/*event data buffer (last 4 bytes -- padding / future)*/
};

/* Encapsulation of 'ioctl' commands */
struct brvsens_cmd {
	unsigned int handle;	/*driver handle*/
	unsigned int delay;		/*poll rate*/
	unsigned int enable;	/*act/deact flag*/
};

#define BRVSENS_DEVICE      "/dev/brvsens"
/*max number of events that can be returned from single read*/
#define BRVSENS_EVENT_NUM   64

/*ioctl interface definition to User Land*/
#define BRVSENS_DRIVER_IOC_MAGIC     'B'

#define BRVSENS_DRIVER_IOC_SET_DELAY _IOW(BRVSENS_DRIVER_IOC_MAGIC,     100, struct brvsens_cmd)
#define BRVSENS_DRIVER_IOC_ACTIVATE	_IOW(BRVSENS_DRIVER_IOC_MAGIC,     101, struct brvsens_cmd)
#define BRVSENS_DRIVER_IOC_CONFIG _IOR(BRVSENS_DRIVER_IOC_MAGIC,     102, unsigned int)

/*Sensor Handle Enumeration; Each handle has different bit set,
which allows us to send board configuration as bitmask.
Bit mapping matches Android Sensor Type
enumeration, as specified in "sensors.h".*/
#define SENSOR_HANDLE_ACCELEROMETER 0x01
	/*(1 << (SENSOR_TYPE_ACCELEROMETER - 1) )*/
#define SENSOR_HANDLE_COMPASS       0x02
	/*(1 << (SENSOR_TYPE_MAGNETIC_FIELD - 1) )*/
#define SENSOR_HANDLE_GYROSCOPE     0x08
	/*(1 << (SENSOR_TYPE_GYROSCOPE - 1) )*/
#define SENSOR_HANDLE_LIGHT         0x10
	/*(1 << (SENSOR_TYPE_LIGHT - 1) )*/
#define SENSOR_HANDLE_PRESSURE      0x20
	/*(1 << (SENSOR_TYPE_PRESSURE - 1) )*/
#define SENSOR_HANDLE_TEMPERATURE   0x40
	/*(1 << (SENSOR_TYPE_TEMPERATURE - 1) )*/
#define SENSOR_HANDLE_PROXIMITY     0x80
	/*(1 << (SENSOR_TYPE_PROXIMITY - 1) )*/

/*Sensor Registration*/
typedef int (*PFNACTIVATE) (void *context, unsigned int flag);
typedef int (*PFNREAD) (void *context, short *buffer);

int brvsens_register(u32 handle, const char *name, void *context,
		     PFNACTIVATE actCbk, PFNREAD readCbk);
int brvsens_deregister(u32 handle);

#endif /* __brvsens_driver_h__ */
