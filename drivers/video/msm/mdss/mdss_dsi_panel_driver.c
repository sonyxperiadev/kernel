
/* Copyright (c) 2012-2014, The Linux Foundation. All rights reserved.
 * Copyright (c) 2014 Sony Mobile Communications Inc.
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

#include "mdss_mdp.h"
#include "mdss_dsi.h"

#define DT_CMD_HDR 6

#define DEFAULT_FPS_LOG_INTERVAL 100
#define DEFAULT_FPS_ARRAY_SIZE 120

#define DSI_PCLK_MIN 3300000
#define DSI_PCLK_MAX 223000000
#define DSI_PCLK_DEFAULT 35000000

#define CHANGE_FPS_MIN 36
#define CHANGE_FPS_MAX 63

struct device virtdev;

struct fps_array {
	u32 frame_nbr;
	u32 time_delta;
};

static struct fps_data {
	struct mutex fps_lock;
	u32 log_interval;
	u32 interval_ms;
	struct timespec timestamp_last;
	u32 frame_counter_last;
	u32 frame_counter;
	u32 fpks;
	struct timespec fpks_ts_last;
	u16 fa_last_array_pos;
	struct fps_array fa[DEFAULT_FPS_ARRAY_SIZE];
	u16 fps_array_cnt;
	bool vps_en;
} vpsd, fpsd;

DEFINE_LED_TRIGGER(bl_led_trigger);

#define ADC_PNUM		2
#define ADC_RNG_MIN		0
#define ADC_RNG_MAX		1

static int lcd_id;
static int vsn_gpio, vsp_gpio;
static uint32_t lcdid_adc;
static bool adc_det;
static bool display_on_in_boot;
static bool display_onoff_state;
static unsigned char panel_id[1];
static int lcm_first_boot = -1;
bool alt_panelid_cmd;
static bool mdss_panel_flip_ud = false;
static int mdss_dsi_panel_detect(struct mdss_panel_data *pdata);
static int mdss_panel_parse_dt(struct device_node *np,
				struct mdss_dsi_ctrl_pdata *ctrl_pdata,
				int driver_ic, char *id_data);
static int mdss_dsi_panel_pcc_setup(struct mdss_panel_data *pdata);
static void vsync_handler(struct mdss_mdp_ctl *ctl, ktime_t t);

struct mdss_mdp_vsync_handler vs_handle;

/* pcc data infomation */
#define PANEL_SKIP_ID			0xff
#define UNUSED				0xff
#define CLR_DATA_REG_LEN_RENE_DEFAULT	2
#define CLR_DATA_REG_LEN_NOVA_DEFAULT	1
#define CLR_DATA_REG_LEN_NOVA_AUO	3
#define CLR_DATA_REG_LEN_RENE_SR	1
#define CENTER_U_DATA	30
#define CENTER_V_DATA	30
enum {
	CLR_DATA_UV_PARAM_TYPE_NONE,
	CLR_DATA_UV_PARAM_TYPE_RENE_DEFAULT,
	CLR_DATA_UV_PARAM_TYPE_NOVA_DEFAULT,
	CLR_DATA_UV_PARAM_TYPE_NOVA_AUO,
	CLR_DATA_UV_PARAM_TYPE_RENE_SR
};

static int __init lcdid_adc_setup(char *str)
{
	uint32_t	res;

	if (!*str)
		return 0;
	if (!kstrtoul(str, 0, (unsigned long *)&res)) {
		lcdid_adc = res;
		adc_det = true;
	}

	return 1;
}
__setup("lcdid_adc=", lcdid_adc_setup);

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
	ctrl->pwm_bl = pwm_request(ctrl->pwm_lpg_chan, "lcd-bklt");
	if (ctrl->pwm_bl == NULL || IS_ERR(ctrl->pwm_bl)) {
		pr_err("%s: Error: lpg_chan=%d pwm request failed",
				__func__, ctrl->pwm_lpg_chan);
	}
}

static void mdss_dsi_panel_bklt_pwm(struct mdss_dsi_ctrl_pdata *ctrl, int level)
{
	int ret;
	u32 duty;

	if (ctrl->pwm_bl == NULL) {
		pr_err("%s: no PWM\n", __func__);
		return;
	}

	duty = level * ctrl->pwm_period;
	duty /= ctrl->bklt_max;

	pr_debug("%s: bklt_ctrl=%d pwm_period=%d pwm_gpio=%d pwm_lpg_chan=%d\n",
			__func__, ctrl->bklt_ctrl, ctrl->pwm_period,
				ctrl->pwm_pmic_gpio, ctrl->pwm_lpg_chan);

	pr_debug("%s: ndx=%d level=%d duty=%d\n", __func__,
					ctrl->ndx, level, duty);

	ret = pwm_config_us(ctrl->pwm_bl, duty, ctrl->pwm_period);
	if (ret) {
		pr_err("%s: pwm_config_us() failed err=%d.\n", __func__, ret);
		return;
	}

	ret = pwm_enable(ctrl->pwm_bl);
	if (ret)
		pr_err("%s: pwm_enable() failed err=%d\n", __func__, ret);
}

static char __attribute__((__unused__)) dcs_cmd[2] = {0x54, 0x00}; /* DTYPE_DCS_READ */
static struct __attribute__((__unused__)) dsi_cmd_desc dcs_read_cmd = {
	{DTYPE_DCS_READ, 1, 0, 1, 5, sizeof(dcs_cmd)},
	dcs_cmd
};

static char dcs_cmd_DA[2] = {0xDA, 0x00}; /* DTYPE_DCS_READ */
static struct dsi_cmd_desc dcs_read_cmd_DA = {
	{DTYPE_DCS_READ, 1, 0, 1, 20, sizeof(dcs_cmd_DA)},
	dcs_cmd_DA
};

static void panel_id_store(int data)
{
	pr_info("panel_id_store data 0x%x pid 0x%x\n", data, panel_id[0]);
	panel_id[0] = data;
}

u32 mdss_dsi_panel_cmd_read(struct mdss_dsi_ctrl_pdata *ctrl, char cmd0,
				char cmd1, struct dsi_cmd_desc cmd_desc,
				void (*fxn)(int), int len)
{
	struct dcs_cmd_req cmdreq;

	dcs_cmd[0] = cmd0;
	dcs_cmd[1] = cmd1;
	memset(&cmdreq, 0, sizeof(cmdreq));
	cmdreq.cmds = &cmd_desc;
	cmdreq.cmds_cnt = 1;
	cmdreq.flags = CMD_REQ_RX | CMD_REQ_COMMIT;
	cmdreq.rlen = len;
	cmdreq.rbuf = ctrl->rx_buf.data;
	cmdreq.cb = fxn; /* call back */
	mdss_dsi_cmdlist_put(ctrl, &cmdreq);
	/*
	 * blocked here, until call back called
	 */

	return 0;
}

static void mdss_dsi_panel_cmds_send(struct mdss_dsi_ctrl_pdata *ctrl,
			struct dsi_panel_cmds *pcmds)
{
	struct dcs_cmd_req cmdreq;

	memset(&cmdreq, 0, sizeof(cmdreq));
	cmdreq.cmds = pcmds->cmds;
	cmdreq.cmds_cnt = pcmds->cmd_cnt;
	cmdreq.flags = CMD_REQ_COMMIT;

	/*Panel ON/Off commands should be sent in DSI Low Power Mode*/
	if (pcmds->link_state == DSI_LP_MODE)
		cmdreq.flags |= CMD_REQ_LP_MODE;

	cmdreq.rlen = 0;
	cmdreq.cb = NULL;

	mdss_dsi_cmdlist_put(ctrl, &cmdreq);
}

static char led_pwm1[2] = {0x51, 0x0};	/* DTYPE_DCS_WRITE1 */
static struct dsi_cmd_desc backlight_cmd = {
	{DTYPE_DCS_WRITE1, 1, 0, 0, 1, sizeof(led_pwm1)},
	led_pwm1
};

static void mdss_dsi_panel_bklt_dcs(struct mdss_dsi_ctrl_pdata *ctrl, int level)
{
	struct dcs_cmd_req cmdreq;

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

static int mdss_dsi_request_gpios(struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{
	int rc = 0;
	static bool requested;

	if (requested)
		return 0;

	if (gpio_is_valid(ctrl_pdata->disp_en_gpio)) {
		rc = gpio_request(ctrl_pdata->disp_en_gpio,
						"disp_enable");
		if (rc) {
			pr_err("request disp_en gpio failed, rc=%d\n",
				       rc);
			goto disp_en_gpio_err;
		}
	}
	rc = gpio_request(ctrl_pdata->rst_gpio, "disp_rst_n");
	if (rc) {
		pr_err("request reset gpio failed, rc=%d\n",
			rc);
		goto rst_gpio_err;
	}
	if (gpio_is_valid(ctrl_pdata->mode_gpio)) {
		rc = gpio_request(ctrl_pdata->mode_gpio, "panel_mode");
		if (rc) {
			pr_err("request panel mode gpio failed,rc=%d\n",
								rc);
			goto mode_gpio_err;
		}
	}

	requested = true;
	return rc;

mode_gpio_err:
	gpio_free(ctrl_pdata->rst_gpio);
rst_gpio_err:
	if (gpio_is_valid(ctrl_pdata->disp_en_gpio))
		gpio_free(ctrl_pdata->disp_en_gpio);
disp_en_gpio_err:
	return rc;
}


static void mdss_dsi_panel_set_gpio_seq(
		int gpio, int seq_num, const int *seq)
{
	int i;

	for (i = 0; i + 1 < seq_num; i += 2) {
		gpio_set_value(gpio, seq[i]);
		usleep_range(seq[i + 1] * 1000, seq[i + 1] * 1000 + 100);
		pr_debug("%s: enable=%d, wait=%dms\n",
			__func__, seq[i], seq[i+1]);
	}
}

static int mdss_dsi_panel_reset_seq(struct mdss_panel_data *pdata, int enable)
{
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

	mdss_dsi_request_gpios(ctrl_pdata);

	pw_seq = (enable) ? &ctrl_pdata->spec_pdata->on_seq :
				&ctrl_pdata->spec_pdata->off_seq;
	mdss_dsi_panel_set_gpio_seq(ctrl_pdata->rst_gpio,
		pw_seq->seq_b_num, pw_seq->rst_b_seq);
	if (pw_seq->disp_en_pre)
		usleep_range(pw_seq->disp_en_pre * 1000,
				pw_seq->disp_en_pre * 1000 + 100);
	if (gpio_is_valid(ctrl_pdata->disp_en_gpio))
		gpio_set_value(ctrl_pdata->disp_en_gpio, enable);

	if (pw_seq->disp_en_post)
		usleep_range(pw_seq->disp_en_post * 1000,
				pw_seq->disp_en_post * 1000 + 100);
	if (gpio_is_valid(ctrl_pdata->mode_gpio) && enable) {
		if (pinfo->mode_gpio_state == MODE_GPIO_HIGH)
			gpio_set_value(ctrl_pdata->mode_gpio, 1);
		else if (pinfo->mode_gpio_state == MODE_GPIO_LOW)
			gpio_set_value(ctrl_pdata->mode_gpio, 0);
	}

	if (alt_panelid_cmd)
	{
		if (enable) {
			msleep(1);
			if (gpio_is_valid(spec_pdata->disp_p5)) {
				gpio_direction_output((spec_pdata->disp_p5), 1);
				msleep(1);
			}
			if (gpio_is_valid(spec_pdata->disp_n5)) {
				gpio_direction_output((spec_pdata->disp_n5), 1);
				msleep(10);
			}

			if (ctrl_pdata->ctrl_state & CTRL_STATE_PANEL_INIT) {
				pr_info("panel not properly turned off\n");
				ctrl_pdata->ctrl_state &= ~CTRL_STATE_PANEL_INIT;
				pr_info("reset panel done\n");
			}

			gpio_direction_output(ctrl_pdata->rst_gpio, 1);
			msleep(150);
		} else {
			gpio_direction_output(ctrl_pdata->rst_gpio, 0);
			msleep(1);
			gpio_direction_output(spec_pdata->disp_n5, 0);
			msleep(1);
			gpio_direction_output(spec_pdata->disp_p5, 0);
			msleep(10);
		}
	} else
		mdss_dsi_panel_set_gpio_seq(ctrl_pdata->rst_gpio,
			pw_seq->seq_num, pw_seq->rst_seq);

	return 0;
}

int mdss_dsi_panel_reset(struct mdss_panel_data *pdata, int enable)
{
	return mdss_dsi_panel_reset_seq(pdata, enable);
}

static char caset[] = {0x2a, 0x00, 0x00, 0x03, 0x00};	/* DTYPE_DCS_LWRITE */
static char paset[] = {0x2b, 0x00, 0x00, 0x05, 0x00};	/* DTYPE_DCS_LWRITE */

static struct dsi_cmd_desc partial_update_enable_cmd[] = {
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 1, sizeof(caset)}, caset},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 1, sizeof(paset)}, paset},
};

static int mdss_dsi_panel_partial_update(struct mdss_panel_data *pdata)
{
	struct mipi_panel_info *mipi;
	struct mdss_dsi_ctrl_pdata *ctrl = NULL;
	struct dcs_cmd_req cmdreq;
	int rc = 0;

	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	ctrl = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);
	mipi  = &pdata->panel_info.mipi;

	pr_debug("%s: ctrl=%p ndx=%d\n", __func__, ctrl, ctrl->ndx);

	caset[1] = (((pdata->panel_info.roi_x) & 0xFF00) >> 8);
	caset[2] = (((pdata->panel_info.roi_x) & 0xFF));
	caset[3] = (((pdata->panel_info.roi_x - 1 + pdata->panel_info.roi_w)
								& 0xFF00) >> 8);
	caset[4] = (((pdata->panel_info.roi_x - 1 + pdata->panel_info.roi_w)
								& 0xFF));
	partial_update_enable_cmd[0].payload = caset;

	paset[1] = (((pdata->panel_info.roi_y) & 0xFF00) >> 8);
	paset[2] = (((pdata->panel_info.roi_y) & 0xFF));
	paset[3] = (((pdata->panel_info.roi_y - 1 + pdata->panel_info.roi_h)
								& 0xFF00) >> 8);
	paset[4] = (((pdata->panel_info.roi_y - 1 + pdata->panel_info.roi_h)
								& 0xFF));
	partial_update_enable_cmd[1].payload = paset;

	pr_debug("%s: enabling partial update\n", __func__);
	memset(&cmdreq, 0, sizeof(cmdreq));
	cmdreq.cmds = partial_update_enable_cmd;
	cmdreq.cmds_cnt = 2;
	cmdreq.flags = CMD_REQ_COMMIT | CMD_CLK_CTRL;
	cmdreq.rlen = 0;
	cmdreq.cb = NULL;

	mdss_dsi_cmdlist_put(ctrl, &cmdreq);

	return rc;
}

static struct mdss_dsi_ctrl_pdata *get_rctrl_data(struct mdss_panel_data *pdata)
{
	if (!pdata || !pdata->next) {
		pr_err("%s: Invalid panel data\n", __func__);
		return NULL;
	}

	return container_of(pdata->next, struct mdss_dsi_ctrl_pdata,
			panel_data);
}

static void mdss_dsi_panel_bl_ctrl(struct mdss_panel_data *pdata,
							u32 bl_level)
{
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;

	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return;
	}

	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);

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
		mdss_dsi_panel_bklt_dcs(ctrl_pdata, bl_level);
		if (ctrl_pdata->shared_pdata.broadcast_enable &&
				ctrl_pdata->ndx == DSI_CTRL_0) {
			struct mdss_dsi_ctrl_pdata *rctrl_pdata = NULL;
			rctrl_pdata = get_rctrl_data(pdata);
			if (!rctrl_pdata) {
				pr_err("%s: Right ctrl data NULL\n", __func__);
				return;
			}
			mdss_dsi_panel_bklt_dcs(rctrl_pdata, bl_level);
		}
		break;
	default:
		pr_err("%s: Unknown bl_ctrl configuration\n",
			__func__);
		break;
	}
}

static void mdss_dsi_panel_fps_array_clear(struct fps_data *fps)
{
	memset(fps->fa, 0, sizeof(fps->fa));
	fps->fps_array_cnt = 0;
}

static ssize_t mdss_dsi_panel_frame_counter(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%i\n", fpsd.frame_counter);
}

static ssize_t mdss_dsi_panel_frames_per_ksecs(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%i\n", fpsd.fpks);
}

static ssize_t mdss_dsi_panel_vsyncs_per_ksecs_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	if (vpsd.vps_en)
		return scnprintf(buf, PAGE_SIZE, "%i\n", vpsd.fpks);
	else
		return scnprintf(buf, PAGE_SIZE,
			"This function is invalid now.\n"
			"Please read again after writing ON.\n");
}

static ssize_t mdss_dsi_panel_vsyncs_per_ksecs_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	int ret = count;
	long vps_en;
	struct mdss_data_type *mdata = mdss_mdp_get_mdata();
	struct mdss_mdp_ctl *ctl = mdata->ctl_off;

	if (kstrtol(buf, 10, &vps_en)) {
		dev_err(dev, "%s: Error, buf = %s\n", __func__, buf);
		ret = -EINVAL;
		goto exit;
	}

	vs_handle.vsync_handler = (mdp_vsync_handler_t)vsync_handler;
	vs_handle.cmd_post_flush = false;

	if (vps_en) {
		vs_handle.enabled = false;
		if (!vpsd.vps_en && (ctl->add_vsync_handler)) {
			ctl->add_vsync_handler(ctl, &vs_handle);
			vpsd.vps_en = true;
			pr_info("%s: vsyncs_per_ksecs is valid\n", __func__);
		}
	} else {
		vs_handle.enabled = true;
		if (vpsd.vps_en && (ctl->remove_vsync_handler)) {
			ctl->remove_vsync_handler(ctl, &vs_handle);
			vpsd.vps_en = false;
			fpsd.fpks = 0;
			pr_info("%s: vsyncs_per_ksecs is invalid\n", __func__);
		}
	}
exit:
	return ret;
}

static ssize_t mdss_dsi_panel_interval_ms_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%i\n", fpsd.interval_ms);
}

static ssize_t mdss_dsi_panel_log_interval_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%i\n", fpsd.log_interval);
}

static ssize_t mdss_dsi_panel_log_interval_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	int ret = count;

	if (sscanf(buf, "%4i", &fpsd.log_interval) != 1) {
		pr_err("%s: Error, buf = %s\n", __func__, buf);
		ret = -EINVAL;
	}
	return ret;
}

#define DEBUG_INTERVAL_ARRAY
static ssize_t mdss_dsi_panel_interval_array_ms(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	u16 i, len, rc = 0;
	char *tmp = buf;

	mutex_lock(&fpsd.fps_lock);
	len = fpsd.fa_last_array_pos;
	/* Get the first frames from the buffer */
	for (i = len + 1; i < DEFAULT_FPS_ARRAY_SIZE; i++) {
		if (fpsd.fa[i].time_delta) {
#ifdef DEBUG_INTERVAL_ARRAY
			/* FrameNumber, buf idx and delta time */
			rc += scnprintf(tmp + rc, PAGE_SIZE - rc ,
							"%03i[%03i]: %i,\n",
							fpsd.fa[i].frame_nbr, i,
							fpsd.fa[i].time_delta);
#else
			rc += scnprintf(tmp + rc, PAGE_SIZE - rc ,
						"%i, ", fpsd.fa[i].time_delta);
#endif
		}
	}
	/* Get the rest frames from the buffer */
	if (len) {
		for (i = 0; i <= len; i++) {
			if (fpsd.fa[i].time_delta) {
#ifdef DEBUG_INTERVAL_ARRAY
				/* FrameNumber, buf idx and delta time */
				rc += scnprintf(tmp + rc, PAGE_SIZE - rc ,
							"%03i[%03i]: %i,\n",
							fpsd.fa[i].frame_nbr, i,
							fpsd.fa[i].time_delta);
#else
				rc += scnprintf(tmp + rc, PAGE_SIZE - rc ,
						"%i, ", fpsd.fa[i].time_delta);
#endif
			}
		}
	}
	rc += scnprintf(tmp + rc, PAGE_SIZE - rc , "\n");

	/* Clear the buffer once it is read */
	mdss_dsi_panel_fps_array_clear(&fpsd);
	mutex_unlock(&fpsd.fps_lock);

	mutex_lock(&vpsd.fps_lock);
	/* Clear the buffer once it is read */
	mdss_dsi_panel_fps_array_clear(&vpsd);
	mutex_unlock(&vpsd.fps_lock);

	return rc;
}

static int mdss_dsi_panel_read_cabc(struct device *dev)
{
	struct platform_device *pdev = NULL;
	struct mdss_dsi_ctrl_pdata *ctrl_pdata;

	pdev = container_of(dev, struct platform_device, dev);

	ctrl_pdata = platform_get_drvdata(pdev);
	if (!ctrl_pdata) {
		dev_err(dev, "%s(%d): no panel connected\n",
							__func__, __LINE__);
		goto exit;
	}

	return ctrl_pdata->spec_pdata->cabc_enabled;
exit:
	return -EINVAL;
}

static void mdss_dsi_panel_write_cabc(struct device *dev, int enable)
{
	struct mdss_dsi_ctrl_pdata *ctrl_pdata;

	ctrl_pdata = dev_get_drvdata(dev);
	if (!ctrl_pdata) {
		dev_err(dev, "%s(%d): no panel connected\n",
							__func__, __LINE__);
		goto exit;
	}

	ctrl_pdata->spec_pdata->cabc_enabled = enable;

exit:
	return;
}

static ssize_t mdss_dsi_panel_cabc_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	int tmp;
	tmp = mdss_dsi_panel_read_cabc(dev);
	return scnprintf(buf, PAGE_SIZE, "%i\n", tmp);
}

static ssize_t mdss_dsi_panel_cabc_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	int ret = count;
	long tmp;

	if (kstrtol(buf, 10, &tmp)) {
		dev_err(dev, "%s: Error, buf = %s\n", __func__, buf);
		ret = -EINVAL;
		goto exit;
	}
	mdss_dsi_panel_write_cabc(dev, tmp);
exit:
	return ret;
}

static ssize_t mdss_dsi_panel_id_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = dev_get_drvdata(dev);
	char const *id = ctrl_pdata->panel_data.panel_info.panel_id_name ?
		ctrl_pdata->panel_data.panel_info.panel_id_name : "default";

	return scnprintf(buf, PAGE_SIZE, "%s\n", id);
}

static ssize_t mdss_dsi_panel_pcc_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = dev_get_drvdata(dev);
	struct mdss_pcc_data *pcc_data = &ctrl_pdata->spec_pdata->pcc_data;
	u32 r, g, b;

	r = g = b = 0;
	if (!pcc_data->color_tbl)
		goto exit;
	if (pcc_data->u_data == 0 && pcc_data->v_data == 0)
		goto exit;
	if (pcc_data->tbl_idx >= pcc_data->tbl_size)
		goto exit;
	if (pcc_data->color_tbl[pcc_data->tbl_idx].color_type == UNUSED)
		goto exit;
	r = pcc_data->color_tbl[pcc_data->tbl_idx].r_data;
	g = pcc_data->color_tbl[pcc_data->tbl_idx].g_data;
	b = pcc_data->color_tbl[pcc_data->tbl_idx].b_data;
exit:
	return scnprintf(buf, PAGE_SIZE, "0x%x 0x%x 0x%x ", r, g, b);
}

#define CHENGE_FPS_REG(a, b) (ctrl_pdata->fps_cmds.cmds[a].payload[b])
#define TE_PAYLOAD_1(a, b) (ctrl_pdata->fps_cmds.cmds[a].payload[b])
#define TE_PAYLOAD_2(a, b) (ctrl_pdata->fps_cmds.cmds[a].payload[(b + 1)])
#define TE_CMD_SIZE(a) (ctrl_pdata->fps_cmds.cmds[a].dchdr.dlen)

static int mdss_dsi_panel_change_fps_fpks_calc
		(struct mdss_dsi_ctrl_pdata *ctrl_pdata, int dfpks) {
	int dfps;
	int dfpks_rev;
	int rc = 0;
	u32 vt, v_total, cur_vfp;
	u32 vfp, vbp, yres, clk;
	u32 te_c_update, tec_cmds, tec_payload;
	u32 fps_cmds, fps_payload;
	static int line_us;
	struct mdss_panel_specific_pdata *spec_pdata = NULL;
	struct mdss_data_type *mdata = mdss_mdp_get_mdata();
	struct msm_fb_data_type *mfd = mdata->ctl_off->mfd;
	struct mdss_overlay_private *mdp5_data = mfd_to_mdp5_data(mfd);
	struct mdss_panel_info *pinfo = &ctrl_pdata->panel_data.panel_info;
	char rtn;

	if (!mdp5_data->ctl || !mdp5_data->ctl->power_on)
		return 0;

	if (!display_onoff_state) {
		pr_err("%s: Disp-On still hasn't been completed. Please retry\n"
			, __func__);
		return -EINVAL;
	}

	if (dfpks == pinfo->mipi.input_fpks) {
		pr_info("%s: fpks is already %d\n", __func__, dfpks);
		return 0;
	}

	dfps = dfpks / 1000;

	spec_pdata = ctrl_pdata->spec_pdata;
	if (!spec_pdata) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	if (pinfo->mipi.mode == DSI_CMD_MODE) {
		if (!ctrl_pdata->fps_cmds.cmd_cnt) {
			pr_err("%s: change fps isn't supported\n", __func__);
			return -EINVAL;
		}

		clk = pinfo->lcdc.display_clock;
		vbp = pinfo->lcdc.driver_ic_vbp;
		vfp = pinfo->lcdc.driver_ic_vfp;
		yres = pinfo->yres;

		rtn = (char)(clk / (dfpks * (yres + vbp + vfp) / 1000));
		pr_debug("%s: [fpks]: reg value = %08x\n", __func__, rtn);
		dfpks_rev = (clk / (rtn * (yres + vbp + vfp) / 1000));
		dfps = dfpks_rev / 1000;

		pr_info("%s: clk=%d vbp=%d vfp=%d yres=%d rtn=0x%x\n",
			__func__, clk, vbp, vfp, yres, rtn);

		fps_cmds = pinfo->lcdc.chenge_fps_cmds_num;
		fps_payload = pinfo->lcdc.chenge_fps_payload_num;
		CHENGE_FPS_REG(fps_cmds, fps_payload) = rtn;

		pinfo->mipi.frame_rate = dfps;

		te_c_update = pinfo->lcdc.te_c_update;
		if (te_c_update) {
			tec_cmds = pinfo->lcdc.te_c_cmds_num;
			tec_payload = pinfo->lcdc.te_c_payload_num;
			if (dfpks > pinfo->lcdc.fps_threshold) {
				/* 60fps Setting */
				TE_PAYLOAD_1(tec_cmds, tec_payload)
				 = (char)pinfo->lcdc.te_c_mode_60fps_0;
				if (TE_CMD_SIZE(tec_cmds) == 3)
					TE_PAYLOAD_2(tec_cmds, tec_payload)
					 = (char)pinfo->lcdc.te_c_mode_60fps_1;
			} else {
				/* 45fps Setting */
				TE_PAYLOAD_1(tec_cmds, tec_payload)
				 = (char)pinfo->lcdc.te_c_mode_45fps_0;
				if (TE_CMD_SIZE(tec_cmds) == 3)
					TE_PAYLOAD_2(tec_cmds, tec_payload)
					 = (char)pinfo->lcdc.te_c_mode_45fps_1;
			}

			if (TE_CMD_SIZE(tec_cmds) == 3)
				pr_debug("%s: TE C Command:0x%02x 0x%02x\n",
					__func__,
					TE_PAYLOAD_1(tec_cmds, tec_payload),
					TE_PAYLOAD_2(tec_cmds, tec_payload));
			else
				pr_debug("%s: TE C Command:0x%02x\n",
					__func__,
					TE_PAYLOAD_1(tec_cmds, tec_payload));
		}

		pr_debug("%s: fps change sequence\n", __func__);
		mdss_dsi_panel_cmds_send(ctrl_pdata, &ctrl_pdata->fps_cmds);

		pr_info("%s: change fpks=%d\n", __func__, dfpks);

	} else {
		if (!line_us) {
			vt = mdss_panel_get_vtotal(&ctrl_pdata
						    ->panel_data.panel_info);
			line_us = (NSEC_PER_SEC / 60) / vt;
		}
		v_total = (NSEC_PER_SEC / dfps) / line_us;
		vfp = v_total
			- (pinfo->lcdc.v_back_porch
			+  pinfo->lcdc.v_pulse_width
			+  pinfo->yres);

		spec_pdata->new_vfp = vfp;
		cur_vfp = pinfo->lcdc.v_front_porch;
		pinfo->lcdc.v_front_porch = vfp;

		rc = mdss_dsi_clk_div_config(pinfo, dfps);
		ctrl_pdata->pclk_rate = pinfo->mipi.dsi_pclk_rate;
		ctrl_pdata->byte_clk_rate = pinfo->clk_rate / 8;
		pinfo->lcdc.v_front_porch = cur_vfp;
		pr_info("%s: change fps=%d vfp=%d\n", __func__, dfps,
				spec_pdata->new_vfp);
	}
	pinfo->new_fps         = dfps;
	pinfo->mipi.input_fpks = dfpks;

	return 0;
}

static ssize_t mdss_dsi_panel_change_fpks_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = dev_get_drvdata(dev);
	int dfpks, rc;

	rc = kstrtoint(buf, 10, &dfpks);
	if (rc < 0) {
		pr_err("%s: Error, buf = %s\n", __func__, buf);
		return rc;
	}

	if (dfpks < 1000 * CHANGE_FPS_MIN
			|| dfpks > 1000 * CHANGE_FPS_MAX) {
		pr_err("%s: invalid value for change_fpks buf = %s\n",
				 __func__, buf);
		return -EINVAL;
	}

	rc = mdss_dsi_panel_change_fps_fpks_calc(ctrl_pdata, dfpks);
	if (rc) {
		pr_err("%s: Error, rc = %d\n", __func__, rc);
		return rc;
	}
	return count;
}

static ssize_t mdss_dsi_panel_change_fpks_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = dev_get_drvdata(dev);
	struct mdss_data_type *mdata = mdss_mdp_get_mdata();
	struct msm_fb_data_type *mfd = mdata->ctl_off->mfd;
	struct mdss_overlay_private *mdp5_data = mfd_to_mdp5_data(mfd);

	if (!mdp5_data->ctl || !mdp5_data->ctl->power_on)
		return 0;

	return scnprintf(buf, PAGE_SIZE, "%d\n",
		ctrl_pdata->panel_data.panel_info.mipi.input_fpks);
}

static ssize_t mdss_dsi_panel_change_fps_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = dev_get_drvdata(dev);
	int dfps, dfpks, rc;

	rc = kstrtoint(buf, 10, &dfps);
	if (rc < 0) {
		pr_err("%s: Error, buf = %s\n", __func__, buf);
		return rc;
	}

	if (dfps >= 1000 * CHANGE_FPS_MIN
			&& dfps <= 1000 * CHANGE_FPS_MAX) {
		dfpks = dfps;
	} else if (dfps >= CHANGE_FPS_MIN && dfps <= CHANGE_FPS_MAX) {
		dfpks = dfps * 1000;
	} else {
		pr_err("%s: invalid value for change_fps buf = %s\n",
				__func__, buf);
		return -EINVAL;
	}

	rc = mdss_dsi_panel_change_fps_fpks_calc(ctrl_pdata, dfpks);
	if (rc) {
		pr_err("%s: Error, rc = %d\n", __func__, rc);
		return rc;
	}

	return count;
}

static ssize_t mdss_dsi_panel_change_fps_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = dev_get_drvdata(dev);
	struct mdss_data_type *mdata = mdss_mdp_get_mdata();
	struct msm_fb_data_type *mfd = mdata->ctl_off->mfd;
	struct mdss_overlay_private *mdp5_data = mfd_to_mdp5_data(mfd);

	if (!mdp5_data->ctl || !mdp5_data->ctl->power_on)
		return 0;

	return scnprintf(buf, PAGE_SIZE, "%d\n",
		ctrl_pdata->panel_data.panel_info.new_fps);
}

static void mdss_dsi_panel_wait_change(
		struct mdss_dsi_ctrl_pdata *ctrl_pdata, bool onoff)
{
	struct mdss_panel_info *pinfo = &ctrl_pdata->panel_data.panel_info;
	struct dsi_cmd_desc *cmds_cmds = NULL;
	char *wait = NULL;
	char wait_60fps, wait_45fps;
	u32 te_c_update = pinfo->lcdc.te_c_update;
	u32 wait_update = pinfo->lcdc.chenge_wait_update;
	u32 fpks = pinfo->mipi.input_fpks;
	u32 cmds_num;

	if (!(te_c_update && wait_update))
		goto exit;

	if (onoff) {
		cmds_num = pinfo->lcdc.chenge_wait_on_cmds_num;
		cmds_cmds = &ctrl_pdata->on_cmds.cmds[cmds_num];
		if (cmds_cmds)
			wait = &ctrl_pdata->on_cmds.cmds[cmds_num].dchdr.wait;
		else
			goto exit;

		wait_60fps = (char)pinfo->lcdc.chenge_wait_on_60fps;
		wait_45fps = (char)pinfo->lcdc.chenge_wait_on_45fps;
	} else {
		cmds_num = pinfo->lcdc.chenge_wait_off_cmds_num;
		cmds_cmds = &ctrl_pdata->off_cmds.cmds[cmds_num];
		if (cmds_cmds)
			wait = &ctrl_pdata->off_cmds.cmds[cmds_num].dchdr.wait;
		else
			goto exit;

		wait_60fps = (char)pinfo->lcdc.chenge_wait_off_60fps;
		wait_45fps = (char)pinfo->lcdc.chenge_wait_off_45fps;
	}

	if (fpks > pinfo->lcdc.fps_threshold)
		*wait = wait_60fps;
	else
		*wait = wait_45fps;
	pr_debug("%s: onoff[%d] wait = %d\n", __func__, onoff, *wait);
exit:
	return;
}

static struct device_attribute panel_attributes[] = {
	__ATTR(frame_counter, S_IRUGO, mdss_dsi_panel_frame_counter, NULL),
	__ATTR(frames_per_ksecs, S_IRUGO,
				mdss_dsi_panel_frames_per_ksecs, NULL),
	__ATTR(vsyncs_per_ksecs, S_IRUSR|S_IRGRP|S_IWUSR|S_IWGRP,
				mdss_dsi_panel_vsyncs_per_ksecs_show,
				mdss_dsi_panel_vsyncs_per_ksecs_store),
	__ATTR(interval_ms, S_IRUGO, mdss_dsi_panel_interval_ms_show, NULL),
	__ATTR(log_interval, S_IRUGO|S_IWUSR|S_IWGRP,
					mdss_dsi_panel_log_interval_show,
					mdss_dsi_panel_log_interval_store),
	__ATTR(interval_array, S_IRUGO,
					mdss_dsi_panel_interval_array_ms, NULL),
	__ATTR(cabc, S_IRUGO|S_IWUSR|S_IWGRP, mdss_dsi_panel_cabc_show,
						mdss_dsi_panel_cabc_store),
	__ATTR(panel_id, S_IRUSR, mdss_dsi_panel_id_show, NULL),
	__ATTR(cc, S_IRUGO, mdss_dsi_panel_pcc_show, NULL),
	__ATTR(change_fps, S_IRUGO|S_IWUSR|S_IWGRP,
					mdss_dsi_panel_change_fps_show,
					mdss_dsi_panel_change_fps_store),
	__ATTR(change_fpks, S_IRUGO|S_IWUSR|S_IWGRP,
					mdss_dsi_panel_change_fpks_show,
					mdss_dsi_panel_change_fpks_store),
};

static int register_attributes(struct device *dev)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(panel_attributes); i++)
		if (device_create_file(dev, panel_attributes + i))
			goto error;
	return 0;
error:
	dev_err(dev, "%s: Unable to create interface\n", __func__);
	for (--i; i >= 0 ; i--)
		device_remove_file(dev, panel_attributes + i);
	return -ENODEV;
}

static int mdss_dsi_panel_on(struct mdss_panel_data *pdata)
{
	struct mipi_panel_info *mipi;
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	struct mdss_panel_specific_pdata *spec_pdata = NULL;
	struct mdss_panel_info *pinfo = NULL;
	u32 fps_cmds, fps_payload;
	char rtn;

	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);

	spec_pdata = ctrl_pdata->spec_pdata;
	if (!spec_pdata) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	if (!spec_pdata->detected && !spec_pdata->init_from_begin && !alt_panelid_cmd)
		return 0;

	mipi = &pdata->panel_info.mipi;

	lcm_first_boot = 0;

	if (spec_pdata->pcc_data.pcc_sts & PCC_STS_UD) {
		mdss_dsi_panel_pcc_setup(pdata);
		spec_pdata->pcc_data.pcc_sts &= ~PCC_STS_UD;
	}

	if (spec_pdata->einit_cmds.cmd_cnt) {
		pr_debug("%s: early init sequence\n", __func__);
		mdss_dsi_panel_cmds_send(ctrl_pdata, &spec_pdata->einit_cmds);
		if (spec_pdata->reset)
			spec_pdata->reset(pdata, 1);
	}

	if (spec_pdata->cabc_early_on_cmds.cmd_cnt &&
					(spec_pdata->cabc_enabled == 1)) {
		pr_debug("%s: early CABC-on sequence\n", __func__);
		mdss_dsi_panel_cmds_send(ctrl_pdata,
			&spec_pdata->cabc_early_on_cmds);
		spec_pdata->cabc_active = 1;
	}

	if (spec_pdata->init_cmds.cmd_cnt) {
		pr_debug("%s: init (exit sleep) sequence\n", __func__);
		mdss_dsi_panel_cmds_send(ctrl_pdata, &spec_pdata->init_cmds);
	}

	if (ctrl_pdata->on_cmds.cmd_cnt && !spec_pdata->disp_on_in_hs) {
		mdss_dsi_panel_wait_change(ctrl_pdata, true);
		pr_debug("%s: panel on sequence (in low speed)\n", __func__);
		mdss_dsi_panel_cmds_send(ctrl_pdata, &ctrl_pdata->on_cmds);
		display_onoff_state = true;
		pr_info("%s: ctrl=%p ndx=%d\n", __func__, ctrl_pdata, ctrl_pdata->ndx);
	}

	if (ctrl_pdata->panel_data.panel_info.mipi.mode == DSI_CMD_MODE) {
		if (ctrl_pdata->fps_cmds.cmd_cnt) {
			pinfo = &ctrl_pdata->panel_data.panel_info;
			fps_cmds = pinfo->lcdc.chenge_fps_cmds_num;
			fps_payload = pinfo->lcdc.chenge_fps_payload_num;
			rtn = CHENGE_FPS_REG(fps_cmds, fps_payload);
			pr_debug("%s: change fps sequence --- rtn = 0x%x\n",
				__func__, rtn);
			mdss_dsi_panel_cmds_send(ctrl_pdata,
						&ctrl_pdata->fps_cmds);
		}
	}

	pr_debug("%s:-\n", __func__);
	return 0;
}

static int mdss_dsi_panel_off(struct mdss_panel_data *pdata)
{
	struct mipi_panel_info *mipi;
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	struct mdss_panel_specific_pdata *spec_pdata = NULL;
	struct mdss_data_type *mdata = mdss_mdp_get_mdata();
	struct mdss_mdp_ctl *ctl = mdata->ctl_off;

	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);

	spec_pdata = ctrl_pdata->spec_pdata;
	if (!spec_pdata) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	if (!spec_pdata->detected) {
		spec_pdata->detected = true;
		if (!spec_pdata->init_from_begin)
			return 0;
	}

	pr_info("%s: ctrl=%p ndx=%d\n", __func__, ctrl_pdata, ctrl_pdata->ndx);

	if (spec_pdata->cabc_deferred_on_cmds.cmd_cnt &&
			spec_pdata->cabc_active) {
		pr_debug("%s: killing CABC deferred\n", __func__);
		cancel_work_sync(&ctrl_pdata->cabc_work);
	}

	mipi = &pdata->panel_info.mipi;

	if (ctrl_pdata->off_cmds.cmd_cnt) {
		mdss_dsi_panel_wait_change(ctrl_pdata, false);
		mdss_dsi_panel_cmds_send(ctrl_pdata, &ctrl_pdata->off_cmds);
		display_onoff_state = false;
	}

	if (spec_pdata->cabc_active && (spec_pdata->cabc_enabled == 0)) {
		pr_debug("%s: sending display off\n", __func__);
		if (ctrl_pdata->cabc_off_cmds.cmd_cnt)
			mdss_dsi_panel_cmds_send(ctrl_pdata,
				&ctrl_pdata->cabc_off_cmds);
		if (ctrl_pdata->cabc_late_off_cmds.cmd_cnt)
			mdss_dsi_panel_cmds_send(ctrl_pdata,
				&ctrl_pdata->cabc_late_off_cmds);
		spec_pdata->cabc_active = 0;
	}

	if ((spec_pdata->new_vfp) &&
		(ctrl_pdata->panel_data.panel_info.lcdc.v_front_porch !=
			spec_pdata->new_vfp))
		ctrl_pdata->panel_data.panel_info.lcdc.v_front_porch =
			spec_pdata->new_vfp;

	vs_handle.vsync_handler = (mdp_vsync_handler_t)vsync_handler;
	vs_handle.cmd_post_flush = false;
	vs_handle.enabled = true;
	if (vpsd.vps_en && (ctl->remove_vsync_handler)) {
		ctl->remove_vsync_handler(ctl, &vs_handle);
		vpsd.vps_en = false;
		fpsd.fpks = 0;
		pr_info("%s: vsyncs_per_ksecs is invalid\n", __func__);
	}

	pr_debug("%s: Done\n", __func__);

	return 0;
}

static void cabc_work_fn(struct work_struct *work)
{
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	struct mdss_panel_specific_pdata *spec_pdata = NULL;

	if (work == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return;
	}

	ctrl_pdata = container_of(work, struct mdss_dsi_ctrl_pdata,
				cabc_work);

	spec_pdata = ctrl_pdata->spec_pdata;
	if (!spec_pdata) {
		pr_err("%s: Invalid input data\n", __func__);
		return;
	}

	pr_debug("%s: CABC deferred sequence\n", __func__);
	mdss_dsi_panel_cmds_send(ctrl_pdata,
		&spec_pdata->cabc_deferred_on_cmds);
	spec_pdata->cabc_active = 1;
	pr_debug("%s: CABC deferred sequence sent\n", __func__);
}

static int mdss_dsi_panel_disp_on(struct mdss_panel_data *pdata)
{
	struct mipi_panel_info *mipi;
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	struct mdss_panel_specific_pdata *spec_pdata = NULL;

	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);

	spec_pdata = ctrl_pdata->spec_pdata;
	if (!spec_pdata) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	if (!spec_pdata->detected)
		return 0;

	mipi = &pdata->panel_info.mipi;

	if ((spec_pdata->cabc_on_cmds.cmd_cnt && spec_pdata->cabc_enabled) ||
		(ctrl_pdata->on_cmds.cmd_cnt && spec_pdata->disp_on_in_hs)) {
		pr_debug("%s: delay after entering video mode\n", __func__);
		msleep(spec_pdata->wait_time_before_on_cmd);
	}

	if (spec_pdata->cabc_on_cmds.cmd_cnt && spec_pdata->cabc_enabled) {
		pr_debug("%s: CABC on sequence\n", __func__);
		mdss_dsi_panel_cmds_send(ctrl_pdata, &spec_pdata->cabc_on_cmds);
		spec_pdata->cabc_active = 1;
	}

	if (ctrl_pdata->on_cmds.cmd_cnt && spec_pdata->disp_on_in_hs) {
		pr_debug("%s: panel on sequence (in high speed)\n", __func__);
		mdss_dsi_panel_wait_change(ctrl_pdata, true);
		mdss_set_tx_power_mode(0, pdata);
		mdss_dsi_panel_cmds_send(ctrl_pdata, &ctrl_pdata->on_cmds);
		display_onoff_state = true;
		pr_info("%s: ctrl=%p ndx=%d\n", __func__, ctrl_pdata, ctrl_pdata->ndx);
	}
	if (spec_pdata->cabc_deferred_on_cmds.cmd_cnt &&
				spec_pdata->cabc_enabled) {
		pr_debug("%s: posting CABC deferred\n", __func__);
		schedule_work(&ctrl_pdata->cabc_work);
	}
	pr_debug("%s: done\n", __func__);

	return 0;
}

static int mdss_dsi_panel_power_on_ex(struct mdss_panel_data *pdata, int enable)
{
	int ret = -EINVAL;
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	struct mdss_panel_specific_pdata *spec_pdata = NULL;
	static int display_power_on;
	static int skip_first_off = 1;
	static u32 down;

	pr_debug("%s: enable=%d\n", __func__, enable);
	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		goto error;
	}

	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);

	spec_pdata = ctrl_pdata->spec_pdata;
	if (!spec_pdata) {
		pr_err("%s: Invalid input data\n", __func__);
		goto error;
	}

	ret = 0;
	if (enable && !display_power_on) {
		if (spec_pdata->down_period) {
			u32 kt = (u32)ktime_to_ms(ktime_get_boottime());
			kt = (kt < down) ? kt + ~down : kt - down;
			if (kt < spec_pdata->down_period)
				usleep_range((spec_pdata->down_period - kt) *
					1000,
					(spec_pdata->down_period - kt) *
					1000 + 100);
		}
		ret = msm_dss_enable_vreg(
			ctrl_pdata->power_data.vreg_config,
			ctrl_pdata->power_data.num_vreg, 1);
		if (ret) {
			pr_err("%s: Failed to enable vregs.rc=%d\n",
				__func__, ret);
			goto error;
		}

		if (gpio_is_valid(vsn_gpio) &&
			gpio_is_valid(vsp_gpio)) {
			usleep_range(19000, 20000);

			gpio_set_value(vsn_gpio, 1);
			gpio_set_value(vsp_gpio, 1);
			ret = mdss_dsi_panel_reset_seq(pdata, 1);
			if (ret)
				pr_err("%s: Failed to enable gpio.\n",
							__func__);
		}

		display_power_on = 1;
	} else if (!enable && skip_first_off && spec_pdata->panel_detect) {
		skip_first_off = 0;
	} else if (!enable) {
		if (gpio_is_valid(vsn_gpio) &&
			gpio_is_valid(vsp_gpio)) {
			gpio_set_value(vsn_gpio, 0);
			gpio_set_value(vsp_gpio, 0);
		}

		ret = mdss_dsi_panel_reset_seq(pdata, 0);
		if (ret) {
			pr_err("%s: Failed to disable gpio.\n", __func__);
			goto error;
		}

		ret = msm_dss_enable_vreg(
			ctrl_pdata->power_data.vreg_config,
			ctrl_pdata->power_data.num_vreg, 0);
		if (ret) {
			pr_err("%s: Failed to disable vregs.rc=%d\n",
				__func__, ret);
		}
		if (spec_pdata->down_period)
			down = (u32)ktime_to_ms(ktime_get_boottime());
		display_power_on = 0;
	}
error:
	return ret;
}

static int mdss_dsi_panel_detect(struct mdss_panel_data *pdata)
{
	struct device_node *np;
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	struct mdss_panel_specific_pdata *spec_pdata = NULL;
	int rc;

	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);

	spec_pdata = ctrl_pdata->spec_pdata;
	if (!spec_pdata) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	if (!spec_pdata->panel_detect)
		return 0;

		pr_debug("%s: Panel ID", __func__);
	if (!alt_panelid_cmd) {
		mdss_dsi_op_mode_config(DSI_CMD_MODE, pdata);
		mdss_dsi_cmds_rx(ctrl_pdata,
				 spec_pdata->id_read_cmds.cmds, 10);

		pr_debug("0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X",
			ctrl_pdata->rx_buf.data[0], ctrl_pdata->rx_buf.data[1],
			ctrl_pdata->rx_buf.data[2], ctrl_pdata->rx_buf.data[3],
			ctrl_pdata->rx_buf.data[4], ctrl_pdata->rx_buf.data[5],
			ctrl_pdata->rx_buf.data[6], ctrl_pdata->rx_buf.data[7]);
	} else {
		mdss_dsi_panel_cmd_read(ctrl_pdata, dcs_cmd_DA[0], dcs_cmd_DA[1],
					dcs_read_cmd_DA, panel_id_store, 1);
		pr_debug("0x%02X \n", panel_id[0]);
	}

	np = of_parse_phandle(
			pdata->panel_pdev->dev.of_node,
			"qcom,dsi-pref-prim-pan", 0);

	if (!alt_panelid_cmd)
		rc = mdss_panel_parse_dt(np, ctrl_pdata,
			spec_pdata->driver_ic, ctrl_pdata->rx_buf.data);
	else
		rc = mdss_panel_parse_dt(np, ctrl_pdata,
			spec_pdata->driver_ic, (char*)panel_id);

	return 0;
}

static int mdss_dsi_panel_update_panel_info(struct mdss_panel_data *pdata)
{
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	struct mdss_panel_specific_pdata *spec_pdata = NULL;
	struct mdss_panel_info *pinfo;
	struct mipi_panel_info *mipi;
	int rc;

	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);

	spec_pdata = ctrl_pdata->spec_pdata;
	if (!spec_pdata) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	if (!spec_pdata->panel_detect)
		return 0;

	pinfo = &(ctrl_pdata->panel_data.panel_info);

	mipi  = &(pinfo->mipi);

	pinfo->type =
		((mipi->mode == DSI_VIDEO_MODE)
			? MIPI_VIDEO_PANEL : MIPI_CMD_PANEL);

	pll_divider_config.clk_rate = pinfo->clk_rate;

	rc = mdss_dsi_clk_div_config(pinfo, mipi->frame_rate);
	if (rc) {
		pr_err("%s: unable to initialize the clk dividers\n", __func__);
		return rc;
	}

	ctrl_pdata->pclk_rate = mipi->dsi_pclk_rate;
	ctrl_pdata->byte_clk_rate = pinfo->clk_rate / 8;

	ctrl_pdata->off_cmds = spec_pdata->off_cmds[DETECTED_CMDS];
	ctrl_pdata->cabc_off_cmds = spec_pdata->cabc_off_cmds[DETECTED_CMDS];
	ctrl_pdata->cabc_late_off_cmds =
				spec_pdata->cabc_late_off_cmds[DETECTED_CMDS];

	if (spec_pdata->cabc_deferred_on_cmds.cmd_cnt)
		INIT_WORK(&ctrl_pdata->cabc_work, cabc_work_fn);

	return 0;
}

static u32 ts_diff_ms(struct timespec lhs, struct timespec rhs)
{
	struct timespec tdiff;
	s64 nsec;
	u32 msec;

	tdiff = timespec_sub(lhs, rhs);
	nsec = timespec_to_ns(&tdiff);
	msec = (u32)nsec;
	do_div(msec, NSEC_PER_MSEC);

	return msec;
}

static void update_fps_data(struct fps_data *fps)
{
	if (mutex_trylock(&fps->fps_lock)) {
		u32 fpks = 0;
		u32 ms_since_last = 0;
		u32 num_frames;
		struct timespec tlast = fps->timestamp_last;
		struct timespec tnow;
		u32 msec;

		getrawmonotonic(&tnow);
		msec = ts_diff_ms(tnow, tlast);
		fps->timestamp_last = tnow;

		fps->interval_ms = msec;
		fps->frame_counter++;
		num_frames = fps->frame_counter - fps->frame_counter_last;

		fps->fa[fps->fps_array_cnt].frame_nbr = fps->frame_counter;
		fps->fa[fps->fps_array_cnt].time_delta = msec;
		fps->fa_last_array_pos = fps->fps_array_cnt;
		(fps->fps_array_cnt)++;
		if (fps->fps_array_cnt >= DEFAULT_FPS_ARRAY_SIZE)
			fps->fps_array_cnt = 0;

		ms_since_last = ts_diff_ms(tnow, fps->fpks_ts_last);

		if (num_frames > 1 && ms_since_last >= fps->log_interval) {
			fpks = (num_frames * 1000000) / ms_since_last;
			fps->fpks_ts_last = tnow;
			fps->frame_counter_last = fps->frame_counter;
			fps->fpks = fpks;
		}
		mutex_unlock(&fps->fps_lock);
	}
}

static void mdss_dsi_panel_fps_data_init(struct fps_data *fps)
{
	fps->frame_counter = 0;
	fps->frame_counter_last = 0;
	fps->log_interval = DEFAULT_FPS_LOG_INTERVAL;
	fps->fpks = 0;
	fps->fa_last_array_pos = 0;
	fps->vps_en = false;
	getrawmonotonic(&fps->timestamp_last);
	mutex_init(&fps->fps_lock);
}

int mdss_dsi_panel_fps_data_update(struct msm_fb_data_type *mfd)
{
	/* Only count fps on primary display */
	if (mfd->index == 0)
		update_fps_data(&fpsd);

	return 0;
}

static void mdss_dsi_panel_vps_data_update(struct msm_fb_data_type *mfd)
{
	/* Only count vpks(hw vsyncs per ksecs) on primary display */
	if (mfd->index == 0)
		update_fps_data(&vpsd);
}

static void vsync_handler(struct mdss_mdp_ctl *ctl, ktime_t t)
{
	struct msm_fb_data_type *mfd = ctl->mfd;

	mdss_dsi_panel_vps_data_update(mfd);
}

static void conv_uv_data(char *data, int param_type, int *u_data, int *v_data)
{
	switch (param_type) {
	case CLR_DATA_UV_PARAM_TYPE_RENE_DEFAULT:
		*u_data = ((data[0] & 0x0F) << 2) |
			/* 4bit of data[0] higher data. */
			((data[1] >> 6) & 0x03);
			/* 2bit of data[1] lower data. */
		*v_data = (data[1] & 0x3F);
			/* Remainder 6bit of data[1] is effective as v_data. */
		break;
	case CLR_DATA_UV_PARAM_TYPE_NOVA_DEFAULT:
	case CLR_DATA_UV_PARAM_TYPE_RENE_SR:
		/* 6bit is effective as u_data */
		*u_data = data[0] & 0x3F;
		/* 6bit is effective as v_data */
		*v_data = data[1] & 0x3F;
		break;
	case CLR_DATA_UV_PARAM_TYPE_NOVA_AUO:
		/* 6bit is effective as u_data */
		*u_data = data[0] & 0x3F;
		/* 6bit is effective as v_data */
		*v_data = data[2] & 0x3F;
		break;
	default:
		pr_err("%s: Failed to conv type:%d\n", __func__, param_type);
		break;
	}
}

static int get_uv_param_len(int param_type, bool *short_response)
{
	int ret = 0;

	*short_response = false;
	switch (param_type) {
	case CLR_DATA_UV_PARAM_TYPE_RENE_DEFAULT:
		ret = CLR_DATA_REG_LEN_RENE_DEFAULT;
		break;
	case CLR_DATA_UV_PARAM_TYPE_NOVA_DEFAULT:
		ret = CLR_DATA_REG_LEN_NOVA_DEFAULT;
		break;
	case CLR_DATA_UV_PARAM_TYPE_NOVA_AUO:
		ret = CLR_DATA_REG_LEN_NOVA_AUO;
		break;
	case CLR_DATA_UV_PARAM_TYPE_RENE_SR:
		ret = CLR_DATA_REG_LEN_RENE_SR;
		*short_response = true;
		break;
	default:
		pr_err("%s: Failed to get param len\n", __func__);
		break;
	}

	return ret;
}

static void get_uv_data(struct mdss_dsi_ctrl_pdata *ctrl_pdata,
		int *u_data, int *v_data)
{
	struct dsi_cmd_desc *cmds = ctrl_pdata->spec_pdata->uv_read_cmds.cmds;
	int param_type = ctrl_pdata->spec_pdata->pcc_data.param_type;
	char buf[MDSS_DSI_LEN];
	char *pos = buf;
	int len;
	int i;
	bool short_response;

	len = get_uv_param_len(param_type, &short_response);

	mdss_dsi_cmd_mdp_busy(ctrl_pdata);
	mdss_bus_bandwidth_ctrl(1);
	mdss_dsi_clk_ctrl(ctrl_pdata, 1);
	for (i = 0; i < ctrl_pdata->spec_pdata->uv_read_cmds.cmd_cnt; i++) {
		if (short_response)
			mdss_dsi_cmds_rx(ctrl_pdata, cmds, 0);
		else
			mdss_dsi_cmds_rx(ctrl_pdata, cmds, len);
		memcpy(pos, ctrl_pdata->rx_buf.data, len);
		pos += len;
		cmds++;
	}
	mdss_dsi_clk_ctrl(ctrl_pdata, 0);
	mdss_bus_bandwidth_ctrl(0);
	conv_uv_data(buf, param_type, u_data, v_data);
}

static int find_color_area(struct mdp_pcc_cfg_data *pcc_config,
	struct mdss_pcc_data *pcc_data)
{
	int i;
	int ret = 0;

	for (i = 0; i < pcc_data->tbl_size; i++) {
		if (pcc_data->u_data < pcc_data->color_tbl[i].u_min)
			continue;
		if (pcc_data->u_data > pcc_data->color_tbl[i].u_max)
			continue;
		if (pcc_data->v_data < pcc_data->color_tbl[i].v_min)
			continue;
		if (pcc_data->v_data > pcc_data->color_tbl[i].v_max)
			continue;
		break;
	}
	pcc_data->tbl_idx = i;
	if (i >= pcc_data->tbl_size) {
		ret = -EINVAL;
		goto exit;
	}

	pcc_config->r.r = pcc_data->color_tbl[i].r_data;
	pcc_config->g.g = pcc_data->color_tbl[i].g_data;
	pcc_config->b.b = pcc_data->color_tbl[i].b_data;
exit:
	return ret;
}

static int mdss_dsi_panel_pcc_setup(struct mdss_panel_data *pdata)
{
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	struct mdss_pcc_data *pcc_data = NULL;
	struct mdss_panel_info *pinfo = NULL;
	int ret;
	u32 copyback;
	struct mdp_pcc_cfg_data pcc_config;

	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);

	pcc_data = &ctrl_pdata->spec_pdata->pcc_data;
	if (!pcc_data->color_tbl)
		goto exit;

	mdss_dsi_op_mode_config(DSI_CMD_MODE, pdata);
	if (ctrl_pdata->spec_pdata->pre_uv_read_cmds.cmds)
		mdss_dsi_panel_cmds_send(
			ctrl_pdata, &ctrl_pdata->spec_pdata->pre_uv_read_cmds);
	if (ctrl_pdata->spec_pdata->uv_read_cmds.cmds)
		get_uv_data(ctrl_pdata, &pcc_data->u_data, &pcc_data->v_data);
	if (pcc_data->param_type == CLR_DATA_UV_PARAM_TYPE_NONE) {
		pcc_data->u_data = CENTER_U_DATA;
		pcc_data->v_data = CENTER_V_DATA;
	}
	if (pcc_data->u_data == 0 && pcc_data->v_data == 0)
		goto exit;

	memset(&pcc_config, 0, sizeof(struct mdp_pcc_cfg_data));

	pinfo = &ctrl_pdata->panel_data.panel_info;
	if (pinfo->rev_u[1] != 0) {
		if (pinfo->rev_u[0] == 0)
			pcc_data->u_data = pcc_data->u_data + pinfo->rev_u[1];
		else if (pcc_data->u_data < pinfo->rev_u[1])
			pcc_data->u_data = 0;
		else
			pcc_data->u_data = pcc_data->u_data - pinfo->rev_u[1];
	}
	if (pinfo->rev_v[1] != 0) {
		if (pinfo->rev_v[0] == 0)
			pcc_data->v_data = pcc_data->v_data + pinfo->rev_v[1];
		else if (pcc_data->v_data < pinfo->rev_v[1])
			pcc_data->v_data = 0;
		else
			pcc_data->v_data = pcc_data->v_data - pinfo->rev_v[1];
	}

	ret = find_color_area(&pcc_config, pcc_data);
	if (ret)
		goto exit;

	if (pcc_data->color_tbl[pcc_data->tbl_idx].color_type != UNUSED) {
		pcc_config.block = MDP_LOGICAL_BLOCK_DISP_0;
		pcc_config.ops = MDP_PP_OPS_ENABLE | MDP_PP_OPS_WRITE;
		ret = mdss_mdp_pcc_config(&pcc_config, &copyback);
		if (ret != 0)
			pr_err("failed by settings of pcc data.\n");
	}

	if (pinfo->rev_u[1] != 0 && pinfo->rev_v[1] != 0)
		pr_info("%s (%d):(ru[0], ru[1])=(%d, %d), (rv[0], rv[1])=(%d, %d)",
			__func__, __LINE__,
			pinfo->rev_u[0], pinfo->rev_u[1],
			pinfo->rev_v[0], pinfo->rev_v[1]);

	pr_info("%s (%d):ct=%d area=%d ud=%d vd=%d r=0x%08X g=0x%08X b=0x%08X",
		__func__, __LINE__,
		pcc_data->color_tbl[pcc_data->tbl_idx].color_type,
		pcc_data->color_tbl[pcc_data->tbl_idx].area_num,
		pcc_data->u_data, pcc_data->v_data,
		pcc_data->color_tbl[pcc_data->tbl_idx].r_data,
		pcc_data->color_tbl[pcc_data->tbl_idx].g_data,
		pcc_data->color_tbl[pcc_data->tbl_idx].b_data);

exit:
	return 0;
}

static void mdss_dsi_parse_lane_swap(struct device_node *np, char *dlane_swap)
{
	const char *data;

	*dlane_swap = DSI_LANE_MAP_0123;
	data = of_get_property(np, "qcom,mdss-dsi-lane-map", NULL);
	if (data) {
		if (!strcmp(data, "lane_map_3012"))
			*dlane_swap = DSI_LANE_MAP_3012;
		else if (!strcmp(data, "lane_map_2301"))
			*dlane_swap = DSI_LANE_MAP_2301;
		else if (!strcmp(data, "lane_map_1230"))
			*dlane_swap = DSI_LANE_MAP_1230;
		else if (!strcmp(data, "lane_map_0321"))
			*dlane_swap = DSI_LANE_MAP_0321;
		else if (!strcmp(data, "lane_map_1032"))
			*dlane_swap = DSI_LANE_MAP_1032;
		else if (!strcmp(data, "lane_map_2103"))
			*dlane_swap = DSI_LANE_MAP_2103;
		else if (!strcmp(data, "lane_map_3210"))
			*dlane_swap = DSI_LANE_MAP_3210;
	}
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

	pcmds->cmd_cnt = 0;

	data = of_get_property(np, cmd_key, &blen);
	if (!data) {
		pr_err("%s: failed, key=%s\n", __func__, cmd_key);
		return 0;
	}

	if (pcmds->buf != NULL)
		kfree(pcmds->buf);

	if (pcmds->cmds != NULL)
		kfree(pcmds->cmds);

	buf = kzalloc(sizeof(char) * blen, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	memcpy(buf, data, blen);

	/* scan dcs commands */
	bp = buf;
	len = blen;
	cnt = 0;
	while (len > sizeof(*dchdr)) {
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
	if (!pcmds->cmds) {
		goto exit_free;
	}

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

	if (link_key != NULL) {
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

static int mdss_dsi_property_read_u32_var(struct device_node *np,
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

static int mdss_panel_dt_get_dst_fmt(u32 bpp, char mipi_mode, u32 pixel_packing,
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
				struct mdss_panel_info *panel_info)
{
	int rc, fbc_enabled = 0;
	u32 tmp;

	fbc_enabled = of_property_read_bool(np,	"qcom,mdss-dsi-fbc-enable");
	if (fbc_enabled) {
		pr_debug("%s:%d FBC panel enabled.\n", __func__, __LINE__);
		panel_info->fbc.enabled = 1;
		rc = of_property_read_u32(np, "qcom,mdss-dsi-fbc-bpp", &tmp);
		panel_info->fbc.target_bpp =	(!rc ? tmp : panel_info->bpp);
		rc = of_property_read_u32(np, "qcom,mdss-dsi-fbc-packing",
				&tmp);
		panel_info->fbc.comp_mode = (!rc ? tmp : 0);
		panel_info->fbc.qerr_enable = of_property_read_bool(np,
			"qcom,mdss-dsi-fbc-quant-error");
		rc = of_property_read_u32(np, "qcom,mdss-dsi-fbc-bias", &tmp);
		panel_info->fbc.cd_bias = (!rc ? tmp : 0);
		panel_info->fbc.pat_enable = of_property_read_bool(np,
				"qcom,mdss-dsi-fbc-pat-mode");
		panel_info->fbc.vlc_enable = of_property_read_bool(np,
				"qcom,mdss-dsi-fbc-vlc-mode");
		panel_info->fbc.bflc_enable = of_property_read_bool(np,
				"qcom,mdss-dsi-fbc-bflc-mode");
		rc = of_property_read_u32(np, "qcom,mdss-dsi-fbc-h-line-budget",
				&tmp);
		panel_info->fbc.line_x_budget = (!rc ? tmp : 0);
		rc = of_property_read_u32(np, "qcom,mdss-dsi-fbc-budget-ctrl",
				&tmp);
		panel_info->fbc.block_x_budget = (!rc ? tmp : 0);
		rc = of_property_read_u32(np, "qcom,mdss-dsi-fbc-block-budget",
				&tmp);
		panel_info->fbc.block_budget = (!rc ? tmp : 0);
		rc = of_property_read_u32(np,
				"qcom,mdss-dsi-fbc-lossless-threshold", &tmp);
		panel_info->fbc.lossless_mode_thd = (!rc ? tmp : 0);
		rc = of_property_read_u32(np,
				"qcom,mdss-dsi-fbc-lossy-threshold", &tmp);
		panel_info->fbc.lossy_mode_thd = (!rc ? tmp : 0);
		rc = of_property_read_u32(np, "qcom,mdss-dsi-fbc-rgb-threshold",
				&tmp);
		panel_info->fbc.lossy_rgb_thd = (!rc ? tmp : 0);
		rc = of_property_read_u32(np,
				"qcom,mdss-dsi-fbc-lossy-mode-idx", &tmp);
		panel_info->fbc.lossy_mode_idx = (!rc ? tmp : 0);
	} else {
		pr_debug("%s:%d Panel does not support FBC.\n",
				__func__, __LINE__);
		panel_info->fbc.enabled = 0;
		panel_info->fbc.target_bpp =
			panel_info->bpp;
	}
	return 0;
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

#define PANEL_FULL_HD_DEFAULT_XRES	1080
#define PANEL_FULL_HD_DEFAULT_YRES	1920

bool mdss_dsi_panel_flip_ud(void)
{
	return mdss_panel_flip_ud;
}

static int mdss_panel_parse_dt(struct device_node *np,
				struct mdss_dsi_ctrl_pdata *ctrl_pdata,
				int driver_ic, char *id_data)
{
	u32 res[2], tmp;
	int rc, i, len;
	const char *data;
	static const char *pdest;
	struct device_node *parent;
	struct device_node *next;
	static const char *panel_name;
	struct mdss_panel_info *pinfo = &(ctrl_pdata->panel_data.panel_info);
	struct mdss_panel_specific_pdata *spec_pdata = NULL;

	spec_pdata = ctrl_pdata->spec_pdata;
	if (!spec_pdata) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	parent = of_get_parent(np);
	for_each_child_of_node(parent, next) {
		rc = of_property_read_u32(next, "somc,driver-ic", &tmp);
		if (rc)
			continue;
		if (!adc_det) {
			if (driver_ic != tmp)
				continue;

			data = of_get_property(next, "somc,panel-id", &len);
			if (!data) {
				pr_err("%s:%d, panel not read\n",
					__func__, __LINE__);
				goto error;
			}
			if (data && id_data) {
				if ((len == 1) && (data[0] == PANEL_SKIP_ID))
					continue;
				for (i = 0; i < len; i++) {
					pr_debug("read data:0x%02X dtsi data:0x%02X",
						id_data[i], data[i]);
					if ((id_data[i] != data[i]) &&
						(data[i] != PANEL_SKIP_ID))
					{
						rc = -ENODEV;
						break;
					}

					rc = 0;
				}
				if (rc)
					continue;
			} else if (data && !id_data) {
				if (((len != 1) || (data[0] != PANEL_SKIP_ID)) 
								&& !alt_panelid_cmd)
					continue;
				else {
					rc = of_property_read_u32(next
						, "somc,panel-detect", &tmp);
					spec_pdata->panel_detect =
							!rc ? tmp : 0;
				}
			}
		} else {
			rc = of_property_read_u32(next, "somc,lcd-id", res);
			if (rc || *res != spec_pdata->lcd_id)
				continue;
			rc = of_property_read_u32_array(next,
					"somc,lcd-id-adc", res, ADC_PNUM);
			if (rc || spec_pdata->adc_uv < res[ADC_RNG_MIN] ||
					res[ADC_RNG_MAX] < spec_pdata->adc_uv)
				continue;
			spec_pdata->driver_ic = tmp;
		}

		panel_name = of_get_property(next,
			"qcom,mdss-dsi-panel-name", NULL);
		if (!panel_name) {
			pr_info("%s:%d, panel name not specified\n",
							__func__, __LINE__);
		} else {
			pinfo->panel_id_name = panel_name;
			pr_info("%s: Panel Name = %s\n", __func__, panel_name);
		}

		tmp = (int) of_property_read_bool(next,
			"somc,first-boot-aware");
		if (tmp)
			lcm_first_boot = tmp;

		mdss_panel_flip_ud = of_property_read_bool(next,
					"qcom,mdss-pan-flip-ud");
		if (mdss_panel_flip_ud)
			pr_info("%s: Detected upside down panel\n", __func__);

		rc = of_property_read_u32(next,
			"qcom,mdss-dsi-panel-width", &tmp);
		if (rc) {
			pr_err("%s:%d, panel width not specified\n",
							__func__, __LINE__);
			return -EINVAL;
		}
		pinfo->xres = tmp;

		rc = of_property_read_u32(next,
			"qcom,mdss-dsi-panel-height", &tmp);
		if (rc) {
			pr_err("%s:%d, panel height not specified\n",
							__func__, __LINE__);
			return -EINVAL;
		}
		pinfo->yres = tmp;

		rc = of_property_read_u32(next,
			"qcom,mdss-pan-physical-width-dimension", &tmp);
		pinfo->physical_width = !rc ? tmp : 0;
		rc = of_property_read_u32(next,
			"qcom,mdss-pan-physical-height-dimension", &tmp);
		pinfo->physical_height = !rc ? tmp : 0;
		rc = of_property_read_u32(next,
			"qcom,mdss-dsi-h-left-border", &tmp);
		pinfo->lcdc.xres_pad = !rc ? tmp : 0;
		rc = of_property_read_u32(next,
			"qcom,mdss-dsi-h-right-border", &tmp);
		if (!rc)
			pinfo->lcdc.xres_pad += tmp;
		rc = of_property_read_u32(next,
			"qcom,mdss-dsi-v-top-border", &tmp);
		pinfo->lcdc.yres_pad = !rc ? tmp : 0;
		rc = of_property_read_u32(next,
			"qcom,mdss-dsi-v-bottom-border", &tmp);
		if (!rc)
			pinfo->lcdc.yres_pad += tmp;
		rc = of_property_read_u32(next, "qcom,mdss-dsi-bpp", &tmp);
		if (rc) {
			pr_err("%s:%d, bpp not specified\n",
				__func__, __LINE__);
			return -EINVAL;
		}
		pinfo->bpp = tmp;
		pinfo->mipi.mode = DSI_VIDEO_MODE;
		data = of_get_property(next, "qcom,mdss-dsi-panel-type", NULL);
		if (data && !strncmp(data, "dsi_cmd_mode", 12))
			pinfo->mipi.mode = DSI_CMD_MODE;
		tmp = 0;
		data = of_get_property(next,
			"qcom,mdss-dsi-pixel-packing", NULL);
		if (data && !strcmp(data, "loose"))
			tmp = 1;
		rc = mdss_panel_dt_get_dst_fmt(pinfo->bpp,
			pinfo->mipi.mode, tmp,
			&pinfo->mipi.dst_format);
		if (rc) {
			pr_debug("%s: problem dst format. Set Default\n",
				__func__);
			pinfo->mipi.dst_format =
				DSI_VIDEO_DST_FORMAT_RGB888;
		}
		pdest = of_get_property(next,
			"qcom,mdss-dsi-panel-destination", NULL);

		if (pdest) {
			if (strlen(pdest) != 9) {
				pr_err("%s: Unknown pdest specified\n",
					__func__);
				return -EINVAL;
			}
			if (!strcmp(pdest, "display_1"))
				pinfo->pdest = DISPLAY_1;
			else if (!strcmp(pdest, "display_2"))
				pinfo->pdest = DISPLAY_2;
			else {
				pr_debug("%s: pdest not specified. Set Default\n",
					__func__);
				pinfo->pdest = DISPLAY_1;
			}
		} else {
			pr_err("%s: pdest not specified\n", __func__);
			return -EINVAL;
		}
		rc = of_property_read_u32(next,
			"qcom,mdss-dsi-h-front-porch", &tmp);
		pinfo->lcdc.h_front_porch = !rc ? tmp : 6;
		rc = of_property_read_u32(next,
			"qcom,mdss-dsi-h-back-porch", &tmp);
		pinfo->lcdc.h_back_porch = !rc ? tmp : 6;
		rc = of_property_read_u32(next,
			"qcom,mdss-dsi-h-pulse-width", &tmp);
		pinfo->lcdc.h_pulse_width = !rc ? tmp : 2;
		rc = of_property_read_u32(next,
			"qcom,mdss-dsi-h-sync-skew", &tmp);
		pinfo->lcdc.hsync_skew = !rc ? tmp : 0;
		rc = of_property_read_u32(next,
			"qcom,mdss-dsi-v-back-porch", &tmp);
		pinfo->lcdc.v_back_porch = !rc ? tmp : 6;
		rc = of_property_read_u32(next,
			"qcom,mdss-dsi-v-front-porch", &tmp);
		pinfo->lcdc.v_front_porch = !rc ? tmp : 6;
		rc = of_property_read_u32(next,
			"qcom,mdss-dsi-v-pulse-width", &tmp);
		pinfo->lcdc.v_pulse_width = !rc ? tmp : 2;
		rc = of_property_read_u32(next,
			"qcom,mdss-dsi-underflow-color", &tmp);
		pinfo->lcdc.underflow_clr = !rc ? tmp : 0xff;
		rc = of_property_read_u32(next,
			"qcom,mdss-dsi-border-color", &tmp);
		pinfo->lcdc.border_clr = !rc ? tmp : 0;
		ctrl_pdata->bklt_ctrl = UNKNOWN_CTRL;
		data = of_get_property(next,
			"qcom,mdss-dsi-bl-pmic-control-type", NULL);
		if (data) {
			if (!strncmp(data, "bl_ctrl_wled", 12)) {
				led_trigger_register_simple("bkl-trigger",
					&bl_led_trigger);
				pr_debug("%s: SUCCESS WLED TRIGGER register\n",
					__func__);
				ctrl_pdata->bklt_ctrl = BL_WLED;
			} else if (!strncmp(data, "bl_ctrl_pwm", 11)) {
				ctrl_pdata->bklt_ctrl = BL_PWM;
				rc = of_property_read_u32(next,
					"qcom,mdss-dsi-bl-pmic-pwm-frequency",
					&tmp);
				if (rc) {
					pr_err("%s:%d, Error, pwm_period\n",
							__func__, __LINE__);
					return -EINVAL;
				}
				ctrl_pdata->pwm_period = tmp;
				rc = of_property_read_u32(next,
					"qcom,mdss-dsi-bl-pmic-bank-select",
					&tmp);
				if (rc) {
					pr_err("%s:%d, Error, lpg channel\n",
							__func__, __LINE__);
					return -EINVAL;
				}
				ctrl_pdata->pwm_lpg_chan = tmp;
				tmp = of_get_named_gpio(next,
					"qcom,mdss-dsi-pwm-gpio", 0);
				ctrl_pdata->pwm_pmic_gpio = tmp;
			} else if (!strncmp(data, "bl_ctrl_dcs", 11)) {
				ctrl_pdata->bklt_ctrl = BL_DCS_CMD;
			}
		}
		rc = of_property_read_u32(next,
			"qcom,mdss-brightness-max-level", &tmp);
		pinfo->brightness_max = (!rc ? tmp : MDSS_MAX_BL_BRIGHTNESS);
		rc = of_property_read_u32(next,
			"qcom,mdss-dsi-bl-min-level", &tmp);
		pinfo->bl_min = !rc ? tmp : 0;
		rc = of_property_read_u32(next,
			"qcom,mdss-dsi-bl-max-level", &tmp);
		pinfo->bl_max = !rc ? tmp : 255;

		rc = of_property_read_u32(next,
			"qcom,mdss-dsi-interleave-mode", &tmp);
		pinfo->mipi.interleave_mode = !rc ? tmp : 0;

		pinfo->mipi.vsync_enable = of_property_read_bool(next,
			"qcom,mdss-dsi-te-check-enable");
		pinfo->mipi.hw_vsync_mode = of_property_read_bool(next,
			"qcom,mdss-dsi-te-using-te-pin");

		rc = of_property_read_u32(next,
			"qcom,mdss-dsi-h-sync-pulse", &tmp);
		pinfo->mipi.pulse_mode_hsa_he = !rc ? tmp : false;

		pinfo->mipi.hfp_power_stop = of_property_read_bool(next,
			"qcom,mdss-dsi-hfp-power-mode");
		pinfo->mipi.hsa_power_stop = of_property_read_bool(next,
			"qcom,mdss-dsi-hsa-power-mode");
		pinfo->mipi.hbp_power_stop = of_property_read_bool(next,
			"qcom,mdss-dsi-hbp-power-mode");
		pinfo->mipi.bllp_power_stop = of_property_read_bool(next,
			"qcom,mdss-dsi-bllp-power-mode");
		pinfo->mipi.eof_bllp_power_stop =
			of_property_read_bool(
				next, "qcom,mdss-dsi-bllp-eof-power-mode");
		pinfo->mipi.traffic_mode = DSI_NON_BURST_SYNCH_PULSE;
		data = of_get_property(next,
			"qcom,mdss-dsi-traffic-mode", NULL);
		if (data) {
			if (!strcmp(data, "non_burst_sync_event"))
				pinfo->mipi.traffic_mode
					 = DSI_NON_BURST_SYNCH_EVENT;
			else if (!strcmp(data, "burst_mode"))
				pinfo->mipi.traffic_mode = DSI_BURST_MODE;
		}
		rc = of_property_read_u32(next,
			"qcom,mdss-dsi-te-dcs-command", &tmp);
		pinfo->mipi.insert_dcs_cmd = !rc ? tmp : 1;
		rc = of_property_read_u32(next,
			"qcom,mdss-dsi-wr-mem-continue", &tmp);
		pinfo->mipi.wr_mem_continue = !rc ? tmp : 0x3c;
		rc = of_property_read_u32(next,
			"qcom,mdss-dsi-wr-mem-start", &tmp);
		pinfo->mipi.wr_mem_start = !rc ? tmp : 0x2c;
		rc = of_property_read_u32(next,
			"qcom,mdss-dsi-te-pin-select", &tmp);
		pinfo->mipi.te_sel = !rc ? tmp : 1;
		rc = of_property_read_u32(next,
			"qcom,mdss-dsi-virtual-channel-id", &tmp);
		pinfo->mipi.vc = !rc ? tmp : 0;
		data = of_get_property(next,
			"qcom,mdss-dsi-color-order", NULL);
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
		pinfo->mipi.rgb_swap = !rc ? tmp : DSI_RGB_SWAP_RGB;
		pinfo->mipi.data_lane0 = of_property_read_bool(next,
			"qcom,mdss-dsi-lane-0-state");
		pinfo->mipi.data_lane1 = of_property_read_bool(next,
			"qcom,mdss-dsi-lane-1-state");
		pinfo->mipi.data_lane2 = of_property_read_bool(next,
			"qcom,mdss-dsi-lane-2-state");
		pinfo->mipi.data_lane3 = of_property_read_bool(next,
			"qcom,mdss-dsi-lane-3-state");

		rc = of_property_read_u32(next,
			"qcom,mdss-dsi-t-clk-pre", &tmp);
		pinfo->mipi.t_clk_pre = !rc ? tmp : 0x24;
		rc = of_property_read_u32(next,
			"qcom,mdss-dsi-t-clk-post", &tmp);
		pinfo->mipi.t_clk_post = !rc ? tmp : 0x03;

		pinfo->mipi.rx_eot_ignore = of_property_read_bool(next,
			"qcom,mdss-dsi-rx-eot-ignore");
		pinfo->mipi.tx_eot_append = of_property_read_bool(next,
			"qcom,mdss-dsi-tx-eot-append");

		rc = of_property_read_u32(next, "qcom,mdss-dsi-stream", &tmp);
		pinfo->mipi.stream = !rc ? tmp : 0;

		data = of_get_property(next,
			"qcom,mdss-dsi-panel-mode-gpio-state", NULL);
		if (data) {
			if (!strcmp(data, "high"))
				pinfo->mode_gpio_state = MODE_GPIO_HIGH;
			else if (!strcmp(data, "low"))
				pinfo->mode_gpio_state = MODE_GPIO_LOW;
		} else {
			pinfo->mode_gpio_state = MODE_GPIO_NOT_VALID;
		}

		rc = of_property_read_u32(next,
			"qcom,mdss-dsi-panel-framerate", &tmp);
		pinfo->mipi.frame_rate = !rc ? tmp : 60;
		pinfo->mipi.input_fpks = pinfo->mipi.frame_rate * 1000;
		rc = of_property_read_u32(next,
			"qcom,mdss-dsi-panel-clockrate", &tmp);
		pinfo->clk_rate = !rc ? tmp : 0;
		data = of_get_property(next,
			"qcom,mdss-dsi-panel-timings", &len);
		if (!data || len != 12) {
			pr_err("%s:%d, Unable to read Phy timing settings",
			       __func__, __LINE__);
			goto error;
		}
		for (i = 0; i < len; i++)
			pinfo->mipi.dsi_phy_db.timing[i] = data[i];

		pinfo->mipi.lp11_init = of_property_read_bool(next,
						"qcom,mdss-dsi-lp11-init");
		rc = of_property_read_u32(next, "qcom,mdss-dsi-init-delay-us",
				&tmp);
		pinfo->mipi.init_delay = (!rc ? tmp : 0);

		data = of_get_property(next,
			"somc,mdss-dsi-lane-config", &len);
		if ((data) && (len == 45)) {
			for (i = 0; i < len; i++) {
				pinfo->mipi.dsi_phy_db.lanecfg[i] =
						data[i];
			}
		} else
			pr_err("%s:%d, Unable to read Phy lane configure",
				__func__, __LINE__);

		mdss_dsi_parse_fbc_params(next, pinfo);

		mdss_dsi_parse_trigger(next, &(pinfo->mipi.mdp_trigger),
			"qcom,mdss-dsi-mdp-trigger");

		mdss_dsi_parse_trigger(next, &(pinfo->mipi.dma_trigger),
			"qcom,mdss-dsi-dma-trigger");

		mdss_dsi_parse_lane_swap(next, &(pinfo->mipi.dlane_swap));

		mdss_dsi_parse_reset_seq(next, pinfo->rst_seq,
			&(pinfo->rst_seq_len),
			"qcom,mdss-dsi-reset-sequence");

		mdss_dsi_parse_dcs_cmds(next, &spec_pdata->einit_cmds,
			"somc,mdss-dsi-early-init-command", NULL);

		mdss_dsi_parse_dcs_cmds(next, &spec_pdata->init_cmds,
			"somc,mdss-dsi-init-command", NULL);

		mdss_dsi_parse_dcs_cmds(next, &ctrl_pdata->on_cmds,
			"qcom,mdss-dsi-on-command",
			"qcom,mdss-dsi-on-command-state");

		if (id_data)
			mdss_dsi_parse_dcs_cmds(next,
				&spec_pdata->off_cmds[DETECTED_CMDS],
				"qcom,mdss-dsi-off-command",
				"qcom,mdss-dsi-off-command-state");
		else
			mdss_dsi_parse_dcs_cmds(next,
				&spec_pdata->off_cmds[DEFAULT_CMDS],
				"qcom,mdss-dsi-off-command",
				"qcom,mdss-dsi-off-command-state");

		mdss_dsi_parse_dcs_cmds(next, &spec_pdata->id_read_cmds,
			"somc,mdss-dsi-id-read-command", NULL);

		rc = of_property_read_u32_array(next,
			"somc,mdss-phy-size-mm", res, 2);
		if (rc)
			pr_err("%s:%d, panel physical size not specified\n",
							__func__, __LINE__);
		pinfo->width = !rc ? res[0] : 0;
		pinfo->height = !rc ? res[1] : 0;

		rc = of_property_read_u32(next,
			"somc,mdss-dsi-wait-time-before-on-cmd", &tmp);
		spec_pdata->wait_time_before_on_cmd = !rc ? tmp : 0;

		rc = of_property_read_u32(next,
			"somc,mdss-dsi-disp-on-in-hs", &tmp);
		spec_pdata->disp_on_in_hs = !rc ? tmp : 0;

		rc = of_property_read_u32(next,
			"somc,mdss-dsi-init-from-begin", &tmp);
		spec_pdata->init_from_begin = !rc ? tmp : 0;

		mdss_dsi_parse_dcs_cmds(next, &spec_pdata->pre_uv_read_cmds,
			"somc,mdss-dsi-pre-uv-command", NULL);

		mdss_dsi_parse_dcs_cmds(next, &spec_pdata->uv_read_cmds,
			"somc,mdss-dsi-uv-command", NULL);

		rc = of_property_read_u32(next,
			"somc,mdss-dsi-uv-param-type", &tmp);
		spec_pdata->pcc_data.param_type =
			(!rc ? tmp : CLR_DATA_UV_PARAM_TYPE_NONE);

		rc = of_property_read_u32(next,
			"somc,mdss-dsi-pcc-table-size", &tmp);
		spec_pdata->pcc_data.tbl_size =
			(!rc ? tmp : 0);

		if (of_property_read_bool(next, "somc,mdss-dsi-pcc-table")) {
			spec_pdata->pcc_data.color_tbl =
				kzalloc(spec_pdata->pcc_data.tbl_size *
					sizeof(struct mdss_pcc_color_tbl),
					GFP_KERNEL);
			if (!spec_pdata->pcc_data.color_tbl) {
				pr_err("no mem assigned: kzalloc fail\n");
				return -ENOMEM;
			}
			rc = of_property_read_u32_array(next,
				"somc,mdss-dsi-pcc-table",
				(u32 *)spec_pdata->pcc_data.color_tbl,
				spec_pdata->pcc_data.tbl_size *
				sizeof(struct mdss_pcc_color_tbl) /
				sizeof(u32));
			if (rc) {
				spec_pdata->pcc_data.tbl_size = 0;
				kzfree(spec_pdata->pcc_data.color_tbl);
				spec_pdata->pcc_data.color_tbl = NULL;
				pr_err("%s:%d, Unable to read pcc table",
					__func__, __LINE__);
				goto error;
			}
			spec_pdata->pcc_data.pcc_sts |= PCC_STS_UD;
		}

		mdss_dsi_parse_dcs_cmds(next,
			&spec_pdata->cabc_early_on_cmds,
			"somc,mdss-dsi-cabc-early-on-command", NULL);

		mdss_dsi_parse_dcs_cmds(next,
			&spec_pdata->cabc_on_cmds,
			"somc,mdss-dsi-cabc-on-command", NULL);

		if (id_data)
			mdss_dsi_parse_dcs_cmds(next,
				&spec_pdata->cabc_off_cmds[DETECTED_CMDS],
				"somc,mdss-dsi-cabc-off-command", NULL);
		else
			mdss_dsi_parse_dcs_cmds(next,
				&spec_pdata->cabc_off_cmds[DEFAULT_CMDS],
				"somc,mdss-dsi-cabc-off-command", NULL);

		if (id_data)
			mdss_dsi_parse_dcs_cmds(next,
				&spec_pdata->cabc_late_off_cmds[DETECTED_CMDS],
				"somc,mdss-dsi-cabc-late-off-command", NULL);
		else
			mdss_dsi_parse_dcs_cmds(next,
				&spec_pdata->cabc_late_off_cmds[DEFAULT_CMDS],
				"somc,mdss-dsi-cabc-late-off-command", NULL);

		mdss_dsi_parse_dcs_cmds(next,
			&spec_pdata->cabc_deferred_on_cmds,
			"somc,mdss-dsi-cabc-deferred-on-command", NULL);

		rc = of_property_read_u32(next,
			"somc,mdss-dsi-cabc-enabled", &tmp);
		spec_pdata->cabc_enabled = !rc ? tmp : 0;

		rc = of_property_read_u32(next, "somc,disp-en-on-pre", &tmp);
		spec_pdata->on_seq.disp_en_pre = !rc ? tmp : 0;
		rc = of_property_read_u32(next, "somc,disp-en-on-post", &tmp);
		spec_pdata->on_seq.disp_en_post = !rc ? tmp : 0;
		(void)mdss_dsi_property_read_u32_var(next,
			"somc,pw-on-rst-seq",
			(u32 **)&spec_pdata->on_seq.rst_seq,
			&spec_pdata->on_seq.seq_num);
		rc = of_property_read_u32(next, "somc,disp-en-off-pre", &tmp);
		spec_pdata->off_seq.disp_en_pre = !rc ? tmp : 0;
		rc = of_property_read_u32(next, "somc,disp-en-off-post", &tmp);
		spec_pdata->off_seq.disp_en_post = !rc ? tmp : 0;
		(void)mdss_dsi_property_read_u32_var(next,
			"somc,pw-off-rst-seq",
			(u32 **)&spec_pdata->off_seq.rst_seq,
			&spec_pdata->off_seq.seq_num);
		(void)mdss_dsi_property_read_u32_var(next,
			"somc,pw-off-rst-b-seq",
			(u32 **)&spec_pdata->off_seq.rst_b_seq,
			&spec_pdata->off_seq.seq_b_num);
		rc = of_property_read_u32(next, "somc,pw-down-period", &tmp);
		spec_pdata->down_period = !rc ? tmp : 0;

		mdss_dsi_parse_dcs_cmds(next, &ctrl_pdata->fps_cmds,
			"somc,chenge-fps-command", NULL);
		rc = of_property_read_u32(next,
			"somc,fps_default", &tmp);
		pinfo->lcdc.fps_default = !rc ? tmp : 0x8E;
		rc = of_property_read_u32(next,
			"somc,display-clock", &tmp);
		pinfo->lcdc.display_clock = !rc ? tmp : 17;
		rc = of_property_read_u32(next,
			"somc,driver-ic-vbp", &tmp);
		pinfo->lcdc.driver_ic_vbp = !rc ? tmp : 2;
		rc = of_property_read_u32(next,
			"somc,driver-ic-vfp", &tmp);
		pinfo->lcdc.driver_ic_vfp = !rc ? tmp : 6;
		rc = of_property_read_u32_array(next,
			"somc,mdss-dsi-u-rev", res, 2);
		if (rc) {
			pinfo->rev_u[0] = 0;
			pinfo->rev_u[1] = 0;
		} else {
			pinfo->rev_u[0] = res[0];
			pinfo->rev_u[1] = res[1];
		}
		rc = of_property_read_u32_array(next,
			"somc,mdss-dsi-v-rev", res, 2);
		if (rc) {
			pinfo->rev_v[0] = 0;
			pinfo->rev_v[1] = 0;
		} else {
			pinfo->rev_v[0] = res[0];
			pinfo->rev_v[1] = res[1];
		}

		rc = of_property_read_u32(next,
			"somc,chenge-wait-update", &tmp);
		pinfo->lcdc.chenge_wait_update =  !rc ? tmp : 0;
		rc = of_property_read_u32_array(next,
			"somc,chenge-wait-on", res, 2);
		pinfo->lcdc.chenge_wait_on_60fps = !rc ? res[0] : 0;
		pinfo->lcdc.chenge_wait_on_45fps = !rc ? res[1] : 0;
		rc = of_property_read_u32_array(next,
			"somc,chenge-wait-off", res, 2);
		pinfo->lcdc.chenge_wait_off_60fps = !rc ? res[0] : 0;
		pinfo->lcdc.chenge_wait_off_45fps = !rc ? res[1] : 0;
		rc = of_property_read_u32_array(next,
			"somc,chenge-wait-cmds-num", res, 2);
		pinfo->lcdc.chenge_wait_on_cmds_num = !rc ? res[0] : 0;
		pinfo->lcdc.chenge_wait_off_cmds_num = !rc ? res[1] : 0;
		rc = of_property_read_u32(next,
			"somc,fps-threshold", &tmp);
		pinfo->lcdc.fps_threshold =  !rc ? tmp : 0;
		rc = of_property_read_u32(next,
			"somc,te-c-update", &tmp);
		pinfo->lcdc.te_c_update =  !rc ? tmp : 0;
		rc = of_property_read_u32_array(next,
			"somc,te-c-mode-60fps", res, 2);
		pinfo->lcdc.te_c_mode_60fps_0 = !rc ? res[0] : 0;
		pinfo->lcdc.te_c_mode_60fps_1 = !rc ? res[1] : 0;
		rc = of_property_read_u32_array(next,
			"somc,te-c-mode-45fps", res, 2);
		pinfo->lcdc.te_c_mode_45fps_0 = !rc ? res[0] : 0;
		pinfo->lcdc.te_c_mode_45fps_1 = !rc ? res[1] : 0;
		rc = of_property_read_u32(next,
			"somc,te-c-cmds-num", &tmp);
		pinfo->lcdc.te_c_cmds_num =  !rc ? tmp : 0;
		rc = of_property_read_u32(next,
			"somc,te-c-payload-num", &tmp);
		pinfo->lcdc.te_c_payload_num =  !rc ? tmp : 0;
		rc = of_property_read_u32(next,
			"somc,chenge-fps-cmds-num", &tmp);
		pinfo->lcdc.chenge_fps_cmds_num = !rc ? tmp : 0;
		rc = of_property_read_u32(next,
			"somc,chenge-fps-payload-num", &tmp);
		pinfo->lcdc.chenge_fps_payload_num = !rc ? tmp : 0;

		rc = of_get_named_gpio(next,
				"somc,mipi-rst-gpio", 0);
		if (gpio_is_valid(rc))
			spec_pdata->mipi_rst = rc;
		else
			pr_warn("%s:%d Unable to get valid GPIO for MIPI RST\n",
								__func__, __LINE__);

		spec_pdata->dsi_seq_hack = of_property_read_bool(next,
						"somc,dsi-restart-hack");

		break;
	}
	return 0;

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

		rc = gpio_tlmm_config(GPIO_CFG(
			spec_pdata->disp_p5, 0,
			GPIO_CFG_OUTPUT,
			GPIO_CFG_PULL_DOWN,
			GPIO_CFG_2MA),
			GPIO_CFG_ENABLE);
		if (rc)
			pr_err("Unable to configure P5 GPIO TLMM\n");

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

		rc = gpio_tlmm_config(GPIO_CFG(
			spec_pdata->disp_n5, 0,
			GPIO_CFG_OUTPUT,
			GPIO_CFG_PULL_DOWN,
			GPIO_CFG_2MA),
			GPIO_CFG_ENABLE);
		if (rc)
			pr_err("Unable to configure N5 GPIO TLMM\n");
	} else
		pr_warn("%s:%d Unable to get valid DISP N5 GPIO\n",
						__func__, __LINE__);

	return rc;
}

int mdss_dsi_panel_init(struct device_node *node,
	struct mdss_dsi_ctrl_pdata *ctrl_pdata,
	bool cmd_cfg_cont_splash)
{
	int rc = 0;
	char *path_name = "mdss_dsi_panel";
	struct device_node *dsi_ctrl_np = NULL;
	struct platform_device *ctrl_pdev = NULL;
	bool cont_splash_enabled;
	bool partial_update_enabled;
	
	struct mdss_panel_specific_pdata *spec_pdata = NULL;

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

	pr_debug("%s:%d, debug info", __func__, __LINE__);

	spec_pdata = ctrl_pdata->spec_pdata;
	if (!spec_pdata) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	dsi_ctrl_np = of_parse_phandle(node,
		"qcom,mdss-dsi-panel-controller", 0);
	if (!dsi_ctrl_np) {
		pr_err("%s: Dsi controller node not initialized\n",
			__func__);
		rc = -EPROBE_DEFER;
		goto error;
	}

	ctrl_pdev = of_find_device_by_node(dsi_ctrl_np);

	rc = dev_set_drvdata(&virtdev, ctrl_pdata);

	vsn_gpio = of_get_named_gpio(node, "somc,vsn-gpio", 0);
	if (gpio_is_valid(vsn_gpio)) {
		rc = gpio_request(vsn_gpio, "lcd_vsn");
		if (rc)
			dev_warn(&virtdev, 
			   "%s: Error requesting VSN GPIO %d\n",
					__func__, rc);
		gpio_direction_output(vsn_gpio, 0);
	} else {
		dev_warn(&virtdev, "%s: vsn_gpio=%d Invalid\n",
				__func__, vsn_gpio);
	}

	vsp_gpio = of_get_named_gpio(node, "somc,vsp-gpio", 0);
	if (gpio_is_valid(vsp_gpio)) {
		rc = gpio_request(vsp_gpio, "lcd_vsp");
		if (rc)
			dev_warn(&virtdev,
			   "%s: Error requesting VSP GPIO %d\n",
					__func__, rc);
		gpio_direction_output(vsp_gpio, 0);
	} else {
		dev_warn(&virtdev, "%s: vsp_gpio=%d Invalid\n",
				__func__, vsn_gpio);
	}

	spec_pdata->driver_ic = PANEL_DRIVER_IC_NONE;
	lcd_id = of_get_named_gpio(node, "somc,dric-gpio", 0);
	if (!gpio_is_valid(lcd_id)) {
		pr_err("%s:%d, DriverIC gpio not specified\n",
						__func__, __LINE__);
		goto exit_lcd_id;
	}
	rc = gpio_request(lcd_id, "lcd_id");
	if (rc) {
		pr_err("%s: request lcd id gpio failed, rc=%d\n",
			__func__, rc);
		goto exit_lcd_id;
	}
	rc = gpio_direction_input(lcd_id);
	if (rc) {
		pr_err("%s: set_direction for lcd_id gpio failed, rc=%d\n",
			__func__, rc);
		gpio_free(lcd_id);
		goto exit_lcd_id;
	}

	mdss_dsi_panel_power_detect(ctrl_pdev, 1);
	spec_pdata->driver_ic = gpio_get_value(lcd_id);
	mdss_dsi_panel_power_detect(ctrl_pdev, 0);

	pr_info("%s: gpio=%d\n", __func__, spec_pdata->driver_ic);
	gpio_free(lcd_id);
exit_lcd_id:

	if (of_property_read_bool(node, "somc,panel-id-read-cmds"))
		adc_det = false;
	if (adc_det) {
		u32 scal = 1;
		spec_pdata->lcd_id = spec_pdata->driver_ic;
		rc = of_property_read_u32(node,
				"somc,mul-channel-scaling", &scal);
		if (rc)
			pr_err("%s: Unable to read somc,mul-channel-scaling\n",
				__func__);
		spec_pdata->adc_uv = lcdid_adc * scal;
		pr_info("%s: physical:%d\n", __func__, spec_pdata->adc_uv);
	}


	alt_panelid_cmd = of_property_read_bool(node,
						"somc,alt-panelid-cmd");
	if (alt_panelid_cmd)
		mdss_dsi_panel_gpios(node, ctrl_pdata);

	rc = mdss_panel_parse_dt(node, ctrl_pdata,
		spec_pdata->driver_ic, NULL);
	if (rc)
		goto error;

	cont_splash_enabled = display_on_in_boot;

	if (!cont_splash_enabled) {
		pr_info("%s:%d Continuous splash flag not found.\n",
				__func__, __LINE__);
		ctrl_pdata->panel_data.panel_info.cont_splash_enabled = 0;
		display_onoff_state = false;
	} else {
		pr_info("%s:%d Continuous splash flag enabled.\n",
				__func__, __LINE__);

		ctrl_pdata->panel_data.panel_info.cont_splash_enabled = 1;
		display_onoff_state = true;
	}

	partial_update_enabled = of_property_read_bool(node,
						"qcom,partial-update-enabled");
	if (partial_update_enabled) {
		pr_info("%s:%d Partial update enabled.\n", __func__, __LINE__);
		ctrl_pdata->panel_data.panel_info.partial_update_enabled = 1;
		ctrl_pdata->partial_update_fnc = mdss_dsi_panel_partial_update;
	} else {
		pr_info("%s:%d Partial update disabled.\n", __func__, __LINE__);
		ctrl_pdata->panel_data.panel_info.partial_update_enabled = 0;
		ctrl_pdata->partial_update_fnc = NULL;
	}

	if (!adc_det) {
		spec_pdata->detect = mdss_dsi_panel_detect;
		spec_pdata->update_panel = mdss_dsi_panel_update_panel_info;
	}
	spec_pdata->panel_power_on = mdss_dsi_panel_power_on_ex;
	spec_pdata->reset = mdss_dsi_panel_reset_seq;
	spec_pdata->disp_on = mdss_dsi_panel_disp_on;
	spec_pdata->update_fps = mdss_dsi_panel_fps_data_update;

	ctrl_pdata->on = mdss_dsi_panel_on;
	ctrl_pdata->off = mdss_dsi_panel_off;
	ctrl_pdata->pcc_setup = mdss_dsi_panel_pcc_setup;
	ctrl_pdata->panel_data.set_backlight = mdss_dsi_panel_bl_ctrl;

	mdss_dsi_panel_fps_data_init(&fpsd);
//	mdss_dsi_panel_fps_data_init(&vpsd);

	vs_handle.vsync_handler = NULL;

	return 0;
error:
	device_unregister(&virtdev);
	return rc;
}
