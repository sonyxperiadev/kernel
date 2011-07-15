/*****************************************************************************
* Copyright 2004 - 2008 Broadcom Corporation.  All rights reserved.
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
*  headset.h
*
*  PURPOSE:
*
*  This file defines the interface to the HEADSET driver.
*
*  NOTES:
*
*****************************************************************************/


#if !defined( LINUX_HEADSET_H )
#define LINUX_HEADSET_H

/* ---- Include Files ---------------------------------------------------- */

#include <linux/ioctl.h>

typedef enum
{
    HEADSET_UNPLUGGED,      /* headset unplugged */
    HEADSET_TOGGLE_A,       /* headset plugged in, toggle state A */
    HEADSET_TOGGLE_B        /* headset plugged in, toggle state B */

} headset_state;

#define HEADSET_STATE_INIT (headset_state)-1

#ifdef __KERNEL__
typedef enum
{
    HEADSET_NULL = 0,       /* NULL event */
    HEADSET_REMOVED,        /* headset unplugged */
    HEADSET_INSERTED,       /* headset inserted */
    HEADSET_BUTTON          /* headset button pressed */

} headset_event;
#endif

/* ---- Constants and Types ---------------------------------------------- */

#define HEADSET_MAGIC   'h'

#define HEADSET_CMD_FIRST               0x80
#define HEADSET_CMD_GET_STATE           0x80
#define HEADSET_CMD_GET_DEBOUNCE        0x81
#define HEADSET_CMD_SET_DEBOUNCE        0x82
#define HEADSET_CMD_LAST                0x82

#define HEADSET_IOCTL_GET_STATE _IOR( HEADSET_MAGIC, HEADSET_CMD_GET_STATE, headset_state )
#define HEADSET_IOCTL_GET_DEBOUNCE _IOR( HEADSET_MAGIC, HEADSET_CMD_GET_DEBOUNCE, int )
#define HEADSET_IOCTL_SET_DEBOUNCE _IOR( HEADSET_MAGIC, HEADSET_CMD_SET_DEBOUNCE, int )

/* ---- Variable Externs ------------------------------------------------- */
/* ---- Function Prototypes ---------------------------------------------- */

#define HEADSET_DEBOUNCE_DEFAULT        50000

#endif  /* LINUX_HEADSET_H */
