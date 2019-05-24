/*
 * Restructured unified display panel driver for Xperia Open Devices
 *                          *** Main ***
 *
 * This driver is based on various SoMC implementations found in
 * copyleft archives for various devices.
 *
 * Copyright (c) 2012-2014, The Linux Foundation. All rights reserved.
 * Copyright (c) Sony Mobile Communications Inc. All rights reserved.
 * Copyright (C) 2014-2016, AngeloGioacchino Del Regno <kholk11@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/of_platform.h>
#include <linux/gpio.h>
#include <linux/qpnp/pin.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/leds.h>
#include <linux/qpnp/pwm.h>
#include <linux/err.h>
#include <linux/regulator/consumer.h>

#include "../mdss_mdp.h"
#include "../mdss_dsi.h"
#include "../mdss_dba_utils.h"
#include "somc_panels.h"

#define DT_CMD_HDR 		  6
#define MIN_REFRESH_RATE	  48
#define DEFAULT_MDP_TRANSFER_TIME 14000

#define VSYNC_DELAY msecs_to_jiffies(17)

struct device virtdev;

DEFINE_LED_TRIGGER(bl_led_trigger);

static bool display_on_in_boot;
static int lcm_first_boot = 0;
bool alt_panelid_cmd;
static bool mdss_panel_flip_ud = false;

static int __init continous_splash_setup(char *str)
{
	if (!str)
		return 0;
	if (!strncmp(str, "on", 2))
		display_on_in_boot = true;
	return 0;
}
__setup("display_status=", continous_splash_setup);

void mdss_dsi_panel_pwm_cfg(struct mdss_dsi_ctrl_pdata *ctrl)
{
	if (ctrl->pwm_pmi)
		return;

	ctrl->pwm_bl = pwm_request(ctrl->pwm_lpg_chan, "lcd-bklt");
	if (ctrl->pwm_bl == NULL || IS_ERR(ctrl->pwm_bl)) {
		pr_err("%s: Error: lpg_chan=%d pwm request failed",
				__func__, ctrl->pwm_lpg_chan);
	}
	ctrl->pwm_enabled = 0;
}

bool mdss_dsi_panel_pwm_enable(struct mdss_dsi_ctrl_pdata *ctrl)
{
	bool status = true;
	if (!ctrl->pwm_enabled)
		goto end;

	if (pwm_enable(ctrl->pwm_bl)) {
		pr_err("%s: pwm_enable() failed\n", __func__);
		status = false;
	}

	ctrl->pwm_enabled = 1;

end:
	return status;
}

static void mdss_dsi_panel_bklt_pwm(struct mdss_dsi_ctrl_pdata *ctrl,
								int level)
{
	int ret;
	u32 duty;
	u32 period_ns;

	if (ctrl->pwm_bl == NULL) {
		pr_err("%s: no PWM\n", __func__);
		return;
	}

	if (level == 0) {
		if (ctrl->pwm_enabled) {
			ret = pwm_config(ctrl->pwm_bl, level,
					ctrl->pwm_period * NSEC_PER_USEC);
			if (ret)
				pr_err("%s: pwm_config() failed err=%d.\n",
						__func__, ret);
			pwm_disable(ctrl->pwm_bl);
		}
		ctrl->pwm_enabled = 0;
		return;
	}

	duty = level * ctrl->pwm_period;
	duty /= ctrl->bklt_max;

	pr_debug("%s: bklt_ctrl=%d pwm_period=%d pwm_gpio=%d pwm_lpg_chan=%d\n",
			__func__, ctrl->bklt_ctrl, ctrl->pwm_period,
				ctrl->pwm_pmic_gpio, ctrl->pwm_lpg_chan);

	pr_debug("%s: ndx=%d level=%d duty=%d\n", __func__,
					ctrl->ndx, level, duty);

	if (ctrl->pwm_period >= USEC_PER_SEC) {
		ret = pwm_config(ctrl->pwm_bl, duty,
				ctrl->pwm_period * NSEC_PER_USEC);
		if (ret) {
			pr_err("%s: pwm_config() failed err=%d.\n",
					__func__, ret);
			return;
		}
	} else {
		period_ns = ctrl->pwm_period * NSEC_PER_USEC;
		ret = pwm_config(ctrl->pwm_bl,
				level * period_ns / ctrl->bklt_max,
				period_ns);
		if (ret) {
			pr_err("%s: pwm_config() failed err=%d.\n",
					__func__, ret);
			return;
		}
	}

	if (!ctrl->pwm_enabled) {
		ret = pwm_enable(ctrl->pwm_bl);
		if (ret)
			pr_err("%s: pwm_enable() failed err=%d\n", __func__,
				ret);
		ctrl->pwm_enabled = 1;
	}
}

static char dcs_cmd[2] = {0x54, 0x00}; /* DTYPE_DCS_READ */
static struct dsi_cmd_desc dcs_read_cmd = {
	{DTYPE_DCS_READ, 1, 0, 1, 5, sizeof(dcs_cmd)},
	dcs_cmd
};

int mdss_dsi_panel_cmd_read(struct mdss_dsi_ctrl_pdata *ctrl, char cmd0,
						char cmd1, void (*fxn)(int),
						char *rbuf, int len)
{
	struct dcs_cmd_req cmdreq;
	struct mdss_panel_info *pinfo;

	pinfo = &(ctrl->panel_data.panel_info);
	if (pinfo->dcs_cmd_by_left) {
		if (ctrl->ndx != DSI_CTRL_LEFT)
			return -EINVAL;
	}

	dcs_cmd[0] = cmd0;
	dcs_cmd[1] = cmd1;
	memset(&cmdreq, 0, sizeof(cmdreq));
	cmdreq.cmds = &dcs_read_cmd;
	cmdreq.cmds_cnt = 1;
	cmdreq.flags = CMD_REQ_RX | CMD_REQ_COMMIT;
	cmdreq.rlen = len;
	cmdreq.rbuf = rbuf;
	cmdreq.cb = fxn; /* call back */
	mdss_dsi_cmdlist_put(ctrl, &cmdreq);
	/*
	 * blocked here, until call back called
	 */

	return 0;
}

void __mdss_dsi_panel_cmds_send(struct mdss_dsi_ctrl_pdata *ctrl,
			struct dsi_panel_cmds *pcmds, u32 flags)
{
	struct dcs_cmd_req cmdreq;
	struct mdss_panel_info *pinfo;

	pinfo = &(ctrl->panel_data.panel_info);
	if (pinfo->dcs_cmd_by_left) {
		if (ctrl->ndx != DSI_CTRL_LEFT)
			return;
	}

	memset(&cmdreq, 0, sizeof(cmdreq));
	cmdreq.cmds = pcmds->cmds;
	cmdreq.cmds_cnt = pcmds->cmd_cnt;
	cmdreq.flags = flags;

	/*Panel ON/Off commands should be sent in DSI Low Power Mode*/
	if (pcmds->link_state == DSI_LP_MODE)
		cmdreq.flags  |= CMD_REQ_LP_MODE;

	cmdreq.rlen = 0;
	cmdreq.cb = NULL;

	mdss_dsi_cmdlist_put(ctrl, &cmdreq);
}

void mdss_dsi_panel_cmds_send(struct mdss_dsi_ctrl_pdata *ctrl,
			struct dsi_panel_cmds *pcmds)
{
	__mdss_dsi_panel_cmds_send(ctrl, pcmds, CMD_REQ_COMMIT);
}

static char led_pwm1[2] = {0x51, 0xFF};	/* DTYPE_DCS_WRITE1 */
static struct dsi_cmd_desc backlight_cmd = {
	{DTYPE_DCS_WRITE1, 1, 0, 0, 1, sizeof(led_pwm1)},
	led_pwm1
};

static void mdss_dsi_panel_bklt_dcs(struct mdss_dsi_ctrl_pdata *ctrl,
								int level)
{
	struct dcs_cmd_req cmdreq;
	struct mdss_panel_info *pinfo;

	pinfo = &(ctrl->panel_data.panel_info);
	if (pinfo->dcs_cmd_by_left) {
		if (ctrl->ndx != DSI_CTRL_LEFT)
			return;
	}

	pr_debug("%s: level=%d\n", __func__, level);

	led_pwm1[1] = (unsigned char)level;

	memset(&cmdreq, 0, sizeof(cmdreq));
	cmdreq.cmds = &backlight_cmd;
	cmdreq.cmds_cnt = 1;
	cmdreq.flags = CMD_REQ_COMMIT | CMD_CLK_CTRL;
	cmdreq.rlen = 0;
	cmdreq.cb = NULL;

	mdss_dsi_cmdlist_put(ctrl, &cmdreq);
}

/*
 * mdss_dsi_request_gpios - Request GPIOs used by somc_panel
 * 
 * This function contains common code and specific calls
 * with a specific structure: common GPIOs go first,
 * implementation-specific GPIO request functions go last.
 *
 * IMPORTANT: Error handling for implementation-specific
 *            functions SHALL NOT be managed in this function.
 */
int mdss_dsi_request_gpios(struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{
	int rc = 0;

	if (likely(ctrl_pdata->spec_pdata->gpios_requested))
		return 0;

	if (gpio_is_valid(ctrl_pdata->rst_gpio)) {
		rc = gpio_request(ctrl_pdata->rst_gpio, "disp_rst_n");
		if (rc) {
			pr_err("request reset gpio failed, rc=%d\n",
				rc);
			goto rst_gpio_err;
		}
	}

	rc = ctrl_pdata->spec_pdata->dsi_request_gpios(ctrl_pdata);

	if (rc == 0)
		ctrl_pdata->spec_pdata->gpios_requested = true;

	return rc;

rst_gpio_err:
	return rc;
}

static void mdss_dsi_panel_set_gpio_seq(
		int gpio, int seq_num, const int *seq)
{
	int i, rc;

	if (unlikely(seq == NULL)) {
		pr_err("%s: WARNING: Reset sequence is NULL! Bailing out.\n",
			__func__);
		return;
	}

	rc = gpio_direction_output(gpio, seq[0]);
	if (rc) {
		pr_err("%s: FATAL: Cannot set direction for reset GPIO %d\n",
			__func__, gpio);
		return;
	}

	for (i = 0; i + 1 < seq_num; i += 2) {
		gpio_set_value(gpio, seq[i]);
		usleep_range(seq[i + 1] * 1000, seq[i + 1] * 1000 + 100);
		pr_debug("%s: enable=%d, wait=%dms\n",
			__func__, seq[i], seq[i+1]);
	}
}

static int mdss_dsi_panel_reset_seq(struct mdss_panel_data *pdata, int enable)
{
	int rc;
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	struct mdss_panel_specific_pdata *spec_pdata = NULL;
	struct mdss_panel_info *pinfo;
	struct mdss_panel_power_seq *pw_seq;

	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);
	spec_pdata = ctrl_pdata->spec_pdata;

	if (lcm_first_boot)
		return 0;

	if (!ctrl_pdata->disp_en_gpio)
		pr_debug("%s:%d, disp_en line not configured\n",
			   __func__, __LINE__);

	if (!ctrl_pdata->rst_gpio)
		pr_debug("%s:%d, reset line not configured\n",
			   __func__, __LINE__);

	pr_debug("%s: enable=%d\n", __func__, enable);
	pinfo = &ctrl_pdata->panel_data.panel_info;

	rc = mdss_dsi_request_gpios(ctrl_pdata);
	if (rc) {
		pr_err("gpio request failed\n");
		return rc;
	}

	pw_seq = (enable) ? &ctrl_pdata->spec_pdata->on_seq :
				&ctrl_pdata->spec_pdata->off_seq;

#ifdef CONFIG_FBDEV_SOMC_PANEL_INCELL
	if (incell_panel_is_seq_for_ewu() && enable)
		pw_seq = &ctrl_pdata->spec_pdata->ewu_seq;
#endif

	if (pw_seq->rst_b_seq)
		mdss_dsi_panel_set_gpio_seq(ctrl_pdata->rst_gpio,
			pw_seq->seq_b_num, pw_seq->rst_b_seq);

	if (pw_seq->disp_en_pre)
		usleep_range(pw_seq->disp_en_pre * 1000,
				pw_seq->disp_en_pre * 1000 + 100);

	if (gpio_is_valid(ctrl_pdata->disp_en_gpio)) {
		if (enable)
			gpio_direction_output(ctrl_pdata->disp_en_gpio, 1);
		else
			gpio_set_value(ctrl_pdata->disp_en_gpio, 0);
	}

	if (pw_seq->disp_en_post)
		usleep_range(pw_seq->disp_en_post * 1000,
				pw_seq->disp_en_post * 1000 + 100);

	if (gpio_is_valid(ctrl_pdata->lcd_mode_sel_gpio) && enable) {
		switch (pinfo->mode_sel_state) {
			case MODE_GPIO_HIGH:
			case MODE_SEL_SINGLE_PORT:
				gpio_direction_output(
					ctrl_pdata->lcd_mode_sel_gpio, 1);
				break;
			case MODE_GPIO_LOW:
			case MODE_SEL_DUAL_PORT:
				gpio_direction_output(
					ctrl_pdata->lcd_mode_sel_gpio, 0);
				break;
			default:
				pr_err("%s: Mode selection UNDEFINED. FIXME!!",
					__func__);
				break;
		}
	}

	if (alt_panelid_cmd)
	{
		if (enable) {
			usleep_range(1000, 1000);
			if (gpio_is_valid(ctrl_pdata->bklt_en_gpio)) {
				gpio_direction_output(ctrl_pdata->bklt_en_gpio, 1);
				usleep_range(500, 1000);
			}
			if (gpio_is_valid(spec_pdata->disp_p5)) {
				gpio_direction_output(spec_pdata->disp_p5, 1);
				usleep_range(1000, 1000);
			}
			if (gpio_is_valid(spec_pdata->disp_n5)) {
				gpio_direction_output(spec_pdata->disp_n5, 1);
				usleep_range(10000, 10000);
			}

			if (ctrl_pdata->ctrl_state & CTRL_STATE_PANEL_INIT) {
				pr_info("panel not properly turned off\n");
				ctrl_pdata->ctrl_state &=
						~CTRL_STATE_PANEL_INIT;
				pr_info("reset panel done\n");
			}

			gpio_direction_output(ctrl_pdata->rst_gpio, 1);
			msleep(150);
		} else {
			if (gpio_is_valid(ctrl_pdata->bklt_en_gpio)) {
				gpio_set_value(ctrl_pdata->bklt_en_gpio, 0);
				usleep_range(1000, 1000);
			}
			gpio_set_value(ctrl_pdata->rst_gpio, 0);
			usleep_range(1000, 1000);
			gpio_set_value(spec_pdata->disp_n5, 0);
			usleep_range(1000, 1000);
			gpio_set_value(spec_pdata->disp_p5, 0);
			usleep_range(10000, 10000);
		}
	} else {
		if (unlikely(pw_seq->rst_seq == NULL)) {
			pr_err("FATAL: power%s reset sequence is NULL!!\n",
				enable ? "on" : "off");
			return 0;
		}

		if (unlikely(pw_seq->seq_num < 0 || pw_seq->seq_num > 20)) {
			pr_err("FATAL: Invalid number of entries for "\
				"power%s sequence!!\n", enable ? "on" : "off");
			return 0;
		};

		mdss_dsi_panel_set_gpio_seq(ctrl_pdata->rst_gpio,
			pw_seq->seq_num, pw_seq->rst_seq);
	}

	return 0;
}

int mdss_dsi_panel_reset(struct mdss_panel_data *pdata, int enable)
{
	return mdss_dsi_panel_reset_seq(pdata, enable);
}

/**
 * mdss_dsi_roi_merge() -  merge two roi into single roi
 *
 * Function used by partial update with only one dsi intf take 2A/2B
 * (column/page) dcs commands.
 */
static int mdss_dsi_roi_merge(struct mdss_dsi_ctrl_pdata *ctrl,
					struct mdss_rect *roi)
{
	struct mdss_panel_info *l_pinfo;
	struct mdss_rect *l_roi;
	struct mdss_rect *r_roi;
	struct mdss_dsi_ctrl_pdata *other = NULL;
	int ans = 0;

	if (ctrl->ndx == DSI_CTRL_LEFT) {
		other = mdss_dsi_get_ctrl_by_index(DSI_CTRL_RIGHT);
		if (!other)
			return ans;
		l_pinfo = &(ctrl->panel_data.panel_info);
		l_roi = &(ctrl->panel_data.panel_info.roi);
		r_roi = &(other->panel_data.panel_info.roi);
	} else  {
		other = mdss_dsi_get_ctrl_by_index(DSI_CTRL_LEFT);
		if (!other)
			return ans;
		l_pinfo = &(other->panel_data.panel_info);
		l_roi = &(other->panel_data.panel_info.roi);
		r_roi = &(ctrl->panel_data.panel_info.roi);
	}

	if (l_roi->w == 0 && l_roi->h == 0) {
		/* right only */
		*roi = *r_roi;
		roi->x += l_pinfo->xres;/* add left full width to x-offset */
	} else {
		/* left only and left+righ */
		*roi = *l_roi;
		roi->w +=  r_roi->w; /* add right width */
		ans = 1;
	}

	return ans;
}

static char caset[] = {0x2a, 0x00, 0x00, 0x03, 0x00};	/* DTYPE_DCS_LWRITE */
static char paset[] = {0x2b, 0x00, 0x00, 0x05, 0x00};	/* DTYPE_DCS_LWRITE */

/*
 * Some panels can support multiple ROIs as part of the below commands
 */
static char caset_dual[] = {0x2a, 0x00, 0x00, 0x03, 0x00, 0x03,
				0x00, 0x00, 0x00, 0x00};/* DTYPE_DCS_LWRITE */
static char paset_dual[] = {0x2b, 0x00, 0x00, 0x05, 0x00, 0x03,
				0x00, 0x00, 0x00, 0x00};/* DTYPE_DCS_LWRITE */

/* pack into one frame before sent */
static struct dsi_cmd_desc set_col_page_addr_cmd[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 1, sizeof(caset)}, caset},	/* packed */
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 1, sizeof(paset)}, paset},
};

static struct dsi_cmd_desc set_dual_col_page_addr_cmd[] = {	/*packed*/
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 1, sizeof(caset_dual)}, caset_dual},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 1, sizeof(paset_dual)}, paset_dual},
};

static void __mdss_dsi_send_col_page_addr(struct mdss_dsi_ctrl_pdata *ctrl,
		struct mdss_rect *roi, bool dual_roi)
{
	if (dual_roi) {
		struct mdss_rect *first, *second;

		first = &ctrl->panel_data.panel_info.dual_roi.first_roi;
		second = &ctrl->panel_data.panel_info.dual_roi.second_roi;

		caset_dual[1] = (((first->x) & 0xFF00) >> 8);
		caset_dual[2] = (((first->x) & 0xFF));
		caset_dual[3] = (((first->x - 1 + first->w) & 0xFF00) >> 8);
		caset_dual[4] = (((first->x - 1 + first->w) & 0xFF));
		/* skip the MPU setting byte*/
		caset_dual[6] = (((second->x) & 0xFF00) >> 8);
		caset_dual[7] = (((second->x) & 0xFF));
		caset_dual[8] = (((second->x - 1 + second->w) & 0xFF00) >> 8);
		caset_dual[9] = (((second->x - 1 + second->w) & 0xFF));
		set_dual_col_page_addr_cmd[0].payload = caset_dual;

		paset_dual[1] = (((first->y) & 0xFF00) >> 8);
		paset_dual[2] = (((first->y) & 0xFF));
		paset_dual[3] = (((first->y - 1 + first->h) & 0xFF00) >> 8);
		paset_dual[4] = (((first->y - 1 + first->h) & 0xFF));
		/* skip the MPU setting byte */
		paset_dual[6] = (((second->y) & 0xFF00) >> 8);
		paset_dual[7] = (((second->y) & 0xFF));
		paset_dual[8] = (((second->y - 1 + second->h) & 0xFF00) >> 8);
		paset_dual[9] = (((second->y - 1 + second->h) & 0xFF));
		set_dual_col_page_addr_cmd[1].payload = paset_dual;
	} else {
		caset[1] = (((roi->x) & 0xFF00) >> 8);
		caset[2] = (((roi->x) & 0xFF));
		caset[3] = (((roi->x - 1 + roi->w) & 0xFF00) >> 8);
		caset[4] = (((roi->x - 1 + roi->w) & 0xFF));
		set_col_page_addr_cmd[0].payload = caset;

		paset[1] = (((roi->y) & 0xFF00) >> 8);
		paset[2] = (((roi->y) & 0xFF));
		paset[3] = (((roi->y - 1 + roi->h) & 0xFF00) >> 8);
		paset[4] = (((roi->y - 1 + roi->h) & 0xFF));
		set_col_page_addr_cmd[1].payload = paset;
	}
	pr_debug("%s Sending 2A 2B cmnd with dual_roi=%d\n", __func__,
			dual_roi);
}

static void mdss_dsi_send_col_page_addr(struct mdss_dsi_ctrl_pdata *ctrl,
				struct mdss_rect *roi, int unicast)
{
	struct dcs_cmd_req cmdreq;
	struct mdss_panel_info *pinfo = &ctrl->panel_data.panel_info;
	bool dual_roi = pinfo->dual_roi.enabled;

	__mdss_dsi_send_col_page_addr(ctrl, roi, dual_roi);

	memset(&cmdreq, 0, sizeof(cmdreq));
	cmdreq.cmds_cnt = 2;
	cmdreq.flags = CMD_REQ_COMMIT | CMD_CLK_CTRL;
	if (unicast)
		cmdreq.flags |= CMD_REQ_UNICAST;
	cmdreq.rlen = 0;
	cmdreq.cb = NULL;

	/* Send default or dual roi 2A/2B cmd */
	cmdreq.cmds = dual_roi ? set_dual_col_page_addr_cmd :
		set_col_page_addr_cmd;
	mdss_dsi_cmdlist_put(ctrl, &cmdreq);
}

static int mdss_dsi_set_col_page_addr(struct mdss_panel_data *pdata,
		bool force_send)
{
	struct mdss_panel_info *pinfo;
	struct mdss_rect roi = {0};
	struct mdss_rect *p_roi;
	struct mdss_rect *c_roi;
	struct mdss_dsi_ctrl_pdata *ctrl = NULL;
	struct mdss_dsi_ctrl_pdata *other = NULL;
	int left_or_both = 0;

	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	ctrl = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);

	pinfo = &pdata->panel_info;
	p_roi = &pinfo->roi;

	/*
	 * to avoid keep sending same col_page info to panel,
	 * if roi_merge enabled, the roi of left ctrl is used
	 * to compare against new merged roi and saved new
	 * merged roi to it after comparing.
	 * if roi_merge disabled, then the calling ctrl's roi
	 * and pinfo's roi are used to compare.
	 */
	if (pinfo->partial_update_roi_merge) {
		left_or_both = mdss_dsi_roi_merge(ctrl, &roi);
		other = mdss_dsi_get_ctrl_by_index(DSI_CTRL_LEFT);
		c_roi = &other->roi;
	} else {
		c_roi = &ctrl->roi;
		roi = *p_roi;
	}

	/* roi had changed, do col_page update */
	if (force_send || !mdss_rect_cmp(c_roi, &roi)) {
		pr_debug("%s: ndx=%d x=%d y=%d w=%d h=%d\n",
				__func__, ctrl->ndx, p_roi->x,
				p_roi->y, p_roi->w, p_roi->h);

		*c_roi = roi; /* keep to ctrl */
		if (c_roi->w == 0 || c_roi->h == 0) {
			/* no new frame update */
			pr_debug("%s: ctrl=%d, no partial roi set\n",
						__func__, ctrl->ndx);
			return 0;
		}

		if (pinfo->dcs_cmd_by_left) {
			if (left_or_both && ctrl->ndx == DSI_CTRL_RIGHT) {
				/* 2A/2B sent by left already */
				return 0;
			}
		}

		if (!mdss_dsi_sync_wait_enable(ctrl)) {
			if (pinfo->dcs_cmd_by_left)
				ctrl = mdss_dsi_get_ctrl_by_index(
							DSI_CTRL_LEFT);
			mdss_dsi_send_col_page_addr(ctrl, &roi, 0);
		} else {
			/*
			 * when sync_wait_broadcast enabled,
			 * need trigger at right ctrl to
			 * start both dcs cmd transmission
			 */
			other = mdss_dsi_get_other_ctrl(ctrl);
			if (!other)
				goto end;

			if (mdss_dsi_is_left_ctrl(ctrl)) {
				if (pinfo->partial_update_roi_merge) {
					/*
					 * roi is the one after merged
					 * to dsi-1 only
					 */
					mdss_dsi_send_col_page_addr(other,
							&roi, 0);
				} else {
					mdss_dsi_send_col_page_addr(ctrl,
							&ctrl->roi, 1);
					mdss_dsi_send_col_page_addr(other,
							&other->roi, 1);
				}
			} else {
				if (pinfo->partial_update_roi_merge) {
					/*
					 * roi is the one after merged
					 * to dsi-1 only
					 */
					mdss_dsi_send_col_page_addr(ctrl,
							&roi, 0);
				} else {
					mdss_dsi_send_col_page_addr(other,
							&other->roi, 1);
					mdss_dsi_send_col_page_addr(ctrl,
							&ctrl->roi, 1);
				}
			}
		}
	}

end:
	return 0;
}

static int mdss_dsi_panel_apply_display_setting(struct mdss_panel_data
					*pdata __attribute__((unused)),
					u32 mode __attribute__((unused)))
{
	pr_err("somc_panel: Tried to set LP commands, but "
		"persistence mode is NOT IMPLEMENTED.\n");
	pr_err("somc_panel: Returning 0 to avoid kernel crash!!!\n");
	return 0;
}

static void mdss_dsi_panel_switch_mode(struct mdss_panel_data *pdata,
							int mode)
{
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	struct mipi_panel_info *mipi;
	struct dsi_panel_cmds *pcmds;
	u32 flags = 0;

	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return;
	}

	mipi  = &pdata->panel_info.mipi;

	if (!mipi->dms_mode)
		return;

	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);

	if (mipi->dms_mode != DYNAMIC_MODE_RESOLUTION_SWITCH_IMMEDIATE) {
		if (mode == SWITCH_TO_CMD_MODE)
			pcmds = &ctrl_pdata->video2cmd;
		else
			pcmds = &ctrl_pdata->cmd2video;
	} else if ((mipi->dms_mode ==
				DYNAMIC_MODE_RESOLUTION_SWITCH_IMMEDIATE)
			&& pdata->current_timing
			&& !list_empty(&pdata->timings_list)) {
		struct dsi_panel_timing *pt;

		pt = container_of(pdata->current_timing,
				struct dsi_panel_timing, timing);

		pr_debug("%s: sending switch commands\n", __func__);
		pcmds = &pt->switch_cmds;
		flags |= CMD_REQ_DMA_TPG;
		mipi->switch_mode_pending = true;
	} else {
		pr_warn("%s: Invalid mode switch attempted\n", __func__);
		return;
	}

	if ((pdata->panel_info.compression_mode == COMPRESSION_DSC) &&
			(mipi->switch_mode_pending == true))
		mdss_dsi_panel_dsc_pps_send(ctrl_pdata, &pdata->panel_info);

	__mdss_dsi_panel_cmds_send(ctrl_pdata, pcmds, flags);

	return;
}

static void mdss_dsi_panel_bl_ctrl(struct mdss_panel_data *pdata,
							u32 bl_level)
{
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	struct somc_panel_color_mgr *color_mgr = NULL;
	struct mdss_dsi_ctrl_pdata *sctrl = NULL;

	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return;
	}

	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);
	color_mgr = ctrl_pdata->spec_pdata->color_mgr;

	if (color_mgr->picadj_data.flags & MDP_PP_OPS_ENABLE)
		color_mgr->picadj_setup(pdata);

	/*
	 * Some backlight controllers specify a minimum duty cycle
	 * for the backlight brightness. If the brightness is less
	 * than it, the controller can malfunction.
	 */

	if ((bl_level < pdata->panel_info.bl_min) && (bl_level != 0))
		bl_level = pdata->panel_info.bl_min;

	switch (ctrl_pdata->bklt_ctrl) {
	case BL_WLED:
		led_trigger_event(bl_led_trigger, bl_level);
		break;
	case BL_PWM:
		mdss_dsi_panel_bklt_pwm(ctrl_pdata, bl_level);
		break;
	case BL_DCS_CMD:
		if (!mdss_dsi_sync_wait_enable(ctrl_pdata)) {
			mdss_dsi_panel_bklt_dcs(ctrl_pdata, bl_level);
			break;
		}
		/*
		 * DCS commands to update backlight are usually sent at
		 * the same time to both the controllers. However, if
		 * sync_wait is enabled, we need to ensure that the
		 * dcs commands are first sent to the non-trigger
		 * controller so that when the commands are triggered,
		 * both controllers receive it at the same time.
		 */
		sctrl = mdss_dsi_get_other_ctrl(ctrl_pdata);
		if (mdss_dsi_sync_wait_trigger(ctrl_pdata)) {
			if (sctrl)
				mdss_dsi_panel_bklt_dcs(sctrl, bl_level);
			mdss_dsi_panel_bklt_dcs(ctrl_pdata, bl_level);
		} else {
			mdss_dsi_panel_bklt_dcs(ctrl_pdata, bl_level);
			if (sctrl)
				mdss_dsi_panel_bklt_dcs(sctrl, bl_level);
		}
		break;
	default:
		pr_err("%s: Unknown bl_ctrl configuration\n",
			__func__);
		break;
	}
}

static ssize_t mdss_dsi_panel_id_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = dev_get_drvdata(dev);
	char const *id = ctrl_pdata->panel_data.panel_info.panel_id_name ?
		ctrl_pdata->panel_data.panel_info.panel_id_name : "default";

	return scnprintf(buf, PAGE_SIZE, "%s\n", id);
}

static struct device_attribute panel_attributes[] = {
	__ATTR(panel_id, S_IRUSR, mdss_dsi_panel_id_show, NULL),
};

static int register_attributes(struct device *dev)
{
	int i, rc = 0;
	for (i = 0; i < ARRAY_SIZE(panel_attributes); i++)
		if (device_create_file(dev, panel_attributes + i))
			goto error;

	rc = somc_panel_fps_register_attr(dev);
	if (unlikely(rc != 0))
		goto end;

	rc = somc_panel_colormgr_register_attr(dev);
	if (unlikely(rc != 0))
		goto end;
end:
	return rc;
error:
	dev_err(dev, "%s: Unable to create interface\n", __func__);
	for (--i; i >= 0 ; i--)
		device_remove_file(dev, panel_attributes + i);
	return -ENODEV;
}

static int mdss_dsi_panel_unblank(struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{
	struct somc_panel_color_mgr *color_mgr =
			ctrl_pdata->spec_pdata->color_mgr;
	int rc;

	rc = color_mgr->unblank_hndl(ctrl_pdata);
	if (unlikely(rc != 0))
		pr_err("%s: Color Manager unblanker failed!!\n", __func__);

	return rc;
}

static int mdss_dsi_panel_on(struct mdss_panel_data *pdata)
{
	struct mipi_panel_info *mipi;
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	struct mdss_panel_specific_pdata *spec_pdata = NULL;
	struct mdss_panel_info *pinfo = NULL;

	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);

	pinfo = &ctrl_pdata->panel_data.panel_info;

	spec_pdata = ctrl_pdata->spec_pdata;
	if (!spec_pdata) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	if (!spec_pdata->detected && !spec_pdata->init_from_begin)
		return 0;

	mipi = &pdata->panel_info.mipi;

	lcm_first_boot = 0;

	if (pdata->panel_info.dsi_master != pdata->panel_info.pdest)
		goto end;

	if ((pinfo->mipi.dms_mode == DYNAMIC_MODE_SWITCH_IMMEDIATE) &&
			(pinfo->mipi.boot_mode != pinfo->mipi.mode)) {
		mdss_dsi_panel_cmds_send(ctrl_pdata,
					&ctrl_pdata->post_dms_on_cmds);
		goto chg_fps;
	}

	if (spec_pdata->einit_cmds.cmd_cnt) {
		pr_debug("%s: early init sequence\n", __func__);
		mdss_dsi_panel_cmds_send(ctrl_pdata, &spec_pdata->einit_cmds);
		//mdss_dsi_panel_reset(pdata, 1);
	}

	if (spec_pdata->init_cmds.cmd_cnt) {
		pr_debug("%s: init (exit sleep) sequence\n", __func__);
		mdss_dsi_panel_cmds_send(ctrl_pdata, &spec_pdata->init_cmds);
	}

	if (ctrl_pdata->on_cmds.cmd_cnt && !pinfo->disp_on_in_hs) {
		pr_debug("%s: panel on sequence (in low speed)\n", __func__);
		mdss_dsi_panel_cmds_send(ctrl_pdata, &ctrl_pdata->on_cmds);
		spec_pdata->disp_onoff_state = true;
		pr_info("%s: ctrl=%p ndx=%d\n", __func__,
					ctrl_pdata, ctrl_pdata->ndx);

		poll_worker_schedule(ctrl_pdata);
	}

chg_fps:
#ifndef CONFIG_FBDEV_SOMC_PANEL_INCELL
	somc_panel_chg_fps_cmds_send(ctrl_pdata);
#endif

	if (pinfo->compression_mode == COMPRESSION_DSC)
		mdss_dsi_panel_dsc_pps_send(ctrl_pdata, pinfo);

	if (ctrl_pdata->ds_registered && pinfo->is_pluggable)
		mdss_dba_utils_video_on(pinfo->dba_data, pinfo);

end:
#ifdef CONFIG_MACH_SONY_YUKON
	spec_pdata->disp_on(pdata);
#endif
	pr_debug("%s:-\n", __func__);
	return 0;
}

static int mdss_dsi_post_panel_on(struct mdss_panel_data *pdata)
{
	struct mdss_dsi_ctrl_pdata *ctrl = NULL;
	struct mdss_panel_specific_pdata *specific = NULL;
	struct mdss_panel_info *pinfo;
	struct dsi_panel_cmds *on_cmds;
	u32 vsync_period = 0;
	int rc = 0;

	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	ctrl = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);

	specific = ctrl->spec_pdata;

	pr_debug("%s: ctrl=%p ndx=%d\n", __func__, ctrl, ctrl->ndx);

	pinfo = &pdata->panel_info;
	if (pinfo->dcs_cmd_by_left) {
		if (ctrl->ndx != DSI_CTRL_LEFT)
			goto end;
	}

	on_cmds = &ctrl->post_panel_on_cmds;

	pr_debug("%s: ctrl=%p cmd_cnt=%d\n", __func__, ctrl, on_cmds->cmd_cnt);

	if (on_cmds->cmd_cnt) {
		msleep(VSYNC_DELAY);	/* wait for 1 vsync passed */
		mdss_dsi_panel_cmds_send(ctrl, on_cmds);
	}

	if (pinfo->is_dba_panel && pinfo->is_pluggable) {
		/* ensure at least 1 frame transfers to down stream device */
		vsync_period = (MSEC_PER_SEC / pinfo->mipi.frame_rate) + 1;
		msleep(vsync_period);
		mdss_dba_utils_hdcp_enable(pinfo->dba_data, true);
	}

	/* NOTE: Any debugging message must be shown from specific function. */
	if (specific->panel_post_on) {
		rc = specific->panel_post_on(pdata);
		if (rc)
			return rc;
	}

	if (pdata->panel_info.pdest == DISPLAY_1)
		somc_panel_fpsman_panel_post_on(ctrl);

end:
	pr_debug("%s:-\n", __func__);
	return rc;
}

static int mdss_dsi_panel_off(struct mdss_panel_data *pdata)
{
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	struct mdss_panel_specific_pdata *spec_pdata = NULL;
	struct mdss_panel_info *pinfo = NULL;

	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	pinfo = &pdata->panel_info;
	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);

	spec_pdata = ctrl_pdata->spec_pdata;
	if (!spec_pdata) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	if (spec_pdata->dsi_panel_off_ex)
		spec_pdata->dsi_panel_off_ex(pdata);

	poll_worker_cancel(ctrl_pdata);

	if (!spec_pdata->detected) {
		spec_pdata->detected = true;
		if (!spec_pdata->init_from_begin)
			return 0;
	}

	if (pinfo->dcs_cmd_by_left) {
		if (ctrl_pdata->ndx != DSI_CTRL_LEFT)
			goto end;
	}

	pr_info("%s: ctrl=%p ndx=%d\n", __func__, ctrl_pdata, ctrl_pdata->ndx);

	if (ctrl_pdata->off_cmds.cmd_cnt) {
		mdss_dsi_panel_cmds_send(ctrl_pdata,
					&ctrl_pdata->off_cmds);
	}

	if ((spec_pdata->new_vfp) &&
		(ctrl_pdata->panel_data.panel_info.lcdc.v_front_porch !=
			spec_pdata->new_vfp))
		ctrl_pdata->panel_data.panel_info.lcdc.v_front_porch =
			spec_pdata->new_vfp;

	if (ctrl_pdata->ds_registered && pinfo->is_pluggable) {
		mdss_dba_utils_video_off(pinfo->dba_data);
		mdss_dba_utils_hdcp_enable(pinfo->dba_data, false);
	}

	if (pdata->panel_info.dsi_master != pdata->panel_info.pdest)
		goto skip_offsequence;

	somc_panel_fpsman_panel_off();

#ifndef CONFIG_FBDEV_SOMC_PANEL_INCELL
	mdss_dsi_panel_reset(pdata, 0);
#endif

end:
	spec_pdata->disp_onoff_state = false;
	pdata->resume_started = true;

skip_offsequence:
	pr_debug("%s: Done\n", __func__);

	return 0;
}

static int mdss_dsi_panel_low_power_config(struct mdss_panel_data *pdata,
	int enable)
{
	struct mdss_dsi_ctrl_pdata *ctrl = NULL;
	struct mdss_panel_info *pinfo;

	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	pinfo = &pdata->panel_info;
	ctrl = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);

	pr_debug("%s: ctrl=%p ndx=%d enable=%d\n", __func__, ctrl, ctrl->ndx,
		enable);

	/* Any panel specific low power commands/config */

	pr_debug("%s:-\n", __func__);
	return 0;
}

static int mdss_dsi_panel_disp_on(struct mdss_panel_data *pdata)
{
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	struct mdss_panel_specific_pdata *spec_pdata = NULL;
	struct mdss_panel_info *pinfo = NULL;

	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);

	pinfo = &pdata->panel_info;
	spec_pdata = ctrl_pdata->spec_pdata;
	if (!spec_pdata) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	if ((!spec_pdata->detected) ||
	    (pdata->panel_info.dsi_master != pdata->panel_info.pdest))
		return 0;

	if (pinfo->dcs_cmd_by_left &&
	    (ctrl_pdata->ndx != DSI_CTRL_LEFT))
		return 0;

	if (ctrl_pdata->on_cmds.cmd_cnt && pinfo->disp_on_in_hs) {
		pr_debug("%s: panel on sequence (in high speed)\n", __func__);
		mdss_dsi_set_tx_power_mode(0, pdata);
		mdss_dsi_panel_cmds_send(ctrl_pdata, &ctrl_pdata->on_cmds);
		spec_pdata->disp_onoff_state = true;
		pr_info("%s: ctrl=%p ndx=%d\n", __func__, ctrl_pdata, ctrl_pdata->ndx);

		poll_worker_schedule(ctrl_pdata);
	}

	pr_debug("%s: done\n", __func__);

	return 0;
}

void somc_panel_down_period_quirk(struct mdss_panel_specific_pdata *spec_pdata)
{
	if (spec_pdata->down_period) {
		u32 kt = (u32)ktime_to_ms(ktime_get());
		kt = (kt < spec_pdata->current_period) ?
			(kt + ~spec_pdata->current_period) :
			(kt - spec_pdata->current_period);
		if (kt < spec_pdata->down_period)
			usleep_range((spec_pdata->down_period - kt) *
				1000,
				(spec_pdata->down_period - kt) *
				1000 + 100);
	}
}

static inline int mdss_dsi_panel_power_lp_ex(struct mdss_panel_data *pdata,
								int enable)
{
	/* TODO: Implement LP1 and LP2 switching for supported panels */
	return 0;
}

static int mdss_dsi_panel_power_ctrl_ex(struct mdss_panel_data *pdata, int enable)
{
	int ret = 0;
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	struct mdss_panel_specific_pdata *spec_pdata = NULL;
	struct mdss_panel_info *pinfo = NULL;

	pr_debug("%s: enable=%d\n", __func__, enable);
	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	/*
	 * If a dynamic mode switch is pending while onlining the panel
	 * the regulators should not be turned off or on.
	 */
	if (pdata->panel_info.dynamic_switch_pending)
		return 0;

	pinfo = &pdata->panel_info;

	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);

	spec_pdata = ctrl_pdata->spec_pdata;
	if (!spec_pdata) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	switch (enable) {
	case MDSS_PANEL_POWER_OFF:
	case MDSS_PANEL_POWER_LCD_DISABLED:
                /* if LCD has not been disabled, then disable it now */
                if ((pinfo->panel_power_state != MDSS_PANEL_POWER_LCD_DISABLED)
                     && (pinfo->panel_power_state != MDSS_PANEL_POWER_OFF))
			ret = spec_pdata->panel_power_off(pdata);
		break;
	case MDSS_PANEL_POWER_ON:
		if (mdss_dsi_is_panel_on_lp(pdata))
			ret = mdss_dsi_panel_power_lp_ex(pdata, false);
		else
			ret = spec_pdata->panel_power_on(pdata);
		break;
	case MDSS_PANEL_POWER_LP1:
	case MDSS_PANEL_POWER_LP2:
		ret = mdss_dsi_panel_power_lp_ex(pdata, true);
		break;
	default:
		pr_err("%s: unknown panel power state requested (%d)\n",
			__func__, enable);
	}
#ifndef CONFIG_FBDEV_SOMC_PANEL_INCELL
	if (ret) {
		msm_dss_enable_vreg(
			ctrl_pdata->panel_power_data.vreg_config,
			ctrl_pdata->panel_power_data.num_vreg, 0);
	}
#endif
	if (!ret)
		pinfo->panel_power_state = enable;

	return ret;
}

static void mdss_dsi_parse_mdp_kickoff_threshold(struct device_node *np,
	struct mdss_panel_info *pinfo)
{
	int len, rc;
	const u32 *src;
	u32 tmp;
	u32 max_delay_us;

	pinfo->mdp_koff_thshold = false;
	src = of_get_property(np, "qcom,mdss-mdp-kickoff-threshold", &len);
	if (!src || (len == 0))
		return;

	rc = of_property_read_u32(np, "qcom,mdss-mdp-kickoff-delay", &tmp);
	if (!rc)
		pinfo->mdp_koff_delay = tmp;
	else
		return;

	if (pinfo->mipi.frame_rate == 0) {
		pr_err("cannot enable guard window, unexpected panel fps\n");
		return;
	}

	pinfo->mdp_koff_thshold_low = be32_to_cpu(src[0]);
	pinfo->mdp_koff_thshold_high = be32_to_cpu(src[1]);
	max_delay_us = 1000000 / pinfo->mipi.frame_rate;

	/* enable the feature if threshold is valid */
	if ((pinfo->mdp_koff_thshold_low < pinfo->mdp_koff_thshold_high) &&
	   ((pinfo->mdp_koff_delay > 0) ||
	    (pinfo->mdp_koff_delay < max_delay_us)))
		pinfo->mdp_koff_thshold = true;

	pr_debug("panel kickoff thshold:[%d, %d] delay:%d (max:%d) enable:%d\n",
		pinfo->mdp_koff_thshold_low,
		pinfo->mdp_koff_thshold_high,
		pinfo->mdp_koff_delay,
		max_delay_us,
		pinfo->mdp_koff_thshold);
}

static void mdss_dsi_parse_trigger(struct device_node *np, char *trigger,
		char *trigger_key)
{
	const char *data;

	*trigger = DSI_CMD_TRIGGER_SW;
	data = of_get_property(np, trigger_key, NULL);
	if (data) {
		if (!strcmp(data, "none"))
			*trigger = DSI_CMD_TRIGGER_NONE;
		else if (!strcmp(data, "trigger_te"))
			*trigger = DSI_CMD_TRIGGER_TE;
		else if (!strcmp(data, "trigger_sw_seof"))
			*trigger = DSI_CMD_TRIGGER_SW_SEOF;
		else if (!strcmp(data, "trigger_sw_te"))
			*trigger = DSI_CMD_TRIGGER_SW_TE;
	}
}

static int mdss_dsi_parse_dcs_cmds(struct device_node *np,
		struct dsi_panel_cmds *pcmds, char *cmd_key, char *link_key)
{
	const char *data;
	int blen = 0, len;
	char *buf, *bp;
	struct dsi_ctrl_hdr *dchdr;
	int i, cnt;

	data = of_get_property(np, cmd_key, &blen);
	if (!data) {
		pr_err("%s: Cannot find property:  %s\n", __func__, cmd_key);
		return -ENOMEM;
	}

	buf = kzalloc(sizeof(char) * blen, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	memcpy(buf, data, blen);

	/* scan dcs commands */
	bp = buf;
	len = blen;
	cnt = 0;
	while (len >= sizeof(*dchdr)) {
		dchdr = (struct dsi_ctrl_hdr *)bp;
		dchdr->dlen = ntohs(dchdr->dlen);
		if (dchdr->dlen > len) {
			pr_err("%s: dtsi cmd=%x error, len=%d",
				__func__, dchdr->dtype, dchdr->dlen);
			goto exit_free;
		}
		bp += sizeof(*dchdr);
		len -= sizeof(*dchdr);
		bp += dchdr->dlen;
		len -= dchdr->dlen;
		cnt++;
	}

	if (len != 0) {
		pr_err("%s: dcs_cmd=%x len=%d error!",
				__func__, buf[0], blen);
		goto exit_free;
	}

	pcmds->cmds = kzalloc(cnt * sizeof(struct dsi_cmd_desc),
						GFP_KERNEL);
	if (!pcmds->cmds)
		goto exit_free;

	pcmds->cmd_cnt = cnt;
	pcmds->buf = buf;
	pcmds->blen = blen;

	bp = buf;
	len = blen;
	for (i = 0; i < cnt; i++) {
		dchdr = (struct dsi_ctrl_hdr *)bp;
		len -= sizeof(*dchdr);
		bp += sizeof(*dchdr);
		pcmds->cmds[i].dchdr = *dchdr;
		pcmds->cmds[i].payload = bp;
		bp += dchdr->dlen;
		len -= dchdr->dlen;
	}

	/*Set default link state to LP Mode*/
	pcmds->link_state = DSI_LP_MODE;

	if (link_key) {
		data = of_get_property(np, link_key, NULL);
		if (data && !strcmp(data, "dsi_hs_mode"))
			pcmds->link_state = DSI_HS_MODE;
		else
			pcmds->link_state = DSI_LP_MODE;
	}

	pr_debug("%s: dcs_cmd=%x len=%d, cmd_cnt=%d link_state=%d\n", __func__,
		pcmds->buf[0], pcmds->blen, pcmds->cmd_cnt, pcmds->link_state);

	return 0;

exit_free:
	kfree(buf);
	return -ENOMEM;
}

int somc_panel_parse_dcs_cmds(struct device_node *np,
		struct dsi_panel_cmds *pcmds, char *cmd_key, char *link_key)
{
	return mdss_dsi_parse_dcs_cmds(np, pcmds, cmd_key, link_key);
}

int mdss_dsi_property_read_u32_var(struct device_node *np,
		char *name, u32 **out_data, int *num)
{
	struct property *prop = of_find_property(np, name, NULL);
	const __be32 *val;
	u32 *out;
	int s;

	if (!prop) {
		pr_debug("%s:%d, unable to read %s", __func__, __LINE__, name);
		return -EINVAL;
	}
	if (!prop->value) {
		pr_debug("%s:%d, no data of %s", __func__, __LINE__, name);
		return -ENODATA;
	}

	*num = prop->length / sizeof(u32);
	if (!*num || *num % 2) {
		pr_debug("%s:%d, error reading %s, length found = %d\n",
			__func__, __LINE__, name, *num);
		return -ENODATA;
	}
	*out_data = kzalloc(prop->length, GFP_KERNEL);
	if (!*out_data) {
		pr_err("%s:no mem assigned: kzalloc fail\n", __func__);
		*num = 0;
		return -ENOMEM;
	}

	val = prop->value;
	out = *out_data;
	s = *num;
	while (s--)
		*out++ = be32_to_cpup(val++);
	return 0;
}

int mdss_panel_get_dst_fmt(u32 bpp, char mipi_mode, u32 pixel_packing,
				char *dst_format)
{
	int rc = 0;
	switch (bpp) {
	case 3:
		*dst_format = DSI_CMD_DST_FORMAT_RGB111;
		break;
	case 8:
		*dst_format = DSI_CMD_DST_FORMAT_RGB332;
		break;
	case 12:
		*dst_format = DSI_CMD_DST_FORMAT_RGB444;
		break;
	case 16:
		switch (mipi_mode) {
		case DSI_VIDEO_MODE:
			*dst_format = DSI_VIDEO_DST_FORMAT_RGB565;
			break;
		case DSI_CMD_MODE:
			*dst_format = DSI_CMD_DST_FORMAT_RGB565;
			break;
		default:
			*dst_format = DSI_VIDEO_DST_FORMAT_RGB565;
			break;
		}
		break;
	case 18:
		switch (mipi_mode) {
		case DSI_VIDEO_MODE:
			if (pixel_packing == 0)
				*dst_format = DSI_VIDEO_DST_FORMAT_RGB666;
			else
				*dst_format = DSI_VIDEO_DST_FORMAT_RGB666_LOOSE;
			break;
		case DSI_CMD_MODE:
			*dst_format = DSI_CMD_DST_FORMAT_RGB666;
			break;
		default:
			if (pixel_packing == 0)
				*dst_format = DSI_VIDEO_DST_FORMAT_RGB666;
			else
				*dst_format = DSI_VIDEO_DST_FORMAT_RGB666_LOOSE;
			break;
		}
		break;
	case 24:
		switch (mipi_mode) {
		case DSI_VIDEO_MODE:
			*dst_format = DSI_VIDEO_DST_FORMAT_RGB888;
			break;
		case DSI_CMD_MODE:
			*dst_format = DSI_CMD_DST_FORMAT_RGB888;
			break;
		default:
			*dst_format = DSI_VIDEO_DST_FORMAT_RGB888;
			break;
		}
		break;
	default:
		rc = -EINVAL;
		break;
	}
	return rc;
}

static int mdss_dsi_parse_fbc_params(struct device_node *np,
			struct mdss_panel_timing *timing)
{
	int rc, fbc_enabled = 0;
	u32 tmp;
	struct fbc_panel_info *fbc = &timing->fbc;

	fbc_enabled = of_property_read_bool(np,	"qcom,mdss-dsi-fbc-enable");
	if (fbc_enabled) {
		pr_debug("%s:%d FBC panel enabled.\n", __func__, __LINE__);
		fbc->enabled = 1;
		rc = of_property_read_u32(np, "qcom,mdss-dsi-fbc-bpp", &tmp);
		fbc->target_bpp =	(!rc ? tmp : 24);
		rc = of_property_read_u32(np, "qcom,mdss-dsi-fbc-packing",
				&tmp);
		fbc->comp_mode = (!rc ? tmp : 0);
		fbc->qerr_enable = of_property_read_bool(np,
			"qcom,mdss-dsi-fbc-quant-error");
		rc = of_property_read_u32(np, "qcom,mdss-dsi-fbc-bias", &tmp);
		fbc->cd_bias = (!rc ? tmp : 0);
		fbc->pat_enable = of_property_read_bool(np,
				"qcom,mdss-dsi-fbc-pat-mode");
		fbc->vlc_enable = of_property_read_bool(np,
				"qcom,mdss-dsi-fbc-vlc-mode");
		fbc->bflc_enable = of_property_read_bool(np,
				"qcom,mdss-dsi-fbc-bflc-mode");
		rc = of_property_read_u32(np, "qcom,mdss-dsi-fbc-h-line-budget",
				&tmp);
		fbc->line_x_budget = (!rc ? tmp : 0);
		rc = of_property_read_u32(np, "qcom,mdss-dsi-fbc-budget-ctrl",
				&tmp);
		fbc->block_x_budget = (!rc ? tmp : 0);
		rc = of_property_read_u32(np, "qcom,mdss-dsi-fbc-block-budget",
				&tmp);
		fbc->block_budget = (!rc ? tmp : 0);
		rc = of_property_read_u32(np,
				"qcom,mdss-dsi-fbc-lossless-threshold", &tmp);
		fbc->lossless_mode_thd = (!rc ? tmp : 0);
		rc = of_property_read_u32(np,
				"qcom,mdss-dsi-fbc-lossy-threshold", &tmp);
		fbc->lossy_mode_thd = (!rc ? tmp : 0);
		rc = of_property_read_u32(np, "qcom,mdss-dsi-fbc-rgb-threshold",
				&tmp);
		fbc->lossy_rgb_thd = (!rc ? tmp : 0);
		rc = of_property_read_u32(np,
				"qcom,mdss-dsi-fbc-lossy-mode-idx", &tmp);
		fbc->lossy_mode_idx = (!rc ? tmp : 0);
		rc = of_property_read_u32(np,
				"qcom,mdss-dsi-fbc-slice-height", &tmp);
		fbc->slice_height = (!rc ? tmp : 0);
		fbc->pred_mode = of_property_read_bool(np,
				"qcom,mdss-dsi-fbc-2d-pred-mode");
		fbc->enc_mode = of_property_read_bool(np,
				"qcom,mdss-dsi-fbc-ver2-mode");
		rc = of_property_read_u32(np,
				"qcom,mdss-dsi-fbc-max-pred-err", &tmp);
		fbc->max_pred_err = (!rc ? tmp : 0);

		timing->compression_mode = COMPRESSION_FBC;
	} else {
		pr_debug("%s:%d Panel does not support FBC.\n",
				__func__, __LINE__);
		fbc->enabled = 0;
		fbc->target_bpp = 24;
	}
	return 0;
}

void mdss_dsi_panel_dsc_pps_send(struct mdss_dsi_ctrl_pdata *ctrl,
				struct mdss_panel_info *pinfo)
{
	struct dsi_panel_cmds pcmds;
	struct dsi_cmd_desc cmd;

	if (!pinfo || (pinfo->compression_mode != COMPRESSION_DSC))
		return;

	memset(&pcmds, 0, sizeof(pcmds));
	memset(&cmd, 0, sizeof(cmd));

	cmd.dchdr.dlen = mdss_panel_dsc_prepare_pps_buf(&pinfo->dsc,
				ctrl->pps_buf, 0);
	cmd.dchdr.dtype = DTYPE_PPS;
	cmd.dchdr.last = 1;
	cmd.dchdr.wait = 10;
	cmd.dchdr.vc = 0;
	cmd.dchdr.ack = 0;
	cmd.payload = ctrl->pps_buf;

	pcmds.cmd_cnt = 1;
	pcmds.cmds = &cmd;
	pcmds.link_state = DSI_LP_MODE;

	mdss_dsi_panel_cmds_send(ctrl, &pcmds);
}

static int mdss_dsi_parse_hdr_settings(struct device_node *np,
		struct mdss_panel_info *pinfo)
{
	int rc = 0;
	struct mdss_panel_hdr_properties *hdr_prop;

	if (!np) {
		pr_err("%s: device node pointer is NULL\n", __func__);
		return -EINVAL;
	}

	if (!pinfo) {
		pr_err("%s: panel info is NULL\n", __func__);
		return -EINVAL;
	}

	hdr_prop = &pinfo->hdr_properties;
	hdr_prop->hdr_enabled = of_property_read_bool(np,
		"qcom,mdss-dsi-panel-hdr-enabled");

	if (hdr_prop->hdr_enabled) {
		rc = of_property_read_u32_array(np,
				"qcom,mdss-dsi-panel-hdr-color-primaries",
				hdr_prop->display_primaries,
				DISPLAY_PRIMARIES_COUNT);
		if (rc) {
			pr_info("%s:%d, Unable to read color primaries,rc:%u",
					__func__, __LINE__,
					hdr_prop->hdr_enabled = false);
			}

		rc = of_property_read_u32(np,
			"qcom,mdss-dsi-panel-peak-brightness",
			&(hdr_prop->peak_brightness));
		if (rc) {
			pr_info("%s:%d, Unable to read hdr brightness, rc:%u",
				__func__, __LINE__, rc);
			hdr_prop->hdr_enabled = false;
		}

		rc = of_property_read_u32(np,
			"qcom,mdss-dsi-panel-blackness-level",
			&(hdr_prop->blackness_level));
		if (rc) {
			pr_info("%s:%d, Unable to read hdr brightness, rc:%u",
				__func__, __LINE__, rc);
			hdr_prop->hdr_enabled = false;
		}
	}
	return 0;
}

static int mdss_dsi_parse_dsc_version(struct device_node *np,
		struct mdss_panel_timing *timing)
{
	u32 data;
	int rc = 0;
	struct dsc_desc *dsc = &timing->dsc;

	rc = of_property_read_u32(np, "qcom,mdss-dsc-version", &data);
	if (rc) {
		dsc->version = 0x11;
		rc = 0;
	} else {
		dsc->version = data & 0xff;
		/* only support DSC 1.1 rev */
		if (dsc->version != 0x11) {
			pr_err("%s: DSC version:%d not supported\n", __func__,
				dsc->version);
			rc = -EINVAL;
			goto end;
		}
	}

	rc = of_property_read_u32(np, "qcom,mdss-dsc-scr-version", &data);
	if (rc) {
		dsc->scr_rev = 0x0;
		rc = 0;
	} else {
		dsc->scr_rev = data & 0xff;
		/* only one scr rev supported */
		if (dsc->scr_rev > 0x1) {
			pr_err("%s: DSC scr version:%d not supported\n",
				__func__, dsc->scr_rev);
			rc = -EINVAL;
			goto end;
		}
	}

end:
	return rc;
}

static int mdss_dsi_parse_dsc_params(struct device_node *np,
		struct mdss_panel_timing *timing, bool is_split_display)
{
	u32 data, intf_width;
	int rc = 0;
	struct dsc_desc *dsc = &timing->dsc;

	if (!np) {
		pr_err("%s: device node pointer is NULL\n", __func__);
		return -EINVAL;
	}

	rc = of_property_read_u32(np, "qcom,mdss-dsc-encoders", &data);
	if (rc) {
		if (!of_find_property(np, "qcom,mdss-dsc-encoders", NULL)) {
			/* property is not defined, default to 1 */
			data = 1;
		} else {
			pr_err("%s: Error parsing qcom,mdss-dsc-encoders\n",
				__func__);
			goto end;
		}
	}

	timing->dsc_enc_total = data;

	if (is_split_display && (timing->dsc_enc_total > 1)) {
		pr_err("%s: Error: for split displays, more than 1 dsc encoder per panel is not allowed.\n",
			__func__);
		goto end;
	}

	rc = of_property_read_u32(np, "qcom,mdss-dsc-slice-height", &data);
	if (rc)
		goto end;
	dsc->slice_height = data;

	rc = of_property_read_u32(np, "qcom,mdss-dsc-slice-width", &data);
	if (rc)
		goto end;
	dsc->slice_width = data;
	intf_width = timing->xres;

	if (intf_width % dsc->slice_width) {
		pr_err("%s: Error: multiple of slice-width:%d should match panel-width:%d\n",
			__func__, dsc->slice_width, intf_width);
		goto end;
	}

	data = intf_width / dsc->slice_width;
	if (((timing->dsc_enc_total > 1) && ((data != 2) && (data != 4))) ||
	    ((timing->dsc_enc_total == 1) && (data > 2))) {
		pr_err("%s: Error: max 2 slice per encoder. slice-width:%d should match panel-width:%d dsc_enc_total:%d\n",
			__func__, dsc->slice_width,
			intf_width, timing->dsc_enc_total);
		goto end;
	}

	rc = of_property_read_u32(np, "qcom,mdss-dsc-slice-per-pkt", &data);
	if (rc)
		goto end;
	dsc->slice_per_pkt = data;

	/*
	 * slice_per_pkt can be either 1 or all slices_per_intf
	 */
	if ((dsc->slice_per_pkt > 1) && (dsc->slice_per_pkt !=
			DIV_ROUND_UP(intf_width, dsc->slice_width))) {
		pr_err("Error: slice_per_pkt can be either 1 or all slices_per_intf\n");
		pr_err("%s: slice_per_pkt=%d, slice_width=%d intf_width=%d\n",
			__func__,
			dsc->slice_per_pkt, dsc->slice_width, intf_width);
		rc = -EINVAL;
		goto end;
	}

	pr_debug("%s: num_enc:%d :slice h=%d w=%d s_pkt=%d\n", __func__,
		timing->dsc_enc_total, dsc->slice_height,
		dsc->slice_width, dsc->slice_per_pkt);

	rc = of_property_read_u32(np, "qcom,mdss-dsc-bit-per-component", &data);
	if (rc)
		goto end;
	dsc->bpc = data;

	rc = of_property_read_u32(np, "qcom,mdss-dsc-bit-per-pixel", &data);
	if (rc)
		goto end;
	dsc->bpp = data;

	pr_debug("%s: bpc=%d bpp=%d\n", __func__,
		dsc->bpc, dsc->bpp);

	dsc->block_pred_enable = of_property_read_bool(np,
			"qcom,mdss-dsc-block-prediction-enable");

	dsc->enable_422 = 0;
	dsc->convert_rgb = 1;
	dsc->vbr_enable = 0;

	dsc->config_by_manufacture_cmd = of_property_read_bool(np,
		"qcom,mdss-dsc-config-by-manufacture-cmd");

	mdss_panel_dsc_parameters_calc(&timing->dsc);
	mdss_panel_dsc_pclk_param_calc(&timing->dsc, intf_width);

	timing->dsc.full_frame_slices =
		DIV_ROUND_UP(intf_width, timing->dsc.slice_width);

	timing->compression_mode = COMPRESSION_DSC;

end:
	return rc;
}

static struct device_node *mdss_dsi_panel_get_dsc_cfg_np(
		struct device_node *np, struct mdss_panel_data *panel_data,
		bool default_timing)
{
	struct device_node *dsc_cfg_np = NULL;


	/* Read the dsc config node specified by command line */
	if (default_timing) {
		dsc_cfg_np = of_get_child_by_name(np,
				panel_data->dsc_cfg_np_name);
		if (!dsc_cfg_np)
			pr_warn_once("%s: cannot find dsc config node:%s\n",
				__func__, panel_data->dsc_cfg_np_name);
	}

	/*
	 * Fall back to default from DT as nothing is specified
	 * in command line.
	 */
	if (!dsc_cfg_np && of_find_property(np, "qcom,config-select", NULL)) {
		dsc_cfg_np = of_parse_phandle(np, "qcom,config-select", 0);
		if (!dsc_cfg_np)
			pr_warn_once("%s:err parsing qcom,config-select\n",
					__func__);
	}

	return dsc_cfg_np;
}

static int mdss_dsi_parse_topology_config(struct device_node *np,
	struct dsi_panel_timing *pt, struct mdss_panel_data *panel_data,
	bool default_timing)
{
	int rc = 0;
	bool is_split_display = panel_data->panel_info.is_split_display;
	const char *data;
	struct mdss_panel_timing *timing = &pt->timing;
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	struct mdss_panel_info *pinfo;
	struct device_node *cfg_np = NULL;

	ctrl_pdata = container_of(panel_data, struct mdss_dsi_ctrl_pdata,
							panel_data);

	pinfo = &ctrl_pdata->panel_data.panel_info;

	cfg_np = mdss_dsi_panel_get_dsc_cfg_np(np,
				&ctrl_pdata->panel_data, default_timing);

	if (cfg_np) {
		if (!of_property_read_u32_array(cfg_np, "qcom,lm-split",
		    timing->lm_widths, 2)) {
			if (mdss_dsi_is_hw_config_split(ctrl_pdata->shared_data)
			    && (timing->lm_widths[1] != 0)) {
				pr_err("%s: lm-split not allowed with split display\n",
					__func__);
				rc = -EINVAL;
				goto end;
			}
		}
		rc = of_property_read_string(cfg_np, "qcom,split-mode", &data);
		if (!rc && !strcmp(data, "pingpong-split"))
			pinfo->use_pingpong_split = true;

		if (((timing->lm_widths[0]) || (timing->lm_widths[1])) &&
		    pinfo->use_pingpong_split) {
			pr_err("%s: pingpong_split cannot be used when lm-split[%d,%d] is specified\n",
				__func__,
				timing->lm_widths[0], timing->lm_widths[1]);
			return -EINVAL;
		}

		pr_info("%s: cfg_node name %s lm_split:%dx%d pp_split:%s\n",
			__func__, cfg_np->name,
			timing->lm_widths[0], timing->lm_widths[1],
			pinfo->use_pingpong_split ? "yes" : "no");
	}

	if (!pinfo->use_pingpong_split &&
	    (timing->lm_widths[0] == 0) && (timing->lm_widths[1] == 0))
		timing->lm_widths[0] = pt->timing.xres;

	data = of_get_property(np, "qcom,compression-mode", NULL);
	if (data) {
		if (cfg_np && !strcmp(data, "dsc")) {
			rc = mdss_dsi_parse_dsc_version(np, &pt->timing);
			if (rc)
				goto end;

			pinfo->send_pps_before_switch =
				of_property_read_bool(np,
				"qcom,mdss-dsi-send-pps-before-switch");

			rc = mdss_dsi_parse_dsc_params(cfg_np, &pt->timing,
					is_split_display);
		} else if (!strcmp(data, "fbc")) {
			rc = mdss_dsi_parse_fbc_params(np, &pt->timing);
		}
	}

end:
	of_node_put(cfg_np);
	return rc;
}

static void mdss_panel_parse_te_params(struct device_node *np,
			struct mdss_panel_timing *timing)
{
	struct mdss_mdp_pp_tear_check *te = &timing->te;
	u32 tmp;
	int rc = 0;
	/*
	 * TE default: dsi byte clock calculated base on 70 fps;
	 * around 14 ms to complete a kickoff cycle if te disabled;
	 * vclk_line base on 60 fps; write is faster than read;
	 * init == start == rdptr;
	 */
	te->tear_check_en =
		!of_property_read_bool(np, "qcom,mdss-tear-check-disable");
	rc = of_property_read_u32
		(np, "qcom,mdss-tear-check-sync-cfg-height", &tmp);
	te->sync_cfg_height = (!rc ? tmp : 0xfff0);
	rc = of_property_read_u32
		(np, "qcom,mdss-tear-check-sync-init-val", &tmp);
	te->vsync_init_val = (!rc ? tmp : timing->yres);
	rc = of_property_read_u32
		(np, "qcom,mdss-tear-check-sync-threshold-start", &tmp);
	te->sync_threshold_start = (!rc ? tmp : 4);
	rc = of_property_read_u32
		(np, "qcom,mdss-tear-check-sync-threshold-continue", &tmp);
	te->sync_threshold_continue = (!rc ? tmp : 4);
	rc = of_property_read_u32(np, "qcom,mdss-tear-check-frame-rate", &tmp);
	te->refx100 = (!rc ? tmp : 6000);
	rc = of_property_read_u32(np, "qcom,mdss-tear-check-start-pos", &tmp);
	//te->start_pos = (!rc ? tmp : te->vsync_init_val);
	te->start_pos = (!rc ? tmp : timing->yres);
	rc = of_property_read_u32
		(np, "qcom,mdss-tear-check-rd-ptr-trigger-intr", &tmp);
	//te->rd_ptr_irq = (!rc ? tmp : te->vsync_init_val + 1);
	te->rd_ptr_irq = (!rc ? tmp : timing->yres + 1);
	te->wr_ptr_irq = 0;
}


static int mdss_dsi_parse_reset_seq(struct device_node *np,
		u32 rst_seq[MDSS_DSI_RST_SEQ_LEN], u32 *rst_len,
		const char *name)
{
	int num = 0, i;
	int rc;
	struct property *data;
	u32 tmp[MDSS_DSI_RST_SEQ_LEN];
	*rst_len = 0;
	data = of_find_property(np, name, &num);
	num /= sizeof(u32);
	if (!data || !num || num > MDSS_DSI_RST_SEQ_LEN || num % 2) {
		pr_debug("%s:%d, error reading %s, length found = %d\n",
			__func__, __LINE__, name, num);
	} else {
		rc = of_property_read_u32_array(np, name, tmp, num);
		if (rc)
			pr_debug("%s:%d, error reading %s, rc = %d\n",
				__func__, __LINE__, name, rc);
		else {
			for (i = 0; i < num; ++i)
				rst_seq[i] = tmp[i];
			*rst_len = num;
		}
	}
	return 0;
}

bool mdss_dsi_panel_flip_ud(void)
{
	return mdss_panel_flip_ud;
}

static int mdss_dsi_gen_read_status(struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{
	if (!mdss_dsi_cmp_panel_reg(ctrl_pdata->status_buf,
		ctrl_pdata->status_value, 0)) {
		pr_err("%s: Read back value from panel is incorrect\n",
							__func__);
		return -EINVAL;
	} else {
		return 1;
	}
}

static int mdss_dsi_nt35596_read_status(struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{
	/* Not supported for Sony (no device uses it) */
	return -EINVAL;
}

static void mdss_dsi_parse_roi_alignment(struct device_node *np,
		struct dsi_panel_timing *pt)
{
	int len = 0;
	u32 value[6];
	struct property *data;
	struct mdss_panel_timing *timing = &pt->timing;

	data = of_find_property(np, "qcom,panel-roi-alignment", &len);
	len /= sizeof(u32);
	if (!data || (len != 6)) {
		pr_debug("%s: Panel roi alignment not found", __func__);
	} else {
		int rc = of_property_read_u32_array(np,
				"qcom,panel-roi-alignment", value, len);
		if (rc)
			pr_debug("%s: Error reading panel roi alignment values",
					__func__);
		else {
			timing->roi_alignment.xstart_pix_align = value[0];
			timing->roi_alignment.ystart_pix_align = value[1];
			timing->roi_alignment.width_pix_align = value[2];
			timing->roi_alignment.height_pix_align = value[3];
			timing->roi_alignment.min_width = value[4];
			timing->roi_alignment.min_height = value[5];
		}

		pr_debug("%s: ROI alignment: [%d, %d, %d, %d, %d, %d]",
			__func__, timing->roi_alignment.xstart_pix_align,
			timing->roi_alignment.width_pix_align,
			timing->roi_alignment.ystart_pix_align,
			timing->roi_alignment.height_pix_align,
			timing->roi_alignment.min_width,
			timing->roi_alignment.min_height);
	}
}

static void mdss_dsi_parse_dms_config(struct device_node *np,
	struct mdss_dsi_ctrl_pdata *ctrl)
{
	struct mdss_panel_info *pinfo = &ctrl->panel_data.panel_info;
	const char *data;
	bool dms_enabled;

	dms_enabled = of_property_read_bool(np,
		"qcom,dynamic-mode-switch-enabled");

	if (!dms_enabled) {
		pinfo->mipi.dms_mode = DYNAMIC_MODE_SWITCH_DISABLED;
		goto exit;
	}

	/* default mode is suspend_resume */
	pinfo->mipi.dms_mode = DYNAMIC_MODE_SWITCH_SUSPEND_RESUME;
	data = of_get_property(np, "qcom,dynamic-mode-switch-type", NULL);
	if (data) {
		if (!strcmp(data, "dynamic-resolution-switch-immediate")) {
			if (!list_empty(&ctrl->panel_data.timings_list))
				pinfo->mipi.dms_mode =
				    DYNAMIC_MODE_RESOLUTION_SWITCH_IMMEDIATE;
			else
				pinfo->mipi.dms_mode =
				    DYNAMIC_MODE_SWITCH_DISABLED;

			goto exit;
		} else if (!strcmp(data, "dynamic-resolution-switch-susres")) {
			pinfo->mipi.dms_mode =
				DYNAMIC_MODE_SWITCH_SUSPEND_RESUME;
			goto exit;
		} else if (!strcmp(data, "dynamic-switch-immediate")) {
			/* Video<=>CMD dynamic mode switch */
			pinfo->mipi.dms_mode = DYNAMIC_MODE_SWITCH_IMMEDIATE;
		}
	}

	mdss_dsi_parse_dcs_cmds(np, &ctrl->video2cmd,
		"qcom,video-to-cmd-mode-switch-commands", NULL);

	mdss_dsi_parse_dcs_cmds(np, &ctrl->cmd2video,
		"qcom,cmd-to-video-mode-switch-commands", NULL);

	mdss_dsi_parse_dcs_cmds(np, &ctrl->post_dms_on_cmds,
		"qcom,mdss-dsi-post-mode-switch-on-command",
		"qcom,mdss-dsi-post-mode-switch-on-command-state");

	if (pinfo->mipi.dms_mode == DYNAMIC_MODE_SWITCH_IMMEDIATE &&
		!ctrl->post_dms_on_cmds.cmd_cnt) {
		pr_warn("%s: No post dms on cmd specified\n", __func__);
		pinfo->mipi.dms_mode = DYNAMIC_MODE_SWITCH_DISABLED;
	}

	if (!ctrl->video2cmd.cmd_cnt || !ctrl->cmd2video.cmd_cnt) {
		pr_warn("%s: No commands specified for dynamic switch\n",
			__func__);
		pinfo->mipi.dms_mode = DYNAMIC_MODE_SWITCH_DISABLED;
	}
exit:
	pr_info("%s: dynamic switch feature enabled: %d\n", __func__,
		pinfo->mipi.dms_mode);
	return;
}

/* the length of all the valid values to be checked should not be great
 * than the length of returned data from read command.
 */
static bool
mdss_dsi_parse_esd_check_valid_params(struct mdss_dsi_ctrl_pdata *ctrl)
{
	int i;

	for (i = 0; i < ctrl->status_cmds.cmd_cnt; ++i) {
		if (ctrl->status_valid_params[i] > ctrl->status_cmds_rlen[i]) {
			pr_debug("%s: ignore valid params!\n", __func__);
			return false;
		}
	}

	return true;
}

static bool mdss_dsi_parse_esd_status_len(struct device_node *np,
	char *prop_key, u32 **target, u32 cmd_cnt)
{
	int tmp;

	if (!of_find_property(np, prop_key, &tmp))
		return false;

	tmp /= sizeof(u32);
	if (tmp != cmd_cnt) {
		pr_err("%s: request property number(%d) not match command count(%d)\n",
			__func__, tmp, cmd_cnt);
		return false;
	}

	*target = kcalloc(tmp, sizeof(u32), GFP_KERNEL);
	if (IS_ERR_OR_NULL(*target)) {
		pr_err("%s: Error allocating memory for property\n",
			__func__);
		return false;
	}

	if (of_property_read_u32_array(np, prop_key, *target, tmp)) {
		pr_err("%s: cannot get values from dts\n", __func__);
		kfree(*target);
		*target = NULL;
		return false;
	}

	return true;
}

static void mdss_dsi_parse_esd_params(struct device_node *np,
	struct mdss_dsi_ctrl_pdata *ctrl)
{
	u32 tmp;
	u32 i, status_len, *lenp;
	int rc;
	struct property *data;
	const char *string;
	struct mdss_panel_info *pinfo = &ctrl->panel_data.panel_info;

	pinfo->esd_check_enabled = of_property_read_bool(np,
		"qcom,esd-check-enabled");

	if (!pinfo->esd_check_enabled)
		return;

	ctrl->status_mode = ESD_MAX;
	rc = of_property_read_string(np,
			"qcom,mdss-dsi-panel-status-check-mode", &string);
	if (!rc) {
		if (!strcmp(string, "bta_check")) {
			ctrl->status_mode = ESD_BTA;
		} else if (!strcmp(string, "reg_read")) {
			ctrl->status_mode = ESD_REG;
			ctrl->check_read_status =
				mdss_dsi_gen_read_status;
		} else if (!strcmp(string, "reg_read_nt35596")) {
			ctrl->status_mode = ESD_REG_NT35596;
			ctrl->status_error_count = 0;
			ctrl->check_read_status =
				mdss_dsi_nt35596_read_status;
		} else if (!strcmp(string, "te_signal_check")) {
			if (pinfo->mipi.mode == DSI_CMD_MODE) {
				ctrl->status_mode = ESD_TE;
			} else {
				pr_err("TE-ESD not valid for video mode\n");
				goto error;
			}
		} else {
			pr_err("No valid panel-status-check-mode string\n");
			goto error;
		}
	}

	if ((ctrl->status_mode == ESD_BTA) || (ctrl->status_mode == ESD_TE) ||
			(ctrl->status_mode == ESD_MAX))
		return;

	mdss_dsi_parse_dcs_cmds(np, &ctrl->status_cmds,
			"qcom,mdss-dsi-panel-status-command",
				"qcom,mdss-dsi-panel-status-command-state");

	rc = of_property_read_u32(np, "qcom,mdss-dsi-panel-max-error-count",
		&tmp);
	ctrl->max_status_error_count = (!rc ? tmp : 0);

	if (!mdss_dsi_parse_esd_status_len(np,
		"qcom,mdss-dsi-panel-status-read-length",
		&ctrl->status_cmds_rlen, ctrl->status_cmds.cmd_cnt)) {
		pinfo->esd_check_enabled = false;
		return;
	}

	if (mdss_dsi_parse_esd_status_len(np,
		"qcom,mdss-dsi-panel-status-valid-params",
		&ctrl->status_valid_params, ctrl->status_cmds.cmd_cnt)) {
		if (!mdss_dsi_parse_esd_check_valid_params(ctrl))
			goto error1;
	}

	status_len = 0;
	lenp = ctrl->status_valid_params ?: ctrl->status_cmds_rlen;
	for (i = 0; i < ctrl->status_cmds.cmd_cnt; ++i)
		status_len += lenp[i];

	data = of_find_property(np, "qcom,mdss-dsi-panel-status-value", &tmp);
	tmp /= sizeof(u32);
	if (!IS_ERR_OR_NULL(data) && tmp != 0 && (tmp % status_len) == 0) {
		ctrl->groups = tmp / status_len;
	} else {
		pr_err("%s: Error parse panel-status-value\n", __func__);
		goto error1;
	}

	ctrl->status_value = kzalloc(sizeof(u32) * status_len * ctrl->groups,
				GFP_KERNEL);
	if (!ctrl->status_value)
		goto error1;

	ctrl->return_buf = kcalloc(status_len * ctrl->groups,
			sizeof(unsigned char), GFP_KERNEL);
	if (!ctrl->return_buf)
		goto error2;

	rc = of_property_read_u32_array(np,
		"qcom,mdss-dsi-panel-status-value",
		ctrl->status_value, ctrl->groups * status_len);
	if (rc) {
		pr_debug("%s: Error reading panel status values\n",
				__func__);
		memset(ctrl->status_value, 0, ctrl->groups * status_len);
	}

	return;

error2:
	kfree(ctrl->status_value);
error1:
	kfree(ctrl->status_valid_params);
	kfree(ctrl->status_cmds_rlen);
error:
	pinfo->esd_check_enabled = false;
}

static void mdss_dsi_parse_partial_update_caps(struct device_node *np,
		struct mdss_dsi_ctrl_pdata *ctrl)
{
	struct mdss_panel_info *pinfo;
	const char *data;

	pinfo = &ctrl->panel_data.panel_info;

	data = of_get_property(np, "qcom,partial-update-enabled", NULL);
	if (data && !strcmp(data, "single_roi"))
		pinfo->partial_update_supported =
			PU_SINGLE_ROI;
	else if (data && !strcmp(data, "dual_roi"))
		pinfo->partial_update_supported =
			PU_DUAL_ROI;
	else if (data && !strcmp(data, "none"))
		pinfo->partial_update_supported =
			PU_NOT_SUPPORTED;
	else
		pinfo->partial_update_supported =
			PU_NOT_SUPPORTED;

	if (pinfo->mipi.mode == DSI_CMD_MODE) {
		pinfo->partial_update_enabled = pinfo->partial_update_supported;
		pr_info("%s: partial_update_enabled=%d\n", __func__,
					pinfo->partial_update_enabled);
		ctrl->set_col_page_addr = mdss_dsi_set_col_page_addr;
		if (pinfo->partial_update_enabled) {
			pinfo->partial_update_roi_merge =
					of_property_read_bool(np,
					"qcom,partial-update-roi-merge");
		}
	}
}

static int mdss_dsi_parse_panel_features(struct device_node *np,
	struct mdss_dsi_ctrl_pdata *ctrl)
{
	struct mdss_panel_info *pinfo;

	pinfo = &ctrl->panel_data.panel_info;

	mdss_dsi_parse_partial_update_caps(np, ctrl);

	pinfo->dcs_cmd_by_left = of_property_read_bool(np,
					"qcom,dcs-cmd-by-left");

	pinfo->ulps_feature_enabled = of_property_read_bool(np,
		"qcom,ulps-enabled");
	pr_info("%s: ulps feature %s\n", __func__,
		(pinfo->ulps_feature_enabled ? "enabled" : "disabled"));

	pinfo->ulps_suspend_enabled = of_property_read_bool(np,
		"qcom,suspend-ulps-enabled");
	pr_info("%s: ulps during suspend feature %s", __func__,
		(pinfo->ulps_suspend_enabled ? "enabled" : "disabled"));

	mdss_dsi_parse_dms_config(np, ctrl);

	pinfo->panel_ack_disabled = pinfo->sim_panel_mode ?
		1 : of_property_read_bool(np, "qcom,panel-ack-disabled");

	pinfo->allow_phy_power_off = of_property_read_bool(np,
		"qcom,panel-allow-phy-poweroff");

	mdss_dsi_parse_esd_params(np, ctrl);

	if (pinfo->panel_ack_disabled && pinfo->esd_check_enabled) {
		pr_warn("ESD should not be enabled if panel ACK is disabled\n");
		pinfo->esd_check_enabled = false;
	}

	if (ctrl->disp_en_gpio <= 0) {
		ctrl->disp_en_gpio = of_get_named_gpio(
			np,
			"qcom,5v-boost-gpio", 0);

		if (!gpio_is_valid(ctrl->disp_en_gpio))
			pr_err("%s:%d, Disp_en gpio not specified\n",
					__func__, __LINE__);
	}

	return 0;
}

static void mdss_dsi_parse_panel_horizontal_line_idle(struct device_node *np,
	struct mdss_dsi_ctrl_pdata *ctrl)
{
	const u32 *src;
	int i, len, cnt;
	struct panel_horizontal_idle *kp;

	if (!np || !ctrl) {
		pr_err("%s: Invalid arguments\n", __func__);
		return;
	}

	src = of_get_property(np, "qcom,mdss-dsi-hor-line-idle", &len);
	if (!src || len == 0)
		return;

	cnt = len % 3; /* 3 fields per entry */
	if (cnt) {
		pr_err("%s: invalid horizontal idle len=%d\n", __func__, len);
		return;
	}

	cnt = len / sizeof(u32);

	kp = kzalloc(sizeof(*kp) * (cnt / 3), GFP_KERNEL);
	if (kp == NULL) {
		pr_err("%s: No memory\n", __func__);
		return;
	}

	ctrl->line_idle = kp;
	for (i = 0; i < cnt; i += 3) {
		kp->min = be32_to_cpu(src[i]);
		kp->max = be32_to_cpu(src[i+1]);
		kp->idle = be32_to_cpu(src[i+2]);
		kp++;
		ctrl->horizontal_idle_cnt++;
	}

	/*
	 * idle is enabled for this controller, this will be used to
	 * enable/disable burst mode since both features are mutually
	 * exclusive.
	 */
	ctrl->idle_enabled = true;

	pr_debug("%s: horizontal_idle_cnt=%d\n", __func__,
				ctrl->horizontal_idle_cnt);
}

static int mdss_dsi_set_refresh_rate_range(struct device_node *pan_node,
		struct mdss_panel_info *pinfo)
{
	int rc = 0;
	rc = of_property_read_u32(pan_node,
			"qcom,mdss-dsi-min-refresh-rate",
			&pinfo->min_fps);
	if (rc) {
		pr_warn("%s:%d, Unable to read min refresh rate\n",
				__func__, __LINE__);

		/*
		 * Since min refresh rate is not specified when dynamic
		 * fps is enabled, using minimum as 30
		 */
		pinfo->min_fps = MIN_REFRESH_RATE;
		rc = 0;
	}

	rc = of_property_read_u32(pan_node,
			"qcom,mdss-dsi-max-refresh-rate",
			&pinfo->max_fps);
	if (rc) {
		pr_warn("%s:%d, Unable to read max refresh rate\n",
				__func__, __LINE__);

		/*
		 * Since max refresh rate was not specified when dynamic
		 * fps is enabled, using the default panel refresh rate
		 * as max refresh rate supported.
		 */
		pinfo->max_fps = pinfo->mipi.frame_rate;
		rc = 0;
	}

	pr_info("dyn_fps: min = %d, max = %d\n",
			pinfo->min_fps, pinfo->max_fps);
	return rc;
}

static void mdss_dsi_parse_dfps_config(struct device_node *pan_node,
			struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{
	const char *data;
	bool dynamic_fps;
	struct mdss_panel_info *pinfo = &(ctrl_pdata->panel_data.panel_info);

	dynamic_fps = of_property_read_bool(pan_node,
			"qcom,mdss-dsi-pan-enable-dynamic-fps");

	if (!dynamic_fps)
		return;

	pinfo->dynamic_fps = true;
	data = of_get_property(pan_node, "qcom,mdss-dsi-pan-fps-update", NULL);
	if (data) {
		if (!strcmp(data, "dfps_suspend_resume_mode")) {
			pinfo->dfps_update = DFPS_SUSPEND_RESUME_MODE;
			pr_debug("dfps mode: suspend/resume\n");
		} else if (!strcmp(data, "dfps_immediate_clk_mode")) {
			pinfo->dfps_update = DFPS_IMMEDIATE_CLK_UPDATE_MODE;
			pr_debug("dfps mode: Immediate clk\n");
		} else if (!strcmp(data, "dfps_immediate_porch_mode_hfp")) {
			pinfo->dfps_update =
				DFPS_IMMEDIATE_PORCH_UPDATE_MODE_HFP;
			pr_debug("dfps mode: Immediate porch HFP\n");
		} else if (!strcmp(data, "dfps_immediate_porch_mode_vfp")) {
			pinfo->dfps_update =
				DFPS_IMMEDIATE_PORCH_UPDATE_MODE_VFP;
			pr_debug("dfps mode: Immediate porch VFP\n");
		} else {
			pinfo->dfps_update = DFPS_SUSPEND_RESUME_MODE;
			pr_debug("default dfps mode: suspend/resume\n");
		}
		mdss_dsi_set_refresh_rate_range(pan_node, pinfo);
	} else {
		pinfo->dynamic_fps = false;
		pr_debug("dfps update mode not configured: disable\n");
	}
	pinfo->new_fps = pinfo->mipi.frame_rate;
	pinfo->current_fps = pinfo->mipi.frame_rate;

	return;
}

static int mdss_dsi_parse_polling_config(struct device_node *pan_node,
			struct mdss_dsi_ctrl_pdata *ctrl)
{
	int rc;
	u32 tmp;
	struct mdss_panel_info *pinfo = &(ctrl->panel_data.panel_info);
	struct mdss_panel_specific_pdata *spec_pdata = ctrl->spec_pdata;

	if (spec_pdata == NULL) {
		pr_err("%s: FATAL: spec_pdata is NULL!!", __func__);
		return -ENODEV;
	}

	spec_pdata->polling.enable
			= of_property_read_bool(pan_node, "somc,poll-enable");

	if (spec_pdata->polling.enable && pinfo->dsi_master == pinfo->pdest) {
		rc = of_property_read_u32(pan_node,
				"somc,poll-intervals", &tmp);
		spec_pdata->polling.intervals = (!rc ? tmp : 10000);

		rc = of_property_read_u32(pan_node,
				"somc,poll-esd-reg-adress", &tmp);
		spec_pdata->polling.esd.reg = (!rc ? tmp : 0x0a);

		rc = of_property_read_u32(pan_node,
				"somc,poll-esd-reg-val", &tmp);
		spec_pdata->polling.esd.correct_val = (!rc ? tmp : 0x9C);
	}

	return 0;
}

int mdss_panel_parse_bl_settings(struct device_node *np,
			struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{
	const char *data;
	int rc = 0;
	u32 tmp;

	ctrl_pdata->bklt_ctrl = UNKNOWN_CTRL;
	data = of_get_property(np, "qcom,mdss-dsi-bl-pmic-control-type", NULL);
	if (data) {
		if (!strcmp(data, "bl_ctrl_wled")) {
			if (ctrl_pdata->spec_pdata->detected) {
				led_trigger_register_simple("bkl-trigger",
					&bl_led_trigger);
				pr_info("%s: WLED bkl-trigger registered!\n",
					__func__);
			}
			ctrl_pdata->bklt_ctrl = BL_WLED;
		} else if (!strcmp(data, "bl_ctrl_pwm")) {
			ctrl_pdata->bklt_ctrl = BL_PWM;
			ctrl_pdata->pwm_pmi = of_property_read_bool(np,
					"qcom,mdss-dsi-bl-pwm-pmi");
			rc = of_property_read_u32(np,
				"qcom,mdss-dsi-bl-pmic-pwm-frequency", &tmp);
			if (rc) {
				pr_err("%s:%d, Error, panel pwm_period\n",
						__func__, __LINE__);
				return -EINVAL;
			}
			ctrl_pdata->pwm_period = tmp;
			if (ctrl_pdata->pwm_pmi) {
				ctrl_pdata->pwm_bl = of_pwm_get(np, NULL);
				if (IS_ERR(ctrl_pdata->pwm_bl)) {
					pr_err("%s: Error, pwm device\n",
								__func__);
					ctrl_pdata->pwm_bl = NULL;
					return -EINVAL;
				}
			} else {
				rc = of_property_read_u32(np,
					"qcom,mdss-dsi-bl-pmic-bank-select",
								 &tmp);
				if (rc) {
					pr_err("%s:%d, Error, lpg channel\n",
							__func__, __LINE__);
					return -EINVAL;
				}
				ctrl_pdata->pwm_lpg_chan = tmp;
				tmp = of_get_named_gpio(np,
					"qcom,mdss-dsi-pwm-gpio", 0);
				ctrl_pdata->pwm_pmic_gpio = tmp;
				pr_debug("%s: Configured PWM bklt ctrl\n",
								 __func__);
			}
		} else if (!strcmp(data, "bl_ctrl_dcs")) {
			ctrl_pdata->bklt_ctrl = BL_DCS_CMD;
			pr_debug("%s: Configured DCS_CMD bklt ctrl\n",
								__func__);
		}
	}
	return 0;
}

void mdss_dsi_unregister_bl_settings(struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{
	if (ctrl_pdata->bklt_ctrl == BL_WLED)
		led_trigger_unregister_simple(bl_led_trigger);
}

int mdss_dsi_panel_timing_switch(struct mdss_dsi_ctrl_pdata *ctrl,
			struct mdss_panel_timing *timing)
{
	struct dsi_panel_timing *pt;
	struct mdss_panel_info *pinfo = &ctrl->panel_data.panel_info;
	int i;

	if (!timing)
		return -EINVAL;

	if (timing == ctrl->panel_data.current_timing) {
		pr_warn("%s: panel timing \"%s\" already set\n", __func__,
				timing->name);
		return 0; /* nothing to do */
	}

	pr_info("%s: ndx=%d switching to panel timing \"%s\"\n", __func__,
			ctrl->ndx, timing->name);

	mdss_panel_info_from_timing(timing, pinfo);

	pt = container_of(timing, struct dsi_panel_timing, timing);
	pinfo->mipi.t_clk_pre = pt->t_clk_pre;
	pinfo->mipi.t_clk_post = pt->t_clk_post;

	for (i = 0; i < ARRAY_SIZE(pt->phy_timing); i++)
		pinfo->mipi.dsi_phy_db.timing[i] = pt->phy_timing[i];

	for (i = 0; i < ARRAY_SIZE(pt->phy_timing_8996); i++)
		pinfo->mipi.dsi_phy_db.timing_8996[i] = pt->phy_timing_8996[i];

	ctrl->spec_pdata->einit_cmds = pt->einit_cmds;
	ctrl->spec_pdata->init_cmds = pt->init_cmds;
	ctrl->on_cmds = pt->on_cmds;
	ctrl->post_panel_on_cmds = pt->post_panel_on_cmds;

	ctrl->panel_data.current_timing = timing;
	if (!timing->clk_rate)
		ctrl->refresh_clk_rate = true;

	/* Set color correction parameters again for the new mode */
	somc_panel_colormgr_reset(ctrl);
	
	/* Refresh FPS immediately in case of refresh rate change */
	somc_panel_fpsman_refresh(ctrl, true);

	mdss_dsi_clk_refresh(&ctrl->panel_data, ctrl->update_phy_timing);

	return 0;
}

static int mdss_dsi_panel_timing_from_dt(struct device_node *np,
		struct dsi_panel_timing *pt,
		struct mdss_panel_data *panel_data)
{
	u32 tmp;
	u64 tmp64;
	int rc, i, len;
	const char *data;
	struct mdss_dsi_ctrl_pdata *ctrl_pdata;
	struct mdss_panel_info *pinfo;
	bool phy_timings_present;

	pinfo = &panel_data->panel_info;

	ctrl_pdata = container_of(panel_data, struct mdss_dsi_ctrl_pdata,
				panel_data);

	rc = of_property_read_u32(np, "qcom,mdss-dsi-panel-width", &tmp);
	if (rc) {
		pr_err("%s:%d, panel width not specified\n",
						__func__, __LINE__);
		return -EINVAL;
	}
	pt->timing.xres = tmp;

	rc = of_property_read_u32(np, "qcom,mdss-dsi-panel-height", &tmp);
	if (rc) {
		pr_err("%s:%d, panel height not specified\n",
						__func__, __LINE__);
		return -EINVAL;
	}
	pt->timing.yres = tmp;

	rc = of_property_read_u32(np, "qcom,mdss-dsi-h-front-porch", &tmp);
	pt->timing.h_front_porch = (!rc ? tmp : 6);
	rc = of_property_read_u32(np, "qcom,mdss-dsi-h-back-porch", &tmp);
	pt->timing.h_back_porch = (!rc ? tmp : 6);
	rc = of_property_read_u32(np, "qcom,mdss-dsi-h-pulse-width", &tmp);
	pt->timing.h_pulse_width = (!rc ? tmp : 2);
	rc = of_property_read_u32(np, "qcom,mdss-dsi-h-sync-skew", &tmp);
	pt->timing.hsync_skew = (!rc ? tmp : 0);
	rc = of_property_read_u32(np, "qcom,mdss-dsi-v-back-porch", &tmp);
	pt->timing.v_back_porch = (!rc ? tmp : 6);
	rc = of_property_read_u32(np, "qcom,mdss-dsi-v-front-porch", &tmp);
	pt->timing.v_front_porch = (!rc ? tmp : 6);
	rc = of_property_read_u32(np, "qcom,mdss-dsi-v-pulse-width", &tmp);
	pt->timing.v_pulse_width = (!rc ? tmp : 2);

	rc = of_property_read_u32(np, "qcom,mdss-dsi-v-top-border", &tmp);
	pt->timing.border_top = !rc ? tmp : 0;
	rc = of_property_read_u32(np, "qcom,mdss-dsi-v-bottom-border", &tmp);
	pt->timing.border_bottom = !rc ? tmp : 0;
	rc = of_property_read_u32(np, "qcom,mdss-dsi-h-left-border", &tmp);
	pt->timing.border_left = !rc ? tmp : 0;
	rc = of_property_read_u32(np, "qcom,mdss-dsi-h-right-border", &tmp);
	pt->timing.border_right = !rc ? tmp : 0;

	/* overriding left/right borders for split display cases */
	if (mdss_dsi_is_hw_config_split(ctrl_pdata->shared_data)) {
		if (panel_data->next)
			pt->timing.border_right = 0;
		else
			pt->timing.border_left = 0;
	}

	rc = of_property_read_u32(np, "qcom,mdss-dsi-panel-framerate", &tmp);
	pt->timing.frame_rate = !rc ? tmp : DEFAULT_FRAME_RATE;
	rc = of_property_read_u64(np, "qcom,mdss-dsi-panel-clockrate", &tmp64);
	if (rc == -EOVERFLOW) {
		tmp64 = 0;
		rc = of_property_read_u32(np,
			"qcom,mdss-dsi-panel-clockrate", (u32 *)&tmp64);
	}
	pt->timing.clk_rate = !rc ? tmp64 : 0;


	data = of_get_property(np, "qcom,mdss-dsi-panel-timings", &len);
	if ((!data) || (len != 12)) {
		pr_err("%s:%d, Unable to read Phy timing settings",
		       __func__, __LINE__);
	} else {
		for (i = 0; i < len; i++)
			pt->phy_timing[i] = data[i];
		phy_timings_present = true;
	}

	data = of_get_property(np, "qcom,mdss-dsi-panel-timings-phy-v2", &len);
	if ((!data) || (len != 40)) {
		pr_debug("%s:%d, Unable to read 8996 Phy lane timing settings",
		       __func__, __LINE__);
	} else {
		for (i = 0; i < len; i++)
			pt->phy_timing_8996[i] = data[i];
		phy_timings_present = true;
	}
	if (!phy_timings_present)
		pr_err("%s: phy timing settings not present\n", __func__);

	rc = of_property_read_u32(np, "qcom,mdss-dsi-t-clk-pre", &tmp);
	pt->t_clk_pre = (!rc ? tmp : 0x24);
	rc = of_property_read_u32(np, "qcom,mdss-dsi-t-clk-post", &tmp);
	pt->t_clk_post = (!rc ? tmp : 0x03);

	if (np->name) {
		pt->timing.name = kstrdup(np->name, GFP_KERNEL);
		pr_info("%s: found new timing \"%s\" (%p)\n", __func__,
				np->name, &pt->timing);
	}

	return 0;
}

static int  mdss_dsi_panel_config_res_properties(struct device_node *np,
		struct dsi_panel_timing *pt,
		struct mdss_panel_data *panel_data,
		bool default_timing)
{
	int rc = 0;

	mdss_dsi_parse_roi_alignment(np, pt);

	mdss_dsi_parse_dcs_cmds(np, &pt->einit_cmds,
		"somc,mdss-dsi-early-init-command", NULL);

	mdss_dsi_parse_dcs_cmds(np, &pt->init_cmds,
		"somc,mdss-dsi-init-command", NULL);

	mdss_dsi_parse_dcs_cmds(np, &pt->on_cmds,
			"qcom,mdss-dsi-on-command",
			"qcom,mdss-dsi-on-command-state");

	mdss_dsi_parse_dcs_cmds(np, &pt->post_panel_on_cmds,
		"qcom,mdss-dsi-post-panel-on-command", NULL);

	mdss_dsi_parse_dcs_cmds(np, &pt->switch_cmds,
			"qcom,mdss-dsi-timing-switch-command",
			"qcom,mdss-dsi-timing-switch-command-state");

	rc = mdss_dsi_parse_topology_config(np, pt, panel_data, default_timing);
	if (rc) {
		pr_err("%s: parsing compression params failed. rc:%d\n",
			__func__, rc);
		return rc;
	}

	mdss_panel_parse_te_params(np, &pt->timing);

	return rc;
}

static int mdss_panel_parse_display_timings(struct device_node *np,
		struct mdss_panel_data *panel_data)
{
	struct mdss_dsi_ctrl_pdata *ctrl;
	struct dsi_panel_timing *modedb;
	struct device_node *timings_np;
	struct device_node *entry;
	int num_timings, rc;
	int i = 0, active_ndx = 0;
	bool default_timing = false;

	ctrl = container_of(panel_data, struct mdss_dsi_ctrl_pdata, panel_data);

	INIT_LIST_HEAD(&panel_data->timings_list);

	timings_np = of_get_child_by_name(np, "qcom,mdss-dsi-display-timings");
	if (!timings_np) {
		struct dsi_panel_timing pt;
		memset(&pt, 0, sizeof(struct dsi_panel_timing));

		/*
		 * display timings node is not available, fallback to reading
		 * timings directly from root node instead
		 */
		pr_debug("reading display-timings from panel node\n");
		rc = mdss_dsi_panel_timing_from_dt(np, &pt, panel_data);
		if (!rc) {
			mdss_dsi_panel_config_res_properties(np,
				&pt, panel_data, true);
			rc = mdss_dsi_panel_timing_switch(ctrl, &pt.timing);
		}
		return rc;
	}

	num_timings = of_get_child_count(timings_np);
	if (num_timings == 0) {
		pr_err("no timings found within display-timings\n");
		rc = -EINVAL;
		goto exit;
	}

	modedb = kcalloc(num_timings, sizeof(*modedb), GFP_KERNEL);
	if (!modedb) {
		pr_err("unable to allocate modedb\n");
		rc = -ENOMEM;
		goto exit;
	}

	for_each_child_of_node(timings_np, entry) {
		rc = mdss_dsi_panel_timing_from_dt(entry, (modedb + i),
				panel_data);
		if (rc) {
			kfree(modedb);
			goto exit;
		}

		default_timing = of_property_read_bool(entry,
				"qcom,mdss-dsi-timing-default");
		if (default_timing)
			active_ndx = i;

		mdss_dsi_panel_config_res_properties(entry,
			(modedb + i), panel_data, default_timing);

		list_add(&modedb[i].timing.list,
				&panel_data->timings_list);
		i++;
	}

	/* Configure default timing settings */
	rc = mdss_dsi_panel_timing_switch(ctrl, &modedb[active_ndx].timing);
	if (rc)
		pr_err("unable to configure default timing settings\n");

exit:
	of_node_put(timings_np);

	return rc;
}


int mdss_panel_parse_dt(struct device_node *np,
			struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{
	u32 res[2], tmp;
	int rc, i, len;
	const char *data;
	static const char *pdest;
	static const char *panel_name;
	const char *bridge_chip_name;
	struct mdss_panel_info *pinfo = &(ctrl_pdata->panel_data.panel_info);
	struct mdss_panel_specific_pdata *spec_pdata = NULL;
	struct device_node *cfg_np = NULL;

	spec_pdata = ctrl_pdata->spec_pdata;
	if (!spec_pdata) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}


	panel_name = of_get_property(np, "qcom,mdss-dsi-panel-name", NULL);
	if (!panel_name) {
		pr_info("%s:%d, panel name not specified\n",
						__func__, __LINE__);
		return -EINVAL;
	} else {
		pinfo->panel_id_name = panel_name;
		pinfo->panel_name[0] = '\0';
		pr_info("%s: Panel Name = %s\n", __func__, panel_name);
		strlcpy(&pinfo->panel_name[0], panel_name, MDSS_MAX_PANEL_LEN);
	}

	rc = of_property_read_u32(np, "somc,panel-detect", &tmp);
	spec_pdata->panel_detect = !rc ? tmp : 0;

	mdss_dsi_parse_dcs_cmds(np, &spec_pdata->id_read_cmds,
		"somc,mdss-dsi-id-read-command", NULL);

	if (mdss_dsi_is_hw_config_split(ctrl_pdata->shared_data))
		pinfo->is_split_display = true;

	if (of_find_property(np, "qcom,config-select", NULL)) {
		cfg_np = of_parse_phandle(np, "qcom,config-select", 0);
		if (!cfg_np) {
			pr_err("%s: error parsing qcom,config-select\n",
				__func__);
		} else {
			if (!of_property_read_u32_array(cfg_np, "qcom,lm-split",
			    pinfo->lm_widths, 2)) {
				if (pinfo->is_split_display &&
				    (pinfo->lm_widths[1] != 0)) {
					pr_err("%s: lm-split not allowed with split display\n",
						__func__);
					return -EINVAL;
				}
			}
		}
	} else {
		pr_debug("%s: qcom,config-select is not present\n", __func__);
	}

	tmp = (int) of_property_read_bool(np, "somc,first-boot-aware");
	if (tmp)
		lcm_first_boot = tmp;

	mdss_panel_flip_ud = of_property_read_bool(np,
				"qcom,mdss-pan-flip-ud");
	if (mdss_panel_flip_ud)
		pr_info("%s: Detected upside down panel\n", __func__);

	rc = of_property_read_u32(np,
		"qcom,mdss-pan-physical-width-dimension", &tmp);
	pinfo->physical_width = !rc ? tmp : 0;
	rc = of_property_read_u32(np,
		"qcom,mdss-pan-physical-height-dimension", &tmp);
	pinfo->physical_height = !rc ? tmp : 0;

	rc = of_property_read_u32(np, "qcom,mdss-dsi-bpp", &tmp);
	if (rc) {
		pr_err("%s:%d, bpp not specified\n", __func__, __LINE__);
		return -EINVAL;
	}
	pinfo->bpp = tmp;
	pinfo->mipi.mode = DSI_VIDEO_MODE;
	data = of_get_property(np, "qcom,mdss-dsi-panel-type", NULL);
	if (data && !strncmp(data, "dsi_cmd_mode", 12))
		pinfo->mipi.mode = DSI_CMD_MODE;
	pinfo->mipi.boot_mode = pinfo->mipi.mode;
	tmp = 0;
	data = of_get_property(np, "qcom,mdss-dsi-pixel-packing", NULL);
	if (data && !strcmp(data, "loose"))
		pinfo->mipi.pixel_packing = 1;
	else
		pinfo->mipi.pixel_packing = 0;
	rc = mdss_panel_get_dst_fmt(pinfo->bpp,
		pinfo->mipi.mode, pinfo->mipi.pixel_packing,
		&pinfo->mipi.dst_format);
	if (rc) {
		pr_debug("%s: problem dst format. Set Default\n",
			__func__);
		pinfo->mipi.dst_format =
			DSI_VIDEO_DST_FORMAT_RGB888;
	}

	pdest = of_get_property(np,
		"qcom,mdss-dsi-panel-destination", NULL);

	if (pdest) {
		if (strlen(pdest) != 9) {
		pr_err("%s: Unknown pdest specified\n", __func__);
			return -EINVAL;
		}
		if (!strcmp(pdest, "display_1")) {
			pinfo->pdest = DISPLAY_1;
			if (of_property_read_bool(np,
						"somc,mdss-dsi-slave"))
				pinfo->dsi_master = DISPLAY_2;
			else
				pinfo->dsi_master = DISPLAY_1;
		} else if (!strcmp(pdest, "display_2")) {
			pinfo->pdest = DISPLAY_2;
			if (of_property_read_bool(np,
						"somc,mdss-dsi-slave"))
				pinfo->dsi_master = DISPLAY_1;
			else
				pinfo->dsi_master = DISPLAY_2;
		} else {
			pr_debug("%s: incorrect pdest. Set Default\n",
				__func__);
			pinfo->pdest = DISPLAY_1;
			pinfo->dsi_master = DISPLAY_1;
		}
	} else {
		pr_debug("%s: pdest not specified. Set Default\n",
				__func__);
		pinfo->pdest = DISPLAY_1;
		pinfo->dsi_master = DISPLAY_1;
	}

	rc = of_property_read_u32(np,
		"qcom,mdss-dsi-underflow-color", &tmp);
	pinfo->lcdc.underflow_clr = !rc ? tmp : 0xff;
	rc = of_property_read_u32(np,
		"qcom,mdss-dsi-border-color", &tmp);
	pinfo->lcdc.border_clr = !rc ? tmp : 0;
	data = of_get_property(np, "qcom,mdss-dsi-panel-orientation", NULL);
	if (data) {
		pr_debug("panel orientation is %s\n", data);
		if (!strcmp(data, "180"))
			pinfo->panel_orientation = MDP_ROT_180;
		else if (!strcmp(data, "hflip"))
			pinfo->panel_orientation = MDP_FLIP_LR;
		else if (!strcmp(data, "vflip"))
			pinfo->panel_orientation = MDP_FLIP_UD;
	}

	/* WORKAROUND:Do it only if we are detecting panel via CMD detection */
	if ((spec_pdata->detect != NULL) && (pinfo->pdest == DISPLAY_1)) {
		rc = mdss_panel_parse_bl_settings(np, ctrl_pdata);
		if (rc)
			pr_err("%s: Error while parsing backlight settings!",
				__func__);
	}

	rc = of_property_read_u32(np, "qcom,mdss-brightness-max-level", &tmp);
	pinfo->brightness_max = (!rc ? tmp : MDSS_MAX_BL_BRIGHTNESS);
	rc = of_property_read_u32(np, "qcom,mdss-dsi-bl-min-level", &tmp);
	pinfo->bl_min = !rc ? tmp : 0;
	rc = of_property_read_u32(np, "qcom,mdss-dsi-bl-max-level", &tmp);
	pinfo->bl_max = !rc ? tmp : 255;

	rc = of_property_read_u32(np, "qcom,mdss-dsi-interleave-mode", &tmp);
	pinfo->mipi.interleave_mode = !rc ? tmp : 0;

	pinfo->mipi.vsync_enable = of_property_read_bool(np,
		"qcom,mdss-dsi-te-check-enable");

	if (pinfo->sim_panel_mode == SIM_SW_TE_MODE)
		pinfo->mipi.hw_vsync_mode = false;
	else
		pinfo->mipi.hw_vsync_mode = of_property_read_bool(np,
			"qcom,mdss-dsi-te-using-te-pin");

	rc = of_property_read_u32(np,
		"qcom,mdss-dsi-h-sync-pulse", &tmp);
	pinfo->mipi.pulse_mode_hsa_he = !rc ? tmp : false;

	pinfo->mipi.hfp_power_stop = of_property_read_bool(np,
		"qcom,mdss-dsi-hfp-power-mode");
	pinfo->mipi.hsa_power_stop = of_property_read_bool(np,
		"qcom,mdss-dsi-hsa-power-mode");
	pinfo->mipi.hbp_power_stop = of_property_read_bool(np,
		"qcom,mdss-dsi-hbp-power-mode");
	pinfo->mipi.last_line_interleave_en = of_property_read_bool(np,
		"qcom,mdss-dsi-last-line-interleave");
	pinfo->mipi.bllp_power_stop = of_property_read_bool(np,
		"qcom,mdss-dsi-bllp-power-mode");
	pinfo->mipi.eof_bllp_power_stop = of_property_read_bool(
			np, "qcom,mdss-dsi-bllp-eof-power-mode");
	pinfo->mipi.traffic_mode = DSI_NON_BURST_SYNCH_PULSE;
	data = of_get_property(np, "qcom,mdss-dsi-traffic-mode", NULL);
	if (data) {
		if (!strcmp(data, "non_burst_sync_event"))
			pinfo->mipi.traffic_mode = DSI_NON_BURST_SYNCH_EVENT;
		else if (!strcmp(data, "burst_mode"))
			pinfo->mipi.traffic_mode = DSI_BURST_MODE;
	}
	rc = of_property_read_u32(np,
		"qcom,mdss-dsi-te-dcs-command", &tmp);
	pinfo->mipi.insert_dcs_cmd = !rc ? tmp : 1;
	rc = of_property_read_u32(np,
		"qcom,mdss-dsi-wr-mem-continue", &tmp);
	pinfo->mipi.wr_mem_continue = !rc ? tmp : 0x3c;
	rc = of_property_read_u32(np,
		"qcom,mdss-dsi-wr-mem-start", &tmp);
	pinfo->mipi.wr_mem_start = !rc ? tmp : 0x2c;
	rc = of_property_read_u32(np,
		"qcom,mdss-dsi-te-pin-select", &tmp);
	pinfo->mipi.te_sel = !rc ? tmp : 1;
	rc = of_property_read_u32(np,
		"qcom,mdss-dsi-virtual-channel-id", &tmp);
	pinfo->mipi.vc = !rc ? tmp : 0;
	pinfo->mipi.rgb_swap = DSI_RGB_SWAP_RGB;
	data = of_get_property(np, "qcom,mdss-dsi-color-order", NULL);
	if (data) {
		if (!strcmp(data, "rgb_swap_rbg"))
			pinfo->mipi.rgb_swap = DSI_RGB_SWAP_RBG;
		else if (!strcmp(data, "rgb_swap_bgr"))
			pinfo->mipi.rgb_swap = DSI_RGB_SWAP_BGR;
		else if (!strcmp(data, "rgb_swap_brg"))
			pinfo->mipi.rgb_swap = DSI_RGB_SWAP_BRG;
		else if (!strcmp(data, "rgb_swap_grb"))
			pinfo->mipi.rgb_swap = DSI_RGB_SWAP_GRB;
		else if (!strcmp(data, "rgb_swap_gbr"))
			pinfo->mipi.rgb_swap = DSI_RGB_SWAP_GBR;
	}
	pinfo->mipi.data_lane0 = of_property_read_bool(np,
		"qcom,mdss-dsi-lane-0-state");
	pinfo->mipi.data_lane1 = of_property_read_bool(np,
		"qcom,mdss-dsi-lane-1-state");
	pinfo->mipi.data_lane2 = of_property_read_bool(np,
		"qcom,mdss-dsi-lane-2-state");
	pinfo->mipi.data_lane3 = of_property_read_bool(np,
		"qcom,mdss-dsi-lane-3-state");

	rc = mdss_panel_parse_display_timings(np,
					&ctrl_pdata->panel_data);
	if (rc)
		return rc;

	rc = mdss_dsi_parse_hdr_settings(np, pinfo);
	if (rc)
		return rc;

	pinfo->mipi.rx_eot_ignore = of_property_read_bool(np,
		"qcom,mdss-dsi-rx-eot-ignore");
	pinfo->mipi.tx_eot_append = of_property_read_bool(np,
		"qcom,mdss-dsi-tx-eot-append");

	rc = of_property_read_u32(np, "qcom,mdss-dsi-stream", &tmp);
	pinfo->mipi.stream = !rc ? tmp : 0;

	data = of_get_property(np,
		"qcom,mdss-dsi-panel-mode-sel-gpio-state", NULL);
	if (data) {
		if (!strcmp(data, "high"))
			pinfo->mode_sel_state = MODE_GPIO_HIGH;
		else if (!strcmp(data, "low"))
			pinfo->mode_sel_state = MODE_GPIO_LOW;
		else if (!strcmp(data, "single_port"))
			pinfo->mode_sel_state = MODE_SEL_SINGLE_PORT;
		else if (!strcmp(data, "dual_port"))
			pinfo->mode_sel_state = MODE_SEL_DUAL_PORT;
	} else {
		pinfo->mode_sel_state = MODE_SEL_DUAL_PORT;
	}

	rc = of_property_read_u32(np, "qcom,mdss-mdp-transfer-time-us", &tmp);
	pinfo->mdp_transfer_time_us = (!rc ? tmp : DEFAULT_MDP_TRANSFER_TIME);

	mdss_dsi_parse_mdp_kickoff_threshold(np, pinfo);

	pinfo->mipi.lp11_init = of_property_read_bool(np,
					"qcom,mdss-dsi-lp11-init");
	rc = of_property_read_u32(np, "qcom,mdss-dsi-init-delay-us",
			&tmp);
	pinfo->mipi.init_delay = (!rc ? tmp : 0);

	rc = of_property_read_u32(np, "qcom,mdss-dsi-post-init-delay", &tmp);
	pinfo->mipi.post_init_delay = (!rc ? tmp : 0);


	data = of_get_property(np,
		"somc,platform-regulator-settings", &len);
	if (!data || len != 7) {
		pr_info("%s:%d, Unable to read SoMC Phy regulator "
					"settings", __func__, __LINE__);
	} else {
		pinfo->mipi.dsi_phy_db.regulator_len = len;
		for (i = 0; i < len; i++)
			pinfo->mipi.dsi_phy_db.regulator[i] = data[i];
	}

	data = of_get_property(np,
		"somc,mdss-dsi-lane-config", &len);
	if ((data) && (len == 45)) {
		for (i = 0; i < len; i++) {
			pinfo->mipi.dsi_phy_db.lanecfg[i] =
					data[i];
		}
		pinfo->mipi.dsi_phy_db.lanecfg_len = len;
	} else
		pr_err("%s:%d, Unable to read Phy lane configure\n",
			__func__, __LINE__);

	mdss_dsi_parse_trigger(np, &(pinfo->mipi.mdp_trigger),
		"qcom,mdss-dsi-mdp-trigger");

	mdss_dsi_parse_trigger(np, &(pinfo->mipi.dma_trigger),
		"qcom,mdss-dsi-dma-trigger");

	mdss_dsi_parse_reset_seq(np, pinfo->rst_seq,
		&(pinfo->rst_seq_len),
		"qcom,mdss-dsi-reset-sequence");

	mdss_dsi_parse_dcs_cmds(np,
		&ctrl_pdata->off_cmds,
		"qcom,mdss-dsi-off-command",
		"qcom,mdss-dsi-off-command-state");

	pinfo->mipi.force_clk_lane_hs = of_property_read_bool(np,
		"qcom,mdss-dsi-force-clock-lane-hs");

	rc = of_property_read_u32_array(np,
		"somc,mdss-phy-size-mm", res, 2);
	if (rc)
		pr_err("%s:%d, panel physical size not specified\n",
							__func__, __LINE__);
	pinfo->width = !rc ? res[0] : 0;
	pinfo->height = !rc ? res[1] : 0;

	rc = of_property_read_u32(np,
		"somc,mdss-dsi-wait-time-before-on-cmd", &tmp);
	pinfo->wait_time_before_on_cmd = !rc ? tmp : 0;

	rc = of_property_read_u32(np,
		"somc,mdss-dsi-disp-on-in-hs", &tmp);
	pinfo->disp_on_in_hs = !rc ? tmp : 0;

	rc = of_property_read_u32(np,
		"somc,mdss-dsi-init-from-begin", &tmp);
	spec_pdata->init_from_begin = !rc ? tmp : 0;

	rc = of_property_read_u32(np, "somc,disp-en-on-pre", &tmp);
	spec_pdata->on_seq.disp_en_pre = !rc ? tmp : 0;
	rc = of_property_read_u32(np, "somc,disp-en-on-post", &tmp);
	spec_pdata->on_seq.disp_en_post = !rc ? tmp : 0;
	(void)mdss_dsi_property_read_u32_var(np,
		"somc,pw-on-rst-seq",
		(u32 **)&spec_pdata->on_seq.rst_seq,
		&spec_pdata->on_seq.seq_num);
	rc = of_property_read_u32(np, "somc,disp-en-off-pre", &tmp);
	spec_pdata->off_seq.disp_en_pre = !rc ? tmp : 0;
	rc = of_property_read_u32(np, "somc,disp-en-off-post", &tmp);
	spec_pdata->off_seq.disp_en_post = !rc ? tmp : 0;
	(void)mdss_dsi_property_read_u32_var(np,
		"somc,pw-off-rst-seq",
		(u32 **)&spec_pdata->off_seq.rst_seq,
		&spec_pdata->off_seq.seq_num);
	(void)mdss_dsi_property_read_u32_var(np,
		"somc,pw-off-rst-b-seq",
		(u32 **)&spec_pdata->off_seq.rst_b_seq,
		&spec_pdata->off_seq.seq_b_num);
	rc = of_property_read_u32(np, "somc,pw-down-period", &tmp);
	spec_pdata->down_period = !rc ? tmp : 0;

	rc = somc_panel_vregs_dt(np, ctrl_pdata);
	if (rc)
		pr_err("%s: Failed to parse lab/ibb vregs DT!!\n", __func__);

	spec_pdata->pwron_reset = of_property_read_bool(np,
					"somc,panel-pwron-reset");

	spec_pdata->dsi_seq_hack = of_property_read_bool(np,
					"somc,dsi-restart-hack");

	rc = mdss_dsi_parse_panel_features(np, ctrl_pdata);
	if (rc)
		pr_err("%s: failed to parse panel features\n",
							__func__);

	mdss_dsi_parse_panel_horizontal_line_idle(np, ctrl_pdata);

	mdss_dsi_parse_dfps_config(np, ctrl_pdata);

	pinfo->is_dba_panel = of_property_read_bool(np,
			"qcom,dba-panel");

	if (pinfo->is_dba_panel) {
		bridge_chip_name = of_get_property(np,
			"qcom,bridge-name", &len);
		if (!bridge_chip_name || len <= 0) {
			pr_err("%s:%d Unable to read qcom,bridge_name, data=%p,len=%d\n",
				__func__, __LINE__, bridge_chip_name, len);
			rc = -EINVAL;
			goto error;
		}
		strlcpy(ctrl_pdata->bridge_name, bridge_chip_name,
			MSM_DBA_CHIP_NAME_MAX_LEN);
	}

	if (pinfo->pdest == pinfo->dsi_master)
	somc_panel_parse_dt_colormgr_config(np, ctrl_pdata);
	if (rc)
		pr_err("%s: Failed to parse Color Manager configuration\n",
			__func__);
	somc_panel_parse_dt_chgfps_config(np, ctrl_pdata);

	rc = mdss_dsi_parse_polling_config(np, ctrl_pdata);
	if (rc)
		pr_err("%s: Failed to parse polling configuration!!\n",
							__func__);

	mdss_dsi_parse_dcs_cmds(np, &ctrl_pdata->post_panel_on_cmds,
		"qcom,mdss-dsi-post-panel-on-command", NULL);

	rc = of_property_read_u32(np,
		"qcom,mdss-dsi-host-esc-clk-freq-hz",
		&pinfo->esc_clk_rate_hz);
	if (rc)
		pinfo->esc_clk_rate_hz = MDSS_DSI_MAX_ESC_CLK_RATE_HZ;

	spec_pdata->gpios_requested = of_property_read_bool(np,
			"somc,reset-gpio-bad-design-quirk");

	return spec_pdata->parse_specific_dt(np, ctrl_pdata);

error:
	return -EINVAL;
}

int mdss_dsi_panel_gpios(struct device_node *node,
		struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{
	struct mdss_panel_specific_pdata *spec_pdata = ctrl_pdata->spec_pdata;
	int rc = 0;

	rc = of_get_named_gpio(node,
			"somc,pwr-positive5-gpio", 0);
	if (gpio_is_valid(rc)) {
		spec_pdata->disp_p5 = rc;

		rc = gpio_request(spec_pdata->disp_p5, "disp_p5");
		if (rc)
			pr_warn("Error requesting P5 GPIO\n");
	} else
		pr_warn("%s:%d Unable to get valid DISP P5 GPIO\n",
						__func__, __LINE__);

	rc = of_get_named_gpio(node,
			"somc,pwr-negative5-gpio", 0);
	if (gpio_is_valid(rc)) {
		spec_pdata->disp_n5 = rc;

		rc = gpio_request(spec_pdata->disp_n5, "disp_n5");
		if (rc)
			pr_warn("Error requesting N5 GPIO\n");
	} else
		pr_warn("%s:%d Unable to get valid DISP N5 GPIO\n",
						__func__, __LINE__);

	return rc;
}

static int mdss_dsi_panel_create_fs(struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{
	int rc = 0;
	char *path_name = "mdss_dsi_panel";

	dev_set_name(&virtdev, "%s", path_name);
	rc = device_register(&virtdev);
	if (rc) {
		pr_err("%s: device_register rc = %d\n", __func__, rc);
		return rc;
	}

	rc = register_attributes(&virtdev);
	if (rc) {
		pr_err("%s: register_attributes rc = %d\n", __func__, rc);
		goto error;
	}

	dev_set_drvdata(&virtdev, ctrl_pdata);

	return 0;
error:
	device_unregister(&virtdev);
	return rc;
}

#if 0
static void mdss_dsi_set_prim_panel(struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{
	struct mdss_dsi_ctrl_pdata *octrl = NULL;
	struct mdss_panel_info *pinfo;

	pinfo = &ctrl_pdata->panel_data.panel_info;

	/*
	 * for Split and Single DSI case default is always primary
	 * and for Dual dsi case below assumptions are made.
	 *	1. DSI controller with bridge chip is always secondary
	 *	2. When there is no brigde chip, DSI1 is secondary
	 */
	pinfo->is_prim_panel = true;
	if (mdss_dsi_is_hw_config_dual(ctrl_pdata->shared_data)) {
		if (pinfo->is_dba_panel) {
			pinfo->is_prim_panel = false;
		} else if (mdss_dsi_is_right_ctrl(ctrl_pdata)) {
			octrl = mdss_dsi_get_other_ctrl(ctrl_pdata);
			if (octrl && octrl->panel_data.panel_info.is_prim_panel)
				pinfo->is_prim_panel = false;
			else
				pinfo->is_prim_panel = true;
		}
	}
}
#endif

int mdss_dsi_panel_init(struct device_node *node,
	struct mdss_dsi_ctrl_pdata *ctrl_pdata,
	int ndx)
{
	int rc = 0;
	struct device_node *dsi_ctrl_np = NULL;
	struct device_node *parent = NULL;
	struct platform_device *ctrl_pdev = NULL;
	struct platform_device *pdev;
	struct mdss_panel_info *pinfo;
	struct poll_ctrl *polling = NULL;
	struct mdss_panel_specific_pdata *spec_pdata = NULL;
	u32 index;

	if (!node || !ctrl_pdata) {
		pr_err("%s: Invalid arguments\n", __func__);
		return -ENODEV;
	}

	spec_pdata = ctrl_pdata->spec_pdata;
	if (!spec_pdata) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	pinfo = &ctrl_pdata->panel_data.panel_info;
	pdev = ctrl_pdata->panel_data.panel_pdev;

	rc = of_property_read_u32(pdev->dev.of_node, "cell-index", &index);
	if (rc) {
		dev_err(&pdev->dev,
			"%s: Cell-index not specified, rc=%d\n",
						__func__, rc);
		//return rc;
	}

	if (!index) {
		rc = mdss_dsi_panel_create_fs(ctrl_pdata);
		if (rc) {
			pr_err("%s: mdss_dsi_panel_create_fs rc = %d\n",
								__func__, rc);
			return rc;
		}
	}

	pr_debug("%s:%d, debug info", __func__, __LINE__);

	dsi_ctrl_np = of_parse_phandle(node,
		"qcom,mdss-dsi-panel-controller", 0);
	if (!dsi_ctrl_np) {
		pr_err("%s: Dsi controller node not initialized\n",
			__func__);
		rc = -EPROBE_DEFER;
		goto error;
	}

	parent = of_get_parent(node);
	ctrl_pdev = of_find_device_by_node(dsi_ctrl_np);

	spec_pdata->vsn_gpio = of_get_named_gpio(parent, "somc,vsn-gpio", 0);
	if (gpio_is_valid(spec_pdata->vsn_gpio)) {
		rc = gpio_request(spec_pdata->vsn_gpio, "lcd_vsn");
		if (rc)
			dev_warn(&virtdev,
			   "%s: Error requesting VSN GPIO %d\n",
					__func__, rc);
		gpio_direction_output(spec_pdata->vsn_gpio, 0);
	} else {
		dev_warn(&virtdev, "%s: spec_pdata->vsn_gpio=%d Invalid\n",
				__func__, spec_pdata->vsn_gpio);
	}

	spec_pdata->vsp_gpio = of_get_named_gpio(parent, "somc,vsp-gpio", 0);
	if (gpio_is_valid(spec_pdata->vsp_gpio)) {
		rc = gpio_request(spec_pdata->vsp_gpio, "lcd_vsp");
		if (rc)
			dev_warn(&virtdev,
			   "%s: Error requesting VSP GPIO %d\n",
					__func__, rc);
		gpio_direction_output(spec_pdata->vsp_gpio, 0);
	} else {
		dev_warn(&virtdev, "%s: spec_pdata->vsp_gpio=%d Invalid\n",
				__func__, spec_pdata->vsn_gpio);
	}

	alt_panelid_cmd = of_property_read_bool(node,
						"somc,alt-panelid-cmd");
	if (alt_panelid_cmd)
		mdss_dsi_panel_gpios(parent, ctrl_pdata);

	rc = incell_panel_driver_init(ctrl_pdata);
	if (rc < 0)
		WARN(1, "Panel specific implementation init FAIL!!\n");

	rc = do_panel_detect(&node, ctrl_pdev,
			ctrl_pdata, alt_panelid_cmd, index);
	if (rc < 0) {
		pr_err("%s: CRITICAL: Panel detection failed!!!\n", __func__);
		return rc;
	}

	rc = mdss_panel_parse_dt(node, ctrl_pdata);
	if (rc) {
		pr_err("%s: CRITICAL: DT parsing failed!!!\n", __func__);
		goto error;
	}

	polling = &spec_pdata->polling;
	if (pinfo->dsi_master == pinfo->pdest) {
		if (polling->enable)
			panel_polling_init(ctrl_pdata);
	}

	//mdss_dsi_set_prim_panel(ctrl_pdata);
	pinfo->dynamic_switch_pending = false;
	pinfo->is_lpm_mode = false;
	pinfo->esd_rdy = false;
	pinfo->persist_mode = false;

	spec_pdata->panel_power_ctrl = mdss_dsi_panel_power_ctrl_ex;
	spec_pdata->reset = mdss_dsi_panel_reset_seq;
	spec_pdata->disp_on = mdss_dsi_panel_disp_on;
	spec_pdata->unblank = mdss_dsi_panel_unblank;

	ctrl_pdata->on = mdss_dsi_panel_on;
	ctrl_pdata->post_panel_on = mdss_dsi_post_panel_on;
	ctrl_pdata->off = mdss_dsi_panel_off;
	ctrl_pdata->low_power_config = mdss_dsi_panel_low_power_config;
	ctrl_pdata->panel_data.set_backlight = mdss_dsi_panel_bl_ctrl;
	ctrl_pdata->panel_data.apply_display_setting =
				mdss_dsi_panel_apply_display_setting;
	ctrl_pdata->switch_mode = mdss_dsi_panel_switch_mode;

	somc_panel_color_manager_init(ctrl_pdata);
	somc_panel_fps_manager_init();

	return 0;
error:
	device_unregister(&virtdev);
	return rc;
}
