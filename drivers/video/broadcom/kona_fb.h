#ifndef __KONA_FB_H__
#define __KONA_FB_H__


#ifdef KONA_FB_DEBUG
#define konafb_debug(fmt, arg...)	\
	printk(KERN_INFO"%s:%d " fmt, __func__, __LINE__, ##arg)
#else
#define konafb_debug(fmt, arg...)	\
	do {	} while (0)
#endif /* KONA_FB_DEBUG */


#define konafb_info(fmt, arg...)	\
	printk(KERN_INFO"%s:%d " fmt, __func__, __LINE__, ##arg)

#define konafb_error(fmt, arg...)	\
	printk(KERN_ERR"%s:%d " fmt, __func__, __LINE__, ##arg)

#define konafb_warning(fmt, arg...)	\
	printk(KERN_WARNING"%s:%d " fmt, __func__, __LINE__, ##arg)

#define konafb_alert(fmt, arg...)	\
	printk(KERN_ALERT"%s:%d " fmt, __func__, __LINE__, ##arg)


extern void *DISP_DRV_GetFuncTable(void);
#ifdef CONFIG_FB_BRCM_CP_CRASH_DUMP_IMAGE_SUPPORT
extern int crash_dump_ui_on;
extern unsigned ramdump_enable;
#endif

#if defined(CONFIG_MACH_HAWAII_SS_LOGANDS) ||	\
	defined(CONFIG_MACH_HAWAII_SS_LOGAN)
#include "lcd/logan_nt35510.h"
#endif
#ifdef CONFIG_LCD_HX8369_CS02_SUPPORT
#include "lcd/hx8369_cs02.h"
#endif
#ifdef CONFIG_LCD_SC7798_CS02_SUPPORT
#include "lcd/sc7798_cs02.h"
#endif
#ifdef CONFIG_LCD_HX8369_SUPPORT
#include "lcd/hx8369.h"
#endif
#ifdef CONFIG_LCD_S6E63M0X_SUPPORT
#include "lcd/s6e63m0x3.h"
#endif

#include "lcd/nt35510.h"
#include "lcd/nt35512.h"
#include "lcd/nt35516.h"
#include "lcd/nt35517.h"
#include "lcd/otm1281a.h"
#include "lcd/otm1283a.h"
#include "lcd/otm8018b.h"
#include "lcd/otm8009a.h"
#include "lcd/ili9806c.h"
#include "lcd/hx8389b.h"
#include "lcd/hx8389_tm.h"
#include "lcd/hx8379_tm.h"
#include "lcd/simulator.h"
#include "lcd/s6e63m0x3.h"

static struct lcd_config *cfgs[] __initdata = {
#ifdef CONFIG_LCD_HX8369_SUPPORT
	&hx8369_cfg,
#endif
#ifdef CONFIG_LCD_HX8369_CS02_SUPPORT
	&hx8369_cfg,
#endif
#ifdef CONFIG_LCD_SC7798_CS02_SUPPORT
	&sc7798_cfg,
#endif
	&nt35512_cfg,
	&nt35516_cfg,
#ifdef CONFIG_LCD_S6E63M0X_SUPPORT
	&s6e63m0x3_cfg,
#endif

	&nt35510_cfg,
	&nt35517_cfg,
	&otm1281a_cfg,
	&otm1283a_cfg,
	&otm8018b_cfg,
	&otm8009a_cfg,
	&ili9806c_cfg,
	&hx8389b_cfg,
	&hx8389_tm_cfg,
	&hx8379_tm_cfg,
	&simulator_cfg,
};

static struct lcd_config * __init get_dispdrv_cfg(const char *name)
{
	int i;
	void *ret = NULL;
	i = sizeof(cfgs) / sizeof(struct lcd_config *);
	while (i--) {
		if (!strcmp(name, cfgs[i]->name)) {
			ret = cfgs[i];
			pr_err("Found a match for %s\n", cfgs[i]->name);
			break;
		}
	}
	return ret;
}

#endif /* __KONA_FB_H__ */
