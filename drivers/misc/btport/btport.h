/*****************************************************************************
 * Copyright 2010-2012 Broadcom Corporation.  All rights reserved.
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
 * FILE FUNCTION: BTLINUXPORT Driver External Definitions
 *
 * REVISION HISTORY:
 */
#ifndef _BTLINUXPORT_
#define _BTLINUXPORT_

#define BTLINUX_PORT_OPENED         0
#define BTLINUX_PORT_CLOSED         1
#define BTLINUX_PORT_ENABLE_MS      2
#define BTLINUX_PORT_SET_BREAK_ON   3
#define BTLINUX_PORT_SET_BREAK_OFF  4
#define BTLINUX_PORT_MODEM_CONTROL  5
#define BTLINUX_PORT_MODEM_STATUS   6
#define BTLINUX_PORT_TX_EMPTY       7

#define MODEM_CNTRL_DTRDSR_MASK        0x0001
#define MODEM_CNTRL_CTSRTS_MASK        0x0002
#define MODEM_CNTRL_RI_MASK            0x0004
#define MODEM_CNTRL_CD_MASK            0x0008

/* rfcomm application side signal bit fields */
#define PORT_DTRDSR_ON          0x01
#define PORT_CTSRTS_ON          0x02
#define PORT_RING_ON            0x04
#define PORT_DCD_ON             0x08

#ifndef BTLINUX_CUSTOM_MODEM_SIGNALS
/* by default use official CD signals TIOCM_CD TIOCM_CAR */
#define BTLINUX_TIOCM_CD 0x040
#else
/* some platform use CD (DCE output signal) on differenct TIOCM bit */
#ifndef BTLINUX_TIOCM_CD
/* TIOCM_OUT1 some customers use this for CD */
#define BTLINUX_TIOCM_CD 0x2000
#endif
#endif

/*
 * ioctl defs
 */
#define IOCTL_BTPORT_GET_EVENT            0x1001
#define IOCTL_BTPORT_SET_EVENT_RESULT     0x1002
#define IOCTL_BTPORT_HANDLE_MCTRL         0x1003
#define IOCTL_BTPORT_CREATE_DEVICE        0x1004
#define IOCTL_BTPORT_RELEASE_DEVICE       0x1005
#define IOCTL_BTPORT_LIST_DEVICE          0x1006
#define IOCTL_BTPORT_GET_DEVICE_INFO      0x1007

#define IOCTL_RFCOMMCREATEDEV 0x100f
#endif
