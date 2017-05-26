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
 *  Filename:      v4l2_logs.h
 *
 *  Description:   defines flags which enable various types of logs.
 *
 ******************************************************************************/


#define TRUE 1
#define FALSE 0

#define V4L2_DBG_INIT     (1 << 0) /* enable logs for init and release in drivers */
#define V4L2_DBG_OPEN     (1 << 1) /* enable logs for open call to drivers */
#define V4L2_DBG_CLOSE    (1 << 2) /* enable logs for close call to drivers */
#define V4L2_DBG_TX       (1 << 3) /* enable logs for tx in drivers */
#define V4L2_DBG_RX       (1 << 4) /* enable logs for rx in drivers */



