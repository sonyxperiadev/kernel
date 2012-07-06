/* Copyright (c) 2009, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef __ASM__ARCH_OEM_RAPI_CLIENT_H
#define __ASM__ARCH_OEM_RAPI_CLIENT_H

/*
 * OEM RAPI CLIENT Driver header file
 */

#include <linux/types.h>
#include <mach/msm_rpcrouter.h>

enum {
	OEM_RAPI_CLIENT_EVENT_NONE = 0,

	/*
	 * list of oem rapi client events
	 */

	/* constants used by charger driver */
	OEM_RAPI_CLIENT_EVENT_BATT_MV_GET,
	OEM_RAPI_CLIENT_EVENT_BATT_MA_GET,
	OEM_RAPI_CLIENT_EVENT_BATT_ID_GET,
	OEM_RAPI_CLIENT_EVENT_BATT_ID_TYPE1_GET,
	OEM_RAPI_CLIENT_EVENT_BATT_ID_TYPE2_GET,
	OEM_RAPI_CLIENT_EVENT_CUTOFF_LEVEL_CB_REGISTER,
	OEM_RAPI_CLIENT_EVENT_CUTOFF_LEVEL_CB_UNREGISTER_SET,
	OEM_RAPI_CLIENT_EVENT_NOTIFY_PLATFORM_SET,
	OEM_RAPI_CLIENT_EVENT_NOTIFY_BDATA_CB_REGISTER_SET,
	OEM_RAPI_CLIENT_EVENT_NOTIFY_BDATA_CB_UNREGISTER_SET,
	OEM_RAPI_CLIENT_EVENT_PM_BATT_FET_SWITCH_SET,
	/* end of constants used by charger driver */

	OEM_RAPI_CLIENT_EVENT_MAX

};

/*
 * This enum lists the events that the server can notify the client of.
 */
enum {
	OEM_RAPI_SERVER_EVENT_NONE = 0,

	/*
	 * list of oem rapi server events
	 */

	/* constants used by charger driver */
	OEM_RAPI_SERVER_EVENT_CUTOFF_CB_EVENT,
	OEM_RAPI_SERVER_EVENT_NOTIFY_BDATA_CB_EVENT,
	/* end of constants used by charger driver */

	OEM_RAPI_SERVER_EVENT_MAX
};

struct oem_rapi_client_streaming_func_cb_arg {
	uint32_t  event;
	void      *handle;
	uint32_t  in_len;
	char      *input;
	uint32_t out_len_valid;
	uint32_t output_valid;
	uint32_t output_size;
};

struct oem_rapi_client_streaming_func_cb_ret {
	uint32_t *out_len;
	char *output;
};

struct oem_rapi_client_streaming_func_arg {
	uint32_t event;
	int (*cb_func)(struct oem_rapi_client_streaming_func_cb_arg *,
		       struct oem_rapi_client_streaming_func_cb_ret *);
	void *handle;
	uint32_t in_len;
	char *input;
	uint32_t out_len_valid;
	uint32_t output_valid;
	uint32_t output_size;
};

struct oem_rapi_client_streaming_func_ret {
	uint32_t *out_len;
	char *output;
};

int oem_rapi_client_streaming_function(
	struct msm_rpc_client *client,
	struct oem_rapi_client_streaming_func_arg *arg,
	struct oem_rapi_client_streaming_func_ret *ret);

int oem_rapi_client_close(void);

struct msm_rpc_client *oem_rapi_client_init(void);

#endif
