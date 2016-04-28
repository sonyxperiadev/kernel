/*****************************************************************************
* Copyright 2011 Broadcom Corporation.  All rights reserved.
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
*
*****************************************************************************
*
*  hdmi.h
*
*  PURPOSE:
*
*  This file defines the interface to the HDMI hotplug detect driver.
*
*  NOTES:
*
*****************************************************************************/

#if !defined( LINUX_HDMI_H )
#define LINUX_HDMI_H

/* ---- Include Files ---------------------------------------------------- */

#include <linux/ioctl.h>

/* FIXME - handle hotplug assert pulse from connected state */

typedef enum {
	HDMI_UNPLUGGED = 0,	/* hdmi unplugged */
	HDMI_CONNECTED,		/* hdmi connected */
	HDMI_WIFI,		/* hdmi over wifi */

} hdmi_state;

#define HDMI_STATE_INIT (hdmi_state)-1

/* ---- Constants and Types ---------------------------------------------- */

#define HDMI_MAGIC   'h'

typedef enum {
	HDMI_CMD_GET_STATE = 0x80,	/* Arbitrary start. */
	HDMI_CMD_SET_SWITCH,

} hdmi_ioctl_t;

#define HDMI_IOCTL_GET_STATE  _IOR( HDMI_MAGIC, HDMI_CMD_GET_STATE, hdmi_state )
#define HDMI_IOCTL_SET_SWITCH _IOR( HDMI_MAGIC, HDMI_CMD_SET_SWITCH, hdmi_state )

/* ---- Variable Externs ------------------------------------------------- */
/* ---- Function Prototypes ---------------------------------------------- */

#if defined(__KERNEL__)
int hdmi_set_wifi_hdmi(int enable);
#endif

#endif /* LINUX_HDMI_H */
