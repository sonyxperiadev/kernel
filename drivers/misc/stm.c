/*
 * Copyright (C) ST-Ericsson SA 2010
 * Author: Pierre Peiffer <pierre.peiffer@stericsson.com> for ST-Ericsson.
 *         Philippe Langlais <philippe.Langlais@stericsson.com> for ST-Ericsson.
 * License terms: GNU General Public License (GPL), version 2.
 */

#include <linux/io.h>
#include <linux/mm.h>
#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/debugfs.h>
#include <trace/stm.h>

/* STM Registers */
#define STM_CR          (stm.virtbase)
#define STM_MMC         (stm.virtbase + 0x008)
#define STM_TER         (stm.virtbase + 0x010)
#define STMPERIPHID0    (stm.virtbase + 0xFC0)
#define STMPERIPHID1    (stm.virtbase + 0xFC8)
#define STMPERIPHID2    (stm.virtbase + 0xFD0)
#define STMPERIPHID3    (stm.virtbase + 0xFD8)
#define STMPCELLID0     (stm.virtbase + 0xFE0)
#define STMPCELLID1     (stm.virtbase + 0xFE8)
#define STMPCELLID2     (stm.virtbase + 0xFF0)
#define STMPCELLID3     (stm.virtbase + 0xFF8)

#define STM_CLOCK_SHIFT 6
#define STM_CLOCK_MASK  0x1C0

/* Hardware mode for all sources */
#define STM_MMC_DEFAULT            0xFFFFFFFF

/* Max number of channels (multiple of 256) */
#define STM_NUMBER_OF_CHANNEL      CONFIG_STM_NUMBER_OF_CHANNEL

/* # dynamically allocated channel with stm_trace_buffer */
#define NB_KERNEL_DYNAMIC_CHANNEL  128

static struct stm_device {
	const struct stm_platform_data *pdata;
	void __iomem *virtbase;
	/* Used to register the allocated channels */
	DECLARE_BITMAP(ch_bitmap, STM_NUMBER_OF_CHANNEL);
} stm;

volatile struct stm_channel __iomem *stm_channels;

static struct cdev cdev;
static struct class *stm_class;
static int stm_major;

static DEFINE_SPINLOCK(lock);

/* Middle value for clock divisor */
static enum clock_div stm_clockdiv = STM_CLOCK_DIV8;

/* Default value for STM output connection */
static enum stm_connection_type stm_connection = STM_DEFAULT_CONNECTION;

#define STM_BUFSIZE    256
struct channel_data {
	DECLARE_BITMAP(bitmap, STM_NUMBER_OF_CHANNEL);
	int numero;
	spinlock_t lock;
	u8 data_buffer[STM_BUFSIZE];
};

static u64 stm_printk_buf[1024/sizeof(u64)];
static arch_spinlock_t stm_buf_lock =
	        (arch_spinlock_t)__ARCH_SPIN_LOCK_UNLOCKED;

int stm_alloc_channel(int offset)
{
	int channel;

	/* Look for a free channel from offset */
	do {
	        channel = find_next_zero_bit(stm.ch_bitmap,
	                        STM_NUMBER_OF_CHANNEL, offset);
	} while ((channel < STM_NUMBER_OF_CHANNEL)
	       && test_and_set_bit(channel, stm.ch_bitmap));
	return channel;
}
EXPORT_SYMBOL(stm_alloc_channel);

void stm_free_channel(int channel)
{
	clear_bit(channel, stm.ch_bitmap);
}
EXPORT_SYMBOL(stm_free_channel);

static int stm_get_channel(struct channel_data *ch_data, int __user *arg)
{
	int channel, err;

	channel = stm_alloc_channel(0);
	if (channel < STM_NUMBER_OF_CHANNEL) {
	        /* One free found ! */
	        err = put_user(channel, arg);
	        if (err)
	                stm_free_channel(channel);
	        else
	                /* Register it in the context of the file */
	                set_bit(channel, ch_data->bitmap);
	} else
	        err = -ENOMEM;
	return err;
}

static int stm_release_channel(struct channel_data *ch_data, int channel)
{
	if ((channel < 0) || (channel >= STM_NUMBER_OF_CHANNEL))
	        return -EINVAL;
	stm_free_channel(channel);
	clear_bit(channel, ch_data->bitmap);
	return 0;
}

/*
 * Trace a buffer on a given channel
 * with auto time stamping on last byte(s) only
 */
int stm_trace_buffer_onchannel(int channel,
	                       const void *data, size_t length)
{
	int i, mod64;
	volatile struct stm_channel __iomem *pch;

	if (channel >= STM_NUMBER_OF_CHANNEL || !stm_channels)
	        return 0;

	pch = &stm_channels[channel];

	/*  Align data pointer to u64 & time stamp last byte(s) */
	mod64 = (int)data & 7;
	i = length - 8 + mod64;
	switch (mod64) {
	case 0:
	        if (i)
	                pch->no_stamp64 = *(u64 *)data;
	        else {
	                pch->stamp64 = *(u64 *)data;
	                return length;
	        }
	        data += 8;
	        break;
	case 1:
	        pch->no_stamp8 = *(u8 *)data;
	        pch->no_stamp16 = *(u16 *)(data+1);
	        if (i)
	                pch->no_stamp32 = *(u32 *)(data+3);
	        else {
	                pch->stamp32 = *(u32 *)(data+3);
	                return length;
	        }
	        data += 7;
	        break;
	case 2:
	        pch->no_stamp16 = *(u16 *)data;
	        if (i)
	                pch->no_stamp32 = *(u32 *)(data+2);
	        else {
	                pch->stamp32 = *(u32 *)(data+2);
	                return length;
	        }
	        data += 6;
	        break;
	case 3:
	        pch->no_stamp8 = *(u8 *)data;
	        if (i)
	                pch->no_stamp32 = *(u32 *)(data+1);
	        else {
	                pch->stamp32 = *(u32 *)(data+1);
	                return length;
	        }
	        data += 5;
	        break;
	case 4:
	        if (i)
	                pch->no_stamp32 = *(u32 *)data;
	        else {
	                pch->stamp32 = *(u32 *)data;
	                return length;
	        }
	        data += 4;
	        break;
	case 5:
	        pch->no_stamp8 = *(u8 *)data;
	        if (i)
	                pch->no_stamp16 = *(u16 *)(data+1);
	        else {
	                pch->stamp16 = *(u16 *)(data+1);
	                return length;
	        }
	        data += 3;
	        break;
	case 6:
	        if (i)
	                pch->no_stamp16 = *(u16 *)data;
	        else {
	                pch->stamp16 = *(u16 *)data;
	                return length;
	        }
	        data += 2;
	        break;
	case 7:
	        if (i)
	                pch->no_stamp8 = *(u8 *)data;
	        else {
	                pch->stamp8 = *(u8 *)data;
	                return length;
	        }
	        data++;
	        break;
	}
	for (;;) {
	        if (i > 8) {
	                pch->no_stamp64 = *(u64 *)data;
	                data += 8;
	                i -= 8;
	        } else if (i == 8) {
	                pch->stamp64 = *(u64 *)data;
	                break;
	        } else if (i > 4) {
	                pch->no_stamp32 = *(u32 *)data;
	                data += 4;
	                i -= 4;
	        } else if (i == 4) {
	                pch->stamp32 = *(u32 *)data;
	                break;
	        } else if (i > 2) {
	                pch->no_stamp16 = *(u16 *)data;
	                data += 2;
	                i -= 2;
	        } else if (i == 2) {
	                pch->stamp16 = *(u16 *)data;
	                break;
	        } else {
	                pch->stamp8 = *(u8 *)data;
	                break;
	        }
	}
	return length;
}
EXPORT_SYMBOL(stm_trace_buffer_onchannel);

static int stm_open(struct inode *inode, struct file *file)
{
	struct channel_data *channel_data;

	channel_data = kzalloc(sizeof(struct channel_data), GFP_KERNEL);
	if (channel_data == NULL)
	        return -ENOMEM;

	spin_lock_init(&channel_data->lock);
	channel_data->numero = -1;   /*  Channel not yet allocated */
	file->private_data = channel_data;

	return 0;
}

static int stm_release(struct inode *inode, struct file *file)
{
	struct channel_data *channel;

	channel = (struct channel_data *)file->private_data;

	/* Free allocated channel if necessary */
	if (channel->numero != -1)
	        stm_free_channel(channel->numero);

	bitmap_andnot(stm.ch_bitmap, stm.ch_bitmap,
	                channel->bitmap, STM_NUMBER_OF_CHANNEL);

	kfree(channel);
	return 0;
}

static ssize_t stm_write(struct file *file, const char __user *buf,
	          size_t size, loff_t *off)
{
	struct channel_data *channel = file->private_data;

	/*  Alloc channel at first write */
	if (channel->numero == -1) {
	        channel->numero = stm_alloc_channel(0);
	        if (channel->numero > STM_NUMBER_OF_CHANNEL)
	                return -ENOMEM;
	}

	if (size > STM_BUFSIZE)
	        size = STM_BUFSIZE;

	spin_lock(&channel->lock);

	if (copy_from_user
	    (channel->data_buffer, (void __user *) buf, size)) {
	        spin_unlock(&channel->lock);
	        return -EFAULT;
	}
	size = stm_trace_buffer_onchannel(channel->numero,
	                channel->data_buffer, size);

	spin_unlock(&channel->lock);

	return size;
}

static int stm_mmap(struct file *file, struct vm_area_struct *vma)
{
	/*
	 * Don't allow a mapping that covers more than the STM channels
	 */
	if ((vma->vm_end - vma->vm_start) >
	    STM_NUMBER_OF_CHANNEL*sizeof(struct stm_channel))
	        return -EINVAL;

	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

	if (io_remap_pfn_range(vma, vma->vm_start,
	                stm.pdata->channels_phys_base>>PAGE_SHIFT,
	                STM_NUMBER_OF_CHANNEL*sizeof(struct stm_channel),
	                vma->vm_page_prot))
	        return -EAGAIN;

	return 0;
}

/* Enable the trace for given sources (bitfield) */
static void stm_enable_src(unsigned int v)
{
	unsigned int cr_val;
	spin_lock(&lock);
	cr_val = readl(STM_CR);
	cr_val &= ~STM_CLOCK_MASK;
	writel(cr_val|(stm_clockdiv<<STM_CLOCK_SHIFT), STM_CR);
	writel(v, STM_TER);
	spin_unlock(&lock);
}

/* Disable all sources */
static void stm_disable_src(void)
{
	writel(0x0, STM_CR);   /* stop clock */
	writel(0x0, STM_TER);  /* Disable cores */
}

/* Set clock speed */
static int stm_set_ckdiv(enum clock_div v)
{
	unsigned int val;

	spin_lock(&lock);
	val = readl(STM_CR);
	val &= ~STM_CLOCK_MASK;
	writel(val | ((v << STM_CLOCK_SHIFT) & STM_CLOCK_MASK), STM_CR);
	spin_unlock(&lock);
	stm_clockdiv = v;

	return 0;
}

/* Return the control register */
static inline unsigned int stm_get_cr(void)
{
	return readl(STM_CR);
}

/*
 * Set Trace MODE lossless/lossy (Software/Hardware)
 * each bit represent the corresponding mode of this source
 */
static inline void stm_set_modes(unsigned int modes)
{
	writel(modes, STM_MMC);
}

/* Get Trace MODE lossless/lossy (Software/Hardware)
 * each bit represent the corresponding mode of this source */
static inline unsigned int stm_get_modes(void)
{
	return readl(STM_MMC);
}

/* Count # of free channels */
static int stm_nb_free_channels(void)
{
	int nb_channels, offset;

	nb_channels = 0;
	offset = 0;
	for (;;) {
	        offset = find_next_zero_bit(stm.ch_bitmap,
	                        STM_NUMBER_OF_CHANNEL, offset);
	        if (offset == STM_NUMBER_OF_CHANNEL)
	                break;
	        offset++;
	        nb_channels++;
	}
	return nb_channels;
}

static long stm_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int err = 0;
	struct channel_data *channel = file->private_data;

	switch (cmd) {

	case STM_CONNECTION:
	        if (stm.pdata->stm_connection)
	                stm.pdata->stm_connection(arg);
	        stm_connection = arg;
	        break;

	case STM_DISABLE:
	        stm_disable_src();
	        break;

	case STM_GET_NB_MAX_CHANNELS:
	        err = put_user(STM_NUMBER_OF_CHANNEL, (unsigned int *)arg);
	        break;

	case STM_GET_NB_FREE_CHANNELS:
	        err = put_user(stm_nb_free_channels(), (unsigned int *)arg);
	        break;

	case STM_GET_CHANNEL_NO:
	        err = put_user(channel->numero, (unsigned int *)arg);
	        break;

	case STM_SET_CLOCK_DIV:
	        err = stm_set_ckdiv((enum clock_div) arg);
	        break;

	case STM_SET_MODE:
	        stm_set_modes(arg);
	        break;

	case STM_GET_MODE:
	        err = put_user(stm_get_modes(), (unsigned int *)arg);
	        break;

	case STM_GET_CTRL_REG:
	        err = put_user(stm_get_cr(), (unsigned int *)arg);
	        break;

	case STM_ENABLE_SRC:
	        stm_enable_src(arg);
	        break;

	case STM_GET_FREE_CHANNEL:
	        err = stm_get_channel(channel, (int *)arg);
	        break;

	case STM_RELEASE_CHANNEL:
	        err = stm_release_channel(channel, arg);
	        break;

	default:
	        err = -EINVAL;
	        break;
	}

	return err;
}

/*
 * Trace a buffer on a dynamically allocated channel
 * with auto time stamping on the first byte(s) only
 * Dynamic channel number >=
 *     STM_NUMBER_OF_CHANNEL - NB_KERNEL_DYNAMIC_CHANNEL
 */
int stm_trace_buffer(const void *data, size_t length)
{
	int channel;

	channel = stm_alloc_channel(STM_NUMBER_OF_CHANNEL
	                - NB_KERNEL_DYNAMIC_CHANNEL);
	if (channel < STM_NUMBER_OF_CHANNEL) {
	        length = stm_trace_buffer_onchannel(channel, data, length);
	        stm_free_channel(channel);
	        return length;
	}
	return 0;
}
EXPORT_SYMBOL(stm_trace_buffer);

static const struct file_operations stm_fops = {
	.owner =                THIS_MODULE,
	.unlocked_ioctl =       stm_ioctl,
	.open =                 stm_open,
	.llseek =               no_llseek,
	.write =                stm_write,
	.release =              stm_release,
	.mmap =                 stm_mmap,
};

/*
 * Init and deinit driver
 */

static int __devinit stm_probe(struct platform_device *pdev)
{
	int retval = 0;

	if (!pdev || !pdev->dev.platform_data)  {
	        pr_alert("No device/platform_data found on STM driver\n");
	        return -ENODEV;
	}

	stm.pdata = pdev->dev.platform_data;

	cdev_init(&cdev, &stm_fops);
	cdev.owner = THIS_MODULE;

	stm_channels =
	        ioremap_nocache(stm.pdata->channels_phys_base,
	                STM_NUMBER_OF_CHANNEL*sizeof(*stm_channels));
	if (stm_channels == NULL) {
	        dev_err(&pdev->dev, "could not remap STM Msg register\n");
	        return -ENODEV;
	}

	stm.virtbase = ioremap_nocache(stm.pdata->regs_phys_base, SZ_4K);
	if (stm.virtbase == NULL) {
	        retval = -EIO;
	        dev_err(&pdev->dev, "could not remap STM Register\n");
	        goto err_channels;
	}

	retval = cdev_add(&cdev, MKDEV(stm_major, 0), 1);
	if (retval) {
	        dev_err(&pdev->dev, "chardev registration failed\n");
	        goto err_channels;
	}

	if (IS_ERR(device_create(stm_class, &pdev->dev,
	                         MKDEV(stm_major, 0), NULL, STM_DEV_NAME)))
	        dev_err(&pdev->dev, "can't create device\n");

	/* Check chip IDs if necessary */
	if (stm.pdata->id_mask) {
	        u32 periph_id, cell_id;

	        periph_id = (readb(STMPERIPHID3)<<24) +
	                         (readb(STMPERIPHID2)<<16) +
	                         (readb(STMPERIPHID1)<<8) +
	                         readb(STMPERIPHID0);
	        cell_id = (readb(STMPCELLID3)<<24) +
	                         (readb(STMPCELLID2)<<16) +
	                         (readb(STMPCELLID1)<<8) +
	                         readb(STMPCELLID0);
	        /* Only warns if it isn't a ST-Ericsson supported one */
	        if ((periph_id & stm.pdata->id_mask) != 0x00080dec ||
	            cell_id != 0xb105f00d) {
	                dev_warn(&pdev->dev, "STM-Trace IC not compatible\n");
	                dev_warn(&pdev->dev, "periph_id=%x\n", periph_id);
	                dev_warn(&pdev->dev, "pcell_id=%x\n", cell_id);
	        }
	}

	/* Reserve channels if necessary */
	if (stm.pdata->channels_reserved_sz) {
	        int i;

	        for (i = 0; i < stm.pdata->channels_reserved_sz; i++) {
	                set_bit(stm.pdata->channels_reserved[i],
	                                stm.ch_bitmap);
	        }
	}
	/* Reserve kernel trace channels on demand */
#ifdef CONFIG_STM_PRINTK
	set_bit(CONFIG_STM_PRINTK_CHANNEL, stm.ch_bitmap);
#endif
#ifdef CONFIG_STM_FTRACE
	set_bit(CONFIG_STM_FTRACE_CHANNEL, stm.ch_bitmap);
#endif
#ifdef CONFIG_STM_CTX_SWITCH
	set_bit(CONFIG_STM_CTX_SWITCH_CHANNEL, stm.ch_bitmap);
#endif
#ifdef CONFIG_STM_WAKEUP
	set_bit(CONFIG_STM_WAKEUP_CHANNEL, stm.ch_bitmap);
#endif
#ifdef CONFIG_STM_STACK_TRACE
	set_bit(CONFIG_STM_STACK_TRACE_CHANNEL, stm.ch_bitmap);
#endif
#ifdef CONFIG_STM_TRACE_PRINTK
	set_bit(CONFIG_STM_TRACE_PRINTK_CHANNEL, stm.ch_bitmap);
	set_bit(CONFIG_STM_TRACE_BPRINTK_CHANNEL, stm.ch_bitmap);
#endif

	if (stm.pdata->stm_connection) {
	        retval = stm.pdata->stm_connection(stm_connection);
	        if (retval) {
	                dev_err(&pdev->dev, "failed to connect STM output\n");
	                goto err_channels;
	        }
	}

	/* Enable STM Masters given in pdata */
	if (stm.pdata->masters_enabled)
	        stm_enable_src(stm.pdata->masters_enabled);

	stm_set_modes(STM_MMC_DEFAULT); /* Set all sources in HW mode */

	dev_info(&pdev->dev, "STM-Trace driver probed successfully\n");
	stm_printk("STM-Trace driver initialized\n");
	return 0;

err_channels:
	iounmap(stm_channels);
	return retval;
}

static int __devexit stm_remove(struct platform_device *pdev)
{
	device_destroy(stm_class, MKDEV(stm_major, 0));
	cdev_del(&cdev);

	if (stm.pdata->stm_connection)
	        (void) stm.pdata->stm_connection(STM_DISCONNECT);

	stm_disable_src();
	iounmap(stm.virtbase);
	iounmap(stm_channels);

	return 0;
}

int stm_printk(const char *fmt, ...)
{
	int ret;
	size_t size;
	va_list args;

	va_start(args, fmt);
	arch_spin_lock(&stm_buf_lock);
	size = vscnprintf((char *)stm_printk_buf,
	                sizeof(stm_printk_buf), fmt, args);
	ret = stm_trace_buffer(stm_printk_buf, size);
	arch_spin_unlock(&stm_buf_lock);
	va_end(args);
	return ret;
}
EXPORT_SYMBOL(stm_printk);

/*
 * Debugfs interface
 */

static int stm_connection_show(void *data, u64 *val)
{
	*val = stm_connection;
	return 0;
}

static int stm_connection_set(void *data, u64 val)
{
	if (stm.pdata->stm_connection) {
	        stm_connection = val;
	        stm.pdata->stm_connection(val);
	}
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(stm_connection_fops, stm_connection_show,
	        stm_connection_set, "%llu\n");

static int stm_clockdiv_show(void *data, u64 *val)
{
	*val = stm_clockdiv;
	return 0;
}

static int stm_clockdiv_set(void *data, u64 val)
{
	stm_set_ckdiv(val);
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(stm_clockdiv_fops, stm_clockdiv_show,
	        stm_clockdiv_set, "%llu\n");

static int stm_masters_enable_show(void *data, u64 *val)
{
	*val = readl(STM_TER);
	return 0;
}

static int stm_masters_enable_set(void *data, u64 val)
{
	stm_enable_src(val);
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(stm_masters_enable_fops, stm_masters_enable_show,
	        stm_masters_enable_set, "%08llx\n");

static int stm_masters_modes_show(void *data, u64 *val)
{
	*val = stm_get_modes();
	return 0;
}

static int stm_masters_modes_set(void *data, u64 val)
{
	stm_set_modes(val);
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(stm_masters_modes_fops, stm_masters_modes_show,
	        stm_masters_modes_set, "%08llx\n");

/* Count # of free channels */
static int stm_free_channels_show(void *data, u64 *val)
{
	*val = stm_nb_free_channels();
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(stm_free_channels_fops, stm_free_channels_show,
	        NULL, "%lld\n");

static __init int stm_init_debugfs(void)
{
	struct dentry *d_stm;

	d_stm =  debugfs_create_dir(STM_DEV_NAME, NULL);
	if (!d_stm)
	        return -ENOMEM;

	(void) debugfs_create_file("connection",  S_IRUGO | S_IWUGO, d_stm,
	                        NULL, &stm_connection_fops);
	(void) debugfs_create_file("clockdiv", S_IRUGO | S_IWUGO, d_stm,
	                        NULL, &stm_clockdiv_fops);
	(void) debugfs_create_file("masters_enable", S_IRUGO | S_IWUGO, d_stm,
	                        NULL, &stm_masters_enable_fops);
	(void) debugfs_create_file("masters_modes", S_IRUGO | S_IWUGO, d_stm,
	                        NULL, &stm_masters_modes_fops);
	(void) debugfs_create_file("free_channels", S_IRUGO, d_stm,
	                        NULL, &stm_free_channels_fops);
	return 0;
}
fs_initcall(stm_init_debugfs);

static struct platform_driver stm_driver = {
	.probe = stm_probe,
	.remove = __devexit_p(stm_remove),
	.driver = {
	        .name = STM_DEV_NAME,
	        .owner = THIS_MODULE,
	}
};

static int __init stm_init(void)
{
	int retval;
	dev_t dev;

	stm_class = class_create(THIS_MODULE, STM_DEV_NAME);
	if (IS_ERR(stm_class)) {
	        pr_err("stm: can't register stm class\n");
	        return PTR_ERR(stm_class);
	}

	retval = alloc_chrdev_region(&dev, 0, 1, STM_DEV_NAME);
	if (retval) {
	        pr_err("stm: can't register character device\n");
	        class_destroy(stm_class);
	        return retval;
	}
	stm_major = MAJOR(dev);
	return platform_driver_register(&stm_driver);
}

static void __exit stm_exit(void)
{
	platform_driver_unregister(&stm_driver);
	unregister_chrdev_region(MKDEV(stm_major, 0), 1);
	class_destroy(stm_class);
}

arch_initcall(stm_init);   /* STM init ASAP need to wait GPIO init */
module_exit(stm_exit);

MODULE_AUTHOR("Paul Ghaleb - ST Microelectronics");
MODULE_AUTHOR("Pierre Peiffer - ST-Ericsson");
MODULE_AUTHOR("Philippe Langlais - ST-Ericsson");
MODULE_DESCRIPTION("System Trace Module driver");
MODULE_ALIAS("stm");
MODULE_ALIAS("stm-trace");
MODULE_LICENSE("GPL v2");
