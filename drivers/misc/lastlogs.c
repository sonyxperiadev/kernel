/*
* Copyright (C) 2012 Sony Mobile Communications AB.
* All rights, including trade secret rights, reserved.
*
*@ file drivers/misc/lastlogs.c
*
*
*/
#include <linux/init.h>
#include <linux/notifier.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/rdtags.h>
#include <linux/io.h>
#include "../../arch/arm/mach-msm/smd_private.h"

static struct device *dev;

struct last_logs {
	uint32_t    sig;
	uint32_t    size;
	uint8_t     data[0];
};

#define KMSGLOG_SIG 0x48144848
#define AMSSLOG_SIG 0x47327894

static char *kmsg_base;
static char *kmsg_buf;
static char *amss_base;
static char *amss_buf;
static size_t kmsg_size, amss_size;
static int ramdump_mode;

static int __init warm_boot(char *str)
{
	unsigned long res;

	if (!*str)
		return 0;

	if (!kstrtoul(str, 0 , &res)) {
		if (res == 0xC0DEDEAD || res == 0xABADBABE)
			ramdump_mode = 1;
	}

	return 1;
}
__setup("warmboot=", warm_boot);

static ssize_t amss_lastlog_read(struct file *file, char __user *buf,
					size_t len, loff_t *offset)
{
	loff_t pos = *offset;
	ssize_t count;

	if (pos >= amss_size)
		return 0;

	count = min(len, (size_t)(amss_size - pos));
	if (copy_to_user(buf, amss_buf + pos, count))
		return -EFAULT;

	*offset += count;
	return count;
}

static const struct file_operations amss_fops = {
	.owner = THIS_MODULE,
	.read = amss_lastlog_read,
};

static int export_amss_logs(void)
{
	struct proc_dir_entry *entry;
	struct last_logs *buffer = (struct last_logs *)amss_base;

	amss_buf = kzalloc(amss_size, GFP_KERNEL);
	if (amss_buf == NULL) {
		dev_err(dev,
			"Failed allocating amss buffer\n");
			return -ENOMEM;
	}

	amss_size = min(buffer->size, amss_size);
	memcpy(amss_buf, buffer->data, amss_size);
	entry = create_proc_entry("last_amsslog",
			S_IFREG | S_IRUGO, NULL);
	if (!entry) {
		dev_err(dev,
			"failed to create last_amsslog proc entry\n");
		kfree(amss_buf);
		amss_buf = NULL;
		return -ENOMEM;
	}

	entry->proc_fops = &amss_fops;
	entry->size = amss_size;
	return 0;
}

static int extract_amss_logs(void)
{
	struct last_logs *buffer = (struct last_logs *)amss_base;
	char *smem_errlog = NULL;
	size_t size;

	smem_errlog = smem_get_entry(SMEM_ERR_CRASH_LOG, &size);

	if (smem_errlog && !memcmp(smem_errlog, "ERR", 3)) {
		buffer->sig = AMSSLOG_SIG;

		if (size > (amss_size - (buffer->data - (uint8_t *)buffer)))
			buffer->size = amss_size -
				(buffer->data - (uint8_t *)buffer);
		else
			buffer->size = size;

		memcpy(buffer->data, smem_errlog, buffer->size);
	} else {
		dev_info(dev, "Failed to extract amss logs\n");
		return -EINVAL;
	}

	return 0;
}

static ssize_t kmsg_lastlog_read(struct file *file, char __user *buf,
				size_t len, loff_t *offset)
{
	loff_t pos = *offset;
	ssize_t count;

	if (pos >= kmsg_size)
		return 0;

	count = min(len, (size_t)(kmsg_size - pos));
	if (copy_to_user(buf, kmsg_buf + pos, count))
		return -EFAULT;

	*offset += count;
	return count;
}

static const struct file_operations kmsg_fops = {
	.owner = THIS_MODULE,
	.read = kmsg_lastlog_read,
};

static int export_kmsg_logs(void)
{
	struct proc_dir_entry *entry;
	struct last_logs *buffer = (struct last_logs *)kmsg_base;

	kmsg_buf = kzalloc(kmsg_size, GFP_KERNEL);
	if (kmsg_buf == NULL) {
		dev_err(dev,
			"failed allocating kmsg buffer\n");
		return -ENOMEM;
	}

	kmsg_size = min(buffer->size, kmsg_size);
	memcpy(kmsg_buf, buffer->data, kmsg_size);
	entry = create_proc_entry("last_kmsg",
			S_IFREG | S_IRUGO, NULL);
	if (!entry) {
		dev_err(dev,
			"failed to create last_kmsg proc entry\n");
		kfree(kmsg_buf);
		kmsg_buf = NULL;
		return -ENOMEM;
	}

	entry->proc_fops = &kmsg_fops;
	entry->size = kmsg_size;
	return 0;
}

#define NR_LOG_INFO 3
#define LOG_INFO_LEN 32

static int extract_last_kmsg(void)
{
	struct last_logs *buffer = (struct last_logs *)kmsg_base;
	unsigned long log_addr[NR_LOG_INFO];
	char log_data[NR_LOG_INFO][LOG_INFO_LEN];
	int i, wrap, size, err, index = 0;
	char *log_buf;
	unsigned int *log_buf_len;
	unsigned int *log_end;
	ssize_t buf_len;
	char *logbuf_name[] = {
		"__log_buf",
		"log_end",
		"log_buf_len",
	};

	for (i = 0; i < NR_LOG_INFO; i++) {
		/* Indicate max size */
		size = LOG_INFO_LEN;
		err = rdtags_get_tag_data(logbuf_name[i], log_data[i], &size);
		if (err < 0) {
			dev_err(dev, "tag %s not found: %d\n",
				logbuf_name[i], err);
			return -EINVAL;
		}

		err = kstrtoul(log_data[i], 16, &log_addr[i]);
		if (err < 0) {
			dev_err(dev, "Failed kstrtoul %s %lx\n",
				log_data[i], log_addr[i]);
			return -EINVAL;
		}

		dev_dbg(dev, "logbuf tags verify %s %lx\n",
				log_data[i], log_addr[i]);
	}

	log_buf_len = (unsigned int *)ioremap(log_addr[2], sizeof(int));
	if (!log_buf_len) {
		dev_err(dev, "log_buf_len: Ioremap Failed !!\n");
		goto exit1;
	}

	dev_dbg(dev, "log_buf_len = %d\n", *log_buf_len);

	log_buf = (char *)ioremap(log_addr[0], (*log_buf_len));
	if (!log_buf) {
		dev_err(dev, "log_buf: Ioremap Failed!!\n");
		goto exit2;
	}

	log_end = (unsigned int *)ioremap(log_addr[1], sizeof(unsigned));
	if (!log_end) {
		dev_err(dev, "log_end: Ioremap Failed!!\n");
		goto exit3;
	}

	if ((*log_end) > (*log_buf_len))
		wrap = 1;
	else
		wrap = 0;

	buf_len = *log_buf_len;
	if (buf_len > (kmsg_size - (buffer->data - (uint8_t *)buffer)))
		buf_len = kmsg_size - (buffer->data - (uint8_t *)buffer);

	buffer->sig = KMSGLOG_SIG;
	buffer->size = buf_len;

	index = (*log_end & (buf_len - 1));

	if (wrap) {
		memcpy(buffer->data, &log_buf[index],
				(buffer->size - index));
		memcpy(&buffer->data[buffer->size - index],
						log_buf, index);
	} else {
		memcpy(buffer->data, log_buf, index);
		buffer->size = index;
	}


	iounmap(log_buf);
	iounmap(log_buf_len);
	iounmap(log_end);
	return 0;
exit3:
	if (log_buf)
		iounmap(log_buf);

	if (log_buf_len)
		iounmap(log_buf_len);

exit2:
	if (log_buf_len)
		iounmap(log_buf_len);

exit1:
	return -ENOMEM;

}

int log_buf_copy(char *dest, int idx, int len);
static int dump_kernel_logs(struct notifier_block *this, unsigned long event,
		void *ptr)
{
	struct last_logs *buffer;
	int size;
	int log_length = (1 << CONFIG_LOG_BUF_SHIFT);

	buffer = (struct last_logs *)kmsg_base;
	size = log_buf_copy(buffer->data, 0, log_length);
	if (size < 0)
		goto exit;

	buffer->sig = KMSGLOG_SIG;
	buffer->size = size;

exit:
	return NOTIFY_DONE;
}

static struct notifier_block lastlogs_panic_block = {
	.notifier_call = dump_kernel_logs,
};

static int lastlogs_driver_probe(struct platform_device *pdev)
{
	struct resource *kmsg_res, *amss_res;
	struct last_logs *kbuffer, *abuffer;
	int ret = 0;

	dev = &pdev->dev;
	kmsg_res = platform_get_resource_byname(pdev,
			IORESOURCE_MEM, "last_kmsg");
	if (!kmsg_res || !kmsg_res->start) {
		dev_err(dev, "last_kmsg resource invalid/absent\n");
		ret = -ENODEV;
		goto amss_log;
	}

	kmsg_size = kmsg_res->end - kmsg_res->start + 1;
	kmsg_base = (char *)ioremap(kmsg_res->start, kmsg_size);
	if (kmsg_base == NULL) {
		dev_err(dev, "failed to map last_kmsg memory\n");
		return -ENOMEM;
	}

	kbuffer = (struct last_logs *)kmsg_base;

	if (ramdump_mode) {
		if (!extract_last_kmsg()) {
			dev_info(dev, "kmsg logs extracted\n");
			export_kmsg_logs();
		}
	} else if (kbuffer->sig == KMSGLOG_SIG) {
		dev_info(dev, "kmsg logs found\n");
		export_kmsg_logs();
		memset(kmsg_base, 0x0, kmsg_size);
	}

amss_log:
	amss_res = platform_get_resource_byname(pdev,
				IORESOURCE_MEM, "last_amsslog");
	if (!amss_res || !amss_res->start) {
		dev_err(dev, "last_amsslog resource invalid/absent\n");
		return -ENODEV;
	}

	amss_size = amss_res->end - amss_res->start + 1;
	amss_base = (char *)ioremap(amss_res->start, amss_size);
	if (!amss_base) {
		dev_err(dev, "failed to map last_amsslog\n");
		return -ENOMEM;
	}

	abuffer = (struct last_logs *)amss_base;

	if (ramdump_mode) {
		if (!extract_amss_logs()) {
			dev_info(dev, "Amss logs extracted\n");
			export_amss_logs();
		}
	} else if (abuffer->sig == AMSSLOG_SIG) {
		dev_info(dev, "Amss logs Found\n");
		export_amss_logs();
		memset(amss_base, 0x0, amss_size);
	}
	iounmap(amss_base);
	atomic_notifier_chain_register(&panic_notifier_list,
			&lastlogs_panic_block);
	return ret;
}

static struct platform_driver lastlogs_driver = {
	.probe = lastlogs_driver_probe,
	.driver = {
		.name   = "last_logs",
	},
};

static int __init lastlogs_module_init(void)
{
	int err;
	err = platform_driver_register(&lastlogs_driver);
	return err;
}

MODULE_AUTHOR("Anirudh Madnurkar <anirudh.x.madnurkar@sonymobile.com>");
MODULE_DESCRIPTION("last crash logs");
MODULE_LICENSE("GPL V2");

module_init(lastlogs_module_init)
