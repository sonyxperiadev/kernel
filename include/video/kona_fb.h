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

struct kona_fb_platform_data {
	char			*dispdrv_name;
	void *(*dispdrv_entry) (void);
	struct 	dispdrv_init_parms   parms;
};

extern void *DISP_DRV_NT35516_GetFuncTable(void);
extern void *DISP_DRV_LG4591_GetFuncTable(void);
extern void *DISP_DRV_OTM1281A_GetFuncTable(void);

struct dispdrv_name_entry {
	char name[10];
	void *(*entry) (void);
};

static struct dispdrv_name_entry dispdrvs[] = {
	{"NT35516", DISP_DRV_NT35516_GetFuncTable},
	{"LG4591", DISP_DRV_LG4591_GetFuncTable},
	{"OTM1281A", DISP_DRV_OTM1281A_GetFuncTable},
};
static void *get_dispdrv_entry(const char *name)
{
	int i;
	void *ret = NULL;
	i = sizeof(dispdrvs) / sizeof(struct dispdrv_name_entry);
	while (i--) {
		if (!strcmp(name, dispdrvs[i].name)) {
			ret = dispdrvs[i].entry;
			pr_err("Found %s\n", dispdrvs[i].name);
			break;
		}
	}
	return ret;
}

#endif /* KONA_FB_H_ */
