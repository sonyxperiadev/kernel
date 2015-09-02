/* kernel/drivers/video/msm/mdss/mhl_sii8620_8061_drv/mhl_lib_timer.c
 *
 * Copyright (C) 2013 Sony Mobile Communications AB.
 * Copyright (C) 2013 Silicon Image Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/hrtimer.h>
#include <linux/fs.h>
#include <linux/semaphore.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/cdev.h>
#include <linux/stringify.h>
#include <asm/uaccess.h>

#include "mhl_lib_timer.h"
#include "mhl_platform.h"

#define MHL_DRIVER_TIMER_NAME "sii8620_timer"
#define MSEC_TO_NSEC(x)		(x * 1000000UL)

static struct mhl_dev_context *get_mhl_device_context(void);
static void mhl_tx_timer_work_handler(struct work_struct *work);
static enum hrtimer_restart mhl_tx_timer_handler(struct hrtimer *timer);
static int is_timer_handle_valid(struct mhl_dev_context *dev_context,
				 void *timer_handle);

struct mhl_dev_context {
	struct workqueue_struct	*timer_work_queue;
	struct list_head	timer_list;
	struct semaphore	isr_lock;
} timer_context;

struct timer_obj {
	struct list_head	list_link;
	struct work_struct	work_item;
	struct hrtimer		hr_timer;
	struct mhl_dev_context	*dev_context;
	uint8_t			flags;
#define TIMER_OBJ_FLAG_WORK_IP	0x01
#define TIMER_OBJ_FLAG_DEL_REQ	0x02
	void			*callback_param;
	void (*timer_callback_handler)(void *callback_param);
};

static struct mhl_dev_context *get_mhl_device_context(void)
{
	return &timer_context;
}

static void mhl_tx_timer_work_handler(struct work_struct *work)
{
	struct timer_obj	*mhl_timer;

	mhl_timer = container_of(work, struct timer_obj, work_item);

	mhl_timer->flags |= TIMER_OBJ_FLAG_WORK_IP;
	if (!down_interruptible(&mhl_timer->dev_context->isr_lock)) {

		mhl_timer->timer_callback_handler(mhl_timer->callback_param);

		up(&mhl_timer->dev_context->isr_lock);
	}
	mhl_timer->flags &= ~TIMER_OBJ_FLAG_WORK_IP;

	if(mhl_timer->flags & TIMER_OBJ_FLAG_DEL_REQ) {
		/*
		 * Deletion of this timer was requested during the execution of
		 * the callback handler so go ahead and delete it now.
		 */
		kfree(mhl_timer);
	}
}

static enum hrtimer_restart mhl_tx_timer_handler(struct hrtimer *timer)
{
	struct timer_obj	*mhl_timer;

	mhl_timer = container_of(timer, struct timer_obj, hr_timer);

	queue_work(mhl_timer->dev_context->timer_work_queue,
			   &mhl_timer->work_item);

	return HRTIMER_NORESTART;
}

static int is_timer_handle_valid(struct mhl_dev_context *dev_context,
				 void *timer_handle)
{
	struct timer_obj	*timer;

	list_for_each_entry(timer, &dev_context->timer_list, list_link) {
		if (timer == timer_handle)
			break;
	}

	if(timer != timer_handle) {
		pr_err("%s: Invalid timer handle %p received\n",
			 __func__, timer_handle);
		return -EINVAL;
	}
	return 0;
}

int mhl_lib_timer_create(void (*callback_handler)(void *callback_param),
			 void *callback_param,
			 void **timer_handle)
{
	struct mhl_dev_context	*dev_context;
	struct timer_obj		*new_timer;

	dev_context = get_mhl_device_context();

	if (callback_handler == NULL)
		return -EINVAL;

	if (dev_context->timer_work_queue == NULL)
		return -ENOMEM;

	new_timer = kmalloc(sizeof(*new_timer), GFP_KERNEL);
	if (new_timer == NULL)
		return -ENOMEM;

	new_timer->timer_callback_handler = callback_handler;
	new_timer->callback_param = callback_param;
	new_timer->flags = 0;

	new_timer->dev_context = dev_context;
	INIT_WORK(&new_timer->work_item, mhl_tx_timer_work_handler);

	list_add(&new_timer->list_link, &dev_context->timer_list);

	hrtimer_init(&new_timer->hr_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	new_timer->hr_timer.function = mhl_tx_timer_handler;
	*timer_handle = new_timer;
	return 0;
}

int mhl_lib_timer_delete(void **timer_handle)
{
	struct mhl_dev_context	*dev_context;
	struct timer_obj		*timer;
	int						status;

	dev_context = get_mhl_device_context();

	status = is_timer_handle_valid(dev_context, *timer_handle);
	if (status == 0) {
		timer = *timer_handle;

		list_del(&timer->list_link);

		hrtimer_cancel(&timer->hr_timer);

		if(timer->flags & TIMER_OBJ_FLAG_WORK_IP) {
			/*
			 * Request to delete timer object came from within the timer's
			 * callback handler.  If we were to proceed with the timer deletion
			 * we would deadlock at cancel_work_sync().  So instead just flag
			 * that the user wants the timer deleted.  Later when the timer
			 * callback completes the timer's work handler will complete the
			 * process of deleting this timer.
			 */
			timer->flags |= TIMER_OBJ_FLAG_DEL_REQ;
		} else {
			cancel_work_sync(&timer->work_item);
			*timer_handle = NULL;
			kfree(timer);
		}
	}

	return status;
}

int mhl_lib_timer_start(void *timer_handle, uint32_t time_msec)
{
	struct mhl_dev_context	*dev_context;
	struct timer_obj	*timer;
	ktime_t			timer_period;
	int			status;

	dev_context = get_mhl_device_context();

	status = is_timer_handle_valid(dev_context, timer_handle);
	if (status == 0) {
		long secs=0;
		timer = timer_handle;

		secs=time_msec/1000;
		time_msec %= 1000;
		timer_period = ktime_set(secs, MSEC_TO_NSEC(time_msec));
		hrtimer_start(&timer->hr_timer, timer_period, HRTIMER_MODE_REL);
	}

	return status;
}

int mhl_lib_timer_stop(void *timer_handle)
{
	struct mhl_dev_context	*dev_context;
	struct timer_obj	*timer;
	int			status;

	dev_context = get_mhl_device_context();

	status = is_timer_handle_valid(dev_context, timer_handle);
	if (status == 0) {
		timer = timer_handle;

		hrtimer_cancel(&timer->hr_timer);
	}
	return status;
}

int mhl_lib_timer_init(void)
{
	int ret = 0;

	struct mhl_dev_context *dev_context;

	dev_context = &timer_context;

	sema_init(&dev_context->isr_lock, 1);
	INIT_LIST_HEAD(&timer_context.timer_list);
	timer_context.timer_work_queue = create_workqueue(MHL_DRIVER_TIMER_NAME);

	if (timer_context.timer_work_queue == NULL) {
		ret = -ENOMEM;
	}

	return ret;
}

void mhl_lib_timer_release(void)
{
	struct timer_obj	*mhl_timer;
	struct mhl_dev_context	*dev_context;

	dev_context = get_mhl_device_context();

	/*
	 * Make sure all outstanding timer objects are canceled and the
	 * memory allocated for them is freed.
	 */

	while(!list_empty(&dev_context->timer_list)) {
		mhl_timer = list_first_entry(&dev_context->timer_list,
									 struct timer_obj, list_link);
		hrtimer_cancel(&mhl_timer->hr_timer);
		list_del(&mhl_timer->list_link);
		kfree(mhl_timer);
	}

	destroy_workqueue(dev_context->timer_work_queue);
	dev_context->timer_work_queue = NULL;
}
