// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyrights (C) 2021 Silicon Mitus, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/device.h>
#include <linux/workqueue.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/usb/typec/sm5038/sm5038_pd.h>
#include <linux/usb/typec/sm5038/sm5038_typec.h>
#include <linux/power/sm5038.h>


struct sm5038_usbpd_data *g_pd_data;
int sm5038_select_pps(int num, int ppsVol, int ppsCur);
int sm5038_select_pdo(int num);
int sm5038_usbpd_get_identity(struct sm5038_usbpd_data *pd_data);
int sm5038_usbpd_get_svids(struct sm5038_usbpd_data *pd_data);
int sm5038_usbpd_get_modes(struct sm5038_usbpd_data *pd_data);
int sm5038_usbpd_enter_mode(struct sm5038_usbpd_data *pd_data);

static int usbpd_sm5038_uevent(struct device *dev, struct kobj_uevent_env *env)
{
	struct sm5038_usbpd_data *pd_data = dev_get_drvdata(dev);
	struct sm5038_phydrv_data *pdic_data = pd_data->phy_driver_data;
	int i;

	if (!pd_data) {
		pr_err("%s : pd_data is null\n", __func__);
		return -ENXIO;
	}

	if (!pdic_data) {
		pr_err("%s : pdic_data is null\n", __func__);
		return -ENXIO;
	}

	add_uevent_var(env, "DATA_ROLE=%s", pdic_data->data_role == TYPEC_HOST ?
			"dfp" : "ufp");

	if (pdic_data->power_role == TYPEC_SINK) {
		add_uevent_var(env, "POWER_ROLE=sink");
		add_uevent_var(env, "SRC_CAP_ID=%d", pd_data->src_cap_id);

		for (i = 0; i < ARRAY_SIZE(pd_data->received_pdos); i++)
			add_uevent_var(env, "PDO%d=%08x", i,
					pd_data->received_pdos[i]);

		add_uevent_var(env, "REQUESTED_PDO=%d", pd_data->selected_pdo_num);
		add_uevent_var(env, "SELECTED_PDO=%d", pd_data->current_pdo_num);
	} else {
		add_uevent_var(env, "POWER_ROLE=source");
		add_uevent_var(env, "PDO=%08x",
				pd_data->source_data_obj.object);
	}

	add_uevent_var(env, "RDO=%08x", pd_data->sink_rdo.object);
	add_uevent_var(env, "CONTRACT=%s", pdic_data->pd_support ?
				"explicit" : "implicit");
/*	add_uevent_var(env, "ALT_MODE=%d", pd->vdm_state == MODE_ENTERED);

	add_uevent_var(env, "SDB=%02x %02x %02x %02x %02x %02x",
			pd->status_db[0], pd->status_db[1], pd->status_db[2],
			pd->status_db[3], pd->status_db[4], pd->status_db[5]);
*/
	return 0;
}

static ssize_t contract_show(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	struct sm5038_usbpd_data *pd_data = dev_get_drvdata(dev);
	struct sm5038_phydrv_data *pdic_data = pd_data->phy_driver_data;

	return scnprintf(buf, PAGE_SIZE, "%s\n",
			pdic_data->pd_support ?  "explicit" : "implicit");
}
static DEVICE_ATTR_RO(contract);

static ssize_t current_pr_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct sm5038_usbpd_data *pd_data = dev_get_drvdata(dev);
	struct sm5038_phydrv_data *pdic_data = pd_data->phy_driver_data;
	const char *pr = "none";

	if (pdic_data->power_role == TYPEC_SINK)
		pr = "sink";
	else if (pdic_data->power_role == TYPEC_SOURCE)
		pr = "source";

	return scnprintf(buf, PAGE_SIZE, "%s\n", pr);
}
static DEVICE_ATTR_RO(current_pr);

static ssize_t current_dr_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct sm5038_usbpd_data *pd_data = dev_get_drvdata(dev);
	struct sm5038_phydrv_data *pdic_data = pd_data->phy_driver_data;
	const char *dr = "none";

	if (pdic_data->data_role == TYPEC_DEVICE)
		dr = "ufp";
	else if (pdic_data->data_role == TYPEC_HOST)
		dr = "dfp";

	return scnprintf(buf, PAGE_SIZE, "%s\n", dr);
}
static DEVICE_ATTR_RO(current_dr);

static ssize_t src_cap_id_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct sm5038_usbpd_data *pd_data = dev_get_drvdata(dev);

	return scnprintf(buf, PAGE_SIZE, "%d\n", pd_data->src_cap_id);
}
static DEVICE_ATTR_RO(src_cap_id);

/* Dump received source PDOs in human-readable format */
static ssize_t pdo_h_show(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	struct sm5038_usbpd_data *pd_data = dev_get_drvdata(dev);
	int i, type;
	ssize_t cnt = 0;
	data_obj_type obj;

	for (i = 0; i < ARRAY_SIZE(pd_data->received_pdos); i++) {
		obj.object = pd_data->received_pdos[i];

		if (obj.object == 0)
			break;

		cnt += scnprintf(&buf[cnt], PAGE_SIZE - cnt, "PDO %d\n", i + 1);
		type = obj.power_data_obj_supply_type.supply_type;

		if (type == POWER_TYPE_FIXED) {
			cnt += scnprintf(&buf[cnt], PAGE_SIZE - cnt,
					"\tFixed supply\n"
					"\tDual-Role Power:%d\n"
					"\tUSB Suspend Supported:%d\n"
					"\tExternally Powered:%d\n"
					"\tUSB Communications Capable:%d\n"
					"\tData Role Swap:%d\n"
					"\tPeak Current:%d\n"
					"\tVoltage:%d (mV)\n"
					"\tMax Current:%d (mA)\n",
					obj.power_data_obj.dual_role_power,
					obj.power_data_obj.usb_suspend_support,
					obj.power_data_obj.externally_powered,
					obj.power_data_obj.usb_comm_capable,
					obj.power_data_obj.data_role_swap,
					obj.power_data_obj.peak_current,
					obj.power_data_obj.voltage * 50,
					obj.power_data_obj.max_current * 10);
		} else if (type == POWER_TYPE_BATTERY) {
			cnt += scnprintf(&buf[cnt], PAGE_SIZE - cnt,
					"\tBattery supply\n"
					"\tMax Voltage:%d (mV)\n"
					"\tMin Voltage:%d (mV)\n"
					"\tMax Power:%d (mW)\n",
					obj.power_data_obj_battery.max_voltage * 50,
					obj.power_data_obj_battery.min_voltage * 50,
					obj.power_data_obj_battery.max_power * 250);
		} else if (type == POWER_TYPE_VARIABLE) {
			cnt += scnprintf(&buf[cnt], PAGE_SIZE - cnt,
					"\tVariable supply\n"
					"\tMax Voltage:%d (mV)\n"
					"\tMin Voltage:%d (mV)\n"
					"\tMax Current:%d (mA)\n",
					obj.power_data_obj_variable.max_voltage * 50,
					obj.power_data_obj_variable.min_voltage * 50,
					obj.power_data_obj_variable.max_current * 10);
		} else if (type == POWER_TYPE_APDO) {
			cnt += scnprintf(&buf[cnt], PAGE_SIZE - cnt,
					"\tProgrammable Power supply\n"
					"\tMax Voltage:%d (mV)\n"
					"\tMin Voltage:%d (mV)\n"
					"\tMax Current:%d (mA)\n",
					obj.power_data_obj_programmable.max_voltage * 100,
					obj.power_data_obj_programmable.min_voltage * 100,
					obj.power_data_obj_programmable.max_current * 50);
		} else {
			cnt += scnprintf(&buf[cnt], PAGE_SIZE - cnt,
					"Invalid PDO\n");
		}

		cnt += scnprintf(&buf[cnt], PAGE_SIZE - cnt, "\n");
	}

	return cnt;
}
static DEVICE_ATTR_RO(pdo_h);

static ssize_t pdo_n_show(struct device *dev, struct device_attribute *attr,
		char *buf);

#define PDO_ATTR(n) {					\
	.attr	= { .name = __stringify(pdo##n), .mode = 0444 },	\
	.show	= pdo_n_show,				\
}
static struct device_attribute dev_attr_pdos[] = {
	PDO_ATTR(1),
	PDO_ATTR(2),
	PDO_ATTR(3),
	PDO_ATTR(4),
	PDO_ATTR(5),
	PDO_ATTR(6),
	PDO_ATTR(7),
};

static ssize_t pdo_n_show(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	struct sm5038_usbpd_data *pd_data = dev_get_drvdata(dev);
	int i;

	for (i = 0; i < ARRAY_SIZE(dev_attr_pdos); i++)
		if (attr == &dev_attr_pdos[i])
			/* dump the PDO as a hex string */
			return snprintf(buf, PAGE_SIZE, "%08x\n",
					pd_data->received_pdos[i]);

	dev_err(&pd_data->dev, "Invalid PDO index\n");
	return -EINVAL;
}

static ssize_t select_pdo_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct sm5038_usbpd_data *pd_data = dev_get_drvdata(dev);
	struct sm5038_phydrv_data *pdic_data = pd_data->phy_driver_data;
	struct sm5038_usbpd_manager_data *manager = &pd_data->manager;
#if defined(CONFIG_SM5038_SHORT_PROTECTION)
	bool vbus_short = false;
#endif
	int src_cap_id;
	int pdo_num, uv = 0, ua = 0;
	int ret;

	if (!pd_data) {
		pr_err("%s : pd_data is null\n", __func__);
		return -ENXIO;
	}

	ret = sscanf(buf, "%d %d %d %d", &src_cap_id, &pdo_num, &uv, &ua);

	if (ret != 2 && ret != 4) {
		pr_err("%s : Must specify <src cap id> <PDO> [<uV> <uA>]\n",
				__func__);
		return -EINVAL;
	}

	if (src_cap_id != pd_data->src_cap_id) {
		pr_err("%s : src_cap_id mismatch.  Requested:%d, current:%d\n",
				__func__, src_cap_id, pd_data->src_cap_id);
		return -EINVAL;
	}

	pr_info(" %s : src_cap_id(%d), pdo_num(%d), uv(%d), ua(%d)\n",
			__func__, src_cap_id, pdo_num, uv, ua);

	if (!pdic_data->is_attached) {
		pr_info(" %s : PDO(%d) is ignored because of plug detached\n",
				__func__, pdo_num);
		return -EPERM;
	}

#if defined(CONFIG_SM5038_SHORT_PROTECTION)
	pd_data->phy_ops.get_short_state(pd_data, &vbus_short);

	if (vbus_short) {
		pr_err(" %s : PDO(%d) is ignored because of vbus short\n",
				__func__, pdo_num);
		return -EPERM;
	}
#endif

	if (pdo_num > pd_data->available_pdo_num) {
		pr_err("%s: request pdo num(%d) is higher that available pdo.\n",
				__func__, pdo_num);
		return -EINVAL;
	} else if (pdo_num < 1) {
		pr_err("%s: invalid PDO :%d\n", __func__, pdo_num);
		return -EINVAL;
	}

	if (pdo_num > 1 && (manager->fled_torch_enable || manager->fled_flash_enable)) {
		pr_info(" %s : PDO(%d) is ignored because of [torch(%d) or flash(%d)]\n",
				__func__, pdo_num, manager->fled_torch_enable, manager->fled_flash_enable);
		return -EPERM;
	}


	if (pd_data->power_list[pdo_num].apdo)
		ret = sm5038_select_pps(pdo_num, uv, ua);
	else
		ret = sm5038_select_pdo(pdo_num);

	return ret ? ret : size;
}


static ssize_t select_pdo_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct sm5038_usbpd_data *pd_data;

	pd_data = dev_get_drvdata(dev);
	if (!pd_data) {
		pr_err("%s : pd_data is null\n", __func__);
		return -ENXIO;
	}

	return scnprintf(buf, PAGE_SIZE, "%d\n", pd_data->selected_pdo_num);
}
static DEVICE_ATTR_RW(select_pdo);

static ssize_t rdo_show(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	struct sm5038_usbpd_data *pd_data = dev_get_drvdata(dev);

	/* dump the RDO as a hex string */
	return scnprintf(buf, PAGE_SIZE, "%08x\n", pd_data->sink_rdo.object);
}
static DEVICE_ATTR_RO(rdo);

static ssize_t rdo_h_show(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	struct sm5038_usbpd_data *pd_data = dev_get_drvdata(dev);
	int pos = pd_data->sink_rdo.request_data_object.object_position;
	int type = ((pd_data->received_pdos[pos-1]) >> 30) & 3;
	int len;

	len = scnprintf(buf, PAGE_SIZE, "Request Data Object\n"
			"\tObj Pos:%d\n"
			"\tGiveback:%d\n"
			"\tCapability Mismatch:%d\n"
			"\tUSB Communications Capable:%d\n"
			"\tNo USB Suspend:%d\n",
			pd_data->sink_rdo.request_data_object.object_position,
			pd_data->sink_rdo.request_data_object.give_back,
			pd_data->sink_rdo.request_data_object.capability_mismatch,
			pd_data->sink_rdo.request_data_object.usb_comm_capable,
			pd_data->sink_rdo.request_data_object.no_usb_suspend);

	switch (type) {
	case POWER_TYPE_FIXED:
	case POWER_TYPE_VARIABLE:
		len += scnprintf(buf + len, PAGE_SIZE - len,
				"(Fixed/Variable)\n"
				"\tOperating Current:%d (mA)\n"
				"\t%s Current:%d (mA)\n",
				pd_data->sink_rdo.request_data_object.op_current * 10,
				pd_data->sink_rdo.request_data_object.give_back ? "Min" : "Max",
				pd_data->sink_rdo.request_data_object.min_current * 10);
		break;

	case POWER_TYPE_BATTERY:
		len += scnprintf(buf + len, PAGE_SIZE - len,
				"(Battery)\n"
				"\tOperating Power:%d (mW)\n"
				"\t%s Power:%d (mW)\n",
				pd_data->sink_rdo.request_data_object_battery.op_power * 250,
				pd_data->sink_rdo.request_data_object_battery.give_back ? "Min" : "Max",
				pd_data->sink_rdo.request_data_object_battery.max_power * 250);
		break;

	case POWER_TYPE_APDO:
		len += scnprintf(buf + len, PAGE_SIZE - len,
				"(Programmable)\n"
				"\tOutput Voltage:%d (mV)\n"
				"\tOperating Current:%d (mA)\n",
				pd_data->sink_rdo.request_data_object_programmable.output_voltage * 20,
				pd_data->sink_rdo.request_data_object_programmable.op_current * 50);
		break;
	}

	return len;
}
static DEVICE_ATTR_RO(rdo_h);

static struct attribute *usbpd_sm5038_attrs[] = {
	&dev_attr_contract.attr,
//	&dev_attr_initial_pr.attr,
	&dev_attr_current_pr.attr,
//	&dev_attr_initial_dr.attr,
	&dev_attr_current_dr.attr,
	&dev_attr_src_cap_id.attr,
	&dev_attr_pdo_h.attr,
	&dev_attr_pdos[0].attr,
	&dev_attr_pdos[1].attr,
	&dev_attr_pdos[2].attr,
	&dev_attr_pdos[3].attr,
	&dev_attr_pdos[4].attr,
	&dev_attr_pdos[5].attr,
	&dev_attr_pdos[6].attr,
	&dev_attr_select_pdo.attr,
	&dev_attr_rdo.attr,
	&dev_attr_rdo_h.attr,
//	&dev_attr_hard_reset.attr,
//	&dev_attr_get_src_cap_ext.attr,
//	&dev_attr_get_status.attr,
//	&dev_attr_get_pps_status.attr,
//	&dev_attr_get_battery_cap.attr,
//	&dev_attr_get_battery_status.attr,
	NULL,
};
ATTRIBUTE_GROUPS(usbpd_sm5038);

static struct class sm5038_usbpd_class = {
	.name = "usbpd_sm5038",
	.owner = THIS_MODULE,
	.dev_uevent = usbpd_sm5038_uevent,
	.dev_groups = usbpd_sm5038_groups,
};

int sm5038_select_pdo(int num)
{
	struct sm5038_usbpd_data *pd_data = g_pd_data;

	pd_data->selected_pdo_num = num;

	pr_info(" %s : PDO(%d) is selected to change\n",
		__func__, pd_data->selected_pdo_num);

	sm5038_usbpd_inform_event(pd_data, MANAGER_NEW_POWER_SRC);

	return 0;
}


int sm5038_select_pps(int num, int ppsVol, int ppsCur)
{
	struct sm5038_usbpd_data *pd_data = g_pd_data;
	struct sm5038_phydrv_data *pdic_data = pd_data->phy_driver_data;
	int timeout = 0;

	pd_data->selected_pdo_num = num;

	if (ppsVol > pd_data->power_list[num].max_voltage) {
		pr_info("%s: ppsVol is over(%d, max:%d)\n",
			__func__, ppsVol, pd_data->power_list[num].max_voltage);
		ppsVol = pd_data->power_list[num].max_voltage;
	} else if (ppsVol < pd_data->power_list[num].min_voltage) {
		pr_info("%s: ppsVol is under(%d, min:%d)\n",
			__func__, ppsVol, pd_data->power_list[num].min_voltage);
		ppsVol = pd_data->power_list[num].min_voltage;
	}

	if (ppsCur > pd_data->power_list[num].max_current) {
		pr_info("%s: ppsCur is over(%d, max:%d)\n",
			__func__, ppsCur, pd_data->power_list[num].max_current);
		ppsCur = pd_data->power_list[num].max_current;
	} else if (ppsCur < 0) {
		pr_info("%s: ppsCur is under(%d, 0)\n",
			__func__, ppsCur);
		ppsCur = 0;
	}

	pd_data->request_pps_vol = ppsVol;
	pd_data->request_pps_cur = ppsCur;

	pr_info(" %s : PPS PDO(%d), voltage(%d), current(%d) is selected to change\n", __func__,
		pd_data->selected_pdo_num, pd_data->request_pps_vol, pd_data->request_pps_vol);

	if ((pdic_data->rp_currentlvl == RP_CURRENT_LEVEL3) &&
			(pd_data->specification_revision == USBPD_REV_30)) {
		sm5038_usbpd_inform_event(pd_data, MANAGER_NEW_POWER_SRC);
	} else {
		pr_info(" %s : PD 3.0, but SinkTxNG state.\n", __func__);
	}

	reinit_completion(&pd_data->pd_completion);
	timeout =
	    wait_for_completion_timeout(&pd_data->pd_completion,
					msecs_to_jiffies(1000));

	if (!timeout)
		return -EBUSY;

	return 0;
}
#if 0
int sm5038_get_apdo_max_power(unsigned int *pdo_pos,
		unsigned int *taMaxVol, unsigned int *taMaxCur, unsigned int *taMaxPwr)
{
	struct sm5038_usbpd_data *pd_data = g_pd_data;
	int i;
	int ret = 0;
	int max_current = 0, max_voltage = 0, max_power = 0;

	if (!pd_data->pd_noti.sink_status.has_apdo) {
		pr_info("%s: pd don't have apdo\n",	__func__);
		return -EPERM;
	}

	/* First, get TA maximum power from the fixed PDO */
	for (i = 1; i <= pd_data->pd_noti.sink_status.available_pdo_num; i++) {
		if (!(pd_data->pd_noti.sink_status.power_list[i].apdo)) {
			max_voltage = pd_data->pd_noti.sink_status.power_list[i].max_voltage;
			max_current = pd_data->pd_noti.sink_status.power_list[i].max_current;
			max_power = (max_voltage * max_current > max_power) ? (max_voltage * max_current) : max_power;
			*taMaxPwr = max_power;	/* mW */
		}
	}

	if (*pdo_pos == 0) {
		/* Get the proper PDO */
		for (i = 1; i <= pd_data->pd_noti.sink_status.available_pdo_num; i++) {
			if (pd_data->pd_noti.sink_status.power_list[i].apdo) {
				if (pd_data->pd_noti.sink_status.power_list[i].max_voltage >= *taMaxVol) {
					*pdo_pos = i;
					*taMaxVol = pd_data->pd_noti.sink_status.power_list[i].max_voltage;
					*taMaxCur = pd_data->pd_noti.sink_status.power_list[i].max_current;
					break;
				}
			}
			if (*pdo_pos)
				break;
		}

		if (*pdo_pos == 0) {
			pr_info("mv (%d) and ma (%d) out of range of APDO\n",
				*taMaxVol, *taMaxCur);
			ret = -EINVAL;
		}
	} else {
		/* If we already have pdo object position, we don't need to search max current */
		ret = -ENOTSUPP;
	}

	pr_info("%s : *pdo_pos(%d), *taMaxVol(%d), *maxCur(%d), *maxPwr(%d)\n",
		__func__, *pdo_pos, *taMaxVol, *taMaxCur, *taMaxPwr);

	return ret;
}
#endif

void sm5038_usbpd_uevent_notifier(struct device *dev)
{
	struct sm5038_usbpd_data *pd_data = dev_get_drvdata(dev);

	kobject_uevent(&pd_data->dev.kobj, KOBJ_CHANGE);
}
EXPORT_SYMBOL_GPL(sm5038_usbpd_uevent_notifier);

void sm5038_usbpd_start_discover_msg_handler(struct work_struct *work)
{
	struct sm5038_usbpd_manager_data *manager =
		container_of(work, struct sm5038_usbpd_manager_data,
				start_discover_msg_handler.work);
	struct sm5038_usbpd_data *pd_data = g_pd_data;

	pr_info("%s: call start discover handler\n", __func__);

	if (manager->alt_sended == 0 && manager->vdm_en == 1) {
		sm5038_usbpd_inform_event(pd_data,
						MANAGER_SEND_DISCOVER_IDENTITY);
		manager->alt_sended = 1;
	}
}

void sm5038_usbpd_start_discover_msg_cancel(struct device *dev)
{
	struct sm5038_usbpd_data *pd_data = dev_get_drvdata(dev);
	struct sm5038_usbpd_manager_data *manager = &pd_data->manager;

	cancel_delayed_work_sync(&manager->start_discover_msg_handler);
}

void sm5038_request_default_power_src(void)
{
	struct sm5038_usbpd_data *pd_data = g_pd_data;
	struct sm5038_usbpd_manager_data *manager = &pd_data->manager;
	int pdo_num = pd_data->selected_pdo_num;

	pr_info(" %s : policy->state = (0x%x), pdo_num = %d, max vol = %d\n", __func__,
		pd_data->policy.state, pdo_num,
		pd_data->power_list[pdo_num].max_voltage);

	if ((pdo_num > 1) &&
		(pd_data->power_list[pdo_num].max_voltage > 5000)) {
		manager->origin_selected_pdo_num = pdo_num;
		sm5038_select_pdo(1);
	}
}
EXPORT_SYMBOL_GPL(sm5038_request_default_power_src);

int sm5038_usbpd_check_fled_state(bool enable, u8 mode)
{
	struct sm5038_usbpd_data *pd_data = g_pd_data;
	struct sm5038_phydrv_data *pdic_data = pd_data->phy_driver_data;
	struct sm5038_usbpd_manager_data *manager = &pd_data->manager;
	int pdo_num = pd_data->selected_pdo_num;

	pr_info("[%s] enable(%d), mode(%d)\n", __func__, enable, mode);

	if (mode == FLEDEN_TORCH_ON) { /* torch */
		cancel_delayed_work(&manager->new_power_handler);
		pr_info("[%s] new_power_handler cancel\n", __func__);

		manager->fled_torch_enable = enable;
	} else if (mode == FLEDEN_FLASH_ON) { /* flash */
		manager->fled_flash_enable = enable;
	}

	pr_info("[%s] fled_torch_enable(%d), fled_flash_enable(%d)\n", __func__,
		manager->fled_torch_enable, manager->fled_flash_enable);

	if (manager->fled_torch_enable || manager->fled_flash_enable) {
		if (pdic_data->pd_support && (pdo_num > 1)) {
			manager->origin_selected_pdo_num = pdo_num;
			sm5038_select_pdo(1);
		}
	}

	if ((manager->fled_torch_enable == false) &&
			(manager->fled_flash_enable == false) && pdic_data->pd_support) {
		if ((mode == FLEDEN_TORCH_ON) && (enable == false)) {
			cancel_delayed_work(&manager->new_power_handler);
			schedule_delayed_work(&manager->new_power_handler,
				msecs_to_jiffies(1000));
			pr_info("[%s] new_power_handler start(1sec)\n", __func__);
		} else {
			if (pdic_data->is_attached && (pd_data->available_pdo_num > 1)) {
				if ((manager->origin_selected_pdo_num > 1) ||
					((manager->origin_selected_pdo_num == 1) &&
					(pd_data->power_list[2].max_voltage < 10000))) {
					sm5038_select_pdo(2);
				}
			}
		}
	}

	return 0;
}
EXPORT_SYMBOL_GPL(sm5038_usbpd_check_fled_state);

void sm5038_usbpd_dp_detach(struct device *dev)
{
	struct sm5038_usbpd_data *pd_data = dev_get_drvdata(dev);
	struct sm5038_usbpd_manager_data *manager = &pd_data->manager;

	pr_info("%s: dp_is_connect %d\n", __func__, manager->dp_is_connect);

	manager->dp_selected_pin = 0;
	manager->dp_is_connect = 0;
	manager->dp_hs_connect = 0;
	manager->pin_assignment = 0;
}
EXPORT_SYMBOL_GPL(sm5038_usbpd_dp_detach);

void sm5038_usbpd_acc_detach(struct device *dev)
{
	struct sm5038_usbpd_data *pd_data = dev_get_drvdata(dev);
	struct sm5038_usbpd_manager_data *manager = &pd_data->manager;

	pr_info("%s\n",	__func__);
	manager->alt_sended = 0;
	manager->vdm_en = 0;
	manager->ext_sended = 0;
	manager->Vendor_ID = 0;
	manager->Product_ID = 0;
	manager->Device_Version = 0;
	manager->SVID_0 = 0;
	manager->SVID_1 = 0;
	manager->SVID_DP = 0;
	manager->Standard_Vendor_ID = 0;
	manager->acc_type = 0;
}
EXPORT_SYMBOL_GPL(sm5038_usbpd_acc_detach);

static void sm5038_usbpd_manager_new_power_handler(struct work_struct *wk)
{
	struct sm5038_usbpd_data *pd_data = g_pd_data;
	struct sm5038_phydrv_data *pdic_data = pd_data->phy_driver_data;
	struct sm5038_usbpd_manager_data *manager = &pd_data->manager;
	int pdo_num = manager->origin_selected_pdo_num;

	pr_info("[%s] is_attached = %d, ori_pdo_num = %d\n",
			__func__, pdic_data->is_attached, pdo_num);
	if (pdic_data->is_attached && pdic_data->pd_support &&
		(pd_data->available_pdo_num > 1)) {
		if ((pdo_num > 1) || ((pdo_num == 1) &&
			(pd_data->power_list[2].max_voltage < 10000))) {
			sm5038_select_pdo(2);
		}
	}
}

void sm5038_usbpd_power_ready(struct device *dev,
	enum usbpd_power_role power_role)
{
	struct sm5038_usbpd_data *pd_data = dev_get_drvdata(dev);
	struct sm5038_policy_data *policy = &pd_data->policy;
	struct sm5038_phydrv_data *pdic_data = pd_data->phy_driver_data;
	struct sm5038_usbpd_manager_data *manager = &pd_data->manager;
	union power_supply_propval val = {0, };
#if defined(CONFIG_SM5038_SHORT_PROTECTION)
	bool short_cable = false;
#endif
	if (!pdic_data->pd_support) {
#if defined(CONFIG_SM5038_SHORT_PROTECTION)
		pd_data->phy_ops.get_short_state(pd_data, &short_cable);
		if (short_cable) {
			pd_data->available_pdo_num = 1;
			pd_data->power_list[1].max_current =
				pd_data->power_list[1].max_current > 1800 ?
				1800 : pd_data->power_list[1].max_current;
		}
#endif
		pdic_data->pd_support = 1;
		pr_info("%s : pd_support : %d, available_pdo : %d\n",
				__func__, pdic_data->pd_support,
				pd_data->available_pdo_num);
		typec_set_pwr_opmode(pdic_data->port, TYPEC_PWR_MODE_PD);
	}

	if (power_role == USBPD_SINK &&
			(policy->last_state == PE_SNK_Transition_Sink ||
			policy->last_state == PE_SNK_Get_Source_Cap_Ext)) {
		manager->origin_selected_pdo_num = pd_data->selected_pdo_num;
		val.intval = POWER_SUPPLY_TYPE_USB_PD;
		sm5038_charger_psy_changed(POWER_SUPPLY_PROP_ONLINE, val);
		if (pd_data->power_list[pd_data->selected_pdo_num].apdo) {
			val.intval = pd_data->request_pps_cur * 1000;
		} else
			val.intval = pd_data->sink_rdo.request_data_object.op_current * 1000;
//		sm5038_charger_psy_changed(POWER_SUPPLY_PROP_PD_CURRENT_MAX, val);
		sm5038_usbpd_uevent_notifier(&pd_data->dev);
		power_supply_changed(pdic_data->psy_usbpd);
	} else if (power_role == USBPD_SOURCE &&
			policy->last_state == PE_SRC_Transition_Supply)
		sm5038_usbpd_uevent_notifier(&pd_data->dev);
}
EXPORT_SYMBOL_GPL(sm5038_usbpd_power_ready);

static int sm5038_usbpd_command_to_policy(struct device *dev,
		sm5038_usbpd_manager_command_type command)
{
	struct sm5038_usbpd_data *pd_data = dev_get_drvdata(dev);
	struct sm5038_usbpd_manager_data *manager = &pd_data->manager;

	manager->cmd = command;

	sm5038_usbpd_kick_policy_work(dev);

	return 0;
}

void sm5038_usbpd_inform_event(struct sm5038_usbpd_data *pd_data,
		sm5038_usbpd_manager_event_type event)
{
	struct sm5038_usbpd_manager_data *manager = &pd_data->manager;

	manager->event = event;

	switch (event) {
	case MANAGER_SEND_DISCOVER_IDENTITY:
		sm5038_usbpd_command_to_policy(&pd_data->dev,
					MANAGER_REQ_VDM_DISCOVER_IDENTITY);
		break;
	case MANAGER_DISCOVER_IDENTITY_ACKED:
		sm5038_usbpd_get_identity(pd_data);
		sm5038_usbpd_command_to_policy(&pd_data->dev,
					MANAGER_REQ_VDM_DISCOVER_SVID);
		break;
	case MANAGER_DISCOVER_SVID_ACKED:
		sm5038_usbpd_get_svids(pd_data);
		sm5038_usbpd_command_to_policy(&pd_data->dev,
					MANAGER_REQ_VDM_DISCOVER_MODE);
		break;
	case MANAGER_DISCOVER_MODE_ACKED:
		sm5038_usbpd_get_modes(pd_data);
		sm5038_usbpd_command_to_policy(&pd_data->dev,
					MANAGER_REQ_VDM_ENTER_MODE);
		break;
	case MANAGER_ENTER_MODE_ACKED:
		sm5038_usbpd_enter_mode(pd_data);
		if (manager->SVID_0 == PD_SID_1)
			sm5038_usbpd_command_to_policy(&pd_data->dev,
					MANAGER_REQ_VDM_STATUS_UPDATE);
		break;
	case MANAGER_STATUS_UPDATE_ACKED:
		sm5038_usbpd_command_to_policy(&pd_data->dev,
					MANAGER_REQ_VDM_DisplayPort_Configure);
		break;
	case MANAGER_DisplayPort_Configure_ACKED:
		break;
	case MANAGER_NEW_POWER_SRC:
		sm5038_usbpd_command_to_policy(&pd_data->dev,
					MANAGER_REQ_NEW_POWER_SRC);
		break;
	case MANAGER_PR_SWAP_REQUEST:
		sm5038_usbpd_command_to_policy(&pd_data->dev,
					MANAGER_REQ_PR_SWAP);
		break;
	case MANAGER_DR_SWAP_REQUEST:
		sm5038_usbpd_command_to_policy(&pd_data->dev,
					MANAGER_REQ_DR_SWAP);
		break;
	default:
		pr_info("%s: not matched event(%d)\n", __func__, event);
	}
}
EXPORT_SYMBOL_GPL(sm5038_usbpd_inform_event);

bool sm5038_usbpd_vdm_request_enabled(struct sm5038_usbpd_data *pd_data)
{
	struct sm5038_usbpd_manager_data *manager = &pd_data->manager;

	if (manager->alt_sended == 1 && manager->vdm_en == 1)
		return true;

	manager->vdm_en = 1;

	schedule_delayed_work(&manager->start_discover_msg_handler, msecs_to_jiffies(tDiscoverIdentity));
	return true;
}
EXPORT_SYMBOL_GPL(sm5038_usbpd_vdm_request_enabled);

#if 0
bool sm5038_usbpd_ext_request_enabled(struct sm5038_usbpd_data *pd_data)
{
	struct sm5038_usbpd_manager_data *manager = &pd_data->manager;
	struct sm5038_phydrv_data *pdic_data = pd_data->phy_driver_data;
	bool ret = false;

	if ((pdic_data->rp_currentlvl == RP_CURRENT_LEVEL3) &&
			(pd_data->specification_revision == USBPD_REV_30)) {
		if (manager->ext_sended)
			ret = false;
		else {
			manager->ext_sended = 1;
			ret = true;
		}
	}
	pr_info("%s: rp_currentlvl(%d), %s\n", __func__, pdic_data->rp_currentlvl,
			pdic_data->rp_currentlvl == RP_CURRENT_LEVEL3 ? "SINK TX OK" : "SINK TX NG");
	return ret;
}
#endif

bool sm5038_usbpd_power_role_swap(struct sm5038_usbpd_data *pd_data)
{
	struct sm5038_usbpd_manager_data *manager = &pd_data->manager;

	return manager->power_role_swap;
}
EXPORT_SYMBOL_GPL(sm5038_usbpd_power_role_swap);

bool sm5038_usbpd_vconn_source_swap(struct sm5038_usbpd_data *pd_data)
{
	struct sm5038_usbpd_manager_data *manager = &pd_data->manager;

	return manager->vconn_source_swap;
}
EXPORT_SYMBOL_GPL(sm5038_usbpd_vconn_source_swap);

void sm5038_usbpd_turn_on_source(struct sm5038_usbpd_data *pd_data)
{
	struct sm5038_phydrv_data *pdic_data = pd_data->phy_driver_data;

	pr_info("%s\n", __func__);

	sm5038_vbus_turn_on_ctrl(pdic_data, 1);
}
EXPORT_SYMBOL_GPL(sm5038_usbpd_turn_on_source);

void sm5038_usbpd_turn_off_power_supply(struct sm5038_usbpd_data *pd_data)
{
	struct sm5038_phydrv_data *pdic_data = pd_data->phy_driver_data;

	pr_info("%s\n", __func__);

	sm5038_vbus_turn_on_ctrl(pdic_data, 0);
	sm5038_usbpd_set_vbus_dischg_gpio(pdic_data, 1);
}
EXPORT_SYMBOL_GPL(sm5038_usbpd_turn_off_power_supply);

void sm5038_usbpd_turn_off_power_sink(struct sm5038_usbpd_data *pd_data)
{
	pd_data->selected_pdo_num = 0;
	pd_data->available_pdo_num = 0;
	pd_data->current_pdo_num = 0;
	pr_info("%s To do charging off?\n", __func__);
}
EXPORT_SYMBOL_GPL(sm5038_usbpd_turn_off_power_sink);

bool sm5038_usbpd_data_role_swap(struct sm5038_usbpd_data *pd_data)
{
	struct sm5038_usbpd_manager_data *manager = &pd_data->manager;
	struct sm5038_phydrv_data *pdic_data = pd_data->phy_driver_data;

	pr_info("%s - %s, %s, data_role_swap : %d\n", __func__,
		pdic_data->typec_power_role == TYPEC_DEVICE ? "ufp":"dfp",
		pdic_data->typec_data_role == TYPEC_SINK ? "snk":"src",
		manager->data_role_swap);

	return manager->data_role_swap;
}
EXPORT_SYMBOL_GPL(sm5038_usbpd_data_role_swap);

int sm5038_usbpd_get_identity(struct sm5038_usbpd_data *pd_data)
{
	struct sm5038_policy_data *policy = &pd_data->policy;
	struct sm5038_usbpd_manager_data *manager = &pd_data->manager;

	manager->Vendor_ID = policy->rx_data_obj[1].id_header.usb_vendor_id;
	manager->Product_ID = policy->rx_data_obj[3].product_vdo.product_id;
	manager->Device_Version =
		policy->rx_data_obj[3].product_vdo.device_version;

	pr_info("%s, Vendor_ID : 0x%x, Product_ID : 0x%x, Device Version : 0x%x\n",
		__func__, manager->Vendor_ID, manager->Product_ID,
		manager->Device_Version);
	return 0;
}

int sm5038_usbpd_get_svids(struct sm5038_usbpd_data *pd_data)
{
	struct sm5038_policy_data *policy = &pd_data->policy;
	struct sm5038_usbpd_manager_data *manager = &pd_data->manager;
	int i = 0, vdo_num = 0;

	vdo_num = policy->rx_msg_header.num_data_objs - 1;
	manager->SVID_DP = 0;
	manager->SVID_0 = policy->rx_data_obj[1].vdm_svid.svid_0;
	manager->SVID_1 = policy->rx_data_obj[1].vdm_svid.svid_1;

	for (i = 0; i < vdo_num; i++) {
		if (policy->rx_data_obj[i+1].vdm_svid.svid_0 == PD_SID_1) {
			manager->SVID_0 = policy->rx_data_obj[i+1].vdm_svid.svid_0;
			pr_info("%s, SVID_%d : 0x%x\n", __func__, (i * 2) + 1, manager->SVID_0);
			break;
		} else if (policy->rx_data_obj[i+1].vdm_svid.svid_1 == PD_SID_1) {
			manager->SVID_0 = policy->rx_data_obj[i+1].vdm_svid.svid_1;
			pr_info("%s, SVID_%d : 0x%x\n", __func__, (i * 2) + 2, manager->SVID_0);
			break;
		}
	}
	if ((manager->SVID_0 != PD_SID_1) && (manager->SVID_1 != PD_SID_1)) {
		pr_info("%s, No have availible SVIDs.\n", __func__);
		manager->SVID_0 = PD_SID_1;
	}

	if (manager->SVID_0 == PD_SID_1)
		manager->SVID_DP = PD_SID_1;
	pr_info("%s, SVID_0 : 0x%x, SVID_1 : 0x%x, SVID_DP : 0x%x\n", __func__,
		manager->SVID_0, manager->SVID_1, manager->SVID_DP);

	if (manager->SVID_DP == PD_SID_1) {
		manager->dp_is_connect = 1;
		/* If you want to support USB SuperSpeed when you connect
		 * Display port dongle, You should change dp_hs_connect depend
		 * on Pin assignment.If DP use 4lane(Pin Assignment C,E,A),
		 * dp_hs_connect is 1. USB can support HS.If DP use
		 * 2lane(Pin Assignment B,D,F), dp_hs_connect is 0. USB
		 * can support SS
		 */
		manager->dp_hs_connect = 1;
		/* notify to dp event */
	}
	return 0;
}

int sm5038_usbpd_get_modes(struct sm5038_usbpd_data *pd_data)
{
	struct sm5038_policy_data *policy = &pd_data->policy;
	struct sm5038_usbpd_manager_data *manager = &pd_data->manager;

	manager->Standard_Vendor_ID =
			policy->rx_data_obj[0].structured_vdm.svid;

	pr_info("%s, Standard_Vendor_ID = 0x%x\n", __func__,
		manager->Standard_Vendor_ID);

	return 0;
}

int sm5038_usbpd_enter_mode(struct sm5038_usbpd_data *pd_data)
{
	struct sm5038_policy_data *policy = &pd_data->policy;
	struct sm5038_usbpd_manager_data *manager = &pd_data->manager;

	manager->Standard_Vendor_ID =
		policy->rx_data_obj[0].structured_vdm.svid;

	return 0;
}

int sm5038_usbpd_exit_mode(struct sm5038_usbpd_data *pd_data, unsigned int mode)
{
	return 0;
}

data_obj_type sm5038_usbpd_select_capability(struct sm5038_usbpd_data *pd_data)
{
	data_obj_type obj;

	int pdo_num = pd_data->selected_pdo_num;
	int output_vol = 0;
	int op_curr = 0;

	if (pd_data->power_list[pdo_num].apdo) {
		output_vol = (pd_data->request_pps_vol / USBPD_OUT_VOLT_UNIT);
		op_curr = (pd_data->request_pps_cur / USBPD_PPS_CURRENT_UNIT);
		obj.request_data_object_programmable.op_current = op_curr;
		obj.request_data_object_programmable.reserved1 = 0;
		obj.request_data_object_programmable.output_voltage = output_vol;
		obj.request_data_object_programmable.reserved2 = 0;
		obj.request_data_object_programmable.unchunked_ext_msg_support = 0;
		obj.request_data_object_programmable.no_usb_suspend = 1;
		obj.request_data_object_programmable.usb_comm_capable = 1;
		obj.request_data_object_programmable.capability_mismatch = 0;
		obj.request_data_object_programmable.reserved3 = 0;
		obj.request_data_object_programmable.object_position = pd_data->selected_pdo_num;
		obj.request_data_object_programmable.reserved4 = 0;
	} else {
		obj.request_data_object.no_usb_suspend = 1;
		obj.request_data_object.usb_comm_capable = 1;
		obj.request_data_object.capability_mismatch = 0;
		obj.request_data_object.give_back = 0;
		obj.request_data_object.min_current =
			pd_data->power_list[pdo_num].max_current / USBPD_CURRENT_UNIT;
		obj.request_data_object.op_current =
			pd_data->power_list[pdo_num].max_current / USBPD_CURRENT_UNIT;
		obj.request_data_object.object_position = pd_data->selected_pdo_num;
	}
	pd_data->sink_rdo.object = obj.object;

	return obj;
}
EXPORT_SYMBOL_GPL(sm5038_usbpd_select_capability);

int sm5038_usbpd_evaluate_capability(struct sm5038_usbpd_data *pd_data)
{
	struct sm5038_policy_data *policy = &pd_data->policy;
#if defined(CONFIG_SM5038_SHORT_PROTECTION)
	struct sm5038_phydrv_data *pdic_data = pd_data->phy_driver_data;
#endif
	struct sm5038_usbpd_manager_data *manager = &pd_data->manager;
	int i = 0;
	int power_type = 0;
	int pd_volt = 0, pd_current;
	int available_pdo_num = 0;
	data_obj_type *pd_obj;
	int min_volt = 0, max_volt = 0, max_current = 0;

	pd_data->specification_revision = USBPD_REV_20;

	for (i = 0; i < policy->rx_msg_header.num_data_objs; i++) {
		pd_obj = &policy->rx_data_obj[i];
		power_type = pd_obj->power_data_obj_supply_type.supply_type;
		pd_data->received_pdos[i] = policy->rx_data_obj[i].object;

		switch (power_type) {
		case POWER_TYPE_FIXED:
			pd_volt = pd_obj->power_data_obj.voltage;
			pd_current = pd_obj->power_data_obj.max_current;
			dev_info(&pd_data->dev, "[%d] FIXED volt(%d)mV, max current(%d)\n",
				i+1, pd_volt * USBPD_VOLT_UNIT,
				pd_current * USBPD_CURRENT_UNIT);
			available_pdo_num = i + 1;
			pd_data->power_list[i + 1].max_voltage =
						pd_volt * USBPD_VOLT_UNIT;
			pd_data->power_list[i + 1].max_current =
						pd_current * USBPD_CURRENT_UNIT;

			pd_data->power_list[i + 1].min_voltage = 0;
			pd_data->power_list[i + 1].apdo = false;
			break;
		case POWER_TYPE_BATTERY:
			pd_volt = pd_obj->power_data_obj_battery.max_voltage;
			dev_info(&pd_data->dev, "[%d] BATTERY volt(%d)mV\n",
						i+1, pd_volt * USBPD_VOLT_UNIT);
			break;
		case POWER_TYPE_VARIABLE:
			pd_volt = pd_obj->power_data_obj_variable.max_voltage;
			dev_info(&pd_data->dev, "[%d] VARIABLE volt(%d)mV\n",
						i+1, pd_volt * USBPD_VOLT_UNIT);
			break;
		case POWER_TYPE_APDO:
			min_volt = pd_obj->power_data_obj_programmable.min_voltage;
			max_volt = pd_obj->power_data_obj_programmable.max_voltage;
			max_current = pd_obj->power_data_obj_programmable.max_current;

			pd_data->specification_revision = USBPD_REV_30;

			dev_info(&pd_data->dev, "[%d] Augmented min_volt(%d)mV, max_volt (%d)mV, max_current(%d)mA\n",
					i+1, min_volt * USBPD_PPS_VOLT_UNIT,
					max_volt * USBPD_PPS_VOLT_UNIT,
					max_current * USBPD_PPS_CURRENT_UNIT);
			if (pd_data->specification_revision == USBPD_REV_30) {
				available_pdo_num = i + 1;
				pd_data->power_list[i + 1].max_voltage = max_volt * USBPD_PPS_VOLT_UNIT;
				pd_data->power_list[i + 1].min_voltage = min_volt * USBPD_PPS_VOLT_UNIT;
				pd_data->power_list[i + 1].max_current = max_current * USBPD_PPS_CURRENT_UNIT;
				pd_data->power_list[i + 1].apdo = true;
			} else
				pd_data->received_pdos[i] = 0;
			break;
		default:
			dev_err(&pd_data->dev, "[%d] Power Type Error\n", i+1);
			break;
		}
	}

#if defined(CONFIG_SM5038_SHORT_PROTECTION)
	if (pdic_data->rp_currentlvl == RP_CURRENT_ABNORMAL) {
		available_pdo_num = 1;
		pd_data->power_list[1].max_current =
				pd_data->power_list[1].max_current > 1800 ?
				1800 : pd_data->power_list[1].max_current;
		dev_info(&pd_data->dev, "Fixed max_current to 1.8A because of vbus short\n");
	}
#endif
	if ((pd_data->available_pdo_num > 0) &&
			(pd_data->available_pdo_num != available_pdo_num))
		pd_data->selected_pdo_num = 1;
	pd_data->available_pdo_num = available_pdo_num;

	if (manager->fled_torch_enable || manager->fled_flash_enable) {
		pr_info(" %s : PDO(%d) is ignored because of [torch(%d) or flash(%d)]\n",
				__func__, available_pdo_num, manager->fled_torch_enable, manager->fled_flash_enable);
	}
	if (available_pdo_num)
		pd_data->src_cap_id++;
	return available_pdo_num;
}
EXPORT_SYMBOL_GPL(sm5038_usbpd_evaluate_capability);

int sm5038_usbpd_match_request(struct sm5038_usbpd_data *pd_data)
{
	unsigned int supply_type
	= pd_data->source_request_obj.power_data_obj_supply_type.supply_type;
	unsigned int mismatch, max_min, op, pos;

	if (supply_type == POWER_TYPE_FIXED) {
		pr_info("REQUEST: FIXED\n");
		goto log_fixed_variable;
	} else if (supply_type == POWER_TYPE_VARIABLE) {
		pr_info("REQUEST: VARIABLE\n");
		goto log_fixed_variable;
	} else if (supply_type == POWER_TYPE_BATTERY) {
		pr_info("REQUEST: BATTERY\n");
		goto log_battery;
	} else {
		pr_info("REQUEST: UNKNOWN Supply type.\n");
		return -EPERM;
	}

log_fixed_variable:
	mismatch =
		pd_data->source_request_obj.request_data_object.capability_mismatch;
	max_min = pd_data->source_request_obj.request_data_object.min_current;
	op = pd_data->source_request_obj.request_data_object.op_current;
	pos = pd_data->source_request_obj.request_data_object.object_position;
	pr_info("Obj position: %d\n", pos);
	pr_info("Mismatch: %d\n", mismatch);
	pr_info("Operating Current: %d mA\n", op*10);
	if (pd_data->source_request_obj.request_data_object.give_back)
		pr_info("Min current: %d mA\n", max_min*10);
	else
		pr_info("Max current: %d mA\n", max_min*10);

	if ((pos > pd_data->source_msg_header.num_data_objs) ||
			(op > pd_data->source_data_obj.power_data_obj.max_current)) {
		pr_info("Invalid Request Message.\n");
		return -EPERM;
	}
	return 0;

log_battery:
	mismatch =
		pd_data->source_request_obj.request_data_object_battery.capability_mismatch;
	return 0;
}
EXPORT_SYMBOL_GPL(sm5038_usbpd_match_request);

static void sm5038_usbpd_read_ext_msg(struct sm5038_usbpd_data *pd_data)
{
	int i = 0, j = 0, k = 0, l = 1, obj_num = 0;
	unsigned short vid = 0, pid = 0, xid = 0;

	pd_data->policy.rx_msg_header.word
		= pd_data->protocol_rx.msg_header.word;
	pd_data->policy.rx_msg_ext_header.word
		= pd_data->protocol_rx.data_obj[0].word[0];
	obj_num = pd_data->policy.rx_msg_header.num_data_objs;

	for (i = 0; i < (obj_num * 4); i++) {
		if ((i != 0) && (i % 4 == 0))
			j++;
		if (i == 0)
			pd_data->policy.rx_data_obj[0].byte[0]
				= pd_data->protocol_rx.data_obj[j].byte[2];
		else if (i == 1)
			pd_data->policy.rx_data_obj[0].byte[1]
				= pd_data->protocol_rx.data_obj[j].byte[3];
		else {
			if ((k != 0) && (k % 4 == 0))
				l++;
			pd_data->policy.rx_data_obj[j].byte[i % 4]
				= pd_data->protocol_rx.data_obj[l].byte[k % 4];
			k++;
		}
	}

	vid = pd_data->policy.rx_data_obj[0].source_capabilities_extended_data1.VID;
	pid = pd_data->policy.rx_data_obj[0].source_capabilities_extended_data1.PID;
	xid = pd_data->policy.rx_data_obj[1].source_capabilities_extended_data2.XID;
	pr_info("%s : VID = 0x%x   PID = 0x%x  XID = 0x%x\n", __func__, vid, pid, xid);
}

static void sm5038_usbpd_read_msg(struct sm5038_usbpd_data *pd_data)
{
	int i;

	pd_data->policy.rx_msg_header.word
		= pd_data->protocol_rx.msg_header.word;
	for (i = 0; i < USBPD_MAX_COUNT_MSG_OBJECT; i++) {
		pd_data->policy.rx_data_obj[i].object
			= pd_data->protocol_rx.data_obj[i].object;
	}
}

static void sm5038_usbpd_protocol_tx(struct sm5038_usbpd_data *pd_data)
{
	struct sm5038_protocol_data *tx = &pd_data->protocol_tx;

	tx->status = DEFAULT_PROTOCOL_NONE;

	if (pd_data->phy_ops.tx_msg(pd_data, &tx->msg_header, tx->data_obj))
		dev_err(&pd_data->dev, "%s error\n", __func__);
	dev_info(&pd_data->dev, "%s: [Tx] [0x%x] [0x%x]\n", __func__,
			tx->msg_header.word, tx->data_obj[0].object);
	tx->msg_header.word = 0;
}

/* return 1: sent with goodcrc, 0: fail */
bool sm5038_usbpd_send_msg(struct sm5038_usbpd_data *pd_data, msg_header *header,
		data_obj_type *obj)
{
	int i;

	if (obj)
		for (i = 0; i < USBPD_MAX_COUNT_MSG_OBJECT; i++)
			pd_data->protocol_tx.data_obj[i].object = obj[i].object;
	else
		header->num_data_objs = 0;

	header->spec_revision = pd_data->specification_revision;
	pd_data->protocol_tx.msg_header.word = header->word;
	sm5038_usbpd_protocol_tx(pd_data);

	return true;
}
EXPORT_SYMBOL_GPL(sm5038_usbpd_send_msg);

inline bool sm5038_usbpd_send_ctrl_msg(struct sm5038_usbpd_data *d, msg_header *h,
		unsigned int msg, unsigned int dr, unsigned int pr)
{
	h->msg_type = msg;
	h->port_data_role = dr;
	h->port_power_role = pr;
	h->num_data_objs = 0;
	return sm5038_usbpd_send_msg(d, h, 0);
}
EXPORT_SYMBOL_GPL(sm5038_usbpd_send_ctrl_msg);

/* return: 0 if timed out, positive is status */
inline unsigned int sm5038_usbpd_wait_msg(struct sm5038_usbpd_data *pd_data,
				unsigned int msg_status, unsigned int ms)
{
	unsigned long ret;

	ret = pd_data->phy_ops.get_status(pd_data, msg_status);
	if (ret) {
		pd_data->policy.abnormal_state = false;
		return ret;
	}
	dev_info(&pd_data->dev,
		"%s: msg_status = %d, time = %d\n", __func__, msg_status, ms);
	/* wait */
	reinit_completion(&pd_data->msg_arrived);
	pd_data->wait_for_msg_arrived = msg_status;
	ret = wait_for_completion_timeout(&pd_data->msg_arrived,
						msecs_to_jiffies(ms));

	if (!pd_data->policy.state) {
		dev_err(&pd_data->dev,
			"%s : return for policy state error\n", __func__);
		pd_data->policy.abnormal_state = true;
		return 0;
	}

	pd_data->policy.abnormal_state = false;

	return pd_data->phy_ops.get_status(pd_data, msg_status);
}
EXPORT_SYMBOL_GPL(sm5038_usbpd_wait_msg);

static void sm5038_usbpd_check_vdm(struct sm5038_usbpd_data *pd_data)
{
	struct sm5038_phydrv_data *pdic_data = pd_data->phy_driver_data;
	unsigned int cmd, cmd_type, vdm_type;

	cmd = pd_data->policy.rx_data_obj[0].structured_vdm.command;
	cmd_type = pd_data->policy.rx_data_obj[0].structured_vdm.command_type;
	vdm_type = pd_data->policy.rx_data_obj[0].structured_vdm.vdm_type;

	dev_info(&pd_data->dev, "%s: cmd = %x, cmd_type = %x, vdm_type = %x\n",
			__func__, cmd, cmd_type, vdm_type);

	/* TD.PD.VDMU.E17 Interruption by VDM Command */
	if (sm5038_get_rx_buf_st(pd_data) && ((cmd == Discover_Identity) ||
			(cmd == Discover_SVIDs) || (cmd == Discover_Modes) ||
			(cmd == Enter_Mode) || (cmd == Exit_Mode)))
		return;

	if (vdm_type == Unstructured_VDM) {
		pdic_data->status_reg |= UVDM_MSG;
		return;
	}

	if (cmd_type == Initiator) {
		switch (cmd) {
		case Discover_Identity:
			pdic_data->status_reg |= VDM_DISCOVER_IDENTITY;
			break;
		case Discover_SVIDs:
			pdic_data->status_reg |= VDM_DISCOVER_SVID;
			break;
		case Discover_Modes:
			pdic_data->status_reg |= VDM_DISCOVER_MODE;
			break;
		case Enter_Mode:
			pdic_data->status_reg |= VDM_ENTER_MODE;
			break;
		case Exit_Mode:
			pdic_data->status_reg |= VDM_EXIT_MODE;
			break;
		case Attention:
			pdic_data->status_reg |= VDM_ATTENTION;
			break;
		case DisplayPort_Status_Update:
			pdic_data->status_reg |= MSG_PASS;
			break;
		case DisplayPort_Configure:
			pdic_data->status_reg |= MSG_PASS;
			break;
		}
	} else if (cmd_type == Responder_ACK) {
		switch (cmd) {
		case Discover_Identity:
			pdic_data->status_reg |= VDM_DISCOVER_IDENTITY;
			break;
		case Discover_SVIDs:
			pdic_data->status_reg |= VDM_DISCOVER_SVID;
			break;
		case Discover_Modes:
			pdic_data->status_reg |= VDM_DISCOVER_MODE;
			break;
		case Enter_Mode:
			pdic_data->status_reg |= VDM_ENTER_MODE;
			break;
		case Exit_Mode:
			pdic_data->status_reg |= VDM_EXIT_MODE;
			break;
		case DisplayPort_Status_Update:
			pdic_data->status_reg |= VDM_DP_STATUS_UPDATE;
			break;
		case DisplayPort_Configure:
			pdic_data->status_reg |= VDM_DP_CONFIGURE;
			break;
		}
	}
}

void sm5038_usbpd_protocol_rx(struct sm5038_usbpd_data *pd_data)
{
	struct sm5038_protocol_data *rx = &pd_data->protocol_rx;
	struct sm5038_phydrv_data *pdic_data = pd_data->phy_driver_data;
	u8 ext_msg;

	if (pd_data->phy_ops.rx_msg(pd_data, &rx->msg_header, rx->data_obj)) {
		dev_err(&pd_data->dev, "%s IO Error\n", __func__);
		return;
	}
	dev_info(&pd_data->dev, "%s: stored_message_id = %x, msg_id = %d\n",
			__func__, rx->stored_message_id, rx->msg_header.msg_id);
	if (rx->msg_header.word == 0) {
		dev_err(&pd_data->dev, "[Rx] No Message.\n");
		return; /* no message */
	} else if (rx->msg_header.msg_type == USBPD_Soft_Reset) {
		pr_info("%s : Got SOFT_RESET.\n", __func__);
		pdic_data->status_reg |= MSG_SOFTRESET;
		return;
	}
	dev_info(&pd_data->dev, "%s: [Rx] [0x%x] [0x%x]\n",
			__func__, rx->msg_header.word, rx->data_obj[0].object);

	if (rx->stored_message_id != rx->msg_header.msg_id) {
		if (pd_data->policy.origin_message == 0x0)
			rx->stored_message_id = rx->msg_header.msg_id;

		ext_msg = pd_data->protocol_rx.msg_header.byte[1] & 0x80;

		if (ext_msg)
			sm5038_usbpd_read_ext_msg(pd_data);
		else
			sm5038_usbpd_read_msg(pd_data);

		dev_info(&pd_data->dev, "%s: ext_msg = %x, obj_num = %d, msg_type = %d\n",
			__func__, ext_msg, pd_data->policy.rx_msg_header.num_data_objs,
					pd_data->policy.rx_msg_header.msg_type);

		if (ext_msg && pd_data->policy.rx_msg_header.spec_revision == 0) {
			return;
		} else if (ext_msg && pd_data->policy.rx_msg_header.spec_revision == USBPD_REV_30) {
			switch (pd_data->policy.rx_msg_header.msg_type) {
			pr_info("%s : Chunked = %d, Chunk Number = %d, Request Chunk = %d, Data Size = %d\n",
					__func__, pd_data->policy.rx_msg_ext_header.chunked, pd_data->policy.rx_msg_ext_header.chunk_number,
					pd_data->policy.rx_msg_ext_header.request_chunk, pd_data->policy.rx_msg_ext_header.data_size);
			case USBPD_Source_Cap_Ext:
				break;
			case USBPD_Status:
				break;
			case USBPD_Get_Battery_Cap:
				break;
			case USBPD_Get_Batt_Status:
				break;
			case USBPD_Battery_Cap:
				break;
			case USBPD_Get_Manuf_Info:
				break;
			case USBPD_Manuf_Info:
				break;
			case USBPD_Security_Request:
				break;
			case USBPD_Security_Response:
				break;
			case USBPD_FW_Update_Req:
				break;
			case USBPD_FW_Update_Res:
				break;
			case USBPD_PPS_Status:
				break;
			case USBPD_Country_Info:
				break;
			case USBPD_Country_Codes:
				break;
			case USBPD_Sink_Cap_Ext:
				break;
			default:
				break;
			}
		} else if (pd_data->policy.rx_msg_header.num_data_objs > 0) {
			switch (pd_data->policy.rx_msg_header.msg_type) {
			case USBPD_Source_Capabilities:
				pdic_data->status_reg |= MSG_SRC_CAP;
				break;
			case USBPD_Request:
				pdic_data->status_reg |= MSG_REQUEST;
				break;
			case USBPD_BIST:
				if (pd_data->policy.state == PE_SNK_Ready ||
						pd_data->policy.state == PE_SRC_Ready) {
					if (pd_data->policy.rx_data_obj[0].bist_data_object.bist_mode ==
							BIST_Carrier_Mode2) {
						pdic_data->status_reg |= MSG_BIST_M2;
					} else if (pd_data->policy.rx_data_obj[0].bist_data_object.bist_mode ==
							BIST_Test_Mode) {
						pdic_data->status_reg |= MSG_NONE;
					} else {
						/* Not Support */
					}
				}
				break;
			case USBPD_Sink_Capabilities:
				pdic_data->status_reg |= MSG_SNK_CAP;
				break;
			case USBPD_Battery_Status:
				break;
			case USBPD_Alert:
				if (pd_data->policy.rx_data_obj->alert_data_obejct.type_of_alert == Battery_Status_Change)
					pdic_data->status_reg |= MSG_GET_BAT_STATUS;
				else
					pdic_data->status_reg |= MSG_GET_STATUS;
				break;
			case USBPD_Vendor_Defined:
				sm5038_usbpd_check_vdm(pd_data);
				break;
			default:
				break;
			}
		} else {
			switch (pd_data->policy.rx_msg_header.msg_type) {
			case USBPD_GoodCRC:
				/* Do nothing */
				break;
			case USBPD_Ping:
				/* Do nothing */
				break;
			case USBPD_GotoMin:
				if (pd_data->policy.state == PE_SNK_Ready)
					pd_data->policy.state =
						PE_SNK_Transition_Sink;
				break;
			case USBPD_Accept:
				pdic_data->status_reg |= MSG_ACCEPT;
				break;
			case USBPD_Reject:
				pdic_data->status_reg |= MSG_REJECT;
				break;
			case USBPD_PS_RDY:
				pdic_data->status_reg |= MSG_PSRDY;
				break;
			case USBPD_Get_Source_Cap:
				pdic_data->status_reg |= MSG_GET_SRC_CAP;
				break;
			case USBPD_Get_Sink_Cap:
				pdic_data->status_reg |= MSG_GET_SNK_CAP;
				break;
			case USBPD_DR_Swap:
				pdic_data->status_reg |= MSG_DR_SWAP;
				break;
			case USBPD_PR_Swap:
				pdic_data->status_reg |= MSG_PR_SWAP;
				break;
			case USBPD_VCONN_Swap:
				pdic_data->status_reg |= MSG_VCONN_SWAP;
				break;
			case USBPD_Wait:
				pdic_data->status_reg |= MSG_WAIT;
				break;
			case USBPD_Soft_Reset:
				pdic_data->status_reg |= MSG_SOFTRESET;
				break;
			case USBPD_Not_Supported:
				break;
			case USBPD_Get_Src_Cap_Ext:
				break;
			case USBPD_Get_Status:
				break;
			case USBPD_FR_Swap:
				break;
			case USBPD_Get_PPS_Status:
				break;
			case USBPD_Get_Country_Codes:
				break;
			case USBPD_Get_Sink_Cap_Ext:
				break;
			default:
				break;
			}
		}
	}
}
EXPORT_SYMBOL_GPL(sm5038_usbpd_protocol_rx);

void sm5038_usbpd_rx_hard_reset(struct device *dev)
{
	struct sm5038_usbpd_data *pd_data = dev_get_drvdata(dev);

	sm5038_usbpd_reinit(dev);
	sm5038_usbpd_policy_reset(pd_data, HARDRESET_RECEIVED);
}
EXPORT_SYMBOL_GPL(sm5038_usbpd_rx_hard_reset);

void sm5038_usbpd_rx_soft_reset(struct sm5038_usbpd_data *pd_data)
{
	sm5038_usbpd_reinit(&pd_data->dev);
	sm5038_usbpd_policy_reset(pd_data, SOFTRESET_RECEIVED);
}
EXPORT_SYMBOL_GPL(sm5038_usbpd_rx_soft_reset);

void sm5038_usbpd_set_ops(struct device *dev, usbpd_phy_ops_type *ops)
{
	struct sm5038_usbpd_data *pd_data = (struct sm5038_usbpd_data *) dev_get_drvdata(dev);

	pd_data->phy_ops.tx_msg = ops->tx_msg;
	pd_data->phy_ops.rx_msg = ops->rx_msg;
	pd_data->phy_ops.hard_reset = ops->hard_reset;
	pd_data->phy_ops.set_power_role = ops->set_power_role;
	pd_data->phy_ops.get_power_role = ops->get_power_role;
	pd_data->phy_ops.set_data_role = ops->set_data_role;
	pd_data->phy_ops.get_data_role = ops->get_data_role;
	pd_data->phy_ops.get_vconn_source = ops->get_vconn_source;
	pd_data->phy_ops.set_vconn_source = ops->set_vconn_source;
	pd_data->phy_ops.set_check_msg_pass = ops->set_check_msg_pass;
	pd_data->phy_ops.get_status = ops->get_status;
	pd_data->phy_ops.poll_status = ops->poll_status;
	pd_data->phy_ops.driver_reset = ops->driver_reset;
	pd_data->phy_ops.get_short_state = ops->get_short_state;
}
EXPORT_SYMBOL_GPL(sm5038_usbpd_set_ops);

static void sm5038_usbpd_init_source_cap_data(struct sm5038_usbpd_manager_data *_data)
{
	msg_header *msg_header = &_data->pd_data->source_msg_header;
	data_obj_type *data_obj = &_data->pd_data->source_data_obj;

	msg_header->msg_type = USBPD_Source_Capabilities;
	msg_header->port_data_role = TYPEC_HOST;
	msg_header->spec_revision = 1;
	msg_header->port_power_role = TYPEC_SOURCE;
	msg_header->num_data_objs = 1;

	data_obj->power_data_obj.max_current = 900 / 10;
	data_obj->power_data_obj.voltage = 5000 / 50;
	data_obj->power_data_obj.supply = POWER_TYPE_FIXED;
	data_obj->power_data_obj.data_role_swap = 1;
	data_obj->power_data_obj.dual_role_power = 1;
	data_obj->power_data_obj.usb_suspend_support = 1;
	data_obj->power_data_obj.usb_comm_capable = 1;
	data_obj->power_data_obj.reserved = 0;
}

static int sm5038_usbpd_manager_init(struct sm5038_usbpd_data *pd_data)
{
	int ret = 0;
	struct sm5038_usbpd_manager_data *manager = &pd_data->manager;
	struct sm5038_phydrv_data *pdic_data = pd_data->phy_driver_data;

	if (manager == NULL) {
		pr_err("%s, usbpd manager data is error!!\n", __func__);
		return -ENOMEM;
	}
	pr_info("%s\n", __func__);

	manager->pd_data = pd_data;
	manager->power_role_swap = true;
	manager->data_role_swap = true;
	manager->vconn_source_swap = pdic_data->vconn_en ? true : false;
	manager->acc_type = 0;
	manager->Vendor_ID = 0;
	manager->Product_ID = 0;
	manager->Device_Version = 0;
	manager->alt_sended = 0;
	manager->vdm_en = 0;
	manager->ext_sended = 0;
	manager->SVID_0 = 0;
	manager->SVID_1 = 0;
	manager->SVID_DP = 0;
	manager->Standard_Vendor_ID = 0;
	manager->fled_flash_enable = 0;
	manager->fled_torch_enable = 0;
	manager->origin_selected_pdo_num = 0;

	sm5038_usbpd_init_source_cap_data(manager);
	INIT_DELAYED_WORK(&manager->new_power_handler,
			sm5038_usbpd_manager_new_power_handler);
	INIT_DELAYED_WORK(&manager->start_discover_msg_handler,
			sm5038_usbpd_start_discover_msg_handler);
	return ret;
}

static void sm5038_usbpd_rx_layer_init(struct sm5038_protocol_data *rx)
{
	int i;

	rx->stored_message_id = USBPD_nMessageIDCount+1;
	rx->msg_header.word = 0;
	rx->status = DEFAULT_PROTOCOL_NONE;
	for (i = 0; i < USBPD_MAX_COUNT_MSG_OBJECT; i++)
		rx->data_obj[i].object = 0;
}

static void sm5038_usbpd_tx_layer_init(struct sm5038_protocol_data *tx)
{
	int i;

	tx->stored_message_id = USBPD_nMessageIDCount+1;
	tx->msg_header.word = 0;
	tx->status = DEFAULT_PROTOCOL_NONE;
	for (i = 0; i < USBPD_MAX_COUNT_MSG_OBJECT; i++)
		tx->data_obj[i].object = 0;
}

void sm5038_usbpd_tx_request_discard(struct sm5038_usbpd_data *pd_data)
{
	struct sm5038_protocol_data *tx = &pd_data->protocol_tx;
	int i;

	tx->msg_header.word = 0;
	for (i = 0; i < USBPD_MAX_COUNT_MSG_OBJECT; i++)
		tx->data_obj[i].object = 0;

	dev_err(&pd_data->dev, "%s\n", __func__);
}
EXPORT_SYMBOL_GPL(sm5038_usbpd_tx_request_discard);

void sm5038_usbpd_init_protocol(struct sm5038_usbpd_data *pd_data)
{
	struct sm5038_policy_data *policy = &pd_data->policy;
	struct sm5038_phydrv_data *pdic_data = pd_data->phy_driver_data;

	if (policy->state == PE_SRC_Startup ||
			policy->state == PE_SNK_Startup ||
			policy->state == PE_SRC_Send_Soft_Reset ||
			policy->state == PE_SNK_Send_Soft_Reset ||
			policy->state == PE_SRC_Soft_Reset ||
			policy->state == PE_SNK_Soft_Reset) {
		if (pdic_data->reset_done == 0)
			sm5038_pd_reset_protocol(pd_data);
	}

	sm5038_usbpd_rx_layer_init(&pd_data->protocol_rx);
	sm5038_usbpd_tx_layer_init(&pd_data->protocol_tx);
}
EXPORT_SYMBOL_GPL(sm5038_usbpd_init_protocol);

static void sm5038_usbpd_init_counters(struct sm5038_usbpd_data *pd_data)
{
	pr_info("%s: init counter\n", __func__);
	pd_data->counter.retry_counter = 0;
	pd_data->counter.message_id_counter = 0;
	pd_data->counter.caps_counter = 0;
	pd_data->counter.hard_reset_counter = 0;
	pd_data->counter.swap_hard_reset_counter = 0;
	pd_data->counter.discover_identity_counter = 0;
}

void sm5038_usbpd_reinit(struct device *dev)
{
	struct sm5038_usbpd_data *pd_data = dev_get_drvdata(dev);

	sm5038_usbpd_init_counters(pd_data);
	sm5038_usbpd_init_protocol(pd_data);
	sm5038_usbpd_init_policy(pd_data);
	reinit_completion(&pd_data->msg_arrived);
	pd_data->wait_for_msg_arrived = 0;
	pd_data->specification_revision = USBPD_REV_20;
	sm5038_usbpd_start_discover_msg_cancel(&pd_data->dev);
	complete(&pd_data->msg_arrived);
	reinit_completion(&pd_data->pd_completion);
	complete(&pd_data->pd_completion);
}
EXPORT_SYMBOL_GPL(sm5038_usbpd_reinit);

static void sm5038_usbpd_release(struct device *dev)
{
	struct sm5038_usbpd_data *pd_data = container_of(dev, struct sm5038_usbpd_data, dev);

	kfree(pd_data);
}


void sm5038_usbpd_device_unregister(struct sm5038_usbpd_data *pd_data)
{
	device_unregister(&pd_data->dev);
}
EXPORT_SYMBOL_GPL(sm5038_usbpd_device_unregister);

void sm5038_usbpd_class_unregister(struct sm5038_usbpd_data *pd_data)
{
	class_unregister(&sm5038_usbpd_class);
}
EXPORT_SYMBOL_GPL(sm5038_usbpd_class_unregister);

int sm5038_usbpd_init(struct device *dev, void *phy_driver_data)
{
	struct sm5038_usbpd_data *pd_data;
	int ret;

	if (!dev)
		return -EINVAL;

	class_register(&sm5038_usbpd_class);

	pd_data = kzalloc(sizeof(*pd_data), GFP_KERNEL);

	if (!pd_data)
		return -ENOMEM;

//	pd_data->dev = dev;
	device_initialize(&pd_data->dev);
	pd_data->phy_driver_data = phy_driver_data;
	pd_data->dev.class = &sm5038_usbpd_class;
	pd_data->dev.parent = dev;
	pd_data->dev.release = sm5038_usbpd_release;
	dev_set_drvdata(dev, pd_data);
	dev_set_drvdata(&pd_data->dev, pd_data);

	dev_set_name(&pd_data->dev, "usbpd_sm5038");

	ret = device_add(&pd_data->dev);
	if (ret) {
		dev_err(&pd_data->dev, "failed to register usbpd_sm5038 (%d)\n",
			ret);
		put_device(&pd_data->dev);
	}

	g_pd_data = pd_data;

	pd_data->specification_revision = USBPD_REV_20;
	sm5038_usbpd_init_counters(pd_data);
	sm5038_usbpd_init_protocol(pd_data);
	sm5038_usbpd_init_policy(pd_data);
	sm5038_usbpd_manager_init(pd_data);

	INIT_WORK(&pd_data->worker, sm5038_usbpd_policy_work);
	init_completion(&pd_data->msg_arrived);
	init_completion(&pd_data->pd_completion);

	return 0;
}
EXPORT_SYMBOL_GPL(sm5038_usbpd_init);

MODULE_LICENSE("GPL");
