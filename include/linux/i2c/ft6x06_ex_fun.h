#ifndef __LINUX_FT6X06_EX_FUN_H__
#define __LINUX_FT6X06_EX_FUN_H__

#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/semaphore.h>
#include <linux/mutex.h>
#include <linux/interrupt.h>
#include <mach/irqs.h>

#include <linux/syscalls.h>
#include <asm/unistd.h>
#include <asm/uaccess.h>
#include <linux/fs.h>
#include <linux/string.h>

#define FT_UPGRADE_AA    0xAA
#define FT_UPGRADE_55    0x55

/*upgrade config of FT5316*/
#define FT5316_UPGRADE_AA_DELAY    50
#define FT5316_UPGRADE_55_DELAY    30
#define FT5316_UPGRADE_ID_1    0x79
#define FT5316_UPGRADE_ID_2    0x07
#define FT5316_UPGRADE_READID_DELAY    10
#define FT5316_UPGRADE_EARSE_DELAY    2000

/*upgrade config of FT5306*/
#define FT5306_UPGRADE_ID_1    0x79
#define FT5306_UPGRADE_ID_2    0x03

/*upgrade config of FT5336*/
#define FT5336_UPGRADE_ID_1    0x79
#define FT5336_UPGRADE_ID_2    0x11


/*upgrade config of FT6X06*/
#define FT6X06_UPGRADE_AA_DELAY    50
#define FT6X06_UPGRADE_55_DELAY    10
#define FT6X06_UPGRADE_ID_1    0x79
#define FT6X06_UPGRADE_ID_2    0x08
#define FT6X06_UPGRADE_READID_DELAY    10
#define FT6X06_UPGRADE_EARSE_DELAY    2000

#define FTS_PACKET_LENGTH    128
#define FTS_SETTING_BUF_LEN    128

#define FTS_UPGRADE_LOOP    10

#define FTS_FACTORYMODE_VALUE    0x40
#define FTS_WORKMODE_VALUE    0x00

#if defined(CONFIG_TOUCHSCREEN_HAWAII_GARNET_FT5306_FW)
#define FTS_UPGRADE_ID_1 FT5306_UPGRADE_ID_1
#define FTS_UPGRADE_ID_2 FT5306_UPGRADE_ID_2
#define FTS_UPGRADE_REG  0xfc
#define NEED_CALIBRATION  1

#elif defined(CONFIG_TOUCHSCREEN_W68_FT6306_FW)
#define FTS_UPGRADE_ID_1 FT6X06_UPGRADE_ID_1
#define FTS_UPGRADE_ID_2 FT6X06_UPGRADE_ID_2
#define FTS_UPGRADE_REG  0xbc
#define NEED_CALIBRATION  0

#elif defined(CONFIG_TOUCHSCREEN_W81_FT6306_FW)
#define FTS_UPGRADE_ID_1 FT6X06_UPGRADE_ID_1
#define FTS_UPGRADE_ID_2 FT6X06_UPGRADE_ID_2
#define FTS_UPGRADE_REG  0xbc
#define NEED_CALIBRATION  0

#elif defined(CONFIG_TOUCHSCREEN_5606_FT6306_FW)
#define FTS_UPGRADE_ID_1 FT6X06_UPGRADE_ID_1
#define FTS_UPGRADE_ID_2 FT6X06_UPGRADE_ID_2
#define FTS_UPGRADE_REG  0xbc
#define NEED_CALIBRATION  0


#elif defined(CONFIG_TOUCHSCREEN_5606_FT5316_FW)
#define FTS_UPGRADE_ID_1 FT5316_UPGRADE_ID_1
#define FTS_UPGRADE_ID_2 FT5316_UPGRADE_ID_2
#define FTS_UPGRADE_REG  0xfc
#define NEED_CALIBRATION  1

#elif defined(CONFIG_TOUCHSCREEN_JAVA_AMETHYST_FT5336_FW)
#define FTS_UPGRADE_ID_1 FT5336_UPGRADE_ID_1
#define FTS_UPGRADE_ID_2 FT5336_UPGRADE_ID_2
#define FTS_UPGRADE_REG  0xfc
#define NEED_CALIBRATION  1

#else
#define FTS_UPGRADE_ID_1 FT5306_UPGRADE_ID_1
#define FTS_UPGRADE_ID_2 FT5306_UPGRADE_ID_2
#define FTS_UPGRADE_REG  0xfc
#define NEED_CALIBRATION  1
#endif

#define FTS_DBG
#ifdef FTS_DBG
#define DBG(fmt, args...) printk(KERN_DEBUG "[FTS]" fmt, ## args)
#else
#define DBG(fmt, args...) do {} while (0)
#endif


#endif
