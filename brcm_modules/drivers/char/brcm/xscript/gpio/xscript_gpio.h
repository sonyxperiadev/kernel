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
*  @file   xscript_gpio.h
*
*  @brief  Interface to the xscript gpio kernel module.
*
*   @note	This driver is typically used for handling xScript gpio kernel module  APIs.
*
****************************************************************************/



#define DEVICE_NAME		"gpio-test"
#define MAG_NUM			'g'
#define PMU_IRQ_NUM		83
#define MMCSD_GPIO_NUM		14
/* put ioctl flags here, use the _IO macro which is 
 found in linux/ioctl.h, takes a letter, and an 
 integer */

#define GPIOMAJOR      0

#define GPIO_GET_VALUE		_IO(MAG_NUM, 1)
#define GPIO_SET_VALUE		_IO(MAG_NUM, 2)
#define GPIO_REQUEST		_IO(MAG_NUM, 3)
#define GPIO_FREE		_IO(MAG_NUM, 4)
#define GPIO_DIR_OUTPUT		_IO(MAG_NUM, 5)
#define GPIO_DIR_INPUT		_IO(MAG_NUM, 6)
#define GPIO_IS_OUTPUT		_IO(MAG_NUM, 7)
#define GPIO_LCD_BACKLIGHT	_IO(MAG_NUM, 9)
#define GPIO_IRQ_TEST		_IO(MAG_NUM, 10)
#define GPIO_PULL_DOWN_UP_EN    _IO(MAG_NUM, 11)
#define GPIO_PULL_DOWN_UP	_IO(MAG_NUM, 12)

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
 

 


