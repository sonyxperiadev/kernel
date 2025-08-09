/*
 * SPDX-License-Identifier: GPL-2.0
 *
 * lxs_ts.c
 *
 * LXS touch core layer
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "lxs_ts.h"

u32 t_pr_dbg_mask;
u32 t_dev_dbg_mask = DBG_NONE;
u32 t_bus_dbg_mask;

/* usage
 * (1) echo <value> > /sys/module/{Touch Module Name}/parameters/pr_dbg_mask
 * (2) insmod {Touch Module Name}.ko pr_dbg_mask=<value>
 */
module_param_named(pr_dbg_mask, t_pr_dbg_mask, uint, S_IRUGO|S_IWUSR|S_IWGRP);

/* usage
 * (1) echo <value> > /sys/module/{Touch Module Name}/parameters/dev_dbg_mask
 * (2) insmod {Touch Module Name}.ko dev_dbg_mask=<value>
 */
module_param_named(dev_dbg_mask, t_dev_dbg_mask, uint, S_IRUGO|S_IWUSR|S_IWGRP);

/* usage
 * (1) echo <value> > /sys/module/{Touch Module Name}/parameters/bus_dbg_mask
 * (2) insmod {Touch Module Name}.ko bus_dbg_mask=<value>
 */
module_param_named(bus_dbg_mask, t_bus_dbg_mask, uint, S_IRUGO|S_IWUSR|S_IWGRP);

static void lxs_ts_options(struct lxs_ts *ts)
{
#if defined(__LXS_SUPPORT_PINCTRL)
	t_dev_info(ts->dev, "build opt  : __LXS_SUPPORT_PINCTRL\n");
#endif

#if defined(__LXS_CONFIG_FB)
	t_dev_info(ts->dev, "build opt  : __LXS_CONFIG_FB\n");
#endif

#if defined(__LXS_SUPPORT_OLED)
	t_dev_info(ts->dev, "build opt  : __LXS_SUPPORT_OLED\n");
#endif

#if defined(__LXS_SUPPORT_LPWG)
	t_dev_info(ts->dev, "build opt  : __LXS_SUPPORT_LPWG\n");
#endif

#if defined(__LXS_SUPPORT_SLEEP)
	t_dev_info(ts->dev, "build opt  : __LXS_SUPPORT_SLEEP\n");
#endif

#if defined(__LXS_SUPPORT_WATCHDOG)
	t_dev_info(ts->dev, "build opt  : __LXS_SUPPORT_WATCHDOG\n");
#endif
}

static int drm_notifier_callback(struct notifier_block *self, unsigned long event, void *data);

static void lxs_ts_params(struct lxs_ts *ts)
{
	lxs_ts_options(ts);

	t_dev_info(ts->dev, "chip type  : 0x%04X\n", ts->chip_type);
	t_dev_info(ts->dev, "chip class : %s\n", ts->chip_id);
	t_dev_info(ts->dev, "chip name  : %s\n", ts->chip_name);
	t_dev_info(ts->dev, "drv name   : %s\n", LXS_TS_NAME);

	if (ts->idx)
		snprintf(ts->input_name, sizeof(ts->input_name), "%s%d", LXS_TS_INPUT, ts->idx);
	else
		strlcpy(ts->input_name, LXS_TS_INPUT, sizeof(ts->input_name));
	t_dev_info(ts->dev, "input name : %s\n", ts->input_name);

	snprintf(ts->input_phys, sizeof(ts->input_phys),
		"%s/%s/%s", dev_name(ts->dev->parent), dev_name(ts->dev), ts->input_name);

	t_dev_info(ts->dev, "max finger : %d\n", ts->max_finger);

	t_dev_info(ts->dev, "mode bit   : 0x%08X\n", ts->mode_allowed);

	t_dev_info(ts->dev, "tx_hdr     : %d\n", ts->bus_tx_hdr_size);
	t_dev_info(ts->dev, "rx_hdr     : %d\n", ts->bus_rx_hdr_size);
	t_dev_info(ts->dev, "tx_dummy   : %d\n", ts->bus_tx_dummy_size);
	t_dev_info(ts->dev, "rx_dummy   : %d\n", ts->bus_rx_dummy_size);
}

#if defined(__LXS_CONFIG_FB)
static int lxs_ts_fb_notifier_cb(
			struct notifier_block *self,
			unsigned long event, void *data)
{
	struct lxs_ts *ts =
		container_of(self, struct lxs_ts, fb_notif);
	struct fb_event *ev = (struct fb_event *)data;
	int blank = 0;

	if (!ev || !ev->data)
		return 0;

	blank = *(int *)ev->data;

	/*
	 * See fb_blank (fbmem.c)
	 * revert effects of the ealry blank effect
	 */
	if (event == FB_R_EARLY_EVENT_BLANK) {
		switch (blank) {
		case FB_BLANK_UNBLANK:
			t_dev_info(ts->dev, "fb_unblank(r_early)\n");
			break;
		case FB_BLANK_POWERDOWN:
			t_dev_info(ts->dev, "fb_blank(r_early)\n");
			break;
		}
		return 0;
	}

	if (event == FB_EARLY_EVENT_BLANK) {
		switch (blank) {
		case FB_BLANK_UNBLANK:
			t_dev_info(ts->dev, "fb_unblank(early)\n");
			break;
		case FB_BLANK_POWERDOWN:
			t_dev_info(ts->dev, "fb_blank(early)\n");
			break;
		}
		return 0;
	}

	if (event == FB_EVENT_BLANK) {
		switch (blank) {
		case FB_BLANK_UNBLANK:
			t_dev_info(ts->dev, "fb_unblank\n");
			break;
		case FB_BLANK_POWERDOWN:
			t_dev_info(ts->dev, "fb_blank\n");
			break;
		}
		return 0;
	}

	return 0;
}

static int lxs_ts_init_pm(struct lxs_ts *ts)
{
	t_dev_info(ts->dev, "init fb_notif\n");

	ts->fb_notif.notifier_call = lxs_ts_fb_notifier_cb;
	return fb_register_client(&ts->fb_notif);
}

static void lxs_ts_free_pm(struct lxs_ts *ts)
{
	t_dev_info(ts->dev, "free fb_notif\n");

	fb_unregister_client(&ts->fb_notif);
}
#else
static inline int lxs_ts_init_pm(struct lxs_ts *ts)
{
	return 0;
}

static inline void lxs_ts_free_pm(struct lxs_ts *ts)
{

}
#endif

#define TS_LOCATION_DETECT_SIZE	6

static void location_detect(struct lxs_ts *ts, char *loc, int x, int y)
{
	int i;

	for (i = 0; i < TS_LOCATION_DETECT_SIZE; ++i)
		loc[i] = 0;

	if (x < ts->plat_data->area_edge)
		strcat(loc, "E.");
	else if (x < (ts->plat_data->max_x - ts->plat_data->area_edge))
		strcat(loc, "C.");
	else
		strcat(loc, "e.");

	if (y < ts->plat_data->area_indicator)
		strcat(loc, "S");
	else if (y < (ts->plat_data->max_y - ts->plat_data->area_navigation))
		strcat(loc, "C");
	else
		strcat(loc, "N");
}

static void lxs_ts_finger_log(struct lxs_ts *ts,
		u32 new_mask, u32 press_mask, u32 release_mask, int release_all)
{
	struct input_dev *input = ts->input;
	const char *iname = dev_name(&input->dev);
	struct touch_data *tdata = ts->tdata;
	int tcount = ts->tcount;
	char location[TS_LOCATION_DETECT_SIZE] = { 0, };
	int i;
	int *mc = ts->mc;
	int *pre_x = ts->pre_x;
	int *pre_y = ts->pre_y;

	for (i = 0; i < ts->max_finger; i++, tdata++) {
		if (new_mask & BIT(i)) {
			location_detect(ts, location, tdata->x, tdata->y);
			if (press_mask & BIT(i)) {
				t_dev_dbg_event(ts->dev,
					"[P] tID:%d.%d x:%d y:%d z:%d major:%d minor:%d or:%d loc:%s tc:%d type:%X %s\n",
					i, (input->mt->trkid - 1) & TRKID_MAX,
					tdata->x, tdata->y, tdata->pressure,
					tdata->width_major, tdata->width_minor,
					tdata->orientation, location, tcount, tdata->type, iname);
				pre_x[i] = tdata->x;
				pre_y[i] = tdata->y;
				mc[i] = 0;
				continue;
			}

			mc[i]++;
			t_dev_dbg_event_m(ts->dev,
				"[C] tID:%d.%d x:%d y:%d z:%d major:%d minor:%d or:%d loc:%s tc:%d type:%X %s\n",
				i, (input->mt->trkid - 1) & TRKID_MAX,
				tdata->x, tdata->y, tdata->pressure,
				tdata->width_major, tdata->width_minor,
				tdata->orientation, location, tcount, tdata->type, iname);
			continue;
		}

		if (release_mask & BIT(i)) {
			char *rstr = "[R]";

			if (release_all)
				rstr = "[RA]";
			else if ((ts->intr_palm & BIT(16)) || (ts->intr_palm & BIT(i)))
				rstr = "[RP]";

			t_dev_dbg_event(ts->dev,
				"%s tID:%d dd:%d,%d mc:%d tc:%d lx:%d ly:%d %s\n",
				rstr, i, pre_x[i] - tdata->x, pre_y[i] - tdata->y,
				mc[i], tcount, tdata->x, tdata->y, iname);
			mc[i]= 0;
			pre_x[i] = 0;
			pre_y[i] = 0;
		}
	}
}

static void report_clear(struct touch_data *tdata)
{
	tdata->report_flag = false;
	tdata->saved = false;
	tdata->saved_data_x = 0;
	tdata->saved_data_y = 0;
}

static void report_save(struct touch_data *tdata)
{
	tdata->saved_data_x = tdata->x;
	tdata->saved_data_y = tdata->y;
	tdata->saved = true;
}

static void report_touch(struct lxs_ts *ts, struct touch_data *tdata, int i, bool stored)
{
	char location[TS_LOCATION_DETECT_SIZE] = { 0, };
	u16 x, y;
	struct input_dev *input = ts->input;

	if (!ts->report_rejected_event_flag && stored)
		return;

	if (stored) {
		if (!tdata->saved)
			return;
		tdata->saved = false;

		x = tdata->saved_data_x;
		y = tdata->saved_data_y;
	} else {
		x = tdata->x;
		y = tdata->y;
	}

	input_mt_slot(input, i);
#if defined(__LXS_CONFIG_INPUT_ANDROID)
	input_mt_report_slot_state(input, MT_TOOL_FINGER, true);
#endif	/* __LXS_CONFIG_INPUT_ANDROID */
	input_report_key(input, BTN_TOUCH, 1);
	input_report_key(input, BTN_TOOL_FINGER, 1);
	input_report_abs(input, ABS_MT_TRACKING_ID, tdata->id);
	input_report_abs(input, ABS_MT_POSITION_X, x);
	input_report_abs(input, ABS_MT_POSITION_Y, y);
	input_report_abs(input, ABS_MT_PRESSURE, tdata->pressure);
	input_report_abs(input, ABS_MT_TOUCH_MAJOR, tdata->width_major);
	input_report_abs(input, ABS_MT_TOUCH_MINOR, tdata->width_minor);
	input_report_abs(input, ABS_MT_ORIENTATION, tdata->orientation);
	if (ts->plat_data->mt_tool_max)
		input_report_abs(input, ABS_MT_TOOL_TYPE, tdata->type);

	if (stored) {
		input_sync(input);

		location_detect(ts, location, x, y);
		t_dev_dbg_event(ts->dev,
			"[G] tID:%d.%d x:%d y:%d z:%d major:%d minor:%d or:%d loc:%s tc:%d type:%X %s\n",
			i, (input->mt->trkid - 1) & TRKID_MAX,
			x, y, tdata->pressure,
			tdata->width_major, tdata->width_minor,
			tdata->orientation, location, ts->tcount, tdata->type, dev_name(&input->dev));
	}
}

static int get_location(struct lxs_ts *ts, u32 x, u32 y)
{
	if (x > ts->plat_data->max_x / 2) {
		if (y > ts->plat_data->max_y / 2)
			return CORNER_3;
		return CORNER_2;
	} else {
		if (y > ts->plat_data->max_y / 2)
			return CORNER_1;
		return CORNER_0;
	}
}

static u32 compute_sum_of_squares(u32 x, u32 y)
{
	return x * x + y * y;
}

static bool check_area(struct lxs_ts *ts, int location, u32 x, u32 y)
{
	if (ts->landscape) {
		switch (location) {
		case CORNER_0:
			break;
		case CORNER_1:
			y = ts->plat_data->max_y - y;
			break;
		case CORNER_2:
			x = ts->plat_data->max_x - x;
			break;
		case CORNER_3:
			x = ts->plat_data->max_x - x;
			y = ts->plat_data->max_y - y;
			break;
		default:
			return false;
		}

		return compute_sum_of_squares(x, y) < ts->circle_range_l[location];
	} else {
		if (location == CORNER_1) {
			y = ts->plat_data->max_y - y;
			location = CORNER_0;
		} else if (location == CORNER_3) {
			x = ts->plat_data->max_x - x;
			y = ts->plat_data->max_y - y;
			location = CORNER_1;
		} else {
			return false;
		}

		return compute_sum_of_squares(x, y) < ts->circle_range_p[location];
	}
}

static void lxs_ts_finger_report_event(struct lxs_ts *ts, int release_all)
{
	struct input_dev *input = ts->input;
	struct touch_data *tdata = ts->tdata;
	const char *iname = NULL;
	u32 old_mask = ts->old_mask;
	u32 new_mask = ts->new_mask;
	u32 press_mask = 0;
	u32 release_mask = 0;
	u32 change_mask = 0;
	u32 event_cnt = ts->event_cnt_finger;
	int is_finger = !!(ts->intr_status & TS_IRQ_FINGER);
//	int tcount = ts->tcount;
	int i;
	int location;
	u32 *area_buffer = (ts->landscape) ? ts->landscape_buffer : ts->portrait_buffer;
	u32 max_x = ts->plat_data->max_x;
	u32 max_y = ts->plat_data->max_y;
	//for grip_area : X-left, X-right, Y-top, Y-bottom
	u32 area_g_x_l = min(max_x, area_buffer[0]);
	u32 area_g_x_r = max_x - area_g_x_l;
	u32 area_g_y_t = min(max_y, area_buffer[1]);
	u32 area_g_y_b = max_y - area_g_y_t;
	//for send_area : X-left, X-right, Y-top, Y-bottom
	u32 area_s_x_l = min(max_x, area_buffer[2]);
	u32 area_s_x_r = max_x - area_s_x_l;
	u32 area_s_y_t = min(max_y, area_buffer[3]);
	u32 area_s_y_b = max_y - area_s_y_t;
	int is_grip_area, is_send_area;

//	t_dev_trcf(idev);

	if (!input) {
		t_dev_err(ts->dev, "no input device (report)\n");
		return;
	}

	iname = dev_name(&input->dev);

	if (!is_finger)
		return;

	change_mask = old_mask ^ new_mask;
	press_mask = new_mask & change_mask;
	release_mask = old_mask & change_mask;

	t_dev_dbg_evt(ts->dev,
		"%s: <%d> mask [new: %04x, old: %04x]\n",
		iname, event_cnt, new_mask, old_mask);
	t_dev_dbg_evt(ts->dev,
		"%s: <%d> mask [change: %04x, press: %04x, release: %04x]\n",
		iname, event_cnt, change_mask, press_mask, release_mask);

	/* Palm state - Report Pressure value 255 */
	if (ts->is_cancel) {
		if (ts->fquirks->cancel_event_finger)
			ts->fquirks->cancel_event_finger(ts);

		ts->is_cancel = 0;
	}

	for (i = 0; i < ts->max_finger; i++, tdata++) {
		if (new_mask & BIT(i)) {
			is_grip_area = 0;
			is_send_area = 0;
			if (!ts->rejection_mode && area_g_x_l) {
				u32 __area_g_y_t = (ts->landscape) ? area_g_y_t : 0;
				u32 __area_s_y_t = (ts->landscape) ? area_s_y_t : 0;

				if ((tdata->x < area_g_x_l || tdata->x > area_g_x_r) &&
					(tdata->y < __area_g_y_t || tdata->y > area_g_y_b))
					is_grip_area = 1;
				else if ((tdata->x > area_s_x_l && tdata->x < area_s_x_r) ||
					(tdata->y > __area_s_y_t && tdata->y < area_s_y_b))
					is_send_area = 1;
			}

			if (press_mask & BIT(i)) {
				if (ts->rejection_mode) {
					location = get_location(ts, tdata->x, tdata->y);
					if (check_area(ts, location, tdata->x, tdata->y)) {
						report_save(tdata);
						continue;
					}
					tdata->report_flag = true;
				} else {
					if (is_grip_area) {
						if (!tdata->report_flag) {
							report_save(tdata);
							continue;
						}
					} else
						tdata->report_flag = true;
				}
			} else {
				if (ts->rejection_mode) {
					location = get_location(ts, tdata->x, tdata->y);
					if (check_area(ts, location, tdata->x, tdata->y))
						continue;
					if (!tdata->report_flag){
						report_touch(ts, tdata, i, true);
						tdata->report_flag = true;
					}
				} else {
					if (!tdata->report_flag){
						if (!is_send_area)
							continue;
						report_touch(ts, tdata, i, true);
						tdata->report_flag = true;
					}
				}
			}
			report_touch(ts, tdata, i, false);
			continue;
		}

		if (release_mask & BIT(i)) {
			report_clear(tdata);
			input_mt_slot(input, i);
		#if defined(__LXS_CONFIG_INPUT_ANDROID)
			input_mt_report_slot_state(input, MT_TOOL_FINGER, false);
		#else	/* __LXS_CONFIG_INPUT_ANDROID */
			input_report_abs(ts->input, ABS_MT_TRACKING_ID, -1);
		#endif	/* __LXS_CONFIG_INPUT_ANDROID */
		}
	}

	if (!new_mask) {
		input_report_key(input, BTN_TOUCH, 0);
		input_report_key(input, BTN_TOOL_FINGER, 0);
	}

	ts->old_mask = new_mask;

	input_sync(input);

	lxs_ts_finger_log(ts, new_mask, press_mask, release_mask, release_all);

	ts->event_cnt_finger++;
}

static void lxs_ts_finger_release_all_event(struct lxs_ts *ts)
{
	ts->is_cancel = 1;
	if (ts->old_mask) {
		ts->new_mask = 0;
		ts->intr_status = TS_IRQ_FINGER;
		lxs_ts_finger_report_event(ts, 1);
		ts->tcount = 0;
		memset(ts->tdata, 0, sizeof(ts->tdata));
	}
	ts->is_cancel = 0;
}

static void lxs_ts_report_event(struct lxs_ts *ts)
{
	lxs_ts_finger_report_event(ts, 0);
}

void lxs_ts_release_all_event(struct lxs_ts *ts)
{
	lxs_ts_finger_release_all_event(ts);
}

/* Initialize multi-touch slot */
static int lxs_ts_input_mt_init_slots(struct lxs_ts *ts, struct input_dev *input)
{
//	struct device *dev = ts->dev;
	int max_id = MAX_FINGER;
	int ret = 0;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 7, 0))
	ret = input_mt_init_slots(input, max_id);
#else
	ret = input_mt_init_slots(input, max_id, INPUT_MT_DIRECT);
#endif

	return ret;
}

static int lxs_ts_finger_init(struct lxs_ts *ts)
{
	struct input_dev *input = NULL;
	int max_pressure = FINGER_MAX_PRESSURE;
	int max_width = FINGER_MAX_WIDTH;
	int max_orient = FINGER_MAX_ORIENT;
	int ret = 0;

	ts->event_cnt_finger = 0;

	if (ts->input) {
		t_dev_err(ts->dev, "input device has been already allocated!\n");
		return -EINVAL;
	}

	input = input_allocate_device();
	if (!input) {
		t_dev_err(ts->dev, "failed to allocate memory for input device\n");
		ret = -ENOMEM;
		goto out;
	}

	/*
	 * To fix sysfs location
	 * With no parent, sysfs folder is created at
	 * '/sys/devices/virtual/input/{input_name}'
	 */
	input->dev.parent = (ts->plat_data->use_input_parent) ? ts->dev : NULL;

	input->name = (const char *)ts->input_name;
	input->phys = (const char *)ts->input_phys;;
	input->id.bustype = ts->bus_type;

	set_bit(EV_SYN, input->evbit);
	set_bit(EV_ABS, input->evbit);
	set_bit(EV_KEY, input->evbit);
	set_bit(KEY_SLEEP, input->keybit);
	set_bit(KEY_WAKEUP, input->keybit);
	set_bit(KEY_POWER, input->keybit);
	set_bit(BTN_TOUCH, input->keybit);
	set_bit(BTN_TOOL_FINGER, input->keybit);
	set_bit(INPUT_PROP_DIRECT, input->propbit);
	input_set_abs_params(input, ABS_MT_POSITION_X, 0, ts->plat_data->max_x, 0, 0);
	input_set_abs_params(input, ABS_MT_POSITION_Y, 0, ts->plat_data->max_y, 0, 0);
	input_set_abs_params(input, ABS_MT_PRESSURE, 0, max_pressure, 0, 0);
	input_set_abs_params(input, ABS_MT_TOUCH_MAJOR, 0, max_width, 0, 0);
	input_set_abs_params(input, ABS_MT_TOUCH_MINOR, 0, max_width, 0, 0);
	input_set_abs_params(input, ABS_MT_ORIENTATION, -max_orient, max_orient, 0, 0);
	if (ts->plat_data->mt_tool_max)
		input_set_abs_params(input, ABS_MT_TOOL_TYPE, 0, ts->plat_data->mt_tool_max, 0, 0);

	ret = lxs_ts_input_mt_init_slots(ts, input);
	if (ret < 0) {
		t_dev_err(ts->dev, "failed to set input_mt_init_slots, %d\n", ret);
		goto out_slot;
	}

	input_set_drvdata(input, ts);

	ret = input_register_device(input);
	if (ret < 0) {
		t_dev_err(ts->dev, "failed to register input device, %d\n", ret);
		goto out_reg;
	}

	ts->input = input;

	t_dev_info(ts->dev, "input device[%s, %s] registered\n",
		dev_name(&input->dev), input->phys);
	t_dev_info(ts->dev, "input finger: x %d, y %d, press %d, width %d, orient %d, t %d\n",
		ts->plat_data->max_x, ts->plat_data->max_y, max_pressure, max_width, max_orient,
		ts->plat_data->mt_tool_max);

	return 0;

out_reg:

out_slot:
	input_free_device(input);

out:
	return ret;
}

static void lxs_ts_finger_free(struct lxs_ts *ts)
{
	struct input_dev *input = ts->input;

	if (input == NULL)
		return;

	input_unregister_device(input);

	ts->input = NULL;

	t_dev_info(ts->dev, "input device[%s] released\n", ts->input_phys);
}

static int lxs_ts_init_input(struct lxs_ts *ts)
{
	int ret = 0;

	if (!ts->plat_data->max_x || !ts->plat_data->max_y) {
		t_dev_err(ts->dev, "coord fault: max_x %d, max_y %d\n",
			ts->plat_data->max_x, ts->plat_data->max_y);
		return -EINVAL;
	}

#if defined(__LXS_CONFIG_INPUT_ANDROID)
	t_dev_info(ts->dev, "input cfg status : __LXS_CONFIG_INPUT_ANDROID\n");
#endif

	ret = lxs_ts_finger_init(ts);
	if (ret < 0)
		goto out_finger;

	return 0;

out_finger:

	return ret;
}

static void lxs_ts_free_input(struct lxs_ts *ts)
{
	lxs_ts_release_all_event(ts);

	lxs_ts_finger_free(ts);
}

static void lxs_ts_event_ctrl(struct lxs_ts *ts)
{
	if (ts->intr_status & TS_IRQ_FINGER)
		lxs_ts_report_event(ts);

	if (ts->intr_status & TS_IRQ_KNOCK) {
		struct input_dev *input = ts->input;

		pm_wakeup_event(ts->dev, 3000);

		lxs_ts_release_all_event(ts);

		t_dev_info(ts->dev, "Send KEY_WAKEUP\n");

		input_report_key(input, KEY_WAKEUP, 1);
		input_sync(input);

		input_report_key(input, KEY_WAKEUP, 0);
		input_sync(input);
	}
}

static int __lxs_ts_irq_start(struct lxs_ts *ts)
{
	int ret = 0;

	if (ts->state_core != TS_CORE_NORMAL)
		return -EINVAL;

	if (gpio_get_value(ts->plat_data->irq_pin))
		return -EINVAL;

	if (ts->suspend_is_on_going)
		return -EINVAL;

	if (ts->state_pm >= TS_DEV_PM_SUSPEND) {
		t_dev_info(ts->dev, "irq resume: interrupt in suspend[%d]\n", ts->state_pm);

		pm_wakeup_event(ts->dev, 1000);

		ret = wait_for_completion_interruptible_timeout(&ts->resume_done, msecs_to_jiffies(500));
		if (!ret) {
			t_dev_err(ts->dev, "irq resume: pm resume is not handled\n");
			return -EFAULT;
		}

		if (ret < 0) {
			t_dev_err(ts->dev, "irq resume: pm resume is not handled, %d\n", ret);
			return ret;
		}

		t_dev_info(ts->dev, "irq_resume: done, %d\n", jiffies_to_msecs(ret));
	}

	return 0;
}

static int __lxs_ts_irq_thread(struct lxs_ts *ts)
{
	int ret = 0;

	ts->intr_palm = 0;
	ts->intr_status = 0;

	ret = lxs_hal_irq_handler(ts);
	if (ret < 0) {
		t_dev_dbg_irq(ts->dev, "Err in irq_handler of %s, %d",
				ts->chip_name, ret);
		if (ret == -ETDSENTESDIRQ) {
			lxs_ts_irq_control(ts, 0);

			queue_delayed_work(ts->wq, &ts->sys_reset_work, 0);
		} else if (ret == -ERESTART) {
			if (ts->state_pm == TS_DEV_PM_RESUME)
				pm_wakeup_event(ts->dev, 1000);

			lxs_hal_reset(ts, TC_HW_RESET_ASYNC, 0);
		}
		return ret;
	}

	lxs_ts_event_ctrl(ts);

	return 0;
}

static irqreturn_t __used lxs_ts_irq_thread(int irq, void *dev_id)
{
	struct lxs_ts *ts = (struct lxs_ts *)dev_id;
	int ret = 0;

	t_dev_dbg_irq(ts->dev, "irq_thread %d\n", irq);

	if (ts->power_enabled == TS_POWER_OFF) {
		return IRQ_HANDLED;
	}

	ret = __lxs_ts_irq_start(ts);
	if (ret < 0)
		return IRQ_HANDLED;

	mutex_lock(&ts->lock);
	__lxs_ts_irq_thread(ts);
	mutex_unlock(&ts->lock);

	return IRQ_HANDLED;
}

void lxs_ts_irq_control(struct lxs_ts *ts, bool on)
{
	int irq_wake_enable = (ts->plat_data->use_lpwg && device_may_wakeup(ts->dev));
	int irq = ts->irq;

	if (ts->fquirks->irq_control) {
		ts->fquirks->irq_control(ts, on);
		return;
	}

	if (!ts->irqflags) {
		t_dev_warn(ts->dev, "irq not initialized\n");
		return;
	}

	t_dev_dbg_irq(ts->dev, "touch_irq_control(%d)\n", on);

	if (on) {
		if (ts->state_irq_enable) {
			t_dev_dbg_irq(ts->dev, "(warn) already irq enabled\n");
			return;
		}

		enable_irq(irq);
		t_dev_info(ts->dev, "irq(%d) enabled\n", irq);

		if (irq_wake_enable) {
			enable_irq_wake(irq);
			t_dev_info(ts->dev, "irq(%d) wake enabled\n", irq);
		}

		ts->state_irq_enable = 1;
	} else {
		if (!ts->state_irq_enable) {
			t_dev_dbg_irq(ts->dev, "(warn) already irq disabled\n");
			return;
		}

		if (irq_wake_enable) {
			disable_irq_wake(irq);
			t_dev_info(ts->dev, "irq(%d) wake disabled\n", irq);
		}

		disable_irq_nosync(irq);
		t_dev_info(ts->dev, "irq(%d) disabled\n", irq);

		ts->state_irq_enable = 0;
	}
}

/*
 * Verify irq handler index(ts->irq) using gpio_to_irq
 */
static int lxs_ts_irq_pin_check(struct lxs_ts *ts)
{
	int pin = ts->plat_data->irq_pin;
	int irq = 0;

	if (!gpio_is_valid(pin))
		return 0;

	irq = gpio_to_irq(pin);
	if (irq <= 0) {
		t_dev_warn(ts->dev, "check irq pin: gpio_to_irq(%d) = %d\n",
			pin, irq);
		goto out;
	}

	if (ts->irq) {
		if (ts->irq != irq)
			t_dev_warn(ts->dev,
				"check irq index: ts->irq = %d vs. gpio_to_irq(%d) = %d\n",
				ts->irq, pin, irq);

		goto out;
	}

	t_dev_info(ts->dev,
		"irq index(%d) is obtained via gpio_to_irq(%d)\n",
		irq, pin);

	ts->irq = irq;

out:
	return irq;
}

static int lxs_ts_request_irq(struct lxs_ts *ts,
			irq_handler_t handler, irq_handler_t thread_fn,
			unsigned long flags, const char *name)
{
	int ret = 0;

	lxs_ts_irq_pin_check(ts);

	if (!ts->irq) {
		t_dev_err(ts->dev, "failed to request irq : zero irq\n");
		return -EFAULT;
	}

	ret = request_threaded_irq(ts->irq, handler, thread_fn, flags, name, (void *)ts);
	if (ret) {
		t_dev_err(ts->dev, "failed to request irq(%d, %s, 0x%X), %d\n",
			ts->irq, name, (u32)flags, ret);
		return ret;
	}

	ts->irqflags = flags;

	t_dev_info(ts->dev, "threaded irq request done(%d, %s, 0x%X)\n",
		ts->irq, name, (u32)flags);

	disable_irq_nosync(ts->irq);
//	t_dev_dbg_irq(ts->dev, "disable irq until init completed\n");

	return 0;
}

static void lxs_ts_free_irq(struct lxs_ts *ts)
{
	if (!ts->irqflags)
		return;

	free_irq(ts->irq, (void *)ts);

	t_dev_info(ts->dev, "irq(%d) release done\n", ts->irq);

	ts->irqflags = 0;
}

int lxs_ts_get_pw_status(void)
{
	int ret = 0;
	incell_pw_status status = { false, false };

	ret = incell_get_power_status(&status);
	if (ret) {
		pr_err("%s: failed get power status\n", __func__);
		goto exit;
	}

	if (status.display_power && status.touch_power) {
		ret = INCELL_OK;
	} else {
		pr_err("%s: power status: disable\n", __func__);
		ret = INCELL_ERROR;
	}

exit:
	return ret;
}

int lxs_ts_pw_lock(struct lxs_ts *ts, incell_pw_lock status)
{
	incell_pw_status pwr_status = { false, false };
	int val;
	int ret = 0;

	if (status == INCELL_DISPLAY_POWER_LOCK) {
		atomic_inc(&ts->lock_cnt);
		val = atomic_read(&ts->lock_cnt);
		t_dev_dbg_base(ts->dev, "POWER_LOCK_COUNT [%d]\n", val);
		if (val != 1)
			return 0;
	} else {
		atomic_dec(&ts->lock_cnt);
		val = atomic_read(&ts->lock_cnt);
		t_dev_dbg_base(ts->dev, "POWER_UNLOCK_COUNT [%d]\n", val);
		if (val != 0)
			return 0;
	}

	ret = incell_power_lock_ctrl(status, &pwr_status);

	switch (ret) {
	case INCELL_OK:
		t_dev_dbg_base(ts->dev, "power status: %s\n", status ? "LOCK" : "UNLOCK");
		break;
	case INCELL_ERROR:
	case INCELL_ALREADY_LOCKED:
	case INCELL_ALREADY_UNLOCKED:
	default:
		t_dev_info(ts->dev, "power lock failed ret:%d\n", ret);
		break;
	}

	return ret;
}

#if defined(__LXS_SUPPORT_WATCHDOG)
#define TS_WATCHDOG_TIME_OFF	0
#define TS_WATCHDOG_TIME_MIN	1000

static int lxs_ts_watchdog_work(struct lxs_ts *ts)
{
	int ret = 0;

	mutex_lock(&ts->lock);
	ret = lxs_hal_watchdog_work(ts);
	mutex_unlock(&ts->lock);
	if (ret < 0) {
		t_dev_err(ts->dev, "%s: recovery, %d\n", __func__, ret);
		ts->watchdog_run = 0;
		return 1;	//reset
	}

	if (ret) {
		t_dev_info(ts->dev, "%s: halt, %d\n", __func__, ret);
		ts->watchdog_run = 0;
		return 0;
	}

	t_dev_dbg_base(ts->dev, "%s: ok (%d ms)\n", __func__, ts->watchdog_time);

	schedule_delayed_work(&ts->watchdog_work, msecs_to_jiffies(ts->watchdog_time));

	return 0;
}

static void lxs_ts_watchdog_work_func(struct work_struct *work)
{
	struct lxs_ts *ts =
			container_of(to_delayed_work(work),
						struct lxs_ts, watchdog_work);
	const char *sts = NULL;
	int do_reset = 0;
	int err = 0;

	if (!ts->watchdog_set)
		return;

	sts = lxs_hal_watchdog_sts(ts);
	if (sts) {
		t_dev_info(ts->dev, "%s: off (%s)\n", __func__, sts);
		return;
	}

	mutex_lock(&ts->watchdog_lock);

	if (!ts->watchdog_run)
		goto out;

	if (ts->watchdog_time == TS_WATCHDOG_TIME_OFF) {
		ts->watchdog_run = 0;
		t_dev_info(ts->dev, "%s: off (time)\n", __func__);
		goto out;
	}

	err = lxs_ts_pw_lock(ts, INCELL_DISPLAY_POWER_LOCK);
	if (err) {
		t_dev_err(ts->dev, "%s: failed lock power, %d\n", __func__, err);
		goto out;
	}

	do_reset = lxs_ts_watchdog_work(ts);

	err = lxs_ts_pw_lock(ts, INCELL_DISPLAY_POWER_UNLOCK);
	if (err)
		t_dev_err(ts->dev, "%s: failed unlock power, %d\n", __func__, err);

out:
	mutex_unlock(&ts->watchdog_lock);

	if (do_reset)
		lxs_hal_reset(ts, TC_HW_RESET_ASYNC, 0);
}

void lxs_ts_watchdog_run(struct lxs_ts *ts)
{
	const char *sts = NULL;

	if (!ts->watchdog_set)
		return;

	sts = lxs_hal_watchdog_sts(ts);
	if (sts) {
		t_dev_info(ts->dev, "%s: off (%s)\n", __func__, sts);
		return;
	}

	mutex_lock(&ts->watchdog_lock);

	if (ts->watchdog_run)
		goto out;

	if (ts->watchdog_time == TS_WATCHDOG_TIME_OFF) {
		t_dev_info(ts->dev, "%s: off (time)\n", __func__);
		goto out;
	}

	ts->watchdog_run = 1;

	schedule_delayed_work(&ts->watchdog_work, msecs_to_jiffies(ts->watchdog_time));

	t_dev_info(ts->dev, "%s: on (%d ms)\n", __func__, ts->watchdog_time);

out:
	mutex_unlock(&ts->watchdog_lock);
}

void lxs_ts_watchdog_stop(struct lxs_ts *ts)
{
	if (!ts->watchdog_set)
		return;

	mutex_lock(&ts->watchdog_lock);

	if (!ts->watchdog_run)
		goto out;

	ts->watchdog_run = 0;

	cancel_delayed_work(&ts->watchdog_work);

	t_dev_info(ts->dev, "%s\n", __func__);

out:
	mutex_unlock(&ts->watchdog_lock);
}

void lxs_ts_watchdog_time(struct lxs_ts *ts, int time)
{
	int old = ts->watchdog_time;

	if (!ts->watchdog_set)
		return;

	if (time && (time < TS_WATCHDOG_TIME_MIN)) {
		t_dev_err(ts->dev, "%s: watchdog_time invalid: %d(< %d)\n",
			__func__, time, TS_WATCHDOG_TIME_MIN);
		return;
	}

	if (old == time) {
		t_dev_info(ts->dev, "%s: watchdog_time same: %d%s\n",
			__func__, time, (time) ? "" : "(off)");
		return;
	}

	lxs_ts_watchdog_stop(ts);

	mutex_lock(&ts->watchdog_lock);

	ts->watchdog_time = time;

	t_dev_info(ts->dev, "%s: watchdog_time changed: %d%s <- %d\n",
		__func__, time, (time) ? "" : "(off)", old);

	mutex_unlock(&ts->watchdog_lock);

	lxs_ts_watchdog_run(ts);
}

static void lxs_ts_watchdog_init(struct lxs_ts *ts)
{
	struct lxs_ts_plat_data *plat_data = ts->plat_data;
	struct device_node *np = ts->dev->of_node;
	u32 val = 0;

	if (ts->watchdog_set)
		return;

	if (!of_property_read_u32(np, "watchdog_support", &val)) {
		plat_data->watchdog.support = (val) ? true : false;
		t_dev_info(ts->dev, "watchdog %s\n",
			(plat_data->watchdog.support) ? "on" : "off");
	} else {
		t_dev_info(ts->dev, "watchdog not found\n");
		plat_data->watchdog.support = false;
	}

	if (plat_data->watchdog.support) {
		if (!of_property_read_u32(np, "watchdog_delay_ms", &val)) {
			t_dev_info(ts->dev, "watchdog delay %d ms\n", val);
			if (val < TS_WATCHDOG_TIME_MIN) {
				t_dev_info(ts->dev, "watchdog disabled, delay invalid\n");
				plat_data->watchdog.support = false;
			} else {
				plat_data->watchdog.delay_ms = val;
			}
		} else {
			t_dev_info(ts->dev, "watchdog delay not found\n");
			plat_data->watchdog.support = false;
		}
	}

	if (!plat_data->watchdog.support)
		return;

	if (!plat_data->watchdog.delay_ms)
		return;

	mutex_init(&ts->watchdog_lock);

	INIT_DELAYED_WORK(&ts->watchdog_work, lxs_ts_watchdog_work_func);

	ts->watchdog_time = plat_data->watchdog.delay_ms;
	ts->watchdog_run = 0;
	ts->watchdog_set = 1;

	t_dev_info(ts->dev, "%s (%d ms)\n", __func__, ts->watchdog_time);
}

static void lxs_ts_watchdog_free(struct lxs_ts *ts)
{
	if (!ts->watchdog_set)
		return;

	ts->watchdog_set = 0;
	ts->watchdog_run = 0;
	ts->watchdog_time = 0;

	cancel_delayed_work_sync(&ts->watchdog_work);

	mutex_destroy(&ts->watchdog_lock);

	t_dev_info(ts->dev, "%s\n", __func__);
}
#else	/* !__LXS_SUPPORT_WATCHDOG */
static inline void lxs_ts_watchdog_init(struct lxs_ts *ts){	}
static inline void lxs_ts_watchdog_free(struct lxs_ts *ts){	}
#endif	/* __LXS_SUPPORT_WATCHDOG */

int lxs_ts_suspend(struct lxs_ts *ts)
{
	int ret = 0;

	t_dev_info(ts->dev, "%s: start\n", __func__);

	if (!ts->probe_done && ts->after_probe.err) {
		t_dev_err(ts->dev, "%s: probe failed. Do after_probe\n", __func__);
	//	queue_delayed_work(ts->wq, &ts->after_probe.start, 0);
		return 0;
	}

	cancel_delayed_work_sync(&ts->sys_reset_work);
	cancel_delayed_work_sync(&ts->upgrade_work);
	cancel_delayed_work_sync(&ts->init_work);
	cancel_delayed_work_sync(&ts->after_probe.start);

	ret = lxs_hal_touch_mode(ts, TS_MODE_SUSPEND);
	if (ret)
		t_dev_err(ts->dev, "%s: failed touch_mode(suspend), %d\n", __func__, ret);

	t_dev_info(ts->dev, "%s: end\n", __func__);
	return 0;
}

int lxs_ts_resume(struct lxs_ts *ts)
{
	int ret = 0;

	t_dev_info(ts->dev, "%s: start\n", __func__);

	if (!ts->probe_done && ts->after_probe.err) {
		t_dev_err(ts->dev, "%s: probe failed. Do after_probe\n", __func__);
		queue_delayed_work(ts->wq, &ts->after_probe.start, 0);
		return 0;
	}

	ret = lxs_hal_touch_mode(ts, TS_MODE_RESUME);
	if (ret)
		t_dev_err(ts->dev, "%s: failed touch_mode(resume), %d\n", __func__, ret);

	t_dev_info(ts->dev, "%s: end\n", __func__);
	return 0;
}

int drm_notifier_callback(struct notifier_block *self, unsigned long event, void *data)
{
	struct drm_ext_event *evdata = (struct drm_ext_event *)data;
	struct lxs_ts *ts = container_of(self, struct lxs_ts, drm_notif);
	struct timespec64 time;
	int blank;

	if (evdata && evdata->data) {
		if (event == DRM_EXT_EVENT_BEFORE_BLANK) {
			blank = *(int *)evdata->data;
			t_dev_info(ts->dev, "Before: %s\n",
				(blank == DRM_BLANK_POWERDOWN) ? "Powerdown" :
				(blank == DRM_BLANK_UNBLANK) ? "Unblank" :
				 "???");
			switch (blank) {
			case DRM_BLANK_POWERDOWN:
				if (!ts->probe_done) {
					t_dev_info(ts->dev, "not already sleep out\n");
					return 0;
				}

				ktime_get_boottime_ts64(&time);
				t_dev_info(ts->dev, "start@%ld.%06ld\n",
					time.tv_sec, time.tv_nsec);
				lxs_ts_suspend(ts);
				ktime_get_boottime_ts64(&time);
				t_dev_info(ts->dev, "end@%ld.%06ld\n",
					time.tv_sec, time.tv_nsec);
				break;
			case DRM_BLANK_UNBLANK:
				break;
			default:
				break;
			}
		} else if (event == DRM_EXT_EVENT_AFTER_BLANK) {
			blank = *(int *)evdata->data;
			t_dev_info(ts->dev, "After: %s\n",
				(blank == DRM_BLANK_POWERDOWN) ? "Powerdown" :
				(blank == DRM_BLANK_UNBLANK) ? "Unblank" :
				 "???");
			switch (blank) {
			case DRM_BLANK_POWERDOWN:
				/* after screen to LP/Off mode */
				if (ts->probe_done)
					lxs_ts_release_all_event(ts);
				break;
			case DRM_BLANK_UNBLANK:
				if (!ts->probe_done) {
					t_dev_info(ts->dev, "not already sleep out\n");
					if (ts->after_probe.err)
						lxs_ts_resume(ts);
				} else {
					ktime_get_boottime_ts64(&time);
					t_dev_info(ts->dev, "start@%ld.%06ld\n",
						time.tv_sec, time.tv_nsec);
					lxs_ts_resume(ts);
					ktime_get_boottime_ts64(&time);
					t_dev_info(ts->dev, "end@%ld.%06ld\n",
						time.tv_sec, time.tv_nsec);
				}
				break;
			default:
				break;
			}
		}
	}

	return 0;
}

#define LXS_TS_RETRY_SESSION_COUNT 30
static void lxs_ts_after_probe_func(struct work_struct *work)
{
	struct lxs_ts *ts =
			container_of(to_delayed_work(work),
						struct lxs_ts, after_probe.start);
	int ret = 0;

	t_dev_info(ts->dev, "LXS(%s) after probe start(%s)\n",
		ts->chip_name, LXS_DRV_VERSION);

	if (ts->probe_done) {
		t_dev_info(ts->dev, "after probe already done\n");
		return;
	}

	if (lxs_ts_get_pw_status()) {
		t_dev_err(ts->dev, "params update, will update feature on resume\n");
		goto after_probe_fail;
	}
	ts->power_enabled = TS_POWER_ON;
	t_dev_info(ts->dev, "%s: change power status to power on\n", __func__);
	lxs_ts_pw_lock(ts, INCELL_DISPLAY_POWER_LOCK);
	mutex_lock(&ts->lock);
	ret = lxs_hal_probe(ts);
	if (ret) {
		t_dev_err(ts->dev, "failed to probe, %d\n", ret);
		goto init_hal_fail;
	}

	if (ts->idx)
		snprintf(ts->irq_name, sizeof(ts->irq_name), "%s%d", LXS_TS_NAME, ts->idx);
	else
		strlcpy(ts->irq_name, LXS_TS_NAME, sizeof(ts->irq_name));
	ret = lxs_ts_request_irq(ts, NULL, lxs_ts_irq_thread,
			IRQF_TRIGGER_FALLING | IRQF_ONESHOT, ts->irq_name);
	if (ret) {
		t_dev_err(ts->dev, "failed to request thread irq(%d), %d\n",
			ts->irq, ret);
		goto request_irq_fail;
	}

	lxs_hal_activate(ts);

	mutex_unlock(&ts->lock);
	lxs_ts_pw_lock(ts, INCELL_DISPLAY_POWER_UNLOCK);

	ts->probe_done = 1;
	ts->after_probe.err = false;

	t_dev_info(ts->dev, "LXS Touch Probe done\n");

#if defined(__LXS_SUPPORT_AUTO_START)
	t_dev_info(ts->dev, "LXS Touch Auto Start\n");
	queue_delayed_work(ts->wq, &ts->init_work, 0);
#endif

	lxs_ts_watchdog_init(ts);

	return;

request_irq_fail:
	lxs_hal_remove(ts);
init_hal_fail:
	mutex_unlock(&ts->lock);
	lxs_ts_pw_lock(ts, INCELL_DISPLAY_POWER_UNLOCK);
after_probe_fail:
	ts->power_enabled = TS_POWER_OFF;
	ts->after_probe_retry ++;
	if (ts->after_probe_retry > LXS_TS_RETRY_SESSION_COUNT) {
		t_dev_err(ts->dev, "stop after_work\n");
		ts->after_probe.err = true;
		return;
	}
	queue_delayed_work(ts->wq, &ts->after_probe.start, 1 * HZ);
}

static void lxs_ts_init_work_func(struct work_struct *work)
{
	struct lxs_ts *ts =
			container_of(to_delayed_work(work),
						struct lxs_ts, init_work);
	int is_probe = (ts->state_core == TS_CORE_PROBE);
	int do_fw_upgrade = 0;
	int err = 0;
	int ret = 0;

	t_dev_info(ts->dev, "LXS(%s) init work start(%s)\n",
		ts->chip_name, LXS_DRV_VERSION);

	if (is_probe)
		do_fw_upgrade |= !!(ts->plat_data->use_fw_upgrade);

	mutex_lock(&ts->lock);

	err = lxs_ts_pw_lock(ts, INCELL_DISPLAY_POWER_LOCK);
	if (err) {
		t_dev_err(ts->dev, "%s: failed lock power, %d\n", __func__, err);
		mutex_unlock(&ts->lock);
		return;
	}

	ret = lxs_hal_init(ts);
	if (!ret)
		lxs_ts_irq_control(ts, 1);

	err = lxs_ts_pw_lock(ts, INCELL_DISPLAY_POWER_UNLOCK);
	if (err)
		t_dev_err(ts->dev, "%s: failed unlock power, %d\n", __func__, err);

	mutex_unlock(&ts->lock);
	if (ret == -ETDBOOTFAIL) {
		/* boot fail detected, do fw_upgrade */
		do_fw_upgrade |= 0x2;
	} else if (ret < 0) {
		if (is_probe) {
			t_dev_err(ts->dev, "LXS(%s) init work failed(%d), try again\n",
				ts->chip_name, ret);

			ts->state_core = TS_CORE_NORMAL;
			queue_delayed_work(ts->wq, &ts->init_work, 0);
			return;
		}

		t_dev_err(ts->dev, "LXS(%s) init work failed, %d\n",
				ts->chip_name, ret);
		return;
	}

	if (do_fw_upgrade) {
		t_dev_info(ts->dev, "Touch F/W upgrade triggered(%Xh)\n", do_fw_upgrade);

		queue_delayed_work(ts->wq, &ts->upgrade_work, 0);
		return;
	}

	ts->state_core = TS_CORE_NORMAL;

	t_dev_dbg_base(ts->dev, "init work done\n");

	if (lxs_ts_irq_level_check(ts))
		lxs_ts_irq_thread(ts->irq, ts);

	lxs_ts_watchdog_run(ts);
}

static int lxs_ts_upgrade_work(struct lxs_ts *ts)
{
	int core_state = ts->state_core;
	int irq_state = ts->state_irq_enable;
	int err = 0;
	int ret = 0;

	t_dev_info(ts->dev, "FW upgrade work func\n");

	ts->state_core = TS_CORE_UPGRADE;
	ts->plat_data->use_fw_upgrade = 0;

	mutex_lock(&ts->lock);
	err = lxs_ts_pw_lock(ts, INCELL_DISPLAY_POWER_LOCK);
	if (err) {
		ts->state_core = core_state;
		t_dev_err(ts->dev, "%s: failed lock power, %d\n", __func__, err);
		mutex_unlock(&ts->lock);
		return -EIO;
	}

	lxs_ts_irq_control(ts, 0);

	ret = lxs_hal_upgrade(ts);

	err = lxs_ts_pw_lock(ts, INCELL_DISPLAY_POWER_UNLOCK);
	if (err)
		t_dev_err(ts->dev, "%s: failed unlock power, %d\n", __func__, err);

	mutex_unlock(&ts->lock);

	/* init force_upgrade */
	ts->force_fwup = TS_FORCE_FWUP_CLEAR;
	ts->test_fwpath[0] = '\0';

	/* upgrade not granted */
	if (ret == EACCES) {
		ts->state_core = core_state;
		if (irq_state)
			lxs_ts_irq_control(ts, 1);

		return 0;	/* skip reset */
	}

	if (ret < 0) {
		if (ret == -EPERM)
			t_dev_err(ts->dev, "FW upgrade skipped\n");
		else
			t_dev_err(ts->dev, "FW upgrade halted, %d\n", ret);
	}

	return 1;		/* do reset */
}

static void lxs_ts_upgrade_work_func(struct work_struct *work)
{
	struct lxs_ts *ts =
			container_of(to_delayed_work(work),
						struct lxs_ts, upgrade_work);
	int pwr_con = !!(ts->plat_data->use_fw_pwr_rst);
	int ret = 0;

	lxs_ts_watchdog_stop(ts);

	ret = lxs_ts_upgrade_work(ts);
	if (ret)
		lxs_hal_reset(ts, TC_HW_RESET_ASYNC, pwr_con);
	else
		lxs_ts_watchdog_run(ts);
}

static void lxs_ts_sys_reset_work_func(struct work_struct *work)
{
	struct lxs_ts *ts =
			container_of(to_delayed_work(work),
				struct lxs_ts, sys_reset_work);
#if defined(__LXS_SUPPORT_SYS_RESET)
	struct input_dev *input = ts->input;

	t_dev_info(ts->dev, "%s: send KEY_SLEEP\n", __func__);

	input_report_key(input, KEY_SLEEP, 1);
	input_sync(input);

	input_report_key(input, KEY_SLEEP, 0);
	input_sync(input);

	lxs_ts_delay(300);

	t_dev_info(ts->dev, "%s: send KEY_WAKEUP\n", __func__);

	input_report_key(input, KEY_WAKEUP, 1);
	input_sync(input);

	input_report_key(input, KEY_WAKEUP, 0);
	input_sync(input);
#else
	t_dev_info(ts->dev, "%s: recovery\n", __func__);

	if (ts->state_pm == TS_DEV_PM_RESUME)
		pm_wakeup_event(ts->dev, 1000);

	lxs_hal_reset(ts, TC_HW_RESET_ASYNC, 0);
#endif
}

static int lxs_ts_init_works(struct lxs_ts *ts)
{
	ts->wq = create_singlethread_workqueue("touch_wq");
	if (!ts->wq) {
		t_dev_err(ts->dev, "failed to create workqueue\n");
		return -ENOMEM;
	}

	INIT_DELAYED_WORK(&ts->after_probe.start, lxs_ts_after_probe_func);
	INIT_DELAYED_WORK(&ts->init_work, lxs_ts_init_work_func);
	INIT_DELAYED_WORK(&ts->upgrade_work, lxs_ts_upgrade_work_func);
	INIT_DELAYED_WORK(&ts->sys_reset_work, lxs_ts_sys_reset_work_func);

	init_completion(&ts->resume_done);
	complete_all(&ts->resume_done);

	return 0;
}

static void lxs_ts_free_works(struct lxs_ts *ts)
{
	complete_all(&ts->resume_done);

	if (ts->wq) {
		destroy_workqueue(ts->wq);
		ts->wq = NULL;
	}
}

static int lxs_ts_parse_dts_pt(struct lxs_ts *ts, struct lxs_ts_plat_data *plat_data)
{
	struct device_node *np = ts->dev->of_node;
	u32 tmp[2] = { 0, };

	/*
	 * DTS example
	 * pt_open_cmp = <0 1000>
	 * pt_short_gnd_cmp = <0 2000>
	 * pt_short_trx_cmp = <0 3000>
	 */
	memset(plat_data->pt_open_cmp, 0, sizeof(plat_data->pt_open_cmp));
	if (of_property_read_u32_array(np, "pt_open_cmp", tmp, 2)) {
		t_dev_warn(ts->dev, "pt(dt): pt_open_cmp not found\n");
	} else {
		plat_data->pt_open_cmp[0] = tmp[0];
		plat_data->pt_open_cmp[1] = tmp[1];
		t_dev_info(ts->dev, "pt(dt): pt_open_cmp %d %d\n", tmp[0], tmp[1]);
	}

	memset(plat_data->pt_short_gnd_cmp, 0, sizeof(plat_data->pt_short_gnd_cmp));
	if (of_property_read_u32_array(np, "pt_short_gnd_cmp", tmp, 2)) {
		t_dev_warn(ts->dev, "pt(dt): pt_short_gnd_cmp not found\n");
	} else {
		plat_data->pt_short_gnd_cmp[0] = tmp[0];
		plat_data->pt_short_gnd_cmp[1] = tmp[1];
		t_dev_info(ts->dev, "pt(dt): pt_short_gnd_cmp %d %d\n", tmp[0], tmp[1]);
	}

	memset(plat_data->pt_short_trx_cmp, 0, sizeof(plat_data->pt_short_trx_cmp));
	if (of_property_read_u32_array(np, "pt_short_trx_cmp", tmp, 2)) {
		t_dev_warn(ts->dev, "pt(dt): pt_short_trx_cmp not found\n");
	} else {
		plat_data->pt_short_trx_cmp[0] = tmp[0];
		plat_data->pt_short_trx_cmp[1] = tmp[1];
		t_dev_info(ts->dev, "pt(dt): pt_short_trx_cmp %d %d\n", tmp[0], tmp[1]);
	}

	memset(plat_data->pt_short_vdd_cmp, 0, sizeof(plat_data->pt_short_vdd_cmp));
	if (of_property_read_u32_array(np, "pt_short_vdd_cmp", tmp, 2)) {
		t_dev_warn(ts->dev, "pt(dt): pt_short_vdd_cmp not found\n");
	} else {
		plat_data->pt_short_vdd_cmp[0] = tmp[0];
		plat_data->pt_short_vdd_cmp[1] = tmp[1];
		t_dev_info(ts->dev, "pt(dt): pt_short_vdd_cmp %d %d\n", tmp[0], tmp[1]);
	}

	memset(plat_data->pt_sync_v_cmp, 0, sizeof(plat_data->pt_sync_v_cmp));
	if (of_property_read_u32_array(np, "pt_sync_v_cmp", tmp, 2)) {
		t_dev_warn(ts->dev, "pt(dt): pt_sync_v_cmp not found\n");
	} else {
		plat_data->pt_sync_v_cmp[0] = tmp[0];
		plat_data->pt_sync_v_cmp[1] = tmp[1];
		t_dev_info(ts->dev, "pt(dt): pt_sync_v_cmp %d %d\n", tmp[0], tmp[1]);
	}

	return 0;
}

static int lxs_ts_parse_dts(struct lxs_ts *ts, struct lxs_ts_plat_data *plat_data)
{
	struct device_node *np = ts->dev->of_node;
	u32 tmp[3] = { 0, };
	u32 rejection_buff[4];
	u32 val;
	int pin_val = -1;

	t_dev_info(ts->dev, "DTS parsing\n");

	if (!of_property_read_u32(np, "use_skip_reset", &val))
		plat_data->use_skip_reset = val;

	if (ts->fquirks->gpio_set_reset != NULL) {
		t_dev_info(ts->dev, "reset pin ignored by gpio quirk\n");
		plat_data->use_skip_reset = 1;
	}

	if (plat_data->use_skip_reset) {
		plat_data->reset_pin = -1;
		t_dev_info(ts->dev, "of gpio  : reset-gpio ignored\n");
	} else {
		pin_val = of_get_named_gpio_flags(np, "reset-gpio", 0, NULL);
		if (!gpio_is_valid(pin_val)) {
			t_dev_err(ts->dev, "of gpio  : reset-gpio invalid, %d\n", pin_val);
			return -EINVAL;
		}
		t_dev_info(ts->dev, "of gpio  : reset-gpio %d\n", pin_val);
		plat_data->reset_pin = pin_val;
	}

	pin_val = of_get_named_gpio_flags(np, "irq-gpio", 0, NULL);
	if (!gpio_is_valid(pin_val)) {
		t_dev_err(ts->dev, "of gpio  : irq-gpio invalid, %d\n", pin_val);
		return -EINVAL;
	}
	t_dev_info(ts->dev, "of gpio  : irq-gpio %d\n", pin_val);
	plat_data->irq_pin = pin_val;

	if (!of_property_read_u32(np, "max_x", &val))
		plat_data->max_x = val;

	if (!of_property_read_u32(np, "max_y", &val))
		plat_data->max_y = val;

	t_dev_info(ts->dev, "coord(dt): max_x %d, max_y %d\n",
		plat_data->max_x, plat_data->max_y);

	if (of_property_read_u32_array(np, "area-size", tmp, 3)) {
		t_dev_info(ts->dev, "coord(dt): zone's size not found\n");
		plat_data->area_indicator = 48;
		plat_data->area_navigation = 96;
		plat_data->area_edge = 60;
	} else {
		plat_data->area_indicator = tmp[0];
		plat_data->area_navigation = tmp[1];
		plat_data->area_edge = tmp[2];
	}
	t_dev_info(ts->dev, "coord(dt): zone's size - indicator:%d, navigation:%d, edge:%d\n",
		plat_data->area_indicator, plat_data->area_navigation, plat_data->area_edge);

	if (!of_property_read_u32(np, "mt_tool_max", &val))
		plat_data->mt_tool_max = val;

#if defined(__LXS_SUPPORT_LPWG)
	if (!of_property_read_u32(np, "use_lpwg", &val))
		plat_data->use_lpwg = val;
#endif

	/* Firmware */
	plat_data->use_firmware = of_property_read_bool(np, "use_firmware");

	if (plat_data->use_firmware) {
		if (!of_property_read_u32(np, "use_fw_upgrade", &val))
			plat_data->use_fw_upgrade = val;

		if (!of_property_read_u32(np, "use_fw_pwr_rst", &val))
			plat_data->use_fw_pwr_rst = val;

		if (!of_property_read_u32(np, "use_fw_ver_diff", &val))
			plat_data->use_fw_ver_diff = val;

		if (!of_property_read_u32(np, "use_fw_skip_pid", &val))
			plat_data->use_fw_skip_pid = val;

		of_property_read_string_index(np, "fw_image", 0, &plat_data->fw_name);
	}

	if (!of_property_read_u32(np, "use_irq_verify", &val))
		plat_data->use_irq_verify = val;

	if (!of_property_read_u32(np, "use_input_parent", &val))
		plat_data->use_input_parent = val;

	if (!of_property_read_u32(np, "use_tc_vblank", &val))
		plat_data->use_tc_vblank = val;

	if (!of_property_read_u32(np, "use_tc_doze", &val))
		plat_data->use_tc_doze = val;

	if (!of_property_read_u32(np, "use_palm_opt", &val))
		plat_data->use_palm_opt = val;

	if (!of_property_read_u32(np, "use_charger_opt", &val))
		plat_data->use_charger_opt = val;

	if (!of_property_read_u32(np, "frame_tbl_type", &val))
		plat_data->frame_tbl_type = val;

	if (!of_property_read_string(np, "panel_spec", &plat_data->panel_spec_path))
		if (plat_data->panel_spec_path)
			t_dev_info(ts->dev, "of_string : panel_spec %s\n", plat_data->panel_spec_path);

	if (!of_property_read_string(np, "self_test", &plat_data->self_test_path))
		if (plat_data->self_test_path)
			t_dev_info(ts->dev, "of_string : self_test %s\n", plat_data->self_test_path);

	memset(ts->portrait_buffer, 0, sizeof(ts->portrait_buffer));
	if (of_property_read_u32_array(np, "lxs,rejection_area_portrait", rejection_buff, 4)) {
		t_dev_warn(ts->dev, "dts: lxs,rejection_area_portrait\n");
	} else {
		memcpy(ts->portrait_buffer, rejection_buff, sizeof(ts->portrait_buffer));
	}

	memset(ts->landscape_buffer, 0, sizeof(ts->landscape_buffer));
	if (of_property_read_u32_array(np, "lxs,rejection_area_landscape", rejection_buff, 4)) {
		t_dev_warn(ts->dev, "dts: lxs,rejection_area_landscape\n");
	} else {
		memcpy(ts->landscape_buffer, rejection_buff, sizeof(ts->landscape_buffer));
	}

	memset(ts->radius_portrait, 0, sizeof(ts->radius_portrait));
	if (of_property_read_u32_array(np, "lxs,rejection_area_portrait_ge", rejection_buff, 2)) {
		t_dev_warn(ts->dev, "dts: lxs,rejection_area_portrait_ge\n");
	} else {
		memcpy(ts->radius_portrait, rejection_buff, sizeof(ts->radius_portrait));
	}

	memset(ts->radius_landscape, 0, sizeof(ts->radius_landscape));
	if (of_property_read_u32_array(np, "lxs,rejection_area_landscape_ge", rejection_buff, 4)) {
		t_dev_warn(ts->dev, "dts: lxs,rejection_area_landscape_ge\n");
	} else {
		memcpy(ts->radius_landscape, rejection_buff, sizeof(ts->radius_landscape));
	}

	lxs_ts_parse_dts_pt(ts, plat_data);

	t_dev_info(ts->dev, "DTS parsing done\n");

	return 0;
}

static const int lxs_ts_d_ftbl_def_np[] = {
	TS_DISP_FRAME_ASYNC,
	TS_DISP_FRAME_60HZ,
	TS_DISP_FRAME_120HZ,
};

static const int lxs_ts_d_ftbl_def_lp[] = {
	TS_DISP_FRAME_ASYNC,
	TS_DISP_FRAME_30HZ,
};

static const int lxs_ts_t_ftbl_def_np[] = {
	TS_TOUCH_FRAME_60HZ,
	TS_TOUCH_FRAME_120HZ,
	TS_TOUCH_FRAME_240HZ_H,
	TS_TOUCH_FRAME_240HZ,
};

static const int lxs_ts_t_ftbl_def_lp[] = {
	TS_TOUCH_FRAME_L60HZ,
	TS_TOUCH_FRAME_L120HZ,
};

static void lxs_ts_setup_frame_tbl(struct lxs_ts *ts)
{
	ts->d_frame_rate_np = TS_DISP_FRAME_60HZ;
	ts->d_frame_rate_lp = TS_DISP_FRAME_ASYNC;
	ts->d_frame_rate_off = TS_DISP_FRAME_ASYNC;
	ts->d_frame_rate_aod = TS_DISP_FRAME_30HZ;

	ts->t_frame_rate_np = TS_TOUCH_FRAME_120HZ;
	ts->t_frame_rate_lp = TS_TOUCH_FRAME_L120HZ;

	memset(ts->d_frame_tbl_np, -1, sizeof(ts->d_frame_tbl_np));
	memset(ts->d_frame_tbl_lp, -1, sizeof(ts->d_frame_tbl_lp));
	memset(ts->t_frame_tbl_np, -1, sizeof(ts->t_frame_tbl_np));
	memset(ts->t_frame_tbl_lp, -1, sizeof(ts->t_frame_tbl_lp));

	memcpy(ts->d_frame_tbl_np, lxs_ts_d_ftbl_def_np, sizeof(lxs_ts_d_ftbl_def_np));
	memcpy(ts->d_frame_tbl_lp, lxs_ts_d_ftbl_def_lp, sizeof(lxs_ts_d_ftbl_def_lp));
	memcpy(ts->t_frame_tbl_np, lxs_ts_t_ftbl_def_np, sizeof(lxs_ts_t_ftbl_def_np));
	memcpy(ts->t_frame_tbl_lp, lxs_ts_t_ftbl_def_lp, sizeof(lxs_ts_t_ftbl_def_lp));

	switch (ts->plat_data->frame_tbl_type) {
	case 1:
		ts->d_frame_rate_np = TS_DISP_FRAME_ASYNC;
		ts->d_frame_rate_lp = TS_DISP_FRAME_ASYNC;
		ts->d_frame_rate_off = TS_DISP_FRAME_ASYNC;
		ts->d_frame_rate_aod = TS_DISP_FRAME_ASYNC;

		memset(ts->d_frame_tbl_np, -1, sizeof(ts->d_frame_tbl_np));
		memset(ts->d_frame_tbl_lp, -1, sizeof(ts->d_frame_tbl_lp));

		ts->d_frame_tbl_np[0] = TS_DISP_FRAME_ASYNC;
		ts->d_frame_tbl_lp[0] = TS_DISP_FRAME_ASYNC;
		break;
	}
}

static int lxs_ts_setup(struct lxs_ts *ts)
{
	struct lxs_ts_plat_data *plat_data = NULL;
	int ret = 0;

	ts->hw_reset_delay = 100;
	ts->sw_reset_delay = 90;

	ts->state_core = TS_CORE_PROBE;
	ts->state_pm = TS_DEV_PM_RESUME;
	ts->state_mode = TS_MODE_RESUME;
	ts->state_sleep = TS_IC_NORMAL;

	if (ts->dev->of_node) {
		plat_data = kzalloc(sizeof(struct lxs_ts_plat_data), GFP_KERNEL);
		if (plat_data == NULL)
			return -ENOMEM;

		ret = lxs_ts_parse_dts(ts, plat_data);
		if (ret) {
			t_dev_err(ts->dev, "DTS parsing failed\n");
			kfree(plat_data);
			return ret;
		}

		ts->dev->platform_data = plat_data;
	} else {
		plat_data = ts->dev->platform_data;
		if (plat_data == NULL) {
			t_dev_err(ts->dev, "No platform data found\n");
			return -ENOMEM;
		}
	}

	ts->plat_data = plat_data;

	t_dev_info(ts->dev, " max_x           = %d\n", plat_data->max_x);
	t_dev_info(ts->dev, " max_y           = %d\n", plat_data->max_y);
	if (plat_data->mt_tool_max)
		t_dev_info(ts->dev, " mt_tool_max     = 0x%X\n", plat_data->mt_tool_max);

	t_dev_info(ts->dev, " use_lpwg        = %d\n", plat_data->use_lpwg);
	t_dev_info(ts->dev, " use_firmware    = %d\n", plat_data->use_firmware);
	t_dev_info(ts->dev, " use_fw_upgrade  = %d\n", plat_data->use_fw_upgrade);

	if (plat_data->use_palm_opt)
		t_dev_info(ts->dev, " use_palm_opt    = 0x%X\n", plat_data->use_palm_opt);

	if (plat_data->use_charger_opt)
		t_dev_info(ts->dev, " use_charger_opt = 0x%X\n", plat_data->use_charger_opt);

	if (plat_data->use_fw_pwr_rst)
		t_dev_info(ts->dev, " use_fw_pwr_rst  = %d\n", plat_data->use_fw_pwr_rst);

	if (plat_data->use_fw_ver_diff)
		t_dev_info(ts->dev, " use_fw_ver_diff = %d\n", plat_data->use_fw_ver_diff);

	if (plat_data->use_fw_skip_pid)
		t_dev_info(ts->dev, " use_fw_skip_pid = %d\n", plat_data->use_fw_skip_pid);

	if (plat_data->frame_tbl_type)
		t_dev_info(ts->dev, " frame_tbl_type  = %d\n", plat_data->frame_tbl_type);

	lxs_ts_setup_frame_tbl(ts);

	return 0;
}

static void lxs_ts_unset(struct lxs_ts *ts)
{
	if (ts->dev->of_node && ts->plat_data) {
		kfree(ts->plat_data);
		ts->plat_data = NULL;
		ts->dev->platform_data = NULL;
	}
}

int lxs_ts_probe(struct lxs_ts *ts)
{
	int ret = 0;
	int i = 0;

	lxs_ts_params(ts);

	if (ts->fquirks->charger_mode) {
		/*
		 * if charger mode, set 'ts->is_charger = 1',
		 */
		ts->fquirks->charger_mode(ts);
	}

	t_dev_info(ts->dev, "LXS Touch Probe%s\n",
		(ts->is_charger) ? "(charger)" : "");

	ret = lxs_ts_setup(ts);
	if (ret) {
		t_dev_err(ts->dev, "failed to parse touch data, %d\n", ret);
		return ret;
	}

	if (ts->is_charger) {
		return 0;
	}

	ret = lxs_ts_init_works(ts);
	if (ret) {
		t_dev_err(ts->dev, "failed to init works, %d\n", ret);
		goto out;
	}

	ret = lxs_ts_init_gpios(ts);
	if (ret) {
		t_dev_err(ts->dev, "failed to init gpios, %d\n", ret);
		goto out_init_gpios;
	}

	ret = lxs_ts_init_input(ts);
	if (ret) {
		t_dev_err(ts->dev, "failed to register input device, %d\n", ret);
		goto out_init_input;
	}

	ret = lxs_ts_init_pm(ts);
	if (ret) {
		t_dev_err(ts->dev, "failed to init pm\n");
		goto out_init_pm;
	}

	ret = lxs_ts_init_sysfs(ts);
	if (ret) {
		t_dev_err(ts->dev, "failed to init sysfs, %d\n", ret);
		goto out_init_sysfs;
	}

	/* init notification callbacks */
	ts->drm_notif.notifier_call = drm_notifier_callback;

	ret = drm_register_client(&ts->drm_notif);
	if (ret) {
		t_dev_err(ts->dev, "Unable to register drm_notifier: %d\n", ret);
		goto out_err_notifier_register_client;
	}

	for (; i < 4; i++) {
		ts->circle_range_l[i] = ts->radius_landscape[i] * ts->radius_landscape[i];
		if (i < 2)
			ts->circle_range_p[i] = ts->radius_portrait[i] * ts->radius_portrait[i];
	}

	ts->report_rejected_event_flag = true;

	ts->probe_init = 1;

	return 0;

out_err_notifier_register_client:
	lxs_ts_free_sysfs(ts);

out_init_sysfs:
	lxs_ts_free_pm(ts);

out_init_pm:
	lxs_ts_free_input(ts);

out_init_input:
	lxs_ts_free_gpios(ts);

out_init_gpios:
	lxs_ts_free_works(ts);

out:
	lxs_ts_unset(ts);
	return ret;
}

void lxs_ts_remove(struct lxs_ts *ts, int is_shutdown)
{
	t_dev_info(ts->dev, "LXS Touch %s%s\n",
		(is_shutdown) ? "Shutdown" : "Remove",
		(ts->is_charger) ? "(charger)" : "");

	if (ts->is_charger)
		return;

	if (!ts->probe_init)
		return;

	mutex_lock(&ts->lock);
	ts->shutdown_called = true;
	mutex_unlock(&ts->lock);

	lxs_ts_watchdog_free(ts);

	lxs_ts_irq_control(ts, 0);

	cancel_delayed_work_sync(&ts->sys_reset_work);
	cancel_delayed_work_sync(&ts->upgrade_work);
	cancel_delayed_work_sync(&ts->init_work);
	cancel_delayed_work_sync(&ts->after_probe.start);

	if (ts->probe_done) {
		lxs_hal_deactivate(ts);
		lxs_ts_free_irq(ts);
		lxs_hal_remove(ts);
		ts->probe_done = 0;
	}

	drm_unregister_client(&ts->drm_notif);

	lxs_ts_free_sysfs(ts);

	lxs_ts_free_pm(ts);

	lxs_ts_free_input(ts);

	lxs_ts_free_gpios(ts);

	lxs_ts_free_works(ts);

	lxs_ts_unset(ts);

	ts->probe_init = 0;
}
