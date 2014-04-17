/*****************************************************************************
* Copyright 2004 - 2009 Broadcom Corporation.  All rights reserved.
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

/*
 * Public header of the Board Specific Configuration (BSC) driver. The driver
 * provides a "register" and "query" mechanism for users to pass BSC
 * parameters
 */

#ifndef _BSC_H_
#define _BSC_H_

#include <linux/ioctl.h>

#define BSC_NAME_LEN    25

typedef struct {
	char name[BSC_NAME_LEN];
	void *data;
	int len;
} BSC_PARAM_T;

#define BSC_MAGIC    'B'

#define BSC_CMD_REGISTER         0x80
#define BSC_CMD_UNREGISTER       0x81
#define BSC_CMD_QUERY            0x82

#define BSC_IOCTL_REGISTER       _IOW(BSC_MAGIC, BSC_CMD_REGISTER, BSC_PARAM_T)
#define BSC_IOCTL_UNREGISTER     _IOW(BSC_MAGIC, BSC_CMD_UNREGISTER, char)	/* arg is char[] */
#define BSC_IOCTL_QUERY          _IOWR(BSC_MAGIC, BSC_CMD_QUERY, BSC_PARAM_T)

/*
 * Routine that is used to register BSC data
 */
int bsc_register(const char *name, void *data, int len);

/*
 * Routine that is used to unregister the BSC data
 */
int bsc_unregister(const char *name);

/*
 * Routine that is used to query the BSC data. Note the memory that 'data'
 * points to should be at least 'len' bytes
 */
int bsc_query(const char *name, void *data, int len);

#endif /* _BSC_H_ */
