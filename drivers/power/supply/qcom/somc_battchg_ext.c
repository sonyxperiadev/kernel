// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2019-2020, The Linux Foundation. All rights reserved.
 */

#include "somc_battchg_ext.h"

static void somc_bcext_handle_glink_response(struct somc_bcext_dev *bcext_dev,
					union glink_oem_resp_msg *resp_msg)
{
	bool expected_resp = false;

	switch (resp_msg->hdr.opcode) {
	case MSG_OPCODE_OEM_READ_BUFFER:
		if (bcext_dev->glink_req_msg.hdr.opcode ==
						MSG_OPCODE_OEM_READ_BUFFER) {
			bcext_dev->glink_resp_msg.rbuf = resp_msg->rbuf;
			expected_resp = true;
		}
		break;
	case MSG_OPCODE_OEM_WRITE_BUFFER:
		if (bcext_dev->glink_req_msg.hdr.opcode ==
						MSG_OPCODE_OEM_WRITE_BUFFER) {
			bcext_dev->glink_resp_msg.wbuf = resp_msg->wbuf;
			expected_resp = true;
		}
		break;
	default:
		break;
	}

	if (expected_resp)
		complete(&bcext_dev->glink_ack);
}

static ssize_t wireless_chg_negotiation_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct somc_bcext_dev *bcext_dev = dev_get_drvdata(dev);
	int size;

	size = scnprintf(buf, PAGE_SIZE, "%d\n",
				bcext_dev->wireless_chg_negotiation);

	return size;
}
static DEVICE_ATTR_RO(wireless_chg_negotiation);

static ssize_t wireless_rvschg_status_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct somc_bcext_dev *bcext_dev = dev_get_drvdata(dev);
	int size;

	size = scnprintf(buf, PAGE_SIZE, "%d\n",
				bcext_dev->wireless_rvschg_status);

	return size;
}
static DEVICE_ATTR_RO(wireless_rvschg_status);

static ssize_t wireless_rvschg_stop_reason_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct somc_bcext_dev *bcext_dev = dev_get_drvdata(dev);
	int size;

	size = scnprintf(buf, PAGE_SIZE, "%d\n",
				bcext_dev->wireless_rvschg_stop_reason);

	return size;
}
static DEVICE_ATTR_RO(wireless_rvschg_stop_reason);

static struct attribute *wlc_attrs[] = {
	&dev_attr_wireless_chg_negotiation.attr,
	&dev_attr_wireless_rvschg_status.attr,
	&dev_attr_wireless_rvschg_stop_reason.attr,
	NULL,
};
ATTRIBUTE_GROUPS(wlc);

static int somc_bcext_handle_glink_notification(struct somc_bcext_dev *bcext_dev,
				struct glink_charger_notify_msg *notify_msg)
{
	int ret = -1;
	char *envp[] = {NULL, NULL};

	pr_info("notification: %#x\n", notify_msg->notification);

	if (notify_msg->hdr.opcode != MSG_OPCODE_OEM_NOTIFY)
		return ret;

	switch (notify_msg->notification) {
	case WIRELESS_CHG_NEGOTIATION_END:
		bcext_dev->wireless_chg_negotiation = 0;
		envp[0] = "SWITCH_STATE=0";
		break;
	case WIRELESS_CHG_NEGOTIATION_START:
		bcext_dev->wireless_chg_negotiation = 1;
		envp[0] = "SWITCH_STATE=1";
		break;
	case WIRELESS_RVSCHG_END:
		bcext_dev->wireless_rvschg_status = 0;
		envp[0] = "RVSCHG_STATUS=0";
		break;
	case WIRELESS_RVSCHG_START:
		bcext_dev->wireless_rvschg_status = 1;
		envp[0] = "RVSCHG_STATUS=1";
		break;
	case WIRELESS_RVSCHG_STOP_REASON_UNKNOWN:
		bcext_dev->wireless_rvschg_stop_reason = 0;
		envp[0] = "RVSCHG_REASON=0";
		break;
	case WIRELESS_RVSCHG_STOP_REASON_BATTERY_STATUS_FULL:
		bcext_dev->wireless_rvschg_stop_reason = 1;
		envp[0] = "RVSCHG_REASON=1";
		break;
	case WIRELESS_RVSCHG_STOP_REASON_THERMAL_MITIGATION:
		bcext_dev->wireless_rvschg_stop_reason = 2;
		envp[0] = "RVSCHG_REASON=2";
		break;
	case WIRELESS_RVSCHG_STOP_REASON_EXT:
		bcext_dev->wireless_rvschg_stop_reason = 3;
		envp[0] = "RVSCHG_REASON=3";
		break;
	case WIRELESS_RVSCHG_STOP_REASON_ERROR:
		bcext_dev->wireless_rvschg_stop_reason = 100;
		envp[0] = "RVSCHG_REASON=100";
		break;
	default:
		return ret;
	}
	ret = kobject_uevent_env(&bcext_dev->vdev->kobj, KOBJ_CHANGE, envp);
	return ret;
}

static int somc_bcext_glink_cb(void *priv, void *data, size_t len)
{
	struct pmic_glink_hdr *hdr = data;
	struct somc_bcext_dev *bcext_dev = priv;

	pr_debug("owner: %u type: %u opcode: %#x len: %zu\n",
				hdr->owner, hdr->type, hdr->opcode, len);

	if (len < sizeof(struct pmic_glink_hdr) || hdr->owner != MSG_OWNER_OEM)
		return 0;

	switch (hdr->type) {
	case MSG_TYPE_REQ_RESP:
		somc_bcext_handle_glink_response(bcext_dev,
					(union glink_oem_resp_msg *)hdr);
		break;
	case MSG_TYPE_NOTIFICATION:
		somc_bcext_handle_glink_notification(bcext_dev,
					(struct glink_charger_notify_msg *)hdr);
		break;
	default:
		break;
	}

	return 0;
}

static void somc_bcext_glink_state_cb(void *priv,
						enum pmic_glink_state state)
{
	struct somc_bcext_dev *bcext_dev = priv;

	pr_debug("state: %d\n", state);

	atomic_set(&bcext_dev->glink_state, state);
}

#define GLINK_WAIT_TIME_MS		1000

static int somc_bcext_glink_request(struct somc_bcext_dev *bcext_dev,
	union glink_oem_req_msg *req_msg, union glink_oem_resp_msg *resp_msg)
{
	void *data;
	size_t len;
	int rc = 0;

	/*
	 * When the subsystem goes down, it's better to return the last
	 * known values until it comes back up. Hence, return 0 so that
	 * pmic_glink_write() is not attempted until pmic glink is up.
	 */
	if (atomic_read(&bcext_dev->glink_state) == PMIC_GLINK_STATE_DOWN) {
		pr_debug("glink state is down\n");
		return -ENODEV;
	}

	mutex_lock(&bcext_dev->glink_rw_lock);
	reinit_completion(&bcext_dev->glink_ack);
	bcext_dev->glink_req_msg = *req_msg;

	data = req_msg;

	if (req_msg->hdr.owner != MSG_OWNER_OEM ||
				req_msg->hdr.type != MSG_TYPE_REQ_RESP) {
		pr_err("unsupported message\n");
		goto out;
	}
	switch (req_msg->hdr.opcode) {
	case MSG_OPCODE_OEM_READ_BUFFER:
		len = sizeof(struct glink_oem_req_msg_read_buffer);
		break;
	case MSG_OPCODE_OEM_WRITE_BUFFER:
		len = sizeof(struct glink_oem_req_msg_write_buffer);
		break;
	default:
		pr_err("unsupported message\n");
		goto out;
	}

	rc = pmic_glink_write(bcext_dev->glink_client, data, len);
	if (rc < 0) {
		pr_err("Error in sending message rc=%d\n", rc);
		goto out;
	}

	if (!wait_for_completion_timeout(&bcext_dev->glink_ack,
				msecs_to_jiffies(GLINK_WAIT_TIME_MS))) {
		pr_err("Error, timed out sending message\n");
		rc = -ETIMEDOUT;
		goto out;
	}

	*resp_msg = bcext_dev->glink_resp_msg;
	memset(&bcext_dev->glink_req_msg, 0, sizeof(union glink_oem_req_msg));

out:
	mutex_unlock(&bcext_dev->glink_rw_lock);
	return rc;
}

static int somc_bcext_get_prop(struct somc_bcext_dev *bcext_dev,
						u32 somc_prop_id, u32 *val)
{
	union glink_oem_req_msg req_msg;
	union glink_oem_resp_msg resp_msg;
	int rc = 0;

	req_msg.hdr.owner = MSG_OWNER_OEM;
	req_msg.hdr.type = MSG_TYPE_REQ_RESP;
	req_msg.hdr.opcode = MSG_OPCODE_OEM_READ_BUFFER;

	req_msg.rbuf.prop_id = somc_prop_id;
	req_msg.rbuf.data_size = 1;

	rc = somc_bcext_glink_request(bcext_dev, &req_msg, &resp_msg);
	if (rc < 0) {
		pr_err("Failed to request reading buffer, rc=%d\n", rc);
		return rc;
	}

	if (resp_msg.rbuf.prop_id != req_msg.rbuf.prop_id ||
					resp_msg.rbuf.data_size != 1) {
		pr_err("Failed to get property\n");
		return -EINVAL;
	}

	*val = resp_msg.rbuf.data_buffer[0];
	return 0;
}

static int somc_bcext_set_prop(struct somc_bcext_dev *bcext_dev,
						u32 somc_prop_id, u32 val)
{
	union glink_oem_req_msg req_msg;
	union glink_oem_resp_msg resp_msg;
	int rc = 0;

	req_msg.hdr.owner = MSG_OWNER_OEM;
	req_msg.hdr.type = MSG_TYPE_REQ_RESP;
	req_msg.hdr.opcode = MSG_OPCODE_OEM_WRITE_BUFFER;

	req_msg.wbuf.prop_id = somc_prop_id;
	req_msg.wbuf.data_buffer[0] = val;
	req_msg.wbuf.data_size = 1;

	rc = somc_bcext_glink_request(bcext_dev, &req_msg, &resp_msg);
	if (rc < 0) {
		pr_err("Failed to request wriring buffer, rc=%d\n", rc);
		return rc;
	}

	if (resp_msg.wbuf.return_status != BATTMNGR_SUCCESS) {
		pr_err("Failed to set property\n");
		return -EINVAL;
	}

	return 0;
}

static void somc_bcext_timed_fake_chg_work(struct work_struct *work)
{
	struct somc_bcext_dev *bcext_dev = container_of(work,
			struct somc_bcext_dev, timed_fake_chg_work.work);

	vote(bcext_dev->fake_chg_votable, TIMED_FAKE_CHG_VOTER, false, 0);
}

static void somc_bcext_start_timed_fake_charging(
		struct somc_bcext_dev *bcext_dev, unsigned int delay_msecs)
{
	unsigned long delay;
	bool delay_scheduled = false;

	if (!delay_msecs)
		return;

	delay = msecs_to_jiffies(delay_msecs);
	vote(bcext_dev->fake_chg_votable, TIMED_FAKE_CHG_VOTER, true, 0);

	if (delayed_work_pending(&bcext_dev->timed_fake_chg_work)) {
		if (bcext_dev->timed_fake_chg_expire > jiffies + delay)
			delay_scheduled = true;
		else
			cancel_delayed_work_sync(
					&bcext_dev->timed_fake_chg_work);
	}
	if (!delay_scheduled) {
		schedule_delayed_work(&bcext_dev->timed_fake_chg_work, delay);
		bcext_dev->timed_fake_chg_expire = jiffies + delay;
	}
}

static void somc_bcext_start_fake_charging(struct somc_bcext_dev *bcext_dev,
							const char *voter)
{
	vote(bcext_dev->fake_chg_votable, voter, true, 0);
}

static void somc_bcext_stop_fake_charging(struct somc_bcext_dev *bcext_dev,
				const char *voter, unsigned int delay_msecs)
{
	int rc;

	rc = get_client_vote(bcext_dev->fake_chg_votable, voter);
	if (!rc || rc == EINVAL)
		return;

	if (delay_msecs)
		somc_bcext_start_timed_fake_charging(bcext_dev, delay_msecs);

	vote(bcext_dev->fake_chg_votable, voter, false, 0);
}

static int somc_bcext_smart_set_suspend(struct somc_bcext_dev *bcext_dev,
						bool smart_charge_suspended)
{
	int rc = 0;

	if (!bcext_dev->smart_charge_enabled) {
		pr_err("Couldn't set smart charge voter due to unactivated\n");
		return rc;
	}

	if (smart_charge_suspended)
		somc_bcext_start_fake_charging(bcext_dev, SMART_EN_VOTER);
	else
		somc_bcext_stop_fake_charging(bcext_dev, SMART_EN_VOTER,
						CHARGE_START_DELAY_TIME);

	rc = vote(bcext_dev->fcc_votable, SMART_EN_VOTER,
						smart_charge_suspended, 0);
	if (rc < 0) {
		pr_err("Couldn't vote en, rc=%d\n", rc);
		return rc;
	}

	pr_debug("voted for smart charging (%d).\n", smart_charge_suspended);
	cancel_delayed_work_sync(&bcext_dev->smart_charge_wdog_work);
	if (smart_charge_suspended)
		schedule_delayed_work(&bcext_dev->smart_charge_wdog_work,
				msecs_to_jiffies(SMART_CHARGE_WDOG_DELAY_MS));

	return rc;
}

static void somc_bcext_smart_charge_wdog_work(struct work_struct *work)
{
	struct somc_bcext_dev *bcext_dev = container_of(work,
			struct somc_bcext_dev, smart_charge_wdog_work.work);

	pr_debug("Smart Charge Watchdog timer has expired.\n");

	vote(bcext_dev->fcc_votable, SMART_EN_VOTER, false, 0);
	somc_bcext_stop_fake_charging(bcext_dev, SMART_EN_VOTER, 0);
}

static int somc_apply_thermal_mitigation(struct somc_bcext_dev *bcext_dev)
{
	int rc = 0;
	struct thermal_mitigation *current_therm_mitig =
			&bcext_dev->therm_mitig[bcext_dev->therm_level];

	rc |= vote(bcext_dev->fcc_votable, THERMAL_VOTER, true,
						current_therm_mitig->fcc_ma);

	switch (bcext_dev->usb_type) {
	case POWER_SUPPLY_USB_TYPE_UNKNOWN:
		rc |= vote(bcext_dev->usb_icl_votable, THERMAL_VOTER, false, 0);
		break;
	case POWER_SUPPLY_USB_TYPE_PD:
	case POWER_SUPPLY_USB_TYPE_PD_DRP:
	case POWER_SUPPLY_USB_TYPE_PD_PPS:
		rc |= vote(bcext_dev->usb_icl_votable, THERMAL_VOTER, true,
					current_therm_mitig->usb_pd_icl_ma);
		break;
	default:
		rc |= vote(bcext_dev->usb_icl_votable, THERMAL_VOTER, true,
					current_therm_mitig->usb_icl_ma);
		break;
	}

	switch (current_therm_mitig->charging_indication) {
	case CHG_IND_DEFAULT:
		rc |= vote(bcext_dev->fake_chg_votable, THERMAL_VOTER,
								false, 0);
		rc |= vote(bcext_dev->fake_chg_disallow_votable, THERMAL_VOTER,
								false, 0);
		break;
	case CHG_IND_FAKE_CHARGING:
		rc |= vote(bcext_dev->fake_chg_votable, THERMAL_VOTER,
								true, 0);
		rc |= vote(bcext_dev->fake_chg_disallow_votable, THERMAL_VOTER,
								false, 0);
		break;
	case CHG_IND_FAKE_CHARGING_DISALLOW:
		rc |= vote(bcext_dev->fake_chg_votable, THERMAL_VOTER,
								false, 0);
		rc |= vote(bcext_dev->fake_chg_disallow_votable, THERMAL_VOTER,
								true, 0);
		break;
	default:
		break;
	}

	if (rc < 0)
		pr_err("Couldn't vote for thermal mitigation values, rc=%d\n",
									rc);
	return rc;
}

static void somc_bcext_offchg_termination_work(struct work_struct *work)
{
	struct somc_bcext_dev *bcext_dev = container_of(work,
			struct somc_bcext_dev, offchg_termination_work.work);

	__pm_wakeup_event(bcext_dev->unplug_wakelock, UNPLUG_WAKE_PERIOD);

	/* key event for power off charge */
	pr_err("input_report_key KEY_F24\n");
	input_report_key(bcext_dev->unplug_key, KEY_F24, 1);
	input_sync(bcext_dev->unplug_key);
	input_report_key(bcext_dev->unplug_key, KEY_F24, 0);
	input_sync(bcext_dev->unplug_key);
}

static ssize_t ets_mode_store(struct class *c, struct class_attribute *attr,
						const char *buf, size_t count)
{
	struct somc_bcext_dev *bcext_dev = container_of(c,
					struct somc_bcext_dev, bcext_class);
	bool ets_mode;

	if (kstrtobool(buf, &ets_mode))
		return -EINVAL;

	somc_bcext_set_prop(bcext_dev, BATTMNGR_SOMC_PROP_ETS_MODE,
								(u32)ets_mode);

	return count;
}
static ssize_t ets_mode_show(struct class *c, struct class_attribute *attr,
								char *buf)
{
	struct somc_bcext_dev *bcext_dev = container_of(c,
					struct somc_bcext_dev, bcext_class);
	u32 ets_mode;

	somc_bcext_get_prop(bcext_dev, BATTMNGR_SOMC_PROP_ETS_MODE, &ets_mode);

	return scnprintf(buf, PAGE_SIZE, "%d\n", (int)ets_mode);
}
static CLASS_ATTR_RW(ets_mode);

static ssize_t batt_id_show(struct class *c, struct class_attribute *attr,
								char *buf)
{
	struct somc_bcext_dev *bcext_dev = container_of(c,
					struct somc_bcext_dev, bcext_class);
	u32 batt_id;

	somc_bcext_get_prop(bcext_dev, BATTMNGR_SOMC_PROP_BATT_ID, &batt_id);

	return scnprintf(buf, PAGE_SIZE, "%d\n", (int)batt_id);
}
static CLASS_ATTR_RO(batt_id);

static ssize_t usb_in_v_show(struct class *c, struct class_attribute *attr,
								char *buf)
{
	struct somc_bcext_dev *bcext_dev = container_of(c,
					struct somc_bcext_dev, bcext_class);
	u32 usb_in_v;

	somc_bcext_get_prop(bcext_dev, BATTMNGR_SOMC_PROP_USB_IN_V, &usb_in_v);

	return scnprintf(buf, PAGE_SIZE, "%d\n", (int)usb_in_v);
}
static CLASS_ATTR_RO(usb_in_v);

static ssize_t usb_input_suspend_store(struct class *c,
		struct class_attribute *attr, const char *buf, size_t count)
{
	struct somc_bcext_dev *bcext_dev = container_of(c,
					struct somc_bcext_dev, bcext_class);
	int val;
	bool enabled;
	int rc;

	if (kstrtoint(buf, 10, &val))
		return -EINVAL;

	if (val == 1)
		enabled = true;
	else if (val == 0)
		enabled = false;
	else
		return -EINVAL;

	rc = vote(bcext_dev->usb_icl_votable, USER_SUSPEND_VOTER, enabled, 0);
	if (rc < 0)
		return -EINVAL;

	return count;
}
static ssize_t usb_input_suspend_show(struct class *c,
					struct class_attribute *attr, char *buf)
{
	struct somc_bcext_dev *bcext_dev = container_of(c,
					struct somc_bcext_dev, bcext_class);
	int val;

	if (is_client_vote_enabled(bcext_dev->usb_icl_votable,
							USER_SUSPEND_VOTER))
		val = 1;
	else
		val = 0;

	return scnprintf(buf, PAGE_SIZE, "%d\n", val);
}
static CLASS_ATTR_RW(usb_input_suspend);

static ssize_t batt_aging_level_store(struct class *c,
						struct class_attribute *attr,
						const char *buf, size_t count)
{
	struct somc_bcext_dev *bcext_dev = container_of(c,
					struct somc_bcext_dev, bcext_class);
	int aging_level;

	if (kstrtoint(buf, 10, &aging_level))
		return -EINVAL;

	somc_bcext_set_prop(bcext_dev, BATTMNGR_SOMC_PROP_BATT_AGING_LEVEL,
								aging_level);

	return count;

}
static ssize_t batt_aging_level_show(struct class *c,
					struct class_attribute *attr, char *buf)
{
	struct somc_bcext_dev *bcext_dev = container_of(c,
					struct somc_bcext_dev, bcext_class);
	int aging_level;

	somc_bcext_get_prop(bcext_dev, BATTMNGR_SOMC_PROP_BATT_AGING_LEVEL,
								&aging_level);

	return scnprintf(buf, PAGE_SIZE, "%d\n", aging_level);
}
static CLASS_ATTR_RW(batt_aging_level);

static ssize_t real_nom_cap_show(struct class *c,
					struct class_attribute *attr, char *buf)
{
	struct somc_bcext_dev *bcext_dev = container_of(c,
					struct somc_bcext_dev, bcext_class);
	int real_nom_cap;

	somc_bcext_get_prop(bcext_dev, BATTMNGR_SOMC_PROP_REAL_NOM_CAP,
								&real_nom_cap);

	return scnprintf(buf, PAGE_SIZE, "%d\n", real_nom_cap);
}
static CLASS_ATTR_RO(real_nom_cap);

static ssize_t wls_fw_update_store(struct class *c,
						struct class_attribute *attr,
						const char *buf, size_t count)
{
	struct somc_bcext_dev *bcext_dev = container_of(c,
					struct somc_bcext_dev, bcext_class);
	bool fw_update;

	if (kstrtobool(buf, &fw_update))
		return -EINVAL;

	somc_bcext_set_prop(bcext_dev, BATTMNGR_SOMC_PROP_WLS_FW_UPDATE,
								(u32)fw_update);

	return count;
}
static ssize_t wls_fw_update_show(struct class *c, struct class_attribute *attr,
								char *buf)
{
	struct somc_bcext_dev *bcext_dev = container_of(c,
					struct somc_bcext_dev, bcext_class);
	u32 fw_update;

	somc_bcext_get_prop(bcext_dev, BATTMNGR_SOMC_PROP_WLS_FW_UPDATE,
								&fw_update);

	return scnprintf(buf, PAGE_SIZE, "%d\n", (int)fw_update);
}
static CLASS_ATTR_RW(wls_fw_update);

static ssize_t wls_int_show(struct class *c, struct class_attribute *attr,
								char *buf)
{
	struct somc_bcext_dev *bcext_dev = container_of(c,
					struct somc_bcext_dev, bcext_class);
	u32 wls_int;

	somc_bcext_get_prop(bcext_dev, BATTMNGR_SOMC_PROP_WLS_INT, &wls_int);

	return scnprintf(buf, PAGE_SIZE, "%d\n", (int)wls_int);
}
static CLASS_ATTR_RO(wls_int);

static ssize_t wls_en_store(struct class *c, struct class_attribute *attr,
						const char *buf, size_t count)
{
	struct somc_bcext_dev *bcext_dev = container_of(c,
					struct somc_bcext_dev, bcext_class);
	bool wls_en;

	if (kstrtobool(buf, &wls_en))
		return -EINVAL;

	somc_bcext_set_prop(bcext_dev, BATTMNGR_SOMC_PROP_WLS_EN, (u32)wls_en);

	return count;
}
static ssize_t wls_en_show(struct class *c, struct class_attribute *attr,
								char *buf)
{
	struct somc_bcext_dev *bcext_dev = container_of(c,
					struct somc_bcext_dev, bcext_class);
	u32 wls_en;

	somc_bcext_get_prop(bcext_dev, BATTMNGR_SOMC_PROP_WLS_EN, &wls_en);

	return scnprintf(buf, PAGE_SIZE, "%d\n", (int)wls_en);
}
static CLASS_ATTR_RW(wls_en);

static ssize_t wls_iout_show(struct class *c, struct class_attribute *attr,
								char *buf)
{
	struct somc_bcext_dev *bcext_dev = container_of(c,
					struct somc_bcext_dev, bcext_class);
	u32 iout;

	somc_bcext_get_prop(bcext_dev, BATTMNGR_SOMC_PROP_WLS_IOUT, &iout);

	return scnprintf(buf, PAGE_SIZE, "%d\n", (int)iout);
}
static CLASS_ATTR_RO(wls_iout);

static ssize_t wls_chip_id_show(struct class *c, struct class_attribute *attr,
								char *buf)
{
	struct somc_bcext_dev *bcext_dev = container_of(c,
					struct somc_bcext_dev, bcext_class);
	u32 chip_id;

	somc_bcext_get_prop(bcext_dev, BATTMNGR_SOMC_PROP_WLS_CHIP_ID,
								&chip_id);

	return scnprintf(buf, PAGE_SIZE, "%d\n", (int)chip_id);
}
static CLASS_ATTR_RO(wls_chip_id);

static ssize_t wls_guaranteed_pwr_show(struct class *c,
				struct class_attribute *attr, char *buf)
{
	struct somc_bcext_dev *bcext_dev = container_of(c,
					struct somc_bcext_dev, bcext_class);
	u32 guaranteed_pwr;

	somc_bcext_get_prop(bcext_dev, BATTMNGR_SOMC_PROP_WLS_GUARANTEED_PWR,
							&guaranteed_pwr);

	return scnprintf(buf, PAGE_SIZE, "%d\n", (int)guaranteed_pwr);
}
static CLASS_ATTR_RO(wls_guaranteed_pwr);

static ssize_t wls_vrect_show(struct class *c, struct class_attribute *attr,
								char *buf)
{
	struct somc_bcext_dev *bcext_dev = container_of(c,
					struct somc_bcext_dev, bcext_class);
	u32 vrect;

	somc_bcext_get_prop(bcext_dev, BATTMNGR_SOMC_PROP_WLS_VRECT, &vrect);

	return scnprintf(buf, PAGE_SIZE, "%d\n", (int)vrect);
}
static CLASS_ATTR_RO(wls_vrect);

static ssize_t wls_vout_show(struct class *c, struct class_attribute *attr,
								char *buf)
{
	struct somc_bcext_dev *bcext_dev = container_of(c,
					struct somc_bcext_dev, bcext_class);
	u32 vout;

	somc_bcext_get_prop(bcext_dev, BATTMNGR_SOMC_PROP_WLS_VOUT, &vout);

	return scnprintf(buf, PAGE_SIZE, "%d\n", (int)vout);
}
static CLASS_ATTR_RO(wls_vout);

static ssize_t wls_potential_pwr_show(struct class *c,
					struct class_attribute *attr, char *buf)
{
	struct somc_bcext_dev *bcext_dev = container_of(c,
					struct somc_bcext_dev, bcext_class);
	u32 potential_pwr;

	somc_bcext_get_prop(bcext_dev, BATTMNGR_SOMC_PROP_WLS_POTENTIAL_PWR,
								&potential_pwr);

	return scnprintf(buf, PAGE_SIZE, "%d\n", (int)potential_pwr);
}
static CLASS_ATTR_RO(wls_potential_pwr);

static ssize_t wls_fw_rev_show(struct class *c, struct class_attribute *attr,
								char *buf)
{
	struct somc_bcext_dev *bcext_dev = container_of(c,
					struct somc_bcext_dev, bcext_class);
	u32 fw_rev;

	somc_bcext_get_prop(bcext_dev, BATTMNGR_SOMC_PROP_WLS_FW_REV, &fw_rev);

	return scnprintf(buf, PAGE_SIZE, "%d\n", (int)fw_rev);
}
static CLASS_ATTR_RO(wls_fw_rev);

static ssize_t batt_soc_show(struct class *c,
					struct class_attribute *attr, char *buf)
{
	struct somc_bcext_dev *bcext_dev = container_of(c,
					struct somc_bcext_dev, bcext_class);
	int batt_soc;

	somc_bcext_get_prop(bcext_dev, BATTMNGR_SOMC_PROP_BATT_SOC,
								&batt_soc);

	return scnprintf(buf, PAGE_SIZE, "%d\n", batt_soc);
}
static CLASS_ATTR_RO(batt_soc);

static ssize_t monotonic_soc_show(struct class *c,
					struct class_attribute *attr, char *buf)
{
	struct somc_bcext_dev *bcext_dev = container_of(c,
					struct somc_bcext_dev, bcext_class);
	int monotonic_soc;

	somc_bcext_get_prop(bcext_dev, BATTMNGR_SOMC_PROP_MONOTONIC_SOC,
								&monotonic_soc);

	return scnprintf(buf, PAGE_SIZE, "%d\n", monotonic_soc);
}
static CLASS_ATTR_RO(monotonic_soc);

static ssize_t system_soc_show(struct class *c,
					struct class_attribute *attr, char *buf)
{
	struct somc_bcext_dev *bcext_dev = container_of(c,
					struct somc_bcext_dev, bcext_class);
	int system_soc;

	somc_bcext_get_prop(bcext_dev, BATTMNGR_SOMC_PROP_SYSTEM_SOC,
								&system_soc);

	return scnprintf(buf, PAGE_SIZE, "%d\n", system_soc);
}
static CLASS_ATTR_RO(system_soc);

static ssize_t wls_tx_ssp_show(struct class *c, struct class_attribute *attr,
								char *buf)
{
	struct somc_bcext_dev *bcext_dev = container_of(c,
					struct somc_bcext_dev, bcext_class);
	u32 tx_ssp;

	somc_bcext_get_prop(bcext_dev, BATTMNGR_SOMC_PROP_WLS_TX_SSP, &tx_ssp);

	return scnprintf(buf, PAGE_SIZE, "%d\n", (int)tx_ssp);
}
static CLASS_ATTR_RO(wls_tx_ssp);

static ssize_t wls_tx_dc_pwr_show(struct class *c, struct class_attribute *attr,
								char *buf)
{
	struct somc_bcext_dev *bcext_dev = container_of(c,
					struct somc_bcext_dev, bcext_class);
	u32 dc_pwr;

	somc_bcext_get_prop(bcext_dev, BATTMNGR_SOMC_PROP_WLS_TX_DC_PWR,
								&dc_pwr);

	return scnprintf(buf, PAGE_SIZE, "%d\n", (int)dc_pwr);
}
static CLASS_ATTR_RO(wls_tx_dc_pwr);

static ssize_t wls_tx_iin_show(struct class *c, struct class_attribute *attr,
								char *buf)
{
	struct somc_bcext_dev *bcext_dev = container_of(c,
					struct somc_bcext_dev, bcext_class);
	u32 iin;

	somc_bcext_get_prop(bcext_dev, BATTMNGR_SOMC_PROP_WLS_TX_IIN, &iin);

	return scnprintf(buf, PAGE_SIZE, "%d\n", (int)iin);
}
static CLASS_ATTR_RO(wls_tx_iin);

static ssize_t wls_cmd_reg_addr_store(struct class *c,
						struct class_attribute *attr,
						const char *buf, size_t count)
{
	struct somc_bcext_dev *bcext_dev = container_of(c,
					struct somc_bcext_dev, bcext_class);
	u32 reg_addr;

	if (kstrtouint(buf, 16, &reg_addr))
		return -EINVAL;

	somc_bcext_set_prop(bcext_dev, BATTMNGR_SOMC_PROP_WLS_CMD_REG_ADDR,
								reg_addr);

	return count;
}
static ssize_t wls_cmd_reg_addr_show(struct class *c,
						struct class_attribute *attr,
								char *buf)
{
	struct somc_bcext_dev *bcext_dev = container_of(c,
					struct somc_bcext_dev, bcext_class);
	u32 reg_addr;

	somc_bcext_get_prop(bcext_dev, BATTMNGR_SOMC_PROP_WLS_CMD_REG_ADDR,
								&reg_addr);

	return scnprintf(buf, PAGE_SIZE, "0x%02x\n", (int)reg_addr);
}
static CLASS_ATTR_RW(wls_cmd_reg_addr);

static ssize_t wls_cmd_reg_data_store(struct class *c,
						struct class_attribute *attr,
						const char *buf, size_t count)
{
	struct somc_bcext_dev *bcext_dev = container_of(c,
					struct somc_bcext_dev, bcext_class);
	u32 reg_data;

	if (kstrtouint(buf, 16, &reg_data))
		return -EINVAL;

	somc_bcext_set_prop(bcext_dev, BATTMNGR_SOMC_PROP_WLS_CMD_REG_DATA,
								reg_data);

	return count;
}
static ssize_t wls_cmd_reg_data_show(struct class *c,
						struct class_attribute *attr,
								char *buf)
{
	struct somc_bcext_dev *bcext_dev = container_of(c,
					struct somc_bcext_dev, bcext_class);
	u32 reg_data;

	somc_bcext_get_prop(bcext_dev, BATTMNGR_SOMC_PROP_WLS_CMD_REG_DATA,
								&reg_data);

	return scnprintf(buf, PAGE_SIZE, "0x%02x\n", (int)reg_data);
}
static CLASS_ATTR_RW(wls_cmd_reg_data);

static ssize_t wls_negotiated_pwr_show(struct class *c,
					struct class_attribute *attr, char *buf)
{
	struct somc_bcext_dev *bcext_dev = container_of(c,
					struct somc_bcext_dev, bcext_class);
	u32 iin;

	somc_bcext_get_prop(bcext_dev, BATTMNGR_SOMC_PROP_WLS_NEGOTIATED_PWR,
									&iin);

	return scnprintf(buf, PAGE_SIZE, "%d\n", (int)iin);
}
static CLASS_ATTR_RO(wls_negotiated_pwr);

static ssize_t lrc_input_suspend_store(struct class *c,
		struct class_attribute *attr, const char *buf, size_t count)
{
	struct somc_bcext_dev *bcext_dev = container_of(c,
					struct somc_bcext_dev, bcext_class);
	int val;
	bool enabled;
	int rc;

	if (kstrtoint(buf, 10, &val))
		return -EINVAL;

	if (val == 1)
		enabled = true;
	else if (val == 0)
		enabled = false;
	else
		return -EINVAL;

	rc = vote(bcext_dev->usb_icl_votable, LRC_VOTER, enabled, 0);
	if (rc < 0)
		return -EINVAL;

	return count;
}
static ssize_t lrc_input_suspend_show(struct class *c,
					struct class_attribute *attr, char *buf)
{
	struct somc_bcext_dev *bcext_dev = container_of(c,
					struct somc_bcext_dev, bcext_class);
	int val;

	if (is_client_vote_enabled(bcext_dev->usb_icl_votable, LRC_VOTER))
		val = 1;
	else
		val = 0;

	return scnprintf(buf, PAGE_SIZE, "%d\n", val);
}
static CLASS_ATTR_RW(lrc_input_suspend);

static ssize_t lrc_charge_disable_store(struct class *c,
		struct class_attribute *attr, const char *buf, size_t count)
{
	struct somc_bcext_dev *bcext_dev = container_of(c,
					struct somc_bcext_dev, bcext_class);
	int val;
	bool enabled;
	int rc;

	if (kstrtoint(buf, 10, &val))
		return -EINVAL;

	if (val == 1)
		enabled = true;
	else if (val == 0)
		enabled = false;
	else
		return -EINVAL;

	rc = vote(bcext_dev->fcc_votable, LRC_VOTER, enabled, 0);
	if (rc < 0)
		return -EINVAL;

	return count;
}
static ssize_t lrc_charge_disable_show(struct class *c,
					struct class_attribute *attr, char *buf)
{
	struct somc_bcext_dev *bcext_dev = container_of(c,
					struct somc_bcext_dev, bcext_class);
	int val;

	if (is_client_vote_enabled(bcext_dev->fcc_votable, LRC_VOTER))
		val = 1;
	else
		val = 0;

	return scnprintf(buf, PAGE_SIZE, "%d\n", val);
}
static CLASS_ATTR_RW(lrc_charge_disable);

static ssize_t smart_charging_activation_store(struct class *c,
		struct class_attribute *attr, const char *buf, size_t count)
{
	struct somc_bcext_dev *bcext_dev = container_of(c,
					struct somc_bcext_dev, bcext_class);
	int val;

	if (kstrtoint(buf, 10, &val))
		return -EINVAL;

	if (val == 1)
		bcext_dev->smart_charge_enabled = true;
	else if (val == 0)
		bcext_dev->smart_charge_enabled = false;
	else
		return -EINVAL;

	return count;
}
static ssize_t smart_charging_activation_show(struct class *c,
					struct class_attribute *attr, char *buf)
{
	struct somc_bcext_dev *bcext_dev = container_of(c,
					struct somc_bcext_dev, bcext_class);

	return scnprintf(buf, PAGE_SIZE, "%d\n",
					bcext_dev->smart_charge_enabled);
}
static CLASS_ATTR_RW(smart_charging_activation);

static ssize_t smart_charging_interruption_store(struct class *c,
		struct class_attribute *attr, const char *buf, size_t count)
{
	struct somc_bcext_dev *bcext_dev = container_of(c,
					struct somc_bcext_dev, bcext_class);
	bool smart_charge_suspended;
	int val;
	int rc;

	if (kstrtoint(buf, 10, &val))
		return -EINVAL;

	if (val == 1)
		smart_charge_suspended = true;
	else if (val == 0)
		smart_charge_suspended = false;
	else
		return -EINVAL;

	rc = somc_bcext_smart_set_suspend(bcext_dev, smart_charge_suspended);
	if (rc < 0)
		return -EINVAL;

	return count;
}
static ssize_t smart_charging_interruption_show(struct class *c,
					struct class_attribute *attr, char *buf)
{
	struct somc_bcext_dev *bcext_dev = container_of(c,
					struct somc_bcext_dev, bcext_class);
	int val;

	if (is_client_vote_enabled(bcext_dev->fcc_votable, SMART_EN_VOTER))
		val = 1;
	else
		val = 0;

	return scnprintf(buf, PAGE_SIZE, "%d\n", val);
}
static CLASS_ATTR_RW(smart_charging_interruption);

static ssize_t smart_charging_status_show(struct class *c,
					struct class_attribute *attr, char *buf)
{
	struct somc_bcext_dev *bcext_dev = container_of(c,
					struct somc_bcext_dev, bcext_class);
	int val;

	if (is_client_vote_enabled(bcext_dev->fcc_votable, SMART_EN_VOTER))
		val = 1;
	else
		val = 0;

	return scnprintf(buf, PAGE_SIZE, "%d\n", val);
}
static CLASS_ATTR_RO(smart_charging_status);

static ssize_t bootup_shutdown_phase_store(struct class *c,
		struct class_attribute *attr, const char *buf, size_t count)
{
	struct somc_bcext_dev *bcext_dev = container_of(c,
					struct somc_bcext_dev, bcext_class);

	if (kstrtoint(buf, 10, &bcext_dev->bootup_shutdown_phase))
		return -EINVAL;

	return count;
}
static ssize_t bootup_shutdown_phase_show(struct class *c,
					struct class_attribute *attr, char *buf)
{
	struct somc_bcext_dev *bcext_dev = container_of(c,
					struct somc_bcext_dev, bcext_class);

	return scnprintf(buf, PAGE_SIZE, "%d\n",
					bcext_dev->bootup_shutdown_phase);
}
static CLASS_ATTR_RW(bootup_shutdown_phase);

static ssize_t system_temp_level_store(struct class *c,
		struct class_attribute *attr, const char *buf, size_t count)
{
	struct somc_bcext_dev *bcext_dev = container_of(c,
					struct somc_bcext_dev, bcext_class);
	int val;

	if (kstrtoint(buf, 0, &val))
		return -EINVAL;

	if (val < 0 || val > MAX_THERM_LEVEL)
		return -EINVAL;

	bcext_dev->therm_level = val;
	somc_apply_thermal_mitigation(bcext_dev);

	somc_bcext_set_prop(bcext_dev, BATTMNGR_SOMC_PROP_SYSTEM_TEMP_LEVEL,
								(u32)val);
	return count;
}
static ssize_t system_temp_level_show(struct class *c,
					struct class_attribute *attr, char *buf)
{
	struct somc_bcext_dev *bcext_dev = container_of(c,
					struct somc_bcext_dev, bcext_class);

	return scnprintf(buf, PAGE_SIZE, "%d\n", bcext_dev->therm_level);
}
static CLASS_ATTR_RW(system_temp_level);

static ssize_t real_temp_store(struct class *c, struct class_attribute *attr,
						const char *buf, size_t count)
{
	struct somc_bcext_dev *bcext_dev = container_of(c,
					struct somc_bcext_dev, bcext_class);
	int real_temp;

	if (kstrtoint(buf, 10, &real_temp))
		return -EINVAL;

	somc_bcext_set_prop(bcext_dev, BATTMNGR_SOMC_PROP_REAL_TEMP,
								(u32)real_temp);

	return count;
}
static ssize_t real_temp_show(struct class *c, struct class_attribute *attr,
								char *buf)
{
	struct somc_bcext_dev *bcext_dev = container_of(c,
					struct somc_bcext_dev, bcext_class);
	u32 real_temp;

	somc_bcext_get_prop(bcext_dev, BATTMNGR_SOMC_PROP_REAL_TEMP, &real_temp);

	return scnprintf(buf, PAGE_SIZE, "%d\n", (int)real_temp);
}
static CLASS_ATTR_RW(real_temp);

static ssize_t batt_temp_show(struct class *c, struct class_attribute *attr,
								char *buf)
{
	struct somc_bcext_dev *bcext_dev = container_of(c,
					struct somc_bcext_dev, bcext_class);
	u32 batt_temp;

	somc_bcext_get_prop(bcext_dev, BATTMNGR_SOMC_PROP_BATT_TEMP,
								&batt_temp);

	return scnprintf(buf, PAGE_SIZE, "%d\n", (int)batt_temp);
}
static CLASS_ATTR_RO(batt_temp);

static ssize_t typec_mode_show(struct class *c, struct class_attribute *attr,
								char *buf)
{
	struct somc_bcext_dev *bcext_dev = container_of(c,
					struct somc_bcext_dev, bcext_class);
	u32 usb_power_opmode;
	char *typec_mode;

	somc_bcext_get_prop(bcext_dev, BATTMNGR_SOMC_PROP_USB_POWER_OPMODE,
							&usb_power_opmode);
	switch (usb_power_opmode) {
	case USB_POWER_OPMODE_NOCONSUMER:
		typec_mode = "Nothing source attached";
		break;
	case USB_POWER_OPMODE_TYPEC_DFT:
		typec_mode = "Type-C source attached (default current)";
		break;
	case USB_POWER_OPMODE_BC:
		typec_mode = "Legacy source attached";
		break;
	case USB_POWER_OPMODE_PD:
		typec_mode = "PD source attached";
		break;
	case USB_POWER_OPMODE_TYPEC_1P5A:
		typec_mode = "Type-C source attached (medium current)";
		break;
	case USB_POWER_OPMODE_TYPEC_3A:
		typec_mode = "Type-C source attached (high current)";
		break;
	default:
		typec_mode = "Unknown";
	}
	return scnprintf(buf, PAGE_SIZE, "%s\n", typec_mode);
}
static CLASS_ATTR_RO(typec_mode);

static ssize_t vcell_max_show(struct class *c, struct class_attribute *attr,
								char *buf)
{
	struct somc_bcext_dev *bcext_dev = container_of(c,
					struct somc_bcext_dev, bcext_class);
	u32 vcell_max;

	somc_bcext_get_prop(bcext_dev, BATTMNGR_SOMC_PROP_VCELL_MAX, &vcell_max);

	return scnprintf(buf, PAGE_SIZE, "%d\n", (int)vcell_max);
}
static CLASS_ATTR_RO(vcell_max);

static ssize_t product_code_show(struct class *c, struct class_attribute *attr,
								char *buf)
{
	struct somc_bcext_dev *bcext_dev = container_of(c,
					struct somc_bcext_dev, bcext_class);

	return scnprintf(buf, PAGE_SIZE, "%d\n", (int)bcext_dev->product_code);
}
static CLASS_ATTR_RO(product_code);

static struct attribute *somc_bcext_class_attrs[] = {
	&class_attr_ets_mode.attr,
	&class_attr_batt_id.attr,
	&class_attr_usb_in_v.attr,
	&class_attr_usb_input_suspend.attr,
	&class_attr_batt_aging_level.attr,
	&class_attr_real_nom_cap.attr,
	&class_attr_wls_fw_update.attr,
	&class_attr_wls_int.attr,
	&class_attr_wls_en.attr,
	&class_attr_wls_iout.attr,
	&class_attr_wls_chip_id.attr,
	&class_attr_wls_tx_ssp.attr,
	&class_attr_wls_tx_dc_pwr.attr,
	&class_attr_wls_tx_iin.attr,
	&class_attr_lrc_input_suspend.attr,
	&class_attr_lrc_charge_disable.attr,
	&class_attr_smart_charging_activation.attr,
	&class_attr_smart_charging_interruption.attr,
	&class_attr_smart_charging_status.attr,
	&class_attr_wls_guaranteed_pwr.attr,
	&class_attr_wls_vrect.attr,
	&class_attr_wls_vout.attr,
	&class_attr_wls_potential_pwr.attr,
	&class_attr_wls_fw_rev.attr,
	&class_attr_bootup_shutdown_phase.attr,
	&class_attr_system_temp_level.attr,
	&class_attr_wls_cmd_reg_addr.attr,
	&class_attr_wls_cmd_reg_data.attr,
	&class_attr_batt_soc.attr,
	&class_attr_monotonic_soc.attr,
	&class_attr_system_soc.attr,
	&class_attr_real_temp.attr,
	&class_attr_batt_temp.attr,
	&class_attr_typec_mode.attr,
	&class_attr_vcell_max.attr,
	&class_attr_wls_negotiated_pwr.attr,
	&class_attr_product_code.attr,
	NULL,
};
ATTRIBUTE_GROUPS(somc_bcext_class);

static irqreturn_t idtp9382a_irq_handler(int irq, void *data)
{
	struct somc_bcext_dev *bcext_dev = data;
	int stat;

	stat = gpio_get_value(bcext_dev->wls_gpio_irq);
	somc_bcext_set_prop(bcext_dev, BATTMNGR_SOMC_PROP_WLS_INT, (u32)stat);
	pr_info("irq_status=%d\n", stat);

	return IRQ_HANDLED;
}

static int somc_bcext_set_product_code(struct somc_bcext_dev *bcext_dev,
							u32 product_code)
{
	if (product_code)
		somc_bcext_set_prop(bcext_dev, BATTMNGR_SOMC_PROP_PRODUCT_CODE,
								product_code);

	return 0;
}

static int somc_bcext_psy_notifier_cb(struct notifier_block *nb,
					unsigned long event, void *data)
{
	struct power_supply *psy = data;
	union power_supply_propval prop = {0, };
	struct somc_bcext_dev *bcext_dev = container_of(nb,
						struct somc_bcext_dev, psy_nb);
	int rc;

	if (event != PSY_EVENT_PROP_CHANGED)
		return NOTIFY_OK;

	if (strcmp(psy->desc->name, "battery") == 0) {
		/* do nothing */
	}
	else if (strcmp(psy->desc->name, "usb") == 0) {
		rc = power_supply_get_property(psy, POWER_SUPPLY_PROP_USB_TYPE,
									&prop);
		if (!rc && prop.intval != bcext_dev->usb_type) {
			bcext_dev->usb_type = prop.intval;
			somc_apply_thermal_mitigation(bcext_dev);
		}

		rc = power_supply_get_property(psy, POWER_SUPPLY_PROP_ONLINE,
									&prop);
		if (rc)
			pr_err("Couldn't get online rc = %d\n", rc);
		else
			bcext_dev->usb_online = prop.intval;
	}
	else if (strcmp(psy->desc->name, "wireless") == 0) {
		rc = power_supply_get_property(psy, POWER_SUPPLY_PROP_ONLINE,
									&prop);
		if (rc)
			pr_err("Couldn't get online rc = %d\n", rc);
		else
			bcext_dev->wireless_online = prop.intval;
	}

	if (bcext_dev->bootup_shutdown_phase == DURING_POWER_OFF_CHARGE) {
		if (!bcext_dev->usb_online && !bcext_dev->wireless_online) {
			if (!delayed_work_pending(
					&bcext_dev->offchg_termination_work))
				schedule_delayed_work(
					&bcext_dev->offchg_termination_work,
					msecs_to_jiffies(
						OFFCHG_TERMINATION_DELAY_MS));
		} else {
			if (delayed_work_pending(
					&bcext_dev->offchg_termination_work))
				cancel_delayed_work_sync(
					&bcext_dev->offchg_termination_work);
		}
	}

	return NOTIFY_OK;
}

static int somc_bcext_usb_icl_vote_cb(struct votable *votable,
				void *data, int icl_ma, const char *client)
{
	struct somc_bcext_dev *bcext_dev = data;

	somc_bcext_set_prop(bcext_dev, BATTMNGR_SOMC_PROP_USB_ICL,
								(u32)icl_ma);

	return 0;
}

static int somc_bcext_fcc_vote_cb(struct votable *votable,
				void *data, int fcc_ma, const char *client)
{
	struct somc_bcext_dev *bcext_dev = data;

	somc_bcext_set_prop(bcext_dev, BATTMNGR_SOMC_PROP_FCC, (u32)fcc_ma);

	return 0;
}

static int somc_bcext_fake_chg_vote_cb(struct votable *votable,
				void *data, int fake_chg, const char *client)
{
	struct somc_bcext_dev *bcext_dev = data;

	somc_bcext_set_prop(bcext_dev, BATTMNGR_SOMC_PROP_FAKE_CHG,
								(u32)fake_chg);

	return 0;
}

static int somc_bcext_fake_chg_disallow_vote_cb(struct votable *votable,
			void *data, int fake_chg_disallow, const char *client)
{
	struct somc_bcext_dev *bcext_dev = data;

	somc_bcext_set_prop(bcext_dev, BATTMNGR_SOMC_PROP_FAKE_CHG_DISALLOW,
							(u32)fake_chg_disallow);

	return 0;
}

static int somc_bcext_create_votables(struct somc_bcext_dev *bcext_dev)
{
	int rc = 0;

	bcext_dev->usb_icl_votable = create_votable("USB_ICL", VOTE_MIN,
						somc_bcext_usb_icl_vote_cb,
						bcext_dev);
	if (IS_ERR(bcext_dev->usb_icl_votable)) {
		rc = PTR_ERR(bcext_dev->usb_icl_votable);
		bcext_dev->usb_icl_votable = NULL;
		goto out;
	}
	rc = vote(bcext_dev->usb_icl_votable, DEFAULT_VOTER, true,
								MAX_USB_ICL_MA);
	if (rc)
		goto out;

	bcext_dev->fcc_votable = create_votable("FCC", VOTE_MIN,
						somc_bcext_fcc_vote_cb,
						bcext_dev);
	if (IS_ERR(bcext_dev->fcc_votable)) {
		rc = PTR_ERR(bcext_dev->fcc_votable);
		bcext_dev->fcc_votable = NULL;
		goto out;
	}
	rc = vote(bcext_dev->fcc_votable, DEFAULT_VOTER, true, MAX_FCC_MA);
	if (rc)
		goto out;

	bcext_dev->fake_chg_votable = create_votable("FAKE_CHG",
						VOTE_SET_ANY,
						somc_bcext_fake_chg_vote_cb,
						bcext_dev);
	if (IS_ERR(bcext_dev->fake_chg_votable)) {
		rc = PTR_ERR(bcext_dev->fake_chg_votable);
		bcext_dev->fake_chg_votable = NULL;
		return rc;
	}

	bcext_dev->fake_chg_disallow_votable = create_votable(
					"FAKE_CHG_DISALLOW", VOTE_SET_ANY,
					somc_bcext_fake_chg_disallow_vote_cb,
					bcext_dev);
	if (IS_ERR(bcext_dev->fake_chg_disallow_votable)) {
		rc = PTR_ERR(bcext_dev->fake_chg_disallow_votable);
		bcext_dev->fake_chg_disallow_votable = NULL;
		return rc;
	}

out:
	return rc;
}

static void somc_bcext_destroy_votables(struct somc_bcext_dev *bcext_dev)
{
	if (bcext_dev->usb_icl_votable) {
		destroy_votable(bcext_dev->usb_icl_votable);
		bcext_dev->usb_icl_votable = NULL;
	}
	if (bcext_dev->fcc_votable) {
		destroy_votable(bcext_dev->fcc_votable);
		bcext_dev->fcc_votable = NULL;
	}
	if (bcext_dev->fake_chg_votable) {
		destroy_votable(bcext_dev->fake_chg_votable);
		bcext_dev->fake_chg_votable = NULL;
	}
	if (bcext_dev->fake_chg_disallow_votable) {
		destroy_votable(bcext_dev->fake_chg_disallow_votable);
		bcext_dev->fake_chg_disallow_votable = NULL;
	}
}

static void somc_bcext_cleanup(struct platform_device *pdev,
					struct somc_bcext_dev *bcext_dev)
{
	int rc;

	class_unregister(&bcext_dev->bcext_class);
	platform_set_drvdata(pdev, NULL);

	if (bcext_dev->glink_client) {
		rc = pmic_glink_unregister_client(bcext_dev->glink_client);
		if (rc < 0)
			pr_err("Error unregistering from pmic_glink, rc=%d\n",
									rc);
		bcext_dev->glink_client = NULL;
	}

	if (bcext_dev->unplug_key) {
		input_free_device(bcext_dev->unplug_key);
		bcext_dev->unplug_key = NULL;
	}

	if (bcext_dev->unplug_wakelock) {
		wakeup_source_unregister(bcext_dev->unplug_wakelock);
		bcext_dev->unplug_wakelock = NULL;
	}

	somc_bcext_destroy_votables(bcext_dev);
}

static int somc_bcext_parse_dt(struct somc_bcext_dev *bcext_dev)
{
	struct device_node *node = bcext_dev->dev->of_node;
	u32 therm_mitig_params[NUM_THERM_MITIG_STEPS];
	int byte_len;
	int i;
	int rc;

	if (!node) {
		pr_err("no dts data\n");
		return -EINVAL;
	}

	rc = of_property_read_u32(node, "somc,bcext-product-code",
						&bcext_dev->product_code);
	if (rc < 0)
		bcext_dev->product_code = 0;

	if (of_find_property(node, "somc,thermal-fcc-ma", &byte_len) &&
			(byte_len / sizeof(u32) == NUM_THERM_MITIG_STEPS)) {
		rc = of_property_read_u32_array(node, "somc,thermal-fcc-ma",
				therm_mitig_params, NUM_THERM_MITIG_STEPS);
		if (rc < 0) {
			pr_err("Couldn't read thermal-fcc-ma rc = %d\n", rc);
			return rc;
		}
		for (i = 0; i < NUM_THERM_MITIG_STEPS; i++)
			bcext_dev->therm_mitig[i].fcc_ma =
							therm_mitig_params[i];
	}

	if (of_find_property(node, "somc,thermal-usb-pd-icl-ma", &byte_len) &&
			(byte_len / sizeof(u32) == NUM_THERM_MITIG_STEPS)) {
		rc = of_property_read_u32_array(node,
				"somc,thermal-usb-pd-icl-ma",
				therm_mitig_params, NUM_THERM_MITIG_STEPS);
		if (rc < 0) {
			pr_err("Couldn't read thermal-usb-pd-icl-ma rc = %d\n",
									rc);
			return rc;
		}
		for (i = 0; i < NUM_THERM_MITIG_STEPS; i++)
			bcext_dev->therm_mitig[i].usb_pd_icl_ma =
							therm_mitig_params[i];
	}

	if (of_find_property(node, "somc,thermal-usb-pd-voltage-mv",
								&byte_len) &&
			(byte_len / sizeof(u32) == NUM_THERM_MITIG_STEPS)) {
		rc = of_property_read_u32_array(node,
				"somc,thermal-usb-pd-voltage-mv",
				therm_mitig_params, NUM_THERM_MITIG_STEPS);
		if (rc < 0) {
			pr_err("Couldn't read thermal-usb-pd-voltage-mv rc = %d\n",
									rc);
			return rc;
		}
		for (i = 0; i < NUM_THERM_MITIG_STEPS; i++)
			bcext_dev->therm_mitig[i].usb_pd_voltage_mv =
							therm_mitig_params[i];
	}

	if (of_find_property(node, "somc,thermal-usb-icl-ma", &byte_len) &&
			(byte_len / sizeof(u32) == NUM_THERM_MITIG_STEPS)) {
		rc = of_property_read_u32_array(node,
				"somc,thermal-usb-icl-ma",
				therm_mitig_params, NUM_THERM_MITIG_STEPS);
		if (rc < 0) {
			pr_err("Couldn't read thermal-usb-icl-ma rc = %d\n",
									rc);
			return rc;
		}
		for (i = 0; i < NUM_THERM_MITIG_STEPS; i++)
			bcext_dev->therm_mitig[i].usb_icl_ma =
							therm_mitig_params[i];
	}

	if (of_find_property(node, "somc,thermal-wls-epp-icl-ma", &byte_len) &&
			(byte_len / sizeof(u32) == NUM_THERM_MITIG_STEPS)) {
		rc = of_property_read_u32_array(node,
				"somc,thermal-wls-epp-icl-ma",
				therm_mitig_params, NUM_THERM_MITIG_STEPS);
		if (rc < 0) {
			pr_err("Couldn't read thermal-wls-epp-icl-ma rc = %d\n",
									rc);
			return rc;
		}
		for (i = 0; i < NUM_THERM_MITIG_STEPS; i++)
			bcext_dev->therm_mitig[i].wls_epp_icl_ma =
							therm_mitig_params[i];
	}

	if (of_find_property(node, "somc,thermal-wls-epp-voltage-mv",
								&byte_len) &&
			(byte_len / sizeof(u32) == NUM_THERM_MITIG_STEPS)) {
		rc = of_property_read_u32_array(node,
				"somc,thermal-wls-epp-voltage-mv",
				therm_mitig_params, NUM_THERM_MITIG_STEPS);
		if (rc < 0) {
			pr_err("Couldn't read thermal-wls-epp-voltage-mv rc = %d\n",
									rc);
			return rc;
		}
		for (i = 0; i < NUM_THERM_MITIG_STEPS; i++)
			bcext_dev->therm_mitig[i].wls_epp_voltage_mv =
							therm_mitig_params[i];
	}

	if (of_find_property(node, "somc,thermal-wls-bpp-icl-ma", &byte_len) &&
			(byte_len / sizeof(u32) == NUM_THERM_MITIG_STEPS)) {
		rc = of_property_read_u32_array(node,
				"somc,thermal-wls-bpp-icl-ma",
				therm_mitig_params, NUM_THERM_MITIG_STEPS);
		if (rc < 0) {
			pr_err("Couldn't read thermal-wls-bpp-icl-ma rc = %d\n",
									rc);
			return rc;
		}
		for (i = 0; i < NUM_THERM_MITIG_STEPS; i++)
			bcext_dev->therm_mitig[i].wls_bpp_icl_ma =
							therm_mitig_params[i];
	}

	if (of_find_property(node, "somc,thermal-charging-indication",
								&byte_len) &&
			(byte_len / sizeof(u32) == NUM_THERM_MITIG_STEPS)) {
		rc = of_property_read_u32_array(node,
				"somc,thermal-charging-indication",
				therm_mitig_params, NUM_THERM_MITIG_STEPS);
		if (rc < 0) {
			pr_err("Couldn't read thermal-charging-indication rc = %d\n",
									rc);
			return rc;
		}
		for (i = 0; i < NUM_THERM_MITIG_STEPS; i++)
			bcext_dev->therm_mitig[i].charging_indication =
							therm_mitig_params[i];
	}

	bcext_dev->wls_gpio_irq = of_get_named_gpio(node,
							"somc,wls-irq-gpio", 0);

	return 0;
}

static int somc_bcext_probe(struct platform_device *pdev)
{
	struct somc_bcext_dev *bcext_dev;
	struct device *dev = &pdev->dev;
	struct pmic_glink_client_data client_data = {};
	int rc = 0;

	bcext_dev = devm_kzalloc(&pdev->dev, sizeof(struct somc_bcext_dev),
								GFP_KERNEL);
	if (!bcext_dev)
		return -ENOMEM;

	bcext_dev->dev = dev;

	bcext_dev->psy_nb.notifier_call = somc_bcext_psy_notifier_cb;
	rc = power_supply_reg_notifier(&bcext_dev->psy_nb);
	if (rc < 0) {
		pr_err("Couldn't register psy notifier rc = %d\n", rc);
		goto error;
	}

	client_data.id = MSG_OWNER_OEM;
	client_data.name = "oem";
	client_data.msg_cb = somc_bcext_glink_cb;
	client_data.priv = bcext_dev;
	client_data.state_cb = somc_bcext_glink_state_cb;

	bcext_dev->glink_client = pmic_glink_register_client(dev, &client_data);
	if (IS_ERR(bcext_dev->glink_client)) {
		rc = PTR_ERR(bcext_dev->glink_client);
		if (rc != -EPROBE_DEFER)
			dev_err(dev,
				"Error in registering with pmic_glink %d\n",
									rc);
		goto error;
	}
	mutex_init(&bcext_dev->glink_rw_lock);
	init_completion(&bcext_dev->glink_ack);
	atomic_set(&bcext_dev->glink_state, PMIC_GLINK_STATE_UP);

	rc = somc_bcext_parse_dt(bcext_dev);
	if (rc < 0)
		goto error;

	rc = somc_bcext_create_votables(bcext_dev);
	if (rc < 0)
		goto error;

	platform_set_drvdata(pdev, bcext_dev);

	INIT_DELAYED_WORK(&bcext_dev->smart_charge_wdog_work,
					somc_bcext_smart_charge_wdog_work);
	INIT_DELAYED_WORK(&bcext_dev->timed_fake_chg_work,
					somc_bcext_timed_fake_chg_work);
	INIT_DELAYED_WORK(&bcext_dev->offchg_termination_work,
					somc_bcext_offchg_termination_work);

	somc_bcext_set_product_code(bcext_dev, (u32)bcext_dev->product_code);
	bcext_dev->usb_type = POWER_SUPPLY_USB_TYPE_UNKNOWN;

	/* virtual device resistration */
	bcext_dev->vcls = class_create(THIS_MODULE, "wlc_switch");
	if (IS_ERR(bcext_dev->vcls)) {
		rc = PTR_ERR(bcext_dev->vcls);
		pr_err("Could not create class rc=%d\n", rc);
		goto error;
	}
	bcext_dev->vcls->dev_groups = wlc_groups;

	bcext_dev->vdev = device_create(bcext_dev->vcls, NULL, MKDEV(0, 0),
					NULL, "wireless_chg");
	if (IS_ERR(bcext_dev->vdev)) {
		pr_err("Could not create device\n");
		rc = -ENODEV;
		goto error;
	}
	dev_set_drvdata(bcext_dev->vdev, bcext_dev);

	/* wakeup source */
	bcext_dev->unplug_wakelock = wakeup_source_register(bcext_dev->dev,
						"unplug_wakelock");
	if (!bcext_dev->unplug_wakelock) {
		dev_err(bcext_dev->dev, "can't create wakeup source\n");
		rc = -ENOMEM;
		goto error;
	}

	/* register input device */
	bcext_dev->unplug_key = input_allocate_device();
	if (!bcext_dev->unplug_key) {
		dev_err(bcext_dev->dev,
				"can't allocate unplug virtual button\n");
		rc = -ENOMEM;
		goto error;
	}

	input_set_capability(bcext_dev->unplug_key, EV_KEY, KEY_F24);
	bcext_dev->unplug_key->name = "SOMC Charger Removal";
	bcext_dev->unplug_key->dev.parent = bcext_dev->dev;
	rc = input_register_device(bcext_dev->unplug_key);
	if (rc) {
		dev_err(bcext_dev->dev, "can't register power key: %d\n", rc);
		rc = -ENOMEM;
		goto error;
	}

	bcext_dev->bcext_class.name = "battchg_ext";
	bcext_dev->bcext_class.class_groups = somc_bcext_class_groups;
	rc = class_register(&bcext_dev->bcext_class);
	if (rc < 0) {
		pr_err("Failed to create battery_class rc=%d\n", rc);
		goto error;
	}

	if (gpio_is_valid(bcext_dev->wls_gpio_irq)) {
		rc = gpio_request(bcext_dev->wls_gpio_irq, "idtp9382a_irq");
		if (rc < 0) {
			dev_err(bcext_dev->dev, "can't request irq (%d)\n",
									rc);
			goto error;
		}

		rc = gpio_direction_input(bcext_dev->wls_gpio_irq);
		if (rc < 0) {
			dev_err(bcext_dev->dev, "can't set input gpio (%d)\n",
									rc);
			goto error;
		}

		bcext_dev->wls_irq = gpio_to_irq(bcext_dev->wls_gpio_irq);
		if (bcext_dev->wls_irq < 0) {
			rc = bcext_dev->wls_irq;
			dev_err(bcext_dev->dev, "can't gpio_to_irq (%d)\n", rc);
			goto error;
		}

		rc = devm_request_threaded_irq(bcext_dev->dev,
				bcext_dev->wls_irq,
				NULL, idtp9382a_irq_handler,
				IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING |
				IRQF_ONESHOT, "idtp9382a", bcext_dev);
		if (rc) {
			dev_err(bcext_dev->dev,
				"can't devm_request_threaded_irq (%d)\n", rc);
			goto error;
		}

		enable_irq_wake(bcext_dev->wls_irq);

		idtp9382a_irq_handler(0, bcext_dev);
	} else {
		pr_err("wls_gpio_irq is not supported\n");
	}

	return 0;

error:
	somc_bcext_cleanup(pdev, bcext_dev);
	return rc;
}

static int somc_bcext_remove(struct platform_device *pdev)
{
	struct somc_bcext_dev *bcext_dev = platform_get_drvdata(pdev);

	somc_bcext_cleanup(pdev, bcext_dev);
	return 0;
}

static const struct of_device_id somc_bcext_match_table[] = {
	{ .compatible = "somc,battery-charger-extension" },
	{},
};

static struct platform_driver somc_bcext_driver = {
	.driver = {
		.name = "somc_battery_charger_extension",
		.of_match_table = somc_bcext_match_table,
	},
	.probe = somc_bcext_probe,
	.remove = somc_bcext_remove,
};
module_platform_driver(somc_bcext_driver);

MODULE_DESCRIPTION("SOMC battery charger extension driver");
MODULE_LICENSE("GPL v2");
