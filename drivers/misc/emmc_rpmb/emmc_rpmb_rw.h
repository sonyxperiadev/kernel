/*********************************************************************
 * *
 * *  Copyright 2010 Broadcom Corporation
 * *
 * *  Unless you and Broadcom execute a separate written software license
 * *  agreement governing use of this software, this software is licensed
 * *  to you under the terms of the GNU
 * *  General Public License version 2 (the GPL), available at
 * *  http://www.broadcom.com/licenses/GPLv2.php with the following added
 * *  to such license:
 * *  As a special exception, the copyright holders of this software give
 * *  you permission to link this software with independent modules, and
 * *  to copy and distribute the resulting executable under terms of your
 * *  choice, provided that you also meet, for each linked independent module,
 * *  the terms and conditions of the license of that module. An independent
 * *  module is a module which is not derived from this software.  The special
 * *  exception does not apply to any modifications of the software.
 * *  Notwithstanding the above, under no circumstances may you combine this
 * *  software in any way with any other Broadcom software provided under a
 * *  license other than the GPL, without Broadcom's express prior written
 * *  consent.
 * ***********************************************************************/

/*
 *
 *  @file   emmc_rpmb_rw.h
 *
 *  @brief  eMMC RPMB protocol driver.
 *
 */

#ifndef EMMC_RPMB_RW_H
#define EMMC_RPMB_RW_H

#define RPMB_DEV_NODE_PATH "/dev/block/mmcblk0rpmb"
#define MAIN_DEV_NODE_PATH "/dev/block/mmcblk0"
#define BOOT0_DEV_NODE_PATH "/dev/block/mmcblk0boot0"
#define BOOT1_DEV_NODE_PATH "/dev/block/mmcblk0boot1"

enum blk_dev_list {
	MAIN_DEV_NODE,
	BOOT0_DEV_NODE,
	BOOT1_DEV_NODE,
	TOTAL_BLK_DEV,
};

/* Utility Macros and Functions to make code endian independent */
const int endian_var = 1;
#define is_bigendian()	(*(char *)&endian_var == 0)

static void reverse_array(unsigned char *a, unsigned int lengthofarray)
{
	unsigned int i;
	if (is_bigendian()) {
		return;
	} else {
		for (i = 0; i <= (lengthofarray - 1) / 2; i++) {
			a[i] = a[i] + a[lengthofarray - 1 - i];
			a[lengthofarray - 1 - i] =
					a[i] - a[lengthofarray - 1 - i];

			a[i] = a[i] - a[lengthofarray - 1 - i];
		}
	}
}

/*
 * Main data structure representing a RPMB Data Frame.
 *
 * RPMB Data Frame follows a MSB first Byte Order.
 *
 * ---------------------------------------------
 * |	 |							 |	   |   |
 * |START|(MSB)	RPMB DATA FRAME (LSB)|CRC16|END|
 * |	 |							 |	   |   |
 * ---------------------------------------------
 *
 * NOTE1: We allocate memory for RPMB data frame in Little Endian
 * Byte Order and manipulate the data frame in LE Byte Order which
 * is the default endianess of our system. However, before sending
 * the Data Packet to RPMB we reverse the packet to make it BE byte
 * order as expected by RPMB partition.
 *
 * NOTE2: Byte order of the RPMB data frame is MSB first,
 * e.g. Write Counter MSB [11] is storing the upmost byte
 * of the counter value.
 */
struct rpmb_data_frame {
	unsigned short req_resp_code; /* [0:1] Request/Response Type */
	unsigned short result; /* [2:3] Operation result */
	unsigned short blk_count; /* [4:5] Number of blks to be read/written */
	unsigned short address; /* [6:7] Serial number of half sector */
	unsigned int write_counter;	/* [8:11] Write count val */
	unsigned char nonce[16]; /* [12:27] Random number generated */
	unsigned char data[256]; /* [28:283] Payload or Data */
	unsigned char key_mac[32]; /* [284:315] key or MAC */
	unsigned char stuff_bytes[196]; /* [316:511] Stuff bytes */
};

#define RPMB_DATA_FRAME_SIZE 512	/* 512 bytes */
#define PAYLOAD_SIZE 256			/* 256 bytes */
#define	PAYLOAD_SIZE_FOR_MAC 284	/* 284 bytes */

/* RPMB Data Frame Byte Order */
#define RPMB_STUFF_BYTES 316 /* 196 bytes */
#define RPMB_KEY_MAC 284 /* 32 bytes */
#define RPMB_DATA 28 /* 256 bytes */
#define RPMB_NONCE 12 /* 16 bytes */
#define RPMB_WRITE_COUNTER 8 /* 2 bytes */
#define RPMB_ADDRESS 6 /* 2 bytes */
#define RPMB_BLOCK_COUNT 4 /* 2 bytes */
#define RPMB_RESULT 2 /* 2 bytes */
#define RPMB_REQ_RESP_CODE 0 /* 2 bytes */

/* Request Message Types */
#define REQUEST_TYPE_AUTH_KEY_PROGRAMMING	0x0001
#define REQUEST_TYPE_READ_WRITE_COUNTER		0x0002
#define REQUEST_TYPE_AUTH_DATA_WRITE		0x0003
#define REQUEST_TYPE_AUTH_DATA_READ			0x0004
#define REQUEST_TYPE_RESULT_REG_READ		0x0005

/* Response Message Types */
#define RESPONSE_TYPE_AUTH_KEY_PROGRAMMING	0x0100
#define RESPONSE_TYPE_READ_WRITE_COUNTER	0x0200
#define RESPONSE_TYPE_AUTH_DATA_WRITE		0x0300
#define RESPONSE_TYPE_AUTH_DATA_READ		0x0400

/* RPMB Operation Results */
#define RESULT_OPERATION_OK					0x0000

#define RESULT_COUNTER_EXPIRED_FLAG			0x0080
#define RESULT_MASK							0x0007

#define RESULT_GENERAL_FAILURE				0x0001
#define RESULT_AUTHENTICATION_FAILURE		0x0002
#define RESULT_COUNTER_FAILURE				0x0003
#define RESULT_ADDRESS_FAILURE				0x0004
#define RESULT_WRITE_FAILURE				0x0005
#define RESULT_READ_FAILURE					0x0006
#define RESULT_AUTH_KEY_NOT_PROGRAMMED		0x0007

#define KEY_MAC_SIZE 32				/* 32 bytes */
#define NONCE_SIZE	16				/* 16 bytes */

/* Special error return values */
#define EGENERR			0x1
#define EAUTHFAIL		0x2
#define ECOUNTERERR		0x3
#define EADDRFAIL		0x4
#define EWRITEFAIL		0x5
#define EREADFAIL		0x6
#define EKEYNOTPROG		0x7
#define EDATACHKFAIL	0x8
#define EOTHER			0x9

/* Number of retries before returning error to caller */
#define RPMB_ACCESS_RETRY	20

#endif /* EMMC_RPMB_RW_H */
