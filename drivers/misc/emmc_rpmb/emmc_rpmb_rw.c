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
#include <linux/blkdev.h>
#include <linux/stat.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/genhd.h>
#include <linux/completion.h>
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

/*keybox key and data address in RPMB partition*/
#define KEYBOX_KEY_RPMB_START_ADDR	0xe
#define KEYBOX_KEY_RPMB_LEN			256
#define KEYBOX_DATA_RPMB_START_ADDR	0xf
#define KEYBOX_DATA_RPMB_LEN			256

static void *bio_buff;
static struct block_device *bdev[TOTAL_BLK_DEV];

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

static void start_queue(int index)
{
	unsigned long flags;
	struct request_queue *q;

	if (bdev[index]) {
		q = bdev_get_queue(bdev[index]);
		if (!q) {
			pr_err("queue not found bdev[index]=%d\n",
				index);
			return;
		}
		spin_lock_irqsave(q->queue_lock, flags);
		blk_start_queue(q);
		spin_unlock_irqrestore(q->queue_lock, flags);
		blkdev_put(bdev[index], FMODE_READ);
		bdev[index] = NULL;
	}
}

static void stop_queue(int index, char *path)
{
	unsigned long flags;
	struct request_queue *q;

	bdev[index] = blkdev_get_by_path(path, FMODE_READ,
			stop_queue);
	if (IS_ERR(bdev[index])) {
		pr_err("failed to get block device %s (%ld)\n",
		      path, PTR_ERR(bdev[index]));
		bdev[index] = NULL;
		return;
	}
	q = bdev_get_queue(bdev[index]);
	if (!q) {
		pr_err("queue not found bdev[index]=%d\n", index);
		return;
	}
	spin_lock_irqsave(q->queue_lock, flags);
	blk_stop_queue(q);
	spin_unlock_irqrestore(q->queue_lock, flags);
}

static void rpmb_access_begin(void)
{
	mutex_lock(&rpmb_mutex);

	stop_queue(MAIN_DEV_NODE, MAIN_DEV_NODE_PATH);
	stop_queue(BOOT0_DEV_NODE, BOOT0_DEV_NODE_PATH);
	stop_queue(BOOT1_DEV_NODE, BOOT1_DEV_NODE_PATH);
}

static void rpmb_access_end(void)
{
	start_queue(BOOT1_DEV_NODE);
	start_queue(BOOT0_DEV_NODE);
	start_queue(MAIN_DEV_NODE);

	mutex_unlock(&rpmb_mutex);
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
	if (!read_buff)
		return -ENOMEM;

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
	if (!read_buff)
		return -ENOMEM;

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

	rpmb_access_begin();

	ret = authenticated_data_read(buff, len, CKDATA_RPMB_START_ADDR);
	if (ret < 0) {
		pr_err("CKData Block read failure\n");
		goto err_out;
	}

	pr_info("Done Reading %d bytes of CKData from RPMB\n", len);

err_out:
	rpmb_access_end();
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

	rpmb_access_begin();

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
	rpmb_access_end();
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

	rpmb_access_begin();

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
	rpmb_access_end();
	if (ret1 || ret2)
		return ret1 ? ret1 : ret2;
	else
		return 0;
}
EXPORT_SYMBOL(read_imei2);

/**
 * @brief	readkeyboxKey : Reads keybox key from eMMC RPMB partition.
 * @param	buff	: Kernel Space buffer to which the data will be read.
 * @param	len     : number of bytes to read.
 * @return	Returns -ve number on error and 0 on success
 *
 * @description	This function reads the Keybox key from the RPMB partition.
 */
int readkeyboxKey(char *buff, int len)
{
	int ret = 0;

	if ((NULL == buff) || (0 == len)) {
		pr_err("readkeyboxKey: buffer or length invalid \r\n");
		return -1;
	}

	if (len > KEYBOX_KEY_RPMB_LEN) {
		pr_err("Cannot read more than keybox block size\n");
		return -1;
	}

	rpmb_access_begin();

	ret = authenticated_data_read(buff, len, KEYBOX_KEY_RPMB_START_ADDR);
	if (ret < 0) {
		pr_err("readkeyboxKey failure\n");
		goto err_out;
	}

	pr_info("Done Reading %d bytes of keybox from RPMB\n", len);

err_out:
	rpmb_access_end();
	return ret;
}
EXPORT_SYMBOL(readkeyboxKey);

/**
 * @brief	readkeyboxData : Reads keybox data from eMMC RPMB partition.
 * @param	buff	: Kernel Space buffer to which the data will be read.
 * @param	len     : number of bytes to read.
 * @return	Returns -ve number on error and 0 on success
 *
 * @description	This function reads the Keybox data from the RPMB partition.
 */
int readkeyboxData(char *buff, int len)
{
	int ret = 0;

	if ((NULL == buff) || (0 == len)) {
		pr_err("readkeyboxData: buffer or length invalid \r\n");
		return -1;
	}

	if (len > KEYBOX_DATA_RPMB_LEN) {
		pr_err("Cannot read more than keybox block size\n");
		return -1;
	}

	rpmb_access_begin();

	ret = authenticated_data_read(buff, len, KEYBOX_DATA_RPMB_START_ADDR);
	if (ret < 0) {
		pr_err("readkeyboxData failure\n");
		goto err_out;
	}

	pr_info("Done Reading %d bytes of keybox from RPMB\n", len);

err_out:
	rpmb_access_end();
	return ret;
}
EXPORT_SYMBOL(readkeyboxData);

/**
 * @brief	writekeyboxData : Writes keybox data from eMMC RPMB partition.
 * @param	buff	: Kernel Space buffer to which the data will be write to RPMB.
 * @param	len     : number of bytes to write.
 * @return	Returns -ve number on error and 0 on success
 *
 * @description	This function Writes the Keybox data from the RPMB partition.
 */
int writekeyboxData(char *buff, int len)
{
	int ret = 0;

	if ((NULL == buff) || (0 == len)) {
		pr_err("writekeyboxData: buffer or length invalid \r\n");
		return -1;
	}

	if (len > KEYBOX_DATA_RPMB_LEN) {
		pr_err("Cannot write more than keybox block size\n");
		return -1;
	}

	rpmb_access_begin();

	ret = authenticated_data_write(buff, len, KEYBOX_DATA_RPMB_START_ADDR);
	if (ret < 0) {
		pr_err("WwritekeyboxData failure\n");
		goto err_out;
	}
	pr_info("Done Writing %d bytes of keybox from RPMB\n", len);

err_out:
	rpmb_access_end();
	return ret;
}
EXPORT_SYMBOL(writekeyboxData);


/* Init routine */
int __init emmc_rpmb_rw_init(void)
{
	/* Allocate 1-page memory for BIO */
	bio_buff = (void *)__get_free_page(GFP_KERNEL);
	pr_info("eMMC RPMB rw driver initialized\n");

	return 0;
}

module_init(emmc_rpmb_rw_init);
