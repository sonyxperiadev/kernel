#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/io.h>

extern void KRIL_SysRpc_Init(void);

MODULE_LICENSE("Dual BSD/GPL");

#define SYSRPC_PROCFS_NAME "sysrpc"

static int procfile_read(char *page, char **start,
			 off_t offset, int count, int *eof, void *data);

static int procfile_write(struct file *file, const char *buffer,
			  unsigned long count, void *data);

static struct proc_dir_entry *g_sysrpc_procfile = 0;

static int __init sysrpc_init(void)
{
	g_sysrpc_procfile = create_proc_entry(SYSRPC_PROCFS_NAME,
					      S_IRWXU | S_IRWXG | S_IRWXO,
					      NULL);

	if (g_sysrpc_procfile == NULL) {
		remove_proc_entry(SYSRPC_PROCFS_NAME, NULL);
		printk(KERN_ALERT "Error: Could not initialize /proc/%s\n",
		       SYSRPC_PROCFS_NAME);
		return -ENOMEM;
	}

	g_sysrpc_procfile->read_proc = procfile_read;
	g_sysrpc_procfile->write_proc = procfile_write;
	g_sysrpc_procfile->mode = S_IRWXU | S_IRWXG | S_IRWXO;

	return 0;
}

static void __exit sysrpc_exit(void)
{
	if (g_sysrpc_procfile)
		remove_proc_entry(SYSRPC_PROCFS_NAME, NULL);

}

#ifdef INCLUDE_UNUSED_CODE
static void safe_strncat(char *dst, const char *src, int len)
{
	int l_dst = strlen(dst);
	int l_src = strlen(src);

	if (l_dst + l_src < len)
		strncat(dst, src, len - l_dst - l_src);
}
#endif /*INCLUDE_UNUSED_CODE*/

static int procfile_read(char *page, char **start, off_t offset,
			 int count, int *eof, void *data)
{
	*page = 0;
	*eof = 1;
	return 1 + strlen(page);
}

static int procfile_write(struct file *file, const char *buffer,
			  unsigned long count, void *data)
{
	return count;
}

module_init(sysrpc_init);
module_exit(sysrpc_exit);
