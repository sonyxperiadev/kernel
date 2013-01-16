/*****************************************************************************
* Copyright 2009 - 2010 Broadcom Corporation.  All rights reserved.
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
#ifndef KONA_FB_H_
#define KONA_FB_H_

#define DISPDRV_NAME_SZ 20
#define REG_NAME_SZ DISPDRV_NAME_SZ

struct hw_rst_info {
	uint32_t gpio;	/* gpio number */
	uint32_t setup;	/* us */
	uint32_t pulse;	/* us */
	uint32_t hold;	/* us */
	bool active;/* low or high */
};

struct kona_fb_platform_data {
	char name[DISPDRV_NAME_SZ];
	char reg_name[REG_NAME_SZ];
	struct hw_rst_info rst;
	bool vmode;
	bool vburst;
	bool cmnd_LP;
	bool te_ctrl;
	uint8_t col_mod_i;
	uint8_t col_mod_o;
	uint16_t width;
	uint16_t height;
	uint8_t fps;
	uint8_t lanes;
	uint32_t hs_bps;
	uint32_t lp_bps;
};

#endif /* KONA_FB_H_ */
