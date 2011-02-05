/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*       @file   arch/arm/plat-bcmap/include/plat/pwm/pwm_core.h
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
 *   @file   pwm_core.h
 *   @brief  Header file to define structures for pwm channel and for
 *   callbacks to interface with pwm channels.
 *
 ****************************************************************************/
#ifndef _PWM_CORE_H_
#define _PWM_CORE_H_

#include <linux/device.h>
#include <plat/pwm/consumer.h>

/*
 * NOTE: MAX_PWM_DEVICE_ID must be defined by the platform. The default
 * value is set in pwm core.
 */

struct pwm_channel;
/** Operations supported by the PWM device drivers */
struct pwm_channel_ops {
	/* callbacks for the pwm device driver */
	int (*request)(struct pwm_channel *chan); /**< PWM channel request callback for the pwm driver */
	void (*free)(struct pwm_channel *chan); /**< Callback to free the pwm channel */
	int (*config)(struct pwm_channel *chan, int duty_ns, int period_ns); /**< Callback to configure the pwm channel */
	int (*enable)(struct pwm_channel *chan); /**< Callback enable the pwm channel */
	void (*disable)(struct pwm_channel *chan); /**< Callback to disable the pwm channel */
	int (*setmode)(struct pwm_channel *chan, int mode); /**< Callback to set the mode of pwm channel */
};

/**
 * Descriptor for a PWM device. There will be one descriptor for each
 * of the PWM channels registered by the PWM device driver
 */
struct pwm_desc {
	int pwm_id; /**< Id and name for a pwm channel. the id must have a value from 0 to MAX_PWM_DEVICE_ID. */
	char *label; /**< Label for pwm device */

	struct pwm_channel_ops *ops; /**< Device call backs */
};

/**
 * This structure is used by the PWM core layer to manage the PWM drivers
 * and consumers.
 */
struct pwm_channel {
	struct pwm_desc *desc; /**< PWM channel descriptor */

	struct list_head node; /**< To hook this structure to the pwm list */

	int in_use; /**< Indicates whether the channel is in use or not */

	void *pwm_data; /**< Device driver specific private data structure */

	struct device *dev; /**< Linux device structure */
};

/*
 * Device Driver APIs
 */
struct pwm_channel *pwm_register(struct pwm_desc *desc,
	struct device *dev,
	void *driver_data);
int pwm_unregister(struct pwm_channel *chan);

#endif /* _PWM_CORE_H_ */
