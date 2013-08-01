/*******************************************************************************
Copyright 2012-2013 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement
governing use of this software, this software is licensed to you under the
terms of the GNU General Public License version 2, available at
http://www.gnu.org/copyleft/gpl.html (the "GPL").

Notwithstanding the above, under no circumstances may you combine this software
in any way with any other Broadcom software provided under a license other than
the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

#include <linux/earlysuspend.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/fb.h>
#include <linux/suspend.h>

/* Debug logs enabled by default */
enum {
	DEBUG_DISABLE = 1U << 0,
	DEBUG_ENABLE  = 1U << 1,
};
static int debug_mask = DEBUG_ENABLE;
module_param_named(debug_mask, debug_mask, int, S_IRUGO | S_IWUSR | S_IWGRP);

#define ES_DBG(x...)	do {			\
	if (debug_mask & DEBUG_ENABLE)		\
		pr_info(x);			\
} while (0)

static struct notifier_block es_notifier;
static DEFINE_MUTEX(early_suspend_lock);
static LIST_HEAD(early_suspend_handlers);
enum {
	EARLY_EVENT_SUSPENDED = 0x1,
	EVENT_SUSPENDED = 0x2,
	SUSPENDED = EVENT_SUSPENDED | EARLY_EVENT_SUSPENDED,
};
static int state;

static int is_blank(int blank)
{
	switch (blank) {
	case FB_BLANK_HSYNC_SUSPEND:
	case FB_BLANK_VSYNC_SUSPEND:
	case FB_BLANK_NORMAL:
		return 1;

	case FB_BLANK_UNBLANK:
		return 0;
	default:
		break;
	}
	return -1;
}
static void early_suspend_func(unsigned long event)
{
	struct early_suspend *pos;

	ES_DBG("early_suspend: call handlers on event = %lu\n", event);
	list_for_each_entry(pos, &early_suspend_handlers, link) {
		if ((event == FB_EVENT_BLANK) && (pos->level <
			EARLY_SUSPEND_LEVEL_BLANK_SCREEN))
			continue;
		else if ((event == FB_EARLY_EVENT_BLANK) && (pos->level >=
			EARLY_SUSPEND_LEVEL_BLANK_SCREEN))
			break;
		if (pos->suspend != NULL) {
			ES_DBG("early_suspend: calling %pf\n", pos->suspend);
			pos->suspend(pos);
		}
	}
	ES_DBG("early_suspend: DONE event = %lu\n", event);

}

static void late_resume_func(unsigned long event)
{
	struct early_suspend *pos;
	ES_DBG("late_resume: call handlers on event = %lu\n", event);
	list_for_each_entry_reverse(pos, &early_suspend_handlers, link) {
		if ((event == FB_EARLY_EVENT_BLANK) && pos->level >=
			EARLY_SUSPEND_LEVEL_BLANK_SCREEN)
			continue;
		else if ((event == FB_EVENT_BLANK) && (pos->level <
			EARLY_SUSPEND_LEVEL_BLANK_SCREEN))
			break;
		if (pos->resume != NULL) {
			ES_DBG("late_resume: calling %pf\n", pos->resume);
			pos->resume(pos);
		}
	}
	ES_DBG("late_resume: DONE event = %lu\n", event);
}

static int early_suspend_notifier_callback(struct notifier_block *self,
					   unsigned long event, void *data)
{
	struct fb_event *evdata = data;
	int blank = *(int *)(evdata->data);

	switch (event) {
	case FB_EVENT_CONBLANK:
	case FB_EVENT_BLANK:
		mutex_lock(&early_suspend_lock);
		if (is_blank(blank) == 1) {
			if (!(state & EVENT_SUSPENDED)) {
				state |= EVENT_SUSPENDED;
				early_suspend_func(FB_EVENT_BLANK);
			}
		} else if (is_blank(blank) == 0) {
			if (state & EVENT_SUSPENDED) {
				state &= ~EVENT_SUSPENDED;
				late_resume_func(FB_EVENT_BLANK);
			}
		}
		break;
	case FB_EARLY_EVENT_BLANK:
		mutex_lock(&early_suspend_lock);
		if (is_blank(blank) == 1) {
			if (!(state & EARLY_EVENT_SUSPENDED)) {
				state |= EARLY_EVENT_SUSPENDED;
				early_suspend_func(FB_EARLY_EVENT_BLANK);
			}
		} else if (is_blank(blank) == 0) {
			if (state & EARLY_EVENT_SUSPENDED) {
				state &= ~EARLY_EVENT_SUSPENDED;
				late_resume_func(FB_EARLY_EVENT_BLANK);
			}
		}
		break;

	case FB_R_EARLY_EVENT_BLANK:
		mutex_lock(&early_suspend_lock);
		if (is_blank(blank) == 1) {
			if (state & EARLY_EVENT_SUSPENDED) {
				state &= ~EARLY_EVENT_SUSPENDED;
				late_resume_func(FB_EARLY_EVENT_BLANK);
			}
		} else if (is_blank(blank) == 0) {
			if (!(state & EARLY_EVENT_SUSPENDED)) {
				state |= EARLY_EVENT_SUSPENDED;
				early_suspend_func(FB_EARLY_EVENT_BLANK);
			}
		}
		break;
	default:
		return 0;
	}
	mutex_unlock(&early_suspend_lock);
	return 0;

}

void register_early_suspend(struct early_suspend *handler)
{
	struct list_head *pos;

	mutex_lock(&early_suspend_lock);
	list_for_each(pos, &early_suspend_handlers) {
		struct early_suspend *e;
		e = list_entry(pos, struct early_suspend, link);
		if (e->level > handler->level)
			break;
	}
	list_add_tail(&handler->link, pos);
	if ((state & EARLY_EVENT_SUSPENDED) && handler->suspend &&
		(handler->level < EARLY_SUSPEND_LEVEL_BLANK_SCREEN))
		handler->suspend(handler);

	else if	((state & EVENT_SUSPENDED) && handler->suspend &&
		(handler->level >= EARLY_SUSPEND_LEVEL_BLANK_SCREEN))
		handler->suspend(handler);

	mutex_unlock(&early_suspend_lock);

}
EXPORT_SYMBOL(register_early_suspend);

void unregister_early_suspend(struct early_suspend *handler)
{
	mutex_lock(&early_suspend_lock);
	list_del(&handler->link);
	mutex_unlock(&early_suspend_lock);

}
EXPORT_SYMBOL(unregister_early_suspend);

static int early_suspend_init(void)
{
	memset(&es_notifier, 0, sizeof(es_notifier));
	es_notifier.notifier_call = early_suspend_notifier_callback;
	fb_register_client(&es_notifier);
	return 0;
}

late_initcall(early_suspend_init);

/* Remove this once all the calls to this function is removed */
void request_suspend_state(suspend_state_t new_state)
{
}


