/*******************************************************************************************
 * Copyright 2010 Broadcom Corporation.  All rights reserved.
 *
 * Unless you and Broadcom execute a separate written software license agreement
 * governing use of this software, this software is licensed to you under the
 * terms of the GNU General Public License version 2, available at
 * http://www.gnu.org/copyleft/gpl.html (the "GPL").
 *
 * Notwithstanding the above, under no circumstances may you combine this software
 * in any way with any other Broadcom software provided under a license other than
 * the GPL, without Broadcom's express prior written consent.
 * *******************************************************************************************/

/***************************************************************************
**
*
*  @file   xscript_regulator.h
*
*  @brief  Interface to the xscript REGULATOR kernel module.
*
*  @note   This driver is typically used for handling xScript REGULATOR kernel module APIs.
*
****************************************************************************/
#define TEST_DEVICE_NAME             "regulator-test"
#define MAG_NUM                 'r'
#define REGULATORMAJOR     0 

/* put ioctl flags here, use the _IO macro which is 
 found in linux/ioctl.h, takes a letter, and an 
 integer */
#define BCM_REGULATOR_GET					_IO(MAG_NUM,1)
#define BCM_REGULATOR_IS_ENABLED			_IO(MAG_NUM,2)
#define BCM_REGULATOR_ENABLE				_IO(MAG_NUM,3)
#define BCM_REGULATOR_SET_VOLTAGE			_IO(MAG_NUM,4)
#define BCM_REGULATOR_GET_VOLTAGE			_IO(MAG_NUM,5)
#define BCM_REGULATOR_DISABLE				_IO(MAG_NUM,6)
#define BCM_REGULATOR_PUT					_IO(MAG_NUM,7)

/* interface for passing structures between user 
 space and kernel space easily */

struct tmod_interface {
	int     in_len;         // input data length
    char   *in_data;        // input data
    int     out_rc;         // return code from the test
    int     in_len_0;        // input data0 length
    int     *in_data_0;       // input data0
};
typedef struct tmod_interface tmod_interface_t;
