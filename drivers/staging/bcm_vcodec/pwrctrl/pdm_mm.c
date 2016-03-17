/*******************************************************************************
Copyright 2013 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement
governing use of this software, this software is licensed to you under the
terms of the GNU General Public License version 2, available at
http://www.gnu.org/copyleft/gpl.html (the "GPL").

Notwithstanding the above, under no circumstances may you combine this software
in any way with any other Broadcom software provided under a license other than
the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>

#include <linux/clk.h>
#include <linux/list.h>
#include <plat/pi_mgr.h>
#include <plat/clock.h>

#include <linux/bcm_pdm_mm.h>
#include "pdm_mm_internal.h"


#define MAX_NAME_LEN		256
#define H264_DFS_DEV_NAME	"h264"
#define H264_COMMON_CLK_NAME	H264_AXI_BUS_CLK_NAME_STR
#define PROC_DIR		"bcm_pdm_mm"
#define PROC_REFCNT		"refcnt"


static const struct bcm_pdm_mm_qos_resource resource_list[] = {
	{
		.name = "mm_qos_res_h264_on",
		.type = BCM_PDM_MM_RESOURCE_TYPE_POWER,
		.resource = BCM_PDM_MM_RESOURCE_H264_POWER,
	},
	{
		.name = "mm_qos_res_h264_freq",
		.type = BCM_PDM_MM_RESOURCE_TYPE_FREQ,
		.resource = BCM_PDM_MM_RESOURCE_H264_FREQ,
		.cvalue = 250000000, /* default to max clock frequency */
	},
};

struct bcm_pdm_mm {
	struct proc_dir_entry *procdir;
	struct proc_dir_entry *procentry;
	/* mutex */
	struct mutex lock;
	/* registered requests */
	struct list_head req_list;
	int refcnt_pwr;
	int refcnt_freq;
	/* resource */
	struct pi_mgr_dfs_node dfs_node;
	struct clk *common_clk;
};

static struct bcm_pdm_mm *pdm_mm;

static const struct bcm_pdm_mm_qos_resource *find_qos_resource(const char *name)
{
	int i;
	int list_size = ARRAY_SIZE(resource_list);

	pr_debug("%s: search %s\n", __func__, name);

	for (i = 0; i < list_size; i++) {
		if (0 == strcmp(resource_list[i].name, name)) {
			pr_debug("%s: found %s\n", __func__, name);
			return &resource_list[i];
		}
	}

	pr_err("%s: \"%s\" not found\n", __func__, name);

	return NULL;
}

static int validate_qos_req(struct bcm_pdm_mm_qos_req *req)
{
	struct bcm_pdm_mm_qos_req *pos = NULL;
	int valid = 0;

	/* sanity check */
	if (!req) {
		pr_err("%s: invalid param", __func__);
		return 0;
	}
	if (!pdm_mm) {
		pr_err("%s: device not initialized", __func__);
		return 0;
	}

	mutex_lock(&pdm_mm->lock);

	list_for_each_entry(pos, &pdm_mm->req_list, node) {
		if (pos == req) {
			valid = 1;
			break;
		}
	}

	mutex_unlock(&pdm_mm->lock);

	return valid;
}

static void activate_qos_req(struct bcm_pdm_mm_qos_req *req)
{
	pr_debug("%s: %s\n", __func__, req->name);

	if (req->type == BCM_PDM_MM_RESOURCE_TYPE_POWER) {
		if (pdm_mm->refcnt_pwr == 0)
			clk_enable(pdm_mm->common_clk);
		pdm_mm->refcnt_pwr++;
		pr_debug("%s: name %s type %d refcnt %d\n", __func__,
			req->name, req->type, pdm_mm->refcnt_pwr);
	} else {
		if (pdm_mm->refcnt_freq == 0)
			pi_mgr_dfs_request_update(&pdm_mm->dfs_node,
				PI_OPP_TURBO);
		pdm_mm->refcnt_freq++;
		pr_debug("%s: name %s type %d refcnt %d\n", __func__,
			req->name, req->type, pdm_mm->refcnt_freq);
	}
	req->is_active = 1;
}

static void deactivate_qos_req(struct bcm_pdm_mm_qos_req *req)
{
	pr_debug("%s: %s\n", __func__, req->name);

	if (req->type == BCM_PDM_MM_RESOURCE_TYPE_POWER) {
		if (pdm_mm->refcnt_pwr == 1)
			clk_disable(pdm_mm->common_clk);
		pdm_mm->refcnt_pwr--;
		pr_debug("%s: name %s type %d refcnt %d\n", __func__,
			req->name, req->type, pdm_mm->refcnt_pwr);
	} else {
		if (pdm_mm->refcnt_freq == 1)
			pi_mgr_dfs_request_update(&pdm_mm->dfs_node,
				PI_OPP_ECONOMY);
		pdm_mm->refcnt_freq--;
		pr_debug("%s: name %s type %d refcnt %d\n", __func__,
			req->name, req->type, pdm_mm->refcnt_freq);
	}
	req->is_active = 0;
}

int bcm_pdm_mm_qos_req_create(struct bcm_pdm_mm_qos_req **req,
	const char *req_name, const char *resource_name)
{
	struct bcm_pdm_mm_qos_req *request;
	const struct bcm_pdm_mm_qos_resource *resource;

	pr_debug("%s:", __func__);

	/* sanity check */
	if (!req || !req_name || !resource_name) {
		pr_err("%s: invalid param", __func__);
		return -EINVAL;
	}
	if (!pdm_mm) {
		pr_err("%s: device not initialized", __func__);
		return -ENODEV;
	}

	/* match resource based on resource name */
	resource = find_qos_resource(resource_name);
	if (!resource) {
		pr_err("%s: invalid resource name", __func__);
		return -ENOENT;
	}

	/* allocate resource for the request */
	request = kzalloc(sizeof(struct bcm_pdm_mm_qos_req), GFP_KERNEL);
	if (!request) {
		pr_err("%s: fail to allocate resource for %s", __func__,
			resource_name);
		return -ENOMEM;
	}

	request->name = kstrndup(req_name, MAX_NAME_LEN, GFP_KERNEL);
	if (!request->name) {
		pr_err("%s: fail to duplicate string %s\n", __func__, req_name);
		kfree(request);
		return -ENOMEM;
	}
	request->type = resource->type;
	request->resource = resource->resource;
	request->cvalue = resource->cvalue;

	INIT_LIST_HEAD(&request->node);

	mutex_lock(&pdm_mm->lock);

	/* add request to list */
	list_add_tail(&request->node, &pdm_mm->req_list);

	mutex_unlock(&pdm_mm->lock);

	*req = request;

	return 0;
}
EXPORT_SYMBOL(bcm_pdm_mm_qos_req_create);

void bcm_pdm_mm_qos_req_destroy(struct bcm_pdm_mm_qos_req *req)
{
	pr_debug("%s:", __func__);

	if (!validate_qos_req(req)) {
		pr_err("%s: invalid destroy req", __func__);
		return;
	}

	mutex_lock(&pdm_mm->lock);

	/* remove request from the list */
	list_del(&req->node);

	/* release resource */
	if (req->is_active) {
		pr_err("%s: try to destroy active request %s", __func__,
			req->name);
		deactivate_qos_req(req);
	}

	kfree(req->name);

	kfree(req);

	mutex_unlock(&pdm_mm->lock);

	return;
}
EXPORT_SYMBOL(bcm_pdm_mm_qos_req_destroy);

int bcm_pdm_mm_qos_req_enable(struct bcm_pdm_mm_qos_req *req)
{
	pr_debug("%s:", __func__);

	if (!validate_qos_req(req)) {
		pr_err("%s: invalid enable req", __func__);
		return -EINVAL;
	}

	if (req->is_active) {
		pr_err("%s: request %s already active", __func__,
			req->name);
		return -EINVAL;
	}

	mutex_lock(&pdm_mm->lock);

	activate_qos_req(req);

	mutex_unlock(&pdm_mm->lock);

	return 0;
}
EXPORT_SYMBOL(bcm_pdm_mm_qos_req_enable);

int bcm_pdm_mm_qos_req_disable(struct bcm_pdm_mm_qos_req *req)
{
	pr_debug("%s:", __func__);

	if (!validate_qos_req(req)) {
		pr_err("%s: invalid disable req", __func__);
		return -EINVAL;
	}

	if (!req->is_active) {
		pr_err("%s: request %s already not active", __func__,
			req->name);
		return -EINVAL;
	}

	mutex_lock(&pdm_mm->lock);

	deactivate_qos_req(req);

	mutex_unlock(&pdm_mm->lock);

	return 0;
}
EXPORT_SYMBOL(bcm_pdm_mm_qos_req_disable);

int bcm_pdm_mm_qos_req_update(struct bcm_pdm_mm_qos_req *req, int cvalue)
{
	pr_debug("%s:", __func__);

	if (!validate_qos_req(req)) {
		pr_err("%s: invalid disable req", __func__);
		return -EINVAL;
	}

	pr_warn("%s: ignored", __func__);

	return 0;
}
EXPORT_SYMBOL(bcm_pdm_mm_qos_req_update);

static int bcm_pdm_mm_read_proc(char *buffer, char **start, off_t off,
		int count, int *eof, void *data)
{
	unsigned int len = 0;
	struct bcm_pdm_mm *local_pdm_mm = (struct bcm_pdm_mm *)data;

	if (!local_pdm_mm) {
		pr_err("%s: device not initialized", __func__);
		return 0;
	}

	if (off > 0)
		return 0;

	len += sprintf(buffer, "bcm_pdm_mm refcnt pwr %d freq %d\n",
		local_pdm_mm->refcnt_pwr, local_pdm_mm->refcnt_freq);

	return len;
}

static int bcm_pdm_mm_probe(struct platform_device *pdev)
{
	int ret;
	struct bcm_pdm_mm *local_pdm_mm;

	pr_debug("%s:", __func__);

	local_pdm_mm = devm_kzalloc(&pdev->dev, sizeof(struct bcm_pdm_mm),
			GFP_KERNEL);
	if (!local_pdm_mm) {
		pr_err("%s: allocation failed", __func__);
		return -ENOMEM;
	}

	local_pdm_mm->procdir = proc_mkdir(PROC_DIR, NULL);
	if (local_pdm_mm->procdir == NULL) {
		pr_err("%s: create proc dir failed\n", __func__);
		ret = -ENOENT;
		goto err_proc_mkdir;
	}

	local_pdm_mm->procentry = create_proc_entry(PROC_REFCNT,
			(S_IRUSR | S_IRGRP), local_pdm_mm->procdir);
	if (local_pdm_mm->procentry == NULL) {
		pr_err("%s: create proc entry failed\n", __func__);
		ret = -ENOENT;
		goto err_proc_create_entry;
	}

	local_pdm_mm->procentry->read_proc = bcm_pdm_mm_read_proc;
	local_pdm_mm->procentry->data = local_pdm_mm;

	local_pdm_mm->common_clk = clk_get(NULL, H264_COMMON_CLK_NAME);
	if (!local_pdm_mm->common_clk) {
		pr_err("%s: fail to get h264 common clk", __func__);
		ret = -EINVAL;
		goto err_get_common_clk;
	}

	ret = pi_mgr_dfs_add_request(&local_pdm_mm->dfs_node,
			(char *)H264_DFS_DEV_NAME, PI_MGR_PI_ID_MM,
			PI_OPP_ECONOMY);
	if (ret) {
		pr_err("%s: failed to register PI DFS request for %s",
			__func__, H264_DFS_DEV_NAME);
		goto err_dfs_req;
	}

	INIT_LIST_HEAD(&local_pdm_mm->req_list);

	mutex_init(&local_pdm_mm->lock);

	pdm_mm = local_pdm_mm;

	return 0;

err_dfs_req:
	clk_put(local_pdm_mm->common_clk);
err_get_common_clk:
	remove_proc_entry(PROC_REFCNT, local_pdm_mm->procdir);
err_proc_create_entry:
	remove_proc_entry(PROC_DIR, NULL);
err_proc_mkdir:
	pdm_mm = NULL;

	return ret;
}

static int bcm_pdm_mm_remove(struct platform_device *pdev)
{
	pr_debug("%s:", __func__);

	if (!pdm_mm) {
		pr_err("%s: driver does not exist", __func__);
		return 0;
	}

	pi_mgr_dfs_request_remove(&pdm_mm->dfs_node);

	clk_put(pdm_mm->common_clk);

	remove_proc_entry(PROC_REFCNT, pdm_mm->procdir);
	remove_proc_entry(PROC_DIR, NULL);

	pdm_mm = NULL;

	return 0;
}

static const struct of_device_id bcm_pdm_mm_of_match[] = {
	{ .compatible = "bcm,pdm-mm", },
	{}
};

MODULE_DEVICE_TABLE(of, bcm_pdm_mm_of_match);

static struct platform_driver bcm_pdm_mm_driver = {
	.driver = {
		.name = "bcm_pdm_mm",
		.owner = THIS_MODULE,
		.of_match_table = bcm_pdm_mm_of_match,
	},
	.probe = bcm_pdm_mm_probe,
	.remove = bcm_pdm_mm_remove,
};

static int __init bcm_pdm_mm_module_init(void)
{
	pr_debug("%s:", __func__);

	return platform_driver_register(&bcm_pdm_mm_driver);
}

static void __exit bcm_pdm_mm_module_exit(void)
{
	pr_debug("%s:", __func__);

	platform_driver_unregister(&bcm_pdm_mm_driver);
}

subsys_initcall(bcm_pdm_mm_module_init);
module_exit(bcm_pdm_mm_module_exit);

MODULE_AUTHOR("Broadcom Corporation");
MODULE_DESCRIPTION("Multimedia Power Domain Manager Driver");
MODULE_LICENSE("GPL");
