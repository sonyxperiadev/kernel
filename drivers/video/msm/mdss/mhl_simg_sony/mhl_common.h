/* vendor/semc/hardware/mhl/mhl_sii8620_8061_drv/mhl_common.h
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

#include <linux/mhl/mhl.h>

#include "si_8620_regs.h"

/*
 * Used in the unit test build
 */
#ifdef UNIT_TEST
#define static
#endif

/* edid read printing with pr_info level*/
#define EDID_DATA_DEBUG_PRINT

/* dump all registers */
/* #ifdef DUMP_ALL_REGS */

/* no use */
#if 0
/* HeartBeat */
/* send periodic GET_STATE and disconnect on heartbeat failure */
#define DISCONN_HEARTBEAT    2
/* send periodic GET_STATE */
#define ISSUE_HEARTBEAT      1
/* do not send periodic GET_STATE */
#define DISABLE_HEARTBEAT    0
#define HEARTBEAT DISABLE_HEARTBEAT
#endif

/* EMSC */
/* #define EMSC */

/*
 * other
 */
#define SI_PACK_THIS_STRUCT __attribute__((__packed__))
#define MHL_DDC_RESET

#endif
