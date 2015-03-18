/*
 * cyttsp4_mtb.c
 * Cypress TrueTouch(TM) Standard Product V4 Multi-touch module.
 * For use with Cypress Txx4xx parts.
 * Supported parts include:
 * TMA4XX
 * TMA1036
 *
 * Copyright (C) 2012 Cypress Semiconductor
 * Copyright (C) 2011 Sony Ericsson Mobile Communications AB.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2, and only version 2, as published by the
 * Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Contact Cypress Semiconductor at www.cypress.com <ttdrivers@cypress.com>
 *
 */

#include <linux/input.h>
#include <linux/input/mt.h>
#include <linux/cyttsp4_core.h>

#include "cyttsp4_mt_common.h"

static void cyttsp4_final_sync(struct input_dev *input, int max_slots,
		int mt_sync_count, unsigned long *ids)
{
	int t;

	for (t = 0; t < max_slots; t++) {
		if (test_bit(t, ids))
			continue;
		input_mt_slot(input, t);
		input_mt_report_slot_state(input, MT_TOOL_FINGER, false);
	}

	input_sync(input);
}

static void cyttsp4_input_report(struct input_dev *input, int sig,
		int t, int type)
{
	input_mt_slot(input, t);

	if (type == CY_OBJ_STANDARD_FINGER || type == CY_OBJ_GLOVE)
		input_mt_report_slot_state(input, MT_TOOL_FINGER, true);
	else if (type == CY_OBJ_STYLUS)
		input_mt_report_slot_state(input, MT_TOOL_PEN, true);
}

static void cyttsp4_report_slot_liftoff(struct cyttsp4_mt_data *md,
		int max_slots)
{
	int t;

	if (md->num_prv_rec == 0)
		return;

	for (t = 0; t < max_slots; t++) {
		input_mt_slot(md->input, t);
		input_mt_report_slot_state(md->input,
			MT_TOOL_FINGER, false);
	}
}

static int cyttsp4_input_register_device(struct input_dev *input, int max_slots)
{
	input_set_abs_params(input, ABS_MT_TOOL_TYPE,
			0, MT_TOOL_FINGER, 0, 0);

	input_mt_init_slots(input, max_slots, 0);
	return input_register_device(input);
}

void cyttsp4_init_function_ptrs(struct cyttsp4_mt_data *md)
{
	md->mt_function.report_slot_liftoff = cyttsp4_report_slot_liftoff;
	md->mt_function.final_sync = cyttsp4_final_sync;
	md->mt_function.input_sync = NULL;
	md->mt_function.input_report = cyttsp4_input_report;
	md->mt_function.input_register_device = cyttsp4_input_register_device;
}

static int __init cyttsp4_mt_init(void)
{
	int rc;
	cyttsp4_mt_driver.driver.owner = THIS_MODULE;
	rc = cyttsp4_register_driver(&cyttsp4_mt_driver);
	pr_info("%s: Cypress TTSP MT v4 multi-touch (Built %s), rc=%d\n",
		 __func__, CY_DRIVER_DATE, rc);
	return rc;
}
module_init(cyttsp4_mt_init);

static void __exit cyttsp4_mt_exit(void)
{
	cyttsp4_unregister_driver(&cyttsp4_mt_driver);
	pr_info("%s: module exit\n", __func__);
}
module_exit(cyttsp4_mt_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Cypress TrueTouch(R) Standard 2D multi-touch driver");
MODULE_AUTHOR("Cypress Semiconductor");
