// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2019-2020, The Linux Foundation. All rights reserved.
 */

#include "somc_battman_dbg.h"
#include "somc_battman_reg.h"

/*****************/
/* Log functions */
/*****************/
static void somc_bmdbg_output_ulog(char *buf)
{
	int i;
	char *log_ptr = buf;

	for (i = 0; buf[i] != '\0'; i++) {
		if (buf[i] == '\n') {
			buf[i] = '\0';
			pr_err("battman: %s\n", log_ptr);
			log_ptr = &buf[i + 1];
		}
	}
}

static void somc_bmdbg_log_request_work(struct work_struct *work)
{
	struct somc_bmdbg_dev *bmdbg_dev = container_of(work,
		struct somc_bmdbg_dev, somc_bmdbg_log_request_work.work);
	struct glink_ulog_req_msg req_msg;
	void *data;
	size_t len;

	req_msg.hdr.owner = MSG_OWNER_OEM;
	req_msg.hdr.type = MSG_TYPE_REQ_RESP;
	req_msg.hdr.opcode = MSG_BATT_MNGR_GET_ULOG_REQ;
	req_msg.max_logsize = MAX_ULOG_READ_BUFFER_SIZE;

	data = &req_msg;
	len = sizeof(struct glink_ulog_req_msg);
	pmic_glink_write(bmdbg_dev->glink_client, data, len);

	schedule_delayed_work(&bmdbg_dev->somc_bmdbg_log_request_work,
				msecs_to_jiffies(bmdbg_dev->bmlog_timer));
}

static void somc_set_bmlog_prop(struct somc_bmdbg_dev *bmdbg_dev)
{
	struct glink_ulog_prop_req_msg req_msg;
	void *data;
	size_t len;

	req_msg.hdr.owner = MSG_OWNER_OEM;
	req_msg.hdr.type = MSG_TYPE_REQ_RESP;
	req_msg.hdr.opcode = MSG_BATT_MNGR_SET_LOGGING_PROP_REQ;
	req_msg.categories = bmdbg_dev->bmlog_categories;
	req_msg.level = bmdbg_dev->bmlog_level;

	data = &req_msg;
	len = sizeof(struct glink_ulog_prop_req_msg);
	pmic_glink_write(bmdbg_dev->glink_client, data, len);
}

static ssize_t log_interval_show(struct class *c, struct class_attribute *attr,
								char *buf)
{
	struct somc_bmdbg_dev *bmdbg_dev = container_of(c,
					struct somc_bmdbg_dev, bmdbg_class);

	return scnprintf(buf, PAGE_SIZE, "%d\n", bmdbg_dev->bmlog_timer);
}
static ssize_t log_interval_store(struct class *c,
		struct class_attribute *attr, const char *buf, size_t count)
{
	struct somc_bmdbg_dev *bmdbg_dev = container_of(c,
					struct somc_bmdbg_dev, bmdbg_class);

	kstrtoint(buf, 10, &bmdbg_dev->bmlog_timer);

	cancel_delayed_work_sync(&bmdbg_dev->somc_bmdbg_log_request_work);
	if (bmdbg_dev->bmlog_timer > 0)
		schedule_delayed_work(&bmdbg_dev->somc_bmdbg_log_request_work,
							msecs_to_jiffies(0));

	return count;
}
static CLASS_ATTR_RW(log_interval);

static ssize_t log_categories_show(struct class *c,
				struct class_attribute *attr, char *buf)
{
	struct somc_bmdbg_dev *bmdbg_dev = container_of(c,
					struct somc_bmdbg_dev, bmdbg_class);

	return scnprintf(buf, PAGE_SIZE, "0x%016llx\n",
						bmdbg_dev->bmlog_categories);
}
static ssize_t log_categories_store(struct class *c,
		struct class_attribute *attr, const char *buf, size_t count)
{
	struct somc_bmdbg_dev *bmdbg_dev = container_of(c,
					struct somc_bmdbg_dev, bmdbg_class);

	kstrtou64(buf, 0, &bmdbg_dev->bmlog_categories);

	somc_set_bmlog_prop(bmdbg_dev);

	return count;
}
static CLASS_ATTR_RW(log_categories);

static ssize_t log_level_show(struct class *c, struct class_attribute *attr,
								char *buf)
{
	struct somc_bmdbg_dev *bmdbg_dev = container_of(c,
					struct somc_bmdbg_dev, bmdbg_class);

	return scnprintf(buf, PAGE_SIZE, "%u\n", bmdbg_dev->bmlog_level);
}
static ssize_t log_level_store(struct class *c, struct class_attribute *attr,
						const char *buf, size_t count)
{
	struct somc_bmdbg_dev *bmdbg_dev = container_of(c,
					struct somc_bmdbg_dev, bmdbg_class);

	kstrtou32(buf, 0, &bmdbg_dev->bmlog_level);

	somc_set_bmlog_prop(bmdbg_dev);

	return count;
}
static CLASS_ATTR_RW(log_level);

/**********************/
/* Reg dump functions */
/**********************/
static void somc_bmdbg_reg_read_resp_handler(struct somc_bmdbg_dev *bmdbg_dev,
							void *data, size_t len)
{
	struct reg_dump_read_resp_msg *reg_read_resp_msg;

	if (len != sizeof(struct reg_dump_read_resp_msg)) {
		pr_err("Invalid read response, glink packet size=%zu\n", len);
		goto exit;
	}
	reg_read_resp_msg = (struct reg_dump_read_resp_msg *)data;

	if ((int)reg_read_resp_msg->byte_count < 0) {
		pr_err("glink read failed, byte_count=%d\n",
					(int)reg_read_resp_msg->byte_count);
		goto exit;
	}

	mutex_lock(&bmdbg_dev->reg_read_lock);
	switch (reg_read_resp_msg->address) {
	case CHGR_BASE:
		memcpy(bmdbg_dev->chgr_reg, reg_read_resp_msg->data,
							MAX_REG_DUMP_SIZE);
		break;
	case DCDC_BASE:
		memcpy(bmdbg_dev->dcdc_reg, reg_read_resp_msg->data,
							MAX_REG_DUMP_SIZE);
		break;
	case BATIF_BASE:
		memcpy(bmdbg_dev->batif_reg, reg_read_resp_msg->data,
							MAX_REG_DUMP_SIZE);
		break;
	case USB_BASE:
		memcpy(bmdbg_dev->usb_reg, reg_read_resp_msg->data,
							MAX_REG_DUMP_SIZE);
		break;
	case WLS_BASE:
		memcpy(bmdbg_dev->wls_reg, reg_read_resp_msg->data,
							MAX_REG_DUMP_SIZE);
		break;
	case TYPEC_BASE:
		memcpy(bmdbg_dev->typec_reg, reg_read_resp_msg->data,
							MAX_REG_DUMP_SIZE);
		break;
	case MISC_BASE:
		memcpy(bmdbg_dev->misc_reg, reg_read_resp_msg->data,
							MAX_REG_DUMP_SIZE);
		break;
	default:
		pr_err("Invalid register address=%08x\n",
						reg_read_resp_msg->address);
		break;
	}
	mutex_unlock(&bmdbg_dev->reg_read_lock);

exit:
	complete(&bmdbg_dev->reg_read_ack);
}

static int somc_bmdbg_update_reg(struct somc_bmdbg_dev *bmdbg_dev, u16 addr)
{
	struct reg_dump_read_req_msg reg_read_req_msg = {{0}};
	int rc;

	reg_read_req_msg.hdr.owner = MSG_OWNER_OEM;
	reg_read_req_msg.hdr.type = MSG_TYPE_REQ_RESP;
	reg_read_req_msg.hdr.opcode = MSG_BATT_MNGR_GET_PMIC_REG_REQ;

	reg_read_req_msg.spmi_bus_id = SPMI_BUS_ID;
	reg_read_req_msg.pmic_sid = PMIC_SLAVE_ID;
	reg_read_req_msg.address = addr;
	reg_read_req_msg.byte_count = MAX_REG_DUMP_SIZE;

	reinit_completion(&bmdbg_dev->reg_read_ack);
	rc = pmic_glink_write(bmdbg_dev->glink_client, &reg_read_req_msg,
					sizeof(struct reg_dump_read_req_msg));
	if (rc) {
		pr_err("pmic_glink_write failed.(rc=%d)\n", rc);
		return rc;
	}

	rc = wait_for_completion_timeout(&bmdbg_dev->reg_read_ack,
				msecs_to_jiffies(GLINK_WAIT_TIME_MS));
	if (!rc) {
		pr_err("Error, timed out sending message\n");
		return -ETIMEDOUT;
	}

	return 0;
}

static void somc_bmdbg_reg_read_request_work(struct work_struct *work)
{
	struct somc_bmdbg_dev *bmdbg_dev = container_of(work,
		struct somc_bmdbg_dev, somc_bmdbg_reg_read_request_work.work);

	if (somc_bmdbg_update_reg(bmdbg_dev, CHGR_BASE))
		pr_err("chgr register read error\n");

	if (somc_bmdbg_update_reg(bmdbg_dev, DCDC_BASE))
		pr_err("chgr register read error\n");

	if (somc_bmdbg_update_reg(bmdbg_dev, BATIF_BASE))
		pr_err("batif register read error\n");

	if (somc_bmdbg_update_reg(bmdbg_dev, USB_BASE))
		pr_err("usb register read error\n");

	if (somc_bmdbg_update_reg(bmdbg_dev, WLS_BASE))
		pr_err("wls register read error\n");

	if (somc_bmdbg_update_reg(bmdbg_dev, TYPEC_BASE))
		pr_err("typec register read error\n");

	if (somc_bmdbg_update_reg(bmdbg_dev, MISC_BASE))
		pr_err("misc register read error\n");

	schedule_delayed_work(&bmdbg_dev->somc_bmdbg_reg_read_request_work,
				msecs_to_jiffies(bmdbg_dev->regupdate_timer));
}

static ssize_t regupdate_interval_show(struct class *c,
					struct class_attribute *attr, char *buf)
{
	struct somc_bmdbg_dev *bmdbg_dev = container_of(c,
					struct somc_bmdbg_dev, bmdbg_class);

	return scnprintf(buf, PAGE_SIZE, "%d\n", bmdbg_dev->regupdate_timer);
}
static ssize_t regupdate_interval_store(struct class *c,
		struct class_attribute *attr, const char *buf, size_t count)
{
	struct somc_bmdbg_dev *bmdbg_dev = container_of(c,
					struct somc_bmdbg_dev, bmdbg_class);

	kstrtoint(buf, 10, &bmdbg_dev->regupdate_timer);

	cancel_delayed_work_sync(&bmdbg_dev->somc_bmdbg_reg_read_request_work);
	if (bmdbg_dev->regupdate_timer > 0)
		schedule_delayed_work(
				&bmdbg_dev->somc_bmdbg_reg_read_request_work,
							msecs_to_jiffies(0));

	return count;
}
static CLASS_ATTR_RW(regupdate_interval);

static ssize_t reg_chgr_chager_status_show(
		struct class *c, struct class_attribute *attr, char *buf)
{
	struct somc_bmdbg_dev *bmdbg_dev = container_of(c,
					struct somc_bmdbg_dev, bmdbg_class);
	size_t size;

	mutex_lock(&bmdbg_dev->reg_read_lock);
	size = scnprintf(buf, PAGE_SIZE, "%02X\n",
				bmdbg_dev->chgr_reg[CHGR_CHARGER_STATUS_REG]);
	mutex_unlock(&bmdbg_dev->reg_read_lock);
	return size;
}
static CLASS_ATTR_RO(reg_chgr_chager_status);

static ssize_t reg_chgr_vbat_status_show(
		struct class *c, struct class_attribute *attr, char *buf)
{
	struct somc_bmdbg_dev *bmdbg_dev = container_of(c,
					struct somc_bmdbg_dev, bmdbg_class);
	size_t size;

	mutex_lock(&bmdbg_dev->reg_read_lock);
	size = scnprintf(buf, PAGE_SIZE, "%02X\n",
				bmdbg_dev->chgr_reg[CHGR_VBAT_STATUS_REG]);
	mutex_unlock(&bmdbg_dev->reg_read_lock);
	return size;
}
static CLASS_ATTR_RO(reg_chgr_vbat_status);

static ssize_t reg_chgr_vbat_status2_show(
		struct class *c, struct class_attribute *attr, char *buf)
{
	struct somc_bmdbg_dev *bmdbg_dev = container_of(c,
					struct somc_bmdbg_dev, bmdbg_class);
	size_t size;

	mutex_lock(&bmdbg_dev->reg_read_lock);
	size = scnprintf(buf, PAGE_SIZE, "%02X\n",
				bmdbg_dev->chgr_reg[CHGR_VBAT_STATUS2_REG]);
	mutex_unlock(&bmdbg_dev->reg_read_lock);
	return size;
}
static CLASS_ATTR_RO(reg_chgr_vbat_status2);

static ssize_t reg_chgr_ibat_status_show(
		struct class *c, struct class_attribute *attr, char *buf)
{
	struct somc_bmdbg_dev *bmdbg_dev = container_of(c,
					struct somc_bmdbg_dev, bmdbg_class);
	size_t size;

	mutex_lock(&bmdbg_dev->reg_read_lock);
	size = scnprintf(buf, PAGE_SIZE, "%02X\n",
				bmdbg_dev->chgr_reg[CHGR_IBAT_STATUS_REG]);
	mutex_unlock(&bmdbg_dev->reg_read_lock);
	return size;
}
static CLASS_ATTR_RO(reg_chgr_ibat_status);

static ssize_t reg_chgr_vflt_status_show(
		struct class *c, struct class_attribute *attr, char *buf)
{
	struct somc_bmdbg_dev *bmdbg_dev = container_of(c,
					struct somc_bmdbg_dev, bmdbg_class);
	size_t size;

	mutex_lock(&bmdbg_dev->reg_read_lock);
	size = scnprintf(buf, PAGE_SIZE, "%02X\n",
				bmdbg_dev->chgr_reg[CHGR_VFLT_STATUS_REG]);
	mutex_unlock(&bmdbg_dev->reg_read_lock);
	return size;
}
static CLASS_ATTR_RO(reg_chgr_vflt_status);

static ssize_t reg_chgr_ichg_status_show(
		struct class *c, struct class_attribute *attr, char *buf)
{
	struct somc_bmdbg_dev *bmdbg_dev = container_of(c,
					struct somc_bmdbg_dev, bmdbg_class);
	size_t size;

	mutex_lock(&bmdbg_dev->reg_read_lock);
	size = scnprintf(buf, PAGE_SIZE, "%02X\n",
				bmdbg_dev->chgr_reg[CHGR_ICHG_STATUS_REG]);
	mutex_unlock(&bmdbg_dev->reg_read_lock);
	return size;
}
static CLASS_ATTR_RO(reg_chgr_ichg_status);

static ssize_t reg_chgr_int_rt_sts_show(
		struct class *c, struct class_attribute *attr, char *buf)
{
	struct somc_bmdbg_dev *bmdbg_dev = container_of(c,
					struct somc_bmdbg_dev, bmdbg_class);
	size_t size;

	mutex_lock(&bmdbg_dev->reg_read_lock);
	size = scnprintf(buf, PAGE_SIZE, "%02X\n",
				bmdbg_dev->chgr_reg[CHGR_INT_RT_STS_REG]);
	mutex_unlock(&bmdbg_dev->reg_read_lock);
	return size;
}
static CLASS_ATTR_RO(reg_chgr_int_rt_sts);

static ssize_t reg_chg_enabled_show(
		struct class *c, struct class_attribute *attr, char *buf)
{
	struct somc_bmdbg_dev *bmdbg_dev = container_of(c,
					struct somc_bmdbg_dev, bmdbg_class);
	size_t size;

	mutex_lock(&bmdbg_dev->reg_read_lock);
	size = scnprintf(buf, PAGE_SIZE, "%02X\n",
					bmdbg_dev->chgr_reg[CHGR_CHG_EN_REG]);
	mutex_unlock(&bmdbg_dev->reg_read_lock);
	return size;
}
static CLASS_ATTR_RO(reg_chg_enabled);

static ssize_t reg_fast_chg_current_cfg_show(
		struct class *c, struct class_attribute *attr, char *buf)
{
	struct somc_bmdbg_dev *bmdbg_dev = container_of(c,
					struct somc_bmdbg_dev, bmdbg_class);
	size_t size;

	mutex_lock(&bmdbg_dev->reg_read_lock);
	size = scnprintf(buf, PAGE_SIZE, "%02X\n",
					bmdbg_dev->chgr_reg[CHGR_ICHG_CFG_REG]);
	mutex_unlock(&bmdbg_dev->reg_read_lock);
	return size;
}
static CLASS_ATTR_RO(reg_fast_chg_current_cfg);

static ssize_t reg_float_voltage_cfg_show(
		struct class *c, struct class_attribute *attr, char *buf)
{
	struct somc_bmdbg_dev *bmdbg_dev = container_of(c,
					struct somc_bmdbg_dev, bmdbg_class);
	size_t size;

	mutex_lock(&bmdbg_dev->reg_read_lock);
	size = scnprintf(buf, PAGE_SIZE, "%02X\n",
					bmdbg_dev->chgr_reg[CHGR_VFLT_CFG_REG]);
	mutex_unlock(&bmdbg_dev->reg_read_lock);
	return size;
}
static CLASS_ATTR_RO(reg_float_voltage_cfg);

static ssize_t reg_icl_max_status_show(
		struct class *c, struct class_attribute *attr, char *buf)
{
	struct somc_bmdbg_dev *bmdbg_dev = container_of(c,
					struct somc_bmdbg_dev, bmdbg_class);
	size_t size;

	mutex_lock(&bmdbg_dev->reg_read_lock);
	size = scnprintf(buf, PAGE_SIZE, "%02X\n",
				bmdbg_dev->dcdc_reg[DCDC_ICL_MAX_STATUS_REG]);
	mutex_unlock(&bmdbg_dev->reg_read_lock);
	return size;
}
static CLASS_ATTR_RO(reg_icl_max_status);

static ssize_t reg_aicl_status_show(
		struct class *c, struct class_attribute *attr, char *buf)
{
	struct somc_bmdbg_dev *bmdbg_dev = container_of(c,
					struct somc_bmdbg_dev, bmdbg_class);
	size_t size;

	mutex_lock(&bmdbg_dev->reg_read_lock);
	size = scnprintf(buf, PAGE_SIZE, "%02X\n",
				bmdbg_dev->dcdc_reg[DCDC_ICL_AICL_STATUS_REG]);
	mutex_unlock(&bmdbg_dev->reg_read_lock);
	return size;
}
static CLASS_ATTR_RO(reg_aicl_status);

static ssize_t reg_icl_status_show(
		struct class *c, struct class_attribute *attr, char *buf)
{
	struct somc_bmdbg_dev *bmdbg_dev = container_of(c,
					struct somc_bmdbg_dev, bmdbg_class);
	size_t size;

	mutex_lock(&bmdbg_dev->reg_read_lock);
	size = scnprintf(buf, PAGE_SIZE, "%02X\n",
				bmdbg_dev->chgr_reg[DCDC_ICL_STATUS_REG]);
	mutex_unlock(&bmdbg_dev->reg_read_lock);
	return size;
}
static CLASS_ATTR_RO(reg_icl_status);

static ssize_t reg_power_path_status_show(
		struct class *c, struct class_attribute *attr, char *buf)
{
	struct somc_bmdbg_dev *bmdbg_dev = container_of(c,
					struct somc_bmdbg_dev, bmdbg_class);
	size_t size;

	mutex_lock(&bmdbg_dev->reg_read_lock);
	size = scnprintf(buf, PAGE_SIZE, "%02X\n",
			bmdbg_dev->chgr_reg[DCDC_POWER_PATH_STATUS_REG]);
	mutex_unlock(&bmdbg_dev->reg_read_lock);
	return size;
}
static CLASS_ATTR_RO(reg_power_path_status);

static ssize_t reg_bst_status_show(
		struct class *c, struct class_attribute *attr, char *buf)
{
	struct somc_bmdbg_dev *bmdbg_dev = container_of(c,
					struct somc_bmdbg_dev, bmdbg_class);
	size_t size;

	mutex_lock(&bmdbg_dev->reg_read_lock);
	size = scnprintf(buf, PAGE_SIZE, "%02X\n",
			bmdbg_dev->dcdc_reg[DCDC_BST_STATUS_REG]);
	mutex_unlock(&bmdbg_dev->reg_read_lock);
	return size;
}
static CLASS_ATTR_RO(reg_bst_status);

static ssize_t reg_dcdc_int_rt_sts_show(
		struct class *c, struct class_attribute *attr, char *buf)
{
	struct somc_bmdbg_dev *bmdbg_dev = container_of(c,
					struct somc_bmdbg_dev, bmdbg_class);
	size_t size;

	mutex_lock(&bmdbg_dev->reg_read_lock);
	size = scnprintf(buf, PAGE_SIZE, "%02X\n",
				bmdbg_dev->chgr_reg[DCDC_INT_RT_STS_REG]);
	mutex_unlock(&bmdbg_dev->reg_read_lock);
	return size;
}
static CLASS_ATTR_RO(reg_dcdc_int_rt_sts);

static ssize_t reg_batif_temp_status_show(
		struct class *c, struct class_attribute *attr, char *buf)
{
	struct somc_bmdbg_dev *bmdbg_dev = container_of(c,
					struct somc_bmdbg_dev, bmdbg_class);
	size_t size;

	mutex_lock(&bmdbg_dev->reg_read_lock);
	size = scnprintf(buf, PAGE_SIZE, "%02X\n",
			bmdbg_dev->batif_reg[BATIF_BAT_TEMP_STATUS_REG]);
	mutex_unlock(&bmdbg_dev->reg_read_lock);
	return size;
}
static CLASS_ATTR_RO(reg_batif_temp_status);

static ssize_t reg_batif_int_rt_sts_show(
		struct class *c, struct class_attribute *attr, char *buf)
{
	struct somc_bmdbg_dev *bmdbg_dev = container_of(c,
					struct somc_bmdbg_dev, bmdbg_class);
	size_t size;

	mutex_lock(&bmdbg_dev->reg_read_lock);
	size = scnprintf(buf, PAGE_SIZE, "%02X\n",
				bmdbg_dev->batif_reg[BATIF_INT_RT_STS_REG]);
	mutex_unlock(&bmdbg_dev->reg_read_lock);
	return size;
}
static CLASS_ATTR_RO(reg_batif_int_rt_sts);

static ssize_t reg_batif_ship_mode_show(
		struct class *c, struct class_attribute *attr, char *buf)
{
	struct somc_bmdbg_dev *bmdbg_dev = container_of(c,
					struct somc_bmdbg_dev, bmdbg_class);
	size_t size;

	mutex_lock(&bmdbg_dev->reg_read_lock);
	size = scnprintf(buf, PAGE_SIZE, "%02X\n",
				bmdbg_dev->batif_reg[BATIF_SHIP_MODE_REG]);
	mutex_unlock(&bmdbg_dev->reg_read_lock);
	return size;
}
static CLASS_ATTR_RO(reg_batif_ship_mode);

static ssize_t reg_usbin_input_status_show(
		struct class *c, struct class_attribute *attr, char *buf)
{
	struct somc_bmdbg_dev *bmdbg_dev = container_of(c,
					struct somc_bmdbg_dev, bmdbg_class);
	size_t size;

	mutex_lock(&bmdbg_dev->reg_read_lock);
	size = scnprintf(buf, PAGE_SIZE, "%02X\n",
				bmdbg_dev->usb_reg[USB_USBIN_INPUT_STATUS_REG]);
	mutex_unlock(&bmdbg_dev->reg_read_lock);
	return size;
}
static CLASS_ATTR_RO(reg_usbin_input_status);

static ssize_t reg_vusb_status_show(
		struct class *c, struct class_attribute *attr, char *buf)
{
	struct somc_bmdbg_dev *bmdbg_dev = container_of(c,
					struct somc_bmdbg_dev, bmdbg_class);
	size_t size;

	mutex_lock(&bmdbg_dev->reg_read_lock);
	size = scnprintf(buf, PAGE_SIZE, "%02X\n",
				bmdbg_dev->usb_reg[USB_VUSB_STATUS_REG]);
	mutex_unlock(&bmdbg_dev->reg_read_lock);
	return size;
}
static CLASS_ATTR_RO(reg_vusb_status);

static ssize_t reg_apsd_status_show(
		struct class *c, struct class_attribute *attr, char *buf)
{
	struct somc_bmdbg_dev *bmdbg_dev = container_of(c,
					struct somc_bmdbg_dev, bmdbg_class);
	size_t size;

	mutex_lock(&bmdbg_dev->reg_read_lock);
	size = scnprintf(buf, PAGE_SIZE, "%02X\n",
				bmdbg_dev->usb_reg[USB_APSD_STATUS_REG]);
	mutex_unlock(&bmdbg_dev->reg_read_lock);
	return size;
}
static CLASS_ATTR_RO(reg_apsd_status);

static ssize_t reg_apsd_result_status_show(
		struct class *c, struct class_attribute *attr, char *buf)
{
	struct somc_bmdbg_dev *bmdbg_dev = container_of(c,
					struct somc_bmdbg_dev, bmdbg_class);
	size_t size;

	mutex_lock(&bmdbg_dev->reg_read_lock);
	size = scnprintf(buf, PAGE_SIZE, "%02X\n",
			bmdbg_dev->usb_reg[USB_APSD_RESULT_STATUS_REG]);
	mutex_unlock(&bmdbg_dev->reg_read_lock);
	return size;
}
static CLASS_ATTR_RO(reg_apsd_result_status);

static ssize_t reg_usb_int_rt_sts_show(
		struct class *c, struct class_attribute *attr, char *buf)
{
	struct somc_bmdbg_dev *bmdbg_dev = container_of(c,
					struct somc_bmdbg_dev, bmdbg_class);
	size_t size;

	mutex_lock(&bmdbg_dev->reg_read_lock);
	size = scnprintf(buf, PAGE_SIZE, "%02X\n",
				bmdbg_dev->usb_reg[USB_INT_RT_STS_REG]);
	mutex_unlock(&bmdbg_dev->reg_read_lock);
	return size;
}
static CLASS_ATTR_RO(reg_usb_int_rt_sts);

static ssize_t reg_usbin_adapter_allow_override_show(
		struct class *c, struct class_attribute *attr, char *buf)
{
	struct somc_bmdbg_dev *bmdbg_dev = container_of(c,
					struct somc_bmdbg_dev, bmdbg_class);
	size_t size;

	mutex_lock(&bmdbg_dev->reg_read_lock);
	size = scnprintf(buf, PAGE_SIZE, "%02X\n",
		bmdbg_dev->usb_reg[USB_USBIN_ADAPTER_ALLOW_OVERRIDE_REG]);
	mutex_unlock(&bmdbg_dev->reg_read_lock);
	return size;
}
static CLASS_ATTR_RO(reg_usbin_adapter_allow_override);

static ssize_t reg_usbin_adapter_allow_cfg_show(
		struct class *c, struct class_attribute *attr, char *buf)
{
	struct somc_bmdbg_dev *bmdbg_dev = container_of(c,
					struct somc_bmdbg_dev, bmdbg_class);
	size_t size;

	mutex_lock(&bmdbg_dev->reg_read_lock);
	size = scnprintf(buf, PAGE_SIZE, "%02X\n",
			bmdbg_dev->usb_reg[USB_USBIN_ADAPTER_ALLOW_CFG_REG]);
	mutex_unlock(&bmdbg_dev->reg_read_lock);
	return size;
}
static CLASS_ATTR_RO(reg_usbin_adapter_allow_cfg);

static ssize_t reg_apsd_enabled_show(
		struct class *c, struct class_attribute *attr, char *buf)
{
	struct somc_bmdbg_dev *bmdbg_dev = container_of(c,
					struct somc_bmdbg_dev, bmdbg_class);
	size_t size;

	mutex_lock(&bmdbg_dev->reg_read_lock);
	size = scnprintf(buf, PAGE_SIZE, "%02X\n",
			bmdbg_dev->usb_reg[USB_APSD_EN_REG]);
	mutex_unlock(&bmdbg_dev->reg_read_lock);
	return size;
}
static CLASS_ATTR_RO(reg_apsd_enabled);

static ssize_t reg_usbin_current_limit_cfg_show(
		struct class *c, struct class_attribute *attr, char *buf)
{
	struct somc_bmdbg_dev *bmdbg_dev = container_of(c,
					struct somc_bmdbg_dev, bmdbg_class);
	size_t size;

	mutex_lock(&bmdbg_dev->reg_read_lock);
	size = scnprintf(buf, PAGE_SIZE, "%02X\n",
				bmdbg_dev->usb_reg[USB_USB_ICL_CFG_REG]);
	mutex_unlock(&bmdbg_dev->reg_read_lock);
	return size;
}
static CLASS_ATTR_RO(reg_usbin_current_limit_cfg);

static ssize_t reg_usb_cmd_il_show(
		struct class *c, struct class_attribute *attr, char *buf)
{
	struct somc_bmdbg_dev *bmdbg_dev = container_of(c,
					struct somc_bmdbg_dev, bmdbg_class);
	size_t size;

	mutex_lock(&bmdbg_dev->reg_read_lock);
	size = scnprintf(buf, PAGE_SIZE, "%02X\n",
				bmdbg_dev->usb_reg[USB_USB_SUSPEND_REG]);
	mutex_unlock(&bmdbg_dev->reg_read_lock);
	return size;
}
static CLASS_ATTR_RO(reg_usb_cmd_il);

static ssize_t reg_wls_input_status_show(
		struct class *c, struct class_attribute *attr, char *buf)
{
	struct somc_bmdbg_dev *bmdbg_dev = container_of(c,
					struct somc_bmdbg_dev, bmdbg_class);
	size_t size;

	mutex_lock(&bmdbg_dev->reg_read_lock);
	size = scnprintf(buf, PAGE_SIZE, "%02X\n",
				bmdbg_dev->wls_reg[WLS_INPUT_STATUS_REG]);
	mutex_unlock(&bmdbg_dev->reg_read_lock);
	return size;
}
static CLASS_ATTR_RO(reg_wls_input_status);

static ssize_t reg_wls_vwls_status_show(
		struct class *c, struct class_attribute *attr, char *buf)
{
	struct somc_bmdbg_dev *bmdbg_dev = container_of(c,
					struct somc_bmdbg_dev, bmdbg_class);
	size_t size;

	mutex_lock(&bmdbg_dev->reg_read_lock);
	size = scnprintf(buf, PAGE_SIZE, "%02X\n",
				bmdbg_dev->wls_reg[WLS_VWLS_STATUS_REG]);
	mutex_unlock(&bmdbg_dev->reg_read_lock);
	return size;
}
static CLASS_ATTR_RO(reg_wls_vwls_status);

static ssize_t reg_wls_int_rt_sts_show(
		struct class *c, struct class_attribute *attr, char *buf)
{
	struct somc_bmdbg_dev *bmdbg_dev = container_of(c,
					struct somc_bmdbg_dev, bmdbg_class);
	size_t size;

	mutex_lock(&bmdbg_dev->reg_read_lock);
	size = scnprintf(buf, PAGE_SIZE, "%02X\n",
				bmdbg_dev->wls_reg[WLS_INT_RT_STS]);
	mutex_unlock(&bmdbg_dev->reg_read_lock);
	return size;
}
static CLASS_ATTR_RO(reg_wls_int_rt_sts);

static ssize_t reg_wls_adapter_allow_cfg_show(
		struct class *c, struct class_attribute *attr, char *buf)
{
	struct somc_bmdbg_dev *bmdbg_dev = container_of(c,
					struct somc_bmdbg_dev, bmdbg_class);
	size_t size;

	mutex_lock(&bmdbg_dev->reg_read_lock);
	size = scnprintf(buf, PAGE_SIZE, "%02X\n",
				bmdbg_dev->wls_reg[WLS_ADAPTER_ALLOW_CFG_REG]);
	mutex_unlock(&bmdbg_dev->reg_read_lock);
	return size;
}
static CLASS_ATTR_RO(reg_wls_adapter_allow_cfg);

static ssize_t reg_wls_icl_cfg_show(
		struct class *c, struct class_attribute *attr, char *buf)
{
	struct somc_bmdbg_dev *bmdbg_dev = container_of(c,
					struct somc_bmdbg_dev, bmdbg_class);
	size_t size;

	mutex_lock(&bmdbg_dev->reg_read_lock);
	size = scnprintf(buf, PAGE_SIZE, "%02X\n",
				bmdbg_dev->wls_reg[WLS_ICL_CFG_REG]);
	mutex_unlock(&bmdbg_dev->reg_read_lock);
	return size;
}
static CLASS_ATTR_RO(reg_wls_icl_cfg);

static ssize_t reg_wls_suspend_show(
		struct class *c, struct class_attribute *attr, char *buf)
{
	struct somc_bmdbg_dev *bmdbg_dev = container_of(c,
					struct somc_bmdbg_dev, bmdbg_class);
	size_t size;

	mutex_lock(&bmdbg_dev->reg_read_lock);
	size = scnprintf(buf, PAGE_SIZE, "%02X\n",
				bmdbg_dev->wls_reg[WLS_SUSPEND_REG]);
	mutex_unlock(&bmdbg_dev->reg_read_lock);
	return size;
}
static CLASS_ATTR_RO(reg_wls_suspend);

static ssize_t reg_type_c_snk_status_show(
		struct class *c, struct class_attribute *attr, char *buf)
{
	struct somc_bmdbg_dev *bmdbg_dev = container_of(c,
					struct somc_bmdbg_dev, bmdbg_class);
	size_t size;

	mutex_lock(&bmdbg_dev->reg_read_lock);
	size = scnprintf(buf, PAGE_SIZE, "%02X\n",
			bmdbg_dev->typec_reg[TYPEC_TYPE_C_SNK_STATUS_REG]);
	mutex_unlock(&bmdbg_dev->reg_read_lock);
	return size;
}
static CLASS_ATTR_RO(reg_type_c_snk_status);

static ssize_t reg_type_c_snk_debug_access_status_show(
		struct class *c, struct class_attribute *attr, char *buf)
{
	struct somc_bmdbg_dev *bmdbg_dev = container_of(c,
					struct somc_bmdbg_dev, bmdbg_class);
	size_t size;

	mutex_lock(&bmdbg_dev->reg_read_lock);
	size = scnprintf(buf, PAGE_SIZE, "%02X\n",
		bmdbg_dev->typec_reg[TYPEC_TYPE_C_SNK_DBG_ACCESS_STATUS_REG]);
	mutex_unlock(&bmdbg_dev->reg_read_lock);
	return size;
}
static CLASS_ATTR_RO(reg_type_c_snk_debug_access_status);

static ssize_t reg_type_c_src_status_show(
		struct class *c, struct class_attribute *attr, char *buf)
{
	struct somc_bmdbg_dev *bmdbg_dev = container_of(c,
					struct somc_bmdbg_dev, bmdbg_class);
	size_t size;

	mutex_lock(&bmdbg_dev->reg_read_lock);
	size = scnprintf(buf, PAGE_SIZE, "%02X\n",
			bmdbg_dev->typec_reg[TYPEC_TYPE_C_SRC_STATUS_REG]);
	mutex_unlock(&bmdbg_dev->reg_read_lock);
	return size;
}
static CLASS_ATTR_RO(reg_type_c_src_status);

static ssize_t reg_type_c_state_machine_status_show(
		struct class *c, struct class_attribute *attr, char *buf)
{
	struct somc_bmdbg_dev *bmdbg_dev = container_of(c,
					struct somc_bmdbg_dev, bmdbg_class);
	size_t size;

	mutex_lock(&bmdbg_dev->reg_read_lock);
	size = scnprintf(buf, PAGE_SIZE, "%02X\n",
		bmdbg_dev->typec_reg[TYPEC_TYPE_C_STATE_MACHINE_STATUS_REG]);
	mutex_unlock(&bmdbg_dev->reg_read_lock);
	return size;
}
static CLASS_ATTR_RO(reg_type_c_state_machine_status);

static ssize_t reg_type_c_misc_status_show(
		struct class *c, struct class_attribute *attr, char *buf)
{
	struct somc_bmdbg_dev *bmdbg_dev = container_of(c,
					struct somc_bmdbg_dev, bmdbg_class);
	size_t size;

	mutex_lock(&bmdbg_dev->reg_read_lock);
	size = scnprintf(buf, PAGE_SIZE, "%02X\n",
			bmdbg_dev->typec_reg[TYPEC_TYPE_C_MISC_STATUS_REG]);
	mutex_unlock(&bmdbg_dev->reg_read_lock);
	return size;
}
static CLASS_ATTR_RO(reg_type_c_misc_status);

static ssize_t reg_type_c_try_snk_src_status_show(
		struct class *c, struct class_attribute *attr, char *buf)
{
	struct somc_bmdbg_dev *bmdbg_dev = container_of(c,
					struct somc_bmdbg_dev, bmdbg_class);
	size_t size;

	mutex_lock(&bmdbg_dev->reg_read_lock);
	size = scnprintf(buf, PAGE_SIZE, "%02X\n",
		bmdbg_dev->typec_reg[TYPEC_TYPE_C_TRY_SNK_SRC_STATUS_REG]);
	mutex_unlock(&bmdbg_dev->reg_read_lock);
	return size;
}
static CLASS_ATTR_RO(reg_type_c_try_snk_src_status);

static ssize_t reg_type_c_legacy_cable_status_show(
		struct class *c, struct class_attribute *attr, char *buf)
{
	struct somc_bmdbg_dev *bmdbg_dev = container_of(c,
					struct somc_bmdbg_dev, bmdbg_class);
	size_t size;

	mutex_lock(&bmdbg_dev->reg_read_lock);
	size = scnprintf(buf, PAGE_SIZE, "%02X\n",
		bmdbg_dev->typec_reg[TYPEC_TYPE_C_LEGACY_CABLE_STATUS_REG]);
	mutex_unlock(&bmdbg_dev->reg_read_lock);
	return size;
}
static CLASS_ATTR_RO(reg_type_c_legacy_cable_status);

static ssize_t reg_typec_int_rt_sts_show(
		struct class *c, struct class_attribute *attr, char *buf)
{
	struct somc_bmdbg_dev *bmdbg_dev = container_of(c,
					struct somc_bmdbg_dev, bmdbg_class);
	size_t size;

	mutex_lock(&bmdbg_dev->reg_read_lock);
	size = scnprintf(buf, PAGE_SIZE, "%02X\n",
				bmdbg_dev->typec_reg[TYPEC_INT_RT_STS_REG]);
	mutex_unlock(&bmdbg_dev->reg_read_lock);
	return size;
}
static CLASS_ATTR_RO(reg_typec_int_rt_sts);

static ssize_t reg_type_c_mode_cfg_show(
		struct class *c, struct class_attribute *attr, char *buf)
{
	struct somc_bmdbg_dev *bmdbg_dev = container_of(c,
					struct somc_bmdbg_dev, bmdbg_class);
	size_t size;

	mutex_lock(&bmdbg_dev->reg_read_lock);
	size = scnprintf(buf, PAGE_SIZE, "%02X\n",
			bmdbg_dev->typec_reg[TYPEC_TYPE_C_MODE_CFG_REG]);
	mutex_unlock(&bmdbg_dev->reg_read_lock);
	return size;
}
static CLASS_ATTR_RO(reg_type_c_mode_cfg);

static ssize_t reg_misc_aicl_status_show(
		struct class *c, struct class_attribute *attr, char *buf)
{
	struct somc_bmdbg_dev *bmdbg_dev = container_of(c,
					struct somc_bmdbg_dev, bmdbg_class);
	size_t size;

	mutex_lock(&bmdbg_dev->reg_read_lock);
	size = scnprintf(buf, PAGE_SIZE, "%02X\n",
				bmdbg_dev->misc_reg[MISC_AICL_STATUS_REG]);
	mutex_unlock(&bmdbg_dev->reg_read_lock);
	return size;
}
static CLASS_ATTR_RO(reg_misc_aicl_status);

static ssize_t reg_misc_wdog_status_show(
		struct class *c, struct class_attribute *attr, char *buf)
{
	struct somc_bmdbg_dev *bmdbg_dev = container_of(c,
					struct somc_bmdbg_dev, bmdbg_class);
	size_t size;

	mutex_lock(&bmdbg_dev->reg_read_lock);
	size = scnprintf(buf, PAGE_SIZE, "%02X\n",
				bmdbg_dev->misc_reg[MISC_WDOG_STATUS_REG]);
	mutex_unlock(&bmdbg_dev->reg_read_lock);
	return size;
}
static CLASS_ATTR_RO(reg_misc_wdog_status);

static ssize_t reg_misc_sysok_reason_status_show(
		struct class *c, struct class_attribute *attr, char *buf)
{
	struct somc_bmdbg_dev *bmdbg_dev = container_of(c,
					struct somc_bmdbg_dev, bmdbg_class);
	size_t size;

	mutex_lock(&bmdbg_dev->reg_read_lock);
	size = scnprintf(buf, PAGE_SIZE, "%02X\n",
			bmdbg_dev->misc_reg[MISC_SYSOK_REASON_STATUS_REG]);
	mutex_unlock(&bmdbg_dev->reg_read_lock);
	return size;
}
static CLASS_ATTR_RO(reg_misc_sysok_reason_status);

static ssize_t reg_misc_int_rt_sts_show(
		struct class *c, struct class_attribute *attr, char *buf)
{
	struct somc_bmdbg_dev *bmdbg_dev = container_of(c,
					struct somc_bmdbg_dev, bmdbg_class);
	size_t size;

	mutex_lock(&bmdbg_dev->reg_read_lock);
	size = scnprintf(buf, PAGE_SIZE, "%02X\n",
				bmdbg_dev->misc_reg[MISC_INT_RT_STS_REG]);
	mutex_unlock(&bmdbg_dev->reg_read_lock);
	return size;
}
static CLASS_ATTR_RO(reg_misc_int_rt_sts);

/*************/
/* Callbacks */
/*************/
static int somc_bmdbg_glink_cb(void *priv, void *data, size_t len)
{
	struct pmic_glink_hdr *hdr = (struct pmic_glink_hdr *)data;
	struct glink_ulog_resp_msg *ulog_resp_msg;
	struct somc_bmdbg_dev *bmdbg_dev = priv;

	pr_debug("owner: %u type: %u opcode: %#x len: %zu\n",
				hdr->owner, hdr->type, hdr->opcode, len);

	if (len < sizeof(struct pmic_glink_hdr) || hdr->owner !=
								MSG_OWNER_OEM)
		return 0;

	if (hdr->opcode == MSG_BATT_MNGR_GET_ULOG_REQ) {
		ulog_resp_msg = (struct glink_ulog_resp_msg *)data;
		switch (ulog_resp_msg->hdr.type) {
		case MSG_TYPE_REQ_RESP:
			somc_bmdbg_output_ulog(ulog_resp_msg->read_buf);
			break;
		default:
			break;
		}
	} else if (hdr->opcode == MSG_BATT_MNGR_GET_PMIC_REG_REQ) {
		switch (hdr->type) {
		case MSG_TYPE_REQ_RESP:
			somc_bmdbg_reg_read_resp_handler(bmdbg_dev, data, len);
			break;
		default:
			break;
		}
	}

	return 0;
}

static void somc_bmdbg_glink_state_cb(void *priv, enum pmic_glink_state state)
{
	struct somc_bmdbg_dev *bmdbg_dev = priv;

	pr_debug("state: %d\n", state);

	atomic_set(&bmdbg_dev->glink_state, state);
}

/******************/
/* Initialization */
/******************/
static struct attribute *somc_bmdbg_class_attrs[] = {
	&class_attr_log_interval.attr,
	&class_attr_log_categories.attr,
	&class_attr_log_level.attr,
	&class_attr_regupdate_interval.attr,
	&class_attr_reg_chgr_chager_status.attr,
	&class_attr_reg_chgr_vbat_status.attr,
	&class_attr_reg_chgr_vbat_status2.attr,
	&class_attr_reg_chgr_ibat_status.attr,
	&class_attr_reg_chgr_vflt_status.attr,
	&class_attr_reg_chgr_ichg_status.attr,
	&class_attr_reg_chgr_int_rt_sts.attr,
	&class_attr_reg_chg_enabled.attr,
	&class_attr_reg_fast_chg_current_cfg.attr,
	&class_attr_reg_float_voltage_cfg.attr,
	&class_attr_reg_icl_max_status.attr,
	&class_attr_reg_aicl_status.attr,
	&class_attr_reg_icl_status.attr,
	&class_attr_reg_power_path_status.attr,
	&class_attr_reg_bst_status.attr,
	&class_attr_reg_dcdc_int_rt_sts.attr,
	&class_attr_reg_batif_temp_status.attr,
	&class_attr_reg_batif_int_rt_sts.attr,
	&class_attr_reg_batif_ship_mode.attr,
	&class_attr_reg_usbin_input_status.attr,
	&class_attr_reg_vusb_status.attr,
	&class_attr_reg_apsd_status.attr,
	&class_attr_reg_apsd_result_status.attr,
	&class_attr_reg_usb_int_rt_sts.attr,
	&class_attr_reg_usbin_adapter_allow_override.attr,
	&class_attr_reg_usbin_adapter_allow_cfg.attr,
	&class_attr_reg_apsd_enabled.attr,
	&class_attr_reg_usbin_current_limit_cfg.attr,
	&class_attr_reg_usb_cmd_il.attr,
	&class_attr_reg_wls_input_status.attr,
	&class_attr_reg_wls_vwls_status.attr,
	&class_attr_reg_wls_int_rt_sts.attr,
	&class_attr_reg_wls_adapter_allow_cfg.attr,
	&class_attr_reg_wls_icl_cfg.attr,
	&class_attr_reg_wls_suspend.attr,
	&class_attr_reg_type_c_snk_status.attr,
	&class_attr_reg_type_c_snk_debug_access_status.attr,
	&class_attr_reg_type_c_src_status.attr,
	&class_attr_reg_type_c_state_machine_status.attr,
	&class_attr_reg_type_c_misc_status.attr,
	&class_attr_reg_type_c_try_snk_src_status.attr,
	&class_attr_reg_type_c_legacy_cable_status.attr,
	&class_attr_reg_typec_int_rt_sts.attr,
	&class_attr_reg_type_c_mode_cfg.attr,
	&class_attr_reg_misc_aicl_status.attr,
	&class_attr_reg_misc_wdog_status.attr,
	&class_attr_reg_misc_sysok_reason_status.attr,
	&class_attr_reg_misc_int_rt_sts.attr,
	NULL,
};
ATTRIBUTE_GROUPS(somc_bmdbg_class);

static int somc_bmdbg_parse_dt(struct somc_bmdbg_dev *bmdbg_dev)
{
/*
	struct device_node *node = bmdbg_dev->dev->of_node;
*/

	return 0;
}

static void somc_bmdbg_cleanup(struct platform_device *pdev,
					struct somc_bmdbg_dev *bmdbg_dev)
{
	int rc;

	class_unregister(&bmdbg_dev->bmdbg_class);
	platform_set_drvdata(pdev, NULL);

	if (bmdbg_dev->glink_client) {
		rc = pmic_glink_unregister_client(bmdbg_dev->glink_client);
		if (rc < 0)
			pr_err("Error unregistering from pmic_glink, rc=%d\n",
									rc);
		bmdbg_dev->glink_client = NULL;
	}
}

static int somc_bmdbg_probe(struct platform_device *pdev)
{
	struct somc_bmdbg_dev *bmdbg_dev;
	struct device *dev = &pdev->dev;
	struct pmic_glink_client_data client_data = {};
	int rc = 0;

	bmdbg_dev = devm_kzalloc(&pdev->dev, sizeof(struct somc_bmdbg_dev),
								GFP_KERNEL);
	if (!bmdbg_dev)
		return -ENOMEM;

	bmdbg_dev->dev = dev;
	bmdbg_dev->bmlog_timer = BMLOG_DELAY_TIME_MS_DEFAULT;
	bmdbg_dev->bmlog_categories = BMLOG_CATEGORIES_DEFAULT;
	bmdbg_dev->bmlog_level = BMLOG_LEVEL_DEFAULT;

	client_data.id = MSG_OWNER_OEM;
	client_data.name = "ulog";
	client_data.msg_cb = somc_bmdbg_glink_cb;
	client_data.priv = bmdbg_dev;
	client_data.state_cb = somc_bmdbg_glink_state_cb;

	bmdbg_dev->glink_client = pmic_glink_register_client(dev, &client_data);
	if (IS_ERR(bmdbg_dev->glink_client)) {
		rc = PTR_ERR(bmdbg_dev->glink_client);
		if (rc != -EPROBE_DEFER)
			dev_err(dev,
				"Error in registering with pmic_glink %d\n",
									rc);
		bmdbg_dev->glink_client = NULL;
		goto error;
	}
	mutex_init(&bmdbg_dev->reg_read_lock);
	init_completion(&bmdbg_dev->reg_read_ack);
	atomic_set(&bmdbg_dev->glink_state, PMIC_GLINK_STATE_UP);

	somc_set_bmlog_prop(bmdbg_dev);

	rc = somc_bmdbg_parse_dt(bmdbg_dev);
	if (rc < 0)
		goto error;

	platform_set_drvdata(pdev, bmdbg_dev);

	INIT_DELAYED_WORK(&bmdbg_dev->somc_bmdbg_log_request_work,
						somc_bmdbg_log_request_work);
	INIT_DELAYED_WORK(&bmdbg_dev->somc_bmdbg_reg_read_request_work,
					somc_bmdbg_reg_read_request_work);

	bmdbg_dev->bmdbg_class.name = "battman_dbg";
	bmdbg_dev->bmdbg_class.class_groups = somc_bmdbg_class_groups;
	rc = class_register(&bmdbg_dev->bmdbg_class);
	if (rc < 0) {
		pr_err("Failed to create somc_bmdbg_class rc=%d\n", rc);
		goto error;
	}

	return 0;

error:
	somc_bmdbg_cleanup(pdev, bmdbg_dev);
	return rc;
}

static int somc_bmdbg_remove(struct platform_device *pdev)
{
	struct somc_bmdbg_dev *bmdbg_dev = platform_get_drvdata(pdev);

	somc_bmdbg_cleanup(pdev, bmdbg_dev);
	return 0;
}

static const struct of_device_id somc_bmdbg_match_table[] = {
	{ .compatible = "somc,battman-log" },
	{},
};

static struct platform_driver somc_bmdbg_driver = {
	.driver = {
		.name = "somc_battman_dbg",
		.of_match_table = somc_bmdbg_match_table,
	},
	.probe = somc_bmdbg_probe,
	.remove = somc_bmdbg_remove,
};
module_platform_driver(somc_bmdbg_driver);

MODULE_DESCRIPTION("SOMC battery manager debug driver");
MODULE_LICENSE("GPL v2");
