#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <asm/uaccess.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netfilter_ipv6.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/skbuff.h>
#include <linux/netlink.h>

#include "mms_block.h"

#ifdef CONFIG_PROC_FS
#include <linux/proc_fs.h>
#endif

static struct white_list_node *g_whitelistnode;
static int g_mms_netlink_pid;
static int g_mms_block_enabled;
static void send_netlinkevent_to_security_center(struct netlink_event *pevent);

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
		struct netlink_event mmsevent;
		if (kstrtoul(buf1+1, 10, &blocked_uid))
			return count;
		mmsevent.type = 0;
		mmsevent.uid = blocked_uid;
		send_netlinkevent_to_security_center(&mmsevent);
	} else if (*buf1 == '#') {
		kstrtoul(buf1+1, 10, &g_mms_block_enabled);
	} else {
		kstrtoul(buf1, 10, &g_mms_netlink_pid);
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

static void send_netlinkevent_to_security_center(struct netlink_event *pevent)
{
	struct sk_buff *skb;
	struct nlmsghdr *nlh;
	u8 *data;
	struct sock *sk = NULL;

	skb = nlmsg_new(sizeof(struct netlink_event), GFP_KERNEL);
	if (skb == NULL)
		return;

	nlh = __nlmsg_put(skb, 0, 0, 0, sizeof(struct netlink_event), 0);
	NETLINK_CB(skb).dst_group = 0;
	data = nlmsg_data(nlh);
	memcpy(data, pevent, sizeof(struct netlink_event));

	sk = netlink_kernel_create(&init_net, NETLINK_USERSOCK, NULL);
	if (sk == NULL)
		return;

	netlink_unicast(sk, skb, g_mms_netlink_pid, MSG_DONTWAIT);
	netlink_kernel_release(sk);
}

static unsigned int mms_block_ip_output(unsigned int hooknum,
	struct sk_buff *skb,
	const struct net_device *in,
	const struct net_device *out,
	int (*okfn)(struct sk_buff *))
{
	struct iphdr *iph;
	struct tcphdr *tcph;
	unsigned char *tcp_payload = NULL;
	uid_t sock_uid = 0;
	const struct file *filp;

	if (!g_mms_block_enabled)
		return NF_ACCEPT;

	if (!skb)
		return NF_ACCEPT;

	if (strnicmp(out->name, "rmnet", 5) != 0)
		return NF_ACCEPT;

	iph = ip_hdr(skb);
	if (iph->protocol != IPPROTO_TCP)
		return NF_ACCEPT;

	tcph = (struct tcphdr *)skb->transport_header;
	tcp_payload = (unsigned char *)tcph + tcph->doff*4;

	if ((unsigned int)tcp_payload+4 > (unsigned int)skb->end)
		return NF_ACCEPT;

	if (strnicmp(tcp_payload, "POST", 4) != 0)
		return NF_ACCEPT;

	if (strnstr(tcp_payload,
			"Content-Type: application/vnd.wap.mms-message", 300)) {
		if (skb->sk != NULL && skb->sk->sk_socket != NULL) {
			filp = skb->sk->sk_socket->file;
			if (filp != NULL)
				sock_uid = filp->f_cred->fsuid;
		}
		if (is_uid_in_white_list(sock_uid) == 0) {
			struct netlink_event event;
			event.type = 0; /* 0: MMS  1: Email*/
			event.uid = sock_uid;
			send_netlinkevent_to_security_center(&event);
			return NF_DROP;
		}
	}

	return NF_ACCEPT;
}

static struct nf_hook_ops mms_block_ip_ops[] = {
	{
		.hook =		mms_block_ip_output,
		.owner =	THIS_MODULE,
		.pf =		PF_INET,
		.hooknum =	NF_INET_LOCAL_OUT,
		.priority =	NF_IP_PRI_MANGLE,
	}
};

const struct file_operations white_list_fops = {
	.read = white_list_read,
	.write = white_list_write,
};

static int __init mms_block_init(void)
{
	int err = 0;

	err = nf_register_hooks(mms_block_ip_ops,
			ARRAY_SIZE(mms_block_ip_ops));
	if (err)
		return err;

#ifdef CONFIG_PROC_FS
	if (!proc_create("mms_white_list", 0666, NULL, &white_list_fops))
		return -ENOMEM;
#endif
	g_whitelistnode = NULL;
	g_mms_netlink_pid = 0;
	g_mms_block_enabled = 1;
	return 0;
}

static void __exit mms_block_exit(void)
{
	nf_unregister_hooks(mms_block_ip_ops, ARRAY_SIZE(mms_block_ip_ops));
#ifdef CONFIG_PROC_FS
	remove_proc_entry("mms_white_list", NULL);
#endif
}

module_init(mms_block_init);
module_exit(mms_block_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Mms block driver");

