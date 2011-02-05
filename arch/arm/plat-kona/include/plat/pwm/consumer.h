/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*       @file   arch/arm/plat-bcmap/include/plat/pwm/consumer.h
*
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*******************************************************************************/
/**
 *
 *   @file   consumer.h
 *   @brief  Header file to define the Structure used to bind consumers to
 *   specific pwm devices.
 ****************************************************************************/
#ifndef _PWM_CONSUMER_H_
#define _PWM_CONSUMER_H_

/**
 * Structure used to bind consumers to specific pwm devices
 */
struct pwm_channel;
struct pwm_device {
	struct pwm_channel *handle; /**< device driver handle */
};

#endif /* _PWM_CONSUMER_H_ */
