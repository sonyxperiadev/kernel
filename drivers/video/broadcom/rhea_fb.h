#ifndef __RHEA_FB_H__
#define __RHEA_FB_H__

#ifdef RHEA_FB_DEBUG
#define rheafb_debug(fmt, arg...)	\
	printk("%s:%d " fmt, __func__, __LINE__, ##arg)
#else
#define rheafb_debug(fmt, arg...)	\
	do {	} while (0)
#endif /* RHEA_FB_DEBUG */

#define rheafb_info(fmt, arg...)	\
	printk(KERN_INFO"%s:%d " fmt, __func__, __LINE__, ##arg)

#define rheafb_error(fmt, arg...)	\
	printk(KERN_ERR"%s:%d " fmt, __func__, __LINE__, ##arg)

#define rheafb_warning(fmt, arg...)	\
	printk(KERN_WARNING"%s:%d " fmt, __func__, __LINE__, ##arg)

#define rheafb_alert(fmt, arg...)	\
	printk(KERN_ALERT"%s:%d " fmt, __func__, __LINE__, ##arg)

#endif /* __RHEA_FB_H__ */
