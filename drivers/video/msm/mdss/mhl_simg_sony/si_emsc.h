/*
 * SiI8620 Linux Driver
 *
 * Copyright (C) 2013 Silicon Image, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2.
 * This program is distributed AS-IS WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; INCLUDING without the implied warranty
 * of MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE or NON-INFRINGEMENT.
 * See the GNU General Public License for more details at
 * http://www.gnu.org/licenses/gpl-2.0.html.
 */

/* vendor/semc/hardware/mhl/mhl_sii8620_8061_drv/si_emsc.h
 *
 * Copyright (C) 2013 Sony Mobile Communications AB.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef _SI_EMSC_H_
#define _SI_EMSC_H_

#define EMSC_RCV_BUFFER_DEFAULT		256
#define EMSC_BLK_MAX_LENGTH		256
#define EMSC_BLK_STD_HDR_LEN		2
#define EMSC_BLK_CMD_MAX_LEN		(EMSC_BLK_MAX_LENGTH - \
					EMSC_BLK_STD_HDR_LEN)

#endif /* #ifndef _SI_EMSC_H_ */
