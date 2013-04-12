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


/*upgrade config of FT6X06*/
#define FT6X06_UPGRADE_AA_DELAY    50
#define FT6X06_UPGRADE_55_DELAY    30
#define FT6X06_UPGRADE_ID_1    0x79
#define FT6X06_UPGRADE_ID_2    0x08
#define FT6X06_UPGRADE_READID_DELAY    10
#define FT6X06_UPGRADE_EARSE_DELAY    2000

#define FTS_PACKET_LENGTH    128
#define FTS_SETTING_BUF_LEN    128

#define FTS_UPGRADE_LOOP    3

#define FTS_FACTORYMODE_VALUE    0x40
#define FTS_WORKMODE_VALUE    0x00

#define FTS_DBG
#ifdef FTS_DBG
#define DBG(fmt, args...) printk(KERN_DEBUG "[FTS]" fmt, ## args)
#else
#define DBG(fmt, args...) do {} while (0)
#endif


#endif
