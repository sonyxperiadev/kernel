/* drivers/video/msm/mdss_dsi_panel_debugfs.c
 *
 * Copyright (c) 2012 Sony Mobile Communications Inc.
 *
 * Sony Mobile DSI display driver debug fs
 *
 * Author: Johan Olson <johan.olson@sonymobile.com>
 * Author: Joakim Wesslen <joakim.wesslen@sonymobile.com>
 * Author: Yutaka Seijyou <Yutaka.X.Seijyou@sonymobile.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include <linux/ctype.h>
#include <linux/debugfs.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/sched.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#include "mdss_dsi.h"
#include "mdss_fb.h"
#include "mdss_mdp.h"
#include "mdss_mdp_hwio.h"

static char *res_buf;
static int buf_sz;

static struct dsi_buf debug_tx_buf;
static struct dsi_buf debug_rx_buf;

#define TMP_BUF_SZ 128
#define MAX_WRITE_DATA 100

enum dbg_cmd_type {
	DCS,
	GEN,
};

static void update_res_buf(char *string)
{
	res_buf = krealloc(res_buf, buf_sz + strnlen(string, TMP_BUF_SZ) + 1,
								GFP_KERNEL);
	if (!res_buf) {
		pr_err("%s: Failed to allocate buffer\n", __func__);
		return;
	}

	memcpy(res_buf + buf_sz, string, strnlen(string, TMP_BUF_SZ) + 1);
	buf_sz += strnlen(string, TMP_BUF_SZ); /* Exclude NULL termination */
}

static void reset_res_buf(void)
{
	kfree(res_buf);
	res_buf = NULL;
	buf_sz = 0;
}

static void print_params(int dtype, u8 reg, int len, u8 *data)
{
	int i = 0;
	char tmp[TMP_BUF_SZ];

	switch (dtype) {
	case DTYPE_GEN_WRITE:
		update_res_buf("GEN_WRITE\n");
		break;
	case DTYPE_GEN_WRITE1:
		update_res_buf("GEN_WRITE1\n");
		break;
	case DTYPE_GEN_WRITE2:
		update_res_buf("GEN_WRITE2\n");
		break;
	case DTYPE_GEN_LWRITE:
		update_res_buf("GEN_LWRITE\n");
		break;
	case DTYPE_GEN_READ:
		update_res_buf("GEN_READ\n");
		break;
	case DTYPE_GEN_READ1:
		update_res_buf("GEN_READ1\n");
		break;
	case DTYPE_GEN_READ2:
		update_res_buf("GEN_READ2\n");
		break;
	case DTYPE_DCS_LWRITE:
		update_res_buf("DCS_LWRITE\n");
		break;
	case DTYPE_DCS_WRITE:
		update_res_buf("DCS_WRITE\n");
		break;
	case DTYPE_DCS_WRITE1:
		update_res_buf("DCS_WRITE1\n");
		break;
	case DTYPE_DCS_READ:
		update_res_buf("DCS_READ\n");
		break;
	default:
		snprintf(tmp, sizeof(tmp), "Unknown dtype = 0x%x\n", dtype);
		update_res_buf(tmp);
	}

	if (len > 0) {
		snprintf(tmp, sizeof(tmp), "reg=0x%.2X\n", reg);
		update_res_buf(tmp);
		snprintf(tmp, sizeof(tmp), "len=%d\n", len);
		update_res_buf(tmp);
		for (i = 0; i < len; i++) {
			snprintf(tmp, sizeof(tmp), "data[%d]=0x%.2X\n", i,
								data[i]);
			update_res_buf(tmp);
		}
	} else {
		update_res_buf("Something went wrong, length is zero.\n");
		snprintf(tmp, sizeof(tmp),
				"reg=0x%.2X, len=%d, data[0]=0x%.2X\n",
				reg, len, data[0]);
		update_res_buf(tmp);
	}
}

static int setup_reg_access(char **buf, const char __user *ubuf, size_t count)
{
	int ret = 0;

	reset_res_buf();

	*buf = kzalloc(sizeof(char) * count, GFP_KERNEL);
	if (!*buf) {
		pr_err("%s: Failed to allocate buffer\n", __func__);
		ret = -ENOMEM;
		goto exit;
	}

	if (copy_from_user(*buf, ubuf, count)) {
		ret = -EFAULT;
		goto exit;
	}
	return 0;

exit:
	kfree(*buf);
	return ret;
}

static int get_cmd_type(char *buf, enum dbg_cmd_type *cmd)
{
	int ret = 0;

	if (!strncmp(buf, "dcs", 3))
		*cmd = DCS;
	else if (!strncmp(buf, "gen", 3))
		*cmd = GEN;
	else
		ret = -EFAULT;
	return ret;
}

static int get_parameters(const char *p, u8 *par_buf, int par_buf_size,
								int *nbr_params)
{
	int ret = 0;

	while (true) {
		if (isspace(*p)) {
			p++;
		} else {
			if (sscanf(p, "%4hhx", &par_buf[*nbr_params]) == 1) {
				(*nbr_params)++;
				while (isxdigit(*p) || (*p == 'x'))
					p++;
			}
		}
		if (*nbr_params > par_buf_size) {
			update_res_buf("Too many parameters\n");
			ret = -EINVAL;
			goto exit;
		}
		if (iscntrl(*p))
			break;
	}
exit:
	return ret;
}

static int prepare_for_reg_access(struct msm_fb_data_type *mfd)
{
	struct mdss_panel_data *pdata;
	int ret = 0;
	struct mdss_mdp_ctl *ctl;
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;

	ctl = mfd_to_ctl(mfd);
	if (!ctl)
		return -ENODEV;

	pdata = dev_get_platdata(&mfd->pdev->dev);
	if (!pdata) {
		pr_err("%s: no panel connected\n", __func__);
		return -ENODEV;
	}

	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);

	mdss_dsi_cmd_mdp_busy(ctrl_pdata);
	mdss_bus_bandwidth_ctrl(1);
	mdss_dsi_clk_ctrl(ctrl_pdata, DSI_ALL_CLKS, 1);

	mdss_dsi_op_mode_config(DSI_CMD_MODE, pdata);

	return ret;
}

static int post_reg_access(struct msm_fb_data_type *mfd)
{
	struct mdss_panel_data *pdata;
	int ret = 0;
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;

	pdata = dev_get_platdata(&mfd->pdev->dev);
	if (!pdata) {
		pr_err("%s: no panel connected\n", __func__);
		return -ENODEV;
	}

	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);

	mdss_dsi_clk_ctrl(ctrl_pdata, DSI_ALL_CLKS, 0);
	mdss_bus_bandwidth_ctrl(0);
	mdss_dsi_op_mode_config(pdata->panel_info.mipi.mode, pdata);

	return ret;
}

static ssize_t reg_read(struct file *file, const char __user *ubuf,
						size_t count, loff_t *ppos)
{
	struct seq_file *s = file->private_data;
	struct msm_fb_data_type *mfd = s->private;
	struct mdss_panel_data *pdata;
	struct mdss_mdp_ctl *ctl;
	u8 params[3]; /* No more than reg + two parameters is allowed */
	char *buf;
	const char *p;
	int ret;
	int nbr_bytes_to_read;
	int i;
	int j;
	enum dbg_cmd_type cmd;
	struct dsi_cmd_desc dsi;
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;

	if (!mfd->panel_power_on) {
		pr_err("%s: panel is NOT on\n", __func__);
		goto exit;
	}

	ctl = mfd_to_ctl(mfd);
	if (!ctl)
		goto exit;

	if (mutex_lock_interruptible(&ctl->lock))
		goto exit;

	pdata = dev_get_platdata(&mfd->pdev->dev);
	if (!pdata) {
		pr_err("%s: no panel connected\n", __func__);
		goto exit;
	}

	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);
	if (!ctrl_pdata) {
		pr_err("%s: Invalid input data\n", __func__);
		goto exit;
	}

	pr_err("%s\n", __func__);

	ret = setup_reg_access(&buf, ubuf, count);
	if (ret)
		goto exit;

	ret = get_cmd_type(buf, &cmd);
	if (ret) {
		update_res_buf("Read - unknown type\n");
		goto fail_free_all;
	}

	p = buf;
	p = p+4;

	/* Get nbr_bytes_to_read */
	if (sscanf(p, "%d", &nbr_bytes_to_read) != 1) {
		update_res_buf("Read - parameter error\n");
		ret = -EINVAL;
		goto fail_free_all;
	}

	while (isxdigit(*p) || (*p == 'x'))
		p++;

	pr_err("%s: nbr_bytes_to_read = %d\n", __func__, nbr_bytes_to_read);
	i = 0;

	ret = get_parameters(p, params, ARRAY_SIZE(params), &i);
	if (ret)
		goto fail_free_all;

	ret = prepare_for_reg_access(mfd);
	if (ret)
		goto fail_free_all;

	if (cmd == DCS) {
		dsi.dchdr.dtype = DTYPE_DCS_READ;
	} else {
		if (i == 1) { /* 0 parameters */
			dsi.dchdr.dtype = DTYPE_GEN_READ;
		} else if (i == 2) { /* 1 parameter */
			dsi.dchdr.dtype = DTYPE_GEN_READ1;
		} else { /* 2 paramters */
			dsi.dchdr.dtype = DTYPE_GEN_READ2;
		}
	}
	dsi.dchdr.last = 1;
	dsi.dchdr.vc = 0;
	dsi.dchdr.ack = 1;
	dsi.dchdr.wait = 5;
	dsi.dchdr.dlen = i;
	dsi.payload = params;

	pr_err("%s: dtype=%d, last=%d, vc=%d, ack=%d, wait=%d, dlen=%d\n",
		__func__,
		dsi.dchdr.dtype, dsi.dchdr.last, dsi.dchdr.vc, dsi.dchdr.ack,
		dsi.dchdr.wait, dsi.dchdr.dlen);
	for (j = 0; j < i; j++)
		pr_err("%s: payload[%d] = 0x%x\n",
			__func__, j, dsi.payload[j]);

	mdss_dsi_cmds_rx(ctrl_pdata, &dsi, nbr_bytes_to_read);

	ret = post_reg_access(mfd);
	if (ret)
		goto fail_free_all;

	print_params(dsi.dchdr.dtype, params[0], ctrl_pdata->rx_buf.len,
			ctrl_pdata->rx_buf.data);

	mutex_unlock(&ctl->lock);

fail_free_all:
	kfree(buf);
exit:
	return count;
}

static ssize_t reg_write(struct file *file, const char __user *ubuf,
						size_t count, loff_t *ppos)
{
	struct seq_file *s = file->private_data;
	struct msm_fb_data_type *mfd = s->private;
	struct mdss_panel_data *pdata;
	struct mdss_mdp_ctl *ctl;
	char *buf;
	const char *p;
	enum dbg_cmd_type cmd;
	u8 data[MAX_WRITE_DATA];
	int i = 0;
	int j;
	int ret;
	struct dsi_cmd_desc dsi;
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;

	if (!mfd->panel_power_on) {
		pr_err("%s: panel is NOT on\n", __func__);
		goto exit;
	}

	ctl = mfd_to_ctl(mfd);
	if (!ctl)
		goto exit;

	if (mutex_lock_interruptible(&ctl->lock))
		goto exit;

	pdata = dev_get_platdata(&mfd->pdev->dev);
	if (!pdata) {
		pr_err("%s: no panel connected\n", __func__);
		goto exit;
	}

	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);
	if (!ctrl_pdata) {
		pr_err("%s: Invalid input data\n", __func__);
		goto exit;
	}

	pr_err("%s\n", __func__);
	ret = setup_reg_access(&buf, ubuf, count);
	if (ret)
		goto exit;

	ret = get_cmd_type(buf, &cmd);
	if (ret) {
		update_res_buf("Write - unknown type\n");
		goto fail_free_all;
	}

	p = buf;
	p = p+4;

	/* Get first param, Register */
	if (sscanf(p, "%4hhx", &data[0]) != 1) {
		update_res_buf("Write - parameter error\n");
		ret = -EINVAL;
		goto fail_free_all;
	}
	i++;

	while (isxdigit(*p) || (*p == 'x'))
		p++;

	ret = get_parameters(p, data, ARRAY_SIZE(data) - 1, &i);
	if (ret)
		goto fail_free_all;

	ret = prepare_for_reg_access(mfd);
	if (ret)
		goto fail_free_all;

	if (cmd == DCS) {
		if (i == 1) { /* 0 parameters */
			dsi.dchdr.dtype = DTYPE_DCS_WRITE;
		} else if (i == 2) { /* 1 parameter */
			dsi.dchdr.dtype = DTYPE_DCS_WRITE1;
		} else { /* Many parameters */
			dsi.dchdr.dtype = DTYPE_DCS_LWRITE;
		}
	} else {
		if (i == 1) { /* 0 parameters */
			dsi.dchdr.dtype = DTYPE_GEN_WRITE;
		} else if (i == 2) { /* 1 parameter */
			dsi.dchdr.dtype = DTYPE_GEN_WRITE1;
		} else if (i == 3) { /* 2 parameters */
			dsi.dchdr.dtype = DTYPE_GEN_WRITE2;
		} else { /* Many parameters */
			dsi.dchdr.dtype = DTYPE_GEN_LWRITE;
		}
	}
	dsi.dchdr.last = 1;
	dsi.dchdr.vc = 0;
	dsi.dchdr.ack = 0;
	dsi.dchdr.wait = 0;
	dsi.dchdr.dlen = i;
	dsi.payload = data;

	pr_err("%s: last = %d, vc = %d, ack = %d, wait = %d, dlen = %d\n",
		__func__,
		dsi.dchdr.last, dsi.dchdr.vc, dsi.dchdr.ack, dsi.dchdr.wait,
		dsi.dchdr.dlen);
	for (j = 0; j < i; j++)
		pr_err("%s: payload[%d] = 0x%x\n",
			__func__, j, dsi.payload[j]);
	mdss_dsi_cmds_tx(ctrl_pdata, &dsi, 1);

	ret = post_reg_access(mfd);
	if (ret)
		goto fail_free_all;

	print_params(dsi.dchdr.dtype, data[0], i, dsi.payload);

	mutex_unlock(&ctl->lock);

fail_free_all:
	kfree(buf);
exit:
	return count;
}

static int result_show(struct seq_file *s, void *unused)
{
	seq_printf(s, "%s", res_buf);
	if (!res_buf)
		seq_printf(s, "\n");
	return 0;
}

static int read_open(struct inode *inode, struct file *file)
{
	return single_open(file, NULL, inode->i_private);
}

static const struct file_operations read_fops = {
	.owner			= THIS_MODULE,
	.open			= read_open,
	.write			= reg_read,
	.llseek			= seq_lseek,
	.release		= single_release,
};

static int write_open(struct inode *inode, struct file *file)
{
	return single_open(file, NULL, inode->i_private);
}

static const struct file_operations write_fops = {
	.owner			= THIS_MODULE,
	.open			= write_open,
	.write			= reg_write,
	.llseek			= seq_lseek,
	.release		= single_release,
};

static int result_open(struct inode *inode, struct file *file)
{
	return single_open(file, result_show, inode->i_private);
}

static const struct file_operations result_fops = {
	.owner		= THIS_MODULE,
	.open		= result_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

#define DSI_BUF_SIZE	1024

void mipi_dsi_panel_create_debugfs(struct msm_fb_data_type *mfd)
{
	struct device *dev;
	struct dentry *root;
	struct mdss_panel_data *pdata;
	struct platform_device *pdev;


	pdata = dev_get_platdata(&mfd->pdev->dev);
	if (!pdata) {
		pr_err("%s: no panel connected\n", __func__);
		return;
	}

	pdev = pdata->panel_pdev;
	if (!pdev) {
		pr_err("%s: no device\n", __func__);
		return;
	}

	dev = &pdev->dev;

	if (!&dev->kobj) {
		pr_err("%s: no &dev->kobj\n", __func__);
		return;
	}

	mdss_dsi_buf_alloc(&debug_tx_buf, ALIGN(DSI_BUF_SIZE, SZ_4K));
	mdss_dsi_buf_alloc(&debug_rx_buf, ALIGN(DSI_BUF_SIZE, SZ_4K));

	dev_info(dev, "%s: create folder %s\n", __func__,
						kobject_name(&dev->kobj));

	root = debugfs_create_dir("mdss_dsi_panel", 0);

	if (!root) {
		dev_err(dev, "%s: dbgfs create dir failed\n", __func__);
	} else {
		if (!debugfs_create_file("read", S_IWUSR, root, mfd,
								&read_fops)) {
			dev_err(dev, "%s: failed to create dbgfs read file\n",
								__func__);
			return;
		}
		if (!debugfs_create_file("write", S_IWUSR, root, mfd,
								&write_fops)) {
			dev_err(dev, "%s: failed to create dbgfs write file\n",
								__func__);
			return;
		}
		if (!debugfs_create_file("result", S_IRUGO, root, mfd,
								&result_fops)) {
			dev_err(dev, "%s: failed to create dbgfs result file\n",
								__func__);
			return;
		}
	}
}
