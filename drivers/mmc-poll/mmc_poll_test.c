/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*	@file	arch/arm/plat-bcmap/sysfs.c
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

/*
 * SYSFS infrastructure specific Broadcom SoCs
 */
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/sysfs.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/slab.h>

#include <linux/mmc-poll/mmc_poll.h>
#include "kona_mmc.h"

struct kobject *mmc_test_kobj;

#define APANIC_PARTITION_BLOCK_START_ADDRESS	(0x70800) /* (0x1C000) */
#define BLOCK_SIZE				(512)
struct mmc *mmc;
unsigned char *wr_buff;
unsigned char *rd_buff;

static void print_mmcinfo(struct mmc *mmc)
{
	pr_info("Device: %s\n", mmc->name);
	pr_info("Manufacturer ID: %x\n", mmc->cid[0] >> 24);
	pr_info("OEM: %x\n", (mmc->cid[0] >> 8) & 0xffff);
	pr_info("Name: %c%c%c%c%c\n", mmc->cid[0] & 0xff,
			(mmc->cid[1] >> 24), (mmc->cid[1] >> 16) & 0xff,
			(mmc->cid[1] >> 8) & 0xff, mmc->cid[1] & 0xff);

	pr_info("Tran Speed: %d\n", mmc->tran_speed);
	pr_info("Rd Block Len: %d\n", mmc->read_bl_len);

	pr_info("%s version %d.%d\n", IS_SD(mmc) ? "SD" : "MMC",
			(mmc->version >> 4) & 0xf, mmc->version & 0xf);

	pr_info("High Capacity: %s\n", mmc->high_capacity ? "Yes" : "No");
	pr_info("Capacity: %lld\n", mmc->capacity);

	pr_info("Bus Width: %d-bit\n", mmc->bus_width);
}

static void fill_buff(unsigned char *ptr, unsigned long size,
			unsigned char val)
{
	int i;
	for (i = 0; i < size; i++)
		*(ptr + i) = val;
	return;
}

static void dump_buff(unsigned char *ptr, unsigned long size)
{
	int i;
	for (i = 0; i < size; i++) {
		if (i%10 == 0)
			pr_info("\n");
		pr_info("0x%x ", *(ptr+i));
	}
	return;
}

static int rhearay_mmc_init(void)
{
	int status;

	mmc_initialize();

	/* For eMMC initilaize the instance number 2 and for SD Card use 1 */
	status = kona_mmc_init(2);
	/* status = kona_mmc_init (1); */
	if (status  < 0) {
		pr_err("rhearay_mmc_init failed\n");
		return -1;
	}
	pr_info("kona_mmc_init done\n");

	/* Use 0 for the first device registered via kona_mmc_init etc */
	mmc = find_mmc_device(0);
	if (mmc == NULL) {
		pr_err("No mmc device found\n");
		return -1;
	}

	if (mmc_init(mmc) < 0) {
		pr_err("mmc init failed\n");
		return -1;
	}
	pr_info("mmc_init done\n");

	print_mmcinfo(mmc);

#if 0
	/* read dummy data */
	fill_buff((unsigned char *)rd_buff, sizeof(rd_buff),
	(unsigned char)0xa5);

	mmc->block_dev.block_read(0, APANIC_PARTITION_BLOCK_START_ADDRESS,
	1, rd_buff);
	if (ret == 0)
		pr_info("kona_mmc_poll_read: block_read returned 0\n");
	else
		pr_info("kona_mmc_poll_read: bock_read returned %d\n", ret);

	dump_buff((unsigned char *)rd_buff, 512);
#endif
	return 0;
}

static ssize_t
kona_mmc_poll_init(struct device *dev, struct device_attribute *attr,
	  const char *buf, size_t n)
{
	int cmd;

	if (sscanf(buf, "%d", &cmd) == 1)
		rhearay_mmc_init();
	else
		pr_info("Usage: echo [any_number] > "
			"/sys/mmc_test/mmc_poll_init\n");

	return n;
}

static ssize_t
kona_mmc_poll_read(struct device *dev, struct device_attribute *attr,
	  const char *buf, size_t n)
{
	unsigned long dev_num, blk;
	int cnt;
	int ret;

	if (sscanf(buf, "%d", &cnt) != 1) {
		pr_info("Usage: echo [number_of_blocks_to_read] > "
			"/sys/mmc_test/mmc_poll_read \r\n");
		return n;
	}

	blk = APANIC_PARTITION_BLOCK_START_ADDRESS;
	dev_num = 0;

	pr_info("Reading %d blocks starting from block number %ld \r\n",
		cnt, blk);

	rd_buff = kmalloc(cnt * BLOCK_SIZE , GFP_KERNEL);
	fill_buff((unsigned char *)rd_buff, cnt * BLOCK_SIZE,
		   (unsigned char)0xa5);

	ret = mmc->block_dev.block_read(dev_num, blk, cnt, rd_buff);
	if (ret == 0)
		pr_info("kona_mmc_poll_read: block_read returned 0 \r\n");
	else
		pr_info("kona_mmc_poll_read: bock_read returned %d \r\n", ret);

	dump_buff((unsigned char *)rd_buff, cnt * BLOCK_SIZE);

	kfree(rd_buff);
	return n;
}

static ssize_t
kona_mmc_poll_write(struct device *dev, struct device_attribute *attr,
	  const char *buf, size_t n)
{
	unsigned long dev_num, blk;
	int ret;
	int cnt, pattern;

	if (sscanf(buf, "%d %x", &cnt, &pattern) != 2) {
		pr_info("Usage: echo [num_blks_to_write] [byte_pattern] > "
			"/sys/mmc_test/mmc_poll_write\n");
		return n;
	}

	blk = APANIC_PARTITION_BLOCK_START_ADDRESS;
	dev_num = 0;

	pr_info("Writing %d blocks starting from block nu %ld with pattern "
		"0x%x\n", cnt, blk, pattern);

	wr_buff = kmalloc(cnt * BLOCK_SIZE , GFP_KERNEL);
	fill_buff((unsigned char *)wr_buff, cnt * BLOCK_SIZE,
		   (unsigned char)pattern);

	ret = mmc->block_dev.block_write(dev_num, blk, cnt, wr_buff);
	if (ret == 0)
		pr_info("kona_mmc_poll_write: block_read returned 0\n");
	else
		pr_info("kona_mmc_poll_write: bock_read returned %d\n", ret);

	kfree(wr_buff);
	return n;
}

static DEVICE_ATTR(mmc_poll_init, 0644, NULL, kona_mmc_poll_init);
static DEVICE_ATTR(mmc_poll_read, 0644, NULL, kona_mmc_poll_read);
static DEVICE_ATTR(mmc_poll_write, 0644, NULL, kona_mmc_poll_write);

static struct attribute *mmc_test_attrs[] = {
	&dev_attr_mmc_poll_init.attr,
	&dev_attr_mmc_poll_read.attr,
	&dev_attr_mmc_poll_write.attr,
	NULL,
};

static struct attribute_group mmc_test_attr_group = {
	.attrs = mmc_test_attrs,
};

static int __init mmc_test_sysfs_init(void)
{
	mmc_test_kobj = kobject_create_and_add("mmc_test", NULL);
	if (!mmc_test_kobj)
		return -ENOMEM;
	return sysfs_create_group(mmc_test_kobj, &mmc_test_attr_group);
}

static void __exit mmc_test_sysfs_exit(void)
{
	sysfs_remove_group(mmc_test_kobj, &mmc_test_attr_group);
}

module_init(mmc_test_sysfs_init);
module_exit(mmc_test_sysfs_exit);
