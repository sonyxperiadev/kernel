/*
 * emmc_rpmb.h Interface for eMMC RPMB driver
 * Copyright (c) 2010 - 2012 Broadcom Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __EMMC_RPMB_H
#define __EMMC_RPMB_H

extern int readCKDataBlock(char *buff, int len);
extern int read_imei1(char *imei1, char *imei_mac1,
					int imei1_len, int imei_mac1_len);
extern int read_imei2(char *imei2, char *imei_mac2,
					int imei2_len, int imei_mac2_len);
extern int readkeyboxKey(char *buff, int len);
extern int readkeyboxData(char *buff, int len);
extern int writekeyboxData(char *buff, int len);

#endif /* __EMMC_RPMB_H */
