#ifndef __MMC328XMA_H__
#define __MMC328XMA_H__

#include <linux/ioctl.h>

#define MMC328XMA_IOCTL_BASE 'm'
#define MMC328XMA_SET_RANGE		_IOW(MMC328XMA_IOCTL_BASE, 1, int)
#define MMC328XMA_SET_MODE		_IOW(MMC328XMA_IOCTL_BASE, 2, int)
#define MMC328XMA_SET_BANDWIDTH		_IOW(MMC328XMA_IOCTL_BASE, 3, int)
#define MMC328XMA_READ_MAGN_XYZ		_IOW(MMC328XMA_IOCTL_BASE, 4, int)

/* Magnetometer Sensor Full Scale */
#define MMC328XMA_0_9G		0x00
#define MMC328XMA_1_2G		0x20
#define MMC328XMA_1_9G		0x40
#define MMC328XMA_2_5G		0x60
#define MMC328XMA_4_0G		0x80
#define MMC328XMA_4_6G		0xA0
#define MMC328XMA_5_5G		0xC0
#define MMC328XMA_7_9G		0xE0

/* Magnetic Sensor Operating Mode */
#define MMC328XMA_NORMAL_MODE	0x00
#define MMC328XMA_POS_BIAS	0x01
#define MMC328XMA_NEG_BIAS	0x02
#define MMC328XMA_CC_MODE	0x00
#define MMC328XMA_SC_MODE	0x01
#define HMC5843_IDLE_MODE	0x02
#define MMC328XMA_SLEEP_MODE	0x03

/* Magnetometer output data rate  */
#define MMC328XMA_ODR_75	0x00	/* 0.75Hz output data rate */
#define MMC328XMA_ODR1_5	0x04	/* 1.5Hz output data rate */
#define MMC328XMA_ODR3_0	0x08	/* 3Hz output data rate */
#define MMC328XMA_ODR7_5	0x0C	/* 7.5Hz output data rate */
#define MMC328XMA_ODR15		0x10	/* 15Hz output data rate */
#define MMC328XMA_ODR30		0x14	/* 30Hz output data rate */
#define MMC328XMA_ODR75		0x18	/* 75Hz output data rate */

#ifdef __KERNEL__

struct mmc328xma_platform_data {

	u8 h_range;

	u16 axis_map_x;
	u16 axis_map_y;
	u16 axis_map_z;

	u8 negate_x;
	u8 negate_y;
	u8 negate_z;

	int (*init)(void);
	void (*exit)(void);
	int (*power_on)(void);
	int (*power_off)(void);
};

#endif /* __KERNEL__ */

#endif  /* __MMC328XMA_H__ */
