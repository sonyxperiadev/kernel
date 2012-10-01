/*****************************************************************************
*  Copyright 2011 Broadcom Corporation.  All rights reserved.
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2, available at
*  http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
*****************************************************************************/

#if !defined( AAA_IOCTL_H )
#define AAA_IOCTL_H

/* ---- Include Files ---------------------------------------------------- */

#if defined( __KERNEL__ )
#include <linux/types.h>	/* Needed for standard types */
#else
#include <stdint.h>
#endif

#include <linux/ioctl.h>

/* ---- Constants and Types ---------------------------------------------- */

/* Type define used to create unique IOCTL number */
#define AAA_MAGIC_TYPE                  'Z'

/* IOCTL commands */
enum aaa_cmd_e {
	AAA_CMD_SET_WRT_CODEC = 0x15,	/* Start at 0x15 arbitrarily */
	AAA_CMD_DSC_WRT_CODEC,
	AAA_CMD_SET_RD_CODEC,
	AAA_CMD_LAST		/* Do no delete */
};

/* IOCTL Data structures */
struct aaa_ioctl_setwrtcodec {
	char name[32];		/*<< (i) name of the codec to use */

};

struct aaa_ioctl_setrdcodec {
	char name[32];		/*<< (i) name of the codec to use */

};

/* IOCTL numbers */
#define AAA_IOCTL_SET_TX_CODEC       _IOR( AAA_MAGIC_TYPE, AAA_CMD_SET_WRT_CODEC, struct aaa_ioctl_setwrtcodec )
#define AAA_IOCTL_DSC_TX_CODEC       _IO( AAA_MAGIC_TYPE, AAA_CMD_DSC_WRT_CODEC )
#define AAA_IOCTL_SET_RX_CODEC       _IOR( AAA_MAGIC_TYPE, AAA_CMD_SET_RD_CODEC, struct aaa_ioctl_setrdcodec )

/* ---- Variable Externs ------------------------------------------ */

/* ---- Function Prototypes --------------------------------------- */

#endif /* AAA_IOCTL_H */
