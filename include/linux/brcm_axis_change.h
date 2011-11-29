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
 * Definitions for any sensor chip that need orientation correction.
 */
#ifndef BRCM_AXIS_CHANGE_H
#define BRCM_AXIS_CHANGE_H

/* Axis switching stuff should be common header as same enum exists for accelerometer and gyro */
typedef enum
{
   axis_x_dir,
   axis_y_dir,
   axis_z_dir,
   axis_x_dir_rev,
   axis_y_dir_rev,
   axis_z_dir_rev
} brcm_axis_change_enum;

struct t_brcm_axis_change
{
   int x_change;
   int y_change;
   int z_change;
}; 

struct t_brcm_sensors_axis_change
{
   struct t_brcm_axis_change *p_accel_axis_change;
   struct t_brcm_axis_change *p_gyro_axis_change;
   struct t_brcm_axis_change *p_compass_axis_change;
   void                      *p_data;
};   

#endif

