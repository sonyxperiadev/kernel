/*
 * Copyright (C) 2017 Sony Mobile Communications Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation.
 */
#ifndef WATER_DETECTION_H_
#define WATER_DETECTION_H_

#include <linux/spinlock.h>
#include <linux/alarmtimer.h>

enum wdet_det_state {
	WDET_NONE,
	WDET_OR,
	WDET_AND,
};

enum wdet_polling_state {
	WDET_POLLING_START,
	WDET_POLLING_STOP,
	WDET_POLLING_OR_DETECTED,
};

enum wdet_timer_kind {
	WDET_TIMER_SHORT,
	WDET_TIMER_LONG,
	WDET_TIMER_STOP,
};

struct wdet;

void wdet_check_water(struct wdet *wdet);
int wdet_polling_set(struct wdet *wdet, enum wdet_polling_state state);
int wdet_polling_reset(struct wdet *wdet);
int wdet_invalid_during_negotiation(struct wdet *wdet);
struct wdet *wdet_create(struct device *parent);
void wdet_destroy(struct wdet *wdet);

#endif /* WATER_DETECTION_H_ */
