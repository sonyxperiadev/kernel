/*******************************************************************************
* Copyright 2011 Broadcom Corporation.  All rights reserved.
*
*             @file     include/video/kona_fb_image_dump.h
*
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*******************************************************************************/
#ifndef _KONA_FB_IMAGE_DUMP_H_
#define _KONA_FB_IMAGE_DUMP_H_

#ifdef CONFIG_FB_BRCM_CP_CRASH_DUMP_IMAGE_SUPPORT
enum crash_dump_image_idx {
	GENERIC_DUMP_START = 0,
	CP_CRASH_DUMP_START,
	CP_RAM_DUMP_START,
	AP_CRASH_DUMP_START,
	AP_RAM_DUMP_START,
	GENERIC_DUMP_END,
	CP_CRASH_DUMP_END,
	AP_CRASH_DUMP_END,
};

extern void kona_display_crash_image(enum crash_dump_image_idx image_idx);
#endif

#endif /* _KONA_FB_IMAGE_DUMP_H_ */
