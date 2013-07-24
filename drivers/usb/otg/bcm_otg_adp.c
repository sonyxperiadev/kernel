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
#include <linux/platform_device.h>
#include <linux/usb/otg.h>
#ifdef CONFIG_MFD_BCM_PMU59xxx
#include "bcmpmu59xxx_otg_xceiv.h"
#include <linux/mfd/bcmpmu59xxx.h>
#else
#include <linux/mfd/bcmpmu.h>
#include "bcmpmu_otg_xceiv.h"
#endif
#include "bcm_otg_adp.h"


static void bcmpmu_otg_xceiv_adp_cprb_done_handler(struct work_struct *work)
{
	struct bcmpmu_otg_xceiv_data *xceiv_data =
	    container_of(work, struct bcmpmu_otg_xceiv_data,
			 bcm_otg_adp_cprb_done_work);
	dev_info(xceiv_data->dev, "ADP calibration probe done\n");
}

static void bcmpmu_otg_xceiv_adp_change_handler(struct work_struct *work)
{
	struct bcmpmu_otg_xceiv_data *xceiv_data =
	    container_of(work, struct bcmpmu_otg_xceiv_data,
			 bcm_otg_adp_change_work);
	dev_info(xceiv_data->dev, "ADP change detected\n");

	/* Need to turn on Vbus within 200ms */
	if (bcmpmu_otg_xceiv_check_id_gnd(xceiv_data)) {
		if (xceiv_data->otg_xceiver.phy.otg->set_vbus)
			xceiv_data->otg_xceiver.phy.otg->set_vbus(&xceiv_data->
								 otg_xceiver.
								 otg, true);

		atomic_notifier_call_chain(&xceiv_data->otg_xceiver.phy.
					   notifier, USB_EVENT_VBUS, NULL);
	} else
		bcmpmu_otg_xceiv_do_srp(xceiv_data);	/* Do SRP */

}

static void bcmpmu_otg_xceiv_adp_sense_end_handler(struct work_struct *work)
{
#ifdef CONFIG_USB_OTG
	uint32_t adp_sns_status = 0;
	struct bcmpmu_otg_xceiv_data *xceiv_data =
	    container_of(work, struct bcmpmu_otg_xceiv_data,
			 bcm_otg_sens_end_work);
	dev_info(xceiv_data->dev, "ADP Sense End detected\n");

	if (xceiv_data->otg_enabled) {
		/* Check if probing was detected */
		bcmpmu_usb_get(xceiv_data->bcmpmu,
			       BCMPMU_USB_CTRL_GET_ADP_SENSE_STATUS,
			       &adp_sns_status);

		if (adp_sns_status)
			schedule_delayed_work(&xceiv_data->
					      bcm_otg_delayed_adp_work,
					      msecs_to_jiffies(T_B_ADP_DETACH));
		else
			bcm_otg_do_adp_probe(xceiv_data);
	}
#endif
}

static int bcmpmu_otg_xceiv_adp_change_notif_handler(struct notifier_block *nb,
						     unsigned long value,
						     void *data)
{
	struct bcmpmu_otg_xceiv_data *xceiv_data =
	    container_of(nb, struct bcmpmu_otg_xceiv_data,
			 bcm_otg_adp_change_done_notifier);

	queue_work(xceiv_data->bcm_otg_work_queue,
		   &xceiv_data->bcm_otg_adp_change_work);

	return 0;
}

static int bcmpmu_otg_xceiv_adp_sns_end_notif_handler(struct notifier_block *nb,
						      unsigned long value,
						      void *data)
{
	struct bcmpmu_otg_xceiv_data *xceiv_data =
	    container_of(nb, struct bcmpmu_otg_xceiv_data,
			 bcm_otg_adp_sns_end_notifier);

	queue_work(xceiv_data->bcm_otg_work_queue,
		   &xceiv_data->bcm_otg_sens_end_work);

	return 0;
}

int bcm_otg_do_adp_calibration_probe(struct bcmpmu_otg_xceiv_data *xceiv_data)
{
	if (!xceiv_data)
		return -EINVAL;

#ifdef CONFIG_USB_OTG
	if (xceiv_data->otg_enabled)
		return bcmpmu_usb_set(xceiv_data->bcmpmu,
				      BCMPMU_USB_CTRL_START_ADP_CAL_PRB, 1);
#endif
	return 0;
}

int bcm_otg_do_adp_probe(struct bcmpmu_otg_xceiv_data *xceiv_data)
{
	int status = 0;

#ifdef CONFIG_USB_OTG
	if (xceiv_data->otg_enabled) {
		status =
		    bcmpmu_usb_set(xceiv_data->bcmpmu,
				   BCMPMU_USB_CTRL_SET_ADP_PRB_MOD,
				   PMU_USB_ADP_MODE_REPEAT);
		if (!status)
			status =
			    bcmpmu_usb_set(xceiv_data->bcmpmu,
					   BCMPMU_USB_CTRL_START_STOP_ADP_PRB,
					   1);
	}
#endif

	return status;
}

int bcm_otg_do_adp_sense(struct bcmpmu_otg_xceiv_data *xceiv_data)
{
#ifdef CONFIG_USB_OTG
	if (xceiv_data->otg_enabled) {
		return
		    bcmpmu_usb_set(xceiv_data->bcmpmu,
				   BCMPMU_USB_CTRL_START_STOP_ADP_SENS_PRB, 1);
	}
#endif

	return 0;
}

int bcm_otg_adp_init(struct bcmpmu_otg_xceiv_data *xceiv_data)
{
	int status = 0;

	/* Init work items for ADP event handling */
	INIT_WORK(&xceiv_data->bcm_otg_adp_cprb_done_work,
		  bcmpmu_otg_xceiv_adp_cprb_done_handler);
	INIT_WORK(&xceiv_data->bcm_otg_adp_change_work,
		  bcmpmu_otg_xceiv_adp_change_handler);
	INIT_WORK(&xceiv_data->bcm_otg_sens_end_work,
		  bcmpmu_otg_xceiv_adp_sense_end_handler);

	if (xceiv_data->bcm_otg_adp_change_done_notifier.notifier_call == NULL) {
		xceiv_data->bcm_otg_adp_change_done_notifier.notifier_call =
		    bcmpmu_otg_xceiv_adp_change_notif_handler;
		bcmpmu_add_notifier(PMU_ACCY_EVT_OUT_ADP_CHANGE,
				    &xceiv_data->
				    bcm_otg_adp_change_done_notifier);

		xceiv_data->bcm_otg_adp_sns_end_notifier.notifier_call =
		    bcmpmu_otg_xceiv_adp_sns_end_notif_handler;
		bcmpmu_add_notifier(PMU_ACCY_EVT_OUT_ADP_SENSE_END,
				    &xceiv_data->bcm_otg_adp_sns_end_notifier);
	}

	/* Set debounce to 1 for A-ADP probe level compliance */
	status =
	    bcmpmu_usb_set(xceiv_data->bcmpmu,
			   BCMPMU_USB_CTRL_SET_VBUS_DEB_TIME, 1);

	/* Set ADP default compliance to n-2 */
	if (!status)
		status =
		    bcmpmu_usb_set(xceiv_data->bcmpmu,
				   BCMPMU_USB_CTRL_SET_ADP_COMP_METHOD, 2);

	return status;

}

MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("USB OTG ADP Interface driver");
MODULE_LICENSE("GPL");
