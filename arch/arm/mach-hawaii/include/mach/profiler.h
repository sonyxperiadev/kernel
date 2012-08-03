/*****************************************************************************
*
* Kona profiler framework
* @file profiler.h
*
* Copyright 2012 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/
#ifndef _PROFILER_H
#define _PROFILER_H

/**
 * maximum length of profiler name
 * (see macro PROFILER_NAME() in plat/profiler.h
 */
#define PROF_NAME_MAX_LEN			(16)

/**
 * Size of the profiler log buffer
 */
#define PROF_CIRC_BUFF_MAX_ENTRIES		(256)
#endif


