#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/skbuff.h>
#include <linux/netfilter.h>
#include <linux/seq_file.h>
#include <net/protocol.h>
#include <net/netfilter/nf_log.h>

#include "nf_internals.h"

/* Internal logging interface, which relies on the real
   LOG target modules */

#define NF_LOG_PREFIXLEN		128
#define NFLOGGER_NAME_LEN		64

static struct list_head nf_loggers_l[NFPROTO_NUMPROTO] __read_mostly;
static DEFINE_MUTEX(nf_log_mutex);

static struct nf_logger *__find_logger(int pf, const char *str_logger)
{
	struct nf_logger *t;

	list_for_each_entry(t, &nf_loggers_l[pf], list[pf]) {
		if (!strnicmp(str_logger, t->name, strlen(t->name)))
			return t;
	}

	return NULL;
}

void nf_log_set(struct net *net, u_int8_t pf, const struct nf_logger *logger)
{
	const struct nf_logger *log;

	if (pf == NFPROTO_UNSPEC)
		return;

	mutex_lock(&nf_log_mutex);
	log = rcu_dereference_protected(net->nf.nf_loggers[pf],
					lockdep_is_held(&nf_log_mutex));
	if (log == NULL)
		rcu_assign_pointer(net->nf.nf_loggers[pf], logger);

	mutex_unlock(&nf_log_mutex);
}
EXPORT_SYMBOL(nf_log_set);

void nf_log_unset(struct net *net, const struct nf_logger *logger)
{
	int i;
	const struct nf_logger *log;

	mutex_lock(&nf_log_mutex);
	for (i = 0; i < NFPROTO_NUMPROTO; i++) {
		log = rcu_dereference_protected(net->nf.nf_loggers[i],
				lockdep_is_held(&nf_log_mutex));
		if (log == logger)
			RCU_INIT_POINTER(net->nf.nf_loggers[i], NULL);
	}
	mutex_unlock(&nf_log_mutex);
	synchronize_rcu();
}
EXPORT_SYMBOL(nf_log_unset);

/* return EEXIST if the same logger is registered, 0 on success. */
int nf_log_register(u_int8_t pf, struct nf_logger *logger)
{
	int i;

	if (pf >= ARRAY_SIZE(init_net.nf.nf_loggers))
		return -EINVAL;

	for (i = 0; i < ARRAY_SIZE(logger->list); i++)
		INIT_LIST_HEAD(&logger->list[i]);

	mutex_lock(&nf_log_mutex);

	if (pf == NFPROTO_UNSPEC) {
		for (i = NFPROTO_UNSPEC; i < NFPROTO_NUMPROTO; i++)
			list_add_tail(&(logger->list[i]), &(nf_loggers_l[i]));
	} else {
		/* register at end of list to honor first register win */
		list_add_tail(&logger->list[pf], &nf_loggers_l[pf]);
	}

	mutex_unlock(&nf_log_mutex);

	return 0;
}
EXPORT_SYMBOL(nf_log_register);

void nf_log_unregister(struct nf_logger *logger)
{
	int i;

	mutex_lock(&nf_log_mutex);
	for (i = 0; i < NFPROTO_NUMPROTO; i++)
		list_del(&logger->list[i]);
	mutex_unlock(&nf_log_mutex);
}
EXPORT_SYMBOL(nf_log_unregister);

int nf_log_bind_pf(struct net *net, u_int8_t pf,
		   const struct nf_logger *logger)
{
	if (pf >= ARRAY_SIZE(net->nf.nf_loggers))
		return -EINVAL;
	mutex_lock(&nf_log_mutex);
	if (__find_logger(pf, logger->name) == NULL) {
		mutex_unlock(&nf_log_mutex);
		return -ENOENT;
	}
	rcu_assign_pointer(net->nf.nf_loggers[pf], logger);
	mutex_unlock(&nf_log_mutex);
	return 0;
}
EXPORT_SYMBOL(nf_log_bind_pf);

void nf_log_unbind_pf(struct net *net, u_int8_t pf)
{
	if (pf >= ARRAY_SIZE(net->nf.nf_loggers))
		return;
	mutex_lock(&nf_log_mutex);
	RCU_INIT_POINTER(net->nf.nf_loggers[pf], NULL);
	mutex_unlock(&nf_log_mutex);
}
EXPORT_SYMBOL(nf_log_unbind_pf);

void nf_log_packet(struct net *net,
		   u_int8_t pf,
		   unsigned int hooknum,
		   const struct sk_buff *skb,
		   const struct net_device *in,
		   const struct net_device *out,
		   const struct nf_loginfo *loginfo,
		   const char *fmt, ...)
{
	va_list args;
	char prefix[NF_LOG_PREFIXLEN];
	const struct nf_logger *logger;

	rcu_read_lock();
	logger = rcu_dereference(net->nf.nf_loggers[pf]);
	if (logger) {
		va_start(args, fmt);
		vsnprintf(prefix, sizeof(prefix), fmt, args);
		va_end(args);
		logger->logfn(net, pf, hooknum, skb, in, out, loginfo, prefix);
	}
	rcu_read_unlock();
}
EXPORT_SYMBOL(nf_log_packet);

#ifdef CONFIG_PROC_FS
static void *seq_start(struct seq_file *seq, loff_t *pos)
{
	struct net *net = seq_file_net(seq);

	mutex_lock(&nf_log_mutex);

	if (*pos >= ARRAY_SIZE(net->nf.nf_loggers))
		return NULL;

	return pos;
}

static void *seq_next(struct seq_file *s, void *v, loff_t *pos)
{
	struct net *net = seq_file_net(s);

	(*pos)++;

	if (*pos >= ARRAY_SIZE(net->nf.nf_loggers))
		return NULL;

	return pos;
}

static void seq_stop(struct seq_file *s, void *v)
{
	mutex_unlock(&nf_log_mutex);
}

static int seq_show(struct seq_file *s, void *v)
{
	loff_t *pos = v;
	const struct nf_logger *logger;
	struct nf_logger *t;
	int ret;
	struct net *net = seq_file_net(s);

	logger = rcu_dereference_protected(net->nf.nf_loggers[*pos],
					   lockdep_is_held(&nf_log_mutex));

	if (!logger)
		ret = seq_printf(s, "%2lld NONE (", *pos);
	else
		ret = seq_printf(s, "%2lld %s (", *pos, logger->name);

	if (ret < 0)
		return ret;

	list_for_each_entry(t, &nf_loggers_l[*pos], list[*pos]) {
		ret = seq_printf(s, "%s", t->name);
		if (ret < 0)
			return ret;
		if (&t->list[*pos] != nf_loggers_l[*pos].prev) {
			ret = seq_printf(s, ",");
			if (ret < 0)
				return ret;
		}
	}

	return seq_printf(s, ")\n");
}

static const struct seq_operations nflog_seq_ops = {
	.start	= seq_start,
	.next	= seq_next,
	.stop	= seq_stop,
	.show	= seq_show,
};

static int nflog_open(struct inode *inode, struct file *file)
{
	return seq_open_net(inode, file, &nflog_seq_ops,
			    sizeof(struct seq_net_private));
}

static const struct file_operations nflog_file_ops = {
	.owner	 = THIS_MODULE,
	.open	 = nflog_open,
	.read	 = seq_read,
	.llseek	 = seq_lseek,
	.release = seq_release_net,
};


#endif /* PROC_FS */

#ifdef CONFIG_SYSCTL
static char nf_log_sysctl_fnames[NFPROTO_NUMPROTO-NFPROTO_UNSPEC][3];
static struct ctl_table nf_log_sysctl_table[NFPROTO_NUMPROTO+1];

static int nf_log_proc_dostring(ctl_table *table, int write,
			 void __user *buffer, size_t *lenp, loff_t *ppos)
{
	const struct nf_logger *logger;
	char buf[NFLOGGER_NAME_LEN];
	size_t size = *lenp;
	int r = 0;
	int tindex = (unsigned long)table->extra1;
	struct net *net = table->extra2;

	if (write) {
		if (size > sizeof(buf))
			size = sizeof(buf);
		if (copy_from_user(buf, buffer, size))
			return -EFAULT;

		if (!strcmp(buf, "NONE")) {
			nf_log_unbind_pf(net, tindex);
			return 0;
		}
		mutex_lock(&nf_log_mutex);
		logger = __find_logger(tindex, buf);
		if (logger == NULL) {
			mutex_unlock(&nf_log_mutex);
			return -ENOENT;
		}
		rcu_assign_pointer(net->nf.nf_loggers[tindex], logger);
		mutex_unlock(&nf_log_mutex);
	} else {
		mutex_lock(&nf_log_mutex);
		logger = rcu_dereference_protected(net->nf.nf_loggers[tindex],
						   lockdep_is_held(&nf_log_mutex));
		if (!logger)
			table->data = "NONE";
		else
			table->data = logger->name;
		r = proc_dostring(table, write, buffer, lenp, ppos);
		mutex_unlock(&nf_log_mutex);
	}

	return r;
}

static int netfilter_log_sysctl_init(struct net *net)
{
	int i;
	struct ctl_table *table;

	table = nf_log_sysctl_table;
	if (!net_eq(net, &init_net)) {
		table = kmemdup(nf_log_sysctl_table,
				 sizeof(nf_log_sysctl_table),
				 GFP_KERNEL);
		if (!table)
			goto err_alloc;
	} else {
		for (i = NFPROTO_UNSPEC; i < NFPROTO_NUMPROTO; i++) {
			snprintf(nf_log_sysctl_fnames[i],
				 3, "%d", i);
			nf_log_sysctl_table[i].procname	=
				nf_log_sysctl_fnames[i];
			nf_log_sysctl_table[i].maxlen =
				NFLOGGER_NAME_LEN * sizeof(char);
			nf_log_sysctl_table[i].mode = 0644;
			nf_log_sysctl_table[i].proc_handler =
				nf_log_proc_dostring;
			nf_log_sysctl_table[i].extra1 =
				(void *)(unsigned long) i;
		}
	}

	for (i = NFPROTO_UNSPEC; i < NFPROTO_NUMPROTO; i++)
		table[i].extra2 = net;

	net->nf.nf_log_dir_header = register_net_sysctl(net,
						"net/netfilter/nf_log",
						table);
	if (!net->nf.nf_log_dir_header)
		goto err_reg;

	return 0;

err_reg:
	if (!net_eq(net, &init_net))
		kfree(table);
err_alloc:
	return -ENOMEM;
}

static void netfilter_log_sysctl_exit(struct net *net)
{
	struct ctl_table *table;

	table = net->nf.nf_log_dir_header->ctl_table_arg;
	unregister_net_sysctl_table(net->nf.nf_log_dir_header);
	if (!net_eq(net, &init_net))
		kfree(table);
}
#else
static int netfilter_log_sysctl_init(struct net *net)
{
	return 0;
}

static void netfilter_log_sysctl_exit(struct net *net)
{
}
#endif /* CONFIG_SYSCTL */

static int __net_init nf_log_net_init(struct net *net)
{
	int ret = -ENOMEM;

#ifdef CONFIG_PROC_FS
	if (!proc_create("nf_log", S_IRUGO,
			 net->nf.proc_netfilter, &nflog_file_ops))
		return ret;
#endif
	ret = netfilter_log_sysctl_init(net);
	if (ret < 0)
		goto out_sysctl;

	return 0;

out_sysctl:
#ifdef CONFIG_PROC_FS
	/* For init_net: errors will trigger panic, don't unroll on error. */
	if (!net_eq(net, &init_net))
		remove_proc_entry("nf_log", net->nf.proc_netfilter);
#endif
	return ret;
}

static void __net_exit nf_log_net_exit(struct net *net)
{
	netfilter_log_sysctl_exit(net);
#ifdef CONFIG_PROC_FS
	remove_proc_entry("nf_log", net->nf.proc_netfilter);
#endif
}

static struct pernet_operations nf_log_net_ops = {
	.init = nf_log_net_init,
	.exit = nf_log_net_exit,
};

int __init netfilter_log_init(void)
{
	int i, ret;

	ret = register_pernet_subsys(&nf_log_net_ops);
	if (ret < 0)
		return ret;

	for (i = NFPROTO_UNSPEC; i < NFPROTO_NUMPROTO; i++)
		INIT_LIST_HEAD(&(nf_loggers_l[i]));

	return 0;
}
