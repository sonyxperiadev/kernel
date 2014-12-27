/*
 * cyttsp4_mt_common.c
 * Cypress TrueTouch(TM) Standard Product V4 Multi-touch module.
 * For use with Cypress Txx4xx parts.
 * Supported parts include:
 * TMA4XX
 * TMA1036
 *
 * Copyright (C) 2012 Cypress Semiconductor
 * Copyright (C) 2011 Sony Ericsson Mobile Communications AB.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2, and only version 2, as published by the
 * Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Contact Cypress Semiconductor at www.cypress.com <ttdrivers@cypress.com>
 *
 */

#include "cyttsp4_mt_common.h"

#if defined(CONFIG_FB)
static int fb_notifier_callback(struct notifier_block *self,
				unsigned long event, void *data);
#endif

#if defined(CONFIG_PM_SLEEP) || defined(CONFIG_PM_RUNTIME)
static int cyttsp4_mt_suspend(struct device *dev);
static int cyttsp4_mt_resume(struct device *dev);
#endif


static void cyttsp4_lift_all(struct cyttsp4_mt_data *md)
{
	if (!md->si)
		return;

	if (md->num_prv_rec != 0) {
		if (md->mt_function.report_slot_liftoff)
			md->mt_function.report_slot_liftoff(md,
				md->si->si_ofs.tch_abs[CY_TCH_T].max);
		input_sync(md->input);
		md->num_prv_rec = 0;
	}
}

static void cyttsp4_mt_process_touch(struct cyttsp4_mt_data *md,
	struct cyttsp4_touch *touch)
{
	struct device *dev = &md->ttsp->dev;
	int tmp;
	bool flipped;

	if (md->pdata->flags & CY_MT_FLAG_FLIP) {
		tmp = touch->abs[CY_TCH_X];
		touch->abs[CY_TCH_X] = touch->abs[CY_TCH_Y];
		touch->abs[CY_TCH_Y] = tmp;
		flipped = true;
	} else
		flipped = false;

	if (md->pdata->flags & CY_MT_FLAG_INV_X) {
		if (flipped)
			touch->abs[CY_TCH_X] = md->si->si_ofs.max_y -
				touch->abs[CY_TCH_X];
		else
			touch->abs[CY_TCH_X] = md->si->si_ofs.max_x -
				touch->abs[CY_TCH_X];
	}
	if (md->pdata->flags & CY_MT_FLAG_INV_Y) {
		if (flipped)
			touch->abs[CY_TCH_Y] = md->si->si_ofs.max_x -
				touch->abs[CY_TCH_Y];
		else
			touch->abs[CY_TCH_Y] = md->si->si_ofs.max_y -
				touch->abs[CY_TCH_Y];
	}

	dev_vdbg(dev, "%s: flip=%s inv-x=%s inv-y=%s x=%04X(%d) y=%04X(%d)\n",
		__func__, flipped ? "true" : "false",
		md->pdata->flags & CY_MT_FLAG_INV_X ? "true" : "false",
		md->pdata->flags & CY_MT_FLAG_INV_Y ? "true" : "false",
		touch->abs[CY_TCH_X], touch->abs[CY_TCH_X],
		touch->abs[CY_TCH_Y], touch->abs[CY_TCH_Y]);
}

static void cyttsp4_get_mt_touches(struct cyttsp4_mt_data *md, int num_cur_rec)
{
	struct device *dev = &md->ttsp->dev;
	struct cyttsp4_sysinfo *si = md->si;
	struct cyttsp4_touch tch;
	int sig;
	int i, j, t = 0;
	int mt_sync_count = 0;
	int a; // no use
	DECLARE_BITMAP(ids, max(CY_TMA1036_MAX_TCH, CY_TMA4XX_MAX_TCH));

	bitmap_zero(ids, si->si_ofs.tch_abs[CY_TCH_T].max);

	for (i = 0; i < num_cur_rec; i++) {
		cyttsp4_get_touch_record(md->ttsp, i, tch.abs);

		/* Discard proximity event */
		if (tch.abs[CY_TCH_O] == CY_OBJ_PROXIMITY) {
			dev_dbg(dev, "%s: Discarding proximity event\n",
				__func__);
			continue;
		}

		if ((tch.abs[CY_TCH_T] < md->pdata->frmwrk->abs
			[(CY_ABS_ID_OST * CY_NUM_ABS_SET) + CY_MIN_OST]) ||
			(tch.abs[CY_TCH_T] > md->pdata->frmwrk->abs
			[(CY_ABS_ID_OST * CY_NUM_ABS_SET) + CY_MAX_OST])) {
			dev_err(dev, "%s: tch=%d -> bad trk_id=%d max_id=%d\n",
				__func__, i, tch.abs[CY_TCH_T],
				md->pdata->frmwrk->abs[(CY_ABS_ID_OST *
				CY_NUM_ABS_SET) + CY_MAX_OST]);
			if (md->mt_function.input_sync)
				md->mt_function.input_sync(md->input);
			mt_sync_count++;
			continue;
		}

		/* Process touch */
		cyttsp4_mt_process_touch(md, &tch);

		/* use 0 based track id's */
		sig = md->pdata->frmwrk->abs
			[(CY_ABS_ID_OST * CY_NUM_ABS_SET) + 0];
		if (sig != CY_IGNORE_VALUE) {
			t = tch.abs[CY_TCH_T] - md->pdata->frmwrk->abs
				[(CY_ABS_ID_OST * CY_NUM_ABS_SET) + CY_MIN_OST];
			if (tch.abs[CY_TCH_E] == CY_EV_LIFTOFF) {
				if(md->ttsp->core->dbg_msg_level & 0x2)
					dev_dbg(dev, "%s: t=%d e=%d lift-off\n",
						__func__, t, tch.abs[CY_TCH_E]);
				/* [Optical][Touch] Driver porting, 20130717, Add Start */
				if (IS_TTSP_VER_GE(si, 2, 3)) {
					if(md->ttsp->core->dbg_msg_level & 0x2) {
						dev_dbg(dev,
							"%s: t=%d x=%d y=%d z=%d M=%d m=%d o=%d e=%d\n",
							__func__, t,
							tch.abs[CY_TCH_X],
							tch.abs[CY_TCH_Y],
							tch.abs[CY_TCH_P],
							tch.abs[CY_TCH_MAJ],
							tch.abs[CY_TCH_MIN],
							tch.abs[CY_TCH_OR],
							tch.abs[CY_TCH_E]);
					}
				} else {
					if(md->ttsp->core->dbg_msg_level & 0x2)
						dev_dbg(dev,
							"%s: t=%d x=%d y=%d z=%d e=%d\n",
							__func__, t,
							tch.abs[CY_TCH_X],
							tch.abs[CY_TCH_Y],
							tch.abs[CY_TCH_P],
							tch.abs[CY_TCH_E]);
				}
				/* [Optical][Touch] Driver porting, 20130717, Add End */

				goto cyttsp4_get_mt_touches_pr_tch;
			}
			if (md->mt_function.input_report)
				md->mt_function.input_report(md->input, sig,
					t, tch.abs[CY_TCH_O]);
			__set_bit(t, ids);
		}

		/* all devices: position and pressure fields */
		for (j = 0; j <= CY_ABS_W_OST ; j++) {
			sig = md->pdata->frmwrk->abs[((CY_ABS_X_OST + j) *
				CY_NUM_ABS_SET) + 0];
			if (sig != CY_IGNORE_VALUE)
				input_report_abs(md->input, sig,
					tch.abs[CY_TCH_X + j]);
		}
		if (IS_TTSP_VER_GE(si, 2, 3)) {
			/*
			 * TMA400 size and orientation fields:
			 * if pressure is non-zero and major touch
			 * signal is zero, then set major and minor touch
			 * signals to minimum non-zero value
			 */
			if (tch.abs[CY_TCH_P] > 0 && tch.abs[CY_TCH_MAJ] == 0)
				tch.abs[CY_TCH_MAJ] = tch.abs[CY_TCH_MIN] = 1;

			/* Get the extended touch fields */
			for (j = 0; j < CY_NUM_EXT_TCH_FIELDS; j++) {
				sig = md->pdata->frmwrk->abs
					[((CY_ABS_MAJ_OST + j) *
					CY_NUM_ABS_SET) + 0];
				if (sig != CY_IGNORE_VALUE)
					input_report_abs(md->input, sig,
						tch.abs[CY_TCH_MAJ + j]);
			}
		}
		if (md->mt_function.input_sync)
			md->mt_function.input_sync(md->input);
		mt_sync_count++;

/* [Optical][Touch] Driver porting, 20130717, Del Start */
cyttsp4_get_mt_touches_pr_tch:
		a = 0;
#if 0
		if (IS_TTSP_VER_GE(si, 2, 3))
			dev_dbg(dev,
				"%s: t=%d x=%d y=%d z=%d M=%d m=%d o=%d e=%d\n",
				__func__, t,
				tch.abs[CY_TCH_X],
				tch.abs[CY_TCH_Y],
				tch.abs[CY_TCH_P],
				tch.abs[CY_TCH_MAJ],
				tch.abs[CY_TCH_MIN],
				tch.abs[CY_TCH_OR],
				tch.abs[CY_TCH_E]);
		else
			dev_dbg(dev,
				"%s: t=%d x=%d y=%d z=%d e=%d\n", __func__,
				t,
				tch.abs[CY_TCH_X],
				tch.abs[CY_TCH_Y],
				tch.abs[CY_TCH_P],
				tch.abs[CY_TCH_E]);
#endif
/* [Optical][Touch] Driver porting, 20130717, Del End*/
	}

	if (md->mt_function.final_sync)
		md->mt_function.final_sync(md->input,
			si->si_ofs.tch_abs[CY_TCH_T].max, mt_sync_count, ids);

	md->num_prv_rec = num_cur_rec;
	md->prv_tch_type = tch.abs[CY_TCH_O];

	return;
}

/* read xy_data for all current touches */
static int cyttsp4_xy_worker(struct cyttsp4_mt_data *md)
{
	struct device *dev = &md->ttsp->dev;
	struct cyttsp4_sysinfo *si = md->si;
	u8 num_cur_rec;
	u8 rep_len;
	u8 rep_stat;
	u8 tt_stat;
	int rc = 0;

	/*
	 * Get event data from cyttsp4 device.
	 * The event data includes all data
	 * for all active touches.
	 * Event data also includes button data
	 */
	rep_len = si->xy_mode[si->si_ofs.rep_ofs];
	rep_stat = si->xy_mode[si->si_ofs.rep_ofs + 1];
	tt_stat = si->xy_mode[si->si_ofs.tt_stat_ofs];

	num_cur_rec = GET_NUM_TOUCH_RECORDS(tt_stat);

	if (rep_len == 0 && num_cur_rec > 0) {
		dev_err(dev, "%s: report length error rep_len=%d num_tch=%d\n",
			__func__, rep_len, num_cur_rec);
		goto cyttsp4_xy_worker_exit;
	}

	/* check any error conditions */
	if (IS_BAD_PKT(rep_stat)) {
		dev_dbg(dev, "%s: Invalid buffer detected\n", __func__);
		rc = 0;
		goto cyttsp4_xy_worker_exit;
	}

	if (IS_LARGE_AREA(tt_stat)) {
		dev_dbg(dev, "%s: Large area detected\n", __func__);
		/* Do not report touch if configured so */
		if (md->pdata->flags & CY_MT_FLAG_NO_TOUCH_ON_LO)
			num_cur_rec = 0;
	}

	if (num_cur_rec > si->si_ofs.max_tchs) {
		dev_err(dev, "%s: %s (n=%d c=%d)\n", __func__,
			"too many tch; set to max tch",
			num_cur_rec, si->si_ofs.max_tchs);
		num_cur_rec = si->si_ofs.max_tchs;
	}

	/* extract xy_data for all currently reported touches */
	dev_vdbg(dev, "%s: extract data num_cur_rec=%d\n", __func__,
		num_cur_rec);
	if (num_cur_rec)
		cyttsp4_get_mt_touches(md, num_cur_rec);
	else
		cyttsp4_lift_all(md);

	dev_vdbg(dev, "%s: done\n", __func__);
	rc = 0;

cyttsp4_xy_worker_exit:
	return rc;
}

static void cyttsp4_mt_send_dummy_event(struct cyttsp4_mt_data *md)
{
	unsigned long ids = 0;

	/* for easy wakeup */
	if (md->mt_function.input_report)
		md->mt_function.input_report(md->input, ABS_MT_TRACKING_ID,
			0, CY_OBJ_STANDARD_FINGER);
	if (md->mt_function.input_sync)
		md->mt_function.input_sync(md->input);
	if (md->mt_function.final_sync)
		md->mt_function.final_sync(md->input, 0, 1, &ids);
	if (md->mt_function.report_slot_liftoff)
		md->mt_function.report_slot_liftoff(md, 1);
	if (md->mt_function.final_sync)
		md->mt_function.final_sync(md->input, 1, 1, &ids);
}

static int cyttsp4_mt_attention(struct cyttsp4_device *ttsp)
{
	struct device *dev = &ttsp->dev;
	struct cyttsp4_mt_data *md = dev_get_drvdata(dev);
	int rc = 0;

	dev_vdbg(dev, "%s\n", __func__);

	mutex_lock(&md->report_lock);
	if (!md->is_suspended) {
		/**/
		if(ttsp->core->enable_touch)
			/* core handles handshake */
			rc = cyttsp4_xy_worker(md);
		/**/
	}
	mutex_unlock(&md->report_lock);
	if (rc < 0)
		dev_err(dev, "%s: xy_worker error r=%d\n", __func__, rc);

	return rc;
}

static int cyttsp4_mt_wake_attention(struct cyttsp4_device *ttsp)
{
	struct cyttsp4_mt_data *md = dev_get_drvdata(&ttsp->dev);

	mutex_lock(&md->report_lock);
	cyttsp4_mt_send_dummy_event(md);
	mutex_unlock(&md->report_lock);
	return 0;
}

static int cyttsp4_startup_attention(struct cyttsp4_device *ttsp)
{
	struct device *dev = &ttsp->dev;
	struct cyttsp4_mt_data *md = dev_get_drvdata(dev);
	int rc = 0;

	dev_vdbg(dev, "%s\n", __func__);

	mutex_lock(&md->report_lock);
	cyttsp4_lift_all(md);
	mutex_unlock(&md->report_lock);
	return rc;
}

static int cyttsp4_mt_open(struct input_dev *input)
{
	struct device *dev = input->dev.parent;
	struct cyttsp4_device *ttsp =
		container_of(dev, struct cyttsp4_device, dev);

	if(ttsp->core->dbg_msg_level & 0x2)
		dev_dbg(dev, "%s\n", __func__);

	pm_runtime_get(dev);

	dev_vdbg(dev, "%s: setup subscriptions\n", __func__);

	/* set up touch call back */
	cyttsp4_subscribe_attention(ttsp, CY_ATTEN_IRQ,
		cyttsp4_mt_attention, CY_MODE_OPERATIONAL);

	/* set up startup call back */
	cyttsp4_subscribe_attention(ttsp, CY_ATTEN_STARTUP,
		cyttsp4_startup_attention, 0);

	/* set up wakeup call back */
	cyttsp4_subscribe_attention(ttsp, CY_ATTEN_WAKE,
		cyttsp4_mt_wake_attention, 0);

	return 0;
}

static void cyttsp4_mt_close(struct input_dev *input)
{
	struct device *dev = input->dev.parent;
	struct cyttsp4_device *ttsp =
		container_of(dev, struct cyttsp4_device, dev);

	if(ttsp->core->dbg_msg_level & 0x2)
		dev_dbg(dev, "%s\n", __func__);

	cyttsp4_unsubscribe_attention(ttsp, CY_ATTEN_IRQ,
		cyttsp4_mt_attention, CY_MODE_OPERATIONAL);

	cyttsp4_unsubscribe_attention(ttsp, CY_ATTEN_STARTUP,
		cyttsp4_startup_attention, 0);

	cyttsp4_unsubscribe_attention(ttsp, CY_ATTEN_WAKE,
		cyttsp4_mt_wake_attention, 0);

	pm_runtime_put(dev);
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void cyttsp4_mt_early_suspend(struct early_suspend *h)
{
	struct cyttsp4_mt_data *md =
		container_of(h, struct cyttsp4_mt_data, es);
	struct device *dev = &md->ttsp->dev;

	dev_dbg(dev, "%s\n", __func__);

#ifndef CONFIG_PM_RUNTIME
	mutex_lock(&md->report_lock);
	md->is_suspended = true;
	cyttsp4_lift_all(md);
	mutex_unlock(&md->report_lock);
#endif

	pm_runtime_put(dev);
}

static void cyttsp4_mt_late_resume(struct early_suspend *h)
{
	struct cyttsp4_mt_data *md =
		container_of(h, struct cyttsp4_mt_data, es);
	struct device *dev = &md->ttsp->dev;

	dev_dbg(dev, "%s\n", __func__);

#ifndef CONFIG_PM_RUNTIME
	mutex_lock(&md->report_lock);
	md->is_suspended = false;
	mutex_unlock(&md->report_lock);
#endif

	pm_runtime_get(dev);
}

static void cyttsp4_setup_early_suspend(struct cyttsp4_mt_data *md)
{
	md->es.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	md->es.suspend = cyttsp4_mt_early_suspend;
	md->es.resume = cyttsp4_mt_late_resume;

	register_early_suspend(&md->es);
}

#elif CONFIG_FB
static int fb_notifier_callback(struct notifier_block *self,
				unsigned long event, void *data)
{
	struct fb_event *evdata = data;
	int *blank;
	struct cyttsp4_mt_data *md =
		container_of(self, struct cyttsp4_mt_data, fb_notif);
	struct device *dev = &md->ttsp->dev;

    	dev_dbg(dev, "%s\n", __func__);

	if (evdata && evdata->data && event == FB_EVENT_BLANK &&
		md && md->ttsp) {
		blank = evdata->data;
		if (*blank == FB_BLANK_UNBLANK && md->is_suspended == true)
			cyttsp4_mt_resume(&(md->ttsp->dev));
		else if (*blank == FB_BLANK_POWERDOWN)
			cyttsp4_mt_suspend(&(md->ttsp->dev));
	}

	return 0;
}
static void cyttsp4_setup_early_suspend(struct cyttsp4_mt_data *md)
{
	int retval = 0;
	struct device *dev = &md->ttsp->dev;

    	dev_dbg(dev, "%s\n", __func__);
	md->fb_notif.notifier_call = fb_notifier_callback;

	retval = fb_register_client(&md->fb_notif);
	if (retval)
		dev_err(&md->ttsp->dev,
			"Unable to register fb_notifier: %d\n", retval);
	return;
}

#else

static void cyttsp4_setup_early_suspend(struct cyttsp4_mt_data *md)
{
	return;
}

#endif

#if defined(CONFIG_PM_SLEEP) || defined(CONFIG_PM_RUNTIME)
static int cyttsp4_mt_suspend(struct device *dev)
{
//#ifndef CONFIG_PM_RUNTIME
	struct cyttsp4_mt_data *md = dev_get_drvdata(dev);
//#endif

	dev_dbg(dev, "%s\n", __func__);

//#ifndef CONFIG_PM_RUNTIME
	mutex_lock(&md->report_lock);
	md->is_suspended = true;
	cyttsp4_lift_all(md);
	mutex_unlock(&md->report_lock);
//#endif

	cyttsp4_core_early_suspend();
//	pm_runtime_put(dev);
	return 0;
}

static int cyttsp4_mt_rt_suspend(struct device *dev)
{
	struct cyttsp4_mt_data *md = dev_get_drvdata(dev);

	dev_dbg(dev, "%s\n", __func__);

	mutex_lock(&md->report_lock);
	md->is_suspended = true;
	cyttsp4_lift_all(md);
	mutex_unlock(&md->report_lock);

	return 0;
}

static int cyttsp4_mt_rt_resume(struct device *dev)
{
	struct cyttsp4_mt_data *md = dev_get_drvdata(dev);

	if(md->ttsp->core->dbg_msg_level & 0x4)
		dev_dbg(dev, "%s\n", __func__);

	mutex_lock(&md->report_lock);
	md->is_suspended = false;
	mutex_unlock(&md->report_lock);

	return 0;
}

static int cyttsp4_mt_resume(struct device *dev)
{
/**/
	struct cyttsp4_mt_data *md = dev_get_drvdata(dev);

	if(md->ttsp->core->dbg_msg_level & 0x4)
		dev_dbg(dev, "%s\n", __func__);

	mutex_lock(&md->report_lock);
	md->is_suspended = false;
	mutex_unlock(&md->report_lock);

	cyttsp4_core_lately_resume();
//	pm_runtime_get(dev);
/**/
	return 0;
}
#endif

#if 0
static const struct dev_pm_ops cyttsp4_mt_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(cyttsp4_mt_suspend, cyttsp4_mt_resume)
	SET_RUNTIME_PM_OPS(cyttsp4_mt_suspend, cyttsp4_mt_rt_resume, NULL)
};
#endif

#if (!defined(CONFIG_FB) && !defined(CONFIG_HAS_EARLYSUSPEND))
static const struct dev_pm_ops cyttsp4_mt_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(cyttsp4_mt_suspend, cyttsp4_mt_resume)
};
#else
static const struct dev_pm_ops cyttsp4_mt_pm_ops = {
	SET_RUNTIME_PM_OPS(cyttsp4_mt_rt_suspend, cyttsp4_mt_rt_resume, NULL)
};
#endif

static int cyttsp4_setup_input_device(struct cyttsp4_device *ttsp)
{
	struct device *dev = &ttsp->dev;
	struct cyttsp4_mt_data *md = dev_get_drvdata(dev);
	int signal = CY_IGNORE_VALUE;
	int max_x, max_y, max_p, min, max;
	int max_x_tmp, max_y_tmp;
	int i;
	int rc;

	dev_vdbg(dev, "%s: Initialize event signals\n", __func__);
	__set_bit(EV_ABS, md->input->evbit);
	__set_bit(EV_REL, md->input->evbit);
	__set_bit(EV_KEY, md->input->evbit);
#ifdef INPUT_PROP_DIRECT
	__set_bit(INPUT_PROP_DIRECT, md->input->propbit);
#endif

	/* If virtualkeys enabled, don't use all screen */
	if (md->pdata->flags & CY_MT_FLAG_VKEYS) {
		max_x_tmp = md->pdata->vkeys_x;
		max_y_tmp = md->pdata->vkeys_y;
	} else {
		max_x_tmp = md->si->si_ofs.max_x;
		max_y_tmp = md->si->si_ofs.max_y;
	}

	/* get maximum values from the sysinfo data */
	if (md->pdata->flags & CY_MT_FLAG_FLIP) {
		max_x = max_y_tmp - 1;
		max_y = max_x_tmp - 1;
	} else {
		max_x = max_x_tmp - 1;
		max_y = max_y_tmp - 1;
	}
	max_p = md->si->si_ofs.max_p;

	/* set event signal capabilities */
	for (i = 0; i < (md->pdata->frmwrk->size / CY_NUM_ABS_SET); i++) {
		signal = md->pdata->frmwrk->abs
			[(i * CY_NUM_ABS_SET) + CY_SIGNAL_OST];
		if (signal != CY_IGNORE_VALUE) {
			__set_bit(signal, md->input->absbit);
			min = md->pdata->frmwrk->abs
				[(i * CY_NUM_ABS_SET) + CY_MIN_OST];
			max = md->pdata->frmwrk->abs
				[(i * CY_NUM_ABS_SET) + CY_MAX_OST];
			if (i == CY_ABS_ID_OST) {
				/* shift track ids down to start at 0 */
				max = max - min;
				min = min - min;
			} else if (i == CY_ABS_X_OST)
				max = max_x;
			else if (i == CY_ABS_Y_OST)
				max = max_y;
			else if (i == CY_ABS_P_OST)
				max = max_p;
			input_set_abs_params(md->input, signal, min, max,
				md->pdata->frmwrk->abs
				[(i * CY_NUM_ABS_SET) + CY_FUZZ_OST],
				md->pdata->frmwrk->abs
				[(i * CY_NUM_ABS_SET) + CY_FLAT_OST]);
//			dev_dbg(dev, "%s: register signal=%02X min=%d max=%d\n",
//				__func__, signal, min, max);
			if (i == CY_ABS_ID_OST && !IS_TTSP_VER_GE(md->si, 2, 3))
				break;
		}
	}

	rc = md->mt_function.input_register_device(md->input,
			md->si->si_ofs.tch_abs[CY_TCH_T].max);
	if (rc < 0)
		dev_err(dev, "%s: Error, failed register input device r=%d\n",
			__func__, rc);
	else
		md->input_device_registered = true;

	return rc;
}

static int cyttsp4_setup_input_attention(struct cyttsp4_device *ttsp)
{
	struct device *dev = &ttsp->dev;
	struct cyttsp4_mt_data *md = dev_get_drvdata(dev);
	int rc = 0;

	dev_vdbg(dev, "%s\n", __func__);

	md->si = cyttsp4_request_sysinfo(ttsp);
	if (!md->si)
		return -EINVAL;

	rc = cyttsp4_setup_input_device(ttsp);

	cyttsp4_unsubscribe_attention(ttsp, CY_ATTEN_STARTUP,
		cyttsp4_setup_input_attention, 0);

	return rc;
}

static int cyttsp4_mt_release(struct cyttsp4_device *ttsp)
{
	struct device *dev = &ttsp->dev;
	struct cyttsp4_mt_data *md = dev_get_drvdata(dev);

	dev_dbg(dev, "%s\n", __func__);

#ifdef CONFIG_HAS_EARLYSUSPEND
	/*
	 * This check is to prevent pm_runtime usage_count drop below zero
	 * because of removing the module while in suspended state
	 */
	if (md->is_suspended)
		pm_runtime_get_noresume(dev);

	unregister_early_suspend(&md->es);
#endif

	if (md->input_device_registered) {
		input_unregister_device(md->input);
	} else {
		input_free_device(md->input);
		cyttsp4_unsubscribe_attention(ttsp, CY_ATTEN_STARTUP,
			cyttsp4_setup_input_attention, 0);
	}

	pm_runtime_suspend(dev);
	pm_runtime_disable(dev);

	dev_set_drvdata(dev, NULL);
	kfree(md);
	return 0;
}

static int cyttsp4_mt_probe(struct cyttsp4_device *ttsp)
{
	struct device *dev = &ttsp->dev;
	struct cyttsp4_mt_data *md;
	struct cyttsp4_mt_platform_data *pdata = dev_get_platdata(dev);
	int rc = 0;

//	dev_info(dev, "%s\n", __func__);
//	dev_dbg(dev, "%s: debug on\n", __func__);
	dev_vdbg(dev, "%s: verbose debug on\n", __func__);

	if (pdata == NULL) {
		dev_err(dev, "%s: Missing platform data\n", __func__);
		rc = -ENODEV;
		goto error_no_pdata;
	}

	md = kzalloc(sizeof(*md), GFP_KERNEL);
	if (md == NULL) {
		dev_err(dev, "%s: Error, kzalloc\n", __func__);
		rc = -ENOMEM;
		goto error_alloc_data_failed;
	}

	cyttsp4_init_function_ptrs(md);

	mutex_init(&md->report_lock);
	md->prv_tch_type = CY_OBJ_STANDARD_FINGER;
	md->ttsp = ttsp;
	md->pdata = pdata;
	dev_set_drvdata(dev, md);
	/* Create the input device and register it. */
	dev_vdbg(dev, "%s: Create the input device and register it\n",
		__func__);
	md->input = input_allocate_device();
	if (md->input == NULL) {
		dev_err(dev, "%s: Error, failed to allocate input device\n",
			__func__);
		rc = -ENOSYS;
		goto error_alloc_failed;
	}

	md->input->name = ttsp->name;
	scnprintf(md->phys, sizeof(md->phys)-1, "%s", dev_name(dev));
	md->input->phys = md->phys;
	md->input->dev.parent = &md->ttsp->dev;
	md->input->open = cyttsp4_mt_open;
	md->input->close = cyttsp4_mt_close;
	input_set_drvdata(md->input, md);

	pm_runtime_enable(dev);

	/* get sysinfo */
	md->si = cyttsp4_request_sysinfo(ttsp);
	if (md->si) {
		rc = cyttsp4_setup_input_device(ttsp);
		if (rc)
			goto error_init_input;
	} else {
		dev_err(dev, "%s: Fail get sysinfo pointer from core p=%p\n",
			__func__, md->si);
		cyttsp4_subscribe_attention(ttsp, CY_ATTEN_STARTUP,
			cyttsp4_setup_input_attention, 0);
	}

//#ifdef CONFIG_HAS_EARLYSUSPEND
	cyttsp4_setup_early_suspend(md);
//#endif

	dev_dbg(dev, "%s: OK\n", __func__);
	return 0;

error_init_input:
	pm_runtime_suspend(dev);
	pm_runtime_disable(dev);
	input_free_device(md->input);
error_alloc_failed:
	dev_set_drvdata(dev, NULL);
	kfree(md);
error_alloc_data_failed:
error_no_pdata:
	dev_err(dev, "%s failed.\n", __func__);
	return rc;
}

struct cyttsp4_driver cyttsp4_mt_driver = {
	.probe = cyttsp4_mt_probe,
	.remove = cyttsp4_mt_release,
	.driver = {
		.name = CYTTSP4_MT_NAME,
		.bus = &cyttsp4_bus_type,
		.pm = &cyttsp4_mt_pm_ops,
	},
};

