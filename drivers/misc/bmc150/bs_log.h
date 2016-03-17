/*
 * (C) Copyright 2013 Bosch Sensortec GmbH All Rights Reserved
 *
 * This software program is licensed subject to the GNU General Public License
 * (GPL).Version 2,June 1991, available at http://www.fsf.org/copyleft/gpl.html
 *
 * @date        Apr 28th, 2011
 * @version     v1.0
 * @brief       Log API for Bosch MEMS Sensor drivers
 */

#ifndef __BS_LOG_H
#define __BS_LOG_H

#include <linux/kernel.h>

#define LOG_LEVEL_E 3
#define LOG_LEVEL_N 5
#define LOG_LEVEL_I 6
#define LOG_LEVEL_D 7

#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_LEVEL_I
#endif

#ifndef MODULE_TAG
#define MODULE_TAG "<>"
#endif

#if (LOG_LEVEL >= LOG_LEVEL_E)
#define PERR(fmt, args...) printk("[BMC150]" "[E]" KERN_ERR MODULE_TAG "<%s><%d>" fmt "\n", __FUNCTION__, __LINE__, ##args)
#else
#define PERR(fmt, args...)
#endif 

#if (LOG_LEVEL >= LOG_LEVEL_N)
#define PNOTICE(fmt, args...) printk("[BMC150]" "[N]" KERN_NOTICE MODULE_TAG "<%s><%d>" fmt "\n", __FUNCTION__, __LINE__, ##args)
#else
#define PNOTICE(fmt, args...)
#endif

#if (LOG_LEVEL >= LOG_LEVEL_I)
#define PINFO(fmt, args...) printk("[BMC150]" "[I]" KERN_INFO MODULE_TAG "<%s><%d>" fmt "\n", __FUNCTION__, __LINE__, ##args)
#else
#define PINFO(fmt, args...)
#endif

#if (LOG_LEVEL >= LOG_LEVEL_D)
#define PDEBUG(fmt, args...) printk("[BMC150]" "[D]" KERN_DEBUG MODULE_TAG "<%s><%d>" fmt "\n", __FUNCTION__, __LINE__, ##args)
#else
#define PDEBUG(fmt, args...)
#endif

#endif
