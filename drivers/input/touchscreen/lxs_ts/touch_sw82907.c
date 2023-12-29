/*
 * SPDX-License-Identifier: GPL-2.0
 *
 * touch_sw82908.c
 *
 * LXS touch entry for SW82907
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "lxs_ts.h"

#define MODE_ALLOWED_SW82907		(0 |	\
									LCD_MODE_BIT_U0 |	\
									LCD_MODE_BIT_U3 |	\
									LCD_MODE_BIT_STOP |	\
									0)

#define BUS_TYPE_SW82907			BUS_I2C

static const struct lxs_ts_reg_quirk reg_quirks_tbl_sw82907[] = {
	{ .old_addr = INFO_PTR, .new_addr = 0x020, },
	{ .old_addr = SPR_CHIP_TEST, .new_addr = 0x022, },
	{ .old_addr = SPR_RST_CTL, .new_addr = 0x004, },
	/* */
	{ .old_addr = SERIAL_DATA_OFFSET, .new_addr = 0x045, },
	/* */
#if 0
	{ .old_addr = PRD_SERIAL_TCM_OFFSET, .new_addr = 0x028, },
	{ .old_addr = PRD_TC_MEM_SEL, .new_addr = 0x8C8, },
#endif
	{ .old_addr = PRD_TC_TEST_MODE_CTL, .new_addr = ADDR_SKIP_MASK, },
	/* not fixed */
	{ .old_addr = PRD_TUNE_RESULT_OFFSET, .new_addr = ADDR_SKIP_MASK, },
#if 0
	{ .old_addr = PRD_M1_M2_RAW_OFFSET, .new_addr = 0x323, },
	{ .old_addr = PRD_TUNE_RESULT_OFFSET, .new_addr = 0x325, },
	{ .old_addr = PRD_OPEN3_SHORT_OFFSET, .new_addr = 0x324, },
#endif
	/* */
	{ .old_addr = ~0, .new_addr = ~0 },		// End signal
};

static const struct of_device_id of_match_tbl_sw82907[] = {
	{ .compatible = "lxs,sw82907" },
	{ },
};
MODULE_DEVICE_TABLE(of, of_match_tbl_sw82907);

static const struct lxs_ts_entry_data ts_entry_data_sw82907 = {
	.chip_type			= CHIP_SW82907,
	.of_match_table 	= of_match_ptr(of_match_tbl_sw82907),
	.chip_id			= "7601",
	.chip_name			= "SW82907",
	.mode_allowed		= MODE_ALLOWED_SW82907,
	.fw_size			= (128<<10),
	/* */
	.bus_type			= BUS_TYPE_SW82907,
	/* */
	.reg_quirks			= reg_quirks_tbl_sw82907,
};

struct lxs_ts_if_driver ts_if_drv_sw82907 = {
	.entry_data = &ts_entry_data_sw82907,
};

static int __init lxs_ts_driver_init(void)
{
	t_pr_info("%s: SW82907 driver init - %s\n", LXS_TS_NAME, LXS_DRV_VERSION);
	return lxs_ts_register_driver(&ts_if_drv_sw82907);
}

static void __exit lxs_ts_driver_exit(void)
{
	lxs_ts_unregister_driver(&ts_if_drv_sw82907);
	t_pr_info("%s: SW82907 driver exit - %s\n", LXS_TS_NAME, LXS_DRV_VERSION);
}
module_init(lxs_ts_driver_init);
module_exit(lxs_ts_driver_exit);

MODULE_AUTHOR("kimhh@lxsemicon.com");
MODULE_DESCRIPTION("LXS Touch SW82907 Driver");
MODULE_VERSION(LXS_DRV_VERSION);
MODULE_LICENSE("GPL");
MODULE_IMPORT_NS(VFS_internal_I_am_really_a_filesystem_and_am_NOT_a_driver);


