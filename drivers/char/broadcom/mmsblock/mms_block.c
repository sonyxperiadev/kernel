#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <asm/uaccess.h>
#include "mms_block.h"

#ifdef CONFIG_PROC_FS
#include <linux/proc_fs.h>
#endif

ssize_t white_list_read(struct file *file, char __user *buf, size_t count,
	loff_t *offset)
{
	struct white_list_node *head;
	char buf_tmp[512];
	unsigned long len = 0;

	head = g_whitelistnode;
	if (head == NULL) {
		len += snprintf(buf_tmp+len, sizeof(buf_tmp)-len, "null\n");
	} else {
		len += snprintf(buf_tmp+len, sizeof(buf_tmp)-len, "%d ",
			head->uid);
		while (head->next != NULL) {
			head = head->next;
			len += snprintf(buf_tmp+len, sizeof(buf_tmp)-len, "%d ",
				head->uid);
		}
		len += snprintf(buf_tmp+len, sizeof(buf_tmp)-len, "\n");
	}
	return simple_read_from_buffer(buf, count, offset, buf_tmp, len);
}

ssize_t white_list_write(struct file *file, const char __user *buf,
	size_t count, loff_t *offset)
{
	struct white_list_node *insert;
	struct white_list_node *head;
	struct white_list_node *tmp;
	unsigned long uid_in = 0;
	char buf1[16] = {0};

	if (count > 16)
		return count;

	if (copy_from_user(buf1, buf, count))
		return count;

	if (*buf1 == '+') {
		if (kstrtoul(buf1+1, 10, &uid_in))
			return count;

		if (g_whitelistnode == NULL) {
			g_whitelistnode = kmalloc(
				sizeof(struct white_list_node), GFP_KERNEL);
			g_whitelistnode->uid = uid_in;
			g_whitelistnode->next = NULL;
			return count;
		}

		head = g_whitelistnode;

		if (head->uid == uid_in)
			return count;

		while (head->next != NULL) {
			head = head->next;
			if (head->uid == uid_in)
				return count;
		}

		insert = kmalloc(sizeof(struct white_list_node), GFP_KERNEL);
		insert->uid = uid_in;
		insert->next = NULL;
		head->next = insert;
	} else if (*buf1 == '-') {
		if (kstrtoul(buf1+1, 10, &uid_in))
			return count;

		head = g_whitelistnode;
		if (head == NULL)
			return count;
		if (head->uid == uid_in) {
			g_whitelistnode = head->next;
			kfree(head);
			return count;
		}
		while (head->next != NULL) {
			tmp = head;
			head = head->next;

			if (head->uid == uid_in) {
				tmp->next = head->next;
				kfree(head);
				return count;
			}
		}
	} else if (*buf1 == '*') {
		int blocked_uid = 0;
		struct mms_event mmsevent;
		if (kstrtoul(buf1+1, 10, &blocked_uid))
			return count;
		mmsevent.uid = blocked_uid;
		send_event_to_security_center(&mmsevent);
	} else {
		kstrtoul(buf1, 10, &g_pid);
	}
	return count;
}

int is_uid_in_white_list(int uid)
{
	struct white_list_node *head;

	head = g_whitelistnode;
	if (head == NULL)
		return 0;

	if (head->uid != uid) {
		while (head->next != NULL) {
			head = (struct white_list_node *)head->next;
			if (head->uid == uid)
				return 1;
		}
		return 0;
	}
	return 1;
}

int get_com_pid(void)
{
	return g_pid;
}

const struct file_operations white_list_fops = {
	.read = white_list_read,
	.write = white_list_write,
};

static int __init mms_block_init(void)
{
#ifdef CONFIG_PROC_FS
	if (!proc_create("mms_white_list", 0666, NULL, &white_list_fops))
		return -ENOMEM;
#endif
	g_whitelistnode = NULL;
	g_pid = 0;
	return 0;
}

static void __exit mms_block_exit(void)
{
#ifdef CONFIG_PROC_FS
	remove_proc_entry("mms_white_list", NULL);
#endif
}

module_init(mms_block_init);
module_exit(mms_block_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Mms block driver");

