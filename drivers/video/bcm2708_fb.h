#ifndef __BCM2708_FB_H
#define __BCM2708_FB_H

/* macros */
#define MAKEFOURCC(ch0, ch1, ch2, ch3) ((unsigned int)(unsigned char)(ch0) \
		| ((unsigned int)(unsigned char)(ch1) << 8) \
		| ((unsigned int)(unsigned char)(ch2) << 16) \
		| ((unsigned int)(unsigned char)(ch3) << 24 ))

#define FBUF_FOURCC	MAKEFOURCC('F', 'B', 'U', 'F')

#define bcm2708fb_debug(fmt, arg...)	\
	printk(KERN_DEBUG"%s:%d " fmt, __func__, __LINE__, ##arg)

#define bcm2708fb_info(fmt, arg...)	\
	printk(KERN_INFO"%s:%d " fmt, __func__, __LINE__, ##arg)

#define bcm2708fb_error(fmt, arg...)	\
	printk(KERN_ERR"%s:%d " fmt, __func__, __LINE__, ##arg)

#define bcm2708fb_warning(fmt, arg...)	\
	printk(KERN_WARNING"%s:%d " fmt, __func__, __LINE__, ##arg)

#define bcm2708fb_alert(fmt, arg...)	\
	printk(KERN_ALERT"%s:%d " fmt, __func__, __LINE__, ##arg)

#define FB_CONTROL			(0x00)
	#define	CTRL_ENABLE_SHIFT	(0)
	#define CTRL_ENABLE_MASK	(0x00000001)
	#define CTRL_DISP_EN_SHIFT	(1)
	#define CTRL_DISP_EN_MASK	(0x00000002)
	#define CTRL_BUFF_IDX_SHIFT	(8)
	#define CTRL_BUFF_IDX_MASK	(0x00000f00)

#define FB_STATUS			(0x04)	/* read only */
	#define STAT_EN_SHIFT		(0)
	#define STAT_EN_MASK		(0x00000001)
	#define STAT_DISP_SHIFT		(1)
	#define STAT_DISP_MASK		(0x00000002)
	#define STAT_BUFF_IDX_SHIFT	(8)
	#define STAT_BUFF_IDX_MASK	(0x00000f00)
	#define	STAT_UPDATE_COUNT_SHIFT	(16)
	#define STAT_UPDATE_COUNT_MASK	(0xffff0000)

#define FB_WIDTH			(0x20)	/* Should be read only?? */

#define FB_HEIGHT			(0x24)	/* Should be read only?? */

#define FB_SET_FORMAT			(0x28)
	#define FB_FORMAT_RGB565	(0)

#define FB_SET_NUM_BUFFERS		(0x2C)

#define FB_SET_DISPLAY			(0x40)

#define FB_GET_BUFFER_PHYS		(0x60)	/* read only */

#define FB_TARGET_LAYER			(0x44)

/* These are not used by the driver yet : ssp */
#define FB_TARGET_X			(0x48)
#define FB_TARGET_Y			(0x4C)
#define FB_TARGET_WIDTH			(0x50)
#define FB_TARGET_HEIGHT		(0x54)


/* Register values used by the driver .. */
#define FRAMEBUFFER_TARGET_DISPLAY	(0)
#define FRAMEBUFFER_WIDTH		(640)
#define FRAMEBUFFER_HEIGHT		(480)
#define FRAMEBUFFER_NUM_BUFFERS		(2)


#endif /* __BCM2708_FB_H */
