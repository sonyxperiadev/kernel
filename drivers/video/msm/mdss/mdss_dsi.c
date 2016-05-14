/* Copyright (c) 2012-2015, The Linux Foundation. All rights reserved.
 * Copyright (C) 2013-2014, Sony Mobile Communications AB.
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

#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>
#include <linux/err.h>
#include <linux/regulator/consumer.h>
#include <linux/leds-qpnp-wled.h>
#include <linux/clk.h>
#include <linux/uaccess.h>
#include <linux/pm_qos.h>

#if defined (CONFIG_REGULATOR_QPNP_LABIBB_SOMC) && \
    defined (CONFIG_FB_MSM_MDSS_SPECIFIC_PANEL)
#include <linux/regulator/qpnp-labibb-regulator.h>
#endif

#include "mdss.h"
#include "mdss_panel.h"
#include "mdss_dsi.h"
#include "mdss_debug.h"
#include "mdss_dba_utils.h"

#define XO_CLK_RATE	19200000

/* Master structure to hold all the information about the DSI/panel */
static struct mdss_dsi_data *mdss_dsi_res;

#define DSI_DISABLE_PC_LATENCY 100
#define DSI_ENABLE_PC_LATENCY PM_QOS_DEFAULT_VALUE

static struct pm_qos_request mdss_dsi_pm_qos_request;

static void mdss_dsi_pm_qos_add_request(struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{
	struct irq_info *irq_info;

	if (!ctrl_pdata || !ctrl_pdata->shared_data)
		return;

	irq_info = ctrl_pdata->dsi_hw->irq_info;

	if (!irq_info)
		return;

	mutex_lock(&ctrl_pdata->shared_data->pm_qos_lock);
	if (!ctrl_pdata->shared_data->pm_qos_req_cnt) {
		pr_debug("%s: add request irq\n", __func__);

		mdss_dsi_pm_qos_request.type = PM_QOS_REQ_AFFINE_IRQ;
		mdss_dsi_pm_qos_request.irq = irq_info->irq;
		pm_qos_add_request(&mdss_dsi_pm_qos_request,
			PM_QOS_CPU_DMA_LATENCY, PM_QOS_DEFAULT_VALUE);
	}
	ctrl_pdata->shared_data->pm_qos_req_cnt++;
	mutex_unlock(&ctrl_pdata->shared_data->pm_qos_lock);
}

static void mdss_dsi_pm_qos_remove_request(struct dsi_shared_data *sdata)
{
	if (!sdata)
		return;

	mutex_lock(&sdata->pm_qos_lock);
	if (sdata->pm_qos_req_cnt) {
		sdata->pm_qos_req_cnt--;
		if (!sdata->pm_qos_req_cnt) {
			pr_debug("%s: remove request", __func__);
			pm_qos_remove_request(&mdss_dsi_pm_qos_request);
		}
	} else {
		pr_warn("%s: unbalanced pm_qos ref count\n", __func__);
	}
	mutex_unlock(&sdata->pm_qos_lock);
}

static void mdss_dsi_pm_qos_update_request(int val)
{
	pr_debug("%s: update request %d", __func__, val);
	pm_qos_update_request(&mdss_dsi_pm_qos_request, val);
}

int mdss_dsi_pinctrl_set_state(struct mdss_dsi_ctrl_pdata *ctrl_pdata,
					bool active);

static struct mdss_dsi_ctrl_pdata *mdss_dsi_get_ctrl(u32 ctrl_id)
{
	if (ctrl_id >= DSI_CTRL_MAX || !mdss_dsi_res)
		return NULL;

	return mdss_dsi_res->ctrl_pdata[ctrl_id];
}

static void mdss_dsi_config_clk_src(struct platform_device *pdev)
{
	struct mdss_dsi_data *dsi_res = platform_get_drvdata(pdev);
	struct dsi_shared_data *sdata = dsi_res->shared_data;

	if (!sdata->ext_byte0_clk || !sdata->ext_byte1_clk ||
		!sdata->ext_pixel0_clk || !sdata->ext_pixel1_clk) {
		pr_debug("%s: config_clk_src not needed\n", __func__);
		return;
	}

	if (mdss_dsi_is_pll_src_default(sdata)) {
		/*
		 * Default Mapping:
		 * 1. dual-dsi/single-dsi:
		 *     DSI0 <--> PLL0
		 *     DSI1 <--> PLL1
		 * 2. split-dsi:
		 *     DSI0 <--> PLL0
		 *     DSI1 <--> PLL0
		 */
		sdata->byte0_parent = sdata->ext_byte0_clk;
		sdata->pixel0_parent = sdata->ext_pixel0_clk;

		if (mdss_dsi_is_hw_config_split(sdata) &&
			!sdata->split_config_independent_pll) {
			sdata->byte1_parent = sdata->byte0_parent;
			sdata->pixel1_parent = sdata->pixel0_parent;
		} else {
			sdata->byte1_parent = sdata->ext_byte1_clk;
			sdata->pixel1_parent = sdata->ext_pixel1_clk;
		}
		pr_debug("%s: default: DSI0 <--> PLL0, DSI1 <--> %s", __func__,
			mdss_dsi_is_hw_config_split(sdata) ? "PLL0" : "PLL1");
	} else {
		/*
		 * For split-dsi and single-dsi use cases, map the PLL source
		 * based on the pll source configuration. It is possible that
		 * for split-dsi case, the only supported config is to source
		 * the clocks from PLL0. This is not explictly checked here as
		 * it should have been already enforced when validating the
		 * board configuration.
		 */
		if (mdss_dsi_is_pll_src_pll0(sdata)) {
			pr_debug("%s: single source: PLL0", __func__);
			sdata->byte0_parent = sdata->ext_byte0_clk;
			sdata->pixel0_parent = sdata->ext_pixel0_clk;
		} else if (mdss_dsi_is_pll_src_pll1(sdata)) {
			pr_debug("%s: single source: PLL1", __func__);
			sdata->byte0_parent = sdata->ext_byte1_clk;
			sdata->pixel0_parent = sdata->ext_pixel1_clk;
		}
		sdata->byte1_parent = sdata->byte0_parent;
		sdata->pixel1_parent = sdata->pixel0_parent;
	}

	return;
}

static char const *mdss_dsi_get_clk_src(struct mdss_dsi_ctrl_pdata *ctrl)
{
	struct dsi_shared_data *sdata;

	if (!ctrl) {
		pr_err("%s: Invalid input data\n", __func__);
		return "????";
	}

	sdata = ctrl->shared_data;

	if (mdss_dsi_is_left_ctrl(ctrl)) {
		if (sdata->byte0_parent == sdata->ext_byte0_clk)
			return "PLL0";
		else
			return "PLL1";
	} else {
		if (sdata->byte1_parent == sdata->ext_byte0_clk)
			return "PLL0";
		else
			return "PLL1";
	}
}

static int mdss_dsi_set_clk_src(struct mdss_dsi_ctrl_pdata *ctrl)
{
	int rc;
	struct dsi_shared_data *sdata;
	struct clk *byte_parent, *pixel_parent;

	if (!ctrl) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	sdata = ctrl->shared_data;

	if (!ctrl->byte_clk_rcg || !ctrl->pixel_clk_rcg) {
		pr_debug("%s: set_clk_src not needed\n", __func__);
		return 0;
	}

	if (mdss_dsi_is_left_ctrl(ctrl)) {
		byte_parent = sdata->byte0_parent;
		pixel_parent = sdata->pixel0_parent;
	} else {
		byte_parent = sdata->byte1_parent;
		pixel_parent = sdata->pixel1_parent;
	}

	rc = clk_set_parent(ctrl->byte_clk_rcg, byte_parent);
	if (rc) {
		pr_err("%s: failed to set parent for byte clk for ctrl%d. rc=%d\n",
			__func__, ctrl->ndx, rc);
		goto error;
	}

	rc = clk_set_parent(ctrl->pixel_clk_rcg, pixel_parent);
	if (rc) {
		pr_err("%s: failed to set parent for pixel clk for ctrl%d. rc=%d\n",
			__func__, ctrl->ndx, rc);
		goto error;
	}

	pr_debug("%s: ctrl%d clock source set to %s", __func__, ctrl->ndx,
		mdss_dsi_get_clk_src(ctrl));

error:
	return rc;
}

static int mdss_dsi_regulator_init(struct platform_device *pdev,
		struct dsi_shared_data *sdata)
{
	int rc = 0, i = 0, j = 0;
#if defined (CONFIG_REGULATOR_QPNP_LABIBB_SOMC) && \
    defined (CONFIG_FB_MSM_MDSS_SPECIFIC_PANEL)
	struct mdss_dsi_ctrl_pdata *ctrl = NULL;
#endif

	if (!pdev || !sdata) {
		pr_err("%s: invalid input\n", __func__);
		return -EINVAL;
	}

	for (i = DSI_CORE_PM; !rc && (i < DSI_MAX_PM); i++) {
		rc = msm_dss_config_vreg(&pdev->dev,
			sdata->power_data[i].vreg_config,
			sdata->power_data[i].num_vreg, 1);
		if (rc) {
			pr_err("%s: failed to init vregs for %s\n",
				__func__, __mdss_dsi_pm_name(i));
			for (j = i-1; j >= DSI_CORE_PM; j--) {
				msm_dss_config_vreg(&pdev->dev,
				sdata->power_data[j].vreg_config,
				sdata->power_data[j].num_vreg, 0);
			}
		}
	}

	return rc;
}

#ifndef CONFIG_FB_MSM_MDSS_SPECIFIC_PANEL
static int mdss_dsi_panel_power_off(struct mdss_panel_data *pdata)
{
	int ret = 0;
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;

	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		ret = -EINVAL;
		goto end;
	}

	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);

	ret = mdss_dsi_panel_reset(pdata, 0);
	if (ret) {
		pr_warn("%s: Panel reset failed. rc=%d\n", __func__, ret);
		ret = 0;
	}

	if (mdss_dsi_pinctrl_set_state(ctrl_pdata, false))
		pr_debug("reset disable: pinctrl not enabled\n");

	ret = msm_dss_enable_vreg(
		ctrl_pdata->panel_power_data.vreg_config,
		ctrl_pdata->panel_power_data.num_vreg, 0);
	if (ret)
		pr_err("%s: failed to disable vregs for %s\n",
			__func__, __mdss_dsi_pm_name(DSI_PANEL_PM));

end:
	return ret;
}

static int mdss_dsi_panel_power_on(struct mdss_panel_data *pdata)
{
	int ret = 0;
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;

	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);

	ret = msm_dss_enable_vreg(
		ctrl_pdata->panel_power_data.vreg_config,
		ctrl_pdata->panel_power_data.num_vreg, 1);
	if (ret) {
		pr_err("%s: failed to enable vregs for %s\n",
			__func__, __mdss_dsi_pm_name(DSI_PANEL_PM));
		return ret;
	}

	/*
	 * If continuous splash screen feature is enabled, then we need to
	 * request all the GPIOs that have already been configured in the
	 * bootloader. This needs to be done irresepective of whether
	 * the lp11_init flag is set or not.
	 */
	if (pdata->panel_info.cont_splash_enabled ||
		!pdata->panel_info.mipi.lp11_init) {
		if (mdss_dsi_pinctrl_set_state(ctrl_pdata, true))
			pr_debug("reset enable: pinctrl not enabled\n");

		ret = mdss_dsi_panel_reset(pdata, 1);
		if (ret)
			pr_err("%s: Panel reset failed. rc=%d\n",
					__func__, ret);
	}

	return ret;
}

static int mdss_dsi_panel_power_lp(struct mdss_panel_data *pdata, int enable)
{
	/* Panel power control when entering/exiting lp mode */
	return 0;
}
#endif

static int mdss_dsi_panel_power_ctrl(struct mdss_panel_data *pdata,
	int power_state)
{
#ifdef CONFIG_FB_MSM_MDSS_SPECIFIC_PANEL
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
#else
	int ret;
	struct mdss_panel_info *pinfo;
#endif

	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

#ifdef CONFIG_FB_MSM_MDSS_SPECIFIC_PANEL
	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
						panel_data);

	return ctrl_pdata->spec_pdata->panel_power_ctrl(pdata, power_state);
#else
	pinfo = &pdata->panel_info;
	pr_debug("%s: cur_power_state=%d req_power_state=%d\n", __func__,
		pinfo->panel_power_state, power_state);

	if (pinfo->panel_power_state == power_state) {
		pr_debug("%s: no change needed\n", __func__);
		return 0;
	}

	/*
	 * If a dynamic mode switch is pending, the regulators should not
	 * be turned off or on.
	 */
	if (pdata->panel_info.dynamic_switch_pending)
		return 0;

	switch (power_state) {
	case MDSS_PANEL_POWER_OFF:
		ret = mdss_dsi_panel_power_off(pdata);
		break;
	case MDSS_PANEL_POWER_ON:
		if (mdss_dsi_is_panel_on_lp(pdata))
			ret = mdss_dsi_panel_power_lp(pdata, false);
		else
			ret = mdss_dsi_panel_power_on(pdata);
		break;
	case MDSS_PANEL_POWER_LP1:
	case MDSS_PANEL_POWER_LP2:
		ret = mdss_dsi_panel_power_lp(pdata, true);
		break;
	default:
		pr_err("%s: unknown panel power state requested (%d)\n",
			__func__, power_state);
		ret = -EINVAL;
	}

	if (!ret)
		pinfo->panel_power_state = power_state;

	return ret;
#endif
}

static void mdss_dsi_put_dt_vreg_data(struct device *dev,
	struct dss_module_power *module_power)
{
	if (!module_power) {
		pr_err("%s: invalid input\n", __func__);
		return;
	}

	if (module_power->vreg_config) {
		devm_kfree(dev, module_power->vreg_config);
		module_power->vreg_config = NULL;
	}
	module_power->num_vreg = 0;
}

static int mdss_dsi_get_dt_vreg_data(struct device *dev,
	struct device_node *of_node, struct dss_module_power *mp,
	enum dsi_pm_type module)
{
	int i = 0, rc = 0;
	u32 tmp = 0;
	struct device_node *supply_node = NULL;
	const char *pm_supply_name = NULL;
	struct device_node *supply_root_node = NULL;

	if (!dev || !mp) {
		pr_err("%s: invalid input\n", __func__);
		rc = -EINVAL;
		return rc;
	}

	mp->num_vreg = 0;
	pm_supply_name = __mdss_dsi_pm_supply_node_name(module);
	supply_root_node = of_get_child_by_name(of_node, pm_supply_name);
	if (!supply_root_node) {
		/*
		 * Try to get the root node for panel power supply using
		 * of_parse_phandle() API if of_get_child_by_name() API fails.
		 */
		supply_root_node = of_parse_phandle(of_node, pm_supply_name, 0);
		if (!supply_root_node) {
			pr_err("no supply entry present: %s\n", pm_supply_name);
			goto novreg;
		}
	}


	for_each_child_of_node(supply_root_node, supply_node) {
		mp->num_vreg++;
	}

	if (mp->num_vreg == 0) {
		pr_debug("%s: no vreg\n", __func__);
		goto novreg;
	} else {
		pr_debug("%s: vreg found. count=%d\n", __func__, mp->num_vreg);
	}

	mp->vreg_config = devm_kzalloc(dev, sizeof(struct dss_vreg) *
		mp->num_vreg, GFP_KERNEL);
	if (!mp->vreg_config) {
		pr_err("%s: can't alloc vreg mem\n", __func__);
		rc = -ENOMEM;
		goto error;
	}

	for_each_child_of_node(supply_root_node, supply_node) {
		const char *st = NULL;
		/* vreg-name */
		rc = of_property_read_string(supply_node,
			"qcom,supply-name", &st);
		if (rc) {
			pr_err("%s: error reading name. rc=%d\n",
				__func__, rc);
			goto error;
		}
		snprintf(mp->vreg_config[i].vreg_name,
			ARRAY_SIZE((mp->vreg_config[i].vreg_name)), "%s", st);
		/* vreg-min-voltage */
		rc = of_property_read_u32(supply_node,
			"qcom,supply-min-voltage", &tmp);
		if (rc) {
			pr_err("%s: error reading min volt. rc=%d\n",
				__func__, rc);
			goto error;
		}
		mp->vreg_config[i].min_voltage = tmp;

		/* vreg-max-voltage */
		rc = of_property_read_u32(supply_node,
			"qcom,supply-max-voltage", &tmp);
		if (rc) {
			pr_err("%s: error reading max volt. rc=%d\n",
				__func__, rc);
			goto error;
		}
		mp->vreg_config[i].max_voltage = tmp;

		/* enable-load */
		rc = of_property_read_u32(supply_node,
			"qcom,supply-enable-load", &tmp);
		if (rc) {
			pr_err("%s: error reading enable load. rc=%d\n",
				__func__, rc);
			goto error;
		}
		mp->vreg_config[i].enable_load = tmp;

		/* disable-load */
		rc = of_property_read_u32(supply_node,
			"qcom,supply-disable-load", &tmp);
		if (rc) {
			pr_err("%s: error reading disable load. rc=%d\n",
				__func__, rc);
			goto error;
		}
		mp->vreg_config[i].disable_load = tmp;

		/* pre-sleep */
		rc = of_property_read_u32(supply_node,
			"qcom,supply-pre-on-sleep", &tmp);
		if (rc) {
			pr_debug("%s: error reading supply pre sleep value. rc=%d\n",
				__func__, rc);
			rc = 0;
		} else {
			mp->vreg_config[i].pre_on_sleep = tmp;
		}

		rc = of_property_read_u32(supply_node,
			"qcom,supply-pre-off-sleep", &tmp);
		if (rc) {
			pr_debug("%s: error reading supply pre sleep value. rc=%d\n",
				__func__, rc);
			rc = 0;
		} else {
			mp->vreg_config[i].pre_off_sleep = tmp;
		}

		/* post-sleep */
		rc = of_property_read_u32(supply_node,
			"qcom,supply-post-on-sleep", &tmp);
		if (rc) {
			pr_debug("%s: error reading supply post sleep value. rc=%d\n",
				__func__, rc);
			rc = 0;
		} else {
			mp->vreg_config[i].post_on_sleep = tmp;
		}

		rc = of_property_read_u32(supply_node,
			"qcom,supply-post-off-sleep", &tmp);
		if (rc) {
			pr_debug("%s: error reading supply post sleep value. rc=%d\n",
				__func__, rc);
			rc = 0;
		} else {
			mp->vreg_config[i].post_off_sleep = tmp;
		}

		pr_debug("%s: %s min=%d, max=%d, enable=%d, disable=%d, preonsleep=%d, postonsleep=%d, preoffsleep=%d, postoffsleep=%d\n",
			__func__,
			mp->vreg_config[i].vreg_name,
			mp->vreg_config[i].min_voltage,
			mp->vreg_config[i].max_voltage,
			mp->vreg_config[i].enable_load,
			mp->vreg_config[i].disable_load,
			mp->vreg_config[i].pre_on_sleep,
			mp->vreg_config[i].post_on_sleep,
			mp->vreg_config[i].pre_off_sleep,
			mp->vreg_config[i].post_off_sleep
			);
		++i;
	}

	return rc;

error:
	if (mp->vreg_config) {
		devm_kfree(dev, mp->vreg_config);
		mp->vreg_config = NULL;
	}
novreg:
	mp->num_vreg = 0;

	return rc;
}

static int mdss_dsi_get_panel_cfg(char *panel_cfg,
				struct mdss_dsi_ctrl_pdata *ctrl)
{
	int rc;
	struct mdss_panel_cfg *pan_cfg = NULL;

	if (!panel_cfg)
		return MDSS_PANEL_INTF_INVALID;

	pan_cfg = ctrl->mdss_util->panel_intf_type(MDSS_PANEL_INTF_DSI);
	if (IS_ERR(pan_cfg)) {
		return PTR_ERR(pan_cfg);
	} else if (!pan_cfg) {
		panel_cfg[0] = 0;
		return 0;
	}

	pr_debug("%s:%d: cfg:[%s]\n", __func__, __LINE__,
		 pan_cfg->arg_cfg);
	rc = strlcpy(panel_cfg, pan_cfg->arg_cfg,
		     sizeof(pan_cfg->arg_cfg));
	return rc;
}

struct buf_data {
	char *buf; /* cmd buf */
	int blen; /* cmd buf length */
	char *string_buf; /* cmd buf as string, 3 bytes per number */
	int sblen; /* string buffer length */
	int sync_flag;
};

struct mdss_dsi_debugfs_info {
	struct dentry *root;
	struct mdss_dsi_ctrl_pdata ctrl_pdata;
	struct buf_data on_cmd;
	struct buf_data off_cmd;
	u32 override_flag;
};

static int mdss_dsi_cmd_state_open(struct inode *inode, struct file *file)
{
	/* non-seekable */
	file->private_data = inode->i_private;
	return nonseekable_open(inode, file);
}

static ssize_t mdss_dsi_cmd_state_read(struct file *file, char __user *buf,
				size_t count, loff_t *ppos)
{
	int *link_state = file->private_data;
	char buffer[32];
	int blen = 0;

	if (*ppos)
		return 0;

	if ((*link_state) == DSI_HS_MODE)
		blen = snprintf(buffer, sizeof(buffer), "dsi_hs_mode\n");
	else
		blen = snprintf(buffer, sizeof(buffer), "dsi_lp_mode\n");

	if (blen < 0)
		return 0;

	if (copy_to_user(buf, buffer, blen))
		return -EFAULT;

	*ppos += blen;
	return blen;
}

static ssize_t mdss_dsi_cmd_state_write(struct file *file,
			const char __user *p, size_t count, loff_t *ppos)
{
	int *link_state = file->private_data;
	char *input;

	input = kmalloc(count, GFP_KERNEL);
	if (!input) {
		pr_err("%s: Failed to allocate memory\n", __func__);
		return -ENOMEM;
	}

	if (copy_from_user(input, p, count))
		return -EFAULT;
	input[count-1] = '\0';

	if (strnstr(input, "dsi_hs_mode", strlen("dsi_hs_mode")))
		*link_state = DSI_HS_MODE;
	else
		*link_state = DSI_LP_MODE;

	kfree(input);
	return count;
}

static const struct file_operations mdss_dsi_cmd_state_fop = {
	.open = mdss_dsi_cmd_state_open,
	.read = mdss_dsi_cmd_state_read,
	.write = mdss_dsi_cmd_state_write,
};

static int mdss_dsi_cmd_open(struct inode *inode, struct file *file)
{
	/* non-seekable */
	file->private_data = inode->i_private;
	return nonseekable_open(inode, file);
}

static ssize_t mdss_dsi_cmd_read(struct file *file, char __user *buf,
				 size_t count, loff_t *ppos)
{
	struct buf_data *pcmds = file->private_data;
	char *bp;
	ssize_t ret = 0;

	if (*ppos == 0) {
		kfree(pcmds->string_buf);
		pcmds->string_buf = NULL;
		pcmds->sblen = 0;
	}

	if (!pcmds->string_buf) {
		/*
		 * Buffer size is the sum of cmd length (3 bytes per number)
		 * with NULL terminater
		 */
		int bsize = ((pcmds->blen)*3 + 1);
		int blen = 0;
		char *buffer;

		buffer = kmalloc(bsize, GFP_KERNEL);
		if (!buffer) {
			pr_err("%s: Failed to allocate memory\n", __func__);
			return -ENOMEM;
		}

		bp = pcmds->buf;
		while ((blen < (bsize-1)) &&
		       (bp < ((pcmds->buf) + (pcmds->blen)))) {
			struct dsi_ctrl_hdr dchdr =
					*((struct dsi_ctrl_hdr *)bp);
			int dhrlen = sizeof(dchdr), dlen;
			char *tmp = (char *)(&dchdr);
			dlen = dchdr.dlen;
			dchdr.dlen = htons(dchdr.dlen);
			while (dhrlen--)
				blen += snprintf(buffer+blen, bsize-blen,
						 "%02x ", (*tmp++));

			bp += sizeof(dchdr);
			while (dlen--)
				blen += snprintf(buffer+blen, bsize-blen,
						 "%02x ", (*bp++));
			buffer[blen-1] = '\n';
		}
		buffer[blen] = '\0';
		pcmds->string_buf = buffer;
		pcmds->sblen = blen;
	}

	/*
	 * The max value of count is PAGE_SIZE(4096).
	 * It may need multiple times of reading if string buf is too large
	 */
	if (*ppos >= (pcmds->sblen)) {
		kfree(pcmds->string_buf);
		pcmds->string_buf = NULL;
		pcmds->sblen = 0;
		return 0; /* the end */
	}
	ret = simple_read_from_buffer(buf, count, ppos, pcmds->string_buf,
				      pcmds->sblen);
	return ret;
}

static ssize_t mdss_dsi_cmd_write(struct file *file, const char __user *p,
				  size_t count, loff_t *ppos)
{
	struct buf_data *pcmds = file->private_data;
	ssize_t ret = 0;
	int blen = 0;
	char *string_buf;

	if (*ppos == 0) {
		kfree(pcmds->string_buf);
		pcmds->string_buf = NULL;
		pcmds->sblen = 0;
	}

	/* Allocate memory for the received string */
	blen = count + (pcmds->sblen);
	string_buf = krealloc(pcmds->string_buf, blen + 1, GFP_KERNEL);
	if (!string_buf) {
		pr_err("%s: Failed to allocate memory\n", __func__);
		return -ENOMEM;
	}

	/* Writing in batches is possible */
	ret = simple_write_to_buffer(string_buf, blen, ppos, p, count);

	string_buf[blen] = '\0';
	pcmds->string_buf = string_buf;
	pcmds->sblen = blen;
	return ret;
}

static int mdss_dsi_cmd_flush(struct file *file, fl_owner_t id)
{
	struct buf_data *pcmds = file->private_data;
	int blen, len, i;
	char *buf, *bufp, *bp;
	struct dsi_ctrl_hdr *dchdr;

	if (!pcmds->string_buf)
		return 0;

	/*
	 * Allocate memory for command buffer
	 * 3 bytes per number, and 2 bytes for the last one
	 */
	blen = ((pcmds->sblen) + 2) / 3;
	buf = kzalloc(blen, GFP_KERNEL);
	if (!buf) {
		pr_err("%s: Failed to allocate memory\n", __func__);
		kfree(pcmds->string_buf);
		pcmds->string_buf = NULL;
		pcmds->sblen = 0;
		return -ENOMEM;
	}

	/* Translate the input string to command array */
	bufp = pcmds->string_buf;
	for (i = 0; i < blen; i++) {
		uint32_t value = 0;
		int step = 0;
		if (sscanf(bufp, "%02x%n", &value, &step) > 0) {
			*(buf+i) = (char)value;
			bufp += step;
		}
	}

	/* Scan dcs commands */
	bp = buf;
	len = blen;
	while (len >= sizeof(*dchdr)) {
		dchdr = (struct dsi_ctrl_hdr *)bp;
		dchdr->dlen = ntohs(dchdr->dlen);
		if (dchdr->dlen > len) {
			pr_err("%s: dtsi cmd=%x error, len=%d\n",
				__func__, dchdr->dtype, dchdr->dlen);
			kfree(buf);
			return -EINVAL;
		}
		bp += sizeof(*dchdr);
		len -= sizeof(*dchdr);
		bp += dchdr->dlen;
		len -= dchdr->dlen;
	}
	if (len != 0) {
		pr_err("%s: dcs_cmd=%x len=%d error!\n", __func__,
				bp[0], len);
		kfree(buf);
		return -EINVAL;
	}

	if (pcmds->sync_flag) {
		pcmds->buf = buf;
		pcmds->blen = blen;
		pcmds->sync_flag = 0;
	} else {
		kfree(pcmds->buf);
		pcmds->buf = buf;
		pcmds->blen = blen;
	}
	return 0;
}

static const struct file_operations mdss_dsi_cmd_fop = {
	.open = mdss_dsi_cmd_open,
	.read = mdss_dsi_cmd_read,
	.write = mdss_dsi_cmd_write,
	.flush = mdss_dsi_cmd_flush,
};

struct dentry *dsi_debugfs_create_dcs_cmd(const char *name, umode_t mode,
				struct dentry *parent, struct buf_data *cmd,
				struct dsi_panel_cmds ctrl_cmds)
{
	cmd->buf = ctrl_cmds.buf;
	cmd->blen = ctrl_cmds.blen;
	cmd->string_buf = NULL;
	cmd->sblen = 0;
	cmd->sync_flag = 1;

	return debugfs_create_file(name, mode, parent,
				   cmd, &mdss_dsi_cmd_fop);
}

#define DEBUGFS_CREATE_DCS_CMD(name, node, cmd, ctrl_cmd) \
	dsi_debugfs_create_dcs_cmd(name, 0644, node, cmd, ctrl_cmd)

static int mdss_dsi_debugfs_setup(struct mdss_panel_data *pdata,
			struct dentry *parent)
{
	struct mdss_dsi_ctrl_pdata *ctrl_pdata, *dfs_ctrl;
	struct mdss_dsi_debugfs_info *dfs;

	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);

	dfs = kzalloc(sizeof(*dfs), GFP_KERNEL);
	if (!dfs) {
		pr_err("%s: No memory to create dsi ctrl debugfs info",
			__func__);
		return -ENOMEM;
	}

	dfs->root = debugfs_create_dir("dsi_ctrl_pdata", parent);
	if (IS_ERR_OR_NULL(dfs->root)) {
		pr_err("%s: debugfs_create_dir dsi fail, error %ld\n",
			__func__, PTR_ERR(dfs->root));
		kfree(dfs);
		return -ENODEV;
	}

	dfs_ctrl = &dfs->ctrl_pdata;
	debugfs_create_u32("override_flag", 0644, dfs->root,
			   &dfs->override_flag);

	debugfs_create_bool("cmd_sync_wait_broadcast", 0644, dfs->root,
			    (u32 *)&dfs_ctrl->cmd_sync_wait_broadcast);
	debugfs_create_bool("cmd_sync_wait_trigger", 0644, dfs->root,
			    (u32 *)&dfs_ctrl->cmd_sync_wait_trigger);

	debugfs_create_file("dsi_on_cmd_state", 0644, dfs->root,
		&dfs_ctrl->on_cmds.link_state, &mdss_dsi_cmd_state_fop);
	debugfs_create_file("dsi_off_cmd_state", 0644, dfs->root,
		&dfs_ctrl->off_cmds.link_state, &mdss_dsi_cmd_state_fop);

	DEBUGFS_CREATE_DCS_CMD("dsi_on_cmd", dfs->root, &dfs->on_cmd,
				ctrl_pdata->on_cmds);
	DEBUGFS_CREATE_DCS_CMD("dsi_off_cmd", dfs->root, &dfs->off_cmd,
				ctrl_pdata->off_cmds);

	dfs->override_flag = 0;
	dfs->ctrl_pdata = *ctrl_pdata;
	ctrl_pdata->debugfs_info = dfs;
	return 0;
}

static int mdss_dsi_debugfs_init(struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{
	struct mdss_panel_data *pdata = &ctrl_pdata->panel_data;
	int rc;

	do {
		struct mdss_panel_info panel_info = pdata->panel_info;
		if (panel_info.debugfs_info) {
			rc = mdss_dsi_debugfs_setup(pdata,
					panel_info.debugfs_info->root);
			if (rc) {
				pr_err("%s: Error in initilizing dsi ctrl debugfs\n",
						__func__);
				return rc;
			}
		}
		pdata = pdata->next;
	} while (pdata);

	pr_debug("%s: Initialized mdss_dsi_debugfs_init\n", __func__);
	return 0;
}

static void mdss_dsi_debugfs_cleanup(struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{
	struct mdss_panel_data *pdata = &ctrl_pdata->panel_data;

	do {
		struct mdss_dsi_ctrl_pdata *ctrl = container_of(pdata,
			struct mdss_dsi_ctrl_pdata, panel_data);
		struct mdss_dsi_debugfs_info *dfs = ctrl->debugfs_info;
		if (dfs && dfs->root)
			debugfs_remove_recursive(dfs->root);
		pdata = pdata->next;
	} while (pdata);
	pr_debug("%s: Cleaned up mdss_dsi_debugfs_info\n", __func__);
}

static int _mdss_dsi_refresh_cmd(struct buf_data *new_cmds,
	struct dsi_panel_cmds *original_pcmds)
{
	char *bp;
	int len, cnt, i;
	struct dsi_ctrl_hdr *dchdr;
	struct dsi_cmd_desc *cmds;

	if (new_cmds->sync_flag)
		return 0;

	bp = new_cmds->buf;
	len = new_cmds->blen;
	cnt = 0;
	/* Scan dcs commands and get dcs command count */
	while (len >= sizeof(*dchdr)) {
		dchdr = (struct dsi_ctrl_hdr *)bp;
		if (dchdr->dlen > len) {
			pr_err("%s: dtsi cmd=%x error, len=%d\n",
				__func__, dchdr->dtype, dchdr->dlen);
			return -EINVAL;
		}
		bp += sizeof(*dchdr) + dchdr->dlen;
		len -= sizeof(*dchdr) + dchdr->dlen;
		cnt++;
	}

	if (len != 0) {
		pr_err("%s: dcs_cmd=%x len=%d error!\n", __func__,
				bp[0], len);
		return -EINVAL;
	}

	/* Reallocate space for dcs commands */
	cmds = kzalloc(cnt * sizeof(struct dsi_cmd_desc), GFP_KERNEL);
	if (!cmds) {
		pr_err("%s: Failed to allocate memory\n", __func__);
		return -ENOMEM;
	}
	kfree(original_pcmds->buf);
	kfree(original_pcmds->cmds);
	original_pcmds->cmd_cnt = cnt;
	original_pcmds->cmds = cmds;
	original_pcmds->buf = new_cmds->buf;
	original_pcmds->blen = new_cmds->blen;

	bp = original_pcmds->buf;
	len = original_pcmds->blen;
	for (i = 0; i < cnt; i++) {
		dchdr = (struct dsi_ctrl_hdr *)bp;
		len -= sizeof(*dchdr);
		bp += sizeof(*dchdr);
		original_pcmds->cmds[i].dchdr = *dchdr;
		original_pcmds->cmds[i].payload = bp;
		bp += dchdr->dlen;
		len -= dchdr->dlen;
	}

	new_cmds->sync_flag = 1;
	return 0;
}

static void mdss_dsi_debugfsinfo_to_dsictrl_info(
			struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{
	struct mdss_dsi_debugfs_info *dfs = ctrl_pdata->debugfs_info;

	ctrl_pdata->cmd_sync_wait_broadcast =
			dfs->ctrl_pdata.cmd_sync_wait_broadcast;
	ctrl_pdata->cmd_sync_wait_trigger =
			dfs->ctrl_pdata.cmd_sync_wait_trigger;

	_mdss_dsi_refresh_cmd(&dfs->on_cmd, &ctrl_pdata->on_cmds);
	_mdss_dsi_refresh_cmd(&dfs->off_cmd, &ctrl_pdata->off_cmds);

	ctrl_pdata->on_cmds.link_state =
			dfs->ctrl_pdata.on_cmds.link_state;
	ctrl_pdata->off_cmds.link_state =
			dfs->ctrl_pdata.off_cmds.link_state;
}

static void mdss_dsi_validate_debugfs_info(
		struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{
	struct mdss_dsi_debugfs_info *dfs = ctrl_pdata->debugfs_info;

	if (dfs->override_flag) {
		pr_debug("%s: Overriding dsi ctrl_pdata with debugfs data\n",
			__func__);
		dfs->override_flag = 0;
		mdss_dsi_debugfsinfo_to_dsictrl_info(ctrl_pdata);
	}
}

static int mdss_dsi_off(struct mdss_panel_data *pdata, int power_state)
{
	int ret = 0;
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	struct mdss_panel_info *panel_info = NULL;

	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);

	mutex_lock(&ctrl_pdata->mutex);
	panel_info = &ctrl_pdata->panel_data.panel_info;

	pr_debug("%s+: ctrl=%p ndx=%d power_state=%d\n",
		__func__, ctrl_pdata, ctrl_pdata->ndx, power_state);

	if (power_state == panel_info->panel_power_state) {
		pr_debug("%s: No change in power state %d -> %d\n", __func__,
			panel_info->panel_power_state, power_state);
		goto end;
	}

	if (mdss_panel_is_power_on(power_state)) {
		pr_debug("%s: dsi_off with panel always on\n", __func__);
		goto panel_power_ctrl;
	}

	if (pdata->panel_info.type == MIPI_CMD_PANEL)
		mdss_dsi_clk_ctrl(ctrl_pdata, DSI_ALL_CLKS, 1);

	if (!pdata->panel_info.ulps_suspend_enabled) {
		/* disable DSI controller */
		mdss_dsi_controller_cfg(0, pdata);

		/* disable DSI phy */
		mdss_dsi_phy_disable(ctrl_pdata);
	}
	ctrl_pdata->ctrl_state &= ~CTRL_STATE_DSI_ACTIVE;

	mdss_dsi_clk_ctrl(ctrl_pdata, DSI_ALL_CLKS, 0);

panel_power_ctrl:
	ret = mdss_dsi_panel_power_ctrl(pdata, power_state);
	if (ret) {
		pr_err("%s: Panel power off failed\n", __func__);
		goto end;
	}

	if (panel_info->dynamic_fps
	    && (panel_info->dfps_update == DFPS_SUSPEND_RESUME_MODE)
	    && (panel_info->new_fps != panel_info->mipi.frame_rate))
		panel_info->mipi.frame_rate = panel_info->new_fps;

end:
	mutex_unlock(&ctrl_pdata->mutex);
	pr_debug("%s-:\n", __func__);

	return ret;
}

int mdss_dsi_switch_mode(struct mdss_panel_data *pdata, int mode)
{
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	struct mipi_panel_info *pinfo;

	if (!pdata) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}
	pr_debug("%s, start\n", __func__);

	pinfo = &pdata->panel_info.mipi;
	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
		panel_data);

	if ((pinfo->dms_mode != DYNAMIC_MODE_RESOLUTION_SWITCH_IMMEDIATE) &&
			(pinfo->dms_mode != DYNAMIC_MODE_SWITCH_IMMEDIATE)) {
		pr_err("%s: Dynamic mode switch not enabled.\n", __func__);
		return -EPERM;
	}

	if (mode == MIPI_VIDEO_PANEL) {
		mode = SWITCH_TO_VIDEO_MODE;
	} else if (mode == MIPI_CMD_PANEL) {
		mode = SWITCH_TO_CMD_MODE;
	} else if (mode == SWITCH_RESOLUTION) {
		pr_debug("Resolution switch mode selected\n");
	} else {
		pr_err("Invalid mode selected, mode=%d\n", mode);
		return -EINVAL;
	}

	mdss_dsi_clk_ctrl(ctrl_pdata, DSI_ALL_CLKS, 1);
	ctrl_pdata->switch_mode(pdata, mode);
	mdss_dsi_clk_ctrl(ctrl_pdata, DSI_ALL_CLKS, 0);

	pr_debug("%s, end\n", __func__);
	return 0;
}

static int mdss_dsi_reconfig(struct mdss_panel_data *pdata, int mode)
{
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	struct mipi_panel_info *pinfo;

	if (!pdata) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}
	pr_debug("%s, start\n", __func__);

	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
		panel_data);
	pinfo = &pdata->panel_info.mipi;

	if (pinfo->dms_mode == DYNAMIC_MODE_SWITCH_IMMEDIATE) {
		/* reset DSI */
		mdss_dsi_clk_ctrl(ctrl_pdata, DSI_ALL_CLKS, 1);
		mdss_dsi_sw_reset(ctrl_pdata, true);
		mdss_dsi_ctrl_setup(ctrl_pdata);
		mdss_dsi_controller_cfg(true, pdata);
		mdss_dsi_clk_ctrl(ctrl_pdata, DSI_ALL_CLKS, 0);
	}

	pr_debug("%s, end\n", __func__);
	return 0;
}
static int mdss_dsi_update_panel_config(struct mdss_dsi_ctrl_pdata *ctrl_pdata,
				int mode)
{
	int ret = 0;
	struct mdss_panel_info *pinfo = &(ctrl_pdata->panel_data.panel_info);

	if (mode == DSI_CMD_MODE) {
		pinfo->mipi.mode = DSI_CMD_MODE;
		pinfo->type = MIPI_CMD_PANEL;
		pinfo->mipi.vsync_enable = 1;
		pinfo->mipi.hw_vsync_mode = 1;
		pinfo->partial_update_enabled = pinfo->partial_update_supported;
	} else {	/*video mode*/
		pinfo->mipi.mode = DSI_VIDEO_MODE;
		pinfo->type = MIPI_VIDEO_PANEL;
		pinfo->mipi.vsync_enable = 0;
		pinfo->mipi.hw_vsync_mode = 0;
		pinfo->partial_update_enabled = 0;
	}

	ctrl_pdata->panel_mode = pinfo->mipi.mode;
	mdss_panel_get_dst_fmt(pinfo->bpp, pinfo->mipi.mode,
			pinfo->mipi.pixel_packing, &(pinfo->mipi.dst_format));
	pinfo->cont_splash_enabled = 0;

	return ret;
}

#ifdef CONFIG_FB_MSM_MDSS_SPECIFIC_PANEL
static void mdss_dsi_reset_dual_display(struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{
	struct mdss_dsi_ctrl_pdata *mctrl_pdata = NULL;

	if (mdss_dsi_res->shared_data->hw_config != SPLIT_DSI) {
		if (mdss_dsi_pinctrl_set_state(ctrl_pdata, true))
			pr_debug("reset enable: pinctrl not enabled\n");
		mdss_dsi_panel_reset(&(ctrl_pdata->panel_data), 1);
	} else if (ctrl_pdata->ndx == DSI_CTRL_1) {
		mctrl_pdata = mdss_dsi_get_other_ctrl(ctrl_pdata);
		if (!mctrl_pdata) {
			pr_warn("%s: Unable to get other control\n",
				__func__);
		} else {
			if (mdss_dsi_pinctrl_set_state(mctrl_pdata, true))
				pr_debug("other reset pinctrl not enabled\n");
			mdss_dsi_panel_reset(&(mctrl_pdata->panel_data), 1);
		}
	} else {
		pr_debug("%s: reset pinctrl not yet\n", __func__);
	}
}
#endif

int mdss_dsi_on(struct mdss_panel_data *pdata)
{
	int ret = 0;
	struct mdss_panel_info *pinfo;
	struct mipi_panel_info *mipi;
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	int cur_power_state;

	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);

	if (ctrl_pdata->debugfs_info)
		mdss_dsi_validate_debugfs_info(ctrl_pdata);

	cur_power_state = pdata->panel_info.panel_power_state;
	pr_debug("%s+: ctrl=%p ndx=%d cur_power_state=%d\n", __func__,
		ctrl_pdata, ctrl_pdata->ndx, cur_power_state);

	pinfo = &pdata->panel_info;
	mipi = &pdata->panel_info.mipi;

	if (mdss_dsi_is_panel_on_interactive(pdata)) {
		/*
		 * all interrupts are disabled at LK
		 * for cont_splash case, intr mask bits need
		 * to be restored to allow dcs command be
		 * sent to panel
		 */
		mdss_dsi_restore_intr_mask(ctrl_pdata);
		pr_debug("%s: panel already on\n", __func__);
		goto end;
	}

	ret = mdss_dsi_panel_power_ctrl(pdata, MDSS_PANEL_POWER_ON);
	if (ret) {
		pr_err("%s:Panel power on failed. rc=%d\n", __func__, ret);
		goto end;
	}

	ret = mdss_dsi_set_clk_src(ctrl_pdata);
	if (ret) {
		pr_err("%s: failed to set clk src. rc=%d\n", __func__, ret);
		goto end;
	}

	if (mdss_panel_is_power_on(cur_power_state)) {
		pr_debug("%s: dsi_on from panel low power state\n", __func__);
		goto end;
	}

	/*
	 * Enable DSI core clocks prior to resetting and initializing DSI
	 * Phy. Phy and ctrl setup need to be done before enabling the link
	 * clocks.
	 */
	mdss_dsi_clk_ctrl(ctrl_pdata, DSI_CORE_CLKS, 1);

	/*
	 * If ULPS during suspend feature is enabled, then DSI PHY was
	 * left on during suspend. In this case, we do not need to reset/init
	 * PHY. This would have already been done when the core clocks are
	 * turned on. However, if cont splash is disabled, the first time DSI
	 * is powered on, phy init needs to be done unconditionally.
	 */
	if (!pdata->panel_info.ulps_suspend_enabled || !ctrl_pdata->ulps) {
		mdss_dsi_phy_sw_reset(ctrl_pdata);
		mdss_dsi_phy_init(ctrl_pdata);
		mdss_dsi_ctrl_setup(ctrl_pdata);
		pr_debug("%s: reset Phy and call ctrl_setup\n", __func__);
	}
	ctrl_pdata->ctrl_state |= CTRL_STATE_DSI_ACTIVE;

	/* DSI link clocks need to be on prior to ctrl sw reset */
	mdss_dsi_clk_ctrl(ctrl_pdata, DSI_LINK_CLKS, 1);
	mdss_dsi_sw_reset(ctrl_pdata, true);

	/*
	 * Issue hardware reset line after enabling the DSI clocks and data
	 * data lanes for LP11 init
	 */
	if (mipi->lp11_init) {
#ifdef CONFIG_FB_MSM_MDSS_SPECIFIC_PANEL
		mdss_dsi_reset_dual_display(ctrl_pdata);
#else
		if (mdss_dsi_pinctrl_set_state(ctrl_pdata, true))
			pr_debug("reset enable: pinctrl not enabled\n");
		mdss_dsi_panel_reset(pdata, 1);
#endif
	}

	if (mipi->init_delay)
		usleep(mipi->init_delay);

	if (mipi->force_clk_lane_hs) {
		u32 tmp;

		tmp = MIPI_INP((ctrl_pdata->ctrl_base) + 0xac);
		tmp |= (1<<28);
		MIPI_OUTP((ctrl_pdata->ctrl_base) + 0xac, tmp);
		wmb();
	}

	if (pdata->panel_info.type == MIPI_CMD_PANEL)
		mdss_dsi_clk_ctrl(ctrl_pdata, DSI_ALL_CLKS, 0);

end:
	pr_debug("%s-:\n", __func__);
	return ret;
}

int mdss_dsi_pinctrl_set_state(
	struct mdss_dsi_ctrl_pdata *ctrl_pdata,
	bool active)
{
	struct pinctrl_state *pin_state;
	struct mdss_panel_info *pinfo = NULL;
	int rc = -EFAULT;

#ifdef CONFIG_FB_MSM_MDSS_SPECIFIC_PANEL
	if (ctrl_pdata->spec_pdata->disp_on_in_boot)
		return 0;
#endif

	if (IS_ERR_OR_NULL(ctrl_pdata->pin_res.pinctrl))
		return PTR_ERR(ctrl_pdata->pin_res.pinctrl);

	pinfo = &ctrl_pdata->panel_data.panel_info;
	if ((mdss_dsi_is_right_ctrl(ctrl_pdata) &&
		mdss_dsi_is_hw_config_split(ctrl_pdata->shared_data)) ||
			pinfo->is_dba_panel) {
		pr_debug("%s:%d, pinctrl config not needed\n",
			__func__, __LINE__);
		return 0;
	}

	pin_state = active ? ctrl_pdata->pin_res.gpio_state_active
				: ctrl_pdata->pin_res.gpio_state_suspend;
	if (!IS_ERR_OR_NULL(pin_state)) {
		rc = pinctrl_select_state(ctrl_pdata->pin_res.pinctrl,
				pin_state);
		if (rc)
			pr_err("%s: can not set %s pins\n", __func__,
			       active ? MDSS_PINCTRL_STATE_DEFAULT
			       : MDSS_PINCTRL_STATE_SLEEP);
	} else {
		pr_err("%s: invalid '%s' pinstate\n", __func__,
		       active ? MDSS_PINCTRL_STATE_DEFAULT
		       : MDSS_PINCTRL_STATE_SLEEP);
	}
	return rc;
}

static int mdss_dsi_pinctrl_init(struct platform_device *pdev)
{
	struct mdss_dsi_ctrl_pdata *ctrl_pdata;

	ctrl_pdata = platform_get_drvdata(pdev);
	ctrl_pdata->pin_res.pinctrl = devm_pinctrl_get(&pdev->dev);
	if (IS_ERR_OR_NULL(ctrl_pdata->pin_res.pinctrl)) {
		pr_err("%s: failed to get pinctrl\n", __func__);
		return PTR_ERR(ctrl_pdata->pin_res.pinctrl);
	}

	ctrl_pdata->pin_res.gpio_state_active
		= pinctrl_lookup_state(ctrl_pdata->pin_res.pinctrl,
				MDSS_PINCTRL_STATE_DEFAULT);
	if (IS_ERR_OR_NULL(ctrl_pdata->pin_res.gpio_state_active))
		pr_warn("%s: can not get default pinstate\n", __func__);

	ctrl_pdata->pin_res.gpio_state_suspend
		= pinctrl_lookup_state(ctrl_pdata->pin_res.pinctrl,
				MDSS_PINCTRL_STATE_SLEEP);
	if (IS_ERR_OR_NULL(ctrl_pdata->pin_res.gpio_state_suspend))
		pr_warn("%s: can not get sleep pinstate\n", __func__);

	return 0;
}

static int mdss_dsi_unblank(struct mdss_panel_data *pdata)
{
	int ret = 0;
	struct mipi_panel_info *mipi;
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;

	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);
	mipi  = &pdata->panel_info.mipi;

	pr_debug("%s+: ctrl=%p ndx=%d cur_power_state=%d\n", __func__,
		ctrl_pdata, ctrl_pdata->ndx,
		pdata->panel_info.panel_power_state);

	mdss_dsi_pm_qos_update_request(DSI_DISABLE_PC_LATENCY);

	mdss_dsi_clk_ctrl(ctrl_pdata, DSI_ALL_CLKS, 1);

	if (mdss_dsi_is_panel_on_lp(pdata)) {
		pr_debug("%s: dsi_unblank with panel always on\n", __func__);
		if (ctrl_pdata->low_power_config)
			ret = ctrl_pdata->low_power_config(pdata, false);
		goto error;
	}

	if (!(ctrl_pdata->ctrl_state & CTRL_STATE_PANEL_INIT)) {
		if (!pdata->panel_info.dynamic_switch_pending) {
			ret = ctrl_pdata->on(pdata);
			if (ret) {
				pr_err("%s: unable to initialize the panel\n",
							__func__);
				goto error;
			}
		}
		ctrl_pdata->ctrl_state |= CTRL_STATE_PANEL_INIT;
	}

	if ((pdata->panel_info.type == MIPI_CMD_PANEL) &&
		mipi->vsync_enable && mipi->hw_vsync_mode) {
#ifdef CONFIG_FB_MSM_MDSS_SPECIFIC_PANEL
		if (pdata->panel_info.dsi_master == pdata->panel_info.pdest)
			mdss_dsi_set_tear_on(ctrl_pdata);
#else
		mdss_dsi_set_tear_on(ctrl_pdata);
#endif
		if (mdss_dsi_is_te_based_esd(ctrl_pdata))
			enable_irq(gpio_to_irq(ctrl_pdata->disp_te_gpio));
	}

error:
	mdss_dsi_clk_ctrl(ctrl_pdata, DSI_ALL_CLKS, 0);

	mdss_dsi_pm_qos_update_request(DSI_ENABLE_PC_LATENCY);

	pr_debug("%s-:\n", __func__);

	return ret;
}

static int mdss_dsi_blank(struct mdss_panel_data *pdata, int power_state)
{
	int ret = 0;
	struct mipi_panel_info *mipi;
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;

	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);
	mipi = &pdata->panel_info.mipi;

	pr_debug("%s+: ctrl=%p ndx=%d power_state=%d\n",
		__func__, ctrl_pdata, ctrl_pdata->ndx, power_state);

	mdss_dsi_clk_ctrl(ctrl_pdata, DSI_ALL_CLKS, 1);
	if (mdss_panel_is_power_on_lp(power_state)) {
		pr_debug("%s: low power state requested\n", __func__);
		if (ctrl_pdata->low_power_config)
			ret = ctrl_pdata->low_power_config(pdata, true);
		goto error;
	}

	if (pdata->panel_info.type == MIPI_VIDEO_PANEL &&
			ctrl_pdata->off_cmds.link_state == DSI_LP_MODE) {
		mdss_dsi_sw_reset(ctrl_pdata, false);
		mdss_dsi_host_init(pdata);
	}

	mdss_dsi_op_mode_config(DSI_CMD_MODE, pdata);

	if (pdata->panel_info.dynamic_switch_pending) {
		pr_info("%s: switching to %s mode\n", __func__,
			(pdata->panel_info.mipi.mode ? "video" : "command"));
		if (pdata->panel_info.type == MIPI_CMD_PANEL) {
			ctrl_pdata->switch_mode(pdata, SWITCH_TO_VIDEO_MODE);
		} else if (pdata->panel_info.type == MIPI_VIDEO_PANEL) {
			ctrl_pdata->switch_mode(pdata, SWITCH_TO_CMD_MODE);
#ifdef CONFIG_FB_MSM_MDSS_SPECIFIC_PANEL
			if (pdata->panel_info.dsi_master == pdata->panel_info.pdest)
				mdss_dsi_set_tear_off(ctrl_pdata);
#else
			mdss_dsi_set_tear_off(ctrl_pdata);
#endif
		}
	}

	if ((pdata->panel_info.type == MIPI_CMD_PANEL) &&
		mipi->vsync_enable && mipi->hw_vsync_mode) {
		if (mdss_dsi_is_te_based_esd(ctrl_pdata)) {
				disable_irq(gpio_to_irq(
					ctrl_pdata->disp_te_gpio));
				atomic_dec(&ctrl_pdata->te_irq_ready);
		}
#ifdef CONFIG_FB_MSM_MDSS_SPECIFIC_PANEL
		if (pdata->panel_info.dsi_master == pdata->panel_info.pdest)
			mdss_dsi_set_tear_off(ctrl_pdata);
#else
		mdss_dsi_set_tear_off(ctrl_pdata);
#endif
	}

	if (ctrl_pdata->ctrl_state & CTRL_STATE_PANEL_INIT) {
		if (!pdata->panel_info.dynamic_switch_pending) {
			ret = ctrl_pdata->off(pdata);
			if (ret) {
				pr_err("%s: Panel OFF failed\n", __func__);
				goto error;
			}
		}
		ctrl_pdata->ctrl_state &= ~CTRL_STATE_PANEL_INIT;
	}

error:
	mdss_dsi_clk_ctrl(ctrl_pdata, DSI_ALL_CLKS, 0);
	pr_debug("%s-:End\n", __func__);
	return ret;
}

static int mdss_dsi_post_panel_on(struct mdss_panel_data *pdata)
{
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;

	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);

	pr_debug("%s+: ctrl=%p ndx=%d\n", __func__,
				ctrl_pdata, ctrl_pdata->ndx);

	mdss_dsi_clk_ctrl(ctrl_pdata, DSI_ALL_CLKS, 1);

	if (ctrl_pdata->post_panel_on)
		ctrl_pdata->post_panel_on(pdata);

	mdss_dsi_clk_ctrl(ctrl_pdata, DSI_ALL_CLKS, 0);
	pr_debug("%s-:\n", __func__);

	return 0;
}

int mdss_dsi_cont_splash_on(struct mdss_panel_data *pdata)
{
	int ret = 0;
	struct mipi_panel_info *mipi;
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;

	pr_info("%s:%d DSI on for continuous splash.\n", __func__, __LINE__);

	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	mipi = &pdata->panel_info.mipi;

	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);

	pr_debug("%s+: ctrl=%p ndx=%d\n", __func__,
				ctrl_pdata, ctrl_pdata->ndx);

	WARN((ctrl_pdata->ctrl_state & CTRL_STATE_PANEL_INIT),
		"Incorrect Ctrl state=0x%x\n", ctrl_pdata->ctrl_state);

	mdss_dsi_ctrl_setup(ctrl_pdata);
	mdss_dsi_sw_reset(ctrl_pdata, true);
	pr_debug("%s-:End\n", __func__);
	return ret;
}

static void __mdss_dsi_update_video_mode_total(struct mdss_panel_data *pdata,
		int new_fps)
{
	u32 hsync_period, vsync_period, ctrl_rev;
	u32 new_dsi_v_total, current_dsi_v_total;
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;

	if (pdata == NULL) {
		pr_err("%s Invalid pdata\n", __func__);
		return;
	}

	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
			panel_data);
	if (ctrl_pdata == NULL) {
		pr_err("%s Invalid ctrl_pdata\n", __func__);
		return;
	}

	vsync_period =
		mdss_panel_get_vtotal(&pdata->panel_info);
	hsync_period =
		mdss_panel_get_htotal(&pdata->panel_info, true);
	current_dsi_v_total =
		MIPI_INP((ctrl_pdata->ctrl_base) + 0x2C);
	new_dsi_v_total =
		((vsync_period - 1) << 16) | (hsync_period - 1);

	MIPI_OUTP((ctrl_pdata->ctrl_base) + 0x2C,
			(current_dsi_v_total | 0x8000000));
	if (new_dsi_v_total & 0x8000000) {
		MIPI_OUTP((ctrl_pdata->ctrl_base) + 0x2C,
				new_dsi_v_total);
	} else {
		MIPI_OUTP((ctrl_pdata->ctrl_base) + 0x2C,
				(new_dsi_v_total | 0x8000000));
		MIPI_OUTP((ctrl_pdata->ctrl_base) + 0x2C,
				(new_dsi_v_total & 0x7ffffff));
	}
	ctrl_rev = MIPI_INP(ctrl_pdata->ctrl_base);
	/* Flush DSI TIMING registers for 8916/8939 */
	if (ctrl_pdata->shared_data->timing_db_mode)
		MIPI_OUTP((ctrl_pdata->ctrl_base) + 0x1e4, 0x1);
	ctrl_pdata->panel_data.panel_info.mipi.frame_rate = new_fps;

}

static void __mdss_dsi_dyn_refresh_config(
		struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{
	int reg_data;

	reg_data = MIPI_INP((ctrl_pdata->ctrl_base) + DSI_DYNAMIC_REFRESH_CTRL);
	reg_data &= ~BIT(12);

	pr_debug("Dynamic fps ctrl = 0x%x\n", reg_data);
	MIPI_OUTP((ctrl_pdata->ctrl_base) + DSI_DYNAMIC_REFRESH_CTRL, reg_data);
}

static void __mdss_dsi_calc_dfps_delay(struct mdss_panel_data *pdata)
{
	u32 esc_clk_rate = XO_CLK_RATE;
	u32 pipe_delay, pipe_delay2 = 0, pll_delay;
	u32 hsync_period = 0;
	u32 pclk_to_esc_ratio, byte_to_esc_ratio, hr_bit_to_esc_ratio;
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	struct mdss_panel_info *pinfo = NULL;
	struct mdss_dsi_phy_ctrl *pd = NULL;

	if (pdata == NULL) {
		pr_err("%s Invalid pdata\n", __func__);
		return;
	}

	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
			panel_data);

	pinfo = &pdata->panel_info;
	pd = &(pinfo->mipi.dsi_phy_db);

	pclk_to_esc_ratio = (ctrl_pdata->pclk_rate / esc_clk_rate);
	byte_to_esc_ratio = (ctrl_pdata->byte_clk_rate / esc_clk_rate);
	hr_bit_to_esc_ratio = ((ctrl_pdata->byte_clk_rate * 4) / esc_clk_rate);

	hsync_period = mdss_panel_get_htotal(pinfo, true);
	pipe_delay = (hsync_period + 1) / pclk_to_esc_ratio;
	if (pinfo->mipi.eof_bllp_power_stop == 0)
		pipe_delay += (17 / pclk_to_esc_ratio) +
			((21 + (pinfo->mipi.t_clk_pre + 1) +
				(pinfo->mipi.t_clk_post + 1)) /
				byte_to_esc_ratio) +
			((((pd->timing[8] >> 1) + 1) +
			((pd->timing[6] >> 1) + 1) +
			((pd->timing[3] * 4) + (pd->timing[5] >> 1) + 1) +
			((pd->timing[7] >> 1) + 1) +
			((pd->timing[1] >> 1) + 1) +
			((pd->timing[4] >> 1) + 1)) / hr_bit_to_esc_ratio);

	if (pinfo->mipi.force_clk_lane_hs)
		pipe_delay2 = (6 / byte_to_esc_ratio) +
			((((pd->timing[1] >> 1) + 1) +
			((pd->timing[4] >> 1) + 1)) / hr_bit_to_esc_ratio);

	/* 130 us pll delay recommended by h/w doc */
	pll_delay = ((130 * esc_clk_rate) / 1000000) * 2;

	MIPI_OUTP((ctrl_pdata->ctrl_base) + DSI_DYNAMIC_REFRESH_PIPE_DELAY,
						pipe_delay);
	MIPI_OUTP((ctrl_pdata->ctrl_base) + DSI_DYNAMIC_REFRESH_PIPE_DELAY2,
						pipe_delay2);
	MIPI_OUTP((ctrl_pdata->ctrl_base) + DSI_DYNAMIC_REFRESH_PLL_DELAY,
						pll_delay);
}

static int __mdss_dsi_dfps_update_clks(struct mdss_panel_data *pdata,
		int new_fps)
{
	int rc = 0;
	u32 data;
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;

	if (pdata == NULL) {
		pr_err("%s Invalid pdata\n", __func__);
		return -EINVAL;
	}

	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
			panel_data);
	if (ctrl_pdata == NULL) {
		pr_err("%s Invalid ctrl_pdata\n", __func__);
		return -EINVAL;
	}

	rc = mdss_dsi_clk_div_config
		(&ctrl_pdata->panel_data.panel_info, new_fps);
	if (rc) {
		pr_err("%s: unable to initialize the clk dividers\n",
				__func__);
		return rc;
	}

	if (pdata->panel_info.dfps_update
			== DFPS_IMMEDIATE_CLK_UPDATE_MODE) {
		__mdss_dsi_dyn_refresh_config(ctrl_pdata);
		__mdss_dsi_calc_dfps_delay(pdata);
		ctrl_pdata->pclk_rate =
			pdata->panel_info.mipi.dsi_pclk_rate;
		ctrl_pdata->byte_clk_rate =
			pdata->panel_info.clk_rate / 8;

		pr_debug("byte_rate=%i\n", ctrl_pdata->byte_clk_rate);
		pr_debug("pclk_rate=%i\n", ctrl_pdata->pclk_rate);

		if (mdss_dsi_is_ctrl_clk_slave(ctrl_pdata)) {
			pr_debug("%s DFPS already updated.\n", __func__);
			ctrl_pdata->panel_data.panel_info.mipi.frame_rate =
				new_fps;
			return rc;
		}

		/* add an extra reference to main clks */
		clk_prepare_enable(ctrl_pdata->pll_byte_clk);
		clk_prepare_enable(ctrl_pdata->pll_pixel_clk);

		/* change the parent to shadow clocks*/
		clk_set_parent(ctrl_pdata->mux_byte_clk,
				ctrl_pdata->shadow_byte_clk);
		clk_set_parent(ctrl_pdata->mux_pixel_clk,
				ctrl_pdata->shadow_pixel_clk);

		rc =  clk_set_rate(ctrl_pdata->byte_clk,
					ctrl_pdata->byte_clk_rate);
		if (rc) {
			pr_err("%s: dsi_byte_clk - clk_set_rate failed\n",
					__func__);
			return rc;
		}

		rc = clk_set_rate(ctrl_pdata->pixel_clk, ctrl_pdata->pclk_rate);
		if (rc) {
			pr_err("%s: dsi_pixel_clk - clk_set_rate failed\n",
				__func__);
			return rc;
		}

		rc = mdss_dsi_en_wait4dynamic_done(ctrl_pdata);
		MIPI_OUTP((ctrl_pdata->ctrl_base) + DSI_DYNAMIC_REFRESH_CTRL,
							0x00);

		data = MIPI_INP((ctrl_pdata->ctrl_base) + 0x0120);
		MIPI_OUTP((ctrl_pdata->ctrl_base) + 0x120, data);
		pr_debug("pll unlock: 0x%x\n", data);
		clk_set_parent(ctrl_pdata->mux_byte_clk,
				ctrl_pdata->pll_byte_clk);
		clk_set_parent(ctrl_pdata->mux_pixel_clk,
				ctrl_pdata->pll_pixel_clk);
		clk_disable_unprepare(ctrl_pdata->pll_byte_clk);
		clk_disable_unprepare(ctrl_pdata->pll_pixel_clk);

		if (!rc)
			ctrl_pdata->panel_data.panel_info.mipi.frame_rate =
				new_fps;
	} else {
		ctrl_pdata->pclk_rate =
			pdata->panel_info.mipi.dsi_pclk_rate;
		ctrl_pdata->byte_clk_rate =
			pdata->panel_info.clk_rate / 8;
	}

	return rc;
}

static int mdss_dsi_dfps_config(struct mdss_panel_data *pdata, int new_fps)
{
	int rc = 0;
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	struct mdss_dsi_ctrl_pdata *sctrl_pdata = NULL;
	struct mdss_panel_info *pinfo;

	pr_debug("%s+:\n", __func__);

	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
			panel_data);

	if (!ctrl_pdata->panel_data.panel_info.dynamic_fps) {
		pr_err("%s: Dynamic fps not enabled for this panel\n",
					__func__);
		return -EINVAL;
	}

	/*
	 * at split display case, DFPS registers were already programmed
	 * while programming the left ctrl(DSI0). Ignore right ctrl (DSI1)
	 * reguest.
	 */
	pinfo = &pdata->panel_info;
	if (mdss_dsi_is_hw_config_split(ctrl_pdata->shared_data)) {
		if (mdss_dsi_is_right_ctrl(ctrl_pdata)) {
			pr_debug("%s DFPS already updated.\n", __func__);
			return rc;
		}
		/* left ctrl to get right ctrl */
		sctrl_pdata = mdss_dsi_get_other_ctrl(ctrl_pdata);
	}

	ctrl_pdata->dfps_status = true;
	if (sctrl_pdata)
		sctrl_pdata->dfps_status = true;

	if (new_fps !=
		ctrl_pdata->panel_data.panel_info.mipi.frame_rate) {
		if (pdata->panel_info.dfps_update
			== DFPS_IMMEDIATE_PORCH_UPDATE_MODE_HFP ||
			pdata->panel_info.dfps_update
			== DFPS_IMMEDIATE_PORCH_UPDATE_MODE_VFP) {

			__mdss_dsi_update_video_mode_total(pdata, new_fps);
			if (sctrl_pdata) {
				pr_debug("%s Updating slave ctrl DFPS\n",
						__func__);
				__mdss_dsi_update_video_mode_total(
						&sctrl_pdata->panel_data,
						new_fps);
			}

		} else {
			rc = __mdss_dsi_dfps_update_clks(pdata, new_fps);
			if (!rc && sctrl_pdata) {
				pr_debug("%s Updating slave ctrl DFPS\n",
						__func__);
				rc = __mdss_dsi_dfps_update_clks(
						&sctrl_pdata->panel_data,
						new_fps);
			}
		}
	} else {
		pr_debug("%s: Panel is already at this FPS\n", __func__);
	}

	return rc;
}

static int mdss_dsi_ctl_partial_roi(struct mdss_panel_data *pdata)
{
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	int rc = -EINVAL;

	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	if (!pdata->panel_info.partial_update_enabled)
		return 0;

	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);

	if (ctrl_pdata->set_col_page_addr)
		rc = ctrl_pdata->set_col_page_addr(pdata, false);

	return rc;
}

static int mdss_dsi_set_stream_size(struct mdss_panel_data *pdata)
{
	u32 stream_ctrl, stream_total, idle;
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	struct mdss_panel_info *pinfo;
	struct dsc_desc *dsc = NULL;
	struct mdss_rect *roi;
	struct panel_horizontal_idle *pidle;
	int i;

	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);

	pinfo = &pdata->panel_info;

	if (!pinfo->partial_update_supported)
		return -EINVAL;

	if (pinfo->compression_mode == COMPRESSION_DSC)
		dsc = &pinfo->dsc;

	roi = &pinfo->roi;

	if (dsc) {
		stream_ctrl = ((dsc->bytes_in_slice + 1) << 16) |
			(pdata->panel_info.mipi.vc << 8) | DTYPE_DCS_LWRITE;
		stream_total = roi->h << 16 | dsc->pclk_per_line;
	} else  {

		stream_ctrl = (((roi->w * 3) + 1) << 16) |
			(pdata->panel_info.mipi.vc << 8) | DTYPE_DCS_LWRITE;
		stream_total = roi->h << 16 | roi->w;
	}

	/* DSI_COMMAND_MODE_MDP_STREAM_CTRL */
	MIPI_OUTP((ctrl_pdata->ctrl_base) + 0x60, stream_ctrl);
	MIPI_OUTP((ctrl_pdata->ctrl_base) + 0x58, stream_ctrl);

	/* DSI_COMMAND_MODE_MDP_STREAM_TOTAL */
	MIPI_OUTP((ctrl_pdata->ctrl_base) + 0x64, stream_total);
	MIPI_OUTP((ctrl_pdata->ctrl_base) + 0x5C, stream_total);

	/* set idle control -- dsi clk cycle */
	idle = 0;
	pidle = ctrl_pdata->line_idle;
	for (i = 0; i < ctrl_pdata->horizontal_idle_cnt; i++) {
		if (roi->w > pidle->min && roi->w <= pidle->max) {
			idle = pidle->idle;
			pr_debug("%s: ndx=%d w=%d range=%d-%d idle=%d\n",
				__func__, ctrl_pdata->ndx, roi->w,
				pidle->min, pidle->max, pidle->idle);
			break;
		}
		pidle++;
	}

	if (idle)
		idle |= BIT(12);	/* enable */

	MIPI_OUTP((ctrl_pdata->ctrl_base) + 0x194, idle);

	return 0;
}

static int mdss_dsi_reset_write_ptr(struct mdss_panel_data *pdata)
{

	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	struct mdss_panel_info *pinfo;
	int rc = 0;

	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
			panel_data);

	pinfo = &ctrl_pdata->panel_data.panel_info;
	mdss_dsi_clk_ctrl(ctrl_pdata, DSI_ALL_CLKS, 1);
	/* Need to reset the DSI core since the pixel stream was stopped. */
	mdss_dsi_sw_reset(ctrl_pdata, true);

	/*
	 * Reset the partial update co-ordinates to the panel height and
	 * width
	 */
	if (pinfo->dcs_cmd_by_left && (ctrl_pdata->ndx == 1))
		goto skip_cmd_send;

	pinfo->roi.x = 0;
	pinfo->roi.y = 0;
	pinfo->roi.w = pinfo->xres;
	if (pinfo->dcs_cmd_by_left)
		pinfo->roi.w = pinfo->xres;
	if (pdata->next)
		pinfo->roi.w += pdata->next->panel_info.xres;
	pinfo->roi.h = pinfo->yres;

	mdss_dsi_set_stream_size(pdata);

	if (ctrl_pdata->set_col_page_addr)
		rc = ctrl_pdata->set_col_page_addr(pdata, true);

skip_cmd_send:
	mdss_dsi_clk_ctrl(ctrl_pdata, DSI_ALL_CLKS, 0);

	pr_debug("%s: DSI%d write ptr reset finished\n", __func__,
			ctrl_pdata->ndx);

	return rc;
}

int mdss_dsi_register_recovery_handler(struct mdss_dsi_ctrl_pdata *ctrl,
	struct mdss_intf_recovery *recovery)
{
	mutex_lock(&ctrl->mutex);
	ctrl->recovery = recovery;
	mutex_unlock(&ctrl->mutex);
	return 0;
}

int mdss_dsi_clk_refresh(struct mdss_panel_data *pdata)
{
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	int rc = 0;

	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
							panel_data);
	rc = mdss_dsi_clk_div_config(&pdata->panel_info,
			pdata->panel_info.mipi.frame_rate);
	if (rc) {
		pr_err("%s: unable to initialize the clk dividers\n",
								__func__);
		return rc;
	}
	ctrl_pdata->refresh_clk_rate = false;
	ctrl_pdata->pclk_rate = pdata->panel_info.mipi.dsi_pclk_rate;
	ctrl_pdata->byte_clk_rate = pdata->panel_info.clk_rate / 8;
	pr_debug("%s ctrl_pdata->byte_clk_rate=%d ctrl_pdata->pclk_rate=%d\n",
		__func__, ctrl_pdata->byte_clk_rate, ctrl_pdata->pclk_rate);
	return rc;
}


static void mdss_dsi_dba_work(struct work_struct *work)
{
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	struct delayed_work *dw = to_delayed_work(work);
	struct mdss_dba_utils_init_data utils_init_data;
	struct mdss_panel_info *pinfo;

	ctrl_pdata = container_of(dw, struct mdss_dsi_ctrl_pdata, dba_work);
	if (!ctrl_pdata) {
		pr_err("%s: invalid ctrl data\n", __func__);
		return;
	}

	pinfo = &ctrl_pdata->panel_data.panel_info;
	if (!pinfo) {
		pr_err("%s: invalid ctrl data\n", __func__);
		return;
	}

	memset(&utils_init_data, 0, sizeof(utils_init_data));

	utils_init_data.chip_name = "adv7533";
	utils_init_data.client_name = "dsi";
	utils_init_data.instance_id = 0;
	utils_init_data.fb_node = ctrl_pdata->fb_node;
	utils_init_data.kobj = ctrl_pdata->kobj;
	utils_init_data.pinfo = pinfo;

	pinfo->dba_data = mdss_dba_utils_init(&utils_init_data);

	if (!IS_ERR_OR_NULL(pinfo->dba_data)) {
		ctrl_pdata->ds_registered = true;
	} else {
		pr_debug("%s: dba device not ready, queue again\n", __func__);
		queue_delayed_work(ctrl_pdata->workq,
				&ctrl_pdata->dba_work, HZ);
	}
}

static int mdss_dsi_check_params(struct mdss_dsi_ctrl_pdata *ctrl, void *arg)
{
	struct mdss_panel_info *reconf_pinfo, *pinfo;
	int rc = 0;

	if (!ctrl || !arg)
		return 0;

	pinfo = &ctrl->panel_data.panel_info;
	if (!pinfo->is_pluggable)
		return 0;

	reconf_pinfo = (struct mdss_panel_info *)arg;

	pr_debug("%s: reconfig xres: %d yres: %d, current xres: %d yres: %d\n",
			__func__, reconf_pinfo->xres, reconf_pinfo->yres,
					pinfo->xres, pinfo->yres);
	if ((reconf_pinfo->xres != pinfo->xres) ||
			(reconf_pinfo->yres != pinfo->yres))
		rc = 1;

	return rc;
}

static int mdss_dsi_event_handler(struct mdss_panel_data *pdata,
				  int event, void *arg)
{
	int rc = 0;
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	struct fb_info *fbi;
	int power_state;
	u32 mode;

	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}
	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);
	pr_debug("%s+: ctrl=%d event=%d\n", __func__, ctrl_pdata->ndx, event);

	MDSS_XLOG(event, arg, ctrl_pdata->ndx, 0x3333);

	switch (event) {
	case MDSS_EVENT_CHECK_PARAMS:
		pr_debug("%s:Entered Case MDSS_EVENT_CHECK_PARAMS\n", __func__);
		if (mdss_dsi_check_params(ctrl_pdata, arg))
			rc = 1;
		ctrl_pdata->refresh_clk_rate = true;
		break;
	case MDSS_EVENT_LINK_READY:
		if (ctrl_pdata->refresh_clk_rate)
			rc = mdss_dsi_clk_refresh(pdata);

		mdss_dsi_get_hw_revision(ctrl_pdata);
		rc = mdss_dsi_on(pdata);
		mdss_dsi_op_mode_config(pdata->panel_info.mipi.mode,
							pdata);
		break;
	case MDSS_EVENT_UNBLANK:
		if (ctrl_pdata->on_cmds.link_state == DSI_LP_MODE)
			rc = mdss_dsi_unblank(pdata);
		break;
	case MDSS_EVENT_POST_PANEL_ON:
		rc = mdss_dsi_post_panel_on(pdata);
		break;
	case MDSS_EVENT_PANEL_ON:
		ctrl_pdata->ctrl_state |= CTRL_STATE_MDP_ACTIVE;
		if (ctrl_pdata->on_cmds.link_state == DSI_HS_MODE)
			rc = mdss_dsi_unblank(pdata);
		pdata->panel_info.esd_rdy = true;
		break;
	case MDSS_EVENT_BLANK:
		power_state = (int) (unsigned long) arg;
		if (ctrl_pdata->off_cmds.link_state == DSI_HS_MODE)
			rc = mdss_dsi_blank(pdata, power_state);
		break;
	case MDSS_EVENT_PANEL_OFF:
		power_state = (int) (unsigned long) arg;
		ctrl_pdata->ctrl_state &= ~CTRL_STATE_MDP_ACTIVE;
		if (ctrl_pdata->off_cmds.link_state == DSI_LP_MODE)
			rc = mdss_dsi_blank(pdata, power_state);
		rc = mdss_dsi_off(pdata, power_state);
		break;
	case MDSS_EVENT_CONT_SPLASH_FINISH:
		if (ctrl_pdata->off_cmds.link_state == DSI_LP_MODE)
			rc = mdss_dsi_blank(pdata, MDSS_PANEL_POWER_OFF);
		ctrl_pdata->ctrl_state &= ~CTRL_STATE_MDP_ACTIVE;
		rc = mdss_dsi_cont_splash_on(pdata);
		break;
	case MDSS_EVENT_PANEL_CLK_CTRL:
		mdss_dsi_clk_req(ctrl_pdata, (int) (unsigned long) arg);
		break;
	case MDSS_EVENT_DSI_CMDLIST_KOFF:
		mdss_dsi_cmdlist_commit(ctrl_pdata, 1);
		break;
	case MDSS_EVENT_PANEL_UPDATE_FPS:
		if (arg != NULL) {
			rc = mdss_dsi_dfps_config(pdata,
					 (int) (unsigned long) arg);
			pr_debug("%s:update fps to = %d\n",
				 __func__, (int) (unsigned long) arg);
		}
		break;
	case MDSS_EVENT_CONT_SPLASH_BEGIN:
		if (ctrl_pdata->off_cmds.link_state == DSI_HS_MODE) {
			/* Panel is Enabled in Bootloader */
			rc = mdss_dsi_blank(pdata, MDSS_PANEL_POWER_OFF);
		}
		break;
	case MDSS_EVENT_ENABLE_PARTIAL_ROI:
		rc = mdss_dsi_ctl_partial_roi(pdata);
		break;
	case MDSS_EVENT_DSI_RESET_WRITE_PTR:
		rc = mdss_dsi_reset_write_ptr(pdata);
		break;
	case MDSS_EVENT_DSI_STREAM_SIZE:
		rc = mdss_dsi_set_stream_size(pdata);
		break;
	case MDSS_EVENT_DSI_UPDATE_PANEL_DATA:
		rc = mdss_dsi_update_panel_config(ctrl_pdata,
					(int)(unsigned long) arg);
		break;
	case MDSS_EVENT_REGISTER_RECOVERY_HANDLER:
		rc = mdss_dsi_register_recovery_handler(ctrl_pdata,
			(struct mdss_intf_recovery *)arg);
		break;
	case MDSS_EVENT_DSI_DYNAMIC_SWITCH:
		mode = (u32)(unsigned long) arg;
		mdss_dsi_switch_mode(pdata, mode);
		break;
	case MDSS_EVENT_DSI_RECONFIG_CMD:
		mode = (u32)(unsigned long) arg;
		rc = mdss_dsi_reconfig(pdata, mode);
		break;
	case MDSS_EVENT_DSI_PANEL_STATUS:
		if (ctrl_pdata->check_status)
			rc = ctrl_pdata->check_status(ctrl_pdata);
		break;
	case MDSS_EVENT_FB_REGISTERED:
		mdss_dsi_debugfs_init(ctrl_pdata);

		fbi = (struct fb_info *)arg;
		if (!fbi || !fbi->dev)
			break;

		ctrl_pdata->kobj = &fbi->dev->kobj;
		ctrl_pdata->fb_node = fbi->node;

		if (IS_ENABLED(CONFIG_MSM_DBA) &&
			pdata->panel_info.is_dba_panel) {
				queue_delayed_work(ctrl_pdata->workq,
					&ctrl_pdata->dba_work, HZ);
		}
		break;
	case MDSS_EVENT_PANEL_TIMING_SWITCH:
		rc = mdss_dsi_panel_timing_switch(ctrl_pdata, arg);
		break;
#ifdef CONFIG_FB_MSM_MDSS_SPECIFIC_PANEL
	case MDSS_EVENT_DISP_ON:
		if (ctrl_pdata->spec_pdata->disp_on)
			ctrl_pdata->spec_pdata->disp_on(pdata);
		break;
#endif	/* CONFIG_FB_MSM_MDSS_SPECIFIC_PANEL */
	default:
		pr_debug("%s: unhandled event=%d\n", __func__, event);
		break;
	}
	pr_debug("%s-:event=%d, rc=%d\n", __func__, event, rc);
	return rc;
}

static int mdss_dsi_set_override_cfg(char *override_cfg,
		struct mdss_dsi_ctrl_pdata *ctrl_pdata, char *panel_cfg)
{
	struct mdss_panel_info *pinfo = &ctrl_pdata->panel_data.panel_info;
	char *token = NULL;

	pr_debug("%s: override config:%s\n", __func__, override_cfg);
	while ((token = strsep(&override_cfg, ":"))) {
		if (!strcmp(token, OVERRIDE_CFG)) {
			continue;
		} else if (!strcmp(token, SIM_HW_TE_PANEL)) {
			pinfo->sim_panel_mode = SIM_HW_TE_MODE;
		} else if (!strcmp(token, SIM_SW_TE_PANEL)) {
			pinfo->sim_panel_mode = SIM_SW_TE_MODE;
		} else if (!strcmp(token, SIM_PANEL)) {
			pinfo->sim_panel_mode = SIM_MODE;
		} else {
			pr_err("%s: invalid override_cfg token: %s\n",
					__func__, token);
			return -EINVAL;
		}
	}
	pr_debug("%s:sim_panel_mode:%d\n", __func__, pinfo->sim_panel_mode);

	return 0;
}

static struct device_node *mdss_dsi_pref_prim_panel(
		struct platform_device *pdev)
{
	struct device_node *dsi_pan_node = NULL;

	pr_debug("%s:%d: Select primary panel from dt\n",
					__func__, __LINE__);
	dsi_pan_node = of_parse_phandle(pdev->dev.of_node,
					"qcom,dsi-pref-prim-pan", 0);
	if (!dsi_pan_node)
		pr_err("%s:can't find panel phandle\n", __func__);

	return dsi_pan_node;
}

/**
 * mdss_dsi_find_panel_of_node(): find device node of dsi panel
 * @pdev: platform_device of the dsi ctrl node
 * @panel_cfg: string containing intf specific config data
 *
 * Function finds the panel device node using the interface
 * specific configuration data. This configuration data is
 * could be derived from the result of bootloader's GCDB
 * panel detection mechanism. If such config data doesn't
 * exist then this panel returns the default panel configured
 * in the device tree.
 *
 * returns pointer to panel node on success, NULL on error.
 */
static struct device_node *mdss_dsi_find_panel_of_node(
		struct platform_device *pdev, char *panel_cfg)
{
	int len, i;
	int ctrl_id = pdev->id - 1;
	char panel_name[MDSS_MAX_PANEL_LEN];
	char ctrl_id_stream[3] =  "0:";
	char *stream = NULL, *pan = NULL,  *override_cfg = NULL;
	struct device_node *dsi_pan_node = NULL, *mdss_node = NULL;
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = platform_get_drvdata(pdev);
	struct mdss_panel_info *pinfo = &ctrl_pdata->panel_data.panel_info;

	len = strlen(panel_cfg);
	if (!len) {
		/* no panel cfg chg, parse dt */
		pr_debug("%s:%d: no cmd line cfg present\n",
			 __func__, __LINE__);
		goto end;
	} else {
		/* check if any override parameters are set */
		pinfo->sim_panel_mode = 0;
		override_cfg = strnstr(panel_cfg, "#" OVERRIDE_CFG, len);
		if (override_cfg) {
			*override_cfg = '\0';
			if (mdss_dsi_set_override_cfg(override_cfg + 1,
					ctrl_pdata, panel_cfg))
				return NULL;
			len = strlen(panel_cfg);
		}

		if (ctrl_id == 1)
			strlcpy(ctrl_id_stream, "1:", 3);

		stream = strnstr(panel_cfg, ctrl_id_stream, len);
		if (!stream) {
			pr_err("controller config is not present\n");
			goto end;
		}
		stream += 2;

		pan = strnchr(stream, strlen(stream), ':');
		if (!pan) {
			strlcpy(panel_name, stream, MDSS_MAX_PANEL_LEN);
		} else {
			for (i = 0; (stream + i) < pan; i++)
				panel_name[i] = *(stream + i);
			panel_name[i] = 0;
		}

		pr_debug("%s:%d:%s:%s\n", __func__, __LINE__,
			 panel_cfg, panel_name);

		mdss_node = of_parse_phandle(pdev->dev.of_node,
					     "qcom,mdss-mdp", 0);

		if (!mdss_node) {
			pr_err("%s: %d: mdss_node null\n",
			       __func__, __LINE__);
			return NULL;
		}
		dsi_pan_node = of_find_node_by_name(mdss_node,
						    panel_name);
		if (!dsi_pan_node) {
			pr_err("%s: invalid pan node, selecting prim panel\n",
			       __func__);
			goto end;
		}
		return dsi_pan_node;
	}
end:
	if (strcmp(panel_name, NONE_PANEL))
		dsi_pan_node = mdss_dsi_pref_prim_panel(pdev);

	return dsi_pan_node;
}

static struct device_node *mdss_dsi_config_panel(struct platform_device *pdev)
{
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = platform_get_drvdata(pdev);
	char panel_cfg[MDSS_MAX_PANEL_LEN];
	struct device_node *dsi_pan_node = NULL;
	int rc = 0;

	if (!ctrl_pdata) {
		pr_err("%s: Unable to get the ctrl_pdata\n", __func__);
		return NULL;
	}

	/* DSI panels can be different between controllers */
	rc = mdss_dsi_get_panel_cfg(panel_cfg, ctrl_pdata);
	if (!rc)
		/* dsi panel cfg not present */
		pr_warn("%s:%d:dsi specific cfg not present\n",
			__func__, __LINE__);

	/* find panel device node */
	dsi_pan_node = mdss_dsi_find_panel_of_node(pdev, panel_cfg);
	if (!dsi_pan_node) {
		pr_err("%s: can't find panel node %s\n", __func__, panel_cfg);
		of_node_put(dsi_pan_node);
		return NULL;
	}

#ifdef CONFIG_FB_MSM_MDSS_SPECIFIC_PANEL
	ctrl_pdata->panel_data.panel_pdev = pdev;
#endif

	rc = mdss_dsi_panel_init(dsi_pan_node, ctrl_pdata);
	if (rc) {
		pr_err("%s: dsi panel init failed\n", __func__);
		of_node_put(dsi_pan_node);
		return NULL;
	}

	return dsi_pan_node;
}

static int mdss_dsi_ctrl_probe(struct platform_device *pdev)
{
	int rc = 0;
	u32 index;
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	struct device_node *dsi_pan_node = NULL;
	const char *ctrl_name;
	struct mdss_util_intf *util;

	if (!pdev || !pdev->dev.of_node) {
		pr_err("%s: pdev not found for DSI controller\n", __func__);
		return -ENODEV;
	}
	rc = of_property_read_u32(pdev->dev.of_node,
				  "cell-index", &index);
	if (rc) {
		dev_err(&pdev->dev, "%s: Cell-index not specified, rc=%d\n",
			__func__, rc);
		return rc;
	}

	if (index == 0)
		pdev->id = 1;
	else
		pdev->id = 2;

	ctrl_pdata = mdss_dsi_get_ctrl(index);
	if (!ctrl_pdata) {
		pr_err("%s: Unable to get the ctrl_pdata\n", __func__);
		return -EINVAL;
	}

	platform_set_drvdata(pdev, ctrl_pdata);

	util = mdss_get_util_intf();
	if (util == NULL) {
		pr_err("Failed to get mdss utility functions\n");
		return -ENODEV;
	}

	ctrl_pdata->mdss_util = util;
	atomic_set(&ctrl_pdata->te_irq_ready, 0);

#ifdef CONFIG_FB_MSM_MDSS_SPECIFIC_PANEL
	ctrl_pdata->spec_pdata = devm_kzalloc(&pdev->dev,
		sizeof(struct mdss_panel_specific_pdata),
		GFP_KERNEL);
	if (!ctrl_pdata->spec_pdata) {
		pr_err("%s: FAILED: cannot allocate spec_pdata\n",
			__func__);
		devm_kfree(&pdev->dev, ctrl_pdata->spec_pdata);
		return -ENOMEM;
	};
#endif

	ctrl_name = of_get_property(pdev->dev.of_node, "label", NULL);
	if (!ctrl_name)
		pr_info("%s:%d, DSI Ctrl name not specified\n",
			__func__, __LINE__);
	else
		pr_info("%s: DSI Ctrl name = %s\n",
			__func__, ctrl_name);

	rc = mdss_dsi_pinctrl_init(pdev);
	if (rc)
		pr_warn("%s: failed to get pin resources\n", __func__);

	if (index == 0)
		ctrl_pdata->panel_data.panel_info.pdest = DISPLAY_1;
	else
		ctrl_pdata->panel_data.panel_info.pdest = DISPLAY_2;

	dsi_pan_node = mdss_dsi_config_panel(pdev);
	if (!dsi_pan_node) {
		pr_err("%s: panel configuration failed\n", __func__);
		return -EINVAL;
	}

	if (!mdss_dsi_is_hw_config_split(ctrl_pdata->shared_data) ||
		(mdss_dsi_is_hw_config_split(ctrl_pdata->shared_data) &&
		(ctrl_pdata->panel_data.panel_info.pdest == DISPLAY_1))) {
		rc = mdss_panel_parse_bl_settings(dsi_pan_node, ctrl_pdata);
		if (rc) {
			pr_warn("%s: dsi bl settings parse failed\n", __func__);
			/* Panels like AMOLED and dsi2hdmi chip
			 * does not need backlight control.
			 * So we should not fail probe here.
			 */
			ctrl_pdata->bklt_ctrl = UNKNOWN_CTRL;
		}
	} else {
		ctrl_pdata->bklt_ctrl = UNKNOWN_CTRL;
	}

	rc = dsi_panel_device_register(pdev, dsi_pan_node, ctrl_pdata);
	if (rc) {
		pr_err("%s: dsi panel dev reg failed\n", __func__);
		goto error_pan_node;
	}

	if (mdss_dsi_is_te_based_esd(ctrl_pdata)) {
		rc = devm_request_irq(&pdev->dev,
			gpio_to_irq(ctrl_pdata->disp_te_gpio),
			hw_vsync_handler, IRQF_TRIGGER_FALLING,
			"VSYNC_GPIO", ctrl_pdata);
		if (rc) {
			pr_err("TE request_irq failed.\n");
			goto error_pan_node;
		}
		disable_irq(gpio_to_irq(ctrl_pdata->disp_te_gpio));
	}

	ctrl_pdata->workq = create_workqueue("mdss_dsi_dba");
	if (!ctrl_pdata->workq) {
		pr_err("%s: Error creating workqueue\n", __func__);
		rc = -EPERM;
		goto error_pan_node;
	}

	INIT_DELAYED_WORK(&ctrl_pdata->dba_work, mdss_dsi_dba_work);

	mdss_dsi_pm_qos_add_request(ctrl_pdata);

#ifdef CONFIG_FB_MSM_MDSS_SPECIFIC_PANEL
	if (ctrl_pdata->panel_data.panel_info.cont_splash_enabled &&
		ctrl_pdata->spec_pdata->pcc_data.pcc_sts & PCC_STS_UD) {
		ctrl_pdata->spec_pdata->pcc_setup(&ctrl_pdata->panel_data);
		ctrl_pdata->spec_pdata->pcc_data.pcc_sts &= ~PCC_STS_UD;
	}
#endif /* CONFIG_FB_MSM_MDSS_SPECIFIC_PANEL */

	pr_debug("%s: Dsi Ctrl->%d initialized\n", __func__, index);

	if (index == 0)
		ctrl_pdata->shared_data->dsi0_active = true;
	else
		ctrl_pdata->shared_data->dsi1_active = true;

	return 0;

error_pan_node:
	mdss_dsi_unregister_bl_settings(ctrl_pdata);
	of_node_put(dsi_pan_node);
	return rc;
}

static int mdss_dsi_parse_dt_params(struct platform_device *pdev,
		struct dsi_shared_data *sdata)
{
	int rc = 0;

	rc = of_property_read_u32(pdev->dev.of_node,
		"qcom,mmss-ulp-clamp-ctrl-offset",
		&sdata->ulps_clamp_ctrl_off);
	if (!rc) {
		rc = of_property_read_u32(pdev->dev.of_node,
			"qcom,mmss-phyreset-ctrl-offset",
			&sdata->ulps_phyrst_ctrl_off);
	}

	sdata->timing_db_mode = of_property_read_bool(
		pdev->dev.of_node, "qcom,timing-db-mode");

	sdata->cmd_clk_ln_recovery_en =
		of_property_read_bool(pdev->dev.of_node,
		"qcom,dsi-clk-ln-recovery");

	return 0;
}

static void mdss_dsi_res_deinit(struct platform_device *pdev)
{
	int i;
	struct mdss_dsi_data *dsi_res = platform_get_drvdata(pdev);
	struct dsi_shared_data *sdata;

	if (!dsi_res) {
		pr_err("%s: DSI root device drvdata not found\n", __func__);
		return;
	}

	for (i = 0; i < DSI_CTRL_MAX; i++) {
		if (dsi_res->ctrl_pdata[i]) {
			if (dsi_res->ctrl_pdata[i]->ds_registered) {
				struct mdss_panel_info *pinfo =
					&dsi_res->ctrl_pdata[i]->
						panel_data.panel_info;

				if (pinfo)
					mdss_dba_utils_deinit(pinfo->dba_data);
			}

			devm_kfree(&pdev->dev, dsi_res->ctrl_pdata[i]);
		}
	}

	sdata = dsi_res->shared_data;
	if (!sdata)
		goto res_release;

	for (i = (DSI_MAX_PM - 1); i >= DSI_CORE_PM; i--) {
		if (msm_dss_config_vreg(&pdev->dev,
				sdata->power_data[i].vreg_config,
				sdata->power_data[i].num_vreg, 1) < 0)
			pr_err("%s: failed to de-init vregs for %s\n",
				__func__, __mdss_dsi_pm_name(i));
		mdss_dsi_put_dt_vreg_data(&pdev->dev,
			&sdata->power_data[i]);
	}

	mdss_dsi_core_clk_deinit(&pdev->dev, sdata);

	if (sdata)
		devm_kfree(&pdev->dev, sdata);

res_release:
	if (dsi_res)
		devm_kfree(&pdev->dev, dsi_res);

	return;
}

static int mdss_dsi_res_init(struct platform_device *pdev)
{
	int rc = 0, i;
	struct dsi_shared_data *sdata;

	mdss_dsi_res = platform_get_drvdata(pdev);
	if (!mdss_dsi_res) {
		mdss_dsi_res = devm_kzalloc(&pdev->dev,
					  sizeof(struct mdss_dsi_data),
					  GFP_KERNEL);
		if (!mdss_dsi_res) {
			pr_err("%s: FAILED: cannot alloc dsi data\n",
			       __func__);
			rc = -ENOMEM;
			goto mem_fail;
		}

		mdss_dsi_res->shared_data = devm_kzalloc(&pdev->dev,
				sizeof(struct dsi_shared_data),
				GFP_KERNEL);
		pr_debug("%s Allocated shared_data=%p\n", __func__,
				mdss_dsi_res->shared_data);
		if (!mdss_dsi_res->shared_data) {
			pr_err("%s Unable to alloc mem for shared_data\n",
					__func__);
			rc = -ENOMEM;
			goto mem_fail;
		}

		sdata = mdss_dsi_res->shared_data;

		rc = mdss_dsi_parse_dt_params(pdev, sdata);
		if (rc) {
			pr_err("%s: failed to parse mdss dsi DT params\n",
				__func__);
			goto mem_fail;
		}

		rc = mdss_dsi_core_clk_init(pdev, sdata);
		if (rc) {
			pr_err("%s: failed to initialize DSI core clocks\n",
				__func__);
			goto mem_fail;
		}

		/* Parse the regulator information */
		for (i = DSI_CORE_PM; i < DSI_MAX_PM; i++) {
			rc = mdss_dsi_get_dt_vreg_data(&pdev->dev,
				pdev->dev.of_node, &sdata->power_data[i], i);
			if (rc) {
				pr_err("%s: '%s' get_dt_vreg_data failed.rc=%d\n",
					__func__, __mdss_dsi_pm_name(i), rc);
				i--;
				for (; i >= DSI_CORE_PM; i--)
					mdss_dsi_put_dt_vreg_data(&pdev->dev,
						&sdata->power_data[i]);
				goto mem_fail;
			}
		}
		rc = mdss_dsi_regulator_init(pdev, sdata);
		if (rc) {
			pr_err("%s: failed to init regulator, rc=%d\n",
							__func__, rc);
			goto mem_fail;
		}

		mutex_init(&sdata->phy_reg_lock);
		mutex_init(&sdata->pm_qos_lock);

		for (i = 0; i < DSI_CTRL_MAX; i++) {
			mdss_dsi_res->ctrl_pdata[i] = devm_kzalloc(&pdev->dev,
					sizeof(struct mdss_dsi_ctrl_pdata),
					GFP_KERNEL);
			if (!mdss_dsi_res->ctrl_pdata[i]) {
				pr_err("%s Unable to alloc mem for ctrl=%d\n",
						__func__, i);
				rc = -ENOMEM;
				goto mem_fail;
			}
			pr_debug("%s Allocated ctrl_pdata[%d]=%p\n",
				__func__, i, mdss_dsi_res->ctrl_pdata[i]);
			mdss_dsi_res->ctrl_pdata[i]->shared_data =
				mdss_dsi_res->shared_data;
		}

		platform_set_drvdata(pdev, mdss_dsi_res);
	}

	mdss_dsi_res->pdev = pdev;
	pr_debug("%s: Setting up mdss_dsi_res=%p\n", __func__, mdss_dsi_res);

	return 0;

mem_fail:
	mdss_dsi_res_deinit(pdev);
	return rc;
}

static int mdss_dsi_parse_hw_cfg(struct platform_device *pdev, char *pan_cfg)
{
	const char *data;
	struct mdss_dsi_data *dsi_res = platform_get_drvdata(pdev);
	struct dsi_shared_data *sdata;
	char dsi_cfg[20];
	char *cfg_prim = NULL, *cfg_sec = NULL;
	int i = 0;

	if (!dsi_res) {
		pr_err("%s: DSI root device drvdata not found\n", __func__);
		return -EINVAL;
	}

	sdata = mdss_dsi_res->shared_data;
	if (!sdata) {
		pr_err("%s: DSI shared data not found\n", __func__);
		return -EINVAL;
	}

	sdata->hw_config = SINGLE_DSI;

	if (pan_cfg)
		cfg_prim = strnstr(pan_cfg, "cfg:", strlen(pan_cfg));
	if (cfg_prim) {
		cfg_prim += 4;
		cfg_sec = strnchr(cfg_prim, strlen(cfg_prim), ':');
		if (!cfg_sec)
			cfg_sec = cfg_prim + strlen(cfg_prim);
		for (i = 0; (cfg_prim + i) < cfg_sec; i++)
			dsi_cfg[i] = *(cfg_prim + i);
		dsi_cfg[i] = '\0';
		data = dsi_cfg;
	} else {
		data = of_get_property(pdev->dev.of_node,
			"hw-config", NULL);
	}

	if (data) {
		if (!strcmp(data, "dual_dsi"))
			sdata->hw_config = DUAL_DSI;
		else if (!strcmp(data, "split_dsi"))
			sdata->hw_config = SPLIT_DSI;
		else if (!strcmp(data, "single_dsi"))
			sdata->hw_config = SINGLE_DSI;
		else
			pr_err("%s: Incorrect string for DSI config:%s. Setting default as SINGLE_DSI\n",
				__func__, data);
	} else {
		pr_err("%s: Error: No DSI HW config found\n",
			__func__);
		return -EINVAL;
	}

	/*
	 * For certain h/w revisions, use both the DSI PLLs for
	 * split DSI use-cases since it is necessary to do so.
	 */
	if (mdss_dsi_is_hw_config_split(sdata))
		sdata->split_config_independent_pll =
			of_property_read_bool(pdev->dev.of_node,
			"qcom,split-dsi-independent-pll");

	pr_debug("%s: DSI h/w configuration is %d\n", __func__,
		sdata->hw_config);

	return 0;
}

static void mdss_dsi_parse_pll_src_cfg(struct platform_device *pdev,
	char *pan_cfg)
{
	const char *data;
	char *pll_ptr, pll_cfg[10] = {'\0'};
	struct dsi_shared_data *sdata = mdss_dsi_res->shared_data;

	sdata->pll_src_config = PLL_SRC_DEFAULT;

	if (pan_cfg) {
		pll_ptr = strnstr(pan_cfg, ":pll0", strlen(pan_cfg));
		if (!pll_ptr) {
			pll_ptr = strnstr(pan_cfg, ":pll1", strlen(pan_cfg));
			if (pll_ptr)
				strlcpy(pll_cfg, "PLL1", strlen(pll_cfg));
		} else {
			strlcpy(pll_cfg, "PLL0", strlen(pll_cfg));
		}
	}
	data = pll_cfg;

	if (!data || !strcmp(data, ""))
		data = of_get_property(pdev->dev.of_node,
			"pll-src-config", NULL);
	if (data) {
		if (!strcmp(data, "PLL0"))
			sdata->pll_src_config = PLL_SRC_0;
		else if (!strcmp(data, "PLL1"))
			sdata->pll_src_config = PLL_SRC_1;
		else
			pr_err("%s: invalid pll src config %s\n",
				__func__, data);
	} else {
		pr_debug("%s: PLL src config not specified\n", __func__);
	}

	pr_debug("%s: pll_src_config = %d", __func__, sdata->pll_src_config);

	return;
}

static int mdss_dsi_validate_pll_src_config(struct dsi_shared_data *sdata)
{
	int rc = 0;

	/*
	 * DSI PLL1 can only drive DSI PHY1. As such:
	 *     - For split dsi config, only PLL0 is supported
	 *     - For dual dsi config, DSI0-PLL0 and DSI1-PLL1 is the only
	 *       possible configuration
	 */
	if (mdss_dsi_is_hw_config_split(sdata) &&
		mdss_dsi_is_pll_src_pll1(sdata)) {
		pr_err("%s: unsupported PLL config: using PLL1 for split-dsi\n",
			__func__);
		rc = -EINVAL;
		goto error;
	}

	if (mdss_dsi_is_hw_config_dual(sdata) &&
		!mdss_dsi_is_pll_src_default(sdata)) {
		pr_debug("%s: pll src config not applicable for dual-dsi\n",
			__func__);
		sdata->pll_src_config = PLL_SRC_DEFAULT;
	}

error:
	return rc;
}

static int mdss_dsi_validate_config(struct platform_device *pdev)
{
	struct dsi_shared_data *sdata = mdss_dsi_res->shared_data;

	return mdss_dsi_validate_pll_src_config(sdata);
}

static const struct of_device_id mdss_dsi_ctrl_dt_match[] = {
	{.compatible = "qcom,mdss-dsi-ctrl"},
	{}
};
MODULE_DEVICE_TABLE(of, mdss_dsi_ctrl_dt_match);

static int mdss_dsi_probe(struct platform_device *pdev)
{
	struct mdss_panel_cfg *pan_cfg = NULL;
	struct mdss_util_intf *util;
	char *panel_cfg;
	int rc = 0;

	util = mdss_get_util_intf();
	if (util == NULL) {
		pr_err("%s: Failed to get mdss utility functions\n", __func__);
		return -ENODEV;
	}

	if (!util->mdp_probe_done) {
		pr_err("%s: MDP not probed yet!\n", __func__);
		return -EPROBE_DEFER;
	}

	if (!pdev || !pdev->dev.of_node) {
		pr_err("%s: DSI driver only supports device tree probe\n",
			__func__);
		return -ENOTSUPP;
	}

	pan_cfg = util->panel_intf_type(MDSS_PANEL_INTF_HDMI);
	if (IS_ERR(pan_cfg)) {
		return PTR_ERR(pan_cfg);
	} else if (pan_cfg) {
		pr_debug("%s: HDMI is primary\n", __func__);
		return -ENODEV;
	}

	pan_cfg = util->panel_intf_type(MDSS_PANEL_INTF_DSI);
	if (IS_ERR_OR_NULL(pan_cfg)) {
		rc = PTR_ERR(pan_cfg);
		goto error;
	} else {
		panel_cfg = pan_cfg->arg_cfg;
	}

	rc = mdss_dsi_res_init(pdev);
	if (rc) {
		pr_err("%s Unable to set dsi res\n", __func__);
		return rc;
	}

	rc = mdss_dsi_parse_hw_cfg(pdev, panel_cfg);
	if (rc) {
		pr_err("%s Unable to parse dsi h/w config\n", __func__);
		mdss_dsi_res_deinit(pdev);
		return rc;
	}

	mdss_dsi_parse_pll_src_cfg(pdev, panel_cfg);

	of_platform_populate(pdev->dev.of_node, mdss_dsi_ctrl_dt_match,
				NULL, &pdev->dev);

	rc = mdss_dsi_validate_config(pdev);
	if (rc) {
		pr_err("%s: Invalid DSI hw configuration\n", __func__);
		goto error;
	}

	mdss_dsi_config_clk_src(pdev);

error:
	return rc;
}

static int mdss_dsi_remove(struct platform_device *pdev)
{
	mdss_dsi_res_deinit(pdev);
	return 0;
}

static int mdss_dsi_ctrl_remove(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = platform_get_drvdata(pdev);

	if (!ctrl_pdata) {
		pr_err("%s: no driver data\n", __func__);
		return -ENODEV;
	}

	mdss_dsi_pm_qos_remove_request(ctrl_pdata->shared_data);

	if (msm_dss_config_vreg(&pdev->dev,
			ctrl_pdata->panel_power_data.vreg_config,
			ctrl_pdata->panel_power_data.num_vreg, 1) < 0)
		pr_err("%s: failed to de-init vregs for %s\n",
				__func__, __mdss_dsi_pm_name(DSI_PANEL_PM));
	mdss_dsi_put_dt_vreg_data(&pdev->dev, &ctrl_pdata->panel_power_data);

	mfd = platform_get_drvdata(pdev);
	msm_dss_iounmap(&ctrl_pdata->mmss_misc_io);
	msm_dss_iounmap(&ctrl_pdata->phy_io);
	msm_dss_iounmap(&ctrl_pdata->ctrl_io);
	mdss_dsi_debugfs_cleanup(ctrl_pdata);

	if (ctrl_pdata->workq)
		destroy_workqueue(ctrl_pdata->workq);

	return 0;
}

#ifdef CONFIG_FB_MSM_MDSS_SPECIFIC_PANEL
int mdss_dsi_panel_power_detect(struct platform_device *pdev, int enable)
{
#ifdef CONFIG_MACH_SONY_RHINE
	int ret;
	static struct regulator *vdd_vreg;

	pr_debug("%s: enable=%d\n", __func__, enable);
	if (!vdd_vreg) {
		vdd_vreg = devm_regulator_get(&pdev->dev, "vdd");
		if (IS_ERR(vdd_vreg)) {
			pr_err("could not get 8941_lvs3, rc = %ld\n",
					PTR_ERR(vdd_vreg));
			return -ENODEV;
		}
	}

	if (enable) {
		ret = regulator_set_optimum_mode(vdd_vreg, 100000);
		if (ret < 0) {
			pr_err("%s: vdd_vreg set regulator mode failed.\n",
						       __func__);
			return ret;
		}

		ret = regulator_enable(vdd_vreg);
		if (ret) {
			pr_err("%s: Failed to enable regulator.\n", __func__);
			return ret;
		}

		msleep(50);
		wmb();
	} else {
		ret = regulator_disable(vdd_vreg);
		if (ret) {
			pr_err("%s: Failed to disable regulator.\n", __func__);
			return ret;
		}

		ret = regulator_set_optimum_mode(vdd_vreg, 100);
		if (ret < 0) {
			pr_err("%s: vdd_vreg set regulator mode failed.\n",
						       __func__);
			return ret;
		}

		msleep(20);
		devm_regulator_put(vdd_vreg);
	}
#endif /* CONFIG_MACH_SONY_RHINE */
#ifdef CONFIG_MACH_SONY_YUKON
	int ret;
	static struct regulator *vddio_vreg;

	if (!vddio_vreg) {

		vddio_vreg = devm_regulator_get(&pdev->dev, "vddio");
		if (IS_ERR(vddio_vreg)) {
			pr_err("could not get 8941_lvs3, rc = %ld\n",
					PTR_ERR(vddio_vreg));
			return -ENODEV;
		}
	}

	if (enable) {
		ret = regulator_set_optimum_mode(vddio_vreg, 100000);
		if (ret < 0) {
			pr_err("%s: vdd_vreg set regulator mode failed.\n",
						       __func__);
			return ret;
		}

		ret = regulator_enable(vddio_vreg);
		if (ret) {
			pr_err("%s: Failed to enable regulator.\n", __func__);
			return ret;
		}

		msleep(50);
		wmb();
	} else {
		ret = regulator_disable(vddio_vreg);
		if (ret) {
			pr_err("%s: Failed to disable regulator.\n", __func__);
			return ret;
		}

		ret = regulator_set_optimum_mode(vddio_vreg, 100);
		if (ret < 0) {
			pr_err("%s: vdd_vreg set regulator mode failed.\n",
						       __func__);
			return ret;
		}

		usleep_range(9000, 10000);
		devm_regulator_put(vddio_vreg);
	}
#endif
	return 0;
}

static int mdss_dsi_intf_ready(struct mdss_panel_data *pdata)
{
	struct mdss_dsi_ctrl_pdata *ctrl_pdata;

	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);
	if (!ctrl_pdata) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}
	ctrl_pdata->spec_pdata->disp_on(pdata);
	return 0;
}
#endif	/* CONFIG_FB_MSM_MDSS_SPECIFIC_PANEL */

struct device dsi_dev;

int mdss_dsi_retrieve_ctrl_resources(struct platform_device *pdev, int mode,
			struct mdss_dsi_ctrl_pdata *ctrl)
{
	int rc = 0;
	u32 index;

	rc = of_property_read_u32(pdev->dev.of_node, "cell-index", &index);
	if (rc) {
		dev_err(&pdev->dev,
			"%s: Cell-index not specified, rc=%d\n",
						__func__, rc);
		return rc;
	}

	if (index == 0) {
		if (mode != DISPLAY_1) {
			pr_err("%s:%d Panel->Ctrl mapping is wrong\n",
				       __func__, __LINE__);
			return -EPERM;
		}
	} else if (index == 1) {
		if (mode != DISPLAY_2) {
			pr_err("%s:%d Panel->Ctrl mapping is wrong\n",
				       __func__, __LINE__);
			return -EPERM;
		}
	} else {
		pr_err("%s:%d Unknown Ctrl mapped to panel\n",
			       __func__, __LINE__);
		return -EPERM;
	}

	rc = msm_dss_ioremap_byname(pdev, &ctrl->ctrl_io, "dsi_ctrl");
	if (rc) {
		pr_err("%s:%d unable to remap dsi ctrl resources\n",
			       __func__, __LINE__);
		return rc;
	}

	ctrl->ctrl_base = ctrl->ctrl_io.base;
	ctrl->reg_size = ctrl->ctrl_io.len;

	rc = msm_dss_ioremap_byname(pdev, &ctrl->phy_io, "dsi_phy");
	if (rc) {
		pr_err("%s:%d unable to remap dsi phy resources\n",
			       __func__, __LINE__);
		return rc;
	}

	rc = msm_dss_ioremap_byname(pdev, &ctrl->phy_regulator_io,
			"dsi_phy_regulator");
	if (rc) {
		pr_err("%s:%d unable to remap dsi phy regulator resources\n",
			       __func__, __LINE__);
		return rc;
	}

	pr_info("%s: ctrl_base=%p ctrl_size=%x phy_base=%p phy_size=%x\n",
		__func__, ctrl->ctrl_base, ctrl->reg_size, ctrl->phy_io.base,
		ctrl->phy_io.len);
	pr_info("%s: phy_regulator_base=%p phy_regulator_size=%x\n", __func__,
		ctrl->phy_regulator_io.base, ctrl->phy_regulator_io.len);

	rc = msm_dss_ioremap_byname(pdev, &ctrl->mmss_misc_io,
		"mmss_misc_phys");
	if (rc) {
		pr_debug("%s:%d mmss_misc IO remap failed\n",
			__func__, __LINE__);
	}

	return 0;
}

static int mdss_dsi_irq_init(struct device *dev, int irq_no,
			struct mdss_dsi_ctrl_pdata *ctrl)
{
	int ret;

	ret = devm_request_irq(dev, irq_no, mdss_dsi_isr,
				IRQF_DISABLED, "DSI", ctrl);
	if (ret) {
		pr_err("msm_dsi_irq_init request_irq() failed!\n");
		return ret;
	}

	disable_irq(irq_no);
	ctrl->dsi_hw->irq_info = kzalloc(sizeof(struct irq_info), GFP_KERNEL);
	if (!ctrl->dsi_hw->irq_info) {
		pr_err("no mem to save irq info: kzalloc fail\n");
		return -ENOMEM;
	}
	ctrl->dsi_hw->irq_info->irq = irq_no;
	ctrl->dsi_hw->irq_info->irq_ena = false;

	return ret;
}

static void mdss_dsi_parse_lane_swap(struct device_node *np, char *dlane_swap)
{
	const char *data;

	*dlane_swap = DSI_LANE_MAP_0123;
	data = of_get_property(np, "qcom,lane-map", NULL);
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

static int mdss_dsi_parse_ctrl_params(struct platform_device *ctrl_pdev,
	struct device_node *pan_node, struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{
	int i, len;
	struct mdss_panel_info *pinfo = &(ctrl_pdata->panel_data.panel_info);
	const char *data;

	data = of_get_property(ctrl_pdev->dev.of_node,
		"qcom,platform-strength-ctrl", &len);
	if ((!data) || (len != 2)) {
		pr_err("%s:%d, Unable to read Phy Strength ctrl settings\n",
			__func__, __LINE__);
		return -EINVAL;
	}
	pinfo->mipi.dsi_phy_db.strength[0] = data[0];
	pinfo->mipi.dsi_phy_db.strength[1] = data[1];

	pinfo->mipi.dsi_phy_db.reg_ldo_mode = of_property_read_bool(
		ctrl_pdev->dev.of_node, "qcom,regulator-ldo-mode");

	data = of_get_property(ctrl_pdev->dev.of_node,
		"qcom,platform-regulator-settings", &len);
	if ((!data) || (len != 7)) {
#ifndef CONFIG_FB_MSM_MDSS_SPECIFIC_PANEL
		pr_err("%s:%d, Unable to read Phy regulator settings\n",
			__func__, __LINE__);
		return -EINVAL;
#else
		pr_debug("%s:%d, Unable to read Phy regulator settings\n",
			__func__, __LINE__);
#endif	/* CONFIG_FB_MSM_MDSS_SPECIFIC_PANEL */
	}
	for (i = 0; i < len; i++) {
		pinfo->mipi.dsi_phy_db.regulator[i]
			= data[i];
	}

	data = of_get_property(ctrl_pdev->dev.of_node,
		"qcom,platform-bist-ctrl", &len);
	if ((!data) || (len != 6)) {
		pr_err("%s:%d, Unable to read Phy Bist Ctrl settings\n",
			__func__, __LINE__);
		return -EINVAL;
	}
	for (i = 0; i < len; i++) {
		pinfo->mipi.dsi_phy_db.bistctrl[i]
			= data[i];
	}

	data = of_get_property(ctrl_pdev->dev.of_node,
		"qcom,platform-lane-config", &len);
	if ((!data) || (len != 45)) {
		pr_err("%s:%d, Unable to read Phy lane configure settings\n",
			__func__, __LINE__);
#ifndef CONFIG_FB_MSM_MDSS_SPECIFIC_PANEL
		return -EINVAL;
#endif
	}
	for (i = 0; i < len; i++) {
		pinfo->mipi.dsi_phy_db.lanecfg[i] =
			data[i];
	}

	ctrl_pdata->cmd_sync_wait_broadcast = of_property_read_bool(
		pan_node, "qcom,cmd-sync-wait-broadcast");

	if (ctrl_pdata->cmd_sync_wait_broadcast &&
		mdss_dsi_is_hw_config_split(ctrl_pdata->shared_data) &&
		(pinfo->pdest == DISPLAY_2))
		ctrl_pdata->cmd_sync_wait_trigger = true;

	pr_debug("%s: cmd_sync_wait_enable=%d trigger=%d\n", __func__,
				ctrl_pdata->cmd_sync_wait_broadcast,
				ctrl_pdata->cmd_sync_wait_trigger);

	mdss_dsi_parse_lane_swap(ctrl_pdev->dev.of_node,
			&(ctrl_pdata->dlane_swap));

	return 0;


}

static int mdss_dsi_parse_gpio_params(struct platform_device *ctrl_pdev,
	struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{
	struct mdss_panel_info *pinfo = &(ctrl_pdata->panel_data.panel_info);

	/*
	 * If disp_en_gpio has been set previously (disp_en_gpio > 0)
	 *  while parsing the panel node, then do not override it
	 */
	if (ctrl_pdata->disp_en_gpio <= 0) {
		ctrl_pdata->disp_en_gpio = of_get_named_gpio(
			ctrl_pdev->dev.of_node,
			"qcom,platform-enable-gpio", 0);

		if (!gpio_is_valid(ctrl_pdata->disp_en_gpio))
			pr_err("%s:%d, Disp_en gpio not specified\n",
					__func__, __LINE__);
	}

	ctrl_pdata->disp_te_gpio = of_get_named_gpio(ctrl_pdev->dev.of_node,
		"qcom,platform-te-gpio", 0);

	if (!gpio_is_valid(ctrl_pdata->disp_te_gpio))
		pr_err("%s:%d, TE gpio not specified\n",
						__func__, __LINE__);

	ctrl_pdata->bklt_en_gpio = of_get_named_gpio(ctrl_pdev->dev.of_node,
		"qcom,platform-bklight-en-gpio", 0);
	if (!gpio_is_valid(ctrl_pdata->bklt_en_gpio))
		pr_info("%s: bklt_en gpio not specified\n", __func__);

	ctrl_pdata->rst_gpio = of_get_named_gpio(ctrl_pdev->dev.of_node,
			 "qcom,platform-reset-gpio", 0);
	if (!gpio_is_valid(ctrl_pdata->rst_gpio))
		pr_err("%s:%d, reset gpio not specified\n",
						__func__, __LINE__);

	if (pinfo->mode_gpio_state != MODE_GPIO_NOT_VALID) {

		ctrl_pdata->mode_gpio = of_get_named_gpio(
					ctrl_pdev->dev.of_node,
					"qcom,platform-mode-gpio", 0);
		if (!gpio_is_valid(ctrl_pdata->mode_gpio))
			pr_info("%s:%d, mode gpio not specified\n",
							__func__, __LINE__);
	} else {
		ctrl_pdata->mode_gpio = -EINVAL;
	}

	ctrl_pdata->lcd_mode_sel_gpio = of_get_named_gpio(
			ctrl_pdev->dev.of_node, "qcom,panel-mode-gpio", 0);
	if (!gpio_is_valid(ctrl_pdata->lcd_mode_sel_gpio)) {
			pr_info("%s:%d, dsc/dual mode gpio not specified\n",
							__func__, __LINE__);
		ctrl_pdata->lcd_mode_sel_gpio = -EINVAL;
	}

/* WARNING: FIXME!!!! MOVE THIS TO SOMC_PANEL!!!! */
#ifdef CONFIG_FB_MSM_MDSS_SPECIFIC_PANEL
	ctrl_pdata->spec_pdata->disp_dcdc_en_gpio
		= of_get_named_gpio(ctrl_pdev->dev.of_node,
			"somc,platform-disp-dcdc-en-gpio", 0);

	if (!gpio_is_valid(ctrl_pdata->spec_pdata->disp_dcdc_en_gpio))
		pr_err("%s:%d, disp_dcdc_en gpio gpio not specified\n",
						__func__, __LINE__);
#endif

	return 0;
}

int dsi_panel_device_register(struct platform_device *ctrl_pdev,
	struct device_node *pan_node, struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{
	struct mipi_panel_info *mipi;
	int rc, data;
	struct dsi_shared_data *sdata;
	struct mdss_panel_info *pinfo = &(ctrl_pdata->panel_data.panel_info);
	struct resource *res;
	struct device_node *fb_node;
	struct platform_device *dsi_dev;

#ifdef CONFIG_FB_MSM_MDSS_SPECIFIC_PANEL
	struct mdss_panel_specific_pdata *spec_pdata = NULL;

	spec_pdata = ctrl_pdata->spec_pdata;
	if (!spec_pdata) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}
#endif

	mipi  = &(pinfo->mipi);

	pinfo->type =
		((mipi->mode == DSI_VIDEO_MODE)
			? MIPI_VIDEO_PANEL : MIPI_CMD_PANEL);

	rc = mdss_dsi_clk_div_config(pinfo, mipi->frame_rate);
	if (rc) {
		pr_err("%s: unable to initialize the clk dividers\n", __func__);
		return rc;
	}

	rc = mdss_dsi_get_dt_vreg_data(&ctrl_pdev->dev, pan_node,
		&ctrl_pdata->panel_power_data, DSI_PANEL_PM);
	if (rc) {
		DEV_ERR("%s: '%s' get_dt_vreg_data failed.rc=%d\n",
			__func__, __mdss_dsi_pm_name(DSI_PANEL_PM), rc);
		return rc;
	}

	rc = msm_dss_config_vreg(&ctrl_pdev->dev,
		ctrl_pdata->panel_power_data.vreg_config,
		ctrl_pdata->panel_power_data.num_vreg, 1);
	if (rc) {
		pr_err("%s: failed to init regulator, rc=%d\n",
						__func__, rc);
		return rc;
	}

	rc = mdss_dsi_parse_ctrl_params(ctrl_pdev, pan_node, ctrl_pdata);
	if (rc) {
		pr_err("%s: failed to parse ctrl settings, rc=%d\n",
						__func__, rc);
		return rc;
	}

	pinfo->panel_max_fps = mdss_panel_get_framerate(pinfo);
	pinfo->panel_max_vtotal = mdss_panel_get_vtotal(pinfo);

	rc = mdss_dsi_parse_gpio_params(ctrl_pdev, ctrl_pdata);
	if (rc) {
		pr_err("%s: failed to parse gpio params, rc=%d\n",
						__func__, rc);
		return rc;
	}

	if (mdss_dsi_link_clk_init(ctrl_pdev, ctrl_pdata)) {
		pr_err("%s: unable to initialize Dsi ctrl clks\n", __func__);
		return -EPERM;
	}

	if (pinfo->dynamic_fps)
		if (mdss_dsi_shadow_clk_init(ctrl_pdev, ctrl_pdata))
			pr_err("unable to initialize shadow ctrl clks\n");

	if (mdss_dsi_retrieve_ctrl_resources(ctrl_pdev,
					     pinfo->pdest,
					     ctrl_pdata)) {
		pr_err("%s: unable to get Dsi controller res\n", __func__);
		return -EPERM;
	}

	ctrl_pdata->panel_data.event_handler = mdss_dsi_event_handler;
#ifdef CONFIG_FB_MSM_MDSS_SPECIFIC_PANEL
	ctrl_pdata->panel_data.intf_ready = mdss_dsi_intf_ready;
	ctrl_pdata->panel_data.detect = spec_pdata->detect;
	ctrl_pdata->panel_data.update_panel = spec_pdata->update_panel;
	ctrl_pdata->panel_data.panel_pdev = ctrl_pdev;
#endif

	if (ctrl_pdata->status_mode == ESD_REG ||
			ctrl_pdata->status_mode == ESD_REG_NT35596)
		ctrl_pdata->check_status = mdss_dsi_reg_status_check;
	else if (ctrl_pdata->status_mode == ESD_BTA)
		ctrl_pdata->check_status = mdss_dsi_bta_status_check;

	if (ctrl_pdata->status_mode == ESD_MAX) {
		pr_err("%s: Using default BTA for ESD check\n", __func__);
		ctrl_pdata->check_status = mdss_dsi_bta_status_check;
	}
	if (ctrl_pdata->bklt_ctrl == BL_PWM)
		mdss_dsi_panel_pwm_cfg(ctrl_pdata);

	mdss_dsi_ctrl_init(&ctrl_pdev->dev, ctrl_pdata);

	ctrl_pdata->dsi_irq_line = of_property_read_bool(
				ctrl_pdev->dev.of_node, "qcom,dsi-irq-line");

	if (ctrl_pdata->dsi_irq_line) {
		/* DSI has it's own irq line */
		res = platform_get_resource(ctrl_pdev, IORESOURCE_IRQ, 0);
		if (!res || res->start == 0) {
			pr_err("%s:%d unable to get the MDSS irq resources\n",
							__func__, __LINE__);
			return -ENODEV;
		}
		rc = mdss_dsi_irq_init(&ctrl_pdev->dev, res->start, ctrl_pdata);
		if (rc) {
			dev_err(&ctrl_pdev->dev, "%s: failed to init irq\n",
							__func__);
			return rc;
		}
	}

	ctrl_pdata->pclk_rate = mipi->dsi_pclk_rate;
	ctrl_pdata->byte_clk_rate = pinfo->clk_rate / 8;
	pr_debug("%s: pclk=%d, bclk=%d\n", __func__,
			ctrl_pdata->pclk_rate, ctrl_pdata->byte_clk_rate);

	ctrl_pdata->ctrl_state = CTRL_STATE_UNKNOWN;

	/*
	 * If ULPS during suspend is enabled, add an extra vote for the
	 * DSI CTRL power module. This keeps the regulator always enabled.
	 * This is needed for the DSI PHY to maintain ULPS state during
	 * suspend also.
	 */
	sdata = ctrl_pdata->shared_data;

	if (pinfo->ulps_suspend_enabled) {
		rc = msm_dss_enable_vreg(
			sdata->power_data[DSI_PHY_PM].vreg_config,
			sdata->power_data[DSI_PHY_PM].num_vreg, 1);
		if (rc) {
			pr_err("%s: failed to enable vregs for DSI_CTRL_PM\n",
				__func__);
			return rc;
		}
	}

	pinfo->cont_splash_enabled =
		ctrl_pdata->mdss_util->panel_intf_status(pinfo->pdest,
		MDSS_PANEL_INTF_DSI) ? true : false;

	pr_info("%s: Continuous splash %s\n", __func__,
		pinfo->cont_splash_enabled ? "enabled" : "disabled");

	if (pinfo->cont_splash_enabled) {
		rc = mdss_dsi_panel_power_ctrl(&(ctrl_pdata->panel_data),
			MDSS_PANEL_POWER_ON);
		if (rc) {
			pr_err("%s: Panel power on failed\n", __func__);
			return rc;
		}
		if (ctrl_pdata->bklt_ctrl == BL_PWM)
			ctrl_pdata->pwm_enabled = 1;
		ctrl_pdata->ctrl_state |= (CTRL_STATE_PANEL_INIT |
			CTRL_STATE_MDP_ACTIVE | CTRL_STATE_DSI_ACTIVE);
		mdss_dsi_clk_ctrl(ctrl_pdata, DSI_ALL_CLKS, 1);
		ctrl_pdata->is_phyreg_enabled = 1;
		mdss_dsi_get_hw_revision(ctrl_pdata);
		if ((ctrl_pdata->shared_data->hw_rev >= MDSS_DSI_HW_REV_103)
			&& (pinfo->type == MIPI_CMD_PANEL)) {
			data = MIPI_INP(ctrl_pdata->ctrl_base + 0x1b8);
			if (data & BIT(16))
				ctrl_pdata->burst_mode_enabled = true;
		}
	} else {
		pinfo->panel_power_state = MDSS_PANEL_POWER_OFF;
	}

	dsi_dev = of_find_device_by_node(ctrl_pdev->dev.of_node->parent);
	if (!dsi_dev) {
		pr_err("Unable to find dsi master device: %s\n",
			ctrl_pdev->dev.of_node->full_name);
		return -ENODEV;
	}

	fb_node = of_parse_phandle(dsi_dev->dev.of_node,
			__mdss_dsi_get_fb_name(ctrl_pdata), 0);
	if (!fb_node) {
		pr_err("Unable to find fb node for device: %s\n",
			ctrl_pdev->name);
		return -ENODEV;
	}

	rc = mdss_register_panel(ctrl_pdev, &(ctrl_pdata->panel_data), fb_node);
	if (rc) {
		pr_err("%s: unable to register MIPI DSI panel\n", __func__);
		return rc;
	}

	if (pinfo->pdest == DISPLAY_1) {
		mdss_debug_register_io("dsi0_ctrl", &ctrl_pdata->ctrl_io, NULL);
		mdss_debug_register_io("dsi0_phy", &ctrl_pdata->phy_io, NULL);
		mdss_debug_register_io("dsi0_phy_regulator",
			&ctrl_pdata->phy_regulator_io, NULL);
		ctrl_pdata->ndx = 0;
	} else {
		mdss_debug_register_io("dsi1_ctrl", &ctrl_pdata->ctrl_io, NULL);
		mdss_debug_register_io("dsi1_phy", &ctrl_pdata->phy_io, NULL);
		mdss_debug_register_io("dsi1_phy_regulator",
			&ctrl_pdata->phy_regulator_io, NULL);
		ctrl_pdata->ndx = 1;
	}

	panel_debug_register_base("panel",
		ctrl_pdata->ctrl_base, ctrl_pdata->reg_size);

	pr_debug("%s: Panel data initialized\n", __func__);
	return 0;
}

static const struct of_device_id mdss_dsi_dt_match[] = {
	{.compatible = "qcom,mdss-dsi"},
	{}
};
MODULE_DEVICE_TABLE(of, mdss_dsi_dt_match);

static struct platform_driver mdss_dsi_driver = {
	.probe = mdss_dsi_probe,
	.remove = mdss_dsi_remove,
	.shutdown = NULL,
	.driver = {
		.name = "mdss_dsi",
		.of_match_table = mdss_dsi_dt_match,
	},
};

static struct platform_driver mdss_dsi_ctrl_driver = {
	.probe = mdss_dsi_ctrl_probe,
	.remove = mdss_dsi_ctrl_remove,
	.shutdown = NULL,
	.driver = {
		.name = "mdss_dsi_ctrl",
		.of_match_table = mdss_dsi_ctrl_dt_match,
	},
};

static int mdss_dsi_register_driver(void)
{
	return platform_driver_register(&mdss_dsi_driver);
}

static int __init mdss_dsi_driver_init(void)
{
	int ret;

	ret = mdss_dsi_register_driver();
	if (ret) {
		pr_err("mdss_dsi_register_driver() failed!\n");
		return ret;
	}

	return ret;
}
module_init(mdss_dsi_driver_init);


static int mdss_dsi_ctrl_register_driver(void)
{
	return platform_driver_register(&mdss_dsi_ctrl_driver);
}

static int __init mdss_dsi_ctrl_driver_init(void)
{
	int ret;

	ret = mdss_dsi_ctrl_register_driver();
	if (ret) {
		pr_err("mdss_dsi_ctrl_register_driver() failed!\n");
		return ret;
	}

	return ret;
}
module_init(mdss_dsi_ctrl_driver_init);

static void __exit mdss_dsi_driver_cleanup(void)
{
	platform_driver_unregister(&mdss_dsi_ctrl_driver);
}
module_exit(mdss_dsi_driver_cleanup);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("DSI controller driver");
MODULE_AUTHOR("Chandan Uddaraju <chandanu@codeaurora.org>");
