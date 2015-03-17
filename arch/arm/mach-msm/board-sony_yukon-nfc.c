/* arch/arm/mach-msm/board-sony_shinano-nfc.c
 *
 * Copyright (C) 2013 Sony Mobile Communications AB.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include <mach/board-nfc.h>

int board_nfc_parse_dt(struct device *dev,
			struct pn547_i2c_platform_data *pdata)
{
	return 0;
}

int board_nfc_hw_lag_check(struct i2c_client *d,
			   struct pn547_i2c_platform_data *pdata)
{
	return 0;
}
