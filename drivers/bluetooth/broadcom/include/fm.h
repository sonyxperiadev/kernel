/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.


 *  Copyright (C) 2009-2014 Broadcom Corporation
 */


/*****************************************************************************
**
**  Name:           fm.h
**
**  Description:    FM common header file between ldisc and protocol FM drivers
**
*****************************************************************************/
#ifndef _FM_H
#define _FM_H

struct fm_event_hdr {
    unsigned char event;
    unsigned char plen;
} __attribute__ ((packed));

#define FM_MAX_FRAME_SIZE 0xFF
#define FM_EVENT_HDR_SIZE 2
#define FM_CH8_PKT 0x8

#endif
