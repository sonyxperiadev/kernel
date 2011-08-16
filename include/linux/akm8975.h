/*****************************************************************************
* Copyright 2011 Broadcom Corporation.  All rights reserved.
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


/*
 * Definitions for akm8975 compass chip.
 */
#ifndef __AKM8975_H__
#define __AKM8975_H__


<<<<<<< HEAD
/* Axis switching stuff should be common header as same enum exists for accelerometer and gyro */
typedef enum
{
   compass_x_dir,
   compass_y_dir,
   compass_z_dir,
   compass_x_dir_rev,
   compass_y_dir_rev,
   compass_z_dir_rev
} akm8975_axis_change_enum;

struct t_akm8975_axis_change
{
   int x_change;
   int y_change;
   int z_change;
}; 
=======
/* IOCTLs for AKM library */
#define ECS_IOCTL_WRITE                 _IOW(AKMIO, 0x02, char[5])
#define ECS_IOCTL_READ                  _IOWR(AKMIO, 0x03, char[5])
#define ECS_IOCTL_SET_MODE              _IOW(AKMIO, 0x04, short)
#define ECS_IOCTL_GETDATA               _IOR(AKMIO, 0x08, char[RBUFF_SIZE])
#define ECS_IOCTL_SET_YPR               _IOW(AKMIO, 0x0C, int[3])
#define ECS_IOCTL_GET_OPEN_STATUS       _IOR(AKMIO, 0x0D, int)
#define ECS_IOCTL_GET_CLOSE_STATUS      _IOR(AKMIO, 0x0E, int)
#define ECS_IOCTL_GET_DELAY             _IOR(AKMIO, 0x30, short)

/* IOCTLs for APPs */
#define ECS_IOCTL_APP_SET_DELAY		_IOW(AKMIO, 0x18, short)
#define ECS_IOCTL_APP_GET_DELAY		ECS_IOCTL_GET_DELAY
/* Set raw magnetic vector flag */
#define ECS_IOCTL_APP_SET_MVFLAG	_IOW(AKMIO, 0x19, short)
/* Get raw magnetic vector flag */
#define ECS_IOCTL_APP_GET_MVFLAG	_IOR(AKMIO, 0x1A, short)
>>>>>>> mps-lmp


#define AKM8975_DEVICE_NAME   "compass"
#define AKM8975_DRV_NAME      "akm8975"


#endif

