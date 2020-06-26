// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *  Copyright (C) 2012-2013 Samsung Electronics Co., Ltd.
 */

#ifndef _EXFAT_CONFIG_H
#define _EXFAT_CONFIG_H

//#define CONFIG_EXFAT_TRACE_IO
//#define CONFIG_EXFAT_TRACE_LOCK /* Trace elapsed time in lock_super(sb) */

#ifndef CONFIG_EXFAT_WRITE_SB_INTERVAL_CSECS
#define CONFIG_EXFAT_WRITE_SB_INTERVAL_CSECS	(dirty_writeback_interval)
#endif

#ifndef CONFIG_EXFAT_DEFAULT_CODEPAGE /* if Kconfig lacked codepage */
#define CONFIG_EXFAT_DEFAULT_CODEPAGE   437
#endif

#ifndef CONFIG_EXFAT_DEFAULT_IOCHARSET /* if Kconfig lacked iocharset */
#define CONFIG_EXFAT_DEFAULT_IOCHARSET  "utf8"
#endif

#ifndef CONFIG_EXFAT_FAT_MIRRORING /* if Kconfig lacked fat-mirroring option */
#define CONFIG_EXFAT_FAT_MIRRORING /* Write FAT 1, FAT 2 simultaneously */
#endif

#ifndef CONFIG_EXFAT_VIRTUAL_XATTR
#define CONFIG_EXFAT_VIRTUAL_XATTR
#endif

#endif /* _EXFAT_CONFIG_H */
