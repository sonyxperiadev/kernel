/* Copyright (c) 2016-2018, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#define pr_fmt(fmt)	"[sde_rsc:%s:%d]: " fmt, __func__, __LINE__

#include <linux/kernel.h>
#include <linux/debugfs.h>
#include <linux/of.h>
#include <linux/string.h>
#include <linux/of_address.h>
#include <linux/component.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/of_platform.h>
#include <linux/module.h>

#include <soc/qcom/rpmh.h>
#include <drm/drmP.h>
#include <drm/drm_irq.h>
#include "sde_rsc_priv.h"
#include "sde_dbg.h"

#define SDE_RSC_DRV_DBG_NAME		"sde_rsc_drv"
#define SDE_RSC_WRAPPER_DBG_NAME	"sde_rsc_wrapper"

/* worst case time to execute the one tcs vote(sleep/wake) - ~1ms */
#define SINGLE_TCS_EXECUTION_TIME				1064000

/* this time is ~1ms - only wake tcs in any mode */
#define RSC_BACKOFF_TIME_NS		 (SINGLE_TCS_EXECUTION_TIME + 100)

/**
 * this time is ~1ms - only wake TCS in mode-0.
 * This time must be greater than backoff time.
 */
#define RSC_MODE_THRESHOLD_TIME_IN_NS	(RSC_BACKOFF_TIME_NS + 2700)

/* this time is ~2ms - sleep+ wake TCS in mode-1 */
#define RSC_TIME_SLOT_0_NS		((SINGLE_TCS_EXECUTION_TIME * 2) + 100)

#define DEFAULT_PANEL_FPS		60
#define DEFAULT_PANEL_JITTER_NUMERATOR	2
#define DEFAULT_PANEL_JITTER_DENOMINATOR 1
#define DEFAULT_PANEL_PREFILL_LINES	25
#define DEFAULT_PANEL_VTOTAL		(480 + DEFAULT_PANEL_PREFILL_LINES)
#define TICKS_IN_NANO_SECOND		1000000000

#define MAX_BUFFER_SIZE 256

#define TRY_CMD_MODE_SWITCH		0xFFFF
#define TRY_CLK_MODE_SWITCH		0xFFFE
#define STATE_UPDATE_NOT_ALLOWED	0xFFFD

/* Primary panel worst case VSYNC expected to be no less than 30fps */
#define PRIMARY_VBLANK_WORST_CASE_MS 34

static struct sde_rsc_priv *rsc_prv_list[MAX_RSC_COUNT];

/**
 * sde_rsc_client_create() - create the client for sde rsc.
 * Different displays like DSI, HDMI, DP, WB, etc should call this
 * api to register their vote for rpmh. They still need to vote for
 * power handle to get the clocks.

 * @rsc_index:   A client will be created on this RSC. As of now only
 *               SDE_RSC_INDEX is valid rsc index.
 * @name:	 Caller needs to provide some valid string to identify
 *               the client. "primary", "dp", "hdmi" are suggested name.
 * @is_primary:	 Caller needs to provide information if client is primary
 *               or not. Primary client votes will be redirected to
 *               display rsc.
 *
 * Return: client node pointer.
 */
struct sde_rsc_client *sde_rsc_client_create(u32 rsc_index, char *client_name,
	bool is_primary_client)
{
	struct sde_rsc_client *client;
	struct sde_rsc_priv *rsc;
	static int id;

	if (!client_name) {
		pr_err("client name is null- not supported\n");
		return ERR_PTR(-EINVAL);
	} else if (rsc_index >= MAX_RSC_COUNT) {
		pr_err("invalid rsc index\n");
		return ERR_PTR(-EINVAL);
	} else if (!rsc_prv_list[rsc_index]) {
		pr_err("rsc not probed yet or not available\n");
		return NULL;
	}

	rsc = rsc_prv_list[rsc_index];
	client = kzalloc(sizeof(struct sde_rsc_client), GFP_KERNEL);
	if (!client)
		return ERR_PTR(-ENOMEM);

	mutex_lock(&rsc->client_lock);
	strlcpy(client->name, client_name, MAX_RSC_CLIENT_NAME_LEN);
	client->current_state = SDE_RSC_IDLE_STATE;
	client->rsc_index = rsc_index;
	client->id = id;
	if (is_primary_client)
		rsc->primary_client = client;
	pr_debug("client %s rsc index:%d primary:%d\n", client_name,
						rsc_index, is_primary_client);

	list_add(&client->list, &rsc->client_list);
	id++;
	mutex_unlock(&rsc->client_lock);

	return client;
}
EXPORT_SYMBOL(sde_rsc_client_create);

/**
 * sde_rsc_client_destroy() - Destroy the sde rsc client.
 *
 * @client:	 Client pointer provided by sde_rsc_client_create().
 *
 * Return: none
 */
void sde_rsc_client_destroy(struct sde_rsc_client *client)
{
	struct sde_rsc_priv *rsc;
	enum sde_rsc_state state;

	if (!client) {
		pr_debug("invalid client\n");
		goto end;
	} else if (client->rsc_index >= MAX_RSC_COUNT) {
		pr_err("invalid rsc index\n");
		goto end;
	}

	pr_debug("client %s destroyed\n", client->name);
	rsc = rsc_prv_list[client->rsc_index];
	if (!rsc)
		goto end;

	mutex_lock(&rsc->client_lock);
	state = client->current_state;
	mutex_unlock(&rsc->client_lock);

	if (state != SDE_RSC_IDLE_STATE) {
		int wait_vblank_crtc_id;

		sde_rsc_client_state_update(client, SDE_RSC_IDLE_STATE, NULL,
				SDE_RSC_INVALID_CRTC_ID, &wait_vblank_crtc_id);

		/* if vblank wait required at shutdown, use a simple sleep */
		if (wait_vblank_crtc_id != SDE_RSC_INVALID_CRTC_ID) {
			pr_err("unexpected sleep required on crtc %d at rsc client destroy\n",
					wait_vblank_crtc_id);
			SDE_EVT32(client->id, state, rsc->current_state,
					client->crtc_id, wait_vblank_crtc_id,
					SDE_EVTLOG_ERROR);
			msleep(PRIMARY_VBLANK_WORST_CASE_MS);
		}
	}
	mutex_lock(&rsc->client_lock);
	list_del_init(&client->list);
	mutex_unlock(&rsc->client_lock);

	kfree(client);
end:
	return;
}
EXPORT_SYMBOL(sde_rsc_client_destroy);

struct sde_rsc_event *sde_rsc_register_event(int rsc_index, uint32_t event_type,
		void (*cb_func)(uint32_t event_type, void *usr), void *usr)
{
	struct sde_rsc_event *evt;
	struct sde_rsc_priv *rsc;

	if (rsc_index >= MAX_RSC_COUNT) {
		pr_err("invalid rsc index:%d\n", rsc_index);
		return ERR_PTR(-EINVAL);
	} else if (!rsc_prv_list[rsc_index]) {
		pr_err("rsc idx:%d not probed yet or not available\n",
								rsc_index);
		return ERR_PTR(-EINVAL);
	} else if (!cb_func || !event_type) {
		pr_err("no event or cb func\n");
		return ERR_PTR(-EINVAL);
	}

	rsc = rsc_prv_list[rsc_index];
	evt = kzalloc(sizeof(struct sde_rsc_event), GFP_KERNEL);
	if (!evt)
		return ERR_PTR(-ENOMEM);

	evt->event_type = event_type;
	evt->rsc_index = rsc_index;
	evt->usr = usr;
	evt->cb_func = cb_func;
	pr_debug("event register type:%d rsc index:%d\n",
						event_type, rsc_index);

	mutex_lock(&rsc->client_lock);
	list_add(&evt->list, &rsc->event_list);
	mutex_unlock(&rsc->client_lock);

	return evt;
}
EXPORT_SYMBOL(sde_rsc_register_event);

void sde_rsc_unregister_event(struct sde_rsc_event *event)
{
	struct sde_rsc_priv *rsc;

	if (!event) {
		pr_debug("invalid event client\n");
		goto end;
	} else if (event->rsc_index >= MAX_RSC_COUNT) {
		pr_err("invalid rsc index\n");
		goto end;
	}

	pr_debug("event client destroyed\n");
	rsc = rsc_prv_list[event->rsc_index];
	if (!rsc)
		goto end;

	mutex_lock(&rsc->client_lock);
	list_del_init(&event->list);
	mutex_unlock(&rsc->client_lock);

	kfree(event);
end:
	return;
}
EXPORT_SYMBOL(sde_rsc_unregister_event);

bool is_sde_rsc_available(int rsc_index)
{
	if (rsc_index >= MAX_RSC_COUNT) {
		pr_err("invalid rsc index:%d\n", rsc_index);
		return false;
	} else if (!rsc_prv_list[rsc_index]) {
		pr_err("rsc idx:%d not probed yet or not available\n",
								rsc_index);
		return false;
	}

	return true;
}
EXPORT_SYMBOL(is_sde_rsc_available);

enum sde_rsc_state get_sde_rsc_current_state(int rsc_index)
{
	struct sde_rsc_priv *rsc;

	if (rsc_index >= MAX_RSC_COUNT) {
		pr_err("invalid rsc index:%d\n", rsc_index);
		return SDE_RSC_IDLE_STATE;
	} else if (!rsc_prv_list[rsc_index]) {
		pr_err("rsc idx:%d not probed yet or not available\n",
								rsc_index);
		return SDE_RSC_IDLE_STATE;
	}

	rsc = rsc_prv_list[rsc_index];
	return rsc->current_state;
}
EXPORT_SYMBOL(get_sde_rsc_current_state);

static int sde_rsc_clk_enable(struct sde_power_handle *phandle,
	struct sde_power_client *pclient, bool enable)
{
	int rc = 0;
	struct dss_module_power *mp;

	if (!phandle || !pclient) {
		pr_err("invalid input argument\n");
		return -EINVAL;
	}

	mp = &phandle->mp;

	if (enable)
		pclient->refcount++;
	else if (pclient->refcount)
		pclient->refcount--;

	if (pclient->refcount)
		pclient->usecase_ndx = VOTE_INDEX_LOW;
	else
		pclient->usecase_ndx = VOTE_INDEX_DISABLE;

	if (phandle->current_usecase_ndx == pclient->usecase_ndx)
		goto end;

	if (enable) {
		rc = msm_dss_enable_clk(mp->clk_config, mp->num_clk, enable);
		if (rc) {
			pr_err("clock enable failed rc:%d\n", rc);
			goto end;
		}
	} else {
		msm_dss_enable_clk(mp->clk_config, mp->num_clk, enable);
	}

	phandle->current_usecase_ndx = pclient->usecase_ndx;

end:
	return rc;
}

static u32 sde_rsc_timer_calculate(struct sde_rsc_priv *rsc,
	struct sde_rsc_cmd_config *cmd_config)
{
	const u32 cxo_period_ns = 52;
	u64 rsc_backoff_time_ns = RSC_BACKOFF_TIME_NS;
	u64 rsc_mode_threshold_time_ns = RSC_MODE_THRESHOLD_TIME_IN_NS;
	u64 rsc_time_slot_0_ns = RSC_TIME_SLOT_0_NS;
	u64 rsc_time_slot_1_ns;
	const u64 pdc_jitter = 20; /* 20% more */

	u64 frame_time_ns, frame_jitter;
	u64 line_time_ns, prefill_time_ns;
	u64 pdc_backoff_time_ns;
	s64 total;
	int ret = 0;

	if (cmd_config)
		memcpy(&rsc->cmd_config, cmd_config, sizeof(*cmd_config));

	/* calculate for 640x480 60 fps resolution by default */
	if (!rsc->cmd_config.fps)
		rsc->cmd_config.fps = DEFAULT_PANEL_FPS;
	if (!rsc->cmd_config.jitter_numer)
		rsc->cmd_config.jitter_numer = DEFAULT_PANEL_JITTER_NUMERATOR;
	if (!rsc->cmd_config.jitter_denom)
		rsc->cmd_config.jitter_denom = DEFAULT_PANEL_JITTER_DENOMINATOR;
	if (!rsc->cmd_config.vtotal)
		rsc->cmd_config.vtotal = DEFAULT_PANEL_VTOTAL;
	if (!rsc->cmd_config.prefill_lines)
		rsc->cmd_config.prefill_lines = DEFAULT_PANEL_PREFILL_LINES;
	pr_debug("frame fps:%d jitter_numer:%d jitter_denom:%d vtotal:%d prefill lines:%d\n",
		rsc->cmd_config.fps, rsc->cmd_config.jitter_numer,
		rsc->cmd_config.jitter_denom, rsc->cmd_config.vtotal,
		rsc->cmd_config.prefill_lines);

	/* 1 nano second */
	frame_time_ns = TICKS_IN_NANO_SECOND;
	frame_time_ns = div_u64(frame_time_ns, rsc->cmd_config.fps);

	frame_jitter = frame_time_ns * rsc->cmd_config.jitter_numer;
	frame_jitter = div_u64(frame_jitter, rsc->cmd_config.jitter_denom);
	/* convert it to percentage */
	frame_jitter = div_u64(frame_jitter, 100);

	line_time_ns = frame_time_ns;
	line_time_ns = div_u64(line_time_ns, rsc->cmd_config.vtotal);
	prefill_time_ns = line_time_ns * rsc->cmd_config.prefill_lines;

	total = frame_time_ns - frame_jitter - prefill_time_ns;
	if (total < 0) {
		pr_err("invalid total time period time:%llu jiter_time:%llu blanking time:%llu\n",
			frame_time_ns, frame_jitter, prefill_time_ns);
		total = 0;
	}

	total = div_u64(total, cxo_period_ns);
	rsc->timer_config.static_wakeup_time_ns = total;

	pr_debug("frame time:%llu frame jiter_time:%llu\n",
			frame_time_ns, frame_jitter);
	pr_debug("line time:%llu prefill time ps:%llu\n",
			line_time_ns, prefill_time_ns);
	pr_debug("static wakeup time:%lld cxo:%u\n", total, cxo_period_ns);

	pdc_backoff_time_ns = rsc_backoff_time_ns;
	rsc_backoff_time_ns = div_u64(rsc_backoff_time_ns, cxo_period_ns);
	rsc->timer_config.rsc_backoff_time_ns = (u32) rsc_backoff_time_ns;

	pdc_backoff_time_ns *= pdc_jitter;
	pdc_backoff_time_ns = div_u64(pdc_backoff_time_ns, 100);
	rsc->timer_config.pdc_backoff_time_ns = (u32) pdc_backoff_time_ns;

	rsc_mode_threshold_time_ns =
			div_u64(rsc_mode_threshold_time_ns, cxo_period_ns);
	rsc->timer_config.rsc_mode_threshold_time_ns
					= (u32) rsc_mode_threshold_time_ns;

	/* time_slot_0 for mode0 latency */
	rsc_time_slot_0_ns = div_u64(rsc_time_slot_0_ns, cxo_period_ns);
	rsc->timer_config.rsc_time_slot_0_ns = (u32) rsc_time_slot_0_ns;

	/* time_slot_1 for mode1 latency */
	rsc_time_slot_1_ns = frame_time_ns;
	rsc_time_slot_1_ns = div_u64(rsc_time_slot_1_ns, cxo_period_ns);
	rsc->timer_config.rsc_time_slot_1_ns = (u32) rsc_time_slot_1_ns;

	/* mode 2 is infinite */
	rsc->timer_config.rsc_time_slot_2_ns = 0xFFFFFFFF;

	/* timer update should be called with client call */
	if (cmd_config && rsc->hw_ops.timer_update) {
		ret = rsc->hw_ops.timer_update(rsc);
		if (ret)
			pr_err("sde rsc: hw timer update failed ret:%d\n", ret);
	/* rsc init should be called during rsc probe - one time only */
	} else if (rsc->hw_ops.init) {
		ret = rsc->hw_ops.init(rsc);
		if (ret)
			pr_err("sde rsc: hw init failed ret:%d\n", ret);
	}

	return ret;
}

static int sde_rsc_switch_to_idle(struct sde_rsc_priv *rsc)
{
	struct sde_rsc_client *client;
	int rc = STATE_UPDATE_NOT_ALLOWED;
	bool idle_switch = true;

	list_for_each_entry(client, &rsc->client_list, list)
		if (client->current_state != SDE_RSC_IDLE_STATE) {
			idle_switch = false;
			break;
		}

	if (!idle_switch) {
		/**
		 * following code needs to run the loop through each
		 * client because they might be in different order
		 * sorting is not possible; only preference is available
		 */

		/* first check if any vid client active */
		list_for_each_entry(client, &rsc->client_list, list)
			if (client->current_state == SDE_RSC_VID_STATE)
				return rc;

		/* now try cmd state switch */
		list_for_each_entry(client, &rsc->client_list, list)
			if (client->current_state == SDE_RSC_CMD_STATE)
				return TRY_CMD_MODE_SWITCH;

		/* now try clk state switch */
		list_for_each_entry(client, &rsc->client_list, list)
			if (client->current_state == SDE_RSC_CLK_STATE)
				return TRY_CLK_MODE_SWITCH;

	} else if (rsc->hw_ops.state_update) {
		rc = rsc->hw_ops.state_update(rsc, SDE_RSC_IDLE_STATE);
		if (!rc)
			rpmh_mode_solver_set(rsc->disp_rsc, true);
	}

	return rc;
}

static int sde_rsc_switch_to_cmd(struct sde_rsc_priv *rsc,
	struct sde_rsc_cmd_config *config,
	struct sde_rsc_client *caller_client,
	int *wait_vblank_crtc_id)
{
	struct sde_rsc_client *client;
	int rc = STATE_UPDATE_NOT_ALLOWED;

	if (!rsc->primary_client) {
		pr_err("primary client not available for cmd state switch\n");
		rc = -EINVAL;
		goto end;
	} else if (caller_client != rsc->primary_client) {
		pr_err("primary client state:%d not cmd state request\n",
			rsc->primary_client->current_state);
		rc = -EINVAL;
		goto end;
	}

	/* update timers - might not be available at next switch */
	if (config)
		sde_rsc_timer_calculate(rsc, config);

	/**
	 * rsc clients can still send config at any time. If a config is
	 * received during cmd_state then vsync_wait will execute with the logic
	 * below. If a config is received when rsc is in AMC mode; A mode
	 * switch will do the vsync wait. updated checks still support all cases
	 * for dynamic mode switch and inline rotation.
	 */
	if (rsc->current_state == SDE_RSC_CMD_STATE) {
		rc = 0;
		if (config)
			goto vsync_wait;
		else
			goto end;
	}

	/* any one client in video state blocks the cmd state switch */
	list_for_each_entry(client, &rsc->client_list, list)
		if (client->current_state == SDE_RSC_VID_STATE)
			goto end;

	if (rsc->hw_ops.state_update) {
		rc = rsc->hw_ops.state_update(rsc, SDE_RSC_CMD_STATE);
		if (!rc)
			rpmh_mode_solver_set(rsc->disp_rsc, true);
	}

vsync_wait:
	/* indicate wait for vsync for vid to cmd state switch & cfg update */
	if (!rc && (rsc->current_state == SDE_RSC_VID_STATE ||
			rsc->current_state == SDE_RSC_CMD_STATE)) {
		/* clear VSYNC timestamp for indication when update completes */
		if (rsc->hw_ops.hw_vsync)
			rsc->hw_ops.hw_vsync(rsc, VSYNC_ENABLE, NULL, 0, 0);
		if (!wait_vblank_crtc_id) {
			pr_err("invalid crtc id wait pointer, client %d\n",
					caller_client->id);
			SDE_EVT32(caller_client->id, rsc->current_state,
					caller_client->crtc_id,
					wait_vblank_crtc_id, SDE_EVTLOG_ERROR);
			msleep(PRIMARY_VBLANK_WORST_CASE_MS);
		} else {
			*wait_vblank_crtc_id = rsc->primary_client->crtc_id;
		}
	}
end:
	return rc;
}

static int sde_rsc_switch_to_clk(struct sde_rsc_priv *rsc,
		int *wait_vblank_crtc_id)
{
	struct sde_rsc_client *client;
	int rc = STATE_UPDATE_NOT_ALLOWED;

	list_for_each_entry(client, &rsc->client_list, list)
		if ((client->current_state == SDE_RSC_VID_STATE) ||
		    (client->current_state == SDE_RSC_CMD_STATE))
			goto end;

	if (rsc->hw_ops.state_update) {
		rc = rsc->hw_ops.state_update(rsc, SDE_RSC_CLK_STATE);
		if (!rc)
			rpmh_mode_solver_set(rsc->disp_rsc, false);
	}

	/* indicate wait for vsync for cmd to clk state switch */
	if (!rc && rsc->primary_client &&
			(rsc->current_state == SDE_RSC_CMD_STATE)) {
		/* clear VSYNC timestamp for indication when update completes */
		if (rsc->hw_ops.hw_vsync)
			rsc->hw_ops.hw_vsync(rsc, VSYNC_ENABLE, NULL, 0, 0);
		if (!wait_vblank_crtc_id) {
			pr_err("invalid crtc id wait pointer provided\n");
			msleep(PRIMARY_VBLANK_WORST_CASE_MS);
		} else {
			*wait_vblank_crtc_id = rsc->primary_client->crtc_id;

			/* increase refcount, so we wait for the next vsync */
			atomic_inc(&rsc->rsc_vsync_wait);
			SDE_EVT32(atomic_read(&rsc->rsc_vsync_wait));
		}
	} else if (atomic_read(&rsc->rsc_vsync_wait)) {
		SDE_EVT32(rsc->primary_client, rsc->current_state,
			atomic_read(&rsc->rsc_vsync_wait));

		/* Wait for the vsync, if the refcount is set */
		rc = wait_event_timeout(rsc->rsc_vsync_waitq,
			atomic_read(&rsc->rsc_vsync_wait) == 0,
			msecs_to_jiffies(PRIMARY_VBLANK_WORST_CASE_MS*2));
		if (!rc) {
			pr_err("Timeout waiting for vsync\n");
			rc = -ETIMEDOUT;
			SDE_EVT32(atomic_read(&rsc->rsc_vsync_wait), rc,
				SDE_EVTLOG_ERROR);
		} else {
			SDE_EVT32(atomic_read(&rsc->rsc_vsync_wait), rc);
			rc = 0;
		}
	}
end:
	return rc;
}

static int sde_rsc_switch_to_vid(struct sde_rsc_priv *rsc,
	struct sde_rsc_cmd_config *config,
	struct sde_rsc_client *caller_client,
	int *wait_vblank_crtc_id)
{
	int rc = 0;

	/* update timers - might not be available at next switch */
	if (config && (caller_client == rsc->primary_client))
		sde_rsc_timer_calculate(rsc, config);

	/* early exit without vsync wait for vid state */
	if (rsc->current_state == SDE_RSC_VID_STATE)
		goto end;

	/* video state switch should be done immediately */
	if (rsc->hw_ops.state_update) {
		rc = rsc->hw_ops.state_update(rsc, SDE_RSC_VID_STATE);
		if (!rc)
			rpmh_mode_solver_set(rsc->disp_rsc, false);
	}

	/* indicate wait for vsync for cmd to vid state switch */
	if (!rc && rsc->primary_client &&
			(rsc->current_state == SDE_RSC_CMD_STATE)) {
		/* clear VSYNC timestamp for indication when update completes */
		if (rsc->hw_ops.hw_vsync)
			rsc->hw_ops.hw_vsync(rsc, VSYNC_ENABLE, NULL, 0, 0);
		if (!wait_vblank_crtc_id) {
			pr_err("invalid crtc id wait pointer provided\n");
			msleep(PRIMARY_VBLANK_WORST_CASE_MS);
		} else {
			*wait_vblank_crtc_id = rsc->primary_client->crtc_id;

			/* increase refcount, so we wait for the next vsync */
			atomic_inc(&rsc->rsc_vsync_wait);
			SDE_EVT32(atomic_read(&rsc->rsc_vsync_wait));
		}
	} else if (atomic_read(&rsc->rsc_vsync_wait)) {
		SDE_EVT32(rsc->primary_client, rsc->current_state,
			atomic_read(&rsc->rsc_vsync_wait));

		/* Wait for the vsync, if the refcount is set */
		rc = wait_event_timeout(rsc->rsc_vsync_waitq,
			atomic_read(&rsc->rsc_vsync_wait) == 0,
			msecs_to_jiffies(PRIMARY_VBLANK_WORST_CASE_MS*2));
		if (!rc) {
			pr_err("Timeout waiting for vsync\n");
			rc = -ETIMEDOUT;
			SDE_EVT32(atomic_read(&rsc->rsc_vsync_wait), rc,
				SDE_EVTLOG_ERROR);
		} else {
			SDE_EVT32(atomic_read(&rsc->rsc_vsync_wait), rc);
			rc = 0;
		}
	}

end:
	return rc;
}

/**
 * sde_rsc_client_get_vsync_refcount() - returns the status of the vsync
 * refcount, to signal if the client needs to reset the refcounting logic
 * @client:	 Client pointer provided by sde_rsc_client_create().
 *
 * Return: value of the vsync refcount.
 */
int sde_rsc_client_get_vsync_refcount(
		struct sde_rsc_client *caller_client)
{
	struct sde_rsc_priv *rsc;

	if (!caller_client) {
		pr_err("invalid client for rsc state update\n");
		return -EINVAL;
	} else if (caller_client->rsc_index >= MAX_RSC_COUNT) {
		pr_err("invalid rsc index\n");
		return -EINVAL;
	}

	rsc = rsc_prv_list[caller_client->rsc_index];
	if (!rsc)
		return 0;

	return atomic_read(&rsc->rsc_vsync_wait);
}

/**
 * sde_rsc_client_reset_vsync_refcount() - reduces the refcounting
 * logic that waits for the vsync.
 * @client:	 Client pointer provided by sde_rsc_client_create().
 *
 * Return: zero if refcount was already zero.
 */
int sde_rsc_client_reset_vsync_refcount(
		struct sde_rsc_client *caller_client)
{
	struct sde_rsc_priv *rsc;
	int ret;

	if (!caller_client) {
		pr_err("invalid client for rsc state update\n");
		return -EINVAL;
	} else if (caller_client->rsc_index >= MAX_RSC_COUNT) {
		pr_err("invalid rsc index\n");
		return -EINVAL;
	}

	rsc = rsc_prv_list[caller_client->rsc_index];
	if (!rsc)
		return 0;

	ret = atomic_add_unless(&rsc->rsc_vsync_wait, -1, 0);
	wake_up_all(&rsc->rsc_vsync_waitq);
	SDE_EVT32(atomic_read(&rsc->rsc_vsync_wait));

	return ret;
}

/**
 * sde_rsc_client_is_state_update_complete() - check if state update is complete
 * RSC state transition is not complete until HW receives VBLANK signal. This
 * function checks RSC HW to determine whether that signal has been received.
 * @client:	 Client pointer provided by sde_rsc_client_create().
 *
 * Return: true if the state update has completed.
 */
bool sde_rsc_client_is_state_update_complete(
		struct sde_rsc_client *caller_client)
{
	struct sde_rsc_priv *rsc;
	u32 vsync_timestamp0 = 0;

	if (!caller_client) {
		pr_err("invalid client for rsc state update\n");
		return false;
	} else if (caller_client->rsc_index >= MAX_RSC_COUNT) {
		pr_err("invalid rsc index\n");
		return false;
	}

	rsc = rsc_prv_list[caller_client->rsc_index];
	if (!rsc)
		return false;

	/**
	 * state updates clear VSYNC timestamp, check if a new one arrived.
	 * use VSYNC mode 0 (CMD TE) always for this, per HW recommendation.
	 */
	if (rsc->hw_ops.hw_vsync)
		vsync_timestamp0 = rsc->hw_ops.hw_vsync(rsc, VSYNC_READ_VSYNC0,
				NULL, 0, 0);

	return vsync_timestamp0 != 0;
}

/**
 * sde_rsc_client_state_update() - rsc client state update
 * Video mode, cmd mode and clk state are suppoed as modes. A client need to
 * set this property during panel config time. A switching client can set the
 * property to change the state
 *
 * @client:	 Client pointer provided by sde_rsc_client_create().
 * @state:	 Client state - video/cmd
 * @config:	 fps, vtotal, porches, etc configuration for command mode
 *               panel
 * @crtc_id:	 current client's crtc id
 * @wait_vblank_crtc_id:	Output parameter. If set to non-zero, rsc hw
 *				state update requires a wait for one vblank on
 *				the primary crtc. In that case, this output
 *				param will be set to the crtc on which to wait.
 *				If SDE_RSC_INVALID_CRTC_ID, no wait necessary
 *
 * Return: error code.
 */
int sde_rsc_client_state_update(struct sde_rsc_client *caller_client,
	enum sde_rsc_state state,
	struct sde_rsc_cmd_config *config, int crtc_id,
	int *wait_vblank_crtc_id)
{
	int rc = 0;
	struct sde_rsc_priv *rsc;

	if (!caller_client) {
		pr_err("invalid client for rsc state update\n");
		return -EINVAL;
	} else if (caller_client->rsc_index >= MAX_RSC_COUNT) {
		pr_err("invalid rsc index\n");
		return -EINVAL;
	}

	rsc = rsc_prv_list[caller_client->rsc_index];
	if (!rsc)
		return -EINVAL;

	if (wait_vblank_crtc_id)
		*wait_vblank_crtc_id = SDE_RSC_INVALID_CRTC_ID;

	mutex_lock(&rsc->client_lock);
	SDE_EVT32_VERBOSE(caller_client->id, caller_client->current_state,
			state, rsc->current_state, SDE_EVTLOG_FUNC_ENTRY);
	caller_client->crtc_id = crtc_id;
	caller_client->current_state = state;

	if (rsc->master_drm == NULL) {
		pr_err("invalid master component binding\n");
		rc = -EINVAL;
		goto end;
	} else if ((rsc->current_state == state) && !config) {
		pr_debug("no state change: %d\n", state);
		goto end;
	}

	pr_debug("%pS: rsc state:%d request client:%s state:%d\n",
		__builtin_return_address(0), rsc->current_state,
		caller_client->name, state);

	if (rsc->current_state == SDE_RSC_IDLE_STATE)
		sde_rsc_clk_enable(&rsc->phandle, rsc->pclient, true);

	switch (state) {
	case SDE_RSC_IDLE_STATE:
		rc = sde_rsc_switch_to_idle(rsc);

		/* video state client might be exiting; try cmd state switch */
		if (rc == TRY_CMD_MODE_SWITCH) {
			rc = sde_rsc_switch_to_cmd(rsc, NULL,
					rsc->primary_client,
					wait_vblank_crtc_id);
			if (!rc)
				state = SDE_RSC_CMD_STATE;

		/* cmd state client might be exiting; try clk state switch */
		} else if (rc == TRY_CLK_MODE_SWITCH) {
			rc = sde_rsc_switch_to_clk(rsc, wait_vblank_crtc_id);
			if (!rc)
				state = SDE_RSC_CLK_STATE;
		}
		break;

	case SDE_RSC_CMD_STATE:
		rc = sde_rsc_switch_to_cmd(rsc, config, caller_client,
				wait_vblank_crtc_id);
		break;

	case SDE_RSC_VID_STATE:
		rc = sde_rsc_switch_to_vid(rsc, config, caller_client,
				wait_vblank_crtc_id);
		break;

	case SDE_RSC_CLK_STATE:
		rc = sde_rsc_switch_to_clk(rsc, wait_vblank_crtc_id);
		break;

	default:
		pr_err("invalid state handling %d\n", state);
		break;
	}

	if (rc == STATE_UPDATE_NOT_ALLOWED) {
		rc = 0;
		SDE_EVT32(caller_client->id, caller_client->current_state,
			state, rsc->current_state, rc, SDE_EVTLOG_FUNC_CASE1);
		goto clk_disable;
	} else if (rc) {
		pr_debug("state:%d update failed rc:%d\n", state, rc);
		SDE_EVT32(caller_client->id, caller_client->current_state,
			state, rsc->current_state, rc, SDE_EVTLOG_FUNC_CASE2);
		goto clk_disable;
	}

	pr_debug("state switch successfully complete: %d\n", state);
	rsc->current_state = state;
	SDE_EVT32(caller_client->id, caller_client->current_state,
			state, rsc->current_state, SDE_EVTLOG_FUNC_EXIT);

clk_disable:
	if (rsc->current_state == SDE_RSC_IDLE_STATE)
		sde_rsc_clk_enable(&rsc->phandle, rsc->pclient, false);
end:
	mutex_unlock(&rsc->client_lock);
	return rc;
}
EXPORT_SYMBOL(sde_rsc_client_state_update);

/**
 * sde_rsc_client_vote() - ab/ib vote from rsc client
 *
 * @client:	 Client pointer provided by sde_rsc_client_create().
 * @bus_id: data bus for which to be voted
 * @ab:		 aggregated bandwidth vote from client.
 * @ib:		 instant bandwidth vote from client.
 *
 * Return: error code.
 */
int sde_rsc_client_vote(struct sde_rsc_client *caller_client,
		u32 bus_id, u64 ab_vote, u64 ib_vote)
{
	int rc = 0, rsc_index;
	struct sde_rsc_priv *rsc;

	if (caller_client && caller_client->rsc_index >= MAX_RSC_COUNT) {
		pr_err("invalid rsc index\n");
		return -EINVAL;
	}

	rsc_index = caller_client ? caller_client->rsc_index : SDE_RSC_INDEX;
	rsc = rsc_prv_list[rsc_index];
	if (!rsc)
		return -EINVAL;

	pr_debug("client:%s ab:%llu ib:%llu\n",
			caller_client ? caller_client->name : "unknown",
			ab_vote, ib_vote);

	mutex_lock(&rsc->client_lock);
	rc = sde_rsc_clk_enable(&rsc->phandle, rsc->pclient, true);
	if (rc)
		goto clk_enable_fail;

	if (rsc->hw_ops.tcs_wait) {
		rc = rsc->hw_ops.tcs_wait(rsc);
		if (rc) {
			pr_err("tcs is still busy; can't send command\n");
			if (rsc->hw_ops.tcs_use_ok)
				rsc->hw_ops.tcs_use_ok(rsc);
			goto end;
		}
	}

	rpmh_invalidate(rsc->disp_rsc);
	sde_power_data_bus_set_quota(&rsc->phandle, rsc->pclient,
		SDE_POWER_HANDLE_DATA_BUS_CLIENT_RT,
		bus_id, ab_vote, ib_vote);
	rpmh_flush(rsc->disp_rsc);

	if (rsc->hw_ops.tcs_use_ok)
		rsc->hw_ops.tcs_use_ok(rsc);

end:
	sde_rsc_clk_enable(&rsc->phandle, rsc->pclient, false);
clk_enable_fail:
	mutex_unlock(&rsc->client_lock);

	return rc;
}
EXPORT_SYMBOL(sde_rsc_client_vote);

#if defined(CONFIG_DEBUG_FS)
void sde_rsc_debug_dump(u32 mux_sel)
{
	struct sde_rsc_priv *rsc;

	rsc = rsc_prv_list[SDE_RSC_INDEX];
	if (!rsc)
		return;

	/* this must be called with rsc clocks enabled */
	if (rsc->hw_ops.debug_dump)
		rsc->hw_ops.debug_dump(rsc, mux_sel);
}
#endif /* defined(CONFIG_DEBUG_FS) */

static int _sde_debugfs_status_show(struct seq_file *s, void *data)
{
	struct sde_rsc_priv *rsc;
	struct sde_rsc_client *client;
	int ret;

	if (!s || !s->private)
		return -EINVAL;

	rsc = s->private;

	mutex_lock(&rsc->client_lock);
	ret = sde_rsc_clk_enable(&rsc->phandle, rsc->pclient, true);
	if (ret)
		goto end;

	seq_printf(s, "rsc current state:%d\n", rsc->current_state);
	seq_printf(s, "wraper backoff time(ns):%d\n",
				rsc->timer_config.static_wakeup_time_ns);
	seq_printf(s, "rsc backoff time(ns):%d\n",
				rsc->timer_config.rsc_backoff_time_ns);
	seq_printf(s, "pdc backoff time(ns):%d\n",
				rsc->timer_config.pdc_backoff_time_ns);
	seq_printf(s, "rsc mode threshold time(ns):%d\n",
				rsc->timer_config.rsc_mode_threshold_time_ns);
	seq_printf(s, "rsc time slot 0(ns):%d\n",
				rsc->timer_config.rsc_time_slot_0_ns);
	seq_printf(s, "rsc time slot 1(ns):%d\n",
				rsc->timer_config.rsc_time_slot_1_ns);
	seq_printf(s, "frame fps:%d jitter_numer:%d jitter_denom:%d vtotal:%d prefill lines:%d\n",
			rsc->cmd_config.fps, rsc->cmd_config.jitter_numer,
			rsc->cmd_config.jitter_denom,
			rsc->cmd_config.vtotal, rsc->cmd_config.prefill_lines);

	seq_puts(s, "\n");

	list_for_each_entry(client, &rsc->client_list, list)
		seq_printf(s, "\t client:%s state:%d\n",
				client->name, client->current_state);

	if (rsc->hw_ops.debug_show) {
		ret = rsc->hw_ops.debug_show(s, rsc);
		if (ret)
			pr_err("sde rsc: hw debug failed ret:%d\n", ret);
	}
	sde_rsc_clk_enable(&rsc->phandle, rsc->pclient, false);

end:
	mutex_unlock(&rsc->client_lock);
	return 0;
}

static int _sde_debugfs_status_open(struct inode *inode, struct file *file)
{
	return single_open(file, _sde_debugfs_status_show, inode->i_private);
}

static int _sde_debugfs_mode_ctrl_open(struct inode *inode, struct file *file)
{
	/* non-seekable */
	file->private_data = inode->i_private;
	return nonseekable_open(inode, file);
}

static ssize_t _sde_debugfs_mode_ctrl_read(struct file *file, char __user *buf,
				size_t count, loff_t *ppos)
{
	struct sde_rsc_priv *rsc = file->private_data;
	char buffer[MAX_BUFFER_SIZE];
	int blen = 0, rc;

	if (*ppos || !rsc || !rsc->hw_ops.mode_ctrl)
		return 0;

	mutex_lock(&rsc->client_lock);
	rc = sde_rsc_clk_enable(&rsc->phandle, rsc->pclient, true);
	if (rc)
		goto end;

	blen = rsc->hw_ops.mode_ctrl(rsc, MODE_READ, buffer,
							MAX_BUFFER_SIZE, 0);

	sde_rsc_clk_enable(&rsc->phandle, rsc->pclient, false);

end:
	mutex_unlock(&rsc->client_lock);
	if (blen <= 0)
		return 0;

	if (copy_to_user(buf, buffer, blen))
		return -EFAULT;

	*ppos += blen;
	return blen;
}

static ssize_t _sde_debugfs_mode_ctrl_write(struct file *file,
			const char __user *p, size_t count, loff_t *ppos)
{
	struct sde_rsc_priv *rsc = file->private_data;
	char *input;
	u32 mode_state = 0;
	int rc;

	if (!rsc || !rsc->hw_ops.mode_ctrl || !count ||
					count > MAX_COUNT_SIZE_SUPPORTED)
		return 0;

	input = kmalloc(count + 1, GFP_KERNEL);
	if (!input)
		return -ENOMEM;

	if (copy_from_user(input, p, count)) {
		kfree(input);
		return -EFAULT;
	}
	input[count] = '\0';

	rc = kstrtoint(input, 0, &mode_state);
	if (rc) {
		pr_err("mode_state: int conversion failed rc:%d\n", rc);
		goto end;
	}

	pr_debug("mode_state: %d\n", mode_state);
	mode_state &= 0x7;
	if (mode_state != ALL_MODES_DISABLED &&
			mode_state != ALL_MODES_ENABLED &&
			mode_state != ONLY_MODE_0_ENABLED &&
			mode_state != ONLY_MODE_0_1_ENABLED) {
		pr_err("invalid mode:%d combination\n", mode_state);
		goto end;
	}

	mutex_lock(&rsc->client_lock);
	rc = sde_rsc_clk_enable(&rsc->phandle, rsc->pclient, true);
	if (rc)
		goto clk_enable_fail;

	rsc->hw_ops.mode_ctrl(rsc, MODE_UPDATE, NULL, 0, mode_state);
	sde_rsc_clk_enable(&rsc->phandle, rsc->pclient, false);

clk_enable_fail:
	mutex_unlock(&rsc->client_lock);
end:
	kfree(input);
	return count;
}

static int _sde_debugfs_vsync_mode_open(struct inode *inode, struct file *file)
{
	/* non-seekable */
	file->private_data = inode->i_private;
	return nonseekable_open(inode, file);
}

static ssize_t _sde_debugfs_vsync_mode_read(struct file *file, char __user *buf,
				size_t count, loff_t *ppos)
{
	struct sde_rsc_priv *rsc = file->private_data;
	char buffer[MAX_BUFFER_SIZE];
	int blen = 0, rc;

	if (*ppos || !rsc || !rsc->hw_ops.hw_vsync)
		return 0;

	mutex_lock(&rsc->client_lock);
	rc = sde_rsc_clk_enable(&rsc->phandle, rsc->pclient, true);
	if (rc)
		goto end;

	blen = rsc->hw_ops.hw_vsync(rsc, VSYNC_READ, buffer,
						MAX_BUFFER_SIZE, 0);

	sde_rsc_clk_enable(&rsc->phandle, rsc->pclient, false);

end:
	mutex_unlock(&rsc->client_lock);
	if (blen <= 0)
		return 0;

	if (copy_to_user(buf, buffer, blen))
		return -EFAULT;

	*ppos += blen;
	return blen;
}

static ssize_t _sde_debugfs_vsync_mode_write(struct file *file,
			const char __user *p, size_t count, loff_t *ppos)
{
	struct sde_rsc_priv *rsc = file->private_data;
	char *input;
	u32 vsync_state = 0;
	int rc;

	if (!rsc || !rsc->hw_ops.hw_vsync || !count ||
				count > MAX_COUNT_SIZE_SUPPORTED)
		return 0;

	input = kmalloc(count + 1, GFP_KERNEL);
	if (!input)
		return -ENOMEM;

	if (copy_from_user(input, p, count)) {
		kfree(input);
		return -EFAULT;
	}
	input[count] = '\0';

	rc = kstrtoint(input, 0, &vsync_state);
	if (rc) {
		pr_err("vsync_state: int conversion failed rc:%d\n", rc);
		goto end;
	}

	pr_debug("vsync_state: %d\n", vsync_state);
	vsync_state &= 0x7;

	mutex_lock(&rsc->client_lock);
	rc = sde_rsc_clk_enable(&rsc->phandle, rsc->pclient, true);
	if (rc)
		goto clk_en_fail;

	if (vsync_state)
		rsc->hw_ops.hw_vsync(rsc, VSYNC_ENABLE, NULL,
							0, vsync_state - 1);
	else
		rsc->hw_ops.hw_vsync(rsc, VSYNC_DISABLE, NULL, 0, 0);

	sde_rsc_clk_enable(&rsc->phandle, rsc->pclient, false);

clk_en_fail:
	mutex_unlock(&rsc->client_lock);
end:
	kfree(input);
	return count;
}

static const struct file_operations debugfs_status_fops = {
	.open =		_sde_debugfs_status_open,
	.read =		seq_read,
	.llseek =	seq_lseek,
	.release =	single_release,
};

static const struct file_operations mode_control_fops = {
	.open =		_sde_debugfs_mode_ctrl_open,
	.read =		_sde_debugfs_mode_ctrl_read,
	.write =	_sde_debugfs_mode_ctrl_write,
};

static const struct file_operations vsync_status_fops = {
	.open =		_sde_debugfs_vsync_mode_open,
	.read =		_sde_debugfs_vsync_mode_read,
	.write =	_sde_debugfs_vsync_mode_write,
};

static void _sde_rsc_init_debugfs(struct sde_rsc_priv *rsc, char *name)
{
	rsc->debugfs_root = debugfs_create_dir(name, NULL);
	if (!rsc->debugfs_root)
		return;

	/* don't error check these */
	debugfs_create_file("status", 0400, rsc->debugfs_root, rsc,
							&debugfs_status_fops);
	debugfs_create_file("mode_control", 0600, rsc->debugfs_root, rsc,
							&mode_control_fops);
	debugfs_create_file("vsync_mode", 0600, rsc->debugfs_root, rsc,
							&vsync_status_fops);
	debugfs_create_x32("debug_mode", 0600, rsc->debugfs_root,
							&rsc->debug_mode);
}

static void sde_rsc_deinit(struct platform_device *pdev,
					struct sde_rsc_priv *rsc)
{
	if (!rsc)
		return;

	if (rsc->pclient)
		sde_rsc_clk_enable(&rsc->phandle, rsc->pclient, false);
	if (rsc->fs)
		devm_regulator_put(rsc->fs);
	if (rsc->wrapper_io.base)
		msm_dss_iounmap(&rsc->wrapper_io);
	if (rsc->drv_io.base)
		msm_dss_iounmap(&rsc->drv_io);
	if (rsc->disp_rsc)
		rpmh_release(rsc->disp_rsc);
	if (rsc->pclient)
		sde_power_client_destroy(&rsc->phandle, rsc->pclient);

	sde_power_resource_deinit(pdev, &rsc->phandle);
	debugfs_remove_recursive(rsc->debugfs_root);
	kfree(rsc);
}

/**
 * sde_rsc_bind - bind rsc device with controlling device
 * @dev:        Pointer to base of platform device
 * @master:     Pointer to container of drm device
 * @data:       Pointer to private data
 * Returns:     Zero on success
 */
static int sde_rsc_bind(struct device *dev,
		struct device *master,
		void *data)
{
	struct sde_rsc_priv *rsc;
	struct drm_device *drm;
	struct platform_device *pdev = to_platform_device(dev);

	if (!dev || !pdev || !master) {
		pr_err("invalid param(s), dev %pK, pdev %pK, master %pK\n",
				dev, pdev, master);
		return -EINVAL;
	}

	drm = dev_get_drvdata(master);
	rsc = platform_get_drvdata(pdev);
	if (!drm || !rsc) {
		pr_err("invalid param(s), drm %pK, rsc %pK\n",
				drm, rsc);
		return -EINVAL;
	}

	mutex_lock(&rsc->client_lock);
	rsc->master_drm = drm;
	mutex_unlock(&rsc->client_lock);

	sde_dbg_reg_register_base(SDE_RSC_DRV_DBG_NAME, rsc->drv_io.base,
							rsc->drv_io.len);
	sde_dbg_reg_register_base(SDE_RSC_WRAPPER_DBG_NAME,
				rsc->wrapper_io.base, rsc->wrapper_io.len);
	return 0;
}

/**
 * sde_rsc_unbind - unbind rsc from controlling device
 * @dev:        Pointer to base of platform device
 * @master:     Pointer to container of drm device
 * @data:       Pointer to private data
 */
static void sde_rsc_unbind(struct device *dev,
		struct device *master, void *data)
{
	struct sde_rsc_priv *rsc;
	struct platform_device *pdev = to_platform_device(dev);

	if (!dev || !pdev) {
		pr_err("invalid param(s)\n");
		return;
	}

	rsc = platform_get_drvdata(pdev);
	if (!rsc) {
		pr_err("invalid display rsc\n");
		return;
	}

	mutex_lock(&rsc->client_lock);
	rsc->master_drm = NULL;
	mutex_unlock(&rsc->client_lock);
}

static const struct component_ops sde_rsc_comp_ops = {
	.bind = sde_rsc_bind,
	.unbind = sde_rsc_unbind,
};

static int sde_rsc_probe(struct platform_device *pdev)
{
	int ret;
	struct sde_rsc_priv *rsc;
	static int counter;
	char  name[MAX_RSC_CLIENT_NAME_LEN];

	rsc = kzalloc(sizeof(*rsc), GFP_KERNEL);
	if (!rsc) {
		ret = -ENOMEM;
		goto rsc_alloc_fail;
	}

	platform_set_drvdata(pdev, rsc);
	of_property_read_u32(pdev->dev.of_node, "qcom,sde-rsc-version",
								&rsc->version);

	ret = sde_power_resource_init(pdev, &rsc->phandle);
	if (ret) {
		pr_err("sde rsc:power resource init failed ret:%d\n", ret);
		goto sde_rsc_fail;
	}

	rsc->pclient = sde_power_client_create(&rsc->phandle, "rsc");
	if (IS_ERR_OR_NULL(rsc->pclient)) {
		ret = PTR_ERR(rsc->pclient);
		rsc->pclient = NULL;
		pr_err("sde rsc:power client create failed ret:%d\n", ret);
		goto sde_rsc_fail;
	}

	/**
	 * sde rsc should always vote through enable path, sleep vote is
	 * set to "0" by default.
	 */
	sde_power_data_bus_state_update(&rsc->phandle, true);

	rsc->disp_rsc = rpmh_get_byname(pdev, "disp_rsc");
	if (IS_ERR_OR_NULL(rsc->disp_rsc)) {
		ret = PTR_ERR(rsc->disp_rsc);
		rsc->disp_rsc = NULL;
		pr_err("sde rsc:get display rsc failed ret:%d\n", ret);
		goto sde_rsc_fail;
	}

	ret = msm_dss_ioremap_byname(pdev, &rsc->wrapper_io, "wrapper");
	if (ret) {
		pr_err("sde rsc: wrapper io data mapping failed ret=%d\n", ret);
		goto sde_rsc_fail;
	}

	ret = msm_dss_ioremap_byname(pdev, &rsc->drv_io, "drv");
	if (ret) {
		pr_err("sde rsc: drv io data mapping failed ret:%d\n", ret);
		goto sde_rsc_fail;
	}

	rsc->fs = devm_regulator_get(&pdev->dev, "vdd");
	if (IS_ERR_OR_NULL(rsc->fs)) {
		rsc->fs = NULL;
		pr_err("unable to get regulator\n");
		goto sde_rsc_fail;
	}

	ret = sde_rsc_hw_register(rsc);
	if (ret) {
		pr_err("sde rsc: hw register failed ret:%d\n", ret);
		goto sde_rsc_fail;
	}

	if (sde_rsc_clk_enable(&rsc->phandle, rsc->pclient, true)) {
		pr_err("failed to enable sde rsc power resources\n");
		goto sde_rsc_fail;
	}

	if (sde_rsc_timer_calculate(rsc, NULL))
		goto sde_rsc_fail;

	sde_rsc_clk_enable(&rsc->phandle, rsc->pclient, false);

	INIT_LIST_HEAD(&rsc->client_list);
	INIT_LIST_HEAD(&rsc->event_list);
	mutex_init(&rsc->client_lock);
	init_waitqueue_head(&rsc->rsc_vsync_waitq);

	pr_info("sde rsc index:%d probed successfully\n",
				SDE_RSC_INDEX + counter);

	rsc_prv_list[SDE_RSC_INDEX + counter] = rsc;
	snprintf(name, MAX_RSC_CLIENT_NAME_LEN, "%s%d", "sde_rsc", counter);
	_sde_rsc_init_debugfs(rsc, name);
	counter++;

	ret = component_add(&pdev->dev, &sde_rsc_comp_ops);
	if (ret)
		pr_debug("component add failed, ret=%d\n", ret);
	ret = 0;

	return ret;

sde_rsc_fail:
	sde_rsc_deinit(pdev, rsc);
rsc_alloc_fail:
	return ret;
}

static int sde_rsc_remove(struct platform_device *pdev)
{
	struct sde_rsc_priv *rsc = platform_get_drvdata(pdev);

	sde_rsc_deinit(pdev, rsc);
	return 0;
}

static const struct of_device_id dt_match[] = {
	{ .compatible = "qcom,sde-rsc"},
	{}
};

MODULE_DEVICE_TABLE(of, dt_match);

static struct platform_driver sde_rsc_platform_driver = {
	.probe      = sde_rsc_probe,
	.remove     = sde_rsc_remove,
	.driver     = {
		.name   = "sde_rsc",
		.of_match_table = dt_match,
		.suppress_bind_attrs = true,
	},
};

static int __init sde_rsc_register(void)
{
	return platform_driver_register(&sde_rsc_platform_driver);
}

static void __exit sde_rsc_unregister(void)
{
	platform_driver_unregister(&sde_rsc_platform_driver);
}

module_init(sde_rsc_register);
module_exit(sde_rsc_unregister);
