/*****************************************************************************
* Copyright 2009 - 2011 Broadcom Corporation.  All rights reserved.
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
/**
*
*  @file    amxr_port_ioctl.h
*
*  @brief   Audio Mixer Port Proxy User IOCTL definitions
*
*****************************************************************************/
#if !defined( AMXR_PORT_IOCTL_H )
#define AMXR_PORT_IOCTL_H

/* ---- Include Files ---------------------------------------------------- */
#include <linux/ioctl.h>
#include <linux/broadcom/amxr.h>
#include <linux/broadcom/amxr_port.h>

/* ---- Constants and Types ---------------------------------------------- */

/* Type define used to create unique IOCTL number */
#define AMXR_PORT_MAGIC			'N'

/* IOCTL commands */
enum amxr_port_cmd_e {
	AMXR_PORT_CMD_CREATE_PORT = 0x30,	/* Start at 0x30 arbitrarily */
	AMXR_PORT_CMD_SET_PORT_DST_FREQ,
	AMXR_PORT_CMD_SET_PORT_SRC_FREQ,
	AMXR_PORT_CMD_SET_PORT_DST_CHANS,
	AMXR_PORT_CMD_SET_PORT_SRC_CHANS,
	AMXR_PORT_CMD_GET_MSG,
	AMXR_PORT_CMD_RELEASE,
	AMXR_PORT_CMD_LAST	/* Do no delete */
};

/* IOCTL Data structures */
struct amxr_port_ioctl_createport {
	char name[32];		/**< (i) Name string */
	AMXR_PORT_CB cb;	/**< (i) Callbacks */
	int dst_hz;		/**< (i) Current destination sampling frequency in Hz */
	int dst_chans;		/**< (i) Number of channels, i.e. mono = 1 */
	int dst_bytes;		/**< (i) Destination period size in bytes */
	int src_hz;		/**< (i) Current source sampling frequency in Hz */
	int src_chans;		/**< (i) Number of channels, i.e. mono = 1 */
	int src_bytes;		/**< (i) Source period size in bytes */
};

struct amxr_port_ioctl_setportfreq {
	int hz;			/**< (i) Sampling frequency in Hz */
	int bytes;		/**< (i) Period size in bytes */
};

struct amxr_port_ioctl_setportchans {
	int chans;		/**< (i) Number of channels: 1 for mono, 2 for stereo, etc */
	int bytes;		/**< (i) Period size in bytes */
};

/* Message definitions */

enum amxr_port_msgid {
	AMXR_PORT_SRCDONE_MSG = 100,
	AMXR_PORT_DSTDONE_MSG,
	AMXR_PORT_DSTCNXSREMOVED_MSG,
};

/* Defines message structure used to transfer information
 * from kernel to user ports. This structure should be as
 * small as possible to limit the amount of memory copies.
 */
struct amxr_port_msg {
	int msgid;		/* Message ID */
	long buf_pgoff;		/* Buffer page offset */
	int bytes;		/* Size of buffer in bytes */
};

/* IOCTL numbers */
#define AMXR_PORT_IOCTL_CREATE_PORT _IOR( AMXR_PORT_MAGIC, AMXR_PORT_CMD_CREATE_PORT, struct amxr_port_ioctl_createport )
#define AMXR_PORT_IOCTL_SET_PORT_DST_FREQ _IOR( AMXR_PORT_MAGIC, AMXR_PORT_CMD_SET_PORT_DST_FREQ, struct amxr_port_ioctl_setportfreq )
#define AMXR_PORT_IOCTL_SET_PORT_SRC_FREQ _IOR( AMXR_PORT_MAGIC, AMXR_PORT_CMD_SET_PORT_SRC_FREQ, struct amxr_port_ioctl_setportfreq )
#define AMXR_PORT_IOCTL_SET_PORT_DST_CHANS _IOR( AMXR_PORT_MAGIC, AMXR_PORT_CMD_SET_PORT_DST_CHANS, struct amxr_port_ioctl_setportchans )
#define AMXR_PORT_IOCTL_SET_PORT_SRC_CHANS _IOR( AMXR_PORT_MAGIC, AMXR_PORT_CMD_SET_PORT_SRC_CHANS, struct amxr_port_ioctl_setportchans )
#define AMXR_PORT_IOCTL_GET_MSG _IO( AMXR_PORT_MAGIC, AMXR_PORT_CMD_GET_MSG )
#define AMXR_PORT_IOCTL_RELEASE _IO( AMXR_PORT_MAGIC, AMXR_PORT_CMD_RELEASE )

/* ---- Variable Externs ------------------------------------------ */
/* ---- Function Prototypes --------------------------------------- */

#endif /* AMXR_PORT_IOCTL_H */
