/*
 *  FM Driver for Connectivity chip of Broadcom Corporation.
 *
 *  FM V4L2 module header.
 *
 *  Copyright (C) 2009 Texas Instruments
 *  Copyright (C) 2009-2014 Broadcom Corporation
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
/************************************************************************************
*
*  Filename:      fmdrv_v4l2.h
*
*  Description:   FM V4L2 module header.
*
***********************************************************************************/

#ifndef _FMDRV_V4L2_H
#define _FMDRV_V4L2_H

#include <media/v4l2-common.h>
#include <media/v4l2-ioctl.h>

int fm_v4l2_init_video_device(struct fmdrv_ops *, int);
void *fm_v4l2_deinit_video_device(void);

#endif
