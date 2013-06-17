/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2012  Broadcom Corporation                                                        */
/*                                                                                              */
/*     Unless you and Broadcom execute a separate written software license agreement governing  */
/*     use of this software, this software is licensed to you under the terms of the GNU        */
/*     General Public License version 2 (the GPL), available at                                 */
/*                                                                                              */
/*          http://www.broadcom.com/licenses/GPLv2.php                                          */
/*                                                                                              */
/*     with the following added to such license:                                                */
/*                                                                                              */
/*     As a special exception, the copyright holders of this software give you permission to    */
/*     link this software with independent modules, and to copy and distribute the resulting    */
/*     executable under terms of your choice, provided that you also meet, for each linked      */
/*     independent module, the terms and conditions of the license of that module.              */
/*     An independent module is a module which is not derived from this software.  The special  */
/*     exception does not apply to any modifications of the software.                           */
/*                                                                                              */
/*     Notwithstanding the above, under no circumstances may you combine this software in any   */
/*     way with any other Broadcom software provided under a license other than the GPL,        */
/*     without Broadcom's express prior written consent.                                        */
/*                                                                                              */
/************************************************************************************************/

#include <linux/of_fdt.h>
#include <linux/mfd/bcmpmu.h>

struct bcmpmu_dt_batt_data {
	int valid;
	char *model;
	uint32_t *voltcap_map_len;
	uint32_t *voltcap_map_width;
	unsigned long voltcap_map_size;
	uint32_t *voltcap_map;
	uint32_t *cpcty;
	uint32_t *eoc;
	uint32_t *esr;
	uint32_t *chrg_1c;
};
struct bcmpmu_dt_batt_data battdata = {0};

struct bcmpmu_dt_pmu_data {
	int valid;
	uint32_t *cutoff;
	uint32_t *fg_slp_curr_ua;
	uint32_t *fg_factor;
	uint32_t *fg_sns_res;
	uint32_t *reginit_len;
	uint32_t *reginit_width;
	unsigned long reginit_size;
	uint32_t *reginit_data;
};
struct bcmpmu_dt_pmu_data pmudata = {0};

int __init early_init_dt_scan_pmu(unsigned long node, const char *uname,
				     int depth, void *data)
{
	const char *prop;
	unsigned long size;
	uint32_t *p;

	if (depth != 1 || strcmp(uname, "bcmpmu") != 0)
		return 0;

	prop = of_get_flat_dt_prop(node, "cutoff", &size);
	if (prop == NULL)
		printk(KERN_INFO "%s: cutoff not found\n", __func__);
	else
		pmudata.cutoff = (uint32_t *)prop;

	prop = of_get_flat_dt_prop(node, "fg_slp_curr_ua", &size);
	if (prop == NULL)
		printk(KERN_INFO "%s: fg_slp_curr_ua not found\n", __func__);
	else
		pmudata.fg_slp_curr_ua = (uint32_t *)prop;

	prop = of_get_flat_dt_prop(node, "fg_factor", &size);
	if (prop == NULL)
		printk(KERN_INFO "%s: fg_factor not found\n", __func__);
	else
		pmudata.fg_factor = (uint32_t *)prop;

	prop = of_get_flat_dt_prop(node, "fg_sns_res", &size);
	if (prop == NULL)
		printk(KERN_INFO "%s: fg_sns_res not found\n", __func__);
	else
		pmudata.fg_sns_res = (uint32_t *)prop;

	prop = of_get_flat_dt_prop(node, "reginit", &size);
	if (prop == NULL)
		printk(KERN_INFO "%s: pmu reginit not found\n", __func__);
	else {
		p = (uint32_t *)prop;
		pmudata.reginit_width = p;
		p++;
		pmudata.reginit_len = p;
	}

	prop = of_get_flat_dt_prop(node, "initdata", &size);
	if (prop == NULL)
		printk(KERN_INFO "%s: pmu initdata found\n", __func__);
	else {
		pmudata.reginit_size = size/4;
		pmudata.reginit_data = (uint32_t *)prop;
	}
	pmudata.valid = 1;
	return 1;
}

int __init early_init_dt_scan_batt(unsigned long node, const char *uname,
				     int depth, void *data)
{
	const char *prop;
	unsigned long size;
	uint32_t *p;

	if (depth != 1 || strcmp(uname, "battery") != 0)
		return 0;

	prop = of_get_flat_dt_prop(node, "charge-eoc", &size);
	if (prop == NULL)
		printk(KERN_INFO "%s: charge-eoc not found\n", __func__);
	else
		battdata.eoc = (uint32_t *)prop;

	prop = of_get_flat_dt_prop(node, "batt-esr", &size);
	if (prop == NULL)
		printk(KERN_INFO "%s: batt-esr not found\n", __func__);
	else
		battdata.esr = (uint32_t *)prop;

	prop = of_get_flat_dt_prop(node, "charge-1c", &size);
	if (prop == NULL)
		printk(KERN_INFO "%s: charge-1c not found\n", __func__);
	else
		battdata.chrg_1c = (uint32_t *)prop;

	prop = of_get_flat_dt_prop(node, "capacity", &size);
	if (prop == NULL)
		printk(KERN_INFO "%s: battery capacity not found\n", __func__);
	else
		battdata.cpcty = (uint32_t *)prop;

	prop = of_get_flat_dt_prop(node, "vcmap-size", &size);
	if (prop == NULL)
		printk(KERN_INFO "%s: battery vcmap-size not found\n", __func__);
	else {
		p = (uint32_t *)prop;
		battdata.voltcap_map_width = p;
		p++;
		battdata.voltcap_map_len = p;
	}
	prop = of_get_flat_dt_prop(node, "vcmap", &size);
	if (prop == NULL)
		printk(KERN_INFO "%s: battery vcmap not found\n", __func__);
	else {
		battdata.voltcap_map = (uint32_t *)prop;
		battdata.voltcap_map_size = size/4;
	}

	prop = of_get_flat_dt_prop(node, "model", &size);
	if (prop == NULL)
		printk(KERN_INFO "%s: battery model not found\n", __func__);
	else
		battdata.model = (char *)prop;

	battdata.valid = 1;
	return 1;
}

void bcmpmu_update_pdata_dt_batt(struct bcmpmu_platform_data *pdata)
{
	uint32_t *p = battdata.voltcap_map;
	uint32_t len = 0, wid = 0;
	int i;

	if (battdata.valid == 0)
		return;

	if (battdata.eoc != 0)
		pdata->chrg_eoc = be32_to_cpu(*battdata.eoc);
	if (battdata.chrg_1c != NULL)
		pdata->chrg_1c_rate = be32_to_cpu(*battdata.chrg_1c);
	if (battdata.esr != 0)
		pdata->batt_impedence = be32_to_cpu(*battdata.esr);
	if (battdata.cpcty != 0)
		pdata->fg_capacity_full = be32_to_cpu(*battdata.cpcty) * 3600;
	if (battdata.model != 0)
		pdata->batt_model = battdata.model;
	if (battdata.voltcap_map_width != 0)
		wid = be32_to_cpu(*battdata.voltcap_map_width);
	if (battdata.voltcap_map_len != 0)
		len = be32_to_cpu(*battdata.voltcap_map_len);

	if (wid * len != battdata.voltcap_map_size)
		printk(KERN_INFO "%s: batt v-c tbl err, w=0x%X, l=0x%X, size=%ld\n",
			__func__, wid, len, battdata.voltcap_map_size);
	else if (len != pdata->batt_voltcap_map_len)
		printk(KERN_INFO "%s: battery vcmap-size incorrect\n", __func__);
	else {
		for (i=0; i<len; i++) {
			pdata->batt_voltcap_map[i].volt = be32_to_cpu(*p++);
			pdata->batt_voltcap_map[i].cap = be32_to_cpu(*p++);
		}
	}
}

void bcmpmu_update_pdata_dt_pmu(struct bcmpmu_platform_data *pdata)
{
	uint32_t *p, *p1;
	struct bcmpmu_rw_data *tbl;
	uint32_t len = 0, wid = 0;
	int i;

	if (pmudata.valid == 0)
		return;

	if (pmudata.cutoff != 0)
		pdata->cutoff_volt = be32_to_cpu(*pmudata.cutoff);
	if (pmudata.fg_slp_curr_ua != NULL)
		pdata->fg_slp_curr_ua = be32_to_cpu(*pmudata.fg_slp_curr_ua);
	if (pmudata.fg_factor != 0)
		pdata->fg_factor = (pdata->fg_factor *
			be32_to_cpu(*pmudata.fg_factor))/1000;
	if (pmudata.fg_sns_res != 0)
		pdata->fg_sns_res = be32_to_cpu(*pmudata.fg_sns_res);

	if (pmudata.reginit_width != 0)
		wid = be32_to_cpu(*pmudata.reginit_width);
	if (pmudata.reginit_len != 0)
		len = be32_to_cpu(*pmudata.reginit_len);

	if (wid * len != pmudata.reginit_size)
		printk(KERN_INFO "%s: pmu reg init table error, w=%d, l=%d size=%ld\n",
			__func__, wid, len, pmudata.reginit_size);
	else {
		tbl = kzalloc(len * sizeof(struct bcmpmu_rw_data), GFP_KERNEL);
		if (tbl == NULL)
			printk(KERN_INFO
				"%s: failed to alloc mem for pdata->init_data.\n",
				__func__);
		else {
			pdata->init_max = len;
			p = pmudata.reginit_data;
			p1 = (uint32_t *)tbl;
			for (i = 0; i < pmudata.reginit_size; i++)
				*p1++ = be32_to_cpu(*p++);
			pdata->init_data = tbl;
		}
	}
}
