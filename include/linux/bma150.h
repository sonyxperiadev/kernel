/*****************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
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

#ifndef _BMA150_H_
#define _BMA150_H_

/* 
 * The output of the sensor may have to be modified depending on how it is
 * mounted on the PCB.
 * In the following order:
 * y_dir_rev, x_dir_rev, z_dir_rev
 * would mean x -> -y, y-> -x, z-> -z
 */
typedef enum
{
   x_dir,
   y_dir,
   z_dir,
   x_dir_rev,
   y_dir_rev,
   z_dir_rev
} bma150_axis_change_enum;

struct t_bma150_axis_change
{
   int x_change;
   int y_change;
   int z_change;
}; 
  
   
/* The two I2C slave device addresses the driver supports. */
#define BMA150_DRIVER_SLAVE_NUMBER_0x38    0x38
#define BMA150_DRIVER_NAME                 "bma150"

#endif    /* _BMA150_H_ */

