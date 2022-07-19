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
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/completion.h>
#include <linux/iio/consumer.h>
#include <linux/version.h>
#include <linux/usb/typec/sm5038/sm5038_pd.h>
#include <linux/usb/typec/sm5038/sm5038_typec.h>
#include <linux/extcon-provider.h>
#include <linux/power/sm5038.h>
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
#include <linux/debugfs.h>
#endif

/*PDX225 code for QN5965F-837  at 2021/11/17 start*/
#if defined(CONFIG_SERIAL_MSM_GENI) && defined(CONFIG_SERIAL_MSM_GENI_CONSOLE)
extern int msm_geni_serial_gpio_suspend(bool state);
#endif
/*PDX225 code for QN5965F-837  at 2021/11/17 end*/

static usbpd_phy_ops_type sm5038_ops;
static struct sm5038_phydrv_data *static_pdic_data;
#define USB_MAX_VOLT		9000000 /* 9V */
static enum power_supply_property usbpd_props[] = {
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_CURRENT_NOW,
	POWER_SUPPLY_PROP_CURRENT_MAX,
	POWER_SUPPLY_PROP_TYPE,
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	POWER_SUPPLY_PROP_USB_TYPE,
#endif
	POWER_SUPPLY_PROP_VOLTAGE_MAX,
	POWER_SUPPLY_PROP_VOLTAGE_MAX_DESIGN,
	POWER_SUPPLY_PROP_SCOPE,
	POWER_SUPPLY_PROP_INPUT_CURRENT_LIMIT,
	POWER_SUPPLY_PROP_POWER_NOW,
};
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
static enum power_supply_usb_type usb_psy_supported_types[] = {
	POWER_SUPPLY_USB_TYPE_UNKNOWN,
	POWER_SUPPLY_USB_TYPE_SDP,
	POWER_SUPPLY_USB_TYPE_DCP,
	POWER_SUPPLY_USB_TYPE_CDP,
	POWER_SUPPLY_USB_TYPE_PD,
};
#endif

static char *usbpd_supplied_to[] = {
	"sm5038-usbpd",
};

static const unsigned int usbpd_extcon_cable[] = {
	EXTCON_USB,
	EXTCON_USB_HOST,
	EXTCON_DISP_DP,
	EXTCON_NONE,
};

enum plug_orientation {
	ORIENTATION_NONE,
	ORIENTATION_CC1,
	ORIENTATION_CC2,
};

static int sm5038_usbpd_reg_init(struct sm5038_phydrv_data *_data);
static void sm5038_driver_reset(void *_data);
static int sm5038_get_plug_orientation(struct sm5038_phydrv_data *_data);

static int sm5038_usbpd_read_reg(struct i2c_client *i2c, u8 reg, u8 *dest)
{
	int ret;
	struct device *dev = &i2c->dev;

	ret = i2c_smbus_read_byte_data(i2c, reg);
	if (ret < 0) {
		dev_err(dev, "%s reg(0x%x), ret(%d)\n", __func__, reg, ret);
		return ret;
	}
	ret &= 0xff;
	*dest = ret;
	return 0;
}

static int sm5038_usbpd_write_reg(struct i2c_client *i2c, u8 reg, u8 value)
{
	int ret;
	struct device *dev = &i2c->dev;

	ret = i2c_smbus_write_byte_data(i2c, reg, value);
	if (ret < 0) {
		dev_err(dev, "%s reg(0x%x), ret(%d)\n", __func__, reg, ret);
	}
	return ret;
}

static int sm5038_usbpd_multi_read(struct i2c_client *i2c,
		u8 reg, int count, u8 *buf)
{
	int ret;
	struct device *dev = &i2c->dev;

	ret = i2c_smbus_read_i2c_block_data(i2c, reg, count, buf);
	if (ret < 0) {
		dev_err(dev, "%s reg(0x%x), ret(%d)\n", __func__, reg, ret);
		return ret;
	}
	return 0;
}

static int sm5038_usbpd_multi_write(struct i2c_client *i2c,
		u8 reg, int count, u8 *buf)
{
	int ret;
	struct device *dev = &i2c->dev;

	ret = i2c_smbus_write_i2c_block_data(i2c, reg, count, buf);
	if (ret < 0) {
		dev_err(dev, "%s reg(0x%x), ret(%d)\n", __func__, reg, ret);
		return ret;
	}
	return 0;
}

int sm5038_charger_cable_type_update(int cable_type)
{
	struct sm5038_phydrv_data *pdic_data = static_pdic_data;

	if (pdic_data == NULL) {
		pr_err("%s NULL data\n", __func__);
		return -ENXIO;
	}	

	pdic_data->chg_cable_type = cable_type;
	return 0;
}
EXPORT_SYMBOL_GPL(sm5038_charger_cable_type_update);

int sm5038_charger_charge_mode_update(int charge_mode)
{
	struct sm5038_phydrv_data *pdic_data = static_pdic_data;

	if (pdic_data == NULL) {
		pr_err("%s NULL data\n", __func__);
		return -ENXIO;
	}	

	pdic_data->chg_charge_mode = charge_mode;
	return 0;
}
EXPORT_SYMBOL_GPL(sm5038_charger_charge_mode_update);

int sm5038_charger_chg_on_status_update(int chg_on_status)
{
	struct sm5038_phydrv_data *pdic_data = static_pdic_data;

	if (pdic_data == NULL) {
		pr_err("%s NULL data\n", __func__);
		return -ENXIO;
	}	

	pdic_data->chg_on_status = chg_on_status;
	return 0;
}
EXPORT_SYMBOL_GPL(sm5038_charger_chg_on_status_update);

int sm5038_charger_input_current_update(int input_current)
{
	struct sm5038_phydrv_data *pdic_data = static_pdic_data;

	if (pdic_data == NULL) {
		pr_err("%s NULL data\n", __func__);
		return -ENXIO;
	}	

	pdic_data->chg_input_current = input_current;
	return 0;
}
EXPORT_SYMBOL_GPL(sm5038_charger_input_current_update);

static inline void stop_usb_host(struct sm5038_phydrv_data *_data)
{
	extcon_set_state_sync(_data->extcon, EXTCON_USB_HOST, 0);
}

static inline void start_usb_host(struct sm5038_phydrv_data *_data, bool ss)
{
	enum plug_orientation cc = sm5038_get_plug_orientation(_data);
	union extcon_property_value val;

	val.intval = (cc == ORIENTATION_CC2);
	extcon_set_property(_data->extcon, EXTCON_USB_HOST,
			EXTCON_PROP_USB_TYPEC_POLARITY, val);

	val.intval = ss;
	extcon_set_property(_data->extcon, EXTCON_USB_HOST,
			EXTCON_PROP_USB_SS, val);

	extcon_set_state_sync(_data->extcon, EXTCON_USB_HOST, 1);
	pr_info("%s, USB SS : %d\n", __func__, ss);
}

static inline void stop_usb_peripheral(struct sm5038_phydrv_data *_data)
{
	extcon_set_state_sync(_data->extcon, EXTCON_USB, 0);
}

static inline void start_usb_peripheral(struct sm5038_phydrv_data *_data)
{
	enum plug_orientation cc = sm5038_get_plug_orientation(_data);
	union extcon_property_value val;

	val.intval = (cc == ORIENTATION_CC2);
	extcon_set_property(_data->extcon, EXTCON_USB,
			EXTCON_PROP_USB_TYPEC_POLARITY, val);

	val.intval = 1;
	extcon_set_property(_data->extcon, EXTCON_USB, EXTCON_PROP_USB_SS, val);

	extcon_set_state_sync(_data->extcon, EXTCON_USB, 1);
}

static int sm5038_read_rx_header(struct i2c_client *i2c,
		msg_header *header)
{
	int ret;

	ret = sm5038_usbpd_multi_read(i2c,
			SM5038_REG_RX_HEADER_00, 2, header->byte);
	return ret;
}

static int sm5038_write_tx_header(struct i2c_client *i2c,
		u8 *buf)
{
	int ret;

	ret = sm5038_usbpd_multi_write(i2c, SM5038_REG_TX_HEADER_00, 2, buf);
	return ret;
}

static void sm5038_set_dfp(struct i2c_client *i2c)
{
	u8 data;

	sm5038_usbpd_read_reg(i2c, SM5038_REG_PD_CNTL2, &data);
	data |= 0x01;
	sm5038_usbpd_write_reg(i2c, SM5038_REG_PD_CNTL2, data);
}

static void sm5038_set_ufp(struct i2c_client *i2c)
{
	u8 data;

	sm5038_usbpd_read_reg(i2c, SM5038_REG_PD_CNTL2, &data);
	data &= ~0x01;
	sm5038_usbpd_write_reg(i2c, SM5038_REG_PD_CNTL2, data);
}

static void sm5038_set_src(struct i2c_client *i2c)
{
	u8 data;

	sm5038_usbpd_read_reg(i2c, SM5038_REG_PD_CNTL2, &data);
	data |= 0x02;
	sm5038_usbpd_write_reg(i2c, SM5038_REG_PD_CNTL2, data);
}

static void sm5038_set_snk(struct i2c_client *i2c)
{
	u8 data;

	sm5038_usbpd_read_reg(i2c, SM5038_REG_PD_CNTL2, &data);
	data &= ~0x02;
	sm5038_usbpd_write_reg(i2c, SM5038_REG_PD_CNTL2, data);
}

static int sm5038_set_attach(struct sm5038_phydrv_data *pdic_data, u8 mode)
{
	int ret = 0;
	struct i2c_client *i2c = pdic_data->i2c;
	struct device *dev = &i2c->dev;

	if (mode == TYPEC_PORT_SRC) {
		sm5038_usbpd_write_reg(i2c, SM5038_REG_CC_CNTL1, 0x49);
		sm5038_usbpd_write_reg(i2c, SM5038_REG_CC_CNTL3, 0x81);
	} else if (mode == TYPEC_PORT_SNK) {
		sm5038_usbpd_write_reg(i2c, SM5038_REG_CC_CNTL1, 0x45);
		sm5038_usbpd_write_reg(i2c, SM5038_REG_CC_CNTL3, 0x82);
	}

	dev_info(dev, "%s sm5038 force to attach\n", __func__);

	return ret;
}

static int sm5038_set_detach(struct sm5038_phydrv_data *pdic_data, u8 mode)
{
	u8 data;
	int ret = 0;
	struct i2c_client *i2c = pdic_data->i2c;
	struct device *dev = &i2c->dev;

	sm5038_usbpd_read_reg(i2c, SM5038_REG_CC_CNTL3, &data);
	data |= 0x08;
	sm5038_usbpd_write_reg(i2c, SM5038_REG_CC_CNTL3, data);

	dev_info(dev, "%s sm5038 force to detach\n", __func__);

	return ret;
}

static int sm5038_set_vconn_source(void *_data, int val)
{
	struct sm5038_usbpd_data *data = (struct sm5038_usbpd_data *) _data;
	struct sm5038_phydrv_data *pdic_data = data->phy_driver_data;
	struct i2c_client *i2c = pdic_data->i2c;
	struct device *dev = &i2c->dev;
	u8 reg_data = 0, reg_val = 0, attach_type = 0;
	int cable_type = 0;

	if (!pdic_data->vconn_en) {
		pr_err("%s, not support vconn source\n", __func__);
		return -EPERM;
	}

	sm5038_usbpd_read_reg(i2c, SM5038_REG_CC_STATUS, &reg_val);

	attach_type = (reg_val & SM5038_ATTACH_TYPE);
	cable_type = (reg_val >> SM5038_CABLE_TYPE_SHIFT) ?
			PWR_CABLE : NON_PWR_CABLE;

	dev_info(dev, "%s ON=%d REG=0x%x, ATTACH_TYPE = %d, CABLE_TYPE = %d\n",
			__func__, val, reg_val, attach_type, cable_type);

	if (val == VCONN_TURN_ON) {
		if (cable_type || pdic_data->pd_support) {
			reg_data = (reg_val & 0x20) ? 0x1A : 0x19;
			sm5038_usbpd_write_reg(i2c, SM5038_REG_CC_CNTL5, reg_data);
		}
	} else if (val == VCONN_TURN_OFF) {
		reg_data = 0x18;
		sm5038_usbpd_write_reg(i2c, SM5038_REG_CC_CNTL5, reg_data);
	} else
		return -EINVAL;

	pdic_data->vconn_source = val;
	return 0;
}

static void sm5038_adc_value_read(void *_data, u8 *adc_value)
{
	struct sm5038_phydrv_data *pdic_data = _data;
	struct i2c_client *i2c = pdic_data->i2c;
	u8 adc_done = 0, reg_data = 0;
	int retry = 0;

	sm5038_usbpd_read_reg(i2c, SM5038_REG_ADC_CNTL1, &adc_done);
	for (retry = 0; retry < 5; retry++) {
		if (!(adc_done & SM5038_ADC_DONE)) {
			pr_info("%s, ADC_DONE is not yet, retry : %d\n", __func__, retry+1);
			sm5038_usbpd_read_reg(i2c, SM5038_REG_ADC_CNTL1, &adc_done);
		} else {
			sm5038_usbpd_read_reg(i2c, SM5038_REG_ADC_CNTL2, &reg_data);
			break;
		}
	}
	*adc_value = reg_data;
}

static int sm5038_vbus_adc_read(void *_data)
{
	struct sm5038_phydrv_data *pdic_data = _data;
	struct i2c_client *i2c = NULL;
	u8 vbus_adc = 0, status1 = 0;
	int vbus_voltage = 0;

	if (!pdic_data)
		return -ENXIO;

	i2c = pdic_data->i2c;
	if (!i2c)
		return -ENXIO;

	sm5038_usbpd_read_reg(i2c, SM5038_REG_STATUS1, &status1);
	sm5038_usbpd_write_reg(i2c, SM5038_REG_ADC_CNTL1, SM5038_ADC_PATH_SEL_VBUS);
	sm5038_adc_value_read(pdic_data, &vbus_adc);
	vbus_voltage = vbus_adc * 100000;

	pr_info("%s, STATUS1 = 0x%x, ADC : 0x%x, VBUS : %d\n",
			__func__, status1, vbus_adc, vbus_voltage);

	return vbus_voltage;
}

#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
void sm5038_serial_gpio_pull_down(struct sm5038_phydrv_data *chg)
{
	int ret = 0;
	if (IS_ERR_OR_NULL(chg->serial_gpio_suspend)) {
		pr_err("serial_gpio_suspend is NULL");
		return ;
	}
	pr_info("%s-%d", __func__, __LINE__);

	ret = pinctrl_select_state(chg->serial_pinctrl, chg->serial_gpio_suspend);
	if (ret < 0) {
		pr_err("Set Suspend pin state error:%d", ret);
	}
}
#endif

#if defined(CONFIG_SM5038_WATER_DETECTION_ENABLE)
static void sm5038_process_cc_water_det(void *data, int state)
{
	struct sm5038_phydrv_data *pdic_data = data;
	union power_supply_propval pval = {0, };

	if (state) {
		pval.intval = 1;
		pdic_data->is_water_detect = true;
	} else {
		pval.intval = 2;
		pdic_data->is_water_detect = false;
	}
	pr_info("%s, [pull down]water state : %d\n", __func__, pdic_data->is_water_detect);
	/*PDX225 code for QN5965F-837  at 2021/11/17 start*/
	#if defined(CONFIG_SERIAL_MSM_GENI) && defined(CONFIG_SERIAL_MSM_GENI_CONSOLE)
	msm_geni_serial_gpio_suspend(pdic_data->is_water_detect);
	#endif
	#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	if(pdic_data->is_water_detect) {
		sm5038_serial_gpio_pull_down(pdic_data);
	}
	#endif
	/*PDX225 code for QN5965F-837  at 2021/11/17 end*/

	if (pdic_data->psy_usbpd)
		power_supply_changed(pdic_data->psy_usbpd);
	sm5038_charger_psy_changed(POWER_SUPPLY_PROP_INPUT_CURRENT_LIMIT, pval);

	pr_info("%s, water state : %d\n", __func__, state);
}
#endif

#if defined(CONFIG_SM5038_SUPPORT_SBU) && defined(CONFIG_SM5038_SHORT_PROTECTION)
static void sm5038_corr_sbu_volt_read(void *_data, u8 *adc_sbu1,
				u8 *adc_sbu2, int mode)
{
	struct sm5038_phydrv_data *pdic_data = _data;
	struct i2c_client *i2c = pdic_data->i2c;
	u8 adc_value1 = 0, adc_value2 = 0;

	if (mode) {
		sm5038_usbpd_write_reg(i2c, SM5038_REG_CORR_CNTL5, 0x98);
		sm5038_usbpd_write_reg(i2c, SM5038_REG_CORR_CNTL6, 0x40);
		usleep_range(5000, 5100);
	}
	sm5038_usbpd_write_reg(i2c, SM5038_REG_ADC_CNTL1, SM5038_ADC_PATH_SEL_SBU1);
	sm5038_adc_value_read(pdic_data, &adc_value1);
	*adc_sbu1 = adc_value1;

	sm5038_usbpd_write_reg(i2c, SM5038_REG_ADC_CNTL1, SM5038_ADC_PATH_SEL_SBU2);
	sm5038_adc_value_read(pdic_data, &adc_value2);
	*adc_sbu2 = adc_value2;

	if (mode) {
		sm5038_usbpd_write_reg(i2c, SM5038_REG_CORR_CNTL5, 0x00);
		sm5038_usbpd_write_reg(i2c, SM5038_REG_CORR_CNTL6, 0x00);
	}

	pr_info("%s, mode : %d, SBU1_VOLT : 0x%x, SBU2_VOLT : 0x%x\n",
			__func__, mode, adc_value1, adc_value2);
}

#if defined(CONFIG_SM5038_SHORT_PROTECTION)
void sm5038_sbu_short_state_check(void *_data)
{
	struct sm5038_phydrv_data *pdic_data = _data;
	u8 adc_sbu1, adc_sbu2, adc_sbu3, adc_sbu4;

	if (pdic_data->is_cc_abnormal_state || pdic_data->is_otg_vboost)
		return;

	sm5038_corr_sbu_volt_read(pdic_data, &adc_sbu1, &adc_sbu2,
			SBU_SOURCING_OFF);
	if (adc_sbu1 > 0x84 || adc_sbu2 > 0x84) {
		sm5038_corr_sbu_volt_read(pdic_data, &adc_sbu3,
				&adc_sbu4, SBU_SOURCING_ON);
		if ((adc_sbu1 < 0x3 || adc_sbu2 < 0x3) &&
				(adc_sbu3 > 0x84 || adc_sbu4 > 0x84)) {
			pdic_data->is_sbu_abnormal_state = true;
			pr_info("%s, SBU-VBUS SHORT\n", __func__);
		}
		return;
	}

	sm5038_corr_sbu_volt_read(pdic_data, &adc_sbu1, &adc_sbu2,
			SBU_SOURCING_ON);
	if ((adc_sbu1 < 0x03 || adc_sbu2 < 0x03) &&
			(adc_sbu1 > 0x84 || adc_sbu2 > 0x84)) {
		pdic_data->is_sbu_abnormal_state = true;
		pr_info("%s, SBU-GND SHORT\n", __func__);
	}
}
#endif
#endif

#if defined(CONFIG_SM5038_SHORT_PROTECTION)
static void sm5038_get_short_state(void *_data, bool *val)
{
	struct sm5038_usbpd_data *data = (struct sm5038_usbpd_data *) _data;
	struct sm5038_phydrv_data *pdic_data = data->phy_driver_data;

#if defined(CONFIG_SM5038_SUPPORT_SBU)
	*val = (pdic_data->is_cc_abnormal_state ||
		pdic_data->is_sbu_abnormal_state);
#else
	*val = pdic_data->is_cc_abnormal_state;
#endif
}
#endif

static void sm5038_check_cc_state(struct sm5038_phydrv_data *pdic_data)
{
	struct i2c_client *i2c = pdic_data->i2c;
	u8 cc_st = 0, status1 = 0;

	if (!pdic_data->typec_try_state_change && !pdic_data->is_usb_therm) {
		sm5038_usbpd_read_reg(i2c, SM5038_REG_STATUS1, &status1);
		sm5038_usbpd_read_reg(i2c, SM5038_REG_PD_STATE0, &cc_st);
		if (!(status1 & SM5038_REG_INT_STATUS1_ATTACH)) {
			if ((cc_st & 0x0F) == 0x1) /* Set CC_DISABLE to CC_UNATT_SNK */
				sm5038_usbpd_write_reg(i2c, SM5038_REG_CC_CNTL3, 0x82);
			else /* Set CC_OP_EN */
				sm5038_usbpd_write_reg(i2c, SM5038_REG_CC_CNTL3, SM5038_CC_OP_EN);
		}
	}

	pr_info("%s, STATUS1 : 0x%x, CC_ST : 0x%x\n", __func__, status1, cc_st);
}

static inline const char *sm5038_src_current(enum typec_pwr_opmode typec_mode)
{
	switch (typec_mode) {
	case TYPEC_PWR_MODE_USB:
		return "default";
	case TYPEC_PWR_MODE_1_5A:
		return "medium - 1.5A";
	case TYPEC_PWR_MODE_3_0A:
		return "high - 3.0A";
	default:
		return "";
	}
}

static void sm5038_notify_rp_current_level(void *_data)
{
	struct sm5038_usbpd_data *pd_data = (struct sm5038_usbpd_data *) _data;
	struct sm5038_phydrv_data *pdic_data = pd_data->phy_driver_data;
	struct i2c_client *i2c = pdic_data->i2c;
	u8 cc_status = 0, rp_currentlvl = 0, prev_lvl = 0;
#if defined(CONFIG_SM5038_SHORT_PROTECTION)
	bool short_cable = false;
#endif
	enum typec_pwr_opmode mode = TYPEC_PWR_MODE_USB;
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	union power_supply_propval pval = {0, };
#endif

#if defined(CONFIG_SM5038_SHORT_PROTECTION)
	sm5038_get_short_state(pd_data, &short_cable);
	if (short_cable)
		return;
#endif
#if defined(CONFIG_SM5038_WATER_DETECTION_ENABLE)
	if (pdic_data->is_water_detect)
		return;
#endif

	sm5038_usbpd_read_reg(i2c, SM5038_REG_CC_STATUS, &cc_status);

	pr_info("%s : CC_STATUS = 0x%x\n", __func__, cc_status);

	/* PDIC = SINK */
	if ((cc_status & SM5038_ATTACH_TYPE) == SM5038_ATTACH_SOURCE) {
		if ((cc_status & SM5038_ADV_CURR) == 0x00) {
			/* 5V/0.5A RP charger is detected by PDIC */
			rp_currentlvl = RP_CURRENT_LEVEL_DEFAULT;
		} else if ((cc_status & SM5038_ADV_CURR) == 0x08) {
			/* 5V/1.5A RP charger is detected by PDIC */
			rp_currentlvl = RP_CURRENT_LEVEL2;
			mode = TYPEC_PWR_MODE_1_5A;
		} else {
			/* 5V/3A RP charger is detected by PDIC */
			rp_currentlvl = RP_CURRENT_LEVEL3;
			mode = TYPEC_PWR_MODE_3_0A;
		}
		prev_lvl = pdic_data->rp_currentlvl;
		pdic_data->rp_currentlvl = rp_currentlvl;

		if (rp_currentlvl != prev_lvl && !pdic_data->pd_support
				&& rp_currentlvl >= RP_CURRENT_LEVEL_DEFAULT) {
			pdic_data->pwr_opmode = mode;
			typec_set_pwr_opmode(pdic_data->port, mode);
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
			switch (pdic_data->rp_currentlvl) {
			case RP_CURRENT_LEVEL_DEFAULT:
				pval.intval =
					SOMC_POWER_SUPPLY_EXT_TYPE_RP_DEFAULT;
				break;
			case RP_CURRENT_LEVEL2:
				pval.intval =
					SOMC_POWER_SUPPLY_EXT_TYPE_RP_1_5A;
				break;
			case RP_CURRENT_LEVEL3:
				pval.intval =
					SOMC_POWER_SUPPLY_EXT_TYPE_RP_3_0A;
				break;
			}
			sm5038_charger_psy_changed(POWER_SUPPLY_PROP_ONLINE,
									pval);
#endif
			pr_info("%s : Type-C Source (%s) connected\n",
					__func__, sm5038_src_current(mode),
					pdic_data->rp_currentlvl);
		}
	}
}

#if defined(CONFIG_SM5038_SHORT_PROTECTION)
static void sm5038_notify_rp_abnormal(void *_data)
{
	struct sm5038_usbpd_data *pd_data = (struct sm5038_usbpd_data *) _data;
	struct sm5038_phydrv_data *pdic_data = pd_data->phy_driver_data;
	struct i2c_client *i2c = pdic_data->i2c;
	u8 cc_status = 0, rp_currentlvl = RP_CURRENT_ABNORMAL;

	sm5038_usbpd_read_reg(i2c, SM5038_REG_CC_STATUS, &cc_status);

	/* PDIC = SINK */
	if ((cc_status & SM5038_ATTACH_TYPE) == SM5038_ATTACH_SOURCE) {
		if (rp_currentlvl != pdic_data->rp_currentlvl) {
			pr_info("%s : rp_currentlvl(%d)\n", __func__,
					pdic_data->rp_currentlvl);
		}
	}
}
#endif

static void sm5038_send_role_swap_message(
	struct sm5038_phydrv_data *usbpd_data, u8 mode)
{
	struct sm5038_usbpd_data *pd_data;

	pd_data = dev_get_drvdata(usbpd_data->dev);
	if (!pd_data) {
		pr_err("%s : pd_data is null\n", __func__);
		return;
	}

	pr_info("%s : send %s\n", __func__,
		mode == POWER_ROLE_SWAP ? "pr_swap" : "dr_swap");
	if (mode == POWER_ROLE_SWAP)
		sm5038_usbpd_inform_event(pd_data, MANAGER_PR_SWAP_REQUEST);
	else
		sm5038_usbpd_inform_event(pd_data, MANAGER_DR_SWAP_REQUEST);
}

void sm5038_rprd_mode_change(struct sm5038_phydrv_data *usbpd_data,
		enum typec_port_type mode)
{
	struct i2c_client *i2c = usbpd_data->i2c;

	pr_info("%s : mode=0x%x\n", __func__, mode);

	switch (mode) {
	case TYPEC_PORT_SRC: /* SRC */
		sm5038_set_detach(usbpd_data, mode);
		msleep(1000);
		sm5038_set_attach(usbpd_data, mode);
		break;
	case TYPEC_PORT_SNK: /* SNK */
		sm5038_set_detach(usbpd_data, mode);
		msleep(1000);
		sm5038_set_attach(usbpd_data, mode);
		break;
	case TYPEC_PORT_DRP: /* DRP */
		sm5038_usbpd_write_reg(i2c,
			SM5038_REG_CC_CNTL1, 0x41);
		break;
	};
}

static void sm5038_role_swap_check(struct work_struct *wk)
{
	struct delayed_work *delay_work =
		container_of(wk, struct delayed_work, work);
	struct sm5038_phydrv_data *usbpd_data =
		container_of(delay_work,
			struct sm5038_phydrv_data, role_swap_work);

	pr_info("%s: role swap check again.\n", __func__);

	usbpd_data->typec_try_state_change = ROLE_SWAP_NONE;
	if (!usbpd_data->is_attached) {
		pr_err("%s: reverse failed, set mode to DRP\n", __func__);
		sm5038_rprd_mode_change(usbpd_data, TYPEC_PORT_DRP);
	}
}

void sm5038_vbus_dischg_work(struct work_struct *work)
{
	struct sm5038_phydrv_data *pdic_data =
		container_of(work, struct sm5038_phydrv_data,
				vbus_dischg_work.work);

	if (gpio_is_valid(pdic_data->vbus_dischg_gpio)) {
		gpio_set_value(pdic_data->vbus_dischg_gpio, 0);
		pr_info("%s vbus_discharging(%d)\n", __func__,
			gpio_get_value(pdic_data->vbus_dischg_gpio));
	}
}

void sm5038_usbpd_set_vbus_dischg_gpio(struct sm5038_phydrv_data
		*pdic_data, int vbus_dischg)
{
	if (!gpio_is_valid(pdic_data->vbus_dischg_gpio))
		return;

	cancel_delayed_work_sync(&pdic_data->vbus_dischg_work);
	gpio_set_value(pdic_data->vbus_dischg_gpio, vbus_dischg);

	if (vbus_dischg > 0)
		schedule_delayed_work(&pdic_data->vbus_dischg_work,
			msecs_to_jiffies(600));

	pr_info("%s vbus_discharging(%d)\n", __func__,
		gpio_get_value(pdic_data->vbus_dischg_gpio));
}
EXPORT_SYMBOL_GPL(sm5038_usbpd_set_vbus_dischg_gpio);

int sm5038_cc_control_command(int enable)
{
	struct sm5038_phydrv_data *pdic_data = static_pdic_data;
	struct i2c_client *i2c = pdic_data->i2c;
	int ret = 0;
	int prev_usb_therm;

	if (pdic_data == NULL) {
		pr_err("%s NULL data\n", __func__);
		return -ENXIO;
	}

	prev_usb_therm = pdic_data->is_usb_therm;

	if (prev_usb_therm == enable)
		return -EPERM;

	if (enable) { /* open */
		ret = sm5038_usbpd_write_reg(i2c, SM5038_REG_CC_CNTL3, 0x88);
		pdic_data->is_usb_therm = 1;
	} else { /* restore */
		ret = sm5038_usbpd_write_reg(i2c, SM5038_REG_CC_CNTL3, 0x82);
		pdic_data->is_usb_therm = 0;
	}
	pr_info(" %s : CC Command = (%s), ret = %d\n", __func__,
		enable ? "Open" : "Restore", ret);

	return ret;
}
EXPORT_SYMBOL_GPL(sm5038_cc_control_command);

static int sm5038_get_plug_orientation(struct sm5038_phydrv_data
		*_data)
{
	struct i2c_client *i2c = _data->i2c;
	int cc_orientation = 0;
	u8 cc_status = 0;

	sm5038_usbpd_read_reg(i2c, SM5038_REG_CC_STATUS, &cc_status);
	if (cc_status & 0x7) { //2:0bit ATTACH_TYPE
		if (cc_status & 0x20) //5bit CABLE_FLIP
			cc_orientation = ORIENTATION_CC2; //CC2_ACTVIE
		else
			cc_orientation = ORIENTATION_CC1; //CC1_ACTIVE
		pr_info(" %s : CC Connect = (%s), ret = 0x%x\n", __func__,
			cc_orientation ? "CC1_ACTIVE" : "CC2_ACTIVE", cc_status);

	} else {
		cc_orientation = ORIENTATION_NONE; //NO_DETERMINATION
		pr_info(" %s : CC Connect = (%s), ret = 0x%x\n", __func__,
			"NONE", cc_status);		
	}

	return cc_orientation;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0)
static int sm5038_dr_set(const struct typec_capability *cap,
		enum typec_data_role role)
#else
static int sm5038_dr_set(struct typec_port *port,
		enum typec_data_role role)
#endif
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0)
	struct sm5038_phydrv_data *usbpd_data =
		container_of(cap, struct sm5038_phydrv_data, typec_cap);
#else
	struct sm5038_phydrv_data *usbpd_data =
		(struct sm5038_phydrv_data *)typec_get_drvdata(port);
#endif
	pr_info("%s : typec_power_role=%d, typec_data_role=%d, role=%d\n",
		__func__, usbpd_data->typec_power_role,
		usbpd_data->typec_data_role, role);

	if (usbpd_data->typec_data_role != TYPEC_DEVICE
		&& usbpd_data->typec_data_role != TYPEC_HOST)
		return -EPERM;
	else if (usbpd_data->typec_data_role == role)
		return -EPERM;

	reinit_completion(&usbpd_data->typec_reverse_completion);
	if (role == TYPEC_DEVICE) {
		pr_info("%s : try reversing, from DFP to UFP\n", __func__);
		usbpd_data->typec_try_state_change = ROLE_SWAP_DR;
		sm5038_send_role_swap_message(usbpd_data, DATA_ROLE_SWAP);
	} else if (role == TYPEC_HOST) {
		pr_info("%s : try reversing, from UFP to DFP\n", __func__);
		usbpd_data->typec_try_state_change = ROLE_SWAP_DR;
		sm5038_send_role_swap_message(usbpd_data, DATA_ROLE_SWAP);
	} else {
		pr_info("%s : invalid typec_role\n", __func__);
		return -EIO;
	}
	if (!wait_for_completion_timeout(&usbpd_data->typec_reverse_completion,
				msecs_to_jiffies(TRY_ROLE_REVERSAL_WAIT_MS))) {
		usbpd_data->typec_try_state_change = ROLE_SWAP_NONE;
		return -ETIMEDOUT;
	}

	return 0;
}
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0)
static int sm5038_pr_set(const struct typec_capability *cap,
		enum typec_role role)
#else
static int sm5038_pr_set(struct typec_port *port,
		enum typec_role role)
#endif
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0)
	struct sm5038_phydrv_data *usbpd_data =
		container_of(cap, struct sm5038_phydrv_data, typec_cap);
#else
	struct sm5038_phydrv_data *usbpd_data =
		(struct sm5038_phydrv_data *)typec_get_drvdata(port);
#endif
	pr_info("%s : typec_power_role=%d, typec_data_role=%d, role=%d\n",
		__func__, usbpd_data->typec_power_role,
		usbpd_data->typec_data_role, role);

	if (usbpd_data->typec_power_role != TYPEC_SINK
	    && usbpd_data->typec_power_role != TYPEC_SOURCE)
		return -EPERM;
	else if (usbpd_data->typec_power_role == role)
		return -EPERM;

	reinit_completion(&usbpd_data->typec_reverse_completion);
	if (role == TYPEC_SINK) {
		pr_info("%s : try reversing, from Source to Sink\n", __func__);
		usbpd_data->typec_try_state_change = ROLE_SWAP_PR;
		sm5038_send_role_swap_message(usbpd_data, POWER_ROLE_SWAP);
	} else if (role == TYPEC_SOURCE) {
		pr_info("%s : try reversing, from Sink to Source\n", __func__);
		usbpd_data->typec_try_state_change = ROLE_SWAP_PR;
		sm5038_send_role_swap_message(usbpd_data, POWER_ROLE_SWAP);
	} else {
		pr_info("%s : invalid typec_role\n", __func__);
		return -EIO;
	}
	if (!wait_for_completion_timeout(&usbpd_data->typec_reverse_completion,
				msecs_to_jiffies(TRY_ROLE_REVERSAL_WAIT_MS))) {
		usbpd_data->typec_try_state_change = ROLE_SWAP_NONE;
		if (usbpd_data->typec_power_role != role)
			return -ETIMEDOUT;
	}

	return 0;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0)
static int sm5038_port_type_set(const struct typec_capability *cap,
		enum typec_port_type port_type)
#else
static int sm5038_port_type_set(struct typec_port *port,
		enum typec_port_type port_type)
#endif
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0)
	struct sm5038_phydrv_data *usbpd_data =
		container_of(cap, struct sm5038_phydrv_data, typec_cap);
#else
	struct sm5038_phydrv_data *usbpd_data =
		(struct sm5038_phydrv_data *)typec_get_drvdata(port);
#endif
	pr_info("%s : typec_power_role=%d, typec_data_role=%d, port_type=%d\n",
		__func__, usbpd_data->typec_power_role,
		usbpd_data->typec_data_role, port_type);

	reinit_completion(&usbpd_data->typec_reverse_completion);
	if (port_type == TYPEC_PORT_DFP) {
		pr_info("%s : try reversing, from UFP(Sink) to DFP(Source)\n",
			__func__);
		usbpd_data->typec_try_state_change = ROLE_SWAP_TYPE;
		sm5038_rprd_mode_change(usbpd_data, TYPEC_PORT_SRC);
	} else if (port_type == TYPEC_PORT_UFP) {
		pr_info("%s : try reversing, from DFP(Source) to UFP(Sink)\n",
			__func__);
		usbpd_data->typec_try_state_change = ROLE_SWAP_TYPE;
		sm5038_rprd_mode_change(usbpd_data, TYPEC_PORT_SNK);
	} else {
		pr_info("%s : invalid typec_role\n", __func__);
		return 0;
	}

	if (!wait_for_completion_timeout(&usbpd_data->typec_reverse_completion,
				msecs_to_jiffies(TRY_ROLE_REVERSAL_WAIT_MS))) {
		usbpd_data->typec_try_state_change = ROLE_SWAP_NONE;
		pr_err("%s: reverse failed, set mode to DRP\n", __func__);
		/* exit from Disabled state and set mode to DRP */
		sm5038_rprd_mode_change(usbpd_data, TYPEC_PORT_DRP);
		return -ETIMEDOUT;
	} else {
		pr_err("%s: reverse success, one more check\n", __func__);
		schedule_delayed_work(&usbpd_data->role_swap_work,
				msecs_to_jiffies(TRY_ROLE_REVERSAL_WAIT_MS));
	}

	return 0;
}

int sm5038_get_pd_support(struct sm5038_phydrv_data *usbpd_data)
{
	if (usbpd_data->pd_support)
		return TYPEC_PWR_MODE_PD;

	return usbpd_data->pwr_opmode;
}


void sm5038_typec_partner_event(void *data, int event)
{
	struct sm5038_phydrv_data *usbpd_data = data;
	struct typec_partner_desc desc;
	enum typec_pwr_opmode mode = TYPEC_PWR_MODE_USB;

	pr_info("%s : typec_power_role=%d typec_data_role=%d, event=%d\n",
			__func__, usbpd_data->typec_power_role,
			usbpd_data->typec_data_role, event);

		if (usbpd_data->partner == NULL) {
			if (event == TYPEC_STATUS_NOTIFY_ATTACH_UFP) {
				mode = sm5038_get_pd_support(usbpd_data);
				typec_set_pwr_opmode(usbpd_data->port, mode);
				desc.usb_pd = mode == TYPEC_PWR_MODE_PD;
				desc.accessory = TYPEC_ACCESSORY_NONE;
				desc.identity = NULL;
				usbpd_data->typec_data_role = TYPEC_DEVICE;
				typec_set_pwr_role(usbpd_data->port,
					usbpd_data->typec_power_role);
				typec_set_data_role(usbpd_data->port,
					usbpd_data->typec_data_role);
				usbpd_data->partner = typec_register_partner(usbpd_data->port,
					&desc);
			} else if (event == TYPEC_STATUS_NOTIFY_ATTACH_DFP) {
				mode = sm5038_get_pd_support(usbpd_data);
				typec_set_pwr_opmode(usbpd_data->port, mode);
				desc.usb_pd = mode == TYPEC_PWR_MODE_PD;
				desc.accessory = TYPEC_ACCESSORY_NONE;
				desc.identity = NULL;
				usbpd_data->typec_data_role = TYPEC_HOST;
				typec_set_pwr_role(usbpd_data->port,
					usbpd_data->typec_power_role);
				typec_set_data_role(usbpd_data->port,
					usbpd_data->typec_data_role);
				usbpd_data->partner = typec_register_partner(usbpd_data->port,
					&desc);
			} else
				pr_info("%s : detach case\n", __func__);
			if (usbpd_data->typec_try_state_change &&
					(event != TYPEC_STATUS_NOTIFY_DETACH)) {
				pr_info("usb: %s, typec_reverse_completion\n", __func__);
				complete(&usbpd_data->typec_reverse_completion);
			}
		} else {
			if (event == TYPEC_STATUS_NOTIFY_ATTACH_UFP) {
				usbpd_data->typec_data_role = TYPEC_DEVICE;
				typec_set_data_role(usbpd_data->port, usbpd_data->typec_data_role);
			} else if (event == TYPEC_STATUS_NOTIFY_ATTACH_DFP) {
				usbpd_data->typec_data_role = TYPEC_HOST;
				typec_set_data_role(usbpd_data->port, usbpd_data->typec_data_role);
			} else
				pr_info("%s : detach case\n", __func__);
	}
}

static void sm5038_process_dr_swap(struct sm5038_phydrv_data *usbpd_data, int val)
{
	if (val == TYPEC_DEVICE) {
		stop_usb_host(usbpd_data);
		start_usb_peripheral(usbpd_data);
		sm5038_typec_partner_event(usbpd_data,
				TYPEC_STATUS_NOTIFY_DETACH/*drp*/);

		sm5038_typec_partner_event(usbpd_data,
				TYPEC_STATUS_NOTIFY_ATTACH_UFP/*ufp*/);
	} else if (val == TYPEC_HOST) {
		stop_usb_peripheral(usbpd_data);
		start_usb_host(usbpd_data, true);
		sm5038_typec_partner_event(usbpd_data,
				TYPEC_STATUS_NOTIFY_DETACH/*drp*/);

		sm5038_typec_partner_event(usbpd_data,
				TYPEC_STATUS_NOTIFY_ATTACH_DFP/*dfp*/);

		/* ensure host is started before allowing DP */
/*		extcon_blocking_sync(usbpd_data->extcon, EXTCON_USB_HOST,
				START_USB_HOST);*/
	} else
		pr_err("%s : invalid val\n", __func__);
}

int sm5038_get_prop_usb_present(struct sm5038_phydrv_data *pdic_data,
				union power_supply_propval *val)
{
	struct i2c_client *i2c = pdic_data->i2c;
	int rc;
	u8 stat;

	rc = sm5038_usbpd_read_reg(i2c, SM5038_REG_STATUS1, &stat);
	if (rc < 0) {
		pr_err("Couldn't read STATUS1 rc=%d\n", rc);
		return rc;
	}

	val->intval = (bool)(stat & SM5038_REG_INT_STATUS1_VBUSPOK);
	return 0;
}

int sm5038_get_prop_usb_online(struct sm5038_phydrv_data *pdic_data,
		union power_supply_propval *val)
{
	struct i2c_client *i2c = pdic_data->i2c;
	int rc;
	int cable_type = 0;
	int charge_mode = 0;
	int chg_on_status = 0;
	u8 stat;

	rc = sm5038_usbpd_read_reg(i2c, SM5038_REG_STATUS1, &stat);
	if (rc < 0) {
		pr_err("Couldn't read STATUS1 rc=%d\n", rc);
		return rc;
	}
	cable_type = pdic_data->chg_cable_type;
	charge_mode = pdic_data->chg_charge_mode;
	chg_on_status = pdic_data->chg_on_status;

	if (stat & SM5038_REG_INT_STATUS1_ATTACH) {
		if ((cable_type == POWER_SUPPLY_TYPE_USB_CDP) ||
				(cable_type == POWER_SUPPLY_TYPE_USB_DCP) ||
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
				(cable_type == POWER_SUPPLY_TYPE_USB_FLOAT) ||
#endif
				(cable_type == POWER_SUPPLY_TYPE_USB_PD)) {
			if (charge_mode < 0) {
				return charge_mode;
			}
			if (charge_mode == 2 || chg_on_status == 0)
				val->intval = 0;
			else
				val->intval = 1;
		} else
			val->intval = 0;
	} else
		val->intval = 0;
	return 0;
}

static int sm5038_usbpd_get_property(struct power_supply *psy,
		enum power_supply_property psp,
		union power_supply_propval *val)
{
	struct sm5038_phydrv_data *pdic_data =
			power_supply_get_drvdata(psy);
	int rc = 0;

	val->intval = 0;

	switch (psp) {
	case POWER_SUPPLY_PROP_PRESENT:
		rc = sm5038_get_prop_usb_present(pdic_data, val);
		pr_info("%s PRESENT get is %d \n",
				__func__, val->intval);
		break;
	case POWER_SUPPLY_PROP_ONLINE:
		rc = sm5038_get_prop_usb_online(pdic_data, val);
		pr_info("%s ONLINE get is %d \n",
				__func__, val->intval);
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		val->intval = sm5038_vbus_adc_read(pdic_data);
		break;
	case POWER_SUPPLY_PROP_CURRENT_NOW:
		rc = -EINVAL;
		break;
	case POWER_SUPPLY_PROP_CURRENT_MAX:
		val->intval = (pdic_data->chg_input_current * 1000);
		pr_info("%s get prop CURRENT_MAX(%d)\n",
				__func__, val->intval);
		break;
	case POWER_SUPPLY_PROP_TYPE:
		val->intval = POWER_SUPPLY_TYPE_USB_PD;
		break;
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	case POWER_SUPPLY_PROP_USB_TYPE:
		switch (pdic_data->chg_cable_type) {
		case POWER_SUPPLY_TYPE_USB:
			val->intval = POWER_SUPPLY_USB_TYPE_SDP;
			break;
		case POWER_SUPPLY_TYPE_USB_DCP:
		case POWER_SUPPLY_TYPE_APPLE_BRICK_ID:
		case POWER_SUPPLY_TYPE_USB_HVDCP:
		case POWER_SUPPLY_TYPE_USB_HVDCP_3:
		case POWER_SUPPLY_TYPE_USB_HVDCP_3P5:
		case POWER_SUPPLY_TYPE_USB_FLOAT:
			val->intval = POWER_SUPPLY_USB_TYPE_DCP;
			break;
		case POWER_SUPPLY_TYPE_USB_CDP:
			val->intval = POWER_SUPPLY_USB_TYPE_CDP;
			break;
		case POWER_SUPPLY_TYPE_USB_PD:
			val->intval = POWER_SUPPLY_USB_TYPE_PD;
			break;
		default:
			val->intval = POWER_SUPPLY_USB_TYPE_UNKNOWN;
		}
		break;
#endif
	case POWER_SUPPLY_PROP_VOLTAGE_MAX:
	case POWER_SUPPLY_PROP_VOLTAGE_MAX_DESIGN:
		val->intval = USB_MAX_VOLT;
		break;
	case POWER_SUPPLY_PROP_SCOPE:
		val->intval = POWER_SUPPLY_SCOPE_UNKNOWN;
		break;
	case POWER_SUPPLY_PROP_INPUT_CURRENT_LIMIT:
		/* USB uses this to set SDP current */
		val->intval = 450;
		break;
	case POWER_SUPPLY_PROP_POWER_NOW:
		rc = -EINVAL;
		break;
	default:
		pr_err("get prop %d is not supported\n", psp);
		break;
	}

	if (rc < 0) {
		pr_debug("Couldn't get prop %d rc = %d\n", psp, rc);
		return -ENODATA;
	}
	return 0;
}

static int sm5038_usbpd_set_property(struct power_supply *psy,
		enum power_supply_property psp,
		const union power_supply_propval *val)
{
	union power_supply_propval pval = {0, };
	int rc = 0;

	switch (psp) {
	case POWER_SUPPLY_PROP_INPUT_CURRENT_LIMIT:
		pval.intval = val->intval;
		sm5038_charger_psy_changed(POWER_SUPPLY_PROP_CURRENT_MAX, pval);
		break;
	default:
		pr_err("set prop %d is not supported\n", psp);
		rc = -EINVAL;
	}
	return rc;
}

static int sm5038_usb_prop_is_writeable(struct power_supply *psy,
		enum power_supply_property psp)
{
	switch (psp) {
	case POWER_SUPPLY_PROP_INPUT_CURRENT_LIMIT:
		return 1;
	default:
		break;
	}

	return 0;
}

int sm5038_usbpd_psy_init(struct sm5038_phydrv_data *_data, struct device *parent)
{
	struct power_supply_config psy_cfg = {};
	int ret = 0;

	if (_data == NULL || parent == NULL) {
		pr_err("%s NULL data\n", __func__);
		return -ENXIO;
	}

	_data->usbpd_desc.name           = "usb";
	_data->usbpd_desc.type           = POWER_SUPPLY_TYPE_USB_PD;
	_data->usbpd_desc.get_property   = sm5038_usbpd_get_property;
	_data->usbpd_desc.set_property   = sm5038_usbpd_set_property;
	_data->usbpd_desc.properties     = usbpd_props;
	_data->usbpd_desc.num_properties = ARRAY_SIZE(usbpd_props);
	_data->usbpd_desc.property_is_writeable = sm5038_usb_prop_is_writeable;
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	_data->usbpd_desc.usb_types      = usb_psy_supported_types,
	_data->usbpd_desc.num_usb_types  = ARRAY_SIZE(usb_psy_supported_types),
#endif

	psy_cfg.drv_data = _data;
	psy_cfg.supplied_to = usbpd_supplied_to;
	psy_cfg.num_supplicants = ARRAY_SIZE(usbpd_supplied_to);

	_data->psy_usbpd = devm_power_supply_register(parent, &_data->usbpd_desc, &psy_cfg);
	if (IS_ERR(_data->psy_usbpd)) {
		ret = (int)PTR_ERR(_data->psy_usbpd);
		pr_err("%s: Failed to Register psy_usbpd, ret : %d\n", __func__, ret);
	}
	return ret;
}


void sm5038_usbpd_set_rp_scr_sel(struct sm5038_usbpd_data *_data,
		int scr_sel)
{
	struct sm5038_phydrv_data *pdic_data = _data->phy_driver_data;
	struct i2c_client *i2c = pdic_data->i2c;
	u8 data = 0;

	pr_info("%s: prev_sel(%d), scr_sel : (%d)\n", __func__,
			pdic_data->scr_sel, scr_sel);

	if (pdic_data->detach_valid || (pdic_data->scr_sel == scr_sel))
		return;

	pdic_data->scr_sel = scr_sel;

	switch (scr_sel) {
	case PLUG_CTRL_RP80: /* 80uA */
		sm5038_usbpd_read_reg(i2c, SM5038_REG_CC_CNTL1, &data);
		data &= 0xCF;
		sm5038_usbpd_write_reg(i2c, SM5038_REG_CC_CNTL1, data);
		break;
	case PLUG_CTRL_RP180: /* 180uA */
		sm5038_usbpd_read_reg(i2c, SM5038_REG_CC_CNTL1, &data);
		data |= 0x10;
		sm5038_usbpd_write_reg(i2c, SM5038_REG_CC_CNTL1, data);
		break;
	case PLUG_CTRL_RP330: /* 330uA */
		sm5038_usbpd_read_reg(i2c, SM5038_REG_CC_CNTL1, &data);
		data |= 0x20;
		sm5038_usbpd_write_reg(i2c, SM5038_REG_CC_CNTL1, data);
		break;
	default:
		break;
	}
}
EXPORT_SYMBOL_GPL(sm5038_usbpd_set_rp_scr_sel);

static int sm5038_write_msg_obj(struct i2c_client *i2c,
		int count, data_obj_type *obj)
{
	int ret = 0;
	int i = 0, j = 0;
	u8 reg[USBPD_MAX_COUNT_RX_PAYLOAD] = {0, };
	struct device *dev = &i2c->dev;

	if (count > SM5038_MAX_NUM_MSG_OBJ)
		dev_err(dev, "%s, not invalid obj count number\n", __func__);
	else {
		for (i = 0; i < (count * 4); i++) {
			if ((i != 0) && (i % 4 == 0))
				j++;
			reg[i] = obj[j].byte[i % 4];
		}
			ret = sm5038_usbpd_multi_write(i2c,
			SM5038_REG_TX_PAYLOAD, (count * 4), reg);
		}
	return ret;
}

static int sm5038_send_msg(void *_data, struct i2c_client *i2c)
{
	struct sm5038_usbpd_data *data = (struct sm5038_usbpd_data *) _data;
	struct sm5038_policy_data *policy = &data->policy;
	int ret;
	u8 val;

	if (policy->origin_message == 0x00)
		val = SM5038_REG_MSG_SEND_TX_SOP_REQ;
	else if (policy->origin_message == 0x01)
		val = SM5038_REG_MSG_SEND_TX_SOPP_REQ;
	else
		val = SM5038_REG_MSG_SEND_TX_SOPPP_REQ;

	pr_info("%s, TX_REQ : %x\n", __func__, val);
	ret = sm5038_usbpd_write_reg(i2c, SM5038_REG_TX_REQ, val);

	return ret;
}

static int sm5038_read_msg_obj(struct i2c_client *i2c,
		int count, data_obj_type *obj)
{
	int ret = 0;
	int i = 0, j = 0;
	u8 reg[USBPD_MAX_COUNT_RX_PAYLOAD] = {0, };
	struct device *dev = &i2c->dev;

	if (count > SM5038_MAX_NUM_MSG_OBJ) {
		dev_err(dev, "%s, not invalid obj count number\n", __func__);
		ret = -EINVAL;
	} else {
		ret = sm5038_usbpd_multi_read(i2c,
			SM5038_REG_RX_PAYLOAD, (count * 4), reg);
		for (i = 0; i < (count * 4); i++) {
			if ((i != 0) && (i % 4 == 0))
				j++;
			obj[j].byte[i % 4] = reg[i];
		}
	}

	return ret;
}

static void sm5038_set_irq_enable(struct sm5038_phydrv_data *_data,
		u8 int0, u8 int1, u8 int2, u8 int3, u8 int4)
{
	u8 int_mask[5]
		= {0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
	int ret = 0;
	struct i2c_client *i2c = _data->i2c;
	struct device *dev = &i2c->dev;

	int_mask[0] &= ~int0;
	int_mask[1] &= ~int1;
	int_mask[2] &= ~int2;
	int_mask[3] &= ~int3;
	int_mask[4] &= ~int4;

	ret = sm5038_usbpd_multi_write(i2c, SM5038_REG_INT_MASK1,
			5, int_mask);

	if (ret < 0)
		dev_err(dev, "err write interrupt mask\n");
}

static void sm5038_driver_reset(void *_data)
{
	struct sm5038_usbpd_data *data = (struct sm5038_usbpd_data *) _data;
	struct sm5038_phydrv_data *pdic_data = data->phy_driver_data;
	int i;

	pdic_data->status_reg = 0;
	data->wait_for_msg_arrived = 0;
	pdic_data->header.word = 0;
	for (i = 0; i < SM5038_MAX_NUM_MSG_OBJ; i++)
		pdic_data->obj[i].object = 0;

	sm5038_set_irq_enable(pdic_data, ENABLED_INT_1, ENABLED_INT_2,
			ENABLED_INT_3, ENABLED_INT_4, ENABLED_INT_5);
}

void sm5038_pd_reset_protocol(void *_data)
{
	struct sm5038_usbpd_data *data = (struct sm5038_usbpd_data *) _data;
	struct sm5038_phydrv_data *pdic_data = data->phy_driver_data;
	struct i2c_client *i2c = pdic_data->i2c;

	/* Rx Buffer Flushing */
	sm5038_usbpd_write_reg(i2c, SM5038_REG_RX_BUF_ST, 0x10);

	/* Reset Protocol Layer */
	sm5038_usbpd_write_reg(i2c, SM5038_REG_PD_CNTL4,
			SM5038_REG_CNTL_PROTOCOL_RESET_MESSAGE);

	pr_info("%s\n", __func__);
}
EXPORT_SYMBOL_GPL(sm5038_pd_reset_protocol);

void sm5038_set_pd_function(void *_data, int enable)
{
	struct sm5038_usbpd_data *data = (struct sm5038_usbpd_data *) _data;
	struct sm5038_phydrv_data *pdic_data = data->phy_driver_data;
	struct i2c_client *i2c = pdic_data->i2c;

	if (enable)
		sm5038_usbpd_write_reg(i2c, SM5038_REG_PD_CNTL1, 0x08);
	else
		sm5038_usbpd_write_reg(i2c, SM5038_REG_PD_CNTL1, 0x00);

	pr_info("%s: enable : (%d), power_role : (%s)\n", __func__,
		enable,	pdic_data->power_role ? "SOURCE" : "SINK");
}
EXPORT_SYMBOL_GPL(sm5038_set_pd_function);

void sm5038_cc_state_hold_on_off(void *_data, int onoff)
{
	struct sm5038_usbpd_data *data = (struct sm5038_usbpd_data *) _data;
	struct sm5038_phydrv_data *pdic_data = data->phy_driver_data;
	struct i2c_client *i2c = pdic_data->i2c;
	u8 val;

	sm5038_usbpd_read_reg(i2c, SM5038_REG_CC_CNTL3, &val);
	if (onoff == 1) {
		val &= 0xCF;
		val |= 0x10;
	} else if (onoff == 2) {
		val &= 0xCF;
		val |= 0x20;
	} else
		val &= 0xCF;
	sm5038_usbpd_write_reg(i2c, SM5038_REG_CC_CNTL3, val);
	pr_info("%s: CC State Hold [%d], val = %x\n", __func__, onoff, val);
}
EXPORT_SYMBOL_GPL(sm5038_cc_state_hold_on_off);

bool sm5038_get_rx_buf_st(void *_data)
{
	struct sm5038_usbpd_data *pd_data = (struct sm5038_usbpd_data *) _data;
	struct sm5038_phydrv_data *pdic_data = pd_data->phy_driver_data;
	struct i2c_client *i2c = pdic_data->i2c;
	u8 val;

	sm5038_usbpd_read_reg(i2c, SM5038_REG_RX_BUF_ST, &val);
	pr_info("%s: RX_BUF_ST [0x%02X]\n", __func__, val);

	if (val & 0x04) /* Rx Buffer Empty */
		return false;
	else
		return true;
}
EXPORT_SYMBOL_GPL(sm5038_get_rx_buf_st);

void sm5038_src_transition_to_default(void *_data)
{
	struct sm5038_usbpd_data *data = (struct sm5038_usbpd_data *) _data;
	struct sm5038_phydrv_data *pdic_data = data->phy_driver_data;
	struct sm5038_usbpd_manager_data *manager = &data->manager;
	struct i2c_client *i2c = pdic_data->i2c;
	u8 val;

	sm5038_usbpd_read_reg(i2c, SM5038_REG_PD_CNTL2, &val);
	val &= 0xEF;
	sm5038_usbpd_write_reg(i2c, SM5038_REG_PD_CNTL2, val); /* BIST Off */

	sm5038_set_vconn_source(data, VCONN_TURN_OFF);
	sm5038_vbus_turn_on_ctrl(pdic_data, 0);
	if (manager->dp_is_connect == 1) {
		sm5038_usbpd_dp_detach(pdic_data->dev);
		manager->alt_sended = 0;
		manager->vdm_en = 0;
	}
	sm5038_set_dfp(i2c);
	sm5038_set_pd_function(data, PD_ENABLE);
	pdic_data->data_role = TYPEC_HOST;
	pdic_data->pd_support = 0;

	sm5038_usbpd_uevent_notifier(&data->dev);

	if (!sm5038_check_vbus_state(data))
		sm5038_usbpd_kick_policy_work(pdic_data->dev);

	dev_info(pdic_data->dev, "%s\n", __func__);
}
EXPORT_SYMBOL_GPL(sm5038_src_transition_to_default);

void sm5038_src_transition_to_pwr_on(void *_data)
{
	struct sm5038_usbpd_data *data = (struct sm5038_usbpd_data *) _data;
	struct sm5038_phydrv_data *pdic_data = data->phy_driver_data;
	struct i2c_client *i2c = pdic_data->i2c;

	sm5038_set_vconn_source(data, VCONN_TURN_ON);
	sm5038_vbus_turn_on_ctrl(pdic_data, 1);

	/* Hard Reset Done Notify to PRL */
	sm5038_usbpd_write_reg(i2c, SM5038_REG_PD_CNTL4,
			SM5038_REG_CNTL_NOTIFY_RESET_DONE);
	dev_info(pdic_data->dev, "%s\n", __func__);
}
EXPORT_SYMBOL_GPL(sm5038_src_transition_to_pwr_on);

void sm5038_snk_transition_to_default(void *_data)
{
	struct sm5038_usbpd_data *data = (struct sm5038_usbpd_data *) _data;
	struct sm5038_phydrv_data *pdic_data = data->phy_driver_data;
	struct sm5038_usbpd_manager_data *manager = &data->manager;
	struct i2c_client *i2c = pdic_data->i2c;

	sm5038_set_vconn_source(data, VCONN_TURN_OFF);
	if (manager->dp_is_connect == 1) {
		sm5038_usbpd_dp_detach(pdic_data->dev);
		manager->alt_sended = 0;
		manager->vdm_en = 0;
	}
	sm5038_set_ufp(i2c);
	pdic_data->data_role = TYPEC_DEVICE;
	pdic_data->pd_support = 0;

	sm5038_usbpd_uevent_notifier(&data->dev);

	/* Hard Reset Done Notify to PRL */
	sm5038_usbpd_write_reg(i2c, SM5038_REG_PD_CNTL4,
			SM5038_REG_CNTL_NOTIFY_RESET_DONE);
	dev_info(pdic_data->dev, "%s\n", __func__);
}
EXPORT_SYMBOL_GPL(sm5038_snk_transition_to_default);

bool sm5038_check_vbus_state(void *_data)
{
	struct sm5038_usbpd_data *pd_data = (struct sm5038_usbpd_data *) _data;
	struct sm5038_phydrv_data *pdic_data = pd_data->phy_driver_data;
	struct i2c_client *i2c = pdic_data->i2c;
	u8 val;

	sm5038_usbpd_read_reg(i2c, SM5038_REG_PROBE0, &val);
	if (val & 0x40) /* VBUS OK */
		return true;
	else
		return false;
}
EXPORT_SYMBOL_GPL(sm5038_check_vbus_state);

static void sm5038_usbpd_abnormal_reset_check(struct sm5038_phydrv_data *pdic_data)
{
	struct sm5038_usbpd_data *pd_data = dev_get_drvdata(pdic_data->dev);
	struct i2c_client *i2c = pdic_data->i2c;
	u8 reg_data = 0;

	sm5038_usbpd_read_reg(i2c, SM5038_REG_CC_CNTL1, &reg_data);
	pr_info("%s, CC_CNTL1 : 0x%x\n", __func__, reg_data);

	if (reg_data == 0x45) { /* surge reset */
		sm5038_driver_reset(pd_data);
		sm5038_usbpd_reg_init(pdic_data);
	}
}

static int sm5038_usbpd_check_normal_audio_device(struct sm5038_phydrv_data *pdic_data)
{
	struct sm5038_usbpd_data *pd_data = dev_get_drvdata(pdic_data->dev);
	struct i2c_client *i2c = pdic_data->i2c;
	u8 adc_cc1 = 0, adc_cc2 = 0;
	int ret = 0;

	sm5038_usbpd_set_rp_scr_sel(pd_data, PLUG_CTRL_RP180);

	sm5038_usbpd_write_reg(i2c, SM5038_REG_ADC_CNTL1, SM5038_ADC_PATH_SEL_CC1);
	sm5038_adc_value_read(pdic_data, &adc_cc1);

	sm5038_usbpd_write_reg(i2c, SM5038_REG_ADC_CNTL1, SM5038_ADC_PATH_SEL_CC2);
	sm5038_adc_value_read(pdic_data, &adc_cc2);

	if ((adc_cc1 <= 0xF) && (adc_cc2 <= 0xF)) { /* Ra/Ra */
		sm5038_usbpd_set_rp_scr_sel(pd_data, PLUG_CTRL_RP80);
		ret = 1;
	}

	pr_info("%s, CC1 : 0x%x, CC2 : 0x%x, ret = %d\n", __func__, adc_cc1, adc_cc2, ret);

	return ret;
}

#if defined(CONFIG_SM5038_WATER_DETECTION_ENABLE)
static void sm5038_abnormal_dev_int_on_off(struct sm5038_phydrv_data *pdic_data, int onoff)
{
	struct i2c_client *i2c = pdic_data->i2c;
	u8 val;

	sm5038_usbpd_read_reg(i2c, 0xEE, &val);
	if (onoff == 1)
		val |= 0x2;
	else
		val &= 0xFD;
	sm5038_usbpd_write_reg(i2c, 0xEE, val);
	pr_info("%s: ABNORMAL_DEV INT [%d], val = 0x%x\n", __func__, onoff, val);
}

static void sm5038_usbpd_check_normal_otg_device(struct sm5038_phydrv_data *pdic_data)
{
	struct i2c_client *i2c = pdic_data->i2c;
	u8 cc1_80ua, cc2_80ua, cc1_fe, cc2_fe, reg_data;
	bool abnormal_st = false;
	int reTry = 0;

	if (pdic_data->abnormal_dev_cnt < 2)
		return;
	else if (pdic_data->abnormal_dev_cnt > 2) {
		sm5038_abnormal_dev_int_on_off(pdic_data, 0);
		/* Timer set 1s */
		sm5038_usbpd_write_reg(i2c, SM5038_REG_GEN_TMR_L, 0x10);
		sm5038_usbpd_write_reg(i2c, SM5038_REG_GEN_TMR_U, 0x27);
		pdic_data->is_timer_expired = true;
		return;
	}

	sm5038_usbpd_write_reg(i2c, 0xFE, 0x02);

	sm5038_usbpd_read_reg(i2c, 0x18, &cc1_80ua);
	sm5038_usbpd_read_reg(i2c, 0x19, &cc2_80ua);
	sm5038_usbpd_read_reg(i2c, 0x1F, &cc1_fe);
	sm5038_usbpd_read_reg(i2c, 0x21, &cc2_fe);

	sm5038_usbpd_write_reg(i2c, 0xFE, 0x00);

	sm5038_usbpd_read_reg(i2c, 0xFE, &reg_data);
	for (reTry = 0; reTry < 5; reTry++) {
		if (reg_data != 0x00) {
			usleep_range(1000, 1100);
			pr_info("%s, 0xFE : 0x%x, retry = %d\n", __func__, reg_data, reTry+1);
			sm5038_usbpd_write_reg(i2c, 0xFE, 0x00);
			sm5038_usbpd_read_reg(i2c, 0xFE, &reg_data);
		} else
			break;
	}
	if (reg_data != 0x00)
		panic("sm5038 usbpd i2c error!!");
	if ((cc1_80ua >= 0xC) && (cc1_80ua <= 0x16) && (cc1_fe < 0x4))
		abnormal_st = true;
	else if ((cc2_80ua >= 0xC) && (cc2_80ua <= 0x16) && (cc2_fe < 0x4))
		abnormal_st = true;

	if (abnormal_st) {
		sm5038_usbpd_write_reg(i2c, 0x93, 0x04);
		sm5038_usbpd_write_reg(i2c, 0x93, 0x00);
		pdic_data->abnormal_dev_cnt = 0;
	}

	pr_info("%s, ABNOR_ST : %d, CC1_80uA[0x%x] CC2_80uA[0x%x] CC1_FE[0x%x] CC2_FE[0x%x]\n",
			__func__, abnormal_st, cc1_80ua, cc2_80ua, cc1_fe, cc2_fe);
}
#endif

static void sm5038_assert_rd(void *_data)
{
	struct sm5038_usbpd_data *data = (struct sm5038_usbpd_data *) _data;
	struct sm5038_phydrv_data *pdic_data = data->phy_driver_data;
	struct i2c_client *i2c = pdic_data->i2c;
	u8 val;

	sm5038_usbpd_read_reg(i2c, SM5038_REG_CC_CNTL7, &val);

	val ^= 0x01;
	/* Apply CC State PR_Swap (Att.Src -> Att.Snk) */
	sm5038_usbpd_write_reg(i2c, SM5038_REG_CC_CNTL7, val);
}

static void sm5038_assert_rp(void *_data)
{
	struct sm5038_usbpd_data *data = (struct sm5038_usbpd_data *) _data;
	struct sm5038_phydrv_data *pdic_data = data->phy_driver_data;
	struct i2c_client *i2c = pdic_data->i2c;
	u8 val;

	sm5038_usbpd_read_reg(i2c, SM5038_REG_CC_CNTL7, &val);

	val ^= 0x01;
	/* Apply CC State PR_Swap (Att.Snk -> Att.Src) */
	sm5038_usbpd_write_reg(i2c, SM5038_REG_CC_CNTL7, val);
}

static unsigned int sm5038_get_status(void *_data, unsigned int flag)
{
	unsigned int ret;
	struct sm5038_usbpd_data *data = (struct sm5038_usbpd_data *) _data;
	struct sm5038_phydrv_data *pdic_data = data->phy_driver_data;

	if (pdic_data->status_reg & flag) {
		ret = pdic_data->status_reg & flag;
		dev_info(pdic_data->dev, "%s: status_reg = (%x)\n",
				__func__, ret);
		pdic_data->status_reg &= ~flag; /* clear the flag */
		return ret;
	} else {
		return 0;
	}
}

static bool sm5038_poll_status(void *_data, int irq)
{
	struct sm5038_usbpd_data *data = (struct sm5038_usbpd_data *) _data;
	struct sm5038_phydrv_data *pdic_data = data->phy_driver_data;
	struct i2c_client *i2c = pdic_data->i2c;
	struct device *dev = &i2c->dev;
	u8 intr[5] = {0};
	u8 status[5] = {0};
	int ret = 0;

	ret = sm5038_usbpd_multi_read(i2c, SM5038_REG_INT1, 5, intr);
	ret = sm5038_usbpd_multi_read(i2c, SM5038_REG_STATUS1, 5, status);

	if (irq == (-1)) {
		intr[0] = (status[0] & ENABLED_INT_1);
		intr[1] = (status[1] & ENABLED_INT_2);
		intr[2] = (status[2] & ENABLED_INT_3);
		intr[3] = (status[3] & ENABLED_INT_4);
		intr[4] = (status[4] & ENABLED_INT_5);
	}

	dev_info(dev, "%s: INT[0x%x 0x%x 0x%x 0x%x 0x%x], STATUS[0x%x 0x%x 0x%x 0x%x 0x%x]\n",
		__func__, intr[0], intr[1], intr[2], intr[3], intr[4],
		status[0], status[1], status[2], status[3], status[4]);

	if ((intr[0] | intr[1] | intr[2] | intr[3] | intr[4]) == 0) {
		sm5038_usbpd_abnormal_reset_check(pdic_data);
		pdic_data->status_reg |= MSG_NONE;
		goto out;
	}
#if defined(CONFIG_SM5038_WATER_DETECTION_ENABLE)
	if (intr[0] & SM5038_REG_INT_STATUS1_TMR_EXP) {
		if (pdic_data->is_timer_expired) {
			pdic_data->is_timer_expired = false;
			if (pdic_data->abnormal_dev_cnt != 0)
				pdic_data->abnormal_dev_cnt = 0;
			sm5038_abnormal_dev_int_on_off(pdic_data, 1);
		}
	}

	if (intr[0]	& SM5038_REG_INT_STATUS1_ABNORMAL_DEV) {
		pdic_data->abnormal_dev_cnt++;
		sm5038_usbpd_check_normal_otg_device(pdic_data);
	}

	if ((intr[2] & SM5038_REG_INT_STATUS3_WATER) &&
			(status[2] & SM5038_REG_INT_STATUS3_WATER))			
			sm5038_process_cc_water_det(pdic_data, WATER_MODE_ON);
#endif
#if defined(CONFIG_SM5038_SHORT_PROTECTION)
	if ((intr[4] & SM5038_REG_INT_STATUS5_CC_ABNORMAL) &&
			(status[4] & SM5038_REG_INT_STATUS5_CC_ABNORMAL)) {
		pdic_data->is_cc_abnormal_state = true;
		if ((status[0] & SM5038_REG_INT_STATUS1_ATTACH) &&
				pdic_data->is_attached) {
			/* rp abnormal */
			sm5038_notify_rp_abnormal(_data);
		}
		pr_info("%s, CC_ABNORMAL\n", __func__);
	}
#endif

	if (intr[1] & SM5038_REG_INT_STATUS2_TX_HDR_ERR)
		pdic_data->soft_reset = true;

	if (intr[1] & SM5038_REG_INT_STATUS2_SRC_ADV_CHG)
		sm5038_notify_rp_current_level(data);

	if (intr[1] & SM5038_REG_INT_STATUS2_VBUS_0V) {
#if defined(CONFIG_SM5038_WATER_DETECTION_ENABLE)
		if ((status[2] & SM5038_REG_INT_STATUS3_WATER_RLS) &&
			(status[0] & SM5038_REG_INT_STATUS1_DETACH) &&
				pdic_data->is_water_detect)
			sm5038_process_cc_water_det(pdic_data, WATER_MODE_OFF);
#endif
	}

	if ((intr[0] & SM5038_REG_INT_STATUS1_VBUSPOK) &&
			(status[0] & SM5038_REG_INT_STATUS1_VBUSPOK))
		sm5038_check_cc_state(pdic_data);

#if defined(CONFIG_SM5038_WATER_DETECTION_ENABLE)
	if (intr[2] & SM5038_REG_INT_STATUS3_WATER_RLS) {
		if ((intr[2] & SM5038_REG_INT_STATUS3_WATER) == 0 &&
				(irq != (-1)) && pdic_data->is_water_detect)
			sm5038_process_cc_water_det(pdic_data, WATER_MODE_OFF);
	}
#endif

	if ((intr[0] & SM5038_REG_INT_STATUS1_DETACH) &&
			(status[0] & SM5038_REG_INT_STATUS1_DETACH)) {
		pdic_data->status_reg |= PLUG_DETACH;
		sm5038_set_vconn_source(data, VCONN_TURN_OFF);
		if (irq != (-1))
			sm5038_usbpd_set_vbus_dischg_gpio(pdic_data, 1);
	}

	mutex_lock(&pdic_data->lpm_mutex);
	if ((intr[0] & SM5038_REG_INT_STATUS1_ATTACH) &&
#if defined(CONFIG_SM5038_WATER_DETECTION_ENABLE)
			(!pdic_data->is_water_detect) &&
#endif
			(status[0] & SM5038_REG_INT_STATUS1_ATTACH)) {
		pdic_data->status_reg |= PLUG_ATTACH;
		if (irq != (-1))
			sm5038_usbpd_set_vbus_dischg_gpio(pdic_data, 0);
	}
	mutex_unlock(&pdic_data->lpm_mutex);

	if (intr[3] & SM5038_REG_INT_STATUS4_HRST_RCVED) {
		pdic_data->status_reg |= MSG_HARDRESET;
		goto out;
	}

	if ((intr[1] & SM5038_REG_INT_STATUS2_VCONN_DISCHG) ||
			(intr[2] & SM5038_REG_INT_STATUS3_VCONN_OCP))
		sm5038_set_vconn_source(data, VCONN_TURN_OFF);

	if (intr[3] & SM5038_REG_INT_STATUS4_RX_DONE)
		sm5038_usbpd_protocol_rx(data);

	if (intr[3] & SM5038_REG_INT_STATUS4_TX_DONE) {
		data->protocol_tx.status = MESSAGE_SENT;
		pdic_data->status_reg |= MSG_GOODCRC;
	}

	if (intr[3] & SM5038_REG_INT_STATUS4_TX_DISCARD) {
		data->protocol_tx.status = TRANSMISSION_ERROR;
		pdic_data->status_reg |= MSG_PASS;
		sm5038_usbpd_tx_request_discard(data);
	}

	if (intr[3] & SM5038_REG_INT_STATUS4_TX_SOP_ERR)
		data->protocol_tx.status = TRANSMISSION_ERROR;

	if ((intr[3] & SM5038_REG_INT_STATUS4_PRL_RST_DONE) ||
			(intr[3] & SM5038_REG_INT_STATUS4_HCRST_DONE))
		pdic_data->reset_done = 1;

out:
	if (pdic_data->status_reg & data->wait_for_msg_arrived) {
		dev_info(pdic_data->dev, "%s: wait_for_msg_arrived = (%d)\n",
				__func__, data->wait_for_msg_arrived);
		data->wait_for_msg_arrived = 0;
		complete(&data->msg_arrived);
	}

	return 0;
}

static int sm5038_hard_reset(void *_data)
{
	struct sm5038_usbpd_data *data = (struct sm5038_usbpd_data *) _data;
	struct sm5038_phydrv_data *pdic_data = data->phy_driver_data;
	struct i2c_client *i2c = pdic_data->i2c;
	int ret;
	u8 val;

	sm5038_usbpd_read_reg(i2c, SM5038_REG_PD_CNTL4, &val);
	val |= SM5038_REG_CNTL_HARD_RESET_MESSAGE;
	ret = sm5038_usbpd_write_reg(i2c, SM5038_REG_PD_CNTL4, val);
	if (ret < 0)
		goto fail;

	pdic_data->status_reg = 0;

	return 0;

fail:
	return -EIO;
}

static int sm5038_receive_message(void *_data)
{
	struct sm5038_usbpd_data *data = (struct sm5038_usbpd_data *) _data;
	struct sm5038_phydrv_data *pdic_data = data->phy_driver_data;
	struct sm5038_policy_data *policy = &data->policy;
	struct i2c_client *i2c = pdic_data->i2c;
	struct device *dev = &i2c->dev;
	int obj_num = 0;
	int ret = 0;
	u8 val;

	ret = sm5038_read_rx_header(i2c, &pdic_data->header);
	if (ret < 0)
		dev_err(dev, "%s read msg header error\n", __func__);

	obj_num = pdic_data->header.num_data_objs;

	if (obj_num > 0) {
		ret = sm5038_read_msg_obj(i2c,
			obj_num, &pdic_data->obj[0]);
	}

	sm5038_usbpd_read_reg(i2c, SM5038_REG_RX_SRC, &val);
	/* 0: SOP, 1: SOP', 2: SOP", 3: SOP' Debug, 4: SOP" Debug */
	policy->origin_message = val & 0x0F;
	dev_info(pdic_data->dev, "%s: Origin of Message = (%x)\n",
			__func__, val);

	/* Notify Rxed Message Read Done */
	sm5038_usbpd_write_reg(i2c, SM5038_REG_RX_BUF, 0x80);

	return ret;
}

static int sm5038_tx_msg(void *_data,
		msg_header *header, data_obj_type *obj)
{
	struct sm5038_usbpd_data *data = (struct sm5038_usbpd_data *) _data;
	struct sm5038_phydrv_data *pdic_data = data->phy_driver_data;
	struct i2c_client *i2c = pdic_data->i2c;
	int ret = 0;
	int count = 0;

	mutex_lock(&pdic_data->_mutex);

	/* if there is no attach, skip tx msg */
	if (pdic_data->detach_valid)
		goto done;

	ret = sm5038_write_tx_header(i2c, header->byte);
	if (ret < 0)
		goto done;

	count = header->num_data_objs;

	if (count > 0) {
		ret = sm5038_write_msg_obj(i2c, count, obj);
		if (ret < 0)
			goto done;
	}

	ret = sm5038_send_msg(data, i2c);
	if (ret < 0)
		goto done;

	pdic_data->status_reg = 0;
	data->wait_for_msg_arrived = 0;

done:
	mutex_unlock(&pdic_data->_mutex);
	return ret;
}

static int sm5038_rx_msg(void *_data,
		msg_header *header, data_obj_type *obj)
{
	struct sm5038_usbpd_data *data = (struct sm5038_usbpd_data *) _data;
	struct sm5038_phydrv_data *pdic_data = data->phy_driver_data;
	int i;
	int count = 0;

	if (!sm5038_receive_message(data)) {
		header->word = pdic_data->header.word;
		count = pdic_data->header.num_data_objs;
		if (count > 0) {
			for (i = 0; i < count; i++)
				obj[i].object = pdic_data->obj[i].object;
		}
		pdic_data->header.word = 0; /* To clear for duplicated call */
		return 0;
	} else {
		return -EINVAL;
	}
}

static int sm5038_get_vconn_source(void *_data, int *val)
{
	struct sm5038_usbpd_data *data = (struct sm5038_usbpd_data *) _data;
	struct sm5038_phydrv_data *pdic_data = data->phy_driver_data;

	*val = pdic_data->vconn_source;
	return 0;
}

/* val : sink(0) or source(1) */
static int sm5038_set_power_role(void *_data, int val)
{
	struct sm5038_usbpd_data *data = (struct sm5038_usbpd_data *) _data;
	struct sm5038_phydrv_data *pdic_data = data->phy_driver_data;

	pr_info("%s: pr_swap received to %s\n",	__func__, val == 1 ? "SRC" : "SNK");

	if (val == TYPEC_SINK) {
		pdic_data->typec_power_role = TYPEC_SINK;
		sm5038_assert_rd(data);
		sm5038_set_snk(pdic_data->i2c);
	} else {
		pdic_data->typec_power_role = TYPEC_SOURCE;
		sm5038_assert_rp(data);
		sm5038_set_src(pdic_data->i2c);
	}

	typec_set_pwr_role(pdic_data->port, pdic_data->typec_power_role);
	pdic_data->power_role = val;
	return 0;
}

static int sm5038_get_power_role(void *_data, int *val)
{
	struct sm5038_usbpd_data *data = (struct sm5038_usbpd_data *) _data;
	struct sm5038_phydrv_data *pdic_data = data->phy_driver_data;
	*val = pdic_data->power_role;
	return 0;
}

static int sm5038_set_data_role(void *_data, int val)
{
	struct sm5038_usbpd_data *data = (struct sm5038_usbpd_data *) _data;
	struct sm5038_phydrv_data *pdic_data = data->phy_driver_data;
	struct i2c_client *i2c = pdic_data->i2c;

	pr_info("%s: dr_swap received to %s\n", __func__, val == 1 ? "DFP" : "UFP");

	if (val == TYPEC_DEVICE)
		sm5038_set_ufp(i2c);
	else
		sm5038_set_dfp(i2c);

	pdic_data->data_role = val;

	sm5038_process_dr_swap(pdic_data, val);
	return 0;
}

static int sm5038_get_data_role(void *_data, int *val)
{
	struct sm5038_usbpd_data *data = (struct sm5038_usbpd_data *) _data;
	struct sm5038_phydrv_data *pdic_data = data->phy_driver_data;
	*val = pdic_data->data_role;
	return 0;
}

static int sm5038_set_check_msg_pass(void *_data, int val)
{
	struct sm5038_usbpd_data *data = (struct sm5038_usbpd_data *) _data;
	struct sm5038_phydrv_data *pdic_data = data->phy_driver_data;

	dev_info(pdic_data->dev, "%s: check_msg_pass val(%d)\n", __func__, val);

	pdic_data->check_msg_pass = val;

	return 0;
}

void sm5038_set_bist_carrier_m2(void *_data)
{
	struct sm5038_usbpd_data *data = (struct sm5038_usbpd_data *) _data;
	struct sm5038_phydrv_data *pdic_data = data->phy_driver_data;
	struct i2c_client *i2c = pdic_data->i2c;
	u8 val;

	sm5038_usbpd_read_reg(i2c, SM5038_REG_PD_CNTL2, &val);
	val |= 0x10;
	sm5038_usbpd_write_reg(i2c, SM5038_REG_PD_CNTL2, val);

	msleep(30);

	sm5038_usbpd_read_reg(i2c, SM5038_REG_PD_CNTL2, &val);
	val &= 0xEF;
	sm5038_usbpd_write_reg(i2c, SM5038_REG_PD_CNTL2, val);
	pr_info("%s\n", __func__);
}
EXPORT_SYMBOL_GPL(sm5038_set_bist_carrier_m2);

void sm5038_error_recovery_mode(void *_data)
{
	struct sm5038_usbpd_data *pd_data = (struct sm5038_usbpd_data *) _data;
//	struct sm5038_phydrv_data *pdic_data = pd_data->phy_driver_data;
//	struct i2c_client *i2c = pdic_data->i2c;
	int power_role = 0;
//	u8 data;

	sm5038_get_power_role(pd_data, &power_role);
#if 0 /* need to check */
	if (pdic_data->is_attached) {
		sm5038_usbpd_read_reg(i2c, SM5038_REG_CC_CNTL3, &data);
		data |= 0x04; /* go to ErrorRecovery State */
		sm5038_usbpd_write_reg(i2c, SM5038_REG_CC_CNTL3, data);
	}
#endif
	pr_info("%s: power_role = %s\n", __func__, power_role ? "SRC" : "SNK");
}
EXPORT_SYMBOL_GPL(sm5038_error_recovery_mode);

#if 0
static int check_usb_killer(struct sm5038_phydrv_data *pdic_data)
{
	struct i2c_client *i2c = pdic_data->i2c;
	u8 reg_data = 0;
	int ret = 0, retry = 0;

	sm5038_usbpd_write_reg(i2c, SM5038_REG_USBK_CNTL, 0x80);
	msleep(5);

	for (retry = 0; retry < 3; retry++) {
		sm5038_usbpd_read_reg(i2c, SM5038_REG_USBK_CNTL, &reg_data);
		pr_info("%s, USBK_CNTL : 0x%x\n", __func__, reg_data);
		if (reg_data & 0x02) {
			if (reg_data & 0x01)
				ret = 1;
			else
				ret = 0;
			break;
		} else {
			pr_info("%s, DPDM_DONE is not yet, retry : %d\n", __func__, retry+1);
		}
	}

	return ret;
}
#endif

void sm5038_charger_psy_changed(enum power_supply_property psp,
		const union power_supply_propval val)
{
	struct power_supply *psy;

	psy = power_supply_get_by_name("battery");

#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
	power_supply_set_property(psy, psp, &val);
#endif
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	if (psy)
		power_supply_set_property(psy, psp, &val);
#endif
}
EXPORT_SYMBOL_GPL(sm5038_charger_psy_changed);

void sm5038_vbus_turn_on_ctrl(struct sm5038_phydrv_data *usbpd_data,
	bool enable)
{
	struct power_supply *charger_otg;
	union power_supply_propval val;
	int on = !!enable;
	int ret = 0;
#if 0
	if (enable && (policy->state != PE_PRS_SNK_SRC_Source_on) &&
			(policy->state != PE_SRC_Transition_to_default) &&
			check_usb_killer(usbpd_data)) {
		pr_info("%s : do not turn on VBUS because of USB Killer.\n",
			__func__);
		return;
	}
#endif
	pr_info("%s : enable=%d\n", __func__, enable);

	charger_otg = power_supply_get_by_name("sm5038-otg");

	if (charger_otg) {
		val.intval = enable;
		usbpd_data->is_otg_vboost = enable;
		ret = charger_otg->desc->set_property(charger_otg,
				POWER_SUPPLY_PROP_ONLINE, &val);
	} else {
		pr_err("%s: Fail to get psy battery\n", __func__);
	}
	if (ret) {
		pr_err("%s: fail to set power_suppy ONLINE property(%d)\n",
			__func__, ret);
	} else {
		pr_info("otg accessory power = %d\n", on);
	}
}
EXPORT_SYMBOL_GPL(sm5038_vbus_turn_on_ctrl);

static int sm5038_usbpd_notify_attach(void *data)
{
	struct sm5038_phydrv_data *pdic_data = data;
	struct i2c_client *i2c = pdic_data->i2c;
	struct device *dev = &i2c->dev;
	struct sm5038_usbpd_data *pd_data = dev_get_drvdata(dev);
//	struct sm5038_usbpd_manager_data *manager = &pd_data->manager;
	bool short_cable = false;
	u8 reg_data;
	int ret = 0;
	int prev_power_role = pdic_data->typec_power_role;
	union power_supply_propval val = {0, };

	ret = sm5038_usbpd_read_reg(i2c, SM5038_REG_CC_STATUS, &reg_data);
	if (ret < 0)
		dev_err(dev, "%s, i2c read CC_STATUS error\n", __func__);
	pdic_data->is_attached = 1;
#if defined(CONFIG_SM5038_WATER_DETECTION_ENABLE)
	pdic_data->abnormal_dev_cnt = 0;
#endif
	/* cc_SOURCE */
	if ((reg_data & SM5038_ATTACH_TYPE) == SM5038_ATTACH_SOURCE) {
		dev_info(dev, "ccstat : cc_SOURCE\n");
		if (prev_power_role == TYPEC_SOURCE)
			sm5038_vbus_turn_on_ctrl(pdic_data, 0);

#if defined(CONFIG_SM5038_SUPPORT_SBU) && defined(CONFIG_SM5038_SHORT_PROTECTION)
		sm5038_sbu_short_state_check(pdic_data);
#endif

#if defined(CONFIG_SM5038_WATER_DETECTION_ENABLE)
		if (pdic_data->is_water_detect)
			return -EPERM;
#endif

		pdic_data->power_role = TYPEC_SINK;
		pdic_data->data_role = TYPEC_DEVICE;
		sm5038_set_snk(i2c);
		sm5038_set_ufp(i2c);

		sm5038_usbpd_policy_reset(pd_data, PLUG_ATTACHED);
#if defined(CONFIG_SM5038_SHORT_PROTECTION)
		sm5038_get_short_state(pd_data, &short_cable);
		if (short_cable) /* rp abnormal */
			sm5038_notify_rp_abnormal(pd_data);
#endif
		if (!short_cable) /* rp current */
			sm5038_notify_rp_current_level(pd_data);

		if (!pdic_data->detach_valid &&
			pdic_data->typec_data_role == TYPEC_HOST) {
			stop_usb_host(pdic_data);
			sm5038_typec_partner_event(pdic_data,
				TYPEC_STATUS_NOTIFY_DETACH);
			dev_info(dev, "directly called from DFP to UFP\n");
		}
		pdic_data->typec_power_role = TYPEC_SINK;
		typec_set_pwr_role(pdic_data->port, TYPEC_SINK);
		start_usb_peripheral(pdic_data);
		sm5038_typec_partner_event(pdic_data,
			TYPEC_STATUS_NOTIFY_ATTACH_UFP);
		if (pdic_data->vconn_source)
			sm5038_set_vconn_source(pd_data, VCONN_TURN_OFF);
	/* cc_SINK */
	} else if ((reg_data & SM5038_ATTACH_TYPE) == SM5038_ATTACH_SINK) {
		dev_info(dev, "Type-C Sink connected\n");
#if 0
		if (check_usb_killer(pdic_data) == 0)
			return -EPERM;
#endif
		if (pdic_data->scr_sel == PLUG_CTRL_RP180)
			sm5038_usbpd_set_rp_scr_sel(pd_data, PLUG_CTRL_RP80);

		sm5038_set_vconn_source(pd_data, VCONN_TURN_ON);

		if (prev_power_role == TYPEC_SINK)
			sm5038_vbus_turn_on_ctrl(pdic_data, 1);
		pdic_data->power_role = TYPEC_SOURCE;
		pdic_data->data_role = TYPEC_HOST;
		sm5038_set_pd_function(pd_data, PD_ENABLE);
		sm5038_usbpd_policy_reset(pd_data, PLUG_ATTACHED);

		val.intval = 1;
		psy_do_property("pc_port", set,
		POWER_SUPPLY_PROP_TECHNOLOGY, val);

		if (!pdic_data->detach_valid &&
			pdic_data->typec_data_role == TYPEC_DEVICE) {
			stop_usb_peripheral(pdic_data);
			sm5038_typec_partner_event(pdic_data,
				TYPEC_STATUS_NOTIFY_DETACH);
			dev_info(dev, "directly called from UFP to DFP\n");
		}
		pdic_data->typec_power_role = TYPEC_SOURCE;
		typec_set_pwr_role(pdic_data->port, TYPEC_SOURCE);
		start_usb_host(pdic_data, true);
		sm5038_typec_partner_event(pdic_data,
				TYPEC_STATUS_NOTIFY_ATTACH_DFP);

		sm5038_set_dfp(i2c);
		sm5038_set_src(i2c);
		msleep(180); /* don't over 310~620ms(tTypeCSinkWaitCap) */
		/* cc_AUDIO */
	} else if ((reg_data & SM5038_ATTACH_TYPE) == SM5038_ATTACH_AUDIO) {
		if (sm5038_usbpd_check_normal_audio_device(pdic_data) == 0) {
			/* Set 'CC_UNATT_SRC' state */
			sm5038_usbpd_write_reg(i2c, SM5038_REG_CC_CNTL3, 0x81);
			return -EPERM;
		}
		dev_info(dev, "ccstat : cc_AUDIO\n");
//		manager->acc_type = QTI_POWER_SUPPLY_TYPEC_SINK_AUDIO_ADAPTER;
	} else {
		dev_err(dev, "%s, PLUG Error\n", __func__);
		return -EPERM;
	}

	pdic_data->detach_valid = false;

	return ret;
}

static void sm5038_usbpd_notify_detach(void *data)
{
	struct sm5038_phydrv_data *pdic_data = data;
	struct i2c_client *i2c = pdic_data->i2c;
	struct device *dev = &i2c->dev;
	struct sm5038_usbpd_data *pd_data = dev_get_drvdata(dev);
	struct sm5038_usbpd_manager_data *manager = &pd_data->manager;
	u8 reg_data = 0;
	union power_supply_propval val = {0, };

	dev_info(dev, "ccstat : cc_No_Connection\n");
	sm5038_vbus_turn_on_ctrl(pdic_data, 0);
	sm5038_usbpd_reinit(dev);
	pdic_data->is_attached = 0;
	pdic_data->status_reg = 0;
	pdic_data->detach_valid = true;
#if defined(CONFIG_SM5038_SHORT_PROTECTION)
	pdic_data->is_cc_abnormal_state = false;
#if defined(CONFIG_SM5038_SUPPORT_SBU)
	pdic_data->is_sbu_abnormal_state = false;
#endif
#endif
	pdic_data->reset_done = 0;
	pdic_data->pd_support = 0;
	pdic_data->rp_currentlvl = RP_CURRENT_LEVEL_NONE;
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	val.intval = SOMC_POWER_SUPPLY_EXT_TYPE_RP_NONE;
	sm5038_charger_psy_changed(POWER_SUPPLY_PROP_ONLINE, val);
#endif
	sm5038_usbpd_policy_reset(pd_data, PLUG_DETACHED);

	if (pdic_data->typec_data_role == TYPEC_HOST) {
		stop_usb_host(pdic_data);
		val.intval = 0;
		psy_do_property("pc_port", set,
		POWER_SUPPLY_PROP_TECHNOLOGY, val);
	} else
		stop_usb_peripheral(pdic_data);

	if (pdic_data->partner) {
		pr_info("%s : typec_unregister_partner - pd_support : %d\n",
			__func__, pdic_data->pd_support);
		if (!IS_ERR(pdic_data->partner))
			typec_unregister_partner(pdic_data->partner);
		pdic_data->partner = NULL;
		pdic_data->typec_power_role = TYPEC_SINK;
		pdic_data->typec_data_role = TYPEC_DEVICE;
		pdic_data->pwr_opmode = TYPEC_PWR_MODE_USB;
	}
	if (pdic_data->typec_try_state_change == ROLE_SWAP_PR ||
		pdic_data->typec_try_state_change == ROLE_SWAP_DR) {
		/* Role change try and new mode detected */
		pr_info("%s : typec_reverse_completion, detached while pd/dr_swap",
			__func__);
		pdic_data->typec_try_state_change = ROLE_SWAP_NONE;
		complete(&pdic_data->typec_reverse_completion);
	}
	sm5038_usbpd_uevent_notifier(&pd_data->dev);
	pdic_data->typec_power_role = TYPEC_SINK;

	sm5038_typec_partner_event(pdic_data,
		TYPEC_STATUS_NOTIFY_DETACH);
	if (!pdic_data->typec_try_state_change &&
			!(pdic_data->scr_sel == PLUG_CTRL_RP180))
		sm5038_rprd_mode_change(pdic_data, TYPEC_PORT_DRP);
#if defined(CONFIG_SM5038_WATER_DETECTION_ENABLE)
	sm5038_usbpd_write_reg(i2c, SM5038_REG_CORR_CNTL5, 0x00);
	sm5038_usbpd_write_reg(i2c, 0xFE, 0x00);
#endif
	/* Set Sink / UFP */
	sm5038_usbpd_write_reg(i2c, SM5038_REG_PD_CNTL2, 0x04);
	sm5038_usbpd_read_reg(i2c, SM5038_REG_PD_STATE3, &reg_data);
	if (reg_data & 0x06) /* Reset Done */
		sm5038_usbpd_write_reg(i2c, SM5038_REG_PD_CNTL4,
				SM5038_REG_CNTL_NOTIFY_RESET_DONE);
	else /* Protocol Layer reset */
		sm5038_usbpd_write_reg(i2c, SM5038_REG_PD_CNTL4, 0x00);
	sm5038_set_pd_function(pd_data, PD_DISABLE);
	sm5038_usbpd_read_reg(i2c, SM5038_REG_CC_CNTL7, &reg_data);
	reg_data &= 0xFC;
	sm5038_usbpd_write_reg(i2c, SM5038_REG_CC_CNTL7, reg_data);
	if (!pdic_data->is_usb_therm)
		sm5038_usbpd_write_reg(i2c, SM5038_REG_CC_CNTL3, SM5038_CC_OP_EN);
	sm5038_usbpd_acc_detach(dev);
	if (manager->dp_is_connect == 1) {
		sm5038_usbpd_dp_detach(dev);
//		extcon_blocking_sync(pdic_data->extcon, EXTCON_DISP_DP, 0);
	}
	if (pdic_data->soft_reset) {
		pdic_data->soft_reset = false;
		dev_info(dev, "Do soft reset.\n");
		sm5038_usbpd_write_reg(i2c, SM5038_REG_SYS_CNTL, SM5038_CC_OP_EN);
		sm5038_driver_reset(pd_data);
		sm5038_usbpd_reg_init(pdic_data);
	}
}

static irqreturn_t sm5038_pdic_irq_thread(int irq, void *data)
{
	struct sm5038_phydrv_data *pdic_data = data;
	struct i2c_client *i2c = pdic_data->i2c;
	struct device *dev = &i2c->dev;
	struct sm5038_usbpd_data *pd_data = dev_get_drvdata(dev);
	int ret = 0;

	dev_info(dev, "%s, irq = %d\n", __func__, irq);

	__pm_stay_awake(pdic_data->irq_ws);
	ret = wait_event_timeout(pdic_data->suspend_wait,
						!pdic_data->suspended,
						msecs_to_jiffies(200));
	if (!ret) {
		pr_info("%s suspend_wait timeout\n", __func__);
		__pm_relax(pdic_data->irq_ws);
		return IRQ_HANDLED;
	}

	mutex_lock(&pdic_data->_mutex);

	sm5038_poll_status(pd_data, irq);

	if (sm5038_get_status(pd_data, MSG_NONE))
		goto out;

#if defined(CONFIG_SM5038_WATER_DETECTION_ENABLE)
	if (pdic_data->is_water_detect)
		goto out;
#endif

	if (sm5038_get_status(pd_data, MSG_HARDRESET)) {
		sm5038_usbpd_rx_hard_reset(dev);
		sm5038_usbpd_kick_policy_work(dev);
		goto out;
	}

	if (sm5038_get_status(pd_data, MSG_SOFTRESET)) {
		sm5038_usbpd_rx_soft_reset(pd_data);
		sm5038_usbpd_kick_policy_work(dev);
		goto out;
	}

	if (sm5038_get_status(pd_data, PLUG_ATTACH)) {
		pr_info("%s PLUG_ATTACHED +++\n", __func__);
		ret = sm5038_usbpd_notify_attach(pdic_data);
		power_supply_changed(pdic_data->psy_usbpd);
	}

	if (sm5038_get_status(pd_data, PLUG_DETACH)) {
		pr_info("%s PLUG_DETACHED ---\n", __func__);
		if (pdic_data->is_otg_vboost)
			sm5038_usbpd_set_rp_scr_sel(pd_data,
					PLUG_CTRL_RP80);
		sm5038_usbpd_notify_detach(pdic_data);
		power_supply_changed(pdic_data->psy_usbpd);
		goto out;
	}

	mutex_lock(&pdic_data->lpm_mutex);
	sm5038_usbpd_kick_policy_work(dev);
	mutex_unlock(&pdic_data->lpm_mutex);
out:
	mutex_unlock(&pdic_data->_mutex);
	__pm_relax(pdic_data->irq_ws);

	return IRQ_HANDLED;
}

static void sm5038_usbpd_data_init(struct sm5038_phydrv_data *_data)
{
	_data->check_msg_pass = false;
	_data->vconn_source = VCONN_TURN_OFF;
	_data->is_attached = 0;
#if defined(CONFIG_SM5038_WATER_DETECTION_ENABLE)
	_data->is_water_detect = false;
	_data->abnormal_dev_cnt = 0;
#endif
	_data->detach_valid = true;
	_data->is_otg_vboost = false;
	_data->is_usb_therm = 0;
	_data->soft_reset = false;
	_data->is_timer_expired = false;
	_data->reset_done = 0;
	_data->scr_sel = PLUG_CTRL_RP80;
	_data->rp_currentlvl = RP_CURRENT_LEVEL_NONE;
#if defined(CONFIG_SM5038_SHORT_PROTECTION)
	_data->is_cc_abnormal_state = false;
#if defined(CONFIG_SM5038_SUPPORT_SBU)
	_data->is_sbu_abnormal_state = false;
#endif
#endif
	_data->pd_support = 0;
	_data->suspended = false;
}

static int sm5038_usbpd_reg_init(struct sm5038_phydrv_data *_data)
{
	struct i2c_client *i2c = _data->i2c;
	u8 reg_data;

	pr_info("%s", __func__);
	/* Release SNK Only */
	sm5038_usbpd_write_reg(i2c, SM5038_REG_CC_CNTL1, 0x41);
	sm5038_check_cc_state(_data);
#if defined(CONFIG_SM5038_WATER_DETECTION_ENABLE)
	/* Release SBU Sourcing */
	sm5038_usbpd_write_reg(i2c, SM5038_REG_CORR_CNTL5, 0x00);
	sm5038_abnormal_dev_int_on_off(_data, 1);
	/* CC & SBU Water Detection Function Enable */
	sm5038_usbpd_write_reg(i2c, SM5038_REG_CORR_CNTL4, 0x03);
	sm5038_usbpd_write_reg(i2c, SM5038_REG_CORR_OPT4, 0x7B);
	sm5038_usbpd_write_reg(i2c, SM5038_REG_CORR_CNTL9, 0x00);
	/* Water Release CC & SBU Threshold 1.2Mohm */
	sm5038_usbpd_write_reg(i2c, SM5038_REG_CORR_TH6, 0x13);
	/* Water Release Check Time Intaval = 20s */
	sm5038_usbpd_write_reg(i2c, 0x95, 0xFA);
#else
	sm5038_usbpd_write_reg(i2c, SM5038_REG_CORR_CNTL4, 0x00);
#endif
	sm5038_usbpd_read_reg(i2c, SM5038_REG_PD_STATE3, &reg_data);
	if (reg_data & 0x06) /* Reset Wait Policy Engine */
		sm5038_usbpd_write_reg(i2c, SM5038_REG_PD_CNTL4,
				SM5038_REG_CNTL_NOTIFY_RESET_DONE);

	return 0;
}

static int sm5038_usbpd_irq_init(struct sm5038_phydrv_data *_data)
{
	struct i2c_client *i2c = _data->i2c;
	struct device *dev = &i2c->dev;
	int ret = 0;

	if (!_data->irq_gpio) {
		dev_err(dev, "%s No interrupt specified\n", __func__);
		return -ENXIO;
	}

	i2c->irq = gpio_to_irq(_data->irq_gpio);

	ret = gpio_request(_data->irq_gpio, "usbpd_irq");
	if (ret) {
		dev_err(_data->dev, "%s: failed requesting gpio %d\n",
			__func__, _data->irq_gpio);
		return ret;
	}
	gpio_direction_input(_data->irq_gpio);

	if (i2c->irq) {
		ret = request_threaded_irq(i2c->irq, NULL,
			sm5038_pdic_irq_thread,
			(IRQF_TRIGGER_LOW | IRQF_ONESHOT),
			"sm5038-usbpd", _data);
		if (ret < 0) {
			dev_err(dev, "%s failed to request irq(%d)\n",
					__func__, i2c->irq);
			gpio_free(_data->irq_gpio);
			return ret;
		}

		ret = enable_irq_wake(i2c->irq);
		if (ret < 0)
			dev_err(dev, "%s failed to enable wakeup src\n",
					__func__);
	}
	sm5038_set_irq_enable(_data, ENABLED_INT_1, ENABLED_INT_2,
			ENABLED_INT_3, ENABLED_INT_4, ENABLED_INT_5);

	return ret;
}

#if 0
static void sm5038_power_off_water_check(struct sm5038_phydrv_data *_data)
{
	struct i2c_client *i2c = _data->i2c;
	u8 adc_sbu1, adc_sbu2, status2, status3;
	int retry = 0;
	bool is_water = true;

	sm5038_usbpd_read_reg(i2c, SM5038_REG_STATUS2, &status2);
	sm5038_usbpd_read_reg(i2c, SM5038_REG_STATUS3, &status3);
	pr_info("%s, STATUS2 = 0x%x, STATUS3 = 0x%x\n", __func__, status2, status3);

	if (status3 & SM5038_REG_INT_STATUS3_WATER ||
			status2 & SM5038_REG_INT_STATUS2_VBUS_0V)
		return;

	for (retry = 0; retry < 3; retry++) {
		if (retry > 0)
			msleep(100);
		sm5038_corr_sbu_volt_read(_data, &adc_sbu1, &adc_sbu2, SBU_SOURCING_OFF);
		if (!(adc_sbu1 > 0x13 && adc_sbu2 > 0x13)) {
			is_water = false;
			break;
		}
	}

	if (is_water) {
		_data->is_water_detect = true;
		sm5038_process_cc_water_det(_data, WATER_MODE_ON);
		pr_info("%s, TA with water.\n", __func__);
	} else
		pr_info("%s, This case is not water.\n", __func__);
}
#endif

#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
static int sm5038_parse_gpio(struct sm5038_phydrv_data *_data, struct device_node *node)
{

	_data->serial_pinctrl = devm_pinctrl_get(_data->dev);
	if (IS_ERR_OR_NULL(_data->serial_pinctrl)) {
		pr_err("serial_pinctrl is NULL!\n");
		return 0;
	}

	_data->serial_gpio_suspend=
		pinctrl_lookup_state(_data->serial_pinctrl,
                        "sleep");
	if (IS_ERR_OR_NULL(_data->serial_gpio_suspend)) {
		pr_err("No suspend config specified!\n");
		return 0;
	}
	return 0;
}
#endif

static int of_sm5038_pdic_dt(struct device *dev,
			struct sm5038_phydrv_data *_data)
{
	struct device_node *np_usbpd = dev->of_node;
	int ret = 0;

	if (np_usbpd == NULL) {
		dev_err(dev, "%s np NULL\n", __func__);
		ret = -EINVAL;
	} else {
		_data->irq_gpio = of_get_named_gpio(np_usbpd,
							"usbpd,usbpd_int", 0);
		if (_data->irq_gpio < 0) {
			dev_err(dev, "error reading usbpd irq = %d\n",
						_data->irq_gpio);
			_data->irq_gpio = 0;
		}
		pr_info("%s irq_gpio = %d", __func__, _data->irq_gpio);

		_data->vbus_dischg_gpio = of_get_named_gpio(np_usbpd,
							"usbpd,vbus_discharging", 0);
		if (gpio_is_valid(_data->vbus_dischg_gpio))
			pr_info("%s vbus_discharging = %d\n",
						__func__, _data->vbus_dischg_gpio);

		if (of_find_property(np_usbpd, "vconn-en", NULL))
			_data->vconn_en = true;
		else
			_data->vconn_en = false;
	}

	#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	ret = sm5038_parse_gpio(_data, np_usbpd);
	if (ret < 0)
		return ret;
#endif
	return ret;
}

#if defined(CONFIG_SM5038_DEBUG_LOG_ENABLE)
static void sm5038_usbpd_debug_reg_log(struct work_struct *work)
{
	struct sm5038_phydrv_data *pdic_data =
		container_of(work, struct sm5038_phydrv_data,
				debug_work.work);
	struct i2c_client *i2c = pdic_data->i2c;
	u8 data[20] = {0, };

	sm5038_usbpd_read_reg(i2c, SM5038_REG_CONTROL, &data[0]);
	sm5038_usbpd_read_reg(i2c, SM5038_REG_CORR_CNTL1, &data[1]);
	sm5038_usbpd_read_reg(i2c, SM5038_REG_CORR_CNTL4, &data[2]);
	sm5038_usbpd_read_reg(i2c, SM5038_REG_CORR_CNTL5, &data[3]);
	sm5038_usbpd_read_reg(i2c, SM5038_REG_CC_STATUS, &data[4]);
	sm5038_usbpd_read_reg(i2c, SM5038_REG_CC_CNTL1, &data[5]);
	sm5038_usbpd_read_reg(i2c, SM5038_REG_CC_CNTL2, &data[6]);
	sm5038_usbpd_read_reg(i2c, SM5038_REG_CC_CNTL3, &data[7]);
	sm5038_usbpd_read_reg(i2c, SM5038_REG_CC_CNTL7, &data[8]);
	sm5038_usbpd_read_reg(i2c, SM5038_REG_PD_CNTL1, &data[9]);
	sm5038_usbpd_read_reg(i2c, SM5038_REG_PD_CNTL4, &data[10]);
	sm5038_usbpd_read_reg(i2c, SM5038_REG_RX_BUF_ST, &data[11]);
	sm5038_usbpd_read_reg(i2c, SM5038_REG_PROBE0, &data[12]);
	sm5038_usbpd_read_reg(i2c, SM5038_REG_PROBE2, &data[13]);
	sm5038_usbpd_read_reg(i2c, SM5038_REG_PROBE3, &data[14]);
	sm5038_usbpd_read_reg(i2c, SM5038_REG_PD_STATE0, &data[15]);
	sm5038_usbpd_read_reg(i2c, SM5038_REG_PD_STATE2, &data[16]);
	sm5038_usbpd_read_reg(i2c, SM5038_REG_PD_STATE3, &data[17]);
	sm5038_usbpd_read_reg(i2c, SM5038_REG_PD_STATE4, &data[18]);
	sm5038_usbpd_read_reg(i2c, SM5038_REG_PD_STATE5, &data[19]);

	pr_info("%s CTRL:0x%02x CR_CT[1: 0x%02x 4:0x%02x 5:0x%02x] CC_ST:0x%02x CC_CT[1:0x%02x 2:0x%02x 3:0x%02x 7:0x%02x] PD_CT[1:0x%02x 4:0x%02x] RX_BUF_ST:0x%02x PROBE[0:0x%02x 2:0x%02x 3:0x%02x] PD_ST[0:0x%02x 2:0x%02x 3:0x%02x 4:0x%02x 5:0x%02x]\n",
			__func__, data[0], data[1], data[2], data[3], data[4],
			data[5], data[6], data[7], data[8], data[9], data[10],
			data[11], data[12], data[13], data[14], data[15],
			data[16], data[17], data[18], data[19]);

	if (!pdic_data->suspended)
		schedule_delayed_work(&pdic_data->debug_work,
				msecs_to_jiffies(60000));
}
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0)
const struct typec_operations sm5038_typec_ops = {
	.dr_set = sm5038_dr_set,
	.pr_set = sm5038_pr_set,
	.port_type_set = sm5038_port_type_set,
};
#endif

enum {
	TYPEC_ORIENTATION = 0,
	TYPEC_REAL_TYPE = 1,
	MOISTURE_DETECTED = 2,
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	TYPEC_MODE = 3,
#endif
};

ssize_t sm5038_usbpd_show_attrs(struct device *dev, struct device_attribute *attr, char *buf);

#define SM5038_TYPEC_ATTR(_name)				\
{												\
	.attr = {.name = #_name, .mode = 0444},		\
	.show = sm5038_usbpd_show_attrs,			\
}

static const char * const SM5038_POWER_SUPPLY_TYPE_TEXT[] = {
	[POWER_SUPPLY_TYPE_UNKNOWN]		= "Unknown",
	[POWER_SUPPLY_TYPE_BATTERY]		= "Battery",
	[POWER_SUPPLY_TYPE_UPS]			= "UPS",
	[POWER_SUPPLY_TYPE_MAINS]		= "Mains",
	[POWER_SUPPLY_TYPE_USB]			= "USB",
	[POWER_SUPPLY_TYPE_USB_DCP]		= "USB_DCP",
	[POWER_SUPPLY_TYPE_USB_CDP]		= "USB_CDP",
	[POWER_SUPPLY_TYPE_USB_ACA]		= "USB_ACA",
	[POWER_SUPPLY_TYPE_USB_TYPE_C]		= "USB_C",
	[POWER_SUPPLY_TYPE_USB_PD]		= "USB_PD",
	[POWER_SUPPLY_TYPE_USB_PD_DRP]		= "USB_PD_DRP",
	[POWER_SUPPLY_TYPE_APPLE_BRICK_ID]	= "BrickID",
	[POWER_SUPPLY_TYPE_WIRELESS]		= "Wireless",
};

static struct device_attribute sm5038_typec_attrs[] = {
	SM5038_TYPEC_ATTR(typec_orientation),
	SM5038_TYPEC_ATTR(typec_real_type),
	SM5038_TYPEC_ATTR(moisture_detected),
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	SM5038_TYPEC_ATTR(typec_mode),
#endif
};

static int sm5038_usbpd_create_attrs(struct device *dev)
{
	unsigned long i;
	int rc;

	for (i = 0; i < ARRAY_SIZE(sm5038_typec_attrs); i++) {
		rc = device_create_file(dev, &sm5038_typec_attrs[i]);
		if (rc)
			goto create_attrs_failed;
	}
	return rc;

create_attrs_failed:
	pr_err("%s: failed (%d)\n", __func__, rc);
	while (i--)
		device_remove_file(dev, &sm5038_typec_attrs[i]);
	return rc;
}

ssize_t sm5038_usbpd_show_attrs(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct power_supply *psy = dev_get_drvdata(dev);
	struct sm5038_phydrv_data *pdic_data = power_supply_get_drvdata(psy);
	const ptrdiff_t offset = attr - sm5038_typec_attrs;
	int i = 0;
	int value = 0;

	if (pdic_data == NULL) {
		pr_err("%s: pdic_data is NULL \n", __func__);
		return -ENODEV;
	}

	switch (offset) {
	case TYPEC_ORIENTATION:
		value = sm5038_get_plug_orientation(pdic_data);
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", value);
		pr_info("%s TYPEC_ORIENTATION get is %d \n", __func__, value);
		break;
	case TYPEC_REAL_TYPE:
		value = pdic_data->chg_cable_type;
		i += scnprintf(buf + i, PAGE_SIZE - i, "%s\n", SM5038_POWER_SUPPLY_TYPE_TEXT[value]);
		pr_info("%s TYPEC_REAL_TYPE get is %d \n", __func__, value);
		break;
#if defined(CONFIG_SM5038_WATER_DETECTION_ENABLE)
	case MOISTURE_DETECTED:
		value = pdic_data->is_water_detect;
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", value);
		pr_info("%s MOISTURE_DETECTED get is %d \n", __func__, value);
		break;
#endif
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	case TYPEC_MODE:
		value = pdic_data->rp_currentlvl;
		if (value == RP_CURRENT_LEVEL_DEFAULT)
			i += scnprintf(buf + i, PAGE_SIZE - i, "%s\n",
					"Source attached (default current)");
		else if (value == RP_CURRENT_LEVEL2)
			i += scnprintf(buf + i, PAGE_SIZE - i, "%s\n",
					"Source attached (medium current)");
		else if (value == RP_CURRENT_LEVEL3)
			i += scnprintf(buf + i, PAGE_SIZE - i, "%s\n",
					"Source attached (high current)");
		else
			i += scnprintf(buf + i, PAGE_SIZE - i, "%s\n",
							"No Source attached");
		pr_debug("%s TYPEC_MODE get is %d \n", __func__, value);
		break;
#endif
	default:
		return -EINVAL;
	}
	return i;
}

#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
#if defined(CONFIG_DEBUG_FS)
static int reg_addr_get(void *data, u64 *val)
{
	struct sm5038_phydrv_data *pdic_data = data;

	*val = (u64)pdic_data->dfs_reg_addr;

	return 0;
}
static int reg_addr_set(void *data, u64 val)
{
	struct sm5038_phydrv_data *pdic_data = data;

	pdic_data->dfs_reg_addr = (u8)val;

	return 0;
}
DEFINE_DEBUGFS_ATTRIBUTE(reg_addr_ops, reg_addr_get, reg_addr_set,
								"0x%02llx\n");

static int reg_data_get(void *data, u64 *val)
{
	struct sm5038_phydrv_data *pdic_data = data;
	u8 reg_data;

	sm5038_usbpd_read_reg(pdic_data->i2c, pdic_data->dfs_reg_addr,
								&reg_data);
	*val = (u64)reg_data;

	return 0;
}
static int reg_data_set(void *data, u64 val)
{
	struct sm5038_phydrv_data *pdic_data = data;
	u8 reg_data;

	reg_data = (u8)val;
	sm5038_usbpd_write_reg(pdic_data->i2c, pdic_data->dfs_reg_addr,
								reg_data);

	return 0;
}
DEFINE_DEBUGFS_ATTRIBUTE(reg_data_ops, reg_data_get, reg_data_set,
								"0x%02llx\n");

static void somc_sm5038_create_debugfs(struct sm5038_phydrv_data *pdic_data)
{
	struct dentry *file;
	struct dentry *dir;

	dir = debugfs_create_dir("sm5038_typec", NULL);
	if (IS_ERR_OR_NULL(dir)) {
		pr_err("Couldn't create sm5038_typec debugfs rc=%ld\n",
								(long)dir);
		return;
	}

	file = debugfs_create_file("reg_addr", S_IFREG | 0644,
						dir, pdic_data, &reg_addr_ops);
	if (IS_ERR_OR_NULL(file))
		pr_err("Couldn't create reg_addr file rc=%ld\n", (long)file);

	file = debugfs_create_file("reg_data", S_IFREG | 0644,
						dir, pdic_data, &reg_data_ops);
	if (IS_ERR_OR_NULL(file))
		pr_err("Couldn't create reg_data file rc=%ld\n", (long)file);
}
#else
static void somc_sm5038_create_debugfs(struct sm5038_phydrv_data *pdic_data)
{
}
#endif
#endif

static int sm5038_usbpd_probe(struct i2c_client *i2c,
				const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = to_i2c_adapter(i2c->dev.parent);
	struct sm5038_phydrv_data *pdic_data;
	struct device *dev = &i2c->dev;
	struct power_supply *charger_otg;

	int ret = 0;
	u8 data = 0;

	pr_info("%s start\n", __func__);

	charger_otg = power_supply_get_by_name("sm5038-otg");

	if (!charger_otg) {
		pr_err("%s: Fail to get psy battery\n", __func__);
		return -EPROBE_DEFER;
	}

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA)) {
		dev_err(dev, "%s: i2c functionality check error\n", __func__);
		ret = -EIO;
		goto err_return;
	}

	pdic_data = kzalloc(sizeof(struct sm5038_phydrv_data), GFP_KERNEL);
	if (!pdic_data) {
		dev_err(dev, "%s: failed to allocate driver data\n", __func__);
		ret = -ENOMEM;
		goto err_return;
	}

	/* save platfom data for gpio control functions */
	pdic_data->dev = &i2c->dev;
	pdic_data->i2c = i2c;

	if (pdic_data->i2c == NULL) {
		pr_err("%s: i2c NULL\n", __func__);
		goto fail_init_irq;
	}

	ret = sm5038_usbpd_read_reg(i2c, SM5038_REG_VENDOR_ID, &data);
	if (ret < 0) {
		ret = -ENODEV;
		goto fail_init_irq;
	} else
		pr_info("%s : vendor_id=0x%x\n", __func__, data);

	ret = of_sm5038_pdic_dt(&i2c->dev, pdic_data);
	if (ret < 0)
		dev_err(dev, "%s: not found dt!\n", __func__);

	init_waitqueue_head(&pdic_data->suspend_wait);

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0)
	if (!(pdic_data->irq_ws)) {
		pdic_data->irq_ws = wakeup_source_create("irq_wake"); // 4.19 Q
		if (pdic_data->irq_ws)
			wakeup_source_add(pdic_data->irq_ws);
	}
#else
	pdic_data->irq_ws = wakeup_source_register(NULL, "irq_wake"); // 5.4 R
#endif
	ret = sm5038_usbpd_init(dev, pdic_data);
	if (ret < 0) {
		dev_err(dev, "failed on usbpd_init\n");
		goto err_return;
	}

	static_pdic_data = pdic_data;
	sm5038_usbpd_set_ops(dev, &sm5038_ops);

	mutex_init(&pdic_data->_mutex);
	mutex_init(&pdic_data->lpm_mutex);

	sm5038_usbpd_reg_init(pdic_data);
	sm5038_usbpd_data_init(pdic_data);

	INIT_DELAYED_WORK(&pdic_data->vbus_dischg_work,
			sm5038_vbus_dischg_work);
#if defined(CONFIG_SM5038_DEBUG_LOG_ENABLE)
	INIT_DELAYED_WORK(&pdic_data->debug_work,
			sm5038_usbpd_debug_reg_log);
	schedule_delayed_work(&pdic_data->debug_work, msecs_to_jiffies(10000));
#endif

	ret = sm5038_usbpd_psy_init(pdic_data, &i2c->dev);
	if (ret < 0) {
		pr_err("faled to register the usbpd psy.\n");
	}

#if 0
	sm5038_power_off_water_check(pdic_data);
#endif

	pdic_data->extcon = devm_extcon_dev_allocate(&i2c->dev, usbpd_extcon_cable);
	if (IS_ERR(pdic_data->extcon)) {
		dev_err(dev, "failed to allocate extcon device\n");
		ret = PTR_ERR(pdic_data->extcon);
	}

	ret = devm_extcon_dev_register(&i2c->dev, pdic_data->extcon);
	if (ret)
		dev_err(dev, "failed to register extcon device\n");

	/* Support reporting polarity and speed via properties */
	extcon_set_property_capability(pdic_data->extcon, EXTCON_USB,
			EXTCON_PROP_USB_TYPEC_POLARITY);
	extcon_set_property_capability(pdic_data->extcon, EXTCON_USB,
			EXTCON_PROP_USB_SS);
	extcon_set_property_capability(pdic_data->extcon, EXTCON_USB_HOST,
			EXTCON_PROP_USB_TYPEC_POLARITY);
	extcon_set_property_capability(pdic_data->extcon, EXTCON_USB_HOST,
			EXTCON_PROP_USB_SS);

	pdic_data->typec_cap.revision = USB_TYPEC_REV_1_2;
	pdic_data->typec_cap.pd_revision = 0x200;
	pdic_data->typec_cap.prefer_role = TYPEC_NO_PREFERRED_ROLE;
	pdic_data->typec_cap.type = TYPEC_PORT_DRP;
	pdic_data->typec_cap.data = TYPEC_PORT_DRD;
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0)
	pdic_data->typec_cap.pr_set = sm5038_pr_set;
	pdic_data->typec_cap.dr_set = sm5038_dr_set;
	pdic_data->typec_cap.port_type_set = sm5038_port_type_set;
#else
	pdic_data->typec_cap.ops = &sm5038_typec_ops;
	pdic_data->typec_cap.driver_data = pdic_data;
#endif
	pdic_data->typec_power_role = TYPEC_SINK;
	pdic_data->typec_data_role = TYPEC_DEVICE;
	pdic_data->typec_try_state_change = ROLE_SWAP_NONE;
	pdic_data->port = typec_register_port(pdic_data->dev,
		&pdic_data->typec_cap);
	if (IS_ERR(pdic_data->port))
		pr_err("%s : unable to register typec_register_port\n", __func__);

	init_completion(&pdic_data->typec_reverse_completion);
	INIT_DELAYED_WORK(&pdic_data->role_swap_work, sm5038_role_swap_check);

	ret = sm5038_usbpd_irq_init(pdic_data);
	if (ret) {
		dev_err(dev, "%s: failed to init irq(%d)\n", __func__, ret);
		goto fail_init_irq;
	}

	ret = sm5038_usbpd_create_attrs(&pdic_data->psy_usbpd->dev);
	if (ret) {
		pr_err("sm5038-usbpd: %s : Failed to create_attrs\n", __func__);
		goto fail_init_irq;
	}

	device_init_wakeup(dev, true);
	/* initial cable detection */
	sm5038_pdic_irq_thread(-1, pdic_data);

#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	somc_sm5038_create_debugfs(pdic_data);

#endif
	pr_info("%s : sm5038 usbpd driver uploaded!\n", __func__);
	return 0;
fail_init_irq:
	if (i2c->irq)
		free_irq(i2c->irq, pdic_data);
	kfree(pdic_data);
err_return:
	return ret;
}

#if defined CONFIG_PM
static int sm5038_usbpd_suspend(struct device *dev)
{
	struct sm5038_usbpd_data *_data = dev_get_drvdata(dev);
	struct sm5038_phydrv_data *pdic_data = _data->phy_driver_data;

	pdic_data->suspended = true;

	if (device_may_wakeup(dev))
		enable_irq_wake(pdic_data->i2c->irq);
#if 0
	disable_irq(pdic_data->i2c->irq);
#endif
#if defined(CONFIG_SM5038_WATER_DETECTION_ENABLE)
	if (pdic_data->is_timer_expired) {
		pdic_data->abnormal_dev_cnt = 0;
		sm5038_usbpd_write_reg(pdic_data->i2c, SM5038_REG_GEN_TMR_L, 0x00);
		sm5038_usbpd_write_reg(pdic_data->i2c, SM5038_REG_GEN_TMR_U, 0x00);
	}
#endif
#if defined(CONFIG_SM5038_DEBUG_LOG_ENABLE)
	cancel_delayed_work_sync(&pdic_data->debug_work);
#endif
	return 0;
}

static int sm5038_usbpd_resume(struct device *dev)
{
	struct sm5038_usbpd_data *_data = dev_get_drvdata(dev);
	struct sm5038_phydrv_data *pdic_data = _data->phy_driver_data;

	pdic_data->suspended = false;
	wake_up(&pdic_data->suspend_wait);

	if (device_may_wakeup(dev))
		disable_irq_wake(pdic_data->i2c->irq);

#if 0
	enable_irq(pdic_data->i2c->irq);
#endif
#if defined(CONFIG_SM5038_WATER_DETECTION_ENABLE)
	if (pdic_data->is_timer_expired) {
		sm5038_abnormal_dev_int_on_off(pdic_data, 1);
		pdic_data->is_timer_expired = false;
	}
#endif
#if defined(CONFIG_SM5038_DEBUG_LOG_ENABLE)
	schedule_delayed_work(&pdic_data->debug_work, msecs_to_jiffies(1500));
#endif
	return 0;
}
#endif

static int sm5038_usbpd_remove(struct i2c_client *i2c)
{
	struct sm5038_usbpd_data *pd_data = dev_get_drvdata(&i2c->dev);
	struct sm5038_phydrv_data *_data = pd_data->phy_driver_data;

	if (_data) {
#if defined(CONFIG_SM5038_DEBUG_LOG_ENABLE)
		cancel_delayed_work_sync(&_data->debug_work);
#endif
		typec_unregister_port(_data->port);
		disable_irq_wake(_data->i2c->irq);
		free_irq(_data->i2c->irq, _data);
		gpio_free(_data->irq_gpio);
		mutex_destroy(&_data->_mutex);
		sm5038_usbpd_set_vbus_dischg_gpio(_data, 0);
		sm5038_usbpd_device_unregister(pd_data);
		sm5038_usbpd_class_unregister(pd_data);
		kfree(_data);
	}
	return 0;
}

static const struct i2c_device_id sm5038_usbpd_i2c_id[] = {
	{ USBPD_DEV_NAME, 0 },
	{}
};
MODULE_DEVICE_TABLE(i2c, sm5038_usbpd_i2c_id);

static const struct of_device_id usbpd_i2c_dt_ids[] = {
	{ .compatible = "sm5038-usbpd" },
	{ }
};

static void sm5038_usbpd_shutdown(struct i2c_client *i2c)
{
	struct sm5038_usbpd_data *pd_data = dev_get_drvdata(&i2c->dev);
	struct sm5038_phydrv_data *_data = pd_data->phy_driver_data;

	if (_data->is_usb_therm)
		sm5038_cc_control_command(0);
#if defined(CONFIG_SM5038_DEBUG_LOG_ENABLE)
	cancel_delayed_work_sync(&_data->debug_work);
#endif
	sm5038_usbpd_set_vbus_dischg_gpio(_data, 0);
	pr_info("%s!\n", __func__);
}

static usbpd_phy_ops_type sm5038_ops = {
	.tx_msg			= sm5038_tx_msg,
	.rx_msg			= sm5038_rx_msg,
	.hard_reset		= sm5038_hard_reset,
	.set_power_role		= sm5038_set_power_role,
	.get_power_role		= sm5038_get_power_role,
	.set_data_role		= sm5038_set_data_role,
	.get_data_role		= sm5038_get_data_role,
	.set_vconn_source	= sm5038_set_vconn_source,
	.get_vconn_source	= sm5038_get_vconn_source,
	.set_check_msg_pass	= sm5038_set_check_msg_pass,
	.get_status		= sm5038_get_status,
	.poll_status		= sm5038_poll_status,
	.driver_reset		= sm5038_driver_reset,
#if defined(CONFIG_SM5038_SHORT_PROTECTION)
	.get_short_state	= sm5038_get_short_state,
#endif
};

#if defined CONFIG_PM
const struct dev_pm_ops sm5038_usbpd_pm = {
	.suspend = sm5038_usbpd_suspend,
	.resume = sm5038_usbpd_resume,
};
#endif

static struct i2c_driver sm5038_usbpd_driver = {
	.driver		= {
		.name	= USBPD_DEV_NAME,
		.of_match_table	= usbpd_i2c_dt_ids,
#if defined CONFIG_PM
		.pm	= &sm5038_usbpd_pm,
#endif /* CONFIG_PM */
	},
	.probe		= sm5038_usbpd_probe,
	.remove		= sm5038_usbpd_remove,
	.shutdown	= sm5038_usbpd_shutdown,
	.id_table	= sm5038_usbpd_i2c_id,
};

static int __init sm5038_usbpd_typec_init(void)
{
	return i2c_add_driver(&sm5038_usbpd_driver);
}
late_initcall(sm5038_usbpd_typec_init);

static void __exit sm5038_usbpd_typec_exit(void)
{
	i2c_del_driver(&sm5038_usbpd_driver);
}
module_exit(sm5038_usbpd_typec_exit);

MODULE_DESCRIPTION("SM5038 USB TYPE-C/PD driver");
MODULE_LICENSE("GPL");
