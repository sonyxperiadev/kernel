/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*       @file   include/linux/broadcom/ckblock_reader.h
*
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

#ifndef __CKBLOCK_READER_H
#define __CKBLOCK_READER_H

/**
 * @brief 	readCKDataBlock : Reads the CK Data block from the MTD device.
 * @param	buff 	: Kernel Space buffer to which the data will be read. 
 * @param	len     : number of bytes to read.
 * @return	Returns -1 on error. On success returns the number of bytes 
 *		actually read 
 *
 * @description	This function reads the CK data block from the MTD device.
 * It returns -1 on error. On success returns the number of bytes actually
 * read from the MTD device.
 */
extern int readCKDataBlock (char *buff, int len);

#endif /* __CKBLOCK_READER_H */
