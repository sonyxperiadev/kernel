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
#include <linux/mfd/bcmpmu.h>
#include <linux/usb/otg.h>
#include "bcmpmu_otg_xceiv.h"

static void bcmpmu_otg_xceiv_adp_cprb_done_handler(struct work_struct *work)
{
	struct bcmpmu_otg_xceiv_data *xceiv_data =
	    container_of(work, struct bcmpmu_otg_xceiv_data,
			 bcm_otg_adp_cprb_done_work);
	dev_info(xceiv_data->dev, "ADP calibration probe done\n");
}

static void bcmpmu_otg_xceiv_adp_change_handler(struct work_struct *work)
{
	unsigned int data = 0;
	struct bcmpmu_otg_xceiv_data *xceiv_data =
	    container_of(work, struct bcmpmu_otg_xceiv_data,
			 bcm_otg_adp_change_work);
	dev_info(xceiv_data->dev, "ADP change detected\n");

	bcmpmu_usb_get(xceiv_data->bcmpmu, BCMPMU_USB_CTRL_GET_ID_VALUE, &data);

	/* Need to turn on Vbus within 200ms */
	if ((data == PMU_USB_ID_GROUND) && xceiv_data->otg_xceiver.xceiver.set_vbus)
		xceiv_data->otg_xceiver.xceiver.set_vbus(&xceiv_data->otg_xceiver.xceiver, true);

	atomic_notifier_call_chain(&xceiv_data->otg_xceiver.xceiver.notifier, USB_EVENT_VBUS, NULL);
}

static int bcmpmu_otg_xceiv_adp_change_notif_handler(struct notifier_block *nb,
					       unsigned long value, void *data)
{
	struct bcmpmu_otg_xceiv_data *xceiv_data =
	    container_of(nb, struct bcmpmu_otg_xceiv_data,
			 bcm_otg_adp_change_done_notifier);

	if (!xceiv_data)
		return -EINVAL;

	queue_work(xceiv_data->bcm_otg_work_queue,
		   &xceiv_data->bcm_otg_adp_change_work);

	return 0;
}
int bcm_otg_do_adp_calibration_probe(struct bcmpmu_otg_xceiv_data *xceiv_data)
{
	return bcmpmu_usb_set(xceiv_data->bcmpmu, BCMPMU_USB_CTRL_START_ADP_CAL_PRB, 1);
}

int bcm_otg_do_adp_probe(struct bcmpmu_otg_xceiv_data *xceiv_data)
{
	int status = 0;

	status = bcmpmu_usb_set(xceiv_data->bcmpmu, BCMPMU_USB_CTRL_SET_ADP_PRB_MOD, PMU_USB_ADP_MODE_REPEAT);
	if (!status)
		status = bcmpmu_usb_set(xceiv_data->bcmpmu, BCMPMU_USB_CTRL_START_STOP_ADP_PRB, 1);

	return status;
}

int bcm_otg_do_adp_sense(struct bcmpmu_otg_xceiv_data *xceiv_data)
{
	return
		bcmpmu_usb_set(xceiv_data->bcmpmu,
		  BCMPMU_USB_CTRL_START_STOP_ADP_SENS_PRB, 1);
}

int bcm_otg_do_adp_sense_then_probe(struct bcmpmu_otg_xceiv_data *xceiv_data)
{
	/* PMU driver does not handle this request so we
	** need to implement this here */
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

	if (xceiv_data->bcm_otg_adp_change_done_notifier.notifier_call == NULL) {
		xceiv_data->bcm_otg_adp_change_done_notifier.notifier_call =
		    bcmpmu_otg_xceiv_adp_change_notif_handler;
		bcmpmu_add_notifier(BCMPMU_USB_EVENT_ADP_CHANGE,
				    &xceiv_data->bcm_otg_adp_change_done_notifier);
	}

	/* Set debounce to 1 for A-ADP probe level compliance */
	status = bcmpmu_usb_set(xceiv_data->bcmpmu, BCMPMU_USB_CTRL_SET_VBUS_DEB_TIME, 1);

	/* Set ADP default compliance to n-2 */
	if (!status)
		status = bcmpmu_usb_set(xceiv_data->bcmpmu, BCMPMU_USB_CTRL_SET_ADP_COMP_METHOD, 2);

	return status;

}

MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("USB OTG ADP Interface driver");
MODULE_LICENSE("GPL");
