/*******************************************************************************************
Copyright 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement 
governing use of this software, this software is licensed to you under the 
terms of the GNU General Public License version 2, available at 
http://www.gnu.org/copyleft/gpl.html (the "GPL"). 

Notwithstanding the above, under no circumstances may you combine this software 
in any way with any other Broadcom software provided under a license other than 
the GPL, without Broadcom's express prior written consent.
*******************************************************************************************/

//***************************************************************************
/**
*
*  @file   xscript_i2s.h
*
*  @brief  Interface to the xscript I2S kernel module.
*
*   @note	This driver is typically used for handling xScript I2S kernel module  APIs.
*
****************************************************************************/
#define TEST_DEVICE_NAME             "i2s-test"
#define MAG_NUM                 'i'
#define I2SMAJOR     0 // 241

/* put ioctl flags here, use the _IO macro which is 
 found in linux/ioctl.h, takes a letter, and an 
 integer */
#define I2S_INTERNAL_LOOPBACK_MODE  _IO(MAG_NUM,1)
/* interface for passing structures between user 
 space and kernel space easily */

struct tmod_interface {
	int     in_len;         // input data length
         int    *in_data;        // input data
        int     out_rc;         // return code from the test
        int     in_len_0;        // input data0 length
        int     *in_data_0;       // input data0
};
typedef struct tmod_interface tmod_interface_t;
