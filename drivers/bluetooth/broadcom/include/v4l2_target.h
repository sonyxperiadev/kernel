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


/*****************************************************************************/

/*******************************************************************************
 *
 *  Filename:      v4l2_target.h
 *
 *  Description:   defines macros to enable/disable the following
 *                      [1] dynamic logging
 *                      [2] HCI snooping
 *
 ******************************************************************************/


 /* Macros to enable/disable debugging. This will overried debug macros in each of
   * V4L2 drivers. Set these to false for production release.*/
#ifndef BTDRV_DEBUG
#define BTDRV_DEBUG FALSE
#endif

#ifndef BTLDISC_DEBUG
#define BTLDISC_DEBUG FALSE
#endif

#ifndef V4L2_FM_DEBUG
#define V4L2_FM_DEBUG FALSE
#endif

/* set this to FALSE to disable HCI snooping for production release */
#ifndef V4L2_SNOOP_ENABLE
#define V4L2_SNOOP_ENABLE TRUE
#endif

#ifndef V4L2_ANT
#define V4L2_ANT TRUE
#endif

#ifndef V4L2_ANT_DEBUG
#define V4L2_ANT_DEBUG FALSE
#endif

