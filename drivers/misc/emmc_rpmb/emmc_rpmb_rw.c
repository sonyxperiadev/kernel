/*******************************************************************************
Copyright 2010 Broadcom Corporation.  All rights reserved.
This program is the proprietary software of Broadcom Corporation and/or its
licensors, and may only be used, duplicated, modified or distributed pursuant
to the terms and conditions of a separate, written license agreement executed
between you and Broadcom (an "Authorized License").

Except as set forth in an Authorized License, Broadcom grants no
license(express or implied), right to use, or waiver of any kind with respect
to the Software, and Broadcom expressly reserves all rights in and to the
Software and all intellectual property rights therein.  IF YOU HAVE NO
AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY,
AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

Except as expressly set forth in the Authorized License,
1. This program, including its structure, sequence and organization, constitutes
the valuable trade secrets of Broadcom, and you shall use all reasonable efforts
to protect the confidentiality thereof, and to use this information only
in connection with your use of Broadcom integrated circuit products.

2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS" AND
WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR WARRANTIES,
EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO THE SOFTWARE.
BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE,
MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF
VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
PERFORMANCE OF THE SOFTWARE.

3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR USE
OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE
POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY
PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED
REMEDY.
*******************************************************************************/

#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/completion.h>
#include <linux/bio.h>
#include <linux/stat.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/genhd.h>
#include <linux/completion.h>
#include <linux/bio.h>
#include <linux/mutex.h>
#include <linux/module.h>

#include "emmc_rpmb_rw.h"
#include "hmac_sha2.h"
#include "sha2.h"

/* Change #define to 1 for debugging */
#define DEBUG	0

/*
 * IMEI, IMEI MAC and CKData locations in RPMB partition
 * address means address of half sector (256B)
 */
#define IMEI1_RPMB_START_ADDR		0x0
#define IMEI1_RPMB_LEN				256
#define IMEIMAC1_RPMB_START_ADDR	0x1
#define IMEIMAC1_RPMB_LEN			256
#define IMEI2_RPMB_START_ADDR		0x2
#define IMEI2_RPMB_LEN				256
#define IMEIMAC2_RPMB_START_ADDR	0x3
#define IMEIMAC2_RPMB_LEN			256
#define CKDATA_RPMB_START_ADDR		0x4
#define CKDATA_RPMB_LEN				2560
#define KEYBOX_RPMB_START_ADDR	0xe
#define KEYBOX_RPMB_LEN			256

struct kobject *emmc_rpmb_test_kobj;
static void *bio_buff;

#define RPMB_DEV_NODE_PATH "/dev/block/mmcblk0rpmb"

int fd; /* File descriptor to hold /dev/mmcblk0rpmb */

static DEFINE_MUTEX(rpmb_mutex);

/* Magic Key Pattern Used For Authentication Key */
const unsigned char magic_key[KEY_MAC_SIZE] = {
	0xba, 0xbe, 0xfa, 0xce, 0xba, 0xbe, 0xfa, 0xce,
	0xba, 0xbe, 0xfa, 0xce, 0xba, 0xbe, 0xfa, 0xce,
	0xba, 0xbe, 0xfa, 0xce, 0xba, 0xbe, 0xfa, 0xce,
	0xba, 0xbe, 0xfa, 0xce, 0xba, 0xbe, 0xfa, 0xce};

const unsigned char nonce_pattern[NONCE_SIZE] = {
	0xa5, 0xa5, 0xa5, 0xa5, 0xa5, 0xa5, 0xa5, 0xa5,
	0xa5, 0xa5, 0xa5, 0xa5, 0xa5, 0xa5, 0xa5, 0xa5};

/* Place holders for HMAC-SHA256 computation */
unsigned char mac_payload_buff[PAYLOAD_SIZE_FOR_MAC];
unsigned char key_mac_buff[KEY_MAC_SIZE];

#if DEBUG
/* Utility function to dump buffer for debug purpose */
static void dump_buffer(const char *cmdbuf, unsigned int size)
{
	uint i;

	pr_info("cmdbuf: %p, size: %d\n", cmdbuf, size);

	for (i = 0; i < size; i++) {
		if (!(i % 64))
			pr_info("\n");
		pr_info("0x%x ", *(cmdbuf + i));
	}
	pr_info("\n");
}
#endif

static void format_auth_read_data_packet(struct rpmb_data_frame *auth_read_data,
		 unsigned short address /* Start address of access */)
{
	/* memset to zero before start */
	memset(auth_read_data, 0, sizeof(struct rpmb_data_frame));

	/* Write 0xa5a5a5a5... for Nonce[27:12] */
	memcpy(auth_read_data->nonce, nonce_pattern, NONCE_SIZE);

	/* Set Read address of half sector */
	auth_read_data->address = address;

	/* Set Request type to indicate Read Request */
	auth_read_data->req_resp_code = REQUEST_TYPE_AUTH_DATA_READ;
}

/* BIO read/write APIs */
static void emmc_rpmb_bio_complete(struct bio *bio, int err)
{
	complete((struct completion *)bio->bi_private);
}

static int block_read(const char *user_dev_path, /* Path to rpmb device */
		char *read_buff, /* User buffer */
		size_t size) /* Size of data to read (in bytes) */
{
	int i = 0, index = 0;
	int err;
	struct block_device *bdev;
	struct bio bio;
	struct bio_vec bio_vec;
	struct completion complete;
	struct page *page;
	int end_sect;

	bdev = blkdev_get_by_path(user_dev_path,
				  FMODE_READ, block_read);

	if (IS_ERR(bdev)) {
		pr_err("failed to get block device %s (%ld)\n",
		      user_dev_path, PTR_ERR(bdev));
		return -ENODEV;
	}

	page = virt_to_page(bio_buff);

	end_sect = (size - 1) / 512;

	for (i = 0; i <= end_sect; i++) {
		bio_init(&bio);
		bio.bi_io_vec = &bio_vec;
		bio_vec.bv_page = page;
		bio_vec.bv_len = 512;
		bio_vec.bv_offset = 0;
		bio.bi_vcnt = 1;
		bio.bi_idx = 0;
		bio.bi_size = 512;
		bio.bi_bdev = bdev;
		bio.bi_sector = 0;
		init_completion(&complete);
		bio.bi_private = &complete;
		bio.bi_end_io = emmc_rpmb_bio_complete;
		submit_bio(READ, &bio);
		wait_for_completion(&complete);
		if (!test_bit(BIO_UPTODATE, &bio.bi_flags)) {
			err = -EIO;
			goto out_blkdev;
		}

		memcpy(read_buff + index, bio_buff, 512);
		index += 512;
	}

	err = size;

out_blkdev:
	blkdev_put(bdev, FMODE_READ);

	return err;
}

static int block_write(const char *user_dev_path, /* Path to rpmb device node */
		const char *write_buff, /* buffer to write to rpmb */
		size_t size, /* size of data to write (in bytes) */
		int flags) /* REQ_META flags for Reliable writes */
{
	int i = 0, index = 0;
	struct block_device *bdev;
	struct bio bio;
	struct bio_vec bio_vec;
	struct completion complete;
	struct page *page;
	int end_sect;

	bdev = blkdev_get_by_path(user_dev_path,
				  FMODE_WRITE, block_write);

	if (IS_ERR(bdev)) {
		pr_err("failed to get block device %s (%ld)\n",
		      user_dev_path, PTR_ERR(bdev));
		return -ENODEV;
	}

	page = virt_to_page(bio_buff);

	end_sect = (size - 1) / 512;

	for (i = 0; i <= end_sect; i++) {
		/* Copy data from user buffer to bio buffer */
		memcpy(bio_buff, write_buff + index, 512);
		index += 512;

		bio_init(&bio);
		bio.bi_io_vec = &bio_vec;
		bio_vec.bv_page = page;
		bio_vec.bv_len = 512;
		bio_vec.bv_offset = 0;
		bio.bi_vcnt = 1;
		bio.bi_idx = 0;
		bio.bi_size = 512;
		bio.bi_bdev = bdev;
		/* Set to 0 because the addr is part of RPMB data frame */
		bio.bi_sector = 0;
		init_completion(&complete);
		bio.bi_private = &complete;
		bio.bi_end_io = emmc_rpmb_bio_complete;
		submit_bio(WRITE | flags, &bio);
		wait_for_completion(&complete);
	}

	blkdev_put(bdev, FMODE_WRITE);

	return 0;
}

static int authenticated_data_read(char *buff, int len, int addr)
{
	int data_cnt = 0; /* in bytes */
	int sec_addr = addr; /* half sector address to read from */
	int ret = 0;
	struct rpmb_data_frame data_packet;
	unsigned char *read_buff = NULL;
	struct rpmb_data_frame *read_data_packet;
	unsigned short result;
	int num_half_sec;
	int retries = RPMB_ACCESS_RETRY;

#if DEBUG
	unsigned char *ptr_auth_read_data = (unsigned char *)&data_packet;
#endif

	/* 0x20000 = 128K = RPMB size */
	if (((sec_addr << 8) + len) > 0x20000) {
		pr_err("authenticated_data_read:"
						"cannot read past end of RPMB\n");
		return -EOTHER;
	}

#if DEBUG
	pr_info("%s: buff: %p, len: %d, addr: %x\n",
				__func__, buff, len, addr);
#endif

	num_half_sec = (len % PAYLOAD_SIZE) ?
						((len / PAYLOAD_SIZE) + 1) :
						(len / PAYLOAD_SIZE);

	/* Allocate memory for read_buff */
	read_buff = kmalloc(RPMB_DATA_FRAME_SIZE, GFP_KERNEL);

	do {
read_retry:
		/* Format Authenticated Read Data Packet */
		format_auth_read_data_packet(&data_packet, sec_addr);

		/*
		 * Reverse Data Packet to make it MSB first byte order,
		 * as expected by RPMB
		 */
		reverse_array((unsigned char *)&data_packet,
					sizeof(struct rpmb_data_frame));

#if DEBUG
		pr_info("%s: Read Data Packet WRITE\n", __func__);
		dump_buffer((char *)ptr_auth_read_data, RPMB_DATA_FRAME_SIZE);
#endif

		/* Write the Data Packet to RPMB */
		ret = block_write(RPMB_DEV_NODE_PATH,
				(char *)&data_packet, /* Data buffer */
				RPMB_DATA_FRAME_SIZE, /* Size of data packet */
				0x0); /* special flags if any */
		if (ret < 0) {
			pr_err("Write Data Packet failed: %d\n", ret);
			ret = -EOTHER;
			goto out;
		}

		memset(&read_buff[0], 0x0, RPMB_DATA_FRAME_SIZE);

		/* Read the data packet from RPMB */
		ret = block_read(RPMB_DEV_NODE_PATH,
				&read_buff[0], /* Data buffer */
				RPMB_DATA_FRAME_SIZE); /* Size of data packet */
		if (ret < 0) {
			pr_err("Read Data Packet failed: %d\n", ret);
			ret = -EOTHER;
			goto out;
		}

#if DEBUG
		pr_info("%s: Read Data Packet READ:\n", __func__);
		dump_buffer((char *)read_buff, RPMB_DATA_FRAME_SIZE);
#endif

		/* read_buff is returned in MSB First Byte Order, reverse it */
		reverse_array(read_buff, RPMB_DATA_FRAME_SIZE);

		read_data_packet = (struct rpmb_data_frame *)read_buff;

#if DEBUG
		pr_info("Result Register Value: 0x%x \r\n",
				read_data_packet->result);
#endif

		/* Check if Authentication Key is programmed & return if NOT */
		result = read_data_packet->result;
		if (result == RESULT_AUTH_KEY_NOT_PROGRAMMED) {
			pr_info("Authentication Key NOT programmed; result: 0x%x\n",
					result);
			ret = -EKEYNOTPROG;
			goto out;
		}

		/*
		* Retry on GENERAL FAILURE
		*
		* "Short Story" on why this retry mechanism is needed:
		* On "Micron eMMC parts", when RPMB transaction session is in
		* progress, if a new (read/write) request is issued to a
		* NON-RPMB (user data) partition on the eMMC, and then we
		* continue the previous RPMB access transaction, the RPMB
		* access returns GENERAL FAILURE.
		*
		* Below log snapshot will help explain this better:
		* -------> START RPMB TRANSACTION <-- (session 1)
		*		--> RPMB CMD6 - Switch to RPMB partition
		*		--> RPMB CMD23 - Set BLK_CNT
		*		--> RPMB CMD25 - Issue a Mult Blk Write
		*		--> RPMB CMD13 - Issue Status CMD and
		*			check Write completion
		* -------> RPMB TRANSACTION ABORTED BY NON-RPMB ACCESS
		*		--> NON-RPMB CMD6 - Switch to Userdata partition
		*		--> NON-RPMB CMD23 - Set BLK_CNT
		*		--> NON-RPMB CMD25 - Issue Mutl Blk Write
		*		--> NON-RPMB CMD12 - Issue Status CMD and
		*			check completion
		* -------> SWITCH BACK TO RPMB PARTITION <--
		*				(Continue session 1)
		*		--> RPMB CMD6 - Switch back to RPMB partition
		*		--> RPMB CMD23 - Set BLK_CNT
		*		--> RPMB CMD18 - Issue a Mult Blk Read
		* The RPMB device returns GENERAL FAILURE in the Read data
		* Packet.
		*
		* When we get into this scenario (GENERAL FAILURE), we simply
		* retry for 20 times and hope that we succeed atleast once.
		*
		* Refer to Sec: 7.6.16.4 in JESD84-A441.pdf on why this
		* needs to be done here.
		*/
		/* General Failure, retry */
		if (result == RESULT_GENERAL_FAILURE) {
			pr_err("RPMB Read Data General Error. Retrying...\n");
			if (--retries) {
				pr_err("Retry count: %d\n", retries);
				goto read_retry;
			} else { /* Retries exhausted, return error */
				pr_err("Retried %d times. No success."
						"Returning error to caller\n",
						RPMB_ACCESS_RETRY);
				ret = -EGENERR;
				goto out;
			}
		}

		/* Authenticate the read data packet */
		memset(mac_payload_buff, 0, PAYLOAD_SIZE_FOR_MAC);
		memset(key_mac_buff, 0, KEY_MAC_SIZE);
		memcpy(mac_payload_buff, (unsigned char *)read_data_packet,
				PAYLOAD_SIZE_FOR_MAC);
		reverse_array(mac_payload_buff, PAYLOAD_SIZE_FOR_MAC);

#if DEBUG
		pr_info("%s: Payload for HMA-SHA256 calculation\n", __func__);
		dump_buffer((char *)mac_payload_buff, PAYLOAD_SIZE_FOR_MAC);
#endif

		/* Call HMAC-SHA256 function to compute MAC */
		hmac_sha256(magic_key, KEY_MAC_SIZE,
				mac_payload_buff, PAYLOAD_SIZE_FOR_MAC,
				key_mac_buff, KEY_MAC_SIZE);

#if DEBUG
		pr_info("%s: HMAC-SHA256 value:\n", __func__);
		dump_buffer((char *)key_mac_buff, KEY_MAC_SIZE);
#endif

		/* Compare computed MAC against MAC value in the data frame */
		reverse_array(key_mac_buff, KEY_MAC_SIZE);
		ret = memcmp(key_mac_buff, read_data_packet->key_mac,
						KEY_MAC_SIZE);
		if (ret) {
			pr_err("%s: HMAC-SHA256 check failed. Data could be invalid",
					__func__);
			ret = -EDATACHKFAIL;
			goto out;
		}

		/* Check for other errors */
		if (read_data_packet->req_resp_code ==
			RESPONSE_TYPE_AUTH_DATA_READ) {

			if (result == RESULT_ADDRESS_FAILURE) {
				pr_err("Write address Error\n");
				ret = -EADDRFAIL;
				goto out;
			}

			if (result == RESULT_AUTHENTICATION_FAILURE) {
				pr_err("Read Data Authentication Failure\n");
				ret = -EAUTHFAIL;
				goto out;
			}

			if (result == RESULT_READ_FAILURE) {
				pr_err("Read Data Error\n");
				ret = -EREADFAIL;
				goto out;
			}
		} else {
			pr_err("Read Data- Wrong response type\n");
			ret = -EOTHER;
			goto out;
		}

		/* Copy data to user buffer */
		if (--num_half_sec) /* Not the last half-sector */
			memcpy(buff + data_cnt, read_data_packet->data,
					PAYLOAD_SIZE);
		else /* Last half sector */
			memcpy(buff + data_cnt, read_data_packet->data, len);

		len -= PAYLOAD_SIZE;
		data_cnt += PAYLOAD_SIZE;
		sec_addr++;
	} while (num_half_sec);

out:
	kfree(read_buff);
	return ret;
}

/**
 * @brief	readCKDataBlock : Reads the CK Data block from the eMMC RPMB partition.
 * @param	buff	: Kernel Space buffer to which the data will be read.
 * @param	len     : number of bytes to read.
 * @return	Returns -ve number on error and 0 on success
 *
 * @description	This function reads the CK data block from the RPMB partition.
 */
int readCKDataBlock(char *buff, int len)
{
	int ret = 0;

	if ((NULL == buff) || (0 == len)) {
		pr_err("readCKDataBlock: buffer or length invalid \r\n");
		return -1;
	}

	if (len > CKDATA_RPMB_LEN) {
		pr_err("Cannot read more than CKData block size\n");
		return -1;
	}

	mutex_lock(&rpmb_mutex);

	ret = authenticated_data_read(buff, len, CKDATA_RPMB_START_ADDR);
	if (ret < 0) {
		pr_err("CKData Block read failure\n");
		goto err_out;
	}

	pr_info("Done Reading %d bytes of CKData from RPMB\n", len);

err_out:
	mutex_unlock(&rpmb_mutex);
	return ret;
}
EXPORT_SYMBOL(readCKDataBlock);

/**
 * @brief	read_imei1: Reads the IMEI and IMEI MAC for SIM1
 *						from the eMMC RPMB partition.
 *
 * @param	imei1		: Kernel buffer to which IMEI1 will be read.
 * @param	imei_mac1	: Kernel buffer to which IMEI MAC1 will be read.
 * @param	imei1_len	: len of IMEI1 (bytes) to read.
 * @param	imei_mac1_len: len of IMEI_MAC1 (bytes) to read.
 *
 * @return	Returns -1 on error and 0 on success
 *
 * @description	This function reads IMEI1/IMEIMAC1 from the RPMB partition.
 */
int read_imei1(char *imei1, char *imei_mac1, int imei1_len, int imei_mac1_len)
{
	int ret1 = 0, ret2 = 0;

	if ((NULL == imei1) || (0 == imei1_len) ||
		(NULL == imei_mac1) || (0 == imei_mac1_len)) {
		pr_err("read_imei1: buffer or length invalid \r\n");
		return -1;
	}

	if ((imei1_len > IMEI1_RPMB_LEN) ||
		(imei_mac1_len > IMEIMAC1_RPMB_LEN)) {
		pr_err("Cannot read more than IMEI1/IMEIMAC1 block size\n");
		return -1;
	}

	mutex_lock(&rpmb_mutex);

	ret1 = authenticated_data_read(imei1, imei1_len,
						IMEI1_RPMB_START_ADDR);
	if (ret1 < 0) {
		pr_err("IMEI1 read failure\n");
		goto err_out;
	}

	pr_info("Done Reading %d bytes of IMEI1 from RPMB\n", imei1_len);

	ret2 = authenticated_data_read(imei_mac1, imei_mac1_len,
						IMEIMAC1_RPMB_START_ADDR);
	if (ret2 < 0) {
		pr_err("IMEI MAC1 read Failure\n");
		goto err_out;
	}

	pr_info("Done Reading %d bytes of IMEI MAC1 from RPMB\n",
							imei_mac1_len);

err_out:
	mutex_unlock(&rpmb_mutex);
	if (ret1 || ret2)
		return ret1 ? ret1 : ret2;
	else
		return 0;
}
EXPORT_SYMBOL(read_imei1);

/**
 * @brief	read_imei2: Reads the IMEI and IMEI MAC for SIM2
 *						from the eMMC RPMB partition.
 *
 * @param	imei2		: Kernel buffer to which IMEI2 will be read.
 * @param	imei_mac2	: Kernel buffer to which IMEI MAC2 will be read.
 * @param	imei2_len	: len of IMEI2 (bytes) to read.
 * @param	imei_mac2_len: len of IMEI_MAC2 (bytes) to read.
 *
 * @return	Returns -1 on error and 0 on success
 *
 * @description	This function reads IMEI2/IMEIMAC2 from the RPMB partition.
 */
int read_imei2(char *imei2, char *imei_mac2, int imei2_len, int imei_mac2_len)
{
	int ret1 = 0, ret2 = 0;

	if ((NULL == imei2) || (0 == imei2_len) ||
		(NULL == imei_mac2) || (0 == imei_mac2_len)) {
		pr_err("read_imei2: buffer or length invalid \r\n");
		return -1;
	}

	if ((imei2_len > IMEI2_RPMB_LEN) ||
		(imei_mac2_len > IMEIMAC2_RPMB_LEN)) {
		pr_err("Cannot read more than IMEI2/IMEIMAC2 block size\n");
		return -1;
	}

	mutex_lock(&rpmb_mutex);

	ret1 = authenticated_data_read(imei2, imei2_len,
						IMEI2_RPMB_START_ADDR);
	if (ret1 < 0) {
		pr_err("IMEI2 read Failure\n");
		goto err_out;
	}

	pr_info("Done Reading %d bytes of IMEI2 from RPMB!\n", imei2_len);

	ret2 = authenticated_data_read(imei_mac2, imei_mac2_len,
						IMEIMAC2_RPMB_START_ADDR);
	if (ret2 < 0) {
		pr_err("IMEI MAC2 read Failure\n");
		goto err_out;
	}

	pr_info("Done Reading %d bytes of IMEI MAC2 from RPMB!\n",
							imei_mac2_len);

err_out:
	mutex_unlock(&rpmb_mutex);
	if (ret1 || ret2)
		return ret1 ? ret1 : ret2;
	else
		return 0;
}
EXPORT_SYMBOL(read_imei2);

/**
 * @brief	readkeybox : Reads keybox data from eMMC RPMB partition.
 * @param	buff	: Kernel Space buffer to which the data will be read.
 * @param	len     : number of bytes to read.
 * @return	Returns -ve number on error and 0 on success
 *
 * @description	This function reads the Keybox data from the RPMB partition.
 */
int readkeybox(char *buff, int len)
{
	int ret = 0;

	if ((NULL == buff) || (0 == len)) {
		pr_err("readkeybox: buffer or length invalid \r\n");
		return -1;
	}

	if (len > KEYBOX_RPMB_LEN) {
		pr_err("Cannot read more than keybox block size\n");
		return -1;
	}

	mutex_lock(&rpmb_mutex);

	ret = authenticated_data_read(buff, len, KEYBOX_RPMB_START_ADDR);
	if (ret < 0) {
		pr_err("Keybox data read failure\n");
		goto err_out;
	}

	pr_info("Done Reading %d bytes of keybox from RPMB\n", len);

err_out:
	mutex_unlock(&rpmb_mutex);
	return ret;
}
EXPORT_SYMBOL(readkeybox);

/* Init routine */
int __init emmc_rpmb_rw_init(void)
{
	/* Allocate 1-page memory for BIO */
	bio_buff = (void *)__get_free_page(GFP_KERNEL);
	pr_info("eMMC RPMB rw driver initialized\n");

	return 0;
}

module_init(emmc_rpmb_rw_init);
