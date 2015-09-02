/* kernel/drivers/video/msm/mdss/mhl_sii8620_8061_drv/mhl_common.h
 *
 * Copyright (C) 2013 Sony Mobile Communications AB.
 * Copyright (C) 2013 Silicon Image Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef __MHL_COMMON_H
#define __MHL_COMMON_H

#include <linux/types.h>

/*
 * Used in the unit test build
 */
#ifdef UNIT_TEST
#define static
#endif


/*
 * Debug print
 */
/* print all debug info */
/* #define DEBUG_PRINT */

#ifdef DEBUG_PRINT
#undef pr_debug
#define pr_debug pr_info
#endif

/* edid read printing with pr_info level*/
#define EDID_DATA_DEBUG_PRINT

/*
 * FIXME
 */
#define EMSC_WORKAROUND
/* #define HEARTBEAT */
/*
 * other
 */
#define MHL_UCHAR_MAX 255

#define SI_PACK_THIS_STRUCT __attribute__((__packed__))
#define MHL_DDC_RESET

enum {
	MHL_SUCCESS = 0,
	MHL_FAIL = -1
};

#endif
