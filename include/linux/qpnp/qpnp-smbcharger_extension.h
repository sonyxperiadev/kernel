/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */
/*
 * Copyright (C) 2015 Sony Mobile Communications Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation.
 */

#ifndef __QPNP_SMBCHARGER_EXTENSION_H
#define __QPNP_SMBCHARGER_EXTENSION_H

#ifdef CONFIG_QPNP_SMBCHARGER_EXTENSION
/*
 * somc_chg_apsd_rerun_request() - Request asynchronous APSD rerun
 *		The api is to perform APSD rerun.
 * @return:	0: queued successfully.
 */
int somc_chg_apsd_rerun_request(void);
#else
int somc_chg_apsd_rerun_request(void)
{
	return -ENODEV;
}
#endif

#endif
