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

#include "emmc_rpmb_rw.h"
#include "hmac_sha2.h"
#include "sha2.h"

struct kobject *emmc_rpmb_test_kobj;
static void *bio_buff;

/* Change #define to 1 for debuggin Change #define to 1 for debuggingg */
#define DEBUG	0

int fd; /* File descriptor to hold /dev/mmcblk0rpmb */

/* Magic Key Pattern Used For Authentication Key */
const unsigned char magic_key[KEY_MAC_SIZE] = {
	0xba, 0xbe, 0xfa, 0xce, 0xba, 0xbe, 0xfa, 0xce,
	0xba, 0xbe, 0xfa, 0xce, 0xba, 0xbe, 0xfa, 0xce,
	0xba, 0xbe, 0xfa, 0xce, 0xba, 0xbe, 0xfa, 0xce,
	0xba, 0xbe, 0xfa, 0xce, 0xba, 0xbe, 0xfa, 0xce
	};

const unsigned char nonce_pattern[NONCE_SIZE] = {
	0xa5, 0xa5, 0xa5, 0xa5, 0xa5, 0xa5, 0xa5, 0xa5,
	0xa5, 0xa5, 0xa5, 0xa5, 0xa5, 0xa5, 0xa5, 0xa5
	};

/* Place holders for HMAC-SHA256 computation */
unsigned char mac_payload_buff[PAYLOAD_SIZE_FOR_MAC];
unsigned char key_mac_buff[KEY_MAC_SIZE];

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

static void format_authentication_key_data_packet(
				struct rpmb_data_frame *authentication_key)
{
	unsigned char key_buff[KEY_MAC_SIZE];

	/* memset to zero before start */
	memset(authentication_key, 0, sizeof(struct rpmb_data_frame));

	/*
	 * Write 0xbabefacebabefacebabeface... for unique authentication key
	 *
	 * NOTE: Reversing is needed here because the key needs to be written
	 * to RPMB in BE byte order. So we write the LE order here which is
	 * converted to the correct BE Byte Order before write() is called in
	 * the calling function.
	 */
	memcpy(key_buff, magic_key, KEY_MAC_SIZE);
	reverse_array(key_buff, KEY_MAC_SIZE);
	memcpy(authentication_key->key_mac, key_buff, KEY_MAC_SIZE);

	/* Set Request type to indicate authentication key programming */
	authentication_key->req_resp_code = REQUEST_TYPE_AUTH_KEY_PROGRAMMING;
}

static void format_read_result_reg_data_packet(
				struct rpmb_data_frame *read_result)
{
	/* memset to zero before start */
	memset(read_result, 0, sizeof(struct rpmb_data_frame));

	/* Set Request type to indicate Result Register Read type request */
	read_result->req_resp_code = REQUEST_TYPE_RESULT_REG_READ;
}

static void format_read_counter_val_data_packet(
				struct rpmb_data_frame *read_counter_val)
{
	/* memset to zero before start */
	memset(read_counter_val, 0, sizeof(struct rpmb_data_frame));

	/* Write 0xa5a5a5a5... for Nonce[27:12] */
	memcpy(read_counter_val->nonce, nonce_pattern, NONCE_SIZE);

	/* Set Request type to indicate Counter Value Read Request Initiation */
	read_counter_val->req_resp_code = REQUEST_TYPE_READ_WRITE_COUNTER;
}

static void format_auth_write_data_packet(
		struct rpmb_data_frame *auth_write_data,
		const char *data, /* Data/Payload */
		int size, /* Size of data to be written in bytes */
		unsigned int write_counter, /* Current Write Counter Value */
		unsigned short address /* Start address of access */)
{
	/* memset to zero before start */
	memset(auth_write_data, 0, sizeof(struct rpmb_data_frame));

#if DEBUG
	pr_info("%s: Copying %d bytes to RPMB data frame\n",
				__func__, size);
#endif

	/* Next copy the Payload */
	memcpy(auth_write_data->data, data, size);

	/* Next copy the Write Counter */
	auth_write_data->write_counter = write_counter;

	/* Copy the Start Address */
	auth_write_data->address = address;

	/* We always only write 1 blk */
	auth_write_data->blk_count = 1;

	/* Set Request type to indicate Write Request */
	auth_write_data->req_resp_code = REQUEST_TYPE_AUTH_DATA_WRITE;

	/* Next Calculate HMAC-SHA256 on the RPMB data frame [283:0] bytes */
	memset(mac_payload_buff, 0, PAYLOAD_SIZE_FOR_MAC);
	memset(key_mac_buff, 0, KEY_MAC_SIZE);
	memcpy(mac_payload_buff, (unsigned char *)auth_write_data,
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

	/*
	 * Reverse and set the MAC value in MAC field of Data Frame.
	 *
	 * NOTE: Reversing is needed here because we computed MAC on BE Data
	 * Frame and we need to reverse the MAC here which will be converted
	 * to the correct Byte Order before write() is called in the calling
	 * function.
	 */
	reverse_array(key_mac_buff, KEY_MAC_SIZE);
	memcpy(auth_write_data->key_mac, key_mac_buff, KEY_MAC_SIZE);

	return;
}

static void format_auth_read_data_packet(
		struct rpmb_data_frame *auth_read_data,
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
		pr_info("failed to get block device %s (%ld)\n",
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

static int program_authentication_key(void)
{
	int ret = 0;
	struct rpmb_data_frame data_packet;
	unsigned char *read_buff;
	struct rpmb_data_frame *read_data_packet;
	unsigned short result;
	int retries = RPMB_ACCESS_RETRY;

#if DEBUG
	unsigned char *ptr_auth_key = (unsigned char *)&data_packet;
	unsigned char *ptr_result_reg = (unsigned char *)&data_packet;
#endif

	/* Alloc memory for read_buff */
	read_buff = kmalloc(RPMB_DATA_FRAME_SIZE, GFP_KERNEL);

key_retry:
	/* format Authentication Key Data Packet */
	format_authentication_key_data_packet(&data_packet);

	/* Reverse Data Packet to MSB first byte order as expected by RPMB */
	reverse_array((unsigned char *)&data_packet,
				sizeof(struct rpmb_data_frame));

#if DEBUG
	pr_info("%s: authentication_key_data WRITE\n", __func__);
	dump_buffer((char *)ptr_auth_key, RPMB_DATA_FRAME_SIZE);
#endif

	/* Write the authentication key data packet to RPMB partition */
	ret = block_write(RPMB_DEV_NODE_PATH,
				(char *)&data_packet, /* Data buffer */
				RPMB_DATA_FRAME_SIZE, /* Size of data packet */
				REQ_META); /* special flags if any */
	if (ret < 0) {
		pr_err("Write Authentication Key Data Packet failed: %d\n",
				ret);
		ret = -EOTHER;
		goto out;
	}

	/* Read Result Register of RPMB area to validate key programming */
	/* Format Read Result Register Data Packet */
	format_read_result_reg_data_packet(&data_packet);

	/* Reverse Data Packet to MSB first byte order as expected by RPMB */
	reverse_array((unsigned char *)&data_packet,
				sizeof(struct rpmb_data_frame));

#if DEBUG
	pr_info("%s: read_result_reg WRITE\n", __func__);
	dump_buffer((char *)ptr_result_reg, RPMB_DATA_FRAME_SIZE);
#endif

	/* Write the Read Result Register data packet to RPMB */
	ret = block_write(RPMB_DEV_NODE_PATH,
				(char *)&data_packet, /* Data buffer */
				RPMB_DATA_FRAME_SIZE, /* Size of data packet */
				0x0); /* special flags if any */
	if (ret < 0) {
		pr_err("Write Result Resgister Data Packet failed: %d\n", ret);
		ret = -EOTHER;
		goto out;
	}

	/* Read the Read Result Register data packet from RPMB */
	ret = block_read(RPMB_DEV_NODE_PATH,
				&read_buff[0], /* Data buffer */
				RPMB_DATA_FRAME_SIZE); /* Size of data packet */
	if (ret < 0) {
		pr_err("Read Result Resgister Data Packet failed: %d\n", ret);
		ret = -EOTHER;
		goto out;
	}

#if DEBUG
	pr_info("%s: Result Reg Data Packet READ:\n", __func__);
	dump_buffer((char *)read_buff, RPMB_DATA_FRAME_SIZE);
#endif

	/* read_buff is returned in MSB First Byte Order, reverse it */
	reverse_array(read_buff, RPMB_DATA_FRAME_SIZE);

	read_data_packet = (struct rpmb_data_frame *)read_buff;

	result = read_data_packet->result;

#if DEBUG
	pr_info("Result Register Value: 0x%x \r\n", read_data_packet->result);
#endif

	/* Check if Authentication is programmed up front and return if NOT */
	if (result == RESULT_AUTH_KEY_NOT_PROGRAMMED) {
		pr_err("Authentication Key NOT programmed; result: 0x%x\n",
				result);
		ret = -EKEYNOTPROG;
		goto out;
	}

	/* Retry on GENERAL FAILURE */
	if (result == RESULT_GENERAL_FAILURE) {
		pr_err("Write Key- GENERAL ERROR. Retrying....\n");
		if (--retries) {
			pr_err("Retry count: %d\n", retries);
			goto key_retry;
		} else {
			pr_err("Retried %d times. No success."
					"Returning error to caller\n",
					RPMB_ACCESS_RETRY);
			ret = -EGENERR;
			goto out;
		}
	}

	/* Check for authentication key programming errors */
	if (read_data_packet->req_resp_code ==
			RESPONSE_TYPE_AUTH_KEY_PROGRAMMING) {
		if (result == RESULT_WRITE_FAILURE) {
			pr_err("Write Authentication Key- WRITE ERROR\n");
			ret = -EWRITEFAIL;
			goto out;
		}
	}

out:
	kfree(read_buff);
	return ret;
}

static int read_counter_value(void)
{
	int ret = 0;
	struct rpmb_data_frame data_packet;
	unsigned char *read_buff;
	struct rpmb_data_frame *read_data_packet;
	unsigned short result;
	unsigned int counter;
	int retries = RPMB_ACCESS_RETRY;

#if DEBUG
	unsigned char *ptr_counter_val = (unsigned char *)&data_packet;
#endif

	/* Alloc memory for read_buff */
	read_buff = kmalloc(RPMB_DATA_FRAME_SIZE, GFP_KERNEL);

counter_retry:
	/* Format the Read counter value Data Packet */
	format_read_counter_val_data_packet(&data_packet);

	/* Reverse Data Packet to MSB first byte order as expected by RPMB */
	reverse_array((unsigned char *)&data_packet,
				sizeof(struct rpmb_data_frame));

#if DEBUG
	pr_info("%s: Counter Value Data Packet WRITE\n", __func__);
	dump_buffer((char *)ptr_counter_val, RPMB_DATA_FRAME_SIZE);
#endif

	/* Write the Read Counter Value Data Packet to RPMB */
	ret = block_write(RPMB_DEV_NODE_PATH,
				(char *)&data_packet, /* Data buffer */
				RPMB_DATA_FRAME_SIZE, /* Size of data packet */
				0x0); /* special flags if any */
	if (ret < 0) {
		pr_err("Write Counter Value Data Packet failed: %d\n", ret);
		ret = -EOTHER;
		goto out;
	}

	/* Read the Counter Value data packet from RPMB */
	ret = block_read(RPMB_DEV_NODE_PATH,
				&read_buff[0], /* Data buffer */
				RPMB_DATA_FRAME_SIZE); /* Size of data packet */
	if (ret < 0) {
		pr_err("Read Counter Value Data Packet failed: %d\n", ret);
		ret = -EOTHER;
		goto out;
	}

#if DEBUG
	pr_info("%s: Counter Value Data Packet READ:\n", __func__);
	dump_buffer((char *)read_buff, RPMB_DATA_FRAME_SIZE);
#endif

	/* read_buff is returned in MSB First Byte Order, reverse it */
	reverse_array(read_buff, RPMB_DATA_FRAME_SIZE);

	read_data_packet = (struct rpmb_data_frame *)read_buff;

	result = read_data_packet->result;
	counter = read_data_packet->write_counter;

#if DEBUG
	pr_info("Result Register Value: 0x%x \r\n", result);
	pr_info("Counter Value: 0x%x \r\n", counter);
#endif

	/* Check if Authentication is programmed up front and return if NOT */
	if (result == RESULT_AUTH_KEY_NOT_PROGRAMMED) {
		pr_err("Authentication Key NOT programmed; result: 0x%x\n",
				result);
		ret = -EKEYNOTPROG;
		goto out;
	}

	/* Retry on GENERAL FAILURE */
	if (result == RESULT_GENERAL_FAILURE) {
		pr_err("Read Counter Value- GENERAL FAILURE. Retrying....\n");
		if (--retries) {
			pr_err("%s: Retry count: %d\n", __func__, retries);
			goto counter_retry;
		} else {
			pr_err("%s: Retried %d times. No success."
					"Returning error to caller\n",
					__func__, RPMB_ACCESS_RETRY);
			ret = -EGENERR;
			goto out;
		}
	}

	/* Authenticate Counter value data packet */
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

	/* Compare the computed MAC against the MAC value in the data frame */
	reverse_array(key_mac_buff, KEY_MAC_SIZE);
	ret = memcmp(key_mac_buff, read_data_packet->key_mac, KEY_MAC_SIZE);
	if (ret) {
		pr_err("%s: HMAC-SHA256 check failed. Data may be invalid",
				__func__);
		ret = -EDATACHKFAIL;
		goto out;
	}

	/* Check for Read Counter Errors */
	if (read_data_packet->req_resp_code ==
		RESPONSE_TYPE_READ_WRITE_COUNTER) {

		if (result & RESULT_COUNTER_EXPIRED_FLAG) {
			pr_err("Write Counter expired\n");
			ret = -ECOUNTERERR;
			goto out;
		} else {
			if (result == RESULT_READ_FAILURE) {
				pr_err("Read Counter Value- READ ERROR\n");
				ret = -EREADFAIL;
				goto out;
			}
		}
	} else {
		pr_err("Read Counter Value- Wrong response type\n");
		ret = -EOTHER;
		goto out;
	}

	kfree(read_buff);
	return counter;
out:
	kfree(read_buff);
	return ret;
}

static int authenticated_data_write(const char *data, int bytes, int addr)
{
	int ret = 0;
	struct rpmb_data_frame data_packet;
	unsigned char *read_buff;
	struct rpmb_data_frame *read_data_packet;
	unsigned short result;
	int counter_val;
	int blkcnt; /* half sector (256B) count */
	int data_size = bytes;
	int retries = RPMB_ACCESS_RETRY;

#if DEBUG
	unsigned char *ptr_auth_write_data = (unsigned char *)&data_packet;
	unsigned char *ptr_result_reg = (unsigned char *)&data_packet;
#endif

	/* If (start + (blkcnt * 512)) > rpmb_size, return error */
	if (((addr << 8) + bytes) > (128*1024)) {
		pr_err("Cannot write past the RPMB paritition size\n");
		return -EOTHER;
	}

	blkcnt = (bytes % PAYLOAD_SIZE) ?
				((bytes / PAYLOAD_SIZE) + 1) :
				(bytes / PAYLOAD_SIZE);

#if DEBUG
	pr_info("data: %p, bytes: %d, blkcnt: %d, addr: %x\n",
			data, bytes, blkcnt, addr);
#endif

	/* alloc memory for read_buff */
	read_buff = kmalloc(RPMB_DATA_FRAME_SIZE, GFP_KERNEL);

	do {
		/* Get counter value */
		counter_val = read_counter_value();
		if (counter_val < 0) {
			pr_err("Counter value read failed\n");
			ret = -ECOUNTERERR;
			goto out;
		}

write_retry:
		/* Format Authenticated Write Data Packet */
		if (blkcnt == 1) /* last half sector data */
			format_auth_write_data_packet(&data_packet, data,
					data_size, counter_val, addr);
		else /* Not the last half sector data */
			format_auth_write_data_packet(&data_packet, data,
					PAYLOAD_SIZE, counter_val, addr);

		/*
		 * Reverse Data Packet to MSB first byte order as
		 * expected by RPMB
		 */
		reverse_array((unsigned char *)&data_packet,
					sizeof(struct rpmb_data_frame));

#if DEBUG
		pr_info("%s: Write Data Packet WRITE\n", __func__);
		dump_buffer((char *)ptr_auth_write_data, RPMB_DATA_FRAME_SIZE);
#endif

		/* Write the Data Packet to RPMB */
		ret = block_write(RPMB_DEV_NODE_PATH,
				(char *)&data_packet, /* Data buffer */
				RPMB_DATA_FRAME_SIZE, /* Size of data packet */
				REQ_META); /* special flags if any */
		if (ret < 0) {
			pr_err("Write Data Packet failed: %d\n", ret);
			ret = -EOTHER;
			goto out;
		}

		/* Format Read Result Register Data Packet */
		format_read_result_reg_data_packet(&data_packet);

		/* Reverse Data Packet to MSB first byte order */
		reverse_array((unsigned char *)&data_packet,
					sizeof(struct rpmb_data_frame));

#if DEBUG
		pr_info("%s: read_result_reg WRITE\n", __func__);
		dump_buffer((char *)ptr_result_reg, RPMB_DATA_FRAME_SIZE);
#endif

		/* Write the Read Result Register data packet to RPMB */
		ret = block_write(RPMB_DEV_NODE_PATH,
				(char *)&data_packet, /* Data buffer */
				RPMB_DATA_FRAME_SIZE, /* Size of data packet */
				0x0); /* special flags if any */
		if (ret < 0) {
			pr_err("Write Result Resgister Data"
					"Packet failed: %d\n", ret);
			ret = -EOTHER;
			goto out;
		}

		/* Read the Read Result Register data packet from RPMB */
		ret = block_read(RPMB_DEV_NODE_PATH,
				&read_buff[0], /* Data buffer */
				RPMB_DATA_FRAME_SIZE); /* Size of data packet */
		if (ret < 0) {
			pr_err("Read Result Resgister Data"
					"Packet failed: %d\n", ret);
			ret = -EOTHER;
			goto out;
		}

#if DEBUG
		pr_info("%s: Result Reg Data Packet READ:\n", __func__);
		dump_buffer((char *)read_buff, RPMB_DATA_FRAME_SIZE);
#endif

		/* read_buff is returned in MSB First Byte Order, reverse it */
		reverse_array(read_buff, RPMB_DATA_FRAME_SIZE);

		read_data_packet = (struct rpmb_data_frame *)read_buff;

#if DEBUG
		pr_info("Result Register Value: 0x%x \r\n",
					read_data_packet->result);
#endif

		/* Check if Authentication is programmed and return if NOT */
		result = read_data_packet->result;
		if (result == RESULT_AUTH_KEY_NOT_PROGRAMMED) {
			pr_err("Authentication Key NOT programmed; result: 0x%x\n",
					result);
			ret = -EKEYNOTPROG;
			goto out;
		}

		/* Retry on GENERAL FAILURE */
		if (result == RESULT_GENERAL_FAILURE) {
			pr_err("Write Data General Failure. Retrying....\n");
			if (--retries) {
				pr_err("Retry count: %d\n", retries);
				goto write_retry;
			} else {
				pr_err("Retried %d times. No success."
						"Returning error to caller\n",
						RPMB_ACCESS_RETRY);
				ret = -EGENERR;
				goto out;
			}
		}

		/* Authenticate Counter value data packet */
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

		/* Compare the computed MAC against MAC in data frame */
		reverse_array(key_mac_buff, KEY_MAC_SIZE);
		ret = memcmp(key_mac_buff, read_data_packet->key_mac,
						KEY_MAC_SIZE);
		if (ret) {
			pr_err("%s: HMAC-SHA256 check failed. Data may be invalid",
					__func__);
			ret = -EDATACHKFAIL;
			goto out;
		}

		/* Check for other write errors */
		if (read_data_packet->req_resp_code ==
			RESPONSE_TYPE_AUTH_DATA_WRITE) {

			if (result & RESULT_COUNTER_EXPIRED_FLAG) {
				pr_err("Write Counter expired\n");
				ret = -ECOUNTERERR;
				goto out;
			} else {
				if (result == RESULT_ADDRESS_FAILURE) {
					pr_err("Write address Error\n");
					ret = -EADDRFAIL;
					goto out;
				}

				if (result == RESULT_AUTHENTICATION_FAILURE) {
					pr_err("Write Data Authentication Failure\n");
					ret = -EAUTHFAIL;
					goto out;
				}

				if (result == RESULT_COUNTER_FAILURE) {
					pr_err("Write Counter Mismatch Error\n");
					ret = -ECOUNTERERR;
					goto out;
				}

				if (result == RESULT_WRITE_FAILURE) {
					pr_err("Write Data Error\n");
					ret = -EWRITEFAIL;
					goto out;
				}
			}
		} else {
			pr_err("Write Data- Wrong response type\n");
			ret = -EOTHER;
			goto out;
		}

		data += PAYLOAD_SIZE;
		data_size -= PAYLOAD_SIZE;
		addr++;
		blkcnt--;
	} while (blkcnt);

out:
	kfree(read_buff);
	return ret;
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

	if (((sec_addr << 8) + len) > 0x20000) { /* 0x20000 = RPMB size */
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
			pr_err("Authentication Key NOT programmed; result: 0x%x\n",
					result);
			ret = -EKEYNOTPROG;
			goto out;
		}

		/* Retry on GENERAL FAILURE */
		if (result == RESULT_GENERAL_FAILURE) {
			pr_err("Read Data General Failure. Retrying....\n");
			if (--retries) {
				pr_err("Retry count: %d\n", retries);
				goto read_retry;
			} else {
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

static ssize_t
rpmb_program_key(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t n)
{
	int cmd;
	int ret = 0;

	if (sscanf(buf, "%d", &cmd) == 1) {
		ret = program_authentication_key();
		if (ret < 0) /* Any other failure, return error */
			pr_err("Authentication key Programming Failed\n");
		else
			pr_info("Done programming Authentication Key!\n");
	} else
		pr_err("Usage: echo [any_number] > "
				"/sys/emmc_rpmb_test/emmc_rpmb_program_key\n");

	return n;
}

static ssize_t
rpmb_get_counter(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t n)
{
	int cmd;
	int write_counter_val;

	if (sscanf(buf, "%d", &cmd) == 1) {
		write_counter_val = read_counter_value();
		if (write_counter_val < 0)
			pr_err("%s: Write counter value read failed\n",
								__func__);
		else
			pr_info("Counter Value: %x\n", write_counter_val);
	} else
		pr_err("Usage: echo [any_number] > "
				"/sys/emmc_rpmb_test/emmc_rpmb_get_counter\n");

	return n;
}

static ssize_t
rpmb_read(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t n)
{
	int addr, bytes;
	char *rd_buff;
	int ret = 0;

	if (sscanf(buf, "%x %d", &addr, &bytes) != 2) {
		pr_err("Usage: echo [half sector addr (in hex)] [num of bytes] > "
				"/sys/emmc_rpmb_test/emmc_rpmb_data_read\n");
		return n;
	}

	rd_buff = kmalloc(bytes, GFP_KERNEL);

	ret = authenticated_data_read(rd_buff, bytes, addr);
	if (ret < 0) {
		pr_err("Read Failure. Try again\n");
		kfree(rd_buff);
		return -1;
	}

	pr_info("Done Reading Data from RPMB addr: 0x%x\n", addr);
	dump_buffer((char *)rd_buff, bytes);

	kfree(rd_buff);
	return n;
}

static ssize_t
rpmb_write(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t n)
{
	int i;
	int addr, bytes, pattern;
	char *wr_buff;
	int ret = 0;

	if (sscanf(buf, "%x %d %x", &addr, &bytes, &pattern) != 3) {
		pr_err("Usage: echo [half sector addr in hex] [num of bytes] "
				"[byte pattern] > "
				"/sys/emmc_rpmb_test/emmc_rpmb_data_write\n");

		return n;
	}

	wr_buff = kmalloc(bytes, GFP_KERNEL);

	for (i = 0; i < bytes; i++)
		*(wr_buff + i) = pattern;

	ret = authenticated_data_write(wr_buff, bytes, addr);
	if (ret < 0) {
		pr_err("Write Failure. Try again\n");
		kfree(wr_buff);
		return -1;
	}

	pr_info("Done Writing Data to RPMB!\n");

	kfree(wr_buff);
	return n;
}

static DEVICE_ATTR(emmc_rpmb_program_key, 0666, NULL, rpmb_program_key);
static DEVICE_ATTR(emmc_rpmb_get_counter, 0666, NULL, rpmb_get_counter);
static DEVICE_ATTR(emmc_rpmb_data_read, 0666, NULL, rpmb_read);
static DEVICE_ATTR(emmc_rpmb_data_write, 0666, NULL, rpmb_write);

static struct attribute *emmc_rpmb_test_attrs[] = {
	&dev_attr_emmc_rpmb_program_key.attr,
	&dev_attr_emmc_rpmb_get_counter.attr,
	&dev_attr_emmc_rpmb_data_read.attr,
	&dev_attr_emmc_rpmb_data_write.attr,
	NULL,
};

static struct attribute_group emmc_rpmb_test_attr_group = {
	.attrs = emmc_rpmb_test_attrs,
};

static int __init emmc_rpmb_init(void)
{
	bio_buff = (void *)__get_free_page(GFP_KERNEL);
	pr_info("eMMC RPMB test driver initialized\n");
	emmc_rpmb_test_kobj = kobject_create_and_add("emmc_rpmb_test", NULL);
	if (!emmc_rpmb_test_kobj)
		return -ENOMEM;
	return sysfs_create_group(emmc_rpmb_test_kobj,
			&emmc_rpmb_test_attr_group);
}

static void __exit emmc_rpmb_exit(void)
{
	sysfs_remove_group(emmc_rpmb_test_kobj, &emmc_rpmb_test_attr_group);
}

module_init(emmc_rpmb_init);
module_exit(emmc_rpmb_exit);
