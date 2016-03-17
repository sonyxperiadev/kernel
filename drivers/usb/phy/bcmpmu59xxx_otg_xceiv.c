/*****************************************************************************
*  Copyright 2001 - 2011 Broadcom Corporation.  All rights reserved.
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2, available at
*  http://www.gnu.org/licenses/old-license/gpl-2.0.html (the "GPL").
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
*****************************************************************************/


#include <linux/module.h>
#include <linux/init.h>
#include <linux/stat.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/usb/otg.h>
#include <linux/usb.h>
#include <linux/usb/gadget.h>
#include <linux/usb/hcd.h>
#include <linux/err.h>
#include <linux/notifier.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/pm_runtime.h>
#include <linux/mfd/bcmpmu59xxx.h>
#include <linux/io.h>
#include <mach/io_map.h>
#include <linux/regulator/consumer.h>
#include <linux/usb/bcm_hsotgctrl.h>
#include "bcmpmu59xxx_otg_xceiv.h"
#include "bcm_otg_adp.h"

#define OTGCTRL1_VBUS_ON 0xDC
#define OTGCTRL1_VBUS_OFF 0xD8

#define HOST_TO_PERIPHERAL_DELAY_MS 1000
#define PERIPHERAL_TO_HOST_DELAY_MS 100
#define USBLDO_RAMP_UP_DELAY_IN_MS 2
#define USB_OTG_SUSPEND_CURRENT 2

static int bcmpmu_otg_xceiv_set_vbus(struct usb_otg *otg, bool enabled)
{
	struct bcmpmu_otg_xceiv_data *xceiv_data =
		dev_get_drvdata(otg->phy->dev);
	int stat;

	/* The order of these operations has temporarily been
	 * swapped due to overcurrent issue caused by slow I2C
	 * operations. I2C operations take >200ms to complete */
	bcm_hsotgctrl_phy_set_vbus_stat(enabled);

	if (enabled && bcmpmu_otg_xceiv_check_id_gnd(xceiv_data)) {
		dev_info(xceiv_data->dev, "Turning on VBUS\n");
		xceiv_data->vbus_enabled = true;
		stat =
		    bcmpmu_usb_set(xceiv_data->bcmpmu,
						BCMPMU_USB_CTRL_VBUS_ON_OFF, 1);
	} else {
		dev_info(xceiv_data->dev, "Turning off VBUS\n");
		xceiv_data->vbus_enabled = false;
		stat =
		    bcmpmu_usb_set(xceiv_data->bcmpmu,
						BCMPMU_USB_CTRL_VBUS_ON_OFF, 0);
	}

	if (stat < 0)
		dev_warn(xceiv_data->dev, "Failed to set VBUS\n");

	return stat;
}

bool bcmpmu_otg_xceiv_check_id_gnd(struct bcmpmu_otg_xceiv_data
				   *xceiv_data)
{
	unsigned int data = 0;
	bool id_gnd = false;

	bcmpmu_usb_get(xceiv_data->bcmpmu, BCMPMU_USB_CTRL_GET_ID_VALUE, &data);
	id_gnd = (data == PMU_USB_ID_GROUND);

	return id_gnd;
}

bool bcmpmu_otg_xceiv_check_id_rid_a(struct bcmpmu_otg_xceiv_data
				  *xceiv_data)
{
	unsigned int data = 0;
	bool id_rid_a = false;

	bcmpmu_usb_get(xceiv_data->bcmpmu, BCMPMU_USB_CTRL_GET_ID_VALUE, &data);
	id_rid_a = (data == PMU_USB_ID_RID_A);

	return id_rid_a;
}

bool bcmpmu_otg_xceiv_check_id_rid_b(struct bcmpmu_otg_xceiv_data
				  *xceiv_data)
{
	unsigned int data = 0;
	bool id_rid_b = false;

	bcmpmu_usb_get(xceiv_data->bcmpmu, BCMPMU_USB_CTRL_GET_ID_VALUE, &data);
	id_rid_b = (data == PMU_USB_ID_RID_B);

	return id_rid_b;
}


bool bcmpmu_otg_xceiv_check_id_rid_c(struct bcmpmu_otg_xceiv_data
				  *xceiv_data)
{
	unsigned int data = 0;
	bool id_rid_c = false;

	bcmpmu_usb_get(xceiv_data->bcmpmu, BCMPMU_USB_CTRL_GET_ID_VALUE, &data);
	id_rid_c = (data == PMU_USB_ID_RID_C);

	return id_rid_c;
}

bool bcmpmu_otg_xceiv_check_id_rid_float(struct bcmpmu_otg_xceiv_data
				  *xceiv_data)
{
	unsigned int data = 0;
	bool id = false;

	bcmpmu_usb_get(xceiv_data->bcmpmu, BCMPMU_USB_CTRL_GET_ID_VALUE, &data);
	id = (data == PMU_USB_ID_FLOAT);

	return id;
}

static void bcmpmu_otg_xceiv_shutdown(struct usb_phy *phy)
{
	struct bcmpmu_otg_xceiv_data *xceiv_data = dev_get_drvdata(phy->dev);

	if (xceiv_data) {
		/* If the Phy is initialized, then De-initialize. Otherwise
		 * calling bcm_hsotgctrl_phy_deinit() will cause clocks
		 * inbalance.
		 * If we do not have this check, if we Plug IN and Plug OUT
		 * the USB cable continuously, end up having the clock
		 * inbalance.
		 */
		if (xceiv_data->otg_xceiver.phy.state != OTG_STATE_UNDEFINED)
			bcm_hsotgctrl_phy_deinit();

		xceiv_data->otg_xceiver.phy.state = OTG_STATE_UNDEFINED;
		if (!xceiv_data->otg_enabled) {
			if (xceiv_data->bcm_hsotg_regulator &&
				    xceiv_data->regulator_enabled) {
				/* This should have no effect for most of
				 * our platforms as "always on" parameter is set
				 */
				regulator_disable(xceiv_data->
					    bcm_hsotg_regulator);
				xceiv_data->regulator_enabled = false;
			}
		}
	}

}

static void bcmpmu_otg_xceiv_set_def_state(
	struct bcmpmu_otg_xceiv_data *xceiv_data, bool default_host)
{

	if (xceiv_data) {
		if (default_host)
			xceiv_data->otg_xceiver.phy.state =
				    OTG_STATE_A_IDLE;
		else
			xceiv_data->otg_xceiver.phy.state =
				    OTG_STATE_B_IDLE;
	}

}

static int bcmpmu_otg_xceiv_start(struct usb_phy *phy)
{
	struct bcmpmu_otg_xceiv_data *xceiv_data = dev_get_drvdata(phy->dev);
	int ret = 0;
	bool id_default_host = false;

	if (!xceiv_data)
		return -EINVAL;

	if (!xceiv_data->otg_enabled) {
		if (xceiv_data->bcm_hsotg_regulator &&
			!xceiv_data->regulator_enabled) {
			ret = regulator_enable(xceiv_data->
					bcm_hsotg_regulator);
			if (ret) {
				dev_warn(xceiv_data->dev,
					"Regulator enable failed\n");
				return ret;
			}
			/* Give 2ms to ramp up USBLDO */
			mdelay(USBLDO_RAMP_UP_DELAY_IN_MS);
			xceiv_data->regulator_enabled = true;
		}
	}

	id_default_host = bcmpmu_otg_xceiv_check_id_gnd(xceiv_data) ||
		bcmpmu_otg_xceiv_check_id_rid_a(xceiv_data);
	/* Initialize OTG PHY, if it is not */
	if (xceiv_data->otg_xceiver.phy.state == OTG_STATE_UNDEFINED)
		bcm_hsotgctrl_phy_init(!id_default_host);

	bcmpmu_otg_xceiv_set_def_state(xceiv_data, id_default_host);

	return 0;
}

static int bcmpmu_otg_xceiv_set_delayed_adp(struct usb_otg *otg)
{
	struct bcmpmu_otg_xceiv_data *xceiv_data =
		dev_get_drvdata(otg->phy->dev);

	if (!xceiv_data)
		return -EINVAL;

	xceiv_data->otg_xceiver.otg_vbus_off = true;

	return 0;
}

static int bcmpmu_otg_xceiv_set_srp_reqd_handler(struct usb_otg *otg)
{
	struct bcmpmu_otg_xceiv_data *xceiv_data =
		dev_get_drvdata(otg->phy->dev);

	if (!xceiv_data)
		return -EINVAL;

	xceiv_data->otg_xceiver.otg_srp_reqd = true;

	return 0;
}

static int bcmpmu_otg_xceiv_set_otg_enable(struct usb_otg *otg,
					   bool enable)
{
	struct bcmpmu_otg_xceiv_data *xceiv_data =
		dev_get_drvdata(otg->phy->dev);

	if (!xceiv_data)
		return -EINVAL;

	xceiv_data->otg_enabled = enable;
	return 0;
}
static int bcmpmu_otg_xceiv_connect(struct usb_phy *phy,
		enum usb_device_speed speed)
{
	struct bcmpmu_otg_xceiv_data *xceiv_data =
		dev_get_drvdata(phy->dev);

	if (!xceiv_data)
		return -EINVAL;

	dev_dbg(phy->dev, "%s speed device has connected\n",
		(speed == USB_SPEED_HIGH) ? "high" : "non-high");

	if (speed == USB_SPEED_HIGH){
		bcm_hsotgctrl_phy_set_non_driving(0);
	}

	return 0;
}

static int bcmpmu_otg_xceiv_disconnect(struct usb_phy *phy,
		enum usb_device_speed speed)
{
	struct bcmpmu_otg_xceiv_data *xceiv_data =
		dev_get_drvdata(phy->dev);

	if (!xceiv_data)
		return -EINVAL;

	dev_dbg(phy->dev, "%s speed device has disconnected\n",
		(speed == USB_SPEED_HIGH) ? "high" : "non-high");

	if (speed == USB_SPEED_HIGH){
		bcm_hsotgctrl_phy_set_non_driving(1);
	}

	return 0;
}



static void bcmpmu_otg_xceiv_suspend_core(struct usb_phy *phy)
{
	atomic_notifier_call_chain(&phy->notifier,
			USB_EVENT_SUSPEND_CORE, NULL);
}

static void bcmpmu_otg_xceiv_wakeup_core(struct usb_phy *phy)
{
	atomic_notifier_call_chain(&phy->notifier,
			USB_EVENT_WAKEUP_CORE, NULL);
}

static int bcmpmu_otg_xceiv_set_suspend(struct usb_phy *phy,
					int suspend)
{
	struct bcmpmu_otg_xceiv_data *xceiv_data =
		dev_get_drvdata(phy->dev);

	if (!xceiv_data)
		return -EINVAL;

	if (!xceiv_data->otg_enabled && suspend){
		bcmpmu_otg_xceiv_suspend_core(phy);
		bcm_hsotgctrl_handle_bus_suspend();
	}

	return 0;
}

static void bcmpmu_otg_xceiv_select_host_mode(struct bcmpmu_otg_xceiv_data
					      *xceiv_data, bool enable)
{
	if (enable) {
		dev_info(xceiv_data->dev, "Switching to Host\n");
		xceiv_data->host = true;
		bcm_hsotgctrl_set_phy_off(false);
		msleep(PERIPHERAL_TO_HOST_DELAY_MS);
		bcm_hsotgctrl_phy_set_id_stat(false);
	} else {
		dev_info(xceiv_data->dev, "Switching to Peripheral\n");
		bcm_hsotgctrl_phy_set_id_stat(true);
		if (xceiv_data->host) {
			xceiv_data->host = false;
			msleep(HOST_TO_PERIPHERAL_DELAY_MS);
		}
	}
}

static int bcmpmu_otg_xceiv_vbus_notif_handler(struct notifier_block *nb,
					       unsigned long value, void *data)
{
	struct bcmpmu_otg_xceiv_data *xceiv_data;
	bool vbus_status = 0;

	switch (value) {
	case PMU_ACCY_EVT_OUT_VBUS_VALID:
		xceiv_data = container_of(nb, struct bcmpmu_otg_xceiv_data,
				bcm_otg_vbus_validity_notifier);
		break;
	case PMU_ACCY_EVT_OUT_SESSION_INVALID:
		xceiv_data = container_of(nb, struct bcmpmu_otg_xceiv_data,
				bcm_otg_session_invalid_notifier);
		break;
	default:
		BUG_ON(1);
	}

	if (!xceiv_data)
		return -EINVAL;

	bcmpmu_usb_get(xceiv_data->bcmpmu, BCMPMU_USB_CTRL_GET_VBUS_STATUS,
		       &vbus_status);

	queue_work(xceiv_data->bcm_otg_work_queue,
		   vbus_status ? &xceiv_data->
		   bcm_otg_vbus_valid_work : &xceiv_data->
		   bcm_otg_vbus_a_invalid_work);

	return 0;
}

static int bcmpmu_otg_xceiv_chg_detection_notif_handler(struct notifier_block
							*nb,
							unsigned long value,
							void *data)
{
	struct bcmpmu_otg_xceiv_data *xceiv_data =
	    container_of(nb, struct bcmpmu_otg_xceiv_data,
			 bcm_otg_chg_detection_notifier);
	u32 usb_charger_type;

	if (!xceiv_data || !data)
		return -EINVAL;

	usb_charger_type = *(u32 *)data;

	dev_info(xceiv_data->dev, "data %x\n", usb_charger_type);
	switch (usb_charger_type) {
	case PMU_CHRGR_TYPE_SDP:
	case PMU_CHRGR_TYPE_CDP:
	case PMU_CHRGR_TYPE_ACA:
		queue_work(xceiv_data->bcm_otg_work_queue,
			   &xceiv_data->bcm_otg_chg_detect_work);
		break;
	default:
		break;
	}

	return 0;

}
static int bcmpmu_otg_xceiv_state_handler(struct notifier_block
					*nb, unsigned long value,
					void *data)
{
	struct bcmpmu_otg_xceiv_data *xceiv_data;
	bool xceiv_state;
	static u8 pmu_xceiv_start;

	xceiv_data = container_of(nb, struct bcmpmu_otg_xceiv_data,
			 bcm_otg_xceive_state_notifier);

	if (!xceiv_data || !data)
		return -EINVAL;

	xceiv_state = *(u32 *)data;

	dev_info(xceiv_data->dev, "----req xceiv_state %x\n", xceiv_state);

	if (xceiv_state) {
		if (xceiv_data->otg_xceiver.phy.state ==
			OTG_STATE_UNDEFINED) {
			bcmpmu_otg_xceiv_start
				(&xceiv_data->otg_xceiver.phy);
			pmu_xceiv_start = 1;
		}
	} else {
		if (pmu_xceiv_start &&
			(xceiv_data->otg_xceiver.phy.state !=
				OTG_STATE_UNDEFINED)) {
			bcmpmu_otg_xceiv_shutdown
				(&xceiv_data->otg_xceiver.phy);
			pmu_xceiv_start = 0;
		}
	}
	return 0;
}
static int bcmpmu_otg_xceiv_id_chg_notif_handler(struct notifier_block *nb,
						 unsigned long value,
						 void *data)
{
	struct bcmpmu_otg_xceiv_data *xceiv_data =
	    container_of(nb, struct bcmpmu_otg_xceiv_data,
			 bcm_otg_id_chg_notifier);

	if (xceiv_data) {
		queue_work(xceiv_data->bcm_otg_work_queue,
			   &xceiv_data->bcm_otg_id_status_change_work);
	}

	return 0;
}

static int bcmpmu_otg_xceiv_set_peripheral(struct usb_otg *otg,
					   struct usb_gadget *gadget)
{
	struct bcmpmu_otg_xceiv_data *xceiv_data =
			dev_get_drvdata(otg->phy->dev);
	int status = 0;
	bool id_default_host = false;

	printk(KERN_ERR "%s xceiv_data = %lx\n",
		__func__, (long unsigned int) xceiv_data);

	dev_dbg(xceiv_data->dev, "Setting Peripheral\n");
	otg->gadget = gadget;

	id_default_host = bcmpmu_otg_xceiv_check_id_gnd(xceiv_data) ||
		  bcmpmu_otg_xceiv_check_id_rid_a(xceiv_data);

	if (!id_default_host) {
		if (xceiv_data->otg_enabled &&
			(bcmpmu_otg_xceiv_check_id_rid_b(xceiv_data) ==
			    false)) { /* No SRP if RID_B */
			/* REVISIT. Shutdown uses sequence for lowest power
			 * and does not meet timing so don't do that in OTG mode
			 * for now. Just do SRP for ADP startup */
			bcmpmu_otg_xceiv_do_srp(xceiv_data);
		} else {
			int data;
			bcmpmu_usb_get(xceiv_data->bcmpmu,
				       BCMPMU_USB_CTRL_GET_CHRGR_TYPE, &data);
			if ((data != PMU_CHRGR_TYPE_SDP)
			    && (data != PMU_CHRGR_TYPE_CDP)) {
				/* Shutdown the core */
				atomic_notifier_call_chain(&xceiv_data->
							   otg_xceiver.phy.
							   notifier,
							   USB_EVENT_NONE,
							   NULL);
			}
		}

	} else {
		bcm_hsotgctrl_phy_set_id_stat(false);
		/* Come up connected  */
		bcm_hsotgctrl_phy_set_non_driving(false);
	}
	/* Notify BC Detection is allowed now */
	bcmpmu_usb_set(xceiv_data->bcmpmu,
			BCMPMU_USB_CTRL_ALLOW_BC_DETECT, 1);
	return status;
}

static int bcmpmu_otg_xceiv_set_vbus_power(struct usb_phy *phy,
					   unsigned int ma)
{
	struct bcmpmu_otg_xceiv_data *xceiv_data = dev_get_drvdata(phy->dev);
	int chrgr_type = 0;

	bcmpmu_usb_get(xceiv_data->bcmpmu, BCMPMU_USB_CTRL_GET_CHRGR_TYPE,
			&chrgr_type);

	if ((chrgr_type == PMU_CHRGR_TYPE_SDP) ||
		(chrgr_type == PMU_CHRGR_TYPE_CDP) ||
		(chrgr_type == PMU_CHRGR_TYPE_ACA))
		return bcmpmu_usb_set(xceiv_data->bcmpmu,
			   BCMPMU_USB_CTRL_CHRG_CURR_LMT, ma);

	return -EINVAL;
}

static int bcmpmu_otg_xceiv_set_host(struct usb_otg *otg,
				     struct usb_bus *host)
{
	struct bcmpmu_otg_xceiv_data *xceiv_data =
		dev_get_drvdata(otg->phy->dev);
	int status = 0;

	dev_dbg(xceiv_data->dev, "Setting Host\n");
	otg->host = host;

	if (host) {
		if (xceiv_data->otg_enabled) {
			/* Wake lock forever in OTG build */
			wake_lock(&xceiv_data->otg_xceiver.xceiver_wake_lock);
			/* Do calibration probe */
			bcm_otg_do_adp_calibration_probe(xceiv_data);
		}

		if (bcmpmu_otg_xceiv_check_id_gnd(xceiv_data) ||
			  bcmpmu_otg_xceiv_check_id_rid_a(xceiv_data)) {
			bcm_hsotgctrl_phy_set_id_stat(false);
			bcm_hsotgctrl_phy_set_non_driving(false);
		} else
			bcm_hsotgctrl_phy_set_id_stat(true);
	}
	return status;
}

static ssize_t bcmpmu_otg_xceiv_wake_store(struct device *dev,
					   struct device_attribute *attr,
					   const char *buf, size_t count)
{
	struct usb_gadget *gadget;
	ssize_t result = 0;
	unsigned int val;
	struct bcmpmu_otg_xceiv_data *xceiv_data =
		dev_get_drvdata(dev);
	int error;

	gadget = xceiv_data->otg_xceiver.phy.otg->gadget;

	/* coverity[secure_coding] */
	result = sscanf(buf, "%u\n", &val);
	if (result != 1) {
		result = -EINVAL;
	} else if (val == 0) {
		dev_warn(xceiv_data->dev, "Illegal value\n");
	} else {
		dev_info(xceiv_data->dev, "Waking up host\n");
		error = usb_gadget_wakeup(gadget);
		if (error)
			dev_err(xceiv_data->dev, "Failed to issue wakeup\n");
	}

	return result < 0 ? result : count;
}

static DEVICE_ATTR(wake, S_IWUSR, NULL, bcmpmu_otg_xceiv_wake_store);

static ssize_t bcmpmu_otg_xceiv_vbus_show(struct device *dev,
					  struct device_attribute *attr,
					  char *buf)
{
	struct bcmpmu_otg_xceiv_data *xceiv_data = dev_get_drvdata(dev);

	return snprintf(buf, 5, "%s\n", xceiv_data->vbus_enabled ? "1" : "0");
}

static ssize_t bcmpmu_otg_xceiv_vbus_store(struct device *dev,
					   struct device_attribute *attr,
					   const char *buf, size_t count)
{
	struct usb_hcd *hcd;
	ssize_t result = 0;
	unsigned int val;
	struct bcmpmu_otg_xceiv_data *xceiv_data = dev_get_drvdata(dev);
	int error;

	hcd = bus_to_hcd(xceiv_data->otg_xceiver.phy.otg->host);
	/* coverity[secure_coding] */
	result = sscanf(buf, "%u\n", &val);
	if (result != 1) {
		result = -EINVAL;
	} else if (val == 0) {
		dev_info(xceiv_data->dev, "Clearing PORT_POWER feature\n");
		error = hcd->driver->hub_control(hcd, ClearPortFeature,
						 USB_PORT_FEAT_POWER, 1, NULL,
						 0);
		if (error)
			dev_err(xceiv_data->dev,
				"Failed to clear PORT_POWER feature\n");
	} else {
		dev_info(xceiv_data->dev, "Setting PORT_POWER feature\n");
		error = hcd->driver->hub_control(hcd, SetPortFeature,
						 USB_PORT_FEAT_POWER, 1, NULL,
						 0);
		if (error)
			dev_err(xceiv_data->dev,
				"Failed to set PORT_POWER feature\n");
	}

	return result < 0 ? result : count;
}

static DEVICE_ATTR(vbus, S_IRUGO | S_IWUSR, bcmpmu_otg_xceiv_vbus_show,
		   bcmpmu_otg_xceiv_vbus_store);

static ssize_t bcmpmu_otg_xceiv_host_show(struct device *dev,
					  struct device_attribute *attr,
					  char *buf)
{
	struct bcmpmu_otg_xceiv_data *xceiv_data = dev_get_drvdata(dev);

	return snprintf(buf, 5, "%s\n", xceiv_data->host ? "1" : "0");
}

static ssize_t bcmpmu_otg_xceiv_host_store(struct device *dev,
					   struct device_attribute *attr,
					   const char *buf, size_t count)
{
	ssize_t result = 0;
	unsigned int val;
	struct bcmpmu_otg_xceiv_data *xceiv_data = dev_get_drvdata(dev);
	/* coverity[secure_coding] */
	result = sscanf(buf, "%u\n", &val);
	if (result != 1)
		result = -EINVAL;
	else
		bcmpmu_otg_xceiv_select_host_mode(xceiv_data, !!val);

	return result < 0 ? result : count;
}

static DEVICE_ATTR(host, S_IRUGO | S_IWUSR, bcmpmu_otg_xceiv_host_show,
		   bcmpmu_otg_xceiv_host_store);

#ifdef CONFIG_USB_OTG
static void bcmpmu_otg_xceiv_srp_failure_handler(unsigned long param)
{
	struct bcmpmu_otg_xceiv_data *xceiv_data =
	    (struct bcmpmu_otg_xceiv_data *)param;
	schedule_delayed_work(&xceiv_data->bcm_otg_delayed_adp_work,
			      msecs_to_jiffies(100));
}

static void bcmpmu_otg_xceiv_sess_end_srp_timer_handler(unsigned long param)
{
	struct bcmpmu_otg_xceiv_data *xceiv_data =
	    (struct bcmpmu_otg_xceiv_data *)param;
	schedule_work(&xceiv_data->bcm_otg_sess_end_srp_work);
}
#endif

static void bcmpmu_otg_xceiv_delayed_adp_handler(struct work_struct *work)
{
	struct bcmpmu_otg_xceiv_data *xceiv_data =
	    container_of(work, struct bcmpmu_otg_xceiv_data,
			 bcm_otg_delayed_adp_work.work);
	dev_info(xceiv_data->dev, "Do ADP probe\n");

	bcm_otg_do_adp_probe(xceiv_data);
	xceiv_data->otg_xceiver.otg_vbus_off = false;
}

static void bcmpmu_otg_xceiv_vbus_invalid_handler(struct work_struct *work)
{
	struct bcmpmu_otg_xceiv_data *xceiv_data =
	    container_of(work, struct bcmpmu_otg_xceiv_data,
			 bcm_otg_vbus_invalid_work);
	dev_info(xceiv_data->dev, "Vbus invalid\n");

	if (xceiv_data->otg_enabled) {
		/* Need to discharge Vbus quickly to session invalid level */
		bcmpmu_usb_set(xceiv_data->bcmpmu, BCMPMU_USB_CTRL_DISCHRG_VBUS,
			       1);
	}
}

static void bcmpmu_otg_xceiv_vbus_valid_handler(struct work_struct *work)
{
	struct bcmpmu_otg_xceiv_data *xceiv_data =
	    container_of(work, struct bcmpmu_otg_xceiv_data,
			 bcm_otg_vbus_valid_work);
	dev_info(xceiv_data->dev, "Vbus valid\n");

#ifdef CONFIG_USB_OTG
	del_timer_sync(&xceiv_data->otg_xceiver.srp_failure_timer);
	del_timer_sync(&xceiv_data->otg_xceiver.sess_end_srp_timer);
#endif
}

static void bcmpmu_otg_xceiv_vbus_a_invalid_handler(struct work_struct *work)
{
	struct bcmpmu_otg_xceiv_data *xceiv_data =
	    container_of(work, struct bcmpmu_otg_xceiv_data,
			 bcm_otg_vbus_a_invalid_work);

	dev_info(xceiv_data->dev, "A session invalid\n");

	/* Inform the core of session invalid level  */
	bcm_hsotgctrl_phy_set_vbus_stat(false);

	if (xceiv_data->otg_enabled) {
		/* Stop Vbus discharge */
		bcmpmu_usb_set(xceiv_data->bcmpmu, BCMPMU_USB_CTRL_DISCHRG_VBUS,
			       0);

		if (bcmpmu_otg_xceiv_check_id_gnd(xceiv_data)) {
			/* Use n-1 method for ADP rise time comparison */
			bcmpmu_usb_set(xceiv_data->bcmpmu,
				       BCMPMU_USB_CTRL_SET_ADP_COMP_METHOD, 1);
			if (xceiv_data->otg_xceiver.otg_vbus_off)
				schedule_delayed_work(&xceiv_data->
					bcm_otg_delayed_adp_work,
					msecs_to_jiffies
					  (T_NO_ADP_DELAY_MIN_IN_MS));
			else
				bcm_otg_do_adp_probe(xceiv_data);
		} else if (!bcmpmu_otg_xceiv_check_id_rid_a(xceiv_data)) {
			if (xceiv_data->otg_xceiver.otg_srp_reqd) {
				/* Start Session End SRP timer */
				xceiv_data->otg_xceiver.sess_end_srp_timer.
				    expires =
				    jiffies +
				    msecs_to_jiffies
				    (T_SESS_END_SRP_START_IN_MS);
				add_timer(&xceiv_data->otg_xceiver.
					  sess_end_srp_timer);
			} else
				bcm_otg_do_adp_sense(xceiv_data);
		}
	} else {
		bool id_default_host = false;

		id_default_host =
			bcmpmu_otg_xceiv_check_id_gnd(xceiv_data) ||
			bcmpmu_otg_xceiv_check_id_rid_a(xceiv_data);

		if (!id_default_host) {
			atomic_notifier_call_chain(&xceiv_data->otg_xceiver.
						   phy.notifier,
						   USB_EVENT_NONE, NULL);
		}
	}
}

void bcmpmu_otg_xceiv_do_srp(struct bcmpmu_otg_xceiv_data *xceiv_data)
{
#ifdef CONFIG_USB_OTG
	if (xceiv_data->otg_xceiver.phy.otg->gadget &&
		    xceiv_data->otg_xceiver.phy.otg->gadget->ops &&
		    xceiv_data->otg_xceiver.phy.otg->gadget->ops->wakeup &&
		    xceiv_data->otg_enabled) {

		bool vbus_status = 0;

		bcmpmu_usb_get(xceiv_data->bcmpmu,
			BCMPMU_USB_CTRL_GET_VBUS_STATUS, &vbus_status);

		/* Should do SRP only if Vbus is not valid */
		if (!vbus_status) {
			bcm_hsotgctrl_phy_set_non_driving(false);
			/* Do SRP */
			xceiv_data->otg_xceiver.phy.otg->gadget->
			    ops->wakeup(xceiv_data->
				otg_xceiver.phy.otg->gadget);
			/* Start SRP failure timer to do ADP probes
			 * if it expires
			 */
			xceiv_data->otg_xceiver.srp_failure_timer.expires =
				  jiffies +
				  msecs_to_jiffies(T_SRP_FAILURE_MAX_IN_MS);
			add_timer(&xceiv_data->otg_xceiver.srp_failure_timer);

			/* SRP initiated. Clear the flag */
			xceiv_data->otg_xceiver.otg_srp_reqd = false;
		}
	}
#endif
}

static void bcmpmu_otg_xceiv_sess_end_srp_handler(struct work_struct *work)
{
	struct bcmpmu_otg_xceiv_data *xceiv_data =
	    container_of(work, struct bcmpmu_otg_xceiv_data,
			 bcm_otg_sess_end_srp_work);

	bcmpmu_otg_xceiv_do_srp(xceiv_data);
}

static void bcmpmu_otg_xceiv_vbus_a_valid_handler(struct work_struct *work)
{
	struct bcmpmu_otg_xceiv_data *xceiv_data =
	    container_of(work, struct bcmpmu_otg_xceiv_data,
			 bcm_otg_vbus_a_valid_work);
	dev_info(xceiv_data->dev, "A session valid\n");
}

static void bcmpmu_otg_xceiv_id_change_handler(struct work_struct *work)
{
	struct bcmpmu_otg_xceiv_data *xceiv_data =
	    container_of(work, struct bcmpmu_otg_xceiv_data,
			 bcm_otg_id_status_change_work);
	bool id_gnd = false;
	bool id_rid_a = false;
	bool id_rid_c = false;

	dev_info(xceiv_data->dev, "ID change detected\n");
	id_gnd = bcmpmu_otg_xceiv_check_id_gnd(xceiv_data);
	id_rid_a = bcmpmu_otg_xceiv_check_id_rid_a(xceiv_data);
	id_rid_c = bcmpmu_otg_xceiv_check_id_rid_c(xceiv_data);

	bcm_hsotgctrl_phy_set_id_stat(!(id_gnd || id_rid_a));

	/* If ID is gnd, we need to turn on Vbus within 200ms
	 * If ID is RID_A/B/C/FLOAT then we should not turn it on
	 */
	bcmpmu_otg_xceiv_set_vbus(&xceiv_data->otg_xceiver.
		    otg, id_gnd ? true : false);

	if (!id_rid_c)
		msleep(HOST_TO_PERIPHERAL_DELAY_MS);

	if (id_gnd || id_rid_a || id_rid_c) {
		if (xceiv_data->otg_xceiver.phy.state ==
				OTG_STATE_UNDEFINED)
			atomic_notifier_call_chain(&xceiv_data->otg_xceiver.phy.
					   notifier, USB_EVENT_ID, NULL);
	} else if (bcmpmu_otg_xceiv_check_id_rid_float(xceiv_data) &&
		(xceiv_data->otg_xceiver.phy.state != OTG_STATE_UNDEFINED)) {
		atomic_notifier_call_chain(&xceiv_data->otg_xceiver.phy.
					   notifier, USB_EVENT_NONE, NULL);
	}
}

static void bcmpmu_otg_xceiv_chg_detect_handler(struct work_struct *work)
{
	struct bcmpmu_otg_xceiv_data *xceiv_data =
	    container_of(work, struct bcmpmu_otg_xceiv_data,
			 bcm_otg_chg_detect_work);

	dev_info(xceiv_data->dev, "Charger detect event\n");

	if (xceiv_data->otg_enabled) {
		/* Core is already up so just set the Vbus status */
		bcm_hsotgctrl_phy_set_vbus_stat(true);
	} else {
		bool id_default_host = false;

		id_default_host =
			bcmpmu_otg_xceiv_check_id_gnd(xceiv_data) ||
			bcmpmu_otg_xceiv_check_id_rid_a(xceiv_data) ||
			bcmpmu_otg_xceiv_check_id_rid_b(xceiv_data);

		if (!id_default_host && xceiv_data->otg_xceiver.phy.otg->gadget)
			atomic_notifier_call_chain(&xceiv_data->otg_xceiver.
						   phy.notifier,
						   USB_EVENT_VBUS, NULL);
	}
}

static void bcmpmu_otg_free_regulator(struct bcmpmu_otg_xceiv_data *xceiv_data)
{
	if (xceiv_data && xceiv_data->bcm_hsotg_regulator &&
		    xceiv_data->regulator_enabled) {
		regulator_disable(xceiv_data->bcm_hsotg_regulator);
		xceiv_data->regulator_enabled = false;
		regulator_put(xceiv_data->bcm_hsotg_regulator);
	}
}

static int __init bcmpmu_otg_xceiv_probe(struct platform_device *pdev)
{
	int error = 0;
	struct bcmpmu_otg_xceiv_data *xceiv_data;
	struct bcmpmu59xxx *bcmpmu = dev_get_drvdata(pdev->dev.parent);

	dev_info(&pdev->dev, "Probing started...\n");

	xceiv_data = kzalloc(sizeof(*xceiv_data), GFP_KERNEL);
	if (!xceiv_data) {
		dev_warn(&pdev->dev, "Memory allocation failed\n");
		return -ENOMEM;
	}

	/* REVISIT: Currently there isn't a way to obtain
	 * regulator string associated with USB. Hardcode for now
	 */
	xceiv_data->bcm_hsotg_regulator =
		regulator_get(NULL, "usb_vcc");

	if (IS_ERR(xceiv_data->bcm_hsotg_regulator)) {
		dev_warn(&pdev->dev, "Failed to get regulator handle\n");
		error = -ENODEV;
		goto error_get_reg;
	}

	/* Enable USB LDO */
	error = regulator_enable(xceiv_data->bcm_hsotg_regulator);
	if (error) {
		dev_warn(xceiv_data->dev, "regulator_enable failed\n");
		goto error_enable_reg;
	}
	/* Give 2ms to ramp up USBLDO */
	mdelay(USBLDO_RAMP_UP_DELAY_IN_MS);
	xceiv_data->regulator_enabled = true;

	xceiv_data->dev = &pdev->dev;
	xceiv_data->bcmpmu = bcmpmu;
	xceiv_data->otg_xceiver.phy.dev = xceiv_data->dev;
	xceiv_data->otg_xceiver.phy.label = "bcmpmu_otg_xceiv";
	xceiv_data->otg_xceiver.phy.otg = &xceiv_data->otg_xceiver.otg;
	xceiv_data->otg_xceiver.otg.phy = &xceiv_data->otg_xceiver.phy;
	xceiv_data->host = false;
	xceiv_data->vbus_enabled = false;

	/* Create a work queue for OTG work items */
	xceiv_data->bcm_otg_work_queue = create_workqueue("bcm_otg_events");
	if (xceiv_data->bcm_otg_work_queue == NULL) {
		dev_warn(&pdev->dev,
			 "BCM OTG events work queue creation failed\n");
		error = -ENOMEM;
		goto error_create_wq;
	}

	/* Create one work item per deferrable function */
	INIT_WORK(&xceiv_data->bcm_otg_vbus_invalid_work,
		  bcmpmu_otg_xceiv_vbus_invalid_handler);
	INIT_WORK(&xceiv_data->bcm_otg_vbus_valid_work,
		  bcmpmu_otg_xceiv_vbus_valid_handler);
	INIT_WORK(&xceiv_data->bcm_otg_vbus_a_invalid_work,
		  bcmpmu_otg_xceiv_vbus_a_invalid_handler);
	INIT_WORK(&xceiv_data->bcm_otg_vbus_a_valid_work,
		  bcmpmu_otg_xceiv_vbus_a_valid_handler);
	INIT_WORK(&xceiv_data->bcm_otg_id_status_change_work,
		  bcmpmu_otg_xceiv_id_change_handler);
	INIT_WORK(&xceiv_data->bcm_otg_chg_detect_work,
		  bcmpmu_otg_xceiv_chg_detect_handler);
	INIT_WORK(&xceiv_data->bcm_otg_sess_end_srp_work,
		  bcmpmu_otg_xceiv_sess_end_srp_handler);
	INIT_DELAYED_WORK(&xceiv_data->bcm_otg_delayed_adp_work,
			  bcmpmu_otg_xceiv_delayed_adp_handler);

	xceiv_data->otg_xceiver.phy.state = OTG_STATE_UNDEFINED;
	xceiv_data->otg_xceiver.phy.type = USB_PHY_TYPE_UNDEFINED;
	xceiv_data->otg_xceiver.phy.otg->set_vbus = bcmpmu_otg_xceiv_set_vbus;
	xceiv_data->otg_xceiver.phy.otg->set_peripheral =
	    bcmpmu_otg_xceiv_set_peripheral;
	xceiv_data->otg_xceiver.phy.set_power =
	    bcmpmu_otg_xceiv_set_vbus_power;
	xceiv_data->otg_xceiver.phy.otg->set_host = bcmpmu_otg_xceiv_set_host;
	xceiv_data->otg_xceiver.phy.shutdown = bcmpmu_otg_xceiv_shutdown;
	xceiv_data->otg_xceiver.phy.init = bcmpmu_otg_xceiv_start;
	xceiv_data->otg_xceiver.phy.otg->set_delayed_adp =
	    bcmpmu_otg_xceiv_set_delayed_adp;
	xceiv_data->otg_xceiver.phy.otg->set_srp_reqd =
	    bcmpmu_otg_xceiv_set_srp_reqd_handler;
	xceiv_data->otg_xceiver.phy.otg->set_otg_enable =
	    bcmpmu_otg_xceiv_set_otg_enable;
	xceiv_data->otg_xceiver.phy.notify_connect = bcmpmu_otg_xceiv_connect;
	xceiv_data->otg_xceiver.phy.notify_disconnect =
		bcmpmu_otg_xceiv_disconnect;

	xceiv_data->otg_xceiver.phy.set_suspend =
	    bcmpmu_otg_xceiv_set_suspend;

	ATOMIC_INIT_NOTIFIER_HEAD(&xceiv_data->otg_xceiver.phy.notifier);

	xceiv_data->bcm_otg_vbus_validity_notifier.notifier_call =
	    bcmpmu_otg_xceiv_vbus_notif_handler;
	bcmpmu_add_notifier(PMU_ACCY_EVT_OUT_VBUS_VALID,
			    &xceiv_data->bcm_otg_vbus_validity_notifier);

	xceiv_data->bcm_otg_session_invalid_notifier.notifier_call =
	    bcmpmu_otg_xceiv_vbus_notif_handler;
	bcmpmu_add_notifier(PMU_ACCY_EVT_OUT_SESSION_INVALID,
			    &xceiv_data->bcm_otg_session_invalid_notifier);

	xceiv_data->bcm_otg_id_chg_notifier.notifier_call =
	    bcmpmu_otg_xceiv_id_chg_notif_handler;
	bcmpmu_add_notifier(PMU_ACCY_EVT_OUT_ID_CHANGE,
			    &xceiv_data->bcm_otg_id_chg_notifier);

	xceiv_data->bcm_otg_chg_detection_notifier.notifier_call =
	    bcmpmu_otg_xceiv_chg_detection_notif_handler;
	bcmpmu_add_notifier(PMU_ACCY_EVT_OUT_CHRGR_TYPE,
			    &xceiv_data->bcm_otg_chg_detection_notifier);

	xceiv_data->bcm_otg_xceive_state_notifier.notifier_call =
		bcmpmu_otg_xceiv_state_handler;
	bcmpmu_add_notifier(PMU_CHRGR_DET_EVT_OUT_XCVR,
			    &xceiv_data->bcm_otg_xceive_state_notifier);

	wake_lock_init(&xceiv_data->otg_xceiver.xceiver_wake_lock,
		       WAKE_LOCK_SUSPEND, "otg_xcvr_wakelock");

#ifdef CONFIG_USB_OTG
	init_timer(&xceiv_data->otg_xceiver.srp_failure_timer);
	xceiv_data->otg_xceiver.srp_failure_timer.data =
	    (unsigned long)xceiv_data;
	xceiv_data->otg_xceiver.srp_failure_timer.function =
	    bcmpmu_otg_xceiv_srp_failure_handler;

	init_timer(&xceiv_data->otg_xceiver.sess_end_srp_timer);
	xceiv_data->otg_xceiver.sess_end_srp_timer.data =
	    (unsigned long)xceiv_data;
	xceiv_data->otg_xceiver.sess_end_srp_timer.function =
	    bcmpmu_otg_xceiv_sess_end_srp_timer_handler;

	error = bcm_otg_adp_init(xceiv_data);
	if (error)
		goto error_attr_host;
#endif

	usb_add_phy(&xceiv_data->otg_xceiver.phy, USB_PHY_TYPE_USB2);

	platform_set_drvdata(pdev, xceiv_data);

	error = device_create_file(&pdev->dev, &dev_attr_host);
	if (error) {
		dev_warn(&pdev->dev, "Failed to create HOST file\n");
		goto error_attr_host;
	}

	error = device_create_file(&pdev->dev, &dev_attr_vbus);
	if (error) {
		dev_warn(&pdev->dev, "Failed to create VBUS file\n");
		goto error_attr_vbus;
	}

	error = device_create_file(&pdev->dev, &dev_attr_wake);
	if (error) {
		dev_warn(&pdev->dev, "Failed to create WAKE file\n");
		goto error_attr_wake;
	}

	/* Check if we should default to A-device */
	xceiv_data->otg_xceiver.phy.otg->default_a =
	    bcmpmu_otg_xceiv_check_id_gnd(xceiv_data) ||
	    bcmpmu_otg_xceiv_check_id_rid_a(xceiv_data);

	/* Initialize OTG PHY */
	error = bcm_hsotgctrl_phy_init(!xceiv_data->otg_xceiver.phy.otg->default_a);
	if (error) {
		printk(KERN_ERR "%s-Fail to USB Phy Init\n", __func__);
		goto error_attr_wake;
	}

	bcmpmu_otg_xceiv_set_def_state(xceiv_data,
		xceiv_data->otg_xceiver.phy.otg->default_a);

	bcm_hsotgctrl_register_wakeup_cb((send_core_event_cb_t)bcmpmu_otg_xceiv_wakeup_core,
									 (void*)&xceiv_data->otg_xceiver.phy);

	pm_runtime_set_active(&pdev->dev);
	pm_runtime_enable(&pdev->dev);

	dev_info(&pdev->dev, "Probing successful\n");

	return 0;

error_attr_wake:
	device_remove_file(xceiv_data->dev, &dev_attr_vbus);

error_attr_vbus:
	device_remove_file(xceiv_data->dev, &dev_attr_host);

error_attr_host:
	wake_lock_destroy(&xceiv_data->otg_xceiver.xceiver_wake_lock);
	destroy_workqueue(xceiv_data->bcm_otg_work_queue);
error_create_wq:
	regulator_disable(xceiv_data->bcm_hsotg_regulator);
error_enable_reg:
	regulator_put(xceiv_data->bcm_hsotg_regulator);
error_get_reg:
	kfree(xceiv_data);
	return error;
}

static int __exit bcmpmu_otg_xceiv_remove(struct platform_device *pdev)
{
	struct bcmpmu_otg_xceiv_data *xceiv_data = platform_get_drvdata(pdev);

	usb_remove_phy(&xceiv_data->otg_xceiver.phy);
	xceiv_data->otg_xceiver.phy.state = OTG_STATE_UNDEFINED;
	device_remove_file(xceiv_data->dev, &dev_attr_wake);
	device_remove_file(xceiv_data->dev, &dev_attr_vbus);
	device_remove_file(xceiv_data->dev, &dev_attr_host);

	pm_runtime_disable(&pdev->dev);

	if (wake_lock_active(&xceiv_data->otg_xceiver.xceiver_wake_lock)) {
		wake_unlock(&xceiv_data->otg_xceiver.xceiver_wake_lock);
		wake_lock_destroy(&xceiv_data->otg_xceiver.xceiver_wake_lock);
	}

	/* Remove notifiers */
	bcmpmu_remove_notifier(PMU_ACCY_EVT_OUT_VBUS_VALID,
			       &xceiv_data->bcm_otg_vbus_validity_notifier);
	bcmpmu_remove_notifier(PMU_ACCY_EVT_OUT_SESSION_INVALID,
			       &xceiv_data->bcm_otg_session_invalid_notifier);
	bcmpmu_remove_notifier(PMU_ACCY_EVT_OUT_ID_CHANGE,
			       &xceiv_data->bcm_otg_id_chg_notifier);
	bcmpmu_remove_notifier(PMU_ACCY_EVT_OUT_CHRGR_TYPE,
			       &xceiv_data->bcm_otg_chg_detection_notifier);
	bcmpmu_remove_notifier(PMU_CHRGR_DET_EVT_OUT_XCVR,
			    &xceiv_data->bcm_otg_xceive_state_notifier);

	destroy_workqueue(xceiv_data->bcm_otg_work_queue);
	bcmpmu_otg_free_regulator(xceiv_data);
	kfree(xceiv_data);

	bcm_hsotgctrl_unregister_wakeup_cb();

	bcm_hsotgctrl_phy_deinit();
	return 0;
}

static int bcmpmu_otg_xceiv_pm_suspend(struct platform_device *pdev,
	pm_message_t state)
{
	int status = 0;
	struct bcmpmu_otg_xceiv_data *xceiv_data = platform_get_drvdata(pdev);
	bool suspend_allowed = false;

	if (bcm_hsotgctrl_is_suspend_allowed(&suspend_allowed)) {
		dev_warn(&pdev->dev,
			"Failed to check if USB allows sys suspend\n");
	} else {
		if ((suspend_allowed == false) || xceiv_data->otg_enabled) {
			/* Don't allow runtime suspend if USB is active
			 * or in OTG mode */
			status = -EBUSY;
		} else {
			/* Allow runtime suspend since USB is not active */
			status = 0;
		}
	}
	return status;
}

static int bcmpmu_otg_xceiv_pm_resume(struct platform_device *pdev)
{
	return 0;
}

static struct platform_driver __refdata bcmpmu_otg_xceiv_driver = {
	.probe = bcmpmu_otg_xceiv_probe,
	.remove = __exit_p(bcmpmu_otg_xceiv_remove),
	.suspend = bcmpmu_otg_xceiv_pm_suspend,
	.resume = bcmpmu_otg_xceiv_pm_resume,
	.driver = {
		   .name = "bcmpmu_otg_xceiv",
		   .owner = THIS_MODULE,
		   },
};

static int __init bcmpmu_otg_xceiv_init(void)
{
	pr_info("Broadcom USB OTG Transceiver Driver\n");

	return platform_driver_register(&bcmpmu_otg_xceiv_driver);
}

subsys_initcall(bcmpmu_otg_xceiv_init);

static void __exit bcmpmu_otg_xceiv_exit(void)
{
	platform_driver_unregister(&bcmpmu_otg_xceiv_driver);
}

module_exit(bcmpmu_otg_xceiv_exit);

MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("USB OTG transceiver driver");
MODULE_LICENSE("GPL");
