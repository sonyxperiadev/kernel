/*
 * Restructured unified display panel driver for Xperia Open Devices
 *                        *** Polling ***
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

#include "../mdss_mdp.h"
#include "../mdss_dsi.h"

#include "somc_panels.h"
#include "panel_debugfs.h"

#define POLL_REG_BYTE_TO_READ 1

struct poll_ctrl *polling;

static struct dsi_ctrl_hdr poll_reg_dchdr = {
	DTYPE_DCS_READ, 1, 0, 1, 5, 1};

/*
 * send_panel_on_seq() - Sends on sequence.
 */
static int send_panel_on_seq(struct mdss_mdp_ctl *ctl,
				struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{
	int ret;
	struct mdss_panel_data *pdata = ctl->panel_data;

	if (!ctl || !pdata) {
		pr_err("%s: Invalid panel data\n", __func__);
		return -EINVAL;
	}

	ret = ctrl_pdata->on(pdata);

	if (pdata->panel_info.disp_on_in_hs) {
		if (ctrl_pdata->spec_pdata->disp_on)
			ret = ctrl_pdata->spec_pdata->disp_on(pdata);
	}

	if (ret)
		pr_err("%s: Failed to send display on sequence\n", __func__);

	pr_debug("%s: Display on sequence completed\n", __func__);
	return ret;
}

/*
 * check_dric_reg_status() - Checks value correct or not
 */
static int check_esd_status(void)
{
	int rc = 0;
	char rbuf;

	panel_cmd_read(polling->ctrl_pdata, &(polling->esd.dsi),
			NULL, &rbuf, polling->esd.nbr_bytes_to_read);

	if (polling->esd.correct_val != rbuf) {
		pr_err("%s:Target reg value isn't correct rbuf = 0x%02x\n",
			__func__, rbuf);

		rc = -EINVAL;
	}

	return rc;
}

/*
 * check_poll_status() - Checks polling status
 */
static int check_poll_status(void)
{
	if (!polling) {
		pr_err("%s: Invalid poll worker\n", __func__);
		return -EINVAL;
	}

	if (!polling->ctrl_pdata) {
		pr_err("%s: Polling Disable\n", __func__);
		return -EINVAL;
	}

	return 0;
}

/*
 * poll_panel_status() - Reads panel register.
 * @work  : driver ic status data
 */
static void poll_panel_status(struct work_struct *work)
{
	int rc = 0;
	struct mdss_data_type *mdata = mdss_mdp_get_mdata();
	struct mdss_mdp_ctl *ctl = mdata->ctl_off;

	rc = check_poll_status();
	if (rc)
		return;

	rc = check_esd_status();
	if (rc)
		send_panel_on_seq(ctl,
				polling->ctrl_pdata);
	else
		schedule_delayed_work(&polling->poll_working,
				msecs_to_jiffies(polling->intervals));
}

void poll_worker_schedule(struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{
	struct poll_ctrl *poll = NULL;
	struct mdss_panel_info *pinfo = NULL;

	if (ctrl_pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return;
	}

	if (!polling)
		return;

	poll = &ctrl_pdata->spec_pdata->polling;
	pinfo = &ctrl_pdata->panel_data.panel_info;

	if (pinfo->dsi_master == pinfo->pdest) {
		if (poll->enable) {
			poll->ctrl_pdata = ctrl_pdata;
			schedule_delayed_work(
				&poll->poll_working,
				msecs_to_jiffies(poll->intervals));
		}
	}
}

void poll_worker_cancel(struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{
	struct poll_ctrl *poll = NULL;
	struct mdss_panel_info *pinfo = NULL;

	if (ctrl_pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return;
	}

	if (!polling)
		return;

	poll = &ctrl_pdata->spec_pdata->polling;
	pinfo = &ctrl_pdata->panel_data.panel_info;

	if (pinfo->dsi_master == pinfo->pdest) {
		if (poll->enable) {
			cancel_delayed_work_sync(&poll->poll_working);
			poll->ctrl_pdata = NULL;
		}
	}
}

int panel_polling_init(struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{
	polling = NULL;

	if (!ctrl_pdata) {
		pr_err("%s: Invalid panel data\n", __func__);
		return -EINVAL;
	}

	polling = &(ctrl_pdata->spec_pdata->polling);
	INIT_DELAYED_WORK(&(polling->poll_working), poll_panel_status);

	polling->esd.dsi.payload = &polling->esd.reg;
	polling->esd.nbr_bytes_to_read = POLL_REG_BYTE_TO_READ;
	polling->esd.dsi.dchdr = poll_reg_dchdr;

	return 0;
}
